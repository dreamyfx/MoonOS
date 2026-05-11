; Copyright (c) 2025-2026 Andrew (dreamyfx)
; MoonOS 1.0.0 x86_64 2026
; This software is released under the GNU Affero General Public License v3.0. See LICENSE file for details.
; This header should be maintained in any file it is present in, as per the AGPL license terms.
bits 64
section .text

extern kmain

global _start
_start:
    cli
    lea rsp, [stack_top]
    xor rbp, rbp
    call kmain
.halt:
    cli
    hlt
    jmp .halt

section .bss
align 16
stack_bottom:
    resb 65536
stack_top:
