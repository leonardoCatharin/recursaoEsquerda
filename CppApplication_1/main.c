#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>
#include <limits.h>
#include <errno.h>

//parametros
#define TAM_PRODUCAO 20
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
//Funções principais
void abrirArquivo(char * nomeArquivo);
void encontraNaoTerminais();
void encontraProducoes();
void verificaRecursaoIndireta();
void converteGramatica();
void printGramatica(char * nomeArq);
//Funções auxiliares: Reconhecimento de recursão indireta
void simplificaGramatica(int src, int indexPr, int des);
//Funções auxiliares: Tratamento de recursão à Esquerda
caracter criaCaracterLinha(char c);
producao criaProducaoAuxiliar(producao pr, caracter cr);
void addCaracterLinhaEmProducoes(int indexNT, caracter cr);
int CaracterLinhaJaCriado(char * ch);
void removeProducao(int indexNT, int indexPro);
producao criaProducaoLambda();
//Funções auxiliares
void printaAjuda();
int naoContido(char ch);
int naoContidoTerminal(char ch);
void copiaBuffer(int indexNT, char * buffer);
void splitProducoes(char * str, int i);
int retornaIndiceNt(char * c);

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
    abrirArquivo(arquivoEntrada);
    encontraNaoTerminais();
    encontraProducoes();
    verificaRecursaoIndireta();
    converteGramatica();
    printGramatica(arquivoSaida);
    fclose(file);
    free(arquivoEntrada);
}

//Funções principais

void abrirArquivo(char * nomeArquivo) {
    if ((file = fopen(nomeArquivo, "r")) == NULL) {
        printf("Arquivo de entrada não encontrado!\n");
        exit(EXIT_FAILURE);
    }
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
    } while (!feof(file));
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
    } while (!feof(file));
    conjuntoT[posConjT++] = '$';
}

void verificaRecursaoIndireta() {
    for (int i = 0; i < posConjNT; i++) {
        for (int j = 0; j < conjuntoNT[i].incProducao; j++) {
            if (isupper(conjuntoNT[i].producao[j].p[0].c[0])) {
                int indexNT = retornaIndiceNt(conjuntoNT[i].producao[j].p[0].c);

                for (int k = 0; k < conjuntoNT[indexNT].incProducao; k++) {
                    if (conjuntoNT[indexNT].producao[k].p[0].c[0]
                            == conjuntoNT[i].caracter.c[0] && conjuntoNT[i].caracter.c[0] != conjuntoNT[indexNT].caracter.c[0]) {
                        simplificaGramatica(i, k, indexNT);
                    }
                }
            }
        }
    }
}

void converteGramatica() {
    for (int i = 0; i < posConjNT; i++) {
        int jaAddCaracterLinha = 0;
        int addLambda = 0;
        caracter cr = criaCaracterLinha(conjuntoNT[i].caracter.c[0]);
        for (int j = 0; j < conjuntoNT[i].incProducao; j++) {
            if (strcmp(conjuntoNT[i].caracter.c, conjuntoNT[i].producao[j].p[0].c) == 0) {
                producao pr = criaProducaoAuxiliar(conjuntoNT[i].producao[j], cr);
                addLambda = 1;
                if (!jaAddCaracterLinha) {
                    addCaracterLinhaEmProducoes(i, cr);
                    jaAddCaracterLinha = 1;
                }

                //verifica se o caracter já foi criado
                int index = CaracterLinhaJaCriado(cr.c);
                if (index > -1) {
                    conjuntoNT[index].producao[conjuntoNT[index].incProducao] = pr;
                    conjuntoNT[index].incProducao++;

                } else {
                    //CARACTER INSERIDO AQUI
                    conjuntoNT[posConjNT].caracter = cr;
                    conjuntoNT[posConjNT].producao[conjuntoNT[posConjNT].incProducao] = pr;
                    conjuntoNT[posConjNT].incProducao++;
                    posConjNT++;
                }
                removeProducao(i, j);
            }
        }
        if (addLambda) {
            producao pro = criaProducaoLambda();
            int index = retornaIndiceNt(cr.c);
            conjuntoNT[index].producao[conjuntoNT[index].incProducao] = pro;
            conjuntoNT[index].incProducao++;
        }
    }
}

void printGramatica(char * nomeArq) {

    if ((file = fopen(nomeArq, "w")) == NULL) {
        printf("Falha na abertura do arquivo");
    }

    for (int i = 0; i < posConjNT; i++) {
        fprintf(file, "%s -> ", conjuntoNT[i].caracter.c);
        for (int j = 0; j < conjuntoNT[i].incProducao; j++) {
            for (int k = 0; k < conjuntoNT[i].producao[j].incCaracter; k++) {
                fprintf(file, "%s", conjuntoNT[i].producao[j].p[k].c);
            }
            if (conjuntoNT[i].producao[j].incCaracter > 0 && j < conjuntoNT[i].incProducao - 1) {
                fprintf(file, "|");
            }
        }

        fprintf(file, " \n");
    }
}

//Funções auxiliares: Reconhecimento de recursão indireta

void simplificaGramatica(int src, int indexPr, int des) {
    //salvaProducoesPosIndexPr
    producao vetProducoesSalvas[50];
    int contVet = 0;
    for (int i = indexPr + 1; i < conjuntoNT[des].incProducao; i++) {
        vetProducoesSalvas[contVet] = conjuntoNT[des].producao[i];
        contVet++;
    }
    conjuntoNT[des].incProducao = indexPr;

    //remove o simbolo não terminal src
    for (int j = 0; j < conjuntoNT[des].producao[indexPr].incCaracter - 1; j++) {
        strcpy(conjuntoNT[des].producao[indexPr].p[j].c,
                conjuntoNT[des].producao[indexPr].p[j + 1].c);
    }
    conjuntoNT[des].producao[indexPr].incCaracter--;

    producao prAux = conjuntoNT[des].producao[indexPr];

    for (int i = 0; i < conjuntoNT[src].incProducao; i++) {
        producao pro;
        pro.incCaracter = 0;

        for (int j = 0; j < conjuntoNT[src].producao[i].incCaracter; j++) {
            strcpy(pro.p[pro.incCaracter].c, conjuntoNT[src].producao[i].p[j].c);
            pro.incCaracter++;
        }

        for (int j = 0; j < prAux.incCaracter; j++) {
            strcpy(pro.p[pro.incCaracter].c, prAux.p[j].c);
            pro.incCaracter++;
        }

        conjuntoNT[des].producao[conjuntoNT[des].incProducao] = pro;
        conjuntoNT[des].incProducao++;
    }

    for (int i = 0; i < contVet; i++) {
        conjuntoNT[des].producao[conjuntoNT[des].incProducao] = vetProducoesSalvas[i];
        conjuntoNT[des].incProducao++;
    }
}

//Funções auxiliares: Tratamento de recursão à Esquerda

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

void addCaracterLinhaEmProducoes(int indexNT, caracter cr) {
    for (int i = 0; i < conjuntoNT[indexNT].incProducao; i++) {
        if (conjuntoNT[indexNT].producao[i].p[0].c[0] !=
                cr.c[0] && conjuntoNT[indexNT].producao[i].p[0].c[0] != LAMBDA) {
            strcpy(conjuntoNT[indexNT].producao[i].p[
                    conjuntoNT[indexNT].producao[i].incCaracter].c,
                    cr.c);
            conjuntoNT[indexNT].producao[i].incCaracter++;
        }
    }
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

void removeProducao(int indexNT, int indexPro) {
    conjuntoNT[indexNT].producao[indexPro].incCaracter = 0;
}

producao criaProducaoLambda() {
    producao toReturn;

    toReturn.incCaracter = 0;

    toReturn.p[toReturn.incCaracter].c[0] = LAMBDA;
    toReturn.p[toReturn.incCaracter].c[1] = '\0';
    toReturn.incCaracter++;

    return toReturn;
}

//Funções Auxiliares

void printaAjuda() {
    printf("****\n");
    printf("Para passar como parâmetro o arquivo de entrada, utilize o seguinte parâmetro:\n");
    printf("\t -i <nome_arquivo>\n");
    printf("Para passar como parâmetro o arquivo de saída, utilize o seguinte parâmetro:\n");
    printf("\t -o <nome_arquivo>\n");
    printf("****\n");
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

int naoContidoTerminal(char ch) {
    int i;
    for (i = 0; i < posConjT; i++) {
        if (conjuntoT[i] == ch) {
            return 0;
        }
    }
    return 1;
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

int retornaIndiceNt(char * c) {
    for (int i = 0; i < posConjNT; i++) {
        if (strcmp(c, conjuntoNT[i].caracter.c) == 0) {
            return i;
        }
    }
    return -1;
}