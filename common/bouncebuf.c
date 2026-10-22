// SPDX-License-Identifier: GPL-2.0+
/*
 * Generic bounce buffer implementation
 *
 * Copyright (C) 2012 Marek Vasut <marex@denx.de>
 */

#समावेश <cpu_func.h>
#समावेश <log.h>
#समावेश <malloc.h>
#समावेश <errno.h>
#समावेश <bouncebuf.h>
#समावेश <asm/cache.h>
#समावेश <linux/dma-mapping.h>

अटल पूर्णांक addr_aligned(काष्ठा bounce_बफर *state)
{
	स्थिर ulong align_mask = ARCH_DMA_MINALIGN - 1;

	/* Check if start is aligned */
	अगर ((ulong)state->user_बफर & align_mask) {
		debug("Unaligned buffer address %p\n", state->user_बफर);
		वापस 0;
	}

	/* Check if length is aligned */
	अगर (state->len != state->len_aligned) {
		debug("Unaligned buffer length %zu\n", state->len);
		वापस 0;
	}

	/* Aligned */
	वापस 1;
}

पूर्णांक bounce_बफर_start_extalign(काष्ठा bounce_बफर *state, व्योम *तथ्य,
				 माप_प्रकार len, अचिन्हित पूर्णांक flags,
				 माप_प्रकार alignment,
				 पूर्णांक (*addr_is_aligned)(काष्ठा bounce_बफर *state))
{
	state->user_बफर = तथ्य;
	state->bounce_बफर = तथ्य;
	state->len = len;
	state->len_aligned = roundup(len, alignment);
	state->flags = flags;

	अगर (!addr_is_aligned(state)) {
		state->bounce_बफर = memalign(alignment,
						state->len_aligned);
		अगर (!state->bounce_बफर)
			वापस -ENOMEM;

		अगर (state->flags & GEN_BB_READ)
			स_नकल(state->bounce_बफर, state->user_बफर,
				state->len);
	}

	/*
	 * Flush data to RAM so DMA reads can pick it up,
	 * and any CPU writebacks don't race with DMA writes
	 */
	dma_map_single(state->bounce_बफर,
		       state->len_aligned,
		       DMA_BIDIRECTIONAL);

	वापस 0;
}

पूर्णांक bounce_बफर_start(काष्ठा bounce_बफर *state, व्योम *तथ्य,
			माप_प्रकार len, अचिन्हित पूर्णांक flags)
{
	वापस bounce_बफर_start_extalign(state, तथ्य, len, flags,
					    ARCH_DMA_MINALIGN,
					    addr_aligned);
}

पूर्णांक bounce_बफर_stop(काष्ठा bounce_बफर *state)
{
	अगर (state->flags & GEN_BB_WRITE) {
		/* Invalidate cache so that CPU can see any newly DMA'd data */
		dma_unmap_single((dma_addr_t)(uintptr_t)state->bounce_बफर,
				 state->len_aligned,
				 DMA_BIDIRECTIONAL);
	}

	अगर (state->bounce_बफर == state->user_बफर)
		वापस 0;

	अगर (state->flags & GEN_BB_WRITE)
		स_नकल(state->user_बफर, state->bounce_बफर, state->len);

	मुक्त(state->bounce_बफर);

	वापस 0;
}
