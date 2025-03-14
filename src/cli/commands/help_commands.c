#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../cli.h"
#include "cmd_registry.h"

// Comando help - Muestra la ayuda general o específica
int cmd_help(char *args[], int arg_count) {
    if (arg_count > 0) {
        // Buscar ayuda específica para el comando
        const CommandEntry *entry = cmd_get_entry(args[0]);
        if (entry && entry->help_text) {
            printf("%s\n", entry->help_text);
            return 0;
        }
        
        // Si no hay ayuda específica para ese comando exacto, 
        // verificamos comandos SQL que pueden estar separados
        if (strcasecmp(args[0], "CREATE") == 0 && arg_count > 1 && 
            strcasecmp(args[1], "TABLE") == 0) {
            entry = cmd_get_entry("CREATE TABLE");
            if (entry && entry->help_text) {
                printf("%s\n", entry->help_text);
                return 0;
            }
        } else if (strcasecmp(args[0], "ALTER") == 0 && arg_count > 1 && 
                   strcasecmp(args[1], "TABLE") == 0) {
            entry = cmd_get_entry("ALTER TABLE");
            if (entry && entry->help_text) {
                printf("%s\n", entry->help_text);
                return 0;
            }
        } else if (strcasecmp(args[0], "INSERT") == 0 && arg_count > 1 && 
                   strcasecmp(args[1], "INTO") == 0) {
            entry = cmd_get_entry("INSERT INTO");
            if (entry && entry->help_text) {
                printf("%s\n", entry->help_text);
                return 0;
            }
        } else if (strcasecmp(args[0], "DELETE") == 0 && arg_count > 1 && 
                   strcasecmp(args[1], "FROM") == 0) {
            entry = cmd_get_entry("DELETE FROM");
            if (entry && entry->help_text) {
                printf("%s\n", entry->help_text);
                return 0;
            }
        } else if (strcasecmp(args[0], "UPDATE") == 0) {
            entry = cmd_get_entry("UPDATE");
            if (entry && entry->help_text) {
                printf("%s\n", entry->help_text);
                return 0;
            }
        } else if (strcasecmp(args[0], "COUNT") == 0 && arg_count > 1 && 
                   strcasecmp(args[1], "FROM") == 0) {
            entry = cmd_get_entry("COUNT");
            if (entry && entry->help_text) {
                printf("%s\n", entry->help_text);
                return 0;
            }
        } else if (strcasecmp(args[0], "add") == 0 || 
                   strcasecmp(args[0], "subtract") == 0 || 
                   strcasecmp(args[0], "multiply") == 0) {
            entry = cmd_get_entry(args[0]);
            if (entry && entry->help_text) {
                printf("%s\n", entry->help_text);
                return 0;
            }
        }
        
        // Mostrar mensaje genérico si no hay ayuda específica
        printf("No hay ayuda disponible para '%s'\n", args[0]);
        return 0;
    }
    
    // Mostrar ayuda general
    printf("\n══════════════════════════════════════════════════\n");
    printf("             NQL Database CLI - Ayuda              \n");
    printf("══════════════════════════════════════════════════\n\n");
    
    printf("--- Comandos Generales ---\n");
    printf("  help                   - Muestra esta ayuda\n");
    printf("  help [comando]         - Muestra ayuda específica sobre un comando\n");
    printf("  exit                   - Salir del programa\n");
    printf("  clear                  - Limpiar pantalla\n\n");
    
    printf("--- Comandos SQL ---\n");
    printf("  CREATE TABLE nombre    - Crea una nueva tabla\n");
    printf("  ALTER TABLE tabla ADD COLUMN col tipo [opciones] - Añade columna\n");
    printf("  INSERT INTO tabla VALUES (val1, val2, ...)       - Inserta datos\n");
    printf("  SELECT * FROM tabla    - Muestra todos los datos de una tabla\n");
    printf("  DESCRIBE tabla         - Muestra la estructura de una tabla\n");
    printf("  DELETE FROM tabla WHERE rowid = id - Elimina filas\n");
    printf("  UPDATE tabla SET col = valor WHERE rowid = id    - Actualiza datos\n");
    printf("  COUNT FROM tabla       - Cuenta los registros de una tabla\n\n");
    
    printf("--- Comandos Utilitarios ---\n");
    printf("  add n1 n2 [n3 ...]     - Suma números\n");
    printf("  subtract n1 n2 [n3 ...] - Resta números\n");
    printf("  multiply n1 n2 [n3 ...] - Multiplica números\n\n");
    
    printf("--- Tipos de datos disponibles ---\n");
    printf("  INT                    - Números enteros\n");
    printf("  FLOAT                  - Números decimales\n");
    printf("  STRING(longitud)       - Texto (especificar longitud máxima)\n");
    printf("  BOOL                   - Valores verdadero/falso\n\n");
    
    printf("--- Opciones de columna ---\n");
    printf("  PRIMARY KEY            - Define la columna como clave primaria\n");
    printf("  NOT NULL               - No permite valores nulos\n\n");
    
    printf("--- Tutorial rápido: Tabla de usuarios ---\n");
    printf("  1. CREATE TABLE usuarios\n");
    printf("  2. ALTER TABLE usuarios ADD COLUMN id INT PRIMARY KEY NOT NULL\n");
    printf("  3. ALTER TABLE usuarios ADD COLUMN nombre STRING(50) NOT NULL\n");
    printf("  4. ALTER TABLE usuarios ADD COLUMN edad INT\n");
    printf("  5. ALTER TABLE usuarios ADD COLUMN genero STRING(1)\n");
    printf("  6. INSERT INTO usuarios VALUES (1, \"Juan\", 25, \"M\")\n");
    printf("  7. INSERT INTO usuarios VALUES (2, \"Ana\", 30, \"F\")\n");
    printf("  8. SELECT * FROM usuarios\n");
    printf("  9. DESCRIBE usuarios\n\n");
    
    return 0;
}

// Comando exit - Sale del programa
int cmd_exit(char *args[], int arg_count) {
    // Este comando es manejado directamente en cli_run()
    return 0;
}

// Comando clear - Limpia la pantalla
int cmd_clear(char *args[], int arg_count) {
    system("clear");
    return 0;
}