// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015 Miao Yan <yanmiaobest@gmail.com>
 * (C) Copyright 2021 Asherah Connor <ashe@kivikakk.ee>
 */

#অন্তর্ভাব <dm.h>
#অন্তর্ভাব <env.h>
#অন্তর্ভাব <mapmem.h>
#অন্তর্ভাব <qfw.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <dm/uclass.h>

পূর্ণসংখ্যা qfw_get_dev(কাঠামো udevice **devp)
{
	ফেরত uclass_first_device_err(UCLASS_QFW, devp);
}

পূর্ণসংখ্যা qfw_online_cpus(কাঠামো udevice *dev)
{
	u16 nb_cpus;

	qfw_read_entry(dev, FW_CFG_NB_CPUS, 2, &nb_cpus);

	ফেরত le16_to_cpu(nb_cpus);
}

পূর্ণসংখ্যা qfw_read_firmware_list(কাঠামো udevice *dev)
{
	পূর্ণসংখ্যা i;
	u32 গুনতি;
	কাঠামো fw_file *file;
	কাঠামো list_শীর্ষ *entry;

	কাঠামো qfw_dev *qdev = dev_get_uclass_priv(dev);

	/* don't read it twice */
	যদি (!list_empty(&qdev->fw_list))
		ফেরত 0;

	qfw_read_entry(dev, FW_CFG_FILE_DIR, 4, &গুনতি);
	যদি (!গুনতি)
		ফেরত 0;

	গুনতি = be32_to_cpu(গুনতি);
	জন্য (i = 0; i < গুনতি; i++) {
		file = malloc(আকার(*file));
		যদি (!file) {
			printf("error: allocating resource\n");
			যাও err;
		}
		qfw_read_entry(dev, FW_CFG_INVALID,
			       আকার(কাঠামো fw_cfg_file), &file->cfg);
		file->addr = 0;
		list_add_লেজ(&file->list, &qdev->fw_list);
	}

	ফেরত 0;

err:
	list_for_each(entry, &qdev->fw_list) {
		file = list_entry(entry, কাঠামো fw_file, list);
		মুক্ত(file);
	}

	ফেরত -ENOMEM;
}

কাঠামো fw_file *qfw_find_file(কাঠামো udevice *dev, ধ্রুবক অক্ষর *name)
{
	কাঠামো list_শীর্ষ *entry;
	কাঠামো fw_file *file;

	কাঠামো qfw_dev *qdev = dev_get_uclass_priv(dev);

	list_for_each(entry, &qdev->fw_list) {
		file = list_entry(entry, কাঠামো fw_file, list);
		যদি (!strcmp(file->cfg.name, name))
			ফেরত file;
	}

	ফেরত ফাঁকা;
}

কাঠামো fw_file *qfw_file_iter_init(কাঠামো udevice *dev,
				   কাঠামো fw_cfg_file_iter *iter)
{
	কাঠামো qfw_dev *qdev = dev_get_uclass_priv(dev);

	iter->entry = qdev->fw_list.পরবর্তী;
	iter->end = &qdev->fw_list;
	ফেরত list_entry((কাঠামো list_শীর্ষ *)iter->entry,
			  কাঠামো fw_file, list);
}

কাঠামো fw_file *qfw_file_iter_পরবর্তী(কাঠামো fw_cfg_file_iter *iter)
{
	iter->entry = ((কাঠামো list_শীর্ষ *)iter->entry)->পরবর্তী;
	ফেরত list_entry((কাঠামো list_শীর্ষ *)iter->entry,
			  কাঠামো fw_file, list);
}

bool qfw_file_iter_end(কাঠামো fw_cfg_file_iter *iter)
{
	ফেরত iter->entry == iter->end;
}

পূর্ণসংখ্যা qemu_fwcfg_setup_kernel(কাঠামো udevice *qfw_dev, ulong load_addr,
			    ulong initrd_addr)
{
	অক্ষর *তথ্য_addr;
	u32 setup_আকার = 0, kernel_আকার = 0, cmdline_আকার = 0, initrd_আকার = 0;

	qfw_read_entry(qfw_dev, FW_CFG_SETUP_আকার, 4, &setup_আকার);
	qfw_read_entry(qfw_dev, FW_CFG_KERNEL_আকার, 4, &kernel_আকার);

	যদি (!kernel_আকার) {
		printf("fatal: no kernel available\n");
		ফেরত -ENOENT;
	}

	তথ্য_addr = map_sysmem(load_addr, 0);
	যদি (setup_আকার) {
		qfw_read_entry(qfw_dev, FW_CFG_SETUP_তথ্য,
			       le32_to_cpu(setup_আকার), তথ্য_addr);
		তথ্য_addr += le32_to_cpu(setup_আকার);
	}

	qfw_read_entry(qfw_dev, FW_CFG_KERNEL_তথ্য,
		       le32_to_cpu(kernel_আকার), তথ্য_addr);
	তথ্য_addr += le32_to_cpu(kernel_আকার);
	env_set_hex("filesize", le32_to_cpu(kernel_আকার));

	তথ্য_addr = map_sysmem(initrd_addr, 0);
	qfw_read_entry(qfw_dev, FW_CFG_INITRD_আকার, 4, &initrd_আকার);
	যদি (!initrd_আকার) {
		printf("warning: no initrd available\n");
	} অন্যথা {
		qfw_read_entry(qfw_dev, FW_CFG_INITRD_তথ্য,
			       le32_to_cpu(initrd_আকার), তথ্য_addr);
		তথ্য_addr += le32_to_cpu(initrd_আকার);
		env_set_hex("filesize", le32_to_cpu(initrd_আকার));
	}

	qfw_read_entry(qfw_dev, FW_CFG_CMDLINE_আকার, 4, &cmdline_আকার);
	যদি (cmdline_আকার) {
		qfw_read_entry(qfw_dev, FW_CFG_CMDLINE_তথ্য,
			       le32_to_cpu(cmdline_আকার), তথ্য_addr);
		/*
		 * if kernel cmdline only contains '\0', (e.g. no -append
		 * when invoking qemu), do not update bootargs
		 */
		যদি (*তথ্য_addr) {
			যদি (env_set("bootargs", তথ্য_addr) < 0)
				printf("warning: unable to change bootargs\n");
		}
	}

	printf("loading kernel to address %lx size %x", load_addr,
	       le32_to_cpu(kernel_আকার));
	যদি (initrd_আকার)
		printf(" initrd %lx size %x\n", initrd_addr,
		       le32_to_cpu(initrd_আকার));
	অন্যথা
		printf("\n");

	ফেরত 0;
}
