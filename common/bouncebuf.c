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

static 整数 addr_aligned(struct bounce_buffer *state)
{
	const ulong align_mask = ARCH_DMA_MINALIGN - 1;

	/* Check if start is aligned */
	如果 ((ulong)state->user_buffer & align_mask) {
		debug("Unaligned buffer address %p\n", state->user_buffer);
		返回 0;
	}

	/* Check if length is aligned */
	如果 (state->len != state->len_aligned) {
		debug("Unaligned buffer length %zu\n", state->len);
		返回 0;
	}

	/* Aligned */
	返回 1;
}

整数 bounce_buffer_start_extalign(struct bounce_buffer *state, void *data,
				 size_t len, unsigned 整数 flags,
				 size_t alignment,
				 整数 (*addr_is_aligned)(struct bounce_buffer *state))
{
	state->user_buffer = data;
	state->bounce_buffer = data;
	state->len = len;
	state->len_aligned = roundup(len, alignment);
	state->flags = flags;

	如果 (!addr_is_aligned(state)) {
		state->bounce_buffer = memalign(alignment,
						state->len_aligned);
		如果 (!state->bounce_buffer)
			返回 -ENOMEM;

		如果 (state->flags & GEN_BB_READ)
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

	返回 0;
}

整数 bounce_buffer_start(struct bounce_buffer *state, void *data,
			size_t len, unsigned 整数 flags)
{
	返回 bounce_buffer_start_extalign(state, data, len, flags,
					    ARCH_DMA_MINALIGN,
					    addr_aligned);
}

整数 bounce_buffer_stop(struct bounce_buffer *state)
{
	如果 (state->flags & GEN_BB_WRITE) {
		/* Invalidate cache so that CPU can see any newly DMA'd data */
		dma_unmap_single((dma_addr_t)(uintptr_t)state->bounce_buffer,
				 state->len_aligned,
				 DMA_BIDIRECTIONAL);
	}

	如果 (state->bounce_buffer == state->user_buffer)
		返回 0;

	如果 (state->flags & GEN_BB_WRITE)
		memcpy(state->user_buffer, state->bounce_buffer, state->len);

	free(state->bounce_buffer);

	返回 0;
}
