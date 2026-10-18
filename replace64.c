/*******************************************************************************
 *License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>*
 *This is free software: you are free to change and redistribute it.           *
 *******************************************************************************/
#define _GNU_SOURCE
#include "symbol.h"
#include <unistd.h>
#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/mman.h>
struct fmap {
	void *data;
	size_t size;
	int fd,prot;
};
int fmap(struct fmap *restrict fp,const char *path,int prot){
	struct stat statbuf[1];
	fp->fd=open(path,(prot&PROT_WRITE)?O_RDWR:O_RDONLY);
	if(fp->fd<0)
		return -1;
	if(fstat(fp->fd,statbuf)<0)
		goto err0;
	fp->data=mmap(NULL,statbuf->st_size,prot,MAP_SHARED,fp->fd,0);
	if(fp->data==MAP_FAILED)
		goto err0;
	fp->size=statbuf->st_size;
	fp->prot=prot;
	return 0;
err0:
	close(fp->fd);
	return -1;
}
void funmap(struct fmap *restrict fp){
	munmap(fp->data,fp->size);
	close(fp->fd);
}
int replace(const char *elf,const char *res,const char *symbol){
	struct fmap melf[1],mres[1];
	uint64_t off;
	if(fmap(melf,elf,PROT_READ|PROT_WRITE)<0){
		warn("cannot open %s",elf);
		return -1;
	}
	off=elf_symbol2addr_raw(melf->data,melf->size,symbol,SYMBOL_ALL);
	if(!off){
		warnx("cannot find symbol \"%s\"",symbol);
		goto err0;
	}
	if(fmap(mres,res,PROT_READ)<0){
		warn("cannot open %s",elf);
		goto err0;
	}
	if(off+mres->size>melf->size){
		warnx("%s is too large",res);
		goto err1;
	}
	memcpy((char *)melf->data+off,mres->data,mres->size);
//	memset(melf->data,0,melf->size);
//	msync(melf->data,melf->size,MS_SYNC);
	funmap(mres);
	funmap(melf);
	return 0;
err1:
	funmap(mres);
err0:
	funmap(melf);
	return -1;
}
int main(int argc,char **argv){
	if(argc<4){
		fprintf(stdout,"usage: %s elf_file symbol infile\n",*argv);
		return EXIT_FAILURE;
	}
	return replace(argv[1],argv[3],argv[2])<0?EXIT_FAILURE:EXIT_SUCCESS;
}
