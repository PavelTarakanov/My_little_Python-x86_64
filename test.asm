section .text
    global _start
_start:
    mov eax, eax

    mov eax, 1
    xor ebx, ebx
    int 0x80

section .data
    a dq 8
