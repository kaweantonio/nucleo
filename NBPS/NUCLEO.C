#include "C:\NBPS\nucleo.h"

#define MAX_PRIORIDADE 10

/* variáveis globais */
APONTA_REG_CRIT a; /* cria variável a para acesso ao flag INDOS */
PTR_DESC_PROC prim = NULL; /* cabeça da fila dos processos prontos */
PTR_DESC_PROC p_salva; /* salvar último PRIM caso dê algum erro. */

PTR_DESC d_esc; /* ponteiro para o descritor da co-rotina do escalador */


PTR_DESC_PROC far procura_proximo_ativo(){
	PTR_DESC_PROC p_aux;

	/* Começa busca pelo próximo processo a partir do próximo descritor de processos apontador por prim */	
	p_aux = prim->prox_desc;

	/* Caminhar pela fila de processos buscando algum ativo ou voltar ao processo atual. */
	while (p_aux->estado != ativo && p_aux->prox_desc != prim->prox_desc){
		p_aux = p_aux->prox_desc;
	}

	/*  Voltou ao processo atual e o estado dele não é ativo. 
		Erro no sistema. Salvar PRIM e avisar retornando NULL. */
	if (p_aux->prox_desc == prim->prox_desc && prim->estado != ativo){
		p_salva = prim;
		return NULL;
	}

	/* Encontrou novo processo sem nenhum erro. */
	return p_aux;
}

/* 	Função auxiliar para retornar uma string do estado do processo. 
	Ela será usada junto à função 'imprime_fila_processos'. */
char * estado_processo(PTR_DESC_PROC p) {
	switch(p->estado) {
		case ativo:
			return "ativo";
		case terminado:
			return "terminado";
		default:
			return "erro";
	}
}

/*  Função auxiliar usada para imprimir fila de processos e 
	seus estados ao final da execução de todos os processos do sistema. */	
void far imprime_fila_processos(){
	PTR_DESC_PROC p_aux;
	p_aux = p_salva->prox_desc;
	do {
		printf("Nome: %s\t Estado: ", p_aux->nome);
		printf("%s\n", estado_processo(p_aux));
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

/* Cria processo e adiciona na fila de processos prontos.
 |end_proc|  endereço de localização do código (ponteiro para função);
 |nome_proc| nome dado pelo usuário para identificação do processo
 |prioridade| define quantas fatias o processo receberá por vez */
void far cria_processo(void far(*end_proc)(), char nome_proc[35], unsigned int prioridade){
	int prioridade_corrigida;
	PTR_DESC_PROC p_aux;
	p_aux = (DESCRITOR_PROC*) malloc(sizeof (DESCRITOR_PROC));
	if (p_aux == NULL){
		printf("\nMemória insuficiente para alocação de descritor\n");
		exit(1);
	}

	/* Correção da prioridade */

	/* Se for menor que zero, colocar 0. */
	prioridade_corrigida = prioridade < 0 ? 0 : prioridade;

	/* Se for maior que máximo, colocar máximo. */
	prioridade_corrigida = prioridade_corrigida > MAX_PRIORIDADE ? 10 : prioridade_corrigida;

	/* inicialização dos campos do descritor */
	strcpy(p_aux->nome, nome_proc);
	p_aux->estado = ativo;
	p_aux->prioridade = prioridade_corrigida;
	p_aux->fatias_restantes = prioridade_corrigida;
	p_aux->contexto = cria_desc();
	newprocess(end_proc, p_aux->contexto);

	insere_fila_prontos(p_aux);
}

void far informacoes_processo(char nome_processo[], unsigned int *prioridade) {
	strcpy(nome_processo, prim->nome);
	*prioridade = prim->prioridade;
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

			/* Verificar se ainda é necessário dar mais fatias ao mesmo processo */
			if(prim->fatias_restantes == 0) { /* Quantidade já acabou */

				/*  Restaurar valor do contador auxiliar de prioridade.
					Isso deve ser feito para que não tenha problemas
					na próxima vez que ele seja escalado. */
				prim->fatias_restantes = prim->prioridade;
				prim = procura_proximo_ativo();
				if (prim == NULL)
					volta_dos();
				p_est->p_destino = prim->contexto;
			}
			else { /* Ainda há fatias. Não trocar o processo, apenas decrementar contador. */
				prim->fatias_restantes--;
			}
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

/*  Terminar processo ignorando porção de tempo perdida na fatia. 
	Ela será gastada num loop infinito. */
void far termina_processo(){
	disable();
	prim->estado = terminado;
	enable();
	while(1);
}
