// SPDX-License-Identifier: GPL-2.0+
/*
 * Simple malloc implementation
 *
 * Copyright (c) 2014 Google, Inc
 */

#घोषणा LOG_CATEGORY LOGC_ALLOC

#समावेश <log.h>
#समावेश <malloc.h>
#समावेश <mapmem.h>
#समावेश <asm/global_data.h>
#समावेश <asm/io.h>
#समावेश <valgrind/valgrind.h>

DECLARE_GLOBAL_तथ्य_PTR;

अटल व्योम *alloc_simple(माप_प्रकार bytes, पूर्णांक align)
{
	ulong addr, new_ptr;
	व्योम *ptr;

	addr = ALIGN(gd->malloc_base + gd->malloc_ptr, align);
	new_ptr = addr + bytes - gd->malloc_base;
	log_debug("size=%lx, ptr=%lx, limit=%x: ", (ulong)bytes, new_ptr,
		  gd->malloc_limit);
	अगर (new_ptr > gd->malloc_limit) {
		log_err("alloc space exhausted ptr %lx limit %x\n", new_ptr,
			gd->malloc_limit);
		वापस शून्य;
	}

	ptr = map_sysmem(addr, bytes);
	gd->malloc_ptr = ALIGN(new_ptr, माप(new_ptr));

	वापस ptr;
}

व्योम *malloc_simple(माप_प्रकार bytes)
{
	व्योम *ptr;

	ptr = alloc_simple(bytes, 1);
	अगर (!ptr)
		वापस ptr;

	log_debug("%lx\n", (ulong)ptr);
	VALGRIND_MALLOCLIKE_BLOCK(ptr, bytes, 0, false);

	वापस ptr;
}

व्योम *memalign_simple(माप_प्रकार align, माप_प्रकार bytes)
{
	व्योम *ptr;

	ptr = alloc_simple(bytes, align);
	अगर (!ptr)
		वापस ptr;
	log_debug("aligned to %lx\n", (ulong)ptr);
	VALGRIND_MALLOCLIKE_BLOCK(ptr, bytes, 0, false);

	वापस ptr;
}

#अगर CONFIG_IS_ENABLED(SYS_MALLOC_SIMPLE)
व्योम *सुस्मृति(माप_प्रकार nmemb, माप_प्रकार elem_आकार)
{
	माप_प्रकार आकार = nmemb * elem_आकार;
	व्योम *ptr;

	ptr = दो_स्मृति(आकार);
	अगर (!ptr)
		वापस ptr;
	स_रखो(ptr, '\0', आकार);

	वापस ptr;
}

#अगर IS_ENABLED(CONFIG_VALGRIND)
व्योम free_simple(व्योम *ptr)
{
	VALGRIND_FREELIKE_BLOCK(ptr, 0);
}
#पूर्ण_अगर
#पूर्ण_अगर

व्योम malloc_simple_info(व्योम)
{
	log_info("malloc_simple: %x bytes used, %x remain\n", gd->malloc_ptr,
		 CONFIG_VAL(SYS_MALLOC_F_LEN) - gd->malloc_ptr);
}
