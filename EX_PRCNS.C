#include <stdio.h>
#include <prodcons.h>

FILE *input, *output, *log;
long unsigned int size, max_buffer, pos_atual = 0, it_serv = 0;
unsigned char *download_servidor;

unsigned char far servidor() {
	unsigned char novo_byte;
	fread(&novo_byte, sizeof(unsigned char), 1, input);
	fprintf(log, "Servidor %d: %02x\n", it_serv++, novo_byte);
	return novo_byte;
}

void far cliente(unsigned char novo_byte) {
	fprintf(log, "Cliente %d: %02x\n", pos_atual, novo_byte);
	download_servidor[pos_atual++] = novo_byte;
}

void far configurar() {
	fseek(input, 0, SEEK_END);
	size = ftell(input);
	fseek(input, 0, SEEK_SET);
	max_buffer = size / 100;
	printf("%lu\n", size);
	download_servidor = (unsigned char *) calloc (size, sizeof(unsigned char));
}

void far salva_dados() {
	fclose(input);
	fwrite(download_servidor, sizeof(unsigned char), size, output);
	free(download_servidor);
	fclose(output);
	fclose(log);
}

void far main() {
	log = fopen("LOGPRCN.txt", "w");
	input = fopen("IMGIN.jpg", "rb");
	output = fopen("IMGOUT.jpg", "wb");
	configurar();
	iniciar_PRODCONS(max_buffer, size, servidor, cliente, salva_dados);
}

