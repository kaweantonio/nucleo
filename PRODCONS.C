#include <stdio.h>
#include <nucleo.h>

#define MAX 10

int buffer[MAX], prox_prod = 0, prox_cons = 0;
semaforo cheia, vazia, mutex;
FILE *arq;

void far produtor() {
	int i;
	for(i = 0; i < 25; i++) {
		P(&vazia);
		P(&mutex);
		buffer[prox_prod] = i;
		prox_prod = (prox_prod + 1) % MAX;
		V(&mutex);
		V(&cheia);
		fprintf(arq, "Produtor colocou %d\n", i);
	}
	termina_processo();
}

void far consumidor() {
	int i, lido;
	for(i = 0; i < 25; i++) {
		P(&cheia);
		P(&mutex);
		lido = buffer[prox_cons];
		prox_cons = (prox_cons + 1) % MAX;
		V(&mutex);
		V(&vazia);
		fprintf(arq, "Consumidor retirou %d\n", lido);
	}
	termina_processo();
}

void far main() {
	arq = fopen("outputProdCons.txt", "w");
	inicia_semaforo(&cheia, 0);
	inicia_semaforo(&vazia, MAX);
	inicia_semaforo(&mutex, 1);
	cria_processo(produtor, "prod");
	cria_processo(consumidor, "cons");
	dispara_sistema();
}
