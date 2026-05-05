#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include "file_using.h"
#include "language.h"

static bool check_file_founded(int argc, int number_of_files);

const int NUMBER_OF_FILES = 2;

int main(int argc, char* argv[])
{
    tree_t* tree = NULL;
    FILE* asm_address = NULL;
    int label_number = 0;

    if (check_file_founded(argc, NUMBER_OF_FILES))
        return FILES_NOT_FOUNDED_ERROR;

    if (tree_init(&tree))
        return ALLOCATION_ERROR;

    if (tree_input(tree, argv[1]))
        return TREE_READING_ERROR;

    if (check_file_opening(argv[2], &asm_address, "w+"))
        return FILE_OPENING_ERROR;
    /*
    if (node_assemblating(tree->root, tree, asm_address, &label_number))
        return ASM_MAKING_ERROR;
    */
    printf("Making asm code is finished\n");

    if (check_file_closing(asm_address))
        printf("Error while closing file!");

    if (tree_dump(tree))
        return DUMP_ERROR;

    tree_destroy(tree);

    printf("Back end success!\n");

    return NO_ERROR;
}

bool check_file_founded(int argc, int number_of_files)
{
    if (argc < number_of_files)
    {
        fprintf(stderr, "Files not founded. Please, give programm file with code tree\n");
        return 1;
    }

    return 0;
}
