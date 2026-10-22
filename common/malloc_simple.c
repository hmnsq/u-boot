// SPDX-License-Identifier: GPL-2.0+
/*
 * Simple malloc implementation
 *
 * Copyright (c) 2014 Google, Inc
 */

#define LOG_CATEGORY LOGC_ALLOC

#અન્તર્ભાવ <log.h>
#અન્તર્ભાવ <malloc.h>
#અન્તર્ભાવ <mapmem.h>
#અન્તર્ભાવ <asm/global_data.h>
#અન્તર્ભાવ <asm/io.h>
#અન્તર્ભાવ <valgrind/valgrind.h>

DECLARE_GLOBAL_DATA_PTR;

સ્થિર શૂન્ય *alloc_simple(size_t bytes, પૂર્ણાંક align)
{
	ulong addr, new_ptr;
	શૂન્ય *ptr;

	addr = ALIGN(gd->malloc_base + gd->malloc_ptr, align);
	new_ptr = addr + bytes - gd->malloc_base;
	log_debug("size=%lx, ptr=%lx, limit=%x: ", (ulong)bytes, new_ptr,
		  gd->malloc_limit);
	જો (new_ptr > gd->malloc_limit) {
		log_err("alloc space exhausted ptr %lx limit %x\n", new_ptr,
			gd->malloc_limit);
		પાછા ખાલી;
	}

	ptr = map_sysmem(addr, bytes);
	gd->malloc_ptr = ALIGN(new_ptr, કદ(new_ptr));

	પાછા ptr;
}

શૂન્ય *malloc_simple(size_t bytes)
{
	શૂન્ય *ptr;

	ptr = alloc_simple(bytes, 1);
	જો (!ptr)
		પાછા ptr;

	log_debug("%lx\n", (ulong)ptr);
	VALGRIND_MALLOCLIKE_BLOCK(ptr, bytes, 0, false);

	પાછા ptr;
}

શૂન્ય *memalign_simple(size_t align, size_t bytes)
{
	શૂન્ય *ptr;

	ptr = alloc_simple(bytes, align);
	જો (!ptr)
		પાછા ptr;
	log_debug("aligned to %lx\n", (ulong)ptr);
	VALGRIND_MALLOCLIKE_BLOCK(ptr, bytes, 0, false);

	પાછા ptr;
}

#જો CONFIG_IS_ENABLED(SYS_MALLOC_SIMPLE)
શૂન્ય *calloc(size_t nmemb, size_t elem_size)
{
	size_t size = nmemb * elem_size;
	શૂન્ય *ptr;

	ptr = malloc(size);
	જો (!ptr)
		પાછા ptr;
	memset(ptr, '\0', size);

	પાછા ptr;
}

#જો IS_ENABLED(CONFIG_VALGRIND)
શૂન્ય free_simple(શૂન્ય *ptr)
{
	VALGRIND_FREELIKE_BLOCK(ptr, 0);
}
#endif
#endif

શૂન્ય malloc_simple_info(શૂન્ય)
{
	log_info("malloc_simple: %x bytes used, %x remain\n", gd->malloc_ptr,
		 CONFIG_VAL(SYS_MALLOC_F_LEN) - gd->malloc_ptr);
}
