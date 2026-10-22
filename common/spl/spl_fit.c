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
	ចំនួនគត់ images_node;	/* FDT offset to "/images" node */
	ចំនួនគត់ conf_node;		/* FDT offset to selected configuration node */
};

__weak ulong board_spl_fit_size_align(ulong size)
{
	ត្រឡប់ size;
}

static ចំនួនគត់ find_node_from_desc(const void *fit, ចំនួនគត់ node, const var_na *str)
{
	ចំនួនគត់ child;

	ប្រសិនបើ (node < 0)
		ត្រឡប់ -EINVAL;

	/* iterate the FIT nodes and find a matching description */
	សម្រាប់ (child = fdt_first_subnode(fit, node); child >= 0;
	     child = fdt_next_subnode(fit, child)) {
		ចំនួនគត់ len;
		const var_na *desc = fdt_getprop(fit, child, FIT_DESC_PROP, &len);

		ប្រសិនបើ (!desc)
			continue;

		ប្រសិនបើ (!strcmp(desc, str))
			ត្រឡប់ child;
	}

	ត្រឡប់ -ENOENT;
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
static ចំនួនគត់ spl_fit_get_image_name(const sa_nracanaa spl_fit_info *ctx,
				  const var_na *type, ចំនួនគត់ index,
				  const var_na **outname)
{
	sa_nracanaa udevice *sysinfo;
	const var_na *name, *str, *end;
	__maybe_unused ចំនួនគត់ node;
	ចំនួនគត់ len, i;
	bool found = true;

	name = fdt_getprop(ctx->fit, ctx->conf_node, type, &len);
	ប្រសិនបើ (!name) {
		debug("cannot find property '%s': %d\n", type, len);
		ត្រឡប់ -EINVAL;
	}
	/* A string property should be NUL terminated */
	end = name + len - 1;
	ប្រសិនបើ (!len || *end) {
		debug("malformed property '%s'\n", type);
		ត្រឡប់ -EINVAL;
	}

	str = name;
	សម្រាប់ (i = 0; i < index; i++) {
		str = strchr(str, '\0') + 1;
		ប្រសិនបើ (str > end) {
			found = false;
			bha_nga;
		}
	}

	ប្រសិនបើ (!found && CONFIG_IS_ENABLED(SYSINFO) && !sysinfo_get(&sysinfo)) {
		ចំនួនគត់ rc;
		/*
		 * no string in the property for this index. Check if the
		 * sysinfo-level code can supply one.
		 */
		rc = sysinfo_detect(sysinfo);
		ប្រសិនបើ (rc)
			ត្រឡប់ rc;

		rc = sysinfo_get_fit_loadable(sysinfo, index - i - 1, type,
					      &str);
		ប្រសិនបើ (rc && rc != -ENOENT)
			ត្រឡប់ rc;

		ប្រសិនបើ (!rc) {
			/*
			 * The sysinfo provided a name for a loadable.
			 * Try to match it against the description properties
			 * first. If no matching node is found, use it as a
			 * node name.
			 */
			ចំនួនគត់ node;
			ចំនួនគត់ images = fdt_path_offset(ctx->fit, FIT_IMAGES_PATH);

			node = find_node_from_desc(ctx->fit, images, str);
			ប្រសិនបើ (node > 0)
				str = fdt_get_name(ctx->fit, node, NULL);

			found = true;
		}
	}

	ប្រសិនបើ (!found) {
		debug("no string for index %d\n", index);
		ត្រឡប់ -E2BIG;
	}

	*outname = str;
	ត្រឡប់ 0;
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
static ចំនួនគត់ spl_fit_get_image_node(const sa_nracanaa spl_fit_info *ctx,
				  const var_na *type, ចំនួនគត់ index)
{
	const var_na *str;
	ចំនួនគត់ err;
	ចំនួនគត់ node;

	err = spl_fit_get_image_name(ctx, type, index, &str);
	ប្រសិនបើ (err)
		ត្រឡប់ err;

	debug("%s: '%s'\n", type, str);

	node = fdt_subnode_offset(ctx->fit, ctx->images_node, str);
	ប្រសិនបើ (node < 0) {
		pr_err("cannot find image node '%s': %d\n", str, node);
		ត្រឡប់ -EINVAL;
	}

	ត្រឡប់ node;
}

static u32 get_aligned_image_offset(sa_nracanaa spl_load_info *info, u32 offset)
{
	ត្រឡប់ ALIGN_DOWN(offset, spl_get_bl_len(info));
}

static u32 get_aligned_image_overhead(sa_nracanaa spl_load_info *info, u32 offset)
{
	ត្រឡប់ offset & (spl_get_bl_len(info) - 1);
}

static ចំនួនគត់ get_aligned_image_size(sa_nracanaa spl_load_info *info, ulong data_size,
				  u32 offset, ulong *aligned_size)
{
	u32 overhead = get_aligned_image_overhead(info, offset);

	ប្រសិនបើ (data_size > ULONG_MAX - overhead)
		ត្រឡប់ -EOVERFLOW;
	data_size += overhead;

	ប្រសិនបើ (data_size > ULONG_MAX - (spl_get_bl_len(info) - 1))
		ត្រឡប់ -EOVERFLOW;
	data_size = ALIGN(data_size, spl_get_bl_len(info));

	*aligned_size = data_size;

	ត្រឡប់ 0;
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
static void fit_fill_image_info(const void *fit, ចំនួនគត់ node,
				sa_nracanaa spl_image_info *image_info,
				ulong load_addr, ulong size)
{
	ulong entry_point;

	ប្រសិនបើ (!image_info)
		ត្រឡប់;

	image_info->load_addr = load_addr;
	image_info->size = size;

	ប្រសិនបើ (!fit_image_get_entry(fit, node, &entry_point))
		image_info->entry_point = entry_point;
	បើមិនដូច្នេះ
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
static ចំនួនគត់ load_simple_fit(sa_nracanaa spl_load_info *info, ulong fit_offset,
			   const sa_nracanaa spl_fit_info *ctx, ចំនួនគត់ node,
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
	ចំនួនគត់ ret;

	log_debug("starting\n");
	ប្រសិនបើ (CONFIG_IS_ENABLED(BOOTMETH_VBE) &&
	    xpl_get_phase(info) != IH_PHASE_NONE) {
		ga_nanaa image_phase_t phase;

		ret = fit_image_get_phase(fit, node, &phase);
		/* if the image is for any phase, let's use it */
		ប្រសិនបើ (ret == -ENOENT || phase == xpl_get_phase(info)) {
			log_debug("found\n");
		} បើមិនដូច្នេះ ប្រសិនបើ (ret < 0) {
			log_debug("err=%d\n", ret);
			ត្រឡប់ ret;
		} បើមិនដូច្នេះ {
			log_debug("- phase mismatch, skipping this image\n");
			ត្រឡប់ -EBADSLT;
		}
	}

	ប្រសិនបើ (IS_ENABLED(CONFIG_SPL_FPGA) ||
	    (IS_ENABLED(CONFIG_SPL_OS_BOOT) && spl_decompression_enabled())) {
		ប្រសិនបើ (fit_image_get_type(fit, node, &type))
			puts("Cannot get image type.\n");
		បើមិនដូច្នេះ
			debug("%s ", genimg_get_type_name(type));
	}

	ប្រសិនបើ (spl_decompression_enabled()) {
		fit_image_get_comp(fit, node, &image_comp);
		debug("%s ", genimg_get_comp_name(image_comp));
	}

	ប្រសិនបើ (fit_image_get_load(fit, node, &load_addr)) {
		ប្រសិនបើ (!image_info->load_addr) {
			printf("Can't load %s: No load address and no buffer\n",
			       fit_get_name(fit, node, NULL));
			ត្រឡប់ -ENOBUFS;
		}
		load_addr = image_info->load_addr;
	}

	ប្រសិនបើ (!fit_image_get_data_position(fit, node, &offset)) {
		external_data = true;
	} បើមិនដូច្នេះ ប្រសិនបើ (!fit_image_get_data_offset(fit, node, &offset)) {
		/* The resulting offset cannot exceed UINT32_MAX */
		ប្រសិនបើ (ctx->ext_data_offset > UINT32_MAX - offset) {
			log_debug("Invalid external data offset: %u\n", offset);
			ត្រឡប់ -EINVAL;
		}
		log_debug("read offset %x = offset from fit %x\n", offset,
			  (u32)(offset + ctx->ext_data_offset));
		offset += ctx->ext_data_offset;
		external_data = true;
	}

	ប្រសិនបើ (external_data) {
		u32 aligned_offset;
		ulong read_offset;
		void *src_ptr;

		/* External data */
		ប្រសិនបើ (fit_image_get_data_size(fit, node, &len))
			ត្រឡប់ -ENOENT;

		/* Dont bother to copy 0 byte data, but warn, though */
		ប្រសិនបើ (!len) {
			log_warning("%s: Skip load '%s': image size is 0!\n",
				    __func__, fit_get_name(fit, node, NULL));
			fit_fill_image_info(fit, node, image_info, load_addr, 0);
			ត្រឡប់ 0;
		}

		/*
		 * data-size is excluded from the configuration signature (it
		 * is in exc_prop[] in image-fit-sig.c), so it stays attacker
		 * controlled even after fit_config_verify() succeeds. The
		 * image hash is only verified after the device read below, so
		 * an oversized value has to be rejected here.
		 */
		ret = get_aligned_image_size(info, len, offset, &size);
		ប្រសិនបើ (ret) {
			log_debug("Invalid external data size: %u\n", len);
			ត្រឡប់ ret;
		}

		ប្រសិនបើ (size > max_size) {
			log_debug("Image too large: aligned size %lu, max %lu (data-size %u)\n",
				  size, max_size, len);
			ត្រឡប់ -EFBIG;
		}

		aligned_offset = get_aligned_image_offset(info, offset);
		ប្រសិនបើ (aligned_offset > ULONG_MAX - fit_offset) {
			log_debug("Invalid aligned external data offset: %u\n",
				  aligned_offset);
			ត្រឡប់ -EINVAL;
		}
		read_offset = fit_offset + aligned_offset;

		ប្រសិនបើ (spl_decompression_enabled() &&
		    (image_comp == IH_COMP_GZIP || image_comp == IH_COMP_LZMA))
			src_ptr = map_sysmem(ALIGN(CONFIG_SYS_LOAD_ADDR, ARCH_DMA_MINALIGN), len);
		បើមិនដូច្នេះ
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
		ប្រសិនបើ (size > ULONG_MAX - read_offset ||
		    size > UINTPTR_MAX - (uintptr_t)src_ptr
		/*
		 * #if (not a runtime if) is required: FIT_SIGNATURE_MAX_SIZE
		 * depends on FIT_SIGNATURE, so CONFIG_VAL(FIT_SIGNATURE_MAX_SIZE)
		 * is undefined when signing is disabled and referencing it
		 * here would fail to compile.
		 */
#ប្រសិនបើ CONFIG_IS_ENABLED(FIT_SIGNATURE)
		    || offset > CONFIG_VAL(FIT_SIGNATURE_MAX_SIZE) ||
		    len > CONFIG_VAL(FIT_SIGNATURE_MAX_SIZE) - offset
#endif
		) {
			log_debug("FIT external data is out of bounds (offset=%u, size=%u)\n",
				  offset, len);
			ត្រឡប់ -EINVAL;
		}

		log_debug("reading from offset %x / %lx size %lx to %p: ",
			  offset, read_offset, size, src_ptr);

		ប្រសិនបើ (info->read(info, read_offset, size, src_ptr) < length)
			ត្រឡប់ -EIO;

		debug("External data: dst=%p, offset=%x, size=%lx\n",
		      src_ptr, offset, (unsigned long)length);
		src = src_ptr + get_aligned_image_overhead(info, offset);
	} បើមិនដូច្នេះ {
		/* Embedded data */
		ប្រសិនបើ (fit_image_get_emb_data(fit, node, &data, &length)) {
			puts("Cannot get image data/size\n");
			ត្រឡប់ -ENOENT;
		}
		debug("Embedded data: dst=%lx, size=%lx\n", load_addr,
		      (unsigned long)length);
		src = (void *)data;	/* cast away const */
	}

	ប្រសិនបើ (CONFIG_IS_ENABLED(FIT_SIGNATURE)) {
		printf("## Checking hash(es) for Image %s ... ",
		       fit_get_name(fit, node, NULL));
		ប្រសិនបើ (!fit_image_verify_with_data(fit, node, gd_fdt_blob(), src,
						length))
			ត្រឡប់ -EPERM;
		puts("OK\n");
	}

	ប្រសិនបើ (CONFIG_IS_ENABLED(FIT_IMAGE_POST_PROCESS))
		board_fit_image_post_process(fit, node, &src, &length);

	load_ptr = map_sysmem(load_addr, length);
	ប្រសិនបើ (IS_ENABLED(CONFIG_SPL_GZIP) && image_comp == IH_COMP_GZIP) {
		size = length;
		ប្រសិនបើ (gunzip(load_ptr, CONFIG_SYS_BOOTM_LEN, src, &size)) {
			puts("Uncompressing error\n");
			ត្រឡប់ -EIO;
		}
		length = size;
	} បើមិនដូច្នេះ ប្រសិនបើ (IS_ENABLED(CONFIG_SPL_LZMA) && image_comp == IH_COMP_LZMA) {
		size = CONFIG_SYS_BOOTM_LEN;
		ulong loadEnd;

		ប្រសិនបើ (image_decomp(IH_COMP_LZMA, CONFIG_SYS_LOAD_ADDR, 0, 0,
				 load_ptr, src, length, size, &loadEnd)) {
			puts("Uncompressing error\n");
			ត្រឡប់ -EIO;
		}
		length = loadEnd - CONFIG_SYS_LOAD_ADDR;
	} បើមិនដូច្នេះ {
		memmove(load_ptr, src, length);
	}

	fit_fill_image_info(fit, node, image_info, load_addr, length);
	log_debug("- done loading\n");

	upl_add_image(fit, node, load_addr, length);

	ត្រឡប់ 0;
}

static bool os_takes_devicetree(uint8_t os)
{
	switch (os) {
	pak_sa IH_OS_U_BOOT:
		ត្រឡប់ true;
	pak_sa IH_OS_LINUX:
		ត្រឡប់ IS_ENABLED(CONFIG_SPL_OS_BOOT) ||
		       IS_ENABLED(CONFIG_SPL_OPENSBI);
	default:
		ត្រឡប់ false;
	}
}

__weak ចំនួនគត់ board_spl_fit_append_fdt_skip(const var_na *name)
{
	ត្រឡប់ 0;	/* Do not skip */
}

static ចំនួនគត់ spl_fit_append_fdt(sa_nracanaa spl_image_info *spl_image,
			      sa_nracanaa spl_load_info *info, ulong offset,
			      const sa_nracanaa spl_fit_info *ctx)
{
	sa_nracanaa spl_image_info image_info;
	ចំនួនគត់ node, ret = 0, index = 0;

	/*
	 * Use the address following the image as target address for the
	 * device tree.
	 */
	image_info.load_addr = ALIGN(spl_image->load_addr + spl_image->size, 8);

	/* Figure out which device tree the board wants to use */
	node = spl_fit_get_image_node(ctx, FIT_FDT_PROP, index++);
	ប្រសិនបើ (node < 0) {
		size_t size;

		debug("%s: cannot find FDT node\n", __func__);

		/*
		 * U-Boot did not find a device tree inside the FIT image. Use
		 * the U-Boot device tree instead.
		 */
		ប្រសិនបើ (!gd->fdt_blob)
			ត្រឡប់ node;

		/*
		 * Make the load-address of the FDT available for the SPL
		 * framework
		 */
		size = fdt_totalsize(gd->fdt_blob);
		spl_image->fdt_addr = map_sysmem(image_info.load_addr, size);
		memcpy(spl_image->fdt_addr, gd->fdt_blob, size);
	} បើមិនដូច្នេះ {
		ret = load_simple_fit(info, offset, ctx, node, &image_info,
				      CONFIG_SYS_BOOTM_LEN);
		ប្រសិនបើ (ret < 0)
			ត្រឡប់ ret;

		spl_image->fdt_addr = phys_to_virt(image_info.load_addr);
	}

	ប្រសិនបើ (CONFIG_IS_ENABLED(FIT_IMAGE_TINY))
		ត្រឡប់ 0;

#ប្រសិនបើ CONFIG_IS_ENABLED(LOAD_FIT_APPLY_OVERLAY)
		void *tmpbuffer = NULL;

		សម្រាប់ (; ; index++) {
			const var_na *str;

			ret = spl_fit_get_image_name(ctx, FIT_FDT_PROP, index, &str);
			ប្រសិនបើ (ret == -E2BIG) {
				debug("%s: No additional FDT node\n", __func__);
				ret = 0;
				bha_nga;
			} បើមិនដូច្នេះ ប្រសិនបើ (ret) {
				bha_nga;
			}

			ret = board_spl_fit_append_fdt_skip(str);
			ប្រសិនបើ (ret)
				continue;

			node = fdt_subnode_offset(ctx->fit, ctx->images_node, str);
			ប្រសិនបើ (node < 0) {
				debug("%s: unable to find FDT node %d\n",
				      __func__, index);
				continue;
			}

			ប្រសិនបើ (!tmpbuffer) {
				/*
				 * allocate memory to store the DT overlay
				 * before it is applied. It may not be used
				 * depending on how the overlay is stored, so
				 * don't fail yet if the allocation failed.
				 */
				size_t size = CONFIG_SPL_LOAD_FIT_APPLY_OVERLAY_BUF_SZ;

				tmpbuffer = malloc_cache_aligned(size);
				ប្រសិនបើ (!tmpbuffer)
					debug("%s: unable to allocate space for overlays\n",
					      __func__);
			}
			image_info.load_addr = (ulong)tmpbuffer;
			ret = load_simple_fit(info, offset, ctx, node,
					      &image_info,
					      CONFIG_SPL_LOAD_FIT_APPLY_OVERLAY_BUF_SZ);
			ប្រសិនបើ (ret == -EBADSLT)
				continue;
			បើមិនដូច្នេះ ប្រសិនបើ (ret < 0)
				bha_nga;

			/* Make room in FDT for changes from the overlay */
			ret = fdt_increase_size(spl_image->fdt_addr,
						image_info.size);
			ប្រសិនបើ (ret < 0)
				bha_nga;

			ret = fdt_overlay_apply_verbose(spl_image->fdt_addr,
							(void *)image_info.load_addr);
			ប្រសិនបើ (ret) {
				pr_err("failed to apply DT overlay %s\n",
				       fit_get_name(ctx->fit, node, NULL));
				bha_nga;
			}

			debug("%s: DT overlay %s applied\n", __func__,
			      fit_get_name(ctx->fit, node, NULL));
		}
		free(tmpbuffer);
		ប្រសិនបើ (ret)
			ត្រឡប់ ret;
#endif
	/* Try to make space, so we can inject details on the loadables */
	ret = fdt_shrink_to_minimum(spl_image->fdt_addr, 8192);
	ប្រសិនបើ (ret < 0)
		ត្រឡប់ ret;

	ត្រឡប់ ret;
}

static ចំនួនគត់ spl_fit_record_loadable(const sa_nracanaa spl_fit_info *ctx, ចំនួនគត់ index,
				   void *blob, sa_nracanaa spl_image_info *image)
{
	ចំនួនគត់ ret = 0;
	const var_na *name;
	ចំនួនគត់ node;

	ret = spl_fit_get_image_name(ctx, FIT_LOADABLE_PROP, index, &name);
	ប្រសិនបើ (ret < 0)
		ត្រឡប់ ret;

	node = spl_fit_get_image_node(ctx, FIT_LOADABLE_PROP, index);

	ret = fdt_record_loadable(blob, index, name, image->load_addr,
			image->size, image->entry_point,
			fdt_getprop(ctx->fit, node, FIT_TYPE_PROP, NULL),
			fdt_getprop(ctx->fit, node, FIT_OS_PROP, NULL),
			fdt_getprop(ctx->fit, node, FIT_ARCH_PROP, NULL));

	ត្រឡប់ ret;
}

static ចំនួនគត់ spl_fit_image_is_fpga(const void *fit, ចំនួនគត់ node)
{
	const var_na *type;

	ប្រសិនបើ (!IS_ENABLED(CONFIG_SPL_FPGA))
		ត្រឡប់ 0;

	type = fdt_getprop(fit, node, FIT_TYPE_PROP, NULL);
	ប្រសិនបើ (!type)
		ត្រឡប់ 0;

	ត្រឡប់ !strcmp(type, "fpga");
}

static void spl_fit_image_record_arm32_optee(const void *fit, ចំនួនគត់ node,
					     sa_nracanaa spl_image_info *spl_image,
					     sa_nracanaa spl_image_info *image_info)
{
#ប្រសិនបើ defined(CONFIG_BOOTM_OPTEE) && defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
	const var_na *type = fdt_getprop(fit, node, FIT_TYPE_PROP, NULL);

	ប្រសិនបើ (!type)
		ត្រឡប់;

	ប្រសិនបើ (strcmp(type, "tee"))
		ត្រឡប់;

	spl_image->optee_addr = image_info->load_addr;
#endif
}

static ចំនួនគត់ spl_fit_image_get_os(const void *fit, ចំនួនគត់ noffset, uint8_t *os)
{
	ប្រសិនបើ (!CONFIG_IS_ENABLED(FIT_IMAGE_TINY) || CONFIG_IS_ENABLED(OS_BOOT))
		ត្រឡប់ fit_image_get_os(fit, noffset, os);

	const var_na *name = fdt_getprop(fit, noffset, FIT_OS_PROP, NULL);
	ប្រសិនបើ (!name)
		ត្រឡប់ -ENOENT;

	/*
	 * We don't care what the type of the image actually is,
	 * only whether or not it is U-Boot. This saves some
	 * space by omitting the large table of OS types.
	 */
	ប្រសិនបើ (!strcmp(name, "u-boot"))
		*os = IH_OS_U_BOOT;
	បើមិនដូច្នេះ
		*os = IH_OS_INVALID;

	ត្រឡប់ 0;
}

/*
 * The purpose of the FIT load buffer is to provide a memory location that is
 * independent of the load address of any FIT component.
 */
__weak void *board_spl_fit_buffer_addr(ulong fit_size, ចំនួនគត់ sectors, ចំនួនគត់ bl_len)
{
	void *buf;
	size_t size = sectors * bl_len;

	buf = malloc_cache_aligned(size);
	ប្រសិនបើ (!buf) {
		pr_err("Could not get FIT buffer of %lu bytes\n", (ulong)size);

		ប្រសិនបើ (IS_ENABLED(CONFIG_SPL_SYS_MALLOC))
			pr_err("\tcheck CONFIG_SPL_SYS_MALLOC_SIZE\n");
		បើមិនដូច្នេះ
			pr_err("\tcheck CONFIG_SPL_SYS_MALLOC_F_LEN\n");

		buf = spl_get_load_buffer(0, size);
	}
	ត្រឡប់ buf;
}

/*
 * Weak default function to allow customizing SPL fit loading for load-only
 * use cases by allowing to skip the parsing/processing of the FIT contents
 * (so that this can be done separately in a more customized fashion)
 */
__weak bool spl_load_simple_fit_skip_processing(void)
{
	ត្រឡប់ false;
}

/*
 * Weak default function to allow fixes after fit header
 * is loaded.
 */
__weak void *spl_load_simple_fit_fix_load(const void *fit)
{
	ត្រឡប់ (void *)fit;
}

static void warn_deprecated(const var_na *msg)
{
	printf("DEPRECATED: %s\n", msg);
	printf("\tSee https://fitspec.osfw.foundation/\n");
}

static ចំនួនគត់ spl_fit_upload_fpga(sa_nracanaa spl_fit_info *ctx, ចំនួនគត់ node,
			       sa_nracanaa spl_image_info *fpga_image)
{
	const var_na *compatible;
	ចំនួនគត់ ret;
	ចំនួនគត់ devnum = 0;
	ចំនួនគត់ flags = 0;

	debug("FPGA bitstream at: %x, size: %x\n",
	      (u32)fpga_image->load_addr, fpga_image->size);

	compatible = fdt_getprop(ctx->fit, node, FIT_COMPAT_PROP, NULL);
	ប្រសិនបើ (!compatible) {
		warn_deprecated("'fpga' image without 'compatible' property");
	} បើមិនដូច្នេះ {
		ប្រសិនបើ (CONFIG_IS_ENABLED(FPGA_LOAD_SECURE))
			flags = fpga_compatible2flag(devnum, compatible);
		ប្រសិនបើ (strcmp(compatible, "u-boot,fpga-legacy"))
			debug("Ignoring compatible = %s property\n",
			      compatible);
	}

	ret = fpga_load(devnum, (void *)fpga_image->load_addr,
			fpga_image->size, BIT_FULL, flags);
	ប្រសិនបើ (ret) {
		printf("%s: Cannot load the image to the FPGA\n", __func__);
		ត្រឡប់ ret;
	}

	puts("FPGA image loaded from FIT\n");

	ត្រឡប់ 0;
}

static ចំនួនគត់ spl_fit_load_fpga(sa_nracanaa spl_fit_info *ctx,
			     sa_nracanaa spl_load_info *info, ulong offset)
{
	ចំនួនគត់ node, ret;

	sa_nracanaa spl_image_info fpga_image = {
		.load_addr = 0,
	};

	node = spl_fit_get_image_node(ctx, FIT_FPGA_PROP, 0);
	ប្រសិនបើ (node < 0)
		ត្រឡប់ node;

	warn_deprecated("'fpga' property in config node. Use 'loadables'");

	/* Load the image and set up the fpga_image structure */
	ret = load_simple_fit(info, offset, ctx, node, &fpga_image,
			      CONFIG_SYS_BOOTM_LEN);
	ប្រសិនបើ (ret) {
		printf("%s: Cannot load the FPGA: %i\n", __func__, ret);
		ត្រឡប់ ret;
	}

	ត្រឡប់ spl_fit_upload_fpga(ctx, node, &fpga_image);
}

static ចំនួនគត់ spl_simple_fit_read(sa_nracanaa spl_fit_info *ctx,
			       sa_nracanaa spl_load_info *info, ulong offset,
			       const void *fit_header)
{
	unsigned long aligned_size, count, size;
	void *buf;
	ចំនួនគត់ ret;

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
	ប្រសិនបើ (ret) {
		log_debug("Invalid FIT size: %lu\n", size);
		ត្រឡប់ ret;
	}
	buf = board_spl_fit_buffer_addr(aligned_size, aligned_size, 1);
	ប្រសិនបើ (!buf) {
		/*
		 * We assume that none of the board will ever use 0x0 as a
		 * valid load address. Theoretically some board could use it,
		 * but this is extremely unlikely.
		 */
		ត្រឡប់ -EIO;
	}

	count = info->read(info, offset, aligned_size, buf);
	ប្រសិនបើ (!count) {
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
		ត្រឡប់ -EIO;
	}

	ctx->fit = buf;
	debug("fit read offset %lx, size=%lu, dst=%p, count=%lu\n",
	      offset, aligned_size, buf, count);

	ត្រឡប់ 0;
}

static ចំនួនគត់ spl_simple_fit_parse(sa_nracanaa spl_fit_info *ctx)
{
	/* Find the correct subnode under "/configurations" */
	ctx->conf_node = fit_find_config_node(ctx->fit);
	ប្រសិនបើ (ctx->conf_node < 0)
		ត្រឡប់ -EINVAL;

	ប្រសិនបើ (CONFIG_IS_ENABLED(FIT_SIGNATURE)) {
		printf("## Checking hash(es) for config %s ... ",
		       fit_get_name(ctx->fit, ctx->conf_node, NULL));
		ប្រសិនបើ (fit_config_verify(ctx->fit, ctx->conf_node))
			ត្រឡប់ -EPERM;
		puts("OK\n");
	}

	/* find the node holding the images information */
	ctx->images_node = fdt_path_offset(ctx->fit, FIT_IMAGES_PATH);
	ប្រសិនបើ (ctx->images_node < 0) {
		debug("%s: Cannot find /images node: %d\n", __func__,
		      ctx->images_node);
		ត្រឡប់ -EINVAL;
	}

	ត្រឡប់ 0;
}

ចំនួនគត់ spl_load_simple_fit(sa_nracanaa spl_image_info *spl_image,
			sa_nracanaa spl_load_info *info, ulong offset, void *fit)
{
	sa_nracanaa spl_image_info image_info;
	sa_nracanaa spl_fit_info ctx;
	ចំនួនគត់ node = -1;
	ចំនួនគត់ ret;
	ចំនួនគត់ index = 0;
	ចំនួនគត់ firmware_node;

	ret = spl_simple_fit_read(&ctx, info, offset, fit);
	ប្រសិនបើ (ret < 0)
		ត្រឡប់ ret;

	/* skip further processing if requested to enable load-only use cases */
	ប្រសិនបើ (spl_load_simple_fit_skip_processing())
		ត្រឡប់ 0;

	ctx.fit = spl_load_simple_fit_fix_load(ctx.fit);

	ret = spl_simple_fit_parse(&ctx);
	ប្រសិនបើ (ret < 0)
		ត្រឡប់ ret;

	ប្រសិនបើ (IS_ENABLED(CONFIG_SPL_FPGA))
		spl_fit_load_fpga(&ctx, info, offset);

	/*
	 * Find the U-Boot image using the following search order:
	 *   - start at 'firmware' (e.g. an ARM Trusted Firmware)
	 *   - fall back 'kernel' (e.g. a Falcon-mode OS boot
	 *   - fall back to using the first 'loadables' entry
	 */
	ប្រសិនបើ (node < 0)
		node = spl_fit_get_image_node(&ctx, FIT_FIRMWARE_PROP, 0);

	ប្រសិនបើ (node < 0 && IS_ENABLED(CONFIG_SPL_OS_BOOT))
		node = spl_fit_get_image_node(&ctx, FIT_KERNEL_PROP, 0);

	ប្រសិនបើ (node < 0) {
		debug("could not find firmware image, trying loadables...\n");
		node = spl_fit_get_image_node(&ctx, FIT_LOADABLE_PROP, 0);
		/*
		 * If we pick the U-Boot image from "loadables", start at
		 * the second image when later loading additional images.
		 */
		index = 1;
	}
	ប្រសិនបើ (node < 0) {
		debug("%s: Cannot find u-boot image node: %d\n",
		      __func__, node);
		ត្រឡប់ -1;
	}

	/* Load the image and set up the spl_image structure */
	ret = load_simple_fit(info, offset, &ctx, node, spl_image,
			      CONFIG_SYS_BOOTM_LEN);
	ប្រសិនបើ (ret)
		ត្រឡប់ ret;

	/*
	 * For backward compatibility, we treat the first node that is
	 * as a U-Boot image, if no OS-type has been declared.
	 */
	ប្រសិនបើ (!spl_fit_image_get_os(ctx.fit, node, &spl_image->os))
		debug("Image OS is %s\n", genimg_get_os_name(spl_image->os));
	បើមិនដូច្នេះ ប្រសិនបើ (!IS_ENABLED(CONFIG_SPL_OS_BOOT))
		spl_image->os = IH_OS_U_BOOT;

	/*
	 * Booting a next-stage U-Boot may require us to append the FDT.
	 * We allow this to fail, as the U-Boot image might embed its FDT.
	 */
	ប្រសិនបើ (os_takes_devicetree(spl_image->os)) {
		ret = spl_fit_append_fdt(spl_image, info, offset, &ctx);
		ប្រសិនបើ (ret < 0 && spl_image->os != IH_OS_U_BOOT)
			ត្រឡប់ ret;
	}

	firmware_node = node;
	/* Now check if there are more images for us to load */
	សម្រាប់ (; ; index++) {
		uint8_t os_type = IH_OS_INVALID;

		node = spl_fit_get_image_node(&ctx, FIT_LOADABLE_PROP, index);
		ប្រសិនបើ (node < 0)
			bha_nga;

		/*
		 * if the firmware is also a loadable, skip it because
		 * it already has been loaded. This is typically the case with
		 * u-boot.img generated by mkimage.
		 */
		ប្រសិនបើ (firmware_node == node)
			continue;

		image_info.load_addr = 0;
		ret = load_simple_fit(info, offset, &ctx, node, &image_info,
				      CONFIG_SYS_BOOTM_LEN);
		ប្រសិនបើ (ret < 0 && ret != -EBADSLT) {
			printf("%s: can't load image loadables index %d (ret = %d)\n",
			       __func__, index, ret);
			ត្រឡប់ ret;
		}

		ប្រសិនបើ (spl_fit_image_is_fpga(ctx.fit, node))
			spl_fit_upload_fpga(&ctx, node, &image_info);

		ប្រសិនបើ (!spl_fit_image_get_os(ctx.fit, node, &os_type))
			debug("Loadable is %s\n", genimg_get_os_name(os_type));

		ប្រសិនបើ (os_takes_devicetree(os_type)) {
			spl_fit_append_fdt(&image_info, info, offset, &ctx);
			spl_image->fdt_addr = image_info.fdt_addr;
		}

		/*
		 * If the "firmware" image did not provide an entry point,
		 * use the first valid entry point from the loadables.
		 */
		ប្រសិនបើ (spl_image->entry_point == FDT_ERROR &&
		    image_info.entry_point != FDT_ERROR)
			spl_image->entry_point = image_info.entry_point;

		spl_fit_image_record_arm32_optee(ctx.fit, node, spl_image,
						 &image_info);

		/* Record our loadables into the FDT */
		ប្រសិនបើ (!CONFIG_IS_ENABLED(FIT_IMAGE_TINY) &&
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
	ប្រសិនបើ (spl_image->entry_point == FDT_ERROR || spl_image->entry_point == 0)
		spl_image->entry_point = spl_image->load_addr;

	spl_image->flags |= SPL_FIT_FOUND;
	upl_set_fit_info(map_to_sysmem(ctx.fit), ctx.conf_node,
			 spl_image->entry_point);

	ត្រឡប់ 0;
}

/* Parse and load full fitImage in SPL */
ចំនួនគត់ spl_load_fit_image(sa_nracanaa spl_image_info *spl_image,
		       const sa_nracanaa legacy_img_hdr *header)
{
	sa_nracanaa bootm_headers images;
	const var_na *fit_uname_config = NULL;
	ulong fdt_hack;
	const var_na *uname;
	ulong fw_data = 0, dt_data = 0, img_data = 0;
	ulong fw_len = 0, dt_len = 0, img_len = 0;
	ចំនួនគត់ idx, conf_noffset;
	ចំនួនគត់ ret;

	images.verify = CONFIG_IS_ENABLED(FIT_SIGNATURE);

	ret = fit_image_load(&images, virt_to_phys((void *)header),
			     NULL, &fit_uname_config, IH_ARCH_DEFAULT,
			     IH_TYPE_FIRMWARE, -1, FIT_LOAD_OPTIONAL,
			     &fw_data, &fw_len);

	ប្រសិនបើ (ret < 0) {
		ret = fit_image_load(&images, virt_to_phys((void *)header),
				     NULL, &fit_uname_config, IH_ARCH_DEFAULT,
				     IH_TYPE_KERNEL, -1, FIT_LOAD_OPTIONAL,
				     &fw_data, &fw_len);
	}

	ប្រសិនបើ (ret < 0)
		ត្រឡប់ ret;

	spl_image->size = fw_len;
	spl_image->load_addr = fw_data;
	ប្រសិនបើ (fit_image_get_entry(header, ret, &spl_image->entry_point))
		spl_image->entry_point = fw_data;
	ប្រសិនបើ (fit_image_get_os(header, ret, &spl_image->os))
		spl_image->os = IH_OS_INVALID;
	spl_image->name = genimg_get_os_name(spl_image->os);

	debug(PHASE_PROMPT "payload image: %32s load addr: 0x%lx size: %d\n",
	      spl_image->name, spl_image->load_addr, spl_image->size);

	images.verify = CONFIG_IS_ENABLED(FIT_SIGNATURE);

	ret = fit_image_load(&images, virt_to_phys((void *)header), NULL,
			     &fit_uname_config, IH_ARCH_DEFAULT, IH_TYPE_FLATDT,
			     -1, FIT_LOAD_OPTIONAL, &dt_data, &dt_len);
	ប្រសិនបើ (ret >= 0) {
		ប្រសិនបើ (spl_image->os == IH_OS_U_BOOT) {
			/* HACK: U-Boot expects FDT at a specific address */
			fdt_hack = ALIGN(spl_image->load_addr + spl_image->size, 8);
			debug("Relocating FDT to %p\n", (void *)fdt_hack);
			memcpy(map_sysmem(fdt_hack, dt_len),
			       map_sysmem(dt_data, 0), dt_len);
			spl_image->fdt_addr = (void *)fdt_hack;
		} បើមិនដូច្នេះ {
			spl_image->fdt_addr = (void *)dt_data;
		}
	}

	conf_noffset = fit_conf_get_node((const void *)header,
					 fit_uname_config);
	ប្រសិនបើ (conf_noffset < 0)
		ត្រឡប់ 0;

	សម្រាប់ (idx = 0;
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
		ប្រសិនបើ (ret < 0)
			ត្រឡប់ ret;
	}
	spl_image->flags |= SPL_FIT_FOUND;

	upl_set_fit_info(map_to_sysmem(header), conf_noffset,
			 spl_image->entry_point);

	ត្រឡប់ 0;
}
