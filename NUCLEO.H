#include <system.h>
#include <stdio.h>
#include <string.h>

/* Esturucta do Descritor de Processo (BCP) */
typedef struct desc_p {
	char nome[35];
	enum { ativo, terminado} estado;
	PTR_DESC contexto;
	struct desc_p *prox_desc;
} DESCRITOR_PROC;

/* ponteiro do tipo DESCRITOR_PROC */
typedef DESCRITOR_PROC *PTR_DESC_PROC;

/* Estrutura para semáforos*/
typedef struct {
	int s;
	PTR_DESC_PROC Q;
} semaforo;

extern void far inicia_semaforo(semaforo *sem, int n);

extern void far P(semaforo *sem);

extern void far V(semaforo *sem); 

extern void far insere_fila_prontos(PTR_DESC_PROC p);

extern void far imprime_fila_processos();

extern PTR_DESC_PROC far procura_proximo_ativo();

extern void far cria_processo(void far(*end_proc)(), char nome_proc[35]);

extern void far volta_dos();

extern void far escalador();

extern void far dispara_sistema();

extern void termina_processo();
