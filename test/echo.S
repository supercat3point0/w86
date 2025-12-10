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

        movw $0, %dx

1:      inb %dx, %al
        cmpb $0, %al
        jz 1f
        outb %al, %dx
        incw %dx
        jmp 1b
1:

        cli
1:      hlt
        jmp 1b

        .section .text.init
        ljmp $0x0000, $_start
