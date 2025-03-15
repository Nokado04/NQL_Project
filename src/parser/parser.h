#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"

// Estructura principal del parser
typedef struct {
    Lexer* lexer;              // El lexer que proporciona los tokens
    Token current_token;       // Token actual
    char* error_message;       // Mensaje de error
    int error_position;        // Posición del error
} Parser;

// Crear un nuevo parser para una cadena SQL
Parser* parser_create(const char* sql);

// Liberar recursos del parser
void parser_free(Parser* parser);

// Analizar la entrada y generar un AST
ASTNode* parser_parse(Parser* parser);

// Verificar si hay un error de sintaxis
int parser_has_error(Parser* parser);

// Obtener mensaje de error de sintaxis
const char* parser_get_error(Parser* parser);

// Funciones para analizar construcciones específicas
ASTNode* parser_parse_statement(Parser* parser);
ASTNode* parser_parse_select(Parser* parser);
ASTNode* parser_parse_insert(Parser* parser);
ASTNode* parser_parse_update(Parser* parser);
ASTNode* parser_parse_delete(Parser* parser);
ASTNode* parser_parse_create_table(Parser* parser);
ASTNode* parser_parse_alter_table(Parser* parser);
ASTNode* parser_parse_drop_table(Parser* parser);

// Funciones para analizar componentes
ASTNode* parser_parse_column_list(Parser* parser);
ASTNode* parser_parse_value_list(Parser* parser);
ASTNode* parser_parse_where_clause(Parser* parser);
ASTNode* parser_parse_assignment_list(Parser* parser);
ASTNode* parser_parse_column_definition(Parser* parser);
ASTNode* parser_parse_expression(Parser* parser);

#endif /* PARSER_H */