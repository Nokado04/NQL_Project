#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "string.utils.h"

/**
 * Elimina espacios en blanco al principio y final de una cadena
 */
char *string_trim(char *str) {
    if (!str) return NULL;
    
    // Eliminar espacios al inicio
    char *start = str;
    while (*start && isspace(*start)) {
        start++;
    }
    
    // Si era todo espacios
    if (*start == '\0') {
        *str = '\0';
        return str;
    }
    
    // Encontrar el final sin espacios
    char *end = start + strlen(start) - 1;
    while (end > start && isspace(*end)) {
        end--;
    }
    
    // Terminar la cadena después del último carácter no-espacio
    *(end + 1) = '\0';
    
    // Si start no es str, mover la cadena
    if (start != str) {
        memmove(str, start, (end - start + 2) * sizeof(char));
    }
    
    return str;
}

/**
 * Convierte una cadena a minúsculas
 */
char *string_to_lower(char *str) {
    if (!str) return NULL;
    
    char *p = str;
    while (*p) {
        *p = tolower(*p);
        p++;
    }
    
    return str;
}

/**
 * Compara dos cadenas sin distinguir mayúsculas/minúsculas
 */
int string_compare_case_insensitive(const char *str1, const char *str2) {
    if (!str1 && !str2) return 0;
    if (!str1) return -1;
    if (!str2) return 1;
    
    return strcasecmp(str1, str2);
}

/**
 * Verifica si una cadena comienza con un prefijo específico (case insensitive)
 */
int string_starts_with(const char *str, const char *prefix) {
    if (!str || !prefix) return 0;
    
    size_t len_str = strlen(str);
    size_t len_prefix = strlen(prefix);
    
    if (len_prefix > len_str) return 0;
    
    return strncasecmp(str, prefix, len_prefix) == 0;
}

/**
 * Duplica una subcadena
 */
char *string_substring(const char *str, int start, int length) {
    if (!str) return NULL;
    
    int str_len = strlen(str);
    
    // Ajustar start si es negativo
    if (start < 0) {
        start = str_len + start;
        if (start < 0) start = 0;
    }
    
    // Si start está fuera de límites
    if (start >= str_len) {
        return strdup("");
    }
    
    // Ajustar length si es necesario
    if (length < 0 || start + length > str_len) {
        length = str_len - start;
    }
    
    // Crear nueva cadena
    char *result = (char*)malloc((length + 1) * sizeof(char));
    if (!result) return NULL;
    
    strncpy(result, str + start, length);
    result[length] = '\0';
    
    return result;
}

/**
 * Divide una cadena en tokens respetando las comillas
 */
int string_tokenize_quoted(char *str, const char *delimiters, 
                          char **tokens, int max_tokens) {
    if (!str || !tokens || max_tokens <= 0) return 0;
    
    int count = 0;
    int in_quotes = 0;
    char *start = str;
    
    while (*str && count < max_tokens) {
        // Manejar comillas
        if (*str == '"') {
            in_quotes = !in_quotes;
            // Si acabamos de empezar una sección con comillas, mover el inicio
            if (in_quotes) {
                start = str + 1;
            }
            // Si acabamos de cerrar una sección con comillas, finalizar el token
            else {
                *str = '\0'; // Reemplazar comilla de cierre por nulo
                tokens[count++] = start;
                start = str + 1;
            }
        }
        // Manejar delimitadores fuera de comillas
        else if (!in_quotes && strchr(delimiters, *str)) {
            *str = '\0'; // Reemplazar delimitador por nulo
            
            // Si hay algo entre este delimitador y el último token/delimitador
            if (start != str) {
                tokens[count++] = start;
                if (count >= max_tokens) break;
            }
            
            start = str + 1;
        }
        
        str++;
    }
    
    // Añadir el último token si existe y hay espacio
    if (*start && count < max_tokens) {
        tokens[count++] = start;
    }
    
    return count;
}