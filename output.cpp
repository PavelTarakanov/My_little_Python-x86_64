#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include "language.h"

tree_errors node_output(node_t* node, tree_t* tree, FILE* output_address)
{
    assert(node);
    assert(tree);
    assert(output_address);

    fprintf(output_address, "( ");
    if (node->type == NUMBER_TYPE)
        fprintf(output_address, "%d", node->value.number_value);
    else if (node->type == VARIABLE_TYPE)
        fprintf(output_address, "%s", tree->variable_list[node->value.variable_number].var_name);
    else if (node->type == OPERATOR_TYPE)
    {
        switch (node->value.operator_name)
        {
        case ADD:
            fprintf(output_address, "+");
            break;
        case SUB:
            fprintf(output_address, "-");
            break;
        case MUL:
            fprintf(output_address, "*");
            break;
        case DIV:
            fprintf(output_address, "/");
            break;
        case ASSIGNMENT:
            fprintf(output_address, "=");
            break;
        case IF:
            fprintf(output_address, "IF");
            break;
        case WHILE:
            fprintf(output_address, "WHILE");
            break;
        case END_OF_PROGRAMM:
            fprintf(output_address, "END_OF_PROGRAMM");
            break;
        case PAR_OPEN:
            fprintf(output_address, "(");
            break;
        case PAR_CLOSE:
            fprintf(output_address, ")");
            break;
        case OP_END:
            fprintf(output_address, ";");
            break;
        case COPMLEX_OPERATOR_OPEN:
            fprintf(output_address, "{");
            break;
        case COPMLEX_OPERATOR_CLOSE:
            fprintf(output_address, "}");
            break;
        case FUNC:
            fprintf(output_address, "FUNC");
            break;
        case PRINT:
            fprintf(output_address, "PRINT");
            break;
        case INPUT:
            fprintf(output_address, "INPUT");
            break;
        default:
            printf("ERROR: unknown operator");
            return UNKNOWN_OPERATOR_ERROR;
        }
    }
    else
        return TYPE_ERROR;

    fprintf(output_address, " ");

    if (node->left == NULL)
        fprintf(output_address, "nil ");
    else
        node_output(node->left, tree, output_address);

    if (node->right == NULL)
        fprintf(output_address, "nil ");
    else
        node_output(node->right, tree, output_address);

    fprintf(output_address, ")");

    return NO_ERROR;
}
