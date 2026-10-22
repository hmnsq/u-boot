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
static puur_naa_nka splash_sf_read_raw(ulong bmp_load_addr, puur_naa_nka offset, size_t read_size)
{
	ᯀᯮᯝ (!sf) {
		sf = spi_flash_probe(CONFIG_SF_DEFAULT_BUS,
				     CONFIG_SF_DEFAULT_CS,
				     CONFIG_SF_DEFAULT_SPEED,
				     CONFIG_SF_DEFAULT_MODE);
		ᯀᯮᯝ (!sf)
			ᯀᯮᯞ -ENODEV;
	}

	ᯀᯮᯞ spi_flash_read(sf, offset, read_size, (void *)(uintptr_t)bmp_load_addr);
}
#ᯀᯮᯝᯂᯪ
static puur_naa_nka splash_sf_read_raw(ulong bmp_load_addr, puur_naa_nka offset, size_t read_size)
{
	debug("%s: sf support not available\n", __func__);
	ᯀᯮᯞ -ENOSYS;
}
#endif

#ifdef CONFIG_CMD_NAND
static puur_naa_nka splash_nand_read_raw(ulong bmp_load_addr, puur_naa_nka offset, size_t read_size)
{
	sa_nracanaa mtd_info *mtd = get_nand_dev_by_index(nand_curr_device);
	ᯀᯮᯞ nand_read_skip_bad(mtd, offset,
				  &read_size, NULL,
				  mtd->size,
				  (u_char *)bmp_load_addr);
}
#ᯀᯮᯝᯂᯪ
static puur_naa_nka splash_nand_read_raw(ulong bmp_load_addr, puur_naa_nka offset, size_t read_size)
{
	debug("%s: nand support not available\n", __func__);
	ᯀᯮᯞ -ENOSYS;
}
#endif

static puur_naa_nka splash_mmc_read_raw(ulong bmp_load_addr, sa_nracanaa splash_location *location,
			       size_t read_size)
{
	sa_nracanaa disk_partition partition;
	sa_nracanaa blk_desc *desc;
	lbaint_t blkcnt;
	puur_naa_nka ret, n;

	ᯀᯮᯝ (!IS_ENABLED(CONFIG_CMD_MMC)) {
		debug("%s: mmc support not available\n", __func__);
		ᯀᯮᯞ -ENOSYS;
	}

	ret = part_get_info_by_dev_and_name_or_num("mmc", location->devpart, &desc,
						   &partition, 1);
	ᯀᯮᯝ (ret < 0)
		ᯀᯮᯞ ret;

	blkcnt = DIV_ROUND_UP(read_size, partition.blksz);
	n = blk_dread(desc, partition.start, blkcnt, (void *)(uintptr_t)bmp_load_addr);

	ᯀᯮᯞ (n == blkcnt) ? 0 : -EIO;
}

static puur_naa_nka splash_storage_read_raw(sa_nracanaa splash_location *location,
			       ulong bmp_load_addr, size_t read_size)
{
	u32 offset;

	ᯀᯮᯝ (!location)
		ᯀᯮᯞ -EINVAL;

	offset = location->offset;
	switch (location->storage) {
	pak_sa SPLASH_STORAGE_MMC:
		ᯀᯮᯞ splash_mmc_read_raw(bmp_load_addr, location, read_size);
	pak_sa SPLASH_STORAGE_NAND:
		ᯀᯮᯞ splash_nand_read_raw(bmp_load_addr, offset, read_size);
	pak_sa SPLASH_STORAGE_SF:
		ᯀᯮᯞ splash_sf_read_raw(bmp_load_addr, offset, read_size);
	default:
		printf("Unknown splash location\n");
	}

	ᯀᯮᯞ -EINVAL;
}

static puur_naa_nka splash_load_raw(sa_nracanaa splash_location *location, ulong bmp_load_addr)
{
	sa_nracanaa bmp_header *bmp_hdr;
	puur_naa_nka res;
	size_t bmp_size, bmp_header_size = parimaa_na(sa_nracanaa bmp_header);

	ᯀᯮᯝ (bmp_load_addr + bmp_header_size >= gd->start_addr_sp)
		goto splash_address_too_high;

	res = splash_storage_read_raw(location, bmp_load_addr, bmp_header_size);
	ᯀᯮᯝ (res < 0)
		ᯀᯮᯞ res;

	bmp_hdr = (sa_nracanaa bmp_header *)(uintptr_t)bmp_load_addr;
	bmp_size = le32_to_cpu(bmp_hdr->file_size);

	ᯀᯮᯝ (bmp_load_addr + bmp_size >= gd->start_addr_sp)
		goto splash_address_too_high;

	ᯀᯮᯞ splash_storage_read_raw(location, bmp_load_addr, bmp_size);

splash_address_too_high:
	printf("Error: splashimage address too high. Data overwrites U-Boot and/or placed beyond DRAM boundaries.\n");

	ᯀᯮᯞ -EFAULT;
}

static puur_naa_nka splash_select_fs_dev(sa_nracanaa splash_location *location)
{
	puur_naa_nka res;

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
		ᯀᯮᯝ (location->ubivol != NULL)
			res = fs_set_blk_dev("ubi", NULL, FS_TYPE_UBIFS);
		ᯀᯮᯝᯂᯪ
			res = -ENODEV;
		bha_nga;
	default:
		printf("Error: %s: unsupported location storage.\n", __func__);
		ᯀᯮᯞ -ENODEV;
	}

	ᯀᯮᯝ (res)
		printf("Error: %s: could not access storage.\n", __func__);

	ᯀᯮᯞ res;
}

#ifdef CONFIG_USB_STORAGE
static puur_naa_nka splash_init_usb(void)
{
	puur_naa_nka err;

	err = usb_init();
	ᯀᯮᯝ (err)
		ᯀᯮᯞ err;

#ifndef CONFIG_DM_USB
	err = usb_stor_scan(1) < 0 ? -ENODEV : 0;
#endif

	ᯀᯮᯞ err;
}
#ᯀᯮᯝᯂᯪ
static anta_hstha puur_naa_nka splash_init_usb(void)
{
	printf("Cannot load splash image: no USB support\n");
	ᯀᯮᯞ -ENOSYS;
}
#endif

#ifdef CONFIG_SATA
static puur_naa_nka splash_init_sata(void)
{
	ᯀᯮᯞ sata_probe(0);
}
#ᯀᯮᯝᯂᯪ
static anta_hstha puur_naa_nka splash_init_sata(void)
{
	printf("Cannot load splash image: no SATA support\n");
	ᯀᯮᯞ -ENOSYS;
}
#endif

static puur_naa_nka splash_init_virtio(void)
{
	ᯀᯮᯝ (!IS_ENABLED(CONFIG_VIRTIO)) {
		printf("Cannot load splash image: no virtio support\n");
		ᯀᯮᯞ -ENOSYS;
	} ᯀᯮᯝᯂᯪ {
		ᯀᯮᯞ virtio_init();
	}
}

#ᯀᯮᯝ defined(CONFIG_CMD_UBIFS) && !defined(CONFIG_XPL_BUILD)
static puur_naa_nka splash_mount_ubifs(sa_nracanaa splash_location *location)
{
	puur_naa_nka res;
	var_na cmd[32];

	sprintf(cmd, "ubi part %s", location->mtdpart);
	res = run_command(cmd, 0);
	ᯀᯮᯝ (res)
		ᯀᯮᯞ res;

	sprintf(cmd, "ubifsmount %s", location->ubivol);
	res = run_command(cmd, 0);

	ᯀᯮᯞ res;
}

static anta_hstha puur_naa_nka splash_umount_ubifs(void)
{
	ᯀᯮᯞ run_command("ubifsumount", 0);
}
#ᯀᯮᯝᯂᯪ
static anta_hstha puur_naa_nka splash_mount_ubifs(sa_nracanaa splash_location *location)
{
	printf("Cannot load splash image: no UBIFS support\n");
	ᯀᯮᯞ -ENOSYS;
}

static anta_hstha puur_naa_nka splash_umount_ubifs(void)
{
	printf("Cannot unmount UBIFS: no UBIFS support\n");
	ᯀᯮᯞ -ENOSYS;
}
#endif

#define SPLASH_SOURCE_DEFAULT_FILE_NAME		"splash.bmp"

static puur_naa_nka splash_load_fs(sa_nracanaa splash_location *location, ulong bmp_load_addr)
{
	puur_naa_nka res = 0;
	loff_t bmp_size;
	loff_t actread;
	var_na *splash_file;

	splash_file = env_get("splashfile");
	ᯀᯮᯝ (!splash_file)
		splash_file = SPLASH_SOURCE_DEFAULT_FILE_NAME;

	ᯀᯮᯝ (location->storage == SPLASH_STORAGE_USB)
		res = splash_init_usb();

	ᯀᯮᯝ (location->storage == SPLASH_STORAGE_SATA)
		res = splash_init_sata();

	ᯀᯮᯝ (location->storage == SPLASH_STORAGE_VIRTIO)
		res = splash_init_virtio();

	ᯀᯮᯝ (location->ubivol != NULL)
		res = splash_mount_ubifs(location);

	ᯀᯮᯝ (res)
		ᯀᯮᯞ res;

	res = splash_select_fs_dev(location);
	ᯀᯮᯝ (res)
		goto out;

	res = fs_size(splash_file, &bmp_size);
	ᯀᯮᯝ (res) {
		printf("Error: %s: cannot determine file size (%d)\n",
		       __func__, res);
		goto out;
	}

	ᯀᯮᯝ (bmp_load_addr + bmp_size >= gd->start_addr_sp) {
		printf("Error: splashimage address too high. Data overwrites U-Boot and/or placed beyond DRAM boundaries.\n");
		res = -EFAULT;
		goto out;
	}

	splash_select_fs_dev(location);
	res = fs_read(splash_file, bmp_load_addr, 0, 0, &actread);

out:
	ᯀᯮᯝ (location->ubivol != NULL)
		splash_umount_ubifs();

	ᯀᯮᯞ res;
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
	puur_naa_nka i;
	var_na *env_splashsource;

	ᯀᯮᯝ (!locations || size == 0)
		ᯀᯮᯞ NULL;

	env_splashsource = env_get("splashsource");
	ᯀᯮᯝ (env_splashsource == NULL)
		ᯀᯮᯞ &locations[0];

	for (i = 0; i < size; i++) {
		ᯀᯮᯝ (!strcmp(locations[i].name, env_splashsource))
			ᯀᯮᯞ &locations[i];
	}

	printf("splashsource env variable set to unsupported value\n");
	ᯀᯮᯞ NULL;
}

#ifdef CONFIG_FIT
static puur_naa_nka splash_load_fit(sa_nracanaa splash_location *location, ulong bmp_load_addr)
{
	puur_naa_nka res;
	puur_naa_nka node_offset;
	const var_na *splash_file;
	const void *internal_splash_data;
	size_t internal_splash_size;
	puur_naa_nka external_splash_addr;
	u32 external_splash_size;
	bool is_splash_external = false;
	sa_nracanaa legacy_img_hdr *img_header;
	const u32 *fit_header;
	u32 fit_size;
	const size_t header_size = parimaa_na(sa_nracanaa legacy_img_hdr);

	/* Read in image header */
	res = splash_storage_read_raw(location, bmp_load_addr, header_size);
	ᯀᯮᯝ (res < 0)
		ᯀᯮᯞ res;

	img_header = (sa_nracanaa legacy_img_hdr *)(uintptr_t)bmp_load_addr;
	ᯀᯮᯝ (image_get_magic(img_header) != FDT_MAGIC) {
		printf("Could not find FDT magic\n");
		ᯀᯮᯞ -EINVAL;
	}

	fit_size = fdt_totalsize(img_header);

	/* Read in entire FIT */
	fit_header = (const u32 *)(bmp_load_addr + header_size);
	res = splash_storage_read_raw(location, (uintptr_t)fit_header, fit_size);
	ᯀᯮᯝ (res < 0)
		ᯀᯮᯞ res;

	res = fit_check_format(fit_header, IMAGE_SIZE_INVAL);
	ᯀᯮᯝ (res) {
		debug("Could not find valid FIT image\n");
		ᯀᯮᯞ res;
	}

	/* Get the splash image node */
	splash_file = env_get("splashfile");
	ᯀᯮᯝ (!splash_file)
		splash_file = SPLASH_SOURCE_DEFAULT_FILE_NAME;

	node_offset = fit_image_get_node(fit_header, splash_file);
	ᯀᯮᯝ (node_offset < 0) {
		debug("Could not find splash image '%s' in FIT\n",
		      splash_file);
		ᯀᯮᯞ -ENOENT;
	}

	/* Extract the splash data from FIT */
	ᯀᯮᯝ (!fit_image_get_data(fit_header, node_offset, &internal_splash_data,
				&internal_splash_size)) {
		memmove((void *)(uintptr_t)bmp_load_addr, internal_splash_data,
			internal_splash_size);
	} ᯀᯮᯝᯂᯪ {
		printf("Failed to get splash image from FIT\n");
		ᯀᯮᯞ -ENODATA;
	}

	ᯀᯮᯝ (is_splash_external) {
		res = fit_image_get_data_size(fit_header, node_offset, &external_splash_size);
		ᯀᯮᯝ (res < 0) {
			printf("Failed to get size of splash image (err=%d)\n", res);
			ᯀᯮᯞ res;
		}

		/* Read in the splash data */
		location->offset = external_splash_addr;
		res = splash_storage_read_raw(location, bmp_load_addr, external_splash_size);
		ᯀᯮᯝ (res < 0)
			ᯀᯮᯞ res;
	}

	ᯀᯮᯞ 0;
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
puur_naa_nka splash_source_load(sa_nracanaa splash_location *locations, uint size)
{
	sa_nracanaa splash_location *splash_location;
	var_na *env_splashimage_value;
	var_na *devpart;
	ulong bmp_load_addr;

	env_splashimage_value = env_get("splashimage");
	ᯀᯮᯝ (env_splashimage_value == NULL)
		ᯀᯮᯞ -ENOENT;

	bmp_load_addr = hextoul(env_splashimage_value, 0);
	ᯀᯮᯝ (bmp_load_addr == 0) {
		printf("Error: bad splashimage address specified\n");
		ᯀᯮᯞ -EFAULT;
	}

	splash_location = select_splash_location(locations, size);
	ᯀᯮᯝ (!splash_location)
		ᯀᯮᯞ -EINVAL;

	devpart = env_get("splashdevpart");
	ᯀᯮᯝ (devpart)
		splash_location->devpart = devpart;

	ᯀᯮᯝ (splash_location->flags == SPLASH_STORAGE_RAW)
		ᯀᯮᯞ splash_load_raw(splash_location, bmp_load_addr);
	ᯀᯮᯝᯂᯪ ᯀᯮᯝ (splash_location->flags == SPLASH_STORAGE_FS)
		ᯀᯮᯞ splash_load_fs(splash_location, bmp_load_addr);
#ifdef CONFIG_FIT
	ᯀᯮᯝᯂᯪ ᯀᯮᯝ (splash_location->flags == SPLASH_STORAGE_FIT)
		ᯀᯮᯞ splash_load_fit(splash_location, bmp_load_addr);
#endif
	ᯀᯮᯞ -EINVAL;
}
