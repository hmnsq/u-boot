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

DECLARE_GLOBAL_معلومات_PTR;

/* provide a way to jump straight into the relocation code, for debugging */
#وصف_ڪريو DEBUG_JUMP	0

تعداد {
	/* margin to allow for stack growth */
	RELOC_ڍير_MARGIN	= 0x800,

	/* align base address for DMA controllers which require it */
	بنياد_ALIGN		= 0x200,

	ڍير_PROT_قدر	= 0x51ce4697,
};

تعريفنوع صحيح (*rcode_func)(بنية spl_image_info *image);

ساكن صحيح تياري_layout(بنية spl_image_info *image, ulong *addrp)
{
	ulong بنياد, fdt_سائيز;
	ulong حد_بندي, rcode_بنياد;
	uint rcode_سائيز;
	صحيح buf_سائيز, margin;
	حرف *rcode_buf;

	حد_بندي = ALIGN(map_منزل_sysmem(&حد_بندي) - RELOC_ڍير_MARGIN, 8);
	image->ڍير_prot = map_sysmem(حد_بندي, حجم(uint));
	*image->ڍير_prot = ڍير_PROT_قدر;

	fdt_سائيز = fdt_totalsize(gd->fdt_blob);
	بنياد = ALIGN(map_منزل_sysmem(gd->fdt_blob) + fdt_سائيز + بنياد_ALIGN - 1,
		     بنياد_ALIGN);

	rcode_سائيز = rcode_پڇاڙي - rcode_شروعات;
	rcode_بنياد = حد_بندي - rcode_سائيز;
	buf_سائيز = rcode_بنياد - بنياد;
	uint need_سائيز = image->سائيز + image->fdt_سائيز;
	margin = buf_سائيز - need_سائيز;
	log_debug("spl_reloc %s->%s: margin%s%lx limit %lx fdt_size %lx base %lx avail %x image %x fdt %lx need %x\n",
		  spl_phase_name(spl_phase()), spl_phase_name(spl_phase() + 1),
		  margin >= 0 ? " " : " -", مطلق_قدر(margin), حد_بندي, fdt_سائيز, بنياد,
		  buf_سائيز, image->سائيز, image->fdt_سائيز, need_سائيز);
	إذا (margin < 0) {
		log_err("Image size %x but buffer is only %x\n", need_سائيز,
			buf_سائيز);
		أرجع -ENOSPC;
	}

	rcode_buf = map_sysmem(rcode_بنياد, rcode_سائيز);
	log_debug("_rcode_start %p: %x -- func %p %x\n", rcode_شروعات,
		  *(uint *)rcode_شروعات, تياري_layout, *(uint *)تياري_layout);

	image->reloc_وٿي = rcode_buf - rcode_شروعات;
	log_debug("_rcode start %lx base %lx size %x offset %lx\n",
		  (ulong)map_منزل_sysmem(rcode_شروعات), rcode_بنياد, rcode_سائيز,
		  image->reloc_وٿي);

	يادگيري_نقل(rcode_buf, rcode_شروعات, rcode_سائيز);

	image->buf = map_sysmem(بنياد, need_سائيز);
	image->fdt_buf = image->buf + image->سائيز;
	image->rcode_buf = rcode_buf;
	*addrp = بنياد;

	أرجع 0;
}

صحيح spl_reloc_prepare(بنية spl_image_info *image, ulong *addrp)
{
	صحيح ret;

	ret = تياري_layout(image, addrp);
	إذا (ret)
		أرجع ret;

	أرجع 0;
}

تعريفنوع فراغ __noreturn (*image_داخلا_noargs_t)(uint چڪري_جاچ, uint unc_len);

/* this is the relocation + jump code that is copied to the top of memory */
__rcode صحيح rcode_reloc_and_jump(بنية spl_image_info *image)
{
	image_داخلا_noargs_t داخلا_2 = (image_داخلا_noargs_t)image->داخلا_نقطو;
	u32 *dst;
	ulong image_len;
	size_t unc_len;
	صحيح ret, چڪري_جاچ;
	uint magic;

	dst = map_sysmem(image->ڀريو_addr, image->سائيز);
	unc_len = (فراغ *)image->rcode_buf - (فراغ *)dst;
	image_len = image->سائيز;
	إذا (*image->ڍير_prot != ڍير_PROT_قدر)
		أرجع -EFAULT;
	magic = get_unaligned_le32(image->buf);
	إذا (CONFIG_IS_ENABLED(LZMA)) {
		سائيز_T lzma_len = unc_len;

		ret = lzma_Buff_منزل_Buff_Decompress((u8 *)dst, &lzma_len,
					       image->buf, image_len);
		unc_len = lzma_len;
	} وإلا إذا (CONFIG_IS_ENABLED(GZIP)) {
		ret = gunzip(dst, unc_len, image->buf, &image_len);
	} وإلا إذا (CONFIG_IS_ENABLED(LZ4) && magic == LZ4F_MAGIC) {
		ret = ulz4fn(image->buf, image_len, dst, &unc_len);
		إذا (ret)
			أرجع ret;
	} وإلا {
		u32 *src, *پڇاڙي_2, *ptr;

		unc_len = image->سائيز;
		لكل (src = image->buf, پڇاڙي_2 = (فراغ *)src + image->سائيز,
		     ptr = dst; src < پڇاڙي_2;)
			*ptr++ = *src++;
	}
	إذا (*image->ڍير_prot != ڍير_PROT_قدر)
		أرجع -EFAULT;

	/* copy in the FDT if needed */
	إذا (image->fdt_سائيز)
		يادگيري_نقل(image->fdt_شروعات, image->fdt_buf, image->fdt_سائيز);

	چڪري_جاچ = crc8(0, (u8 *)dst, unc_len);

	/* jump to the entry point */
	داخلا_2(چڪري_جاچ, unc_len);
}

صحيح spl_reloc_jump(بنية spl_image_info *image, spl_jump_منزل_image_t jump)
{
	rcode_func loader;
	صحيح ret;

	log_debug("malloc usage %x bytes (%d KB of %d KB)\n", gd->malloc_ptr,
		  gd->malloc_ptr / 1024, CONFIG_VAL(SYS_MALLOC_F_LEN) / 1024);

	إذا (*image->ڍير_prot != ڍير_PROT_قدر) {
		log_err("stack busted, cannot continue\n");
		أرجع -EFAULT;
	}
	loader = (rcode_func)(فراغ *)rcode_reloc_and_jump + image->reloc_وٿي;
	log_debug("Jumping via %p to %lx - image %p size %x load %lx\n", loader,
		  image->داخلا_نقطو, image, image->سائيز, image->ڀريو_addr);

	log_debug("unc_len %lx\n",
		  image->rcode_buf - map_sysmem(image->ڀريو_addr, image->سائيز));
	إذا (DEBUG_JUMP) {
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
