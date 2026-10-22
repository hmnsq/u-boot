// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015 Miao Yan <yanmiaobest@gmail.com>
 * (C) Copyright 2021 Asherah Connor <ashe@kivikakk.ee>
 */

#अन्तर्भाव <dm.h>
#अन्तर्भाव <env.h>
#अन्तर्भाव <mapmem.h>
#अन्तर्भाव <qfw.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <dm/uclass.h>

पूर्णांक qfw_get_dev(संरचना udevice **devp)
{
	परत uclass_first_device_err(UCLASS_QFW, devp);
}

पूर्णांक qfw_online_cpus(संरचना udevice *dev)
{
	u16 nb_cpus;

	qfw_read_entry(dev, FW_CFG_NB_CPUS, 2, &nb_cpus);

	परत le16_to_cpu(nb_cpus);
}

पूर्णांक qfw_read_firmware_list(संरचना udevice *dev)
{
	पूर्णांक i;
	u32 मोजणी;
	संरचना fw_file *file;
	संरचना list_head *entry;

	संरचना qfw_dev *qdev = dev_get_uclass_priv(dev);

	/* don't read it twice */
	जर (!list_empty(&qdev->fw_list))
		परत 0;

	qfw_read_entry(dev, FW_CFG_FILE_DIR, 4, &मोजणी);
	जर (!मोजणी)
		परत 0;

	मोजणी = be32_to_cpu(मोजणी);
	साठी (i = 0; i < मोजणी; i++) {
		file = दो_स्मृति(आकार(*file));
		जर (!file) {
			म_लिखो("error: allocating resource\n");
			जा err;
		}
		qfw_read_entry(dev, FW_CFG_INVALID,
			       आकार(संरचना fw_cfg_file), &file->cfg);
		file->addr = 0;
		list_add_tail(&file->list, &qdev->fw_list);
	}

	परत 0;

err:
	list_for_each(entry, &qdev->fw_list) {
		file = list_entry(entry, संरचना fw_file, list);
		मुक्त(file);
	}

	परत -ENOMEM;
}

संरचना fw_file *qfw_find_file(संरचना udevice *dev, स्थिरांक अक्षर *name)
{
	संरचना list_head *entry;
	संरचना fw_file *file;

	संरचना qfw_dev *qdev = dev_get_uclass_priv(dev);

	list_for_each(entry, &qdev->fw_list) {
		file = list_entry(entry, संरचना fw_file, list);
		जर (!म_भेद(file->cfg.name, name))
			परत file;
	}

	परत रिक्त;
}

संरचना fw_file *qfw_file_iter_init(संरचना udevice *dev,
				   संरचना fw_cfg_file_iter *iter)
{
	संरचना qfw_dev *qdev = dev_get_uclass_priv(dev);

	iter->entry = qdev->fw_list.next;
	iter->end = &qdev->fw_list;
	परत list_entry((संरचना list_head *)iter->entry,
			  संरचना fw_file, list);
}

संरचना fw_file *qfw_file_iter_next(संरचना fw_cfg_file_iter *iter)
{
	iter->entry = ((संरचना list_head *)iter->entry)->next;
	परत list_entry((संरचना list_head *)iter->entry,
			  संरचना fw_file, list);
}

bool qfw_file_iter_end(संरचना fw_cfg_file_iter *iter)
{
	परत iter->entry == iter->end;
}

पूर्णांक qemu_fwcfg_setup_kernel(संरचना udevice *qfw_dev, ulong load_addr,
			    ulong initrd_addr)
{
	अक्षर *data_addr;
	u32 setup_size = 0, kernel_size = 0, cmdline_size = 0, initrd_size = 0;

	qfw_read_entry(qfw_dev, FW_CFG_SETUP_SIZE, 4, &setup_size);
	qfw_read_entry(qfw_dev, FW_CFG_KERNEL_SIZE, 4, &kernel_size);

	जर (!kernel_size) {
		म_लिखो("fatal: no kernel available\n");
		परत -ENOENT;
	}

	data_addr = map_sysmem(load_addr, 0);
	जर (setup_size) {
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
	जर (!initrd_size) {
		म_लिखो("warning: no initrd available\n");
	} अन्यथा {
		qfw_read_entry(qfw_dev, FW_CFG_INITRD_DATA,
			       le32_to_cpu(initrd_size), data_addr);
		data_addr += le32_to_cpu(initrd_size);
		env_set_hex("filesize", le32_to_cpu(initrd_size));
	}

	qfw_read_entry(qfw_dev, FW_CFG_CMDLINE_SIZE, 4, &cmdline_size);
	जर (cmdline_size) {
		qfw_read_entry(qfw_dev, FW_CFG_CMDLINE_DATA,
			       le32_to_cpu(cmdline_size), data_addr);
		/*
		 * if kernel cmdline only contains '\0', (e.g. no -append
		 * when invoking qemu), do not update bootargs
		 */
		जर (*data_addr) {
			जर (env_set("bootargs", data_addr) < 0)
				म_लिखो("warning: unable to change bootargs\n");
		}
	}

	म_लिखो("loading kernel to address %lx size %x", load_addr,
	       le32_to_cpu(kernel_size));
	जर (initrd_size)
		म_लिखो(" initrd %lx size %x\n", initrd_addr,
		       le32_to_cpu(initrd_size));
	अन्यथा
		म_लिखो("\n");

	परत 0;
}
