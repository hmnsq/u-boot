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
#அந்தர்பாவ "mcheck.h"

#எனில் defined(MCHECK_HEAP_PROTECTION)
#define mcheck_flood memset

// these are from /dev/random:
#define MAGICWORD	0x99ccf430fa562a05ULL
#define MAGICFREE	0x4875e63c0c6fc08eULL
#define MAGICTAIL	0x918dbcd7df78dcd6ULL
#define MALLOCFLOOD	((எழுத்து)0xb6)
#define FREEFLOOD	((எழுத்து)0xf5)
#define PADDINGFLOOD	((எழுத்து)0x58)

// my normal run demands 4427-6449 chunks:
#define REGISTRY_SZ	6608
#define CANARY_DEPTH	2

// avoid problems with BSS at early stage:
நிலையான எழுத்து mcheck_pedantic_flag __section(".data") = 0;
நிலையான வெறுமை *mcheck_registry[REGISTRY_SZ] __section(".data") = {0};
நிலையான size_t mcheck_chunk_மொத்தம் __section(".data") = 0;
நிலையான size_t mcheck_chunk_மொத்தம்_max __section(".data") = 0;

வகைவரை unsigned long long mcheck_elem;
வகைவரை கட்டமைப்பு {
	mcheck_elem elems[CANARY_DEPTH];
} mcheck_canary;
கட்டமைப்பு mcheck_hdr {
	size_t size; /* Exact size requested by user.  */
	size_t aln_skip; /* Ignored bytes, before the mcheck_hdr, to fulfill alignment */
	mcheck_canary canary; /* Magic number to check header integrity.  */
};

நிலையான வெறுமை mcheck_default_abort(எண்ணிக்கை mcheck_status status, மாறிலி வெறுமை *p)
{
	மாறிலி எழுத்து *செய்தி;

	தேர்வு (status) {
	நிலை MCHECK_OK:
		செய்தி = "memory is consistent, library is buggy\n";
		நிறுத்து;
	நிலை MCHECK_HEAD:
		செய்தி = "memory clobbered before allocated block\n";
		நிறுத்து;
	நிலை MCHECK_TAIL:
		செய்தி = "memory clobbered past end of allocated block\n";
		நிறுத்து;
	நிலை MCHECK_FREE:
		செய்தி = "block freed twice\n";
		நிறுத்து;
	இயல்பு:
		செய்தி = "bogus mcheck_status, library is buggy\n";
		நிறுத்து;
	}
	printf("\n\nmcheck: %p:%s!!! [%zu]\n\n", p, செய்தி, mcheck_chunk_மொத்தம்_max);
}

நிலையான mcheck_abortfunc_t mcheck_abortfunc = &mcheck_default_abort;

நிலையான உள்வரி size_t allign_size_up(size_t sz, size_t grain)
{
	திரும்பு (sz + grain - 1) & ~(grain - 1);
}

#define mcheck_allign_customer_size(SZ) allign_size_up(SZ, அளவு(mcheck_elem))
#define mcheck_evaluate_memalign_prefix_size(ALIGN) allign_size_up(அளவு(கட்டமைப்பு mcheck_hdr), ALIGN)

நிலையான எண்ணிக்கை mcheck_status mcheck_OnNok(எண்ணிக்கை mcheck_status status, மாறிலி வெறுமை *p)
{
	(*mcheck_abortfunc)(status, p);
	திரும்பு status;
}

நிலையான எண்ணிக்கை mcheck_status mcheck_checkhdr(மாறிலி கட்டமைப்பு mcheck_hdr *hdr)
{
	முழு i;

	க்கு (i = 0; i < CANARY_DEPTH; ++i)
		எனில் (hdr->canary.elems[i] == MAGICFREE)
			திரும்பு mcheck_OnNok(MCHECK_FREE, hdr + 1);

	க்கு (i = 0; i < CANARY_DEPTH; ++i)
		எனில் (hdr->canary.elems[i] != MAGICWORD)
			திரும்பு mcheck_OnNok(MCHECK_HEAD, hdr + 1);

	மாறிலி size_t payload_size = hdr->size;
	மாறிலி size_t payload_size_aligned = mcheck_allign_customer_size(payload_size);
	மாறிலி size_t padd_size = payload_size_aligned - hdr->size;

	மாறிலி எழுத்து *payload = (மாறிலி எழுத்து *)&hdr[1];

	க்கு (i = 0; i < padd_size; ++i)
		எனில் (payload[payload_size + i] != PADDINGFLOOD)
			திரும்பு mcheck_OnNok(MCHECK_TAIL, hdr + 1);

	மாறிலி mcheck_canary *tail = (மாறிலி mcheck_canary *)&payload[payload_size_aligned];

	க்கு (i = 0; i < CANARY_DEPTH; ++i)
		எனில் (tail->elems[i] != MAGICTAIL)
			திரும்பு mcheck_OnNok(MCHECK_TAIL, hdr + 1);
	திரும்பு MCHECK_OK;
}

எண்ணிக்கை { KEEP_CONTENT = 0, CLEAN_CONTENT, ANY_ALIGNMENT = 1 };
நிலையான வெறுமை *mcheck_free_helper(வெறுமை *ptr, முழு clean_content)
{
	எனில் (!ptr)
		திரும்பு ptr;

	கட்டமைப்பு mcheck_hdr *hdr = &((கட்டமைப்பு mcheck_hdr *)ptr)[-1];
	முழு i;

	mcheck_checkhdr(hdr);
	க்கு (i = 0; i < CANARY_DEPTH; ++i)
		hdr->canary.elems[i] = MAGICFREE;

	எனில் (clean_content)
		mcheck_flood(ptr, FREEFLOOD, mcheck_allign_customer_size(hdr->size));

	க்கு (i = 0; i < REGISTRY_SZ; ++i)
		எனில் (mcheck_registry[i] == hdr) {
			mcheck_registry[i] = 0;
			நிறுத்து;
		}

	--mcheck_chunk_மொத்தம்;
	திரும்பு (எழுத்து *)hdr - hdr->aln_skip;
}

நிலையான வெறுமை *mcheck_free_prehook(வெறுமை *ptr) { திரும்பு mcheck_free_helper(ptr, CLEAN_CONTENT); }
நிலையான வெறுமை *mcheck_reallocfree_prehook(வெறுமை *ptr) { திரும்பு mcheck_free_helper(ptr, KEEP_CONTENT); }

நிலையான size_t mcheck_alloc_prehook(size_t sz)
{
	sz = mcheck_allign_customer_size(sz);
	திரும்பு அளவு(கட்டமைப்பு mcheck_hdr) + sz + அளவு(mcheck_canary);
}

நிலையான வெறுமை *mcheck_allocated_helper(வெறுமை *altoghether_ptr, size_t customer_sz,
				     size_t alignment, முழு clean_content)
{
	மாறிலி size_t slop = alignment ?
		mcheck_evaluate_memalign_prefix_size(alignment) - அளவு(கட்டமைப்பு mcheck_hdr) : 0;
	கட்டமைப்பு mcheck_hdr *hdr = (கட்டமைப்பு mcheck_hdr *)((எழுத்து *)altoghether_ptr + slop);
	முழு i;

	hdr->size = customer_sz;
	hdr->aln_skip = slop;
	க்கு (i = 0; i < CANARY_DEPTH; ++i)
		hdr->canary.elems[i] = MAGICWORD;

	எழுத்து *payload = (எழுத்து *)&hdr[1];

	எனில் (clean_content)
		mcheck_flood(payload, MALLOCFLOOD, customer_sz);

	மாறிலி size_t customer_size_aligned = mcheck_allign_customer_size(customer_sz);

	mcheck_flood(payload + customer_sz, PADDINGFLOOD, customer_size_aligned - customer_sz);

	mcheck_canary *tail = (mcheck_canary *)&payload[customer_size_aligned];

	க்கு (i = 0; i < CANARY_DEPTH; ++i)
		tail->elems[i] = MAGICTAIL;

	++mcheck_chunk_மொத்தம்;
	எனில் (mcheck_chunk_மொத்தம் > mcheck_chunk_மொத்தம்_max)
		mcheck_chunk_மொத்தம்_max = mcheck_chunk_மொத்தம்;

	க்கு (i = 0; i < REGISTRY_SZ; ++i)
		எனில் (!mcheck_registry[i]) {
			mcheck_registry[i] = hdr;
			திரும்பு payload; // normal end
		}

	நிலையான எழுத்து *overflow_செய்தி = "\n\n\nERROR: mcheck registry overflow, pedantic check would be incomplete!!\n\n\n\n";

	printf("%s", overflow_செய்தி);
	overflow_செய்தி = "(mcheck registry full)";
	திரும்பு payload;
}

நிலையான வெறுமை *mcheck_alloc_posthook(வெறுமை *altoghether_ptr, size_t customer_sz)
{
	திரும்பு mcheck_allocated_helper(altoghether_ptr, customer_sz, ANY_ALIGNMENT, CLEAN_CONTENT);
}

நிலையான வெறுமை *mcheck_alloc_noclean_posthook(வெறுமை *altoghether_ptr, size_t customer_sz)
{
	திரும்பு mcheck_allocated_helper(altoghether_ptr, customer_sz, ANY_ALIGNMENT, KEEP_CONTENT);
}

நிலையான size_t mcheck_memalign_prehook(size_t alig, size_t sz)
{
	திரும்பு mcheck_evaluate_memalign_prefix_size(alig) + sz + அளவு(mcheck_canary);
}

நிலையான வெறுமை *mcheck_memalign_posthook(size_t alignment, வெறுமை *altoghether_ptr, size_t customer_sz)
{
	திரும்பு mcheck_allocated_helper(altoghether_ptr, customer_sz, alignment, CLEAN_CONTENT);
}

நிலையான எண்ணிக்கை mcheck_status mcheck_mprobe(வெறுமை *ptr)
{
	கட்டமைப்பு mcheck_hdr *hdr = &((கட்டமைப்பு mcheck_hdr *)ptr)[-1];

	திரும்பு mcheck_checkhdr(hdr);
}

நிலையான வெறுமை mcheck_pedantic_check(வெறுமை)
{
	முழு i;

	க்கு (i = 0; i < REGISTRY_SZ; ++i)
		எனில் (mcheck_registry[i])
			mcheck_checkhdr(mcheck_registry[i]);
}

நிலையான வெறுமை mcheck_pedantic_prehook(வெறுமை)
{
	எனில் (mcheck_pedantic_flag)
		mcheck_pedantic_check();
}

நிலையான வெறுமை mcheck_initialize(mcheck_abortfunc_t new_func, எழுத்து pedantic_flag)
{
	mcheck_abortfunc = (new_func) ? new_func : &mcheck_default_abort;
	mcheck_pedantic_flag = pedantic_flag;
}

வெறுமை mcheck_on_ramrelocation(size_t offset)
{
	எழுத்து *p;
	முழு i;
	// Simple, but inaccurate strategy: drop the pre-reloc heap
	க்கு (i = 0; i < REGISTRY_SZ; ++i)
		எனில் ((p = mcheck_registry[i]) != வெற்று ) {
			printf("mcheck, WRN: forgetting %p chunk\n", p);
			mcheck_registry[i] = 0;
		}

	mcheck_chunk_மொத்தம் = 0;
}
#endif
#endif
