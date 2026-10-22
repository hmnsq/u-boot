// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#అన్తర్భావ <errno.h>
#అన్తర్భావ <fpga.h>
#అన్తర్భావ <gzip.h>
#అన్తర్భావ <image.h>
#అన్తర్భావ <log.h>
#అన్తర్భావ <memalign.h>
#అన్తర్భావ <mapmem.h>
#అన్తర్భావ <spl.h>
#అన్తర్భావ <upl.h>
#అన్తర్భావ <sysinfo.h>
#అన్తర్భావ <asm/global_data.h>
#అన్తర్భావ <asm/io.h>
#అన్తర్భావ <linux/libfdt.h>
#అన్తర్భావ <linux/printk.h>

DECLARE_GLOBAL_దత్తాంశం_PTR;

నిర్మాణం spl_fit_info {
	స్థిరరాశి శూన్య *fit;	/* Pointer to a valid FIT blob */
	size_t ext_దత్తాంశం_స్థానభ్రంశం;	/* Offset to FIT external data (end of FIT) */
	పూర్ణాంకం images_కణుపు;	/* FDT offset to "/images" node */
	పూర్ణాంకం conf_కణుపు;		/* FDT offset to selected configuration node */
};

__weak ulong board_spl_fit_కొలత_align(ulong కొలత)
{
	తిరిగి కొలత;
}

స్థిర పూర్ణాంకం కనుగొను_కణుపు_from_desc(స్థిరరాశి శూన్య *fit, పూర్ణాంకం కణుపు, స్థిరరాశి అక్షరం *str)
{
	పూర్ణాంకం బిడ్డ;

	ఒకవేళ (కణుపు < 0)
		తిరిగి -EINVAL;

	/* iterate the FIT nodes and find a matching description */
	కొరకు (బిడ్డ = fdt_మొదటి_subnode(fit, కణుపు); బిడ్డ >= 0;
	     బిడ్డ = fdt_తదుపరి_subnode(fit, బిడ్డ)) {
		పూర్ణాంకం len;
		స్థిరరాశి అక్షరం *desc = fdt_getprop(fit, బిడ్డ, FIT_DESC_PROP, &len);

		ఒకవేళ (!desc)
			కొనసాగు;

		ఒకవేళ (!strcmp(desc, str))
			తిరిగి బిడ్డ;
	}

	తిరిగి -ENOENT;
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
స్థిర పూర్ణాంకం spl_fit_get_image_name(స్థిరరాశి నిర్మాణం spl_fit_info *ctx,
				  స్థిరరాశి అక్షరం *type, పూర్ణాంకం సూచీ,
				  స్థిరరాశి అక్షరం **outname)
{
	నిర్మాణం udevice *sysinfo;
	స్థిరరాశి అక్షరం *name, *str, *చివర_2;
	__maybe_unused పూర్ణాంకం కణుపు;
	పూర్ణాంకం len, i;
	bool దొరికింది = true;

	name = fdt_getprop(ctx->fit, ctx->conf_కణుపు, type, &len);
	ఒకవేళ (!name) {
		debug("cannot find property '%s': %d\n", type, len);
		తిరిగి -EINVAL;
	}
	/* A string property should be NUL terminated */
	చివర_2 = name + len - 1;
	ఒకవేళ (!len || *చివర_2) {
		debug("malformed property '%s'\n", type);
		తిరిగి -EINVAL;
	}

	str = name;
	కొరకు (i = 0; i < సూచీ; i++) {
		str = strchr(str, '\0') + 1;
		ఒకవేళ (str > చివర_2) {
			దొరికింది = false;
			ఆపు;
		}
	}

	ఒకవేళ (!దొరికింది && CONFIG_IS_ENABLED(SYSINFO) && !sysinfo_get(&sysinfo)) {
		పూర్ణాంకం rc;
		/*
		 * no string in the property for this index. Check if the
		 * sysinfo-level code can supply one.
		 */
		rc = sysinfo_detect(sysinfo);
		ఒకవేళ (rc)
			తిరిగి rc;

		rc = sysinfo_get_fit_loadable(sysinfo, సూచీ - i - 1, type,
					      &str);
		ఒకవేళ (rc && rc != -ENOENT)
			తిరిగి rc;

		ఒకవేళ (!rc) {
			/*
			 * The sysinfo provided a name for a loadable.
			 * Try to match it against the description properties
			 * first. If no matching node is found, use it as a
			 * node name.
			 */
			పూర్ణాంకం కణుపు;
			పూర్ణాంకం images = fdt_మార్గం_స్థానభ్రంశం(ctx->fit, FIT_IMAGES_మార్గం);

			కణుపు = కనుగొను_కణుపు_from_desc(ctx->fit, images, str);
			ఒకవేళ (కణుపు > 0)
				str = fdt_get_name(ctx->fit, కణుపు, ఖాళీ);

			దొరికింది = true;
		}
	}

	ఒకవేళ (!దొరికింది) {
		debug("no string for index %d\n", సూచీ);
		తిరిగి -E2BIG;
	}

	*outname = str;
	తిరిగి 0;
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
స్థిర పూర్ణాంకం spl_fit_get_image_కణుపు(స్థిరరాశి నిర్మాణం spl_fit_info *ctx,
				  స్థిరరాశి అక్షరం *type, పూర్ణాంకం సూచీ)
{
	స్థిరరాశి అక్షరం *str;
	పూర్ణాంకం err;
	పూర్ణాంకం కణుపు;

	err = spl_fit_get_image_name(ctx, type, సూచీ, &str);
	ఒకవేళ (err)
		తిరిగి err;

	debug("%s: '%s'\n", type, str);

	కణుపు = fdt_subnode_స్థానభ్రంశం(ctx->fit, ctx->images_కణుపు, str);
	ఒకవేళ (కణుపు < 0) {
		pr_err("cannot find image node '%s': %d\n", str, కణుపు);
		తిరిగి -EINVAL;
	}

	తిరిగి కణుపు;
}

స్థిర u32 get_aligned_image_స్థానభ్రంశం(నిర్మాణం spl_ఎక్కించు_info *info, u32 స్థానభ్రంశం)
{
	తిరిగి ALIGN_కిందికి(స్థానభ్రంశం, spl_get_bl_len(info));
}

స్థిర u32 get_aligned_image_overhead(నిర్మాణం spl_ఎక్కించు_info *info, u32 స్థానభ్రంశం)
{
	తిరిగి స్థానభ్రంశం & (spl_get_bl_len(info) - 1);
}

స్థిర పూర్ణాంకం get_aligned_image_కొలత(నిర్మాణం spl_ఎక్కించు_info *info, ulong దత్తాంశం_కొలత,
				  u32 స్థానభ్రంశం, ulong *aligned_కొలత)
{
	u32 overhead = get_aligned_image_overhead(info, స్థానభ్రంశం);

	ఒకవేళ (దత్తాంశం_కొలత > ULONG_MAX - overhead)
		తిరిగి -EOVERFLOW;
	దత్తాంశం_కొలత += overhead;

	ఒకవేళ (దత్తాంశం_కొలత > ULONG_MAX - (spl_get_bl_len(info) - 1))
		తిరిగి -EOVERFLOW;
	దత్తాంశం_కొలత = ALIGN(దత్తాంశం_కొలత, spl_get_bl_len(info));

	*aligned_కొలత = దత్తాంశం_కొలత;

	తిరిగి 0;
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
స్థిర శూన్య fit_fill_image_info(స్థిరరాశి శూన్య *fit, పూర్ణాంకం కణుపు,
				నిర్మాణం spl_image_info *image_info,
				ulong ఎక్కించు_addr, ulong కొలత)
{
	ulong నమోదు_బిందువు;

	ఒకవేళ (!image_info)
		తిరిగి;

	image_info->ఎక్కించు_addr = ఎక్కించు_addr;
	image_info->కొలత = కొలత;

	ఒకవేళ (!fit_image_get_నమోదు(fit, కణుపు, &నమోదు_బిందువు))
		image_info->నమోదు_బిందువు = నమోదు_బిందువు;
	లేకపోతే
		image_info->నమోదు_బిందువు = FDT_ERROR;
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
స్థిర పూర్ణాంకం ఎక్కించు_simple_fit(నిర్మాణం spl_ఎక్కించు_info *info, ulong fit_స్థానభ్రంశం,
			   స్థిరరాశి నిర్మాణం spl_fit_info *ctx, పూర్ణాంకం కణుపు,
			   నిర్మాణం spl_image_info *image_info, ulong max_కొలత)
{
	u32 స్థానభ్రంశం;
	u32 len;
	size_t నిడివి;
	ulong కొలత;
	ulong ఎక్కించు_addr;
	శూన్య *ఎక్కించు_ptr;
	శూన్య *src;
	uint8_t image_comp = -1, type = -1;
	స్థిరరాశి శూన్య *దత్తాంశం;
	స్థిరరాశి శూన్య *fit = ctx->fit;
	bool external_దత్తాంశం = false;
	పూర్ణాంకం ret;

	log_debug("starting\n");
	ఒకవేళ (CONFIG_IS_ENABLED(BOOTMETH_VBE) &&
	    xpl_get_phase(info) != IH_PHASE_NONE) {
		గణన image_phase_t phase;

		ret = fit_image_get_phase(fit, కణుపు, &phase);
		/* if the image is for any phase, let's use it */
		ఒకవేళ (ret == -ENOENT || phase == xpl_get_phase(info)) {
			log_debug("found\n");
		} లేకపోతే ఒకవేళ (ret < 0) {
			log_debug("err=%d\n", ret);
			తిరిగి ret;
		} లేకపోతే {
			log_debug("- phase mismatch, skipping this image\n");
			తిరిగి -EBADSLT;
		}
	}

	ఒకవేళ (IS_ENABLED(CONFIG_SPL_FPGA) ||
	    (IS_ENABLED(CONFIG_SPL_OS_BOOT) && spl_decompression_enabled())) {
		ఒకవేళ (fit_image_get_type(fit, కణుపు, &type))
			puts("Cannot get image type.\n");
		లేకపోతే
			debug("%s ", genimg_get_type_name(type));
	}

	ఒకవేళ (spl_decompression_enabled()) {
		fit_image_get_comp(fit, కణుపు, &image_comp);
		debug("%s ", genimg_get_comp_name(image_comp));
	}

	ఒకవేళ (fit_image_get_ఎక్కించు(fit, కణుపు, &ఎక్కించు_addr)) {
		ఒకవేళ (!image_info->ఎక్కించు_addr) {
			రాయి("Can't load %s: No load address and no buffer\n",
			       fit_get_name(fit, కణుపు, ఖాళీ));
			తిరిగి -ENOBUFS;
		}
		ఎక్కించు_addr = image_info->ఎక్కించు_addr;
	}

	ఒకవేళ (!fit_image_get_దత్తాంశం_position(fit, కణుపు, &స్థానభ్రంశం)) {
		external_దత్తాంశం = true;
	} లేకపోతే ఒకవేళ (!fit_image_get_దత్తాంశం_స్థానభ్రంశం(fit, కణుపు, &స్థానభ్రంశం)) {
		/* The resulting offset cannot exceed UINT32_MAX */
		ఒకవేళ (ctx->ext_దత్తాంశం_స్థానభ్రంశం > UINT32_MAX - స్థానభ్రంశం) {
			log_debug("Invalid external data offset: %u\n", స్థానభ్రంశం);
			తిరిగి -EINVAL;
		}
		log_debug("read offset %x = offset from fit %x\n", స్థానభ్రంశం,
			  (u32)(స్థానభ్రంశం + ctx->ext_దత్తాంశం_స్థానభ్రంశం));
		స్థానభ్రంశం += ctx->ext_దత్తాంశం_స్థానభ్రంశం;
		external_దత్తాంశం = true;
	}

	ఒకవేళ (external_దత్తాంశం) {
		u32 aligned_స్థానభ్రంశం;
		ulong read_స్థానభ్రంశం;
		శూన్య *src_ptr;

		/* External data */
		ఒకవేళ (fit_image_get_దత్తాంశం_కొలత(fit, కణుపు, &len))
			తిరిగి -ENOENT;

		/* Dont bother to copy 0 byte data, but warn, though */
		ఒకవేళ (!len) {
			log_warning("%s: Skip load '%s': image size is 0!\n",
				    __func__, fit_get_name(fit, కణుపు, ఖాళీ));
			fit_fill_image_info(fit, కణుపు, image_info, ఎక్కించు_addr, 0);
			తిరిగి 0;
		}

		/*
		 * data-size is excluded from the configuration signature (it
		 * is in exc_prop[] in image-fit-sig.c), so it stays attacker
		 * controlled even after fit_config_verify() succeeds. The
		 * image hash is only verified after the device read below, so
		 * an oversized value has to be rejected here.
		 */
		ret = get_aligned_image_కొలత(info, len, స్థానభ్రంశం, &కొలత);
		ఒకవేళ (ret) {
			log_debug("Invalid external data size: %u\n", len);
			తిరిగి ret;
		}

		ఒకవేళ (కొలత > max_కొలత) {
			log_debug("Image too large: aligned size %lu, max %lu (data-size %u)\n",
				  కొలత, max_కొలత, len);
			తిరిగి -EFBIG;
		}

		aligned_స్థానభ్రంశం = get_aligned_image_స్థానభ్రంశం(info, స్థానభ్రంశం);
		ఒకవేళ (aligned_స్థానభ్రంశం > ULONG_MAX - fit_స్థానభ్రంశం) {
			log_debug("Invalid aligned external data offset: %u\n",
				  aligned_స్థానభ్రంశం);
			తిరిగి -EINVAL;
		}
		read_స్థానభ్రంశం = fit_స్థానభ్రంశం + aligned_స్థానభ్రంశం;

		ఒకవేళ (spl_decompression_enabled() &&
		    (image_comp == IH_COMP_GZIP || image_comp == IH_COMP_LZMA))
			src_ptr = map_sysmem(ALIGN(CONFIG_SYS_ఎక్కించు_ADDR, ARCH_DMA_MINALIGN), len);
		లేకపోతే
			src_ptr = map_sysmem(ALIGN(ఎక్కించు_addr, ARCH_DMA_MINALIGN), len);
		నిడివి = len;

		/*
		 * For non-signed FIT images, we can check that
		 * (read_offset + size) does not wrap and that
		 * (src_ptr + size) does not exceed the addressable range.
		 * For signed FITs, we can additionally check that
		 * (offset + len) doesn't exceed the allowed FIT image
		 * maximum size.
		 */
		ఒకవేళ (కొలత > ULONG_MAX - read_స్థానభ్రంశం ||
		    కొలత > UINTPTR_MAX - (uintptr_t)src_ptr
		/*
		 * #if (not a runtime if) is required: FIT_SIGNATURE_MAX_SIZE
		 * depends on FIT_SIGNATURE, so CONFIG_VAL(FIT_SIGNATURE_MAX_SIZE)
		 * is undefined when signing is disabled and referencing it
		 * here would fail to compile.
		 */
#ఒకవేళ CONFIG_IS_ENABLED(FIT_SIGNATURE)
		    || స్థానభ్రంశం > CONFIG_VAL(FIT_SIGNATURE_MAX_కొలత) ||
		    len > CONFIG_VAL(FIT_SIGNATURE_MAX_కొలత) - స్థానభ్రంశం
#అయితే_ముగింపు
		) {
			log_debug("FIT external data is out of bounds (offset=%u, size=%u)\n",
				  స్థానభ్రంశం, len);
			తిరిగి -EINVAL;
		}

		log_debug("reading from offset %x / %lx size %lx to %p: ",
			  స్థానభ్రంశం, read_స్థానభ్రంశం, కొలత, src_ptr);

		ఒకవేళ (info->పఠ(info, read_స్థానభ్రంశం, కొలత, src_ptr) < నిడివి)
			తిరిగి -EIO;

		debug("External data: dst=%p, offset=%x, size=%lx\n",
		      src_ptr, స్థానభ్రంశం, (గుర్తులేని దీర్ఘ)నిడివి);
		src = src_ptr + get_aligned_image_overhead(info, స్థానభ్రంశం);
	} లేకపోతే {
		/* Embedded data */
		ఒకవేళ (fit_image_get_emb_దత్తాంశం(fit, కణుపు, &దత్తాంశం, &నిడివి)) {
			puts("Cannot get image data/size\n");
			తిరిగి -ENOENT;
		}
		debug("Embedded data: dst=%lx, size=%lx\n", ఎక్కించు_addr,
		      (గుర్తులేని దీర్ఘ)నిడివి);
		src = (శూన్య *)దత్తాంశం;	/* cast away const */
	}

	ఒకవేళ (CONFIG_IS_ENABLED(FIT_SIGNATURE)) {
		రాయి("## Checking hash(es) for Image %s ... ",
		       fit_get_name(fit, కణుపు, ఖాళీ));
		ఒకవేళ (!fit_image_verify_with_దత్తాంశం(fit, కణుపు, gd_fdt_blob(), src,
						నిడివి))
			తిరిగి -EPERM;
		puts("OK\n");
	}

	ఒకవేళ (CONFIG_IS_ENABLED(FIT_IMAGE_POST_PROCESS))
		board_fit_image_post_process(fit, కణుపు, &src, &నిడివి);

	ఎక్కించు_ptr = map_sysmem(ఎక్కించు_addr, నిడివి);
	ఒకవేళ (IS_ENABLED(CONFIG_SPL_GZIP) && image_comp == IH_COMP_GZIP) {
		కొలత = నిడివి;
		ఒకవేళ (gunzip(ఎక్కించు_ptr, CONFIG_SYS_BOOTM_LEN, src, &కొలత)) {
			puts("Uncompressing error\n");
			తిరిగి -EIO;
		}
		నిడివి = కొలత;
	} లేకపోతే ఒకవేళ (IS_ENABLED(CONFIG_SPL_LZMA) && image_comp == IH_COMP_LZMA) {
		కొలత = CONFIG_SYS_BOOTM_LEN;
		ulong ఎక్కించు_చివర;

		ఒకవేళ (image_decomp(IH_COMP_LZMA, CONFIG_SYS_ఎక్కించు_ADDR, 0, 0,
				 ఎక్కించు_ptr, src, నిడివి, కొలత, &ఎక్కించు_చివర)) {
			puts("Uncompressing error\n");
			తిరిగి -EIO;
		}
		నిడివి = ఎక్కించు_చివర - CONFIG_SYS_ఎక్కించు_ADDR;
	} లేకపోతే {
		memmove(ఎక్కించు_ptr, src, నిడివి);
	}

	fit_fill_image_info(fit, కణుపు, image_info, ఎక్కించు_addr, నిడివి);
	log_debug("- done loading\n");

	upl_కూడు_image(fit, కణుపు, ఎక్కించు_addr, నిడివి);

	తిరిగి 0;
}

స్థిర bool os_takes_devicetree(uint8_t os)
{
	ఎంపిక (os) {
	స్థితి IH_OS_U_BOOT:
		తిరిగి true;
	స్థితి IH_OS_LINUX:
		తిరిగి IS_ENABLED(CONFIG_SPL_OS_BOOT) ||
		       IS_ENABLED(CONFIG_SPL_OPENSBI);
	అప్రమేయ:
		తిరిగి false;
	}
}

__weak పూర్ణాంకం board_spl_fit_append_fdt_skip(స్థిరరాశి అక్షరం *name)
{
	తిరిగి 0;	/* Do not skip */
}

స్థిర పూర్ణాంకం spl_fit_append_fdt(నిర్మాణం spl_image_info *spl_image,
			      నిర్మాణం spl_ఎక్కించు_info *info, ulong స్థానభ్రంశం,
			      స్థిరరాశి నిర్మాణం spl_fit_info *ctx)
{
	నిర్మాణం spl_image_info image_info;
	పూర్ణాంకం కణుపు, ret = 0, సూచీ = 0;

	/*
	 * Use the address following the image as target address for the
	 * device tree.
	 */
	image_info.ఎక్కించు_addr = ALIGN(spl_image->ఎక్కించు_addr + spl_image->కొలత, 8);

	/* Figure out which device tree the board wants to use */
	కణుపు = spl_fit_get_image_కణుపు(ctx, FIT_FDT_PROP, సూచీ++);
	ఒకవేళ (కణుపు < 0) {
		size_t కొలత;

		debug("%s: cannot find FDT node\n", __func__);

		/*
		 * U-Boot did not find a device tree inside the FIT image. Use
		 * the U-Boot device tree instead.
		 */
		ఒకవేళ (!gd->fdt_blob)
			తిరిగి కణుపు;

		/*
		 * Make the load-address of the FDT available for the SPL
		 * framework
		 */
		కొలత = fdt_totalsize(gd->fdt_blob);
		spl_image->fdt_addr = map_sysmem(image_info.ఎక్కించు_addr, కొలత);
		జ్ఞాపకం_నకలు(spl_image->fdt_addr, gd->fdt_blob, కొలత);
	} లేకపోతే {
		ret = ఎక్కించు_simple_fit(info, స్థానభ్రంశం, ctx, కణుపు, &image_info,
				      CONFIG_SYS_BOOTM_LEN);
		ఒకవేళ (ret < 0)
			తిరిగి ret;

		spl_image->fdt_addr = phys_గమ్యం_virt(image_info.ఎక్కించు_addr);
	}

	ఒకవేళ (CONFIG_IS_ENABLED(FIT_IMAGE_TINY))
		తిరిగి 0;

#ఒకవేళ CONFIG_IS_ENABLED(ఎక్కించు_FIT_APPLY_OVERLAY)
		శూన్య *tmpbuffer = ఖాళీ;

		కొరకు (; ; సూచీ++) {
			స్థిరరాశి అక్షరం *str;

			ret = spl_fit_get_image_name(ctx, FIT_FDT_PROP, సూచీ, &str);
			ఒకవేళ (ret == -E2BIG) {
				debug("%s: No additional FDT node\n", __func__);
				ret = 0;
				ఆపు;
			} లేకపోతే ఒకవేళ (ret) {
				ఆపు;
			}

			ret = board_spl_fit_append_fdt_skip(str);
			ఒకవేళ (ret)
				కొనసాగు;

			కణుపు = fdt_subnode_స్థానభ్రంశం(ctx->fit, ctx->images_కణుపు, str);
			ఒకవేళ (కణుపు < 0) {
				debug("%s: unable to find FDT node %d\n",
				      __func__, సూచీ);
				కొనసాగు;
			}

			ఒకవేళ (!tmpbuffer) {
				/*
				 * allocate memory to store the DT overlay
				 * before it is applied. It may not be used
				 * depending on how the overlay is stored, so
				 * don't fail yet if the allocation failed.
				 */
				size_t కొలత = CONFIG_SPL_ఎక్కించు_FIT_APPLY_OVERLAY_BUF_SZ;

				tmpbuffer = malloc_cache_aligned(కొలత);
				ఒకవేళ (!tmpbuffer)
					debug("%s: unable to allocate space for overlays\n",
					      __func__);
			}
			image_info.ఎక్కించు_addr = (ulong)tmpbuffer;
			ret = ఎక్కించు_simple_fit(info, స్థానభ్రంశం, ctx, కణుపు,
					      &image_info,
					      CONFIG_SPL_ఎక్కించు_FIT_APPLY_OVERLAY_BUF_SZ);
			ఒకవేళ (ret == -EBADSLT)
				కొనసాగు;
			లేకపోతే ఒకవేళ (ret < 0)
				ఆపు;

			/* Make room in FDT for changes from the overlay */
			ret = fdt_increase_కొలత(spl_image->fdt_addr,
						image_info.కొలత);
			ఒకవేళ (ret < 0)
				ఆపు;

			ret = fdt_overlay_apply_verbose(spl_image->fdt_addr,
							(శూన్య *)image_info.ఎక్కించు_addr);
			ఒకవేళ (ret) {
				pr_err("failed to apply DT overlay %s\n",
				       fit_get_name(ctx->fit, కణుపు, ఖాళీ));
				ఆపు;
			}

			debug("%s: DT overlay %s applied\n", __func__,
			      fit_get_name(ctx->fit, కణుపు, ఖాళీ));
		}
		విడుదల(tmpbuffer);
		ఒకవేళ (ret)
			తిరిగి ret;
#అయితే_ముగింపు
	/* Try to make space, so we can inject details on the loadables */
	ret = fdt_shrink_గమ్యం_minimum(spl_image->fdt_addr, 8192);
	ఒకవేళ (ret < 0)
		తిరిగి ret;

	తిరిగి ret;
}

స్థిర పూర్ణాంకం spl_fit_రికార్డు_loadable(స్థిరరాశి నిర్మాణం spl_fit_info *ctx, పూర్ణాంకం సూచీ,
				   శూన్య *blob, నిర్మాణం spl_image_info *image)
{
	పూర్ణాంకం ret = 0;
	స్థిరరాశి అక్షరం *name;
	పూర్ణాంకం కణుపు;

	ret = spl_fit_get_image_name(ctx, FIT_LOADABLE_PROP, సూచీ, &name);
	ఒకవేళ (ret < 0)
		తిరిగి ret;

	కణుపు = spl_fit_get_image_కణుపు(ctx, FIT_LOADABLE_PROP, సూచీ);

	ret = fdt_రికార్డు_loadable(blob, సూచీ, name, image->ఎక్కించు_addr,
			image->కొలత, image->నమోదు_బిందువు,
			fdt_getprop(ctx->fit, కణుపు, FIT_TYPE_PROP, ఖాళీ),
			fdt_getprop(ctx->fit, కణుపు, FIT_OS_PROP, ఖాళీ),
			fdt_getprop(ctx->fit, కణుపు, FIT_ARCH_PROP, ఖాళీ));

	తిరిగి ret;
}

స్థిర పూర్ణాంకం spl_fit_image_is_fpga(స్థిరరాశి శూన్య *fit, పూర్ణాంకం కణుపు)
{
	స్థిరరాశి అక్షరం *type;

	ఒకవేళ (!IS_ENABLED(CONFIG_SPL_FPGA))
		తిరిగి 0;

	type = fdt_getprop(fit, కణుపు, FIT_TYPE_PROP, ఖాళీ);
	ఒకవేళ (!type)
		తిరిగి 0;

	తిరిగి !strcmp(type, "fpga");
}

స్థిర శూన్య spl_fit_image_రికార్డు_arm32_optee(స్థిరరాశి శూన్య *fit, పూర్ణాంకం కణుపు,
					     నిర్మాణం spl_image_info *spl_image,
					     నిర్మాణం spl_image_info *image_info)
{
#ఒకవేళ defined(CONFIG_BOOTM_OPTEE) && defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
	స్థిరరాశి అక్షరం *type = fdt_getprop(fit, కణుపు, FIT_TYPE_PROP, ఖాళీ);

	ఒకవేళ (!type)
		తిరిగి;

	ఒకవేళ (strcmp(type, "tee"))
		తిరిగి;

	spl_image->optee_addr = image_info->ఎక్కించు_addr;
#అయితే_ముగింపు
}

స్థిర పూర్ణాంకం spl_fit_image_get_os(స్థిరరాశి శూన్య *fit, పూర్ణాంకం noffset, uint8_t *os)
{
	ఒకవేళ (!CONFIG_IS_ENABLED(FIT_IMAGE_TINY) || CONFIG_IS_ENABLED(OS_BOOT))
		తిరిగి fit_image_get_os(fit, noffset, os);

	స్థిరరాశి అక్షరం *name = fdt_getprop(fit, noffset, FIT_OS_PROP, ఖాళీ);
	ఒకవేళ (!name)
		తిరిగి -ENOENT;

	/*
	 * We don't care what the type of the image actually is,
	 * only whether or not it is U-Boot. This saves some
	 * space by omitting the large table of OS types.
	 */
	ఒకవేళ (!strcmp(name, "u-boot"))
		*os = IH_OS_U_BOOT;
	లేకపోతే
		*os = IH_OS_INVALID;

	తిరిగి 0;
}

/*
 * The purpose of the FIT load buffer is to provide a memory location that is
 * independent of the load address of any FIT component.
 */
__weak శూన్య *board_spl_fit_బఫరు_addr(ulong fit_కొలత, పూర్ణాంకం sectors, పూర్ణాంకం bl_len)
{
	శూన్య *buf;
	size_t కొలత = sectors * bl_len;

	buf = malloc_cache_aligned(కొలత);
	ఒకవేళ (!buf) {
		pr_err("Could not get FIT buffer of %lu bytes\n", (ulong)కొలత);

		ఒకవేళ (IS_ENABLED(CONFIG_SPL_SYS_MALLOC))
			pr_err("\tcheck CONFIG_SPL_SYS_MALLOC_SIZE\n");
		లేకపోతే
			pr_err("\tcheck CONFIG_SPL_SYS_MALLOC_F_LEN\n");

		buf = spl_get_ఎక్కించు_బఫరు(0, కొలత);
	}
	తిరిగి buf;
}

/*
 * Weak default function to allow customizing SPL fit loading for load-only
 * use cases by allowing to skip the parsing/processing of the FIT contents
 * (so that this can be done separately in a more customized fashion)
 */
__weak bool spl_ఎక్కించు_simple_fit_skip_processing(శూన్య)
{
	తిరిగి false;
}

/*
 * Weak default function to allow fixes after fit header
 * is loaded.
 */
__weak శూన్య *spl_ఎక్కించు_simple_fit_fix_ఎక్కించు(స్థిరరాశి శూన్య *fit)
{
	తిరిగి (శూన్య *)fit;
}

స్థిర శూన్య warn_deprecated(స్థిరరాశి అక్షరం *సందేశం)
{
	రాయి("DEPRECATED: %s\n", సందేశం);
	రాయి("\tSee https://fitspec.osfw.foundation/\n");
}

స్థిర పూర్ణాంకం spl_fit_upload_fpga(నిర్మాణం spl_fit_info *ctx, పూర్ణాంకం కణుపు,
			       నిర్మాణం spl_image_info *fpga_image)
{
	స్థిరరాశి అక్షరం *compatible;
	పూర్ణాంకం ret;
	పూర్ణాంకం devnum = 0;
	పూర్ణాంకం flags = 0;

	debug("FPGA bitstream at: %x, size: %x\n",
	      (u32)fpga_image->ఎక్కించు_addr, fpga_image->కొలత);

	compatible = fdt_getprop(ctx->fit, కణుపు, FIT_COMPAT_PROP, ఖాళీ);
	ఒకవేళ (!compatible) {
		warn_deprecated("'fpga' image without 'compatible' property");
	} లేకపోతే {
		ఒకవేళ (CONFIG_IS_ENABLED(FPGA_ఎక్కించు_SECURE))
			flags = fpga_compatible2flag(devnum, compatible);
		ఒకవేళ (strcmp(compatible, "u-boot,fpga-legacy"))
			debug("Ignoring compatible = %s property\n",
			      compatible);
	}

	ret = fpga_ఎక్కించు(devnum, (శూన్య *)fpga_image->ఎక్కించు_addr,
			fpga_image->కొలత, బిట్టు_నిండిన, flags);
	ఒకవేళ (ret) {
		రాయి("%s: Cannot load the image to the FPGA\n", __func__);
		తిరిగి ret;
	}

	puts("FPGA image loaded from FIT\n");

	తిరిగి 0;
}

స్థిర పూర్ణాంకం spl_fit_ఎక్కించు_fpga(నిర్మాణం spl_fit_info *ctx,
			     నిర్మాణం spl_ఎక్కించు_info *info, ulong స్థానభ్రంశం)
{
	పూర్ణాంకం కణుపు, ret;

	నిర్మాణం spl_image_info fpga_image = {
		.ఎక్కించు_addr = 0,
	};

	కణుపు = spl_fit_get_image_కణుపు(ctx, FIT_FPGA_PROP, 0);
	ఒకవేళ (కణుపు < 0)
		తిరిగి కణుపు;

	warn_deprecated("'fpga' property in config node. Use 'loadables'");

	/* Load the image and set up the fpga_image structure */
	ret = ఎక్కించు_simple_fit(info, స్థానభ్రంశం, ctx, కణుపు, &fpga_image,
			      CONFIG_SYS_BOOTM_LEN);
	ఒకవేళ (ret) {
		రాయి("%s: Cannot load the FPGA: %i\n", __func__, ret);
		తిరిగి ret;
	}

	తిరిగి spl_fit_upload_fpga(ctx, కణుపు, &fpga_image);
}

స్థిర పూర్ణాంకం spl_simple_fit_read(నిర్మాణం spl_fit_info *ctx,
			       నిర్మాణం spl_ఎక్కించు_info *info, ulong స్థానభ్రంశం,
			       స్థిరరాశి శూన్య *fit_శీర్షిక)
{
	గుర్తులేని దీర్ఘ aligned_కొలత, లెక్క, కొలత;
	శూన్య *buf;
	పూర్ణాంకం ret;

	/*
	 * For FIT with external data, figure out where the external images
	 * start. This is the base for the data-offset properties in each
	 * image.
	 */
	కొలత = ALIGN(fdt_totalsize(fit_శీర్షిక), 4);
	కొలత = board_spl_fit_కొలత_align(కొలత);
	ctx->ext_దత్తాంశం_స్థానభ్రంశం = ALIGN(కొలత, 4);

	/*
	 * So far we only have one block of data from the FIT. Read the entire
	 * thing, including that first block.
	 *
	 * For FIT with data embedded, data is loaded as part of FIT image.
	 * For FIT with external data, data is not loaded in this step.
	 */
	ret = get_aligned_image_కొలత(info, కొలత, 0, &aligned_కొలత);
	ఒకవేళ (ret) {
		log_debug("Invalid FIT size: %lu\n", కొలత);
		తిరిగి ret;
	}
	buf = board_spl_fit_బఫరు_addr(aligned_కొలత, aligned_కొలత, 1);
	ఒకవేళ (!buf) {
		/*
		 * We assume that none of the board will ever use 0x0 as a
		 * valid load address. Theoretically some board could use it,
		 * but this is extremely unlikely.
		 */
		తిరిగి -EIO;
	}

	లెక్క = info->పఠ(info, స్థానభ్రంశం, aligned_కొలత, buf);
	ఒకవేళ (!లెక్క) {
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
		తిరిగి -EIO;
	}

	ctx->fit = buf;
	debug("fit read offset %lx, size=%lu, dst=%p, count=%lu\n",
	      స్థానభ్రంశం, aligned_కొలత, buf, లెక్క);

	తిరిగి 0;
}

స్థిర పూర్ణాంకం spl_simple_fit_parse(నిర్మాణం spl_fit_info *ctx)
{
	/* Find the correct subnode under "/configurations" */
	ctx->conf_కణుపు = fit_కనుగొను_config_కణుపు(ctx->fit);
	ఒకవేళ (ctx->conf_కణుపు < 0)
		తిరిగి -EINVAL;

	ఒకవేళ (CONFIG_IS_ENABLED(FIT_SIGNATURE)) {
		రాయి("## Checking hash(es) for config %s ... ",
		       fit_get_name(ctx->fit, ctx->conf_కణుపు, ఖాళీ));
		ఒకవేళ (fit_config_verify(ctx->fit, ctx->conf_కణుపు))
			తిరిగి -EPERM;
		puts("OK\n");
	}

	/* find the node holding the images information */
	ctx->images_కణుపు = fdt_మార్గం_స్థానభ్రంశం(ctx->fit, FIT_IMAGES_మార్గం);
	ఒకవేళ (ctx->images_కణుపు < 0) {
		debug("%s: Cannot find /images node: %d\n", __func__,
		      ctx->images_కణుపు);
		తిరిగి -EINVAL;
	}

	తిరిగి 0;
}

పూర్ణాంకం spl_ఎక్కించు_simple_fit(నిర్మాణం spl_image_info *spl_image,
			నిర్మాణం spl_ఎక్కించు_info *info, ulong స్థానభ్రంశం, శూన్య *fit)
{
	నిర్మాణం spl_image_info image_info;
	నిర్మాణం spl_fit_info ctx;
	పూర్ణాంకం కణుపు = -1;
	పూర్ణాంకం ret;
	పూర్ణాంకం సూచీ = 0;
	పూర్ణాంకం firmware_కణుపు;

	ret = spl_simple_fit_read(&ctx, info, స్థానభ్రంశం, fit);
	ఒకవేళ (ret < 0)
		తిరిగి ret;

	/* skip further processing if requested to enable load-only use cases */
	ఒకవేళ (spl_ఎక్కించు_simple_fit_skip_processing())
		తిరిగి 0;

	ctx.fit = spl_ఎక్కించు_simple_fit_fix_ఎక్కించు(ctx.fit);

	ret = spl_simple_fit_parse(&ctx);
	ఒకవేళ (ret < 0)
		తిరిగి ret;

	ఒకవేళ (IS_ENABLED(CONFIG_SPL_FPGA))
		spl_fit_ఎక్కించు_fpga(&ctx, info, స్థానభ్రంశం);

	/*
	 * Find the U-Boot image using the following search order:
	 *   - start at 'firmware' (e.g. an ARM Trusted Firmware)
	 *   - fall back 'kernel' (e.g. a Falcon-mode OS boot
	 *   - fall back to using the first 'loadables' entry
	 */
	ఒకవేళ (కణుపు < 0)
		కణుపు = spl_fit_get_image_కణుపు(&ctx, FIT_FIRMWARE_PROP, 0);

	ఒకవేళ (కణుపు < 0 && IS_ENABLED(CONFIG_SPL_OS_BOOT))
		కణుపు = spl_fit_get_image_కణుపు(&ctx, FIT_KERNEL_PROP, 0);

	ఒకవేళ (కణుపు < 0) {
		debug("could not find firmware image, trying loadables...\n");
		కణుపు = spl_fit_get_image_కణుపు(&ctx, FIT_LOADABLE_PROP, 0);
		/*
		 * If we pick the U-Boot image from "loadables", start at
		 * the second image when later loading additional images.
		 */
		సూచీ = 1;
	}
	ఒకవేళ (కణుపు < 0) {
		debug("%s: Cannot find u-boot image node: %d\n",
		      __func__, కణుపు);
		తిరిగి -1;
	}

	/* Load the image and set up the spl_image structure */
	ret = ఎక్కించు_simple_fit(info, స్థానభ్రంశం, &ctx, కణుపు, spl_image,
			      CONFIG_SYS_BOOTM_LEN);
	ఒకవేళ (ret)
		తిరిగి ret;

	/*
	 * For backward compatibility, we treat the first node that is
	 * as a U-Boot image, if no OS-type has been declared.
	 */
	ఒకవేళ (!spl_fit_image_get_os(ctx.fit, కణుపు, &spl_image->os))
		debug("Image OS is %s\n", genimg_get_os_name(spl_image->os));
	లేకపోతే ఒకవేళ (!IS_ENABLED(CONFIG_SPL_OS_BOOT))
		spl_image->os = IH_OS_U_BOOT;

	/*
	 * Booting a next-stage U-Boot may require us to append the FDT.
	 * We allow this to fail, as the U-Boot image might embed its FDT.
	 */
	ఒకవేళ (os_takes_devicetree(spl_image->os)) {
		ret = spl_fit_append_fdt(spl_image, info, స్థానభ్రంశం, &ctx);
		ఒకవేళ (ret < 0 && spl_image->os != IH_OS_U_BOOT)
			తిరిగి ret;
	}

	firmware_కణుపు = కణుపు;
	/* Now check if there are more images for us to load */
	కొరకు (; ; సూచీ++) {
		uint8_t os_type = IH_OS_INVALID;

		కణుపు = spl_fit_get_image_కణుపు(&ctx, FIT_LOADABLE_PROP, సూచీ);
		ఒకవేళ (కణుపు < 0)
			ఆపు;

		/*
		 * if the firmware is also a loadable, skip it because
		 * it already has been loaded. This is typically the case with
		 * u-boot.img generated by mkimage.
		 */
		ఒకవేళ (firmware_కణుపు == కణుపు)
			కొనసాగు;

		image_info.ఎక్కించు_addr = 0;
		ret = ఎక్కించు_simple_fit(info, స్థానభ్రంశం, &ctx, కణుపు, &image_info,
				      CONFIG_SYS_BOOTM_LEN);
		ఒకవేళ (ret < 0 && ret != -EBADSLT) {
			రాయి("%s: can't load image loadables index %d (ret = %d)\n",
			       __func__, సూచీ, ret);
			తిరిగి ret;
		}

		ఒకవేళ (spl_fit_image_is_fpga(ctx.fit, కణుపు))
			spl_fit_upload_fpga(&ctx, కణుపు, &image_info);

		ఒకవేళ (!spl_fit_image_get_os(ctx.fit, కణుపు, &os_type))
			debug("Loadable is %s\n", genimg_get_os_name(os_type));

		ఒకవేళ (os_takes_devicetree(os_type)) {
			spl_fit_append_fdt(&image_info, info, స్థానభ్రంశం, &ctx);
			spl_image->fdt_addr = image_info.fdt_addr;
		}

		/*
		 * If the "firmware" image did not provide an entry point,
		 * use the first valid entry point from the loadables.
		 */
		ఒకవేళ (spl_image->నమోదు_బిందువు == FDT_ERROR &&
		    image_info.నమోదు_బిందువు != FDT_ERROR)
			spl_image->నమోదు_బిందువు = image_info.నమోదు_బిందువు;

		spl_fit_image_రికార్డు_arm32_optee(ctx.fit, కణుపు, spl_image,
						 &image_info);

		/* Record our loadables into the FDT */
		ఒకవేళ (!CONFIG_IS_ENABLED(FIT_IMAGE_TINY) &&
		    xpl_get_fdt_update(info) && spl_image->fdt_addr)
			spl_fit_రికార్డు_loadable(&ctx, సూచీ,
						spl_image->fdt_addr,
						&image_info);
	}

	/*
	 * If a platform does not provide CONFIG_SYS_UBOOT_START, U-Boot's
	 * Makefile will set it to 0 and it will end up as the entry point
	 * here. What it actually means is: use the load address.
	 */
	ఒకవేళ (spl_image->నమోదు_బిందువు == FDT_ERROR || spl_image->నమోదు_బిందువు == 0)
		spl_image->నమోదు_బిందువు = spl_image->ఎక్కించు_addr;

	spl_image->flags |= SPL_FIT_దొరికింది;
	upl_సమితి_fit_info(map_గమ్యం_sysmem(ctx.fit), ctx.conf_కణుపు,
			 spl_image->నమోదు_బిందువు);

	తిరిగి 0;
}

/* Parse and load full fitImage in SPL */
పూర్ణాంకం spl_ఎక్కించు_fit_image(నిర్మాణం spl_image_info *spl_image,
		       స్థిరరాశి నిర్మాణం legacy_img_hdr *శీర్షిక)
{
	నిర్మాణం bootm_శీర్షికలు images;
	స్థిరరాశి అక్షరం *fit_uname_config = ఖాళీ;
	ulong fdt_hack;
	స్థిరరాశి అక్షరం *uname;
	ulong fw_దత్తాంశం = 0, dt_దత్తాంశం = 0, img_దత్తాంశం = 0;
	ulong fw_len = 0, dt_len = 0, img_len = 0;
	పూర్ణాంకం idx, conf_noffset;
	పూర్ణాంకం ret;

	images.verify = CONFIG_IS_ENABLED(FIT_SIGNATURE);

	ret = fit_image_ఎక్కించు(&images, virt_గమ్యం_phys((శూన్య *)శీర్షిక),
			     ఖాళీ, &fit_uname_config, IH_ARCH_DEFAULT,
			     IH_TYPE_FIRMWARE, -1, FIT_ఎక్కించు_OPTIONAL,
			     &fw_దత్తాంశం, &fw_len);

	ఒకవేళ (ret < 0) {
		ret = fit_image_ఎక్కించు(&images, virt_గమ్యం_phys((శూన్య *)శీర్షిక),
				     ఖాళీ, &fit_uname_config, IH_ARCH_DEFAULT,
				     IH_TYPE_KERNEL, -1, FIT_ఎక్కించు_OPTIONAL,
				     &fw_దత్తాంశం, &fw_len);
	}

	ఒకవేళ (ret < 0)
		తిరిగి ret;

	spl_image->కొలత = fw_len;
	spl_image->ఎక్కించు_addr = fw_దత్తాంశం;
	ఒకవేళ (fit_image_get_నమోదు(శీర్షిక, ret, &spl_image->నమోదు_బిందువు))
		spl_image->నమోదు_బిందువు = fw_దత్తాంశం;
	ఒకవేళ (fit_image_get_os(శీర్షిక, ret, &spl_image->os))
		spl_image->os = IH_OS_INVALID;
	spl_image->name = genimg_get_os_name(spl_image->os);

	debug(PHASE_PROMPT "payload image: %32s load addr: 0x%lx size: %d\n",
	      spl_image->name, spl_image->ఎక్కించు_addr, spl_image->కొలత);

	images.verify = CONFIG_IS_ENABLED(FIT_SIGNATURE);

	ret = fit_image_ఎక్కించు(&images, virt_గమ్యం_phys((శూన్య *)శీర్షిక), ఖాళీ,
			     &fit_uname_config, IH_ARCH_DEFAULT, IH_TYPE_FLATDT,
			     -1, FIT_ఎక్కించు_OPTIONAL, &dt_దత్తాంశం, &dt_len);
	ఒకవేళ (ret >= 0) {
		ఒకవేళ (spl_image->os == IH_OS_U_BOOT) {
			/* HACK: U-Boot expects FDT at a specific address */
			fdt_hack = ALIGN(spl_image->ఎక్కించు_addr + spl_image->కొలత, 8);
			debug("Relocating FDT to %p\n", (శూన్య *)fdt_hack);
			జ్ఞాపకం_నకలు(map_sysmem(fdt_hack, dt_len),
			       map_sysmem(dt_దత్తాంశం, 0), dt_len);
			spl_image->fdt_addr = (శూన్య *)fdt_hack;
		} లేకపోతే {
			spl_image->fdt_addr = (శూన్య *)dt_దత్తాంశం;
		}
	}

	conf_noffset = fit_conf_get_కణుపు((స్థిరరాశి శూన్య *)శీర్షిక,
					 fit_uname_config);
	ఒకవేళ (conf_noffset < 0)
		తిరిగి 0;

	కొరకు (idx = 0;
	     uname = fdt_stringlist_get((స్థిరరాశి శూన్య *)శీర్షిక, conf_noffset,
					FIT_LOADABLE_PROP, idx,
				ఖాళీ), uname;
	     idx++) {
		images.verify = CONFIG_IS_ENABLED(FIT_SIGNATURE);

		ret = fit_image_ఎక్కించు(&images, virt_గమ్యం_phys((శూన్య *)శీర్షిక),
				     &uname, &fit_uname_config,
				     IH_ARCH_DEFAULT, IH_TYPE_LOADABLE, -1,
				     FIT_ఎక్కించు_OPTIONAL_NON_ZERO,
				     &img_దత్తాంశం, &img_len);
		ఒకవేళ (ret < 0)
			తిరిగి ret;
	}
	spl_image->flags |= SPL_FIT_దొరికింది;

	upl_సమితి_fit_info(map_గమ్యం_sysmem(శీర్షిక), conf_noffset,
			 spl_image->నమోదు_బిందువు);

	తిరిగి 0;
}
