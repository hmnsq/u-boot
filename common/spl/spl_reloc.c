// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2024 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#అన్తర్భావ <gzip.h>
#అన్తర్భావ <image.h>
#అన్తర్భావ <log.h>
#అన్తర్భావ <mapmem.h>
#అన్తర్భావ <spl.h>
#అన్తర్భావ <asm/global_data.h>
#అన్తర్భావ <asm/io.h>
#అన్తర్భావ <asm/sections.h>
#అన్తర్భావ <asm/unaligned.h>
#అన్తర్భావ <linux/types.h>
#అన్తర్భావ <lzma/LzmaTypes.h>
#అన్తర్భావ <lzma/LzmaDec.h>
#అన్తర్భావ <lzma/LzmaTools.h>
#అన్తర్భావ <u-boot/crc.h>
#అన్తర్భావ <u-boot/lz4.h>

DECLARE_GLOBAL_దత్తాంశం_PTR;

/* provide a way to jump straight into the relocation code, for debugging */
#నిర్వచించు DEBUG_JUMP	0

గణన {
	/* margin to allow for stack growth */
	RELOC_దొంతర_MARGIN	= 0x800,

	/* align base address for DMA controllers which require it */
	పునాది_ALIGN		= 0x200,

	దొంతర_PROT_విలువ	= 0x51ce4697,
};

రకనిర్వచనం పూర్ణాంకం (*rcode_func)(నిర్మాణం spl_image_info *image);

స్థిర పూర్ణాంకం అమరిక_layout(నిర్మాణం spl_image_info *image, ulong *addrp)
{
	ulong పునాది, fdt_కొలత;
	ulong హద్దు, rcode_పునాది;
	uint rcode_కొలత;
	పూర్ణాంకం buf_కొలత, margin;
	అక్షరం *rcode_buf;

	హద్దు = ALIGN(map_గమ్యం_sysmem(&హద్దు) - RELOC_దొంతర_MARGIN, 8);
	image->దొంతర_prot = map_sysmem(హద్దు, పరిమాణం(uint));
	*image->దొంతర_prot = దొంతర_PROT_విలువ;

	fdt_కొలత = fdt_totalsize(gd->fdt_blob);
	పునాది = ALIGN(map_గమ్యం_sysmem(gd->fdt_blob) + fdt_కొలత + పునాది_ALIGN - 1,
		     పునాది_ALIGN);

	rcode_కొలత = rcode_చివర - rcode_ప్రారంభం;
	rcode_పునాది = హద్దు - rcode_కొలత;
	buf_కొలత = rcode_పునాది - పునాది;
	uint need_కొలత = image->కొలత + image->fdt_కొలత;
	margin = buf_కొలత - need_కొలత;
	log_debug("spl_reloc %s->%s: margin%s%lx limit %lx fdt_size %lx base %lx avail %x image %x fdt %lx need %x\n",
		  spl_phase_name(spl_phase()), spl_phase_name(spl_phase() + 1),
		  margin >= 0 ? " " : " -", నిరపేక్ష(margin), హద్దు, fdt_కొలత, పునాది,
		  buf_కొలత, image->కొలత, image->fdt_కొలత, need_కొలత);
	ఒకవేళ (margin < 0) {
		log_err("Image size %x but buffer is only %x\n", need_కొలత,
			buf_కొలత);
		తిరిగి -ENOSPC;
	}

	rcode_buf = map_sysmem(rcode_పునాది, rcode_కొలత);
	log_debug("_rcode_start %p: %x -- func %p %x\n", rcode_ప్రారంభం,
		  *(uint *)rcode_ప్రారంభం, అమరిక_layout, *(uint *)అమరిక_layout);

	image->reloc_స్థానభ్రంశం = rcode_buf - rcode_ప్రారంభం;
	log_debug("_rcode start %lx base %lx size %x offset %lx\n",
		  (ulong)map_గమ్యం_sysmem(rcode_ప్రారంభం), rcode_పునాది, rcode_కొలత,
		  image->reloc_స్థానభ్రంశం);

	జ్ఞాపకం_నకలు(rcode_buf, rcode_ప్రారంభం, rcode_కొలత);

	image->buf = map_sysmem(పునాది, need_కొలత);
	image->fdt_buf = image->buf + image->కొలత;
	image->rcode_buf = rcode_buf;
	*addrp = పునాది;

	తిరిగి 0;
}

పూర్ణాంకం spl_reloc_prepare(నిర్మాణం spl_image_info *image, ulong *addrp)
{
	పూర్ణాంకం ret;

	ret = అమరిక_layout(image, addrp);
	ఒకవేళ (ret)
		తిరిగి ret;

	తిరిగి 0;
}

రకనిర్వచనం శూన్య __noreturn (*image_నమోదు_noargs_t)(uint చక్రీయ_తనిఖీ, uint unc_len);

/* this is the relocation + jump code that is copied to the top of memory */
__rcode పూర్ణాంకం rcode_reloc_and_jump(నిర్మాణం spl_image_info *image)
{
	image_నమోదు_noargs_t నమోదు_2 = (image_నమోదు_noargs_t)image->నమోదు_బిందువు;
	u32 *dst;
	ulong image_len;
	size_t unc_len;
	పూర్ణాంకం ret, చక్రీయ_తనిఖీ;
	uint magic;

	dst = map_sysmem(image->ఎక్కించు_addr, image->కొలత);
	unc_len = (శూన్య *)image->rcode_buf - (శూన్య *)dst;
	image_len = image->కొలత;
	ఒకవేళ (*image->దొంతర_prot != దొంతర_PROT_విలువ)
		తిరిగి -EFAULT;
	magic = get_unaligned_le32(image->buf);
	ఒకవేళ (CONFIG_IS_ENABLED(LZMA)) {
		కొలత_T lzma_len = unc_len;

		ret = lzma_Buff_గమ్యం_Buff_Decompress((u8 *)dst, &lzma_len,
					       image->buf, image_len);
		unc_len = lzma_len;
	} లేకపోతే ఒకవేళ (CONFIG_IS_ENABLED(GZIP)) {
		ret = gunzip(dst, unc_len, image->buf, &image_len);
	} లేకపోతే ఒకవేళ (CONFIG_IS_ENABLED(LZ4) && magic == LZ4F_MAGIC) {
		ret = ulz4fn(image->buf, image_len, dst, &unc_len);
		ఒకవేళ (ret)
			తిరిగి ret;
	} లేకపోతే {
		u32 *src, *చివర_2, *ptr;

		unc_len = image->కొలత;
		కొరకు (src = image->buf, చివర_2 = (శూన్య *)src + image->కొలత,
		     ptr = dst; src < చివర_2;)
			*ptr++ = *src++;
	}
	ఒకవేళ (*image->దొంతర_prot != దొంతర_PROT_విలువ)
		తిరిగి -EFAULT;

	/* copy in the FDT if needed */
	ఒకవేళ (image->fdt_కొలత)
		జ్ఞాపకం_నకలు(image->fdt_ప్రారంభం, image->fdt_buf, image->fdt_కొలత);

	చక్రీయ_తనిఖీ = crc8(0, (u8 *)dst, unc_len);

	/* jump to the entry point */
	నమోదు_2(చక్రీయ_తనిఖీ, unc_len);
}

పూర్ణాంకం spl_reloc_jump(నిర్మాణం spl_image_info *image, spl_jump_గమ్యం_image_t jump)
{
	rcode_func loader;
	పూర్ణాంకం ret;

	log_debug("malloc usage %x bytes (%d KB of %d KB)\n", gd->malloc_ptr,
		  gd->malloc_ptr / 1024, CONFIG_VAL(SYS_MALLOC_F_LEN) / 1024);

	ఒకవేళ (*image->దొంతర_prot != దొంతర_PROT_విలువ) {
		log_err("stack busted, cannot continue\n");
		తిరిగి -EFAULT;
	}
	loader = (rcode_func)(శూన్య *)rcode_reloc_and_jump + image->reloc_స్థానభ్రంశం;
	log_debug("Jumping via %p to %lx - image %p size %x load %lx\n", loader,
		  image->నమోదు_బిందువు, image, image->కొలత, image->ఎక్కించు_addr);

	log_debug("unc_len %lx\n",
		  image->rcode_buf - map_sysmem(image->ఎక్కించు_addr, image->కొలత));
	ఒకవేళ (DEBUG_JUMP) {
		rcode_reloc_and_jump(image);
	} లేకపోతే {
		/*
		 * Must disable LOG_DEBUG since the decompressor cannot call
		 * log functions, printf(), etc.
		 */
		_Static_assert(DEBUG_JUMP || !_DEBUG,
			       "Cannot have debug output from decompressor");
		ret = loader(image);
	}

	తిరిగి -EFAULT;
}
