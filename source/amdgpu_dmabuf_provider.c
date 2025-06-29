#include "amdgpu_memory_provider.h"

#include <stdlib.h>
#include <stddef.h>

#include <unistd.h>

#include <hip/hip_runtime.h>

#include <hsa/amd_hsa_common.h>
#include <hsa/hsa_ext_amd.h>

struct amdgpu_memory_buffer {
	size_t size;
	size_t offset;

	void *memory;
	int fd;
};

struct amdgpu_memory_buffer *amdgpu_dmabuf_alloc(size_t size)
{
	struct amdgpu_memory_buffer *buffer;
	hipError_t error;
	hsa_status_t status;

	buffer = (struct amdgpu_memory_buffer *)
		 malloc(sizeof(struct amdgpu_memory_buffer));
	if (buffer == NULL)
		goto RETURN_NULL;
	
	buffer->size = size;

	error = hipMalloc((void **) &buffer->memory, buffer->size);
	if (error != hipSuccess)
		goto FREE_BUFFER;

	status = hsa_amd_portable_export_dmabuf(
		(void *) buffer->memory, buffer->size,
		&buffer->fd, &buffer->offset
	);

	if (status != HSA_STATUS_SUCCESS)
		goto FREE_HIP;

	return buffer;

FREE_HIP:	(void) hipFree(buffer->memory);
FREE_BUFFER:	free(buffer);
RETURN_NULL:	return NULL;
}

void amdgpu_dmabuf_free(struct amdgpu_memory_buffer *buffer)
{
	hsa_amd_portable_close_dmabuf(buffer->fd);
	(void) hipFree(buffer->memory);
	free(buffer);
}

void amdgpu_dmabuf_memcpy_from(
		void *dst, struct amdgpu_memory_buffer *src,
		size_t offset, size_t size
) {
	hipError_t error;

	error = hipMemcpy(
		dst, &((char *) src->memory)[offset],
		size, hipMemcpyDeviceToHost
	);
}

void amdgpu_dmabuf_memcpy_to(
		struct amdgpu_memory_buffer *dst, void *src,
		size_t offset, size_t size
) {
	hipError_t error;

	error = hipMemcpy(
		&((char *) dst->memory)[offset], src,
		size, hipMemcpyHostToDevice
	);
}

struct amdgpu_memory_provider amdgpu_dmabuf_provider = {
	.alloc = amdgpu_dmabuf_alloc,
	.free = amdgpu_dmabuf_free,
	.memcpy_from = amdgpu_dmabuf_memcpy_from,
	.memcpy_to = amdgpu_dmabuf_memcpy_to
};
