// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#অন্তর্ভাব <errno.h>
#অন্তর্ভাব <fpga.h>
#অন্তর্ভাব <gzip.h>
#অন্তর্ভাব <image.h>
#অন্তর্ভাব <log.h>
#অন্তর্ভাব <memalign.h>
#অন্তর্ভাব <mapmem.h>
#অন্তর্ভাব <spl.h>
#অন্তর্ভাব <upl.h>
#অন্তর্ভাব <sysinfo.h>
#অন্তর্ভাব <asm/global_data.h>
#অন্তর্ভাব <asm/io.h>
#অন্তর্ভাব <linux/libfdt.h>
#অন্তর্ভাব <linux/printk.h>

DECLARE_GLOBAL_তথ্য_PTR;

কাঠামো spl_fit_info {
	ধ্রুবক শূন্য *fit;	/* Pointer to a valid FIT blob */
	size_t ext_তথ্য_offset;	/* Offset to FIT external data (end of FIT) */
	পূর্ণসংখ্যা images_গ্রন্থি;	/* FDT offset to "/images" node */
	পূর্ণসংখ্যা conf_গ্রন্থি;		/* FDT offset to selected configuration node */
};

__weak ulong board_spl_fit_আকার_align(ulong আকার_2)
{
	ফেরত আকার_2;
}

স্থির পূর্ণসংখ্যা find_গ্রন্থি_from_desc(ধ্রুবক শূন্য *fit, পূর্ণসংখ্যা গ্রন্থি, ধ্রুবক অক্ষর *str)
{
	পূর্ণসংখ্যা child;

	যদি (গ্রন্থি < 0)
		ফেরত -EINVAL;

	/* iterate the FIT nodes and find a matching description */
	জন্য (child = fdt_first_subnode(fit, গ্রন্থি); child >= 0;
	     child = fdt_পরবর্তী_subnode(fit, child)) {
		পূর্ণসংখ্যা len;
		ধ্রুবক অক্ষর *desc = fdt_getprop(fit, child, FIT_DESC_PROP, &len);

		যদি (!desc)
			চালু;

		যদি (!strcmp(desc, str))
			ফেরত child;
	}

	ফেরত -ENOENT;
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
স্থির পূর্ণসংখ্যা spl_fit_get_image_name(ধ্রুবক কাঠামো spl_fit_info *ctx,
				  ধ্রুবক অক্ষর *type, পূর্ণসংখ্যা সূচক,
				  ধ্রুবক অক্ষর **outname)
{
	কাঠামো udevice *sysinfo;
	ধ্রুবক অক্ষর *name, *str, *end;
	__maybe_unused পূর্ণসংখ্যা গ্রন্থি;
	পূর্ণসংখ্যা len, i;
	bool found = true;

	name = fdt_getprop(ctx->fit, ctx->conf_গ্রন্থি, type, &len);
	যদি (!name) {
		debug("cannot find property '%s': %d\n", type, len);
		ফেরত -EINVAL;
	}
	/* A string property should be NUL terminated */
	end = name + len - 1;
	যদি (!len || *end) {
		debug("malformed property '%s'\n", type);
		ফেরত -EINVAL;
	}

	str = name;
	জন্য (i = 0; i < সূচক; i++) {
		str = strchr(str, '\0') + 1;
		যদি (str > end) {
			found = false;
			বিরতি;
		}
	}

	যদি (!found && CONFIG_IS_ENABLED(SYSINFO) && !sysinfo_get(&sysinfo)) {
		পূর্ণসংখ্যা rc;
		/*
		 * no string in the property for this index. Check if the
		 * sysinfo-level code can supply one.
		 */
		rc = sysinfo_detect(sysinfo);
		যদি (rc)
			ফেরত rc;

		rc = sysinfo_get_fit_loadable(sysinfo, সূচক - i - 1, type,
					      &str);
		যদি (rc && rc != -ENOENT)
			ফেরত rc;

		যদি (!rc) {
			/*
			 * The sysinfo provided a name for a loadable.
			 * Try to match it against the description properties
			 * first. If no matching node is found, use it as a
			 * node name.
			 */
			পূর্ণসংখ্যা গ্রন্থি;
			পূর্ণসংখ্যা images = fdt_path_offset(ctx->fit, FIT_IMAGES_PATH);

			গ্রন্থি = find_গ্রন্থি_from_desc(ctx->fit, images, str);
			যদি (গ্রন্থি > 0)
				str = fdt_get_name(ctx->fit, গ্রন্থি, ফাঁকা);

			found = true;
		}
	}

	যদি (!found) {
		debug("no string for index %d\n", সূচক);
		ফেরত -E2BIG;
	}

	*outname = str;
	ফেরত 0;
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
স্থির পূর্ণসংখ্যা spl_fit_get_image_গ্রন্থি(ধ্রুবক কাঠামো spl_fit_info *ctx,
				  ধ্রুবক অক্ষর *type, পূর্ণসংখ্যা সূচক)
{
	ধ্রুবক অক্ষর *str;
	পূর্ণসংখ্যা err;
	পূর্ণসংখ্যা গ্রন্থি;

	err = spl_fit_get_image_name(ctx, type, সূচক, &str);
	যদি (err)
		ফেরত err;

	debug("%s: '%s'\n", type, str);

	গ্রন্থি = fdt_subnode_offset(ctx->fit, ctx->images_গ্রন্থি, str);
	যদি (গ্রন্থি < 0) {
		pr_err("cannot find image node '%s': %d\n", str, গ্রন্থি);
		ফেরত -EINVAL;
	}

	ফেরত গ্রন্থি;
}

স্থির u32 get_aligned_image_offset(কাঠামো spl_load_info *info, u32 offset)
{
	ফেরত ALIGN_DOWN(offset, spl_get_bl_len(info));
}

স্থির u32 get_aligned_image_overhead(কাঠামো spl_load_info *info, u32 offset)
{
	ফেরত offset & (spl_get_bl_len(info) - 1);
}

স্থির পূর্ণসংখ্যা get_aligned_image_আকার(কাঠামো spl_load_info *info, ulong তথ্য_আকার,
				  u32 offset, ulong *aligned_আকার)
{
	u32 overhead = get_aligned_image_overhead(info, offset);

	যদি (তথ্য_আকার > ULONG_MAX - overhead)
		ফেরত -EOVERFLOW;
	তথ্য_আকার += overhead;

	যদি (তথ্য_আকার > ULONG_MAX - (spl_get_bl_len(info) - 1))
		ফেরত -EOVERFLOW;
	তথ্য_আকার = ALIGN(তথ্য_আকার, spl_get_bl_len(info));

	*aligned_আকার = তথ্য_আকার;

	ফেরত 0;
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
স্থির শূন্য fit_fill_image_info(ধ্রুবক শূন্য *fit, পূর্ণসংখ্যা গ্রন্থি,
				কাঠামো spl_image_info *image_info,
				ulong load_addr, ulong আকার_2)
{
	ulong entry_point;

	যদি (!image_info)
		ফেরত;

	image_info->load_addr = load_addr;
	image_info->আকার_2 = আকার_2;

	যদি (!fit_image_get_entry(fit, গ্রন্থি, &entry_point))
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
স্থির পূর্ণসংখ্যা load_simple_fit(কাঠামো spl_load_info *info, ulong fit_offset,
			   ধ্রুবক কাঠামো spl_fit_info *ctx, পূর্ণসংখ্যা গ্রন্থি,
			   কাঠামো spl_image_info *image_info, ulong max_আকার)
{
	u32 offset;
	u32 len;
	size_t দৈর্ঘ্য;
	ulong আকার_2;
	ulong load_addr;
	শূন্য *load_ptr;
	শূন্য *src;
	uint8_t image_comp = -1, type = -1;
	ধ্রুবক শূন্য *তথ্য;
	ধ্রুবক শূন্য *fit = ctx->fit;
	bool external_তথ্য = false;
	পূর্ণসংখ্যা ret;

	log_debug("starting\n");
	যদি (CONFIG_IS_ENABLED(BOOTMETH_VBE) &&
	    xpl_get_phase(info) != IH_PHASE_NONE) {
		গণনা image_phase_t phase;

		ret = fit_image_get_phase(fit, গ্রন্থি, &phase);
		/* if the image is for any phase, let's use it */
		যদি (ret == -ENOENT || phase == xpl_get_phase(info)) {
			log_debug("found\n");
		} অন্যথা যদি (ret < 0) {
			log_debug("err=%d\n", ret);
			ফেরত ret;
		} অন্যথা {
			log_debug("- phase mismatch, skipping this image\n");
			ফেরত -EBADSLT;
		}
	}

	যদি (IS_ENABLED(CONFIG_SPL_FPGA) ||
	    (IS_ENABLED(CONFIG_SPL_OS_BOOT) && spl_decompression_enabled())) {
		যদি (fit_image_get_type(fit, গ্রন্থি, &type))
			puts("Cannot get image type.\n");
		অন্যথা
			debug("%s ", genimg_get_type_name(type));
	}

	যদি (spl_decompression_enabled()) {
		fit_image_get_comp(fit, গ্রন্থি, &image_comp);
		debug("%s ", genimg_get_comp_name(image_comp));
	}

	যদি (fit_image_get_load(fit, গ্রন্থি, &load_addr)) {
		যদি (!image_info->load_addr) {
			printf("Can't load %s: No load address and no buffer\n",
			       fit_get_name(fit, গ্রন্থি, ফাঁকা));
			ফেরত -ENOBUFS;
		}
		load_addr = image_info->load_addr;
	}

	যদি (!fit_image_get_তথ্য_position(fit, গ্রন্থি, &offset)) {
		external_তথ্য = true;
	} অন্যথা যদি (!fit_image_get_তথ্য_offset(fit, গ্রন্থি, &offset)) {
		/* The resulting offset cannot exceed UINT32_MAX */
		যদি (ctx->ext_তথ্য_offset > UINT32_MAX - offset) {
			log_debug("Invalid external data offset: %u\n", offset);
			ফেরত -EINVAL;
		}
		log_debug("read offset %x = offset from fit %x\n", offset,
			  (u32)(offset + ctx->ext_তথ্য_offset));
		offset += ctx->ext_তথ্য_offset;
		external_তথ্য = true;
	}

	যদি (external_তথ্য) {
		u32 aligned_offset;
		ulong read_offset;
		শূন্য *src_ptr;

		/* External data */
		যদি (fit_image_get_তথ্য_আকার(fit, গ্রন্থি, &len))
			ফেরত -ENOENT;

		/* Dont bother to copy 0 byte data, but warn, though */
		যদি (!len) {
			log_warning("%s: Skip load '%s': image size is 0!\n",
				    __func__, fit_get_name(fit, গ্রন্থি, ফাঁকা));
			fit_fill_image_info(fit, গ্রন্থি, image_info, load_addr, 0);
			ফেরত 0;
		}

		/*
		 * data-size is excluded from the configuration signature (it
		 * is in exc_prop[] in image-fit-sig.c), so it stays attacker
		 * controlled even after fit_config_verify() succeeds. The
		 * image hash is only verified after the device read below, so
		 * an oversized value has to be rejected here.
		 */
		ret = get_aligned_image_আকার(info, len, offset, &আকার_2);
		যদি (ret) {
			log_debug("Invalid external data size: %u\n", len);
			ফেরত ret;
		}

		যদি (আকার_2 > max_আকার) {
			log_debug("Image too large: aligned size %lu, max %lu (data-size %u)\n",
				  আকার_2, max_আকার, len);
			ফেরত -EFBIG;
		}

		aligned_offset = get_aligned_image_offset(info, offset);
		যদি (aligned_offset > ULONG_MAX - fit_offset) {
			log_debug("Invalid aligned external data offset: %u\n",
				  aligned_offset);
			ফেরত -EINVAL;
		}
		read_offset = fit_offset + aligned_offset;

		যদি (spl_decompression_enabled() &&
		    (image_comp == IH_COMP_GZIP || image_comp == IH_COMP_LZMA))
			src_ptr = map_sysmem(ALIGN(CONFIG_SYS_LOAD_ADDR, ARCH_DMA_MINALIGN), len);
		অন্যথা
			src_ptr = map_sysmem(ALIGN(load_addr, ARCH_DMA_MINALIGN), len);
		দৈর্ঘ্য = len;

		/*
		 * For non-signed FIT images, we can check that
		 * (read_offset + size) does not wrap and that
		 * (src_ptr + size) does not exceed the addressable range.
		 * For signed FITs, we can additionally check that
		 * (offset + len) doesn't exceed the allowed FIT image
		 * maximum size.
		 */
		যদি (আকার_2 > ULONG_MAX - read_offset ||
		    আকার_2 > UINTPTR_MAX - (uintptr_t)src_ptr
		/*
		 * #if (not a runtime if) is required: FIT_SIGNATURE_MAX_SIZE
		 * depends on FIT_SIGNATURE, so CONFIG_VAL(FIT_SIGNATURE_MAX_SIZE)
		 * is undefined when signing is disabled and referencing it
		 * here would fail to compile.
		 */
#যদি CONFIG_IS_ENABLED(FIT_SIGNATURE)
		    || offset > CONFIG_VAL(FIT_SIGNATURE_MAX_আকার) ||
		    len > CONFIG_VAL(FIT_SIGNATURE_MAX_আকার) - offset
#endif
		) {
			log_debug("FIT external data is out of bounds (offset=%u, size=%u)\n",
				  offset, len);
			ফেরত -EINVAL;
		}

		log_debug("reading from offset %x / %lx size %lx to %p: ",
			  offset, read_offset, আকার_2, src_ptr);

		যদি (info->পঠ(info, read_offset, আকার_2, src_ptr) < দৈর্ঘ্য)
			ফেরত -EIO;

		debug("External data: dst=%p, offset=%x, size=%lx\n",
		      src_ptr, offset, (unsigned long)দৈর্ঘ্য);
		src = src_ptr + get_aligned_image_overhead(info, offset);
	} অন্যথা {
		/* Embedded data */
		যদি (fit_image_get_emb_তথ্য(fit, গ্রন্থি, &তথ্য, &দৈর্ঘ্য)) {
			puts("Cannot get image data/size\n");
			ফেরত -ENOENT;
		}
		debug("Embedded data: dst=%lx, size=%lx\n", load_addr,
		      (unsigned long)দৈর্ঘ্য);
		src = (শূন্য *)তথ্য;	/* cast away const */
	}

	যদি (CONFIG_IS_ENABLED(FIT_SIGNATURE)) {
		printf("## Checking hash(es) for Image %s ... ",
		       fit_get_name(fit, গ্রন্থি, ফাঁকা));
		যদি (!fit_image_verify_with_তথ্য(fit, গ্রন্থি, gd_fdt_blob(), src,
						দৈর্ঘ্য))
			ফেরত -EPERM;
		puts("OK\n");
	}

	যদি (CONFIG_IS_ENABLED(FIT_IMAGE_POST_PROCESS))
		board_fit_image_post_process(fit, গ্রন্থি, &src, &দৈর্ঘ্য);

	load_ptr = map_sysmem(load_addr, দৈর্ঘ্য);
	যদি (IS_ENABLED(CONFIG_SPL_GZIP) && image_comp == IH_COMP_GZIP) {
		আকার_2 = দৈর্ঘ্য;
		যদি (gunzip(load_ptr, CONFIG_SYS_BOOTM_LEN, src, &আকার_2)) {
			puts("Uncompressing error\n");
			ফেরত -EIO;
		}
		দৈর্ঘ্য = আকার_2;
	} অন্যথা যদি (IS_ENABLED(CONFIG_SPL_LZMA) && image_comp == IH_COMP_LZMA) {
		আকার_2 = CONFIG_SYS_BOOTM_LEN;
		ulong loadEnd;

		যদি (image_decomp(IH_COMP_LZMA, CONFIG_SYS_LOAD_ADDR, 0, 0,
				 load_ptr, src, দৈর্ঘ্য, আকার_2, &loadEnd)) {
			puts("Uncompressing error\n");
			ফেরত -EIO;
		}
		দৈর্ঘ্য = loadEnd - CONFIG_SYS_LOAD_ADDR;
	} অন্যথা {
		memmove(load_ptr, src, দৈর্ঘ্য);
	}

	fit_fill_image_info(fit, গ্রন্থি, image_info, load_addr, দৈর্ঘ্য);
	log_debug("- done loading\n");

	upl_add_image(fit, গ্রন্থি, load_addr, দৈর্ঘ্য);

	ফেরত 0;
}

স্থির bool os_takes_devicetree(uint8_t os)
{
	নির্বাচন (os) {
	ক্ষেত্র IH_OS_U_BOOT:
		ফেরত true;
	ক্ষেত্র IH_OS_LINUX:
		ফেরত IS_ENABLED(CONFIG_SPL_OS_BOOT) ||
		       IS_ENABLED(CONFIG_SPL_OPENSBI);
	পূর্বনির্ধারিত:
		ফেরত false;
	}
}

__weak পূর্ণসংখ্যা board_spl_fit_append_fdt_skip(ধ্রুবক অক্ষর *name)
{
	ফেরত 0;	/* Do not skip */
}

স্থির পূর্ণসংখ্যা spl_fit_append_fdt(কাঠামো spl_image_info *spl_image,
			      কাঠামো spl_load_info *info, ulong offset,
			      ধ্রুবক কাঠামো spl_fit_info *ctx)
{
	কাঠামো spl_image_info image_info;
	পূর্ণসংখ্যা গ্রন্থি, ret = 0, সূচক = 0;

	/*
	 * Use the address following the image as target address for the
	 * device tree.
	 */
	image_info.load_addr = ALIGN(spl_image->load_addr + spl_image->আকার_2, 8);

	/* Figure out which device tree the board wants to use */
	গ্রন্থি = spl_fit_get_image_গ্রন্থি(ctx, FIT_FDT_PROP, সূচক++);
	যদি (গ্রন্থি < 0) {
		size_t আকার_2;

		debug("%s: cannot find FDT node\n", __func__);

		/*
		 * U-Boot did not find a device tree inside the FIT image. Use
		 * the U-Boot device tree instead.
		 */
		যদি (!gd->fdt_blob)
			ফেরত গ্রন্থি;

		/*
		 * Make the load-address of the FDT available for the SPL
		 * framework
		 */
		আকার_2 = fdt_totalsize(gd->fdt_blob);
		spl_image->fdt_addr = map_sysmem(image_info.load_addr, আকার_2);
		memcpy(spl_image->fdt_addr, gd->fdt_blob, আকার_2);
	} অন্যথা {
		ret = load_simple_fit(info, offset, ctx, গ্রন্থি, &image_info,
				      CONFIG_SYS_BOOTM_LEN);
		যদি (ret < 0)
			ফেরত ret;

		spl_image->fdt_addr = phys_to_virt(image_info.load_addr);
	}

	যদি (CONFIG_IS_ENABLED(FIT_IMAGE_TINY))
		ফেরত 0;

#যদি CONFIG_IS_ENABLED(LOAD_FIT_APPLY_OVERLAY)
		শূন্য *tmpbuffer = ফাঁকা;

		জন্য (; ; সূচক++) {
			ধ্রুবক অক্ষর *str;

			ret = spl_fit_get_image_name(ctx, FIT_FDT_PROP, সূচক, &str);
			যদি (ret == -E2BIG) {
				debug("%s: No additional FDT node\n", __func__);
				ret = 0;
				বিরতি;
			} অন্যথা যদি (ret) {
				বিরতি;
			}

			ret = board_spl_fit_append_fdt_skip(str);
			যদি (ret)
				চালু;

			গ্রন্থি = fdt_subnode_offset(ctx->fit, ctx->images_গ্রন্থি, str);
			যদি (গ্রন্থি < 0) {
				debug("%s: unable to find FDT node %d\n",
				      __func__, সূচক);
				চালু;
			}

			যদি (!tmpbuffer) {
				/*
				 * allocate memory to store the DT overlay
				 * before it is applied. It may not be used
				 * depending on how the overlay is stored, so
				 * don't fail yet if the allocation failed.
				 */
				size_t আকার_2 = CONFIG_SPL_LOAD_FIT_APPLY_OVERLAY_BUF_SZ;

				tmpbuffer = malloc_cache_aligned(আকার_2);
				যদি (!tmpbuffer)
					debug("%s: unable to allocate space for overlays\n",
					      __func__);
			}
			image_info.load_addr = (ulong)tmpbuffer;
			ret = load_simple_fit(info, offset, ctx, গ্রন্থি,
					      &image_info,
					      CONFIG_SPL_LOAD_FIT_APPLY_OVERLAY_BUF_SZ);
			যদি (ret == -EBADSLT)
				চালু;
			অন্যথা যদি (ret < 0)
				বিরতি;

			/* Make room in FDT for changes from the overlay */
			ret = fdt_increase_আকার(spl_image->fdt_addr,
						image_info.আকার_2);
			যদি (ret < 0)
				বিরতি;

			ret = fdt_overlay_apply_verbose(spl_image->fdt_addr,
							(শূন্য *)image_info.load_addr);
			যদি (ret) {
				pr_err("failed to apply DT overlay %s\n",
				       fit_get_name(ctx->fit, গ্রন্থি, ফাঁকা));
				বিরতি;
			}

			debug("%s: DT overlay %s applied\n", __func__,
			      fit_get_name(ctx->fit, গ্রন্থি, ফাঁকা));
		}
		মুক্ত(tmpbuffer);
		যদি (ret)
			ফেরত ret;
#endif
	/* Try to make space, so we can inject details on the loadables */
	ret = fdt_shrink_to_minimum(spl_image->fdt_addr, 8192);
	যদি (ret < 0)
		ফেরত ret;

	ফেরত ret;
}

স্থির পূর্ণসংখ্যা spl_fit_record_loadable(ধ্রুবক কাঠামো spl_fit_info *ctx, পূর্ণসংখ্যা সূচক,
				   শূন্য *blob, কাঠামো spl_image_info *image)
{
	পূর্ণসংখ্যা ret = 0;
	ধ্রুবক অক্ষর *name;
	পূর্ণসংখ্যা গ্রন্থি;

	ret = spl_fit_get_image_name(ctx, FIT_LOADABLE_PROP, সূচক, &name);
	যদি (ret < 0)
		ফেরত ret;

	গ্রন্থি = spl_fit_get_image_গ্রন্থি(ctx, FIT_LOADABLE_PROP, সূচক);

	ret = fdt_record_loadable(blob, সূচক, name, image->load_addr,
			image->আকার_2, image->entry_point,
			fdt_getprop(ctx->fit, গ্রন্থি, FIT_TYPE_PROP, ফাঁকা),
			fdt_getprop(ctx->fit, গ্রন্থি, FIT_OS_PROP, ফাঁকা),
			fdt_getprop(ctx->fit, গ্রন্থি, FIT_ARCH_PROP, ফাঁকা));

	ফেরত ret;
}

স্থির পূর্ণসংখ্যা spl_fit_image_is_fpga(ধ্রুবক শূন্য *fit, পূর্ণসংখ্যা গ্রন্থি)
{
	ধ্রুবক অক্ষর *type;

	যদি (!IS_ENABLED(CONFIG_SPL_FPGA))
		ফেরত 0;

	type = fdt_getprop(fit, গ্রন্থি, FIT_TYPE_PROP, ফাঁকা);
	যদি (!type)
		ফেরত 0;

	ফেরত !strcmp(type, "fpga");
}

স্থির শূন্য spl_fit_image_record_arm32_optee(ধ্রুবক শূন্য *fit, পূর্ণসংখ্যা গ্রন্থি,
					     কাঠামো spl_image_info *spl_image,
					     কাঠামো spl_image_info *image_info)
{
#যদি defined(CONFIG_BOOTM_OPTEE) && defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
	ধ্রুবক অক্ষর *type = fdt_getprop(fit, গ্রন্থি, FIT_TYPE_PROP, ফাঁকা);

	যদি (!type)
		ফেরত;

	যদি (strcmp(type, "tee"))
		ফেরত;

	spl_image->optee_addr = image_info->load_addr;
#endif
}

স্থির পূর্ণসংখ্যা spl_fit_image_get_os(ধ্রুবক শূন্য *fit, পূর্ণসংখ্যা noffset, uint8_t *os)
{
	যদি (!CONFIG_IS_ENABLED(FIT_IMAGE_TINY) || CONFIG_IS_ENABLED(OS_BOOT))
		ফেরত fit_image_get_os(fit, noffset, os);

	ধ্রুবক অক্ষর *name = fdt_getprop(fit, noffset, FIT_OS_PROP, ফাঁকা);
	যদি (!name)
		ফেরত -ENOENT;

	/*
	 * We don't care what the type of the image actually is,
	 * only whether or not it is U-Boot. This saves some
	 * space by omitting the large table of OS types.
	 */
	যদি (!strcmp(name, "u-boot"))
		*os = IH_OS_U_BOOT;
	অন্যথা
		*os = IH_OS_INVALID;

	ফেরত 0;
}

/*
 * The purpose of the FIT load buffer is to provide a memory location that is
 * independent of the load address of any FIT component.
 */
__weak শূন্য *board_spl_fit_বাফার_addr(ulong fit_আকার, পূর্ণসংখ্যা sectors, পূর্ণসংখ্যা bl_len)
{
	শূন্য *buf;
	size_t আকার_2 = sectors * bl_len;

	buf = malloc_cache_aligned(আকার_2);
	যদি (!buf) {
		pr_err("Could not get FIT buffer of %lu bytes\n", (ulong)আকার_2);

		যদি (IS_ENABLED(CONFIG_SPL_SYS_MALLOC))
			pr_err("\tcheck CONFIG_SPL_SYS_MALLOC_SIZE\n");
		অন্যথা
			pr_err("\tcheck CONFIG_SPL_SYS_MALLOC_F_LEN\n");

		buf = spl_get_load_বাফার(0, আকার_2);
	}
	ফেরত buf;
}

/*
 * Weak default function to allow customizing SPL fit loading for load-only
 * use cases by allowing to skip the parsing/processing of the FIT contents
 * (so that this can be done separately in a more customized fashion)
 */
__weak bool spl_load_simple_fit_skip_processing(শূন্য)
{
	ফেরত false;
}

/*
 * Weak default function to allow fixes after fit header
 * is loaded.
 */
__weak শূন্য *spl_load_simple_fit_fix_load(ধ্রুবক শূন্য *fit)
{
	ফেরত (শূন্য *)fit;
}

স্থির শূন্য warn_deprecated(ধ্রুবক অক্ষর *বার্তা)
{
	printf("DEPRECATED: %s\n", বার্তা);
	printf("\tSee https://fitspec.osfw.foundation/\n");
}

স্থির পূর্ণসংখ্যা spl_fit_upload_fpga(কাঠামো spl_fit_info *ctx, পূর্ণসংখ্যা গ্রন্থি,
			       কাঠামো spl_image_info *fpga_image)
{
	ধ্রুবক অক্ষর *compatible;
	পূর্ণসংখ্যা ret;
	পূর্ণসংখ্যা devnum = 0;
	পূর্ণসংখ্যা flags = 0;

	debug("FPGA bitstream at: %x, size: %x\n",
	      (u32)fpga_image->load_addr, fpga_image->আকার_2);

	compatible = fdt_getprop(ctx->fit, গ্রন্থি, FIT_COMPAT_PROP, ফাঁকা);
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
			fpga_image->আকার_2, BIT_FULL, flags);
	যদি (ret) {
		printf("%s: Cannot load the image to the FPGA\n", __func__);
		ফেরত ret;
	}

	puts("FPGA image loaded from FIT\n");

	ফেরত 0;
}

স্থির পূর্ণসংখ্যা spl_fit_load_fpga(কাঠামো spl_fit_info *ctx,
			     কাঠামো spl_load_info *info, ulong offset)
{
	পূর্ণসংখ্যা গ্রন্থি, ret;

	কাঠামো spl_image_info fpga_image = {
		.load_addr = 0,
	};

	গ্রন্থি = spl_fit_get_image_গ্রন্থি(ctx, FIT_FPGA_PROP, 0);
	যদি (গ্রন্থি < 0)
		ফেরত গ্রন্থি;

	warn_deprecated("'fpga' property in config node. Use 'loadables'");

	/* Load the image and set up the fpga_image structure */
	ret = load_simple_fit(info, offset, ctx, গ্রন্থি, &fpga_image,
			      CONFIG_SYS_BOOTM_LEN);
	যদি (ret) {
		printf("%s: Cannot load the FPGA: %i\n", __func__, ret);
		ফেরত ret;
	}

	ফেরত spl_fit_upload_fpga(ctx, গ্রন্থি, &fpga_image);
}

স্থির পূর্ণসংখ্যা spl_simple_fit_read(কাঠামো spl_fit_info *ctx,
			       কাঠামো spl_load_info *info, ulong offset,
			       ধ্রুবক শূন্য *fit_header)
{
	unsigned long aligned_আকার, গুনতি, আকার_2;
	শূন্য *buf;
	পূর্ণসংখ্যা ret;

	/*
	 * For FIT with external data, figure out where the external images
	 * start. This is the base for the data-offset properties in each
	 * image.
	 */
	আকার_2 = ALIGN(fdt_totalsize(fit_header), 4);
	আকার_2 = board_spl_fit_আকার_align(আকার_2);
	ctx->ext_তথ্য_offset = ALIGN(আকার_2, 4);

	/*
	 * So far we only have one block of data from the FIT. Read the entire
	 * thing, including that first block.
	 *
	 * For FIT with data embedded, data is loaded as part of FIT image.
	 * For FIT with external data, data is not loaded in this step.
	 */
	ret = get_aligned_image_আকার(info, আকার_2, 0, &aligned_আকার);
	যদি (ret) {
		log_debug("Invalid FIT size: %lu\n", আকার_2);
		ফেরত ret;
	}
	buf = board_spl_fit_বাফার_addr(aligned_আকার, aligned_আকার, 1);
	যদি (!buf) {
		/*
		 * We assume that none of the board will ever use 0x0 as a
		 * valid load address. Theoretically some board could use it,
		 * but this is extremely unlikely.
		 */
		ফেরত -EIO;
	}

	গুনতি = info->পঠ(info, offset, aligned_আকার, buf);
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
		ফেরত -EIO;
	}

	ctx->fit = buf;
	debug("fit read offset %lx, size=%lu, dst=%p, count=%lu\n",
	      offset, aligned_আকার, buf, গুনতি);

	ফেরত 0;
}

স্থির পূর্ণসংখ্যা spl_simple_fit_parse(কাঠামো spl_fit_info *ctx)
{
	/* Find the correct subnode under "/configurations" */
	ctx->conf_গ্রন্থি = fit_find_config_গ্রন্থি(ctx->fit);
	যদি (ctx->conf_গ্রন্থি < 0)
		ফেরত -EINVAL;

	যদি (CONFIG_IS_ENABLED(FIT_SIGNATURE)) {
		printf("## Checking hash(es) for config %s ... ",
		       fit_get_name(ctx->fit, ctx->conf_গ্রন্থি, ফাঁকা));
		যদি (fit_config_verify(ctx->fit, ctx->conf_গ্রন্থি))
			ফেরত -EPERM;
		puts("OK\n");
	}

	/* find the node holding the images information */
	ctx->images_গ্রন্থি = fdt_path_offset(ctx->fit, FIT_IMAGES_PATH);
	যদি (ctx->images_গ্রন্থি < 0) {
		debug("%s: Cannot find /images node: %d\n", __func__,
		      ctx->images_গ্রন্থি);
		ফেরত -EINVAL;
	}

	ফেরত 0;
}

পূর্ণসংখ্যা spl_load_simple_fit(কাঠামো spl_image_info *spl_image,
			কাঠামো spl_load_info *info, ulong offset, শূন্য *fit)
{
	কাঠামো spl_image_info image_info;
	কাঠামো spl_fit_info ctx;
	পূর্ণসংখ্যা গ্রন্থি = -1;
	পূর্ণসংখ্যা ret;
	পূর্ণসংখ্যা সূচক = 0;
	পূর্ণসংখ্যা firmware_গ্রন্থি;

	ret = spl_simple_fit_read(&ctx, info, offset, fit);
	যদি (ret < 0)
		ফেরত ret;

	/* skip further processing if requested to enable load-only use cases */
	যদি (spl_load_simple_fit_skip_processing())
		ফেরত 0;

	ctx.fit = spl_load_simple_fit_fix_load(ctx.fit);

	ret = spl_simple_fit_parse(&ctx);
	যদি (ret < 0)
		ফেরত ret;

	যদি (IS_ENABLED(CONFIG_SPL_FPGA))
		spl_fit_load_fpga(&ctx, info, offset);

	/*
	 * Find the U-Boot image using the following search order:
	 *   - start at 'firmware' (e.g. an ARM Trusted Firmware)
	 *   - fall back 'kernel' (e.g. a Falcon-mode OS boot
	 *   - fall back to using the first 'loadables' entry
	 */
	যদি (গ্রন্থি < 0)
		গ্রন্থি = spl_fit_get_image_গ্রন্থি(&ctx, FIT_FIRMWARE_PROP, 0);

	যদি (গ্রন্থি < 0 && IS_ENABLED(CONFIG_SPL_OS_BOOT))
		গ্রন্থি = spl_fit_get_image_গ্রন্থি(&ctx, FIT_KERNEL_PROP, 0);

	যদি (গ্রন্থি < 0) {
		debug("could not find firmware image, trying loadables...\n");
		গ্রন্থি = spl_fit_get_image_গ্রন্থি(&ctx, FIT_LOADABLE_PROP, 0);
		/*
		 * If we pick the U-Boot image from "loadables", start at
		 * the second image when later loading additional images.
		 */
		সূচক = 1;
	}
	যদি (গ্রন্থি < 0) {
		debug("%s: Cannot find u-boot image node: %d\n",
		      __func__, গ্রন্থি);
		ফেরত -1;
	}

	/* Load the image and set up the spl_image structure */
	ret = load_simple_fit(info, offset, &ctx, গ্রন্থি, spl_image,
			      CONFIG_SYS_BOOTM_LEN);
	যদি (ret)
		ফেরত ret;

	/*
	 * For backward compatibility, we treat the first node that is
	 * as a U-Boot image, if no OS-type has been declared.
	 */
	যদি (!spl_fit_image_get_os(ctx.fit, গ্রন্থি, &spl_image->os))
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
			ফেরত ret;
	}

	firmware_গ্রন্থি = গ্রন্থি;
	/* Now check if there are more images for us to load */
	জন্য (; ; সূচক++) {
		uint8_t os_type = IH_OS_INVALID;

		গ্রন্থি = spl_fit_get_image_গ্রন্থি(&ctx, FIT_LOADABLE_PROP, সূচক);
		যদি (গ্রন্থি < 0)
			বিরতি;

		/*
		 * if the firmware is also a loadable, skip it because
		 * it already has been loaded. This is typically the case with
		 * u-boot.img generated by mkimage.
		 */
		যদি (firmware_গ্রন্থি == গ্রন্থি)
			চালু;

		image_info.load_addr = 0;
		ret = load_simple_fit(info, offset, &ctx, গ্রন্থি, &image_info,
				      CONFIG_SYS_BOOTM_LEN);
		যদি (ret < 0 && ret != -EBADSLT) {
			printf("%s: can't load image loadables index %d (ret = %d)\n",
			       __func__, সূচক, ret);
			ফেরত ret;
		}

		যদি (spl_fit_image_is_fpga(ctx.fit, গ্রন্থি))
			spl_fit_upload_fpga(&ctx, গ্রন্থি, &image_info);

		যদি (!spl_fit_image_get_os(ctx.fit, গ্রন্থি, &os_type))
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

		spl_fit_image_record_arm32_optee(ctx.fit, গ্রন্থি, spl_image,
						 &image_info);

		/* Record our loadables into the FDT */
		যদি (!CONFIG_IS_ENABLED(FIT_IMAGE_TINY) &&
		    xpl_get_fdt_update(info) && spl_image->fdt_addr)
			spl_fit_record_loadable(&ctx, সূচক,
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
	upl_set_fit_info(map_to_sysmem(ctx.fit), ctx.conf_গ্রন্থি,
			 spl_image->entry_point);

	ফেরত 0;
}

/* Parse and load full fitImage in SPL */
পূর্ণসংখ্যা spl_load_fit_image(কাঠামো spl_image_info *spl_image,
		       ধ্রুবক কাঠামো legacy_img_hdr *header)
{
	কাঠামো bootm_headers images;
	ধ্রুবক অক্ষর *fit_uname_config = ফাঁকা;
	ulong fdt_hack;
	ধ্রুবক অক্ষর *uname;
	ulong fw_তথ্য = 0, dt_তথ্য = 0, img_তথ্য = 0;
	ulong fw_len = 0, dt_len = 0, img_len = 0;
	পূর্ণসংখ্যা idx, conf_noffset;
	পূর্ণসংখ্যা ret;

	images.verify = CONFIG_IS_ENABLED(FIT_SIGNATURE);

	ret = fit_image_load(&images, virt_to_phys((শূন্য *)header),
			     ফাঁকা, &fit_uname_config, IH_ARCH_DEFAULT,
			     IH_TYPE_FIRMWARE, -1, FIT_LOAD_OPTIONAL,
			     &fw_তথ্য, &fw_len);

	যদি (ret < 0) {
		ret = fit_image_load(&images, virt_to_phys((শূন্য *)header),
				     ফাঁকা, &fit_uname_config, IH_ARCH_DEFAULT,
				     IH_TYPE_KERNEL, -1, FIT_LOAD_OPTIONAL,
				     &fw_তথ্য, &fw_len);
	}

	যদি (ret < 0)
		ফেরত ret;

	spl_image->আকার_2 = fw_len;
	spl_image->load_addr = fw_তথ্য;
	যদি (fit_image_get_entry(header, ret, &spl_image->entry_point))
		spl_image->entry_point = fw_তথ্য;
	যদি (fit_image_get_os(header, ret, &spl_image->os))
		spl_image->os = IH_OS_INVALID;
	spl_image->name = genimg_get_os_name(spl_image->os);

	debug(PHASE_PROMPT "payload image: %32s load addr: 0x%lx size: %d\n",
	      spl_image->name, spl_image->load_addr, spl_image->আকার_2);

	images.verify = CONFIG_IS_ENABLED(FIT_SIGNATURE);

	ret = fit_image_load(&images, virt_to_phys((শূন্য *)header), ফাঁকা,
			     &fit_uname_config, IH_ARCH_DEFAULT, IH_TYPE_FLATDT,
			     -1, FIT_LOAD_OPTIONAL, &dt_তথ্য, &dt_len);
	যদি (ret >= 0) {
		যদি (spl_image->os == IH_OS_U_BOOT) {
			/* HACK: U-Boot expects FDT at a specific address */
			fdt_hack = ALIGN(spl_image->load_addr + spl_image->আকার_2, 8);
			debug("Relocating FDT to %p\n", (শূন্য *)fdt_hack);
			memcpy(map_sysmem(fdt_hack, dt_len),
			       map_sysmem(dt_তথ্য, 0), dt_len);
			spl_image->fdt_addr = (শূন্য *)fdt_hack;
		} অন্যথা {
			spl_image->fdt_addr = (শূন্য *)dt_তথ্য;
		}
	}

	conf_noffset = fit_conf_get_গ্রন্থি((ধ্রুবক শূন্য *)header,
					 fit_uname_config);
	যদি (conf_noffset < 0)
		ফেরত 0;

	জন্য (idx = 0;
	     uname = fdt_stringlist_get((ধ্রুবক শূন্য *)header, conf_noffset,
					FIT_LOADABLE_PROP, idx,
				ফাঁকা), uname;
	     idx++) {
		images.verify = CONFIG_IS_ENABLED(FIT_SIGNATURE);

		ret = fit_image_load(&images, virt_to_phys((শূন্য *)header),
				     &uname, &fit_uname_config,
				     IH_ARCH_DEFAULT, IH_TYPE_LOADABLE, -1,
				     FIT_LOAD_OPTIONAL_NON_ZERO,
				     &img_তথ্য, &img_len);
		যদি (ret < 0)
			ফেরত ret;
	}
	spl_image->flags |= SPL_FIT_FOUND;

	upl_set_fit_info(map_to_sysmem(header), conf_noffset,
			 spl_image->entry_point);

	ফেরত 0;
}
