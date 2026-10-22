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
#ضمن "mcheck.h"

#إذا defined(MCHECK_HEAP_PROTECTION)
#define mcheck_flood memset

// these are from /dev/random:
#define MAGICWORD	0x99ccf430fa562a05ULL
#define MAGICFREE	0x4875e63c0c6fc08eULL
#define MAGICTAIL	0x918dbcd7df78dcd6ULL
#define MALLOCFLOOD	((حرف)0xb6)
#define FREEFLOOD	((حرف)0xf5)
#define PADDINGFLOOD	((حرف)0x58)

// my normal run demands 4427-6449 chunks:
#define REGISTRY_SZ	6608
#define CANARY_DEPTH	2

// avoid problems with BSS at early stage:
ساكن حرف mcheck_pedantic_flag __section(".data") = 0;
ساكن فراغ *mcheck_registry[REGISTRY_SZ] __section(".data") = {0};
ساكن size_t mcheck_chunk_كدي __section(".data") = 0;
ساكن size_t mcheck_chunk_كدي_max __section(".data") = 0;

تعريفنوع unsigned long long mcheck_elem;
تعريفنوع بنية {
	mcheck_elem elems[CANARY_DEPTH];
} mcheck_canary;
بنية mcheck_hdr {
	size_t size; /* Exact size requested by user.  */
	size_t aln_skip; /* Ignored bytes, before the mcheck_hdr, to fulfill alignment */
	mcheck_canary canary; /* Magic number to check header integrity.  */
};

ساكن فراغ mcheck_default_abort(تعداد mcheck_status status, ثابت فراغ *p)
{
	ثابت حرف *سكو;

	بدل (status) {
	حالة MCHECK_OK:
		سكو = "memory is consistent, library is buggy\n";
		اكسر;
	حالة MCHECK_HEAD:
		سكو = "memory clobbered before allocated block\n";
		اكسر;
	حالة MCHECK_TAIL:
		سكو = "memory clobbered past end of allocated block\n";
		اكسر;
	حالة MCHECK_FREE:
		سكو = "block freed twice\n";
		اكسر;
	افتراضي:
		سكو = "bogus mcheck_status, library is buggy\n";
		اكسر;
	}
	printf("\n\nmcheck: %p:%s!!! [%zu]\n\n", p, سكو, mcheck_chunk_كدي_max);
}

ساكن mcheck_abortfunc_t mcheck_abortfunc = &mcheck_default_abort;

ساكن مضمن size_t allign_size_up(size_t sz, size_t grain)
{
	أرجع (sz + grain - 1) & ~(grain - 1);
}

#define mcheck_allign_customer_size(SZ) allign_size_up(SZ, حجم(mcheck_elem))
#define mcheck_evaluate_memalign_prefix_size(ALIGN) allign_size_up(حجم(بنية mcheck_hdr), ALIGN)

ساكن تعداد mcheck_status mcheck_OnNok(تعداد mcheck_status status, ثابت فراغ *p)
{
	(*mcheck_abortfunc)(status, p);
	أرجع status;
}

ساكن تعداد mcheck_status mcheck_checkhdr(ثابت بنية mcheck_hdr *hdr)
{
	صحيح i;

	لكل (i = 0; i < CANARY_DEPTH; ++i)
		إذا (hdr->canary.elems[i] == MAGICFREE)
			أرجع mcheck_OnNok(MCHECK_FREE, hdr + 1);

	لكل (i = 0; i < CANARY_DEPTH; ++i)
		إذا (hdr->canary.elems[i] != MAGICWORD)
			أرجع mcheck_OnNok(MCHECK_HEAD, hdr + 1);

	ثابت size_t payload_size = hdr->size;
	ثابت size_t payload_size_aligned = mcheck_allign_customer_size(payload_size);
	ثابت size_t padd_size = payload_size_aligned - hdr->size;

	ثابت حرف *payload = (ثابت حرف *)&hdr[1];

	لكل (i = 0; i < padd_size; ++i)
		إذا (payload[payload_size + i] != PADDINGFLOOD)
			أرجع mcheck_OnNok(MCHECK_TAIL, hdr + 1);

	ثابت mcheck_canary *tail = (ثابت mcheck_canary *)&payload[payload_size_aligned];

	لكل (i = 0; i < CANARY_DEPTH; ++i)
		إذا (tail->elems[i] != MAGICTAIL)
			أرجع mcheck_OnNok(MCHECK_TAIL, hdr + 1);
	أرجع MCHECK_OK;
}

تعداد { KEEP_CONTENT = 0, CLEAN_CONTENT, ANY_ALIGNMENT = 1 };
ساكن فراغ *mcheck_free_helper(فراغ *ptr, صحيح clean_content)
{
	إذا (!ptr)
		أرجع ptr;

	بنية mcheck_hdr *hdr = &((بنية mcheck_hdr *)ptr)[-1];
	صحيح i;

	mcheck_checkhdr(hdr);
	لكل (i = 0; i < CANARY_DEPTH; ++i)
		hdr->canary.elems[i] = MAGICFREE;

	إذا (clean_content)
		mcheck_flood(ptr, FREEFLOOD, mcheck_allign_customer_size(hdr->size));

	لكل (i = 0; i < REGISTRY_SZ; ++i)
		إذا (mcheck_registry[i] == hdr) {
			mcheck_registry[i] = 0;
			اكسر;
		}

	--mcheck_chunk_كدي;
	أرجع (حرف *)hdr - hdr->aln_skip;
}

ساكن فراغ *mcheck_free_prehook(فراغ *ptr) { أرجع mcheck_free_helper(ptr, CLEAN_CONTENT); }
ساكن فراغ *mcheck_reallocfree_prehook(فراغ *ptr) { أرجع mcheck_free_helper(ptr, KEEP_CONTENT); }

ساكن size_t mcheck_alloc_prehook(size_t sz)
{
	sz = mcheck_allign_customer_size(sz);
	أرجع حجم(بنية mcheck_hdr) + sz + حجم(mcheck_canary);
}

ساكن فراغ *mcheck_allocated_helper(فراغ *altoghether_ptr, size_t customer_sz,
				     size_t alignment, صحيح clean_content)
{
	ثابت size_t slop = alignment ?
		mcheck_evaluate_memalign_prefix_size(alignment) - حجم(بنية mcheck_hdr) : 0;
	بنية mcheck_hdr *hdr = (بنية mcheck_hdr *)((حرف *)altoghether_ptr + slop);
	صحيح i;

	hdr->size = customer_sz;
	hdr->aln_skip = slop;
	لكل (i = 0; i < CANARY_DEPTH; ++i)
		hdr->canary.elems[i] = MAGICWORD;

	حرف *payload = (حرف *)&hdr[1];

	إذا (clean_content)
		mcheck_flood(payload, MALLOCFLOOD, customer_sz);

	ثابت size_t customer_size_aligned = mcheck_allign_customer_size(customer_sz);

	mcheck_flood(payload + customer_sz, PADDINGFLOOD, customer_size_aligned - customer_sz);

	mcheck_canary *tail = (mcheck_canary *)&payload[customer_size_aligned];

	لكل (i = 0; i < CANARY_DEPTH; ++i)
		tail->elems[i] = MAGICTAIL;

	++mcheck_chunk_كدي;
	إذا (mcheck_chunk_كدي > mcheck_chunk_كدي_max)
		mcheck_chunk_كدي_max = mcheck_chunk_كدي;

	لكل (i = 0; i < REGISTRY_SZ; ++i)
		إذا (!mcheck_registry[i]) {
			mcheck_registry[i] = hdr;
			أرجع payload; // normal end
		}

	ساكن حرف *overflow_سكو = "\n\n\nERROR: mcheck registry overflow, pedantic check would be incomplete!!\n\n\n\n";

	printf("%s", overflow_سكو);
	overflow_سكو = "(mcheck registry full)";
	أرجع payload;
}

ساكن فراغ *mcheck_alloc_posthook(فراغ *altoghether_ptr, size_t customer_sz)
{
	أرجع mcheck_allocated_helper(altoghether_ptr, customer_sz, ANY_ALIGNMENT, CLEAN_CONTENT);
}

ساكن فراغ *mcheck_alloc_noclean_posthook(فراغ *altoghether_ptr, size_t customer_sz)
{
	أرجع mcheck_allocated_helper(altoghether_ptr, customer_sz, ANY_ALIGNMENT, KEEP_CONTENT);
}

ساكن size_t mcheck_memalign_prehook(size_t alig, size_t sz)
{
	أرجع mcheck_evaluate_memalign_prefix_size(alig) + sz + حجم(mcheck_canary);
}

ساكن فراغ *mcheck_memalign_posthook(size_t alignment, فراغ *altoghether_ptr, size_t customer_sz)
{
	أرجع mcheck_allocated_helper(altoghether_ptr, customer_sz, alignment, CLEAN_CONTENT);
}

ساكن تعداد mcheck_status mcheck_mprobe(فراغ *ptr)
{
	بنية mcheck_hdr *hdr = &((بنية mcheck_hdr *)ptr)[-1];

	أرجع mcheck_checkhdr(hdr);
}

ساكن فراغ mcheck_pedantic_check(فراغ)
{
	صحيح i;

	لكل (i = 0; i < REGISTRY_SZ; ++i)
		إذا (mcheck_registry[i])
			mcheck_checkhdr(mcheck_registry[i]);
}

ساكن فراغ mcheck_pedantic_prehook(فراغ)
{
	إذا (mcheck_pedantic_flag)
		mcheck_pedantic_check();
}

ساكن فراغ mcheck_initialize(mcheck_abortfunc_t new_func, حرف pedantic_flag)
{
	mcheck_abortfunc = (new_func) ? new_func : &mcheck_default_abort;
	mcheck_pedantic_flag = pedantic_flag;
}

فراغ mcheck_on_ramrelocation(size_t offset)
{
	حرف *p;
	صحيح i;
	// Simple, but inaccurate strategy: drop the pre-reloc heap
	لكل (i = 0; i < REGISTRY_SZ; ++i)
		إذا ((p = mcheck_registry[i]) != عدم ) {
			printf("mcheck, WRN: forgetting %p chunk\n", p);
			mcheck_registry[i] = 0;
		}

	mcheck_chunk_كدي = 0;
}
#endif
#endif
