#include <stdio.h>
#include <nucleo.h>

unsigned char far (*produz)();
void far (*consome)(unsigned char lido); 
void far (*retorno)();
int prox_prod = 0, prox_cons = 0;
long unsigned int max, n;
semaforo cheia, vazia, mutex;
unsigned char *buffer;

void far produtor() {
	long unsigned int i;
	for(i = 0; i < n; i++) {
		P(&vazia);
		P(&mutex);
		buffer[prox_prod] = produz();
		prox_prod = (prox_prod + 1) % max;
		V(&mutex);
		V(&cheia);
	}
	termina_processo();
}

void far consumidor() {
	long unsigned int i;
	for(i = 0; i < n; i++) {
		P(&cheia);
		P(&mutex);
		consome(buffer[prox_cons]);
		prox_cons = (prox_cons + 1) % max;
		V(&mutex);
		V(&vazia);
	}
	free(buffer);
	retorno();
	termina_processo();
}

void iniciar_PRODCONS(long unsigned int max_buffer, 
					  long unsigned int num_it, 
					  unsigned char far (*func_produz)(), 
					  void far (*func_consome)(unsigned char lido),
					  void far (*func_retorno)()) {
	max = max_buffer;
	n = num_it;
	buffer = (unsigned char *) calloc (max_buffer, 1);
	produz = func_produz;
	consome = func_consome;
	retorno = func_retorno;
	inicia_semaforo(&cheia, 0);
	inicia_semaforo(&vazia, max_buffer);
	inicia_semaforo(&mutex, 1);
	cria_processo(produtor, "prod");
	cria_processo(consumidor, "cons");
	dispara_sistema();
}

/* void far main() {
	config_PRODCONS(10, 25, produz_teste, consome_teste);
	arq = fopen("outputProdCons.txt", "w");
	inicia_semaforo(&cheia, 0);
	inicia_semaforo(&vazia, max_buffer);
	inicia_semaforo(&mutex, 1);
	cria_processo(produtor, "prod");
	cria_processo(consumidor, "cons");
	dispara_sistema();
} */
