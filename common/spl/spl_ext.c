// SPDX-License-Identifier: GPL-2.0+

#અન્તર્ભાવ <env.h>
#અન્તર્ભાવ <part.h>
#અન્તર્ભાવ <spl.h>
#અન્તર્ભાવ <spl_load.h>
#અન્તર્ભાવ <ext4fs.h>
#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <image.h>

સ્થિર ulong spl_fit_read(સંરચના spl_load_info *load, ulong file_offset,
			  ulong size, શૂન્ય *buf)
{
	સંરચના legacy_img_hdr *header;
	પૂર્ણાંક ret;
	loff_t actlen;

	ret = ext4fs_read(buf, file_offset, size, &actlen);
	જો (ret)
		પાછા ret;

	જો (CONFIG_IS_ENABLED(OS_BOOT)) {
		header = (સંરચના legacy_img_hdr *)buf;
		જો (image_get_magic(header) != FDT_MAGIC)
			પાછા size;
	}

	પાછા actlen;
}

પૂર્ણાંક spl_load_image_ext(સંરચના spl_image_info *spl_image,
		       સંરચના spl_boot_device *bootdev,
		       સંરચના blk_desc *block_dev, પૂર્ણાંક partition,
		       અચલ અક્ષર *filename)
{
	s32 err;
	loff_t filelen;
	સંરચના disk_partition part_info = {};
	સંરચના spl_load_info load;

	જો (part_get_info(block_dev, partition, &part_info)) {
		printf("spl: no partition table found\n");
		પાછા -1;
	}

	ext4fs_set_blk_dev(block_dev, &part_info);

	err = ext4fs_mount();
	જો (!err) {
		printf("%s: ext4fs mount err - %d\n", __func__, err);
		પાછા -1;
	}

	err = ext4fs_open(filename, &filelen);
	જો (err < 0) {
		puts("spl: ext4fs_open failed\n");
		જાઓ end;
	}

	spl_load_init(&load, spl_fit_read, ખાલી, 1);
	err = spl_load(spl_image, bootdev, &load, filelen, 0);

end:
	જો (err < 0)
		printf("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);

	પાછા err < 0;
}

#જો CONFIG_IS_ENABLED(OS_BOOT)
પૂર્ણાંક spl_load_image_ext_os(સંરચના spl_image_info *spl_image,
			  સંરચના spl_boot_device *bootdev,
			  સંરચના blk_desc *block_dev, પૂર્ણાંક partition)
{
	પૂર્ણાંક err;
	__maybe_unused loff_t filelen, actlen;
	સંરચના disk_partition part_info = {};
	__maybe_unused અક્ષર *file;

	જો (part_get_info(block_dev, partition, &part_info)) {
		printf("spl: no partition table found\n");
		પાછા -1;
	}

	ext4fs_set_blk_dev(block_dev, &part_info);

	err = ext4fs_mount();
	જો (!err) {
		printf("%s: ext4fs mount err - %d\n", __func__, err);
		પાછા -1;
	}

	જો (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		જાઓ defaults;

	file = env_get("falcon_image_file");
	જો (file) {
		err = spl_load_image_ext(spl_image, bootdev, block_dev,
					 partition, file);
		જો (err != 0) {
			puts("spl: falling back to default\n");
			જાઓ defaults;
		}

#જો IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
		ext4fs_set_blk_dev(block_dev, &part_info);
		ext4fs_mount();
		file = env_get("falcon_args_file");
		જો (file) {
			err = ext4fs_open(file, &filelen);
			જો (err < 0) {
				puts("spl: ext4fs_open failed\n");
				જાઓ defaults;
			}
			err = ext4fs_read((શૂન્ય *)CONFIG_SPL_PAYLOAD_ARGS_ADDR,
					  0, filelen, &actlen);
			જો (err < 0) {
				printf("spl: error reading args %s, err - %d, falling back to default\n",
				       file, err);
				જાઓ defaults;
			}
			પાછા 0;
		} નહીંતર {
			puts("spl: falcon_args_file not set in environment, falling back to default\n");
		}
#endif
	} નહીંતર {
		puts("spl: falcon_image_file not set in environment, falling back to default\n");
	}

defaults:

	err = spl_load_image_ext(spl_image, bootdev, block_dev, partition,
				 CONFIG_SPL_FS_LOAD_KERNEL_NAME);

	જો (err)
		પાછા err;

#જો IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	ext4fs_set_blk_dev(block_dev, &part_info);
	ext4fs_mount();
	err = ext4fs_open(CONFIG_SPL_FS_LOAD_ARGS_NAME, &filelen);
	જો (err < 0)
		puts("spl: ext4fs_open failed\n");

	err = ext4fs_read((શૂન્ય *)CONFIG_SPL_PAYLOAD_ARGS_ADDR, 0, filelen, &actlen);
	જો (err < 0) {
		printf("%s: error reading image %s, err - %d\n",
		       __func__, CONFIG_SPL_FS_LOAD_ARGS_NAME, err);
		પાછા -1;
	}
#endif

	પાછા 0;
}
#નહીંતર
પૂર્ણાંક spl_load_image_ext_os(સંરચના spl_image_info *spl_image,
			  સંરચના spl_boot_device *bootdev,
			  સંરચના blk_desc *block_dev, પૂર્ણાંક partition)
{
	પાછા -ENOSYS;
}
#endif
