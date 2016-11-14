#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "util.h"

#define TRUE 1
#define FALSE 0
#define NMAX 1000
/*A norma dos vetores corresponde à intensidade
de cada componente de cor*/


/*Na biblioteca do C o valor já é dado em radianos*/
float horizontal_component(float comp, float angle) {
    return comp * sin(angle);
}

float vertical_component(float comp, float angle) {
    return comp * cos(angle);
}

float transfer(float neighbor, float current){
    return ((1 - neighbor) * current) / 4;

    /*Precisa entender o que é o C_d da fórmula*/
}

/*Checa se a posição dada pelos índices está na borda da matriz*/
int isBorder(int i, int j) {
    if (i == 0 || j == 0 || i == NMAX || j == NMAX)
        return TRUE;
    else
        return FALSE;
}

void cp(Pixel **M, Pixel **M2, int lines, int columns) {

	int i, j;
	
	for (i = 0; i < lines; i++) {
		for (j = 0; j < columns; j++) {
			M2[i][j].Rx = M[i][j].Rx;
			M2[i][j].Ry = M[i][j].Ry;
			M2[i][j].G = M[i][j].G;
			M2[i][j].Bx = M[i][j].Bx; 
			M2[i][j].By = M[i][j].By; 
		}
		
	}

}