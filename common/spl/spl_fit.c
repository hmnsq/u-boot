// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <fpga.h>
#અન્તર્ભાવ <gzip.h>
#અન્તર્ભાવ <image.h>
#અન્તર્ભાવ <log.h>
#અન્તર્ભાવ <memalign.h>
#અન્તર્ભાવ <mapmem.h>
#અન્તર્ભાવ <spl.h>
#અન્તર્ભાવ <upl.h>
#અન્તર્ભાવ <sysinfo.h>
#અન્તર્ભાવ <asm/global_data.h>
#અન્તર્ભાવ <asm/io.h>
#અન્તર્ભાવ <linux/libfdt.h>
#અન્તર્ભાવ <linux/printk.h>

DECLARE_GLOBAL_DATA_PTR;

સંરચના spl_fit_info {
	અચલ શૂન્ય *fit;	/* Pointer to a valid FIT blob */
	size_t ext_data_offset;	/* Offset to FIT external data (end of FIT) */
	પૂર્ણાંક images_ગાંઠ;	/* FDT offset to "/images" node */
	પૂર્ણાંક conf_ગાંઠ;		/* FDT offset to selected configuration node */
};

__weak ulong board_spl_fit_size_align(ulong size)
{
	પાછા size;
}

સ્થિર પૂર્ણાંક find_ગાંઠ_from_desc(અચલ શૂન્ય *fit, પૂર્ણાંક ગાંઠ, અચલ અક્ષર *str)
{
	પૂર્ણાંક child;

	જો (ગાંઠ < 0)
		પાછા -EINVAL;

	/* iterate the FIT nodes and find a matching description */
	માટે (child = fdt_first_subnode(fit, ગાંઠ); child >= 0;
	     child = fdt_next_subnode(fit, child)) {
		પૂર્ણાંક len;
		અચલ અક્ષર *desc = fdt_getprop(fit, child, FIT_DESC_PROP, &len);

		જો (!desc)
			ચાલુ;

		જો (!strcmp(desc, str))
			પાછા child;
	}

	પાછા -ENOENT;
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
સ્થિર પૂર્ણાંક spl_fit_get_image_name(અચલ સંરચના spl_fit_info *ctx,
				  અચલ અક્ષર *type, પૂર્ણાંક index,
				  અચલ અક્ષર **outname)
{
	સંરચના udevice *sysinfo;
	અચલ અક્ષર *name, *str, *end;
	__maybe_unused પૂર્ણાંક ગાંઠ;
	પૂર્ણાંક len, i;
	bool found = true;

	name = fdt_getprop(ctx->fit, ctx->conf_ગાંઠ, type, &len);
	જો (!name) {
		debug("cannot find property '%s': %d\n", type, len);
		પાછા -EINVAL;
	}
	/* A string property should be NUL terminated */
	end = name + len - 1;
	જો (!len || *end) {
		debug("malformed property '%s'\n", type);
		પાછા -EINVAL;
	}

	str = name;
	માટે (i = 0; i < index; i++) {
		str = strchr(str, '\0') + 1;
		જો (str > end) {
			found = false;
			તોડો;
		}
	}

	જો (!found && CONFIG_IS_ENABLED(SYSINFO) && !sysinfo_get(&sysinfo)) {
		પૂર્ણાંક rc;
		/*
		 * no string in the property for this index. Check if the
		 * sysinfo-level code can supply one.
		 */
		rc = sysinfo_detect(sysinfo);
		જો (rc)
			પાછા rc;

		rc = sysinfo_get_fit_loadable(sysinfo, index - i - 1, type,
					      &str);
		જો (rc && rc != -ENOENT)
			પાછા rc;

		જો (!rc) {
			/*
			 * The sysinfo provided a name for a loadable.
			 * Try to match it against the description properties
			 * first. If no matching node is found, use it as a
			 * node name.
			 */
			પૂર્ણાંક ગાંઠ;
			પૂર્ણાંક images = fdt_path_offset(ctx->fit, FIT_IMAGES_PATH);

			ગાંઠ = find_ગાંઠ_from_desc(ctx->fit, images, str);
			જો (ગાંઠ > 0)
				str = fdt_get_name(ctx->fit, ગાંઠ, ખાલી);

			found = true;
		}
	}

	જો (!found) {
		debug("no string for index %d\n", index);
		પાછા -E2BIG;
	}

	*outname = str;
	પાછા 0;
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
સ્થિર પૂર્ણાંક spl_fit_get_image_ગાંઠ(અચલ સંરચના spl_fit_info *ctx,
				  અચલ અક્ષર *type, પૂર્ણાંક index)
{
	અચલ અક્ષર *str;
	પૂર્ણાંક err;
	પૂર્ણાંક ગાંઠ;

	err = spl_fit_get_image_name(ctx, type, index, &str);
	જો (err)
		પાછા err;

	debug("%s: '%s'\n", type, str);

	ગાંઠ = fdt_subnode_offset(ctx->fit, ctx->images_ગાંઠ, str);
	જો (ગાંઠ < 0) {
		pr_err("cannot find image node '%s': %d\n", str, ગાંઠ);
		પાછા -EINVAL;
	}

	પાછા ગાંઠ;
}

સ્થિર u32 get_aligned_image_offset(સંરચના spl_load_info *info, u32 offset)
{
	પાછા ALIGN_DOWN(offset, spl_get_bl_len(info));
}

સ્થિર u32 get_aligned_image_overhead(સંરચના spl_load_info *info, u32 offset)
{
	પાછા offset & (spl_get_bl_len(info) - 1);
}

સ્થિર પૂર્ણાંક get_aligned_image_size(સંરચના spl_load_info *info, ulong data_size,
				  u32 offset, ulong *aligned_size)
{
	u32 overhead = get_aligned_image_overhead(info, offset);

	જો (data_size > ULONG_MAX - overhead)
		પાછા -EOVERFLOW;
	data_size += overhead;

	જો (data_size > ULONG_MAX - (spl_get_bl_len(info) - 1))
		પાછા -EOVERFLOW;
	data_size = ALIGN(data_size, spl_get_bl_len(info));

	*aligned_size = data_size;

	પાછા 0;
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
સ્થિર શૂન્ય fit_fill_image_info(અચલ શૂન્ય *fit, પૂર્ણાંક ગાંઠ,
				સંરચના spl_image_info *image_info,
				ulong load_addr, ulong size)
{
	ulong entry_point;

	જો (!image_info)
		પાછા;

	image_info->load_addr = load_addr;
	image_info->size = size;

	જો (!fit_image_get_entry(fit, ગાંઠ, &entry_point))
		image_info->entry_point = entry_point;
	નહીંતર
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
સ્થિર પૂર્ણાંક load_simple_fit(સંરચના spl_load_info *info, ulong fit_offset,
			   અચલ સંરચના spl_fit_info *ctx, પૂર્ણાંક ગાંઠ,
			   સંરચના spl_image_info *image_info, ulong max_size)
{
	u32 offset;
	u32 len;
	size_t length;
	ulong size;
	ulong load_addr;
	શૂન્ય *load_ptr;
	શૂન્ય *src;
	uint8_t image_comp = -1, type = -1;
	અચલ શૂન્ય *data;
	અચલ શૂન્ય *fit = ctx->fit;
	bool external_data = false;
	પૂર્ણાંક ret;

	log_debug("starting\n");
	જો (CONFIG_IS_ENABLED(BOOTMETH_VBE) &&
	    xpl_get_phase(info) != IH_PHASE_NONE) {
		ગણના image_phase_t phase;

		ret = fit_image_get_phase(fit, ગાંઠ, &phase);
		/* if the image is for any phase, let's use it */
		જો (ret == -ENOENT || phase == xpl_get_phase(info)) {
			log_debug("found\n");
		} નહીંતર જો (ret < 0) {
			log_debug("err=%d\n", ret);
			પાછા ret;
		} નહીંતર {
			log_debug("- phase mismatch, skipping this image\n");
			પાછા -EBADSLT;
		}
	}

	જો (IS_ENABLED(CONFIG_SPL_FPGA) ||
	    (IS_ENABLED(CONFIG_SPL_OS_BOOT) && spl_decompression_enabled())) {
		જો (fit_image_get_type(fit, ગાંઠ, &type))
			puts("Cannot get image type.\n");
		નહીંતર
			debug("%s ", genimg_get_type_name(type));
	}

	જો (spl_decompression_enabled()) {
		fit_image_get_comp(fit, ગાંઠ, &image_comp);
		debug("%s ", genimg_get_comp_name(image_comp));
	}

	જો (fit_image_get_load(fit, ગાંઠ, &load_addr)) {
		જો (!image_info->load_addr) {
			printf("Can't load %s: No load address and no buffer\n",
			       fit_get_name(fit, ગાંઠ, ખાલી));
			પાછા -ENOBUFS;
		}
		load_addr = image_info->load_addr;
	}

	જો (!fit_image_get_data_position(fit, ગાંઠ, &offset)) {
		external_data = true;
	} નહીંતર જો (!fit_image_get_data_offset(fit, ગાંઠ, &offset)) {
		/* The resulting offset cannot exceed UINT32_MAX */
		જો (ctx->ext_data_offset > UINT32_MAX - offset) {
			log_debug("Invalid external data offset: %u\n", offset);
			પાછા -EINVAL;
		}
		log_debug("read offset %x = offset from fit %x\n", offset,
			  (u32)(offset + ctx->ext_data_offset));
		offset += ctx->ext_data_offset;
		external_data = true;
	}

	જો (external_data) {
		u32 aligned_offset;
		ulong read_offset;
		શૂન્ય *src_ptr;

		/* External data */
		જો (fit_image_get_data_size(fit, ગાંઠ, &len))
			પાછા -ENOENT;

		/* Dont bother to copy 0 byte data, but warn, though */
		જો (!len) {
			log_warning("%s: Skip load '%s': image size is 0!\n",
				    __func__, fit_get_name(fit, ગાંઠ, ખાલી));
			fit_fill_image_info(fit, ગાંઠ, image_info, load_addr, 0);
			પાછા 0;
		}

		/*
		 * data-size is excluded from the configuration signature (it
		 * is in exc_prop[] in image-fit-sig.c), so it stays attacker
		 * controlled even after fit_config_verify() succeeds. The
		 * image hash is only verified after the device read below, so
		 * an oversized value has to be rejected here.
		 */
		ret = get_aligned_image_size(info, len, offset, &size);
		જો (ret) {
			log_debug("Invalid external data size: %u\n", len);
			પાછા ret;
		}

		જો (size > max_size) {
			log_debug("Image too large: aligned size %lu, max %lu (data-size %u)\n",
				  size, max_size, len);
			પાછા -EFBIG;
		}

		aligned_offset = get_aligned_image_offset(info, offset);
		જો (aligned_offset > ULONG_MAX - fit_offset) {
			log_debug("Invalid aligned external data offset: %u\n",
				  aligned_offset);
			પાછા -EINVAL;
		}
		read_offset = fit_offset + aligned_offset;

		જો (spl_decompression_enabled() &&
		    (image_comp == IH_COMP_GZIP || image_comp == IH_COMP_LZMA))
			src_ptr = map_sysmem(ALIGN(CONFIG_SYS_LOAD_ADDR, ARCH_DMA_MINALIGN), len);
		નહીંતર
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
		જો (size > ULONG_MAX - read_offset ||
		    size > UINTPTR_MAX - (uintptr_t)src_ptr
		/*
		 * #if (not a runtime if) is required: FIT_SIGNATURE_MAX_SIZE
		 * depends on FIT_SIGNATURE, so CONFIG_VAL(FIT_SIGNATURE_MAX_SIZE)
		 * is undefined when signing is disabled and referencing it
		 * here would fail to compile.
		 */
#જો CONFIG_IS_ENABLED(FIT_SIGNATURE)
		    || offset > CONFIG_VAL(FIT_SIGNATURE_MAX_SIZE) ||
		    len > CONFIG_VAL(FIT_SIGNATURE_MAX_SIZE) - offset
#endif
		) {
			log_debug("FIT external data is out of bounds (offset=%u, size=%u)\n",
				  offset, len);
			પાછા -EINVAL;
		}

		log_debug("reading from offset %x / %lx size %lx to %p: ",
			  offset, read_offset, size, src_ptr);

		જો (info->પઠ(info, read_offset, size, src_ptr) < length)
			પાછા -EIO;

		debug("External data: dst=%p, offset=%x, size=%lx\n",
		      src_ptr, offset, (unsigned long)length);
		src = src_ptr + get_aligned_image_overhead(info, offset);
	} નહીંતર {
		/* Embedded data */
		જો (fit_image_get_emb_data(fit, ગાંઠ, &data, &length)) {
			puts("Cannot get image data/size\n");
			પાછા -ENOENT;
		}
		debug("Embedded data: dst=%lx, size=%lx\n", load_addr,
		      (unsigned long)length);
		src = (શૂન્ય *)data;	/* cast away const */
	}

	જો (CONFIG_IS_ENABLED(FIT_SIGNATURE)) {
		printf("## Checking hash(es) for Image %s ... ",
		       fit_get_name(fit, ગાંઠ, ખાલી));
		જો (!fit_image_verify_with_data(fit, ગાંઠ, gd_fdt_blob(), src,
						length))
			પાછા -EPERM;
		puts("OK\n");
	}

	જો (CONFIG_IS_ENABLED(FIT_IMAGE_POST_PROCESS))
		board_fit_image_post_process(fit, ગાંઠ, &src, &length);

	load_ptr = map_sysmem(load_addr, length);
	જો (IS_ENABLED(CONFIG_SPL_GZIP) && image_comp == IH_COMP_GZIP) {
		size = length;
		જો (gunzip(load_ptr, CONFIG_SYS_BOOTM_LEN, src, &size)) {
			puts("Uncompressing error\n");
			પાછા -EIO;
		}
		length = size;
	} નહીંતર જો (IS_ENABLED(CONFIG_SPL_LZMA) && image_comp == IH_COMP_LZMA) {
		size = CONFIG_SYS_BOOTM_LEN;
		ulong loadEnd;

		જો (image_decomp(IH_COMP_LZMA, CONFIG_SYS_LOAD_ADDR, 0, 0,
				 load_ptr, src, length, size, &loadEnd)) {
			puts("Uncompressing error\n");
			પાછા -EIO;
		}
		length = loadEnd - CONFIG_SYS_LOAD_ADDR;
	} નહીંતર {
		memmove(load_ptr, src, length);
	}

	fit_fill_image_info(fit, ગાંઠ, image_info, load_addr, length);
	log_debug("- done loading\n");

	upl_add_image(fit, ગાંઠ, load_addr, length);

	પાછા 0;
}

સ્થિર bool os_takes_devicetree(uint8_t os)
{
	પસંદગી (os) {
	સ્થિતિ IH_OS_U_BOOT:
		પાછા true;
	સ્થિતિ IH_OS_LINUX:
		પાછા IS_ENABLED(CONFIG_SPL_OS_BOOT) ||
		       IS_ENABLED(CONFIG_SPL_OPENSBI);
	મૂળભૂત:
		પાછા false;
	}
}

__weak પૂર્ણાંક board_spl_fit_append_fdt_skip(અચલ અક્ષર *name)
{
	પાછા 0;	/* Do not skip */
}

સ્થિર પૂર્ણાંક spl_fit_append_fdt(સંરચના spl_image_info *spl_image,
			      સંરચના spl_load_info *info, ulong offset,
			      અચલ સંરચના spl_fit_info *ctx)
{
	સંરચના spl_image_info image_info;
	પૂર્ણાંક ગાંઠ, ret = 0, index = 0;

	/*
	 * Use the address following the image as target address for the
	 * device tree.
	 */
	image_info.load_addr = ALIGN(spl_image->load_addr + spl_image->size, 8);

	/* Figure out which device tree the board wants to use */
	ગાંઠ = spl_fit_get_image_ગાંઠ(ctx, FIT_FDT_PROP, index++);
	જો (ગાંઠ < 0) {
		size_t size;

		debug("%s: cannot find FDT node\n", __func__);

		/*
		 * U-Boot did not find a device tree inside the FIT image. Use
		 * the U-Boot device tree instead.
		 */
		જો (!gd->fdt_blob)
			પાછા ગાંઠ;

		/*
		 * Make the load-address of the FDT available for the SPL
		 * framework
		 */
		size = fdt_totalsize(gd->fdt_blob);
		spl_image->fdt_addr = map_sysmem(image_info.load_addr, size);
		memcpy(spl_image->fdt_addr, gd->fdt_blob, size);
	} નહીંતર {
		ret = load_simple_fit(info, offset, ctx, ગાંઠ, &image_info,
				      CONFIG_SYS_BOOTM_LEN);
		જો (ret < 0)
			પાછા ret;

		spl_image->fdt_addr = phys_to_virt(image_info.load_addr);
	}

	જો (CONFIG_IS_ENABLED(FIT_IMAGE_TINY))
		પાછા 0;

#જો CONFIG_IS_ENABLED(LOAD_FIT_APPLY_OVERLAY)
		શૂન્ય *tmpbuffer = ખાલી;

		માટે (; ; index++) {
			અચલ અક્ષર *str;

			ret = spl_fit_get_image_name(ctx, FIT_FDT_PROP, index, &str);
			જો (ret == -E2BIG) {
				debug("%s: No additional FDT node\n", __func__);
				ret = 0;
				તોડો;
			} નહીંતર જો (ret) {
				તોડો;
			}

			ret = board_spl_fit_append_fdt_skip(str);
			જો (ret)
				ચાલુ;

			ગાંઠ = fdt_subnode_offset(ctx->fit, ctx->images_ગાંઠ, str);
			જો (ગાંઠ < 0) {
				debug("%s: unable to find FDT node %d\n",
				      __func__, index);
				ચાલુ;
			}

			જો (!tmpbuffer) {
				/*
				 * allocate memory to store the DT overlay
				 * before it is applied. It may not be used
				 * depending on how the overlay is stored, so
				 * don't fail yet if the allocation failed.
				 */
				size_t size = CONFIG_SPL_LOAD_FIT_APPLY_OVERLAY_BUF_SZ;

				tmpbuffer = malloc_cache_aligned(size);
				જો (!tmpbuffer)
					debug("%s: unable to allocate space for overlays\n",
					      __func__);
			}
			image_info.load_addr = (ulong)tmpbuffer;
			ret = load_simple_fit(info, offset, ctx, ગાંઠ,
					      &image_info,
					      CONFIG_SPL_LOAD_FIT_APPLY_OVERLAY_BUF_SZ);
			જો (ret == -EBADSLT)
				ચાલુ;
			નહીંતર જો (ret < 0)
				તોડો;

			/* Make room in FDT for changes from the overlay */
			ret = fdt_increase_size(spl_image->fdt_addr,
						image_info.size);
			જો (ret < 0)
				તોડો;

			ret = fdt_overlay_apply_verbose(spl_image->fdt_addr,
							(શૂન્ય *)image_info.load_addr);
			જો (ret) {
				pr_err("failed to apply DT overlay %s\n",
				       fit_get_name(ctx->fit, ગાંઠ, ખાલી));
				તોડો;
			}

			debug("%s: DT overlay %s applied\n", __func__,
			      fit_get_name(ctx->fit, ગાંઠ, ખાલી));
		}
		મુક્ત(tmpbuffer);
		જો (ret)
			પાછા ret;
#endif
	/* Try to make space, so we can inject details on the loadables */
	ret = fdt_shrink_to_minimum(spl_image->fdt_addr, 8192);
	જો (ret < 0)
		પાછા ret;

	પાછા ret;
}

સ્થિર પૂર્ણાંક spl_fit_record_loadable(અચલ સંરચના spl_fit_info *ctx, પૂર્ણાંક index,
				   શૂન્ય *blob, સંરચના spl_image_info *image)
{
	પૂર્ણાંક ret = 0;
	અચલ અક્ષર *name;
	પૂર્ણાંક ગાંઠ;

	ret = spl_fit_get_image_name(ctx, FIT_LOADABLE_PROP, index, &name);
	જો (ret < 0)
		પાછા ret;

	ગાંઠ = spl_fit_get_image_ગાંઠ(ctx, FIT_LOADABLE_PROP, index);

	ret = fdt_record_loadable(blob, index, name, image->load_addr,
			image->size, image->entry_point,
			fdt_getprop(ctx->fit, ગાંઠ, FIT_TYPE_PROP, ખાલી),
			fdt_getprop(ctx->fit, ગાંઠ, FIT_OS_PROP, ખાલી),
			fdt_getprop(ctx->fit, ગાંઠ, FIT_ARCH_PROP, ખાલી));

	પાછા ret;
}

સ્થિર પૂર્ણાંક spl_fit_image_is_fpga(અચલ શૂન્ય *fit, પૂર્ણાંક ગાંઠ)
{
	અચલ અક્ષર *type;

	જો (!IS_ENABLED(CONFIG_SPL_FPGA))
		પાછા 0;

	type = fdt_getprop(fit, ગાંઠ, FIT_TYPE_PROP, ખાલી);
	જો (!type)
		પાછા 0;

	પાછા !strcmp(type, "fpga");
}

સ્થિર શૂન્ય spl_fit_image_record_arm32_optee(અચલ શૂન્ય *fit, પૂર્ણાંક ગાંઠ,
					     સંરચના spl_image_info *spl_image,
					     સંરચના spl_image_info *image_info)
{
#જો defined(CONFIG_BOOTM_OPTEE) && defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
	અચલ અક્ષર *type = fdt_getprop(fit, ગાંઠ, FIT_TYPE_PROP, ખાલી);

	જો (!type)
		પાછા;

	જો (strcmp(type, "tee"))
		પાછા;

	spl_image->optee_addr = image_info->load_addr;
#endif
}

સ્થિર પૂર્ણાંક spl_fit_image_get_os(અચલ શૂન્ય *fit, પૂર્ણાંક noffset, uint8_t *os)
{
	જો (!CONFIG_IS_ENABLED(FIT_IMAGE_TINY) || CONFIG_IS_ENABLED(OS_BOOT))
		પાછા fit_image_get_os(fit, noffset, os);

	અચલ અક્ષર *name = fdt_getprop(fit, noffset, FIT_OS_PROP, ખાલી);
	જો (!name)
		પાછા -ENOENT;

	/*
	 * We don't care what the type of the image actually is,
	 * only whether or not it is U-Boot. This saves some
	 * space by omitting the large table of OS types.
	 */
	જો (!strcmp(name, "u-boot"))
		*os = IH_OS_U_BOOT;
	નહીંતર
		*os = IH_OS_INVALID;

	પાછા 0;
}

/*
 * The purpose of the FIT load buffer is to provide a memory location that is
 * independent of the load address of any FIT component.
 */
__weak શૂન્ય *board_spl_fit_buffer_addr(ulong fit_size, પૂર્ણાંક sectors, પૂર્ણાંક bl_len)
{
	શૂન્ય *buf;
	size_t size = sectors * bl_len;

	buf = malloc_cache_aligned(size);
	જો (!buf) {
		pr_err("Could not get FIT buffer of %lu bytes\n", (ulong)size);

		જો (IS_ENABLED(CONFIG_SPL_SYS_MALLOC))
			pr_err("\tcheck CONFIG_SPL_SYS_MALLOC_SIZE\n");
		નહીંતર
			pr_err("\tcheck CONFIG_SPL_SYS_MALLOC_F_LEN\n");

		buf = spl_get_load_buffer(0, size);
	}
	પાછા buf;
}

/*
 * Weak default function to allow customizing SPL fit loading for load-only
 * use cases by allowing to skip the parsing/processing of the FIT contents
 * (so that this can be done separately in a more customized fashion)
 */
__weak bool spl_load_simple_fit_skip_processing(શૂન્ય)
{
	પાછા false;
}

/*
 * Weak default function to allow fixes after fit header
 * is loaded.
 */
__weak શૂન્ય *spl_load_simple_fit_fix_load(અચલ શૂન્ય *fit)
{
	પાછા (શૂન્ય *)fit;
}

સ્થિર શૂન્ય warn_deprecated(અચલ અક્ષર *સંદેશ)
{
	printf("DEPRECATED: %s\n", સંદેશ);
	printf("\tSee https://fitspec.osfw.foundation/\n");
}

સ્થિર પૂર્ણાંક spl_fit_upload_fpga(સંરચના spl_fit_info *ctx, પૂર્ણાંક ગાંઠ,
			       સંરચના spl_image_info *fpga_image)
{
	અચલ અક્ષર *compatible;
	પૂર્ણાંક ret;
	પૂર્ણાંક devnum = 0;
	પૂર્ણાંક flags = 0;

	debug("FPGA bitstream at: %x, size: %x\n",
	      (u32)fpga_image->load_addr, fpga_image->size);

	compatible = fdt_getprop(ctx->fit, ગાંઠ, FIT_COMPAT_PROP, ખાલી);
	જો (!compatible) {
		warn_deprecated("'fpga' image without 'compatible' property");
	} નહીંતર {
		જો (CONFIG_IS_ENABLED(FPGA_LOAD_SECURE))
			flags = fpga_compatible2flag(devnum, compatible);
		જો (strcmp(compatible, "u-boot,fpga-legacy"))
			debug("Ignoring compatible = %s property\n",
			      compatible);
	}

	ret = fpga_load(devnum, (શૂન્ય *)fpga_image->load_addr,
			fpga_image->size, BIT_FULL, flags);
	જો (ret) {
		printf("%s: Cannot load the image to the FPGA\n", __func__);
		પાછા ret;
	}

	puts("FPGA image loaded from FIT\n");

	પાછા 0;
}

સ્થિર પૂર્ણાંક spl_fit_load_fpga(સંરચના spl_fit_info *ctx,
			     સંરચના spl_load_info *info, ulong offset)
{
	પૂર્ણાંક ગાંઠ, ret;

	સંરચના spl_image_info fpga_image = {
		.load_addr = 0,
	};

	ગાંઠ = spl_fit_get_image_ગાંઠ(ctx, FIT_FPGA_PROP, 0);
	જો (ગાંઠ < 0)
		પાછા ગાંઠ;

	warn_deprecated("'fpga' property in config node. Use 'loadables'");

	/* Load the image and set up the fpga_image structure */
	ret = load_simple_fit(info, offset, ctx, ગાંઠ, &fpga_image,
			      CONFIG_SYS_BOOTM_LEN);
	જો (ret) {
		printf("%s: Cannot load the FPGA: %i\n", __func__, ret);
		પાછા ret;
	}

	પાછા spl_fit_upload_fpga(ctx, ગાંઠ, &fpga_image);
}

સ્થિર પૂર્ણાંક spl_simple_fit_read(સંરચના spl_fit_info *ctx,
			       સંરચના spl_load_info *info, ulong offset,
			       અચલ શૂન્ય *fit_header)
{
	unsigned long aligned_size, ગણતરી, size;
	શૂન્ય *buf;
	પૂર્ણાંક ret;

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
	જો (ret) {
		log_debug("Invalid FIT size: %lu\n", size);
		પાછા ret;
	}
	buf = board_spl_fit_buffer_addr(aligned_size, aligned_size, 1);
	જો (!buf) {
		/*
		 * We assume that none of the board will ever use 0x0 as a
		 * valid load address. Theoretically some board could use it,
		 * but this is extremely unlikely.
		 */
		પાછા -EIO;
	}

	ગણતરી = info->પઠ(info, offset, aligned_size, buf);
	જો (!ગણતરી) {
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
		પાછા -EIO;
	}

	ctx->fit = buf;
	debug("fit read offset %lx, size=%lu, dst=%p, count=%lu\n",
	      offset, aligned_size, buf, ગણતરી);

	પાછા 0;
}

સ્થિર પૂર્ણાંક spl_simple_fit_parse(સંરચના spl_fit_info *ctx)
{
	/* Find the correct subnode under "/configurations" */
	ctx->conf_ગાંઠ = fit_find_config_ગાંઠ(ctx->fit);
	જો (ctx->conf_ગાંઠ < 0)
		પાછા -EINVAL;

	જો (CONFIG_IS_ENABLED(FIT_SIGNATURE)) {
		printf("## Checking hash(es) for config %s ... ",
		       fit_get_name(ctx->fit, ctx->conf_ગાંઠ, ખાલી));
		જો (fit_config_verify(ctx->fit, ctx->conf_ગાંઠ))
			પાછા -EPERM;
		puts("OK\n");
	}

	/* find the node holding the images information */
	ctx->images_ગાંઠ = fdt_path_offset(ctx->fit, FIT_IMAGES_PATH);
	જો (ctx->images_ગાંઠ < 0) {
		debug("%s: Cannot find /images node: %d\n", __func__,
		      ctx->images_ગાંઠ);
		પાછા -EINVAL;
	}

	પાછા 0;
}

પૂર્ણાંક spl_load_simple_fit(સંરચના spl_image_info *spl_image,
			સંરચના spl_load_info *info, ulong offset, શૂન્ય *fit)
{
	સંરચના spl_image_info image_info;
	સંરચના spl_fit_info ctx;
	પૂર્ણાંક ગાંઠ = -1;
	પૂર્ણાંક ret;
	પૂર્ણાંક index = 0;
	પૂર્ણાંક firmware_ગાંઠ;

	ret = spl_simple_fit_read(&ctx, info, offset, fit);
	જો (ret < 0)
		પાછા ret;

	/* skip further processing if requested to enable load-only use cases */
	જો (spl_load_simple_fit_skip_processing())
		પાછા 0;

	ctx.fit = spl_load_simple_fit_fix_load(ctx.fit);

	ret = spl_simple_fit_parse(&ctx);
	જો (ret < 0)
		પાછા ret;

	જો (IS_ENABLED(CONFIG_SPL_FPGA))
		spl_fit_load_fpga(&ctx, info, offset);

	/*
	 * Find the U-Boot image using the following search order:
	 *   - start at 'firmware' (e.g. an ARM Trusted Firmware)
	 *   - fall back 'kernel' (e.g. a Falcon-mode OS boot
	 *   - fall back to using the first 'loadables' entry
	 */
	જો (ગાંઠ < 0)
		ગાંઠ = spl_fit_get_image_ગાંઠ(&ctx, FIT_FIRMWARE_PROP, 0);

	જો (ગાંઠ < 0 && IS_ENABLED(CONFIG_SPL_OS_BOOT))
		ગાંઠ = spl_fit_get_image_ગાંઠ(&ctx, FIT_KERNEL_PROP, 0);

	જો (ગાંઠ < 0) {
		debug("could not find firmware image, trying loadables...\n");
		ગાંઠ = spl_fit_get_image_ગાંઠ(&ctx, FIT_LOADABLE_PROP, 0);
		/*
		 * If we pick the U-Boot image from "loadables", start at
		 * the second image when later loading additional images.
		 */
		index = 1;
	}
	જો (ગાંઠ < 0) {
		debug("%s: Cannot find u-boot image node: %d\n",
		      __func__, ગાંઠ);
		પાછા -1;
	}

	/* Load the image and set up the spl_image structure */
	ret = load_simple_fit(info, offset, &ctx, ગાંઠ, spl_image,
			      CONFIG_SYS_BOOTM_LEN);
	જો (ret)
		પાછા ret;

	/*
	 * For backward compatibility, we treat the first node that is
	 * as a U-Boot image, if no OS-type has been declared.
	 */
	જો (!spl_fit_image_get_os(ctx.fit, ગાંઠ, &spl_image->os))
		debug("Image OS is %s\n", genimg_get_os_name(spl_image->os));
	નહીંતર જો (!IS_ENABLED(CONFIG_SPL_OS_BOOT))
		spl_image->os = IH_OS_U_BOOT;

	/*
	 * Booting a next-stage U-Boot may require us to append the FDT.
	 * We allow this to fail, as the U-Boot image might embed its FDT.
	 */
	જો (os_takes_devicetree(spl_image->os)) {
		ret = spl_fit_append_fdt(spl_image, info, offset, &ctx);
		જો (ret < 0 && spl_image->os != IH_OS_U_BOOT)
			પાછા ret;
	}

	firmware_ગાંઠ = ગાંઠ;
	/* Now check if there are more images for us to load */
	માટે (; ; index++) {
		uint8_t os_type = IH_OS_INVALID;

		ગાંઠ = spl_fit_get_image_ગાંઠ(&ctx, FIT_LOADABLE_PROP, index);
		જો (ગાંઠ < 0)
			તોડો;

		/*
		 * if the firmware is also a loadable, skip it because
		 * it already has been loaded. This is typically the case with
		 * u-boot.img generated by mkimage.
		 */
		જો (firmware_ગાંઠ == ગાંઠ)
			ચાલુ;

		image_info.load_addr = 0;
		ret = load_simple_fit(info, offset, &ctx, ગાંઠ, &image_info,
				      CONFIG_SYS_BOOTM_LEN);
		જો (ret < 0 && ret != -EBADSLT) {
			printf("%s: can't load image loadables index %d (ret = %d)\n",
			       __func__, index, ret);
			પાછા ret;
		}

		જો (spl_fit_image_is_fpga(ctx.fit, ગાંઠ))
			spl_fit_upload_fpga(&ctx, ગાંઠ, &image_info);

		જો (!spl_fit_image_get_os(ctx.fit, ગાંઠ, &os_type))
			debug("Loadable is %s\n", genimg_get_os_name(os_type));

		જો (os_takes_devicetree(os_type)) {
			spl_fit_append_fdt(&image_info, info, offset, &ctx);
			spl_image->fdt_addr = image_info.fdt_addr;
		}

		/*
		 * If the "firmware" image did not provide an entry point,
		 * use the first valid entry point from the loadables.
		 */
		જો (spl_image->entry_point == FDT_ERROR &&
		    image_info.entry_point != FDT_ERROR)
			spl_image->entry_point = image_info.entry_point;

		spl_fit_image_record_arm32_optee(ctx.fit, ગાંઠ, spl_image,
						 &image_info);

		/* Record our loadables into the FDT */
		જો (!CONFIG_IS_ENABLED(FIT_IMAGE_TINY) &&
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
	જો (spl_image->entry_point == FDT_ERROR || spl_image->entry_point == 0)
		spl_image->entry_point = spl_image->load_addr;

	spl_image->flags |= SPL_FIT_FOUND;
	upl_set_fit_info(map_to_sysmem(ctx.fit), ctx.conf_ગાંઠ,
			 spl_image->entry_point);

	પાછા 0;
}

/* Parse and load full fitImage in SPL */
પૂર્ણાંક spl_load_fit_image(સંરચના spl_image_info *spl_image,
		       અચલ સંરચના legacy_img_hdr *header)
{
	સંરચના bootm_headers images;
	અચલ અક્ષર *fit_uname_config = ખાલી;
	ulong fdt_hack;
	અચલ અક્ષર *uname;
	ulong fw_data = 0, dt_data = 0, img_data = 0;
	ulong fw_len = 0, dt_len = 0, img_len = 0;
	પૂર્ણાંક idx, conf_noffset;
	પૂર્ણાંક ret;

	images.verify = CONFIG_IS_ENABLED(FIT_SIGNATURE);

	ret = fit_image_load(&images, virt_to_phys((શૂન્ય *)header),
			     ખાલી, &fit_uname_config, IH_ARCH_DEFAULT,
			     IH_TYPE_FIRMWARE, -1, FIT_LOAD_OPTIONAL,
			     &fw_data, &fw_len);

	જો (ret < 0) {
		ret = fit_image_load(&images, virt_to_phys((શૂન્ય *)header),
				     ખાલી, &fit_uname_config, IH_ARCH_DEFAULT,
				     IH_TYPE_KERNEL, -1, FIT_LOAD_OPTIONAL,
				     &fw_data, &fw_len);
	}

	જો (ret < 0)
		પાછા ret;

	spl_image->size = fw_len;
	spl_image->load_addr = fw_data;
	જો (fit_image_get_entry(header, ret, &spl_image->entry_point))
		spl_image->entry_point = fw_data;
	જો (fit_image_get_os(header, ret, &spl_image->os))
		spl_image->os = IH_OS_INVALID;
	spl_image->name = genimg_get_os_name(spl_image->os);

	debug(PHASE_PROMPT "payload image: %32s load addr: 0x%lx size: %d\n",
	      spl_image->name, spl_image->load_addr, spl_image->size);

	images.verify = CONFIG_IS_ENABLED(FIT_SIGNATURE);

	ret = fit_image_load(&images, virt_to_phys((શૂન્ય *)header), ખાલી,
			     &fit_uname_config, IH_ARCH_DEFAULT, IH_TYPE_FLATDT,
			     -1, FIT_LOAD_OPTIONAL, &dt_data, &dt_len);
	જો (ret >= 0) {
		જો (spl_image->os == IH_OS_U_BOOT) {
			/* HACK: U-Boot expects FDT at a specific address */
			fdt_hack = ALIGN(spl_image->load_addr + spl_image->size, 8);
			debug("Relocating FDT to %p\n", (શૂન્ય *)fdt_hack);
			memcpy(map_sysmem(fdt_hack, dt_len),
			       map_sysmem(dt_data, 0), dt_len);
			spl_image->fdt_addr = (શૂન્ય *)fdt_hack;
		} નહીંતર {
			spl_image->fdt_addr = (શૂન્ય *)dt_data;
		}
	}

	conf_noffset = fit_conf_get_ગાંઠ((અચલ શૂન્ય *)header,
					 fit_uname_config);
	જો (conf_noffset < 0)
		પાછા 0;

	માટે (idx = 0;
	     uname = fdt_stringlist_get((અચલ શૂન્ય *)header, conf_noffset,
					FIT_LOADABLE_PROP, idx,
				ખાલી), uname;
	     idx++) {
		images.verify = CONFIG_IS_ENABLED(FIT_SIGNATURE);

		ret = fit_image_load(&images, virt_to_phys((શૂન્ય *)header),
				     &uname, &fit_uname_config,
				     IH_ARCH_DEFAULT, IH_TYPE_LOADABLE, -1,
				     FIT_LOAD_OPTIONAL_NON_ZERO,
				     &img_data, &img_len);
		જો (ret < 0)
			પાછા ret;
	}
	spl_image->flags |= SPL_FIT_FOUND;

	upl_set_fit_info(map_to_sysmem(header), conf_noffset,
			 spl_image->entry_point);

	પાછા 0;
}
