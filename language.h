#ifndef LANGUAGE_H
#define LANGUAGE_H

enum type_t{
    OPERATOR_TYPE = 0,
    VARIABLE_TYPE = 1,
    NUMBER_TYPE = 2,
};

enum operations_t{
    ADD = 1,
    SUB = 2,
    MUL = 3,
    DIV = 4,
    ASSIGNMENT = 5,
    IF = 6,
    WHILE = 7,
    END_OF_PROGRAMM = 8,
    PAR_OPEN = 9,
    PAR_CLOSE = 10,
    OP_END = 11,
    COPMLEX_OPERATOR_OPEN = 12,
    COPMLEX_OPERATOR_CLOSE = 13,
    FUNC = 14,
    PRINT = 15,
    INPUT = 16,
};

union tree_elem_t{operations_t operator_name;
                  int number_value;
                  int variable_number;};


struct node_t{tree_elem_t value;
              type_t type;
              node_t* left;
              node_t* right;
              node_t* parent;
};

struct variable_t{char* var_name;
                  int var_value;
};

struct tree_t{node_t* root;
              variable_t* variable_list;
              int number_of_variables;
              node_t** node_list_begin;
              unsigned int node_list_len;
};


enum tree_errors{
    NO_ERROR = 0,
    ALLOCATION_ERROR = 1,
    FILE_OPENING_ERROR = 2,
    FILE_CLOSING_ERROR = 3,
    GRAPH_MAKING_ERROR = 4,
    FILES_NOT_FOUNDED_ERROR = 5,
    READING_ERROR = 6,
    FILE_STATISTICS_ERROR = 7,
    WAY_READING_ERROR = 8,
    TYPE_ERROR = 9,
    UNKNOWN_OPERATOR_ERROR = 10,
    DUMP_ERROR = 11,
    TREE_READING_ERROR = 12,
    ASM_MAKING_ERROR = 13,
};

tree_errors tree_init(tree_t** tree);
void infix_tree_destroy(tree_t* tree);
void tree_destroy(tree_t* tree);

node_t* node_init(tree_elem_t value, type_t type, node_t* left, node_t* right);
void node_destroy(node_t* node);
void make_parents(node_t* node, node_t* parent);

node_t* equation_simplification(node_t* node, tree_t* tree);

node_t* infix_read(char* file_name, tree_t* tree);

tree_errors tree_dump(tree_t* tree);
tree_errors node_output(node_t* node, tree_t* tree, FILE* output_address);

tree_errors tree_input(tree_t* tree, char* file_name);
tree_errors node_assemblating(node_t* node, tree_t* tree, FILE* asm_address, int* label_number);

#endif //LANGUAGE_H

