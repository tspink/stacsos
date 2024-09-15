/* SPDX-License-Identifier: MIT */

/* StACSOS Kernel - Core
 *
 * Copyright (C) University of St Andrews 2024.  All Rights Reserved.
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
.code64
.section .startup.text64, "ax"

.align 16

.globl start64
.type start64, %function
start64:
	// OK - we're nearly there now!  We've made it to 64-bit mode, so get the
	// final things ready before we go into our main C++ code.

	// Re-initialise segment registers
	mov $0x10, %eax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %ss

	xor %eax, %eax
	mov %ax, %fs
	mov %ax, %gs

	// This is where physical memory is directly mapped 1-1 in the virtual address space.
	mov $0xffff800000000000, %rcx

	// Re-load the temporary kernel stack address, but in the upper address space.
	mov $101000, %rsp
	add %rcx, %rsp

	// EDI (and by transitivity RDI) is live with a pointer to the multiboot information
	// structure, but we need to pop it into the upper address space.
	add %rcx, %rdi

	// Jump to the x86 initialisation entrypoint in start.cpp
	jmp x86_start
.size start64,.-start64
