#ifndef AMD_GPU_DMABUF_PROVIDER_H__
#define AMD_GPU_DMABUF_PROVIDER_H__

#include <stddef.h>

struct amdgpu_memory_buffer {
	size_t size;
	size_t offset;

	void *memory;
	int fd;
};

#endif
