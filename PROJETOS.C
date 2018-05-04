#include <stdio.h>
#include <nucleo.h>

PTR_DESC_PROC p1, p2;

void far processo1(){
	while(1){
		printf("1");
	}
}

void far processo2(){
	while(1){
		printf("2");
	}
}

void far processo3(){
	while(1){
		printf("3");
	}
}

main(){
	cria_processo(processo1,"proc1");
	cria_processo(processo2,"proc2");
	cria_processo(processo3,"proc3");
	dispara_sistema();
}
