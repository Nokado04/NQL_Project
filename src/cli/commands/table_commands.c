#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../../db/database.h"
#include "../../db/table.h"
#include "../../db/value.h"
#include "cmd_registry.h"

/*
* Comando para crear una tabla
* CREATE TABLE nombre_tabla
*/
int cmd_create_table(char *args[], int arg_count) {
    if (arg_count < 1) {
        printf("Error: Sintaxis: CREATE TABLE nombre_tabla\n");
        return -1;
    }
    
    const char* table_name = args[0];
    
    // Crear la tabla
    Table* table = db_create_table(table_name);
    if (!table) {
        return -1;
    }
    
    printf("Tabla creada: %s\n", table_name);
    printf("\nPara añadir columnas use:\n");
    printf("ALTER TABLE %s ADD COLUMN nombre_columna tipo [opciones]\n\n", table_name);
    printf("Ejemplos de tipos: INT, FLOAT, STRING(50), BOOL\n");
    printf("Opciones: PRIMARY KEY, NOT NULL\n");
    
    return 0;
}

/*
* Comando para modificar una tabla
* ALTER TABLE nombre_tabla ADD COLUMN nombre_columna tipo [opciones]
*/
int cmd_alter_table(char *args[], int arg_count) {
    if (arg_count < 5) {
        printf("Error: Sintaxis: ALTER TABLE nombre_tabla ADD COLUMN nombre_columna tipo [opciones]\n");
        printf("Tipos disponibles: INT, FLOAT, STRING(max_length), BOOL\n");
        printf("Opciones: PRIMARY KEY, NOT NULL\n");
        return -1;
    }
    
    // Verificar que sea "ADD COLUMN"
    if (strcasecmp(args[1], "ADD") != 0 || strcasecmp(args[2], "COLUMN") != 0) {
        printf("Error: Solo se admite la operación 'ADD COLUMN'.\n");
        return -1;
    }
    
    const char* table_name = args[0];
    const char* column_name = args[3];
    const char* type_str = args[4];
    
    // Buscar la tabla
    Table* table = db_find_table(table_name);
    if (!table) {
        printf("Error: Tabla '%s' no encontrada.\n", table_name);
        return -1;
    }
    
    // Parsear tipo de dato
    DataType type;
    int max_length = 0;
    
    if (strcasecmp(type_str, "INT") == 0) {
        type = TYPE_INT;
    } else if (strcasecmp(type_str, "FLOAT") == 0) {
        type = TYPE_FLOAT;
    } else if (strcasecmp(type_str, "BOOL") == 0) {
        type = TYPE_BOOL;
    } else if (strncasecmp(type_str, "STRING", 6) == 0) {
        type = TYPE_STRING;
        
        // Extraer longitud máxima de STRING(max_length)
        if (strlen(type_str) > 7 && type_str[6] == '(') {
            char* end_ptr = strchr(type_str + 7, ')');
            if (end_ptr) {
                *end_ptr = '\0';
                max_length = atoi(type_str + 7);
            }
        }
        
        if (max_length <= 0) {
            max_length = 255; // Valor por defecto
        }
    } else {
        printf("Error: Tipo de dato '%s' no válido. Use INT, FLOAT, STRING(max_length), o BOOL.\n", type_str);
        return -1;
    }
    
    // Verificar opciones
    int is_primary_key = 0;
    int allows_null = 1; // Por defecto permite nulos
    
    for (int i = 5; i < arg_count; i++) {
        if (strcasecmp(args[i], "PRIMARY") == 0 && i+1 < arg_count && 
            strcasecmp(args[i+1], "KEY") == 0) {
            is_primary_key = 1;
            i++; // Saltar "KEY"
        } else if (strcasecmp(args[i], "NOT") == 0 && i+1 < arg_count && 
                  strcasecmp(args[i+1], "NULL") == 0) {
            allows_null = 0;
            i++; // Saltar "NULL"
        }
    }
    
    // Agregar la columna
    if (table_add_column(table, column_name, type, max_length, 
                         is_primary_key, allows_null) == 0) {
        printf("Columna añadida: %s (%s)\n", column_name, 
              column_type_to_string(type, max_length));
        return 0;
    } else {
        printf("Error: No se pudo añadir la columna '%s'\n", column_name);
        return -1;
    }
}

/*
* Comando para describir la estructura de una tabla
* DESCRIBE nombre_tabla
*/
int cmd_describe(char *args[], int arg_count) {
    if (arg_count < 1) {
        printf("Error: Sintaxis: DESCRIBE nombre_tabla\n");
        return -1;
    }
    
    const char* table_name = args[0];
    
    // Buscar la tabla
    Table* table = db_find_table(table_name);
    if (!table) {
        printf("Error: Tabla '%s' no encontrada.\n", table_name);
        return -1;
    }
    
    // Imprimir información de la estructura de la tabla
    printf("Tabla: %s\n", table_name);
    printf("+------------+--------------+------------+-------------+\n");
    printf("| Campo      | Tipo         | Nulo       | Clave       |\n");
    printf("+------------+--------------+------------+-------------+\n");
    
    for (int i = 0; i < table->num_columns; i++) {
        Column col = table->columns[i];
        const char* type_str = column_type_to_string(col.type, col.max_length);
        
        printf("| %-10s | %-12s | %-10s | %-11s |\n",
               col.name,
               type_str,
               col.allows_null ? "SI" : "NO",
               col.is_primary_key ? "PRIMARIA" : "");
    }
    
    printf("+------------+--------------+------------+-------------+\n");
    printf("%d columna%s en tabla\n", table->num_columns, 
           table->num_columns == 1 ? "" : "s");
    
    return 0;
}