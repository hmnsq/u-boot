// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <fpga.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <gzip.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <image.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <log.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <memalign.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <mapmem.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <spl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <upl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sysinfo.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/global_data.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/io.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/libfdt.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/printk.h>

DECLARE_GLOBAL_ਅੰਕੜੇ_PTR;

ਬਣਤਰ spl_fit_info {
	ਅਚਲ ਖਾਲੀ *fit;	/* Pointer to a valid FIT blob */
	size_t ext_ਅੰਕੜੇ_ਵਿੱਥ;	/* Offset to FIT external data (end of FIT) */
	ਪੂਰਨ images_ਗੰਢ;	/* FDT offset to "/images" node */
	ਪੂਰਨ conf_ਗੰਢ;		/* FDT offset to selected configuration node */
};

__weak ulong board_spl_fit_ਮਾਪ_align(ulong ਮਾਪ)
{
	ਵਾਪਸ ਮਾਪ;
}

ਸਥਿਰ ਪੂਰਨ ਲੱਭੋ_ਗੰਢ_from_desc(ਅਚਲ ਖਾਲੀ *fit, ਪੂਰਨ ਗੰਢ, ਅਚਲ ਅੱਖਰ *str)
{
	ਪੂਰਨ ਬੱਚਾ;

	ਜੇ (ਗੰਢ < 0)
		ਵਾਪਸ -EINVAL;

	/* iterate the FIT nodes and find a matching description */
	ਲਈ (ਬੱਚਾ = fdt_ਪਹਿਲਾ_subnode(fit, ਗੰਢ); ਬੱਚਾ >= 0;
	     ਬੱਚਾ = fdt_ਅਗਲਾ_subnode(fit, ਬੱਚਾ)) {
		ਪੂਰਨ len;
		ਅਚਲ ਅੱਖਰ *desc = fdt_getprop(fit, ਬੱਚਾ, FIT_DESC_PROP, &len);

		ਜੇ (!desc)
			ਜਾਰੀ;

		ਜੇ (!strcmp(desc, str))
			ਵਾਪਸ ਬੱਚਾ;
	}

	ਵਾਪਸ -ENOENT;
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
ਸਥਿਰ ਪੂਰਨ spl_fit_get_image_name(ਅਚਲ ਬਣਤਰ spl_fit_info *ctx,
				  ਅਚਲ ਅੱਖਰ *type, ਪੂਰਨ ਸੂਚਕਾਂਕ,
				  ਅਚਲ ਅੱਖਰ **outname)
{
	ਬਣਤਰ udevice *sysinfo;
	ਅਚਲ ਅੱਖਰ *name, *str, *ਸਿਰਾ_2;
	__maybe_unused ਪੂਰਨ ਗੰਢ;
	ਪੂਰਨ len, i;
	bool ਮਿਲਿਆ = true;

	name = fdt_getprop(ctx->fit, ctx->conf_ਗੰਢ, type, &len);
	ਜੇ (!name) {
		debug("cannot find property '%s': %d\n", type, len);
		ਵਾਪਸ -EINVAL;
	}
	/* A string property should be NUL terminated */
	ਸਿਰਾ_2 = name + len - 1;
	ਜੇ (!len || *ਸਿਰਾ_2) {
		debug("malformed property '%s'\n", type);
		ਵਾਪਸ -EINVAL;
	}

	str = name;
	ਲਈ (i = 0; i < ਸੂਚਕਾਂਕ; i++) {
		str = strchr(str, '\0') + 1;
		ਜੇ (str > ਸਿਰਾ_2) {
			ਮਿਲਿਆ = false;
			ਰੋਕੋ;
		}
	}

	ਜੇ (!ਮਿਲਿਆ && CONFIG_IS_ENABLED(SYSINFO) && !sysinfo_get(&sysinfo)) {
		ਪੂਰਨ rc;
		/*
		 * no string in the property for this index. Check if the
		 * sysinfo-level code can supply one.
		 */
		rc = sysinfo_detect(sysinfo);
		ਜੇ (rc)
			ਵਾਪਸ rc;

		rc = sysinfo_get_fit_loadable(sysinfo, ਸੂਚਕਾਂਕ - i - 1, type,
					      &str);
		ਜੇ (rc && rc != -ENOENT)
			ਵਾਪਸ rc;

		ਜੇ (!rc) {
			/*
			 * The sysinfo provided a name for a loadable.
			 * Try to match it against the description properties
			 * first. If no matching node is found, use it as a
			 * node name.
			 */
			ਪੂਰਨ ਗੰਢ;
			ਪੂਰਨ images = fdt_ਰਸਤਾ_ਵਿੱਥ(ctx->fit, FIT_IMAGES_ਰਸਤਾ);

			ਗੰਢ = ਲੱਭੋ_ਗੰਢ_from_desc(ctx->fit, images, str);
			ਜੇ (ਗੰਢ > 0)
				str = fdt_get_name(ctx->fit, ਗੰਢ, ਨਿਰਾ);

			ਮਿਲਿਆ = true;
		}
	}

	ਜੇ (!ਮਿਲਿਆ) {
		debug("no string for index %d\n", ਸੂਚਕਾਂਕ);
		ਵਾਪਸ -E2BIG;
	}

	*outname = str;
	ਵਾਪਸ 0;
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
ਸਥਿਰ ਪੂਰਨ spl_fit_get_image_ਗੰਢ(ਅਚਲ ਬਣਤਰ spl_fit_info *ctx,
				  ਅਚਲ ਅੱਖਰ *type, ਪੂਰਨ ਸੂਚਕਾਂਕ)
{
	ਅਚਲ ਅੱਖਰ *str;
	ਪੂਰਨ err;
	ਪੂਰਨ ਗੰਢ;

	err = spl_fit_get_image_name(ctx, type, ਸੂਚਕਾਂਕ, &str);
	ਜੇ (err)
		ਵਾਪਸ err;

	debug("%s: '%s'\n", type, str);

	ਗੰਢ = fdt_subnode_ਵਿੱਥ(ctx->fit, ctx->images_ਗੰਢ, str);
	ਜੇ (ਗੰਢ < 0) {
		pr_err("cannot find image node '%s': %d\n", str, ਗੰਢ);
		ਵਾਪਸ -EINVAL;
	}

	ਵਾਪਸ ਗੰਢ;
}

ਸਥਿਰ u32 get_aligned_image_ਵਿੱਥ(ਬਣਤਰ spl_ਲੱਦੋ_info *info, u32 ਵਿੱਥ)
{
	ਵਾਪਸ ALIGN_ਥੱਲੇ(ਵਿੱਥ, spl_get_bl_len(info));
}

ਸਥਿਰ u32 get_aligned_image_overhead(ਬਣਤਰ spl_ਲੱਦੋ_info *info, u32 ਵਿੱਥ)
{
	ਵਾਪਸ ਵਿੱਥ & (spl_get_bl_len(info) - 1);
}

ਸਥਿਰ ਪੂਰਨ get_aligned_image_ਮਾਪ(ਬਣਤਰ spl_ਲੱਦੋ_info *info, ulong ਅੰਕੜੇ_ਮਾਪ,
				  u32 ਵਿੱਥ, ulong *aligned_ਮਾਪ)
{
	u32 overhead = get_aligned_image_overhead(info, ਵਿੱਥ);

	ਜੇ (ਅੰਕੜੇ_ਮਾਪ > ULONG_MAX - overhead)
		ਵਾਪਸ -EOVERFLOW;
	ਅੰਕੜੇ_ਮਾਪ += overhead;

	ਜੇ (ਅੰਕੜੇ_ਮਾਪ > ULONG_MAX - (spl_get_bl_len(info) - 1))
		ਵਾਪਸ -EOVERFLOW;
	ਅੰਕੜੇ_ਮਾਪ = ALIGN(ਅੰਕੜੇ_ਮਾਪ, spl_get_bl_len(info));

	*aligned_ਮਾਪ = ਅੰਕੜੇ_ਮਾਪ;

	ਵਾਪਸ 0;
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
ਸਥਿਰ ਖਾਲੀ fit_fill_image_info(ਅਚਲ ਖਾਲੀ *fit, ਪੂਰਨ ਗੰਢ,
				ਬਣਤਰ spl_image_info *image_info,
				ulong ਲੱਦੋ_addr, ulong ਮਾਪ)
{
	ulong ਇੰਦਰਾਜ_ਬਿੰਦੂ;

	ਜੇ (!image_info)
		ਵਾਪਸ;

	image_info->ਲੱਦੋ_addr = ਲੱਦੋ_addr;
	image_info->ਮਾਪ = ਮਾਪ;

	ਜੇ (!fit_image_get_ਇੰਦਰਾਜ(fit, ਗੰਢ, &ਇੰਦਰਾਜ_ਬਿੰਦੂ))
		image_info->ਇੰਦਰਾਜ_ਬਿੰਦੂ = ਇੰਦਰਾਜ_ਬਿੰਦੂ;
	ਨਹੀਂਤਾਂ
		image_info->ਇੰਦਰਾਜ_ਬਿੰਦੂ = FDT_ERROR;
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
ਸਥਿਰ ਪੂਰਨ ਲੱਦੋ_simple_fit(ਬਣਤਰ spl_ਲੱਦੋ_info *info, ulong fit_ਵਿੱਥ,
			   ਅਚਲ ਬਣਤਰ spl_fit_info *ctx, ਪੂਰਨ ਗੰਢ,
			   ਬਣਤਰ spl_image_info *image_info, ulong max_ਮਾਪ)
{
	u32 ਵਿੱਥ;
	u32 len;
	size_t ਲੰਮਾਈ;
	ulong ਮਾਪ;
	ulong ਲੱਦੋ_addr;
	ਖਾਲੀ *ਲੱਦੋ_ptr;
	ਖਾਲੀ *src;
	uint8_t image_comp = -1, type = -1;
	ਅਚਲ ਖਾਲੀ *ਅੰਕੜੇ;
	ਅਚਲ ਖਾਲੀ *fit = ctx->fit;
	bool external_ਅੰਕੜੇ = false;
	ਪੂਰਨ ret;

	log_debug("starting\n");
	ਜੇ (CONFIG_IS_ENABLED(BOOTMETH_VBE) &&
	    xpl_get_phase(info) != IH_PHASE_NONE) {
		ਗਿਣਤੀ image_phase_t phase;

		ret = fit_image_get_phase(fit, ਗੰਢ, &phase);
		/* if the image is for any phase, let's use it */
		ਜੇ (ret == -ENOENT || phase == xpl_get_phase(info)) {
			log_debug("found\n");
		} ਨਹੀਂਤਾਂ ਜੇ (ret < 0) {
			log_debug("err=%d\n", ret);
			ਵਾਪਸ ret;
		} ਨਹੀਂਤਾਂ {
			log_debug("- phase mismatch, skipping this image\n");
			ਵਾਪਸ -EBADSLT;
		}
	}

	ਜੇ (IS_ENABLED(CONFIG_SPL_FPGA) ||
	    (IS_ENABLED(CONFIG_SPL_OS_BOOT) && spl_decompression_enabled())) {
		ਜੇ (fit_image_get_type(fit, ਗੰਢ, &type))
			puts("Cannot get image type.\n");
		ਨਹੀਂਤਾਂ
			debug("%s ", genimg_get_type_name(type));
	}

	ਜੇ (spl_decompression_enabled()) {
		fit_image_get_comp(fit, ਗੰਢ, &image_comp);
		debug("%s ", genimg_get_comp_name(image_comp));
	}

	ਜੇ (fit_image_get_ਲੱਦੋ(fit, ਗੰਢ, &ਲੱਦੋ_addr)) {
		ਜੇ (!image_info->ਲੱਦੋ_addr) {
			ਲਿਖੋ("Can't load %s: No load address and no buffer\n",
			       fit_get_name(fit, ਗੰਢ, ਨਿਰਾ));
			ਵਾਪਸ -ENOBUFS;
		}
		ਲੱਦੋ_addr = image_info->ਲੱਦੋ_addr;
	}

	ਜੇ (!fit_image_get_ਅੰਕੜੇ_position(fit, ਗੰਢ, &ਵਿੱਥ)) {
		external_ਅੰਕੜੇ = true;
	} ਨਹੀਂਤਾਂ ਜੇ (!fit_image_get_ਅੰਕੜੇ_ਵਿੱਥ(fit, ਗੰਢ, &ਵਿੱਥ)) {
		/* The resulting offset cannot exceed UINT32_MAX */
		ਜੇ (ctx->ext_ਅੰਕੜੇ_ਵਿੱਥ > UINT32_MAX - ਵਿੱਥ) {
			log_debug("Invalid external data offset: %u\n", ਵਿੱਥ);
			ਵਾਪਸ -EINVAL;
		}
		log_debug("read offset %x = offset from fit %x\n", ਵਿੱਥ,
			  (u32)(ਵਿੱਥ + ctx->ext_ਅੰਕੜੇ_ਵਿੱਥ));
		ਵਿੱਥ += ctx->ext_ਅੰਕੜੇ_ਵਿੱਥ;
		external_ਅੰਕੜੇ = true;
	}

	ਜੇ (external_ਅੰਕੜੇ) {
		u32 aligned_ਵਿੱਥ;
		ulong read_ਵਿੱਥ;
		ਖਾਲੀ *src_ptr;

		/* External data */
		ਜੇ (fit_image_get_ਅੰਕੜੇ_ਮਾਪ(fit, ਗੰਢ, &len))
			ਵਾਪਸ -ENOENT;

		/* Dont bother to copy 0 byte data, but warn, though */
		ਜੇ (!len) {
			log_warning("%s: Skip load '%s': image size is 0!\n",
				    __func__, fit_get_name(fit, ਗੰਢ, ਨਿਰਾ));
			fit_fill_image_info(fit, ਗੰਢ, image_info, ਲੱਦੋ_addr, 0);
			ਵਾਪਸ 0;
		}

		/*
		 * data-size is excluded from the configuration signature (it
		 * is in exc_prop[] in image-fit-sig.c), so it stays attacker
		 * controlled even after fit_config_verify() succeeds. The
		 * image hash is only verified after the device read below, so
		 * an oversized value has to be rejected here.
		 */
		ret = get_aligned_image_ਮਾਪ(info, len, ਵਿੱਥ, &ਮਾਪ);
		ਜੇ (ret) {
			log_debug("Invalid external data size: %u\n", len);
			ਵਾਪਸ ret;
		}

		ਜੇ (ਮਾਪ > max_ਮਾਪ) {
			log_debug("Image too large: aligned size %lu, max %lu (data-size %u)\n",
				  ਮਾਪ, max_ਮਾਪ, len);
			ਵਾਪਸ -EFBIG;
		}

		aligned_ਵਿੱਥ = get_aligned_image_ਵਿੱਥ(info, ਵਿੱਥ);
		ਜੇ (aligned_ਵਿੱਥ > ULONG_MAX - fit_ਵਿੱਥ) {
			log_debug("Invalid aligned external data offset: %u\n",
				  aligned_ਵਿੱਥ);
			ਵਾਪਸ -EINVAL;
		}
		read_ਵਿੱਥ = fit_ਵਿੱਥ + aligned_ਵਿੱਥ;

		ਜੇ (spl_decompression_enabled() &&
		    (image_comp == IH_COMP_GZIP || image_comp == IH_COMP_LZMA))
			src_ptr = map_sysmem(ALIGN(CONFIG_SYS_ਲੱਦੋ_ADDR, ARCH_DMA_MINALIGN), len);
		ਨਹੀਂਤਾਂ
			src_ptr = map_sysmem(ALIGN(ਲੱਦੋ_addr, ARCH_DMA_MINALIGN), len);
		ਲੰਮਾਈ = len;

		/*
		 * For non-signed FIT images, we can check that
		 * (read_offset + size) does not wrap and that
		 * (src_ptr + size) does not exceed the addressable range.
		 * For signed FITs, we can additionally check that
		 * (offset + len) doesn't exceed the allowed FIT image
		 * maximum size.
		 */
		ਜੇ (ਮਾਪ > ULONG_MAX - read_ਵਿੱਥ ||
		    ਮਾਪ > UINTPTR_MAX - (uintptr_t)src_ptr
		/*
		 * #if (not a runtime if) is required: FIT_SIGNATURE_MAX_SIZE
		 * depends on FIT_SIGNATURE, so CONFIG_VAL(FIT_SIGNATURE_MAX_SIZE)
		 * is undefined when signing is disabled and referencing it
		 * here would fail to compile.
		 */
#ਜੇ CONFIG_IS_ENABLED(FIT_SIGNATURE)
		    || ਵਿੱਥ > CONFIG_VAL(FIT_SIGNATURE_MAX_ਮਾਪ) ||
		    len > CONFIG_VAL(FIT_SIGNATURE_MAX_ਮਾਪ) - ਵਿੱਥ
#ਜੇ_ਅੰਤ
		) {
			log_debug("FIT external data is out of bounds (offset=%u, size=%u)\n",
				  ਵਿੱਥ, len);
			ਵਾਪਸ -EINVAL;
		}

		log_debug("reading from offset %x / %lx size %lx to %p: ",
			  ਵਿੱਥ, read_ਵਿੱਥ, ਮਾਪ, src_ptr);

		ਜੇ (info->ਪਠ(info, read_ਵਿੱਥ, ਮਾਪ, src_ptr) < ਲੰਮਾਈ)
			ਵਾਪਸ -EIO;

		debug("External data: dst=%p, offset=%x, size=%lx\n",
		      src_ptr, ਵਿੱਥ, (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)ਲੰਮਾਈ);
		src = src_ptr + get_aligned_image_overhead(info, ਵਿੱਥ);
	} ਨਹੀਂਤਾਂ {
		/* Embedded data */
		ਜੇ (fit_image_get_emb_ਅੰਕੜੇ(fit, ਗੰਢ, &ਅੰਕੜੇ, &ਲੰਮਾਈ)) {
			puts("Cannot get image data/size\n");
			ਵਾਪਸ -ENOENT;
		}
		debug("Embedded data: dst=%lx, size=%lx\n", ਲੱਦੋ_addr,
		      (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)ਲੰਮਾਈ);
		src = (ਖਾਲੀ *)ਅੰਕੜੇ;	/* cast away const */
	}

	ਜੇ (CONFIG_IS_ENABLED(FIT_SIGNATURE)) {
		ਲਿਖੋ("## Checking hash(es) for Image %s ... ",
		       fit_get_name(fit, ਗੰਢ, ਨਿਰਾ));
		ਜੇ (!fit_image_verify_with_ਅੰਕੜੇ(fit, ਗੰਢ, gd_fdt_blob(), src,
						ਲੰਮਾਈ))
			ਵਾਪਸ -EPERM;
		puts("OK\n");
	}

	ਜੇ (CONFIG_IS_ENABLED(FIT_IMAGE_POST_PROCESS))
		board_fit_image_post_process(fit, ਗੰਢ, &src, &ਲੰਮਾਈ);

	ਲੱਦੋ_ptr = map_sysmem(ਲੱਦੋ_addr, ਲੰਮਾਈ);
	ਜੇ (IS_ENABLED(CONFIG_SPL_GZIP) && image_comp == IH_COMP_GZIP) {
		ਮਾਪ = ਲੰਮਾਈ;
		ਜੇ (gunzip(ਲੱਦੋ_ptr, CONFIG_SYS_BOOTM_LEN, src, &ਮਾਪ)) {
			puts("Uncompressing error\n");
			ਵਾਪਸ -EIO;
		}
		ਲੰਮਾਈ = ਮਾਪ;
	} ਨਹੀਂਤਾਂ ਜੇ (IS_ENABLED(CONFIG_SPL_LZMA) && image_comp == IH_COMP_LZMA) {
		ਮਾਪ = CONFIG_SYS_BOOTM_LEN;
		ulong ਲੱਦੋ_ਸਿਰਾ;

		ਜੇ (image_decomp(IH_COMP_LZMA, CONFIG_SYS_ਲੱਦੋ_ADDR, 0, 0,
				 ਲੱਦੋ_ptr, src, ਲੰਮਾਈ, ਮਾਪ, &ਲੱਦੋ_ਸਿਰਾ)) {
			puts("Uncompressing error\n");
			ਵਾਪਸ -EIO;
		}
		ਲੰਮਾਈ = ਲੱਦੋ_ਸਿਰਾ - CONFIG_SYS_ਲੱਦੋ_ADDR;
	} ਨਹੀਂਤਾਂ {
		memmove(ਲੱਦੋ_ptr, src, ਲੰਮਾਈ);
	}

	fit_fill_image_info(fit, ਗੰਢ, image_info, ਲੱਦੋ_addr, ਲੰਮਾਈ);
	log_debug("- done loading\n");

	upl_ਜੋੜੋ_image(fit, ਗੰਢ, ਲੱਦੋ_addr, ਲੰਮਾਈ);

	ਵਾਪਸ 0;
}

ਸਥਿਰ bool os_takes_devicetree(uint8_t os)
{
	ਚੋਣ (os) {
	ਹਾਲਤ IH_OS_U_BOOT:
		ਵਾਪਸ true;
	ਹਾਲਤ IH_OS_LINUX:
		ਵਾਪਸ IS_ENABLED(CONFIG_SPL_OS_BOOT) ||
		       IS_ENABLED(CONFIG_SPL_OPENSBI);
	ਮੂਲ:
		ਵਾਪਸ false;
	}
}

__weak ਪੂਰਨ board_spl_fit_append_fdt_skip(ਅਚਲ ਅੱਖਰ *name)
{
	ਵਾਪਸ 0;	/* Do not skip */
}

ਸਥਿਰ ਪੂਰਨ spl_fit_append_fdt(ਬਣਤਰ spl_image_info *spl_image,
			      ਬਣਤਰ spl_ਲੱਦੋ_info *info, ulong ਵਿੱਥ,
			      ਅਚਲ ਬਣਤਰ spl_fit_info *ctx)
{
	ਬਣਤਰ spl_image_info image_info;
	ਪੂਰਨ ਗੰਢ, ret = 0, ਸੂਚਕਾਂਕ = 0;

	/*
	 * Use the address following the image as target address for the
	 * device tree.
	 */
	image_info.ਲੱਦੋ_addr = ALIGN(spl_image->ਲੱਦੋ_addr + spl_image->ਮਾਪ, 8);

	/* Figure out which device tree the board wants to use */
	ਗੰਢ = spl_fit_get_image_ਗੰਢ(ctx, FIT_FDT_PROP, ਸੂਚਕਾਂਕ++);
	ਜੇ (ਗੰਢ < 0) {
		size_t ਮਾਪ;

		debug("%s: cannot find FDT node\n", __func__);

		/*
		 * U-Boot did not find a device tree inside the FIT image. Use
		 * the U-Boot device tree instead.
		 */
		ਜੇ (!gd->fdt_blob)
			ਵਾਪਸ ਗੰਢ;

		/*
		 * Make the load-address of the FDT available for the SPL
		 * framework
		 */
		ਮਾਪ = fdt_totalsize(gd->fdt_blob);
		spl_image->fdt_addr = map_sysmem(image_info.ਲੱਦੋ_addr, ਮਾਪ);
		ਸਮ੍ਰਿਤੀ_ਨਕਲ(spl_image->fdt_addr, gd->fdt_blob, ਮਾਪ);
	} ਨਹੀਂਤਾਂ {
		ret = ਲੱਦੋ_simple_fit(info, ਵਿੱਥ, ctx, ਗੰਢ, &image_info,
				      CONFIG_SYS_BOOTM_LEN);
		ਜੇ (ret < 0)
			ਵਾਪਸ ret;

		spl_image->fdt_addr = phys_ਮੰਜ਼ਿਲ_virt(image_info.ਲੱਦੋ_addr);
	}

	ਜੇ (CONFIG_IS_ENABLED(FIT_IMAGE_TINY))
		ਵਾਪਸ 0;

#ਜੇ CONFIG_IS_ENABLED(ਲੱਦੋ_FIT_APPLY_OVERLAY)
		ਖਾਲੀ *tmpbuffer = ਨਿਰਾ;

		ਲਈ (; ; ਸੂਚਕਾਂਕ++) {
			ਅਚਲ ਅੱਖਰ *str;

			ret = spl_fit_get_image_name(ctx, FIT_FDT_PROP, ਸੂਚਕਾਂਕ, &str);
			ਜੇ (ret == -E2BIG) {
				debug("%s: No additional FDT node\n", __func__);
				ret = 0;
				ਰੋਕੋ;
			} ਨਹੀਂਤਾਂ ਜੇ (ret) {
				ਰੋਕੋ;
			}

			ret = board_spl_fit_append_fdt_skip(str);
			ਜੇ (ret)
				ਜਾਰੀ;

			ਗੰਢ = fdt_subnode_ਵਿੱਥ(ctx->fit, ctx->images_ਗੰਢ, str);
			ਜੇ (ਗੰਢ < 0) {
				debug("%s: unable to find FDT node %d\n",
				      __func__, ਸੂਚਕਾਂਕ);
				ਜਾਰੀ;
			}

			ਜੇ (!tmpbuffer) {
				/*
				 * allocate memory to store the DT overlay
				 * before it is applied. It may not be used
				 * depending on how the overlay is stored, so
				 * don't fail yet if the allocation failed.
				 */
				size_t ਮਾਪ = CONFIG_SPL_ਲੱਦੋ_FIT_APPLY_OVERLAY_BUF_SZ;

				tmpbuffer = malloc_cache_aligned(ਮਾਪ);
				ਜੇ (!tmpbuffer)
					debug("%s: unable to allocate space for overlays\n",
					      __func__);
			}
			image_info.ਲੱਦੋ_addr = (ulong)tmpbuffer;
			ret = ਲੱਦੋ_simple_fit(info, ਵਿੱਥ, ctx, ਗੰਢ,
					      &image_info,
					      CONFIG_SPL_ਲੱਦੋ_FIT_APPLY_OVERLAY_BUF_SZ);
			ਜੇ (ret == -EBADSLT)
				ਜਾਰੀ;
			ਨਹੀਂਤਾਂ ਜੇ (ret < 0)
				ਰੋਕੋ;

			/* Make room in FDT for changes from the overlay */
			ret = fdt_increase_ਮਾਪ(spl_image->fdt_addr,
						image_info.ਮਾਪ);
			ਜੇ (ret < 0)
				ਰੋਕੋ;

			ret = fdt_overlay_apply_verbose(spl_image->fdt_addr,
							(ਖਾਲੀ *)image_info.ਲੱਦੋ_addr);
			ਜੇ (ret) {
				pr_err("failed to apply DT overlay %s\n",
				       fit_get_name(ctx->fit, ਗੰਢ, ਨਿਰਾ));
				ਰੋਕੋ;
			}

			debug("%s: DT overlay %s applied\n", __func__,
			      fit_get_name(ctx->fit, ਗੰਢ, ਨਿਰਾ));
		}
		ਮੁਕਤ(tmpbuffer);
		ਜੇ (ret)
			ਵਾਪਸ ret;
#ਜੇ_ਅੰਤ
	/* Try to make space, so we can inject details on the loadables */
	ret = fdt_shrink_ਮੰਜ਼ਿਲ_minimum(spl_image->fdt_addr, 8192);
	ਜੇ (ret < 0)
		ਵਾਪਸ ret;

	ਵਾਪਸ ret;
}

ਸਥਿਰ ਪੂਰਨ spl_fit_ਰਿਕਾਰਡ_loadable(ਅਚਲ ਬਣਤਰ spl_fit_info *ctx, ਪੂਰਨ ਸੂਚਕਾਂਕ,
				   ਖਾਲੀ *blob, ਬਣਤਰ spl_image_info *image)
{
	ਪੂਰਨ ret = 0;
	ਅਚਲ ਅੱਖਰ *name;
	ਪੂਰਨ ਗੰਢ;

	ret = spl_fit_get_image_name(ctx, FIT_LOADABLE_PROP, ਸੂਚਕਾਂਕ, &name);
	ਜੇ (ret < 0)
		ਵਾਪਸ ret;

	ਗੰਢ = spl_fit_get_image_ਗੰਢ(ctx, FIT_LOADABLE_PROP, ਸੂਚਕਾਂਕ);

	ret = fdt_ਰਿਕਾਰਡ_loadable(blob, ਸੂਚਕਾਂਕ, name, image->ਲੱਦੋ_addr,
			image->ਮਾਪ, image->ਇੰਦਰਾਜ_ਬਿੰਦੂ,
			fdt_getprop(ctx->fit, ਗੰਢ, FIT_TYPE_PROP, ਨਿਰਾ),
			fdt_getprop(ctx->fit, ਗੰਢ, FIT_OS_PROP, ਨਿਰਾ),
			fdt_getprop(ctx->fit, ਗੰਢ, FIT_ARCH_PROP, ਨਿਰਾ));

	ਵਾਪਸ ret;
}

ਸਥਿਰ ਪੂਰਨ spl_fit_image_is_fpga(ਅਚਲ ਖਾਲੀ *fit, ਪੂਰਨ ਗੰਢ)
{
	ਅਚਲ ਅੱਖਰ *type;

	ਜੇ (!IS_ENABLED(CONFIG_SPL_FPGA))
		ਵਾਪਸ 0;

	type = fdt_getprop(fit, ਗੰਢ, FIT_TYPE_PROP, ਨਿਰਾ);
	ਜੇ (!type)
		ਵਾਪਸ 0;

	ਵਾਪਸ !strcmp(type, "fpga");
}

ਸਥਿਰ ਖਾਲੀ spl_fit_image_ਰਿਕਾਰਡ_arm32_optee(ਅਚਲ ਖਾਲੀ *fit, ਪੂਰਨ ਗੰਢ,
					     ਬਣਤਰ spl_image_info *spl_image,
					     ਬਣਤਰ spl_image_info *image_info)
{
#ਜੇ defined(CONFIG_BOOTM_OPTEE) && defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
	ਅਚਲ ਅੱਖਰ *type = fdt_getprop(fit, ਗੰਢ, FIT_TYPE_PROP, ਨਿਰਾ);

	ਜੇ (!type)
		ਵਾਪਸ;

	ਜੇ (strcmp(type, "tee"))
		ਵਾਪਸ;

	spl_image->optee_addr = image_info->ਲੱਦੋ_addr;
#ਜੇ_ਅੰਤ
}

ਸਥਿਰ ਪੂਰਨ spl_fit_image_get_os(ਅਚਲ ਖਾਲੀ *fit, ਪੂਰਨ noffset, uint8_t *os)
{
	ਜੇ (!CONFIG_IS_ENABLED(FIT_IMAGE_TINY) || CONFIG_IS_ENABLED(OS_BOOT))
		ਵਾਪਸ fit_image_get_os(fit, noffset, os);

	ਅਚਲ ਅੱਖਰ *name = fdt_getprop(fit, noffset, FIT_OS_PROP, ਨਿਰਾ);
	ਜੇ (!name)
		ਵਾਪਸ -ENOENT;

	/*
	 * We don't care what the type of the image actually is,
	 * only whether or not it is U-Boot. This saves some
	 * space by omitting the large table of OS types.
	 */
	ਜੇ (!strcmp(name, "u-boot"))
		*os = IH_OS_U_BOOT;
	ਨਹੀਂਤਾਂ
		*os = IH_OS_INVALID;

	ਵਾਪਸ 0;
}

/*
 * The purpose of the FIT load buffer is to provide a memory location that is
 * independent of the load address of any FIT component.
 */
__weak ਖਾਲੀ *board_spl_fit_ਬਫਰ_addr(ulong fit_ਮਾਪ, ਪੂਰਨ sectors, ਪੂਰਨ bl_len)
{
	ਖਾਲੀ *buf;
	size_t ਮਾਪ = sectors * bl_len;

	buf = malloc_cache_aligned(ਮਾਪ);
	ਜੇ (!buf) {
		pr_err("Could not get FIT buffer of %lu bytes\n", (ulong)ਮਾਪ);

		ਜੇ (IS_ENABLED(CONFIG_SPL_SYS_MALLOC))
			pr_err("\tcheck CONFIG_SPL_SYS_MALLOC_SIZE\n");
		ਨਹੀਂਤਾਂ
			pr_err("\tcheck CONFIG_SPL_SYS_MALLOC_F_LEN\n");

		buf = spl_get_ਲੱਦੋ_ਬਫਰ(0, ਮਾਪ);
	}
	ਵਾਪਸ buf;
}

/*
 * Weak default function to allow customizing SPL fit loading for load-only
 * use cases by allowing to skip the parsing/processing of the FIT contents
 * (so that this can be done separately in a more customized fashion)
 */
__weak bool spl_ਲੱਦੋ_simple_fit_skip_processing(ਖਾਲੀ)
{
	ਵਾਪਸ false;
}

/*
 * Weak default function to allow fixes after fit header
 * is loaded.
 */
__weak ਖਾਲੀ *spl_ਲੱਦੋ_simple_fit_fix_ਲੱਦੋ(ਅਚਲ ਖਾਲੀ *fit)
{
	ਵਾਪਸ (ਖਾਲੀ *)fit;
}

ਸਥਿਰ ਖਾਲੀ warn_deprecated(ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ)
{
	ਲਿਖੋ("DEPRECATED: %s\n", ਸੁਨੇਹਾ);
	ਲਿਖੋ("\tSee https://fitspec.osfw.foundation/\n");
}

ਸਥਿਰ ਪੂਰਨ spl_fit_upload_fpga(ਬਣਤਰ spl_fit_info *ctx, ਪੂਰਨ ਗੰਢ,
			       ਬਣਤਰ spl_image_info *fpga_image)
{
	ਅਚਲ ਅੱਖਰ *compatible;
	ਪੂਰਨ ret;
	ਪੂਰਨ devnum = 0;
	ਪੂਰਨ flags = 0;

	debug("FPGA bitstream at: %x, size: %x\n",
	      (u32)fpga_image->ਲੱਦੋ_addr, fpga_image->ਮਾਪ);

	compatible = fdt_getprop(ctx->fit, ਗੰਢ, FIT_COMPAT_PROP, ਨਿਰਾ);
	ਜੇ (!compatible) {
		warn_deprecated("'fpga' image without 'compatible' property");
	} ਨਹੀਂਤਾਂ {
		ਜੇ (CONFIG_IS_ENABLED(FPGA_ਲੱਦੋ_SECURE))
			flags = fpga_compatible2flag(devnum, compatible);
		ਜੇ (strcmp(compatible, "u-boot,fpga-legacy"))
			debug("Ignoring compatible = %s property\n",
			      compatible);
	}

	ret = fpga_ਲੱਦੋ(devnum, (ਖਾਲੀ *)fpga_image->ਲੱਦੋ_addr,
			fpga_image->ਮਾਪ, ਦੋਆਧਾਰੀ_ਅੰਕ_ਭਰਿਆ, flags);
	ਜੇ (ret) {
		ਲਿਖੋ("%s: Cannot load the image to the FPGA\n", __func__);
		ਵਾਪਸ ret;
	}

	puts("FPGA image loaded from FIT\n");

	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ spl_fit_ਲੱਦੋ_fpga(ਬਣਤਰ spl_fit_info *ctx,
			     ਬਣਤਰ spl_ਲੱਦੋ_info *info, ulong ਵਿੱਥ)
{
	ਪੂਰਨ ਗੰਢ, ret;

	ਬਣਤਰ spl_image_info fpga_image = {
		.ਲੱਦੋ_addr = 0,
	};

	ਗੰਢ = spl_fit_get_image_ਗੰਢ(ctx, FIT_FPGA_PROP, 0);
	ਜੇ (ਗੰਢ < 0)
		ਵਾਪਸ ਗੰਢ;

	warn_deprecated("'fpga' property in config node. Use 'loadables'");

	/* Load the image and set up the fpga_image structure */
	ret = ਲੱਦੋ_simple_fit(info, ਵਿੱਥ, ctx, ਗੰਢ, &fpga_image,
			      CONFIG_SYS_BOOTM_LEN);
	ਜੇ (ret) {
		ਲਿਖੋ("%s: Cannot load the FPGA: %i\n", __func__, ret);
		ਵਾਪਸ ret;
	}

	ਵਾਪਸ spl_fit_upload_fpga(ctx, ਗੰਢ, &fpga_image);
}

ਸਥਿਰ ਪੂਰਨ spl_simple_fit_read(ਬਣਤਰ spl_fit_info *ctx,
			       ਬਣਤਰ spl_ਲੱਦੋ_info *info, ulong ਵਿੱਥ,
			       ਅਚਲ ਖਾਲੀ *fit_ਸਿਰਲੇਖ)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ aligned_ਮਾਪ, ਗਿਣਤੀ_2, ਮਾਪ;
	ਖਾਲੀ *buf;
	ਪੂਰਨ ret;

	/*
	 * For FIT with external data, figure out where the external images
	 * start. This is the base for the data-offset properties in each
	 * image.
	 */
	ਮਾਪ = ALIGN(fdt_totalsize(fit_ਸਿਰਲੇਖ), 4);
	ਮਾਪ = board_spl_fit_ਮਾਪ_align(ਮਾਪ);
	ctx->ext_ਅੰਕੜੇ_ਵਿੱਥ = ALIGN(ਮਾਪ, 4);

	/*
	 * So far we only have one block of data from the FIT. Read the entire
	 * thing, including that first block.
	 *
	 * For FIT with data embedded, data is loaded as part of FIT image.
	 * For FIT with external data, data is not loaded in this step.
	 */
	ret = get_aligned_image_ਮਾਪ(info, ਮਾਪ, 0, &aligned_ਮਾਪ);
	ਜੇ (ret) {
		log_debug("Invalid FIT size: %lu\n", ਮਾਪ);
		ਵਾਪਸ ret;
	}
	buf = board_spl_fit_ਬਫਰ_addr(aligned_ਮਾਪ, aligned_ਮਾਪ, 1);
	ਜੇ (!buf) {
		/*
		 * We assume that none of the board will ever use 0x0 as a
		 * valid load address. Theoretically some board could use it,
		 * but this is extremely unlikely.
		 */
		ਵਾਪਸ -EIO;
	}

	ਗਿਣਤੀ_2 = info->ਪਠ(info, ਵਿੱਥ, aligned_ਮਾਪ, buf);
	ਜੇ (!ਗਿਣਤੀ_2) {
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
		ਵਾਪਸ -EIO;
	}

	ctx->fit = buf;
	debug("fit read offset %lx, size=%lu, dst=%p, count=%lu\n",
	      ਵਿੱਥ, aligned_ਮਾਪ, buf, ਗਿਣਤੀ_2);

	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ spl_simple_fit_parse(ਬਣਤਰ spl_fit_info *ctx)
{
	/* Find the correct subnode under "/configurations" */
	ctx->conf_ਗੰਢ = fit_ਲੱਭੋ_config_ਗੰਢ(ctx->fit);
	ਜੇ (ctx->conf_ਗੰਢ < 0)
		ਵਾਪਸ -EINVAL;

	ਜੇ (CONFIG_IS_ENABLED(FIT_SIGNATURE)) {
		ਲਿਖੋ("## Checking hash(es) for config %s ... ",
		       fit_get_name(ctx->fit, ctx->conf_ਗੰਢ, ਨਿਰਾ));
		ਜੇ (fit_config_verify(ctx->fit, ctx->conf_ਗੰਢ))
			ਵਾਪਸ -EPERM;
		puts("OK\n");
	}

	/* find the node holding the images information */
	ctx->images_ਗੰਢ = fdt_ਰਸਤਾ_ਵਿੱਥ(ctx->fit, FIT_IMAGES_ਰਸਤਾ);
	ਜੇ (ctx->images_ਗੰਢ < 0) {
		debug("%s: Cannot find /images node: %d\n", __func__,
		      ctx->images_ਗੰਢ);
		ਵਾਪਸ -EINVAL;
	}

	ਵਾਪਸ 0;
}

ਪੂਰਨ spl_ਲੱਦੋ_simple_fit(ਬਣਤਰ spl_image_info *spl_image,
			ਬਣਤਰ spl_ਲੱਦੋ_info *info, ulong ਵਿੱਥ, ਖਾਲੀ *fit)
{
	ਬਣਤਰ spl_image_info image_info;
	ਬਣਤਰ spl_fit_info ctx;
	ਪੂਰਨ ਗੰਢ = -1;
	ਪੂਰਨ ret;
	ਪੂਰਨ ਸੂਚਕਾਂਕ = 0;
	ਪੂਰਨ firmware_ਗੰਢ;

	ret = spl_simple_fit_read(&ctx, info, ਵਿੱਥ, fit);
	ਜੇ (ret < 0)
		ਵਾਪਸ ret;

	/* skip further processing if requested to enable load-only use cases */
	ਜੇ (spl_ਲੱਦੋ_simple_fit_skip_processing())
		ਵਾਪਸ 0;

	ctx.fit = spl_ਲੱਦੋ_simple_fit_fix_ਲੱਦੋ(ctx.fit);

	ret = spl_simple_fit_parse(&ctx);
	ਜੇ (ret < 0)
		ਵਾਪਸ ret;

	ਜੇ (IS_ENABLED(CONFIG_SPL_FPGA))
		spl_fit_ਲੱਦੋ_fpga(&ctx, info, ਵਿੱਥ);

	/*
	 * Find the U-Boot image using the following search order:
	 *   - start at 'firmware' (e.g. an ARM Trusted Firmware)
	 *   - fall back 'kernel' (e.g. a Falcon-mode OS boot
	 *   - fall back to using the first 'loadables' entry
	 */
	ਜੇ (ਗੰਢ < 0)
		ਗੰਢ = spl_fit_get_image_ਗੰਢ(&ctx, FIT_FIRMWARE_PROP, 0);

	ਜੇ (ਗੰਢ < 0 && IS_ENABLED(CONFIG_SPL_OS_BOOT))
		ਗੰਢ = spl_fit_get_image_ਗੰਢ(&ctx, FIT_KERNEL_PROP, 0);

	ਜੇ (ਗੰਢ < 0) {
		debug("could not find firmware image, trying loadables...\n");
		ਗੰਢ = spl_fit_get_image_ਗੰਢ(&ctx, FIT_LOADABLE_PROP, 0);
		/*
		 * If we pick the U-Boot image from "loadables", start at
		 * the second image when later loading additional images.
		 */
		ਸੂਚਕਾਂਕ = 1;
	}
	ਜੇ (ਗੰਢ < 0) {
		debug("%s: Cannot find u-boot image node: %d\n",
		      __func__, ਗੰਢ);
		ਵਾਪਸ -1;
	}

	/* Load the image and set up the spl_image structure */
	ret = ਲੱਦੋ_simple_fit(info, ਵਿੱਥ, &ctx, ਗੰਢ, spl_image,
			      CONFIG_SYS_BOOTM_LEN);
	ਜੇ (ret)
		ਵਾਪਸ ret;

	/*
	 * For backward compatibility, we treat the first node that is
	 * as a U-Boot image, if no OS-type has been declared.
	 */
	ਜੇ (!spl_fit_image_get_os(ctx.fit, ਗੰਢ, &spl_image->os))
		debug("Image OS is %s\n", genimg_get_os_name(spl_image->os));
	ਨਹੀਂਤਾਂ ਜੇ (!IS_ENABLED(CONFIG_SPL_OS_BOOT))
		spl_image->os = IH_OS_U_BOOT;

	/*
	 * Booting a next-stage U-Boot may require us to append the FDT.
	 * We allow this to fail, as the U-Boot image might embed its FDT.
	 */
	ਜੇ (os_takes_devicetree(spl_image->os)) {
		ret = spl_fit_append_fdt(spl_image, info, ਵਿੱਥ, &ctx);
		ਜੇ (ret < 0 && spl_image->os != IH_OS_U_BOOT)
			ਵਾਪਸ ret;
	}

	firmware_ਗੰਢ = ਗੰਢ;
	/* Now check if there are more images for us to load */
	ਲਈ (; ; ਸੂਚਕਾਂਕ++) {
		uint8_t os_type = IH_OS_INVALID;

		ਗੰਢ = spl_fit_get_image_ਗੰਢ(&ctx, FIT_LOADABLE_PROP, ਸੂਚਕਾਂਕ);
		ਜੇ (ਗੰਢ < 0)
			ਰੋਕੋ;

		/*
		 * if the firmware is also a loadable, skip it because
		 * it already has been loaded. This is typically the case with
		 * u-boot.img generated by mkimage.
		 */
		ਜੇ (firmware_ਗੰਢ == ਗੰਢ)
			ਜਾਰੀ;

		image_info.ਲੱਦੋ_addr = 0;
		ret = ਲੱਦੋ_simple_fit(info, ਵਿੱਥ, &ctx, ਗੰਢ, &image_info,
				      CONFIG_SYS_BOOTM_LEN);
		ਜੇ (ret < 0 && ret != -EBADSLT) {
			ਲਿਖੋ("%s: can't load image loadables index %d (ret = %d)\n",
			       __func__, ਸੂਚਕਾਂਕ, ret);
			ਵਾਪਸ ret;
		}

		ਜੇ (spl_fit_image_is_fpga(ctx.fit, ਗੰਢ))
			spl_fit_upload_fpga(&ctx, ਗੰਢ, &image_info);

		ਜੇ (!spl_fit_image_get_os(ctx.fit, ਗੰਢ, &os_type))
			debug("Loadable is %s\n", genimg_get_os_name(os_type));

		ਜੇ (os_takes_devicetree(os_type)) {
			spl_fit_append_fdt(&image_info, info, ਵਿੱਥ, &ctx);
			spl_image->fdt_addr = image_info.fdt_addr;
		}

		/*
		 * If the "firmware" image did not provide an entry point,
		 * use the first valid entry point from the loadables.
		 */
		ਜੇ (spl_image->ਇੰਦਰਾਜ_ਬਿੰਦੂ == FDT_ERROR &&
		    image_info.ਇੰਦਰਾਜ_ਬਿੰਦੂ != FDT_ERROR)
			spl_image->ਇੰਦਰਾਜ_ਬਿੰਦੂ = image_info.ਇੰਦਰਾਜ_ਬਿੰਦੂ;

		spl_fit_image_ਰਿਕਾਰਡ_arm32_optee(ctx.fit, ਗੰਢ, spl_image,
						 &image_info);

		/* Record our loadables into the FDT */
		ਜੇ (!CONFIG_IS_ENABLED(FIT_IMAGE_TINY) &&
		    xpl_get_fdt_update(info) && spl_image->fdt_addr)
			spl_fit_ਰਿਕਾਰਡ_loadable(&ctx, ਸੂਚਕਾਂਕ,
						spl_image->fdt_addr,
						&image_info);
	}

	/*
	 * If a platform does not provide CONFIG_SYS_UBOOT_START, U-Boot's
	 * Makefile will set it to 0 and it will end up as the entry point
	 * here. What it actually means is: use the load address.
	 */
	ਜੇ (spl_image->ਇੰਦਰਾਜ_ਬਿੰਦੂ == FDT_ERROR || spl_image->ਇੰਦਰਾਜ_ਬਿੰਦੂ == 0)
		spl_image->ਇੰਦਰਾਜ_ਬਿੰਦੂ = spl_image->ਲੱਦੋ_addr;

	spl_image->flags |= SPL_FIT_ਮਿਲਿਆ;
	upl_ਸੈੱਟ_fit_info(map_ਮੰਜ਼ਿਲ_sysmem(ctx.fit), ctx.conf_ਗੰਢ,
			 spl_image->ਇੰਦਰਾਜ_ਬਿੰਦੂ);

	ਵਾਪਸ 0;
}

/* Parse and load full fitImage in SPL */
ਪੂਰਨ spl_ਲੱਦੋ_fit_image(ਬਣਤਰ spl_image_info *spl_image,
		       ਅਚਲ ਬਣਤਰ legacy_img_hdr *ਸਿਰਲੇਖ)
{
	ਬਣਤਰ bootm_ਸਿਰਲੇਖਾਂ images;
	ਅਚਲ ਅੱਖਰ *fit_uname_config = ਨਿਰਾ;
	ulong fdt_hack;
	ਅਚਲ ਅੱਖਰ *uname;
	ulong fw_ਅੰਕੜੇ = 0, dt_ਅੰਕੜੇ = 0, img_ਅੰਕੜੇ = 0;
	ulong fw_len = 0, dt_len = 0, img_len = 0;
	ਪੂਰਨ idx, conf_noffset;
	ਪੂਰਨ ret;

	images.verify = CONFIG_IS_ENABLED(FIT_SIGNATURE);

	ret = fit_image_ਲੱਦੋ(&images, virt_ਮੰਜ਼ਿਲ_phys((ਖਾਲੀ *)ਸਿਰਲੇਖ),
			     ਨਿਰਾ, &fit_uname_config, IH_ARCH_DEFAULT,
			     IH_TYPE_FIRMWARE, -1, FIT_ਲੱਦੋ_OPTIONAL,
			     &fw_ਅੰਕੜੇ, &fw_len);

	ਜੇ (ret < 0) {
		ret = fit_image_ਲੱਦੋ(&images, virt_ਮੰਜ਼ਿਲ_phys((ਖਾਲੀ *)ਸਿਰਲੇਖ),
				     ਨਿਰਾ, &fit_uname_config, IH_ARCH_DEFAULT,
				     IH_TYPE_KERNEL, -1, FIT_ਲੱਦੋ_OPTIONAL,
				     &fw_ਅੰਕੜੇ, &fw_len);
	}

	ਜੇ (ret < 0)
		ਵਾਪਸ ret;

	spl_image->ਮਾਪ = fw_len;
	spl_image->ਲੱਦੋ_addr = fw_ਅੰਕੜੇ;
	ਜੇ (fit_image_get_ਇੰਦਰਾਜ(ਸਿਰਲੇਖ, ret, &spl_image->ਇੰਦਰਾਜ_ਬਿੰਦੂ))
		spl_image->ਇੰਦਰਾਜ_ਬਿੰਦੂ = fw_ਅੰਕੜੇ;
	ਜੇ (fit_image_get_os(ਸਿਰਲੇਖ, ret, &spl_image->os))
		spl_image->os = IH_OS_INVALID;
	spl_image->name = genimg_get_os_name(spl_image->os);

	debug(PHASE_PROMPT "payload image: %32s load addr: 0x%lx size: %d\n",
	      spl_image->name, spl_image->ਲੱਦੋ_addr, spl_image->ਮਾਪ);

	images.verify = CONFIG_IS_ENABLED(FIT_SIGNATURE);

	ret = fit_image_ਲੱਦੋ(&images, virt_ਮੰਜ਼ਿਲ_phys((ਖਾਲੀ *)ਸਿਰਲੇਖ), ਨਿਰਾ,
			     &fit_uname_config, IH_ARCH_DEFAULT, IH_TYPE_FLATDT,
			     -1, FIT_ਲੱਦੋ_OPTIONAL, &dt_ਅੰਕੜੇ, &dt_len);
	ਜੇ (ret >= 0) {
		ਜੇ (spl_image->os == IH_OS_U_BOOT) {
			/* HACK: U-Boot expects FDT at a specific address */
			fdt_hack = ALIGN(spl_image->ਲੱਦੋ_addr + spl_image->ਮਾਪ, 8);
			debug("Relocating FDT to %p\n", (ਖਾਲੀ *)fdt_hack);
			ਸਮ੍ਰਿਤੀ_ਨਕਲ(map_sysmem(fdt_hack, dt_len),
			       map_sysmem(dt_ਅੰਕੜੇ, 0), dt_len);
			spl_image->fdt_addr = (ਖਾਲੀ *)fdt_hack;
		} ਨਹੀਂਤਾਂ {
			spl_image->fdt_addr = (ਖਾਲੀ *)dt_ਅੰਕੜੇ;
		}
	}

	conf_noffset = fit_conf_get_ਗੰਢ((ਅਚਲ ਖਾਲੀ *)ਸਿਰਲੇਖ,
					 fit_uname_config);
	ਜੇ (conf_noffset < 0)
		ਵਾਪਸ 0;

	ਲਈ (idx = 0;
	     uname = fdt_stringlist_get((ਅਚਲ ਖਾਲੀ *)ਸਿਰਲੇਖ, conf_noffset,
					FIT_LOADABLE_PROP, idx,
				ਨਿਰਾ), uname;
	     idx++) {
		images.verify = CONFIG_IS_ENABLED(FIT_SIGNATURE);

		ret = fit_image_ਲੱਦੋ(&images, virt_ਮੰਜ਼ਿਲ_phys((ਖਾਲੀ *)ਸਿਰਲੇਖ),
				     &uname, &fit_uname_config,
				     IH_ARCH_DEFAULT, IH_TYPE_LOADABLE, -1,
				     FIT_ਲੱਦੋ_OPTIONAL_NON_ZERO,
				     &img_ਅੰਕੜੇ, &img_len);
		ਜੇ (ret < 0)
			ਵਾਪਸ ret;
	}
	spl_image->flags |= SPL_FIT_ਮਿਲਿਆ;

	upl_ਸੈੱਟ_fit_info(map_ਮੰਜ਼ਿਲ_sysmem(ਸਿਰਲੇਖ), conf_noffset,
			 spl_image->ਇੰਦਰਾਜ_ਬਿੰਦੂ);

	ਵਾਪਸ 0;
}
