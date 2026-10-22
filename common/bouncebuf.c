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

ساکن صحیح addr_aligned(ساخت bounce_بفر *state)
{
	ثابت ulong align_نقاب = ARCH_DMA_MINALIGN - 1;

	/* Check if start is aligned */
	اگر ((ulong)state->user_بفر & align_نقاب) {
		debug("Unaligned buffer address %p\n", state->user_بفر);
		واپس 0;
	}

	/* Check if length is aligned */
	اگر (state->len != state->len_aligned) {
		debug("Unaligned buffer length %zu\n", state->len);
		واپس 0;
	}

	/* Aligned */
	واپس 1;
}

صحیح bounce_بفر_آغاز_extalign(ساخت bounce_بفر *state, خالی *ڈیٹا,
				 size_t len, غیرعلامتی صحیح flags,
				 size_t alignment,
				 صحیح (*addr_is_aligned)(ساخت bounce_بفر *state))
{
	state->user_بفر = ڈیٹا;
	state->bounce_بفر = ڈیٹا;
	state->len = len;
	state->len_aligned = roundup(len, alignment);
	state->flags = flags;

	اگر (!addr_is_aligned(state)) {
		state->bounce_بفر = memalign(alignment,
						state->len_aligned);
		اگر (!state->bounce_بفر)
			واپس -ENOMEM;

		اگر (state->flags & GEN_BB_READ)
			حافظہ_نقل(state->bounce_بفر, state->user_بفر,
				state->len);
	}

	/*
	 * Flush data to RAM so DMA reads can pick it up,
	 * and any CPU writebacks don't race with DMA writes
	 */
	dma_map_single(state->bounce_بفر,
		       state->len_aligned,
		       DMA_BIDIRECTIONAL);

	واپس 0;
}

صحیح bounce_بفر_آغاز(ساخت bounce_بفر *state, خالی *ڈیٹا,
			size_t len, غیرعلامتی صحیح flags)
{
	واپس bounce_بفر_آغاز_extalign(state, ڈیٹا, len, flags,
					    ARCH_DMA_MINALIGN,
					    addr_aligned);
}

صحیح bounce_بفر_stop(ساخت bounce_بفر *state)
{
	اگر (state->flags & GEN_BB_WRITE) {
		/* Invalidate cache so that CPU can see any newly DMA'd data */
		dma_unmap_single((dma_addr_t)(uintptr_t)state->bounce_بفر,
				 state->len_aligned,
				 DMA_BIDIRECTIONAL);
	}

	اگر (state->bounce_بفر == state->user_بفر)
		واپس 0;

	اگر (state->flags & GEN_BB_WRITE)
		حافظہ_نقل(state->user_بفر, state->bounce_بفر, state->len);

	آزاد(state->bounce_بفر);

	واپس 0;
}
