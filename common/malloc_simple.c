// SPDX-License-Identifier: GPL-2.0+
/*
 * Simple malloc implementation
 *
 * Copyright (c) 2014 Google, Inc
 */

#define LOG_CATEGORY LOGC_ALLOC

#অন্তর্ভাব <log.h>
#অন্তর্ভাব <malloc.h>
#অন্তর্ভাব <mapmem.h>
#অন্তর্ভাব <asm/global_data.h>
#অন্তর্ভাব <asm/io.h>
#অন্তর্ভাব <valgrind/valgrind.h>

DECLARE_GLOBAL_তথ্য_PTR;

স্থির শূন্য *alloc_simple(size_t bytes, পূর্ণসংখ্যা align)
{
	ulong addr, new_ptr;
	শূন্য *ptr;

	addr = ALIGN(gd->malloc_base + gd->malloc_ptr, align);
	new_ptr = addr + bytes - gd->malloc_base;
	log_debug("size=%lx, ptr=%lx, limit=%x: ", (ulong)bytes, new_ptr,
		  gd->malloc_limit);
	যদি (new_ptr > gd->malloc_limit) {
		log_err("alloc space exhausted ptr %lx limit %x\n", new_ptr,
			gd->malloc_limit);
		ফেরত ফাঁকা;
	}

	ptr = map_sysmem(addr, bytes);
	gd->malloc_ptr = ALIGN(new_ptr, আকার(new_ptr));

	ফেরত ptr;
}

শূন্য *malloc_simple(size_t bytes)
{
	শূন্য *ptr;

	ptr = alloc_simple(bytes, 1);
	যদি (!ptr)
		ফেরত ptr;

	log_debug("%lx\n", (ulong)ptr);
	VALGRIND_MALLOCLIKE_BLOCK(ptr, bytes, 0, false);

	ফেরত ptr;
}

শূন্য *memalign_simple(size_t align, size_t bytes)
{
	শূন্য *ptr;

	ptr = alloc_simple(bytes, align);
	যদি (!ptr)
		ফেরত ptr;
	log_debug("aligned to %lx\n", (ulong)ptr);
	VALGRIND_MALLOCLIKE_BLOCK(ptr, bytes, 0, false);

	ফেরত ptr;
}

#যদি CONFIG_IS_ENABLED(SYS_MALLOC_SIMPLE)
শূন্য *calloc(size_t nmemb, size_t elem_আকার)
{
	size_t আকার_2 = nmemb * elem_আকার;
	শূন্য *ptr;

	ptr = malloc(আকার_2);
	যদি (!ptr)
		ফেরত ptr;
	memset(ptr, '\0', আকার_2);

	ফেরত ptr;
}

#যদি IS_ENABLED(CONFIG_VALGRIND)
শূন্য free_simple(শূন্য *ptr)
{
	VALGRIND_FREELIKE_BLOCK(ptr, 0);
}
#endif
#endif

শূন্য malloc_simple_info(শূন্য)
{
	log_info("malloc_simple: %x bytes used, %x remain\n", gd->malloc_ptr,
		 CONFIG_VAL(SYS_MALLOC_F_LEN) - gd->malloc_ptr);
}
