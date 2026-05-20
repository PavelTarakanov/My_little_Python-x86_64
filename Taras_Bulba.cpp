#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <sys/stat.h>
#include "language.h"

const unsigned int VAR_LIST_LEN = 100;
const unsigned int START_CODE_LEN = 0x5000;

tree_errors back_end_base_init(back_end_base_t** back_end_base)
{
    assert(back_end_base);

    (*back_end_base) = (back_end_base_t*) calloc(1, sizeof(back_end_base_t));

    if ((*back_end_base) == NULL)
        return ALLOCATION_ERROR;

    (*back_end_base)->byte_code = (uint8_t*) calloc(START_CODE_LEN, sizeof(uint8_t));

    if ((*back_end_base)->byte_code == NULL)
        return ALLOCATION_ERROR;

    (*back_end_base)->regs = (register_info_t*) calloc(NUMBER_OF_REGS, sizeof(register_info_t));

    if ((*back_end_base)->regs == NULL)
        return ALLOCATION_ERROR;

    for (int i = 0; i < NUMBER_OF_REGS; i++)
    {
        (*back_end_base)->regs[i].free_flag = true;
        (*back_end_base)->regs[i].reg_value = 0;
    }

    (*back_end_base)->regs[4].free_flag = false;
    (*back_end_base)->regs[5].free_flag = false;//заблокировал rsp и rbp

    (*back_end_base)->tree = (tree_t*) calloc(1, sizeof(tree_t));

    if ((*back_end_base)->tree == NULL)
        return ALLOCATION_ERROR;

    (*back_end_base)->tree->root = NULL;
    (*back_end_base)->tree->number_of_variables = 0;
    (*back_end_base)->tree->variable_list = (variable_t*) calloc(VAR_LIST_LEN, sizeof(variable_t));

    if ((*back_end_base)->tree->variable_list == NULL)
        return ALLOCATION_ERROR;

    return NO_ERROR;
}

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

void back_end_destroy(back_end_base_t* back_end_base)
{
    assert(back_end_base);

    tree_destroy(back_end_base->tree);
    free(back_end_base->byte_code);
    free(back_end_base->regs);
    free(back_end_base);
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
