/* SPDX-License-Identifier: MIT */

/* StACSOS Kernel
 *
 * Copyright (C) University of St Andrews 2024.  All Rights Reserved.
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
.text

/* Spin Lock Implemenentations */

/* --------------------------- */
.align 16

.globl spinlock_acquire
.type spinlock_acquire, %function
spinlock_acquire:
    lock btsl $0, (%rdi)
    jc 1f
    ret

1:
    pause
    testl $1, (%rdi)
    jnz 1b
    jmp spinlock_acquire
.size spinlock_acquire,.-spinlock_acquire

/* --------------------------- */
.align 16

.globl spinlock_release
.type spinlock_release, %function
spinlock_release:
    movl $0, (%rdi)
    ret
.size spinlock_release,.-spinlock_release

/* --------------------------- */
.align 16

.globl spinlock_irq_acquire
.type spinlock_irq_acquire, %function
spinlock_irq_acquire:
    pushf
    popq (%rsi)
    cli

1:
    lock btsl $0, (%rdi)
    jc 2f
    ret

2:
    pause
    testl $1, (%rdi)
    jnz 2b
    jmp 1b
.size spinlock_irq_acquire,.-spinlock_irq_acquire

/* --------------------------- */
.align 16

.globl spinlock_irq_release
.type spinlock_irq_release, %function
spinlock_irq_release:
    movl $0, (%rdi)
    testl $0x200, %esi
    jz 1f
    sti

1:
    ret
.size spinlock_irq_release,.-spinlock_irq_release
