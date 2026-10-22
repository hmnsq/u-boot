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

цяло qfw_get_dev(struct udevice **devp)
{
	върни uclass_first_device_err(UCLASS_QFW, devp);
}

цяло qfw_online_cpus(struct udevice *dev)
{
	u16 nb_cpus;

	qfw_read_entry(dev, FW_CFG_NB_CPUS, 2, &nb_cpus);

	върни le16_to_cpu(nb_cpus);
}

цяло qfw_read_firmware_list(struct udevice *dev)
{
	цяло i;
	u32 count;
	struct fw_file *file;
	struct list_head *entry;

	struct qfw_dev *qdev = dev_get_uclass_priv(dev);

	/* don't read it twice */
	ако (!list_empty(&qdev->fw_list))
		върни 0;

	qfw_read_entry(dev, FW_CFG_FILE_DIR, 4, &count);
	ако (!count)
		върни 0;

	count = be32_to_cpu(count);
	за (i = 0; i < count; i++) {
		file = malloc(sizeof(*file));
		ако (!file) {
			printf("error: allocating resource\n");
			goto err;
		}
		qfw_read_entry(dev, FW_CFG_INVALID,
			       sizeof(struct fw_cfg_file), &file->cfg);
		file->addr = 0;
		list_add_tail(&file->list, &qdev->fw_list);
	}

	върни 0;

err:
	list_for_each(entry, &qdev->fw_list) {
		file = list_entry(entry, struct fw_file, list);
		free(file);
	}

	върни -ENOMEM;
}

struct fw_file *qfw_find_file(struct udevice *dev, const char *name)
{
	struct list_head *entry;
	struct fw_file *file;

	struct qfw_dev *qdev = dev_get_uclass_priv(dev);

	list_for_each(entry, &qdev->fw_list) {
		file = list_entry(entry, struct fw_file, list);
		ако (!strcmp(file->cfg.name, name))
			върни file;
	}

	върни NULL;
}

struct fw_file *qfw_file_iter_init(struct udevice *dev,
				   struct fw_cfg_file_iter *iter)
{
	struct qfw_dev *qdev = dev_get_uclass_priv(dev);

	iter->entry = qdev->fw_list.next;
	iter->end = &qdev->fw_list;
	върни list_entry((struct list_head *)iter->entry,
			  struct fw_file, list);
}

struct fw_file *qfw_file_iter_next(struct fw_cfg_file_iter *iter)
{
	iter->entry = ((struct list_head *)iter->entry)->next;
	върни list_entry((struct list_head *)iter->entry,
			  struct fw_file, list);
}

bool qfw_file_iter_end(struct fw_cfg_file_iter *iter)
{
	върни iter->entry == iter->end;
}

цяло qemu_fwcfg_setup_kernel(struct udevice *qfw_dev, ulong load_addr,
			    ulong initrd_addr)
{
	char *data_addr;
	u32 setup_size = 0, kernel_size = 0, cmdline_size = 0, initrd_size = 0;

	qfw_read_entry(qfw_dev, FW_CFG_SETUP_SIZE, 4, &setup_size);
	qfw_read_entry(qfw_dev, FW_CFG_KERNEL_SIZE, 4, &kernel_size);

	ако (!kernel_size) {
		printf("fatal: no kernel available\n");
		върни -ENOENT;
	}

	data_addr = map_sysmem(load_addr, 0);
	ако (setup_size) {
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
	ако (!initrd_size) {
		printf("warning: no initrd available\n");
	} иначе {
		qfw_read_entry(qfw_dev, FW_CFG_INITRD_DATA,
			       le32_to_cpu(initrd_size), data_addr);
		data_addr += le32_to_cpu(initrd_size);
		env_set_hex("filesize", le32_to_cpu(initrd_size));
	}

	qfw_read_entry(qfw_dev, FW_CFG_CMDLINE_SIZE, 4, &cmdline_size);
	ако (cmdline_size) {
		qfw_read_entry(qfw_dev, FW_CFG_CMDLINE_DATA,
			       le32_to_cpu(cmdline_size), data_addr);
		/*
		 * if kernel cmdline only contains '\0', (e.g. no -append
		 * when invoking qemu), do not update bootargs
		 */
		ако (*data_addr) {
			ако (env_set("bootargs", data_addr) < 0)
				printf("warning: unable to change bootargs\n");
		}
	}

	printf("loading kernel to address %lx size %x", load_addr,
	       le32_to_cpu(kernel_size));
	ако (initrd_size)
		printf(" initrd %lx size %x\n", initrd_addr,
		       le32_to_cpu(initrd_size));
	иначе
		printf("\n");

	върни 0;
}
