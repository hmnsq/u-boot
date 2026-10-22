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

DECLARE_GLOBAL_انکڑے_PTR;

/* provide a way to jump straight into the relocation code, for debugging */
#تعریف_کرو DEBUG_JUMP	0

تعداد {
	/* margin to allow for stack growth */
	RELOC_ڈھیر_MARGIN	= 0x800,

	/* align base address for DMA controllers which require it */
	ادھار_ALIGN		= 0x200,

	ڈھیر_PROT_قدر	= 0x51ce4697,
};

تعريفنوع صحيح (*rcode_func)(بنية spl_image_info *image);

ساكن صحيح تیاری_layout(بنية spl_image_info *image, ulong *addrp)
{
	ulong ادھار, fdt_ناپ;
	ulong حد_بندی, rcode_ادھار;
	uint rcode_ناپ;
	صحيح buf_ناپ, margin;
	حرف *rcode_buf;

	حد_بندی = ALIGN(map_منزل_sysmem(&حد_بندی) - RELOC_ڈھیر_MARGIN, 8);
	image->ڈھیر_prot = map_sysmem(حد_بندی, حجم(uint));
	*image->ڈھیر_prot = ڈھیر_PROT_قدر;

	fdt_ناپ = fdt_totalsize(gd->fdt_blob);
	ادھار = ALIGN(map_منزل_sysmem(gd->fdt_blob) + fdt_ناپ + ادھار_ALIGN - 1,
		     ادھار_ALIGN);

	rcode_ناپ = rcode_سرا - rcode_شروع;
	rcode_ادھار = حد_بندی - rcode_ناپ;
	buf_ناپ = rcode_ادھار - ادھار;
	uint need_ناپ = image->ناپ + image->fdt_ناپ;
	margin = buf_ناپ - need_ناپ;
	log_debug("spl_reloc %s->%s: margin%s%lx limit %lx fdt_size %lx base %lx avail %x image %x fdt %lx need %x\n",
		  spl_phase_name(spl_phase()), spl_phase_name(spl_phase() + 1),
		  margin >= 0 ? " " : " -", مطلق_قدر(margin), حد_بندی, fdt_ناپ, ادھار,
		  buf_ناپ, image->ناپ, image->fdt_ناپ, need_ناپ);
	إذا (margin < 0) {
		log_err("Image size %x but buffer is only %x\n", need_ناپ,
			buf_ناپ);
		أرجع -ENOSPC;
	}

	rcode_buf = map_sysmem(rcode_ادھار, rcode_ناپ);
	log_debug("_rcode_start %p: %x -- func %p %x\n", rcode_شروع,
		  *(uint *)rcode_شروع, تیاری_layout, *(uint *)تیاری_layout);

	image->reloc_وتھ = rcode_buf - rcode_شروع;
	log_debug("_rcode start %lx base %lx size %x offset %lx\n",
		  (ulong)map_منزل_sysmem(rcode_شروع), rcode_ادھار, rcode_ناپ,
		  image->reloc_وتھ);

	حافظہ_نقل(rcode_buf, rcode_شروع, rcode_ناپ);

	image->buf = map_sysmem(ادھار, need_ناپ);
	image->fdt_buf = image->buf + image->ناپ;
	image->rcode_buf = rcode_buf;
	*addrp = ادھار;

	أرجع 0;
}

صحيح spl_reloc_prepare(بنية spl_image_info *image, ulong *addrp)
{
	صحيح ret;

	ret = تیاری_layout(image, addrp);
	إذا (ret)
		أرجع ret;

	أرجع 0;
}

تعريفنوع فراغ __noreturn (*image_اندراج_noargs_t)(uint چکری_جانچ, uint unc_len);

/* this is the relocation + jump code that is copied to the top of memory */
__rcode صحيح rcode_reloc_and_jump(بنية spl_image_info *image)
{
	image_اندراج_noargs_t اندراج = (image_اندراج_noargs_t)image->اندراج_نقطہ;
	u32 *dst;
	ulong image_len;
	size_t unc_len;
	صحيح ret, چکری_جانچ;
	uint magic;

	dst = map_sysmem(image->لدو_addr, image->ناپ);
	unc_len = (فراغ *)image->rcode_buf - (فراغ *)dst;
	image_len = image->ناپ;
	إذا (*image->ڈھیر_prot != ڈھیر_PROT_قدر)
		أرجع -EFAULT;
	magic = get_unaligned_le32(image->buf);
	إذا (CONFIG_IS_ENABLED(LZMA)) {
		ناپ_T lzma_len = unc_len;

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
		u32 *src, *سرا_2, *ptr;

		unc_len = image->ناپ;
		لكل (src = image->buf, سرا_2 = (فراغ *)src + image->ناپ,
		     ptr = dst; src < سرا_2;)
			*ptr++ = *src++;
	}
	إذا (*image->ڈھیر_prot != ڈھیر_PROT_قدر)
		أرجع -EFAULT;

	/* copy in the FDT if needed */
	إذا (image->fdt_ناپ)
		حافظہ_نقل(image->fdt_شروع, image->fdt_buf, image->fdt_ناپ);

	چکری_جانچ = crc8(0, (u8 *)dst, unc_len);

	/* jump to the entry point */
	اندراج(چکری_جانچ, unc_len);
}

صحيح spl_reloc_jump(بنية spl_image_info *image, spl_jump_منزل_image_t jump)
{
	rcode_func loader;
	صحيح ret;

	log_debug("malloc usage %x bytes (%d KB of %d KB)\n", gd->malloc_ptr,
		  gd->malloc_ptr / 1024, CONFIG_VAL(SYS_MALLOC_F_LEN) / 1024);

	إذا (*image->ڈھیر_prot != ڈھیر_PROT_قدر) {
		log_err("stack busted, cannot continue\n");
		أرجع -EFAULT;
	}
	loader = (rcode_func)(فراغ *)rcode_reloc_and_jump + image->reloc_وتھ;
	log_debug("Jumping via %p to %lx - image %p size %x load %lx\n", loader,
		  image->اندراج_نقطہ, image, image->ناپ, image->لدو_addr);

	log_debug("unc_len %lx\n",
		  image->rcode_buf - map_sysmem(image->لدو_addr, image->ناپ));
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
