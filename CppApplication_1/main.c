#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>

//parametros
#define tamProducao 10
#define tamConjuntos 50
#define tamConjuntoNT 100
#define tamConjuntoT 100
#define DELIMITADOR '|'
#define  LAMBDA 'e'
#define FIM_STRING '$'

//estruturas

typedef struct producao {
    char p[tamProducao];
};

typedef struct nt {
    char c;
    struct producao producao[tamConjuntos]; // produções de c
    int incProducao;
    char first[tamConjuntos];
    int incFirst;
    char follow[tamConjuntos];
    int incFollow;
};

//globais
FILE * file;
struct nt conjuntoNT[tamConjuntoNT];
char conjuntoT[tamConjuntoT];
struct producao matrizResultado[200][200];
int posConjNT = 0;
int posConjT = 0;
int backup = 0;
int backupFollow = 0;

//assinatura de funções
void abrirArquivo(char * nomeArquivo);
int naoContido(char ch);
void encontraNaoTerminais();
void encontraProducoes();
void splitProducoes(char * str, int i);
void first();
void printNt(char* nomeArq);
int retornaIndiceNt(char c);
int contem(int des, char c);
int contemVazio(int index);
int contemFollow(int des, char c);
void updateFirst(int NT, int posProducao);
void adicionaFirst(int des, int src);
void adicionaFollow(int des, int src);
void adicionaFirstnoFollow(int des, int src);
int somaFirsts();
int change();
void follow();
int somaFollow();
int changeFollow();
char* split(char* str, int index);
void criaTabela();
int contemFimString(int index);
void insereProducaoNaMatriz(int linha, int coluna, char* producao);
int retornaIndiceColunaTerminal(char c);
int naoContidoTerminal(char ch);

int main(int argc, char * argv[]) {
        
    int option = -1;
    char* arquivoEntrada    = NULL;
    char* arquivoSaida      = NULL;

    while ((option = getopt (argc, argv, "i:o:h")) != -1){
        switch (option){
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
    if(arquivoEntrada == NULL){
        printf("\nArquivo de entrada não especificado, favor passar como parâmetro. Caso precise de ajuda, utilize o parâmetro \"-h\"\n");
        exit(0);
    }
    if(arquivoSaida == NULL){
        printf("\nArquivo de saída não especificado, favor passar como parâmetro. Caso precise de ajuda, utilize o parâmetro \"-h\"\n");
        exit(0);
    }
    // if(arquivoEntrada != NULL && arquivoSaida != NULL){
    abrirArquivo(arquivoEntrada);
    encontraNaoTerminais();
    encontraProducoes();
    fclose(file);
    free(arquivoEntrada);
    first();
    follow();
    criaTabela();
    printNt(arquivoSaida);    
    
}

void printaAjuda(){
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
    char buffer[tamProducao];
    int incBuffer = 0;
    buffer[0] = '\0';

    fseek(file, 0, SEEK_SET);

    do {
        cArquivo = fgetc(file);
        if (fgetc(file) == '-') {
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

void first() {
    backup = -1;
    do {
        int NT = 0;
        while (NT <= posConjNT) {
            for (int i = 0; i < conjuntoNT[NT].incProducao; i++) { //para cada produção de NT
                if (!isupper(conjuntoNT[NT].producao[i].p[0])) {
                    if (contem(NT, conjuntoNT[NT].producao[i].p[0]) == 0) {
                        conjuntoNT[NT].first[conjuntoNT[NT].incFirst] = conjuntoNT[NT].producao[i].p[0];
                        conjuntoNT[NT].incFirst++;
                    }
                } else {
                    adicionaFirst(
                            NT,
                            retornaIndiceNt(conjuntoNT[NT].producao[i].p[0])
                            );

                    for (int i = 0; i < conjuntoNT[NT].incProducao; i++) {
                        updateFirst(NT, i);
                    }
                }
            }
            NT++;
        }
    } while (change() == 1);
}

int change() {
    int soma = somaFirsts();
    if (backup != soma) {
        backup = soma;
        return 1;
    }
    return 0;
}

int somaFirsts() {
    int soma = 0;
    for (int i = 0; i < posConjNT; i++) {
        soma = soma + strlen(conjuntoNT[i].first);
    }

    return soma;
}

void updateFirst(int NT, int posProducao) {
    int i;
    for (i = 0; i < strlen(conjuntoNT[NT].producao[posProducao].p); i++) {
        if (!isupper(conjuntoNT[NT].producao[posProducao].p[i])) {
            if (contem(NT, conjuntoNT[NT].producao[posProducao].p[i]) == 0) {
                conjuntoNT[NT].first[conjuntoNT[NT].incFirst] = conjuntoNT[NT].producao[posProducao].p[i];
                conjuntoNT[NT].incFirst++;
            }
            break;
        }

        adicionaFirst(NT, retornaIndiceNt(conjuntoNT[NT].producao[posProducao].p[i]));

        if (contemVazio(retornaIndiceNt(conjuntoNT[NT].producao[posProducao].p[i])) == 0) {
            break;
        }
    }
    if (i == strlen(conjuntoNT[NT].producao[posProducao].p)) {
        if (contemVazio(NT) == 0) {
            conjuntoNT[NT].first[conjuntoNT[NT].incFirst] = LAMBDA;
            conjuntoNT[NT].incFirst++;
        }
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

void adicionaFirst(int des, int src) {
    for (int i = 0; i < conjuntoNT[src].incFirst; i++) {
        if ((contem(des, conjuntoNT[src].first[i]) == 0) && (conjuntoNT[src].first[i] != LAMBDA)) {
            conjuntoNT[des].first[conjuntoNT[des].incFirst] = conjuntoNT[src].first[i];
            conjuntoNT[des].incFirst++;
        }
    }
}

void adicionaFirstnoFollow(int des, int src) {
    for (int i = 0; i < conjuntoNT[src].incFirst; i++) {
        if ((contemFollow(des, conjuntoNT[src].first[i]) == 0)) {
            if(conjuntoNT[src].first[i] != 'e'){
                conjuntoNT[des].follow[conjuntoNT[des].incFollow] = conjuntoNT[src].first[i];
                conjuntoNT[des].incFollow++;
            }
        }
    }
}

void adicionaFollow(int des, int src) {
    for (int i = 0; i < conjuntoNT[src].incFollow; i++) {
        if ((contemFollow(des, conjuntoNT[src].follow[i]) == 0)) {
            conjuntoNT[des].follow[conjuntoNT[des].incFollow] = conjuntoNT[src].follow[i];
            conjuntoNT[des].incFollow++;
        }
    }
}

int contem(int des, char c) {
    for (int i = 0; i < conjuntoNT[des].incFirst; i++) {
        if (conjuntoNT[des].first[i] == c) {
            return 1;
        }
    }
    return 0;
}

int contemFollow(int des, char c) {
    for (int i = 0; i < conjuntoNT[des].incFollow; i++) {
        if (conjuntoNT[des].follow[i] == c) {
            return 1;
        }
    }
    return 0;
}

int contemVazio(int index) {
    return contem(index, LAMBDA);
}

void printNt(char* nomeArq) {
    //    abrirArquivo(nomeArq);
    if ((file = fopen(nomeArq, "w")) == NULL) {
        printf("Falha na abertura do arquivo");
    }

    for (int j = 0; j < posConjNT; j++) {
        fprintf(file, "%c->", conjuntoNT[j].c);
        for (int n = 0; n < conjuntoNT[j].incProducao; n++) {
            fprintf(file, "%s|", conjuntoNT[j].producao[n].p);
        }
        fprintf(file, "\n\t First: {");
        for (int n = 0; n < conjuntoNT[j].incFirst; n++) {
            if (n) {
                fprintf(file, ",");
            }
            fprintf(file, "%c", conjuntoNT[j].first[n]);
        }
        fprintf(file, "}\n");
        fprintf(file, "\t Follow: {");
        for (int n = 0; n < conjuntoNT[j].incFollow; n++) {
            if (n) {
                fprintf(file, ",");
            }
            fprintf(file, "%c", conjuntoNT[j].follow[n]);
        }
        fprintf(file, "}\n");

    }
    fprintf(file, "\n");

    fprintf(file, "\t|");
    for (int j = 0; j < posConjT; j++) {
        fprintf(file, "%c \t|", conjuntoT[j]);
    }
    fprintf(file, "\n");
    for (int i = 0; i < posConjNT; i++) {
        fprintf(file, "%c \t|", conjuntoNT[i].c);
        for (int j = 0; j < posConjT; j++) {
            if (strcmp(matrizResultado[i][j].p, "") != 0) {
                fprintf(file, "%c->%s \t|", conjuntoNT[i].c, matrizResultado[i][j].p);
            } else {
                fprintf(file, "\t|");
            }
        }
        fprintf(file, "\n");
    }
    fclose(file);
}

char* split(char* str, int index) {
    char resultado[strlen(str) - index + 1];
    int j = 0;
    for (int i = index, len = strlen(str); i < len; i++, j++) {
        resultado[j] = str[i];
    }
    resultado[j] = '\0';
    return resultado;

}

char* removeVazio(char* str) {
    char retorno[strlen(str)];

    for (int i = 0, j = 0; i < strlen(str); i++) {
        if (str[i] != LAMBDA) {
            retorno[j++] = str[i];
        }
    }
    return retorno;
}

void follow() {
    backupFollow = -1;
    conjuntoNT[0].follow[0] = '$';
    conjuntoNT[0].follow[1] = '\0';
    conjuntoNT[0].incFollow = 1;
    do {
        int count = 0;
        while (count <= posConjNT) {
            for (int j = 0; j < conjuntoNT[count].incProducao; j++) {
                for (int k = 0, len = strlen(conjuntoNT[count].producao[j].p); k < len; k++) {
                    int indiceK = retornaIndiceNt(conjuntoNT[count].producao[j].p[k]);
                    if (k == (len - 1)) {
                        if (isupper(conjuntoNT[count].producao[j].p[k])) {
                            adicionaFollow(indiceK, count);
                        }
                        break;
                    }
                    if (isupper(conjuntoNT[count].producao[j].p[k])) {
                        if (isupper(conjuntoNT[count].producao[j].p[k + 1])) {
                            adicionaFirstnoFollow(indiceK, retornaIndiceNt(conjuntoNT[count].producao[j].p[k + 1]));
                            if (contemVazio(retornaIndiceNt(conjuntoNT[count].producao[j].p[k + 1])) == 1) {
                                adicionaFollow(indiceK, count);
                            }
                        } else if (contemFollow(indiceK, conjuntoNT[count].producao[j].p[k + 1]) == 0) {
                            conjuntoNT[indiceK].follow[conjuntoNT[indiceK].incFollow] = conjuntoNT[count].producao[j].p[k + 1];
                            conjuntoNT[indiceK].incFollow++;
                        }
                    }
                }
            }
            count++;
        }
    } while (changeFollow() == 1);



}

int somaFollow() {
    int soma = 0;
    for (int i = 0; i < posConjNT; i++) {
        soma = soma + strlen(conjuntoNT[i].follow);
    }
    return soma;
}

int changeFollow() {
    int soma = somaFollow();
    if (backupFollow != soma) {
        backupFollow = soma;
        return 1;
    }
    return 0;
}

void criaTabela() {
    int flagVazio = 0;
    for (int i = 0; i < posConjNT; i++) { //para cada não terminal 
        for (int j = 0; j < conjuntoNT[i].incProducao; j++) { //para cada producao de i(não terminal)
            if (!isupper(conjuntoNT[i].producao[j].p[0]) && conjuntoNT[i].producao[j].p[0] != LAMBDA) { //se é um terminal então ele é o first na produção j
                insereProducaoNaMatriz(i, retornaIndiceColunaTerminal(conjuntoNT[i].producao[j].p[0]), conjuntoNT[i].producao[j].p);
            } else if (conjuntoNT[i].producao[j].p[0] != LAMBDA) {
                int indiceProducaoP = -1;
                int indiceConjunto;
                do {
                    indiceProducaoP++;
                    indiceConjunto = retornaIndiceNt(conjuntoNT[i].producao[j].p[indiceProducaoP]);
                    for (int k = 0; k < conjuntoNT[indiceConjunto].incFirst; k++) {

                        insereProducaoNaMatriz(i, retornaIndiceColunaTerminal(conjuntoNT[indiceConjunto].first[k]), conjuntoNT[i].producao[j].p);

                        if (conjuntoNT[i].first[k] == LAMBDA) {
                            flagVazio = 1;
                        }
                    }
                } while ((contemVazio(indiceConjunto) == 1) && (indiceProducaoP < strlen(conjuntoNT[i].producao[j].p)));
            } else {
                flagVazio = 1;
            }
            if (flagVazio == 1) {
                for (int k = 0; k < conjuntoNT[i].incFollow; k++) {
                    insereProducaoNaMatriz(i, retornaIndiceColunaTerminal(conjuntoNT[i].follow[k]), conjuntoNT[i].producao[j].p);
                }
            }
            if ((flagVazio == 1) && (contemFimString(i) == 1)) {
                insereProducaoNaMatriz(i, retornaIndiceColunaTerminal(FIM_STRING), conjuntoNT[i].producao[j].p);
            }
            flagVazio = 0;
        }
    }
}

int contemFimString(int index) {
    for (int i = 0; i < conjuntoNT[index].incFollow; i++) {
        if (conjuntoNT[index].follow[i] == FIM_STRING) {
            return 1;
        }
    }

    return 0;
}

void insereProducaoNaMatriz(int linha, int coluna, char* producao) {
    strcpy(matrizResultado[linha][coluna].p, producao);
}

int retornaIndiceColunaTerminal(char c) {
    for (int j = 0; j < posConjT; j++) {
        if (conjuntoT[j] == c) {
            return j;
        }
    }
    return -1;
}