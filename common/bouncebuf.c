// SPDX-License-Identifier: GPL-2.0+
/*
 * Generic bounce buffer implementation
 *
 * Copyright (C) 2012 Marek Vasut <marex@denx.de>
 */

#अन्तर्भाव <cpu_func.h>
#अन्तर्भाव <log.h>
#अन्तर्भाव <malloc.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <bouncebuf.h>
#अन्तर्भाव <asm/cache.h>
#अन्तर्भाव <linux/dma-mapping.h>

स्थिर पूर्णाङ्क addr_aligned(संरचना bounce_buffer *state)
{
	अचर ulong align_mask = ARCH_DMA_MINALIGN - 1;

	/* Check if start is aligned */
	यदि ((ulong)state->user_buffer & align_mask) {
		debug("Unaligned buffer address %p\n", state->user_buffer);
		फर्काऊ 0;
	}

	/* Check if length is aligned */
	यदि (state->len != state->len_aligned) {
		debug("Unaligned buffer length %zu\n", state->len);
		फर्काऊ 0;
	}

	/* Aligned */
	फर्काऊ 1;
}

पूर्णाङ्क bounce_buffer_start_extalign(संरचना bounce_buffer *state, शून्य *data,
				 माप_प्रकार len, अचिन्हित पूर्णाङ्क flags,
				 माप_प्रकार alignment,
				 पूर्णाङ्क (*addr_is_aligned)(संरचना bounce_buffer *state))
{
	state->user_buffer = data;
	state->bounce_buffer = data;
	state->len = len;
	state->len_aligned = roundup(len, alignment);
	state->flags = flags;

	यदि (!addr_is_aligned(state)) {
		state->bounce_buffer = memalign(alignment,
						state->len_aligned);
		यदि (!state->bounce_buffer)
			फर्काऊ -ENOMEM;

		यदि (state->flags & GEN_BB_READ)
			स_नकल(state->bounce_buffer, state->user_buffer,
				state->len);
	}

	/*
	 * Flush data to RAM so DMA reads can pick it up,
	 * and any CPU writebacks don't race with DMA writes
	 */
	dma_map_single(state->bounce_buffer,
		       state->len_aligned,
		       DMA_BIDIRECTIONAL);

	फर्काऊ 0;
}

पूर्णाङ्क bounce_buffer_start(संरचना bounce_buffer *state, शून्य *data,
			माप_प्रकार len, अचिन्हित पूर्णाङ्क flags)
{
	फर्काऊ bounce_buffer_start_extalign(state, data, len, flags,
					    ARCH_DMA_MINALIGN,
					    addr_aligned);
}

पूर्णाङ्क bounce_buffer_stop(संरचना bounce_buffer *state)
{
	यदि (state->flags & GEN_BB_WRITE) {
		/* Invalidate cache so that CPU can see any newly DMA'd data */
		dma_unmap_single((dma_addr_t)(uintptr_t)state->bounce_buffer,
				 state->len_aligned,
				 DMA_BIDIRECTIONAL);
	}

	यदि (state->bounce_buffer == state->user_buffer)
		फर्काऊ 0;

	यदि (state->flags & GEN_BB_WRITE)
		स_नकल(state->user_buffer, state->bounce_buffer, state->len);

	मुक्त(state->bounce_buffer);

	फर्काऊ 0;
}
