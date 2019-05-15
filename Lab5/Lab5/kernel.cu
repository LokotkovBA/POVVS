
#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include "targ.h"
#include <stdio.h>
#include <cstdint>
#include <conio.h>

#define N 5
unsigned char* data1;
unsigned width, height, pbpp;


__global__ void fun_kernel(unsigned char* result_data, unsigned char* data1, int height, int width, int steps)
{
	int thread = blockIdx.x * blockDim.x + threadIdx.x;

	int neww = (width - 1)*N + 1;
	int newh = (height - 1)*N + 1;
	float res, d1, d2, d3, d4, u, t;
	int h, w, p1, p2, p3, p4;

	float pom = (height - 1) / (newh - 1);
	float pom2 = (width - 1) / (neww - 1);


	for (int j = 0; j < steps; j++)
	{
		res = (thread*steps + j) * pom;
		h = (int)floor(res);
		if (h < 0) {
			h = 0;
		}
		else {
			if (h >= height - 1) {
				h = height - 2;
			}
		}
		u = res - h;
		for (int i = thread; i < neww; i++)
		{
			res = (i) * pom2;
			w = (int)floor(res);
			if (w < 0) {
				w = 0;
			}
			else {
				if (w >= width - 1) {
					w = width - 2;
				}
				t = res - w;

				/* Коэффициенты */
				d1 = (1 - t) * (1 - u);
				d2 = t * (1 - u);
				d3 = t * u;
				d4 = (1 - t) * u;

				/* Окрестные пиксели: a[i][j] */
				p1 = data1[w + h * width];
				p2 = data1[w + h * width + 1];
				p3 = data1[w + 1 + h * width + 1];
				p4 = data1[w + 1 + h * width];

				result_data[i + (thread*steps + j)* neww] = p1 * d1 + p2 * d2 + p3 * d3 + p4 * d4;
			}
		}
	}
}

int main()
{
	int blocks = 256;
	int blocksize = 512;
	if (!Targa2Array("C:/Users/B.Lokotkov/Desktop/_git/POVVS/Lab5/sample2.tga", &data1, &width, &height, &pbpp))
	{
		std::cout << "Can't read file";
		return -1;
	}
	int neww = (width - 1)*N + 1;
	int newh = (height - 1)*N + 1;
	int steps;
	float elapsedTime;
	cudaEvent_t start, stop; //Индентификатор событий
	cudaEventCreate(&start);
	cudaEventCreate(&stop);

	unsigned char* dev_data;
	unsigned char* dev_result_data;
	unsigned char* result_data = new unsigned char[neww * newh];

	cudaMalloc((void**)&dev_data, width * height * sizeof(unsigned char));
	cudaMalloc((void**)&dev_result_data, neww * newh * sizeof(unsigned char));

	steps = (int)newh / (blocks*blocksize);

	cudaEventRecord(start, 0); //Фиксация события start
	cudaMemcpy(dev_data, data1, height * width * sizeof(unsigned char), cudaMemcpyHostToDevice);
	cudaMemcpy(dev_result_data, result_data, height * width * sizeof(unsigned char), cudaMemcpyHostToDevice);

	fun_kernel << < blocks, blocksize >> > (dev_result_data, dev_data, height, width, steps);
	cudaMemcpy(result_data, dev_result_data, newh * neww * sizeof(unsigned char), cudaMemcpyDeviceToHost);
	cudaEventRecord(stop, 0); //Фиксация события stop
	cudaEventSynchronize(stop); //Синхронизация host и device по событию stop


	cudaEventElapsedTime(&elapsedTime, start, stop);
	std::cout << "Time: " << elapsedTime;
	Array2Targa("result.tga", result_data, neww, newh, pbpp);
	cudaFree(dev_data);
	cudaFree(dev_result_data);
	_getch();
}


/*// Helper function for using CUDA to add vectors in parallel.
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
