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

static ຈຳນວນເຕັມ addr_aligned(sa_nracanaa bounce_buffer *state)
{
	const ulong align_mask = ARCH_DMA_MINALIGN - 1;

	/* Check if start is aligned */
	ຖ້າ ((ulong)state->user_buffer & align_mask) {
		debug("Unaligned buffer address %p\n", state->user_buffer);
		ສົ່ງຄືນ 0;
	}

	/* Check if length is aligned */
	ຖ້າ (state->len != state->len_aligned) {
		debug("Unaligned buffer length %zu\n", state->len);
		ສົ່ງຄືນ 0;
	}

	/* Aligned */
	ສົ່ງຄືນ 1;
}

ຈຳນວນເຕັມ bounce_buffer_start_extalign(sa_nracanaa bounce_buffer *state, void *data,
				 size_t len, unsigned ຈຳນວນເຕັມ flags,
				 size_t alignment,
				 ຈຳນວນເຕັມ (*addr_is_aligned)(sa_nracanaa bounce_buffer *state))
{
	state->user_buffer = data;
	state->bounce_buffer = data;
	state->len = len;
	state->len_aligned = roundup(len, alignment);
	state->flags = flags;

	ຖ້າ (!addr_is_aligned(state)) {
		state->bounce_buffer = memalign(alignment,
						state->len_aligned);
		ຖ້າ (!state->bounce_buffer)
			ສົ່ງຄືນ -ENOMEM;

		ຖ້າ (state->flags & GEN_BB_READ)
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

	ສົ່ງຄືນ 0;
}

ຈຳນວນເຕັມ bounce_buffer_start(sa_nracanaa bounce_buffer *state, void *data,
			size_t len, unsigned ຈຳນວນເຕັມ flags)
{
	ສົ່ງຄືນ bounce_buffer_start_extalign(state, data, len, flags,
					    ARCH_DMA_MINALIGN,
					    addr_aligned);
}

ຈຳນວນເຕັມ bounce_buffer_stop(sa_nracanaa bounce_buffer *state)
{
	ຖ້າ (state->flags & GEN_BB_WRITE) {
		/* Invalidate cache so that CPU can see any newly DMA'd data */
		dma_unmap_single((dma_addr_t)(uintptr_t)state->bounce_buffer,
				 state->len_aligned,
				 DMA_BIDIRECTIONAL);
	}

	ຖ້າ (state->bounce_buffer == state->user_buffer)
		ສົ່ງຄືນ 0;

	ຖ້າ (state->flags & GEN_BB_WRITE)
		memcpy(state->user_buffer, state->bounce_buffer, state->len);

	free(state->bounce_buffer);

	ສົ່ງຄືນ 0;
}
