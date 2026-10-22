// SPDX-License-Identifier: GPL-2.0+
/*
 * Simple malloc implementation
 *
 * Copyright (c) 2014 Google, Inc
 */

#ವ್ಯಾಖ್ಯಾನಿಸು LOG_CATEGORY LOGC_ALLOC

#ಅನ್ತರ್ಭಾವ <log.h>
#ಅನ್ತರ್ಭಾವ <malloc.h>
#ಅನ್ತರ್ಭಾವ <mapmem.h>
#ಅನ್ತರ್ಭಾವ <asm/global_data.h>
#ಅನ್ತರ್ಭಾವ <asm/io.h>
#ಅನ್ತರ್ಭಾವ <valgrind/valgrind.h>

DECLARE_GLOBAL_ದತ್ತಾಂಶ_PTR;

ಸ್ಥಿರ ಶೂನ್ಯ *alloc_simple(size_t ಬೈಟುಗಳು, ಪೂರ್ಣಾಂಕ align)
{
	ulong addr, new_ptr;
	ಶೂನ್ಯ *ptr;

	addr = ALIGN(gd->malloc_ತಳ + gd->malloc_ptr, align);
	new_ptr = addr + ಬೈಟುಗಳು - gd->malloc_ತಳ;
	log_debug("size=%lx, ptr=%lx, limit=%x: ", (ulong)ಬೈಟುಗಳು, new_ptr,
		  gd->malloc_ಎಲ್ಲೆ);
	ಆದರೆ (new_ptr > gd->malloc_ಎಲ್ಲೆ) {
		log_err("alloc space exhausted ptr %lx limit %x\n", new_ptr,
			gd->malloc_ಎಲ್ಲೆ);
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	ptr = map_sysmem(addr, ಬೈಟುಗಳು);
	gd->malloc_ptr = ALIGN(new_ptr, ಗಾತ್ರ(new_ptr));

	ಹಿಂತಿರುಗಿ ptr;
}

ಶೂನ್ಯ *malloc_simple(size_t ಬೈಟುಗಳು)
{
	ಶೂನ್ಯ *ptr;

	ptr = alloc_simple(ಬೈಟುಗಳು, 1);
	ಆದರೆ (!ptr)
		ಹಿಂತಿರುಗಿ ptr;

	log_debug("%lx\n", (ulong)ptr);
	VALGRIND_MALLOCLIKE_ಖಂಡ(ptr, ಬೈಟುಗಳು, 0, false);

	ಹಿಂತಿರುಗಿ ptr;
}

ಶೂನ್ಯ *memalign_simple(size_t align, size_t ಬೈಟುಗಳು)
{
	ಶೂನ್ಯ *ptr;

	ptr = alloc_simple(ಬೈಟುಗಳು, align);
	ಆದರೆ (!ptr)
		ಹಿಂತಿರುಗಿ ptr;
	log_debug("aligned to %lx\n", (ulong)ptr);
	VALGRIND_MALLOCLIKE_ಖಂಡ(ptr, ಬೈಟುಗಳು, 0, false);

	ಹಿಂತಿರುಗಿ ptr;
}

#ಆದರೆ CONFIG_IS_ENABLED(SYS_MALLOC_SIMPLE)
ಶೂನ್ಯ *ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(size_t nmemb, size_t elem_ಗಾತ್ರ)
{
	size_t ಗಾತ್ರ_2 = nmemb * elem_ಗಾತ್ರ;
	ಶೂನ್ಯ *ptr;

	ptr = ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ_2);
	ಆದರೆ (!ptr)
		ಹಿಂತಿರುಗಿ ptr;
	memset(ptr, '\0', ಗಾತ್ರ_2);

	ಹಿಂತಿರುಗಿ ptr;
}

#ಆದರೆ IS_ENABLED(CONFIG_VALGRIND)
ಶೂನ್ಯ free_simple(ಶೂನ್ಯ *ptr)
{
	VALGRIND_FREELIKE_ಖಂಡ(ptr, 0);
}
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ

ಶೂನ್ಯ malloc_simple_info(ಶೂನ್ಯ)
{
	log_info("malloc_simple: %x bytes used, %x remain\n", gd->malloc_ptr,
		 CONFIG_VAL(SYS_MALLOC_F_LEN) - gd->malloc_ptr);
}
