#ifndef MEMORY_PROVIDER_H__
#define MEMORY_PROVIDER_H__

#include <stddef.h>

typedef struct memory_provider *MemoryProvider;
typedef struct memory *Memory;

MemoryProvider memory_provider_create(char *name);

Memory memory_provider_alloc(MemoryProvider , size_t size);
int memory_provider_copy(MemoryProvider dst_provider,
			 Memory dst, Memory src, size_t size);
int memory_provider_copy_async(MemoryProvider dst_prov, Memory dst,
			       MemoryProvider src_prov, Memory src,
			       size_t size);
int memory_provider_free(MemoryProvider, Memory );

void memory_provider_destroy(MemoryProvider );

int memory_provider_wait(MemoryProvider provider);

const char *memory_provider_get_error(MemoryProvider );

#endif
