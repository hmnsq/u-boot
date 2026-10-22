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

DECLARE_GLOBAL_दत्त_PTR;

संरचना spl_fit_info {
	ध्रुव शून्य *fit;	/* Pointer to a valid FIT blob */
	size_t ext_दत्त_विस्थापनम्;	/* Offset to FIT external data (end of FIT) */
	पूर्णाङ्क images_ग्रन्थिः;	/* FDT offset to "/images" node */
	पूर्णाङ्क conf_ग्रन्थिः;		/* FDT offset to selected configuration node */
};

__weak ulong board_spl_fit_आकार_align(ulong आकार)
{
	प्रत्यागम आकार;
}

स्थिर पूर्णाङ्क find_ग्रन्थिः_from_desc(ध्रुव शून्य *fit, पूर्णाङ्क ग्रन्थिः, ध्रुव वर्ण *str)
{
	पूर्णाङ्क child;

	यदि (ग्रन्थिः < 0)
		प्रत्यागम -EINVAL;

	/* iterate the FIT nodes and find a matching description */
	प्रति (child = fdt_first_subnode(fit, ग्रन्थिः); child >= 0;
	     child = fdt_अग्र_subnode(fit, child)) {
		पूर्णाङ्क len;
		ध्रुव वर्ण *desc = fdt_getprop(fit, child, FIT_DESC_PROP, &len);

		यदि (!desc)
			अनुवर्त;

		यदि (!वाक्यं_तुलय(desc, str))
			प्रत्यागम child;
	}

	प्रत्यागम -ENOENT;
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
स्थिर पूर्णाङ्क spl_fit_get_image_name(ध्रुव संरचना spl_fit_info *ctx,
				  ध्रुव वर्ण *type, पूर्णाङ्क सूचकांक,
				  ध्रुव वर्ण **outname)
{
	संरचना udevice *sysinfo;
	ध्रुव वर्ण *name, *str, *अन्तः_2;
	__maybe_unused पूर्णाङ्क ग्रन्थिः;
	पूर्णाङ्क len, i;
	bool प्राप्तम् = true;

	name = fdt_getprop(ctx->fit, ctx->conf_ग्रन्थिः, type, &len);
	यदि (!name) {
		debug("cannot find property '%s': %d\n", type, len);
		प्रत्यागम -EINVAL;
	}
	/* A string property should be NUL terminated */
	अन्तः_2 = name + len - 1;
	यदि (!len || *अन्तः_2) {
		debug("malformed property '%s'\n", type);
		प्रत्यागम -EINVAL;
	}

	str = name;
	प्रति (i = 0; i < सूचकांक; i++) {
		str = strchr(str, '\0') + 1;
		यदि (str > अन्तः_2) {
			प्राप्तम् = false;
			भङ्ग;
		}
	}

	यदि (!प्राप्तम् && CONFIG_IS_ENABLED(SYSINFO) && !sysinfo_get(&sysinfo)) {
		पूर्णाङ्क rc;
		/*
		 * no string in the property for this index. Check if the
		 * sysinfo-level code can supply one.
		 */
		rc = sysinfo_detect(sysinfo);
		यदि (rc)
			प्रत्यागम rc;

		rc = sysinfo_get_fit_loadable(sysinfo, सूचकांक - i - 1, type,
					      &str);
		यदि (rc && rc != -ENOENT)
			प्रत्यागम rc;

		यदि (!rc) {
			/*
			 * The sysinfo provided a name for a loadable.
			 * Try to match it against the description properties
			 * first. If no matching node is found, use it as a
			 * node name.
			 */
			पूर्णाङ्क ग्रन्थिः;
			पूर्णाङ्क images = fdt_मार्गः_विस्थापनम्(ctx->fit, FIT_IMAGES_मार्गः);

			ग्रन्थिः = find_ग्रन्थिः_from_desc(ctx->fit, images, str);
			यदि (ग्रन्थिः > 0)
				str = fdt_get_name(ctx->fit, ग्रन्थिः, रिक्त);

			प्राप्तम् = true;
		}
	}

	यदि (!प्राप्तम्) {
		debug("no string for index %d\n", सूचकांक);
		प्रत्यागम -E2BIG;
	}

	*outname = str;
	प्रत्यागम 0;
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
स्थिर पूर्णाङ्क spl_fit_get_image_ग्रन्थिः(ध्रुव संरचना spl_fit_info *ctx,
				  ध्रुव वर्ण *type, पूर्णाङ्क सूचकांक)
{
	ध्रुव वर्ण *str;
	पूर्णाङ्क err;
	पूर्णाङ्क ग्रन्थिः;

	err = spl_fit_get_image_name(ctx, type, सूचकांक, &str);
	यदि (err)
		प्रत्यागम err;

	debug("%s: '%s'\n", type, str);

	ग्रन्थिः = fdt_subnode_विस्थापनम्(ctx->fit, ctx->images_ग्रन्थिः, str);
	यदि (ग्रन्थिः < 0) {
		pr_err("cannot find image node '%s': %d\n", str, ग्रन्थिः);
		प्रत्यागम -EINVAL;
	}

	प्रत्यागम ग्रन्थिः;
}

स्थिर u32 get_aligned_image_विस्थापनम्(संरचना spl_आरोपय_info *info, u32 विस्थापनम्)
{
	प्रत्यागम ALIGN_अधस्तात्(विस्थापनम्, spl_get_bl_len(info));
}

स्थिर u32 get_aligned_image_overhead(संरचना spl_आरोपय_info *info, u32 विस्थापनम्)
{
	प्रत्यागम विस्थापनम् & (spl_get_bl_len(info) - 1);
}

स्थिर पूर्णाङ्क get_aligned_image_आकार(संरचना spl_आरोपय_info *info, ulong दत्त_आकार,
				  u32 विस्थापनम्, ulong *aligned_आकार)
{
	u32 overhead = get_aligned_image_overhead(info, विस्थापनम्);

	यदि (दत्त_आकार > ULONG_MAX - overhead)
		प्रत्यागम -EOVERFLOW;
	दत्त_आकार += overhead;

	यदि (दत्त_आकार > ULONG_MAX - (spl_get_bl_len(info) - 1))
		प्रत्यागम -EOVERFLOW;
	दत्त_आकार = ALIGN(दत्त_आकार, spl_get_bl_len(info));

	*aligned_आकार = दत्त_आकार;

	प्रत्यागम 0;
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
स्थिर शून्य fit_fill_image_info(ध्रुव शून्य *fit, पूर्णाङ्क ग्रन्थिः,
				संरचना spl_image_info *image_info,
				ulong आरोपय_addr, ulong आकार)
{
	ulong प्रविष्टिः_point;

	यदि (!image_info)
		प्रत्यागम;

	image_info->आरोपय_addr = आरोपय_addr;
	image_info->आकार = आकार;

	यदि (!fit_image_get_प्रविष्टिः(fit, ग्रन्थिः, &प्रविष्टिः_point))
		image_info->प्रविष्टिः_point = प्रविष्टिः_point;
	अन्यथा
		image_info->प्रविष्टिः_point = FDT_ERROR;
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
स्थिर पूर्णाङ्क आरोपय_simple_fit(संरचना spl_आरोपय_info *info, ulong fit_विस्थापनम्,
			   ध्रुव संरचना spl_fit_info *ctx, पूर्णाङ्क ग्रन्थिः,
			   संरचना spl_image_info *image_info, ulong max_आकार)
{
	u32 विस्थापनम्;
	u32 len;
	size_t दीर्घता;
	ulong आकार;
	ulong आरोपय_addr;
	शून्य *आरोपय_ptr;
	शून्य *src;
	uint8_t image_comp = -1, type = -1;
	ध्रुव शून्य *दत्त;
	ध्रुव शून्य *fit = ctx->fit;
	bool external_दत्त = false;
	पूर्णाङ्क ret;

	log_debug("starting\n");
	यदि (CONFIG_IS_ENABLED(BOOTMETH_VBE) &&
	    xpl_get_phase(info) != IH_PHASE_NONE) {
		गणना image_phase_t phase;

		ret = fit_image_get_phase(fit, ग्रन्थिः, &phase);
		/* if the image is for any phase, let's use it */
		यदि (ret == -ENOENT || phase == xpl_get_phase(info)) {
			log_debug("found\n");
		} अन्यथा यदि (ret < 0) {
			log_debug("err=%d\n", ret);
			प्रत्यागम ret;
		} अन्यथा {
			log_debug("- phase mismatch, skipping this image\n");
			प्रत्यागम -EBADSLT;
		}
	}

	यदि (IS_ENABLED(CONFIG_SPL_FPGA) ||
	    (IS_ENABLED(CONFIG_SPL_OS_BOOT) && spl_decompression_enabled())) {
		यदि (fit_image_get_type(fit, ग्रन्थिः, &type))
			पङ्क्तिं_लिख("Cannot get image type.\n");
		अन्यथा
			debug("%s ", genimg_get_type_name(type));
	}

	यदि (spl_decompression_enabled()) {
		fit_image_get_comp(fit, ग्रन्थिः, &image_comp);
		debug("%s ", genimg_get_comp_name(image_comp));
	}

	यदि (fit_image_get_आरोपय(fit, ग्रन्थिः, &आरोपय_addr)) {
		यदि (!image_info->आरोपय_addr) {
			प्रारूप_लिख("Can't load %s: No load address and no buffer\n",
			       fit_get_name(fit, ग्रन्थिः, रिक्त));
			प्रत्यागम -ENOBUFS;
		}
		आरोपय_addr = image_info->आरोपय_addr;
	}

	यदि (!fit_image_get_दत्त_position(fit, ग्रन्थिः, &विस्थापनम्)) {
		external_दत्त = true;
	} अन्यथा यदि (!fit_image_get_दत्त_विस्थापनम्(fit, ग्रन्थिः, &विस्थापनम्)) {
		/* The resulting offset cannot exceed UINT32_MAX */
		यदि (ctx->ext_दत्त_विस्थापनम् > UINT32_MAX - विस्थापनम्) {
			log_debug("Invalid external data offset: %u\n", विस्थापनम्);
			प्रत्यागम -EINVAL;
		}
		log_debug("read offset %x = offset from fit %x\n", विस्थापनम्,
			  (u32)(विस्थापनम् + ctx->ext_दत्त_विस्थापनम्));
		विस्थापनम् += ctx->ext_दत्त_विस्थापनम्;
		external_दत्त = true;
	}

	यदि (external_दत्त) {
		u32 aligned_विस्थापनम्;
		ulong read_विस्थापनम्;
		शून्य *src_ptr;

		/* External data */
		यदि (fit_image_get_दत्त_आकार(fit, ग्रन्थिः, &len))
			प्रत्यागम -ENOENT;

		/* Dont bother to copy 0 byte data, but warn, though */
		यदि (!len) {
			log_warning("%s: Skip load '%s': image size is 0!\n",
				    __func__, fit_get_name(fit, ग्रन्थिः, रिक्त));
			fit_fill_image_info(fit, ग्रन्थिः, image_info, आरोपय_addr, 0);
			प्रत्यागम 0;
		}

		/*
		 * data-size is excluded from the configuration signature (it
		 * is in exc_prop[] in image-fit-sig.c), so it stays attacker
		 * controlled even after fit_config_verify() succeeds. The
		 * image hash is only verified after the device read below, so
		 * an oversized value has to be rejected here.
		 */
		ret = get_aligned_image_आकार(info, len, विस्थापनम्, &आकार);
		यदि (ret) {
			log_debug("Invalid external data size: %u\n", len);
			प्रत्यागम ret;
		}

		यदि (आकार > max_आकार) {
			log_debug("Image too large: aligned size %lu, max %lu (data-size %u)\n",
				  आकार, max_आकार, len);
			प्रत्यागम -EFBIG;
		}

		aligned_विस्थापनम् = get_aligned_image_विस्थापनम्(info, विस्थापनम्);
		यदि (aligned_विस्थापनम् > ULONG_MAX - fit_विस्थापनम्) {
			log_debug("Invalid aligned external data offset: %u\n",
				  aligned_विस्थापनम्);
			प्रत्यागम -EINVAL;
		}
		read_विस्थापनम् = fit_विस्थापनम् + aligned_विस्थापनम्;

		यदि (spl_decompression_enabled() &&
		    (image_comp == IH_COMP_GZIP || image_comp == IH_COMP_LZMA))
			src_ptr = map_sysmem(ALIGN(CONFIG_SYS_आरोपय_ADDR, ARCH_DMA_MINALIGN), len);
		अन्यथा
			src_ptr = map_sysmem(ALIGN(आरोपय_addr, ARCH_DMA_MINALIGN), len);
		दीर्घता = len;

		/*
		 * For non-signed FIT images, we can check that
		 * (read_offset + size) does not wrap and that
		 * (src_ptr + size) does not exceed the addressable range.
		 * For signed FITs, we can additionally check that
		 * (offset + len) doesn't exceed the allowed FIT image
		 * maximum size.
		 */
		यदि (आकार > ULONG_MAX - read_विस्थापनम् ||
		    आकार > UINTPTR_MAX - (uintptr_t)src_ptr
		/*
		 * #if (not a runtime if) is required: FIT_SIGNATURE_MAX_SIZE
		 * depends on FIT_SIGNATURE, so CONFIG_VAL(FIT_SIGNATURE_MAX_SIZE)
		 * is undefined when signing is disabled and referencing it
		 * here would fail to compile.
		 */
#यदि CONFIG_IS_ENABLED(FIT_SIGNATURE)
		    || विस्थापनम् > CONFIG_VAL(FIT_SIGNATURE_MAX_आकार) ||
		    len > CONFIG_VAL(FIT_SIGNATURE_MAX_आकार) - विस्थापनम्
#यदि_अन्तः
		) {
			log_debug("FIT external data is out of bounds (offset=%u, size=%u)\n",
				  विस्थापनम्, len);
			प्रत्यागम -EINVAL;
		}

		log_debug("reading from offset %x / %lx size %lx to %p: ",
			  विस्थापनम्, read_विस्थापनम्, आकार, src_ptr);

		यदि (info->पठ(info, read_विस्थापनम्, आकार, src_ptr) < दीर्घता)
			प्रत्यागम -EIO;

		debug("External data: dst=%p, offset=%x, size=%lx\n",
		      src_ptr, विस्थापनम्, (अचिह्नित दीर्घ)दीर्घता);
		src = src_ptr + get_aligned_image_overhead(info, विस्थापनम्);
	} अन्यथा {
		/* Embedded data */
		यदि (fit_image_get_emb_दत्त(fit, ग्रन्थिः, &दत्त, &दीर्घता)) {
			पङ्क्तिं_लिख("Cannot get image data/size\n");
			प्रत्यागम -ENOENT;
		}
		debug("Embedded data: dst=%lx, size=%lx\n", आरोपय_addr,
		      (अचिह्नित दीर्घ)दीर्घता);
		src = (शून्य *)दत्त;	/* cast away const */
	}

	यदि (CONFIG_IS_ENABLED(FIT_SIGNATURE)) {
		प्रारूप_लिख("## Checking hash(es) for Image %s ... ",
		       fit_get_name(fit, ग्रन्थिः, रिक्त));
		यदि (!fit_image_verify_with_दत्त(fit, ग्रन्थिः, gd_fdt_blob(), src,
						दीर्घता))
			प्रत्यागम -EPERM;
		पङ्क्तिं_लिख("OK\n");
	}

	यदि (CONFIG_IS_ENABLED(FIT_IMAGE_POST_PROCESS))
		board_fit_image_post_process(fit, ग्रन्थिः, &src, &दीर्घता);

	आरोपय_ptr = map_sysmem(आरोपय_addr, दीर्घता);
	यदि (IS_ENABLED(CONFIG_SPL_GZIP) && image_comp == IH_COMP_GZIP) {
		आकार = दीर्घता;
		यदि (gunzip(आरोपय_ptr, CONFIG_SYS_BOOTM_LEN, src, &आकार)) {
			पङ्क्तिं_लिख("Uncompressing error\n");
			प्रत्यागम -EIO;
		}
		दीर्घता = आकार;
	} अन्यथा यदि (IS_ENABLED(CONFIG_SPL_LZMA) && image_comp == IH_COMP_LZMA) {
		आकार = CONFIG_SYS_BOOTM_LEN;
		ulong आरोपय_अन्तः;

		यदि (image_decomp(IH_COMP_LZMA, CONFIG_SYS_आरोपय_ADDR, 0, 0,
				 आरोपय_ptr, src, दीर्घता, आकार, &आरोपय_अन्तः)) {
			पङ्क्तिं_लिख("Uncompressing error\n");
			प्रत्यागम -EIO;
		}
		दीर्घता = आरोपय_अन्तः - CONFIG_SYS_आरोपय_ADDR;
	} अन्यथा {
		memmove(आरोपय_ptr, src, दीर्घता);
	}

	fit_fill_image_info(fit, ग्रन्थिः, image_info, आरोपय_addr, दीर्घता);
	log_debug("- done loading\n");

	upl_सङ्कलय_image(fit, ग्रन्थिः, आरोपय_addr, दीर्घता);

	प्रत्यागम 0;
}

स्थिर bool os_takes_devicetree(uint8_t os)
{
	विकल्प (os) {
	पक्ष IH_OS_U_BOOT:
		प्रत्यागम true;
	पक्ष IH_OS_LINUX:
		प्रत्यागम IS_ENABLED(CONFIG_SPL_OS_BOOT) ||
		       IS_ENABLED(CONFIG_SPL_OPENSBI);
	स्वभाव:
		प्रत्यागम false;
	}
}

__weak पूर्णाङ्क board_spl_fit_append_fdt_skip(ध्रुव वर्ण *name)
{
	प्रत्यागम 0;	/* Do not skip */
}

स्थिर पूर्णाङ्क spl_fit_append_fdt(संरचना spl_image_info *spl_image,
			      संरचना spl_आरोपय_info *info, ulong विस्थापनम्,
			      ध्रुव संरचना spl_fit_info *ctx)
{
	संरचना spl_image_info image_info;
	पूर्णाङ्क ग्रन्थिः, ret = 0, सूचकांक = 0;

	/*
	 * Use the address following the image as target address for the
	 * device tree.
	 */
	image_info.आरोपय_addr = ALIGN(spl_image->आरोपय_addr + spl_image->आकार, 8);

	/* Figure out which device tree the board wants to use */
	ग्रन्थिः = spl_fit_get_image_ग्रन्थिः(ctx, FIT_FDT_PROP, सूचकांक++);
	यदि (ग्रन्थिः < 0) {
		size_t आकार;

		debug("%s: cannot find FDT node\n", __func__);

		/*
		 * U-Boot did not find a device tree inside the FIT image. Use
		 * the U-Boot device tree instead.
		 */
		यदि (!gd->fdt_blob)
			प्रत्यागम ग्रन्थिः;

		/*
		 * Make the load-address of the FDT available for the SPL
		 * framework
		 */
		आकार = fdt_totalsize(gd->fdt_blob);
		spl_image->fdt_addr = map_sysmem(image_info.आरोपय_addr, आकार);
		स्मृतिं_प्रतिलिख(spl_image->fdt_addr, gd->fdt_blob, आकार);
	} अन्यथा {
		ret = आरोपय_simple_fit(info, विस्थापनम्, ctx, ग्रन्थिः, &image_info,
				      CONFIG_SYS_BOOTM_LEN);
		यदि (ret < 0)
			प्रत्यागम ret;

		spl_image->fdt_addr = phys_गन्तव्यम्_virt(image_info.आरोपय_addr);
	}

	यदि (CONFIG_IS_ENABLED(FIT_IMAGE_TINY))
		प्रत्यागम 0;

#यदि CONFIG_IS_ENABLED(आरोपय_FIT_APPLY_OVERLAY)
		शून्य *tmpbuffer = रिक्त;

		प्रति (; ; सूचकांक++) {
			ध्रुव वर्ण *str;

			ret = spl_fit_get_image_name(ctx, FIT_FDT_PROP, सूचकांक, &str);
			यदि (ret == -E2BIG) {
				debug("%s: No additional FDT node\n", __func__);
				ret = 0;
				भङ्ग;
			} अन्यथा यदि (ret) {
				भङ्ग;
			}

			ret = board_spl_fit_append_fdt_skip(str);
			यदि (ret)
				अनुवर्त;

			ग्रन्थिः = fdt_subnode_विस्थापनम्(ctx->fit, ctx->images_ग्रन्थिः, str);
			यदि (ग्रन्थिः < 0) {
				debug("%s: unable to find FDT node %d\n",
				      __func__, सूचकांक);
				अनुवर्त;
			}

			यदि (!tmpbuffer) {
				/*
				 * allocate memory to store the DT overlay
				 * before it is applied. It may not be used
				 * depending on how the overlay is stored, so
				 * don't fail yet if the allocation failed.
				 */
				size_t आकार = CONFIG_SPL_आरोपय_FIT_APPLY_OVERLAY_BUF_SZ;

				tmpbuffer = malloc_cache_aligned(आकार);
				यदि (!tmpbuffer)
					debug("%s: unable to allocate space for overlays\n",
					      __func__);
			}
			image_info.आरोपय_addr = (ulong)tmpbuffer;
			ret = आरोपय_simple_fit(info, विस्थापनम्, ctx, ग्रन्थिः,
					      &image_info,
					      CONFIG_SPL_आरोपय_FIT_APPLY_OVERLAY_BUF_SZ);
			यदि (ret == -EBADSLT)
				अनुवर्त;
			अन्यथा यदि (ret < 0)
				भङ्ग;

			/* Make room in FDT for changes from the overlay */
			ret = fdt_increase_आकार(spl_image->fdt_addr,
						image_info.आकार);
			यदि (ret < 0)
				भङ्ग;

			ret = fdt_overlay_apply_verbose(spl_image->fdt_addr,
							(शून्य *)image_info.आरोपय_addr);
			यदि (ret) {
				pr_err("failed to apply DT overlay %s\n",
				       fit_get_name(ctx->fit, ग्रन्थिः, रिक्त));
				भङ्ग;
			}

			debug("%s: DT overlay %s applied\n", __func__,
			      fit_get_name(ctx->fit, ग्रन्थिः, रिक्त));
		}
		मुक्त(tmpbuffer);
		यदि (ret)
			प्रत्यागम ret;
#यदि_अन्तः
	/* Try to make space, so we can inject details on the loadables */
	ret = fdt_shrink_गन्तव्यम्_minimum(spl_image->fdt_addr, 8192);
	यदि (ret < 0)
		प्रत्यागम ret;

	प्रत्यागम ret;
}

स्थिर पूर्णाङ्क spl_fit_लेखः_loadable(ध्रुव संरचना spl_fit_info *ctx, पूर्णाङ्क सूचकांक,
				   शून्य *blob, संरचना spl_image_info *image)
{
	पूर्णाङ्क ret = 0;
	ध्रुव वर्ण *name;
	पूर्णाङ्क ग्रन्थिः;

	ret = spl_fit_get_image_name(ctx, FIT_LOADABLE_PROP, सूचकांक, &name);
	यदि (ret < 0)
		प्रत्यागम ret;

	ग्रन्थिः = spl_fit_get_image_ग्रन्थिः(ctx, FIT_LOADABLE_PROP, सूचकांक);

	ret = fdt_लेखः_loadable(blob, सूचकांक, name, image->आरोपय_addr,
			image->आकार, image->प्रविष्टिः_point,
			fdt_getprop(ctx->fit, ग्रन्थिः, FIT_TYPE_PROP, रिक्त),
			fdt_getprop(ctx->fit, ग्रन्थिः, FIT_OS_PROP, रिक्त),
			fdt_getprop(ctx->fit, ग्रन्थिः, FIT_ARCH_PROP, रिक्त));

	प्रत्यागम ret;
}

स्थिर पूर्णाङ्क spl_fit_image_is_fpga(ध्रुव शून्य *fit, पूर्णाङ्क ग्रन्थिः)
{
	ध्रुव वर्ण *type;

	यदि (!IS_ENABLED(CONFIG_SPL_FPGA))
		प्रत्यागम 0;

	type = fdt_getprop(fit, ग्रन्थिः, FIT_TYPE_PROP, रिक्त);
	यदि (!type)
		प्रत्यागम 0;

	प्रत्यागम !वाक्यं_तुलय(type, "fpga");
}

स्थिर शून्य spl_fit_image_लेखः_arm32_optee(ध्रुव शून्य *fit, पूर्णाङ्क ग्रन्थिः,
					     संरचना spl_image_info *spl_image,
					     संरचना spl_image_info *image_info)
{
#यदि defined(CONFIG_BOOTM_OPTEE) && defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
	ध्रुव वर्ण *type = fdt_getprop(fit, ग्रन्थिः, FIT_TYPE_PROP, रिक्त);

	यदि (!type)
		प्रत्यागम;

	यदि (वाक्यं_तुलय(type, "tee"))
		प्रत्यागम;

	spl_image->optee_addr = image_info->आरोपय_addr;
#यदि_अन्तः
}

स्थिर पूर्णाङ्क spl_fit_image_get_os(ध्रुव शून्य *fit, पूर्णाङ्क noffset, uint8_t *os)
{
	यदि (!CONFIG_IS_ENABLED(FIT_IMAGE_TINY) || CONFIG_IS_ENABLED(OS_BOOT))
		प्रत्यागम fit_image_get_os(fit, noffset, os);

	ध्रुव वर्ण *name = fdt_getprop(fit, noffset, FIT_OS_PROP, रिक्त);
	यदि (!name)
		प्रत्यागम -ENOENT;

	/*
	 * We don't care what the type of the image actually is,
	 * only whether or not it is U-Boot. This saves some
	 * space by omitting the large table of OS types.
	 */
	यदि (!वाक्यं_तुलय(name, "u-boot"))
		*os = IH_OS_U_BOOT;
	अन्यथा
		*os = IH_OS_INVALID;

	प्रत्यागम 0;
}

/*
 * The purpose of the FIT load buffer is to provide a memory location that is
 * independent of the load address of any FIT component.
 */
__weak शून्य *board_spl_fit_बफर_addr(ulong fit_आकार, पूर्णाङ्क sectors, पूर्णाङ्क bl_len)
{
	शून्य *buf;
	size_t आकार = sectors * bl_len;

	buf = malloc_cache_aligned(आकार);
	यदि (!buf) {
		pr_err("Could not get FIT buffer of %lu bytes\n", (ulong)आकार);

		यदि (IS_ENABLED(CONFIG_SPL_SYS_MALLOC))
			pr_err("\tcheck CONFIG_SPL_SYS_MALLOC_SIZE\n");
		अन्यथा
			pr_err("\tcheck CONFIG_SPL_SYS_MALLOC_F_LEN\n");

		buf = spl_get_आरोपय_बफर(0, आकार);
	}
	प्रत्यागम buf;
}

/*
 * Weak default function to allow customizing SPL fit loading for load-only
 * use cases by allowing to skip the parsing/processing of the FIT contents
 * (so that this can be done separately in a more customized fashion)
 */
__weak bool spl_आरोपय_simple_fit_skip_processing(शून्य)
{
	प्रत्यागम false;
}

/*
 * Weak default function to allow fixes after fit header
 * is loaded.
 */
__weak शून्य *spl_आरोपय_simple_fit_fix_आरोपय(ध्रुव शून्य *fit)
{
	प्रत्यागम (शून्य *)fit;
}

स्थिर शून्य warn_deprecated(ध्रुव वर्ण *सन्देशः)
{
	प्रारूप_लिख("DEPRECATED: %s\n", सन्देशः);
	प्रारूप_लिख("\tSee https://fitspec.osfw.foundation/\n");
}

स्थिर पूर्णाङ्क spl_fit_upload_fpga(संरचना spl_fit_info *ctx, पूर्णाङ्क ग्रन्थिः,
			       संरचना spl_image_info *fpga_image)
{
	ध्रुव वर्ण *compatible;
	पूर्णाङ्क ret;
	पूर्णाङ्क devnum = 0;
	पूर्णाङ्क flags = 0;

	debug("FPGA bitstream at: %x, size: %x\n",
	      (u32)fpga_image->आरोपय_addr, fpga_image->आकार);

	compatible = fdt_getprop(ctx->fit, ग्रन्थिः, FIT_COMPAT_PROP, रिक्त);
	यदि (!compatible) {
		warn_deprecated("'fpga' image without 'compatible' property");
	} अन्यथा {
		यदि (CONFIG_IS_ENABLED(FPGA_आरोपय_SECURE))
			flags = fpga_compatible2flag(devnum, compatible);
		यदि (वाक्यं_तुलय(compatible, "u-boot,fpga-legacy"))
			debug("Ignoring compatible = %s property\n",
			      compatible);
	}

	ret = fpga_आरोपय(devnum, (शून्य *)fpga_image->आरोपय_addr,
			fpga_image->आकार, BIT_पूर्णम्, flags);
	यदि (ret) {
		प्रारूप_लिख("%s: Cannot load the image to the FPGA\n", __func__);
		प्रत्यागम ret;
	}

	पङ्क्तिं_लिख("FPGA image loaded from FIT\n");

	प्रत्यागम 0;
}

स्थिर पूर्णाङ्क spl_fit_आरोपय_fpga(संरचना spl_fit_info *ctx,
			     संरचना spl_आरोपय_info *info, ulong विस्थापनम्)
{
	पूर्णाङ्क ग्रन्थिः, ret;

	संरचना spl_image_info fpga_image = {
		.आरोपय_addr = 0,
	};

	ग्रन्थिः = spl_fit_get_image_ग्रन्थिः(ctx, FIT_FPGA_PROP, 0);
	यदि (ग्रन्थिः < 0)
		प्रत्यागम ग्रन्थिः;

	warn_deprecated("'fpga' property in config node. Use 'loadables'");

	/* Load the image and set up the fpga_image structure */
	ret = आरोपय_simple_fit(info, विस्थापनम्, ctx, ग्रन्थिः, &fpga_image,
			      CONFIG_SYS_BOOTM_LEN);
	यदि (ret) {
		प्रारूप_लिख("%s: Cannot load the FPGA: %i\n", __func__, ret);
		प्रत्यागम ret;
	}

	प्रत्यागम spl_fit_upload_fpga(ctx, ग्रन्थिः, &fpga_image);
}

स्थिर पूर्णाङ्क spl_simple_fit_read(संरचना spl_fit_info *ctx,
			       संरचना spl_आरोपय_info *info, ulong विस्थापनम्,
			       ध्रुव शून्य *fit_शीर्षकम्)
{
	अचिह्नित दीर्घ aligned_आकार, गणनम्, आकार;
	शून्य *buf;
	पूर्णाङ्क ret;

	/*
	 * For FIT with external data, figure out where the external images
	 * start. This is the base for the data-offset properties in each
	 * image.
	 */
	आकार = ALIGN(fdt_totalsize(fit_शीर्षकम्), 4);
	आकार = board_spl_fit_आकार_align(आकार);
	ctx->ext_दत्त_विस्थापनम् = ALIGN(आकार, 4);

	/*
	 * So far we only have one block of data from the FIT. Read the entire
	 * thing, including that first block.
	 *
	 * For FIT with data embedded, data is loaded as part of FIT image.
	 * For FIT with external data, data is not loaded in this step.
	 */
	ret = get_aligned_image_आकार(info, आकार, 0, &aligned_आकार);
	यदि (ret) {
		log_debug("Invalid FIT size: %lu\n", आकार);
		प्रत्यागम ret;
	}
	buf = board_spl_fit_बफर_addr(aligned_आकार, aligned_आकार, 1);
	यदि (!buf) {
		/*
		 * We assume that none of the board will ever use 0x0 as a
		 * valid load address. Theoretically some board could use it,
		 * but this is extremely unlikely.
		 */
		प्रत्यागम -EIO;
	}

	गणनम् = info->पठ(info, विस्थापनम्, aligned_आकार, buf);
	यदि (!गणनम्) {
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
		प्रत्यागम -EIO;
	}

	ctx->fit = buf;
	debug("fit read offset %lx, size=%lu, dst=%p, count=%lu\n",
	      विस्थापनम्, aligned_आकार, buf, गणनम्);

	प्रत्यागम 0;
}

स्थिर पूर्णाङ्क spl_simple_fit_parse(संरचना spl_fit_info *ctx)
{
	/* Find the correct subnode under "/configurations" */
	ctx->conf_ग्रन्थिः = fit_find_config_ग्रन्थिः(ctx->fit);
	यदि (ctx->conf_ग्रन्थिः < 0)
		प्रत्यागम -EINVAL;

	यदि (CONFIG_IS_ENABLED(FIT_SIGNATURE)) {
		प्रारूप_लिख("## Checking hash(es) for config %s ... ",
		       fit_get_name(ctx->fit, ctx->conf_ग्रन्थिः, रिक्त));
		यदि (fit_config_verify(ctx->fit, ctx->conf_ग्रन्थिः))
			प्रत्यागम -EPERM;
		पङ्क्तिं_लिख("OK\n");
	}

	/* find the node holding the images information */
	ctx->images_ग्रन्थिः = fdt_मार्गः_विस्थापनम्(ctx->fit, FIT_IMAGES_मार्गः);
	यदि (ctx->images_ग्रन्थिः < 0) {
		debug("%s: Cannot find /images node: %d\n", __func__,
		      ctx->images_ग्रन्थिः);
		प्रत्यागम -EINVAL;
	}

	प्रत्यागम 0;
}

पूर्णाङ्क spl_आरोपय_simple_fit(संरचना spl_image_info *spl_image,
			संरचना spl_आरोपय_info *info, ulong विस्थापनम्, शून्य *fit)
{
	संरचना spl_image_info image_info;
	संरचना spl_fit_info ctx;
	पूर्णाङ्क ग्रन्थिः = -1;
	पूर्णाङ्क ret;
	पूर्णाङ्क सूचकांक = 0;
	पूर्णाङ्क firmware_ग्रन्थिः;

	ret = spl_simple_fit_read(&ctx, info, विस्थापनम्, fit);
	यदि (ret < 0)
		प्रत्यागम ret;

	/* skip further processing if requested to enable load-only use cases */
	यदि (spl_आरोपय_simple_fit_skip_processing())
		प्रत्यागम 0;

	ctx.fit = spl_आरोपय_simple_fit_fix_आरोपय(ctx.fit);

	ret = spl_simple_fit_parse(&ctx);
	यदि (ret < 0)
		प्रत्यागम ret;

	यदि (IS_ENABLED(CONFIG_SPL_FPGA))
		spl_fit_आरोपय_fpga(&ctx, info, विस्थापनम्);

	/*
	 * Find the U-Boot image using the following search order:
	 *   - start at 'firmware' (e.g. an ARM Trusted Firmware)
	 *   - fall back 'kernel' (e.g. a Falcon-mode OS boot
	 *   - fall back to using the first 'loadables' entry
	 */
	यदि (ग्रन्थिः < 0)
		ग्रन्थिः = spl_fit_get_image_ग्रन्थिः(&ctx, FIT_FIRMWARE_PROP, 0);

	यदि (ग्रन्थिः < 0 && IS_ENABLED(CONFIG_SPL_OS_BOOT))
		ग्रन्थिः = spl_fit_get_image_ग्रन्थिः(&ctx, FIT_KERNEL_PROP, 0);

	यदि (ग्रन्थिः < 0) {
		debug("could not find firmware image, trying loadables...\n");
		ग्रन्थिः = spl_fit_get_image_ग्रन्थिः(&ctx, FIT_LOADABLE_PROP, 0);
		/*
		 * If we pick the U-Boot image from "loadables", start at
		 * the second image when later loading additional images.
		 */
		सूचकांक = 1;
	}
	यदि (ग्रन्थिः < 0) {
		debug("%s: Cannot find u-boot image node: %d\n",
		      __func__, ग्रन्थिः);
		प्रत्यागम -1;
	}

	/* Load the image and set up the spl_image structure */
	ret = आरोपय_simple_fit(info, विस्थापनम्, &ctx, ग्रन्थिः, spl_image,
			      CONFIG_SYS_BOOTM_LEN);
	यदि (ret)
		प्रत्यागम ret;

	/*
	 * For backward compatibility, we treat the first node that is
	 * as a U-Boot image, if no OS-type has been declared.
	 */
	यदि (!spl_fit_image_get_os(ctx.fit, ग्रन्थिः, &spl_image->os))
		debug("Image OS is %s\n", genimg_get_os_name(spl_image->os));
	अन्यथा यदि (!IS_ENABLED(CONFIG_SPL_OS_BOOT))
		spl_image->os = IH_OS_U_BOOT;

	/*
	 * Booting a next-stage U-Boot may require us to append the FDT.
	 * We allow this to fail, as the U-Boot image might embed its FDT.
	 */
	यदि (os_takes_devicetree(spl_image->os)) {
		ret = spl_fit_append_fdt(spl_image, info, विस्थापनम्, &ctx);
		यदि (ret < 0 && spl_image->os != IH_OS_U_BOOT)
			प्रत्यागम ret;
	}

	firmware_ग्रन्थिः = ग्रन्थिः;
	/* Now check if there are more images for us to load */
	प्रति (; ; सूचकांक++) {
		uint8_t os_type = IH_OS_INVALID;

		ग्रन्थिः = spl_fit_get_image_ग्रन्थिः(&ctx, FIT_LOADABLE_PROP, सूचकांक);
		यदि (ग्रन्थिः < 0)
			भङ्ग;

		/*
		 * if the firmware is also a loadable, skip it because
		 * it already has been loaded. This is typically the case with
		 * u-boot.img generated by mkimage.
		 */
		यदि (firmware_ग्रन्थिः == ग्रन्थिः)
			अनुवर्त;

		image_info.आरोपय_addr = 0;
		ret = आरोपय_simple_fit(info, विस्थापनम्, &ctx, ग्रन्थिः, &image_info,
				      CONFIG_SYS_BOOTM_LEN);
		यदि (ret < 0 && ret != -EBADSLT) {
			प्रारूप_लिख("%s: can't load image loadables index %d (ret = %d)\n",
			       __func__, सूचकांक, ret);
			प्रत्यागम ret;
		}

		यदि (spl_fit_image_is_fpga(ctx.fit, ग्रन्थिः))
			spl_fit_upload_fpga(&ctx, ग्रन्थिः, &image_info);

		यदि (!spl_fit_image_get_os(ctx.fit, ग्रन्थिः, &os_type))
			debug("Loadable is %s\n", genimg_get_os_name(os_type));

		यदि (os_takes_devicetree(os_type)) {
			spl_fit_append_fdt(&image_info, info, विस्थापनम्, &ctx);
			spl_image->fdt_addr = image_info.fdt_addr;
		}

		/*
		 * If the "firmware" image did not provide an entry point,
		 * use the first valid entry point from the loadables.
		 */
		यदि (spl_image->प्रविष्टिः_point == FDT_ERROR &&
		    image_info.प्रविष्टिः_point != FDT_ERROR)
			spl_image->प्रविष्टिः_point = image_info.प्रविष्टिः_point;

		spl_fit_image_लेखः_arm32_optee(ctx.fit, ग्रन्थिः, spl_image,
						 &image_info);

		/* Record our loadables into the FDT */
		यदि (!CONFIG_IS_ENABLED(FIT_IMAGE_TINY) &&
		    xpl_get_fdt_update(info) && spl_image->fdt_addr)
			spl_fit_लेखः_loadable(&ctx, सूचकांक,
						spl_image->fdt_addr,
						&image_info);
	}

	/*
	 * If a platform does not provide CONFIG_SYS_UBOOT_START, U-Boot's
	 * Makefile will set it to 0 and it will end up as the entry point
	 * here. What it actually means is: use the load address.
	 */
	यदि (spl_image->प्रविष्टिः_point == FDT_ERROR || spl_image->प्रविष्टिः_point == 0)
		spl_image->प्रविष्टिः_point = spl_image->आरोपय_addr;

	spl_image->flags |= SPL_FIT_प्राप्तम्;
	upl_समुच्चयः_fit_info(map_गन्तव्यम्_sysmem(ctx.fit), ctx.conf_ग्रन्थिः,
			 spl_image->प्रविष्टिः_point);

	प्रत्यागम 0;
}

/* Parse and load full fitImage in SPL */
पूर्णाङ्क spl_आरोपय_fit_image(संरचना spl_image_info *spl_image,
		       ध्रुव संरचना legacy_img_hdr *शीर्षकम्)
{
	संरचना bootm_शीर्षकाणि images;
	ध्रुव वर्ण *fit_uname_config = रिक्त;
	ulong fdt_hack;
	ध्रुव वर्ण *uname;
	ulong fw_दत्त = 0, dt_दत्त = 0, img_दत्त = 0;
	ulong fw_len = 0, dt_len = 0, img_len = 0;
	पूर्णाङ्क idx, conf_noffset;
	पूर्णाङ्क ret;

	images.verify = CONFIG_IS_ENABLED(FIT_SIGNATURE);

	ret = fit_image_आरोपय(&images, virt_गन्तव्यम्_phys((शून्य *)शीर्षकम्),
			     रिक्त, &fit_uname_config, IH_ARCH_DEFAULT,
			     IH_TYPE_FIRMWARE, -1, FIT_आरोपय_OPTIONAL,
			     &fw_दत्त, &fw_len);

	यदि (ret < 0) {
		ret = fit_image_आरोपय(&images, virt_गन्तव्यम्_phys((शून्य *)शीर्षकम्),
				     रिक्त, &fit_uname_config, IH_ARCH_DEFAULT,
				     IH_TYPE_KERNEL, -1, FIT_आरोपय_OPTIONAL,
				     &fw_दत्त, &fw_len);
	}

	यदि (ret < 0)
		प्रत्यागम ret;

	spl_image->आकार = fw_len;
	spl_image->आरोपय_addr = fw_दत्त;
	यदि (fit_image_get_प्रविष्टिः(शीर्षकम्, ret, &spl_image->प्रविष्टिः_point))
		spl_image->प्रविष्टिः_point = fw_दत्त;
	यदि (fit_image_get_os(शीर्षकम्, ret, &spl_image->os))
		spl_image->os = IH_OS_INVALID;
	spl_image->name = genimg_get_os_name(spl_image->os);

	debug(PHASE_PROMPT "payload image: %32s load addr: 0x%lx size: %d\n",
	      spl_image->name, spl_image->आरोपय_addr, spl_image->आकार);

	images.verify = CONFIG_IS_ENABLED(FIT_SIGNATURE);

	ret = fit_image_आरोपय(&images, virt_गन्तव्यम्_phys((शून्य *)शीर्षकम्), रिक्त,
			     &fit_uname_config, IH_ARCH_DEFAULT, IH_TYPE_FLATDT,
			     -1, FIT_आरोपय_OPTIONAL, &dt_दत्त, &dt_len);
	यदि (ret >= 0) {
		यदि (spl_image->os == IH_OS_U_BOOT) {
			/* HACK: U-Boot expects FDT at a specific address */
			fdt_hack = ALIGN(spl_image->आरोपय_addr + spl_image->आकार, 8);
			debug("Relocating FDT to %p\n", (शून्य *)fdt_hack);
			स्मृतिं_प्रतिलिख(map_sysmem(fdt_hack, dt_len),
			       map_sysmem(dt_दत्त, 0), dt_len);
			spl_image->fdt_addr = (शून्य *)fdt_hack;
		} अन्यथा {
			spl_image->fdt_addr = (शून्य *)dt_दत्त;
		}
	}

	conf_noffset = fit_conf_get_ग्रन्थिः((ध्रुव शून्य *)शीर्षकम्,
					 fit_uname_config);
	यदि (conf_noffset < 0)
		प्रत्यागम 0;

	प्रति (idx = 0;
	     uname = fdt_stringlist_get((ध्रुव शून्य *)शीर्षकम्, conf_noffset,
					FIT_LOADABLE_PROP, idx,
				रिक्त), uname;
	     idx++) {
		images.verify = CONFIG_IS_ENABLED(FIT_SIGNATURE);

		ret = fit_image_आरोपय(&images, virt_गन्तव्यम्_phys((शून्य *)शीर्षकम्),
				     &uname, &fit_uname_config,
				     IH_ARCH_DEFAULT, IH_TYPE_LOADABLE, -1,
				     FIT_आरोपय_OPTIONAL_NON_ZERO,
				     &img_दत्त, &img_len);
		यदि (ret < 0)
			प्रत्यागम ret;
	}
	spl_image->flags |= SPL_FIT_प्राप्तम्;

	upl_समुच्चयः_fit_info(map_गन्तव्यम्_sysmem(शीर्षकम्), conf_noffset,
			 spl_image->प्रविष्टिः_point);

	प्रत्यागम 0;
}
