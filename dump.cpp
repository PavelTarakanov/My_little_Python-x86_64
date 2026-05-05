#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include "file_using.h"
#include "language.h"

static tree_errors node_visual_dump(node_t* node, FILE* const dump_address, char* node_way, tree_t* tree);
static tree_errors print_label(FILE* dump_address, node_t* node, tree_t* tree);
static tree_errors operator_print(FILE* dump_address, node_t* node);

tree_errors tree_dump(tree_t* tree)
{
    assert(tree);

    FILE* dump_address = NULL;
    char root_address[10] = "Z";

    if (check_file_opening("dump.txt" , &dump_address, "w+"))
        return FILE_OPENING_ERROR;

    fprintf(dump_address, "digraph{\n");

    if (node_visual_dump(tree->root, dump_address, root_address, tree))
        return DUMP_ERROR;

    fprintf(dump_address, "}");

    if (check_file_closing(dump_address))
        return FILE_CLOSING_ERROR;

    if (system("dot dump.txt -T png -o visual_dump.png") != 0)
        return GRAPH_MAKING_ERROR;

    return NO_ERROR;
}

tree_errors node_visual_dump(node_t* node, FILE* const dump_address, char* node_way, tree_t* tree)
{
    assert(node);
    assert(dump_address);

    char *left_way = (char*) calloc(strlen(node_way) + 2, sizeof(char));
    char *right_way = (char*) calloc(strlen(node_way) + 2 , sizeof(char));
    tree_errors error = NO_ERROR;

    if (left_way == NULL || right_way == NULL)
        return ALLOCATION_ERROR;

    strcpy(left_way, node_way);
    strcpy(right_way, node_way);


    fprintf(dump_address, "\t%s", node_way);
    if (node->type == OPERATOR_TYPE)
        fprintf(dump_address, "[shape = record, style=\"filled\",fillcolor=\"lightgreen\", ");
    else if (node->type == VARIABLE_TYPE)
        fprintf(dump_address, "[shape = record, style=\"filled\",fillcolor=\"lightblue\", ");
    else if (node->type == NUMBER_TYPE)
        fprintf(dump_address, "[shape = record, style=\"filled\",fillcolor=\"yellow\", ");
    else
        return TYPE_ERROR;

    print_label(dump_address, node, tree);

    if (node->left != NULL)
    {
        fprintf(dump_address, "\t%s -> %s\n", node_way, strcat(left_way, "L"));
        error = node_visual_dump(node->left, dump_address, left_way, tree);

        if (error)
            return error;
    }
    if (node->right != NULL)
    {
        fprintf(dump_address, "\t%s -> %s\n", node_way, strcat(right_way, "R"));
        error = node_visual_dump(node->right, dump_address, right_way, tree);

        if (error)
            return error;
    }

    free(left_way);
    free(right_way);

    return error;
}

tree_errors print_label(FILE* dump_address, node_t* node, tree_t* tree)
{
    assert(dump_address);
    assert(node);

    if (node->type == NUMBER_TYPE)
        fprintf(dump_address, "label=\"{%d}\"];\n", node->value.number_value);
    else if (node->type == OPERATOR_TYPE)
    {
        if (operator_print(dump_address, node))
            return UNKNOWN_OPERATOR_ERROR;
    }
    else if (node->type == VARIABLE_TYPE)
        fprintf(dump_address, "label=\"{%s}\"];\n", tree->variable_list[node->value.variable_number].var_name);

    return NO_ERROR;
}

tree_errors operator_print(FILE* dump_address, node_t* node)
{
    assert(dump_address);
    assert(node);

    switch(node->value.operator_name)
    {
        case ADD:
            fprintf(dump_address, "label=\"{+}\"];\n");
            break;
        case SUB:
            fprintf(dump_address, "label=\"{-}\"];\n");
            break;
        case MUL:
            fprintf(dump_address, "label=\"{*}\"];\n");
            break;
        case DIV:
            fprintf(dump_address, "label=\"{/}\"];\n");
            break;
        case ASSIGNMENT:
            fprintf(dump_address, "label=\"{=}\"];\n");
            break;
        case IF:
            fprintf(dump_address, "label=\"{IF}\"];\n");
            break;
        case WHILE:
            fprintf(dump_address, "label=\"{WHILE}\"];\n");
            break;
        case END_OF_PROGRAMM:
            fprintf(dump_address, "label=\"{END_OF_PROGRAMM}\"];\n");
            break;
        case PAR_OPEN:
            fprintf(dump_address, "label=\"{PAR_OPEN}\"];\n");
            break;
        case PAR_CLOSE:
            fprintf(dump_address, "label=\"{PAR_CLOSE}\"];\n");
            break;
        case OP_END:
            fprintf(dump_address, "label=\"{OP_END}\"];\n");
            break;
        case COPMLEX_OPERATOR_OPEN:
            fprintf(dump_address, "label=\"{COPMLEX_OPERATOR_OPEN}\"];\n");
            break;
        case COPMLEX_OPERATOR_CLOSE:
            fprintf(dump_address, "label=\"{COPMLEX_OPERATOR_CLOSE}\"];\n");
            break;
        case FUNC:
            fprintf(dump_address, "label=\"{FUNC}\"];\n");
            break;
        case PRINT:
            fprintf(dump_address, "label=\"{PRINT}\"];\n");
            break;
        case INPUT:
            fprintf(dump_address, "label=\"{INPUT}\"];\n");
            break;
        default:
            printf("ERROR: unknown operator");
            return UNKNOWN_OPERATOR_ERROR;
        }

    return NO_ERROR;
}
