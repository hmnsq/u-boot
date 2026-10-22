// SPDX-License-Identifier: GPL-2.0+
/*
 * Simple malloc implementation
 *
 * Copyright (c) 2014 Google, Inc
 */

#define LOG_CATEGORY LOGC_ALLOC

#ضمن <log.h>
#ضمن <malloc.h>
#ضمن <mapmem.h>
#ضمن <asm/global_data.h>
#ضمن <asm/io.h>
#ضمن <valgrind/valgrind.h>

DECLARE_GLOBAL_DATA_PTR;

ساكن فراغ *alloc_simple(size_t bytes, صحيح align)
{
	ulong addr, new_ptr;
	فراغ *ptr;

	addr = ALIGN(gd->malloc_base + gd->malloc_ptr, align);
	new_ptr = addr + bytes - gd->malloc_base;
	log_debug("size=%lx, ptr=%lx, limit=%x: ", (ulong)bytes, new_ptr,
		  gd->malloc_limit);
	إذا (new_ptr > gd->malloc_limit) {
		log_err("alloc space exhausted ptr %lx limit %x\n", new_ptr,
			gd->malloc_limit);
		إرجاع عدم;
	}

	ptr = map_sysmem(addr, bytes);
	gd->malloc_ptr = ALIGN(new_ptr, حجم(new_ptr));

	إرجاع ptr;
}

فراغ *malloc_simple(size_t bytes)
{
	فراغ *ptr;

	ptr = alloc_simple(bytes, 1);
	إذا (!ptr)
		إرجاع ptr;

	log_debug("%lx\n", (ulong)ptr);
	VALGRIND_MALLOCLIKE_BLOCK(ptr, bytes, 0, false);

	إرجاع ptr;
}

فراغ *memalign_simple(size_t align, size_t bytes)
{
	فراغ *ptr;

	ptr = alloc_simple(bytes, align);
	إذا (!ptr)
		إرجاع ptr;
	log_debug("aligned to %lx\n", (ulong)ptr);
	VALGRIND_MALLOCLIKE_BLOCK(ptr, bytes, 0, false);

	إرجاع ptr;
}

#إذا CONFIG_IS_ENABLED(SYS_MALLOC_SIMPLE)
فراغ *calloc(size_t nmemb, size_t elem_size)
{
	size_t size = nmemb * elem_size;
	فراغ *ptr;

	ptr = malloc(size);
	إذا (!ptr)
		إرجاع ptr;
	memset(ptr, '\0', size);

	إرجاع ptr;
}

#إذا IS_ENABLED(CONFIG_VALGRIND)
فراغ free_simple(فراغ *ptr)
{
	VALGRIND_FREELIKE_BLOCK(ptr, 0);
}
#endif
#endif

فراغ malloc_simple_info(فراغ)
{
	log_info("malloc_simple: %x bytes used, %x remain\n", gd->malloc_ptr,
		 CONFIG_VAL(SYS_MALLOC_F_LEN) - gd->malloc_ptr);
}
