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
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ _MCHECKCORE_INC_H
#ವ್ಯಾಖ್ಯಾನಿಸು _MCHECKCORE_INC_H      1
#ಅನ್ತರ್ಭಾವ "mcheck.h"

#ಆದರೆ defined(MCHECK_ರಾಶಿ_PROTECTION)
#ವ್ಯಾಖ್ಯಾನಿಸು mcheck_flood memset

// these are from /dev/random:
#ವ್ಯಾಖ್ಯಾನಿಸು MAGICWORD	0x99ccf430fa562a05ULL
#ವ್ಯಾಖ್ಯಾನಿಸು MAGICFREE	0x4875e63c0c6fc08eULL
#ವ್ಯಾಖ್ಯಾನಿಸು MAGICTAIL	0x918dbcd7df78dcd6ULL
#ವ್ಯಾಖ್ಯಾನಿಸು MALLOCFLOOD	((ಅಕ್ಷರ)0xb6)
#ವ್ಯಾಖ್ಯಾನಿಸು FREEFLOOD	((ಅಕ್ಷರ)0xf5)
#ವ್ಯಾಖ್ಯಾನಿಸು PADDINGFLOOD	((ಅಕ್ಷರ)0x58)

// my normal run demands 4427-6449 chunks:
#ವ್ಯಾಖ್ಯಾನಿಸು REGISTRY_SZ	6608
#ವ್ಯಾಖ್ಯಾನಿಸು CANARY_ಆಳ	2

// avoid problems with BSS at early stage:
ಸ್ಥಿರ ಅಕ್ಷರ mcheck_pedantic_flag __section(".data") = 0;
ಸ್ಥಿರ ಶೂನ್ಯ *mcheck_registry[REGISTRY_SZ] __section(".data") = {0};
ಸ್ಥಿರ size_t mcheck_chunk_ಎಣಿಕೆ __section(".data") = 0;
ಸ್ಥಿರ size_t mcheck_chunk_ಎಣಿಕೆ_max __section(".data") = 0;

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ mcheck_elem;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ {
	mcheck_elem elems[CANARY_ಆಳ];
} mcheck_canary;
ರಚನೆ mcheck_hdr {
	size_t ಗಾತ್ರ_2; /* Exact size requested by user.  */
	size_t aln_skip; /* Ignored bytes, before the mcheck_hdr, to fulfill alignment */
	mcheck_canary canary; /* Magic number to check header integrity.  */
};

ಸ್ಥಿರ ಶೂನ್ಯ mcheck_default_abort(ಎಣಿಕೆ mcheck_status ಸ್ಥಿತಿ_2, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *p)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ;

	ಆಯ್ಕೆ (ಸ್ಥಿತಿ_2) {
	ಸ್ಥಿತಿ MCHECK_OK:
		ಸಂದೇಶ = "memory is consistent, library is buggy\n";
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ MCHECK_HEAD:
		ಸಂದೇಶ = "memory clobbered before allocated block\n";
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ MCHECK_TAIL:
		ಸಂದೇಶ = "memory clobbered past end of allocated block\n";
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ MCHECK_FREE:
		ಸಂದೇಶ = "block freed twice\n";
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ಸಂದೇಶ = "bogus mcheck_status, library is buggy\n";
		ನಿಲ್ಲಿಸು;
	}
	ಬರೆ("\n\nmcheck: %p:%s!!! [%zu]\n\n", p, ಸಂದೇಶ, mcheck_chunk_ಎಣಿಕೆ_max);
}

ಸ್ಥಿರ mcheck_abortfunc_t mcheck_abortfunc = &mcheck_default_abort;

ಸ್ಥಿರ ಸಾಲೊಳಗೆ size_t allign_ಗಾತ್ರ_ಮೇಲೆ(size_t sz, size_t grain)
{
	ಹಿಂತಿರುಗಿ (sz + grain - 1) & ~(grain - 1);
}

#ವ್ಯಾಖ್ಯಾನಿಸು mcheck_allign_customer_ಗಾತ್ರ(SZ) allign_ಗಾತ್ರ_ಮೇಲೆ(SZ, ಗಾತ್ರ(mcheck_elem))
#ವ್ಯಾಖ್ಯಾನಿಸು mcheck_evaluate_memalign_ಪೂರ್ವಪ್ರತ್ಯಯ_ಗಾತ್ರ(ALIGN) allign_ಗಾತ್ರ_ಮೇಲೆ(ಗಾತ್ರ(ರಚನೆ mcheck_hdr), ALIGN)

ಸ್ಥಿರ ಎಣಿಕೆ mcheck_status mcheck_OnNok(ಎಣಿಕೆ mcheck_status ಸ್ಥಿತಿ_2, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *p)
{
	(*mcheck_abortfunc)(ಸ್ಥಿತಿ_2, p);
	ಹಿಂತಿರುಗಿ ಸ್ಥಿತಿ_2;
}

ಸ್ಥಿರ ಎಣಿಕೆ mcheck_status mcheck_checkhdr(ಸ್ಥಿರಾಂಕ ರಚನೆ mcheck_hdr *hdr)
{
	ಪೂರ್ಣಾಂಕ i;

	ಗಾಗಿ (i = 0; i < CANARY_ಆಳ; ++i)
		ಆದರೆ (hdr->canary.elems[i] == MAGICFREE)
			ಹಿಂತಿರುಗಿ mcheck_OnNok(MCHECK_FREE, hdr + 1);

	ಗಾಗಿ (i = 0; i < CANARY_ಆಳ; ++i)
		ಆದರೆ (hdr->canary.elems[i] != MAGICWORD)
			ಹಿಂತಿರುಗಿ mcheck_OnNok(MCHECK_HEAD, hdr + 1);

	ಸ್ಥಿರಾಂಕ size_t payload_ಗಾತ್ರ = hdr->ಗಾತ್ರ_2;
	ಸ್ಥಿರಾಂಕ size_t payload_ಗಾತ್ರ_aligned = mcheck_allign_customer_ಗಾತ್ರ(payload_ಗಾತ್ರ);
	ಸ್ಥಿರಾಂಕ size_t padd_ಗಾತ್ರ = payload_ಗಾತ್ರ_aligned - hdr->ಗಾತ್ರ_2;

	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *payload = (ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *)&hdr[1];

	ಗಾಗಿ (i = 0; i < padd_ಗಾತ್ರ; ++i)
		ಆದರೆ (payload[payload_ಗಾತ್ರ + i] != PADDINGFLOOD)
			ಹಿಂತಿರುಗಿ mcheck_OnNok(MCHECK_TAIL, hdr + 1);

	ಸ್ಥಿರಾಂಕ mcheck_canary *ಬಾಲ = (ಸ್ಥಿರಾಂಕ mcheck_canary *)&payload[payload_ಗಾತ್ರ_aligned];

	ಗಾಗಿ (i = 0; i < CANARY_ಆಳ; ++i)
		ಆದರೆ (ಬಾಲ->elems[i] != MAGICTAIL)
			ಹಿಂತಿರುಗಿ mcheck_OnNok(MCHECK_TAIL, hdr + 1);
	ಹಿಂತಿರುಗಿ MCHECK_OK;
}

ಎಣಿಕೆ { KEEP_CONTENT = 0, CLEAN_CONTENT, ANY_ALIGNMENT = 1 };
ಸ್ಥಿರ ಶೂನ್ಯ *mcheck_free_helper(ಶೂನ್ಯ *ptr, ಪೂರ್ಣಾಂಕ clean_content)
{
	ಆದರೆ (!ptr)
		ಹಿಂತಿರುಗಿ ptr;

	ರಚನೆ mcheck_hdr *hdr = &((ರಚನೆ mcheck_hdr *)ptr)[-1];
	ಪೂರ್ಣಾಂಕ i;

	mcheck_checkhdr(hdr);
	ಗಾಗಿ (i = 0; i < CANARY_ಆಳ; ++i)
		hdr->canary.elems[i] = MAGICFREE;

	ಆದರೆ (clean_content)
		mcheck_flood(ptr, FREEFLOOD, mcheck_allign_customer_ಗಾತ್ರ(hdr->ಗಾತ್ರ_2));

	ಗಾಗಿ (i = 0; i < REGISTRY_SZ; ++i)
		ಆದರೆ (mcheck_registry[i] == hdr) {
			mcheck_registry[i] = 0;
			ನಿಲ್ಲಿಸು;
		}

	--mcheck_chunk_ಎಣಿಕೆ;
	ಹಿಂತಿರುಗಿ (ಅಕ್ಷರ *)hdr - hdr->aln_skip;
}

ಸ್ಥಿರ ಶೂನ್ಯ *mcheck_free_prehook(ಶೂನ್ಯ *ptr) { ಹಿಂತಿರುಗಿ mcheck_free_helper(ptr, CLEAN_CONTENT); }
ಸ್ಥಿರ ಶೂನ್ಯ *mcheck_reallocfree_prehook(ಶೂನ್ಯ *ptr) { ಹಿಂತಿರುಗಿ mcheck_free_helper(ptr, KEEP_CONTENT); }

ಸ್ಥಿರ size_t mcheck_alloc_prehook(size_t sz)
{
	sz = mcheck_allign_customer_ಗಾತ್ರ(sz);
	ಹಿಂತಿರುಗಿ ಗಾತ್ರ(ರಚನೆ mcheck_hdr) + sz + ಗಾತ್ರ(mcheck_canary);
}

ಸ್ಥಿರ ಶೂನ್ಯ *mcheck_allocated_helper(ಶೂನ್ಯ *altoghether_ptr, size_t customer_sz,
				     size_t alignment, ಪೂರ್ಣಾಂಕ clean_content)
{
	ಸ್ಥಿರಾಂಕ size_t slop = alignment ?
		mcheck_evaluate_memalign_ಪೂರ್ವಪ್ರತ್ಯಯ_ಗಾತ್ರ(alignment) - ಗಾತ್ರ(ರಚನೆ mcheck_hdr) : 0;
	ರಚನೆ mcheck_hdr *hdr = (ರಚನೆ mcheck_hdr *)((ಅಕ್ಷರ *)altoghether_ptr + slop);
	ಪೂರ್ಣಾಂಕ i;

	hdr->ಗಾತ್ರ_2 = customer_sz;
	hdr->aln_skip = slop;
	ಗಾಗಿ (i = 0; i < CANARY_ಆಳ; ++i)
		hdr->canary.elems[i] = MAGICWORD;

	ಅಕ್ಷರ *payload = (ಅಕ್ಷರ *)&hdr[1];

	ಆದರೆ (clean_content)
		mcheck_flood(payload, MALLOCFLOOD, customer_sz);

	ಸ್ಥಿರಾಂಕ size_t customer_ಗಾತ್ರ_aligned = mcheck_allign_customer_ಗಾತ್ರ(customer_sz);

	mcheck_flood(payload + customer_sz, PADDINGFLOOD, customer_ಗಾತ್ರ_aligned - customer_sz);

	mcheck_canary *ಬಾಲ = (mcheck_canary *)&payload[customer_ಗಾತ್ರ_aligned];

	ಗಾಗಿ (i = 0; i < CANARY_ಆಳ; ++i)
		ಬಾಲ->elems[i] = MAGICTAIL;

	++mcheck_chunk_ಎಣಿಕೆ;
	ಆದರೆ (mcheck_chunk_ಎಣಿಕೆ > mcheck_chunk_ಎಣಿಕೆ_max)
		mcheck_chunk_ಎಣಿಕೆ_max = mcheck_chunk_ಎಣಿಕೆ;

	ಗಾಗಿ (i = 0; i < REGISTRY_SZ; ++i)
		ಆದರೆ (!mcheck_registry[i]) {
			mcheck_registry[i] = hdr;
			ಹಿಂತಿರುಗಿ payload; // normal end
		}

	ಸ್ಥಿರ ಅಕ್ಷರ *ಅತಿಪ್ರವಾಹ_ಸಂದೇಶ = "\n\n\nERROR: mcheck registry overflow, pedantic check would be incomplete!!\n\n\n\n";

	ಬರೆ("%s", ಅತಿಪ್ರವಾಹ_ಸಂದೇಶ);
	ಅತಿಪ್ರವಾಹ_ಸಂದೇಶ = "(mcheck registry full)";
	ಹಿಂತಿರುಗಿ payload;
}

ಸ್ಥಿರ ಶೂನ್ಯ *mcheck_alloc_posthook(ಶೂನ್ಯ *altoghether_ptr, size_t customer_sz)
{
	ಹಿಂತಿರುಗಿ mcheck_allocated_helper(altoghether_ptr, customer_sz, ANY_ALIGNMENT, CLEAN_CONTENT);
}

ಸ್ಥಿರ ಶೂನ್ಯ *mcheck_alloc_noclean_posthook(ಶೂನ್ಯ *altoghether_ptr, size_t customer_sz)
{
	ಹಿಂತಿರುಗಿ mcheck_allocated_helper(altoghether_ptr, customer_sz, ANY_ALIGNMENT, KEEP_CONTENT);
}

ಸ್ಥಿರ size_t mcheck_memalign_prehook(size_t alig, size_t sz)
{
	ಹಿಂತಿರುಗಿ mcheck_evaluate_memalign_ಪೂರ್ವಪ್ರತ್ಯಯ_ಗಾತ್ರ(alig) + sz + ಗಾತ್ರ(mcheck_canary);
}

ಸ್ಥಿರ ಶೂನ್ಯ *mcheck_memalign_posthook(size_t alignment, ಶೂನ್ಯ *altoghether_ptr, size_t customer_sz)
{
	ಹಿಂತಿರುಗಿ mcheck_allocated_helper(altoghether_ptr, customer_sz, alignment, CLEAN_CONTENT);
}

ಸ್ಥಿರ ಎಣಿಕೆ mcheck_status mcheck_mprobe(ಶೂನ್ಯ *ptr)
{
	ರಚನೆ mcheck_hdr *hdr = &((ರಚನೆ mcheck_hdr *)ptr)[-1];

	ಹಿಂತಿರುಗಿ mcheck_checkhdr(hdr);
}

ಸ್ಥಿರ ಶೂನ್ಯ mcheck_pedantic_check(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಂಕ i;

	ಗಾಗಿ (i = 0; i < REGISTRY_SZ; ++i)
		ಆದರೆ (mcheck_registry[i])
			mcheck_checkhdr(mcheck_registry[i]);
}

ಸ್ಥಿರ ಶೂನ್ಯ mcheck_pedantic_prehook(ಶೂನ್ಯ)
{
	ಆದರೆ (mcheck_pedantic_flag)
		mcheck_pedantic_check();
}

ಸ್ಥಿರ ಶೂನ್ಯ mcheck_initialize(mcheck_abortfunc_t new_func, ಅಕ್ಷರ pedantic_flag)
{
	mcheck_abortfunc = (new_func) ? new_func : &mcheck_default_abort;
	mcheck_pedantic_flag = pedantic_flag;
}

ಶೂನ್ಯ mcheck_on_ramrelocation(size_t ಸ್ಥಾನಾಂತರ)
{
	ಅಕ್ಷರ *p;
	ಪೂರ್ಣಾಂಕ i;
	// Simple, but inaccurate strategy: drop the pre-reloc heap
	ಗಾಗಿ (i = 0; i < REGISTRY_SZ; ++i)
		ಆದರೆ ((p = mcheck_registry[i]) != ಖಾಲಿ ) {
			ಬರೆ("mcheck, WRN: forgetting %p chunk\n", p);
			mcheck_registry[i] = 0;
		}

	mcheck_chunk_ಎಣಿಕೆ = 0;
}
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
