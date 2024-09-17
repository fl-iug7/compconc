#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

// Variáveis globais
pthread_mutex_t mutex; // Mutex para garantir exclusão mútua
pthread_cond_t cond;  // Variável de condição para sincronização entre threads
long int soma = 0;    // Variável compartilhada que será incrementada
int count = 0;        // Contador dos múltiplos de 10 processados
int print_flag = 0;   // Flag para indicar quando a soma é múltiplo de 10
long int last_number = 0;  // Variável de condição para o print dos valores entre threads


// Função executada pelas threads de trabalho
void *ExecutaTarefa(void *arg) {
    long int id = (long int) arg;
    printf("Thread : %ld esta executando...\n", id);

    while (count < 20) {
        // Verifica se a soma é múltiplo de 10
        if (soma % 10 == 0) {
            pthread_mutex_lock(&mutex); // Adquire o mutex
            print_flag = 1; // Sinaliza que a soma é múltiplo de 10 e precisa ser impressa

            // Espera até que a soma seja impressa e o print_flag seja resetado
            while (print_flag) {
                pthread_cond_wait(&cond, &mutex); // Espera pela condição para continuar

                // Sinaliza todas as threads se o limite foi atingido
                if (count >= 20) {
                    pthread_cond_broadcast(&cond);
                }
            }

            pthread_mutex_unlock(&mutex); // Libera o mutex
        }

        soma++; // Incrementa a variável soma
    }

    printf("Thread : %ld terminou!\n", id);
    pthread_exit(NULL);
}


// Função executada pela thread de log
void *extra(void *args) {
    printf("Extra : esta executando...\n");

    while (count < 20) {
        // Verifica se a soma deve ser impressa
        if (print_flag) {
            pthread_mutex_lock(&mutex); // Adquire o mutex
            
            if (soma != last_number || soma > last_number) {
                printf("soma = %ld \n", soma); // Imprime o valor de soma
                count++; // Incrementa o contador   
                last_number = soma; // Atualiza a flag
            }

            print_flag = 0; // Reseta a flag

            pthread_cond_signal(&cond); // Sinaliza uma thread esperando pela condição

            pthread_mutex_unlock(&mutex); // Libera o mutex
        }

        // Sinaliza todas as threads se o contador atingir o limite
        if (count >= 20) {
            pthread_cond_broadcast(&cond); 
        }
    }

    printf("Extra : terminou!\n");
    pthread_exit(NULL); 
}


// Fluxo principal
int main(int argc, char *argv[]) {
    pthread_t *tid; // identificadores das threads no sistema
    int nthreads;   // quantidade de threads (passada linha de comando)

    // Lê e avalia os parâmetros de entrada
    if (argc < 2) {
        printf("Digite: %s <numero de threads>\n", argv[0]);
        return 1;
    }

    nthreads = atoi(argv[1]);

    // Aloca as estruturas
    tid = (pthread_t *) malloc(sizeof(pthread_t) * (nthreads + 1));
    if (tid == NULL) {
        puts("ERRO--malloc");
        return 2;
    }

    // Inicializa o mutex (lock de exclusão mútua) e a condição
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    // Cria as threads
    for (long int t = 0; t < nthreads; t++) {
        if (pthread_create(&tid[t], NULL, ExecutaTarefa, (void *)t)) {
            printf("--ERRO: pthread_create()\n");
            exit(-1);
        }
    }

    // Cria thread de log
    if (pthread_create(&tid[nthreads], NULL, extra, NULL)) {
        printf("--ERRO: pthread_create()\n");
        exit(-1);
    }

    // Espera todas as threads terminarem
    for (int t = 0; t < nthreads + 1; t++) {
        if (pthread_join(tid[t], NULL)) {
            printf("--ERRO: pthread_join() \n");
            exit(-1);
        }
    }

    // Finaliza o mutex e a condição
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    printf("Valor de 'soma' = %ld\n", last_number);

    return 0;
}
