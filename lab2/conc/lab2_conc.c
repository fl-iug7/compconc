#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h> 


// Variáveis globais
long int n;        // Dimensão dos vetores
float *vetor1;     // Ponteiro para o primeiro vetor
float *vetor2;     // Ponteiro para o segundo vetor
double *resultados_parciais; // Array para armazenar os resultados parciais de cada thread
int nthreads;      // Número de threads

// Estrutura para os argumentos das threads
typedef struct {
    long int id;       // Id da thread
    long int inicio;   // Índice inicial para a thread
    long int fim;      // Índice final para a thread
} t_Args;


// Função executada por cada thread
void *calcula_produto_interno(void *arg) {
    t_Args *args = (t_Args *) arg;
    double soma_local = 0.0;

    for (long int i = args->inicio; i < args->fim; i++) {
        soma_local += vetor1[i] * vetor2[i];
    }

    // Armazena o resultado parcial no array global de resultados
    resultados_parciais[args->id] = soma_local;

    free(arg);
    pthread_exit(NULL);
}


int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Uso: %s <arquivo de entrada> <numero de threads>\n", argv[0]);
        return 1;
    }

    FILE *arq = fopen(argv[1], "rb");
    if (arq == NULL) {
        printf("--ERRO: fopen()\n");
        return 2;
    }

    // Lê a dimensão dos vetores
    fread(&n, sizeof(long int), 1, arq);

    // Aloca memória para os vetores
    vetor1 = (float *) malloc(sizeof(float) * n);
    vetor2 = (float *) malloc(sizeof(float) * n);
    if (vetor1 == NULL || vetor2 == NULL) {
        printf("--ERRO: malloc()\n");
        return 3;
    }

    // Lê os vetores do arquivo
    fread(vetor1, sizeof(float), n, arq);
    fread(vetor2, sizeof(float), n, arq);

    // Lê o valor do produto interno armazenado no arquivo
    double produto_interno_armazenado;
    fread(&produto_interno_armazenado, sizeof(double), 1, arq);
    fclose(arq);

    // Inicializa as threads
    nthreads = atoi(argv[2]);
    if (nthreads > n) {
        nthreads = n;
    }

    pthread_t *tid = (pthread_t *) malloc(sizeof(pthread_t) * nthreads);
    resultados_parciais = (double *) malloc(sizeof(double) * nthreads);

    long int bloco = n / nthreads;
    long int resto = n % nthreads;

    for (long int i = 0; i < nthreads; i++) {
        t_Args *args = malloc(sizeof(t_Args));
        args->id = i;
        args->inicio = i * bloco;
        args->fim = (i == nthreads - 1) ? (i + 1) * bloco + resto : (i + 1) * bloco;

        if (pthread_create(&tid[i], NULL, calcula_produto_interno, (void *) args)) {
            printf("--ERRO: pthread_create()\n");
            return 4;
        }
    }

    // Aguarda todas as threads finalizarem
    for (long int i = 0; i < nthreads; i++) {
        if (pthread_join(tid[i], NULL)) {
            printf("--ERRO: pthread_join()\n");
            return 5;
        }
    }

    // Soma os resultados parciais de cada thread
    double produto_interno_concorrente = 0.0;
    for (long int i = 0; i < nthreads; i++) {
        produto_interno_concorrente += resultados_parciais[i];
    }

    free(tid);
    free(vetor1);
    free(vetor2);
    free(resultados_parciais);

    // Exibe o produto interno calculado e compara com o valor armazenado
    printf("Produto interno concorrente: %.6lf\n", produto_interno_concorrente);
    printf("Produto interno armazenado:  %.6lf\n", produto_interno_armazenado);

    // Calcula a variação relativa
    double variacao_relativa = fabs((produto_interno_armazenado - produto_interno_concorrente) / produto_interno_armazenado);
    printf("Variação relativa: %.6lf\n", variacao_relativa);

    return 0;
}
