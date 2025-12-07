        // SPDX-License-Identifier: GPL-3.0-or-later

        .global _start

        .text
        .code16
_start:
        movw $0x0000, %ax
        movw %ax, %ds
        movw %ax, %es
        movw $0xf000, %ax
        movw %ax, %ss
        movw $0xfff0, %sp

        movw $0, %si
        movw %si, %dx

1:      movb hello(%si), %al
        outb %al, %dx
        incw %si
        incw %dx
        cmpw $len, %dx
        jne 1b

        cli
1:      hlt
        jmp 1b

        .section .rodata
hello:
        .ascii "Hello, world!\n"
        .set len, . - hello

        .section .text.init
        ljmp $0x0000, $_start
