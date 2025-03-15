#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"

// Palabras clave reconocidas por el lexer
static const char* keywords[] = {
    "SELECT", "FROM", "WHERE", "INSERT", "INTO", "VALUES",
    "UPDATE", "SET", "DELETE", "CREATE", "TABLE", "ALTER",
    "ADD", "COLUMN", "DROP", "PRIMARY", "KEY", "NOT",
    "NULL", "INT", "FLOAT", "STRING", "BOOL", "TRUE",
    "FALSE", "AND", "OR", NULL
};

// Verifica si una cadena es una palabra clave
static int is_keyword(const char* str) {
    for (int i = 0; keywords[i] != NULL; i++) {
        if (strcasecmp(str, keywords[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

// Funciones auxiliares para verificar caracteres
static int is_whitespace(char c) {
    return c == ' ' || c == '\t' || c == '\n' || c == '\r';
}

static int is_alpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static int is_digit(char c) {
    return c >= '0' && c <= '9';
}

static int is_identifier_start(char c) {
    return is_alpha(c) || c == '_';
}

static int is_identifier_char(char c) {
    return is_alpha(c) || is_digit(c) || c == '_';
}

static int is_operator_char(char c) {
    return c == '+' || c == '-' || c == '*' || c == '/' || 
           c == '=' || c == '<' || c == '>' || c == '!';
}

static int is_punctuation(char c) {
    return c == '(' || c == ')' || c == ',' || c == ';' || c == '.';
}

// Función para crear un nuevo token vacío
static Token create_token() {
    Token token;
    token.type = TOKEN_ERROR;
    token.value = NULL;
    token.line = 0;
    token.column = 0;
    token.position = 0;
    return token;
}

// Función para establecer un error en el lexer
static void lexer_set_error(Lexer* lexer, const char* message) {
    if (lexer->error_message) {
        free(lexer->error_message);
    }
    lexer->error_message = strdup(message);
}

// Función para crear un nuevo lexer
Lexer* lexer_create(const char* input) {
    Lexer* lexer = (Lexer*) malloc(sizeof(Lexer));
    if (!lexer) return NULL;
    
    lexer->input = input;
    lexer->position = 0;
    lexer->line = 1;
    lexer->column = 1;
    lexer->current_token = create_token();
    lexer->error_message = NULL;
    
    return lexer;
}

// Función para liberar un lexer
void lexer_free(Lexer* lexer) {
    if (!lexer) return;
    
    token_free(&lexer->current_token);
    
    if (lexer->error_message) {
        free(lexer->error_message);
    }
    
    free(lexer);
}

// Funciones auxiliares para avanzar en el input
static char lexer_peek(Lexer* lexer) {
    if (!lexer->input[lexer->position]) {
        return '\0'; // Fin de entrada
    }
    return lexer->input[lexer->position];
}

static char lexer_peek_next(Lexer* lexer) {
    if (!lexer->input[lexer->position] || !lexer->input[lexer->position + 1]) {
        return '\0'; // Fin de entrada
    }
    return lexer->input[lexer->position + 1];
}

static char lexer_advance(Lexer* lexer) {
    if (!lexer->input[lexer->position]) {
        return '\0'; // Fin de entrada
    }
    
    char c = lexer->input[lexer->position++];
    
    if (c == '\n') {
        lexer->line++;
        lexer->column = 1;
    } else {
        lexer->column++;
    }
    
    return c;
}

// Función para omitir espacios en blanco
static void lexer_skip_whitespace(Lexer* lexer) {
    while (is_whitespace(lexer_peek(lexer))) {
        lexer_advance(lexer);
    }
}

// Función para omitir comentarios
static void lexer_skip_comments(Lexer* lexer) {
    if (lexer_peek(lexer) == '-' && lexer_peek_next(lexer) == '-') {
        // Comentario de línea
        lexer_advance(lexer); // Consumir el primer '-'
        lexer_advance(lexer); // Consumir el segundo '-'
        
        while (lexer_peek(lexer) != '\0' && lexer_peek(lexer) != '\n') {
            lexer_advance(lexer);
        }
    }
    else if (lexer_peek(lexer) == '/' && lexer_peek_next(lexer) == '*') {
        // Comentario multi-línea
        lexer_advance(lexer); // Consumir '/'
        lexer_advance(lexer); // Consumir '*'
        
        while (lexer_peek(lexer) != '\0') {
            if (lexer_peek(lexer) == '*' && lexer_peek_next(lexer) == '/') {
                lexer_advance(lexer); // Consumir '*'
                lexer_advance(lexer); // Consumir '/'
                break;
            }
            lexer_advance(lexer);
        }
    }
}

// Función para leer un identificador o palabra clave
static Token lexer_read_identifier(Lexer* lexer) {
    int start_position = lexer->position;
    int start_line = lexer->line;
    int start_column = lexer->column;
    
    while (is_identifier_char(lexer_peek(lexer))) {
        lexer_advance(lexer);
    }
    
    // Extraer la cadena del identificador
    int length = lexer->position - start_position;
    char* identifier = (char*) malloc(length + 1);
    if (!identifier) {
        lexer_set_error(lexer, "Error de memoria al leer identificador");
        Token token = create_token();
        token.position = start_position;
        token.line = start_line;
        token.column = start_column;
        return token;
    }
    
    strncpy(identifier, lexer->input + start_position, length);
    identifier[length] = '\0';
    
    // Crear token
    Token token = create_token();
    token.position = start_position;
    token.line = start_line;
    token.column = start_column;
    token.value = identifier;
    
    // Determinar si es palabra clave o identificador
    if (is_keyword(identifier)) {
        token.type = TOKEN_KEYWORD;
    } else {
        token.type = TOKEN_IDENTIFIER;
    }
    
    return token;
}

// Función para leer un número (entero o decimal)
static Token lexer_read_number(Lexer* lexer) {
    int start_position = lexer->position;
    int start_line = lexer->line;
    int start_column = lexer->column;
    int is_float = 0;
    
    // Leer parte entera
    while (is_digit(lexer_peek(lexer))) {
        lexer_advance(lexer);
    }
    
    // Verificar si es decimal
    if (lexer_peek(lexer) == '.' && is_digit(lexer_peek_next(lexer))) {
        is_float = 1;
        lexer_advance(lexer); // Consumir '.'
        
        // Leer parte decimal
        while (is_digit(lexer_peek(lexer))) {
            lexer_advance(lexer);
        }
    }
    
    // Extraer la cadena del número
    int length = lexer->position - start_position;
    char* number_str = (char*) malloc(length + 1);
    if (!number_str) {
        lexer_set_error(lexer, "Error de memoria al leer número");
        Token token = create_token();
        token.position = start_position;
        token.line = start_line;
        token.column = start_column;
        return token;
    }
    
    strncpy(number_str, lexer->input + start_position, length);
    number_str[length] = '\0';
    
    // Crear token
    Token token = create_token();
    token.position = start_position;
    token.line = start_line;
    token.column = start_column;
    token.value = number_str;
    token.type = is_float ? TOKEN_FLOAT : TOKEN_INTEGER;
    
    return token;
}

// Función para leer una cadena literal ("texto")
static Token lexer_read_string(Lexer* lexer) {
    int start_position = lexer->position;
    int start_line = lexer->line;
    int start_column = lexer->column;
    
    lexer_advance(lexer); // Consumir la comilla inicial
    
    // Inicializar buffer para la cadena
    int buffer_size = 16;
    int buffer_pos = 0;
    char* buffer = (char*) malloc(buffer_size);
    if (!buffer) {
        lexer_set_error(lexer, "Error de memoria al leer cadena");
        Token token = create_token();
        token.position = start_position;
        token.line = start_line;
        token.column = start_column;
        return token;
    }
    
    // Leer hasta la comilla final
    while (lexer_peek(lexer) != '\0' && lexer_peek(lexer) != '"') {
        // Manejar caracteres de escape
        if (lexer_peek(lexer) == '\\') {
            lexer_advance(lexer); // Consumir '\'
            
            if (lexer_peek(lexer) == '\0') {
                free(buffer);
                lexer_set_error(lexer, "Cadena no cerrada (EOF después de \\)");
                Token token = create_token();
                token.position = start_position;
                token.line = start_line;
                token.column = start_column;
                return token;
            }
            
            char c;
            switch (lexer_peek(lexer)) {
                case 'n': c = '\n'; break;
                case 't': c = '\t'; break;
                case 'r': c = '\r'; break;
                case '\\': c = '\\'; break;
                case '"': c = '"'; break;
                default: c = lexer_peek(lexer);
            }
            
            // Aumentar el buffer si es necesario
            if (buffer_pos >= buffer_size - 1) {
                buffer_size *= 2;
                buffer = (char*) realloc(buffer, buffer_size);
                if (!buffer) {
                    lexer_set_error(lexer, "Error de memoria al expandir buffer");
                    Token token = create_token();
                    token.position = start_position;
                    token.line = start_line;
                    token.column = start_column;
                    return token;
                }
            }
            
            buffer[buffer_pos++] = c;
            lexer_advance(lexer);
        } else {
            // Aumentar el buffer si es necesario
            if (buffer_pos >= buffer_size - 1) {
                buffer_size *= 2;
                buffer = (char*) realloc(buffer, buffer_size);
                if (!buffer) {
                    lexer_set_error(lexer, "Error de memoria al expandir buffer");
                    Token token = create_token();
                    token.position = start_position;
                    token.line = start_line;
                    token.column = start_column;
                    return token;
                }
            }
            
            buffer[buffer_pos++] = lexer_peek(lexer);
            lexer_advance(lexer);
        }
    }
    
    if (lexer_peek(lexer) != '"') {
        free(buffer);
        lexer_set_error(lexer, "Cadena no cerrada (EOF antes de \")");
        Token token = create_token();
        token.position = start_position;
        token.line = start_line;
        token.column = start_column;
        return token;
    }
    
    lexer_advance(lexer); // Consumir la comilla final
    
    // Añadir terminador de cadena
    buffer[buffer_pos] = '\0';
    
    // Crear token
    Token token = create_token();
    token.position = start_position;
    token.line = start_line;
    token.column = start_column;
    token.value = buffer;
    token.type = TOKEN_STRING;
    
    return token;
}

// Función para leer un operador
static Token lexer_read_operator(Lexer* lexer) {
    int start_position = lexer->position;
    int start_line = lexer->line;
    int start_column = lexer->column;
    
    char first = lexer_advance(lexer);
    char* op_str = NULL;
    
    // Operadores de dos caracteres
    if ((first == '<' && lexer_peek(lexer) == '>') ||   // <>
        (first == '<' && lexer_peek(lexer) == '=') ||   // <=
        (first == '>' && lexer_peek(lexer) == '=') ||   // >=
        (first == '!' && lexer_peek(lexer) == '=')) {   // !=
        
        char second = lexer_advance(lexer);
        op_str = (char*) malloc(3);
        if (!op_str) {
            lexer_set_error(lexer, "Error de memoria al leer operador");
            Token token = create_token();
            token.position = start_position;
            token.line = start_line;
            token.column = start_column;
            return token;
        }
        
        op_str[0] = first;
        op_str[1] = second;
        op_str[2] = '\0';
    } else {
        // Operadores de un carácter
        op_str = (char*) malloc(2);
        if (!op_str) {
            lexer_set_error(lexer, "Error de memoria al leer operador");
            Token token = create_token();
            token.position = start_position;
            token.line = start_line;
            token.column = start_column;
            return token;
        }
        
        op_str[0] = first;
        op_str[1] = '\0';
    }
    
    // Crear token
    Token token = create_token();
    token.position = start_position;
    token.line = start_line;
    token.column = start_column;
    token.value = op_str;
    token.type = TOKEN_OPERATOR;
    
    return token;
}

// Función principal para obtener el siguiente token
Token lexer_next_token(Lexer* lexer) {
    // Omitir espacios en blanco y comentarios
    lexer_skip_whitespace(lexer);
    lexer_skip_comments(lexer);
    
    // Guardar la posición actual para el token
    int start_position = lexer->position;
    int start_line = lexer->line;
    int start_column = lexer->column;
    
    // Detectar fin de entrada
    if (lexer_peek(lexer) == '\0') {
        Token token = create_token();
        token.type = TOKEN_EOF;
        token.position = start_position;
        token.line = start_line;
        token.column = start_column;
        lexer->current_token = token;
        return token;
    }
    
    // Identificadores y palabras clave
    if (is_identifier_start(lexer_peek(lexer))) {
        lexer->current_token = lexer_read_identifier(lexer);
        return lexer->current_token;
    }
    
    // Números
    if (is_digit(lexer_peek(lexer))) {
        lexer->current_token = lexer_read_number(lexer);
        return lexer->current_token;
    }
    
    // Cadenas
    if (lexer_peek(lexer) == '"') {
        lexer->current_token = lexer_read_string(lexer);
        return lexer->current_token;
    }
    
    // Operadores
    if (is_operator_char(lexer_peek(lexer))) {
        lexer->current_token = lexer_read_operator(lexer);
        return lexer->current_token;
    }
    
    // Puntuación
    if (is_punctuation(lexer_peek(lexer))) {
        char c = lexer_advance(lexer);
        
        Token token = create_token();
        token.type = TOKEN_PUNCTUATION;
        token.position = start_position;
        token.line = start_line;
        token.column = start_column;
        
        token.value = (char*) malloc(2);
        if (!token.value) {
            lexer_set_error(lexer, "Error de memoria al crear token de puntuación");
            return token;
        }
        
        token.value[0] = c;
        token.value[1] = '\0';
        
        lexer->current_token = token;
        return token;
    }
    
    // Carácter desconocido
    char unknown = lexer_advance(lexer);
    
    Token token = create_token();
    token.type = TOKEN_ERROR;
    token.position = start_position;
    token.line = start_line;
    token.column = start_column;
    
    char error_msg[50];
    snprintf(error_msg, sizeof(error_msg), "Carácter desconocido: '%c' (0x%02X)", 
             unknown, (unsigned char)unknown);
    lexer_set_error(lexer, error_msg);
    
    lexer->current_token = token;
    return token;
}

// Función para ver el siguiente token sin consumirlo
Token lexer_peek_token(Lexer* lexer) {
    // Guardar estado actual
    int saved_position = lexer->position;
    int saved_line = lexer->line;
    int saved_column = lexer->column;
    Token saved_token = lexer->current_token;
    
    // Hacer una copia del valor del token para evitar doble liberación
    if (saved_token.value) {
        saved_token.value = strdup(saved_token.value);
    }
    
    // Obtener el siguiente token
    Token next_token = lexer_next_token(lexer);
    
    // Hacer una copia del token completo
    Token result = create_token();    result.type = next_token.type;
    result.position = next_token.position;
    result.line = next_token.line;
    result.column = next_token.column;
    
    if (next_token.value) {
        result.value = strdup(next_token.value);
    }
    
    // Restaurar estado
    lexer->position = saved_position;
    lexer->line = saved_line;
    lexer->column = saved_column;
    
    // Liberar el token actual
    if (lexer->current_token.value) {
        free(lexer->current_token.value);
    }
    
    // Restaurar el token original
    lexer->current_token = saved_token;
    
    return result;
}

// Verificar si el lexer está en estado de error
int lexer_has_error(Lexer* lexer) {
    return lexer->error_message != NULL;
}

// Obtener el mensaje de error
const char* lexer_get_error(Lexer* lexer) {
    return lexer->error_message;
}

// Convertir tipo de token a cadena (para depuración)
const char* token_type_to_string(TokenType type) {
    switch (type) {
        case TOKEN_KEYWORD: return "KEYWORD";
        case TOKEN_IDENTIFIER: return "IDENTIFIER";
        case TOKEN_STRING: return "STRING";
        case TOKEN_INTEGER: return "INTEGER";
        case TOKEN_FLOAT: return "FLOAT";
        case TOKEN_OPERATOR: return "OPERATOR";
        case TOKEN_PUNCTUATION: return "PUNCTUATION";
        case TOKEN_EOF: return "EOF";
        case TOKEN_ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

void token_free(Token* token) {
    if (token && token->value) {
        free(token->value);
        token->value = NULL;
    }
}