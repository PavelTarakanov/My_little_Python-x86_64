section .data
    n dd 0
    output_str db '1' times 20 db 0
section .text
    global _start
_start:
    mov rax, 4
    mov [n], rax

    mov rax, [n]
    mov rdx, rax        ; параметр прямо в rdx
    call print

    mov rax, 60         ; sys_exit для 64-bit
    xor rdi, rdi
    syscall

print:
    push rdx
    push rdi
    push rcx

    lea rdi, output_str

    mov rax, rdx
    cmp rax, 0
    jg pozitive
    not rax
    inc rax
    mov byte [rdi], '-'
    inc rdi
pozitive:
    xor rcx, rcx        ; обнулить ВЕСЬ rcx, а не только cl

dec_count_digits:
    inc rcx
    xor rdx, rdx
    mov rbx, 10
    div rbx
    push rdx
    test rax, rax
    jnz dec_count_digits

print_dec:
    pop rax
    add al, '0'
    mov [rdi], al
    inc rdi
    loop print_dec

    ; НЕ удалять pop rdi и pop rdx здесь!
    ; Они будут в конце

print_and_free_buffer:
    push rsi
    lea rsi, output_str
    lea rdi, output_str
    xor rcx, rcx

len_calculate:
    cmp byte [rdi], 0
    je print_str
    inc rcx
    inc rdi
    jmp len_calculate

print_str:
    mov rax, 1
    mov rdi, 1
    mov rdx, rcx + 1
    syscall

    mov rcx, 20
    lea rdi, output_str

free_buffer:
    mov byte [rdi], 0
    inc rdi
    loop free_buffer

    pop rsi
    pop rcx
    pop rdi
    pop rdx
    ret
