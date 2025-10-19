/* SPDX-License-Identifier: GPL-3.0-or-later */

ENTRY(_start);
OUTPUT_FORMAT(binary);

SECTIONS {
  . = 0x00000;

  .text : {
    FILL(0x00);

    . = 0x00400;
    *(.text);
    *(.rodata);
    *(.data);
    *(COMMON);
    *(.bss);
    . = 0xffff0;
    *(.text.init);
  }

  /DISCARD/ : {
    *(.note.gnu.property);
  }
}
