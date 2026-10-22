// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <fpga.h>
#அந்தர்பாவ <gzip.h>
#அந்தர்பாவ <image.h>
#அந்தர்பாவ <log.h>
#அந்தர்பாவ <memalign.h>
#அந்தர்பாவ <mapmem.h>
#அந்தர்பாவ <spl.h>
#அந்தர்பாவ <upl.h>
#அந்தர்பாவ <sysinfo.h>
#அந்தர்பாவ <asm/global_data.h>
#அந்தர்பாவ <asm/io.h>
#அந்தர்பாவ <linux/libfdt.h>
#அந்தர்பாவ <linux/printk.h>

DECLARE_GLOBAL_DATA_PTR;

கட்டமைப்பு spl_fit_info {
	மாறிலி வெறுமை *fit;	/* Pointer to a valid FIT blob */
	size_t ext_data_offset;	/* Offset to FIT external data (end of FIT) */
	முழு images_முனை;	/* FDT offset to "/images" node */
	முழு conf_முனை;		/* FDT offset to selected configuration node */
};

__weak ulong board_spl_fit_size_align(ulong size)
{
	திரும்பு size;
}

நிலையான முழு find_முனை_from_desc(மாறிலி வெறுமை *fit, முழு முனை, மாறிலி எழுத்து *str)
{
	முழு child;

	எனில் (முனை < 0)
		திரும்பு -EINVAL;

	/* iterate the FIT nodes and find a matching description */
	க்கு (child = fdt_first_subnode(fit, முனை); child >= 0;
	     child = fdt_next_subnode(fit, child)) {
		முழு len;
		மாறிலி எழுத்து *desc = fdt_getprop(fit, child, FIT_DESC_PROP, &len);

		எனில் (!desc)
			தொடர்;

		எனில் (!strcmp(desc, str))
			திரும்பு child;
	}

	திரும்பு -ENOENT;
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
நிலையான முழு spl_fit_get_image_name(மாறிலி கட்டமைப்பு spl_fit_info *ctx,
				  மாறிலி எழுத்து *type, முழு index,
				  மாறிலி எழுத்து **outname)
{
	கட்டமைப்பு udevice *sysinfo;
	மாறிலி எழுத்து *name, *str, *end;
	__maybe_unused முழு முனை;
	முழு len, i;
	bool found = true;

	name = fdt_getprop(ctx->fit, ctx->conf_முனை, type, &len);
	எனில் (!name) {
		debug("cannot find property '%s': %d\n", type, len);
		திரும்பு -EINVAL;
	}
	/* A string property should be NUL terminated */
	end = name + len - 1;
	எனில் (!len || *end) {
		debug("malformed property '%s'\n", type);
		திரும்பு -EINVAL;
	}

	str = name;
	க்கு (i = 0; i < index; i++) {
		str = strchr(str, '\0') + 1;
		எனில் (str > end) {
			found = false;
			நிறுத்து;
		}
	}

	எனில் (!found && CONFIG_IS_ENABLED(SYSINFO) && !sysinfo_get(&sysinfo)) {
		முழு rc;
		/*
		 * no string in the property for this index. Check if the
		 * sysinfo-level code can supply one.
		 */
		rc = sysinfo_detect(sysinfo);
		எனில் (rc)
			திரும்பு rc;

		rc = sysinfo_get_fit_loadable(sysinfo, index - i - 1, type,
					      &str);
		எனில் (rc && rc != -ENOENT)
			திரும்பு rc;

		எனில் (!rc) {
			/*
			 * The sysinfo provided a name for a loadable.
			 * Try to match it against the description properties
			 * first. If no matching node is found, use it as a
			 * node name.
			 */
			முழு முனை;
			முழு images = fdt_path_offset(ctx->fit, FIT_IMAGES_PATH);

			முனை = find_முனை_from_desc(ctx->fit, images, str);
			எனில் (முனை > 0)
				str = fdt_get_name(ctx->fit, முனை, வெற்று);

			found = true;
		}
	}

	எனில் (!found) {
		debug("no string for index %d\n", index);
		திரும்பு -E2BIG;
	}

	*outname = str;
	திரும்பு 0;
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
நிலையான முழு spl_fit_get_image_முனை(மாறிலி கட்டமைப்பு spl_fit_info *ctx,
				  மாறிலி எழுத்து *type, முழு index)
{
	மாறிலி எழுத்து *str;
	முழு err;
	முழு முனை;

	err = spl_fit_get_image_name(ctx, type, index, &str);
	எனில் (err)
		திரும்பு err;

	debug("%s: '%s'\n", type, str);

	முனை = fdt_subnode_offset(ctx->fit, ctx->images_முனை, str);
	எனில் (முனை < 0) {
		pr_err("cannot find image node '%s': %d\n", str, முனை);
		திரும்பு -EINVAL;
	}

	திரும்பு முனை;
}

நிலையான u32 get_aligned_image_offset(கட்டமைப்பு spl_load_info *info, u32 offset)
{
	திரும்பு ALIGN_DOWN(offset, spl_get_bl_len(info));
}

நிலையான u32 get_aligned_image_overhead(கட்டமைப்பு spl_load_info *info, u32 offset)
{
	திரும்பு offset & (spl_get_bl_len(info) - 1);
}

நிலையான முழு get_aligned_image_size(கட்டமைப்பு spl_load_info *info, ulong data_size,
				  u32 offset, ulong *aligned_size)
{
	u32 overhead = get_aligned_image_overhead(info, offset);

	எனில் (data_size > ULONG_MAX - overhead)
		திரும்பு -EOVERFLOW;
	data_size += overhead;

	எனில் (data_size > ULONG_MAX - (spl_get_bl_len(info) - 1))
		திரும்பு -EOVERFLOW;
	data_size = ALIGN(data_size, spl_get_bl_len(info));

	*aligned_size = data_size;

	திரும்பு 0;
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
நிலையான வெறுமை fit_fill_image_info(மாறிலி வெறுமை *fit, முழு முனை,
				கட்டமைப்பு spl_image_info *image_info,
				ulong load_addr, ulong size)
{
	ulong entry_point;

	எனில் (!image_info)
		திரும்பு;

	image_info->load_addr = load_addr;
	image_info->size = size;

	எனில் (!fit_image_get_entry(fit, முனை, &entry_point))
		image_info->entry_point = entry_point;
	இல்லையெனில்
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
நிலையான முழு load_simple_fit(கட்டமைப்பு spl_load_info *info, ulong fit_offset,
			   மாறிலி கட்டமைப்பு spl_fit_info *ctx, முழு முனை,
			   கட்டமைப்பு spl_image_info *image_info, ulong max_size)
{
	u32 offset;
	u32 len;
	size_t length;
	ulong size;
	ulong load_addr;
	வெறுமை *load_ptr;
	வெறுமை *src;
	uint8_t image_comp = -1, type = -1;
	மாறிலி வெறுமை *data;
	மாறிலி வெறுமை *fit = ctx->fit;
	bool external_data = false;
	முழு ret;

	log_debug("starting\n");
	எனில் (CONFIG_IS_ENABLED(BOOTMETH_VBE) &&
	    xpl_get_phase(info) != IH_PHASE_NONE) {
		எண்ணிக்கை image_phase_t phase;

		ret = fit_image_get_phase(fit, முனை, &phase);
		/* if the image is for any phase, let's use it */
		எனில் (ret == -ENOENT || phase == xpl_get_phase(info)) {
			log_debug("found\n");
		} இல்லையெனில் எனில் (ret < 0) {
			log_debug("err=%d\n", ret);
			திரும்பு ret;
		} இல்லையெனில் {
			log_debug("- phase mismatch, skipping this image\n");
			திரும்பு -EBADSLT;
		}
	}

	எனில் (IS_ENABLED(CONFIG_SPL_FPGA) ||
	    (IS_ENABLED(CONFIG_SPL_OS_BOOT) && spl_decompression_enabled())) {
		எனில் (fit_image_get_type(fit, முனை, &type))
			puts("Cannot get image type.\n");
		இல்லையெனில்
			debug("%s ", genimg_get_type_name(type));
	}

	எனில் (spl_decompression_enabled()) {
		fit_image_get_comp(fit, முனை, &image_comp);
		debug("%s ", genimg_get_comp_name(image_comp));
	}

	எனில் (fit_image_get_load(fit, முனை, &load_addr)) {
		எனில் (!image_info->load_addr) {
			printf("Can't load %s: No load address and no buffer\n",
			       fit_get_name(fit, முனை, வெற்று));
			திரும்பு -ENOBUFS;
		}
		load_addr = image_info->load_addr;
	}

	எனில் (!fit_image_get_data_position(fit, முனை, &offset)) {
		external_data = true;
	} இல்லையெனில் எனில் (!fit_image_get_data_offset(fit, முனை, &offset)) {
		/* The resulting offset cannot exceed UINT32_MAX */
		எனில் (ctx->ext_data_offset > UINT32_MAX - offset) {
			log_debug("Invalid external data offset: %u\n", offset);
			திரும்பு -EINVAL;
		}
		log_debug("read offset %x = offset from fit %x\n", offset,
			  (u32)(offset + ctx->ext_data_offset));
		offset += ctx->ext_data_offset;
		external_data = true;
	}

	எனில் (external_data) {
		u32 aligned_offset;
		ulong read_offset;
		வெறுமை *src_ptr;

		/* External data */
		எனில் (fit_image_get_data_size(fit, முனை, &len))
			திரும்பு -ENOENT;

		/* Dont bother to copy 0 byte data, but warn, though */
		எனில் (!len) {
			log_warning("%s: Skip load '%s': image size is 0!\n",
				    __func__, fit_get_name(fit, முனை, வெற்று));
			fit_fill_image_info(fit, முனை, image_info, load_addr, 0);
			திரும்பு 0;
		}

		/*
		 * data-size is excluded from the configuration signature (it
		 * is in exc_prop[] in image-fit-sig.c), so it stays attacker
		 * controlled even after fit_config_verify() succeeds. The
		 * image hash is only verified after the device read below, so
		 * an oversized value has to be rejected here.
		 */
		ret = get_aligned_image_size(info, len, offset, &size);
		எனில் (ret) {
			log_debug("Invalid external data size: %u\n", len);
			திரும்பு ret;
		}

		எனில் (size > max_size) {
			log_debug("Image too large: aligned size %lu, max %lu (data-size %u)\n",
				  size, max_size, len);
			திரும்பு -EFBIG;
		}

		aligned_offset = get_aligned_image_offset(info, offset);
		எனில் (aligned_offset > ULONG_MAX - fit_offset) {
			log_debug("Invalid aligned external data offset: %u\n",
				  aligned_offset);
			திரும்பு -EINVAL;
		}
		read_offset = fit_offset + aligned_offset;

		எனில் (spl_decompression_enabled() &&
		    (image_comp == IH_COMP_GZIP || image_comp == IH_COMP_LZMA))
			src_ptr = map_sysmem(ALIGN(CONFIG_SYS_LOAD_ADDR, ARCH_DMA_MINALIGN), len);
		இல்லையெனில்
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
		எனில் (size > ULONG_MAX - read_offset ||
		    size > UINTPTR_MAX - (uintptr_t)src_ptr
		/*
		 * #if (not a runtime if) is required: FIT_SIGNATURE_MAX_SIZE
		 * depends on FIT_SIGNATURE, so CONFIG_VAL(FIT_SIGNATURE_MAX_SIZE)
		 * is undefined when signing is disabled and referencing it
		 * here would fail to compile.
		 */
#எனில் CONFIG_IS_ENABLED(FIT_SIGNATURE)
		    || offset > CONFIG_VAL(FIT_SIGNATURE_MAX_SIZE) ||
		    len > CONFIG_VAL(FIT_SIGNATURE_MAX_SIZE) - offset
#endif
		) {
			log_debug("FIT external data is out of bounds (offset=%u, size=%u)\n",
				  offset, len);
			திரும்பு -EINVAL;
		}

		log_debug("reading from offset %x / %lx size %lx to %p: ",
			  offset, read_offset, size, src_ptr);

		எனில் (info->பட(info, read_offset, size, src_ptr) < length)
			திரும்பு -EIO;

		debug("External data: dst=%p, offset=%x, size=%lx\n",
		      src_ptr, offset, (unsigned long)length);
		src = src_ptr + get_aligned_image_overhead(info, offset);
	} இல்லையெனில் {
		/* Embedded data */
		எனில் (fit_image_get_emb_data(fit, முனை, &data, &length)) {
			puts("Cannot get image data/size\n");
			திரும்பு -ENOENT;
		}
		debug("Embedded data: dst=%lx, size=%lx\n", load_addr,
		      (unsigned long)length);
		src = (வெறுமை *)data;	/* cast away const */
	}

	எனில் (CONFIG_IS_ENABLED(FIT_SIGNATURE)) {
		printf("## Checking hash(es) for Image %s ... ",
		       fit_get_name(fit, முனை, வெற்று));
		எனில் (!fit_image_verify_with_data(fit, முனை, gd_fdt_blob(), src,
						length))
			திரும்பு -EPERM;
		puts("OK\n");
	}

	எனில் (CONFIG_IS_ENABLED(FIT_IMAGE_POST_PROCESS))
		board_fit_image_post_process(fit, முனை, &src, &length);

	load_ptr = map_sysmem(load_addr, length);
	எனில் (IS_ENABLED(CONFIG_SPL_GZIP) && image_comp == IH_COMP_GZIP) {
		size = length;
		எனில் (gunzip(load_ptr, CONFIG_SYS_BOOTM_LEN, src, &size)) {
			puts("Uncompressing error\n");
			திரும்பு -EIO;
		}
		length = size;
	} இல்லையெனில் எனில் (IS_ENABLED(CONFIG_SPL_LZMA) && image_comp == IH_COMP_LZMA) {
		size = CONFIG_SYS_BOOTM_LEN;
		ulong loadEnd;

		எனில் (image_decomp(IH_COMP_LZMA, CONFIG_SYS_LOAD_ADDR, 0, 0,
				 load_ptr, src, length, size, &loadEnd)) {
			puts("Uncompressing error\n");
			திரும்பு -EIO;
		}
		length = loadEnd - CONFIG_SYS_LOAD_ADDR;
	} இல்லையெனில் {
		memmove(load_ptr, src, length);
	}

	fit_fill_image_info(fit, முனை, image_info, load_addr, length);
	log_debug("- done loading\n");

	upl_add_image(fit, முனை, load_addr, length);

	திரும்பு 0;
}

நிலையான bool os_takes_devicetree(uint8_t os)
{
	தேர்வு (os) {
	நிலை IH_OS_U_BOOT:
		திரும்பு true;
	நிலை IH_OS_LINUX:
		திரும்பு IS_ENABLED(CONFIG_SPL_OS_BOOT) ||
		       IS_ENABLED(CONFIG_SPL_OPENSBI);
	இயல்பு:
		திரும்பு false;
	}
}

__weak முழு board_spl_fit_append_fdt_skip(மாறிலி எழுத்து *name)
{
	திரும்பு 0;	/* Do not skip */
}

நிலையான முழு spl_fit_append_fdt(கட்டமைப்பு spl_image_info *spl_image,
			      கட்டமைப்பு spl_load_info *info, ulong offset,
			      மாறிலி கட்டமைப்பு spl_fit_info *ctx)
{
	கட்டமைப்பு spl_image_info image_info;
	முழு முனை, ret = 0, index = 0;

	/*
	 * Use the address following the image as target address for the
	 * device tree.
	 */
	image_info.load_addr = ALIGN(spl_image->load_addr + spl_image->size, 8);

	/* Figure out which device tree the board wants to use */
	முனை = spl_fit_get_image_முனை(ctx, FIT_FDT_PROP, index++);
	எனில் (முனை < 0) {
		size_t size;

		debug("%s: cannot find FDT node\n", __func__);

		/*
		 * U-Boot did not find a device tree inside the FIT image. Use
		 * the U-Boot device tree instead.
		 */
		எனில் (!gd->fdt_blob)
			திரும்பு முனை;

		/*
		 * Make the load-address of the FDT available for the SPL
		 * framework
		 */
		size = fdt_totalsize(gd->fdt_blob);
		spl_image->fdt_addr = map_sysmem(image_info.load_addr, size);
		memcpy(spl_image->fdt_addr, gd->fdt_blob, size);
	} இல்லையெனில் {
		ret = load_simple_fit(info, offset, ctx, முனை, &image_info,
				      CONFIG_SYS_BOOTM_LEN);
		எனில் (ret < 0)
			திரும்பு ret;

		spl_image->fdt_addr = phys_to_virt(image_info.load_addr);
	}

	எனில் (CONFIG_IS_ENABLED(FIT_IMAGE_TINY))
		திரும்பு 0;

#எனில் CONFIG_IS_ENABLED(LOAD_FIT_APPLY_OVERLAY)
		வெறுமை *tmpbuffer = வெற்று;

		க்கு (; ; index++) {
			மாறிலி எழுத்து *str;

			ret = spl_fit_get_image_name(ctx, FIT_FDT_PROP, index, &str);
			எனில் (ret == -E2BIG) {
				debug("%s: No additional FDT node\n", __func__);
				ret = 0;
				நிறுத்து;
			} இல்லையெனில் எனில் (ret) {
				நிறுத்து;
			}

			ret = board_spl_fit_append_fdt_skip(str);
			எனில் (ret)
				தொடர்;

			முனை = fdt_subnode_offset(ctx->fit, ctx->images_முனை, str);
			எனில் (முனை < 0) {
				debug("%s: unable to find FDT node %d\n",
				      __func__, index);
				தொடர்;
			}

			எனில் (!tmpbuffer) {
				/*
				 * allocate memory to store the DT overlay
				 * before it is applied. It may not be used
				 * depending on how the overlay is stored, so
				 * don't fail yet if the allocation failed.
				 */
				size_t size = CONFIG_SPL_LOAD_FIT_APPLY_OVERLAY_BUF_SZ;

				tmpbuffer = malloc_cache_aligned(size);
				எனில் (!tmpbuffer)
					debug("%s: unable to allocate space for overlays\n",
					      __func__);
			}
			image_info.load_addr = (ulong)tmpbuffer;
			ret = load_simple_fit(info, offset, ctx, முனை,
					      &image_info,
					      CONFIG_SPL_LOAD_FIT_APPLY_OVERLAY_BUF_SZ);
			எனில் (ret == -EBADSLT)
				தொடர்;
			இல்லையெனில் எனில் (ret < 0)
				நிறுத்து;

			/* Make room in FDT for changes from the overlay */
			ret = fdt_increase_size(spl_image->fdt_addr,
						image_info.size);
			எனில் (ret < 0)
				நிறுத்து;

			ret = fdt_overlay_apply_verbose(spl_image->fdt_addr,
							(வெறுமை *)image_info.load_addr);
			எனில் (ret) {
				pr_err("failed to apply DT overlay %s\n",
				       fit_get_name(ctx->fit, முனை, வெற்று));
				நிறுத்து;
			}

			debug("%s: DT overlay %s applied\n", __func__,
			      fit_get_name(ctx->fit, முனை, வெற்று));
		}
		விடுவி(tmpbuffer);
		எனில் (ret)
			திரும்பு ret;
#endif
	/* Try to make space, so we can inject details on the loadables */
	ret = fdt_shrink_to_minimum(spl_image->fdt_addr, 8192);
	எனில் (ret < 0)
		திரும்பு ret;

	திரும்பு ret;
}

நிலையான முழு spl_fit_record_loadable(மாறிலி கட்டமைப்பு spl_fit_info *ctx, முழு index,
				   வெறுமை *blob, கட்டமைப்பு spl_image_info *image)
{
	முழு ret = 0;
	மாறிலி எழுத்து *name;
	முழு முனை;

	ret = spl_fit_get_image_name(ctx, FIT_LOADABLE_PROP, index, &name);
	எனில் (ret < 0)
		திரும்பு ret;

	முனை = spl_fit_get_image_முனை(ctx, FIT_LOADABLE_PROP, index);

	ret = fdt_record_loadable(blob, index, name, image->load_addr,
			image->size, image->entry_point,
			fdt_getprop(ctx->fit, முனை, FIT_TYPE_PROP, வெற்று),
			fdt_getprop(ctx->fit, முனை, FIT_OS_PROP, வெற்று),
			fdt_getprop(ctx->fit, முனை, FIT_ARCH_PROP, வெற்று));

	திரும்பு ret;
}

நிலையான முழு spl_fit_image_is_fpga(மாறிலி வெறுமை *fit, முழு முனை)
{
	மாறிலி எழுத்து *type;

	எனில் (!IS_ENABLED(CONFIG_SPL_FPGA))
		திரும்பு 0;

	type = fdt_getprop(fit, முனை, FIT_TYPE_PROP, வெற்று);
	எனில் (!type)
		திரும்பு 0;

	திரும்பு !strcmp(type, "fpga");
}

நிலையான வெறுமை spl_fit_image_record_arm32_optee(மாறிலி வெறுமை *fit, முழு முனை,
					     கட்டமைப்பு spl_image_info *spl_image,
					     கட்டமைப்பு spl_image_info *image_info)
{
#எனில் defined(CONFIG_BOOTM_OPTEE) && defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
	மாறிலி எழுத்து *type = fdt_getprop(fit, முனை, FIT_TYPE_PROP, வெற்று);

	எனில் (!type)
		திரும்பு;

	எனில் (strcmp(type, "tee"))
		திரும்பு;

	spl_image->optee_addr = image_info->load_addr;
#endif
}

நிலையான முழு spl_fit_image_get_os(மாறிலி வெறுமை *fit, முழு noffset, uint8_t *os)
{
	எனில் (!CONFIG_IS_ENABLED(FIT_IMAGE_TINY) || CONFIG_IS_ENABLED(OS_BOOT))
		திரும்பு fit_image_get_os(fit, noffset, os);

	மாறிலி எழுத்து *name = fdt_getprop(fit, noffset, FIT_OS_PROP, வெற்று);
	எனில் (!name)
		திரும்பு -ENOENT;

	/*
	 * We don't care what the type of the image actually is,
	 * only whether or not it is U-Boot. This saves some
	 * space by omitting the large table of OS types.
	 */
	எனில் (!strcmp(name, "u-boot"))
		*os = IH_OS_U_BOOT;
	இல்லையெனில்
		*os = IH_OS_INVALID;

	திரும்பு 0;
}

/*
 * The purpose of the FIT load buffer is to provide a memory location that is
 * independent of the load address of any FIT component.
 */
__weak வெறுமை *board_spl_fit_buffer_addr(ulong fit_size, முழு sectors, முழு bl_len)
{
	வெறுமை *buf;
	size_t size = sectors * bl_len;

	buf = malloc_cache_aligned(size);
	எனில் (!buf) {
		pr_err("Could not get FIT buffer of %lu bytes\n", (ulong)size);

		எனில் (IS_ENABLED(CONFIG_SPL_SYS_MALLOC))
			pr_err("\tcheck CONFIG_SPL_SYS_MALLOC_SIZE\n");
		இல்லையெனில்
			pr_err("\tcheck CONFIG_SPL_SYS_MALLOC_F_LEN\n");

		buf = spl_get_load_buffer(0, size);
	}
	திரும்பு buf;
}

/*
 * Weak default function to allow customizing SPL fit loading for load-only
 * use cases by allowing to skip the parsing/processing of the FIT contents
 * (so that this can be done separately in a more customized fashion)
 */
__weak bool spl_load_simple_fit_skip_processing(வெறுமை)
{
	திரும்பு false;
}

/*
 * Weak default function to allow fixes after fit header
 * is loaded.
 */
__weak வெறுமை *spl_load_simple_fit_fix_load(மாறிலி வெறுமை *fit)
{
	திரும்பு (வெறுமை *)fit;
}

நிலையான வெறுமை warn_deprecated(மாறிலி எழுத்து *செய்தி)
{
	printf("DEPRECATED: %s\n", செய்தி);
	printf("\tSee https://fitspec.osfw.foundation/\n");
}

நிலையான முழு spl_fit_upload_fpga(கட்டமைப்பு spl_fit_info *ctx, முழு முனை,
			       கட்டமைப்பு spl_image_info *fpga_image)
{
	மாறிலி எழுத்து *compatible;
	முழு ret;
	முழு devnum = 0;
	முழு flags = 0;

	debug("FPGA bitstream at: %x, size: %x\n",
	      (u32)fpga_image->load_addr, fpga_image->size);

	compatible = fdt_getprop(ctx->fit, முனை, FIT_COMPAT_PROP, வெற்று);
	எனில் (!compatible) {
		warn_deprecated("'fpga' image without 'compatible' property");
	} இல்லையெனில் {
		எனில் (CONFIG_IS_ENABLED(FPGA_LOAD_SECURE))
			flags = fpga_compatible2flag(devnum, compatible);
		எனில் (strcmp(compatible, "u-boot,fpga-legacy"))
			debug("Ignoring compatible = %s property\n",
			      compatible);
	}

	ret = fpga_load(devnum, (வெறுமை *)fpga_image->load_addr,
			fpga_image->size, BIT_FULL, flags);
	எனில் (ret) {
		printf("%s: Cannot load the image to the FPGA\n", __func__);
		திரும்பு ret;
	}

	puts("FPGA image loaded from FIT\n");

	திரும்பு 0;
}

நிலையான முழு spl_fit_load_fpga(கட்டமைப்பு spl_fit_info *ctx,
			     கட்டமைப்பு spl_load_info *info, ulong offset)
{
	முழு முனை, ret;

	கட்டமைப்பு spl_image_info fpga_image = {
		.load_addr = 0,
	};

	முனை = spl_fit_get_image_முனை(ctx, FIT_FPGA_PROP, 0);
	எனில் (முனை < 0)
		திரும்பு முனை;

	warn_deprecated("'fpga' property in config node. Use 'loadables'");

	/* Load the image and set up the fpga_image structure */
	ret = load_simple_fit(info, offset, ctx, முனை, &fpga_image,
			      CONFIG_SYS_BOOTM_LEN);
	எனில் (ret) {
		printf("%s: Cannot load the FPGA: %i\n", __func__, ret);
		திரும்பு ret;
	}

	திரும்பு spl_fit_upload_fpga(ctx, முனை, &fpga_image);
}

நிலையான முழு spl_simple_fit_read(கட்டமைப்பு spl_fit_info *ctx,
			       கட்டமைப்பு spl_load_info *info, ulong offset,
			       மாறிலி வெறுமை *fit_header)
{
	unsigned long aligned_size, மொத்தம், size;
	வெறுமை *buf;
	முழு ret;

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
	எனில் (ret) {
		log_debug("Invalid FIT size: %lu\n", size);
		திரும்பு ret;
	}
	buf = board_spl_fit_buffer_addr(aligned_size, aligned_size, 1);
	எனில் (!buf) {
		/*
		 * We assume that none of the board will ever use 0x0 as a
		 * valid load address. Theoretically some board could use it,
		 * but this is extremely unlikely.
		 */
		திரும்பு -EIO;
	}

	மொத்தம் = info->பட(info, offset, aligned_size, buf);
	எனில் (!மொத்தம்) {
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
		திரும்பு -EIO;
	}

	ctx->fit = buf;
	debug("fit read offset %lx, size=%lu, dst=%p, count=%lu\n",
	      offset, aligned_size, buf, மொத்தம்);

	திரும்பு 0;
}

நிலையான முழு spl_simple_fit_parse(கட்டமைப்பு spl_fit_info *ctx)
{
	/* Find the correct subnode under "/configurations" */
	ctx->conf_முனை = fit_find_config_முனை(ctx->fit);
	எனில் (ctx->conf_முனை < 0)
		திரும்பு -EINVAL;

	எனில் (CONFIG_IS_ENABLED(FIT_SIGNATURE)) {
		printf("## Checking hash(es) for config %s ... ",
		       fit_get_name(ctx->fit, ctx->conf_முனை, வெற்று));
		எனில் (fit_config_verify(ctx->fit, ctx->conf_முனை))
			திரும்பு -EPERM;
		puts("OK\n");
	}

	/* find the node holding the images information */
	ctx->images_முனை = fdt_path_offset(ctx->fit, FIT_IMAGES_PATH);
	எனில் (ctx->images_முனை < 0) {
		debug("%s: Cannot find /images node: %d\n", __func__,
		      ctx->images_முனை);
		திரும்பு -EINVAL;
	}

	திரும்பு 0;
}

முழு spl_load_simple_fit(கட்டமைப்பு spl_image_info *spl_image,
			கட்டமைப்பு spl_load_info *info, ulong offset, வெறுமை *fit)
{
	கட்டமைப்பு spl_image_info image_info;
	கட்டமைப்பு spl_fit_info ctx;
	முழு முனை = -1;
	முழு ret;
	முழு index = 0;
	முழு firmware_முனை;

	ret = spl_simple_fit_read(&ctx, info, offset, fit);
	எனில் (ret < 0)
		திரும்பு ret;

	/* skip further processing if requested to enable load-only use cases */
	எனில் (spl_load_simple_fit_skip_processing())
		திரும்பு 0;

	ctx.fit = spl_load_simple_fit_fix_load(ctx.fit);

	ret = spl_simple_fit_parse(&ctx);
	எனில் (ret < 0)
		திரும்பு ret;

	எனில் (IS_ENABLED(CONFIG_SPL_FPGA))
		spl_fit_load_fpga(&ctx, info, offset);

	/*
	 * Find the U-Boot image using the following search order:
	 *   - start at 'firmware' (e.g. an ARM Trusted Firmware)
	 *   - fall back 'kernel' (e.g. a Falcon-mode OS boot
	 *   - fall back to using the first 'loadables' entry
	 */
	எனில் (முனை < 0)
		முனை = spl_fit_get_image_முனை(&ctx, FIT_FIRMWARE_PROP, 0);

	எனில் (முனை < 0 && IS_ENABLED(CONFIG_SPL_OS_BOOT))
		முனை = spl_fit_get_image_முனை(&ctx, FIT_KERNEL_PROP, 0);

	எனில் (முனை < 0) {
		debug("could not find firmware image, trying loadables...\n");
		முனை = spl_fit_get_image_முனை(&ctx, FIT_LOADABLE_PROP, 0);
		/*
		 * If we pick the U-Boot image from "loadables", start at
		 * the second image when later loading additional images.
		 */
		index = 1;
	}
	எனில் (முனை < 0) {
		debug("%s: Cannot find u-boot image node: %d\n",
		      __func__, முனை);
		திரும்பு -1;
	}

	/* Load the image and set up the spl_image structure */
	ret = load_simple_fit(info, offset, &ctx, முனை, spl_image,
			      CONFIG_SYS_BOOTM_LEN);
	எனில் (ret)
		திரும்பு ret;

	/*
	 * For backward compatibility, we treat the first node that is
	 * as a U-Boot image, if no OS-type has been declared.
	 */
	எனில் (!spl_fit_image_get_os(ctx.fit, முனை, &spl_image->os))
		debug("Image OS is %s\n", genimg_get_os_name(spl_image->os));
	இல்லையெனில் எனில் (!IS_ENABLED(CONFIG_SPL_OS_BOOT))
		spl_image->os = IH_OS_U_BOOT;

	/*
	 * Booting a next-stage U-Boot may require us to append the FDT.
	 * We allow this to fail, as the U-Boot image might embed its FDT.
	 */
	எனில் (os_takes_devicetree(spl_image->os)) {
		ret = spl_fit_append_fdt(spl_image, info, offset, &ctx);
		எனில் (ret < 0 && spl_image->os != IH_OS_U_BOOT)
			திரும்பு ret;
	}

	firmware_முனை = முனை;
	/* Now check if there are more images for us to load */
	க்கு (; ; index++) {
		uint8_t os_type = IH_OS_INVALID;

		முனை = spl_fit_get_image_முனை(&ctx, FIT_LOADABLE_PROP, index);
		எனில் (முனை < 0)
			நிறுத்து;

		/*
		 * if the firmware is also a loadable, skip it because
		 * it already has been loaded. This is typically the case with
		 * u-boot.img generated by mkimage.
		 */
		எனில் (firmware_முனை == முனை)
			தொடர்;

		image_info.load_addr = 0;
		ret = load_simple_fit(info, offset, &ctx, முனை, &image_info,
				      CONFIG_SYS_BOOTM_LEN);
		எனில் (ret < 0 && ret != -EBADSLT) {
			printf("%s: can't load image loadables index %d (ret = %d)\n",
			       __func__, index, ret);
			திரும்பு ret;
		}

		எனில் (spl_fit_image_is_fpga(ctx.fit, முனை))
			spl_fit_upload_fpga(&ctx, முனை, &image_info);

		எனில் (!spl_fit_image_get_os(ctx.fit, முனை, &os_type))
			debug("Loadable is %s\n", genimg_get_os_name(os_type));

		எனில் (os_takes_devicetree(os_type)) {
			spl_fit_append_fdt(&image_info, info, offset, &ctx);
			spl_image->fdt_addr = image_info.fdt_addr;
		}

		/*
		 * If the "firmware" image did not provide an entry point,
		 * use the first valid entry point from the loadables.
		 */
		எனில் (spl_image->entry_point == FDT_ERROR &&
		    image_info.entry_point != FDT_ERROR)
			spl_image->entry_point = image_info.entry_point;

		spl_fit_image_record_arm32_optee(ctx.fit, முனை, spl_image,
						 &image_info);

		/* Record our loadables into the FDT */
		எனில் (!CONFIG_IS_ENABLED(FIT_IMAGE_TINY) &&
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
	எனில் (spl_image->entry_point == FDT_ERROR || spl_image->entry_point == 0)
		spl_image->entry_point = spl_image->load_addr;

	spl_image->flags |= SPL_FIT_FOUND;
	upl_set_fit_info(map_to_sysmem(ctx.fit), ctx.conf_முனை,
			 spl_image->entry_point);

	திரும்பு 0;
}

/* Parse and load full fitImage in SPL */
முழு spl_load_fit_image(கட்டமைப்பு spl_image_info *spl_image,
		       மாறிலி கட்டமைப்பு legacy_img_hdr *header)
{
	கட்டமைப்பு bootm_headers images;
	மாறிலி எழுத்து *fit_uname_config = வெற்று;
	ulong fdt_hack;
	மாறிலி எழுத்து *uname;
	ulong fw_data = 0, dt_data = 0, img_data = 0;
	ulong fw_len = 0, dt_len = 0, img_len = 0;
	முழு idx, conf_noffset;
	முழு ret;

	images.verify = CONFIG_IS_ENABLED(FIT_SIGNATURE);

	ret = fit_image_load(&images, virt_to_phys((வெறுமை *)header),
			     வெற்று, &fit_uname_config, IH_ARCH_DEFAULT,
			     IH_TYPE_FIRMWARE, -1, FIT_LOAD_OPTIONAL,
			     &fw_data, &fw_len);

	எனில் (ret < 0) {
		ret = fit_image_load(&images, virt_to_phys((வெறுமை *)header),
				     வெற்று, &fit_uname_config, IH_ARCH_DEFAULT,
				     IH_TYPE_KERNEL, -1, FIT_LOAD_OPTIONAL,
				     &fw_data, &fw_len);
	}

	எனில் (ret < 0)
		திரும்பு ret;

	spl_image->size = fw_len;
	spl_image->load_addr = fw_data;
	எனில் (fit_image_get_entry(header, ret, &spl_image->entry_point))
		spl_image->entry_point = fw_data;
	எனில் (fit_image_get_os(header, ret, &spl_image->os))
		spl_image->os = IH_OS_INVALID;
	spl_image->name = genimg_get_os_name(spl_image->os);

	debug(PHASE_PROMPT "payload image: %32s load addr: 0x%lx size: %d\n",
	      spl_image->name, spl_image->load_addr, spl_image->size);

	images.verify = CONFIG_IS_ENABLED(FIT_SIGNATURE);

	ret = fit_image_load(&images, virt_to_phys((வெறுமை *)header), வெற்று,
			     &fit_uname_config, IH_ARCH_DEFAULT, IH_TYPE_FLATDT,
			     -1, FIT_LOAD_OPTIONAL, &dt_data, &dt_len);
	எனில் (ret >= 0) {
		எனில் (spl_image->os == IH_OS_U_BOOT) {
			/* HACK: U-Boot expects FDT at a specific address */
			fdt_hack = ALIGN(spl_image->load_addr + spl_image->size, 8);
			debug("Relocating FDT to %p\n", (வெறுமை *)fdt_hack);
			memcpy(map_sysmem(fdt_hack, dt_len),
			       map_sysmem(dt_data, 0), dt_len);
			spl_image->fdt_addr = (வெறுமை *)fdt_hack;
		} இல்லையெனில் {
			spl_image->fdt_addr = (வெறுமை *)dt_data;
		}
	}

	conf_noffset = fit_conf_get_முனை((மாறிலி வெறுமை *)header,
					 fit_uname_config);
	எனில் (conf_noffset < 0)
		திரும்பு 0;

	க்கு (idx = 0;
	     uname = fdt_stringlist_get((மாறிலி வெறுமை *)header, conf_noffset,
					FIT_LOADABLE_PROP, idx,
				வெற்று), uname;
	     idx++) {
		images.verify = CONFIG_IS_ENABLED(FIT_SIGNATURE);

		ret = fit_image_load(&images, virt_to_phys((வெறுமை *)header),
				     &uname, &fit_uname_config,
				     IH_ARCH_DEFAULT, IH_TYPE_LOADABLE, -1,
				     FIT_LOAD_OPTIONAL_NON_ZERO,
				     &img_data, &img_len);
		எனில் (ret < 0)
			திரும்பு ret;
	}
	spl_image->flags |= SPL_FIT_FOUND;

	upl_set_fit_info(map_to_sysmem(header), conf_noffset,
			 spl_image->entry_point);

	திரும்பு 0;
}
