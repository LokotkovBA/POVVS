
#include "cuda_runtime.h"
#include "device_launch_parameters.h"

#include "targ.h"
#include <stdio.h>
#include <cstdint>


__global__ void bilinear_interpolation_kernel(uint8_t *output, uint8_t *input, uint8_t pitchOutput, uint8_t pitchInput, uint8_t bytesPerPixelInput, uint8_t bytesPerPixelOutput, float xRatio, float yRatio) {
	int x = (int)(xRatio * blockIdx.x);
	int y = (int)(yRatio * blockIdx.y);

	uint8_t *a; uint8_t *b; uint8_t *c; uint8_t *d;
	float xDist, yDist, blue, red, green;

	// X and Y distance difference
	xDist = (xRatio * blockIdx.x) - x;
	yDist = (yRatio * blockIdx.y) - y;

	// Points
	a = input + y * pitchInput + x * bytesPerPixelInput;
	b = input + y * pitchInput + (x + 1) * bytesPerPixelInput;
	c = input + (y + 1) * pitchInput + x * bytesPerPixelInput;
	d = input + (y + 1) * pitchInput + (x + 1) * bytesPerPixelInput;

	// Calc
	blue = (a[2])*(1 - xDist)*(1 - yDist) + (b[2])*(xDist)*(1 - yDist) + (c[2])*(yDist)*(1 - xDist) + (d[2])*(xDist * yDist);

	uint8_t *p = output + blockIdx.y * pitchOutput + blockIdx.x * bytesPerPixelOutput;
	*(uint32_t*)p = 0xff000000 | ((((int)red) << 16)) | ((((int)green) << 8)) | ((int)blue);
}


int main()
{
	unsigned char* data;


	unsigned width, height, pbpp;
	int blocks = 8;
	if (!Targa2Array("C:/Users/B.Lokotkov/Desktop/_git/POVVS/Lab5/sample2.tga", &data, &width, &height, &pbpp))
	{
		std::cout << "Can't read file";
		return -1;
	}
	int N, nblocks, nthreads;
	std::cout << "N = ";
	std::cin >> N;

    return 0;
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
