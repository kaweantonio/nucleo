#include <stdio.h>
#include <prodcons.h>

FILE *input, *output, *log;
/* Long unsigned porque tamanho do arquivo pode ser muito grande, aí não caberia no int. */
long unsigned int size, max_buffer, pos_atual = 0, it_serv = 0;
/* Unsigned char para não perder dados do bit mais significativo, 
   pois quando a variável é signed, o último bit é usado pra isso (o sinal). */
unsigned char *download_servidor;

/* Função que será chamada pelo produtor (produz) */
unsigned char far servidor() {
	unsigned char novo_byte;
	/* 	Salva o próximo byte lido de input em novo_byte.
		aqui, na verdade, poderia substituir o sizeof(unsigned char) por 1, já que
		um unsigned char tem exatamente um byte de tamanho, mas não coloquei porque
		como estava dando erro, talvez pudesse ser disso. */
	fread(&novo_byte, sizeof(unsigned char), 1, input);
	/* Salvar no log qual o byte escrito no buffer. */
	fprintf(log, "Servidor %d: %02x\n", it_serv++, novo_byte);
	return novo_byte;
}

/* Função que será chamada pelo consumidor (consome) */
void far cliente(unsigned char novo_byte) {
	/* Salvar no log qual o byte lido. */
	fprintf(log, "Cliente %d: %02x\n", pos_atual, novo_byte);
	/* Salvar no arquivo de saída o dado lido e avançar o iterador para próxima posição*/
	download_servidor[pos_atual++] = novo_byte;
}

/*  Definir tamanho do arquivo, do buffer usado pelo prod/cons 
	e alocação do buffer que salvará as informações recebidas pelo cliente. */
void far configurar() {
	/* Posiciona ponteiro de leitura no final do arquivo */
	fseek(input, 0, SEEK_END);
	/* Vê em qual byte o ponteiro está. Isso é usado para saber o tamanho do arquivo. */
	size = ftell(input);
	/* Volta ponteiro de leitura ao início do arquivo para poder começar a leitura. */
	fseek(input, 0, SEEK_SET);
	max_buffer = size / 100;
	printf("%lu\n", size);
	download_servidor = (unsigned char *) calloc (size, sizeof(unsigned char));
}

/*  Chamada pelo consumidor após consumir último dado do arquivo
	Essa função é a que salva os dados lidos no arquivo de saída e termina
	a execução, desalocando e fechando quem precisa. */
void far salva_dados() {
	/* Arquivo input não será mais necessário. */
	fclose(input);
	/*  Salvar no arquivo de saída os dados do buffer que armazenou os valores
		recebidos do servidor. */
	fwrite(download_servidor, sizeof(unsigned char), size, output);
	/* Buffer não é mais necessário */
	free(download_servidor);
	/* Fechar últimos dois arquivos */
	fclose(output);
	fclose(log);
}

void far main() {
	log = fopen("LOGPRCN.txt", "w"); /* modo de acesso: escrita */
	input = fopen("IMGIN.jpg", "rb"); /* modo de acesso: leitura de arquivo binário */
	output = fopen("IMGOUT.jpg", "wb"); /* modo de acesso: escrita de arquivo binário */
	configurar();
	iniciar_PRODCONS(max_buffer, size, servidor, cliente, salva_dados);
}


/* -------------------------- TESTE -------------------------*/
/*
FILE *input, *output, *log;
int tamanho = 200, max_buffer = 20, pos_prod = 0, pos_cons = 0;
char it_serv = 96;

unsigned char far func_produz() {
	fprintf(log, "Produziu (%d): %c\n", pos_prod++, ++it_serv);
	return it_serv;
}

void far func_consome(unsigned char lido) {
	fprintf(log, "Consumiu (%d): %c\n", pos_cons++, lido);
}

void far func_retorna() {
	printf("ACABOU");
}

void far main() {
	log = fopen("LOGPRCN.txt", "w");
	iniciar_PRODCONS(max_buffer, tamanho, func_produz, func_consome, func_retorna);
}
*/