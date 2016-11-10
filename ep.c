#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>

typedef struct cell {
  /* Matrix cells */
	float r;           /* Red component */
	float g;           /* Green component */
  float b;           /* Blue component */
  int border;        /* Is a border cell? */
  /* Components */
  float theta;       /* Angle determined by green component. theta = 2*PI*g */
  float cxr;         /* Red horizontal component. cxr = r * Sin(theta) */
  float cyr;         /* Red vertical component. cyr = r * Cos(theta) */
  float cxb;         /* Blue horizontal component.
                     cxb = b * Sin(theta) with opposite direction to cxr */
  float cyb;         /* Blue vertical component.
                     cyb = b * Cos(theta) with opposite direction to cyr */
} Cell;

typedef struct thread {
  /* Thread informartion */
	int id;            /* Thread id */
	int pixel_i;       /* actual pixel x axis component this thread is at */
  int pixel_j;       /* actual pixel y axis component this thread is at */
  int iteration;     /* number of iterations this thread performed.
                        When reach hsl and vil together, it gains +1 iteration */
  /* The following numbers delimitates the image pixels this thread will act on */
  int vil;           /* vertical inferior limit */
  int vsl;           /* vertical superior limit */
  int hil;           /* horizontal inferior limit */
  int hsl;           /* horizontal superior limit */
} Thread;

/* Constants */
#define PI        3.14159
#define TRUE      1
#define FALSE     0
#define RGB_RANGE 256

/* Prototypes */
void *disturb(void *);
void image_initial_setup(Thread *);
int read_image(char *);
void assign_attr(Thread *, int);
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

  /* Assign identification number to the threads and sector limits */
  assign_attr(argt, processors);

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

  image_initial_setup(thread);

  return NULL;
}

/* Setup initial configuration of angles and components of each pixel */
void image_initial_setup(Thread *thread)
{
  for(thread->pixel_i = thread->vil; thread->pixel_i <= thread->vsl; thread->pixel_i++)
    for(thread->pixel_j = thread->hil; thread->pixel_j <= thread->hsl; thread->pixel_j++) {
      if(image[thread->pixel_i][thread->pixel_j].border) continue;
      image[thread->pixel_i][thread->pixel_j].theta = 2 * PI * image[thread->pixel_i][thread->pixel_j].g;
      image[thread->pixel_i][thread->pixel_j].cxr = image[thread->pixel_i][thread->pixel_j].r * sin(image[thread->pixel_i][thread->pixel_j].theta);
      image[thread->pixel_i][thread->pixel_j].cyr = image[thread->pixel_i][thread->pixel_j].r * cos(image[thread->pixel_i][thread->pixel_j].theta);
      image[thread->pixel_i][thread->pixel_j].cxb = (-1) * image[thread->pixel_i][thread->pixel_j].b * sin(image[thread->pixel_i][thread->pixel_j].theta);
      image[thread->pixel_i][thread->pixel_j].cyb = (-1) * image[thread->pixel_i][thread->pixel_j].b * cos(image[thread->pixel_i][thread->pixel_j].theta);
    }
}

/* Assign attributes to threads */
void assign_attr(Thread* argt, int processors)
{
  int sector_side, remaining_pixels, i;

  if(image_width >= image_height) {
    sector_side = image_width / processors;
    remaining_pixels = image_width % processors;
    for(i = 0; i < processors; i++, remaining_pixels--) {
      argt[i].id = i;
      argt[i].iteration = 0;
      if(i == 0) {
        argt[i].vil = 0;
        if(remaining_pixels > 0) argt[i].vsl = sector_side;
        else argt[i].vsl = sector_side - 1;
        argt[i].hil = 0;
        argt[i].hsl = image_height - 1;
      }
      else {
        argt[i].vil = argt[i - 1].vsl + 1;
        if(remaining_pixels > 0) argt[i].vsl = argt[i].vil + sector_side;
        else argt[i].vsl = argt[i].vil + sector_side - 1;
        argt[i].hil = 0;
        argt[i].hsl = image_height - 1;
      }
    }
  }
  else {
    sector_side = image_height / processors;
    remaining_pixels = image_height % processors;
    for(i = 0; i < processors; i++, remaining_pixels--) {
      argt[i].id = i;
      argt[i].iteration = 0;
      if(i == 0) {
        argt[i].hil = 0;
        if(remaining_pixels > 0) argt[i].hsl = sector_side;
        else argt[i].hsl = sector_side - 1;
        argt[i].vil = 0;
        argt[i].vsl = image_width - 1;
      }
      else {
        argt[i].hil = argt[i - 1].hsl + 1;
        if(remaining_pixels > 0) argt[i].hsl = argt[i].hil + sector_side;
        else argt[i].hsl = argt[i].hil + sector_side - 1;
        argt[i].vil = 0;
        argt[i].vsl = image_width - 1;
      }
    }
  }
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
      for(col = 0; col < image_height; col++) {
        fscanf(fp, "%f %f %f", &image[lin][col].r, &image[lin][col].g, &image[lin][col].b);
        image[lin][col].r = image[lin][col].r / RGB_RANGE;
        image[lin][col].g = image[lin][col].g / RGB_RANGE;
        image[lin][col].b = image[lin][col].b / RGB_RANGE;
        if(lin == 0 || lin == image_width - 1 || col == 0 || col == image_height - 1)
          image[lin][col].border = TRUE;
        else
          image[lin][col].border = FALSE;
      }
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
