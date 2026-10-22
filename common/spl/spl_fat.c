// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2014
 * Texas Instruments, <www.ti.com>
 *
 * Dan Murphy <dmurphy@ti.com>
 *
 * FAT Image Functions copied from spl_mmc.c
 */

#अन्तर्भाव <env.h>
#अन्तर्भाव <log.h>
#अन्तर्भाव <spl.h>
#अन्तर्भाव <spl_load.h>
#अन्तर्भाव <fat.h>
#अन्तर्भाव <errno.h>
#अन्तर्भाव <image.h>
#अन्तर्भाव <linux/libfdt.h>
#अन्तर्भाव <asm/cache.h>

स्थिर पूर्णाङ्क fat_registered;

शून्य spl_fat_force_reregister(शून्य)
{
	fat_registered = 0;
}

स्थिर पूर्णाङ्क spl_पञ्जिकाकोशः_fat_device(संरचना blk_desc *खण्डम्_dev, पूर्णाङ्क partition)
{
	पूर्णाङ्क err = 0;

	यदि (fat_registered)
		प्रत्यागम err;

	err = fat_पञ्जिकाकोशः_device(खण्डम्_dev, partition);
	यदि (err) {
		प्रारूप_लिख("%s: fat register err - %d\n", __func__, err);
		प्रत्यागम err;
	}

	fat_registered = 1;

	प्रत्यागम err;
}

स्थिर ulong spl_fit_read(संरचना spl_आरोपय_info *आरोपय, ulong file_विस्थापनम्,
			  ulong आकार, शून्य *buf)
{
	संरचना legacy_img_hdr *शीर्षकम्;
	loff_t actread;
	पूर्णाङ्क ret;
	वर्ण *filename = आरोपय->priv;

	ret = fat_read_file(filename, buf, file_विस्थापनम्, आकार, &actread);
	यदि (ret)
		प्रत्यागम ret;

	यदि (CONFIG_IS_ENABLED(OS_BOOT)) {
		शीर्षकम् = (संरचना legacy_img_hdr *)buf;
		यदि (image_get_magic(शीर्षकम्) != FDT_MAGIC)
			प्रत्यागम आकार;
	}

	प्रत्यागम actread;
}

पूर्णाङ्क spl_आरोपय_image_fat(संरचना spl_image_info *spl_image,
		       संरचना spl_boot_device *bootdev,
		       संरचना blk_desc *खण्डम्_dev, पूर्णाङ्क partition,
		       ध्रुव वर्ण *filename)
{
	पूर्णाङ्क err;
	loff_t आकार;
	संरचना spl_आरोपय_info आरोपय;

	err = spl_पञ्जिकाकोशः_fat_device(खण्डम्_dev, partition);
	यदि (err)
		गच्छ अन्तः_2;

	/*
	 * Avoid pulling in this function for other image types since we are
	 * very short on space on some boards.
	 */
	यदि (IS_ENABLED(CONFIG_SPL_आरोपय_FIT_पूर्णम्)) {
		err = fat_आकार(filename, &आकार);
		यदि (err)
			गच्छ अन्तः_2;
	} अन्यथा {
		आकार = 0;
	}

	spl_आरोपय_init(&आरोपय, spl_fit_read, (शून्य *)filename,
		      IS_ENABLED(CONFIG_SPL_FS_FAT_DMA_ALIGN) ?
		      ARCH_DMA_MINALIGN : 1);

	err = spl_आरोपय(spl_image, bootdev, &आरोपय, आकार, 0);

अन्तः_2:
	यदि (err < 0) {
		spl_fat_force_reregister();
		प्रारूप_लिख("%s: error reading image %s, err - %d\n",
		       __func__, filename, err);
	}

	प्रत्यागम err;
}

#यदि CONFIG_IS_ENABLED(OS_BOOT)
पूर्णाङ्क spl_आरोपय_image_fat_os(संरचना spl_image_info *spl_image,
			  संरचना spl_boot_device *bootdev,
			  संरचना blk_desc *खण्डम्_dev, पूर्णाङ्क partition)
{
	पूर्णाङ्क err;
	__maybe_unused वर्ण *file;

	err = spl_पञ्जिकाकोशः_fat_device(खण्डम्_dev, partition);
	यदि (err)
		प्रत्यागम err;

	यदि (!CONFIG_IS_ENABLED(ENV_SUPPORT))
		गच्छ defaults;

	file = env_get("falcon_image_file");
	यदि (file) {
		err = spl_आरोपय_image_fat(spl_image, bootdev, खण्डम्_dev,
					 partition, file);
		यदि (err != 0) {
			पङ्क्तिं_लिख("spl: falling back to default\n");
			गच्छ defaults;
		}

#यदि IS_ENABLED(CONFIG_SPL_OS_BOOT_तर्काः)
		file = env_get("falcon_args_file");
		यदि (file) {
			err = file_fat_read(
				file, (शून्य *)CONFIG_SPL_PAYLOAD_तर्काः_ADDR, 0);
			यदि (err <= 0) {
				प्रारूप_लिख("spl: error reading args %s, err - %d, falling back to default\n",
				       file, err);
				गच्छ defaults;
			}
			प्रत्यागम 0;
		} अन्यथा
			पङ्क्तिं_लिख("spl: falcon_args_file not set in environment, falling back to default\n");
#यदि_अन्तः

	} अन्यथा
		पङ्क्तिं_लिख("spl: falcon_image_file not set in environment, falling back to default\n");

defaults:

	err = spl_आरोपय_image_fat(spl_image, bootdev, खण्डम्_dev, partition,
				 CONFIG_SPL_FS_आरोपय_KERNEL_NAME);

	यदि (err)
		प्रत्यागम err;

#यदि IS_ENABLED(CONFIG_SPL_OS_BOOT_तर्काः)
	err = file_fat_read(CONFIG_SPL_FS_आरोपय_तर्काः_NAME,
			    (शून्य *)CONFIG_SPL_PAYLOAD_तर्काः_ADDR, 0);
	यदि (err <= 0) {
		प्रारूप_लिख("%s: error reading image %s, err - %d\n",
		       __func__, CONFIG_SPL_FS_आरोपय_तर्काः_NAME, err);
		प्रत्यागम -1;
	}
#यदि_अन्तः

	प्रत्यागम 0;
}
#अन्यथा
पूर्णाङ्क spl_आरोपय_image_fat_os(संरचना spl_image_info *spl_image,
			  संरचना spl_boot_device *bootdev,
			  संरचना blk_desc *खण्डम्_dev, पूर्णाङ्क partition)
{
	प्रत्यागम -ENOSYS;
}
#यदि_अन्तः
