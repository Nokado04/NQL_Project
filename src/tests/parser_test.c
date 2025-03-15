#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../parser/lexer.h"
#include "../parser/parser.h"
#include "../parser/ast.h"
#include "../parser/validator.h"
#include "../parser/grammar.h"
#include "../db/database.h"

// Constantes para el formato de salida
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_RESET   "\x1b[0m"

// Funciones de utilidad
void print_test_result(const char* test_name, int success) {
    printf("[%s] %s: %s\n", 
           success ? ANSI_COLOR_GREEN "PASS" ANSI_COLOR_RESET : ANSI_COLOR_RED "FAIL" ANSI_COLOR_RESET,
           test_name,
           success ? "✓" : "✗");
}

void print_separator() {
    printf(ANSI_COLOR_YELLOW "----------------------------------------\n" ANSI_COLOR_RESET);
}

// ============= PRUEBAS DEL LEXER =============

void test_lexer_basic_tokens() {
    printf(ANSI_COLOR_BLUE "Prueba 1: Tokenización básica\n" ANSI_COLOR_RESET);
    
    const char* sql = "SELECT * FROM tabla WHERE columna = 10;";
    printf("SQL: %s\n", sql);
    
    Lexer* lexer = lexer_create(sql);
    if (!lexer) {
        printf("Error: No se pudo crear el lexer\n");
        return;
    }
    
    int success = 1;
    int token_count = 0;
    
    // Tokens esperados
    TokenType expected_types[] = {
        TOKEN_KEYWORD,    // SELECT
        TOKEN_PUNCTUATION, // *
        TOKEN_KEYWORD,    // FROM
        TOKEN_IDENTIFIER, // tabla
        TOKEN_KEYWORD,    // WHERE
        TOKEN_IDENTIFIER, // columna
        TOKEN_OPERATOR,   // =
        TOKEN_INTEGER,    // 10
        TOKEN_PUNCTUATION, // ;
        TOKEN_EOF        // Fin
    };
    
    const char* expected_values[] = {
        "SELECT", "*", "FROM", "tabla", "WHERE", "columna", "=", "10", ";", NULL
    };
    
    Token token;
    while ((token = lexer_next_token(lexer)).type != TOKEN_EOF && token_count < 9) {
        printf("Token %d: Tipo=%s, Valor='%s'\n", 
               token_count + 1, 
               token_type_to_string(token.type), 
               token.value ? token.value : "NULL");
        
        // Verificar tipo y valor
        if (token.type != expected_types[token_count] || 
            strcmp(token.value, expected_values[token_count]) != 0) {
            success = 0;
        }
        
        token_count++;
    }
    
    // Verificar el número correcto de tokens
    if (token_count != 9) {
        success = 0;
    }
    
    print_test_result("Tokenización básica", success);
    lexer_free(lexer);
}

// Probar un string complejo con espacios y caracteres especiales
void test_lexer_string_literals() {
    printf(ANSI_COLOR_BLUE "Prueba 2: Literales de cadena\n" ANSI_COLOR_RESET);
    
    const char* sql = "INSERT INTO usuarios VALUES (1, \"Juan Pérez\", 25.5);";
    printf("SQL: %s\n", sql);
    
    Lexer* lexer = lexer_create(sql);
    if (!lexer) {
        printf("Error: No se pudo crear el lexer\n");
        return;
    }
    
    // Buscar el literal de cadena
    Token token;
    int found_string = 0;
    
    while ((token = lexer_next_token(lexer)).type != TOKEN_EOF) {
        printf("Token: Tipo=%s, Valor='%s'\n", 
               token_type_to_string(token.type), 
               token.value ? token.value : "NULL");
        
        if (token.type == TOKEN_STRING && strcmp(token.value, "Juan Pérez") == 0) {
            found_string = 1;
            break;
        }
    }
    
    print_test_result("Literales de cadena", found_string);
    lexer_free(lexer);
}

// Probar keywords vs identificadores
void test_lexer_keywords_vs_identifiers() {
    printf(ANSI_COLOR_BLUE "Prueba 3: Keywords vs Identificadores\n" ANSI_COLOR_RESET);
    
    const char* sql = "SELECT select FROM from WHERE select_column";
    printf("SQL: %s\n", sql);
    
    Lexer* lexer = lexer_create(sql);
    if (!lexer) {
        printf("Error: No se pudo crear el lexer\n");
        return;
    }
    
    Token tokens[6];
    int i = 0;
    
    while (i < 6 && (tokens[i] = lexer_next_token(lexer)).type != TOKEN_EOF) {
        printf("Token %d: Tipo=%s, Valor='%s'\n", 
               i + 1, 
               token_type_to_string(tokens[i].type), 
               tokens[i].value ? tokens[i].value : "NULL");
        i++;
    }
    
    // Verificar tipos esperados
    int success = (i == 6 &&
                  tokens[0].type == TOKEN_KEYWORD &&    // SELECT
                  tokens[1].type == TOKEN_IDENTIFIER && // select
                  tokens[2].type == TOKEN_KEYWORD &&    // FROM
                  tokens[3].type == TOKEN_IDENTIFIER && // from
                  tokens[4].type == TOKEN_KEYWORD &&    // WHERE
                  tokens[5].type == TOKEN_IDENTIFIER);  // select_column
    
    print_test_result("Keywords vs Identificadores", success);
    lexer_free(lexer);
}

// ============= PRUEBAS DE LA BASE DE DATOS DE PRUEBA =============

// Crear una base de datos de ejemplo para las pruebas
Database* create_test_database() {
    printf(ANSI_COLOR_BLUE "Creando base de datos de prueba...\n" ANSI_COLOR_RESET);
    
    // Crear la base de datos (debe existir en el código real)
    Database* db = (Database*)malloc(sizeof(Database));
    if (!db) {
        printf("Error: No se pudo asignar memoria para la base de datos\n");
        return NULL;
    }
    
    // Inicializar la base de datos
    db->name = strdup("test_db");
    db->num_tables = 0;
    db->max_tables = 10;
    db->tables = (Table**)malloc(sizeof(Table*) * 10);
    
    if (!db->tables) {
        printf("Error: No se pudo asignar memoria para las tablas\n");
        free(db->name);
        free(db);
        return NULL;
    }
    
    // Crear tabla de usuarios
    Table* usuarios = (Table*)malloc(sizeof(Table));
    if (!usuarios) {
        printf("Error: No se pudo asignar memoria para la tabla usuarios\n");
        free(db->tables);
        free(db->name);
        free(db);
        return NULL;
    }
    
    usuarios->name = strdup("usuarios");
    usuarios->num_columns = 4;
    usuarios->columns = (Column*)malloc(sizeof(Column) * 4);
    
    if (!usuarios->columns) {
        printf("Error: No se pudo asignar memoria para las columnas\n");
        free(usuarios->name);
        free(usuarios);
        free(db->tables);
        free(db->name);
        free(db);
        return NULL;
    }
    
    // Configurar columnas
    usuarios->columns[0].name = strdup("id");
    usuarios->columns[0].type = TYPE_INT;  // Asumiendo que TYPE_INT está definido en database.h
    usuarios->columns[0].max_length = 0;
    usuarios->columns[0].is_primary_key = 1;
    usuarios->columns[0].allows_null = 0;
    
    usuarios->columns[1].name = strdup("nombre");
    usuarios->columns[1].type = TYPE_STRING;
    usuarios->columns[1].max_length = 50;
    usuarios->columns[1].is_primary_key = 0;
    usuarios->columns[1].allows_null = 0;
    
    usuarios->columns[2].name = strdup("edad");
    usuarios->columns[2].type = TYPE_INT;
    usuarios->columns[2].max_length = 0;
    usuarios->columns[2].is_primary_key = 0;
    usuarios->columns[2].allows_null = 1;
    
    usuarios->columns[3].name = strdup("activo");
    usuarios->columns[3].type = TYPE_BOOL;
    usuarios->columns[3].max_length = 0;
    usuarios->columns[3].is_primary_key = 0;
    usuarios->columns[3].allows_null = 0;
    
    // Añadir a la base de datos
    db->tables[0] = usuarios;
    db->num_tables = 1;
    
    printf("Base de datos creada con 1 tabla (usuarios) y 4 columnas\n");
    return db;
}

void free_test_database(Database* db) {
    if (!db) return;
    
    for (int i = 0; i < db->num_tables; i++) {
        Table* table = db->tables[i];
        for (int j = 0; j < table->num_columns; j++) {
            free(table->columns[j].name);
        }
        free(table->columns);
        free(table->name);
        free(table);
    }
    
    free(db->tables);
    free(db->name);
    free(db);
}

// En caso de que funcione el parser, podemos probar esta función
// Alternativa: Para los propósitos de prueba, podemos crear AST manualmente
void test_parser_if_available(const char* sql) {
    printf(ANSI_COLOR_BLUE "Intentando parsear: %s\n" ANSI_COLOR_RESET, sql);
    
    // Verificar si la función parser_create está disponible (puede fallar si parser.c está incompleto)
    Parser* parser = parser_create(sql);
    if (!parser) {
        printf("Parser no disponible o no pudo inicializarse\n");
        return;
    }
    
    // Probar el parser
    ASTNode* ast = parser_parse(parser);
    if (!ast) {
        printf("Error al parsear: %s\n", 
               parser->error_message ? parser->error_message : "Error desconocido");
    } else {
        printf("Parseo exitoso, AST creado\n");
        
        // Si tenemos la función ast_print, podemos usarla
        // ast_print(ast, 0);
        
        // Liberar AST
        // ast_free_node(ast);
    }
    
    // Liberar parser
    parser_free(parser);
}

// Crear un AST manualmente para pruebas (simulando lo que haría el parser)
ASTNode* create_test_select_ast() {
    // Crear un nodo para "SELECT * FROM usuarios"
    // Crear lista de columnas (* = all)
    ColumnListData* columns_data = (ColumnListData*)malloc(sizeof(ColumnListData));
    columns_data->is_all = 1;
    columns_data->count = 0;
    columns_data->columns = NULL;
    
    ASTNode* columns_node = (ASTNode*)malloc(sizeof(ASTNode));
    columns_node->type = NODE_COLUMN_LIST;
    columns_node->data = columns_data;
    columns_node->parent = NULL;
    columns_node->next = NULL;
    columns_node->prev = NULL;
    columns_node->free_data = NULL; // Idealmente, tendríamos una función free_column_list
    
    // Crear nodo SELECT
    SelectStmtData* select_data = (SelectStmtData*)malloc(sizeof(SelectStmtData));
    select_data->table_name = strdup("usuarios");
    select_data->columns = columns_node;
    select_data->where_clause = NULL; // Sin cláusula WHERE
    
    ASTNode* select_node = (ASTNode*)malloc(sizeof(ASTNode));
    select_node->type = NODE_SELECT_STMT;
    select_node->data = select_data;
    select_node->parent = NULL;
    select_node->next = NULL;
    select_node->prev = NULL;
    select_node->free_data = NULL; // Idealmente, tendríamos una función free_select_stmt
    
    // Establecer relación padre-hijo
    columns_node->parent = select_node;
    
    return select_node;
}

// ============= PRUEBA MANUAL DE VALIDACIÓN =============

void test_validator_with_manual_ast(Database* db) {
    printf(ANSI_COLOR_BLUE "Prueba de validación con AST manual\n" ANSI_COLOR_RESET);
    
    // Crear AST manualmente
    ASTNode* ast = create_test_select_ast();
    printf("AST manual creado para SELECT * FROM usuarios\n");
    
    // Crear resultado de validación
    ValidationResult* result = validator_create_result();
    if (!result) {
        printf("Error: No se pudo crear el resultado de validación\n");
        // Liberar AST (idealmente con una función ast_free_node)
        free(((SelectStmtData*)ast->data)->table_name);
        free(ast->data);
        free(((ColumnListData*)((SelectStmtData*)ast->data)->columns->data));
        free(((SelectStmtData*)ast->data)->columns);
        free(ast);
        return;
    }
    
    // Validar AST
    int valid = validator_validate(ast, db, result);
    
    if (valid) {
        printf(ANSI_COLOR_GREEN "Validación exitosa: AST es válido contra el esquema\n" ANSI_COLOR_RESET);
    } else {
        printf(ANSI_COLOR_RED "Validación fallida: %s (código: %d)\n" ANSI_COLOR_RESET,
               result->error_message ? result->error_message : "Error desconocido",
               result->error_code);
    }
    
    // Liberar recursos
    validator_free_result(result);
    
    // Liberar AST (idealmente con una función ast_free_node)
    free(((SelectStmtData*)ast->data)->table_name);
    free(ast->data);
    free(((ColumnListData*)((SelectStmtData*)ast->data)->columns->data));
    free(((SelectStmtData*)ast->data)->columns);
    free(ast);
}

// ============= FUNCIÓN PRINCIPAL =============

int main() {
    printf(ANSI_COLOR_YELLOW "=== PRUEBAS DEL ANALIZADOR SQL ===\n" ANSI_COLOR_RESET);
    
    // Mostrar la especificación de la gramática
    printf(ANSI_COLOR_BLUE "Gramática SQL soportada:\n" ANSI_COLOR_RESET);
    // Descomenta si grammar_get_specification está implementado
    // printf("%s\n", grammar_get_specification());
    print_separator();
    
    // Pruebas del lexer
    test_lexer_basic_tokens();
    print_separator();
    
    test_lexer_string_literals();
    print_separator();
    
    test_lexer_keywords_vs_identifiers();
    print_separator();
    
    // Crear base de datos de prueba
    Database* db = create_test_database();
    if (!db) {
        printf("Error: No se pudo crear la base de datos de prueba\n");
        return 1;
    }
    print_separator();
    
    // Intentar usar el parser si está disponible
    test_parser_if_available("SELECT * FROM usuarios");
    print_separator();
    
    // Prueba de validación con AST creado manualmente
    test_validator_with_manual_ast(db);
    print_separator();
    
    // Liberar recursos
    free_test_database(db);
    
    printf(ANSI_COLOR_YELLOW "=== PRUEBAS COMPLETADAS ===\n" ANSI_COLOR_RESET);
    return 0;
}