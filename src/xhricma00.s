.intel_syntax noprefix
.code64

# could be generalized to 6 registers, not needed
.macro lut_idx reg
    # reduce float to [0, 2pi]
    # float - 2pi * floor(float/2pi)

    # float/2pi
    vdivps ymm15, \reg, [rip + TWO_PI] 

    # floor(float/2pi)
    vroundps ymm15, ymm15, ROUND_MODE_FLOOR 

    # 2pi * floor(float/2pi)
    vmulps ymm15, ymm15, [rip + TWO_PI]  
    
    # float - 2pi * floor(float/2pi)
    vsubps \reg, \reg, ymm15

    # map to int
    #round((float * (N-1)) / 2pi)

    # LUT size, N
    mov eax, [rbp + 48]
    vmovd xmm15, eax
    vpbroadcastd ymm15, xmm15 

    # int -> float
    vcvtdq2ps ymm15, ymm15

    # N - 1
    vsubps ymm15, ymm15, [rip + ONE]
    # float * (N-1)
    vmulps \reg, \reg, ymm15 
    # (float * (N-1)) / 2pi
    vdivps \reg, \reg, [rip + TWO_PI] 
    # round
    vroundps \reg, \reg, ROUND_MODE_NEAREST

    # float -> int 
    vcvtps2dq \reg, \reg 
.endm

.macro sin dst src
    lut_idx \src

    # mask
    mov eax, -1
    vmovd xmm15, eax
    vpbroadcastd ymm15, xmm15

    mov rax, [rbp + 32]
    vgatherdps \dst, [rax + \src*4], ymm15
.endm

.macro cos dst src
    lut_idx \src

    # mask
    mov eax, -1
    vmovd xmm15, eax
    vpbroadcastd ymm15, xmm15

    mov rax, [rbp + 40]
    vgatherdps \dst, [rax + \src*4], ymm15
.endm

.macro waves_arr n
    # offset (numWaves * n)
    mov rax, r12 # numWaves
    mov rdx, \n
    mul rdx
    add rax, [rbp + 16] # waves base
.endm

.macro waveamp dst
    mov rax, [rbp + 16] # waves base
    vbroadcastss \dst, [rax + 4 * rcx]
.endm

.macro wavelen dst
    waves_arr 4
    vbroadcastss \dst, [rax + 4 * rcx]
.endm

.macro wavespd dst
    waves_arr 8
    vbroadcastss \dst, [rax + 4 * rcx]
.endm

.macro wavedirx dst
    waves_arr 12
    vbroadcastss \dst, [rax + 4 * rcx]
.endm

.macro wavediry dst
    waves_arr 16
    vbroadcastss \dst, [rax + 4 * rcx]
.endm

.macro wavephs dst
    waves_arr 20
    vbroadcastss \dst, [rax + 4 * rcx]
.endm

.data
    .align 32
    TWO_PI: .float 6.283185307, 6.283185307, 6.283185307, 6.283185307, 6.283185307, 6.283185307, 6.283185307, 6.283185307
    ONE: .float 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0
    HALF: .float 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5
    M_ONE: .float -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0, -1.0

    LOCAL_VARS_SIZE = 104
    ROUND_MODE_NEAREST = 0
    ROUND_MODE_FLOOR = 1

.text

.global updateVertices_simd
updateVertices_simd:
    push rbp
    mov rbp, rsp 
    sub rsp, LOCAL_VARS_SIZE
    
    # Args:
    # rdi       - vertices_y    qw - float*
    # rsi       - normals       qw - float*
    # rdx       - numVertices   qw - ulong
    # rcx       - origWorldX    qw - float*
    # r8        - origWorldZ    qw - float*
    # r9        - gridSize      dw - ulong
    # RBP + 16  - waves         qw - float*
    # RBP + 24  - numWaves      qw - ulong
    # RBP + 32  - sinLUT        qw - float*
    # RBP + 40  - cosLUT        qw - float*
    # RBP + 48  - lutSize       qw - ulong
    # xmm0      - time      

    # numVertices ^2 = gridSize
    # no need for gridSize so overwritten, could maybe check or remove from function call

    mov r15, r9 
    mov r9, r8
    mov r8, rcx
    mov r12, [rbp + 24]
    mov r10, rdi
    mov r11, rsi

    # Register usage:
    # rax       - 
    # rbx       - vector indexing
    # rcx       - wave index 
    # rdx       -
    # r8,9      - originalX,Z base
    # r10       - vertices base
    # r11       - normals base  
    # r12       - wave count 
    # r13       - z index
    # r14       - x index
    # r15       - grid size
    # ymm0      - time
    # ymm1      - total vertex height
    # ymm2-4    - tanX
    # ymm5-7    - tanZ
    # ymm8,9    - originalX,Z values
    
    # time
    vbroadcastss ymm0, xmm0 

    # x_loop index
    mov r14, 0 

x_loop:
    cmp r14, r15
    je x_end
    
    # z_loop index
    mov r13, 0

z_loop:
    cmp r13, r15
    je z_end

    # waves_loop index 
    mov rcx , 0

    # total_height = 0
    vxorps ymm1, ymm1, ymm1
    
    # tangentX.x = 1
    vmovaps ymm2, [rip + ONE]
    # tangentX.y = 0
    vxorps ymm3, ymm3, ymm3
    # tangentX.z = 0
    vxorps ymm4, ymm4, ymm4
    # tangentZ.x = 0
    vxorps ymm5, ymm5, ymm5
    # tangentZ.y = 0
    vxorps ymm6, ymm6, ymm6
    # tangentZ.z = 1
    vmovaps ymm7, [rip + ONE]
 
    mov rax, r15 # gridSize
    mul r14 # size * x
    add rax, r13 # + z
    mov rbx, rax # offset
    
    # originalX
    vmovups ymm8, [r8 + 4*rbx]
    # originalZ
    vmovups ymm9, [r9 + 4*rbx]
   
waves_loop:
    cmp rcx , r12
    je waves_end

    # 2 * pi
    vmovaps ymm10, [rip + TWO_PI]

    # wave.wavelength
    wavelen ymm11

    # k
    vdivps ymm10, ymm10, ymm11

    # amplitude
    waveamp ymm11 

    # amplitude * 0.5
    vmulps ymm11, ymm11, [rip + HALF]

    # k * time
    vmulps ymm12, ymm10, ymm0

    # sin(k*time)
    sin ymm14 ymm12

    # 1 + sin(k*time)
    vaddps ymm12, ymm14, [rip + ONE]

    # periodicAmplitude
    vmulps ymm11, ymm11, ymm12

    # amp_k
    vmulps ymm12, ymm11, ymm10

    # wave.direction.x
    wavedirx ymm13

    # wave.direction.y
    wavediry ymm14

    # dir.x * origX
    vmulps ymm13, ymm13, ymm8
    # dir.z * origZ
    vmulps ymm14, ymm14, ymm9
    # dotProduct
    vaddps ymm13, ymm13, ymm14

    # wave.speed
    wavespd ymm14

    # speed * time
    vmulps ymm14, ymm14, ymm0

    # dotProduct - speed * time
    vsubps ymm13, ymm13, ymm14

    # wave.phase
    wavephs ymm14

    # dotProduct - speed * time + wave.phase
    vaddps ymm13, ymm13, ymm14

    # phase
    vmulps ymm10, ymm13, ymm10

    # sinTerm
    vmovups [rsp - 8], ymm10
    sin ymm13 ymm10 
    vmovups ymm10, [rsp - 8]
    
    # waveHeightValue
    vmulps ymm11, ymm11, ymm13

    # totalHeight += waveHeightValue
    vaddps ymm1, ymm1, ymm11 

    # cosTerm
    cos ymm11 ymm10

    # ymm10 - phase - x
    # ymm11 - cosTerm
    # ymm12 - amp_k 
    # ymm13 - sinTerm

    # wave.direction.x
    wavedirx ymm15
    
    # amp_dir_x
    vmulps ymm14, ymm12, ymm15

    # wave.direcion.y
    wavediry ymm15

    # amp_dir_y
    vmulps ymm15, ymm12, ymm15

    # ymm10 - phase    -x 
    # ymm11 - cosTerm
    # ymm12 - amp_k    -x 
    # ymm13 - sinTerm
    # ymm14 - amp_dir_x
    # ymm15 - amp_dir_y

    # amp_dir_x * sinTerm
    vmulps ymm10, ymm14, ymm13
    # amp_dir_y * sinTerm
    vmulps ymm12, ymm15, ymm13

    # amp_x_sin
    vmulps ymm10, ymm10, [rip + M_ONE]
    # amp_y_sin
    vmulps ymm12, ymm12, [rip + M_ONE]

    # amp_dir_x *= cosTerm
    vmulps ymm14, ymm14, ymm11
    # amp_dir_y *= cosTerm
    vmulps ymm15, ymm15, ymm11

    # tangentX.y += amp_dir_x
    vaddps ymm3, ymm3, ymm14
    # tangentZ.y += amp_dir_y
    vaddps ymm6, ymm6, ymm15

    wavedirx ymm14
    wavediry ymm15

    # tangentX.x = amp_x_sin * wave.direction.x
    vmulps ymm11, ymm10, ymm14
    vaddps ymm2, ymm2, ymm11

    # tangentX.z = amp_x_sin * wave.direction.y
    vmulps ymm11, ymm10, ymm15
    vaddps ymm4, ymm4, ymm11

    # tangentZ.x = amp_y_sin * wave.direction.x
    vmulps ymm11, ymm12, ymm14
    vaddps ymm5, ymm5, ymm11

    # tangentX.z = amp_y_sin * wave.direction.y
    vmulps ymm11, ymm12, ymm15
    vaddps ymm7, ymm7, ymm11

    inc rcx
    jmp waves_loop

waves_end:

    # vertex.y = total_height
    vmovups [r10 + 4*rbx], ymm1

    # tanZ.y * tanX.z
    vmulps ymm10, ymm6, ymm4
    # tanZ.z * tanX.y
    vmulps ymm11, ymm7, ymm3
    # cross_x
    vsubps ymm10, ymm10, ymm11

    # tanZ.z * tanX.x
    vmulps ymm11, ymm7, ymm2
    # tanZ.x * tanX.z
    vmulps ymm12, ymm5, ymm4
    # cross_y
    vsubps ymm11, ymm11, ymm12

    # tanZ.x * tanX.y
    vmulps ymm12, ymm5, ymm3
    # tanZ.y * tanX.x
    vmulps ymm13, ymm6, ymm2
    # cross_z
    vsubps ymm12, ymm12, ymm13

    # cross_x^2
    vmulps ymm13, ymm10, ymm10
    # cross_y^2
    vmulps ymm14, ymm11, ymm11
    # cross_x^2 + cross_y^2
    vaddps ymm13, ymm13, ymm14
    # cross_z^2
    vmulps ymm15, ymm12, ymm12
    # cross_x^2 + cross_y^2 + cross_z^2
    vaddps ymm13, ymm13, ymm15

    # len
    vsqrtps ymm14, ymm13

    # cross_x /= len
    vdivps ymm10, ymm10, ymm14
    # cross_y /= len
    vdivps ymm11, ymm11, ymm14
    # cross_z /= len
    vdivps ymm12, ymm12, ymm14

    # store norms in memory
    # this all could be achieved with scatter if AVX512 was available

    lea rdi, [r11 + 4*rbx]

    vextractf128 xmm8, ymm10, 0

    pextrd [rdi], xmm8, 0
    pextrd [rdi + 12], xmm8, 1
    pextrd [rdi + 24], xmm8, 2
    pextrd [rdi + 36], xmm8, 3

    vextractf128 xmm8, ymm10, 1

    pextrd [rdi + 48], xmm8, 0
    pextrd [rdi + 60], xmm8, 1
    pextrd [rdi + 72], xmm8, 2
    pextrd [rdi + 84], xmm8, 3

    vextractf128 xmm8, ymm11, 0

    pextrd [rdi + 4], xmm8, 0
    pextrd [rdi + 16], xmm8, 1
    pextrd [rdi + 28], xmm8, 2
    pextrd [rdi + 40], xmm8, 3

    vextractf128 xmm8, ymm11, 1

    pextrd [rdi + 52], xmm8, 0
    pextrd [rdi + 64], xmm8, 1
    pextrd [rdi + 76], xmm8, 2
    pextrd [rdi + 88], xmm8, 3

    vextractf128 xmm8, ymm12, 0

    pextrd [rdi + 8], xmm8, 0
    pextrd [rdi + 20], xmm8, 1
    pextrd [rdi + 32], xmm8, 2
    pextrd [rdi + 44], xmm8, 3

    vextractf128 xmm8, ymm12, 1

    pextrd [rdi + 56], xmm8, 0
    pextrd [rdi + 68], xmm8, 1
    pextrd [rdi + 80], xmm8, 2
    pextrd [rdi + 92], xmm8, 3

    add r13, 8

    jmp z_loop
z_end:
    inc r14

    jmp x_loop

x_end:
    add rsp, LOCAL_VARS_SIZE
    mov rsp, rbp
    pop rbp
    ret
