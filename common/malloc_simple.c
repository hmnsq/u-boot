// SPDX-License-Identifier: GPL-2.0+
/*
 * Simple malloc implementation
 *
 * Copyright (c) 2014 Google, Inc
 */

#define LOG_CATEGORY LOGC_ALLOC

#अन्तर्भाव <log.h>
#अन्तर्भाव <malloc.h>
#अन्तर्भाव <mapmem.h>
#अन्तर्भाव <asm/global_data.h>
#अन्तर्भाव <asm/io.h>
#अन्तर्भाव <valgrind/valgrind.h>

DECLARE_GLOBAL_DATA_PTR;

स्थिर शून्य *alloc_simple(size_t bytes, पूर्णाङ्क align)
{
	ulong addr, new_ptr;
	शून्य *ptr;

	addr = ALIGN(gd->malloc_base + gd->malloc_ptr, align);
	new_ptr = addr + bytes - gd->malloc_base;
	log_debug("size=%lx, ptr=%lx, limit=%x: ", (ulong)bytes, new_ptr,
		  gd->malloc_limit);
	यदि (new_ptr > gd->malloc_limit) {
		log_err("alloc space exhausted ptr %lx limit %x\n", new_ptr,
			gd->malloc_limit);
		प्रत्यागम रिक्त;
	}

	ptr = map_sysmem(addr, bytes);
	gd->malloc_ptr = ALIGN(new_ptr, परिमाण(new_ptr));

	प्रत्यागम ptr;
}

शून्य *malloc_simple(size_t bytes)
{
	शून्य *ptr;

	ptr = alloc_simple(bytes, 1);
	यदि (!ptr)
		प्रत्यागम ptr;

	log_debug("%lx\n", (ulong)ptr);
	VALGRIND_MALLOCLIKE_BLOCK(ptr, bytes, 0, false);

	प्रत्यागम ptr;
}

शून्य *memalign_simple(size_t align, size_t bytes)
{
	शून्य *ptr;

	ptr = alloc_simple(bytes, align);
	यदि (!ptr)
		प्रत्यागम ptr;
	log_debug("aligned to %lx\n", (ulong)ptr);
	VALGRIND_MALLOCLIKE_BLOCK(ptr, bytes, 0, false);

	प्रत्यागम ptr;
}

#यदि CONFIG_IS_ENABLED(SYS_MALLOC_SIMPLE)
शून्य *calloc(size_t nmemb, size_t elem_size)
{
	size_t size = nmemb * elem_size;
	शून्य *ptr;

	ptr = malloc(size);
	यदि (!ptr)
		प्रत्यागम ptr;
	memset(ptr, '\0', size);

	प्रत्यागम ptr;
}

#यदि IS_ENABLED(CONFIG_VALGRIND)
शून्य free_simple(शून्य *ptr)
{
	VALGRIND_FREELIKE_BLOCK(ptr, 0);
}
#endif
#endif

शून्य malloc_simple_info(शून्य)
{
	log_info("malloc_simple: %x bytes used, %x remain\n", gd->malloc_ptr,
		 CONFIG_VAL(SYS_MALLOC_F_LEN) - gd->malloc_ptr);
}
