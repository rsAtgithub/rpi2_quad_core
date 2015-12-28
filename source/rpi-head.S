//  Copyright (c) 2015-2016, Rushikesh Shingnapurkar
//  All rights reserved.
//
//  Redistribution and use in source and binary forms, with or without
//  modification, are permitted provided that the following conditions are met:
//
//  1. Redistributions of source code must retain the above copyright notice,
//      this list of conditions and the following disclaimer.
//
//  2. Redistributions in binary form must reproduce the above copyright notice,
//      this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
//  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
//  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
//  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
//  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
//  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
//  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//  POSSIBILITY OF SUCH DAMAGE.


.section ".text"

.global _get_cpu_id
.global _trigger_cpu_1
.global _trigger_cpu_2
.global _trigger_cpu_3


// Referred from
// https://github.com/PeterLemon/RaspberryPi/tree/master/SMP/SMPINIT
// unsigned int _get_cpu_id(void);
_get_cpu_id:
    mrc p15,0,r0,c0,c0,5
    and r0, #0x03
    mov pc, lr

// First code to be executed by secondary cores.
// void _trigger_cpu_1(void)
_trigger_cpu_1:
    // Setup stack pointer
    ldr     sp,=cpu1_stack
    // We have defined it to be an array and normally stack grows top to bottom.
    // So, add the size of array here.
    add     sp,#1000
    // Get address of function which will decide what's next for this core.
    ldr     r0,=__start_core_1
    // Jump
    mov     pc, r0


// void _trigger_cpu_2(void)
_trigger_cpu_2:
    ldr     sp,=cpu2_stack
    add     sp,#1000
    ldr     r0,=fill_task
    mov     pc, r0

// void _trigger_cpu_3(void)
_trigger_cpu_3:
    ldr     sp,=cpu3_stack
    add     sp,#1000
    ldr     r0,=fill_task
    mov     pc, r0


