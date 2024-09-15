/* SPDX-License-Identifier: MIT */

/* StACSOS Kernel - Core
 *
 * Copyright (C) University of St Andrews 2024.  All Rights Reserved.
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#define MULTIBOOT_HEADER_FLAGS  0x00000003
#define MULTIBOOT_HEADER_MAGIC  0x1BADB002

/* --- Multiboot Header --- */

.data
.section .multiboot.header, "a"

/* Multiboot Header Section, contains the magic number, a series of flags and the checksum. */
.align 16
.type multiboot_header, %object
multiboot_header:
    .long MULTIBOOT_HEADER_MAGIC
    .long MULTIBOOT_HEADER_FLAGS
    .long -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)
.size multiboot_header,.-multiboot_header

/* --- Multiboot entry point --- */

.code32
.section .multiboot.text, "ax"

.align 16
.globl multiboot_start
.type multiboot_start, %function
multiboot_start:
	// Disable interrupts.
	cli

	// Make sure the incoming magic number is correct.
	cmp $0x2BADB002, %eax
	jne 1f

	// Initialise the stack pointer at 1MB + 4096.
    mov $0x101000, %esp

    // Reset EFLAGS.
    pushl $0
    popf

	// EBX is live with the multiboot information structure.
	jmp start32

1:
	// We should probably try and display a panic message here,
	// but we're unlikely to ever see this.  More things have gone
	// wrong if we end up here.
	pause
	hlt
	jmp 1b
.size multiboot_start,.-multiboot_start
