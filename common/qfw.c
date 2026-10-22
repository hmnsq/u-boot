// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015 Miao Yan <yanmiaobest@gmail.com>
 * (C) Copyright 2021 Asherah Connor <ashe@kivikakk.ee>
 */

#ଅନ୍ତର୍ଭାଵ <dm.h>
#ଅନ୍ତର୍ଭାଵ <env.h>
#ଅନ୍ତର୍ଭାଵ <mapmem.h>
#ଅନ୍ତର୍ଭାଵ <qfw.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <dm/uclass.h>

ପୂର୍ଣ୍ଣସଂଖ୍ୟା qfw_get_dev(ସଂରଚନା udevice **devp)
{
	ଫେରନ୍ତୁ uclass_first_device_err(UCLASS_QFW, devp);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା qfw_online_cpus(ସଂରଚନା udevice *dev)
{
	u16 nb_cpus;

	qfw_read_entry(dev, FW_CFG_NB_CPUS, 2, &nb_cpus);

	ଫେରନ୍ତୁ le16_to_cpu(nb_cpus);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା qfw_read_firmware_list(ସଂରଚନା udevice *dev)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	u32 ଗଣନା_2;
	ସଂରଚନା fw_file *file;
	ସଂରଚନା list_head *entry;

	ସଂରଚନା qfw_dev *qdev = dev_get_uclass_priv(dev);

	/* don't read it twice */
	ଯଦି (!list_empty(&qdev->fw_list))
		ଫେରନ୍ତୁ 0;

	qfw_read_entry(dev, FW_CFG_FILE_DIR, 4, &ଗଣନା_2);
	ଯଦି (!ଗଣନା_2)
		ଫେରନ୍ତୁ 0;

	ଗଣନା_2 = be32_to_cpu(ଗଣନା_2);
	ପାଇଁ (i = 0; i < ଗଣନା_2; i++) {
		file = malloc(ଆକାର(*file));
		ଯଦି (!file) {
			printf("error: allocating resource\n");
			ଯାଅ err;
		}
		qfw_read_entry(dev, FW_CFG_INVALID,
			       ଆକାର(ସଂରଚନା fw_cfg_file), &file->cfg);
		file->addr = 0;
		list_add_tail(&file->list, &qdev->fw_list);
	}

	ଫେରନ୍ତୁ 0;

err:
	list_for_each(entry, &qdev->fw_list) {
		file = list_entry(entry, ସଂରଚନା fw_file, list);
		ମୁକ୍ତ(file);
	}

	ଫେରନ୍ତୁ -ENOMEM;
}

ସଂରଚନା fw_file *qfw_find_file(ସଂରଚନା udevice *dev, ଅଚଳ ଅକ୍ଷର *name)
{
	ସଂରଚନା list_head *entry;
	ସଂରଚନା fw_file *file;

	ସଂରଚନା qfw_dev *qdev = dev_get_uclass_priv(dev);

	list_for_each(entry, &qdev->fw_list) {
		file = list_entry(entry, ସଂରଚନା fw_file, list);
		ଯଦି (!strcmp(file->cfg.name, name))
			ଫେରନ୍ତୁ file;
	}

	ଫେରନ୍ତୁ ଖାଲି;
}

ସଂରଚନା fw_file *qfw_file_iter_init(ସଂରଚନା udevice *dev,
				   ସଂରଚନା fw_cfg_file_iter *iter)
{
	ସଂରଚନା qfw_dev *qdev = dev_get_uclass_priv(dev);

	iter->entry = qdev->fw_list.next;
	iter->end = &qdev->fw_list;
	ଫେରନ୍ତୁ list_entry((ସଂରଚନା list_head *)iter->entry,
			  ସଂରଚନା fw_file, list);
}

ସଂରଚନା fw_file *qfw_file_iter_next(ସଂରଚନା fw_cfg_file_iter *iter)
{
	iter->entry = ((ସଂରଚନା list_head *)iter->entry)->next;
	ଫେରନ୍ତୁ list_entry((ସଂରଚନା list_head *)iter->entry,
			  ସଂରଚନା fw_file, list);
}

bool qfw_file_iter_end(ସଂରଚନା fw_cfg_file_iter *iter)
{
	ଫେରନ୍ତୁ iter->entry == iter->end;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା qemu_fwcfg_setup_kernel(ସଂରଚନା udevice *qfw_dev, ulong load_addr,
			    ulong initrd_addr)
{
	ଅକ୍ଷର *data_addr;
	u32 setup_size = 0, kernel_size = 0, cmdline_size = 0, initrd_size = 0;

	qfw_read_entry(qfw_dev, FW_CFG_SETUP_SIZE, 4, &setup_size);
	qfw_read_entry(qfw_dev, FW_CFG_KERNEL_SIZE, 4, &kernel_size);

	ଯଦି (!kernel_size) {
		printf("fatal: no kernel available\n");
		ଫେରନ୍ତୁ -ENOENT;
	}

	data_addr = map_sysmem(load_addr, 0);
	ଯଦି (setup_size) {
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
	ଯଦି (!initrd_size) {
		printf("warning: no initrd available\n");
	} ଅନ୍ୟଥା {
		qfw_read_entry(qfw_dev, FW_CFG_INITRD_DATA,
			       le32_to_cpu(initrd_size), data_addr);
		data_addr += le32_to_cpu(initrd_size);
		env_set_hex("filesize", le32_to_cpu(initrd_size));
	}

	qfw_read_entry(qfw_dev, FW_CFG_CMDLINE_SIZE, 4, &cmdline_size);
	ଯଦି (cmdline_size) {
		qfw_read_entry(qfw_dev, FW_CFG_CMDLINE_DATA,
			       le32_to_cpu(cmdline_size), data_addr);
		/*
		 * if kernel cmdline only contains '\0', (e.g. no -append
		 * when invoking qemu), do not update bootargs
		 */
		ଯଦି (*data_addr) {
			ଯଦି (env_set("bootargs", data_addr) < 0)
				printf("warning: unable to change bootargs\n");
		}
	}

	printf("loading kernel to address %lx size %x", load_addr,
	       le32_to_cpu(kernel_size));
	ଯଦି (initrd_size)
		printf(" initrd %lx size %x\n", initrd_addr,
		       le32_to_cpu(initrd_size));
	ଅନ୍ୟଥା
		printf("\n");

	ଫେରନ୍ତୁ 0;
}
