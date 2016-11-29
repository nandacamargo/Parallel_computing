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

int main(int argc, char **argv) {

	FILE *arq1, *arq2;
	char *infile, *outfile;
	char a[MAX_LINE];
	int nr_inter, nr_proc/*, nr_threads*/;
	int i, j, k, cont, columns, lines, comp_max_val;
	double val, distribute;
	double gx, gy, g, angle;
    Pixel **M, **M2, **aux; /*Matriz de pixels*/

    i = j = k = 0;
   	/*Modo de usar*/
   	if (argc < 5) {
   		printf("Modo de usar:\n\tArg1: nome do arquivo de entrada;\n\tArg2: nome do arquivo de saída\n\t");
		printf("Arg3: número de iterações;\n\tArg4: número de processadores.\n\t");
   		exit(1);
   	}

    infile = argv[1];
	outfile = argv[2];
	nr_inter = atoi(argv[3]);
	nr_proc = atoi(argv[4]);

	if (nr_proc <= 0) nr_proc = 1;
    
	arq1 = fopen(infile, "r");

   	if (arq1 == NULL)
	    printf("Erro, não foi possível abrir o arquivo\n");
	else {
		/*Read the input file*/

		if (DEBUG) printf("Arquivo aberto!\n");

		cont = 0;
	    while ((a[0] = fgetc(arq1)) != EOF) {
		    if (a[0] == '#' || a[0] == 'P') {
		        fgets(a, MAX_LINE, arq1);
		    }
			else if (cont == 0) {
				ungetc(a[0], arq1);
				fscanf(arq1,"%d %d\n", &columns, &lines);
				fscanf(arq1,"%d\n", &comp_max_val);
				cont++;

	
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
		    		    fscanf(arq1, "%lf %lf %lf", &M[i][j].R, &M[i][j].G, &M[i][j].B);
		    		    M[i][j].R /= RGB_SIZE;
		    		    M[i][j].G /= RGB_SIZE; 
		    		    /*M2[i][j].G = (2*PI * M2[i][j].G) / RGB_SIZE; */
		    		    M[i][j].B /= RGB_SIZE;
		    		    M[i][j].ang = 2 * PI * M[i][j].G;

						/* Calcular Rx, Ry, Bx e By quando ler a entrada \/*/
						M[i][j].Rx = horizontal_component(M[i][j].R, M[i][j].G);
						M[i][j].Bx = (-1) * horizontal_component(M[i][j].B, M[i][j].G);
						M[i][j].Ry = vertical_component(M[i][j].R, M[i][j].G);
						M[i][j].By = (-1) * vertical_component(M[i][j].B, M[i][j].G);
		    		}
		    	}
		    	break;
		    }

	    }
	}

	fclose(arq1);
	if (DEBUG) printf("Arquivo lido!\n");


    /*IMPORTANTE: As bordas nunca se alteram.*/
	for (k = 0; k < nr_inter; k++) {

		if (lines - 2 < nr_proc) nr_proc = 1;
	   
	   	aux = M;
	    M = M2;
	    M2 = aux;

	    cp(M, M2, lines, columns);

		#pragma omp parallel firstprivate(lines, columns) private(i, j, val) num_threads(nr_proc)
		{
		    int thread_num = omp_get_thread_num();
		    int num_threads = omp_get_num_threads();
		    int rest = (lines - 2) % num_threads;   /*lines-2 porque elimina as bordas*/
		    int start, end;

		    /*Divide os chunks para cada thread. O + 1 é para pular o zero, que é borda*/
		    /*Como sempre é menor estrito que end, não precisa se preocupar com a borda final*/
		    start = thread_num * (lines - 2) / num_threads + 1;
		    if (thread_num != 0  && (thread_num - 1) < rest) start++;

		    end = (thread_num + 1) * (lines - 2) / num_threads + 1;
		    if (thread_num < rest) end++;
		
 			for (i = start; i < end; i++) {	/*Por causa da borda*/
				for (j = 1; j < columns - 1; j++) {

					if (M2[i][j].Rx > 0) {
						if (j != columns -1) {
							val = transfer(M2[i][j+1].R, M2[i][j].Rx);
							if (i != start && i != end) {
									M[i][j+1].Rx += val;
									M[i][j].Rx -= val;
							}
							else {
								#pragma omp critical 
								{
									M[i][j+1].Rx += val;
									M[i][j].Rx -= val;
							    }
							}	
						}
						if (j != 1) {
							val = transfer(M2[i][j-1].B, M2[i][j].Bx);
							if (i != start && i != end) {
									/*Recebe no sentido oposto*/
									M[i][j-1].Bx += val; 
									M[i][j].Bx -= val;
							}
							else {
								#pragma omp critical 
								{
									M[i][j-1].Bx += val;
									M[i][j].Bx -= val;
							    }
							}								
						}
					}
					else { /*Recebe um valor positivo*/
						if (j != 1) {
							val = transfer(M2[i][j-1].R, M2[i][j].Rx);
							if (i != start && i != end) {
									M[i][j-1].Rx -= val;
									M[i][j].Rx += val;
							}
							else {
								#pragma omp critical 
								{
									M[i][j-1].Rx -= val;
									M[i][j].Rx += val;
							    }
							}
						}

						if (j != columns - 1) {
							val = transfer(M2[i][j+1].B, M2[i][j].Bx);
							if (i != start && i != end) {
									M[i][j+1].Bx -= val;  /*Recebe no sentido oposto*/
									M[i][j].Bx += val;
							}
							else {
								#pragma omp critical 
								{
									M[i][j+1].Bx -= val;
									M[i][j].Bx += val;
							    }
							}
						}
					}

					if (M2[i][j].Ry > 0) {
						if (i != 1) {
							val = transfer(M2[i-1][j].R, M2[i][j].Ry);
							if (i != start && i != end) {
									M[i-1][j].Ry += val;
									M[i][j].Ry -= val;
							}
							else {
								#pragma omp critical 
								{
									M[i-1][j].Ry += val;
									M[i][j].Ry -= val;
							    }
							}
						}
						if (i != lines - 1) {
							val = transfer(M2[i+1][j].B, M2[i][j].By);
							if (i != start && i != end) {
									M[i+1][j].By += val;
									M[i][j].By -= val;
							}
							else {
								#pragma omp critical 
								{
									M[i+1][j].By += val;
									M[i][j].By -= val;
							    }
							}
						}
					}

					else { /*Recebe um valor positivo*/
						if (i != lines - 1) {
							val = transfer(M2[i+1][j].R, M2[i][j].Ry);
							if (i != start && i != end) {
									M[i+1][j].Ry -= val;
									M[i][j].Ry += val;
							}
							else {
								#pragma omp critical 
								{
									M[i+1][j].Ry -= val;
									M[i][j].Ry += val;
							    }
							}
						}
						if (i != 1) {
							val = transfer(M2[i-1][j].B, M2[i][j].By);
							if (i != start && i != end) {
									M[i-1][j].By -= val;
									M[i][j].By += val;
							}
							else {
								#pragma omp critical 
								{
									M[i-1][j].By -= val;
									M[i][j].By += val;
							    }
							}
						}
					}
				}
			}

		}

		/*O bloco abaixo calcula as componentes R e B dos pixels*/
		for (i = 1; i < lines - 1; i++) {
			for (j = 1; j < columns - 1; j++) {
				M[i][j].R = sqrt((M[i][j].Rx*M[i][j].Rx) + (M[i][j].Ry*M[i][j].Ry));
				M[i][j].B = sqrt((M[i][j].Bx*M[i][j].Bx) + (M[i][j].By*M[i][j].By));
			}
		}

		/*O bloco abaixo checa se os pixels vizinhos estouraram*/
		for (i = 1; i < lines - 1; i++) {
			for (j = 1; j < columns - 1; j++) {

				/*Checa o R*/
				if (M[i][j].R > 1) {
					distribute = (M[i][j].R - 1) / 4;
					M[i][j].R = 1;

					/*Os if's checam se os vizinhos não estão na borda e não serão estourados*/

					if (i-1 > 0 && M[i-1][j].R + distribute < 1) M[i-1][j].R += distribute;
					if (i+1 < lines && M[i+1][j].R + distribute < 1) M[i+1][j].R += distribute;
					if (j-1 > 0 && M[i][j-1].R + distribute < 1) M[i][j-1].R += distribute;
					if (j+1 < columns && M[i][j+1].R + distribute < 1) M[i][j+1].R += distribute;
				}

				/*Checa o B*/
				if (M[i][j].B > 1) {
					distribute = (M[i][j].B - 1) / 4;
					M[i][j].B = 1;

					/*Os if's checam se os vizinhos não estão na borda e não serão estourados*/
					if (i-1 > 0 && M[i-1][j].B + distribute < 1) M[i-1][j].B += distribute;
					if (i+1 < lines && M[i+1][j].B + distribute < 1) M[i+1][j].B += distribute;
					if (j-1 > 0 && M[i][j-1].B + distribute < 1) M[i][j-1].B += distribute;
					if (j+1 < columns && M[i][j+1].B + distribute < 1) M[i][j+1].B += distribute;
				}
			}
		}

		/*Laço para atualizar G*/
		for (i = 1; i < lines - 1; i++) {
			#pragma omp parallel for num_threads(nr_proc) schedule(dynamic)
			for (j = 1; j < columns - 1; j++) {
				gx = M[i][j].Rx + M[i][j].Bx;
				gy = M[i][j].Ry + M[i][j].By;
				g = sqrt((gx*gx) + (gy*gy));
				angle = 2 * PI * g;
				M[i][j].ang += angle;
				
				while (M[i][j].ang > 2 * PI)
					M[i][j].ang -= 2*PI;
				
				M[i][j].G = M[i][j].ang / (2 * PI);
			}
		}
	}
	

	/*Escreve no arquivo de saída*/
	arq2 = fopen(outfile, "w");

   	if (arq2 == NULL)
	    printf("Erro, não foi possível abrir o arquivo\n");
	else {

	    fprintf(arq2, "P3\n%d  %d\n255\n", columns, lines);
	    
	    
    	for (i = 0; i < lines; i++)
			for (j = 0; j < columns; j++)
				fprintf(arq2, "%d  %d  %d\n",
			   		(int)(RGB_SIZE* M[i][j].R), (int)(RGB_SIZE* M[i][j].G), (int)(RGB_SIZE* M[i][j].B));

	    fprintf(stdout, "A imagem foi salva no arquivo: %s\n", outfile);
	    fclose(arq2);

	}

	for (i = 0; i < lines; i++) {
		free(M[i]);
	}
	free(M);

 	return 0;
}