#ifndef VALIDATOR_H
#define VALIDATOR_H

#include "ast.h"
#include "../db/database.h"

// Estructura para almacenar resultado de validación
typedef struct {
    int error_code;
    char* error_message;
    int error_position;
} ValidationResult;

// Inicializa el resultado de validación
ValidationResult* validator_create_result();

// Libera un resultado de validación
void validator_free_result(ValidationResult* result);

// Función principal para validar un AST contra una base de datos
int validator_validate(ASTNode* ast, Database* db, ValidationResult* result);

// Validar tipos específicos de sentencias
int validator_validate_select(ASTNode* node, Database* db, ValidationResult* result);
int validator_validate_insert(ASTNode* node, Database* db, ValidationResult* result);
int validator_validate_update(ASTNode* node, Database* db, ValidationResult* result);
int validator_validate_delete(ASTNode* node, Database* db, ValidationResult* result);
int validator_validate_create_table(ASTNode* node, Database* db, ValidationResult* result);
int validator_validate_alter_table(ASTNode* node, Database* db, ValidationResult* result);
int validator_validate_drop_table(ASTNode* node, Database* db, ValidationResult* result);

// Validar componentes comunes
int validator_validate_column_list(ASTNode* columns, Table* table, ValidationResult* result);
int validator_validate_expression(ASTNode* expr, Table* table, ValidationResult* result);
int validator_validate_where_clause(ASTNode* where, Table* table, ValidationResult* result);
int validator_validate_values(ASTNode* values, Table* table, ValidationResult* result);

// Utilidades
Table* validator_find_table(const char* table_name, Database* db);
int validator_check_column_exists(const char* column_name, Table* table);
int validator_check_type_compatibility(int expected_type, int actual_type);
int validator_set_error(ValidationResult* result, int code, const char* message);

#endif /* VALIDATOR_H */