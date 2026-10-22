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
	ध्रुव ulong align_mask = ARCH_DMA_MINALIGN - 1;

	/* Check if start is aligned */
	जइ ((ulong)state->user_buffer & align_mask) {
		debug("Unaligned buffer address %p\n", state->user_buffer);
		प्रत्यागम 0;
	}

	/* Check if length is aligned */
	जइ (state->len != state->len_aligned) {
		debug("Unaligned buffer length %zu\n", state->len);
		प्रत्यागम 0;
	}

	/* Aligned */
	प्रत्यागम 1;
}

पूर्णाङ्क bounce_buffer_start_extalign(संरचना bounce_buffer *state, शून्य *data,
				 size_t len, unsigned पूर्णाङ्क flags,
				 size_t alignment,
				 पूर्णाङ्क (*addr_is_aligned)(संरचना bounce_buffer *state))
{
	state->user_buffer = data;
	state->bounce_buffer = data;
	state->len = len;
	state->len_aligned = roundup(len, alignment);
	state->flags = flags;

	जइ (!addr_is_aligned(state)) {
		state->bounce_buffer = memalign(alignment,
						state->len_aligned);
		जइ (!state->bounce_buffer)
			प्रत्यागम -ENOMEM;

		जइ (state->flags & GEN_BB_READ)
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

	प्रत्यागम 0;
}

पूर्णाङ्क bounce_buffer_start(संरचना bounce_buffer *state, शून्य *data,
			size_t len, unsigned पूर्णाङ्क flags)
{
	प्रत्यागम bounce_buffer_start_extalign(state, data, len, flags,
					    ARCH_DMA_MINALIGN,
					    addr_aligned);
}

पूर्णाङ्क bounce_buffer_stop(संरचना bounce_buffer *state)
{
	जइ (state->flags & GEN_BB_WRITE) {
		/* Invalidate cache so that CPU can see any newly DMA'd data */
		dma_unmap_single((dma_addr_t)(uintptr_t)state->bounce_buffer,
				 state->len_aligned,
				 DMA_BIDIRECTIONAL);
	}

	जइ (state->bounce_buffer == state->user_buffer)
		प्रत्यागम 0;

	जइ (state->flags & GEN_BB_WRITE)
		memcpy(state->user_buffer, state->bounce_buffer, state->len);

	मुक्त(state->bounce_buffer);

	प्रत्यागम 0;
}
