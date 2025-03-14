#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "row.h"
#include "table.h" 

// Función para buscar una fila por valor de clave primaria
int row_find_by_primary_key(Table* table, Value key_value, int* row_index) {
    if (!table || !row_index) return -1;
    
    // Buscar la columna de clave primaria
    int pk_col = -1;
    for (int i = 0; i < table->num_columns; i++) {
        if (table->columns[i].is_primary_key) {
            pk_col = i;
            break;
        }
    }
    
    // Si no hay columna de clave primaria
    if (pk_col == -1) return -1;
    
    // Buscar la fila con el valor de clave primaria
    for (int i = 0; i < table->num_rows; i++) {
        Value row_value = table->rows[i].values[pk_col];
        int match = 0;
        
        // Comparar según el tipo de dato
        switch (table->columns[pk_col].type) {
            case TYPE_INT:
                match = (row_value.int_val == key_value.int_val);
                break;
            case TYPE_FLOAT:
                match = (row_value.float_val == key_value.float_val);
                break;
            case TYPE_STRING:
                if (row_value.string_val && key_value.string_val) {
                    match = (strcmp(row_value.string_val, key_value.string_val) == 0);
                }
                break;
            case TYPE_BOOL:
                match = (row_value.bool_val == key_value.bool_val);
                break;
        }
        
        if (match) {
            *row_index = i;
            return 0;
        }
    }
    
    return -1; // No se encontró la fila
}