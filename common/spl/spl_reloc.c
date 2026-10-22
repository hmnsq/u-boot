// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2024 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#شامل <gzip.h>
#شامل <image.h>
#شامل <log.h>
#شامل <mapmem.h>
#شامل <spl.h>
#شامل <asm/global_data.h>
#شامل <asm/io.h>
#شامل <asm/sections.h>
#شامل <asm/unaligned.h>
#شامل <linux/types.h>
#شامل <lzma/LzmaTypes.h>
#شامل <lzma/LzmaDec.h>
#شامل <lzma/LzmaTools.h>
#شامل <u-boot/crc.h>
#شامل <u-boot/lz4.h>

DECLARE_GLOBAL_ڈیٹا_PTR;

/* provide a way to jump straight into the relocation code, for debugging */
#تعریف_کرو DEBUG_JUMP	0

شمار {
	/* margin to allow for stack growth */
	RELOC_ڈھیر_MARGIN	= 0x800,

	/* align base address for DMA controllers which require it */
	بنیاد_ALIGN		= 0x200,

	ڈھیر_PROT_قدر	= 0x51ce4697,
};

قسمتعریف صحیح (*rcode_func)(ساخت spl_image_info *image);

ساکن صحیح تیاری_layout(ساخت spl_image_info *image, ulong *addrp)
{
	ulong بنیاد, fdt_سائز;
	ulong حد_بندی, rcode_بنیاد;
	uint rcode_سائز;
	صحیح buf_سائز, margin;
	حرف *rcode_buf;

	حد_بندی = ALIGN(map_مقصد_sysmem(&حد_بندی) - RELOC_ڈھیر_MARGIN, 8);
	image->ڈھیر_prot = map_sysmem(حد_بندی, حجم(uint));
	*image->ڈھیر_prot = ڈھیر_PROT_قدر;

	fdt_سائز = fdt_totalsize(gd->fdt_blob);
	بنیاد = ALIGN(map_مقصد_sysmem(gd->fdt_blob) + fdt_سائز + بنیاد_ALIGN - 1,
		     بنیاد_ALIGN);

	rcode_سائز = rcode_انتہا - rcode_آغاز;
	rcode_بنیاد = حد_بندی - rcode_سائز;
	buf_سائز = rcode_بنیاد - بنیاد;
	uint need_سائز = image->سائز + image->fdt_سائز;
	margin = buf_سائز - need_سائز;
	log_debug("spl_reloc %s->%s: margin%s%lx limit %lx fdt_size %lx base %lx avail %x image %x fdt %lx need %x\n",
		  spl_phase_name(spl_phase()), spl_phase_name(spl_phase() + 1),
		  margin >= 0 ? " " : " -", مطلق_قدر(margin), حد_بندی, fdt_سائز, بنیاد,
		  buf_سائز, image->سائز, image->fdt_سائز, need_سائز);
	اگر (margin < 0) {
		log_err("Image size %x but buffer is only %x\n", need_سائز,
			buf_سائز);
		واپس -ENOSPC;
	}

	rcode_buf = map_sysmem(rcode_بنیاد, rcode_سائز);
	log_debug("_rcode_start %p: %x -- func %p %x\n", rcode_آغاز,
		  *(uint *)rcode_آغاز, تیاری_layout, *(uint *)تیاری_layout);

	image->reloc_انحراف = rcode_buf - rcode_آغاز;
	log_debug("_rcode start %lx base %lx size %x offset %lx\n",
		  (ulong)map_مقصد_sysmem(rcode_آغاز), rcode_بنیاد, rcode_سائز,
		  image->reloc_انحراف);

	حافظہ_نقل(rcode_buf, rcode_آغاز, rcode_سائز);

	image->buf = map_sysmem(بنیاد, need_سائز);
	image->fdt_buf = image->buf + image->سائز;
	image->rcode_buf = rcode_buf;
	*addrp = بنیاد;

	واپس 0;
}

صحیح spl_reloc_prepare(ساخت spl_image_info *image, ulong *addrp)
{
	صحیح ret;

	ret = تیاری_layout(image, addrp);
	اگر (ret)
		واپس ret;

	واپس 0;
}

قسمتعریف خالی __noreturn (*image_اندراج_noargs_t)(uint چکری_جانچ, uint unc_len);

/* this is the relocation + jump code that is copied to the top of memory */
__rcode صحیح rcode_reloc_and_jump(ساخت spl_image_info *image)
{
	image_اندراج_noargs_t اندراج_2 = (image_اندراج_noargs_t)image->اندراج_نقطہ;
	u32 *dst;
	ulong image_len;
	size_t unc_len;
	صحیح ret, چکری_جانچ;
	uint magic;

	dst = map_sysmem(image->بھرو_addr, image->سائز);
	unc_len = (خالی *)image->rcode_buf - (خالی *)dst;
	image_len = image->سائز;
	اگر (*image->ڈھیر_prot != ڈھیر_PROT_قدر)
		واپس -EFAULT;
	magic = get_unaligned_le32(image->buf);
	اگر (CONFIG_IS_ENABLED(LZMA)) {
		سائز_T lzma_len = unc_len;

		ret = lzma_Buff_مقصد_Buff_Decompress((u8 *)dst, &lzma_len,
					       image->buf, image_len);
		unc_len = lzma_len;
	} ورنہ اگر (CONFIG_IS_ENABLED(GZIP)) {
		ret = gunzip(dst, unc_len, image->buf, &image_len);
	} ورنہ اگر (CONFIG_IS_ENABLED(LZ4) && magic == LZ4F_MAGIC) {
		ret = ulz4fn(image->buf, image_len, dst, &unc_len);
		اگر (ret)
			واپس ret;
	} ورنہ {
		u32 *src, *انتہا_2, *ptr;

		unc_len = image->سائز;
		برائے (src = image->buf, انتہا_2 = (خالی *)src + image->سائز,
		     ptr = dst; src < انتہا_2;)
			*ptr++ = *src++;
	}
	اگر (*image->ڈھیر_prot != ڈھیر_PROT_قدر)
		واپس -EFAULT;

	/* copy in the FDT if needed */
	اگر (image->fdt_سائز)
		حافظہ_نقل(image->fdt_آغاز, image->fdt_buf, image->fdt_سائز);

	چکری_جانچ = crc8(0, (u8 *)dst, unc_len);

	/* jump to the entry point */
	اندراج_2(چکری_جانچ, unc_len);
}

صحیح spl_reloc_jump(ساخت spl_image_info *image, spl_jump_مقصد_image_t jump)
{
	rcode_func loader;
	صحیح ret;

	log_debug("malloc usage %x bytes (%d KB of %d KB)\n", gd->malloc_ptr,
		  gd->malloc_ptr / 1024, CONFIG_VAL(SYS_MALLOC_F_LEN) / 1024);

	اگر (*image->ڈھیر_prot != ڈھیر_PROT_قدر) {
		log_err("stack busted, cannot continue\n");
		واپس -EFAULT;
	}
	loader = (rcode_func)(خالی *)rcode_reloc_and_jump + image->reloc_انحراف;
	log_debug("Jumping via %p to %lx - image %p size %x load %lx\n", loader,
		  image->اندراج_نقطہ, image, image->سائز, image->بھرو_addr);

	log_debug("unc_len %lx\n",
		  image->rcode_buf - map_sysmem(image->بھرو_addr, image->سائز));
	اگر (DEBUG_JUMP) {
		rcode_reloc_and_jump(image);
	} ورنہ {
		/*
		 * Must disable LOG_DEBUG since the decompressor cannot call
		 * log functions, printf(), etc.
		 */
		_Static_assert(DEBUG_JUMP || !_DEBUG,
			       "Cannot have debug output from decompressor");
		ret = loader(image);
	}

	واپس -EFAULT;
}
