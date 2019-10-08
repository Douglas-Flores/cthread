/*
 *	Programa de exemplo de uso da biblioteca cthread
 *
 *	Versão 1.0 - 14/04/2016
 *
 *	Sistemas Operacionais I - www.inf.ufrgs.br
 *
 */

#include <stdio.h>
#include "../include/cthread.h"

void* func0(void *arg) {
	printf("Eu sou a thread ID0 imprimindo %d\n", *((int *)arg));
	cyield();
	return;
}

void* func1(void *arg) {
	printf("Eu sou a thread ID1 imprimindo %d\n", *((int *)arg));
}

int main() {

	int	id0, id1;
	int i;

	id0 = ccreate(func0, (void *)&i, 0);
	cyield();
	/*csem_t* sem = malloc(sizeof(csem_t));
	csem_init(sem, 1);*/
	//id1 = ccreate(func1, (void *)&i, 0);

	printf("Eu sou a main após a criação de ID0 e ID1\n");

	printf("Eu sou a main voltando para terminar o programa\n");

    /*char* nomes = (char *) malloc(100);
    cidentify(nomes, 50);
    free(nomes);*/
}
