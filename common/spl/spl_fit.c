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

DECLARE_GLOBAL_ڈیٹا_PTR;

ساخت spl_fit_info {
	ثابت خالی *fit;	/* Pointer to a valid FIT blob */
	size_t ext_ڈیٹا_انحراف;	/* Offset to FIT external data (end of FIT) */
	صحیح images_گرہ;	/* FDT offset to "/images" node */
	صحیح conf_گرہ;		/* FDT offset to selected configuration node */
};

__weak ulong board_spl_fit_سائز_align(ulong سائز)
{
	واپس سائز;
}

ساکن صحیح ڈھونڈو_گرہ_from_desc(ثابت خالی *fit, صحیح گرہ, ثابت حرف *str)
{
	صحیح بچہ;

	اگر (گرہ < 0)
		واپس -EINVAL;

	/* iterate the FIT nodes and find a matching description */
	برائے (بچہ = fdt_پہلا_subnode(fit, گرہ); بچہ >= 0;
	     بچہ = fdt_اگلا_subnode(fit, بچہ)) {
		صحیح len;
		ثابت حرف *desc = fdt_getprop(fit, بچہ, FIT_DESC_PROP, &len);

		اگر (!desc)
			جاری;

		اگر (!strcmp(desc, str))
			واپس بچہ;
	}

	واپس -ENOENT;
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
ساکن صحیح spl_fit_get_image_name(ثابت ساخت spl_fit_info *ctx,
				  ثابت حرف *type, صحیح اشاریہ,
				  ثابت حرف **outname)
{
	ساخت udevice *sysinfo;
	ثابت حرف *name, *str, *انتہا_2;
	__maybe_unused صحیح گرہ;
	صحیح len, i;
	bool ملا = true;

	name = fdt_getprop(ctx->fit, ctx->conf_گرہ, type, &len);
	اگر (!name) {
		debug("cannot find property '%s': %d\n", type, len);
		واپس -EINVAL;
	}
	/* A string property should be NUL terminated */
	انتہا_2 = name + len - 1;
	اگر (!len || *انتہا_2) {
		debug("malformed property '%s'\n", type);
		واپس -EINVAL;
	}

	str = name;
	برائے (i = 0; i < اشاریہ; i++) {
		str = strchr(str, '\0') + 1;
		اگر (str > انتہا_2) {
			ملا = false;
			توڑو;
		}
	}

	اگر (!ملا && CONFIG_IS_ENABLED(SYSINFO) && !sysinfo_get(&sysinfo)) {
		صحیح rc;
		/*
		 * no string in the property for this index. Check if the
		 * sysinfo-level code can supply one.
		 */
		rc = sysinfo_detect(sysinfo);
		اگر (rc)
			واپس rc;

		rc = sysinfo_get_fit_loadable(sysinfo, اشاریہ - i - 1, type,
					      &str);
		اگر (rc && rc != -ENOENT)
			واپس rc;

		اگر (!rc) {
			/*
			 * The sysinfo provided a name for a loadable.
			 * Try to match it against the description properties
			 * first. If no matching node is found, use it as a
			 * node name.
			 */
			صحیح گرہ;
			صحیح images = fdt_راستہ_انحراف(ctx->fit, FIT_IMAGES_راستہ);

			گرہ = ڈھونڈو_گرہ_from_desc(ctx->fit, images, str);
			اگر (گرہ > 0)
				str = fdt_get_name(ctx->fit, گرہ, تہی);

			ملا = true;
		}
	}

	اگر (!ملا) {
		debug("no string for index %d\n", اشاریہ);
		واپس -E2BIG;
	}

	*outname = str;
	واپس 0;
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
ساکن صحیح spl_fit_get_image_گرہ(ثابت ساخت spl_fit_info *ctx,
				  ثابت حرف *type, صحیح اشاریہ)
{
	ثابت حرف *str;
	صحیح err;
	صحیح گرہ;

	err = spl_fit_get_image_name(ctx, type, اشاریہ, &str);
	اگر (err)
		واپس err;

	debug("%s: '%s'\n", type, str);

	گرہ = fdt_subnode_انحراف(ctx->fit, ctx->images_گرہ, str);
	اگر (گرہ < 0) {
		pr_err("cannot find image node '%s': %d\n", str, گرہ);
		واپس -EINVAL;
	}

	واپس گرہ;
}

ساکن u32 get_aligned_image_انحراف(ساخت spl_بھرو_info *info, u32 انحراف)
{
	واپس ALIGN_نیچے_کو(انحراف, spl_get_bl_len(info));
}

ساکن u32 get_aligned_image_overhead(ساخت spl_بھرو_info *info, u32 انحراف)
{
	واپس انحراف & (spl_get_bl_len(info) - 1);
}

ساکن صحیح get_aligned_image_سائز(ساخت spl_بھرو_info *info, ulong ڈیٹا_سائز,
				  u32 انحراف, ulong *aligned_سائز)
{
	u32 overhead = get_aligned_image_overhead(info, انحراف);

	اگر (ڈیٹا_سائز > ULONG_MAX - overhead)
		واپس -EOVERFLOW;
	ڈیٹا_سائز += overhead;

	اگر (ڈیٹا_سائز > ULONG_MAX - (spl_get_bl_len(info) - 1))
		واپس -EOVERFLOW;
	ڈیٹا_سائز = ALIGN(ڈیٹا_سائز, spl_get_bl_len(info));

	*aligned_سائز = ڈیٹا_سائز;

	واپس 0;
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
ساکن خالی fit_fill_image_info(ثابت خالی *fit, صحیح گرہ,
				ساخت spl_image_info *image_info,
				ulong بھرو_addr, ulong سائز)
{
	ulong اندراج_نقطہ;

	اگر (!image_info)
		واپس;

	image_info->بھرو_addr = بھرو_addr;
	image_info->سائز = سائز;

	اگر (!fit_image_get_اندراج(fit, گرہ, &اندراج_نقطہ))
		image_info->اندراج_نقطہ = اندراج_نقطہ;
	ورنہ
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
ساکن صحیح بھرو_simple_fit(ساخت spl_بھرو_info *info, ulong fit_انحراف,
			   ثابت ساخت spl_fit_info *ctx, صحیح گرہ,
			   ساخت spl_image_info *image_info, ulong max_سائز)
{
	u32 انحراف;
	u32 len;
	size_t لمبائی;
	ulong سائز;
	ulong بھرو_addr;
	خالی *بھرو_ptr;
	خالی *src;
	uint8_t image_comp = -1, type = -1;
	ثابت خالی *ڈیٹا;
	ثابت خالی *fit = ctx->fit;
	bool external_ڈیٹا = false;
	صحیح ret;

	log_debug("starting\n");
	اگر (CONFIG_IS_ENABLED(BOOTMETH_VBE) &&
	    xpl_get_phase(info) != IH_PHASE_NONE) {
		شمار image_phase_t phase;

		ret = fit_image_get_phase(fit, گرہ, &phase);
		/* if the image is for any phase, let's use it */
		اگر (ret == -ENOENT || phase == xpl_get_phase(info)) {
			log_debug("found\n");
		} ورنہ اگر (ret < 0) {
			log_debug("err=%d\n", ret);
			واپس ret;
		} ورنہ {
			log_debug("- phase mismatch, skipping this image\n");
			واپس -EBADSLT;
		}
	}

	اگر (IS_ENABLED(CONFIG_SPL_FPGA) ||
	    (IS_ENABLED(CONFIG_SPL_OS_BOOT) && spl_decompression_enabled())) {
		اگر (fit_image_get_type(fit, گرہ, &type))
			puts("Cannot get image type.\n");
		ورنہ
			debug("%s ", genimg_get_type_name(type));
	}

	اگر (spl_decompression_enabled()) {
		fit_image_get_comp(fit, گرہ, &image_comp);
		debug("%s ", genimg_get_comp_name(image_comp));
	}

	اگر (fit_image_get_بھرو(fit, گرہ, &بھرو_addr)) {
		اگر (!image_info->بھرو_addr) {
			لکھو("Can't load %s: No load address and no buffer\n",
			       fit_get_name(fit, گرہ, تہی));
			واپس -ENOBUFS;
		}
		بھرو_addr = image_info->بھرو_addr;
	}

	اگر (!fit_image_get_ڈیٹا_position(fit, گرہ, &انحراف)) {
		external_ڈیٹا = true;
	} ورنہ اگر (!fit_image_get_ڈیٹا_انحراف(fit, گرہ, &انحراف)) {
		/* The resulting offset cannot exceed UINT32_MAX */
		اگر (ctx->ext_ڈیٹا_انحراف > UINT32_MAX - انحراف) {
			log_debug("Invalid external data offset: %u\n", انحراف);
			واپس -EINVAL;
		}
		log_debug("read offset %x = offset from fit %x\n", انحراف,
			  (u32)(انحراف + ctx->ext_ڈیٹا_انحراف));
		انحراف += ctx->ext_ڈیٹا_انحراف;
		external_ڈیٹا = true;
	}

	اگر (external_ڈیٹا) {
		u32 aligned_انحراف;
		ulong read_انحراف;
		خالی *src_ptr;

		/* External data */
		اگر (fit_image_get_ڈیٹا_سائز(fit, گرہ, &len))
			واپس -ENOENT;

		/* Dont bother to copy 0 byte data, but warn, though */
		اگر (!len) {
			log_warning("%s: Skip load '%s': image size is 0!\n",
				    __func__, fit_get_name(fit, گرہ, تہی));
			fit_fill_image_info(fit, گرہ, image_info, بھرو_addr, 0);
			واپس 0;
		}

		/*
		 * data-size is excluded from the configuration signature (it
		 * is in exc_prop[] in image-fit-sig.c), so it stays attacker
		 * controlled even after fit_config_verify() succeeds. The
		 * image hash is only verified after the device read below, so
		 * an oversized value has to be rejected here.
		 */
		ret = get_aligned_image_سائز(info, len, انحراف, &سائز);
		اگر (ret) {
			log_debug("Invalid external data size: %u\n", len);
			واپس ret;
		}

		اگر (سائز > max_سائز) {
			log_debug("Image too large: aligned size %lu, max %lu (data-size %u)\n",
				  سائز, max_سائز, len);
			واپس -EFBIG;
		}

		aligned_انحراف = get_aligned_image_انحراف(info, انحراف);
		اگر (aligned_انحراف > ULONG_MAX - fit_انحراف) {
			log_debug("Invalid aligned external data offset: %u\n",
				  aligned_انحراف);
			واپس -EINVAL;
		}
		read_انحراف = fit_انحراف + aligned_انحراف;

		اگر (spl_decompression_enabled() &&
		    (image_comp == IH_COMP_GZIP || image_comp == IH_COMP_LZMA))
			src_ptr = map_sysmem(ALIGN(CONFIG_SYS_بھرو_ADDR, ARCH_DMA_MINALIGN), len);
		ورنہ
			src_ptr = map_sysmem(ALIGN(بھرو_addr, ARCH_DMA_MINALIGN), len);
		لمبائی = len;

		/*
		 * For non-signed FIT images, we can check that
		 * (read_offset + size) does not wrap and that
		 * (src_ptr + size) does not exceed the addressable range.
		 * For signed FITs, we can additionally check that
		 * (offset + len) doesn't exceed the allowed FIT image
		 * maximum size.
		 */
		اگر (سائز > ULONG_MAX - read_انحراف ||
		    سائز > UINTPTR_MAX - (uintptr_t)src_ptr
		/*
		 * #if (not a runtime if) is required: FIT_SIGNATURE_MAX_SIZE
		 * depends on FIT_SIGNATURE, so CONFIG_VAL(FIT_SIGNATURE_MAX_SIZE)
		 * is undefined when signing is disabled and referencing it
		 * here would fail to compile.
		 */
#اگر CONFIG_IS_ENABLED(FIT_SIGNATURE)
		    || انحراف > CONFIG_VAL(FIT_SIGNATURE_MAX_سائز) ||
		    len > CONFIG_VAL(FIT_SIGNATURE_MAX_سائز) - انحراف
#اگر_ختم
		) {
			log_debug("FIT external data is out of bounds (offset=%u, size=%u)\n",
				  انحراف, len);
			واپس -EINVAL;
		}

		log_debug("reading from offset %x / %lx size %lx to %p: ",
			  انحراف, read_انحراف, سائز, src_ptr);

		اگر (info->اقرأ(info, read_انحراف, سائز, src_ptr) < لمبائی)
			واپس -EIO;

		debug("External data: dst=%p, offset=%x, size=%lx\n",
		      src_ptr, انحراف, (غیرعلامتی طویل)لمبائی);
		src = src_ptr + get_aligned_image_overhead(info, انحراف);
	} ورنہ {
		/* Embedded data */
		اگر (fit_image_get_emb_ڈیٹا(fit, گرہ, &ڈیٹا, &لمبائی)) {
			puts("Cannot get image data/size\n");
			واپس -ENOENT;
		}
		debug("Embedded data: dst=%lx, size=%lx\n", بھرو_addr,
		      (غیرعلامتی طویل)لمبائی);
		src = (خالی *)ڈیٹا;	/* cast away const */
	}

	اگر (CONFIG_IS_ENABLED(FIT_SIGNATURE)) {
		لکھو("## Checking hash(es) for Image %s ... ",
		       fit_get_name(fit, گرہ, تہی));
		اگر (!fit_image_verify_with_ڈیٹا(fit, گرہ, gd_fdt_blob(), src,
						لمبائی))
			واپس -EPERM;
		puts("OK\n");
	}

	اگر (CONFIG_IS_ENABLED(FIT_IMAGE_POST_PROCESS))
		board_fit_image_post_process(fit, گرہ, &src, &لمبائی);

	بھرو_ptr = map_sysmem(بھرو_addr, لمبائی);
	اگر (IS_ENABLED(CONFIG_SPL_GZIP) && image_comp == IH_COMP_GZIP) {
		سائز = لمبائی;
		اگر (gunzip(بھرو_ptr, CONFIG_SYS_BOOTM_LEN, src, &سائز)) {
			puts("Uncompressing error\n");
			واپس -EIO;
		}
		لمبائی = سائز;
	} ورنہ اگر (IS_ENABLED(CONFIG_SPL_LZMA) && image_comp == IH_COMP_LZMA) {
		سائز = CONFIG_SYS_BOOTM_LEN;
		ulong بھرو_انتہا;

		اگر (image_decomp(IH_COMP_LZMA, CONFIG_SYS_بھرو_ADDR, 0, 0,
				 بھرو_ptr, src, لمبائی, سائز, &بھرو_انتہا)) {
			puts("Uncompressing error\n");
			واپس -EIO;
		}
		لمبائی = بھرو_انتہا - CONFIG_SYS_بھرو_ADDR;
	} ورنہ {
		memmove(بھرو_ptr, src, لمبائی);
	}

	fit_fill_image_info(fit, گرہ, image_info, بھرو_addr, لمبائی);
	log_debug("- done loading\n");

	upl_جوڑو_image(fit, گرہ, بھرو_addr, لمبائی);

	واپس 0;
}

ساکن bool os_takes_devicetree(uint8_t os)
{
	انتخاب (os) {
	حالت IH_OS_U_BOOT:
		واپس true;
	حالت IH_OS_LINUX:
		واپس IS_ENABLED(CONFIG_SPL_OS_BOOT) ||
		       IS_ENABLED(CONFIG_SPL_OPENSBI);
	طےشدہ:
		واپس false;
	}
}

__weak صحیح board_spl_fit_append_fdt_skip(ثابت حرف *name)
{
	واپس 0;	/* Do not skip */
}

ساکن صحیح spl_fit_append_fdt(ساخت spl_image_info *spl_image,
			      ساخت spl_بھرو_info *info, ulong انحراف,
			      ثابت ساخت spl_fit_info *ctx)
{
	ساخت spl_image_info image_info;
	صحیح گرہ, ret = 0, اشاریہ = 0;

	/*
	 * Use the address following the image as target address for the
	 * device tree.
	 */
	image_info.بھرو_addr = ALIGN(spl_image->بھرو_addr + spl_image->سائز, 8);

	/* Figure out which device tree the board wants to use */
	گرہ = spl_fit_get_image_گرہ(ctx, FIT_FDT_PROP, اشاریہ++);
	اگر (گرہ < 0) {
		size_t سائز;

		debug("%s: cannot find FDT node\n", __func__);

		/*
		 * U-Boot did not find a device tree inside the FIT image. Use
		 * the U-Boot device tree instead.
		 */
		اگر (!gd->fdt_blob)
			واپس گرہ;

		/*
		 * Make the load-address of the FDT available for the SPL
		 * framework
		 */
		سائز = fdt_totalsize(gd->fdt_blob);
		spl_image->fdt_addr = map_sysmem(image_info.بھرو_addr, سائز);
		حافظہ_نقل(spl_image->fdt_addr, gd->fdt_blob, سائز);
	} ورنہ {
		ret = بھرو_simple_fit(info, انحراف, ctx, گرہ, &image_info,
				      CONFIG_SYS_BOOTM_LEN);
		اگر (ret < 0)
			واپس ret;

		spl_image->fdt_addr = phys_مقصد_virt(image_info.بھرو_addr);
	}

	اگر (CONFIG_IS_ENABLED(FIT_IMAGE_TINY))
		واپس 0;

#اگر CONFIG_IS_ENABLED(بھرو_FIT_APPLY_OVERLAY)
		خالی *tmpbuffer = تہی;

		برائے (; ; اشاریہ++) {
			ثابت حرف *str;

			ret = spl_fit_get_image_name(ctx, FIT_FDT_PROP, اشاریہ, &str);
			اگر (ret == -E2BIG) {
				debug("%s: No additional FDT node\n", __func__);
				ret = 0;
				توڑو;
			} ورنہ اگر (ret) {
				توڑو;
			}

			ret = board_spl_fit_append_fdt_skip(str);
			اگر (ret)
				جاری;

			گرہ = fdt_subnode_انحراف(ctx->fit, ctx->images_گرہ, str);
			اگر (گرہ < 0) {
				debug("%s: unable to find FDT node %d\n",
				      __func__, اشاریہ);
				جاری;
			}

			اگر (!tmpbuffer) {
				/*
				 * allocate memory to store the DT overlay
				 * before it is applied. It may not be used
				 * depending on how the overlay is stored, so
				 * don't fail yet if the allocation failed.
				 */
				size_t سائز = CONFIG_SPL_بھرو_FIT_APPLY_OVERLAY_BUF_SZ;

				tmpbuffer = malloc_cache_aligned(سائز);
				اگر (!tmpbuffer)
					debug("%s: unable to allocate space for overlays\n",
					      __func__);
			}
			image_info.بھرو_addr = (ulong)tmpbuffer;
			ret = بھرو_simple_fit(info, انحراف, ctx, گرہ,
					      &image_info,
					      CONFIG_SPL_بھرو_FIT_APPLY_OVERLAY_BUF_SZ);
			اگر (ret == -EBADSLT)
				جاری;
			ورنہ اگر (ret < 0)
				توڑو;

			/* Make room in FDT for changes from the overlay */
			ret = fdt_increase_سائز(spl_image->fdt_addr,
						image_info.سائز);
			اگر (ret < 0)
				توڑو;

			ret = fdt_overlay_apply_verbose(spl_image->fdt_addr,
							(خالی *)image_info.بھرو_addr);
			اگر (ret) {
				pr_err("failed to apply DT overlay %s\n",
				       fit_get_name(ctx->fit, گرہ, تہی));
				توڑو;
			}

			debug("%s: DT overlay %s applied\n", __func__,
			      fit_get_name(ctx->fit, گرہ, تہی));
		}
		آزاد(tmpbuffer);
		اگر (ret)
			واپس ret;
#اگر_ختم
	/* Try to make space, so we can inject details on the loadables */
	ret = fdt_shrink_مقصد_minimum(spl_image->fdt_addr, 8192);
	اگر (ret < 0)
		واپس ret;

	واپس ret;
}

ساکن صحیح spl_fit_ریکارڈ_loadable(ثابت ساخت spl_fit_info *ctx, صحیح اشاریہ,
				   خالی *blob, ساخت spl_image_info *image)
{
	صحیح ret = 0;
	ثابت حرف *name;
	صحیح گرہ;

	ret = spl_fit_get_image_name(ctx, FIT_LOADABLE_PROP, اشاریہ, &name);
	اگر (ret < 0)
		واپس ret;

	گرہ = spl_fit_get_image_گرہ(ctx, FIT_LOADABLE_PROP, اشاریہ);

	ret = fdt_ریکارڈ_loadable(blob, اشاریہ, name, image->بھرو_addr,
			image->سائز, image->اندراج_نقطہ,
			fdt_getprop(ctx->fit, گرہ, FIT_TYPE_PROP, تہی),
			fdt_getprop(ctx->fit, گرہ, FIT_OS_PROP, تہی),
			fdt_getprop(ctx->fit, گرہ, FIT_ARCH_PROP, تہی));

	واپس ret;
}

ساکن صحیح spl_fit_image_is_fpga(ثابت خالی *fit, صحیح گرہ)
{
	ثابت حرف *type;

	اگر (!IS_ENABLED(CONFIG_SPL_FPGA))
		واپس 0;

	type = fdt_getprop(fit, گرہ, FIT_TYPE_PROP, تہی);
	اگر (!type)
		واپس 0;

	واپس !strcmp(type, "fpga");
}

ساکن خالی spl_fit_image_ریکارڈ_arm32_optee(ثابت خالی *fit, صحیح گرہ,
					     ساخت spl_image_info *spl_image,
					     ساخت spl_image_info *image_info)
{
#اگر defined(CONFIG_BOOTM_OPTEE) && defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
	ثابت حرف *type = fdt_getprop(fit, گرہ, FIT_TYPE_PROP, تہی);

	اگر (!type)
		واپس;

	اگر (strcmp(type, "tee"))
		واپس;

	spl_image->optee_addr = image_info->بھرو_addr;
#اگر_ختم
}

ساکن صحیح spl_fit_image_get_os(ثابت خالی *fit, صحیح noffset, uint8_t *os)
{
	اگر (!CONFIG_IS_ENABLED(FIT_IMAGE_TINY) || CONFIG_IS_ENABLED(OS_BOOT))
		واپس fit_image_get_os(fit, noffset, os);

	ثابت حرف *name = fdt_getprop(fit, noffset, FIT_OS_PROP, تہی);
	اگر (!name)
		واپس -ENOENT;

	/*
	 * We don't care what the type of the image actually is,
	 * only whether or not it is U-Boot. This saves some
	 * space by omitting the large table of OS types.
	 */
	اگر (!strcmp(name, "u-boot"))
		*os = IH_OS_U_BOOT;
	ورنہ
		*os = IH_OS_INVALID;

	واپس 0;
}

/*
 * The purpose of the FIT load buffer is to provide a memory location that is
 * independent of the load address of any FIT component.
 */
__weak خالی *board_spl_fit_بفر_addr(ulong fit_سائز, صحیح sectors, صحیح bl_len)
{
	خالی *buf;
	size_t سائز = sectors * bl_len;

	buf = malloc_cache_aligned(سائز);
	اگر (!buf) {
		pr_err("Could not get FIT buffer of %lu bytes\n", (ulong)سائز);

		اگر (IS_ENABLED(CONFIG_SPL_SYS_MALLOC))
			pr_err("\tcheck CONFIG_SPL_SYS_MALLOC_SIZE\n");
		ورنہ
			pr_err("\tcheck CONFIG_SPL_SYS_MALLOC_F_LEN\n");

		buf = spl_get_بھرو_بفر(0, سائز);
	}
	واپس buf;
}

/*
 * Weak default function to allow customizing SPL fit loading for load-only
 * use cases by allowing to skip the parsing/processing of the FIT contents
 * (so that this can be done separately in a more customized fashion)
 */
__weak bool spl_بھرو_simple_fit_skip_processing(خالی)
{
	واپس false;
}

/*
 * Weak default function to allow fixes after fit header
 * is loaded.
 */
__weak خالی *spl_بھرو_simple_fit_fix_بھرو(ثابت خالی *fit)
{
	واپس (خالی *)fit;
}

ساکن خالی warn_deprecated(ثابت حرف *پیغام)
{
	لکھو("DEPRECATED: %s\n", پیغام);
	لکھو("\tSee https://fitspec.osfw.foundation/\n");
}

ساکن صحیح spl_fit_upload_fpga(ساخت spl_fit_info *ctx, صحیح گرہ,
			       ساخت spl_image_info *fpga_image)
{
	ثابت حرف *compatible;
	صحیح ret;
	صحیح devnum = 0;
	صحیح flags = 0;

	debug("FPGA bitstream at: %x, size: %x\n",
	      (u32)fpga_image->بھرو_addr, fpga_image->سائز);

	compatible = fdt_getprop(ctx->fit, گرہ, FIT_COMPAT_PROP, تہی);
	اگر (!compatible) {
		warn_deprecated("'fpga' image without 'compatible' property");
	} ورنہ {
		اگر (CONFIG_IS_ENABLED(FPGA_بھرو_SECURE))
			flags = fpga_compatible2flag(devnum, compatible);
		اگر (strcmp(compatible, "u-boot,fpga-legacy"))
			debug("Ignoring compatible = %s property\n",
			      compatible);
	}

	ret = fpga_بھرو(devnum, (خالی *)fpga_image->بھرو_addr,
			fpga_image->سائز, ثنائی_ہندسہ_بھرا, flags);
	اگر (ret) {
		لکھو("%s: Cannot load the image to the FPGA\n", __func__);
		واپس ret;
	}

	puts("FPGA image loaded from FIT\n");

	واپس 0;
}

ساکن صحیح spl_fit_بھرو_fpga(ساخت spl_fit_info *ctx,
			     ساخت spl_بھرو_info *info, ulong انحراف)
{
	صحیح گرہ, ret;

	ساخت spl_image_info fpga_image = {
		.بھرو_addr = 0,
	};

	گرہ = spl_fit_get_image_گرہ(ctx, FIT_FPGA_PROP, 0);
	اگر (گرہ < 0)
		واپس گرہ;

	warn_deprecated("'fpga' property in config node. Use 'loadables'");

	/* Load the image and set up the fpga_image structure */
	ret = بھرو_simple_fit(info, انحراف, ctx, گرہ, &fpga_image,
			      CONFIG_SYS_BOOTM_LEN);
	اگر (ret) {
		لکھو("%s: Cannot load the FPGA: %i\n", __func__, ret);
		واپس ret;
	}

	واپس spl_fit_upload_fpga(ctx, گرہ, &fpga_image);
}

ساکن صحیح spl_simple_fit_read(ساخت spl_fit_info *ctx,
			       ساخت spl_بھرو_info *info, ulong انحراف,
			       ثابت خالی *fit_سرخی)
{
	غیرعلامتی طویل aligned_سائز, گنتی, سائز;
	خالی *buf;
	صحیح ret;

	/*
	 * For FIT with external data, figure out where the external images
	 * start. This is the base for the data-offset properties in each
	 * image.
	 */
	سائز = ALIGN(fdt_totalsize(fit_سرخی), 4);
	سائز = board_spl_fit_سائز_align(سائز);
	ctx->ext_ڈیٹا_انحراف = ALIGN(سائز, 4);

	/*
	 * So far we only have one block of data from the FIT. Read the entire
	 * thing, including that first block.
	 *
	 * For FIT with data embedded, data is loaded as part of FIT image.
	 * For FIT with external data, data is not loaded in this step.
	 */
	ret = get_aligned_image_سائز(info, سائز, 0, &aligned_سائز);
	اگر (ret) {
		log_debug("Invalid FIT size: %lu\n", سائز);
		واپس ret;
	}
	buf = board_spl_fit_بفر_addr(aligned_سائز, aligned_سائز, 1);
	اگر (!buf) {
		/*
		 * We assume that none of the board will ever use 0x0 as a
		 * valid load address. Theoretically some board could use it,
		 * but this is extremely unlikely.
		 */
		واپس -EIO;
	}

	گنتی = info->اقرأ(info, انحراف, aligned_سائز, buf);
	اگر (!گنتی) {
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
		واپس -EIO;
	}

	ctx->fit = buf;
	debug("fit read offset %lx, size=%lu, dst=%p, count=%lu\n",
	      انحراف, aligned_سائز, buf, گنتی);

	واپس 0;
}

ساکن صحیح spl_simple_fit_parse(ساخت spl_fit_info *ctx)
{
	/* Find the correct subnode under "/configurations" */
	ctx->conf_گرہ = fit_ڈھونڈو_config_گرہ(ctx->fit);
	اگر (ctx->conf_گرہ < 0)
		واپس -EINVAL;

	اگر (CONFIG_IS_ENABLED(FIT_SIGNATURE)) {
		لکھو("## Checking hash(es) for config %s ... ",
		       fit_get_name(ctx->fit, ctx->conf_گرہ, تہی));
		اگر (fit_config_verify(ctx->fit, ctx->conf_گرہ))
			واپس -EPERM;
		puts("OK\n");
	}

	/* find the node holding the images information */
	ctx->images_گرہ = fdt_راستہ_انحراف(ctx->fit, FIT_IMAGES_راستہ);
	اگر (ctx->images_گرہ < 0) {
		debug("%s: Cannot find /images node: %d\n", __func__,
		      ctx->images_گرہ);
		واپس -EINVAL;
	}

	واپس 0;
}

صحیح spl_بھرو_simple_fit(ساخت spl_image_info *spl_image,
			ساخت spl_بھرو_info *info, ulong انحراف, خالی *fit)
{
	ساخت spl_image_info image_info;
	ساخت spl_fit_info ctx;
	صحیح گرہ = -1;
	صحیح ret;
	صحیح اشاریہ = 0;
	صحیح firmware_گرہ;

	ret = spl_simple_fit_read(&ctx, info, انحراف, fit);
	اگر (ret < 0)
		واپس ret;

	/* skip further processing if requested to enable load-only use cases */
	اگر (spl_بھرو_simple_fit_skip_processing())
		واپس 0;

	ctx.fit = spl_بھرو_simple_fit_fix_بھرو(ctx.fit);

	ret = spl_simple_fit_parse(&ctx);
	اگر (ret < 0)
		واپس ret;

	اگر (IS_ENABLED(CONFIG_SPL_FPGA))
		spl_fit_بھرو_fpga(&ctx, info, انحراف);

	/*
	 * Find the U-Boot image using the following search order:
	 *   - start at 'firmware' (e.g. an ARM Trusted Firmware)
	 *   - fall back 'kernel' (e.g. a Falcon-mode OS boot
	 *   - fall back to using the first 'loadables' entry
	 */
	اگر (گرہ < 0)
		گرہ = spl_fit_get_image_گرہ(&ctx, FIT_FIRMWARE_PROP, 0);

	اگر (گرہ < 0 && IS_ENABLED(CONFIG_SPL_OS_BOOT))
		گرہ = spl_fit_get_image_گرہ(&ctx, FIT_KERNEL_PROP, 0);

	اگر (گرہ < 0) {
		debug("could not find firmware image, trying loadables...\n");
		گرہ = spl_fit_get_image_گرہ(&ctx, FIT_LOADABLE_PROP, 0);
		/*
		 * If we pick the U-Boot image from "loadables", start at
		 * the second image when later loading additional images.
		 */
		اشاریہ = 1;
	}
	اگر (گرہ < 0) {
		debug("%s: Cannot find u-boot image node: %d\n",
		      __func__, گرہ);
		واپس -1;
	}

	/* Load the image and set up the spl_image structure */
	ret = بھرو_simple_fit(info, انحراف, &ctx, گرہ, spl_image,
			      CONFIG_SYS_BOOTM_LEN);
	اگر (ret)
		واپس ret;

	/*
	 * For backward compatibility, we treat the first node that is
	 * as a U-Boot image, if no OS-type has been declared.
	 */
	اگر (!spl_fit_image_get_os(ctx.fit, گرہ, &spl_image->os))
		debug("Image OS is %s\n", genimg_get_os_name(spl_image->os));
	ورنہ اگر (!IS_ENABLED(CONFIG_SPL_OS_BOOT))
		spl_image->os = IH_OS_U_BOOT;

	/*
	 * Booting a next-stage U-Boot may require us to append the FDT.
	 * We allow this to fail, as the U-Boot image might embed its FDT.
	 */
	اگر (os_takes_devicetree(spl_image->os)) {
		ret = spl_fit_append_fdt(spl_image, info, انحراف, &ctx);
		اگر (ret < 0 && spl_image->os != IH_OS_U_BOOT)
			واپس ret;
	}

	firmware_گرہ = گرہ;
	/* Now check if there are more images for us to load */
	برائے (; ; اشاریہ++) {
		uint8_t os_type = IH_OS_INVALID;

		گرہ = spl_fit_get_image_گرہ(&ctx, FIT_LOADABLE_PROP, اشاریہ);
		اگر (گرہ < 0)
			توڑو;

		/*
		 * if the firmware is also a loadable, skip it because
		 * it already has been loaded. This is typically the case with
		 * u-boot.img generated by mkimage.
		 */
		اگر (firmware_گرہ == گرہ)
			جاری;

		image_info.بھرو_addr = 0;
		ret = بھرو_simple_fit(info, انحراف, &ctx, گرہ, &image_info,
				      CONFIG_SYS_BOOTM_LEN);
		اگر (ret < 0 && ret != -EBADSLT) {
			لکھو("%s: can't load image loadables index %d (ret = %d)\n",
			       __func__, اشاریہ, ret);
			واپس ret;
		}

		اگر (spl_fit_image_is_fpga(ctx.fit, گرہ))
			spl_fit_upload_fpga(&ctx, گرہ, &image_info);

		اگر (!spl_fit_image_get_os(ctx.fit, گرہ, &os_type))
			debug("Loadable is %s\n", genimg_get_os_name(os_type));

		اگر (os_takes_devicetree(os_type)) {
			spl_fit_append_fdt(&image_info, info, انحراف, &ctx);
			spl_image->fdt_addr = image_info.fdt_addr;
		}

		/*
		 * If the "firmware" image did not provide an entry point,
		 * use the first valid entry point from the loadables.
		 */
		اگر (spl_image->اندراج_نقطہ == FDT_ERROR &&
		    image_info.اندراج_نقطہ != FDT_ERROR)
			spl_image->اندراج_نقطہ = image_info.اندراج_نقطہ;

		spl_fit_image_ریکارڈ_arm32_optee(ctx.fit, گرہ, spl_image,
						 &image_info);

		/* Record our loadables into the FDT */
		اگر (!CONFIG_IS_ENABLED(FIT_IMAGE_TINY) &&
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
	اگر (spl_image->اندراج_نقطہ == FDT_ERROR || spl_image->اندراج_نقطہ == 0)
		spl_image->اندراج_نقطہ = spl_image->بھرو_addr;

	spl_image->flags |= SPL_FIT_ملا;
	upl_سیٹ_fit_info(map_مقصد_sysmem(ctx.fit), ctx.conf_گرہ,
			 spl_image->اندراج_نقطہ);

	واپس 0;
}

/* Parse and load full fitImage in SPL */
صحیح spl_بھرو_fit_image(ساخت spl_image_info *spl_image,
		       ثابت ساخت legacy_img_hdr *سرخی)
{
	ساخت bootm_سرخیاں images;
	ثابت حرف *fit_uname_config = تہی;
	ulong fdt_hack;
	ثابت حرف *uname;
	ulong fw_ڈیٹا = 0, dt_ڈیٹا = 0, img_ڈیٹا = 0;
	ulong fw_len = 0, dt_len = 0, img_len = 0;
	صحیح idx, conf_noffset;
	صحیح ret;

	images.verify = CONFIG_IS_ENABLED(FIT_SIGNATURE);

	ret = fit_image_بھرو(&images, virt_مقصد_phys((خالی *)سرخی),
			     تہی, &fit_uname_config, IH_ARCH_DEFAULT,
			     IH_TYPE_FIRMWARE, -1, FIT_بھرو_OPTIONAL,
			     &fw_ڈیٹا, &fw_len);

	اگر (ret < 0) {
		ret = fit_image_بھرو(&images, virt_مقصد_phys((خالی *)سرخی),
				     تہی, &fit_uname_config, IH_ARCH_DEFAULT,
				     IH_TYPE_KERNEL, -1, FIT_بھرو_OPTIONAL,
				     &fw_ڈیٹا, &fw_len);
	}

	اگر (ret < 0)
		واپس ret;

	spl_image->سائز = fw_len;
	spl_image->بھرو_addr = fw_ڈیٹا;
	اگر (fit_image_get_اندراج(سرخی, ret, &spl_image->اندراج_نقطہ))
		spl_image->اندراج_نقطہ = fw_ڈیٹا;
	اگر (fit_image_get_os(سرخی, ret, &spl_image->os))
		spl_image->os = IH_OS_INVALID;
	spl_image->name = genimg_get_os_name(spl_image->os);

	debug(PHASE_PROMPT "payload image: %32s load addr: 0x%lx size: %d\n",
	      spl_image->name, spl_image->بھرو_addr, spl_image->سائز);

	images.verify = CONFIG_IS_ENABLED(FIT_SIGNATURE);

	ret = fit_image_بھرو(&images, virt_مقصد_phys((خالی *)سرخی), تہی,
			     &fit_uname_config, IH_ARCH_DEFAULT, IH_TYPE_FLATDT,
			     -1, FIT_بھرو_OPTIONAL, &dt_ڈیٹا, &dt_len);
	اگر (ret >= 0) {
		اگر (spl_image->os == IH_OS_U_BOOT) {
			/* HACK: U-Boot expects FDT at a specific address */
			fdt_hack = ALIGN(spl_image->بھرو_addr + spl_image->سائز, 8);
			debug("Relocating FDT to %p\n", (خالی *)fdt_hack);
			حافظہ_نقل(map_sysmem(fdt_hack, dt_len),
			       map_sysmem(dt_ڈیٹا, 0), dt_len);
			spl_image->fdt_addr = (خالی *)fdt_hack;
		} ورنہ {
			spl_image->fdt_addr = (خالی *)dt_ڈیٹا;
		}
	}

	conf_noffset = fit_conf_get_گرہ((ثابت خالی *)سرخی,
					 fit_uname_config);
	اگر (conf_noffset < 0)
		واپس 0;

	برائے (idx = 0;
	     uname = fdt_stringlist_get((ثابت خالی *)سرخی, conf_noffset,
					FIT_LOADABLE_PROP, idx,
				تہی), uname;
	     idx++) {
		images.verify = CONFIG_IS_ENABLED(FIT_SIGNATURE);

		ret = fit_image_بھرو(&images, virt_مقصد_phys((خالی *)سرخی),
				     &uname, &fit_uname_config,
				     IH_ARCH_DEFAULT, IH_TYPE_LOADABLE, -1,
				     FIT_بھرو_OPTIONAL_NON_ZERO,
				     &img_ڈیٹا, &img_len);
		اگر (ret < 0)
			واپس ret;
	}
	spl_image->flags |= SPL_FIT_ملا;

	upl_سیٹ_fit_info(map_مقصد_sysmem(سرخی), conf_noffset,
			 spl_image->اندراج_نقطہ);

	واپس 0;
}
