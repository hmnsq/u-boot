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

DECLARE_GLOBAL_DATA_PTR;

/* provide a way to jump straight into the relocation code, for debugging */
#define DEBUG_JUMP	0

गणना {
	/* margin to allow for stack growth */
	RELOC_चळत_MARGIN	= 0x800,

	/* align base address for DMA controllers which require it */
	BASE_ALIGN		= 0x200,

	चळत_PROT_मूल्य	= 0x51ce4697,
};

प्रकारव्याख्या पूर्णांक (*rcode_func)(संरचना spl_image_info *image);

स्थिर पूर्णांक setup_layout(संरचना spl_image_info *image, ulong *addrp)
{
	ulong base, fdt_size;
	ulong limit, rcode_base;
	uint rcode_size;
	पूर्णांक buf_size, margin;
	अक्षर *rcode_buf;

	limit = ALIGN(map_to_sysmem(&limit) - RELOC_चळत_MARGIN, 8);
	image->चळत_prot = map_sysmem(limit, आकार(uint));
	*image->चळत_prot = चळत_PROT_मूल्य;

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
		  margin >= 0 ? " " : " -", असल(margin), limit, fdt_size, base,
		  buf_size, image->size, image->fdt_size, need_size);
	जर (margin < 0) {
		log_err("Image size %x but buffer is only %x\n", need_size,
			buf_size);
		परत -ENOSPC;
	}

	rcode_buf = map_sysmem(rcode_base, rcode_size);
	log_debug("_rcode_start %p: %x -- func %p %x\n", _rcode_start,
		  *(uint *)_rcode_start, setup_layout, *(uint *)setup_layout);

	image->reloc_offset = rcode_buf - _rcode_start;
	log_debug("_rcode start %lx base %lx size %x offset %lx\n",
		  (ulong)map_to_sysmem(_rcode_start), rcode_base, rcode_size,
		  image->reloc_offset);

	स_नकल(rcode_buf, _rcode_start, rcode_size);

	image->buf = map_sysmem(base, need_size);
	image->fdt_buf = image->buf + image->size;
	image->rcode_buf = rcode_buf;
	*addrp = base;

	परत 0;
}

पूर्णांक spl_reloc_prepare(संरचना spl_image_info *image, ulong *addrp)
{
	पूर्णांक ret;

	ret = setup_layout(image, addrp);
	जर (ret)
		परत ret;

	परत 0;
}

प्रकारव्याख्या शून्य __noreturn (*image_entry_noargs_t)(uint crc, uint unc_len);

/* this is the relocation + jump code that is copied to the top of memory */
__rcode पूर्णांक rcode_reloc_and_jump(संरचना spl_image_info *image)
{
	image_entry_noargs_t entry = (image_entry_noargs_t)image->entry_point;
	u32 *dst;
	ulong image_len;
	माप_प्रकार unc_len;
	पूर्णांक ret, crc;
	uint magic;

	dst = map_sysmem(image->load_addr, image->size);
	unc_len = (शून्य *)image->rcode_buf - (शून्य *)dst;
	image_len = image->size;
	जर (*image->चळत_prot != चळत_PROT_मूल्य)
		परत -EFAULT;
	magic = get_unaligned_le32(image->buf);
	जर (CONFIG_IS_ENABLED(LZMA)) {
		SizeT lzma_len = unc_len;

		ret = lzmaBuffToBuffDecompress((u8 *)dst, &lzma_len,
					       image->buf, image_len);
		unc_len = lzma_len;
	} अन्यथा जर (CONFIG_IS_ENABLED(GZIP)) {
		ret = gunzip(dst, unc_len, image->buf, &image_len);
	} अन्यथा जर (CONFIG_IS_ENABLED(LZ4) && magic == LZ4F_MAGIC) {
		ret = ulz4fn(image->buf, image_len, dst, &unc_len);
		जर (ret)
			परत ret;
	} अन्यथा {
		u32 *src, *end, *ptr;

		unc_len = image->size;
		साठी (src = image->buf, end = (शून्य *)src + image->size,
		     ptr = dst; src < end;)
			*ptr++ = *src++;
	}
	जर (*image->चळत_prot != चळत_PROT_मूल्य)
		परत -EFAULT;

	/* copy in the FDT if needed */
	जर (image->fdt_size)
		स_नकल(image->fdt_start, image->fdt_buf, image->fdt_size);

	crc = crc8(0, (u8 *)dst, unc_len);

	/* jump to the entry point */
	entry(crc, unc_len);
}

पूर्णांक spl_reloc_jump(संरचना spl_image_info *image, spl_jump_to_image_t jump)
{
	rcode_func loader;
	पूर्णांक ret;

	log_debug("malloc usage %x bytes (%d KB of %d KB)\n", gd->malloc_ptr,
		  gd->malloc_ptr / 1024, CONFIG_VAL(SYS_MALLOC_F_LEN) / 1024);

	जर (*image->चळत_prot != चळत_PROT_मूल्य) {
		log_err("stack busted, cannot continue\n");
		परत -EFAULT;
	}
	loader = (rcode_func)(शून्य *)rcode_reloc_and_jump + image->reloc_offset;
	log_debug("Jumping via %p to %lx - image %p size %x load %lx\n", loader,
		  image->entry_point, image, image->size, image->load_addr);

	log_debug("unc_len %lx\n",
		  image->rcode_buf - map_sysmem(image->load_addr, image->size));
	जर (DEBUG_JUMP) {
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

	परत -EFAULT;
}
