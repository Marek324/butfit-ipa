.intel_syntax noprefix

.data
    brightness_val_mmx: .byte 50, 50, 50, 50, 50, 50, 50, 50
    max_pixel_val:      .byte 255, 255, 255, 255, 255, 255, 255, 255
    zero_pixel_val:     .byte 0, 0, 0, 0, 0, 0, 0, 0
    constants:          .byte 50,50,50,50,50,50,50,50

.text

#linux: parametry RDI, RSI, RDX, RCX

.global brightness_mmx
brightness_mmx:
    push rbp
    mov rbp, rsp # asm

    mov rbx, rcx
    mov rcx, rdx

    mov rax, 0x0101010101010101
    mul rbx
    movq mm0, rax
    mov rbx, rax

    mov rax, rsi
    mul rcx 
    mov rdx, 3
    mul rdx
    sar rax, 3
    mov rcx, rax

    loop_mmx:
        movq mm1,[rdi+rcx*8]
        paddusb mm1, mm0
        movq [rdi+rcx*8], mm1

        loop loop_mmx

    mov rsp, rbp
    pop rbp
    ret

.global brightness_asm
brightness_asm:
    push rbp
    mov rbp, rsp

    mov rbx, rcx

    mov r9, 255

    mov rax, rsi
    mul rdx 
    mov rdx, 3
    mul rdx
    mov rcx, rax

    loop_asm:
        mov al,[rdi+rcx]
        add al, bl
        cmovc rax, r9
        mov [rdi+rcx],al

        loop loop_asm

    mov rsp, rbp
    pop rbp
    ret