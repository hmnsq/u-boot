// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#समावेश <errno.h>
#समावेश <fpga.h>
#समावेश <gzip.h>
#समावेश <image.h>
#समावेश <log.h>
#समावेश <memalign.h>
#समावेश <mapmem.h>
#समावेश <spl.h>
#समावेश <upl.h>
#समावेश <sysinfo.h>
#समावेश <asm/global_data.h>
#समावेश <asm/io.h>
#समावेश <linux/libfdt.h>
#समावेश <linux/printk.h>

DECLARE_GLOBAL_तथ्य_PTR;

काष्ठा spl_fit_info {
	स्थिर व्योम *fit;	/* Pointer to a valid FIT blob */
	माप_प्रकार ext_तथ्य_offset;	/* Offset to FIT external data (end of FIT) */
	पूर्णांक images_गाँठ;	/* FDT offset to "/images" node */
	पूर्णांक conf_गाँठ;		/* FDT offset to selected configuration node */
};

__weak ulong board_spl_fit_आकार_align(ulong आकार)
{
	वापस आकार;
}

अटल पूर्णांक find_गाँठ_from_desc(स्थिर व्योम *fit, पूर्णांक गाँठ, स्थिर अक्षर *str)
{
	पूर्णांक child;

	अगर (गाँठ < 0)
		वापस -EINVAL;

	/* iterate the FIT nodes and find a matching description */
	क्रम (child = fdt_first_subnode(fit, गाँठ); child >= 0;
	     child = fdt_अग्र_subnode(fit, child)) {
		पूर्णांक len;
		स्थिर अक्षर *desc = fdt_getprop(fit, child, FIT_DESC_PROP, &len);

		अगर (!desc)
			जारी;

		अगर (!म_भेद(desc, str))
			वापस child;
	}

	वापस -ENOENT;
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
अटल पूर्णांक spl_fit_get_image_name(स्थिर काष्ठा spl_fit_info *ctx,
				  स्थिर अक्षर *type, पूर्णांक सूचकांक,
				  स्थिर अक्षर **outname)
{
	काष्ठा udevice *sysinfo;
	स्थिर अक्षर *name, *str, *end;
	__maybe_unused पूर्णांक गाँठ;
	पूर्णांक len, i;
	bool found = true;

	name = fdt_getprop(ctx->fit, ctx->conf_गाँठ, type, &len);
	अगर (!name) {
		debug("cannot find property '%s': %d\n", type, len);
		वापस -EINVAL;
	}
	/* A string property should be NUL terminated */
	end = name + len - 1;
	अगर (!len || *end) {
		debug("malformed property '%s'\n", type);
		वापस -EINVAL;
	}

	str = name;
	क्रम (i = 0; i < सूचकांक; i++) {
		str = म_अक्षर(str, '\0') + 1;
		अगर (str > end) {
			found = false;
			अवरोध;
		}
	}

	अगर (!found && CONFIG_IS_ENABLED(SYSINFO) && !sysinfo_get(&sysinfo)) {
		पूर्णांक rc;
		/*
		 * no string in the property for this index. Check if the
		 * sysinfo-level code can supply one.
		 */
		rc = sysinfo_detect(sysinfo);
		अगर (rc)
			वापस rc;

		rc = sysinfo_get_fit_loadable(sysinfo, सूचकांक - i - 1, type,
					      &str);
		अगर (rc && rc != -ENOENT)
			वापस rc;

		अगर (!rc) {
			/*
			 * The sysinfo provided a name for a loadable.
			 * Try to match it against the description properties
			 * first. If no matching node is found, use it as a
			 * node name.
			 */
			पूर्णांक गाँठ;
			पूर्णांक images = fdt_path_offset(ctx->fit, FIT_IMAGES_PATH);

			गाँठ = find_गाँठ_from_desc(ctx->fit, images, str);
			अगर (गाँठ > 0)
				str = fdt_get_name(ctx->fit, गाँठ, शून्य);

			found = true;
		}
	}

	अगर (!found) {
		debug("no string for index %d\n", सूचकांक);
		वापस -E2BIG;
	}

	*outname = str;
	वापस 0;
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
अटल पूर्णांक spl_fit_get_image_गाँठ(स्थिर काष्ठा spl_fit_info *ctx,
				  स्थिर अक्षर *type, पूर्णांक सूचकांक)
{
	स्थिर अक्षर *str;
	पूर्णांक err;
	पूर्णांक गाँठ;

	err = spl_fit_get_image_name(ctx, type, सूचकांक, &str);
	अगर (err)
		वापस err;

	debug("%s: '%s'\n", type, str);

	गाँठ = fdt_subnode_offset(ctx->fit, ctx->images_गाँठ, str);
	अगर (गाँठ < 0) {
		pr_err("cannot find image node '%s': %d\n", str, गाँठ);
		वापस -EINVAL;
	}

	वापस गाँठ;
}

अटल u32 get_aligned_image_offset(काष्ठा spl_load_info *info, u32 offset)
{
	वापस ALIGN_DOWN(offset, spl_get_bl_len(info));
}

अटल u32 get_aligned_image_overhead(काष्ठा spl_load_info *info, u32 offset)
{
	वापस offset & (spl_get_bl_len(info) - 1);
}

अटल पूर्णांक get_aligned_image_आकार(काष्ठा spl_load_info *info, ulong तथ्य_आकार,
				  u32 offset, ulong *aligned_आकार)
{
	u32 overhead = get_aligned_image_overhead(info, offset);

	अगर (तथ्य_आकार > अच_दीर्घ_उच्च - overhead)
		वापस -EOVERFLOW;
	तथ्य_आकार += overhead;

	अगर (तथ्य_आकार > अच_दीर्घ_उच्च - (spl_get_bl_len(info) - 1))
		वापस -EOVERFLOW;
	तथ्य_आकार = ALIGN(तथ्य_आकार, spl_get_bl_len(info));

	*aligned_आकार = तथ्य_आकार;

	वापस 0;
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
अटल व्योम fit_fill_image_info(स्थिर व्योम *fit, पूर्णांक गाँठ,
				काष्ठा spl_image_info *image_info,
				ulong load_addr, ulong आकार)
{
	ulong entry_point;

	अगर (!image_info)
		वापस;

	image_info->load_addr = load_addr;
	image_info->आकार = आकार;

	अगर (!fit_image_get_entry(fit, गाँठ, &entry_point))
		image_info->entry_point = entry_point;
	अन्यथा
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
अटल पूर्णांक load_simple_fit(काष्ठा spl_load_info *info, ulong fit_offset,
			   स्थिर काष्ठा spl_fit_info *ctx, पूर्णांक गाँठ,
			   काष्ठा spl_image_info *image_info, ulong max_आकार)
{
	u32 offset;
	u32 len;
	माप_प्रकार लंबाई;
	ulong आकार;
	ulong load_addr;
	व्योम *load_ptr;
	व्योम *src;
	uint8_t image_comp = -1, type = -1;
	स्थिर व्योम *तथ्य;
	स्थिर व्योम *fit = ctx->fit;
	bool external_तथ्य = false;
	पूर्णांक ret;

	log_debug("starting\n");
	अगर (CONFIG_IS_ENABLED(BOOTMETH_VBE) &&
	    xpl_get_phase(info) != IH_PHASE_NONE) {
		क्रमागत image_phase_t phase;

		ret = fit_image_get_phase(fit, गाँठ, &phase);
		/* if the image is for any phase, let's use it */
		अगर (ret == -ENOENT || phase == xpl_get_phase(info)) {
			log_debug("found\n");
		} अन्यथा अगर (ret < 0) {
			log_debug("err=%d\n", ret);
			वापस ret;
		} अन्यथा {
			log_debug("- phase mismatch, skipping this image\n");
			वापस -EBADSLT;
		}
	}

	अगर (IS_ENABLED(CONFIG_SPL_FPGA) ||
	    (IS_ENABLED(CONFIG_SPL_OS_BOOT) && spl_decompression_enabled())) {
		अगर (fit_image_get_type(fit, गाँठ, &type))
			माला_दो("Cannot get image type.\n");
		अन्यथा
			debug("%s ", genimg_get_type_name(type));
	}

	अगर (spl_decompression_enabled()) {
		fit_image_get_comp(fit, गाँठ, &image_comp);
		debug("%s ", genimg_get_comp_name(image_comp));
	}

	अगर (fit_image_get_load(fit, गाँठ, &load_addr)) {
		अगर (!image_info->load_addr) {
			म_लिखो("Can't load %s: No load address and no buffer\n",
			       fit_get_name(fit, गाँठ, शून्य));
			वापस -ENOBUFS;
		}
		load_addr = image_info->load_addr;
	}

	अगर (!fit_image_get_तथ्य_position(fit, गाँठ, &offset)) {
		external_तथ्य = true;
	} अन्यथा अगर (!fit_image_get_तथ्य_offset(fit, गाँठ, &offset)) {
		/* The resulting offset cannot exceed UINT32_MAX */
		अगर (ctx->ext_तथ्य_offset > UINT32_MAX - offset) {
			log_debug("Invalid external data offset: %u\n", offset);
			वापस -EINVAL;
		}
		log_debug("read offset %x = offset from fit %x\n", offset,
			  (u32)(offset + ctx->ext_तथ्य_offset));
		offset += ctx->ext_तथ्य_offset;
		external_तथ्य = true;
	}

	अगर (external_तथ्य) {
		u32 aligned_offset;
		ulong read_offset;
		व्योम *src_ptr;

		/* External data */
		अगर (fit_image_get_तथ्य_आकार(fit, गाँठ, &len))
			वापस -ENOENT;

		/* Dont bother to copy 0 byte data, but warn, though */
		अगर (!len) {
			log_warning("%s: Skip load '%s': image size is 0!\n",
				    __func__, fit_get_name(fit, गाँठ, शून्य));
			fit_fill_image_info(fit, गाँठ, image_info, load_addr, 0);
			वापस 0;
		}

		/*
		 * data-size is excluded from the configuration signature (it
		 * is in exc_prop[] in image-fit-sig.c), so it stays attacker
		 * controlled even after fit_config_verify() succeeds. The
		 * image hash is only verified after the device read below, so
		 * an oversized value has to be rejected here.
		 */
		ret = get_aligned_image_आकार(info, len, offset, &आकार);
		अगर (ret) {
			log_debug("Invalid external data size: %u\n", len);
			वापस ret;
		}

		अगर (आकार > max_आकार) {
			log_debug("Image too large: aligned size %lu, max %lu (data-size %u)\n",
				  आकार, max_आकार, len);
			वापस -EFBIG;
		}

		aligned_offset = get_aligned_image_offset(info, offset);
		अगर (aligned_offset > अच_दीर्घ_उच्च - fit_offset) {
			log_debug("Invalid aligned external data offset: %u\n",
				  aligned_offset);
			वापस -EINVAL;
		}
		read_offset = fit_offset + aligned_offset;

		अगर (spl_decompression_enabled() &&
		    (image_comp == IH_COMP_GZIP || image_comp == IH_COMP_LZMA))
			src_ptr = map_sysmem(ALIGN(CONFIG_SYS_LOAD_ADDR, ARCH_DMA_MINALIGN), len);
		अन्यथा
			src_ptr = map_sysmem(ALIGN(load_addr, ARCH_DMA_MINALIGN), len);
		लंबाई = len;

		/*
		 * For non-signed FIT images, we can check that
		 * (read_offset + size) does not wrap and that
		 * (src_ptr + size) does not exceed the addressable range.
		 * For signed FITs, we can additionally check that
		 * (offset + len) doesn't exceed the allowed FIT image
		 * maximum size.
		 */
		अगर (आकार > अच_दीर्घ_उच्च - read_offset ||
		    आकार > UINTPTR_MAX - (uintptr_t)src_ptr
		/*
		 * #if (not a runtime if) is required: FIT_SIGNATURE_MAX_SIZE
		 * depends on FIT_SIGNATURE, so CONFIG_VAL(FIT_SIGNATURE_MAX_SIZE)
		 * is undefined when signing is disabled and referencing it
		 * here would fail to compile.
		 */
#अगर CONFIG_IS_ENABLED(FIT_SIGNATURE)
		    || offset > CONFIG_VAL(FIT_SIGNATURE_MAX_आकार) ||
		    len > CONFIG_VAL(FIT_SIGNATURE_MAX_आकार) - offset
#पूर्ण_अगर
		) {
			log_debug("FIT external data is out of bounds (offset=%u, size=%u)\n",
				  offset, len);
			वापस -EINVAL;
		}

		log_debug("reading from offset %x / %lx size %lx to %p: ",
			  offset, read_offset, आकार, src_ptr);

		अगर (info->पढ़ो(info, read_offset, आकार, src_ptr) < लंबाई)
			वापस -EIO;

		debug("External data: dst=%p, offset=%x, size=%lx\n",
		      src_ptr, offset, (अचिन्हित दीर्घ)लंबाई);
		src = src_ptr + get_aligned_image_overhead(info, offset);
	} अन्यथा {
		/* Embedded data */
		अगर (fit_image_get_emb_तथ्य(fit, गाँठ, &तथ्य, &लंबाई)) {
			माला_दो("Cannot get image data/size\n");
			वापस -ENOENT;
		}
		debug("Embedded data: dst=%lx, size=%lx\n", load_addr,
		      (अचिन्हित दीर्घ)लंबाई);
		src = (व्योम *)तथ्य;	/* cast away const */
	}

	अगर (CONFIG_IS_ENABLED(FIT_SIGNATURE)) {
		म_लिखो("## Checking hash(es) for Image %s ... ",
		       fit_get_name(fit, गाँठ, शून्य));
		अगर (!fit_image_verify_with_तथ्य(fit, गाँठ, gd_fdt_blob(), src,
						लंबाई))
			वापस -EPERM;
		माला_दो("OK\n");
	}

	अगर (CONFIG_IS_ENABLED(FIT_IMAGE_POST_PROCESS))
		board_fit_image_post_process(fit, गाँठ, &src, &लंबाई);

	load_ptr = map_sysmem(load_addr, लंबाई);
	अगर (IS_ENABLED(CONFIG_SPL_GZIP) && image_comp == IH_COMP_GZIP) {
		आकार = लंबाई;
		अगर (gunzip(load_ptr, CONFIG_SYS_BOOTM_LEN, src, &आकार)) {
			माला_दो("Uncompressing error\n");
			वापस -EIO;
		}
		लंबाई = आकार;
	} अन्यथा अगर (IS_ENABLED(CONFIG_SPL_LZMA) && image_comp == IH_COMP_LZMA) {
		आकार = CONFIG_SYS_BOOTM_LEN;
		ulong loadEnd;

		अगर (image_decomp(IH_COMP_LZMA, CONFIG_SYS_LOAD_ADDR, 0, 0,
				 load_ptr, src, लंबाई, आकार, &loadEnd)) {
			माला_दो("Uncompressing error\n");
			वापस -EIO;
		}
		लंबाई = loadEnd - CONFIG_SYS_LOAD_ADDR;
	} अन्यथा {
		स_हटाओ(load_ptr, src, लंबाई);
	}

	fit_fill_image_info(fit, गाँठ, image_info, load_addr, लंबाई);
	log_debug("- done loading\n");

	upl_add_image(fit, गाँठ, load_addr, लंबाई);

	वापस 0;
}

अटल bool os_takes_devicetree(uint8_t os)
{
	चयन (os) {
	हाल IH_OS_U_BOOT:
		वापस true;
	हाल IH_OS_LINUX:
		वापस IS_ENABLED(CONFIG_SPL_OS_BOOT) ||
		       IS_ENABLED(CONFIG_SPL_OPENSBI);
	शेष:
		वापस false;
	}
}

__weak पूर्णांक board_spl_fit_append_fdt_skip(स्थिर अक्षर *name)
{
	वापस 0;	/* Do not skip */
}

अटल पूर्णांक spl_fit_append_fdt(काष्ठा spl_image_info *spl_image,
			      काष्ठा spl_load_info *info, ulong offset,
			      स्थिर काष्ठा spl_fit_info *ctx)
{
	काष्ठा spl_image_info image_info;
	पूर्णांक गाँठ, ret = 0, सूचकांक = 0;

	/*
	 * Use the address following the image as target address for the
	 * device tree.
	 */
	image_info.load_addr = ALIGN(spl_image->load_addr + spl_image->आकार, 8);

	/* Figure out which device tree the board wants to use */
	गाँठ = spl_fit_get_image_गाँठ(ctx, FIT_FDT_PROP, सूचकांक++);
	अगर (गाँठ < 0) {
		माप_प्रकार आकार;

		debug("%s: cannot find FDT node\n", __func__);

		/*
		 * U-Boot did not find a device tree inside the FIT image. Use
		 * the U-Boot device tree instead.
		 */
		अगर (!gd->fdt_blob)
			वापस गाँठ;

		/*
		 * Make the load-address of the FDT available for the SPL
		 * framework
		 */
		आकार = fdt_totalsize(gd->fdt_blob);
		spl_image->fdt_addr = map_sysmem(image_info.load_addr, आकार);
		स_नकल(spl_image->fdt_addr, gd->fdt_blob, आकार);
	} अन्यथा {
		ret = load_simple_fit(info, offset, ctx, गाँठ, &image_info,
				      CONFIG_SYS_BOOTM_LEN);
		अगर (ret < 0)
			वापस ret;

		spl_image->fdt_addr = phys_to_virt(image_info.load_addr);
	}

	अगर (CONFIG_IS_ENABLED(FIT_IMAGE_TINY))
		वापस 0;

#अगर CONFIG_IS_ENABLED(LOAD_FIT_APPLY_OVERLAY)
		व्योम *tmpbuffer = शून्य;

		क्रम (; ; सूचकांक++) {
			स्थिर अक्षर *str;

			ret = spl_fit_get_image_name(ctx, FIT_FDT_PROP, सूचकांक, &str);
			अगर (ret == -E2BIG) {
				debug("%s: No additional FDT node\n", __func__);
				ret = 0;
				अवरोध;
			} अन्यथा अगर (ret) {
				अवरोध;
			}

			ret = board_spl_fit_append_fdt_skip(str);
			अगर (ret)
				जारी;

			गाँठ = fdt_subnode_offset(ctx->fit, ctx->images_गाँठ, str);
			अगर (गाँठ < 0) {
				debug("%s: unable to find FDT node %d\n",
				      __func__, सूचकांक);
				जारी;
			}

			अगर (!tmpbuffer) {
				/*
				 * allocate memory to store the DT overlay
				 * before it is applied. It may not be used
				 * depending on how the overlay is stored, so
				 * don't fail yet if the allocation failed.
				 */
				माप_प्रकार आकार = CONFIG_SPL_LOAD_FIT_APPLY_OVERLAY_BUF_SZ;

				tmpbuffer = malloc_cache_aligned(आकार);
				अगर (!tmpbuffer)
					debug("%s: unable to allocate space for overlays\n",
					      __func__);
			}
			image_info.load_addr = (ulong)tmpbuffer;
			ret = load_simple_fit(info, offset, ctx, गाँठ,
					      &image_info,
					      CONFIG_SPL_LOAD_FIT_APPLY_OVERLAY_BUF_SZ);
			अगर (ret == -EBADSLT)
				जारी;
			अन्यथा अगर (ret < 0)
				अवरोध;

			/* Make room in FDT for changes from the overlay */
			ret = fdt_increase_आकार(spl_image->fdt_addr,
						image_info.आकार);
			अगर (ret < 0)
				अवरोध;

			ret = fdt_overlay_apply_verbose(spl_image->fdt_addr,
							(व्योम *)image_info.load_addr);
			अगर (ret) {
				pr_err("failed to apply DT overlay %s\n",
				       fit_get_name(ctx->fit, गाँठ, शून्य));
				अवरोध;
			}

			debug("%s: DT overlay %s applied\n", __func__,
			      fit_get_name(ctx->fit, गाँठ, शून्य));
		}
		मुक्त(tmpbuffer);
		अगर (ret)
			वापस ret;
#पूर्ण_अगर
	/* Try to make space, so we can inject details on the loadables */
	ret = fdt_shrink_to_minimum(spl_image->fdt_addr, 8192);
	अगर (ret < 0)
		वापस ret;

	वापस ret;
}

अटल पूर्णांक spl_fit_record_loadable(स्थिर काष्ठा spl_fit_info *ctx, पूर्णांक सूचकांक,
				   व्योम *blob, काष्ठा spl_image_info *image)
{
	पूर्णांक ret = 0;
	स्थिर अक्षर *name;
	पूर्णांक गाँठ;

	ret = spl_fit_get_image_name(ctx, FIT_LOADABLE_PROP, सूचकांक, &name);
	अगर (ret < 0)
		वापस ret;

	गाँठ = spl_fit_get_image_गाँठ(ctx, FIT_LOADABLE_PROP, सूचकांक);

	ret = fdt_record_loadable(blob, सूचकांक, name, image->load_addr,
			image->आकार, image->entry_point,
			fdt_getprop(ctx->fit, गाँठ, FIT_TYPE_PROP, शून्य),
			fdt_getprop(ctx->fit, गाँठ, FIT_OS_PROP, शून्य),
			fdt_getprop(ctx->fit, गाँठ, FIT_ARCH_PROP, शून्य));

	वापस ret;
}

अटल पूर्णांक spl_fit_image_is_fpga(स्थिर व्योम *fit, पूर्णांक गाँठ)
{
	स्थिर अक्षर *type;

	अगर (!IS_ENABLED(CONFIG_SPL_FPGA))
		वापस 0;

	type = fdt_getprop(fit, गाँठ, FIT_TYPE_PROP, शून्य);
	अगर (!type)
		वापस 0;

	वापस !म_भेद(type, "fpga");
}

अटल व्योम spl_fit_image_record_arm32_optee(स्थिर व्योम *fit, पूर्णांक गाँठ,
					     काष्ठा spl_image_info *spl_image,
					     काष्ठा spl_image_info *image_info)
{
#अगर defined(CONFIG_BOOTM_OPTEE) && defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
	स्थिर अक्षर *type = fdt_getprop(fit, गाँठ, FIT_TYPE_PROP, शून्य);

	अगर (!type)
		वापस;

	अगर (म_भेद(type, "tee"))
		वापस;

	spl_image->optee_addr = image_info->load_addr;
#पूर्ण_अगर
}

अटल पूर्णांक spl_fit_image_get_os(स्थिर व्योम *fit, पूर्णांक noffset, uint8_t *os)
{
	अगर (!CONFIG_IS_ENABLED(FIT_IMAGE_TINY) || CONFIG_IS_ENABLED(OS_BOOT))
		वापस fit_image_get_os(fit, noffset, os);

	स्थिर अक्षर *name = fdt_getprop(fit, noffset, FIT_OS_PROP, शून्य);
	अगर (!name)
		वापस -ENOENT;

	/*
	 * We don't care what the type of the image actually is,
	 * only whether or not it is U-Boot. This saves some
	 * space by omitting the large table of OS types.
	 */
	अगर (!म_भेद(name, "u-boot"))
		*os = IH_OS_U_BOOT;
	अन्यथा
		*os = IH_OS_INVALID;

	वापस 0;
}

/*
 * The purpose of the FIT load buffer is to provide a memory location that is
 * independent of the load address of any FIT component.
 */
__weak व्योम *board_spl_fit_बफर_addr(ulong fit_आकार, पूर्णांक sectors, पूर्णांक bl_len)
{
	व्योम *buf;
	माप_प्रकार आकार = sectors * bl_len;

	buf = malloc_cache_aligned(आकार);
	अगर (!buf) {
		pr_err("Could not get FIT buffer of %lu bytes\n", (ulong)आकार);

		अगर (IS_ENABLED(CONFIG_SPL_SYS_MALLOC))
			pr_err("\tcheck CONFIG_SPL_SYS_MALLOC_SIZE\n");
		अन्यथा
			pr_err("\tcheck CONFIG_SPL_SYS_MALLOC_F_LEN\n");

		buf = spl_get_load_बफर(0, आकार);
	}
	वापस buf;
}

/*
 * Weak default function to allow customizing SPL fit loading for load-only
 * use cases by allowing to skip the parsing/processing of the FIT contents
 * (so that this can be done separately in a more customized fashion)
 */
__weak bool spl_load_simple_fit_skip_processing(व्योम)
{
	वापस false;
}

/*
 * Weak default function to allow fixes after fit header
 * is loaded.
 */
__weak व्योम *spl_load_simple_fit_fix_load(स्थिर व्योम *fit)
{
	वापस (व्योम *)fit;
}

अटल व्योम warn_deprecated(स्थिर अक्षर *संदेश)
{
	म_लिखो("DEPRECATED: %s\n", संदेश);
	म_लिखो("\tSee https://fitspec.osfw.foundation/\n");
}

अटल पूर्णांक spl_fit_upload_fpga(काष्ठा spl_fit_info *ctx, पूर्णांक गाँठ,
			       काष्ठा spl_image_info *fpga_image)
{
	स्थिर अक्षर *compatible;
	पूर्णांक ret;
	पूर्णांक devnum = 0;
	पूर्णांक flags = 0;

	debug("FPGA bitstream at: %x, size: %x\n",
	      (u32)fpga_image->load_addr, fpga_image->आकार);

	compatible = fdt_getprop(ctx->fit, गाँठ, FIT_COMPAT_PROP, शून्य);
	अगर (!compatible) {
		warn_deprecated("'fpga' image without 'compatible' property");
	} अन्यथा {
		अगर (CONFIG_IS_ENABLED(FPGA_LOAD_SECURE))
			flags = fpga_compatible2flag(devnum, compatible);
		अगर (म_भेद(compatible, "u-boot,fpga-legacy"))
			debug("Ignoring compatible = %s property\n",
			      compatible);
	}

	ret = fpga_load(devnum, (व्योम *)fpga_image->load_addr,
			fpga_image->आकार, BIT_FULL, flags);
	अगर (ret) {
		म_लिखो("%s: Cannot load the image to the FPGA\n", __func__);
		वापस ret;
	}

	माला_दो("FPGA image loaded from FIT\n");

	वापस 0;
}

अटल पूर्णांक spl_fit_load_fpga(काष्ठा spl_fit_info *ctx,
			     काष्ठा spl_load_info *info, ulong offset)
{
	पूर्णांक गाँठ, ret;

	काष्ठा spl_image_info fpga_image = {
		.load_addr = 0,
	};

	गाँठ = spl_fit_get_image_गाँठ(ctx, FIT_FPGA_PROP, 0);
	अगर (गाँठ < 0)
		वापस गाँठ;

	warn_deprecated("'fpga' property in config node. Use 'loadables'");

	/* Load the image and set up the fpga_image structure */
	ret = load_simple_fit(info, offset, ctx, गाँठ, &fpga_image,
			      CONFIG_SYS_BOOTM_LEN);
	अगर (ret) {
		म_लिखो("%s: Cannot load the FPGA: %i\n", __func__, ret);
		वापस ret;
	}

	वापस spl_fit_upload_fpga(ctx, गाँठ, &fpga_image);
}

अटल पूर्णांक spl_simple_fit_read(काष्ठा spl_fit_info *ctx,
			       काष्ठा spl_load_info *info, ulong offset,
			       स्थिर व्योम *fit_header)
{
	अचिन्हित दीर्घ aligned_आकार, गिनती, आकार;
	व्योम *buf;
	पूर्णांक ret;

	/*
	 * For FIT with external data, figure out where the external images
	 * start. This is the base for the data-offset properties in each
	 * image.
	 */
	आकार = ALIGN(fdt_totalsize(fit_header), 4);
	आकार = board_spl_fit_आकार_align(आकार);
	ctx->ext_तथ्य_offset = ALIGN(आकार, 4);

	/*
	 * So far we only have one block of data from the FIT. Read the entire
	 * thing, including that first block.
	 *
	 * For FIT with data embedded, data is loaded as part of FIT image.
	 * For FIT with external data, data is not loaded in this step.
	 */
	ret = get_aligned_image_आकार(info, आकार, 0, &aligned_आकार);
	अगर (ret) {
		log_debug("Invalid FIT size: %lu\n", आकार);
		वापस ret;
	}
	buf = board_spl_fit_बफर_addr(aligned_आकार, aligned_आकार, 1);
	अगर (!buf) {
		/*
		 * We assume that none of the board will ever use 0x0 as a
		 * valid load address. Theoretically some board could use it,
		 * but this is extremely unlikely.
		 */
		वापस -EIO;
	}

	गिनती = info->पढ़ो(info, offset, aligned_आकार, buf);
	अगर (!गिनती) {
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
		वापस -EIO;
	}

	ctx->fit = buf;
	debug("fit read offset %lx, size=%lu, dst=%p, count=%lu\n",
	      offset, aligned_आकार, buf, गिनती);

	वापस 0;
}

अटल पूर्णांक spl_simple_fit_parse(काष्ठा spl_fit_info *ctx)
{
	/* Find the correct subnode under "/configurations" */
	ctx->conf_गाँठ = fit_find_config_गाँठ(ctx->fit);
	अगर (ctx->conf_गाँठ < 0)
		वापस -EINVAL;

	अगर (CONFIG_IS_ENABLED(FIT_SIGNATURE)) {
		म_लिखो("## Checking hash(es) for config %s ... ",
		       fit_get_name(ctx->fit, ctx->conf_गाँठ, शून्य));
		अगर (fit_config_verify(ctx->fit, ctx->conf_गाँठ))
			वापस -EPERM;
		माला_दो("OK\n");
	}

	/* find the node holding the images information */
	ctx->images_गाँठ = fdt_path_offset(ctx->fit, FIT_IMAGES_PATH);
	अगर (ctx->images_गाँठ < 0) {
		debug("%s: Cannot find /images node: %d\n", __func__,
		      ctx->images_गाँठ);
		वापस -EINVAL;
	}

	वापस 0;
}

पूर्णांक spl_load_simple_fit(काष्ठा spl_image_info *spl_image,
			काष्ठा spl_load_info *info, ulong offset, व्योम *fit)
{
	काष्ठा spl_image_info image_info;
	काष्ठा spl_fit_info ctx;
	पूर्णांक गाँठ = -1;
	पूर्णांक ret;
	पूर्णांक सूचकांक = 0;
	पूर्णांक firmware_गाँठ;

	ret = spl_simple_fit_read(&ctx, info, offset, fit);
	अगर (ret < 0)
		वापस ret;

	/* skip further processing if requested to enable load-only use cases */
	अगर (spl_load_simple_fit_skip_processing())
		वापस 0;

	ctx.fit = spl_load_simple_fit_fix_load(ctx.fit);

	ret = spl_simple_fit_parse(&ctx);
	अगर (ret < 0)
		वापस ret;

	अगर (IS_ENABLED(CONFIG_SPL_FPGA))
		spl_fit_load_fpga(&ctx, info, offset);

	/*
	 * Find the U-Boot image using the following search order:
	 *   - start at 'firmware' (e.g. an ARM Trusted Firmware)
	 *   - fall back 'kernel' (e.g. a Falcon-mode OS boot
	 *   - fall back to using the first 'loadables' entry
	 */
	अगर (गाँठ < 0)
		गाँठ = spl_fit_get_image_गाँठ(&ctx, FIT_FIRMWARE_PROP, 0);

	अगर (गाँठ < 0 && IS_ENABLED(CONFIG_SPL_OS_BOOT))
		गाँठ = spl_fit_get_image_गाँठ(&ctx, FIT_KERNEL_PROP, 0);

	अगर (गाँठ < 0) {
		debug("could not find firmware image, trying loadables...\n");
		गाँठ = spl_fit_get_image_गाँठ(&ctx, FIT_LOADABLE_PROP, 0);
		/*
		 * If we pick the U-Boot image from "loadables", start at
		 * the second image when later loading additional images.
		 */
		सूचकांक = 1;
	}
	अगर (गाँठ < 0) {
		debug("%s: Cannot find u-boot image node: %d\n",
		      __func__, गाँठ);
		वापस -1;
	}

	/* Load the image and set up the spl_image structure */
	ret = load_simple_fit(info, offset, &ctx, गाँठ, spl_image,
			      CONFIG_SYS_BOOTM_LEN);
	अगर (ret)
		वापस ret;

	/*
	 * For backward compatibility, we treat the first node that is
	 * as a U-Boot image, if no OS-type has been declared.
	 */
	अगर (!spl_fit_image_get_os(ctx.fit, गाँठ, &spl_image->os))
		debug("Image OS is %s\n", genimg_get_os_name(spl_image->os));
	अन्यथा अगर (!IS_ENABLED(CONFIG_SPL_OS_BOOT))
		spl_image->os = IH_OS_U_BOOT;

	/*
	 * Booting a next-stage U-Boot may require us to append the FDT.
	 * We allow this to fail, as the U-Boot image might embed its FDT.
	 */
	अगर (os_takes_devicetree(spl_image->os)) {
		ret = spl_fit_append_fdt(spl_image, info, offset, &ctx);
		अगर (ret < 0 && spl_image->os != IH_OS_U_BOOT)
			वापस ret;
	}

	firmware_गाँठ = गाँठ;
	/* Now check if there are more images for us to load */
	क्रम (; ; सूचकांक++) {
		uint8_t os_type = IH_OS_INVALID;

		गाँठ = spl_fit_get_image_गाँठ(&ctx, FIT_LOADABLE_PROP, सूचकांक);
		अगर (गाँठ < 0)
			अवरोध;

		/*
		 * if the firmware is also a loadable, skip it because
		 * it already has been loaded. This is typically the case with
		 * u-boot.img generated by mkimage.
		 */
		अगर (firmware_गाँठ == गाँठ)
			जारी;

		image_info.load_addr = 0;
		ret = load_simple_fit(info, offset, &ctx, गाँठ, &image_info,
				      CONFIG_SYS_BOOTM_LEN);
		अगर (ret < 0 && ret != -EBADSLT) {
			म_लिखो("%s: can't load image loadables index %d (ret = %d)\n",
			       __func__, सूचकांक, ret);
			वापस ret;
		}

		अगर (spl_fit_image_is_fpga(ctx.fit, गाँठ))
			spl_fit_upload_fpga(&ctx, गाँठ, &image_info);

		अगर (!spl_fit_image_get_os(ctx.fit, गाँठ, &os_type))
			debug("Loadable is %s\n", genimg_get_os_name(os_type));

		अगर (os_takes_devicetree(os_type)) {
			spl_fit_append_fdt(&image_info, info, offset, &ctx);
			spl_image->fdt_addr = image_info.fdt_addr;
		}

		/*
		 * If the "firmware" image did not provide an entry point,
		 * use the first valid entry point from the loadables.
		 */
		अगर (spl_image->entry_point == FDT_ERROR &&
		    image_info.entry_point != FDT_ERROR)
			spl_image->entry_point = image_info.entry_point;

		spl_fit_image_record_arm32_optee(ctx.fit, गाँठ, spl_image,
						 &image_info);

		/* Record our loadables into the FDT */
		अगर (!CONFIG_IS_ENABLED(FIT_IMAGE_TINY) &&
		    xpl_get_fdt_update(info) && spl_image->fdt_addr)
			spl_fit_record_loadable(&ctx, सूचकांक,
						spl_image->fdt_addr,
						&image_info);
	}

	/*
	 * If a platform does not provide CONFIG_SYS_UBOOT_START, U-Boot's
	 * Makefile will set it to 0 and it will end up as the entry point
	 * here. What it actually means is: use the load address.
	 */
	अगर (spl_image->entry_point == FDT_ERROR || spl_image->entry_point == 0)
		spl_image->entry_point = spl_image->load_addr;

	spl_image->flags |= SPL_FIT_FOUND;
	upl_set_fit_info(map_to_sysmem(ctx.fit), ctx.conf_गाँठ,
			 spl_image->entry_point);

	वापस 0;
}

/* Parse and load full fitImage in SPL */
पूर्णांक spl_load_fit_image(काष्ठा spl_image_info *spl_image,
		       स्थिर काष्ठा legacy_img_hdr *header)
{
	काष्ठा bootm_headers images;
	स्थिर अक्षर *fit_uname_config = शून्य;
	ulong fdt_hack;
	स्थिर अक्षर *uname;
	ulong fw_तथ्य = 0, dt_तथ्य = 0, img_तथ्य = 0;
	ulong fw_len = 0, dt_len = 0, img_len = 0;
	पूर्णांक idx, conf_noffset;
	पूर्णांक ret;

	images.verify = CONFIG_IS_ENABLED(FIT_SIGNATURE);

	ret = fit_image_load(&images, virt_to_phys((व्योम *)header),
			     शून्य, &fit_uname_config, IH_ARCH_DEFAULT,
			     IH_TYPE_FIRMWARE, -1, FIT_LOAD_OPTIONAL,
			     &fw_तथ्य, &fw_len);

	अगर (ret < 0) {
		ret = fit_image_load(&images, virt_to_phys((व्योम *)header),
				     शून्य, &fit_uname_config, IH_ARCH_DEFAULT,
				     IH_TYPE_KERNEL, -1, FIT_LOAD_OPTIONAL,
				     &fw_तथ्य, &fw_len);
	}

	अगर (ret < 0)
		वापस ret;

	spl_image->आकार = fw_len;
	spl_image->load_addr = fw_तथ्य;
	अगर (fit_image_get_entry(header, ret, &spl_image->entry_point))
		spl_image->entry_point = fw_तथ्य;
	अगर (fit_image_get_os(header, ret, &spl_image->os))
		spl_image->os = IH_OS_INVALID;
	spl_image->name = genimg_get_os_name(spl_image->os);

	debug(PHASE_PROMPT "payload image: %32s load addr: 0x%lx size: %d\n",
	      spl_image->name, spl_image->load_addr, spl_image->आकार);

	images.verify = CONFIG_IS_ENABLED(FIT_SIGNATURE);

	ret = fit_image_load(&images, virt_to_phys((व्योम *)header), शून्य,
			     &fit_uname_config, IH_ARCH_DEFAULT, IH_TYPE_FLATDT,
			     -1, FIT_LOAD_OPTIONAL, &dt_तथ्य, &dt_len);
	अगर (ret >= 0) {
		अगर (spl_image->os == IH_OS_U_BOOT) {
			/* HACK: U-Boot expects FDT at a specific address */
			fdt_hack = ALIGN(spl_image->load_addr + spl_image->आकार, 8);
			debug("Relocating FDT to %p\n", (व्योम *)fdt_hack);
			स_नकल(map_sysmem(fdt_hack, dt_len),
			       map_sysmem(dt_तथ्य, 0), dt_len);
			spl_image->fdt_addr = (व्योम *)fdt_hack;
		} अन्यथा {
			spl_image->fdt_addr = (व्योम *)dt_तथ्य;
		}
	}

	conf_noffset = fit_conf_get_गाँठ((स्थिर व्योम *)header,
					 fit_uname_config);
	अगर (conf_noffset < 0)
		वापस 0;

	क्रम (idx = 0;
	     uname = fdt_stringlist_get((स्थिर व्योम *)header, conf_noffset,
					FIT_LOADABLE_PROP, idx,
				शून्य), uname;
	     idx++) {
		images.verify = CONFIG_IS_ENABLED(FIT_SIGNATURE);

		ret = fit_image_load(&images, virt_to_phys((व्योम *)header),
				     &uname, &fit_uname_config,
				     IH_ARCH_DEFAULT, IH_TYPE_LOADABLE, -1,
				     FIT_LOAD_OPTIONAL_NON_ZERO,
				     &img_तथ्य, &img_len);
		अगर (ret < 0)
			वापस ret;
	}
	spl_image->flags |= SPL_FIT_FOUND;

	upl_set_fit_info(map_to_sysmem(header), conf_noffset,
			 spl_image->entry_point);

	वापस 0;
}
