// SPDX-License-Identifier: GPL-2.0+

#ضمن <env.h>
#ضمن <part.h>
#ضمن <spl.h>
#ضمن <spl_load.h>
#ضمن <ext4fs.h>
#ضمن <errno.h>
#ضمن <image.h>

ساكن ulong spl_fit_read(بنية spl_ڀريو_info *ڀريو, ulong file_وٿي,
			  ulong سائيز, فراغ *buf)
{
	بنية legacy_img_hdr *سرو;
	صحيح ret;
	loff_t actlen;

	ret = ext4fs_read(buf, file_وٿي, سائيز, &actlen);
	إذا (ret)
		أرجع ret;

	إذا (CONFIG_IS_ENABLED(OS_BOOT)) {
		سرو = (بنية legacy_img_hdr *)buf;
		إذا (image_get_magic(سرو) != FDT_MAGIC)
			أرجع سائيز;
	}

	أرجع actlen;
}

صحيح spl_ڀريو_image_ext(بنية spl_image_info *spl_image,
		       بنية spl_boot_device *bootdev,
		       بنية blk_desc *بلاڪ_dev, صحيح حصا_ڪريو,
		       ثابت حرف *filename)
{
	s32 err;
	loff_t filelen;
	بنية disk_حصا_ڪريو part_info = {};
	بنية spl_ڀريو_info ڀريو;

	إذا (part_get_info(بلاڪ_dev, حصا_ڪريو, &part_info)) {
		لکو("spl: no partition table found\n");
		أرجع -1;
	}

	ext4fs_سيٽ_blk_dev(بلاڪ_dev, &part_info);

	err = ext4fs_mount();
	إذا (!err) {
		لکو("%s: ext4fs mount err - %d\n", __func__, err);
		أرجع -1;
	}

	err = ext4fs_open(filename, &filelen);
	إذا (err < 0) {
		puts("spl: ext4fs_open failed\n");
		اذهب پڇاڙي_2;
	}

	spl_ڀريو_init(&ڀريو, spl_fit_read, عدم, 1);
	err = spl_ڀريو(spl_image, bootdev, &ڀريو, filelen, 0);

پڇاڙي_2:
	إذا (err < 0)
		لکو("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);

	أرجع err < 0;
}

#إذا CONFIG_IS_ENABLED(OS_BOOT)
صحيح spl_ڀريو_image_ext_os(بنية spl_image_info *spl_image,
			  بنية spl_boot_device *bootdev,
			  بنية blk_desc *بلاڪ_dev, صحيح حصا_ڪريو)
{
	صحيح err;
	__maybe_unused loff_t filelen, actlen;
	بنية disk_حصا_ڪريو part_info = {};
	__maybe_unused حرف *file;

	إذا (part_get_info(بلاڪ_dev, حصا_ڪريو, &part_info)) {
		لکو("spl: no partition table found\n");
		أرجع -1;
	}

	ext4fs_سيٽ_blk_dev(بلاڪ_dev, &part_info);

	err = ext4fs_mount();
	إذا (!err) {
		لکو("%s: ext4fs mount err - %d\n", __func__, err);
		أرجع -1;
	}

	إذا (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		اذهب defaults;

	file = env_get("falcon_image_file");
	إذا (file) {
		err = spl_ڀريو_image_ext(spl_image, bootdev, بلاڪ_dev,
					 حصا_ڪريو, file);
		إذا (err != 0) {
			puts("spl: falling back to default\n");
			اذهب defaults;
		}

#إذا IS_ENABLED(CONFIG_SPL_OS_BOOT_دليلون)
		ext4fs_سيٽ_blk_dev(بلاڪ_dev, &part_info);
		ext4fs_mount();
		file = env_get("falcon_args_file");
		إذا (file) {
			err = ext4fs_open(file, &filelen);
			إذا (err < 0) {
				puts("spl: ext4fs_open failed\n");
				اذهب defaults;
			}
			err = ext4fs_read((فراغ *)CONFIG_SPL_PAYLOAD_دليلون_ADDR,
					  0, filelen, &actlen);
			إذا (err < 0) {
				لکو("spl: error reading args %s, err - %d, falling back to default\n",
				       file, err);
				اذهب defaults;
			}
			أرجع 0;
		} وإلا {
			puts("spl: falcon_args_file not set in environment, falling back to default\n");
		}
#جيڪڏهن_پورو
	} وإلا {
		puts("spl: falcon_image_file not set in environment, falling back to default\n");
	}

defaults:

	err = spl_ڀريو_image_ext(spl_image, bootdev, بلاڪ_dev, حصا_ڪريو,
				 CONFIG_SPL_FS_ڀريو_KERNEL_NAME);

	إذا (err)
		أرجع err;

#إذا IS_ENABLED(CONFIG_SPL_OS_BOOT_دليلون)
	ext4fs_سيٽ_blk_dev(بلاڪ_dev, &part_info);
	ext4fs_mount();
	err = ext4fs_open(CONFIG_SPL_FS_ڀريو_دليلون_NAME, &filelen);
	إذا (err < 0)
		puts("spl: ext4fs_open failed\n");

	err = ext4fs_read((فراغ *)CONFIG_SPL_PAYLOAD_دليلون_ADDR, 0, filelen, &actlen);
	إذا (err < 0) {
		لکو("%s: error reading image %s, err - %d\n",
		       __func__, CONFIG_SPL_FS_ڀريو_دليلون_NAME, err);
		أرجع -1;
	}
#جيڪڏهن_پورو

	أرجع 0;
}
#وإلا
صحيح spl_ڀريو_image_ext_os(بنية spl_image_info *spl_image,
			  بنية spl_boot_device *bootdev,
			  بنية blk_desc *بلاڪ_dev, صحيح حصا_ڪريو)
{
	أرجع -ENOSYS;
}
#جيڪڏهن_پورو
