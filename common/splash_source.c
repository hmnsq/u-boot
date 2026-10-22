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
static struct spi_flash *sf;
static 정수 splash_sf_read_raw(ulong bmp_load_addr, 정수 offset, size_t read_size)
{
	만약 (!sf) {
		sf = spi_flash_probe(CONFIG_SF_DEFAULT_BUS,
				     CONFIG_SF_DEFAULT_CS,
				     CONFIG_SF_DEFAULT_SPEED,
				     CONFIG_SF_DEFAULT_MODE);
		만약 (!sf)
			반환 -ENODEV;
	}

	반환 spi_flash_read(sf, offset, read_size, (void *)(uintptr_t)bmp_load_addr);
}
#아니면
static 정수 splash_sf_read_raw(ulong bmp_load_addr, 정수 offset, size_t read_size)
{
	debug("%s: sf support not available\n", __func__);
	반환 -ENOSYS;
}
#endif

#ifdef CONFIG_CMD_NAND
static 정수 splash_nand_read_raw(ulong bmp_load_addr, 정수 offset, size_t read_size)
{
	struct mtd_info *mtd = get_nand_dev_by_index(nand_curr_device);
	반환 nand_read_skip_bad(mtd, offset,
				  &read_size, NULL,
				  mtd->size,
				  (u_char *)bmp_load_addr);
}
#아니면
static 정수 splash_nand_read_raw(ulong bmp_load_addr, 정수 offset, size_t read_size)
{
	debug("%s: nand support not available\n", __func__);
	반환 -ENOSYS;
}
#endif

static 정수 splash_mmc_read_raw(ulong bmp_load_addr, struct splash_location *location,
			       size_t read_size)
{
	struct disk_partition partition;
	struct blk_desc *desc;
	lbaint_t blkcnt;
	정수 ret, n;

	만약 (!IS_ENABLED(CONFIG_CMD_MMC)) {
		debug("%s: mmc support not available\n", __func__);
		반환 -ENOSYS;
	}

	ret = part_get_info_by_dev_and_name_or_num("mmc", location->devpart, &desc,
						   &partition, 1);
	만약 (ret < 0)
		반환 ret;

	blkcnt = DIV_ROUND_UP(read_size, partition.blksz);
	n = blk_dread(desc, partition.start, blkcnt, (void *)(uintptr_t)bmp_load_addr);

	반환 (n == blkcnt) ? 0 : -EIO;
}

static 정수 splash_storage_read_raw(struct splash_location *location,
			       ulong bmp_load_addr, size_t read_size)
{
	u32 offset;

	만약 (!location)
		반환 -EINVAL;

	offset = location->offset;
	switch (location->storage) {
	case SPLASH_STORAGE_MMC:
		반환 splash_mmc_read_raw(bmp_load_addr, location, read_size);
	case SPLASH_STORAGE_NAND:
		반환 splash_nand_read_raw(bmp_load_addr, offset, read_size);
	case SPLASH_STORAGE_SF:
		반환 splash_sf_read_raw(bmp_load_addr, offset, read_size);
	default:
		printf("Unknown splash location\n");
	}

	반환 -EINVAL;
}

static 정수 splash_load_raw(struct splash_location *location, ulong bmp_load_addr)
{
	struct bmp_header *bmp_hdr;
	정수 res;
	size_t bmp_size, bmp_header_size = sizeof(struct bmp_header);

	만약 (bmp_load_addr + bmp_header_size >= gd->start_addr_sp)
		goto splash_address_too_high;

	res = splash_storage_read_raw(location, bmp_load_addr, bmp_header_size);
	만약 (res < 0)
		반환 res;

	bmp_hdr = (struct bmp_header *)(uintptr_t)bmp_load_addr;
	bmp_size = le32_to_cpu(bmp_hdr->file_size);

	만약 (bmp_load_addr + bmp_size >= gd->start_addr_sp)
		goto splash_address_too_high;

	반환 splash_storage_read_raw(location, bmp_load_addr, bmp_size);

splash_address_too_high:
	printf("Error: splashimage address too high. Data overwrites U-Boot and/or placed beyond DRAM boundaries.\n");

	반환 -EFAULT;
}

static 정수 splash_select_fs_dev(struct splash_location *location)
{
	정수 res;

	switch (location->storage) {
	case SPLASH_STORAGE_MMC:
		res = fs_set_blk_dev("mmc", location->devpart, FS_TYPE_ANY);
		break;
	case SPLASH_STORAGE_USB:
		res = fs_set_blk_dev("usb", location->devpart, FS_TYPE_ANY);
		break;
	case SPLASH_STORAGE_SATA:
		res = fs_set_blk_dev("sata", location->devpart, FS_TYPE_ANY);
		break;
	case SPLASH_STORAGE_NAND:
		만약 (location->ubivol != NULL)
			res = fs_set_blk_dev("ubi", NULL, FS_TYPE_UBIFS);
		아니면
			res = -ENODEV;
		break;
	default:
		printf("Error: %s: unsupported location storage.\n", __func__);
		반환 -ENODEV;
	}

	만약 (res)
		printf("Error: %s: could not access storage.\n", __func__);

	반환 res;
}

#ifdef CONFIG_USB_STORAGE
static 정수 splash_init_usb(void)
{
	정수 err;

	err = usb_init();
	만약 (err)
		반환 err;

#ifndef CONFIG_DM_USB
	err = usb_stor_scan(1) < 0 ? -ENODEV : 0;
#endif

	반환 err;
}
#아니면
static inline 정수 splash_init_usb(void)
{
	printf("Cannot load splash image: no USB support\n");
	반환 -ENOSYS;
}
#endif

#ifdef CONFIG_SATA
static 정수 splash_init_sata(void)
{
	반환 sata_probe(0);
}
#아니면
static inline 정수 splash_init_sata(void)
{
	printf("Cannot load splash image: no SATA support\n");
	반환 -ENOSYS;
}
#endif

static 정수 splash_init_virtio(void)
{
	만약 (!IS_ENABLED(CONFIG_VIRTIO)) {
		printf("Cannot load splash image: no virtio support\n");
		반환 -ENOSYS;
	} 아니면 {
		반환 virtio_init();
	}
}

#만약 defined(CONFIG_CMD_UBIFS) && !defined(CONFIG_XPL_BUILD)
static 정수 splash_mount_ubifs(struct splash_location *location)
{
	정수 res;
	char cmd[32];

	sprintf(cmd, "ubi part %s", location->mtdpart);
	res = run_command(cmd, 0);
	만약 (res)
		반환 res;

	sprintf(cmd, "ubifsmount %s", location->ubivol);
	res = run_command(cmd, 0);

	반환 res;
}

static inline 정수 splash_umount_ubifs(void)
{
	반환 run_command("ubifsumount", 0);
}
#아니면
static inline 정수 splash_mount_ubifs(struct splash_location *location)
{
	printf("Cannot load splash image: no UBIFS support\n");
	반환 -ENOSYS;
}

static inline 정수 splash_umount_ubifs(void)
{
	printf("Cannot unmount UBIFS: no UBIFS support\n");
	반환 -ENOSYS;
}
#endif

#define SPLASH_SOURCE_DEFAULT_FILE_NAME		"splash.bmp"

static 정수 splash_load_fs(struct splash_location *location, ulong bmp_load_addr)
{
	정수 res = 0;
	loff_t bmp_size;
	loff_t actread;
	char *splash_file;

	splash_file = env_get("splashfile");
	만약 (!splash_file)
		splash_file = SPLASH_SOURCE_DEFAULT_FILE_NAME;

	만약 (location->storage == SPLASH_STORAGE_USB)
		res = splash_init_usb();

	만약 (location->storage == SPLASH_STORAGE_SATA)
		res = splash_init_sata();

	만약 (location->storage == SPLASH_STORAGE_VIRTIO)
		res = splash_init_virtio();

	만약 (location->ubivol != NULL)
		res = splash_mount_ubifs(location);

	만약 (res)
		반환 res;

	res = splash_select_fs_dev(location);
	만약 (res)
		goto out;

	res = fs_size(splash_file, &bmp_size);
	만약 (res) {
		printf("Error: %s: cannot determine file size (%d)\n",
		       __func__, res);
		goto out;
	}

	만약 (bmp_load_addr + bmp_size >= gd->start_addr_sp) {
		printf("Error: splashimage address too high. Data overwrites U-Boot and/or placed beyond DRAM boundaries.\n");
		res = -EFAULT;
		goto out;
	}

	splash_select_fs_dev(location);
	res = fs_read(splash_file, bmp_load_addr, 0, 0, &actread);

out:
	만약 (location->ubivol != NULL)
		splash_umount_ubifs();

	반환 res;
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
static struct splash_location *select_splash_location(
			    struct splash_location *locations, uint size)
{
	정수 i;
	char *env_splashsource;

	만약 (!locations || size == 0)
		반환 NULL;

	env_splashsource = env_get("splashsource");
	만약 (env_splashsource == NULL)
		반환 &locations[0];

	위해 (i = 0; i < size; i++) {
		만약 (!strcmp(locations[i].name, env_splashsource))
			반환 &locations[i];
	}

	printf("splashsource env variable set to unsupported value\n");
	반환 NULL;
}

#ifdef CONFIG_FIT
static 정수 splash_load_fit(struct splash_location *location, ulong bmp_load_addr)
{
	정수 res;
	정수 node_offset;
	const char *splash_file;
	const void *internal_splash_data;
	size_t internal_splash_size;
	정수 external_splash_addr;
	u32 external_splash_size;
	bool is_splash_external = false;
	struct legacy_img_hdr *img_header;
	const u32 *fit_header;
	u32 fit_size;
	const size_t header_size = sizeof(struct legacy_img_hdr);

	/* Read in image header */
	res = splash_storage_read_raw(location, bmp_load_addr, header_size);
	만약 (res < 0)
		반환 res;

	img_header = (struct legacy_img_hdr *)(uintptr_t)bmp_load_addr;
	만약 (image_get_magic(img_header) != FDT_MAGIC) {
		printf("Could not find FDT magic\n");
		반환 -EINVAL;
	}

	fit_size = fdt_totalsize(img_header);

	/* Read in entire FIT */
	fit_header = (const u32 *)(bmp_load_addr + header_size);
	res = splash_storage_read_raw(location, (uintptr_t)fit_header, fit_size);
	만약 (res < 0)
		반환 res;

	res = fit_check_format(fit_header, IMAGE_SIZE_INVAL);
	만약 (res) {
		debug("Could not find valid FIT image\n");
		반환 res;
	}

	/* Get the splash image node */
	splash_file = env_get("splashfile");
	만약 (!splash_file)
		splash_file = SPLASH_SOURCE_DEFAULT_FILE_NAME;

	node_offset = fit_image_get_node(fit_header, splash_file);
	만약 (node_offset < 0) {
		debug("Could not find splash image '%s' in FIT\n",
		      splash_file);
		반환 -ENOENT;
	}

	/* Extract the splash data from FIT */
	만약 (!fit_image_get_data(fit_header, node_offset, &internal_splash_data,
				&internal_splash_size)) {
		memmove((void *)(uintptr_t)bmp_load_addr, internal_splash_data,
			internal_splash_size);
	} 아니면 {
		printf("Failed to get splash image from FIT\n");
		반환 -ENODATA;
	}

	만약 (is_splash_external) {
		res = fit_image_get_data_size(fit_header, node_offset, &external_splash_size);
		만약 (res < 0) {
			printf("Failed to get size of splash image (err=%d)\n", res);
			반환 res;
		}

		/* Read in the splash data */
		location->offset = external_splash_addr;
		res = splash_storage_read_raw(location, bmp_load_addr, external_splash_size);
		만약 (res < 0)
			반환 res;
	}

	반환 0;
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
정수 splash_source_load(struct splash_location *locations, uint size)
{
	struct splash_location *splash_location;
	char *env_splashimage_value;
	char *devpart;
	ulong bmp_load_addr;

	env_splashimage_value = env_get("splashimage");
	만약 (env_splashimage_value == NULL)
		반환 -ENOENT;

	bmp_load_addr = hextoul(env_splashimage_value, 0);
	만약 (bmp_load_addr == 0) {
		printf("Error: bad splashimage address specified\n");
		반환 -EFAULT;
	}

	splash_location = select_splash_location(locations, size);
	만약 (!splash_location)
		반환 -EINVAL;

	devpart = env_get("splashdevpart");
	만약 (devpart)
		splash_location->devpart = devpart;

	만약 (splash_location->flags == SPLASH_STORAGE_RAW)
		반환 splash_load_raw(splash_location, bmp_load_addr);
	아니면 만약 (splash_location->flags == SPLASH_STORAGE_FS)
		반환 splash_load_fs(splash_location, bmp_load_addr);
#ifdef CONFIG_FIT
	아니면 만약 (splash_location->flags == SPLASH_STORAGE_FIT)
		반환 splash_load_fit(splash_location, bmp_load_addr);
#endif
	반환 -EINVAL;
}
