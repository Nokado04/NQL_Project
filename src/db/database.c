#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "database.h"

// Variables globales
static Table* tables[MAX_TABLES];
static int num_tables = 0;

// Inicializa la base de datos
void db_init() {
    // Inicializar a NULL todas las tablas
    for (int i = 0; i < MAX_TABLES; i++) {
        tables[i] = NULL;
    }
    num_tables = 0;
}

// Limpia los recursos de la base de datos
void db_cleanup() {
    // Liberar todas las tablas existentes
    for (int i = 0; i < num_tables; i++) {
        if (tables[i]) {
            table_free(tables[i]);
            tables[i] = NULL;
        }
    }
    num_tables = 0;
}

// Crea una nueva tabla
Table *db_create_table(const char *name) {
    // Verificar límite de tablas
    if (num_tables >= MAX_TABLES) {
        printf("Error: Se ha alcanzado el límite máximo de tablas (%d)\n", MAX_TABLES);
        return NULL;
    }
    
    // Verificar si ya existe una tabla con ese nombre
    for (int i = 0; i < num_tables; i++) {
        if (tables[i] && strcmp(tables[i]->name, name) == 0) {
            printf("Error: Ya existe una tabla con el nombre '%s'\n", name);
            return NULL;
        }
    }
    
    // Crear la tabla
    Table *table = table_create(name);
    if (!table) {
        printf("Error: No se pudo crear la tabla '%s'\n", name);
        return NULL;
    }
    
    // Añadir a la lista de tablas
    tables[num_tables++] = table;
    
    return table;
}

// Busca una tabla por nombre
Table *db_find_table(const char *name) {
    for (int i = 0; i < num_tables; i++) {
        if (tables[i] && strcmp(tables[i]->name, name) == 0) {
            return tables[i];
        }
    }
    return NULL;
}

// Elimina una tabla
int db_drop_table(const char *name) {
    for (int i = 0; i < num_tables; i++) {
        if (tables[i] && strcmp(tables[i]->name, name) == 0) {
            // Liberar la tabla
            table_free(tables[i]);
            
            // Compactar el array moviendo las tablas restantes
            for (int j = i; j < num_tables - 1; j++) {
                tables[j] = tables[j + 1];
            }
            tables[num_tables - 1] = NULL;
            num_tables--;
            
            return 0;
        }
    }
    
    return -1; // Tabla no encontrada
}

// Obtiene la lista de tablas
char **db_get_table_names(int *count) {
    if (!count) return NULL;
    
    // No hay tablas
    if (num_tables == 0) {
        *count = 0;
        return NULL;
    }
    
    // Crear array para los nombres
    char **names = (char**)malloc(num_tables * sizeof(char*));
    if (!names) {
        *count = 0;
        return NULL;
    }
    
    // Copiar los nombres
    for (int i = 0; i < num_tables; i++) {
        if (tables[i] && tables[i]->name) {
            names[i] = strdup(tables[i]->name);
        } else {
            names[i] = strdup(""); // Nombre vacío por seguridad
        }
    }
    
    *count = num_tables;
    return names;
}