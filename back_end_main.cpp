#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>
#include "language.h"

static bool check_file_founded(int argc, int number_of_files);

const int NUMBER_OF_FILES = 2;

int main(int argc, char* argv[])
{
    back_end_base_t* back_end_base = NULL;

    if (check_file_founded(argc, NUMBER_OF_FILES))
        return FILES_NOT_FOUNDED_ERROR;

    if (back_end_base_init(&back_end_base))
        return ALLOCATION_ERROR;

    if (tree_input(back_end_base->tree, argv[1]))
    {
        back_end_destroy(back_end_base);
        return TREE_READING_ERROR;
    }

    if (make_asm_code(back_end_base, argv))
        return ASM_MAKING_ERROR;

    if (tree_dump(back_end_base->tree))
    {
        back_end_destroy(back_end_base);
        return DUMP_ERROR;
    }

    back_end_destroy(back_end_base);

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
