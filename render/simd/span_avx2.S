/*
 * AVX2 span, eight pixels per iteration.
 *
 * Bit-identical to render/simd/span_ref.c, by the same three rules the SSE2
 * implementation follows: cvttps2dq because a C cast truncates; a selected
 * rather than unconditional bias across 2^31, because no packed
 * float-to-unsigned exists before AVX-512 and truncation toward zero is not
 * floor below the boundary; and the left-to-right float accumulation the
 * reference uses, because float addition is not associative.
 *
 * Three things are genuinely better here than at four lanes, rather than
 * merely twice as wide:
 *
 *  - vpmaskmovd is a real masked load AND store. The SSE2 path has neither,
 *    so it reads the colour and depth it is about to write and stages the
 *    1..3 pixel tail through a stack buffer. Here the tail is just a lane
 *    mask: a masked load does not fault on its masked-off elements, so
 *    nothing is read or written past the caller's buffers and there is no
 *    staging at all.
 *
 *  - vpmaxud gives the unsigned depth compare directly. v >= stored is
 *    max(v, stored) == v, with none of SSE2's biasing into signed space.
 *
 *  - vpgatherdd fetches eight texels at once. A byte gather still does not
 *    exist, which is why the shade is computed rather than looked up.
 *
 * The load mask is the lane mask, never the keep mask: keep depends on the
 * value loaded, so using it would be circular.
 *
 * The shade runs on the two 128-bit halves rather than across the whole
 * register. AVX2's byte unpacks and packs operate within each 128-bit lane,
 * so a 256-bit version needs lane fixups to put the pixels back in order;
 * doing the halves separately is the same instruction count without that
 * class of bug.
 */

#define S_dst            0
#define S_depth          8
#define S_count         16
#define S_w0            20
#define S_w1            24
#define S_w2            28
#define S_dw0           32
#define S_dw1           36
#define S_dw2           40
#define S_az            44
#define S_bz            48
#define S_cz            52
#define S_areaInverse   56
#define S_invAw         60
#define S_invBw         64
#define S_invCw         68
#define S_tca           72
#define S_tcb           80
#define S_tcc           88
#define S_tex           96
#define S_wmask        104
#define S_hmask        108
#define S_pow2         112
#define S_flat         116
#define S_factor       128

#define T_pixels 8

/* Frame slots, 32-byte aligned, addressed off %rsp. %rbp cannot be used as
   the base: it holds the unaligned entry %rsp so the epilogue can restore it,
   and vmovaps needs 32-byte alignment. */
#define F_az        0
#define F_bz       32
#define F_cz       64
#define F_ai       96
#define F_iaw     128
#define F_ibw     160
#define F_icw     192
#define F_tcax    224
#define F_tcbx    256
#define F_tccx    288
#define F_tcay    320
#define F_tcby    352
#define F_tccy    384
#define F_wdim    416
#define F_hdim    448
#define F_wmask   480
#define F_hmask   512
#define F_shadev  544
#define F_scratch 576
#define F_idx     608
#define F_texbuf  640
#define F_wshift  672
#define F_texptr  704
#define F_step1   736
#define F_step2   768
#define F_SIZE    832

/* Splat one float from the PingoSpan into a frame slot. */
.macro SPLAT off, slot
    vbroadcastss \off(%rdi), %ymm7
    vmovaps      %ymm7, \slot(%rsp)
.endm

/*
 * Depth for one group of eight. Leaves the quantised value in %ymm8 and the
 * keep mask in %ymm15, and writes the depth buffer. \lane is the active-lane
 * mask register.
 */
.macro DEPTH_PART lane
    vcvtdq2ps %ymm0, %ymm4
    vcvtdq2ps %ymm1, %ymm5
    vcvtdq2ps %ymm2, %ymm6

    vmulps    F_az(%rsp), %ymm4, %ymm10
    vmulps    F_bz(%rsp), %ymm5, %ymm11
    vaddps    %ymm11, %ymm10, %ymm10     /* (w0*az + w1*bz) */
    vmulps    F_cz(%rsp), %ymm6, %ymm11
    vaddps    %ymm11, %ymm10, %ymm10     /* + w2*cz */
    vxorps    .Lsignbit(%rip), %ymm10, %ymm10   /* negate */
    vmulps    F_ai(%rsp), %ymm10, %ymm10        /* depth */

    /* "depth < 0.0f || depth > 1.0f", both false for a NaN as the C is. */
    vcmpltps  .Lzero(%rip), %ymm10, %ymm12
    vcmpgtps  .Lone(%rip), %ymm10, %ymm13
    vorps     %ymm13, %ymm12, %ymm12

    /* Quantise, selecting across the 2^31 boundary. */
    vmulps    .Lscale(%rip), %ymm10, %ymm14
    vsubps    .Lbias(%rip), %ymm14, %ymm7
    vcvttps2dq %ymm14, %ymm8
    vcvttps2dq %ymm7, %ymm7
    vpor      .Lsignbit(%rip), %ymm7, %ymm7
    vcmpltps  .Lbias(%rip), %ymm14, %ymm11
    vpand     %ymm11, %ymm8, %ymm8
    vpandn    %ymm7, %ymm11, %ymm11
    vpor      %ymm11, %ymm8, %ymm8       /* %ymm8 = v */

    /* Masked load: the mask is the lane mask, because keep depends on what
       is loaded. Masked-off elements do not fault, which is what lets the
       tail be a mask rather than a staged copy. */
    vpmaskmovd (%rdx), \lane, %ymm13     /* stored */
    vpmaxud   %ymm13, %ymm8, %ymm14
    vpcmpeqd  %ymm8, %ymm14, %ymm14      /* max(v,stored) == v -> v >= stored */

    vpandn    %ymm14, %ymm12, %ymm15     /* in range AND passes depth */
    vpand     \lane, %ymm15, %ymm15
    vpmaskmovd %ymm8, %ymm15, (%rdx)
.endm

.macro ADVANCE
    vpaddd    %ymm3, %ymm0, %ymm0
    vpaddd    F_step1(%rsp), %ymm1, %ymm1
    vpaddd    F_step2(%rsp), %ymm2, %ymm2
    addq      $32, %rsi
    addq      $32, %rdx
.endm

/* Shade four pixels held as bytes in \x, in place, using F_shadev. */
.macro SHADE4 x, t0, t1, t2, t3
    vpxor       %xmm14, %xmm14, %xmm14
    vpunpcklbw  %xmm14, \x, \t0
    vpunpckhbw  %xmm14, \x, \t1
    vpunpcklwd  %xmm14, \t0, \t2
    vpunpckhwd  %xmm14, \t0, \t0
    vpunpcklwd  %xmm14, \t1, \t3
    vpunpckhwd  %xmm14, \t1, \t1
    vcvtdq2ps   \t2, \t2
    vcvtdq2ps   \t0, \t0
    vcvtdq2ps   \t3, \t3
    vcvtdq2ps   \t1, \t1
    vmulps      F_shadev(%rsp), \t2, \t2
    vmulps      F_shadev(%rsp), \t0, \t0
    vmulps      F_shadev(%rsp), \t3, \t3
    vmulps      F_shadev(%rsp), \t1, \t1
    vcvttps2dq  \t2, \t2
    vcvttps2dq  \t0, \t0
    vcvttps2dq  \t3, \t3
    vcvttps2dq  \t1, \t1
    vpackssdw   \t0, \t2, \t2
    vpackssdw   \t1, \t3, \t3
    vpackuswb   \t3, \t2, \x
.endm

/*
 * Textured completion. Consumes the depth-keep mask in %ymm15, the lane mask
 * in %ymm9, and the converted w vectors %ymm4/5/6 that DEPTH_PART leaves.
 *
 * A macro and not a call: every frame slot here is addressed off %rsp, and a
 * call pushes a return address, which would shift %rsp by eight and silently
 * misread every one of them.
 */
.macro SPAN_TEXTURED
    vcvtdq2ps %ymm0, %ymm4
    vcvtdq2ps %ymm1, %ymm5
    vcvtdq2ps %ymm2, %ymm6

    vmulps    F_iaw(%rsp), %ymm4, %ymm10
    vmulps    F_ibw(%rsp), %ymm5, %ymm11
    vaddps    %ymm11, %ymm10, %ymm10
    vmulps    F_icw(%rsp), %ymm6, %ymm11
    vaddps    %ymm11, %ymm10, %ymm10     /* interpInvW */

    /* The reference writes depth and then abandons the pixel when interpInvW
       is zero, so the colour mask carries an extra term the depth mask does
       not. Using one for both would paint a pixel the reference leaves. */
    vcmpneqps .Lzero(%rip), %ymm10, %ymm12
    vpand     %ymm12, %ymm15, %ymm15

    vmovaps   .Lone(%rip), %ymm12
    vdivps    %ymm10, %ymm12, %ymm12     /* w; divps not rcpps - the
                                            reference divides */

    vmulps    F_tcax(%rsp), %ymm4, %ymm10
    vmulps    F_tcbx(%rsp), %ymm5, %ymm11
    vaddps    %ymm11, %ymm10, %ymm10
    vmulps    F_tccx(%rsp), %ymm6, %ymm11
    vaddps    %ymm11, %ymm10, %ymm10
    vmulps    %ymm12, %ymm10, %ymm10
    vmulps    F_wdim(%rsp), %ymm10, %ymm10
    vcvttps2dq %ymm10, %ymm10
    vpand     F_wmask(%rsp), %ymm10, %ymm10   /* sx */

    vmulps    F_tcay(%rsp), %ymm4, %ymm13
    vmulps    F_tcby(%rsp), %ymm5, %ymm11
    vaddps    %ymm11, %ymm13, %ymm13
    vmulps    F_tccy(%rsp), %ymm6, %ymm11
    vaddps    %ymm11, %ymm13, %ymm13
    vmulps    %ymm12, %ymm13, %ymm13
    vmulps    F_hdim(%rsp), %ymm13, %ymm13
    vcvttps2dq %ymm13, %ymm13
    vpand     F_hmask(%rsp), %ymm13, %ymm13   /* sy */

    /* idx = sx + sy*(wmask+1); the dimension is a power of two. */
    vpslld    F_wshift(%rsp), %ymm13, %ymm13
    vpaddd    %ymm10, %ymm13, %ymm13

    /* Eight texels at once. sx and sy are masked to the texture, so even the
       0x80000000 that cvttps2dq yields for the infinity a zero interpInvW
       produces still indexes in range - no clamping needed. */
    movq      F_texptr(%rsp), %r11
    vmovdqa   %ymm9, %ymm11              /* gather mask is consumed */
    vpxor     %ymm7, %ymm7, %ymm7
    vpgatherdd %ymm11, (%r11,%ymm13,4), %ymm7

    /* Shade the two 128-bit halves separately: AVX2's byte packs work within
       a lane, and doing them apart avoids the lane-order fixups. */
    vextracti128 $1, %ymm7, %xmm8
    SHADE4    %xmm7, %xmm10, %xmm11, %xmm12, %xmm13
    SHADE4    %xmm8, %xmm10, %xmm11, %xmm12, %xmm13
    vinserti128 $1, %xmm8, %ymm7, %ymm7

    vpmaskmovd %ymm7, %ymm15, (%rsi)
.endm

    .text
    .globl  pingo_span_avx2
    .type   pingo_span_avx2, @function
pingo_span_avx2:
    movl    S_count(%rdi), %ecx
    testl   %ecx, %ecx
    jle     .Lret_plain
    cmpq    $0, S_tex(%rdi)
    je      .Lsetup
    cmpl    $0, S_pow2(%rdi)
    je      pingo_span_ref              /* general sampler stays scalar */

.Lsetup:
    pushq   %rbp
    movq    %rsp, %rbp
    andq    $-32, %rsp
    subq    $F_SIZE, %rsp

    movq    S_dst(%rdi), %rsi
    movq    S_depth(%rdi), %rdx

    SPLAT   S_az, F_az
    SPLAT   S_bz, F_bz
    SPLAT   S_cz, F_cz
    SPLAT   S_areaInverse, F_ai

    /* w vectors {w, w+dw, ... w+7dw}, built in GPRs: there is no cheap
       packed 32-bit multiply by a lane index. */
    movl    S_w0(%rdi), %eax
    movl    S_dw0(%rdi), %r8d
    movl    %eax, F_scratch+0(%rsp)
    .irp i, 4,8,12,16,20,24,28
    addl    %r8d, %eax
    movl    %eax, F_scratch+\i(%rsp)
    .endr
    vmovdqa F_scratch(%rsp), %ymm0

    movl    S_w1(%rdi), %eax
    movl    S_dw1(%rdi), %r9d
    movl    %eax, F_scratch+0(%rsp)
    .irp i, 4,8,12,16,20,24,28
    addl    %r9d, %eax
    movl    %eax, F_scratch+\i(%rsp)
    .endr
    vmovdqa F_scratch(%rsp), %ymm1

    movl    S_w2(%rdi), %eax
    movl    S_dw2(%rdi), %r10d
    movl    %eax, F_scratch+0(%rsp)
    .irp i, 4,8,12,16,20,24,28
    addl    %r10d, %eax
    movl    %eax, F_scratch+\i(%rsp)
    .endr
    vmovdqa F_scratch(%rsp), %ymm2

    /* Per-group step is eight pixels' worth. Only ymm3 is spare to hold one;
       the other two live in rodata slots written here. */
    shll    $3, %r8d
    movl    %r8d, F_scratch(%rsp)
    vbroadcastss F_scratch(%rsp), %ymm3
    shll    $3, %r9d
    movl    %r9d, F_scratch(%rsp)
    vbroadcastss F_scratch(%rsp), %ymm7
    vmovdqa %ymm7, F_step1(%rsp)
    shll    $3, %r10d
    movl    %r10d, F_scratch(%rsp)
    vbroadcastss F_scratch(%rsp), %ymm7
    vmovdqa %ymm7, F_step2(%rsp)

    cmpq    $0, S_tex(%rdi)
    jne     .Ltex_setup

    vbroadcastss S_flat(%rdi), %ymm6
    vmovdqa %ymm6, F_texbuf(%rsp)       /* park it; ymm6 is scratch in the body */
    jmp     .Lflat_entry

.Ltex_setup:
    SPLAT   S_invAw, F_iaw
    SPLAT   S_invBw, F_ibw
    SPLAT   S_invCw, F_icw
    SPLAT   S_tca+0, F_tcax
    SPLAT   S_tcb+0, F_tcbx
    SPLAT   S_tcc+0, F_tccx
    SPLAT   S_tca+4, F_tcay
    SPLAT   S_tcb+4, F_tcby
    SPLAT   S_tcc+4, F_tccy

    movl    S_wmask(%rdi), %eax
    movl    %eax, F_scratch(%rsp)
    vbroadcastss F_scratch(%rsp), %ymm7
    vmovdqa %ymm7, F_wmask(%rsp)
    incl    %eax
    cvtsi2ssl %eax, %xmm7
    movss   %xmm7, F_scratch(%rsp)
    vbroadcastss F_scratch(%rsp), %ymm7
    vmovaps %ymm7, F_wdim(%rsp)
    bsfl    %eax, %eax                  /* log2 width: idx uses a shift */
    movq    $0, F_wshift+8(%rsp)
    movq    %rax, F_wshift(%rsp)

    movl    S_hmask(%rdi), %eax
    movl    %eax, F_scratch(%rsp)
    vbroadcastss F_scratch(%rsp), %ymm7
    vmovdqa %ymm7, F_hmask(%rsp)
    incl    %eax
    cvtsi2ssl %eax, %xmm7
    movss   %xmm7, F_scratch(%rsp)
    vbroadcastss F_scratch(%rsp), %ymm7
    vmovaps %ymm7, F_hdim(%rsp)

    movq    S_tex(%rdi), %rax
    movq    T_pixels(%rax), %rax
    movq    %rax, F_texptr(%rsp)

    /* {f,f,f,1.0} per pixel, so alpha passes through unchanged. */
    vbroadcastss S_factor(%rdi), %ymm7
    vmovaps %ymm7, F_shadev(%rsp)
    movl    $0x3f800000, %eax
    movl    %eax, F_shadev+12(%rsp)
    movl    %eax, F_shadev+28(%rsp)

.Ltex_entry:
    vmovdqa .Lall_lanes(%rip), %ymm9
    cmpl    $8, %ecx
    jl      .Ltex_tail
.Ltex_loop:
    vmovdqa .Lall_lanes(%rip), %ymm9
    DEPTH_PART %ymm9
    SPAN_TEXTURED
    ADVANCE
    subl    $8, %ecx
    cmpl    $8, %ecx
    jge     .Ltex_loop
.Ltex_tail:
    testl   %ecx, %ecx
    jle     .Ldone
    movslq  %ecx, %rax
    shlq    $5, %rax
    leaq    .Ltail_mask(%rip), %r11
    vmovdqa (%r11,%rax), %ymm9
    DEPTH_PART %ymm9
    SPAN_TEXTURED
    jmp     .Ldone

.Lflat_entry:
    cmpl    $8, %ecx
    jl      .Lflat_tail
.Lflat_loop:
    vmovdqa .Lall_lanes(%rip), %ymm9
    DEPTH_PART %ymm9
    vmovdqa F_texbuf(%rsp), %ymm6
    vpmaskmovd %ymm6, %ymm15, (%rsi)
    ADVANCE
    subl    $8, %ecx
    cmpl    $8, %ecx
    jge     .Lflat_loop
.Lflat_tail:
    testl   %ecx, %ecx
    jle     .Ldone
    movslq  %ecx, %rax
    shlq    $5, %rax
    leaq    .Ltail_mask(%rip), %r11
    vmovdqa (%r11,%rax), %ymm9
    DEPTH_PART %ymm9
    vmovdqa F_texbuf(%rsp), %ymm6
    vpmaskmovd %ymm6, %ymm15, (%rsi)

.Ldone:
    vzeroupper                          /* or every later SSE instruction in
                                           the caller pays a transition */
    movq    %rbp, %rsp
    popq    %rbp
.Lret_plain:
    ret

    .size   pingo_span_avx2, .-pingo_span_avx2

    .section .rodata,"a",@progbits
    .align  32
.Lzero:      .float 0,0,0,0,0,0,0,0
.Lone:       .float 1.0,1.0,1.0,1.0,1.0,1.0,1.0,1.0
.Lscale:     .float 4294967296.0,4294967296.0,4294967296.0,4294967296.0,4294967296.0,4294967296.0,4294967296.0,4294967296.0
.Lbias:      .float 2147483648.0,2147483648.0,2147483648.0,2147483648.0,2147483648.0,2147483648.0,2147483648.0,2147483648.0
.Lsignbit:   .long 0x80000000,0x80000000,0x80000000,0x80000000,0x80000000,0x80000000,0x80000000,0x80000000
.Lall_lanes: .long -1,-1,-1,-1,-1,-1,-1,-1
/* Active-lane masks indexed by remaining count, 0..7 low lanes. */
.Ltail_mask:
    .long 0,0,0,0,0,0,0,0
    .long -1,0,0,0,0,0,0,0
    .long -1,-1,0,0,0,0,0,0
    .long -1,-1,-1,0,0,0,0,0
    .long -1,-1,-1,-1,0,0,0,0
    .long -1,-1,-1,-1,-1,0,0,0
    .long -1,-1,-1,-1,-1,-1,0,0
    .long -1,-1,-1,-1,-1,-1,-1,0

    .section .note.GNU-stack,"",@progbits
