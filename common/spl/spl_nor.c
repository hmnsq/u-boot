// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012 Stefan Roese <sr@denx.de>
 */

#ضمن <config.h>
#ضمن <image.h>
#ضمن <imx_container.h>
#ضمن <log.h>
#ضمن <spl.h>
#ضمن <spl_load.h>

ساكن ulong spl_nor_load_read(بنية spl_load_info *load, ulong sector,
			       ulong count, فراغ *buf)
{
	debug("%s: sector %lx, count %lx, buf %p\n",
	      __func__, sector, count, buf);
	memcpy(buf, map_sysmem(sector, count), count);

	החזר count;
}

unsigned long __weak spl_nor_get_uboot_base(فراغ)
{
	החזר CFG_SYS_UBOOT_BASE;
}

#אם IS_ENABLED(CONFIG_SPL_OS_BOOT)
ساكن صحيح spl_nor_load_image_os(بنية spl_image_info *spl_image,
				 بنية spl_boot_device *bootdev)
{
	/*
	 * Load Linux from its location in NOR flash to its defined
	 * location in SDRAM
	 */
	ثابت بنية legacy_img_hdr *header =
		(ثابت بنية legacy_img_hdr *)CONFIG_SYS_OS_BASE;
	بنية spl_load_info load;

#ifdef CONFIG_SPL_LOAD_FIT
	אם (image_get_magic(header) == FDT_MAGIC) {
		صحيح ret;

		debug("Found FIT\n");
		spl_load_init(&load, spl_nor_load_read, عدم, 1);

		ret = spl_load_simple_fit(spl_image, &load, CONFIG_SYS_OS_BASE,
					  (فراغ *)header);

#אם defined CONFIG_SPL_PAYLOAD_ARGS_ADDR && defined CONFIG_CMD_SPL_NOR_OFS
		memcpy((فراغ *)CONFIG_SPL_PAYLOAD_ARGS_ADDR,
		       (فراغ *)CONFIG_CMD_SPL_NOR_OFS,
		       CONFIG_CMD_SPL_WRITE_SIZE);
#endif
		החזר ret;
	}
#endif
	אם (image_get_os(header) != IH_OS_LINUX)
		החזר -EINVAL;

	/* happy - was a Linux */
	صحيح ret;

	ret = spl_parse_image_header(spl_image, bootdev, header);
	אם (ret)
		החזר ret;

	memcpy((فراغ *)spl_image->load_addr,
	       (فراغ *)(CONFIG_SYS_OS_BASE + حجم(بنية legacy_img_hdr)),
	       spl_image->size);

	החזר 0;
}
#endif

ساكن صحيح spl_nor_load_image(بنية spl_image_info *spl_image,
			      بنية spl_boot_device *bootdev)
{
	بنية spl_load_info load;

	/*
	 * Loading of the payload to SDRAM is done with skipping of
	 * the mkimage header in this SPL NOR driver
	 */
	spl_image->flags |= SPL_COPY_PAYLOAD_ONLY;

#אם IS_ENABLED(CONFIG_SPL_OS_BOOT)
	صحيح err;

	אם (!spl_start_uboot()) {
		err = spl_nor_load_image_os(spl_image, bootdev);
		אם (!err)
			החזר 0;

		printf("%s: Failed in falcon boot: %d", __func__, err);
		אם (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			החזר err;
		printf("Fallback to U-Boot\n");
	}
#endif

	/*
	 * Load real U-Boot from its location in NOR flash to its
	 * defined location in SDRAM
	 */
	spl_load_init(&load, spl_nor_load_read, عدم, 1);
	החזר spl_load(spl_image, bootdev, &load, 0, spl_nor_get_uboot_base());
}
SPL_LOAD_IMAGE_METHOD("NOR", 0, BOOT_DEVICE_NOR, spl_nor_load_image);
