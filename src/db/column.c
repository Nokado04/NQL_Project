#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "column.h"
#include "table.h" 

// Función para obtener el índice de una columna por su nombre
int column_get_index(Table* table, const char* column_name) {
    if (!table || !column_name) return -1;
    
    for (int i = 0; i < table->num_columns; i++) {
        if (strcmp(table->columns[i].name, column_name) == 0) {
            return i;
        }
    }
    
    return -1; // No se encontró la columna
}

// Función para obtener el tipo de dato de una columna como cadena
const char* column_type_to_string(DataType type, int max_length) {
    static char buffer[20];
    
    switch (type) {
        case TYPE_INT:
            return "INT";
        case TYPE_FLOAT:
            return "FLOAT";
        case TYPE_STRING:
            sprintf(buffer, "STRING(%d)", max_length);
            return buffer;
        case TYPE_BOOL:
            return "BOOL";
        default:
            return "UNKNOWN";
    }
}