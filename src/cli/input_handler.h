#ifndef INPUT_HANDLER_H
#define INPUT_HANDLER_H

#define MAX_COMMAND_LENGTH 100
#define MAX_INPUT_LENGTH 1024
#define MAX_ARGS 50

// Inicializa el sistema de entrada
void input_init();

// Lee una línea de entrada
char *input_read(const char *prompt);

// Añade una entrada al historial
void input_add_history(const char *line);

// Función de autocompletado para readline
char **input_completion(const char *text, int start, int end);

// Función generadora para autocompletado
char *input_command_generator(const char *text, int state);

// Parsea la entrada en comando y argumentos
void input_parse(char *input, char *command, char *args[], int *arg_count);

// Limpia recursos del sistema de entrada
void input_cleanup();

#endif
