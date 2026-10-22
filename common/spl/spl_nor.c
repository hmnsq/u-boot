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

static ulong spl_nor_load_read(struct spl_load_info *load, ulong sector,
			       ulong count, void *buf)
{
	debug("%s: sector %lx, count %lx, buf %p\n",
	      __func__, sector, count, buf);
	memcpy(buf, map_sysmem(sector, count), count);

	врати count;
}

unsigned long __weak spl_nor_get_uboot_base(void)
{
	врати CFG_SYS_UBOOT_BASE;
}

#ако IS_ENABLED(CONFIG_SPL_OS_BOOT)
static цео spl_nor_load_image_os(struct spl_image_info *spl_image,
				 struct spl_boot_device *bootdev)
{
	/*
	 * Load Linux from its location in NOR flash to its defined
	 * location in SDRAM
	 */
	const struct legacy_img_hdr *header =
		(const struct legacy_img_hdr *)CONFIG_SYS_OS_BASE;
	struct spl_load_info load;

#ifdef CONFIG_SPL_LOAD_FIT
	ако (image_get_magic(header) == FDT_MAGIC) {
		цео ret;

		debug("Found FIT\n");
		spl_load_init(&load, spl_nor_load_read, NULL, 1);

		ret = spl_load_simple_fit(spl_image, &load, CONFIG_SYS_OS_BASE,
					  (void *)header);

#ако defined CONFIG_SPL_PAYLOAD_ARGS_ADDR && defined CONFIG_CMD_SPL_NOR_OFS
		memcpy((void *)CONFIG_SPL_PAYLOAD_ARGS_ADDR,
		       (void *)CONFIG_CMD_SPL_NOR_OFS,
		       CONFIG_CMD_SPL_WRITE_SIZE);
#endif
		врати ret;
	}
#endif
	ако (image_get_os(header) != IH_OS_LINUX)
		врати -EINVAL;

	/* happy - was a Linux */
	цео ret;

	ret = spl_parse_image_header(spl_image, bootdev, header);
	ако (ret)
		врати ret;

	memcpy((void *)spl_image->load_addr,
	       (void *)(CONFIG_SYS_OS_BASE + sizeof(struct legacy_img_hdr)),
	       spl_image->size);

	врати 0;
}
#endif

static цео spl_nor_load_image(struct spl_image_info *spl_image,
			      struct spl_boot_device *bootdev)
{
	struct spl_load_info load;

	/*
	 * Loading of the payload to SDRAM is done with skipping of
	 * the mkimage header in this SPL NOR driver
	 */
	spl_image->flags |= SPL_COPY_PAYLOAD_ONLY;

#ако IS_ENABLED(CONFIG_SPL_OS_BOOT)
	цео err;

	ако (!spl_start_uboot()) {
		err = spl_nor_load_image_os(spl_image, bootdev);
		ако (!err)
			врати 0;

		printf("%s: Failed in falcon boot: %d", __func__, err);
		ако (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			врати err;
		printf("Fallback to U-Boot\n");
	}
#endif

	/*
	 * Load real U-Boot from its location in NOR flash to its
	 * defined location in SDRAM
	 */
	spl_load_init(&load, spl_nor_load_read, NULL, 1);
	врати spl_load(spl_image, bootdev, &load, 0, spl_nor_get_uboot_base());
}
SPL_LOAD_IMAGE_METHOD("NOR", 0, BOOT_DEVICE_NOR, spl_nor_load_image);
