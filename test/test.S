        // SPDX-License-Identifier: GPL-3.0-or-later

        .global _start

        .text
        .code16
_start:
        movw $0x0000, %ax
        movw %ax, %ds
        movw %ax, %es
        movw $0x7000, %ax
        movw %ax, %ss
        movw $0x0000, %sp

        movw $8, %di
        call fibonacci

        cli
1:      hlt
        jmp 1b

fibonacci:
        movw $0, %ax
        movw $1, %dx
        cmpw $0, %di
        jz 2f
1:      addw %dx, %ax
        xchgw %dx, %ax
        decw %di
        jnz 1b      
2:      ret

        .section .text.init
        ljmp $0x0000, $_start
