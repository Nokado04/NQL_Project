#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../parser/lexer.h"
#include "../parser/parser.h"
#include "../parser/ast.h"

#define TEST_QUERY(sql) do { \
    printf("\n===== Probando: %s =====\n", sql); \
    Parser* p = parser_create(sql); \
    ASTNode* ast = parser_parse(p); \
    if (!ast) { \
        printf("Error al parsear: %s\n", parser_get_error(p)); \
    } else { \
        printf("Parseo exitoso:\n"); \
        ast_print(ast, 0); \
        ast_free_node(ast); \
    } \
    parser_free(p); \
} while(0)

int main() {
    printf("=== PRUEBA SIMPLE DEL PARSER SQL ===\n");
    
    // Pruebas básicas
    TEST_QUERY("SELECT * FROM usuarios");
    TEST_QUERY("SELECT id, nombre FROM productos WHERE precio > 100");
    TEST_QUERY("INSERT INTO usuarios VALUES (1, \"Juan\", 30, TRUE)");
    TEST_QUERY("UPDATE productos SET precio = 200, stock = stock - 1 + 2 + 2 -3 WHERE id = 5");
    TEST_QUERY("DELETE FROM pedidos WHERE fecha < \"2023-01-01\"");
    
    // Pruebas de DDL
    TEST_QUERY("CREATE TABLE clientes (id INT PRIMARY KEY, nombre STRING(50) NOT NULL, activo BOOL)");
    TEST_QUERY("ALTER TABLE productos ADD COLUMN descripcion STRING(200)");
    TEST_QUERY("DROP TABLE temp");
    
    printf("\n=== PRUEBAS COMPLETADAS ===\n");
    return 0;
}