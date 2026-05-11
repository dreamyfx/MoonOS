; Copyright (c) 2025-2026 Andrew (dreamyfx)
; MoonOS 1.0.0 x86_64 2026
; This software is released under the GNU Affero General Public License v3.0. See LICENSE file for details.
; This header should be maintained in any file it is present in, as per the AGPL license terms.
bits 64
section .text

extern syscall_dispatch
extern g_kernel_rsp

global syscall_entry
syscall_entry:
    swapgs
    mov [gs:16], rsp
    mov rsp, [gs:8]

    push qword 0x1B
    push qword [gs:16]
    push r11
    push qword 0x23
    push rcx

    push rax
    push rcx
    push rdx
    push rbx
    push rbp
    push rsi
    push rdi
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15

    mov rdi, rax
    mov rsi, rsp
    call syscall_dispatch

    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdi
    pop rsi
    pop rbp
    pop rbx
    pop rdx
    pop rcx

    add rsp, 8
    pop rcx
    add rsp, 8
    pop r11
    pop rsp
    add rsp, 8
    swapgs
    sysretq
