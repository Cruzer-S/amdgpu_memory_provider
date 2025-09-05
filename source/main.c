#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <hsa/hsa.h>
#include <hsa/hsa_ext_amd.h>

#include <unistd.h>

#include "memory_provider.h"

#define PAGE_SIZE	4096

void do_something(MemoryProvider gp, MemoryProvider rp,
		  Memory vmem1, Memory vmem2, Memory rmem)
{
	Memory buffer;

	for (int i = 0; i < PAGE_SIZE; i++)
		((char *) rmem)[i] = ('a' + (i % 26));

	if (memory_provider_allow_access(rp, gp, rmem) == -1)
		exit(EXIT_FAILURE);

	memory_provider_copy(gp, vmem1, rmem, PAGE_SIZE);

	memory_provider_free(rp, (Memory) buffer);
}

int main(void)
{
	struct memory_provider *gp, *rp;
	Memory vmem1, vmem2, rmem;

	rp = memory_provider_create("AMD Ryzen 5 9600X 6-Core Processor");
	if (rp == NULL)
		exit(EXIT_FAILURE);

	gp = memory_provider_create("gfx1101");
	if (gp == NULL)
		exit(EXIT_FAILURE);

	vmem1 = memory_provider_alloc(gp, PAGE_SIZE);
	if (vmem1 == NULL) {
		fprintf(stderr, "failed to provider->alloc(): %s\n",
	  		memory_provider_get_error(gp));
		exit(EXIT_FAILURE);
	}

	vmem2 = memory_provider_alloc(gp, PAGE_SIZE);
	if (vmem2 == NULL) {
		fprintf(stderr, "failed to provider->alloc(): %s\n",
	  		memory_provider_get_error(gp));
		exit(EXIT_FAILURE);
	}

	rmem = memory_provider_alloc(rp, PAGE_SIZE);
	if (rmem == NULL) {
		fprintf(stderr, "failed to provider->alloc(): %s\n",
	  		memory_provider_get_error(rp));
		exit(EXIT_FAILURE);
	}

	do_something(gp, rp, vmem1, vmem2, rmem);

	if (memory_provider_free(gp, vmem1) == -1) {
		fprintf(stderr, "failed to provider->free(): %s\n",
	  		memory_provider_get_error(gp));
		exit(EXIT_FAILURE);
	}

	if (memory_provider_free(gp, vmem2) == -1) {
		fprintf(stderr, "failed to provider->free(): %s\n",
	  		memory_provider_get_error(gp));
		exit(EXIT_FAILURE);
	}

	if (memory_provider_free(rp, rmem) == -1) {
		fprintf(stderr, "failed to provider->free(): %s\n",
	  		memory_provider_get_error(rp));
		exit(EXIT_FAILURE);
	}

	memory_provider_destroy(gp);
	memory_provider_destroy(rp);

	return 0;
}
