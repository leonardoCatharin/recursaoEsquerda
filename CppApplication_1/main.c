#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>

//parametros
#define TAM_PRODUCAO 10
#define TAM_CONJUNTO 50
#define DELIMITADOR '|'
#define  LAMBDA 'e'
#define FIM_STRING '$'

//estruturas

typedef struct {
    char p[TAM_PRODUCAO];
} producao;

typedef struct {
    char c;
    producao producao[TAM_CONJUNTO];
    int incProducao;
} nt;

//globais
FILE * file;
nt conjuntoNT[TAM_CONJUNTO];
char conjuntoT[TAM_CONJUNTO];
int posConjNT = 0;
int posConjT = 0;

//assinatura de funções
void abrirArquivo(char * nomeArquivo);
int naoContido(char ch);
void encontraNaoTerminais();
void encontraProducoes();
void splitProducoes(char * str, int i);
int naoContidoTerminal(char ch);
void printaAjuda();
void printGramatica();

int main(int argc, char * argv[]) {

    int option = -1;
    char* arquivoEntrada = NULL;
    char* arquivoSaida = NULL;

    while ((option = getopt(argc, argv, "i:o:h")) != -1) {
        switch (option) {
            case 'i':
                arquivoEntrada = malloc(strlen(strdup(optarg)) + 1);
                strcpy(arquivoEntrada, strdup(optarg));
                break;
            case 'o':
                arquivoSaida = malloc(strlen(strdup(optarg)) + 1);
                strcpy(arquivoSaida, strdup(optarg));
                break;
            case 'h':
                printaAjuda();
                exit(0);
                break;
            default:
                printaAjuda();
                break;
        }
    }
    if (arquivoEntrada == NULL) {
        printf("\nArquivo de entrada não especificado, favor passar como parâmetro. Caso precise de ajuda, utilize o parâmetro \"-h\"\n");
        exit(0);
    }
    if (arquivoSaida == NULL) {
        printf("\nArquivo de saída não especificado, favor passar como parâmetro. Caso precise de ajuda, utilize o parâmetro \"-h\"\n");
        exit(0);
    }
    // if(arquivoEntrada != NULL && arquivoSaida != NULL){
    abrirArquivo(arquivoEntrada);
    encontraNaoTerminais();
    encontraProducoes();
    fclose(file);
    free(arquivoEntrada);
    printGramatica();
}

void printaAjuda() {
    printf("****\n");
    printf("Para passar como parâmetro o arquivo de entrada, utilize o seguinte parâmetro:\n");
    printf("\t -i <nome_arquivo>\n");
    printf("Para passar como parâmetro o arquivo de saída, utilize o seguinte parâmetro:\n");
    printf("\t -o <nome_arquivo>\n");
    printf("****\n");
}

void abrirArquivo(char * nomeArquivo) {
    if ((file = fopen(nomeArquivo, "r")) == NULL) {
        printf("Falha na abertura do arquivo");
    }
}

int naoContido(char ch) {
    int i;
    for (i = 0; i < posConjNT; i++) {
        if (conjuntoNT[i].c == ch) {
            return 0;
        }
    }

    return 1;
}

void encontraNaoTerminais() {
    int flag = 0;
    char ch = '\0';
    do {
        ch = fgetc(file);
        if (isupper(ch) && naoContido(ch)) {
            conjuntoNT[posConjNT++].c = ch;
        } else if (!isupper(ch) && naoContidoTerminal(ch) &&
                ch != '|' && ch != '-' && ch != '\n' &&
                ch != EOF && ch != LAMBDA && ch != ' ') {
            conjuntoT[posConjT++] = ch;
        }
        if (ch == '-' && naoContidoTerminal(ch)) {
            if (flag == 1) {
                conjuntoT[posConjT++] = ch;
            } else {
                flag = 1;
            }
        }
        if (ch == '\n') {
            flag = 0;
        }

    } while (ch != EOF);
    conjuntoT[posConjT++] = '$';
}

int naoContidoTerminal(char ch) {
    int i;
    for (i = 0; i < posConjT; i++) {
        if (conjuntoT[i] == ch) {
            return 0;
        }
    }
    return 1;
}

void encontraProducoes() {
    char ch = '\0';
    char cArquivo = '\0';
    char buffer[TAM_PRODUCAO];
    int incBuffer = 0;
    buffer[0] = '\0';
    int flag = 1;

    fseek(file, 0, SEEK_SET);

    do {
        if (flag) {
            cArquivo = fgetc(file);
            flag = 0;
        }
        if (fgetc(file) == '-') {

            flag = 1;

            for (int i = 0; i < posConjNT; i++) {
                if (cArquivo == conjuntoNT[i].c) {
                    cArquivo = fgetc(file);
                    while (cArquivo != '\n') {
                        if (cArquivo != ' ') {
                            buffer[incBuffer] = cArquivo;
                            buffer[incBuffer + 1] = '\0';
                            incBuffer++;
                        }
                        cArquivo = fgetc(file);
                    }
                    splitProducoes(buffer, i);

                    buffer[0] = '\0';
                    incBuffer = 0;
                }
            }
        }
    } while (cArquivo != EOF);
}

void splitProducoes(char* str, int i) {
    char buffer[50];
    buffer[0] = '\0';
    int index = 0;
    int incBuffer = 0;

    while (str[index] != '\0') {
        if (str[index] == DELIMITADOR) {
            strcpy(conjuntoNT[i].producao[conjuntoNT[i].incProducao].p, buffer);
            conjuntoNT[i].incProducao++;
            buffer[0] = '\0';
            incBuffer = 0;
        } else {
            buffer[incBuffer] = str[index];
            buffer[incBuffer + 1] = '\0';
            incBuffer++;
        }
        index++;
    }
    buffer[incBuffer] = '\0';
    strcpy(conjuntoNT[i].producao[conjuntoNT[i].incProducao].p, buffer);
    conjuntoNT[i].incProducao++;
}

void printGramatica() {
    for (int i = 0; i < posConjNT; i++) {
        printf("%c -> ", conjuntoNT[i].c);

        for (int j = 0; j < conjuntoNT[i].incProducao; j++) {
            printf("%s |", conjuntoNT[i].producao[j].p);
        }

        printf(" \n");
    }
}

int retornaIndiceNt(char c) {
    for (int i = 0; i < posConjNT; i++) {
        if (c == conjuntoNT[i].c) {
            return i;
        }
    }
    return -1;
}