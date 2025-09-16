#include "memory_provider.h"

#include <stddef.h>
#include <string.h>
#include <stdlib.h>

#include <hsa/hsa.h>
#include <hsa/hsa_ext_amd.h>

#define RDU(G, S) ( ( (((S) - 1) / (G)) + 1 ) * (G) )

struct memory_pool {
	hsa_amd_memory_pool_t pool;
	size_t granule;
};

struct memory_provider {
	hsa_agent_t device;
	struct memory_pool memory_pool;

	hsa_signal_t signal;
	hsa_queue_t *queue;

	char dev_name[64];

	hsa_status_t status;
	const char *message;
};

static int is_hsa_initialized = 0;

static hsa_status_t find_device(hsa_agent_t device, void *data)
{
	MemoryProvider provider;
	hsa_device_type_t device_type;
	char name[64];

	provider = (MemoryProvider) data;

	provider->status = hsa_agent_get_info(
		device, HSA_AGENT_INFO_DEVICE, &device_type
	);
	if (provider->status != HSA_STATUS_SUCCESS)
		return provider->status;

	provider->status = hsa_agent_get_info(
		device, HSA_AGENT_INFO_NAME, name
	);
	if (provider->status != HSA_STATUS_SUCCESS)
		return provider->status;

	if (strcmp(provider->dev_name, name))
		return HSA_STATUS_SUCCESS;

	provider->device = device;

	return HSA_STATUS_INFO_BREAK;
}

static hsa_status_t find_pool(hsa_amd_memory_pool_t pool, void *data)
{
	MemoryProvider provider;
	hsa_amd_segment_t segment;
	hsa_status_t status;
	bool can_allocate;

	provider = (MemoryProvider) data;

	status = hsa_amd_memory_pool_get_info(
		pool, HSA_AMD_MEMORY_POOL_INFO_SEGMENT, &segment
	);
	if (status != HSA_STATUS_SUCCESS)
		return status;

	if (segment != HSA_AMD_SEGMENT_GLOBAL)
		return HSA_STATUS_SUCCESS;

	status = hsa_amd_memory_pool_get_info(
		pool,
		HSA_AMD_MEMORY_POOL_INFO_RUNTIME_ALLOC_ALLOWED,
		&can_allocate
	);
	if (status != HSA_STATUS_SUCCESS)
		return status;

	if ( !can_allocate )
		return HSA_STATUS_SUCCESS;

	status = hsa_amd_memory_pool_get_info(
		pool,
		HSA_AMD_MEMORY_POOL_INFO_RUNTIME_ALLOC_GRANULE,
		&provider->memory_pool.granule
	);
	if (status != HSA_STATUS_SUCCESS)
		return status;

	provider->memory_pool.pool = pool;

	return HSA_STATUS_INFO_BREAK;
}

MemoryProvider memory_provider_create(char *dev_name)
{
	MemoryProvider provider;
	hsa_status_t status;

	provider = malloc(sizeof(struct memory_provider));
	if (provider == NULL)
		goto RETURN_NULL;

	if (is_hsa_initialized <= 0) {
		status = hsa_init();
		if (status != HSA_STATUS_SUCCESS)
			goto FREE_PROVIDER;
	}

	strncpy(provider->dev_name, dev_name, 64);

	status = hsa_iterate_agents(find_device, provider);
	if (status != HSA_STATUS_INFO_BREAK)
		goto HSA_SHUT_DOWN;

	status = hsa_amd_agent_iterate_memory_pools(
		provider->device, find_pool, provider
	);
	if (status != HSA_STATUS_INFO_BREAK)
		goto HSA_SHUT_DOWN;

	status = hsa_signal_create(0, 0, NULL, &provider->signal);
	if (status != HSA_STATUS_SUCCESS)
		goto HSA_SHUT_DOWN;

	is_hsa_initialized++;

	return provider;

HSA_SHUT_DOWN:	(void) hsa_shut_down();
FREE_PROVIDER:	free(provider);
RETURN_NULL:	return NULL;
}

void memory_provider_destroy(MemoryProvider provider)
{
	if (--is_hsa_initialized <= 0)
		(void) hsa_shut_down();
	free(provider);
}

Memory memory_provider_alloc(MemoryProvider provider, size_t size)
{
	Memory memory;

	size_t rdu_size;

	rdu_size = RDU(provider->memory_pool.granule, size);

	provider->status = hsa_amd_memory_pool_allocate(
		provider->memory_pool.pool, rdu_size,
		0, (void **) &memory
	);
	if (provider->status != HSA_STATUS_SUCCESS)
		return NULL;

	return memory;
}

int memory_provider_copy(MemoryProvider dst_provider,
			 Memory dst, Memory src, size_t size)
{
	dst_provider->status = hsa_memory_copy(dst, src, size);

	if (dst_provider->status != HSA_STATUS_SUCCESS)
		return -1;

	return 0;
}

int memory_provider_copy_async(MemoryProvider dst_prov, Memory dst,
			       MemoryProvider src_prov, Memory src,
			       size_t size)
{
	hsa_signal_add_relaxed(dst_prov->signal, 1);

	dst_prov->status = hsa_amd_memory_async_copy(
		dst, dst_prov->device,
		src, src_prov->device,
		size,
		0, NULL, dst_prov->signal
	);
	if (dst_prov->status != HSA_STATUS_SUCCESS)
		return -1;

	return 0;
}

int memory_provider_wait(MemoryProvider provider)
{
	provider->status = hsa_signal_wait_scacquire(
		provider->signal, HSA_SIGNAL_CONDITION_EQ,
		0, UINT64_MAX,
		HSA_WAIT_STATE_BLOCKED
	);

	if (provider->status != HSA_STATUS_SUCCESS)
		return -1;

	return 0;
}

int memory_provider_free(MemoryProvider provider, Memory memory)
{
	provider->status = hsa_amd_memory_pool_free(memory);

	if (provider->status != HSA_STATUS_SUCCESS)
		return -1;

	return 0;
}

int memory_provider_allow_access(MemoryProvider dst_prov,
				 MemoryProvider src_prov,
				 Memory dst_memory)
{
	dst_prov->status = hsa_amd_agents_allow_access(
		2,
		(hsa_agent_t [2]) { dst_prov->device, src_prov->device },
		NULL,
		dst_memory
	);

	if (dst_prov->status != HSA_STATUS_SUCCESS)
		return -1;

	return 0;
}

int memory_provider_queue_create(MemoryProvider provider, void (*callback)(hsa_status_t status, hsa_queue_t *source, void *data))
{
	provider->status = hsa_queue_create(
		provider->device,
		1024, HSA_QUEUE_TYPE_SINGLE,
		callback, provider,
		UINT32_MAX, UINT32_MAX,
		&provider->queue
	);

	if (provider->status != HSA_STATUS_SUCCESS)
		return -1;

	return 0;
}

void memory_provider_queue_destroy(MemoryProvider provider)
{
	(void) hsa_queue_destroy(provider->queue);
}

const char *memory_provider_get_error(MemoryProvider provider)
{
	hsa_status_string(provider->status, &provider->message);

	return provider->message;
}
