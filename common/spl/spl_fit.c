// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <fpga.h>
#ಅನ್ತರ್ಭಾವ <gzip.h>
#ಅನ್ತರ್ಭಾವ <image.h>
#ಅನ್ತರ್ಭಾವ <log.h>
#ಅನ್ತರ್ಭಾವ <memalign.h>
#ಅನ್ತರ್ಭಾವ <mapmem.h>
#ಅನ್ತರ್ಭಾವ <spl.h>
#ಅನ್ತರ್ಭಾವ <upl.h>
#ಅನ್ತರ್ಭಾವ <sysinfo.h>
#ಅನ್ತರ್ಭಾವ <asm/global_data.h>
#ಅನ್ತರ್ಭಾವ <asm/io.h>
#ಅನ್ತರ್ಭಾವ <linux/libfdt.h>
#ಅನ್ತರ್ಭಾವ <linux/printk.h>

DECLARE_GLOBAL_ದತ್ತಾಂಶ_PTR;

ರಚನೆ spl_fit_info {
	ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *fit;	/* Pointer to a valid FIT blob */
	size_t ext_ದತ್ತಾಂಶ_ಸ್ಥಾನಾಂತರ;	/* Offset to FIT external data (end of FIT) */
	ಪೂರ್ಣಾಂಕ images_ಗಂಟು;	/* FDT offset to "/images" node */
	ಪೂರ್ಣಾಂಕ conf_ಗಂಟು;		/* FDT offset to selected configuration node */
};

__weak ulong board_spl_fit_ಗಾತ್ರ_align(ulong ಗಾತ್ರ_2)
{
	ಹಿಂತಿರುಗಿ ಗಾತ್ರ_2;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಕಂಡುಹಿಡಿ_ಗಂಟು_from_desc(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *fit, ಪೂರ್ಣಾಂಕ ಗಂಟು, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str)
{
	ಪೂರ್ಣಾಂಕ ಮಗು;

	ಆದರೆ (ಗಂಟು < 0)
		ಹಿಂತಿರುಗಿ -EINVAL;

	/* iterate the FIT nodes and find a matching description */
	ಗಾಗಿ (ಮಗು = fdt_ಮೊದಲ_subnode(fit, ಗಂಟು); ಮಗು >= 0;
	     ಮಗು = fdt_ಮುಂದಿನ_subnode(fit, ಮಗು)) {
		ಪೂರ್ಣಾಂಕ len;
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *desc = fdt_getprop(fit, ಮಗು, FIT_DESC_PROP, &len);

		ಆದರೆ (!desc)
			ಮುಂದುವರಿ;

		ಆದರೆ (!strcmp(desc, str))
			ಹಿಂತಿರುಗಿ ಮಗು;
	}

	ಹಿಂತಿರುಗಿ -ENOENT;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ spl_fit_get_image_name(ಸ್ಥಿರಾಂಕ ರಚನೆ spl_fit_info *ctx,
				  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *type, ಪೂರ್ಣಾಂಕ ಸೂಚ್ಯಂಕ,
				  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **outname)
{
	ರಚನೆ udevice *sysinfo;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name, *str, *ಕೊನೆ_2;
	__maybe_unused ಪೂರ್ಣಾಂಕ ಗಂಟು;
	ಪೂರ್ಣಾಂಕ len, i;
	bool ಸಿಕ್ಕಿತು = true;

	name = fdt_getprop(ctx->fit, ctx->conf_ಗಂಟು, type, &len);
	ಆದರೆ (!name) {
		debug("cannot find property '%s': %d\n", type, len);
		ಹಿಂತಿರುಗಿ -EINVAL;
	}
	/* A string property should be NUL terminated */
	ಕೊನೆ_2 = name + len - 1;
	ಆದರೆ (!len || *ಕೊನೆ_2) {
		debug("malformed property '%s'\n", type);
		ಹಿಂತಿರುಗಿ -EINVAL;
	}

	str = name;
	ಗಾಗಿ (i = 0; i < ಸೂಚ್ಯಂಕ; i++) {
		str = strchr(str, '\0') + 1;
		ಆದರೆ (str > ಕೊನೆ_2) {
			ಸಿಕ್ಕಿತು = false;
			ನಿಲ್ಲಿಸು;
		}
	}

	ಆದರೆ (!ಸಿಕ್ಕಿತು && CONFIG_IS_ENABLED(SYSINFO) && !sysinfo_get(&sysinfo)) {
		ಪೂರ್ಣಾಂಕ rc;
		/*
		 * no string in the property for this index. Check if the
		 * sysinfo-level code can supply one.
		 */
		rc = sysinfo_detect(sysinfo);
		ಆದರೆ (rc)
			ಹಿಂತಿರುಗಿ rc;

		rc = sysinfo_get_fit_loadable(sysinfo, ಸೂಚ್ಯಂಕ - i - 1, type,
					      &str);
		ಆದರೆ (rc && rc != -ENOENT)
			ಹಿಂತಿರುಗಿ rc;

		ಆದರೆ (!rc) {
			/*
			 * The sysinfo provided a name for a loadable.
			 * Try to match it against the description properties
			 * first. If no matching node is found, use it as a
			 * node name.
			 */
			ಪೂರ್ಣಾಂಕ ಗಂಟು;
			ಪೂರ್ಣಾಂಕ images = fdt_ಮಾರ್ಗ_ಸ್ಥಾನಾಂತರ(ctx->fit, FIT_IMAGES_ಮಾರ್ಗ);

			ಗಂಟು = ಕಂಡುಹಿಡಿ_ಗಂಟು_from_desc(ctx->fit, images, str);
			ಆದರೆ (ಗಂಟು > 0)
				str = fdt_get_name(ctx->fit, ಗಂಟು, ಖಾಲಿ);

			ಸಿಕ್ಕಿತು = true;
		}
	}

	ಆದರೆ (!ಸಿಕ್ಕಿತು) {
		debug("no string for index %d\n", ಸೂಚ್ಯಂಕ);
		ಹಿಂತಿರುಗಿ -E2BIG;
	}

	*outname = str;
	ಹಿಂತಿರುಗಿ 0;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ spl_fit_get_image_ಗಂಟು(ಸ್ಥಿರಾಂಕ ರಚನೆ spl_fit_info *ctx,
				  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *type, ಪೂರ್ಣಾಂಕ ಸೂಚ್ಯಂಕ)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str;
	ಪೂರ್ಣಾಂಕ err;
	ಪೂರ್ಣಾಂಕ ಗಂಟು;

	err = spl_fit_get_image_name(ctx, type, ಸೂಚ್ಯಂಕ, &str);
	ಆದರೆ (err)
		ಹಿಂತಿರುಗಿ err;

	debug("%s: '%s'\n", type, str);

	ಗಂಟು = fdt_subnode_ಸ್ಥಾನಾಂತರ(ctx->fit, ctx->images_ಗಂಟು, str);
	ಆದರೆ (ಗಂಟು < 0) {
		pr_err("cannot find image node '%s': %d\n", str, ಗಂಟು);
		ಹಿಂತಿರುಗಿ -EINVAL;
	}

	ಹಿಂತಿರುಗಿ ಗಂಟು;
}

ಸ್ಥಿರ u32 get_aligned_image_ಸ್ಥಾನಾಂತರ(ರಚನೆ spl_ತುಂಬು_info *info, u32 ಸ್ಥಾನಾಂತರ)
{
	ಹಿಂತಿರುಗಿ ALIGN_ಕೆಳಕ್ಕೆ(ಸ್ಥಾನಾಂತರ, spl_get_bl_len(info));
}

ಸ್ಥಿರ u32 get_aligned_image_overhead(ರಚನೆ spl_ತುಂಬು_info *info, u32 ಸ್ಥಾನಾಂತರ)
{
	ಹಿಂತಿರುಗಿ ಸ್ಥಾನಾಂತರ & (spl_get_bl_len(info) - 1);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ get_aligned_image_ಗಾತ್ರ(ರಚನೆ spl_ತುಂಬು_info *info, ulong ದತ್ತಾಂಶ_ಗಾತ್ರ,
				  u32 ಸ್ಥಾನಾಂತರ, ulong *aligned_ಗಾತ್ರ)
{
	u32 overhead = get_aligned_image_overhead(info, ಸ್ಥಾನಾಂತರ);

	ಆದರೆ (ದತ್ತಾಂಶ_ಗಾತ್ರ > ULONG_MAX - overhead)
		ಹಿಂತಿರುಗಿ -EOVERFLOW;
	ದತ್ತಾಂಶ_ಗಾತ್ರ += overhead;

	ಆದರೆ (ದತ್ತಾಂಶ_ಗಾತ್ರ > ULONG_MAX - (spl_get_bl_len(info) - 1))
		ಹಿಂತಿರುಗಿ -EOVERFLOW;
	ದತ್ತಾಂಶ_ಗಾತ್ರ = ALIGN(ದತ್ತಾಂಶ_ಗಾತ್ರ, spl_get_bl_len(info));

	*aligned_ಗಾತ್ರ = ದತ್ತಾಂಶ_ಗಾತ್ರ;

	ಹಿಂತಿರುಗಿ 0;
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
ಸ್ಥಿರ ಶೂನ್ಯ fit_fill_image_info(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *fit, ಪೂರ್ಣಾಂಕ ಗಂಟು,
				ರಚನೆ spl_image_info *image_info,
				ulong ತುಂಬು_addr, ulong ಗಾತ್ರ_2)
{
	ulong ನಮೂದು_ಬಿಂದು;

	ಆದರೆ (!image_info)
		ಹಿಂತಿರುಗಿ;

	image_info->ತುಂಬು_addr = ತುಂಬು_addr;
	image_info->ಗಾತ್ರ_2 = ಗಾತ್ರ_2;

	ಆದರೆ (!fit_image_get_ನಮೂದು(fit, ಗಂಟು, &ನಮೂದು_ಬಿಂದು))
		image_info->ನಮೂದು_ಬಿಂದು = ನಮೂದು_ಬಿಂದು;
	ಇಲ್ಲದಿದ್ದರೆ
		image_info->ನಮೂದು_ಬಿಂದು = FDT_ERROR;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ತುಂಬು_simple_fit(ರಚನೆ spl_ತುಂಬು_info *info, ulong fit_ಸ್ಥಾನಾಂತರ,
			   ಸ್ಥಿರಾಂಕ ರಚನೆ spl_fit_info *ctx, ಪೂರ್ಣಾಂಕ ಗಂಟು,
			   ರಚನೆ spl_image_info *image_info, ulong max_ಗಾತ್ರ)
{
	u32 ಸ್ಥಾನಾಂತರ;
	u32 len;
	size_t ಉದ್ದಳತೆ;
	ulong ಗಾತ್ರ_2;
	ulong ತುಂಬು_addr;
	ಶೂನ್ಯ *ತುಂಬು_ptr;
	ಶೂನ್ಯ *src;
	uint8_t image_comp = -1, type = -1;
	ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *ದತ್ತಾಂಶ;
	ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *fit = ctx->fit;
	bool external_ದತ್ತಾಂಶ = false;
	ಪೂರ್ಣಾಂಕ ret;

	log_debug("starting\n");
	ಆದರೆ (CONFIG_IS_ENABLED(BOOTMETH_VBE) &&
	    xpl_get_phase(info) != IH_PHASE_NONE) {
		ಎಣಿಕೆ image_phase_t phase;

		ret = fit_image_get_phase(fit, ಗಂಟು, &phase);
		/* if the image is for any phase, let's use it */
		ಆದರೆ (ret == -ENOENT || phase == xpl_get_phase(info)) {
			log_debug("found\n");
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ret < 0) {
			log_debug("err=%d\n", ret);
			ಹಿಂತಿರುಗಿ ret;
		} ಇಲ್ಲದಿದ್ದರೆ {
			log_debug("- phase mismatch, skipping this image\n");
			ಹಿಂತಿರುಗಿ -EBADSLT;
		}
	}

	ಆದರೆ (IS_ENABLED(CONFIG_SPL_FPGA) ||
	    (IS_ENABLED(CONFIG_SPL_OS_BOOT) && spl_decompression_enabled())) {
		ಆದರೆ (fit_image_get_type(fit, ಗಂಟು, &type))
			puts("Cannot get image type.\n");
		ಇಲ್ಲದಿದ್ದರೆ
			debug("%s ", genimg_get_type_name(type));
	}

	ಆದರೆ (spl_decompression_enabled()) {
		fit_image_get_comp(fit, ಗಂಟು, &image_comp);
		debug("%s ", genimg_get_comp_name(image_comp));
	}

	ಆದರೆ (fit_image_get_ತುಂಬು(fit, ಗಂಟು, &ತುಂಬು_addr)) {
		ಆದರೆ (!image_info->ತುಂಬು_addr) {
			ಬರೆ("Can't load %s: No load address and no buffer\n",
			       fit_get_name(fit, ಗಂಟು, ಖಾಲಿ));
			ಹಿಂತಿರುಗಿ -ENOBUFS;
		}
		ತುಂಬು_addr = image_info->ತುಂಬು_addr;
	}

	ಆದರೆ (!fit_image_get_ದತ್ತಾಂಶ_position(fit, ಗಂಟು, &ಸ್ಥಾನಾಂತರ)) {
		external_ದತ್ತಾಂಶ = true;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (!fit_image_get_ದತ್ತಾಂಶ_ಸ್ಥಾನಾಂತರ(fit, ಗಂಟು, &ಸ್ಥಾನಾಂತರ)) {
		/* The resulting offset cannot exceed UINT32_MAX */
		ಆದರೆ (ctx->ext_ದತ್ತಾಂಶ_ಸ್ಥಾನಾಂತರ > UINT32_MAX - ಸ್ಥಾನಾಂತರ) {
			log_debug("Invalid external data offset: %u\n", ಸ್ಥಾನಾಂತರ);
			ಹಿಂತಿರುಗಿ -EINVAL;
		}
		log_debug("read offset %x = offset from fit %x\n", ಸ್ಥಾನಾಂತರ,
			  (u32)(ಸ್ಥಾನಾಂತರ + ctx->ext_ದತ್ತಾಂಶ_ಸ್ಥಾನಾಂತರ));
		ಸ್ಥಾನಾಂತರ += ctx->ext_ದತ್ತಾಂಶ_ಸ್ಥಾನಾಂತರ;
		external_ದತ್ತಾಂಶ = true;
	}

	ಆದರೆ (external_ದತ್ತಾಂಶ) {
		u32 aligned_ಸ್ಥಾನಾಂತರ;
		ulong read_ಸ್ಥಾನಾಂತರ;
		ಶೂನ್ಯ *src_ptr;

		/* External data */
		ಆದರೆ (fit_image_get_ದತ್ತಾಂಶ_ಗಾತ್ರ(fit, ಗಂಟು, &len))
			ಹಿಂತಿರುಗಿ -ENOENT;

		/* Dont bother to copy 0 byte data, but warn, though */
		ಆದರೆ (!len) {
			log_warning("%s: Skip load '%s': image size is 0!\n",
				    __func__, fit_get_name(fit, ಗಂಟು, ಖಾಲಿ));
			fit_fill_image_info(fit, ಗಂಟು, image_info, ತುಂಬು_addr, 0);
			ಹಿಂತಿರುಗಿ 0;
		}

		/*
		 * data-size is excluded from the configuration signature (it
		 * is in exc_prop[] in image-fit-sig.c), so it stays attacker
		 * controlled even after fit_config_verify() succeeds. The
		 * image hash is only verified after the device read below, so
		 * an oversized value has to be rejected here.
		 */
		ret = get_aligned_image_ಗಾತ್ರ(info, len, ಸ್ಥಾನಾಂತರ, &ಗಾತ್ರ_2);
		ಆದರೆ (ret) {
			log_debug("Invalid external data size: %u\n", len);
			ಹಿಂತಿರುಗಿ ret;
		}

		ಆದರೆ (ಗಾತ್ರ_2 > max_ಗಾತ್ರ) {
			log_debug("Image too large: aligned size %lu, max %lu (data-size %u)\n",
				  ಗಾತ್ರ_2, max_ಗಾತ್ರ, len);
			ಹಿಂತಿರುಗಿ -EFBIG;
		}

		aligned_ಸ್ಥಾನಾಂತರ = get_aligned_image_ಸ್ಥಾನಾಂತರ(info, ಸ್ಥಾನಾಂತರ);
		ಆದರೆ (aligned_ಸ್ಥಾನಾಂತರ > ULONG_MAX - fit_ಸ್ಥಾನಾಂತರ) {
			log_debug("Invalid aligned external data offset: %u\n",
				  aligned_ಸ್ಥಾನಾಂತರ);
			ಹಿಂತಿರುಗಿ -EINVAL;
		}
		read_ಸ್ಥಾನಾಂತರ = fit_ಸ್ಥಾನಾಂತರ + aligned_ಸ್ಥಾನಾಂತರ;

		ಆದರೆ (spl_decompression_enabled() &&
		    (image_comp == IH_COMP_GZIP || image_comp == IH_COMP_LZMA))
			src_ptr = map_sysmem(ALIGN(CONFIG_SYS_ತುಂಬು_ADDR, ARCH_DMA_MINALIGN), len);
		ಇಲ್ಲದಿದ್ದರೆ
			src_ptr = map_sysmem(ALIGN(ತುಂಬು_addr, ARCH_DMA_MINALIGN), len);
		ಉದ್ದಳತೆ = len;

		/*
		 * For non-signed FIT images, we can check that
		 * (read_offset + size) does not wrap and that
		 * (src_ptr + size) does not exceed the addressable range.
		 * For signed FITs, we can additionally check that
		 * (offset + len) doesn't exceed the allowed FIT image
		 * maximum size.
		 */
		ಆದರೆ (ಗಾತ್ರ_2 > ULONG_MAX - read_ಸ್ಥಾನಾಂತರ ||
		    ಗಾತ್ರ_2 > UINTPTR_MAX - (uintptr_t)src_ptr
		/*
		 * #if (not a runtime if) is required: FIT_SIGNATURE_MAX_SIZE
		 * depends on FIT_SIGNATURE, so CONFIG_VAL(FIT_SIGNATURE_MAX_SIZE)
		 * is undefined when signing is disabled and referencing it
		 * here would fail to compile.
		 */
#ಆದರೆ CONFIG_IS_ENABLED(FIT_SIGNATURE)
		    || ಸ್ಥಾನಾಂತರ > CONFIG_VAL(FIT_SIGNATURE_MAX_ಗಾತ್ರ) ||
		    len > CONFIG_VAL(FIT_SIGNATURE_MAX_ಗಾತ್ರ) - ಸ್ಥಾನಾಂತರ
#ಆದರೆ_ಅಂತ್ಯ
		) {
			log_debug("FIT external data is out of bounds (offset=%u, size=%u)\n",
				  ಸ್ಥಾನಾಂತರ, len);
			ಹಿಂತಿರುಗಿ -EINVAL;
		}

		log_debug("reading from offset %x / %lx size %lx to %p: ",
			  ಸ್ಥಾನಾಂತರ, read_ಸ್ಥಾನಾಂತರ, ಗಾತ್ರ_2, src_ptr);

		ಆದರೆ (info->ಪಠ(info, read_ಸ್ಥಾನಾಂತರ, ಗಾತ್ರ_2, src_ptr) < ಉದ್ದಳತೆ)
			ಹಿಂತಿರುಗಿ -EIO;

		debug("External data: dst=%p, offset=%x, size=%lx\n",
		      src_ptr, ಸ್ಥಾನಾಂತರ, (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)ಉದ್ದಳತೆ);
		src = src_ptr + get_aligned_image_overhead(info, ಸ್ಥಾನಾಂತರ);
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* Embedded data */
		ಆದರೆ (fit_image_get_emb_ದತ್ತಾಂಶ(fit, ಗಂಟು, &ದತ್ತಾಂಶ, &ಉದ್ದಳತೆ)) {
			puts("Cannot get image data/size\n");
			ಹಿಂತಿರುಗಿ -ENOENT;
		}
		debug("Embedded data: dst=%lx, size=%lx\n", ತುಂಬು_addr,
		      (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)ಉದ್ದಳತೆ);
		src = (ಶೂನ್ಯ *)ದತ್ತಾಂಶ;	/* cast away const */
	}

	ಆದರೆ (CONFIG_IS_ENABLED(FIT_SIGNATURE)) {
		ಬರೆ("## Checking hash(es) for Image %s ... ",
		       fit_get_name(fit, ಗಂಟು, ಖಾಲಿ));
		ಆದರೆ (!fit_image_verify_with_ದತ್ತಾಂಶ(fit, ಗಂಟು, gd_fdt_blob(), src,
						ಉದ್ದಳತೆ))
			ಹಿಂತಿರುಗಿ -EPERM;
		puts("OK\n");
	}

	ಆದರೆ (CONFIG_IS_ENABLED(FIT_IMAGE_POST_PROCESS))
		board_fit_image_post_process(fit, ಗಂಟು, &src, &ಉದ್ದಳತೆ);

	ತುಂಬು_ptr = map_sysmem(ತುಂಬು_addr, ಉದ್ದಳತೆ);
	ಆದರೆ (IS_ENABLED(CONFIG_SPL_GZIP) && image_comp == IH_COMP_GZIP) {
		ಗಾತ್ರ_2 = ಉದ್ದಳತೆ;
		ಆದರೆ (gunzip(ತುಂಬು_ptr, CONFIG_SYS_BOOTM_LEN, src, &ಗಾತ್ರ_2)) {
			puts("Uncompressing error\n");
			ಹಿಂತಿರುಗಿ -EIO;
		}
		ಉದ್ದಳತೆ = ಗಾತ್ರ_2;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (IS_ENABLED(CONFIG_SPL_LZMA) && image_comp == IH_COMP_LZMA) {
		ಗಾತ್ರ_2 = CONFIG_SYS_BOOTM_LEN;
		ulong ತುಂಬು_ಕೊನೆ;

		ಆದರೆ (image_decomp(IH_COMP_LZMA, CONFIG_SYS_ತುಂಬು_ADDR, 0, 0,
				 ತುಂಬು_ptr, src, ಉದ್ದಳತೆ, ಗಾತ್ರ_2, &ತುಂಬು_ಕೊನೆ)) {
			puts("Uncompressing error\n");
			ಹಿಂತಿರುಗಿ -EIO;
		}
		ಉದ್ದಳತೆ = ತುಂಬು_ಕೊನೆ - CONFIG_SYS_ತುಂಬು_ADDR;
	} ಇಲ್ಲದಿದ್ದರೆ {
		memmove(ತುಂಬು_ptr, src, ಉದ್ದಳತೆ);
	}

	fit_fill_image_info(fit, ಗಂಟು, image_info, ತುಂಬು_addr, ಉದ್ದಳತೆ);
	log_debug("- done loading\n");

	upl_ಕೂಡಿಸು_image(fit, ಗಂಟು, ತುಂಬು_addr, ಉದ್ದಳತೆ);

	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ bool os_takes_devicetree(uint8_t os)
{
	ಆಯ್ಕೆ (os) {
	ಸ್ಥಿತಿ IH_OS_U_BOOT:
		ಹಿಂತಿರುಗಿ true;
	ಸ್ಥಿತಿ IH_OS_LINUX:
		ಹಿಂತಿರುಗಿ IS_ENABLED(CONFIG_SPL_OS_BOOT) ||
		       IS_ENABLED(CONFIG_SPL_OPENSBI);
	ಪೂರ್ವನಿಯೋಜಿತ:
		ಹಿಂತಿರುಗಿ false;
	}
}

__weak ಪೂರ್ಣಾಂಕ board_spl_fit_append_fdt_skip(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name)
{
	ಹಿಂತಿರುಗಿ 0;	/* Do not skip */
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ spl_fit_append_fdt(ರಚನೆ spl_image_info *spl_image,
			      ರಚನೆ spl_ತುಂಬು_info *info, ulong ಸ್ಥಾನಾಂತರ,
			      ಸ್ಥಿರಾಂಕ ರಚನೆ spl_fit_info *ctx)
{
	ರಚನೆ spl_image_info image_info;
	ಪೂರ್ಣಾಂಕ ಗಂಟು, ret = 0, ಸೂಚ್ಯಂಕ = 0;

	/*
	 * Use the address following the image as target address for the
	 * device tree.
	 */
	image_info.ತುಂಬು_addr = ALIGN(spl_image->ತುಂಬು_addr + spl_image->ಗಾತ್ರ_2, 8);

	/* Figure out which device tree the board wants to use */
	ಗಂಟು = spl_fit_get_image_ಗಂಟು(ctx, FIT_FDT_PROP, ಸೂಚ್ಯಂಕ++);
	ಆದರೆ (ಗಂಟು < 0) {
		size_t ಗಾತ್ರ_2;

		debug("%s: cannot find FDT node\n", __func__);

		/*
		 * U-Boot did not find a device tree inside the FIT image. Use
		 * the U-Boot device tree instead.
		 */
		ಆದರೆ (!gd->fdt_blob)
			ಹಿಂತಿರುಗಿ ಗಂಟು;

		/*
		 * Make the load-address of the FDT available for the SPL
		 * framework
		 */
		ಗಾತ್ರ_2 = fdt_totalsize(gd->fdt_blob);
		spl_image->fdt_addr = map_sysmem(image_info.ತುಂಬು_addr, ಗಾತ್ರ_2);
		ಸ್ಮರಣೆ_ನಕಲು(spl_image->fdt_addr, gd->fdt_blob, ಗಾತ್ರ_2);
	} ಇಲ್ಲದಿದ್ದರೆ {
		ret = ತುಂಬು_simple_fit(info, ಸ್ಥಾನಾಂತರ, ctx, ಗಂಟು, &image_info,
				      CONFIG_SYS_BOOTM_LEN);
		ಆದರೆ (ret < 0)
			ಹಿಂತಿರುಗಿ ret;

		spl_image->fdt_addr = phys_ಗಮ್ಯ_virt(image_info.ತುಂಬು_addr);
	}

	ಆದರೆ (CONFIG_IS_ENABLED(FIT_IMAGE_TINY))
		ಹಿಂತಿರುಗಿ 0;

#ಆದರೆ CONFIG_IS_ENABLED(ತುಂಬು_FIT_APPLY_OVERLAY)
		ಶೂನ್ಯ *tmpbuffer = ಖಾಲಿ;

		ಗಾಗಿ (; ; ಸೂಚ್ಯಂಕ++) {
			ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str;

			ret = spl_fit_get_image_name(ctx, FIT_FDT_PROP, ಸೂಚ್ಯಂಕ, &str);
			ಆದರೆ (ret == -E2BIG) {
				debug("%s: No additional FDT node\n", __func__);
				ret = 0;
				ನಿಲ್ಲಿಸು;
			} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ret) {
				ನಿಲ್ಲಿಸು;
			}

			ret = board_spl_fit_append_fdt_skip(str);
			ಆದರೆ (ret)
				ಮುಂದುವರಿ;

			ಗಂಟು = fdt_subnode_ಸ್ಥಾನಾಂತರ(ctx->fit, ctx->images_ಗಂಟು, str);
			ಆದರೆ (ಗಂಟು < 0) {
				debug("%s: unable to find FDT node %d\n",
				      __func__, ಸೂಚ್ಯಂಕ);
				ಮುಂದುವರಿ;
			}

			ಆದರೆ (!tmpbuffer) {
				/*
				 * allocate memory to store the DT overlay
				 * before it is applied. It may not be used
				 * depending on how the overlay is stored, so
				 * don't fail yet if the allocation failed.
				 */
				size_t ಗಾತ್ರ_2 = CONFIG_SPL_ತುಂಬು_FIT_APPLY_OVERLAY_BUF_SZ;

				tmpbuffer = malloc_cache_aligned(ಗಾತ್ರ_2);
				ಆದರೆ (!tmpbuffer)
					debug("%s: unable to allocate space for overlays\n",
					      __func__);
			}
			image_info.ತುಂಬು_addr = (ulong)tmpbuffer;
			ret = ತುಂಬು_simple_fit(info, ಸ್ಥಾನಾಂತರ, ctx, ಗಂಟು,
					      &image_info,
					      CONFIG_SPL_ತುಂಬು_FIT_APPLY_OVERLAY_BUF_SZ);
			ಆದರೆ (ret == -EBADSLT)
				ಮುಂದುವರಿ;
			ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ret < 0)
				ನಿಲ್ಲಿಸು;

			/* Make room in FDT for changes from the overlay */
			ret = fdt_increase_ಗಾತ್ರ(spl_image->fdt_addr,
						image_info.ಗಾತ್ರ_2);
			ಆದರೆ (ret < 0)
				ನಿಲ್ಲಿಸು;

			ret = fdt_overlay_apply_verbose(spl_image->fdt_addr,
							(ಶೂನ್ಯ *)image_info.ತುಂಬು_addr);
			ಆದರೆ (ret) {
				pr_err("failed to apply DT overlay %s\n",
				       fit_get_name(ctx->fit, ಗಂಟು, ಖಾಲಿ));
				ನಿಲ್ಲಿಸು;
			}

			debug("%s: DT overlay %s applied\n", __func__,
			      fit_get_name(ctx->fit, ಗಂಟು, ಖಾಲಿ));
		}
		ಬಿಡುಗಡೆ(tmpbuffer);
		ಆದರೆ (ret)
			ಹಿಂತಿರುಗಿ ret;
#ಆದರೆ_ಅಂತ್ಯ
	/* Try to make space, so we can inject details on the loadables */
	ret = fdt_shrink_ಗಮ್ಯ_minimum(spl_image->fdt_addr, 8192);
	ಆದರೆ (ret < 0)
		ಹಿಂತಿರುಗಿ ret;

	ಹಿಂತಿರುಗಿ ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ spl_fit_ದಾಖಲೆ_loadable(ಸ್ಥಿರಾಂಕ ರಚನೆ spl_fit_info *ctx, ಪೂರ್ಣಾಂಕ ಸೂಚ್ಯಂಕ,
				   ಶೂನ್ಯ *blob, ರಚನೆ spl_image_info *image)
{
	ಪೂರ್ಣಾಂಕ ret = 0;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name;
	ಪೂರ್ಣಾಂಕ ಗಂಟು;

	ret = spl_fit_get_image_name(ctx, FIT_LOADABLE_PROP, ಸೂಚ್ಯಂಕ, &name);
	ಆದರೆ (ret < 0)
		ಹಿಂತಿರುಗಿ ret;

	ಗಂಟು = spl_fit_get_image_ಗಂಟು(ctx, FIT_LOADABLE_PROP, ಸೂಚ್ಯಂಕ);

	ret = fdt_ದಾಖಲೆ_loadable(blob, ಸೂಚ್ಯಂಕ, name, image->ತುಂಬು_addr,
			image->ಗಾತ್ರ_2, image->ನಮೂದು_ಬಿಂದು,
			fdt_getprop(ctx->fit, ಗಂಟು, FIT_TYPE_PROP, ಖಾಲಿ),
			fdt_getprop(ctx->fit, ಗಂಟು, FIT_OS_PROP, ಖಾಲಿ),
			fdt_getprop(ctx->fit, ಗಂಟು, FIT_ARCH_PROP, ಖಾಲಿ));

	ಹಿಂತಿರುಗಿ ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ spl_fit_image_is_fpga(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *fit, ಪೂರ್ಣಾಂಕ ಗಂಟು)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *type;

	ಆದರೆ (!IS_ENABLED(CONFIG_SPL_FPGA))
		ಹಿಂತಿರುಗಿ 0;

	type = fdt_getprop(fit, ಗಂಟು, FIT_TYPE_PROP, ಖಾಲಿ);
	ಆದರೆ (!type)
		ಹಿಂತಿರುಗಿ 0;

	ಹಿಂತಿರುಗಿ !strcmp(type, "fpga");
}

ಸ್ಥಿರ ಶೂನ್ಯ spl_fit_image_ದಾಖಲೆ_arm32_optee(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *fit, ಪೂರ್ಣಾಂಕ ಗಂಟು,
					     ರಚನೆ spl_image_info *spl_image,
					     ರಚನೆ spl_image_info *image_info)
{
#ಆದರೆ defined(CONFIG_BOOTM_OPTEE) && defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *type = fdt_getprop(fit, ಗಂಟು, FIT_TYPE_PROP, ಖಾಲಿ);

	ಆದರೆ (!type)
		ಹಿಂತಿರುಗಿ;

	ಆದರೆ (strcmp(type, "tee"))
		ಹಿಂತಿರುಗಿ;

	spl_image->optee_addr = image_info->ತುಂಬು_addr;
#ಆದರೆ_ಅಂತ್ಯ
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ spl_fit_image_get_os(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *fit, ಪೂರ್ಣಾಂಕ noffset, uint8_t *os)
{
	ಆದರೆ (!CONFIG_IS_ENABLED(FIT_IMAGE_TINY) || CONFIG_IS_ENABLED(OS_BOOT))
		ಹಿಂತಿರುಗಿ fit_image_get_os(fit, noffset, os);

	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name = fdt_getprop(fit, noffset, FIT_OS_PROP, ಖಾಲಿ);
	ಆದರೆ (!name)
		ಹಿಂತಿರುಗಿ -ENOENT;

	/*
	 * We don't care what the type of the image actually is,
	 * only whether or not it is U-Boot. This saves some
	 * space by omitting the large table of OS types.
	 */
	ಆದರೆ (!strcmp(name, "u-boot"))
		*os = IH_OS_U_BOOT;
	ಇಲ್ಲದಿದ್ದರೆ
		*os = IH_OS_INVALID;

	ಹಿಂತಿರುಗಿ 0;
}

/*
 * The purpose of the FIT load buffer is to provide a memory location that is
 * independent of the load address of any FIT component.
 */
__weak ಶೂನ್ಯ *board_spl_fit_ಬಫರ್_addr(ulong fit_ಗಾತ್ರ, ಪೂರ್ಣಾಂಕ sectors, ಪೂರ್ಣಾಂಕ bl_len)
{
	ಶೂನ್ಯ *buf;
	size_t ಗಾತ್ರ_2 = sectors * bl_len;

	buf = malloc_cache_aligned(ಗಾತ್ರ_2);
	ಆದರೆ (!buf) {
		pr_err("Could not get FIT buffer of %lu bytes\n", (ulong)ಗಾತ್ರ_2);

		ಆದರೆ (IS_ENABLED(CONFIG_SPL_SYS_MALLOC))
			pr_err("\tcheck CONFIG_SPL_SYS_MALLOC_SIZE\n");
		ಇಲ್ಲದಿದ್ದರೆ
			pr_err("\tcheck CONFIG_SPL_SYS_MALLOC_F_LEN\n");

		buf = spl_get_ತುಂಬು_ಬಫರ್(0, ಗಾತ್ರ_2);
	}
	ಹಿಂತಿರುಗಿ buf;
}

/*
 * Weak default function to allow customizing SPL fit loading for load-only
 * use cases by allowing to skip the parsing/processing of the FIT contents
 * (so that this can be done separately in a more customized fashion)
 */
__weak bool spl_ತುಂಬು_simple_fit_skip_processing(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ false;
}

/*
 * Weak default function to allow fixes after fit header
 * is loaded.
 */
__weak ಶೂನ್ಯ *spl_ತುಂಬು_simple_fit_fix_ತುಂಬು(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *fit)
{
	ಹಿಂತಿರುಗಿ (ಶೂನ್ಯ *)fit;
}

ಸ್ಥಿರ ಶೂನ್ಯ warn_deprecated(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ)
{
	ಬರೆ("DEPRECATED: %s\n", ಸಂದೇಶ);
	ಬರೆ("\tSee https://fitspec.osfw.foundation/\n");
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ spl_fit_upload_fpga(ರಚನೆ spl_fit_info *ctx, ಪೂರ್ಣಾಂಕ ಗಂಟು,
			       ರಚನೆ spl_image_info *fpga_image)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *compatible;
	ಪೂರ್ಣಾಂಕ ret;
	ಪೂರ್ಣಾಂಕ devnum = 0;
	ಪೂರ್ಣಾಂಕ flags = 0;

	debug("FPGA bitstream at: %x, size: %x\n",
	      (u32)fpga_image->ತುಂಬು_addr, fpga_image->ಗಾತ್ರ_2);

	compatible = fdt_getprop(ctx->fit, ಗಂಟು, FIT_COMPAT_PROP, ಖಾಲಿ);
	ಆದರೆ (!compatible) {
		warn_deprecated("'fpga' image without 'compatible' property");
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಆದರೆ (CONFIG_IS_ENABLED(FPGA_ತುಂಬು_SECURE))
			flags = fpga_compatible2flag(devnum, compatible);
		ಆದರೆ (strcmp(compatible, "u-boot,fpga-legacy"))
			debug("Ignoring compatible = %s property\n",
			      compatible);
	}

	ret = fpga_ತುಂಬು(devnum, (ಶೂನ್ಯ *)fpga_image->ತುಂಬು_addr,
			fpga_image->ಗಾತ್ರ_2, ಬಿಟ್_ತುಂಬಿದ, flags);
	ಆದರೆ (ret) {
		ಬರೆ("%s: Cannot load the image to the FPGA\n", __func__);
		ಹಿಂತಿರುಗಿ ret;
	}

	puts("FPGA image loaded from FIT\n");

	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ spl_fit_ತುಂಬು_fpga(ರಚನೆ spl_fit_info *ctx,
			     ರಚನೆ spl_ತುಂಬು_info *info, ulong ಸ್ಥಾನಾಂತರ)
{
	ಪೂರ್ಣಾಂಕ ಗಂಟು, ret;

	ರಚನೆ spl_image_info fpga_image = {
		.ತುಂಬು_addr = 0,
	};

	ಗಂಟು = spl_fit_get_image_ಗಂಟು(ctx, FIT_FPGA_PROP, 0);
	ಆದರೆ (ಗಂಟು < 0)
		ಹಿಂತಿರುಗಿ ಗಂಟು;

	warn_deprecated("'fpga' property in config node. Use 'loadables'");

	/* Load the image and set up the fpga_image structure */
	ret = ತುಂಬು_simple_fit(info, ಸ್ಥಾನಾಂತರ, ctx, ಗಂಟು, &fpga_image,
			      CONFIG_SYS_BOOTM_LEN);
	ಆದರೆ (ret) {
		ಬರೆ("%s: Cannot load the FPGA: %i\n", __func__, ret);
		ಹಿಂತಿರುಗಿ ret;
	}

	ಹಿಂತಿರುಗಿ spl_fit_upload_fpga(ctx, ಗಂಟು, &fpga_image);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ spl_simple_fit_read(ರಚನೆ spl_fit_info *ctx,
			       ರಚನೆ spl_ತುಂಬು_info *info, ulong ಸ್ಥಾನಾಂತರ,
			       ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *fit_ಶೀರ್ಷಿಕೆ)
{
	ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ aligned_ಗಾತ್ರ, ಎಣಿಕೆ_2, ಗಾತ್ರ_2;
	ಶೂನ್ಯ *buf;
	ಪೂರ್ಣಾಂಕ ret;

	/*
	 * For FIT with external data, figure out where the external images
	 * start. This is the base for the data-offset properties in each
	 * image.
	 */
	ಗಾತ್ರ_2 = ALIGN(fdt_totalsize(fit_ಶೀರ್ಷಿಕೆ), 4);
	ಗಾತ್ರ_2 = board_spl_fit_ಗಾತ್ರ_align(ಗಾತ್ರ_2);
	ctx->ext_ದತ್ತಾಂಶ_ಸ್ಥಾನಾಂತರ = ALIGN(ಗಾತ್ರ_2, 4);

	/*
	 * So far we only have one block of data from the FIT. Read the entire
	 * thing, including that first block.
	 *
	 * For FIT with data embedded, data is loaded as part of FIT image.
	 * For FIT with external data, data is not loaded in this step.
	 */
	ret = get_aligned_image_ಗಾತ್ರ(info, ಗಾತ್ರ_2, 0, &aligned_ಗಾತ್ರ);
	ಆದರೆ (ret) {
		log_debug("Invalid FIT size: %lu\n", ಗಾತ್ರ_2);
		ಹಿಂತಿರುಗಿ ret;
	}
	buf = board_spl_fit_ಬಫರ್_addr(aligned_ಗಾತ್ರ, aligned_ಗಾತ್ರ, 1);
	ಆದರೆ (!buf) {
		/*
		 * We assume that none of the board will ever use 0x0 as a
		 * valid load address. Theoretically some board could use it,
		 * but this is extremely unlikely.
		 */
		ಹಿಂತಿರುಗಿ -EIO;
	}

	ಎಣಿಕೆ_2 = info->ಪಠ(info, ಸ್ಥಾನಾಂತರ, aligned_ಗಾತ್ರ, buf);
	ಆದರೆ (!ಎಣಿಕೆ_2) {
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
		ಹಿಂತಿರುಗಿ -EIO;
	}

	ctx->fit = buf;
	debug("fit read offset %lx, size=%lu, dst=%p, count=%lu\n",
	      ಸ್ಥಾನಾಂತರ, aligned_ಗಾತ್ರ, buf, ಎಣಿಕೆ_2);

	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ spl_simple_fit_parse(ರಚನೆ spl_fit_info *ctx)
{
	/* Find the correct subnode under "/configurations" */
	ctx->conf_ಗಂಟು = fit_ಕಂಡುಹಿಡಿ_config_ಗಂಟು(ctx->fit);
	ಆದರೆ (ctx->conf_ಗಂಟು < 0)
		ಹಿಂತಿರುಗಿ -EINVAL;

	ಆದರೆ (CONFIG_IS_ENABLED(FIT_SIGNATURE)) {
		ಬರೆ("## Checking hash(es) for config %s ... ",
		       fit_get_name(ctx->fit, ctx->conf_ಗಂಟು, ಖಾಲಿ));
		ಆದರೆ (fit_config_verify(ctx->fit, ctx->conf_ಗಂಟು))
			ಹಿಂತಿರುಗಿ -EPERM;
		puts("OK\n");
	}

	/* find the node holding the images information */
	ctx->images_ಗಂಟು = fdt_ಮಾರ್ಗ_ಸ್ಥಾನಾಂತರ(ctx->fit, FIT_IMAGES_ಮಾರ್ಗ);
	ಆದರೆ (ctx->images_ಗಂಟು < 0) {
		debug("%s: Cannot find /images node: %d\n", __func__,
		      ctx->images_ಗಂಟು);
		ಹಿಂತಿರುಗಿ -EINVAL;
	}

	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ spl_ತುಂಬು_simple_fit(ರಚನೆ spl_image_info *spl_image,
			ರಚನೆ spl_ತುಂಬು_info *info, ulong ಸ್ಥಾನಾಂತರ, ಶೂನ್ಯ *fit)
{
	ರಚನೆ spl_image_info image_info;
	ರಚನೆ spl_fit_info ctx;
	ಪೂರ್ಣಾಂಕ ಗಂಟು = -1;
	ಪೂರ್ಣಾಂಕ ret;
	ಪೂರ್ಣಾಂಕ ಸೂಚ್ಯಂಕ = 0;
	ಪೂರ್ಣಾಂಕ firmware_ಗಂಟು;

	ret = spl_simple_fit_read(&ctx, info, ಸ್ಥಾನಾಂತರ, fit);
	ಆದರೆ (ret < 0)
		ಹಿಂತಿರುಗಿ ret;

	/* skip further processing if requested to enable load-only use cases */
	ಆದರೆ (spl_ತುಂಬು_simple_fit_skip_processing())
		ಹಿಂತಿರುಗಿ 0;

	ctx.fit = spl_ತುಂಬು_simple_fit_fix_ತುಂಬು(ctx.fit);

	ret = spl_simple_fit_parse(&ctx);
	ಆದರೆ (ret < 0)
		ಹಿಂತಿರುಗಿ ret;

	ಆದರೆ (IS_ENABLED(CONFIG_SPL_FPGA))
		spl_fit_ತುಂಬು_fpga(&ctx, info, ಸ್ಥಾನಾಂತರ);

	/*
	 * Find the U-Boot image using the following search order:
	 *   - start at 'firmware' (e.g. an ARM Trusted Firmware)
	 *   - fall back 'kernel' (e.g. a Falcon-mode OS boot
	 *   - fall back to using the first 'loadables' entry
	 */
	ಆದರೆ (ಗಂಟು < 0)
		ಗಂಟು = spl_fit_get_image_ಗಂಟು(&ctx, FIT_FIRMWARE_PROP, 0);

	ಆದರೆ (ಗಂಟು < 0 && IS_ENABLED(CONFIG_SPL_OS_BOOT))
		ಗಂಟು = spl_fit_get_image_ಗಂಟು(&ctx, FIT_KERNEL_PROP, 0);

	ಆದರೆ (ಗಂಟು < 0) {
		debug("could not find firmware image, trying loadables...\n");
		ಗಂಟು = spl_fit_get_image_ಗಂಟು(&ctx, FIT_LOADABLE_PROP, 0);
		/*
		 * If we pick the U-Boot image from "loadables", start at
		 * the second image when later loading additional images.
		 */
		ಸೂಚ್ಯಂಕ = 1;
	}
	ಆದರೆ (ಗಂಟು < 0) {
		debug("%s: Cannot find u-boot image node: %d\n",
		      __func__, ಗಂಟು);
		ಹಿಂತಿರುಗಿ -1;
	}

	/* Load the image and set up the spl_image structure */
	ret = ತುಂಬು_simple_fit(info, ಸ್ಥಾನಾಂತರ, &ctx, ಗಂಟು, spl_image,
			      CONFIG_SYS_BOOTM_LEN);
	ಆದರೆ (ret)
		ಹಿಂತಿರುಗಿ ret;

	/*
	 * For backward compatibility, we treat the first node that is
	 * as a U-Boot image, if no OS-type has been declared.
	 */
	ಆದರೆ (!spl_fit_image_get_os(ctx.fit, ಗಂಟು, &spl_image->os))
		debug("Image OS is %s\n", genimg_get_os_name(spl_image->os));
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (!IS_ENABLED(CONFIG_SPL_OS_BOOT))
		spl_image->os = IH_OS_U_BOOT;

	/*
	 * Booting a next-stage U-Boot may require us to append the FDT.
	 * We allow this to fail, as the U-Boot image might embed its FDT.
	 */
	ಆದರೆ (os_takes_devicetree(spl_image->os)) {
		ret = spl_fit_append_fdt(spl_image, info, ಸ್ಥಾನಾಂತರ, &ctx);
		ಆದರೆ (ret < 0 && spl_image->os != IH_OS_U_BOOT)
			ಹಿಂತಿರುಗಿ ret;
	}

	firmware_ಗಂಟು = ಗಂಟು;
	/* Now check if there are more images for us to load */
	ಗಾಗಿ (; ; ಸೂಚ್ಯಂಕ++) {
		uint8_t os_type = IH_OS_INVALID;

		ಗಂಟು = spl_fit_get_image_ಗಂಟು(&ctx, FIT_LOADABLE_PROP, ಸೂಚ್ಯಂಕ);
		ಆದರೆ (ಗಂಟು < 0)
			ನಿಲ್ಲಿಸು;

		/*
		 * if the firmware is also a loadable, skip it because
		 * it already has been loaded. This is typically the case with
		 * u-boot.img generated by mkimage.
		 */
		ಆದರೆ (firmware_ಗಂಟು == ಗಂಟು)
			ಮುಂದುವರಿ;

		image_info.ತುಂಬು_addr = 0;
		ret = ತುಂಬು_simple_fit(info, ಸ್ಥಾನಾಂತರ, &ctx, ಗಂಟು, &image_info,
				      CONFIG_SYS_BOOTM_LEN);
		ಆದರೆ (ret < 0 && ret != -EBADSLT) {
			ಬರೆ("%s: can't load image loadables index %d (ret = %d)\n",
			       __func__, ಸೂಚ್ಯಂಕ, ret);
			ಹಿಂತಿರುಗಿ ret;
		}

		ಆದರೆ (spl_fit_image_is_fpga(ctx.fit, ಗಂಟು))
			spl_fit_upload_fpga(&ctx, ಗಂಟು, &image_info);

		ಆದರೆ (!spl_fit_image_get_os(ctx.fit, ಗಂಟು, &os_type))
			debug("Loadable is %s\n", genimg_get_os_name(os_type));

		ಆದರೆ (os_takes_devicetree(os_type)) {
			spl_fit_append_fdt(&image_info, info, ಸ್ಥಾನಾಂತರ, &ctx);
			spl_image->fdt_addr = image_info.fdt_addr;
		}

		/*
		 * If the "firmware" image did not provide an entry point,
		 * use the first valid entry point from the loadables.
		 */
		ಆದರೆ (spl_image->ನಮೂದು_ಬಿಂದು == FDT_ERROR &&
		    image_info.ನಮೂದು_ಬಿಂದು != FDT_ERROR)
			spl_image->ನಮೂದು_ಬಿಂದು = image_info.ನಮೂದು_ಬಿಂದು;

		spl_fit_image_ದಾಖಲೆ_arm32_optee(ctx.fit, ಗಂಟು, spl_image,
						 &image_info);

		/* Record our loadables into the FDT */
		ಆದರೆ (!CONFIG_IS_ENABLED(FIT_IMAGE_TINY) &&
		    xpl_get_fdt_update(info) && spl_image->fdt_addr)
			spl_fit_ದಾಖಲೆ_loadable(&ctx, ಸೂಚ್ಯಂಕ,
						spl_image->fdt_addr,
						&image_info);
	}

	/*
	 * If a platform does not provide CONFIG_SYS_UBOOT_START, U-Boot's
	 * Makefile will set it to 0 and it will end up as the entry point
	 * here. What it actually means is: use the load address.
	 */
	ಆದರೆ (spl_image->ನಮೂದು_ಬಿಂದು == FDT_ERROR || spl_image->ನಮೂದು_ಬಿಂದು == 0)
		spl_image->ನಮೂದು_ಬಿಂದು = spl_image->ತುಂಬು_addr;

	spl_image->flags |= SPL_FIT_ಸಿಕ್ಕಿತು;
	upl_ಗಣ_fit_info(map_ಗಮ್ಯ_sysmem(ctx.fit), ctx.conf_ಗಂಟು,
			 spl_image->ನಮೂದು_ಬಿಂದು);

	ಹಿಂತಿರುಗಿ 0;
}

/* Parse and load full fitImage in SPL */
ಪೂರ್ಣಾಂಕ spl_ತುಂಬು_fit_image(ರಚನೆ spl_image_info *spl_image,
		       ಸ್ಥಿರಾಂಕ ರಚನೆ legacy_img_hdr *ಶೀರ್ಷಿಕೆ)
{
	ರಚನೆ bootm_ಶೀರ್ಷಿಕೆಗಳು images;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fit_uname_config = ಖಾಲಿ;
	ulong fdt_hack;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *uname;
	ulong fw_ದತ್ತಾಂಶ = 0, dt_ದತ್ತಾಂಶ = 0, img_ದತ್ತಾಂಶ = 0;
	ulong fw_len = 0, dt_len = 0, img_len = 0;
	ಪೂರ್ಣಾಂಕ idx, conf_noffset;
	ಪೂರ್ಣಾಂಕ ret;

	images.verify = CONFIG_IS_ENABLED(FIT_SIGNATURE);

	ret = fit_image_ತುಂಬು(&images, virt_ಗಮ್ಯ_phys((ಶೂನ್ಯ *)ಶೀರ್ಷಿಕೆ),
			     ಖಾಲಿ, &fit_uname_config, IH_ARCH_DEFAULT,
			     IH_TYPE_FIRMWARE, -1, FIT_ತುಂಬು_OPTIONAL,
			     &fw_ದತ್ತಾಂಶ, &fw_len);

	ಆದರೆ (ret < 0) {
		ret = fit_image_ತುಂಬು(&images, virt_ಗಮ್ಯ_phys((ಶೂನ್ಯ *)ಶೀರ್ಷಿಕೆ),
				     ಖಾಲಿ, &fit_uname_config, IH_ARCH_DEFAULT,
				     IH_TYPE_KERNEL, -1, FIT_ತುಂಬು_OPTIONAL,
				     &fw_ದತ್ತಾಂಶ, &fw_len);
	}

	ಆದರೆ (ret < 0)
		ಹಿಂತಿರುಗಿ ret;

	spl_image->ಗಾತ್ರ_2 = fw_len;
	spl_image->ತುಂಬು_addr = fw_ದತ್ತಾಂಶ;
	ಆದರೆ (fit_image_get_ನಮೂದು(ಶೀರ್ಷಿಕೆ, ret, &spl_image->ನಮೂದು_ಬಿಂದು))
		spl_image->ನಮೂದು_ಬಿಂದು = fw_ದತ್ತಾಂಶ;
	ಆದರೆ (fit_image_get_os(ಶೀರ್ಷಿಕೆ, ret, &spl_image->os))
		spl_image->os = IH_OS_INVALID;
	spl_image->name = genimg_get_os_name(spl_image->os);

	debug(PHASE_PROMPT "payload image: %32s load addr: 0x%lx size: %d\n",
	      spl_image->name, spl_image->ತುಂಬು_addr, spl_image->ಗಾತ್ರ_2);

	images.verify = CONFIG_IS_ENABLED(FIT_SIGNATURE);

	ret = fit_image_ತುಂಬು(&images, virt_ಗಮ್ಯ_phys((ಶೂನ್ಯ *)ಶೀರ್ಷಿಕೆ), ಖಾಲಿ,
			     &fit_uname_config, IH_ARCH_DEFAULT, IH_TYPE_FLATDT,
			     -1, FIT_ತುಂಬು_OPTIONAL, &dt_ದತ್ತಾಂಶ, &dt_len);
	ಆದರೆ (ret >= 0) {
		ಆದರೆ (spl_image->os == IH_OS_U_BOOT) {
			/* HACK: U-Boot expects FDT at a specific address */
			fdt_hack = ALIGN(spl_image->ತುಂಬು_addr + spl_image->ಗಾತ್ರ_2, 8);
			debug("Relocating FDT to %p\n", (ಶೂನ್ಯ *)fdt_hack);
			ಸ್ಮರಣೆ_ನಕಲು(map_sysmem(fdt_hack, dt_len),
			       map_sysmem(dt_ದತ್ತಾಂಶ, 0), dt_len);
			spl_image->fdt_addr = (ಶೂನ್ಯ *)fdt_hack;
		} ಇಲ್ಲದಿದ್ದರೆ {
			spl_image->fdt_addr = (ಶೂನ್ಯ *)dt_ದತ್ತಾಂಶ;
		}
	}

	conf_noffset = fit_conf_get_ಗಂಟು((ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *)ಶೀರ್ಷಿಕೆ,
					 fit_uname_config);
	ಆದರೆ (conf_noffset < 0)
		ಹಿಂತಿರುಗಿ 0;

	ಗಾಗಿ (idx = 0;
	     uname = fdt_stringlist_get((ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *)ಶೀರ್ಷಿಕೆ, conf_noffset,
					FIT_LOADABLE_PROP, idx,
				ಖಾಲಿ), uname;
	     idx++) {
		images.verify = CONFIG_IS_ENABLED(FIT_SIGNATURE);

		ret = fit_image_ತುಂಬು(&images, virt_ಗಮ್ಯ_phys((ಶೂನ್ಯ *)ಶೀರ್ಷಿಕೆ),
				     &uname, &fit_uname_config,
				     IH_ARCH_DEFAULT, IH_TYPE_LOADABLE, -1,
				     FIT_ತುಂಬು_OPTIONAL_NON_ZERO,
				     &img_ದತ್ತಾಂಶ, &img_len);
		ಆದರೆ (ret < 0)
			ಹಿಂತಿರುಗಿ ret;
	}
	spl_image->flags |= SPL_FIT_ಸಿಕ್ಕಿತು;

	upl_ಗಣ_fit_info(map_ಗಮ್ಯ_sysmem(ಶೀರ್ಷಿಕೆ), conf_noffset,
			 spl_image->ನಮೂದು_ಬಿಂದು);

	ಹಿಂತಿರುಗಿ 0;
}
