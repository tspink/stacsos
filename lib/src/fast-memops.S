/* SPDX-License-Identifier: MIT */

/* StACSOS Kernel - Core
 *
 * Copyright (C) University of St Andrews 2024.  All Rights Reserved.
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
.text

/* -------------------------- */
/* memcpy                     */
/* -------------------------- */

.align 16
.globl __x86_memcpy
.type __x86_memcpy,%function
__x86_memcpy:
	// memcpy returns the destination pointer, so store
	// it into RAX
	mov %rdi, %rax

	// Clear the direction flag, so that the string operation
	// works on increasing memory addresses.
	cld

	// Load the number of bytes to copy, and divide by eight.
	mov %rdx, %rcx
	shr $3, %rcx

	// Perform the 8-byte string copy.
	rep movsq %ds:(%rsi), %es:(%rdi)

	// Copy the remaining bytes, if not a multiple of eight.
	mov %edx, %ecx
	and $7, %ecx

	rep movsb %ds:(%rsi), %es:(%rdi)

	// Finished!
	ret
.size __x86_memcpy,.-__x86_memcpy

/* -------------------------- */
/* memcmp                     */
/* -------------------------- */

.align 16
.globl __x86_memcmp
.type __x86_memcmp,%function
__x86_memcmp:
	test %rdx, %rdx
	je 3f
	xor %eax, %eax
	jmp 2f

.align 16
1:
	add $1, %rax
	cmp %rax, %rdx
	je 3f

.align 16
2:
	movzbl (%rdi,%rax), %ecx
	movzbl (%rsi,%rax), %r8d
	cmp %r8b, %cl
	je 1b
	movzbl %cl, %eax
	sub %r8d, %eax
	ret

.align 16
3:
	xor %eax, %eax
	ret
.size __x86_memcmp,.-__x86_memcmp

/* -------------------------- */
/* memset and bzero           */
/* -------------------------- */

.align 16
.globl __x86_memset
.type __x86_memset,%function
__x86_memset:
	// If the value to set is zero, then skip forwards to perform
	// a regular bzero operation.
	test %esi, %esi
	jz 1f

	// Clear the direction flag, then jiggle the value to write
	// into EAX, and the number of bytes into RCX.
	cld
	mov %esi, %eax
	mov %rdx, %rcx

	// Launch into the bzero core.
	mov %rdi, %r8
	jmp 2f

.align 16
1:
	// Move the number of bytes to zero into RSI
	mov %rdx, %rsi

	// -- fallthrough -- //

.align 16
.globl __x86_bzero
.type __x86_bzero,%function
__x86_bzero:
	mov %rdi, %r8

	// Clear the direction flag
	cld

	// Move the number of bytes to zero into RCX, and divide by eight
	mov %rsi, %rcx
	shr $3, %rcx

	// Clear RAX, as this will contain the value to be written to
	// memory.
	xor %eax, %eax

	// Store as many 8-byte zeros as possible into memory
	rep stosq %rax, %es:(%rdi)

	// Prepare to write the remaining bytes
	mov %esi, %ecx
	and $7, %ecx

.align 16
2:
	// Store the remaining bytes to memory.  Note, that if we reached
	// this point from memset, then %al will contain the byte that we
	// want to store into memory.  If we came here from bzero, %al
	// will be zero.
	rep stosb %al, %es:(%rdi)

	// The return value is the destination pointer.
	mov %r8, %rax
	ret
.size __x86_memset,.-__x86_memset
.size __x86_bzero,.-__x86_bzero

/* -------------------------- */
/* pzero                      */
/* -------------------------- */

.align 16
.globl __x86_pzero
.type __x86_pzero,%function
__x86_pzero:
	mov %rdi, %r8

	// Clear the direction flag
	cld

	// Move the number of pages to zero into RCX, and multiply by 512,
	// which is the number of 64-bit words to clear.
	mov %rsi, %rcx
	shl $9, %rcx

	// Clear RAX, as this will contain the value to be written to
	// memory.
	xor %eax, %eax

	// Store as many 8-byte zeros as possible into memory
	rep stosq %rax, %es:(%rdi)

	mov %r8, %rax
	ret
.size __x86_pzero,.-__x86_pzero

/* -------------------------- */
/* strlen                     */
/* -------------------------- */

.align 16
.globl __x86_strlen
.type __x86_strlen,%function
__x86_strlen:
	cmpb $0, (%rdi)
	je 2f
	mov %rdi, %rax

.align 16
1:
	addq $1, %rax
	cmpb $0, (%rax)
	jne 1b
	subl %edi, %eax
	ret

.align 16
2:
	xorl    %eax, %eax
	ret
.size __x86_strlen,.-__x86_strlen

/* -------------------------- */
/* strncpy                    */
/* -------------------------- */

.align 16
.globl __x86_strncpy
.type __x86_strncpy,%function
__x86_strncpy:
	movzbl (%rsi), %r8d
	mov %rdi, %rax

	// Is length zero?
	test %rdx, %rdx
	je 3f

	// Is the source byte zero?
	test %r8b, %r8b
	je 3f

	// Let's copy!
	xor %ecx, %ecx
	jmp 2f

.align 16
1:
	// Check we haven't got a null byte.
	test %r8b, %r8b
	je 3f

.align 16
2:
	// Copy the source byte into the destination byte
	movb %r8b, (%rax, %rcx)

	// Increment pointer offset
	add $1, %rcx

	// Load next byte
	movzbl (%rsi, %rcx), %r8d

	// Check we haven't exceeded the length
	cmp %rcx, %rdx
	ja 1b

.align 16
3:
	ret
.size __x86_strncpy,.-__x86_strncpy


/* -------------------------- */
/* strcmp                     */
/* -------------------------- */

.align 16
.globl __x86_strcmp
.type __x86_strcmp,%function
__x86_strcmp:
        jmp     2f

1:
        cmpb    %al, (%rsi)
        jne     3f
        addq    $1, %rdi
        addq    $1, %rsi

2:
        movzbl  (%rdi), %eax
        testb   %al, %al
        jne     1b
        xorl    %eax, %eax

3:
        movzbl  (%rsi), %edx
        subl    %edx, %eax
        ret
.size __x86_strcmp,.-__x86_strcmp
