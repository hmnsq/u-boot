// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2024 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#অন্তর্ভাব <gzip.h>
#অন্তর্ভাব <image.h>
#অন্তর্ভাব <log.h>
#অন্তর্ভাব <mapmem.h>
#অন্তর্ভাব <spl.h>
#অন্তর্ভাব <asm/global_data.h>
#অন্তর্ভাব <asm/io.h>
#অন্তর্ভাব <asm/sections.h>
#অন্তর্ভাব <asm/unaligned.h>
#অন্তর্ভাব <linux/types.h>
#অন্তর্ভাব <lzma/LzmaTypes.h>
#অন্তর্ভাব <lzma/LzmaDec.h>
#অন্তর্ভাব <lzma/LzmaTools.h>
#অন্তর্ভাব <u-boot/crc.h>
#অন্তর্ভাব <u-boot/lz4.h>

DECLARE_GLOBAL_তথ্য_PTR;

/* provide a way to jump straight into the relocation code, for debugging */
#define DEBUG_JUMP	0

গণনা {
	/* margin to allow for stack growth */
	RELOC_স্তূপ_MARGIN	= 0x800,

	/* align base address for DMA controllers which require it */
	BASE_ALIGN		= 0x200,

	স্তূপ_PROT_মান	= 0x51ce4697,
};

প্রকারলেখ পূর্ণসংখ্যা (*rcode_func)(কাঠামো spl_image_info *image);

স্থির পূর্ণসংখ্যা setup_layout(কাঠামো spl_image_info *image, ulong *addrp)
{
	ulong base, fdt_আকার;
	ulong limit, rcode_base;
	uint rcode_আকার;
	পূর্ণসংখ্যা buf_আকার, margin;
	অক্ষর *rcode_buf;

	limit = ALIGN(map_to_sysmem(&limit) - RELOC_স্তূপ_MARGIN, 8);
	image->স্তূপ_prot = map_sysmem(limit, আকার(uint));
	*image->স্তূপ_prot = স্তূপ_PROT_মান;

	fdt_আকার = fdt_totalsize(gd->fdt_blob);
	base = ALIGN(map_to_sysmem(gd->fdt_blob) + fdt_আকার + BASE_ALIGN - 1,
		     BASE_ALIGN);

	rcode_আকার = _rcode_end - _rcode_start;
	rcode_base = limit - rcode_আকার;
	buf_আকার = rcode_base - base;
	uint need_আকার = image->আকার_2 + image->fdt_আকার;
	margin = buf_আকার - need_আকার;
	log_debug("spl_reloc %s->%s: margin%s%lx limit %lx fdt_size %lx base %lx avail %x image %x fdt %lx need %x\n",
		  spl_phase_name(spl_phase()), spl_phase_name(spl_phase() + 1),
		  margin >= 0 ? " " : " -", abs(margin), limit, fdt_আকার, base,
		  buf_আকার, image->আকার_2, image->fdt_আকার, need_আকার);
	যদি (margin < 0) {
		log_err("Image size %x but buffer is only %x\n", need_আকার,
			buf_আকার);
		ফেরত -ENOSPC;
	}

	rcode_buf = map_sysmem(rcode_base, rcode_আকার);
	log_debug("_rcode_start %p: %x -- func %p %x\n", _rcode_start,
		  *(uint *)_rcode_start, setup_layout, *(uint *)setup_layout);

	image->reloc_offset = rcode_buf - _rcode_start;
	log_debug("_rcode start %lx base %lx size %x offset %lx\n",
		  (ulong)map_to_sysmem(_rcode_start), rcode_base, rcode_আকার,
		  image->reloc_offset);

	memcpy(rcode_buf, _rcode_start, rcode_আকার);

	image->buf = map_sysmem(base, need_আকার);
	image->fdt_buf = image->buf + image->আকার_2;
	image->rcode_buf = rcode_buf;
	*addrp = base;

	ফেরত 0;
}

পূর্ণসংখ্যা spl_reloc_prepare(কাঠামো spl_image_info *image, ulong *addrp)
{
	পূর্ণসংখ্যা ret;

	ret = setup_layout(image, addrp);
	যদি (ret)
		ফেরত ret;

	ফেরত 0;
}

প্রকারলেখ শূন্য __noreturn (*image_entry_noargs_t)(uint crc, uint unc_len);

/* this is the relocation + jump code that is copied to the top of memory */
__rcode পূর্ণসংখ্যা rcode_reloc_and_jump(কাঠামো spl_image_info *image)
{
	image_entry_noargs_t entry = (image_entry_noargs_t)image->entry_point;
	u32 *dst;
	ulong image_len;
	size_t unc_len;
	পূর্ণসংখ্যা ret, crc;
	uint magic;

	dst = map_sysmem(image->load_addr, image->আকার_2);
	unc_len = (শূন্য *)image->rcode_buf - (শূন্য *)dst;
	image_len = image->আকার_2;
	যদি (*image->স্তূপ_prot != স্তূপ_PROT_মান)
		ফেরত -EFAULT;
	magic = get_unaligned_le32(image->buf);
	যদি (CONFIG_IS_ENABLED(LZMA)) {
		আকার_T lzma_len = unc_len;

		ret = lzmaBuffToBuffDecompress((u8 *)dst, &lzma_len,
					       image->buf, image_len);
		unc_len = lzma_len;
	} অন্যথা যদি (CONFIG_IS_ENABLED(GZIP)) {
		ret = gunzip(dst, unc_len, image->buf, &image_len);
	} অন্যথা যদি (CONFIG_IS_ENABLED(LZ4) && magic == LZ4F_MAGIC) {
		ret = ulz4fn(image->buf, image_len, dst, &unc_len);
		যদি (ret)
			ফেরত ret;
	} অন্যথা {
		u32 *src, *end, *ptr;

		unc_len = image->আকার_2;
		জন্য (src = image->buf, end = (শূন্য *)src + image->আকার_2,
		     ptr = dst; src < end;)
			*ptr++ = *src++;
	}
	যদি (*image->স্তূপ_prot != স্তূপ_PROT_মান)
		ফেরত -EFAULT;

	/* copy in the FDT if needed */
	যদি (image->fdt_আকার)
		memcpy(image->fdt_start, image->fdt_buf, image->fdt_আকার);

	crc = crc8(0, (u8 *)dst, unc_len);

	/* jump to the entry point */
	entry(crc, unc_len);
}

পূর্ণসংখ্যা spl_reloc_jump(কাঠামো spl_image_info *image, spl_jump_to_image_t jump)
{
	rcode_func loader;
	পূর্ণসংখ্যা ret;

	log_debug("malloc usage %x bytes (%d KB of %d KB)\n", gd->malloc_ptr,
		  gd->malloc_ptr / 1024, CONFIG_VAL(SYS_MALLOC_F_LEN) / 1024);

	যদি (*image->স্তূপ_prot != স্তূপ_PROT_মান) {
		log_err("stack busted, cannot continue\n");
		ফেরত -EFAULT;
	}
	loader = (rcode_func)(শূন্য *)rcode_reloc_and_jump + image->reloc_offset;
	log_debug("Jumping via %p to %lx - image %p size %x load %lx\n", loader,
		  image->entry_point, image, image->আকার_2, image->load_addr);

	log_debug("unc_len %lx\n",
		  image->rcode_buf - map_sysmem(image->load_addr, image->আকার_2));
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

	ফেরত -EFAULT;
}
