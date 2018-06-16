#include "C:\NBT\nucleo.h"

PTR_DESC_PROC p1, p2;
enum retorno{sucesso = 0, proc_inacessivel = 1, fila_cheia = 2};

/*  Teste 1.1 - Status 1 (Fracasso - processo não existe) 
    Nesse teste, o fracasso ocorre devido a um erro de digitação do programador

void far proc_emissor(){
    char msg[35] = "Essa mensagem nunca sera lida";
    int status;

    status = envia(msg, "proc_recpetor");
    if (status == proc_inacessivel){
        printf("Nao foi possivel encontrar processo destino para enviar a mensagem! Abortar\n");
    }

    termina_processo();
}

void far proc_receptor(){
    char msg[35];

    recebe(msg, "proc_emissor");

    // como processo nunca receberá mensagem o mesmo não será acordado, logo sistema irá terminar com proc_receptor bloqueado 

    termina_processo();
}*/

/*  Teste 1.2 - Status 1 (Fracasso - processo existe mas está terminado) 

void far proc_emissor(){
    char msg[35] = "Essa mensagem nunca sera lida";
    int status;

    status = envia(msg, "proc_receptor");
    if (status == proc_inacessivel){
        printf("Nao foi possivel encontrar processo destino para enviar a mensagem! Abortar\n");
    }

    termina_processo();
}

void far proc_receptor(){
    termina_processo();
} 
*/

/*  Teste 2 - Status 2 (Fracasso - fila de mensagens cheia) 

void far proc_emissor(){
    char msg[35] = "Testando troca de mensagens";
    int status;

    status = envia(msg, "proc_receptor");

    switch(status) {
        case proc_inacessivel:
            printf("Nao foi possivel encontrar processo destino para enviar a mensagem! Abortar 1\n");
        break;
    
        case fila_cheia:
            printf("Fila cheia 1\n");
        break;

        case sucesso:
            printf("Mensagem enviada! 1\n");
        break;

        default:
            printf("Erro não especificado. Abortar 1\n");
    }

    termina_processo();
}

void far proc_emissor2(){
    char msg[35] = "Testando troca de mensagens 2";
    int status;

    status = envia(msg, "proc_receptor");
    switch(status) {
        case proc_inacessivel:
            printf("Nao foi possivel encontrar processo destino para enviar a mensagem! Abortar 2\n");
        break;
    
        case fila_cheia:
            printf("Fila cheia 2\n");
        break;

        case sucesso:
            printf("Mensagem enviada! 2\n");
        break;

        default:
            printf("Erro não especificado. Abortar 2\n");
    }

    termina_processo();
}

void far proc_emissor3(){
    char msg[35] = "Testando troca de mensagens 3";
    int status;

    status = envia(msg, "proc_receptor");
    switch(status) {
        case proc_inacessivel:
            printf("Nao foi possivel encontrar processo destino para enviar a mensagem! Abortar 1\n");
        break;
    
        case fila_cheia:
            printf("Fila cheia 1\n");
        break;

        case sucesso:
            printf("Mensagem enviada! 1\n");
        break;

        default:
            printf("Erro não especificado. Abortar 1\n");
    }

    termina_processo();
}

void far proc_receptor(){
    char msg[35], emissor[35];
    int i;

    for (i = 0; i < 2; i++){
        recebe(msg, emissor);
        printf("Recebeu mensagem %s do processo %s\n\n", msg, emissor);
    }
    termina_processo();
} 
*/

/*  Teste 3 - Status 2 (Sucesso - mensagens enviadas com sucesso)

    Troca de mensagens envolvendo os 20 primeiros números da Sequência de Fibonacci
    a partir do terceiro número da sequência (F2)
*/

int x, y, z, fibo;
semaforo mutex;

void far proc_emissor(){
    char msg[35];
    int status, i = 0;

    for(i = 0; i < 10; i++){
        P(&mutex);
        z = x+y;
        sprintf(msg, "Fibo[%d] = %d", fibo, z);
        x = y;
        y = z;
        fibo++;
        V(&mutex);

        status = envia(msg, "proc_receptor");

        if(status == proc_inacessivel) {
            printf("Nao foi possivel encontrar processo destino para enviar a mensagem!\n");
        }
        else if(status == fila_cheia) {
            while(status == fila_cheia) {
                status = envia(msg, "proc_receptor");
            }
        }
    }

    termina_processo();
}

void far proc_receptor(){
    char msg[35], emissor[35];
    int i = 0;

    while (i < 20){
        recebe(msg, emissor);
        printf("Recebeu mensagem[%d] %s do processo %s\n\n", i+1, msg, emissor);
        i++;
    }
    termina_processo();
} 

main() {
    /* Teste 1.1
    cria_processo(proc_emissor, "proc_emissor", 1);
    cria_processo(proc_receptor, "proc_receptor", 1);
    */
    
    /* Teste 1.2
    cria_processo(proc_receptor, "proc_receptor", 1);
    cria_processo(proc_emissor, "proc_emissor", 1);
    */

    /* Teste 2
    cria_processo(proc_emissor, "proc_emissor1", 0);
    cria_processo(proc_emissor2, "proc_emissor2", 0);
    cria_processo(proc_emissor3, "proc_emissor3", 1);
    cria_processo(proc_receptor, "proc_receptor", 2);
    */

    /* Teste 3 */
    fibo = 2;
    x = 0;
    y = 1;
    inicia_semaforo(&mutex, 1);
    cria_processo(proc_emissor, "proc_emissor1", 0);
    cria_processo(proc_emissor, "proc_emissor2", 0);
    cria_processo(proc_receptor, "proc_receptor", 2);
    dispara_sistema();
}