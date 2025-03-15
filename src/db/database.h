#ifndef DATABASE_H
#define DATABASE_H

#include "table.h"

// Número máximo de tablas
#define MAX_TABLES 100

// Estructura de la base de datos
typedef struct {
    Table **tables;
    int num_tables;
    char *name;	
    int max_tables;
} Database;


// Inicializa la base de datos
void db_init();

// Limpia los recursos de la base de datos
void db_cleanup();

// Crea una nueva tabla
Table *db_create_table(const char *name);

// Busca una tabla por nombre
Table *db_find_table(const char *name);

// Elimina una tabla
int db_drop_table(const char *name);

// Obtiene la lista de tablas
char **db_get_table_names(int *count);

#endif