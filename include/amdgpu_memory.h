#ifndef AMDGPU_MEMORY_H__
#define AMDGPU_MEMORY_H__

#include "memory_provider.h"

int amdgpu_memory_export_dmabuf(Memory );
int amdgpu_memory_close_dmabuf(int );

const char *amdgpu_memory_get_error(void);

#endif
