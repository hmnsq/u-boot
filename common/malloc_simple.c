// SPDX-License-Identifier: GPL-2.0+
/*
 * Simple malloc implementation
 *
 * Copyright (c) 2014 Google, Inc
 */

#define LOG_CATEGORY LOGC_ALLOC

#அந்தர்பாவ <log.h>
#அந்தர்பாவ <malloc.h>
#அந்தர்பாவ <mapmem.h>
#அந்தர்பாவ <asm/global_data.h>
#அந்தர்பாவ <asm/io.h>
#அந்தர்பாவ <valgrind/valgrind.h>

DECLARE_GLOBAL_DATA_PTR;

நிலையான வெறுமை *alloc_simple(size_t bytes, முழு align)
{
	ulong addr, new_ptr;
	வெறுமை *ptr;

	addr = ALIGN(gd->malloc_base + gd->malloc_ptr, align);
	new_ptr = addr + bytes - gd->malloc_base;
	log_debug("size=%lx, ptr=%lx, limit=%x: ", (ulong)bytes, new_ptr,
		  gd->malloc_limit);
	எனில் (new_ptr > gd->malloc_limit) {
		log_err("alloc space exhausted ptr %lx limit %x\n", new_ptr,
			gd->malloc_limit);
		திரும்பு வெற்று;
	}

	ptr = map_sysmem(addr, bytes);
	gd->malloc_ptr = ALIGN(new_ptr, அளவு(new_ptr));

	திரும்பு ptr;
}

வெறுமை *malloc_simple(size_t bytes)
{
	வெறுமை *ptr;

	ptr = alloc_simple(bytes, 1);
	எனில் (!ptr)
		திரும்பு ptr;

	log_debug("%lx\n", (ulong)ptr);
	VALGRIND_MALLOCLIKE_BLOCK(ptr, bytes, 0, false);

	திரும்பு ptr;
}

வெறுமை *memalign_simple(size_t align, size_t bytes)
{
	வெறுமை *ptr;

	ptr = alloc_simple(bytes, align);
	எனில் (!ptr)
		திரும்பு ptr;
	log_debug("aligned to %lx\n", (ulong)ptr);
	VALGRIND_MALLOCLIKE_BLOCK(ptr, bytes, 0, false);

	திரும்பு ptr;
}

#எனில் CONFIG_IS_ENABLED(SYS_MALLOC_SIMPLE)
வெறுமை *calloc(size_t nmemb, size_t elem_size)
{
	size_t size = nmemb * elem_size;
	வெறுமை *ptr;

	ptr = malloc(size);
	எனில் (!ptr)
		திரும்பு ptr;
	memset(ptr, '\0', size);

	திரும்பு ptr;
}

#எனில் IS_ENABLED(CONFIG_VALGRIND)
வெறுமை free_simple(வெறுமை *ptr)
{
	VALGRIND_FREELIKE_BLOCK(ptr, 0);
}
#endif
#endif

வெறுமை malloc_simple_info(வெறுமை)
{
	log_info("malloc_simple: %x bytes used, %x remain\n", gd->malloc_ptr,
		 CONFIG_VAL(SYS_MALLOC_F_LEN) - gd->malloc_ptr);
}
