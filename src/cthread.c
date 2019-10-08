#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/support.h"
#include "../include/cthread.h"
#include "../include/cdata.h"

#include <signal.h>

#define STACK_SIZE SIGSTKSZ
#define MAIN_TID 0
#define INICIALIZADA 1
#define TRUE 1
#define FALSE 0

int flag_init = 0;
int id_atual = 0;

PFILA2 apto, bloqueado, exec;
ucontext_t *init = NULL, *end = NULL;
TCB_t *MAIN, *executando;

int novoTID(){
    id_atual++;
    return id_atual;
}

TCB_t* createTCB(){
    TCB_t *newTCB = (TCB_t*) malloc(sizeof(TCB_t));

    if(newTCB == NULL){
        return NULL;
    }

    newTCB->tid = novoTID();
    newTCB->prio = 0;
    newTCB->state = PROCST_APTO;

    return newTCB;
}

void makeContext(ucontext_t *context, void* (start), void *arg, ucontext_t* link){
    char *pilha = (char*) malloc(STACK_SIZE);

    if(pilha == NULL ){
        context = NULL;
        return;
    }


    context = (ucontext_t*) malloc(sizeof(ucontext_t));
    if(context == NULL ){
        context = NULL;
        return;
    }

    getcontext(context);
    context->uc_stack.ss_sp = pilha;
    context->uc_stack.ss_size = STACK_SIZE;
    context->uc_link = link;
    makecontext(context, (void *)start, 1, arg);
}

TCB_t* getNextApto(){
    TCB_t *nextProcess;
    PNODE2 temp;

    if(FirstFila2(apto) == 0){
        temp = GetAtIteratorFila2(apto);
        nextProcess = temp->node;
        DeleteAtIteratorFila2(apto);

        return nextProcess;
    }

    return -1;
}

void escalonador(){
    executando = getNextApto();
    if(executando == NULL){
        printf("\nErro Escalonador\n");
        return;
    }
    executando->state = PROCST_EXEC;
    setcontext(&(executando->context));
}

void finalizar(){
    TCB_t *tcb_current = executando;
    getcontext(&(tcb_current->context));

    /*if(estaSendoEsperada(tcb_atual->tid)){
    terminaJoin(tcb_atual->tid);
    }
    free(&(tcb_atual->context));
    free(tcb_atual);
    setcontext(init);*/
}

int initialize(){
    flag_init = 1;

    init = malloc(sizeof(ucontext_t*));
    makeContext(init, escalonador, NULL, NULL);
    end = malloc(sizeof(ucontext_t*));
    makeContext(end, finalizar, NULL, NULL);

    MAIN = (TCB_t*) malloc(sizeof(TCB_t));
    if(init == NULL || end == NULL || MAIN == NULL)
        return -1;

    apto = malloc(sizeof(PFILA2));
    CreateFila2(apto);
    bloqueado = malloc(sizeof(PFILA2));
    CreateFila2(bloqueado);
    exec = malloc(sizeof(PFILA2));
    CreateFila2(exec);

    MAIN->state = PROCST_EXEC;
    MAIN->tid = MAIN_TID;
    MAIN->prio = 0;

    AppendFila2(exec, MAIN);

    return 0;
}

int ccreate (void* (*start)(void*), void *arg, int prio){
    if(flag_init != 1)
        if(initialize() != 0)
            return -1;

    TCB_t *newTCB;
    newTCB = createTCB();
    makeContext(&(newTCB->context),start, arg, end);

    if(newTCB == NULL || &(newTCB->context) == NULL){
        return -1;
    }

    AppendFila2(apto, newTCB);

    return 0;
}

int cyield(void){
    TCB_t *tcb_current = exec->first;
    //Adiciona o TCB da thread em execução no final da fila de apto
    PNODE2 newNode = (PNODE2) malloc(sizeof(NODE2));
    newNode->node = tcb_current;
    if(tcb_current == NULL || newNode == NULL)
        return -1;
    AppendFila2(apto, newNode);

    //Retira o Primeiro da fila de apto e insere na fila de executando
    TCB_t* newTCB = getNextApto();
    if(FirstFila2(exec) == 0)
        DeleteAtIteratorFila2(exec);
    AppendFila2(exec, newTCB);

    return swapcontext(&(tcb_current->context), &(newTCB->context));
}

int csem_init(csem_t *sem, int count){
    //Inicializa contador
    sem->count = count;

    //Inicializa fila
    PFILA2 bloc_sem = malloc(sizeof(PFILA2));

    if(bloc_sem == NULL)
        return -1;

    if(CreateFila2(bloc_sem) != 0)
        return -1;

    sem->fila = bloc_sem;

    return 0;

}

int cidentify (char *name, int size) {
    strncpy (name, "Douglas Souza Flores - 262524\nGiulia - 000000", size);
    return 0;
}
