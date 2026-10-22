// SPDX-License-Identifier: GPL-2.0+
/*
 * Generic bounce buffer implementation
 *
 * Copyright (C) 2012 Marek Vasut <marex@denx.de>
 */

#ضمن <cpu_func.h>
#ضمن <log.h>
#ضمن <malloc.h>
#ضمن <errno.h>
#ضمن <bouncebuf.h>
#ضمن <asm/cache.h>
#ضمن <linux/dma-mapping.h>

ساكن صحيح addr_aligned(بنية bounce_buffer *state)
{
	ثابت ulong align_mask = ARCH_DMA_MINALIGN - 1;

	/* Check if start is aligned */
	𐤀𐤌 ((ulong)state->user_buffer & align_mask) {
		debug("Unaligned buffer address %p\n", state->user_buffer);
		أرجع 0;
	}

	/* Check if length is aligned */
	𐤀𐤌 (state->len != state->len_aligned) {
		debug("Unaligned buffer length %zu\n", state->len);
		أرجع 0;
	}

	/* Aligned */
	أرجع 1;
}

صحيح bounce_buffer_start_extalign(بنية bounce_buffer *state, فراغ *data,
				 size_t len, unsigned صحيح flags,
				 size_t alignment,
				 صحيح (*addr_is_aligned)(بنية bounce_buffer *state))
{
	state->user_buffer = data;
	state->bounce_buffer = data;
	state->len = len;
	state->len_aligned = roundup(len, alignment);
	state->flags = flags;

	𐤀𐤌 (!addr_is_aligned(state)) {
		state->bounce_buffer = memalign(alignment,
						state->len_aligned);
		𐤀𐤌 (!state->bounce_buffer)
			أرجع -ENOMEM;

		𐤀𐤌 (state->flags & GEN_BB_READ)
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

	أرجع 0;
}

صحيح bounce_buffer_start(بنية bounce_buffer *state, فراغ *data,
			size_t len, unsigned صحيح flags)
{
	أرجع bounce_buffer_start_extalign(state, data, len, flags,
					    ARCH_DMA_MINALIGN,
					    addr_aligned);
}

صحيح bounce_buffer_stop(بنية bounce_buffer *state)
{
	𐤀𐤌 (state->flags & GEN_BB_WRITE) {
		/* Invalidate cache so that CPU can see any newly DMA'd data */
		dma_unmap_single((dma_addr_t)(uintptr_t)state->bounce_buffer,
				 state->len_aligned,
				 DMA_BIDIRECTIONAL);
	}

	𐤀𐤌 (state->bounce_buffer == state->user_buffer)
		أرجع 0;

	𐤀𐤌 (state->flags & GEN_BB_WRITE)
		memcpy(state->user_buffer, state->bounce_buffer, state->len);

	حرر(state->bounce_buffer);

	أرجع 0;
}
