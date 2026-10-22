// SPDX-License-Identifier: GPL-2.0+

#include <env.h>
#include <part.h>
#include <spl.h>
#include <spl_load.h>
#include <ext4fs.h>
#include <errno.h>
#include <image.h>

static ulong spl_fit_read(struct spl_load_info *load, ulong file_offset,
			  ulong size, void *buf)
{
	struct legacy_img_hdr *header;
	số_nguyên ret;
	loff_t actlen;

	ret = ext4fs_read(buf, file_offset, size, &actlen);
	nếu (ret)
		trả_về ret;

	nếu (CONFIG_IS_ENABLED(OS_BOOT)) {
		header = (struct legacy_img_hdr *)buf;
		nếu (image_get_magic(header) != FDT_MAGIC)
			trả_về size;
	}

	trả_về actlen;
}

số_nguyên spl_load_image_ext(struct spl_image_info *spl_image,
		       struct spl_boot_device *bootdev,
		       struct blk_desc *block_dev, số_nguyên partition,
		       const char *filename)
{
	s32 err;
	loff_t filelen;
	struct disk_partition part_info = {};
	struct spl_load_info load;

	nếu (part_get_info(block_dev, partition, &part_info)) {
		printf("spl: no partition table found\n");
		trả_về -1;
	}

	ext4fs_set_blk_dev(block_dev, &part_info);

	err = ext4fs_mount();
	nếu (!err) {
		printf("%s: ext4fs mount err - %d\n", __func__, err);
		trả_về -1;
	}

	err = ext4fs_open(filename, &filelen);
	nếu (err < 0) {
		puts("spl: ext4fs_open failed\n");
		goto end;
	}

	spl_load_init(&load, spl_fit_read, NULL, 1);
	err = spl_load(spl_image, bootdev, &load, filelen, 0);

end:
	nếu (err < 0)
		printf("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);

	trả_về err < 0;
}

#nếu CONFIG_IS_ENABLED(OS_BOOT)
số_nguyên spl_load_image_ext_os(struct spl_image_info *spl_image,
			  struct spl_boot_device *bootdev,
			  struct blk_desc *block_dev, số_nguyên partition)
{
	số_nguyên err;
	__maybe_unused loff_t filelen, actlen;
	struct disk_partition part_info = {};
	__maybe_unused char *file;

	nếu (part_get_info(block_dev, partition, &part_info)) {
		printf("spl: no partition table found\n");
		trả_về -1;
	}

	ext4fs_set_blk_dev(block_dev, &part_info);

	err = ext4fs_mount();
	nếu (!err) {
		printf("%s: ext4fs mount err - %d\n", __func__, err);
		trả_về -1;
	}

	nếu (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		goto defaults;

	file = env_get("falcon_image_file");
	nếu (file) {
		err = spl_load_image_ext(spl_image, bootdev, block_dev,
					 partition, file);
		nếu (err != 0) {
			puts("spl: falling back to default\n");
			goto defaults;
		}

#nếu IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
		ext4fs_set_blk_dev(block_dev, &part_info);
		ext4fs_mount();
		file = env_get("falcon_args_file");
		nếu (file) {
			err = ext4fs_open(file, &filelen);
			nếu (err < 0) {
				puts("spl: ext4fs_open failed\n");
				goto defaults;
			}
			err = ext4fs_read((void *)CONFIG_SPL_PAYLOAD_ARGS_ADDR,
					  0, filelen, &actlen);
			nếu (err < 0) {
				printf("spl: error reading args %s, err - %d, falling back to default\n",
				       file, err);
				goto defaults;
			}
			trả_về 0;
		} không_thì {
			puts("spl: falcon_args_file not set in environment, falling back to default\n");
		}
#endif
	} không_thì {
		puts("spl: falcon_image_file not set in environment, falling back to default\n");
	}

defaults:

	err = spl_load_image_ext(spl_image, bootdev, block_dev, partition,
				 CONFIG_SPL_FS_LOAD_KERNEL_NAME);

	nếu (err)
		trả_về err;

#nếu IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	ext4fs_set_blk_dev(block_dev, &part_info);
	ext4fs_mount();
	err = ext4fs_open(CONFIG_SPL_FS_LOAD_ARGS_NAME, &filelen);
	nếu (err < 0)
		puts("spl: ext4fs_open failed\n");

	err = ext4fs_read((void *)CONFIG_SPL_PAYLOAD_ARGS_ADDR, 0, filelen, &actlen);
	nếu (err < 0) {
		printf("%s: error reading image %s, err - %d\n",
		       __func__, CONFIG_SPL_FS_LOAD_ARGS_NAME, err);
		trả_về -1;
	}
#endif

	trả_về 0;
}
#không_thì
số_nguyên spl_load_image_ext_os(struct spl_image_info *spl_image,
			  struct spl_boot_device *bootdev,
			  struct blk_desc *block_dev, số_nguyên partition)
{
	trả_về -ENOSYS;
}
#endif
