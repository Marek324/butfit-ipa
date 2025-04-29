.intel_syntax noprefix
.code64


.data
    # pi dd 3.1415926535

.text


.global updateVertices_simd
updateVertices_simd:
    push rbp
    mov rbp, rsp 
    sub rsp, 64



    add rsp, 64
    mov rsp, rbp
    pop rbp
    ret
