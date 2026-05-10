#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <stdint.h>
#include "file_using.h"
#include "language.h"

#define free_reg(reg_number) back_end_base->regs[reg_number].free_flag = true
#define print_add(bite_code_address, reg_1, reg_2) fputc(0xc0 + (reg_2 << 3) + reg_1, bite_code_address)

static const char* REG_NAMES[8] = {"eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi"};

static uint8_t find_free_reg(back_end_base_t* back_end_base);
static void print_const(FILE* bite_code_address, int value);

uint8_t make_node_code(node_t* node, back_end_base_t* back_end_base,
                           FILE* bite_code_address, FILE* asm_code_address)
{
    assert(node);
    assert(back_end_base);
    assert(bite_code_address);
    assert(asm_code_address);

    uint8_t reg_1 = 0xff;
    uint8_t reg_2 = 0xff;

    //printf("type = %d\nvalue = %d\n", node->type, node->value.number_value);

    if (node->type == NUMBER_TYPE)
    {
        reg_1 = find_free_reg(back_end_base);
        if (reg_1 >= NUMBER_OF_REGS)//TODO если нет свободных регистров
        {
            fprintf(asm_code_address, " ");
        }
        else
        {
            fprintf(asm_code_address, "mov %s, %d\n", REG_NAMES[reg_1], node->value.number_value);
            fputc(0xb8 + reg_1, bite_code_address);
            print_const(bite_code_address, node->value.number_value);
        }
        return reg_1;
    }
    if (node->type == VARIABLE_TYPE)
    {
        reg_1 = find_free_reg(back_end_base);
        if (reg_1 >= NUMBER_OF_REGS)
        {
            fprintf(asm_code_address, " ");
        }
        else//TODO ассемблерный вывод
        {
            fprintf(asm_code_address, "mov %s, [%s]\n", REG_NAMES[reg_1],
                    back_end_base->tree->variable_list[node->value.variable_number].var_name);
        }

        return reg_1;
    }
    else if (node->type == OPERATOR_TYPE)
    {
        switch(node->value.operator_name)
        {
            case ADD:
                reg_1 = make_node_code(node->left, back_end_base, bite_code_address, asm_code_address);
                reg_2 = make_node_code(node->right, back_end_base, bite_code_address, asm_code_address);
                fprintf(asm_code_address, "add %s, %s\n", REG_NAMES[reg_1], REG_NAMES[reg_2]);
                fputc(0x01, bite_code_address);
                print_add(bite_code_address, reg_1, reg_2);
                free_reg(reg_2);
                return reg_1;
            case SUB:
                reg_1 = make_node_code(node->left, back_end_base, bite_code_address, asm_code_address);
                reg_2 = make_node_code(node->right, back_end_base, bite_code_address, asm_code_address);
                fprintf(asm_code_address, "sub %s, %s\n", REG_NAMES[reg_1], REG_NAMES[reg_2]);
                free_reg(reg_2);
                return reg_1;
            case MUL:
                reg_1 = make_node_code(node->left, back_end_base, bite_code_address, asm_code_address);
                reg_2 = make_node_code(node->right, back_end_base, bite_code_address, asm_code_address);
                fprintf(asm_code_address, "mul %s, %s\n", REG_NAMES[reg_1], REG_NAMES[reg_2]);
                free_reg(reg_2);
                return reg_1;
            case DIV://TODO
            case ASSIGNMENT:
                reg_1 = make_node_code(node->right, back_end_base, bite_code_address, asm_code_address);
                fprintf(asm_code_address, "mov [%s], %s\n",
                        back_end_base->tree->variable_list[node->left->value.variable_number].var_name,
                        REG_NAMES[reg_1]);
                free_reg(reg_1);
                return 0;
            case IF:

            case WHILE:
            case END_OF_PROGRAMM:
                return 0x0;
            case PAR_OPEN:
            case PAR_CLOSE:
                return 0xff;
            case OP_END:
                make_node_code(node->left, back_end_base, bite_code_address, asm_code_address);
                if (node->right != NULL) make_node_code(node->right, back_end_base, bite_code_address, asm_code_address);
                return 0x0;
            case COPMLEX_OPERATOR_OPEN:
            case COPMLEX_OPERATOR_CLOSE:
            case FUNC:
            case PRINT:
            case INPUT:
            default:
                printf("ERROR: unknown operator");
                return 0xff;
        }
    }

    return 0;
}

uint8_t find_free_reg(back_end_base_t* back_end_base)
{
    assert(back_end_base);

    for (uint8_t i = 0; i < NUMBER_OF_REGS; i++)
        if (back_end_base->regs[i].free_flag == true)
        {
            back_end_base->regs[i].free_flag = false;
            //printf("Finded = %d\n", i);
            return i;
        }

    //printf("Not founded!\n");

    return 0xff;
}

void print_const(FILE* bite_code_address, int value)
{
    assert(bite_code_address);

    for (int i = 0; i < 4; i++)
    {
        fputc((uint8_t) value, bite_code_address);
        value = value >> 8;
    }
}
