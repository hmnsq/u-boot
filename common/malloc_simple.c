// SPDX-License-Identifier: GPL-2.0+
/*
 * Simple malloc implementation
 *
 * Copyright (c) 2014 Google, Inc
 */

#تعریف_کرو LOG_CATEGORY LOGC_ALLOC

#شامل <log.h>
#شامل <malloc.h>
#شامل <mapmem.h>
#شامل <asm/global_data.h>
#شامل <asm/io.h>
#شامل <valgrind/valgrind.h>

DECLARE_GLOBAL_ڈیٹا_PTR;

ساکن خالی *alloc_simple(size_t بائٹیں, صحیح align)
{
	ulong addr, new_ptr;
	خالی *ptr;

	addr = ALIGN(gd->malloc_بنیاد + gd->malloc_ptr, align);
	new_ptr = addr + بائٹیں - gd->malloc_بنیاد;
	log_debug("size=%lx, ptr=%lx, limit=%x: ", (ulong)بائٹیں, new_ptr,
		  gd->malloc_حد_بندی);
	اگر (new_ptr > gd->malloc_حد_بندی) {
		log_err("alloc space exhausted ptr %lx limit %x\n", new_ptr,
			gd->malloc_حد_بندی);
		واپس تہی;
	}

	ptr = map_sysmem(addr, بائٹیں);
	gd->malloc_ptr = ALIGN(new_ptr, حجم(new_ptr));

	واپس ptr;
}

خالی *malloc_simple(size_t بائٹیں)
{
	خالی *ptr;

	ptr = alloc_simple(بائٹیں, 1);
	اگر (!ptr)
		واپس ptr;

	log_debug("%lx\n", (ulong)ptr);
	VALGRIND_MALLOCLIKE_بلاک(ptr, بائٹیں, 0, false);

	واپس ptr;
}

خالی *memalign_simple(size_t align, size_t بائٹیں)
{
	خالی *ptr;

	ptr = alloc_simple(بائٹیں, align);
	اگر (!ptr)
		واپس ptr;
	log_debug("aligned to %lx\n", (ulong)ptr);
	VALGRIND_MALLOCLIKE_بلاک(ptr, بائٹیں, 0, false);

	واپس ptr;
}

#اگر CONFIG_IS_ENABLED(SYS_MALLOC_SIMPLE)
خالی *صفر_حافظہ_دو(size_t nmemb, size_t elem_سائز)
{
	size_t سائز = nmemb * elem_سائز;
	خالی *ptr;

	ptr = حافظہ_دو(سائز);
	اگر (!ptr)
		واپس ptr;
	memset(ptr, '\0', سائز);

	واپس ptr;
}

#اگر IS_ENABLED(CONFIG_VALGRIND)
خالی free_simple(خالی *ptr)
{
	VALGRIND_FREELIKE_بلاک(ptr, 0);
}
#اگر_ختم
#اگر_ختم

خالی malloc_simple_info(خالی)
{
	log_info("malloc_simple: %x bytes used, %x remain\n", gd->malloc_ptr,
		 CONFIG_VAL(SYS_MALLOC_F_LEN) - gd->malloc_ptr);
}
