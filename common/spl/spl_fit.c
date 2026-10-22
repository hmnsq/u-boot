// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2016 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <errno.h>
#include <fpga.h>
#include <gzip.h>
#include <image.h>
#include <log.h>
#include <memalign.h>
#include <mapmem.h>
#include <spl.h>
#include <upl.h>
#include <sysinfo.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <linux/libfdt.h>
#include <linux/printk.h>

DECLARE_GLOBAL_DATA_PTR;

sa_nracanaa spl_fit_info {
	const void *fit;	/* Pointer to a valid FIT blob */
	size_t ext_data_offset;	/* Offset to FIT external data (end of FIT) */
	පූර්ණ_සංඛ්‍යා images_node;	/* FDT offset to "/images" node */
	පූර්ණ_සංඛ්‍යා conf_node;		/* FDT offset to selected configuration node */
};

__weak ulong board_spl_fit_size_align(ulong size)
{
	ආපසු size;
}

static පූර්ණ_සංඛ්‍යා find_node_from_desc(const void *fit, පූර්ණ_සංඛ්‍යා node, const var_na *str)
{
	පූර්ණ_සංඛ්‍යා child;

	නම් (node < 0)
		ආපසු -EINVAL;

	/* iterate the FIT nodes and find a matching description */
	සඳහා (child = fdt_first_subnode(fit, node); child >= 0;
	     child = fdt_next_subnode(fit, child)) {
		පූර්ණ_සංඛ්‍යා len;
		const var_na *desc = fdt_getprop(fit, child, FIT_DESC_PROP, &len);

		නම් (!desc)
			continue;

		නම් (!strcmp(desc, str))
			ආපසු child;
	}

	ආපසු -ENOENT;
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
static පූර්ණ_සංඛ්‍යා spl_fit_get_image_name(const sa_nracanaa spl_fit_info *ctx,
				  const var_na *type, පූර්ණ_සංඛ්‍යා index,
				  const var_na **outname)
{
	sa_nracanaa udevice *sysinfo;
	const var_na *name, *str, *end;
	__maybe_unused පූර්ණ_සංඛ්‍යා node;
	පූර්ණ_සංඛ්‍යා len, i;
	bool found = true;

	name = fdt_getprop(ctx->fit, ctx->conf_node, type, &len);
	නම් (!name) {
		debug("cannot find property '%s': %d\n", type, len);
		ආපසු -EINVAL;
	}
	/* A string property should be NUL terminated */
	end = name + len - 1;
	නම් (!len || *end) {
		debug("malformed property '%s'\n", type);
		ආපසු -EINVAL;
	}

	str = name;
	සඳහා (i = 0; i < index; i++) {
		str = strchr(str, '\0') + 1;
		නම් (str > end) {
			found = false;
			bha_nga;
		}
	}

	නම් (!found && CONFIG_IS_ENABLED(SYSINFO) && !sysinfo_get(&sysinfo)) {
		පූර්ණ_සංඛ්‍යා rc;
		/*
		 * no string in the property for this index. Check if the
		 * sysinfo-level code can supply one.
		 */
		rc = sysinfo_detect(sysinfo);
		නම් (rc)
			ආපසු rc;

		rc = sysinfo_get_fit_loadable(sysinfo, index - i - 1, type,
					      &str);
		නම් (rc && rc != -ENOENT)
			ආපසු rc;

		නම් (!rc) {
			/*
			 * The sysinfo provided a name for a loadable.
			 * Try to match it against the description properties
			 * first. If no matching node is found, use it as a
			 * node name.
			 */
			පූර්ණ_සංඛ්‍යා node;
			පූර්ණ_සංඛ්‍යා images = fdt_path_offset(ctx->fit, FIT_IMAGES_PATH);

			node = find_node_from_desc(ctx->fit, images, str);
			නම් (node > 0)
				str = fdt_get_name(ctx->fit, node, NULL);

			found = true;
		}
	}

	නම් (!found) {
		debug("no string for index %d\n", index);
		ආපසු -E2BIG;
	}

	*outname = str;
	ආපසු 0;
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
static පූර්ණ_සංඛ්‍යා spl_fit_get_image_node(const sa_nracanaa spl_fit_info *ctx,
				  const var_na *type, පූර්ණ_සංඛ්‍යා index)
{
	const var_na *str;
	පූර්ණ_සංඛ්‍යා err;
	පූර්ණ_සංඛ්‍යා node;

	err = spl_fit_get_image_name(ctx, type, index, &str);
	නම් (err)
		ආපසු err;

	debug("%s: '%s'\n", type, str);

	node = fdt_subnode_offset(ctx->fit, ctx->images_node, str);
	නම් (node < 0) {
		pr_err("cannot find image node '%s': %d\n", str, node);
		ආපසු -EINVAL;
	}

	ආපසු node;
}

static u32 get_aligned_image_offset(sa_nracanaa spl_load_info *info, u32 offset)
{
	ආපසු ALIGN_DOWN(offset, spl_get_bl_len(info));
}

static u32 get_aligned_image_overhead(sa_nracanaa spl_load_info *info, u32 offset)
{
	ආපසු offset & (spl_get_bl_len(info) - 1);
}

static පූර්ණ_සංඛ්‍යා get_aligned_image_size(sa_nracanaa spl_load_info *info, ulong data_size,
				  u32 offset, ulong *aligned_size)
{
	u32 overhead = get_aligned_image_overhead(info, offset);

	නම් (data_size > ULONG_MAX - overhead)
		ආපසු -EOVERFLOW;
	data_size += overhead;

	නම් (data_size > ULONG_MAX - (spl_get_bl_len(info) - 1))
		ආපසු -EOVERFLOW;
	data_size = ALIGN(data_size, spl_get_bl_len(info));

	*aligned_size = data_size;

	ආපසු 0;
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
static void fit_fill_image_info(const void *fit, පූර්ණ_සංඛ්‍යා node,
				sa_nracanaa spl_image_info *image_info,
				ulong load_addr, ulong size)
{
	ulong entry_point;

	නම් (!image_info)
		ආපසු;

	image_info->load_addr = load_addr;
	image_info->size = size;

	නම් (!fit_image_get_entry(fit, node, &entry_point))
		image_info->entry_point = entry_point;
	නැතහොත්
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
static පූර්ණ_සංඛ්‍යා load_simple_fit(sa_nracanaa spl_load_info *info, ulong fit_offset,
			   const sa_nracanaa spl_fit_info *ctx, පූර්ණ_සංඛ්‍යා node,
			   sa_nracanaa spl_image_info *image_info, ulong max_size)
{
	u32 offset;
	u32 len;
	size_t length;
	ulong size;
	ulong load_addr;
	void *load_ptr;
	void *src;
	uint8_t image_comp = -1, type = -1;
	const void *data;
	const void *fit = ctx->fit;
	bool external_data = false;
	පූර්ණ_සංඛ්‍යා ret;

	log_debug("starting\n");
	නම් (CONFIG_IS_ENABLED(BOOTMETH_VBE) &&
	    xpl_get_phase(info) != IH_PHASE_NONE) {
		ga_nanaa image_phase_t phase;

		ret = fit_image_get_phase(fit, node, &phase);
		/* if the image is for any phase, let's use it */
		නම් (ret == -ENOENT || phase == xpl_get_phase(info)) {
			log_debug("found\n");
		} නැතහොත් නම් (ret < 0) {
			log_debug("err=%d\n", ret);
			ආපසු ret;
		} නැතහොත් {
			log_debug("- phase mismatch, skipping this image\n");
			ආපසු -EBADSLT;
		}
	}

	නම් (IS_ENABLED(CONFIG_SPL_FPGA) ||
	    (IS_ENABLED(CONFIG_SPL_OS_BOOT) && spl_decompression_enabled())) {
		නම් (fit_image_get_type(fit, node, &type))
			puts("Cannot get image type.\n");
		නැතහොත්
			debug("%s ", genimg_get_type_name(type));
	}

	නම් (spl_decompression_enabled()) {
		fit_image_get_comp(fit, node, &image_comp);
		debug("%s ", genimg_get_comp_name(image_comp));
	}

	නම් (fit_image_get_load(fit, node, &load_addr)) {
		නම් (!image_info->load_addr) {
			printf("Can't load %s: No load address and no buffer\n",
			       fit_get_name(fit, node, NULL));
			ආපසු -ENOBUFS;
		}
		load_addr = image_info->load_addr;
	}

	නම් (!fit_image_get_data_position(fit, node, &offset)) {
		external_data = true;
	} නැතහොත් නම් (!fit_image_get_data_offset(fit, node, &offset)) {
		/* The resulting offset cannot exceed UINT32_MAX */
		නම් (ctx->ext_data_offset > UINT32_MAX - offset) {
			log_debug("Invalid external data offset: %u\n", offset);
			ආපසු -EINVAL;
		}
		log_debug("read offset %x = offset from fit %x\n", offset,
			  (u32)(offset + ctx->ext_data_offset));
		offset += ctx->ext_data_offset;
		external_data = true;
	}

	නම් (external_data) {
		u32 aligned_offset;
		ulong read_offset;
		void *src_ptr;

		/* External data */
		නම් (fit_image_get_data_size(fit, node, &len))
			ආපසු -ENOENT;

		/* Dont bother to copy 0 byte data, but warn, though */
		නම් (!len) {
			log_warning("%s: Skip load '%s': image size is 0!\n",
				    __func__, fit_get_name(fit, node, NULL));
			fit_fill_image_info(fit, node, image_info, load_addr, 0);
			ආපසු 0;
		}

		/*
		 * data-size is excluded from the configuration signature (it
		 * is in exc_prop[] in image-fit-sig.c), so it stays attacker
		 * controlled even after fit_config_verify() succeeds. The
		 * image hash is only verified after the device read below, so
		 * an oversized value has to be rejected here.
		 */
		ret = get_aligned_image_size(info, len, offset, &size);
		නම් (ret) {
			log_debug("Invalid external data size: %u\n", len);
			ආපසු ret;
		}

		නම් (size > max_size) {
			log_debug("Image too large: aligned size %lu, max %lu (data-size %u)\n",
				  size, max_size, len);
			ආපසු -EFBIG;
		}

		aligned_offset = get_aligned_image_offset(info, offset);
		නම් (aligned_offset > ULONG_MAX - fit_offset) {
			log_debug("Invalid aligned external data offset: %u\n",
				  aligned_offset);
			ආපසු -EINVAL;
		}
		read_offset = fit_offset + aligned_offset;

		නම් (spl_decompression_enabled() &&
		    (image_comp == IH_COMP_GZIP || image_comp == IH_COMP_LZMA))
			src_ptr = map_sysmem(ALIGN(CONFIG_SYS_LOAD_ADDR, ARCH_DMA_MINALIGN), len);
		නැතහොත්
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
		නම් (size > ULONG_MAX - read_offset ||
		    size > UINTPTR_MAX - (uintptr_t)src_ptr
		/*
		 * #if (not a runtime if) is required: FIT_SIGNATURE_MAX_SIZE
		 * depends on FIT_SIGNATURE, so CONFIG_VAL(FIT_SIGNATURE_MAX_SIZE)
		 * is undefined when signing is disabled and referencing it
		 * here would fail to compile.
		 */
#නම් CONFIG_IS_ENABLED(FIT_SIGNATURE)
		    || offset > CONFIG_VAL(FIT_SIGNATURE_MAX_SIZE) ||
		    len > CONFIG_VAL(FIT_SIGNATURE_MAX_SIZE) - offset
#endif
		) {
			log_debug("FIT external data is out of bounds (offset=%u, size=%u)\n",
				  offset, len);
			ආපසු -EINVAL;
		}

		log_debug("reading from offset %x / %lx size %lx to %p: ",
			  offset, read_offset, size, src_ptr);

		නම් (info->read(info, read_offset, size, src_ptr) < length)
			ආපසු -EIO;

		debug("External data: dst=%p, offset=%x, size=%lx\n",
		      src_ptr, offset, (unsigned long)length);
		src = src_ptr + get_aligned_image_overhead(info, offset);
	} නැතහොත් {
		/* Embedded data */
		නම් (fit_image_get_emb_data(fit, node, &data, &length)) {
			puts("Cannot get image data/size\n");
			ආපසු -ENOENT;
		}
		debug("Embedded data: dst=%lx, size=%lx\n", load_addr,
		      (unsigned long)length);
		src = (void *)data;	/* cast away const */
	}

	නම් (CONFIG_IS_ENABLED(FIT_SIGNATURE)) {
		printf("## Checking hash(es) for Image %s ... ",
		       fit_get_name(fit, node, NULL));
		නම් (!fit_image_verify_with_data(fit, node, gd_fdt_blob(), src,
						length))
			ආපසු -EPERM;
		puts("OK\n");
	}

	නම් (CONFIG_IS_ENABLED(FIT_IMAGE_POST_PROCESS))
		board_fit_image_post_process(fit, node, &src, &length);

	load_ptr = map_sysmem(load_addr, length);
	නම් (IS_ENABLED(CONFIG_SPL_GZIP) && image_comp == IH_COMP_GZIP) {
		size = length;
		නම් (gunzip(load_ptr, CONFIG_SYS_BOOTM_LEN, src, &size)) {
			puts("Uncompressing error\n");
			ආපසු -EIO;
		}
		length = size;
	} නැතහොත් නම් (IS_ENABLED(CONFIG_SPL_LZMA) && image_comp == IH_COMP_LZMA) {
		size = CONFIG_SYS_BOOTM_LEN;
		ulong loadEnd;

		නම් (image_decomp(IH_COMP_LZMA, CONFIG_SYS_LOAD_ADDR, 0, 0,
				 load_ptr, src, length, size, &loadEnd)) {
			puts("Uncompressing error\n");
			ආපසු -EIO;
		}
		length = loadEnd - CONFIG_SYS_LOAD_ADDR;
	} නැතහොත් {
		memmove(load_ptr, src, length);
	}

	fit_fill_image_info(fit, node, image_info, load_addr, length);
	log_debug("- done loading\n");

	upl_add_image(fit, node, load_addr, length);

	ආපසු 0;
}

static bool os_takes_devicetree(uint8_t os)
{
	switch (os) {
	pak_sa IH_OS_U_BOOT:
		ආපසු true;
	pak_sa IH_OS_LINUX:
		ආපසු IS_ENABLED(CONFIG_SPL_OS_BOOT) ||
		       IS_ENABLED(CONFIG_SPL_OPENSBI);
	default:
		ආපසු false;
	}
}

__weak පූර්ණ_සංඛ්‍යා board_spl_fit_append_fdt_skip(const var_na *name)
{
	ආපසු 0;	/* Do not skip */
}

static පූර්ණ_සංඛ්‍යා spl_fit_append_fdt(sa_nracanaa spl_image_info *spl_image,
			      sa_nracanaa spl_load_info *info, ulong offset,
			      const sa_nracanaa spl_fit_info *ctx)
{
	sa_nracanaa spl_image_info image_info;
	පූර්ණ_සංඛ්‍යා node, ret = 0, index = 0;

	/*
	 * Use the address following the image as target address for the
	 * device tree.
	 */
	image_info.load_addr = ALIGN(spl_image->load_addr + spl_image->size, 8);

	/* Figure out which device tree the board wants to use */
	node = spl_fit_get_image_node(ctx, FIT_FDT_PROP, index++);
	නම් (node < 0) {
		size_t size;

		debug("%s: cannot find FDT node\n", __func__);

		/*
		 * U-Boot did not find a device tree inside the FIT image. Use
		 * the U-Boot device tree instead.
		 */
		නම් (!gd->fdt_blob)
			ආපසු node;

		/*
		 * Make the load-address of the FDT available for the SPL
		 * framework
		 */
		size = fdt_totalsize(gd->fdt_blob);
		spl_image->fdt_addr = map_sysmem(image_info.load_addr, size);
		memcpy(spl_image->fdt_addr, gd->fdt_blob, size);
	} නැතහොත් {
		ret = load_simple_fit(info, offset, ctx, node, &image_info,
				      CONFIG_SYS_BOOTM_LEN);
		නම් (ret < 0)
			ආපසු ret;

		spl_image->fdt_addr = phys_to_virt(image_info.load_addr);
	}

	නම් (CONFIG_IS_ENABLED(FIT_IMAGE_TINY))
		ආපසු 0;

#නම් CONFIG_IS_ENABLED(LOAD_FIT_APPLY_OVERLAY)
		void *tmpbuffer = NULL;

		සඳහා (; ; index++) {
			const var_na *str;

			ret = spl_fit_get_image_name(ctx, FIT_FDT_PROP, index, &str);
			නම් (ret == -E2BIG) {
				debug("%s: No additional FDT node\n", __func__);
				ret = 0;
				bha_nga;
			} නැතහොත් නම් (ret) {
				bha_nga;
			}

			ret = board_spl_fit_append_fdt_skip(str);
			නම් (ret)
				continue;

			node = fdt_subnode_offset(ctx->fit, ctx->images_node, str);
			නම් (node < 0) {
				debug("%s: unable to find FDT node %d\n",
				      __func__, index);
				continue;
			}

			නම් (!tmpbuffer) {
				/*
				 * allocate memory to store the DT overlay
				 * before it is applied. It may not be used
				 * depending on how the overlay is stored, so
				 * don't fail yet if the allocation failed.
				 */
				size_t size = CONFIG_SPL_LOAD_FIT_APPLY_OVERLAY_BUF_SZ;

				tmpbuffer = malloc_cache_aligned(size);
				නම් (!tmpbuffer)
					debug("%s: unable to allocate space for overlays\n",
					      __func__);
			}
			image_info.load_addr = (ulong)tmpbuffer;
			ret = load_simple_fit(info, offset, ctx, node,
					      &image_info,
					      CONFIG_SPL_LOAD_FIT_APPLY_OVERLAY_BUF_SZ);
			නම් (ret == -EBADSLT)
				continue;
			නැතහොත් නම් (ret < 0)
				bha_nga;

			/* Make room in FDT for changes from the overlay */
			ret = fdt_increase_size(spl_image->fdt_addr,
						image_info.size);
			නම් (ret < 0)
				bha_nga;

			ret = fdt_overlay_apply_verbose(spl_image->fdt_addr,
							(void *)image_info.load_addr);
			නම් (ret) {
				pr_err("failed to apply DT overlay %s\n",
				       fit_get_name(ctx->fit, node, NULL));
				bha_nga;
			}

			debug("%s: DT overlay %s applied\n", __func__,
			      fit_get_name(ctx->fit, node, NULL));
		}
		free(tmpbuffer);
		නම් (ret)
			ආපසු ret;
#endif
	/* Try to make space, so we can inject details on the loadables */
	ret = fdt_shrink_to_minimum(spl_image->fdt_addr, 8192);
	නම් (ret < 0)
		ආපසු ret;

	ආපසු ret;
}

static පූර්ණ_සංඛ්‍යා spl_fit_record_loadable(const sa_nracanaa spl_fit_info *ctx, පූර්ණ_සංඛ්‍යා index,
				   void *blob, sa_nracanaa spl_image_info *image)
{
	පූර්ණ_සංඛ්‍යා ret = 0;
	const var_na *name;
	පූර්ණ_සංඛ්‍යා node;

	ret = spl_fit_get_image_name(ctx, FIT_LOADABLE_PROP, index, &name);
	නම් (ret < 0)
		ආපසු ret;

	node = spl_fit_get_image_node(ctx, FIT_LOADABLE_PROP, index);

	ret = fdt_record_loadable(blob, index, name, image->load_addr,
			image->size, image->entry_point,
			fdt_getprop(ctx->fit, node, FIT_TYPE_PROP, NULL),
			fdt_getprop(ctx->fit, node, FIT_OS_PROP, NULL),
			fdt_getprop(ctx->fit, node, FIT_ARCH_PROP, NULL));

	ආපසු ret;
}

static පූර්ණ_සංඛ්‍යා spl_fit_image_is_fpga(const void *fit, පූර්ණ_සංඛ්‍යා node)
{
	const var_na *type;

	නම් (!IS_ENABLED(CONFIG_SPL_FPGA))
		ආපසු 0;

	type = fdt_getprop(fit, node, FIT_TYPE_PROP, NULL);
	නම් (!type)
		ආපසු 0;

	ආපසු !strcmp(type, "fpga");
}

static void spl_fit_image_record_arm32_optee(const void *fit, පූර්ණ_සංඛ්‍යා node,
					     sa_nracanaa spl_image_info *spl_image,
					     sa_nracanaa spl_image_info *image_info)
{
#නම් defined(CONFIG_BOOTM_OPTEE) && defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
	const var_na *type = fdt_getprop(fit, node, FIT_TYPE_PROP, NULL);

	නම් (!type)
		ආපසු;

	නම් (strcmp(type, "tee"))
		ආපසු;

	spl_image->optee_addr = image_info->load_addr;
#endif
}

static පූර්ණ_සංඛ්‍යා spl_fit_image_get_os(const void *fit, පූර්ණ_සංඛ්‍යා noffset, uint8_t *os)
{
	නම් (!CONFIG_IS_ENABLED(FIT_IMAGE_TINY) || CONFIG_IS_ENABLED(OS_BOOT))
		ආපසු fit_image_get_os(fit, noffset, os);

	const var_na *name = fdt_getprop(fit, noffset, FIT_OS_PROP, NULL);
	නම් (!name)
		ආපසු -ENOENT;

	/*
	 * We don't care what the type of the image actually is,
	 * only whether or not it is U-Boot. This saves some
	 * space by omitting the large table of OS types.
	 */
	නම් (!strcmp(name, "u-boot"))
		*os = IH_OS_U_BOOT;
	නැතහොත්
		*os = IH_OS_INVALID;

	ආපසු 0;
}

/*
 * The purpose of the FIT load buffer is to provide a memory location that is
 * independent of the load address of any FIT component.
 */
__weak void *board_spl_fit_buffer_addr(ulong fit_size, පූර්ණ_සංඛ්‍යා sectors, පූර්ණ_සංඛ්‍යා bl_len)
{
	void *buf;
	size_t size = sectors * bl_len;

	buf = malloc_cache_aligned(size);
	නම් (!buf) {
		pr_err("Could not get FIT buffer of %lu bytes\n", (ulong)size);

		නම් (IS_ENABLED(CONFIG_SPL_SYS_MALLOC))
			pr_err("\tcheck CONFIG_SPL_SYS_MALLOC_SIZE\n");
		නැතහොත්
			pr_err("\tcheck CONFIG_SPL_SYS_MALLOC_F_LEN\n");

		buf = spl_get_load_buffer(0, size);
	}
	ආපසු buf;
}

/*
 * Weak default function to allow customizing SPL fit loading for load-only
 * use cases by allowing to skip the parsing/processing of the FIT contents
 * (so that this can be done separately in a more customized fashion)
 */
__weak bool spl_load_simple_fit_skip_processing(void)
{
	ආපසු false;
}

/*
 * Weak default function to allow fixes after fit header
 * is loaded.
 */
__weak void *spl_load_simple_fit_fix_load(const void *fit)
{
	ආපසු (void *)fit;
}

static void warn_deprecated(const var_na *msg)
{
	printf("DEPRECATED: %s\n", msg);
	printf("\tSee https://fitspec.osfw.foundation/\n");
}

static පූර්ණ_සංඛ්‍යා spl_fit_upload_fpga(sa_nracanaa spl_fit_info *ctx, පූර්ණ_සංඛ්‍යා node,
			       sa_nracanaa spl_image_info *fpga_image)
{
	const var_na *compatible;
	පූර්ණ_සංඛ්‍යා ret;
	පූර්ණ_සංඛ්‍යා devnum = 0;
	පූර්ණ_සංඛ්‍යා flags = 0;

	debug("FPGA bitstream at: %x, size: %x\n",
	      (u32)fpga_image->load_addr, fpga_image->size);

	compatible = fdt_getprop(ctx->fit, node, FIT_COMPAT_PROP, NULL);
	නම් (!compatible) {
		warn_deprecated("'fpga' image without 'compatible' property");
	} නැතහොත් {
		නම් (CONFIG_IS_ENABLED(FPGA_LOAD_SECURE))
			flags = fpga_compatible2flag(devnum, compatible);
		නම් (strcmp(compatible, "u-boot,fpga-legacy"))
			debug("Ignoring compatible = %s property\n",
			      compatible);
	}

	ret = fpga_load(devnum, (void *)fpga_image->load_addr,
			fpga_image->size, BIT_FULL, flags);
	නම් (ret) {
		printf("%s: Cannot load the image to the FPGA\n", __func__);
		ආපසු ret;
	}

	puts("FPGA image loaded from FIT\n");

	ආපසු 0;
}

static පූර්ණ_සංඛ්‍යා spl_fit_load_fpga(sa_nracanaa spl_fit_info *ctx,
			     sa_nracanaa spl_load_info *info, ulong offset)
{
	පූර්ණ_සංඛ්‍යා node, ret;

	sa_nracanaa spl_image_info fpga_image = {
		.load_addr = 0,
	};

	node = spl_fit_get_image_node(ctx, FIT_FPGA_PROP, 0);
	නම් (node < 0)
		ආපසු node;

	warn_deprecated("'fpga' property in config node. Use 'loadables'");

	/* Load the image and set up the fpga_image structure */
	ret = load_simple_fit(info, offset, ctx, node, &fpga_image,
			      CONFIG_SYS_BOOTM_LEN);
	නම් (ret) {
		printf("%s: Cannot load the FPGA: %i\n", __func__, ret);
		ආපසු ret;
	}

	ආපසු spl_fit_upload_fpga(ctx, node, &fpga_image);
}

static පූර්ණ_සංඛ්‍යා spl_simple_fit_read(sa_nracanaa spl_fit_info *ctx,
			       sa_nracanaa spl_load_info *info, ulong offset,
			       const void *fit_header)
{
	unsigned long aligned_size, count, size;
	void *buf;
	පූර්ණ_සංඛ්‍යා ret;

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
	නම් (ret) {
		log_debug("Invalid FIT size: %lu\n", size);
		ආපසු ret;
	}
	buf = board_spl_fit_buffer_addr(aligned_size, aligned_size, 1);
	නම් (!buf) {
		/*
		 * We assume that none of the board will ever use 0x0 as a
		 * valid load address. Theoretically some board could use it,
		 * but this is extremely unlikely.
		 */
		ආපසු -EIO;
	}

	count = info->read(info, offset, aligned_size, buf);
	නම් (!count) {
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
		ආපසු -EIO;
	}

	ctx->fit = buf;
	debug("fit read offset %lx, size=%lu, dst=%p, count=%lu\n",
	      offset, aligned_size, buf, count);

	ආපසු 0;
}

static පූර්ණ_සංඛ්‍යා spl_simple_fit_parse(sa_nracanaa spl_fit_info *ctx)
{
	/* Find the correct subnode under "/configurations" */
	ctx->conf_node = fit_find_config_node(ctx->fit);
	නම් (ctx->conf_node < 0)
		ආපසු -EINVAL;

	නම් (CONFIG_IS_ENABLED(FIT_SIGNATURE)) {
		printf("## Checking hash(es) for config %s ... ",
		       fit_get_name(ctx->fit, ctx->conf_node, NULL));
		නම් (fit_config_verify(ctx->fit, ctx->conf_node))
			ආපසු -EPERM;
		puts("OK\n");
	}

	/* find the node holding the images information */
	ctx->images_node = fdt_path_offset(ctx->fit, FIT_IMAGES_PATH);
	නම් (ctx->images_node < 0) {
		debug("%s: Cannot find /images node: %d\n", __func__,
		      ctx->images_node);
		ආපසු -EINVAL;
	}

	ආපසු 0;
}

පූර්ණ_සංඛ්‍යා spl_load_simple_fit(sa_nracanaa spl_image_info *spl_image,
			sa_nracanaa spl_load_info *info, ulong offset, void *fit)
{
	sa_nracanaa spl_image_info image_info;
	sa_nracanaa spl_fit_info ctx;
	පූර්ණ_සංඛ්‍යා node = -1;
	පූර්ණ_සංඛ්‍යා ret;
	පූර්ණ_සංඛ්‍යා index = 0;
	පූර්ණ_සංඛ්‍යා firmware_node;

	ret = spl_simple_fit_read(&ctx, info, offset, fit);
	නම් (ret < 0)
		ආපසු ret;

	/* skip further processing if requested to enable load-only use cases */
	නම් (spl_load_simple_fit_skip_processing())
		ආපසු 0;

	ctx.fit = spl_load_simple_fit_fix_load(ctx.fit);

	ret = spl_simple_fit_parse(&ctx);
	නම් (ret < 0)
		ආපසු ret;

	නම් (IS_ENABLED(CONFIG_SPL_FPGA))
		spl_fit_load_fpga(&ctx, info, offset);

	/*
	 * Find the U-Boot image using the following search order:
	 *   - start at 'firmware' (e.g. an ARM Trusted Firmware)
	 *   - fall back 'kernel' (e.g. a Falcon-mode OS boot
	 *   - fall back to using the first 'loadables' entry
	 */
	නම් (node < 0)
		node = spl_fit_get_image_node(&ctx, FIT_FIRMWARE_PROP, 0);

	නම් (node < 0 && IS_ENABLED(CONFIG_SPL_OS_BOOT))
		node = spl_fit_get_image_node(&ctx, FIT_KERNEL_PROP, 0);

	නම් (node < 0) {
		debug("could not find firmware image, trying loadables...\n");
		node = spl_fit_get_image_node(&ctx, FIT_LOADABLE_PROP, 0);
		/*
		 * If we pick the U-Boot image from "loadables", start at
		 * the second image when later loading additional images.
		 */
		index = 1;
	}
	නම් (node < 0) {
		debug("%s: Cannot find u-boot image node: %d\n",
		      __func__, node);
		ආපසු -1;
	}

	/* Load the image and set up the spl_image structure */
	ret = load_simple_fit(info, offset, &ctx, node, spl_image,
			      CONFIG_SYS_BOOTM_LEN);
	නම් (ret)
		ආපසු ret;

	/*
	 * For backward compatibility, we treat the first node that is
	 * as a U-Boot image, if no OS-type has been declared.
	 */
	නම් (!spl_fit_image_get_os(ctx.fit, node, &spl_image->os))
		debug("Image OS is %s\n", genimg_get_os_name(spl_image->os));
	නැතහොත් නම් (!IS_ENABLED(CONFIG_SPL_OS_BOOT))
		spl_image->os = IH_OS_U_BOOT;

	/*
	 * Booting a next-stage U-Boot may require us to append the FDT.
	 * We allow this to fail, as the U-Boot image might embed its FDT.
	 */
	නම් (os_takes_devicetree(spl_image->os)) {
		ret = spl_fit_append_fdt(spl_image, info, offset, &ctx);
		නම් (ret < 0 && spl_image->os != IH_OS_U_BOOT)
			ආපසු ret;
	}

	firmware_node = node;
	/* Now check if there are more images for us to load */
	සඳහා (; ; index++) {
		uint8_t os_type = IH_OS_INVALID;

		node = spl_fit_get_image_node(&ctx, FIT_LOADABLE_PROP, index);
		නම් (node < 0)
			bha_nga;

		/*
		 * if the firmware is also a loadable, skip it because
		 * it already has been loaded. This is typically the case with
		 * u-boot.img generated by mkimage.
		 */
		නම් (firmware_node == node)
			continue;

		image_info.load_addr = 0;
		ret = load_simple_fit(info, offset, &ctx, node, &image_info,
				      CONFIG_SYS_BOOTM_LEN);
		නම් (ret < 0 && ret != -EBADSLT) {
			printf("%s: can't load image loadables index %d (ret = %d)\n",
			       __func__, index, ret);
			ආපසු ret;
		}

		නම් (spl_fit_image_is_fpga(ctx.fit, node))
			spl_fit_upload_fpga(&ctx, node, &image_info);

		නම් (!spl_fit_image_get_os(ctx.fit, node, &os_type))
			debug("Loadable is %s\n", genimg_get_os_name(os_type));

		නම් (os_takes_devicetree(os_type)) {
			spl_fit_append_fdt(&image_info, info, offset, &ctx);
			spl_image->fdt_addr = image_info.fdt_addr;
		}

		/*
		 * If the "firmware" image did not provide an entry point,
		 * use the first valid entry point from the loadables.
		 */
		නම් (spl_image->entry_point == FDT_ERROR &&
		    image_info.entry_point != FDT_ERROR)
			spl_image->entry_point = image_info.entry_point;

		spl_fit_image_record_arm32_optee(ctx.fit, node, spl_image,
						 &image_info);

		/* Record our loadables into the FDT */
		නම් (!CONFIG_IS_ENABLED(FIT_IMAGE_TINY) &&
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
	නම් (spl_image->entry_point == FDT_ERROR || spl_image->entry_point == 0)
		spl_image->entry_point = spl_image->load_addr;

	spl_image->flags |= SPL_FIT_FOUND;
	upl_set_fit_info(map_to_sysmem(ctx.fit), ctx.conf_node,
			 spl_image->entry_point);

	ආපසු 0;
}

/* Parse and load full fitImage in SPL */
පූර්ණ_සංඛ්‍යා spl_load_fit_image(sa_nracanaa spl_image_info *spl_image,
		       const sa_nracanaa legacy_img_hdr *header)
{
	sa_nracanaa bootm_headers images;
	const var_na *fit_uname_config = NULL;
	ulong fdt_hack;
	const var_na *uname;
	ulong fw_data = 0, dt_data = 0, img_data = 0;
	ulong fw_len = 0, dt_len = 0, img_len = 0;
	පූර්ණ_සංඛ්‍යා idx, conf_noffset;
	පූර්ණ_සංඛ්‍යා ret;

	images.verify = CONFIG_IS_ENABLED(FIT_SIGNATURE);

	ret = fit_image_load(&images, virt_to_phys((void *)header),
			     NULL, &fit_uname_config, IH_ARCH_DEFAULT,
			     IH_TYPE_FIRMWARE, -1, FIT_LOAD_OPTIONAL,
			     &fw_data, &fw_len);

	නම් (ret < 0) {
		ret = fit_image_load(&images, virt_to_phys((void *)header),
				     NULL, &fit_uname_config, IH_ARCH_DEFAULT,
				     IH_TYPE_KERNEL, -1, FIT_LOAD_OPTIONAL,
				     &fw_data, &fw_len);
	}

	නම් (ret < 0)
		ආපසු ret;

	spl_image->size = fw_len;
	spl_image->load_addr = fw_data;
	නම් (fit_image_get_entry(header, ret, &spl_image->entry_point))
		spl_image->entry_point = fw_data;
	නම් (fit_image_get_os(header, ret, &spl_image->os))
		spl_image->os = IH_OS_INVALID;
	spl_image->name = genimg_get_os_name(spl_image->os);

	debug(PHASE_PROMPT "payload image: %32s load addr: 0x%lx size: %d\n",
	      spl_image->name, spl_image->load_addr, spl_image->size);

	images.verify = CONFIG_IS_ENABLED(FIT_SIGNATURE);

	ret = fit_image_load(&images, virt_to_phys((void *)header), NULL,
			     &fit_uname_config, IH_ARCH_DEFAULT, IH_TYPE_FLATDT,
			     -1, FIT_LOAD_OPTIONAL, &dt_data, &dt_len);
	නම් (ret >= 0) {
		නම් (spl_image->os == IH_OS_U_BOOT) {
			/* HACK: U-Boot expects FDT at a specific address */
			fdt_hack = ALIGN(spl_image->load_addr + spl_image->size, 8);
			debug("Relocating FDT to %p\n", (void *)fdt_hack);
			memcpy(map_sysmem(fdt_hack, dt_len),
			       map_sysmem(dt_data, 0), dt_len);
			spl_image->fdt_addr = (void *)fdt_hack;
		} නැතහොත් {
			spl_image->fdt_addr = (void *)dt_data;
		}
	}

	conf_noffset = fit_conf_get_node((const void *)header,
					 fit_uname_config);
	නම් (conf_noffset < 0)
		ආපසු 0;

	සඳහා (idx = 0;
	     uname = fdt_stringlist_get((const void *)header, conf_noffset,
					FIT_LOADABLE_PROP, idx,
				NULL), uname;
	     idx++) {
		images.verify = CONFIG_IS_ENABLED(FIT_SIGNATURE);

		ret = fit_image_load(&images, virt_to_phys((void *)header),
				     &uname, &fit_uname_config,
				     IH_ARCH_DEFAULT, IH_TYPE_LOADABLE, -1,
				     FIT_LOAD_OPTIONAL_NON_ZERO,
				     &img_data, &img_len);
		නම් (ret < 0)
			ආපසු ret;
	}
	spl_image->flags |= SPL_FIT_FOUND;

	upl_set_fit_info(map_to_sysmem(header), conf_noffset,
			 spl_image->entry_point);

	ආපසු 0;
}
