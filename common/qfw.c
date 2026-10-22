// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015 Miao Yan <yanmiaobest@gmail.com>
 * (C) Copyright 2021 Asherah Connor <ashe@kivikakk.ee>
 */

#അന്തര്ഭാവ <dm.h>
#അന്തര്ഭാവ <env.h>
#അന്തര്ഭാവ <mapmem.h>
#അന്തര്ഭാവ <qfw.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <dm/uclass.h>

പൂർണ്ണസംഖ്യ qfw_get_dev(ഘടന udevice **devp)
{
	തിരികെ uclass_first_device_err(UCLASS_QFW, devp);
}

പൂർണ്ണസംഖ്യ qfw_online_cpus(ഘടന udevice *dev)
{
	u16 nb_cpus;

	qfw_read_entry(dev, FW_CFG_NB_CPUS, 2, &nb_cpus);

	തിരികെ le16_to_cpu(nb_cpus);
}

പൂർണ്ണസംഖ്യ qfw_read_firmware_list(ഘടന udevice *dev)
{
	പൂർണ്ണസംഖ്യ i;
	u32 എണ്ണം;
	ഘടന fw_file *file;
	ഘടന list_head *entry;

	ഘടന qfw_dev *qdev = dev_get_uclass_priv(dev);

	/* don't read it twice */
	എങ്കിൽ (!list_empty(&qdev->fw_list))
		തിരികെ 0;

	qfw_read_entry(dev, FW_CFG_FILE_DIR, 4, &എണ്ണം);
	എങ്കിൽ (!എണ്ണം)
		തിരികെ 0;

	എണ്ണം = be32_to_cpu(എണ്ണം);
	വേണ്ടി (i = 0; i < എണ്ണം; i++) {
		file = malloc(വലുപ്പം(*file));
		എങ്കിൽ (!file) {
			printf("error: allocating resource\n");
			പോകുക err;
		}
		qfw_read_entry(dev, FW_CFG_INVALID,
			       വലുപ്പം(ഘടന fw_cfg_file), &file->cfg);
		file->addr = 0;
		list_add_tail(&file->list, &qdev->fw_list);
	}

	തിരികെ 0;

err:
	list_for_each(entry, &qdev->fw_list) {
		file = list_entry(entry, ഘടന fw_file, list);
		സ്വതന്ത്രം(file);
	}

	തിരികെ -ENOMEM;
}

ഘടന fw_file *qfw_find_file(ഘടന udevice *dev, സ്ഥിരം അക്ഷരം *name)
{
	ഘടന list_head *entry;
	ഘടന fw_file *file;

	ഘടന qfw_dev *qdev = dev_get_uclass_priv(dev);

	list_for_each(entry, &qdev->fw_list) {
		file = list_entry(entry, ഘടന fw_file, list);
		എങ്കിൽ (!strcmp(file->cfg.name, name))
			തിരികെ file;
	}

	തിരികെ ശൂന്യ;
}

ഘടന fw_file *qfw_file_iter_init(ഘടന udevice *dev,
				   ഘടന fw_cfg_file_iter *iter)
{
	ഘടന qfw_dev *qdev = dev_get_uclass_priv(dev);

	iter->entry = qdev->fw_list.next;
	iter->end = &qdev->fw_list;
	തിരികെ list_entry((ഘടന list_head *)iter->entry,
			  ഘടന fw_file, list);
}

ഘടന fw_file *qfw_file_iter_next(ഘടന fw_cfg_file_iter *iter)
{
	iter->entry = ((ഘടന list_head *)iter->entry)->next;
	തിരികെ list_entry((ഘടന list_head *)iter->entry,
			  ഘടന fw_file, list);
}

bool qfw_file_iter_end(ഘടന fw_cfg_file_iter *iter)
{
	തിരികെ iter->entry == iter->end;
}

പൂർണ്ണസംഖ്യ qemu_fwcfg_setup_kernel(ഘടന udevice *qfw_dev, ulong load_addr,
			    ulong initrd_addr)
{
	അക്ഷരം *data_addr;
	u32 setup_size = 0, kernel_size = 0, cmdline_size = 0, initrd_size = 0;

	qfw_read_entry(qfw_dev, FW_CFG_SETUP_SIZE, 4, &setup_size);
	qfw_read_entry(qfw_dev, FW_CFG_KERNEL_SIZE, 4, &kernel_size);

	എങ്കിൽ (!kernel_size) {
		printf("fatal: no kernel available\n");
		തിരികെ -ENOENT;
	}

	data_addr = map_sysmem(load_addr, 0);
	എങ്കിൽ (setup_size) {
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
	എങ്കിൽ (!initrd_size) {
		printf("warning: no initrd available\n");
	} അല്ലെങ്കിൽ {
		qfw_read_entry(qfw_dev, FW_CFG_INITRD_DATA,
			       le32_to_cpu(initrd_size), data_addr);
		data_addr += le32_to_cpu(initrd_size);
		env_set_hex("filesize", le32_to_cpu(initrd_size));
	}

	qfw_read_entry(qfw_dev, FW_CFG_CMDLINE_SIZE, 4, &cmdline_size);
	എങ്കിൽ (cmdline_size) {
		qfw_read_entry(qfw_dev, FW_CFG_CMDLINE_DATA,
			       le32_to_cpu(cmdline_size), data_addr);
		/*
		 * if kernel cmdline only contains '\0', (e.g. no -append
		 * when invoking qemu), do not update bootargs
		 */
		എങ്കിൽ (*data_addr) {
			എങ്കിൽ (env_set("bootargs", data_addr) < 0)
				printf("warning: unable to change bootargs\n");
		}
	}

	printf("loading kernel to address %lx size %x", load_addr,
	       le32_to_cpu(kernel_size));
	എങ്കിൽ (initrd_size)
		printf(" initrd %lx size %x\n", initrd_addr,
		       le32_to_cpu(initrd_size));
	അല്ലെങ്കിൽ
		printf("\n");

	തിരികെ 0;
}
