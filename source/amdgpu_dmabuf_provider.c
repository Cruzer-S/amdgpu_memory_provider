#include "amdgpu_memory_provider.h"
#include "amdgpu_dmabuf_provider.h"

#include <stdlib.h>
#include <stddef.h>

#include <unistd.h>

#include <hip/hip_runtime.h>

#include <hsa/amd_hsa_common.h>
#include <hsa/hsa_ext_amd.h>

amdgpu_memory_buffer amdgpu_dmabuf_alloc(size_t size)
{
	struct amdgpu_dmabuf_buffer *buffer;
	hipError_t error;
	hsa_status_t status;

	buffer = (struct amdgpu_dmabuf_buffer *)
		 malloc(sizeof(struct amdgpu_dmabuf_buffer));
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

void amdgpu_dmabuf_free(amdgpu_memory_buffer pbuffer)
{
	struct amdgpu_dmabuf_buffer *buffer = pbuffer;

	hsa_amd_portable_close_dmabuf(buffer->fd);
	(void) hipFree(buffer->memory);
	free(buffer);
}

void amdgpu_dmabuf_memcpy_from(
		void *dst, amdgpu_memory_buffer psrc,
		size_t offset, size_t size
) {
	struct amdgpu_dmabuf_buffer *src = psrc;
	hipError_t error;

	error = hipMemcpy(
		dst, &((char *) src->memory)[offset],
		size, hipMemcpyDeviceToHost
	);
}

void amdgpu_dmabuf_memcpy_to(
		amdgpu_memory_buffer pdst, void *src,
		size_t offset, size_t size
) {
	struct amdgpu_dmabuf_buffer *dst = pdst;
	hipError_t error;

	error = hipMemcpy(
		&((char *) dst->memory)[offset], src,
		size, hipMemcpyHostToDevice
	);
}

void amdgpu_dmabuf_memmove_to(
		amdgpu_memory_buffer psrc, void *dst,
		size_t offset, size_t size
) {
	struct amdgpu_dmabuf_buffer *src = psrc;

	hipMemcpy(&((char *) src->memory)[offset], dst,
	   	  size, hipMemcpyDeviceToDevice);
}

struct amdgpu_memory_provider amdgpu_dmabuf_provider = {
	.alloc = amdgpu_dmabuf_alloc,
	.free = amdgpu_dmabuf_free,
	.memcpy_from = amdgpu_dmabuf_memcpy_from,
	.memcpy_to = amdgpu_dmabuf_memcpy_to,
	.memmove_to = amdgpu_dmabuf_memmove_to,
};
