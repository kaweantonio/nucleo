#include <stdio.h>
#include <nucleo.h>

PTR_DESC_PROC p1, p2;

void far processo1(){
	int i = 0;
	while(i < 10000){
		printf("Processo 1 ");
		i++;
	}
	termina_processo();
}

void far processo2(){
	int i = 0;
	while(i < 10000){
		printf("Processo 2 ");
		i++;
	}
	termina_processo();
}

void far processo3(){
	int i = 0;
	while(i < 10000){
		printf("Processo 3 ");
		i++;
	}
	termina_processo();
}

main(){
	cria_processo(processo1,"proc1");
	cria_processo(processo2,"proc2");
	cria_processo(processo3,"proc3");
	dispara_sistema();
}
