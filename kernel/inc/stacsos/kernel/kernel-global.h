/* SPDX-License-Identifier: MIT */

/* StACSOS - Kernel
 *
 * Copyright (c) University of St Andrews 2024
 * Tom Spink <tcs6@st-andrews.ac.uk>
 */
#pragma once

static inline void *phys_to_virt(unsigned long phys_addr)
{
    return (void *)(phys_addr + 0xffff'8000'0000'0000);
}
