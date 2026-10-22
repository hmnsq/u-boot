// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#ضمن <errno.h>
#ضمن <fpga.h>
#ضمن <gzip.h>
#ضمن <image.h>
#ضمن <log.h>
#ضمن <memalign.h>
#ضمن <mapmem.h>
#ضمن <spl.h>
#ضمن <upl.h>
#ضمن <sysinfo.h>
#ضمن <asm/global_data.h>
#ضمن <asm/io.h>
#ضمن <linux/libfdt.h>
#ضمن <linux/printk.h>

DECLARE_GLOBAL_DATA_PTR;

بنية spl_fit_info {
	ثابت فراغ *fit;	/* Pointer to a valid FIT blob */
	size_t ext_data_offset;	/* Offset to FIT external data (end of FIT) */
	صحيح images_گرێ;	/* FDT offset to "/images" node */
	صحيح conf_گرێ;		/* FDT offset to selected configuration node */
};

__weak ulong board_spl_fit_size_align(ulong size)
{
	أرجع size;
}

ساكن صحيح find_گرێ_from_desc(ثابت فراغ *fit, صحيح گرێ, ثابت حرف *str)
{
	صحيح child;

	إذا (گرێ < 0)
		أرجع -EINVAL;

	/* iterate the FIT nodes and find a matching description */
	لكل (child = fdt_first_subnode(fit, گرێ); child >= 0;
	     child = fdt_next_subnode(fit, child)) {
		صحيح len;
		ثابت حرف *desc = fdt_getprop(fit, child, FIT_DESC_PROP, &len);

		إذا (!desc)
			استمر;

		إذا (!strcmp(desc, str))
			أرجع child;
	}

	أرجع -ENOENT;
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
ساكن صحيح spl_fit_get_image_name(ثابت بنية spl_fit_info *ctx,
				  ثابت حرف *type, صحيح index,
				  ثابت حرف **outname)
{
	بنية udevice *sysinfo;
	ثابت حرف *name, *str, *end;
	__maybe_unused صحيح گرێ;
	صحيح len, i;
	bool found = true;

	name = fdt_getprop(ctx->fit, ctx->conf_گرێ, type, &len);
	إذا (!name) {
		debug("cannot find property '%s': %d\n", type, len);
		أرجع -EINVAL;
	}
	/* A string property should be NUL terminated */
	end = name + len - 1;
	إذا (!len || *end) {
		debug("malformed property '%s'\n", type);
		أرجع -EINVAL;
	}

	str = name;
	لكل (i = 0; i < index; i++) {
		str = strchr(str, '\0') + 1;
		إذا (str > end) {
			found = false;
			اكسر;
		}
	}

	إذا (!found && CONFIG_IS_ENABLED(SYSINFO) && !sysinfo_get(&sysinfo)) {
		صحيح rc;
		/*
		 * no string in the property for this index. Check if the
		 * sysinfo-level code can supply one.
		 */
		rc = sysinfo_detect(sysinfo);
		إذا (rc)
			أرجع rc;

		rc = sysinfo_get_fit_loadable(sysinfo, index - i - 1, type,
					      &str);
		إذا (rc && rc != -ENOENT)
			أرجع rc;

		إذا (!rc) {
			/*
			 * The sysinfo provided a name for a loadable.
			 * Try to match it against the description properties
			 * first. If no matching node is found, use it as a
			 * node name.
			 */
			صحيح گرێ;
			صحيح images = fdt_path_offset(ctx->fit, FIT_IMAGES_PATH);

			گرێ = find_گرێ_from_desc(ctx->fit, images, str);
			إذا (گرێ > 0)
				str = fdt_get_name(ctx->fit, گرێ, عدم);

			found = true;
		}
	}

	إذا (!found) {
		debug("no string for index %d\n", index);
		أرجع -E2BIG;
	}

	*outname = str;
	أرجع 0;
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
ساكن صحيح spl_fit_get_image_گرێ(ثابت بنية spl_fit_info *ctx,
				  ثابت حرف *type, صحيح index)
{
	ثابت حرف *str;
	صحيح err;
	صحيح گرێ;

	err = spl_fit_get_image_name(ctx, type, index, &str);
	إذا (err)
		أرجع err;

	debug("%s: '%s'\n", type, str);

	گرێ = fdt_subnode_offset(ctx->fit, ctx->images_گرێ, str);
	إذا (گرێ < 0) {
		pr_err("cannot find image node '%s': %d\n", str, گرێ);
		أرجع -EINVAL;
	}

	أرجع گرێ;
}

ساكن u32 get_aligned_image_offset(بنية spl_load_info *info, u32 offset)
{
	أرجع ALIGN_DOWN(offset, spl_get_bl_len(info));
}

ساكن u32 get_aligned_image_overhead(بنية spl_load_info *info, u32 offset)
{
	أرجع offset & (spl_get_bl_len(info) - 1);
}

ساكن صحيح get_aligned_image_size(بنية spl_load_info *info, ulong data_size,
				  u32 offset, ulong *aligned_size)
{
	u32 overhead = get_aligned_image_overhead(info, offset);

	إذا (data_size > ULONG_MAX - overhead)
		أرجع -EOVERFLOW;
	data_size += overhead;

	إذا (data_size > ULONG_MAX - (spl_get_bl_len(info) - 1))
		أرجع -EOVERFLOW;
	data_size = ALIGN(data_size, spl_get_bl_len(info));

	*aligned_size = data_size;

	أرجع 0;
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
ساكن فراغ fit_fill_image_info(ثابت فراغ *fit, صحيح گرێ,
				بنية spl_image_info *image_info,
				ulong load_addr, ulong size)
{
	ulong entry_point;

	إذا (!image_info)
		أرجع;

	image_info->load_addr = load_addr;
	image_info->size = size;

	إذا (!fit_image_get_entry(fit, گرێ, &entry_point))
		image_info->entry_point = entry_point;
	وإلا
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
ساكن صحيح load_simple_fit(بنية spl_load_info *info, ulong fit_offset,
			   ثابت بنية spl_fit_info *ctx, صحيح گرێ,
			   بنية spl_image_info *image_info, ulong max_size)
{
	u32 offset;
	u32 len;
	size_t length;
	ulong size;
	ulong load_addr;
	فراغ *load_ptr;
	فراغ *src;
	uint8_t image_comp = -1, type = -1;
	ثابت فراغ *data;
	ثابت فراغ *fit = ctx->fit;
	bool external_data = false;
	صحيح ret;

	log_debug("starting\n");
	إذا (CONFIG_IS_ENABLED(BOOTMETH_VBE) &&
	    xpl_get_phase(info) != IH_PHASE_NONE) {
		تعداد image_phase_t phase;

		ret = fit_image_get_phase(fit, گرێ, &phase);
		/* if the image is for any phase, let's use it */
		إذا (ret == -ENOENT || phase == xpl_get_phase(info)) {
			log_debug("found\n");
		} وإلا إذا (ret < 0) {
			log_debug("err=%d\n", ret);
			أرجع ret;
		} وإلا {
			log_debug("- phase mismatch, skipping this image\n");
			أرجع -EBADSLT;
		}
	}

	إذا (IS_ENABLED(CONFIG_SPL_FPGA) ||
	    (IS_ENABLED(CONFIG_SPL_OS_BOOT) && spl_decompression_enabled())) {
		إذا (fit_image_get_type(fit, گرێ, &type))
			puts("Cannot get image type.\n");
		وإلا
			debug("%s ", genimg_get_type_name(type));
	}

	إذا (spl_decompression_enabled()) {
		fit_image_get_comp(fit, گرێ, &image_comp);
		debug("%s ", genimg_get_comp_name(image_comp));
	}

	إذا (fit_image_get_load(fit, گرێ, &load_addr)) {
		إذا (!image_info->load_addr) {
			printf("Can't load %s: No load address and no buffer\n",
			       fit_get_name(fit, گرێ, عدم));
			أرجع -ENOBUFS;
		}
		load_addr = image_info->load_addr;
	}

	إذا (!fit_image_get_data_position(fit, گرێ, &offset)) {
		external_data = true;
	} وإلا إذا (!fit_image_get_data_offset(fit, گرێ, &offset)) {
		/* The resulting offset cannot exceed UINT32_MAX */
		إذا (ctx->ext_data_offset > UINT32_MAX - offset) {
			log_debug("Invalid external data offset: %u\n", offset);
			أرجع -EINVAL;
		}
		log_debug("read offset %x = offset from fit %x\n", offset,
			  (u32)(offset + ctx->ext_data_offset));
		offset += ctx->ext_data_offset;
		external_data = true;
	}

	إذا (external_data) {
		u32 aligned_offset;
		ulong read_offset;
		فراغ *src_ptr;

		/* External data */
		إذا (fit_image_get_data_size(fit, گرێ, &len))
			أرجع -ENOENT;

		/* Dont bother to copy 0 byte data, but warn, though */
		إذا (!len) {
			log_warning("%s: Skip load '%s': image size is 0!\n",
				    __func__, fit_get_name(fit, گرێ, عدم));
			fit_fill_image_info(fit, گرێ, image_info, load_addr, 0);
			أرجع 0;
		}

		/*
		 * data-size is excluded from the configuration signature (it
		 * is in exc_prop[] in image-fit-sig.c), so it stays attacker
		 * controlled even after fit_config_verify() succeeds. The
		 * image hash is only verified after the device read below, so
		 * an oversized value has to be rejected here.
		 */
		ret = get_aligned_image_size(info, len, offset, &size);
		إذا (ret) {
			log_debug("Invalid external data size: %u\n", len);
			أرجع ret;
		}

		إذا (size > max_size) {
			log_debug("Image too large: aligned size %lu, max %lu (data-size %u)\n",
				  size, max_size, len);
			أرجع -EFBIG;
		}

		aligned_offset = get_aligned_image_offset(info, offset);
		إذا (aligned_offset > ULONG_MAX - fit_offset) {
			log_debug("Invalid aligned external data offset: %u\n",
				  aligned_offset);
			أرجع -EINVAL;
		}
		read_offset = fit_offset + aligned_offset;

		إذا (spl_decompression_enabled() &&
		    (image_comp == IH_COMP_GZIP || image_comp == IH_COMP_LZMA))
			src_ptr = map_sysmem(ALIGN(CONFIG_SYS_LOAD_ADDR, ARCH_DMA_MINALIGN), len);
		وإلا
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
		إذا (size > ULONG_MAX - read_offset ||
		    size > UINTPTR_MAX - (uintptr_t)src_ptr
		/*
		 * #if (not a runtime if) is required: FIT_SIGNATURE_MAX_SIZE
		 * depends on FIT_SIGNATURE, so CONFIG_VAL(FIT_SIGNATURE_MAX_SIZE)
		 * is undefined when signing is disabled and referencing it
		 * here would fail to compile.
		 */
#إذا CONFIG_IS_ENABLED(FIT_SIGNATURE)
		    || offset > CONFIG_VAL(FIT_SIGNATURE_MAX_SIZE) ||
		    len > CONFIG_VAL(FIT_SIGNATURE_MAX_SIZE) - offset
#endif
		) {
			log_debug("FIT external data is out of bounds (offset=%u, size=%u)\n",
				  offset, len);
			أرجع -EINVAL;
		}

		log_debug("reading from offset %x / %lx size %lx to %p: ",
			  offset, read_offset, size, src_ptr);

		إذا (info->اقرأ(info, read_offset, size, src_ptr) < length)
			أرجع -EIO;

		debug("External data: dst=%p, offset=%x, size=%lx\n",
		      src_ptr, offset, (unsigned long)length);
		src = src_ptr + get_aligned_image_overhead(info, offset);
	} وإلا {
		/* Embedded data */
		إذا (fit_image_get_emb_data(fit, گرێ, &data, &length)) {
			puts("Cannot get image data/size\n");
			أرجع -ENOENT;
		}
		debug("Embedded data: dst=%lx, size=%lx\n", load_addr,
		      (unsigned long)length);
		src = (فراغ *)data;	/* cast away const */
	}

	إذا (CONFIG_IS_ENABLED(FIT_SIGNATURE)) {
		printf("## Checking hash(es) for Image %s ... ",
		       fit_get_name(fit, گرێ, عدم));
		إذا (!fit_image_verify_with_data(fit, گرێ, gd_fdt_blob(), src,
						length))
			أرجع -EPERM;
		puts("OK\n");
	}

	إذا (CONFIG_IS_ENABLED(FIT_IMAGE_POST_PROCESS))
		board_fit_image_post_process(fit, گرێ, &src, &length);

	load_ptr = map_sysmem(load_addr, length);
	إذا (IS_ENABLED(CONFIG_SPL_GZIP) && image_comp == IH_COMP_GZIP) {
		size = length;
		إذا (gunzip(load_ptr, CONFIG_SYS_BOOTM_LEN, src, &size)) {
			puts("Uncompressing error\n");
			أرجع -EIO;
		}
		length = size;
	} وإلا إذا (IS_ENABLED(CONFIG_SPL_LZMA) && image_comp == IH_COMP_LZMA) {
		size = CONFIG_SYS_BOOTM_LEN;
		ulong loadEnd;

		إذا (image_decomp(IH_COMP_LZMA, CONFIG_SYS_LOAD_ADDR, 0, 0,
				 load_ptr, src, length, size, &loadEnd)) {
			puts("Uncompressing error\n");
			أرجع -EIO;
		}
		length = loadEnd - CONFIG_SYS_LOAD_ADDR;
	} وإلا {
		memmove(load_ptr, src, length);
	}

	fit_fill_image_info(fit, گرێ, image_info, load_addr, length);
	log_debug("- done loading\n");

	upl_add_image(fit, گرێ, load_addr, length);

	أرجع 0;
}

ساكن bool os_takes_devicetree(uint8_t os)
{
	بدل (os) {
	حالة IH_OS_U_BOOT:
		أرجع true;
	حالة IH_OS_LINUX:
		أرجع IS_ENABLED(CONFIG_SPL_OS_BOOT) ||
		       IS_ENABLED(CONFIG_SPL_OPENSBI);
	افتراضي:
		أرجع false;
	}
}

__weak صحيح board_spl_fit_append_fdt_skip(ثابت حرف *name)
{
	أرجع 0;	/* Do not skip */
}

ساكن صحيح spl_fit_append_fdt(بنية spl_image_info *spl_image,
			      بنية spl_load_info *info, ulong offset,
			      ثابت بنية spl_fit_info *ctx)
{
	بنية spl_image_info image_info;
	صحيح گرێ, ret = 0, index = 0;

	/*
	 * Use the address following the image as target address for the
	 * device tree.
	 */
	image_info.load_addr = ALIGN(spl_image->load_addr + spl_image->size, 8);

	/* Figure out which device tree the board wants to use */
	گرێ = spl_fit_get_image_گرێ(ctx, FIT_FDT_PROP, index++);
	إذا (گرێ < 0) {
		size_t size;

		debug("%s: cannot find FDT node\n", __func__);

		/*
		 * U-Boot did not find a device tree inside the FIT image. Use
		 * the U-Boot device tree instead.
		 */
		إذا (!gd->fdt_blob)
			أرجع گرێ;

		/*
		 * Make the load-address of the FDT available for the SPL
		 * framework
		 */
		size = fdt_totalsize(gd->fdt_blob);
		spl_image->fdt_addr = map_sysmem(image_info.load_addr, size);
		memcpy(spl_image->fdt_addr, gd->fdt_blob, size);
	} وإلا {
		ret = load_simple_fit(info, offset, ctx, گرێ, &image_info,
				      CONFIG_SYS_BOOTM_LEN);
		إذا (ret < 0)
			أرجع ret;

		spl_image->fdt_addr = phys_to_virt(image_info.load_addr);
	}

	إذا (CONFIG_IS_ENABLED(FIT_IMAGE_TINY))
		أرجع 0;

#إذا CONFIG_IS_ENABLED(LOAD_FIT_APPLY_OVERLAY)
		فراغ *tmpbuffer = عدم;

		لكل (; ; index++) {
			ثابت حرف *str;

			ret = spl_fit_get_image_name(ctx, FIT_FDT_PROP, index, &str);
			إذا (ret == -E2BIG) {
				debug("%s: No additional FDT node\n", __func__);
				ret = 0;
				اكسر;
			} وإلا إذا (ret) {
				اكسر;
			}

			ret = board_spl_fit_append_fdt_skip(str);
			إذا (ret)
				استمر;

			گرێ = fdt_subnode_offset(ctx->fit, ctx->images_گرێ, str);
			إذا (گرێ < 0) {
				debug("%s: unable to find FDT node %d\n",
				      __func__, index);
				استمر;
			}

			إذا (!tmpbuffer) {
				/*
				 * allocate memory to store the DT overlay
				 * before it is applied. It may not be used
				 * depending on how the overlay is stored, so
				 * don't fail yet if the allocation failed.
				 */
				size_t size = CONFIG_SPL_LOAD_FIT_APPLY_OVERLAY_BUF_SZ;

				tmpbuffer = malloc_cache_aligned(size);
				إذا (!tmpbuffer)
					debug("%s: unable to allocate space for overlays\n",
					      __func__);
			}
			image_info.load_addr = (ulong)tmpbuffer;
			ret = load_simple_fit(info, offset, ctx, گرێ,
					      &image_info,
					      CONFIG_SPL_LOAD_FIT_APPLY_OVERLAY_BUF_SZ);
			إذا (ret == -EBADSLT)
				استمر;
			وإلا إذا (ret < 0)
				اكسر;

			/* Make room in FDT for changes from the overlay */
			ret = fdt_increase_size(spl_image->fdt_addr,
						image_info.size);
			إذا (ret < 0)
				اكسر;

			ret = fdt_overlay_apply_verbose(spl_image->fdt_addr,
							(فراغ *)image_info.load_addr);
			إذا (ret) {
				pr_err("failed to apply DT overlay %s\n",
				       fit_get_name(ctx->fit, گرێ, عدم));
				اكسر;
			}

			debug("%s: DT overlay %s applied\n", __func__,
			      fit_get_name(ctx->fit, گرێ, عدم));
		}
		حرر(tmpbuffer);
		إذا (ret)
			أرجع ret;
#endif
	/* Try to make space, so we can inject details on the loadables */
	ret = fdt_shrink_to_minimum(spl_image->fdt_addr, 8192);
	إذا (ret < 0)
		أرجع ret;

	أرجع ret;
}

ساكن صحيح spl_fit_record_loadable(ثابت بنية spl_fit_info *ctx, صحيح index,
				   فراغ *blob, بنية spl_image_info *image)
{
	صحيح ret = 0;
	ثابت حرف *name;
	صحيح گرێ;

	ret = spl_fit_get_image_name(ctx, FIT_LOADABLE_PROP, index, &name);
	إذا (ret < 0)
		أرجع ret;

	گرێ = spl_fit_get_image_گرێ(ctx, FIT_LOADABLE_PROP, index);

	ret = fdt_record_loadable(blob, index, name, image->load_addr,
			image->size, image->entry_point,
			fdt_getprop(ctx->fit, گرێ, FIT_TYPE_PROP, عدم),
			fdt_getprop(ctx->fit, گرێ, FIT_OS_PROP, عدم),
			fdt_getprop(ctx->fit, گرێ, FIT_ARCH_PROP, عدم));

	أرجع ret;
}

ساكن صحيح spl_fit_image_is_fpga(ثابت فراغ *fit, صحيح گرێ)
{
	ثابت حرف *type;

	إذا (!IS_ENABLED(CONFIG_SPL_FPGA))
		أرجع 0;

	type = fdt_getprop(fit, گرێ, FIT_TYPE_PROP, عدم);
	إذا (!type)
		أرجع 0;

	أرجع !strcmp(type, "fpga");
}

ساكن فراغ spl_fit_image_record_arm32_optee(ثابت فراغ *fit, صحيح گرێ,
					     بنية spl_image_info *spl_image,
					     بنية spl_image_info *image_info)
{
#إذا defined(CONFIG_BOOTM_OPTEE) && defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
	ثابت حرف *type = fdt_getprop(fit, گرێ, FIT_TYPE_PROP, عدم);

	إذا (!type)
		أرجع;

	إذا (strcmp(type, "tee"))
		أرجع;

	spl_image->optee_addr = image_info->load_addr;
#endif
}

ساكن صحيح spl_fit_image_get_os(ثابت فراغ *fit, صحيح noffset, uint8_t *os)
{
	إذا (!CONFIG_IS_ENABLED(FIT_IMAGE_TINY) || CONFIG_IS_ENABLED(OS_BOOT))
		أرجع fit_image_get_os(fit, noffset, os);

	ثابت حرف *name = fdt_getprop(fit, noffset, FIT_OS_PROP, عدم);
	إذا (!name)
		أرجع -ENOENT;

	/*
	 * We don't care what the type of the image actually is,
	 * only whether or not it is U-Boot. This saves some
	 * space by omitting the large table of OS types.
	 */
	إذا (!strcmp(name, "u-boot"))
		*os = IH_OS_U_BOOT;
	وإلا
		*os = IH_OS_INVALID;

	أرجع 0;
}

/*
 * The purpose of the FIT load buffer is to provide a memory location that is
 * independent of the load address of any FIT component.
 */
__weak فراغ *board_spl_fit_buffer_addr(ulong fit_size, صحيح sectors, صحيح bl_len)
{
	فراغ *buf;
	size_t size = sectors * bl_len;

	buf = malloc_cache_aligned(size);
	إذا (!buf) {
		pr_err("Could not get FIT buffer of %lu bytes\n", (ulong)size);

		إذا (IS_ENABLED(CONFIG_SPL_SYS_MALLOC))
			pr_err("\tcheck CONFIG_SPL_SYS_MALLOC_SIZE\n");
		وإلا
			pr_err("\tcheck CONFIG_SPL_SYS_MALLOC_F_LEN\n");

		buf = spl_get_load_buffer(0, size);
	}
	أرجع buf;
}

/*
 * Weak default function to allow customizing SPL fit loading for load-only
 * use cases by allowing to skip the parsing/processing of the FIT contents
 * (so that this can be done separately in a more customized fashion)
 */
__weak bool spl_load_simple_fit_skip_processing(فراغ)
{
	أرجع false;
}

/*
 * Weak default function to allow fixes after fit header
 * is loaded.
 */
__weak فراغ *spl_load_simple_fit_fix_load(ثابت فراغ *fit)
{
	أرجع (فراغ *)fit;
}

ساكن فراغ warn_deprecated(ثابت حرف *پەیام)
{
	printf("DEPRECATED: %s\n", پەیام);
	printf("\tSee https://fitspec.osfw.foundation/\n");
}

ساكن صحيح spl_fit_upload_fpga(بنية spl_fit_info *ctx, صحيح گرێ,
			       بنية spl_image_info *fpga_image)
{
	ثابت حرف *compatible;
	صحيح ret;
	صحيح devnum = 0;
	صحيح flags = 0;

	debug("FPGA bitstream at: %x, size: %x\n",
	      (u32)fpga_image->load_addr, fpga_image->size);

	compatible = fdt_getprop(ctx->fit, گرێ, FIT_COMPAT_PROP, عدم);
	إذا (!compatible) {
		warn_deprecated("'fpga' image without 'compatible' property");
	} وإلا {
		إذا (CONFIG_IS_ENABLED(FPGA_LOAD_SECURE))
			flags = fpga_compatible2flag(devnum, compatible);
		إذا (strcmp(compatible, "u-boot,fpga-legacy"))
			debug("Ignoring compatible = %s property\n",
			      compatible);
	}

	ret = fpga_load(devnum, (فراغ *)fpga_image->load_addr,
			fpga_image->size, BIT_FULL, flags);
	إذا (ret) {
		printf("%s: Cannot load the image to the FPGA\n", __func__);
		أرجع ret;
	}

	puts("FPGA image loaded from FIT\n");

	أرجع 0;
}

ساكن صحيح spl_fit_load_fpga(بنية spl_fit_info *ctx,
			     بنية spl_load_info *info, ulong offset)
{
	صحيح گرێ, ret;

	بنية spl_image_info fpga_image = {
		.load_addr = 0,
	};

	گرێ = spl_fit_get_image_گرێ(ctx, FIT_FPGA_PROP, 0);
	إذا (گرێ < 0)
		أرجع گرێ;

	warn_deprecated("'fpga' property in config node. Use 'loadables'");

	/* Load the image and set up the fpga_image structure */
	ret = load_simple_fit(info, offset, ctx, گرێ, &fpga_image,
			      CONFIG_SYS_BOOTM_LEN);
	إذا (ret) {
		printf("%s: Cannot load the FPGA: %i\n", __func__, ret);
		أرجع ret;
	}

	أرجع spl_fit_upload_fpga(ctx, گرێ, &fpga_image);
}

ساكن صحيح spl_simple_fit_read(بنية spl_fit_info *ctx,
			       بنية spl_load_info *info, ulong offset,
			       ثابت فراغ *fit_header)
{
	unsigned long aligned_size, ژماردن, size;
	فراغ *buf;
	صحيح ret;

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
	إذا (ret) {
		log_debug("Invalid FIT size: %lu\n", size);
		أرجع ret;
	}
	buf = board_spl_fit_buffer_addr(aligned_size, aligned_size, 1);
	إذا (!buf) {
		/*
		 * We assume that none of the board will ever use 0x0 as a
		 * valid load address. Theoretically some board could use it,
		 * but this is extremely unlikely.
		 */
		أرجع -EIO;
	}

	ژماردن = info->اقرأ(info, offset, aligned_size, buf);
	إذا (!ژماردن) {
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
		أرجع -EIO;
	}

	ctx->fit = buf;
	debug("fit read offset %lx, size=%lu, dst=%p, count=%lu\n",
	      offset, aligned_size, buf, ژماردن);

	أرجع 0;
}

ساكن صحيح spl_simple_fit_parse(بنية spl_fit_info *ctx)
{
	/* Find the correct subnode under "/configurations" */
	ctx->conf_گرێ = fit_find_config_گرێ(ctx->fit);
	إذا (ctx->conf_گرێ < 0)
		أرجع -EINVAL;

	إذا (CONFIG_IS_ENABLED(FIT_SIGNATURE)) {
		printf("## Checking hash(es) for config %s ... ",
		       fit_get_name(ctx->fit, ctx->conf_گرێ, عدم));
		إذا (fit_config_verify(ctx->fit, ctx->conf_گرێ))
			أرجع -EPERM;
		puts("OK\n");
	}

	/* find the node holding the images information */
	ctx->images_گرێ = fdt_path_offset(ctx->fit, FIT_IMAGES_PATH);
	إذا (ctx->images_گرێ < 0) {
		debug("%s: Cannot find /images node: %d\n", __func__,
		      ctx->images_گرێ);
		أرجع -EINVAL;
	}

	أرجع 0;
}

صحيح spl_load_simple_fit(بنية spl_image_info *spl_image,
			بنية spl_load_info *info, ulong offset, فراغ *fit)
{
	بنية spl_image_info image_info;
	بنية spl_fit_info ctx;
	صحيح گرێ = -1;
	صحيح ret;
	صحيح index = 0;
	صحيح firmware_گرێ;

	ret = spl_simple_fit_read(&ctx, info, offset, fit);
	إذا (ret < 0)
		أرجع ret;

	/* skip further processing if requested to enable load-only use cases */
	إذا (spl_load_simple_fit_skip_processing())
		أرجع 0;

	ctx.fit = spl_load_simple_fit_fix_load(ctx.fit);

	ret = spl_simple_fit_parse(&ctx);
	إذا (ret < 0)
		أرجع ret;

	إذا (IS_ENABLED(CONFIG_SPL_FPGA))
		spl_fit_load_fpga(&ctx, info, offset);

	/*
	 * Find the U-Boot image using the following search order:
	 *   - start at 'firmware' (e.g. an ARM Trusted Firmware)
	 *   - fall back 'kernel' (e.g. a Falcon-mode OS boot
	 *   - fall back to using the first 'loadables' entry
	 */
	إذا (گرێ < 0)
		گرێ = spl_fit_get_image_گرێ(&ctx, FIT_FIRMWARE_PROP, 0);

	إذا (گرێ < 0 && IS_ENABLED(CONFIG_SPL_OS_BOOT))
		گرێ = spl_fit_get_image_گرێ(&ctx, FIT_KERNEL_PROP, 0);

	إذا (گرێ < 0) {
		debug("could not find firmware image, trying loadables...\n");
		گرێ = spl_fit_get_image_گرێ(&ctx, FIT_LOADABLE_PROP, 0);
		/*
		 * If we pick the U-Boot image from "loadables", start at
		 * the second image when later loading additional images.
		 */
		index = 1;
	}
	إذا (گرێ < 0) {
		debug("%s: Cannot find u-boot image node: %d\n",
		      __func__, گرێ);
		أرجع -1;
	}

	/* Load the image and set up the spl_image structure */
	ret = load_simple_fit(info, offset, &ctx, گرێ, spl_image,
			      CONFIG_SYS_BOOTM_LEN);
	إذا (ret)
		أرجع ret;

	/*
	 * For backward compatibility, we treat the first node that is
	 * as a U-Boot image, if no OS-type has been declared.
	 */
	إذا (!spl_fit_image_get_os(ctx.fit, گرێ, &spl_image->os))
		debug("Image OS is %s\n", genimg_get_os_name(spl_image->os));
	وإلا إذا (!IS_ENABLED(CONFIG_SPL_OS_BOOT))
		spl_image->os = IH_OS_U_BOOT;

	/*
	 * Booting a next-stage U-Boot may require us to append the FDT.
	 * We allow this to fail, as the U-Boot image might embed its FDT.
	 */
	إذا (os_takes_devicetree(spl_image->os)) {
		ret = spl_fit_append_fdt(spl_image, info, offset, &ctx);
		إذا (ret < 0 && spl_image->os != IH_OS_U_BOOT)
			أرجع ret;
	}

	firmware_گرێ = گرێ;
	/* Now check if there are more images for us to load */
	لكل (; ; index++) {
		uint8_t os_type = IH_OS_INVALID;

		گرێ = spl_fit_get_image_گرێ(&ctx, FIT_LOADABLE_PROP, index);
		إذا (گرێ < 0)
			اكسر;

		/*
		 * if the firmware is also a loadable, skip it because
		 * it already has been loaded. This is typically the case with
		 * u-boot.img generated by mkimage.
		 */
		إذا (firmware_گرێ == گرێ)
			استمر;

		image_info.load_addr = 0;
		ret = load_simple_fit(info, offset, &ctx, گرێ, &image_info,
				      CONFIG_SYS_BOOTM_LEN);
		إذا (ret < 0 && ret != -EBADSLT) {
			printf("%s: can't load image loadables index %d (ret = %d)\n",
			       __func__, index, ret);
			أرجع ret;
		}

		إذا (spl_fit_image_is_fpga(ctx.fit, گرێ))
			spl_fit_upload_fpga(&ctx, گرێ, &image_info);

		إذا (!spl_fit_image_get_os(ctx.fit, گرێ, &os_type))
			debug("Loadable is %s\n", genimg_get_os_name(os_type));

		إذا (os_takes_devicetree(os_type)) {
			spl_fit_append_fdt(&image_info, info, offset, &ctx);
			spl_image->fdt_addr = image_info.fdt_addr;
		}

		/*
		 * If the "firmware" image did not provide an entry point,
		 * use the first valid entry point from the loadables.
		 */
		إذا (spl_image->entry_point == FDT_ERROR &&
		    image_info.entry_point != FDT_ERROR)
			spl_image->entry_point = image_info.entry_point;

		spl_fit_image_record_arm32_optee(ctx.fit, گرێ, spl_image,
						 &image_info);

		/* Record our loadables into the FDT */
		إذا (!CONFIG_IS_ENABLED(FIT_IMAGE_TINY) &&
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
	إذا (spl_image->entry_point == FDT_ERROR || spl_image->entry_point == 0)
		spl_image->entry_point = spl_image->load_addr;

	spl_image->flags |= SPL_FIT_FOUND;
	upl_set_fit_info(map_to_sysmem(ctx.fit), ctx.conf_گرێ,
			 spl_image->entry_point);

	أرجع 0;
}

/* Parse and load full fitImage in SPL */
صحيح spl_load_fit_image(بنية spl_image_info *spl_image,
		       ثابت بنية legacy_img_hdr *header)
{
	بنية bootm_headers images;
	ثابت حرف *fit_uname_config = عدم;
	ulong fdt_hack;
	ثابت حرف *uname;
	ulong fw_data = 0, dt_data = 0, img_data = 0;
	ulong fw_len = 0, dt_len = 0, img_len = 0;
	صحيح idx, conf_noffset;
	صحيح ret;

	images.verify = CONFIG_IS_ENABLED(FIT_SIGNATURE);

	ret = fit_image_load(&images, virt_to_phys((فراغ *)header),
			     عدم, &fit_uname_config, IH_ARCH_DEFAULT,
			     IH_TYPE_FIRMWARE, -1, FIT_LOAD_OPTIONAL,
			     &fw_data, &fw_len);

	إذا (ret < 0) {
		ret = fit_image_load(&images, virt_to_phys((فراغ *)header),
				     عدم, &fit_uname_config, IH_ARCH_DEFAULT,
				     IH_TYPE_KERNEL, -1, FIT_LOAD_OPTIONAL,
				     &fw_data, &fw_len);
	}

	إذا (ret < 0)
		أرجع ret;

	spl_image->size = fw_len;
	spl_image->load_addr = fw_data;
	إذا (fit_image_get_entry(header, ret, &spl_image->entry_point))
		spl_image->entry_point = fw_data;
	إذا (fit_image_get_os(header, ret, &spl_image->os))
		spl_image->os = IH_OS_INVALID;
	spl_image->name = genimg_get_os_name(spl_image->os);

	debug(PHASE_PROMPT "payload image: %32s load addr: 0x%lx size: %d\n",
	      spl_image->name, spl_image->load_addr, spl_image->size);

	images.verify = CONFIG_IS_ENABLED(FIT_SIGNATURE);

	ret = fit_image_load(&images, virt_to_phys((فراغ *)header), عدم,
			     &fit_uname_config, IH_ARCH_DEFAULT, IH_TYPE_FLATDT,
			     -1, FIT_LOAD_OPTIONAL, &dt_data, &dt_len);
	إذا (ret >= 0) {
		إذا (spl_image->os == IH_OS_U_BOOT) {
			/* HACK: U-Boot expects FDT at a specific address */
			fdt_hack = ALIGN(spl_image->load_addr + spl_image->size, 8);
			debug("Relocating FDT to %p\n", (فراغ *)fdt_hack);
			memcpy(map_sysmem(fdt_hack, dt_len),
			       map_sysmem(dt_data, 0), dt_len);
			spl_image->fdt_addr = (فراغ *)fdt_hack;
		} وإلا {
			spl_image->fdt_addr = (فراغ *)dt_data;
		}
	}

	conf_noffset = fit_conf_get_گرێ((ثابت فراغ *)header,
					 fit_uname_config);
	إذا (conf_noffset < 0)
		أرجع 0;

	لكل (idx = 0;
	     uname = fdt_stringlist_get((ثابت فراغ *)header, conf_noffset,
					FIT_LOADABLE_PROP, idx,
				عدم), uname;
	     idx++) {
		images.verify = CONFIG_IS_ENABLED(FIT_SIGNATURE);

		ret = fit_image_load(&images, virt_to_phys((فراغ *)header),
				     &uname, &fit_uname_config,
				     IH_ARCH_DEFAULT, IH_TYPE_LOADABLE, -1,
				     FIT_LOAD_OPTIONAL_NON_ZERO,
				     &img_data, &img_len);
		إذا (ret < 0)
			أرجع ret;
	}
	spl_image->flags |= SPL_FIT_FOUND;

	upl_set_fit_info(map_to_sysmem(header), conf_noffset,
			 spl_image->entry_point);

	أرجع 0;
}
