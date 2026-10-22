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

DECLARE_GLOBAL_معلومات_PTR;

بنية spl_fit_info {
	ثابت فراغ *fit;	/* Pointer to a valid FIT blob */
	size_t ext_معلومات_وٿي;	/* Offset to FIT external data (end of FIT) */
	صحيح images_ڳنڍ;	/* FDT offset to "/images" node */
	صحيح conf_ڳنڍ;		/* FDT offset to selected configuration node */
};

__weak ulong board_spl_fit_سائيز_align(ulong سائيز)
{
	أرجع سائيز;
}

ساكن صحيح ڳوليو_ڳنڍ_from_desc(ثابت فراغ *fit, صحيح ڳنڍ, ثابت حرف *str)
{
	صحيح ٻار;

	إذا (ڳنڍ < 0)
		أرجع -EINVAL;

	/* iterate the FIT nodes and find a matching description */
	لكل (ٻار = fdt_پهريون_subnode(fit, ڳنڍ); ٻار >= 0;
	     ٻار = fdt_ايندڙ_subnode(fit, ٻار)) {
		صحيح len;
		ثابت حرف *desc = fdt_getprop(fit, ٻار, FIT_DESC_PROP, &len);

		إذا (!desc)
			استمر;

		إذا (!strcmp(desc, str))
			أرجع ٻار;
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
				  ثابت حرف *type, صحيح اشاريو,
				  ثابت حرف **outname)
{
	بنية udevice *sysinfo;
	ثابت حرف *name, *str, *پڇاڙي_2;
	__maybe_unused صحيح ڳنڍ;
	صحيح len, i;
	bool مليو = true;

	name = fdt_getprop(ctx->fit, ctx->conf_ڳنڍ, type, &len);
	إذا (!name) {
		debug("cannot find property '%s': %d\n", type, len);
		أرجع -EINVAL;
	}
	/* A string property should be NUL terminated */
	پڇاڙي_2 = name + len - 1;
	إذا (!len || *پڇاڙي_2) {
		debug("malformed property '%s'\n", type);
		أرجع -EINVAL;
	}

	str = name;
	لكل (i = 0; i < اشاريو; i++) {
		str = strchr(str, '\0') + 1;
		إذا (str > پڇاڙي_2) {
			مليو = false;
			اكسر;
		}
	}

	إذا (!مليو && CONFIG_IS_ENABLED(SYSINFO) && !sysinfo_get(&sysinfo)) {
		صحيح rc;
		/*
		 * no string in the property for this index. Check if the
		 * sysinfo-level code can supply one.
		 */
		rc = sysinfo_detect(sysinfo);
		إذا (rc)
			أرجع rc;

		rc = sysinfo_get_fit_loadable(sysinfo, اشاريو - i - 1, type,
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
			صحيح ڳنڍ;
			صحيح images = fdt_رستو_وٿي(ctx->fit, FIT_IMAGES_رستو);

			ڳنڍ = ڳوليو_ڳنڍ_from_desc(ctx->fit, images, str);
			إذا (ڳنڍ > 0)
				str = fdt_get_name(ctx->fit, ڳنڍ, عدم);

			مليو = true;
		}
	}

	إذا (!مليو) {
		debug("no string for index %d\n", اشاريو);
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
ساكن صحيح spl_fit_get_image_ڳنڍ(ثابت بنية spl_fit_info *ctx,
				  ثابت حرف *type, صحيح اشاريو)
{
	ثابت حرف *str;
	صحيح err;
	صحيح ڳنڍ;

	err = spl_fit_get_image_name(ctx, type, اشاريو, &str);
	إذا (err)
		أرجع err;

	debug("%s: '%s'\n", type, str);

	ڳنڍ = fdt_subnode_وٿي(ctx->fit, ctx->images_ڳنڍ, str);
	إذا (ڳنڍ < 0) {
		pr_err("cannot find image node '%s': %d\n", str, ڳنڍ);
		أرجع -EINVAL;
	}

	أرجع ڳنڍ;
}

ساكن u32 get_aligned_image_وٿي(بنية spl_ڀريو_info *info, u32 وٿي)
{
	أرجع ALIGN_هيٺ_ڏانهن(وٿي, spl_get_bl_len(info));
}

ساكن u32 get_aligned_image_overhead(بنية spl_ڀريو_info *info, u32 وٿي)
{
	أرجع وٿي & (spl_get_bl_len(info) - 1);
}

ساكن صحيح get_aligned_image_سائيز(بنية spl_ڀريو_info *info, ulong معلومات_سائيز,
				  u32 وٿي, ulong *aligned_سائيز)
{
	u32 overhead = get_aligned_image_overhead(info, وٿي);

	إذا (معلومات_سائيز > ULONG_MAX - overhead)
		أرجع -EOVERFLOW;
	معلومات_سائيز += overhead;

	إذا (معلومات_سائيز > ULONG_MAX - (spl_get_bl_len(info) - 1))
		أرجع -EOVERFLOW;
	معلومات_سائيز = ALIGN(معلومات_سائيز, spl_get_bl_len(info));

	*aligned_سائيز = معلومات_سائيز;

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
ساكن فراغ fit_fill_image_info(ثابت فراغ *fit, صحيح ڳنڍ,
				بنية spl_image_info *image_info,
				ulong ڀريو_addr, ulong سائيز)
{
	ulong داخلا_نقطو;

	إذا (!image_info)
		أرجع;

	image_info->ڀريو_addr = ڀريو_addr;
	image_info->سائيز = سائيز;

	إذا (!fit_image_get_داخلا(fit, ڳنڍ, &داخلا_نقطو))
		image_info->داخلا_نقطو = داخلا_نقطو;
	وإلا
		image_info->داخلا_نقطو = FDT_ERROR;
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
ساكن صحيح ڀريو_simple_fit(بنية spl_ڀريو_info *info, ulong fit_وٿي,
			   ثابت بنية spl_fit_info *ctx, صحيح ڳنڍ,
			   بنية spl_image_info *image_info, ulong max_سائيز)
{
	u32 وٿي;
	u32 len;
	size_t ڊيگهائي;
	ulong سائيز;
	ulong ڀريو_addr;
	فراغ *ڀريو_ptr;
	فراغ *src;
	uint8_t image_comp = -1, type = -1;
	ثابت فراغ *معلومات;
	ثابت فراغ *fit = ctx->fit;
	bool external_معلومات = false;
	صحيح ret;

	log_debug("starting\n");
	إذا (CONFIG_IS_ENABLED(BOOTMETH_VBE) &&
	    xpl_get_phase(info) != IH_PHASE_NONE) {
		تعداد image_phase_t phase;

		ret = fit_image_get_phase(fit, ڳنڍ, &phase);
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
		إذا (fit_image_get_type(fit, ڳنڍ, &type))
			puts("Cannot get image type.\n");
		وإلا
			debug("%s ", genimg_get_type_name(type));
	}

	إذا (spl_decompression_enabled()) {
		fit_image_get_comp(fit, ڳنڍ, &image_comp);
		debug("%s ", genimg_get_comp_name(image_comp));
	}

	إذا (fit_image_get_ڀريو(fit, ڳنڍ, &ڀريو_addr)) {
		إذا (!image_info->ڀريو_addr) {
			لکو("Can't load %s: No load address and no buffer\n",
			       fit_get_name(fit, ڳنڍ, عدم));
			أرجع -ENOBUFS;
		}
		ڀريو_addr = image_info->ڀريو_addr;
	}

	إذا (!fit_image_get_معلومات_position(fit, ڳنڍ, &وٿي)) {
		external_معلومات = true;
	} وإلا إذا (!fit_image_get_معلومات_وٿي(fit, ڳنڍ, &وٿي)) {
		/* The resulting offset cannot exceed UINT32_MAX */
		إذا (ctx->ext_معلومات_وٿي > UINT32_MAX - وٿي) {
			log_debug("Invalid external data offset: %u\n", وٿي);
			أرجع -EINVAL;
		}
		log_debug("read offset %x = offset from fit %x\n", وٿي,
			  (u32)(وٿي + ctx->ext_معلومات_وٿي));
		وٿي += ctx->ext_معلومات_وٿي;
		external_معلومات = true;
	}

	إذا (external_معلومات) {
		u32 aligned_وٿي;
		ulong read_وٿي;
		فراغ *src_ptr;

		/* External data */
		إذا (fit_image_get_معلومات_سائيز(fit, ڳنڍ, &len))
			أرجع -ENOENT;

		/* Dont bother to copy 0 byte data, but warn, though */
		إذا (!len) {
			log_warning("%s: Skip load '%s': image size is 0!\n",
				    __func__, fit_get_name(fit, ڳنڍ, عدم));
			fit_fill_image_info(fit, ڳنڍ, image_info, ڀريو_addr, 0);
			أرجع 0;
		}

		/*
		 * data-size is excluded from the configuration signature (it
		 * is in exc_prop[] in image-fit-sig.c), so it stays attacker
		 * controlled even after fit_config_verify() succeeds. The
		 * image hash is only verified after the device read below, so
		 * an oversized value has to be rejected here.
		 */
		ret = get_aligned_image_سائيز(info, len, وٿي, &سائيز);
		إذا (ret) {
			log_debug("Invalid external data size: %u\n", len);
			أرجع ret;
		}

		إذا (سائيز > max_سائيز) {
			log_debug("Image too large: aligned size %lu, max %lu (data-size %u)\n",
				  سائيز, max_سائيز, len);
			أرجع -EFBIG;
		}

		aligned_وٿي = get_aligned_image_وٿي(info, وٿي);
		إذا (aligned_وٿي > ULONG_MAX - fit_وٿي) {
			log_debug("Invalid aligned external data offset: %u\n",
				  aligned_وٿي);
			أرجع -EINVAL;
		}
		read_وٿي = fit_وٿي + aligned_وٿي;

		إذا (spl_decompression_enabled() &&
		    (image_comp == IH_COMP_GZIP || image_comp == IH_COMP_LZMA))
			src_ptr = map_sysmem(ALIGN(CONFIG_SYS_ڀريو_ADDR, ARCH_DMA_MINALIGN), len);
		وإلا
			src_ptr = map_sysmem(ALIGN(ڀريو_addr, ARCH_DMA_MINALIGN), len);
		ڊيگهائي = len;

		/*
		 * For non-signed FIT images, we can check that
		 * (read_offset + size) does not wrap and that
		 * (src_ptr + size) does not exceed the addressable range.
		 * For signed FITs, we can additionally check that
		 * (offset + len) doesn't exceed the allowed FIT image
		 * maximum size.
		 */
		إذا (سائيز > ULONG_MAX - read_وٿي ||
		    سائيز > UINTPTR_MAX - (uintptr_t)src_ptr
		/*
		 * #if (not a runtime if) is required: FIT_SIGNATURE_MAX_SIZE
		 * depends on FIT_SIGNATURE, so CONFIG_VAL(FIT_SIGNATURE_MAX_SIZE)
		 * is undefined when signing is disabled and referencing it
		 * here would fail to compile.
		 */
#إذا CONFIG_IS_ENABLED(FIT_SIGNATURE)
		    || وٿي > CONFIG_VAL(FIT_SIGNATURE_MAX_سائيز) ||
		    len > CONFIG_VAL(FIT_SIGNATURE_MAX_سائيز) - وٿي
#جيڪڏهن_پورو
		) {
			log_debug("FIT external data is out of bounds (offset=%u, size=%u)\n",
				  وٿي, len);
			أرجع -EINVAL;
		}

		log_debug("reading from offset %x / %lx size %lx to %p: ",
			  وٿي, read_وٿي, سائيز, src_ptr);

		إذا (info->اقرأ(info, read_وٿي, سائيز, src_ptr) < ڊيگهائي)
			أرجع -EIO;

		debug("External data: dst=%p, offset=%x, size=%lx\n",
		      src_ptr, وٿي, (بنانشان ڊگهو)ڊيگهائي);
		src = src_ptr + get_aligned_image_overhead(info, وٿي);
	} وإلا {
		/* Embedded data */
		إذا (fit_image_get_emb_معلومات(fit, ڳنڍ, &معلومات, &ڊيگهائي)) {
			puts("Cannot get image data/size\n");
			أرجع -ENOENT;
		}
		debug("Embedded data: dst=%lx, size=%lx\n", ڀريو_addr,
		      (بنانشان ڊگهو)ڊيگهائي);
		src = (فراغ *)معلومات;	/* cast away const */
	}

	إذا (CONFIG_IS_ENABLED(FIT_SIGNATURE)) {
		لکو("## Checking hash(es) for Image %s ... ",
		       fit_get_name(fit, ڳنڍ, عدم));
		إذا (!fit_image_verify_with_معلومات(fit, ڳنڍ, gd_fdt_blob(), src,
						ڊيگهائي))
			أرجع -EPERM;
		puts("OK\n");
	}

	إذا (CONFIG_IS_ENABLED(FIT_IMAGE_POST_PROCESS))
		board_fit_image_post_process(fit, ڳنڍ, &src, &ڊيگهائي);

	ڀريو_ptr = map_sysmem(ڀريو_addr, ڊيگهائي);
	إذا (IS_ENABLED(CONFIG_SPL_GZIP) && image_comp == IH_COMP_GZIP) {
		سائيز = ڊيگهائي;
		إذا (gunzip(ڀريو_ptr, CONFIG_SYS_BOOTM_LEN, src, &سائيز)) {
			puts("Uncompressing error\n");
			أرجع -EIO;
		}
		ڊيگهائي = سائيز;
	} وإلا إذا (IS_ENABLED(CONFIG_SPL_LZMA) && image_comp == IH_COMP_LZMA) {
		سائيز = CONFIG_SYS_BOOTM_LEN;
		ulong ڀريو_پڇاڙي;

		إذا (image_decomp(IH_COMP_LZMA, CONFIG_SYS_ڀريو_ADDR, 0, 0,
				 ڀريو_ptr, src, ڊيگهائي, سائيز, &ڀريو_پڇاڙي)) {
			puts("Uncompressing error\n");
			أرجع -EIO;
		}
		ڊيگهائي = ڀريو_پڇاڙي - CONFIG_SYS_ڀريو_ADDR;
	} وإلا {
		memmove(ڀريو_ptr, src, ڊيگهائي);
	}

	fit_fill_image_info(fit, ڳنڍ, image_info, ڀريو_addr, ڊيگهائي);
	log_debug("- done loading\n");

	upl_جوڙيو_image(fit, ڳنڍ, ڀريو_addr, ڊيگهائي);

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
			      بنية spl_ڀريو_info *info, ulong وٿي,
			      ثابت بنية spl_fit_info *ctx)
{
	بنية spl_image_info image_info;
	صحيح ڳنڍ, ret = 0, اشاريو = 0;

	/*
	 * Use the address following the image as target address for the
	 * device tree.
	 */
	image_info.ڀريو_addr = ALIGN(spl_image->ڀريو_addr + spl_image->سائيز, 8);

	/* Figure out which device tree the board wants to use */
	ڳنڍ = spl_fit_get_image_ڳنڍ(ctx, FIT_FDT_PROP, اشاريو++);
	إذا (ڳنڍ < 0) {
		size_t سائيز;

		debug("%s: cannot find FDT node\n", __func__);

		/*
		 * U-Boot did not find a device tree inside the FIT image. Use
		 * the U-Boot device tree instead.
		 */
		إذا (!gd->fdt_blob)
			أرجع ڳنڍ;

		/*
		 * Make the load-address of the FDT available for the SPL
		 * framework
		 */
		سائيز = fdt_totalsize(gd->fdt_blob);
		spl_image->fdt_addr = map_sysmem(image_info.ڀريو_addr, سائيز);
		يادگيري_نقل(spl_image->fdt_addr, gd->fdt_blob, سائيز);
	} وإلا {
		ret = ڀريو_simple_fit(info, وٿي, ctx, ڳنڍ, &image_info,
				      CONFIG_SYS_BOOTM_LEN);
		إذا (ret < 0)
			أرجع ret;

		spl_image->fdt_addr = phys_منزل_virt(image_info.ڀريو_addr);
	}

	إذا (CONFIG_IS_ENABLED(FIT_IMAGE_TINY))
		أرجع 0;

#إذا CONFIG_IS_ENABLED(ڀريو_FIT_APPLY_OVERLAY)
		فراغ *tmpbuffer = عدم;

		لكل (; ; اشاريو++) {
			ثابت حرف *str;

			ret = spl_fit_get_image_name(ctx, FIT_FDT_PROP, اشاريو, &str);
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

			ڳنڍ = fdt_subnode_وٿي(ctx->fit, ctx->images_ڳنڍ, str);
			إذا (ڳنڍ < 0) {
				debug("%s: unable to find FDT node %d\n",
				      __func__, اشاريو);
				استمر;
			}

			إذا (!tmpbuffer) {
				/*
				 * allocate memory to store the DT overlay
				 * before it is applied. It may not be used
				 * depending on how the overlay is stored, so
				 * don't fail yet if the allocation failed.
				 */
				size_t سائيز = CONFIG_SPL_ڀريو_FIT_APPLY_OVERLAY_BUF_SZ;

				tmpbuffer = malloc_cache_aligned(سائيز);
				إذا (!tmpbuffer)
					debug("%s: unable to allocate space for overlays\n",
					      __func__);
			}
			image_info.ڀريو_addr = (ulong)tmpbuffer;
			ret = ڀريو_simple_fit(info, وٿي, ctx, ڳنڍ,
					      &image_info,
					      CONFIG_SPL_ڀريو_FIT_APPLY_OVERLAY_BUF_SZ);
			إذا (ret == -EBADSLT)
				استمر;
			وإلا إذا (ret < 0)
				اكسر;

			/* Make room in FDT for changes from the overlay */
			ret = fdt_increase_سائيز(spl_image->fdt_addr,
						image_info.سائيز);
			إذا (ret < 0)
				اكسر;

			ret = fdt_overlay_apply_verbose(spl_image->fdt_addr,
							(فراغ *)image_info.ڀريو_addr);
			إذا (ret) {
				pr_err("failed to apply DT overlay %s\n",
				       fit_get_name(ctx->fit, ڳنڍ, عدم));
				اكسر;
			}

			debug("%s: DT overlay %s applied\n", __func__,
			      fit_get_name(ctx->fit, ڳنڍ, عدم));
		}
		حرر(tmpbuffer);
		إذا (ret)
			أرجع ret;
#جيڪڏهن_پورو
	/* Try to make space, so we can inject details on the loadables */
	ret = fdt_shrink_منزل_minimum(spl_image->fdt_addr, 8192);
	إذا (ret < 0)
		أرجع ret;

	أرجع ret;
}

ساكن صحيح spl_fit_رڪارڊ_loadable(ثابت بنية spl_fit_info *ctx, صحيح اشاريو,
				   فراغ *blob, بنية spl_image_info *image)
{
	صحيح ret = 0;
	ثابت حرف *name;
	صحيح ڳنڍ;

	ret = spl_fit_get_image_name(ctx, FIT_LOADABLE_PROP, اشاريو, &name);
	إذا (ret < 0)
		أرجع ret;

	ڳنڍ = spl_fit_get_image_ڳنڍ(ctx, FIT_LOADABLE_PROP, اشاريو);

	ret = fdt_رڪارڊ_loadable(blob, اشاريو, name, image->ڀريو_addr,
			image->سائيز, image->داخلا_نقطو,
			fdt_getprop(ctx->fit, ڳنڍ, FIT_TYPE_PROP, عدم),
			fdt_getprop(ctx->fit, ڳنڍ, FIT_OS_PROP, عدم),
			fdt_getprop(ctx->fit, ڳنڍ, FIT_ARCH_PROP, عدم));

	أرجع ret;
}

ساكن صحيح spl_fit_image_is_fpga(ثابت فراغ *fit, صحيح ڳنڍ)
{
	ثابت حرف *type;

	إذا (!IS_ENABLED(CONFIG_SPL_FPGA))
		أرجع 0;

	type = fdt_getprop(fit, ڳنڍ, FIT_TYPE_PROP, عدم);
	إذا (!type)
		أرجع 0;

	أرجع !strcmp(type, "fpga");
}

ساكن فراغ spl_fit_image_رڪارڊ_arm32_optee(ثابت فراغ *fit, صحيح ڳنڍ,
					     بنية spl_image_info *spl_image,
					     بنية spl_image_info *image_info)
{
#إذا defined(CONFIG_BOOTM_OPTEE) && defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
	ثابت حرف *type = fdt_getprop(fit, ڳنڍ, FIT_TYPE_PROP, عدم);

	إذا (!type)
		أرجع;

	إذا (strcmp(type, "tee"))
		أرجع;

	spl_image->optee_addr = image_info->ڀريو_addr;
#جيڪڏهن_پورو
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
__weak فراغ *board_spl_fit_بفر_addr(ulong fit_سائيز, صحيح sectors, صحيح bl_len)
{
	فراغ *buf;
	size_t سائيز = sectors * bl_len;

	buf = malloc_cache_aligned(سائيز);
	إذا (!buf) {
		pr_err("Could not get FIT buffer of %lu bytes\n", (ulong)سائيز);

		إذا (IS_ENABLED(CONFIG_SPL_SYS_MALLOC))
			pr_err("\tcheck CONFIG_SPL_SYS_MALLOC_SIZE\n");
		وإلا
			pr_err("\tcheck CONFIG_SPL_SYS_MALLOC_F_LEN\n");

		buf = spl_get_ڀريو_بفر(0, سائيز);
	}
	أرجع buf;
}

/*
 * Weak default function to allow customizing SPL fit loading for load-only
 * use cases by allowing to skip the parsing/processing of the FIT contents
 * (so that this can be done separately in a more customized fashion)
 */
__weak bool spl_ڀريو_simple_fit_skip_processing(فراغ)
{
	أرجع false;
}

/*
 * Weak default function to allow fixes after fit header
 * is loaded.
 */
__weak فراغ *spl_ڀريو_simple_fit_fix_ڀريو(ثابت فراغ *fit)
{
	أرجع (فراغ *)fit;
}

ساكن فراغ warn_deprecated(ثابت حرف *نياپو)
{
	لکو("DEPRECATED: %s\n", نياپو);
	لکو("\tSee https://fitspec.osfw.foundation/\n");
}

ساكن صحيح spl_fit_upload_fpga(بنية spl_fit_info *ctx, صحيح ڳنڍ,
			       بنية spl_image_info *fpga_image)
{
	ثابت حرف *compatible;
	صحيح ret;
	صحيح devnum = 0;
	صحيح flags = 0;

	debug("FPGA bitstream at: %x, size: %x\n",
	      (u32)fpga_image->ڀريو_addr, fpga_image->سائيز);

	compatible = fdt_getprop(ctx->fit, ڳنڍ, FIT_COMPAT_PROP, عدم);
	إذا (!compatible) {
		warn_deprecated("'fpga' image without 'compatible' property");
	} وإلا {
		إذا (CONFIG_IS_ENABLED(FPGA_ڀريو_SECURE))
			flags = fpga_compatible2flag(devnum, compatible);
		إذا (strcmp(compatible, "u-boot,fpga-legacy"))
			debug("Ignoring compatible = %s property\n",
			      compatible);
	}

	ret = fpga_ڀريو(devnum, (فراغ *)fpga_image->ڀريو_addr,
			fpga_image->سائيز, ٻٽو_انگ_ڀريل, flags);
	إذا (ret) {
		لکو("%s: Cannot load the image to the FPGA\n", __func__);
		أرجع ret;
	}

	puts("FPGA image loaded from FIT\n");

	أرجع 0;
}

ساكن صحيح spl_fit_ڀريو_fpga(بنية spl_fit_info *ctx,
			     بنية spl_ڀريو_info *info, ulong وٿي)
{
	صحيح ڳنڍ, ret;

	بنية spl_image_info fpga_image = {
		.ڀريو_addr = 0,
	};

	ڳنڍ = spl_fit_get_image_ڳنڍ(ctx, FIT_FPGA_PROP, 0);
	إذا (ڳنڍ < 0)
		أرجع ڳنڍ;

	warn_deprecated("'fpga' property in config node. Use 'loadables'");

	/* Load the image and set up the fpga_image structure */
	ret = ڀريو_simple_fit(info, وٿي, ctx, ڳنڍ, &fpga_image,
			      CONFIG_SYS_BOOTM_LEN);
	إذا (ret) {
		لکو("%s: Cannot load the FPGA: %i\n", __func__, ret);
		أرجع ret;
	}

	أرجع spl_fit_upload_fpga(ctx, ڳنڍ, &fpga_image);
}

ساكن صحيح spl_simple_fit_read(بنية spl_fit_info *ctx,
			       بنية spl_ڀريو_info *info, ulong وٿي,
			       ثابت فراغ *fit_سرو)
{
	بنانشان ڊگهو aligned_سائيز, ڳڻپ, سائيز;
	فراغ *buf;
	صحيح ret;

	/*
	 * For FIT with external data, figure out where the external images
	 * start. This is the base for the data-offset properties in each
	 * image.
	 */
	سائيز = ALIGN(fdt_totalsize(fit_سرو), 4);
	سائيز = board_spl_fit_سائيز_align(سائيز);
	ctx->ext_معلومات_وٿي = ALIGN(سائيز, 4);

	/*
	 * So far we only have one block of data from the FIT. Read the entire
	 * thing, including that first block.
	 *
	 * For FIT with data embedded, data is loaded as part of FIT image.
	 * For FIT with external data, data is not loaded in this step.
	 */
	ret = get_aligned_image_سائيز(info, سائيز, 0, &aligned_سائيز);
	إذا (ret) {
		log_debug("Invalid FIT size: %lu\n", سائيز);
		أرجع ret;
	}
	buf = board_spl_fit_بفر_addr(aligned_سائيز, aligned_سائيز, 1);
	إذا (!buf) {
		/*
		 * We assume that none of the board will ever use 0x0 as a
		 * valid load address. Theoretically some board could use it,
		 * but this is extremely unlikely.
		 */
		أرجع -EIO;
	}

	ڳڻپ = info->اقرأ(info, وٿي, aligned_سائيز, buf);
	إذا (!ڳڻپ) {
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
	      وٿي, aligned_سائيز, buf, ڳڻپ);

	أرجع 0;
}

ساكن صحيح spl_simple_fit_parse(بنية spl_fit_info *ctx)
{
	/* Find the correct subnode under "/configurations" */
	ctx->conf_ڳنڍ = fit_ڳوليو_config_ڳنڍ(ctx->fit);
	إذا (ctx->conf_ڳنڍ < 0)
		أرجع -EINVAL;

	إذا (CONFIG_IS_ENABLED(FIT_SIGNATURE)) {
		لکو("## Checking hash(es) for config %s ... ",
		       fit_get_name(ctx->fit, ctx->conf_ڳنڍ, عدم));
		إذا (fit_config_verify(ctx->fit, ctx->conf_ڳنڍ))
			أرجع -EPERM;
		puts("OK\n");
	}

	/* find the node holding the images information */
	ctx->images_ڳنڍ = fdt_رستو_وٿي(ctx->fit, FIT_IMAGES_رستو);
	إذا (ctx->images_ڳنڍ < 0) {
		debug("%s: Cannot find /images node: %d\n", __func__,
		      ctx->images_ڳنڍ);
		أرجع -EINVAL;
	}

	أرجع 0;
}

صحيح spl_ڀريو_simple_fit(بنية spl_image_info *spl_image,
			بنية spl_ڀريو_info *info, ulong وٿي, فراغ *fit)
{
	بنية spl_image_info image_info;
	بنية spl_fit_info ctx;
	صحيح ڳنڍ = -1;
	صحيح ret;
	صحيح اشاريو = 0;
	صحيح firmware_ڳنڍ;

	ret = spl_simple_fit_read(&ctx, info, وٿي, fit);
	إذا (ret < 0)
		أرجع ret;

	/* skip further processing if requested to enable load-only use cases */
	إذا (spl_ڀريو_simple_fit_skip_processing())
		أرجع 0;

	ctx.fit = spl_ڀريو_simple_fit_fix_ڀريو(ctx.fit);

	ret = spl_simple_fit_parse(&ctx);
	إذا (ret < 0)
		أرجع ret;

	إذا (IS_ENABLED(CONFIG_SPL_FPGA))
		spl_fit_ڀريو_fpga(&ctx, info, وٿي);

	/*
	 * Find the U-Boot image using the following search order:
	 *   - start at 'firmware' (e.g. an ARM Trusted Firmware)
	 *   - fall back 'kernel' (e.g. a Falcon-mode OS boot
	 *   - fall back to using the first 'loadables' entry
	 */
	إذا (ڳنڍ < 0)
		ڳنڍ = spl_fit_get_image_ڳنڍ(&ctx, FIT_FIRMWARE_PROP, 0);

	إذا (ڳنڍ < 0 && IS_ENABLED(CONFIG_SPL_OS_BOOT))
		ڳنڍ = spl_fit_get_image_ڳنڍ(&ctx, FIT_KERNEL_PROP, 0);

	إذا (ڳنڍ < 0) {
		debug("could not find firmware image, trying loadables...\n");
		ڳنڍ = spl_fit_get_image_ڳنڍ(&ctx, FIT_LOADABLE_PROP, 0);
		/*
		 * If we pick the U-Boot image from "loadables", start at
		 * the second image when later loading additional images.
		 */
		اشاريو = 1;
	}
	إذا (ڳنڍ < 0) {
		debug("%s: Cannot find u-boot image node: %d\n",
		      __func__, ڳنڍ);
		أرجع -1;
	}

	/* Load the image and set up the spl_image structure */
	ret = ڀريو_simple_fit(info, وٿي, &ctx, ڳنڍ, spl_image,
			      CONFIG_SYS_BOOTM_LEN);
	إذا (ret)
		أرجع ret;

	/*
	 * For backward compatibility, we treat the first node that is
	 * as a U-Boot image, if no OS-type has been declared.
	 */
	إذا (!spl_fit_image_get_os(ctx.fit, ڳنڍ, &spl_image->os))
		debug("Image OS is %s\n", genimg_get_os_name(spl_image->os));
	وإلا إذا (!IS_ENABLED(CONFIG_SPL_OS_BOOT))
		spl_image->os = IH_OS_U_BOOT;

	/*
	 * Booting a next-stage U-Boot may require us to append the FDT.
	 * We allow this to fail, as the U-Boot image might embed its FDT.
	 */
	إذا (os_takes_devicetree(spl_image->os)) {
		ret = spl_fit_append_fdt(spl_image, info, وٿي, &ctx);
		إذا (ret < 0 && spl_image->os != IH_OS_U_BOOT)
			أرجع ret;
	}

	firmware_ڳنڍ = ڳنڍ;
	/* Now check if there are more images for us to load */
	لكل (; ; اشاريو++) {
		uint8_t os_type = IH_OS_INVALID;

		ڳنڍ = spl_fit_get_image_ڳنڍ(&ctx, FIT_LOADABLE_PROP, اشاريو);
		إذا (ڳنڍ < 0)
			اكسر;

		/*
		 * if the firmware is also a loadable, skip it because
		 * it already has been loaded. This is typically the case with
		 * u-boot.img generated by mkimage.
		 */
		إذا (firmware_ڳنڍ == ڳنڍ)
			استمر;

		image_info.ڀريو_addr = 0;
		ret = ڀريو_simple_fit(info, وٿي, &ctx, ڳنڍ, &image_info,
				      CONFIG_SYS_BOOTM_LEN);
		إذا (ret < 0 && ret != -EBADSLT) {
			لکو("%s: can't load image loadables index %d (ret = %d)\n",
			       __func__, اشاريو, ret);
			أرجع ret;
		}

		إذا (spl_fit_image_is_fpga(ctx.fit, ڳنڍ))
			spl_fit_upload_fpga(&ctx, ڳنڍ, &image_info);

		إذا (!spl_fit_image_get_os(ctx.fit, ڳنڍ, &os_type))
			debug("Loadable is %s\n", genimg_get_os_name(os_type));

		إذا (os_takes_devicetree(os_type)) {
			spl_fit_append_fdt(&image_info, info, وٿي, &ctx);
			spl_image->fdt_addr = image_info.fdt_addr;
		}

		/*
		 * If the "firmware" image did not provide an entry point,
		 * use the first valid entry point from the loadables.
		 */
		إذا (spl_image->داخلا_نقطو == FDT_ERROR &&
		    image_info.داخلا_نقطو != FDT_ERROR)
			spl_image->داخلا_نقطو = image_info.داخلا_نقطو;

		spl_fit_image_رڪارڊ_arm32_optee(ctx.fit, ڳنڍ, spl_image,
						 &image_info);

		/* Record our loadables into the FDT */
		إذا (!CONFIG_IS_ENABLED(FIT_IMAGE_TINY) &&
		    xpl_get_fdt_update(info) && spl_image->fdt_addr)
			spl_fit_رڪارڊ_loadable(&ctx, اشاريو,
						spl_image->fdt_addr,
						&image_info);
	}

	/*
	 * If a platform does not provide CONFIG_SYS_UBOOT_START, U-Boot's
	 * Makefile will set it to 0 and it will end up as the entry point
	 * here. What it actually means is: use the load address.
	 */
	إذا (spl_image->داخلا_نقطو == FDT_ERROR || spl_image->داخلا_نقطو == 0)
		spl_image->داخلا_نقطو = spl_image->ڀريو_addr;

	spl_image->flags |= SPL_FIT_مليو;
	upl_سيٽ_fit_info(map_منزل_sysmem(ctx.fit), ctx.conf_ڳنڍ,
			 spl_image->داخلا_نقطو);

	أرجع 0;
}

/* Parse and load full fitImage in SPL */
صحيح spl_ڀريو_fit_image(بنية spl_image_info *spl_image,
		       ثابت بنية legacy_img_hdr *سرو)
{
	بنية bootm_سرا images;
	ثابت حرف *fit_uname_config = عدم;
	ulong fdt_hack;
	ثابت حرف *uname;
	ulong fw_معلومات = 0, dt_معلومات = 0, img_معلومات = 0;
	ulong fw_len = 0, dt_len = 0, img_len = 0;
	صحيح idx, conf_noffset;
	صحيح ret;

	images.verify = CONFIG_IS_ENABLED(FIT_SIGNATURE);

	ret = fit_image_ڀريو(&images, virt_منزل_phys((فراغ *)سرو),
			     عدم, &fit_uname_config, IH_ARCH_DEFAULT,
			     IH_TYPE_FIRMWARE, -1, FIT_ڀريو_OPTIONAL,
			     &fw_معلومات, &fw_len);

	إذا (ret < 0) {
		ret = fit_image_ڀريو(&images, virt_منزل_phys((فراغ *)سرو),
				     عدم, &fit_uname_config, IH_ARCH_DEFAULT,
				     IH_TYPE_KERNEL, -1, FIT_ڀريو_OPTIONAL,
				     &fw_معلومات, &fw_len);
	}

	إذا (ret < 0)
		أرجع ret;

	spl_image->سائيز = fw_len;
	spl_image->ڀريو_addr = fw_معلومات;
	إذا (fit_image_get_داخلا(سرو, ret, &spl_image->داخلا_نقطو))
		spl_image->داخلا_نقطو = fw_معلومات;
	إذا (fit_image_get_os(سرو, ret, &spl_image->os))
		spl_image->os = IH_OS_INVALID;
	spl_image->name = genimg_get_os_name(spl_image->os);

	debug(PHASE_PROMPT "payload image: %32s load addr: 0x%lx size: %d\n",
	      spl_image->name, spl_image->ڀريو_addr, spl_image->سائيز);

	images.verify = CONFIG_IS_ENABLED(FIT_SIGNATURE);

	ret = fit_image_ڀريو(&images, virt_منزل_phys((فراغ *)سرو), عدم,
			     &fit_uname_config, IH_ARCH_DEFAULT, IH_TYPE_FLATDT,
			     -1, FIT_ڀريو_OPTIONAL, &dt_معلومات, &dt_len);
	إذا (ret >= 0) {
		إذا (spl_image->os == IH_OS_U_BOOT) {
			/* HACK: U-Boot expects FDT at a specific address */
			fdt_hack = ALIGN(spl_image->ڀريو_addr + spl_image->سائيز, 8);
			debug("Relocating FDT to %p\n", (فراغ *)fdt_hack);
			يادگيري_نقل(map_sysmem(fdt_hack, dt_len),
			       map_sysmem(dt_معلومات, 0), dt_len);
			spl_image->fdt_addr = (فراغ *)fdt_hack;
		} وإلا {
			spl_image->fdt_addr = (فراغ *)dt_معلومات;
		}
	}

	conf_noffset = fit_conf_get_ڳنڍ((ثابت فراغ *)سرو,
					 fit_uname_config);
	إذا (conf_noffset < 0)
		أرجع 0;

	لكل (idx = 0;
	     uname = fdt_stringlist_get((ثابت فراغ *)سرو, conf_noffset,
					FIT_LOADABLE_PROP, idx,
				عدم), uname;
	     idx++) {
		images.verify = CONFIG_IS_ENABLED(FIT_SIGNATURE);

		ret = fit_image_ڀريو(&images, virt_منزل_phys((فراغ *)سرو),
				     &uname, &fit_uname_config,
				     IH_ARCH_DEFAULT, IH_TYPE_LOADABLE, -1,
				     FIT_ڀريو_OPTIONAL_NON_ZERO,
				     &img_معلومات, &img_len);
		إذا (ret < 0)
			أرجع ret;
	}
	spl_image->flags |= SPL_FIT_مليو;

	upl_سيٽ_fit_info(map_منزل_sysmem(سرو), conf_noffset,
			 spl_image->داخلا_نقطو);

	أرجع 0;
}
