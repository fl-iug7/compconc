#include <stdio.h>
#include <stdlib.h>
#include <time.h>


#define MAX 1000  // Define o valor máximo para a geração de números aleatórios


int main(int argc, char *argv[]) {
    // Verifica se o número de argumentos passados é suficiente
    if (argc < 3) {
        fprintf(stderr, "Uso: %s <dimensao> <arquivo_saida>\n", argv[0]);
        return 1;
    }

    // Lê a dimensão dos vetores e o nome do arquivo de saída a partir dos argumentos
    long int N = atol(argv[1]);  // Converte o argumento para o tipo long int
    char *arquivo_saida = argv[2];

    // Aloca memória para dois vetores de tamanho N
    float *vetor1 = (float *) malloc(sizeof(float) * N);
    float *vetor2 = (float *) malloc(sizeof(float) * N);

    if (!vetor1 || !vetor2) {
        fprintf(stderr, "Erro de alocação de memória\n");
        return 2;
    }

    // Inicializa o gerador de números aleatórios com base no tempo atual
    srand(time(NULL));

    // Gera os valores aleatórios para os vetores e calcula o produto interno
    double produto_interno = 0.0;
    for (long int i = 0; i < N; i++) {
        // Preenche os vetores com números aleatórios entre 0 e MAX/3.0
        vetor1[i] = (rand() % MAX) / 3.0;
        vetor2[i] = (rand() % MAX) / 3.0;
        // Calcula o produto interno acumulando o valor do produto dos elementos correspondentes
        produto_interno += vetor1[i] * vetor2[i];
    }

    // Abre o arquivo para escrita no formato binário
    FILE *arquivo = fopen(arquivo_saida, "wb");
    if (!arquivo) {
        fprintf(stderr, "Erro ao abrir o arquivo\n");
        free(vetor1);
        free(vetor2);
        return 3;
    }

    // Escreve a dimensão dos vetores, os vetores e o produto interno no arquivo
    fwrite(&N, sizeof(long int), 1, arquivo);            // Escreve a dimensão N
    fwrite(vetor1, sizeof(float), N, arquivo);           // Escreve o vetor 1
    fwrite(vetor2, sizeof(float), N, arquivo);           // Escreve o vetor 2
    fwrite(&produto_interno, sizeof(double), 1, arquivo);// Escreve o produto interno

    // Libera a memória alocada para os vetores e fecha o arquivo
    free(vetor1);
    free(vetor2);
    fclose(arquivo);

    // Reabre o arquivo para leitura
    arquivo = fopen(arquivo_saida, "rb");
    if (!arquivo) {
        fprintf(stderr, "Erro ao abrir o arquivo para leitura\n");
        return 4;
    }

    // Lê a dimensão dos vetores, os vetores e o produto interno do arquivo
    fread(&N, sizeof(long int), 1, arquivo);

    vetor1 = (float *) malloc(sizeof(float) * N);
    vetor2 = (float *) malloc(sizeof(float) * N);
    fread(vetor1, sizeof(float), N, arquivo);
    fread(vetor2, sizeof(float), N, arquivo);
    fread(&produto_interno, sizeof(double), 1, arquivo);

    // Imprime os vetores e o produto interno lido do arquivo
    printf("Dimensão N: %ld\n", N);
    printf("Vetor 1: ");
    for (long int i = 0; i < N; i++) {
        printf("%f ", vetor1[i]);
    }
    printf("\nVetor 2: ");
    for (long int i = 0; i < N; i++) {
        printf("%f ", vetor2[i]);
    }
    printf("\nProduto interno: %lf\n", produto_interno);

    // Libera a memória alocada e fecha o arquivo
    free(vetor1);
    free(vetor2);
    fclose(arquivo);

    return 0;
}
