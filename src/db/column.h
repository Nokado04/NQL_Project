#ifndef COLUMN_H
#define COLUMN_H

#include "value.h"

// Forward declaration
struct Table;

// Estructura para definir una columna
typedef struct {
    char* name;
    DataType type;
    int max_length;  // For strings
    int is_primary_key;
    int allows_null;
} Column;

// Función para obtener el tipo de dato de una columna como cadena
const char* column_type_to_string(DataType type, int max_length);

// Obtener el índice de una columna por su nombre 
// Cambiamos para usar forward declaration
int column_get_index(struct Table* table, const char* column_name);

#endif