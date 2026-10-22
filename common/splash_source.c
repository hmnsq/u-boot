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

DECLARE_GLOBAL_معلومات_PTR;

#که_تعریف_شوی CONFIG_SPI_FLASH
ساكن بنية spi_flash *sf;
ساكن صحيح splash_sf_read_raw(ulong bmp_بارول_addr, صحيح دننه_واټن, size_t read_اندازه)
{
	إذا (!sf) {
		sf = spi_flash_probe(CONFIG_SF_DEFAULT_BUS,
				     CONFIG_SF_DEFAULT_CS,
				     CONFIG_SF_DEFAULT_چټکتیا,
				     CONFIG_SF_DEFAULT_ډېرپېښ);
		إذا (!sf)
			أرجع -ENODEV;
	}

	أرجع spi_flash_read(sf, دننه_واټن, read_اندازه, (فراغ *)(uintptr_t)bmp_بارول_addr);
}
#وإلا
ساكن صحيح splash_sf_read_raw(ulong bmp_بارول_addr, صحيح دننه_واټن, size_t read_اندازه)
{
	debug("%s: sf support not available\n", __func__);
	أرجع -ENOSYS;
}
#د_که_پای

#که_تعریف_شوی CONFIG_CMD_NAND
ساكن صحيح splash_nand_read_raw(ulong bmp_بارول_addr, صحيح دننه_واټن, size_t read_اندازه)
{
	بنية mtd_info *mtd = get_nand_dev_by_شاخص(nand_curr_device);
	أرجع nand_read_skip_bad(mtd, دننه_واټن,
				  &read_اندازه, عدم,
				  mtd->اندازه,
				  (u_char *)bmp_بارول_addr);
}
#وإلا
ساكن صحيح splash_nand_read_raw(ulong bmp_بارول_addr, صحيح دننه_واټن, size_t read_اندازه)
{
	debug("%s: nand support not available\n", __func__);
	أرجع -ENOSYS;
}
#د_که_پای

ساكن صحيح splash_mmc_read_raw(ulong bmp_بارول_addr, بنية splash_location *location,
			       size_t read_اندازه)
{
	بنية disk_برخه_کول برخه_کول;
	بنية blk_desc *desc;
	lbaint_t blkcnt;
	صحيح ret, n;

	إذا (!IS_ENABLED(CONFIG_CMD_MMC)) {
		debug("%s: mmc support not available\n", __func__);
		أرجع -ENOSYS;
	}

	ret = part_get_info_by_dev_and_name_or_عدد("mmc", location->devpart, &desc,
						   &برخه_کول, 1);
	إذا (ret < 0)
		أرجع ret;

	blkcnt = DIV_پړاو_پورته(read_اندازه, برخه_کول.blksz);
	n = blk_dread(desc, برخه_کول.پیل, blkcnt, (فراغ *)(uintptr_t)bmp_بارول_addr);

	أرجع (n == blkcnt) ? 0 : -EIO;
}

ساكن صحيح splash_storage_read_raw(بنية splash_location *location,
			       ulong bmp_بارول_addr, size_t read_اندازه)
{
	u32 دننه_واټن;

	إذا (!location)
		أرجع -EINVAL;

	دننه_واټن = location->دننه_واټن;
	بدل (location->storage) {
	حالة SPLASH_STORAGE_MMC:
		أرجع splash_mmc_read_raw(bmp_بارول_addr, location, read_اندازه);
	حالة SPLASH_STORAGE_NAND:
		أرجع splash_nand_read_raw(bmp_بارول_addr, دننه_واټن, read_اندازه);
	حالة SPLASH_STORAGE_SF:
		أرجع splash_sf_read_raw(bmp_بارول_addr, دننه_واټن, read_اندازه);
	افتراضي:
		چاپ_کړه("Unknown splash location\n");
	}

	أرجع -EINVAL;
}

ساكن صحيح splash_بارول_raw(بنية splash_location *location, ulong bmp_بارول_addr)
{
	بنية bmp_سرلیک *bmp_hdr;
	صحيح res;
	size_t bmp_اندازه, bmp_سرلیک_اندازه = حجم(بنية bmp_سرلیک);

	إذا (bmp_بارول_addr + bmp_سرلیک_اندازه >= gd->پیل_addr_sp)
		اذهب splash_پته_too_لوړ;

	res = splash_storage_read_raw(location, bmp_بارول_addr, bmp_سرلیک_اندازه);
	إذا (res < 0)
		أرجع res;

	bmp_hdr = (بنية bmp_سرلیک *)(uintptr_t)bmp_بارول_addr;
	bmp_اندازه = le32_منزل_cpu(bmp_hdr->file_اندازه);

	إذا (bmp_بارول_addr + bmp_اندازه >= gd->پیل_addr_sp)
		اذهب splash_پته_too_لوړ;

	أرجع splash_storage_read_raw(location, bmp_بارول_addr, bmp_اندازه);

splash_پته_too_لوړ:
	چاپ_کړه("Error: splashimage address too high. Data overwrites U-Boot and/or placed beyond DRAM boundaries.\n");

	أرجع -EFAULT;
}

ساكن صحيح splash_select_fs_dev(بنية splash_location *location)
{
	صحيح res;

	بدل (location->storage) {
	حالة SPLASH_STORAGE_MMC:
		res = fs_سېټ_blk_dev("mmc", location->devpart, FS_TYPE_ANY);
		اكسر;
	حالة SPLASH_STORAGE_USB:
		res = fs_سېټ_blk_dev("usb", location->devpart, FS_TYPE_ANY);
		اكسر;
	حالة SPLASH_STORAGE_SATA:
		res = fs_سېټ_blk_dev("sata", location->devpart, FS_TYPE_ANY);
		اكسر;
	حالة SPLASH_STORAGE_NAND:
		إذا (location->ubivol != عدم)
			res = fs_سېټ_blk_dev("ubi", عدم, FS_TYPE_UBIFS);
		وإلا
			res = -ENODEV;
		اكسر;
	افتراضي:
		چاپ_کړه("Error: %s: unsupported location storage.\n", __func__);
		أرجع -ENODEV;
	}

	إذا (res)
		چاپ_کړه("Error: %s: could not access storage.\n", __func__);

	أرجع res;
}

#که_تعریف_شوی CONFIG_USB_STORAGE
ساكن صحيح splash_init_usb(فراغ)
{
	صحيح err;

	err = usb_init();
	إذا (err)
		أرجع err;

#که_نه_تعریف_شوی CONFIG_DM_USB
	err = usb_stor_لټون(1) < 0 ? -ENODEV : 0;
#د_که_پای

	أرجع err;
}
#وإلا
ساكن مضمن صحيح splash_init_usb(فراغ)
{
	چاپ_کړه("Cannot load splash image: no USB support\n");
	أرجع -ENOSYS;
}
#د_که_پای

#که_تعریف_شوی CONFIG_SATA
ساكن صحيح splash_init_sata(فراغ)
{
	أرجع sata_probe(0);
}
#وإلا
ساكن مضمن صحيح splash_init_sata(فراغ)
{
	چاپ_کړه("Cannot load splash image: no SATA support\n");
	أرجع -ENOSYS;
}
#د_که_پای

ساكن صحيح splash_init_virtio(فراغ)
{
	إذا (!IS_ENABLED(CONFIG_VIRTIO)) {
		چاپ_کړه("Cannot load splash image: no virtio support\n");
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
	res = پرلپسې_command(cmd, 0);
	إذا (res)
		أرجع res;

	sprintf(cmd, "ubifsmount %s", location->ubivol);
	res = پرلپسې_command(cmd, 0);

	أرجع res;
}

ساكن مضمن صحيح splash_umount_ubifs(فراغ)
{
	أرجع پرلپسې_command("ubifsumount", 0);
}
#وإلا
ساكن مضمن صحيح splash_mount_ubifs(بنية splash_location *location)
{
	چاپ_کړه("Cannot load splash image: no UBIFS support\n");
	أرجع -ENOSYS;
}

ساكن مضمن صحيح splash_umount_ubifs(فراغ)
{
	چاپ_کړه("Cannot unmount UBIFS: no UBIFS support\n");
	أرجع -ENOSYS;
}
#د_که_پای

#تعریف_کړه SPLASH_سرچینه_DEFAULT_FILE_NAME		"splash.bmp"

ساكن صحيح splash_بارول_fs(بنية splash_location *location, ulong bmp_بارول_addr)
{
	صحيح res = 0;
	loff_t bmp_اندازه;
	loff_t actread;
	حرف *splash_file;

	splash_file = env_get("splashfile");
	إذا (!splash_file)
		splash_file = SPLASH_سرچینه_DEFAULT_FILE_NAME;

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
		اذهب بهر;

	res = fs_اندازه(splash_file, &bmp_اندازه);
	إذا (res) {
		چاپ_کړه("Error: %s: cannot determine file size (%d)\n",
		       __func__, res);
		اذهب بهر;
	}

	إذا (bmp_بارول_addr + bmp_اندازه >= gd->پیل_addr_sp) {
		چاپ_کړه("Error: splashimage address too high. Data overwrites U-Boot and/or placed beyond DRAM boundaries.\n");
		res = -EFAULT;
		اذهب بهر;
	}

	splash_select_fs_dev(location);
	res = fs_read(splash_file, bmp_بارول_addr, 0, 0, &actread);

بهر:
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

	چاپ_کړه("splashsource env variable set to unsupported value\n");
	أرجع عدم;
}

#که_تعریف_شوی CONFIG_FIT
ساكن صحيح splash_بارول_fit(بنية splash_location *location, ulong bmp_بارول_addr)
{
	صحيح res;
	صحيح غوټه_دننه_واټن;
	ثابت حرف *splash_file;
	ثابت فراغ *دننی_splash_معلومات;
	size_t دننی_splash_اندازه;
	صحيح external_splash_addr;
	u32 external_splash_اندازه;
	bool is_splash_external = false;
	بنية legacy_img_hdr *img_سرلیک;
	ثابت u32 *fit_سرلیک;
	u32 fit_اندازه;
	ثابت size_t سرلیک_اندازه = حجم(بنية legacy_img_hdr);

	/* Read in image header */
	res = splash_storage_read_raw(location, bmp_بارول_addr, سرلیک_اندازه);
	إذا (res < 0)
		أرجع res;

	img_سرلیک = (بنية legacy_img_hdr *)(uintptr_t)bmp_بارول_addr;
	إذا (image_get_magic(img_سرلیک) != FDT_MAGIC) {
		چاپ_کړه("Could not find FDT magic\n");
		أرجع -EINVAL;
	}

	fit_اندازه = fdt_totalsize(img_سرلیک);

	/* Read in entire FIT */
	fit_سرلیک = (ثابت u32 *)(bmp_بارول_addr + سرلیک_اندازه);
	res = splash_storage_read_raw(location, (uintptr_t)fit_سرلیک, fit_اندازه);
	إذا (res < 0)
		أرجع res;

	res = fit_check_format(fit_سرلیک, IMAGE_اندازه_INVAL);
	إذا (res) {
		debug("Could not find valid FIT image\n");
		أرجع res;
	}

	/* Get the splash image node */
	splash_file = env_get("splashfile");
	إذا (!splash_file)
		splash_file = SPLASH_سرچینه_DEFAULT_FILE_NAME;

	غوټه_دننه_واټن = fit_image_get_غوټه(fit_سرلیک, splash_file);
	إذا (غوټه_دننه_واټن < 0) {
		debug("Could not find splash image '%s' in FIT\n",
		      splash_file);
		أرجع -ENOENT;
	}

	/* Extract the splash data from FIT */
	إذا (!fit_image_get_معلومات(fit_سرلیک, غوټه_دننه_واټن, &دننی_splash_معلومات,
				&دننی_splash_اندازه)) {
		memmove((فراغ *)(uintptr_t)bmp_بارول_addr, دننی_splash_معلومات,
			دننی_splash_اندازه);
	} وإلا {
		چاپ_کړه("Failed to get splash image from FIT\n");
		أرجع -ENODATA;
	}

	إذا (is_splash_external) {
		res = fit_image_get_معلومات_اندازه(fit_سرلیک, غوټه_دننه_واټن, &external_splash_اندازه);
		إذا (res < 0) {
			چاپ_کړه("Failed to get size of splash image (err=%d)\n", res);
			أرجع res;
		}

		/* Read in the splash data */
		location->دننه_واټن = external_splash_addr;
		res = splash_storage_read_raw(location, bmp_بارول_addr, external_splash_اندازه);
		إذا (res < 0)
			أرجع res;
	}

	أرجع 0;
}
#د_که_پای /* CONFIG_FIT */

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
صحيح splash_سرچینه_بارول(بنية splash_location *locations, uint اندازه)
{
	بنية splash_location *splash_location;
	حرف *env_splashimage_ارزښت;
	حرف *devpart;
	ulong bmp_بارول_addr;

	env_splashimage_ارزښت = env_get("splashimage");
	إذا (env_splashimage_ارزښت == عدم)
		أرجع -ENOENT;

	bmp_بارول_addr = hextoul(env_splashimage_ارزښت, 0);
	إذا (bmp_بارول_addr == 0) {
		چاپ_کړه("Error: bad splashimage address specified\n");
		أرجع -EFAULT;
	}

	splash_location = select_splash_location(locations, اندازه);
	إذا (!splash_location)
		أرجع -EINVAL;

	devpart = env_get("splashdevpart");
	إذا (devpart)
		splash_location->devpart = devpart;

	إذا (splash_location->flags == SPLASH_STORAGE_RAW)
		أرجع splash_بارول_raw(splash_location, bmp_بارول_addr);
	وإلا إذا (splash_location->flags == SPLASH_STORAGE_FS)
		أرجع splash_بارول_fs(splash_location, bmp_بارول_addr);
#که_تعریف_شوی CONFIG_FIT
	وإلا إذا (splash_location->flags == SPLASH_STORAGE_FIT)
		أرجع splash_بارول_fit(splash_location, bmp_بارول_addr);
#د_که_پای
	أرجع -EINVAL;
}
