

/*******************************************************************************
 *License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>*
 *This is free software: you are free to change and redistribute it.           *
 *******************************************************************************/
#ifndef _SYMBOL_H_
#define _SYMBOL_H_
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>
#include <elf.h>
#define SYMBOL_LOCAL ((uint16_t)(1<<STB_LOCAL))
#define SYMBOL_GLOBAL ((uint16_t)(1<<STB_GLOBAL))
#define SYMBOL_WEAK ((uint16_t)(1<<STB_WEAK))
#define SYMBOL_ALL (SYMBOL_LOCAL|SYMBOL_GLOBAL|SYMBOL_WEAK)
struct elf_stat {
	const void *data;
	size_t size;
	const Elf64_Shdr *shdr;
	uint16_t shnum;
	const char *shstrtab;
	size_t shstrtab_size;
};
int elf_read(struct elf_stat *restrict sp,const void *image,size_t size);
const Elf64_Shdr *elf_section(const struct elf_stat *restrict sp,const void *s_name);
uint64_t elf_symbol(const struct elf_stat *restrict sp,const char *symbol,const char *symtab,const char *strtab,uint16_t flag);
uint64_t elf_symbol2addr(const struct elf_stat *restrict sp,const char *symbol,uint16_t flag);
uint64_t elf_symbol2addr_raw(const void *image,size_t size,const char *symbol,uint16_t flag);
#endif
