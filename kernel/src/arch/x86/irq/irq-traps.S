/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */

.macro PUSH_STATE
	push %rax
	push %rcx
	push %rdx
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

#ifdef USE_FSGSBASE
	rdfsbase %rax
	push %rax

	rdgsbase %rax
	push %rax
#else
	// FSBASE
	movl $0xc0000100, %ecx
	rdmsr
	shl $32, %rdx
	or %rdx, %rax

	push %rax

	// GSBASE
	movl $0xc0000101, %ecx
	rdmsr
	shl $32, %rdx
	or %rdx, %rax

	push %rax
#endif
.endm

.macro POP_STATE
#ifdef USE_FSGSBASE
	pop %rax
	wrgsbase %rax

	pop %rax
	wrfsbase %rax
#else
	// GSBASE
	movl $0xc0000101, %ecx

	pop %rdx
	mov %edx, %eax
	shr $32, %rdx
	wrmsr

	// FSBASE
	movl $0xc0000100, %ecx

	pop %rdx
	mov %edx, %eax
	shr $32, %rdx
	wrmsr
#endif

	pop %r15
	pop %r14
	pop %r13
	pop %r12
	pop %r11
	pop %r10
	pop %r9
	pop %r8
	pop %rdi
	pop %rsi
	pop %rbp
	pop %rbx
	pop %rdx
	pop %rcx
	pop %rax
.endm

.macro TRAP_BEGIN,has_arg
.if ! \has_arg
	push $0
.endif
	PUSH_STATE

	cmpw $0x08, 152(%rsp)
	je 1f
	swapgs
1:

	// Store the stack pointer (which is holding the saved context) into GS:8
	// GS holds the current TCB, and GS:8 is a pointer to the saved context.
	mov %rsp, %gs:8
.endm

.macro TRAP_COMPLETE
	// Prepare to return from interrupt
	mov %gs:8, %rsp

	cmpw $0x08, 152(%rsp)
	je 1f
	swapgs
1:

	POP_STATE
	add $8, %rsp
	iretq
.endm

.text
.align 16
.globl x86_return_to_task
.type x86_return_to_task,%function
x86_return_to_task:
	TRAP_COMPLETE
.size x86_return_to_task,.-x86_return_to_task

.macro IRQ_TRAP_PRE,nr,has_arg
.text
.align 16
.globl x86_irq_trap_\nr
.type x86_irq_trap_\nr, %function
x86_irq_trap_\nr:
	TRAP_BEGIN \has_arg
	mov $\nr, %edi
    mov %rsp, %rsi
.endm

.macro IRQ_TRAP_POST,nr
	TRAP_COMPLETE
.size x86_irq_trap_\nr,.-x86_irq_trap_\nr
.endm

.macro IRQ_TRAP,nr,has_arg
IRQ_TRAP_PRE \nr \has_arg
	call x86_handle_irq
IRQ_TRAP_POST \nr
.endm

IRQ_TRAP 0  0
IRQ_TRAP 1  0
IRQ_TRAP 2  0
IRQ_TRAP 3  0
IRQ_TRAP 4  0
IRQ_TRAP 5  0
IRQ_TRAP 6  0
IRQ_TRAP 7  0
IRQ_TRAP 8  1
IRQ_TRAP 9  0
IRQ_TRAP 10 1
IRQ_TRAP 11 1
IRQ_TRAP 12 1
IRQ_TRAP 13 1
IRQ_TRAP 14 1
IRQ_TRAP 15 0
IRQ_TRAP 16 0
IRQ_TRAP 17 1
IRQ_TRAP 18 0
IRQ_TRAP 19 0
IRQ_TRAP 20 0
IRQ_TRAP 21 0
IRQ_TRAP 22 0
IRQ_TRAP 23 0
IRQ_TRAP 24 0
IRQ_TRAP 25 0
IRQ_TRAP 26 0
IRQ_TRAP 27 0
IRQ_TRAP 28 0
IRQ_TRAP 29 0
IRQ_TRAP 30 1
IRQ_TRAP 31 0

IRQ_TRAP 32 0
IRQ_TRAP 33 0
IRQ_TRAP 34 0
IRQ_TRAP 35 0
IRQ_TRAP 36 0
IRQ_TRAP 37 0
IRQ_TRAP 38 0
IRQ_TRAP 39 0
IRQ_TRAP 40 0
IRQ_TRAP 41 0
IRQ_TRAP 42 0
IRQ_TRAP 43 0
IRQ_TRAP 44 0
IRQ_TRAP 45 0
IRQ_TRAP 46 0
IRQ_TRAP 47 0
IRQ_TRAP 48 0
IRQ_TRAP 49 0
IRQ_TRAP 50 0
IRQ_TRAP 51 0
IRQ_TRAP 52 0
IRQ_TRAP 53 0
IRQ_TRAP 54 0
IRQ_TRAP 55 0
IRQ_TRAP 56 0
IRQ_TRAP 57 0
IRQ_TRAP 58 0
IRQ_TRAP 59 0
IRQ_TRAP 60 0
IRQ_TRAP 61 0
IRQ_TRAP 62 0
IRQ_TRAP 63 0
IRQ_TRAP 64 0
IRQ_TRAP 65 0
IRQ_TRAP 66 0
IRQ_TRAP 67 0
IRQ_TRAP 68 0
IRQ_TRAP 69 0
IRQ_TRAP 70 0
IRQ_TRAP 71 0
IRQ_TRAP 72 0
IRQ_TRAP 73 0
IRQ_TRAP 74 0
IRQ_TRAP 75 0
IRQ_TRAP 76 0
IRQ_TRAP 77 0
IRQ_TRAP 78 0
IRQ_TRAP 79 0
IRQ_TRAP 80 0
IRQ_TRAP 81 0
IRQ_TRAP 82 0
IRQ_TRAP 83 0
IRQ_TRAP 84 0
IRQ_TRAP 85 0
IRQ_TRAP 86 0
IRQ_TRAP 87 0
IRQ_TRAP 88 0
IRQ_TRAP 89 0
IRQ_TRAP 90 0
IRQ_TRAP 91 0
IRQ_TRAP 92 0
IRQ_TRAP 93 0
IRQ_TRAP 94 0
IRQ_TRAP 95 0
IRQ_TRAP 96 0
IRQ_TRAP 97 0
IRQ_TRAP 98 0
IRQ_TRAP 99 0
IRQ_TRAP 100 0
IRQ_TRAP 101 0
IRQ_TRAP 102 0
IRQ_TRAP 103 0
IRQ_TRAP 104 0
IRQ_TRAP 105 0
IRQ_TRAP 106 0
IRQ_TRAP 107 0
IRQ_TRAP 108 0
IRQ_TRAP 109 0
IRQ_TRAP 110 0
IRQ_TRAP 111 0
IRQ_TRAP 112 0
IRQ_TRAP 113 0
IRQ_TRAP 114 0
IRQ_TRAP 115 0
IRQ_TRAP 116 0
IRQ_TRAP 117 0
IRQ_TRAP 118 0
IRQ_TRAP 119 0
IRQ_TRAP 120 0
IRQ_TRAP 121 0
IRQ_TRAP 122 0
IRQ_TRAP 123 0
IRQ_TRAP 124 0
IRQ_TRAP 125 0
IRQ_TRAP 126 0
IRQ_TRAP 127 0
IRQ_TRAP 128 0
IRQ_TRAP 129 0
IRQ_TRAP 130 0
IRQ_TRAP 131 0
IRQ_TRAP 132 0
IRQ_TRAP 133 0
IRQ_TRAP 134 0
IRQ_TRAP 135 0
IRQ_TRAP 136 0
IRQ_TRAP 137 0
IRQ_TRAP 138 0
IRQ_TRAP 139 0
IRQ_TRAP 140 0
IRQ_TRAP 141 0
IRQ_TRAP 142 0
IRQ_TRAP 143 0
IRQ_TRAP 144 0
IRQ_TRAP 145 0
IRQ_TRAP 146 0
IRQ_TRAP 147 0
IRQ_TRAP 148 0
IRQ_TRAP 149 0
IRQ_TRAP 150 0
IRQ_TRAP 151 0
IRQ_TRAP 152 0
IRQ_TRAP 153 0
IRQ_TRAP 154 0
IRQ_TRAP 155 0
IRQ_TRAP 156 0
IRQ_TRAP 157 0
IRQ_TRAP 158 0
IRQ_TRAP 159 0
IRQ_TRAP 160 0
IRQ_TRAP 161 0
IRQ_TRAP 162 0
IRQ_TRAP 163 0
IRQ_TRAP 164 0
IRQ_TRAP 165 0
IRQ_TRAP 166 0
IRQ_TRAP 167 0
IRQ_TRAP 168 0
IRQ_TRAP 169 0
IRQ_TRAP 170 0
IRQ_TRAP 171 0
IRQ_TRAP 172 0
IRQ_TRAP 173 0
IRQ_TRAP 174 0
IRQ_TRAP 175 0
IRQ_TRAP 176 0
IRQ_TRAP 177 0
IRQ_TRAP 178 0
IRQ_TRAP 179 0
IRQ_TRAP 180 0
IRQ_TRAP 181 0
IRQ_TRAP 182 0
IRQ_TRAP 183 0
IRQ_TRAP 184 0
IRQ_TRAP 185 0
IRQ_TRAP 186 0
IRQ_TRAP 187 0
IRQ_TRAP 188 0
IRQ_TRAP 189 0
IRQ_TRAP 190 0
IRQ_TRAP 191 0
IRQ_TRAP 192 0
IRQ_TRAP 193 0
IRQ_TRAP 194 0
IRQ_TRAP 195 0
IRQ_TRAP 196 0
IRQ_TRAP 197 0
IRQ_TRAP 198 0
IRQ_TRAP 199 0
IRQ_TRAP 200 0
IRQ_TRAP 201 0
IRQ_TRAP 202 0
IRQ_TRAP 203 0
IRQ_TRAP 204 0
IRQ_TRAP 205 0
IRQ_TRAP 206 0
IRQ_TRAP 207 0
IRQ_TRAP 208 0
IRQ_TRAP 209 0
IRQ_TRAP 210 0
IRQ_TRAP 211 0
IRQ_TRAP 212 0
IRQ_TRAP 213 0
IRQ_TRAP 214 0
IRQ_TRAP 215 0
IRQ_TRAP 216 0
IRQ_TRAP 217 0
IRQ_TRAP 218 0
IRQ_TRAP 219 0
IRQ_TRAP 220 0
IRQ_TRAP 221 0
IRQ_TRAP 222 0
IRQ_TRAP 223 0
IRQ_TRAP 224 0
IRQ_TRAP 225 0
IRQ_TRAP 226 0
IRQ_TRAP 227 0
IRQ_TRAP 228 0
IRQ_TRAP 229 0
IRQ_TRAP 230 0
IRQ_TRAP 231 0
IRQ_TRAP 232 0
IRQ_TRAP 233 0
IRQ_TRAP 234 0
IRQ_TRAP 235 0
IRQ_TRAP 236 0
IRQ_TRAP 237 0
IRQ_TRAP 238 0
IRQ_TRAP 239 0
IRQ_TRAP 240 0
IRQ_TRAP 241 0
IRQ_TRAP 242 0
IRQ_TRAP 243 0
IRQ_TRAP 244 0
IRQ_TRAP 245 0
IRQ_TRAP 246 0
IRQ_TRAP 247 0
IRQ_TRAP 248 0
IRQ_TRAP 249 0
IRQ_TRAP 250 0
IRQ_TRAP 251 0
IRQ_TRAP 252 0
IRQ_TRAP 253 0
IRQ_TRAP 254 0
IRQ_TRAP 255 0

.align 16
.globl reload_cs
.type reload_cs,%function
reload_cs:
    // Get hold of the return RIP
    pop %rax

    // Remember the stack pointer of the caller
    mov %rsp, %rcx

    pushq $0x10     // SS
    push %rcx       // RSP
    pushf           // RFLAGS
    pushq $0x08     // CS
    push %rax       // RIP

    // Return from interrupt
    iretq
.size reload_cs,.-reload_cs

.align 16
.globl x86_call_isr
.type x86_call_isr,%function
x86_call_isr:
	push %rbp
	mov %rsp, %rbp

	// Align stack pointer
	and	$~0xf, %rsp

	pushq $0x10		// KERNEL_SS
	pushq %rbp		// RSP
	pushfq			// RFLAGS
	pushq $0x08		// KERNEL_CS
	cli
	callq *%rdi

	pop %rbp
	ret
.size x86_call_isr,.-x86_call_isr
