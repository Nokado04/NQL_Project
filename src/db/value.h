#ifndef VALUE_H
#define VALUE_H

// Data types supported in our simple SQL-like table
typedef enum {
    TYPE_INT,
    TYPE_FLOAT,
    TYPE_STRING,
    TYPE_BOOL
} DataType;

// Structure to hold a single cell value
typedef union {
    int int_val;
    float float_val;
    char* string_val;
    int bool_val;
} Value;

// Function prototypes for data type conversion
Value string_to_value(const char* str, DataType type);
const char* value_to_string(Value value, DataType type);

#endif