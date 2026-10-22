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
#تعریف_کړه DEBUG_JUMP	0

تعداد {
	/* margin to allow for stack growth */
	RELOC_ډېری_MARGIN	= 0x800,

	/* align base address for DMA controllers which require it */
	بنسټ_ALIGN		= 0x200,

	ډېری_PROT_ارزښت	= 0x51ce4697,
};

تعريفنوع صحيح (*rcode_func)(بنية spl_image_info *image);

ساكن صحيح تیاری_layout(بنية spl_image_info *image, ulong *addrp)
{
	ulong بنسټ, fdt_اندازه;
	ulong حد, rcode_بنسټ;
	uint rcode_اندازه;
	صحيح buf_اندازه, margin;
	حرف *rcode_buf;

	حد = ALIGN(map_منزل_sysmem(&حد) - RELOC_ډېری_MARGIN, 8);
	image->ډېری_prot = map_sysmem(حد, حجم(uint));
	*image->ډېری_prot = ډېری_PROT_ارزښت;

	fdt_اندازه = fdt_totalsize(gd->fdt_blob);
	بنسټ = ALIGN(map_منزل_sysmem(gd->fdt_blob) + fdt_اندازه + بنسټ_ALIGN - 1,
		     بنسټ_ALIGN);

	rcode_اندازه = rcode_پای_ټکی - rcode_پیل;
	rcode_بنسټ = حد - rcode_اندازه;
	buf_اندازه = rcode_بنسټ - بنسټ;
	uint need_اندازه = image->اندازه + image->fdt_اندازه;
	margin = buf_اندازه - need_اندازه;
	log_debug("spl_reloc %s->%s: margin%s%lx limit %lx fdt_size %lx base %lx avail %x image %x fdt %lx need %x\n",
		  spl_phase_name(spl_phase()), spl_phase_name(spl_phase() + 1),
		  margin >= 0 ? " " : " -", مطلق_قیمت(margin), حد, fdt_اندازه, بنسټ,
		  buf_اندازه, image->اندازه, image->fdt_اندازه, need_اندازه);
	إذا (margin < 0) {
		log_err("Image size %x but buffer is only %x\n", need_اندازه,
			buf_اندازه);
		أرجع -ENOSPC;
	}

	rcode_buf = map_sysmem(rcode_بنسټ, rcode_اندازه);
	log_debug("_rcode_start %p: %x -- func %p %x\n", rcode_پیل,
		  *(uint *)rcode_پیل, تیاری_layout, *(uint *)تیاری_layout);

	image->reloc_دننه_واټن = rcode_buf - rcode_پیل;
	log_debug("_rcode start %lx base %lx size %x offset %lx\n",
		  (ulong)map_منزل_sysmem(rcode_پیل), rcode_بنسټ, rcode_اندازه,
		  image->reloc_دننه_واټن);

	حافظه_نقل(rcode_buf, rcode_پیل, rcode_اندازه);

	image->buf = map_sysmem(بنسټ, need_اندازه);
	image->fdt_buf = image->buf + image->اندازه;
	image->rcode_buf = rcode_buf;
	*addrp = بنسټ;

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

تعريفنوع فراغ __noreturn (*image_ثبت_noargs_t)(uint دوراني_ازموینه, uint unc_len);

/* this is the relocation + jump code that is copied to the top of memory */
__rcode صحيح rcode_reloc_and_jump(بنية spl_image_info *image)
{
	image_ثبت_noargs_t ثبت = (image_ثبت_noargs_t)image->ثبت_ټکی;
	u32 *dst;
	ulong image_len;
	size_t unc_len;
	صحيح ret, دوراني_ازموینه;
	uint magic;

	dst = map_sysmem(image->بارول_addr, image->اندازه);
	unc_len = (فراغ *)image->rcode_buf - (فراغ *)dst;
	image_len = image->اندازه;
	إذا (*image->ډېری_prot != ډېری_PROT_ارزښت)
		أرجع -EFAULT;
	magic = get_unaligned_le32(image->buf);
	إذا (CONFIG_IS_ENABLED(LZMA)) {
		اندازه_T lzma_len = unc_len;

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
		u32 *src, *پای_ټکی_2, *ptr;

		unc_len = image->اندازه;
		لكل (src = image->buf, پای_ټکی_2 = (فراغ *)src + image->اندازه,
		     ptr = dst; src < پای_ټکی_2;)
			*ptr++ = *src++;
	}
	إذا (*image->ډېری_prot != ډېری_PROT_ارزښت)
		أرجع -EFAULT;

	/* copy in the FDT if needed */
	إذا (image->fdt_اندازه)
		حافظه_نقل(image->fdt_پیل, image->fdt_buf, image->fdt_اندازه);

	دوراني_ازموینه = crc8(0, (u8 *)dst, unc_len);

	/* jump to the entry point */
	ثبت(دوراني_ازموینه, unc_len);
}

صحيح spl_reloc_jump(بنية spl_image_info *image, spl_jump_منزل_image_t jump)
{
	rcode_func loader;
	صحيح ret;

	log_debug("malloc usage %x bytes (%d KB of %d KB)\n", gd->malloc_ptr,
		  gd->malloc_ptr / 1024, CONFIG_VAL(SYS_MALLOC_F_LEN) / 1024);

	إذا (*image->ډېری_prot != ډېری_PROT_ارزښت) {
		log_err("stack busted, cannot continue\n");
		أرجع -EFAULT;
	}
	loader = (rcode_func)(فراغ *)rcode_reloc_and_jump + image->reloc_دننه_واټن;
	log_debug("Jumping via %p to %lx - image %p size %x load %lx\n", loader,
		  image->ثبت_ټکی, image, image->اندازه, image->بارول_addr);

	log_debug("unc_len %lx\n",
		  image->rcode_buf - map_sysmem(image->بارول_addr, image->اندازه));
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
