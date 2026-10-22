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

DECLARE_GLOBAL_داده_PTR;

/* provide a way to jump straight into the relocation code, for debugging */
#تعریف_کن DEBUG_JUMP	0

تعداد {
	/* margin to allow for stack growth */
	RELOC_پشته_MARGIN	= 0x800,

	/* align base address for DMA controllers which require it */
	پایه_ALIGN		= 0x200,

	پشته_PROT_مقدار	= 0x51ce4697,
};

تعريفنوع صحيح (*rcode_func)(بنية spl_image_info *image);

ساكن صحيح راهاندازی_layout(بنية spl_image_info *image, ulong *addrp)
{
	ulong پایه, fdt_اندازه;
	ulong حد, rcode_پایه;
	uint rcode_اندازه;
	صحيح buf_اندازه, margin;
	حرف *rcode_buf;

	حد = ALIGN(map_مقصد_sysmem(&حد) - RELOC_پشته_MARGIN, 8);
	image->پشته_prot = map_sysmem(حد, حجم(uint));
	*image->پشته_prot = پشته_PROT_مقدار;

	fdt_اندازه = fdt_totalsize(gd->fdt_blob);
	پایه = ALIGN(map_مقصد_sysmem(gd->fdt_blob) + fdt_اندازه + پایه_ALIGN - 1,
		     پایه_ALIGN);

	rcode_اندازه = rcode_انتها - rcode_شروع;
	rcode_پایه = حد - rcode_اندازه;
	buf_اندازه = rcode_پایه - پایه;
	uint need_اندازه = image->اندازه + image->fdt_اندازه;
	margin = buf_اندازه - need_اندازه;
	log_debug("spl_reloc %s->%s: margin%s%lx limit %lx fdt_size %lx base %lx avail %x image %x fdt %lx need %x\n",
		  spl_phase_name(spl_phase()), spl_phase_name(spl_phase() + 1),
		  margin >= 0 ? " " : " -", قدر_مطلق(margin), حد, fdt_اندازه, پایه,
		  buf_اندازه, image->اندازه, image->fdt_اندازه, need_اندازه);
	إذا (margin < 0) {
		log_err("Image size %x but buffer is only %x\n", need_اندازه,
			buf_اندازه);
		أرجع -ENOSPC;
	}

	rcode_buf = map_sysmem(rcode_پایه, rcode_اندازه);
	log_debug("_rcode_start %p: %x -- func %p %x\n", rcode_شروع,
		  *(uint *)rcode_شروع, راهاندازی_layout, *(uint *)راهاندازی_layout);

	image->reloc_جابه_جایی = rcode_buf - rcode_شروع;
	log_debug("_rcode start %lx base %lx size %x offset %lx\n",
		  (ulong)map_مقصد_sysmem(rcode_شروع), rcode_پایه, rcode_اندازه,
		  image->reloc_جابه_جایی);

	کپی_حافظه(rcode_buf, rcode_شروع, rcode_اندازه);

	image->buf = map_sysmem(پایه, need_اندازه);
	image->fdt_buf = image->buf + image->اندازه;
	image->rcode_buf = rcode_buf;
	*addrp = پایه;

	أرجع 0;
}

صحيح spl_reloc_prepare(بنية spl_image_info *image, ulong *addrp)
{
	صحيح ret;

	ret = راهاندازی_layout(image, addrp);
	إذا (ret)
		أرجع ret;

	أرجع 0;
}

تعريفنوع فراغ __noreturn (*image_مدخل_noargs_t)(uint بررسی_چرخهای, uint unc_len);

/* this is the relocation + jump code that is copied to the top of memory */
__rcode صحيح rcode_reloc_and_jump(بنية spl_image_info *image)
{
	image_مدخل_noargs_t مدخل = (image_مدخل_noargs_t)image->مدخل_نقطه;
	u32 *dst;
	ulong image_len;
	size_t unc_len;
	صحيح ret, بررسی_چرخهای;
	uint magic;

	dst = map_sysmem(image->بارگیری_addr, image->اندازه);
	unc_len = (فراغ *)image->rcode_buf - (فراغ *)dst;
	image_len = image->اندازه;
	إذا (*image->پشته_prot != پشته_PROT_مقدار)
		أرجع -EFAULT;
	magic = get_unaligned_le32(image->buf);
	إذا (CONFIG_IS_ENABLED(LZMA)) {
		اندازه_T lzma_len = unc_len;

		ret = lzma_Buff_مقصد_Buff_Decompress((u8 *)dst, &lzma_len,
					       image->buf, image_len);
		unc_len = lzma_len;
	} وإلا إذا (CONFIG_IS_ENABLED(GZIP)) {
		ret = gunzip(dst, unc_len, image->buf, &image_len);
	} وإلا إذا (CONFIG_IS_ENABLED(LZ4) && magic == LZ4F_MAGIC) {
		ret = ulz4fn(image->buf, image_len, dst, &unc_len);
		إذا (ret)
			أرجع ret;
	} وإلا {
		u32 *src, *انتها_2, *ptr;

		unc_len = image->اندازه;
		لكل (src = image->buf, انتها_2 = (فراغ *)src + image->اندازه,
		     ptr = dst; src < انتها_2;)
			*ptr++ = *src++;
	}
	إذا (*image->پشته_prot != پشته_PROT_مقدار)
		أرجع -EFAULT;

	/* copy in the FDT if needed */
	إذا (image->fdt_اندازه)
		کپی_حافظه(image->fdt_شروع, image->fdt_buf, image->fdt_اندازه);

	بررسی_چرخهای = crc8(0, (u8 *)dst, unc_len);

	/* jump to the entry point */
	مدخل(بررسی_چرخهای, unc_len);
}

صحيح spl_reloc_jump(بنية spl_image_info *image, spl_jump_مقصد_image_t jump)
{
	rcode_func loader;
	صحيح ret;

	log_debug("malloc usage %x bytes (%d KB of %d KB)\n", gd->malloc_ptr,
		  gd->malloc_ptr / 1024, CONFIG_VAL(SYS_MALLOC_F_LEN) / 1024);

	إذا (*image->پشته_prot != پشته_PROT_مقدار) {
		log_err("stack busted, cannot continue\n");
		أرجع -EFAULT;
	}
	loader = (rcode_func)(فراغ *)rcode_reloc_and_jump + image->reloc_جابه_جایی;
	log_debug("Jumping via %p to %lx - image %p size %x load %lx\n", loader,
		  image->مدخل_نقطه, image, image->اندازه, image->بارگیری_addr);

	log_debug("unc_len %lx\n",
		  image->rcode_buf - map_sysmem(image->بارگیری_addr, image->اندازه));
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
