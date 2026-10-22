// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2024 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#அந்தர்பாவ <gzip.h>
#அந்தர்பாவ <image.h>
#அந்தர்பாவ <log.h>
#அந்தர்பாவ <mapmem.h>
#அந்தர்பாவ <spl.h>
#அந்தர்பாவ <asm/global_data.h>
#அந்தர்பாவ <asm/io.h>
#அந்தர்பாவ <asm/sections.h>
#அந்தர்பாவ <asm/unaligned.h>
#அந்தர்பாவ <linux/types.h>
#அந்தர்பாவ <lzma/LzmaTypes.h>
#அந்தர்பாவ <lzma/LzmaDec.h>
#அந்தர்பாவ <lzma/LzmaTools.h>
#அந்தர்பாவ <u-boot/crc.h>
#அந்தர்பாவ <u-boot/lz4.h>

DECLARE_GLOBAL_DATA_PTR;

/* provide a way to jump straight into the relocation code, for debugging */
#define DEBUG_JUMP	0

எண்ணிக்கை {
	/* margin to allow for stack growth */
	RELOC_அடுக்கு_MARGIN	= 0x800,

	/* align base address for DMA controllers which require it */
	BASE_ALIGN		= 0x200,

	அடுக்கு_PROT_மதிப்பு	= 0x51ce4697,
};

வகைவரை முழு (*rcode_func)(கட்டமைப்பு spl_image_info *image);

நிலையான முழு setup_layout(கட்டமைப்பு spl_image_info *image, ulong *addrp)
{
	ulong base, fdt_size;
	ulong limit, rcode_base;
	uint rcode_size;
	முழு buf_size, margin;
	எழுத்து *rcode_buf;

	limit = ALIGN(map_to_sysmem(&limit) - RELOC_அடுக்கு_MARGIN, 8);
	image->அடுக்கு_prot = map_sysmem(limit, அளவு(uint));
	*image->அடுக்கு_prot = அடுக்கு_PROT_மதிப்பு;

	fdt_size = fdt_totalsize(gd->fdt_blob);
	base = ALIGN(map_to_sysmem(gd->fdt_blob) + fdt_size + BASE_ALIGN - 1,
		     BASE_ALIGN);

	rcode_size = _rcode_end - _rcode_start;
	rcode_base = limit - rcode_size;
	buf_size = rcode_base - base;
	uint need_size = image->size + image->fdt_size;
	margin = buf_size - need_size;
	log_debug("spl_reloc %s->%s: margin%s%lx limit %lx fdt_size %lx base %lx avail %x image %x fdt %lx need %x\n",
		  spl_phase_name(spl_phase()), spl_phase_name(spl_phase() + 1),
		  margin >= 0 ? " " : " -", abs(margin), limit, fdt_size, base,
		  buf_size, image->size, image->fdt_size, need_size);
	எனில் (margin < 0) {
		log_err("Image size %x but buffer is only %x\n", need_size,
			buf_size);
		திரும்பு -ENOSPC;
	}

	rcode_buf = map_sysmem(rcode_base, rcode_size);
	log_debug("_rcode_start %p: %x -- func %p %x\n", _rcode_start,
		  *(uint *)_rcode_start, setup_layout, *(uint *)setup_layout);

	image->reloc_offset = rcode_buf - _rcode_start;
	log_debug("_rcode start %lx base %lx size %x offset %lx\n",
		  (ulong)map_to_sysmem(_rcode_start), rcode_base, rcode_size,
		  image->reloc_offset);

	memcpy(rcode_buf, _rcode_start, rcode_size);

	image->buf = map_sysmem(base, need_size);
	image->fdt_buf = image->buf + image->size;
	image->rcode_buf = rcode_buf;
	*addrp = base;

	திரும்பு 0;
}

முழு spl_reloc_prepare(கட்டமைப்பு spl_image_info *image, ulong *addrp)
{
	முழு ret;

	ret = setup_layout(image, addrp);
	எனில் (ret)
		திரும்பு ret;

	திரும்பு 0;
}

வகைவரை வெறுமை __noreturn (*image_entry_noargs_t)(uint crc, uint unc_len);

/* this is the relocation + jump code that is copied to the top of memory */
__rcode முழு rcode_reloc_and_jump(கட்டமைப்பு spl_image_info *image)
{
	image_entry_noargs_t entry = (image_entry_noargs_t)image->entry_point;
	u32 *dst;
	ulong image_len;
	size_t unc_len;
	முழு ret, crc;
	uint magic;

	dst = map_sysmem(image->load_addr, image->size);
	unc_len = (வெறுமை *)image->rcode_buf - (வெறுமை *)dst;
	image_len = image->size;
	எனில் (*image->அடுக்கு_prot != அடுக்கு_PROT_மதிப்பு)
		திரும்பு -EFAULT;
	magic = get_unaligned_le32(image->buf);
	எனில் (CONFIG_IS_ENABLED(LZMA)) {
		SizeT lzma_len = unc_len;

		ret = lzmaBuffToBuffDecompress((u8 *)dst, &lzma_len,
					       image->buf, image_len);
		unc_len = lzma_len;
	} இல்லையெனில் எனில் (CONFIG_IS_ENABLED(GZIP)) {
		ret = gunzip(dst, unc_len, image->buf, &image_len);
	} இல்லையெனில் எனில் (CONFIG_IS_ENABLED(LZ4) && magic == LZ4F_MAGIC) {
		ret = ulz4fn(image->buf, image_len, dst, &unc_len);
		எனில் (ret)
			திரும்பு ret;
	} இல்லையெனில் {
		u32 *src, *end, *ptr;

		unc_len = image->size;
		க்கு (src = image->buf, end = (வெறுமை *)src + image->size,
		     ptr = dst; src < end;)
			*ptr++ = *src++;
	}
	எனில் (*image->அடுக்கு_prot != அடுக்கு_PROT_மதிப்பு)
		திரும்பு -EFAULT;

	/* copy in the FDT if needed */
	எனில் (image->fdt_size)
		memcpy(image->fdt_start, image->fdt_buf, image->fdt_size);

	crc = crc8(0, (u8 *)dst, unc_len);

	/* jump to the entry point */
	entry(crc, unc_len);
}

முழு spl_reloc_jump(கட்டமைப்பு spl_image_info *image, spl_jump_to_image_t jump)
{
	rcode_func loader;
	முழு ret;

	log_debug("malloc usage %x bytes (%d KB of %d KB)\n", gd->malloc_ptr,
		  gd->malloc_ptr / 1024, CONFIG_VAL(SYS_MALLOC_F_LEN) / 1024);

	எனில் (*image->அடுக்கு_prot != அடுக்கு_PROT_மதிப்பு) {
		log_err("stack busted, cannot continue\n");
		திரும்பு -EFAULT;
	}
	loader = (rcode_func)(வெறுமை *)rcode_reloc_and_jump + image->reloc_offset;
	log_debug("Jumping via %p to %lx - image %p size %x load %lx\n", loader,
		  image->entry_point, image, image->size, image->load_addr);

	log_debug("unc_len %lx\n",
		  image->rcode_buf - map_sysmem(image->load_addr, image->size));
	எனில் (DEBUG_JUMP) {
		rcode_reloc_and_jump(image);
	} இல்லையெனில் {
		/*
		 * Must disable LOG_DEBUG since the decompressor cannot call
		 * log functions, printf(), etc.
		 */
		_Static_assert(DEBUG_JUMP || !_DEBUG,
			       "Cannot have debug output from decompressor");
		ret = loader(image);
	}

	திரும்பு -EFAULT;
}
