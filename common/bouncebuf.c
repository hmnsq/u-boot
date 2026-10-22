// SPDX-License-Identifier: GPL-2.0+
/*
 * Generic bounce buffer implementation
 *
 * Copyright (C) 2012 Marek Vasut <marex@denx.de>
 */

#অন্তর্ভাব <cpu_func.h>
#অন্তর্ভাব <log.h>
#অন্তর্ভাব <malloc.h>
#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <bouncebuf.h>
#অন্তর্ভাব <asm/cache.h>
#অন্তর্ভাব <linux/dma-mapping.h>

স্থির পূর্ণাঙ্ক addr_aligned(সংরচনা bounce_buffer *state)
{
	ধ্রুব ulong align_mask = ARCH_DMA_MINALIGN - 1;

	/* Check if start is aligned */
	যদি ((ulong)state->user_buffer & align_mask) {
		debug("Unaligned buffer address %p\n", state->user_buffer);
		প্রত্যাগম 0;
	}

	/* Check if length is aligned */
	যদি (state->len != state->len_aligned) {
		debug("Unaligned buffer length %zu\n", state->len);
		প্রত্যাগম 0;
	}

	/* Aligned */
	প্রত্যাগম 1;
}

পূর্ণাঙ্ক bounce_buffer_start_extalign(সংরচনা bounce_buffer *state, শূন্য *data,
				 size_t len, unsigned পূর্ণাঙ্ক flags,
				 size_t alignment,
				 পূর্ণাঙ্ক (*addr_is_aligned)(সংরচনা bounce_buffer *state))
{
	state->user_buffer = data;
	state->bounce_buffer = data;
	state->len = len;
	state->len_aligned = roundup(len, alignment);
	state->flags = flags;

	যদি (!addr_is_aligned(state)) {
		state->bounce_buffer = memalign(alignment,
						state->len_aligned);
		যদি (!state->bounce_buffer)
			প্রত্যাগম -ENOMEM;

		যদি (state->flags & GEN_BB_READ)
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

	প্রত্যাগম 0;
}

পূর্ণাঙ্ক bounce_buffer_start(সংরচনা bounce_buffer *state, শূন্য *data,
			size_t len, unsigned পূর্ণাঙ্ক flags)
{
	প্রত্যাগম bounce_buffer_start_extalign(state, data, len, flags,
					    ARCH_DMA_MINALIGN,
					    addr_aligned);
}

পূর্ণাঙ্ক bounce_buffer_stop(সংরচনা bounce_buffer *state)
{
	যদি (state->flags & GEN_BB_WRITE) {
		/* Invalidate cache so that CPU can see any newly DMA'd data */
		dma_unmap_single((dma_addr_t)(uintptr_t)state->bounce_buffer,
				 state->len_aligned,
				 DMA_BIDIRECTIONAL);
	}

	যদি (state->bounce_buffer == state->user_buffer)
		প্রত্যাগম 0;

	যদি (state->flags & GEN_BB_WRITE)
		memcpy(state->user_buffer, state->bounce_buffer, state->len);

	মুক্ত(state->bounce_buffer);

	প্রত্যাগম 0;
}
