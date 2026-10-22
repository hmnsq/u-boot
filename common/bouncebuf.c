// SPDX-License-Identifier: GPL-2.0+
/*
 * Generic bounce buffer implementation
 *
 * Copyright (C) 2012 Marek Vasut <marex@denx.de>
 */

#include <cpu_func.h>
#include <log.h>
#include <malloc.h>
#include <errno.h>
#include <bouncebuf.h>
#include <asm/cache.h>
#include <linux/dma-mapping.h>

static số_nguyên addr_aligned(struct bounce_buffer *state)
{
	const ulong align_mask = ARCH_DMA_MINALIGN - 1;

	/* Check if start is aligned */
	nếu ((ulong)state->user_buffer & align_mask) {
		debug("Unaligned buffer address %p\n", state->user_buffer);
		trả_về 0;
	}

	/* Check if length is aligned */
	nếu (state->len != state->len_aligned) {
		debug("Unaligned buffer length %zu\n", state->len);
		trả_về 0;
	}

	/* Aligned */
	trả_về 1;
}

số_nguyên bounce_buffer_start_extalign(struct bounce_buffer *state, void *data,
				 size_t len, unsigned số_nguyên flags,
				 size_t alignment,
				 số_nguyên (*addr_is_aligned)(struct bounce_buffer *state))
{
	state->user_buffer = data;
	state->bounce_buffer = data;
	state->len = len;
	state->len_aligned = roundup(len, alignment);
	state->flags = flags;

	nếu (!addr_is_aligned(state)) {
		state->bounce_buffer = memalign(alignment,
						state->len_aligned);
		nếu (!state->bounce_buffer)
			trả_về -ENOMEM;

		nếu (state->flags & GEN_BB_READ)
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

	trả_về 0;
}

số_nguyên bounce_buffer_start(struct bounce_buffer *state, void *data,
			size_t len, unsigned số_nguyên flags)
{
	trả_về bounce_buffer_start_extalign(state, data, len, flags,
					    ARCH_DMA_MINALIGN,
					    addr_aligned);
}

số_nguyên bounce_buffer_stop(struct bounce_buffer *state)
{
	nếu (state->flags & GEN_BB_WRITE) {
		/* Invalidate cache so that CPU can see any newly DMA'd data */
		dma_unmap_single((dma_addr_t)(uintptr_t)state->bounce_buffer,
				 state->len_aligned,
				 DMA_BIDIRECTIONAL);
	}

	nếu (state->bounce_buffer == state->user_buffer)
		trả_về 0;

	nếu (state->flags & GEN_BB_WRITE)
		memcpy(state->user_buffer, state->bounce_buffer, state->len);

	free(state->bounce_buffer);

	trả_về 0;
}
