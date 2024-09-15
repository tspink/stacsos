/* SPDX-License-Identifier: MIT */

/* StACSOS Kernel - Core
 *
 * Copyright (C) University of St Andrews 2024.  All Rights Reserved.
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
.code32
.section .startup.text32, "ax"

.align 16

#define SCRATCH_MEMORY  0x100000
#define STACK_TOP       (SCRATCH_MEMORY + 0x1000)
#define PML4            (SCRATCH_MEMORY + 0x1000)
#define PDP_LO          (SCRATCH_MEMORY + 0x2000)
#define PDP_MID         (SCRATCH_MEMORY + 0x3000)
#define PDP_HI          (SCRATCH_MEMORY + 0x4000)

/* CR0 */
#define CR0_PE  (1u << 0)
#define CR0_MP  (1u << 1)
#define CR0_EM  (1u << 2)
#define CR0_TS  (1u << 3)
#define CR0_ET  (1u << 4)
#define CR0_NE  (1u << 5)
#define CR0_WP  (1u << 16)
#define CR0_AM  (1u << 18)
#define CR0_NW  (1u << 29)
#define CR0_CD  (1u << 30)
#define CR0_PG  (1u << 31)

/* CR4 */
#define CR4_VME      (1u << 0)
#define CR4_PVI      (1u << 1)
#define CR4_TSD      (1u << 2)
#define CR4_DE       (1u << 3)
#define CR4_PSE      (1u << 4)
#define CR4_PAE      (1u << 5)
#define CR4_MCE      (1u << 6)
#define CR4_PGE      (1u << 7)
#define CR4_PCE      (1u << 8)
#define CR4_OSFXSR   (1u << 9)
#define CR4_OSXMMEXCPT (1u << 10)
#define CR4_UMIP     (1u << 11)
#define CR4_LA57     (1u << 12)
#define CR4_VMXE     (1u << 13)
#define CR4_SMXE     (1u << 14)
#define CR4_FSGSBASE (1u << 16)
#define CR4_PCIDE    (1u << 17)
#define CR4_OSXSAVE  (1u << 18)
#define CR4_SMEP     (1u << 20)
#define CR4_SMAP     (1u << 21)
#define CR4_PKE      (1u << 22)

/* EFER */
#define EFER_SCE    (1u << 0)
#define EFER_DPE    (1u << 1)
#define EFER_SEWBAD (1u << 2)
#define EFER_GEWBAD (1u << 3)
#define EFER_L2D    (1u << 4)
#define EFER_LME    (1u << 8)
#define EFER_LMA    (1u << 10)
#define EFER_NXE    (1u << 11)
#define EFER_SVME   (1u << 12)
#define EFER_LMSLE  (1u << 13)
#define EFER_FFXSR  (1u << 14)
#define EFER_TCE    (1u << 15)

/**
 * 32-bit kernel entry point
 */
.globl start32
.type start32, %function
start32:
    // The stack is VALID on entry to this function, and EBX
    // contains the multiboot information structure.

    // Save the multiboot information structure
    push %ebx

    // Disable the PIC
    mov $0xff, %al
    out %al, $0xa1
    out %al, $0x21

    // *** Prepare the initial page tables *** //

    // Prepare the PML4
    movl $(PML4), %ebx

    // Clear three consecutive memory pages: PML4, PDP_LO, and PDP_HI
    movl $4, %ecx
    call zero_pages32

    // Update cr3 with the address of the PML4
    mov %ebx, %cr3

    // EBX holds the address of the PML4, so write the address of the
    // PDP_LO (with PRESENT + W flags) into the first slot.
    movl $(PDP_LO | 3), 0x000(%ebx)

    // Write the address of PDP_MID into the slot just above the break.
    movl $(PDP_MID | 3), 0x800(%ebx)

    // Write the address of PDP_HI into the last slot.
    movl $(PDP_HI | 3), 0xff8(%ebx)

    // Now, configure the PDPs.

    // First, check that 1G mappings are available on our CPU, and error
    // out if not.  TODO: Try 2M mappings.
    mov $0x80000001, %eax
    cpuid
    bt $26, %edx
    jnc no_1g

    // PDP_LO
    movl $(PDP_LO), %ebx            // Address of PDP_LO into EBX
    movl $0x00000083, 0x000(%ebx)	// 1G mapping:  0 ->  0
    movl $0x40000083, 0x008(%ebx)	// 1G mapping: 1G -> 1G

    // PDP_MID
    movl $(PDP_MID), %ebx            // Address of PDP_MID into EBX
    movl $0x00000083, 0x000(%ebx)	// 1G mapping: 0xffff800000000000 -> 0
    movl $0x40000083, 0x008(%ebx)	// 1G mapping: 0xffff800040000000 -> 1G

    // PDP_HI
    movl $(PDP_HI), %ebx            // Address of PDP_HI into EBX
    movl $0x00000083, 0xff0(%ebx)	// 1G mapping: 0xffffffff80000000 -> 0
    movl $0x40000083, 0xff8(%ebx)	// 1G mapping: 0xffffffffc0000000 -> 1G

    // *** Initialise control registers *** //

    // Check for FSGSBASE
    mov $7, %eax
    xor %ecx, %ecx
    cpuid
    bt $0, %ebx

    mov $(CR4_PSE | CR4_PAE | CR4_PGE | CR4_OSFXSR | CR4_OSXMMEXCPT | CR4_FSGSBASE), %eax
    jc has_fsgs

    and $(~CR4_FSGSBASE), %eax

has_fsgs:

    // Initialise CR4
    mov %eax, %cr4

    // Initialise EFER
    mov $(0xC0000080), %ecx
    xor %edx, %edx
    mov $(EFER_SCE | EFER_LME | EFER_NXE), %eax
    wrmsr

    // Initialise CR0
    mov $(CR0_PG | CR0_PE | CR0_MP | CR0_WP | CR0_NE), %eax
    mov %eax, %cr0

    // Restore the pointer to the multiboot information structure.  This
    // goes into EDI because of the calling convention we're about to
    // start using.
    pop %edi

    // Load a null IDT
    lidt idtp

    // Load the temporary GDT
    lgdt gdpt

    // Long jump to 64-bit code
    ljmp $0x8, $start64

no_1g:
    hlt
    jmp no_1g
.size start32,.-start32

/**
 * Quickly zeroes out a number of 4k pages.
 * @ebx: Base address of page.
 * @ecx: Number of pages.
 *
 * Clobbers: %edi, %eax, %ecx & flags
 */
.align 16
.type zero_pages32, %function
zero_pages32:
	mov %ebx, %edi
	xor %eax, %eax
	cld
	shl $10, %ecx
	repz stosl
	ret
.size zero_pages32,. - zero_pages32

.section .startup.data, "aw"

/* Null IDT pointer */
.align 4
.type idtp, %object
idtp:
    .word 0
    .long 0
.size idtp,.-idtp

/* Temporary GDT */
.align 4
.type gdt, %object
gdt:
    .quad 0x0000000000000000		// NULL

    // 64-bit Code Segment @ 0x08
    .quad 0x00209A0000000000

    // Data Segment @ 0x10
    .quad 0x0000920000000000
gdt_end:
.size gdt,.-gdt

/* Temporary GDT pointer */
.align 4
.type gdpt, %object
gdpt:
    .word (gdt_end - gdt)
    .long gdt
.size gdpt,.-gdpt
