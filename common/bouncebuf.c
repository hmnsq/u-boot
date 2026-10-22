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

স্থির পূর্ণসংখ্যা addr_aligned(কাঠামো bounce_বাফার *state)
{
	ধ্রুবক ulong align_mask = ARCH_DMA_MINALIGN - 1;

	/* Check if start is aligned */
	যদি ((ulong)state->user_বাফার & align_mask) {
		debug("Unaligned buffer address %p\n", state->user_বাফার);
		ফেরত 0;
	}

	/* Check if length is aligned */
	যদি (state->len != state->len_aligned) {
		debug("Unaligned buffer length %zu\n", state->len);
		ফেরত 0;
	}

	/* Aligned */
	ফেরত 1;
}

পূর্ণসংখ্যা bounce_বাফার_start_extalign(কাঠামো bounce_বাফার *state, শূন্য *তথ্য,
				 size_t len, unsigned পূর্ণসংখ্যা flags,
				 size_t alignment,
				 পূর্ণসংখ্যা (*addr_is_aligned)(কাঠামো bounce_বাফার *state))
{
	state->user_বাফার = তথ্য;
	state->bounce_বাফার = তথ্য;
	state->len = len;
	state->len_aligned = roundup(len, alignment);
	state->flags = flags;

	যদি (!addr_is_aligned(state)) {
		state->bounce_বাফার = memalign(alignment,
						state->len_aligned);
		যদি (!state->bounce_বাফার)
			ফেরত -ENOMEM;

		যদি (state->flags & GEN_BB_READ)
			memcpy(state->bounce_বাফার, state->user_বাফার,
				state->len);
	}

	/*
	 * Flush data to RAM so DMA reads can pick it up,
	 * and any CPU writebacks don't race with DMA writes
	 */
	dma_map_single(state->bounce_বাফার,
		       state->len_aligned,
		       DMA_BIDIRECTIONAL);

	ফেরত 0;
}

পূর্ণসংখ্যা bounce_বাফার_start(কাঠামো bounce_বাফার *state, শূন্য *তথ্য,
			size_t len, unsigned পূর্ণসংখ্যা flags)
{
	ফেরত bounce_বাফার_start_extalign(state, তথ্য, len, flags,
					    ARCH_DMA_MINALIGN,
					    addr_aligned);
}

পূর্ণসংখ্যা bounce_বাফার_stop(কাঠামো bounce_বাফার *state)
{
	যদি (state->flags & GEN_BB_WRITE) {
		/* Invalidate cache so that CPU can see any newly DMA'd data */
		dma_unmap_single((dma_addr_t)(uintptr_t)state->bounce_বাফার,
				 state->len_aligned,
				 DMA_BIDIRECTIONAL);
	}

	যদি (state->bounce_বাফার == state->user_বাফার)
		ফেরত 0;

	যদি (state->flags & GEN_BB_WRITE)
		memcpy(state->user_বাফার, state->bounce_বাফার, state->len);

	মুক্ত(state->bounce_বাফার);

	ফেরত 0;
}
