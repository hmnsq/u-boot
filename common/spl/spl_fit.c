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
	цео images_node;	/* FDT offset to "/images" node */
	цео conf_node;		/* FDT offset to selected configuration node */
};

__weak ulong board_spl_fit_size_align(ulong size)
{
	врати size;
}

static цео find_node_from_desc(const void *fit, цео node, const char *str)
{
	цео child;

	ако (node < 0)
		врати -EINVAL;

	/* iterate the FIT nodes and find a matching description */
	за (child = fdt_first_subnode(fit, node); child >= 0;
	     child = fdt_next_subnode(fit, child)) {
		цео len;
		const char *desc = fdt_getprop(fit, child, FIT_DESC_PROP, &len);

		ако (!desc)
			continue;

		ако (!strcmp(desc, str))
			врати child;
	}

	врати -ENOENT;
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
static цео spl_fit_get_image_name(const struct spl_fit_info *ctx,
				  const char *type, цео index,
				  const char **outname)
{
	struct udevice *sysinfo;
	const char *name, *str, *end;
	__maybe_unused цео node;
	цео len, i;
	bool found = true;

	name = fdt_getprop(ctx->fit, ctx->conf_node, type, &len);
	ако (!name) {
		debug("cannot find property '%s': %d\n", type, len);
		врати -EINVAL;
	}
	/* A string property should be NUL terminated */
	end = name + len - 1;
	ако (!len || *end) {
		debug("malformed property '%s'\n", type);
		врати -EINVAL;
	}

	str = name;
	за (i = 0; i < index; i++) {
		str = strchr(str, '\0') + 1;
		ако (str > end) {
			found = false;
			break;
		}
	}

	ако (!found && CONFIG_IS_ENABLED(SYSINFO) && !sysinfo_get(&sysinfo)) {
		цео rc;
		/*
		 * no string in the property for this index. Check if the
		 * sysinfo-level code can supply one.
		 */
		rc = sysinfo_detect(sysinfo);
		ако (rc)
			врати rc;

		rc = sysinfo_get_fit_loadable(sysinfo, index - i - 1, type,
					      &str);
		ако (rc && rc != -ENOENT)
			врати rc;

		ако (!rc) {
			/*
			 * The sysinfo provided a name for a loadable.
			 * Try to match it against the description properties
			 * first. If no matching node is found, use it as a
			 * node name.
			 */
			цео node;
			цео images = fdt_path_offset(ctx->fit, FIT_IMAGES_PATH);

			node = find_node_from_desc(ctx->fit, images, str);
			ако (node > 0)
				str = fdt_get_name(ctx->fit, node, NULL);

			found = true;
		}
	}

	ако (!found) {
		debug("no string for index %d\n", index);
		врати -E2BIG;
	}

	*outname = str;
	врати 0;
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
static цео spl_fit_get_image_node(const struct spl_fit_info *ctx,
				  const char *type, цео index)
{
	const char *str;
	цео err;
	цео node;

	err = spl_fit_get_image_name(ctx, type, index, &str);
	ако (err)
		врати err;

	debug("%s: '%s'\n", type, str);

	node = fdt_subnode_offset(ctx->fit, ctx->images_node, str);
	ако (node < 0) {
		pr_err("cannot find image node '%s': %d\n", str, node);
		врати -EINVAL;
	}

	врати node;
}

static u32 get_aligned_image_offset(struct spl_load_info *info, u32 offset)
{
	врати ALIGN_DOWN(offset, spl_get_bl_len(info));
}

static u32 get_aligned_image_overhead(struct spl_load_info *info, u32 offset)
{
	врати offset & (spl_get_bl_len(info) - 1);
}

static цео get_aligned_image_size(struct spl_load_info *info, ulong data_size,
				  u32 offset, ulong *aligned_size)
{
	u32 overhead = get_aligned_image_overhead(info, offset);

	ако (data_size > ULONG_MAX - overhead)
		врати -EOVERFLOW;
	data_size += overhead;

	ако (data_size > ULONG_MAX - (spl_get_bl_len(info) - 1))
		врати -EOVERFLOW;
	data_size = ALIGN(data_size, spl_get_bl_len(info));

	*aligned_size = data_size;

	врати 0;
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
static void fit_fill_image_info(const void *fit, цео node,
				struct spl_image_info *image_info,
				ulong load_addr, ulong size)
{
	ulong entry_point;

	ако (!image_info)
		врати;

	image_info->load_addr = load_addr;
	image_info->size = size;

	ако (!fit_image_get_entry(fit, node, &entry_point))
		image_info->entry_point = entry_point;
	инако
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
static цео load_simple_fit(struct spl_load_info *info, ulong fit_offset,
			   const struct spl_fit_info *ctx, цео node,
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
	цео ret;

	log_debug("starting\n");
	ако (CONFIG_IS_ENABLED(BOOTMETH_VBE) &&
	    xpl_get_phase(info) != IH_PHASE_NONE) {
		enum image_phase_t phase;

		ret = fit_image_get_phase(fit, node, &phase);
		/* if the image is for any phase, let's use it */
		ако (ret == -ENOENT || phase == xpl_get_phase(info)) {
			log_debug("found\n");
		} инако ако (ret < 0) {
			log_debug("err=%d\n", ret);
			врати ret;
		} инако {
			log_debug("- phase mismatch, skipping this image\n");
			врати -EBADSLT;
		}
	}

	ако (IS_ENABLED(CONFIG_SPL_FPGA) ||
	    (IS_ENABLED(CONFIG_SPL_OS_BOOT) && spl_decompression_enabled())) {
		ако (fit_image_get_type(fit, node, &type))
			puts("Cannot get image type.\n");
		инако
			debug("%s ", genimg_get_type_name(type));
	}

	ако (spl_decompression_enabled()) {
		fit_image_get_comp(fit, node, &image_comp);
		debug("%s ", genimg_get_comp_name(image_comp));
	}

	ако (fit_image_get_load(fit, node, &load_addr)) {
		ако (!image_info->load_addr) {
			printf("Can't load %s: No load address and no buffer\n",
			       fit_get_name(fit, node, NULL));
			врати -ENOBUFS;
		}
		load_addr = image_info->load_addr;
	}

	ако (!fit_image_get_data_position(fit, node, &offset)) {
		external_data = true;
	} инако ако (!fit_image_get_data_offset(fit, node, &offset)) {
		/* The resulting offset cannot exceed UINT32_MAX */
		ако (ctx->ext_data_offset > UINT32_MAX - offset) {
			log_debug("Invalid external data offset: %u\n", offset);
			врати -EINVAL;
		}
		log_debug("read offset %x = offset from fit %x\n", offset,
			  (u32)(offset + ctx->ext_data_offset));
		offset += ctx->ext_data_offset;
		external_data = true;
	}

	ако (external_data) {
		u32 aligned_offset;
		ulong read_offset;
		void *src_ptr;

		/* External data */
		ако (fit_image_get_data_size(fit, node, &len))
			врати -ENOENT;

		/* Dont bother to copy 0 byte data, but warn, though */
		ако (!len) {
			log_warning("%s: Skip load '%s': image size is 0!\n",
				    __func__, fit_get_name(fit, node, NULL));
			fit_fill_image_info(fit, node, image_info, load_addr, 0);
			врати 0;
		}

		/*
		 * data-size is excluded from the configuration signature (it
		 * is in exc_prop[] in image-fit-sig.c), so it stays attacker
		 * controlled even after fit_config_verify() succeeds. The
		 * image hash is only verified after the device read below, so
		 * an oversized value has to be rejected here.
		 */
		ret = get_aligned_image_size(info, len, offset, &size);
		ако (ret) {
			log_debug("Invalid external data size: %u\n", len);
			врати ret;
		}

		ако (size > max_size) {
			log_debug("Image too large: aligned size %lu, max %lu (data-size %u)\n",
				  size, max_size, len);
			врати -EFBIG;
		}

		aligned_offset = get_aligned_image_offset(info, offset);
		ако (aligned_offset > ULONG_MAX - fit_offset) {
			log_debug("Invalid aligned external data offset: %u\n",
				  aligned_offset);
			врати -EINVAL;
		}
		read_offset = fit_offset + aligned_offset;

		ако (spl_decompression_enabled() &&
		    (image_comp == IH_COMP_GZIP || image_comp == IH_COMP_LZMA))
			src_ptr = map_sysmem(ALIGN(CONFIG_SYS_LOAD_ADDR, ARCH_DMA_MINALIGN), len);
		инако
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
		ако (size > ULONG_MAX - read_offset ||
		    size > UINTPTR_MAX - (uintptr_t)src_ptr
		/*
		 * #if (not a runtime if) is required: FIT_SIGNATURE_MAX_SIZE
		 * depends on FIT_SIGNATURE, so CONFIG_VAL(FIT_SIGNATURE_MAX_SIZE)
		 * is undefined when signing is disabled and referencing it
		 * here would fail to compile.
		 */
#ако CONFIG_IS_ENABLED(FIT_SIGNATURE)
		    || offset > CONFIG_VAL(FIT_SIGNATURE_MAX_SIZE) ||
		    len > CONFIG_VAL(FIT_SIGNATURE_MAX_SIZE) - offset
#endif
		) {
			log_debug("FIT external data is out of bounds (offset=%u, size=%u)\n",
				  offset, len);
			врати -EINVAL;
		}

		log_debug("reading from offset %x / %lx size %lx to %p: ",
			  offset, read_offset, size, src_ptr);

		ако (info->read(info, read_offset, size, src_ptr) < length)
			врати -EIO;

		debug("External data: dst=%p, offset=%x, size=%lx\n",
		      src_ptr, offset, (unsigned long)length);
		src = src_ptr + get_aligned_image_overhead(info, offset);
	} инако {
		/* Embedded data */
		ако (fit_image_get_emb_data(fit, node, &data, &length)) {
			puts("Cannot get image data/size\n");
			врати -ENOENT;
		}
		debug("Embedded data: dst=%lx, size=%lx\n", load_addr,
		      (unsigned long)length);
		src = (void *)data;	/* cast away const */
	}

	ако (CONFIG_IS_ENABLED(FIT_SIGNATURE)) {
		printf("## Checking hash(es) for Image %s ... ",
		       fit_get_name(fit, node, NULL));
		ако (!fit_image_verify_with_data(fit, node, gd_fdt_blob(), src,
						length))
			врати -EPERM;
		puts("OK\n");
	}

	ако (CONFIG_IS_ENABLED(FIT_IMAGE_POST_PROCESS))
		board_fit_image_post_process(fit, node, &src, &length);

	load_ptr = map_sysmem(load_addr, length);
	ако (IS_ENABLED(CONFIG_SPL_GZIP) && image_comp == IH_COMP_GZIP) {
		size = length;
		ако (gunzip(load_ptr, CONFIG_SYS_BOOTM_LEN, src, &size)) {
			puts("Uncompressing error\n");
			врати -EIO;
		}
		length = size;
	} инако ако (IS_ENABLED(CONFIG_SPL_LZMA) && image_comp == IH_COMP_LZMA) {
		size = CONFIG_SYS_BOOTM_LEN;
		ulong loadEnd;

		ако (image_decomp(IH_COMP_LZMA, CONFIG_SYS_LOAD_ADDR, 0, 0,
				 load_ptr, src, length, size, &loadEnd)) {
			puts("Uncompressing error\n");
			врати -EIO;
		}
		length = loadEnd - CONFIG_SYS_LOAD_ADDR;
	} инако {
		memmove(load_ptr, src, length);
	}

	fit_fill_image_info(fit, node, image_info, load_addr, length);
	log_debug("- done loading\n");

	upl_add_image(fit, node, load_addr, length);

	врати 0;
}

static bool os_takes_devicetree(uint8_t os)
{
	switch (os) {
	case IH_OS_U_BOOT:
		врати true;
	case IH_OS_LINUX:
		врати IS_ENABLED(CONFIG_SPL_OS_BOOT) ||
		       IS_ENABLED(CONFIG_SPL_OPENSBI);
	default:
		врати false;
	}
}

__weak цео board_spl_fit_append_fdt_skip(const char *name)
{
	врати 0;	/* Do not skip */
}

static цео spl_fit_append_fdt(struct spl_image_info *spl_image,
			      struct spl_load_info *info, ulong offset,
			      const struct spl_fit_info *ctx)
{
	struct spl_image_info image_info;
	цео node, ret = 0, index = 0;

	/*
	 * Use the address following the image as target address for the
	 * device tree.
	 */
	image_info.load_addr = ALIGN(spl_image->load_addr + spl_image->size, 8);

	/* Figure out which device tree the board wants to use */
	node = spl_fit_get_image_node(ctx, FIT_FDT_PROP, index++);
	ако (node < 0) {
		size_t size;

		debug("%s: cannot find FDT node\n", __func__);

		/*
		 * U-Boot did not find a device tree inside the FIT image. Use
		 * the U-Boot device tree instead.
		 */
		ако (!gd->fdt_blob)
			врати node;

		/*
		 * Make the load-address of the FDT available for the SPL
		 * framework
		 */
		size = fdt_totalsize(gd->fdt_blob);
		spl_image->fdt_addr = map_sysmem(image_info.load_addr, size);
		memcpy(spl_image->fdt_addr, gd->fdt_blob, size);
	} инако {
		ret = load_simple_fit(info, offset, ctx, node, &image_info,
				      CONFIG_SYS_BOOTM_LEN);
		ако (ret < 0)
			врати ret;

		spl_image->fdt_addr = phys_to_virt(image_info.load_addr);
	}

	ако (CONFIG_IS_ENABLED(FIT_IMAGE_TINY))
		врати 0;

#ако CONFIG_IS_ENABLED(LOAD_FIT_APPLY_OVERLAY)
		void *tmpbuffer = NULL;

		за (; ; index++) {
			const char *str;

			ret = spl_fit_get_image_name(ctx, FIT_FDT_PROP, index, &str);
			ако (ret == -E2BIG) {
				debug("%s: No additional FDT node\n", __func__);
				ret = 0;
				break;
			} инако ако (ret) {
				break;
			}

			ret = board_spl_fit_append_fdt_skip(str);
			ако (ret)
				continue;

			node = fdt_subnode_offset(ctx->fit, ctx->images_node, str);
			ако (node < 0) {
				debug("%s: unable to find FDT node %d\n",
				      __func__, index);
				continue;
			}

			ако (!tmpbuffer) {
				/*
				 * allocate memory to store the DT overlay
				 * before it is applied. It may not be used
				 * depending on how the overlay is stored, so
				 * don't fail yet if the allocation failed.
				 */
				size_t size = CONFIG_SPL_LOAD_FIT_APPLY_OVERLAY_BUF_SZ;

				tmpbuffer = malloc_cache_aligned(size);
				ако (!tmpbuffer)
					debug("%s: unable to allocate space for overlays\n",
					      __func__);
			}
			image_info.load_addr = (ulong)tmpbuffer;
			ret = load_simple_fit(info, offset, ctx, node,
					      &image_info,
					      CONFIG_SPL_LOAD_FIT_APPLY_OVERLAY_BUF_SZ);
			ако (ret == -EBADSLT)
				continue;
			инако ако (ret < 0)
				break;

			/* Make room in FDT for changes from the overlay */
			ret = fdt_increase_size(spl_image->fdt_addr,
						image_info.size);
			ако (ret < 0)
				break;

			ret = fdt_overlay_apply_verbose(spl_image->fdt_addr,
							(void *)image_info.load_addr);
			ако (ret) {
				pr_err("failed to apply DT overlay %s\n",
				       fit_get_name(ctx->fit, node, NULL));
				break;
			}

			debug("%s: DT overlay %s applied\n", __func__,
			      fit_get_name(ctx->fit, node, NULL));
		}
		free(tmpbuffer);
		ако (ret)
			врати ret;
#endif
	/* Try to make space, so we can inject details on the loadables */
	ret = fdt_shrink_to_minimum(spl_image->fdt_addr, 8192);
	ако (ret < 0)
		врати ret;

	врати ret;
}

static цео spl_fit_record_loadable(const struct spl_fit_info *ctx, цео index,
				   void *blob, struct spl_image_info *image)
{
	цео ret = 0;
	const char *name;
	цео node;

	ret = spl_fit_get_image_name(ctx, FIT_LOADABLE_PROP, index, &name);
	ако (ret < 0)
		врати ret;

	node = spl_fit_get_image_node(ctx, FIT_LOADABLE_PROP, index);

	ret = fdt_record_loadable(blob, index, name, image->load_addr,
			image->size, image->entry_point,
			fdt_getprop(ctx->fit, node, FIT_TYPE_PROP, NULL),
			fdt_getprop(ctx->fit, node, FIT_OS_PROP, NULL),
			fdt_getprop(ctx->fit, node, FIT_ARCH_PROP, NULL));

	врати ret;
}

static цео spl_fit_image_is_fpga(const void *fit, цео node)
{
	const char *type;

	ако (!IS_ENABLED(CONFIG_SPL_FPGA))
		врати 0;

	type = fdt_getprop(fit, node, FIT_TYPE_PROP, NULL);
	ако (!type)
		врати 0;

	врати !strcmp(type, "fpga");
}

static void spl_fit_image_record_arm32_optee(const void *fit, цео node,
					     struct spl_image_info *spl_image,
					     struct spl_image_info *image_info)
{
#ако defined(CONFIG_BOOTM_OPTEE) && defined(CONFIG_ARM) && !defined(CONFIG_ARM64)
	const char *type = fdt_getprop(fit, node, FIT_TYPE_PROP, NULL);

	ако (!type)
		врати;

	ако (strcmp(type, "tee"))
		врати;

	spl_image->optee_addr = image_info->load_addr;
#endif
}

static цео spl_fit_image_get_os(const void *fit, цео noffset, uint8_t *os)
{
	ако (!CONFIG_IS_ENABLED(FIT_IMAGE_TINY) || CONFIG_IS_ENABLED(OS_BOOT))
		врати fit_image_get_os(fit, noffset, os);

	const char *name = fdt_getprop(fit, noffset, FIT_OS_PROP, NULL);
	ако (!name)
		врати -ENOENT;

	/*
	 * We don't care what the type of the image actually is,
	 * only whether or not it is U-Boot. This saves some
	 * space by omitting the large table of OS types.
	 */
	ако (!strcmp(name, "u-boot"))
		*os = IH_OS_U_BOOT;
	инако
		*os = IH_OS_INVALID;

	врати 0;
}

/*
 * The purpose of the FIT load buffer is to provide a memory location that is
 * independent of the load address of any FIT component.
 */
__weak void *board_spl_fit_buffer_addr(ulong fit_size, цео sectors, цео bl_len)
{
	void *buf;
	size_t size = sectors * bl_len;

	buf = malloc_cache_aligned(size);
	ако (!buf) {
		pr_err("Could not get FIT buffer of %lu bytes\n", (ulong)size);

		ако (IS_ENABLED(CONFIG_SPL_SYS_MALLOC))
			pr_err("\tcheck CONFIG_SPL_SYS_MALLOC_SIZE\n");
		инако
			pr_err("\tcheck CONFIG_SPL_SYS_MALLOC_F_LEN\n");

		buf = spl_get_load_buffer(0, size);
	}
	врати buf;
}

/*
 * Weak default function to allow customizing SPL fit loading for load-only
 * use cases by allowing to skip the parsing/processing of the FIT contents
 * (so that this can be done separately in a more customized fashion)
 */
__weak bool spl_load_simple_fit_skip_processing(void)
{
	врати false;
}

/*
 * Weak default function to allow fixes after fit header
 * is loaded.
 */
__weak void *spl_load_simple_fit_fix_load(const void *fit)
{
	врати (void *)fit;
}

static void warn_deprecated(const char *msg)
{
	printf("DEPRECATED: %s\n", msg);
	printf("\tSee https://fitspec.osfw.foundation/\n");
}

static цео spl_fit_upload_fpga(struct spl_fit_info *ctx, цео node,
			       struct spl_image_info *fpga_image)
{
	const char *compatible;
	цео ret;
	цео devnum = 0;
	цео flags = 0;

	debug("FPGA bitstream at: %x, size: %x\n",
	      (u32)fpga_image->load_addr, fpga_image->size);

	compatible = fdt_getprop(ctx->fit, node, FIT_COMPAT_PROP, NULL);
	ако (!compatible) {
		warn_deprecated("'fpga' image without 'compatible' property");
	} инако {
		ако (CONFIG_IS_ENABLED(FPGA_LOAD_SECURE))
			flags = fpga_compatible2flag(devnum, compatible);
		ако (strcmp(compatible, "u-boot,fpga-legacy"))
			debug("Ignoring compatible = %s property\n",
			      compatible);
	}

	ret = fpga_load(devnum, (void *)fpga_image->load_addr,
			fpga_image->size, BIT_FULL, flags);
	ако (ret) {
		printf("%s: Cannot load the image to the FPGA\n", __func__);
		врати ret;
	}

	puts("FPGA image loaded from FIT\n");

	врати 0;
}

static цео spl_fit_load_fpga(struct spl_fit_info *ctx,
			     struct spl_load_info *info, ulong offset)
{
	цео node, ret;

	struct spl_image_info fpga_image = {
		.load_addr = 0,
	};

	node = spl_fit_get_image_node(ctx, FIT_FPGA_PROP, 0);
	ако (node < 0)
		врати node;

	warn_deprecated("'fpga' property in config node. Use 'loadables'");

	/* Load the image and set up the fpga_image structure */
	ret = load_simple_fit(info, offset, ctx, node, &fpga_image,
			      CONFIG_SYS_BOOTM_LEN);
	ако (ret) {
		printf("%s: Cannot load the FPGA: %i\n", __func__, ret);
		врати ret;
	}

	врати spl_fit_upload_fpga(ctx, node, &fpga_image);
}

static цео spl_simple_fit_read(struct spl_fit_info *ctx,
			       struct spl_load_info *info, ulong offset,
			       const void *fit_header)
{
	unsigned long aligned_size, count, size;
	void *buf;
	цео ret;

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
	ако (ret) {
		log_debug("Invalid FIT size: %lu\n", size);
		врати ret;
	}
	buf = board_spl_fit_buffer_addr(aligned_size, aligned_size, 1);
	ако (!buf) {
		/*
		 * We assume that none of the board will ever use 0x0 as a
		 * valid load address. Theoretically some board could use it,
		 * but this is extremely unlikely.
		 */
		врати -EIO;
	}

	count = info->read(info, offset, aligned_size, buf);
	ако (!count) {
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
		врати -EIO;
	}

	ctx->fit = buf;
	debug("fit read offset %lx, size=%lu, dst=%p, count=%lu\n",
	      offset, aligned_size, buf, count);

	врати 0;
}

static цео spl_simple_fit_parse(struct spl_fit_info *ctx)
{
	/* Find the correct subnode under "/configurations" */
	ctx->conf_node = fit_find_config_node(ctx->fit);
	ако (ctx->conf_node < 0)
		врати -EINVAL;

	ако (CONFIG_IS_ENABLED(FIT_SIGNATURE)) {
		printf("## Checking hash(es) for config %s ... ",
		       fit_get_name(ctx->fit, ctx->conf_node, NULL));
		ако (fit_config_verify(ctx->fit, ctx->conf_node))
			врати -EPERM;
		puts("OK\n");
	}

	/* find the node holding the images information */
	ctx->images_node = fdt_path_offset(ctx->fit, FIT_IMAGES_PATH);
	ако (ctx->images_node < 0) {
		debug("%s: Cannot find /images node: %d\n", __func__,
		      ctx->images_node);
		врати -EINVAL;
	}

	врати 0;
}

цео spl_load_simple_fit(struct spl_image_info *spl_image,
			struct spl_load_info *info, ulong offset, void *fit)
{
	struct spl_image_info image_info;
	struct spl_fit_info ctx;
	цео node = -1;
	цео ret;
	цео index = 0;
	цео firmware_node;

	ret = spl_simple_fit_read(&ctx, info, offset, fit);
	ако (ret < 0)
		врати ret;

	/* skip further processing if requested to enable load-only use cases */
	ако (spl_load_simple_fit_skip_processing())
		врати 0;

	ctx.fit = spl_load_simple_fit_fix_load(ctx.fit);

	ret = spl_simple_fit_parse(&ctx);
	ако (ret < 0)
		врати ret;

	ако (IS_ENABLED(CONFIG_SPL_FPGA))
		spl_fit_load_fpga(&ctx, info, offset);

	/*
	 * Find the U-Boot image using the following search order:
	 *   - start at 'firmware' (e.g. an ARM Trusted Firmware)
	 *   - fall back 'kernel' (e.g. a Falcon-mode OS boot
	 *   - fall back to using the first 'loadables' entry
	 */
	ако (node < 0)
		node = spl_fit_get_image_node(&ctx, FIT_FIRMWARE_PROP, 0);

	ако (node < 0 && IS_ENABLED(CONFIG_SPL_OS_BOOT))
		node = spl_fit_get_image_node(&ctx, FIT_KERNEL_PROP, 0);

	ако (node < 0) {
		debug("could not find firmware image, trying loadables...\n");
		node = spl_fit_get_image_node(&ctx, FIT_LOADABLE_PROP, 0);
		/*
		 * If we pick the U-Boot image from "loadables", start at
		 * the second image when later loading additional images.
		 */
		index = 1;
	}
	ако (node < 0) {
		debug("%s: Cannot find u-boot image node: %d\n",
		      __func__, node);
		врати -1;
	}

	/* Load the image and set up the spl_image structure */
	ret = load_simple_fit(info, offset, &ctx, node, spl_image,
			      CONFIG_SYS_BOOTM_LEN);
	ако (ret)
		врати ret;

	/*
	 * For backward compatibility, we treat the first node that is
	 * as a U-Boot image, if no OS-type has been declared.
	 */
	ако (!spl_fit_image_get_os(ctx.fit, node, &spl_image->os))
		debug("Image OS is %s\n", genimg_get_os_name(spl_image->os));
	инако ако (!IS_ENABLED(CONFIG_SPL_OS_BOOT))
		spl_image->os = IH_OS_U_BOOT;

	/*
	 * Booting a next-stage U-Boot may require us to append the FDT.
	 * We allow this to fail, as the U-Boot image might embed its FDT.
	 */
	ако (os_takes_devicetree(spl_image->os)) {
		ret = spl_fit_append_fdt(spl_image, info, offset, &ctx);
		ако (ret < 0 && spl_image->os != IH_OS_U_BOOT)
			врати ret;
	}

	firmware_node = node;
	/* Now check if there are more images for us to load */
	за (; ; index++) {
		uint8_t os_type = IH_OS_INVALID;

		node = spl_fit_get_image_node(&ctx, FIT_LOADABLE_PROP, index);
		ако (node < 0)
			break;

		/*
		 * if the firmware is also a loadable, skip it because
		 * it already has been loaded. This is typically the case with
		 * u-boot.img generated by mkimage.
		 */
		ако (firmware_node == node)
			continue;

		image_info.load_addr = 0;
		ret = load_simple_fit(info, offset, &ctx, node, &image_info,
				      CONFIG_SYS_BOOTM_LEN);
		ако (ret < 0 && ret != -EBADSLT) {
			printf("%s: can't load image loadables index %d (ret = %d)\n",
			       __func__, index, ret);
			врати ret;
		}

		ако (spl_fit_image_is_fpga(ctx.fit, node))
			spl_fit_upload_fpga(&ctx, node, &image_info);

		ако (!spl_fit_image_get_os(ctx.fit, node, &os_type))
			debug("Loadable is %s\n", genimg_get_os_name(os_type));

		ако (os_takes_devicetree(os_type)) {
			spl_fit_append_fdt(&image_info, info, offset, &ctx);
			spl_image->fdt_addr = image_info.fdt_addr;
		}

		/*
		 * If the "firmware" image did not provide an entry point,
		 * use the first valid entry point from the loadables.
		 */
		ако (spl_image->entry_point == FDT_ERROR &&
		    image_info.entry_point != FDT_ERROR)
			spl_image->entry_point = image_info.entry_point;

		spl_fit_image_record_arm32_optee(ctx.fit, node, spl_image,
						 &image_info);

		/* Record our loadables into the FDT */
		ако (!CONFIG_IS_ENABLED(FIT_IMAGE_TINY) &&
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
	ако (spl_image->entry_point == FDT_ERROR || spl_image->entry_point == 0)
		spl_image->entry_point = spl_image->load_addr;

	spl_image->flags |= SPL_FIT_FOUND;
	upl_set_fit_info(map_to_sysmem(ctx.fit), ctx.conf_node,
			 spl_image->entry_point);

	врати 0;
}

/* Parse and load full fitImage in SPL */
цео spl_load_fit_image(struct spl_image_info *spl_image,
		       const struct legacy_img_hdr *header)
{
	struct bootm_headers images;
	const char *fit_uname_config = NULL;
	ulong fdt_hack;
	const char *uname;
	ulong fw_data = 0, dt_data = 0, img_data = 0;
	ulong fw_len = 0, dt_len = 0, img_len = 0;
	цео idx, conf_noffset;
	цео ret;

	images.verify = CONFIG_IS_ENABLED(FIT_SIGNATURE);

	ret = fit_image_load(&images, virt_to_phys((void *)header),
			     NULL, &fit_uname_config, IH_ARCH_DEFAULT,
			     IH_TYPE_FIRMWARE, -1, FIT_LOAD_OPTIONAL,
			     &fw_data, &fw_len);

	ако (ret < 0) {
		ret = fit_image_load(&images, virt_to_phys((void *)header),
				     NULL, &fit_uname_config, IH_ARCH_DEFAULT,
				     IH_TYPE_KERNEL, -1, FIT_LOAD_OPTIONAL,
				     &fw_data, &fw_len);
	}

	ако (ret < 0)
		врати ret;

	spl_image->size = fw_len;
	spl_image->load_addr = fw_data;
	ако (fit_image_get_entry(header, ret, &spl_image->entry_point))
		spl_image->entry_point = fw_data;
	ако (fit_image_get_os(header, ret, &spl_image->os))
		spl_image->os = IH_OS_INVALID;
	spl_image->name = genimg_get_os_name(spl_image->os);

	debug(PHASE_PROMPT "payload image: %32s load addr: 0x%lx size: %d\n",
	      spl_image->name, spl_image->load_addr, spl_image->size);

	images.verify = CONFIG_IS_ENABLED(FIT_SIGNATURE);

	ret = fit_image_load(&images, virt_to_phys((void *)header), NULL,
			     &fit_uname_config, IH_ARCH_DEFAULT, IH_TYPE_FLATDT,
			     -1, FIT_LOAD_OPTIONAL, &dt_data, &dt_len);
	ако (ret >= 0) {
		ако (spl_image->os == IH_OS_U_BOOT) {
			/* HACK: U-Boot expects FDT at a specific address */
			fdt_hack = ALIGN(spl_image->load_addr + spl_image->size, 8);
			debug("Relocating FDT to %p\n", (void *)fdt_hack);
			memcpy(map_sysmem(fdt_hack, dt_len),
			       map_sysmem(dt_data, 0), dt_len);
			spl_image->fdt_addr = (void *)fdt_hack;
		} инако {
			spl_image->fdt_addr = (void *)dt_data;
		}
	}

	conf_noffset = fit_conf_get_node((const void *)header,
					 fit_uname_config);
	ако (conf_noffset < 0)
		врати 0;

	за (idx = 0;
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
		ако (ret < 0)
			врати ret;
	}
	spl_image->flags |= SPL_FIT_FOUND;

	upl_set_fit_info(map_to_sysmem(header), conf_noffset,
			 spl_image->entry_point);

	врати 0;
}
