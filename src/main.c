#include <stdio.h>
#include "cli/cli.h"
#include "db/database.h"

int main(){
    // Inicializar la base de datos
    db_init();
    
    // Inicializar la interfaz de línea de comandos
    cli_init();
    
    // Mostrar mensaje de bienvenida
    printf("NQL Database CLI - Type 'help' for commands, 'exit' to quit\n");
    
    // Ejecutar el bucle principal de la CLI
    cli_run();
    
    // Limpieza al salir
    cli_cleanup();
    db_cleanup();
    return 0;
}