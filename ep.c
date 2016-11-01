#include <stdio.h>
#include <stdlib.h>
#include "util.h"

#define TRUE 1
#define FALSE 0
#define DEBUG 0
#define NMAX 1000
#define PI 3.14159265359

int main(int argc, char **argv) {

	FILE *arq1, *arq2;
	char *infile, *outfile;
	int nr_inter, nr_proc, i, j, k;
	float hor, ver;
	Pixel M[NMAX][NMAX]; /*Matriz de pixels*/

   	/*Usemode*/
   	if (argc < 5) {
   		printf("Modo de usar:\n\tArg1: nome do arquivo de entrada;\n\tArg2: nome do arquivo de saída\n\t");
		printf("Arg3: número de iterações;\n\tArg4: número de processadores.\n\t");
   		exit(1);
   	}

    infile = argv[1];
	outfile = argv[2];
	nr_inter = atoi(argv[3]);
	nr_proc = atoi(argv[4]);

	arq1 = fopen(infile, "r");

   	if (arq1 == NULL)
	    printf("Erro, não foi possível abrir o arquivo\n");
	else {
			/*Read the input file*/
	}

	fclose(arq1);

	// Calcular Rx, Ry, Bx e By quando ler a entrada \/
	M2[i][j].Rx = horizontal_component(M[i][j].R, M[i][j].G);
	M2[i][j].Bx = horizontal_component(M[i][j].B, M[i][j].G);
	M2[i][j].Ry = vertical_component(M[i][j].R, M[i][j].G);
	M2[i][j].By = vertical_component(M[i][j].B, M[i][j].G);

    /*IMPORTANTE: As bordas nunca se alteram.
	* Precisa fazer verificar se não é borda*/
	for (k = 0; k < nr_inter; k++) {
		for (i in M2) {
			for (j in M2) {

				M2[i][j].Rx += M[i][j].Rx; // <- Copiar a matriz M em M2 antes dos laços e tirar as linhas (1)
				M2[i][j].Bx += M[i][j].Bx; // (1)
				if (M[i][j].Rx > 0) {
					M2[i][j+1].Rx += transfer(M[i][j+1].R, M[i][j].Rx);
					M2[i][j-1].Bx += transfer(M[i][j-1].B, M[i][j].Bx); /*Recebe no sentido oposto*/
				}
				else { /*Recebe um valor positivo*/
					M2[i][j-1].Rx -= transfer(M[i][j-1].R, M[i][j].Rx);
					M2[i][j+1].Bx -= transfer(M[i][j+1].B, M[i][j].Bx); /*Recebe no sentido oposto*/
				}

				M2[i][j].Ry += M[i][j].Ry; // (1)
				M2[i][j].By += M[i][j].By; // (1)
				if (M[i][j].Ry > 0) {
					M2[i-1][j].Ry += transfer(M[i-1][j].R, M[i][j].Ry);
					M2[i+1][j].By += transfer(M[i+1][j].B, M[i][j].By);
				}

				else { /*Recebe um valor positivo*/
					M2[i+1][j].Ry -= transfer(M[i+1][j].R, M[i][j].Ry);
					M2[i-1][j].By -= transfer(M[i-1][j].B, M[i][j].By);
				}

				// Checar se os pixels vizinhos estouraram

			}
		}

		/*Laço para atualizar G*/
	}

	/*Feito isso, checar se algum valor ultrapassou 1
	*ou ficou negativo (embora acho que não dê para
	*ficar negativo)*/


	/*Escrever no arquivo de saída*/
	arq2 = fopen(outfile, "w");

   	if (arq2 == NULL)
	    printf("Erro, não foi possível abrir o arquivo\n");
	else {
			/*Write matriz in outfile*/
	}

 	return 0;
 }
