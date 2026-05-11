; Copyright (c) 2025-2026 Andrew (dreamyfx)
; MoonOS 1.0.0 x86_64 2026
; This software is released under the GNU Affero General Public License v3.0. See LICENSE file for details.
; This header should be maintained in any file it is present in, as per the AGPL license terms.
bits 64
section .text

global ctx_switch
ctx_switch:
    push rbx
    push rbp
    push r12
    push r13
    push r14
    push r15
    mov [rdi], rsp
    mov rsp, [rsi]
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbp
    pop rbx
    ret

global jump_usermode
jump_usermode:
    mov ax, 0x1B
    mov ds, ax
    mov es, ax
    push qword 0x1B
    push rsi
    pushfq
    pop rax
    or rax, 0x200
    push rax
    push qword 0x23
    push rdi
    iretq
