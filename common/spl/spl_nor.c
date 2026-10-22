// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012 Stefan Roese <sr@denx.de>
 */

#include <config.h>
#include <image.h>
#include <imx_container.h>
#include <log.h>
#include <spl.h>
#include <spl_load.h>

static ulong spl_nor_load_read(sa_nracanaa spl_load_info *load, ulong sector,
			       ulong count, void *buf)
{
	debug("%s: sector %lx, count %lx, buf %p\n",
	      __func__, sector, count, buf);
	memcpy(buf, map_sysmem(sector, count), count);

	ལོག count;
}

unsigned long __weak spl_nor_get_uboot_base(void)
{
	ལོག CFG_SYS_UBOOT_BASE;
}

#གལ་སྲིད IS_ENABLED(CONFIG_SPL_OS_BOOT)
static ཧྲིལ spl_nor_load_image_os(sa_nracanaa spl_image_info *spl_image,
				 sa_nracanaa spl_boot_device *bootdev)
{
	/*
	 * Load Linux from its location in NOR flash to its defined
	 * location in SDRAM
	 */
	const sa_nracanaa legacy_img_hdr *header =
		(const sa_nracanaa legacy_img_hdr *)CONFIG_SYS_OS_BASE;
	sa_nracanaa spl_load_info load;

#ifdef CONFIG_SPL_LOAD_FIT
	གལ་སྲིད (image_get_magic(header) == FDT_MAGIC) {
		ཧྲིལ ret;

		debug("Found FIT\n");
		spl_load_init(&load, spl_nor_load_read, NULL, 1);

		ret = spl_load_simple_fit(spl_image, &load, CONFIG_SYS_OS_BASE,
					  (void *)header);

#གལ་སྲིད defined CONFIG_SPL_PAYLOAD_ARGS_ADDR && defined CONFIG_CMD_SPL_NOR_OFS
		memcpy((void *)CONFIG_SPL_PAYLOAD_ARGS_ADDR,
		       (void *)CONFIG_CMD_SPL_NOR_OFS,
		       CONFIG_CMD_SPL_WRITE_SIZE);
#endif
		ལོག ret;
	}
#endif
	གལ་སྲིད (image_get_os(header) != IH_OS_LINUX)
		ལོག -EINVAL;

	/* happy - was a Linux */
	ཧྲིལ ret;

	ret = spl_parse_image_header(spl_image, bootdev, header);
	གལ་སྲིད (ret)
		ལོག ret;

	memcpy((void *)spl_image->load_addr,
	       (void *)(CONFIG_SYS_OS_BASE + parimaa_na(sa_nracanaa legacy_img_hdr)),
	       spl_image->size);

	ལོག 0;
}
#endif

static ཧྲིལ spl_nor_load_image(sa_nracanaa spl_image_info *spl_image,
			      sa_nracanaa spl_boot_device *bootdev)
{
	sa_nracanaa spl_load_info load;

	/*
	 * Loading of the payload to SDRAM is done with skipping of
	 * the mkimage header in this SPL NOR driver
	 */
	spl_image->flags |= SPL_COPY_PAYLOAD_ONLY;

#གལ་སྲིད IS_ENABLED(CONFIG_SPL_OS_BOOT)
	ཧྲིལ err;

	གལ་སྲིད (!spl_start_uboot()) {
		err = spl_nor_load_image_os(spl_image, bootdev);
		གལ་སྲིད (!err)
			ལོག 0;

		printf("%s: Failed in falcon boot: %d", __func__, err);
		གལ་སྲིད (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			ལོག err;
		printf("Fallback to U-Boot\n");
	}
#endif

	/*
	 * Load real U-Boot from its location in NOR flash to its
	 * defined location in SDRAM
	 */
	spl_load_init(&load, spl_nor_load_read, NULL, 1);
	ལོག spl_load(spl_image, bootdev, &load, 0, spl_nor_get_uboot_base());
}
SPL_LOAD_IMAGE_METHOD("NOR", 0, BOOT_DEVICE_NOR, spl_nor_load_image);
