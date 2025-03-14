#ifndef STRING_UTILS_H
#define STRING_UTILS_H

/**
 * Elimina espacios en blanco al principio y final de una cadena
 * @param str Cadena a procesar (se modifica in-place)
 * @return Puntero a la cadena modificada
 */
char *string_trim(char *str);

/**
 * Convierte una cadena a minúsculas
 * @param str Cadena a convertir (se modifica in-place)
 * @return Puntero a la cadena modificada
 */
char *string_to_lower(char *str);

/**
 * Compara dos cadenas sin distinguir mayúsculas/minúsculas
 * @param str1 Primera cadena
 * @param str2 Segunda cadena
 * @return 0 si son iguales, <0 si str1<str2, >0 si str1>str2
 */
int string_compare_case_insensitive(const char *str1, const char *str2);

/**
 * Verifica si una cadena comienza con un prefijo específico (case insensitive)
 * @param str Cadena a verificar
 * @param prefix Prefijo a buscar
 * @return 1 si la cadena comienza con el prefijo, 0 en caso contrario
 */
int string_starts_with(const char *str, const char *prefix);

/**
 * Duplica una subcadena
 * @param str Cadena origen
 * @param start Índice de inicio
 * @param length Longitud de la subcadena (-1 para hasta el final)
 * @return Nueva cadena (debe liberarse con free)
 */
char *string_substring(const char *str, int start, int length);

/**
 * Divide una cadena en tokens respetando las comillas
 * @param str Cadena a dividir (se modifica)
 * @param delimiters Caracteres delimitadores
 * @param tokens Array donde se almacenarán los tokens
 * @param max_tokens Número máximo de tokens a extraer
 * @return Número de tokens encontrados
 */
int string_tokenize_quoted(char *str, const char *delimiters, char **tokens, int max_tokens);

#endif /* STRING_UTILS_H */