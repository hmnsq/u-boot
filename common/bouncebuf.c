// SPDX-License-Identifier: GPL-2.0+
/*
 * Generic bounce buffer implementation
 *
 * Copyright (C) 2012 Marek Vasut <marex@denx.de>
 */

#अन्तर्भाव <cpu_func.h>
#अन्तर्भाव <log.h>
#अन्तर्भाव <malloc.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <bouncebuf.h>
#अन्तर्भाव <asm/cache.h>
#अन्तर्भाव <linux/dma-mapping.h>

स्थिर पूर्णाङ्क addr_aligned(संरचना bounce_बफर *state)
{
	ध्रुव ulong align_आवरणम् = ARCH_DMA_MINALIGN - 1;

	/* Check if start is aligned */
	यदि ((ulong)state->user_बफर & align_आवरणम्) {
		debug("Unaligned buffer address %p\n", state->user_बफर);
		प्रत्यागम 0;
	}

	/* Check if length is aligned */
	यदि (state->len != state->len_aligned) {
		debug("Unaligned buffer length %zu\n", state->len);
		प्रत्यागम 0;
	}

	/* Aligned */
	प्रत्यागम 1;
}

पूर्णाङ्क bounce_बफर_start_extalign(संरचना bounce_बफर *state, शून्य *दत्त,
				 size_t len, अचिह्नित पूर्णाङ्क flags,
				 size_t alignment,
				 पूर्णाङ्क (*addr_is_aligned)(संरचना bounce_बफर *state))
{
	state->user_बफर = दत्त;
	state->bounce_बफर = दत्त;
	state->len = len;
	state->len_aligned = roundup(len, alignment);
	state->flags = flags;

	यदि (!addr_is_aligned(state)) {
		state->bounce_बफर = memalign(alignment,
						state->len_aligned);
		यदि (!state->bounce_बफर)
			प्रत्यागम -ENOMEM;

		यदि (state->flags & GEN_BB_READ)
			स्मृतिं_प्रतिलिख(state->bounce_बफर, state->user_बफर,
				state->len);
	}

	/*
	 * Flush data to RAM so DMA reads can pick it up,
	 * and any CPU writebacks don't race with DMA writes
	 */
	dma_map_single(state->bounce_बफर,
		       state->len_aligned,
		       DMA_BIDIRECTIONAL);

	प्रत्यागम 0;
}

पूर्णाङ्क bounce_बफर_start(संरचना bounce_बफर *state, शून्य *दत्त,
			size_t len, अचिह्नित पूर्णाङ्क flags)
{
	प्रत्यागम bounce_बफर_start_extalign(state, दत्त, len, flags,
					    ARCH_DMA_MINALIGN,
					    addr_aligned);
}

पूर्णाङ्क bounce_बफर_stop(संरचना bounce_बफर *state)
{
	यदि (state->flags & GEN_BB_WRITE) {
		/* Invalidate cache so that CPU can see any newly DMA'd data */
		dma_unmap_single((dma_addr_t)(uintptr_t)state->bounce_बफर,
				 state->len_aligned,
				 DMA_BIDIRECTIONAL);
	}

	यदि (state->bounce_बफर == state->user_बफर)
		प्रत्यागम 0;

	यदि (state->flags & GEN_BB_WRITE)
		स्मृतिं_प्रतिलिख(state->user_बफर, state->bounce_बफर, state->len);

	मुक्त(state->bounce_बफर);

	प्रत्यागम 0;
}
