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

DECLARE_GLOBAL_انکڑے_PTR;

بنية spl_fit_info {
	ثابت فراغ *fit;	/* Pointer to a valid FIT blob */
	size_t ext_انکڑے_وتھ;	/* Offset to FIT external data (end of FIT) */
	صحيح images_گرہ;	/* FDT offset to "/images" node */
	صحيح conf_گرہ;		/* FDT offset to selected configuration node */
};

__weak ulong board_spl_fit_ناپ_align(ulong ناپ)
{
	أرجع ناپ;
}

ساكن صحيح لبھو_گرہ_from_desc(ثابت فراغ *fit, صحيح گرہ, ثابت حرف *str)
{
	صحيح بچہ;

	إذا (گرہ < 0)
		أرجع -EINVAL;

	/* iterate the FIT nodes and find a matching description */
	لكل (بچہ = fdt_پہلا_subnode(fit, گرہ); بچہ >= 0;
	     بچہ = fdt_اگلا_subnode(fit, بچہ)) {
		صحيح len;
		ثابت حرف *desc = fdt_getprop(fit, بچہ, FIT_DESC_PROP, &len);

		إذا (!desc)
			استمر;

		إذا (!strcmp(desc, str))
			أرجع بچہ;
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
				  ثابت حرف *type, صحيح اشاریہ,
				  ثابت حرف **outname)
{
	بنية udevice *sysinfo;
	ثابت حرف *name, *str, *سرا_2;
	__maybe_unused صحيح گرہ;
	صحيح len, i;
	bool لبھیا = true;

	name = fdt_getprop(ctx->fit, ctx->conf_گرہ, type, &len);
	إذا (!name) {
		debug("cannot find property '%s': %d\n", type, len);
		أرجع -EINVAL;
	}
	/* A string property should be NUL terminated */
	سرا_2 = name + len - 1;
	إذا (!len || *سرا_2) {
		debug("malformed property '%s'\n", type);
		أرجع -EINVAL;
	}

	str = name;
	لكل (i = 0; i < اشاریہ; i++) {
		str = strchr(str, '\0') + 1;
		إذا (str > سرا_2) {
			لبھیا = false;
			اكسر;
		}
	}

	إذا (!لبھیا && CONFIG_IS_ENABLED(SYSINFO) && !sysinfo_get(&sysinfo)) {
		صحيح rc;
		/*
		 * no string in the property for this index. Check if the
		 * sysinfo-level code can supply one.
		 */
		rc = sysinfo_detect(sysinfo);
		إذا (rc)
			أرجع rc;

		rc = sysinfo_get_fit_loadable(sysinfo, اشاریہ - i - 1, type,
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
			صحيح گرہ;
			صحيح images = fdt_رستہ_وتھ(ctx->fit, FIT_IMAGES_رستہ);

			گرہ = لبھو_گرہ_from_desc(ctx->fit, images, str);
			إذا (گرہ > 0)
				str = fdt_get_name(ctx->fit, گرہ, عدم);

			لبھیا = true;
		}
	}

	إذا (!لبھیا) {
		debug("no string for index %d\n", اشاریہ);
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
ساكن صحيح spl_fit_get_image_گرہ(ثابت بنية spl_fit_info *ctx,
				  ثابت حرف *type, صحيح اشاریہ)
{
	ثابت حرف *str;
	صحيح err;
	صحيح گرہ;

	err = spl_fit_get_image_name(ctx, type, اشاریہ, &str);
	إذا (err)
		أرجع err;

	debug("%s: '%s'\n", type, str);

	گرہ = fdt_subnode_وتھ(ctx->fit, ctx->images_گرہ, str);
	إذا (گرہ < 0) {
		pr_err("cannot find image node '%s': %d\n", str, گرہ);
		أرجع -EINVAL;
	}

	أرجع گرہ;
}

ساكن u32 get_aligned_image_وتھ(بنية spl_لدو_info *info, u32 وتھ)
{
	أرجع ALIGN_تھلے(وتھ, spl_get_bl_len(info));
}

ساكن u32 get_aligned_image_overhead(بنية spl_لدو_info *info, u32 وتھ)
{
	أرجع وتھ & (spl_get_bl_len(info) - 1);
}

ساكن صحيح get_aligned_image_ناپ(بنية spl_لدو_info *info, ulong انکڑے_ناپ,
				  u32 وتھ, ulong *aligned_ناپ)
{
	u32 overhead = get_aligned_image_overhead(info, وتھ);

	إذا (انکڑے_ناپ > ULONG_MAX - overhead)
		أرجع -EOVERFLOW;
	انکڑے_ناپ += overhead;

	إذا (انکڑے_ناپ > ULONG_MAX - (spl_get_bl_len(info) - 1))
		أرجع -EOVERFLOW;
	انکڑے_ناپ = ALIGN(انکڑے_ناپ, spl_get_bl_len(info));

	*aligned_ناپ = انکڑے_ناپ;

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
ساكن فراغ fit_fill_image_info(ثابت فراغ *fit, صحيح گرہ,
				بنية spl_image_info *image_info,
				ulong لدو_addr, ulong ناپ)
{
	ulong اندراج_نقطہ;

	إذا (!image_info)
		أرجع;

	image_info->لدو_addr = لدو_addr;
	image_info->ناپ = ناپ;

	إذا (!fit_image_get_اندراج(fit, گرہ, &اندراج_نقطہ))
		image_info->اندراج_نقطہ = اندراج_نقطہ;
	وإلا
		image_info->اندراج_نقطہ = FDT_ERROR;
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
ساكن صحيح لدو_simple_fit(بنية spl_لدو_info *info, ulong fit_وتھ,
			   ثابت بنية spl_fit_info *ctx, صحيح گرہ,
			   بنية spl_image_info *image_info, ulong max_ناپ)
{
	u32 وتھ;
	u32 len;
	size_t لمائی;
	ulong ناپ;
	ulong لدو_addr;
	فراغ *لدو_ptr;
	فراغ *src;
	uint8_t image_comp = -1, type = -1;
	ثابت فراغ *انکڑے;
	ثابت فراغ *fit = ctx->fit;
	bool external_انکڑے = false;
	صحيح ret;

	log_debug("starting\n");
	إذا (CONFIG_IS_ENABLED(BOOTMETH_VBE) &&
	    xpl_get_phase(info) != IH_PHASE_NONE) {
		تعداد image_phase_t phase;

		ret = fit_image_get_phase(fit, گرہ, &phase);
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
		إذا (fit_image_get_type(fit, گرہ, &type))
			puts("Cannot get image type.\n");
		وإلا
			debug("%s ", genimg_get_type_name(type));
	}

	إذا (spl_decompression_enabled()) {
		fit_image_get_comp(fit, گرہ, &image_comp);
		debug("%s ", genimg_get_comp_name(image_comp));
	}

	إذا (fit_image_get_لدو(fit, گرہ, &لدو_addr)) {
		إذا (!image_info->لدو_addr) {
			لکھو("Can't load %s: No load address and no buffer\n",
			       fit_get_name(fit, گرہ, عدم));
			أرجع -ENOBUFS;
		}
		لدو_addr = image_info->لدو_addr;
	}

	إذا (!fit_image_get_انکڑے_position(fit, گرہ, &وتھ)) {
		external_انکڑے = true;
	} وإلا إذا (!fit_image_get_انکڑے_وتھ(fit, گرہ, &وتھ)) {
		/* The resulting offset cannot exceed UINT32_MAX */
		إذا (ctx->ext_انکڑے_وتھ > UINT32_MAX - وتھ) {
			log_debug("Invalid external data offset: %u\n", وتھ);
			أرجع -EINVAL;
		}
		log_debug("read offset %x = offset from fit %x\n", وتھ,
			  (u32)(وتھ + ctx->ext_انکڑے_وتھ));
		وتھ += ctx->ext_انکڑے_وتھ;
		external_انکڑے = true;
	}

	إذا (external_انکڑے) {
		u32 aligned_وتھ;
		ulong read_وتھ;
		فراغ *src_ptr;

		/* External data */
		إذا (fit_image_get_انکڑے_ناپ(fit, گرہ, &len))
			أرجع -ENOENT;

		/* Dont bother to copy 0 byte data, but warn, though */
		إذا (!len) {
			log_warning("%s: Skip load '%s': image size is 0!\n",
				    __func__, fit_get_name(fit, گرہ, عدم));
			fit_fill_image_info(fit, گرہ, image_info, لدو_addr, 0);
			أرجع 0;
		}

		/*
		 * data-size is excluded from the configuration signature (it
		 * is in exc_prop[] in image-fit-sig.c), so it stays attacker
		 * controlled even after fit_config_verify() succeeds. The
		 * image hash is only verified after the device read below, so
		 * an oversized value has to be rejected here.
		 */
		ret = get_aligned_image_ناپ(info, len, وتھ, &ناپ);
		إذا (ret) {
			log_debug("Invalid external data size: %u\n", len);
			أرجع ret;
		}

		إذا (ناپ > max_ناپ) {
			log_debug("Image too large: aligned size %lu, max %lu (data-size %u)\n",
				  ناپ, max_ناپ, len);
			أرجع -EFBIG;
		}

		aligned_وتھ = get_aligned_image_وتھ(info, وتھ);
		إذا (aligned_وتھ > ULONG_MAX - fit_وتھ) {
			log_debug("Invalid aligned external data offset: %u\n",
				  aligned_وتھ);
			أرجع -EINVAL;
		}
		read_وتھ = fit_وتھ + aligned_وتھ;

		إذا (spl_decompression_enabled() &&
		    (image_comp == IH_COMP_GZIP || image_comp == IH_COMP_LZMA))
			src_ptr = map_sysmem(ALIGN(CONFIG_SYS_لدو_ADDR, ARCH_DMA_MINALIGN), len);
		وإلا
			src_ptr = map_sysmem(ALIGN(لدو_addr, ARCH_DMA_MINALIGN), len);
		لمائی = len;

		/*
		 * For non-signed FIT images, we can check that
		 * (read_offset + size) does not wrap and that
		 * (src_ptr + size) does not exceed the addressable range.
		 * For signed FITs, we can additionally check that
		 * (offset + len) doesn't exceed the allowed FIT image
		 * maximum size.
		 */
		إذا (ناپ > ULONG_MAX - read_وتھ ||
		    ناپ > UINTPTR_MAX - (uintptr_t)src_ptr
		/*
		 * #if (not a runtime if) is required: FIT_SIGNATURE_MAX_SIZE
		 * depends on FIT_SIGNATURE, so CONFIG_VAL(FIT_SIGNATURE_MAX_SIZE)
		 * is undefined when signing is disabled and referencing it
		 * here would fail to compile.
		 */
#إذا CONFIG_IS_ENABLED(FIT_SIGNATURE)
		    || وتھ > CONFIG_VAL(FIT_SIGNATURE_MAX_ناپ) ||
		    len > CONFIG_VAL(FIT_SIGNATURE_MAX_ناپ) - وتھ
#جے_انت
		) {
			log_debug("FIT external data is out of bounds (offset=%u, size=%u)\n",
				  وتھ, len);
			أرجع -EINVAL;
		}

		log_debug("reading from offset %x / %lx size %lx to %p: ",
			  وتھ, read_وتھ, ناپ, src_ptr);

		إذا (info->اقرأ(info, read_وتھ, ناپ, src_ptr) < لمائی)
			أرجع -EIO;

		debug("External data: dst=%p, offset=%x, size=%lx\n",
		      src_ptr, وتھ, (بنانشانی لما)لمائی);
		src = src_ptr + get_aligned_image_overhead(info, وتھ);
	} وإلا {
		/* Embedded data */
		إذا (fit_image_get_emb_انکڑے(fit, گرہ, &انکڑے, &لمائی)) {
			puts("Cannot get image data/size\n");
			أرجع -ENOENT;
		}
		debug("Embedded data: dst=%lx, size=%lx\n", لدو_addr,
		      (بنانشانی لما)لمائی);
		src = (فراغ *)انکڑے;	/* cast away const */
	}

	إذا (CONFIG_IS_ENABLED(FIT_SIGNATURE)) {
		لکھو("## Checking hash(es) for Image %s ... ",
		       fit_get_name(fit, گرہ, عدم));
		إذا (!fit_image_verify_with_انکڑے(fit, گرہ, gd_fdt_blob(), src,
						لمائی))
			أرجع -EPERM;
		puts("OK\n");
	}

	إذا (CONFIG_IS_ENABLED(FIT_IMAGE_POST_PROCESS))
		board_fit_image_post_process(fit, گرہ, &src, &لمائی);

	لدو_ptr = map_sysmem(لدو_addr, لمائی);
	إذا (IS_ENABLED(CONFIG_SPL_GZIP) && image_comp == IH_COMP_GZIP) {
		ناپ = لمائی;
		إذا (gunzip(لدو_ptr, CONFIG_SYS_BOOTM_LEN, src, &ناپ)) {
			puts("Uncompressing error\n");
			أرجع -EIO;
		}
		لمائی = ناپ;
	} وإلا إذا (IS_ENABLED(CONFIG_SPL_LZMA) && image_comp == IH_COMP_LZMA) {
		ناپ = CONFIG_SYS_BOOTM_LEN;
		ulong لدو_سرا;

		إذا (image_decomp(IH_COMP_LZMA, CONFIG_SYS_لدو_ADDR, 0, 0,
				 لدو_ptr, src, لمائی, ناپ, &لدو_سرا)) {
			puts("Uncompressing error\n");
			أرجع -EIO;
		}
		لمائی = لدو_سرا - CONFIG_SYS_لدو_ADDR;
	} وإلا {
		memmove(لدو_ptr, src, لمائی);
	}

	fit_fill_image_info(fit, گرہ, image_info, لدو_addr, لمائی);
	log_debug("- done loading\n");

	upl_جوڑو_image(fit, گرہ, لدو_addr, لمائی);

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
			      بنية spl_لدو_info *info, ulong وتھ,
			      ثابت بنية spl_fit_info *ctx)
{
	بنية spl_image_info image_info;
	صحيح گرہ, ret = 0, اشاریہ = 0;

	/*
	 * Use the address following the image as target address for the
	 * device tree.
	 */
	image_info.لدو_addr = ALIGN(spl_image->لدو_addr + spl_image->ناپ, 8);

	/* Figure out which device tree the board wants to use */
	گرہ = spl_fit_get_image_گرہ(ctx, FIT_FDT_PROP, اشاریہ++);
	إذا (گرہ < 0) {
		size_t ناپ;

		debug("%s: cannot find FDT node\n", __func__);

		/*
		 * U-Boot did not find a device tree inside the FIT image. Use
		 * the U-Boot device tree instead.
		 */
		إذا (!gd->fdt_blob)
			أرجع گرہ;

		/*
		 * Make the load-address of the FDT available for the SPL
		 * framework
		 */
		ناپ = fdt_totalsize(gd->fdt_blob);
		spl_image->fdt_addr = map_sysmem(image_info.لدو_addr, ناپ);
		حافظہ_نقل(spl_image->fdt_addr, gd->fdt_blob, ناپ);
	} وإلا {
		ret = لدو_simple_fit(info, وتھ, ctx, گرہ, &image_info,
				      CONFIG_SYS_BOOTM_LEN);
		إذا (ret < 0)
			أرجع ret;

		spl_image->fdt_addr = phys_منزل_virt(image_info.لدو_addr);
	}

	إذا (CONFIG_IS_ENABLED(FIT_IMAGE_TINY))
		أرجع 0;

#إذا CONFIG_IS_ENABLED(لدو_FIT_APPLY_OVERLAY)
		فراغ *tmpbuffer = عدم;

		لكل (; ; اشاریہ++) {
			ثابت حرف *str;

			ret = spl_fit_get_image_name(ctx, FIT_FDT_PROP, اشاریہ, &str);
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

			گرہ = fdt_subnode_وتھ(ctx->fit, ctx->images_گرہ, str);
			إذا (گرہ < 0) {
				debug("%s: unable to find FDT node %d\n",
				      __func__, اشاریہ);
				استمر;
			}

			إذا (!tmpbuffer) {
				/*
				 * allocate memory to store the DT overlay
				 * before it is applied. It may not be used
				 * depending on how the overlay is stored, so
				 * don't fail yet if the allocation failed.
				 */
				size_t ناپ = CONFIG_SPL_لدو_FIT_APPLY_OVERLAY_BUF_SZ;

				tmpbuffer = malloc_cache_aligned(ناپ);
				إذا (!tmpbuffer)
					debug("%s: unable to allocate space for overlays\n",
					      __func__);
			}
			image_info.لدو_addr = (ulong)tmpbuffer;
			ret = لدو_simple_fit(info, وتھ, ctx, گرہ,
					      &image_info,
					      CONFIG_SPL_لدو_FIT_APPLY_OVERLAY_BUF_SZ);
			إذا (ret == -EBADSLT)
				استمر;
			وإلا إذا (ret < 0)
				اكسر;

			/* Make room in FDT for changes from the overlay */
			ret = fdt_increase_ناپ(spl_image->fdt_addr,
						image_info.ناپ);
			إذا (ret < 0)
				اكسر;

			ret = fdt_overlay_apply_verbose(spl_image->fdt_addr,
							(فراغ *)image_info.لدو_addr);
			إذا (ret) {
				pr_err("failed to apply DT overlay %s\n",
				       fit_get_name(ctx->fit, گرہ, عدم));
				اكسر;
			}

			debug("%s: DT overlay %s applied\n", __func__,
			      fit_get_name(ctx->fit, گرہ, عدم));
		}
		حرر(tmpbuffer);
		إذا (ret)
			أرجع ret;
#جے_انت
	/* Try to make space, so we can inject details on the loadables */
	ret = fdt_shrink_منزل_minimum(spl_image->fdt_addr, 8192);
	إذا (ret < 0)
		أرجع ret;

	أرجع ret;
}

ساكن صحيح spl_fit_ریکارڈ_loadable(ثابت بنية spl_fit_info *ctx, صحيح اشاریہ,
				   فراغ *blob, بنية spl_image_info *image)
{
	صحيح ret = 0;
	ثابت حرف *name;
	صحيح گرہ;

	ret = spl_fit_get_image_name(ctx, FIT_LOADABLE_PROP, اشاریہ, &name);
	إذا (ret < 0)
		أرجع ret;

	گرہ = spl_fit_get_image_گرہ(ctx, FIT_LOADABLE_PROP, اشاریہ);

	ret = fdt_ریکارڈ_loadable(blob, اشاریہ, name, image->لدو_addr,
			image->ناپ, image->اندراج_نقطہ,
			fdt_getprop(ctx->fit, گرہ, FIT_TYPE_PROP, عدم),
			fdt_getprop(ctx->fit, گرہ, FIT_OS_PROP, عدم),
			fdt_getprop(ctx->fit, گرہ, FIT_ARCH_PROP, عدم));

	أرجع ret;
}

ساكن صحيح spl_fit_image_is_fpga(ثابت فراغ *fit, صحيح گرہ)
{
	ثابت حرف *type;

	إذا (!IS_ENABLED(CONFIG_SPL_FPGA))
		أرجع 0;

	type = fdt_getprop(fit, گرہ, FIT_TYPE_PROP, عدم);
	إذا (!type)
		أرجع 0;

	أرجع !strcmp(type, "fpga");
}

ساكن فراغ spl_fit_image_ریکارڈ_arm32_optee(ثابت فراغ *fit, صحيح گرہ,
					     بنية spl_image_info *spl_image,
					     بنية spl_image_info *image_info)
{
#إذا defined(CONFIG_BOOTM_OPTEE) && defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
	ثابت حرف *type = fdt_getprop(fit, گرہ, FIT_TYPE_PROP, عدم);

	إذا (!type)
		أرجع;

	إذا (strcmp(type, "tee"))
		أرجع;

	spl_image->optee_addr = image_info->لدو_addr;
#جے_انت
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
__weak فراغ *board_spl_fit_بفر_addr(ulong fit_ناپ, صحيح sectors, صحيح bl_len)
{
	فراغ *buf;
	size_t ناپ = sectors * bl_len;

	buf = malloc_cache_aligned(ناپ);
	إذا (!buf) {
		pr_err("Could not get FIT buffer of %lu bytes\n", (ulong)ناپ);

		إذا (IS_ENABLED(CONFIG_SPL_SYS_MALLOC))
			pr_err("\tcheck CONFIG_SPL_SYS_MALLOC_SIZE\n");
		وإلا
			pr_err("\tcheck CONFIG_SPL_SYS_MALLOC_F_LEN\n");

		buf = spl_get_لدو_بفر(0, ناپ);
	}
	أرجع buf;
}

/*
 * Weak default function to allow customizing SPL fit loading for load-only
 * use cases by allowing to skip the parsing/processing of the FIT contents
 * (so that this can be done separately in a more customized fashion)
 */
__weak bool spl_لدو_simple_fit_skip_processing(فراغ)
{
	أرجع false;
}

/*
 * Weak default function to allow fixes after fit header
 * is loaded.
 */
__weak فراغ *spl_لدو_simple_fit_fix_لدو(ثابت فراغ *fit)
{
	أرجع (فراغ *)fit;
}

ساكن فراغ warn_deprecated(ثابت حرف *پیغام)
{
	لکھو("DEPRECATED: %s\n", پیغام);
	لکھو("\tSee https://fitspec.osfw.foundation/\n");
}

ساكن صحيح spl_fit_upload_fpga(بنية spl_fit_info *ctx, صحيح گرہ,
			       بنية spl_image_info *fpga_image)
{
	ثابت حرف *compatible;
	صحيح ret;
	صحيح devnum = 0;
	صحيح flags = 0;

	debug("FPGA bitstream at: %x, size: %x\n",
	      (u32)fpga_image->لدو_addr, fpga_image->ناپ);

	compatible = fdt_getprop(ctx->fit, گرہ, FIT_COMPAT_PROP, عدم);
	إذا (!compatible) {
		warn_deprecated("'fpga' image without 'compatible' property");
	} وإلا {
		إذا (CONFIG_IS_ENABLED(FPGA_لدو_SECURE))
			flags = fpga_compatible2flag(devnum, compatible);
		إذا (strcmp(compatible, "u-boot,fpga-legacy"))
			debug("Ignoring compatible = %s property\n",
			      compatible);
	}

	ret = fpga_لدو(devnum, (فراغ *)fpga_image->لدو_addr,
			fpga_image->ناپ, ثنائی_ہندسہ_بھریا, flags);
	إذا (ret) {
		لکھو("%s: Cannot load the image to the FPGA\n", __func__);
		أرجع ret;
	}

	puts("FPGA image loaded from FIT\n");

	أرجع 0;
}

ساكن صحيح spl_fit_لدو_fpga(بنية spl_fit_info *ctx,
			     بنية spl_لدو_info *info, ulong وتھ)
{
	صحيح گرہ, ret;

	بنية spl_image_info fpga_image = {
		.لدو_addr = 0,
	};

	گرہ = spl_fit_get_image_گرہ(ctx, FIT_FPGA_PROP, 0);
	إذا (گرہ < 0)
		أرجع گرہ;

	warn_deprecated("'fpga' property in config node. Use 'loadables'");

	/* Load the image and set up the fpga_image structure */
	ret = لدو_simple_fit(info, وتھ, ctx, گرہ, &fpga_image,
			      CONFIG_SYS_BOOTM_LEN);
	إذا (ret) {
		لکھو("%s: Cannot load the FPGA: %i\n", __func__, ret);
		أرجع ret;
	}

	أرجع spl_fit_upload_fpga(ctx, گرہ, &fpga_image);
}

ساكن صحيح spl_simple_fit_read(بنية spl_fit_info *ctx,
			       بنية spl_لدو_info *info, ulong وتھ,
			       ثابت فراغ *fit_سرخی)
{
	بنانشانی لما aligned_ناپ, گنتی, ناپ;
	فراغ *buf;
	صحيح ret;

	/*
	 * For FIT with external data, figure out where the external images
	 * start. This is the base for the data-offset properties in each
	 * image.
	 */
	ناپ = ALIGN(fdt_totalsize(fit_سرخی), 4);
	ناپ = board_spl_fit_ناپ_align(ناپ);
	ctx->ext_انکڑے_وتھ = ALIGN(ناپ, 4);

	/*
	 * So far we only have one block of data from the FIT. Read the entire
	 * thing, including that first block.
	 *
	 * For FIT with data embedded, data is loaded as part of FIT image.
	 * For FIT with external data, data is not loaded in this step.
	 */
	ret = get_aligned_image_ناپ(info, ناپ, 0, &aligned_ناپ);
	إذا (ret) {
		log_debug("Invalid FIT size: %lu\n", ناپ);
		أرجع ret;
	}
	buf = board_spl_fit_بفر_addr(aligned_ناپ, aligned_ناپ, 1);
	إذا (!buf) {
		/*
		 * We assume that none of the board will ever use 0x0 as a
		 * valid load address. Theoretically some board could use it,
		 * but this is extremely unlikely.
		 */
		أرجع -EIO;
	}

	گنتی = info->اقرأ(info, وتھ, aligned_ناپ, buf);
	إذا (!گنتی) {
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
	      وتھ, aligned_ناپ, buf, گنتی);

	أرجع 0;
}

ساكن صحيح spl_simple_fit_parse(بنية spl_fit_info *ctx)
{
	/* Find the correct subnode under "/configurations" */
	ctx->conf_گرہ = fit_لبھو_config_گرہ(ctx->fit);
	إذا (ctx->conf_گرہ < 0)
		أرجع -EINVAL;

	إذا (CONFIG_IS_ENABLED(FIT_SIGNATURE)) {
		لکھو("## Checking hash(es) for config %s ... ",
		       fit_get_name(ctx->fit, ctx->conf_گرہ, عدم));
		إذا (fit_config_verify(ctx->fit, ctx->conf_گرہ))
			أرجع -EPERM;
		puts("OK\n");
	}

	/* find the node holding the images information */
	ctx->images_گرہ = fdt_رستہ_وتھ(ctx->fit, FIT_IMAGES_رستہ);
	إذا (ctx->images_گرہ < 0) {
		debug("%s: Cannot find /images node: %d\n", __func__,
		      ctx->images_گرہ);
		أرجع -EINVAL;
	}

	أرجع 0;
}

صحيح spl_لدو_simple_fit(بنية spl_image_info *spl_image,
			بنية spl_لدو_info *info, ulong وتھ, فراغ *fit)
{
	بنية spl_image_info image_info;
	بنية spl_fit_info ctx;
	صحيح گرہ = -1;
	صحيح ret;
	صحيح اشاریہ = 0;
	صحيح firmware_گرہ;

	ret = spl_simple_fit_read(&ctx, info, وتھ, fit);
	إذا (ret < 0)
		أرجع ret;

	/* skip further processing if requested to enable load-only use cases */
	إذا (spl_لدو_simple_fit_skip_processing())
		أرجع 0;

	ctx.fit = spl_لدو_simple_fit_fix_لدو(ctx.fit);

	ret = spl_simple_fit_parse(&ctx);
	إذا (ret < 0)
		أرجع ret;

	إذا (IS_ENABLED(CONFIG_SPL_FPGA))
		spl_fit_لدو_fpga(&ctx, info, وتھ);

	/*
	 * Find the U-Boot image using the following search order:
	 *   - start at 'firmware' (e.g. an ARM Trusted Firmware)
	 *   - fall back 'kernel' (e.g. a Falcon-mode OS boot
	 *   - fall back to using the first 'loadables' entry
	 */
	إذا (گرہ < 0)
		گرہ = spl_fit_get_image_گرہ(&ctx, FIT_FIRMWARE_PROP, 0);

	إذا (گرہ < 0 && IS_ENABLED(CONFIG_SPL_OS_BOOT))
		گرہ = spl_fit_get_image_گرہ(&ctx, FIT_KERNEL_PROP, 0);

	إذا (گرہ < 0) {
		debug("could not find firmware image, trying loadables...\n");
		گرہ = spl_fit_get_image_گرہ(&ctx, FIT_LOADABLE_PROP, 0);
		/*
		 * If we pick the U-Boot image from "loadables", start at
		 * the second image when later loading additional images.
		 */
		اشاریہ = 1;
	}
	إذا (گرہ < 0) {
		debug("%s: Cannot find u-boot image node: %d\n",
		      __func__, گرہ);
		أرجع -1;
	}

	/* Load the image and set up the spl_image structure */
	ret = لدو_simple_fit(info, وتھ, &ctx, گرہ, spl_image,
			      CONFIG_SYS_BOOTM_LEN);
	إذا (ret)
		أرجع ret;

	/*
	 * For backward compatibility, we treat the first node that is
	 * as a U-Boot image, if no OS-type has been declared.
	 */
	إذا (!spl_fit_image_get_os(ctx.fit, گرہ, &spl_image->os))
		debug("Image OS is %s\n", genimg_get_os_name(spl_image->os));
	وإلا إذا (!IS_ENABLED(CONFIG_SPL_OS_BOOT))
		spl_image->os = IH_OS_U_BOOT;

	/*
	 * Booting a next-stage U-Boot may require us to append the FDT.
	 * We allow this to fail, as the U-Boot image might embed its FDT.
	 */
	إذا (os_takes_devicetree(spl_image->os)) {
		ret = spl_fit_append_fdt(spl_image, info, وتھ, &ctx);
		إذا (ret < 0 && spl_image->os != IH_OS_U_BOOT)
			أرجع ret;
	}

	firmware_گرہ = گرہ;
	/* Now check if there are more images for us to load */
	لكل (; ; اشاریہ++) {
		uint8_t os_type = IH_OS_INVALID;

		گرہ = spl_fit_get_image_گرہ(&ctx, FIT_LOADABLE_PROP, اشاریہ);
		إذا (گرہ < 0)
			اكسر;

		/*
		 * if the firmware is also a loadable, skip it because
		 * it already has been loaded. This is typically the case with
		 * u-boot.img generated by mkimage.
		 */
		إذا (firmware_گرہ == گرہ)
			استمر;

		image_info.لدو_addr = 0;
		ret = لدو_simple_fit(info, وتھ, &ctx, گرہ, &image_info,
				      CONFIG_SYS_BOOTM_LEN);
		إذا (ret < 0 && ret != -EBADSLT) {
			لکھو("%s: can't load image loadables index %d (ret = %d)\n",
			       __func__, اشاریہ, ret);
			أرجع ret;
		}

		إذا (spl_fit_image_is_fpga(ctx.fit, گرہ))
			spl_fit_upload_fpga(&ctx, گرہ, &image_info);

		إذا (!spl_fit_image_get_os(ctx.fit, گرہ, &os_type))
			debug("Loadable is %s\n", genimg_get_os_name(os_type));

		إذا (os_takes_devicetree(os_type)) {
			spl_fit_append_fdt(&image_info, info, وتھ, &ctx);
			spl_image->fdt_addr = image_info.fdt_addr;
		}

		/*
		 * If the "firmware" image did not provide an entry point,
		 * use the first valid entry point from the loadables.
		 */
		إذا (spl_image->اندراج_نقطہ == FDT_ERROR &&
		    image_info.اندراج_نقطہ != FDT_ERROR)
			spl_image->اندراج_نقطہ = image_info.اندراج_نقطہ;

		spl_fit_image_ریکارڈ_arm32_optee(ctx.fit, گرہ, spl_image,
						 &image_info);

		/* Record our loadables into the FDT */
		إذا (!CONFIG_IS_ENABLED(FIT_IMAGE_TINY) &&
		    xpl_get_fdt_update(info) && spl_image->fdt_addr)
			spl_fit_ریکارڈ_loadable(&ctx, اشاریہ,
						spl_image->fdt_addr,
						&image_info);
	}

	/*
	 * If a platform does not provide CONFIG_SYS_UBOOT_START, U-Boot's
	 * Makefile will set it to 0 and it will end up as the entry point
	 * here. What it actually means is: use the load address.
	 */
	إذا (spl_image->اندراج_نقطہ == FDT_ERROR || spl_image->اندراج_نقطہ == 0)
		spl_image->اندراج_نقطہ = spl_image->لدو_addr;

	spl_image->flags |= SPL_FIT_لبھیا;
	upl_سیٹ_fit_info(map_منزل_sysmem(ctx.fit), ctx.conf_گرہ,
			 spl_image->اندراج_نقطہ);

	أرجع 0;
}

/* Parse and load full fitImage in SPL */
صحيح spl_لدو_fit_image(بنية spl_image_info *spl_image,
		       ثابت بنية legacy_img_hdr *سرخی)
{
	بنية bootm_سرخیاں images;
	ثابت حرف *fit_uname_config = عدم;
	ulong fdt_hack;
	ثابت حرف *uname;
	ulong fw_انکڑے = 0, dt_انکڑے = 0, img_انکڑے = 0;
	ulong fw_len = 0, dt_len = 0, img_len = 0;
	صحيح idx, conf_noffset;
	صحيح ret;

	images.verify = CONFIG_IS_ENABLED(FIT_SIGNATURE);

	ret = fit_image_لدو(&images, virt_منزل_phys((فراغ *)سرخی),
			     عدم, &fit_uname_config, IH_ARCH_DEFAULT,
			     IH_TYPE_FIRMWARE, -1, FIT_لدو_OPTIONAL,
			     &fw_انکڑے, &fw_len);

	إذا (ret < 0) {
		ret = fit_image_لدو(&images, virt_منزل_phys((فراغ *)سرخی),
				     عدم, &fit_uname_config, IH_ARCH_DEFAULT,
				     IH_TYPE_KERNEL, -1, FIT_لدو_OPTIONAL,
				     &fw_انکڑے, &fw_len);
	}

	إذا (ret < 0)
		أرجع ret;

	spl_image->ناپ = fw_len;
	spl_image->لدو_addr = fw_انکڑے;
	إذا (fit_image_get_اندراج(سرخی, ret, &spl_image->اندراج_نقطہ))
		spl_image->اندراج_نقطہ = fw_انکڑے;
	إذا (fit_image_get_os(سرخی, ret, &spl_image->os))
		spl_image->os = IH_OS_INVALID;
	spl_image->name = genimg_get_os_name(spl_image->os);

	debug(PHASE_PROMPT "payload image: %32s load addr: 0x%lx size: %d\n",
	      spl_image->name, spl_image->لدو_addr, spl_image->ناپ);

	images.verify = CONFIG_IS_ENABLED(FIT_SIGNATURE);

	ret = fit_image_لدو(&images, virt_منزل_phys((فراغ *)سرخی), عدم,
			     &fit_uname_config, IH_ARCH_DEFAULT, IH_TYPE_FLATDT,
			     -1, FIT_لدو_OPTIONAL, &dt_انکڑے, &dt_len);
	إذا (ret >= 0) {
		إذا (spl_image->os == IH_OS_U_BOOT) {
			/* HACK: U-Boot expects FDT at a specific address */
			fdt_hack = ALIGN(spl_image->لدو_addr + spl_image->ناپ, 8);
			debug("Relocating FDT to %p\n", (فراغ *)fdt_hack);
			حافظہ_نقل(map_sysmem(fdt_hack, dt_len),
			       map_sysmem(dt_انکڑے, 0), dt_len);
			spl_image->fdt_addr = (فراغ *)fdt_hack;
		} وإلا {
			spl_image->fdt_addr = (فراغ *)dt_انکڑے;
		}
	}

	conf_noffset = fit_conf_get_گرہ((ثابت فراغ *)سرخی,
					 fit_uname_config);
	إذا (conf_noffset < 0)
		أرجع 0;

	لكل (idx = 0;
	     uname = fdt_stringlist_get((ثابت فراغ *)سرخی, conf_noffset,
					FIT_LOADABLE_PROP, idx,
				عدم), uname;
	     idx++) {
		images.verify = CONFIG_IS_ENABLED(FIT_SIGNATURE);

		ret = fit_image_لدو(&images, virt_منزل_phys((فراغ *)سرخی),
				     &uname, &fit_uname_config,
				     IH_ARCH_DEFAULT, IH_TYPE_LOADABLE, -1,
				     FIT_لدو_OPTIONAL_NON_ZERO,
				     &img_انکڑے, &img_len);
		إذا (ret < 0)
			أرجع ret;
	}
	spl_image->flags |= SPL_FIT_لبھیا;

	upl_سیٹ_fit_info(map_منزل_sysmem(سرخی), conf_noffset,
			 spl_image->اندراج_نقطہ);

	أرجع 0;
}
