// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2024 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#ಅನ್ತರ್ಭಾವ <gzip.h>
#ಅನ್ತರ್ಭಾವ <image.h>
#ಅನ್ತರ್ಭಾವ <log.h>
#ಅನ್ತರ್ಭಾವ <mapmem.h>
#ಅನ್ತರ್ಭಾವ <spl.h>
#ಅನ್ತರ್ಭಾವ <asm/global_data.h>
#ಅನ್ತರ್ಭಾವ <asm/io.h>
#ಅನ್ತರ್ಭಾವ <asm/sections.h>
#ಅನ್ತರ್ಭಾವ <asm/unaligned.h>
#ಅನ್ತರ್ಭಾವ <linux/types.h>
#ಅನ್ತರ್ಭಾವ <lzma/LzmaTypes.h>
#ಅನ್ತರ್ಭಾವ <lzma/LzmaDec.h>
#ಅನ್ತರ್ಭಾವ <lzma/LzmaTools.h>
#ಅನ್ತರ್ಭಾವ <u-boot/crc.h>
#ಅನ್ತರ್ಭಾವ <u-boot/lz4.h>

DECLARE_GLOBAL_ದತ್ತಾಂಶ_PTR;

/* provide a way to jump straight into the relocation code, for debugging */
#ವ್ಯಾಖ್ಯಾನಿಸು DEBUG_JUMP	0

ಎಣಿಕೆ {
	/* margin to allow for stack growth */
	RELOC_ರಾಶಿ_MARGIN	= 0x800,

	/* align base address for DMA controllers which require it */
	ತಳ_ALIGN		= 0x200,

	ರಾಶಿ_PROT_ಮೌಲ್ಯ	= 0x51ce4697,
};

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಪೂರ್ಣಾಂಕ (*rcode_func)(ರಚನೆ spl_image_info *image);

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಸಿದ್ಧತೆ_layout(ರಚನೆ spl_image_info *image, ulong *addrp)
{
	ulong ತಳ, fdt_ಗಾತ್ರ;
	ulong ಎಲ್ಲೆ, rcode_ತಳ;
	uint rcode_ಗಾತ್ರ;
	ಪೂರ್ಣಾಂಕ buf_ಗಾತ್ರ, margin;
	ಅಕ್ಷರ *rcode_buf;

	ಎಲ್ಲೆ = ALIGN(map_ಗಮ್ಯ_sysmem(&ಎಲ್ಲೆ) - RELOC_ರಾಶಿ_MARGIN, 8);
	image->ರಾಶಿ_prot = map_sysmem(ಎಲ್ಲೆ, ಗಾತ್ರ(uint));
	*image->ರಾಶಿ_prot = ರಾಶಿ_PROT_ಮೌಲ್ಯ;

	fdt_ಗಾತ್ರ = fdt_totalsize(gd->fdt_blob);
	ತಳ = ALIGN(map_ಗಮ್ಯ_sysmem(gd->fdt_blob) + fdt_ಗಾತ್ರ + ತಳ_ALIGN - 1,
		     ತಳ_ALIGN);

	rcode_ಗಾತ್ರ = rcode_ಕೊನೆ - rcode_ಆರಂಭ;
	rcode_ತಳ = ಎಲ್ಲೆ - rcode_ಗಾತ್ರ;
	buf_ಗಾತ್ರ = rcode_ತಳ - ತಳ;
	uint need_ಗಾತ್ರ = image->ಗಾತ್ರ_2 + image->fdt_ಗಾತ್ರ;
	margin = buf_ಗಾತ್ರ - need_ಗಾತ್ರ;
	log_debug("spl_reloc %s->%s: margin%s%lx limit %lx fdt_size %lx base %lx avail %x image %x fdt %lx need %x\n",
		  spl_phase_name(spl_phase()), spl_phase_name(spl_phase() + 1),
		  margin >= 0 ? " " : " -", ನಿರಪೇಕ್ಷ(margin), ಎಲ್ಲೆ, fdt_ಗಾತ್ರ, ತಳ,
		  buf_ಗಾತ್ರ, image->ಗಾತ್ರ_2, image->fdt_ಗಾತ್ರ, need_ಗಾತ್ರ);
	ಆದರೆ (margin < 0) {
		log_err("Image size %x but buffer is only %x\n", need_ಗಾತ್ರ,
			buf_ಗಾತ್ರ);
		ಹಿಂತಿರುಗಿ -ENOSPC;
	}

	rcode_buf = map_sysmem(rcode_ತಳ, rcode_ಗಾತ್ರ);
	log_debug("_rcode_start %p: %x -- func %p %x\n", rcode_ಆರಂಭ,
		  *(uint *)rcode_ಆರಂಭ, ಸಿದ್ಧತೆ_layout, *(uint *)ಸಿದ್ಧತೆ_layout);

	image->reloc_ಸ್ಥಾನಾಂತರ = rcode_buf - rcode_ಆರಂಭ;
	log_debug("_rcode start %lx base %lx size %x offset %lx\n",
		  (ulong)map_ಗಮ್ಯ_sysmem(rcode_ಆರಂಭ), rcode_ತಳ, rcode_ಗಾತ್ರ,
		  image->reloc_ಸ್ಥಾನಾಂತರ);

	ಸ್ಮರಣೆ_ನಕಲು(rcode_buf, rcode_ಆರಂಭ, rcode_ಗಾತ್ರ);

	image->buf = map_sysmem(ತಳ, need_ಗಾತ್ರ);
	image->fdt_buf = image->buf + image->ಗಾತ್ರ_2;
	image->rcode_buf = rcode_buf;
	*addrp = ತಳ;

	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ spl_reloc_prepare(ರಚನೆ spl_image_info *image, ulong *addrp)
{
	ಪೂರ್ಣಾಂಕ ret;

	ret = ಸಿದ್ಧತೆ_layout(image, addrp);
	ಆದರೆ (ret)
		ಹಿಂತಿರುಗಿ ret;

	ಹಿಂತಿರುಗಿ 0;
}

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಶೂನ್ಯ __noreturn (*image_ನಮೂದು_noargs_t)(uint ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ, uint unc_len);

/* this is the relocation + jump code that is copied to the top of memory */
__rcode ಪೂರ್ಣಾಂಕ rcode_reloc_and_jump(ರಚನೆ spl_image_info *image)
{
	image_ನಮೂದು_noargs_t ನಮೂದು = (image_ನಮೂದು_noargs_t)image->ನಮೂದು_ಬಿಂದು;
	u32 *dst;
	ulong image_len;
	size_t unc_len;
	ಪೂರ್ಣಾಂಕ ret, ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ;
	uint magic;

	dst = map_sysmem(image->ತುಂಬು_addr, image->ಗಾತ್ರ_2);
	unc_len = (ಶೂನ್ಯ *)image->rcode_buf - (ಶೂನ್ಯ *)dst;
	image_len = image->ಗಾತ್ರ_2;
	ಆದರೆ (*image->ರಾಶಿ_prot != ರಾಶಿ_PROT_ಮೌಲ್ಯ)
		ಹಿಂತಿರುಗಿ -EFAULT;
	magic = get_unaligned_le32(image->buf);
	ಆದರೆ (CONFIG_IS_ENABLED(LZMA)) {
		ಗಾತ್ರ_T lzma_len = unc_len;

		ret = lzma_Buff_ಗಮ್ಯ_Buff_Decompress((u8 *)dst, &lzma_len,
					       image->buf, image_len);
		unc_len = lzma_len;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (CONFIG_IS_ENABLED(GZIP)) {
		ret = gunzip(dst, unc_len, image->buf, &image_len);
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (CONFIG_IS_ENABLED(LZ4) && magic == LZ4F_MAGIC) {
		ret = ulz4fn(image->buf, image_len, dst, &unc_len);
		ಆದರೆ (ret)
			ಹಿಂತಿರುಗಿ ret;
	} ಇಲ್ಲದಿದ್ದರೆ {
		u32 *src, *ಕೊನೆ_2, *ptr;

		unc_len = image->ಗಾತ್ರ_2;
		ಗಾಗಿ (src = image->buf, ಕೊನೆ_2 = (ಶೂನ್ಯ *)src + image->ಗಾತ್ರ_2,
		     ptr = dst; src < ಕೊನೆ_2;)
			*ptr++ = *src++;
	}
	ಆದರೆ (*image->ರಾಶಿ_prot != ರಾಶಿ_PROT_ಮೌಲ್ಯ)
		ಹಿಂತಿರುಗಿ -EFAULT;

	/* copy in the FDT if needed */
	ಆದರೆ (image->fdt_ಗಾತ್ರ)
		ಸ್ಮರಣೆ_ನಕಲು(image->fdt_ಆರಂಭ, image->fdt_buf, image->fdt_ಗಾತ್ರ);

	ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ = crc8(0, (u8 *)dst, unc_len);

	/* jump to the entry point */
	ನಮೂದು(ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ, unc_len);
}

ಪೂರ್ಣಾಂಕ spl_reloc_jump(ರಚನೆ spl_image_info *image, spl_jump_ಗಮ್ಯ_image_t jump)
{
	rcode_func loader;
	ಪೂರ್ಣಾಂಕ ret;

	log_debug("malloc usage %x bytes (%d KB of %d KB)\n", gd->malloc_ptr,
		  gd->malloc_ptr / 1024, CONFIG_VAL(SYS_MALLOC_F_LEN) / 1024);

	ಆದರೆ (*image->ರಾಶಿ_prot != ರಾಶಿ_PROT_ಮೌಲ್ಯ) {
		log_err("stack busted, cannot continue\n");
		ಹಿಂತಿರುಗಿ -EFAULT;
	}
	loader = (rcode_func)(ಶೂನ್ಯ *)rcode_reloc_and_jump + image->reloc_ಸ್ಥಾನಾಂತರ;
	log_debug("Jumping via %p to %lx - image %p size %x load %lx\n", loader,
		  image->ನಮೂದು_ಬಿಂದು, image, image->ಗಾತ್ರ_2, image->ತುಂಬು_addr);

	log_debug("unc_len %lx\n",
		  image->rcode_buf - map_sysmem(image->ತುಂಬು_addr, image->ಗಾತ್ರ_2));
	ಆದರೆ (DEBUG_JUMP) {
		rcode_reloc_and_jump(image);
	} ಇಲ್ಲದಿದ್ದರೆ {
		/*
		 * Must disable LOG_DEBUG since the decompressor cannot call
		 * log functions, printf(), etc.
		 */
		_Static_assert(DEBUG_JUMP || !_DEBUG,
			       "Cannot have debug output from decompressor");
		ret = loader(image);
	}

	ಹಿಂತಿರುಗಿ -EFAULT;
}
