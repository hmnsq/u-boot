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

DECLARE_GLOBAL_انکڑے_PTR;

#جے_تعریف_شدہ CONFIG_SPI_FLASH
ساكن بنية spi_flash *sf;
ساكن صحيح splash_sf_read_raw(ulong bmp_لدو_addr, صحيح وتھ, size_t read_ناپ)
{
	إذا (!sf) {
		sf = spi_flash_probe(CONFIG_SF_DEFAULT_BUS,
				     CONFIG_SF_DEFAULT_CS,
				     CONFIG_SF_DEFAULT_رفتار,
				     CONFIG_SF_DEFAULT_منوال);
		إذا (!sf)
			أرجع -ENODEV;
	}

	أرجع spi_flash_read(sf, وتھ, read_ناپ, (فراغ *)(uintptr_t)bmp_لدو_addr);
}
#وإلا
ساكن صحيح splash_sf_read_raw(ulong bmp_لدو_addr, صحيح وتھ, size_t read_ناپ)
{
	debug("%s: sf support not available\n", __func__);
	أرجع -ENOSYS;
}
#جے_انت

#جے_تعریف_شدہ CONFIG_CMD_NAND
ساكن صحيح splash_nand_read_raw(ulong bmp_لدو_addr, صحيح وتھ, size_t read_ناپ)
{
	بنية mtd_info *mtd = get_nand_dev_by_اشاریہ(nand_curr_device);
	أرجع nand_read_skip_bad(mtd, وتھ,
				  &read_ناپ, عدم,
				  mtd->ناپ,
				  (u_char *)bmp_لدو_addr);
}
#وإلا
ساكن صحيح splash_nand_read_raw(ulong bmp_لدو_addr, صحيح وتھ, size_t read_ناپ)
{
	debug("%s: nand support not available\n", __func__);
	أرجع -ENOSYS;
}
#جے_انت

ساكن صحيح splash_mmc_read_raw(ulong bmp_لدو_addr, بنية splash_location *location,
			       size_t read_ناپ)
{
	بنية disk_حصے_کرو حصے_کرو;
	بنية blk_desc *desc;
	lbaint_t blkcnt;
	صحيح ret, n;

	إذا (!IS_ENABLED(CONFIG_CMD_MMC)) {
		debug("%s: mmc support not available\n", __func__);
		أرجع -ENOSYS;
	}

	ret = part_get_info_by_dev_and_name_or_عدد("mmc", location->devpart, &desc,
						   &حصے_کرو, 1);
	إذا (ret < 0)
		أرجع ret;

	blkcnt = DIV_گیڑا_اتے(read_ناپ, حصے_کرو.blksz);
	n = blk_dread(desc, حصے_کرو.شروع, blkcnt, (فراغ *)(uintptr_t)bmp_لدو_addr);

	أرجع (n == blkcnt) ? 0 : -EIO;
}

ساكن صحيح splash_storage_read_raw(بنية splash_location *location,
			       ulong bmp_لدو_addr, size_t read_ناپ)
{
	u32 وتھ;

	إذا (!location)
		أرجع -EINVAL;

	وتھ = location->وتھ;
	بدل (location->storage) {
	حالة SPLASH_STORAGE_MMC:
		أرجع splash_mmc_read_raw(bmp_لدو_addr, location, read_ناپ);
	حالة SPLASH_STORAGE_NAND:
		أرجع splash_nand_read_raw(bmp_لدو_addr, وتھ, read_ناپ);
	حالة SPLASH_STORAGE_SF:
		أرجع splash_sf_read_raw(bmp_لدو_addr, وتھ, read_ناپ);
	افتراضي:
		لکھو("Unknown splash location\n");
	}

	أرجع -EINVAL;
}

ساكن صحيح splash_لدو_raw(بنية splash_location *location, ulong bmp_لدو_addr)
{
	بنية bmp_سرخی *bmp_hdr;
	صحيح res;
	size_t bmp_ناپ, bmp_سرخی_ناپ = حجم(بنية bmp_سرخی);

	إذا (bmp_لدو_addr + bmp_سرخی_ناپ >= gd->شروع_addr_sp)
		اذهب splash_پتہ_too_بلند;

	res = splash_storage_read_raw(location, bmp_لدو_addr, bmp_سرخی_ناپ);
	إذا (res < 0)
		أرجع res;

	bmp_hdr = (بنية bmp_سرخی *)(uintptr_t)bmp_لدو_addr;
	bmp_ناپ = le32_منزل_cpu(bmp_hdr->file_ناپ);

	إذا (bmp_لدو_addr + bmp_ناپ >= gd->شروع_addr_sp)
		اذهب splash_پتہ_too_بلند;

	أرجع splash_storage_read_raw(location, bmp_لدو_addr, bmp_ناپ);

splash_پتہ_too_بلند:
	لکھو("Error: splashimage address too high. Data overwrites U-Boot and/or placed beyond DRAM boundaries.\n");

	أرجع -EFAULT;
}

ساكن صحيح splash_select_fs_dev(بنية splash_location *location)
{
	صحيح res;

	بدل (location->storage) {
	حالة SPLASH_STORAGE_MMC:
		res = fs_سیٹ_blk_dev("mmc", location->devpart, FS_TYPE_ANY);
		اكسر;
	حالة SPLASH_STORAGE_USB:
		res = fs_سیٹ_blk_dev("usb", location->devpart, FS_TYPE_ANY);
		اكسر;
	حالة SPLASH_STORAGE_SATA:
		res = fs_سیٹ_blk_dev("sata", location->devpart, FS_TYPE_ANY);
		اكسر;
	حالة SPLASH_STORAGE_NAND:
		إذا (location->ubivol != عدم)
			res = fs_سیٹ_blk_dev("ubi", عدم, FS_TYPE_UBIFS);
		وإلا
			res = -ENODEV;
		اكسر;
	افتراضي:
		لکھو("Error: %s: unsupported location storage.\n", __func__);
		أرجع -ENODEV;
	}

	إذا (res)
		لکھو("Error: %s: could not access storage.\n", __func__);

	أرجع res;
}

#جے_تعریف_شدہ CONFIG_USB_STORAGE
ساكن صحيح splash_init_usb(فراغ)
{
	صحيح err;

	err = usb_init();
	إذا (err)
		أرجع err;

#جے_بنا_تعریف CONFIG_DM_USB
	err = usb_stor_جانچ(1) < 0 ? -ENODEV : 0;
#جے_انت

	أرجع err;
}
#وإلا
ساكن مضمن صحيح splash_init_usb(فراغ)
{
	لکھو("Cannot load splash image: no USB support\n");
	أرجع -ENOSYS;
}
#جے_انت

#جے_تعریف_شدہ CONFIG_SATA
ساكن صحيح splash_init_sata(فراغ)
{
	أرجع sata_probe(0);
}
#وإلا
ساكن مضمن صحيح splash_init_sata(فراغ)
{
	لکھو("Cannot load splash image: no SATA support\n");
	أرجع -ENOSYS;
}
#جے_انت

ساكن صحيح splash_init_virtio(فراغ)
{
	إذا (!IS_ENABLED(CONFIG_VIRTIO)) {
		لکھو("Cannot load splash image: no virtio support\n");
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
	res = لگاتار_command(cmd, 0);
	إذا (res)
		أرجع res;

	sprintf(cmd, "ubifsmount %s", location->ubivol);
	res = لگاتار_command(cmd, 0);

	أرجع res;
}

ساكن مضمن صحيح splash_umount_ubifs(فراغ)
{
	أرجع لگاتار_command("ubifsumount", 0);
}
#وإلا
ساكن مضمن صحيح splash_mount_ubifs(بنية splash_location *location)
{
	لکھو("Cannot load splash image: no UBIFS support\n");
	أرجع -ENOSYS;
}

ساكن مضمن صحيح splash_umount_ubifs(فراغ)
{
	لکھو("Cannot unmount UBIFS: no UBIFS support\n");
	أرجع -ENOSYS;
}
#جے_انت

#تعریف_کرو SPLASH_سروت_DEFAULT_FILE_NAME		"splash.bmp"

ساكن صحيح splash_لدو_fs(بنية splash_location *location, ulong bmp_لدو_addr)
{
	صحيح res = 0;
	loff_t bmp_ناپ;
	loff_t actread;
	حرف *splash_file;

	splash_file = env_get("splashfile");
	إذا (!splash_file)
		splash_file = SPLASH_سروت_DEFAULT_FILE_NAME;

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
		اذهب باہرلا;

	res = fs_ناپ(splash_file, &bmp_ناپ);
	إذا (res) {
		لکھو("Error: %s: cannot determine file size (%d)\n",
		       __func__, res);
		اذهب باہرلا;
	}

	إذا (bmp_لدو_addr + bmp_ناپ >= gd->شروع_addr_sp) {
		لکھو("Error: splashimage address too high. Data overwrites U-Boot and/or placed beyond DRAM boundaries.\n");
		res = -EFAULT;
		اذهب باہرلا;
	}

	splash_select_fs_dev(location);
	res = fs_read(splash_file, bmp_لدو_addr, 0, 0, &actread);

باہرلا:
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
			    بنية splash_location *locations, uint ناپ)
{
	صحيح i;
	حرف *env_splashsource;

	إذا (!locations || ناپ == 0)
		أرجع عدم;

	env_splashsource = env_get("splashsource");
	إذا (env_splashsource == عدم)
		أرجع &locations[0];

	لكل (i = 0; i < ناپ; i++) {
		إذا (!strcmp(locations[i].name, env_splashsource))
			أرجع &locations[i];
	}

	لکھو("splashsource env variable set to unsupported value\n");
	أرجع عدم;
}

#جے_تعریف_شدہ CONFIG_FIT
ساكن صحيح splash_لدو_fit(بنية splash_location *location, ulong bmp_لدو_addr)
{
	صحيح res;
	صحيح گرہ_وتھ;
	ثابت حرف *splash_file;
	ثابت فراغ *اندرونی_splash_انکڑے;
	size_t اندرونی_splash_ناپ;
	صحيح external_splash_addr;
	u32 external_splash_ناپ;
	bool is_splash_external = false;
	بنية legacy_img_hdr *img_سرخی;
	ثابت u32 *fit_سرخی;
	u32 fit_ناپ;
	ثابت size_t سرخی_ناپ = حجم(بنية legacy_img_hdr);

	/* Read in image header */
	res = splash_storage_read_raw(location, bmp_لدو_addr, سرخی_ناپ);
	إذا (res < 0)
		أرجع res;

	img_سرخی = (بنية legacy_img_hdr *)(uintptr_t)bmp_لدو_addr;
	إذا (image_get_magic(img_سرخی) != FDT_MAGIC) {
		لکھو("Could not find FDT magic\n");
		أرجع -EINVAL;
	}

	fit_ناپ = fdt_totalsize(img_سرخی);

	/* Read in entire FIT */
	fit_سرخی = (ثابت u32 *)(bmp_لدو_addr + سرخی_ناپ);
	res = splash_storage_read_raw(location, (uintptr_t)fit_سرخی, fit_ناپ);
	إذا (res < 0)
		أرجع res;

	res = fit_check_format(fit_سرخی, IMAGE_ناپ_INVAL);
	إذا (res) {
		debug("Could not find valid FIT image\n");
		أرجع res;
	}

	/* Get the splash image node */
	splash_file = env_get("splashfile");
	إذا (!splash_file)
		splash_file = SPLASH_سروت_DEFAULT_FILE_NAME;

	گرہ_وتھ = fit_image_get_گرہ(fit_سرخی, splash_file);
	إذا (گرہ_وتھ < 0) {
		debug("Could not find splash image '%s' in FIT\n",
		      splash_file);
		أرجع -ENOENT;
	}

	/* Extract the splash data from FIT */
	إذا (!fit_image_get_انکڑے(fit_سرخی, گرہ_وتھ, &اندرونی_splash_انکڑے,
				&اندرونی_splash_ناپ)) {
		memmove((فراغ *)(uintptr_t)bmp_لدو_addr, اندرونی_splash_انکڑے,
			اندرونی_splash_ناپ);
	} وإلا {
		لکھو("Failed to get splash image from FIT\n");
		أرجع -ENODATA;
	}

	إذا (is_splash_external) {
		res = fit_image_get_انکڑے_ناپ(fit_سرخی, گرہ_وتھ, &external_splash_ناپ);
		إذا (res < 0) {
			لکھو("Failed to get size of splash image (err=%d)\n", res);
			أرجع res;
		}

		/* Read in the splash data */
		location->وتھ = external_splash_addr;
		res = splash_storage_read_raw(location, bmp_لدو_addr, external_splash_ناپ);
		إذا (res < 0)
			أرجع res;
	}

	أرجع 0;
}
#جے_انت /* CONFIG_FIT */

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
صحيح splash_سروت_لدو(بنية splash_location *locations, uint ناپ)
{
	بنية splash_location *splash_location;
	حرف *env_splashimage_قدر;
	حرف *devpart;
	ulong bmp_لدو_addr;

	env_splashimage_قدر = env_get("splashimage");
	إذا (env_splashimage_قدر == عدم)
		أرجع -ENOENT;

	bmp_لدو_addr = hextoul(env_splashimage_قدر, 0);
	إذا (bmp_لدو_addr == 0) {
		لکھو("Error: bad splashimage address specified\n");
		أرجع -EFAULT;
	}

	splash_location = select_splash_location(locations, ناپ);
	إذا (!splash_location)
		أرجع -EINVAL;

	devpart = env_get("splashdevpart");
	إذا (devpart)
		splash_location->devpart = devpart;

	إذا (splash_location->flags == SPLASH_STORAGE_RAW)
		أرجع splash_لدو_raw(splash_location, bmp_لدو_addr);
	وإلا إذا (splash_location->flags == SPLASH_STORAGE_FS)
		أرجع splash_لدو_fs(splash_location, bmp_لدو_addr);
#جے_تعریف_شدہ CONFIG_FIT
	وإلا إذا (splash_location->flags == SPLASH_STORAGE_FIT)
		أرجع splash_لدو_fit(splash_location, bmp_لدو_addr);
#جے_انت
	أرجع -EINVAL;
}
