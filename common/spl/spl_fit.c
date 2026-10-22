// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#شامل <errno.h>
#شامل <fpga.h>
#شامل <gzip.h>
#شامل <image.h>
#شامل <log.h>
#شامل <memalign.h>
#شامل <mapmem.h>
#شامل <spl.h>
#شامل <upl.h>
#شامل <sysinfo.h>
#شامل <asm/global_data.h>
#شامل <asm/io.h>
#شامل <linux/libfdt.h>
#شامل <linux/printk.h>

DECLARE_GLOBAL_DATA_PTR;

ساختار spl_fit_info {
	ثابت تهی *fit;	/* Pointer to a valid FIT blob */
	size_t ext_data_offset;	/* Offset to FIT external data (end of FIT) */
	صحیح images_گره;	/* FDT offset to "/images" node */
	صحیح conf_گره;		/* FDT offset to selected configuration node */
};

__weak ulong board_spl_fit_size_align(ulong size)
{
	بازگشت size;
}

ایستا صحیح find_گره_from_desc(ثابت تهی *fit, صحیح گره, ثابت نویسه *str)
{
	صحیح child;

	اگر (گره < 0)
		بازگشت -EINVAL;

	/* iterate the FIT nodes and find a matching description */
	برای (child = fdt_first_subnode(fit, گره); child >= 0;
	     child = fdt_next_subnode(fit, child)) {
		صحیح len;
		ثابت نویسه *desc = fdt_getprop(fit, child, FIT_DESC_PROP, &len);

		اگر (!desc)
			ادامه;

		اگر (!strcmp(desc, str))
			بازگشت child;
	}

	بازگشت -ENOENT;
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
ایستا صحیح spl_fit_get_image_name(ثابت ساختار spl_fit_info *ctx,
				  ثابت نویسه *type, صحیح index,
				  ثابت نویسه **outname)
{
	ساختار udevice *sysinfo;
	ثابت نویسه *name, *str, *end;
	__maybe_unused صحیح گره;
	صحیح len, i;
	bool found = true;

	name = fdt_getprop(ctx->fit, ctx->conf_گره, type, &len);
	اگر (!name) {
		debug("cannot find property '%s': %d\n", type, len);
		بازگشت -EINVAL;
	}
	/* A string property should be NUL terminated */
	end = name + len - 1;
	اگر (!len || *end) {
		debug("malformed property '%s'\n", type);
		بازگشت -EINVAL;
	}

	str = name;
	برای (i = 0; i < index; i++) {
		str = strchr(str, '\0') + 1;
		اگر (str > end) {
			found = false;
			بشکن;
		}
	}

	اگر (!found && CONFIG_IS_ENABLED(SYSINFO) && !sysinfo_get(&sysinfo)) {
		صحیح rc;
		/*
		 * no string in the property for this index. Check if the
		 * sysinfo-level code can supply one.
		 */
		rc = sysinfo_detect(sysinfo);
		اگر (rc)
			بازگشت rc;

		rc = sysinfo_get_fit_loadable(sysinfo, index - i - 1, type,
					      &str);
		اگر (rc && rc != -ENOENT)
			بازگشت rc;

		اگر (!rc) {
			/*
			 * The sysinfo provided a name for a loadable.
			 * Try to match it against the description properties
			 * first. If no matching node is found, use it as a
			 * node name.
			 */
			صحیح گره;
			صحیح images = fdt_path_offset(ctx->fit, FIT_IMAGES_PATH);

			گره = find_گره_from_desc(ctx->fit, images, str);
			اگر (گره > 0)
				str = fdt_get_name(ctx->fit, گره, پوچ);

			found = true;
		}
	}

	اگر (!found) {
		debug("no string for index %d\n", index);
		بازگشت -E2BIG;
	}

	*outname = str;
	بازگشت 0;
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
ایستا صحیح spl_fit_get_image_گره(ثابت ساختار spl_fit_info *ctx,
				  ثابت نویسه *type, صحیح index)
{
	ثابت نویسه *str;
	صحیح err;
	صحیح گره;

	err = spl_fit_get_image_name(ctx, type, index, &str);
	اگر (err)
		بازگشت err;

	debug("%s: '%s'\n", type, str);

	گره = fdt_subnode_offset(ctx->fit, ctx->images_گره, str);
	اگر (گره < 0) {
		pr_err("cannot find image node '%s': %d\n", str, گره);
		بازگشت -EINVAL;
	}

	بازگشت گره;
}

ایستا u32 get_aligned_image_offset(ساختار spl_load_info *info, u32 offset)
{
	بازگشت ALIGN_DOWN(offset, spl_get_bl_len(info));
}

ایستا u32 get_aligned_image_overhead(ساختار spl_load_info *info, u32 offset)
{
	بازگشت offset & (spl_get_bl_len(info) - 1);
}

ایستا صحیح get_aligned_image_size(ساختار spl_load_info *info, ulong data_size,
				  u32 offset, ulong *aligned_size)
{
	u32 overhead = get_aligned_image_overhead(info, offset);

	اگر (data_size > ULONG_MAX - overhead)
		بازگشت -EOVERFLOW;
	data_size += overhead;

	اگر (data_size > ULONG_MAX - (spl_get_bl_len(info) - 1))
		بازگشت -EOVERFLOW;
	data_size = ALIGN(data_size, spl_get_bl_len(info));

	*aligned_size = data_size;

	بازگشت 0;
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
ایستا تهی fit_fill_image_info(ثابت تهی *fit, صحیح گره,
				ساختار spl_image_info *image_info,
				ulong load_addr, ulong size)
{
	ulong entry_point;

	اگر (!image_info)
		بازگشت;

	image_info->load_addr = load_addr;
	image_info->size = size;

	اگر (!fit_image_get_entry(fit, گره, &entry_point))
		image_info->entry_point = entry_point;
	وگرنه
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
ایستا صحیح load_simple_fit(ساختار spl_load_info *info, ulong fit_offset,
			   ثابت ساختار spl_fit_info *ctx, صحیح گره,
			   ساختار spl_image_info *image_info, ulong max_size)
{
	u32 offset;
	u32 len;
	size_t length;
	ulong size;
	ulong load_addr;
	تهی *load_ptr;
	تهی *src;
	uint8_t image_comp = -1, type = -1;
	ثابت تهی *data;
	ثابت تهی *fit = ctx->fit;
	bool external_data = false;
	صحیح ret;

	log_debug("starting\n");
	اگر (CONFIG_IS_ENABLED(BOOTMETH_VBE) &&
	    xpl_get_phase(info) != IH_PHASE_NONE) {
		شمارش image_phase_t phase;

		ret = fit_image_get_phase(fit, گره, &phase);
		/* if the image is for any phase, let's use it */
		اگر (ret == -ENOENT || phase == xpl_get_phase(info)) {
			log_debug("found\n");
		} وگرنه اگر (ret < 0) {
			log_debug("err=%d\n", ret);
			بازگشت ret;
		} وگرنه {
			log_debug("- phase mismatch, skipping this image\n");
			بازگشت -EBADSLT;
		}
	}

	اگر (IS_ENABLED(CONFIG_SPL_FPGA) ||
	    (IS_ENABLED(CONFIG_SPL_OS_BOOT) && spl_decompression_enabled())) {
		اگر (fit_image_get_type(fit, گره, &type))
			puts("Cannot get image type.\n");
		وگرنه
			debug("%s ", genimg_get_type_name(type));
	}

	اگر (spl_decompression_enabled()) {
		fit_image_get_comp(fit, گره, &image_comp);
		debug("%s ", genimg_get_comp_name(image_comp));
	}

	اگر (fit_image_get_load(fit, گره, &load_addr)) {
		اگر (!image_info->load_addr) {
			printf("Can't load %s: No load address and no buffer\n",
			       fit_get_name(fit, گره, پوچ));
			بازگشت -ENOBUFS;
		}
		load_addr = image_info->load_addr;
	}

	اگر (!fit_image_get_data_position(fit, گره, &offset)) {
		external_data = true;
	} وگرنه اگر (!fit_image_get_data_offset(fit, گره, &offset)) {
		/* The resulting offset cannot exceed UINT32_MAX */
		اگر (ctx->ext_data_offset > UINT32_MAX - offset) {
			log_debug("Invalid external data offset: %u\n", offset);
			بازگشت -EINVAL;
		}
		log_debug("read offset %x = offset from fit %x\n", offset,
			  (u32)(offset + ctx->ext_data_offset));
		offset += ctx->ext_data_offset;
		external_data = true;
	}

	اگر (external_data) {
		u32 aligned_offset;
		ulong read_offset;
		تهی *src_ptr;

		/* External data */
		اگر (fit_image_get_data_size(fit, گره, &len))
			بازگشت -ENOENT;

		/* Dont bother to copy 0 byte data, but warn, though */
		اگر (!len) {
			log_warning("%s: Skip load '%s': image size is 0!\n",
				    __func__, fit_get_name(fit, گره, پوچ));
			fit_fill_image_info(fit, گره, image_info, load_addr, 0);
			بازگشت 0;
		}

		/*
		 * data-size is excluded from the configuration signature (it
		 * is in exc_prop[] in image-fit-sig.c), so it stays attacker
		 * controlled even after fit_config_verify() succeeds. The
		 * image hash is only verified after the device read below, so
		 * an oversized value has to be rejected here.
		 */
		ret = get_aligned_image_size(info, len, offset, &size);
		اگر (ret) {
			log_debug("Invalid external data size: %u\n", len);
			بازگشت ret;
		}

		اگر (size > max_size) {
			log_debug("Image too large: aligned size %lu, max %lu (data-size %u)\n",
				  size, max_size, len);
			بازگشت -EFBIG;
		}

		aligned_offset = get_aligned_image_offset(info, offset);
		اگر (aligned_offset > ULONG_MAX - fit_offset) {
			log_debug("Invalid aligned external data offset: %u\n",
				  aligned_offset);
			بازگشت -EINVAL;
		}
		read_offset = fit_offset + aligned_offset;

		اگر (spl_decompression_enabled() &&
		    (image_comp == IH_COMP_GZIP || image_comp == IH_COMP_LZMA))
			src_ptr = map_sysmem(ALIGN(CONFIG_SYS_LOAD_ADDR, ARCH_DMA_MINALIGN), len);
		وگرنه
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
		اگر (size > ULONG_MAX - read_offset ||
		    size > UINTPTR_MAX - (uintptr_t)src_ptr
		/*
		 * #if (not a runtime if) is required: FIT_SIGNATURE_MAX_SIZE
		 * depends on FIT_SIGNATURE, so CONFIG_VAL(FIT_SIGNATURE_MAX_SIZE)
		 * is undefined when signing is disabled and referencing it
		 * here would fail to compile.
		 */
#اگر CONFIG_IS_ENABLED(FIT_SIGNATURE)
		    || offset > CONFIG_VAL(FIT_SIGNATURE_MAX_SIZE) ||
		    len > CONFIG_VAL(FIT_SIGNATURE_MAX_SIZE) - offset
#endif
		) {
			log_debug("FIT external data is out of bounds (offset=%u, size=%u)\n",
				  offset, len);
			بازگشت -EINVAL;
		}

		log_debug("reading from offset %x / %lx size %lx to %p: ",
			  offset, read_offset, size, src_ptr);

		اگر (info->اقرأ(info, read_offset, size, src_ptr) < length)
			بازگشت -EIO;

		debug("External data: dst=%p, offset=%x, size=%lx\n",
		      src_ptr, offset, (unsigned long)length);
		src = src_ptr + get_aligned_image_overhead(info, offset);
	} وگرنه {
		/* Embedded data */
		اگر (fit_image_get_emb_data(fit, گره, &data, &length)) {
			puts("Cannot get image data/size\n");
			بازگشت -ENOENT;
		}
		debug("Embedded data: dst=%lx, size=%lx\n", load_addr,
		      (unsigned long)length);
		src = (تهی *)data;	/* cast away const */
	}

	اگر (CONFIG_IS_ENABLED(FIT_SIGNATURE)) {
		printf("## Checking hash(es) for Image %s ... ",
		       fit_get_name(fit, گره, پوچ));
		اگر (!fit_image_verify_with_data(fit, گره, gd_fdt_blob(), src,
						length))
			بازگشت -EPERM;
		puts("OK\n");
	}

	اگر (CONFIG_IS_ENABLED(FIT_IMAGE_POST_PROCESS))
		board_fit_image_post_process(fit, گره, &src, &length);

	load_ptr = map_sysmem(load_addr, length);
	اگر (IS_ENABLED(CONFIG_SPL_GZIP) && image_comp == IH_COMP_GZIP) {
		size = length;
		اگر (gunzip(load_ptr, CONFIG_SYS_BOOTM_LEN, src, &size)) {
			puts("Uncompressing error\n");
			بازگشت -EIO;
		}
		length = size;
	} وگرنه اگر (IS_ENABLED(CONFIG_SPL_LZMA) && image_comp == IH_COMP_LZMA) {
		size = CONFIG_SYS_BOOTM_LEN;
		ulong loadEnd;

		اگر (image_decomp(IH_COMP_LZMA, CONFIG_SYS_LOAD_ADDR, 0, 0,
				 load_ptr, src, length, size, &loadEnd)) {
			puts("Uncompressing error\n");
			بازگشت -EIO;
		}
		length = loadEnd - CONFIG_SYS_LOAD_ADDR;
	} وگرنه {
		memmove(load_ptr, src, length);
	}

	fit_fill_image_info(fit, گره, image_info, load_addr, length);
	log_debug("- done loading\n");

	upl_add_image(fit, گره, load_addr, length);

	بازگشت 0;
}

ایستا bool os_takes_devicetree(uint8_t os)
{
	انتخاب (os) {
	حالت IH_OS_U_BOOT:
		بازگشت true;
	حالت IH_OS_LINUX:
		بازگشت IS_ENABLED(CONFIG_SPL_OS_BOOT) ||
		       IS_ENABLED(CONFIG_SPL_OPENSBI);
	پیشفرض:
		بازگشت false;
	}
}

__weak صحیح board_spl_fit_append_fdt_skip(ثابت نویسه *name)
{
	بازگشت 0;	/* Do not skip */
}

ایستا صحیح spl_fit_append_fdt(ساختار spl_image_info *spl_image,
			      ساختار spl_load_info *info, ulong offset,
			      ثابت ساختار spl_fit_info *ctx)
{
	ساختار spl_image_info image_info;
	صحیح گره, ret = 0, index = 0;

	/*
	 * Use the address following the image as target address for the
	 * device tree.
	 */
	image_info.load_addr = ALIGN(spl_image->load_addr + spl_image->size, 8);

	/* Figure out which device tree the board wants to use */
	گره = spl_fit_get_image_گره(ctx, FIT_FDT_PROP, index++);
	اگر (گره < 0) {
		size_t size;

		debug("%s: cannot find FDT node\n", __func__);

		/*
		 * U-Boot did not find a device tree inside the FIT image. Use
		 * the U-Boot device tree instead.
		 */
		اگر (!gd->fdt_blob)
			بازگشت گره;

		/*
		 * Make the load-address of the FDT available for the SPL
		 * framework
		 */
		size = fdt_totalsize(gd->fdt_blob);
		spl_image->fdt_addr = map_sysmem(image_info.load_addr, size);
		memcpy(spl_image->fdt_addr, gd->fdt_blob, size);
	} وگرنه {
		ret = load_simple_fit(info, offset, ctx, گره, &image_info,
				      CONFIG_SYS_BOOTM_LEN);
		اگر (ret < 0)
			بازگشت ret;

		spl_image->fdt_addr = phys_to_virt(image_info.load_addr);
	}

	اگر (CONFIG_IS_ENABLED(FIT_IMAGE_TINY))
		بازگشت 0;

#اگر CONFIG_IS_ENABLED(LOAD_FIT_APPLY_OVERLAY)
		تهی *tmpbuffer = پوچ;

		برای (; ; index++) {
			ثابت نویسه *str;

			ret = spl_fit_get_image_name(ctx, FIT_FDT_PROP, index, &str);
			اگر (ret == -E2BIG) {
				debug("%s: No additional FDT node\n", __func__);
				ret = 0;
				بشکن;
			} وگرنه اگر (ret) {
				بشکن;
			}

			ret = board_spl_fit_append_fdt_skip(str);
			اگر (ret)
				ادامه;

			گره = fdt_subnode_offset(ctx->fit, ctx->images_گره, str);
			اگر (گره < 0) {
				debug("%s: unable to find FDT node %d\n",
				      __func__, index);
				ادامه;
			}

			اگر (!tmpbuffer) {
				/*
				 * allocate memory to store the DT overlay
				 * before it is applied. It may not be used
				 * depending on how the overlay is stored, so
				 * don't fail yet if the allocation failed.
				 */
				size_t size = CONFIG_SPL_LOAD_FIT_APPLY_OVERLAY_BUF_SZ;

				tmpbuffer = malloc_cache_aligned(size);
				اگر (!tmpbuffer)
					debug("%s: unable to allocate space for overlays\n",
					      __func__);
			}
			image_info.load_addr = (ulong)tmpbuffer;
			ret = load_simple_fit(info, offset, ctx, گره,
					      &image_info,
					      CONFIG_SPL_LOAD_FIT_APPLY_OVERLAY_BUF_SZ);
			اگر (ret == -EBADSLT)
				ادامه;
			وگرنه اگر (ret < 0)
				بشکن;

			/* Make room in FDT for changes from the overlay */
			ret = fdt_increase_size(spl_image->fdt_addr,
						image_info.size);
			اگر (ret < 0)
				بشکن;

			ret = fdt_overlay_apply_verbose(spl_image->fdt_addr,
							(تهی *)image_info.load_addr);
			اگر (ret) {
				pr_err("failed to apply DT overlay %s\n",
				       fit_get_name(ctx->fit, گره, پوچ));
				بشکن;
			}

			debug("%s: DT overlay %s applied\n", __func__,
			      fit_get_name(ctx->fit, گره, پوچ));
		}
		آزاد(tmpbuffer);
		اگر (ret)
			بازگشت ret;
#endif
	/* Try to make space, so we can inject details on the loadables */
	ret = fdt_shrink_to_minimum(spl_image->fdt_addr, 8192);
	اگر (ret < 0)
		بازگشت ret;

	بازگشت ret;
}

ایستا صحیح spl_fit_record_loadable(ثابت ساختار spl_fit_info *ctx, صحیح index,
				   تهی *blob, ساختار spl_image_info *image)
{
	صحیح ret = 0;
	ثابت نویسه *name;
	صحیح گره;

	ret = spl_fit_get_image_name(ctx, FIT_LOADABLE_PROP, index, &name);
	اگر (ret < 0)
		بازگشت ret;

	گره = spl_fit_get_image_گره(ctx, FIT_LOADABLE_PROP, index);

	ret = fdt_record_loadable(blob, index, name, image->load_addr,
			image->size, image->entry_point,
			fdt_getprop(ctx->fit, گره, FIT_TYPE_PROP, پوچ),
			fdt_getprop(ctx->fit, گره, FIT_OS_PROP, پوچ),
			fdt_getprop(ctx->fit, گره, FIT_ARCH_PROP, پوچ));

	بازگشت ret;
}

ایستا صحیح spl_fit_image_is_fpga(ثابت تهی *fit, صحیح گره)
{
	ثابت نویسه *type;

	اگر (!IS_ENABLED(CONFIG_SPL_FPGA))
		بازگشت 0;

	type = fdt_getprop(fit, گره, FIT_TYPE_PROP, پوچ);
	اگر (!type)
		بازگشت 0;

	بازگشت !strcmp(type, "fpga");
}

ایستا تهی spl_fit_image_record_arm32_optee(ثابت تهی *fit, صحیح گره,
					     ساختار spl_image_info *spl_image,
					     ساختار spl_image_info *image_info)
{
#اگر defined(CONFIG_BOOTM_OPTEE) && defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
	ثابت نویسه *type = fdt_getprop(fit, گره, FIT_TYPE_PROP, پوچ);

	اگر (!type)
		بازگشت;

	اگر (strcmp(type, "tee"))
		بازگشت;

	spl_image->optee_addr = image_info->load_addr;
#endif
}

ایستا صحیح spl_fit_image_get_os(ثابت تهی *fit, صحیح noffset, uint8_t *os)
{
	اگر (!CONFIG_IS_ENABLED(FIT_IMAGE_TINY) || CONFIG_IS_ENABLED(OS_BOOT))
		بازگشت fit_image_get_os(fit, noffset, os);

	ثابت نویسه *name = fdt_getprop(fit, noffset, FIT_OS_PROP, پوچ);
	اگر (!name)
		بازگشت -ENOENT;

	/*
	 * We don't care what the type of the image actually is,
	 * only whether or not it is U-Boot. This saves some
	 * space by omitting the large table of OS types.
	 */
	اگر (!strcmp(name, "u-boot"))
		*os = IH_OS_U_BOOT;
	وگرنه
		*os = IH_OS_INVALID;

	بازگشت 0;
}

/*
 * The purpose of the FIT load buffer is to provide a memory location that is
 * independent of the load address of any FIT component.
 */
__weak تهی *board_spl_fit_buffer_addr(ulong fit_size, صحیح sectors, صحیح bl_len)
{
	تهی *buf;
	size_t size = sectors * bl_len;

	buf = malloc_cache_aligned(size);
	اگر (!buf) {
		pr_err("Could not get FIT buffer of %lu bytes\n", (ulong)size);

		اگر (IS_ENABLED(CONFIG_SPL_SYS_MALLOC))
			pr_err("\tcheck CONFIG_SPL_SYS_MALLOC_SIZE\n");
		وگرنه
			pr_err("\tcheck CONFIG_SPL_SYS_MALLOC_F_LEN\n");

		buf = spl_get_load_buffer(0, size);
	}
	بازگشت buf;
}

/*
 * Weak default function to allow customizing SPL fit loading for load-only
 * use cases by allowing to skip the parsing/processing of the FIT contents
 * (so that this can be done separately in a more customized fashion)
 */
__weak bool spl_load_simple_fit_skip_processing(تهی)
{
	بازگشت false;
}

/*
 * Weak default function to allow fixes after fit header
 * is loaded.
 */
__weak تهی *spl_load_simple_fit_fix_load(ثابت تهی *fit)
{
	بازگشت (تهی *)fit;
}

ایستا تهی warn_deprecated(ثابت نویسه *پیام)
{
	printf("DEPRECATED: %s\n", پیام);
	printf("\tSee https://fitspec.osfw.foundation/\n");
}

ایستا صحیح spl_fit_upload_fpga(ساختار spl_fit_info *ctx, صحیح گره,
			       ساختار spl_image_info *fpga_image)
{
	ثابت نویسه *compatible;
	صحیح ret;
	صحیح devnum = 0;
	صحیح flags = 0;

	debug("FPGA bitstream at: %x, size: %x\n",
	      (u32)fpga_image->load_addr, fpga_image->size);

	compatible = fdt_getprop(ctx->fit, گره, FIT_COMPAT_PROP, پوچ);
	اگر (!compatible) {
		warn_deprecated("'fpga' image without 'compatible' property");
	} وگرنه {
		اگر (CONFIG_IS_ENABLED(FPGA_LOAD_SECURE))
			flags = fpga_compatible2flag(devnum, compatible);
		اگر (strcmp(compatible, "u-boot,fpga-legacy"))
			debug("Ignoring compatible = %s property\n",
			      compatible);
	}

	ret = fpga_load(devnum, (تهی *)fpga_image->load_addr,
			fpga_image->size, BIT_FULL, flags);
	اگر (ret) {
		printf("%s: Cannot load the image to the FPGA\n", __func__);
		بازگشت ret;
	}

	puts("FPGA image loaded from FIT\n");

	بازگشت 0;
}

ایستا صحیح spl_fit_load_fpga(ساختار spl_fit_info *ctx,
			     ساختار spl_load_info *info, ulong offset)
{
	صحیح گره, ret;

	ساختار spl_image_info fpga_image = {
		.load_addr = 0,
	};

	گره = spl_fit_get_image_گره(ctx, FIT_FPGA_PROP, 0);
	اگر (گره < 0)
		بازگشت گره;

	warn_deprecated("'fpga' property in config node. Use 'loadables'");

	/* Load the image and set up the fpga_image structure */
	ret = load_simple_fit(info, offset, ctx, گره, &fpga_image,
			      CONFIG_SYS_BOOTM_LEN);
	اگر (ret) {
		printf("%s: Cannot load the FPGA: %i\n", __func__, ret);
		بازگشت ret;
	}

	بازگشت spl_fit_upload_fpga(ctx, گره, &fpga_image);
}

ایستا صحیح spl_simple_fit_read(ساختار spl_fit_info *ctx,
			       ساختار spl_load_info *info, ulong offset,
			       ثابت تهی *fit_header)
{
	unsigned long aligned_size, شمار, size;
	تهی *buf;
	صحیح ret;

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
	اگر (ret) {
		log_debug("Invalid FIT size: %lu\n", size);
		بازگشت ret;
	}
	buf = board_spl_fit_buffer_addr(aligned_size, aligned_size, 1);
	اگر (!buf) {
		/*
		 * We assume that none of the board will ever use 0x0 as a
		 * valid load address. Theoretically some board could use it,
		 * but this is extremely unlikely.
		 */
		بازگشت -EIO;
	}

	شمار = info->اقرأ(info, offset, aligned_size, buf);
	اگر (!شمار) {
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
		بازگشت -EIO;
	}

	ctx->fit = buf;
	debug("fit read offset %lx, size=%lu, dst=%p, count=%lu\n",
	      offset, aligned_size, buf, شمار);

	بازگشت 0;
}

ایستا صحیح spl_simple_fit_parse(ساختار spl_fit_info *ctx)
{
	/* Find the correct subnode under "/configurations" */
	ctx->conf_گره = fit_find_config_گره(ctx->fit);
	اگر (ctx->conf_گره < 0)
		بازگشت -EINVAL;

	اگر (CONFIG_IS_ENABLED(FIT_SIGNATURE)) {
		printf("## Checking hash(es) for config %s ... ",
		       fit_get_name(ctx->fit, ctx->conf_گره, پوچ));
		اگر (fit_config_verify(ctx->fit, ctx->conf_گره))
			بازگشت -EPERM;
		puts("OK\n");
	}

	/* find the node holding the images information */
	ctx->images_گره = fdt_path_offset(ctx->fit, FIT_IMAGES_PATH);
	اگر (ctx->images_گره < 0) {
		debug("%s: Cannot find /images node: %d\n", __func__,
		      ctx->images_گره);
		بازگشت -EINVAL;
	}

	بازگشت 0;
}

صحیح spl_load_simple_fit(ساختار spl_image_info *spl_image,
			ساختار spl_load_info *info, ulong offset, تهی *fit)
{
	ساختار spl_image_info image_info;
	ساختار spl_fit_info ctx;
	صحیح گره = -1;
	صحیح ret;
	صحیح index = 0;
	صحیح firmware_گره;

	ret = spl_simple_fit_read(&ctx, info, offset, fit);
	اگر (ret < 0)
		بازگشت ret;

	/* skip further processing if requested to enable load-only use cases */
	اگر (spl_load_simple_fit_skip_processing())
		بازگشت 0;

	ctx.fit = spl_load_simple_fit_fix_load(ctx.fit);

	ret = spl_simple_fit_parse(&ctx);
	اگر (ret < 0)
		بازگشت ret;

	اگر (IS_ENABLED(CONFIG_SPL_FPGA))
		spl_fit_load_fpga(&ctx, info, offset);

	/*
	 * Find the U-Boot image using the following search order:
	 *   - start at 'firmware' (e.g. an ARM Trusted Firmware)
	 *   - fall back 'kernel' (e.g. a Falcon-mode OS boot
	 *   - fall back to using the first 'loadables' entry
	 */
	اگر (گره < 0)
		گره = spl_fit_get_image_گره(&ctx, FIT_FIRMWARE_PROP, 0);

	اگر (گره < 0 && IS_ENABLED(CONFIG_SPL_OS_BOOT))
		گره = spl_fit_get_image_گره(&ctx, FIT_KERNEL_PROP, 0);

	اگر (گره < 0) {
		debug("could not find firmware image, trying loadables...\n");
		گره = spl_fit_get_image_گره(&ctx, FIT_LOADABLE_PROP, 0);
		/*
		 * If we pick the U-Boot image from "loadables", start at
		 * the second image when later loading additional images.
		 */
		index = 1;
	}
	اگر (گره < 0) {
		debug("%s: Cannot find u-boot image node: %d\n",
		      __func__, گره);
		بازگشت -1;
	}

	/* Load the image and set up the spl_image structure */
	ret = load_simple_fit(info, offset, &ctx, گره, spl_image,
			      CONFIG_SYS_BOOTM_LEN);
	اگر (ret)
		بازگشت ret;

	/*
	 * For backward compatibility, we treat the first node that is
	 * as a U-Boot image, if no OS-type has been declared.
	 */
	اگر (!spl_fit_image_get_os(ctx.fit, گره, &spl_image->os))
		debug("Image OS is %s\n", genimg_get_os_name(spl_image->os));
	وگرنه اگر (!IS_ENABLED(CONFIG_SPL_OS_BOOT))
		spl_image->os = IH_OS_U_BOOT;

	/*
	 * Booting a next-stage U-Boot may require us to append the FDT.
	 * We allow this to fail, as the U-Boot image might embed its FDT.
	 */
	اگر (os_takes_devicetree(spl_image->os)) {
		ret = spl_fit_append_fdt(spl_image, info, offset, &ctx);
		اگر (ret < 0 && spl_image->os != IH_OS_U_BOOT)
			بازگشت ret;
	}

	firmware_گره = گره;
	/* Now check if there are more images for us to load */
	برای (; ; index++) {
		uint8_t os_type = IH_OS_INVALID;

		گره = spl_fit_get_image_گره(&ctx, FIT_LOADABLE_PROP, index);
		اگر (گره < 0)
			بشکن;

		/*
		 * if the firmware is also a loadable, skip it because
		 * it already has been loaded. This is typically the case with
		 * u-boot.img generated by mkimage.
		 */
		اگر (firmware_گره == گره)
			ادامه;

		image_info.load_addr = 0;
		ret = load_simple_fit(info, offset, &ctx, گره, &image_info,
				      CONFIG_SYS_BOOTM_LEN);
		اگر (ret < 0 && ret != -EBADSLT) {
			printf("%s: can't load image loadables index %d (ret = %d)\n",
			       __func__, index, ret);
			بازگشت ret;
		}

		اگر (spl_fit_image_is_fpga(ctx.fit, گره))
			spl_fit_upload_fpga(&ctx, گره, &image_info);

		اگر (!spl_fit_image_get_os(ctx.fit, گره, &os_type))
			debug("Loadable is %s\n", genimg_get_os_name(os_type));

		اگر (os_takes_devicetree(os_type)) {
			spl_fit_append_fdt(&image_info, info, offset, &ctx);
			spl_image->fdt_addr = image_info.fdt_addr;
		}

		/*
		 * If the "firmware" image did not provide an entry point,
		 * use the first valid entry point from the loadables.
		 */
		اگر (spl_image->entry_point == FDT_ERROR &&
		    image_info.entry_point != FDT_ERROR)
			spl_image->entry_point = image_info.entry_point;

		spl_fit_image_record_arm32_optee(ctx.fit, گره, spl_image,
						 &image_info);

		/* Record our loadables into the FDT */
		اگر (!CONFIG_IS_ENABLED(FIT_IMAGE_TINY) &&
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
	اگر (spl_image->entry_point == FDT_ERROR || spl_image->entry_point == 0)
		spl_image->entry_point = spl_image->load_addr;

	spl_image->flags |= SPL_FIT_FOUND;
	upl_set_fit_info(map_to_sysmem(ctx.fit), ctx.conf_گره,
			 spl_image->entry_point);

	بازگشت 0;
}

/* Parse and load full fitImage in SPL */
صحیح spl_load_fit_image(ساختار spl_image_info *spl_image,
		       ثابت ساختار legacy_img_hdr *header)
{
	ساختار bootm_headers images;
	ثابت نویسه *fit_uname_config = پوچ;
	ulong fdt_hack;
	ثابت نویسه *uname;
	ulong fw_data = 0, dt_data = 0, img_data = 0;
	ulong fw_len = 0, dt_len = 0, img_len = 0;
	صحیح idx, conf_noffset;
	صحیح ret;

	images.verify = CONFIG_IS_ENABLED(FIT_SIGNATURE);

	ret = fit_image_load(&images, virt_to_phys((تهی *)header),
			     پوچ, &fit_uname_config, IH_ARCH_DEFAULT,
			     IH_TYPE_FIRMWARE, -1, FIT_LOAD_OPTIONAL,
			     &fw_data, &fw_len);

	اگر (ret < 0) {
		ret = fit_image_load(&images, virt_to_phys((تهی *)header),
				     پوچ, &fit_uname_config, IH_ARCH_DEFAULT,
				     IH_TYPE_KERNEL, -1, FIT_LOAD_OPTIONAL,
				     &fw_data, &fw_len);
	}

	اگر (ret < 0)
		بازگشت ret;

	spl_image->size = fw_len;
	spl_image->load_addr = fw_data;
	اگر (fit_image_get_entry(header, ret, &spl_image->entry_point))
		spl_image->entry_point = fw_data;
	اگر (fit_image_get_os(header, ret, &spl_image->os))
		spl_image->os = IH_OS_INVALID;
	spl_image->name = genimg_get_os_name(spl_image->os);

	debug(PHASE_PROMPT "payload image: %32s load addr: 0x%lx size: %d\n",
	      spl_image->name, spl_image->load_addr, spl_image->size);

	images.verify = CONFIG_IS_ENABLED(FIT_SIGNATURE);

	ret = fit_image_load(&images, virt_to_phys((تهی *)header), پوچ,
			     &fit_uname_config, IH_ARCH_DEFAULT, IH_TYPE_FLATDT,
			     -1, FIT_LOAD_OPTIONAL, &dt_data, &dt_len);
	اگر (ret >= 0) {
		اگر (spl_image->os == IH_OS_U_BOOT) {
			/* HACK: U-Boot expects FDT at a specific address */
			fdt_hack = ALIGN(spl_image->load_addr + spl_image->size, 8);
			debug("Relocating FDT to %p\n", (تهی *)fdt_hack);
			memcpy(map_sysmem(fdt_hack, dt_len),
			       map_sysmem(dt_data, 0), dt_len);
			spl_image->fdt_addr = (تهی *)fdt_hack;
		} وگرنه {
			spl_image->fdt_addr = (تهی *)dt_data;
		}
	}

	conf_noffset = fit_conf_get_گره((ثابت تهی *)header,
					 fit_uname_config);
	اگر (conf_noffset < 0)
		بازگشت 0;

	برای (idx = 0;
	     uname = fdt_stringlist_get((ثابت تهی *)header, conf_noffset,
					FIT_LOADABLE_PROP, idx,
				پوچ), uname;
	     idx++) {
		images.verify = CONFIG_IS_ENABLED(FIT_SIGNATURE);

		ret = fit_image_load(&images, virt_to_phys((تهی *)header),
				     &uname, &fit_uname_config,
				     IH_ARCH_DEFAULT, IH_TYPE_LOADABLE, -1,
				     FIT_LOAD_OPTIONAL_NON_ZERO,
				     &img_data, &img_len);
		اگر (ret < 0)
			بازگشت ret;
	}
	spl_image->flags |= SPL_FIT_FOUND;

	upl_set_fit_info(map_to_sysmem(header), conf_noffset,
			 spl_image->entry_point);

	بازگشت 0;
}
