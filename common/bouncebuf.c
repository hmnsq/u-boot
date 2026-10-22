// SPDX-License-Identifier: GPL-2.0+
/*
 * Generic bounce buffer implementation
 *
 * Copyright (C) 2012 Marek Vasut <marex@denx.de>
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <cpu_func.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <log.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <malloc.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <bouncebuf.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/cache.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/dma-mapping.h>

ਸਥਿਰ ਪੂਰਨ addr_aligned(ਬਣਤਰ bounce_ਬਫਰ *state)
{
	ਅਚਲ ulong align_ਨਕਾਬ = ARCH_DMA_MINALIGN - 1;

	/* Check if start is aligned */
	ਜੇ ((ulong)state->user_ਬਫਰ & align_ਨਕਾਬ) {
		debug("Unaligned buffer address %p\n", state->user_ਬਫਰ);
		ਵਾਪਸ 0;
	}

	/* Check if length is aligned */
	ਜੇ (state->len != state->len_aligned) {
		debug("Unaligned buffer length %zu\n", state->len);
		ਵਾਪਸ 0;
	}

	/* Aligned */
	ਵਾਪਸ 1;
}

ਪੂਰਨ bounce_ਬਫਰ_ਸ਼ੁਰੂ_extalign(ਬਣਤਰ bounce_ਬਫਰ *state, ਖਾਲੀ *ਅੰਕੜੇ,
				 size_t len, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ flags,
				 size_t alignment,
				 ਪੂਰਨ (*addr_is_aligned)(ਬਣਤਰ bounce_ਬਫਰ *state))
{
	state->user_ਬਫਰ = ਅੰਕੜੇ;
	state->bounce_ਬਫਰ = ਅੰਕੜੇ;
	state->len = len;
	state->len_aligned = roundup(len, alignment);
	state->flags = flags;

	ਜੇ (!addr_is_aligned(state)) {
		state->bounce_ਬਫਰ = memalign(alignment,
						state->len_aligned);
		ਜੇ (!state->bounce_ਬਫਰ)
			ਵਾਪਸ -ENOMEM;

		ਜੇ (state->flags & GEN_BB_READ)
			ਸਮ੍ਰਿਤੀ_ਨਕਲ(state->bounce_ਬਫਰ, state->user_ਬਫਰ,
				state->len);
	}

	/*
	 * Flush data to RAM so DMA reads can pick it up,
	 * and any CPU writebacks don't race with DMA writes
	 */
	dma_map_single(state->bounce_ਬਫਰ,
		       state->len_aligned,
		       DMA_BIDIRECTIONAL);

	ਵਾਪਸ 0;
}

ਪੂਰਨ bounce_ਬਫਰ_ਸ਼ੁਰੂ(ਬਣਤਰ bounce_ਬਫਰ *state, ਖਾਲੀ *ਅੰਕੜੇ,
			size_t len, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ flags)
{
	ਵਾਪਸ bounce_ਬਫਰ_ਸ਼ੁਰੂ_extalign(state, ਅੰਕੜੇ, len, flags,
					    ARCH_DMA_MINALIGN,
					    addr_aligned);
}

ਪੂਰਨ bounce_ਬਫਰ_stop(ਬਣਤਰ bounce_ਬਫਰ *state)
{
	ਜੇ (state->flags & GEN_BB_WRITE) {
		/* Invalidate cache so that CPU can see any newly DMA'd data */
		dma_unmap_single((dma_addr_t)(uintptr_t)state->bounce_ਬਫਰ,
				 state->len_aligned,
				 DMA_BIDIRECTIONAL);
	}

	ਜੇ (state->bounce_ਬਫਰ == state->user_ਬਫਰ)
		ਵਾਪਸ 0;

	ਜੇ (state->flags & GEN_BB_WRITE)
		ਸਮ੍ਰਿਤੀ_ਨਕਲ(state->user_ਬਫਰ, state->bounce_ਬਫਰ, state->len);

	ਮੁਕਤ(state->bounce_ਬਫਰ);

	ਵਾਪਸ 0;
}
