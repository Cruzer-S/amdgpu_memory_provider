#ifndef AMD_GPU_MEMBUF_PROVIDER_H__
#define AMD_GPU_MEMBUF_PROVIDER_H__

#include <stddef.h>

struct amdgpu_membuf_buffer {
	size_t size;
	size_t offset;

	void *memory;
};

#endif
