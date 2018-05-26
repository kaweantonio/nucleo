#include <stdio.h>
#include <nucleo.h>

#define MAX 200

int buffer[MAX], prox_elem = 0;
semaforo cheia, vazia, mutex;
FILE *arq;

void far produtor() {
	int i;
	for(i = 0; i < 1000; i++) {
		imprime_fila_processos();
		P(&vazia);
		P(&mutex);
		buffer[prox_elem] = prox_elem;
		prox_elem = (prox_elem + 1) % MAX;
		V(&mutex);
		V(&cheia);
		fprintf(arq, "Produtor colocou %d\n", prox_elem - 1 < 0 ? MAX : prox_elem-1);
	}
	termina_processo();
}

void far consumidor() {
	int i, lido;
	for(i = 0; i < 1000; i++) {
		imprime_fila_processos();
		P(&cheia);
		P(&mutex);
		prox_elem--;
		prox_elem = prox_elem < 0 ? MAX : prox_elem;
		lido = buffer[prox_elem];
		buffer[prox_elem] = 9999;
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
}