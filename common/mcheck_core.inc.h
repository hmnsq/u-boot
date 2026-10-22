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
#यदि_अपरिभाषितम् _MCHECKCORE_INC_H
#परिभाषय _MCHECKCORE_INC_H      1
#अन्तर्भाव "mcheck.h"

#यदि defined(MCHECK_HEAP_PROTECTION)
#परिभाषय mcheck_flood स्मृतिं_पूरय

// these are from /dev/random:
#परिभाषय MAGICWORD	0x99ccf430fa562a05ULL
#परिभाषय MAGICFREE	0x4875e63c0c6fc08eULL
#परिभाषय MAGICTAIL	0x918dbcd7df78dcd6ULL
#परिभाषय MALLOCFLOOD	((वर्ण)0xb6)
#परिभाषय FREEFLOOD	((वर्ण)0xf5)
#परिभाषय PADDINGFLOOD	((वर्ण)0x58)

// my normal run demands 4427-6449 chunks:
#परिभाषय REGISTRY_SZ	6608
#परिभाषय CANARY_गभीरता	2

// avoid problems with BSS at early stage:
स्थिर वर्ण mcheck_pedantic_flag __section(".data") = 0;
स्थिर शून्य *mcheck_registry[REGISTRY_SZ] __section(".data") = {0};
स्थिर size_t mcheck_chunk_गणनम् __section(".data") = 0;
स्थिर size_t mcheck_chunk_गणनम्_max __section(".data") = 0;

प्रकारनिर्देश अचिह्नित दीर्घ दीर्घ mcheck_elem;
प्रकारनिर्देश संरचना {
	mcheck_elem elems[CANARY_गभीरता];
} mcheck_canary;
संरचना mcheck_hdr {
	size_t आकार; /* Exact size requested by user.  */
	size_t aln_skip; /* Ignored bytes, before the mcheck_hdr, to fulfill alignment */
	mcheck_canary canary; /* Magic number to check header integrity.  */
};

स्थिर शून्य mcheck_default_abort(गणना mcheck_status स्थितिः, ध्रुव शून्य *p)
{
	ध्रुव वर्ण *सन्देशः;

	विकल्प (स्थितिः) {
	पक्ष MCHECK_OK:
		सन्देशः = "memory is consistent, library is buggy\n";
		भङ्ग;
	पक्ष MCHECK_HEAD:
		सन्देशः = "memory clobbered before allocated block\n";
		भङ्ग;
	पक्ष MCHECK_TAIL:
		सन्देशः = "memory clobbered past end of allocated block\n";
		भङ्ग;
	पक्ष MCHECK_FREE:
		सन्देशः = "block freed twice\n";
		भङ्ग;
	स्वभाव:
		सन्देशः = "bogus mcheck_status, library is buggy\n";
		भङ्ग;
	}
	प्रारूप_लिख("\n\nmcheck: %p:%s!!! [%zu]\n\n", p, सन्देशः, mcheck_chunk_गणनम्_max);
}

स्थिर mcheck_abortfunc_t mcheck_abortfunc = &mcheck_default_abort;

स्थिर अन्तःस्थ size_t allign_आकार_ऊर्ध्वम्(size_t sz, size_t grain)
{
	प्रत्यागम (sz + grain - 1) & ~(grain - 1);
}

#परिभाषय mcheck_allign_customer_आकार(SZ) allign_आकार_ऊर्ध्वम्(SZ, परिमाण(mcheck_elem))
#परिभाषय mcheck_evaluate_memalign_उपसर्गः_आकार(ALIGN) allign_आकार_ऊर्ध्वम्(परिमाण(संरचना mcheck_hdr), ALIGN)

स्थिर गणना mcheck_status mcheck_OnNok(गणना mcheck_status स्थितिः, ध्रुव शून्य *p)
{
	(*mcheck_abortfunc)(स्थितिः, p);
	प्रत्यागम स्थितिः;
}

स्थिर गणना mcheck_status mcheck_checkhdr(ध्रुव संरचना mcheck_hdr *hdr)
{
	पूर्णाङ्क i;

	प्रति (i = 0; i < CANARY_गभीरता; ++i)
		यदि (hdr->canary.elems[i] == MAGICFREE)
			प्रत्यागम mcheck_OnNok(MCHECK_FREE, hdr + 1);

	प्रति (i = 0; i < CANARY_गभीरता; ++i)
		यदि (hdr->canary.elems[i] != MAGICWORD)
			प्रत्यागम mcheck_OnNok(MCHECK_HEAD, hdr + 1);

	ध्रुव size_t payload_आकार = hdr->आकार;
	ध्रुव size_t payload_आकार_aligned = mcheck_allign_customer_आकार(payload_आकार);
	ध्रुव size_t padd_आकार = payload_आकार_aligned - hdr->आकार;

	ध्रुव वर्ण *payload = (ध्रुव वर्ण *)&hdr[1];

	प्रति (i = 0; i < padd_आकार; ++i)
		यदि (payload[payload_आकार + i] != PADDINGFLOOD)
			प्रत्यागम mcheck_OnNok(MCHECK_TAIL, hdr + 1);

	ध्रुव mcheck_canary *पुच्छ = (ध्रुव mcheck_canary *)&payload[payload_आकार_aligned];

	प्रति (i = 0; i < CANARY_गभीरता; ++i)
		यदि (पुच्छ->elems[i] != MAGICTAIL)
			प्रत्यागम mcheck_OnNok(MCHECK_TAIL, hdr + 1);
	प्रत्यागम MCHECK_OK;
}

गणना { KEEP_CONTENT = 0, CLEAN_CONTENT, ANY_ALIGNMENT = 1 };
स्थिर शून्य *mcheck_free_helper(शून्य *ptr, पूर्णाङ्क clean_content)
{
	यदि (!ptr)
		प्रत्यागम ptr;

	संरचना mcheck_hdr *hdr = &((संरचना mcheck_hdr *)ptr)[-1];
	पूर्णाङ्क i;

	mcheck_checkhdr(hdr);
	प्रति (i = 0; i < CANARY_गभीरता; ++i)
		hdr->canary.elems[i] = MAGICFREE;

	यदि (clean_content)
		mcheck_flood(ptr, FREEFLOOD, mcheck_allign_customer_आकार(hdr->आकार));

	प्रति (i = 0; i < REGISTRY_SZ; ++i)
		यदि (mcheck_registry[i] == hdr) {
			mcheck_registry[i] = 0;
			भङ्ग;
		}

	--mcheck_chunk_गणनम्;
	प्रत्यागम (वर्ण *)hdr - hdr->aln_skip;
}

स्थिर शून्य *mcheck_free_prehook(शून्य *ptr) { प्रत्यागम mcheck_free_helper(ptr, CLEAN_CONTENT); }
स्थिर शून्य *mcheck_reallocfree_prehook(शून्य *ptr) { प्रत्यागम mcheck_free_helper(ptr, KEEP_CONTENT); }

स्थिर size_t mcheck_alloc_prehook(size_t sz)
{
	sz = mcheck_allign_customer_आकार(sz);
	प्रत्यागम परिमाण(संरचना mcheck_hdr) + sz + परिमाण(mcheck_canary);
}

स्थिर शून्य *mcheck_allocated_helper(शून्य *altoghether_ptr, size_t customer_sz,
				     size_t alignment, पूर्णाङ्क clean_content)
{
	ध्रुव size_t slop = alignment ?
		mcheck_evaluate_memalign_उपसर्गः_आकार(alignment) - परिमाण(संरचना mcheck_hdr) : 0;
	संरचना mcheck_hdr *hdr = (संरचना mcheck_hdr *)((वर्ण *)altoghether_ptr + slop);
	पूर्णाङ्क i;

	hdr->आकार = customer_sz;
	hdr->aln_skip = slop;
	प्रति (i = 0; i < CANARY_गभीरता; ++i)
		hdr->canary.elems[i] = MAGICWORD;

	वर्ण *payload = (वर्ण *)&hdr[1];

	यदि (clean_content)
		mcheck_flood(payload, MALLOCFLOOD, customer_sz);

	ध्रुव size_t customer_आकार_aligned = mcheck_allign_customer_आकार(customer_sz);

	mcheck_flood(payload + customer_sz, PADDINGFLOOD, customer_आकार_aligned - customer_sz);

	mcheck_canary *पुच्छ = (mcheck_canary *)&payload[customer_आकार_aligned];

	प्रति (i = 0; i < CANARY_गभीरता; ++i)
		पुच्छ->elems[i] = MAGICTAIL;

	++mcheck_chunk_गणनम्;
	यदि (mcheck_chunk_गणनम् > mcheck_chunk_गणनम्_max)
		mcheck_chunk_गणनम्_max = mcheck_chunk_गणनम्;

	प्रति (i = 0; i < REGISTRY_SZ; ++i)
		यदि (!mcheck_registry[i]) {
			mcheck_registry[i] = hdr;
			प्रत्यागम payload; // normal end
		}

	स्थिर वर्ण *अतिप्रवाहः_सन्देशः = "\n\n\nERROR: mcheck registry overflow, pedantic check would be incomplete!!\n\n\n\n";

	प्रारूप_लिख("%s", अतिप्रवाहः_सन्देशः);
	अतिप्रवाहः_सन्देशः = "(mcheck registry full)";
	प्रत्यागम payload;
}

स्थिर शून्य *mcheck_alloc_posthook(शून्य *altoghether_ptr, size_t customer_sz)
{
	प्रत्यागम mcheck_allocated_helper(altoghether_ptr, customer_sz, ANY_ALIGNMENT, CLEAN_CONTENT);
}

स्थिर शून्य *mcheck_alloc_noclean_posthook(शून्य *altoghether_ptr, size_t customer_sz)
{
	प्रत्यागम mcheck_allocated_helper(altoghether_ptr, customer_sz, ANY_ALIGNMENT, KEEP_CONTENT);
}

स्थिर size_t mcheck_memalign_prehook(size_t alig, size_t sz)
{
	प्रत्यागम mcheck_evaluate_memalign_उपसर्गः_आकार(alig) + sz + परिमाण(mcheck_canary);
}

स्थिर शून्य *mcheck_memalign_posthook(size_t alignment, शून्य *altoghether_ptr, size_t customer_sz)
{
	प्रत्यागम mcheck_allocated_helper(altoghether_ptr, customer_sz, alignment, CLEAN_CONTENT);
}

स्थिर गणना mcheck_status mcheck_mprobe(शून्य *ptr)
{
	संरचना mcheck_hdr *hdr = &((संरचना mcheck_hdr *)ptr)[-1];

	प्रत्यागम mcheck_checkhdr(hdr);
}

स्थिर शून्य mcheck_pedantic_check(शून्य)
{
	पूर्णाङ्क i;

	प्रति (i = 0; i < REGISTRY_SZ; ++i)
		यदि (mcheck_registry[i])
			mcheck_checkhdr(mcheck_registry[i]);
}

स्थिर शून्य mcheck_pedantic_prehook(शून्य)
{
	यदि (mcheck_pedantic_flag)
		mcheck_pedantic_check();
}

स्थिर शून्य mcheck_initialize(mcheck_abortfunc_t new_func, वर्ण pedantic_flag)
{
	mcheck_abortfunc = (new_func) ? new_func : &mcheck_default_abort;
	mcheck_pedantic_flag = pedantic_flag;
}

शून्य mcheck_on_ramrelocation(size_t विस्थापनम्)
{
	वर्ण *p;
	पूर्णाङ्क i;
	// Simple, but inaccurate strategy: drop the pre-reloc heap
	प्रति (i = 0; i < REGISTRY_SZ; ++i)
		यदि ((p = mcheck_registry[i]) != रिक्त ) {
			प्रारूप_लिख("mcheck, WRN: forgetting %p chunk\n", p);
			mcheck_registry[i] = 0;
		}

	mcheck_chunk_गणनम् = 0;
}
#यदि_अन्तः
#यदि_अन्तः
