#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include <stdlib.h>

// Tipos de tokens
typedef enum {
    TOKEN_KEYWORD,        // Palabras reservadas (SELECT, FROM, etc.)
    TOKEN_IDENTIFIER,     // Identificadores (nombres de tablas, columnas)
    TOKEN_STRING,         // Literales de cadena ("texto")
    TOKEN_INTEGER,        // Literales enteros (123)
    TOKEN_FLOAT,          // Literales con punto flotante (123.45)
    TOKEN_OPERATOR,       // Operadores (+, -, *, /, =, etc.)
    TOKEN_PUNCTUATION,    // Puntuación (paréntesis, comas, etc.)
    TOKEN_EOF,            // Fin de entrada
    TOKEN_ERROR           // Error léxico
} TokenType;

// Estructura para almacenar un token
typedef struct {
    TokenType type;       // Tipo de token
    char* value;          // Valor del token como cadena
    int line;             // Línea donde se encontró el token
    int column;           // Columna donde se encontró el token
    int position;         // Posición absoluta en la cadena de entrada
} Token;

// Estructura principal del lexer
typedef struct {
    const char* input;    // Texto SQL completo
    int position;         // Posición actual en el texto
    int line;             // Línea actual
    int column;           // Columna actual
    Token current_token;  // Token actual
    char* error_message;  // Mensaje de error
} Lexer;

// Función para crear un nuevo lexer
Lexer* lexer_create(const char* input);

// Función para liberar un lexer
void lexer_free(Lexer* lexer);

// Funcion para liberar un token
void token_free(Token* token);

// Función para obtener el siguiente token
Token lexer_next_token(Lexer* lexer);

// Función para ver el siguiente token sin consumirlo
Token lexer_peek_token(Lexer* lexer);

// Función para comprobar si el lexer está en un estado de error
int lexer_has_error(Lexer* lexer);

// Función para obtener un mensaje de error
const char* lexer_get_error(Lexer* lexer);

// Función para convertir un tipo de token a cadena (para depuración)
const char* token_type_to_string(TokenType type);

#endif /* LEXER_H */