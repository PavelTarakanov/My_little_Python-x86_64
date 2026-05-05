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
    FILE* output_address = NULL;

    if (check_file_founded(argc, NUMBER_OF_FILES))
        return FILES_NOT_FOUNDED_ERROR;

    if (tree_init(&tree))
        return ALLOCATION_ERROR;

    tree->root = infix_read(argv[1], tree);
    tree_dump(tree);

    if (check_file_opening(argv[2], &output_address, "w+"))
        return FILE_OPENING_ERROR;

    node_output(tree->root, tree, output_address);

    printf("Front end success!\n");

    if (check_file_closing(output_address))
        printf("Error whil1e closing file!");

    infix_tree_destroy(tree);
}

bool check_file_founded(int argc, int number_of_files)
{
    if (argc < number_of_files)
    {
        fprintf(stderr, "Files not founded. Please, give programm two files "
                        "- file with code on Omnissiah's language and file for tree input\n");
        return 1;
    }

    return 0;
}
