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

/* vairáveis globais */
PTR_DESC_PROC prim = NULL; /* cabeça da fila dos processos prontos */

PTR_DESC d_esc; /* ponteiro para o descritor da co-rotina do escalador */

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

PTR_DESC_PROC far procura_proximo_ativo(){
	PTR_DESC_PROC p;

	p = prim;

	while (p->prox_desc->estado != ativo && p->prox_desc != prim){
		p = p->prox_desc;
	}

	if (p->prox_desc == prim)
		return NULL;
	return p->prox_desc;
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
	p_est->p_origem = d_esc;
	p_est->p_destino = prim->contexto;
	p_est->num_vetor = 8;

	while(1){
		iotransfer();
		disable();
		prim = prim->prox_desc;
		/*prim = procura_proximo_ativo();

		if (prim == NULL)
			volta_dos();*/
		p_est->p_destino = prim->contexto;
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

/*
void far termina_processo(){
	PTR_DESC_PROC proc_termina;
	disable();
	prim->estado = terminado;
	p_aux = procura_proximo_ativo();
	proc_termina = prim;
	prim = p_aux;
	enable();
	trasnfer(proc_termina->contexto, prim->contexto);
}*/
