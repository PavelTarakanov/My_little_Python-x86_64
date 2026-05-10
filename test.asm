section .text
    extern printf
    global _start
_start:
    mov rax, 1
    mov rbx, 1
    mov rcx, 1
    mov rdx, 1
    mov rdi, 1
    mov rsi, 1
    mov rbp, 1
    mov rsp, 1

    mov rax, rax
    mov rax, rbx
    mov rax, rcx
    mov rax, rdx
    mov rax, rdi
    mov rax, rsi
    mov rax, rbp
    mov rax, rsp


    mov rbx, rax
    mov rbx, rbx
    mov rbx, rcx
    mov rbx, rdx
    mov rbx, rdi

    add eax, eax
    mov eax, 1
    xor ebx, ebx
    int 0x80

section .data
    a dq 8
    b dq 12
