#include <system.h>
#include <stdio.h>
#include <string.h>

/* Estrutura para Registros do endereço do flag INDOS retornado pela função 0x34 */
typedef struct registros {
	unsigned bx1, es1;
} regis;

/* Union da estrutura Regis para uma variável de 2 bytes (char) */
typedef union k {
	regis x;
	char far *y;
} APONTA_REG_CRIT;

/* Estrutura do Descritor de Processo (BCP) */
typedef struct desc_p {
	char nome[35];
	enum {ativo, bloq_P, terminado} estado;
	PTR_DESC contexto;
	struct desc_p *prox_desc;
	struct desc_p *fila_sem;
} DESCRITOR_PROC;

/* ponteiro do tipo DESCRITOR_PROC */
typedef DESCRITOR_PROC *PTR_DESC_PROC; 

/* vairáveis globais */
APONTA_REG_CRIT a; /* cria variável a para acesso ao flag INDOS */
PTR_DESC_PROC prim = NULL; /* cabeça da fila dos processos prontos */
PTR_DESC_PROC p_salva;

PTR_DESC d_esc; /* ponteiro para o descritor da co-rotina do escalador */

/* Estrutura para semáforos*/
typedef struct {
	int s;
	PTR_DESC_PROC Q;
} semaforo;

void far inicia_semaforo(semaforo *sem, int n){
	sem->s = n;
	sem->Q = NULL;
}

void far insere_fila_bloqueados(PTR_DESC_PROC Q){	
	PTR_DESC_PROC p_aux;

	if (!Q){
		Q = prim;
		Q->fila_sem = NULL;
		return;
	}
	p_aux = Q;

	while (p_aux->fila_sem != NULL){
		p_aux = p_aux->fila_sem;
	}

	p_aux->fila_sem = prim;
	prim->fila_sem = NULL;
}

void far remove_fila_bloqueados(PTR_DESC_PROC Q){
	PTR_DESC_PROC p_aux;
	Q->estado = ativo;
	p_aux = Q->fila_sem;
	Q = Q->fila_sem;
	p_aux->fila_sem = NULL;
}

PTR_DESC_PROC far procura_proximo_ativo(){
	PTR_DESC_PROC p_aux;

	/* começa busca pelo próximo processo a partir do próximo descritor de processos apontador por prim */	
	p_aux = prim->prox_desc;

	while (p_aux->estado != ativo && p_aux->prox_desc != prim->prox_desc){
		p_aux = p_aux->prox_desc;
	}

	if (p_aux->prox_desc == prim->prox_desc && prim->estado == terminado){
		p_salva = prim;
		return NULL;
	}
	return p_aux;
}

void far imprime_fila_processos(){
	PTR_DESC_PROC p_aux;
	p_aux = p_salva->prox_desc;
	do {
		printf("Nome: %s\t Estado: ", p_aux->nome);
		if (p_aux->estado == terminado)
			printf("terminado\n");
		else printf("ativo\n");
		p_aux = p_aux->prox_desc;
	} while (p_aux != p_salva->prox_desc);
}

void far volta_dos(){
	disable();
	setvect(8, p_est->int_anterior);
	enable();
	imprime_fila_processos();
	exit(0);
}

void far P(semaforo *sem){
	PTR_DESC_PROC p_aux;
	disable();

	if (sem->s > 0){
		(sem->s)--;
		enable();
	} else {
		prim->estado = bloq_P;
		p_aux = prim;
		insere_fila_bloqueados(sem->Q);
		prim = procura_proximo_ativo();
		if (prim == NULL)
			volta_dos();
		transfer(p_aux->contexto, prim->contexto);
	}
}

void far V(semaforo *sem){
	disable();
	if (sem->Q){
		remove_fila_bloqueados(sem->Q);
	} else {
		(sem->s)++;
	}
	enable();
}

void far insere_fila_prontos(PTR_DESC_PROC p){	
	PTR_DESC_PROC q;
	if (!prim){
		prim = p;
		prim->prox_desc = prim;
		return;
	}
	
	q = prim;

	while (q->prox_desc != prim){
		q = q->prox_desc;
	}

	q->prox_desc = p;
	p->prox_desc = prim;
}

/* cria processo e adiciona na fila de processos prontos (por enquanto)
 |end_proc|  endereço de localização na memória do processo;
 |nome_proc| nome dado pelo usuário para identificação do processo */
void far cria_processo(void far(*end_proc)(), char nome_proc[35]){
	PTR_DESC_PROC p_aux;
	p_aux = (DESCRITOR_PROC*) malloc(sizeof (DESCRITOR_PROC));;
	if (p_aux == NULL){
		printf("\nMemória insuficiente para alocação de descritor\n");
		exit(1);
	}

	/* inicialização dos campos do descritor */
	strcpy(p_aux->nome, nome_proc);
	p_aux->estado = ativo;
	p_aux->contexto = cria_desc();
	newprocess(end_proc, p_aux->contexto);

	insere_fila_prontos(p_aux);
}

void far escalador(){
	int i;
	p_est->p_origem = d_esc;
	p_est->p_destino = prim->contexto;
	p_est->num_vetor = 8;

	/* inicia ponteiro para Região Crítica do DOS */
	_AH = 0x34;
	_AL = 0x00;
	geninterrupt(0x21);
	a.x.bx1 = _BX;
	a.x.es1 = _ES;

	while(1){
		iotransfer();
		disable();
		
		/*  verifica se processo ainda está usando algum recurso do DOS (chamada ao DOS).
			Caso verdadeiro, processo está na R.C, portanto não escalona outro processo.
			Se falso, processo não está na R.C, portanto escalona outro processo normalmente.
		*/
		if (*a.y == 0) {
			prim = procura_proximo_ativo();
			if (prim == NULL)
				volta_dos();
			p_est->p_destino = prim->contexto;
		}
		enable();
	}
}

void far dispara_sistema(){
	PTR_DESC desc_dispara;
	d_esc = cria_desc();
	desc_dispara = cria_desc();
	newprocess(escalador, d_esc);
	transfer(desc_dispara, d_esc);
}

/* termina_processo para quando se deseja salvar a fatia de tempo restante que o processo teria caso não tivesse terminado
void far termina_processo(){
	PTR_DESC_PROC p_aux;
	disable();
	prim->estado = terminado;
	p_aux = prim;
	prim = procura_proximo_ativo();
	if (prim == NULL)
		volta_dos();
	transfer(p_aux->contexto, prim->contexto);
}*/

/* termina_processo que ignora fatia de tempo perdida pelo processo */
void far termina_processo(){
	disable();
	prim->estado = terminado;
	enable();
	while(1);
}
