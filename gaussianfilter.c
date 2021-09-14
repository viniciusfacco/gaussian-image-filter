#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define MAX_NAME 256 /* tamanho maximo para nome de arquivo */
#define PI 3.14159265359

int **oldimage;
int **newimage;
double **matrizpesos;
int nthreads;
int linhas;
int colunas;
int tamanho; //se a divisao nao for inteira aqui vai o mais 1
int sobra; //resto da divisao que vamos distribuir
int raio; //distancia para considerar os pixels
double somapesos;

void tratamento_normal(int li, int lf);
void tratamento_plinha();
void tratamento_ulinha(int l);
void tratamento_pcoluna(int li, int lf);
void tratamento_ucoluna(int li, int lf);
void InicializaMatrizPesos();
void *filtra(int id);
double **AlocaMatrizDouble(int lin, int col);
double **LiberaMatrizDouble(int lin, int col, double **mat);
int **AlocaMatriz(int lin, int col);
int **LiberaMatriz(int lin, int col, int **mat);

int main() {
    FILE *arqin;
    FILE *arqout;
    char narqin[MAX_NAME] = "c:\\temp\\reddead.ppm";
    char narqout[MAX_NAME] = "c:\\temp\\reddead2.ppm";
    char key[128];
    int i, j, max, r, g, b;
    
    printf("Quantas threads?\n");
    scanf("%d", &nthreads);
    pthread_t threads[nthreads];
    
    printf("Qual raio?\n");
    scanf("%d", &raio);
    
    matrizpesos = AlocaMatrizDouble(raio*2+1, raio*2+1);
    
    InicializaMatrizPesos();
    
//    printf("E qual a imagem? (maximo %d caracteres)\n", MAX_NAME);
//    scanf("%s", &narqin);
    
//    printf("E o destino? (maximo %d caracteres)\n", MAX_NAME);
//    scanf("%s", &narqout);
    
    printf("Arquivo de entrada: %s\n", narqin);
    arqin = fopen(narqin, "r");
    
    if (arqin == NULL) {
        printf("Erro na abertura do arquivo %s\n", narqin);
        return 0;
    }
    
    printf("Arquivo de saida: %s\n", narqout);
    arqout = fopen(narqout, "w");
    
    if (arqout == NULL) {
        printf("Erro na abertura do arquivo %s\n", narqin);
        return 0;
    }
    
    fscanf(arqin, "%s", key);//leio cabeçalho
    fprintf(arqout, "%s\n", key);//já escrevo o cabeçalho no novo arquivo
    printf("Arquivo tipo: %s \n", key);    
    fscanf(arqin, "%d %d %d", &colunas, &linhas, &max);//leio mais dados do cabeçalho
    fprintf(arqout, "%d %d \n%d", colunas, linhas, max);//já escrevo esses dados no novo arquivo
    printf("Colunas = %d \nLinhas = %d \n", colunas, linhas);
    
    //vamos definir o tamanho para cada um
    tamanho = linhas / nthreads;
    if ((linhas % nthreads) > 0){
        sobra = linhas % nthreads;
    } else {
        sobra = 0;
    }
    
    printf("Tamanho %d\n", tamanho);
    
    //por enquanto nao vamos aceitar imagem com apenas uma linha
    if(linhas < nthreads){
        printf("Mais threads do que dados %s\n", narqin);
        return 0;
    }
            
    oldimage = AlocaMatriz(linhas, colunas);    
    newimage = AlocaMatriz(linhas, colunas);

    for (i = 0; i <= linhas - 1; i++)
        for (j = 0; j <= colunas - 1; j++) {
            fscanf(arqin, " %d %d %d ", &r, &g, &b);
            //printf("RGB: %d %d %d \n", r, g, b);
            oldimage[i][j] = r*1000000+g*1000+b;
/*
            rgb = oldimage[i][j];
            nr = rgb/1000000;
            ng = (rgb-r*1000000)/1000;
            nb = rgb-r*1000000-g*1000;
            if ((nr != r) || (ng != g) || (nb != b)) printf("errooou");
            printf("Valor: %d\n", rgb);            
            printf("Valor R: %d\n", r);
            printf("Valor G: %d\n", g);
            printf("Valor B: %d\n", b);
*/
        }
    
    //vamos criar as threads    
    for(i = 0; i < nthreads; i++){
        pthread_create (&threads[i], NULL, filtra, i);
        printf("Thread %d iniciada.\n", i);
    }
        
    //vamos receber as threads
    for(i = 0; i < nthreads; i++){
        pthread_join(threads[i], NULL);
        printf("Thread %d terminada.\n", i);
    }
    
    //escrever novo arquivo    
    for (i = 0; i <= linhas - 1; i++){
        fprintf(arqout, "\n");
        for (j = 0; j <= colunas - 1; j++) {
            r = newimage[i][j]/1000000;
            g = (newimage[i][j]-r*1000000)/1000;
            b = newimage[i][j]-r*1000000-g*1000;
            fprintf(arqout, "%d %d %d ", r, g, b);
        }
    }
    
//    for (i = 0; i <= linhas - 1; i++) for (j = 0; j <= colunas - 1; j++) printf("RGB: %d %d %d \n", newimage[i][j*3], newimage[i][j*3+1], newimage[i][j*3+2]);
    
    LiberaMatriz(linhas, colunas, oldimage);
    LiberaMatriz(linhas, colunas, newimage);
    LiberaMatrizDouble(raio*2+1, raio*2+1,matrizpesos);
    
    fclose(arqin);
    fclose(arqout);
    
    printf("Fim programa.\n");
    return 0;
}

/* metodo funcionando testado semana passada
void tratamento_normal(int li, int lf){
//    printf("tratamento normal");
    int r,g,b, nr, ng, nb;
    int l;
    int c;
    int lin_mat_pes, col_mat_pes;
    double acumular, acumulag, acumulab;
    for(l = li; l <= lf; l++){
        for(c = raio; c < colunas - raio; c++){
            acumular = 0;
            acumulag = 0;
            acumulab = 0;
            for(lin_mat_pes = 0; lin_mat_pes < raio*2+1; lin_mat_pes++){
                for(col_mat_pes = 0; col_mat_pes < raio*2+1; col_mat_pes++){
                    r = oldimage[l-raio+lin_mat_pes][c-raio+col_mat_pes]/1000000;
                    g = (oldimage[l-raio+lin_mat_pes][c-raio+col_mat_pes]-r*1000000)/1000;;
                    b = oldimage[l-raio+lin_mat_pes][c-raio+col_mat_pes]-r*1000000-g*1000;
                    //printf("(%d,%d)B da volta: %d\n", lin_mat_pes, col_mat_pes, b);
                    //printf("R: %d  Peso: %.4f Resultado: %.4f\n", r, matrizpesos[lin_mat_pes][col_mat_pes], matrizpesos[lin_mat_pes][col_mat_pes]*r);
                    acumular += (r * matrizpesos[lin_mat_pes][col_mat_pes]);
                    acumulag += (g * matrizpesos[lin_mat_pes][col_mat_pes]);
                    //printf("vou multi... %.4f\n", b * matrizpesos[lin_mat_pes][col_mat_pes]);
                    acumulab += (b * matrizpesos[lin_mat_pes][col_mat_pes]);
                }
            }
            //printf("Acumulado: %.4f\n", acumulab);
            nr = acumular;
            ng = acumulag;
            nb = acumulab;
            //r = oldimage[l][c]/1000000;
            //g = (oldimage[l][c]-r*1000000)/1000;;
            //b = oldimage[l][c]-r*1000000-g*1000;
            //printf("RGB %d,%d original: %d, %d, %d\n", l, c, r, g, b);
            //printf("RGB resultan: %d, %d, %d\n", nr, ng, nb);
            newimage[l][c] = nr*1000000+ng*1000+nb;
        }
    }
}
*/

//novo método trata tudo
void tratamento_normal(int li, int lf){
    int dls, dli, dce, dcd;
    int r,g,b, nr, ng, nb;
    int l;
    int c;
    int lin_mat_pes, col_mat_pes;
    double acumular, acumulag, acumulab;
    for(l = li; l <= lf; l++){
	if ((l - raio) < 0){
            dls = raio - l;
	} else dls = 0;
	if ((l + raio) >= linhas){
            dli = l + raio - (linhas -1);
	} else dli = 0;
        //printf("Linha: %d ... dls(%d) e dli(%d)\n", l, dls, dli);
        for(c = 0; c < colunas; c++){
            acumular = 0;
            acumulag = 0;
            acumulab = 0;
            if ((c - raio) < 0){
		dce = raio - c;				
            } else dce = 0;
            if ((c + raio) >= colunas){
		dcd = c + raio - (colunas -1);
            } else dcd = 0;
            for(lin_mat_pes = dls; lin_mat_pes < (raio*2+1-dli); lin_mat_pes++){
                for(col_mat_pes = dce; col_mat_pes < (raio*2+1-dcd); col_mat_pes++){
                    r = oldimage[l-raio+lin_mat_pes][c-raio+col_mat_pes]/1000000;
                    g = (oldimage[l-raio+lin_mat_pes][c-raio+col_mat_pes]-r*1000000)/1000;;
                    b = oldimage[l-raio+lin_mat_pes][c-raio+col_mat_pes]-r*1000000-g*1000;
                    acumular += (r * matrizpesos[lin_mat_pes][col_mat_pes]);
                    acumulag += (g * matrizpesos[lin_mat_pes][col_mat_pes]);
                    acumulab += (b * matrizpesos[lin_mat_pes][col_mat_pes]);
                }
            }
            nr = acumular;
            ng = acumulag;
            nb = acumulab;
            newimage[l][c] = nr*1000000+ng*1000+nb;
        }
    }
}

void tratamento_plinha(){
//    printf("tratamento plinha");
    int r, g, b;
    int c;
    //primeira linha e primeira coluna
    r = 0;
    g = 1;
    b = 2;
    newimage[0][r] = (          //R
              oldimage[0][r]    //l     , c
            + oldimage[1][r]    //l + 1 , c
            + oldimage[0][r+3]  //l     , c + 1
            + oldimage[1][r+3]  //l + 1 , c + 1
            ) / 4;
    
    newimage[0][g] = (          //G
              oldimage[0][g]    //l     , c
            + oldimage[1][g]    //l + 1 , c
            + oldimage[0][g+3]  //l     , c + 1
            + oldimage[1][g+3]  //l + 1 , c + 1
            ) / 4;
    
    newimage[0][b] = (          //B
              oldimage[0][b]    //l     , c
            + oldimage[1][b]    //l + 1 , c
            + oldimage[0][b+3]  //l     , c + 1
            + oldimage[1][b+3]  //l + 1 , c + 1
            ) / 4;
    
    //primeira linha e ultima coluna
    r = (colunas * 3) - 3;
    g = r+1;
    b = r+2;
    newimage[0][r] = (          //R
              oldimage[0][r]    //l     , c
            + oldimage[1][r]    //l + 1 , c
            + oldimage[0][r-3]  //l     , c - 1
            + oldimage[1][r-3]  //l + 1 , c - 1
            ) / 4;
    
    newimage[0][g] = (          //G
              oldimage[0][g]    //l     , c
            + oldimage[1][g]    //l + 1 , c
            + oldimage[0][g-3]  //l     , c - 1
            + oldimage[1][g-3]  //l + 1 , c - 1
            ) / 4;
    
    newimage[0][b] = (          //B
              oldimage[0][b]    //l     , c
            + oldimage[1][b]    //l + 1 , c
            + oldimage[0][b-3]  //l     , c - 1
            + oldimage[1][b-3]  //l + 1 , c - 1
            ) / 4;
    
    for(c = 1; c < colunas - 1; c++){
        r = c*3;
        g = r+1;
        b = r+2;
        newimage[0][r] = (          //R
                  oldimage[0][r]    //l     , c
                + oldimage[1][r]    //l + 1 , c
                + oldimage[0][r-3]  //l     , c - 1
                + oldimage[1][r-3]  //l + 1 , c - 1
                + oldimage[0][r+3]  //l     , c + 1
                + oldimage[1][r+3]  //l + 1 , c + 1
                ) / 6;

        newimage[0][g] = (          //G
                  oldimage[0][g]    //l     , c
                + oldimage[1][g]    //l + 1 , c
                + oldimage[0][g-3]  //l     , c - 1
                + oldimage[1][g-3]  //l + 1 , c - 1
                + oldimage[0][g+3]  //l     , c + 1
                + oldimage[1][g+3]  //l + 1 , c + 1
                ) / 6;

        newimage[0][b] = (          //B
                  oldimage[0][b]    //l     , c
                + oldimage[1][b]    //l + 1 , c
                + oldimage[0][b-3]  //l     , c - 1
                + oldimage[1][b-3]  //l + 1 , c - 1
                + oldimage[0][b+3]  //l     , c + 1
                + oldimage[1][b+3]  //l + 1 , c + 1
                ) / 6;
    }
}

void tratamento_ulinha(int l){
//    printf("tratamento ulinha");
    int r, g, b;
    int c;
    //ultima linha e primeira coluna
    r = 0;
    g = 1;
    b = 2;
    l = linhas - 1;
    newimage[l][r] = (          //R
              oldimage[l][r]    //l     , c
            + oldimage[l-1][r]  //l - 1 , c
            + oldimage[l][r+3]  //l     , c + 1
            + oldimage[l-1][r+3]//l - 1 , c + 1
            ) / 4;
    
    newimage[l][g] = (          //G
              oldimage[l][g]    //l     , c
            + oldimage[l-1][g]  //l - 1 , c
            + oldimage[l][g+3]  //l     , c + 1
            + oldimage[l-1][g+3]//l - 1 , c + 1
            ) / 4;
    
    newimage[l][b] = (          //B
              oldimage[l][b]    //l     , c
            + oldimage[l-1][b]  //l - 1 , c
            + oldimage[l][b+3]  //l     , c + 1
            + oldimage[l-1][b+3]//l - 1 , c + 1
            ) / 4;
    
    //ultima linha e ultima coluna
    r = (colunas * 3) - 3;
    g = r+1;
    b = r+2;
    newimage[l][r] = (          //R
              oldimage[l][r]    //l     , c
            + oldimage[l-1][r]  //l - 1 , c
            + oldimage[l][r-3]  //l     , c - 1
            + oldimage[l-1][r-3]//l - 1 , c - 1
            ) / 4;
    
    newimage[l][g] = (          //G
              oldimage[l][g]    //l     , c
            + oldimage[l-1][g]  //l - 1 , c
            + oldimage[l][g-3]  //l     , c - 1
            + oldimage[l-1][g-3]//l - 1 , c - 1
            ) / 4;
    
    newimage[l][b] = (          //b
              oldimage[l][b]    //l     , c
            + oldimage[l-1][b]  //l - 1 , c
            + oldimage[l][b-3]  //l     , c - 1
            + oldimage[l-1][b-3]//l - 1 , c - 1
            ) / 4;
    
    for(c = 1; c < colunas - 1; c++){
        r = c*3;
        g = r+1;
        b = r+2;
        newimage[l][r] = (          //R
                  oldimage[l][r]    //l     , c
                + oldimage[l-1][r]  //l - 1 , c
                + oldimage[l][r-3]  //l     , c - 1
                + oldimage[l-1][r-3]//l - 1 , c - 1
                + oldimage[l][r+3]  //l     , c + 1
                + oldimage[l-1][r+3]//l - 1 , c + 1
                ) / 6;

        newimage[l][g] = (          //G
                  oldimage[l][g]    //l     , c
                + oldimage[l-1][g]  //l - 1 , c
                + oldimage[l][g-3]  //l     , c - 1
                + oldimage[l-1][g-3]//l - 1 , c - 1
                + oldimage[l][g+3]  //l     , c + 1
                + oldimage[l-1][g+3]//l - 1 , c + 1
                ) / 6;

        newimage[l][b] = (          //B
                  oldimage[l][b]    //l     , c
                + oldimage[l-1][b]  //l - 1 , c
                + oldimage[l][b-3]  //l     , c - 1
                + oldimage[l-1][b-3]//l - 1 , c - 1
                + oldimage[l][b+3]  //l     , c + 1
                + oldimage[l-1][b+3]//l - 1 , c + 1
                ) / 6;
    }    
}

void tratamento_pcoluna(int li, int lf){
//    printf("tratamento pcoluna");
    int r, g, b;
    int l;
    for(l = li; l <= lf; l++){
        r = 0;
        g = 1;
        b = 2;
        newimage[l][r] = (          //R
                  oldimage[l][r]    //l     , c
                + oldimage[l-1][r]  //l - 1 , c
                + oldimage[l+1][r]  //l + 1 , c
                + oldimage[l][r+3]  //l     , c + 1
                + oldimage[l-l][r+3]//l - 1 , c + 1
                + oldimage[l+1][r+3]//l + 1 , c + 1
                ) / 6;

        newimage[l][g] = (          //G
                  oldimage[l][g]    //l     , c
                + oldimage[l-1][g]  //l - 1 , c
                + oldimage[l+1][g]  //l + 1 , c
                + oldimage[l][g+3]  //l     , c + 1
                + oldimage[l-l][g+3]//l - 1 , c + 1
                + oldimage[l+1][g+3]//l + 1 , c + 1
                ) / 6;
        
        newimage[l][b] = (          //B
                  oldimage[l][b]    //l     , c
                + oldimage[l-1][b]  //l - 1 , c
                + oldimage[l+1][b]  //l + 1 , c
                + oldimage[l][b+3]  //l     , c + 1
                + oldimage[l-l][b+3]//l - 1 , c + 1
                + oldimage[l+1][b+3]//l + 1 , c + 1
                ) / 6;
    }
}

void tratamento_ucoluna(int li, int lf){
//    printf("tratamento ucoluna");
    int r, g, b;
    int l;
    for(l = li; l <= lf; l++){
        r = (colunas * 3) - 3;
        g = r + 1;
        b = r + 2;
        newimage[l][r] = (          //R
                  oldimage[l][r]    //l     , c
                + oldimage[l-1][r]  //l - 1 , c
                + oldimage[l+1][r]  //l + 1 , c
                + oldimage[l][r-3]  //l     , c - 1
                + oldimage[l-l][r-3]//l - 1 , c - 1
                + oldimage[l+1][r-3]//l + 1 , c - 1
                ) / 6;

        newimage[l][g] = (          //G
                  oldimage[l][g]    //l     , c
                + oldimage[l-1][g]  //l - 1 , c
                + oldimage[l+1][g]  //l + 1 , c
                + oldimage[l][g-3]  //l     , c - 1
                + oldimage[l-l][g-3]//l - 1 , c - 1
                + oldimage[l+1][g-3]//l + 1 , c - 1
                ) / 6;
        
        newimage[l][b] = (          //B
                  oldimage[l][b]    //l     , c
                + oldimage[l-1][b]  //l - 1 , c
                + oldimage[l+1][b]  //l + 1 , c
                + oldimage[l][b-3]  //l     , c - 1
                + oldimage[l-l][b-3]//l - 1 , c - 1
                + oldimage[l+1][b-3]//l + 1 , c - 1
                ) / 6;
    }
}

void InicializaMatrizPesos(){
    int i, j;
    double e, g;
    somapesos = 0;
    float sigma = raio;
    for(i = 0; i < sigma*2+1; i++){
        //printf("\n");
        for(j = 0; j < raio*2+1; j++){
            e = pow(exp(1), ((-1)*(pow((i-sigma),2)+pow((j-sigma),2))/(2*pow(sigma,2))));
            //printf("P(%d,%d)\n", i, j);
            //printf("E = %.4f - PARTEDECIMA = %.4f\n", e, partedecima);
            g = e/(2*PI*pow(sigma,2));
            matrizpesos[i][j] = g;
            somapesos+= g;
            //printf("P(%d,%d) = %.4f ;", i, j, g);
        }
    }
    for(i = 0; i < sigma*2+1; i++){
        //printf("de novo \n");
        for(j = 0; j < raio*2+1; j++){
            matrizpesos[i][j] = matrizpesos[i][j] / somapesos;
            //printf("P(%d,%d) = %.4f ;", i, j, matrizpesos[i][j]);
        }
    }
    //printf("somapesos = %.5f\n", somapesos);
}

void *filtra(int id){
    int linhai, linhaf, deslocamento, meutam;
    //printf("id = %d e inc = %d", id, incremento);
    meutam = tamanho;
    deslocamento = 0;
    if(sobra > 0){
        if(id < sobra){
            meutam++;
        } else {
            deslocamento = sobra;
        }
    }
    linhai = (meutam) * id + deslocamento;
    if(id == nthreads -1){
        linhaf = linhas - 1;
    } else {
        linhaf = (meutam) + linhai - 1;
    }
    printf("Eu sou a thread %d fico com: linha %d a %d\n", id, linhai, linhaf);
    
    tratamento_normal(linhai, linhaf);
    //if(id == 0){ //se essa thread pega a primeira parte
    //    tratamento_plinha(linhai);
    //    linhai++;
    //} 
    //if (id == (nthreads - 1)){ //se essa thread pega a última parte
    //    tratamento_ulinha(linhaf);
    //    linhaf--;
    //}
    //if(linhai <= linhaf){
    //    tratamento_pcoluna(linhai, linhaf);
    //    tratamento_ucoluna(linhai, linhaf);        
    //    if (colunas > 2){
    //        tratamento_normal(linhai, linhaf);
    //    }
    //}
}

int **AlocaMatriz (int lin, int col){
    int **mat;  /* ponteiro para a matriz */
    int i;    /* variavel auxiliar      */
    if (lin < 1 || col < 1) { /* verifica parametros recebidos */
        printf("** Erro: Parametro invalido **\n");
        return(NULL);
    }
    /* aloca as linhas da matriz */
    mat = (int **) calloc (lin, sizeof(int *));
    if (mat == NULL) {
        printf("** Erro: Memoria Insuficiente **");
        return(NULL);
    }
    /* aloca as colunas da matriz */
    for (i = 0; i < lin; i++){
        mat[i] = (int*) calloc (col, sizeof(int));
        if (mat[i] == NULL) {
            printf("** Erro: Memoria Insuficiente **");
            return(NULL);
        }
    }
    return(mat); /* retorna o ponteiro para a matriz */
}

int **LiberaMatriz(int lin, int col, int **mat){
    int i;  /* variavel auxiliar */
    if(mat == NULL) return(NULL);
    if(lin < 1 || col < 1){  /* verifica parametros recebidos */
        printf("** Erro: Parametro invalido **\n");
        return(mat);
    }
    for(i=0; i<lin; i++) free(mat[i]); /* libera as linhas da matriz */
    free(mat);      /* libera a matriz */
    return(NULL); /* retorna um ponteiro nulo */
}

double **AlocaMatrizDouble(int lin, int col){
    double **mat;  /* ponteiro para a matriz */
    int i;    /* variavel auxiliar      */
    if (lin < 1 || col < 1) { /* verifica parametros recebidos */
        printf("** Erro: Parametro invalido **\n");
        return(NULL);
    }
    /* aloca as linhas da matriz */
    mat = (double **) calloc (lin, sizeof(double *));
    if (mat == NULL) {
        printf("** Erro: Memoria Insuficiente **");
        return(NULL);
    }
    /* aloca as colunas da matriz */
    for (i = 0; i < lin; i++){
        mat[i] = (double*) calloc (col, sizeof(double));
        if (mat[i] == NULL) {
            printf("** Erro: Memoria Insuficiente **");
            return(NULL);
        }
    }
    return(mat); /* retorna o ponteiro para a matriz */
}

double **LiberaMatrizDouble(int lin, int col, double **mat){
    int i;  /* variavel auxiliar */
    if(mat == NULL) return(NULL);
    if(lin < 1 || col < 1){  /* verifica parametros recebidos */
        printf("** Erro: Parametro invalido **\n");
        return(mat);
    }
    for(i=0; i<lin; i++) free(mat[i]); /* libera as linhas da matriz */
    free(mat);      /* libera a matriz */
    return(NULL); /* retorna um ponteiro nulo */
}