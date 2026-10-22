// SPDX-License-Identifier: GPL-2.0+
/*
 * Generic bounce buffer implementation
 *
 * Copyright (C) 2012 Marek Vasut <marex@denx.de>
 */

#ଅନ୍ତର୍ଭାଵ <cpu_func.h>
#ଅନ୍ତର୍ଭାଵ <log.h>
#ଅନ୍ତର୍ଭାଵ <malloc.h>
#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <bouncebuf.h>
#ଅନ୍ତର୍ଭାଵ <asm/cache.h>
#ଅନ୍ତର୍ଭାଵ <linux/dma-mapping.h>

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା addr_aligned(ସଂରଚନା bounce_buffer *state)
{
	ଅଚଳ ulong align_mask = ARCH_DMA_MINALIGN - 1;

	/* Check if start is aligned */
	ଯଦି ((ulong)state->user_buffer & align_mask) {
		debug("Unaligned buffer address %p\n", state->user_buffer);
		ଫେରନ୍ତୁ 0;
	}

	/* Check if length is aligned */
	ଯଦି (state->len != state->len_aligned) {
		debug("Unaligned buffer length %zu\n", state->len);
		ଫେରନ୍ତୁ 0;
	}

	/* Aligned */
	ଫେରନ୍ତୁ 1;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା bounce_buffer_start_extalign(ସଂରଚନା bounce_buffer *state, ଶୂନ୍ୟ *data,
				 size_t len, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags,
				 size_t alignment,
				 ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*addr_is_aligned)(ସଂରଚନା bounce_buffer *state))
{
	state->user_buffer = data;
	state->bounce_buffer = data;
	state->len = len;
	state->len_aligned = roundup(len, alignment);
	state->flags = flags;

	ଯଦି (!addr_is_aligned(state)) {
		state->bounce_buffer = memalign(alignment,
						state->len_aligned);
		ଯଦି (!state->bounce_buffer)
			ଫେରନ୍ତୁ -ENOMEM;

		ଯଦି (state->flags & GEN_BB_READ)
			memcpy(state->bounce_buffer, state->user_buffer,
				state->len);
	}

	/*
	 * Flush data to RAM so DMA reads can pick it up,
	 * and any CPU writebacks don't race with DMA writes
	 */
	dma_map_single(state->bounce_buffer,
		       state->len_aligned,
		       DMA_BIDIRECTIONAL);

	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା bounce_buffer_start(ସଂରଚନା bounce_buffer *state, ଶୂନ୍ୟ *data,
			size_t len, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags)
{
	ଫେରନ୍ତୁ bounce_buffer_start_extalign(state, data, len, flags,
					    ARCH_DMA_MINALIGN,
					    addr_aligned);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା bounce_buffer_stop(ସଂରଚନା bounce_buffer *state)
{
	ଯଦି (state->flags & GEN_BB_WRITE) {
		/* Invalidate cache so that CPU can see any newly DMA'd data */
		dma_unmap_single((dma_addr_t)(uintptr_t)state->bounce_buffer,
				 state->len_aligned,
				 DMA_BIDIRECTIONAL);
	}

	ଯଦି (state->bounce_buffer == state->user_buffer)
		ଫେରନ୍ତୁ 0;

	ଯଦି (state->flags & GEN_BB_WRITE)
		memcpy(state->user_buffer, state->bounce_buffer, state->len);

	ମୁକ୍ତ(state->bounce_buffer);

	ଫେରନ୍ତୁ 0;
}
