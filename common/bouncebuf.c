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

ساكن صحيح addr_aligned(بنية bounce_میانگیر *state)
{
	ثابت ulong align_ماسک = ARCH_DMA_MINALIGN - 1;

	/* Check if start is aligned */
	إذا ((ulong)state->user_میانگیر & align_ماسک) {
		debug("Unaligned buffer address %p\n", state->user_میانگیر);
		أرجع 0;
	}

	/* Check if length is aligned */
	إذا (state->len != state->len_aligned) {
		debug("Unaligned buffer length %zu\n", state->len);
		أرجع 0;
	}

	/* Aligned */
	أرجع 1;
}

صحيح bounce_میانگیر_شروع_extalign(بنية bounce_میانگیر *state, فراغ *داده,
				 size_t len, بیعلامت صحيح flags,
				 size_t alignment,
				 صحيح (*addr_is_aligned)(بنية bounce_میانگیر *state))
{
	state->user_میانگیر = داده;
	state->bounce_میانگیر = داده;
	state->len = len;
	state->len_aligned = roundup(len, alignment);
	state->flags = flags;

	إذا (!addr_is_aligned(state)) {
		state->bounce_میانگیر = memalign(alignment,
						state->len_aligned);
		إذا (!state->bounce_میانگیر)
			أرجع -ENOMEM;

		إذا (state->flags & GEN_BB_READ)
			کپی_حافظه(state->bounce_میانگیر, state->user_میانگیر,
				state->len);
	}

	/*
	 * Flush data to RAM so DMA reads can pick it up,
	 * and any CPU writebacks don't race with DMA writes
	 */
	dma_map_single(state->bounce_میانگیر,
		       state->len_aligned,
		       DMA_BIDIRECTIONAL);

	أرجع 0;
}

صحيح bounce_میانگیر_شروع(بنية bounce_میانگیر *state, فراغ *داده,
			size_t len, بیعلامت صحيح flags)
{
	أرجع bounce_میانگیر_شروع_extalign(state, داده, len, flags,
					    ARCH_DMA_MINALIGN,
					    addr_aligned);
}

صحيح bounce_میانگیر_stop(بنية bounce_میانگیر *state)
{
	إذا (state->flags & GEN_BB_WRITE) {
		/* Invalidate cache so that CPU can see any newly DMA'd data */
		dma_unmap_single((dma_addr_t)(uintptr_t)state->bounce_میانگیر,
				 state->len_aligned,
				 DMA_BIDIRECTIONAL);
	}

	إذا (state->bounce_میانگیر == state->user_میانگیر)
		أرجع 0;

	إذا (state->flags & GEN_BB_WRITE)
		کپی_حافظه(state->user_میانگیر, state->bounce_میانگیر, state->len);

	حرر(state->bounce_میانگیر);

	أرجع 0;
}
