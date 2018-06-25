#include "C:\NBPR\nucleo.h"

#define MAX_NIVEL_PRIORIDADE 4

/* variáveis globais */
APONTA_REG_CRIT a; /* cria variável a para acesso ao flag INDOS */
PTR_DESC_PROC prim = NULL; /* cabeça da fila dos processos prontos */
PTR_DESC_PROC p_salva; /* salvar último PRIM caso dê algum erro. */

PTR_DESC d_esc; /* ponteiro para o descritor da co-rotina do escalador */

FILA_PRIORIDADE *lista_filas_prioridade = NULL;
FILA_PRIORIDADE fila_atual;

int nivel_atual = MAX_NIVEL_PRIORIDADE;

void far iniciar_filas_processos() {
	int i;
	lista_filas_prioridade = (FILA_PRIORIDADE*) malloc (MAX_NIVEL_PRIORIDADE * sizeof(FILA_PRIORIDADE));
	
	/* Verificar se foi possível alocar. */
	if(lista_filas_prioridade == NULL) {
		printf("\nNão foi possível alocar fila de prioridades. Finalizar.\n");
		exit(1);
	}

	/* Se tiver sido, inicializar ponteiros de controle das filas. */
	for(i = 0; i < MAX_NIVEL_PRIORIDADE; i++) {
		fila_atual = lista_filas_prioridade[i];
		fila_atual.inicio = NULL;
		fila_atual.fim = NULL;
	}
}

PTR_DESC_PROC far prox_processo_nivel() {
	PTR_DESC_PROC p_aux;
	
	p_aux = fila_atual.inicio;

	/* Caminhar pela fila de processos buscando algum ativo ou chegar ao final da fila. */
	while (p_aux->prox_desc != NULL && p_aux->estado != ativo){
		p_aux = p_aux->prox_desc;
	}

	return p_aux;
}

void far mudar_nivel_prioridade() {
	int i, prox_nivel;
	PTR_DESC_PROC p_aux;
	FILA_PRIORIDADE prox_fila;

	prox_nivel = (nivel_atual + 1) % MAX_NIVEL_PRIORIDADE;

	/* Se desceu nível, adicionar processos do nível anterior à fila atual. */ 
	if(prox_nivel > 0) {
		prox_fila = lista_filas_prioridade[prox_nivel];
		p_aux = prox_fila.fim;
		p_aux->prox_desc = fila_atual.inicio;
		nivel_atual = prox_nivel;
		fila_atual = prox_fila;
	}

	/* Senão, subiu de nível. Retornar filas ao estado inicial. */
	else {
		for(i = 0; i < MAX_NIVEL_PRIORIDADE; i++) {
			prox_fila = lista_filas_prioridade[i];
			p_aux = prox_fila.fim;
			p_aux->prox_desc = NULL;
		}
	}
}


PTR_DESC_PROC far procura_proximo_ativo(){
	int i;
	PTR_DESC_PROC p_aux = prox_processo_nivel();

	/* Se for NULL, mudar de fila. */
	if(p_aux == NULL) {
		/*  Iterar pela fila de prioridades. Como a fila atual já foi verificada,
			iniciar contador em 1. */
		for(i = 1; i < MAX_NIVEL_PRIORIDADE; i++) { 
			mudar_nivel_prioridade();
			p_aux = prox_processo_nivel();

			if(p_aux != NULL) {
				break;
			}
		}
	}

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

void far insere_fila_prontos(PTR_DESC_PROC p, int nivel){	
	/* Adiciona processo ao final da fila */

	int i;
	PTR_DESC_PROC q;

	/* Na primeira execução, alocar filas. */
	if(lista_filas_prioridade == NULL) {
		iniciar_filas_processos();
	}

	/* Atualizar início da fila. */
	if(lista_filas_prioridade[nivel].inicio == NULL) {
		lista_filas_prioridade[nivel].inicio = p;
	}

	/* Inserir processo no final da fila e atualizar ponteiros. */
	if(lista_filas_prioridade[nivel].fim == NULL) {
		lista_filas_prioridade[nivel].fim = p;
	}
	else {
		q = lista_filas_prioridade[nivel].fim;
		q->prox_desc = p;
		lista_filas_prioridade[nivel].fim = p;
	}
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
	prioridade_corrigida = prioridade < 1 ? 1 : prioridade;

	/* Se for maior que máximo, colocar máximo. */
	prioridade_corrigida = prioridade_corrigida > MAX_NIVEL_PRIORIDADE ? MAX_NIVEL_PRIORIDADE : prioridade_corrigida;

	/* inicialização dos campos do descritor */
	strcpy(p_aux->nome, nome_proc);
	p_aux->estado = ativo;
	p_aux->prioridade = prioridade_corrigida;
	p_aux->fatias_restantes = prioridade_corrigida;
	p_aux->contexto = cria_desc();
	p_aux->prox_desc = NULL;
	newprocess(end_proc, p_aux->contexto);

	insere_fila_prontos(p_aux, prioridade_corrigida);
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
	printf("askdaijda");
	while(1){
		iotransfer();
		printf("askdaijda");
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
