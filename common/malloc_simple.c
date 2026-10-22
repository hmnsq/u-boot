// SPDX-License-Identifier: GPL-2.0+
/*
 * Simple malloc implementation
 *
 * Copyright (c) 2014 Google, Inc
 */

#define LOG_CATEGORY LOGC_ALLOC

#شامل <log.h>
#شامل <malloc.h>
#شامل <mapmem.h>
#شامل <asm/global_data.h>
#شامل <asm/io.h>
#شامل <valgrind/valgrind.h>

DECLARE_GLOBAL_DATA_PTR;

ایستا تهی *alloc_simple(size_t bytes, صحیح align)
{
	ulong addr, new_ptr;
	تهی *ptr;

	addr = ALIGN(gd->malloc_base + gd->malloc_ptr, align);
	new_ptr = addr + bytes - gd->malloc_base;
	log_debug("size=%lx, ptr=%lx, limit=%x: ", (ulong)bytes, new_ptr,
		  gd->malloc_limit);
	اگر (new_ptr > gd->malloc_limit) {
		log_err("alloc space exhausted ptr %lx limit %x\n", new_ptr,
			gd->malloc_limit);
		بازگشت پوچ;
	}

	ptr = map_sysmem(addr, bytes);
	gd->malloc_ptr = ALIGN(new_ptr, اندازه(new_ptr));

	بازگشت ptr;
}

تهی *malloc_simple(size_t bytes)
{
	تهی *ptr;

	ptr = alloc_simple(bytes, 1);
	اگر (!ptr)
		بازگشت ptr;

	log_debug("%lx\n", (ulong)ptr);
	VALGRIND_MALLOCLIKE_BLOCK(ptr, bytes, 0, false);

	بازگشت ptr;
}

تهی *memalign_simple(size_t align, size_t bytes)
{
	تهی *ptr;

	ptr = alloc_simple(bytes, align);
	اگر (!ptr)
		بازگشت ptr;
	log_debug("aligned to %lx\n", (ulong)ptr);
	VALGRIND_MALLOCLIKE_BLOCK(ptr, bytes, 0, false);

	بازگشت ptr;
}

#اگر CONFIG_IS_ENABLED(SYS_MALLOC_SIMPLE)
تهی *calloc(size_t nmemb, size_t elem_size)
{
	size_t size = nmemb * elem_size;
	تهی *ptr;

	ptr = malloc(size);
	اگر (!ptr)
		بازگشت ptr;
	memset(ptr, '\0', size);

	بازگشت ptr;
}

#اگر IS_ENABLED(CONFIG_VALGRIND)
تهی free_simple(تهی *ptr)
{
	VALGRIND_FREELIKE_BLOCK(ptr, 0);
}
#endif
#endif

تهی malloc_simple_info(تهی)
{
	log_info("malloc_simple: %x bytes used, %x remain\n", gd->malloc_ptr,
		 CONFIG_VAL(SYS_MALLOC_F_LEN) - gd->malloc_ptr);
}
