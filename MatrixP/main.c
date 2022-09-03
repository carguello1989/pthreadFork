#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <pthread.h>
#include <time.h>       // for clock_t, clock(), CLOCKS_PER_SEC
#include <unistd.h>     // for sleep()

/* Size de la matriz */
#define MATRIX_SIZE 10

 /* Esta macro sirve para redireccionar la matrix en 2 dimensiones */
#define array(arr, i, j) arr[(int) MATRIX_SIZE * (int) i + (int) j]

/* Punteros enteros para cada matrix */
static int *matrix_a, *matrix_b, *matrix_result;

/* Argument struct for each thread */
typedef struct arg_struct
{
  int *a;
  int *b;
  int *c;
  int row;
} thread_args;

/* LLenado de matriz 1 to 10 */
void fill_matrix(int *matrix)
{
  for (int i = 0; i < MATRIX_SIZE; i++)
  {
    for (int j = 0; j < MATRIX_SIZE; j++)
    {
      array(matrix, i, j) = rand() % 10 + 1;
    }
  }
  return;
}

/* sendFile the given matrix */
void sendFile_matrix(int *matrix, int print_width, int numberMatrix)
{
  FILE* fichero;
  fichero = fopen("Mat_R.txt", "wt");

  for (int i = 0; i < MATRIX_SIZE; i++)
  {
    for (int j = 0; j <  MATRIX_SIZE; j++)
    {
        fputs("------Start------\n", fichero);
        fputs("\n------Row: \n", fichero);
        fprintf(fichero,"%d", i);
        fputs("\n------Column: \n", fichero);
        fprintf(fichero,"%d", j);
        fputs("\n------Result: \n", fichero);
        fprintf(fichero,"%d", array(matrix, i, j));
        fputs("\n------End------\n", fichero);
    }
  }
  fclose(fichero);
  return;
}

int *matrix_page(int *matrix, unsigned long m_size)
{
  matrix = mmap(0, m_size, PROT_READ | PROT_WRITE,
    MAP_SHARED | MAP_ANONYMOUS, -1, 0);
  if (matrix == (void *) -1)
  {
    exit(EXIT_FAILURE);
  }
  memset((void *) matrix, 0, m_size);
  return matrix;
}

void matrix_unmap(int *matrix, unsigned long m_size)
{
  if (munmap(matrix, m_size) == -1)
  {
    exit(EXIT_FAILURE);
  }
}

__attribute__ ((noreturn)) void row_multiply(void *row_args)
{
  thread_args *args = (thread_args *) row_args;
  for(int i = 0; i < MATRIX_SIZE; i++)
  {
    for (int j = 0; j < MATRIX_SIZE; j++)
    {
      int add = array(args->a, args->row, j) * array(args->b, j, i);
      array(args->c, args->row, i) += add;
    }
  }
  pthread_exit(0);
}

int main(void)
{

   FILE* fichero;
   fichero = fopen("Stats.txt", "wt");

   double secsFinal = 0;

   for(int i = 0; i < MATRIX_SIZE; i++){

       struct timeval start, stop;
       double secs = 0;

       gettimeofday(&start, NULL);

      /* Calculate the memory size of the matrices */
      unsigned long m_size = sizeof(int) * (unsigned long) (MATRIX_SIZE * MATRIX_SIZE);

      /* Map matrix_a, matrix_b, and matrix_c into a memory page */
      matrix_a = matrix_page(matrix_a, m_size);
      matrix_b = matrix_page(matrix_b, m_size);
      matrix_result = matrix_page(matrix_result, m_size);

      fill_matrix(matrix_a);
      fill_matrix(matrix_b);

      sendFile_matrix(matrix_a, 10, i);
      sendFile_matrix(matrix_b, 10, i);

      //user fork
      fork();


      sendFile_matrix(matrix_result, 10, i);

      /* Give back the allocated memory pages */
      matrix_unmap(matrix_a, m_size);
      matrix_unmap(matrix_b, m_size);
      matrix_unmap(matrix_result, m_size);

      gettimeofday(&stop, NULL);

      secs = (double)(stop.tv_usec - start.tv_usec) / 1000000 + (double)(stop.tv_sec - start.tv_sec);

      fputs("------Start process------\n", fichero);
      fputs("\n------Process number: \n", fichero);
      fprintf(fichero,"%d", i);
      fputs("\n------Process time SECONDS: \n", fichero);
      fprintf(fichero,"%f", secs);
      fputs("\n------End process------\n", fichero);

      secsFinal= secsFinal+ secs;
  }

  fputs("\n------Process time Final SECONDS: \n", fichero);
  fprintf(fichero,"%f", secsFinal);

  fclose(fichero);
}
