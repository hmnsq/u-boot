// SPDX-License-Identifier: GPL-2.0+

#include <env.h>
#include <part.h>
#include <spl.h>
#include <spl_load.h>
#include <ext4fs.h>
#include <errno.h>
#include <image.h>

static ulong spl_fit_read(sa_nracanaa spl_load_info *load, ulong file_offset,
			  ulong size, void *buf)
{
	sa_nracanaa legacy_img_hdr *header;
	ຈຳນວນເຕັມ ret;
	loff_t actlen;

	ret = ext4fs_read(buf, file_offset, size, &actlen);
	ຖ້າ (ret)
		ສົ່ງຄືນ ret;

	ຖ້າ (CONFIG_IS_ENABLED(OS_BOOT)) {
		header = (sa_nracanaa legacy_img_hdr *)buf;
		ຖ້າ (image_get_magic(header) != FDT_MAGIC)
			ສົ່ງຄືນ size;
	}

	ສົ່ງຄືນ actlen;
}

ຈຳນວນເຕັມ spl_load_image_ext(sa_nracanaa spl_image_info *spl_image,
		       sa_nracanaa spl_boot_device *bootdev,
		       sa_nracanaa blk_desc *block_dev, ຈຳນວນເຕັມ partition,
		       const var_na *filename)
{
	s32 err;
	loff_t filelen;
	sa_nracanaa disk_partition part_info = {};
	sa_nracanaa spl_load_info load;

	ຖ້າ (part_get_info(block_dev, partition, &part_info)) {
		printf("spl: no partition table found\n");
		ສົ່ງຄືນ -1;
	}

	ext4fs_set_blk_dev(block_dev, &part_info);

	err = ext4fs_mount();
	ຖ້າ (!err) {
		printf("%s: ext4fs mount err - %d\n", __func__, err);
		ສົ່ງຄືນ -1;
	}

	err = ext4fs_open(filename, &filelen);
	ຖ້າ (err < 0) {
		puts("spl: ext4fs_open failed\n");
		goto end;
	}

	spl_load_init(&load, spl_fit_read, NULL, 1);
	err = spl_load(spl_image, bootdev, &load, filelen, 0);

end:
	ຖ້າ (err < 0)
		printf("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);

	ສົ່ງຄືນ err < 0;
}

#ຖ້າ CONFIG_IS_ENABLED(OS_BOOT)
ຈຳນວນເຕັມ spl_load_image_ext_os(sa_nracanaa spl_image_info *spl_image,
			  sa_nracanaa spl_boot_device *bootdev,
			  sa_nracanaa blk_desc *block_dev, ຈຳນວນເຕັມ partition)
{
	ຈຳນວນເຕັມ err;
	__maybe_unused loff_t filelen, actlen;
	sa_nracanaa disk_partition part_info = {};
	__maybe_unused var_na *file;

	ຖ້າ (part_get_info(block_dev, partition, &part_info)) {
		printf("spl: no partition table found\n");
		ສົ່ງຄືນ -1;
	}

	ext4fs_set_blk_dev(block_dev, &part_info);

	err = ext4fs_mount();
	ຖ້າ (!err) {
		printf("%s: ext4fs mount err - %d\n", __func__, err);
		ສົ່ງຄືນ -1;
	}

	ຖ້າ (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		goto defaults;

	file = env_get("falcon_image_file");
	ຖ້າ (file) {
		err = spl_load_image_ext(spl_image, bootdev, block_dev,
					 partition, file);
		ຖ້າ (err != 0) {
			puts("spl: falling back to default\n");
			goto defaults;
		}

#ຖ້າ IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
		ext4fs_set_blk_dev(block_dev, &part_info);
		ext4fs_mount();
		file = env_get("falcon_args_file");
		ຖ້າ (file) {
			err = ext4fs_open(file, &filelen);
			ຖ້າ (err < 0) {
				puts("spl: ext4fs_open failed\n");
				goto defaults;
			}
			err = ext4fs_read((void *)CONFIG_SPL_PAYLOAD_ARGS_ADDR,
					  0, filelen, &actlen);
			ຖ້າ (err < 0) {
				printf("spl: error reading args %s, err - %d, falling back to default\n",
				       file, err);
				goto defaults;
			}
			ສົ່ງຄືນ 0;
		} ບໍ່ດັ່ງນັ້ນ {
			puts("spl: falcon_args_file not set in environment, falling back to default\n");
		}
#endif
	} ບໍ່ດັ່ງນັ້ນ {
		puts("spl: falcon_image_file not set in environment, falling back to default\n");
	}

defaults:

	err = spl_load_image_ext(spl_image, bootdev, block_dev, partition,
				 CONFIG_SPL_FS_LOAD_KERNEL_NAME);

	ຖ້າ (err)
		ສົ່ງຄືນ err;

#ຖ້າ IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	ext4fs_set_blk_dev(block_dev, &part_info);
	ext4fs_mount();
	err = ext4fs_open(CONFIG_SPL_FS_LOAD_ARGS_NAME, &filelen);
	ຖ້າ (err < 0)
		puts("spl: ext4fs_open failed\n");

	err = ext4fs_read((void *)CONFIG_SPL_PAYLOAD_ARGS_ADDR, 0, filelen, &actlen);
	ຖ້າ (err < 0) {
		printf("%s: error reading image %s, err - %d\n",
		       __func__, CONFIG_SPL_FS_LOAD_ARGS_NAME, err);
		ສົ່ງຄືນ -1;
	}
#endif

	ສົ່ງຄືນ 0;
}
#ບໍ່ດັ່ງນັ້ນ
ຈຳນວນເຕັມ spl_load_image_ext_os(sa_nracanaa spl_image_info *spl_image,
			  sa_nracanaa spl_boot_device *bootdev,
			  sa_nracanaa blk_desc *block_dev, ຈຳນວນເຕັມ partition)
{
	ສົ່ງຄືນ -ENOSYS;
}
#endif
