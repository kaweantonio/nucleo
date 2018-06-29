#include "C:\NBPR\nucleo.h"

#define MAX_NIVEL_PRIORIDADE 4

/* variáveis globais */
APONTA_REG_CRIT a; /* cria variável a para acesso ao flag INDOS */
PTR_DESC_PROC prim = NULL; /* cabeça da fila dos processos prontos */
PTR_DESC_PROC p_salva; /* salvar último PRIM caso dê algum erro. */

PTR_DESC d_esc; /* ponteiro para o descritor da co-rotina do escalador */

FILA_PRIORIDADE *lista_filas_prioridade = NULL; /* vetor que conterá as filas de prioridade */
FILA_PRIORIDADE fila_atual; /* ponteiro para a fila de prioridades em uso */

int nivel_atual = MAX_NIVEL_PRIORIDADE - 1; /* inicia no nível mais alto */

PTR_DESC_PROC far prox_processo_nivel() {
	int i;
	PTR_DESC_PROC p_aux; /* ponteiro auxiliar que será usado para caminhar pela fila de processos */

	/* Usando for para passar pelos níveis anteriores */
	/* Inicia no nível atual e vai até o nível máximo. Isso simula a redução de prioridade dos processos */
	/* quando há alteração do nível. */
	for(i = nivel_atual; i < MAX_NIVEL_PRIORIDADE; i++) {
		p_aux = lista_filas_prioridade[i].inicio;

		/* Caminhar pela fila de processos até chegar em NULL (final da fila). */
		while (p_aux != NULL) {

			/* Continuar até encontrar processo ativo */
			if(p_aux->estado != ativo) {
				p_aux = p_aux->prox_desc;
			}
			else { /* Quando encontrar algum com estado == ativo, sair do laço. */
				break;
			}
		}

		/* Usado para sair do laço mais externo. */
		if(p_aux != NULL){
			break;
		}
	}
 
	return p_aux;
}

void far mudar_nivel_prioridade() {
	/* Se o nível for diferente de zero, decrementar. Em caso contrário, retornar ao valor máximo. */
	nivel_atual = nivel_atual == 0 ? MAX_NIVEL_PRIORIDADE - 1 : nivel_atual - 1;

	/* Atualizar ponteiro da fila_atual. */
	fila_atual = lista_filas_prioridade[nivel_atual];
}


PTR_DESC_PROC far procura_proximo_ativo(){
	int i;
	PTR_DESC_PROC p_aux = prox_processo_nivel();

	/* Se for NULL, mudar de fila. */
	if(p_aux == NULL) {
		/*  Iterar pela fila de prioridades. Como a fila atual já foi verificada,
			iniciar contador em 1. */
		for(i = 1; i < MAX_NIVEL_PRIORIDADE; i++) {
			/* Mudar nível de prioridade e procurar processo ativo no novo nível */ 
			mudar_nivel_prioridade();
			p_aux = prox_processo_nivel();

			/* Se encontrar algum processo ativo, parar repetição. */
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
void far imprime_fila_processos() {
	int i;
	PTR_DESC_PROC p_aux;

	for (i = MAX_NIVEL_PRIORIDADE - 1; i >= 0; i--) {
		printf("\n\t\t --- Nivel %d ---\n", i+1);
		p_aux = lista_filas_prioridade[i].inicio;
		do {
			printf("Nome: %s\t Estado: %s\n", p_aux->nome, estado_processo(p_aux));
			p_aux = p_aux->prox_desc;
		} while(p_aux != NULL);
	}
}


void far volta_dos(){
	disable();
	setvect(8, p_est->int_anterior);
	enable();
	imprime_fila_processos();
	exit(0);
}

void far iniciar_filas_processos() {
	int i;
	lista_filas_prioridade = (FILA_PRIORIDADE*) calloc (MAX_NIVEL_PRIORIDADE, sizeof(FILA_PRIORIDADE));
	
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

	/* Decrementar prioridade, pois os vetores são indexados com início em zero. */
	prioridade_corrigida--;

	/* inicialização dos campos do descritor */
	strcpy(p_aux->nome, nome_proc);
	p_aux->estado = ativo;
	p_aux->prioridade = prioridade_corrigida;
	p_aux->contexto = cria_desc();
	p_aux->prox_desc = NULL;
	newprocess(end_proc, p_aux->contexto);

	insere_fila_prontos(p_aux, prioridade_corrigida);
}

void far informacoes_processo(char nome_processo[], unsigned int *prioridade) {
	strcpy(nome_processo, prim->nome);
	*prioridade = prim->prioridade;
}

void inicializa_prim() {
	int i;
	PTR_DESC_PROC p_aux;

	/* Buscar processo no nível de prioridade mais alto */
	/* Se não encontrar, buscar nos níveis inferiores. */
	for(i = MAX_NIVEL_PRIORIDADE - 1; i >= 0; i--) {
		p_aux = lista_filas_prioridade[i].inicio;
		if(p_aux != NULL) {
			break;
		}
	}

	/* Se não houver nenhum processo em nenhuma fila, volta_dos */
	if(p_aux == NULL) {
		volta_dos();
	}
	else { /* Caso contrário, atribuir a PRIM o processo encontrado. */
		prim = p_aux;
	}
}

void far escalador(){
	int i;

	inicializa_prim();

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

/*  Terminar processo ignorando porção de tempo perdida na fatia. 
	Ela será gastada num loop infinito. */
void far termina_processo(){
	disable();
	prim->estado = terminado;
	enable();
	while(1);
}

