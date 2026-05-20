#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <sys/stat.h>
#include <stdint.h>
#include "file_using.h"
#include "commands.h"
#include "language.h"

#define free_reg(reg_number) back_end_base->regs[reg_number].free_flag = true

static const char* REG_NAMES[8] = {"rax", "rcx", "rdx", "rbx", "rsp", "rbp", "rsi", "rdi"};

static uint8_t make_node_code(node_t* node, back_end_base_t* back_end_base,
                           FILE* bite_code_address, FILE* asm_code_address);
static uint8_t find_free_reg(back_end_base_t* back_end_base);
static tree_errors print_header(back_end_base_t* back_end_base);
static void print_data(back_end_base_t* back_end_base);
static void print_const_to_reg(back_end_base_t* back_end_base, uint8_t reg, int value);
static void print_reg_to_reg(back_end_base_t* back_end_base, uint8_t reg_1, uint8_t reg_2);
static void print_mem_to_reg(back_end_base_t* back_end_base,
                      uint8_t reg, unsigned int variable_number);
static void print_reg_to_mem(back_end_base_t* back_end_base,
                      uint8_t reg, unsigned int variable_number);
static void print_const(back_end_base_t* back_end_base, int value);
static void print_const_4_byte(back_end_base_t* back_end_base, int value);
static void print_add(back_end_base_t* back_end_base,
               uint8_t term_1, uint8_t term_2);
static void print_sub(back_end_base_t* back_end_base,
               uint8_t minuend, uint8_t subtrahend);
static void print_mul(back_end_base_t* back_end_base,
               uint8_t multiplier_1, uint8_t multiplier_2);
static void print_div(back_end_base_t* back_end_base,
               uint8_t transmitter_register, uint8_t receiver_register);
static void print_mov_reg_to_reg(back_end_base_t* back_end_base,
               uint8_t transmitter_register, uint8_t receiver_register);
static void print_end_of_programm(back_end_base_t* back_end_base);
static void print_asm_lib(FILE* asm_code_address, FILE* byte_code_address);

tree_errors make_asm_code(back_end_base_t* back_end_base, char** argv)
{
    assert(back_end_base);

    FILE* asm_code_address = NULL;
    FILE* byte_code_address = NULL;

    if (check_file_opening(argv[2], &asm_code_address, "w+"))
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

    fprintf(asm_code_address, "section .data\n");

    for(int i = 0; i < back_end_base->tree->number_of_variables; i++)
    {
        fprintf(asm_code_address, "\t%s dq 0\n", back_end_base->tree->variable_list[i].var_name);
        back_end_base->data_section_len += 8;
    }

    fprintf(asm_code_address, "\tinput_str times 20 db 0\n");
    fprintf(asm_code_address, "\toutput_str times 20 db 0\n");
    back_end_base->data_section_len += 40;

    fprintf(asm_code_address, "section .text\n\tglobal _start\n_start:\n");

    print_header(back_end_base);

    if (make_node_code(back_end_base->tree->root, back_end_base,
                       byte_code_address, asm_code_address))
        return ASM_MAKING_ERROR;

    print_data(back_end_base);

    for (unsigned int i = 0; i < back_end_base->instruction_pointer; i++)
        fputc(back_end_base->byte_code[i], byte_code_address);

    print_asm_lib(asm_code_address, byte_code_address);

    if (check_file_closing(asm_code_address))
        printf("Error while closing file!");

    if (check_file_closing(byte_code_address))
        printf("Error while closing file!");

    if (system("nasm -f elf64 -l asm.lst -o asm.o asm.txt"))
        return NASM_ERROR;

    if (system("ld -o asm asm.o"))
        return LD_ERROR;

    printf("Making asm code is finished\n");

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

            print_const_to_reg(back_end_base, reg_1, node->value.number_value);
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
        else
        {
            fprintf(asm_code_address, "\tmov %s, [%s]\n", REG_NAMES[reg_1],
                    back_end_base->tree->variable_list[node->value.variable_number].var_name);
            print_mem_to_reg(back_end_base, reg_1, node->value.variable_number);
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

                print_add(back_end_base, reg_1, reg_2);

                free_reg(reg_2);
                return reg_1;
            case SUB:
                reg_1 = make_node_code(node->left, back_end_base, byte_code_address, asm_code_address);
                reg_2 = make_node_code(node->right, back_end_base, byte_code_address, asm_code_address);

                fprintf(asm_code_address, "\tsub %s, %s\n", REG_NAMES[reg_1], REG_NAMES[reg_2]);

                print_sub(back_end_base, reg_1, reg_2);

                free_reg(reg_2);
                return reg_1;
            case MUL:
                reg_1 = make_node_code(node->left, back_end_base, byte_code_address, asm_code_address);
                reg_2 = make_node_code(node->right, back_end_base, byte_code_address, asm_code_address);

                fprintf(asm_code_address, "\timul %s, %s\n", REG_NAMES[reg_1], REG_NAMES[reg_2]);

                print_mul(back_end_base, reg_1, reg_2);

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

                print_div(back_end_base, reg_1, reg_2);

                return reg_1;
            case ASSIGNMENT:
                reg_1 = make_node_code(node->right, back_end_base, byte_code_address, asm_code_address);
                fprintf(asm_code_address, "\tmov [%s], %s\n",
                        back_end_base->tree->variable_list[node->left->value.variable_number].var_name,
                        REG_NAMES[reg_1]);

                print_reg_to_mem(back_end_base, reg_1, node->left->value.variable_number);

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
                reg_2 = make_node_code(node->right, back_end_base, byte_code_address, asm_code_address);
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

                print_end_of_programm(back_end_base);
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

tree_errors print_header(back_end_base_t* back_end_base)
{
    assert(back_end_base);

    for (; back_end_base->instruction_pointer < 0x40; back_end_base->instruction_pointer++)
        back_end_base->byte_code[back_end_base->instruction_pointer] = ELF_HEADER[back_end_base->instruction_pointer];

    for (; back_end_base->instruction_pointer < 0x78; back_end_base->instruction_pointer++)
        back_end_base->byte_code[back_end_base->instruction_pointer] = PROGRAMM_HEADER[back_end_base->instruction_pointer - 0x40];

    for (; back_end_base->instruction_pointer < 0xb0; back_end_base->instruction_pointer++)
        back_end_base->byte_code[back_end_base->instruction_pointer] = TEXT_HEADER[back_end_base->instruction_pointer - 0x78];

    for (; back_end_base->instruction_pointer < 0xe8; back_end_base->instruction_pointer++)
        back_end_base->byte_code[back_end_base->instruction_pointer] = DATA_HEADER[back_end_base->instruction_pointer - 0xb0];

    for (; back_end_base->instruction_pointer < 0x1000; back_end_base->instruction_pointer++)
        back_end_base->byte_code[back_end_base->instruction_pointer] = 0x00;

    return NO_ERROR;
}

void print_const_to_reg(back_end_base_t* back_end_base, uint8_t reg, int value)
{
    assert(back_end_base);

    back_end_base->byte_code[back_end_base->instruction_pointer] = 0x48;
    back_end_base->byte_code[back_end_base->instruction_pointer + 1] = 0xb8 + reg;
    back_end_base->instruction_pointer += 2;

    print_const(back_end_base, value);

    return;
}
void print_reg_to_reg(back_end_base_t* back_end_base, uint8_t reg_1, uint8_t reg_2)
{
    assert(back_end_base);//reg_1 приёмник, reg_2 источник

    back_end_base->byte_code[back_end_base->instruction_pointer] = (uint8_t) (0xc0 + (reg_2 << 3) + reg_1);
    back_end_base->instruction_pointer++;

    return;
}

void print_mem_to_reg(back_end_base_t* back_end_base,
                      uint8_t reg, unsigned int variable_number)
{
    assert(back_end_base);

    back_end_base->byte_code[back_end_base->instruction_pointer] = 0x48;//для 64-битных
    back_end_base->byte_code[back_end_base->instruction_pointer + 1] = 0x8b;//непосредственно код mov
    back_end_base->byte_code[back_end_base->instruction_pointer + 2] = (uint8_t) ((reg << 3) + 0b100);//в какой регистр
    back_end_base->byte_code[back_end_base->instruction_pointer + 3] = 0x25;//SIB-байт

    back_end_base->instruction_pointer += 4;
    print_const_4_byte(back_end_base,
                0x402000 + (variable_number * 8));

    return;
}

void print_reg_to_mem(back_end_base_t* back_end_base,
                      uint8_t reg, unsigned int variable_number)
{
    assert(back_end_base);

    back_end_base->byte_code[back_end_base->instruction_pointer] = 0x48;//для 64-битных
    back_end_base->byte_code[back_end_base->instruction_pointer + 1] = 0x89;//непосредственно код mov
    back_end_base->byte_code[back_end_base->instruction_pointer + 2] = (uint8_t) ((reg << 3) + 0b100);//из какого регистра
    back_end_base->byte_code[back_end_base->instruction_pointer + 3] = 0x25;//SIB-байт

    back_end_base->instruction_pointer += 4;
    print_const_4_byte(back_end_base,
                0x402000 + (variable_number * 8));
    return;
}

void print_const(back_end_base_t* back_end_base, int value)
{
    assert(back_end_base);

    for (int i = 0; i < 8; i++)
    {
        back_end_base->byte_code[back_end_base->instruction_pointer] = (uint8_t) value;
        back_end_base->instruction_pointer++;
        value = value >> 8;
    }
}

void print_const_4_byte(back_end_base_t* back_end_base, int value)
{
    assert(back_end_base);

    for (int i = 0; i < 4; i++)
    {
        back_end_base->byte_code[back_end_base->instruction_pointer] = (uint8_t) value;
        back_end_base->instruction_pointer++;
        value = value >> 8;
    }
}

void print_add(back_end_base_t* back_end_base,
               uint8_t term_1, uint8_t term_2)
{
    assert(back_end_base);

    back_end_base->byte_code[back_end_base->instruction_pointer] = 0x48;//для 64-битных
    back_end_base->byte_code[back_end_base->instruction_pointer + 1] = 0x01;//непосредственно код add

    back_end_base->instruction_pointer += 2;

    print_reg_to_reg(back_end_base, term_1, term_2);//результат в term_1

    return;
}

void print_sub(back_end_base_t* back_end_base,
               uint8_t minuend, uint8_t subtrahend)
{
    assert(back_end_base);

    back_end_base->byte_code[back_end_base->instruction_pointer] = 0x48;//для 64-битных
    back_end_base->byte_code[back_end_base->instruction_pointer + 1] = 0x29;//непосредственно sub

    back_end_base->instruction_pointer += 2;

    print_reg_to_reg(back_end_base, minuend, subtrahend);//результат в minued(уменьшаемое)

    return;
}

void print_mul(back_end_base_t* back_end_base,
               uint8_t multiplier_1, uint8_t multiplier_2)
{
    assert(back_end_base);

    back_end_base->byte_code[back_end_base->instruction_pointer] = 0x48;//для 64-битных
    back_end_base->byte_code[back_end_base->instruction_pointer + 1] = 0x0f;
    back_end_base->byte_code[back_end_base->instruction_pointer + 2] = 0xaf;//непосредственно imul

    back_end_base->instruction_pointer += 3;

    print_reg_to_reg(back_end_base, multiplier_2, multiplier_1);//результат в multiplier_1

}

void print_div(back_end_base_t* back_end_base,
               uint8_t dividend, uint8_t divider)
{
    assert(back_end_base);

    back_end_base->byte_code[back_end_base->instruction_pointer] = 0x50;//push rax
    back_end_base->byte_code[back_end_base->instruction_pointer + 1] = 0x52;//push rdx

    back_end_base->instruction_pointer += 2;

    print_mov_reg_to_reg(back_end_base, dividend, 1);
    //делимое в eax

    back_end_base->byte_code[back_end_base->instruction_pointer] = 0x48;//для 64-битных
    back_end_base->byte_code[back_end_base->instruction_pointer + 1] = 0x31;//непосредственно xor
    back_end_base->byte_code[back_end_base->instruction_pointer + 2] = 0xd2;//edx, edx

    back_end_base->byte_code[back_end_base->instruction_pointer + 3] = 0x48;//для 64-битных
    back_end_base->byte_code[back_end_base->instruction_pointer + 4] = 0xf7;//непосредственно div

    back_end_base->instruction_pointer += 5;

    print_reg_to_reg(back_end_base, divider, 6);//div делитель

    print_mov_reg_to_reg(back_end_base, 1, dividend);

    back_end_base->byte_code[back_end_base->instruction_pointer] = 0x5a;//pop rdx
    back_end_base->byte_code[back_end_base->instruction_pointer + 1] = 0x58;//pop rax

    back_end_base->instruction_pointer += 2;

    return;
}

void print_mov_reg_to_reg(back_end_base_t* back_end_base,
               uint8_t transmitter_register, uint8_t receiver_register)
{
    assert(back_end_base);

    back_end_base->byte_code[back_end_base->instruction_pointer] = 0x48;//для 64-битных
    back_end_base->byte_code[back_end_base->instruction_pointer + 1] = 0x89;//непосредственно mov

    print_reg_to_reg(back_end_base, receiver_register, transmitter_register);

    return;
}

void print_data(back_end_base_t* back_end_base)
{
    assert(back_end_base);

    for(;back_end_base->instruction_pointer < 0x2000; back_end_base->instruction_pointer++)
        back_end_base->byte_code[back_end_base->instruction_pointer] = 0x00;

    for (int i = 0; i < back_end_base->tree->number_of_variables; i++)
        print_const(back_end_base, back_end_base->tree->variable_list[i].var_value);

    return;
}

void print_end_of_programm(back_end_base_t* back_end_base)
{
    assert(back_end_base);

    back_end_base->byte_code[back_end_base->instruction_pointer] = 0xb8;
    back_end_base->byte_code[back_end_base->instruction_pointer + 1] = 0x3c;
    back_end_base->byte_code[back_end_base->instruction_pointer + 2] = 0x00;
    back_end_base->byte_code[back_end_base->instruction_pointer + 3] = 0x00;
    back_end_base->byte_code[back_end_base->instruction_pointer + 4] = 0x00;
    back_end_base->byte_code[back_end_base->instruction_pointer + 5] = 0x48;
    back_end_base->byte_code[back_end_base->instruction_pointer + 6] = 0x31;
    back_end_base->byte_code[back_end_base->instruction_pointer + 7] = 0xff;
    back_end_base->byte_code[back_end_base->instruction_pointer + 8] = 0x0f;
    back_end_base->byte_code[back_end_base->instruction_pointer + 9] = 0x05;

    back_end_base->instruction_pointer += 10;

    return;
}

void print_asm_lib(FILE* asm_code_address, FILE* byte_code_address)
{
    assert(asm_code_address);
    assert(byte_code_address);

    fprintf(asm_code_address,
        ";------------------------------------------------------------------\n"
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

    fprintf(asm_code_address,
        ";------------------------------------------------------------\n"
        "input:\n"
        "\tpush rbx\n"
        "\tpush rcx\n"
        "\tpush rdx\n"
        "\tpush rdi\n"
        "\tpush rsi\n"
        "\tmov rax, 0\n"
        "\tmov rdi, 0\n"
        "\tlea rsi, input_str\n"
        "\tmov rdx, 20\n"
        "\tsyscall\n"
        "\n"
        "\tlea rdi, input_str\n"
        "\txor rax, rax\n"
        "\tmov rsi, 1                   ;флаг знака\n"
        "\n"
        "\txor rcx, rcx\n"
        "\tskip_spaces:\n"
        "\tmov cl, [rdi]\n"
        "\tcmp cl, ' '\n"
        "\tjne check_sign\n"
        "\tinc rdi\n"
        "\tjmp skip_spaces\n"
        "check_sign:\n"
        "\tmov cl, [rdi]\n"
        "\tcmp cl, '-'\n"
        "\tje set_negative\n"
        "\tjmp make_number\n"
        "set_negative:\n"
        "\tmov rsi, -1\n"
        "\tinc rdi\n"
        "\n"
        "make_number:\n"
        "\tmov cl, [rdi]\n"
        "\tcmp cl, '0'\n"
        "\tjl done\n"
        "\tcmp cl, '9'\n"
        "\tjg done\n"
        "\n"
        "\tsub cl, '0'\n"
        "\timul rax, 10\n"
        "\tadd rax, rcx\n"
        "\n"
        "\tinc rdi\n"
        "\tjmp make_number\n"
        "done:\n"
        "\timul rax, rsi\n"
        "\tmov rcx, 20\n"
        "\tlea rdi, input_str\n"
        "free_input_buffer:\n"
        "\tmov byte [rdi], 0\n"
        "\tinc rdi\n"
        "\tloop free_input_buffer\n"
        "\n"
        "\tpop rsi\n"
        "\tpop rdi\n"
        "\tpop rdx\n"
        "\tpop rcx\n"
        "\tpop rbx\n"
        "\n"
        "\tret\n");
    return;
}
