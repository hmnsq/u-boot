// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#अन्तर्भाव <errno.h>
#अन्तर्भाव <fpga.h>
#अन्तर्भाव <gzip.h>
#अन्तर्भाव <image.h>
#अन्तर्भाव <log.h>
#अन्तर्भाव <memalign.h>
#अन्तर्भाव <mapmem.h>
#अन्तर्भाव <spl.h>
#अन्तर्भाव <upl.h>
#अन्तर्भाव <sysinfo.h>
#अन्तर्भाव <asm/global_data.h>
#अन्तर्भाव <asm/io.h>
#अन्तर्भाव <linux/libfdt.h>
#अन्तर्भाव <linux/printk.h>

DECLARE_GLOBAL_DATA_PTR;

संरचना spl_fit_info {
	अचर शून्य *fit;	/* Pointer to a valid FIT blob */
	माप_प्रकार ext_data_offset;	/* Offset to FIT external data (end of FIT) */
	पूर्णाङ्क images_गाँठो;	/* FDT offset to "/images" node */
	पूर्णाङ्क conf_गाँठो;		/* FDT offset to selected configuration node */
};

__weak ulong board_spl_fit_size_align(ulong size)
{
	फर्काऊ size;
}

स्थिर पूर्णाङ्क find_गाँठो_from_desc(अचर शून्य *fit, पूर्णाङ्क गाँठो, अचर वर्ण *str)
{
	पूर्णाङ्क child;

	यदि (गाँठो < 0)
		फर्काऊ -EINVAL;

	/* iterate the FIT nodes and find a matching description */
	लागि (child = fdt_first_subnode(fit, गाँठो); child >= 0;
	     child = fdt_next_subnode(fit, child)) {
		पूर्णाङ्क len;
		अचर वर्ण *desc = fdt_getprop(fit, child, FIT_DESC_PROP, &len);

		यदि (!desc)
			जारी;

		यदि (!म_भेद(desc, str))
			फर्काऊ child;
	}

	फर्काऊ -ENOENT;
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
स्थिर पूर्णाङ्क spl_fit_get_image_name(अचर संरचना spl_fit_info *ctx,
				  अचर वर्ण *type, पूर्णाङ्क index,
				  अचर वर्ण **outname)
{
	संरचना udevice *sysinfo;
	अचर वर्ण *name, *str, *end;
	__maybe_unused पूर्णाङ्क गाँठो;
	पूर्णाङ्क len, i;
	bool found = true;

	name = fdt_getprop(ctx->fit, ctx->conf_गाँठो, type, &len);
	यदि (!name) {
		debug("cannot find property '%s': %d\n", type, len);
		फर्काऊ -EINVAL;
	}
	/* A string property should be NUL terminated */
	end = name + len - 1;
	यदि (!len || *end) {
		debug("malformed property '%s'\n", type);
		फर्काऊ -EINVAL;
	}

	str = name;
	लागि (i = 0; i < index; i++) {
		str = म_अक्षर(str, '\0') + 1;
		यदि (str > end) {
			found = false;
			रोक;
		}
	}

	यदि (!found && CONFIG_IS_ENABLED(SYSINFO) && !sysinfo_get(&sysinfo)) {
		पूर्णाङ्क rc;
		/*
		 * no string in the property for this index. Check if the
		 * sysinfo-level code can supply one.
		 */
		rc = sysinfo_detect(sysinfo);
		यदि (rc)
			फर्काऊ rc;

		rc = sysinfo_get_fit_loadable(sysinfo, index - i - 1, type,
					      &str);
		यदि (rc && rc != -ENOENT)
			फर्काऊ rc;

		यदि (!rc) {
			/*
			 * The sysinfo provided a name for a loadable.
			 * Try to match it against the description properties
			 * first. If no matching node is found, use it as a
			 * node name.
			 */
			पूर्णाङ्क गाँठो;
			पूर्णाङ्क images = fdt_path_offset(ctx->fit, FIT_IMAGES_PATH);

			गाँठो = find_गाँठो_from_desc(ctx->fit, images, str);
			यदि (गाँठो > 0)
				str = fdt_get_name(ctx->fit, गाँठो, रिक्त);

			found = true;
		}
	}

	यदि (!found) {
		debug("no string for index %d\n", index);
		फर्काऊ -E2BIG;
	}

	*outname = str;
	फर्काऊ 0;
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
स्थिर पूर्णाङ्क spl_fit_get_image_गाँठो(अचर संरचना spl_fit_info *ctx,
				  अचर वर्ण *type, पूर्णाङ्क index)
{
	अचर वर्ण *str;
	पूर्णाङ्क err;
	पूर्णाङ्क गाँठो;

	err = spl_fit_get_image_name(ctx, type, index, &str);
	यदि (err)
		फर्काऊ err;

	debug("%s: '%s'\n", type, str);

	गाँठो = fdt_subnode_offset(ctx->fit, ctx->images_गाँठो, str);
	यदि (गाँठो < 0) {
		pr_err("cannot find image node '%s': %d\n", str, गाँठो);
		फर्काऊ -EINVAL;
	}

	फर्काऊ गाँठो;
}

स्थिर u32 get_aligned_image_offset(संरचना spl_load_info *info, u32 offset)
{
	फर्काऊ ALIGN_DOWN(offset, spl_get_bl_len(info));
}

स्थिर u32 get_aligned_image_overhead(संरचना spl_load_info *info, u32 offset)
{
	फर्काऊ offset & (spl_get_bl_len(info) - 1);
}

स्थिर पूर्णाङ्क get_aligned_image_size(संरचना spl_load_info *info, ulong data_size,
				  u32 offset, ulong *aligned_size)
{
	u32 overhead = get_aligned_image_overhead(info, offset);

	यदि (data_size > अच_दीर्घ_उच्च - overhead)
		फर्काऊ -EOVERFLOW;
	data_size += overhead;

	यदि (data_size > अच_दीर्घ_उच्च - (spl_get_bl_len(info) - 1))
		फर्काऊ -EOVERFLOW;
	data_size = ALIGN(data_size, spl_get_bl_len(info));

	*aligned_size = data_size;

	फर्काऊ 0;
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
स्थिर शून्य fit_fill_image_info(अचर शून्य *fit, पूर्णाङ्क गाँठो,
				संरचना spl_image_info *image_info,
				ulong load_addr, ulong size)
{
	ulong entry_point;

	यदि (!image_info)
		फर्काऊ;

	image_info->load_addr = load_addr;
	image_info->size = size;

	यदि (!fit_image_get_entry(fit, गाँठो, &entry_point))
		image_info->entry_point = entry_point;
	अन्यथा
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
स्थिर पूर्णाङ्क load_simple_fit(संरचना spl_load_info *info, ulong fit_offset,
			   अचर संरचना spl_fit_info *ctx, पूर्णाङ्क गाँठो,
			   संरचना spl_image_info *image_info, ulong max_size)
{
	u32 offset;
	u32 len;
	माप_प्रकार length;
	ulong size;
	ulong load_addr;
	शून्य *load_ptr;
	शून्य *src;
	uint8_t image_comp = -1, type = -1;
	अचर शून्य *data;
	अचर शून्य *fit = ctx->fit;
	bool external_data = false;
	पूर्णाङ्क ret;

	log_debug("starting\n");
	यदि (CONFIG_IS_ENABLED(BOOTMETH_VBE) &&
	    xpl_get_phase(info) != IH_PHASE_NONE) {
		गणना image_phase_t phase;

		ret = fit_image_get_phase(fit, गाँठो, &phase);
		/* if the image is for any phase, let's use it */
		यदि (ret == -ENOENT || phase == xpl_get_phase(info)) {
			log_debug("found\n");
		} अन्यथा यदि (ret < 0) {
			log_debug("err=%d\n", ret);
			फर्काऊ ret;
		} अन्यथा {
			log_debug("- phase mismatch, skipping this image\n");
			फर्काऊ -EBADSLT;
		}
	}

	यदि (IS_ENABLED(CONFIG_SPL_FPGA) ||
	    (IS_ENABLED(CONFIG_SPL_OS_BOOT) && spl_decompression_enabled())) {
		यदि (fit_image_get_type(fit, गाँठो, &type))
			माला_दो("Cannot get image type.\n");
		अन्यथा
			debug("%s ", genimg_get_type_name(type));
	}

	यदि (spl_decompression_enabled()) {
		fit_image_get_comp(fit, गाँठो, &image_comp);
		debug("%s ", genimg_get_comp_name(image_comp));
	}

	यदि (fit_image_get_load(fit, गाँठो, &load_addr)) {
		यदि (!image_info->load_addr) {
			म_लिखो("Can't load %s: No load address and no buffer\n",
			       fit_get_name(fit, गाँठो, रिक्त));
			फर्काऊ -ENOBUFS;
		}
		load_addr = image_info->load_addr;
	}

	यदि (!fit_image_get_data_position(fit, गाँठो, &offset)) {
		external_data = true;
	} अन्यथा यदि (!fit_image_get_data_offset(fit, गाँठो, &offset)) {
		/* The resulting offset cannot exceed UINT32_MAX */
		यदि (ctx->ext_data_offset > UINT32_MAX - offset) {
			log_debug("Invalid external data offset: %u\n", offset);
			फर्काऊ -EINVAL;
		}
		log_debug("read offset %x = offset from fit %x\n", offset,
			  (u32)(offset + ctx->ext_data_offset));
		offset += ctx->ext_data_offset;
		external_data = true;
	}

	यदि (external_data) {
		u32 aligned_offset;
		ulong read_offset;
		शून्य *src_ptr;

		/* External data */
		यदि (fit_image_get_data_size(fit, गाँठो, &len))
			फर्काऊ -ENOENT;

		/* Dont bother to copy 0 byte data, but warn, though */
		यदि (!len) {
			log_warning("%s: Skip load '%s': image size is 0!\n",
				    __func__, fit_get_name(fit, गाँठो, रिक्त));
			fit_fill_image_info(fit, गाँठो, image_info, load_addr, 0);
			फर्काऊ 0;
		}

		/*
		 * data-size is excluded from the configuration signature (it
		 * is in exc_prop[] in image-fit-sig.c), so it stays attacker
		 * controlled even after fit_config_verify() succeeds. The
		 * image hash is only verified after the device read below, so
		 * an oversized value has to be rejected here.
		 */
		ret = get_aligned_image_size(info, len, offset, &size);
		यदि (ret) {
			log_debug("Invalid external data size: %u\n", len);
			फर्काऊ ret;
		}

		यदि (size > max_size) {
			log_debug("Image too large: aligned size %lu, max %lu (data-size %u)\n",
				  size, max_size, len);
			फर्काऊ -EFBIG;
		}

		aligned_offset = get_aligned_image_offset(info, offset);
		यदि (aligned_offset > अच_दीर्घ_उच्च - fit_offset) {
			log_debug("Invalid aligned external data offset: %u\n",
				  aligned_offset);
			फर्काऊ -EINVAL;
		}
		read_offset = fit_offset + aligned_offset;

		यदि (spl_decompression_enabled() &&
		    (image_comp == IH_COMP_GZIP || image_comp == IH_COMP_LZMA))
			src_ptr = map_sysmem(ALIGN(CONFIG_SYS_LOAD_ADDR, ARCH_DMA_MINALIGN), len);
		अन्यथा
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
		यदि (size > अच_दीर्घ_उच्च - read_offset ||
		    size > UINTPTR_MAX - (uintptr_t)src_ptr
		/*
		 * #if (not a runtime if) is required: FIT_SIGNATURE_MAX_SIZE
		 * depends on FIT_SIGNATURE, so CONFIG_VAL(FIT_SIGNATURE_MAX_SIZE)
		 * is undefined when signing is disabled and referencing it
		 * here would fail to compile.
		 */
#यदि CONFIG_IS_ENABLED(FIT_SIGNATURE)
		    || offset > CONFIG_VAL(FIT_SIGNATURE_MAX_SIZE) ||
		    len > CONFIG_VAL(FIT_SIGNATURE_MAX_SIZE) - offset
#endif
		) {
			log_debug("FIT external data is out of bounds (offset=%u, size=%u)\n",
				  offset, len);
			फर्काऊ -EINVAL;
		}

		log_debug("reading from offset %x / %lx size %lx to %p: ",
			  offset, read_offset, size, src_ptr);

		यदि (info->पठ(info, read_offset, size, src_ptr) < length)
			फर्काऊ -EIO;

		debug("External data: dst=%p, offset=%x, size=%lx\n",
		      src_ptr, offset, (अचिन्हित दीर्घ)length);
		src = src_ptr + get_aligned_image_overhead(info, offset);
	} अन्यथा {
		/* Embedded data */
		यदि (fit_image_get_emb_data(fit, गाँठो, &data, &length)) {
			माला_दो("Cannot get image data/size\n");
			फर्काऊ -ENOENT;
		}
		debug("Embedded data: dst=%lx, size=%lx\n", load_addr,
		      (अचिन्हित दीर्घ)length);
		src = (शून्य *)data;	/* cast away const */
	}

	यदि (CONFIG_IS_ENABLED(FIT_SIGNATURE)) {
		म_लिखो("## Checking hash(es) for Image %s ... ",
		       fit_get_name(fit, गाँठो, रिक्त));
		यदि (!fit_image_verify_with_data(fit, गाँठो, gd_fdt_blob(), src,
						length))
			फर्काऊ -EPERM;
		माला_दो("OK\n");
	}

	यदि (CONFIG_IS_ENABLED(FIT_IMAGE_POST_PROCESS))
		board_fit_image_post_process(fit, गाँठो, &src, &length);

	load_ptr = map_sysmem(load_addr, length);
	यदि (IS_ENABLED(CONFIG_SPL_GZIP) && image_comp == IH_COMP_GZIP) {
		size = length;
		यदि (gunzip(load_ptr, CONFIG_SYS_BOOTM_LEN, src, &size)) {
			माला_दो("Uncompressing error\n");
			फर्काऊ -EIO;
		}
		length = size;
	} अन्यथा यदि (IS_ENABLED(CONFIG_SPL_LZMA) && image_comp == IH_COMP_LZMA) {
		size = CONFIG_SYS_BOOTM_LEN;
		ulong loadEnd;

		यदि (image_decomp(IH_COMP_LZMA, CONFIG_SYS_LOAD_ADDR, 0, 0,
				 load_ptr, src, length, size, &loadEnd)) {
			माला_दो("Uncompressing error\n");
			फर्काऊ -EIO;
		}
		length = loadEnd - CONFIG_SYS_LOAD_ADDR;
	} अन्यथा {
		स_हटाओ(load_ptr, src, length);
	}

	fit_fill_image_info(fit, गाँठो, image_info, load_addr, length);
	log_debug("- done loading\n");

	upl_add_image(fit, गाँठो, load_addr, length);

	फर्काऊ 0;
}

स्थिर bool os_takes_devicetree(uint8_t os)
{
	विकल्प (os) {
	पक्ष IH_OS_U_BOOT:
		फर्काऊ true;
	पक्ष IH_OS_LINUX:
		फर्काऊ IS_ENABLED(CONFIG_SPL_OS_BOOT) ||
		       IS_ENABLED(CONFIG_SPL_OPENSBI);
	स्वभाव:
		फर्काऊ false;
	}
}

__weak पूर्णाङ्क board_spl_fit_append_fdt_skip(अचर वर्ण *name)
{
	फर्काऊ 0;	/* Do not skip */
}

स्थिर पूर्णाङ्क spl_fit_append_fdt(संरचना spl_image_info *spl_image,
			      संरचना spl_load_info *info, ulong offset,
			      अचर संरचना spl_fit_info *ctx)
{
	संरचना spl_image_info image_info;
	पूर्णाङ्क गाँठो, ret = 0, index = 0;

	/*
	 * Use the address following the image as target address for the
	 * device tree.
	 */
	image_info.load_addr = ALIGN(spl_image->load_addr + spl_image->size, 8);

	/* Figure out which device tree the board wants to use */
	गाँठो = spl_fit_get_image_गाँठो(ctx, FIT_FDT_PROP, index++);
	यदि (गाँठो < 0) {
		माप_प्रकार size;

		debug("%s: cannot find FDT node\n", __func__);

		/*
		 * U-Boot did not find a device tree inside the FIT image. Use
		 * the U-Boot device tree instead.
		 */
		यदि (!gd->fdt_blob)
			फर्काऊ गाँठो;

		/*
		 * Make the load-address of the FDT available for the SPL
		 * framework
		 */
		size = fdt_totalsize(gd->fdt_blob);
		spl_image->fdt_addr = map_sysmem(image_info.load_addr, size);
		स_नकल(spl_image->fdt_addr, gd->fdt_blob, size);
	} अन्यथा {
		ret = load_simple_fit(info, offset, ctx, गाँठो, &image_info,
				      CONFIG_SYS_BOOTM_LEN);
		यदि (ret < 0)
			फर्काऊ ret;

		spl_image->fdt_addr = phys_to_virt(image_info.load_addr);
	}

	यदि (CONFIG_IS_ENABLED(FIT_IMAGE_TINY))
		फर्काऊ 0;

#यदि CONFIG_IS_ENABLED(LOAD_FIT_APPLY_OVERLAY)
		शून्य *tmpbuffer = रिक्त;

		लागि (; ; index++) {
			अचर वर्ण *str;

			ret = spl_fit_get_image_name(ctx, FIT_FDT_PROP, index, &str);
			यदि (ret == -E2BIG) {
				debug("%s: No additional FDT node\n", __func__);
				ret = 0;
				रोक;
			} अन्यथा यदि (ret) {
				रोक;
			}

			ret = board_spl_fit_append_fdt_skip(str);
			यदि (ret)
				जारी;

			गाँठो = fdt_subnode_offset(ctx->fit, ctx->images_गाँठो, str);
			यदि (गाँठो < 0) {
				debug("%s: unable to find FDT node %d\n",
				      __func__, index);
				जारी;
			}

			यदि (!tmpbuffer) {
				/*
				 * allocate memory to store the DT overlay
				 * before it is applied. It may not be used
				 * depending on how the overlay is stored, so
				 * don't fail yet if the allocation failed.
				 */
				माप_प्रकार size = CONFIG_SPL_LOAD_FIT_APPLY_OVERLAY_BUF_SZ;

				tmpbuffer = malloc_cache_aligned(size);
				यदि (!tmpbuffer)
					debug("%s: unable to allocate space for overlays\n",
					      __func__);
			}
			image_info.load_addr = (ulong)tmpbuffer;
			ret = load_simple_fit(info, offset, ctx, गाँठो,
					      &image_info,
					      CONFIG_SPL_LOAD_FIT_APPLY_OVERLAY_BUF_SZ);
			यदि (ret == -EBADSLT)
				जारी;
			अन्यथा यदि (ret < 0)
				रोक;

			/* Make room in FDT for changes from the overlay */
			ret = fdt_increase_size(spl_image->fdt_addr,
						image_info.size);
			यदि (ret < 0)
				रोक;

			ret = fdt_overlay_apply_verbose(spl_image->fdt_addr,
							(शून्य *)image_info.load_addr);
			यदि (ret) {
				pr_err("failed to apply DT overlay %s\n",
				       fit_get_name(ctx->fit, गाँठो, रिक्त));
				रोक;
			}

			debug("%s: DT overlay %s applied\n", __func__,
			      fit_get_name(ctx->fit, गाँठो, रिक्त));
		}
		मुक्त(tmpbuffer);
		यदि (ret)
			फर्काऊ ret;
#endif
	/* Try to make space, so we can inject details on the loadables */
	ret = fdt_shrink_to_minimum(spl_image->fdt_addr, 8192);
	यदि (ret < 0)
		फर्काऊ ret;

	फर्काऊ ret;
}

स्थिर पूर्णाङ्क spl_fit_record_loadable(अचर संरचना spl_fit_info *ctx, पूर्णाङ्क index,
				   शून्य *blob, संरचना spl_image_info *image)
{
	पूर्णाङ्क ret = 0;
	अचर वर्ण *name;
	पूर्णाङ्क गाँठो;

	ret = spl_fit_get_image_name(ctx, FIT_LOADABLE_PROP, index, &name);
	यदि (ret < 0)
		फर्काऊ ret;

	गाँठो = spl_fit_get_image_गाँठो(ctx, FIT_LOADABLE_PROP, index);

	ret = fdt_record_loadable(blob, index, name, image->load_addr,
			image->size, image->entry_point,
			fdt_getprop(ctx->fit, गाँठो, FIT_TYPE_PROP, रिक्त),
			fdt_getprop(ctx->fit, गाँठो, FIT_OS_PROP, रिक्त),
			fdt_getprop(ctx->fit, गाँठो, FIT_ARCH_PROP, रिक्त));

	फर्काऊ ret;
}

स्थिर पूर्णाङ्क spl_fit_image_is_fpga(अचर शून्य *fit, पूर्णाङ्क गाँठो)
{
	अचर वर्ण *type;

	यदि (!IS_ENABLED(CONFIG_SPL_FPGA))
		फर्काऊ 0;

	type = fdt_getprop(fit, गाँठो, FIT_TYPE_PROP, रिक्त);
	यदि (!type)
		फर्काऊ 0;

	फर्काऊ !म_भेद(type, "fpga");
}

स्थिर शून्य spl_fit_image_record_arm32_optee(अचर शून्य *fit, पूर्णाङ्क गाँठो,
					     संरचना spl_image_info *spl_image,
					     संरचना spl_image_info *image_info)
{
#यदि defined(CONFIG_BOOTM_OPTEE) && defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
	अचर वर्ण *type = fdt_getprop(fit, गाँठो, FIT_TYPE_PROP, रिक्त);

	यदि (!type)
		फर्काऊ;

	यदि (म_भेद(type, "tee"))
		फर्काऊ;

	spl_image->optee_addr = image_info->load_addr;
#endif
}

स्थिर पूर्णाङ्क spl_fit_image_get_os(अचर शून्य *fit, पूर्णाङ्क noffset, uint8_t *os)
{
	यदि (!CONFIG_IS_ENABLED(FIT_IMAGE_TINY) || CONFIG_IS_ENABLED(OS_BOOT))
		फर्काऊ fit_image_get_os(fit, noffset, os);

	अचर वर्ण *name = fdt_getprop(fit, noffset, FIT_OS_PROP, रिक्त);
	यदि (!name)
		फर्काऊ -ENOENT;

	/*
	 * We don't care what the type of the image actually is,
	 * only whether or not it is U-Boot. This saves some
	 * space by omitting the large table of OS types.
	 */
	यदि (!म_भेद(name, "u-boot"))
		*os = IH_OS_U_BOOT;
	अन्यथा
		*os = IH_OS_INVALID;

	फर्काऊ 0;
}

/*
 * The purpose of the FIT load buffer is to provide a memory location that is
 * independent of the load address of any FIT component.
 */
__weak शून्य *board_spl_fit_buffer_addr(ulong fit_size, पूर्णाङ्क sectors, पूर्णाङ्क bl_len)
{
	शून्य *buf;
	माप_प्रकार size = sectors * bl_len;

	buf = malloc_cache_aligned(size);
	यदि (!buf) {
		pr_err("Could not get FIT buffer of %lu bytes\n", (ulong)size);

		यदि (IS_ENABLED(CONFIG_SPL_SYS_MALLOC))
			pr_err("\tcheck CONFIG_SPL_SYS_MALLOC_SIZE\n");
		अन्यथा
			pr_err("\tcheck CONFIG_SPL_SYS_MALLOC_F_LEN\n");

		buf = spl_get_load_buffer(0, size);
	}
	फर्काऊ buf;
}

/*
 * Weak default function to allow customizing SPL fit loading for load-only
 * use cases by allowing to skip the parsing/processing of the FIT contents
 * (so that this can be done separately in a more customized fashion)
 */
__weak bool spl_load_simple_fit_skip_processing(शून्य)
{
	फर्काऊ false;
}

/*
 * Weak default function to allow fixes after fit header
 * is loaded.
 */
__weak शून्य *spl_load_simple_fit_fix_load(अचर शून्य *fit)
{
	फर्काऊ (शून्य *)fit;
}

स्थिर शून्य warn_deprecated(अचर वर्ण *सन्देश)
{
	म_लिखो("DEPRECATED: %s\n", सन्देश);
	म_लिखो("\tSee https://fitspec.osfw.foundation/\n");
}

स्थिर पूर्णाङ्क spl_fit_upload_fpga(संरचना spl_fit_info *ctx, पूर्णाङ्क गाँठो,
			       संरचना spl_image_info *fpga_image)
{
	अचर वर्ण *compatible;
	पूर्णाङ्क ret;
	पूर्णाङ्क devnum = 0;
	पूर्णाङ्क flags = 0;

	debug("FPGA bitstream at: %x, size: %x\n",
	      (u32)fpga_image->load_addr, fpga_image->size);

	compatible = fdt_getprop(ctx->fit, गाँठो, FIT_COMPAT_PROP, रिक्त);
	यदि (!compatible) {
		warn_deprecated("'fpga' image without 'compatible' property");
	} अन्यथा {
		यदि (CONFIG_IS_ENABLED(FPGA_LOAD_SECURE))
			flags = fpga_compatible2flag(devnum, compatible);
		यदि (म_भेद(compatible, "u-boot,fpga-legacy"))
			debug("Ignoring compatible = %s property\n",
			      compatible);
	}

	ret = fpga_load(devnum, (शून्य *)fpga_image->load_addr,
			fpga_image->size, BIT_FULL, flags);
	यदि (ret) {
		म_लिखो("%s: Cannot load the image to the FPGA\n", __func__);
		फर्काऊ ret;
	}

	माला_दो("FPGA image loaded from FIT\n");

	फर्काऊ 0;
}

स्थिर पूर्णाङ्क spl_fit_load_fpga(संरचना spl_fit_info *ctx,
			     संरचना spl_load_info *info, ulong offset)
{
	पूर्णाङ्क गाँठो, ret;

	संरचना spl_image_info fpga_image = {
		.load_addr = 0,
	};

	गाँठो = spl_fit_get_image_गाँठो(ctx, FIT_FPGA_PROP, 0);
	यदि (गाँठो < 0)
		फर्काऊ गाँठो;

	warn_deprecated("'fpga' property in config node. Use 'loadables'");

	/* Load the image and set up the fpga_image structure */
	ret = load_simple_fit(info, offset, ctx, गाँठो, &fpga_image,
			      CONFIG_SYS_BOOTM_LEN);
	यदि (ret) {
		म_लिखो("%s: Cannot load the FPGA: %i\n", __func__, ret);
		फर्काऊ ret;
	}

	फर्काऊ spl_fit_upload_fpga(ctx, गाँठो, &fpga_image);
}

स्थिर पूर्णाङ्क spl_simple_fit_read(संरचना spl_fit_info *ctx,
			       संरचना spl_load_info *info, ulong offset,
			       अचर शून्य *fit_header)
{
	अचिन्हित दीर्घ aligned_size, गन्ती, size;
	शून्य *buf;
	पूर्णाङ्क ret;

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
	यदि (ret) {
		log_debug("Invalid FIT size: %lu\n", size);
		फर्काऊ ret;
	}
	buf = board_spl_fit_buffer_addr(aligned_size, aligned_size, 1);
	यदि (!buf) {
		/*
		 * We assume that none of the board will ever use 0x0 as a
		 * valid load address. Theoretically some board could use it,
		 * but this is extremely unlikely.
		 */
		फर्काऊ -EIO;
	}

	गन्ती = info->पठ(info, offset, aligned_size, buf);
	यदि (!गन्ती) {
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
		फर्काऊ -EIO;
	}

	ctx->fit = buf;
	debug("fit read offset %lx, size=%lu, dst=%p, count=%lu\n",
	      offset, aligned_size, buf, गन्ती);

	फर्काऊ 0;
}

स्थिर पूर्णाङ्क spl_simple_fit_parse(संरचना spl_fit_info *ctx)
{
	/* Find the correct subnode under "/configurations" */
	ctx->conf_गाँठो = fit_find_config_गाँठो(ctx->fit);
	यदि (ctx->conf_गाँठो < 0)
		फर्काऊ -EINVAL;

	यदि (CONFIG_IS_ENABLED(FIT_SIGNATURE)) {
		म_लिखो("## Checking hash(es) for config %s ... ",
		       fit_get_name(ctx->fit, ctx->conf_गाँठो, रिक्त));
		यदि (fit_config_verify(ctx->fit, ctx->conf_गाँठो))
			फर्काऊ -EPERM;
		माला_दो("OK\n");
	}

	/* find the node holding the images information */
	ctx->images_गाँठो = fdt_path_offset(ctx->fit, FIT_IMAGES_PATH);
	यदि (ctx->images_गाँठो < 0) {
		debug("%s: Cannot find /images node: %d\n", __func__,
		      ctx->images_गाँठो);
		फर्काऊ -EINVAL;
	}

	फर्काऊ 0;
}

पूर्णाङ्क spl_load_simple_fit(संरचना spl_image_info *spl_image,
			संरचना spl_load_info *info, ulong offset, शून्य *fit)
{
	संरचना spl_image_info image_info;
	संरचना spl_fit_info ctx;
	पूर्णाङ्क गाँठो = -1;
	पूर्णाङ्क ret;
	पूर्णाङ्क index = 0;
	पूर्णाङ्क firmware_गाँठो;

	ret = spl_simple_fit_read(&ctx, info, offset, fit);
	यदि (ret < 0)
		फर्काऊ ret;

	/* skip further processing if requested to enable load-only use cases */
	यदि (spl_load_simple_fit_skip_processing())
		फर्काऊ 0;

	ctx.fit = spl_load_simple_fit_fix_load(ctx.fit);

	ret = spl_simple_fit_parse(&ctx);
	यदि (ret < 0)
		फर्काऊ ret;

	यदि (IS_ENABLED(CONFIG_SPL_FPGA))
		spl_fit_load_fpga(&ctx, info, offset);

	/*
	 * Find the U-Boot image using the following search order:
	 *   - start at 'firmware' (e.g. an ARM Trusted Firmware)
	 *   - fall back 'kernel' (e.g. a Falcon-mode OS boot
	 *   - fall back to using the first 'loadables' entry
	 */
	यदि (गाँठो < 0)
		गाँठो = spl_fit_get_image_गाँठो(&ctx, FIT_FIRMWARE_PROP, 0);

	यदि (गाँठो < 0 && IS_ENABLED(CONFIG_SPL_OS_BOOT))
		गाँठो = spl_fit_get_image_गाँठो(&ctx, FIT_KERNEL_PROP, 0);

	यदि (गाँठो < 0) {
		debug("could not find firmware image, trying loadables...\n");
		गाँठो = spl_fit_get_image_गाँठो(&ctx, FIT_LOADABLE_PROP, 0);
		/*
		 * If we pick the U-Boot image from "loadables", start at
		 * the second image when later loading additional images.
		 */
		index = 1;
	}
	यदि (गाँठो < 0) {
		debug("%s: Cannot find u-boot image node: %d\n",
		      __func__, गाँठो);
		फर्काऊ -1;
	}

	/* Load the image and set up the spl_image structure */
	ret = load_simple_fit(info, offset, &ctx, गाँठो, spl_image,
			      CONFIG_SYS_BOOTM_LEN);
	यदि (ret)
		फर्काऊ ret;

	/*
	 * For backward compatibility, we treat the first node that is
	 * as a U-Boot image, if no OS-type has been declared.
	 */
	यदि (!spl_fit_image_get_os(ctx.fit, गाँठो, &spl_image->os))
		debug("Image OS is %s\n", genimg_get_os_name(spl_image->os));
	अन्यथा यदि (!IS_ENABLED(CONFIG_SPL_OS_BOOT))
		spl_image->os = IH_OS_U_BOOT;

	/*
	 * Booting a next-stage U-Boot may require us to append the FDT.
	 * We allow this to fail, as the U-Boot image might embed its FDT.
	 */
	यदि (os_takes_devicetree(spl_image->os)) {
		ret = spl_fit_append_fdt(spl_image, info, offset, &ctx);
		यदि (ret < 0 && spl_image->os != IH_OS_U_BOOT)
			फर्काऊ ret;
	}

	firmware_गाँठो = गाँठो;
	/* Now check if there are more images for us to load */
	लागि (; ; index++) {
		uint8_t os_type = IH_OS_INVALID;

		गाँठो = spl_fit_get_image_गाँठो(&ctx, FIT_LOADABLE_PROP, index);
		यदि (गाँठो < 0)
			रोक;

		/*
		 * if the firmware is also a loadable, skip it because
		 * it already has been loaded. This is typically the case with
		 * u-boot.img generated by mkimage.
		 */
		यदि (firmware_गाँठो == गाँठो)
			जारी;

		image_info.load_addr = 0;
		ret = load_simple_fit(info, offset, &ctx, गाँठो, &image_info,
				      CONFIG_SYS_BOOTM_LEN);
		यदि (ret < 0 && ret != -EBADSLT) {
			म_लिखो("%s: can't load image loadables index %d (ret = %d)\n",
			       __func__, index, ret);
			फर्काऊ ret;
		}

		यदि (spl_fit_image_is_fpga(ctx.fit, गाँठो))
			spl_fit_upload_fpga(&ctx, गाँठो, &image_info);

		यदि (!spl_fit_image_get_os(ctx.fit, गाँठो, &os_type))
			debug("Loadable is %s\n", genimg_get_os_name(os_type));

		यदि (os_takes_devicetree(os_type)) {
			spl_fit_append_fdt(&image_info, info, offset, &ctx);
			spl_image->fdt_addr = image_info.fdt_addr;
		}

		/*
		 * If the "firmware" image did not provide an entry point,
		 * use the first valid entry point from the loadables.
		 */
		यदि (spl_image->entry_point == FDT_ERROR &&
		    image_info.entry_point != FDT_ERROR)
			spl_image->entry_point = image_info.entry_point;

		spl_fit_image_record_arm32_optee(ctx.fit, गाँठो, spl_image,
						 &image_info);

		/* Record our loadables into the FDT */
		यदि (!CONFIG_IS_ENABLED(FIT_IMAGE_TINY) &&
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
	यदि (spl_image->entry_point == FDT_ERROR || spl_image->entry_point == 0)
		spl_image->entry_point = spl_image->load_addr;

	spl_image->flags |= SPL_FIT_FOUND;
	upl_set_fit_info(map_to_sysmem(ctx.fit), ctx.conf_गाँठो,
			 spl_image->entry_point);

	फर्काऊ 0;
}

/* Parse and load full fitImage in SPL */
पूर्णाङ्क spl_load_fit_image(संरचना spl_image_info *spl_image,
		       अचर संरचना legacy_img_hdr *header)
{
	संरचना bootm_headers images;
	अचर वर्ण *fit_uname_config = रिक्त;
	ulong fdt_hack;
	अचर वर्ण *uname;
	ulong fw_data = 0, dt_data = 0, img_data = 0;
	ulong fw_len = 0, dt_len = 0, img_len = 0;
	पूर्णाङ्क idx, conf_noffset;
	पूर्णाङ्क ret;

	images.verify = CONFIG_IS_ENABLED(FIT_SIGNATURE);

	ret = fit_image_load(&images, virt_to_phys((शून्य *)header),
			     रिक्त, &fit_uname_config, IH_ARCH_DEFAULT,
			     IH_TYPE_FIRMWARE, -1, FIT_LOAD_OPTIONAL,
			     &fw_data, &fw_len);

	यदि (ret < 0) {
		ret = fit_image_load(&images, virt_to_phys((शून्य *)header),
				     रिक्त, &fit_uname_config, IH_ARCH_DEFAULT,
				     IH_TYPE_KERNEL, -1, FIT_LOAD_OPTIONAL,
				     &fw_data, &fw_len);
	}

	यदि (ret < 0)
		फर्काऊ ret;

	spl_image->size = fw_len;
	spl_image->load_addr = fw_data;
	यदि (fit_image_get_entry(header, ret, &spl_image->entry_point))
		spl_image->entry_point = fw_data;
	यदि (fit_image_get_os(header, ret, &spl_image->os))
		spl_image->os = IH_OS_INVALID;
	spl_image->name = genimg_get_os_name(spl_image->os);

	debug(PHASE_PROMPT "payload image: %32s load addr: 0x%lx size: %d\n",
	      spl_image->name, spl_image->load_addr, spl_image->size);

	images.verify = CONFIG_IS_ENABLED(FIT_SIGNATURE);

	ret = fit_image_load(&images, virt_to_phys((शून्य *)header), रिक्त,
			     &fit_uname_config, IH_ARCH_DEFAULT, IH_TYPE_FLATDT,
			     -1, FIT_LOAD_OPTIONAL, &dt_data, &dt_len);
	यदि (ret >= 0) {
		यदि (spl_image->os == IH_OS_U_BOOT) {
			/* HACK: U-Boot expects FDT at a specific address */
			fdt_hack = ALIGN(spl_image->load_addr + spl_image->size, 8);
			debug("Relocating FDT to %p\n", (शून्य *)fdt_hack);
			स_नकल(map_sysmem(fdt_hack, dt_len),
			       map_sysmem(dt_data, 0), dt_len);
			spl_image->fdt_addr = (शून्य *)fdt_hack;
		} अन्यथा {
			spl_image->fdt_addr = (शून्य *)dt_data;
		}
	}

	conf_noffset = fit_conf_get_गाँठो((अचर शून्य *)header,
					 fit_uname_config);
	यदि (conf_noffset < 0)
		फर्काऊ 0;

	लागि (idx = 0;
	     uname = fdt_stringlist_get((अचर शून्य *)header, conf_noffset,
					FIT_LOADABLE_PROP, idx,
				रिक्त), uname;
	     idx++) {
		images.verify = CONFIG_IS_ENABLED(FIT_SIGNATURE);

		ret = fit_image_load(&images, virt_to_phys((शून्य *)header),
				     &uname, &fit_uname_config,
				     IH_ARCH_DEFAULT, IH_TYPE_LOADABLE, -1,
				     FIT_LOAD_OPTIONAL_NON_ZERO,
				     &img_data, &img_len);
		यदि (ret < 0)
			फर्काऊ ret;
	}
	spl_image->flags |= SPL_FIT_FOUND;

	upl_set_fit_info(map_to_sysmem(header), conf_noffset,
			 spl_image->entry_point);

	फर्काऊ 0;
}
