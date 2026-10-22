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
#اگر_غیر_تعریف_شدہ _MCHECKCORE_INC_H
#تعریف_کرو _MCHECKCORE_INC_H      1
#شامل "mcheck.h"

#اگر defined(MCHECK_ڈھیر_PROTECTION)
#تعریف_کرو mcheck_flood memset

// these are from /dev/random:
#تعریف_کرو MAGICWORD	0x99ccf430fa562a05ULL
#تعریف_کرو MAGICFREE	0x4875e63c0c6fc08eULL
#تعریف_کرو MAGICTAIL	0x918dbcd7df78dcd6ULL
#تعریف_کرو MALLOCFLOOD	((حرف)0xb6)
#تعریف_کرو FREEFLOOD	((حرف)0xf5)
#تعریف_کرو PADDINGFLOOD	((حرف)0x58)

// my normal run demands 4427-6449 chunks:
#تعریف_کرو REGISTRY_SZ	6608
#تعریف_کرو CANARY_گہرائی	2

// avoid problems with BSS at early stage:
ساکن حرف mcheck_pedantic_flag __section(".data") = 0;
ساکن خالی *mcheck_registry[REGISTRY_SZ] __section(".data") = {0};
ساکن size_t mcheck_chunk_گنتی __section(".data") = 0;
ساکن size_t mcheck_chunk_گنتی_max __section(".data") = 0;

قسمتعریف غیرعلامتی طویل طویل mcheck_elem;
قسمتعریف ساخت {
	mcheck_elem elems[CANARY_گہرائی];
} mcheck_canary;
ساخت mcheck_hdr {
	size_t سائز; /* Exact size requested by user.  */
	size_t aln_skip; /* Ignored bytes, before the mcheck_hdr, to fulfill alignment */
	mcheck_canary canary; /* Magic number to check header integrity.  */
};

ساکن خالی mcheck_default_abort(شمار mcheck_status حالت_2, ثابت خالی *p)
{
	ثابت حرف *پیغام;

	انتخاب (حالت_2) {
	حالت MCHECK_OK:
		پیغام = "memory is consistent, library is buggy\n";
		توڑو;
	حالت MCHECK_HEAD:
		پیغام = "memory clobbered before allocated block\n";
		توڑو;
	حالت MCHECK_TAIL:
		پیغام = "memory clobbered past end of allocated block\n";
		توڑو;
	حالت MCHECK_FREE:
		پیغام = "block freed twice\n";
		توڑو;
	طےشدہ:
		پیغام = "bogus mcheck_status, library is buggy\n";
		توڑو;
	}
	لکھو("\n\nmcheck: %p:%s!!! [%zu]\n\n", p, پیغام, mcheck_chunk_گنتی_max);
}

ساکن mcheck_abortfunc_t mcheck_abortfunc = &mcheck_default_abort;

ساکن اندرونی size_t allign_سائز_اوپر(size_t sz, size_t grain)
{
	واپس (sz + grain - 1) & ~(grain - 1);
}

#تعریف_کرو mcheck_allign_customer_سائز(SZ) allign_سائز_اوپر(SZ, حجم(mcheck_elem))
#تعریف_کرو mcheck_evaluate_memalign_سابقہ_سائز(ALIGN) allign_سائز_اوپر(حجم(ساخت mcheck_hdr), ALIGN)

ساکن شمار mcheck_status mcheck_OnNok(شمار mcheck_status حالت_2, ثابت خالی *p)
{
	(*mcheck_abortfunc)(حالت_2, p);
	واپس حالت_2;
}

ساکن شمار mcheck_status mcheck_checkhdr(ثابت ساخت mcheck_hdr *hdr)
{
	صحیح i;

	برائے (i = 0; i < CANARY_گہرائی; ++i)
		اگر (hdr->canary.elems[i] == MAGICFREE)
			واپس mcheck_OnNok(MCHECK_FREE, hdr + 1);

	برائے (i = 0; i < CANARY_گہرائی; ++i)
		اگر (hdr->canary.elems[i] != MAGICWORD)
			واپس mcheck_OnNok(MCHECK_HEAD, hdr + 1);

	ثابت size_t payload_سائز = hdr->سائز;
	ثابت size_t payload_سائز_aligned = mcheck_allign_customer_سائز(payload_سائز);
	ثابت size_t padd_سائز = payload_سائز_aligned - hdr->سائز;

	ثابت حرف *payload = (ثابت حرف *)&hdr[1];

	برائے (i = 0; i < padd_سائز; ++i)
		اگر (payload[payload_سائز + i] != PADDINGFLOOD)
			واپس mcheck_OnNok(MCHECK_TAIL, hdr + 1);

	ثابت mcheck_canary *دم = (ثابت mcheck_canary *)&payload[payload_سائز_aligned];

	برائے (i = 0; i < CANARY_گہرائی; ++i)
		اگر (دم->elems[i] != MAGICTAIL)
			واپس mcheck_OnNok(MCHECK_TAIL, hdr + 1);
	واپس MCHECK_OK;
}

شمار { KEEP_CONTENT = 0, CLEAN_CONTENT, ANY_ALIGNMENT = 1 };
ساکن خالی *mcheck_free_helper(خالی *ptr, صحیح clean_content)
{
	اگر (!ptr)
		واپس ptr;

	ساخت mcheck_hdr *hdr = &((ساخت mcheck_hdr *)ptr)[-1];
	صحیح i;

	mcheck_checkhdr(hdr);
	برائے (i = 0; i < CANARY_گہرائی; ++i)
		hdr->canary.elems[i] = MAGICFREE;

	اگر (clean_content)
		mcheck_flood(ptr, FREEFLOOD, mcheck_allign_customer_سائز(hdr->سائز));

	برائے (i = 0; i < REGISTRY_SZ; ++i)
		اگر (mcheck_registry[i] == hdr) {
			mcheck_registry[i] = 0;
			توڑو;
		}

	--mcheck_chunk_گنتی;
	واپس (حرف *)hdr - hdr->aln_skip;
}

ساکن خالی *mcheck_free_prehook(خالی *ptr) { واپس mcheck_free_helper(ptr, CLEAN_CONTENT); }
ساکن خالی *mcheck_reallocfree_prehook(خالی *ptr) { واپس mcheck_free_helper(ptr, KEEP_CONTENT); }

ساکن size_t mcheck_alloc_prehook(size_t sz)
{
	sz = mcheck_allign_customer_سائز(sz);
	واپس حجم(ساخت mcheck_hdr) + sz + حجم(mcheck_canary);
}

ساکن خالی *mcheck_allocated_helper(خالی *altoghether_ptr, size_t customer_sz,
				     size_t alignment, صحیح clean_content)
{
	ثابت size_t slop = alignment ?
		mcheck_evaluate_memalign_سابقہ_سائز(alignment) - حجم(ساخت mcheck_hdr) : 0;
	ساخت mcheck_hdr *hdr = (ساخت mcheck_hdr *)((حرف *)altoghether_ptr + slop);
	صحیح i;

	hdr->سائز = customer_sz;
	hdr->aln_skip = slop;
	برائے (i = 0; i < CANARY_گہرائی; ++i)
		hdr->canary.elems[i] = MAGICWORD;

	حرف *payload = (حرف *)&hdr[1];

	اگر (clean_content)
		mcheck_flood(payload, MALLOCFLOOD, customer_sz);

	ثابت size_t customer_سائز_aligned = mcheck_allign_customer_سائز(customer_sz);

	mcheck_flood(payload + customer_sz, PADDINGFLOOD, customer_سائز_aligned - customer_sz);

	mcheck_canary *دم = (mcheck_canary *)&payload[customer_سائز_aligned];

	برائے (i = 0; i < CANARY_گہرائی; ++i)
		دم->elems[i] = MAGICTAIL;

	++mcheck_chunk_گنتی;
	اگر (mcheck_chunk_گنتی > mcheck_chunk_گنتی_max)
		mcheck_chunk_گنتی_max = mcheck_chunk_گنتی;

	برائے (i = 0; i < REGISTRY_SZ; ++i)
		اگر (!mcheck_registry[i]) {
			mcheck_registry[i] = hdr;
			واپس payload; // normal end
		}

	ساکن حرف *حد_سے_تجاوز_پیغام = "\n\n\nERROR: mcheck registry overflow, pedantic check would be incomplete!!\n\n\n\n";

	لکھو("%s", حد_سے_تجاوز_پیغام);
	حد_سے_تجاوز_پیغام = "(mcheck registry full)";
	واپس payload;
}

ساکن خالی *mcheck_alloc_posthook(خالی *altoghether_ptr, size_t customer_sz)
{
	واپس mcheck_allocated_helper(altoghether_ptr, customer_sz, ANY_ALIGNMENT, CLEAN_CONTENT);
}

ساکن خالی *mcheck_alloc_noclean_posthook(خالی *altoghether_ptr, size_t customer_sz)
{
	واپس mcheck_allocated_helper(altoghether_ptr, customer_sz, ANY_ALIGNMENT, KEEP_CONTENT);
}

ساکن size_t mcheck_memalign_prehook(size_t alig, size_t sz)
{
	واپس mcheck_evaluate_memalign_سابقہ_سائز(alig) + sz + حجم(mcheck_canary);
}

ساکن خالی *mcheck_memalign_posthook(size_t alignment, خالی *altoghether_ptr, size_t customer_sz)
{
	واپس mcheck_allocated_helper(altoghether_ptr, customer_sz, alignment, CLEAN_CONTENT);
}

ساکن شمار mcheck_status mcheck_mprobe(خالی *ptr)
{
	ساخت mcheck_hdr *hdr = &((ساخت mcheck_hdr *)ptr)[-1];

	واپس mcheck_checkhdr(hdr);
}

ساکن خالی mcheck_pedantic_check(خالی)
{
	صحیح i;

	برائے (i = 0; i < REGISTRY_SZ; ++i)
		اگر (mcheck_registry[i])
			mcheck_checkhdr(mcheck_registry[i]);
}

ساکن خالی mcheck_pedantic_prehook(خالی)
{
	اگر (mcheck_pedantic_flag)
		mcheck_pedantic_check();
}

ساکن خالی mcheck_initialize(mcheck_abortfunc_t new_func, حرف pedantic_flag)
{
	mcheck_abortfunc = (new_func) ? new_func : &mcheck_default_abort;
	mcheck_pedantic_flag = pedantic_flag;
}

خالی mcheck_on_ramrelocation(size_t انحراف)
{
	حرف *p;
	صحیح i;
	// Simple, but inaccurate strategy: drop the pre-reloc heap
	برائے (i = 0; i < REGISTRY_SZ; ++i)
		اگر ((p = mcheck_registry[i]) != تہی ) {
			لکھو("mcheck, WRN: forgetting %p chunk\n", p);
			mcheck_registry[i] = 0;
		}

	mcheck_chunk_گنتی = 0;
}
#اگر_ختم
#اگر_ختم
