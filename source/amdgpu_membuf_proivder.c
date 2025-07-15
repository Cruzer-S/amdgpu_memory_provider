#include "amdgpu_memory_provider.h"
#include "amdgpu_membuf_provider.h"

#include <stddef.h>
#include <stdlib.h>

#include <hip/hip_runtime.h>

amdgpu_memory_buffer amdgpu_membuf_alloc(size_t size)
{
	struct amdgpu_membuf_buffer *buffer;
	hipError_t error;

	buffer = (struct amdgpu_membuf_buffer *)
		 malloc(sizeof(struct amdgpu_membuf_buffer));
	if (buffer == NULL)
		goto RETURN_NULL;
	
	buffer->size = size;

	error = hipMalloc((void **) &buffer->memory, buffer->size);
	if (error != hipSuccess)
		goto FREE_BUFFER;

	return buffer;

FREE_HIP:	(void) hipFree(buffer->memory);
FREE_BUFFER:	free(buffer);
RETURN_NULL:	return NULL;
}

void amdgpu_membuf_free(amdgpu_memory_buffer pbuffer)
{
	struct amdgpu_membuf_buffer *buffer = pbuffer;

	(void) hipFree(buffer->memory);
	free(buffer);
}

void amdgpu_membuf_memcpy_from(
		void *dst, amdgpu_memory_buffer psrc,
		size_t offset, size_t size
) {
	struct amdgpu_membuf_buffer *src = psrc;
	hipError_t error;

	error = hipMemcpy(
		dst, &((char *) src->memory)[offset],
		size, hipMemcpyDeviceToHost
	);
}

void amdgpu_membuf_memcpy_to(
		amdgpu_memory_buffer pdst, void *src,
		size_t offset, size_t size
) {
	struct amdgpu_membuf_buffer *dst = pdst;
	hipError_t error;

	error = hipMemcpy(
		&((char *) dst->memory)[offset], src,
		size, hipMemcpyHostToDevice
	);
}

void amdgpu_membuf_memmove_to(
		amdgpu_memory_buffer psrc, void *dst,
		size_t offset, size_t size
) {

	struct amdgpu_membuf_buffer *src = psrc;
	hipMemcpy(&((char *) src->memory)[offset], dst,
	   	  size, hipMemcpyDeviceToDevice);
}


struct amdgpu_memory_provider amdgpu_membuf_provider = {
	.alloc = amdgpu_membuf_alloc,
	.free = amdgpu_membuf_free,
	.memcpy_from = amdgpu_membuf_memcpy_from,
	.memcpy_to = amdgpu_membuf_memcpy_to,
	.memmove_to = amdgpu_membuf_memmove_to,
};
