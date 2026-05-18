#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <sys/stat.h>
#include "file_using.h"
#include "language.h"

static bool check_file_founded(int argc, int number_of_files);

const int NUMBER_OF_FILES = 2;

int main(int argc, char* argv[])//TODO объединить функции в более крупные: закрытие файлов и free в одну, открытие и чтение в другую
{
    back_end_base_t* back_end_base = NULL;
    FILE* asm_address = NULL;
    FILE* byte_code_address = NULL;

    if (check_file_founded(argc, NUMBER_OF_FILES))
        return FILES_NOT_FOUNDED_ERROR;

    if (back_end_base_init(&back_end_base))
        return ALLOCATION_ERROR;

    if (tree_input(back_end_base->tree, argv[1]))
    {
        back_end_destroy(back_end_base);
        return TREE_READING_ERROR;
    }

    if (check_file_opening(argv[2], &asm_address, "w+"))
    {
        back_end_destroy(back_end_base);
        return FILE_OPENING_ERROR;
    }

    if (check_file_opening(argv[3], &byte_code_address, "w+"))
    {
        back_end_destroy(back_end_base);
        return FILE_OPENING_ERROR;
    }

    printf("Start asm code making\n");

    if (make_asm_code(back_end_base, byte_code_address, asm_address))
        return ASM_MAKING_ERROR;

    printf("Making asm code is finished\n");

    if (check_file_closing(asm_address))
        printf("Error while closing file!");

    if (tree_dump(back_end_base->tree))
    {
        back_end_destroy(back_end_base);
        return DUMP_ERROR;
    }

    system("nasm -f elf64 -o test.o asm.txt");
    system("ld -o test test.o");

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
