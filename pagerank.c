/*
File:			pagerank.c
Purpose:		PageRank computation using MATLAB
Authors:		Rahul Gupta and Sanid Singhal
Student #s:		64014079 and 75252882
CS Accounts:	a8w1b and q7v1b
Date:			November 25, 2018
*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "engine.h"

/*Declaring constants */
#define  SIZE_BUFFER 256
#define  MATRIX "web.txt"

/* Functions to be used in main */
int matrix_dimension(FILE* matrixfile);
double* parse_m(FILE* matrixfile, int dim);
double* transpose_m(double* prev_matrix, int dim);
void matlab_calculator (Engine *ep);

int main(void) {

	/* Declaring variables */
	Engine *ep = NULL; // A pointer to a MATLAB engine object
	mxArray *matrix_ml = NULL, *result = NULL;
	double* matrix;
	int dim = 0, err = 0, i = 0;
	size_t Result_size, m = 0;
	FILE* matrixfile = NULL;

	/* Open file containing matrix */
	err = fopen_s(&matrixfile, MATRIX, "r");
	if (err) {
		fprintf(stderr, "Error in opening the file: %s\n");
		system("pause");
		return 1;
	}

	if (matrixfile) {

		/* Using function call to get the matrix length*/
		dim = matrix_dimension(matrixfile);
		if (dim == 0) {
			printf("The file does not contain any matrix \n");
			system("pause");
			return 1;
		} 

		/* Storing and Transposing the MATRIX from the file */
		else {
			matrix = transpose_m(parse_m(matrixfile, dim), dim);
		}
	}
	else {
		fprintf(stderr, "Error in parsing the matrix file: %s\n");
		system("pause");
		return 1;
	}
	
	/* Starts MATLAB engine */
	if ( !(ep = engOpen(NULL)) ) {
		fprintf(stderr, "\nCan't start MATLAB engine\n");
		system("pause");
		return 1;
    }

	/* Copying the matrix in MATLAB */
	matrix_ml = mxCreateDoubleMatrix(dim, dim, mxREAL);
	memcpy((void*) mxGetPr(matrix_ml), (void *)matrix, dim * dim * sizeof(double)); // one-dimensional array so that it is contiguous in the heap

	/* Passes the matrix into the MATLAB engine */
	if ( engPutVariable(ep, "ConnectivityMatrix", matrix_ml) ) {
        fprintf(stderr, "\nCannot write the array to MATLAB \n");
        system("pause");
        exit(1); // Same as return 1;
    }

	/* Function that performs all the algorithm in MATLAB */
	matlab_calculator (ep);

	/* Getting the  calculated pagerank from MATLAB engine */
    if ((result = engGetVariable(ep,"PageRank")) == NULL) {
        fprintf(stderr, "\nFailed to get the pagerank\n");
        system("pause");
        exit(1);
    } 

	/* Printing out the result */
	else {
        Result_size = mxGetNumberOfElements(result);
        printf("NODE  RANK\n");
		printf("\n");
        for (m = 0; m < Result_size; ++m) {
		    printf("%2d%10.4f\n", m+1, *(mxGetPr(result) + m) );
        }
		printf("\n");
    }	

	/* Allocated Memory needs to be free */
	mxDestroyArray(matrix_ml);
    mxDestroyArray(result);
	free(matrix);
	matrix_ml = NULL;
    result = NULL;
	matrix = NULL;

	/* Closes MATLAB engine */
    if ( engClose(ep) ) {
		fprintf(stderr, "\nFailed to close MATLAB engine\n");
    }

	system("pause"); // To read the terminal window
    return 0; 
}

/*  
	Function that returns the dimension of the square matrix
*/
int matrix_dimension(FILE* matrixfile) {
	
	/* Declare variables */
	int dim = 0;
	char line[SIZE_BUFFER];

	/* Checks for empty file */
	if(!fgets(line, SIZE_BUFFER, matrixfile)) {
		return 0;
	}

	/* Calculates dimension from line*/
	//Taken from the previous LAB 4
	dim = (int)strlen(fgets(line, SIZE_BUFFER, matrixfile));	
	fseek(matrixfile, 0, SEEK_SET);

	if (strchr(line, '\r') != NULL) {
		dim = dim- 2;
	}
	else {
		dim = dim-1;
	}

	return (dim - 1) / 2 + 1;
}

/*  
	Allocates memory in heap to store the connectivity matrix
	RETURN: pointer to array of doubles containing the values in the matrix
*/
double* parse_m(FILE* matrixfile, int dim)
{
	/* Declare variables */
	int  clm, i = 0;
	double* matrix = NULL;
	char line[SIZE_BUFFER];

	/* Allocates memory in heap */
	matrix = (double*)calloc(dim * dim, sizeof(double));

	/* Stores each number in file to cell in matrix */
	while (fgets(line, SIZE_BUFFER, matrixfile)) {
		for (clm = 0; clm < 2 * dim - 1; clm += 2) {
			matrix[i] = line[clm] - '0'; // character in file
			i++;
		}
	}

	return matrix;
}

/*  
	Transposes a matrix
	RETURN: pointer to array of transposed matrix
*/
double* transpose_m(double* prev_matrix, int dim) {
	
	/* Declare variables */
	double* matrix_new = NULL;
	int i, j;

	/* Memory Allocation used from the previous LAB */
	
	/* Allocates memory in heap for new matrix */
	matrix_new = (double*)calloc(dim * dim, sizeof(double));
	
	/* Transposes values in old matrix and stores them in new matrix (switches columns and rows) */
	for(i = 0; i < dim; i++) {
		for(j = 0; j < dim; j++) {
			matrix_new[i * dim + j] = prev_matrix[i + dim * j]; 			
		}
	}

	/* Free memory of the old matrix */
	free(prev_matrix);
	prev_matrix = NULL;

	return matrix_new;
}

/*  
	Wrapper function containing MATLAB commands to be executed for the PageRank algorithm
	PARAM: pointer to MATLAB engine
*/
void matlab_calculator (Engine *ep) {
	/* Checking for the error in the MATRIX */
	
	if ( engEvalString(ep, "[rows, columns] = size(ConnectivityMatrix)")  ) {
        fprintf(stderr, "\nError calculating pagerank  \n");
        system("pause");
        exit(1);
    }
	if ( engEvalString(ep, "dim = size(ConnectivityMatrix, 1)")  ) {
        fprintf(stderr, "\nError calculating pagerank  \n");
        system("pause");
        exit(1);
    }
	if ( engEvalString(ep, "columnsums = sum(ConnectivityMatrix, 1)")  ) {
        fprintf(stderr, "\nError calculating pagerank  \n");
        system("pause");
        exit(1);
    }
	if ( engEvalString(ep, "p = 0.85")  ) {
        fprintf(stderr, "\nError calculating pagerank  \n");
        system("pause");
        exit(1);
    }
	if ( engEvalString(ep, "zerocolumns = find(columnsums~=0)")  ) {
        fprintf(stderr, "\nError calculating pagerank  \n");
        system("pause");
        exit(1);
    }
	if ( engEvalString(ep, "D = sparse( zerocolumns, zerocolumns, 1./columnsums(zerocolumns), dim, dim)")  ) {
        fprintf(stderr, "\nError calculating pagerank  \n");
        system("pause");
        exit(1);
    }
	if ( engEvalString(ep, "StochasticMatrix =  ConnectivityMatrix * D")  ) {
        fprintf(stderr, "\nError calculating pagerank  \n");
        system("pause");
        exit(1);
    }
	if ( engEvalString(ep, "[row, column] = find(columnsums==0)")  ) {
        fprintf(stderr, "\nError calculating pagerank  \n");
        system("pause");
        exit(1);
    }
	if ( engEvalString(ep, "StochasticMatrix(:,column) = 1./dim")  ) {
        fprintf(stderr, "\nError calculating pagerank  \n");
        system("pause");
        exit(1);
    }
	if ( engEvalString(ep, "Q = ones(dim, dim)")  ) {
        fprintf(stderr, "\nError calculating pagerank  \n");
        system("pause");
        exit(1);
    }
	if ( engEvalString(ep, "TransitionMatrix = p * StochasticMatrix + (1 - p) * (Q/dim)")  ) {
        fprintf(stderr, "\nError calculating pagerank  \n");
        system("pause");
        exit(1);
    }
	if ( engEvalString(ep, "PageRank = ones(dim, 1)")  ) {
        fprintf(stderr, "\nError calculating pagerank  \n");
        system("pause");
        exit(1);
    }
	if ( engEvalString(ep, "for i = 1:100 PageRank =  TransitionMatrix * PageRank; end")  ) {
        fprintf(stderr, "\nError calculating pagerank  \n");
        system("pause");
        exit(1);
    }
	if ( engEvalString(ep, "PageRank = PageRank / sum(PageRank)")  ) {
        fprintf(stderr, "\nError calculating pagerank  \n");
        system("pause");
        exit(1);
    }
}

