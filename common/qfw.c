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

ಪೂರ್ಣಾಙ್ಕ qfw_get_dev(ಸಂರಚನಾ udevice **devp)
{
	ಪ್ರತ್ಯಾಗಮ uclass_first_device_err(UCLASS_QFW, devp);
}

ಪೂರ್ಣಾಙ್ಕ qfw_online_cpus(ಸಂರಚನಾ udevice *dev)
{
	u16 nb_cpus;

	qfw_read_entry(dev, FW_CFG_NB_CPUS, 2, &nb_cpus);

	ಪ್ರತ್ಯಾಗಮ le16_to_cpu(nb_cpus);
}

ಪೂರ್ಣಾಙ್ಕ qfw_read_firmware_list(ಸಂರಚನಾ udevice *dev)
{
	ಪೂರ್ಣಾಙ್ಕ i;
	u32 ಎಣಿಕೆ;
	ಸಂರಚನಾ fw_file *file;
	ಸಂರಚನಾ list_head *entry;

	ಸಂರಚನಾ qfw_dev *qdev = dev_get_uclass_priv(dev);

	/* don't read it twice */
	ಯದಿ (!list_empty(&qdev->fw_list))
		ಪ್ರತ್ಯಾಗಮ 0;

	qfw_read_entry(dev, FW_CFG_FILE_DIR, 4, &ಎಣಿಕೆ);
	ಯದಿ (!ಎಣಿಕೆ)
		ಪ್ರತ್ಯಾಗಮ 0;

	ಎಣಿಕೆ = be32_to_cpu(ಎಣಿಕೆ);
	ಪ್ರತಿ (i = 0; i < ಎಣಿಕೆ; i++) {
		file = malloc(ಪರಿಮಾಣ(*file));
		ಯದಿ (!file) {
			printf("error: allocating resource\n");
			ಗಚ್ಛ err;
		}
		qfw_read_entry(dev, FW_CFG_INVALID,
			       ಪರಿಮಾಣ(ಸಂರಚನಾ fw_cfg_file), &file->cfg);
		file->addr = 0;
		list_add_tail(&file->list, &qdev->fw_list);
	}

	ಪ್ರತ್ಯಾಗಮ 0;

err:
	list_for_each(entry, &qdev->fw_list) {
		file = list_entry(entry, ಸಂರಚನಾ fw_file, list);
		ಮುಕ್ತ(file);
	}

	ಪ್ರತ್ಯಾಗಮ -ENOMEM;
}

ಸಂರಚನಾ fw_file *qfw_find_file(ಸಂರಚನಾ udevice *dev, ಧ್ರುವ ವರ್ಣ *name)
{
	ಸಂರಚನಾ list_head *entry;
	ಸಂರಚನಾ fw_file *file;

	ಸಂರಚನಾ qfw_dev *qdev = dev_get_uclass_priv(dev);

	list_for_each(entry, &qdev->fw_list) {
		file = list_entry(entry, ಸಂರಚನಾ fw_file, list);
		ಯದಿ (!strcmp(file->cfg.name, name))
			ಪ್ರತ್ಯಾಗಮ file;
	}

	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

ಸಂರಚನಾ fw_file *qfw_file_iter_init(ಸಂರಚನಾ udevice *dev,
				   ಸಂರಚನಾ fw_cfg_file_iter *iter)
{
	ಸಂರಚನಾ qfw_dev *qdev = dev_get_uclass_priv(dev);

	iter->entry = qdev->fw_list.next;
	iter->end = &qdev->fw_list;
	ಪ್ರತ್ಯಾಗಮ list_entry((ಸಂರಚನಾ list_head *)iter->entry,
			  ಸಂರಚನಾ fw_file, list);
}

ಸಂರಚನಾ fw_file *qfw_file_iter_next(ಸಂರಚನಾ fw_cfg_file_iter *iter)
{
	iter->entry = ((ಸಂರಚನಾ list_head *)iter->entry)->next;
	ಪ್ರತ್ಯಾಗಮ list_entry((ಸಂರಚನಾ list_head *)iter->entry,
			  ಸಂರಚನಾ fw_file, list);
}

bool qfw_file_iter_end(ಸಂರಚನಾ fw_cfg_file_iter *iter)
{
	ಪ್ರತ್ಯಾಗಮ iter->entry == iter->end;
}

ಪೂರ್ಣಾಙ್ಕ qemu_fwcfg_setup_kernel(ಸಂರಚನಾ udevice *qfw_dev, ulong load_addr,
			    ulong initrd_addr)
{
	ವರ್ಣ *data_addr;
	u32 setup_size = 0, kernel_size = 0, cmdline_size = 0, initrd_size = 0;

	qfw_read_entry(qfw_dev, FW_CFG_SETUP_SIZE, 4, &setup_size);
	qfw_read_entry(qfw_dev, FW_CFG_KERNEL_SIZE, 4, &kernel_size);

	ಯದಿ (!kernel_size) {
		printf("fatal: no kernel available\n");
		ಪ್ರತ್ಯಾಗಮ -ENOENT;
	}

	data_addr = map_sysmem(load_addr, 0);
	ಯದಿ (setup_size) {
		qfw_read_entry(qfw_dev, FW_CFG_SETUP_DATA,
			       le32_to_cpu(setup_size), data_addr);
		data_addr += le32_to_cpu(setup_size);
	}

	qfw_read_entry(qfw_dev, FW_CFG_KERNEL_DATA,
		       le32_to_cpu(kernel_size), data_addr);
	data_addr += le32_to_cpu(kernel_size);
	env_set_hex("filesize", le32_to_cpu(kernel_size));

	data_addr = map_sysmem(initrd_addr, 0);
	qfw_read_entry(qfw_dev, FW_CFG_INITRD_SIZE, 4, &initrd_size);
	ಯದಿ (!initrd_size) {
		printf("warning: no initrd available\n");
	} ಅನ್ಯಥಾ {
		qfw_read_entry(qfw_dev, FW_CFG_INITRD_DATA,
			       le32_to_cpu(initrd_size), data_addr);
		data_addr += le32_to_cpu(initrd_size);
		env_set_hex("filesize", le32_to_cpu(initrd_size));
	}

	qfw_read_entry(qfw_dev, FW_CFG_CMDLINE_SIZE, 4, &cmdline_size);
	ಯದಿ (cmdline_size) {
		qfw_read_entry(qfw_dev, FW_CFG_CMDLINE_DATA,
			       le32_to_cpu(cmdline_size), data_addr);
		/*
		 * if kernel cmdline only contains '\0', (e.g. no -append
		 * when invoking qemu), do not update bootargs
		 */
		ಯದಿ (*data_addr) {
			ಯದಿ (env_set("bootargs", data_addr) < 0)
				printf("warning: unable to change bootargs\n");
		}
	}

	printf("loading kernel to address %lx size %x", load_addr,
	       le32_to_cpu(kernel_size));
	ಯದಿ (initrd_size)
		printf(" initrd %lx size %x\n", initrd_addr,
		       le32_to_cpu(initrd_size));
	ಅನ್ಯಥಾ
		printf("\n");

	ಪ್ರತ್ಯಾಗಮ 0;
}
