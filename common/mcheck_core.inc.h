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
#অন্তর্ভাৱ "mcheck.h"

#যদি defined(MCHECK_HEAP_PROTECTION)
#define mcheck_flood memset

// these are from /dev/random:
#define MAGICWORD	0x99ccf430fa562a05ULL
#define MAGICFREE	0x4875e63c0c6fc08eULL
#define MAGICTAIL	0x918dbcd7df78dcd6ULL
#define MALLOCFLOOD	((ৱর্ণ)0xb6)
#define FREEFLOOD	((ৱর্ণ)0xf5)
#define PADDINGFLOOD	((ৱর্ণ)0x58)

// my normal run demands 4427-6449 chunks:
#define REGISTRY_SZ	6608
#define CANARY_DEPTH	2

// avoid problems with BSS at early stage:
স্থির ৱর্ণ mcheck_pedantic_flag __section(".data") = 0;
স্থির শূন্য *mcheck_registry[REGISTRY_SZ] __section(".data") = {0};
স্থির size_t mcheck_chunk_গুনতি __section(".data") = 0;
স্থির size_t mcheck_chunk_গুনতি_max __section(".data") = 0;

প্রকারনির্দেশ unsigned long long mcheck_elem;
প্রকারনির্দেশ সংরচনা {
	mcheck_elem elems[CANARY_DEPTH];
} mcheck_canary;
সংরচনা mcheck_hdr {
	size_t size; /* Exact size requested by user.  */
	size_t aln_skip; /* Ignored bytes, before the mcheck_hdr, to fulfill alignment */
	mcheck_canary canary; /* Magic number to check header integrity.  */
};

স্থির শূন্য mcheck_default_abort(গণনা mcheck_status status, ধ্রুৱ শূন্য *p)
{
	ধ্রুৱ ৱর্ণ *বাৰ্তা;

	ৱিকল্প (status) {
	পক্ষ MCHECK_OK:
		বাৰ্তা = "memory is consistent, library is buggy\n";
		ভঙ্গ;
	পক্ষ MCHECK_HEAD:
		বাৰ্তা = "memory clobbered before allocated block\n";
		ভঙ্গ;
	পক্ষ MCHECK_TAIL:
		বাৰ্তা = "memory clobbered past end of allocated block\n";
		ভঙ্গ;
	পক্ষ MCHECK_FREE:
		বাৰ্তা = "block freed twice\n";
		ভঙ্গ;
	স্ৱভাৱ:
		বাৰ্তা = "bogus mcheck_status, library is buggy\n";
		ভঙ্গ;
	}
	printf("\n\nmcheck: %p:%s!!! [%zu]\n\n", p, বাৰ্তা, mcheck_chunk_গুনতি_max);
}

স্থির mcheck_abortfunc_t mcheck_abortfunc = &mcheck_default_abort;

স্থির অন্তঃস্থ size_t allign_size_up(size_t sz, size_t grain)
{
	ঘূৰাই (sz + grain - 1) & ~(grain - 1);
}

#define mcheck_allign_customer_size(SZ) allign_size_up(SZ, পরিমাণ(mcheck_elem))
#define mcheck_evaluate_memalign_prefix_size(ALIGN) allign_size_up(পরিমাণ(সংরচনা mcheck_hdr), ALIGN)

স্থির গণনা mcheck_status mcheck_OnNok(গণনা mcheck_status status, ধ্রুৱ শূন্য *p)
{
	(*mcheck_abortfunc)(status, p);
	ঘূৰাই status;
}

স্থির গণনা mcheck_status mcheck_checkhdr(ধ্রুৱ সংরচনা mcheck_hdr *hdr)
{
	পূর্ণাঙ্ক i;

	বাবে (i = 0; i < CANARY_DEPTH; ++i)
		যদি (hdr->canary.elems[i] == MAGICFREE)
			ঘূৰাই mcheck_OnNok(MCHECK_FREE, hdr + 1);

	বাবে (i = 0; i < CANARY_DEPTH; ++i)
		যদি (hdr->canary.elems[i] != MAGICWORD)
			ঘূৰাই mcheck_OnNok(MCHECK_HEAD, hdr + 1);

	ধ্রুৱ size_t payload_size = hdr->size;
	ধ্রুৱ size_t payload_size_aligned = mcheck_allign_customer_size(payload_size);
	ধ্রুৱ size_t padd_size = payload_size_aligned - hdr->size;

	ধ্রুৱ ৱর্ণ *payload = (ধ্রুৱ ৱর্ণ *)&hdr[1];

	বাবে (i = 0; i < padd_size; ++i)
		যদি (payload[payload_size + i] != PADDINGFLOOD)
			ঘূৰাই mcheck_OnNok(MCHECK_TAIL, hdr + 1);

	ধ্রুৱ mcheck_canary *tail = (ধ্রুৱ mcheck_canary *)&payload[payload_size_aligned];

	বাবে (i = 0; i < CANARY_DEPTH; ++i)
		যদি (tail->elems[i] != MAGICTAIL)
			ঘূৰাই mcheck_OnNok(MCHECK_TAIL, hdr + 1);
	ঘূৰাই MCHECK_OK;
}

গণনা { KEEP_CONTENT = 0, CLEAN_CONTENT, ANY_ALIGNMENT = 1 };
স্থির শূন্য *mcheck_free_helper(শূন্য *ptr, পূর্ণাঙ্ক clean_content)
{
	যদি (!ptr)
		ঘূৰাই ptr;

	সংরচনা mcheck_hdr *hdr = &((সংরচনা mcheck_hdr *)ptr)[-1];
	পূর্ণাঙ্ক i;

	mcheck_checkhdr(hdr);
	বাবে (i = 0; i < CANARY_DEPTH; ++i)
		hdr->canary.elems[i] = MAGICFREE;

	যদি (clean_content)
		mcheck_flood(ptr, FREEFLOOD, mcheck_allign_customer_size(hdr->size));

	বাবে (i = 0; i < REGISTRY_SZ; ++i)
		যদি (mcheck_registry[i] == hdr) {
			mcheck_registry[i] = 0;
			ভঙ্গ;
		}

	--mcheck_chunk_গুনতি;
	ঘূৰাই (ৱর্ণ *)hdr - hdr->aln_skip;
}

স্থির শূন্য *mcheck_free_prehook(শূন্য *ptr) { ঘূৰাই mcheck_free_helper(ptr, CLEAN_CONTENT); }
স্থির শূন্য *mcheck_reallocfree_prehook(শূন্য *ptr) { ঘূৰাই mcheck_free_helper(ptr, KEEP_CONTENT); }

স্থির size_t mcheck_alloc_prehook(size_t sz)
{
	sz = mcheck_allign_customer_size(sz);
	ঘূৰাই পরিমাণ(সংরচনা mcheck_hdr) + sz + পরিমাণ(mcheck_canary);
}

স্থির শূন্য *mcheck_allocated_helper(শূন্য *altoghether_ptr, size_t customer_sz,
				     size_t alignment, পূর্ণাঙ্ক clean_content)
{
	ধ্রুৱ size_t slop = alignment ?
		mcheck_evaluate_memalign_prefix_size(alignment) - পরিমাণ(সংরচনা mcheck_hdr) : 0;
	সংরচনা mcheck_hdr *hdr = (সংরচনা mcheck_hdr *)((ৱর্ণ *)altoghether_ptr + slop);
	পূর্ণাঙ্ক i;

	hdr->size = customer_sz;
	hdr->aln_skip = slop;
	বাবে (i = 0; i < CANARY_DEPTH; ++i)
		hdr->canary.elems[i] = MAGICWORD;

	ৱর্ণ *payload = (ৱর্ণ *)&hdr[1];

	যদি (clean_content)
		mcheck_flood(payload, MALLOCFLOOD, customer_sz);

	ধ্রুৱ size_t customer_size_aligned = mcheck_allign_customer_size(customer_sz);

	mcheck_flood(payload + customer_sz, PADDINGFLOOD, customer_size_aligned - customer_sz);

	mcheck_canary *tail = (mcheck_canary *)&payload[customer_size_aligned];

	বাবে (i = 0; i < CANARY_DEPTH; ++i)
		tail->elems[i] = MAGICTAIL;

	++mcheck_chunk_গুনতি;
	যদি (mcheck_chunk_গুনতি > mcheck_chunk_গুনতি_max)
		mcheck_chunk_গুনতি_max = mcheck_chunk_গুনতি;

	বাবে (i = 0; i < REGISTRY_SZ; ++i)
		যদি (!mcheck_registry[i]) {
			mcheck_registry[i] = hdr;
			ঘূৰাই payload; // normal end
		}

	স্থির ৱর্ণ *overflow_বাৰ্তা = "\n\n\nERROR: mcheck registry overflow, pedantic check would be incomplete!!\n\n\n\n";

	printf("%s", overflow_বাৰ্তা);
	overflow_বাৰ্তা = "(mcheck registry full)";
	ঘূৰাই payload;
}

স্থির শূন্য *mcheck_alloc_posthook(শূন্য *altoghether_ptr, size_t customer_sz)
{
	ঘূৰাই mcheck_allocated_helper(altoghether_ptr, customer_sz, ANY_ALIGNMENT, CLEAN_CONTENT);
}

স্থির শূন্য *mcheck_alloc_noclean_posthook(শূন্য *altoghether_ptr, size_t customer_sz)
{
	ঘূৰাই mcheck_allocated_helper(altoghether_ptr, customer_sz, ANY_ALIGNMENT, KEEP_CONTENT);
}

স্থির size_t mcheck_memalign_prehook(size_t alig, size_t sz)
{
	ঘূৰাই mcheck_evaluate_memalign_prefix_size(alig) + sz + পরিমাণ(mcheck_canary);
}

স্থির শূন্য *mcheck_memalign_posthook(size_t alignment, শূন্য *altoghether_ptr, size_t customer_sz)
{
	ঘূৰাই mcheck_allocated_helper(altoghether_ptr, customer_sz, alignment, CLEAN_CONTENT);
}

স্থির গণনা mcheck_status mcheck_mprobe(শূন্য *ptr)
{
	সংরচনা mcheck_hdr *hdr = &((সংরচনা mcheck_hdr *)ptr)[-1];

	ঘূৰাই mcheck_checkhdr(hdr);
}

স্থির শূন্য mcheck_pedantic_check(শূন্য)
{
	পূর্ণাঙ্ক i;

	বাবে (i = 0; i < REGISTRY_SZ; ++i)
		যদি (mcheck_registry[i])
			mcheck_checkhdr(mcheck_registry[i]);
}

স্থির শূন্য mcheck_pedantic_prehook(শূন্য)
{
	যদি (mcheck_pedantic_flag)
		mcheck_pedantic_check();
}

স্থির শূন্য mcheck_initialize(mcheck_abortfunc_t new_func, ৱর্ণ pedantic_flag)
{
	mcheck_abortfunc = (new_func) ? new_func : &mcheck_default_abort;
	mcheck_pedantic_flag = pedantic_flag;
}

শূন্য mcheck_on_ramrelocation(size_t offset)
{
	ৱর্ণ *p;
	পূর্ণাঙ্ক i;
	// Simple, but inaccurate strategy: drop the pre-reloc heap
	বাবে (i = 0; i < REGISTRY_SZ; ++i)
		যদি ((p = mcheck_registry[i]) != রিক্ত ) {
			printf("mcheck, WRN: forgetting %p chunk\n", p);
			mcheck_registry[i] = 0;
		}

	mcheck_chunk_গুনতি = 0;
}
#endif
#endif
