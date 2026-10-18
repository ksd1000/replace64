#include <stdio.h>
static char AA[89000]={'h','a','h','a'};
static char license[35150]={'h','a','h','a'};
static char BB[64000]={'h','a','h','a'};
//__atttibute__((section(".data")))
int main(void){
	printf("%s\n",license);
	printf("%p,%p\n",AA,BB);
	return 0;
}
