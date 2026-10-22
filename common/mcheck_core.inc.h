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
#ಅನ್ತರ್ಭಾವ "mcheck.h"

#ಯದಿ defined(MCHECK_HEAP_PROTECTION)
#define mcheck_flood memset

// these are from /dev/random:
#define MAGICWORD	0x99ccf430fa562a05ULL
#define MAGICFREE	0x4875e63c0c6fc08eULL
#define MAGICTAIL	0x918dbcd7df78dcd6ULL
#define MALLOCFLOOD	((ವರ್ಣ)0xb6)
#define FREEFLOOD	((ವರ್ಣ)0xf5)
#define PADDINGFLOOD	((ವರ್ಣ)0x58)

// my normal run demands 4427-6449 chunks:
#define REGISTRY_SZ	6608
#define CANARY_DEPTH	2

// avoid problems with BSS at early stage:
ಸ್ಥಿರ ವರ್ಣ mcheck_pedantic_flag __section(".data") = 0;
ಸ್ಥಿರ ಶೂನ್ಯ *mcheck_registry[REGISTRY_SZ] __section(".data") = {0};
ಸ್ಥಿರ size_t mcheck_chunk_ಎಣಿಕೆ __section(".data") = 0;
ಸ್ಥಿರ size_t mcheck_chunk_ಎಣಿಕೆ_max __section(".data") = 0;

ಪ್ರಕಾರನಿರ್ದೇಶ unsigned long long mcheck_elem;
ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ {
	mcheck_elem elems[CANARY_DEPTH];
} mcheck_canary;
ಸಂರಚನಾ mcheck_hdr {
	size_t size; /* Exact size requested by user.  */
	size_t aln_skip; /* Ignored bytes, before the mcheck_hdr, to fulfill alignment */
	mcheck_canary canary; /* Magic number to check header integrity.  */
};

ಸ್ಥಿರ ಶೂನ್ಯ mcheck_default_abort(ಗಣನಾ mcheck_status status, ಧ್ರುವ ಶೂನ್ಯ *p)
{
	ಧ್ರುವ ವರ್ಣ *ಸಂದೇಶ;

	ವಿಕಲ್ಪ (status) {
	ಪಕ್ಷ MCHECK_OK:
		ಸಂದೇಶ = "memory is consistent, library is buggy\n";
		ಭಙ್ಗ;
	ಪಕ್ಷ MCHECK_HEAD:
		ಸಂದೇಶ = "memory clobbered before allocated block\n";
		ಭಙ್ಗ;
	ಪಕ್ಷ MCHECK_TAIL:
		ಸಂದೇಶ = "memory clobbered past end of allocated block\n";
		ಭಙ್ಗ;
	ಪಕ್ಷ MCHECK_FREE:
		ಸಂದೇಶ = "block freed twice\n";
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		ಸಂದೇಶ = "bogus mcheck_status, library is buggy\n";
		ಭಙ್ಗ;
	}
	printf("\n\nmcheck: %p:%s!!! [%zu]\n\n", p, ಸಂದೇಶ, mcheck_chunk_ಎಣಿಕೆ_max);
}

ಸ್ಥಿರ mcheck_abortfunc_t mcheck_abortfunc = &mcheck_default_abort;

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ size_t allign_size_up(size_t sz, size_t grain)
{
	ಪ್ರತ್ಯಾಗಮ (sz + grain - 1) & ~(grain - 1);
}

#define mcheck_allign_customer_size(SZ) allign_size_up(SZ, ಪರಿಮಾಣ(mcheck_elem))
#define mcheck_evaluate_memalign_prefix_size(ALIGN) allign_size_up(ಪರಿಮಾಣ(ಸಂರಚನಾ mcheck_hdr), ALIGN)

ಸ್ಥಿರ ಗಣನಾ mcheck_status mcheck_OnNok(ಗಣನಾ mcheck_status status, ಧ್ರುವ ಶೂನ್ಯ *p)
{
	(*mcheck_abortfunc)(status, p);
	ಪ್ರತ್ಯಾಗಮ status;
}

ಸ್ಥಿರ ಗಣನಾ mcheck_status mcheck_checkhdr(ಧ್ರುವ ಸಂರಚನಾ mcheck_hdr *hdr)
{
	ಪೂರ್ಣಾಙ್ಕ i;

	ಪ್ರತಿ (i = 0; i < CANARY_DEPTH; ++i)
		ಯದಿ (hdr->canary.elems[i] == MAGICFREE)
			ಪ್ರತ್ಯಾಗಮ mcheck_OnNok(MCHECK_FREE, hdr + 1);

	ಪ್ರತಿ (i = 0; i < CANARY_DEPTH; ++i)
		ಯದಿ (hdr->canary.elems[i] != MAGICWORD)
			ಪ್ರತ್ಯಾಗಮ mcheck_OnNok(MCHECK_HEAD, hdr + 1);

	ಧ್ರುವ size_t payload_size = hdr->size;
	ಧ್ರುವ size_t payload_size_aligned = mcheck_allign_customer_size(payload_size);
	ಧ್ರುವ size_t padd_size = payload_size_aligned - hdr->size;

	ಧ್ರುವ ವರ್ಣ *payload = (ಧ್ರುವ ವರ್ಣ *)&hdr[1];

	ಪ್ರತಿ (i = 0; i < padd_size; ++i)
		ಯದಿ (payload[payload_size + i] != PADDINGFLOOD)
			ಪ್ರತ್ಯಾಗಮ mcheck_OnNok(MCHECK_TAIL, hdr + 1);

	ಧ್ರುವ mcheck_canary *tail = (ಧ್ರುವ mcheck_canary *)&payload[payload_size_aligned];

	ಪ್ರತಿ (i = 0; i < CANARY_DEPTH; ++i)
		ಯದಿ (tail->elems[i] != MAGICTAIL)
			ಪ್ರತ್ಯಾಗಮ mcheck_OnNok(MCHECK_TAIL, hdr + 1);
	ಪ್ರತ್ಯಾಗಮ MCHECK_OK;
}

ಗಣನಾ { KEEP_CONTENT = 0, CLEAN_CONTENT, ANY_ALIGNMENT = 1 };
ಸ್ಥಿರ ಶೂನ್ಯ *mcheck_free_helper(ಶೂನ್ಯ *ptr, ಪೂರ್ಣಾಙ್ಕ clean_content)
{
	ಯದಿ (!ptr)
		ಪ್ರತ್ಯಾಗಮ ptr;

	ಸಂರಚನಾ mcheck_hdr *hdr = &((ಸಂರಚನಾ mcheck_hdr *)ptr)[-1];
	ಪೂರ್ಣಾಙ್ಕ i;

	mcheck_checkhdr(hdr);
	ಪ್ರತಿ (i = 0; i < CANARY_DEPTH; ++i)
		hdr->canary.elems[i] = MAGICFREE;

	ಯದಿ (clean_content)
		mcheck_flood(ptr, FREEFLOOD, mcheck_allign_customer_size(hdr->size));

	ಪ್ರತಿ (i = 0; i < REGISTRY_SZ; ++i)
		ಯದಿ (mcheck_registry[i] == hdr) {
			mcheck_registry[i] = 0;
			ಭಙ್ಗ;
		}

	--mcheck_chunk_ಎಣಿಕೆ;
	ಪ್ರತ್ಯಾಗಮ (ವರ್ಣ *)hdr - hdr->aln_skip;
}

ಸ್ಥಿರ ಶೂನ್ಯ *mcheck_free_prehook(ಶೂನ್ಯ *ptr) { ಪ್ರತ್ಯಾಗಮ mcheck_free_helper(ptr, CLEAN_CONTENT); }
ಸ್ಥಿರ ಶೂನ್ಯ *mcheck_reallocfree_prehook(ಶೂನ್ಯ *ptr) { ಪ್ರತ್ಯಾಗಮ mcheck_free_helper(ptr, KEEP_CONTENT); }

ಸ್ಥಿರ size_t mcheck_alloc_prehook(size_t sz)
{
	sz = mcheck_allign_customer_size(sz);
	ಪ್ರತ್ಯಾಗಮ ಪರಿಮಾಣ(ಸಂರಚನಾ mcheck_hdr) + sz + ಪರಿಮಾಣ(mcheck_canary);
}

ಸ್ಥಿರ ಶೂನ್ಯ *mcheck_allocated_helper(ಶೂನ್ಯ *altoghether_ptr, size_t customer_sz,
				     size_t alignment, ಪೂರ್ಣಾಙ್ಕ clean_content)
{
	ಧ್ರುವ size_t slop = alignment ?
		mcheck_evaluate_memalign_prefix_size(alignment) - ಪರಿಮಾಣ(ಸಂರಚನಾ mcheck_hdr) : 0;
	ಸಂರಚನಾ mcheck_hdr *hdr = (ಸಂರಚನಾ mcheck_hdr *)((ವರ್ಣ *)altoghether_ptr + slop);
	ಪೂರ್ಣಾಙ್ಕ i;

	hdr->size = customer_sz;
	hdr->aln_skip = slop;
	ಪ್ರತಿ (i = 0; i < CANARY_DEPTH; ++i)
		hdr->canary.elems[i] = MAGICWORD;

	ವರ್ಣ *payload = (ವರ್ಣ *)&hdr[1];

	ಯದಿ (clean_content)
		mcheck_flood(payload, MALLOCFLOOD, customer_sz);

	ಧ್ರುವ size_t customer_size_aligned = mcheck_allign_customer_size(customer_sz);

	mcheck_flood(payload + customer_sz, PADDINGFLOOD, customer_size_aligned - customer_sz);

	mcheck_canary *tail = (mcheck_canary *)&payload[customer_size_aligned];

	ಪ್ರತಿ (i = 0; i < CANARY_DEPTH; ++i)
		tail->elems[i] = MAGICTAIL;

	++mcheck_chunk_ಎಣಿಕೆ;
	ಯದಿ (mcheck_chunk_ಎಣಿಕೆ > mcheck_chunk_ಎಣಿಕೆ_max)
		mcheck_chunk_ಎಣಿಕೆ_max = mcheck_chunk_ಎಣಿಕೆ;

	ಪ್ರತಿ (i = 0; i < REGISTRY_SZ; ++i)
		ಯದಿ (!mcheck_registry[i]) {
			mcheck_registry[i] = hdr;
			ಪ್ರತ್ಯಾಗಮ payload; // normal end
		}

	ಸ್ಥಿರ ವರ್ಣ *overflow_ಸಂದೇಶ = "\n\n\nERROR: mcheck registry overflow, pedantic check would be incomplete!!\n\n\n\n";

	printf("%s", overflow_ಸಂದೇಶ);
	overflow_ಸಂದೇಶ = "(mcheck registry full)";
	ಪ್ರತ್ಯಾಗಮ payload;
}

ಸ್ಥಿರ ಶೂನ್ಯ *mcheck_alloc_posthook(ಶೂನ್ಯ *altoghether_ptr, size_t customer_sz)
{
	ಪ್ರತ್ಯಾಗಮ mcheck_allocated_helper(altoghether_ptr, customer_sz, ANY_ALIGNMENT, CLEAN_CONTENT);
}

ಸ್ಥಿರ ಶೂನ್ಯ *mcheck_alloc_noclean_posthook(ಶೂನ್ಯ *altoghether_ptr, size_t customer_sz)
{
	ಪ್ರತ್ಯಾಗಮ mcheck_allocated_helper(altoghether_ptr, customer_sz, ANY_ALIGNMENT, KEEP_CONTENT);
}

ಸ್ಥಿರ size_t mcheck_memalign_prehook(size_t alig, size_t sz)
{
	ಪ್ರತ್ಯಾಗಮ mcheck_evaluate_memalign_prefix_size(alig) + sz + ಪರಿಮಾಣ(mcheck_canary);
}

ಸ್ಥಿರ ಶೂನ್ಯ *mcheck_memalign_posthook(size_t alignment, ಶೂನ್ಯ *altoghether_ptr, size_t customer_sz)
{
	ಪ್ರತ್ಯಾಗಮ mcheck_allocated_helper(altoghether_ptr, customer_sz, alignment, CLEAN_CONTENT);
}

ಸ್ಥಿರ ಗಣನಾ mcheck_status mcheck_mprobe(ಶೂನ್ಯ *ptr)
{
	ಸಂರಚನಾ mcheck_hdr *hdr = &((ಸಂರಚನಾ mcheck_hdr *)ptr)[-1];

	ಪ್ರತ್ಯಾಗಮ mcheck_checkhdr(hdr);
}

ಸ್ಥಿರ ಶೂನ್ಯ mcheck_pedantic_check(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಙ್ಕ i;

	ಪ್ರತಿ (i = 0; i < REGISTRY_SZ; ++i)
		ಯದಿ (mcheck_registry[i])
			mcheck_checkhdr(mcheck_registry[i]);
}

ಸ್ಥಿರ ಶೂನ್ಯ mcheck_pedantic_prehook(ಶೂನ್ಯ)
{
	ಯದಿ (mcheck_pedantic_flag)
		mcheck_pedantic_check();
}

ಸ್ಥಿರ ಶೂನ್ಯ mcheck_initialize(mcheck_abortfunc_t new_func, ವರ್ಣ pedantic_flag)
{
	mcheck_abortfunc = (new_func) ? new_func : &mcheck_default_abort;
	mcheck_pedantic_flag = pedantic_flag;
}

ಶೂನ್ಯ mcheck_on_ramrelocation(size_t offset)
{
	ವರ್ಣ *p;
	ಪೂರ್ಣಾಙ್ಕ i;
	// Simple, but inaccurate strategy: drop the pre-reloc heap
	ಪ್ರತಿ (i = 0; i < REGISTRY_SZ; ++i)
		ಯದಿ ((p = mcheck_registry[i]) != ರಿಕ್ತ ) {
			printf("mcheck, WRN: forgetting %p chunk\n", p);
			mcheck_registry[i] = 0;
		}

	mcheck_chunk_ಎಣಿಕೆ = 0;
}
#endif
#endif
