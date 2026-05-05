#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include "file_using.h"
#include "language.h"

#define NUM_(value) node_init((tree_elem_t) {.number_value = value}, NUMBER_TYPE, NULL, NULL)//+
#define VAR_(number) node_init((tree_elem_t) {.variable_number = number}, VARIABLE_TYPE, NULL, NULL)
#define ADD_() node_init((tree_elem_t) {.operator_name = ADD}, OPERATOR_TYPE, NULL, NULL)//+
#define SUB_() node_init((tree_elem_t) {.operator_name = SUB}, OPERATOR_TYPE, NULL, NULL)//+
#define MUL_() node_init((tree_elem_t) {.operator_name = MUL}, OPERATOR_TYPE, NULL, NULL)//+
#define DIV_() node_init((tree_elem_t) {.operator_name = DIV}, OPERATOR_TYPE, NULL, NULL)//+
#define WHILE_() node_init((tree_elem_t) {.operator_name = WHILE}, OPERATOR_TYPE, NULL, NULL)//+
#define IF_() node_init((tree_elem_t) {.operator_name = IF}, OPERATOR_TYPE, NULL, NULL)//+
#define END_OF_PROGRAMM_() node_init((tree_elem_t) {.operator_name = END_OF_PROGRAMM}, OPERATOR_TYPE, NULL, NULL)//+
#define OP_END_() node_init((tree_elem_t) {.operator_name = OP_END}, OPERATOR_TYPE, NULL, NULL)//+
#define ASSIGNMENT_() node_init((tree_elem_t) {.operator_name = ASSIGNMENT}, OPERATOR_TYPE, NULL, NULL)//+
#define PRINT_() node_init((tree_elem_t) {.operator_name = PRINT}, OPERATOR_TYPE, NULL, NULL)//+
#define INPUT_() node_init((tree_elem_t) {.operator_name = INPUT}, OPERATOR_TYPE, NULL, NULL)//+

static node_t* make_new_var(char* str, tree_t* tree);
static node_t* node_input(char** buffer, tree_t* tree);

tree_errors tree_input(tree_t* tree, char* file_name)
{
    assert(file_name);
    assert(tree);

    FILE* input_address = NULL;
    struct stat statistics = {};
    char* buffer = NULL;
    char* buffer_begin = NULL;
    int buffer_len = 0;
    char symbol = 0;

    if (check_file_opening(file_name, &input_address, "r"))
        return FILE_OPENING_ERROR;

    if (fstat(fileno(input_address), &statistics))
        return FILE_STATISTICS_ERROR;

    buffer = (char*) calloc(statistics.st_size, sizeof(char));

    if (buffer == NULL)
        return ALLOCATION_ERROR;

    for (int i = 0; i < statistics.st_size; i++)
    {
        symbol = (char) fgetc(input_address);

        if (isgraph(symbol))
        {
            buffer[buffer_len] = symbol;
            buffer_len++;
        }
    }

    for (int i = 0; i < buffer_len; i++)
        printf("%c", buffer[i]);

    printf("\n");

    buffer_begin = buffer;

    tree->root = node_input(&buffer, tree);

    if (check_file_closing(input_address))
        printf("Error while closing file!");

    free(buffer_begin);

    return NO_ERROR;
}

node_t* node_input(char** buffer, tree_t* tree)
{
    assert(buffer);
    assert(tree);

    node_t* new_node = NULL;
    int val = 0;
    int i = 0;
    char command[256] = {0};

    if (**buffer == '(')
    {
        (*buffer)++;
        if (isdigit(**buffer) || (**buffer == '-' && isdigit((*buffer)[1])))
        {
            sscanf(*buffer, "%d", &val);
            sprintf(command, "%d", val);
            *buffer += strlen(command);
            new_node = NUM_(val);
        }
        else if (**buffer == '+')
        {
            new_node = ADD_();
            (*buffer)++;
        }
        else if (**buffer == '-')
        {
            new_node = SUB_();
            (*buffer)++;
        }
        else if (**buffer == '*')
        {
            new_node = MUL_();
            (*buffer)++;
        }
        else if (**buffer == '/')
        {
            new_node = DIV_();
            (*buffer)++;
        }
        else if (**buffer == ';')
        {
            new_node = OP_END_();
            (*buffer)++;
        }
        else if (**buffer == '=')
        {
            new_node = ASSIGNMENT_();
            (*buffer)++;
        }
        else if (strncmp(*buffer, "WHILE", strlen("WHILE")) == 0)
        {
            new_node = WHILE_();
            *buffer += strlen("WHILE");
        }
        else if (strncmp(*buffer, "IF", strlen("IF")) == 0)
        {
            new_node = IF_();
            *buffer += strlen("IF");
        }
        else if (strncmp(*buffer, "END_OF_PROGRAMM", strlen("END_OF_PROGRAMM")) == 0)
        {
            new_node = END_OF_PROGRAMM_();
            *buffer += strlen("END_OF_PROGRAMM");
        }
        else if (strncmp(*buffer, "PRINT", strlen("PRINT")) == 0)
        {
            new_node = PRINT_();
            *buffer += strlen("PRINT");
        }
        else if (strncmp(*buffer, "INPUT", strlen("INPUT")) == 0)
        {
            new_node = INPUT_();
            *buffer += strlen("INPUT");
        }
        else
        {
            i = 0;

            while (**buffer != ')' && i < 255 &&
                    !((*buffer)[0] == 'n' && (*buffer)[1] == 'i' && (*buffer)[2] == 'l'))
            {
                command[i] = **buffer;
                (*buffer)++;
                i++;
            }
            new_node = make_new_var(command, tree);
            strncpy(command, "", 256);
        }
        new_node->left = node_input(buffer, tree);
        new_node->right = node_input(buffer, tree);

        if (**buffer == ')')
            (*buffer)++;
        else
            printf("Error while reading tree - no ')'! buffer = %20s\n", *buffer);
        return new_node;
    }
    else if ((*buffer)[0] == 'n' && (*buffer)[1] == 'i' && (*buffer)[2] == 'l')
    {
        *buffer += 3*sizeof(char);
        return NULL;
    }
    else
    {
        printf("Error while reading tree!buffer = %20s\n", *buffer);
        return NULL;
    }
}

node_t* make_new_var(char* str, tree_t* tree)
{
    assert(tree);
    assert(str);

    for (int i = 0; i < tree->number_of_variables; i++)
        if (strcmp(str, tree->variable_list[i].var_name) == 0)
            return VAR_(i);

    tree->variable_list[tree->number_of_variables].var_name = strdup(str);

    if (tree->variable_list[tree->number_of_variables].var_name == NULL)
        return NULL;

    tree->number_of_variables++;

    return VAR_(tree->number_of_variables - 1);
}

#undef NUM_
#undef ADD_
#undef SUB_
#undef MUL_
#undef DIV_
#undef ASSIGNMENT_
#undef WHILE_
#undef IF_
#undef END_OF_PROGRAMM_
#undef OP_END_
#undef PRINT_
#undef INPUT_
