// SPDX-License-Identifier: GPL-2.0+
/*
 * Simple malloc implementation
 *
 * Copyright (c) 2014 Google, Inc
 */

#నిర్వచించు LOG_CATEGORY LOGC_ALLOC

#అన్తర్భావ <log.h>
#అన్తర్భావ <malloc.h>
#అన్తర్భావ <mapmem.h>
#అన్తర్భావ <asm/global_data.h>
#అన్తర్భావ <asm/io.h>
#అన్తర్భావ <valgrind/valgrind.h>

DECLARE_GLOBAL_దత్తాంశం_PTR;

స్థిర శూన్య *alloc_simple(size_t బైట్లు, పూర్ణాంకం align)
{
	ulong addr, new_ptr;
	శూన్య *ptr;

	addr = ALIGN(gd->malloc_పునాది + gd->malloc_ptr, align);
	new_ptr = addr + బైట్లు - gd->malloc_పునాది;
	log_debug("size=%lx, ptr=%lx, limit=%x: ", (ulong)బైట్లు, new_ptr,
		  gd->malloc_హద్దు);
	ఒకవేళ (new_ptr > gd->malloc_హద్దు) {
		log_err("alloc space exhausted ptr %lx limit %x\n", new_ptr,
			gd->malloc_హద్దు);
		తిరిగి ఖాళీ;
	}

	ptr = map_sysmem(addr, బైట్లు);
	gd->malloc_ptr = ALIGN(new_ptr, పరిమాణం(new_ptr));

	తిరిగి ptr;
}

శూన్య *malloc_simple(size_t బైట్లు)
{
	శూన్య *ptr;

	ptr = alloc_simple(బైట్లు, 1);
	ఒకవేళ (!ptr)
		తిరిగి ptr;

	log_debug("%lx\n", (ulong)ptr);
	VALGRIND_MALLOCLIKE_ఖండం(ptr, బైట్లు, 0, false);

	తిరిగి ptr;
}

శూన్య *memalign_simple(size_t align, size_t బైట్లు)
{
	శూన్య *ptr;

	ptr = alloc_simple(బైట్లు, align);
	ఒకవేళ (!ptr)
		తిరిగి ptr;
	log_debug("aligned to %lx\n", (ulong)ptr);
	VALGRIND_MALLOCLIKE_ఖండం(ptr, బైట్లు, 0, false);

	తిరిగి ptr;
}

#ఒకవేళ CONFIG_IS_ENABLED(SYS_MALLOC_SIMPLE)
శూన్య *సున్నా_జ్ఞాపకం_ఇవ్వు(size_t nmemb, size_t elem_కొలత)
{
	size_t కొలత = nmemb * elem_కొలత;
	శూన్య *ptr;

	ptr = జ్ఞాపకం_ఇవ్వు(కొలత);
	ఒకవేళ (!ptr)
		తిరిగి ptr;
	memset(ptr, '\0', కొలత);

	తిరిగి ptr;
}

#ఒకవేళ IS_ENABLED(CONFIG_VALGRIND)
శూన్య free_simple(శూన్య *ptr)
{
	VALGRIND_FREELIKE_ఖండం(ptr, 0);
}
#అయితే_ముగింపు
#అయితే_ముగింపు

శూన్య malloc_simple_info(శూన్య)
{
	log_info("malloc_simple: %x bytes used, %x remain\n", gd->malloc_ptr,
		 CONFIG_VAL(SYS_MALLOC_F_LEN) - gd->malloc_ptr);
}
