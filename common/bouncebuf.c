// SPDX-License-Identifier: GPL-2.0+
/*
 * Generic bounce buffer implementation
 *
 * Copyright (C) 2012 Marek Vasut <marex@denx.de>
 */

#شامل <cpu_func.h>
#شامل <log.h>
#شامل <malloc.h>
#شامل <errno.h>
#شامل <bouncebuf.h>
#شامل <asm/cache.h>
#شامل <linux/dma-mapping.h>

ایستا صحیح addr_aligned(ساختار bounce_buffer *state)
{
	ثابت ulong align_mask = ARCH_DMA_MINALIGN - 1;

	/* Check if start is aligned */
	اگر ((ulong)state->user_buffer & align_mask) {
		debug("Unaligned buffer address %p\n", state->user_buffer);
		بازگشت 0;
	}

	/* Check if length is aligned */
	اگر (state->len != state->len_aligned) {
		debug("Unaligned buffer length %zu\n", state->len);
		بازگشت 0;
	}

	/* Aligned */
	بازگشت 1;
}

صحیح bounce_buffer_start_extalign(ساختار bounce_buffer *state, تهی *data,
				 size_t len, unsigned صحیح flags,
				 size_t alignment,
				 صحیح (*addr_is_aligned)(ساختار bounce_buffer *state))
{
	state->user_buffer = data;
	state->bounce_buffer = data;
	state->len = len;
	state->len_aligned = roundup(len, alignment);
	state->flags = flags;

	اگر (!addr_is_aligned(state)) {
		state->bounce_buffer = memalign(alignment,
						state->len_aligned);
		اگر (!state->bounce_buffer)
			بازگشت -ENOMEM;

		اگر (state->flags & GEN_BB_READ)
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

	بازگشت 0;
}

صحیح bounce_buffer_start(ساختار bounce_buffer *state, تهی *data,
			size_t len, unsigned صحیح flags)
{
	بازگشت bounce_buffer_start_extalign(state, data, len, flags,
					    ARCH_DMA_MINALIGN,
					    addr_aligned);
}

صحیح bounce_buffer_stop(ساختار bounce_buffer *state)
{
	اگر (state->flags & GEN_BB_WRITE) {
		/* Invalidate cache so that CPU can see any newly DMA'd data */
		dma_unmap_single((dma_addr_t)(uintptr_t)state->bounce_buffer,
				 state->len_aligned,
				 DMA_BIDIRECTIONAL);
	}

	اگر (state->bounce_buffer == state->user_buffer)
		بازگشت 0;

	اگر (state->flags & GEN_BB_WRITE)
		memcpy(state->user_buffer, state->bounce_buffer, state->len);

	آزاد(state->bounce_buffer);

	بازگشت 0;
}
