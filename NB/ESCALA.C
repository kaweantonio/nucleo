#include <stdio.h>
#include <system.h>

PTR_DESC dtic, dtac, d_esc, dmain;

void far escalador(){
	p_est->p_origem = d_esc;
	p_est->p_destino = dtic;
	p_est->num_vetor = 8;
	
	while(1){
		iotransfer();
		disable();
		if(p_est->p_destino == dtic)
			p_est->p_destino = dtac;
		else
			p_est->p_destino = dtic;
		enable();
	}
}

void far tic(){
	while(1){
		printf("tic ");
	}
}

void far tac(){
	while(1){
		printf("tac ");
	}
}

main(){
	dtic = cria_desc();
	dtac = cria_desc();
	d_esc = cria_desc();
	dmain = cria_desc();
	newprocess(tic, dtic);
	newprocess(tac, dtac);
	newprocess(escalador, d_esc);
	transfer(dmain, d_esc);
}
