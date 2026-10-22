// SPDX-License-Identifier: GPL-2.0+
/*
 * Simple malloc implementation
 *
 * Copyright (c) 2014 Google, Inc
 */

#تعریف_کرو LOG_CATEGORY LOGC_ALLOC

#ضمن <log.h>
#ضمن <malloc.h>
#ضمن <mapmem.h>
#ضمن <asm/global_data.h>
#ضمن <asm/io.h>
#ضمن <valgrind/valgrind.h>

DECLARE_GLOBAL_انکڑے_PTR;

ساكن فراغ *alloc_simple(size_t بائٹاں, صحيح align)
{
	ulong addr, new_ptr;
	فراغ *ptr;

	addr = ALIGN(gd->malloc_ادھار + gd->malloc_ptr, align);
	new_ptr = addr + بائٹاں - gd->malloc_ادھار;
	log_debug("size=%lx, ptr=%lx, limit=%x: ", (ulong)بائٹاں, new_ptr,
		  gd->malloc_حد_بندی);
	إذا (new_ptr > gd->malloc_حد_بندی) {
		log_err("alloc space exhausted ptr %lx limit %x\n", new_ptr,
			gd->malloc_حد_بندی);
		أرجع عدم;
	}

	ptr = map_sysmem(addr, بائٹاں);
	gd->malloc_ptr = ALIGN(new_ptr, حجم(new_ptr));

	أرجع ptr;
}

فراغ *malloc_simple(size_t بائٹاں)
{
	فراغ *ptr;

	ptr = alloc_simple(بائٹاں, 1);
	إذا (!ptr)
		أرجع ptr;

	log_debug("%lx\n", (ulong)ptr);
	VALGRIND_MALLOCLIKE_بلاک(ptr, بائٹاں, 0, false);

	أرجع ptr;
}

فراغ *memalign_simple(size_t align, size_t بائٹاں)
{
	فراغ *ptr;

	ptr = alloc_simple(بائٹاں, align);
	إذا (!ptr)
		أرجع ptr;
	log_debug("aligned to %lx\n", (ulong)ptr);
	VALGRIND_MALLOCLIKE_بلاک(ptr, بائٹاں, 0, false);

	أرجع ptr;
}

#إذا CONFIG_IS_ENABLED(SYS_MALLOC_SIMPLE)
فراغ *صفر_حافظہ_دیو(size_t nmemb, size_t elem_ناپ)
{
	size_t ناپ = nmemb * elem_ناپ;
	فراغ *ptr;

	ptr = حافظہ_دیو(ناپ);
	إذا (!ptr)
		أرجع ptr;
	memset(ptr, '\0', ناپ);

	أرجع ptr;
}

#إذا IS_ENABLED(CONFIG_VALGRIND)
فراغ free_simple(فراغ *ptr)
{
	VALGRIND_FREELIKE_بلاک(ptr, 0);
}
#جے_انت
#جے_انت

فراغ malloc_simple_info(فراغ)
{
	log_info("malloc_simple: %x bytes used, %x remain\n", gd->malloc_ptr,
		 CONFIG_VAL(SYS_MALLOC_F_LEN) - gd->malloc_ptr);
}
