// SPDX-License-Identifier: GPL-2.0+
/*
 * Generic bounce buffer implementation
 *
 * Copyright (C) 2012 Marek Vasut <marex@denx.de>
 */

#ಅನ್ತರ್ಭಾವ <cpu_func.h>
#ಅನ್ತರ್ಭಾವ <log.h>
#ಅನ್ತರ್ಭಾವ <malloc.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <bouncebuf.h>
#ಅನ್ತರ್ಭಾವ <asm/cache.h>
#ಅನ್ತರ್ಭಾವ <linux/dma-mapping.h>

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ addr_aligned(ಸಂರಚನಾ bounce_buffer *state)
{
	ಧ್ರುವ ulong align_mask = ARCH_DMA_MINALIGN - 1;

	/* Check if start is aligned */
	ಯದಿ ((ulong)state->user_buffer & align_mask) {
		debug("Unaligned buffer address %p\n", state->user_buffer);
		ಪ್ರತ್ಯಾಗಮ 0;
	}

	/* Check if length is aligned */
	ಯದಿ (state->len != state->len_aligned) {
		debug("Unaligned buffer length %zu\n", state->len);
		ಪ್ರತ್ಯಾಗಮ 0;
	}

	/* Aligned */
	ಪ್ರತ್ಯಾಗಮ 1;
}

ಪೂರ್ಣಾಙ್ಕ bounce_buffer_start_extalign(ಸಂರಚನಾ bounce_buffer *state, ಶೂನ್ಯ *data,
				 size_t len, unsigned ಪೂರ್ಣಾಙ್ಕ flags,
				 size_t alignment,
				 ಪೂರ್ಣಾಙ್ಕ (*addr_is_aligned)(ಸಂರಚನಾ bounce_buffer *state))
{
	state->user_buffer = data;
	state->bounce_buffer = data;
	state->len = len;
	state->len_aligned = roundup(len, alignment);
	state->flags = flags;

	ಯದಿ (!addr_is_aligned(state)) {
		state->bounce_buffer = memalign(alignment,
						state->len_aligned);
		ಯದಿ (!state->bounce_buffer)
			ಪ್ರತ್ಯಾಗಮ -ENOMEM;

		ಯದಿ (state->flags & GEN_BB_READ)
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

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ bounce_buffer_start(ಸಂರಚನಾ bounce_buffer *state, ಶೂನ್ಯ *data,
			size_t len, unsigned ಪೂರ್ಣಾಙ್ಕ flags)
{
	ಪ್ರತ್ಯಾಗಮ bounce_buffer_start_extalign(state, data, len, flags,
					    ARCH_DMA_MINALIGN,
					    addr_aligned);
}

ಪೂರ್ಣಾಙ್ಕ bounce_buffer_stop(ಸಂರಚನಾ bounce_buffer *state)
{
	ಯದಿ (state->flags & GEN_BB_WRITE) {
		/* Invalidate cache so that CPU can see any newly DMA'd data */
		dma_unmap_single((dma_addr_t)(uintptr_t)state->bounce_buffer,
				 state->len_aligned,
				 DMA_BIDIRECTIONAL);
	}

	ಯದಿ (state->bounce_buffer == state->user_buffer)
		ಪ್ರತ್ಯಾಗಮ 0;

	ಯದಿ (state->flags & GEN_BB_WRITE)
		memcpy(state->user_buffer, state->bounce_buffer, state->len);

	ಮುಕ್ತ(state->bounce_buffer);

	ಪ್ರತ್ಯಾಗಮ 0;
}
