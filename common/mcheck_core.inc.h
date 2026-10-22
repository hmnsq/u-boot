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
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ _MCHECKCORE_INC_H
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ _MCHECKCORE_INC_H      1
#ਅਨ੍ਤਰ੍ਭਾਵ "mcheck.h"

#ਜੇ defined(MCHECK_ਢੇਰ_PROTECTION)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ mcheck_flood memset

// these are from /dev/random:
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAGICWORD	0x99ccf430fa562a05ULL
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAGICFREE	0x4875e63c0c6fc08eULL
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAGICTAIL	0x918dbcd7df78dcd6ULL
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MALLOCFLOOD	((ਅੱਖਰ)0xb6)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FREEFLOOD	((ਅੱਖਰ)0xf5)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PADDINGFLOOD	((ਅੱਖਰ)0x58)

// my normal run demands 4427-6449 chunks:
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ REGISTRY_SZ	6608
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CANARY_ਡੂੰਘਾਈ	2

// avoid problems with BSS at early stage:
ਸਥਿਰ ਅੱਖਰ mcheck_pedantic_flag __section(".data") = 0;
ਸਥਿਰ ਖਾਲੀ *mcheck_registry[REGISTRY_SZ] __section(".data") = {0};
ਸਥਿਰ size_t mcheck_chunk_ਗਿਣਤੀ __section(".data") = 0;
ਸਥਿਰ size_t mcheck_chunk_ਗਿਣਤੀ_max __section(".data") = 0;

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ mcheck_elem;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ {
	mcheck_elem elems[CANARY_ਡੂੰਘਾਈ];
} mcheck_canary;
ਬਣਤਰ mcheck_hdr {
	size_t ਮਾਪ; /* Exact size requested by user.  */
	size_t aln_skip; /* Ignored bytes, before the mcheck_hdr, to fulfill alignment */
	mcheck_canary canary; /* Magic number to check header integrity.  */
};

ਸਥਿਰ ਖਾਲੀ mcheck_default_abort(ਗਿਣਤੀ mcheck_status ਸਥਿਤੀ, ਅਚਲ ਖਾਲੀ *p)
{
	ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ;

	ਚੋਣ (ਸਥਿਤੀ) {
	ਹਾਲਤ MCHECK_OK:
		ਸੁਨੇਹਾ = "memory is consistent, library is buggy\n";
		ਰੋਕੋ;
	ਹਾਲਤ MCHECK_HEAD:
		ਸੁਨੇਹਾ = "memory clobbered before allocated block\n";
		ਰੋਕੋ;
	ਹਾਲਤ MCHECK_TAIL:
		ਸੁਨੇਹਾ = "memory clobbered past end of allocated block\n";
		ਰੋਕੋ;
	ਹਾਲਤ MCHECK_FREE:
		ਸੁਨੇਹਾ = "block freed twice\n";
		ਰੋਕੋ;
	ਮੂਲ:
		ਸੁਨੇਹਾ = "bogus mcheck_status, library is buggy\n";
		ਰੋਕੋ;
	}
	ਲਿਖੋ("\n\nmcheck: %p:%s!!! [%zu]\n\n", p, ਸੁਨੇਹਾ, mcheck_chunk_ਗਿਣਤੀ_max);
}

ਸਥਿਰ mcheck_abortfunc_t mcheck_abortfunc = &mcheck_default_abort;

ਸਥਿਰ ਸਤਰਅੰਦਰ size_t allign_ਮਾਪ_ਉੱਪਰ(size_t sz, size_t grain)
{
	ਵਾਪਸ (sz + grain - 1) & ~(grain - 1);
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ mcheck_allign_customer_ਮਾਪ(SZ) allign_ਮਾਪ_ਉੱਪਰ(SZ, ਆਕਾਰ(mcheck_elem))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ mcheck_evaluate_memalign_ਅਗੇਤਰ_ਮਾਪ(ALIGN) allign_ਮਾਪ_ਉੱਪਰ(ਆਕਾਰ(ਬਣਤਰ mcheck_hdr), ALIGN)

ਸਥਿਰ ਗਿਣਤੀ mcheck_status mcheck_OnNok(ਗਿਣਤੀ mcheck_status ਸਥਿਤੀ, ਅਚਲ ਖਾਲੀ *p)
{
	(*mcheck_abortfunc)(ਸਥਿਤੀ, p);
	ਵਾਪਸ ਸਥਿਤੀ;
}

ਸਥਿਰ ਗਿਣਤੀ mcheck_status mcheck_checkhdr(ਅਚਲ ਬਣਤਰ mcheck_hdr *hdr)
{
	ਪੂਰਨ i;

	ਲਈ (i = 0; i < CANARY_ਡੂੰਘਾਈ; ++i)
		ਜੇ (hdr->canary.elems[i] == MAGICFREE)
			ਵਾਪਸ mcheck_OnNok(MCHECK_FREE, hdr + 1);

	ਲਈ (i = 0; i < CANARY_ਡੂੰਘਾਈ; ++i)
		ਜੇ (hdr->canary.elems[i] != MAGICWORD)
			ਵਾਪਸ mcheck_OnNok(MCHECK_HEAD, hdr + 1);

	ਅਚਲ size_t payload_ਮਾਪ = hdr->ਮਾਪ;
	ਅਚਲ size_t payload_ਮਾਪ_aligned = mcheck_allign_customer_ਮਾਪ(payload_ਮਾਪ);
	ਅਚਲ size_t padd_ਮਾਪ = payload_ਮਾਪ_aligned - hdr->ਮਾਪ;

	ਅਚਲ ਅੱਖਰ *payload = (ਅਚਲ ਅੱਖਰ *)&hdr[1];

	ਲਈ (i = 0; i < padd_ਮਾਪ; ++i)
		ਜੇ (payload[payload_ਮਾਪ + i] != PADDINGFLOOD)
			ਵਾਪਸ mcheck_OnNok(MCHECK_TAIL, hdr + 1);

	ਅਚਲ mcheck_canary *ਪੂਛ = (ਅਚਲ mcheck_canary *)&payload[payload_ਮਾਪ_aligned];

	ਲਈ (i = 0; i < CANARY_ਡੂੰਘਾਈ; ++i)
		ਜੇ (ਪੂਛ->elems[i] != MAGICTAIL)
			ਵਾਪਸ mcheck_OnNok(MCHECK_TAIL, hdr + 1);
	ਵਾਪਸ MCHECK_OK;
}

ਗਿਣਤੀ { KEEP_CONTENT = 0, CLEAN_CONTENT, ANY_ALIGNMENT = 1 };
ਸਥਿਰ ਖਾਲੀ *mcheck_free_helper(ਖਾਲੀ *ptr, ਪੂਰਨ clean_content)
{
	ਜੇ (!ptr)
		ਵਾਪਸ ptr;

	ਬਣਤਰ mcheck_hdr *hdr = &((ਬਣਤਰ mcheck_hdr *)ptr)[-1];
	ਪੂਰਨ i;

	mcheck_checkhdr(hdr);
	ਲਈ (i = 0; i < CANARY_ਡੂੰਘਾਈ; ++i)
		hdr->canary.elems[i] = MAGICFREE;

	ਜੇ (clean_content)
		mcheck_flood(ptr, FREEFLOOD, mcheck_allign_customer_ਮਾਪ(hdr->ਮਾਪ));

	ਲਈ (i = 0; i < REGISTRY_SZ; ++i)
		ਜੇ (mcheck_registry[i] == hdr) {
			mcheck_registry[i] = 0;
			ਰੋਕੋ;
		}

	--mcheck_chunk_ਗਿਣਤੀ;
	ਵਾਪਸ (ਅੱਖਰ *)hdr - hdr->aln_skip;
}

ਸਥਿਰ ਖਾਲੀ *mcheck_free_prehook(ਖਾਲੀ *ptr) { ਵਾਪਸ mcheck_free_helper(ptr, CLEAN_CONTENT); }
ਸਥਿਰ ਖਾਲੀ *mcheck_reallocfree_prehook(ਖਾਲੀ *ptr) { ਵਾਪਸ mcheck_free_helper(ptr, KEEP_CONTENT); }

ਸਥਿਰ size_t mcheck_alloc_prehook(size_t sz)
{
	sz = mcheck_allign_customer_ਮਾਪ(sz);
	ਵਾਪਸ ਆਕਾਰ(ਬਣਤਰ mcheck_hdr) + sz + ਆਕਾਰ(mcheck_canary);
}

ਸਥਿਰ ਖਾਲੀ *mcheck_allocated_helper(ਖਾਲੀ *altoghether_ptr, size_t customer_sz,
				     size_t alignment, ਪੂਰਨ clean_content)
{
	ਅਚਲ size_t slop = alignment ?
		mcheck_evaluate_memalign_ਅਗੇਤਰ_ਮਾਪ(alignment) - ਆਕਾਰ(ਬਣਤਰ mcheck_hdr) : 0;
	ਬਣਤਰ mcheck_hdr *hdr = (ਬਣਤਰ mcheck_hdr *)((ਅੱਖਰ *)altoghether_ptr + slop);
	ਪੂਰਨ i;

	hdr->ਮਾਪ = customer_sz;
	hdr->aln_skip = slop;
	ਲਈ (i = 0; i < CANARY_ਡੂੰਘਾਈ; ++i)
		hdr->canary.elems[i] = MAGICWORD;

	ਅੱਖਰ *payload = (ਅੱਖਰ *)&hdr[1];

	ਜੇ (clean_content)
		mcheck_flood(payload, MALLOCFLOOD, customer_sz);

	ਅਚਲ size_t customer_ਮਾਪ_aligned = mcheck_allign_customer_ਮਾਪ(customer_sz);

	mcheck_flood(payload + customer_sz, PADDINGFLOOD, customer_ਮਾਪ_aligned - customer_sz);

	mcheck_canary *ਪੂਛ = (mcheck_canary *)&payload[customer_ਮਾਪ_aligned];

	ਲਈ (i = 0; i < CANARY_ਡੂੰਘਾਈ; ++i)
		ਪੂਛ->elems[i] = MAGICTAIL;

	++mcheck_chunk_ਗਿਣਤੀ;
	ਜੇ (mcheck_chunk_ਗਿਣਤੀ > mcheck_chunk_ਗਿਣਤੀ_max)
		mcheck_chunk_ਗਿਣਤੀ_max = mcheck_chunk_ਗਿਣਤੀ;

	ਲਈ (i = 0; i < REGISTRY_SZ; ++i)
		ਜੇ (!mcheck_registry[i]) {
			mcheck_registry[i] = hdr;
			ਵਾਪਸ payload; // normal end
		}

	ਸਥਿਰ ਅੱਖਰ *ਉਛਾਲ_ਸੁਨੇਹਾ = "\n\n\nERROR: mcheck registry overflow, pedantic check would be incomplete!!\n\n\n\n";

	ਲਿਖੋ("%s", ਉਛਾਲ_ਸੁਨੇਹਾ);
	ਉਛਾਲ_ਸੁਨੇਹਾ = "(mcheck registry full)";
	ਵਾਪਸ payload;
}

ਸਥਿਰ ਖਾਲੀ *mcheck_alloc_posthook(ਖਾਲੀ *altoghether_ptr, size_t customer_sz)
{
	ਵਾਪਸ mcheck_allocated_helper(altoghether_ptr, customer_sz, ANY_ALIGNMENT, CLEAN_CONTENT);
}

ਸਥਿਰ ਖਾਲੀ *mcheck_alloc_noclean_posthook(ਖਾਲੀ *altoghether_ptr, size_t customer_sz)
{
	ਵਾਪਸ mcheck_allocated_helper(altoghether_ptr, customer_sz, ANY_ALIGNMENT, KEEP_CONTENT);
}

ਸਥਿਰ size_t mcheck_memalign_prehook(size_t alig, size_t sz)
{
	ਵਾਪਸ mcheck_evaluate_memalign_ਅਗੇਤਰ_ਮਾਪ(alig) + sz + ਆਕਾਰ(mcheck_canary);
}

ਸਥਿਰ ਖਾਲੀ *mcheck_memalign_posthook(size_t alignment, ਖਾਲੀ *altoghether_ptr, size_t customer_sz)
{
	ਵਾਪਸ mcheck_allocated_helper(altoghether_ptr, customer_sz, alignment, CLEAN_CONTENT);
}

ਸਥਿਰ ਗਿਣਤੀ mcheck_status mcheck_mprobe(ਖਾਲੀ *ptr)
{
	ਬਣਤਰ mcheck_hdr *hdr = &((ਬਣਤਰ mcheck_hdr *)ptr)[-1];

	ਵਾਪਸ mcheck_checkhdr(hdr);
}

ਸਥਿਰ ਖਾਲੀ mcheck_pedantic_check(ਖਾਲੀ)
{
	ਪੂਰਨ i;

	ਲਈ (i = 0; i < REGISTRY_SZ; ++i)
		ਜੇ (mcheck_registry[i])
			mcheck_checkhdr(mcheck_registry[i]);
}

ਸਥਿਰ ਖਾਲੀ mcheck_pedantic_prehook(ਖਾਲੀ)
{
	ਜੇ (mcheck_pedantic_flag)
		mcheck_pedantic_check();
}

ਸਥਿਰ ਖਾਲੀ mcheck_initialize(mcheck_abortfunc_t new_func, ਅੱਖਰ pedantic_flag)
{
	mcheck_abortfunc = (new_func) ? new_func : &mcheck_default_abort;
	mcheck_pedantic_flag = pedantic_flag;
}

ਖਾਲੀ mcheck_on_ramrelocation(size_t ਵਿੱਥ)
{
	ਅੱਖਰ *p;
	ਪੂਰਨ i;
	// Simple, but inaccurate strategy: drop the pre-reloc heap
	ਲਈ (i = 0; i < REGISTRY_SZ; ++i)
		ਜੇ ((p = mcheck_registry[i]) != ਨਿਰਾ ) {
			ਲਿਖੋ("mcheck, WRN: forgetting %p chunk\n", p);
			mcheck_registry[i] = 0;
		}

	mcheck_chunk_ਗਿਣਤੀ = 0;
}
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
