#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "table.h"

/*
* Función para crear una tabla
* @param name Nombre de la tabla
* @return Puntero a la tabla creada
*/
Table* table_create(const char* name) {
    Table* table = (Table*)malloc(sizeof(Table));
    if (!table) return NULL;
    
    table->name = strdup(name);
    table->columns = NULL;
    table->num_columns = 0;
    table->rows = NULL;
    table->num_rows = 0;
    table->capacity = 0;
    
    return table;
}

/*
* Función para liberar la memoria de una tabla
* @param table Puntero a la tabla a liberar
*/
void table_free(Table* table) {
    if (!table) return;
    
    // Liberar memoria de las columnas
    if (table->columns) {
        for (int i = 0; i < table->num_columns; i++) {
            free(table->columns[i].name);
        }
        free(table->columns);
    }
    
    // Liberar memoria de las filas y sus valores
    if (table->rows) {
        for (int i = 0; i < table->num_rows; i++) {
            if (table->rows[i].values) {
                // Liberar strings en valores de tipo string
                for (int j = 0; j < table->num_columns; j++) {
                    if (table->columns[j].type == TYPE_STRING && 
                        table->rows[i].values[j].string_val) {
                        free(table->rows[i].values[j].string_val);
                    }
                }
                free(table->rows[i].values);
            }
        }
        free(table->rows);
    }
    
    free(table->name);
    free(table);
}

/*
* Función para agregar una columna a una tabla
* @param table Puntero a la tabla
* @param name Nombre de la columna
* @param type Tipo de dato de la columna
* @param max_length Longitud maxima para strings
* @param is_primary_key Indica si es clave primaria
* @param allows_null Indica si permite valores nulos
* @return 0 si se agregó correctamente, -1 si hubo un error
*/
int table_add_column(Table* table, const char* name, DataType type, 
                     int max_length, int is_primary_key, int allows_null) {
    if (!table) return -1;
    
    // Expandir el array de columnas
    Column* new_columns = (Column*)realloc(table->columns, 
                               (table->num_columns + 1) * sizeof(Column));
    if (!new_columns) return -1;
    
    table->columns = new_columns;
    
    // Inicializar la nueva columna
    table->columns[table->num_columns].name = strdup(name);
    table->columns[table->num_columns].type = type;
    table->columns[table->num_columns].max_length = max_length;
    table->columns[table->num_columns].is_primary_key = is_primary_key;
    table->columns[table->num_columns].allows_null = allows_null;
    
    table->num_columns++;
    
    // Si ya existen filas, debemos expandir sus arrays de valores
    for (int i = 0; i < table->num_rows; i++) {
        Value* new_values = (Value*)realloc(table->rows[i].values, 
                                         table->num_columns * sizeof(Value));
        if (!new_values) return -1;
        
        table->rows[i].values = new_values;
        // Inicializar el nuevo valor como nulo (0)
        memset(&table->rows[i].values[table->num_columns - 1], 0, sizeof(Value));
    }
    
    return 0;
}

/*
* Función para agregar una fila a una tabla
* @param table Puntero a la tabla
* @param values Arreglo de valores para la fila
* @return 0 si se agregó correctamente, -1 si hubo un error
*/
int table_add_row(Table* table, Value* values) {
    if (!table || !values) return -1;
    
    // Expandir el array de filas si es necesario
    if (table->num_rows >= table->capacity) {
        int new_capacity = table->capacity == 0 ? 1 : table->capacity * 2;
        Row* new_rows = (Row*)realloc(table->rows, new_capacity * sizeof(Row));
        if (!new_rows) return -1;
        
        table->rows = new_rows;
        table->capacity = new_capacity;
    }
    
    // Inicializar la nueva fila
    table->rows[table->num_rows].values = (Value*)malloc(table->num_columns * sizeof(Value));
    if (!table->rows[table->num_rows].values) return -1;
    
    table->rows[table->num_rows].is_deleted = 0;
    
    // Copiar los valores proporcionados
    for (int i = 0; i < table->num_columns; i++) {
        if (table->columns[i].type == TYPE_STRING && values[i].string_val) {
            table->rows[table->num_rows].values[i].string_val = strdup(values[i].string_val);
        } else {
            table->rows[table->num_rows].values[i] = values[i];
        }
    }
    
    table->num_rows++;
    
    return 0;
}

/*
* Función para eliminar una fila de una tabla
* @param table Puntero a la tabla
* @param row_index Índice de la fila a eliminar
* @return 0 si se eliminó correctamente, -1 si hubo un error
*/
int table_delete_row(Table* table, int row_index) {
    if (!table || row_index < 0 || row_index >= table->num_rows) return -1;
    
    // Liberar memoria de los valores de tipo string
    for (int j = 0; j < table->num_columns; j++) {
        if (table->columns[j].type == TYPE_STRING && 
            table->rows[row_index].values[j].string_val) {
            free(table->rows[row_index].values[j].string_val);
        }
    }
    
    // Liberar memoria de los valores de la fila
    free(table->rows[row_index].values);
    
    // Mover las filas restantes hacia arriba
    for (int i = row_index; i < table->num_rows - 1; i++) {
        table->rows[i] = table->rows[i + 1];
    }
    
    table->num_rows--;
    
    return 0;
}

/*
* Función para imprimir una tabla con formato simple
* @param table Puntero a la tabla a imprimir
*/
void table_print(Table* table) {
    if (!table) return;
    
    // Imprimir el nombre de la tabla
    printf("Table: %s\n", table->name);
    
    // Imprimir los nombres de las columnas
    for (int i = 0; i < table->num_columns; i++) {
        printf("%s\t", table->columns[i].name);
    }
    printf("\n");
    
    // Imprimir los valores de las filas
    for (int i = 0; i < table->num_rows; i++) {
        for (int j = 0; j < table->num_columns; j++) {
            Value value = table->rows[i].values[j];
            const char* str_value = value_to_string(value, table->columns[j].type);
            printf("%s\t", str_value);
        }
        printf("\n");
    }
}

/*
* Función para imprimir una tabla con formato mejorado
* @param table Puntero a la tabla a imprimir
*/
void table_print_formatted(Table* table) {
    if (!table || table->num_columns == 0) {
        printf("Tabla vacía o sin columnas definidas.\n");
        return;
    }
    
    // Calcular el ancho máximo para cada columna
    int* col_widths = (int*)malloc(table->num_columns * sizeof(int));
    for (int i = 0; i < table->num_columns; i++) {
        // Inicializar con el ancho del nombre de columna
        col_widths[i] = strlen(table->columns[i].name);
        
        // Revisar todos los valores para encontrar el más ancho
        for (int j = 0; j < table->num_rows; j++) {
            const char* str_value = value_to_string(table->rows[j].values[i], 
                                                  table->columns[i].type);
            int value_width = strlen(str_value);
            if (value_width > col_widths[i]) {
                col_widths[i] = value_width;
            }
        }
        
        // Añadir espacio para padding
        col_widths[i] += 2;
    }
    
    // Imprimir encabezado con nombre de tabla
    printf("Tabla: %s\n", table->name);
    
    // Imprimir línea superior
    printf("+");
    for (int i = 0; i < table->num_columns; i++) {
        for (int j = 0; j < col_widths[i]; j++) printf("-");
        printf("+");
    }
    printf("\n");
    
    // Imprimir nombres de columnas
    printf("|");
    for (int i = 0; i < table->num_columns; i++) {
        printf(" %-*s|", col_widths[i]-2, table->columns[i].name);
    }
    printf("\n");
    
    // Imprimir línea divisoria
    printf("+");
    for (int i = 0; i < table->num_columns; i++) {
        for (int j = 0; j < col_widths[i]; j++) printf("-");
        printf("+");
    }
    printf("\n");
    
    // Imprimir filas
    for (int i = 0; i < table->num_rows; i++) {
        printf("|");
        for (int j = 0; j < table->num_columns; j++) {
            const char* str_value = value_to_string(table->rows[i].values[j], 
                                                  table->columns[j].type);
            printf(" %-*s|", col_widths[j]-2, str_value);
        }
        printf("\n");
    }
    
    // Imprimir línea inferior
    printf("+");
    for (int i = 0; i < table->num_columns; i++) {
        for (int j = 0; j < col_widths[i]; j++) printf("-");
        printf("+");
    }
    printf("\n");
    
    // Imprimir conteo de filas
    printf("%d fila%s en total\n", table->num_rows, table->num_rows == 1 ? "" : "s");
    
    free(col_widths);
}
