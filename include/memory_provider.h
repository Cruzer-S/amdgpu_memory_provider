#ifndef MEMORY_PROVIDER_H__
#define MEMORY_PROVIDER_H__

#include <stddef.h>

typedef void *Memory;

struct memory_provider {
	Memory (*alloc)(size_t size);
	int (*free)(Memory ctx);

	int (*memcpy_from)(
		void *dst, Memory src,
		size_t offset, size_t size
	);
	int (*memcpy_to)(
		Memory dst, void *src,
		size_t offset, size_t size
	);
	int (*memmove_to)(
		Memory src, void *dst,
		size_t offset, size_t size
	);
	const char *(*get_error)(void);
};

extern struct memory_provider amdgpu_memory_provider;

#endif
