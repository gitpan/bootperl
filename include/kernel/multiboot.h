/* multiboot.h - the header for Multiboot */
/* taken from the documentation for GRUB and adapted for xos by bsmith */
/* Copyright (C) 1999, 2001  Free Software Foundation, Inc.
   
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.
   
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
   
   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

#if !defined(MULTIBOOT_H)
#define MULTIBOOT_H

/* The magic number for the Multiboot header.  */
#define MULTIBOOT_HEADER_MAGIC		0x1BADB002

/* The flags for the Multiboot header.  */
#define MULTIBOOT_HEADER_FLAGS		0x00000003

/* The magic number passed by a Multiboot-compliant boot loader.  */
#define MULTIBOOT_BOOTLOADER_MAGIC	0x2BADB002

#ifndef __ASSEMBLY__
/* The Multiboot header.  */
struct multiboot_header {
  unsigned long magic;
  unsigned long flags;
  unsigned long checksum;
  unsigned long header_addr;
  unsigned long load_addr;
  unsigned long load_end_addr;
  unsigned long bss_end_addr;
  unsigned long entry_addr;
};

/* The symbol table for a.out.  */
struct aout_symbol_table {
  unsigned long tabsize;
  unsigned long strsize;
  unsigned long addr;
  unsigned long reserved;
};

/* The section header table for ELF.  */
struct elf_section_header_table {
  unsigned long num;
  unsigned long size;
  unsigned long addr;
  unsigned long shndx;
};

/* The Multiboot information.  */
struct multiboot_info {
  unsigned long flags;
  unsigned long mem_lower; /* tick */
  unsigned long mem_upper; /* tick */
  unsigned long boot_device; /* tick */
  unsigned long cmdline; /* tick */
  unsigned long mods_count;
  unsigned long mods_addr;
  union {
    struct aout_symbol_table aout_sym;
    struct elf_section_header_table elf_sec; /* tick */
  } u;
  unsigned long mmap_length;
  unsigned long mmap_addr;
  unsigned long drives_length; /* tick */
  unsigned long drives_addr; /* tick */
  unsigned long config_table; /* tick */
  unsigned long boot_loader_name; /* tick */
  unsigned long apm_table;
  unsigned long vbe_control_info;
  unsigned long vbe_mode_info;
  unsigned long vbe_mode;
  unsigned long vbe_interface_seg;
  unsigned long vbe_interface_off;
  unsigned long vbe_interface_len;
};

/* The module structure.  */
struct multiboot_module {
  unsigned long mod_start;
  unsigned long mod_end;
  unsigned long string;
  unsigned long reserved;
};

/* The memory map. Be careful that the offset 0 is base_addr_low
   but no size.  */
struct memory_map {
  unsigned long size;
  unsigned long base_addr_low;
  unsigned long base_addr_high;
  unsigned long length_low;
  unsigned long length_high;
  unsigned long type;
};

#endif /* ! __ASMEMBLY__ */

#endif /* !defined(MULTIBOOT_H) */
