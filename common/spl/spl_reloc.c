// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2024 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#અન્તર્ભાવ <gzip.h>
#અન્તર્ભાવ <image.h>
#અન્તર્ભાવ <log.h>
#અન્તર્ભાવ <mapmem.h>
#અન્તર્ભાવ <spl.h>
#અન્તર્ભાવ <asm/global_data.h>
#અન્તર્ભાવ <asm/io.h>
#અન્તર્ભાવ <asm/sections.h>
#અન્તર્ભાવ <asm/unaligned.h>
#અન્તર્ભાવ <linux/types.h>
#અન્તર્ભાવ <lzma/LzmaTypes.h>
#અન્તર્ભાવ <lzma/LzmaDec.h>
#અન્તર્ભાવ <lzma/LzmaTools.h>
#અન્તર્ભાવ <u-boot/crc.h>
#અન્તર્ભાવ <u-boot/lz4.h>

DECLARE_GLOBAL_DATA_PTR;

/* provide a way to jump straight into the relocation code, for debugging */
#define DEBUG_JUMP	0

ગણના {
	/* margin to allow for stack growth */
	RELOC_ઢગલો_MARGIN	= 0x800,

	/* align base address for DMA controllers which require it */
	BASE_ALIGN		= 0x200,

	ઢગલો_PROT_કિંમત	= 0x51ce4697,
};

પ્રકારવ્યાખ્યા પૂર્ણાંક (*rcode_func)(સંરચના spl_image_info *image);

સ્થિર પૂર્ણાંક setup_layout(સંરચના spl_image_info *image, ulong *addrp)
{
	ulong base, fdt_size;
	ulong limit, rcode_base;
	uint rcode_size;
	પૂર્ણાંક buf_size, margin;
	અક્ષર *rcode_buf;

	limit = ALIGN(map_to_sysmem(&limit) - RELOC_ઢગલો_MARGIN, 8);
	image->ઢગલો_prot = map_sysmem(limit, કદ(uint));
	*image->ઢગલો_prot = ઢગલો_PROT_કિંમત;

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
	જો (margin < 0) {
		log_err("Image size %x but buffer is only %x\n", need_size,
			buf_size);
		પાછા -ENOSPC;
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

	પાછા 0;
}

પૂર્ણાંક spl_reloc_prepare(સંરચના spl_image_info *image, ulong *addrp)
{
	પૂર્ણાંક ret;

	ret = setup_layout(image, addrp);
	જો (ret)
		પાછા ret;

	પાછા 0;
}

પ્રકારવ્યાખ્યા શૂન્ય __noreturn (*image_entry_noargs_t)(uint crc, uint unc_len);

/* this is the relocation + jump code that is copied to the top of memory */
__rcode પૂર્ણાંક rcode_reloc_and_jump(સંરચના spl_image_info *image)
{
	image_entry_noargs_t entry = (image_entry_noargs_t)image->entry_point;
	u32 *dst;
	ulong image_len;
	size_t unc_len;
	પૂર્ણાંક ret, crc;
	uint magic;

	dst = map_sysmem(image->load_addr, image->size);
	unc_len = (શૂન્ય *)image->rcode_buf - (શૂન્ય *)dst;
	image_len = image->size;
	જો (*image->ઢગલો_prot != ઢગલો_PROT_કિંમત)
		પાછા -EFAULT;
	magic = get_unaligned_le32(image->buf);
	જો (CONFIG_IS_ENABLED(LZMA)) {
		SizeT lzma_len = unc_len;

		ret = lzmaBuffToBuffDecompress((u8 *)dst, &lzma_len,
					       image->buf, image_len);
		unc_len = lzma_len;
	} નહીંતર જો (CONFIG_IS_ENABLED(GZIP)) {
		ret = gunzip(dst, unc_len, image->buf, &image_len);
	} નહીંતર જો (CONFIG_IS_ENABLED(LZ4) && magic == LZ4F_MAGIC) {
		ret = ulz4fn(image->buf, image_len, dst, &unc_len);
		જો (ret)
			પાછા ret;
	} નહીંતર {
		u32 *src, *end, *ptr;

		unc_len = image->size;
		માટે (src = image->buf, end = (શૂન્ય *)src + image->size,
		     ptr = dst; src < end;)
			*ptr++ = *src++;
	}
	જો (*image->ઢગલો_prot != ઢગલો_PROT_કિંમત)
		પાછા -EFAULT;

	/* copy in the FDT if needed */
	જો (image->fdt_size)
		memcpy(image->fdt_start, image->fdt_buf, image->fdt_size);

	crc = crc8(0, (u8 *)dst, unc_len);

	/* jump to the entry point */
	entry(crc, unc_len);
}

પૂર્ણાંક spl_reloc_jump(સંરચના spl_image_info *image, spl_jump_to_image_t jump)
{
	rcode_func loader;
	પૂર્ણાંક ret;

	log_debug("malloc usage %x bytes (%d KB of %d KB)\n", gd->malloc_ptr,
		  gd->malloc_ptr / 1024, CONFIG_VAL(SYS_MALLOC_F_LEN) / 1024);

	જો (*image->ઢગલો_prot != ઢગલો_PROT_કિંમત) {
		log_err("stack busted, cannot continue\n");
		પાછા -EFAULT;
	}
	loader = (rcode_func)(શૂન્ય *)rcode_reloc_and_jump + image->reloc_offset;
	log_debug("Jumping via %p to %lx - image %p size %x load %lx\n", loader,
		  image->entry_point, image, image->size, image->load_addr);

	log_debug("unc_len %lx\n",
		  image->rcode_buf - map_sysmem(image->load_addr, image->size));
	જો (DEBUG_JUMP) {
		rcode_reloc_and_jump(image);
	} નહીંતર {
		/*
		 * Must disable LOG_DEBUG since the decompressor cannot call
		 * log functions, printf(), etc.
		 */
		_Static_assert(DEBUG_JUMP || !_DEBUG,
			       "Cannot have debug output from decompressor");
		ret = loader(image);
	}

	પાછા -EFAULT;
}
