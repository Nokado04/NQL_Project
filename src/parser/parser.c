#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"

// Funciones de utilidad para el parser

// Función para establecer un error de parseo
static void parser_set_error(Parser* parser, const char* message) {
    if (parser->error_message) {
        free(parser->error_message);
    }
    parser->error_message = strdup(message);
    parser->error_position = parser->current_token.position;
}

// Función para consumir un token y avanzar al siguiente
static void parser_consume(Parser* parser) {
    // Liberar el token actual si existe
    token_free(&parser->current_token);
    
    // Obtener el siguiente token
    parser->current_token = lexer_next_token(parser->lexer);
}

// Función para verificar si el token actual es del tipo esperado
static int parser_check(Parser* parser, TokenType type) {
    return parser->current_token.type == type;
}

// Función para verificar si el token actual es una palabra clave específica
static int parser_check_keyword(Parser* parser, const char* keyword) {
    return parser->current_token.type == TOKEN_KEYWORD &&
           strcasecmp(parser->current_token.value, keyword) == 0;
}

// Función para consumir un token si es del tipo esperado, o generar un error
static int parser_match(Parser* parser, TokenType type) {
    if (parser_check(parser, type)) {
        parser_consume(parser);
        return 1;
    }
    
    char error[100];
    snprintf(error, sizeof(error), "Se esperaba token de tipo %s", token_type_to_string(type));
    parser_set_error(parser, error);
    return 0;
}

// Función para consumir una palabra clave específica, o generar un error
static int parser_match_keyword(Parser* parser, const char* keyword) {
    if (parser_check_keyword(parser, keyword)) {
        parser_consume(parser);
        return 1;
    }
    
    char error[100];
    snprintf(error, sizeof(error), "Se esperaba la palabra clave '%s'", keyword);
    parser_set_error(parser, error);
    return 0;
}

// Crear un nuevo parser
Parser* parser_create(const char* sql) {
    Parser* parser = (Parser*) malloc(sizeof(Parser));
    if (!parser) return NULL;
    
    parser->lexer = lexer_create(sql);
    if (!parser->lexer) {
        free(parser);
        return NULL;
    }
    
    parser->error_message = NULL;
    parser->error_position = -1;
    
    // Inicializar con el primer token
    parser->current_token = lexer_next_token(parser->lexer);
    
    return parser;
}

// Verificar si hay error en el parser
int parser_has_error(Parser* parser) {
    return parser->error_message != NULL;
}

// Obtener mensaje de error
const char* parser_get_error(Parser* parser) {
    return parser->error_message;
}

// Parsear un literal
static ASTNode* parser_parse_literal(Parser* parser) {
    Token token = parser->current_token; // Guardar referencia al token actual
    
    if (token.type == TOKEN_INTEGER) {
        int value = atoi(token.value);
        parser_consume(parser); // Ya no obtenemos un valor de retorno
        return ast_create_literal_int(value);
    }
    else if (token.type == TOKEN_FLOAT) {
        char* value_copy = strdup(token.value);
        parser_consume(parser);
        double value = atof(value_copy);
        free(value_copy);
        return ast_create_literal_float(value);
    }
    else if (token.type == TOKEN_STRING) {
        char* value_copy = strdup(token.value);
        parser_consume(parser);
        ASTNode* result = ast_create_literal_string(value_copy);
        free(value_copy); // ast_create_literal_string ya hace una copia
        return result;
    }
    else if (parser_check_keyword(parser, "TRUE")) {
        parser_consume(parser);
        return ast_create_literal_bool(1);
    }
    else if (parser_check_keyword(parser, "FALSE")) {
        parser_consume(parser);
        return ast_create_literal_bool(0);
    }
    else if (parser_check_keyword(parser, "NULL")) {
        parser_consume(parser);
        return ast_create_literal_null();
    }
    
    parser_set_error(parser, "Se esperaba un literal");
    return NULL;
}

// Parsear un identificador
static ASTNode* parser_parse_identifier(Parser* parser) {
    if (parser->current_token.type == TOKEN_IDENTIFIER) {
        // Copiar el valor antes de consumir el token
        char* name_copy = strdup(parser->current_token.value);
        parser_consume(parser);
        
        ASTNode* result = ast_create_identifier(name_copy);
        free(name_copy); // Liberar la copia después de usarla
        return result;
    }
    
    parser_set_error(parser, "Se esperaba un identificador");
    return NULL;
}

// Precedencia de operadores binarios
static int get_binary_precedence(const char* op) {
    if (strcmp(op, "*") == 0 || strcmp(op, "/") == 0)
        return 5;
    if (strcmp(op, "+") == 0 || strcmp(op, "-") == 0)
        return 4;
    if (strcmp(op, "=") == 0 || strcmp(op, "<>") == 0 ||
        strcmp(op, "<") == 0 || strcmp(op, ">") == 0 ||
        strcmp(op, "<=") == 0 || strcmp(op, ">=") == 0)
        return 3;
    if (strcasecmp(op, "AND") == 0)
        return 2;
    if (strcasecmp(op, "OR") == 0)
        return 1;
    return 0;
}

// Convertir string de operador a tipo de operador
static BinaryOpType get_binary_op_type(const char* op) {
    if (strcmp(op, "+") == 0) return OP_PLUS;
    if (strcmp(op, "-") == 0) return OP_MINUS;
    if (strcmp(op, "*") == 0) return OP_MULTIPLY;
    if (strcmp(op, "/") == 0) return OP_DIVIDE;
    if (strcmp(op, "=") == 0) return OP_EQ;
    if (strcmp(op, "<>") == 0) return OP_NEQ;
    if (strcmp(op, "<") == 0) return OP_LT;
    if (strcmp(op, ">") == 0) return OP_GT;
    if (strcmp(op, "<=") == 0) return OP_LTE;
    if (strcmp(op, ">=") == 0) return OP_GTE;
    if (strcasecmp(op, "AND") == 0) return OP_AND;
    if (strcasecmp(op, "OR") == 0) return OP_OR;
    return -1; // Error
}

// Declaración anticipada para recursividad
ASTNode* parser_parse_expression(Parser* parser);
ASTNode* parser_parse_expression_prec(Parser* parser, int precedence);

// Parsear operador unario
static UnaryOpType get_unary_op_type(const char* op) {
    if (strcmp(op, "-") == 0) return OP_NEG;
    if (strcasecmp(op, "NOT") == 0) return OP_NOT;
    return -1; // Error
}

// Parsear una expresión primaria (parte básica de una expresión)
static ASTNode* parser_parse_primary(Parser* parser) {
    // Manejo de paréntesis
    if (parser->current_token.type == TOKEN_PUNCTUATION && 
        strcmp(parser->current_token.value, "(") == 0) {
        parser_consume(parser);
        ASTNode* expr = parser_parse_expression(parser);
        
        if (!expr) return NULL;
        
        if (parser->current_token.type != TOKEN_PUNCTUATION || 
            strcmp(parser->current_token.value, ")") != 0) {
            parser_set_error(parser, "Se esperaba un paréntesis de cierre ')'");
            ast_free_node(expr);
            return NULL;
        }
        
        parser_consume(parser); // Consumir el paréntesis de cierre
        return expr;
    }
    
    // Operadores unarios
    if ((parser->current_token.type == TOKEN_OPERATOR && 
         strcmp(parser->current_token.value, "-") == 0) ||
        (parser->current_token.type == TOKEN_KEYWORD && 
         strcasecmp(parser->current_token.value, "NOT") == 0)) {
        
        char* op = strdup(parser->current_token.value);
        UnaryOpType type = get_unary_op_type(op);
        free(op);
        
        parser_consume(parser);
        ASTNode* operand = parser_parse_primary(parser);
        
        if (!operand) return NULL;
        
        return ast_create_unary_expr(type, operand);
    }
    
    // Identificador
    if (parser->current_token.type == TOKEN_IDENTIFIER) {
        return parser_parse_identifier(parser);
    }
    
    // Literal
    if (parser->current_token.type == TOKEN_INTEGER || 
        parser->current_token.type == TOKEN_FLOAT || 
        parser->current_token.type == TOKEN_STRING ||
        (parser->current_token.type == TOKEN_KEYWORD && 
         (strcasecmp(parser->current_token.value, "TRUE") == 0 || 
          strcasecmp(parser->current_token.value, "FALSE") == 0 || 
          strcasecmp(parser->current_token.value, "NULL") == 0))) {
        
        return parser_parse_literal(parser);
    }
    
    parser_set_error(parser, "Se esperaba una expresión primaria (literal, identificador o expresión entre paréntesis)");
    return NULL;
}

// Parsear una expresión con una precedencia mínima
ASTNode* parser_parse_expression_prec(Parser* parser, int precedence) {
    // Parsear el lado izquierdo de la expresión
    ASTNode* left = parser_parse_primary(parser);
    
    if (!left) return NULL;
    
    // Mientras haya un operador con precedencia suficiente
    while (parser->current_token.type == TOKEN_OPERATOR || 
          (parser->current_token.type == TOKEN_KEYWORD && 
           (strcasecmp(parser->current_token.value, "AND") == 0 || 
            strcasecmp(parser->current_token.value, "OR") == 0))) {
        
        char* op = strdup(parser->current_token.value);
        int op_prec = get_binary_precedence(op);
        
        // Si la precedencia no es suficiente, salimos del bucle
        if (op_prec < precedence) {
            free(op);
            break;
        }
        
        parser_consume(parser);
        
        // Parsear el lado derecho con mayor precedencia
        ASTNode* right = parser_parse_expression_prec(parser, op_prec + 1);
        
        if (!right) {
            free(op);
            ast_free_node(left);
            return NULL;
        }
        
        // Crear nodo para la expresión binaria
        BinaryOpType op_type = get_binary_op_type(op);
        free(op);
        
        ASTNode* binary = ast_create_binary_expr(op_type, left, right);
        
        if (!binary) {
            ast_free_node(left);
            ast_free_node(right);
            return NULL;
        }
        
        left = binary;
    }
    
    return left;
}

// Parsear una expresión completa
ASTNode* parser_parse_expression(Parser* parser) {
    return parser_parse_expression_prec(parser, 0);
}

// Parsear una lista de valores
ASTNode* parser_parse_value_list(Parser* parser) {
    // Lista de valores - eliminamos la verificación de paréntesis
    ASTNode** values = NULL;
    int count = 0;
    int capacity = 8;
    
    values = (ASTNode**)malloc(sizeof(ASTNode*) * capacity);
    if (!values) {
        parser_set_error(parser, "Error de memoria al crear lista de valores");
        return NULL;
    }
    
    // Parsear valores separados por comas
    while (1) {
        ASTNode* expr = parser_parse_expression(parser);
        if (!expr) {
            for (int i = 0; i < count; i++) {
                ast_free_node(values[i]);
            }
            free(values);
            return NULL;
        }
        
        // Añadir a la lista
        if (count >= capacity) {
            capacity *= 2;
            ASTNode** new_values = (ASTNode**)realloc(values, sizeof(ASTNode*) * capacity);
            if (!new_values) {
                parser_set_error(parser, "Error de memoria al expandir lista de valores");
                ast_free_node(expr);
                for (int i = 0; i < count; i++) {
                    ast_free_node(values[i]);
                }
                free(values);
                return NULL;
            }
            values = new_values;
        }
        
        values[count++] = expr;
        
        // Si hay una coma, esperamos otro valor
        if (parser->current_token.type == TOKEN_PUNCTUATION && 
            strcmp(parser->current_token.value, ",") == 0) {
            parser_consume(parser);
        } else {
            break;
        }
    }
    
    if (parser->current_token.type != TOKEN_PUNCTUATION || 
        strcmp(parser->current_token.value, ")") != 0) {
        parser_set_error(parser, "Se esperaba ')' para cerrar la lista de valores");
        for (int i = 0; i < count; i++) {
            ast_free_node(values[i]);
        }
        free(values);
        return NULL;
    }

    parser_consume(parser); // Consumir el paréntesis DESPUÉS de verificarlo
    
    // Crear nodo de la lista
    ASTNode* value_list = ast_create_value_list(values, count);
    
    // Liberar el array (sus contenidos ahora pertenecen al nodo)
    free(values);
    
    return value_list;
}

// Parsear una lista de columnas
ASTNode* parser_parse_column_list(Parser* parser) {
    // Caso especial: SELECT *
    if (parser->current_token.type == TOKEN_OPERATOR && 
        strcmp(parser->current_token.value, "*") == 0) {
        parser_consume(parser);
        return ast_create_column_list(1, NULL, 0);
    }
    
    // Lista normal de columnas separadas por comas
    char** columns = NULL;
    int count = 0;
    int capacity = 8;
    
    columns = (char**)malloc(sizeof(char*) * capacity);
    if (!columns) {
        parser_set_error(parser, "Error de memoria al crear lista de columnas");
        return NULL;
    }
    
    while (1) {
        if (parser->current_token.type != TOKEN_IDENTIFIER) {
            parser_set_error(parser, "Se esperaba un nombre de columna");
            for (int i = 0; i < count; i++) {
                free(columns[i]);
            }
            free(columns);
            return NULL;
        }
        
        // Añadir columna a la lista
        if (count >= capacity) {
            capacity *= 2;
            char** new_columns = (char**)realloc(columns, sizeof(char*) * capacity);
            if (!new_columns) {
                parser_set_error(parser, "Error de memoria al expandir lista de columnas");
                for (int i = 0; i < count; i++) {
                    free(columns[i]);
                }
                free(columns);
                return NULL;
            }
            columns = new_columns;
        }
        
        columns[count++] = strdup(parser->current_token.value);
        parser_consume(parser);
        
        // Si hay una coma, esperamos otra columna
        if (parser->current_token.type == TOKEN_PUNCTUATION && 
            strcmp(parser->current_token.value, ",") == 0) {
            parser_consume(parser);
        } else {
            break;
        }
    }
    
    // Crear nodo de la lista de columnas
    ASTNode* column_list = ast_create_column_list(0, columns, count);
    
    // Liberar el array (sus contenidos ahora pertenecen al nodo)
    for (int i = 0; i < count; i++) {
        free(columns[i]);
    }
    free(columns);
    
    return column_list;
}

// Parsear una cláusula WHERE
ASTNode* parser_parse_where_clause(Parser* parser) {
    if (!parser_match_keyword(parser, "WHERE")) {
        return NULL; // No es un error, WHERE es opcional
    }
    
    ASTNode* condition = parser_parse_expression(parser);
    if (!condition) return NULL;
    
    return ast_create_where_clause(condition);
}

// Parsear una asignación (para UPDATE)
static ASTNode* parser_parse_assignment(Parser* parser) {
    if (parser->current_token.type != TOKEN_IDENTIFIER) {
        parser_set_error(parser, "Se esperaba un nombre de columna");
        return NULL;
    }
    
    char* column_name = strdup(parser->current_token.value);
    parser_consume(parser);
    
    if (parser->current_token.type != TOKEN_OPERATOR || 
        strcmp(parser->current_token.value, "=") != 0) {
        parser_set_error(parser, "Se esperaba '=' después del nombre de columna");
        free(column_name);
        return NULL;
    }
    
    parser_consume(parser);
    
    ASTNode* value = parser_parse_expression(parser);
    if (!value) {
        free(column_name);
        return NULL;
    }
    
    ASTNode* assignment = ast_create_assignment(column_name, value);
    free(column_name);
    
    return assignment;
}

// Parsear una lista de asignaciones (para UPDATE)
ASTNode* parser_parse_assignment_list(Parser* parser) {
    ASTNode* first_assignment = parser_parse_assignment(parser);
    if (!first_assignment) return NULL;
    
    ASTNode* current = first_assignment;
    
    // Si hay una coma, esperamos otra asignación
    while (parser->current_token.type == TOKEN_PUNCTUATION && 
           strcmp(parser->current_token.value, ",") == 0) {
        parser_consume(parser);
        
        ASTNode* next_assignment = parser_parse_assignment(parser);
        if (!next_assignment) {
            ast_free_node(first_assignment);
            return NULL;
        }
        
        // Enlazar como hermano
        ast_append_sibling(current, next_assignment);
        current = next_assignment;
    }
    
    return first_assignment;
}

// Parsear una definición de columna (para CREATE TABLE)
ASTNode* parser_parse_column_definition(Parser* parser) {
    // Nombre de columna
    if (parser->current_token.type != TOKEN_IDENTIFIER) {
        parser_set_error(parser, "Se esperaba un nombre de columna");
        return NULL;
    }
    
    char* column_name = strdup(parser->current_token.value);
    parser_consume(parser);
    
    // Tipo de datos
    if (!parser_check_keyword(parser, "INT") && 
        !parser_check_keyword(parser, "FLOAT") && 
        !parser_check_keyword(parser, "STRING") && 
        !parser_check_keyword(parser, "BOOL")) {
        parser_set_error(parser, "Se esperaba un tipo de datos (INT, FLOAT, STRING, BOOL)");
        free(column_name);
        return NULL;
    }
    
    int data_type;
    int max_length = 0;
    
    if (parser_check_keyword(parser, "INT")) {
        data_type = 1; // INT
        parser_consume(parser);
    }
    else if (parser_check_keyword(parser, "FLOAT")) {
        data_type = 2; // FLOAT
        parser_consume(parser);
    }
    else if (parser_check_keyword(parser, "STRING")) {
        data_type = 3; // STRING
        parser_consume(parser);
        
        // Para STRING, necesitamos la longitud
        if (parser->current_token.type != TOKEN_PUNCTUATION || 
            strcmp(parser->current_token.value, "(") != 0) {
            parser_set_error(parser, "Se esperaba '(' después de STRING");
            free(column_name);
            return NULL;
        }
        
        parser_consume(parser);
        
        if (parser->current_token.type != TOKEN_INTEGER) {
            parser_set_error(parser, "Se esperaba un número entero para la longitud");
            free(column_name);
            return NULL;
        }
        
        max_length = atoi(parser->current_token.value);
        parser_consume(parser);
        
        if (parser->current_token.type != TOKEN_PUNCTUATION || 
            strcmp(parser->current_token.value, ")") != 0) {
            parser_set_error(parser, "Se esperaba ')' después de la longitud");
            free(column_name);
            return NULL;
        }
        
        parser_consume(parser);
    }
    else if (parser_check_keyword(parser, "BOOL")) {
        data_type = 4; // BOOL
        parser_consume(parser);
    }
    
    // Opciones adicionales
    int is_primary_key = 0;
    int allows_null = 1;
    
    // Las opciones pueden aparecer en cualquier orden
    while (parser_check_keyword(parser, "PRIMARY") || parser_check_keyword(parser, "NOT")) {
        if (parser_check_keyword(parser, "PRIMARY")) {
            parser_consume(parser);
            
            if (!parser_match_keyword(parser, "KEY")) {
                free(column_name);
                return NULL;
            }
            
            is_primary_key = 1;
        }
        else if (parser_check_keyword(parser, "NOT")) {
            parser_consume(parser);
            
            if (!parser_match_keyword(parser, "NULL")) {
                free(column_name);
                return NULL;
            }
            
            allows_null = 0;
        }
    }
    
    ASTNode* column_def = ast_create_column_def(column_name, data_type, max_length, 
                                               is_primary_key, allows_null);
    free(column_name);
    
    return column_def;
}

// Parsear una sentencia SELECT
ASTNode* parser_parse_select(Parser* parser) {
    // SELECT
    if (!parser_match_keyword(parser, "SELECT")) {
        return NULL;
    }
    
    // Lista de columnas
    ASTNode* columns = parser_parse_column_list(parser);
    if (!columns) return NULL;
    
    // FROM
    if (!parser_match_keyword(parser, "FROM")) {
        ast_free_node(columns);
        return NULL;
    }
    
    // Nombre de tabla
    if (parser->current_token.type != TOKEN_IDENTIFIER) {
        parser_set_error(parser, "Se esperaba un nombre de tabla");
        ast_free_node(columns);
        return NULL;
    }
    
    char* table_name = strdup(parser->current_token.value);
    parser_consume(parser);
    
    // Cláusula WHERE (opcional)
    ASTNode* where = parser_parse_where_clause(parser);
    
    // Crear nodo SELECT
    ASTNode* select = ast_create_select(table_name, columns, where);
    free(table_name);
    
    if (!select) {
        ast_free_node(columns);
        if (where) ast_free_node(where);
        return NULL;
    }
    
    return select;
}

// Parsear una sentencia INSERT
ASTNode* parser_parse_insert(Parser* parser) {
    // INSERT INTO
    if (!parser_match_keyword(parser, "INSERT")) {
        return NULL;
    }
    
    // Revisar esta parte
    if (!parser_match_keyword(parser, "INTO")) {
        parser_set_error(parser, "Se esperaba la palabra clave 'INTO'");
        return NULL;
    }
    
    // Nombre de tabla
    if (parser->current_token.type != TOKEN_IDENTIFIER) {
        parser_set_error(parser, "Se esperaba un nombre de tabla");
        return NULL;
    }
    
    char* table_name = strdup(parser->current_token.value);
    parser_consume(parser);
    
    // VALUES
    if (!parser_match_keyword(parser, "VALUES")) {
        free(table_name);
        parser_set_error(parser, "Se esperaba la palabra clave 'VALUES'");
        return NULL;
    }
    
    // Verificar el paréntesis de apertura
    if (parser->current_token.type != TOKEN_PUNCTUATION || 
        !parser->current_token.value ||
        strcmp(parser->current_token.value, "(") != 0) {
        free(table_name);
        parser_set_error(parser, "Se esperaba '(' para iniciar la lista de valores");
        return NULL;
    }
    parser_consume(parser); // Añadir esta línea para consumir el paréntesis
    
    // Lista de valores
    ASTNode* values = parser_parse_value_list(parser);
    if (!values) {
        free(table_name);
        return NULL;
    }
    
    // Crear nodo INSERT
    ASTNode* insert = ast_create_insert(table_name, values);
    free(table_name);
    
    if (!insert) {
        ast_free_node(values);
        return NULL;
    }
    
    return insert;
}

// Parsear una sentencia UPDATE
ASTNode* parser_parse_update(Parser* parser) {
    // UPDATE
    if (!parser_match_keyword(parser, "UPDATE")) {
        return NULL;
    }
    
    // Nombre de tabla
    if (parser->current_token.type != TOKEN_IDENTIFIER) {
        parser_set_error(parser, "Se esperaba un nombre de tabla");
        return NULL;
    }
    
    char* table_name = strdup(parser->current_token.value);
    parser_consume(parser);
    
    // SET
    if (!parser_match_keyword(parser, "SET")) {
        free(table_name);
        return NULL;
    }
    
    // Lista de asignaciones
    ASTNode* assignments = parser_parse_assignment_list(parser);
    if (!assignments) {
        free(table_name);
        return NULL;
    }
    
    // Cláusula WHERE (opcional)
    ASTNode* where = parser_parse_where_clause(parser);
    
    // Crear nodo UPDATE
    ASTNode* update = ast_create_update(table_name, assignments, where);
    free(table_name);
    
    if (!update) {
        ast_free_node(assignments);
        if (where) ast_free_node(where);
        return NULL;
    }
    
    return update;
}

// Parsear una sentencia DELETE
ASTNode* parser_parse_delete(Parser* parser) {
    // DELETE FROM
    if (!parser_match_keyword(parser, "DELETE")) {
        return NULL;
    }
    
    if (!parser_match_keyword(parser, "FROM")) {
        return NULL;
    }
    
    // Nombre de tabla
    if (parser->current_token.type != TOKEN_IDENTIFIER) {
        parser_set_error(parser, "Se esperaba un nombre de tabla");
        return NULL;
    }
    
    char* table_name = strdup(parser->current_token.value);
    parser_consume(parser);
    
    // Cláusula WHERE (opcional)
    ASTNode* where = parser_parse_where_clause(parser);
    
    // Crear nodo DELETE
    ASTNode* delete_node = ast_create_delete(table_name, where);
    free(table_name);
    
    if (!delete_node) {
        if (where) ast_free_node(where);
        return NULL;
    }
    
    return delete_node;
}

// Parsear una sentencia CREATE TABLE
ASTNode* parser_parse_create_table(Parser* parser) {
    // CREATE TABLE
    if (!parser_match_keyword(parser, "CREATE")) {
        return NULL;
    }
    
    if (!parser_match_keyword(parser, "TABLE")) {
        return NULL;
    }
    
    // Nombre de tabla
    if (parser->current_token.type != TOKEN_IDENTIFIER) {
        parser_set_error(parser, "Se esperaba un nombre de tabla");
        return NULL;
    }
    
    char* table_name = strdup(parser->current_token.value);
    parser_consume(parser);
    
    // Lista de definiciones de columnas (opcional)
    ASTNode* columns = NULL;
    
    if (parser->current_token.type == TOKEN_PUNCTUATION && 
        strcmp(parser->current_token.value, "(") == 0) {
        
        parser_consume(parser);
        
        // Lista de columnas
        ASTNode** columns_array = NULL;
        int count = 0;
        int capacity = 8;
        
        columns_array = (ASTNode**)malloc(sizeof(ASTNode*) * capacity);
        if (!columns_array) {
            parser_set_error(parser, "Error de memoria al crear lista de columnas");
            free(table_name);
            return NULL;
        }
        
        // Parsear definiciones de columnas separadas por comas
        while (1) {
            ASTNode* col_def = parser_parse_column_definition(parser);
            if (!col_def) {
                for (int i = 0; i < count; i++) {
                    ast_free_node(columns_array[i]);
                }
                free(columns_array);
                free(table_name);
                return NULL;
            }
            
            // Añadir a la lista
            if (count >= capacity) {
                capacity *= 2;
                ASTNode** new_columns = (ASTNode**)realloc(columns_array, sizeof(ASTNode*) * capacity);
                if (!new_columns) {
                    parser_set_error(parser, "Error de memoria al expandir lista de columnas");
                    ast_free_node(col_def);
                    for (int i = 0; i < count; i++) {
                        ast_free_node(columns_array[i]);
                    }
                    free(columns_array);
                    free(table_name);
                    return NULL;
                }
                columns_array = new_columns;
            }

            columns_array[count++] = col_def;

            // Si hay una coma, esperamos otra definición de columna
            if (parser->current_token.type == TOKEN_PUNCTUATION && 
                strcmp(parser->current_token.value, ",") == 0) {
                parser_consume(parser);
            } else {
                break;
            }
        }

        // Esperar paréntesis de cierre
        if (parser->current_token.type != TOKEN_PUNCTUATION || 
            strcmp(parser->current_token.value, ")") != 0) {
            parser_set_error(parser, "Se esperaba ')' para cerrar la lista de columnas");
            for (int i = 0; i < count; i++) {
                ast_free_node(columns_array[i]);
            }
            free(columns_array);
            free(table_name);
            return NULL;
        } else {
            parser_consume(parser); 
        }

        // Crear nodo de la lista de columnas
        columns = ast_create_value_list(columns_array, count);

        // Liberar el array (sus contenidos ahora pertenecen al nodo)
        free(columns_array);
    }

    // Crear nodo CREATE TABLE
    ASTNode* create_table = ast_create_create_table(table_name, columns);
    free(table_name);

    if (!create_table) {
        if (columns) ast_free_node(columns);
        return NULL;
    }

    return create_table;
}

// Parsear una sentencia DROP TABLE
ASTNode* parser_parse_drop_table(Parser* parser) {
    // DROP TABLE
    if (!parser_match_keyword(parser, "DROP")) {
        return NULL;
    }
    
    if (!parser_match_keyword(parser, "TABLE")) {
        return NULL;
    }
    
    // Nombre de tabla
    if (parser->current_token.type != TOKEN_IDENTIFIER) {
        parser_set_error(parser, "Se esperaba un nombre de tabla");
        return NULL;
    }
    
    char* table_name = strdup(parser->current_token.value);
    parser_consume(parser);
    
    // Crear nodo DROP TABLE
    ASTNode* drop_table = ast_create_drop_table(table_name);
    free(table_name);
    
    if (!drop_table) {
        return NULL;
    }
    
    return drop_table;
}


// Parsear una sentencia ALTER TABLE
ASTNode* parser_parse_alter_table(Parser* parser) {
    // ALTER TABLE
    if (!parser_match_keyword(parser, "ALTER")) {
        return NULL;
    }
    
    if (!parser_match_keyword(parser, "TABLE")) {
        return NULL;
    }
    
    // Nombre de tabla
    if (parser->current_token.type != TOKEN_IDENTIFIER) {
        parser_set_error(parser, "Se esperaba un nombre de tabla");
        return NULL;
    }
    
    char* table_name = strdup(parser->current_token.value);
    parser_consume(parser);
    
    // Palabra clave ADD o DROP
    if (!parser_match_keyword(parser, "ADD") && !parser_match_keyword(parser, "DROP")) {
        free(table_name);
        return NULL;
    }
    
    // Definición de columna (para ADD) o nombre de columna (para DROP)
    ASTNode* column = NULL;
    char* column_name = NULL;
    
    if (parser_check_keyword(parser, "COLUMN")) {
        parser_consume(parser);
        
        if (parser->current_token.type != TOKEN_IDENTIFIER) {
            parser_set_error(parser, "Se esperaba un nombre de columna");
            free(table_name);
            return NULL;
        }
        
        // Ahora tenemos el nombre, lo guardamos
        char* name = strdup(parser->current_token.value);
        parser_consume(parser);
        
        // Procesamos el tipo y otras restricciones
        if (!parser_check_keyword(parser, "INT") && 
            !parser_check_keyword(parser, "FLOAT") && 
            !parser_check_keyword(parser, "STRING") && 
            !parser_check_keyword(parser, "BOOL")) {
            parser_set_error(parser, "Se esperaba un tipo de datos");
            free(name);
            free(table_name);
            return NULL;
        }
        
        // Implementación del manejo de tipos
        int data_type;
        int max_length = 0;
        
        if (parser_check_keyword(parser, "INT")) {
            data_type = 1; // INT
            parser_consume(parser);
        }
        else if (parser_check_keyword(parser, "FLOAT")) {
            data_type = 2; // FLOAT
            parser_consume(parser);
        }
        else if (parser_check_keyword(parser, "STRING")) {
            data_type = 3; // STRING
            parser_consume(parser);
            
            // Para STRING, necesitamos la longitud
            if (parser->current_token.type != TOKEN_PUNCTUATION || 
                strcmp(parser->current_token.value, "(") != 0) {
                parser_set_error(parser, "Se esperaba '(' después de STRING");
                free(name);
                free(table_name);
                return NULL;
            }
            
            parser_consume(parser);
            
            if (parser->current_token.type != TOKEN_INTEGER) {
                parser_set_error(parser, "Se esperaba un número entero para la longitud");
                free(name);
                free(table_name);
                return NULL;
            }
            
            max_length = atoi(parser->current_token.value);
            parser_consume(parser);
            
            if (parser->current_token.type != TOKEN_PUNCTUATION || 
                strcmp(parser->current_token.value, ")") != 0) {
                parser_set_error(parser, "Se esperaba ')' después de la longitud");
                free(name);
                free(table_name);
                return NULL;
            }
            
            parser_consume(parser);
        }
        else if (parser_check_keyword(parser, "BOOL")) {
            data_type = 4; // BOOL
            parser_consume(parser);
        }
        
        // Opciones adicionales
        int is_primary_key = 0;
        int allows_null = 1;
        
        // Las opciones pueden aparecer en cualquier orden
        while (parser_check_keyword(parser, "PRIMARY") || parser_check_keyword(parser, "NOT")) {
            if (parser_check_keyword(parser, "PRIMARY")) {
                parser_consume(parser);
                
                if (!parser_match_keyword(parser, "KEY")) {
                    free(name);
                    free(table_name);
                    return NULL;
                }
                
                is_primary_key = 1;
            }
            else if (parser_check_keyword(parser, "NOT")) {
                parser_consume(parser);
                
                if (!parser_match_keyword(parser, "NULL")) {
                    free(name);
                    free(table_name);
                    return NULL;
                }
                
                allows_null = 0;
            }
        }
        
        // Crear la definición de columna
        column = ast_create_column_def(name, data_type, max_length, is_primary_key, allows_null);
        free(name);
        
        if (!column) {
            free(table_name);
            return NULL;
        }
    }
    else {
        column = parser_parse_column_definition(parser);
        if (!column) {
            free(table_name);
            return NULL;
        }
    }
    
    // Crear nodo ALTER TABLE
    ASTNode* alter_table = ast_create_alter_table(table_name, column);
    free(table_name);
    if (column_name) {
        ast_set_column_name(alter_table, column_name);
        free(column_name);
    }
    
    if (!alter_table) {
        if (column) ast_free_node(column);
        return NULL;
    }
    
    return alter_table;
}

// Parsear una sentencia
ASTNode* parser_parse_statement(Parser* parser) {
    // Versión corregida:
    if (parser_check_keyword(parser, "SELECT"))
        return parser_parse_select(parser);
    else if (parser_check_keyword(parser, "INSERT"))
        return parser_parse_insert(parser);
    else if (parser_check_keyword(parser, "UPDATE"))
        return parser_parse_update(parser);
    else if (parser_check_keyword(parser, "DELETE"))
        return parser_parse_delete(parser);
    else if (parser_check_keyword(parser, "CREATE"))
        return parser_parse_create_table(parser);
    else if (parser_check_keyword(parser, "DROP"))
        return parser_parse_drop_table(parser);
    else if (parser_check_keyword(parser, "ALTER"))
        return parser_parse_alter_table(parser);
    else {
        parser_set_error(parser, "Sentencia SQL desconocida");
        return NULL;
    }
}

ASTNode* parser_parse(Parser* parser){
    return parser_parse_statement(parser);
}


// Función para liberar recursos del parser
void parser_free(Parser* parser) {
    if (!parser) return;
    
    // Liberar token actual del parser
    token_free(&parser->current_token);
    
    // Liberar mensaje de error
    if (parser->error_message) {
        free(parser->error_message);
        parser->error_message = NULL;
    }
    
    // Liberar lexer correctamente
    if (parser->lexer) {
        // Liberar el token del lexer antes de liberar el lexer
        token_free(&parser->lexer->current_token);
        lexer_free(parser->lexer);
        parser->lexer = NULL;
    }
    
    free(parser);
}