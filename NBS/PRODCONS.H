#include "C:\NBS\nucleo.h"

void iniciar_PRODCONS(long unsigned int max, 
					  long unsigned int num_it, 
					  unsigned char far (*func_produz)(), 
					  void far (*func_consome)(unsigned char lido), 
					  void far (*func_retorno)());