/* ld script to make a ELF kernel image
 * Written by Benjamin Smith
 */
OUTPUT_FORMAT("elf32-i386", "elf32-i386",
	      "elf32-i386")
OUTPUT_ARCH(i386)
ENTRY(_start)
SECTIONS
{
  . = 0x100000 + SIZEOF_HEADERS;
  _text = .;
  .text : {
        *(.text)
        *(.fixup)
        *(.gnu.warning)
        } = 0x9090
  _etext = .;

  .rodata : { *(.rodata) }
  .kstrtab : { *(.kstrtab) }

  .data : {
        *(.data)
        }
  _edata = .;

  . = ALIGN(4096);
  __init_begin = .;	/* init code and data */
  .text.init : { *(.text.init) }
  .data.init : { *(.data.init) }
  __init_end = .;

  . = ALIGN(4096);
  __bss_start = .;
  .bss : {
        *(.bss)
        }
  _end = . ;

  /DISCARD/ : {
        *(.text.exit)
        *(.data.exit)
        *(.exitcall.exit)
        }

  /* Stabs debugging sections.  */
  .stab 0 : { *(.stab) }
  .stabstr 0 : { *(.stabstr) }
  .stab.excl 0 : { *(.stab.excl) }
  .stab.exclstr 0 : { *(.stab.exclstr) }
  .stab.index 0 : { *(.stab.index) }
  .stab.indexstr 0 : { *(.stab.indexstr) }
  .comment 0 : { *(.comment) }
}
