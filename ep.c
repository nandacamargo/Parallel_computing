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
    Pixel **M, **M2/*, **aux*/; /*Matriz de pixels*/

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
    /*nr_threads = 2 * nr_proc;*/ /*Considerando hyperthread*/

    /*id = malloc(nr_threads * sizeof(int));*/

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

		        /*if (DEBUG) printf("Ignorando comentários...\n");*/
		    }
			else if (cont == 0) {
				ungetc(a[0], arq1);
				fscanf(arq1,"%d %d\n", &columns, &lines);
				fscanf(arq1,"%d\n", &comp_max_val);
				cont++;

				/*if (DEBUG) {
					printf("Num_linhas = %d, num_colunas = %d\n", lines, columns);
					printf("comp_max_val = %d\n", comp_max_val);
				}*/

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
		    		    fscanf(arq1, "%lf %lf %lf", &M2[i][j].R, &M2[i][j].G, &M2[i][j].B);
		    		    M2[i][j].R /= RGB_SIZE;
		    		    M2[i][j].G /= RGB_SIZE; 
		    		    /*M2[i][j].G = (2*PI * M2[i][j].G) / RGB_SIZE; */
		    		    M2[i][j].B /= RGB_SIZE;
		    		    M2[i][j].ang = 2*PI * M[i][j].G;

						/* Calcular Rx, Ry, Bx e By quando ler a entrada \/*/
						M2[i][j].Rx = horizontal_component(M2[i][j].R, M2[i][j].G);
						M2[i][j].Bx = (-1) * horizontal_component(M2[i][j].B, M2[i][j].G);
						M2[i][j].Ry = vertical_component(M2[i][j].R, M2[i][j].G);
						M2[i][j].By = (-1) * vertical_component(M2[i][j].B, M2[i][j].G);
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

		/*aux = M;
		M = M2;
		M2 = aux;*/
		/*cp(M2, M, lines, columns);*/

		if (lines - 2 < nr_proc) nr_proc = 1;

		/*#pragma omp parallel for private(i, j, val, thread_num, num_threads, start,end, rest) {*/
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

		    /*if (DEBUG) {
			    printf("thread =%d start=%d end=%d\n", thread_num, start, end);
			}*/

			/*for (i = 1; i < lines - 1; i++) { */
 			
 			for (i = start; i < end; i++) {	/*Por causa da borda*/
				for (j = 1; j < columns - 1; j++) {

					if (M2[i][j].Rx > 0) {
						if (j != columns -1) {
							val = transfer(M2[i][j+1].R, M2[i][j].Rx);
							if (i != start && i != end) {
									M2[i][j+1].Rx += val;
									M2[i][j].Rx -= val;
							}
							else {
								#pragma omp critical 
								{
									M2[i][j+1].Rx += val;
									M2[i][j].Rx -= val;
							    }
							}	
						}
						if (j != 1) {
							val = transfer(M2[i][j-1].B, M2[i][j].Bx);
							if (i != start && i != end) {
									/*Recebe no sentido oposto*/
									M2[i][j-1].Bx += val; 
									M2[i][j].Bx -= val;
							}
							else {
								#pragma omp critical 
								{
									M2[i][j-1].Bx += val;
									M2[i][j].Bx -= val;
							    }
							}								
						}
					}
					else { /*Recebe um valor positivo*/
						if (j != 1) {
							val = transfer(M2[i][j-1].R, M2[i][j].Rx);
							if (i != start && i != end) {
									M2[i][j-1].Rx -= val;
									M2[i][j].Rx += val;
							}
							else {
								#pragma omp critical 
								{
									M2[i][j-1].Rx -= val;
									M2[i][j].Rx += val;
							    }
							}
						}

						if (j != columns - 1) {
							val = transfer(M2[i][j+1].B, M2[i][j].Bx);
							if (i != start && i != end) {
									M2[i][j+1].Bx -= val;  /*Recebe no sentido oposto*/
									M2[i][j].Bx += val;
							}
							else {
								#pragma omp critical 
								{
									M2[i][j+1].Bx -= val;
									M2[i][j].Bx += val;
							    }
							}
						}
					}

					if (M2[i][j].Ry > 0) {
						if (i != 1) {
							val = transfer(M2[i-1][j].R, M2[i][j].Ry);
							if (i != start && i != end) {
									M2[i-1][j].Ry += val;
									M2[i][j].Ry -= val;
							}
							else {
								#pragma omp critical 
								{
									M2[i-1][j].Ry += val;
									M2[i][j].Ry -= val;
							    }
							}
						}
						if (i != lines - 1) {
							val = transfer(M2[i+1][j].B, M2[i][j].By);
							if (i != start && i != end) {
									M2[i+1][j].By += val;
									M2[i][j].By -= val;
							}
							else {
								#pragma omp critical 
								{
									M2[i+1][j].By += val;
									M2[i][j].By -= val;
							    }
							}
						}
					}

					else { /*Recebe um valor positivo*/
						if (i != lines - 1) {
							val = transfer(M2[i+1][j].R, M2[i][j].Ry);
							if (i != start && i != end) {
									M2[i+1][j].Ry -= val;
									M2[i][j].Ry += val;
							}
							else {
								#pragma omp critical 
								{
									M2[i+1][j].Ry -= val;
									M2[i][j].Ry += val;
							    }
							}
						}
						if (i != 1) {
							val = transfer(M2[i-1][j].B, M2[i][j].By);
							if (i != start && i != end) {
									M2[i-1][j].By -= val;
									M2[i][j].By += val;
							}
							else {
								#pragma omp critical 
								{
									M2[i-1][j].By -= val;
									M2[i][j].By += val;
							    }
							}
						}
					}
				}
			}
		

			/*if (DEBUG) {
				printf("M:\n");
				for (i = 0; i < lines; i++) {
				    for (j = 0; j < columns; j++)
				    	printf("%f ", M[i][j].R);
				    printf("\n");
				}			
			}*/

			/*if (DEBUG) {
				printf("M2:\n");
				for (i = 0; i < lines; i++) {
				    for (j = 0; j < columns; j++)
				    	printf("%f ", M2[i][j].R);
				    printf("\n");
				}			
			}*/

		}

		/*O bloco abaixo checa se os pixels vizinhos estouraram*/
		for (i = 1; i < lines - 1; i++) {
			for (j = 1; j < columns - 1; j++) {

				/*Checa o R*/
				if (M2[i][j].R > 1) {
					distribute = (M2[i][j].R - 1) / 4;
					M2[i][j].R = 1;

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
			#pragma omp parallel for num_threads(nr_proc) schedule(dynamic)
			for (j = 1; j < columns - 1; j++) {

				gx = M2[i][j].Rx + M2[i][j].Bx;
				gy = M2[i][j].Ry + M2[i][j].By;
				g = sqrt((gx*gx) + (gy*gy));
				angle = 2 * PI * g;
				M[i][j].ang += angle;
				
				M2[i][j].G += g;
				
				if (M2[i][j].ang > 2 * PI)
					M2[i][j].ang -= 2*PI;
			}
		}
	}
	
	
    /*for (i = 0; i < lines; i++) {
		for (j = 0; j < columns; j++) {
			printf("%d %d %d", (int)(RGB_SIZE* M2[i][j].R), (int)((RGB_SIZE* M2[i][j].G) / (2*PI)), (int)(RGB_SIZE* M2[i][j].B));
		}
		printf("\n");
	}*/

	/*Escreve no arquivo de saída*/
	arq2 = fopen(outfile, "w");

   	if (arq2 == NULL)
	    printf("Erro, não foi possível abrir o arquivo\n");
	else {

		/*sprintf(outfile, "%s.ppm", outfile);*/
	    fprintf(arq2, "P3\n%d %d\n255\n", columns, lines);
	    
	    
    	for (i = 0; i < lines; i++)
			for (j = 0; j < columns; j++)
				fprintf(arq2, "%d %d %d \n",
			   		(int)(RGB_SIZE* M2[i][j].R), (int)(RGB_SIZE* M2[i][j].ang), (int)(RGB_SIZE* M2[i][j].B));

	    fprintf(stdout, "A imagem foi salva no arquivo: %s\n", outfile);
	    fclose(arq2);

	}

	for (i = 0; i < lines; i++) {
		free(M[i]);
		free(M2[i]);
	}
	free(M);
	free(M2);

 	return 0;
}