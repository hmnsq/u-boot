// SPDX-License-Identifier: GPL-2.0+
/*
 * Generic bounce buffer implementation
 *
 * Copyright (C) 2012 Marek Vasut <marex@denx.de>
 */

#അന്തര്ഭാവ <cpu_func.h>
#അന്തര്ഭാവ <log.h>
#അന്തര്ഭാവ <malloc.h>
#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <bouncebuf.h>
#അന്തര്ഭാവ <asm/cache.h>
#അന്തര്ഭാവ <linux/dma-mapping.h>

സ്ഥിതം പൂർണ്ണസംഖ്യ addr_aligned(ഘടന bounce_buffer *state)
{
	സ്ഥിരം ulong align_mask = ARCH_DMA_MINALIGN - 1;

	/* Check if start is aligned */
	എങ്കിൽ ((ulong)state->user_buffer & align_mask) {
		debug("Unaligned buffer address %p\n", state->user_buffer);
		തിരികെ 0;
	}

	/* Check if length is aligned */
	എങ്കിൽ (state->len != state->len_aligned) {
		debug("Unaligned buffer length %zu\n", state->len);
		തിരികെ 0;
	}

	/* Aligned */
	തിരികെ 1;
}

പൂർണ്ണസംഖ്യ bounce_buffer_start_extalign(ഘടന bounce_buffer *state, ശൂന്യം *data,
				 size_t len, unsigned പൂർണ്ണസംഖ്യ flags,
				 size_t alignment,
				 പൂർണ്ണസംഖ്യ (*addr_is_aligned)(ഘടന bounce_buffer *state))
{
	state->user_buffer = data;
	state->bounce_buffer = data;
	state->len = len;
	state->len_aligned = roundup(len, alignment);
	state->flags = flags;

	എങ്കിൽ (!addr_is_aligned(state)) {
		state->bounce_buffer = memalign(alignment,
						state->len_aligned);
		എങ്കിൽ (!state->bounce_buffer)
			തിരികെ -ENOMEM;

		എങ്കിൽ (state->flags & GEN_BB_READ)
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

	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ bounce_buffer_start(ഘടന bounce_buffer *state, ശൂന്യം *data,
			size_t len, unsigned പൂർണ്ണസംഖ്യ flags)
{
	തിരികെ bounce_buffer_start_extalign(state, data, len, flags,
					    ARCH_DMA_MINALIGN,
					    addr_aligned);
}

പൂർണ്ണസംഖ്യ bounce_buffer_stop(ഘടന bounce_buffer *state)
{
	എങ്കിൽ (state->flags & GEN_BB_WRITE) {
		/* Invalidate cache so that CPU can see any newly DMA'd data */
		dma_unmap_single((dma_addr_t)(uintptr_t)state->bounce_buffer,
				 state->len_aligned,
				 DMA_BIDIRECTIONAL);
	}

	എങ്കിൽ (state->bounce_buffer == state->user_buffer)
		തിരികെ 0;

	എങ്കിൽ (state->flags & GEN_BB_WRITE)
		memcpy(state->user_buffer, state->bounce_buffer, state->len);

	സ്വതന്ത്രം(state->bounce_buffer);

	തിരികെ 0;
}
