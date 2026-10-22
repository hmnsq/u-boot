// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2024 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#അന്തര്ഭാവ <gzip.h>
#അന്തര്ഭാവ <image.h>
#അന്തര്ഭാവ <log.h>
#അന്തര്ഭാവ <mapmem.h>
#അന്തര്ഭാവ <spl.h>
#അന്തര്ഭാവ <asm/global_data.h>
#അന്തര്ഭാവ <asm/io.h>
#അന്തര്ഭാവ <asm/sections.h>
#അന്തര്ഭാവ <asm/unaligned.h>
#അന്തര്ഭാവ <linux/types.h>
#അന്തര്ഭാവ <lzma/LzmaTypes.h>
#അന്തര്ഭാവ <lzma/LzmaDec.h>
#അന്തര്ഭാവ <lzma/LzmaTools.h>
#അന്തര്ഭാവ <u-boot/crc.h>
#അന്തര്ഭാവ <u-boot/lz4.h>

DECLARE_GLOBAL_DATA_PTR;

/* provide a way to jump straight into the relocation code, for debugging */
#define DEBUG_JUMP	0

എണ്ണൽ {
	/* margin to allow for stack growth */
	RELOC_അട്ടി_MARGIN	= 0x800,

	/* align base address for DMA controllers which require it */
	BASE_ALIGN		= 0x200,

	അട്ടി_PROT_മൂല്യം	= 0x51ce4697,
};

തരനിർവചനം പൂർണ്ണസംഖ്യ (*rcode_func)(ഘടന spl_image_info *image);

സ്ഥിതം പൂർണ്ണസംഖ്യ setup_layout(ഘടന spl_image_info *image, ulong *addrp)
{
	ulong base, fdt_size;
	ulong limit, rcode_base;
	uint rcode_size;
	പൂർണ്ണസംഖ്യ buf_size, margin;
	അക്ഷരം *rcode_buf;

	limit = ALIGN(map_to_sysmem(&limit) - RELOC_അട്ടി_MARGIN, 8);
	image->അട്ടി_prot = map_sysmem(limit, വലുപ്പം(uint));
	*image->അട്ടി_prot = അട്ടി_PROT_മൂല്യം;

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
		  margin >= 0 ? " " : " -", abs(margin), limit, fdt_size, base,
		  buf_size, image->size, image->fdt_size, need_size);
	എങ്കിൽ (margin < 0) {
		log_err("Image size %x but buffer is only %x\n", need_size,
			buf_size);
		തിരികെ -ENOSPC;
	}

	rcode_buf = map_sysmem(rcode_base, rcode_size);
	log_debug("_rcode_start %p: %x -- func %p %x\n", _rcode_start,
		  *(uint *)_rcode_start, setup_layout, *(uint *)setup_layout);

	image->reloc_offset = rcode_buf - _rcode_start;
	log_debug("_rcode start %lx base %lx size %x offset %lx\n",
		  (ulong)map_to_sysmem(_rcode_start), rcode_base, rcode_size,
		  image->reloc_offset);

	memcpy(rcode_buf, _rcode_start, rcode_size);

	image->buf = map_sysmem(base, need_size);
	image->fdt_buf = image->buf + image->size;
	image->rcode_buf = rcode_buf;
	*addrp = base;

	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ spl_reloc_prepare(ഘടന spl_image_info *image, ulong *addrp)
{
	പൂർണ്ണസംഖ്യ ret;

	ret = setup_layout(image, addrp);
	എങ്കിൽ (ret)
		തിരികെ ret;

	തിരികെ 0;
}

തരനിർവചനം ശൂന്യം __noreturn (*image_entry_noargs_t)(uint crc, uint unc_len);

/* this is the relocation + jump code that is copied to the top of memory */
__rcode പൂർണ്ണസംഖ്യ rcode_reloc_and_jump(ഘടന spl_image_info *image)
{
	image_entry_noargs_t entry = (image_entry_noargs_t)image->entry_point;
	u32 *dst;
	ulong image_len;
	size_t unc_len;
	പൂർണ്ണസംഖ്യ ret, crc;
	uint magic;

	dst = map_sysmem(image->load_addr, image->size);
	unc_len = (ശൂന്യം *)image->rcode_buf - (ശൂന്യം *)dst;
	image_len = image->size;
	എങ്കിൽ (*image->അട്ടി_prot != അട്ടി_PROT_മൂല്യം)
		തിരികെ -EFAULT;
	magic = get_unaligned_le32(image->buf);
	എങ്കിൽ (CONFIG_IS_ENABLED(LZMA)) {
		SizeT lzma_len = unc_len;

		ret = lzmaBuffToBuffDecompress((u8 *)dst, &lzma_len,
					       image->buf, image_len);
		unc_len = lzma_len;
	} അല്ലെങ്കിൽ എങ്കിൽ (CONFIG_IS_ENABLED(GZIP)) {
		ret = gunzip(dst, unc_len, image->buf, &image_len);
	} അല്ലെങ്കിൽ എങ്കിൽ (CONFIG_IS_ENABLED(LZ4) && magic == LZ4F_MAGIC) {
		ret = ulz4fn(image->buf, image_len, dst, &unc_len);
		എങ്കിൽ (ret)
			തിരികെ ret;
	} അല്ലെങ്കിൽ {
		u32 *src, *end, *ptr;

		unc_len = image->size;
		വേണ്ടി (src = image->buf, end = (ശൂന്യം *)src + image->size,
		     ptr = dst; src < end;)
			*ptr++ = *src++;
	}
	എങ്കിൽ (*image->അട്ടി_prot != അട്ടി_PROT_മൂല്യം)
		തിരികെ -EFAULT;

	/* copy in the FDT if needed */
	എങ്കിൽ (image->fdt_size)
		memcpy(image->fdt_start, image->fdt_buf, image->fdt_size);

	crc = crc8(0, (u8 *)dst, unc_len);

	/* jump to the entry point */
	entry(crc, unc_len);
}

പൂർണ്ണസംഖ്യ spl_reloc_jump(ഘടന spl_image_info *image, spl_jump_to_image_t jump)
{
	rcode_func loader;
	പൂർണ്ണസംഖ്യ ret;

	log_debug("malloc usage %x bytes (%d KB of %d KB)\n", gd->malloc_ptr,
		  gd->malloc_ptr / 1024, CONFIG_VAL(SYS_MALLOC_F_LEN) / 1024);

	എങ്കിൽ (*image->അട്ടി_prot != അട്ടി_PROT_മൂല്യം) {
		log_err("stack busted, cannot continue\n");
		തിരികെ -EFAULT;
	}
	loader = (rcode_func)(ശൂന്യം *)rcode_reloc_and_jump + image->reloc_offset;
	log_debug("Jumping via %p to %lx - image %p size %x load %lx\n", loader,
		  image->entry_point, image, image->size, image->load_addr);

	log_debug("unc_len %lx\n",
		  image->rcode_buf - map_sysmem(image->load_addr, image->size));
	എങ്കിൽ (DEBUG_JUMP) {
		rcode_reloc_and_jump(image);
	} അല്ലെങ്കിൽ {
		/*
		 * Must disable LOG_DEBUG since the decompressor cannot call
		 * log functions, printf(), etc.
		 */
		_Static_assert(DEBUG_JUMP || !_DEBUG,
			       "Cannot have debug output from decompressor");
		ret = loader(image);
	}

	തിരികെ -EFAULT;
}
