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

पूर्णाङ्क qfw_get_dev(संरचना udevice **devp)
{
	प्रत्यागम uclass_first_device_err(UCLASS_QFW, devp);
}

पूर्णाङ्क qfw_online_cpus(संरचना udevice *dev)
{
	u16 nb_cpus;

	qfw_read_प्रविष्टिः(dev, FW_CFG_NB_CPUS, 2, &nb_cpus);

	प्रत्यागम le16_गन्तव्यम्_cpu(nb_cpus);
}

पूर्णाङ्क qfw_read_firmware_list(संरचना udevice *dev)
{
	पूर्णाङ्क i;
	u32 गणनम्;
	संरचना fw_file *file;
	संरचना list_शीर्ष *प्रविष्टिः;

	संरचना qfw_dev *qdev = dev_get_uclass_priv(dev);

	/* don't read it twice */
	यदि (!list_रिक्तानि(&qdev->fw_list))
		प्रत्यागम 0;

	qfw_read_प्रविष्टिः(dev, FW_CFG_FILE_DIR, 4, &गणनम्);
	यदि (!गणनम्)
		प्रत्यागम 0;

	गणनम् = be32_गन्तव्यम्_cpu(गणनम्);
	प्रति (i = 0; i < गणनम्; i++) {
		file = स्मृतिं_देहि(परिमाण(*file));
		यदि (!file) {
			प्रारूप_लिख("error: allocating resource\n");
			गच्छ err;
		}
		qfw_read_प्रविष्टिः(dev, FW_CFG_INVALID,
			       परिमाण(संरचना fw_cfg_file), &file->cfg);
		file->addr = 0;
		list_सङ्कलय_पुच्छ(&file->list, &qdev->fw_list);
	}

	प्रत्यागम 0;

err:
	list_for_each(प्रविष्टिः, &qdev->fw_list) {
		file = list_प्रविष्टिः(प्रविष्टिः, संरचना fw_file, list);
		मुक्त(file);
	}

	प्रत्यागम -ENOMEM;
}

संरचना fw_file *qfw_find_file(संरचना udevice *dev, ध्रुव वर्ण *name)
{
	संरचना list_शीर्ष *प्रविष्टिः;
	संरचना fw_file *file;

	संरचना qfw_dev *qdev = dev_get_uclass_priv(dev);

	list_for_each(प्रविष्टिः, &qdev->fw_list) {
		file = list_प्रविष्टिः(प्रविष्टिः, संरचना fw_file, list);
		यदि (!वाक्यं_तुलय(file->cfg.name, name))
			प्रत्यागम file;
	}

	प्रत्यागम रिक्त;
}

संरचना fw_file *qfw_file_iter_init(संरचना udevice *dev,
				   संरचना fw_cfg_file_iter *iter)
{
	संरचना qfw_dev *qdev = dev_get_uclass_priv(dev);

	iter->प्रविष्टिः = qdev->fw_list.अग्र;
	iter->अन्तः_2 = &qdev->fw_list;
	प्रत्यागम list_प्रविष्टिः((संरचना list_शीर्ष *)iter->प्रविष्टिः,
			  संरचना fw_file, list);
}

संरचना fw_file *qfw_file_iter_अग्र(संरचना fw_cfg_file_iter *iter)
{
	iter->प्रविष्टिः = ((संरचना list_शीर्ष *)iter->प्रविष्टिः)->अग्र;
	प्रत्यागम list_प्रविष्टिः((संरचना list_शीर्ष *)iter->प्रविष्टिः,
			  संरचना fw_file, list);
}

bool qfw_file_iter_अन्तः(संरचना fw_cfg_file_iter *iter)
{
	प्रत्यागम iter->प्रविष्टिः == iter->अन्तः_2;
}

पूर्णाङ्क qemu_fwcfg_सज्जता_kernel(संरचना udevice *qfw_dev, ulong आरोपय_addr,
			    ulong initrd_addr)
{
	वर्ण *दत्त_addr;
	u32 सज्जता_आकार = 0, kernel_आकार = 0, cmdline_आकार = 0, initrd_आकार = 0;

	qfw_read_प्रविष्टिः(qfw_dev, FW_CFG_सज्जता_आकार, 4, &सज्जता_आकार);
	qfw_read_प्रविष्टिः(qfw_dev, FW_CFG_KERNEL_आकार, 4, &kernel_आकार);

	यदि (!kernel_आकार) {
		प्रारूप_लिख("fatal: no kernel available\n");
		प्रत्यागम -ENOENT;
	}

	दत्त_addr = map_sysmem(आरोपय_addr, 0);
	यदि (सज्जता_आकार) {
		qfw_read_प्रविष्टिः(qfw_dev, FW_CFG_सज्जता_दत्त,
			       le32_गन्तव्यम्_cpu(सज्जता_आकार), दत्त_addr);
		दत्त_addr += le32_गन्तव्यम्_cpu(सज्जता_आकार);
	}

	qfw_read_प्रविष्टिः(qfw_dev, FW_CFG_KERNEL_दत्त,
		       le32_गन्तव्यम्_cpu(kernel_आकार), दत्त_addr);
	दत्त_addr += le32_गन्तव्यम्_cpu(kernel_आकार);
	env_समुच्चयः_hex("filesize", le32_गन्तव्यम्_cpu(kernel_आकार));

	दत्त_addr = map_sysmem(initrd_addr, 0);
	qfw_read_प्रविष्टिः(qfw_dev, FW_CFG_INITRD_आकार, 4, &initrd_आकार);
	यदि (!initrd_आकार) {
		प्रारूप_लिख("warning: no initrd available\n");
	} अन्यथा {
		qfw_read_प्रविष्टिः(qfw_dev, FW_CFG_INITRD_दत्त,
			       le32_गन्तव्यम्_cpu(initrd_आकार), दत्त_addr);
		दत्त_addr += le32_गन्तव्यम्_cpu(initrd_आकार);
		env_समुच्चयः_hex("filesize", le32_गन्तव्यम्_cpu(initrd_आकार));
	}

	qfw_read_प्रविष्टिः(qfw_dev, FW_CFG_CMDLINE_आकार, 4, &cmdline_आकार);
	यदि (cmdline_आकार) {
		qfw_read_प्रविष्टिः(qfw_dev, FW_CFG_CMDLINE_दत्त,
			       le32_गन्तव्यम्_cpu(cmdline_आकार), दत्त_addr);
		/*
		 * if kernel cmdline only contains '\0', (e.g. no -append
		 * when invoking qemu), do not update bootargs
		 */
		यदि (*दत्त_addr) {
			यदि (env_समुच्चयः("bootargs", दत्त_addr) < 0)
				प्रारूप_लिख("warning: unable to change bootargs\n");
		}
	}

	प्रारूप_लिख("loading kernel to address %lx size %x", आरोपय_addr,
	       le32_गन्तव्यम्_cpu(kernel_आकार));
	यदि (initrd_आकार)
		प्रारूप_लिख(" initrd %lx size %x\n", initrd_addr,
		       le32_गन्तव्यम्_cpu(initrd_आकार));
	अन्यथा
		प्रारूप_लिख("\n");

	प्रत्यागम 0;
}
