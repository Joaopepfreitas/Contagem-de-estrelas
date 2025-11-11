#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mpi.h"

// Estrutura para representar a imagem PGM
typedef struct {
    int largura;
    int altura;
    int valor_maximo;
    int *dados; // Usamos um array 1D para facilitar o envio com MPI
} ImagemPGM;

// Prototipos de funções
ImagemPGM* ler_pgm_p2(const char *caminho_arquivo);
void liberar_imagem_pgm(ImagemPGM *imagem);
int contar_estrelas(int *dados, int largura, int altura, int limiar);

// O código mestre
void mestre(int num_processos, const char *caminho_arquivo) {
    ImagemPGM *imagem_completa = ler_pgm_p2(caminho_arquivo);
    if (!imagem_completa) {
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    int total_estrelas = 0;
    int altura_por_processo = imagem_completa->altura / (num_processos - 1);
    int sobra_altura = imagem_completa->altura % (num_processos - 1);

    int offset_dados = 0;
    int tag_dados = 1;
    int tag_contagem = 2;
    int limiar_estrelas = 100; // Limiar de brilho para considerar um pixel como estrela

    // Envia pedaços da imagem para cada escravo (rank 1 a n-1)
    for (int i = 1; i < num_processos; i++) {
        int altura_fatia = altura_por_processo;
        if (i == num_processos - 1) { // Adiciona a sobra ao último processo
            altura_fatia += sobra_altura;
        }
        int tamanho_fatia = imagem_completa->largura * altura_fatia;

        MPI_Send(&imagem_completa->largura, 1, MPI_INT, i, tag_dados, MPI_COMM_WORLD);
        MPI_Send(&altura_fatia, 1, MPI_INT, i, tag_dados, MPI_COMM_WORLD);
        MPI_Send(&limiar_estrelas, 1, MPI_INT, i, tag_dados, MPI_COMM_WORLD);
        MPI_Send(imagem_completa->dados + offset_dados, tamanho_fatia, MPI_INT, i, tag_dados, MPI_COMM_WORLD);

        offset_dados += tamanho_fatia;
    }
    
    // Recebe a contagem de estrelas de cada escravo
    for (int i = 1; i < num_processos; i++) {
        int contagem_parcial;
        MPI_Recv(&contagem_parcial, 1, MPI_INT, i, tag_contagem, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        total_estrelas += contagem_parcial;
    }

    printf("Contagem total de estrelas: %d\n", total_estrelas);

    liberar_imagem_pgm(imagem_completa);
}

// O código escravo
void escravo() {
    int largura, altura_fatia, limiar;
    int tag_dados = 1;
    int tag_contagem = 2;
    
    // Recebe os metadados da fatia de imagem
    MPI_Recv(&largura, 1, MPI_INT, 0, tag_dados, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv(&altura_fatia, 1, MPI_INT, 0, tag_dados, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv(&limiar, 1, MPI_INT, 0, tag_dados, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    int tamanho_fatia = largura * altura_fatia;
    int *dados_fatia = (int*) malloc(tamanho_fatia * sizeof(int));
    
    if (!dados_fatia) {
        perror("Erro de alocação de memória no escravo");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    
    // Recebe os dados da fatia
    MPI_Recv(dados_fatia, tamanho_fatia, MPI_INT, 0, tag_dados, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    
    // Conta as estrelas na sua fatia
    int contagem = contar_estrelas(dados_fatia, largura, altura_fatia, limiar);
    
    // Envia a contagem de volta para o mestre
    MPI_Send(&contagem, 1, MPI_INT, 0, tag_contagem, MPI_COMM_WORLD);
    
    free(dados_fatia);
}

// Funções de suporte

// Implementação da leitura PGM P2
ImagemPGM* ler_pgm_p2(const char *caminho_arquivo) {
    FILE *arquivo = fopen(caminho_arquivo, "r");
    if (!arquivo) {
        perror("Erro ao abrir o arquivo");
        return NULL;
    }

    char magic_number[3];
    fscanf(arquivo, "%2s", magic_number);
    if (strcmp(magic_number, "P2") != 0) {
        fprintf(stderr, "Erro: O arquivo não está no formato PGM P2.\n");
        fclose(arquivo);
        return NULL;
    }

    char c;
    while ((c = fgetc(arquivo)) == '\n' || c == '#') {
        if (c == '#') {
            while (fgetc(arquivo) != '\n');
        }
    }
    fseek(arquivo, -1, SEEK_CUR);

    ImagemPGM *imagem = (ImagemPGM*) malloc(sizeof(ImagemPGM));
    if (!imagem) {
        fclose(arquivo);
        perror("Erro de alocação de memória para a imagem");
        return NULL;
    }
    
    fscanf(arquivo, "%d %d", &imagem->largura, &imagem->altura);
    fscanf(arquivo, "%d", &imagem->valor_maximo);

    int total_pixels = imagem->largura * imagem->altura;
    imagem->dados = (int*) malloc(total_pixels * sizeof(int));
    if (!imagem->dados) {
        fclose(arquivo);
        free(imagem);
        perror("Erro de alocação de memória para os pixels");
        return NULL;
    }

    for (int i = 0; i < total_pixels; i++) {
        fscanf(arquivo, "%d", &imagem->dados[i]);
    }

    fclose(arquivo);
    return imagem;
}

// Libera a memória da imagem
void liberar_imagem_pgm(ImagemPGM *imagem) {
    if (imagem) {
        free(imagem->dados);
        free(imagem);
    }
}

// Funcao para contar estrelas na fatia
int contar_estrelas(int *dados, int largura, int altura, int limiar) {
    int contagem = 0;
    for (int i = 0; i < largura * altura; i++) {
        if (dados[i] > limiar) {
            contagem++;
        }
    }
    return contagem;
}

// Funcao principal
int main(int argc, char *argv[]) {
    int rank, num_processos;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &num_processos);

    if (num_processos < 2) {
        fprintf(stderr, "Erro: Este programa requer pelo menos 2 processos (1 mestre, 1 escravo).\n");
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    
    if (rank == 0) {
        if (argc != 2) {
            fprintf(stderr, "Uso: mpirun -np <num_processos> %s <caminho_para_imagem.pgm>\n", argv[0]);
            MPI_Abort(MPI_COMM_WORLD, 1);
        }
        mestre(num_processos, argv[1]);
    } else {
        escravo();
    }

    MPI_Finalize();
    return 0;
}