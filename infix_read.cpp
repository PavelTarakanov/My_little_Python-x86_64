#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <math.h>
#include "file_using.h"
#include "language.h"

#define NUM_(value) node_init((tree_elem_t) {.number_value = value}, NUMBER_TYPE, NULL, NULL)
#define VAR_(number) node_init((tree_elem_t) {.variable_number = number}, VARIABLE_TYPE, NULL, NULL)
#define ADD_() node_init((tree_elem_t) {.operator_name = ADD}, OPERATOR_TYPE, NULL, NULL)
#define SUB_() node_init((tree_elem_t) {.operator_name = SUB}, OPERATOR_TYPE, NULL, NULL)
#define MUL_() node_init((tree_elem_t) {.operator_name = MUL}, OPERATOR_TYPE, NULL, NULL)
#define DIV_() node_init((tree_elem_t) {.operator_name = DIV}, OPERATOR_TYPE, NULL, NULL)
#define UNAR_MINUS_() node_init((tree_elem_t) {.operator_name = UNAR_MINUS}, OPERATOR_TYPE, NULL, NULL)
#define WHILE_() node_init((tree_elem_t) {.operator_name = WHILE}, OPERATOR_TYPE, NULL, NULL)
#define IF_() node_init((tree_elem_t) {.operator_name = IF}, OPERATOR_TYPE, NULL, NULL)
#define END_OF_PROGRAMM_() node_init((tree_elem_t) {.operator_name = END_OF_PROGRAMM}, OPERATOR_TYPE, NULL, NULL)
#define PAR_OPEN_() node_init((tree_elem_t) {.operator_name = PAR_OPEN}, OPERATOR_TYPE, NULL, NULL)
#define PAR_CLOSE_() node_init((tree_elem_t) {.operator_name = PAR_CLOSE}, OPERATOR_TYPE, NULL, NULL)
#define OP_END_() node_init((tree_elem_t) {.operator_name = OP_END}, OPERATOR_TYPE, NULL, NULL)
#define ASSIGNMENT_() node_init((tree_elem_t) {.operator_name = ASSIGNMENT}, OPERATOR_TYPE, NULL, NULL)
#define COPMLEX_OPERATOR_OPEN_() node_init((tree_elem_t) {.operator_name = COPMLEX_OPERATOR_OPEN}, OPERATOR_TYPE, NULL, NULL)
#define COPMLEX_OPERATOR_CLOSE_() node_init((tree_elem_t) {.operator_name = COPMLEX_OPERATOR_CLOSE}, OPERATOR_TYPE, NULL, NULL)
#define PRINT_() node_init((tree_elem_t) {.operator_name = PRINT}, OPERATOR_TYPE, NULL, NULL)
#define INPUT_() node_init((tree_elem_t) {.operator_name = INPUT}, OPERATOR_TYPE, NULL, NULL)

static node_t** lexic_analiz(char** buffer, tree_t* tree);
static node_t* get_g(node_t*** node_list, tree_t* tree);
static node_t* get_op_list(node_t*** node_list, tree_t* tree);
static node_t* get_op(node_t*** node_list, tree_t* tree);
static node_t* get_print(node_t*** node_list, tree_t* tree);
static node_t* get_input(node_t*** node_list, tree_t* tree);
static node_t* get_if(node_t*** node_list, tree_t* tree);
static node_t* get_while(node_t*** node_list, tree_t* tree);
static node_t* get_a(node_t*** node_list, tree_t* tree);
static node_t* get_e(node_t*** node_list, tree_t* tree);
static node_t* get_t(node_t*** node_list, tree_t* tree);
static node_t* get_p(node_t*** node_list, tree_t* tree);
static node_t* get_n(node_t*** node_list);
static node_t* get_v(node_t*** node_list, tree_t* tree);
static node_t* make_new_var(char* str, tree_t* tree);

const int START_STR_LEN = 10;
const char WHILE_COMMAND[] = "До тех пор, пока, волею Омниссии";
const char COMPLEX_OPERATOR_OPEN_COMMAND[] = "Именем Омниссии, исполни";
const char COMPLEX_OPERATOR_CLOSE_COMMAND[] = "Вознеси хвалу Омниссии";
const char IF_COMMAND[] = "Ежели, хвала Омниссиии";
const char END_OF_PROGRAMM_COMMAND[] = "Поблагодари святой когитатор за успешное выполнение";
const char ASSIGNMENT_COMMAND[] = "присвой значение этой частице Омниссии";
const char ADD_COMMAND[] = "сложи сии священные числа";
const char SUB_COMMAND[] = "вычти сии священные числа";
const char MUL_COMMAND[] = "умножь сии священные числа";
const char DIV_COMMAND[] = "раздели сии священные числа";
const char PRINT_COMMAND[] = "Поделись с адептом священным значением выражения";
const char INPUT_COMMAND[] = "Потребуй у адепта Бога-Машины значение числа";

node_t* infix_read(char* file_name, tree_t* tree)
{
    assert(file_name);
    assert(tree);

    FILE* input_address = NULL;
    struct stat statistics = {};
    char* buffer = NULL;
    char* buffer_begin = NULL;
    size_t buffer_len = 0;
    node_t* value = NULL;
    node_t** node_list = NULL;

    if (check_file_opening(file_name, &input_address, "r"))
        return NULL;

    if (fstat(fileno(input_address), &statistics))
        return NULL;

    buffer = (char*) calloc(statistics.st_size+1, sizeof(char));

    if (buffer == NULL)
        return NULL;

    buffer_begin = buffer;
    buffer_len = fread(buffer, sizeof(char), statistics.st_size, input_address);

    for (size_t i = 0; i < buffer_len; i++)
        printf("%c", buffer[i]);

    printf("\n");

    if (check_file_closing(input_address))
        printf("Error while closing file!");

    node_list = lexic_analiz(&buffer, tree);
    tree->node_list_begin = node_list;

    if (node_list == NULL)
        return NULL;

    printf("Lexic analiz is finished\n");

    value = get_g(&node_list, tree);

    printf("Syntax analiz is finished\n");

    free(buffer_begin);

    return value;
}

node_t** lexic_analiz(char** buffer, tree_t* tree)
{
    assert(buffer);
    assert(tree);

    char *buffer_at_start = *buffer;
    unsigned int node_list_len = 100;
    unsigned int node_number = 0;
    node_t** node_list = (node_t**) calloc(node_list_len, sizeof(node_t*));
    int val = 0;
    char var_name[256] = {0};
    char nul_str[256] = {0};

    if (node_list == NULL)
        return NULL;

    while (**buffer != 0)
    {
        //printf("buffer = [%c]\n", **buffer);
        if (node_number == node_list_len)
        {
            node_list_len *= 2;
            node_list = (node_t**) realloc(node_list, node_list_len*sizeof(node_t*));
        }

        if (strncmp(*buffer, COMPLEX_OPERATOR_OPEN_COMMAND, strlen(COMPLEX_OPERATOR_OPEN_COMMAND)) == 0)
        {
            node_list[node_number] = COPMLEX_OPERATOR_OPEN_();
            *buffer += strlen(COMPLEX_OPERATOR_OPEN_COMMAND);
        }
        else if (strncmp(*buffer, COMPLEX_OPERATOR_CLOSE_COMMAND, strlen(COMPLEX_OPERATOR_CLOSE_COMMAND)) == 0)
        {
            node_list[node_number] = COPMLEX_OPERATOR_CLOSE_();
            *buffer += strlen(COMPLEX_OPERATOR_CLOSE_COMMAND);
        }
        else if (strncmp(*buffer, WHILE_COMMAND, strlen(WHILE_COMMAND)) == 0)
        {
            node_list[node_number] = WHILE_();
            *buffer += strlen(WHILE_COMMAND);
        }
        else if (strncmp(*buffer, IF_COMMAND, strlen(IF_COMMAND)) == 0)
        {
            node_list[node_number] = IF_();
            *buffer += strlen(IF_COMMAND);
        }
        else if (strncmp(*buffer, END_OF_PROGRAMM_COMMAND, strlen(END_OF_PROGRAMM_COMMAND)) == 0)
        {
            node_list[node_number] = END_OF_PROGRAMM_();
            *buffer += strlen(END_OF_PROGRAMM_COMMAND);
        }
        else if (strncmp(*buffer, ASSIGNMENT_COMMAND, strlen(ASSIGNMENT_COMMAND)) == 0)
        {
            node_list[node_number] = ASSIGNMENT_();
            *buffer += strlen(ASSIGNMENT_COMMAND);
        }
        else if (strncmp(*buffer, ADD_COMMAND, strlen(ADD_COMMAND)) == 0)
        {
            node_list[node_number] = ADD_();
            *buffer += strlen(ADD_COMMAND);
        }
        else if (strncmp(*buffer, SUB_COMMAND, strlen(SUB_COMMAND)) == 0)
        {
            node_list[node_number] = SUB_();
            *buffer += strlen(SUB_COMMAND);
        }
        else if (strncmp(*buffer, MUL_COMMAND, strlen(MUL_COMMAND)) == 0)
        {
            node_list[node_number] = MUL_();
            *buffer += strlen(MUL_COMMAND);
        }
        else if (strncmp(*buffer, DIV_COMMAND, strlen(DIV_COMMAND)) == 0)
        {
            node_list[node_number] = DIV_();
            *buffer += strlen(DIV_COMMAND);
        }
        else if (strncmp(*buffer, PRINT_COMMAND, strlen(PRINT_COMMAND)) == 0)
        {
            node_list[node_number] = PRINT_();
            *buffer += strlen(PRINT_COMMAND);
        }
        else if (strncmp(*buffer, INPUT_COMMAND, strlen(INPUT_COMMAND)) == 0)
        {
            node_list[node_number] = INPUT_();
            *buffer += strlen(INPUT_COMMAND);
        }
        else if (**buffer == ';')
        {
            node_list[node_number] = OP_END_();
            (*buffer)++;
        }
        else if (**buffer == '(')
        {
            node_list[node_number] = PAR_OPEN_();
            (*buffer)++;
        }
        else if (**buffer == ')')
        {
            node_list[node_number] = PAR_CLOSE_();
            (*buffer)++;
        }
        else if (isdigit(**buffer))//TODO отрицательные числа
        {
            val = 0;
            buffer_at_start = *buffer;

            while ('0' <= **buffer && **buffer <= '9')
            {
                val = val*10 + (**buffer - '0');
                (*buffer)++;
            }
            if (buffer_at_start != *buffer)
                node_list[node_number] = NUM_(val);
        }
        else if (isalpha(**buffer) || **buffer == '_')
        {
            strncpy(var_name, nul_str, 256);
            for (int i = 0; i < 255; i++)
            {
                if (isalpha(**buffer) || **buffer == '_')
                    var_name[i] = **buffer;
                else
                    break;

                (*buffer)++;
            }

            node_list[node_number] = make_new_var(var_name, tree);
        }
        else if (isspace(**buffer))
        {
            (*buffer)++;
            node_number--;
        }
        else
        {
            printf("Unknown_command! buffer = %s\n", *buffer);
            return NULL;
        }

        node_number++;
    }

    tree->node_list_len = node_list_len;

    //for (size_t i = 0; i < node_number; i++)
    //    printf("node_list[%ld] = %d\n", i, node_list[i]->value.number_value);

    return node_list;
}

node_t* get_g(node_t*** node_list, tree_t* tree)
{
    assert(node_list);
    assert(tree);

    node_t* val = get_op_list(node_list, tree);

    if ((**node_list)->type != OPERATOR_TYPE || (**node_list)->value.operator_name != END_OF_PROGRAMM)
        printf("Syntax_Error in get_g\n");

    (*node_list)++;
    return val;
}

node_t* get_op_list(node_t*** node_list, tree_t* tree)
{
    assert(node_list);
    assert(tree);

    node_t* op = get_op(node_list, tree);
    node_t* root = NULL;
    node_t* op_end = NULL;

    //printf("%p", op);

    while (op != NULL && (**node_list)->type == OPERATOR_TYPE && (**node_list)->value.operator_name == OP_END)
    {
        if (root == NULL)
        {
            root = **node_list;
            op_end = **node_list;
        }
        else
        {
            op_end->right = **node_list;
            op_end = **node_list;
        }
        op_end->left = op;
        (*node_list)++;
        op  = get_op(node_list, tree);
    }

    if ((**node_list)->type == OPERATOR_TYPE && (**node_list)->value.operator_name == END_OF_PROGRAMM)
        op_end->right = **node_list;
    if (root == NULL)
        printf("Error in get_op_list!\n");

    return root;
}

node_t* get_op(node_t*** node_list, tree_t* tree)
{
    assert(node_list);
    assert(tree);

    node_t* op = NULL;

    if ((**node_list)->type == OPERATOR_TYPE && (**node_list)->value.operator_name == COPMLEX_OPERATOR_OPEN)
    {
        (*node_list)++;
        op = get_op_list(node_list, tree);

        if ((**node_list)->type == OPERATOR_TYPE && (**node_list)->value.operator_name == COPMLEX_OPERATOR_CLOSE)
            (*node_list)++;
        else
        {
            printf("Error in get_op: NO COPMLEX_OPERATOR_CLOSE!\n");
            return NULL;
        }
    }
    else
    {
        op = get_if(node_list, tree);
        if (op == NULL)
            op = get_while(node_list, tree);

        if (op == NULL)
            op = get_a(node_list, tree);

        if (op == NULL)
            op = get_print(node_list, tree);

        if (op == NULL)
            op = get_input(node_list, tree);
    }

    return op;
}

node_t* get_print(node_t*** node_list, tree_t* tree)
{
    assert(node_list);
    assert(tree);

    node_t* print = NULL;
    node_t* equation = NULL;

    if ((**node_list)->type == OPERATOR_TYPE && (**node_list)->value.operator_name == PRINT)
    {
        print = **node_list;
        (*node_list)++;
        if ((**node_list)->type == OPERATOR_TYPE && (**node_list)->value.operator_name == PAR_OPEN)
        {
            (*node_list)++;
            equation = get_e(node_list, tree);
            if (equation == NULL ||
                !((**node_list)->type == OPERATOR_TYPE && (**node_list)->value.operator_name == PAR_CLOSE))
            {
                printf("Error in get_print!\n");
                return NULL;
            }
            else
                (*node_list)++;
        }
    }
    else
        return NULL;

    print->left = equation;
    return print;
}

node_t* get_input(node_t*** node_list, tree_t* tree)
{
    assert(node_list);
    assert(tree);

    node_t* input = NULL;
    node_t* var = NULL;

    if ((**node_list)->type == OPERATOR_TYPE && (**node_list)->value.operator_name == INPUT)
    {
        input = **node_list;
        (*node_list)++;
        if ((**node_list)->type == OPERATOR_TYPE && (**node_list)->value.operator_name == PAR_OPEN)
        {
            (*node_list)++;
            var = get_v(node_list, tree);
            if (var == NULL ||
                !((**node_list)->type == OPERATOR_TYPE && (**node_list)->value.operator_name == PAR_CLOSE))
            {
                printf("Error in get_print!\n");
                return NULL;
            }
            else
                (*node_list)++;
        }
        else
        {
            printf("Error in get_input!\n");
        }
    }
    else
        return NULL;

    input->left = var;

    return input;
}

node_t* get_if(node_t*** node_list, tree_t* tree)
{
    assert(node_list);
    assert(tree);

    node_t* condition = NULL;
    node_t* equation = NULL;
    node_t* condition_body = NULL;

    if ((**node_list)->type == OPERATOR_TYPE && (**node_list)->value.operator_name == IF)
    {
        condition = **node_list;
        (*node_list)++;
    }
    else
        return NULL;

    if ((**node_list)->type == OPERATOR_TYPE && (**node_list)->value.operator_name == PAR_OPEN)
    {
        (*node_list)++;
        equation = get_e(node_list, tree);

        if (equation == NULL)
        {
            (*node_list)++;
            return NULL;
        }

        if ((**node_list)->type == OPERATOR_TYPE && (**node_list)->value.operator_name == PAR_CLOSE)
            (*node_list)++;
        else
        {
            printf("Error in get_while - no par_close!\n");
            return NULL;
        }
    }

    condition_body = get_op(node_list, tree);

    if (condition_body == NULL)
        return NULL;

    condition->left = equation;
    condition->right = condition_body;

    return condition;
}

node_t* get_while(node_t*** node_list, tree_t* tree)
{
    assert(node_list);
    assert(tree);

    node_t* cycle = NULL;
    node_t* condition = NULL;
    node_t* cycle_body = NULL;
    node_t** node_list_at_start = *node_list;

    if ((**node_list)->type == OPERATOR_TYPE && (**node_list)->value.operator_name == WHILE)
    {
        cycle = **node_list;
        (*node_list)++;
    }
    else
        return NULL;

    if ((**node_list)->type == OPERATOR_TYPE && (**node_list)->value.operator_name == PAR_OPEN)
    {
        (*node_list)++;
        condition = get_e(node_list, tree);

        if (condition == NULL)
        {
            *node_list = node_list_at_start;
            return NULL;
        }

        if ((**node_list)->type == OPERATOR_TYPE && (**node_list)->value.operator_name == PAR_CLOSE)
            (*node_list)++;
        else
        {
            printf("Error in get_while - no par_close!\n");
            return NULL;
        }
    }

    cycle_body = get_op(node_list, tree);

    if (cycle_body == NULL)
    {
        *node_list = node_list_at_start;
        return NULL;
    }

    cycle->left = condition;
    cycle->right = cycle_body;

    return cycle;
}

node_t* get_a(node_t*** node_list, tree_t* tree)
{
    assert(node_list);
    assert(tree);

    node_t* var = NULL;
    node_t* assignment = NULL;
    node_t* val = NULL;

    if ((**node_list)->type == VARIABLE_TYPE)
    {
        var = **node_list;
        (*node_list)++;
    }
    else
        return NULL;

    //printf("1\n");
    //printf("%d = %d\n", (**node_list)->value.operator_name, ASSIGNMENT);

    if ((**node_list)->type == OPERATOR_TYPE && (**node_list)->value.operator_name == ASSIGNMENT)
    {
        assignment = **node_list;
        (*node_list)++;
    }
    else
        return NULL;

    //printf("1\n");
    val = get_e(node_list, tree);

    if (val == NULL)
        return NULL;

    assignment->left = var;
    assignment->right = val;
    //printf("1\n");
    return assignment;
}

node_t* get_e(node_t*** node_list, tree_t* tree)
{
    assert(node_list);
    assert(tree);

    node_t* val = get_t(node_list, tree);
    node_t* operator_node = NULL;

    while ((**node_list)->type == OPERATOR_TYPE &&
           ((**node_list)->value.operator_name == ADD || (**node_list)->value.operator_name == SUB))
    {
        operator_node = **node_list;
        (*node_list)++;

        operator_node->left = val;
        operator_node->right = get_t(node_list, tree);

        val = operator_node;
    }

    return val;
}

node_t* get_t(node_t*** node_list, tree_t* tree)
{
    assert(node_list);
    assert(tree);

    node_t* val = get_p(node_list, tree);
    node_t* operator_node = NULL;

    while ((**node_list)->type == OPERATOR_TYPE &&
           ((**node_list)->value.operator_name == MUL || (**node_list)->value.operator_name == DIV))
    {
        operator_node = **node_list;
        (*node_list)++;

        operator_node->left = val;
        operator_node->right = get_p(node_list, tree);

        val = operator_node;
    }

    return val;
}

node_t* get_p(node_t*** node_list, tree_t* tree)
{
    assert(node_list);
    assert(tree);

    node_t* val = NULL;

    if ((**node_list)->type == OPERATOR_TYPE && (**node_list)->value.operator_name == PAR_OPEN)
    {
        (*node_list)++;
        val = get_e(node_list, tree);

        if ((**node_list)->type == OPERATOR_TYPE && (**node_list)->value.operator_name == PAR_CLOSE)
            (*node_list)++;
        else
            printf("Syntax_Error in get_p\n");

        return val;
    }
    else
        val = get_n(node_list);
    if (val == NULL)
        val = get_v(node_list, tree);

    return val;
}

node_t* get_n(node_t*** node_list)
{
    assert(node_list);

    node_t* val = **node_list;

    if (val->type == NUMBER_TYPE)
    {
        (*node_list)++;
        return val;
    }
    else return NULL;
}

node_t* get_v(node_t*** node_list, tree_t* tree)
{
    assert(node_list);
    assert(tree);

    node_t* val = **node_list;

    if (val->type == VARIABLE_TYPE)
    {
        (*node_list)++;
        return val;
    }
    else return NULL;
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
#undef PAR_OPEN_
#undef PAR_CLOSE_
#undef OP_END_
#undef COPMLEX_OPERATOR_OPEN_
#undef COPMLEX_OPERATOR_CLOSE_
#undef PRINT_
#undef INPUT_
