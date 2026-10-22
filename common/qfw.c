// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015 Miao Yan <yanmiaobest@gmail.com>
 * (C) Copyright 2021 Asherah Connor <ashe@kivikakk.ee>
 */

#அந்தர்பாவ <dm.h>
#அந்தர்பாவ <env.h>
#அந்தர்பாவ <mapmem.h>
#அந்தர்பாவ <qfw.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <dm/uclass.h>

முழு qfw_get_dev(கட்டமைப்பு udevice **devp)
{
	திரும்பு uclass_first_device_err(UCLASS_QFW, devp);
}

முழு qfw_online_cpus(கட்டமைப்பு udevice *dev)
{
	u16 nb_cpus;

	qfw_read_entry(dev, FW_CFG_NB_CPUS, 2, &nb_cpus);

	திரும்பு le16_to_cpu(nb_cpus);
}

முழு qfw_read_firmware_list(கட்டமைப்பு udevice *dev)
{
	முழு i;
	u32 மொத்தம்;
	கட்டமைப்பு fw_file *file;
	கட்டமைப்பு list_head *entry;

	கட்டமைப்பு qfw_dev *qdev = dev_get_uclass_priv(dev);

	/* don't read it twice */
	எனில் (!list_empty(&qdev->fw_list))
		திரும்பு 0;

	qfw_read_entry(dev, FW_CFG_FILE_DIR, 4, &மொத்தம்);
	எனில் (!மொத்தம்)
		திரும்பு 0;

	மொத்தம் = be32_to_cpu(மொத்தம்);
	க்கு (i = 0; i < மொத்தம்; i++) {
		file = malloc(அளவு(*file));
		எனில் (!file) {
			printf("error: allocating resource\n");
			செல் err;
		}
		qfw_read_entry(dev, FW_CFG_INVALID,
			       அளவு(கட்டமைப்பு fw_cfg_file), &file->cfg);
		file->addr = 0;
		list_add_tail(&file->list, &qdev->fw_list);
	}

	திரும்பு 0;

err:
	list_for_each(entry, &qdev->fw_list) {
		file = list_entry(entry, கட்டமைப்பு fw_file, list);
		விடுவி(file);
	}

	திரும்பு -ENOMEM;
}

கட்டமைப்பு fw_file *qfw_find_file(கட்டமைப்பு udevice *dev, மாறிலி எழுத்து *name)
{
	கட்டமைப்பு list_head *entry;
	கட்டமைப்பு fw_file *file;

	கட்டமைப்பு qfw_dev *qdev = dev_get_uclass_priv(dev);

	list_for_each(entry, &qdev->fw_list) {
		file = list_entry(entry, கட்டமைப்பு fw_file, list);
		எனில் (!strcmp(file->cfg.name, name))
			திரும்பு file;
	}

	திரும்பு வெற்று;
}

கட்டமைப்பு fw_file *qfw_file_iter_init(கட்டமைப்பு udevice *dev,
				   கட்டமைப்பு fw_cfg_file_iter *iter)
{
	கட்டமைப்பு qfw_dev *qdev = dev_get_uclass_priv(dev);

	iter->entry = qdev->fw_list.next;
	iter->end = &qdev->fw_list;
	திரும்பு list_entry((கட்டமைப்பு list_head *)iter->entry,
			  கட்டமைப்பு fw_file, list);
}

கட்டமைப்பு fw_file *qfw_file_iter_next(கட்டமைப்பு fw_cfg_file_iter *iter)
{
	iter->entry = ((கட்டமைப்பு list_head *)iter->entry)->next;
	திரும்பு list_entry((கட்டமைப்பு list_head *)iter->entry,
			  கட்டமைப்பு fw_file, list);
}

bool qfw_file_iter_end(கட்டமைப்பு fw_cfg_file_iter *iter)
{
	திரும்பு iter->entry == iter->end;
}

முழு qemu_fwcfg_setup_kernel(கட்டமைப்பு udevice *qfw_dev, ulong load_addr,
			    ulong initrd_addr)
{
	எழுத்து *data_addr;
	u32 setup_size = 0, kernel_size = 0, cmdline_size = 0, initrd_size = 0;

	qfw_read_entry(qfw_dev, FW_CFG_SETUP_SIZE, 4, &setup_size);
	qfw_read_entry(qfw_dev, FW_CFG_KERNEL_SIZE, 4, &kernel_size);

	எனில் (!kernel_size) {
		printf("fatal: no kernel available\n");
		திரும்பு -ENOENT;
	}

	data_addr = map_sysmem(load_addr, 0);
	எனில் (setup_size) {
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
	எனில் (!initrd_size) {
		printf("warning: no initrd available\n");
	} இல்லையெனில் {
		qfw_read_entry(qfw_dev, FW_CFG_INITRD_DATA,
			       le32_to_cpu(initrd_size), data_addr);
		data_addr += le32_to_cpu(initrd_size);
		env_set_hex("filesize", le32_to_cpu(initrd_size));
	}

	qfw_read_entry(qfw_dev, FW_CFG_CMDLINE_SIZE, 4, &cmdline_size);
	எனில் (cmdline_size) {
		qfw_read_entry(qfw_dev, FW_CFG_CMDLINE_DATA,
			       le32_to_cpu(cmdline_size), data_addr);
		/*
		 * if kernel cmdline only contains '\0', (e.g. no -append
		 * when invoking qemu), do not update bootargs
		 */
		எனில் (*data_addr) {
			எனில் (env_set("bootargs", data_addr) < 0)
				printf("warning: unable to change bootargs\n");
		}
	}

	printf("loading kernel to address %lx size %x", load_addr,
	       le32_to_cpu(kernel_size));
	எனில் (initrd_size)
		printf(" initrd %lx size %x\n", initrd_addr,
		       le32_to_cpu(initrd_size));
	இல்லையெனில்
		printf("\n");

	திரும்பு 0;
}
