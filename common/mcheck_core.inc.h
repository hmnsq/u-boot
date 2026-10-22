/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright (C) 2024 Free Software Foundation, Inc.
 * Written by Eugene Uriev, based on glibc 2.0 prototype of Mike Haertel.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 * <https://www.gnu.org/licenses/>
 */

/*
 * TL;DR: this is a porting of glibc mcheck into U-Boot
 *
 * This file contains no entities for external linkage.
 * So mcheck protection may be used in parallel, e.g. for "malloc_simple(..)" and "malloc(..)".
 * To do so, the file should be shared/include twice, - without linkage conflicts.
 * I.e. "core"-part is shared as a source, but not as a binary.
 * Maybe some optimization here make sense, to engage more binary sharing too.
 * But, currently I strive to keep it as simple, as possible.
 * And this, programmers'-only, mode don't pretend to be main.
 *
 * This library is aware of U-Boot specific. It's also aware of ARM alignment concerns.
 * Unlike glibc-clients, U-Boot has limited malloc-usage, and only one thread.
 * So it's better to make the protection heavier.
 * Thus overflow canary here is greater, than glibc's one. Underflow canary is bigger too.
 * U-Boot also allows to use fixed-size heap-registry, instead of double-linked list in glibc.
 *
 * Heavy canary allows to catch not only memset(..)-errors,
 * but overflow/underflow of struct-array access:
 *	{
 *		struct mystruct* p = malloc(sizeof(struct mystruct) * N);
 *		p[-1].field1 = 0;
 *		p[N].field2 = 13;
 *	}
 * TODO: In order to guarantee full coverage of that kind of errors, a user can add variable-size
 *       canaries here. So pre- and post-canary with size >= reqested_size, could be provided
 *       (with the price of 3x heap-usage). Therefore, it would catch 100% of changes beyond
 *       an array, for index(+1/-1) errors.
 *
 * U-Boot is a BL, not an OS with a lib. Activity of the library is set not in runtime,
 * rather in compile-time, by MCHECK_HEAP_PROTECTION macro. That guarantees that
 * we haven't missed first malloc.
 */

/*
 * Testing
 *  This library had been successfully tested for U-Boot @ ARM SoC chip / 64bits.
 *  Proven for both default and pedantic mode: confirms U-Boot to be clean, and catches
 *  intentional/testing corruptions. Working with malloc_trim is not tested.
 */
#ifndef _MCHECKCORE_INC_H
#define _MCHECKCORE_INC_H      1
#अन्तर्भाव "mcheck.h"

#जर defined(MCHECK_HEAP_PROTECTION)
#define mcheck_flood स_रखो

// these are from /dev/random:
#define MAGICWORD	0x99ccf430fa562a05ULL
#define MAGICFREE	0x4875e63c0c6fc08eULL
#define MAGICTAIL	0x918dbcd7df78dcd6ULL
#define MALLOCFLOOD	((अक्षर)0xb6)
#define FREEFLOOD	((अक्षर)0xf5)
#define PADDINGFLOOD	((अक्षर)0x58)

// my normal run demands 4427-6449 chunks:
#define REGISTRY_SZ	6608
#define CANARY_DEPTH	2

// avoid problems with BSS at early stage:
स्थिर अक्षर mcheck_pedantic_flag __section(".data") = 0;
स्थिर शून्य *mcheck_registry[REGISTRY_SZ] __section(".data") = {0};
स्थिर माप_प्रकार mcheck_chunk_मोजणी __section(".data") = 0;
स्थिर माप_प्रकार mcheck_chunk_मोजणी_max __section(".data") = 0;

प्रकारव्याख्या अचिन्हित दीर्घ दीर्घ mcheck_elem;
प्रकारव्याख्या संरचना {
	mcheck_elem elems[CANARY_DEPTH];
} mcheck_canary;
संरचना mcheck_hdr {
	माप_प्रकार size; /* Exact size requested by user.  */
	माप_प्रकार aln_skip; /* Ignored bytes, before the mcheck_hdr, to fulfill alignment */
	mcheck_canary canary; /* Magic number to check header integrity.  */
};

स्थिर शून्य mcheck_default_abort(गणना mcheck_status status, स्थिरांक शून्य *p)
{
	स्थिरांक अक्षर *संदेश;

	निवड (status) {
	स्थिती MCHECK_OK:
		संदेश = "memory is consistent, library is buggy\n";
		थांबा;
	स्थिती MCHECK_HEAD:
		संदेश = "memory clobbered before allocated block\n";
		थांबा;
	स्थिती MCHECK_TAIL:
		संदेश = "memory clobbered past end of allocated block\n";
		थांबा;
	स्थिती MCHECK_FREE:
		संदेश = "block freed twice\n";
		थांबा;
	पूर्वनिर्धारित:
		संदेश = "bogus mcheck_status, library is buggy\n";
		थांबा;
	}
	म_लिखो("\n\nmcheck: %p:%s!!! [%zu]\n\n", p, संदेश, mcheck_chunk_मोजणी_max);
}

स्थिर mcheck_abortfunc_t mcheck_abortfunc = &mcheck_default_abort;

स्थिर ओळीत माप_प्रकार allign_size_up(माप_प्रकार sz, माप_प्रकार grain)
{
	परत (sz + grain - 1) & ~(grain - 1);
}

#define mcheck_allign_customer_size(SZ) allign_size_up(SZ, आकार(mcheck_elem))
#define mcheck_evaluate_memalign_prefix_size(ALIGN) allign_size_up(आकार(संरचना mcheck_hdr), ALIGN)

स्थिर गणना mcheck_status mcheck_OnNok(गणना mcheck_status status, स्थिरांक शून्य *p)
{
	(*mcheck_abortfunc)(status, p);
	परत status;
}

स्थिर गणना mcheck_status mcheck_checkhdr(स्थिरांक संरचना mcheck_hdr *hdr)
{
	पूर्णांक i;

	साठी (i = 0; i < CANARY_DEPTH; ++i)
		जर (hdr->canary.elems[i] == MAGICFREE)
			परत mcheck_OnNok(MCHECK_FREE, hdr + 1);

	साठी (i = 0; i < CANARY_DEPTH; ++i)
		जर (hdr->canary.elems[i] != MAGICWORD)
			परत mcheck_OnNok(MCHECK_HEAD, hdr + 1);

	स्थिरांक माप_प्रकार payload_size = hdr->size;
	स्थिरांक माप_प्रकार payload_size_aligned = mcheck_allign_customer_size(payload_size);
	स्थिरांक माप_प्रकार padd_size = payload_size_aligned - hdr->size;

	स्थिरांक अक्षर *payload = (स्थिरांक अक्षर *)&hdr[1];

	साठी (i = 0; i < padd_size; ++i)
		जर (payload[payload_size + i] != PADDINGFLOOD)
			परत mcheck_OnNok(MCHECK_TAIL, hdr + 1);

	स्थिरांक mcheck_canary *tail = (स्थिरांक mcheck_canary *)&payload[payload_size_aligned];

	साठी (i = 0; i < CANARY_DEPTH; ++i)
		जर (tail->elems[i] != MAGICTAIL)
			परत mcheck_OnNok(MCHECK_TAIL, hdr + 1);
	परत MCHECK_OK;
}

गणना { KEEP_CONTENT = 0, CLEAN_CONTENT, ANY_ALIGNMENT = 1 };
स्थिर शून्य *mcheck_free_helper(शून्य *ptr, पूर्णांक clean_content)
{
	जर (!ptr)
		परत ptr;

	संरचना mcheck_hdr *hdr = &((संरचना mcheck_hdr *)ptr)[-1];
	पूर्णांक i;

	mcheck_checkhdr(hdr);
	साठी (i = 0; i < CANARY_DEPTH; ++i)
		hdr->canary.elems[i] = MAGICFREE;

	जर (clean_content)
		mcheck_flood(ptr, FREEFLOOD, mcheck_allign_customer_size(hdr->size));

	साठी (i = 0; i < REGISTRY_SZ; ++i)
		जर (mcheck_registry[i] == hdr) {
			mcheck_registry[i] = 0;
			थांबा;
		}

	--mcheck_chunk_मोजणी;
	परत (अक्षर *)hdr - hdr->aln_skip;
}

स्थिर शून्य *mcheck_free_prehook(शून्य *ptr) { परत mcheck_free_helper(ptr, CLEAN_CONTENT); }
स्थिर शून्य *mcheck_reallocfree_prehook(शून्य *ptr) { परत mcheck_free_helper(ptr, KEEP_CONTENT); }

स्थिर माप_प्रकार mcheck_alloc_prehook(माप_प्रकार sz)
{
	sz = mcheck_allign_customer_size(sz);
	परत आकार(संरचना mcheck_hdr) + sz + आकार(mcheck_canary);
}

स्थिर शून्य *mcheck_allocated_helper(शून्य *altoghether_ptr, माप_प्रकार customer_sz,
				     माप_प्रकार alignment, पूर्णांक clean_content)
{
	स्थिरांक माप_प्रकार slop = alignment ?
		mcheck_evaluate_memalign_prefix_size(alignment) - आकार(संरचना mcheck_hdr) : 0;
	संरचना mcheck_hdr *hdr = (संरचना mcheck_hdr *)((अक्षर *)altoghether_ptr + slop);
	पूर्णांक i;

	hdr->size = customer_sz;
	hdr->aln_skip = slop;
	साठी (i = 0; i < CANARY_DEPTH; ++i)
		hdr->canary.elems[i] = MAGICWORD;

	अक्षर *payload = (अक्षर *)&hdr[1];

	जर (clean_content)
		mcheck_flood(payload, MALLOCFLOOD, customer_sz);

	स्थिरांक माप_प्रकार customer_size_aligned = mcheck_allign_customer_size(customer_sz);

	mcheck_flood(payload + customer_sz, PADDINGFLOOD, customer_size_aligned - customer_sz);

	mcheck_canary *tail = (mcheck_canary *)&payload[customer_size_aligned];

	साठी (i = 0; i < CANARY_DEPTH; ++i)
		tail->elems[i] = MAGICTAIL;

	++mcheck_chunk_मोजणी;
	जर (mcheck_chunk_मोजणी > mcheck_chunk_मोजणी_max)
		mcheck_chunk_मोजणी_max = mcheck_chunk_मोजणी;

	साठी (i = 0; i < REGISTRY_SZ; ++i)
		जर (!mcheck_registry[i]) {
			mcheck_registry[i] = hdr;
			परत payload; // normal end
		}

	स्थिर अक्षर *overflow_संदेश = "\n\n\nERROR: mcheck registry overflow, pedantic check would be incomplete!!\n\n\n\n";

	म_लिखो("%s", overflow_संदेश);
	overflow_संदेश = "(mcheck registry full)";
	परत payload;
}

स्थिर शून्य *mcheck_alloc_posthook(शून्य *altoghether_ptr, माप_प्रकार customer_sz)
{
	परत mcheck_allocated_helper(altoghether_ptr, customer_sz, ANY_ALIGNMENT, CLEAN_CONTENT);
}

स्थिर शून्य *mcheck_alloc_noclean_posthook(शून्य *altoghether_ptr, माप_प्रकार customer_sz)
{
	परत mcheck_allocated_helper(altoghether_ptr, customer_sz, ANY_ALIGNMENT, KEEP_CONTENT);
}

स्थिर माप_प्रकार mcheck_memalign_prehook(माप_प्रकार alig, माप_प्रकार sz)
{
	परत mcheck_evaluate_memalign_prefix_size(alig) + sz + आकार(mcheck_canary);
}

स्थिर शून्य *mcheck_memalign_posthook(माप_प्रकार alignment, शून्य *altoghether_ptr, माप_प्रकार customer_sz)
{
	परत mcheck_allocated_helper(altoghether_ptr, customer_sz, alignment, CLEAN_CONTENT);
}

स्थिर गणना mcheck_status mcheck_mprobe(शून्य *ptr)
{
	संरचना mcheck_hdr *hdr = &((संरचना mcheck_hdr *)ptr)[-1];

	परत mcheck_checkhdr(hdr);
}

स्थिर शून्य mcheck_pedantic_check(शून्य)
{
	पूर्णांक i;

	साठी (i = 0; i < REGISTRY_SZ; ++i)
		जर (mcheck_registry[i])
			mcheck_checkhdr(mcheck_registry[i]);
}

स्थिर शून्य mcheck_pedantic_prehook(शून्य)
{
	जर (mcheck_pedantic_flag)
		mcheck_pedantic_check();
}

स्थिर शून्य mcheck_initialize(mcheck_abortfunc_t new_func, अक्षर pedantic_flag)
{
	mcheck_abortfunc = (new_func) ? new_func : &mcheck_default_abort;
	mcheck_pedantic_flag = pedantic_flag;
}

शून्य mcheck_on_ramrelocation(माप_प्रकार offset)
{
	अक्षर *p;
	पूर्णांक i;
	// Simple, but inaccurate strategy: drop the pre-reloc heap
	साठी (i = 0; i < REGISTRY_SZ; ++i)
		जर ((p = mcheck_registry[i]) != रिक्त ) {
			म_लिखो("mcheck, WRN: forgetting %p chunk\n", p);
			mcheck_registry[i] = 0;
		}

	mcheck_chunk_मोजणी = 0;
}
#endif
#endif
