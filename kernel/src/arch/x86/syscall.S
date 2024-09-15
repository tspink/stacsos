/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
.align 16

.globl syscall_entry
.type syscall_entry,%function
syscall_entry:
    // Activate KERNEL STACK
    swapgs
    mov %rsp, %gs:0x20
    mov %gs:0x18, %rsp

    // Save user context - don't need to save RAX and RDX, as they are return values.
	push %rcx
	push %rbx
	push %rbp
	push %rsi
	push %rdi
	push %r8
	push %r9
	push %r10
	push %r11
	push %r12
	push %r13
	push %r14
	push %r15

	xor %rbp, %rbp

	// Now, we can't use RCX and R11 for arguments, because RCX is the return address, and R11 are saved flags.
	// But, we've pushed them to the stack, so we can twiddle some values.

	// Syscall Arg 4 is in r8 -> ABI Arg 5 in r8
	mov %rdx, %rcx		// Syscall Arg 3 -> ABI Arg 4
	mov %rsi, %rdx		// Syscall Arg 2 -> ABI Arg 3
	mov %rdi, %rsi		// Syscall Arg 1 -> ABI Arg 2
	mov %rax, %rdi		// Syscall Num -> ABI Arg 1

	// RDI, RSI, RDX, RCX, R8, R9 should now be live with syscall arguments (as appropriate)
    call handle_syscall

    // Restore user context (remember to skip RAX and RDX)
	pop %r15
	pop %r14
	pop %r13
	pop %r12
	pop %r11	// This happens to contain the return flags
	pop %r10
	pop %r9
	pop %r8
	pop %rdi
	pop %rsi
	pop %rbp
	pop %rbx
	pop %rcx	// This happens to contain the return IP

    // Restore USER STACK
    mov %gs:0x20, %rsp
    swapgs

    sysretq
.size syscall_entry,.-syscall_entry
