// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2024 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#ضمن <gzip.h>
#ضمن <image.h>
#ضمن <log.h>
#ضمن <mapmem.h>
#ضمن <spl.h>
#ضمن <asm/global_data.h>
#ضمن <asm/io.h>
#ضمن <asm/sections.h>
#ضمن <asm/unaligned.h>
#ضمن <linux/types.h>
#ضمن <lzma/LzmaTypes.h>
#ضمن <lzma/LzmaDec.h>
#ضمن <lzma/LzmaTools.h>
#ضمن <u-boot/crc.h>
#ضمن <u-boot/lz4.h>

DECLARE_GLOBAL_DATA_PTR;

/* provide a way to jump straight into the relocation code, for debugging */
#define DEBUG_JUMP	0

تعداد {
	/* margin to allow for stack growth */
	RELOC_STACK_MARGIN	= 0x800,

	/* align base address for DMA controllers which require it */
	BASE_ALIGN		= 0x200,

	STACK_PROT_VALUE	= 0x51ce4697,
};

تعريفنوع صحيح (*rcode_func)(بنية spl_image_info *image);

ساكن صحيح setup_layout(بنية spl_image_info *image, ulong *addrp)
{
	ulong base, fdt_size;
	ulong limit, rcode_base;
	uint rcode_size;
	صحيح buf_size, margin;
	حرف *rcode_buf;

	limit = ALIGN(map_to_sysmem(&limit) - RELOC_STACK_MARGIN, 8);
	image->stack_prot = map_sysmem(limit, حجم(uint));
	*image->stack_prot = STACK_PROT_VALUE;

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
	𐡄𐡍 (margin < 0) {
		log_err("Image size %x but buffer is only %x\n", need_size,
			buf_size);
		أرجع -ENOSPC;
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

	أرجع 0;
}

صحيح spl_reloc_prepare(بنية spl_image_info *image, ulong *addrp)
{
	صحيح ret;

	ret = setup_layout(image, addrp);
	𐡄𐡍 (ret)
		أرجع ret;

	أرجع 0;
}

تعريفنوع فراغ __noreturn (*image_entry_noargs_t)(uint crc, uint unc_len);

/* this is the relocation + jump code that is copied to the top of memory */
__rcode صحيح rcode_reloc_and_jump(بنية spl_image_info *image)
{
	image_entry_noargs_t entry = (image_entry_noargs_t)image->entry_point;
	u32 *dst;
	ulong image_len;
	size_t unc_len;
	صحيح ret, crc;
	uint magic;

	dst = map_sysmem(image->load_addr, image->size);
	unc_len = (فراغ *)image->rcode_buf - (فراغ *)dst;
	image_len = image->size;
	𐡄𐡍 (*image->stack_prot != STACK_PROT_VALUE)
		أرجع -EFAULT;
	magic = get_unaligned_le32(image->buf);
	𐡄𐡍 (CONFIG_IS_ENABLED(LZMA)) {
		SizeT lzma_len = unc_len;

		ret = lzmaBuffToBuffDecompress((u8 *)dst, &lzma_len,
					       image->buf, image_len);
		unc_len = lzma_len;
	} وإلا 𐡄𐡍 (CONFIG_IS_ENABLED(GZIP)) {
		ret = gunzip(dst, unc_len, image->buf, &image_len);
	} وإلا 𐡄𐡍 (CONFIG_IS_ENABLED(LZ4) && magic == LZ4F_MAGIC) {
		ret = ulz4fn(image->buf, image_len, dst, &unc_len);
		𐡄𐡍 (ret)
			أرجع ret;
	} وإلا {
		u32 *src, *end, *ptr;

		unc_len = image->size;
		لكل (src = image->buf, end = (فراغ *)src + image->size,
		     ptr = dst; src < end;)
			*ptr++ = *src++;
	}
	𐡄𐡍 (*image->stack_prot != STACK_PROT_VALUE)
		أرجع -EFAULT;

	/* copy in the FDT if needed */
	𐡄𐡍 (image->fdt_size)
		memcpy(image->fdt_start, image->fdt_buf, image->fdt_size);

	crc = crc8(0, (u8 *)dst, unc_len);

	/* jump to the entry point */
	entry(crc, unc_len);
}

صحيح spl_reloc_jump(بنية spl_image_info *image, spl_jump_to_image_t jump)
{
	rcode_func loader;
	صحيح ret;

	log_debug("malloc usage %x bytes (%d KB of %d KB)\n", gd->malloc_ptr,
		  gd->malloc_ptr / 1024, CONFIG_VAL(SYS_MALLOC_F_LEN) / 1024);

	𐡄𐡍 (*image->stack_prot != STACK_PROT_VALUE) {
		log_err("stack busted, cannot continue\n");
		أرجع -EFAULT;
	}
	loader = (rcode_func)(فراغ *)rcode_reloc_and_jump + image->reloc_offset;
	log_debug("Jumping via %p to %lx - image %p size %x load %lx\n", loader,
		  image->entry_point, image, image->size, image->load_addr);

	log_debug("unc_len %lx\n",
		  image->rcode_buf - map_sysmem(image->load_addr, image->size));
	𐡄𐡍 (DEBUG_JUMP) {
		rcode_reloc_and_jump(image);
	} وإلا {
		/*
		 * Must disable LOG_DEBUG since the decompressor cannot call
		 * log functions, printf(), etc.
		 */
		_Static_assert(DEBUG_JUMP || !_DEBUG,
			       "Cannot have debug output from decompressor");
		ret = loader(image);
	}

	أرجع -EFAULT;
}
