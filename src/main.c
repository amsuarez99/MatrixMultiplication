// ! Requirements
// - Execute the multiplication 15 times,
// - Run the code 5 times for each execution
// - Manage double precision for all the arrays

#include <omp.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define MAX_INPUT_SIZE 100
#define CALCULATIONS 15
#define EXECUTIONS 5

long timediff(clock_t t1, clock_t t2) {
  long elapsed;
  elapsed = ((double)t2 - t1) / CLOCKS_PER_SEC * 1000;
  return elapsed;
}

typedef struct {
  char *name;
  size_t columns;
  size_t rows;
  double *data;
} Matrix;

typedef struct {
  double timeTaken;
} Execution_Metric;

Matrix *serialC;

void copySerialC(Matrix *c) {
  serialC = (Matrix *)malloc(sizeof(Matrix));
  serialC->columns = c->columns;
  serialC->rows = c->rows;
  const size_t matrixBytes = sizeof(double) * serialC->columns * serialC->rows;
  serialC->data = (double *)malloc(matrixBytes);

  if (serialC->data == NULL) {
    printf("Couldn\t separate the specified memory. Exiting...");
    exit(1);
  }

  memcpy(serialC->data, c->data, matrixBytes);
}

void verifyResultMat(Matrix *mat) {
  printf("VERIFYING RESULTS...\n");
  for (size_t rows; rows < mat->rows; rows++)
    for (size_t columns; columns < mat->columns; columns++) {
      size_t index = rows * mat->rows + columns;
      if (mat->data[index] != serialC->data[index]) {
        printf("ERROR: Results are not the same\n");
        exit(1);
      }
    }
  printf("RESULTS VERIFIED! They are the same!\n");
}

void printMetrics(Execution_Metric *metrics) {
  for (int i = 0; i < EXECUTIONS; i++) {
    printf("%lf\n", metrics[i].timeTaken);
  }
}

void printMatrix(Matrix *mat) {
  printf("PRINTING MATRIX %s\n", mat->name);
  for (size_t rows; rows < mat->rows; rows++) {
    for (size_t columns; columns < mat->columns; columns++)
      printf("%lf", *(mat->data + rows * mat->columns + columns));
    printf("\n");
  }
}

char *getInput() {
  char *input = (char *)malloc(MAX_INPUT_SIZE);
  memset(input, '\0', MAX_INPUT_SIZE);
  scanf("%s", input);
  return input;
}

Matrix *getMatrixInfo(char *matName) {
  Matrix *mat = (Matrix *)malloc(sizeof(Matrix));

  printf("How many rows does Matrix %s have? ", matName);
  scanf("%zu", &mat->rows);

  printf("How many columns does Matrix %s have? ", matName);
  scanf("%zu", &mat->columns);

  mat->data = (double *)malloc(sizeof(double) * mat->rows * mat->columns);
  mat->name = (char *)malloc(strlen(matName) + 1);
  memcpy(mat->name, matName, strlen(matName) + 1);

  if (mat->data == NULL) {
    printf("Couldn\t separate the specified memory. Exiting...");
    exit(1);
  }

  return mat;
}

void freeMatrix(Matrix *mat) {
  free(mat->data);
  free(mat);
}

void fillMatrix(Matrix *mat, bool readTanspose) {
  printf("What is the fileName for matrix %s ", mat->name);
  char *fileName = getInput();
  FILE *file = fopen(fileName, "r");
  if (file == NULL) {
    printf("Error opening file!\n");
    exit(1);
  }

  double tempFloat;
  for (size_t i = 0; i < mat->rows; i++) {
    for (size_t j = 0; j < mat->columns; j++) {
      if (feof(file)) {
        printf("The specified dimensions aren\'t met!\n");
        exit(1);
      }
      fscanf(file, "%lf\n", &tempFloat);
      if (readTanspose)
        *(mat->data + (j * mat->rows) + i) = tempFloat;
      else
        *(mat->data + (i * mat->columns) + j) = tempFloat;
    }
  }

  if (!feof(file)) {
    printf("The specified dimensions aren\'t met");
    exit(1);
  }
  fclose(file);
  free(fileName);
}

void serialProcedure(Matrix *a, Matrix *b, Matrix *c) {
  for (size_t i = 0; i < c->rows; i++)
    for (size_t j = 0; j < c->columns; j++) {
      c->data[i * c->columns + j] = 0;
      for (int k = 0; k < a->columns; k++)
        c->data[i * c->columns + j] += a->data[i * a->columns + k] * b->data[j * a->columns + k];
    }
}

void ompProcedure(Matrix *a, Matrix *b, Matrix *c) {
#pragma omp parallel for
  for (size_t i = 0; i < c->rows; i++)
    for (size_t j = 0; j < c->columns; j++) {
      c->data[i * c->columns + j] = 0;
      for (size_t k = 0; k < a->columns; k++)
        c->data[i * c->columns + j] += a->data[i * a->columns + k] * b->data[j * a->columns + k];
    }
}

void printTable(Execution_Metric *a, Execution_Metric *b) {
  printf("Run#\t|Serial\t\t|Parallel1\t\t\n");
  double avg[2] = {0, 0};
  for (int i = 0; i < EXECUTIONS; i++) {
    avg[0] += a[i].timeTaken;
    avg[1] += b[i].timeTaken;
    printf("%d\t|%lf\t|%lf\n", i + 1, a[i].timeTaken, b[i].timeTaken);
  }

  printf("promedio|%lf\t|%lf\n", avg[0], avg[1]);
  printf("%% vs serial|\t\t|%lf\n", avg[1] / avg[0]);
}

void calculateMetrics(Execution_Metric *metrics, void (*f)(Matrix *, Matrix *, Matrix *), Matrix *a, Matrix *b, Matrix *c) {
  int iteration = 0, calculation;
  clock_t start, end;
  while (iteration < EXECUTIONS) {
    calculation = 0;

    start = clock();
    while (calculation < CALCULATIONS) {
      f(a, b, c);
      calculation++;
    }
    end = clock();

    metrics[iteration].timeTaken = timediff(start, end);
    iteration++;
  }
}

Matrix *getResultMatrixSpecifications(Matrix *a, Matrix *b) {
  if (a->columns != b->rows) {
    printf("Can\'t compute the matrix multilication (The columns of A must be the same as the rows of B)");
    exit(1);
  }

  Matrix *c = (Matrix *)malloc(sizeof(Matrix));
  c->rows = a->rows;
  c->columns = b->columns;
  c->data = (double *)malloc(sizeof(double) * c->rows * c->columns);

  if (c->data == NULL) {
    printf("Couldn't separate the specified memory. Exiting...\n");
    exit(1);
  }
  return c;
}

void writeMultiplicationRes(Matrix *c) {
  FILE *file = fopen("matrixC.txt", "w");
  for (size_t i = 0; i < c->rows; i++)
    for (size_t j = 0; j < c->columns; j++)
      fprintf(file, "%0.10lf\n", c->data[i * c->columns + j]);
  fclose(file);
}

void main() {
  Matrix *a = getMatrixInfo("a");
  fillMatrix(a, false);
  Matrix *b = getMatrixInfo("b");
  fillMatrix(b, true);
  Matrix *c = getResultMatrixSpecifications(a, b);

  size_t temp = b->columns;
  b->columns = b->rows;
  b->rows = temp;

  Execution_Metric Serial_Metrics[EXECUTIONS];
  Execution_Metric OMP_Metrics[EXECUTIONS];
  Execution_Metric Intrinsics_Metrics[EXECUTIONS];

  calculateMetrics(Serial_Metrics, (void *)serialProcedure, a, b, c);
  writeMultiplicationRes(c);
  copySerialC(c);

  calculateMetrics(OMP_Metrics, ompProcedure, a, b, c);
  verifyResultMat(c);

  printTable(Serial_Metrics, OMP_Metrics);

  // calculateMetrics(Intrinsics_Metrics, intrinsicsProcedure, a, b, c);
  // verifyResultMat(c);

  freeMatrix(a);
  freeMatrix(b);
  freeMatrix(c);
  freeMatrix(serialC);
}