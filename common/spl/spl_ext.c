// SPDX-License-Identifier: GPL-2.0+

#ضمن <env.h>
#ضمن <part.h>
#ضمن <spl.h>
#ضمن <spl_load.h>
#ضمن <ext4fs.h>
#ضمن <errno.h>
#ضمن <image.h>

ساكن ulong spl_fit_read(بنية spl_بارول_info *بارول, ulong file_دننه_واټن,
			  ulong اندازه, فراغ *buf)
{
	بنية legacy_img_hdr *سرلیک;
	صحيح ret;
	loff_t actlen;

	ret = ext4fs_read(buf, file_دننه_واټن, اندازه, &actlen);
	إذا (ret)
		أرجع ret;

	إذا (CONFIG_IS_ENABLED(OS_BOOT)) {
		سرلیک = (بنية legacy_img_hdr *)buf;
		إذا (image_get_magic(سرلیک) != FDT_MAGIC)
			أرجع اندازه;
	}

	أرجع actlen;
}

صحيح spl_بارول_image_ext(بنية spl_image_info *spl_image,
		       بنية spl_boot_device *bootdev,
		       بنية blk_desc *بلاک_dev, صحيح برخه_کول,
		       ثابت حرف *filename)
{
	s32 err;
	loff_t filelen;
	بنية disk_برخه_کول part_info = {};
	بنية spl_بارول_info بارول;

	إذا (part_get_info(بلاک_dev, برخه_کول, &part_info)) {
		چاپ_کړه("spl: no partition table found\n");
		أرجع -1;
	}

	ext4fs_سېټ_blk_dev(بلاک_dev, &part_info);

	err = ext4fs_mount();
	إذا (!err) {
		چاپ_کړه("%s: ext4fs mount err - %d\n", __func__, err);
		أرجع -1;
	}

	err = ext4fs_open(filename, &filelen);
	إذا (err < 0) {
		puts("spl: ext4fs_open failed\n");
		اذهب پای_ټکی_2;
	}

	spl_بارول_init(&بارول, spl_fit_read, عدم, 1);
	err = spl_بارول(spl_image, bootdev, &بارول, filelen, 0);

پای_ټکی_2:
	إذا (err < 0)
		چاپ_کړه("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);

	أرجع err < 0;
}

#إذا CONFIG_IS_ENABLED(OS_BOOT)
صحيح spl_بارول_image_ext_os(بنية spl_image_info *spl_image,
			  بنية spl_boot_device *bootdev,
			  بنية blk_desc *بلاک_dev, صحيح برخه_کول)
{
	صحيح err;
	__maybe_unused loff_t filelen, actlen;
	بنية disk_برخه_کول part_info = {};
	__maybe_unused حرف *file;

	إذا (part_get_info(بلاک_dev, برخه_کول, &part_info)) {
		چاپ_کړه("spl: no partition table found\n");
		أرجع -1;
	}

	ext4fs_سېټ_blk_dev(بلاک_dev, &part_info);

	err = ext4fs_mount();
	إذا (!err) {
		چاپ_کړه("%s: ext4fs mount err - %d\n", __func__, err);
		أرجع -1;
	}

	إذا (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		اذهب defaults;

	file = env_get("falcon_image_file");
	إذا (file) {
		err = spl_بارول_image_ext(spl_image, bootdev, بلاک_dev,
					 برخه_کول, file);
		إذا (err != 0) {
			puts("spl: falling back to default\n");
			اذهب defaults;
		}

#إذا IS_ENABLED(CONFIG_SPL_OS_BOOT_دلیلونه)
		ext4fs_سېټ_blk_dev(بلاک_dev, &part_info);
		ext4fs_mount();
		file = env_get("falcon_args_file");
		إذا (file) {
			err = ext4fs_open(file, &filelen);
			إذا (err < 0) {
				puts("spl: ext4fs_open failed\n");
				اذهب defaults;
			}
			err = ext4fs_read((فراغ *)CONFIG_SPL_PAYLOAD_دلیلونه_ADDR,
					  0, filelen, &actlen);
			إذا (err < 0) {
				چاپ_کړه("spl: error reading args %s, err - %d, falling back to default\n",
				       file, err);
				اذهب defaults;
			}
			أرجع 0;
		} وإلا {
			puts("spl: falcon_args_file not set in environment, falling back to default\n");
		}
#د_که_پای
	} وإلا {
		puts("spl: falcon_image_file not set in environment, falling back to default\n");
	}

defaults:

	err = spl_بارول_image_ext(spl_image, bootdev, بلاک_dev, برخه_کول,
				 CONFIG_SPL_FS_بارول_KERNEL_NAME);

	إذا (err)
		أرجع err;

#إذا IS_ENABLED(CONFIG_SPL_OS_BOOT_دلیلونه)
	ext4fs_سېټ_blk_dev(بلاک_dev, &part_info);
	ext4fs_mount();
	err = ext4fs_open(CONFIG_SPL_FS_بارول_دلیلونه_NAME, &filelen);
	إذا (err < 0)
		puts("spl: ext4fs_open failed\n");

	err = ext4fs_read((فراغ *)CONFIG_SPL_PAYLOAD_دلیلونه_ADDR, 0, filelen, &actlen);
	إذا (err < 0) {
		چاپ_کړه("%s: error reading image %s, err - %d\n",
		       __func__, CONFIG_SPL_FS_بارول_دلیلونه_NAME, err);
		أرجع -1;
	}
#د_که_پای

	أرجع 0;
}
#وإلا
صحيح spl_بارول_image_ext_os(بنية spl_image_info *spl_image,
			  بنية spl_boot_device *bootdev,
			  بنية blk_desc *بلاک_dev, صحيح برخه_کول)
{
	أرجع -ENOSYS;
}
#د_که_پای
