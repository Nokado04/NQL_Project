#ifndef AST_H
#define AST_H

#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"  // Comentar hasta implementar el lexer

// Tipos de nodos AST
typedef enum {
    NODE_SELECT_STMT,
    NODE_INSERT_STMT,
    NODE_UPDATE_STMT,
    NODE_DELETE_STMT,
    NODE_CREATE_TABLE_STMT,
    NODE_ALTER_TABLE_STMT,
    NODE_DROP_TABLE_STMT,
    NODE_COLUMN_DEF,
    NODE_COLUMN_LIST,
    NODE_VALUE_LIST,
    NODE_WHERE_CLAUSE,
    NODE_ASSIGNMENT,
    NODE_BINARY_EXPR,
    NODE_UNARY_EXPR,
    NODE_IDENTIFIER,
    NODE_LITERAL
} ASTNodeType;

// Tipos de operadores binarios
typedef enum {
    OP_PLUS,
    OP_MINUS,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_EQ,
    OP_NEQ,
    OP_LT,
    OP_GT,
    OP_LTE,
    OP_GTE,
    OP_AND,
    OP_OR
} BinaryOpType;

// Tipos de operadores unarios
typedef enum {
    OP_NOT,
    OP_NEG
} UnaryOpType;

// Tipos de literales
typedef enum {
    LIT_INTEGER,
    LIT_FLOAT,
    LIT_STRING,
    LIT_BOOLEAN,
    LIT_NULL
} LiteralType;

// Declaración forward de ASTNode
typedef struct ASTNode ASTNode;

// Datos para SELECT
typedef struct {
    char* table_name;
    ASTNode* columns;     // NODE_COLUMN_LIST
    ASTNode* where_clause; // NODE_WHERE_CLAUSE (opcional)
} SelectStmtData;

// Datos para INSERT
typedef struct {
    char* table_name;
    ASTNode* values;      // NODE_VALUE_LIST
} InsertStmtData;

// Datos para UPDATE
typedef struct {
    char* table_name;
    ASTNode* assignments;  // Lista de NODE_ASSIGNMENT
    ASTNode* where_clause; // NODE_WHERE_CLAUSE (opcional)
} UpdateStmtData;

// Datos para DELETE
typedef struct {
    char* table_name;
    ASTNode* where_clause; // NODE_WHERE_CLAUSE (opcional)
} DeleteStmtData;

// Datos para CREATE TABLE
typedef struct {
    char* table_name;
    ASTNode* columns;      // Lista de NODE_COLUMN_DEF
} CreateTableStmtData;

// Datos para ALTER TABLE
typedef struct {
    char* table_name;
    ASTNode* column;       // NODE_COLUMN_DEF
} AlterTableStmtData;

// Datos para DROP TABLE
typedef struct {
    char* table_name;
} DropTableStmtData;

// Datos para definición de columna
typedef struct {
    char* name;
    int data_type;
    int max_length;
    int is_primary_key;
    int allows_null;
} ColumnDefData;

// Datos para lista de columnas
typedef struct {
    int count;
    int is_all;  // Para SELECT *
    char** columns;
} ColumnListData;

// Datos para lista de valores
typedef struct {
    int count;
    ASTNode** values;
} ValueListData;

// Datos para cláusula WHERE
typedef struct {
    ASTNode* condition;
} WhereClauseData;

// Datos para asignación
typedef struct {
    char* column_name;
    ASTNode* value;
} AssignmentData;

// Datos para expresión binaria
typedef struct {
    BinaryOpType op_type;
    ASTNode* left;
    ASTNode* right;
} BinaryExprData;

// Datos para expresión unaria
typedef struct {
    UnaryOpType op_type;
    ASTNode* operand;
} UnaryExprData;

// Datos para identificador
typedef struct {
    char* name;
} IdentifierData;

// Datos para literal
typedef struct {
    LiteralType lit_type;
    union {
        int int_value;
        double float_value;
        char* string_value;
        int bool_value;
    };
} LiteralData;

// Función para liberar un tipo específico de datos
typedef void (*ASTNodeFreeFunc)(void*);

// Estructura principal del nodo AST
struct ASTNode {
    ASTNodeType type;
    void* data;
    ASTNodeFreeFunc free_data;
    ASTNode* parent;
    ASTNode* next;    // Para listas de nodos (sibling)
    ASTNode* prev;    // Para listas bidireccionales
};

// Funciones de creación de nodos
ASTNode* ast_create_node(ASTNodeType type);
ASTNode* ast_create_select(char* table_name, ASTNode* columns, ASTNode* where);
ASTNode* ast_create_insert(char* table_name, ASTNode* values);
ASTNode* ast_create_update(char* table_name, ASTNode* assignments, ASTNode* where);
ASTNode* ast_create_delete(char* table_name, ASTNode* where);
ASTNode* ast_create_create_table(char* table_name, ASTNode* columns);
ASTNode* ast_create_alter_table(char* table_name, ASTNode* column);
ASTNode* ast_create_drop_table(char* table_name);
ASTNode* ast_create_column_def(char* name, int data_type, int max_length, int is_primary_key, int allows_null);
ASTNode* ast_create_column_list(int is_all, char** columns, int count);
ASTNode* ast_create_value_list(ASTNode** values, int count);
ASTNode* ast_create_where_clause(ASTNode* condition);
ASTNode* ast_create_assignment(char* column_name, ASTNode* value);
ASTNode* ast_create_binary_expr(BinaryOpType op, ASTNode* left, ASTNode* right);
ASTNode* ast_create_unary_expr(UnaryOpType op, ASTNode* operand);
ASTNode* ast_create_identifier(char* name);
ASTNode* ast_create_literal_int(int value);
ASTNode* ast_create_literal_float(double value);
ASTNode* ast_create_literal_string(char* value);
ASTNode* ast_create_literal_bool(int value);
ASTNode* ast_create_literal_null();

// Añadir esta línea cerca de las otras declaraciones de funciones AST
void ast_set_column_name(ASTNode* node, const char* column_name);

// Funciones para manipulación de AST
void ast_free_node(ASTNode* node);
void ast_append_sibling(ASTNode* node, ASTNode* sibling);
ASTNode* ast_get_next_sibling(ASTNode* node);
void ast_print(ASTNode* node, int level);

// Estructura para AST completo
typedef struct {
    ASTNode* root;
    const char* error_message;
    int error_position;
    int has_error;
} ParsedAST;

// Funciones para AST completo
ParsedAST* ast_create();
void ast_free(ParsedAST* ast);

#endif /* AST_H */