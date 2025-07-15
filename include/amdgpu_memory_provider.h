#ifndef AMD_GPU_MEMORY_PROVIDER_H__
#define AMD_GPU_MEMORY_PROVIDER_H__

#include <stddef.h>

typedef void *amdgpu_memory_buffer;

struct amdgpu_memory_provider {
	amdgpu_memory_buffer (*alloc)(size_t size);
	void (*free)(amdgpu_memory_buffer ctx);
	void (*memcpy_from)(
		void *dst, amdgpu_memory_buffer src,
		size_t offset, size_t size
	);
	void (*memcpy_to)(
		amdgpu_memory_buffer dst, void *src,
		size_t offset, size_t size
	);
	void (*memmove_to)(
		amdgpu_memory_buffer src, void *dst,
		size_t offset, size_t size
	);
};

extern struct amdgpu_memory_provider amdgpu_dmabuf_provider;
extern struct amdgpu_memory_provider amdgpu_membuf_provider;

#endif
