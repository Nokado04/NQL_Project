#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../db/database.h"
#include "../../db/table.h"
#include "../../db/value.h"
#include "../input_handler.h"
#include "cmd_registry.h"

/*
* Comando para insertar datos en una tabla
* INSERT INTO tabla VALUES (valor1, valor2, ...)
*/
int cmd_insert(char *args[], int arg_count) {
    if (arg_count < 3) {
        printf("Error: Sintaxis: INSERT INTO nombre_tabla VALUES (valor1, valor2, ...)\n");
        printf("Ejemplo: INSERT INTO usuarios VALUES (1, \"Juan\", 25, \"M\")\n");
        return -1;
    }
    
    const char* table_name = args[0];
    
    // Verificar que el segundo argumento sea "VALUES"
    if (strcasecmp(args[1], "VALUES") != 0) {
        printf("Error: Se esperaba la palabra clave 'VALUES'.\n");
        return -1;
    }
    
    // Buscar la tabla
    Table* table = db_find_table(table_name);
    if (!table) {
        printf("Error: Tabla '%s' no encontrada.\n", table_name);
        return -1;
    }
    
    // Procesar los valores como una sola cadena para separar correctamente
    char combined_values[MAX_INPUT_LENGTH] = "";
    for (int i = 2; i < arg_count; i++) {
        strcat(combined_values, args[i]);
        if (i < arg_count - 1) {
            strcat(combined_values, " ");
        }
    }
    
    // Eliminar los paréntesis exteriores
    char *values_str = combined_values;
    
    // Buscar el primer paréntesis abierto
    char *start = strchr(values_str, '(');
    if (!start) {
        printf("Error: Los valores deben estar entre paréntesis: (val1, val2, ...)\n");
        return -1;
    }
    
    // Buscar el último paréntesis cerrado
    char *end = strrchr(values_str, ')');
    if (!end) {
        printf("Error: Falta el paréntesis de cierre\n");
        return -1;
    }
    
    *end = '\0';  // Remover el paréntesis de cierre
    values_str = start + 1;  // Saltar el paréntesis de apertura
    
    // Dividir la cadena por comas, respetando las comillas
    char *value_tokens[MAX_ARGS];
    int num_values = 0;
    
    char *token = strtok(values_str, ",");
    while (token != NULL && num_values < MAX_ARGS) {
        // Eliminar espacios en blanco al inicio y final
        while (*token == ' ') token++;
        char *end_token = token + strlen(token) - 1;
        while (end_token > token && *end_token == ' ') {
            *end_token = '\0';
            end_token--;
        }
        
        value_tokens[num_values++] = token;
        token = strtok(NULL, ",");
    }
    
    // Verificar que el número de valores coincida con el número de columnas
    if (num_values != table->num_columns) {
        printf("Error: La tabla '%s' tiene %d columnas, pero se proporcionaron %d valores.\n", 
               table_name, table->num_columns, num_values);
        printf("Se esperaban valores para: ");
        for (int i = 0; i < table->num_columns; i++) {
            printf("%s%s", table->columns[i].name, i < table->num_columns - 1 ? ", " : "");
        }
        printf("\n");
        return -1;
    }
    
    // Convertir los valores al tipo adecuado y realizar la inserción
    Value values[table->num_columns];
    for (int i = 0; i < table->num_columns; i++) {
        const char* str_value = value_tokens[i];
        
        // Si el valor está entre comillas, quitar las comillas
        if (str_value[0] == '"' && str_value[strlen(str_value)-1] == '"') {
            str_value++; // Saltar la comilla inicial
            char* value_copy = strdup(str_value);
            value_copy[strlen(value_copy)-1] = '\0'; // Quitar comilla final
            values[i] = string_to_value(value_copy, table->columns[i].type);
            free(value_copy);
        } else {
            values[i] = string_to_value(str_value, table->columns[i].type);
        }
    }
    
    // Insertar la fila en la tabla
    if (table_add_row(table, values) == 0) {
        printf("1 fila insertada en %s\n", table_name);
        return 0;
    } else {
        printf("Error: No se pudo insertar la fila\n");
        return -1;
    }
}

/*
* Comando para consultar datos con sintaxis SQL
* SELECT [*|columnas] FROM tabla [WHERE condicion]
*/
int cmd_select(char *args[], int arg_count) {
    if (arg_count < 3) {
        printf("Error: Sintaxis: SELECT * FROM nombre_tabla\n");
        printf("Nota: Actualmente solo se admite 'SELECT * FROM tabla'\n");
        return -1;
    }
    
    // Verificar que sea SELECT *
    if (strcmp(args[0], "*") != 0) {
        printf("Error: Actualmente solo se admite 'SELECT *'\n");
        return -1;
    }
    
    // Verificar que el segundo argumento sea FROM
    if (strcasecmp(args[1], "FROM") != 0) {
        printf("Error: Se esperaba la palabra clave 'FROM'\n");
        return -1;
    }
    
    const char* table_name = args[2];
    
    // Buscar la tabla
    Table* table = db_find_table(table_name);
    if (!table) {
        printf("Error: Tabla '%s' no encontrada.\n", table_name);
        return -1;
    }
    
    // Mostrar la tabla con formato mejorado
    table_print_formatted(table);
    
    return 0;
}

/*
* Comando para eliminar filas con sintaxis SQL
* DELETE FROM tabla WHERE rowid = valor
*/
int cmd_delete(char *args[], int arg_count) {
    if (arg_count < 5) {
        printf("Error: Sintaxis: DELETE FROM nombre_tabla WHERE rowid = índice\n");
        printf("Nota: Actualmente solo se admite eliminación por índice de fila.\n");
        return -1;
    }
    
    const char* table_name = args[0];
    
    // Verificar sintaxis WHERE rowid = valor
    if (strcasecmp(args[1], "WHERE") != 0 || 
        strcasecmp(args[2], "rowid") != 0 || 
        strcmp(args[3], "=") != 0) {
        printf("Error: Actualmente solo se admite 'WHERE rowid = índice'\n");
        return -1;
    }
    
    // Buscar la tabla
    Table* table = db_find_table(table_name);
    if (!table) {
        printf("Error: Tabla '%s' no encontrada.\n", table_name);
        return -1;
    }
    
    int row_index = atoi(args[4]);
    
    // Verificar que el índice sea válido
    if (row_index < 0 || row_index >= table->num_rows) {
        printf("Error: Índice de fila %d fuera de rango (0-%d)\n", 
               row_index, table->num_rows - 1);
        return -1;
    }
    
    // Eliminar la fila
    if (table_delete_row(table, row_index) == 0) {
        printf("1 fila eliminada de %s\n", table_name);
        return 0;
    } else {
        printf("Error: No se pudo eliminar la fila con índice %d\n", row_index);
        return -1;
    }
}

/*
* Comando adicional para contar registros
* COUNT FROM tabla [WHERE condición]
*/
int cmd_count(char *args[], int arg_count) {
    if (arg_count < 2) {
        printf("Error: Sintaxis: COUNT FROM nombre_tabla\n");
        return -1;
    }
    
    // Verificar que el primer argumento sea FROM
    if (strcasecmp(args[0], "FROM") != 0) {
        printf("Error: Se esperaba la palabra clave 'FROM'\n");
        return -1;
    }
    
    const char* table_name = args[1];
    
    // Buscar la tabla
    Table* table = db_find_table(table_name);
    if (!table) {
        printf("Error: Tabla '%s' no encontrada.\n", table_name);
        return -1;
    }
    
    // Por ahora, simplemente contamos todas las filas
    printf("Cantidad de registros en %s: %d\n", table_name, table->num_rows);
    
    return 0;
}

/*
* Comando para actualizar datos en una tabla
* UPDATE tabla SET columna = valor WHERE rowid = índice
*/
int cmd_update(char *args[], int arg_count) {
    if (arg_count < 7) {
        printf("Error: Sintaxis: UPDATE nombre_tabla SET columna = valor WHERE rowid = índice\n");
        printf("Nota: Actualmente solo se admite actualización por índice de fila.\n");
        return -1;
    }
    
    const char* table_name = args[0];
    
    // Verificar que el segundo argumento sea SET
    if (strcasecmp(args[1], "SET") != 0) {
        printf("Error: Se esperaba la palabra clave 'SET'\n");
        return -1;
    }
    
    // Buscar la tabla
    Table* table = db_find_table(table_name);
    if (!table) {
        printf("Error: Tabla '%s' no encontrada.\n", table_name);
        return -1;
    }
    
    const char* column_name = args[2];
    
    // Verificar que el cuarto argumento sea un signo igual
    if (strcmp(args[3], "=") != 0) {
        printf("Error: Se esperaba un signo igual después del nombre de columna\n");
        return -1;
    }
    
    const char* new_value = args[4];
    
    // Verificar que el sexto argumento sea WHERE
    if (strcasecmp(args[5], "WHERE") != 0 ||
        strcasecmp(args[6], "rowid") != 0 ||
        strcmp(args[7], "=") != 0) {
        printf("Error: Actualmente solo se admite 'WHERE rowid = índice'\n");
        return -1;
    }
    
    int row_index = atoi(args[8]);
    
    // Verificar que el índice sea válido
    if (row_index < 0 || row_index >= table->num_rows) {
        printf("Error: Índice de fila %d fuera de rango (0-%d)\n", 
               row_index, table->num_rows - 1);
        return -1;
    }
    
    // Buscar el índice de la columna
    int col_index = -1;
    for (int i = 0; i < table->num_columns; i++) {
        if (strcasecmp(table->columns[i].name, column_name) == 0) {
            col_index = i;
            break;
        }
    }
    
    if (col_index == -1) {
        printf("Error: Columna '%s' no encontrada en la tabla '%s'\n", 
               column_name, table_name);
        return -1;
    }
    
    // Limpiar valor antiguo si es STRING
    if (table->columns[col_index].type == TYPE_STRING && 
        table->rows[row_index].values[col_index].string_val) {
        free(table->rows[row_index].values[col_index].string_val);
    }
    
    // Actualizar con el nuevo valor
    // Si el valor está entre comillas, quitar las comillas para STRING
    if (table->columns[col_index].type == TYPE_STRING && 
        new_value[0] == '"' && new_value[strlen(new_value)-1] == '"') {
        char* value_copy = strdup(new_value + 1); // Saltar comilla inicial
        value_copy[strlen(value_copy)-1] = '\0';  // Quitar comilla final
        table->rows[row_index].values[col_index] = string_to_value(value_copy, 
                                                      table->columns[col_index].type);
        free(value_copy);
    } else {
        table->rows[row_index].values[col_index] = string_to_value(new_value, 
                                                      table->columns[col_index].type);
    }
    
    printf("1 fila actualizada en %s\n", table_name);
    
    return 0;
}