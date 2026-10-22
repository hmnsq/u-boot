// SPDX-License-Identifier: GPL-2.0+
/*
 * Generic bounce buffer implementation
 *
 * Copyright (C) 2012 Marek Vasut <marex@denx.de>
 */

#ಅನ್ತರ್ಭಾವ <cpu_func.h>
#ಅನ್ತರ್ಭಾವ <log.h>
#ಅನ್ತರ್ಭಾವ <malloc.h>
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <bouncebuf.h>
#ಅನ್ತರ್ಭಾವ <asm/cache.h>
#ಅನ್ತರ್ಭಾವ <linux/dma-mapping.h>

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ addr_aligned(ರಚನೆ bounce_ಬಫರ್ *state)
{
	ಸ್ಥಿರಾಂಕ ulong align_ಮುಖವಾಡ = ARCH_DMA_MINALIGN - 1;

	/* Check if start is aligned */
	ಆದರೆ ((ulong)state->user_ಬಫರ್ & align_ಮುಖವಾಡ) {
		debug("Unaligned buffer address %p\n", state->user_ಬಫರ್);
		ಹಿಂತಿರುಗಿ 0;
	}

	/* Check if length is aligned */
	ಆದರೆ (state->len != state->len_aligned) {
		debug("Unaligned buffer length %zu\n", state->len);
		ಹಿಂತಿರುಗಿ 0;
	}

	/* Aligned */
	ಹಿಂತಿರುಗಿ 1;
}

ಪೂರ್ಣಾಂಕ bounce_ಬಫರ್_ಆರಂಭ_extalign(ರಚನೆ bounce_ಬಫರ್ *state, ಶೂನ್ಯ *ದತ್ತಾಂಶ,
				 size_t len, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ flags,
				 size_t alignment,
				 ಪೂರ್ಣಾಂಕ (*addr_is_aligned)(ರಚನೆ bounce_ಬಫರ್ *state))
{
	state->user_ಬಫರ್ = ದತ್ತಾಂಶ;
	state->bounce_ಬಫರ್ = ದತ್ತಾಂಶ;
	state->len = len;
	state->len_aligned = roundup(len, alignment);
	state->flags = flags;

	ಆದರೆ (!addr_is_aligned(state)) {
		state->bounce_ಬಫರ್ = memalign(alignment,
						state->len_aligned);
		ಆದರೆ (!state->bounce_ಬಫರ್)
			ಹಿಂತಿರುಗಿ -ENOMEM;

		ಆದರೆ (state->flags & GEN_BB_READ)
			ಸ್ಮರಣೆ_ನಕಲು(state->bounce_ಬಫರ್, state->user_ಬಫರ್,
				state->len);
	}

	/*
	 * Flush data to RAM so DMA reads can pick it up,
	 * and any CPU writebacks don't race with DMA writes
	 */
	dma_map_single(state->bounce_ಬಫರ್,
		       state->len_aligned,
		       DMA_BIDIRECTIONAL);

	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ bounce_ಬಫರ್_ಆರಂಭ(ರಚನೆ bounce_ಬಫರ್ *state, ಶೂನ್ಯ *ದತ್ತಾಂಶ,
			size_t len, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ flags)
{
	ಹಿಂತಿರುಗಿ bounce_ಬಫರ್_ಆರಂಭ_extalign(state, ದತ್ತಾಂಶ, len, flags,
					    ARCH_DMA_MINALIGN,
					    addr_aligned);
}

ಪೂರ್ಣಾಂಕ bounce_ಬಫರ್_stop(ರಚನೆ bounce_ಬಫರ್ *state)
{
	ಆದರೆ (state->flags & GEN_BB_WRITE) {
		/* Invalidate cache so that CPU can see any newly DMA'd data */
		dma_unmap_single((dma_addr_t)(uintptr_t)state->bounce_ಬಫರ್,
				 state->len_aligned,
				 DMA_BIDIRECTIONAL);
	}

	ಆದರೆ (state->bounce_ಬಫರ್ == state->user_ಬಫರ್)
		ಹಿಂತಿರುಗಿ 0;

	ಆದರೆ (state->flags & GEN_BB_WRITE)
		ಸ್ಮರಣೆ_ನಕಲು(state->user_ಬಫರ್, state->bounce_ಬಫರ್, state->len);

	ಬಿಡುಗಡೆ(state->bounce_ಬಫರ್);

	ಹಿಂತಿರುಗಿ 0;
}
