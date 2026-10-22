// SPDX-License-Identifier: GPL-2.0+
/*
 * Simple malloc implementation
 *
 * Copyright (c) 2014 Google, Inc
 */

#परिभाषय LOG_CATEGORY LOGC_ALLOC

#अन्तर्भाव <log.h>
#अन्तर्भाव <malloc.h>
#अन्तर्भाव <mapmem.h>
#अन्तर्भाव <asm/global_data.h>
#अन्तर्भाव <asm/io.h>
#अन्तर्भाव <valgrind/valgrind.h>

DECLARE_GLOBAL_दत्त_PTR;

स्थिर शून्य *alloc_simple(size_t अष्टकानि, पूर्णाङ्क align)
{
	ulong addr, new_ptr;
	शून्य *ptr;

	addr = ALIGN(gd->malloc_आधारः + gd->malloc_ptr, align);
	new_ptr = addr + अष्टकानि - gd->malloc_आधारः;
	log_debug("size=%lx, ptr=%lx, limit=%x: ", (ulong)अष्टकानि, new_ptr,
		  gd->malloc_सीमा);
	यदि (new_ptr > gd->malloc_सीमा) {
		log_err("alloc space exhausted ptr %lx limit %x\n", new_ptr,
			gd->malloc_सीमा);
		प्रत्यागम रिक्त;
	}

	ptr = map_sysmem(addr, अष्टकानि);
	gd->malloc_ptr = ALIGN(new_ptr, परिमाण(new_ptr));

	प्रत्यागम ptr;
}

शून्य *malloc_simple(size_t अष्टकानि)
{
	शून्य *ptr;

	ptr = alloc_simple(अष्टकानि, 1);
	यदि (!ptr)
		प्रत्यागम ptr;

	log_debug("%lx\n", (ulong)ptr);
	VALGRIND_MALLOCLIKE_खण्डम्(ptr, अष्टकानि, 0, false);

	प्रत्यागम ptr;
}

शून्य *memalign_simple(size_t align, size_t अष्टकानि)
{
	शून्य *ptr;

	ptr = alloc_simple(अष्टकानि, align);
	यदि (!ptr)
		प्रत्यागम ptr;
	log_debug("aligned to %lx\n", (ulong)ptr);
	VALGRIND_MALLOCLIKE_खण्डम्(ptr, अष्टकानि, 0, false);

	प्रत्यागम ptr;
}

#यदि CONFIG_IS_ENABLED(SYS_MALLOC_SIMPLE)
शून्य *शून्यस्मृतिं_देहि(size_t nmemb, size_t elem_आकार)
{
	size_t आकार = nmemb * elem_आकार;
	शून्य *ptr;

	ptr = स्मृतिं_देहि(आकार);
	यदि (!ptr)
		प्रत्यागम ptr;
	स्मृतिं_पूरय(ptr, '\0', आकार);

	प्रत्यागम ptr;
}

#यदि IS_ENABLED(CONFIG_VALGRIND)
शून्य free_simple(शून्य *ptr)
{
	VALGRIND_FREELIKE_खण्डम्(ptr, 0);
}
#यदि_अन्तः
#यदि_अन्तः

शून्य malloc_simple_info(शून्य)
{
	log_info("malloc_simple: %x bytes used, %x remain\n", gd->malloc_ptr,
		 CONFIG_VAL(SYS_MALLOC_F_LEN) - gd->malloc_ptr);
}
