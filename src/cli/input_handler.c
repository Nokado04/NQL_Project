#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "input_handler.h"
#include "commands/cmd_registry.h"

// Inicializa el sistema de entrada
void input_init() {
    // Configurar readline para autocompletado
    rl_attempted_completion_function = input_completion;
}

// Lee una línea de entrada
char *input_read(const char *prompt) {
    // Usa readline para obtener entrada con autocompletado
    return readline(prompt);
}

// Añade una entrada al historial
void input_add_history(const char *line) {
    if (line && *line) {
        add_history(line);
    }
}

// Función generadora para autocompletado de comandos
char *input_command_generator(const char *text, int state) {
    static int list_index;
    static const char **commands;
    
    // En el primer llamado, inicializar el generador
    if (!state) {
        list_index = 0;
        commands = cmd_get_command_names();
    }
    
    // Si no hay comandos disponibles
    if (!commands) return NULL;
    
    // Devolver coincidencias sucesivas
    while (commands[list_index]) {
        const char *name = commands[list_index++];
        
        if (name && strncasecmp(name, text, strlen(text)) == 0) {
            return strdup(name); // readline liberará este string
        }
    }
    
    return NULL; // No más coincidencias
}

// Función principal de autocompletado
char **input_completion(const char *text, int start, int end) {
    // Si estamos al inicio de la línea, completar con nombres de comandos
    if (start == 0) {
        return rl_completion_matches(text, input_command_generator);
    }
    
    // De lo contrario, usar el comportamiento por defecto
    rl_attempted_completion_over = 0;
    return NULL;
}

// Función para limpiar los argumentos liberando memoria si es necesario
void input_cleanup_args(char *args[], int *arg_count) {
    if (!args) return;
    
    for (int i = 0; i < MAX_ARGS; i++) {
        // Verificar que el puntero no sea NULL y que sea una dirección de memoria válida
        // (Esto no es perfecto pero ayudará a evitar los errores más obvios)
        if (args[i] != NULL && args[i] > (char*)0x1000) {
            free(args[i]);
            args[i] = NULL;
        } else {
            // Si el puntero no parece válido, simplemente marcarlo como NULL sin liberar
            args[i] = NULL;
        }
    }
    
    if (arg_count) *arg_count = 0;
}

// Versión mejorada de input_parse
void input_parse(char *input, char *command, char *args[], int *arg_count) {
    // Inicializar valores por defecto
    command[0] = '\0';
    
    // NO limpiar los argumentos automáticamente
    // input_cleanup_args(args, arg_count);
    
    // Simplemente inicializar el contador de argumentos
    *arg_count = 0;
    
    // Verificar entrada válida
    if (!input || !*input) {
        return;
    }
    
    // Hacer una copia de la entrada para no modificar el original
    char *input_copy = strdup(input);
    if (!input_copy) {
        return;
    }
    
    // Identificar primero el comando (CREATE, SELECT, etc.)
    char *token = strtok(input_copy, " \t");
    if (!token) {
        free(input_copy);
        return;
    }
    
    // Para comandos SQL comunes, capturar la primera o primeras dos palabras como comando
    if (strcasecmp(token, "CREATE") == 0 || 
        strcasecmp(token, "ALTER") == 0 || 
        strcasecmp(token, "INSERT") == 0 || 
        strcasecmp(token, "SELECT") == 0 || 
        strcasecmp(token, "DELETE") == 0 ||
        strcasecmp(token, "DESCRIBE") == 0) {
        
        char first_word[MAX_COMMAND_LENGTH];
        strncpy(first_word, token, MAX_COMMAND_LENGTH - 1);
        first_word[MAX_COMMAND_LENGTH - 1] = '\0';
        
        // Capturar segunda palabra para comandos de dos palabras
        token = strtok(NULL, " \t");
        if (token && (
            (strcasecmp(first_word, "CREATE") == 0 && strcasecmp(token, "TABLE") == 0) ||
            (strcasecmp(first_word, "ALTER") == 0 && strcasecmp(token, "TABLE") == 0) ||
            (strcasecmp(first_word, "INSERT") == 0 && strcasecmp(token, "INTO") == 0) ||
            (strcasecmp(first_word, "DELETE") == 0 && strcasecmp(token, "FROM") == 0)
           )) {
            
            size_t first_len = strlen(first_word);
            size_t token_len = strlen(token);
            if (first_len + 1 + token_len >= MAX_COMMAND_LENGTH) {
                // Copia solo lo que quepa
                strncpy(command, first_word, MAX_COMMAND_LENGTH - 1);
                command[MAX_COMMAND_LENGTH - 1] = '\0';
            } else {
                // Hay suficiente espacio
                sprintf(command, "%s %s", first_word, token);
            }
            token = strtok(NULL, " \t");
        } else {
            strncpy(command, first_word, MAX_COMMAND_LENGTH - 1);
            command[MAX_COMMAND_LENGTH - 1] = '\0';
        }
    } else {
        strncpy(command, token, MAX_COMMAND_LENGTH - 1);
        command[MAX_COMMAND_LENGTH - 1] = '\0';
        token = strtok(NULL, " \t");
    }
    
    // Procesar argumentos con manejo de comillas
    char buffer[MAX_INPUT_LENGTH] = "";
    int in_quotes = 0;
    
    while (token && *arg_count < MAX_ARGS) {
        // Verificar si comienza con comilla
        if (!in_quotes && token[0] == '"') {
            in_quotes = 1;
            strcpy(buffer, token + 1);  // Copiar sin la comilla inicial
            
            // Si termina con comilla en el mismo token
            size_t len = strlen(buffer);
            if (len > 0 && buffer[len - 1] == '"') {
                buffer[len - 1] = '\0';  // Quitar comilla final
                in_quotes = 0;
                args[(*arg_count)++] = strdup(buffer);
                buffer[0] = '\0';
            }
        }
        // Si ya estamos procesando un texto con comillas
        else if (in_quotes) {
            size_t len = strlen(buffer);
            buffer[len] = ' ';  // Añadir espacio entre tokens
            strcpy(buffer + len + 1, token);
            
            // Verificar si termina con comilla
            len = strlen(buffer);
            if (len > 0 && buffer[len - 1] == '"') {
                buffer[len - 1] = '\0';  // Quitar comilla final
                in_quotes = 0;
                args[(*arg_count)++] = strdup(buffer);
                buffer[0] = '\0';
            }
        }
        // Token normal sin comillas
        else {
            args[(*arg_count)++] = strdup(token);
        }
        
        token = strtok(NULL, " \t");
    }
    
    // Si quedó un texto con comillas sin cerrar, lo agregamos igual
    if (in_quotes && buffer[0]) {
        args[(*arg_count)++] = strdup(buffer);
    }
    
    free(input_copy);
}

// Limpia recursos del sistema de entrada
void input_cleanup() {
    // Limpiar historial de readline
    clear_history();
}