#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Comando para sumar números
int cmd_add(char *args[], int arg_count) {
    if (arg_count < 2) {
        printf("Error: Se requieren al menos dos números para sumar\n");
        printf("Uso: add número1 número2 [número3 ...]\n");
        return -1;
    }
    
    int sum = 0;
    for (int i = 0; i < arg_count; i++) {
        sum += atoi(args[i]);
    }
    
    printf("Resultado: %d\n", sum);
    return 0;
}

// Comando para restar números
int cmd_subtract(char *args[], int arg_count) {
    if (arg_count < 2) {
        printf("Error: Se requieren al menos dos números para restar\n");
        printf("Uso: subtract número1 número2 [número3 ...]\n");
        return -1;
    }
    
    int result = atoi(args[0]);
    for (int i = 1; i < arg_count; i++) {
        result -= atoi(args[i]);
    }
    
    printf("Resultado: %d\n", result);
    return 0;
}

// Comando para multiplicar números
int cmd_multiply(char *args[], int arg_count) {
    if (arg_count < 2) {
        printf("Error: Se requieren al menos dos números para multiplicar\n");
        printf("Uso: multiply número1 número2 [número3 ...]\n");
        return -1;
    }
    
    int result = 1;
    for (int i = 0; i < arg_count; i++) {
        result *= atoi(args[i]);
    }
    
    printf("Resultado: %d\n", result);
    return 0;
}