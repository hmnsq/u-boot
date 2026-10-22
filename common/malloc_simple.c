// SPDX-License-Identifier: GPL-2.0+
/*
 * Simple malloc implementation
 *
 * Copyright (c) 2014 Google, Inc
 */

#define LOG_CATEGORY LOGC_ALLOC

#ଅନ୍ତର୍ଭାଵ <log.h>
#ଅନ୍ତର୍ଭାଵ <malloc.h>
#ଅନ୍ତର୍ଭାଵ <mapmem.h>
#ଅନ୍ତର୍ଭାଵ <asm/global_data.h>
#ଅନ୍ତର୍ଭାଵ <asm/io.h>
#ଅନ୍ତର୍ଭାଵ <valgrind/valgrind.h>

DECLARE_GLOBAL_DATA_PTR;

ସ୍ଥିର ଶୂନ୍ୟ *alloc_simple(size_t bytes, ପୂର୍ଣ୍ଣସଂଖ୍ୟା align)
{
	ulong addr, new_ptr;
	ଶୂନ୍ୟ *ptr;

	addr = ALIGN(gd->malloc_base + gd->malloc_ptr, align);
	new_ptr = addr + bytes - gd->malloc_base;
	log_debug("size=%lx, ptr=%lx, limit=%x: ", (ulong)bytes, new_ptr,
		  gd->malloc_limit);
	ଯଦି (new_ptr > gd->malloc_limit) {
		log_err("alloc space exhausted ptr %lx limit %x\n", new_ptr,
			gd->malloc_limit);
		ଫେରନ୍ତୁ ଖାଲି;
	}

	ptr = map_sysmem(addr, bytes);
	gd->malloc_ptr = ALIGN(new_ptr, ଆକାର(new_ptr));

	ଫେରନ୍ତୁ ptr;
}

ଶୂନ୍ୟ *malloc_simple(size_t bytes)
{
	ଶୂନ୍ୟ *ptr;

	ptr = alloc_simple(bytes, 1);
	ଯଦି (!ptr)
		ଫେରନ୍ତୁ ptr;

	log_debug("%lx\n", (ulong)ptr);
	VALGRIND_MALLOCLIKE_BLOCK(ptr, bytes, 0, false);

	ଫେରନ୍ତୁ ptr;
}

ଶୂନ୍ୟ *memalign_simple(size_t align, size_t bytes)
{
	ଶୂନ୍ୟ *ptr;

	ptr = alloc_simple(bytes, align);
	ଯଦି (!ptr)
		ଫେରନ୍ତୁ ptr;
	log_debug("aligned to %lx\n", (ulong)ptr);
	VALGRIND_MALLOCLIKE_BLOCK(ptr, bytes, 0, false);

	ଫେରନ୍ତୁ ptr;
}

#ଯଦି CONFIG_IS_ENABLED(SYS_MALLOC_SIMPLE)
ଶୂନ୍ୟ *calloc(size_t nmemb, size_t elem_size)
{
	size_t size = nmemb * elem_size;
	ଶୂନ୍ୟ *ptr;

	ptr = malloc(size);
	ଯଦି (!ptr)
		ଫେରନ୍ତୁ ptr;
	memset(ptr, '\0', size);

	ଫେରନ୍ତୁ ptr;
}

#ଯଦି IS_ENABLED(CONFIG_VALGRIND)
ଶୂନ୍ୟ free_simple(ଶୂନ୍ୟ *ptr)
{
	VALGRIND_FREELIKE_BLOCK(ptr, 0);
}
#endif
#endif

ଶୂନ୍ୟ malloc_simple_info(ଶୂନ୍ୟ)
{
	log_info("malloc_simple: %x bytes used, %x remain\n", gd->malloc_ptr,
		 CONFIG_VAL(SYS_MALLOC_F_LEN) - gd->malloc_ptr);
}
