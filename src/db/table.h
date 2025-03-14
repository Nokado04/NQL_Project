#ifndef TABLE_H
#define TABLE_H

#include "value.h"
#include "column.h"
#include "row.h"

// Estructura para la tabla
typedef struct Table {
    char *name;
    Column *columns;
    int num_columns;
    Row *rows;
    int num_rows;
    int capacity;
} Table;

// Crea una nueva tabla
Table *table_create(const char *name);

// Libera una tabla
void table_free(Table *table);

// Añade una columna a la tabla
int table_add_column(Table *table, const char *name, DataType type, int max_length, int is_primary_key, int allows_null);

// Añade una fila a la tabla
int table_add_row(Table *table, Value *values);

// Elimina una fila de la tabla
int table_delete_row(Table *table, int row_index);

// Imprime la tabla con formato
void table_print(Table *table);

// Imprime la tabla con formato mejorado
void table_print_formatted(Table *table);

#endif