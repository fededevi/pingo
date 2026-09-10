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
#define S_tex           96
#define S_flat         116

/* Stack frame, all 16-byte aligned off %rbp. */
#define F_az      -128
#define F_bz      -112
#define F_cz       -96
#define F_ai       -80
#define F_colbuf   -64
#define F_depbuf   -48
#define F_scratch  -32
#define F_lane     -16

/*
 * One group of four pixels.
 *   in:  xmm0/1/2 = w0/w1/w2 vectors, xmm3/4/5 = per-group steps,
 *        xmm6 = splatted flat colour, %rsi = dst, %rdx = depth,
 *        \lane = memory operand holding the active-lane mask
 *   out: buffers updated, xmm0/1/2 advanced, %rsi/%rdx advanced by 16
 */
.macro SPAN_GROUP lane
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

    movdqu    (%rsi), %xmm9             /* blend colour */
    movdqa    %xmm6, %xmm12
    pand      %xmm15, %xmm12
    movdqa    %xmm15, %xmm13
    pandn     %xmm9, %xmm13
    por       %xmm13, %xmm12
    movdqu    %xmm12, (%rsi)

    paddd     %xmm3, %xmm0
    paddd     %xmm4, %xmm1
    paddd     %xmm5, %xmm2
    addq      $16, %rsi
    addq      $16, %rdx
.endm

    .text
    .globl  pingo_span_sse2
    .type   pingo_span_sse2, @function
pingo_span_sse2:
    movl    S_count(%rdi), %ecx
    testl   %ecx, %ecx
    jle     .Lret_plain                 /* a count of 0 is legitimate */
    cmpq    $0, S_tex(%rdi)
    jne     pingo_span_ref              /* textured: tail-call the reference */

    pushq   %rbp
    movq    %rsp, %rbp
    subq    $144, %rsp                  /* frame; keeps %rbp 16-aligned */

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

    cmpl    $4, %ecx
    jl      .Ltail

.Lloop4:
    SPAN_GROUP .Lall_lanes(%rip)
    subl    $4, %ecx
    cmpl    $4, %ecx
    jge     .Lloop4

.Ltail:
    testl   %ecx, %ecx
    jle     .Ldone

    /* 1..3 pixels left. Stage them through the frame so the identical vector
       body can run without reading past the caller's buffers. */
    movslq  %ecx, %rax
    shlq    $4, %rax                    /* 16 bytes of lane mask per count */
    leaq    .Ltail_mask(%rip), %r11
    movdqa  (%r11,%rax), %xmm7
    movdqa  %xmm7, F_lane(%rbp)

    movl    %ecx, %eax
.Lcopy_in:
    movl    -4(%rsi,%rax,4), %r8d
    movl    %r8d, F_colbuf-4(%rbp,%rax,4)
    movl    -4(%rdx,%rax,4), %r8d
    movl    %r8d, F_depbuf-4(%rbp,%rax,4)
    decl    %eax
    jnz     .Lcopy_in

    movq    %rsi, %r8                   /* remember the real destinations */
    movq    %rdx, %r9
    leaq    F_colbuf(%rbp), %rsi
    leaq    F_depbuf(%rbp), %rdx

    SPAN_GROUP F_lane(%rbp)

    movq    %r8, %rsi
    movq    %r9, %rdx
    movl    %ecx, %eax
.Lcopy_out:
    movl    F_colbuf-4(%rbp,%rax,4), %r10d
    movl    %r10d, -4(%rsi,%rax,4)
    movl    F_depbuf-4(%rbp,%rax,4), %r10d
    movl    %r10d, -4(%rdx,%rax,4)
    decl    %eax
    jnz     .Lcopy_out

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
