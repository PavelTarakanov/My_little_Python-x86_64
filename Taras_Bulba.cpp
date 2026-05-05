#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include "file_using.h"
#include "language.h"

const unsigned int VAR_LIST_LEN = 100;

tree_errors tree_init(tree_t** tree)
{
    assert(tree);

    (*tree) = (tree_t*) calloc(1, sizeof(tree_t));
    if ((*tree) == NULL)
        return ALLOCATION_ERROR;

    (*tree)->root = NULL;
    (*tree)->number_of_variables = 0;
    (*tree)->variable_list = (variable_t*) calloc(VAR_LIST_LEN, sizeof(variable_t));

    if ((*tree)->variable_list == NULL)
        return ALLOCATION_ERROR;

    return NO_ERROR;
}

node_t* node_init(tree_elem_t value, type_t type, node_t* left, node_t* right)
{
    node_t* node = (node_t*) calloc(1, sizeof(node_t));

    if (node == NULL)
        return NULL;

    node->type = type;
    node->value = value;
    node->left = left;
    node->right = right;

    return node;
}

void node_destroy(node_t* node)
{
    assert(node);

    if (node->left != NULL)
        node_destroy(node->left);
    if (node->right != NULL)
        node_destroy(node->right);

    free(node);

    return;
}

void infix_tree_destroy(tree_t* tree)
{
    assert(tree);

    for (unsigned int i = 0; i < tree->node_list_len; i++)
        free(tree->node_list_begin[i]);

    for (int i = 0; i < tree->number_of_variables; i++)
        free(tree->variable_list[i].var_name);

    free(tree->node_list_begin);
    free(tree->variable_list);
    free(tree);
}

void tree_destroy(tree_t* tree)
{
    assert(tree);

    if (tree->root != NULL)
        node_destroy(tree->root);

    for (int i = 0; i < tree->number_of_variables; i++)
        free(tree->variable_list[i].var_name);

    free(tree->variable_list);
    free(tree);
}

void make_parents(node_t* node, node_t* parent)
{
    assert(node);

    node->parent = parent;
    if (node->left != NULL) make_parents(node->left, node);
    if (node->right != NULL) make_parents(node->right, node);

    return;
}
