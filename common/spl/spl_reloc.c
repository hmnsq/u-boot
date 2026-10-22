// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2024 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#अन्तर्भाव <gzip.h>
#अन्तर्भाव <image.h>
#अन्तर्भाव <log.h>
#अन्तर्भाव <mapmem.h>
#अन्तर्भाव <spl.h>
#अन्तर्भाव <asm/global_data.h>
#अन्तर्भाव <asm/io.h>
#अन्तर्भाव <asm/sections.h>
#अन्तर्भाव <asm/unaligned.h>
#अन्तर्भाव <linux/types.h>
#अन्तर्भाव <lzma/LzmaTypes.h>
#अन्तर्भाव <lzma/LzmaDec.h>
#अन्तर्भाव <lzma/LzmaTools.h>
#अन्तर्भाव <u-boot/crc.h>
#अन्तर्भाव <u-boot/lz4.h>

DECLARE_GLOBAL_दत्त_PTR;

/* provide a way to jump straight into the relocation code, for debugging */
#परिभाषय DEBUG_JUMP	0

गणना {
	/* margin to allow for stack growth */
	RELOC_राशिः_MARGIN	= 0x800,

	/* align base address for DMA controllers which require it */
	आधारः_ALIGN		= 0x200,

	राशिः_PROT_मानम्	= 0x51ce4697,
};

प्रकारनिर्देश पूर्णाङ्क (*rcode_func)(संरचना spl_image_info *image);

स्थिर पूर्णाङ्क सज्जता_layout(संरचना spl_image_info *image, ulong *addrp)
{
	ulong आधारः, fdt_आकार;
	ulong सीमा, rcode_आधारः;
	uint rcode_आकार;
	पूर्णाङ्क buf_आकार, margin;
	वर्ण *rcode_buf;

	सीमा = ALIGN(map_गन्तव्यम्_sysmem(&सीमा) - RELOC_राशिः_MARGIN, 8);
	image->राशिः_prot = map_sysmem(सीमा, परिमाण(uint));
	*image->राशिः_prot = राशिः_PROT_मानम्;

	fdt_आकार = fdt_totalsize(gd->fdt_blob);
	आधारः = ALIGN(map_गन्तव्यम्_sysmem(gd->fdt_blob) + fdt_आकार + आधारः_ALIGN - 1,
		     आधारः_ALIGN);

	rcode_आकार = rcode_अन्तः - _rcode_start;
	rcode_आधारः = सीमा - rcode_आकार;
	buf_आकार = rcode_आधारः - आधारः;
	uint need_आकार = image->आकार + image->fdt_आकार;
	margin = buf_आकार - need_आकार;
	log_debug("spl_reloc %s->%s: margin%s%lx limit %lx fdt_size %lx base %lx avail %x image %x fdt %lx need %x\n",
		  spl_phase_name(spl_phase()), spl_phase_name(spl_phase() + 1),
		  margin >= 0 ? " " : " -", निरपेक्षम्(margin), सीमा, fdt_आकार, आधारः,
		  buf_आकार, image->आकार, image->fdt_आकार, need_आकार);
	यदि (margin < 0) {
		log_err("Image size %x but buffer is only %x\n", need_आकार,
			buf_आकार);
		प्रत्यागम -ENOSPC;
	}

	rcode_buf = map_sysmem(rcode_आधारः, rcode_आकार);
	log_debug("_rcode_start %p: %x -- func %p %x\n", _rcode_start,
		  *(uint *)_rcode_start, सज्जता_layout, *(uint *)सज्जता_layout);

	image->reloc_विस्थापनम् = rcode_buf - _rcode_start;
	log_debug("_rcode start %lx base %lx size %x offset %lx\n",
		  (ulong)map_गन्तव्यम्_sysmem(_rcode_start), rcode_आधारः, rcode_आकार,
		  image->reloc_विस्थापनम्);

	स्मृतिं_प्रतिलिख(rcode_buf, _rcode_start, rcode_आकार);

	image->buf = map_sysmem(आधारः, need_आकार);
	image->fdt_buf = image->buf + image->आकार;
	image->rcode_buf = rcode_buf;
	*addrp = आधारः;

	प्रत्यागम 0;
}

पूर्णाङ्क spl_reloc_prepare(संरचना spl_image_info *image, ulong *addrp)
{
	पूर्णाङ्क ret;

	ret = सज्जता_layout(image, addrp);
	यदि (ret)
		प्रत्यागम ret;

	प्रत्यागम 0;
}

प्रकारनिर्देश शून्य __noreturn (*image_प्रविष्टिः_noargs_t)(uint crc, uint unc_len);

/* this is the relocation + jump code that is copied to the top of memory */
__rcode पूर्णाङ्क rcode_reloc_and_jump(संरचना spl_image_info *image)
{
	image_प्रविष्टिः_noargs_t प्रविष्टिः = (image_प्रविष्टिः_noargs_t)image->प्रविष्टिः_point;
	u32 *dst;
	ulong image_len;
	size_t unc_len;
	पूर्णाङ्क ret, crc;
	uint magic;

	dst = map_sysmem(image->आरोपय_addr, image->आकार);
	unc_len = (शून्य *)image->rcode_buf - (शून्य *)dst;
	image_len = image->आकार;
	यदि (*image->राशिः_prot != राशिः_PROT_मानम्)
		प्रत्यागम -EFAULT;
	magic = get_unaligned_le32(image->buf);
	यदि (CONFIG_IS_ENABLED(LZMA)) {
		आकार_T lzma_len = unc_len;

		ret = lzma_Buff_गन्तव्यम्_Buff_Decompress((u8 *)dst, &lzma_len,
					       image->buf, image_len);
		unc_len = lzma_len;
	} अन्यथा यदि (CONFIG_IS_ENABLED(GZIP)) {
		ret = gunzip(dst, unc_len, image->buf, &image_len);
	} अन्यथा यदि (CONFIG_IS_ENABLED(LZ4) && magic == LZ4F_MAGIC) {
		ret = ulz4fn(image->buf, image_len, dst, &unc_len);
		यदि (ret)
			प्रत्यागम ret;
	} अन्यथा {
		u32 *src, *अन्तः_2, *ptr;

		unc_len = image->आकार;
		प्रति (src = image->buf, अन्तः_2 = (शून्य *)src + image->आकार,
		     ptr = dst; src < अन्तः_2;)
			*ptr++ = *src++;
	}
	यदि (*image->राशिः_prot != राशिः_PROT_मानम्)
		प्रत्यागम -EFAULT;

	/* copy in the FDT if needed */
	यदि (image->fdt_आकार)
		स्मृतिं_प्रतिलिख(image->fdt_start, image->fdt_buf, image->fdt_आकार);

	crc = crc8(0, (u8 *)dst, unc_len);

	/* jump to the entry point */
	प्रविष्टिः(crc, unc_len);
}

पूर्णाङ्क spl_reloc_jump(संरचना spl_image_info *image, spl_jump_गन्तव्यम्_image_t jump)
{
	rcode_func loader;
	पूर्णाङ्क ret;

	log_debug("malloc usage %x bytes (%d KB of %d KB)\n", gd->malloc_ptr,
		  gd->malloc_ptr / 1024, CONFIG_VAL(SYS_MALLOC_F_LEN) / 1024);

	यदि (*image->राशिः_prot != राशिः_PROT_मानम्) {
		log_err("stack busted, cannot continue\n");
		प्रत्यागम -EFAULT;
	}
	loader = (rcode_func)(शून्य *)rcode_reloc_and_jump + image->reloc_विस्थापनम्;
	log_debug("Jumping via %p to %lx - image %p size %x load %lx\n", loader,
		  image->प्रविष्टिः_point, image, image->आकार, image->आरोपय_addr);

	log_debug("unc_len %lx\n",
		  image->rcode_buf - map_sysmem(image->आरोपय_addr, image->आकार));
	यदि (DEBUG_JUMP) {
		rcode_reloc_and_jump(image);
	} अन्यथा {
		/*
		 * Must disable LOG_DEBUG since the decompressor cannot call
		 * log functions, printf(), etc.
		 */
		_Static_assert(DEBUG_JUMP || !_DEBUG,
			       "Cannot have debug output from decompressor");
		ret = loader(image);
	}

	प्रत्यागम -EFAULT;
}
