// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014 CompuLab, Ltd. <www.compulab.co.il>
 *
 * Authors: Igor Grinberg <grinberg@compulab.co.il>
 */

#شامل <bmp_layout.h>
#شامل <command.h>
#شامل <env.h>
#شامل <errno.h>
#شامل <fs.h>
#شامل <fdt_support.h>
#شامل <image.h>
#شامل <log.h>
#شامل <nand.h>
#شامل <sata.h>
#شامل <spi.h>
#شامل <spi_flash.h>
#شامل <splash.h>
#شامل <usb.h>
#شامل <virtio.h>
#شامل <asm/global_data.h>

DECLARE_GLOBAL_ڈیٹا_PTR;

#اگر_تعریف_شدہ CONFIG_SPI_FLASH
ساکن ساخت spi_flash *sf;
ساکن صحیح splash_sf_read_raw(ulong bmp_بھرو_addr, صحیح انحراف, size_t read_سائز)
{
	اگر (!sf) {
		sf = spi_flash_probe(CONFIG_SF_DEFAULT_BUS,
				     CONFIG_SF_DEFAULT_CS,
				     CONFIG_SF_DEFAULT_رفتار,
				     CONFIG_SF_DEFAULT_منوال);
		اگر (!sf)
			واپس -ENODEV;
	}

	واپس spi_flash_read(sf, انحراف, read_سائز, (خالی *)(uintptr_t)bmp_بھرو_addr);
}
#ورنہ
ساکن صحیح splash_sf_read_raw(ulong bmp_بھرو_addr, صحیح انحراف, size_t read_سائز)
{
	debug("%s: sf support not available\n", __func__);
	واپس -ENOSYS;
}
#اگر_ختم

#اگر_تعریف_شدہ CONFIG_CMD_NAND
ساکن صحیح splash_nand_read_raw(ulong bmp_بھرو_addr, صحیح انحراف, size_t read_سائز)
{
	ساخت mtd_info *mtd = get_nand_dev_by_اشاریہ(nand_curr_device);
	واپس nand_read_skip_bad(mtd, انحراف,
				  &read_سائز, تہی,
				  mtd->سائز,
				  (u_char *)bmp_بھرو_addr);
}
#ورنہ
ساکن صحیح splash_nand_read_raw(ulong bmp_بھرو_addr, صحیح انحراف, size_t read_سائز)
{
	debug("%s: nand support not available\n", __func__);
	واپس -ENOSYS;
}
#اگر_ختم

ساکن صحیح splash_mmc_read_raw(ulong bmp_بھرو_addr, ساخت splash_location *location,
			       size_t read_سائز)
{
	ساخت disk_تقسیم تقسیم;
	ساخت blk_desc *desc;
	lbaint_t blkcnt;
	صحیح ret, n;

	اگر (!IS_ENABLED(CONFIG_CMD_MMC)) {
		debug("%s: mmc support not available\n", __func__);
		واپس -ENOSYS;
	}

	ret = part_get_info_by_dev_and_name_or_عدد("mmc", location->devpart, &desc,
						   &تقسیم, 1);
	اگر (ret < 0)
		واپس ret;

	blkcnt = DIV_باری_اوپر(read_سائز, تقسیم.blksz);
	n = blk_dread(desc, تقسیم.آغاز, blkcnt, (خالی *)(uintptr_t)bmp_بھرو_addr);

	واپس (n == blkcnt) ? 0 : -EIO;
}

ساکن صحیح splash_storage_read_raw(ساخت splash_location *location,
			       ulong bmp_بھرو_addr, size_t read_سائز)
{
	u32 انحراف;

	اگر (!location)
		واپس -EINVAL;

	انحراف = location->انحراف;
	انتخاب (location->storage) {
	حالت SPLASH_STORAGE_MMC:
		واپس splash_mmc_read_raw(bmp_بھرو_addr, location, read_سائز);
	حالت SPLASH_STORAGE_NAND:
		واپس splash_nand_read_raw(bmp_بھرو_addr, انحراف, read_سائز);
	حالت SPLASH_STORAGE_SF:
		واپس splash_sf_read_raw(bmp_بھرو_addr, انحراف, read_سائز);
	طےشدہ:
		لکھو("Unknown splash location\n");
	}

	واپس -EINVAL;
}

ساکن صحیح splash_بھرو_raw(ساخت splash_location *location, ulong bmp_بھرو_addr)
{
	ساخت bmp_سرخی *bmp_hdr;
	صحیح res;
	size_t bmp_سائز, bmp_سرخی_سائز = حجم(ساخت bmp_سرخی);

	اگر (bmp_بھرو_addr + bmp_سرخی_سائز >= gd->آغاز_addr_sp)
		جاؤ splash_پتہ_too_بلند;

	res = splash_storage_read_raw(location, bmp_بھرو_addr, bmp_سرخی_سائز);
	اگر (res < 0)
		واپس res;

	bmp_hdr = (ساخت bmp_سرخی *)(uintptr_t)bmp_بھرو_addr;
	bmp_سائز = le32_مقصد_cpu(bmp_hdr->file_سائز);

	اگر (bmp_بھرو_addr + bmp_سائز >= gd->آغاز_addr_sp)
		جاؤ splash_پتہ_too_بلند;

	واپس splash_storage_read_raw(location, bmp_بھرو_addr, bmp_سائز);

splash_پتہ_too_بلند:
	لکھو("Error: splashimage address too high. Data overwrites U-Boot and/or placed beyond DRAM boundaries.\n");

	واپس -EFAULT;
}

ساکن صحیح splash_select_fs_dev(ساخت splash_location *location)
{
	صحیح res;

	انتخاب (location->storage) {
	حالت SPLASH_STORAGE_MMC:
		res = fs_سیٹ_blk_dev("mmc", location->devpart, FS_TYPE_ANY);
		توڑو;
	حالت SPLASH_STORAGE_USB:
		res = fs_سیٹ_blk_dev("usb", location->devpart, FS_TYPE_ANY);
		توڑو;
	حالت SPLASH_STORAGE_SATA:
		res = fs_سیٹ_blk_dev("sata", location->devpart, FS_TYPE_ANY);
		توڑو;
	حالت SPLASH_STORAGE_NAND:
		اگر (location->ubivol != تہی)
			res = fs_سیٹ_blk_dev("ubi", تہی, FS_TYPE_UBIFS);
		ورنہ
			res = -ENODEV;
		توڑو;
	طےشدہ:
		لکھو("Error: %s: unsupported location storage.\n", __func__);
		واپس -ENODEV;
	}

	اگر (res)
		لکھو("Error: %s: could not access storage.\n", __func__);

	واپس res;
}

#اگر_تعریف_شدہ CONFIG_USB_STORAGE
ساکن صحیح splash_init_usb(خالی)
{
	صحیح err;

	err = usb_init();
	اگر (err)
		واپس err;

#اگر_غیر_تعریف_شدہ CONFIG_DM_USB
	err = usb_stor_جائزہ(1) < 0 ? -ENODEV : 0;
#اگر_ختم

	واپس err;
}
#ورنہ
ساکن اندرونی صحیح splash_init_usb(خالی)
{
	لکھو("Cannot load splash image: no USB support\n");
	واپس -ENOSYS;
}
#اگر_ختم

#اگر_تعریف_شدہ CONFIG_SATA
ساکن صحیح splash_init_sata(خالی)
{
	واپس sata_probe(0);
}
#ورنہ
ساکن اندرونی صحیح splash_init_sata(خالی)
{
	لکھو("Cannot load splash image: no SATA support\n");
	واپس -ENOSYS;
}
#اگر_ختم

ساکن صحیح splash_init_virtio(خالی)
{
	اگر (!IS_ENABLED(CONFIG_VIRTIO)) {
		لکھو("Cannot load splash image: no virtio support\n");
		واپس -ENOSYS;
	} ورنہ {
		واپس virtio_init();
	}
}

#اگر defined(CONFIG_CMD_UBIFS) && !defined(CONFIG_XPL_BUILD)
ساکن صحیح splash_mount_ubifs(ساخت splash_location *location)
{
	صحیح res;
	حرف cmd[32];

	sprintf(cmd, "ubi part %s", location->mtdpart);
	res = تسلسل_command(cmd, 0);
	اگر (res)
		واپس res;

	sprintf(cmd, "ubifsmount %s", location->ubivol);
	res = تسلسل_command(cmd, 0);

	واپس res;
}

ساکن اندرونی صحیح splash_umount_ubifs(خالی)
{
	واپس تسلسل_command("ubifsumount", 0);
}
#ورنہ
ساکن اندرونی صحیح splash_mount_ubifs(ساخت splash_location *location)
{
	لکھو("Cannot load splash image: no UBIFS support\n");
	واپس -ENOSYS;
}

ساکن اندرونی صحیح splash_umount_ubifs(خالی)
{
	لکھو("Cannot unmount UBIFS: no UBIFS support\n");
	واپس -ENOSYS;
}
#اگر_ختم

#تعریف_کرو SPLASH_ماخذ_DEFAULT_FILE_NAME		"splash.bmp"

ساکن صحیح splash_بھرو_fs(ساخت splash_location *location, ulong bmp_بھرو_addr)
{
	صحیح res = 0;
	loff_t bmp_سائز;
	loff_t actread;
	حرف *splash_file;

	splash_file = env_get("splashfile");
	اگر (!splash_file)
		splash_file = SPLASH_ماخذ_DEFAULT_FILE_NAME;

	اگر (location->storage == SPLASH_STORAGE_USB)
		res = splash_init_usb();

	اگر (location->storage == SPLASH_STORAGE_SATA)
		res = splash_init_sata();

	اگر (location->storage == SPLASH_STORAGE_VIRTIO)
		res = splash_init_virtio();

	اگر (location->ubivol != تہی)
		res = splash_mount_ubifs(location);

	اگر (res)
		واپس res;

	res = splash_select_fs_dev(location);
	اگر (res)
		جاؤ برآمد;

	res = fs_سائز(splash_file, &bmp_سائز);
	اگر (res) {
		لکھو("Error: %s: cannot determine file size (%d)\n",
		       __func__, res);
		جاؤ برآمد;
	}

	اگر (bmp_بھرو_addr + bmp_سائز >= gd->آغاز_addr_sp) {
		لکھو("Error: splashimage address too high. Data overwrites U-Boot and/or placed beyond DRAM boundaries.\n");
		res = -EFAULT;
		جاؤ برآمد;
	}

	splash_select_fs_dev(location);
	res = fs_read(splash_file, bmp_بھرو_addr, 0, 0, &actread);

برآمد:
	اگر (location->ubivol != تہی)
		splash_umount_ubifs();

	واپس res;
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
ساکن ساخت splash_location *select_splash_location(
			    ساخت splash_location *locations, uint سائز)
{
	صحیح i;
	حرف *env_splashsource;

	اگر (!locations || سائز == 0)
		واپس تہی;

	env_splashsource = env_get("splashsource");
	اگر (env_splashsource == تہی)
		واپس &locations[0];

	برائے (i = 0; i < سائز; i++) {
		اگر (!strcmp(locations[i].name, env_splashsource))
			واپس &locations[i];
	}

	لکھو("splashsource env variable set to unsupported value\n");
	واپس تہی;
}

#اگر_تعریف_شدہ CONFIG_FIT
ساکن صحیح splash_بھرو_fit(ساخت splash_location *location, ulong bmp_بھرو_addr)
{
	صحیح res;
	صحیح گرہ_انحراف;
	ثابت حرف *splash_file;
	ثابت خالی *اندرونی_splash_ڈیٹا;
	size_t اندرونی_splash_سائز;
	صحیح external_splash_addr;
	u32 external_splash_سائز;
	bool is_splash_external = false;
	ساخت legacy_img_hdr *img_سرخی;
	ثابت u32 *fit_سرخی;
	u32 fit_سائز;
	ثابت size_t سرخی_سائز = حجم(ساخت legacy_img_hdr);

	/* Read in image header */
	res = splash_storage_read_raw(location, bmp_بھرو_addr, سرخی_سائز);
	اگر (res < 0)
		واپس res;

	img_سرخی = (ساخت legacy_img_hdr *)(uintptr_t)bmp_بھرو_addr;
	اگر (image_get_magic(img_سرخی) != FDT_MAGIC) {
		لکھو("Could not find FDT magic\n");
		واپس -EINVAL;
	}

	fit_سائز = fdt_totalsize(img_سرخی);

	/* Read in entire FIT */
	fit_سرخی = (ثابت u32 *)(bmp_بھرو_addr + سرخی_سائز);
	res = splash_storage_read_raw(location, (uintptr_t)fit_سرخی, fit_سائز);
	اگر (res < 0)
		واپس res;

	res = fit_check_format(fit_سرخی, IMAGE_سائز_INVAL);
	اگر (res) {
		debug("Could not find valid FIT image\n");
		واپس res;
	}

	/* Get the splash image node */
	splash_file = env_get("splashfile");
	اگر (!splash_file)
		splash_file = SPLASH_ماخذ_DEFAULT_FILE_NAME;

	گرہ_انحراف = fit_image_get_گرہ(fit_سرخی, splash_file);
	اگر (گرہ_انحراف < 0) {
		debug("Could not find splash image '%s' in FIT\n",
		      splash_file);
		واپس -ENOENT;
	}

	/* Extract the splash data from FIT */
	اگر (!fit_image_get_ڈیٹا(fit_سرخی, گرہ_انحراف, &اندرونی_splash_ڈیٹا,
				&اندرونی_splash_سائز)) {
		memmove((خالی *)(uintptr_t)bmp_بھرو_addr, اندرونی_splash_ڈیٹا,
			اندرونی_splash_سائز);
	} ورنہ {
		لکھو("Failed to get splash image from FIT\n");
		واپس -ENODATA;
	}

	اگر (is_splash_external) {
		res = fit_image_get_ڈیٹا_سائز(fit_سرخی, گرہ_انحراف, &external_splash_سائز);
		اگر (res < 0) {
			لکھو("Failed to get size of splash image (err=%d)\n", res);
			واپس res;
		}

		/* Read in the splash data */
		location->انحراف = external_splash_addr;
		res = splash_storage_read_raw(location, bmp_بھرو_addr, external_splash_سائز);
		اگر (res < 0)
			واپس res;
	}

	واپس 0;
}
#اگر_ختم /* CONFIG_FIT */

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
صحیح splash_ماخذ_بھرو(ساخت splash_location *locations, uint سائز)
{
	ساخت splash_location *splash_location;
	حرف *env_splashimage_قدر;
	حرف *devpart;
	ulong bmp_بھرو_addr;

	env_splashimage_قدر = env_get("splashimage");
	اگر (env_splashimage_قدر == تہی)
		واپس -ENOENT;

	bmp_بھرو_addr = hextoul(env_splashimage_قدر, 0);
	اگر (bmp_بھرو_addr == 0) {
		لکھو("Error: bad splashimage address specified\n");
		واپس -EFAULT;
	}

	splash_location = select_splash_location(locations, سائز);
	اگر (!splash_location)
		واپس -EINVAL;

	devpart = env_get("splashdevpart");
	اگر (devpart)
		splash_location->devpart = devpart;

	اگر (splash_location->flags == SPLASH_STORAGE_RAW)
		واپس splash_بھرو_raw(splash_location, bmp_بھرو_addr);
	ورنہ اگر (splash_location->flags == SPLASH_STORAGE_FS)
		واپس splash_بھرو_fs(splash_location, bmp_بھرو_addr);
#اگر_تعریف_شدہ CONFIG_FIT
	ورنہ اگر (splash_location->flags == SPLASH_STORAGE_FIT)
		واپس splash_بھرو_fit(splash_location, bmp_بھرو_addr);
#اگر_ختم
	واپس -EINVAL;
}
