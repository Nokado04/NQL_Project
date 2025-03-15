#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

/**
 * Funciones auxiliares privadas para liberar cada tipo específico de datos
 */

static void free_select_stmt(void* data) {
    SelectStmtData* stmt_data = (SelectStmtData*)data;
    if (stmt_data) {
        if (stmt_data->table_name) free(stmt_data->table_name);
        // Los nodos columns y where_clause se liberan en ast_free_node
        free(stmt_data);
    }
}

static void free_insert_stmt(void* data) {
    InsertStmtData* stmt_data = (InsertStmtData*)data;
    if (stmt_data) {
        if (stmt_data->table_name) free(stmt_data->table_name);
        // El nodo values se libera en ast_free_node
        free(stmt_data);
    }
}

static void free_update_stmt(void* data) {
    UpdateStmtData* stmt_data = (UpdateStmtData*)data;
    if (stmt_data) {
        if (stmt_data->table_name) free(stmt_data->table_name);
        // Los nodos assignments y where_clause se liberan en ast_free_node
        free(stmt_data);
    }
}

static void free_delete_stmt(void* data) {
    DeleteStmtData* stmt_data = (DeleteStmtData*)data;
    if (stmt_data) {
        if (stmt_data->table_name) free(stmt_data->table_name);
        // El nodo where_clause se libera en ast_free_node
        free(stmt_data);
    }
}

static void free_create_table_stmt(void* data) {
    CreateTableStmtData* stmt_data = (CreateTableStmtData*)data;
    if (stmt_data) {
        if (stmt_data->table_name) free(stmt_data->table_name);
        // El nodo columns se libera en ast_free_node
        free(stmt_data);
    }
}

static void free_alter_table_stmt(void* data) {
    AlterTableStmtData* stmt_data = (AlterTableStmtData*)data;
    if (stmt_data) {
        if (stmt_data->table_name) free(stmt_data->table_name);
        // El nodo column se libera en ast_free_node
        free(stmt_data);
    }
}

static void free_drop_table_stmt(void* data) {
    DropTableStmtData* stmt_data = (DropTableStmtData*)data;
    if (stmt_data) {
        if (stmt_data->table_name) free(stmt_data->table_name);
        free(stmt_data);
    }
}

static void free_column_def(void* data) {
    ColumnDefData* col_data = (ColumnDefData*)data;
    if (col_data) {
        if (col_data->name) free(col_data->name);
        free(col_data);
    }
}

static void free_column_list(void* data) {
    ColumnListData* list_data = (ColumnListData*)data;
    if (list_data) {
        if (list_data->columns) {
            for (int i = 0; i < list_data->count; i++) {
                if (list_data->columns[i]) free(list_data->columns[i]);
            }
            free(list_data->columns);
        }
        free(list_data);
    }
}

static void free_value_list(void* data) {
    ValueListData* list_data = (ValueListData*)data;
    if (list_data) {
        if (list_data->values) {
            // No liberamos los nodos aquí, eso se hace en ast_free_node
            free(list_data->values);
        }
        free(list_data);
    }
}

static void free_where_clause(void* data) {
    WhereClauseData* where_data = (WhereClauseData*)data;
    if (where_data) {
        // El nodo condition se libera en ast_free_node
        free(where_data);
    }
}

static void free_assignment(void* data) {
    AssignmentData* assign_data = (AssignmentData*)data;
    if (assign_data) {
        if (assign_data->column_name) free(assign_data->column_name);
        // El nodo value se libera en ast_free_node
        free(assign_data);
    }
}

static void free_binary_expr(void* data) {
    BinaryExprData* expr_data = (BinaryExprData*)data;
    if (expr_data) {
        // Los nodos left y right se liberan en ast_free_node
        free(expr_data);
    }
}

static void free_unary_expr(void* data) {
    UnaryExprData* expr_data = (UnaryExprData*)data;
    if (expr_data) {
        // El nodo operand se libera en ast_free_node
        free(expr_data);
    }
}

static void free_identifier(void* data) {
    IdentifierData* id_data = (IdentifierData*)data;
    if (id_data) {
        if (id_data->name) free(id_data->name);
        free(id_data);
    }
}

static void free_literal(void* data) {
    LiteralData* lit_data = (LiteralData*)data;
    if (lit_data) {
        if (lit_data->lit_type == LIT_STRING && lit_data->string_value) {
            free(lit_data->string_value);
        }
        free(lit_data);
    }
}

/**
 * Funciones de creación de nodos
 */

ASTNode* ast_create_node(ASTNodeType type) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (!node) return NULL;
    
    node->type = type;
    node->data = NULL;
    node->free_data = NULL;
    node->parent = NULL;
    node->next = NULL;
    node->prev = NULL;
    
    return node;
}

ASTNode* ast_create_select(char* table_name, ASTNode* columns, ASTNode* where) {
    ASTNode* node = ast_create_node(NODE_SELECT_STMT);
    if (!node) return NULL;
    
    SelectStmtData* data = (SelectStmtData*)malloc(sizeof(SelectStmtData));
    if (!data) {
        free(node);
        return NULL;
    }
    
    data->table_name = table_name ? strdup(table_name) : NULL;
    data->columns = columns;
    data->where_clause = where;
    
    node->data = data;
    node->free_data = free_select_stmt;
    
    // Establecer relaciones padre-hijo
    if (columns) columns->parent = node;
    if (where) where->parent = node;
    
    return node;
}

ASTNode* ast_create_insert(char* table_name, ASTNode* values) {
    ASTNode* node = ast_create_node(NODE_INSERT_STMT);
    if (!node) return NULL;
    
    InsertStmtData* data = (InsertStmtData*)malloc(sizeof(InsertStmtData));
    if (!data) {
        free(node);
        return NULL;
    }
    
    data->table_name = table_name ? strdup(table_name) : NULL;
    data->values = values;
    
    node->data = data;
    node->free_data = free_insert_stmt;
    
    // Establecer relaciones padre-hijo
    if (values) values->parent = node;
    
    return node;
}

ASTNode* ast_create_update(char* table_name, ASTNode* assignments, ASTNode* where) {
    ASTNode* node = ast_create_node(NODE_UPDATE_STMT);
    if (!node) return NULL;
    
    UpdateStmtData* data = (UpdateStmtData*)malloc(sizeof(UpdateStmtData));
    if (!data) {
        free(node);
        return NULL;
    }
    
    data->table_name = table_name ? strdup(table_name) : NULL;
    data->assignments = assignments;
    data->where_clause = where;
    
    node->data = data;
    node->free_data = free_update_stmt;
    
    // Establecer relaciones padre-hijo
    if (assignments) assignments->parent = node;
    if (where) where->parent = node;
    
    return node;
}

ASTNode* ast_create_delete(char* table_name, ASTNode* where) {
    ASTNode* node = ast_create_node(NODE_DELETE_STMT);
    if (!node) return NULL;
    
    DeleteStmtData* data = (DeleteStmtData*)malloc(sizeof(DeleteStmtData));
    if (!data) {
        free(node);
        return NULL;
    }
    
    data->table_name = table_name ? strdup(table_name) : NULL;
    data->where_clause = where;
    
    node->data = data;
    node->free_data = free_delete_stmt;
    
    // Establecer relaciones padre-hijo
    if (where) where->parent = node;
    
    return node;
}

ASTNode* ast_create_create_table(char* table_name, ASTNode* columns) {
    ASTNode* node = ast_create_node(NODE_CREATE_TABLE_STMT);
    if (!node) return NULL;
    
    CreateTableStmtData* data = (CreateTableStmtData*)malloc(sizeof(CreateTableStmtData));
    if (!data) {
        free(node);
        return NULL;
    }
    
    data->table_name = table_name ? strdup(table_name) : NULL;
    data->columns = columns;
    
    node->data = data;
    node->free_data = free_create_table_stmt;
    
    // Establecer relaciones padre-hijo
    if (columns) columns->parent = node;
    
    return node;
}

ASTNode* ast_create_alter_table(char* table_name, ASTNode* column) {
    ASTNode* node = ast_create_node(NODE_ALTER_TABLE_STMT);
    if (!node) return NULL;
    
    AlterTableStmtData* data = (AlterTableStmtData*)malloc(sizeof(AlterTableStmtData));
    if (!data) {
        free(node);
        return NULL;
    }
    
    data->table_name = table_name ? strdup(table_name) : NULL;
    data->column = column;
    
    node->data = data;
    node->free_data = free_alter_table_stmt;
    
    // Establecer relaciones padre-hijo
    if (column) column->parent = node;
    
    return node;
}

ASTNode* ast_create_drop_table(char* table_name) {
    ASTNode* node = ast_create_node(NODE_DROP_TABLE_STMT);
    if (!node) return NULL;
    
    DropTableStmtData* data = (DropTableStmtData*)malloc(sizeof(DropTableStmtData));
    if (!data) {
        free(node);
        return NULL;
    }
    
    data->table_name = table_name ? strdup(table_name) : NULL;
    
    node->data = data;
    node->free_data = free_drop_table_stmt;
    
    return node;
}

ASTNode* ast_create_column_def(char* name, int data_type, int max_length, int is_primary_key, int allows_null) {
    ASTNode* node = ast_create_node(NODE_COLUMN_DEF);
    if (!node) return NULL;
    
    ColumnDefData* data = (ColumnDefData*)malloc(sizeof(ColumnDefData));
    if (!data) {
        free(node);
        return NULL;
    }
    
    data->name = name ? strdup(name) : NULL;
    data->data_type = data_type;
    data->max_length = max_length;
    data->is_primary_key = is_primary_key;
    data->allows_null = allows_null;
    
    node->data = data;
    node->free_data = free_column_def;
    
    return node;
}

ASTNode* ast_create_column_list(int is_all, char** columns, int count) {
    ASTNode* node = ast_create_node(NODE_COLUMN_LIST);
    if (!node) return NULL;
    
    ColumnListData* data = (ColumnListData*)malloc(sizeof(ColumnListData));
    if (!data) {
        free(node);
        return NULL;
    }
    
    data->is_all = is_all;
    data->count = count;
    
    if (count > 0 && columns) {
        data->columns = (char**)malloc(count * sizeof(char*));
        if (!data->columns) {
            free(data);
            free(node);
            return NULL;
        }
        
        for (int i = 0; i < count; i++) {
            data->columns[i] = columns[i] ? strdup(columns[i]) : NULL;
        }
    } else {
        data->columns = NULL;
    }
    
    node->data = data;
    node->free_data = free_column_list;
    
    return node;
}

ASTNode* ast_create_value_list(ASTNode** values, int count) {
    ASTNode* node = ast_create_node(NODE_VALUE_LIST);
    if (!node) return NULL;
    
    ValueListData* data = (ValueListData*)malloc(sizeof(ValueListData));
    if (!data) {
        free(node);
        return NULL;
    }
    
    data->count = count;
    
    if (count > 0 && values) {
        data->values = (ASTNode**)malloc(count * sizeof(ASTNode*));
        if (!data->values) {
            free(data);
            free(node);
            return NULL;
        }
        
        for (int i = 0; i < count; i++) {
            data->values[i] = values[i];
            if (values[i]) values[i]->parent = node;
        }
    } else {
        data->values = NULL;
    }
    
    node->data = data;
    node->free_data = free_value_list;
    
    return node;
}

ASTNode* ast_create_where_clause(ASTNode* condition) {
    ASTNode* node = ast_create_node(NODE_WHERE_CLAUSE);
    if (!node) return NULL;
    
    WhereClauseData* data = (WhereClauseData*)malloc(sizeof(WhereClauseData));
    if (!data) {
        free(node);
        return NULL;
    }
    
    data->condition = condition;
    
    node->data = data;
    node->free_data = free_where_clause;
    
    // Establecer relaciones padre-hijo
    if (condition) condition->parent = node;
    
    return node;
}

ASTNode* ast_create_assignment(char* column_name, ASTNode* value) {
    ASTNode* node = ast_create_node(NODE_ASSIGNMENT);
    if (!node) return NULL;
    
    AssignmentData* data = (AssignmentData*)malloc(sizeof(AssignmentData));
    if (!data) {
        free(node);
        return NULL;
    }
    
    data->column_name = column_name ? strdup(column_name) : NULL;
    data->value = value;
    
    node->data = data;
    node->free_data = free_assignment;
    
    // Establecer relaciones padre-hijo
    if (value) value->parent = node;
    
    return node;
}

ASTNode* ast_create_binary_expr(BinaryOpType op, ASTNode* left, ASTNode* right) {
    ASTNode* node = ast_create_node(NODE_BINARY_EXPR);
    if (!node) return NULL;
    
    BinaryExprData* data = (BinaryExprData*)malloc(sizeof(BinaryExprData));
    if (!data) {
        free(node);
        return NULL;
    }
    
    data->op_type = op;
    data->left = left;
    data->right = right;
    
    node->data = data;
    node->free_data = free_binary_expr;
    
    // Establecer relaciones padre-hijo
    if (left) left->parent = node;
    if (right) right->parent = node;
    
    return node;
}

ASTNode* ast_create_unary_expr(UnaryOpType op, ASTNode* operand) {
    ASTNode* node = ast_create_node(NODE_UNARY_EXPR);
    if (!node) return NULL;
    
    UnaryExprData* data = (UnaryExprData*)malloc(sizeof(UnaryExprData));
    if (!data) {
        free(node);
        return NULL;
    }
    
    data->op_type = op;
    data->operand = operand;
    
    node->data = data;
    node->free_data = free_unary_expr;
    
    // Establecer relaciones padre-hijo
    if (operand) operand->parent = node;
    
    return node;
}

ASTNode* ast_create_identifier(char* name) {
    ASTNode* node = ast_create_node(NODE_IDENTIFIER);
    if (!node) return NULL;
    
    IdentifierData* data = (IdentifierData*)malloc(sizeof(IdentifierData));
    if (!data) {
        free(node);
        return NULL;
    }
    
    data->name = name ? strdup(name) : NULL;
    
    node->data = data;
    node->free_data = free_identifier;
    
    return node;
}

ASTNode* ast_create_literal_int(int value) {
    ASTNode* node = ast_create_node(NODE_LITERAL);
    if (!node) return NULL;
    
    LiteralData* data = (LiteralData*)malloc(sizeof(LiteralData));
    if (!data) {
        free(node);
        return NULL;
    }
    
    data->lit_type = LIT_INTEGER;
    data->int_value = value;
    
    node->data = data;
    node->free_data = free_literal;
    
    return node;
}

ASTNode* ast_create_literal_float(double value) {
    ASTNode* node = ast_create_node(NODE_LITERAL);
    if (!node) return NULL;
    
    LiteralData* data = (LiteralData*)malloc(sizeof(LiteralData));
    if (!data) {
        free(node);
        return NULL;
    }
    
    data->lit_type = LIT_FLOAT;
    data->float_value = value;
    
    node->data = data;
    node->free_data = free_literal;
    
    return node;
}

ASTNode* ast_create_literal_string(char* value) {
    ASTNode* node = ast_create_node(NODE_LITERAL);
    if (!node) return NULL;
    
    LiteralData* data = (LiteralData*)malloc(sizeof(LiteralData));
    if (!data) {
        free(node);
        return NULL;
    }
    
    data->lit_type = LIT_STRING;
    data->string_value = value ? strdup(value) : NULL;
    
    node->data = data;
    node->free_data = free_literal;
    
    return node;
}

ASTNode* ast_create_literal_bool(int value) {
    ASTNode* node = ast_create_node(NODE_LITERAL);
    if (!node) return NULL;
    
    LiteralData* data = (LiteralData*)malloc(sizeof(LiteralData));
    if (!data) {
        free(node);
        return NULL;
    }
    
    data->lit_type = LIT_BOOLEAN;
    data->bool_value = value;
    
    node->data = data;
    node->free_data = free_literal;
    
    return node;
}

ASTNode* ast_create_literal_null() {
    ASTNode* node = ast_create_node(NODE_LITERAL);
    if (!node) return NULL;
    
    LiteralData* data = (LiteralData*)malloc(sizeof(LiteralData));
    if (!data) {
        free(node);
        return NULL;
    }
    
    data->lit_type = LIT_NULL;
    
    node->data = data;
    node->free_data = free_literal;
    
    return node;
}

/**
 * Funciones para manipulación de AST
 */

void ast_append_sibling(ASTNode* node, ASTNode* sibling) {
    if (!node || !sibling) return;
    
    // Si el nodo ya tiene hermanos, buscar el último
    ASTNode* last = node;
    while (last->next) {
        last = last->next;
    }
    
    // Enlazar
    last->next = sibling;
    sibling->prev = last;
}

// Obtiene el siguiente nodo hermano en una lista de nodos
ASTNode* ast_get_next_sibling(ASTNode* node) {
    if (!node) return NULL;
    return node->next;
}

void ast_free_node(ASTNode* node) {
    if (!node) return;
    
    // Primero liberar recursivamente todos los hijos según el tipo de nodo
    switch (node->type) {
        case NODE_SELECT_STMT:
            if (node->data) {
                SelectStmtData* data = (SelectStmtData*)node->data;
                if (data->columns) ast_free_node(data->columns);
                if (data->where_clause) ast_free_node(data->where_clause);
            }
            break;
            
        case NODE_INSERT_STMT:
            if (node->data) {
                InsertStmtData* data = (InsertStmtData*)node->data;
                if (data->values) ast_free_node(data->values);
            }
            break;
            
        case NODE_UPDATE_STMT:
            if (node->data) {
                UpdateStmtData* data = (UpdateStmtData*)node->data;
                
                // Liberar lista de asignaciones (que pueden ser varios nodos enlazados)
                ASTNode* current = data->assignments;
                while (current) {
                    ASTNode* next = current->next;
                    current->next = NULL;  // Desconectar para evitar recursión infinita
                    ast_free_node(current);
                    current = next;
                }
                
                if (data->where_clause) ast_free_node(data->where_clause);
            }
            break;
            
        case NODE_DELETE_STMT:
            if (node->data) {
                DeleteStmtData* data = (DeleteStmtData*)node->data;
                if (data->where_clause) ast_free_node(data->where_clause);
            }
            break;
            
        case NODE_CREATE_TABLE_STMT:
            if (node->data) {
                CreateTableStmtData* data = (CreateTableStmtData*)node->data;
                
                // Liberar lista de definiciones de columna
                if (data->columns) ast_free_node(data->columns);
            }
            break;
            
        case NODE_ALTER_TABLE_STMT:
            if (node->data) {
                AlterTableStmtData* data = (AlterTableStmtData*)node->data;
                if (data->column) ast_free_node(data->column);
            }
            break;
            
        case NODE_VALUE_LIST:
            if (node->data) {
                ValueListData* data = (ValueListData*)node->data;
                for (int i = 0; i < data->count; i++) {
                    if (data->values[i]) ast_free_node(data->values[i]);
                }
            }
            break;
            
        case NODE_WHERE_CLAUSE:
            if (node->data) {
                WhereClauseData* data = (WhereClauseData*)node->data;
                if (data->condition) ast_free_node(data->condition);
            }
            break;
            
        case NODE_ASSIGNMENT:
            if (node->data) {
                AssignmentData* data = (AssignmentData*)node->data;
                if (data->value) ast_free_node(data->value);
            }
            break;
            
        case NODE_BINARY_EXPR:
            if (node->data) {
                BinaryExprData* data = (BinaryExprData*)node->data;
                if (data->left) ast_free_node(data->left);
                if (data->right) ast_free_node(data->right);
            }
            break;
            
        case NODE_UNARY_EXPR:
            if (node->data) {
                UnaryExprData* data = (UnaryExprData*)node->data;
                if (data->operand) ast_free_node(data->operand);
            }
            break;
            
        default:
            // Los nodos hoja (NODE_IDENTIFIER, NODE_LITERAL, etc.) no tienen hijos que liberar
            break;
    }
    
    // Liberar los datos específicos del nodo usando la función asociada
    if (node->free_data && node->data) {
        node->free_data(node->data);
    }
    
    // Liberar el nodo siguiente (hermano)
    if (node->next) {
        ast_free_node(node->next);
    }
    
    // Finalmente liberar el nodo mismo
    free(node);
}

// Función para imprimir indentación
void ast_print_indent(int level) {
    for (int i = 0; i < level; i++) {
        printf("  ");
    }
}

// Función para imprimir un AST (para depuración)
void ast_print(ASTNode* node, int level) {
    if (!node) {
        ast_print_indent(level);
        printf("(null)\n");
        return;
    }
    
    ast_print_indent(level);
    
    // Imprimir tipo de nodo
    switch (node->type) {
        case NODE_SELECT_STMT: {
            SelectStmtData* data = (SelectStmtData*)node->data;
            printf("SELECT (from: %s)\n", data->table_name ? data->table_name : "NULL");
            
            if (data->columns) {
                ast_print_indent(level+1);
                printf("COLUMNS:\n");
                ast_print(data->columns, level+2);
            }
            
            if (data->where_clause) {
                ast_print_indent(level+1);
                printf("WHERE:\n");
                ast_print(data->where_clause, level+2);
            }
            break;
        }
        
        case NODE_INSERT_STMT: {
            InsertStmtData* data = (InsertStmtData*)node->data;
            printf("INSERT (into: %s)\n", data->table_name ? data->table_name : "NULL");
            
            if (data->values) {
                ast_print_indent(level+1);
                printf("VALUES:\n");
                ast_print(data->values, level+2);
            }
            break;
        }
        
        case NODE_UPDATE_STMT: {
            UpdateStmtData* data = (UpdateStmtData*)node->data;
            printf("UPDATE (table: %s)\n", data->table_name ? data->table_name : "NULL");
            
            if (data->assignments) {
                ast_print_indent(level+1);
                printf("SET:\n");
                
                // Aquí necesitamos recorrer todos los nodos de asignación hermanos
                ASTNode* current = data->assignments;
                while (current) {
                    ast_print(current, level+2);
                    current = current->next;  // Asegurar que estamos siguiendo los hermanos
                }
            }
            
            if (data->where_clause) {
                ast_print_indent(level+1);
                printf("WHERE:\n");
                ast_print(data->where_clause, level+2);
            }
            break;
        }
        
        case NODE_DELETE_STMT: {
            DeleteStmtData* data = (DeleteStmtData*)node->data;
            printf("DELETE (from: %s)\n", data->table_name ? data->table_name : "NULL");
            
            if (data->where_clause) {
                ast_print_indent(level+1);
                printf("WHERE:\n");
                ast_print(data->where_clause, level+2);
            }
            break;
        }
        
        case NODE_CREATE_TABLE_STMT: {
            CreateTableStmtData* data = (CreateTableStmtData*)node->data;
            printf("CREATE TABLE (name: %s)\n", data->table_name ? data->table_name : "NULL");
            
            if (data->columns) {
                ast_print_indent(level+1);
                printf("COLUMNS:\n");
                ast_print(data->columns, level+2);
            }
            break;
        }
        
        case NODE_ALTER_TABLE_STMT: {
            AlterTableStmtData* data = (AlterTableStmtData*)node->data;
            printf("ALTER TABLE (name: %s)\n", data->table_name ? data->table_name : "NULL");
            
            if (data->column) {
                ast_print_indent(level+1);
                printf("ADD COLUMN:\n");
                ast_print(data->column, level+2);
            }
            break;
        }
        
        case NODE_DROP_TABLE_STMT: {
            DropTableStmtData* data = (DropTableStmtData*)node->data;
            printf("DROP TABLE (name: %s)\n", data->table_name ? data->table_name : "NULL");
            break;
        }
        
        case NODE_COLUMN_DEF: {
            ColumnDefData* data = (ColumnDefData*)node->data;
            printf("COLUMN DEF: %s (tipo: %d, ", 
                   data->name ? data->name : "NULL", data->data_type);
            printf("PK: %d, NULL: %d)\n", data->is_primary_key, data->allows_null);
            break;
        }

        case NODE_COLUMN_LIST: {
            ColumnListData* data = (ColumnListData*)node->data;
            printf("COLUMN LIST (%s):\n", data->is_all ? "*" : "columnas");
            
            for (int i = 0; i < data->count; i++) {
                ast_print_indent(level+1);
                printf("%s\n", data->columns[i] ? data->columns[i] : "NULL");
            }
            break;
        }

        case NODE_VALUE_LIST: {
            ValueListData* data = (ValueListData*)node->data;
            printf("VALUE LIST:\n");
            
            for (int i = 0; i < data->count; i++) {
                ast_print(data->values[i], level+1);
            }
            break;
        }

        case NODE_WHERE_CLAUSE: {
            WhereClauseData* data = (WhereClauseData*)node->data;
            printf("WHERE CLAUSE:\n");
            ast_print(data->condition, level+1);
            break;
        }

        case NODE_ASSIGNMENT: {
            AssignmentData* data = (AssignmentData*)node->data;
            printf("ASSIGNMENT: %s\n", data->column_name ? data->column_name : "NULL");
            ast_print(data->value, level+1);
            break;
        }

        case NODE_BINARY_EXPR: {
            BinaryExprData* data = (BinaryExprData*)node->data;
            // Mostrar el tipo de operador como texto, no como número
            const char* op_str;
            switch (data->op_type) {
                case OP_EQ: op_str = "="; break;
                case OP_NEQ: op_str = "<>"; break;
                case OP_LT: op_str = "<"; break;
                case OP_GT: op_str = ">"; break;
                case OP_LTE: op_str = "<="; break;
                case OP_GTE: op_str = ">="; break;
                case OP_AND: op_str = "AND"; break;
                case OP_OR: op_str = "OR"; break;
                case OP_PLUS: op_str = "+"; break;
                case OP_MINUS: op_str = "-"; break;
                case OP_MULTIPLY: op_str = "*"; break;
                case OP_DIVIDE: op_str = "/"; break;
                default: op_str = "???";
            }
            
            printf("BINARY EXPR: %s\n", op_str);
            ast_print(data->left, level+1);
            ast_print(data->right, level+1);
            break;
        }

        case NODE_UNARY_EXPR: {
            UnaryExprData* data = (UnaryExprData*)node->data;
            printf("UNARY EXPR: %d\n", data->op_type);
            ast_print(data->operand, level+1);
            break;
        }

        case NODE_IDENTIFIER: {
            IdentifierData* data = (IdentifierData*)node->data;
            printf("IDENTIFIER: %s\n", data->name ? data->name : "NULL");
            break;
        }

        case NODE_LITERAL: {
            LiteralData* data = (LiteralData*)node->data;
            switch (data->lit_type) {
                case LIT_INTEGER:
                    printf("LITERAL INT: %d\n", data->int_value);
                    break;
                case LIT_FLOAT:
                    printf("LITERAL FLOAT: %f\n", data->float_value);
                    break;
                case LIT_STRING:
                    printf("LITERAL STRING: %s\n", data->string_value ? data->string_value : "NULL");
                    break;
                case LIT_BOOLEAN:
                    printf("LITERAL BOOL: %s\n", data->bool_value ? "true" : "false");
                    break;
                case LIT_NULL:
                    printf("LITERAL NULL\n");
                    break;
            }
            break;
        }

        default:
            printf("TIPO DESCONOCIDO\n");
            break;

    }
}

// Funciones para AST completo
ParsedAST* ast_create() {
    ParsedAST* ast = (ParsedAST*)malloc(sizeof(ParsedAST));
    if (!ast) return NULL;
    
    ast->root = NULL;
    
    return ast;
}
void ast_free(ParsedAST* ast) {
    if (!ast) return;
    
    if (ast->root) {
        ast_free_node(ast->root);
    }
    
    free(ast);
}

void ast_set_column_name(ASTNode* node, const char* column_name) {
    if (!node || node->type != NODE_ALTER_TABLE_STMT || !column_name) return;
    
    AlterTableStmtData* data = (AlterTableStmtData*)node->data;
    
    // Si ya existe una columna, construir una temporal
    if (!data->column) {
        // Creamos una definición de columna simple
        data->column = ast_create_column_def(
            strdup(column_name),  // nombre 
            0,                    // tipo (por defecto)
            0,                    // longitud máxima
            0,                    // no es clave primaria
            1                     // permite NULL
        );
    } else if (data->column->type == NODE_COLUMN_DEF) {
        // Si ya existe una definición de columna, actualizar el nombre
        ColumnDefData* col_data = (ColumnDefData*)data->column->data;
        if (col_data->name) free(col_data->name);
        col_data->name = strdup(column_name);
    }
}



