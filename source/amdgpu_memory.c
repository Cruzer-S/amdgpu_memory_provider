#include "memory_provider.h"

#include <stddef.h>
#include <stdlib.h>

#include <hip/hip_runtime.h>

#include <hsa/hsa_ext_amd.h>

typedef struct amdgpu_memory {
	size_t size;
	size_t offset;

	void *context;

	int dmabuf_fd;
} *AMDGPU_Memory;

static hipError_t error;
static hsa_status_t status;

static Memory memory_alloc(size_t size)
{
	AMDGPU_Memory memory;

	memory = (AMDGPU_Memory) malloc(sizeof(struct amdgpu_memory));
	if (memory == NULL)
		goto RETURN_NULL;
	
	memory->size = size;

	error = hipMalloc((void **) &memory->context, memory->size);
	if (error != hipSuccess)
		goto FREE_BUFFER;

	return memory;

FREE_HIP:	(void) hipFree(memory->context);
FREE_BUFFER:	free(memory);
RETURN_NULL:	return NULL;
}

static int memory_free(Memory pmemory)
{
	AMDGPU_Memory memory = (AMDGPU_Memory) pmemory;

	error = hipFree(memory->context);
	free(memory);

	return error == hipSuccess ? 0 : -1;
}

static int memcpy_from(void *dst, Memory psrc, size_t offset, size_t size)
{
	AMDGPU_Memory src = (AMDGPU_Memory) psrc;

	error = hipMemcpy(
		dst, &((char *) src->context)[offset],
		size, hipMemcpyDeviceToHost
	);

	return error == hipSuccess ? 0 : -1;
}

static int memcpy_to(Memory pdst, void *src, size_t offset, size_t size)
{
	AMDGPU_Memory dst = (AMDGPU_Memory) pdst;

	error = hipMemcpy(
		&((char *) dst->context)[offset], src,
		size, hipMemcpyHostToDevice
	);

	return error == hipSuccess ? 0 : -1;
}

static int memmove_to(Memory psrc, Memory pdst,
		      size_t src_offset, size_t dst_offset,
		      size_t size)
{
	AMDGPU_Memory src = (AMDGPU_Memory) psrc;
	AMDGPU_Memory dst = (AMDGPU_Memory) pdst;

	error = hipMemcpy(
		&((char *) src->context)[src_offset],
		&((char *) dst->context)[dst_offset],
	   	size, hipMemcpyDeviceToDevice
	);

	return error == hipSuccess ? 0 : -1;
}

static const char *get_error(void)
{
	return hipGetErrorString(error);
}

static size_t get_size(Memory memory)
{
	return ((AMDGPU_Memory) memory)->size;
}

struct memory_provider amdgpu_memory_provider = {
	.alloc = memory_alloc,
	.free = memory_free,
	.memcpy_from = memcpy_from,
	.memcpy_to = memcpy_to,
	.memmove_to = memmove_to,
	.get_error = get_error,
	.get_size = get_size
};

int amdgpu_memory_export_dmabuf(Memory psrc)
{
	AMDGPU_Memory src = psrc;

	status = hsa_amd_portable_export_dmabuf(
		src->context,
		src->size,
		&src->dmabuf_fd,
		&src->offset
	);

	return status == HSA_STATUS_SUCCESS ? 0 : -1;
}

int amdgpu_memory_get_dmabuf_fd(Memory src)
{
	return ((AMDGPU_Memory) src)->dmabuf_fd;
}

int amdgpu_memory_close_dmabuf(Memory src)
{
	status = hsa_amd_portable_close_dmabuf(
		((AMDGPU_Memory) src)->dmabuf_fd
	);

	return status == HSA_STATUS_SUCCESS ? 0 : -1;
}

const char *amdgpu_memory_get_error(void)
{
	static const char *string;

	(void) hsa_status_string(status, &string);

	return string;
}
