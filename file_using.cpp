#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "file_using.h"

bool check_file_opening(const char * const input_address, FILE** file_ptr, const char* const access_rights)
{
    if ((*file_ptr = fopen(input_address, access_rights)) == NULL)
    {
        fprintf(stderr, "Can't open file\" %s\"\n", input_address);
        return 1;
    }

    return 0;
}

bool check_file_closing(FILE* input_address)
{
    assert(input_address);

    if (fclose(input_address) != 0)
    {
        fprintf(stderr, "Error while closing file\n");
        return 1;
    }

    return 0;
}

