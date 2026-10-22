// SPDX-License-Identifier: GPL-2.0+
/*
 * Simple malloc implementation
 *
 * Copyright (c) 2014 Google, Inc
 */

#define LOG_CATEGORY LOGC_ALLOC

#അന്തര്ഭാവ <log.h>
#അന്തര്ഭാവ <malloc.h>
#അന്തര്ഭാവ <mapmem.h>
#അന്തര്ഭാവ <asm/global_data.h>
#അന്തര്ഭാവ <asm/io.h>
#അന്തര്ഭാവ <valgrind/valgrind.h>

DECLARE_GLOBAL_DATA_PTR;

സ്ഥിതം ശൂന്യം *alloc_simple(size_t bytes, പൂർണ്ണസംഖ്യ align)
{
	ulong addr, new_ptr;
	ശൂന്യം *ptr;

	addr = ALIGN(gd->malloc_base + gd->malloc_ptr, align);
	new_ptr = addr + bytes - gd->malloc_base;
	log_debug("size=%lx, ptr=%lx, limit=%x: ", (ulong)bytes, new_ptr,
		  gd->malloc_limit);
	എങ്കിൽ (new_ptr > gd->malloc_limit) {
		log_err("alloc space exhausted ptr %lx limit %x\n", new_ptr,
			gd->malloc_limit);
		തിരികെ ശൂന്യ;
	}

	ptr = map_sysmem(addr, bytes);
	gd->malloc_ptr = ALIGN(new_ptr, വലുപ്പം(new_ptr));

	തിരികെ ptr;
}

ശൂന്യം *malloc_simple(size_t bytes)
{
	ശൂന്യം *ptr;

	ptr = alloc_simple(bytes, 1);
	എങ്കിൽ (!ptr)
		തിരികെ ptr;

	log_debug("%lx\n", (ulong)ptr);
	VALGRIND_MALLOCLIKE_BLOCK(ptr, bytes, 0, false);

	തിരികെ ptr;
}

ശൂന്യം *memalign_simple(size_t align, size_t bytes)
{
	ശൂന്യം *ptr;

	ptr = alloc_simple(bytes, align);
	എങ്കിൽ (!ptr)
		തിരികെ ptr;
	log_debug("aligned to %lx\n", (ulong)ptr);
	VALGRIND_MALLOCLIKE_BLOCK(ptr, bytes, 0, false);

	തിരികെ ptr;
}

#എങ്കിൽ CONFIG_IS_ENABLED(SYS_MALLOC_SIMPLE)
ശൂന്യം *calloc(size_t nmemb, size_t elem_size)
{
	size_t size = nmemb * elem_size;
	ശൂന്യം *ptr;

	ptr = malloc(size);
	എങ്കിൽ (!ptr)
		തിരികെ ptr;
	memset(ptr, '\0', size);

	തിരികെ ptr;
}

#എങ്കിൽ IS_ENABLED(CONFIG_VALGRIND)
ശൂന്യം free_simple(ശൂന്യം *ptr)
{
	VALGRIND_FREELIKE_BLOCK(ptr, 0);
}
#endif
#endif

ശൂന്യം malloc_simple_info(ശൂന്യം)
{
	log_info("malloc_simple: %x bytes used, %x remain\n", gd->malloc_ptr,
		 CONFIG_VAL(SYS_MALLOC_F_LEN) - gd->malloc_ptr);
}
