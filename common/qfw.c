// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015 Miao Yan <yanmiaobest@gmail.com>
 * (C) Copyright 2021 Asherah Connor <ashe@kivikakk.ee>
 */

#समावेश <dm.h>
#समावेश <env.h>
#समावेश <mapmem.h>
#समावेश <qfw.h>
#समावेश <stdlib.h>
#समावेश <dm/uclass.h>

पूर्णांक qfw_get_dev(काष्ठा udevice **devp)
{
	वापस uclass_first_device_err(UCLASS_QFW, devp);
}

पूर्णांक qfw_online_cpus(काष्ठा udevice *dev)
{
	u16 nb_cpus;

	qfw_read_entry(dev, FW_CFG_NB_CPUS, 2, &nb_cpus);

	वापस le16_to_cpu(nb_cpus);
}

पूर्णांक qfw_read_firmware_list(काष्ठा udevice *dev)
{
	पूर्णांक i;
	u32 गिनती;
	काष्ठा fw_file *file;
	काष्ठा list_शीर्ष *entry;

	काष्ठा qfw_dev *qdev = dev_get_uclass_priv(dev);

	/* don't read it twice */
	अगर (!list_empty(&qdev->fw_list))
		वापस 0;

	qfw_read_entry(dev, FW_CFG_FILE_DIR, 4, &गिनती);
	अगर (!गिनती)
		वापस 0;

	गिनती = be32_to_cpu(गिनती);
	क्रम (i = 0; i < गिनती; i++) {
		file = दो_स्मृति(माप(*file));
		अगर (!file) {
			म_लिखो("error: allocating resource\n");
			जाओ err;
		}
		qfw_read_entry(dev, FW_CFG_INVALID,
			       माप(काष्ठा fw_cfg_file), &file->cfg);
		file->addr = 0;
		list_add_पुच्छ(&file->list, &qdev->fw_list);
	}

	वापस 0;

err:
	list_for_each(entry, &qdev->fw_list) {
		file = list_entry(entry, काष्ठा fw_file, list);
		मुक्त(file);
	}

	वापस -ENOMEM;
}

काष्ठा fw_file *qfw_find_file(काष्ठा udevice *dev, स्थिर अक्षर *name)
{
	काष्ठा list_शीर्ष *entry;
	काष्ठा fw_file *file;

	काष्ठा qfw_dev *qdev = dev_get_uclass_priv(dev);

	list_for_each(entry, &qdev->fw_list) {
		file = list_entry(entry, काष्ठा fw_file, list);
		अगर (!म_भेद(file->cfg.name, name))
			वापस file;
	}

	वापस शून्य;
}

काष्ठा fw_file *qfw_file_iter_init(काष्ठा udevice *dev,
				   काष्ठा fw_cfg_file_iter *iter)
{
	काष्ठा qfw_dev *qdev = dev_get_uclass_priv(dev);

	iter->entry = qdev->fw_list.अग्र;
	iter->end = &qdev->fw_list;
	वापस list_entry((काष्ठा list_शीर्ष *)iter->entry,
			  काष्ठा fw_file, list);
}

काष्ठा fw_file *qfw_file_iter_अग्र(काष्ठा fw_cfg_file_iter *iter)
{
	iter->entry = ((काष्ठा list_शीर्ष *)iter->entry)->अग्र;
	वापस list_entry((काष्ठा list_शीर्ष *)iter->entry,
			  काष्ठा fw_file, list);
}

bool qfw_file_iter_end(काष्ठा fw_cfg_file_iter *iter)
{
	वापस iter->entry == iter->end;
}

पूर्णांक qemu_fwcfg_setup_kernel(काष्ठा udevice *qfw_dev, ulong load_addr,
			    ulong initrd_addr)
{
	अक्षर *तथ्य_addr;
	u32 setup_आकार = 0, kernel_आकार = 0, cmdline_आकार = 0, initrd_आकार = 0;

	qfw_read_entry(qfw_dev, FW_CFG_SETUP_आकार, 4, &setup_आकार);
	qfw_read_entry(qfw_dev, FW_CFG_KERNEL_आकार, 4, &kernel_आकार);

	अगर (!kernel_आकार) {
		म_लिखो("fatal: no kernel available\n");
		वापस -ENOENT;
	}

	तथ्य_addr = map_sysmem(load_addr, 0);
	अगर (setup_आकार) {
		qfw_read_entry(qfw_dev, FW_CFG_SETUP_तथ्य,
			       le32_to_cpu(setup_आकार), तथ्य_addr);
		तथ्य_addr += le32_to_cpu(setup_आकार);
	}

	qfw_read_entry(qfw_dev, FW_CFG_KERNEL_तथ्य,
		       le32_to_cpu(kernel_आकार), तथ्य_addr);
	तथ्य_addr += le32_to_cpu(kernel_आकार);
	env_set_hex("filesize", le32_to_cpu(kernel_आकार));

	तथ्य_addr = map_sysmem(initrd_addr, 0);
	qfw_read_entry(qfw_dev, FW_CFG_INITRD_आकार, 4, &initrd_आकार);
	अगर (!initrd_आकार) {
		म_लिखो("warning: no initrd available\n");
	} अन्यथा {
		qfw_read_entry(qfw_dev, FW_CFG_INITRD_तथ्य,
			       le32_to_cpu(initrd_आकार), तथ्य_addr);
		तथ्य_addr += le32_to_cpu(initrd_आकार);
		env_set_hex("filesize", le32_to_cpu(initrd_आकार));
	}

	qfw_read_entry(qfw_dev, FW_CFG_CMDLINE_आकार, 4, &cmdline_आकार);
	अगर (cmdline_आकार) {
		qfw_read_entry(qfw_dev, FW_CFG_CMDLINE_तथ्य,
			       le32_to_cpu(cmdline_आकार), तथ्य_addr);
		/*
		 * if kernel cmdline only contains '\0', (e.g. no -append
		 * when invoking qemu), do not update bootargs
		 */
		अगर (*तथ्य_addr) {
			अगर (env_set("bootargs", तथ्य_addr) < 0)
				म_लिखो("warning: unable to change bootargs\n");
		}
	}

	म_लिखो("loading kernel to address %lx size %x", load_addr,
	       le32_to_cpu(kernel_आकार));
	अगर (initrd_आकार)
		म_लिखो(" initrd %lx size %x\n", initrd_addr,
		       le32_to_cpu(initrd_आकार));
	अन्यथा
		म_लिखो("\n");

	वापस 0;
}
