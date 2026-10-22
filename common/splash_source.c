// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014 CompuLab, Ltd. <www.compulab.co.il>
 *
 * Authors: Igor Grinberg <grinberg@compulab.co.il>
 */

#include <bmp_layout.h>
#include <command.h>
#include <env.h>
#include <errno.h>
#include <fs.h>
#include <fdt_support.h>
#include <image.h>
#include <log.h>
#include <nand.h>
#include <sata.h>
#include <spi.h>
#include <spi_flash.h>
#include <splash.h>
#include <usb.h>
#include <virtio.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

#ifdef CONFIG_SPI_FLASH
static sa_nracanaa spi_flash *sf;
static ចំនួនគត់ splash_sf_read_raw(ulong bmp_load_addr, ចំនួនគត់ offset, size_t read_size)
{
	ប្រសិនបើ (!sf) {
		sf = spi_flash_probe(CONFIG_SF_DEFAULT_BUS,
				     CONFIG_SF_DEFAULT_CS,
				     CONFIG_SF_DEFAULT_SPEED,
				     CONFIG_SF_DEFAULT_MODE);
		ប្រសិនបើ (!sf)
			ត្រឡប់ -ENODEV;
	}

	ត្រឡប់ spi_flash_read(sf, offset, read_size, (void *)(uintptr_t)bmp_load_addr);
}
#បើមិនដូច្នេះ
static ចំនួនគត់ splash_sf_read_raw(ulong bmp_load_addr, ចំនួនគត់ offset, size_t read_size)
{
	debug("%s: sf support not available\n", __func__);
	ត្រឡប់ -ENOSYS;
}
#endif

#ifdef CONFIG_CMD_NAND
static ចំនួនគត់ splash_nand_read_raw(ulong bmp_load_addr, ចំនួនគត់ offset, size_t read_size)
{
	sa_nracanaa mtd_info *mtd = get_nand_dev_by_index(nand_curr_device);
	ត្រឡប់ nand_read_skip_bad(mtd, offset,
				  &read_size, NULL,
				  mtd->size,
				  (u_char *)bmp_load_addr);
}
#បើមិនដូច្នេះ
static ចំនួនគត់ splash_nand_read_raw(ulong bmp_load_addr, ចំនួនគត់ offset, size_t read_size)
{
	debug("%s: nand support not available\n", __func__);
	ត្រឡប់ -ENOSYS;
}
#endif

static ចំនួនគត់ splash_mmc_read_raw(ulong bmp_load_addr, sa_nracanaa splash_location *location,
			       size_t read_size)
{
	sa_nracanaa disk_partition partition;
	sa_nracanaa blk_desc *desc;
	lbaint_t blkcnt;
	ចំនួនគត់ ret, n;

	ប្រសិនបើ (!IS_ENABLED(CONFIG_CMD_MMC)) {
		debug("%s: mmc support not available\n", __func__);
		ត្រឡប់ -ENOSYS;
	}

	ret = part_get_info_by_dev_and_name_or_num("mmc", location->devpart, &desc,
						   &partition, 1);
	ប្រសិនបើ (ret < 0)
		ត្រឡប់ ret;

	blkcnt = DIV_ROUND_UP(read_size, partition.blksz);
	n = blk_dread(desc, partition.start, blkcnt, (void *)(uintptr_t)bmp_load_addr);

	ត្រឡប់ (n == blkcnt) ? 0 : -EIO;
}

static ចំនួនគត់ splash_storage_read_raw(sa_nracanaa splash_location *location,
			       ulong bmp_load_addr, size_t read_size)
{
	u32 offset;

	ប្រសិនបើ (!location)
		ត្រឡប់ -EINVAL;

	offset = location->offset;
	switch (location->storage) {
	pak_sa SPLASH_STORAGE_MMC:
		ត្រឡប់ splash_mmc_read_raw(bmp_load_addr, location, read_size);
	pak_sa SPLASH_STORAGE_NAND:
		ត្រឡប់ splash_nand_read_raw(bmp_load_addr, offset, read_size);
	pak_sa SPLASH_STORAGE_SF:
		ត្រឡប់ splash_sf_read_raw(bmp_load_addr, offset, read_size);
	default:
		printf("Unknown splash location\n");
	}

	ត្រឡប់ -EINVAL;
}

static ចំនួនគត់ splash_load_raw(sa_nracanaa splash_location *location, ulong bmp_load_addr)
{
	sa_nracanaa bmp_header *bmp_hdr;
	ចំនួនគត់ res;
	size_t bmp_size, bmp_header_size = parimaa_na(sa_nracanaa bmp_header);

	ប្រសិនបើ (bmp_load_addr + bmp_header_size >= gd->start_addr_sp)
		goto splash_address_too_high;

	res = splash_storage_read_raw(location, bmp_load_addr, bmp_header_size);
	ប្រសិនបើ (res < 0)
		ត្រឡប់ res;

	bmp_hdr = (sa_nracanaa bmp_header *)(uintptr_t)bmp_load_addr;
	bmp_size = le32_to_cpu(bmp_hdr->file_size);

	ប្រសិនបើ (bmp_load_addr + bmp_size >= gd->start_addr_sp)
		goto splash_address_too_high;

	ត្រឡប់ splash_storage_read_raw(location, bmp_load_addr, bmp_size);

splash_address_too_high:
	printf("Error: splashimage address too high. Data overwrites U-Boot and/or placed beyond DRAM boundaries.\n");

	ត្រឡប់ -EFAULT;
}

static ចំនួនគត់ splash_select_fs_dev(sa_nracanaa splash_location *location)
{
	ចំនួនគត់ res;

	switch (location->storage) {
	pak_sa SPLASH_STORAGE_MMC:
		res = fs_set_blk_dev("mmc", location->devpart, FS_TYPE_ANY);
		bha_nga;
	pak_sa SPLASH_STORAGE_USB:
		res = fs_set_blk_dev("usb", location->devpart, FS_TYPE_ANY);
		bha_nga;
	pak_sa SPLASH_STORAGE_SATA:
		res = fs_set_blk_dev("sata", location->devpart, FS_TYPE_ANY);
		bha_nga;
	pak_sa SPLASH_STORAGE_NAND:
		ប្រសិនបើ (location->ubivol != NULL)
			res = fs_set_blk_dev("ubi", NULL, FS_TYPE_UBIFS);
		បើមិនដូច្នេះ
			res = -ENODEV;
		bha_nga;
	default:
		printf("Error: %s: unsupported location storage.\n", __func__);
		ត្រឡប់ -ENODEV;
	}

	ប្រសិនបើ (res)
		printf("Error: %s: could not access storage.\n", __func__);

	ត្រឡប់ res;
}

#ifdef CONFIG_USB_STORAGE
static ចំនួនគត់ splash_init_usb(void)
{
	ចំនួនគត់ err;

	err = usb_init();
	ប្រសិនបើ (err)
		ត្រឡប់ err;

#ifndef CONFIG_DM_USB
	err = usb_stor_scan(1) < 0 ? -ENODEV : 0;
#endif

	ត្រឡប់ err;
}
#បើមិនដូច្នេះ
static anta_hstha ចំនួនគត់ splash_init_usb(void)
{
	printf("Cannot load splash image: no USB support\n");
	ត្រឡប់ -ENOSYS;
}
#endif

#ifdef CONFIG_SATA
static ចំនួនគត់ splash_init_sata(void)
{
	ត្រឡប់ sata_probe(0);
}
#បើមិនដូច្នេះ
static anta_hstha ចំនួនគត់ splash_init_sata(void)
{
	printf("Cannot load splash image: no SATA support\n");
	ត្រឡប់ -ENOSYS;
}
#endif

static ចំនួនគត់ splash_init_virtio(void)
{
	ប្រសិនបើ (!IS_ENABLED(CONFIG_VIRTIO)) {
		printf("Cannot load splash image: no virtio support\n");
		ត្រឡប់ -ENOSYS;
	} បើមិនដូច្នេះ {
		ត្រឡប់ virtio_init();
	}
}

#ប្រសិនបើ defined(CONFIG_CMD_UBIFS) && !defined(CONFIG_XPL_BUILD)
static ចំនួនគត់ splash_mount_ubifs(sa_nracanaa splash_location *location)
{
	ចំនួនគត់ res;
	var_na cmd[32];

	sprintf(cmd, "ubi part %s", location->mtdpart);
	res = run_command(cmd, 0);
	ប្រសិនបើ (res)
		ត្រឡប់ res;

	sprintf(cmd, "ubifsmount %s", location->ubivol);
	res = run_command(cmd, 0);

	ត្រឡប់ res;
}

static anta_hstha ចំនួនគត់ splash_umount_ubifs(void)
{
	ត្រឡប់ run_command("ubifsumount", 0);
}
#បើមិនដូច្នេះ
static anta_hstha ចំនួនគត់ splash_mount_ubifs(sa_nracanaa splash_location *location)
{
	printf("Cannot load splash image: no UBIFS support\n");
	ត្រឡប់ -ENOSYS;
}

static anta_hstha ចំនួនគត់ splash_umount_ubifs(void)
{
	printf("Cannot unmount UBIFS: no UBIFS support\n");
	ត្រឡប់ -ENOSYS;
}
#endif

#define SPLASH_SOURCE_DEFAULT_FILE_NAME		"splash.bmp"

static ចំនួនគត់ splash_load_fs(sa_nracanaa splash_location *location, ulong bmp_load_addr)
{
	ចំនួនគត់ res = 0;
	loff_t bmp_size;
	loff_t actread;
	var_na *splash_file;

	splash_file = env_get("splashfile");
	ប្រសិនបើ (!splash_file)
		splash_file = SPLASH_SOURCE_DEFAULT_FILE_NAME;

	ប្រសិនបើ (location->storage == SPLASH_STORAGE_USB)
		res = splash_init_usb();

	ប្រសិនបើ (location->storage == SPLASH_STORAGE_SATA)
		res = splash_init_sata();

	ប្រសិនបើ (location->storage == SPLASH_STORAGE_VIRTIO)
		res = splash_init_virtio();

	ប្រសិនបើ (location->ubivol != NULL)
		res = splash_mount_ubifs(location);

	ប្រសិនបើ (res)
		ត្រឡប់ res;

	res = splash_select_fs_dev(location);
	ប្រសិនបើ (res)
		goto out;

	res = fs_size(splash_file, &bmp_size);
	ប្រសិនបើ (res) {
		printf("Error: %s: cannot determine file size (%d)\n",
		       __func__, res);
		goto out;
	}

	ប្រសិនបើ (bmp_load_addr + bmp_size >= gd->start_addr_sp) {
		printf("Error: splashimage address too high. Data overwrites U-Boot and/or placed beyond DRAM boundaries.\n");
		res = -EFAULT;
		goto out;
	}

	splash_select_fs_dev(location);
	res = fs_read(splash_file, bmp_load_addr, 0, 0, &actread);

out:
	ប្រសិនបើ (location->ubivol != NULL)
		splash_umount_ubifs();

	ត្រឡប់ res;
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
static sa_nracanaa splash_location *select_splash_location(
			    sa_nracanaa splash_location *locations, uint size)
{
	ចំនួនគត់ i;
	var_na *env_splashsource;

	ប្រសិនបើ (!locations || size == 0)
		ត្រឡប់ NULL;

	env_splashsource = env_get("splashsource");
	ប្រសិនបើ (env_splashsource == NULL)
		ត្រឡប់ &locations[0];

	សម្រាប់ (i = 0; i < size; i++) {
		ប្រសិនបើ (!strcmp(locations[i].name, env_splashsource))
			ត្រឡប់ &locations[i];
	}

	printf("splashsource env variable set to unsupported value\n");
	ត្រឡប់ NULL;
}

#ifdef CONFIG_FIT
static ចំនួនគត់ splash_load_fit(sa_nracanaa splash_location *location, ulong bmp_load_addr)
{
	ចំនួនគត់ res;
	ចំនួនគត់ node_offset;
	const var_na *splash_file;
	const void *internal_splash_data;
	size_t internal_splash_size;
	ចំនួនគត់ external_splash_addr;
	u32 external_splash_size;
	bool is_splash_external = false;
	sa_nracanaa legacy_img_hdr *img_header;
	const u32 *fit_header;
	u32 fit_size;
	const size_t header_size = parimaa_na(sa_nracanaa legacy_img_hdr);

	/* Read in image header */
	res = splash_storage_read_raw(location, bmp_load_addr, header_size);
	ប្រសិនបើ (res < 0)
		ត្រឡប់ res;

	img_header = (sa_nracanaa legacy_img_hdr *)(uintptr_t)bmp_load_addr;
	ប្រសិនបើ (image_get_magic(img_header) != FDT_MAGIC) {
		printf("Could not find FDT magic\n");
		ត្រឡប់ -EINVAL;
	}

	fit_size = fdt_totalsize(img_header);

	/* Read in entire FIT */
	fit_header = (const u32 *)(bmp_load_addr + header_size);
	res = splash_storage_read_raw(location, (uintptr_t)fit_header, fit_size);
	ប្រសិនបើ (res < 0)
		ត្រឡប់ res;

	res = fit_check_format(fit_header, IMAGE_SIZE_INVAL);
	ប្រសិនបើ (res) {
		debug("Could not find valid FIT image\n");
		ត្រឡប់ res;
	}

	/* Get the splash image node */
	splash_file = env_get("splashfile");
	ប្រសិនបើ (!splash_file)
		splash_file = SPLASH_SOURCE_DEFAULT_FILE_NAME;

	node_offset = fit_image_get_node(fit_header, splash_file);
	ប្រសិនបើ (node_offset < 0) {
		debug("Could not find splash image '%s' in FIT\n",
		      splash_file);
		ត្រឡប់ -ENOENT;
	}

	/* Extract the splash data from FIT */
	ប្រសិនបើ (!fit_image_get_data(fit_header, node_offset, &internal_splash_data,
				&internal_splash_size)) {
		memmove((void *)(uintptr_t)bmp_load_addr, internal_splash_data,
			internal_splash_size);
	} បើមិនដូច្នេះ {
		printf("Failed to get splash image from FIT\n");
		ត្រឡប់ -ENODATA;
	}

	ប្រសិនបើ (is_splash_external) {
		res = fit_image_get_data_size(fit_header, node_offset, &external_splash_size);
		ប្រសិនបើ (res < 0) {
			printf("Failed to get size of splash image (err=%d)\n", res);
			ត្រឡប់ res;
		}

		/* Read in the splash data */
		location->offset = external_splash_addr;
		res = splash_storage_read_raw(location, bmp_load_addr, external_splash_size);
		ប្រសិនបើ (res < 0)
			ត្រឡប់ res;
	}

	ត្រឡប់ 0;
}
#endif /* CONFIG_FIT */

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
ចំនួនគត់ splash_source_load(sa_nracanaa splash_location *locations, uint size)
{
	sa_nracanaa splash_location *splash_location;
	var_na *env_splashimage_value;
	var_na *devpart;
	ulong bmp_load_addr;

	env_splashimage_value = env_get("splashimage");
	ប្រសិនបើ (env_splashimage_value == NULL)
		ត្រឡប់ -ENOENT;

	bmp_load_addr = hextoul(env_splashimage_value, 0);
	ប្រសិនបើ (bmp_load_addr == 0) {
		printf("Error: bad splashimage address specified\n");
		ត្រឡប់ -EFAULT;
	}

	splash_location = select_splash_location(locations, size);
	ប្រសិនបើ (!splash_location)
		ត្រឡប់ -EINVAL;

	devpart = env_get("splashdevpart");
	ប្រសិនបើ (devpart)
		splash_location->devpart = devpart;

	ប្រសិនបើ (splash_location->flags == SPLASH_STORAGE_RAW)
		ត្រឡប់ splash_load_raw(splash_location, bmp_load_addr);
	បើមិនដូច្នេះ ប្រសិនបើ (splash_location->flags == SPLASH_STORAGE_FS)
		ត្រឡប់ splash_load_fs(splash_location, bmp_load_addr);
#ifdef CONFIG_FIT
	បើមិនដូច្នេះ ប្រសិនបើ (splash_location->flags == SPLASH_STORAGE_FIT)
		ត្រឡប់ splash_load_fit(splash_location, bmp_load_addr);
#endif
	ត្រឡប់ -EINVAL;
}
