// SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
/*
 * Copyright (C) 2016
 * Ladislav Michl <ladis@linux-mips.org>
 */

#समावेश <config.h>
#समावेश <image.h>
#समावेश <nand.h>
#समावेश <onenand_uboot.h>
#समावेश <ubispl.h>
#समावेश <spl.h>

#अगर IS_ENABLED(CONFIG_SPL_OS_BOOT)
पूर्णांक spl_ubi_load_image_os(काष्ठा spl_image_info *spl_image,
			  काष्ठा spl_boot_device *bootdev,
			  काष्ठा ubispl_info *info)
{
	काष्ठा legacy_img_hdr *header;
	काष्ठा ubispl_load volumes[2];
	पूर्णांक err;

	volumes[0].vol_id = CONFIG_SPL_UBI_LOAD_KERNEL_ID;
	volumes[0].load_addr = (व्योम *)CONFIG_SYS_LOAD_ADDR;
#अगर IS_ENABLED(CONFIG_SPL_OS_BOOT_ARGS)
	volumes[1].vol_id = CONFIG_SPL_UBI_LOAD_ARGS_ID;
	volumes[1].load_addr = (व्योम *)CONFIG_SPL_PAYLOAD_ARGS_ADDR;

	err = ubispl_load_volumes(info, volumes, 2);
#अन्यथा
	err = ubispl_load_volumes(info, volumes, 1);
#पूर्ण_अगर
	अगर (err)
		वापस err;

	header = (काष्ठा legacy_img_hdr *)volumes[0].load_addr;
	spl_parse_image_header(spl_image, bootdev, header);
	माला_दो("Linux loaded.\n");

	वापस 0;
}
#पूर्ण_अगर

पूर्णांक spl_ubi_load_image(काष्ठा spl_image_info *spl_image,
		       काष्ठा spl_boot_device *bootdev)
{
	काष्ठा legacy_img_hdr *header;
	काष्ठा ubispl_info info;
	काष्ठा ubispl_load volumes[2];
	पूर्णांक ret = 1;

	चयन (bootdev->boot_device) {
#अगर_घोषित CONFIG_SPL_NAND_SUPPORT
	हाल BOOT_DEVICE_NAND:
		nand_init();
		info.पढ़ो = nand_spl_read_block;
		info.peb_आकार = CONFIG_SYS_NAND_BLOCK_आकार;
		अवरोध;
#पूर्ण_अगर
#अगर_घोषित CONFIG_SPL_ONENAND_SUPPORT
	हाल BOOT_DEVICE_ONENAND:
		info.पढ़ो = onenand_spl_read_block;
		info.peb_आकार = CFG_SYS_ONENAND_BLOCK_आकार;
		अवरोध;
#पूर्ण_अगर
	शेष:
		जाओ out;
	}
	info.ubi = (काष्ठा ubi_scan_info *)CONFIG_SPL_UBI_INFO_ADDR;
	info.fastmap = IS_ENABLED(CONFIG_MTD_UBI_FASTMAP);

	info.peb_offset = CONFIG_SPL_UBI_PEB_OFFSET;
	info.vid_offset = CONFIG_SPL_UBI_VID_OFFSET;
	info.leb_start = CONFIG_SPL_UBI_LEB_START;
	info.peb_गिनती = CONFIG_SPL_UBI_MAX_PEBS - info.peb_offset;

#अगर CONFIG_IS_ENABLED(OS_BOOT)
	अगर (!spl_start_uboot()) {
		ret = spl_ubi_load_image_os(spl_image, bootdev, &info);
		अगर (!ret)
			वापस 0;

		म_लिखो("%s: Failed in falcon boot: %d", __func__, ret);
		अगर (IS_ENABLED(CONFIG_SPL_OS_BOOT_SECURE))
			वापस ret;
		म_लिखो("Fallback to U-Boot\n");
	}
#पूर्ण_अगर

	header = spl_get_load_बफर(-माप(*header), माप(header));
#अगर_घोषित CONFIG_SPL_UBI_LOAD_BY_VOLNAME
	volumes[0].vol_id = -1;
	म_नकलन(volumes[0].name,
		CONFIG_SPL_UBI_LOAD_MONITOR_VOLNAME,
		UBI_VOL_NAME_MAX + 1);
#अन्यथा
	volumes[0].vol_id = CONFIG_SPL_UBI_LOAD_MONITOR_ID;
#पूर्ण_अगर
	volumes[0].load_addr = (व्योम *)header;

	ret = ubispl_load_volumes(&info, volumes, 1);
	अगर (!ret)
		spl_parse_image_header(spl_image, bootdev, header);
out:
#अगर_घोषित CONFIG_SPL_NAND_SUPPORT
	अगर (bootdev->boot_device == BOOT_DEVICE_NAND)
		nand_deselect();
#पूर्ण_अगर
	वापस ret;
}
/* Use priorty 0 so that Ubi will override NAND and ONENAND methods */
SPL_LOAD_IMAGE_METHOD("NAND", 0, BOOT_DEVICE_NAND, spl_ubi_load_image);
SPL_LOAD_IMAGE_METHOD("OneNAND", 0, BOOT_DEVICE_ONENAND, spl_ubi_load_image);
