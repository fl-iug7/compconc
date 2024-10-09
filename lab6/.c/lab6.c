#include <stdio.h>
#include <stdlib.h>
#include "list_int.h"
#include <pthread.h>
#include "timer.h"

#define QTDE_OPS 10000000 // quantidade de operacoes sobre a lista (insercao, remocao, consulta)
#define QTDE_INI 100 // quantidade de insercoes iniciais na lista
#define MAX_VALUE 100 // valor maximo a ser inserido

// lista compartilhada iniciada 
struct list_node_s* head_p = NULL; 
// quantidade de threads no programa
int nthreads;
int escritor_ativo = 0;     // flag indicando se há um escritor ativo
int escritores_fila = 0;    // contagem de escritores na fila
int leitores = 0;           // contagem de leitores ativos

pthread_mutex_t mutex;
pthread_cond_t cond_leitor, cond_escritor;

// Função para bloquear leitores com prioridade para escritores
void reader_lock() {
    pthread_mutex_lock(&mutex);
    while (escritor_ativo || escritores_fila) {
        pthread_cond_wait(&cond_leitor, &mutex);  // leitores esperam se houver escritores ativos ou na fila
    }
    leitores++;  // incrementa o contador de leitores ativos
    pthread_mutex_unlock(&mutex);
}

// Função para desbloquear leitores
void reader_unlock() {
    pthread_mutex_lock(&mutex);
    leitores--;  // decrementa o contador de leitores ativos
    if (leitores == 0) {
        pthread_cond_signal(&cond_escritor);  // acorda um escritor se não houver mais leitores
    }
    pthread_mutex_unlock(&mutex);
}

// Função para bloquear escritores, aguardando leitores terminarem
void writer_lock() {
    pthread_mutex_lock(&mutex);
    escritores_fila++;  // incrementa o contador de escritores na fila
    while (leitores > 0 || escritor_ativo) {
        pthread_cond_wait(&cond_escritor, &mutex);  // escritores esperam se houver leitores ou escritor ativo
    }
    escritores_fila--;  // decrementa o contador de escritores na fila
    escritor_ativo = 1;  // marca que há um escritor ativo
    pthread_mutex_unlock(&mutex);
}

// Função para desbloquear escritores
void writer_unlock() {
    pthread_mutex_lock(&mutex);
    escritor_ativo = 0;  // marca que o escritor terminou
    if (escritores_fila > 0) {
        pthread_cond_signal(&cond_escritor);  // acorda outro escritor se houver escritores esperando
    } else {
        pthread_cond_broadcast(&cond_leitor);  // acorda todos os leitores se não houver escritores esperando
    }
    pthread_mutex_unlock(&mutex);
}

// Tarefa das threads
void* tarefa(void* arg) {
    long int id = (long int) arg;
    int op;
    int in = 0, out = 0, read = 0;

    // realiza operações de consulta (98%), inserção (1%) e remoção (1%)
    for (long int i = id; i < QTDE_OPS; i += nthreads) {
        op = rand() % 100;
        if (op < 98) {  // operação de leitura
            printf("Thread %ld lendo.\n", id);
            reader_lock();
            Member(i % MAX_VALUE, head_p);  /* Ignore return value */
            reader_unlock();
            printf("Thread %ld leu!\n", id);
            read++;
        } else if (op < 99) {  // operação de inserção
            printf("Thread %ld inserindo.\n", id);
            writer_lock();
            Insert(i % MAX_VALUE, &head_p);  /* Ignore return value */
            writer_unlock();
            printf("Thread %ld inseriu %ld!\n", id, i % MAX_VALUE);
            in++;
        } else {  // operação de remoção
            printf("Thread %ld apagando.\n", id);
            writer_lock();
            Delete(i % MAX_VALUE, &head_p);  /* Ignore return value */
            writer_unlock();
            printf("Thread %ld apagou %ld!\n", id, i % MAX_VALUE);
            out++;
        }
    }

    // Registra a quantidade de operações realizadas por tipo
    printf("Thread %ld: inserções=%d remoções=%d leituras=%d\n", id, in, out, read);
    pthread_exit(NULL);
}

/*-----------------------------------------------------------------*/
int main(int argc, char* argv[]) {
    pthread_t *tid;
    double ini, fim, delta;

    // verifica se o número de threads foi passado na linha de comando
    if (argc < 2) {
        printf("Digite: %s <numero de threads>\n", argv[0]);
        return 1;
    }
    nthreads = atoi(argv[1]);

    // insere os primeiros elementos na lista
    for (int i = 0; i < QTDE_INI; i++) {
        Insert(i % MAX_VALUE, &head_p);  /* Ignore return value */
    }

    // aloca espaço de memória para o vetor de identificadores de threads no sistema
    tid = malloc(sizeof(pthread_t) * nthreads);
    if (tid == NULL) {
        printf("--ERRO: malloc()\n");
        return 2;
    }

    // tomada de tempo inicial
    GET_TIME(ini);

    // inicializa o mutex e as variáveis de condição
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond_escritor, NULL);
    pthread_cond_init(&cond_leitor, NULL);

    // cria as threads
    for (long int i = 0; i < nthreads; i++) {
        if (pthread_create(tid + i, NULL, tarefa, (void*) i)) {
            printf("--ERRO: pthread_create()\n");
            return 3;
        }
    }

    // aguarda as threads terminarem
    for (int i = 0; i < nthreads; i++) {
        if (pthread_join(*(tid + i), NULL)) {
            printf("--ERRO: pthread_join()\n");
            return 4;
        }
    }

    // tomada de tempo final
    GET_TIME(fim);
    delta = fim - ini;
    printf("Tempo: %lf\n", delta);

    // libera os mutexes e as variáveis de condição
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond_leitor);
    pthread_cond_destroy(&cond_escritor);

    // libera o espaço de memória do vetor de threads
    free(tid);

    // libera o espaço de memória da lista
    Free_list(&head_p);

    return 0;
}  /* main */
