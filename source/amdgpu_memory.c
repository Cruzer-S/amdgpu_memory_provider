#include "memory_provider.h"

#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include <hsa/hsa.h>
#include <hsa/hsa_ext_amd.h>

struct memory_agent {
	hsa_agent_t device;
	hsa_amd_memory_pool_t memory_pool;
};

struct memory_provider function_table;

typedef struct amdgpu_memory_provider {
	struct memory_provider func_table;
	struct memory_agent agent;

	char dev_name[64];

	hsa_status_t status;
} *AMDGPU_MemoryProvider;

typedef struct amdgpu_memory {
	size_t size;
	size_t offset;

	void *context;

	int dmabuf_fd;
} *AMDGPU_Memory;

static hsa_status_t find_GPUs(hsa_agent_t device, void *data)
{
	AMDGPU_MemoryProvider provider;
	hsa_device_type_t device_type;
	char name[64];

	provider = data;

	provider->status = hsa_agent_get_info(
		device, HSA_AGENT_INFO_DEVICE, &device_type
	);
	if (provider->status != HSA_STATUS_SUCCESS)
		return provider->status;

	if (device_type != HSA_DEVICE_TYPE_GPU)
		return HSA_STATUS_SUCCESS;

	provider->status = hsa_agent_get_info(
		device, HSA_AGENT_INFO_NAME, name
	);
	if (provider->status != HSA_STATUS_SUCCESS)
		return provider->status;

	if (strcmp(provider->dev_name, name))
		return HSA_STATUS_SUCCESS;

	provider->agent.device = device;

	return HSA_STATUS_SUCCESS;
}

MemoryProvider amdgpu_provider_create(char *dev_name)
{
	AMDGPU_MemoryProvider provider;
	hsa_status_t status;

	provider = (AMDGPU_MemoryProvider)
		   malloc(sizeof(struct amdgpu_memory_provider));
	if (provider == NULL)
		goto RETURN_NULL;

	status = hsa_init();
	if (status != HSA_STATUS_SUCCESS)
		goto FREE_PROVIDER;

	strncpy(provider->dev_name, dev_name, 64);

	status = hsa_iterate_agents(find_GPUs, provider);
	if (status != HSA_STATUS_SUCCESS)
		goto FREE_PROVIDER;

	provider->func_table = function_table;

	return provider;
FREE_PROVIDER:	free(provider);
RETURN_NULL:	return NULL;
}

void amdgpu_provider_destroy(MemoryProvider provider)
{
	(void) hsa_shut_down();
	free(provider);
}

int amdgpu_memory_get_dmabuf_fd(Memory memory)
{
	return -1;
}

int amdgpu_memory_close_dmabuf(Memory memory)
{
	return -1;
}

const char *amdgpu_memory_get_error(MemoryProvider provider)
{
	return NULL;
}

