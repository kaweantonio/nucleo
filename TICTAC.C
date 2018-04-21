#include <stdio.h>
#include <system.h>

PTR_DESC dtic, dtac, dmain;

void far tic(){
	int cont = 0;
	while(cont < 100){
		printf("tic ");
		transfer(dtic, dtac);
		cont++;
	}
	printf("%d", cont);
	transfer(dtic, dmain);
}

void far tac(){
	while(1){
		printf(" tac\n");
		transfer(dtac, dtic);
	}
}

main(){
	dtic = cria_desc();
	dtac = cria_desc();
	dmain = cria_desc();
	newprocess(tic, dtic);
	newprocess(tac, dtac);
	transfer(dmain, dtic);
}
