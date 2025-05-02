.intel_syntax noprefix
.code64

.data
    LOCAL_VARS_SIZE = 40
    TWO_PI: .float 6.283185307
    ONE: .float 1.0
    HALF: .float 0.5
    

.text

.global updateVertices_simd
updateVertices_simd:
    push rbp
    mov rbp, rsp 
    sub rsp, LOCAL_VARS_SIZE
    
    # rdi       - vertices_y    qw - float*
    # rsi       - normals       qw - float*
    # rdx       - numVertices   qw - ulong
    # rcx       - origWorldX    qw - float*
    # r8        - origWorldZ    qw - float*
    # r9        - gridSize      dw - int
    # RBP + 16  - waves         qw - float*
    # RBP + 24  - numWaves      qw - ulong
    # RBP + 32  - sinLUT        qw - float*
    # RBP + 40  - cosLUT        qw - float*
    # RBP + 48  - lutSize       qw - ulong
    # xmm0      - time      

    mov r10, rdx 
    mov r11, rcx
    
    # numWaves
    mov r12, [rbp + 24]
    
    # waves AoS offsets
    # amplitude     - 0
    # wavelength    - size * 4
    # speed         - size * 8
    # direction_x   - size * 12
    # direction_z   - size * 16
    # phase         - size * 20

    # loop setup

    # x_loop counter
    mov r15, 0 
    # y_loop counter
    mov r14, 0 

    vbroadcastss ymm0, xmm0 # time, const for one function iterations

    # originalXZ offset
    mov rax, r9 # size
    mul r15 # size * x
    add rax, r14 # + z
    mov rbx, rax # offset

x_loop:
    cmp r15, r9
    je x_end

z_loop:
    cmp r14, r9
    jae z_end

    # total_height = 0
    vxorps ymm1, ymm1, ymm1
    
    # tangentX.x = 1
    vbroadcastss ymm2, [rip + ONE]
    # tangentX.y = 0
    vxorps ymm3, ymm3, ymm3
    # tangentX.z = 0
    vxorps ymm4, ymm4, ymm4
    # tangentZ.x = 0
    vxorps ymm5, ymm5, ymm5
    # tangentZ.y = 0
    vxorps ymm6, ymm6, ymm6
    # tangentZ.z = 1
    vbroadcastss ymm7, [rip + ONE]
    
    vmovups ymm8, [r11 + rbx]
    vmovups ymm9, [r8 + rbx]

    mov rcx, [rbp + 24]
    
waves_loop:

    vbroadcastss ymm10, [rip + TWO_PI]

    mov rax, [rbp + 24] # numWaves
    mov rdx, 4 # sizeof(float)
    mul rdx # wavelength offset
    add rax, [rbp + 16] # waves base
    vbroadcastss ymm11, [rax + 4 * rcx - 4]

    loop waves_loop 

    add r14, 8
    # inc offset by 8 floats, no need to mul again
    add rbx, 32

    jmp z_loop
z_end:

    inc r15

    jmp x_loop

x_end:
    add rsp, LOCAL_VARS_SIZE
    mov rsp, rbp
    pop rbp
    ret
