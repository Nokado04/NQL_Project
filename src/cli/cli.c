#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cli.h"
#include "input_handler.h"
#include "commands/cmd_registry.h"

// Inicializa la CLI
void cli_init() {
    input_init();
    cmd_registry_init();
}

// Ejecuta el bucle principal
void cli_run() {
    char *input;
    char command[MAX_COMMAND_LENGTH];
    char *args[MAX_ARGS] = {NULL}; 
    int arg_count = 0;
    
    while (1) {
        // Leer entrada del usuario
        input = input_read("NQL> ");
        
        if (!input) {
            printf("\n");
            break;
        }
        
        if (strlen(input) > 0) {
            input_add_history(input);
        }
        
        // Parsear la entrada
        input_parse(input, command, args, &arg_count);
        
        // Ejecutar el comando
        if (strlen(command) > 0) {
            if (strcmp(command, "exit") == 0) {
                printf("Goodbye!\n");
                free(input);
                break;
            } else {
                cmd_execute(command, args, arg_count);
            }
        }
        
        free(input);
    }
}

void cli_cleanup() {
    input_cleanup();
    cmd_registry_cleanup();
}