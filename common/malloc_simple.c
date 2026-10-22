// SPDX-License-Identifier: GPL-2.0+
/*
 * Simple malloc implementation
 *
 * Copyright (c) 2014 Google, Inc
 */

#وصف_ڪريو LOG_CATEGORY LOGC_ALLOC

#ضمن <log.h>
#ضمن <malloc.h>
#ضمن <mapmem.h>
#ضمن <asm/global_data.h>
#ضمن <asm/io.h>
#ضمن <valgrind/valgrind.h>

DECLARE_GLOBAL_معلومات_PTR;

ساكن فراغ *alloc_simple(size_t بائيٽون, صحيح align)
{
	ulong addr, new_ptr;
	فراغ *ptr;

	addr = ALIGN(gd->malloc_بنياد + gd->malloc_ptr, align);
	new_ptr = addr + بائيٽون - gd->malloc_بنياد;
	log_debug("size=%lx, ptr=%lx, limit=%x: ", (ulong)بائيٽون, new_ptr,
		  gd->malloc_حد_بندي);
	إذا (new_ptr > gd->malloc_حد_بندي) {
		log_err("alloc space exhausted ptr %lx limit %x\n", new_ptr,
			gd->malloc_حد_بندي);
		أرجع عدم;
	}

	ptr = map_sysmem(addr, بائيٽون);
	gd->malloc_ptr = ALIGN(new_ptr, حجم(new_ptr));

	أرجع ptr;
}

فراغ *malloc_simple(size_t بائيٽون)
{
	فراغ *ptr;

	ptr = alloc_simple(بائيٽون, 1);
	إذا (!ptr)
		أرجع ptr;

	log_debug("%lx\n", (ulong)ptr);
	VALGRIND_MALLOCLIKE_بلاڪ(ptr, بائيٽون, 0, false);

	أرجع ptr;
}

فراغ *memalign_simple(size_t align, size_t بائيٽون)
{
	فراغ *ptr;

	ptr = alloc_simple(بائيٽون, align);
	إذا (!ptr)
		أرجع ptr;
	log_debug("aligned to %lx\n", (ulong)ptr);
	VALGRIND_MALLOCLIKE_بلاڪ(ptr, بائيٽون, 0, false);

	أرجع ptr;
}

#إذا CONFIG_IS_ENABLED(SYS_MALLOC_SIMPLE)
فراغ *صفر_يادگيري_ڏيو(size_t nmemb, size_t elem_سائيز)
{
	size_t سائيز = nmemb * elem_سائيز;
	فراغ *ptr;

	ptr = يادگيري_ڏيو(سائيز);
	إذا (!ptr)
		أرجع ptr;
	memset(ptr, '\0', سائيز);

	أرجع ptr;
}

#إذا IS_ENABLED(CONFIG_VALGRIND)
فراغ free_simple(فراغ *ptr)
{
	VALGRIND_FREELIKE_بلاڪ(ptr, 0);
}
#جيڪڏهن_پورو
#جيڪڏهن_پورو

فراغ malloc_simple_info(فراغ)
{
	log_info("malloc_simple: %x bytes used, %x remain\n", gd->malloc_ptr,
		 CONFIG_VAL(SYS_MALLOC_F_LEN) - gd->malloc_ptr);
}
