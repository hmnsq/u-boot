// SPDX-License-Identifier: GPL-2.0+
/*
 * Simple malloc implementation
 *
 * Copyright (c) 2014 Google, Inc
 */

#تعریف_کن LOG_CATEGORY LOGC_ALLOC

#ضمن <log.h>
#ضمن <malloc.h>
#ضمن <mapmem.h>
#ضمن <asm/global_data.h>
#ضمن <asm/io.h>
#ضمن <valgrind/valgrind.h>

DECLARE_GLOBAL_داده_PTR;

ساكن فراغ *alloc_simple(size_t بایتها, صحيح align)
{
	ulong addr, new_ptr;
	فراغ *ptr;

	addr = ALIGN(gd->malloc_پایه + gd->malloc_ptr, align);
	new_ptr = addr + بایتها - gd->malloc_پایه;
	log_debug("size=%lx, ptr=%lx, limit=%x: ", (ulong)بایتها, new_ptr,
		  gd->malloc_حد);
	إذا (new_ptr > gd->malloc_حد) {
		log_err("alloc space exhausted ptr %lx limit %x\n", new_ptr,
			gd->malloc_حد);
		أرجع عدم;
	}

	ptr = map_sysmem(addr, بایتها);
	gd->malloc_ptr = ALIGN(new_ptr, حجم(new_ptr));

	أرجع ptr;
}

فراغ *malloc_simple(size_t بایتها)
{
	فراغ *ptr;

	ptr = alloc_simple(بایتها, 1);
	إذا (!ptr)
		أرجع ptr;

	log_debug("%lx\n", (ulong)ptr);
	VALGRIND_MALLOCLIKE_بلوک(ptr, بایتها, 0, false);

	أرجع ptr;
}

فراغ *memalign_simple(size_t align, size_t بایتها)
{
	فراغ *ptr;

	ptr = alloc_simple(بایتها, align);
	إذا (!ptr)
		أرجع ptr;
	log_debug("aligned to %lx\n", (ulong)ptr);
	VALGRIND_MALLOCLIKE_بلوک(ptr, بایتها, 0, false);

	أرجع ptr;
}

#إذا CONFIG_IS_ENABLED(SYS_MALLOC_SIMPLE)
فراغ *حافظه_صفر_بده(size_t nmemb, size_t elem_اندازه)
{
	size_t اندازه = nmemb * elem_اندازه;
	فراغ *ptr;

	ptr = حافظه_بده(اندازه);
	إذا (!ptr)
		أرجع ptr;
	memset(ptr, '\0', اندازه);

	أرجع ptr;
}

#إذا IS_ENABLED(CONFIG_VALGRIND)
فراغ free_simple(فراغ *ptr)
{
	VALGRIND_FREELIKE_بلوک(ptr, 0);
}
#پایان_اگر
#پایان_اگر

فراغ malloc_simple_info(فراغ)
{
	log_info("malloc_simple: %x bytes used, %x remain\n", gd->malloc_ptr,
		 CONFIG_VAL(SYS_MALLOC_F_LEN) - gd->malloc_ptr);
}
