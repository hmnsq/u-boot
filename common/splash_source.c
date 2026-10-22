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

#جيڪڏهن_وصف_ٿيل CONFIG_SPI_FLASH
ساكن بنية spi_flash *sf;
ساكن صحيح splash_sf_read_raw(ulong bmp_ڀريو_addr, صحيح وٿي, size_t read_سائيز)
{
	إذا (!sf) {
		sf = spi_flash_probe(CONFIG_SF_DEFAULT_BUS,
				     CONFIG_SF_DEFAULT_CS,
				     CONFIG_SF_DEFAULT_رفتار,
				     CONFIG_SF_DEFAULT_گهڻائي);
		إذا (!sf)
			أرجع -ENODEV;
	}

	أرجع spi_flash_read(sf, وٿي, read_سائيز, (فراغ *)(uintptr_t)bmp_ڀريو_addr);
}
#وإلا
ساكن صحيح splash_sf_read_raw(ulong bmp_ڀريو_addr, صحيح وٿي, size_t read_سائيز)
{
	debug("%s: sf support not available\n", __func__);
	أرجع -ENOSYS;
}
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل CONFIG_CMD_NAND
ساكن صحيح splash_nand_read_raw(ulong bmp_ڀريو_addr, صحيح وٿي, size_t read_سائيز)
{
	بنية mtd_info *mtd = get_nand_dev_by_اشاريو(nand_curr_device);
	أرجع nand_read_skip_bad(mtd, وٿي,
				  &read_سائيز, عدم,
				  mtd->سائيز,
				  (u_char *)bmp_ڀريو_addr);
}
#وإلا
ساكن صحيح splash_nand_read_raw(ulong bmp_ڀريو_addr, صحيح وٿي, size_t read_سائيز)
{
	debug("%s: nand support not available\n", __func__);
	أرجع -ENOSYS;
}
#جيڪڏهن_پورو

ساكن صحيح splash_mmc_read_raw(ulong bmp_ڀريو_addr, بنية splash_location *location,
			       size_t read_سائيز)
{
	بنية disk_حصا_ڪريو حصا_ڪريو;
	بنية blk_desc *desc;
	lbaint_t blkcnt;
	صحيح ret, n;

	إذا (!IS_ENABLED(CONFIG_CMD_MMC)) {
		debug("%s: mmc support not available\n", __func__);
		أرجع -ENOSYS;
	}

	ret = part_get_info_by_dev_and_name_or_انگ("mmc", location->devpart, &desc,
						   &حصا_ڪريو, 1);
	إذا (ret < 0)
		أرجع ret;

	blkcnt = DIV_ڦيرو_مٿي(read_سائيز, حصا_ڪريو.blksz);
	n = blk_dread(desc, حصا_ڪريو.شروعات, blkcnt, (فراغ *)(uintptr_t)bmp_ڀريو_addr);

	أرجع (n == blkcnt) ? 0 : -EIO;
}

ساكن صحيح splash_storage_read_raw(بنية splash_location *location,
			       ulong bmp_ڀريو_addr, size_t read_سائيز)
{
	u32 وٿي;

	إذا (!location)
		أرجع -EINVAL;

	وٿي = location->وٿي;
	بدل (location->storage) {
	حالة SPLASH_STORAGE_MMC:
		أرجع splash_mmc_read_raw(bmp_ڀريو_addr, location, read_سائيز);
	حالة SPLASH_STORAGE_NAND:
		أرجع splash_nand_read_raw(bmp_ڀريو_addr, وٿي, read_سائيز);
	حالة SPLASH_STORAGE_SF:
		أرجع splash_sf_read_raw(bmp_ڀريو_addr, وٿي, read_سائيز);
	افتراضي:
		لکو("Unknown splash location\n");
	}

	أرجع -EINVAL;
}

ساكن صحيح splash_ڀريو_raw(بنية splash_location *location, ulong bmp_ڀريو_addr)
{
	بنية bmp_سرو *bmp_hdr;
	صحيح res;
	size_t bmp_سائيز, bmp_سرو_سائيز = حجم(بنية bmp_سرو);

	إذا (bmp_ڀريو_addr + bmp_سرو_سائيز >= gd->شروعات_addr_sp)
		اذهب splash_پتو_too_مٿاهون;

	res = splash_storage_read_raw(location, bmp_ڀريو_addr, bmp_سرو_سائيز);
	إذا (res < 0)
		أرجع res;

	bmp_hdr = (بنية bmp_سرو *)(uintptr_t)bmp_ڀريو_addr;
	bmp_سائيز = le32_منزل_cpu(bmp_hdr->file_سائيز);

	إذا (bmp_ڀريو_addr + bmp_سائيز >= gd->شروعات_addr_sp)
		اذهب splash_پتو_too_مٿاهون;

	أرجع splash_storage_read_raw(location, bmp_ڀريو_addr, bmp_سائيز);

splash_پتو_too_مٿاهون:
	لکو("Error: splashimage address too high. Data overwrites U-Boot and/or placed beyond DRAM boundaries.\n");

	أرجع -EFAULT;
}

ساكن صحيح splash_select_fs_dev(بنية splash_location *location)
{
	صحيح res;

	بدل (location->storage) {
	حالة SPLASH_STORAGE_MMC:
		res = fs_سيٽ_blk_dev("mmc", location->devpart, FS_TYPE_ANY);
		اكسر;
	حالة SPLASH_STORAGE_USB:
		res = fs_سيٽ_blk_dev("usb", location->devpart, FS_TYPE_ANY);
		اكسر;
	حالة SPLASH_STORAGE_SATA:
		res = fs_سيٽ_blk_dev("sata", location->devpart, FS_TYPE_ANY);
		اكسر;
	حالة SPLASH_STORAGE_NAND:
		إذا (location->ubivol != عدم)
			res = fs_سيٽ_blk_dev("ubi", عدم, FS_TYPE_UBIFS);
		وإلا
			res = -ENODEV;
		اكسر;
	افتراضي:
		لکو("Error: %s: unsupported location storage.\n", __func__);
		أرجع -ENODEV;
	}

	إذا (res)
		لکو("Error: %s: could not access storage.\n", __func__);

	أرجع res;
}

#جيڪڏهن_وصف_ٿيل CONFIG_USB_STORAGE
ساكن صحيح splash_init_usb(فراغ)
{
	صحيح err;

	err = usb_init();
	إذا (err)
		أرجع err;

#جيڪڏهن_وصف_نه_ٿيل CONFIG_DM_USB
	err = usb_stor_جاچ(1) < 0 ? -ENODEV : 0;
#جيڪڏهن_پورو

	أرجع err;
}
#وإلا
ساكن مضمن صحيح splash_init_usb(فراغ)
{
	لکو("Cannot load splash image: no USB support\n");
	أرجع -ENOSYS;
}
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل CONFIG_SATA
ساكن صحيح splash_init_sata(فراغ)
{
	أرجع sata_probe(0);
}
#وإلا
ساكن مضمن صحيح splash_init_sata(فراغ)
{
	لکو("Cannot load splash image: no SATA support\n");
	أرجع -ENOSYS;
}
#جيڪڏهن_پورو

ساكن صحيح splash_init_virtio(فراغ)
{
	إذا (!IS_ENABLED(CONFIG_VIRTIO)) {
		لکو("Cannot load splash image: no virtio support\n");
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
	res = لاڳيتو_command(cmd, 0);
	إذا (res)
		أرجع res;

	sprintf(cmd, "ubifsmount %s", location->ubivol);
	res = لاڳيتو_command(cmd, 0);

	أرجع res;
}

ساكن مضمن صحيح splash_umount_ubifs(فراغ)
{
	أرجع لاڳيتو_command("ubifsumount", 0);
}
#وإلا
ساكن مضمن صحيح splash_mount_ubifs(بنية splash_location *location)
{
	لکو("Cannot load splash image: no UBIFS support\n");
	أرجع -ENOSYS;
}

ساكن مضمن صحيح splash_umount_ubifs(فراغ)
{
	لکو("Cannot unmount UBIFS: no UBIFS support\n");
	أرجع -ENOSYS;
}
#جيڪڏهن_پورو

#وصف_ڪريو SPLASH_ذريعو_DEFAULT_FILE_NAME		"splash.bmp"

ساكن صحيح splash_ڀريو_fs(بنية splash_location *location, ulong bmp_ڀريو_addr)
{
	صحيح res = 0;
	loff_t bmp_سائيز;
	loff_t actread;
	حرف *splash_file;

	splash_file = env_get("splashfile");
	إذا (!splash_file)
		splash_file = SPLASH_ذريعو_DEFAULT_FILE_NAME;

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
		اذهب ٻاهريون;

	res = fs_سائيز(splash_file, &bmp_سائيز);
	إذا (res) {
		لکو("Error: %s: cannot determine file size (%d)\n",
		       __func__, res);
		اذهب ٻاهريون;
	}

	إذا (bmp_ڀريو_addr + bmp_سائيز >= gd->شروعات_addr_sp) {
		لکو("Error: splashimage address too high. Data overwrites U-Boot and/or placed beyond DRAM boundaries.\n");
		res = -EFAULT;
		اذهب ٻاهريون;
	}

	splash_select_fs_dev(location);
	res = fs_read(splash_file, bmp_ڀريو_addr, 0, 0, &actread);

ٻاهريون:
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
			    بنية splash_location *locations, uint سائيز)
{
	صحيح i;
	حرف *env_splashsource;

	إذا (!locations || سائيز == 0)
		أرجع عدم;

	env_splashsource = env_get("splashsource");
	إذا (env_splashsource == عدم)
		أرجع &locations[0];

	لكل (i = 0; i < سائيز; i++) {
		إذا (!strcmp(locations[i].name, env_splashsource))
			أرجع &locations[i];
	}

	لکو("splashsource env variable set to unsupported value\n");
	أرجع عدم;
}

#جيڪڏهن_وصف_ٿيل CONFIG_FIT
ساكن صحيح splash_ڀريو_fit(بنية splash_location *location, ulong bmp_ڀريو_addr)
{
	صحيح res;
	صحيح ڳنڍ_وٿي;
	ثابت حرف *splash_file;
	ثابت فراغ *اندروني_splash_معلومات;
	size_t اندروني_splash_سائيز;
	صحيح external_splash_addr;
	u32 external_splash_سائيز;
	bool is_splash_external = false;
	بنية legacy_img_hdr *img_سرو;
	ثابت u32 *fit_سرو;
	u32 fit_سائيز;
	ثابت size_t سرو_سائيز = حجم(بنية legacy_img_hdr);

	/* Read in image header */
	res = splash_storage_read_raw(location, bmp_ڀريو_addr, سرو_سائيز);
	إذا (res < 0)
		أرجع res;

	img_سرو = (بنية legacy_img_hdr *)(uintptr_t)bmp_ڀريو_addr;
	إذا (image_get_magic(img_سرو) != FDT_MAGIC) {
		لکو("Could not find FDT magic\n");
		أرجع -EINVAL;
	}

	fit_سائيز = fdt_totalsize(img_سرو);

	/* Read in entire FIT */
	fit_سرو = (ثابت u32 *)(bmp_ڀريو_addr + سرو_سائيز);
	res = splash_storage_read_raw(location, (uintptr_t)fit_سرو, fit_سائيز);
	إذا (res < 0)
		أرجع res;

	res = fit_check_format(fit_سرو, IMAGE_سائيز_INVAL);
	إذا (res) {
		debug("Could not find valid FIT image\n");
		أرجع res;
	}

	/* Get the splash image node */
	splash_file = env_get("splashfile");
	إذا (!splash_file)
		splash_file = SPLASH_ذريعو_DEFAULT_FILE_NAME;

	ڳنڍ_وٿي = fit_image_get_ڳنڍ(fit_سرو, splash_file);
	إذا (ڳنڍ_وٿي < 0) {
		debug("Could not find splash image '%s' in FIT\n",
		      splash_file);
		أرجع -ENOENT;
	}

	/* Extract the splash data from FIT */
	إذا (!fit_image_get_معلومات(fit_سرو, ڳنڍ_وٿي, &اندروني_splash_معلومات,
				&اندروني_splash_سائيز)) {
		memmove((فراغ *)(uintptr_t)bmp_ڀريو_addr, اندروني_splash_معلومات,
			اندروني_splash_سائيز);
	} وإلا {
		لکو("Failed to get splash image from FIT\n");
		أرجع -ENODATA;
	}

	إذا (is_splash_external) {
		res = fit_image_get_معلومات_سائيز(fit_سرو, ڳنڍ_وٿي, &external_splash_سائيز);
		إذا (res < 0) {
			لکو("Failed to get size of splash image (err=%d)\n", res);
			أرجع res;
		}

		/* Read in the splash data */
		location->وٿي = external_splash_addr;
		res = splash_storage_read_raw(location, bmp_ڀريو_addr, external_splash_سائيز);
		إذا (res < 0)
			أرجع res;
	}

	أرجع 0;
}
#جيڪڏهن_پورو /* CONFIG_FIT */

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
صحيح splash_ذريعو_ڀريو(بنية splash_location *locations, uint سائيز)
{
	بنية splash_location *splash_location;
	حرف *env_splashimage_قدر;
	حرف *devpart;
	ulong bmp_ڀريو_addr;

	env_splashimage_قدر = env_get("splashimage");
	إذا (env_splashimage_قدر == عدم)
		أرجع -ENOENT;

	bmp_ڀريو_addr = hextoul(env_splashimage_قدر, 0);
	إذا (bmp_ڀريو_addr == 0) {
		لکو("Error: bad splashimage address specified\n");
		أرجع -EFAULT;
	}

	splash_location = select_splash_location(locations, سائيز);
	إذا (!splash_location)
		أرجع -EINVAL;

	devpart = env_get("splashdevpart");
	إذا (devpart)
		splash_location->devpart = devpart;

	إذا (splash_location->flags == SPLASH_STORAGE_RAW)
		أرجع splash_ڀريو_raw(splash_location, bmp_ڀريو_addr);
	وإلا إذا (splash_location->flags == SPLASH_STORAGE_FS)
		أرجع splash_ڀريو_fs(splash_location, bmp_ڀريو_addr);
#جيڪڏهن_وصف_ٿيل CONFIG_FIT
	وإلا إذا (splash_location->flags == SPLASH_STORAGE_FIT)
		أرجع splash_ڀريو_fit(splash_location, bmp_ڀريو_addr);
#جيڪڏهن_پورو
	أرجع -EINVAL;
}
