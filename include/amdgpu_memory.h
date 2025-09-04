#ifndef AMDGPU_MEMORY_H__
#define AMDGPU_MEMORY_H__

#include "memory_provider.h"

MemoryProvider amdgpu_provider_create(void);
void amdgpu_provider_destroy(MemoryProvider );

int amdgpu_memory_export_dmabuf(MemoryProvider , Memory );

int amdgpu_memory_get_dmabuf_fd(Memory );

int amdgpu_memory_close_dmabuf(Memory );

const char *amdgpu_memory_get_error(MemoryProvider );

#endif
