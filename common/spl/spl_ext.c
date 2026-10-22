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
	puur_naa_nka ret;
	loff_t actlen;

	ret = ext4fs_read(buf, file_offset, size, &actlen);
	ᮚᮦᮔ᮪ (ret)
		ᮝᮜᮤ ret;

	ᮚᮦᮔ᮪ (CONFIG_IS_ENABLED(OS_BOOT)) {
		header = (sa_nracanaa legacy_img_hdr *)buf;
		ᮚᮦᮔ᮪ (image_get_magic(header) != FDT_MAGIC)
			ᮝᮜᮤ size;
	}

	ᮝᮜᮤ actlen;
}

puur_naa_nka spl_load_image_ext(sa_nracanaa spl_image_info *spl_image,
		       sa_nracanaa spl_boot_device *bootdev,
		       sa_nracanaa blk_desc *block_dev, puur_naa_nka partition,
		       const var_na *filename)
{
	s32 err;
	loff_t filelen;
	sa_nracanaa disk_partition part_info = {};
	sa_nracanaa spl_load_info load;

	ᮚᮦᮔ᮪ (part_get_info(block_dev, partition, &part_info)) {
		printf("spl: no partition table found\n");
		ᮝᮜᮤ -1;
	}

	ext4fs_set_blk_dev(block_dev, &part_info);

	err = ext4fs_mount();
	ᮚᮦᮔ᮪ (!err) {
		printf("%s: ext4fs mount err - %d\n", __func__, err);
		ᮝᮜᮤ -1;
	}

	err = ext4fs_open(filename, &filelen);
	ᮚᮦᮔ᮪ (err < 0) {
		puts("spl: ext4fs_open failed\n");
		goto end;
	}

	spl_load_init(&load, spl_fit_read, NULL, 1);
	err = spl_load(spl_image, bootdev, &load, filelen, 0);

end:
	ᮚᮦᮔ᮪ (err < 0)
		printf("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);

	ᮝᮜᮤ err < 0;
}

#ᮚᮦᮔ᮪ CONFIG_IS_ENABLED(OS_BOOT)
puur_naa_nka spl_load_image_ext_os(sa_nracanaa spl_image_info *spl_image,
			  sa_nracanaa spl_boot_device *bootdev,
			  sa_nracanaa blk_desc *block_dev, puur_naa_nka partition)
{
	puur_naa_nka err;
	__maybe_unused loff_t filelen, actlen;
	sa_nracanaa disk_partition part_info = {};
	__maybe_unused var_na *file;

	ᮚᮦᮔ᮪ (part_get_info(block_dev, partition, &part_info)) {
		printf("spl: no partition table found\n");
		ᮝᮜᮤ -1;
	}

	ext4fs_set_blk_dev(block_dev, &part_info);

	err = ext4fs_mount();
	ᮚᮦᮔ᮪ (!err) {
		printf("%s: ext4fs mount err - %d\n", __func__, err);
		ᮝᮜᮤ -1;
	}

	ᮚᮦᮔ᮪ (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		goto defaults;

	file = env_get("falcon_image_file");
	ᮚᮦᮔ᮪ (file) {
		err = spl_load_image_ext(spl_image, bootdev, block_dev,
					 partition, file);
		ᮚᮦᮔ᮪ (err != 0) {
			puts("spl: falling back to default\n");
			goto defaults;
		}

#ᮚᮦᮔ᮪ IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
		ext4fs_set_blk_dev(block_dev, &part_info);
		ext4fs_mount();
		file = env_get("falcon_args_file");
		ᮚᮦᮔ᮪ (file) {
			err = ext4fs_open(file, &filelen);
			ᮚᮦᮔ᮪ (err < 0) {
				puts("spl: ext4fs_open failed\n");
				goto defaults;
			}
			err = ext4fs_read((void *)CONFIG_SPL_PAYLOAD_ARGS_ADDR,
					  0, filelen, &actlen);
			ᮚᮦᮔ᮪ (err < 0) {
				printf("spl: error reading args %s, err - %d, falling back to default\n",
				       file, err);
				goto defaults;
			}
			ᮝᮜᮤ 0;
		} ᮜᮤᮚ {
			puts("spl: falcon_args_file not set in environment, falling back to default\n");
		}
#endif
	} ᮜᮤᮚ {
		puts("spl: falcon_image_file not set in environment, falling back to default\n");
	}

defaults:

	err = spl_load_image_ext(spl_image, bootdev, block_dev, partition,
				 CONFIG_SPL_FS_LOAD_KERNEL_NAME);

	ᮚᮦᮔ᮪ (err)
		ᮝᮜᮤ err;

#ᮚᮦᮔ᮪ IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	ext4fs_set_blk_dev(block_dev, &part_info);
	ext4fs_mount();
	err = ext4fs_open(CONFIG_SPL_FS_LOAD_ARGS_NAME, &filelen);
	ᮚᮦᮔ᮪ (err < 0)
		puts("spl: ext4fs_open failed\n");

	err = ext4fs_read((void *)CONFIG_SPL_PAYLOAD_ARGS_ADDR, 0, filelen, &actlen);
	ᮚᮦᮔ᮪ (err < 0) {
		printf("%s: error reading image %s, err - %d\n",
		       __func__, CONFIG_SPL_FS_LOAD_ARGS_NAME, err);
		ᮝᮜᮤ -1;
	}
#endif

	ᮝᮜᮤ 0;
}
#ᮜᮤᮚ
puur_naa_nka spl_load_image_ext_os(sa_nracanaa spl_image_info *spl_image,
			  sa_nracanaa spl_boot_device *bootdev,
			  sa_nracanaa blk_desc *block_dev, puur_naa_nka partition)
{
	ᮝᮜᮤ -ENOSYS;
}
#endif
