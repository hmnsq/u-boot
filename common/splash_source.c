// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014 CompuLab, Ltd. <www.compulab.co.il>
 *
 * Authors: Igor Grinberg <grinberg@compulab.co.il>
 */

#ضمن <bmp_layout.h>
#ضمن <command.h>
#ضمن <env.h>
#ضمن <errno.h>
#ضمن <fs.h>
#ضمن <fdt_support.h>
#ضمن <image.h>
#ضمن <log.h>
#ضمن <nand.h>
#ضمن <sata.h>
#ضمن <spi.h>
#ضمن <spi_flash.h>
#ضمن <splash.h>
#ضمن <usb.h>
#ضمن <virtio.h>
#ضمن <asm/global_data.h>

DECLARE_GLOBAL_داده_PTR;

#اگر_تعریف_شده CONFIG_SPI_FLASH
ساكن بنية spi_flash *sf;
ساكن صحيح splash_sf_read_raw(ulong bmp_بارگیری_addr, صحيح جابه_جایی, size_t read_اندازه)
{
	إذا (!sf) {
		sf = spi_flash_probe(CONFIG_SF_DEFAULT_BUS,
				     CONFIG_SF_DEFAULT_CS,
				     CONFIG_SF_DEFAULT_سرعت,
				     CONFIG_SF_DEFAULT_نما);
		إذا (!sf)
			أرجع -ENODEV;
	}

	أرجع spi_flash_read(sf, جابه_جایی, read_اندازه, (فراغ *)(uintptr_t)bmp_بارگیری_addr);
}
#وإلا
ساكن صحيح splash_sf_read_raw(ulong bmp_بارگیری_addr, صحيح جابه_جایی, size_t read_اندازه)
{
	debug("%s: sf support not available\n", __func__);
	أرجع -ENOSYS;
}
#پایان_اگر

#اگر_تعریف_شده CONFIG_CMD_NAND
ساكن صحيح splash_nand_read_raw(ulong bmp_بارگیری_addr, صحيح جابه_جایی, size_t read_اندازه)
{
	بنية mtd_info *mtd = get_nand_dev_by_شاخص(nand_curr_device);
	أرجع nand_read_skip_bad(mtd, جابه_جایی,
				  &read_اندازه, عدم,
				  mtd->اندازه,
				  (u_char *)bmp_بارگیری_addr);
}
#وإلا
ساكن صحيح splash_nand_read_raw(ulong bmp_بارگیری_addr, صحيح جابه_جایی, size_t read_اندازه)
{
	debug("%s: nand support not available\n", __func__);
	أرجع -ENOSYS;
}
#پایان_اگر

ساكن صحيح splash_mmc_read_raw(ulong bmp_بارگیری_addr, بنية splash_location *location,
			       size_t read_اندازه)
{
	بنية disk_افراز افراز;
	بنية blk_desc *desc;
	lbaint_t blkcnt;
	صحيح ret, n;

	إذا (!IS_ENABLED(CONFIG_CMD_MMC)) {
		debug("%s: mmc support not available\n", __func__);
		أرجع -ENOSYS;
	}

	ret = part_get_info_by_dev_and_name_or_رقم("mmc", location->devpart, &desc,
						   &افراز, 1);
	إذا (ret < 0)
		أرجع ret;

	blkcnt = DIV_دور_به_بالا(read_اندازه, افراز.blksz);
	n = blk_dread(desc, افراز.شروع, blkcnt, (فراغ *)(uintptr_t)bmp_بارگیری_addr);

	أرجع (n == blkcnt) ? 0 : -EIO;
}

ساكن صحيح splash_storage_read_raw(بنية splash_location *location,
			       ulong bmp_بارگیری_addr, size_t read_اندازه)
{
	u32 جابه_جایی;

	إذا (!location)
		أرجع -EINVAL;

	جابه_جایی = location->جابه_جایی;
	بدل (location->storage) {
	حالة SPLASH_STORAGE_MMC:
		أرجع splash_mmc_read_raw(bmp_بارگیری_addr, location, read_اندازه);
	حالة SPLASH_STORAGE_NAND:
		أرجع splash_nand_read_raw(bmp_بارگیری_addr, جابه_جایی, read_اندازه);
	حالة SPLASH_STORAGE_SF:
		أرجع splash_sf_read_raw(bmp_بارگیری_addr, جابه_جایی, read_اندازه);
	افتراضي:
		چاپ_کن("Unknown splash location\n");
	}

	أرجع -EINVAL;
}

ساكن صحيح splash_بارگیری_raw(بنية splash_location *location, ulong bmp_بارگیری_addr)
{
	بنية bmp_سرآیند *bmp_hdr;
	صحيح res;
	size_t bmp_اندازه, bmp_سرآیند_اندازه = حجم(بنية bmp_سرآیند);

	إذا (bmp_بارگیری_addr + bmp_سرآیند_اندازه >= gd->شروع_addr_sp)
		اذهب splash_آدرس_too_بالا;

	res = splash_storage_read_raw(location, bmp_بارگیری_addr, bmp_سرآیند_اندازه);
	إذا (res < 0)
		أرجع res;

	bmp_hdr = (بنية bmp_سرآیند *)(uintptr_t)bmp_بارگیری_addr;
	bmp_اندازه = le32_مقصد_cpu(bmp_hdr->file_اندازه);

	إذا (bmp_بارگیری_addr + bmp_اندازه >= gd->شروع_addr_sp)
		اذهب splash_آدرس_too_بالا;

	أرجع splash_storage_read_raw(location, bmp_بارگیری_addr, bmp_اندازه);

splash_آدرس_too_بالا:
	چاپ_کن("Error: splashimage address too high. Data overwrites U-Boot and/or placed beyond DRAM boundaries.\n");

	أرجع -EFAULT;
}

ساكن صحيح splash_select_fs_dev(بنية splash_location *location)
{
	صحيح res;

	بدل (location->storage) {
	حالة SPLASH_STORAGE_MMC:
		res = fs_مجموعه_blk_dev("mmc", location->devpart, FS_TYPE_ANY);
		اكسر;
	حالة SPLASH_STORAGE_USB:
		res = fs_مجموعه_blk_dev("usb", location->devpart, FS_TYPE_ANY);
		اكسر;
	حالة SPLASH_STORAGE_SATA:
		res = fs_مجموعه_blk_dev("sata", location->devpart, FS_TYPE_ANY);
		اكسر;
	حالة SPLASH_STORAGE_NAND:
		إذا (location->ubivol != عدم)
			res = fs_مجموعه_blk_dev("ubi", عدم, FS_TYPE_UBIFS);
		وإلا
			res = -ENODEV;
		اكسر;
	افتراضي:
		چاپ_کن("Error: %s: unsupported location storage.\n", __func__);
		أرجع -ENODEV;
	}

	إذا (res)
		چاپ_کن("Error: %s: could not access storage.\n", __func__);

	أرجع res;
}

#اگر_تعریف_شده CONFIG_USB_STORAGE
ساكن صحيح splash_init_usb(فراغ)
{
	صحيح err;

	err = usb_init();
	إذا (err)
		أرجع err;

#اگر_تعریف_نشده CONFIG_DM_USB
	err = usb_stor_پیمایش(1) < 0 ? -ENODEV : 0;
#پایان_اگر

	أرجع err;
}
#وإلا
ساكن مضمن صحيح splash_init_usb(فراغ)
{
	چاپ_کن("Cannot load splash image: no USB support\n");
	أرجع -ENOSYS;
}
#پایان_اگر

#اگر_تعریف_شده CONFIG_SATA
ساكن صحيح splash_init_sata(فراغ)
{
	أرجع sata_probe(0);
}
#وإلا
ساكن مضمن صحيح splash_init_sata(فراغ)
{
	چاپ_کن("Cannot load splash image: no SATA support\n");
	أرجع -ENOSYS;
}
#پایان_اگر

ساكن صحيح splash_init_virtio(فراغ)
{
	إذا (!IS_ENABLED(CONFIG_VIRTIO)) {
		چاپ_کن("Cannot load splash image: no virtio support\n");
		أرجع -ENOSYS;
	} وإلا {
		أرجع virtio_init();
	}
}

#إذا defined(CONFIG_CMD_UBIFS) && !defined(CONFIG_XPL_BUILD)
ساكن صحيح splash_mount_ubifs(بنية splash_location *location)
{
	صحيح res;
	حرف cmd[32];

	sprintf(cmd, "ubi part %s", location->mtdpart);
	res = پیاپی_command(cmd, 0);
	إذا (res)
		أرجع res;

	sprintf(cmd, "ubifsmount %s", location->ubivol);
	res = پیاپی_command(cmd, 0);

	أرجع res;
}

ساكن مضمن صحيح splash_umount_ubifs(فراغ)
{
	أرجع پیاپی_command("ubifsumount", 0);
}
#وإلا
ساكن مضمن صحيح splash_mount_ubifs(بنية splash_location *location)
{
	چاپ_کن("Cannot load splash image: no UBIFS support\n");
	أرجع -ENOSYS;
}

ساكن مضمن صحيح splash_umount_ubifs(فراغ)
{
	چاپ_کن("Cannot unmount UBIFS: no UBIFS support\n");
	أرجع -ENOSYS;
}
#پایان_اگر

#تعریف_کن SPLASH_منبع_DEFAULT_FILE_NAME		"splash.bmp"

ساكن صحيح splash_بارگیری_fs(بنية splash_location *location, ulong bmp_بارگیری_addr)
{
	صحيح res = 0;
	loff_t bmp_اندازه;
	loff_t actread;
	حرف *splash_file;

	splash_file = env_get("splashfile");
	إذا (!splash_file)
		splash_file = SPLASH_منبع_DEFAULT_FILE_NAME;

	إذا (location->storage == SPLASH_STORAGE_USB)
		res = splash_init_usb();

	إذا (location->storage == SPLASH_STORAGE_SATA)
		res = splash_init_sata();

	إذا (location->storage == SPLASH_STORAGE_VIRTIO)
		res = splash_init_virtio();

	إذا (location->ubivol != عدم)
		res = splash_mount_ubifs(location);

	إذا (res)
		أرجع res;

	res = splash_select_fs_dev(location);
	إذا (res)
		اذهب بیرون;

	res = fs_اندازه(splash_file, &bmp_اندازه);
	إذا (res) {
		چاپ_کن("Error: %s: cannot determine file size (%d)\n",
		       __func__, res);
		اذهب بیرون;
	}

	إذا (bmp_بارگیری_addr + bmp_اندازه >= gd->شروع_addr_sp) {
		چاپ_کن("Error: splashimage address too high. Data overwrites U-Boot and/or placed beyond DRAM boundaries.\n");
		res = -EFAULT;
		اذهب بیرون;
	}

	splash_select_fs_dev(location);
	res = fs_read(splash_file, bmp_بارگیری_addr, 0, 0, &actread);

بیرون:
	إذا (location->ubivol != عدم)
		splash_umount_ubifs();

	أرجع res;
}

/**
 * select_splash_location - return the splash location based on board support
 *			    and env variable "splashsource".
 *
 * @locations:		An array of supported splash locations.
 * @size:		Size of splash_locations array.
 *
 * @return: If a null set of splash locations is given, or
 *	    splashsource env variable is set to unsupported value
 *			return NULL.
 *	    If splashsource env variable is not defined
 *			return the first entry in splash_locations as default.
 *	    If splashsource env variable contains a supported value
 *			return the location selected by splashsource.
 */
ساكن بنية splash_location *select_splash_location(
			    بنية splash_location *locations, uint اندازه)
{
	صحيح i;
	حرف *env_splashsource;

	إذا (!locations || اندازه == 0)
		أرجع عدم;

	env_splashsource = env_get("splashsource");
	إذا (env_splashsource == عدم)
		أرجع &locations[0];

	لكل (i = 0; i < اندازه; i++) {
		إذا (!strcmp(locations[i].name, env_splashsource))
			أرجع &locations[i];
	}

	چاپ_کن("splashsource env variable set to unsupported value\n");
	أرجع عدم;
}

#اگر_تعریف_شده CONFIG_FIT
ساكن صحيح splash_بارگیری_fit(بنية splash_location *location, ulong bmp_بارگیری_addr)
{
	صحيح res;
	صحيح گره_جابه_جایی;
	ثابت حرف *splash_file;
	ثابت فراغ *داخلی_splash_داده;
	size_t داخلی_splash_اندازه;
	صحيح external_splash_addr;
	u32 external_splash_اندازه;
	bool is_splash_external = false;
	بنية legacy_img_hdr *img_سرآیند;
	ثابت u32 *fit_سرآیند;
	u32 fit_اندازه;
	ثابت size_t سرآیند_اندازه = حجم(بنية legacy_img_hdr);

	/* Read in image header */
	res = splash_storage_read_raw(location, bmp_بارگیری_addr, سرآیند_اندازه);
	إذا (res < 0)
		أرجع res;

	img_سرآیند = (بنية legacy_img_hdr *)(uintptr_t)bmp_بارگیری_addr;
	إذا (image_get_magic(img_سرآیند) != FDT_MAGIC) {
		چاپ_کن("Could not find FDT magic\n");
		أرجع -EINVAL;
	}

	fit_اندازه = fdt_totalsize(img_سرآیند);

	/* Read in entire FIT */
	fit_سرآیند = (ثابت u32 *)(bmp_بارگیری_addr + سرآیند_اندازه);
	res = splash_storage_read_raw(location, (uintptr_t)fit_سرآیند, fit_اندازه);
	إذا (res < 0)
		أرجع res;

	res = fit_check_format(fit_سرآیند, IMAGE_اندازه_INVAL);
	إذا (res) {
		debug("Could not find valid FIT image\n");
		أرجع res;
	}

	/* Get the splash image node */
	splash_file = env_get("splashfile");
	إذا (!splash_file)
		splash_file = SPLASH_منبع_DEFAULT_FILE_NAME;

	گره_جابه_جایی = fit_image_get_گره(fit_سرآیند, splash_file);
	إذا (گره_جابه_جایی < 0) {
		debug("Could not find splash image '%s' in FIT\n",
		      splash_file);
		أرجع -ENOENT;
	}

	/* Extract the splash data from FIT */
	إذا (!fit_image_get_داده(fit_سرآیند, گره_جابه_جایی, &داخلی_splash_داده,
				&داخلی_splash_اندازه)) {
		memmove((فراغ *)(uintptr_t)bmp_بارگیری_addr, داخلی_splash_داده,
			داخلی_splash_اندازه);
	} وإلا {
		چاپ_کن("Failed to get splash image from FIT\n");
		أرجع -ENODATA;
	}

	إذا (is_splash_external) {
		res = fit_image_get_داده_اندازه(fit_سرآیند, گره_جابه_جایی, &external_splash_اندازه);
		إذا (res < 0) {
			چاپ_کن("Failed to get size of splash image (err=%d)\n", res);
			أرجع res;
		}

		/* Read in the splash data */
		location->جابه_جایی = external_splash_addr;
		res = splash_storage_read_raw(location, bmp_بارگیری_addr, external_splash_اندازه);
		إذا (res < 0)
			أرجع res;
	}

	أرجع 0;
}
#پایان_اگر /* CONFIG_FIT */

/**
 * splash_source_load - load splash image from a supported location.
 *
 * Select a splash image location based on the value of splashsource environment
 * variable and the board supported splash source locations, and load a
 * splashimage to the address pointed to by splashimage environment variable.
 *
 * @locations:		An array of supported splash locations.
 * @size:		Size of splash_locations array.
 *
 * @return: 0 on success, negative value on failure.
 */
صحيح splash_منبع_بارگیری(بنية splash_location *locations, uint اندازه)
{
	بنية splash_location *splash_location;
	حرف *env_splashimage_مقدار;
	حرف *devpart;
	ulong bmp_بارگیری_addr;

	env_splashimage_مقدار = env_get("splashimage");
	إذا (env_splashimage_مقدار == عدم)
		أرجع -ENOENT;

	bmp_بارگیری_addr = hextoul(env_splashimage_مقدار, 0);
	إذا (bmp_بارگیری_addr == 0) {
		چاپ_کن("Error: bad splashimage address specified\n");
		أرجع -EFAULT;
	}

	splash_location = select_splash_location(locations, اندازه);
	إذا (!splash_location)
		أرجع -EINVAL;

	devpart = env_get("splashdevpart");
	إذا (devpart)
		splash_location->devpart = devpart;

	إذا (splash_location->flags == SPLASH_STORAGE_RAW)
		أرجع splash_بارگیری_raw(splash_location, bmp_بارگیری_addr);
	وإلا إذا (splash_location->flags == SPLASH_STORAGE_FS)
		أرجع splash_بارگیری_fs(splash_location, bmp_بارگیری_addr);
#اگر_تعریف_شده CONFIG_FIT
	وإلا إذا (splash_location->flags == SPLASH_STORAGE_FIT)
		أرجع splash_بارگیری_fit(splash_location, bmp_بارگیری_addr);
#پایان_اگر
	أرجع -EINVAL;
}
