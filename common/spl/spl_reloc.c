// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2024 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#ଅନ୍ତର୍ଭାଵ <gzip.h>
#ଅନ୍ତର୍ଭାଵ <image.h>
#ଅନ୍ତର୍ଭାଵ <log.h>
#ଅନ୍ତର୍ଭାଵ <mapmem.h>
#ଅନ୍ତର୍ଭାଵ <spl.h>
#ଅନ୍ତର୍ଭାଵ <asm/global_data.h>
#ଅନ୍ତର୍ଭାଵ <asm/io.h>
#ଅନ୍ତର୍ଭାଵ <asm/sections.h>
#ଅନ୍ତର୍ଭାଵ <asm/unaligned.h>
#ଅନ୍ତର୍ଭାଵ <linux/types.h>
#ଅନ୍ତର୍ଭାଵ <lzma/LzmaTypes.h>
#ଅନ୍ତର୍ଭାଵ <lzma/LzmaDec.h>
#ଅନ୍ତର୍ଭାଵ <lzma/LzmaTools.h>
#ଅନ୍ତର୍ଭାଵ <u-boot/crc.h>
#ଅନ୍ତର୍ଭାଵ <u-boot/lz4.h>

DECLARE_GLOBAL_DATA_PTR;

/* provide a way to jump straight into the relocation code, for debugging */
#define DEBUG_JUMP	0

ଗଣନା {
	/* margin to allow for stack growth */
	RELOC_ଗଦା_MARGIN	= 0x800,

	/* align base address for DMA controllers which require it */
	BASE_ALIGN		= 0x200,

	ଗଦା_PROT_ମୂଲ୍ୟ	= 0x51ce4697,
};

ପ୍ରକାରବ୍ୟାଖ୍ୟା ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*rcode_func)(ସଂରଚନା spl_image_info *image);

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା setup_layout(ସଂରଚନା spl_image_info *image, ulong *addrp)
{
	ulong base, fdt_size;
	ulong limit, rcode_base;
	uint rcode_size;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା buf_size, margin;
	ଅକ୍ଷର *rcode_buf;

	limit = ALIGN(map_to_sysmem(&limit) - RELOC_ଗଦା_MARGIN, 8);
	image->ଗଦା_prot = map_sysmem(limit, ଆକାର(uint));
	*image->ଗଦା_prot = ଗଦା_PROT_ମୂଲ୍ୟ;

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
	ଯଦି (margin < 0) {
		log_err("Image size %x but buffer is only %x\n", need_size,
			buf_size);
		ଫେରନ୍ତୁ -ENOSPC;
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

	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା spl_reloc_prepare(ସଂରଚନା spl_image_info *image, ulong *addrp)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	ret = setup_layout(image, addrp);
	ଯଦି (ret)
		ଫେରନ୍ତୁ ret;

	ଫେରନ୍ତୁ 0;
}

ପ୍ରକାରବ୍ୟାଖ୍ୟା ଶୂନ୍ୟ __noreturn (*image_entry_noargs_t)(uint crc, uint unc_len);

/* this is the relocation + jump code that is copied to the top of memory */
__rcode ପୂର୍ଣ୍ଣସଂଖ୍ୟା rcode_reloc_and_jump(ସଂରଚନା spl_image_info *image)
{
	image_entry_noargs_t entry = (image_entry_noargs_t)image->entry_point;
	u32 *dst;
	ulong image_len;
	size_t unc_len;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret, crc;
	uint magic;

	dst = map_sysmem(image->load_addr, image->size);
	unc_len = (ଶୂନ୍ୟ *)image->rcode_buf - (ଶୂନ୍ୟ *)dst;
	image_len = image->size;
	ଯଦି (*image->ଗଦା_prot != ଗଦା_PROT_ମୂଲ୍ୟ)
		ଫେରନ୍ତୁ -EFAULT;
	magic = get_unaligned_le32(image->buf);
	ଯଦି (CONFIG_IS_ENABLED(LZMA)) {
		SizeT lzma_len = unc_len;

		ret = lzmaBuffToBuffDecompress((u8 *)dst, &lzma_len,
					       image->buf, image_len);
		unc_len = lzma_len;
	} ଅନ୍ୟଥା ଯଦି (CONFIG_IS_ENABLED(GZIP)) {
		ret = gunzip(dst, unc_len, image->buf, &image_len);
	} ଅନ୍ୟଥା ଯଦି (CONFIG_IS_ENABLED(LZ4) && magic == LZ4F_MAGIC) {
		ret = ulz4fn(image->buf, image_len, dst, &unc_len);
		ଯଦି (ret)
			ଫେରନ୍ତୁ ret;
	} ଅନ୍ୟଥା {
		u32 *src, *end, *ptr;

		unc_len = image->size;
		ପାଇଁ (src = image->buf, end = (ଶୂନ୍ୟ *)src + image->size,
		     ptr = dst; src < end;)
			*ptr++ = *src++;
	}
	ଯଦି (*image->ଗଦା_prot != ଗଦା_PROT_ମୂଲ୍ୟ)
		ଫେରନ୍ତୁ -EFAULT;

	/* copy in the FDT if needed */
	ଯଦି (image->fdt_size)
		memcpy(image->fdt_start, image->fdt_buf, image->fdt_size);

	crc = crc8(0, (u8 *)dst, unc_len);

	/* jump to the entry point */
	entry(crc, unc_len);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା spl_reloc_jump(ସଂରଚନା spl_image_info *image, spl_jump_to_image_t jump)
{
	rcode_func loader;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	log_debug("malloc usage %x bytes (%d KB of %d KB)\n", gd->malloc_ptr,
		  gd->malloc_ptr / 1024, CONFIG_VAL(SYS_MALLOC_F_LEN) / 1024);

	ଯଦି (*image->ଗଦା_prot != ଗଦା_PROT_ମୂଲ୍ୟ) {
		log_err("stack busted, cannot continue\n");
		ଫେରନ୍ତୁ -EFAULT;
	}
	loader = (rcode_func)(ଶୂନ୍ୟ *)rcode_reloc_and_jump + image->reloc_offset;
	log_debug("Jumping via %p to %lx - image %p size %x load %lx\n", loader,
		  image->entry_point, image, image->size, image->load_addr);

	log_debug("unc_len %lx\n",
		  image->rcode_buf - map_sysmem(image->load_addr, image->size));
	ଯଦି (DEBUG_JUMP) {
		rcode_reloc_and_jump(image);
	} ଅନ୍ୟଥା {
		/*
		 * Must disable LOG_DEBUG since the decompressor cannot call
		 * log functions, printf(), etc.
		 */
		_Static_assert(DEBUG_JUMP || !_DEBUG,
			       "Cannot have debug output from decompressor");
		ret = loader(image);
	}

	ଫେରନ୍ତୁ -EFAULT;
}
