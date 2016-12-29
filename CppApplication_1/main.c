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
    char c[3];
} caracter;

typedef struct {
    caracter p[TAM_PRODUCAO];
    int incCaracter;
} producao;

typedef struct {
    caracter caracter;
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
int CaracterLinhaJaCriado(char * ch);
int naoContido(char ch);
void encontraNaoTerminais();
void encontraProducoes();
void splitProducoes(char * str, int i);
int naoContidoTerminal(char ch);
void printaAjuda();
void printGramatica();
void copiaBuffer(int indexNT, char * buffer);
caracter criaCaracterLinha(char c);
producao criaProducaoAuxiliar(producao pr, caracter cr);
void removerProducao(int indexNT, int indexPro);

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

    for (int i = 0; i < posConjNT; i++) {
        for (int j = 0; j < conjuntoNT[i].incProducao; j++) {
            if (strcmp(conjuntoNT[i].caracter.c, conjuntoNT[i].producao[j].p[0].c) == 0) {

                caracter cr = criaCaracterLinha(conjuntoNT[i].caracter.c[0]);
                producao pr = criaProducaoAuxiliar(conjuntoNT[i].producao[j], cr);
                
                //ver se o caracter linha já foi criado
                int index = CaracterLinhaJaCriado(cr.c);
                if (index > -1) {
                    conjuntoNT[index].producao[conjuntoNT[index].incProducao] = pr;
                    conjuntoNT[index].incProducao++;

                } else {
                    conjuntoNT[posConjNT].caracter = cr;
                    conjuntoNT[posConjNT].producao[conjuntoNT[posConjNT].incProducao] = pr;
                    conjuntoNT[posConjNT].incProducao++;
                    posConjNT++;
                }
                removeProducao(i, j);
            }
        }
    }

    fclose(file);
    free(arquivoEntrada);
    printGramatica();
}

caracter criaCaracterLinha(char c) {
    caracter cr;

    cr.c[0] = c;
    cr.c[1] = '\'';
    cr.c[2] = '\0';

    return cr;
}

producao criaProducaoAuxiliar(producao pr, caracter cr) {
    producao toReturn;

    toReturn.incCaracter = 0;

    for (int i = 1; i < pr.incCaracter; i++) {
        strcpy(toReturn.p[toReturn.incCaracter].c,
                pr.p[i].c);
        toReturn.incCaracter++;
    }

    strcpy(toReturn.p[toReturn.incCaracter].c,
            cr.c);
    toReturn.incCaracter++;

    return toReturn;
}

void removeProducao(int indexNT, int indexPro) {
    conjuntoNT[indexNT].producao[indexPro].incCaracter = 0;
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
        if (conjuntoNT[i].caracter.c[0] == ch) {
            return 0;
        }
    }

    return 1;
}

int CaracterLinhaJaCriado(char * ch) {
    int i;
    for (i = 0; i < posConjNT; i++) {
        if (strcmp(conjuntoNT[i].caracter.c, ch) == 0) {
            return i;
        }
    }

    return -1;
}

void encontraNaoTerminais() {
    int flag = 0;
    char ch = '\0';
    do {
        ch = fgetc(file);
        if (isupper(ch) && naoContido(ch)) {
            conjuntoNT[posConjNT++].caracter.c[0] = ch;
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
                if (cArquivo == conjuntoNT[i].caracter.c[0]) {
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

void copiaBuffer(int indexNT, char * buffer) {
    for (int k = 0; k < strlen(buffer); k++) {
        conjuntoNT[indexNT].producao[conjuntoNT[indexNT].incProducao]
                .p[conjuntoNT[indexNT].producao[conjuntoNT[indexNT].incProducao].incCaracter]
                .c[0] = buffer[k];

        conjuntoNT[indexNT].producao[conjuntoNT[indexNT].incProducao].incCaracter++;
    }
}

void splitProducoes(char* str, int i) {
    char buffer[50];
    buffer[0] = '\0';
    int index = 0;
    int incBuffer = 0;

    while (str[index] != '\0') {
        if (str[index] == DELIMITADOR) {
            copiaBuffer(i, buffer);
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
    copiaBuffer(i, buffer);
    conjuntoNT[i].incProducao++;
}

void printGramatica() {
    for (int i = 0; i < posConjNT; i++) {
        printf("%s -> ", conjuntoNT[i].caracter.c);
        for (int j = 0; j < conjuntoNT[i].incProducao; j++) {
            for (int k = 0; k < conjuntoNT[i].producao[j].incCaracter; k++) {
                printf("%s", conjuntoNT[i].producao[j].p[k].c);
            }
            printf("|");
        }

        printf(" \n");
    }
}

int retornaIndiceNt(char c) {
    for (int i = 0; i < posConjNT; i++) {
        if (c == conjuntoNT[i].caracter.c[0]) {
            return i;
        }
    }
    return -1;
}