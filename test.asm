section .text
    extern printf
    global _start
_start:

    mov eax, eax
    mov eax, 1
    mov ebx, 1
    mov ecx, 1
    mov edx, 1
    mov edi, 1
    mov esi, 1
    mov ebp, 1
    mov esp, 1

    push rax
    push rdx

    pop rdx
    pop rax

    push rax
    push rdx
    mov eax, eax
    xor edx, edx
    div ecx
    mov eax, eax
    pop rdx
    pop rax

    xor edx, edx

    add eax, eax
    mov eax, 1
    xor ebx, ebx
    int 0x80

section .data
    a dq 8
    b dq 12
