#ifndef GRAMMAR_H
#define GRAMMAR_H

/*
 * Este archivo define la gramática SQL soportada por NQL.
 * Se usa como referencia para el parser y para documentación.
 *
 * Notación BNF simplificada:
 * | significa alternativa
 * <...> significa un elemento no terminal
 * [...] significa un elemento opcional
 * {...} significa repetición (0 o más veces)
 */

/*
 * <statement> ::= <select_stmt> | <insert_stmt> | <update_stmt> | <delete_stmt> | 
 *                <create_table_stmt> | <alter_table_stmt> | <drop_table_stmt>
 *
 * <select_stmt> ::= SELECT <column_list> FROM <table_name> [<where_clause>]
 *
 * <insert_stmt> ::= INSERT INTO <table_name> VALUES <value_list>
 *
 * <update_stmt> ::= UPDATE <table_name> SET <assignment_list> [<where_clause>]
 *
 * <delete_stmt> ::= DELETE FROM <table_name> [<where_clause>]
 *
 * <create_table_stmt> ::= CREATE TABLE <table_name> [<column_def_list>]
 *
 * <alter_table_stmt> ::= ALTER TABLE <table_name> ADD COLUMN <column_def>
 *
 * <drop_table_stmt> ::= DROP TABLE <table_name>
 *
 * <column_list> ::= * | <identifier> {, <identifier>}
 *
 * <column_def_list> ::= ( <column_def> {, <column_def>} )
 *
 * <column_def> ::= <identifier> <data_type> [PRIMARY KEY] [NOT NULL]
 *
 * <data_type> ::= INT | FLOAT | STRING(<integer>) | BOOL
 *
 * <assignment_list> ::= <assignment> {, <assignment>}
 *
 * <assignment> ::= <identifier> = <expression>
 *
 * <where_clause> ::= WHERE <expression>
 *
 * <value_list> ::= ( <expression> {, <expression>} )
 *
 * <expression> ::= <literal> | <identifier> | <unary_expr> | <binary_expr> | ( <expression> )
 *
 * <binary_expr> ::= <expression> <binary_op> <expression>
 *
 * <unary_expr> ::= <unary_op> <expression>
 *
 * <binary_op> ::= + | - | * | / | = | <> | < | > | <= | >= | AND | OR
 *
 * <unary_op> ::= - | NOT
 *
 * <literal> ::= <integer> | <float> | <string> | <boolean> | NULL
 *
 * <integer> ::= <digit> {<digit>}
 *
 * <float> ::= <integer> . <integer>
 *
 * <string> ::= " {<character>} "
 *
 * <boolean> ::= TRUE | FALSE
 *
 * <table_name> ::= <identifier>
 *
 * <identifier> ::= <letter> {<letter> | <digit> | _}
 */

// Función para obtener la especificación de la gramática como cadena
const char* grammar_get_specification();

#endif /* GRAMMAR_H */