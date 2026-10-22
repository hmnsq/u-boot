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
#నిర్వచితం_కాకపోతే _MCHECKCORE_INC_H
#నిర్వచించు _MCHECKCORE_INC_H      1
#అన్తర్భావ "mcheck.h"

#ఒకవేళ defined(MCHECK_కుప్ప_PROTECTION)
#నిర్వచించు mcheck_flood memset

// these are from /dev/random:
#నిర్వచించు MAGICWORD	0x99ccf430fa562a05ULL
#నిర్వచించు MAGICFREE	0x4875e63c0c6fc08eULL
#నిర్వచించు MAGICTAIL	0x918dbcd7df78dcd6ULL
#నిర్వచించు MALLOCFLOOD	((అక్షరం)0xb6)
#నిర్వచించు FREEFLOOD	((అక్షరం)0xf5)
#నిర్వచించు PADDINGFLOOD	((అక్షరం)0x58)

// my normal run demands 4427-6449 chunks:
#నిర్వచించు REGISTRY_SZ	6608
#నిర్వచించు CANARY_లోతు	2

// avoid problems with BSS at early stage:
స్థిర అక్షరం mcheck_pedantic_flag __section(".data") = 0;
స్థిర శూన్య *mcheck_registry[REGISTRY_SZ] __section(".data") = {0};
స్థిర size_t mcheck_chunk_లెక్క __section(".data") = 0;
స్థిర size_t mcheck_chunk_లెక్క_max __section(".data") = 0;

రకనిర్వచనం గుర్తులేని దీర్ఘ దీర్ఘ mcheck_elem;
రకనిర్వచనం నిర్మాణం {
	mcheck_elem elems[CANARY_లోతు];
} mcheck_canary;
నిర్మాణం mcheck_hdr {
	size_t కొలత; /* Exact size requested by user.  */
	size_t aln_skip; /* Ignored bytes, before the mcheck_hdr, to fulfill alignment */
	mcheck_canary canary; /* Magic number to check header integrity.  */
};

స్థిర శూన్య mcheck_default_abort(గణన mcheck_status స్థితి_2, స్థిరరాశి శూన్య *p)
{
	స్థిరరాశి అక్షరం *సందేశం;

	ఎంపిక (స్థితి_2) {
	స్థితి MCHECK_OK:
		సందేశం = "memory is consistent, library is buggy\n";
		ఆపు;
	స్థితి MCHECK_HEAD:
		సందేశం = "memory clobbered before allocated block\n";
		ఆపు;
	స్థితి MCHECK_TAIL:
		సందేశం = "memory clobbered past end of allocated block\n";
		ఆపు;
	స్థితి MCHECK_FREE:
		సందేశం = "block freed twice\n";
		ఆపు;
	అప్రమేయ:
		సందేశం = "bogus mcheck_status, library is buggy\n";
		ఆపు;
	}
	రాయి("\n\nmcheck: %p:%s!!! [%zu]\n\n", p, సందేశం, mcheck_chunk_లెక్క_max);
}

స్థిర mcheck_abortfunc_t mcheck_abortfunc = &mcheck_default_abort;

స్థిర అంతఃవరుస size_t allign_కొలత_పైకి(size_t sz, size_t grain)
{
	తిరిగి (sz + grain - 1) & ~(grain - 1);
}

#నిర్వచించు mcheck_allign_customer_కొలత(SZ) allign_కొలత_పైకి(SZ, పరిమాణం(mcheck_elem))
#నిర్వచించు mcheck_evaluate_memalign_ఉపసర్గ_కొలత(ALIGN) allign_కొలత_పైకి(పరిమాణం(నిర్మాణం mcheck_hdr), ALIGN)

స్థిర గణన mcheck_status mcheck_OnNok(గణన mcheck_status స్థితి_2, స్థిరరాశి శూన్య *p)
{
	(*mcheck_abortfunc)(స్థితి_2, p);
	తిరిగి స్థితి_2;
}

స్థిర గణన mcheck_status mcheck_checkhdr(స్థిరరాశి నిర్మాణం mcheck_hdr *hdr)
{
	పూర్ణాంకం i;

	కొరకు (i = 0; i < CANARY_లోతు; ++i)
		ఒకవేళ (hdr->canary.elems[i] == MAGICFREE)
			తిరిగి mcheck_OnNok(MCHECK_FREE, hdr + 1);

	కొరకు (i = 0; i < CANARY_లోతు; ++i)
		ఒకవేళ (hdr->canary.elems[i] != MAGICWORD)
			తిరిగి mcheck_OnNok(MCHECK_HEAD, hdr + 1);

	స్థిరరాశి size_t payload_కొలత = hdr->కొలత;
	స్థిరరాశి size_t payload_కొలత_aligned = mcheck_allign_customer_కొలత(payload_కొలత);
	స్థిరరాశి size_t padd_కొలత = payload_కొలత_aligned - hdr->కొలత;

	స్థిరరాశి అక్షరం *payload = (స్థిరరాశి అక్షరం *)&hdr[1];

	కొరకు (i = 0; i < padd_కొలత; ++i)
		ఒకవేళ (payload[payload_కొలత + i] != PADDINGFLOOD)
			తిరిగి mcheck_OnNok(MCHECK_TAIL, hdr + 1);

	స్థిరరాశి mcheck_canary *తోక = (స్థిరరాశి mcheck_canary *)&payload[payload_కొలత_aligned];

	కొరకు (i = 0; i < CANARY_లోతు; ++i)
		ఒకవేళ (తోక->elems[i] != MAGICTAIL)
			తిరిగి mcheck_OnNok(MCHECK_TAIL, hdr + 1);
	తిరిగి MCHECK_OK;
}

గణన { KEEP_CONTENT = 0, CLEAN_CONTENT, ANY_ALIGNMENT = 1 };
స్థిర శూన్య *mcheck_free_helper(శూన్య *ptr, పూర్ణాంకం clean_content)
{
	ఒకవేళ (!ptr)
		తిరిగి ptr;

	నిర్మాణం mcheck_hdr *hdr = &((నిర్మాణం mcheck_hdr *)ptr)[-1];
	పూర్ణాంకం i;

	mcheck_checkhdr(hdr);
	కొరకు (i = 0; i < CANARY_లోతు; ++i)
		hdr->canary.elems[i] = MAGICFREE;

	ఒకవేళ (clean_content)
		mcheck_flood(ptr, FREEFLOOD, mcheck_allign_customer_కొలత(hdr->కొలత));

	కొరకు (i = 0; i < REGISTRY_SZ; ++i)
		ఒకవేళ (mcheck_registry[i] == hdr) {
			mcheck_registry[i] = 0;
			ఆపు;
		}

	--mcheck_chunk_లెక్క;
	తిరిగి (అక్షరం *)hdr - hdr->aln_skip;
}

స్థిర శూన్య *mcheck_free_prehook(శూన్య *ptr) { తిరిగి mcheck_free_helper(ptr, CLEAN_CONTENT); }
స్థిర శూన్య *mcheck_reallocfree_prehook(శూన్య *ptr) { తిరిగి mcheck_free_helper(ptr, KEEP_CONTENT); }

స్థిర size_t mcheck_alloc_prehook(size_t sz)
{
	sz = mcheck_allign_customer_కొలత(sz);
	తిరిగి పరిమాణం(నిర్మాణం mcheck_hdr) + sz + పరిమాణం(mcheck_canary);
}

స్థిర శూన్య *mcheck_allocated_helper(శూన్య *altoghether_ptr, size_t customer_sz,
				     size_t alignment, పూర్ణాంకం clean_content)
{
	స్థిరరాశి size_t slop = alignment ?
		mcheck_evaluate_memalign_ఉపసర్గ_కొలత(alignment) - పరిమాణం(నిర్మాణం mcheck_hdr) : 0;
	నిర్మాణం mcheck_hdr *hdr = (నిర్మాణం mcheck_hdr *)((అక్షరం *)altoghether_ptr + slop);
	పూర్ణాంకం i;

	hdr->కొలత = customer_sz;
	hdr->aln_skip = slop;
	కొరకు (i = 0; i < CANARY_లోతు; ++i)
		hdr->canary.elems[i] = MAGICWORD;

	అక్షరం *payload = (అక్షరం *)&hdr[1];

	ఒకవేళ (clean_content)
		mcheck_flood(payload, MALLOCFLOOD, customer_sz);

	స్థిరరాశి size_t customer_కొలత_aligned = mcheck_allign_customer_కొలత(customer_sz);

	mcheck_flood(payload + customer_sz, PADDINGFLOOD, customer_కొలత_aligned - customer_sz);

	mcheck_canary *తోక = (mcheck_canary *)&payload[customer_కొలత_aligned];

	కొరకు (i = 0; i < CANARY_లోతు; ++i)
		తోక->elems[i] = MAGICTAIL;

	++mcheck_chunk_లెక్క;
	ఒకవేళ (mcheck_chunk_లెక్క > mcheck_chunk_లెక్క_max)
		mcheck_chunk_లెక్క_max = mcheck_chunk_లెక్క;

	కొరకు (i = 0; i < REGISTRY_SZ; ++i)
		ఒకవేళ (!mcheck_registry[i]) {
			mcheck_registry[i] = hdr;
			తిరిగి payload; // normal end
		}

	స్థిర అక్షరం *అధికప్రవాహం_సందేశం = "\n\n\nERROR: mcheck registry overflow, pedantic check would be incomplete!!\n\n\n\n";

	రాయి("%s", అధికప్రవాహం_సందేశం);
	అధికప్రవాహం_సందేశం = "(mcheck registry full)";
	తిరిగి payload;
}

స్థిర శూన్య *mcheck_alloc_posthook(శూన్య *altoghether_ptr, size_t customer_sz)
{
	తిరిగి mcheck_allocated_helper(altoghether_ptr, customer_sz, ANY_ALIGNMENT, CLEAN_CONTENT);
}

స్థిర శూన్య *mcheck_alloc_noclean_posthook(శూన్య *altoghether_ptr, size_t customer_sz)
{
	తిరిగి mcheck_allocated_helper(altoghether_ptr, customer_sz, ANY_ALIGNMENT, KEEP_CONTENT);
}

స్థిర size_t mcheck_memalign_prehook(size_t alig, size_t sz)
{
	తిరిగి mcheck_evaluate_memalign_ఉపసర్గ_కొలత(alig) + sz + పరిమాణం(mcheck_canary);
}

స్థిర శూన్య *mcheck_memalign_posthook(size_t alignment, శూన్య *altoghether_ptr, size_t customer_sz)
{
	తిరిగి mcheck_allocated_helper(altoghether_ptr, customer_sz, alignment, CLEAN_CONTENT);
}

స్థిర గణన mcheck_status mcheck_mprobe(శూన్య *ptr)
{
	నిర్మాణం mcheck_hdr *hdr = &((నిర్మాణం mcheck_hdr *)ptr)[-1];

	తిరిగి mcheck_checkhdr(hdr);
}

స్థిర శూన్య mcheck_pedantic_check(శూన్య)
{
	పూర్ణాంకం i;

	కొరకు (i = 0; i < REGISTRY_SZ; ++i)
		ఒకవేళ (mcheck_registry[i])
			mcheck_checkhdr(mcheck_registry[i]);
}

స్థిర శూన్య mcheck_pedantic_prehook(శూన్య)
{
	ఒకవేళ (mcheck_pedantic_flag)
		mcheck_pedantic_check();
}

స్థిర శూన్య mcheck_initialize(mcheck_abortfunc_t new_func, అక్షరం pedantic_flag)
{
	mcheck_abortfunc = (new_func) ? new_func : &mcheck_default_abort;
	mcheck_pedantic_flag = pedantic_flag;
}

శూన్య mcheck_on_ramrelocation(size_t స్థానభ్రంశం)
{
	అక్షరం *p;
	పూర్ణాంకం i;
	// Simple, but inaccurate strategy: drop the pre-reloc heap
	కొరకు (i = 0; i < REGISTRY_SZ; ++i)
		ఒకవేళ ((p = mcheck_registry[i]) != ఖాళీ ) {
			రాయి("mcheck, WRN: forgetting %p chunk\n", p);
			mcheck_registry[i] = 0;
		}

	mcheck_chunk_లెక్క = 0;
}
#అయితే_ముగింపు
#అయితే_ముగింపు
