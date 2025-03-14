#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "value.h"

// Convierte una cadena a un valor del tipo especificado
Value string_to_value(const char* str, DataType type) {
    Value value;
    memset(&value, 0, sizeof(Value)); // Inicializar con ceros
    
    if (!str) return value;
    
    switch (type) {
        case TYPE_INT:
            value.int_val = atoi(str);
            break;
        case TYPE_FLOAT:
            value.float_val = atof(str);
            break;
        case TYPE_STRING:
            value.string_val = strdup(str);
            break;
        case TYPE_BOOL:
            // Aceptar diferentes formatos de booleano
            if (strcasecmp(str, "true") == 0 || strcasecmp(str, "1") == 0 || 
                strcasecmp(str, "yes") == 0 || strcasecmp(str, "y") == 0) {
                value.bool_val = 1;
            } else {
                value.bool_val = 0;
            }
            break;
    }
    return value;
}

// Convierte un valor al formato de cadena
const char* value_to_string(Value value, DataType type) {
    static char buffer[256]; // Buffer estático para la conversión
    
    switch (type) {
        case TYPE_INT:
            snprintf(buffer, sizeof(buffer), "%d", value.int_val);
            break;
        case TYPE_FLOAT:
            snprintf(buffer, sizeof(buffer), "%.2f", value.float_val);
            break;
        case TYPE_STRING:
            if (value.string_val) {
                snprintf(buffer, sizeof(buffer), "%s", value.string_val);
            } else {
                buffer[0] = '\0'; // String vacío para NULL
            }
            break;
        case TYPE_BOOL:
            snprintf(buffer, sizeof(buffer), "%s", value.bool_val ? "true" : "false");
            break;
        default:
            buffer[0] = '\0';
    }
    
    return buffer;
}