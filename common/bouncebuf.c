// SPDX-License-Identifier: GPL-2.0+
/*
 * Generic bounce buffer implementation
 *
 * Copyright (C) 2012 Marek Vasut <marex@denx.de>
 */

#అన్తర్భావ <cpu_func.h>
#అన్తర్భావ <log.h>
#అన్తర్భావ <malloc.h>
#అన్తర్భావ <errno.h>
#అన్తర్భావ <bouncebuf.h>
#అన్తర్భావ <asm/cache.h>
#అన్తర్భావ <linux/dma-mapping.h>

స్థిర పూర్ణాంకం addr_aligned(నిర్మాణం bounce_బఫరు *state)
{
	స్థిరరాశి ulong align_ముసుగు = ARCH_DMA_MINALIGN - 1;

	/* Check if start is aligned */
	ఒకవేళ ((ulong)state->user_బఫరు & align_ముసుగు) {
		debug("Unaligned buffer address %p\n", state->user_బఫరు);
		తిరిగి 0;
	}

	/* Check if length is aligned */
	ఒకవేళ (state->len != state->len_aligned) {
		debug("Unaligned buffer length %zu\n", state->len);
		తిరిగి 0;
	}

	/* Aligned */
	తిరిగి 1;
}

పూర్ణాంకం bounce_బఫరు_ప్రారంభం_extalign(నిర్మాణం bounce_బఫరు *state, శూన్య *దత్తాంశం,
				 size_t len, గుర్తులేని పూర్ణాంకం flags,
				 size_t alignment,
				 పూర్ణాంకం (*addr_is_aligned)(నిర్మాణం bounce_బఫరు *state))
{
	state->user_బఫరు = దత్తాంశం;
	state->bounce_బఫరు = దత్తాంశం;
	state->len = len;
	state->len_aligned = roundup(len, alignment);
	state->flags = flags;

	ఒకవేళ (!addr_is_aligned(state)) {
		state->bounce_బఫరు = memalign(alignment,
						state->len_aligned);
		ఒకవేళ (!state->bounce_బఫరు)
			తిరిగి -ENOMEM;

		ఒకవేళ (state->flags & GEN_BB_READ)
			జ్ఞాపకం_నకలు(state->bounce_బఫరు, state->user_బఫరు,
				state->len);
	}

	/*
	 * Flush data to RAM so DMA reads can pick it up,
	 * and any CPU writebacks don't race with DMA writes
	 */
	dma_map_single(state->bounce_బఫరు,
		       state->len_aligned,
		       DMA_BIDIRECTIONAL);

	తిరిగి 0;
}

పూర్ణాంకం bounce_బఫరు_ప్రారంభం(నిర్మాణం bounce_బఫరు *state, శూన్య *దత్తాంశం,
			size_t len, గుర్తులేని పూర్ణాంకం flags)
{
	తిరిగి bounce_బఫరు_ప్రారంభం_extalign(state, దత్తాంశం, len, flags,
					    ARCH_DMA_MINALIGN,
					    addr_aligned);
}

పూర్ణాంకం bounce_బఫరు_stop(నిర్మాణం bounce_బఫరు *state)
{
	ఒకవేళ (state->flags & GEN_BB_WRITE) {
		/* Invalidate cache so that CPU can see any newly DMA'd data */
		dma_unmap_single((dma_addr_t)(uintptr_t)state->bounce_బఫరు,
				 state->len_aligned,
				 DMA_BIDIRECTIONAL);
	}

	ఒకవేళ (state->bounce_బఫరు == state->user_బఫరు)
		తిరిగి 0;

	ఒకవేళ (state->flags & GEN_BB_WRITE)
		జ్ఞాపకం_నకలు(state->user_బఫరు, state->bounce_బఫరు, state->len);

	విడుదల(state->bounce_బఫరు);

	తిరిగి 0;
}
