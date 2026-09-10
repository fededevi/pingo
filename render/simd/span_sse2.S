/*
 * SSE2 span, four pixels per iteration. Flat-fill path only; the textured
 * path still goes to the C reference.
 *
 * Bit-identity with render/simd/span_ref.c is the requirement, not
 * approximation, and three things had to be exact rather than close:
 *
 *  - A C cast truncates toward zero, so the convert is cvttps2dq. cvtps2dq
 *    rounds to nearest and differs by one depth unit on about half of inputs.
 *
 *  - Neither SSE2 nor AVX2 has a packed float-to-unsigned convert; that
 *    arrives with AVX-512. cvttps2dq is signed, so a value at or above 2^31
 *    wraps. For non-negative d, floor(d - 2^31) == floor(d) - 2^31 exactly
 *    because 2^31 is an integer, so both convert paths are computed and
 *    selected on d < 2^31. Biasing unconditionally would be off by one below
 *    the boundary, where truncation toward zero is not floor.
 *
 *  - The depth compare is unsigned and SSE2 has only pcmpgtd, so both
 *    operands are biased by 0x80000000 and compared signed.
 *
 * The float sum is accumulated left to right - (w0*az + w1*bz) + w2*cz -
 * because float addition is not associative and the reference does it in that
 * order.
 *
 * SSE2 has no per-lane 32-bit masked store: maskmovdqu is byte-granular,
 * non-temporal and slow. So this loads the destination, blends with
 * pand/pandn/por and stores unconditionally, which means it reads the colour
 * and depth it is about to write where the scalar path does not. That is why
 * SSE2's ceiling on an already memory-bound fill is lower than its width
 * suggests.
 *
 * The 1..3 pixel tail is not a second implementation. It copies the remaining
 * pixels into a stack buffer, runs the identical vector body against that, and
 * copies back - so there is no scalar duplicate of the quantisation to drift,
 * and nothing is ever loaded from past the end of the caller's buffers.
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

/* Stack frame, all 16-byte aligned off %rbp. */
#define F_az      -128
#define F_bz      -112
#define F_cz       -96
#define F_ai       -80
#define F_colbuf   -64
#define F_depbuf   -48
#define F_scratch  -32
#define F_lane     -16
/* Textured path only. */
#define F_iaw     -144
#define F_ibw     -160
#define F_icw     -176
#define F_tcax    -192
#define F_tcbx    -208
#define F_tccx    -224
#define F_tcay    -240
#define F_tcby    -256
#define F_tccy    -272
#define F_wdim    -288
#define F_hdim    -304
#define F_wmask   -320
#define F_hmask   -336
#define F_shadev  -352
#define F_idx     -368
#define F_texbuf  -384
#define F_wshift  -400
#define F_texptr  -416

/*
 * One group of four pixels.
 *   in:  xmm0/1/2 = w0/w1/w2 vectors, xmm3/4/5 = per-group steps,
 *        xmm6 = splatted flat colour, %rsi = dst, %rdx = depth,
 *        \lane = memory operand holding the active-lane mask
 *   out: buffers updated, xmm0/1/2 advanced, %rsi/%rdx advanced by 16
 */
.macro DEPTH_PART lane
    cvtdq2ps  %xmm0, %xmm7
    cvtdq2ps  %xmm1, %xmm8
    cvtdq2ps  %xmm2, %xmm9
    mulps     F_az(%rbp), %xmm7
    mulps     F_bz(%rbp), %xmm8
    addps     %xmm8, %xmm7              /* (w0*az + w1*bz) */
    mulps     F_cz(%rbp), %xmm9
    addps     %xmm9, %xmm7              /* + w2*cz */
    xorps     .Lsignbit(%rip), %xmm7    /* negate */
    mulps     F_ai(%rbp), %xmm7         /* xmm7 = depth */

    /* Range guard, matching "depth < 0.0f || depth > 1.0f". Both compares are
       written so a NaN answers false, as the C does. */
    movaps    %xmm7, %xmm10
    cmpltps   .Lzero(%rip), %xmm10      /* depth < 0 */
    movaps    .Lone(%rip), %xmm11
    cmpltps   %xmm7, %xmm11             /* 1 < depth */
    por       %xmm11, %xmm10            /* out of range */

    /* Quantise to uint32 with truncation, selecting across the 2^31 boundary. */
    movaps    %xmm7, %xmm12
    mulps     .Lscale(%rip), %xmm12     /* d = depth * 2^32 */
    movaps    %xmm12, %xmm13
    subps     .Lbias(%rip), %xmm13
    cvttps2dq %xmm12, %xmm8             /* (int)d              */
    cvttps2dq %xmm13, %xmm9             /* (int)(d - 2^31)     */
    por       .Lsignbit(%rip), %xmm9    /* | 0x80000000        */
    movaps    %xmm12, %xmm14
    cmpltps   .Lbias(%rip), %xmm14      /* d < 2^31 */
    pand      %xmm14, %xmm8
    pandn     %xmm9, %xmm14
    por       %xmm14, %xmm8             /* xmm8 = v */

    /* Unsigned v >= stored, via biased signed compare. */
    movdqu    (%rdx), %xmm9             /* stored */
    movdqa    %xmm9, %xmm13
    pxor      .Lsignbit(%rip), %xmm13
    movdqa    %xmm8, %xmm14
    pxor      .Lsignbit(%rip), %xmm14
    pcmpgtd   %xmm14, %xmm13            /* stored > v -> reject */

    por       %xmm13, %xmm10            /* every reason to reject */
    pcmpeqd   %xmm15, %xmm15
    pxor      %xmm10, %xmm15            /* keep = ~reject */
    pand      \lane, %xmm15             /* and only the active lanes */

    movdqa    %xmm8, %xmm12             /* blend depth */
    pand      %xmm15, %xmm12
    movdqa    %xmm15, %xmm13
    pandn     %xmm9, %xmm13
    por       %xmm13, %xmm12
    movdqu    %xmm12, (%rdx)

.endm

/* Blends one 4-pixel colour vector (in \src) through the mask in %xmm15. */
.macro BLEND_COLOUR src
    movdqu    (%rsi), %xmm9
    movdqa    \src, %xmm12
    pand      %xmm15, %xmm12
    movdqa    %xmm15, %xmm13
    pandn     %xmm9, %xmm13
    por       %xmm13, %xmm12
    movdqu    %xmm12, (%rsi)
.endm

/*
 * The 1..3 pixel tail is not a second implementation. It copies the remaining
 * pixels into a stack buffer, the identical vector body runs against that, and
 * the result is copied back - so there is no scalar duplicate of the
 * quantisation to drift out of agreement, and nothing is ever read from past
 * the end of the caller's buffers.
 */
.macro STAGE_TAIL
    movslq  %ecx, %rax
    shlq    $4, %rax                    /* 16 bytes of lane mask per count */
    leaq    .Ltail_mask(%rip), %r11
    movdqa  (%r11,%rax), %xmm7
    movdqa  %xmm7, F_lane(%rbp)

    movl    %ecx, %eax
1:  movl    -4(%rsi,%rax,4), %r8d
    movl    %r8d, F_colbuf-4(%rbp,%rax,4)
    movl    -4(%rdx,%rax,4), %r8d
    movl    %r8d, F_depbuf-4(%rbp,%rax,4)
    decl    %eax
    jnz     1b

    movq    %rsi, %r8
    movq    %rdx, %r9
    leaq    F_colbuf(%rbp), %rsi
    leaq    F_depbuf(%rbp), %rdx
.endm

.macro UNSTAGE_TAIL
    movq    %r8, %rsi
    movq    %r9, %rdx
    movl    %ecx, %eax
1:  movl    F_colbuf-4(%rbp,%rax,4), %r10d
    movl    %r10d, -4(%rsi,%rax,4)
    movl    F_depbuf-4(%rbp,%rax,4), %r10d
    movl    %r10d, -4(%rdx,%rax,4)
    decl    %eax
    jnz     1b
.endm

.macro ADVANCE
    paddd     %xmm3, %xmm0
    paddd     %xmm4, %xmm1
    paddd     %xmm5, %xmm2
    addq      $16, %rsi
    addq      $16, %rdx
.endm

/*
 * Textured completion, pow2 sampler only. Consumes the depth-keep mask in
 * %xmm15 and the still-current w vectors in %xmm0/1/2.
 *
 * The reference writes depth and then abandons the pixel when interpInvW is
 * zero, so the colour mask is the depth mask AND (interpInvW != 0) - the two
 * masks are genuinely different and using one for both would write colour to
 * a pixel the reference leaves alone.
 *
 * The texel index needs no clamping: sx and sy are masked to the texture
 * dimensions, so even the 0x80000000 that cvttps2dq yields for an infinity or
 * a NaN lands in range. That matters because a lane whose interpInvW is zero
 * divides by zero and its coordinates are meaningless, yet the load still
 * happens.
 */
.macro SPAN_TEXTURED
    cvtdq2ps  %xmm0, %xmm7
    cvtdq2ps  %xmm1, %xmm8
    cvtdq2ps  %xmm2, %xmm9

    /* interpInvW, left to right as the reference accumulates it. */
    movaps    %xmm7, %xmm10
    mulps     F_iaw(%rbp), %xmm10
    movaps    %xmm8, %xmm11
    mulps     F_ibw(%rbp), %xmm11
    addps     %xmm11, %xmm10
    movaps    %xmm9, %xmm11
    mulps     F_icw(%rbp), %xmm11
    addps     %xmm11, %xmm10            /* xmm10 = interpInvW */

    /* interpInvW != 0, which the colour mask needs. */
    movaps    %xmm10, %xmm14
    cmpneqps  .Lzero(%rip), %xmm14
    pand      %xmm14, %xmm15            /* colour keep = depth keep & nonzero */

    movaps    .Lone(%rip), %xmm12
    divps     %xmm10, %xmm12            /* w = 1/interpInvW; divps, not rcpps -
                                           the reference divides and rcpps is
                                           an approximation */

    /* u = (w0*tca.x + w1*tcb.x + w2*tcc.x) * w, then the pow2 sampler:
       sx = (int)(u * (wmask+1)) & wmask. */
    movaps    %xmm7, %xmm10
    mulps     F_tcax(%rbp), %xmm10
    movaps    %xmm8, %xmm11
    mulps     F_tcbx(%rbp), %xmm11
    addps     %xmm11, %xmm10
    movaps    %xmm9, %xmm11
    mulps     F_tccx(%rbp), %xmm11
    addps     %xmm11, %xmm10
    mulps     %xmm12, %xmm10
    mulps     F_wdim(%rbp), %xmm10
    cvttps2dq %xmm10, %xmm10
    pand      F_wmask(%rbp), %xmm10     /* xmm10 = sx */

    movaps    %xmm7, %xmm13
    mulps     F_tcay(%rbp), %xmm13
    movaps    %xmm8, %xmm11
    mulps     F_tcby(%rbp), %xmm11
    addps     %xmm11, %xmm13
    movaps    %xmm9, %xmm11
    mulps     F_tccy(%rbp), %xmm11
    addps     %xmm11, %xmm13
    mulps     %xmm12, %xmm13
    mulps     F_hdim(%rbp), %xmm13
    cvttps2dq %xmm13, %xmm13
    pand      F_hmask(%rbp), %xmm13     /* xmm13 = sy */

    /* idx = sx + sy * (wmask+1). The dimension is a power of two, so this is
       a shift - SSE2 has no 32-bit packed multiply. */
    pslld     F_wshift(%rbp), %xmm13
    paddd     %xmm10, %xmm13
    movdqa    %xmm13, F_idx(%rbp)

    /* Four scalar loads: SSE2 has no gather. */
    movq      F_texptr(%rbp), %r11
    movl      F_idx+0(%rbp), %eax
    movl      (%r11,%rax,4), %r10d
    movl      %r10d, F_texbuf+0(%rbp)
    movl      F_idx+4(%rbp), %eax
    movl      (%r11,%rax,4), %r10d
    movl      %r10d, F_texbuf+4(%rbp)
    movl      F_idx+8(%rbp), %eax
    movl      (%r11,%rax,4), %r10d
    movl      %r10d, F_texbuf+8(%rbp)
    movl      F_idx+12(%rbp), %eax
    movl      (%r11,%rax,4), %r10d
    movl      %r10d, F_texbuf+12(%rbp)
    movdqa    F_texbuf(%rbp), %xmm7

    /*
     * Shade. The multiply is computed rather than looked up even when the
     * caller supplied a shade table, and the result is still bit-identical:
     * pixel_shade_table_init fills v[i] with (uint8_t)(i*f), the very
     * expression pixel_mul evaluates, and object.c builds the table from the
     * same factor it passes here. That matters because neither SSE2 nor AVX2
     * can gather bytes, so a vectorised table lookup would be four scalar
     * loads per channel.
     *
     * F_shadev is {f, f, f, 1.0} so alpha passes through: (uint8_t)(a*1.0f)
     * is a, which is what pixel_mul does by copying p.a.
     */
    pxor      %xmm14, %xmm14
    movdqa    %xmm7, %xmm8
    punpcklbw %xmm14, %xmm8             /* pixels 0,1 as words */
    movdqa    %xmm7, %xmm9
    punpckhbw %xmm14, %xmm9             /* pixels 2,3 as words */

    movdqa    %xmm8, %xmm10
    punpcklwd %xmm14, %xmm10            /* pixel 0: b,g,r,a as dwords */
    punpckhwd %xmm14, %xmm8             /* pixel 1 */
    movdqa    %xmm9, %xmm11
    punpcklwd %xmm14, %xmm11            /* pixel 2 */
    punpckhwd %xmm14, %xmm9             /* pixel 3 */

    cvtdq2ps  %xmm10, %xmm10
    cvtdq2ps  %xmm8,  %xmm8
    cvtdq2ps  %xmm11, %xmm11
    cvtdq2ps  %xmm9,  %xmm9
    mulps     F_shadev(%rbp), %xmm10
    mulps     F_shadev(%rbp), %xmm8
    mulps     F_shadev(%rbp), %xmm11
    mulps     F_shadev(%rbp), %xmm9
    cvttps2dq %xmm10, %xmm10
    cvttps2dq %xmm8,  %xmm8
    cvttps2dq %xmm11, %xmm11
    cvttps2dq %xmm9,  %xmm9

    packssdw  %xmm8,  %xmm10            /* pixels 0,1 back to words */
    packssdw  %xmm9,  %xmm11            /* pixels 2,3 */
    packuswb  %xmm11, %xmm10            /* all four back to bytes */

    BLEND_COLOUR %xmm10
.endm

.macro SPAN_FLAT lane
    DEPTH_PART \lane
    BLEND_COLOUR %xmm6
    ADVANCE
.endm

.macro SPAN_TEX lane
    DEPTH_PART \lane
    SPAN_TEXTURED
    ADVANCE
.endm

    .text
    .globl  pingo_span_sse2
    .p2align 4
    .type   pingo_span_sse2, @function
pingo_span_sse2:
    movl    S_count(%rdi), %ecx
    testl   %ecx, %ecx
    jle     .Lret_plain                 /* a count of 0 is legitimate */

    /* Flat, textured-pow2, or hand back to the reference. Only the pow2
       sampler is implemented here; every texture the project ships is
       power-of-two and the general sampler is for the rest, so it is not
       worth a second vector path. */
    cmpq    $0, S_tex(%rdi)
    je      .Lsetup
    cmpl    $0, S_pow2(%rdi)
    je      pingo_span_ref

.Lsetup:
    pushq   %rbp
    movq    %rsp, %rbp
    subq    $432, %rsp                  /* frame; keeps %rbp 16-aligned */

    movq    S_dst(%rdi), %rsi
    movq    S_depth(%rdi), %rdx

    /* Splat the per-triangle floats into the frame, so all sixteen xmm
       registers stay available as scratch in the body. */
    movss   S_az(%rdi), %xmm7
    shufps  $0, %xmm7, %xmm7
    movaps  %xmm7, F_az(%rbp)
    movss   S_bz(%rdi), %xmm7
    shufps  $0, %xmm7, %xmm7
    movaps  %xmm7, F_bz(%rbp)
    movss   S_cz(%rdi), %xmm7
    shufps  $0, %xmm7, %xmm7
    movaps  %xmm7, F_cz(%rbp)
    movss   S_areaInverse(%rdi), %xmm7
    shufps  $0, %xmm7, %xmm7
    movaps  %xmm7, F_ai(%rbp)

    movd    S_flat(%rdi), %xmm6
    pshufd  $0, %xmm6, %xmm6

    /* w vectors: {w, w+dw, w+2dw, w+3dw}. SSE2 has no pmulld, so these are
       built once in general-purpose registers rather than multiplied. */
    movl    S_w0(%rdi), %eax
    movl    S_dw0(%rdi), %r8d
    movl    %eax, F_scratch+0(%rbp)
    addl    %r8d, %eax
    movl    %eax, F_scratch+4(%rbp)
    addl    %r8d, %eax
    movl    %eax, F_scratch+8(%rbp)
    addl    %r8d, %eax
    movl    %eax, F_scratch+12(%rbp)
    movdqa  F_scratch(%rbp), %xmm0

    movl    S_w1(%rdi), %eax
    movl    S_dw1(%rdi), %r9d
    movl    %eax, F_scratch+0(%rbp)
    addl    %r9d, %eax
    movl    %eax, F_scratch+4(%rbp)
    addl    %r9d, %eax
    movl    %eax, F_scratch+8(%rbp)
    addl    %r9d, %eax
    movl    %eax, F_scratch+12(%rbp)
    movdqa  F_scratch(%rbp), %xmm1

    movl    S_w2(%rdi), %eax
    movl    S_dw2(%rdi), %r10d
    movl    %eax, F_scratch+0(%rbp)
    addl    %r10d, %eax
    movl    %eax, F_scratch+4(%rbp)
    addl    %r10d, %eax
    movl    %eax, F_scratch+8(%rbp)
    addl    %r10d, %eax
    movl    %eax, F_scratch+12(%rbp)
    movdqa  F_scratch(%rbp), %xmm2

    /* Per-group step is four pixels' worth. */
    shll    $2, %r8d
    movd    %r8d, %xmm3
    pshufd  $0, %xmm3, %xmm3
    shll    $2, %r9d
    movd    %r9d, %xmm4
    pshufd  $0, %xmm4, %xmm4
    shll    $2, %r10d
    movd    %r10d, %xmm5
    pshufd  $0, %xmm5, %xmm5

    cmpq    $0, S_tex(%rdi)
    je      .Lflat_loop_entry

    /* ---- textured setup ------------------------------------------- */
    movss   S_invAw(%rdi), %xmm7
    shufps  $0, %xmm7, %xmm7
    movaps  %xmm7, F_iaw(%rbp)
    movss   S_invBw(%rdi), %xmm7
    shufps  $0, %xmm7, %xmm7
    movaps  %xmm7, F_ibw(%rbp)
    movss   S_invCw(%rdi), %xmm7
    shufps  $0, %xmm7, %xmm7
    movaps  %xmm7, F_icw(%rbp)

    movss   S_tca+0(%rdi), %xmm7
    shufps  $0, %xmm7, %xmm7
    movaps  %xmm7, F_tcax(%rbp)
    movss   S_tcb+0(%rdi), %xmm7
    shufps  $0, %xmm7, %xmm7
    movaps  %xmm7, F_tcbx(%rbp)
    movss   S_tcc+0(%rdi), %xmm7
    shufps  $0, %xmm7, %xmm7
    movaps  %xmm7, F_tccx(%rbp)
    movss   S_tca+4(%rdi), %xmm7
    shufps  $0, %xmm7, %xmm7
    movaps  %xmm7, F_tcay(%rbp)
    movss   S_tcb+4(%rdi), %xmm7
    shufps  $0, %xmm7, %xmm7
    movaps  %xmm7, F_tcby(%rbp)
    movss   S_tcc+4(%rdi), %xmm7
    shufps  $0, %xmm7, %xmm7
    movaps  %xmm7, F_tccy(%rbp)

    /* Masks as integers, and the dimensions as floats. */
    movl    S_wmask(%rdi), %eax
    movd    %eax, %xmm7
    pshufd  $0, %xmm7, %xmm7
    movdqa  %xmm7, F_wmask(%rbp)
    incl    %eax                        /* wmask + 1, a power of two */
    cvtsi2ss %eax, %xmm7
    shufps  $0, %xmm7, %xmm7
    movaps  %xmm7, F_wdim(%rbp)
    /* log2 of the width, because idx = sx + sy*(wmask+1) has to be a shift:
       SSE2 has no 32-bit packed multiply. */
    bsfl    %eax, %eax
    movq    $0, F_wshift+8(%rbp)
    movq    %rax, F_wshift(%rbp)

    movl    S_hmask(%rdi), %eax
    movd    %eax, %xmm7
    pshufd  $0, %xmm7, %xmm7
    movdqa  %xmm7, F_hmask(%rbp)
    incl    %eax
    cvtsi2ss %eax, %xmm7
    shufps  $0, %xmm7, %xmm7
    movaps  %xmm7, F_hdim(%rbp)

    movq    S_tex(%rdi), %rax
    movq    T_pixels(%rax), %rax
    movq    %rax, F_texptr(%rbp)

    /* {f, f, f, 1.0}: alpha passes through, matching pixel_mul copying p.a. */
    movss   S_factor(%rdi), %xmm7
    shufps  $0, %xmm7, %xmm7
    movaps  %xmm7, F_shadev(%rbp)
    movl    $0x3f800000, %eax           /* 1.0f */
    movl    %eax, F_shadev+12(%rbp)

    cmpl    $4, %ecx
    jl      .Ltex_tail
    .p2align 5                      /* loop head on a 32-byte boundary, so
                                           performance does not depend on where
                                           the linker happened to put it */
.Ltex_loop:
    SPAN_TEX .Lall_lanes(%rip)
    subl    $4, %ecx
    cmpl    $4, %ecx
    jge     .Ltex_loop
.Ltex_tail:
    testl   %ecx, %ecx
    jle     .Ldone
    STAGE_TAIL
    SPAN_TEX F_lane(%rbp)
    UNSTAGE_TAIL
    jmp     .Ldone

.Lflat_loop_entry:
    cmpl    $4, %ecx
    jl      .Lflat_tail
    .p2align 5                      /* loop head on a 32-byte boundary, so
                                           performance does not depend on where
                                           the linker happened to put it */
.Lflat_loop:
    SPAN_FLAT .Lall_lanes(%rip)
    subl    $4, %ecx
    cmpl    $4, %ecx
    jge     .Lflat_loop
.Lflat_tail:
    testl   %ecx, %ecx
    jle     .Ldone
    STAGE_TAIL
    SPAN_FLAT F_lane(%rbp)
    UNSTAGE_TAIL

.Ldone:
    movq    %rbp, %rsp
    popq    %rbp
.Lret_plain:
    ret
    .size   pingo_span_sse2, .-pingo_span_sse2

/*
 * Plain .rodata, deliberately not .rodata.cst16 with the "M" flag. A
 * mergeable section declares each 16-byte unit independently mergeable, so
 * the assembler may split and reorder them and a label cannot be relied on to
 * address a contiguous multi-row table. .Ltail_mask is indexed at
 * base + 16*count, which silently resolved to the start of the section
 * instead - the keep mask came back as the float constant 1.0, and the tail
 * wrote garbage while the main loop, whose mask is a single 16-byte unit,
 * worked perfectly.
 */
    .section .rodata,"a",@progbits
    .align  16
.Lzero:      .float 0.0, 0.0, 0.0, 0.0
.Lone:       .float 1.0, 1.0, 1.0, 1.0
.Lscale:     .float 4294967296.0, 4294967296.0, 4294967296.0, 4294967296.0
.Lbias:      .float 2147483648.0, 2147483648.0, 2147483648.0, 2147483648.0
.Lsignbit:   .long  0x80000000, 0x80000000, 0x80000000, 0x80000000
.Lall_lanes: .long  -1, -1, -1, -1
/* Active-lane masks indexed by remaining count: 0, 1, 2 or 3 low lanes. */
.Ltail_mask: .long 0, 0, 0, 0
             .long -1, 0, 0, 0
             .long -1, -1, 0, 0
             .long -1, -1, -1, 0

    .section .note.GNU-stack,"",@progbits
