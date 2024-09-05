#define _POSIX_C_SOURCE 199309L // Para clock_gettime

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

float *mat1, *mat2, *result; // Matrizes
int nThreads;                // Número de threads

// Estrutura para passar parâmetros para as threads
typedef struct {
    int id;
    int R1, R2, C2;
} tArgs;

// Função executadas pelas threads
void *multMat(void *arg) {
    tArgs *args = (tArgs *)arg;

    for (int i = args->id; i < args->R1; i += nThreads) {
        for (int j = 0; j < args->C2; j++) {
            result[i * args->C2 + j] = 0;
            for (int k = 0; k < args->R2; k++) {
                result[i * args->C2 + j] += mat1[i * args->R2 + k] * mat2[k * args->C2 + j];
            }
        }
    }
    
    pthread_exit(NULL);
}

// Inicializa a matriz a partir de um arquivo
void initMat(float **mat, int *rows, int *columns, char *filename) {
    long long size;
    size_t ret;
    FILE *file;

    // Abrindo do arquivo
    file = fopen(filename, "rb");
    if (!file) {
        fprintf(stderr, "--ERRO: Não foi possível abrir o arquivo %s\n", filename);
        exit(-1);
    }

    // Lendo as dimensões da matriz
    ret = fread(rows, sizeof(int), 1, file);
    ret = fread(columns, sizeof(int), 1, file);
    size = (*rows) * (*columns);

    // Alocação de memória
    *mat = (float *)malloc(size * sizeof(float));
    if (!*mat) {
        fprintf(stderr, "--ERRO: Falha ao alocar memória\n");
        exit(-2);
    }

    // Lendo os dados da matriz
    ret = fread(*mat, sizeof(float), size, file);
    if (ret < size) {
        fprintf(stderr, "--ERRO: Leitura incompleta da matriz\n");
        exit(-3);
    }

    fclose(file);
}

// Função principal
int main(int argc, char *argv[]) {
    int R1, C1, R2, C2;
    long long resultSize;
    FILE *file;
    tArgs *args;
    pthread_t *threads;
    size_t ret;
    struct timespec start, end;
    double elapsedTime;

    // Verificando os argumentos
    if (argc < 5) {
        fprintf(stderr, "Uso: %s <arquivo_entrada_1> <arquivo_entrada_2> <arquivo_saida> <num_threads>\n", argv[0]);
        return -1;
    }

    // Lendo as matrizes dos arquivos
    clock_gettime(CLOCK_MONOTONIC, &start);
    initMat(&mat1, &R1, &C1, argv[1]);
    initMat(&mat2, &R2, &C2, argv[2]);
    nThreads = atoi(argv[4]);

    // Verificando a compatibilidade das matrizes
    if (C1 != R2) {
        fprintf(stderr, "--ERRO: Dimensões incompatíveis entre as matrizes\n");
        return -2;
    }

    // Alocando de memória para o resultado
    resultSize = R1 * C2;
    result = (float *)malloc(resultSize * sizeof(float));
    if (!result) {
        fprintf(stderr, "--ERRO: Falha ao alocar memória para o resultado\n");
        return -3;
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsedTime = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1.0e9;
    printf("Tempo de inicialização: %f segundos\n", elapsedTime);

    // Alocando de memória para as threads e seus argumentos
    threads = (pthread_t *)malloc(nThreads * sizeof(pthread_t));
    args = (tArgs *)malloc(nThreads * sizeof(tArgs));

    // Criando as threads
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < nThreads; i++) {
        args[i].id = i;
        args[i].R1 = R1;
        args[i].R2 = R2;
        args[i].C2 = C2;

        if (pthread_create(&threads[i], NULL, multMat, (void *)&args[i])) {
            fprintf(stderr, "--ERRO: Falha ao criar thread\n");
            return -4;
        }
        
    }

    // Esperando a finalização das threads
    for (int i = 0; i < nThreads; i++) {
        pthread_join(threads[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsedTime = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1.0e9;
    printf("Tempo de processamento: %f segundos\n", elapsedTime);

    // Escrita do resultado em arquivo
    clock_gettime(CLOCK_MONOTONIC, &start);
    file = fopen(argv[3], "wb");
    ret = fwrite(&R1, sizeof(int), 1, file);
    ret = fwrite(&C2, sizeof(int), 1, file);
    ret = fwrite(result, sizeof(float), resultSize, file);
    if (ret < resultSize) {
        fprintf(stderr, "--ERRO: Falha ao escrever no arquivo de saída\n");
        return -5;
    }

    fclose(file);

    clock_gettime(CLOCK_MONOTONIC, &end);
    elapsedTime = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1.0e9;
    printf("Tempo de finalização: %f segundos\n", elapsedTime);

    // Liberação de memória
    free(mat1);
    free(mat2);
    free(result);
    free(args);
    free(threads);

    return 0;
}
