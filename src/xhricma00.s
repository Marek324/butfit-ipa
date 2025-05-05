.intel_syntax noprefix
.code64

.data
    LOCAL_VARS_SIZE = 40
    TWO_PI: .float 6.283185307
    ONE: .float 1.0
    HALF: .float 0.5
    
    ROUND_MODE_NEAREST = 0
    ROUND_MODE_FLOOR = 1

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

    # time
    vbroadcastss ymm0, xmm0 

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
    
    # originalX
    vmovups ymm8, [r11 + rbx]
    # originalZ
    vmovups ymm9, [r8 + rbx]

    mov rcx, [rbp + 24]
    
waves_loop:

    # 2 * pi
    vbroadcastss ymm10, [rip + TWO_PI]

    mov rax, [rbp + 24] # numWaves
    mov rdx, 4 # sizeof(float)
    mul rdx # wavelength offset
    add rax, [rbp + 16] # waves base
    # wavelength
    vbroadcastss ymm11, [rax + 4 * rcx - 4]

    # k
    vdivps ymm10, ymm10, ymm11

    mov rax, [rbp + 16] # waves base
    # amplitude - not offset
    vbroadcastss ymm11, [rax + 4 * rcx - 4]
    
    vbroadcastss ymm12, [rip + HALF]
    # amplitude * 0.5
    vmulps ymm11, ymm11, ymm12

    # k * time
    vmulps ymm12, ymm10, ymm0

    # range reduction: k_time - 2pi * floor(k_time/2pi)
    # 2pi
    vbroadcastss ymm13, [rip + TWO_PI]

    # k_time / 2pi
    vdivps ymm14, ymm12, ymm13

    # floor
    vroundps ymm14, ymm14, ROUND_MODE_FLOOR 

    # 2 * floor(..)
    vmulps ymm14, ymm13, ymm14
    
    # k_time - ..
    # k*time [0,2pi]
    vsubps ymm12, ymm12, ymm14

    # map to int

    # LUT size
    vbroadcastsd ymm14, [rbp + 48] 
    # int -> float
    vcvtdq2ps ymm14, ymm14
    vbroadcastss ymm15, [rip + ONE]
    # N - 1
    vsubps ymm14, ymm14, ymm15
    # k * time * (N-1)
    vmulps ymm12, ymm12, ymm14
    # / 2pi
    vdivps ymm12, ymm12, ymm13
    # round
    vroundps ymm12, ymm12, ROUND_MODE_NEAREST

    # convert to int 
    vcvtps2dq ymm12, ymm12
    
    # ymm10 - k
    # ymm11 - amp * 0.5
    # ymm12 - k*time LUT index

    # * 4 - sizeof float 
    # mov eax, 4
    # vmovd xmm13, eax
    # vpbroadcastd ymm13, xmm13
    # vmulps ymm12, ymm12, ymm13
    
    # offset + base
    # mov eax, [rbp + 32]
    # vmovd xmm13, eax
    # vpbroadcastd ymm13, xmm13
    # vaddps ymm12, ymm12, ymm13

    # mask
    mov eax, -1
    vmovd xmm13, eax
    vpbroadcastd ymm13, xmm13

    mov rax, [rbp + 32]
    vgatherdps ymm14, [rax + ymm12*4], ymm13

    dec rcx
    jz waves_end
    jmp waves_loop 
waves_end:

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
