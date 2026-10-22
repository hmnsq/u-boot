// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015 Miao Yan <yanmiaobest@gmail.com>
 * (C) Copyright 2021 Asherah Connor <ashe@kivikakk.ee>
 */

#অন্তর্ভাৱ <dm.h>
#অন্তর্ভাৱ <env.h>
#অন্তর্ভাৱ <mapmem.h>
#অন্তর্ভাৱ <qfw.h>
#অন্তর্ভাৱ <stdlib.h>
#অন্তর্ভাৱ <dm/uclass.h>

পূর্ণাঙ্ক qfw_get_dev(সংরচনা udevice **devp)
{
	ঘূৰাই uclass_first_device_err(UCLASS_QFW, devp);
}

পূর্ণাঙ্ক qfw_online_cpus(সংরচনা udevice *dev)
{
	u16 nb_cpus;

	qfw_read_entry(dev, FW_CFG_NB_CPUS, 2, &nb_cpus);

	ঘূৰাই le16_to_cpu(nb_cpus);
}

পূর্ণাঙ্ক qfw_read_firmware_list(সংরচনা udevice *dev)
{
	পূর্ণাঙ্ক i;
	u32 গুনতি;
	সংরচনা fw_file *file;
	সংরচনা list_head *entry;

	সংরচনা qfw_dev *qdev = dev_get_uclass_priv(dev);

	/* don't read it twice */
	যদি (!list_empty(&qdev->fw_list))
		ঘূৰাই 0;

	qfw_read_entry(dev, FW_CFG_FILE_DIR, 4, &গুনতি);
	যদি (!গুনতি)
		ঘূৰাই 0;

	গুনতি = be32_to_cpu(গুনতি);
	বাবে (i = 0; i < গুনতি; i++) {
		file = malloc(পরিমাণ(*file));
		যদি (!file) {
			printf("error: allocating resource\n");
			গচ্ছ err;
		}
		qfw_read_entry(dev, FW_CFG_INVALID,
			       পরিমাণ(সংরচনা fw_cfg_file), &file->cfg);
		file->addr = 0;
		list_add_tail(&file->list, &qdev->fw_list);
	}

	ঘূৰাই 0;

err:
	list_for_each(entry, &qdev->fw_list) {
		file = list_entry(entry, সংরচনা fw_file, list);
		মুক্ত(file);
	}

	ঘূৰাই -ENOMEM;
}

সংরচনা fw_file *qfw_find_file(সংরচনা udevice *dev, ধ্রুৱ ৱর্ণ *name)
{
	সংরচনা list_head *entry;
	সংরচনা fw_file *file;

	সংরচনা qfw_dev *qdev = dev_get_uclass_priv(dev);

	list_for_each(entry, &qdev->fw_list) {
		file = list_entry(entry, সংরচনা fw_file, list);
		যদি (!strcmp(file->cfg.name, name))
			ঘূৰাই file;
	}

	ঘূৰাই রিক্ত;
}

সংরচনা fw_file *qfw_file_iter_init(সংরচনা udevice *dev,
				   সংরচনা fw_cfg_file_iter *iter)
{
	সংরচনা qfw_dev *qdev = dev_get_uclass_priv(dev);

	iter->entry = qdev->fw_list.next;
	iter->end = &qdev->fw_list;
	ঘূৰাই list_entry((সংরচনা list_head *)iter->entry,
			  সংরচনা fw_file, list);
}

সংরচনা fw_file *qfw_file_iter_next(সংরচনা fw_cfg_file_iter *iter)
{
	iter->entry = ((সংরচনা list_head *)iter->entry)->next;
	ঘূৰাই list_entry((সংরচনা list_head *)iter->entry,
			  সংরচনা fw_file, list);
}

bool qfw_file_iter_end(সংরচনা fw_cfg_file_iter *iter)
{
	ঘূৰাই iter->entry == iter->end;
}

পূর্ণাঙ্ক qemu_fwcfg_setup_kernel(সংরচনা udevice *qfw_dev, ulong load_addr,
			    ulong initrd_addr)
{
	ৱর্ণ *data_addr;
	u32 setup_size = 0, kernel_size = 0, cmdline_size = 0, initrd_size = 0;

	qfw_read_entry(qfw_dev, FW_CFG_SETUP_SIZE, 4, &setup_size);
	qfw_read_entry(qfw_dev, FW_CFG_KERNEL_SIZE, 4, &kernel_size);

	যদি (!kernel_size) {
		printf("fatal: no kernel available\n");
		ঘূৰাই -ENOENT;
	}

	data_addr = map_sysmem(load_addr, 0);
	যদি (setup_size) {
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
	যদি (!initrd_size) {
		printf("warning: no initrd available\n");
	} অন্যথা {
		qfw_read_entry(qfw_dev, FW_CFG_INITRD_DATA,
			       le32_to_cpu(initrd_size), data_addr);
		data_addr += le32_to_cpu(initrd_size);
		env_set_hex("filesize", le32_to_cpu(initrd_size));
	}

	qfw_read_entry(qfw_dev, FW_CFG_CMDLINE_SIZE, 4, &cmdline_size);
	যদি (cmdline_size) {
		qfw_read_entry(qfw_dev, FW_CFG_CMDLINE_DATA,
			       le32_to_cpu(cmdline_size), data_addr);
		/*
		 * if kernel cmdline only contains '\0', (e.g. no -append
		 * when invoking qemu), do not update bootargs
		 */
		যদি (*data_addr) {
			যদি (env_set("bootargs", data_addr) < 0)
				printf("warning: unable to change bootargs\n");
		}
	}

	printf("loading kernel to address %lx size %x", load_addr,
	       le32_to_cpu(kernel_size));
	যদি (initrd_size)
		printf(" initrd %lx size %x\n", initrd_addr,
		       le32_to_cpu(initrd_size));
	অন্যথা
		printf("\n");

	ঘূৰাই 0;
}
