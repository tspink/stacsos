/* SPDX-License-Identifier: MIT */

/* StACSOS - userspace standard library
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */

.text

.align 16
.globl _start
.type _start,%function
_start:
    push $0
    jmp start_main
.size _start,.-_start
