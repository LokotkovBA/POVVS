
#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include <stdio.h>

#define M 381 //21*7 + 234
#define N 512

__global__ void func_Kernel(char *A, char *B, char *C, double *D, double *X, int s)
{
	//----------------------------------------------------------------тут начинается пункт 3----------------------------------------------------------
	/*int idx_thread = blockIdx.x * blockDim.x + threadIdx.x;
	int i, j;

	for (i = 0; i < N; i++)
	{
		for (j = 0; j < 2 * M; j++)
		{
			X[idx_thread*s + i] = (double)A[idx_thread*s + i] * X[idx_thread*s + i] * (X[idx_thread*s + i] * C[idx_thread*s + i] + B[idx_thread*s + i]) / D[idx_thread*s + i];
		}
	}*/
	//----------------------------------------------------------------тут заканчивается пункт 3----------------------------------------------------------

	//----------------------------------------------------------------тут начинается пункт 5----------------------------------------------------------
	// blockIdx.x	- номер блока в задаче
	// blockDim.x	- размер блока
	// threadIdx.x	- номер потока в текущем блоке
	// gridDim.x	- размер сетки в блоках
	__shared__ char as[N], char bs[N], char cs[N];
	__shared__ double xs[N], double ds[N];

	int i_thread = threadIdx.x;
	int idx_thread = blockIdx.x * blockDim.x + threadIdx.x; // номер потока
	int threadCountLocal = blockDim.x; //размерность блока
	int threadCountGlobal = gridDim.x * blockDim.x;


	for (int i = idx_thread; i < N; i += threadCountGlobal)
	{
		as[i_thread] = A[i];
		bs[i_thread] = B[i];
		cs[i_thread] = C[i];
		ds[i_thread] = D[i];
		xs[i_thread] = X[i];
		for (int j = 0; j < 2 * M; j++)
		{
			xs[i_thread] = (double)A[i] * xs[i_thread] * (xs[i_thread] * C[i] + B[i]) / D[i]; //(double)(A[i] * xs[i_thread] + C[i] * xs[i_thread]) / B[i]; 
		}
		X[i] = xs[i_thread];
	}

	//----------------------------------------------------------------тут заканчивается пункт 5----------------------------------------------------------

}

int main()
{
	char A[N], B[N], C[N];

	double X[N], D[N];

	char *dev_a, *dev_b, *dev_c;
	double *dev_d, *dev_x;

	int steps, amIt;
	int blocks[3] = { 1, 2, 4 };
	int blocksize[4] = { 1,4,32,256 };
	float srd = 0;
	float elapsedTime;
	int i, j;

	/*for(i = 0; i < N; i++)
	{
		for (j = 0; j < 2 * M; j++)
		{
			X[i] = (double)A[i] * X[i] * (X[i] * C[i] + B[i]) / D[i];
		}
	}*/


	while (true)
	{
		printf("Number of iterations: ");
		scanf("%d", &amIt);

		int numIt = amIt;
		

		for (int numB = 0; numB < 3; numB++)
		{
			for (int numT = 0; numT < 4; numT++)
			{
				amIt = numIt;
				while (amIt > 0)
				{
					cudaEvent_t start, stop; //ids of events
					cudaEventCreate(&start); //inits of events
					cudaEventCreate(&stop);

					for (i = 0; i < N; i++)
					{
						A[i] = i + 1;
						B[i] = i + 2;
						C[i] = i + 3;
						D[i] = i + 4;
					}



					// Choose which GPU to run on, change this on a multi-GPU system.
					cudaSetDevice(0);


					// Allocate GPU buffers for five vectors
					cudaMalloc((void**)&dev_a, N * sizeof(char));
					cudaMalloc((void**)&dev_b, N * sizeof(char));
					cudaMalloc((void**)&dev_c, N * sizeof(char));
					cudaMalloc((void**)&dev_d, N * sizeof(double));
					cudaMalloc((void**)&dev_x, N * sizeof(double));

					//blocks = 1; blocksize = 1;
					steps = (int)N / (blocks[numB]*blocksize[numT]);

					cudaEventRecord(start, 0); //capture of event start

					// Copy input vectors from host memory to GPU buffers.
					cudaMemcpy(dev_a, A, N * sizeof(char), cudaMemcpyHostToDevice);
					cudaMemcpy(dev_b, B, N * sizeof(char), cudaMemcpyHostToDevice);
					cudaMemcpy(dev_c, C, N * sizeof(char), cudaMemcpyHostToDevice);
					cudaMemcpy(dev_d, D, N * sizeof(double), cudaMemcpyHostToDevice);
					cudaMemcpy(dev_x, X, N * sizeof(double), cudaMemcpyHostToDevice);





					// Launch a kernel on the GPU with one thread for each element.
					func_Kernel << <blocks[numB], blocksize[numT] >> > (dev_a, dev_b, dev_c, dev_d, dev_x, steps);

					// cudaDeviceSynchronize waits for the kernel to finish
					cudaDeviceSynchronize();

					// Copy output vector from GPU buffer to host memory.
					cudaMemcpy(X, dev_x, N * sizeof(double), cudaMemcpyDeviceToHost);

					cudaFree(dev_a);
					cudaFree(dev_b);
					cudaFree(dev_c);
					cudaFree(dev_d);
					cudaFree(dev_x);

					cudaEventRecord(stop, 0); //capture of event stop
					cudaEventSynchronize(stop); //waits for an event to complete
					cudaEventElapsedTime(&elapsedTime, start, stop); //computes the elapsed time between events

					//printf("[%d]Elapsed time of GPU computing = %f ms\n", numIt - amIt + 1, elapsedTime);

					srd += elapsedTime;

					// cudaDeviceReset must be called before exiting in order for profiling and
					// tracing tools such as Nsight and Visual Profiler to show complete traces.
					cudaDeviceReset();
					amIt--;
				}

				srd /= numIt;
				printf("[blocks: %d, threads: %d]Average elapsed time of GPU computing = %f ms\n", blocks[numB], blocksize[numT], srd);
				printf("============================================================\n");
				srd = 0;
			}

		}
	}

	return 0;
}

/*

// Helper function for using CUDA to add vectors in parallel.
cudaError_t addWithCuda(int *c, const int *a, const int *b, unsigned int size)
{
    int *dev_a = 0;
    int *dev_b = 0;
    int *dev_c = 0;
    cudaError_t cudaStatus;

    // Choose which GPU to run on, change this on a multi-GPU system.
    cudaStatus = cudaSetDevice(0);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaSetDevice failed!  Do you have a CUDA-capable GPU installed?");
        goto Error;
    }

    // Allocate GPU buffers for three vectors (two input, one output)    .
    cudaStatus = cudaMalloc((void**)&dev_c, size * sizeof(int));
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMalloc failed!");
        goto Error;
    }

    cudaStatus = cudaMalloc((void**)&dev_a, size * sizeof(int));
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMalloc failed!");
        goto Error;
    }

    cudaStatus = cudaMalloc((void**)&dev_b, size * sizeof(int));
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMalloc failed!");
        goto Error;
    }

    // Copy input vectors from host memory to GPU buffers.
    cudaStatus = cudaMemcpy(dev_a, a, size * sizeof(int), cudaMemcpyHostToDevice);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMemcpy failed!");
        goto Error;
    }

    cudaStatus = cudaMemcpy(dev_b, b, size * sizeof(int), cudaMemcpyHostToDevice);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMemcpy failed!");
        goto Error;
    }

    // Launch a kernel on the GPU with one thread for each element.
    addKernel<<<1, size>>>(dev_c, dev_a, dev_b);

    // Check for any errors launching the kernel
    cudaStatus = cudaGetLastError();
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "addKernel launch failed: %s\n", cudaGetErrorString(cudaStatus));
        goto Error;
    }
    
    // cudaDeviceSynchronize waits for the kernel to finish, and returns
    // any errors encountered during the launch.
    cudaStatus = cudaDeviceSynchronize();
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaDeviceSynchronize returned error code %d after launching addKernel!\n", cudaStatus);
        goto Error;
    }

    // Copy output vector from GPU buffer to host memory.
    cudaStatus = cudaMemcpy(c, dev_c, size * sizeof(int), cudaMemcpyDeviceToHost);
    if (cudaStatus != cudaSuccess) {
        fprintf(stderr, "cudaMemcpy failed!");
        goto Error;
    }

Error:
    cudaFree(dev_c);
    cudaFree(dev_a);
    cudaFree(dev_b);
    
    return cudaStatus;
}*/
