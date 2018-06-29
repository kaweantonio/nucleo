#include "C:\NBPS\nucleo.h"
#include <limits.h>

void far processo(){
	int i = 0, j = 0; 

	/* Variáveis para armazenar dados referentes ao processo. */
	char nome_processo[35], numero_processo;
	unsigned int prioridade; 

	/* Buscar informações do processo atual. */
	informacoes_processo(nome_processo, &prioridade);

	/* 	Pegar o número desse processo.
		O número é o primeiro caracter do nome do processo. */
	numero_processo = nome_processo[0];

	/* Contar de zero a INT_MAX várias vezes */
	for(i = 0; i < 50; i++) {
		while(++j < INT_MAX);
		j = 0;
	}

	/* Emitir aviso de que o processo terminou sua execução. */
	printf("Processo %c, com prioridade %u, terminou.\n\n", numero_processo, prioridade + 1);
	termina_processo();
}

main(){
	cria_processo(processo,"9proc", 1);
	cria_processo(processo,"8proc", 1);
	cria_processo(processo,"7proc", 1);
	cria_processo(processo,"6proc", 2);
	cria_processo(processo,"5proc", 4);
	cria_processo(processo,"4proc", 4);
	cria_processo(processo,"3proc", 3);
	cria_processo(processo,"2proc", 2);
	cria_processo(processo,"1proc", 1);
	dispara_sistema();
}
