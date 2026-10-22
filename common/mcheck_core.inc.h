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
#अगर_अघोषित _MCHECKCORE_INC_H
#घोषणा _MCHECKCORE_INC_H      1
#समावेश "mcheck.h"

#अगर defined(MCHECK_HEAP_PROTECTION)
#घोषणा mcheck_flood स_रखो

// these are from /dev/random:
#घोषणा MAGICWORD	0x99ccf430fa562a05ULL
#घोषणा MAGICFREE	0x4875e63c0c6fc08eULL
#घोषणा MAGICTAIL	0x918dbcd7df78dcd6ULL
#घोषणा MALLOCFLOOD	((अक्षर)0xb6)
#घोषणा FREEFLOOD	((अक्षर)0xf5)
#घोषणा PADDINGFLOOD	((अक्षर)0x58)

// my normal run demands 4427-6449 chunks:
#घोषणा REGISTRY_SZ	6608
#घोषणा CANARY_DEPTH	2

// avoid problems with BSS at early stage:
अटल अक्षर mcheck_pedantic_flag __section(".data") = 0;
अटल व्योम *mcheck_registry[REGISTRY_SZ] __section(".data") = {0};
अटल माप_प्रकार mcheck_chunk_गिनती __section(".data") = 0;
अटल माप_प्रकार mcheck_chunk_गिनती_max __section(".data") = 0;

प्रकार अचिन्हित दीर्घ दीर्घ mcheck_elem;
प्रकार काष्ठा {
	mcheck_elem elems[CANARY_DEPTH];
} mcheck_canary;
काष्ठा mcheck_hdr {
	माप_प्रकार आकार; /* Exact size requested by user.  */
	माप_प्रकार aln_skip; /* Ignored bytes, before the mcheck_hdr, to fulfill alignment */
	mcheck_canary canary; /* Magic number to check header integrity.  */
};

अटल व्योम mcheck_default_abort(क्रमागत mcheck_status status, स्थिर व्योम *p)
{
	स्थिर अक्षर *संदेश;

	चयन (status) {
	हाल MCHECK_OK:
		संदेश = "memory is consistent, library is buggy\n";
		अवरोध;
	हाल MCHECK_HEAD:
		संदेश = "memory clobbered before allocated block\n";
		अवरोध;
	हाल MCHECK_TAIL:
		संदेश = "memory clobbered past end of allocated block\n";
		अवरोध;
	हाल MCHECK_FREE:
		संदेश = "block freed twice\n";
		अवरोध;
	शेष:
		संदेश = "bogus mcheck_status, library is buggy\n";
		अवरोध;
	}
	म_लिखो("\n\nmcheck: %p:%s!!! [%zu]\n\n", p, संदेश, mcheck_chunk_गिनती_max);
}

अटल mcheck_abortfunc_t mcheck_abortfunc = &mcheck_default_abort;

अटल अंतरभूत माप_प्रकार allign_आकार_up(माप_प्रकार sz, माप_प्रकार grain)
{
	वापस (sz + grain - 1) & ~(grain - 1);
}

#घोषणा mcheck_allign_customer_आकार(SZ) allign_आकार_up(SZ, माप(mcheck_elem))
#घोषणा mcheck_evaluate_memalign_prefix_आकार(ALIGN) allign_आकार_up(माप(काष्ठा mcheck_hdr), ALIGN)

अटल क्रमागत mcheck_status mcheck_OnNok(क्रमागत mcheck_status status, स्थिर व्योम *p)
{
	(*mcheck_abortfunc)(status, p);
	वापस status;
}

अटल क्रमागत mcheck_status mcheck_checkhdr(स्थिर काष्ठा mcheck_hdr *hdr)
{
	पूर्णांक i;

	क्रम (i = 0; i < CANARY_DEPTH; ++i)
		अगर (hdr->canary.elems[i] == MAGICFREE)
			वापस mcheck_OnNok(MCHECK_FREE, hdr + 1);

	क्रम (i = 0; i < CANARY_DEPTH; ++i)
		अगर (hdr->canary.elems[i] != MAGICWORD)
			वापस mcheck_OnNok(MCHECK_HEAD, hdr + 1);

	स्थिर माप_प्रकार payload_आकार = hdr->आकार;
	स्थिर माप_प्रकार payload_आकार_aligned = mcheck_allign_customer_आकार(payload_आकार);
	स्थिर माप_प्रकार padd_आकार = payload_आकार_aligned - hdr->आकार;

	स्थिर अक्षर *payload = (स्थिर अक्षर *)&hdr[1];

	क्रम (i = 0; i < padd_आकार; ++i)
		अगर (payload[payload_आकार + i] != PADDINGFLOOD)
			वापस mcheck_OnNok(MCHECK_TAIL, hdr + 1);

	स्थिर mcheck_canary *पुच्छ = (स्थिर mcheck_canary *)&payload[payload_आकार_aligned];

	क्रम (i = 0; i < CANARY_DEPTH; ++i)
		अगर (पुच्छ->elems[i] != MAGICTAIL)
			वापस mcheck_OnNok(MCHECK_TAIL, hdr + 1);
	वापस MCHECK_OK;
}

क्रमागत { KEEP_CONTENT = 0, CLEAN_CONTENT, ANY_ALIGNMENT = 1 };
अटल व्योम *mcheck_free_helper(व्योम *ptr, पूर्णांक clean_content)
{
	अगर (!ptr)
		वापस ptr;

	काष्ठा mcheck_hdr *hdr = &((काष्ठा mcheck_hdr *)ptr)[-1];
	पूर्णांक i;

	mcheck_checkhdr(hdr);
	क्रम (i = 0; i < CANARY_DEPTH; ++i)
		hdr->canary.elems[i] = MAGICFREE;

	अगर (clean_content)
		mcheck_flood(ptr, FREEFLOOD, mcheck_allign_customer_आकार(hdr->आकार));

	क्रम (i = 0; i < REGISTRY_SZ; ++i)
		अगर (mcheck_registry[i] == hdr) {
			mcheck_registry[i] = 0;
			अवरोध;
		}

	--mcheck_chunk_गिनती;
	वापस (अक्षर *)hdr - hdr->aln_skip;
}

अटल व्योम *mcheck_free_prehook(व्योम *ptr) { वापस mcheck_free_helper(ptr, CLEAN_CONTENT); }
अटल व्योम *mcheck_reallocfree_prehook(व्योम *ptr) { वापस mcheck_free_helper(ptr, KEEP_CONTENT); }

अटल माप_प्रकार mcheck_alloc_prehook(माप_प्रकार sz)
{
	sz = mcheck_allign_customer_आकार(sz);
	वापस माप(काष्ठा mcheck_hdr) + sz + माप(mcheck_canary);
}

अटल व्योम *mcheck_allocated_helper(व्योम *altoghether_ptr, माप_प्रकार customer_sz,
				     माप_प्रकार alignment, पूर्णांक clean_content)
{
	स्थिर माप_प्रकार slop = alignment ?
		mcheck_evaluate_memalign_prefix_आकार(alignment) - माप(काष्ठा mcheck_hdr) : 0;
	काष्ठा mcheck_hdr *hdr = (काष्ठा mcheck_hdr *)((अक्षर *)altoghether_ptr + slop);
	पूर्णांक i;

	hdr->आकार = customer_sz;
	hdr->aln_skip = slop;
	क्रम (i = 0; i < CANARY_DEPTH; ++i)
		hdr->canary.elems[i] = MAGICWORD;

	अक्षर *payload = (अक्षर *)&hdr[1];

	अगर (clean_content)
		mcheck_flood(payload, MALLOCFLOOD, customer_sz);

	स्थिर माप_प्रकार customer_आकार_aligned = mcheck_allign_customer_आकार(customer_sz);

	mcheck_flood(payload + customer_sz, PADDINGFLOOD, customer_आकार_aligned - customer_sz);

	mcheck_canary *पुच्छ = (mcheck_canary *)&payload[customer_आकार_aligned];

	क्रम (i = 0; i < CANARY_DEPTH; ++i)
		पुच्छ->elems[i] = MAGICTAIL;

	++mcheck_chunk_गिनती;
	अगर (mcheck_chunk_गिनती > mcheck_chunk_गिनती_max)
		mcheck_chunk_गिनती_max = mcheck_chunk_गिनती;

	क्रम (i = 0; i < REGISTRY_SZ; ++i)
		अगर (!mcheck_registry[i]) {
			mcheck_registry[i] = hdr;
			वापस payload; // normal end
		}

	अटल अक्षर *overflow_संदेश = "\n\n\nERROR: mcheck registry overflow, pedantic check would be incomplete!!\n\n\n\n";

	म_लिखो("%s", overflow_संदेश);
	overflow_संदेश = "(mcheck registry full)";
	वापस payload;
}

अटल व्योम *mcheck_alloc_posthook(व्योम *altoghether_ptr, माप_प्रकार customer_sz)
{
	वापस mcheck_allocated_helper(altoghether_ptr, customer_sz, ANY_ALIGNMENT, CLEAN_CONTENT);
}

अटल व्योम *mcheck_alloc_noclean_posthook(व्योम *altoghether_ptr, माप_प्रकार customer_sz)
{
	वापस mcheck_allocated_helper(altoghether_ptr, customer_sz, ANY_ALIGNMENT, KEEP_CONTENT);
}

अटल माप_प्रकार mcheck_memalign_prehook(माप_प्रकार alig, माप_प्रकार sz)
{
	वापस mcheck_evaluate_memalign_prefix_आकार(alig) + sz + माप(mcheck_canary);
}

अटल व्योम *mcheck_memalign_posthook(माप_प्रकार alignment, व्योम *altoghether_ptr, माप_प्रकार customer_sz)
{
	वापस mcheck_allocated_helper(altoghether_ptr, customer_sz, alignment, CLEAN_CONTENT);
}

अटल क्रमागत mcheck_status mcheck_mprobe(व्योम *ptr)
{
	काष्ठा mcheck_hdr *hdr = &((काष्ठा mcheck_hdr *)ptr)[-1];

	वापस mcheck_checkhdr(hdr);
}

अटल व्योम mcheck_pedantic_check(व्योम)
{
	पूर्णांक i;

	क्रम (i = 0; i < REGISTRY_SZ; ++i)
		अगर (mcheck_registry[i])
			mcheck_checkhdr(mcheck_registry[i]);
}

अटल व्योम mcheck_pedantic_prehook(व्योम)
{
	अगर (mcheck_pedantic_flag)
		mcheck_pedantic_check();
}

अटल व्योम mcheck_initialize(mcheck_abortfunc_t new_func, अक्षर pedantic_flag)
{
	mcheck_abortfunc = (new_func) ? new_func : &mcheck_default_abort;
	mcheck_pedantic_flag = pedantic_flag;
}

व्योम mcheck_on_ramrelocation(माप_प्रकार offset)
{
	अक्षर *p;
	पूर्णांक i;
	// Simple, but inaccurate strategy: drop the pre-reloc heap
	क्रम (i = 0; i < REGISTRY_SZ; ++i)
		अगर ((p = mcheck_registry[i]) != शून्य ) {
			म_लिखो("mcheck, WRN: forgetting %p chunk\n", p);
			mcheck_registry[i] = 0;
		}

	mcheck_chunk_गिनती = 0;
}
#पूर्ण_अगर
#पूर्ण_अगर
