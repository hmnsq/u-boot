// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <fpga.h>
#അന്തര്ഭാവ <gzip.h>
#അന്തര്ഭാവ <image.h>
#അന്തര്ഭാവ <log.h>
#അന്തര്ഭാവ <memalign.h>
#അന്തര്ഭാവ <mapmem.h>
#അന്തര്ഭാവ <spl.h>
#അന്തര്ഭാവ <upl.h>
#അന്തര്ഭാവ <sysinfo.h>
#അന്തര്ഭാവ <asm/global_data.h>
#അന്തര്ഭാവ <asm/io.h>
#അന്തര്ഭാവ <linux/libfdt.h>
#അന്തര്ഭാവ <linux/printk.h>

DECLARE_GLOBAL_DATA_PTR;

ഘടന spl_fit_info {
	സ്ഥിരം ശൂന്യം *fit;	/* Pointer to a valid FIT blob */
	size_t ext_data_offset;	/* Offset to FIT external data (end of FIT) */
	പൂർണ്ണസംഖ്യ images_കണ്ണി;	/* FDT offset to "/images" node */
	പൂർണ്ണസംഖ്യ conf_കണ്ണി;		/* FDT offset to selected configuration node */
};

__weak ulong board_spl_fit_size_align(ulong size)
{
	തിരികെ size;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ find_കണ്ണി_from_desc(സ്ഥിരം ശൂന്യം *fit, പൂർണ്ണസംഖ്യ കണ്ണി, സ്ഥിരം അക്ഷരം *str)
{
	പൂർണ്ണസംഖ്യ child;

	എങ്കിൽ (കണ്ണി < 0)
		തിരികെ -EINVAL;

	/* iterate the FIT nodes and find a matching description */
	വേണ്ടി (child = fdt_first_subnode(fit, കണ്ണി); child >= 0;
	     child = fdt_next_subnode(fit, child)) {
		പൂർണ്ണസംഖ്യ len;
		സ്ഥിരം അക്ഷരം *desc = fdt_getprop(fit, child, FIT_DESC_PROP, &len);

		എങ്കിൽ (!desc)
			തുടരുക;

		എങ്കിൽ (!strcmp(desc, str))
			തിരികെ child;
	}

	തിരികെ -ENOENT;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ spl_fit_get_image_name(സ്ഥിരം ഘടന spl_fit_info *ctx,
				  സ്ഥിരം അക്ഷരം *type, പൂർണ്ണസംഖ്യ index,
				  സ്ഥിരം അക്ഷരം **outname)
{
	ഘടന udevice *sysinfo;
	സ്ഥിരം അക്ഷരം *name, *str, *end;
	__maybe_unused പൂർണ്ണസംഖ്യ കണ്ണി;
	പൂർണ്ണസംഖ്യ len, i;
	bool found = true;

	name = fdt_getprop(ctx->fit, ctx->conf_കണ്ണി, type, &len);
	എങ്കിൽ (!name) {
		debug("cannot find property '%s': %d\n", type, len);
		തിരികെ -EINVAL;
	}
	/* A string property should be NUL terminated */
	end = name + len - 1;
	എങ്കിൽ (!len || *end) {
		debug("malformed property '%s'\n", type);
		തിരികെ -EINVAL;
	}

	str = name;
	വേണ്ടി (i = 0; i < index; i++) {
		str = strchr(str, '\0') + 1;
		എങ്കിൽ (str > end) {
			found = false;
			നിർത്തുക;
		}
	}

	എങ്കിൽ (!found && CONFIG_IS_ENABLED(SYSINFO) && !sysinfo_get(&sysinfo)) {
		പൂർണ്ണസംഖ്യ rc;
		/*
		 * no string in the property for this index. Check if the
		 * sysinfo-level code can supply one.
		 */
		rc = sysinfo_detect(sysinfo);
		എങ്കിൽ (rc)
			തിരികെ rc;

		rc = sysinfo_get_fit_loadable(sysinfo, index - i - 1, type,
					      &str);
		എങ്കിൽ (rc && rc != -ENOENT)
			തിരികെ rc;

		എങ്കിൽ (!rc) {
			/*
			 * The sysinfo provided a name for a loadable.
			 * Try to match it against the description properties
			 * first. If no matching node is found, use it as a
			 * node name.
			 */
			പൂർണ്ണസംഖ്യ കണ്ണി;
			പൂർണ്ണസംഖ്യ images = fdt_path_offset(ctx->fit, FIT_IMAGES_PATH);

			കണ്ണി = find_കണ്ണി_from_desc(ctx->fit, images, str);
			എങ്കിൽ (കണ്ണി > 0)
				str = fdt_get_name(ctx->fit, കണ്ണി, ശൂന്യ);

			found = true;
		}
	}

	എങ്കിൽ (!found) {
		debug("no string for index %d\n", index);
		തിരികെ -E2BIG;
	}

	*outname = str;
	തിരികെ 0;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ spl_fit_get_image_കണ്ണി(സ്ഥിരം ഘടന spl_fit_info *ctx,
				  സ്ഥിരം അക്ഷരം *type, പൂർണ്ണസംഖ്യ index)
{
	സ്ഥിരം അക്ഷരം *str;
	പൂർണ്ണസംഖ്യ err;
	പൂർണ്ണസംഖ്യ കണ്ണി;

	err = spl_fit_get_image_name(ctx, type, index, &str);
	എങ്കിൽ (err)
		തിരികെ err;

	debug("%s: '%s'\n", type, str);

	കണ്ണി = fdt_subnode_offset(ctx->fit, ctx->images_കണ്ണി, str);
	എങ്കിൽ (കണ്ണി < 0) {
		pr_err("cannot find image node '%s': %d\n", str, കണ്ണി);
		തിരികെ -EINVAL;
	}

	തിരികെ കണ്ണി;
}

സ്ഥിതം u32 get_aligned_image_offset(ഘടന spl_load_info *info, u32 offset)
{
	തിരികെ ALIGN_DOWN(offset, spl_get_bl_len(info));
}

സ്ഥിതം u32 get_aligned_image_overhead(ഘടന spl_load_info *info, u32 offset)
{
	തിരികെ offset & (spl_get_bl_len(info) - 1);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ get_aligned_image_size(ഘടന spl_load_info *info, ulong data_size,
				  u32 offset, ulong *aligned_size)
{
	u32 overhead = get_aligned_image_overhead(info, offset);

	എങ്കിൽ (data_size > ULONG_MAX - overhead)
		തിരികെ -EOVERFLOW;
	data_size += overhead;

	എങ്കിൽ (data_size > ULONG_MAX - (spl_get_bl_len(info) - 1))
		തിരികെ -EOVERFLOW;
	data_size = ALIGN(data_size, spl_get_bl_len(info));

	*aligned_size = data_size;

	തിരികെ 0;
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
സ്ഥിതം ശൂന്യം fit_fill_image_info(സ്ഥിരം ശൂന്യം *fit, പൂർണ്ണസംഖ്യ കണ്ണി,
				ഘടന spl_image_info *image_info,
				ulong load_addr, ulong size)
{
	ulong entry_point;

	എങ്കിൽ (!image_info)
		തിരികെ;

	image_info->load_addr = load_addr;
	image_info->size = size;

	എങ്കിൽ (!fit_image_get_entry(fit, കണ്ണി, &entry_point))
		image_info->entry_point = entry_point;
	അല്ലെങ്കിൽ
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
സ്ഥിതം പൂർണ്ണസംഖ്യ load_simple_fit(ഘടന spl_load_info *info, ulong fit_offset,
			   സ്ഥിരം ഘടന spl_fit_info *ctx, പൂർണ്ണസംഖ്യ കണ്ണി,
			   ഘടന spl_image_info *image_info, ulong max_size)
{
	u32 offset;
	u32 len;
	size_t length;
	ulong size;
	ulong load_addr;
	ശൂന്യം *load_ptr;
	ശൂന്യം *src;
	uint8_t image_comp = -1, type = -1;
	സ്ഥിരം ശൂന്യം *data;
	സ്ഥിരം ശൂന്യം *fit = ctx->fit;
	bool external_data = false;
	പൂർണ്ണസംഖ്യ ret;

	log_debug("starting\n");
	എങ്കിൽ (CONFIG_IS_ENABLED(BOOTMETH_VBE) &&
	    xpl_get_phase(info) != IH_PHASE_NONE) {
		എണ്ണൽ image_phase_t phase;

		ret = fit_image_get_phase(fit, കണ്ണി, &phase);
		/* if the image is for any phase, let's use it */
		എങ്കിൽ (ret == -ENOENT || phase == xpl_get_phase(info)) {
			log_debug("found\n");
		} അല്ലെങ്കിൽ എങ്കിൽ (ret < 0) {
			log_debug("err=%d\n", ret);
			തിരികെ ret;
		} അല്ലെങ്കിൽ {
			log_debug("- phase mismatch, skipping this image\n");
			തിരികെ -EBADSLT;
		}
	}

	എങ്കിൽ (IS_ENABLED(CONFIG_SPL_FPGA) ||
	    (IS_ENABLED(CONFIG_SPL_OS_BOOT) && spl_decompression_enabled())) {
		എങ്കിൽ (fit_image_get_type(fit, കണ്ണി, &type))
			puts("Cannot get image type.\n");
		അല്ലെങ്കിൽ
			debug("%s ", genimg_get_type_name(type));
	}

	എങ്കിൽ (spl_decompression_enabled()) {
		fit_image_get_comp(fit, കണ്ണി, &image_comp);
		debug("%s ", genimg_get_comp_name(image_comp));
	}

	എങ്കിൽ (fit_image_get_load(fit, കണ്ണി, &load_addr)) {
		എങ്കിൽ (!image_info->load_addr) {
			printf("Can't load %s: No load address and no buffer\n",
			       fit_get_name(fit, കണ്ണി, ശൂന്യ));
			തിരികെ -ENOBUFS;
		}
		load_addr = image_info->load_addr;
	}

	എങ്കിൽ (!fit_image_get_data_position(fit, കണ്ണി, &offset)) {
		external_data = true;
	} അല്ലെങ്കിൽ എങ്കിൽ (!fit_image_get_data_offset(fit, കണ്ണി, &offset)) {
		/* The resulting offset cannot exceed UINT32_MAX */
		എങ്കിൽ (ctx->ext_data_offset > UINT32_MAX - offset) {
			log_debug("Invalid external data offset: %u\n", offset);
			തിരികെ -EINVAL;
		}
		log_debug("read offset %x = offset from fit %x\n", offset,
			  (u32)(offset + ctx->ext_data_offset));
		offset += ctx->ext_data_offset;
		external_data = true;
	}

	എങ്കിൽ (external_data) {
		u32 aligned_offset;
		ulong read_offset;
		ശൂന്യം *src_ptr;

		/* External data */
		എങ്കിൽ (fit_image_get_data_size(fit, കണ്ണി, &len))
			തിരികെ -ENOENT;

		/* Dont bother to copy 0 byte data, but warn, though */
		എങ്കിൽ (!len) {
			log_warning("%s: Skip load '%s': image size is 0!\n",
				    __func__, fit_get_name(fit, കണ്ണി, ശൂന്യ));
			fit_fill_image_info(fit, കണ്ണി, image_info, load_addr, 0);
			തിരികെ 0;
		}

		/*
		 * data-size is excluded from the configuration signature (it
		 * is in exc_prop[] in image-fit-sig.c), so it stays attacker
		 * controlled even after fit_config_verify() succeeds. The
		 * image hash is only verified after the device read below, so
		 * an oversized value has to be rejected here.
		 */
		ret = get_aligned_image_size(info, len, offset, &size);
		എങ്കിൽ (ret) {
			log_debug("Invalid external data size: %u\n", len);
			തിരികെ ret;
		}

		എങ്കിൽ (size > max_size) {
			log_debug("Image too large: aligned size %lu, max %lu (data-size %u)\n",
				  size, max_size, len);
			തിരികെ -EFBIG;
		}

		aligned_offset = get_aligned_image_offset(info, offset);
		എങ്കിൽ (aligned_offset > ULONG_MAX - fit_offset) {
			log_debug("Invalid aligned external data offset: %u\n",
				  aligned_offset);
			തിരികെ -EINVAL;
		}
		read_offset = fit_offset + aligned_offset;

		എങ്കിൽ (spl_decompression_enabled() &&
		    (image_comp == IH_COMP_GZIP || image_comp == IH_COMP_LZMA))
			src_ptr = map_sysmem(ALIGN(CONFIG_SYS_LOAD_ADDR, ARCH_DMA_MINALIGN), len);
		അല്ലെങ്കിൽ
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
		എങ്കിൽ (size > ULONG_MAX - read_offset ||
		    size > UINTPTR_MAX - (uintptr_t)src_ptr
		/*
		 * #if (not a runtime if) is required: FIT_SIGNATURE_MAX_SIZE
		 * depends on FIT_SIGNATURE, so CONFIG_VAL(FIT_SIGNATURE_MAX_SIZE)
		 * is undefined when signing is disabled and referencing it
		 * here would fail to compile.
		 */
#എങ്കിൽ CONFIG_IS_ENABLED(FIT_SIGNATURE)
		    || offset > CONFIG_VAL(FIT_SIGNATURE_MAX_SIZE) ||
		    len > CONFIG_VAL(FIT_SIGNATURE_MAX_SIZE) - offset
#endif
		) {
			log_debug("FIT external data is out of bounds (offset=%u, size=%u)\n",
				  offset, len);
			തിരികെ -EINVAL;
		}

		log_debug("reading from offset %x / %lx size %lx to %p: ",
			  offset, read_offset, size, src_ptr);

		എങ്കിൽ (info->പഠ(info, read_offset, size, src_ptr) < length)
			തിരികെ -EIO;

		debug("External data: dst=%p, offset=%x, size=%lx\n",
		      src_ptr, offset, (unsigned long)length);
		src = src_ptr + get_aligned_image_overhead(info, offset);
	} അല്ലെങ്കിൽ {
		/* Embedded data */
		എങ്കിൽ (fit_image_get_emb_data(fit, കണ്ണി, &data, &length)) {
			puts("Cannot get image data/size\n");
			തിരികെ -ENOENT;
		}
		debug("Embedded data: dst=%lx, size=%lx\n", load_addr,
		      (unsigned long)length);
		src = (ശൂന്യം *)data;	/* cast away const */
	}

	എങ്കിൽ (CONFIG_IS_ENABLED(FIT_SIGNATURE)) {
		printf("## Checking hash(es) for Image %s ... ",
		       fit_get_name(fit, കണ്ണി, ശൂന്യ));
		എങ്കിൽ (!fit_image_verify_with_data(fit, കണ്ണി, gd_fdt_blob(), src,
						length))
			തിരികെ -EPERM;
		puts("OK\n");
	}

	എങ്കിൽ (CONFIG_IS_ENABLED(FIT_IMAGE_POST_PROCESS))
		board_fit_image_post_process(fit, കണ്ണി, &src, &length);

	load_ptr = map_sysmem(load_addr, length);
	എങ്കിൽ (IS_ENABLED(CONFIG_SPL_GZIP) && image_comp == IH_COMP_GZIP) {
		size = length;
		എങ്കിൽ (gunzip(load_ptr, CONFIG_SYS_BOOTM_LEN, src, &size)) {
			puts("Uncompressing error\n");
			തിരികെ -EIO;
		}
		length = size;
	} അല്ലെങ്കിൽ എങ്കിൽ (IS_ENABLED(CONFIG_SPL_LZMA) && image_comp == IH_COMP_LZMA) {
		size = CONFIG_SYS_BOOTM_LEN;
		ulong loadEnd;

		എങ്കിൽ (image_decomp(IH_COMP_LZMA, CONFIG_SYS_LOAD_ADDR, 0, 0,
				 load_ptr, src, length, size, &loadEnd)) {
			puts("Uncompressing error\n");
			തിരികെ -EIO;
		}
		length = loadEnd - CONFIG_SYS_LOAD_ADDR;
	} അല്ലെങ്കിൽ {
		memmove(load_ptr, src, length);
	}

	fit_fill_image_info(fit, കണ്ണി, image_info, load_addr, length);
	log_debug("- done loading\n");

	upl_add_image(fit, കണ്ണി, load_addr, length);

	തിരികെ 0;
}

സ്ഥിതം bool os_takes_devicetree(uint8_t os)
{
	തിരഞ്ഞെടുപ്പ് (os) {
	അവസ്ഥ IH_OS_U_BOOT:
		തിരികെ true;
	അവസ്ഥ IH_OS_LINUX:
		തിരികെ IS_ENABLED(CONFIG_SPL_OS_BOOT) ||
		       IS_ENABLED(CONFIG_SPL_OPENSBI);
	സ്ഥിരസ്ഥിതി:
		തിരികെ false;
	}
}

__weak പൂർണ്ണസംഖ്യ board_spl_fit_append_fdt_skip(സ്ഥിരം അക്ഷരം *name)
{
	തിരികെ 0;	/* Do not skip */
}

സ്ഥിതം പൂർണ്ണസംഖ്യ spl_fit_append_fdt(ഘടന spl_image_info *spl_image,
			      ഘടന spl_load_info *info, ulong offset,
			      സ്ഥിരം ഘടന spl_fit_info *ctx)
{
	ഘടന spl_image_info image_info;
	പൂർണ്ണസംഖ്യ കണ്ണി, ret = 0, index = 0;

	/*
	 * Use the address following the image as target address for the
	 * device tree.
	 */
	image_info.load_addr = ALIGN(spl_image->load_addr + spl_image->size, 8);

	/* Figure out which device tree the board wants to use */
	കണ്ണി = spl_fit_get_image_കണ്ണി(ctx, FIT_FDT_PROP, index++);
	എങ്കിൽ (കണ്ണി < 0) {
		size_t size;

		debug("%s: cannot find FDT node\n", __func__);

		/*
		 * U-Boot did not find a device tree inside the FIT image. Use
		 * the U-Boot device tree instead.
		 */
		എങ്കിൽ (!gd->fdt_blob)
			തിരികെ കണ്ണി;

		/*
		 * Make the load-address of the FDT available for the SPL
		 * framework
		 */
		size = fdt_totalsize(gd->fdt_blob);
		spl_image->fdt_addr = map_sysmem(image_info.load_addr, size);
		memcpy(spl_image->fdt_addr, gd->fdt_blob, size);
	} അല്ലെങ്കിൽ {
		ret = load_simple_fit(info, offset, ctx, കണ്ണി, &image_info,
				      CONFIG_SYS_BOOTM_LEN);
		എങ്കിൽ (ret < 0)
			തിരികെ ret;

		spl_image->fdt_addr = phys_to_virt(image_info.load_addr);
	}

	എങ്കിൽ (CONFIG_IS_ENABLED(FIT_IMAGE_TINY))
		തിരികെ 0;

#എങ്കിൽ CONFIG_IS_ENABLED(LOAD_FIT_APPLY_OVERLAY)
		ശൂന്യം *tmpbuffer = ശൂന്യ;

		വേണ്ടി (; ; index++) {
			സ്ഥിരം അക്ഷരം *str;

			ret = spl_fit_get_image_name(ctx, FIT_FDT_PROP, index, &str);
			എങ്കിൽ (ret == -E2BIG) {
				debug("%s: No additional FDT node\n", __func__);
				ret = 0;
				നിർത്തുക;
			} അല്ലെങ്കിൽ എങ്കിൽ (ret) {
				നിർത്തുക;
			}

			ret = board_spl_fit_append_fdt_skip(str);
			എങ്കിൽ (ret)
				തുടരുക;

			കണ്ണി = fdt_subnode_offset(ctx->fit, ctx->images_കണ്ണി, str);
			എങ്കിൽ (കണ്ണി < 0) {
				debug("%s: unable to find FDT node %d\n",
				      __func__, index);
				തുടരുക;
			}

			എങ്കിൽ (!tmpbuffer) {
				/*
				 * allocate memory to store the DT overlay
				 * before it is applied. It may not be used
				 * depending on how the overlay is stored, so
				 * don't fail yet if the allocation failed.
				 */
				size_t size = CONFIG_SPL_LOAD_FIT_APPLY_OVERLAY_BUF_SZ;

				tmpbuffer = malloc_cache_aligned(size);
				എങ്കിൽ (!tmpbuffer)
					debug("%s: unable to allocate space for overlays\n",
					      __func__);
			}
			image_info.load_addr = (ulong)tmpbuffer;
			ret = load_simple_fit(info, offset, ctx, കണ്ണി,
					      &image_info,
					      CONFIG_SPL_LOAD_FIT_APPLY_OVERLAY_BUF_SZ);
			എങ്കിൽ (ret == -EBADSLT)
				തുടരുക;
			അല്ലെങ്കിൽ എങ്കിൽ (ret < 0)
				നിർത്തുക;

			/* Make room in FDT for changes from the overlay */
			ret = fdt_increase_size(spl_image->fdt_addr,
						image_info.size);
			എങ്കിൽ (ret < 0)
				നിർത്തുക;

			ret = fdt_overlay_apply_verbose(spl_image->fdt_addr,
							(ശൂന്യം *)image_info.load_addr);
			എങ്കിൽ (ret) {
				pr_err("failed to apply DT overlay %s\n",
				       fit_get_name(ctx->fit, കണ്ണി, ശൂന്യ));
				നിർത്തുക;
			}

			debug("%s: DT overlay %s applied\n", __func__,
			      fit_get_name(ctx->fit, കണ്ണി, ശൂന്യ));
		}
		സ്വതന്ത്രം(tmpbuffer);
		എങ്കിൽ (ret)
			തിരികെ ret;
#endif
	/* Try to make space, so we can inject details on the loadables */
	ret = fdt_shrink_to_minimum(spl_image->fdt_addr, 8192);
	എങ്കിൽ (ret < 0)
		തിരികെ ret;

	തിരികെ ret;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ spl_fit_record_loadable(സ്ഥിരം ഘടന spl_fit_info *ctx, പൂർണ്ണസംഖ്യ index,
				   ശൂന്യം *blob, ഘടന spl_image_info *image)
{
	പൂർണ്ണസംഖ്യ ret = 0;
	സ്ഥിരം അക്ഷരം *name;
	പൂർണ്ണസംഖ്യ കണ്ണി;

	ret = spl_fit_get_image_name(ctx, FIT_LOADABLE_PROP, index, &name);
	എങ്കിൽ (ret < 0)
		തിരികെ ret;

	കണ്ണി = spl_fit_get_image_കണ്ണി(ctx, FIT_LOADABLE_PROP, index);

	ret = fdt_record_loadable(blob, index, name, image->load_addr,
			image->size, image->entry_point,
			fdt_getprop(ctx->fit, കണ്ണി, FIT_TYPE_PROP, ശൂന്യ),
			fdt_getprop(ctx->fit, കണ്ണി, FIT_OS_PROP, ശൂന്യ),
			fdt_getprop(ctx->fit, കണ്ണി, FIT_ARCH_PROP, ശൂന്യ));

	തിരികെ ret;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ spl_fit_image_is_fpga(സ്ഥിരം ശൂന്യം *fit, പൂർണ്ണസംഖ്യ കണ്ണി)
{
	സ്ഥിരം അക്ഷരം *type;

	എങ്കിൽ (!IS_ENABLED(CONFIG_SPL_FPGA))
		തിരികെ 0;

	type = fdt_getprop(fit, കണ്ണി, FIT_TYPE_PROP, ശൂന്യ);
	എങ്കിൽ (!type)
		തിരികെ 0;

	തിരികെ !strcmp(type, "fpga");
}

സ്ഥിതം ശൂന്യം spl_fit_image_record_arm32_optee(സ്ഥിരം ശൂന്യം *fit, പൂർണ്ണസംഖ്യ കണ്ണി,
					     ഘടന spl_image_info *spl_image,
					     ഘടന spl_image_info *image_info)
{
#എങ്കിൽ defined(CONFIG_BOOTM_OPTEE) && defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
	സ്ഥിരം അക്ഷരം *type = fdt_getprop(fit, കണ്ണി, FIT_TYPE_PROP, ശൂന്യ);

	എങ്കിൽ (!type)
		തിരികെ;

	എങ്കിൽ (strcmp(type, "tee"))
		തിരികെ;

	spl_image->optee_addr = image_info->load_addr;
#endif
}

സ്ഥിതം പൂർണ്ണസംഖ്യ spl_fit_image_get_os(സ്ഥിരം ശൂന്യം *fit, പൂർണ്ണസംഖ്യ noffset, uint8_t *os)
{
	എങ്കിൽ (!CONFIG_IS_ENABLED(FIT_IMAGE_TINY) || CONFIG_IS_ENABLED(OS_BOOT))
		തിരികെ fit_image_get_os(fit, noffset, os);

	സ്ഥിരം അക്ഷരം *name = fdt_getprop(fit, noffset, FIT_OS_PROP, ശൂന്യ);
	എങ്കിൽ (!name)
		തിരികെ -ENOENT;

	/*
	 * We don't care what the type of the image actually is,
	 * only whether or not it is U-Boot. This saves some
	 * space by omitting the large table of OS types.
	 */
	എങ്കിൽ (!strcmp(name, "u-boot"))
		*os = IH_OS_U_BOOT;
	അല്ലെങ്കിൽ
		*os = IH_OS_INVALID;

	തിരികെ 0;
}

/*
 * The purpose of the FIT load buffer is to provide a memory location that is
 * independent of the load address of any FIT component.
 */
__weak ശൂന്യം *board_spl_fit_buffer_addr(ulong fit_size, പൂർണ്ണസംഖ്യ sectors, പൂർണ്ണസംഖ്യ bl_len)
{
	ശൂന്യം *buf;
	size_t size = sectors * bl_len;

	buf = malloc_cache_aligned(size);
	എങ്കിൽ (!buf) {
		pr_err("Could not get FIT buffer of %lu bytes\n", (ulong)size);

		എങ്കിൽ (IS_ENABLED(CONFIG_SPL_SYS_MALLOC))
			pr_err("\tcheck CONFIG_SPL_SYS_MALLOC_SIZE\n");
		അല്ലെങ്കിൽ
			pr_err("\tcheck CONFIG_SPL_SYS_MALLOC_F_LEN\n");

		buf = spl_get_load_buffer(0, size);
	}
	തിരികെ buf;
}

/*
 * Weak default function to allow customizing SPL fit loading for load-only
 * use cases by allowing to skip the parsing/processing of the FIT contents
 * (so that this can be done separately in a more customized fashion)
 */
__weak bool spl_load_simple_fit_skip_processing(ശൂന്യം)
{
	തിരികെ false;
}

/*
 * Weak default function to allow fixes after fit header
 * is loaded.
 */
__weak ശൂന്യം *spl_load_simple_fit_fix_load(സ്ഥിരം ശൂന്യം *fit)
{
	തിരികെ (ശൂന്യം *)fit;
}

സ്ഥിതം ശൂന്യം warn_deprecated(സ്ഥിരം അക്ഷരം *സന്ദേശം)
{
	printf("DEPRECATED: %s\n", സന്ദേശം);
	printf("\tSee https://fitspec.osfw.foundation/\n");
}

സ്ഥിതം പൂർണ്ണസംഖ്യ spl_fit_upload_fpga(ഘടന spl_fit_info *ctx, പൂർണ്ണസംഖ്യ കണ്ണി,
			       ഘടന spl_image_info *fpga_image)
{
	സ്ഥിരം അക്ഷരം *compatible;
	പൂർണ്ണസംഖ്യ ret;
	പൂർണ്ണസംഖ്യ devnum = 0;
	പൂർണ്ണസംഖ്യ flags = 0;

	debug("FPGA bitstream at: %x, size: %x\n",
	      (u32)fpga_image->load_addr, fpga_image->size);

	compatible = fdt_getprop(ctx->fit, കണ്ണി, FIT_COMPAT_PROP, ശൂന്യ);
	എങ്കിൽ (!compatible) {
		warn_deprecated("'fpga' image without 'compatible' property");
	} അല്ലെങ്കിൽ {
		എങ്കിൽ (CONFIG_IS_ENABLED(FPGA_LOAD_SECURE))
			flags = fpga_compatible2flag(devnum, compatible);
		എങ്കിൽ (strcmp(compatible, "u-boot,fpga-legacy"))
			debug("Ignoring compatible = %s property\n",
			      compatible);
	}

	ret = fpga_load(devnum, (ശൂന്യം *)fpga_image->load_addr,
			fpga_image->size, BIT_FULL, flags);
	എങ്കിൽ (ret) {
		printf("%s: Cannot load the image to the FPGA\n", __func__);
		തിരികെ ret;
	}

	puts("FPGA image loaded from FIT\n");

	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ spl_fit_load_fpga(ഘടന spl_fit_info *ctx,
			     ഘടന spl_load_info *info, ulong offset)
{
	പൂർണ്ണസംഖ്യ കണ്ണി, ret;

	ഘടന spl_image_info fpga_image = {
		.load_addr = 0,
	};

	കണ്ണി = spl_fit_get_image_കണ്ണി(ctx, FIT_FPGA_PROP, 0);
	എങ്കിൽ (കണ്ണി < 0)
		തിരികെ കണ്ണി;

	warn_deprecated("'fpga' property in config node. Use 'loadables'");

	/* Load the image and set up the fpga_image structure */
	ret = load_simple_fit(info, offset, ctx, കണ്ണി, &fpga_image,
			      CONFIG_SYS_BOOTM_LEN);
	എങ്കിൽ (ret) {
		printf("%s: Cannot load the FPGA: %i\n", __func__, ret);
		തിരികെ ret;
	}

	തിരികെ spl_fit_upload_fpga(ctx, കണ്ണി, &fpga_image);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ spl_simple_fit_read(ഘടന spl_fit_info *ctx,
			       ഘടന spl_load_info *info, ulong offset,
			       സ്ഥിരം ശൂന്യം *fit_header)
{
	unsigned long aligned_size, എണ്ണം, size;
	ശൂന്യം *buf;
	പൂർണ്ണസംഖ്യ ret;

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
	എങ്കിൽ (ret) {
		log_debug("Invalid FIT size: %lu\n", size);
		തിരികെ ret;
	}
	buf = board_spl_fit_buffer_addr(aligned_size, aligned_size, 1);
	എങ്കിൽ (!buf) {
		/*
		 * We assume that none of the board will ever use 0x0 as a
		 * valid load address. Theoretically some board could use it,
		 * but this is extremely unlikely.
		 */
		തിരികെ -EIO;
	}

	എണ്ണം = info->പഠ(info, offset, aligned_size, buf);
	എങ്കിൽ (!എണ്ണം) {
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
		തിരികെ -EIO;
	}

	ctx->fit = buf;
	debug("fit read offset %lx, size=%lu, dst=%p, count=%lu\n",
	      offset, aligned_size, buf, എണ്ണം);

	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ spl_simple_fit_parse(ഘടന spl_fit_info *ctx)
{
	/* Find the correct subnode under "/configurations" */
	ctx->conf_കണ്ണി = fit_find_config_കണ്ണി(ctx->fit);
	എങ്കിൽ (ctx->conf_കണ്ണി < 0)
		തിരികെ -EINVAL;

	എങ്കിൽ (CONFIG_IS_ENABLED(FIT_SIGNATURE)) {
		printf("## Checking hash(es) for config %s ... ",
		       fit_get_name(ctx->fit, ctx->conf_കണ്ണി, ശൂന്യ));
		എങ്കിൽ (fit_config_verify(ctx->fit, ctx->conf_കണ്ണി))
			തിരികെ -EPERM;
		puts("OK\n");
	}

	/* find the node holding the images information */
	ctx->images_കണ്ണി = fdt_path_offset(ctx->fit, FIT_IMAGES_PATH);
	എങ്കിൽ (ctx->images_കണ്ണി < 0) {
		debug("%s: Cannot find /images node: %d\n", __func__,
		      ctx->images_കണ്ണി);
		തിരികെ -EINVAL;
	}

	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ spl_load_simple_fit(ഘടന spl_image_info *spl_image,
			ഘടന spl_load_info *info, ulong offset, ശൂന്യം *fit)
{
	ഘടന spl_image_info image_info;
	ഘടന spl_fit_info ctx;
	പൂർണ്ണസംഖ്യ കണ്ണി = -1;
	പൂർണ്ണസംഖ്യ ret;
	പൂർണ്ണസംഖ്യ index = 0;
	പൂർണ്ണസംഖ്യ firmware_കണ്ണി;

	ret = spl_simple_fit_read(&ctx, info, offset, fit);
	എങ്കിൽ (ret < 0)
		തിരികെ ret;

	/* skip further processing if requested to enable load-only use cases */
	എങ്കിൽ (spl_load_simple_fit_skip_processing())
		തിരികെ 0;

	ctx.fit = spl_load_simple_fit_fix_load(ctx.fit);

	ret = spl_simple_fit_parse(&ctx);
	എങ്കിൽ (ret < 0)
		തിരികെ ret;

	എങ്കിൽ (IS_ENABLED(CONFIG_SPL_FPGA))
		spl_fit_load_fpga(&ctx, info, offset);

	/*
	 * Find the U-Boot image using the following search order:
	 *   - start at 'firmware' (e.g. an ARM Trusted Firmware)
	 *   - fall back 'kernel' (e.g. a Falcon-mode OS boot
	 *   - fall back to using the first 'loadables' entry
	 */
	എങ്കിൽ (കണ്ണി < 0)
		കണ്ണി = spl_fit_get_image_കണ്ണി(&ctx, FIT_FIRMWARE_PROP, 0);

	എങ്കിൽ (കണ്ണി < 0 && IS_ENABLED(CONFIG_SPL_OS_BOOT))
		കണ്ണി = spl_fit_get_image_കണ്ണി(&ctx, FIT_KERNEL_PROP, 0);

	എങ്കിൽ (കണ്ണി < 0) {
		debug("could not find firmware image, trying loadables...\n");
		കണ്ണി = spl_fit_get_image_കണ്ണി(&ctx, FIT_LOADABLE_PROP, 0);
		/*
		 * If we pick the U-Boot image from "loadables", start at
		 * the second image when later loading additional images.
		 */
		index = 1;
	}
	എങ്കിൽ (കണ്ണി < 0) {
		debug("%s: Cannot find u-boot image node: %d\n",
		      __func__, കണ്ണി);
		തിരികെ -1;
	}

	/* Load the image and set up the spl_image structure */
	ret = load_simple_fit(info, offset, &ctx, കണ്ണി, spl_image,
			      CONFIG_SYS_BOOTM_LEN);
	എങ്കിൽ (ret)
		തിരികെ ret;

	/*
	 * For backward compatibility, we treat the first node that is
	 * as a U-Boot image, if no OS-type has been declared.
	 */
	എങ്കിൽ (!spl_fit_image_get_os(ctx.fit, കണ്ണി, &spl_image->os))
		debug("Image OS is %s\n", genimg_get_os_name(spl_image->os));
	അല്ലെങ്കിൽ എങ്കിൽ (!IS_ENABLED(CONFIG_SPL_OS_BOOT))
		spl_image->os = IH_OS_U_BOOT;

	/*
	 * Booting a next-stage U-Boot may require us to append the FDT.
	 * We allow this to fail, as the U-Boot image might embed its FDT.
	 */
	എങ്കിൽ (os_takes_devicetree(spl_image->os)) {
		ret = spl_fit_append_fdt(spl_image, info, offset, &ctx);
		എങ്കിൽ (ret < 0 && spl_image->os != IH_OS_U_BOOT)
			തിരികെ ret;
	}

	firmware_കണ്ണി = കണ്ണി;
	/* Now check if there are more images for us to load */
	വേണ്ടി (; ; index++) {
		uint8_t os_type = IH_OS_INVALID;

		കണ്ണി = spl_fit_get_image_കണ്ണി(&ctx, FIT_LOADABLE_PROP, index);
		എങ്കിൽ (കണ്ണി < 0)
			നിർത്തുക;

		/*
		 * if the firmware is also a loadable, skip it because
		 * it already has been loaded. This is typically the case with
		 * u-boot.img generated by mkimage.
		 */
		എങ്കിൽ (firmware_കണ്ണി == കണ്ണി)
			തുടരുക;

		image_info.load_addr = 0;
		ret = load_simple_fit(info, offset, &ctx, കണ്ണി, &image_info,
				      CONFIG_SYS_BOOTM_LEN);
		എങ്കിൽ (ret < 0 && ret != -EBADSLT) {
			printf("%s: can't load image loadables index %d (ret = %d)\n",
			       __func__, index, ret);
			തിരികെ ret;
		}

		എങ്കിൽ (spl_fit_image_is_fpga(ctx.fit, കണ്ണി))
			spl_fit_upload_fpga(&ctx, കണ്ണി, &image_info);

		എങ്കിൽ (!spl_fit_image_get_os(ctx.fit, കണ്ണി, &os_type))
			debug("Loadable is %s\n", genimg_get_os_name(os_type));

		എങ്കിൽ (os_takes_devicetree(os_type)) {
			spl_fit_append_fdt(&image_info, info, offset, &ctx);
			spl_image->fdt_addr = image_info.fdt_addr;
		}

		/*
		 * If the "firmware" image did not provide an entry point,
		 * use the first valid entry point from the loadables.
		 */
		എങ്കിൽ (spl_image->entry_point == FDT_ERROR &&
		    image_info.entry_point != FDT_ERROR)
			spl_image->entry_point = image_info.entry_point;

		spl_fit_image_record_arm32_optee(ctx.fit, കണ്ണി, spl_image,
						 &image_info);

		/* Record our loadables into the FDT */
		എങ്കിൽ (!CONFIG_IS_ENABLED(FIT_IMAGE_TINY) &&
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
	എങ്കിൽ (spl_image->entry_point == FDT_ERROR || spl_image->entry_point == 0)
		spl_image->entry_point = spl_image->load_addr;

	spl_image->flags |= SPL_FIT_FOUND;
	upl_set_fit_info(map_to_sysmem(ctx.fit), ctx.conf_കണ്ണി,
			 spl_image->entry_point);

	തിരികെ 0;
}

/* Parse and load full fitImage in SPL */
പൂർണ്ണസംഖ്യ spl_load_fit_image(ഘടന spl_image_info *spl_image,
		       സ്ഥിരം ഘടന legacy_img_hdr *header)
{
	ഘടന bootm_headers images;
	സ്ഥിരം അക്ഷരം *fit_uname_config = ശൂന്യ;
	ulong fdt_hack;
	സ്ഥിരം അക്ഷരം *uname;
	ulong fw_data = 0, dt_data = 0, img_data = 0;
	ulong fw_len = 0, dt_len = 0, img_len = 0;
	പൂർണ്ണസംഖ്യ idx, conf_noffset;
	പൂർണ്ണസംഖ്യ ret;

	images.verify = CONFIG_IS_ENABLED(FIT_SIGNATURE);

	ret = fit_image_load(&images, virt_to_phys((ശൂന്യം *)header),
			     ശൂന്യ, &fit_uname_config, IH_ARCH_DEFAULT,
			     IH_TYPE_FIRMWARE, -1, FIT_LOAD_OPTIONAL,
			     &fw_data, &fw_len);

	എങ്കിൽ (ret < 0) {
		ret = fit_image_load(&images, virt_to_phys((ശൂന്യം *)header),
				     ശൂന്യ, &fit_uname_config, IH_ARCH_DEFAULT,
				     IH_TYPE_KERNEL, -1, FIT_LOAD_OPTIONAL,
				     &fw_data, &fw_len);
	}

	എങ്കിൽ (ret < 0)
		തിരികെ ret;

	spl_image->size = fw_len;
	spl_image->load_addr = fw_data;
	എങ്കിൽ (fit_image_get_entry(header, ret, &spl_image->entry_point))
		spl_image->entry_point = fw_data;
	എങ്കിൽ (fit_image_get_os(header, ret, &spl_image->os))
		spl_image->os = IH_OS_INVALID;
	spl_image->name = genimg_get_os_name(spl_image->os);

	debug(PHASE_PROMPT "payload image: %32s load addr: 0x%lx size: %d\n",
	      spl_image->name, spl_image->load_addr, spl_image->size);

	images.verify = CONFIG_IS_ENABLED(FIT_SIGNATURE);

	ret = fit_image_load(&images, virt_to_phys((ശൂന്യം *)header), ശൂന്യ,
			     &fit_uname_config, IH_ARCH_DEFAULT, IH_TYPE_FLATDT,
			     -1, FIT_LOAD_OPTIONAL, &dt_data, &dt_len);
	എങ്കിൽ (ret >= 0) {
		എങ്കിൽ (spl_image->os == IH_OS_U_BOOT) {
			/* HACK: U-Boot expects FDT at a specific address */
			fdt_hack = ALIGN(spl_image->load_addr + spl_image->size, 8);
			debug("Relocating FDT to %p\n", (ശൂന്യം *)fdt_hack);
			memcpy(map_sysmem(fdt_hack, dt_len),
			       map_sysmem(dt_data, 0), dt_len);
			spl_image->fdt_addr = (ശൂന്യം *)fdt_hack;
		} അല്ലെങ്കിൽ {
			spl_image->fdt_addr = (ശൂന്യം *)dt_data;
		}
	}

	conf_noffset = fit_conf_get_കണ്ണി((സ്ഥിരം ശൂന്യം *)header,
					 fit_uname_config);
	എങ്കിൽ (conf_noffset < 0)
		തിരികെ 0;

	വേണ്ടി (idx = 0;
	     uname = fdt_stringlist_get((സ്ഥിരം ശൂന്യം *)header, conf_noffset,
					FIT_LOADABLE_PROP, idx,
				ശൂന്യ), uname;
	     idx++) {
		images.verify = CONFIG_IS_ENABLED(FIT_SIGNATURE);

		ret = fit_image_load(&images, virt_to_phys((ശൂന്യം *)header),
				     &uname, &fit_uname_config,
				     IH_ARCH_DEFAULT, IH_TYPE_LOADABLE, -1,
				     FIT_LOAD_OPTIONAL_NON_ZERO,
				     &img_data, &img_len);
		എങ്കിൽ (ret < 0)
			തിരികെ ret;
	}
	spl_image->flags |= SPL_FIT_FOUND;

	upl_set_fit_info(map_to_sysmem(header), conf_noffset,
			 spl_image->entry_point);

	തിരികെ 0;
}
