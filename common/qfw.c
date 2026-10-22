// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015 Miao Yan <yanmiaobest@gmail.com>
 * (C) Copyright 2021 Asherah Connor <ashe@kivikakk.ee>
 */

#ಅನ್ತರ್ಭಾವ <dm.h>
#ಅನ್ತರ್ಭಾವ <env.h>
#ಅನ್ತರ್ಭಾವ <mapmem.h>
#ಅನ್ತರ್ಭಾವ <qfw.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <dm/uclass.h>

ಪೂರ್ಣಾಂಕ qfw_get_dev(ರಚನೆ udevice **devp)
{
	ಹಿಂತಿರುಗಿ uclass_ಮೊದಲ_device_err(UCLASS_QFW, devp);
}

ಪೂರ್ಣಾಂಕ qfw_online_cpus(ರಚನೆ udevice *dev)
{
	u16 nb_cpus;

	qfw_read_ನಮೂದು(dev, FW_CFG_NB_CPUS, 2, &nb_cpus);

	ಹಿಂತಿರುಗಿ le16_ಗಮ್ಯ_cpu(nb_cpus);
}

ಪೂರ್ಣಾಂಕ qfw_read_firmware_list(ರಚನೆ udevice *dev)
{
	ಪೂರ್ಣಾಂಕ i;
	u32 ಎಣಿಕೆ_2;
	ರಚನೆ fw_file *file;
	ರಚನೆ list_ತಲೆ *ನಮೂದು;

	ರಚನೆ qfw_dev *qdev = dev_get_uclass_priv(dev);

	/* don't read it twice */
	ಆದರೆ (!list_ಖಾಲಿಗಳು(&qdev->fw_list))
		ಹಿಂತಿರುಗಿ 0;

	qfw_read_ನಮೂದು(dev, FW_CFG_FILE_DIR, 4, &ಎಣಿಕೆ_2);
	ಆದರೆ (!ಎಣಿಕೆ_2)
		ಹಿಂತಿರುಗಿ 0;

	ಎಣಿಕೆ_2 = be32_ಗಮ್ಯ_cpu(ಎಣಿಕೆ_2);
	ಗಾಗಿ (i = 0; i < ಎಣಿಕೆ_2; i++) {
		file = ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ(*file));
		ಆದರೆ (!file) {
			ಬರೆ("error: allocating resource\n");
			ಹೋಗು err;
		}
		qfw_read_ನಮೂದು(dev, FW_CFG_INVALID,
			       ಗಾತ್ರ(ರಚನೆ fw_cfg_file), &file->cfg);
		file->addr = 0;
		list_ಕೂಡಿಸು_ಬಾಲ(&file->list, &qdev->fw_list);
	}

	ಹಿಂತಿರುಗಿ 0;

err:
	list_for_each(ನಮೂದು, &qdev->fw_list) {
		file = list_ನಮೂದು(ನಮೂದು, ರಚನೆ fw_file, list);
		ಬಿಡುಗಡೆ(file);
	}

	ಹಿಂತಿರುಗಿ -ENOMEM;
}

ರಚನೆ fw_file *qfw_ಕಂಡುಹಿಡಿ_file(ರಚನೆ udevice *dev, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name)
{
	ರಚನೆ list_ತಲೆ *ನಮೂದು;
	ರಚನೆ fw_file *file;

	ರಚನೆ qfw_dev *qdev = dev_get_uclass_priv(dev);

	list_for_each(ನಮೂದು, &qdev->fw_list) {
		file = list_ನಮೂದು(ನಮೂದು, ರಚನೆ fw_file, list);
		ಆದರೆ (!strcmp(file->cfg.name, name))
			ಹಿಂತಿರುಗಿ file;
	}

	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

ರಚನೆ fw_file *qfw_file_iter_init(ರಚನೆ udevice *dev,
				   ರಚನೆ fw_cfg_file_iter *iter)
{
	ರಚನೆ qfw_dev *qdev = dev_get_uclass_priv(dev);

	iter->ನಮೂದು = qdev->fw_list.ಮುಂದಿನ;
	iter->ಕೊನೆ_2 = &qdev->fw_list;
	ಹಿಂತಿರುಗಿ list_ನಮೂದು((ರಚನೆ list_ತಲೆ *)iter->ನಮೂದು,
			  ರಚನೆ fw_file, list);
}

ರಚನೆ fw_file *qfw_file_iter_ಮುಂದಿನ(ರಚನೆ fw_cfg_file_iter *iter)
{
	iter->ನಮೂದು = ((ರಚನೆ list_ತಲೆ *)iter->ನಮೂದು)->ಮುಂದಿನ;
	ಹಿಂತಿರುಗಿ list_ನಮೂದು((ರಚನೆ list_ತಲೆ *)iter->ನಮೂದು,
			  ರಚನೆ fw_file, list);
}

bool qfw_file_iter_ಕೊನೆ(ರಚನೆ fw_cfg_file_iter *iter)
{
	ಹಿಂತಿರುಗಿ iter->ನಮೂದು == iter->ಕೊನೆ_2;
}

ಪೂರ್ಣಾಂಕ qemu_fwcfg_ಸಿದ್ಧತೆ_kernel(ರಚನೆ udevice *qfw_dev, ulong ತುಂಬು_addr,
			    ulong initrd_addr)
{
	ಅಕ್ಷರ *ದತ್ತಾಂಶ_addr;
	u32 ಸಿದ್ಧತೆ_ಗಾತ್ರ = 0, kernel_ಗಾತ್ರ = 0, cmdline_ಗಾತ್ರ = 0, initrd_ಗಾತ್ರ = 0;

	qfw_read_ನಮೂದು(qfw_dev, FW_CFG_ಸಿದ್ಧತೆ_ಗಾತ್ರ, 4, &ಸಿದ್ಧತೆ_ಗಾತ್ರ);
	qfw_read_ನಮೂದು(qfw_dev, FW_CFG_KERNEL_ಗಾತ್ರ, 4, &kernel_ಗಾತ್ರ);

	ಆದರೆ (!kernel_ಗಾತ್ರ) {
		ಬರೆ("fatal: no kernel available\n");
		ಹಿಂತಿರುಗಿ -ENOENT;
	}

	ದತ್ತಾಂಶ_addr = map_sysmem(ತುಂಬು_addr, 0);
	ಆದರೆ (ಸಿದ್ಧತೆ_ಗಾತ್ರ) {
		qfw_read_ನಮೂದು(qfw_dev, FW_CFG_ಸಿದ್ಧತೆ_ದತ್ತಾಂಶ,
			       le32_ಗಮ್ಯ_cpu(ಸಿದ್ಧತೆ_ಗಾತ್ರ), ದತ್ತಾಂಶ_addr);
		ದತ್ತಾಂಶ_addr += le32_ಗಮ್ಯ_cpu(ಸಿದ್ಧತೆ_ಗಾತ್ರ);
	}

	qfw_read_ನಮೂದು(qfw_dev, FW_CFG_KERNEL_ದತ್ತಾಂಶ,
		       le32_ಗಮ್ಯ_cpu(kernel_ಗಾತ್ರ), ದತ್ತಾಂಶ_addr);
	ದತ್ತಾಂಶ_addr += le32_ಗಮ್ಯ_cpu(kernel_ಗಾತ್ರ);
	env_ಗಣ_hex("filesize", le32_ಗಮ್ಯ_cpu(kernel_ಗಾತ್ರ));

	ದತ್ತಾಂಶ_addr = map_sysmem(initrd_addr, 0);
	qfw_read_ನಮೂದು(qfw_dev, FW_CFG_INITRD_ಗಾತ್ರ, 4, &initrd_ಗಾತ್ರ);
	ಆದರೆ (!initrd_ಗಾತ್ರ) {
		ಬರೆ("warning: no initrd available\n");
	} ಇಲ್ಲದಿದ್ದರೆ {
		qfw_read_ನಮೂದು(qfw_dev, FW_CFG_INITRD_ದತ್ತಾಂಶ,
			       le32_ಗಮ್ಯ_cpu(initrd_ಗಾತ್ರ), ದತ್ತಾಂಶ_addr);
		ದತ್ತಾಂಶ_addr += le32_ಗಮ್ಯ_cpu(initrd_ಗಾತ್ರ);
		env_ಗಣ_hex("filesize", le32_ಗಮ್ಯ_cpu(initrd_ಗಾತ್ರ));
	}

	qfw_read_ನಮೂದು(qfw_dev, FW_CFG_CMDLINE_ಗಾತ್ರ, 4, &cmdline_ಗಾತ್ರ);
	ಆದರೆ (cmdline_ಗಾತ್ರ) {
		qfw_read_ನಮೂದು(qfw_dev, FW_CFG_CMDLINE_ದತ್ತಾಂಶ,
			       le32_ಗಮ್ಯ_cpu(cmdline_ಗಾತ್ರ), ದತ್ತಾಂಶ_addr);
		/*
		 * if kernel cmdline only contains '\0', (e.g. no -append
		 * when invoking qemu), do not update bootargs
		 */
		ಆದರೆ (*ದತ್ತಾಂಶ_addr) {
			ಆದರೆ (env_ಗಣ("bootargs", ದತ್ತಾಂಶ_addr) < 0)
				ಬರೆ("warning: unable to change bootargs\n");
		}
	}

	ಬರೆ("loading kernel to address %lx size %x", ತುಂಬು_addr,
	       le32_ಗಮ್ಯ_cpu(kernel_ಗಾತ್ರ));
	ಆದರೆ (initrd_ಗಾತ್ರ)
		ಬರೆ(" initrd %lx size %x\n", initrd_addr,
		       le32_ಗಮ್ಯ_cpu(initrd_ಗಾತ್ರ));
	ಇಲ್ಲದಿದ್ದರೆ
		ಬರೆ("\n");

	ಹಿಂತಿರುಗಿ 0;
}
