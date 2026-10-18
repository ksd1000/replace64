
/*******************************************************************************
 *License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>*
 *This is free software: you are free to change and redistribute it.           *
 *******************************************************************************/
#define _GNU_SOURCE
#include "symbol.h"

#include <string.h>
#define ELF_MAGIC ((uint32_t)('\x7f'|('E'<<8)|('L'<<16)|('F'<<24)))

int elf_read(struct elf_stat *restrict sp,const void *image,size_t size){
	const char *endp;
	if(size<sizeof(Elf64_Ehdr)
			||*(const uint32_t *)image!=ELF_MAGIC){
		return -1;
	}
#define eh ((const Elf64_Ehdr *)image)
#define cimage ((const char *)image)
	if(eh->e_ident[EI_CLASS]!=ELFCLASS64||eh->e_shoff>=size)
		return -1;
	endp=cimage+size;
	sp->shdr=(const Elf64_Shdr *)(cimage+eh->e_shoff);
	if((const char *)sp->shdr>=endp)
		return -8;
	sp->shnum=eh->e_shnum;
	if((const char *)(sp->shdr+sp->shnum)>endp)
		return -2;
	if(eh->e_shstrndx>=sp->shnum)
		return -3;
	sp->shstrtab=cimage+sp->shdr[eh->e_shstrndx].sh_offset;
	sp->shstrtab_size=sp->shdr[eh->e_shstrndx].sh_size;
	if(sp->shstrtab+sp->shstrtab_size>endp)
		return -4;
	for(const Elf64_Shdr *shdr=sp->shdr,*shdr_end=sp->shdr+sp->shnum;shdr<shdr_end;++shdr){
		if(shdr->sh_type!=SHT_NOBITS&&cimage+shdr->sh_offset+shdr->sh_size>endp)
			return -5;
		if(shdr->sh_name>=sp->shstrtab_size)
			return -6;
		for(const char *p=sp->shstrtab+shdr->sh_name,*endp1=sp->shstrtab+sp->shstrtab_size;;){
			if(*p)
				break;
			++p;
			if(p>=endp1)
				return -7;
		}
	}
	sp->data=image;
	sp->size=size;
	return 0;
}
const Elf64_Shdr *elf_section(const struct elf_stat *restrict sp,const void *s_name){
	size_t len;
	len=strlen(s_name);
	for(const Elf64_Shdr *shdr=sp->shdr,*shdr_end=sp->shdr+sp->shnum;shdr<shdr_end;++shdr){
		if(shdr->sh_name+len>=sp->shstrtab_size)
			continue;
		if(!memcmp(s_name,sp->shstrtab+shdr->sh_name,len+1))
			return shdr;
	}
	return NULL;
}
uint64_t elf_symbol(const struct elf_stat *restrict sp,const char *symbol,const char *symtab,const char *strtab,uint16_t flag){
	size_t len,len1;
	const char *p;
	uint32_t off;
	const Elf64_Shdr *st,*sy;
	const Elf64_Sym *s,*s_end;
	sy=elf_section(sp,symtab);
	if(!sy)
		return 0;
	st=elf_section(sp,strtab);
	if(!st)
		return 0;
	len=strlen(symbol)+1;
	if(st->sh_size<len)
		return 0;
#define cdata ((const char *)sp->data)
	p=cdata+st->sh_offset;
	len1=st->sh_size;
	for(const char *p2=p,*p1;;){
		p1=memmem(p2,len1,symbol,len);
		if(!p1){
			return 0;
		}
		if(p1>p2&&p1[-1]!=0){
			++p1;
			len1-=p1-p2;
			p2=p1;
			continue;
		}
		off=p1-p;
		break;
	}
	s=(Elf64_Sym *)(cdata+sy->sh_offset);
	s_end=s+sy->sh_size/sizeof(Elf64_Sym);
	for(;s<s_end;++s){
		if(!((unsigned short)(1<<(s->st_info>>(unsigned char)4))&flag))
			continue;
		if(s->st_name==off){
			switch(s->st_shndx){
				case SHN_ABS:
				case SHN_COMMON:
				case SHN_UNDEF:
				case SHN_XINDEX:
					return 0;
				default:
//		__builtin_printf("at:%llx+%llx=%llx\n",(sp->shdr+s->st_shndx)->sh_offset,s->st_value,(sp->shdr+s->st_shndx)->sh_offset+s->st_value);
					return (sp->shdr+s->st_shndx)->sh_offset+s->st_value;
			}
		}
	}
	return 0;
}
uint64_t elf_symbol2addr(const struct elf_stat *restrict sp,const char *symbol,uint16_t flag){
	uint64_t r;
	r=elf_symbol(sp,symbol,".symtab",".strtab",flag);
	if(!r)
		r=elf_symbol(sp,symbol,".dynsym",".dynstr",flag);
	return r;
}

uint64_t elf_symbol2addr_raw(const void *image,size_t size,const char *symbol,uint16_t flag){
	struct elf_stat sp[1];
	if(elf_read(sp,image,size)<0)
		return 0;
	return elf_symbol2addr(sp,symbol,flag);
}
