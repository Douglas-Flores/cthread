#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/support.h"
#include "../include/cthread.h"
#include "../include/cdata.h"

#include <signal.h>

#define STACK_SIZE SIGSTKSZ
#define MAIN_TID 0
#define TRUE 1
#define FALSE 0

int flag_init = 0;
int id_atual = 0;

PFILA2 apto, bloqueado, exec, join;
ucontext_t *init = NULL, *end = NULL;
TCB_t *MAIN, *executando;

int novoTID(){
    id_atual++;
    return id_atual;
}

void makeContext(ucontext_t *context, void* (start), void *arg, ucontext_t* link){
    char *pilha = (char*) malloc(STACK_SIZE);

    if(pilha == NULL)
        printf("\nFalha na criacao da pilha\n");

    getcontext(context);
    context->uc_stack.ss_sp = pilha;
    context->uc_stack.ss_size = STACK_SIZE;
    context->uc_link = link;
    makecontext(context, (void *)start, 1, arg);
}

TCB_t* getNextApto(){
    TCB_t *nextProcess;
    nextProcess = (TCB_t*) malloc(sizeof(TCB_t));
    PNODE2 temp;

    if(FirstFila2(apto) == 0){
        temp = apto->first;
        nextProcess = temp->node;
        DeleteAtIteratorFila2(apto);

        return nextProcess;
    }

    return -1;
}

/*void escalonador(){
    executando = getNextApto();
    if(executando == NULL){
        printf("\nErro Escalonador\n");
        return;
    }
    executando->state = PROCST_EXEC;
    setcontext(&(executando->context));
}*/

void terminar(){
    TCB_t *tcb_current = exec->first->node;

    FirstFila2(exec);
    DeleteAtIteratorFila2(exec);

    if(isWaited(tcb_current->tid) == TRUE)
        unblockWaiting(tcb_current);

    TCB_t* newTCB = (TCB_t*) malloc(sizeof(TCB_t));
    newTCB = getNextApto();

    newTCB->state = PROCST_EXEC;

    AppendFila2(exec, newTCB);

    swapcontext(&(tcb_current->context), &(newTCB->context));

    free(&(tcb_current->context));
    free(tcb_current);
}

int unblockWaiting(TCB_t* process){
    if(NextFila2(join) == 1){
        printf("Fila Vazia!\n");
        return -1;
    }
    if(FirstFila2(join) != 0)
        return FALSE;
    PNODE2 newNode = join->first;
    TCB_t *tcb_waiting = newNode->node;
    join->it = join->first;
    int *waited;
    int i = 0;

    do{
        if(i==0)
            i = 1;
        else
            newNode = newNode->next;

        tcb_waiting = newNode->node;

        NextFila2(join);
        newNode = newNode->next;
        waited = newNode->node;

        if(*waited == process->tid){
            DeleteAtIteratorFila2(join);
            join->it = newNode->ant;
            DeleteAtIteratorFila2(join);
            tcb_waiting->state = PROCST_APTO;
            AppendFila2(apto, tcb_waiting);

            return 0;
        }

    }while(NextFila2(join) == 0);

    return 0;
}

int isWaited(int tid){
    if(NextFila2(join) == 1)
        return FALSE;

    if(FirstFila2(join) != 0)
        return FALSE;

    PNODE2 newNode = join->first;
    TCB_t *tcb_current = newNode->node;
    join->it = join->first;
    int *waited;
    int i = 0;

    do{
        if(i==0)
            i = 1;
        else
            newNode = newNode->next;

        tcb_current = newNode->node;

        NextFila2(join);
        newNode = newNode->next;
        waited = newNode->node;

        if(*waited == tid)
            return TRUE;

    }while(NextFila2(join) == 0);

    return FALSE;
}

int initialize(){
    flag_init = 1;

    /*init = (ucontext_t*) malloc(sizeof(ucontext_t));
    makeContext(init, escalonador, NULL, NULL);*/
    end = (ucontext_t *) malloc(sizeof(ucontext_t));
    makeContext(end, terminar, 0, &(MAIN->context));

    MAIN = (TCB_t*) malloc(sizeof(TCB_t));
    if( MAIN == NULL)
        return -1;

    apto = malloc(sizeof(PFILA2));
    CreateFila2(apto);
    bloqueado = malloc(sizeof(PFILA2));
    CreateFila2(bloqueado);
    exec = malloc(sizeof(PFILA2));
    CreateFila2(exec);
    join = malloc(sizeof(PFILA2));
    CreateFila2(join);

    MAIN->state = PROCST_EXEC;
    MAIN->tid = MAIN_TID;
    MAIN->prio = 0;

    getcontext(&(MAIN->context));
    AppendFila2(exec, MAIN);

    return 0;
}

void printJoinQueue(){
    if(NextFila2(join) == 1){
        printf("Fila Vazia!\n");
        return;
    }
    FirstFila2(join);
    PNODE2 newNode = join->first;
    TCB_t *tcb_current = newNode->node;
    join->it = join->first;
    int *waited;
    int i = 0;

    do{
        if(i==0)
            i = 1;
        else
            newNode = newNode->next;

        tcb_current = newNode->node;
        printf("Esperando: %d\n", tcb_current->tid);
        NextFila2(join);
        newNode = newNode->next;
        waited = newNode->node;
        printf("Esperado: %d\n\n", *waited);
    }while(NextFila2(join) == 0);

}

int ccreate (void* (*start)(void*), void *arg, int prio){
    if(flag_init != 1)
        if(initialize() != 0)
            return -1;

    TCB_t *newTCB = (TCB_t*) malloc(sizeof(TCB_t));

    if(newTCB == NULL)
        return -1;

    newTCB->tid = novoTID();
    newTCB->prio = 0;
    newTCB->state = PROCST_APTO;

    makeContext(&(newTCB->context),start, arg, end);

    if(newTCB == NULL || &(newTCB->context) == NULL){
        return -1;
    }

    AppendFila2(apto, newTCB);


    return 0;
}

int cyield(void){
    TCB_t *tcb_current = exec->first->node;
    //Adiciona o TCB da thread em execucao no final da fila de apto
    tcb_current->state = PROCST_APTO;
    if(tcb_current == NULL)
        return -1;
    AppendFila2(apto, tcb_current);

    //Retira o Primeiro da fila de apto e insere na fila de executando
    TCB_t* newTCB = (TCB_t*) malloc(sizeof(TCB_t));
    newTCB = getNextApto();
    newTCB->state = PROCST_EXEC;

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

int cwait(csem_t *sem){
    TCB_t* tcb_current = exec->first->node;

    if(sem->count <= 0){
        //Adiciona o TCB da thread em execucao no final da fila de bloqueado do semáforo
        tcb_current->state = PROCST_BLOQ;
        if(tcb_current == NULL)
            return -1;
        AppendFila2(sem->fila, tcb_current);

        //Retira o Primeiro da fila de apto e insere na fila de executando
        TCB_t* newTCB = (TCB_t*) malloc(sizeof(TCB_t));
        newTCB = getNextApto();
        newTCB->state = PROCST_EXEC;

        if(FirstFila2(exec) == 0)
            DeleteAtIteratorFila2(exec);
        else
            return -1;

        AppendFila2(exec, newTCB);

        sem->count--;
        return swapcontext(&(tcb_current->context), &(newTCB->context));
    }

    sem->count--;
    return 0;

}

int csignal(csem_t *sem){
    TCB_t* tcb_current = exec->first->node;

    if(sem->count <= 0 || NextFila2(sem->fila) != 1){
        //printf("TID Saindo: %d\n", tcb_current->tid);
        //Retira o Primeiro da fila de bloqueados do semáforo e insere na fila de aptos
        TCB_t* newTCB = (TCB_t*) malloc(sizeof(TCB_t));
        newTCB = sem->fila->first->node;
        newTCB->state = PROCST_APTO;

        if(FirstFila2(sem->fila) == 0)
            DeleteAtIteratorFila2(sem->fila);
        else
            return -1;

        AppendFila2(apto, newTCB);
    }

    sem->count++;

    return 0;
}

int cjoin(int tid){
    if(isWaited(tid) == TRUE)
        return -1;

    TCB_t *tcb_current = exec->first->node;
    if(tcb_current == NULL)
        return -1;
    //Adiciona o processo 'waiting' na fila de join
    FirstFila2(join);
    AppendFila2(join, tcb_current);
    //Adiciona o processo 'waited' na fila de join
    AppendFila2(join, &tid);
    //Adiciona o TCB da thread em execucao no final da fila de bloqueados
    tcb_current->state = PROCST_BLOQ;
    AppendFila2(bloqueado, tcb_current);

    //Retira o Primeiro da fila de apto e insere na fila de executando
    TCB_t* newTCB = (TCB_t*) malloc(sizeof(TCB_t));
    newTCB = getNextApto();
    newTCB->state = PROCST_EXEC;

    if(FirstFila2(exec) == 0)
        DeleteAtIteratorFila2(exec);

    AppendFila2(exec, newTCB);

    printJoinQueue();

    return swapcontext(&(tcb_current->context), &(newTCB->context));
}

int cidentify (char *name, int size) {
    strncpy (name, "Douglas Souza Flores - 262524\nGiulia - 000000", size);
    return 0;
}
