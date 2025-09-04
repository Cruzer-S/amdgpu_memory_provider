#ifndef MEMORY_PROVIDER_H__
#define MEMORY_PROVIDER_H__

#include <stddef.h>

typedef void *Memory;
typedef void *MemoryProvider;

struct memory_provider {
	Memory (*alloc)(MemoryProvider , size_t size);
	int (*free)(MemoryProvider , Memory ctx);

	int (*memcpy_from)(
		MemoryProvider ,
		void *dst, Memory src,
		size_t offset, size_t size
	);
	int (*memcpy_to)(
		MemoryProvider ,
		Memory dst, void *src,
		size_t offset, size_t size
	);
	int (*memmove_to)(
		MemoryProvider ,
		Memory src, Memory dst,
		size_t src_offset, size_t dst_offset, size_t size
	);
	const char *(*get_error)(MemoryProvider );
	size_t (*get_size)(MemoryProvider , Memory );
};

#endif
