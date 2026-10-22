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
#شامل "mcheck.h"

#اگر defined(MCHECK_HEAP_PROTECTION)
#define mcheck_flood memset

// these are from /dev/random:
#define MAGICWORD	0x99ccf430fa562a05ULL
#define MAGICFREE	0x4875e63c0c6fc08eULL
#define MAGICTAIL	0x918dbcd7df78dcd6ULL
#define MALLOCFLOOD	((نویسه)0xb6)
#define FREEFLOOD	((نویسه)0xf5)
#define PADDINGFLOOD	((نویسه)0x58)

// my normal run demands 4427-6449 chunks:
#define REGISTRY_SZ	6608
#define CANARY_DEPTH	2

// avoid problems with BSS at early stage:
ایستا نویسه mcheck_pedantic_flag __section(".data") = 0;
ایستا تهی *mcheck_registry[REGISTRY_SZ] __section(".data") = {0};
ایستا size_t mcheck_chunk_شمار __section(".data") = 0;
ایستا size_t mcheck_chunk_شمار_max __section(".data") = 0;

تعریفنوع unsigned long long mcheck_elem;
تعریفنوع ساختار {
	mcheck_elem elems[CANARY_DEPTH];
} mcheck_canary;
ساختار mcheck_hdr {
	size_t size; /* Exact size requested by user.  */
	size_t aln_skip; /* Ignored bytes, before the mcheck_hdr, to fulfill alignment */
	mcheck_canary canary; /* Magic number to check header integrity.  */
};

ایستا تهی mcheck_default_abort(شمارش mcheck_status status, ثابت تهی *p)
{
	ثابت نویسه *پیام;

	انتخاب (status) {
	حالت MCHECK_OK:
		پیام = "memory is consistent, library is buggy\n";
		بشکن;
	حالت MCHECK_HEAD:
		پیام = "memory clobbered before allocated block\n";
		بشکن;
	حالت MCHECK_TAIL:
		پیام = "memory clobbered past end of allocated block\n";
		بشکن;
	حالت MCHECK_FREE:
		پیام = "block freed twice\n";
		بشکن;
	پیشفرض:
		پیام = "bogus mcheck_status, library is buggy\n";
		بشکن;
	}
	printf("\n\nmcheck: %p:%s!!! [%zu]\n\n", p, پیام, mcheck_chunk_شمار_max);
}

ایستا mcheck_abortfunc_t mcheck_abortfunc = &mcheck_default_abort;

ایستا درونخطی size_t allign_size_up(size_t sz, size_t grain)
{
	بازگشت (sz + grain - 1) & ~(grain - 1);
}

#define mcheck_allign_customer_size(SZ) allign_size_up(SZ, اندازه(mcheck_elem))
#define mcheck_evaluate_memalign_prefix_size(ALIGN) allign_size_up(اندازه(ساختار mcheck_hdr), ALIGN)

ایستا شمارش mcheck_status mcheck_OnNok(شمارش mcheck_status status, ثابت تهی *p)
{
	(*mcheck_abortfunc)(status, p);
	بازگشت status;
}

ایستا شمارش mcheck_status mcheck_checkhdr(ثابت ساختار mcheck_hdr *hdr)
{
	صحیح i;

	برای (i = 0; i < CANARY_DEPTH; ++i)
		اگر (hdr->canary.elems[i] == MAGICFREE)
			بازگشت mcheck_OnNok(MCHECK_FREE, hdr + 1);

	برای (i = 0; i < CANARY_DEPTH; ++i)
		اگر (hdr->canary.elems[i] != MAGICWORD)
			بازگشت mcheck_OnNok(MCHECK_HEAD, hdr + 1);

	ثابت size_t payload_size = hdr->size;
	ثابت size_t payload_size_aligned = mcheck_allign_customer_size(payload_size);
	ثابت size_t padd_size = payload_size_aligned - hdr->size;

	ثابت نویسه *payload = (ثابت نویسه *)&hdr[1];

	برای (i = 0; i < padd_size; ++i)
		اگر (payload[payload_size + i] != PADDINGFLOOD)
			بازگشت mcheck_OnNok(MCHECK_TAIL, hdr + 1);

	ثابت mcheck_canary *tail = (ثابت mcheck_canary *)&payload[payload_size_aligned];

	برای (i = 0; i < CANARY_DEPTH; ++i)
		اگر (tail->elems[i] != MAGICTAIL)
			بازگشت mcheck_OnNok(MCHECK_TAIL, hdr + 1);
	بازگشت MCHECK_OK;
}

شمارش { KEEP_CONTENT = 0, CLEAN_CONTENT, ANY_ALIGNMENT = 1 };
ایستا تهی *mcheck_free_helper(تهی *ptr, صحیح clean_content)
{
	اگر (!ptr)
		بازگشت ptr;

	ساختار mcheck_hdr *hdr = &((ساختار mcheck_hdr *)ptr)[-1];
	صحیح i;

	mcheck_checkhdr(hdr);
	برای (i = 0; i < CANARY_DEPTH; ++i)
		hdr->canary.elems[i] = MAGICFREE;

	اگر (clean_content)
		mcheck_flood(ptr, FREEFLOOD, mcheck_allign_customer_size(hdr->size));

	برای (i = 0; i < REGISTRY_SZ; ++i)
		اگر (mcheck_registry[i] == hdr) {
			mcheck_registry[i] = 0;
			بشکن;
		}

	--mcheck_chunk_شمار;
	بازگشت (نویسه *)hdr - hdr->aln_skip;
}

ایستا تهی *mcheck_free_prehook(تهی *ptr) { بازگشت mcheck_free_helper(ptr, CLEAN_CONTENT); }
ایستا تهی *mcheck_reallocfree_prehook(تهی *ptr) { بازگشت mcheck_free_helper(ptr, KEEP_CONTENT); }

ایستا size_t mcheck_alloc_prehook(size_t sz)
{
	sz = mcheck_allign_customer_size(sz);
	بازگشت اندازه(ساختار mcheck_hdr) + sz + اندازه(mcheck_canary);
}

ایستا تهی *mcheck_allocated_helper(تهی *altoghether_ptr, size_t customer_sz,
				     size_t alignment, صحیح clean_content)
{
	ثابت size_t slop = alignment ?
		mcheck_evaluate_memalign_prefix_size(alignment) - اندازه(ساختار mcheck_hdr) : 0;
	ساختار mcheck_hdr *hdr = (ساختار mcheck_hdr *)((نویسه *)altoghether_ptr + slop);
	صحیح i;

	hdr->size = customer_sz;
	hdr->aln_skip = slop;
	برای (i = 0; i < CANARY_DEPTH; ++i)
		hdr->canary.elems[i] = MAGICWORD;

	نویسه *payload = (نویسه *)&hdr[1];

	اگر (clean_content)
		mcheck_flood(payload, MALLOCFLOOD, customer_sz);

	ثابت size_t customer_size_aligned = mcheck_allign_customer_size(customer_sz);

	mcheck_flood(payload + customer_sz, PADDINGFLOOD, customer_size_aligned - customer_sz);

	mcheck_canary *tail = (mcheck_canary *)&payload[customer_size_aligned];

	برای (i = 0; i < CANARY_DEPTH; ++i)
		tail->elems[i] = MAGICTAIL;

	++mcheck_chunk_شمار;
	اگر (mcheck_chunk_شمار > mcheck_chunk_شمار_max)
		mcheck_chunk_شمار_max = mcheck_chunk_شمار;

	برای (i = 0; i < REGISTRY_SZ; ++i)
		اگر (!mcheck_registry[i]) {
			mcheck_registry[i] = hdr;
			بازگشت payload; // normal end
		}

	ایستا نویسه *overflow_پیام = "\n\n\nERROR: mcheck registry overflow, pedantic check would be incomplete!!\n\n\n\n";

	printf("%s", overflow_پیام);
	overflow_پیام = "(mcheck registry full)";
	بازگشت payload;
}

ایستا تهی *mcheck_alloc_posthook(تهی *altoghether_ptr, size_t customer_sz)
{
	بازگشت mcheck_allocated_helper(altoghether_ptr, customer_sz, ANY_ALIGNMENT, CLEAN_CONTENT);
}

ایستا تهی *mcheck_alloc_noclean_posthook(تهی *altoghether_ptr, size_t customer_sz)
{
	بازگشت mcheck_allocated_helper(altoghether_ptr, customer_sz, ANY_ALIGNMENT, KEEP_CONTENT);
}

ایستا size_t mcheck_memalign_prehook(size_t alig, size_t sz)
{
	بازگشت mcheck_evaluate_memalign_prefix_size(alig) + sz + اندازه(mcheck_canary);
}

ایستا تهی *mcheck_memalign_posthook(size_t alignment, تهی *altoghether_ptr, size_t customer_sz)
{
	بازگشت mcheck_allocated_helper(altoghether_ptr, customer_sz, alignment, CLEAN_CONTENT);
}

ایستا شمارش mcheck_status mcheck_mprobe(تهی *ptr)
{
	ساختار mcheck_hdr *hdr = &((ساختار mcheck_hdr *)ptr)[-1];

	بازگشت mcheck_checkhdr(hdr);
}

ایستا تهی mcheck_pedantic_check(تهی)
{
	صحیح i;

	برای (i = 0; i < REGISTRY_SZ; ++i)
		اگر (mcheck_registry[i])
			mcheck_checkhdr(mcheck_registry[i]);
}

ایستا تهی mcheck_pedantic_prehook(تهی)
{
	اگر (mcheck_pedantic_flag)
		mcheck_pedantic_check();
}

ایستا تهی mcheck_initialize(mcheck_abortfunc_t new_func, نویسه pedantic_flag)
{
	mcheck_abortfunc = (new_func) ? new_func : &mcheck_default_abort;
	mcheck_pedantic_flag = pedantic_flag;
}

تهی mcheck_on_ramrelocation(size_t offset)
{
	نویسه *p;
	صحیح i;
	// Simple, but inaccurate strategy: drop the pre-reloc heap
	برای (i = 0; i < REGISTRY_SZ; ++i)
		اگر ((p = mcheck_registry[i]) != پوچ ) {
			printf("mcheck, WRN: forgetting %p chunk\n", p);
			mcheck_registry[i] = 0;
		}

	mcheck_chunk_شمار = 0;
}
#endif
#endif
