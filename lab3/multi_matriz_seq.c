#include <stdio.h>
#include <stdlib.h>
#include <time.h>

float *mat1, *mat2, *mat3; // Ponteiros para as matrizes

// Função para inicializar a matriz a partir de um arquivo binário
void initMat(float **mat, int *rows, int *columns, char *fileName) {
    FILE *file;
    size_t ret;
    long long int size;

    // Abrindo o arquivo
    file = fopen(fileName, "rb");
    if (!file) {
        fprintf(stderr, "--ERRO: Abrir o arquivo %s\n", fileName);
        exit(-2);
    }

    // Lendo as dimensões da matriz
    ret = fread(rows, sizeof(int), 1, file);
    ret = fread(columns, sizeof(int), 1, file);
    if (ret < 1) {
        fprintf(stderr, "--ERRO: Ler as dimensões do arquivo %s\n", fileName);
        exit(-3);
    }

    // Calculando o tamanho e aloca memória para a matriz
    size = (*rows) * (*columns);
    *mat = (float *)malloc(sizeof(float) * size);
    if (!*mat) {
        fprintf(stderr, "--ERRO: Alocar memória para a matriz\n");
        exit(-4);
    }

    // Lendo os elementos da matriz
    ret = fread(*mat, sizeof(float), size, file);
    if (ret < size) {
        fprintf(stderr, "--ERRO: Ler os elementos da matriz do arquivo %s\n", fileName);
        exit(-5);
    }

    fclose(file);
}

// Função principal
int main(int argc, char *argv[]) {
    int R1, C1, R2, C2;          // Dimensões das matrizes
    long long int resultSize;    // Tamanho da matriz resultante
    FILE *fileOut;               // Ponteiro para o arquivo de saída
    size_t ret;                  // Retorno da leitura/escrita
    struct timespec start, end;  // Medição de tempo
    double delta;

    // Verificando se os parâmetros estão corretos
    if (argc < 4) {
        fprintf(stderr, "Uso: %s <arquivo_matriz_1> <arquivo_matriz_2> <arquivo_saida>\n", argv[0]);
        return -1;
    }

    // Inicializando as matrizes 1 e 2 a partir dos arquivos binários
    clock_gettime(CLOCK_MONOTONIC, &start);
    initMat(&mat1, &R1, &C1, argv[1]);
    initMat(&mat2, &R2, &C2, argv[2]);

    // Verificando se as matrizes podem ser multiplicadas
    if (C1 != R2) {
        fprintf(stderr, "Erro: Número de colunas da primeira matriz (%d) deve ser igual ao número de linhas da segunda matriz (%d)\n", C1, R2);
        return -6;
    }

    // Alocando memória para a matriz resultante
    resultSize = R1 * C2;
    mat3 = (float *)malloc(sizeof(float) * resultSize);
    if (!mat3) {
        fprintf(stderr, "--ERRO: Alocar memória para a matriz resultante\n");
        return -4;
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    delta = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1.0e9;
    printf("Tempo de preparação: %lf segundos\n", delta);

    // Multiplicação de matrizes
    clock_gettime(CLOCK_MONOTONIC, &start);
    for (int i = 0; i < R1; i++) {
        for (int j = 0; j < C2; j++) {
            mat3[i * C2 + j] = 0;
            for (int k = 0; k < C1; k++) {
                mat3[i * C2 + j] += mat1[i * C1 + k] * mat2[k * C2 + j];
            }
        }
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    delta = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1.0e9;
    printf("Tempo de processamento (multiplicação): %lf segundos\n", delta);

    // Escrevendo a matriz resultante no arquivo de saída
    clock_gettime(CLOCK_MONOTONIC, &start);
    fileOut = fopen(argv[3], "wb");
    if (!fileOut) {
        fprintf(stderr, "--ERRO: Abrir o arquivo de saída %s\n", argv[3]);
        return -3;
    }

    ret = fwrite(&R1, sizeof(int), 1, fileOut);
    ret = fwrite(&C2, sizeof(int), 1, fileOut);
    ret = fwrite(mat3, sizeof(float), resultSize, fileOut);

    if (ret < resultSize) {
        fprintf(stderr, "--ERRO: Escrever a matriz resultante no arquivo\n");
        return -7;
    }

    // Liberando a memória alocada e fecha o arquivo de saída
    fclose(fileOut);
    free(mat1);
    free(mat2);
    free(mat3);

    clock_gettime(CLOCK_MONOTONIC, &end);
    delta = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1.0e9;
    printf("Tempo de finalização: %lf segundos\n", delta);

    return 0;
}
