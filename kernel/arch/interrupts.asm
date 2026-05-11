; Copyright (c) 2025-2026 Andrew (dreamyfx)
; MoonOS 1.0.0 x86_64 2026
; This software is released under the GNU Affero General Public License v3.0. See LICENSE file for details.
; This header should be maintained in any file it is present in, as per the AGPL license terms.
bits 64
section .text

extern isr_handler

%macro PUSH_REGS 0
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
%endmacro

%macro POP_REGS 0
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
    pop rax
%endmacro

%macro ISR_NOERR 1
isr_stub_%1:
    push qword 0
    push qword %1
    PUSH_REGS
    mov rdi, rsp
    call isr_handler
    POP_REGS
    add rsp, 16
    iretq
%endmacro

%macro ISR_ERR 1
isr_stub_%1:
    push qword %1
    PUSH_REGS
    mov rdi, rsp
    call isr_handler
    POP_REGS
    add rsp, 16
    iretq
%endmacro

ISR_NOERR 0
ISR_NOERR 1
ISR_NOERR 2
ISR_NOERR 3
ISR_NOERR 4
ISR_NOERR 5
ISR_NOERR 6
ISR_NOERR 7
ISR_ERR   8
ISR_NOERR 9
ISR_ERR   10
ISR_ERR   11
ISR_ERR   12
ISR_ERR   13
ISR_ERR   14
ISR_NOERR 15
ISR_NOERR 16
ISR_ERR   17
ISR_NOERR 18
ISR_NOERR 19
ISR_NOERR 20
ISR_NOERR 21
ISR_NOERR 22
ISR_NOERR 23
ISR_NOERR 24
ISR_NOERR 25
ISR_NOERR 26
ISR_NOERR 27
ISR_NOERR 28
ISR_NOERR 29
ISR_ERR   30
ISR_NOERR 31
ISR_NOERR 32
ISR_NOERR 33
ISR_NOERR 34
ISR_NOERR 35
ISR_NOERR 36
ISR_NOERR 37
ISR_NOERR 38
ISR_NOERR 39
ISR_NOERR 40
ISR_NOERR 41
ISR_NOERR 42
ISR_NOERR 43
ISR_NOERR 44
ISR_NOERR 45
ISR_NOERR 46
ISR_NOERR 47

section .data
global isr_stubs
isr_stubs:
%assign i 0
%rep 48
    dq isr_stub_%+i
%assign i i+1
%endrep
