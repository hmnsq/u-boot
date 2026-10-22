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

struct spl_fit_info {
	const void *fit;	/* Pointer to a valid FIT blob */
	size_t ext_data_offset;	/* Offset to FIT external data (end of FIT) */
	整数 images_node;	/* FDT offset to "/images" node */
	整数 conf_node;		/* FDT offset to selected configuration node */
};

__weak ulong board_spl_fit_size_align(ulong size)
{
	返回 size;
}

static 整数 find_node_from_desc(const void *fit, 整数 node, const char *str)
{
	整数 child;

	如果 (node < 0)
		返回 -EINVAL;

	/* iterate the FIT nodes and find a matching description */
	为 (child = fdt_first_subnode(fit, node); child >= 0;
	     child = fdt_next_subnode(fit, child)) {
		整数 len;
		const char *desc = fdt_getprop(fit, child, FIT_DESC_PROP, &len);

		如果 (!desc)
			continue;

		如果 (!strcmp(desc, str))
			返回 child;
	}

	返回 -ENOENT;
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
static 整数 spl_fit_get_image_name(const struct spl_fit_info *ctx,
				  const char *type, 整数 index,
				  const char **outname)
{
	struct udevice *sysinfo;
	const char *name, *str, *end;
	__maybe_unused 整数 node;
	整数 len, i;
	bool found = true;

	name = fdt_getprop(ctx->fit, ctx->conf_node, type, &len);
	如果 (!name) {
		debug("cannot find property '%s': %d\n", type, len);
		返回 -EINVAL;
	}
	/* A string property should be NUL terminated */
	end = name + len - 1;
	如果 (!len || *end) {
		debug("malformed property '%s'\n", type);
		返回 -EINVAL;
	}

	str = name;
	为 (i = 0; i < index; i++) {
		str = strchr(str, '\0') + 1;
		如果 (str > end) {
			found = false;
			break;
		}
	}

	如果 (!found && CONFIG_IS_ENABLED(SYSINFO) && !sysinfo_get(&sysinfo)) {
		整数 rc;
		/*
		 * no string in the property for this index. Check if the
		 * sysinfo-level code can supply one.
		 */
		rc = sysinfo_detect(sysinfo);
		如果 (rc)
			返回 rc;

		rc = sysinfo_get_fit_loadable(sysinfo, index - i - 1, type,
					      &str);
		如果 (rc && rc != -ENOENT)
			返回 rc;

		如果 (!rc) {
			/*
			 * The sysinfo provided a name for a loadable.
			 * Try to match it against the description properties
			 * first. If no matching node is found, use it as a
			 * node name.
			 */
			整数 node;
			整数 images = fdt_path_offset(ctx->fit, FIT_IMAGES_PATH);

			node = find_node_from_desc(ctx->fit, images, str);
			如果 (node > 0)
				str = fdt_get_name(ctx->fit, node, NULL);

			found = true;
		}
	}

	如果 (!found) {
		debug("no string for index %d\n", index);
		返回 -E2BIG;
	}

	*outname = str;
	返回 0;
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
static 整数 spl_fit_get_image_node(const struct spl_fit_info *ctx,
				  const char *type, 整数 index)
{
	const char *str;
	整数 err;
	整数 node;

	err = spl_fit_get_image_name(ctx, type, index, &str);
	如果 (err)
		返回 err;

	debug("%s: '%s'\n", type, str);

	node = fdt_subnode_offset(ctx->fit, ctx->images_node, str);
	如果 (node < 0) {
		pr_err("cannot find image node '%s': %d\n", str, node);
		返回 -EINVAL;
	}

	返回 node;
}

static u32 get_aligned_image_offset(struct spl_load_info *info, u32 offset)
{
	返回 ALIGN_DOWN(offset, spl_get_bl_len(info));
}

static u32 get_aligned_image_overhead(struct spl_load_info *info, u32 offset)
{
	返回 offset & (spl_get_bl_len(info) - 1);
}

static 整数 get_aligned_image_size(struct spl_load_info *info, ulong data_size,
				  u32 offset, ulong *aligned_size)
{
	u32 overhead = get_aligned_image_overhead(info, offset);

	如果 (data_size > ULONG_MAX - overhead)
		返回 -EOVERFLOW;
	data_size += overhead;

	如果 (data_size > ULONG_MAX - (spl_get_bl_len(info) - 1))
		返回 -EOVERFLOW;
	data_size = ALIGN(data_size, spl_get_bl_len(info));

	*aligned_size = data_size;

	返回 0;
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
static void fit_fill_image_info(const void *fit, 整数 node,
				struct spl_image_info *image_info,
				ulong load_addr, ulong size)
{
	ulong entry_point;

	如果 (!image_info)
		返回;

	image_info->load_addr = load_addr;
	image_info->size = size;

	如果 (!fit_image_get_entry(fit, node, &entry_point))
		image_info->entry_point = entry_point;
	否则
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
static 整数 load_simple_fit(struct spl_load_info *info, ulong fit_offset,
			   const struct spl_fit_info *ctx, 整数 node,
			   struct spl_image_info *image_info, ulong max_size)
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
	整数 ret;

	log_debug("starting\n");
	如果 (CONFIG_IS_ENABLED(BOOTMETH_VBE) &&
	    xpl_get_phase(info) != IH_PHASE_NONE) {
		enum image_phase_t phase;

		ret = fit_image_get_phase(fit, node, &phase);
		/* if the image is for any phase, let's use it */
		如果 (ret == -ENOENT || phase == xpl_get_phase(info)) {
			log_debug("found\n");
		} 否则 如果 (ret < 0) {
			log_debug("err=%d\n", ret);
			返回 ret;
		} 否则 {
			log_debug("- phase mismatch, skipping this image\n");
			返回 -EBADSLT;
		}
	}

	如果 (IS_ENABLED(CONFIG_SPL_FPGA) ||
	    (IS_ENABLED(CONFIG_SPL_OS_BOOT) && spl_decompression_enabled())) {
		如果 (fit_image_get_type(fit, node, &type))
			puts("Cannot get image type.\n");
		否则
			debug("%s ", genimg_get_type_name(type));
	}

	如果 (spl_decompression_enabled()) {
		fit_image_get_comp(fit, node, &image_comp);
		debug("%s ", genimg_get_comp_name(image_comp));
	}

	如果 (fit_image_get_load(fit, node, &load_addr)) {
		如果 (!image_info->load_addr) {
			printf("Can't load %s: No load address and no buffer\n",
			       fit_get_name(fit, node, NULL));
			返回 -ENOBUFS;
		}
		load_addr = image_info->load_addr;
	}

	如果 (!fit_image_get_data_position(fit, node, &offset)) {
		external_data = true;
	} 否则 如果 (!fit_image_get_data_offset(fit, node, &offset)) {
		/* The resulting offset cannot exceed UINT32_MAX */
		如果 (ctx->ext_data_offset > UINT32_MAX - offset) {
			log_debug("Invalid external data offset: %u\n", offset);
			返回 -EINVAL;
		}
		log_debug("read offset %x = offset from fit %x\n", offset,
			  (u32)(offset + ctx->ext_data_offset));
		offset += ctx->ext_data_offset;
		external_data = true;
	}

	如果 (external_data) {
		u32 aligned_offset;
		ulong read_offset;
		void *src_ptr;

		/* External data */
		如果 (fit_image_get_data_size(fit, node, &len))
			返回 -ENOENT;

		/* Dont bother to copy 0 byte data, but warn, though */
		如果 (!len) {
			log_warning("%s: Skip load '%s': image size is 0!\n",
				    __func__, fit_get_name(fit, node, NULL));
			fit_fill_image_info(fit, node, image_info, load_addr, 0);
			返回 0;
		}

		/*
		 * data-size is excluded from the configuration signature (it
		 * is in exc_prop[] in image-fit-sig.c), so it stays attacker
		 * controlled even after fit_config_verify() succeeds. The
		 * image hash is only verified after the device read below, so
		 * an oversized value has to be rejected here.
		 */
		ret = get_aligned_image_size(info, len, offset, &size);
		如果 (ret) {
			log_debug("Invalid external data size: %u\n", len);
			返回 ret;
		}

		如果 (size > max_size) {
			log_debug("Image too large: aligned size %lu, max %lu (data-size %u)\n",
				  size, max_size, len);
			返回 -EFBIG;
		}

		aligned_offset = get_aligned_image_offset(info, offset);
		如果 (aligned_offset > ULONG_MAX - fit_offset) {
			log_debug("Invalid aligned external data offset: %u\n",
				  aligned_offset);
			返回 -EINVAL;
		}
		read_offset = fit_offset + aligned_offset;

		如果 (spl_decompression_enabled() &&
		    (image_comp == IH_COMP_GZIP || image_comp == IH_COMP_LZMA))
			src_ptr = map_sysmem(ALIGN(CONFIG_SYS_LOAD_ADDR, ARCH_DMA_MINALIGN), len);
		否则
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
		如果 (size > ULONG_MAX - read_offset ||
		    size > UINTPTR_MAX - (uintptr_t)src_ptr
		/*
		 * #if (not a runtime if) is required: FIT_SIGNATURE_MAX_SIZE
		 * depends on FIT_SIGNATURE, so CONFIG_VAL(FIT_SIGNATURE_MAX_SIZE)
		 * is undefined when signing is disabled and referencing it
		 * here would fail to compile.
		 */
#如果 CONFIG_IS_ENABLED(FIT_SIGNATURE)
		    || offset > CONFIG_VAL(FIT_SIGNATURE_MAX_SIZE) ||
		    len > CONFIG_VAL(FIT_SIGNATURE_MAX_SIZE) - offset
#endif
		) {
			log_debug("FIT external data is out of bounds (offset=%u, size=%u)\n",
				  offset, len);
			返回 -EINVAL;
		}

		log_debug("reading from offset %x / %lx size %lx to %p: ",
			  offset, read_offset, size, src_ptr);

		如果 (info->read(info, read_offset, size, src_ptr) < length)
			返回 -EIO;

		debug("External data: dst=%p, offset=%x, size=%lx\n",
		      src_ptr, offset, (unsigned long)length);
		src = src_ptr + get_aligned_image_overhead(info, offset);
	} 否则 {
		/* Embedded data */
		如果 (fit_image_get_emb_data(fit, node, &data, &length)) {
			puts("Cannot get image data/size\n");
			返回 -ENOENT;
		}
		debug("Embedded data: dst=%lx, size=%lx\n", load_addr,
		      (unsigned long)length);
		src = (void *)data;	/* cast away const */
	}

	如果 (CONFIG_IS_ENABLED(FIT_SIGNATURE)) {
		printf("## Checking hash(es) for Image %s ... ",
		       fit_get_name(fit, node, NULL));
		如果 (!fit_image_verify_with_data(fit, node, gd_fdt_blob(), src,
						length))
			返回 -EPERM;
		puts("OK\n");
	}

	如果 (CONFIG_IS_ENABLED(FIT_IMAGE_POST_PROCESS))
		board_fit_image_post_process(fit, node, &src, &length);

	load_ptr = map_sysmem(load_addr, length);
	如果 (IS_ENABLED(CONFIG_SPL_GZIP) && image_comp == IH_COMP_GZIP) {
		size = length;
		如果 (gunzip(load_ptr, CONFIG_SYS_BOOTM_LEN, src, &size)) {
			puts("Uncompressing error\n");
			返回 -EIO;
		}
		length = size;
	} 否则 如果 (IS_ENABLED(CONFIG_SPL_LZMA) && image_comp == IH_COMP_LZMA) {
		size = CONFIG_SYS_BOOTM_LEN;
		ulong loadEnd;

		如果 (image_decomp(IH_COMP_LZMA, CONFIG_SYS_LOAD_ADDR, 0, 0,
				 load_ptr, src, length, size, &loadEnd)) {
			puts("Uncompressing error\n");
			返回 -EIO;
		}
		length = loadEnd - CONFIG_SYS_LOAD_ADDR;
	} 否则 {
		memmove(load_ptr, src, length);
	}

	fit_fill_image_info(fit, node, image_info, load_addr, length);
	log_debug("- done loading\n");

	upl_add_image(fit, node, load_addr, length);

	返回 0;
}

static bool os_takes_devicetree(uint8_t os)
{
	switch (os) {
	case IH_OS_U_BOOT:
		返回 true;
	case IH_OS_LINUX:
		返回 IS_ENABLED(CONFIG_SPL_OS_BOOT) ||
		       IS_ENABLED(CONFIG_SPL_OPENSBI);
	default:
		返回 false;
	}
}

__weak 整数 board_spl_fit_append_fdt_skip(const char *name)
{
	返回 0;	/* Do not skip */
}

static 整数 spl_fit_append_fdt(struct spl_image_info *spl_image,
			      struct spl_load_info *info, ulong offset,
			      const struct spl_fit_info *ctx)
{
	struct spl_image_info image_info;
	整数 node, ret = 0, index = 0;

	/*
	 * Use the address following the image as target address for the
	 * device tree.
	 */
	image_info.load_addr = ALIGN(spl_image->load_addr + spl_image->size, 8);

	/* Figure out which device tree the board wants to use */
	node = spl_fit_get_image_node(ctx, FIT_FDT_PROP, index++);
	如果 (node < 0) {
		size_t size;

		debug("%s: cannot find FDT node\n", __func__);

		/*
		 * U-Boot did not find a device tree inside the FIT image. Use
		 * the U-Boot device tree instead.
		 */
		如果 (!gd->fdt_blob)
			返回 node;

		/*
		 * Make the load-address of the FDT available for the SPL
		 * framework
		 */
		size = fdt_totalsize(gd->fdt_blob);
		spl_image->fdt_addr = map_sysmem(image_info.load_addr, size);
		memcpy(spl_image->fdt_addr, gd->fdt_blob, size);
	} 否则 {
		ret = load_simple_fit(info, offset, ctx, node, &image_info,
				      CONFIG_SYS_BOOTM_LEN);
		如果 (ret < 0)
			返回 ret;

		spl_image->fdt_addr = phys_to_virt(image_info.load_addr);
	}

	如果 (CONFIG_IS_ENABLED(FIT_IMAGE_TINY))
		返回 0;

#如果 CONFIG_IS_ENABLED(LOAD_FIT_APPLY_OVERLAY)
		void *tmpbuffer = NULL;

		为 (; ; index++) {
			const char *str;

			ret = spl_fit_get_image_name(ctx, FIT_FDT_PROP, index, &str);
			如果 (ret == -E2BIG) {
				debug("%s: No additional FDT node\n", __func__);
				ret = 0;
				break;
			} 否则 如果 (ret) {
				break;
			}

			ret = board_spl_fit_append_fdt_skip(str);
			如果 (ret)
				continue;

			node = fdt_subnode_offset(ctx->fit, ctx->images_node, str);
			如果 (node < 0) {
				debug("%s: unable to find FDT node %d\n",
				      __func__, index);
				continue;
			}

			如果 (!tmpbuffer) {
				/*
				 * allocate memory to store the DT overlay
				 * before it is applied. It may not be used
				 * depending on how the overlay is stored, so
				 * don't fail yet if the allocation failed.
				 */
				size_t size = CONFIG_SPL_LOAD_FIT_APPLY_OVERLAY_BUF_SZ;

				tmpbuffer = malloc_cache_aligned(size);
				如果 (!tmpbuffer)
					debug("%s: unable to allocate space for overlays\n",
					      __func__);
			}
			image_info.load_addr = (ulong)tmpbuffer;
			ret = load_simple_fit(info, offset, ctx, node,
					      &image_info,
					      CONFIG_SPL_LOAD_FIT_APPLY_OVERLAY_BUF_SZ);
			如果 (ret == -EBADSLT)
				continue;
			否则 如果 (ret < 0)
				break;

			/* Make room in FDT for changes from the overlay */
			ret = fdt_increase_size(spl_image->fdt_addr,
						image_info.size);
			如果 (ret < 0)
				break;

			ret = fdt_overlay_apply_verbose(spl_image->fdt_addr,
							(void *)image_info.load_addr);
			如果 (ret) {
				pr_err("failed to apply DT overlay %s\n",
				       fit_get_name(ctx->fit, node, NULL));
				break;
			}

			debug("%s: DT overlay %s applied\n", __func__,
			      fit_get_name(ctx->fit, node, NULL));
		}
		free(tmpbuffer);
		如果 (ret)
			返回 ret;
#endif
	/* Try to make space, so we can inject details on the loadables */
	ret = fdt_shrink_to_minimum(spl_image->fdt_addr, 8192);
	如果 (ret < 0)
		返回 ret;

	返回 ret;
}

static 整数 spl_fit_record_loadable(const struct spl_fit_info *ctx, 整数 index,
				   void *blob, struct spl_image_info *image)
{
	整数 ret = 0;
	const char *name;
	整数 node;

	ret = spl_fit_get_image_name(ctx, FIT_LOADABLE_PROP, index, &name);
	如果 (ret < 0)
		返回 ret;

	node = spl_fit_get_image_node(ctx, FIT_LOADABLE_PROP, index);

	ret = fdt_record_loadable(blob, index, name, image->load_addr,
			image->size, image->entry_point,
			fdt_getprop(ctx->fit, node, FIT_TYPE_PROP, NULL),
			fdt_getprop(ctx->fit, node, FIT_OS_PROP, NULL),
			fdt_getprop(ctx->fit, node, FIT_ARCH_PROP, NULL));

	返回 ret;
}

static 整数 spl_fit_image_is_fpga(const void *fit, 整数 node)
{
	const char *type;

	如果 (!IS_ENABLED(CONFIG_SPL_FPGA))
		返回 0;

	type = fdt_getprop(fit, node, FIT_TYPE_PROP, NULL);
	如果 (!type)
		返回 0;

	返回 !strcmp(type, "fpga");
}

static void spl_fit_image_record_arm32_optee(const void *fit, 整数 node,
					     struct spl_image_info *spl_image,
					     struct spl_image_info *image_info)
{
#如果 defined(CONFIG_BOOTM_OPTEE) && defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
	const char *type = fdt_getprop(fit, node, FIT_TYPE_PROP, NULL);

	如果 (!type)
		返回;

	如果 (strcmp(type, "tee"))
		返回;

	spl_image->optee_addr = image_info->load_addr;
#endif
}

static 整数 spl_fit_image_get_os(const void *fit, 整数 noffset, uint8_t *os)
{
	如果 (!CONFIG_IS_ENABLED(FIT_IMAGE_TINY) || CONFIG_IS_ENABLED(OS_BOOT))
		返回 fit_image_get_os(fit, noffset, os);

	const char *name = fdt_getprop(fit, noffset, FIT_OS_PROP, NULL);
	如果 (!name)
		返回 -ENOENT;

	/*
	 * We don't care what the type of the image actually is,
	 * only whether or not it is U-Boot. This saves some
	 * space by omitting the large table of OS types.
	 */
	如果 (!strcmp(name, "u-boot"))
		*os = IH_OS_U_BOOT;
	否则
		*os = IH_OS_INVALID;

	返回 0;
}

/*
 * The purpose of the FIT load buffer is to provide a memory location that is
 * independent of the load address of any FIT component.
 */
__weak void *board_spl_fit_buffer_addr(ulong fit_size, 整数 sectors, 整数 bl_len)
{
	void *buf;
	size_t size = sectors * bl_len;

	buf = malloc_cache_aligned(size);
	如果 (!buf) {
		pr_err("Could not get FIT buffer of %lu bytes\n", (ulong)size);

		如果 (IS_ENABLED(CONFIG_SPL_SYS_MALLOC))
			pr_err("\tcheck CONFIG_SPL_SYS_MALLOC_SIZE\n");
		否则
			pr_err("\tcheck CONFIG_SPL_SYS_MALLOC_F_LEN\n");

		buf = spl_get_load_buffer(0, size);
	}
	返回 buf;
}

/*
 * Weak default function to allow customizing SPL fit loading for load-only
 * use cases by allowing to skip the parsing/processing of the FIT contents
 * (so that this can be done separately in a more customized fashion)
 */
__weak bool spl_load_simple_fit_skip_processing(void)
{
	返回 false;
}

/*
 * Weak default function to allow fixes after fit header
 * is loaded.
 */
__weak void *spl_load_simple_fit_fix_load(const void *fit)
{
	返回 (void *)fit;
}

static void warn_deprecated(const char *msg)
{
	printf("DEPRECATED: %s\n", msg);
	printf("\tSee https://fitspec.osfw.foundation/\n");
}

static 整数 spl_fit_upload_fpga(struct spl_fit_info *ctx, 整数 node,
			       struct spl_image_info *fpga_image)
{
	const char *compatible;
	整数 ret;
	整数 devnum = 0;
	整数 flags = 0;

	debug("FPGA bitstream at: %x, size: %x\n",
	      (u32)fpga_image->load_addr, fpga_image->size);

	compatible = fdt_getprop(ctx->fit, node, FIT_COMPAT_PROP, NULL);
	如果 (!compatible) {
		warn_deprecated("'fpga' image without 'compatible' property");
	} 否则 {
		如果 (CONFIG_IS_ENABLED(FPGA_LOAD_SECURE))
			flags = fpga_compatible2flag(devnum, compatible);
		如果 (strcmp(compatible, "u-boot,fpga-legacy"))
			debug("Ignoring compatible = %s property\n",
			      compatible);
	}

	ret = fpga_load(devnum, (void *)fpga_image->load_addr,
			fpga_image->size, BIT_FULL, flags);
	如果 (ret) {
		printf("%s: Cannot load the image to the FPGA\n", __func__);
		返回 ret;
	}

	puts("FPGA image loaded from FIT\n");

	返回 0;
}

static 整数 spl_fit_load_fpga(struct spl_fit_info *ctx,
			     struct spl_load_info *info, ulong offset)
{
	整数 node, ret;

	struct spl_image_info fpga_image = {
		.load_addr = 0,
	};

	node = spl_fit_get_image_node(ctx, FIT_FPGA_PROP, 0);
	如果 (node < 0)
		返回 node;

	warn_deprecated("'fpga' property in config node. Use 'loadables'");

	/* Load the image and set up the fpga_image structure */
	ret = load_simple_fit(info, offset, ctx, node, &fpga_image,
			      CONFIG_SYS_BOOTM_LEN);
	如果 (ret) {
		printf("%s: Cannot load the FPGA: %i\n", __func__, ret);
		返回 ret;
	}

	返回 spl_fit_upload_fpga(ctx, node, &fpga_image);
}

static 整数 spl_simple_fit_read(struct spl_fit_info *ctx,
			       struct spl_load_info *info, ulong offset,
			       const void *fit_header)
{
	unsigned long aligned_size, count, size;
	void *buf;
	整数 ret;

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
	如果 (ret) {
		log_debug("Invalid FIT size: %lu\n", size);
		返回 ret;
	}
	buf = board_spl_fit_buffer_addr(aligned_size, aligned_size, 1);
	如果 (!buf) {
		/*
		 * We assume that none of the board will ever use 0x0 as a
		 * valid load address. Theoretically some board could use it,
		 * but this is extremely unlikely.
		 */
		返回 -EIO;
	}

	count = info->read(info, offset, aligned_size, buf);
	如果 (!count) {
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
		返回 -EIO;
	}

	ctx->fit = buf;
	debug("fit read offset %lx, size=%lu, dst=%p, count=%lu\n",
	      offset, aligned_size, buf, count);

	返回 0;
}

static 整数 spl_simple_fit_parse(struct spl_fit_info *ctx)
{
	/* Find the correct subnode under "/configurations" */
	ctx->conf_node = fit_find_config_node(ctx->fit);
	如果 (ctx->conf_node < 0)
		返回 -EINVAL;

	如果 (CONFIG_IS_ENABLED(FIT_SIGNATURE)) {
		printf("## Checking hash(es) for config %s ... ",
		       fit_get_name(ctx->fit, ctx->conf_node, NULL));
		如果 (fit_config_verify(ctx->fit, ctx->conf_node))
			返回 -EPERM;
		puts("OK\n");
	}

	/* find the node holding the images information */
	ctx->images_node = fdt_path_offset(ctx->fit, FIT_IMAGES_PATH);
	如果 (ctx->images_node < 0) {
		debug("%s: Cannot find /images node: %d\n", __func__,
		      ctx->images_node);
		返回 -EINVAL;
	}

	返回 0;
}

整数 spl_load_simple_fit(struct spl_image_info *spl_image,
			struct spl_load_info *info, ulong offset, void *fit)
{
	struct spl_image_info image_info;
	struct spl_fit_info ctx;
	整数 node = -1;
	整数 ret;
	整数 index = 0;
	整数 firmware_node;

	ret = spl_simple_fit_read(&ctx, info, offset, fit);
	如果 (ret < 0)
		返回 ret;

	/* skip further processing if requested to enable load-only use cases */
	如果 (spl_load_simple_fit_skip_processing())
		返回 0;

	ctx.fit = spl_load_simple_fit_fix_load(ctx.fit);

	ret = spl_simple_fit_parse(&ctx);
	如果 (ret < 0)
		返回 ret;

	如果 (IS_ENABLED(CONFIG_SPL_FPGA))
		spl_fit_load_fpga(&ctx, info, offset);

	/*
	 * Find the U-Boot image using the following search order:
	 *   - start at 'firmware' (e.g. an ARM Trusted Firmware)
	 *   - fall back 'kernel' (e.g. a Falcon-mode OS boot
	 *   - fall back to using the first 'loadables' entry
	 */
	如果 (node < 0)
		node = spl_fit_get_image_node(&ctx, FIT_FIRMWARE_PROP, 0);

	如果 (node < 0 && IS_ENABLED(CONFIG_SPL_OS_BOOT))
		node = spl_fit_get_image_node(&ctx, FIT_KERNEL_PROP, 0);

	如果 (node < 0) {
		debug("could not find firmware image, trying loadables...\n");
		node = spl_fit_get_image_node(&ctx, FIT_LOADABLE_PROP, 0);
		/*
		 * If we pick the U-Boot image from "loadables", start at
		 * the second image when later loading additional images.
		 */
		index = 1;
	}
	如果 (node < 0) {
		debug("%s: Cannot find u-boot image node: %d\n",
		      __func__, node);
		返回 -1;
	}

	/* Load the image and set up the spl_image structure */
	ret = load_simple_fit(info, offset, &ctx, node, spl_image,
			      CONFIG_SYS_BOOTM_LEN);
	如果 (ret)
		返回 ret;

	/*
	 * For backward compatibility, we treat the first node that is
	 * as a U-Boot image, if no OS-type has been declared.
	 */
	如果 (!spl_fit_image_get_os(ctx.fit, node, &spl_image->os))
		debug("Image OS is %s\n", genimg_get_os_name(spl_image->os));
	否则 如果 (!IS_ENABLED(CONFIG_SPL_OS_BOOT))
		spl_image->os = IH_OS_U_BOOT;

	/*
	 * Booting a next-stage U-Boot may require us to append the FDT.
	 * We allow this to fail, as the U-Boot image might embed its FDT.
	 */
	如果 (os_takes_devicetree(spl_image->os)) {
		ret = spl_fit_append_fdt(spl_image, info, offset, &ctx);
		如果 (ret < 0 && spl_image->os != IH_OS_U_BOOT)
			返回 ret;
	}

	firmware_node = node;
	/* Now check if there are more images for us to load */
	为 (; ; index++) {
		uint8_t os_type = IH_OS_INVALID;

		node = spl_fit_get_image_node(&ctx, FIT_LOADABLE_PROP, index);
		如果 (node < 0)
			break;

		/*
		 * if the firmware is also a loadable, skip it because
		 * it already has been loaded. This is typically the case with
		 * u-boot.img generated by mkimage.
		 */
		如果 (firmware_node == node)
			continue;

		image_info.load_addr = 0;
		ret = load_simple_fit(info, offset, &ctx, node, &image_info,
				      CONFIG_SYS_BOOTM_LEN);
		如果 (ret < 0 && ret != -EBADSLT) {
			printf("%s: can't load image loadables index %d (ret = %d)\n",
			       __func__, index, ret);
			返回 ret;
		}

		如果 (spl_fit_image_is_fpga(ctx.fit, node))
			spl_fit_upload_fpga(&ctx, node, &image_info);

		如果 (!spl_fit_image_get_os(ctx.fit, node, &os_type))
			debug("Loadable is %s\n", genimg_get_os_name(os_type));

		如果 (os_takes_devicetree(os_type)) {
			spl_fit_append_fdt(&image_info, info, offset, &ctx);
			spl_image->fdt_addr = image_info.fdt_addr;
		}

		/*
		 * If the "firmware" image did not provide an entry point,
		 * use the first valid entry point from the loadables.
		 */
		如果 (spl_image->entry_point == FDT_ERROR &&
		    image_info.entry_point != FDT_ERROR)
			spl_image->entry_point = image_info.entry_point;

		spl_fit_image_record_arm32_optee(ctx.fit, node, spl_image,
						 &image_info);

		/* Record our loadables into the FDT */
		如果 (!CONFIG_IS_ENABLED(FIT_IMAGE_TINY) &&
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
	如果 (spl_image->entry_point == FDT_ERROR || spl_image->entry_point == 0)
		spl_image->entry_point = spl_image->load_addr;

	spl_image->flags |= SPL_FIT_FOUND;
	upl_set_fit_info(map_to_sysmem(ctx.fit), ctx.conf_node,
			 spl_image->entry_point);

	返回 0;
}

/* Parse and load full fitImage in SPL */
整数 spl_load_fit_image(struct spl_image_info *spl_image,
		       const struct legacy_img_hdr *header)
{
	struct bootm_headers images;
	const char *fit_uname_config = NULL;
	ulong fdt_hack;
	const char *uname;
	ulong fw_data = 0, dt_data = 0, img_data = 0;
	ulong fw_len = 0, dt_len = 0, img_len = 0;
	整数 idx, conf_noffset;
	整数 ret;

	images.verify = CONFIG_IS_ENABLED(FIT_SIGNATURE);

	ret = fit_image_load(&images, virt_to_phys((void *)header),
			     NULL, &fit_uname_config, IH_ARCH_DEFAULT,
			     IH_TYPE_FIRMWARE, -1, FIT_LOAD_OPTIONAL,
			     &fw_data, &fw_len);

	如果 (ret < 0) {
		ret = fit_image_load(&images, virt_to_phys((void *)header),
				     NULL, &fit_uname_config, IH_ARCH_DEFAULT,
				     IH_TYPE_KERNEL, -1, FIT_LOAD_OPTIONAL,
				     &fw_data, &fw_len);
	}

	如果 (ret < 0)
		返回 ret;

	spl_image->size = fw_len;
	spl_image->load_addr = fw_data;
	如果 (fit_image_get_entry(header, ret, &spl_image->entry_point))
		spl_image->entry_point = fw_data;
	如果 (fit_image_get_os(header, ret, &spl_image->os))
		spl_image->os = IH_OS_INVALID;
	spl_image->name = genimg_get_os_name(spl_image->os);

	debug(PHASE_PROMPT "payload image: %32s load addr: 0x%lx size: %d\n",
	      spl_image->name, spl_image->load_addr, spl_image->size);

	images.verify = CONFIG_IS_ENABLED(FIT_SIGNATURE);

	ret = fit_image_load(&images, virt_to_phys((void *)header), NULL,
			     &fit_uname_config, IH_ARCH_DEFAULT, IH_TYPE_FLATDT,
			     -1, FIT_LOAD_OPTIONAL, &dt_data, &dt_len);
	如果 (ret >= 0) {
		如果 (spl_image->os == IH_OS_U_BOOT) {
			/* HACK: U-Boot expects FDT at a specific address */
			fdt_hack = ALIGN(spl_image->load_addr + spl_image->size, 8);
			debug("Relocating FDT to %p\n", (void *)fdt_hack);
			memcpy(map_sysmem(fdt_hack, dt_len),
			       map_sysmem(dt_data, 0), dt_len);
			spl_image->fdt_addr = (void *)fdt_hack;
		} 否则 {
			spl_image->fdt_addr = (void *)dt_data;
		}
	}

	conf_noffset = fit_conf_get_node((const void *)header,
					 fit_uname_config);
	如果 (conf_noffset < 0)
		返回 0;

	为 (idx = 0;
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
		如果 (ret < 0)
			返回 ret;
	}
	spl_image->flags |= SPL_FIT_FOUND;

	upl_set_fit_info(map_to_sysmem(header), conf_noffset,
			 spl_image->entry_point);

	返回 0;
}
