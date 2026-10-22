// SPDX-License-Identifier: GPL-2.0+
/*
 * Simple malloc implementation
 *
 * Copyright (c) 2014 Google, Inc
 */

#define LOG_CATEGORY LOGC_ALLOC

#ಅನ್ತರ್ಭಾವ <log.h>
#ಅನ್ತರ್ಭಾವ <malloc.h>
#ಅನ್ತರ್ಭಾವ <mapmem.h>
#ಅನ್ತರ್ಭಾವ <asm/global_data.h>
#ಅನ್ತರ್ಭಾವ <asm/io.h>
#ಅನ್ತರ್ಭಾವ <valgrind/valgrind.h>

DECLARE_GLOBAL_DATA_PTR;

ಸ್ಥಿರ ಶೂನ್ಯ *alloc_simple(size_t bytes, ಪೂರ್ಣಾಙ್ಕ align)
{
	ulong addr, new_ptr;
	ಶೂನ್ಯ *ptr;

	addr = ALIGN(gd->malloc_base + gd->malloc_ptr, align);
	new_ptr = addr + bytes - gd->malloc_base;
	log_debug("size=%lx, ptr=%lx, limit=%x: ", (ulong)bytes, new_ptr,
		  gd->malloc_limit);
	ಯದಿ (new_ptr > gd->malloc_limit) {
		log_err("alloc space exhausted ptr %lx limit %x\n", new_ptr,
			gd->malloc_limit);
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}

	ptr = map_sysmem(addr, bytes);
	gd->malloc_ptr = ALIGN(new_ptr, ಪರಿಮಾಣ(new_ptr));

	ಪ್ರತ್ಯಾಗಮ ptr;
}

ಶೂನ್ಯ *malloc_simple(size_t bytes)
{
	ಶೂನ್ಯ *ptr;

	ptr = alloc_simple(bytes, 1);
	ಯದಿ (!ptr)
		ಪ್ರತ್ಯಾಗಮ ptr;

	log_debug("%lx\n", (ulong)ptr);
	VALGRIND_MALLOCLIKE_BLOCK(ptr, bytes, 0, false);

	ಪ್ರತ್ಯಾಗಮ ptr;
}

ಶೂನ್ಯ *memalign_simple(size_t align, size_t bytes)
{
	ಶೂನ್ಯ *ptr;

	ptr = alloc_simple(bytes, align);
	ಯದಿ (!ptr)
		ಪ್ರತ್ಯಾಗಮ ptr;
	log_debug("aligned to %lx\n", (ulong)ptr);
	VALGRIND_MALLOCLIKE_BLOCK(ptr, bytes, 0, false);

	ಪ್ರತ್ಯಾಗಮ ptr;
}

#ಯದಿ CONFIG_IS_ENABLED(SYS_MALLOC_SIMPLE)
ಶೂನ್ಯ *calloc(size_t nmemb, size_t elem_size)
{
	size_t size = nmemb * elem_size;
	ಶೂನ್ಯ *ptr;

	ptr = malloc(size);
	ಯದಿ (!ptr)
		ಪ್ರತ್ಯಾಗಮ ptr;
	memset(ptr, '\0', size);

	ಪ್ರತ್ಯಾಗಮ ptr;
}

#ಯದಿ IS_ENABLED(CONFIG_VALGRIND)
ಶೂನ್ಯ free_simple(ಶೂನ್ಯ *ptr)
{
	VALGRIND_FREELIKE_BLOCK(ptr, 0);
}
#endif
#endif

ಶೂನ್ಯ malloc_simple_info(ಶೂನ್ಯ)
{
	log_info("malloc_simple: %x bytes used, %x remain\n", gd->malloc_ptr,
		 CONFIG_VAL(SYS_MALLOC_F_LEN) - gd->malloc_ptr);
}
