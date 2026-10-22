// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2024 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#समावेश <gzip.h>
#समावेश <image.h>
#समावेश <log.h>
#समावेश <mapmem.h>
#समावेश <spl.h>
#समावेश <asm/global_data.h>
#समावेश <asm/io.h>
#समावेश <asm/sections.h>
#समावेश <asm/unaligned.h>
#समावेश <linux/types.h>
#समावेश <lzma/LzmaTypes.h>
#समावेश <lzma/LzmaDec.h>
#समावेश <lzma/LzmaTools.h>
#समावेश <u-boot/crc.h>
#समावेश <u-boot/lz4.h>

DECLARE_GLOBAL_तथ्य_PTR;

/* provide a way to jump straight into the relocation code, for debugging */
#घोषणा DEBUG_JUMP	0

क्रमागत {
	/* margin to allow for stack growth */
	RELOC_ढेर_MARGIN	= 0x800,

	/* align base address for DMA controllers which require it */
	BASE_ALIGN		= 0x200,

	ढेर_PROT_मूल्य	= 0x51ce4697,
};

प्रकार पूर्णांक (*rcode_func)(काष्ठा spl_image_info *image);

अटल पूर्णांक setup_layout(काष्ठा spl_image_info *image, ulong *addrp)
{
	ulong base, fdt_आकार;
	ulong limit, rcode_base;
	uint rcode_आकार;
	पूर्णांक buf_आकार, margin;
	अक्षर *rcode_buf;

	limit = ALIGN(map_to_sysmem(&limit) - RELOC_ढेर_MARGIN, 8);
	image->ढेर_prot = map_sysmem(limit, माप(uint));
	*image->ढेर_prot = ढेर_PROT_मूल्य;

	fdt_आकार = fdt_totalsize(gd->fdt_blob);
	base = ALIGN(map_to_sysmem(gd->fdt_blob) + fdt_आकार + BASE_ALIGN - 1,
		     BASE_ALIGN);

	rcode_आकार = _rcode_end - _rcode_start;
	rcode_base = limit - rcode_आकार;
	buf_आकार = rcode_base - base;
	uint need_आकार = image->आकार + image->fdt_आकार;
	margin = buf_आकार - need_आकार;
	log_debug("spl_reloc %s->%s: margin%s%lx limit %lx fdt_size %lx base %lx avail %x image %x fdt %lx need %x\n",
		  spl_phase_name(spl_phase()), spl_phase_name(spl_phase() + 1),
		  margin >= 0 ? " " : " -", असल(margin), limit, fdt_आकार, base,
		  buf_आकार, image->आकार, image->fdt_आकार, need_आकार);
	अगर (margin < 0) {
		log_err("Image size %x but buffer is only %x\n", need_आकार,
			buf_आकार);
		वापस -ENOSPC;
	}

	rcode_buf = map_sysmem(rcode_base, rcode_आकार);
	log_debug("_rcode_start %p: %x -- func %p %x\n", _rcode_start,
		  *(uint *)_rcode_start, setup_layout, *(uint *)setup_layout);

	image->reloc_offset = rcode_buf - _rcode_start;
	log_debug("_rcode start %lx base %lx size %x offset %lx\n",
		  (ulong)map_to_sysmem(_rcode_start), rcode_base, rcode_आकार,
		  image->reloc_offset);

	स_नकल(rcode_buf, _rcode_start, rcode_आकार);

	image->buf = map_sysmem(base, need_आकार);
	image->fdt_buf = image->buf + image->आकार;
	image->rcode_buf = rcode_buf;
	*addrp = base;

	वापस 0;
}

पूर्णांक spl_reloc_prepare(काष्ठा spl_image_info *image, ulong *addrp)
{
	पूर्णांक ret;

	ret = setup_layout(image, addrp);
	अगर (ret)
		वापस ret;

	वापस 0;
}

प्रकार व्योम __noreturn (*image_entry_noargs_t)(uint crc, uint unc_len);

/* this is the relocation + jump code that is copied to the top of memory */
__rcode पूर्णांक rcode_reloc_and_jump(काष्ठा spl_image_info *image)
{
	image_entry_noargs_t entry = (image_entry_noargs_t)image->entry_point;
	u32 *dst;
	ulong image_len;
	माप_प्रकार unc_len;
	पूर्णांक ret, crc;
	uint magic;

	dst = map_sysmem(image->load_addr, image->आकार);
	unc_len = (व्योम *)image->rcode_buf - (व्योम *)dst;
	image_len = image->आकार;
	अगर (*image->ढेर_prot != ढेर_PROT_मूल्य)
		वापस -EFAULT;
	magic = get_unaligned_le32(image->buf);
	अगर (CONFIG_IS_ENABLED(LZMA)) {
		आकार_T lzma_len = unc_len;

		ret = lzmaBuffToBuffDecompress((u8 *)dst, &lzma_len,
					       image->buf, image_len);
		unc_len = lzma_len;
	} अन्यथा अगर (CONFIG_IS_ENABLED(GZIP)) {
		ret = gunzip(dst, unc_len, image->buf, &image_len);
	} अन्यथा अगर (CONFIG_IS_ENABLED(LZ4) && magic == LZ4F_MAGIC) {
		ret = ulz4fn(image->buf, image_len, dst, &unc_len);
		अगर (ret)
			वापस ret;
	} अन्यथा {
		u32 *src, *end, *ptr;

		unc_len = image->आकार;
		क्रम (src = image->buf, end = (व्योम *)src + image->आकार,
		     ptr = dst; src < end;)
			*ptr++ = *src++;
	}
	अगर (*image->ढेर_prot != ढेर_PROT_मूल्य)
		वापस -EFAULT;

	/* copy in the FDT if needed */
	अगर (image->fdt_आकार)
		स_नकल(image->fdt_start, image->fdt_buf, image->fdt_आकार);

	crc = crc8(0, (u8 *)dst, unc_len);

	/* jump to the entry point */
	entry(crc, unc_len);
}

पूर्णांक spl_reloc_jump(काष्ठा spl_image_info *image, spl_jump_to_image_t jump)
{
	rcode_func loader;
	पूर्णांक ret;

	log_debug("malloc usage %x bytes (%d KB of %d KB)\n", gd->malloc_ptr,
		  gd->malloc_ptr / 1024, CONFIG_VAL(SYS_MALLOC_F_LEN) / 1024);

	अगर (*image->ढेर_prot != ढेर_PROT_मूल्य) {
		log_err("stack busted, cannot continue\n");
		वापस -EFAULT;
	}
	loader = (rcode_func)(व्योम *)rcode_reloc_and_jump + image->reloc_offset;
	log_debug("Jumping via %p to %lx - image %p size %x load %lx\n", loader,
		  image->entry_point, image, image->आकार, image->load_addr);

	log_debug("unc_len %lx\n",
		  image->rcode_buf - map_sysmem(image->load_addr, image->आकार));
	अगर (DEBUG_JUMP) {
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

	वापस -EFAULT;
}
