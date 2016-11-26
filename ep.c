#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <math.h>
#include <omp.h>
#include "util.h"

#define TRUE 1
#define FALSE 0
#define DEBUG 1
#define NMAX 1000
#define MAX_LINE 256 /*Intervalo [0, 255]*/
#define RGB_SIZE 256
#define PI 3.14159265359

long int  x;
pthread_mutex_t mut = PTHREAD_MUTEX_INITIALIZER;

/*Colocar futuramente essa função no util*/
void * do_job() {
  	pthread_mutex_lock(&mut);
	x++;
	pthread_mutex_unlock(&mut);

    pthread_exit(0);
}


int main(int argc, char **argv) {

	FILE *arq1, *arq2;
	char *infile, *outfile;
	char a[MAX_LINE];
	int nr_inter, nr_proc, nr_threads;
	int i, j, k, cont, columns, lines, comp_max_val;
	pthread_t *id;
	float val, distribute;
	float gx, gy, g;
    Pixel **M, **M2, **aux; /*Matriz de pixels*/

    i = j = k = 0;
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
    nr_threads = 2 * nr_proc; /*Considerando hyperthread*/

    id = malloc(nr_threads * sizeof(int));

	arq1 = fopen(infile, "r");

   	if (arq1 == NULL)
	    printf("Erro, não foi possível abrir o arquivo\n");
	else {
		/*Read the input file*/

		if (DEBUG) printf("Consegui abrir o arquivo!\n");

		cont = 0;
	    while ((a[0] = fgetc(arq1)) != EOF) {
		    if (a[0] == '#' || a[0] == 'P') {
		        fgets(a, MAX_LINE, arq1);

		        if (DEBUG) printf("Ignorando comentários...\n");
		    }
			else if (cont == 0) {
				ungetc(a[0], arq1);
				fscanf(arq1,"%d %d\n", &columns, &lines);
				fscanf(arq1,"%d\n", &comp_max_val);
				cont++;

				if (DEBUG) {
					printf("Num_linhas = %d, num_colunas = %d\n", lines, columns);
					printf("comp_max_val = %d\n", comp_max_val);
				}

				/*Alocação das matrizes*/
				M = (Pixel **) malloc(lines * sizeof(Pixel*));
		    	M2 = (Pixel **) malloc(lines * sizeof(Pixel*));
		    	for (i = 0; i < lines; i++) {
		    		M[i] = (Pixel *) malloc(columns * sizeof(Pixel));
		    		M2[i] = (Pixel *) malloc(columns * sizeof(Pixel));
		    	}
			}
		    else {
		    	ungetc(a[0], arq1);
		    	for (i = 0; i < lines; i++) {
		    		for (j = 0; j < columns; j++) {
		    		    fscanf(arq1, "%f %f %f", &M[i][j].R, &M[i][j].G, &M[i][j].B);
		    		    M[i][j].R /= RGB_SIZE; 
		    		    M[i][j].G = (2*PI * M[i][j].G) / RGB_SIZE; 
		    		    M[i][j].B /= RGB_SIZE;

						/* Calcular Rx, Ry, Bx e By quando ler a entrada \/*/
						M[i][j].Rx = horizontal_component(M[i][j].R, M[i][j].G);
						M[i][j].Bx = horizontal_component(M[i][j].B, M[i][j].G);
						M[i][j].Ry = vertical_component(M[i][j].R, M[i][j].G);
						M[i][j].By = vertical_component(M[i][j].B, M[i][j].G);
		    		}
		    	}
		    	break;
		    }

	    }
	}

	fclose(arq1);
	if (DEBUG) printf("Arquivo lido!\n");

	

	/*Create the threads and divide the work between them*/
    for (i = 0; i < nr_threads; i++)   pthread_create(&id[i], NULL, do_job, NULL);
    for (i = 0; i < nr_threads; i++)   pthread_join(id[i], NULL);

    /*IMPORTANTE: As bordas nunca se alteram.*/
	for (k = 0; k < nr_inter; k++) {

		aux = M;
		M = M2;
		M2 = aux;
		/*cp(M, M2, lines, columns);*/

		/*if (DEBUG) {
			printf("M2:\n");
			for (i = 0; i < lines; i++) {
			    for (j = 0; j < columns; j++)
			    	printf("%f ", M2[i][j].R);
			    printf("\n");
			}			
		}*/


		for (i = 1; i < lines - 1; i++) {  /*Por causa da borda*/
			for (j = 1; j < columns - 1; j++) {

				if (M[i][j].Rx > 0) {
					if (j != columns -1) {
						val = transfer(M[i][j+1].R, M[i][j].Rx);
						M2[i][j+1].Rx += val;
						M2[i][j].Rx -= val;
					}
					if (j != 1) {
						val = transfer(M[i][j-1].B, M[i][j].Bx);
						M2[i][j-1].Bx += val; /*Recebe no sentido oposto*/
						M2[i][j].Bx -= val;
					}
				}
				else { /*Recebe um valor positivo*/
					if (j != 1) {
						val = transfer(M[i][j-1].R, M[i][j].Rx);
						M2[i][j-1].Rx -= val;
						M2[i][j].Rx += val;
					}
					if (j != columns - 1) {
						val = transfer(M[i][j+1].B, M[i][j].Bx);
						M2[i][j+1].Bx -= val;  /*Recebe no sentido oposto*/
						M2[i][j].Bx += val;
					}
				}

				if (M[i][j].Ry > 0) {
					if (i != 1) {
						val = transfer(M[i-1][j].R, M[i][j].Ry);
						M2[i-1][j].Ry += val;
						M2[i][j].Ry -= val;
					}
					if (i != lines - 1) {
						val = transfer(M[i+1][j].B, M[i][j].By);
						M2[i+1][j].By += val;
						M2[i][j].By -= val;
					}
				}

				else { /*Recebe um valor positivo*/
					if (i != lines - 1) {
						val = transfer(M[i+1][j].R, M[i][j].Ry);
						M2[i+1][j].Ry -= val;
						M2[i][j].Ry += val;
					}
					if (i != 1) {
						val = transfer(M[i-1][j].B, M[i][j].By);
						M2[i-1][j].By -= val;
						M2[i][j].By += val;
					}
				}
			}
		}

	/*O bloco abaixo checa se os pixels vizinhos estouraram*/
		for (i = 1; i < lines - 1; i++) {
			for (j = 1; j < columns - 1; j++) {
				/*Paralelizar as checagens do R e B se tiver pelo menos oito threads, podendo
				deixar os 4 if's internos em paralelo*/

				/*Checa o R*/
				if (M2[i][j].R > 1) {
					distribute = (M2[i][j].R - 1) / 4;
					M2[i][j].R = 1;

					/*Dá para parelelizar os if's abaixo*/

					/*Os if's checam se os vizinhos não estão na borda e não serão estourados*/
					if (i-1 > 0 && M2[i-1][j].R + distribute < 1) M2[i-1][j].R += distribute;
					if (i+1 < lines && M2[i+1][j].R + distribute < 1) M2[i+1][j].R += distribute;
					if (j-1 > 0 && M2[i][j-1].R + distribute < 1) M2[i][j-1].R += distribute;
					if (j+1 < columns && M2[i][j+1].R + distribute < 1) M2[i][j+1].R += distribute;
				}

				/*Checa o B*/
				if (M2[i][j].B > 1) {
					distribute = (M2[i][j].B - 1) / 4;
					M2[i][j].B = 1;

					/*Os if's checam se os vizinhos não estão na borda e não serão estourados*/
					if (i-1 > 0 && M2[i-1][j].B + distribute < 1) M2[i-1][j].B += distribute;
					if (i+1 < lines && M2[i+1][j].B + distribute < 1) M2[i+1][j].B += distribute;
					if (j-1 > 0 && M2[i][j-1].B + distribute < 1) M2[i][j-1].B += distribute;
					if (j+1 < columns && M2[i][j+1].B + distribute < 1) M2[i][j+1].B += distribute;
				}
			}
		}

		/*Laço para atualizar G*/
		for (i = 1; i < lines - 1; i++) {
			for (j = 1; j < columns - 1; j++) {

				gx = M2[i][j].Rx + M2[i][j].Bx;
				gy = M2[i][j].Ry + M2[i][j].By;
				g = sqrt((gx*gx) + (gy*gy));
				
				M2[i][j].G += g;
				
				if (M2[i][j].G > 2 * PI)
					M2[i][j].G -= 2 * PI;
			}
		}
	}

	/*Feito isso, checar se algum valor ultrapassou 1
	*ou ficou negativo (embora provavelmente não dê para
	*ficar negativo)*/


	/*Escreve no arquivo de saída*/
	arq2 = fopen(outfile, "w");

   	if (arq2 == NULL)
	    printf("Erro, não foi possível abrir o arquivo\n");
	else {

		/*sprintf(outfile, "%s.ppm", outfile);*/
	    fprintf(arq2, "P3\n%d %d\n255\n", columns, lines);

	    for (i = 0; i < lines; i++) {
			for (j = 0; j < columns; j++) {
				fprintf(arq2, "%.3f %.3f %.3f    ",
				   (float)(RGB_SIZE* M2[i][j].R), (float)((RGB_SIZE* M2[i][j].G) / (2*PI)), (float)(RGB_SIZE* M2[i][j].B));
		    }
		    fprintf(arq2, "\n");   
		}

	    fprintf(stdout, "A imagem foi salva no arquivo: %s\n", outfile);
	    fclose(arq2);

	}

 	return 0;
 }
