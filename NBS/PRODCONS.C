#include "C:\NBS\prodcons.h"

/* 	Declaração das funções que serão chamadas dentro do produtor/consumidor
	e que terão valor atribuído através do parâmetro passado ao void iniciar_PRODCONS */
unsigned char far (*produz)();
void far (*consome)(unsigned char lido); 
void far (*retorno)();

/* Iteradores do produtor e consumidor */
int prox_prod = 0, prox_cons = 0;

/*  max = tamanho máximo do buffer
	n = número de iterações a serem executadas */
long unsigned int max, n;

/*  Declaração dos semáforos.
	cheia = células cheias do buffer. Deve ser iniciado em 0.
	vazia = células vazias do buffer. Deve ser iniciado com max.
	mutex = exclusão mútua do buffer. Deve ser iniciado em 1. */
semaforo cheia, vazia, mutex;

/* Buffer compartilhado */
unsigned char *buffer;

void far produtor() {
	long unsigned int i;
	for(i = 0; i < n; i++) {
		P(&vazia);
		P(&mutex);
		buffer[prox_prod] = produz();
		/* Se prox_prod > max, prox_prod = 0; */
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
		/* Se prox_cons > max, prox_cons = 0; */
		prox_cons = (prox_cons + 1) % max;
		V(&mutex);
		V(&vazia);
	}
	/* Desalocação do buffer compartilhado */
	free(buffer);

	/*  Função usada para salvar algum dado que se ache necessário ou qualquer
		outra coisa a ser feita antes de ser chamado o volta_dos */
	retorno();
	termina_processo();
}

void iniciar_PRODCONS(long unsigned int max_buffer, /* Tamanho do buffer compartilhado */
					  long unsigned int num_it, /* Número máximo de iterações */
					  unsigned char far (*func_produz)(), /* Função chamada pelo produtor. Retornará valor a ser inserido no buffer. */
					  void far (*func_consome)(unsigned char lido), /* Função chamada pelo consumidor. Executa alguma ação com o valor retirado do buffer. */
					  void far (*func_retorno)()) { /* Função chamada quando consumidor termina de ler dados do produtor, isto é, quando chega no máximo de iterações e sai do for. */
	/* Configuração das constantes */
	max = max_buffer;
	n = num_it;

	/* Alocação do buffer usado pelo prod/cons */
	buffer = (unsigned char *) calloc (max_buffer, sizeof(unsigned char));

	/* 	Setar funções que serão chamadas pelo produtor/consumidor e que
		vieram através de chamada externa à interface do produtor/consumidor. */ 
	produz = func_produz;
	consome = func_consome;
	retorno = func_retorno;

	/* Inicialização dos semáforos com os valores necessários */
	inicia_semaforo(&cheia, 0);
	inicia_semaforo(&vazia, max_buffer);
	inicia_semaforo(&mutex, 1);

	/* Adição dos processos na fila de processos prontos para serem alocados */
	cria_processo(produtor, "prod");
	cria_processo(consumidor, "cons");

	/* Iniciar */
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
