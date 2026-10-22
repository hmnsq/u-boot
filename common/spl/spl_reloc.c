// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2024 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <gzip.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <image.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <log.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <mapmem.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <spl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/global_data.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/io.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/sections.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/unaligned.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/types.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <lzma/LzmaTypes.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <lzma/LzmaDec.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <lzma/LzmaTools.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <u-boot/crc.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <u-boot/lz4.h>

DECLARE_GLOBAL_ਅੰਕੜੇ_PTR;

/* provide a way to jump straight into the relocation code, for debugging */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEBUG_JUMP	0

ਗਿਣਤੀ {
	/* margin to allow for stack growth */
	RELOC_ਢੇਰ_MARGIN	= 0x800,

	/* align base address for DMA controllers which require it */
	ਅਧਾਰ_ALIGN		= 0x200,

	ਢੇਰ_PROT_ਮੁੱਲ	= 0x51ce4697,
};

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਪੂਰਨ (*rcode_func)(ਬਣਤਰ spl_image_info *image);

ਸਥਿਰ ਪੂਰਨ ਤਿਆਰੀ_layout(ਬਣਤਰ spl_image_info *image, ulong *addrp)
{
	ulong ਅਧਾਰ, fdt_ਮਾਪ;
	ulong ਹੱਦਬੰਦੀ, rcode_ਅਧਾਰ;
	uint rcode_ਮਾਪ;
	ਪੂਰਨ buf_ਮਾਪ, margin;
	ਅੱਖਰ *rcode_buf;

	ਹੱਦਬੰਦੀ = ALIGN(map_ਮੰਜ਼ਿਲ_sysmem(&ਹੱਦਬੰਦੀ) - RELOC_ਢੇਰ_MARGIN, 8);
	image->ਢੇਰ_prot = map_sysmem(ਹੱਦਬੰਦੀ, ਆਕਾਰ(uint));
	*image->ਢੇਰ_prot = ਢੇਰ_PROT_ਮੁੱਲ;

	fdt_ਮਾਪ = fdt_totalsize(gd->fdt_blob);
	ਅਧਾਰ = ALIGN(map_ਮੰਜ਼ਿਲ_sysmem(gd->fdt_blob) + fdt_ਮਾਪ + ਅਧਾਰ_ALIGN - 1,
		     ਅਧਾਰ_ALIGN);

	rcode_ਮਾਪ = rcode_ਸਿਰਾ - rcode_ਸ਼ੁਰੂ;
	rcode_ਅਧਾਰ = ਹੱਦਬੰਦੀ - rcode_ਮਾਪ;
	buf_ਮਾਪ = rcode_ਅਧਾਰ - ਅਧਾਰ;
	uint need_ਮਾਪ = image->ਮਾਪ + image->fdt_ਮਾਪ;
	margin = buf_ਮਾਪ - need_ਮਾਪ;
	log_debug("spl_reloc %s->%s: margin%s%lx limit %lx fdt_size %lx base %lx avail %x image %x fdt %lx need %x\n",
		  spl_phase_name(spl_phase()), spl_phase_name(spl_phase() + 1),
		  margin >= 0 ? " " : " -", ਨਿਰਪੇਖ(margin), ਹੱਦਬੰਦੀ, fdt_ਮਾਪ, ਅਧਾਰ,
		  buf_ਮਾਪ, image->ਮਾਪ, image->fdt_ਮਾਪ, need_ਮਾਪ);
	ਜੇ (margin < 0) {
		log_err("Image size %x but buffer is only %x\n", need_ਮਾਪ,
			buf_ਮਾਪ);
		ਵਾਪਸ -ENOSPC;
	}

	rcode_buf = map_sysmem(rcode_ਅਧਾਰ, rcode_ਮਾਪ);
	log_debug("_rcode_start %p: %x -- func %p %x\n", rcode_ਸ਼ੁਰੂ,
		  *(uint *)rcode_ਸ਼ੁਰੂ, ਤਿਆਰੀ_layout, *(uint *)ਤਿਆਰੀ_layout);

	image->reloc_ਵਿੱਥ = rcode_buf - rcode_ਸ਼ੁਰੂ;
	log_debug("_rcode start %lx base %lx size %x offset %lx\n",
		  (ulong)map_ਮੰਜ਼ਿਲ_sysmem(rcode_ਸ਼ੁਰੂ), rcode_ਅਧਾਰ, rcode_ਮਾਪ,
		  image->reloc_ਵਿੱਥ);

	ਸਮ੍ਰਿਤੀ_ਨਕਲ(rcode_buf, rcode_ਸ਼ੁਰੂ, rcode_ਮਾਪ);

	image->buf = map_sysmem(ਅਧਾਰ, need_ਮਾਪ);
	image->fdt_buf = image->buf + image->ਮਾਪ;
	image->rcode_buf = rcode_buf;
	*addrp = ਅਧਾਰ;

	ਵਾਪਸ 0;
}

ਪੂਰਨ spl_reloc_prepare(ਬਣਤਰ spl_image_info *image, ulong *addrp)
{
	ਪੂਰਨ ret;

	ret = ਤਿਆਰੀ_layout(image, addrp);
	ਜੇ (ret)
		ਵਾਪਸ ret;

	ਵਾਪਸ 0;
}

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਖਾਲੀ __noreturn (*image_ਇੰਦਰਾਜ_noargs_t)(uint ਚੱਕਰੀ_ਜਾਂਚ, uint unc_len);

/* this is the relocation + jump code that is copied to the top of memory */
__rcode ਪੂਰਨ rcode_reloc_and_jump(ਬਣਤਰ spl_image_info *image)
{
	image_ਇੰਦਰਾਜ_noargs_t ਇੰਦਰਾਜ = (image_ਇੰਦਰਾਜ_noargs_t)image->ਇੰਦਰਾਜ_ਬਿੰਦੂ;
	u32 *dst;
	ulong image_len;
	size_t unc_len;
	ਪੂਰਨ ret, ਚੱਕਰੀ_ਜਾਂਚ;
	uint magic;

	dst = map_sysmem(image->ਲੱਦੋ_addr, image->ਮਾਪ);
	unc_len = (ਖਾਲੀ *)image->rcode_buf - (ਖਾਲੀ *)dst;
	image_len = image->ਮਾਪ;
	ਜੇ (*image->ਢੇਰ_prot != ਢੇਰ_PROT_ਮੁੱਲ)
		ਵਾਪਸ -EFAULT;
	magic = get_unaligned_le32(image->buf);
	ਜੇ (CONFIG_IS_ENABLED(LZMA)) {
		ਮਾਪ_T lzma_len = unc_len;

		ret = lzma_Buff_ਮੰਜ਼ਿਲ_Buff_Decompress((u8 *)dst, &lzma_len,
					       image->buf, image_len);
		unc_len = lzma_len;
	} ਨਹੀਂਤਾਂ ਜੇ (CONFIG_IS_ENABLED(GZIP)) {
		ret = gunzip(dst, unc_len, image->buf, &image_len);
	} ਨਹੀਂਤਾਂ ਜੇ (CONFIG_IS_ENABLED(LZ4) && magic == LZ4F_MAGIC) {
		ret = ulz4fn(image->buf, image_len, dst, &unc_len);
		ਜੇ (ret)
			ਵਾਪਸ ret;
	} ਨਹੀਂਤਾਂ {
		u32 *src, *ਸਿਰਾ_2, *ptr;

		unc_len = image->ਮਾਪ;
		ਲਈ (src = image->buf, ਸਿਰਾ_2 = (ਖਾਲੀ *)src + image->ਮਾਪ,
		     ptr = dst; src < ਸਿਰਾ_2;)
			*ptr++ = *src++;
	}
	ਜੇ (*image->ਢੇਰ_prot != ਢੇਰ_PROT_ਮੁੱਲ)
		ਵਾਪਸ -EFAULT;

	/* copy in the FDT if needed */
	ਜੇ (image->fdt_ਮਾਪ)
		ਸਮ੍ਰਿਤੀ_ਨਕਲ(image->fdt_ਸ਼ੁਰੂ, image->fdt_buf, image->fdt_ਮਾਪ);

	ਚੱਕਰੀ_ਜਾਂਚ = crc8(0, (u8 *)dst, unc_len);

	/* jump to the entry point */
	ਇੰਦਰਾਜ(ਚੱਕਰੀ_ਜਾਂਚ, unc_len);
}

ਪੂਰਨ spl_reloc_jump(ਬਣਤਰ spl_image_info *image, spl_jump_ਮੰਜ਼ਿਲ_image_t jump)
{
	rcode_func loader;
	ਪੂਰਨ ret;

	log_debug("malloc usage %x bytes (%d KB of %d KB)\n", gd->malloc_ptr,
		  gd->malloc_ptr / 1024, CONFIG_VAL(SYS_MALLOC_F_LEN) / 1024);

	ਜੇ (*image->ਢੇਰ_prot != ਢੇਰ_PROT_ਮੁੱਲ) {
		log_err("stack busted, cannot continue\n");
		ਵਾਪਸ -EFAULT;
	}
	loader = (rcode_func)(ਖਾਲੀ *)rcode_reloc_and_jump + image->reloc_ਵਿੱਥ;
	log_debug("Jumping via %p to %lx - image %p size %x load %lx\n", loader,
		  image->ਇੰਦਰਾਜ_ਬਿੰਦੂ, image, image->ਮਾਪ, image->ਲੱਦੋ_addr);

	log_debug("unc_len %lx\n",
		  image->rcode_buf - map_sysmem(image->ਲੱਦੋ_addr, image->ਮਾਪ));
	ਜੇ (DEBUG_JUMP) {
		rcode_reloc_and_jump(image);
	} ਨਹੀਂਤਾਂ {
		/*
		 * Must disable LOG_DEBUG since the decompressor cannot call
		 * log functions, printf(), etc.
		 */
		_Static_assert(DEBUG_JUMP || !_DEBUG,
			       "Cannot have debug output from decompressor");
		ret = loader(image);
	}

	ਵਾਪਸ -EFAULT;
}
