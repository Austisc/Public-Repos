#include "cuda_runtime.h"
#include "device_launch_parameters.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h> 
#include <iostream>
#include <omp.h>
/*

Kernel Function for matrix initialisation
@matrix int* address of matrix
@numRows int number of rows
@numCOls int number of cols

*/

__global__ void MatInit(int* matrix, int numRows, int numCols) {
    int row = blockIdx.x * blockDim.x + threadIdx.x;
    int col = blockIdx.y * blockDim.y + threadIdx.y;
    //row and col for 1D Mat
    if (row < numRows && col < numCols) { //so not all will have have been initliase
        matrix[row * numCols + col] = row*col;//initialise as 1
    }
}
/*

Kernel Function to transpose matrix
@matrix int* address of input
@matrix int* address of output
@numRows int number of rows
@numCols int number of cols

*/
__global__ void TransposeKernel(const int* input, int* output, int numRows, int numCols) {
    int row = blockIdx.x * blockDim.x + threadIdx.x; //index of row
    int col = blockIdx.y * blockDim.y + threadIdx.y;//index of col

    if (row < numRows && col < numCols) {
        output[row * numCols + col] = input[col * numRows + row]; //to transpose
    }
}

/*
Host Function for matrix initialisation

@numRows int number of rows
@numCOls int number of cols
@return matrix int* 
*/

int* InitializeMatrix(int numRows, int numCols) {
    int* c_matrix;
    int* new_matrix = (int*)malloc(numRows * numCols * sizeof(int));
    // allocate device memory for devices
    cudaMalloc((void**)&c_matrix, numRows * numCols * sizeof(int));

    // define thread block and grid dimension
    dim3 dimBlock(16, 16);
    dim3 gridSize((numCols + dimBlock.x - 1) / dimBlock.x, (numRows + dimBlock.y - 1) / dimBlock.y); //using dimBlock to determine gridSize

    // launch kernel to initialize the matrix on the device
    MatInit << <gridSize, dimBlock >> > (c_matrix, numRows, numCols);


    // copy matrix generate by kernel function MatInit from device to host
    cudaMemcpy(new_matrix, c_matrix, numRows * numCols * sizeof(int), cudaMemcpyDeviceToHost);
    // free device memory
    cudaFree(c_matrix);
    return new_matrix;

}


/*
Host Function for matrix transpose
using kernel function to transpose
@numRows int number of rows
@numCOls int number of cols
@return matrix int*
*/


int* Transpose(int* Mat, int numRows, int numCols) {


    int* d_input = NULL;
    int* d_output = NULL;
    int* output = (int*)malloc(numRows * numCols * sizeof(int));

    // allocate device memory for input and output matrix
    cudaMalloc((void**)&d_input, numRows * numCols * sizeof(int));
    cudaMalloc((void**)&d_output,numRows *numCols * sizeof(int)); 

    // copy input matrix for transpose from host to device
    cudaMemcpy(d_input,Mat,numRows * numCols * sizeof(int), cudaMemcpyHostToDevice);

    // define thread block and grid dimensions
    dim3 dimBlock(16,16);
    dim3 gridSize((numCols + dimBlock.x - 1) / dimBlock.x, (numRows + dimBlock.y - 1) / dimBlock.y); //using dimBlock to determine gridSize
    

    // launch the transpose kernel function
    TransposeKernel << <gridSize, dimBlock >> > (d_input, d_output, numRows, numCols);

    // copy the transposed matrix from device to host
    cudaMemcpy(output, d_output, numRows * numCols * sizeof(int), cudaMemcpyDeviceToHost);

    // free device memory
    cudaFree(d_input);
    cudaFree(d_output);
    //return the transposed matrix
    return output;
}
//constant for L and G
#define BLOCKSIZE 128

#define L 32
#define G 32
/*
Kernel Function for matrix Multiplicaiton
it deal with sub-grpup of matrix deifned by LxL which it will produce LxLxG matrix
thus, it will only deal with sizes of matrices that is multiplicaiton of 32 usign the techniques from research paper
//Step I: global memory reduction
//Step II : shared memory reductions
@matA int* MatrixA
@matB int* MatrixB
@matC int* MatrixC
@numRows int number of rows
@numCOls int number of cols
@N the original size of Matrix 
sources: https://ieeexplore.ieee.org/abstract/document/5704475
*/

__global__ void MMKernel(int* MatA, int* MatB, int* MatC, int numRows, int numCols, int N) {
        //Coalesced global memory asses
        // it is all loaded from globl memory, Each thread will be sharing the same block
        // which it can lead to On-fly computing which it  while reading , it will not copy the local memory,
        // but accumlates as a partial sum at PS __shared__ blocksize which for each thread
        __shared__ int PS[BLOCKSIZE];
        unsigned int i = blockIdx.z *(BLOCKSIZE) + threadIdx.x;
        unsigned int tid = threadIdx.x;
        unsigned int gridSize = BLOCKSIZE * gridDim.z;

        PS[tid] = 0;

        // Step I: global memory reduction 
    
        while (i < N) {
            
            PS[tid] += MatA[blockIdx.x * L + i] * MatB[blockIdx.x * L + i]; // compute the sharing 
            i += gridSize;
        }

 
    __syncthreads();

    // Step 2: shared memory reduction
    //  Bank conflitc t free shared memory usage.
        //since there is an shared memory, parallel reduction flow can avoids the bank conflicts , which across the conflict shared memory 
        // by making threads to share the memory, which half of the threads can immediatelly access the result from the shared memory.
        // 
        // 
         //Loop Unrolling 
         // using reverse binary tree flow without for loop since GPU for loop has efficinecy and since we will use 2^N which =< BlockSize
         // 
         // 
         // removing number of threads synchronization , which does not exceed 32, so synthread is removed for the last iteration
         // since active threads during last 6 iteration is inside the same wrap
         // 
         // 
         // 
         // 
        //since it is outside the wrap ,for threads id >64 so, __synthread() is needed
    
        if (BLOCKSIZE >= 256) {
            if (tid < 128) {
                PS[tid] += PS[tid + 128];
            }
            __syncthreads();
        }
        if (BLOCKSIZE >= 128) {
            if (tid < 64) {
                PS[tid] += PS[tid + 64];
            }
            __syncthreads();
        }
        //sicne it is inside the wrap for threads id =<64 so, __synthread() is not needed
        if (tid < 32) {
            if (BLOCKSIZE >= 64) { PS[tid] += PS[tid + 32]; 
            }
            if (BLOCKSIZE >= 32) { PS[tid] += PS[tid + 16];
            }
            if (BLOCKSIZE >= 16) { PS[tid] += PS[tid + 8]; 
            }
            if (BLOCKSIZE >= 8) { PS[tid] += PS[tid + 4]; 
            }
            if (BLOCKSIZE >= 4) { PS[tid] += PS[tid + 2];  
            }
            if (BLOCKSIZE >= 2) { PS[tid] += PS[tid + 1];
            }
           
        }
        // it will sub the last index
        if (tid == 0) {
            int index = blockIdx.x*L*G + blockIdx.y*G + blockIdx.z ;
            MatC[index] = PS[tid];
        }
        __syncthreads();
    }


/*
Host  Function for matrix Multiplicaiton
it will partition matrix into subgroup and then the subgroup matrix will be done by sub-group matrix multiplcation
and then subgroup will be copy to the corresponding address of Matrix C as intended
Note: only work for  32 of NxN matrix with multiplicaiton of 32 

@matA int* MatrixA
@matB int* MatrixB
@numRows int number of rows
@numCOls int number of cols
@ return matC int* MatrixC
*/


int*  matrixMM(int* MatA, int* MatB, unsigned int numRows, unsigned int numCols) {

        int* subMatA = NULL;
        int* subMatB = NULL;
        int* subMatC = NULL;

        //initialise all the matrix
        //allocate memory
        int* MatC = (int*)malloc(numRows * numCols  * sizeof(int));
        
       
        // partition it into N/L X N/L groups with each subgroup got size of LxL matrixes
        for (unsigned int i = 0; i < numRows / L; i++) 
            for (unsigned int j = 0; j < numCols / L; j++) {

              
                subMatA = (int*)malloc(L * numRows * sizeof(int)); // it will have L xN subgroup of Matrix A for subgroup MM
                subMatB = (int*)malloc(L * numCols * sizeof(int)); // it will have L xN subgroup of Matrix B for subgroup MM
                subMatC = (int*)malloc(L * L * G * sizeof(int));// it will have L xL xG subgroup of Matrix C using kernel function MM

                // partition sub-group of Mat A and Mat B from i×L to ((i+1)×L-1) but respresent into 1D array 
                //using #pragma to improve the intilialisation of subMatrix
                #pragma omp parallel for collapse(2)
                for (unsigned int row = 0; row < L; row++) {
                    for (unsigned int col = 0; col < numCols; col++) {
                        unsigned int ind = (i * L + row) * numCols + col;

                        if (ind < numRows * numCols) {
                            subMatA[row * numCols + col] = MatA[ind]; 
                            subMatB[row * numCols + col] = MatB[ind];
                        }
                    }
                }
                int* c_MatA = NULL;
                int* c_MatB = NULL;
                int* c_MatC = NULL;
               
                cudaMalloc((void**)&c_MatA, L * numRows * sizeof(int));
                cudaMalloc((void**)&c_MatB, L * numCols * sizeof(int));
                cudaMalloc((void**)&c_MatC, L * L * G * sizeof(int));
                cudaMemset(c_MatA, 0, L * numRows * sizeof(int));
                cudaMemset(c_MatB, 0, L * numCols * sizeof(int)); //set all index in devices to 0
                cudaMemset(c_MatC, 0, L * L * G * sizeof(int)); //set all index in devices to 0
               
                cudaMemcpy(c_MatA, subMatA, L * numRows * sizeof(int), cudaMemcpyHostToDevice); //copy to the device memory
                cudaMemcpy(c_MatB, subMatB, L * numCols * sizeof(int), cudaMemcpyHostToDevice); //copy to the device memory

                // define thread block and grid dimensions
                dim3 dimBlock(BLOCKSIZE, 1, 1); //BlockSize will be using size 256
                dim3 gridSize(L, L, G); // it will initialise gridSize LxLxG which the same for matrix output


                //laucnh the Matrix Multiplcation kernal function
                MMKernel << <gridSize, dimBlock >> > (c_MatA, c_MatB, c_MatC, L, L, numRows);
                cudaMemcpy(subMatC, c_MatC, L * L * G * sizeof(int), cudaMemcpyDeviceToHost); //copy from device to host

                int N = numRows;
               
                //reduce each partial result according to a and b which is respective to i and j and reduce to a single value for Mat(i,j)
                int sum;
                //using omp to improve parallelisaiton with reduction fo the sum
                #pragma omp parallel for collapse(2) redunction(+:sum)
                for (unsigned int a = 0; a < L; a++) {
                    for ( unsigned int b = 0; b < L; b++) {
                        int sum = 0;
                        for (unsigned int c = 0; c < G; c++) {
                            sum += subMatC[a *(L )* G + b * G + c];//reduce the partial result to a single Value

                        }
                        
                        unsigned int index = (i * L + a) * numCols + (j*L+ b);//index to assign to MatC
                        /*printf("Index: %d\n", index);*/
                        MatC[index] = sum;
                       
                    }
                }
                cudaFree(c_MatA);
                cudaFree(c_MatB);
                cudaFree(c_MatC);
            }
        //free all memory
       
        free(subMatA);
        free(subMatB);
        free(subMatC);

        return MatC;
    }




int main(int argc, char* argv[]) {
    const unsigned int numRows = pow(2,10); //  as long as it is multiplicaiton of L and power of 2
    const unsigned int numCols = pow(2,10); // as long as it is multiplicaiton of L  and power of 2

    // call the function to initialize the matrix
    int* h_matrix = InitializeMatrix(numRows, numCols);  // initialise Matrix A
    int* g_matrix = InitializeMatrix(numRows, numCols); //initialise Matrix B 
    int* t_matrix = Transpose(g_matrix, numRows, numCols); //tranpose  Matrix B 

    clock_t start_timeA = clock();

    //Naive Multiplication
    int* MatC = (int*)malloc(numCols * numRows * sizeof(int));
    for (int i = 0; i < numRows; i++) {
        for (int j = 0; j < numCols; j++) {
            MatC[i * numCols + j] = 0;
            for (int k = 0; k < numCols; k++) {
                MatC[i * numCols + j] += h_matrix[i * numCols + k] * g_matrix[k * numCols + j];
            }
        }
    }
    clock_t end_timeA = clock();// end time
    double elapsed_secondAs = static_cast<double>(end_timeA - start_timeA) / CLOCKS_PER_SEC; // time finish MM
    
    clock_t start_time = clock();
    //Matrix Multiplication calculation
    int* MatCC = matrixMM(h_matrix, t_matrix, numRows, numCols);
    //for lop to indicates the correctness of the matrix since both are 1 matrices in my cases, each Mat(i,j) will equal to N
    clock_t end_time = clock();// end time
    double elapsed_seconds = static_cast<double>(end_time - start_time) / CLOCKS_PER_SEC; // time finish MM
    int br = 0;
   
    for (int i = 0; i < numRows; i++) {
        int br = 0;
        for (int j = 0; j < numCols; j++) {
           
            int index = i * numCols + j;
          
            if (MatCC[index] !=  MatC[index]) {
                br = 1;
                //printf(" % d, % d ", i, j);
                break;
               
            }
        }
        if (br) {
            break;
        }

      
        
    }

 
    if((br==0)) {// if it is completed, it will indicates time
        std::cout << "Elapsed time of  Naive Matrix Multiplication : " << elapsed_secondAs << " seconds\n" << std::endl;
        std::cout << "Elapsed time of  CUDA Matrix Multiplication : " << elapsed_seconds << " seconds\n" << std::endl;
    }
    else { // if it is bad, it will not indicates time
        std::cout << "Else , It is incorrect "<< std::endl; // if there is an error
    }
    free(g_matrix);
    free(MatCC);
    return 0;
}
