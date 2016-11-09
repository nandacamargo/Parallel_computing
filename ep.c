#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

typedef struct cell {
  /* Matrix cells */
	int r;             /* Red component */
	int g;             /* Green component */
  int b;             /* Blue component */
} Cell;

typedef struct thread {
  /* Thread informarion */
	int id;            /* Thread id */
	int pixel_i;       /* actual pixel x axis component this thread is at */
  int pixel_j;       /* actual pixel y axis component this thread is at */
  int disturbances;  /* Number of already disturbed pixels by this thread */
  int iteration;     /* number of iterations this thread performed.
                        When all pixels are disturbed, it gains +1 iteration */
  /* The following numbers delimitates the image pixels this thread will act on */
  int vil;           /* vertical inferior limit */
  int vsl;           /* vertical superior limit */
  int hil;           /* horizontal inferior limit */
  int hsl;           /* horizontal superior limit */
} Thread;

/* Constants */
#define PI 3.14159

/* Prototypes */
void *disturb(void *);
int read_image(char *);
void assign_id(Thread *, int);
int arguments_fine(int, char**);
void use();

/* Globals */
int iterations;     /* All threads must perform this number of iterations */
int image_width;    /* Image width */
int image_height;   /* Image height */
Cell** image;       /* Each matrix cell represents a pixel */

int main (int argc, char** argv)
{
  int i, processors, lin;
  pthread_t *threads;
  Thread *argt;

  /* Program params checkup */
  if(!arguments_fine(argc, argv)) {
    use(); return 1;
  }

  /* Assign program params to variables */
  iterations = atoi(argv[3]);
  if((processors = atoi(argv[4])) == 0)
    /* Get number of available processors */
    processors = sysconf(_SC_NPROCESSORS_ONLN);

  /* Reads the input information and assign values to remaining globals */
  if(!read_image(argv[1]))
    return 1;

  /* Allocate threads according to the number of available processors */
  threads = malloc(processors * sizeof(*threads));
  argt = malloc(processors * sizeof(*argt));

  /* Assign identification number to the threads */
  assign_id(argt, processors);

  /* Create and execute threads */
  for(i = 0; i < processors; i++) {
		if(pthread_create(&threads[i], NULL, disturb, &argt[i])) {
			printf("Error creating thread.");
			abort();
		}
	}
	for(i = 0; i < processors; i++) {
		if(pthread_join(threads[i], NULL)) {
			printf("Error joining thread.");
			abort();
		}
	}

  free(threads); threads = NULL;
  free(argt); argt = NULL;
  for(lin = 0; lin < image_width; lin++) {
    free(image[lin]); image[lin] = NULL;
  }
  free(image); image = NULL;
  return 0;
}

/* Parallel computing function */
void *disturb(void *argt)
{
  Thread *thread = ((Thread*) argt);

  printf("Thread number %d\n", thread->id);
  return NULL;
}

/* Assign id to threads */
void assign_id(Thread* argt, int processors)
{
  int i;

  for(i = 0; i < processors; i++) argt[i].id = i;
}

/* Reads the input file */
int read_image(char *input)
{
  char c;
  FILE *fp;

  if((fp = fopen(input, "r")) == NULL) {
    printf("Error: unable to find PPM format input. Check README.txt file for further information on what is expected.\n");
    return 0;
  }
  else {
    int lin = 0, col = 0;

    /* P3 format */
    fscanf(fp, "P3\n");

    /*Comments*/
    while((c = getc(fp)) == '#')
      while((c = getc(fp)) != '\n') continue;
    ungetc(c, fp);

    /* Image width and height */
    if(fscanf(fp, "%d %d\n", &image_width, &image_height) != 2) {
       printf("Error: something has happened while reading image size.\n");
       return 0;
    }

    /* Allocates the image matrix */
    image = malloc(image_width * sizeof(Cell*));
    for(lin = 0; lin < image_width; lin++) image[lin] = malloc(image_height * sizeof(Cell));

    /* Components maximum value */
    fscanf(fp, "255\n");

    /* Image Pixels */
    for(lin = 0; lin < image_width; lin++)
      for(col = 0; col < image_height; col++)
        fscanf(fp, "%d %d %d", &image[lin][col].r, &image[lin][col].g, &image[lin][col].b);
  }

  return 1;
}

/* Program arguments basic checkup */
int arguments_fine(int argc, char** argv)
{
  int i; char c;

  if(argc > 5) {
    printf("Error: too many arguments (%d/4).\n", argc);
    return 0;
  }
  else if(argc < 5) {
    printf("Error: too few arguments (%d/4).\n", argc);
    return 0;
  }
  for(c = argv[3][i = 0]; c != '\0'; c = argv[3][++i])
    if(c > '9' || c < '0') {
      printf("Error: argument 3 must be an integer.\n");
      return 0;
    }
  for(c = argv[4][i = 0]; c != '\0'; c = argv[4][++i])
    if(c > '9' || c < '0') {
      printf("Error: argument 4 must be an integer.\n");
      return 0;
    }

  if(atoi(argv[3]) == 0) {
    printf("Error: argument 3 must be > 0.\n");
    return 0;
  }

  return 1;
}

/* Use */
void use()
{
  printf("\nUse:\n\t./ep [A1] [A2] [A3] [A4]\nwhere:\n");
  printf("A1: Input file name.\n");
  printf("A2: Output file name.\n");
  printf("A3: Number of iterations to be done by the program.\n");
  printf("A4: Number of processors. If 0, the program will use the number of processors as default value.\n");
}
