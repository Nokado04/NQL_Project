#ifndef CMD_REGISTRY_H
#define CMD_REGISTRY_H

// Tipo de función de comando
typedef int (*CommandFunction)(char *args[], int arg_count);

// Estructura para almacenar información de un comando
typedef struct {
    const char *name;           // Nombre del comando
    CommandFunction function;   // Función que implementa el comando
    const char *description;    // Descripción corta del comando
    const char *help_text;      // Texto de ayuda extendido
} CommandEntry;

// Inicializa el registro de comandos
void cmd_registry_init();

// Limpia los recursos del registro de comandos
void cmd_registry_cleanup();

// Ejecuta un comando por su nombre
int cmd_execute(const char *command, char *args[], int arg_count);

// Obtiene una lista de los nombres de comandos disponibles
const char **cmd_get_command_names();

// Obtiene la entrada de un comando por su nombre
const CommandEntry *cmd_get_entry(const char *command);

#endif /* CMD_REGISTRY_H */