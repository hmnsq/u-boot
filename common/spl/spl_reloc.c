// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2024 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#অন্তর্ভাৱ <gzip.h>
#অন্তর্ভাৱ <image.h>
#অন্তর্ভাৱ <log.h>
#অন্তর্ভাৱ <mapmem.h>
#অন্তর্ভাৱ <spl.h>
#অন্তর্ভাৱ <asm/global_data.h>
#অন্তর্ভাৱ <asm/io.h>
#অন্তর্ভাৱ <asm/sections.h>
#অন্তর্ভাৱ <asm/unaligned.h>
#অন্তর্ভাৱ <linux/types.h>
#অন্তর্ভাৱ <lzma/LzmaTypes.h>
#অন্তর্ভাৱ <lzma/LzmaDec.h>
#অন্তর্ভাৱ <lzma/LzmaTools.h>
#অন্তর্ভাৱ <u-boot/crc.h>
#অন্তর্ভাৱ <u-boot/lz4.h>

DECLARE_GLOBAL_DATA_PTR;

/* provide a way to jump straight into the relocation code, for debugging */
#define DEBUG_JUMP	0

গণনা {
	/* margin to allow for stack growth */
	RELOC_স্তূপ_MARGIN	= 0x800,

	/* align base address for DMA controllers which require it */
	BASE_ALIGN		= 0x200,

	স্তূপ_PROT_মান	= 0x51ce4697,
};

প্রকারনির্দেশ পূর্ণাঙ্ক (*rcode_func)(সংরচনা spl_image_info *image);

স্থির পূর্ণাঙ্ক setup_layout(সংরচনা spl_image_info *image, ulong *addrp)
{
	ulong base, fdt_size;
	ulong limit, rcode_base;
	uint rcode_size;
	পূর্ণাঙ্ক buf_size, margin;
	ৱর্ণ *rcode_buf;

	limit = ALIGN(map_to_sysmem(&limit) - RELOC_স্তূপ_MARGIN, 8);
	image->স্তূপ_prot = map_sysmem(limit, পরিমাণ(uint));
	*image->স্তূপ_prot = স্তূপ_PROT_মান;

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
	যদি (margin < 0) {
		log_err("Image size %x but buffer is only %x\n", need_size,
			buf_size);
		ঘূৰাই -ENOSPC;
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

	ঘূৰাই 0;
}

পূর্ণাঙ্ক spl_reloc_prepare(সংরচনা spl_image_info *image, ulong *addrp)
{
	পূর্ণাঙ্ক ret;

	ret = setup_layout(image, addrp);
	যদি (ret)
		ঘূৰাই ret;

	ঘূৰাই 0;
}

প্রকারনির্দেশ শূন্য __noreturn (*image_entry_noargs_t)(uint crc, uint unc_len);

/* this is the relocation + jump code that is copied to the top of memory */
__rcode পূর্ণাঙ্ক rcode_reloc_and_jump(সংরচনা spl_image_info *image)
{
	image_entry_noargs_t entry = (image_entry_noargs_t)image->entry_point;
	u32 *dst;
	ulong image_len;
	size_t unc_len;
	পূর্ণাঙ্ক ret, crc;
	uint magic;

	dst = map_sysmem(image->load_addr, image->size);
	unc_len = (শূন্য *)image->rcode_buf - (শূন্য *)dst;
	image_len = image->size;
	যদি (*image->স্তূপ_prot != স্তূপ_PROT_মান)
		ঘূৰাই -EFAULT;
	magic = get_unaligned_le32(image->buf);
	যদি (CONFIG_IS_ENABLED(LZMA)) {
		SizeT lzma_len = unc_len;

		ret = lzmaBuffToBuffDecompress((u8 *)dst, &lzma_len,
					       image->buf, image_len);
		unc_len = lzma_len;
	} অন্যথা যদি (CONFIG_IS_ENABLED(GZIP)) {
		ret = gunzip(dst, unc_len, image->buf, &image_len);
	} অন্যথা যদি (CONFIG_IS_ENABLED(LZ4) && magic == LZ4F_MAGIC) {
		ret = ulz4fn(image->buf, image_len, dst, &unc_len);
		যদি (ret)
			ঘূৰাই ret;
	} অন্যথা {
		u32 *src, *end, *ptr;

		unc_len = image->size;
		বাবে (src = image->buf, end = (শূন্য *)src + image->size,
		     ptr = dst; src < end;)
			*ptr++ = *src++;
	}
	যদি (*image->স্তূপ_prot != স্তূপ_PROT_মান)
		ঘূৰাই -EFAULT;

	/* copy in the FDT if needed */
	যদি (image->fdt_size)
		memcpy(image->fdt_start, image->fdt_buf, image->fdt_size);

	crc = crc8(0, (u8 *)dst, unc_len);

	/* jump to the entry point */
	entry(crc, unc_len);
}

পূর্ণাঙ্ক spl_reloc_jump(সংরচনা spl_image_info *image, spl_jump_to_image_t jump)
{
	rcode_func loader;
	পূর্ণাঙ্ক ret;

	log_debug("malloc usage %x bytes (%d KB of %d KB)\n", gd->malloc_ptr,
		  gd->malloc_ptr / 1024, CONFIG_VAL(SYS_MALLOC_F_LEN) / 1024);

	যদি (*image->স্তূপ_prot != স্তূপ_PROT_মান) {
		log_err("stack busted, cannot continue\n");
		ঘূৰাই -EFAULT;
	}
	loader = (rcode_func)(শূন্য *)rcode_reloc_and_jump + image->reloc_offset;
	log_debug("Jumping via %p to %lx - image %p size %x load %lx\n", loader,
		  image->entry_point, image, image->size, image->load_addr);

	log_debug("unc_len %lx\n",
		  image->rcode_buf - map_sysmem(image->load_addr, image->size));
	যদি (DEBUG_JUMP) {
		rcode_reloc_and_jump(image);
	} অন্যথা {
		/*
		 * Must disable LOG_DEBUG since the decompressor cannot call
		 * log functions, printf(), etc.
		 */
		_Static_assert(DEBUG_JUMP || !_DEBUG,
			       "Cannot have debug output from decompressor");
		ret = loader(image);
	}

	ঘূৰাই -EFAULT;
}
