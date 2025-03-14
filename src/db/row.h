#ifndef ROW_H
#define ROW_H

#include "value.h"

// Forward declaration
struct Table;

// Estructura para la fila de una tabla
typedef struct {
    Value* values;  // Array of values, one for each column
    int is_deleted; // Soft delete flag
} Row;

// Función para buscar una fila por valor de clave primaria
int row_find_by_primary_key(struct Table* table, Value key_value, int* row_index);

#endif