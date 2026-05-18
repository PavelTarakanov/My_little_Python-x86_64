#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <sys/stat.h>
#include <stdint.h>
#include "commands.h"
#include "language.h"

#define free_reg(reg_number) back_end_base->regs[reg_number].free_flag = true
#define print_reg_to_reg(byte_code_address, reg_1, reg_2) fputc(0xc0 + (reg_2 << 3) + reg_1, byte_code_address)
//reg_1 приёмник, reg_2 источник
#define print_elf_header(byte_code_address) for (int i = 0; i < 64; i++) fputc(ELF_HEADER[i], byte_code_address)
static const char* REG_NAMES[8] = {"rax", "rcx", "rdx", "rbx", "rsp", "rbp", "rsi", "rdi"};

static uint8_t make_node_code(node_t* node, back_end_base_t* back_end_base,
                           FILE* bite_code_address, FILE* asm_code_address);
static uint8_t find_free_reg(back_end_base_t* back_end_base);
static void print_const(FILE* byte_code_address, int value);
static void print_add(FILE* byte_code_address,
               uint8_t term_1, uint8_t term_2);
static void print_sub(FILE* byte_code_address,
               uint8_t minuend, uint8_t subtrahend);
static void print_mul(FILE* byte_code_address,
               uint8_t multiplier_1, uint8_t multiplier_2);
static void print_div(FILE* byte_code_address,
               uint8_t transmitter_register, uint8_t receiver_register);
static void print_mov_reg_to_reg(FILE* byte_code_address,
               uint8_t transmitter_register, uint8_t receiver_register);
static void print_lib(FILE* asm_code_address, FILE* byte_code_address);

tree_errors make_asm_code(back_end_base_t* back_end_base,
                           FILE* byte_code_address, FILE* asm_code_address)
{
    assert(back_end_base);
    assert(byte_code_address);
    assert(asm_code_address);

    fprintf(asm_code_address, "section .data\n");

    for(int i = 0; i < back_end_base->tree->number_of_variables; i++)
    {
        fprintf(asm_code_address, "\t%s dd 0\n", back_end_base->tree->variable_list[i].var_name);
    }

    fprintf(asm_code_address, "\toutput_str times 20 db 0\n");

    fprintf(asm_code_address, "section .text\n\tglobal _start\n_start:\n");

    print_elf_header(byte_code_address);

    if (make_node_code(back_end_base->tree->root, back_end_base,
                       byte_code_address, asm_code_address))
        return ASM_MAKING_ERROR;

    print_lib(asm_code_address, byte_code_address);

    return NO_ERROR;
}

uint8_t make_node_code(node_t* node, back_end_base_t* back_end_base,
                           FILE* byte_code_address, FILE* asm_code_address)
{
    assert(node);
    assert(back_end_base);
    assert(byte_code_address);
    assert(asm_code_address);

    uint8_t reg_1 = 0xff;
    uint8_t reg_2 = 0xff;
    unsigned int actual_label_number = back_end_base->label_number;

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
            fprintf(asm_code_address, "\tmov %s, %d\n", REG_NAMES[reg_1], node->value.number_value);
            fputc(0xb8 + reg_1, byte_code_address);
            print_const(byte_code_address, node->value.number_value);
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
        else//TODO бинарный вывод
        {
            fprintf(asm_code_address, "\tmov %s, [%s]\n", REG_NAMES[reg_1],
                    back_end_base->tree->variable_list[node->value.variable_number].var_name);
        }

        return reg_1;
    }
    else if (node->type == OPERATOR_TYPE)
    {
        switch(node->value.operator_name)
        {
            case ADD:
                reg_1 = make_node_code(node->left, back_end_base, byte_code_address, asm_code_address);
                reg_2 = make_node_code(node->right, back_end_base, byte_code_address, asm_code_address);

                fprintf(asm_code_address, "\tadd %s, %s\n", REG_NAMES[reg_1], REG_NAMES[reg_2]);

                print_add(byte_code_address, reg_1, reg_2);

                free_reg(reg_2);
                return reg_1;
            case SUB:
                reg_1 = make_node_code(node->left, back_end_base, byte_code_address, asm_code_address);
                reg_2 = make_node_code(node->right, back_end_base, byte_code_address, asm_code_address);

                fprintf(asm_code_address, "\tsub %s, %s\n", REG_NAMES[reg_1], REG_NAMES[reg_2]);

                print_sub(byte_code_address, reg_1, reg_2);

                free_reg(reg_2);
                return reg_1;
            case MUL:
                reg_1 = make_node_code(node->left, back_end_base, byte_code_address, asm_code_address);
                reg_2 = make_node_code(node->right, back_end_base, byte_code_address, asm_code_address);

                fprintf(asm_code_address, "\tmul %s, %s\n", REG_NAMES[reg_1], REG_NAMES[reg_2]);

                print_mul(byte_code_address, reg_1, reg_2);

                free_reg(reg_2);
                return reg_1;
            case DIV:
                reg_1 = make_node_code(node->left, back_end_base, byte_code_address, asm_code_address);
                reg_2 = make_node_code(node->right, back_end_base, byte_code_address, asm_code_address);

                fprintf(asm_code_address, "\tpush rax\n\tpush rdx\n"
                                          "\tmov rax, %s\n\txor rdx, rdx\n"
                                          "\tdiv %s\n"
                                          "\tmov %s, rax\n"
                                          "\tpop rdx\n\tpop rax\n",
                                          REG_NAMES[reg_1], REG_NAMES[reg_2], REG_NAMES[reg_1]);

                print_div(byte_code_address, reg_1, reg_2);

                return reg_1;
            case ASSIGNMENT:
                reg_1 = make_node_code(node->right, back_end_base, byte_code_address, asm_code_address);
                fprintf(asm_code_address, "\tmov [%s], %s\n",
                        back_end_base->tree->variable_list[node->left->value.variable_number].var_name,
                        REG_NAMES[reg_1]);
                free_reg(reg_1);
                fprintf(asm_code_address, "\n");
                return 0x0;
            case IF:
                back_end_base->label_number++;
                reg_1 = make_node_code(node->left, back_end_base, byte_code_address, asm_code_address);
                fprintf(asm_code_address, "\tcmp %s, 0\n"
                                          "\tje label_%u\n",
                                          REG_NAMES[reg_1], actual_label_number);
                reg_2 = make_node_code(node->right, back_end_base, byte_code_address, asm_code_address);
                fprintf(asm_code_address, "label_%u:\n", actual_label_number);
                fprintf(asm_code_address, "\n");

                free_reg(reg_1);//TODO можно ли освободить раньше?
                free_reg(reg_2);
                return 0x0;
            case WHILE:
                back_end_base->label_number++;
                reg_1 = make_node_code(node->left, back_end_base, byte_code_address, asm_code_address);
                free_reg(reg_1);
                fprintf(asm_code_address, "\tcmp %s, 0\n"
                                          "\tje label_%u\n",
                                          REG_NAMES[reg_1], actual_label_number);
                back_end_base->label_number++;
                fprintf(asm_code_address, "label_%u:\n", actual_label_number + 1);
                reg_2 = make_node_code(node->left, back_end_base, byte_code_address, asm_code_address);
                reg_1 = make_node_code(node->left, back_end_base, byte_code_address, asm_code_address);
                free_reg(reg_2);
                free_reg(reg_1);
                fprintf(asm_code_address, "\tcmp %s, 0\n"
                                          "\tjne label_%u\n"
                                          "label_%u:\n",
                                          REG_NAMES[reg_2], actual_label_number + 1,
                                          actual_label_number);
                return 0x0;
            case END_OF_PROGRAMM:
                fprintf(asm_code_address, "\tmov rax, 60\n"
                                          "\txor rdi, rdi\n"
                                          "\tsyscall\n");
                return 0x0;
            case OP_END:
                make_node_code(node->left, back_end_base, byte_code_address, asm_code_address);
                if (node->right != NULL) make_node_code(node->right, back_end_base, byte_code_address, asm_code_address);
                return 0x0;
            case PRINT:
                reg_1 = make_node_code(node->left, back_end_base, byte_code_address, asm_code_address);
                fprintf(asm_code_address,
                       "\tpush rax\n"
                       "\tmov rax, %s\n"
                       "\tcall print\n"
                       "\tpop rax\n", REG_NAMES[reg_1]);
                return 0x0;
            case INPUT:
                fprintf(asm_code_address,
                       "\tpush rax\n"
                       "\tcall input\n"
                       "\tmov [%s], rax\n"
                       "\tpop rax\n",
                       back_end_base->tree->variable_list[node->left->value.variable_number].var_name);
                return 0x0;
            case PAR_OPEN:
            case PAR_CLOSE:
            case COPMLEX_OPERATOR_OPEN:
            case COPMLEX_OPERATOR_CLOSE:
            case FUNC:
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

void print_const(FILE* byte_code_address, int value)
{
    assert(byte_code_address);

    for (int i = 0; i < 4; i++)
    {
        fputc((uint8_t) value, byte_code_address);
        value = value >> 8;
    }
}

void print_add(FILE* byte_code_address,
               uint8_t term_1, uint8_t term_2)
{
    assert(byte_code_address);

    fputc(0x01, byte_code_address);
    print_reg_to_reg(byte_code_address, term_1, term_2);//результат в term_1

    return;
}

void print_sub(FILE* byte_code_address,
               uint8_t minuend, uint8_t subtrahend)
{
    assert(byte_code_address);

    fputc(0x29, byte_code_address);
    print_reg_to_reg(byte_code_address, minuend, subtrahend);//результат в minued(уменьшаемое)
}

void print_mul(FILE* byte_code_address,
               uint8_t multiplier_1, uint8_t multiplier_2)
{
    assert(byte_code_address);

    fputc(0x0f, byte_code_address);
    fputc(0xaf, byte_code_address);
    print_reg_to_reg(byte_code_address, multiplier_1, multiplier_2);//результат в multiplier_1

}
void print_div(FILE* byte_code_address,
               uint8_t dividend, uint8_t divider)
{
    assert(byte_code_address);

    fputc(0x50, byte_code_address);//push rax
    fputc(0x52, byte_code_address);//push rdx

    print_mov_reg_to_reg(byte_code_address, dividend, 1);
    //делимое в eax

    fputc(0x31, byte_code_address);
    fputc(0xd2, byte_code_address);//xor edx, edx

    fputc(0xf7, byte_code_address);
    print_reg_to_reg(byte_code_address, divider, 6);//div divider

    print_mov_reg_to_reg(byte_code_address, 1, dividend);

    fputc(0x5a, byte_code_address);//pop rdx
    fputc(0x58, byte_code_address);//pop rax
}

void print_mov_reg_to_reg(FILE* byte_code_address,
               uint8_t transmitter_register, uint8_t receiver_register)
{
    assert(byte_code_address);

    fputc(0x89, byte_code_address);
    print_reg_to_reg(byte_code_address, receiver_register, transmitter_register);
}

void print_lib(FILE* asm_code_address, FILE* byte_code_address)
{
    assert(asm_code_address);
    assert(byte_code_address);

    fprintf(asm_code_address,
        "print:\n"
        "\tlea rdi, output_str\n"
        "\n"
        "\tcmp rax, 0\n"
        "\tjg pozitive\n"
        "\tnot rax\n"
        "\tinc rax\n"
        "\tmov byte [rdi], '-'\n"
        "\tinc rdi\n"
        "pozitive:\n"
        "\txor rcx, rcx                ;rcx = 0\n"
        "\tmov cl, 0                   ;counter\n"
        "\n"
        "dec_count_digits:\n"
        "\tinc cl                      ;cl++\n"
        "\txor rdx, rdx\n"
        "\tmov rbx, 10\n"
        "\tdiv rbx\n"
        "\tpush rdx\n"
        "\ttest rax, rax\n"
        "\tjnz dec_count_digits\n"
        "\n"
        "print_dec:\n"
        "\tpop rax\n"
        "\tadd al, '0'\n"
        "\tmov [rdi], al\n"
        "\tinc rdi                     ;кdi++\n"
        "\tloop print_dec\n"
        "\n"
        "\tmov byte [rdi], 10\n"
        "\n"
        "print_and_free_buffer:\n"
        "\tlea rsi, output_str      ; copy address\n"
        "\tlea rdi, output_str      ; copy begin(to cmp symbol abd '\\0')\n"
        "\txor rcx, rcx            ; len counter\n"
        "\n"
        "len_calculate:\n"
        "\tcmp byte [rdi], 0       ; if '\\0'\n"
        "\tje print_str\n"
        "\tinc rcx                 ; len counter++\n"
        "\tinc rdi                 ; next symabol\n"
        "\tjmp len_calculate\n"
        "\n"
        "print_str:\n"
        "\tmov rax, 1              ; syscall number\n"
        "\tmov rdi, 1              ; stdout\n"
        "\tmov rdx, rcx            ; str len\n"
        "\tsyscall                 ; print str\n"
        "\n"
        "\tmov rcx, 20\n"
        "\tlea rdi, output_str\n"
        "\n"
        "free_buffer:\n"
        "\n"
        "\tmov byte [rdi], 0\n"
        "\tinc rdi\n"
        "\tloop free_buffer\n"
        "\n"
        "\tret\n");
    return;
}
