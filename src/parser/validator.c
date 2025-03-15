#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "validator.h"

// Inicializar un resultado de validación
ValidationResult* validator_create_result() {
    ValidationResult* result = (ValidationResult*)malloc(sizeof(ValidationResult));
    if (result) {
        result->error_code = 0;
        result->error_message = NULL;
        result->error_position = -1;
    }
    return result;
}

// Liberar un resultado de validación
void validator_free_result(ValidationResult* result) {
    if (!result) return;
    
    if (result->error_message) {
        free(result->error_message);
    }
    
    free(result);
}

// Establecer un error de validación
int validator_set_error(ValidationResult* result, int code, const char* message) {
    if (!result) return 0;
    
    result->error_code = code;
    
    if (result->error_message) {
        free(result->error_message);
    }
    
    result->error_message = strdup(message);
    return 0;  // Retorna 0 para indicar fallo
}

// Buscar una tabla por nombre
Table* validator_find_table(const char* table_name, Database* db) {
    if (!table_name || !db) return NULL;
    
    for (int i = 0; i < db->num_tables; i++) {
        if (strcasecmp(db->tables[i]->name, table_name) == 0) {
            return db->tables[i];
        }
    }
    
    return NULL;
}

// Verificar si una columna existe en una tabla
int validator_check_column_exists(const char* column_name, Table* table) {
    if (!column_name || !table) return 0;
    
    for (int i = 0; i < table->num_columns; i++) {
        if (strcasecmp(table->columns[i].name, column_name) == 0) {
            return 1;
        }
    }
    
    return 0;
}

// Obtener una columna por nombre
Column* validator_get_column(const char* column_name, Table* table) {
    if (!column_name || !table) return NULL;
    
    for (int i = 0; i < table->num_columns; i++) {
        if (strcasecmp(table->columns[i].name, column_name) == 0) {
            return &table->columns[i];
        }
    }
    
    return NULL;
}

// Verificar compatibilidad de tipos
int validator_check_type_compatibility(int expected_type, int actual_type) {
    // Si los tipos son exactamente iguales
    if (expected_type == actual_type) return 1;
    
    // Conversiones implícitas permitidas:
    // INT -> FLOAT
    if (expected_type == TYPE_FLOAT && actual_type == TYPE_INT) return 1;
    
    // BOOL -> INT
    if (expected_type == TYPE_INT && actual_type == TYPE_BOOL) return 1;
    
    // STRING puede convertirse en cualquier otro tipo (depende del valor)
    if (expected_type == TYPE_STRING) return 1;
    
    return 0;
}

// Convierte de LiteralType (del AST) a tipo de columna (de la base de datos)
int ast_type_to_column_type(LiteralType lit_type) {
    switch (lit_type) {
        case LIT_INTEGER: return TYPE_INT;
        case LIT_FLOAT: return TYPE_FLOAT;
        case LIT_STRING: return TYPE_STRING;
        case LIT_BOOLEAN: return TYPE_BOOL;
        default: return -1;  // Tipo desconocido
    }
}

// Validar una lista de columnas contra una tabla
int validator_validate_column_list(ASTNode* columns_node, Table* table, ValidationResult* result) {
    if (!columns_node || !table || !result) return 0;
    
    // Si columns_node no es de tipo NODE_COLUMN_LIST, es un error
    if (columns_node->type != NODE_COLUMN_LIST) {
        return validator_set_error(result, 101, "Se esperaba una lista de columnas");
    }
    
    ColumnListData* columns = (ColumnListData*)columns_node->data;
    
    // Si es SELECT *, todas las columnas son válidas
    if (columns->is_all) {
        return 1;
    }
    
    // Verificar cada columna individualmente
    for (int i = 0; i < columns->count; i++) {
        if (!validator_check_column_exists(columns->columns[i], table)) {
            char error[200];
            snprintf(error, sizeof(error), "La columna '%s' no existe en la tabla '%s'", 
                     columns->columns[i], table->name);
            return validator_set_error(result, 102, error);
        }
    }
    
    return 1;
}

// Validar una expresión contra una tabla
int validator_validate_expression(ASTNode* expr, Table* table, ValidationResult* result) {
    if (!expr || !table || !result) return 0;
    
    switch (expr->type) {
        case NODE_IDENTIFIER: {
            // Verificar que el identificador sea una columna válida
            IdentifierData* id_data = (IdentifierData*)expr->data;
            if (!validator_check_column_exists(id_data->name, table)) {
                char error[200];
                snprintf(error, sizeof(error), "La columna '%s' no existe en la tabla '%s'", 
                         id_data->name, table->name);
                return validator_set_error(result, 103, error);
            }
            break;
        }
        case NODE_BINARY_EXPR: {
            // Validar recursivamente ambos lados de la expresión
            BinaryExprData* bin_data = (BinaryExprData*)expr->data;
            if (!validator_validate_expression(bin_data->left, table, result)) {
                return 0;
            }
            if (!validator_validate_expression(bin_data->right, table, result)) {
                return 0;
            }
            
            // Validar operaciones específicas (como comparación entre tipos compatibles)
            if (bin_data->op_type == OP_EQ || bin_data->op_type == OP_NEQ ||
                bin_data->op_type == OP_LT || bin_data->op_type == OP_GT ||
                bin_data->op_type == OP_LTE || bin_data->op_type == OP_GTE) {
                
                // Si estamos comparando una columna con un literal, verificar compatibilidad
                if (bin_data->left->type == NODE_IDENTIFIER && bin_data->right->type == NODE_LITERAL) {
                    IdentifierData* id_data = (IdentifierData*)bin_data->left->data;
                    LiteralData* lit_data = (LiteralData*)bin_data->right->data;
                    
                    Column* column = validator_get_column(id_data->name, table);
                    if (column) {
                        int literal_type = ast_type_to_column_type(lit_data->lit_type);
                        if (!validator_check_type_compatibility(column->type, literal_type)) {
                            char error[200];
                            snprintf(error, sizeof(error), 
                                    "Incompatibilidad de tipos: no se puede comparar columna '%s' (%d) con valor de tipo %d", 
                                    id_data->name, column->type, literal_type);
                            return validator_set_error(result, 104, error);
                        }
                    }
                }
                else if (bin_data->right->type == NODE_IDENTIFIER && bin_data->left->type == NODE_LITERAL) {
                    IdentifierData* id_data = (IdentifierData*)bin_data->right->data;
                    LiteralData* lit_data = (LiteralData*)bin_data->left->data;
                    
                    Column* column = validator_get_column(id_data->name, table);
                    if (column) {
                        int literal_type = ast_type_to_column_type(lit_data->lit_type);
                        if (!validator_check_type_compatibility(column->type, literal_type)) {
                            char error[200];
                            snprintf(error, sizeof(error), 
                                    "Incompatibilidad de tipos: no se puede comparar columna '%s' (%d) con valor de tipo %d", 
                                    id_data->name, column->type, literal_type);
                            return validator_set_error(result, 104, error);
                        }
                    }
                }
            }
            break;
        }
        case NODE_UNARY_EXPR: {
            // Validar recursivamente el operando
            UnaryExprData* un_data = (UnaryExprData*)expr->data;
            if (!validator_validate_expression(un_data->operand, table, result)) {
                return 0;
            }
            break;
        }
        case NODE_LITERAL:
            // Los literales siempre son válidos en sí mismos
            break;
        default:
            return validator_set_error(result, 105, "Tipo de expresión no válido");
    }
    
    return 1;
}

// Validar una cláusula WHERE
int validator_validate_where_clause(ASTNode* where_node, Table* table, ValidationResult* result) {
    if (!where_node || !table || !result) return 0;
    
    // Si where_node no es de tipo NODE_WHERE_CLAUSE, es un error
    if (where_node->type != NODE_WHERE_CLAUSE) {
        return validator_set_error(result, 106, "Se esperaba una cláusula WHERE");
    }
    
    WhereClauseData* where = (WhereClauseData*)where_node->data;
    
    // Validar la condición
    return validator_validate_expression(where->condition, table, result);
}

// Validar valores contra una tabla (para INSERT)
int validator_validate_values(ASTNode* values_node, Table* table, ValidationResult* result) {
    if (!values_node || !table || !result) return 0;
    
    // Si values_node no es de tipo NODE_VALUE_LIST, es un error
    if (values_node->type != NODE_VALUE_LIST) {
        return validator_set_error(result, 107, "Se esperaba una lista de valores");
    }
    
    ValueListData* values = (ValueListData*)values_node->data;
    
    // Verificar que el número de valores coincida con el número de columnas
    if (values->count != table->num_columns) {
        char error[200];
        snprintf(error, sizeof(error), "Número incorrecto de valores. Se esperaban %d valores, pero se proporcionaron %d", 
                 table->num_columns, values->count);
        return validator_set_error(result, 108, error);
    }
    
    // Verificar el tipo de cada valor
    for (int i = 0; i < values->count; i++) {
        ASTNode* value = values->values[i];
        
        // Para literales, verificar compatibilidad de tipo
        if (value->type == NODE_LITERAL) {
            LiteralData* lit_data = (LiteralData*)value->data;
            int literal_type = ast_type_to_column_type(lit_data->lit_type);
            
            if (!validator_check_type_compatibility(table->columns[i].type, literal_type)) {
                char error[200];
                snprintf(error, sizeof(error), "Tipo no compatible para columna '%s'. Valor de tipo %d no es compatible con columna de tipo %d", 
                         table->columns[i].name, literal_type, table->columns[i].type);
                return validator_set_error(result, 109, error);
            }
            
            // Verificar restricción NOT NULL
            if (lit_data->lit_type == LIT_NULL && !table->columns[i].allows_null) {
                char error[200];
                snprintf(error, sizeof(error), "No se permite NULL en la columna '%s'", 
                         table->columns[i].name);
                return validator_set_error(result, 110, error);
            }
            
            // Para strings, verificar longitud máxima
            if (lit_data->lit_type == LIT_STRING && table->columns[i].type == TYPE_STRING) {
                int str_length = strlen(lit_data->string_value);
                if (str_length > table->columns[i].max_length) {
                    char error[200];
                    snprintf(error, sizeof(error), "El valor excede la longitud máxima para columna '%s'. Longitud: %d, máximo permitido: %d", 
                             table->columns[i].name, str_length, table->columns[i].max_length);
                    return validator_set_error(result, 111, error);
                }
            }
        } else {
            // Solo se permiten literales en INSERT VALUES
            return validator_set_error(result, 112, "Los valores para INSERT deben ser literales");
        }
    }
    
    return 1;
}

// Validar lista de asignaciones para UPDATE
int validator_validate_assignments(ASTNode* assignments_node, Table* table, ValidationResult* result) {
    if (!assignments_node || !table || !result) return 0;
    
    ASTNode* current = assignments_node;
    
    // Validar cada asignación
    while (current != NULL) {
        if (current->type != NODE_ASSIGNMENT) {
            return validator_set_error(result, 113, "Se esperaba una asignación");
        }
        
        AssignmentData* assign_data = (AssignmentData*)current->data;
        
        // Verificar que la columna exista
        if (!validator_check_column_exists(assign_data->column_name, table)) {
            char error[200];
            snprintf(error, sizeof(error), "La columna '%s' no existe en la tabla '%s'", 
                     assign_data->column_name, table->name);
            return validator_set_error(result, 114, error);
        }
        
        Column* column = validator_get_column(assign_data->column_name, table);
        
        // No permitir modificar la clave primaria
        if (column->is_primary_key) {
            char error[200];
            snprintf(error, sizeof(error), "No se puede modificar la columna '%s' porque es clave primaria", 
                     assign_data->column_name);
            return validator_set_error(result, 115, error);
        }
        
        // Validar el valor asignado
        if (assign_data->value->type == NODE_LITERAL) {
            LiteralData* lit_data = (LiteralData*)assign_data->value->data;
            int literal_type = ast_type_to_column_type(lit_data->lit_type);
            
            if (!validator_check_type_compatibility(column->type, literal_type)) {
                char error[200];
                snprintf(error, sizeof(error), "Tipo no compatible para columna '%s'. Valor de tipo %d no es compatible con columna de tipo %d", 
                         column->name, literal_type, column->type);
                return validator_set_error(result, 116, error);
            }
            
            // Verificar restricción NOT NULL
            if (lit_data->lit_type == LIT_NULL && !column->allows_null) {
                char error[200];
                snprintf(error, sizeof(error), "No se permite NULL en la columna '%s'", column->name);
                return validator_set_error(result, 117, error);
            }
            
            // Para strings, verificar longitud máxima
            if (lit_data->lit_type == LIT_STRING && column->type == TYPE_STRING) {
                int str_length = strlen(lit_data->string_value);
                if (str_length > column->max_length) {
                    char error[200];
                    snprintf(error, sizeof(error), "El valor excede la longitud máxima para columna '%s'. Longitud: %d, máximo permitido: %d", 
                             column->name, str_length, column->max_length);
                    return validator_set_error(result, 118, error);
                }
            }
        } else {
            // Validar expresiones más complejas
            if (!validator_validate_expression(assign_data->value, table, result)) {
                return 0;
            }
        }
        
        // Avanzar al siguiente nodo de asignación (si hay varios)
        current = ast_get_next_sibling(current);
    }
    
    return 1;
}

// Validar definiciones de columnas para CREATE TABLE
int validator_validate_column_defs(ASTNode* column_defs_node, ValidationResult* result) {
    if (!column_defs_node || !result) return 0;
    
    if (column_defs_node->type != NODE_VALUE_LIST) {
        return validator_set_error(result, 119, "Se esperaba una lista de definiciones de columnas");
    }
    
    ValueListData* columns = (ValueListData*)column_defs_node->data;
    int primary_key_count = 0;
    
    // Verificar cada definición de columna
    for (int i = 0; i < columns->count; i++) {
        ASTNode* col_node = columns->values[i];
        
        if (col_node->type != NODE_COLUMN_DEF) {
            return validator_set_error(result, 120, "Se esperaba una definición de columna");
        }
        
        ColumnDefData* col_def = (ColumnDefData*)col_node->data;
        
        // Verificar que el nombre no esté duplicado
        for (int j = 0; j < i; j++) {
            ASTNode* prev_col = columns->values[j];
            ColumnDefData* prev_def = (ColumnDefData*)prev_col->data;
            
            if (strcasecmp(col_def->name, prev_def->name) == 0) {
                char error[200];
                snprintf(error, sizeof(error), "Nombre de columna duplicado: '%s'", col_def->name);
                return validator_set_error(result, 121, error);
            }
        }
        
        // Verificar tipo de datos válido
        if (col_def->data_type < 0 || col_def->data_type > 3) { // Asumiendo 4 tipos: INT, FLOAT, STRING, BOOL
            char error[200];
            snprintf(error, sizeof(error), "Tipo de datos no válido para columna '%s'", col_def->name);
            return validator_set_error(result, 122, error);
        }
        
        // Para STRING, verificar que max_length sea positivo
        if (col_def->data_type == 2 && col_def->max_length <= 0) { // 2 = TYPE_STRING
            char error[200];
            snprintf(error, sizeof(error), "Longitud inválida para columna STRING '%s': debe ser positiva", 
                     col_def->name);
            return validator_set_error(result, 123, error);
        }
        
        // Contar claves primarias
        if (col_def->is_primary_key) {
            primary_key_count++;
            if (primary_key_count > 1) {
                return validator_set_error(result, 124, "Una tabla solo puede tener una clave primaria");
            }
        }
    }
    
    return 1;
}

// Validar una sentencia SELECT
int validator_validate_select(ASTNode* node, Database* db, ValidationResult* result) {
    if (!node || !db || !result) return 0;
    
    SelectStmtData* data = (SelectStmtData*)node->data;
    
    // Verificar que la tabla existe
    Table* table = validator_find_table(data->table_name, db);
    if (!table) {
        char error[200];
        snprintf(error, sizeof(error), "La tabla '%s' no existe", data->table_name);
        return validator_set_error(result, 201, error);
    }
    
    // Validar columnas
    if (!validator_validate_column_list(data->columns, table, result)) {
        return 0;
    }
    
    // Validar cláusula WHERE si existe
    if (data->where_clause && !validator_validate_where_clause(data->where_clause, table, result)) {
        return 0;
    }
    
    return 1;
}

// Validar una sentencia INSERT
int validator_validate_insert(ASTNode* node, Database* db, ValidationResult* result) {
    if (!node || !db || !result) return 0;
    
    InsertStmtData* data = (InsertStmtData*)node->data;
    
    // Verificar que la tabla existe
    Table* table = validator_find_table(data->table_name, db);
    if (!table) {
        char error[200];
        snprintf(error, sizeof(error), "La tabla '%s' no existe", data->table_name);
        return validator_set_error(result, 202, error);
    }
    
    // Validar valores
    if (!validator_validate_values(data->values, table, result)) {
        return 0;
    }
    
    return 1;
}

// Validar una sentencia UPDATE
int validator_validate_update(ASTNode* node, Database* db, ValidationResult* result) {
    if (!node || !db || !result) return 0;
    
    UpdateStmtData* data = (UpdateStmtData*)node->data;
    
    // Verificar que la tabla existe
    Table* table = validator_find_table(data->table_name, db);
    if (!table) {
        char error[200];
        snprintf(error, sizeof(error), "La tabla '%s' no existe", data->table_name);
        return validator_set_error(result, 203, error);
    }
    
    // Validar asignaciones
    if (!validator_validate_assignments(data->assignments, table, result)) {
        return 0;
    }
    
    // Validar cláusula WHERE si existe
    if (data->where_clause && !validator_validate_where_clause(data->where_clause, table, result)) {
        return 0;
    }
    
    return 1;
}

// Validar una sentencia DELETE
int validator_validate_delete(ASTNode* node, Database* db, ValidationResult* result) {
    if (!node || !db || !result) return 0;
    
    DeleteStmtData* data = (DeleteStmtData*)node->data;
    
    // Verificar que la tabla existe
    Table* table = validator_find_table(data->table_name, db);
    if (!table) {
        char error[200];
        snprintf(error, sizeof(error), "La tabla '%s' no existe", data->table_name);
        return validator_set_error(result, 204, error);
    }
    
    // Validar cláusula WHERE si existe
    if (data->where_clause && !validator_validate_where_clause(data->where_clause, table, result)) {
        return 0;
    }
    
    return 1;
}

// Validar una sentencia CREATE TABLE
int validator_validate_create_table(ASTNode* node, Database* db, ValidationResult* result) {
    if (!node || !db || !result) return 0;
    
    CreateTableStmtData* data = (CreateTableStmtData*)node->data;
    
    // Verificar que la tabla NO existe (debe ser nueva)
    Table* existing_table = validator_find_table(data->table_name, db);
    if (existing_table) {
        char error[200];
        snprintf(error, sizeof(error), "La tabla '%s' ya existe", data->table_name);
        return validator_set_error(result, 205, error);
    }
    
    // Si hay definiciones de columnas, validarlas
    if (data->columns) {
        if (!validator_validate_column_defs(data->columns, result)) {
            return 0;
        }
    } else {
        // En NQL, requerir al menos una columna
        return validator_set_error(result, 206, "Se debe definir al menos una columna");
    }
    
    return 1;
}

// Validar una sentencia ALTER TABLE
int validator_validate_alter_table(ASTNode* node, Database* db, ValidationResult* result) {
    if (!node || !db || !result) return 0;
    
    AlterTableStmtData* data = (AlterTableStmtData*)node->data;
    
    // Verificar que la tabla existe
    Table* table = validator_find_table(data->table_name, db);
    if (!table) {
        char error[200];
        snprintf(error, sizeof(error), "La tabla '%s' no existe", data->table_name);
        return validator_set_error(result, 207, error);
    }
    
    // Verificar la definición de columna
    if (!data->column || data->column->type != NODE_COLUMN_DEF) {
        return validator_set_error(result, 208, "Se esperaba una definición de columna válida");
    }
    
    ColumnDefData* col_def = (ColumnDefData*)data->column->data;
    
    // Verificar que la columna no existe ya
    if (validator_check_column_exists(col_def->name, table)) {
        char error[200];
        snprintf(error, sizeof(error), "La columna '%s' ya existe en la tabla '%s'", 
                 col_def->name, table->name);
        return validator_set_error(result, 209, error);
    }
    
    // Verificar tipo de datos válido
    if (col_def->data_type < 0 || col_def->data_type > 3) { // Asumiendo 4 tipos: INT, FLOAT, STRING, BOOL
        char error[200];
        snprintf(error, sizeof(error), "Tipo de datos no válido para columna '%s'", col_def->name);
        return validator_set_error(result, 210, error);
    }
    
    // Para STRING, verificar que max_length sea positivo
    if (col_def->data_type == 2 && col_def->max_length <= 0) { // 2 = TYPE_STRING
        char error[200];
        snprintf(error, sizeof(error), "Longitud inválida para columna STRING '%s': debe ser positiva", 
                 col_def->name);
        return validator_set_error(result, 211, error);
    }
    
    // Verificar que no se está intentando añadir una clave primaria si ya existe una
    if (col_def->is_primary_key) {
        for (int i = 0; i < table->num_columns; i++) {
            if (table->columns[i].is_primary_key) {
                return validator_set_error(result, 212, "La tabla ya tiene una clave primaria");
            }
        }
    }
    
    return 1;
}

// Validar una sentencia DROP TABLE
int validator_validate_drop_table(ASTNode* node, Database* db, ValidationResult* result) {
    if (!node || !db || !result) return 0;
    
    DropTableStmtData* data = (DropTableStmtData*)node->data;
    
    // Verificar que la tabla existe
    Table* table = validator_find_table(data->table_name, db);
    if (!table) {
        char error[200];
        snprintf(error, sizeof(error), "La tabla '%s' no existe", data->table_name);
        return validator_set_error(result, 213, error);
    }
    
    // En una implementación más completa, podríamos verificar si hay restricciones
    // de clave foránea que impidan eliminar la tabla
    
    return 1;
}

// Validar un AST completo
int validator_validate(ASTNode* node, Database* db, ValidationResult* result) {
    if (!node || !db || !result) return 0;
    
    switch (node->type) {
        case NODE_SELECT_STMT:
            return validator_validate_select(node, db, result);
        case NODE_INSERT_STMT:
            return validator_validate_insert(node, db, result);
        case NODE_UPDATE_STMT:
            return validator_validate_update(node, db, result);
        case NODE_DELETE_STMT:
            return validator_validate_delete(node, db, result);
        case NODE_CREATE_TABLE_STMT:
            return validator_validate_create_table(node, db, result);
        case NODE_ALTER_TABLE_STMT:
            return validator_validate_alter_table(node, db, result);
        case NODE_DROP_TABLE_STMT:
            return validator_validate_drop_table(node, db, result);
        default:
            return validator_set_error(result, 301, "Tipo de nodo no soportado para validación");
    }
}

