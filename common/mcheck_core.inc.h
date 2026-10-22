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
#അന്തര്ഭാവ "mcheck.h"

#എങ്കിൽ defined(MCHECK_HEAP_PROTECTION)
#define mcheck_flood memset

// these are from /dev/random:
#define MAGICWORD	0x99ccf430fa562a05ULL
#define MAGICFREE	0x4875e63c0c6fc08eULL
#define MAGICTAIL	0x918dbcd7df78dcd6ULL
#define MALLOCFLOOD	((അക്ഷരം)0xb6)
#define FREEFLOOD	((അക്ഷരം)0xf5)
#define PADDINGFLOOD	((അക്ഷരം)0x58)

// my normal run demands 4427-6449 chunks:
#define REGISTRY_SZ	6608
#define CANARY_DEPTH	2

// avoid problems with BSS at early stage:
സ്ഥിതം അക്ഷരം mcheck_pedantic_flag __section(".data") = 0;
സ്ഥിതം ശൂന്യം *mcheck_registry[REGISTRY_SZ] __section(".data") = {0};
സ്ഥിതം size_t mcheck_chunk_എണ്ണം __section(".data") = 0;
സ്ഥിതം size_t mcheck_chunk_എണ്ണം_max __section(".data") = 0;

തരനിർവചനം unsigned long long mcheck_elem;
തരനിർവചനം ഘടന {
	mcheck_elem elems[CANARY_DEPTH];
} mcheck_canary;
ഘടന mcheck_hdr {
	size_t size; /* Exact size requested by user.  */
	size_t aln_skip; /* Ignored bytes, before the mcheck_hdr, to fulfill alignment */
	mcheck_canary canary; /* Magic number to check header integrity.  */
};

സ്ഥിതം ശൂന്യം mcheck_default_abort(എണ്ണൽ mcheck_status status, സ്ഥിരം ശൂന്യം *p)
{
	സ്ഥിരം അക്ഷരം *സന്ദേശം;

	തിരഞ്ഞെടുപ്പ് (status) {
	അവസ്ഥ MCHECK_OK:
		സന്ദേശം = "memory is consistent, library is buggy\n";
		നിർത്തുക;
	അവസ്ഥ MCHECK_HEAD:
		സന്ദേശം = "memory clobbered before allocated block\n";
		നിർത്തുക;
	അവസ്ഥ MCHECK_TAIL:
		സന്ദേശം = "memory clobbered past end of allocated block\n";
		നിർത്തുക;
	അവസ്ഥ MCHECK_FREE:
		സന്ദേശം = "block freed twice\n";
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		സന്ദേശം = "bogus mcheck_status, library is buggy\n";
		നിർത്തുക;
	}
	printf("\n\nmcheck: %p:%s!!! [%zu]\n\n", p, സന്ദേശം, mcheck_chunk_എണ്ണം_max);
}

സ്ഥിതം mcheck_abortfunc_t mcheck_abortfunc = &mcheck_default_abort;

സ്ഥിതം വരിയിൽ size_t allign_size_up(size_t sz, size_t grain)
{
	തിരികെ (sz + grain - 1) & ~(grain - 1);
}

#define mcheck_allign_customer_size(SZ) allign_size_up(SZ, വലുപ്പം(mcheck_elem))
#define mcheck_evaluate_memalign_prefix_size(ALIGN) allign_size_up(വലുപ്പം(ഘടന mcheck_hdr), ALIGN)

സ്ഥിതം എണ്ണൽ mcheck_status mcheck_OnNok(എണ്ണൽ mcheck_status status, സ്ഥിരം ശൂന്യം *p)
{
	(*mcheck_abortfunc)(status, p);
	തിരികെ status;
}

സ്ഥിതം എണ്ണൽ mcheck_status mcheck_checkhdr(സ്ഥിരം ഘടന mcheck_hdr *hdr)
{
	പൂർണ്ണസംഖ്യ i;

	വേണ്ടി (i = 0; i < CANARY_DEPTH; ++i)
		എങ്കിൽ (hdr->canary.elems[i] == MAGICFREE)
			തിരികെ mcheck_OnNok(MCHECK_FREE, hdr + 1);

	വേണ്ടി (i = 0; i < CANARY_DEPTH; ++i)
		എങ്കിൽ (hdr->canary.elems[i] != MAGICWORD)
			തിരികെ mcheck_OnNok(MCHECK_HEAD, hdr + 1);

	സ്ഥിരം size_t payload_size = hdr->size;
	സ്ഥിരം size_t payload_size_aligned = mcheck_allign_customer_size(payload_size);
	സ്ഥിരം size_t padd_size = payload_size_aligned - hdr->size;

	സ്ഥിരം അക്ഷരം *payload = (സ്ഥിരം അക്ഷരം *)&hdr[1];

	വേണ്ടി (i = 0; i < padd_size; ++i)
		എങ്കിൽ (payload[payload_size + i] != PADDINGFLOOD)
			തിരികെ mcheck_OnNok(MCHECK_TAIL, hdr + 1);

	സ്ഥിരം mcheck_canary *tail = (സ്ഥിരം mcheck_canary *)&payload[payload_size_aligned];

	വേണ്ടി (i = 0; i < CANARY_DEPTH; ++i)
		എങ്കിൽ (tail->elems[i] != MAGICTAIL)
			തിരികെ mcheck_OnNok(MCHECK_TAIL, hdr + 1);
	തിരികെ MCHECK_OK;
}

എണ്ണൽ { KEEP_CONTENT = 0, CLEAN_CONTENT, ANY_ALIGNMENT = 1 };
സ്ഥിതം ശൂന്യം *mcheck_free_helper(ശൂന്യം *ptr, പൂർണ്ണസംഖ്യ clean_content)
{
	എങ്കിൽ (!ptr)
		തിരികെ ptr;

	ഘടന mcheck_hdr *hdr = &((ഘടന mcheck_hdr *)ptr)[-1];
	പൂർണ്ണസംഖ്യ i;

	mcheck_checkhdr(hdr);
	വേണ്ടി (i = 0; i < CANARY_DEPTH; ++i)
		hdr->canary.elems[i] = MAGICFREE;

	എങ്കിൽ (clean_content)
		mcheck_flood(ptr, FREEFLOOD, mcheck_allign_customer_size(hdr->size));

	വേണ്ടി (i = 0; i < REGISTRY_SZ; ++i)
		എങ്കിൽ (mcheck_registry[i] == hdr) {
			mcheck_registry[i] = 0;
			നിർത്തുക;
		}

	--mcheck_chunk_എണ്ണം;
	തിരികെ (അക്ഷരം *)hdr - hdr->aln_skip;
}

സ്ഥിതം ശൂന്യം *mcheck_free_prehook(ശൂന്യം *ptr) { തിരികെ mcheck_free_helper(ptr, CLEAN_CONTENT); }
സ്ഥിതം ശൂന്യം *mcheck_reallocfree_prehook(ശൂന്യം *ptr) { തിരികെ mcheck_free_helper(ptr, KEEP_CONTENT); }

സ്ഥിതം size_t mcheck_alloc_prehook(size_t sz)
{
	sz = mcheck_allign_customer_size(sz);
	തിരികെ വലുപ്പം(ഘടന mcheck_hdr) + sz + വലുപ്പം(mcheck_canary);
}

സ്ഥിതം ശൂന്യം *mcheck_allocated_helper(ശൂന്യം *altoghether_ptr, size_t customer_sz,
				     size_t alignment, പൂർണ്ണസംഖ്യ clean_content)
{
	സ്ഥിരം size_t slop = alignment ?
		mcheck_evaluate_memalign_prefix_size(alignment) - വലുപ്പം(ഘടന mcheck_hdr) : 0;
	ഘടന mcheck_hdr *hdr = (ഘടന mcheck_hdr *)((അക്ഷരം *)altoghether_ptr + slop);
	പൂർണ്ണസംഖ്യ i;

	hdr->size = customer_sz;
	hdr->aln_skip = slop;
	വേണ്ടി (i = 0; i < CANARY_DEPTH; ++i)
		hdr->canary.elems[i] = MAGICWORD;

	അക്ഷരം *payload = (അക്ഷരം *)&hdr[1];

	എങ്കിൽ (clean_content)
		mcheck_flood(payload, MALLOCFLOOD, customer_sz);

	സ്ഥിരം size_t customer_size_aligned = mcheck_allign_customer_size(customer_sz);

	mcheck_flood(payload + customer_sz, PADDINGFLOOD, customer_size_aligned - customer_sz);

	mcheck_canary *tail = (mcheck_canary *)&payload[customer_size_aligned];

	വേണ്ടി (i = 0; i < CANARY_DEPTH; ++i)
		tail->elems[i] = MAGICTAIL;

	++mcheck_chunk_എണ്ണം;
	എങ്കിൽ (mcheck_chunk_എണ്ണം > mcheck_chunk_എണ്ണം_max)
		mcheck_chunk_എണ്ണം_max = mcheck_chunk_എണ്ണം;

	വേണ്ടി (i = 0; i < REGISTRY_SZ; ++i)
		എങ്കിൽ (!mcheck_registry[i]) {
			mcheck_registry[i] = hdr;
			തിരികെ payload; // normal end
		}

	സ്ഥിതം അക്ഷരം *overflow_സന്ദേശം = "\n\n\nERROR: mcheck registry overflow, pedantic check would be incomplete!!\n\n\n\n";

	printf("%s", overflow_സന്ദേശം);
	overflow_സന്ദേശം = "(mcheck registry full)";
	തിരികെ payload;
}

സ്ഥിതം ശൂന്യം *mcheck_alloc_posthook(ശൂന്യം *altoghether_ptr, size_t customer_sz)
{
	തിരികെ mcheck_allocated_helper(altoghether_ptr, customer_sz, ANY_ALIGNMENT, CLEAN_CONTENT);
}

സ്ഥിതം ശൂന്യം *mcheck_alloc_noclean_posthook(ശൂന്യം *altoghether_ptr, size_t customer_sz)
{
	തിരികെ mcheck_allocated_helper(altoghether_ptr, customer_sz, ANY_ALIGNMENT, KEEP_CONTENT);
}

സ്ഥിതം size_t mcheck_memalign_prehook(size_t alig, size_t sz)
{
	തിരികെ mcheck_evaluate_memalign_prefix_size(alig) + sz + വലുപ്പം(mcheck_canary);
}

സ്ഥിതം ശൂന്യം *mcheck_memalign_posthook(size_t alignment, ശൂന്യം *altoghether_ptr, size_t customer_sz)
{
	തിരികെ mcheck_allocated_helper(altoghether_ptr, customer_sz, alignment, CLEAN_CONTENT);
}

സ്ഥിതം എണ്ണൽ mcheck_status mcheck_mprobe(ശൂന്യം *ptr)
{
	ഘടന mcheck_hdr *hdr = &((ഘടന mcheck_hdr *)ptr)[-1];

	തിരികെ mcheck_checkhdr(hdr);
}

സ്ഥിതം ശൂന്യം mcheck_pedantic_check(ശൂന്യം)
{
	പൂർണ്ണസംഖ്യ i;

	വേണ്ടി (i = 0; i < REGISTRY_SZ; ++i)
		എങ്കിൽ (mcheck_registry[i])
			mcheck_checkhdr(mcheck_registry[i]);
}

സ്ഥിതം ശൂന്യം mcheck_pedantic_prehook(ശൂന്യം)
{
	എങ്കിൽ (mcheck_pedantic_flag)
		mcheck_pedantic_check();
}

സ്ഥിതം ശൂന്യം mcheck_initialize(mcheck_abortfunc_t new_func, അക്ഷരം pedantic_flag)
{
	mcheck_abortfunc = (new_func) ? new_func : &mcheck_default_abort;
	mcheck_pedantic_flag = pedantic_flag;
}

ശൂന്യം mcheck_on_ramrelocation(size_t offset)
{
	അക്ഷരം *p;
	പൂർണ്ണസംഖ്യ i;
	// Simple, but inaccurate strategy: drop the pre-reloc heap
	വേണ്ടി (i = 0; i < REGISTRY_SZ; ++i)
		എങ്കിൽ ((p = mcheck_registry[i]) != ശൂന്യ ) {
			printf("mcheck, WRN: forgetting %p chunk\n", p);
			mcheck_registry[i] = 0;
		}

	mcheck_chunk_എണ്ണം = 0;
}
#endif
#endif
