; Copyright (c) 2025-2026 Andrew (dreamyfx)
; MoonOS 1.0.0 x86_64 2026
; This software is released under the GNU Affero General Public License v3.0. See LICENSE file for details.
; This header should be maintained in any file it is present in, as per the AGPL license terms.
bits 64
section .text

global gdt_flush
gdt_flush:
    lgdt [rdi]
    push 0x08
    lea rax, [rel .reload]
    push rax
    retfq
.reload:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    ret

global tss_flush
tss_flush:
    mov ax, 0x28
    ltr ax
    ret
