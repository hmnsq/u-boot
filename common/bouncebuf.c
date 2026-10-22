// SPDX-License-Identifier: GPL-2.0+
/*
 * Generic bounce buffer implementation
 *
 * Copyright (C) 2012 Marek Vasut <marex@denx.de>
 */

#அந்தர்பாவ <cpu_func.h>
#அந்தர்பாவ <log.h>
#அந்தர்பாவ <malloc.h>
#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <bouncebuf.h>
#அந்தர்பாவ <asm/cache.h>
#அந்தர்பாவ <linux/dma-mapping.h>

நிலையான முழு addr_aligned(கட்டமைப்பு bounce_buffer *state)
{
	மாறிலி ulong align_mask = ARCH_DMA_MINALIGN - 1;

	/* Check if start is aligned */
	எனில் ((ulong)state->user_buffer & align_mask) {
		debug("Unaligned buffer address %p\n", state->user_buffer);
		திரும்பு 0;
	}

	/* Check if length is aligned */
	எனில் (state->len != state->len_aligned) {
		debug("Unaligned buffer length %zu\n", state->len);
		திரும்பு 0;
	}

	/* Aligned */
	திரும்பு 1;
}

முழு bounce_buffer_start_extalign(கட்டமைப்பு bounce_buffer *state, வெறுமை *data,
				 size_t len, unsigned முழு flags,
				 size_t alignment,
				 முழு (*addr_is_aligned)(கட்டமைப்பு bounce_buffer *state))
{
	state->user_buffer = data;
	state->bounce_buffer = data;
	state->len = len;
	state->len_aligned = roundup(len, alignment);
	state->flags = flags;

	எனில் (!addr_is_aligned(state)) {
		state->bounce_buffer = memalign(alignment,
						state->len_aligned);
		எனில் (!state->bounce_buffer)
			திரும்பு -ENOMEM;

		எனில் (state->flags & GEN_BB_READ)
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

	திரும்பு 0;
}

முழு bounce_buffer_start(கட்டமைப்பு bounce_buffer *state, வெறுமை *data,
			size_t len, unsigned முழு flags)
{
	திரும்பு bounce_buffer_start_extalign(state, data, len, flags,
					    ARCH_DMA_MINALIGN,
					    addr_aligned);
}

முழு bounce_buffer_stop(கட்டமைப்பு bounce_buffer *state)
{
	எனில் (state->flags & GEN_BB_WRITE) {
		/* Invalidate cache so that CPU can see any newly DMA'd data */
		dma_unmap_single((dma_addr_t)(uintptr_t)state->bounce_buffer,
				 state->len_aligned,
				 DMA_BIDIRECTIONAL);
	}

	எனில் (state->bounce_buffer == state->user_buffer)
		திரும்பு 0;

	எனில் (state->flags & GEN_BB_WRITE)
		memcpy(state->user_buffer, state->bounce_buffer, state->len);

	விடுவி(state->bounce_buffer);

	திரும்பு 0;
}
