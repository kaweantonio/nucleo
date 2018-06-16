#include "C:\NBPS\nucleo.h"
#include <time.h>
#include <limits.h>

void far processo(){
	int i = 0, j = 0; 

	/* Variáveis para armazenar dados referentes ao processo. */
	char nome_processo[35], numero_processo;
	unsigned int prioridade; 

	/* Variável usadas para calcular quantidade de clocks usadas no programa. */
	clock_t tempo_execucao;

	/* Registrar início da execução do processo. */
	tempo_execucao = clock();

	/* Buscar informações do processo atual. */
	informacoes_processo(nome_processo, &prioridade);

	/* 	Pegar o número desse processo.
		O número é o primeiro caracter do nome do processo. */
	numero_processo = nome_processo[0];

	for(i = 0; i < 50; i++) {
		while(++j < INT_MAX);
		j = 0;
	}

	/* Registrar término da execução do processo. */
	tempo_execucao = clock() - tempo_execucao;

	printf("Processo %c, com prioridade %u, levou %d clocks para terminar.\n\n", numero_processo, prioridade, tempo_execucao);
	termina_processo();
}

main(){
	cria_processo(processo,"1proc", 4);
	cria_processo(processo,"2proc", 3);
	cria_processo(processo,"3proc", 2);
	cria_processo(processo,"4proc", 1);
	dispara_sistema();
}
