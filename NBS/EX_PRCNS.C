#include "C:\NBS\prodcons.h"

FILE *input, *output;
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
	return novo_byte;
}

/* Função que será chamada pelo consumidor (consome) */
void far cliente(unsigned char novo_byte) {
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
}

void far main() {
	input = fopen("C:\\NBS\\IMGIN.jpg", "rb"); /* modo de acesso: leitura de arquivo binário */
	output = fopen("C:\\NBS\\IMGOUT.jpg", "wb"); /* modo de acesso: escrita de arquivo binário */
	configurar();
	iniciar_PRODCONS(max_buffer, size, servidor, cliente, salva_dados);
}
