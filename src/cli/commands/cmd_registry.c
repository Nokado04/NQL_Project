#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../cli.h"
#include "cmd_registry.h"

// Declaraciones de funciones de comandos
// Comandos de ayuda
int cmd_help(char *args[], int arg_count);
int cmd_exit(char *args[], int arg_count);
int cmd_clear(char *args[], int arg_count);

// Comandos de tabla
int cmd_create_table(char *args[], int arg_count);
int cmd_alter_table(char *args[], int arg_count);
int cmd_describe(char *args[], int arg_count);

// Comandos de datos
int cmd_insert(char *args[], int arg_count);
int cmd_select(char *args[], int arg_count);
int cmd_delete(char *args[], int arg_count);
int cmd_update(char *args[], int arg_count);
int cmd_count(char *args[], int arg_count);

// Comandos utilitarios
int cmd_add(char *args[], int arg_count);
int cmd_subtract(char *args[], int arg_count);
int cmd_multiply(char *args[], int arg_count);

// Textos de ayuda detallados
static const char *help_create_table = 
    "\n══════════ Ayuda: CREATE TABLE ══════════\n\n"
    "Sintaxis: CREATE TABLE nombre_tabla\n\n"
    "Función: Crea una nueva tabla en la base de datos.\n\n"
    "Ejemplo:\n"
    "  NQL> CREATE TABLE usuarios\n"
    "  Tabla creada: usuarios\n\n"
    "Después de crear la tabla, use ALTER TABLE para añadir columnas.";

static const char *help_alter_table = 
    "\n══════════ Ayuda: ALTER TABLE ══════════\n\n"
    "Sintaxis: ALTER TABLE nombre_tabla ADD COLUMN nombre_columna tipo [opciones]\n\n"
    "Función: Añade una columna a una tabla existente.\n\n"
    "Tipos disponibles:\n"
    "  - INT              (números enteros)\n"
    "  - FLOAT            (números decimales)\n"
    "  - STRING(longitud) (texto, especifique longitud máxima)\n"
    "  - BOOL             (valores verdadero/falso)\n\n"
    "Opciones:\n"
    "  - PRIMARY KEY      (define la columna como clave primaria)\n"
    "  - NOT NULL         (no permite valores nulos)\n\n"
    "Ejemplos:\n"
    "  NQL> ALTER TABLE usuarios ADD COLUMN id INT PRIMARY KEY NOT NULL\n"
    "  Column added: id (INT)\n\n"
    "  NQL> ALTER TABLE usuarios ADD COLUMN nombre STRING(50) NOT NULL\n"
    "  Column added: nombre (STRING(50))";

static const char *help_insert = 
    "\n══════════ Ayuda: INSERT INTO ══════════\n\n"
    "Sintaxis: INSERT INTO nombre_tabla VALUES (valor1, valor2, ...)\n\n"
    "Función: Inserta una nueva fila en la tabla con los valores especificados.\n\n"
    "Notas:\n"
    "  - Los valores de texto deben ir entre comillas dobles.\n"
    "  - El número de valores debe coincidir con el número de columnas.\n"
    "  - Los valores deben estar en el mismo orden que las columnas.\n\n"
    "Ejemplo:\n"
    "  NQL> INSERT INTO usuarios VALUES (1, \"Juan Pérez\", 25, \"M\")\n"
    "  1 fila insertada en usuarios\n\n"
    "Para ver qué columnas tiene una tabla, use: DESCRIBE nombre_tabla";

static const char *help_select = 
    "\n══════════ Ayuda: SELECT ══════════\n\n"
    "Sintaxis: SELECT * FROM nombre_tabla\n\n"
    "Función: Muestra todos los datos almacenados en una tabla.\n\n"
    "Nota: Actualmente solo se admite SELECT * (todas las columnas).\n\n"
    "Ejemplo:\n"
    "  NQL> SELECT * FROM usuarios\n"
    "  +----+------------+-----+--------+\n"
    "  | id | nombre     | edad| genero |\n"
    "  +----+------------+-----+--------+\n"
    "  | 1  | Juan Pérez | 25  | M      |\n"
    "  | 2  | Ana López  | 30  | F      |\n"
    "  +----+------------+-----+--------+\n"
    "  2 filas en total";

static const char *help_delete = 
    "\n══════════ Ayuda: DELETE FROM ══════════\n\n"
    "Sintaxis: DELETE FROM nombre_tabla WHERE rowid = índice\n\n"
    "Función: Elimina una fila específica de la tabla.\n\n"
    "Nota: Actualmente solo se permite eliminar por índice de fila.\n\n"
    "Ejemplo:\n"
    "  NQL> DELETE FROM usuarios WHERE rowid = 1\n"
    "  1 fila eliminada de usuarios";

static const char *help_describe = 
    "\n══════════ Ayuda: DESCRIBE ══════════\n\n"
    "Sintaxis: DESCRIBE nombre_tabla\n\n"
    "Función: Muestra la estructura de una tabla (columnas, tipos, etc).\n\n"
    "Ejemplo:\n"
    "  NQL> DESCRIBE usuarios\n"
    "  Tabla: usuarios\n"
    "  +------------+--------------+------------+-------------+\n"
    "  | Campo      | Tipo         | Nulo       | Clave       |\n"
    "  +------------+--------------+------------+-------------+\n"
    "  | id         | INT          | NO         | PRIMARIA    |\n"
    "  | nombre     | STRING(50)   | NO         |             |\n"
    "  | edad       | INT          | SI         |             |\n"
    "  | genero     | STRING(1)    | SI         |             |\n"
    "  +------------+--------------+------------+-------------+\n"
    "  4 columnas en tabla";

static const char *help_update = 
    "\n══════════ Ayuda: UPDATE ══════════\n\n"
    "Sintaxis: UPDATE nombre_tabla SET columna = valor WHERE rowid = índice\n\n"
    "Función: Actualiza el valor de una celda específica en una tabla.\n\n"
    "Nota: Actualmente solo se permite actualizar por índice de fila.\n\n"
    "Ejemplo:\n"
    "  NQL> UPDATE usuarios SET edad = 26 WHERE rowid = 0\n"
    "  1 fila actualizada en usuarios";

static const char *help_count = 
    "\n══════════ Ayuda: COUNT ══════════\n\n"
    "Sintaxis: COUNT FROM nombre_tabla\n\n"
    "Función: Cuenta el número de filas en una tabla.\n\n"
    "Ejemplo:\n"
    "  NQL> COUNT FROM usuarios\n"
    "  Cantidad de registros en usuarios: 2";

static const char *help_utils = 
    "\n══════════ Ayuda: Comandos Utilitarios ══════════\n\n"
    "NQL incluye algunos comandos utilitarios básicos:\n\n"
    "  add número1 número2 [número3 ...]\n"
    "    Suma los números proporcionados\n"
    "    Ejemplo: add 5 10 15  ->  Resultado: 30\n\n"
    "  subtract número1 número2 [número3 ...]\n"
    "    Resta los números del primero\n"
    "    Ejemplo: subtract 20 5 3  ->  Resultado: 12\n\n"
    "  multiply número1 número2 [número3 ...]\n"
    "    Multiplica los números proporcionados\n"
    "    Ejemplo: multiply 2 3 4  ->  Resultado: 24";

#define MAX_COMMANDS 30
static CommandEntry commands[MAX_COMMANDS];
static int num_commands = 0;

// Registro de comandos disponibles
static void initialize_commands() {
    // Comandos generales
    commands[num_commands++] = (CommandEntry){"help", cmd_help, "Muestra ayuda sobre los comandos disponibles", "Utilice 'help [comando]' para obtener ayuda específica sobre un comando"};
    commands[num_commands++] = (CommandEntry){"exit", cmd_exit, "Sale del programa", "Utilice este comando para salir de la aplicación y volver al shell"};
    commands[num_commands++] = (CommandEntry){"clear", cmd_clear, "Limpia la pantalla", "Borra todo el contenido de la pantalla para una mejor legibilidad"};
    
    // Comandos SQL
    commands[num_commands++] = (CommandEntry){"CREATE TABLE", cmd_create_table, "Crea una nueva tabla", help_create_table};
    commands[num_commands++] = (CommandEntry){"ALTER TABLE", cmd_alter_table, "Modifica una tabla existente", help_alter_table};
    commands[num_commands++] = (CommandEntry){"INSERT INTO", cmd_insert, "Inserta datos en una tabla", help_insert};
    commands[num_commands++] = (CommandEntry){"SELECT", cmd_select, "Consulta datos de una tabla", help_select};
    commands[num_commands++] = (CommandEntry){"DELETE FROM", cmd_delete, "Elimina datos de una tabla", help_delete};
    commands[num_commands++] = (CommandEntry){"DESCRIBE", cmd_describe, "Muestra la estructura de una tabla", help_describe};
    commands[num_commands++] = (CommandEntry){"UPDATE", cmd_update, "Actualiza datos en una tabla", help_update};
    commands[num_commands++] = (CommandEntry){"COUNT", cmd_count, "Cuenta registros en una tabla", help_count};
    
    // Comandos utilitarios
    commands[num_commands++] = (CommandEntry){"add", cmd_add, "Suma números", help_utils};
    commands[num_commands++] = (CommandEntry){"subtract", cmd_subtract, "Resta números", help_utils};
    commands[num_commands++] = (CommandEntry){"multiply", cmd_multiply, "Multiplica números", help_utils};
    
    // Comandos alternativos (para compatibilidad)
    commands[num_commands++] = (CommandEntry){"create_table", cmd_create_table, "Crea una nueva tabla", help_create_table};
    commands[num_commands++] = (CommandEntry){"alter_table", cmd_alter_table, "Modifica una tabla existente", help_alter_table};
    commands[num_commands++] = (CommandEntry){"insert", cmd_insert, "Inserta datos en una tabla", help_insert};
    commands[num_commands++] = (CommandEntry){"select", cmd_select, "Consulta datos de una tabla", help_select};
    commands[num_commands++] = (CommandEntry){"delete", cmd_delete, "Elimina datos de una tabla", help_delete};
    commands[num_commands++] = (CommandEntry){"describe", cmd_describe, "Muestra la estructura de una tabla", help_describe};
    commands[num_commands++] = (CommandEntry){"update", cmd_update, "Actualiza datos en una tabla", help_update};
    commands[num_commands++] = (CommandEntry){"count", cmd_count, "Cuenta registros en una tabla"};
    
    // Marca de fin de lista
    commands[num_commands++] = (CommandEntry){NULL, NULL, NULL, NULL};
}

// Array con solo los nombres de los comandos (para autocompletado)
static const char **command_names = NULL;

void cmd_registry_init() {
    // Inicializar el array de comandos
    initialize_commands();
    
    // Contar comandos para dimensionar el array (num_commands - 1 porque excluimos el NULL)
    int count = num_commands - 1;
    
    // Crear y llenar array de nombres
    command_names = (const char **)malloc((count + 1) * sizeof(const char *));
    if (command_names) {
        int i;
        for (i = 0; i < count; i++) {
            command_names[i] = commands[i].name;
        }
        command_names[i] = NULL; // Terminar con NULL
    }
}

void cmd_registry_cleanup() {
    if (command_names) {
        free(command_names);
        command_names = NULL;
    }
}

int cmd_execute(const char *command, char *args[], int arg_count) {
    if (!command) return -1;
    
    // Buscar el comando en el registro
    for (int i = 0; commands[i].name != NULL; i++) {
        if (strcasecmp(command, commands[i].name) == 0) {
            // Encontrado, ejecutar la función asociada
            if (commands[i].function) {
                return commands[i].function(args, arg_count);
            }
            break;
        }
    }
    // Comando no encontrado
    printf("Comando desconocido: %s\n", command);
    printf("Escriba 'help' para ver los comandos disponibles.\n");
    return -1;
}

const char **cmd_get_command_names() {
    return command_names;
}

const CommandEntry *cmd_get_entry(const char *command) {
    if (!command) return NULL;
    
    for (int i = 0; commands[i].name != NULL; i++) {
        if (strcasecmp(command, commands[i].name) == 0) {
            return &commands[i];
        }
    }
    
    return NULL;
}