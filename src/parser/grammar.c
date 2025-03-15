#include "grammar.h"

// Devuelve la especificación completa de la gramática como una cadena
const char* grammar_get_specification() {
    static const char* spec =
        "<statement> ::= <select_stmt> | <insert_stmt> | <update_stmt> | <delete_stmt> | "
        "<create_table_stmt> | <alter_table_stmt> | <drop_table_stmt>\n\n"
        
        "<select_stmt> ::= SELECT <column_list> FROM <table_name> [<where_clause>]\n\n"
        
        "<insert_stmt> ::= INSERT INTO <table_name> VALUES <value_list>\n\n"
        
        "<update_stmt> ::= UPDATE <table_name> SET <assignment_list> [<where_clause>]\n\n"
        
        "<delete_stmt> ::= DELETE FROM <table_name> [<where_clause>]\n\n"
        
        "<create_table_stmt> ::= CREATE TABLE <table_name> [<column_def_list>]\n\n"
        
        "<alter_table_stmt> ::= ALTER TABLE <table_name> ADD COLUMN <column_def>\n\n"
        
        "<drop_table_stmt> ::= DROP TABLE <table_name>\n\n"
        
        "<column_list> ::= * | <identifier> {, <identifier>}\n\n"
        
        "<column_def_list> ::= ( <column_def> {, <column_def>} )\n\n"
        
        "<column_def> ::= <identifier> <data_type> [PRIMARY KEY] [NOT NULL]\n\n"
        
        "<data_type> ::= INT | FLOAT | STRING(<integer>) | BOOL\n\n"
        
        "<assignment_list> ::= <assignment> {, <assignment>}\n\n"
        
        "<assignment> ::= <identifier> = <expression>\n\n"
        
        "<where_clause> ::= WHERE <expression>\n\n"
        
        "<value_list> ::= ( <expression> {, <expression>} )\n\n"
        
        "<expression> ::= <literal> | <identifier> | <unary_expr> | <binary_expr> | ( <expression> )\n\n"
        
        "<binary_expr> ::= <expression> <binary_op> <expression>\n\n"
        
        "<unary_expr> ::= <unary_op> <expression>\n\n"
        
        "<binary_op> ::= + | - | * | / | = | <> | < | > | <= | >= | AND | OR\n\n"
        
        "<unary_op> ::= - | NOT\n\n"
        
        "<literal> ::= <integer> | <float> | <string> | <boolean> | NULL\n\n"
        
        "<integer> ::= <digit> {<digit>}\n\n"
        
        "<float> ::= <integer> . <integer>\n\n"
        
        "<string> ::= \" {<character>} \"\n\n"
        
        "<boolean> ::= TRUE | FALSE\n\n"
        
        "<table_name> ::= <identifier>\n\n"
        
        "<identifier> ::= <letter> {<letter> | <digit> | _}";
    
    return spec;
}