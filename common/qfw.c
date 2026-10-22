// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015 Miao Yan <yanmiaobest@gmail.com>
 * (C) Copyright 2021 Asherah Connor <ashe@kivikakk.ee>
 */

#ضمن <dm.h>
#ضمن <env.h>
#ضمن <mapmem.h>
#ضمن <qfw.h>
#ضمن <stdlib.h>
#ضمن <dm/uclass.h>

صحيح qfw_get_dev(بنية udevice **devp)
{
	החזר uclass_first_device_err(UCLASS_QFW, devp);
}

صحيح qfw_online_cpus(بنية udevice *dev)
{
	u16 nb_cpus;

	qfw_read_entry(dev, FW_CFG_NB_CPUS, 2, &nb_cpus);

	החזר le16_to_cpu(nb_cpus);
}

صحيح qfw_read_firmware_list(بنية udevice *dev)
{
	صحيح i;
	u32 count;
	بنية fw_file *file;
	بنية list_head *entry;

	بنية qfw_dev *qdev = dev_get_uclass_priv(dev);

	/* don't read it twice */
	אם (!list_empty(&qdev->fw_list))
		החזר 0;

	qfw_read_entry(dev, FW_CFG_FILE_DIR, 4, &count);
	אם (!count)
		החזר 0;

	count = be32_to_cpu(count);
	עבור (i = 0; i < count; i++) {
		file = malloc(حجم(*file));
		אם (!file) {
			printf("error: allocating resource\n");
			اذهب err;
		}
		qfw_read_entry(dev, FW_CFG_INVALID,
			       حجم(بنية fw_cfg_file), &file->cfg);
		file->addr = 0;
		list_add_tail(&file->list, &qdev->fw_list);
	}

	החזר 0;

err:
	list_for_each(entry, &qdev->fw_list) {
		file = list_entry(entry, بنية fw_file, list);
		حرر(file);
	}

	החזר -ENOMEM;
}

بنية fw_file *qfw_find_file(بنية udevice *dev, ثابت حرف *name)
{
	بنية list_head *entry;
	بنية fw_file *file;

	بنية qfw_dev *qdev = dev_get_uclass_priv(dev);

	list_for_each(entry, &qdev->fw_list) {
		file = list_entry(entry, بنية fw_file, list);
		אם (!strcmp(file->cfg.name, name))
			החזר file;
	}

	החזר عدم;
}

بنية fw_file *qfw_file_iter_init(بنية udevice *dev,
				   بنية fw_cfg_file_iter *iter)
{
	بنية qfw_dev *qdev = dev_get_uclass_priv(dev);

	iter->entry = qdev->fw_list.next;
	iter->end = &qdev->fw_list;
	החזר list_entry((بنية list_head *)iter->entry,
			  بنية fw_file, list);
}

بنية fw_file *qfw_file_iter_next(بنية fw_cfg_file_iter *iter)
{
	iter->entry = ((بنية list_head *)iter->entry)->next;
	החזר list_entry((بنية list_head *)iter->entry,
			  بنية fw_file, list);
}

bool qfw_file_iter_end(بنية fw_cfg_file_iter *iter)
{
	החזר iter->entry == iter->end;
}

صحيح qemu_fwcfg_setup_kernel(بنية udevice *qfw_dev, ulong load_addr,
			    ulong initrd_addr)
{
	حرف *data_addr;
	u32 setup_size = 0, kernel_size = 0, cmdline_size = 0, initrd_size = 0;

	qfw_read_entry(qfw_dev, FW_CFG_SETUP_SIZE, 4, &setup_size);
	qfw_read_entry(qfw_dev, FW_CFG_KERNEL_SIZE, 4, &kernel_size);

	אם (!kernel_size) {
		printf("fatal: no kernel available\n");
		החזר -ENOENT;
	}

	data_addr = map_sysmem(load_addr, 0);
	אם (setup_size) {
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
	אם (!initrd_size) {
		printf("warning: no initrd available\n");
	} אחרת {
		qfw_read_entry(qfw_dev, FW_CFG_INITRD_DATA,
			       le32_to_cpu(initrd_size), data_addr);
		data_addr += le32_to_cpu(initrd_size);
		env_set_hex("filesize", le32_to_cpu(initrd_size));
	}

	qfw_read_entry(qfw_dev, FW_CFG_CMDLINE_SIZE, 4, &cmdline_size);
	אם (cmdline_size) {
		qfw_read_entry(qfw_dev, FW_CFG_CMDLINE_DATA,
			       le32_to_cpu(cmdline_size), data_addr);
		/*
		 * if kernel cmdline only contains '\0', (e.g. no -append
		 * when invoking qemu), do not update bootargs
		 */
		אם (*data_addr) {
			אם (env_set("bootargs", data_addr) < 0)
				printf("warning: unable to change bootargs\n");
		}
	}

	printf("loading kernel to address %lx size %x", load_addr,
	       le32_to_cpu(kernel_size));
	אם (initrd_size)
		printf(" initrd %lx size %x\n", initrd_addr,
		       le32_to_cpu(initrd_size));
	אחרת
		printf("\n");

	החזר 0;
}
