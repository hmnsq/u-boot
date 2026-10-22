// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <fpga.h>
#ଅନ୍ତର୍ଭାଵ <gzip.h>
#ଅନ୍ତର୍ଭାଵ <image.h>
#ଅନ୍ତର୍ଭାଵ <log.h>
#ଅନ୍ତର୍ଭାଵ <memalign.h>
#ଅନ୍ତର୍ଭାଵ <mapmem.h>
#ଅନ୍ତର୍ଭାଵ <spl.h>
#ଅନ୍ତର୍ଭାଵ <upl.h>
#ଅନ୍ତର୍ଭାଵ <sysinfo.h>
#ଅନ୍ତର୍ଭାଵ <asm/global_data.h>
#ଅନ୍ତର୍ଭାଵ <asm/io.h>
#ଅନ୍ତର୍ଭାଵ <linux/libfdt.h>
#ଅନ୍ତର୍ଭାଵ <linux/printk.h>

DECLARE_GLOBAL_DATA_PTR;

ସଂରଚନା spl_fit_info {
	ଅଚଳ ଶୂନ୍ୟ *fit;	/* Pointer to a valid FIT blob */
	size_t ext_data_offset;	/* Offset to FIT external data (end of FIT) */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା images_ଗଣ୍ଠି;	/* FDT offset to "/images" node */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା conf_ଗଣ୍ଠି;		/* FDT offset to selected configuration node */
};

__weak ulong board_spl_fit_size_align(ulong size)
{
	ଫେରନ୍ତୁ size;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା find_ଗଣ୍ଠି_from_desc(ଅଚଳ ଶୂନ୍ୟ *fit, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣ୍ଠି, ଅଚଳ ଅକ୍ଷର *str)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା child;

	ଯଦି (ଗଣ୍ଠି < 0)
		ଫେରନ୍ତୁ -EINVAL;

	/* iterate the FIT nodes and find a matching description */
	ପାଇଁ (child = fdt_first_subnode(fit, ଗଣ୍ଠି); child >= 0;
	     child = fdt_next_subnode(fit, child)) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା len;
		ଅଚଳ ଅକ୍ଷର *desc = fdt_getprop(fit, child, FIT_DESC_PROP, &len);

		ଯଦି (!desc)
			ଜାରି;

		ଯଦି (!strcmp(desc, str))
			ଫେରନ୍ତୁ child;
	}

	ଫେରନ୍ତୁ -ENOENT;
}

/**
 * spl_fit_get_image_name(): By using the matching configuration subnode,
 * retrieve the name of an image, specified by a property name and an index
 * into that.
 * @fit:	Pointer to the FDT blob.
 * @images:	Offset of the /images subnode.
 * @type:	Name of the property within the configuration subnode.
 * @index:	Index into the list of strings in this property.
 * @outname:	Name of the image
 *
 * Return:	0 on success, or a negative error number
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା spl_fit_get_image_name(ଅଚଳ ସଂରଚନା spl_fit_info *ctx,
				  ଅଚଳ ଅକ୍ଷର *type, ପୂର୍ଣ୍ଣସଂଖ୍ୟା index,
				  ଅଚଳ ଅକ୍ଷର **outname)
{
	ସଂରଚନା udevice *sysinfo;
	ଅଚଳ ଅକ୍ଷର *name, *str, *end;
	__maybe_unused ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣ୍ଠି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା len, i;
	bool found = true;

	name = fdt_getprop(ctx->fit, ctx->conf_ଗଣ୍ଠି, type, &len);
	ଯଦି (!name) {
		debug("cannot find property '%s': %d\n", type, len);
		ଫେରନ୍ତୁ -EINVAL;
	}
	/* A string property should be NUL terminated */
	end = name + len - 1;
	ଯଦି (!len || *end) {
		debug("malformed property '%s'\n", type);
		ଫେରନ୍ତୁ -EINVAL;
	}

	str = name;
	ପାଇଁ (i = 0; i < index; i++) {
		str = strchr(str, '\0') + 1;
		ଯଦି (str > end) {
			found = false;
			ଭାଙ୍ଗ;
		}
	}

	ଯଦି (!found && CONFIG_IS_ENABLED(SYSINFO) && !sysinfo_get(&sysinfo)) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
		/*
		 * no string in the property for this index. Check if the
		 * sysinfo-level code can supply one.
		 */
		rc = sysinfo_detect(sysinfo);
		ଯଦି (rc)
			ଫେରନ୍ତୁ rc;

		rc = sysinfo_get_fit_loadable(sysinfo, index - i - 1, type,
					      &str);
		ଯଦି (rc && rc != -ENOENT)
			ଫେରନ୍ତୁ rc;

		ଯଦି (!rc) {
			/*
			 * The sysinfo provided a name for a loadable.
			 * Try to match it against the description properties
			 * first. If no matching node is found, use it as a
			 * node name.
			 */
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣ୍ଠି;
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା images = fdt_path_offset(ctx->fit, FIT_IMAGES_PATH);

			ଗଣ୍ଠି = find_ଗଣ୍ଠି_from_desc(ctx->fit, images, str);
			ଯଦି (ଗଣ୍ଠି > 0)
				str = fdt_get_name(ctx->fit, ଗଣ୍ଠି, ଖାଲି);

			found = true;
		}
	}

	ଯଦି (!found) {
		debug("no string for index %d\n", index);
		ଫେରନ୍ତୁ -E2BIG;
	}

	*outname = str;
	ଫେରନ୍ତୁ 0;
}

/**
 * spl_fit_get_image_node(): By using the matching configuration subnode,
 * retrieve the name of an image, specified by a property name and an index
 * into that.
 * @fit:	Pointer to the FDT blob.
 * @images:	Offset of the /images subnode.
 * @type:	Name of the property within the configuration subnode.
 * @index:	Index into the list of strings in this property.
 *
 * Return:	the node offset of the respective image node or a negative
 *		error number.
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା spl_fit_get_image_ଗଣ୍ଠି(ଅଚଳ ସଂରଚନା spl_fit_info *ctx,
				  ଅଚଳ ଅକ୍ଷର *type, ପୂର୍ଣ୍ଣସଂଖ୍ୟା index)
{
	ଅଚଳ ଅକ୍ଷର *str;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା err;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣ୍ଠି;

	err = spl_fit_get_image_name(ctx, type, index, &str);
	ଯଦି (err)
		ଫେରନ୍ତୁ err;

	debug("%s: '%s'\n", type, str);

	ଗଣ୍ଠି = fdt_subnode_offset(ctx->fit, ctx->images_ଗଣ୍ଠି, str);
	ଯଦି (ଗଣ୍ଠି < 0) {
		pr_err("cannot find image node '%s': %d\n", str, ଗଣ୍ଠି);
		ଫେରନ୍ତୁ -EINVAL;
	}

	ଫେରନ୍ତୁ ଗଣ୍ଠି;
}

ସ୍ଥିର u32 get_aligned_image_offset(ସଂରଚନା spl_load_info *info, u32 offset)
{
	ଫେରନ୍ତୁ ALIGN_DOWN(offset, spl_get_bl_len(info));
}

ସ୍ଥିର u32 get_aligned_image_overhead(ସଂରଚନା spl_load_info *info, u32 offset)
{
	ଫେରନ୍ତୁ offset & (spl_get_bl_len(info) - 1);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା get_aligned_image_size(ସଂରଚନା spl_load_info *info, ulong data_size,
				  u32 offset, ulong *aligned_size)
{
	u32 overhead = get_aligned_image_overhead(info, offset);

	ଯଦି (data_size > ULONG_MAX - overhead)
		ଫେରନ୍ତୁ -EOVERFLOW;
	data_size += overhead;

	ଯଦି (data_size > ULONG_MAX - (spl_get_bl_len(info) - 1))
		ଫେରନ୍ତୁ -EOVERFLOW;
	data_size = ALIGN(data_size, spl_get_bl_len(info));

	*aligned_size = data_size;

	ଫେରନ୍ତୁ 0;
}

/**
 * fit_fill_image_info(): describe a loaded image to the caller
 * @fit:	points to the FIT image
 * @node:	offset of the DT node describing the image
 * @image_info:	filled in with where the image ended up and how big it is;
 *		ignored if NULL
 * @load_addr:	address the image was loaded to
 * @size:	number of bytes loaded, which may be zero
 */
ସ୍ଥିର ଶୂନ୍ୟ fit_fill_image_info(ଅଚଳ ଶୂନ୍ୟ *fit, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣ୍ଠି,
				ସଂରଚନା spl_image_info *image_info,
				ulong load_addr, ulong size)
{
	ulong entry_point;

	ଯଦି (!image_info)
		ଫେରନ୍ତୁ;

	image_info->load_addr = load_addr;
	image_info->size = size;

	ଯଦି (!fit_image_get_entry(fit, ଗଣ୍ଠି, &entry_point))
		image_info->entry_point = entry_point;
	ଅନ୍ୟଥା
		image_info->entry_point = FDT_ERROR;
}

/**
 * load_simple_fit(): load the image described in a certain FIT node
 * @info:	points to information about the device to load data from
 * @fit_offset:	the offset of the FIT image on the device
 * @ctx:	points to the FIT context structure
 * @node:	offset of the DT node describing the image to load (relative
 *		to @fit)
 * @image_info:	will be filled with information about the loaded image
 *		If the FIT node does not contain a "load" (address) property,
 *		the image gets loaded to the address pointed to by the
 *		load_addr member in this struct, if load_addr is not 0
 * @max_size:	maximum number of bytes that may be written to the
 *		destination; an image whose data exceeds this is rejected
 *		before it is read from the device
 *
 * Return:	0 on success, -EBADSLT if this image is not the correct phase
 * (for CONFIG_BOOTMETH_VBE_SIMPLE_FW), or another negative error number on
 * other error.
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା load_simple_fit(ସଂରଚନା spl_load_info *info, ulong fit_offset,
			   ଅଚଳ ସଂରଚନା spl_fit_info *ctx, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣ୍ଠି,
			   ସଂରଚନା spl_image_info *image_info, ulong max_size)
{
	u32 offset;
	u32 len;
	size_t length;
	ulong size;
	ulong load_addr;
	ଶୂନ୍ୟ *load_ptr;
	ଶୂନ୍ୟ *src;
	uint8_t image_comp = -1, type = -1;
	ଅଚଳ ଶୂନ୍ୟ *data;
	ଅଚଳ ଶୂନ୍ୟ *fit = ctx->fit;
	bool external_data = false;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	log_debug("starting\n");
	ଯଦି (CONFIG_IS_ENABLED(BOOTMETH_VBE) &&
	    xpl_get_phase(info) != IH_PHASE_NONE) {
		ଗଣନା image_phase_t phase;

		ret = fit_image_get_phase(fit, ଗଣ୍ଠି, &phase);
		/* if the image is for any phase, let's use it */
		ଯଦି (ret == -ENOENT || phase == xpl_get_phase(info)) {
			log_debug("found\n");
		} ଅନ୍ୟଥା ଯଦି (ret < 0) {
			log_debug("err=%d\n", ret);
			ଫେରନ୍ତୁ ret;
		} ଅନ୍ୟଥା {
			log_debug("- phase mismatch, skipping this image\n");
			ଫେରନ୍ତୁ -EBADSLT;
		}
	}

	ଯଦି (IS_ENABLED(CONFIG_SPL_FPGA) ||
	    (IS_ENABLED(CONFIG_SPL_OS_BOOT) && spl_decompression_enabled())) {
		ଯଦି (fit_image_get_type(fit, ଗଣ୍ଠି, &type))
			puts("Cannot get image type.\n");
		ଅନ୍ୟଥା
			debug("%s ", genimg_get_type_name(type));
	}

	ଯଦି (spl_decompression_enabled()) {
		fit_image_get_comp(fit, ଗଣ୍ଠି, &image_comp);
		debug("%s ", genimg_get_comp_name(image_comp));
	}

	ଯଦି (fit_image_get_load(fit, ଗଣ୍ଠି, &load_addr)) {
		ଯଦି (!image_info->load_addr) {
			printf("Can't load %s: No load address and no buffer\n",
			       fit_get_name(fit, ଗଣ୍ଠି, ଖାଲି));
			ଫେରନ୍ତୁ -ENOBUFS;
		}
		load_addr = image_info->load_addr;
	}

	ଯଦି (!fit_image_get_data_position(fit, ଗଣ୍ଠି, &offset)) {
		external_data = true;
	} ଅନ୍ୟଥା ଯଦି (!fit_image_get_data_offset(fit, ଗଣ୍ଠି, &offset)) {
		/* The resulting offset cannot exceed UINT32_MAX */
		ଯଦି (ctx->ext_data_offset > UINT32_MAX - offset) {
			log_debug("Invalid external data offset: %u\n", offset);
			ଫେରନ୍ତୁ -EINVAL;
		}
		log_debug("read offset %x = offset from fit %x\n", offset,
			  (u32)(offset + ctx->ext_data_offset));
		offset += ctx->ext_data_offset;
		external_data = true;
	}

	ଯଦି (external_data) {
		u32 aligned_offset;
		ulong read_offset;
		ଶୂନ୍ୟ *src_ptr;

		/* External data */
		ଯଦି (fit_image_get_data_size(fit, ଗଣ୍ଠି, &len))
			ଫେରନ୍ତୁ -ENOENT;

		/* Dont bother to copy 0 byte data, but warn, though */
		ଯଦି (!len) {
			log_warning("%s: Skip load '%s': image size is 0!\n",
				    __func__, fit_get_name(fit, ଗଣ୍ଠି, ଖାଲି));
			fit_fill_image_info(fit, ଗଣ୍ଠି, image_info, load_addr, 0);
			ଫେରନ୍ତୁ 0;
		}

		/*
		 * data-size is excluded from the configuration signature (it
		 * is in exc_prop[] in image-fit-sig.c), so it stays attacker
		 * controlled even after fit_config_verify() succeeds. The
		 * image hash is only verified after the device read below, so
		 * an oversized value has to be rejected here.
		 */
		ret = get_aligned_image_size(info, len, offset, &size);
		ଯଦି (ret) {
			log_debug("Invalid external data size: %u\n", len);
			ଫେରନ୍ତୁ ret;
		}

		ଯଦି (size > max_size) {
			log_debug("Image too large: aligned size %lu, max %lu (data-size %u)\n",
				  size, max_size, len);
			ଫେରନ୍ତୁ -EFBIG;
		}

		aligned_offset = get_aligned_image_offset(info, offset);
		ଯଦି (aligned_offset > ULONG_MAX - fit_offset) {
			log_debug("Invalid aligned external data offset: %u\n",
				  aligned_offset);
			ଫେରନ୍ତୁ -EINVAL;
		}
		read_offset = fit_offset + aligned_offset;

		ଯଦି (spl_decompression_enabled() &&
		    (image_comp == IH_COMP_GZIP || image_comp == IH_COMP_LZMA))
			src_ptr = map_sysmem(ALIGN(CONFIG_SYS_LOAD_ADDR, ARCH_DMA_MINALIGN), len);
		ଅନ୍ୟଥା
			src_ptr = map_sysmem(ALIGN(load_addr, ARCH_DMA_MINALIGN), len);
		length = len;

		/*
		 * For non-signed FIT images, we can check that
		 * (read_offset + size) does not wrap and that
		 * (src_ptr + size) does not exceed the addressable range.
		 * For signed FITs, we can additionally check that
		 * (offset + len) doesn't exceed the allowed FIT image
		 * maximum size.
		 */
		ଯଦି (size > ULONG_MAX - read_offset ||
		    size > UINTPTR_MAX - (uintptr_t)src_ptr
		/*
		 * #if (not a runtime if) is required: FIT_SIGNATURE_MAX_SIZE
		 * depends on FIT_SIGNATURE, so CONFIG_VAL(FIT_SIGNATURE_MAX_SIZE)
		 * is undefined when signing is disabled and referencing it
		 * here would fail to compile.
		 */
#ଯଦି CONFIG_IS_ENABLED(FIT_SIGNATURE)
		    || offset > CONFIG_VAL(FIT_SIGNATURE_MAX_SIZE) ||
		    len > CONFIG_VAL(FIT_SIGNATURE_MAX_SIZE) - offset
#endif
		) {
			log_debug("FIT external data is out of bounds (offset=%u, size=%u)\n",
				  offset, len);
			ଫେରନ୍ତୁ -EINVAL;
		}

		log_debug("reading from offset %x / %lx size %lx to %p: ",
			  offset, read_offset, size, src_ptr);

		ଯଦି (info->ପଠ(info, read_offset, size, src_ptr) < length)
			ଫେରନ୍ତୁ -EIO;

		debug("External data: dst=%p, offset=%x, size=%lx\n",
		      src_ptr, offset, (unsigned long)length);
		src = src_ptr + get_aligned_image_overhead(info, offset);
	} ଅନ୍ୟଥା {
		/* Embedded data */
		ଯଦି (fit_image_get_emb_data(fit, ଗଣ୍ଠି, &data, &length)) {
			puts("Cannot get image data/size\n");
			ଫେରନ୍ତୁ -ENOENT;
		}
		debug("Embedded data: dst=%lx, size=%lx\n", load_addr,
		      (unsigned long)length);
		src = (ଶୂନ୍ୟ *)data;	/* cast away const */
	}

	ଯଦି (CONFIG_IS_ENABLED(FIT_SIGNATURE)) {
		printf("## Checking hash(es) for Image %s ... ",
		       fit_get_name(fit, ଗଣ୍ଠି, ଖାଲି));
		ଯଦି (!fit_image_verify_with_data(fit, ଗଣ୍ଠି, gd_fdt_blob(), src,
						length))
			ଫେରନ୍ତୁ -EPERM;
		puts("OK\n");
	}

	ଯଦି (CONFIG_IS_ENABLED(FIT_IMAGE_POST_PROCESS))
		board_fit_image_post_process(fit, ଗଣ୍ଠି, &src, &length);

	load_ptr = map_sysmem(load_addr, length);
	ଯଦି (IS_ENABLED(CONFIG_SPL_GZIP) && image_comp == IH_COMP_GZIP) {
		size = length;
		ଯଦି (gunzip(load_ptr, CONFIG_SYS_BOOTM_LEN, src, &size)) {
			puts("Uncompressing error\n");
			ଫେରନ୍ତୁ -EIO;
		}
		length = size;
	} ଅନ୍ୟଥା ଯଦି (IS_ENABLED(CONFIG_SPL_LZMA) && image_comp == IH_COMP_LZMA) {
		size = CONFIG_SYS_BOOTM_LEN;
		ulong loadEnd;

		ଯଦି (image_decomp(IH_COMP_LZMA, CONFIG_SYS_LOAD_ADDR, 0, 0,
				 load_ptr, src, length, size, &loadEnd)) {
			puts("Uncompressing error\n");
			ଫେରନ୍ତୁ -EIO;
		}
		length = loadEnd - CONFIG_SYS_LOAD_ADDR;
	} ଅନ୍ୟଥା {
		memmove(load_ptr, src, length);
	}

	fit_fill_image_info(fit, ଗଣ୍ଠି, image_info, load_addr, length);
	log_debug("- done loading\n");

	upl_add_image(fit, ଗଣ୍ଠି, load_addr, length);

	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର bool os_takes_devicetree(uint8_t os)
{
	ବାଛ (os) {
	ସ୍ଥିତି IH_OS_U_BOOT:
		ଫେରନ୍ତୁ true;
	ସ୍ଥିତି IH_OS_LINUX:
		ଫେରନ୍ତୁ IS_ENABLED(CONFIG_SPL_OS_BOOT) ||
		       IS_ENABLED(CONFIG_SPL_OPENSBI);
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଫେରନ୍ତୁ false;
	}
}

__weak ପୂର୍ଣ୍ଣସଂଖ୍ୟା board_spl_fit_append_fdt_skip(ଅଚଳ ଅକ୍ଷର *name)
{
	ଫେରନ୍ତୁ 0;	/* Do not skip */
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା spl_fit_append_fdt(ସଂରଚନା spl_image_info *spl_image,
			      ସଂରଚନା spl_load_info *info, ulong offset,
			      ଅଚଳ ସଂରଚନା spl_fit_info *ctx)
{
	ସଂରଚନା spl_image_info image_info;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣ୍ଠି, ret = 0, index = 0;

	/*
	 * Use the address following the image as target address for the
	 * device tree.
	 */
	image_info.load_addr = ALIGN(spl_image->load_addr + spl_image->size, 8);

	/* Figure out which device tree the board wants to use */
	ଗଣ୍ଠି = spl_fit_get_image_ଗଣ୍ଠି(ctx, FIT_FDT_PROP, index++);
	ଯଦି (ଗଣ୍ଠି < 0) {
		size_t size;

		debug("%s: cannot find FDT node\n", __func__);

		/*
		 * U-Boot did not find a device tree inside the FIT image. Use
		 * the U-Boot device tree instead.
		 */
		ଯଦି (!gd->fdt_blob)
			ଫେରନ୍ତୁ ଗଣ୍ଠି;

		/*
		 * Make the load-address of the FDT available for the SPL
		 * framework
		 */
		size = fdt_totalsize(gd->fdt_blob);
		spl_image->fdt_addr = map_sysmem(image_info.load_addr, size);
		memcpy(spl_image->fdt_addr, gd->fdt_blob, size);
	} ଅନ୍ୟଥା {
		ret = load_simple_fit(info, offset, ctx, ଗଣ୍ଠି, &image_info,
				      CONFIG_SYS_BOOTM_LEN);
		ଯଦି (ret < 0)
			ଫେରନ୍ତୁ ret;

		spl_image->fdt_addr = phys_to_virt(image_info.load_addr);
	}

	ଯଦି (CONFIG_IS_ENABLED(FIT_IMAGE_TINY))
		ଫେରନ୍ତୁ 0;

#ଯଦି CONFIG_IS_ENABLED(LOAD_FIT_APPLY_OVERLAY)
		ଶୂନ୍ୟ *tmpbuffer = ଖାଲି;

		ପାଇଁ (; ; index++) {
			ଅଚଳ ଅକ୍ଷର *str;

			ret = spl_fit_get_image_name(ctx, FIT_FDT_PROP, index, &str);
			ଯଦି (ret == -E2BIG) {
				debug("%s: No additional FDT node\n", __func__);
				ret = 0;
				ଭାଙ୍ଗ;
			} ଅନ୍ୟଥା ଯଦି (ret) {
				ଭାଙ୍ଗ;
			}

			ret = board_spl_fit_append_fdt_skip(str);
			ଯଦି (ret)
				ଜାରି;

			ଗଣ୍ଠି = fdt_subnode_offset(ctx->fit, ctx->images_ଗଣ୍ଠି, str);
			ଯଦି (ଗଣ୍ଠି < 0) {
				debug("%s: unable to find FDT node %d\n",
				      __func__, index);
				ଜାରି;
			}

			ଯଦି (!tmpbuffer) {
				/*
				 * allocate memory to store the DT overlay
				 * before it is applied. It may not be used
				 * depending on how the overlay is stored, so
				 * don't fail yet if the allocation failed.
				 */
				size_t size = CONFIG_SPL_LOAD_FIT_APPLY_OVERLAY_BUF_SZ;

				tmpbuffer = malloc_cache_aligned(size);
				ଯଦି (!tmpbuffer)
					debug("%s: unable to allocate space for overlays\n",
					      __func__);
			}
			image_info.load_addr = (ulong)tmpbuffer;
			ret = load_simple_fit(info, offset, ctx, ଗଣ୍ଠି,
					      &image_info,
					      CONFIG_SPL_LOAD_FIT_APPLY_OVERLAY_BUF_SZ);
			ଯଦି (ret == -EBADSLT)
				ଜାରି;
			ଅନ୍ୟଥା ଯଦି (ret < 0)
				ଭାଙ୍ଗ;

			/* Make room in FDT for changes from the overlay */
			ret = fdt_increase_size(spl_image->fdt_addr,
						image_info.size);
			ଯଦି (ret < 0)
				ଭାଙ୍ଗ;

			ret = fdt_overlay_apply_verbose(spl_image->fdt_addr,
							(ଶୂନ୍ୟ *)image_info.load_addr);
			ଯଦି (ret) {
				pr_err("failed to apply DT overlay %s\n",
				       fit_get_name(ctx->fit, ଗଣ୍ଠି, ଖାଲି));
				ଭାଙ୍ଗ;
			}

			debug("%s: DT overlay %s applied\n", __func__,
			      fit_get_name(ctx->fit, ଗଣ୍ଠି, ଖାଲି));
		}
		ମୁକ୍ତ(tmpbuffer);
		ଯଦି (ret)
			ଫେରନ୍ତୁ ret;
#endif
	/* Try to make space, so we can inject details on the loadables */
	ret = fdt_shrink_to_minimum(spl_image->fdt_addr, 8192);
	ଯଦି (ret < 0)
		ଫେରନ୍ତୁ ret;

	ଫେରନ୍ତୁ ret;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା spl_fit_record_loadable(ଅଚଳ ସଂରଚନା spl_fit_info *ctx, ପୂର୍ଣ୍ଣସଂଖ୍ୟା index,
				   ଶୂନ୍ୟ *blob, ସଂରଚନା spl_image_info *image)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = 0;
	ଅଚଳ ଅକ୍ଷର *name;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣ୍ଠି;

	ret = spl_fit_get_image_name(ctx, FIT_LOADABLE_PROP, index, &name);
	ଯଦି (ret < 0)
		ଫେରନ୍ତୁ ret;

	ଗଣ୍ଠି = spl_fit_get_image_ଗଣ୍ଠି(ctx, FIT_LOADABLE_PROP, index);

	ret = fdt_record_loadable(blob, index, name, image->load_addr,
			image->size, image->entry_point,
			fdt_getprop(ctx->fit, ଗଣ୍ଠି, FIT_TYPE_PROP, ଖାଲି),
			fdt_getprop(ctx->fit, ଗଣ୍ଠି, FIT_OS_PROP, ଖାଲି),
			fdt_getprop(ctx->fit, ଗଣ୍ଠି, FIT_ARCH_PROP, ଖାଲି));

	ଫେରନ୍ତୁ ret;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା spl_fit_image_is_fpga(ଅଚଳ ଶୂନ୍ୟ *fit, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣ୍ଠି)
{
	ଅଚଳ ଅକ୍ଷର *type;

	ଯଦି (!IS_ENABLED(CONFIG_SPL_FPGA))
		ଫେରନ୍ତୁ 0;

	type = fdt_getprop(fit, ଗଣ୍ଠି, FIT_TYPE_PROP, ଖାଲି);
	ଯଦି (!type)
		ଫେରନ୍ତୁ 0;

	ଫେରନ୍ତୁ !strcmp(type, "fpga");
}

ସ୍ଥିର ଶୂନ୍ୟ spl_fit_image_record_arm32_optee(ଅଚଳ ଶୂନ୍ୟ *fit, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣ୍ଠି,
					     ସଂରଚନା spl_image_info *spl_image,
					     ସଂରଚନା spl_image_info *image_info)
{
#ଯଦି defined(CONFIG_BOOTM_OPTEE) && defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
	ଅଚଳ ଅକ୍ଷର *type = fdt_getprop(fit, ଗଣ୍ଠି, FIT_TYPE_PROP, ଖାଲି);

	ଯଦି (!type)
		ଫେରନ୍ତୁ;

	ଯଦି (strcmp(type, "tee"))
		ଫେରନ୍ତୁ;

	spl_image->optee_addr = image_info->load_addr;
#endif
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା spl_fit_image_get_os(ଅଚଳ ଶୂନ୍ୟ *fit, ପୂର୍ଣ୍ଣସଂଖ୍ୟା noffset, uint8_t *os)
{
	ଯଦି (!CONFIG_IS_ENABLED(FIT_IMAGE_TINY) || CONFIG_IS_ENABLED(OS_BOOT))
		ଫେରନ୍ତୁ fit_image_get_os(fit, noffset, os);

	ଅଚଳ ଅକ୍ଷର *name = fdt_getprop(fit, noffset, FIT_OS_PROP, ଖାଲି);
	ଯଦି (!name)
		ଫେରନ୍ତୁ -ENOENT;

	/*
	 * We don't care what the type of the image actually is,
	 * only whether or not it is U-Boot. This saves some
	 * space by omitting the large table of OS types.
	 */
	ଯଦି (!strcmp(name, "u-boot"))
		*os = IH_OS_U_BOOT;
	ଅନ୍ୟଥା
		*os = IH_OS_INVALID;

	ଫେରନ୍ତୁ 0;
}

/*
 * The purpose of the FIT load buffer is to provide a memory location that is
 * independent of the load address of any FIT component.
 */
__weak ଶୂନ୍ୟ *board_spl_fit_buffer_addr(ulong fit_size, ପୂର୍ଣ୍ଣସଂଖ୍ୟା sectors, ପୂର୍ଣ୍ଣସଂଖ୍ୟା bl_len)
{
	ଶୂନ୍ୟ *buf;
	size_t size = sectors * bl_len;

	buf = malloc_cache_aligned(size);
	ଯଦି (!buf) {
		pr_err("Could not get FIT buffer of %lu bytes\n", (ulong)size);

		ଯଦି (IS_ENABLED(CONFIG_SPL_SYS_MALLOC))
			pr_err("\tcheck CONFIG_SPL_SYS_MALLOC_SIZE\n");
		ଅନ୍ୟଥା
			pr_err("\tcheck CONFIG_SPL_SYS_MALLOC_F_LEN\n");

		buf = spl_get_load_buffer(0, size);
	}
	ଫେରନ୍ତୁ buf;
}

/*
 * Weak default function to allow customizing SPL fit loading for load-only
 * use cases by allowing to skip the parsing/processing of the FIT contents
 * (so that this can be done separately in a more customized fashion)
 */
__weak bool spl_load_simple_fit_skip_processing(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ false;
}

/*
 * Weak default function to allow fixes after fit header
 * is loaded.
 */
__weak ଶୂନ୍ୟ *spl_load_simple_fit_fix_load(ଅଚଳ ଶୂନ୍ୟ *fit)
{
	ଫେରନ୍ତୁ (ଶୂନ୍ୟ *)fit;
}

ସ୍ଥିର ଶୂନ୍ୟ warn_deprecated(ଅଚଳ ଅକ୍ଷର *ସନ୍ଦେଶ)
{
	printf("DEPRECATED: %s\n", ସନ୍ଦେଶ);
	printf("\tSee https://fitspec.osfw.foundation/\n");
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା spl_fit_upload_fpga(ସଂରଚନା spl_fit_info *ctx, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣ୍ଠି,
			       ସଂରଚନା spl_image_info *fpga_image)
{
	ଅଚଳ ଅକ୍ଷର *compatible;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା devnum = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags = 0;

	debug("FPGA bitstream at: %x, size: %x\n",
	      (u32)fpga_image->load_addr, fpga_image->size);

	compatible = fdt_getprop(ctx->fit, ଗଣ୍ଠି, FIT_COMPAT_PROP, ଖାଲି);
	ଯଦି (!compatible) {
		warn_deprecated("'fpga' image without 'compatible' property");
	} ଅନ୍ୟଥା {
		ଯଦି (CONFIG_IS_ENABLED(FPGA_LOAD_SECURE))
			flags = fpga_compatible2flag(devnum, compatible);
		ଯଦି (strcmp(compatible, "u-boot,fpga-legacy"))
			debug("Ignoring compatible = %s property\n",
			      compatible);
	}

	ret = fpga_load(devnum, (ଶୂନ୍ୟ *)fpga_image->load_addr,
			fpga_image->size, BIT_FULL, flags);
	ଯଦି (ret) {
		printf("%s: Cannot load the image to the FPGA\n", __func__);
		ଫେରନ୍ତୁ ret;
	}

	puts("FPGA image loaded from FIT\n");

	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା spl_fit_load_fpga(ସଂରଚନା spl_fit_info *ctx,
			     ସଂରଚନା spl_load_info *info, ulong offset)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣ୍ଠି, ret;

	ସଂରଚନା spl_image_info fpga_image = {
		.load_addr = 0,
	};

	ଗଣ୍ଠି = spl_fit_get_image_ଗଣ୍ଠି(ctx, FIT_FPGA_PROP, 0);
	ଯଦି (ଗଣ୍ଠି < 0)
		ଫେରନ୍ତୁ ଗଣ୍ଠି;

	warn_deprecated("'fpga' property in config node. Use 'loadables'");

	/* Load the image and set up the fpga_image structure */
	ret = load_simple_fit(info, offset, ctx, ଗଣ୍ଠି, &fpga_image,
			      CONFIG_SYS_BOOTM_LEN);
	ଯଦି (ret) {
		printf("%s: Cannot load the FPGA: %i\n", __func__, ret);
		ଫେରନ୍ତୁ ret;
	}

	ଫେରନ୍ତୁ spl_fit_upload_fpga(ctx, ଗଣ୍ଠି, &fpga_image);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା spl_simple_fit_read(ସଂରଚନା spl_fit_info *ctx,
			       ସଂରଚନା spl_load_info *info, ulong offset,
			       ଅଚଳ ଶୂନ୍ୟ *fit_header)
{
	unsigned long aligned_size, ଗଣନା_2, size;
	ଶୂନ୍ୟ *buf;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	/*
	 * For FIT with external data, figure out where the external images
	 * start. This is the base for the data-offset properties in each
	 * image.
	 */
	size = ALIGN(fdt_totalsize(fit_header), 4);
	size = board_spl_fit_size_align(size);
	ctx->ext_data_offset = ALIGN(size, 4);

	/*
	 * So far we only have one block of data from the FIT. Read the entire
	 * thing, including that first block.
	 *
	 * For FIT with data embedded, data is loaded as part of FIT image.
	 * For FIT with external data, data is not loaded in this step.
	 */
	ret = get_aligned_image_size(info, size, 0, &aligned_size);
	ଯଦି (ret) {
		log_debug("Invalid FIT size: %lu\n", size);
		ଫେରନ୍ତୁ ret;
	}
	buf = board_spl_fit_buffer_addr(aligned_size, aligned_size, 1);
	ଯଦି (!buf) {
		/*
		 * We assume that none of the board will ever use 0x0 as a
		 * valid load address. Theoretically some board could use it,
		 * but this is extremely unlikely.
		 */
		ଫେରନ୍ତୁ -EIO;
	}

	ଗଣନା_2 = info->ପଠ(info, offset, aligned_size, buf);
	ଯଦି (!ଗଣନା_2) {
		/*
		 * FIT could not be read. This means we should free the
		 * memory allocated by board_spl_fit_buffer_addr().
		 * Unfortunately, we don't know what memory allocation
		 * mechanism was used:
		 *   - For the SPL_SYS_MALLOC_SIMPLE case nothing could
		 *     be done. The memory just could not be freed.
		 *   - For statically allocated memory buffer we can try
		 *     to reuse previously allocated memory (example:
		 *     board_spl_fit_buffer_addr() function from the
		 *     file test/image/spl_load.c).
		 *   - For normall malloc() -- memory leak can't be easily
		 *     avoided. To somehow reduce memory consumption the
		 *     next calls of board_spl_fit_buffer_addr() could
		 *     reallocate previously allocated buffer and use
		 *     them again. This is somethat similar to the approach
		 *     used for statically allocated buffer.
		 *
		 * Please note:
		 *   - FIT images with data placed outside of the FIT
		 *     structure will cause small memory leak (several
		 *     kilobytes),
		 *   - FIT images with data placed inside to the FIT
		 *     structure may cause huge memory leak (up to
		 *     several megabytes). Do NOT use such images!
		 */
		ଫେରନ୍ତୁ -EIO;
	}

	ctx->fit = buf;
	debug("fit read offset %lx, size=%lu, dst=%p, count=%lu\n",
	      offset, aligned_size, buf, ଗଣନା_2);

	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା spl_simple_fit_parse(ସଂରଚନା spl_fit_info *ctx)
{
	/* Find the correct subnode under "/configurations" */
	ctx->conf_ଗଣ୍ଠି = fit_find_config_ଗଣ୍ଠି(ctx->fit);
	ଯଦି (ctx->conf_ଗଣ୍ଠି < 0)
		ଫେରନ୍ତୁ -EINVAL;

	ଯଦି (CONFIG_IS_ENABLED(FIT_SIGNATURE)) {
		printf("## Checking hash(es) for config %s ... ",
		       fit_get_name(ctx->fit, ctx->conf_ଗଣ୍ଠି, ଖାଲି));
		ଯଦି (fit_config_verify(ctx->fit, ctx->conf_ଗଣ୍ଠି))
			ଫେରନ୍ତୁ -EPERM;
		puts("OK\n");
	}

	/* find the node holding the images information */
	ctx->images_ଗଣ୍ଠି = fdt_path_offset(ctx->fit, FIT_IMAGES_PATH);
	ଯଦି (ctx->images_ଗଣ୍ଠି < 0) {
		debug("%s: Cannot find /images node: %d\n", __func__,
		      ctx->images_ଗଣ୍ଠି);
		ଫେରନ୍ତୁ -EINVAL;
	}

	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା spl_load_simple_fit(ସଂରଚନା spl_image_info *spl_image,
			ସଂରଚନା spl_load_info *info, ulong offset, ଶୂନ୍ୟ *fit)
{
	ସଂରଚନା spl_image_info image_info;
	ସଂରଚନା spl_fit_info ctx;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣ୍ଠି = -1;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା index = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା firmware_ଗଣ୍ଠି;

	ret = spl_simple_fit_read(&ctx, info, offset, fit);
	ଯଦି (ret < 0)
		ଫେରନ୍ତୁ ret;

	/* skip further processing if requested to enable load-only use cases */
	ଯଦି (spl_load_simple_fit_skip_processing())
		ଫେରନ୍ତୁ 0;

	ctx.fit = spl_load_simple_fit_fix_load(ctx.fit);

	ret = spl_simple_fit_parse(&ctx);
	ଯଦି (ret < 0)
		ଫେରନ୍ତୁ ret;

	ଯଦି (IS_ENABLED(CONFIG_SPL_FPGA))
		spl_fit_load_fpga(&ctx, info, offset);

	/*
	 * Find the U-Boot image using the following search order:
	 *   - start at 'firmware' (e.g. an ARM Trusted Firmware)
	 *   - fall back 'kernel' (e.g. a Falcon-mode OS boot
	 *   - fall back to using the first 'loadables' entry
	 */
	ଯଦି (ଗଣ୍ଠି < 0)
		ଗଣ୍ଠି = spl_fit_get_image_ଗଣ୍ଠି(&ctx, FIT_FIRMWARE_PROP, 0);

	ଯଦି (ଗଣ୍ଠି < 0 && IS_ENABLED(CONFIG_SPL_OS_BOOT))
		ଗଣ୍ଠି = spl_fit_get_image_ଗଣ୍ଠି(&ctx, FIT_KERNEL_PROP, 0);

	ଯଦି (ଗଣ୍ଠି < 0) {
		debug("could not find firmware image, trying loadables...\n");
		ଗଣ୍ଠି = spl_fit_get_image_ଗଣ୍ଠି(&ctx, FIT_LOADABLE_PROP, 0);
		/*
		 * If we pick the U-Boot image from "loadables", start at
		 * the second image when later loading additional images.
		 */
		index = 1;
	}
	ଯଦି (ଗଣ୍ଠି < 0) {
		debug("%s: Cannot find u-boot image node: %d\n",
		      __func__, ଗଣ୍ଠି);
		ଫେରନ୍ତୁ -1;
	}

	/* Load the image and set up the spl_image structure */
	ret = load_simple_fit(info, offset, &ctx, ଗଣ୍ଠି, spl_image,
			      CONFIG_SYS_BOOTM_LEN);
	ଯଦି (ret)
		ଫେରନ୍ତୁ ret;

	/*
	 * For backward compatibility, we treat the first node that is
	 * as a U-Boot image, if no OS-type has been declared.
	 */
	ଯଦି (!spl_fit_image_get_os(ctx.fit, ଗଣ୍ଠି, &spl_image->os))
		debug("Image OS is %s\n", genimg_get_os_name(spl_image->os));
	ଅନ୍ୟଥା ଯଦି (!IS_ENABLED(CONFIG_SPL_OS_BOOT))
		spl_image->os = IH_OS_U_BOOT;

	/*
	 * Booting a next-stage U-Boot may require us to append the FDT.
	 * We allow this to fail, as the U-Boot image might embed its FDT.
	 */
	ଯଦି (os_takes_devicetree(spl_image->os)) {
		ret = spl_fit_append_fdt(spl_image, info, offset, &ctx);
		ଯଦି (ret < 0 && spl_image->os != IH_OS_U_BOOT)
			ଫେରନ୍ତୁ ret;
	}

	firmware_ଗଣ୍ଠି = ଗଣ୍ଠି;
	/* Now check if there are more images for us to load */
	ପାଇଁ (; ; index++) {
		uint8_t os_type = IH_OS_INVALID;

		ଗଣ୍ଠି = spl_fit_get_image_ଗଣ୍ଠି(&ctx, FIT_LOADABLE_PROP, index);
		ଯଦି (ଗଣ୍ଠି < 0)
			ଭାଙ୍ଗ;

		/*
		 * if the firmware is also a loadable, skip it because
		 * it already has been loaded. This is typically the case with
		 * u-boot.img generated by mkimage.
		 */
		ଯଦି (firmware_ଗଣ୍ଠି == ଗଣ୍ଠି)
			ଜାରି;

		image_info.load_addr = 0;
		ret = load_simple_fit(info, offset, &ctx, ଗଣ୍ଠି, &image_info,
				      CONFIG_SYS_BOOTM_LEN);
		ଯଦି (ret < 0 && ret != -EBADSLT) {
			printf("%s: can't load image loadables index %d (ret = %d)\n",
			       __func__, index, ret);
			ଫେରନ୍ତୁ ret;
		}

		ଯଦି (spl_fit_image_is_fpga(ctx.fit, ଗଣ୍ଠି))
			spl_fit_upload_fpga(&ctx, ଗଣ୍ଠି, &image_info);

		ଯଦି (!spl_fit_image_get_os(ctx.fit, ଗଣ୍ଠି, &os_type))
			debug("Loadable is %s\n", genimg_get_os_name(os_type));

		ଯଦି (os_takes_devicetree(os_type)) {
			spl_fit_append_fdt(&image_info, info, offset, &ctx);
			spl_image->fdt_addr = image_info.fdt_addr;
		}

		/*
		 * If the "firmware" image did not provide an entry point,
		 * use the first valid entry point from the loadables.
		 */
		ଯଦି (spl_image->entry_point == FDT_ERROR &&
		    image_info.entry_point != FDT_ERROR)
			spl_image->entry_point = image_info.entry_point;

		spl_fit_image_record_arm32_optee(ctx.fit, ଗଣ୍ଠି, spl_image,
						 &image_info);

		/* Record our loadables into the FDT */
		ଯଦି (!CONFIG_IS_ENABLED(FIT_IMAGE_TINY) &&
		    xpl_get_fdt_update(info) && spl_image->fdt_addr)
			spl_fit_record_loadable(&ctx, index,
						spl_image->fdt_addr,
						&image_info);
	}

	/*
	 * If a platform does not provide CONFIG_SYS_UBOOT_START, U-Boot's
	 * Makefile will set it to 0 and it will end up as the entry point
	 * here. What it actually means is: use the load address.
	 */
	ଯଦି (spl_image->entry_point == FDT_ERROR || spl_image->entry_point == 0)
		spl_image->entry_point = spl_image->load_addr;

	spl_image->flags |= SPL_FIT_FOUND;
	upl_set_fit_info(map_to_sysmem(ctx.fit), ctx.conf_ଗଣ୍ଠି,
			 spl_image->entry_point);

	ଫେରନ୍ତୁ 0;
}

/* Parse and load full fitImage in SPL */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା spl_load_fit_image(ସଂରଚନା spl_image_info *spl_image,
		       ଅଚଳ ସଂରଚନା legacy_img_hdr *header)
{
	ସଂରଚନା bootm_headers images;
	ଅଚଳ ଅକ୍ଷର *fit_uname_config = ଖାଲି;
	ulong fdt_hack;
	ଅଚଳ ଅକ୍ଷର *uname;
	ulong fw_data = 0, dt_data = 0, img_data = 0;
	ulong fw_len = 0, dt_len = 0, img_len = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx, conf_noffset;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	images.verify = CONFIG_IS_ENABLED(FIT_SIGNATURE);

	ret = fit_image_load(&images, virt_to_phys((ଶୂନ୍ୟ *)header),
			     ଖାଲି, &fit_uname_config, IH_ARCH_DEFAULT,
			     IH_TYPE_FIRMWARE, -1, FIT_LOAD_OPTIONAL,
			     &fw_data, &fw_len);

	ଯଦି (ret < 0) {
		ret = fit_image_load(&images, virt_to_phys((ଶୂନ୍ୟ *)header),
				     ଖାଲି, &fit_uname_config, IH_ARCH_DEFAULT,
				     IH_TYPE_KERNEL, -1, FIT_LOAD_OPTIONAL,
				     &fw_data, &fw_len);
	}

	ଯଦି (ret < 0)
		ଫେରନ୍ତୁ ret;

	spl_image->size = fw_len;
	spl_image->load_addr = fw_data;
	ଯଦି (fit_image_get_entry(header, ret, &spl_image->entry_point))
		spl_image->entry_point = fw_data;
	ଯଦି (fit_image_get_os(header, ret, &spl_image->os))
		spl_image->os = IH_OS_INVALID;
	spl_image->name = genimg_get_os_name(spl_image->os);

	debug(PHASE_PROMPT "payload image: %32s load addr: 0x%lx size: %d\n",
	      spl_image->name, spl_image->load_addr, spl_image->size);

	images.verify = CONFIG_IS_ENABLED(FIT_SIGNATURE);

	ret = fit_image_load(&images, virt_to_phys((ଶୂନ୍ୟ *)header), ଖାଲି,
			     &fit_uname_config, IH_ARCH_DEFAULT, IH_TYPE_FLATDT,
			     -1, FIT_LOAD_OPTIONAL, &dt_data, &dt_len);
	ଯଦି (ret >= 0) {
		ଯଦି (spl_image->os == IH_OS_U_BOOT) {
			/* HACK: U-Boot expects FDT at a specific address */
			fdt_hack = ALIGN(spl_image->load_addr + spl_image->size, 8);
			debug("Relocating FDT to %p\n", (ଶୂନ୍ୟ *)fdt_hack);
			memcpy(map_sysmem(fdt_hack, dt_len),
			       map_sysmem(dt_data, 0), dt_len);
			spl_image->fdt_addr = (ଶୂନ୍ୟ *)fdt_hack;
		} ଅନ୍ୟଥା {
			spl_image->fdt_addr = (ଶୂନ୍ୟ *)dt_data;
		}
	}

	conf_noffset = fit_conf_get_ଗଣ୍ଠି((ଅଚଳ ଶୂନ୍ୟ *)header,
					 fit_uname_config);
	ଯଦି (conf_noffset < 0)
		ଫେରନ୍ତୁ 0;

	ପାଇଁ (idx = 0;
	     uname = fdt_stringlist_get((ଅଚଳ ଶୂନ୍ୟ *)header, conf_noffset,
					FIT_LOADABLE_PROP, idx,
				ଖାଲି), uname;
	     idx++) {
		images.verify = CONFIG_IS_ENABLED(FIT_SIGNATURE);

		ret = fit_image_load(&images, virt_to_phys((ଶୂନ୍ୟ *)header),
				     &uname, &fit_uname_config,
				     IH_ARCH_DEFAULT, IH_TYPE_LOADABLE, -1,
				     FIT_LOAD_OPTIONAL_NON_ZERO,
				     &img_data, &img_len);
		ଯଦି (ret < 0)
			ଫେରନ୍ତୁ ret;
	}
	spl_image->flags |= SPL_FIT_FOUND;

	upl_set_fit_info(map_to_sysmem(header), conf_noffset,
			 spl_image->entry_point);

	ଫେରନ୍ତୁ 0;
}
