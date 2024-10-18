#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <string.h>

#define N 10  // Tamanho do buffer de leitura
#define BUFFER_SIZE 1024  // Tamanho máximo dos buffers
#define NTHREADS 3  // Número de threads

char buffer1[N];         // Buffer compartilhado entre thread 1 e 2
char buffer2[BUFFER_SIZE]; // Buffer compartilhado entre thread 2 e 3
int buffer2_index = 0;   // Índice de escrita em buffer2

sem_t empty1, full1;     // Semáforos para controle do buffer1
sem_t empty2, full2;     // Semáforos para controle do buffer2

FILE *inputFile; // Ponteiro para o arquivo de entrada


// Thread 1: Lê os caracteres do arquivo e armazena no buffer1
void* thread1_func(void* arg) {
    while (1) {
        sem_wait(&empty1);  // Espera até que o buffer1 esteja vazio

        // Lê até N caracteres do arquivo e armazena em buffer1
        int bytesRead = fread(buffer1, 1, N, inputFile);
        if (bytesRead <= 0) {
            // Se o arquivo terminou, libera o semáforo para a thread 2 não ficar bloqueada
            sem_post(&full1);
            break;  // Sai do loop se não há mais dados
        }

        sem_post(&full1);  // Informa que o buffer1 está cheio

        if (bytesRead < N) {
            // Arquivo terminou de ser lido, saímos do loop
            break;
        }
    }

    pthread_exit(NULL); // Finaliza a thread
}


// Thread 2: Processa os caracteres do buffer1 e coloca no buffer2
void* thread2_func(void* arg) {
    int count = 1;  // Contador para controle de quantos caracteres processar antes de inserir \n
    int charsProcessedInLine = 0;  // Contador de caracteres processados na linha atual
    int n = 0;  // Controla o valor de n até o próximo \n (inicialmente 2n + 1)

    while (1) {
        sem_wait(&full1);  // Espera até que o buffer1 esteja cheio
        sem_wait(&empty2);  // Espera até que o buffer2 esteja vazio

        for (int i = 0; i < N; i++) {
            buffer2[buffer2_index++] = buffer1[i]; // Copia caracteres de buffer1 para buffer2
            charsProcessedInLine++; // Incrementa o contador de caracteres processados

            // Verifica se é o momento de inserir um \n
            if (n <= 10 && charsProcessedInLine == (2 * n + 1)) {
                buffer2[buffer2_index++] = '\n'; // Insere um nova linha
                n++;  // Incrementa o valor de n

                charsProcessedInLine = 0;  // Reseta o contador de caracteres da linha
            }
            // Quando n atinge 10, sempre insere \n a cada 10 caracteres
            else if (n > 10 && charsProcessedInLine == 10) {
                buffer2[buffer2_index++] = '\n'; // Insere uma nova linha
                charsProcessedInLine = 0;  // Reseta o contador de caracteres da linha
            }
        }

        sem_post(&empty1);  // Informa que o buffer1 está vazio
        sem_post(&full2);   // Informa que o buffer2 está cheio

        if (feof(inputFile)) {
            break; // Sai do loop se o arquivo foi lido completamente
        }
    }

    pthread_exit(NULL); // Finaliza a thread
}


// Thread 3: Imprime os caracteres do buffer2 na tela
void* thread3_func(void* arg) {
    while (1) {
        sem_wait(&full2);  // Espera até que o buffer2 esteja cheio

        // Imprime o conteúdo do buffer2
        for (int i = 0; i < buffer2_index; i++) {
            putchar(buffer2[i]); // Exibe cada caractere no buffer2
        }

        buffer2_index = 0;  // Reinicia o índice do buffer2

        sem_post(&empty2);  // Informa que o buffer2 está vazio

        if (feof(inputFile)) {
            break; // Sai do loop se o arquivo foi lido completamente
        }
    }

    pthread_exit(NULL); // Finaliza a thread
}


int main(int argc, char* argv[]) {
    // Identificadores das threads no sistema
    pthread_t *tid;

    if (argc < 2) {
        fprintf(stderr, "Uso: %s <arquivo de entrada>\n", argv[0]);
        return 1; // Erro se o arquivo de entrada não for especificado
    }

    // Abre o arquivo para leitura
    inputFile = fopen(argv[1], "r");
    if (!inputFile) {
        perror("Erro ao abrir o arquivo"); // Exibe erro se o arquivo não puder ser aberto
        return 1;
    }

    // Inicializa os semáforos
    sem_init(&empty1, 0, 1);  // Buffer1 começa vazio
    sem_init(&full1, 0, 0);   // Buffer1 começa vazio
    sem_init(&empty2, 0, 1);  // Buffer2 começa vazio
    sem_init(&full2, 0, 0);   // Buffer2 começa vazio

    // Aloca as estruturas para os identificadores de thread
    tid = (pthread_t*) malloc(sizeof(pthread_t)*(NTHREADS));
    if (tid == NULL) {
        puts("--ERRO: malloc()"); 
        return 2; // Erro na alocação de memória
    }

    // Cria as três threads
    if (pthread_create(&tid[0], NULL, thread1_func, NULL)) { 
        printf("--ERRO: pthread_create() thread1\n"); 
        exit(-1); 
    }

    if (pthread_create(&tid[1], NULL, thread2_func, NULL)) { 
        printf("--ERRO: pthread_create() thread2\n"); 
        exit(-1); 
    }

    if (pthread_create(&tid[2], NULL, thread3_func, NULL)) { 
        printf("--ERRO: pthread_create() thread3\n"); 
        exit(-1); 
    }

    // Espera todas as threads terminarem
    for (int t = 0; t < NTHREADS; t++) {
        if (pthread_join(tid[t], NULL)) {
            printf("--ERRO: pthread_join() \n");
            exit(-1); // Erro ao esperar a thread terminar
        }
    }

    // Destroi os semáforos
    sem_destroy(&empty1);
    sem_destroy(&full1);
    sem_destroy(&empty2);
    sem_destroy(&full2);

    // Fecha o arquivo
    fclose(inputFile);
    printf("\n");

    return 0; // Finaliza o programa com sucesso
}
