#include <stdio.h>
#include <stdlib.h>

#include "amdgpu_memory_provider.h"

#define SIZE_X 6
#define SIZE_Y 6
#define SIZE (SIZE_X * SIZE_Y)

void print_buffer(float *buffer, int x, int y)
{
	for (int i = 0; i < y; i++) {
		for (int j = 0; j < x; j++)
			printf("%3g", buffer[(i * y) + j]);

		putchar('\n');
	}
}

void clear_buffer(float *buffer, int size)
{
	for (int i = 0; i < size; i++)
		buffer[i] = 0.0f;
}

int main(void)
{
	struct amdgpu_memory_provider *provider;
	struct amdgpu_memory_buffer *gpu_buffer;
	float host_buffer[SIZE];

	provider = &amdgpu_dmabuf_provider;
	if (!provider) {
		fprintf(stderr, "failed to get memory provider\n");
		exit(EXIT_FAILURE);
	}

	gpu_buffer = provider->alloc(sizeof(float) * SIZE);
	if (gpu_buffer == NULL) {
		fprintf(stderr, "failed to alloc gpu_buffer\n");
		exit(EXIT_FAILURE);
	}

	clear_buffer(host_buffer, SIZE);
	puts("clear host-buffer");
	print_buffer(host_buffer, SIZE_X, SIZE_Y);

	provider->memcpy_to(gpu_buffer, host_buffer, 0, sizeof(float) * SIZE);
	printf("copy host-buffer[%d:%d] to gpu-buffer[%d:%d]\n\n",
	        0, SIZE, 0, SIZE);

	for (int i = 0; i < SIZE; i++) host_buffer[i] = i + 1.0f;
	puts("initialize host-buffer with sequential numbers");
	print_buffer(host_buffer, SIZE_X, SIZE_Y);


	provider->memcpy_to(gpu_buffer, host_buffer, 0, sizeof(float) * SIZE);
	printf("copy host-buffer[%d:%d] to gpu-buffer[%d:%d]\n\n",
	        0, SIZE, 0, SIZE);

	clear_buffer(host_buffer, SIZE);
	puts("clear host-buffer");
	print_buffer(host_buffer, SIZE_X, SIZE_Y);

	const int COPY_SIZE = (SIZE_Y / 2) * SIZE_X;
	printf("copy device-buffer[%d:%d] to host-buffer[%d:%d]\n\n",
		SIZE_X, SIZE_X + COPY_SIZE,
		(SIZE_X * 2), (SIZE_X * 2) + COPY_SIZE
	);
	provider->memcpy_from(
		&host_buffer[SIZE_X * 2], gpu_buffer,
		SIZE_X * sizeof(float), COPY_SIZE * sizeof(float)
	);
	print_buffer(host_buffer, SIZE_X, SIZE_Y);
	putchar('\n');
	
	provider->free(gpu_buffer);

	return 0;
}
