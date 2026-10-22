// SPDX-License-Identifier: GPL-2.0+
/*
 * Generic bounce buffer implementation
 *
 * Copyright (C) 2012 Marek Vasut <marex@denx.de>
 */

#અન્તર્ભાવ <cpu_func.h>
#અન્તર્ભાવ <log.h>
#અન્તર્ભાવ <malloc.h>
#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <bouncebuf.h>
#અન્તર્ભાવ <asm/cache.h>
#અન્તર્ભાવ <linux/dma-mapping.h>

સ્થિર પૂર્ણાંક addr_aligned(સંરચના bounce_buffer *state)
{
	અચલ ulong align_mask = ARCH_DMA_MINALIGN - 1;

	/* Check if start is aligned */
	જો ((ulong)state->user_buffer & align_mask) {
		debug("Unaligned buffer address %p\n", state->user_buffer);
		પાછા 0;
	}

	/* Check if length is aligned */
	જો (state->len != state->len_aligned) {
		debug("Unaligned buffer length %zu\n", state->len);
		પાછા 0;
	}

	/* Aligned */
	પાછા 1;
}

પૂર્ણાંક bounce_buffer_start_extalign(સંરચના bounce_buffer *state, શૂન્ય *data,
				 size_t len, unsigned પૂર્ણાંક flags,
				 size_t alignment,
				 પૂર્ણાંક (*addr_is_aligned)(સંરચના bounce_buffer *state))
{
	state->user_buffer = data;
	state->bounce_buffer = data;
	state->len = len;
	state->len_aligned = roundup(len, alignment);
	state->flags = flags;

	જો (!addr_is_aligned(state)) {
		state->bounce_buffer = memalign(alignment,
						state->len_aligned);
		જો (!state->bounce_buffer)
			પાછા -ENOMEM;

		જો (state->flags & GEN_BB_READ)
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

	પાછા 0;
}

પૂર્ણાંક bounce_buffer_start(સંરચના bounce_buffer *state, શૂન્ય *data,
			size_t len, unsigned પૂર્ણાંક flags)
{
	પાછા bounce_buffer_start_extalign(state, data, len, flags,
					    ARCH_DMA_MINALIGN,
					    addr_aligned);
}

પૂર્ણાંક bounce_buffer_stop(સંરચના bounce_buffer *state)
{
	જો (state->flags & GEN_BB_WRITE) {
		/* Invalidate cache so that CPU can see any newly DMA'd data */
		dma_unmap_single((dma_addr_t)(uintptr_t)state->bounce_buffer,
				 state->len_aligned,
				 DMA_BIDIRECTIONAL);
	}

	જો (state->bounce_buffer == state->user_buffer)
		પાછા 0;

	જો (state->flags & GEN_BB_WRITE)
		memcpy(state->user_buffer, state->bounce_buffer, state->len);

	મુક્ત(state->bounce_buffer);

	પાછા 0;
}
