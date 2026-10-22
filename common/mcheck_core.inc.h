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
#અન્તર્ભાવ "mcheck.h"

#જો defined(MCHECK_HEAP_PROTECTION)
#define mcheck_flood memset

// these are from /dev/random:
#define MAGICWORD	0x99ccf430fa562a05ULL
#define MAGICFREE	0x4875e63c0c6fc08eULL
#define MAGICTAIL	0x918dbcd7df78dcd6ULL
#define MALLOCFLOOD	((અક્ષર)0xb6)
#define FREEFLOOD	((અક્ષર)0xf5)
#define PADDINGFLOOD	((અક્ષર)0x58)

// my normal run demands 4427-6449 chunks:
#define REGISTRY_SZ	6608
#define CANARY_DEPTH	2

// avoid problems with BSS at early stage:
સ્થિર અક્ષર mcheck_pedantic_flag __section(".data") = 0;
સ્થિર શૂન્ય *mcheck_registry[REGISTRY_SZ] __section(".data") = {0};
સ્થિર size_t mcheck_chunk_ગણતરી __section(".data") = 0;
સ્થિર size_t mcheck_chunk_ગણતરી_max __section(".data") = 0;

પ્રકારવ્યાખ્યા unsigned long long mcheck_elem;
પ્રકારવ્યાખ્યા સંરચના {
	mcheck_elem elems[CANARY_DEPTH];
} mcheck_canary;
સંરચના mcheck_hdr {
	size_t size; /* Exact size requested by user.  */
	size_t aln_skip; /* Ignored bytes, before the mcheck_hdr, to fulfill alignment */
	mcheck_canary canary; /* Magic number to check header integrity.  */
};

સ્થિર શૂન્ય mcheck_default_abort(ગણના mcheck_status status, અચલ શૂન્ય *p)
{
	અચલ અક્ષર *સંદેશ;

	પસંદગી (status) {
	સ્થિતિ MCHECK_OK:
		સંદેશ = "memory is consistent, library is buggy\n";
		તોડો;
	સ્થિતિ MCHECK_HEAD:
		સંદેશ = "memory clobbered before allocated block\n";
		તોડો;
	સ્થિતિ MCHECK_TAIL:
		સંદેશ = "memory clobbered past end of allocated block\n";
		તોડો;
	સ્થિતિ MCHECK_FREE:
		સંદેશ = "block freed twice\n";
		તોડો;
	મૂળભૂત:
		સંદેશ = "bogus mcheck_status, library is buggy\n";
		તોડો;
	}
	printf("\n\nmcheck: %p:%s!!! [%zu]\n\n", p, સંદેશ, mcheck_chunk_ગણતરી_max);
}

સ્થિર mcheck_abortfunc_t mcheck_abortfunc = &mcheck_default_abort;

સ્થિર અંતઃરેખ size_t allign_size_up(size_t sz, size_t grain)
{
	પાછા (sz + grain - 1) & ~(grain - 1);
}

#define mcheck_allign_customer_size(SZ) allign_size_up(SZ, કદ(mcheck_elem))
#define mcheck_evaluate_memalign_prefix_size(ALIGN) allign_size_up(કદ(સંરચના mcheck_hdr), ALIGN)

સ્થિર ગણના mcheck_status mcheck_OnNok(ગણના mcheck_status status, અચલ શૂન્ય *p)
{
	(*mcheck_abortfunc)(status, p);
	પાછા status;
}

સ્થિર ગણના mcheck_status mcheck_checkhdr(અચલ સંરચના mcheck_hdr *hdr)
{
	પૂર્ણાંક i;

	માટે (i = 0; i < CANARY_DEPTH; ++i)
		જો (hdr->canary.elems[i] == MAGICFREE)
			પાછા mcheck_OnNok(MCHECK_FREE, hdr + 1);

	માટે (i = 0; i < CANARY_DEPTH; ++i)
		જો (hdr->canary.elems[i] != MAGICWORD)
			પાછા mcheck_OnNok(MCHECK_HEAD, hdr + 1);

	અચલ size_t payload_size = hdr->size;
	અચલ size_t payload_size_aligned = mcheck_allign_customer_size(payload_size);
	અચલ size_t padd_size = payload_size_aligned - hdr->size;

	અચલ અક્ષર *payload = (અચલ અક્ષર *)&hdr[1];

	માટે (i = 0; i < padd_size; ++i)
		જો (payload[payload_size + i] != PADDINGFLOOD)
			પાછા mcheck_OnNok(MCHECK_TAIL, hdr + 1);

	અચલ mcheck_canary *tail = (અચલ mcheck_canary *)&payload[payload_size_aligned];

	માટે (i = 0; i < CANARY_DEPTH; ++i)
		જો (tail->elems[i] != MAGICTAIL)
			પાછા mcheck_OnNok(MCHECK_TAIL, hdr + 1);
	પાછા MCHECK_OK;
}

ગણના { KEEP_CONTENT = 0, CLEAN_CONTENT, ANY_ALIGNMENT = 1 };
સ્થિર શૂન્ય *mcheck_free_helper(શૂન્ય *ptr, પૂર્ણાંક clean_content)
{
	જો (!ptr)
		પાછા ptr;

	સંરચના mcheck_hdr *hdr = &((સંરચના mcheck_hdr *)ptr)[-1];
	પૂર્ણાંક i;

	mcheck_checkhdr(hdr);
	માટે (i = 0; i < CANARY_DEPTH; ++i)
		hdr->canary.elems[i] = MAGICFREE;

	જો (clean_content)
		mcheck_flood(ptr, FREEFLOOD, mcheck_allign_customer_size(hdr->size));

	માટે (i = 0; i < REGISTRY_SZ; ++i)
		જો (mcheck_registry[i] == hdr) {
			mcheck_registry[i] = 0;
			તોડો;
		}

	--mcheck_chunk_ગણતરી;
	પાછા (અક્ષર *)hdr - hdr->aln_skip;
}

સ્થિર શૂન્ય *mcheck_free_prehook(શૂન્ય *ptr) { પાછા mcheck_free_helper(ptr, CLEAN_CONTENT); }
સ્થિર શૂન્ય *mcheck_reallocfree_prehook(શૂન્ય *ptr) { પાછા mcheck_free_helper(ptr, KEEP_CONTENT); }

સ્થિર size_t mcheck_alloc_prehook(size_t sz)
{
	sz = mcheck_allign_customer_size(sz);
	પાછા કદ(સંરચના mcheck_hdr) + sz + કદ(mcheck_canary);
}

સ્થિર શૂન્ય *mcheck_allocated_helper(શૂન્ય *altoghether_ptr, size_t customer_sz,
				     size_t alignment, પૂર્ણાંક clean_content)
{
	અચલ size_t slop = alignment ?
		mcheck_evaluate_memalign_prefix_size(alignment) - કદ(સંરચના mcheck_hdr) : 0;
	સંરચના mcheck_hdr *hdr = (સંરચના mcheck_hdr *)((અક્ષર *)altoghether_ptr + slop);
	પૂર્ણાંક i;

	hdr->size = customer_sz;
	hdr->aln_skip = slop;
	માટે (i = 0; i < CANARY_DEPTH; ++i)
		hdr->canary.elems[i] = MAGICWORD;

	અક્ષર *payload = (અક્ષર *)&hdr[1];

	જો (clean_content)
		mcheck_flood(payload, MALLOCFLOOD, customer_sz);

	અચલ size_t customer_size_aligned = mcheck_allign_customer_size(customer_sz);

	mcheck_flood(payload + customer_sz, PADDINGFLOOD, customer_size_aligned - customer_sz);

	mcheck_canary *tail = (mcheck_canary *)&payload[customer_size_aligned];

	માટે (i = 0; i < CANARY_DEPTH; ++i)
		tail->elems[i] = MAGICTAIL;

	++mcheck_chunk_ગણતરી;
	જો (mcheck_chunk_ગણતરી > mcheck_chunk_ગણતરી_max)
		mcheck_chunk_ગણતરી_max = mcheck_chunk_ગણતરી;

	માટે (i = 0; i < REGISTRY_SZ; ++i)
		જો (!mcheck_registry[i]) {
			mcheck_registry[i] = hdr;
			પાછા payload; // normal end
		}

	સ્થિર અક્ષર *overflow_સંદેશ = "\n\n\nERROR: mcheck registry overflow, pedantic check would be incomplete!!\n\n\n\n";

	printf("%s", overflow_સંદેશ);
	overflow_સંદેશ = "(mcheck registry full)";
	પાછા payload;
}

સ્થિર શૂન્ય *mcheck_alloc_posthook(શૂન્ય *altoghether_ptr, size_t customer_sz)
{
	પાછા mcheck_allocated_helper(altoghether_ptr, customer_sz, ANY_ALIGNMENT, CLEAN_CONTENT);
}

સ્થિર શૂન્ય *mcheck_alloc_noclean_posthook(શૂન્ય *altoghether_ptr, size_t customer_sz)
{
	પાછા mcheck_allocated_helper(altoghether_ptr, customer_sz, ANY_ALIGNMENT, KEEP_CONTENT);
}

સ્થિર size_t mcheck_memalign_prehook(size_t alig, size_t sz)
{
	પાછા mcheck_evaluate_memalign_prefix_size(alig) + sz + કદ(mcheck_canary);
}

સ્થિર શૂન્ય *mcheck_memalign_posthook(size_t alignment, શૂન્ય *altoghether_ptr, size_t customer_sz)
{
	પાછા mcheck_allocated_helper(altoghether_ptr, customer_sz, alignment, CLEAN_CONTENT);
}

સ્થિર ગણના mcheck_status mcheck_mprobe(શૂન્ય *ptr)
{
	સંરચના mcheck_hdr *hdr = &((સંરચના mcheck_hdr *)ptr)[-1];

	પાછા mcheck_checkhdr(hdr);
}

સ્થિર શૂન્ય mcheck_pedantic_check(શૂન્ય)
{
	પૂર્ણાંક i;

	માટે (i = 0; i < REGISTRY_SZ; ++i)
		જો (mcheck_registry[i])
			mcheck_checkhdr(mcheck_registry[i]);
}

સ્થિર શૂન્ય mcheck_pedantic_prehook(શૂન્ય)
{
	જો (mcheck_pedantic_flag)
		mcheck_pedantic_check();
}

સ્થિર શૂન્ય mcheck_initialize(mcheck_abortfunc_t new_func, અક્ષર pedantic_flag)
{
	mcheck_abortfunc = (new_func) ? new_func : &mcheck_default_abort;
	mcheck_pedantic_flag = pedantic_flag;
}

શૂન્ય mcheck_on_ramrelocation(size_t offset)
{
	અક્ષર *p;
	પૂર્ણાંક i;
	// Simple, but inaccurate strategy: drop the pre-reloc heap
	માટે (i = 0; i < REGISTRY_SZ; ++i)
		જો ((p = mcheck_registry[i]) != ખાલી ) {
			printf("mcheck, WRN: forgetting %p chunk\n", p);
			mcheck_registry[i] = 0;
		}

	mcheck_chunk_ગણતરી = 0;
}
#endif
#endif
