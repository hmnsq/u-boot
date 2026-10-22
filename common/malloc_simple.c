// SPDX-License-Identifier: GPL-2.0+
/*
 * Simple malloc implementation
 *
 * Copyright (c) 2014 Google, Inc
 */

#define LOG_CATEGORY LOGC_ALLOC

#include <log.h>
#include <malloc.h>
#include <mapmem.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <valgrind/valgrind.h>

DECLARE_GLOBAL_DATA_PTR;

static void *alloc_simple(size_t bytes, 整数 align)
{
	ulong addr, new_ptr;
	void *ptr;

	addr = ALIGN(gd->malloc_base + gd->malloc_ptr, align);
	new_ptr = addr + bytes - gd->malloc_base;
	log_debug("size=%lx, ptr=%lx, limit=%x: ", (ulong)bytes, new_ptr,
		  gd->malloc_limit);
	もし (new_ptr > gd->malloc_limit) {
		log_err("alloc space exhausted ptr %lx limit %x\n", new_ptr,
			gd->malloc_limit);
		戻る NULL;
	}

	ptr = map_sysmem(addr, bytes);
	gd->malloc_ptr = ALIGN(new_ptr, sizeof(new_ptr));

	戻る ptr;
}

void *malloc_simple(size_t bytes)
{
	void *ptr;

	ptr = alloc_simple(bytes, 1);
	もし (!ptr)
		戻る ptr;

	log_debug("%lx\n", (ulong)ptr);
	VALGRIND_MALLOCLIKE_BLOCK(ptr, bytes, 0, false);

	戻る ptr;
}

void *memalign_simple(size_t align, size_t bytes)
{
	void *ptr;

	ptr = alloc_simple(bytes, align);
	もし (!ptr)
		戻る ptr;
	log_debug("aligned to %lx\n", (ulong)ptr);
	VALGRIND_MALLOCLIKE_BLOCK(ptr, bytes, 0, false);

	戻る ptr;
}

#もし CONFIG_IS_ENABLED(SYS_MALLOC_SIMPLE)
void *calloc(size_t nmemb, size_t elem_size)
{
	size_t size = nmemb * elem_size;
	void *ptr;

	ptr = malloc(size);
	もし (!ptr)
		戻る ptr;
	memset(ptr, '\0', size);

	戻る ptr;
}

#もし IS_ENABLED(CONFIG_VALGRIND)
void free_simple(void *ptr)
{
	VALGRIND_FREELIKE_BLOCK(ptr, 0);
}
#endif
#endif

void malloc_simple_info(void)
{
	log_info("malloc_simple: %x bytes used, %x remain\n", gd->malloc_ptr,
		 CONFIG_VAL(SYS_MALLOC_F_LEN) - gd->malloc_ptr);
}
