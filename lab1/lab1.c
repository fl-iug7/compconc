#include <stdio.h>
#include <stdlib.h> 
#include <pthread.h>



// Estrutura de dados para armazenar os argumentos da thread
typedef struct {
    int idThread, nThreads, tamVetor;
    int *vetor;
} t_Args;


// Função executada pelas threads
void *incrementa_vetor(void *arg) {
    t_Args *args = (t_Args *) arg;

    // Determina o intervalo de elementos que a thread irá processar
    int tamanho_por_thread = args->tamVetor / args->nThreads;
    int inicio = args->idThread * tamanho_por_thread;

    // Ajusta o fim para a última thread cobrir o resto do vetor
    int fim = inicio + tamanho_por_thread;
    if (args->idThread == args->nThreads - 1) {
        fim = args->tamVetor;
    }

    // Incrementa cada elemento da porção do vetor atribuída à thread
    for(int i = inicio; i < fim; i++) {
        args->vetor[i] += 1;
    }

    free(arg);
    pthread_exit(NULL);
}


// Função para inicializar o vetor
void inicializa_vetor(int *vetor, int N) {
    // Preenche o vetor com valores sequenciais
    for(int i = 0; i < N; i++) {
        vetor[i] = i; 
    }
}


// Função para verificar o resultado final
void verifica_vetor(int *vetor, int N) {
    for(int i = 0; i < N; i++) {
        if(vetor[i] != i + 1) {
            printf("--Erro no índice %d do vetor: esperado %d, obtido %d\n", i, i + 1, vetor[i]);
            return;
        }
    }

    printf("\nVetor validado!");
}


// Função para imprimir o vetor
void imprime_vetor(int *vetor, int N) {
    for(int i = 0; i < N; i++) {
        printf("%d ", vetor[i]);
    }

    printf("\n\n");
}


// Função principal do programa
int main(int argc, char* argv[]) {
    int M;
    int N;

    // Verifica se os argumentos 'quantidade de threads' e 'tamanho vetor' foram passados e armazena seus valores
    if(argc < 3) {
        printf("--ERRO: informe a quantidade de threads e o tamanho do vetor <%s> <M> <N>\n", argv[0]);
        return 1;
    }

    M = atoi(argv[1]); // Número de threads
    N = atoi(argv[2]); // Tamanho do vetor

    // Inicializa o vetor
    int vetor[N];
    inicializa_vetor(vetor, N);
    printf("Vetor Inicial: ");
    imprime_vetor(vetor, N); 

    // Identificadores das threads no sistema
    pthread_t tid_sistema[M];

    // Cria as threads
    for(int i = 0; i < M; i++) {
        printf("--Aloca e preenche argumentos para thread %d\n", i);

        t_Args *args = malloc(sizeof(t_Args));
        if (args == NULL) {
            printf("--ERRO: malloc()\n"); 
            return 1;
        }

        args->idThread = i;
        args->nThreads = M;
        args->tamVetor = N;
        args->vetor = vetor;

        printf("--Cria a thread %d\n", i);

        if (pthread_create(&tid_sistema[i], NULL, incrementa_vetor, (void*) args)) {
            printf("--ERRO: pthread_create()\n");
            return 2;
        }
    }

    // Espera todas as threads terminarem
    for (int i = 0; i < M; i++) {
        if (pthread_join(tid_sistema[i], NULL)) {
            printf("--ERRO: pthread_join() da thread %d\n", i);
        } 
    }

    // Verifica o resultado final
    verifica_vetor(vetor, N);
    printf("\nVetor Final: ");
    imprime_vetor(vetor, N); 

    // Log da função principal
    printf("Thread principal terminou.\n");

    return 0;
}
