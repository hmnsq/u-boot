// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015 Miao Yan <yanmiaobest@gmail.com>
 * (C) Copyright 2021 Asherah Connor <ashe@kivikakk.ee>
 */

#અન્તર્ભાવ <dm.h>
#અન્તર્ભાવ <env.h>
#અન્તર્ભાવ <mapmem.h>
#અન્તર્ભાવ <qfw.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <dm/uclass.h>

પૂર્ણાંક qfw_get_dev(સંરચના udevice **devp)
{
	પાછા uclass_first_device_err(UCLASS_QFW, devp);
}

પૂર્ણાંક qfw_online_cpus(સંરચના udevice *dev)
{
	u16 nb_cpus;

	qfw_read_entry(dev, FW_CFG_NB_CPUS, 2, &nb_cpus);

	પાછા le16_to_cpu(nb_cpus);
}

પૂર્ણાંક qfw_read_firmware_list(સંરચના udevice *dev)
{
	પૂર્ણાંક i;
	u32 ગણતરી;
	સંરચના fw_file *file;
	સંરચના list_head *entry;

	સંરચના qfw_dev *qdev = dev_get_uclass_priv(dev);

	/* don't read it twice */
	જો (!list_empty(&qdev->fw_list))
		પાછા 0;

	qfw_read_entry(dev, FW_CFG_FILE_DIR, 4, &ગણતરી);
	જો (!ગણતરી)
		પાછા 0;

	ગણતરી = be32_to_cpu(ગણતરી);
	માટે (i = 0; i < ગણતરી; i++) {
		file = malloc(કદ(*file));
		જો (!file) {
			printf("error: allocating resource\n");
			જાઓ err;
		}
		qfw_read_entry(dev, FW_CFG_INVALID,
			       કદ(સંરચના fw_cfg_file), &file->cfg);
		file->addr = 0;
		list_add_tail(&file->list, &qdev->fw_list);
	}

	પાછા 0;

err:
	list_for_each(entry, &qdev->fw_list) {
		file = list_entry(entry, સંરચના fw_file, list);
		મુક્ત(file);
	}

	પાછા -ENOMEM;
}

સંરચના fw_file *qfw_find_file(સંરચના udevice *dev, અચલ અક્ષર *name)
{
	સંરચના list_head *entry;
	સંરચના fw_file *file;

	સંરચના qfw_dev *qdev = dev_get_uclass_priv(dev);

	list_for_each(entry, &qdev->fw_list) {
		file = list_entry(entry, સંરચના fw_file, list);
		જો (!strcmp(file->cfg.name, name))
			પાછા file;
	}

	પાછા ખાલી;
}

સંરચના fw_file *qfw_file_iter_init(સંરચના udevice *dev,
				   સંરચના fw_cfg_file_iter *iter)
{
	સંરચના qfw_dev *qdev = dev_get_uclass_priv(dev);

	iter->entry = qdev->fw_list.next;
	iter->end = &qdev->fw_list;
	પાછા list_entry((સંરચના list_head *)iter->entry,
			  સંરચના fw_file, list);
}

સંરચના fw_file *qfw_file_iter_next(સંરચના fw_cfg_file_iter *iter)
{
	iter->entry = ((સંરચના list_head *)iter->entry)->next;
	પાછા list_entry((સંરચના list_head *)iter->entry,
			  સંરચના fw_file, list);
}

bool qfw_file_iter_end(સંરચના fw_cfg_file_iter *iter)
{
	પાછા iter->entry == iter->end;
}

પૂર્ણાંક qemu_fwcfg_setup_kernel(સંરચના udevice *qfw_dev, ulong load_addr,
			    ulong initrd_addr)
{
	અક્ષર *data_addr;
	u32 setup_size = 0, kernel_size = 0, cmdline_size = 0, initrd_size = 0;

	qfw_read_entry(qfw_dev, FW_CFG_SETUP_SIZE, 4, &setup_size);
	qfw_read_entry(qfw_dev, FW_CFG_KERNEL_SIZE, 4, &kernel_size);

	જો (!kernel_size) {
		printf("fatal: no kernel available\n");
		પાછા -ENOENT;
	}

	data_addr = map_sysmem(load_addr, 0);
	જો (setup_size) {
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
	જો (!initrd_size) {
		printf("warning: no initrd available\n");
	} નહીંતર {
		qfw_read_entry(qfw_dev, FW_CFG_INITRD_DATA,
			       le32_to_cpu(initrd_size), data_addr);
		data_addr += le32_to_cpu(initrd_size);
		env_set_hex("filesize", le32_to_cpu(initrd_size));
	}

	qfw_read_entry(qfw_dev, FW_CFG_CMDLINE_SIZE, 4, &cmdline_size);
	જો (cmdline_size) {
		qfw_read_entry(qfw_dev, FW_CFG_CMDLINE_DATA,
			       le32_to_cpu(cmdline_size), data_addr);
		/*
		 * if kernel cmdline only contains '\0', (e.g. no -append
		 * when invoking qemu), do not update bootargs
		 */
		જો (*data_addr) {
			જો (env_set("bootargs", data_addr) < 0)
				printf("warning: unable to change bootargs\n");
		}
	}

	printf("loading kernel to address %lx size %x", load_addr,
	       le32_to_cpu(kernel_size));
	જો (initrd_size)
		printf(" initrd %lx size %x\n", initrd_addr,
		       le32_to_cpu(initrd_size));
	નહીંતર
		printf("\n");

	પાછા 0;
}
