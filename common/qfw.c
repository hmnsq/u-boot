// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015 Miao Yan <yanmiaobest@gmail.com>
 * (C) Copyright 2021 Asherah Connor <ashe@kivikakk.ee>
 */

#include <dm.h>
#include <env.h>
#include <mapmem.h>
#include <qfw.h>
#include <stdlib.h>
#include <dm/uclass.h>

පූර්ණ_සංඛ්‍යා qfw_get_dev(sa_nracanaa udevice **devp)
{
	ආපසු uclass_first_device_err(UCLASS_QFW, devp);
}

පූර්ණ_සංඛ්‍යා qfw_online_cpus(sa_nracanaa udevice *dev)
{
	u16 nb_cpus;

	qfw_read_entry(dev, FW_CFG_NB_CPUS, 2, &nb_cpus);

	ආපසු le16_to_cpu(nb_cpus);
}

පූර්ණ_සංඛ්‍යා qfw_read_firmware_list(sa_nracanaa udevice *dev)
{
	පූර්ණ_සංඛ්‍යා i;
	u32 count;
	sa_nracanaa fw_file *file;
	sa_nracanaa list_head *entry;

	sa_nracanaa qfw_dev *qdev = dev_get_uclass_priv(dev);

	/* don't read it twice */
	නම් (!list_empty(&qdev->fw_list))
		ආපසු 0;

	qfw_read_entry(dev, FW_CFG_FILE_DIR, 4, &count);
	නම් (!count)
		ආපසු 0;

	count = be32_to_cpu(count);
	සඳහා (i = 0; i < count; i++) {
		file = malloc(parimaa_na(*file));
		නම් (!file) {
			printf("error: allocating resource\n");
			goto err;
		}
		qfw_read_entry(dev, FW_CFG_INVALID,
			       parimaa_na(sa_nracanaa fw_cfg_file), &file->cfg);
		file->addr = 0;
		list_add_tail(&file->list, &qdev->fw_list);
	}

	ආපසු 0;

err:
	list_for_each(entry, &qdev->fw_list) {
		file = list_entry(entry, sa_nracanaa fw_file, list);
		free(file);
	}

	ආපසු -ENOMEM;
}

sa_nracanaa fw_file *qfw_find_file(sa_nracanaa udevice *dev, const var_na *name)
{
	sa_nracanaa list_head *entry;
	sa_nracanaa fw_file *file;

	sa_nracanaa qfw_dev *qdev = dev_get_uclass_priv(dev);

	list_for_each(entry, &qdev->fw_list) {
		file = list_entry(entry, sa_nracanaa fw_file, list);
		නම් (!strcmp(file->cfg.name, name))
			ආපසු file;
	}

	ආපසු NULL;
}

sa_nracanaa fw_file *qfw_file_iter_init(sa_nracanaa udevice *dev,
				   sa_nracanaa fw_cfg_file_iter *iter)
{
	sa_nracanaa qfw_dev *qdev = dev_get_uclass_priv(dev);

	iter->entry = qdev->fw_list.next;
	iter->end = &qdev->fw_list;
	ආපසු list_entry((sa_nracanaa list_head *)iter->entry,
			  sa_nracanaa fw_file, list);
}

sa_nracanaa fw_file *qfw_file_iter_next(sa_nracanaa fw_cfg_file_iter *iter)
{
	iter->entry = ((sa_nracanaa list_head *)iter->entry)->next;
	ආපසු list_entry((sa_nracanaa list_head *)iter->entry,
			  sa_nracanaa fw_file, list);
}

bool qfw_file_iter_end(sa_nracanaa fw_cfg_file_iter *iter)
{
	ආපසු iter->entry == iter->end;
}

පූර්ණ_සංඛ්‍යා qemu_fwcfg_setup_kernel(sa_nracanaa udevice *qfw_dev, ulong load_addr,
			    ulong initrd_addr)
{
	var_na *data_addr;
	u32 setup_size = 0, kernel_size = 0, cmdline_size = 0, initrd_size = 0;

	qfw_read_entry(qfw_dev, FW_CFG_SETUP_SIZE, 4, &setup_size);
	qfw_read_entry(qfw_dev, FW_CFG_KERNEL_SIZE, 4, &kernel_size);

	නම් (!kernel_size) {
		printf("fatal: no kernel available\n");
		ආපසු -ENOENT;
	}

	data_addr = map_sysmem(load_addr, 0);
	නම් (setup_size) {
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
	නම් (!initrd_size) {
		printf("warning: no initrd available\n");
	} නැතහොත් {
		qfw_read_entry(qfw_dev, FW_CFG_INITRD_DATA,
			       le32_to_cpu(initrd_size), data_addr);
		data_addr += le32_to_cpu(initrd_size);
		env_set_hex("filesize", le32_to_cpu(initrd_size));
	}

	qfw_read_entry(qfw_dev, FW_CFG_CMDLINE_SIZE, 4, &cmdline_size);
	නම් (cmdline_size) {
		qfw_read_entry(qfw_dev, FW_CFG_CMDLINE_DATA,
			       le32_to_cpu(cmdline_size), data_addr);
		/*
		 * if kernel cmdline only contains '\0', (e.g. no -append
		 * when invoking qemu), do not update bootargs
		 */
		නම් (*data_addr) {
			නම් (env_set("bootargs", data_addr) < 0)
				printf("warning: unable to change bootargs\n");
		}
	}

	printf("loading kernel to address %lx size %x", load_addr,
	       le32_to_cpu(kernel_size));
	නම් (initrd_size)
		printf(" initrd %lx size %x\n", initrd_addr,
		       le32_to_cpu(initrd_size));
	නැතහොත්
		printf("\n");

	ආපසු 0;
}
