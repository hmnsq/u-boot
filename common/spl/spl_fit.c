// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#অন্তর্ভাৱ <errno.h>
#অন্তর্ভাৱ <fpga.h>
#অন্তর্ভাৱ <gzip.h>
#অন্তর্ভাৱ <image.h>
#অন্তর্ভাৱ <log.h>
#অন্তর্ভাৱ <memalign.h>
#অন্তর্ভাৱ <mapmem.h>
#অন্তর্ভাৱ <spl.h>
#অন্তর্ভাৱ <upl.h>
#অন্তর্ভাৱ <sysinfo.h>
#অন্তর্ভাৱ <asm/global_data.h>
#অন্তর্ভাৱ <asm/io.h>
#অন্তর্ভাৱ <linux/libfdt.h>
#অন্তর্ভাৱ <linux/printk.h>

DECLARE_GLOBAL_DATA_PTR;

সংরচনা spl_fit_info {
	ধ্রুৱ শূন্য *fit;	/* Pointer to a valid FIT blob */
	size_t ext_data_offset;	/* Offset to FIT external data (end of FIT) */
	পূর্ণাঙ্ক images_গ্ৰন্থি;	/* FDT offset to "/images" node */
	পূর্ণাঙ্ক conf_গ্ৰন্থি;		/* FDT offset to selected configuration node */
};

__weak ulong board_spl_fit_size_align(ulong size)
{
	ঘূৰাই size;
}

স্থির পূর্ণাঙ্ক find_গ্ৰন্থি_from_desc(ধ্রুৱ শূন্য *fit, পূর্ণাঙ্ক গ্ৰন্থি, ধ্রুৱ ৱর্ণ *str)
{
	পূর্ণাঙ্ক child;

	যদি (গ্ৰন্থি < 0)
		ঘূৰাই -EINVAL;

	/* iterate the FIT nodes and find a matching description */
	বাবে (child = fdt_first_subnode(fit, গ্ৰন্থি); child >= 0;
	     child = fdt_next_subnode(fit, child)) {
		পূর্ণাঙ্ক len;
		ধ্রুৱ ৱর্ণ *desc = fdt_getprop(fit, child, FIT_DESC_PROP, &len);

		যদি (!desc)
			অনুৱর্ত;

		যদি (!strcmp(desc, str))
			ঘূৰাই child;
	}

	ঘূৰাই -ENOENT;
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
স্থির পূর্ণাঙ্ক spl_fit_get_image_name(ধ্রুৱ সংরচনা spl_fit_info *ctx,
				  ধ্রুৱ ৱর্ণ *type, পূর্ণাঙ্ক index,
				  ধ্রুৱ ৱর্ণ **outname)
{
	সংরচনা udevice *sysinfo;
	ধ্রুৱ ৱর্ণ *name, *str, *end;
	__maybe_unused পূর্ণাঙ্ক গ্ৰন্থি;
	পূর্ণাঙ্ক len, i;
	bool found = true;

	name = fdt_getprop(ctx->fit, ctx->conf_গ্ৰন্থি, type, &len);
	যদি (!name) {
		debug("cannot find property '%s': %d\n", type, len);
		ঘূৰাই -EINVAL;
	}
	/* A string property should be NUL terminated */
	end = name + len - 1;
	যদি (!len || *end) {
		debug("malformed property '%s'\n", type);
		ঘূৰাই -EINVAL;
	}

	str = name;
	বাবে (i = 0; i < index; i++) {
		str = strchr(str, '\0') + 1;
		যদি (str > end) {
			found = false;
			ভঙ্গ;
		}
	}

	যদি (!found && CONFIG_IS_ENABLED(SYSINFO) && !sysinfo_get(&sysinfo)) {
		পূর্ণাঙ্ক rc;
		/*
		 * no string in the property for this index. Check if the
		 * sysinfo-level code can supply one.
		 */
		rc = sysinfo_detect(sysinfo);
		যদি (rc)
			ঘূৰাই rc;

		rc = sysinfo_get_fit_loadable(sysinfo, index - i - 1, type,
					      &str);
		যদি (rc && rc != -ENOENT)
			ঘূৰাই rc;

		যদি (!rc) {
			/*
			 * The sysinfo provided a name for a loadable.
			 * Try to match it against the description properties
			 * first. If no matching node is found, use it as a
			 * node name.
			 */
			পূর্ণাঙ্ক গ্ৰন্থি;
			পূর্ণাঙ্ক images = fdt_path_offset(ctx->fit, FIT_IMAGES_PATH);

			গ্ৰন্থি = find_গ্ৰন্থি_from_desc(ctx->fit, images, str);
			যদি (গ্ৰন্থি > 0)
				str = fdt_get_name(ctx->fit, গ্ৰন্থি, রিক্ত);

			found = true;
		}
	}

	যদি (!found) {
		debug("no string for index %d\n", index);
		ঘূৰাই -E2BIG;
	}

	*outname = str;
	ঘূৰাই 0;
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
স্থির পূর্ণাঙ্ক spl_fit_get_image_গ্ৰন্থি(ধ্রুৱ সংরচনা spl_fit_info *ctx,
				  ধ্রুৱ ৱর্ণ *type, পূর্ণাঙ্ক index)
{
	ধ্রুৱ ৱর্ণ *str;
	পূর্ণাঙ্ক err;
	পূর্ণাঙ্ক গ্ৰন্থি;

	err = spl_fit_get_image_name(ctx, type, index, &str);
	যদি (err)
		ঘূৰাই err;

	debug("%s: '%s'\n", type, str);

	গ্ৰন্থি = fdt_subnode_offset(ctx->fit, ctx->images_গ্ৰন্থি, str);
	যদি (গ্ৰন্থি < 0) {
		pr_err("cannot find image node '%s': %d\n", str, গ্ৰন্থি);
		ঘূৰাই -EINVAL;
	}

	ঘূৰাই গ্ৰন্থি;
}

স্থির u32 get_aligned_image_offset(সংরচনা spl_load_info *info, u32 offset)
{
	ঘূৰাই ALIGN_DOWN(offset, spl_get_bl_len(info));
}

স্থির u32 get_aligned_image_overhead(সংরচনা spl_load_info *info, u32 offset)
{
	ঘূৰাই offset & (spl_get_bl_len(info) - 1);
}

স্থির পূর্ণাঙ্ক get_aligned_image_size(সংরচনা spl_load_info *info, ulong data_size,
				  u32 offset, ulong *aligned_size)
{
	u32 overhead = get_aligned_image_overhead(info, offset);

	যদি (data_size > ULONG_MAX - overhead)
		ঘূৰাই -EOVERFLOW;
	data_size += overhead;

	যদি (data_size > ULONG_MAX - (spl_get_bl_len(info) - 1))
		ঘূৰাই -EOVERFLOW;
	data_size = ALIGN(data_size, spl_get_bl_len(info));

	*aligned_size = data_size;

	ঘূৰাই 0;
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
স্থির শূন্য fit_fill_image_info(ধ্রুৱ শূন্য *fit, পূর্ণাঙ্ক গ্ৰন্থি,
				সংরচনা spl_image_info *image_info,
				ulong load_addr, ulong size)
{
	ulong entry_point;

	যদি (!image_info)
		ঘূৰাই;

	image_info->load_addr = load_addr;
	image_info->size = size;

	যদি (!fit_image_get_entry(fit, গ্ৰন্থি, &entry_point))
		image_info->entry_point = entry_point;
	অন্যথা
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
স্থির পূর্ণাঙ্ক load_simple_fit(সংরচনা spl_load_info *info, ulong fit_offset,
			   ধ্রুৱ সংরচনা spl_fit_info *ctx, পূর্ণাঙ্ক গ্ৰন্থি,
			   সংরচনা spl_image_info *image_info, ulong max_size)
{
	u32 offset;
	u32 len;
	size_t length;
	ulong size;
	ulong load_addr;
	শূন্য *load_ptr;
	শূন্য *src;
	uint8_t image_comp = -1, type = -1;
	ধ্রুৱ শূন্য *data;
	ধ্রুৱ শূন্য *fit = ctx->fit;
	bool external_data = false;
	পূর্ণাঙ্ক ret;

	log_debug("starting\n");
	যদি (CONFIG_IS_ENABLED(BOOTMETH_VBE) &&
	    xpl_get_phase(info) != IH_PHASE_NONE) {
		গণনা image_phase_t phase;

		ret = fit_image_get_phase(fit, গ্ৰন্থি, &phase);
		/* if the image is for any phase, let's use it */
		যদি (ret == -ENOENT || phase == xpl_get_phase(info)) {
			log_debug("found\n");
		} অন্যথা যদি (ret < 0) {
			log_debug("err=%d\n", ret);
			ঘূৰাই ret;
		} অন্যথা {
			log_debug("- phase mismatch, skipping this image\n");
			ঘূৰাই -EBADSLT;
		}
	}

	যদি (IS_ENABLED(CONFIG_SPL_FPGA) ||
	    (IS_ENABLED(CONFIG_SPL_OS_BOOT) && spl_decompression_enabled())) {
		যদি (fit_image_get_type(fit, গ্ৰন্থি, &type))
			puts("Cannot get image type.\n");
		অন্যথা
			debug("%s ", genimg_get_type_name(type));
	}

	যদি (spl_decompression_enabled()) {
		fit_image_get_comp(fit, গ্ৰন্থি, &image_comp);
		debug("%s ", genimg_get_comp_name(image_comp));
	}

	যদি (fit_image_get_load(fit, গ্ৰন্থি, &load_addr)) {
		যদি (!image_info->load_addr) {
			printf("Can't load %s: No load address and no buffer\n",
			       fit_get_name(fit, গ্ৰন্থি, রিক্ত));
			ঘূৰাই -ENOBUFS;
		}
		load_addr = image_info->load_addr;
	}

	যদি (!fit_image_get_data_position(fit, গ্ৰন্থি, &offset)) {
		external_data = true;
	} অন্যথা যদি (!fit_image_get_data_offset(fit, গ্ৰন্থি, &offset)) {
		/* The resulting offset cannot exceed UINT32_MAX */
		যদি (ctx->ext_data_offset > UINT32_MAX - offset) {
			log_debug("Invalid external data offset: %u\n", offset);
			ঘূৰাই -EINVAL;
		}
		log_debug("read offset %x = offset from fit %x\n", offset,
			  (u32)(offset + ctx->ext_data_offset));
		offset += ctx->ext_data_offset;
		external_data = true;
	}

	যদি (external_data) {
		u32 aligned_offset;
		ulong read_offset;
		শূন্য *src_ptr;

		/* External data */
		যদি (fit_image_get_data_size(fit, গ্ৰন্থি, &len))
			ঘূৰাই -ENOENT;

		/* Dont bother to copy 0 byte data, but warn, though */
		যদি (!len) {
			log_warning("%s: Skip load '%s': image size is 0!\n",
				    __func__, fit_get_name(fit, গ্ৰন্থি, রিক্ত));
			fit_fill_image_info(fit, গ্ৰন্থি, image_info, load_addr, 0);
			ঘূৰাই 0;
		}

		/*
		 * data-size is excluded from the configuration signature (it
		 * is in exc_prop[] in image-fit-sig.c), so it stays attacker
		 * controlled even after fit_config_verify() succeeds. The
		 * image hash is only verified after the device read below, so
		 * an oversized value has to be rejected here.
		 */
		ret = get_aligned_image_size(info, len, offset, &size);
		যদি (ret) {
			log_debug("Invalid external data size: %u\n", len);
			ঘূৰাই ret;
		}

		যদি (size > max_size) {
			log_debug("Image too large: aligned size %lu, max %lu (data-size %u)\n",
				  size, max_size, len);
			ঘূৰাই -EFBIG;
		}

		aligned_offset = get_aligned_image_offset(info, offset);
		যদি (aligned_offset > ULONG_MAX - fit_offset) {
			log_debug("Invalid aligned external data offset: %u\n",
				  aligned_offset);
			ঘূৰাই -EINVAL;
		}
		read_offset = fit_offset + aligned_offset;

		যদি (spl_decompression_enabled() &&
		    (image_comp == IH_COMP_GZIP || image_comp == IH_COMP_LZMA))
			src_ptr = map_sysmem(ALIGN(CONFIG_SYS_LOAD_ADDR, ARCH_DMA_MINALIGN), len);
		অন্যথা
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
		যদি (size > ULONG_MAX - read_offset ||
		    size > UINTPTR_MAX - (uintptr_t)src_ptr
		/*
		 * #if (not a runtime if) is required: FIT_SIGNATURE_MAX_SIZE
		 * depends on FIT_SIGNATURE, so CONFIG_VAL(FIT_SIGNATURE_MAX_SIZE)
		 * is undefined when signing is disabled and referencing it
		 * here would fail to compile.
		 */
#যদি CONFIG_IS_ENABLED(FIT_SIGNATURE)
		    || offset > CONFIG_VAL(FIT_SIGNATURE_MAX_SIZE) ||
		    len > CONFIG_VAL(FIT_SIGNATURE_MAX_SIZE) - offset
#endif
		) {
			log_debug("FIT external data is out of bounds (offset=%u, size=%u)\n",
				  offset, len);
			ঘূৰাই -EINVAL;
		}

		log_debug("reading from offset %x / %lx size %lx to %p: ",
			  offset, read_offset, size, src_ptr);

		যদি (info->পঠ(info, read_offset, size, src_ptr) < length)
			ঘূৰাই -EIO;

		debug("External data: dst=%p, offset=%x, size=%lx\n",
		      src_ptr, offset, (unsigned long)length);
		src = src_ptr + get_aligned_image_overhead(info, offset);
	} অন্যথা {
		/* Embedded data */
		যদি (fit_image_get_emb_data(fit, গ্ৰন্থি, &data, &length)) {
			puts("Cannot get image data/size\n");
			ঘূৰাই -ENOENT;
		}
		debug("Embedded data: dst=%lx, size=%lx\n", load_addr,
		      (unsigned long)length);
		src = (শূন্য *)data;	/* cast away const */
	}

	যদি (CONFIG_IS_ENABLED(FIT_SIGNATURE)) {
		printf("## Checking hash(es) for Image %s ... ",
		       fit_get_name(fit, গ্ৰন্থি, রিক্ত));
		যদি (!fit_image_verify_with_data(fit, গ্ৰন্থি, gd_fdt_blob(), src,
						length))
			ঘূৰাই -EPERM;
		puts("OK\n");
	}

	যদি (CONFIG_IS_ENABLED(FIT_IMAGE_POST_PROCESS))
		board_fit_image_post_process(fit, গ্ৰন্থি, &src, &length);

	load_ptr = map_sysmem(load_addr, length);
	যদি (IS_ENABLED(CONFIG_SPL_GZIP) && image_comp == IH_COMP_GZIP) {
		size = length;
		যদি (gunzip(load_ptr, CONFIG_SYS_BOOTM_LEN, src, &size)) {
			puts("Uncompressing error\n");
			ঘূৰাই -EIO;
		}
		length = size;
	} অন্যথা যদি (IS_ENABLED(CONFIG_SPL_LZMA) && image_comp == IH_COMP_LZMA) {
		size = CONFIG_SYS_BOOTM_LEN;
		ulong loadEnd;

		যদি (image_decomp(IH_COMP_LZMA, CONFIG_SYS_LOAD_ADDR, 0, 0,
				 load_ptr, src, length, size, &loadEnd)) {
			puts("Uncompressing error\n");
			ঘূৰাই -EIO;
		}
		length = loadEnd - CONFIG_SYS_LOAD_ADDR;
	} অন্যথা {
		memmove(load_ptr, src, length);
	}

	fit_fill_image_info(fit, গ্ৰন্থি, image_info, load_addr, length);
	log_debug("- done loading\n");

	upl_add_image(fit, গ্ৰন্থি, load_addr, length);

	ঘূৰাই 0;
}

স্থির bool os_takes_devicetree(uint8_t os)
{
	ৱিকল্প (os) {
	পক্ষ IH_OS_U_BOOT:
		ঘূৰাই true;
	পক্ষ IH_OS_LINUX:
		ঘূৰাই IS_ENABLED(CONFIG_SPL_OS_BOOT) ||
		       IS_ENABLED(CONFIG_SPL_OPENSBI);
	স্ৱভাৱ:
		ঘূৰাই false;
	}
}

__weak পূর্ণাঙ্ক board_spl_fit_append_fdt_skip(ধ্রুৱ ৱর্ণ *name)
{
	ঘূৰাই 0;	/* Do not skip */
}

স্থির পূর্ণাঙ্ক spl_fit_append_fdt(সংরচনা spl_image_info *spl_image,
			      সংরচনা spl_load_info *info, ulong offset,
			      ধ্রুৱ সংরচনা spl_fit_info *ctx)
{
	সংরচনা spl_image_info image_info;
	পূর্ণাঙ্ক গ্ৰন্থি, ret = 0, index = 0;

	/*
	 * Use the address following the image as target address for the
	 * device tree.
	 */
	image_info.load_addr = ALIGN(spl_image->load_addr + spl_image->size, 8);

	/* Figure out which device tree the board wants to use */
	গ্ৰন্থি = spl_fit_get_image_গ্ৰন্থি(ctx, FIT_FDT_PROP, index++);
	যদি (গ্ৰন্থি < 0) {
		size_t size;

		debug("%s: cannot find FDT node\n", __func__);

		/*
		 * U-Boot did not find a device tree inside the FIT image. Use
		 * the U-Boot device tree instead.
		 */
		যদি (!gd->fdt_blob)
			ঘূৰাই গ্ৰন্থি;

		/*
		 * Make the load-address of the FDT available for the SPL
		 * framework
		 */
		size = fdt_totalsize(gd->fdt_blob);
		spl_image->fdt_addr = map_sysmem(image_info.load_addr, size);
		memcpy(spl_image->fdt_addr, gd->fdt_blob, size);
	} অন্যথা {
		ret = load_simple_fit(info, offset, ctx, গ্ৰন্থি, &image_info,
				      CONFIG_SYS_BOOTM_LEN);
		যদি (ret < 0)
			ঘূৰাই ret;

		spl_image->fdt_addr = phys_to_virt(image_info.load_addr);
	}

	যদি (CONFIG_IS_ENABLED(FIT_IMAGE_TINY))
		ঘূৰাই 0;

#যদি CONFIG_IS_ENABLED(LOAD_FIT_APPLY_OVERLAY)
		শূন্য *tmpbuffer = রিক্ত;

		বাবে (; ; index++) {
			ধ্রুৱ ৱর্ণ *str;

			ret = spl_fit_get_image_name(ctx, FIT_FDT_PROP, index, &str);
			যদি (ret == -E2BIG) {
				debug("%s: No additional FDT node\n", __func__);
				ret = 0;
				ভঙ্গ;
			} অন্যথা যদি (ret) {
				ভঙ্গ;
			}

			ret = board_spl_fit_append_fdt_skip(str);
			যদি (ret)
				অনুৱর্ত;

			গ্ৰন্থি = fdt_subnode_offset(ctx->fit, ctx->images_গ্ৰন্থি, str);
			যদি (গ্ৰন্থি < 0) {
				debug("%s: unable to find FDT node %d\n",
				      __func__, index);
				অনুৱর্ত;
			}

			যদি (!tmpbuffer) {
				/*
				 * allocate memory to store the DT overlay
				 * before it is applied. It may not be used
				 * depending on how the overlay is stored, so
				 * don't fail yet if the allocation failed.
				 */
				size_t size = CONFIG_SPL_LOAD_FIT_APPLY_OVERLAY_BUF_SZ;

				tmpbuffer = malloc_cache_aligned(size);
				যদি (!tmpbuffer)
					debug("%s: unable to allocate space for overlays\n",
					      __func__);
			}
			image_info.load_addr = (ulong)tmpbuffer;
			ret = load_simple_fit(info, offset, ctx, গ্ৰন্থি,
					      &image_info,
					      CONFIG_SPL_LOAD_FIT_APPLY_OVERLAY_BUF_SZ);
			যদি (ret == -EBADSLT)
				অনুৱর্ত;
			অন্যথা যদি (ret < 0)
				ভঙ্গ;

			/* Make room in FDT for changes from the overlay */
			ret = fdt_increase_size(spl_image->fdt_addr,
						image_info.size);
			যদি (ret < 0)
				ভঙ্গ;

			ret = fdt_overlay_apply_verbose(spl_image->fdt_addr,
							(শূন্য *)image_info.load_addr);
			যদি (ret) {
				pr_err("failed to apply DT overlay %s\n",
				       fit_get_name(ctx->fit, গ্ৰন্থি, রিক্ত));
				ভঙ্গ;
			}

			debug("%s: DT overlay %s applied\n", __func__,
			      fit_get_name(ctx->fit, গ্ৰন্থি, রিক্ত));
		}
		মুক্ত(tmpbuffer);
		যদি (ret)
			ঘূৰাই ret;
#endif
	/* Try to make space, so we can inject details on the loadables */
	ret = fdt_shrink_to_minimum(spl_image->fdt_addr, 8192);
	যদি (ret < 0)
		ঘূৰাই ret;

	ঘূৰাই ret;
}

স্থির পূর্ণাঙ্ক spl_fit_record_loadable(ধ্রুৱ সংরচনা spl_fit_info *ctx, পূর্ণাঙ্ক index,
				   শূন্য *blob, সংরচনা spl_image_info *image)
{
	পূর্ণাঙ্ক ret = 0;
	ধ্রুৱ ৱর্ণ *name;
	পূর্ণাঙ্ক গ্ৰন্থি;

	ret = spl_fit_get_image_name(ctx, FIT_LOADABLE_PROP, index, &name);
	যদি (ret < 0)
		ঘূৰাই ret;

	গ্ৰন্থি = spl_fit_get_image_গ্ৰন্থি(ctx, FIT_LOADABLE_PROP, index);

	ret = fdt_record_loadable(blob, index, name, image->load_addr,
			image->size, image->entry_point,
			fdt_getprop(ctx->fit, গ্ৰন্থি, FIT_TYPE_PROP, রিক্ত),
			fdt_getprop(ctx->fit, গ্ৰন্থি, FIT_OS_PROP, রিক্ত),
			fdt_getprop(ctx->fit, গ্ৰন্থি, FIT_ARCH_PROP, রিক্ত));

	ঘূৰাই ret;
}

স্থির পূর্ণাঙ্ক spl_fit_image_is_fpga(ধ্রুৱ শূন্য *fit, পূর্ণাঙ্ক গ্ৰন্থি)
{
	ধ্রুৱ ৱর্ণ *type;

	যদি (!IS_ENABLED(CONFIG_SPL_FPGA))
		ঘূৰাই 0;

	type = fdt_getprop(fit, গ্ৰন্থি, FIT_TYPE_PROP, রিক্ত);
	যদি (!type)
		ঘূৰাই 0;

	ঘূৰাই !strcmp(type, "fpga");
}

স্থির শূন্য spl_fit_image_record_arm32_optee(ধ্রুৱ শূন্য *fit, পূর্ণাঙ্ক গ্ৰন্থি,
					     সংরচনা spl_image_info *spl_image,
					     সংরচনা spl_image_info *image_info)
{
#যদি defined(CONFIG_BOOTM_OPTEE) && defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
	ধ্রুৱ ৱর্ণ *type = fdt_getprop(fit, গ্ৰন্থি, FIT_TYPE_PROP, রিক্ত);

	যদি (!type)
		ঘূৰাই;

	যদি (strcmp(type, "tee"))
		ঘূৰাই;

	spl_image->optee_addr = image_info->load_addr;
#endif
}

স্থির পূর্ণাঙ্ক spl_fit_image_get_os(ধ্রুৱ শূন্য *fit, পূর্ণাঙ্ক noffset, uint8_t *os)
{
	যদি (!CONFIG_IS_ENABLED(FIT_IMAGE_TINY) || CONFIG_IS_ENABLED(OS_BOOT))
		ঘূৰাই fit_image_get_os(fit, noffset, os);

	ধ্রুৱ ৱর্ণ *name = fdt_getprop(fit, noffset, FIT_OS_PROP, রিক্ত);
	যদি (!name)
		ঘূৰাই -ENOENT;

	/*
	 * We don't care what the type of the image actually is,
	 * only whether or not it is U-Boot. This saves some
	 * space by omitting the large table of OS types.
	 */
	যদি (!strcmp(name, "u-boot"))
		*os = IH_OS_U_BOOT;
	অন্যথা
		*os = IH_OS_INVALID;

	ঘূৰাই 0;
}

/*
 * The purpose of the FIT load buffer is to provide a memory location that is
 * independent of the load address of any FIT component.
 */
__weak শূন্য *board_spl_fit_buffer_addr(ulong fit_size, পূর্ণাঙ্ক sectors, পূর্ণাঙ্ক bl_len)
{
	শূন্য *buf;
	size_t size = sectors * bl_len;

	buf = malloc_cache_aligned(size);
	যদি (!buf) {
		pr_err("Could not get FIT buffer of %lu bytes\n", (ulong)size);

		যদি (IS_ENABLED(CONFIG_SPL_SYS_MALLOC))
			pr_err("\tcheck CONFIG_SPL_SYS_MALLOC_SIZE\n");
		অন্যথা
			pr_err("\tcheck CONFIG_SPL_SYS_MALLOC_F_LEN\n");

		buf = spl_get_load_buffer(0, size);
	}
	ঘূৰাই buf;
}

/*
 * Weak default function to allow customizing SPL fit loading for load-only
 * use cases by allowing to skip the parsing/processing of the FIT contents
 * (so that this can be done separately in a more customized fashion)
 */
__weak bool spl_load_simple_fit_skip_processing(শূন্য)
{
	ঘূৰাই false;
}

/*
 * Weak default function to allow fixes after fit header
 * is loaded.
 */
__weak শূন্য *spl_load_simple_fit_fix_load(ধ্রুৱ শূন্য *fit)
{
	ঘূৰাই (শূন্য *)fit;
}

স্থির শূন্য warn_deprecated(ধ্রুৱ ৱর্ণ *বাৰ্তা)
{
	printf("DEPRECATED: %s\n", বাৰ্তা);
	printf("\tSee https://fitspec.osfw.foundation/\n");
}

স্থির পূর্ণাঙ্ক spl_fit_upload_fpga(সংরচনা spl_fit_info *ctx, পূর্ণাঙ্ক গ্ৰন্থি,
			       সংরচনা spl_image_info *fpga_image)
{
	ধ্রুৱ ৱর্ণ *compatible;
	পূর্ণাঙ্ক ret;
	পূর্ণাঙ্ক devnum = 0;
	পূর্ণাঙ্ক flags = 0;

	debug("FPGA bitstream at: %x, size: %x\n",
	      (u32)fpga_image->load_addr, fpga_image->size);

	compatible = fdt_getprop(ctx->fit, গ্ৰন্থি, FIT_COMPAT_PROP, রিক্ত);
	যদি (!compatible) {
		warn_deprecated("'fpga' image without 'compatible' property");
	} অন্যথা {
		যদি (CONFIG_IS_ENABLED(FPGA_LOAD_SECURE))
			flags = fpga_compatible2flag(devnum, compatible);
		যদি (strcmp(compatible, "u-boot,fpga-legacy"))
			debug("Ignoring compatible = %s property\n",
			      compatible);
	}

	ret = fpga_load(devnum, (শূন্য *)fpga_image->load_addr,
			fpga_image->size, BIT_FULL, flags);
	যদি (ret) {
		printf("%s: Cannot load the image to the FPGA\n", __func__);
		ঘূৰাই ret;
	}

	puts("FPGA image loaded from FIT\n");

	ঘূৰাই 0;
}

স্থির পূর্ণাঙ্ক spl_fit_load_fpga(সংরচনা spl_fit_info *ctx,
			     সংরচনা spl_load_info *info, ulong offset)
{
	পূর্ণাঙ্ক গ্ৰন্থি, ret;

	সংরচনা spl_image_info fpga_image = {
		.load_addr = 0,
	};

	গ্ৰন্থি = spl_fit_get_image_গ্ৰন্থি(ctx, FIT_FPGA_PROP, 0);
	যদি (গ্ৰন্থি < 0)
		ঘূৰাই গ্ৰন্থি;

	warn_deprecated("'fpga' property in config node. Use 'loadables'");

	/* Load the image and set up the fpga_image structure */
	ret = load_simple_fit(info, offset, ctx, গ্ৰন্থি, &fpga_image,
			      CONFIG_SYS_BOOTM_LEN);
	যদি (ret) {
		printf("%s: Cannot load the FPGA: %i\n", __func__, ret);
		ঘূৰাই ret;
	}

	ঘূৰাই spl_fit_upload_fpga(ctx, গ্ৰন্থি, &fpga_image);
}

স্থির পূর্ণাঙ্ক spl_simple_fit_read(সংরচনা spl_fit_info *ctx,
			       সংরচনা spl_load_info *info, ulong offset,
			       ধ্রুৱ শূন্য *fit_header)
{
	unsigned long aligned_size, গুনতি, size;
	শূন্য *buf;
	পূর্ণাঙ্ক ret;

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
	যদি (ret) {
		log_debug("Invalid FIT size: %lu\n", size);
		ঘূৰাই ret;
	}
	buf = board_spl_fit_buffer_addr(aligned_size, aligned_size, 1);
	যদি (!buf) {
		/*
		 * We assume that none of the board will ever use 0x0 as a
		 * valid load address. Theoretically some board could use it,
		 * but this is extremely unlikely.
		 */
		ঘূৰাই -EIO;
	}

	গুনতি = info->পঠ(info, offset, aligned_size, buf);
	যদি (!গুনতি) {
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
		ঘূৰাই -EIO;
	}

	ctx->fit = buf;
	debug("fit read offset %lx, size=%lu, dst=%p, count=%lu\n",
	      offset, aligned_size, buf, গুনতি);

	ঘূৰাই 0;
}

স্থির পূর্ণাঙ্ক spl_simple_fit_parse(সংরচনা spl_fit_info *ctx)
{
	/* Find the correct subnode under "/configurations" */
	ctx->conf_গ্ৰন্থি = fit_find_config_গ্ৰন্থি(ctx->fit);
	যদি (ctx->conf_গ্ৰন্থি < 0)
		ঘূৰাই -EINVAL;

	যদি (CONFIG_IS_ENABLED(FIT_SIGNATURE)) {
		printf("## Checking hash(es) for config %s ... ",
		       fit_get_name(ctx->fit, ctx->conf_গ্ৰন্থি, রিক্ত));
		যদি (fit_config_verify(ctx->fit, ctx->conf_গ্ৰন্থি))
			ঘূৰাই -EPERM;
		puts("OK\n");
	}

	/* find the node holding the images information */
	ctx->images_গ্ৰন্থি = fdt_path_offset(ctx->fit, FIT_IMAGES_PATH);
	যদি (ctx->images_গ্ৰন্থি < 0) {
		debug("%s: Cannot find /images node: %d\n", __func__,
		      ctx->images_গ্ৰন্থি);
		ঘূৰাই -EINVAL;
	}

	ঘূৰাই 0;
}

পূর্ণাঙ্ক spl_load_simple_fit(সংরচনা spl_image_info *spl_image,
			সংরচনা spl_load_info *info, ulong offset, শূন্য *fit)
{
	সংরচনা spl_image_info image_info;
	সংরচনা spl_fit_info ctx;
	পূর্ণাঙ্ক গ্ৰন্থি = -1;
	পূর্ণাঙ্ক ret;
	পূর্ণাঙ্ক index = 0;
	পূর্ণাঙ্ক firmware_গ্ৰন্থি;

	ret = spl_simple_fit_read(&ctx, info, offset, fit);
	যদি (ret < 0)
		ঘূৰাই ret;

	/* skip further processing if requested to enable load-only use cases */
	যদি (spl_load_simple_fit_skip_processing())
		ঘূৰাই 0;

	ctx.fit = spl_load_simple_fit_fix_load(ctx.fit);

	ret = spl_simple_fit_parse(&ctx);
	যদি (ret < 0)
		ঘূৰাই ret;

	যদি (IS_ENABLED(CONFIG_SPL_FPGA))
		spl_fit_load_fpga(&ctx, info, offset);

	/*
	 * Find the U-Boot image using the following search order:
	 *   - start at 'firmware' (e.g. an ARM Trusted Firmware)
	 *   - fall back 'kernel' (e.g. a Falcon-mode OS boot
	 *   - fall back to using the first 'loadables' entry
	 */
	যদি (গ্ৰন্থি < 0)
		গ্ৰন্থি = spl_fit_get_image_গ্ৰন্থি(&ctx, FIT_FIRMWARE_PROP, 0);

	যদি (গ্ৰন্থি < 0 && IS_ENABLED(CONFIG_SPL_OS_BOOT))
		গ্ৰন্থি = spl_fit_get_image_গ্ৰন্থি(&ctx, FIT_KERNEL_PROP, 0);

	যদি (গ্ৰন্থি < 0) {
		debug("could not find firmware image, trying loadables...\n");
		গ্ৰন্থি = spl_fit_get_image_গ্ৰন্থি(&ctx, FIT_LOADABLE_PROP, 0);
		/*
		 * If we pick the U-Boot image from "loadables", start at
		 * the second image when later loading additional images.
		 */
		index = 1;
	}
	যদি (গ্ৰন্থি < 0) {
		debug("%s: Cannot find u-boot image node: %d\n",
		      __func__, গ্ৰন্থি);
		ঘূৰাই -1;
	}

	/* Load the image and set up the spl_image structure */
	ret = load_simple_fit(info, offset, &ctx, গ্ৰন্থি, spl_image,
			      CONFIG_SYS_BOOTM_LEN);
	যদি (ret)
		ঘূৰাই ret;

	/*
	 * For backward compatibility, we treat the first node that is
	 * as a U-Boot image, if no OS-type has been declared.
	 */
	যদি (!spl_fit_image_get_os(ctx.fit, গ্ৰন্থি, &spl_image->os))
		debug("Image OS is %s\n", genimg_get_os_name(spl_image->os));
	অন্যথা যদি (!IS_ENABLED(CONFIG_SPL_OS_BOOT))
		spl_image->os = IH_OS_U_BOOT;

	/*
	 * Booting a next-stage U-Boot may require us to append the FDT.
	 * We allow this to fail, as the U-Boot image might embed its FDT.
	 */
	যদি (os_takes_devicetree(spl_image->os)) {
		ret = spl_fit_append_fdt(spl_image, info, offset, &ctx);
		যদি (ret < 0 && spl_image->os != IH_OS_U_BOOT)
			ঘূৰাই ret;
	}

	firmware_গ্ৰন্থি = গ্ৰন্থি;
	/* Now check if there are more images for us to load */
	বাবে (; ; index++) {
		uint8_t os_type = IH_OS_INVALID;

		গ্ৰন্থি = spl_fit_get_image_গ্ৰন্থি(&ctx, FIT_LOADABLE_PROP, index);
		যদি (গ্ৰন্থি < 0)
			ভঙ্গ;

		/*
		 * if the firmware is also a loadable, skip it because
		 * it already has been loaded. This is typically the case with
		 * u-boot.img generated by mkimage.
		 */
		যদি (firmware_গ্ৰন্থি == গ্ৰন্থি)
			অনুৱর্ত;

		image_info.load_addr = 0;
		ret = load_simple_fit(info, offset, &ctx, গ্ৰন্থি, &image_info,
				      CONFIG_SYS_BOOTM_LEN);
		যদি (ret < 0 && ret != -EBADSLT) {
			printf("%s: can't load image loadables index %d (ret = %d)\n",
			       __func__, index, ret);
			ঘূৰাই ret;
		}

		যদি (spl_fit_image_is_fpga(ctx.fit, গ্ৰন্থি))
			spl_fit_upload_fpga(&ctx, গ্ৰন্থি, &image_info);

		যদি (!spl_fit_image_get_os(ctx.fit, গ্ৰন্থি, &os_type))
			debug("Loadable is %s\n", genimg_get_os_name(os_type));

		যদি (os_takes_devicetree(os_type)) {
			spl_fit_append_fdt(&image_info, info, offset, &ctx);
			spl_image->fdt_addr = image_info.fdt_addr;
		}

		/*
		 * If the "firmware" image did not provide an entry point,
		 * use the first valid entry point from the loadables.
		 */
		যদি (spl_image->entry_point == FDT_ERROR &&
		    image_info.entry_point != FDT_ERROR)
			spl_image->entry_point = image_info.entry_point;

		spl_fit_image_record_arm32_optee(ctx.fit, গ্ৰন্থি, spl_image,
						 &image_info);

		/* Record our loadables into the FDT */
		যদি (!CONFIG_IS_ENABLED(FIT_IMAGE_TINY) &&
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
	যদি (spl_image->entry_point == FDT_ERROR || spl_image->entry_point == 0)
		spl_image->entry_point = spl_image->load_addr;

	spl_image->flags |= SPL_FIT_FOUND;
	upl_set_fit_info(map_to_sysmem(ctx.fit), ctx.conf_গ্ৰন্থি,
			 spl_image->entry_point);

	ঘূৰাই 0;
}

/* Parse and load full fitImage in SPL */
পূর্ণাঙ্ক spl_load_fit_image(সংরচনা spl_image_info *spl_image,
		       ধ্রুৱ সংরচনা legacy_img_hdr *header)
{
	সংরচনা bootm_headers images;
	ধ্রুৱ ৱর্ণ *fit_uname_config = রিক্ত;
	ulong fdt_hack;
	ধ্রুৱ ৱর্ণ *uname;
	ulong fw_data = 0, dt_data = 0, img_data = 0;
	ulong fw_len = 0, dt_len = 0, img_len = 0;
	পূর্ণাঙ্ক idx, conf_noffset;
	পূর্ণাঙ্ক ret;

	images.verify = CONFIG_IS_ENABLED(FIT_SIGNATURE);

	ret = fit_image_load(&images, virt_to_phys((শূন্য *)header),
			     রিক্ত, &fit_uname_config, IH_ARCH_DEFAULT,
			     IH_TYPE_FIRMWARE, -1, FIT_LOAD_OPTIONAL,
			     &fw_data, &fw_len);

	যদি (ret < 0) {
		ret = fit_image_load(&images, virt_to_phys((শূন্য *)header),
				     রিক্ত, &fit_uname_config, IH_ARCH_DEFAULT,
				     IH_TYPE_KERNEL, -1, FIT_LOAD_OPTIONAL,
				     &fw_data, &fw_len);
	}

	যদি (ret < 0)
		ঘূৰাই ret;

	spl_image->size = fw_len;
	spl_image->load_addr = fw_data;
	যদি (fit_image_get_entry(header, ret, &spl_image->entry_point))
		spl_image->entry_point = fw_data;
	যদি (fit_image_get_os(header, ret, &spl_image->os))
		spl_image->os = IH_OS_INVALID;
	spl_image->name = genimg_get_os_name(spl_image->os);

	debug(PHASE_PROMPT "payload image: %32s load addr: 0x%lx size: %d\n",
	      spl_image->name, spl_image->load_addr, spl_image->size);

	images.verify = CONFIG_IS_ENABLED(FIT_SIGNATURE);

	ret = fit_image_load(&images, virt_to_phys((শূন্য *)header), রিক্ত,
			     &fit_uname_config, IH_ARCH_DEFAULT, IH_TYPE_FLATDT,
			     -1, FIT_LOAD_OPTIONAL, &dt_data, &dt_len);
	যদি (ret >= 0) {
		যদি (spl_image->os == IH_OS_U_BOOT) {
			/* HACK: U-Boot expects FDT at a specific address */
			fdt_hack = ALIGN(spl_image->load_addr + spl_image->size, 8);
			debug("Relocating FDT to %p\n", (শূন্য *)fdt_hack);
			memcpy(map_sysmem(fdt_hack, dt_len),
			       map_sysmem(dt_data, 0), dt_len);
			spl_image->fdt_addr = (শূন্য *)fdt_hack;
		} অন্যথা {
			spl_image->fdt_addr = (শূন্য *)dt_data;
		}
	}

	conf_noffset = fit_conf_get_গ্ৰন্থি((ধ্রুৱ শূন্য *)header,
					 fit_uname_config);
	যদি (conf_noffset < 0)
		ঘূৰাই 0;

	বাবে (idx = 0;
	     uname = fdt_stringlist_get((ধ্রুৱ শূন্য *)header, conf_noffset,
					FIT_LOADABLE_PROP, idx,
				রিক্ত), uname;
	     idx++) {
		images.verify = CONFIG_IS_ENABLED(FIT_SIGNATURE);

		ret = fit_image_load(&images, virt_to_phys((শূন্য *)header),
				     &uname, &fit_uname_config,
				     IH_ARCH_DEFAULT, IH_TYPE_LOADABLE, -1,
				     FIT_LOAD_OPTIONAL_NON_ZERO,
				     &img_data, &img_len);
		যদি (ret < 0)
			ঘূৰাই ret;
	}
	spl_image->flags |= SPL_FIT_FOUND;

	upl_set_fit_info(map_to_sysmem(header), conf_noffset,
			 spl_image->entry_point);

	ঘূৰাই 0;
}
