// SPDX-License-Identifier: GPL-2.0+
/*
 * Simple malloc implementation
 *
 * Copyright (c) 2014 Google, Inc
 */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LOG_CATEGORY LOGC_ALLOC

#ਅਨ੍ਤਰ੍ਭਾਵ <log.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <malloc.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <mapmem.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/global_data.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/io.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <valgrind/valgrind.h>

DECLARE_GLOBAL_ਅੰਕੜੇ_PTR;

ਸਥਿਰ ਖਾਲੀ *alloc_simple(size_t ਬਾਈਟਾਂ, ਪੂਰਨ align)
{
	ulong addr, new_ptr;
	ਖਾਲੀ *ptr;

	addr = ALIGN(gd->malloc_ਅਧਾਰ + gd->malloc_ptr, align);
	new_ptr = addr + ਬਾਈਟਾਂ - gd->malloc_ਅਧਾਰ;
	log_debug("size=%lx, ptr=%lx, limit=%x: ", (ulong)ਬਾਈਟਾਂ, new_ptr,
		  gd->malloc_ਹੱਦਬੰਦੀ);
	ਜੇ (new_ptr > gd->malloc_ਹੱਦਬੰਦੀ) {
		log_err("alloc space exhausted ptr %lx limit %x\n", new_ptr,
			gd->malloc_ਹੱਦਬੰਦੀ);
		ਵਾਪਸ ਨਿਰਾ;
	}

	ptr = map_sysmem(addr, ਬਾਈਟਾਂ);
	gd->malloc_ptr = ALIGN(new_ptr, ਆਕਾਰ(new_ptr));

	ਵਾਪਸ ptr;
}

ਖਾਲੀ *malloc_simple(size_t ਬਾਈਟਾਂ)
{
	ਖਾਲੀ *ptr;

	ptr = alloc_simple(ਬਾਈਟਾਂ, 1);
	ਜੇ (!ptr)
		ਵਾਪਸ ptr;

	log_debug("%lx\n", (ulong)ptr);
	VALGRIND_MALLOCLIKE_ਖੰਡ(ptr, ਬਾਈਟਾਂ, 0, false);

	ਵਾਪਸ ptr;
}

ਖਾਲੀ *memalign_simple(size_t align, size_t ਬਾਈਟਾਂ)
{
	ਖਾਲੀ *ptr;

	ptr = alloc_simple(ਬਾਈਟਾਂ, align);
	ਜੇ (!ptr)
		ਵਾਪਸ ptr;
	log_debug("aligned to %lx\n", (ulong)ptr);
	VALGRIND_MALLOCLIKE_ਖੰਡ(ptr, ਬਾਈਟਾਂ, 0, false);

	ਵਾਪਸ ptr;
}

#ਜੇ CONFIG_IS_ENABLED(SYS_MALLOC_SIMPLE)
ਖਾਲੀ *ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(size_t nmemb, size_t elem_ਮਾਪ)
{
	size_t ਮਾਪ = nmemb * elem_ਮਾਪ;
	ਖਾਲੀ *ptr;

	ptr = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਮਾਪ);
	ਜੇ (!ptr)
		ਵਾਪਸ ptr;
	memset(ptr, '\0', ਮਾਪ);

	ਵਾਪਸ ptr;
}

#ਜੇ IS_ENABLED(CONFIG_VALGRIND)
ਖਾਲੀ free_simple(ਖਾਲੀ *ptr)
{
	VALGRIND_FREELIKE_ਖੰਡ(ptr, 0);
}
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ

ਖਾਲੀ malloc_simple_info(ਖਾਲੀ)
{
	log_info("malloc_simple: %x bytes used, %x remain\n", gd->malloc_ptr,
		 CONFIG_VAL(SYS_MALLOC_F_LEN) - gd->malloc_ptr);
}
