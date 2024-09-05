#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define TEXTO 

int main(int argc, char *argv[]) {
    float *matriz;           // Ponteiro para a matriz
    int linhas, colunas;     // Dimensões da matriz
    long long int tam;       // Quantidade de elementos na matriz
    FILE *arquivoSaida;      // Ponteiro para o arquivo de saída
    size_t ret;              // Variável para verificar sucesso na escrita

    // Verificando se os argumentos foram passados corretamente
    if (argc < 4) {
        fprintf(stderr, "Uso: %s <linhas> <colunas> <arquivo_saida>\n", argv[0]);
        return 1;
    }

    // Recebendo as dimensões da matriz
    linhas = atoi(argv[1]);
    colunas = atoi(argv[2]);
    tam = linhas * colunas;

    // Alocando memória para a matriz
    matriz = (float *)malloc(sizeof(float) * tam);
    if (!matriz) {
        fprintf(stderr, "ERRO: Alocar memória\n");
        return 2;
    }

    // Gerando valores aleatórios para a matriz
    srand(time(NULL));
    for (long long int i = 0; i < tam; i++) {
        matriz[i] = ((rand() % 1000) * 0.1);  // Gera valores float entre 0.0 e 99.9
    }

    // Exibindo a matriz
    #ifdef TEXTO
    for (int i = 0; i < linhas; i++) {
        for (int j = 0; j < colunas; j++)
        {
            printf("%.2f ", matriz[i * colunas + j]);
        }
        printf("\n");
    }
    #endif

    // Abrindo o arquivo de saída para escrita binária
    arquivoSaida = fopen(argv[3], "wb");
    if (!arquivoSaida) {
        fprintf(stderr, "--ERRO: Abrir arquivo de saída\n");
        free(matriz);
        return 3;
    }

    // Escrevendo as dimensões da matriz no arquivo
    ret = fwrite(&linhas, sizeof(int), 1, arquivoSaida);
    ret = fwrite(&colunas, sizeof(int), 1, arquivoSaida);

    // Escrevendo os valores da matriz no arquivo
    ret = fwrite(matriz, sizeof(float), tam, arquivoSaida);
    if (ret < tam) {
        fprintf(stderr, "ERRO: Escrever no arquivo\n");
        fclose(arquivoSaida);
        free(matriz);
        return 4;
    }

    // Fechando o arquivo e libera a memória alocada
    fclose(arquivoSaida);
    free(matriz);

    return 0;
}
