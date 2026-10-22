// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015 Miao Yan <yanmiaobest@gmail.com>
 * (C) Copyright 2021 Asherah Connor <ashe@kivikakk.ee>
 */

#అన్తర్భావ <dm.h>
#అన్తర్భావ <env.h>
#అన్తర్భావ <mapmem.h>
#అన్తర్భావ <qfw.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <dm/uclass.h>

పూర్ణాంకం qfw_get_dev(నిర్మాణం udevice **devp)
{
	తిరిగి uclass_మొదటి_device_err(UCLASS_QFW, devp);
}

పూర్ణాంకం qfw_online_cpus(నిర్మాణం udevice *dev)
{
	u16 nb_cpus;

	qfw_read_నమోదు(dev, FW_CFG_NB_CPUS, 2, &nb_cpus);

	తిరిగి le16_గమ్యం_cpu(nb_cpus);
}

పూర్ణాంకం qfw_read_firmware_list(నిర్మాణం udevice *dev)
{
	పూర్ణాంకం i;
	u32 లెక్క;
	నిర్మాణం fw_file *file;
	నిర్మాణం list_తల *నమోదు_2;

	నిర్మాణం qfw_dev *qdev = dev_get_uclass_priv(dev);

	/* don't read it twice */
	ఒకవేళ (!list_ఖాళీలు(&qdev->fw_list))
		తిరిగి 0;

	qfw_read_నమోదు(dev, FW_CFG_FILE_DIR, 4, &లెక్క);
	ఒకవేళ (!లెక్క)
		తిరిగి 0;

	లెక్క = be32_గమ్యం_cpu(లెక్క);
	కొరకు (i = 0; i < లెక్క; i++) {
		file = జ్ఞాపకం_ఇవ్వు(పరిమాణం(*file));
		ఒకవేళ (!file) {
			రాయి("error: allocating resource\n");
			వెళ్ళు err;
		}
		qfw_read_నమోదు(dev, FW_CFG_INVALID,
			       పరిమాణం(నిర్మాణం fw_cfg_file), &file->cfg);
		file->addr = 0;
		list_కూడు_తోక(&file->list, &qdev->fw_list);
	}

	తిరిగి 0;

err:
	list_for_each(నమోదు_2, &qdev->fw_list) {
		file = list_నమోదు(నమోదు_2, నిర్మాణం fw_file, list);
		విడుదల(file);
	}

	తిరిగి -ENOMEM;
}

నిర్మాణం fw_file *qfw_కనుగొను_file(నిర్మాణం udevice *dev, స్థిరరాశి అక్షరం *name)
{
	నిర్మాణం list_తల *నమోదు_2;
	నిర్మాణం fw_file *file;

	నిర్మాణం qfw_dev *qdev = dev_get_uclass_priv(dev);

	list_for_each(నమోదు_2, &qdev->fw_list) {
		file = list_నమోదు(నమోదు_2, నిర్మాణం fw_file, list);
		ఒకవేళ (!strcmp(file->cfg.name, name))
			తిరిగి file;
	}

	తిరిగి ఖాళీ;
}

నిర్మాణం fw_file *qfw_file_iter_init(నిర్మాణం udevice *dev,
				   నిర్మాణం fw_cfg_file_iter *iter)
{
	నిర్మాణం qfw_dev *qdev = dev_get_uclass_priv(dev);

	iter->నమోదు_2 = qdev->fw_list.తదుపరి;
	iter->చివర_2 = &qdev->fw_list;
	తిరిగి list_నమోదు((నిర్మాణం list_తల *)iter->నమోదు_2,
			  నిర్మాణం fw_file, list);
}

నిర్మాణం fw_file *qfw_file_iter_తదుపరి(నిర్మాణం fw_cfg_file_iter *iter)
{
	iter->నమోదు_2 = ((నిర్మాణం list_తల *)iter->నమోదు_2)->తదుపరి;
	తిరిగి list_నమోదు((నిర్మాణం list_తల *)iter->నమోదు_2,
			  నిర్మాణం fw_file, list);
}

bool qfw_file_iter_చివర(నిర్మాణం fw_cfg_file_iter *iter)
{
	తిరిగి iter->నమోదు_2 == iter->చివర_2;
}

పూర్ణాంకం qemu_fwcfg_అమరిక_kernel(నిర్మాణం udevice *qfw_dev, ulong ఎక్కించు_addr,
			    ulong initrd_addr)
{
	అక్షరం *దత్తాంశం_addr;
	u32 అమరిక_కొలత = 0, kernel_కొలత = 0, cmdline_కొలత = 0, initrd_కొలత = 0;

	qfw_read_నమోదు(qfw_dev, FW_CFG_అమరిక_కొలత, 4, &అమరిక_కొలత);
	qfw_read_నమోదు(qfw_dev, FW_CFG_KERNEL_కొలత, 4, &kernel_కొలత);

	ఒకవేళ (!kernel_కొలత) {
		రాయి("fatal: no kernel available\n");
		తిరిగి -ENOENT;
	}

	దత్తాంశం_addr = map_sysmem(ఎక్కించు_addr, 0);
	ఒకవేళ (అమరిక_కొలత) {
		qfw_read_నమోదు(qfw_dev, FW_CFG_అమరిక_దత్తాంశం,
			       le32_గమ్యం_cpu(అమరిక_కొలత), దత్తాంశం_addr);
		దత్తాంశం_addr += le32_గమ్యం_cpu(అమరిక_కొలత);
	}

	qfw_read_నమోదు(qfw_dev, FW_CFG_KERNEL_దత్తాంశం,
		       le32_గమ్యం_cpu(kernel_కొలత), దత్తాంశం_addr);
	దత్తాంశం_addr += le32_గమ్యం_cpu(kernel_కొలత);
	env_సమితి_hex("filesize", le32_గమ్యం_cpu(kernel_కొలత));

	దత్తాంశం_addr = map_sysmem(initrd_addr, 0);
	qfw_read_నమోదు(qfw_dev, FW_CFG_INITRD_కొలత, 4, &initrd_కొలత);
	ఒకవేళ (!initrd_కొలత) {
		రాయి("warning: no initrd available\n");
	} లేకపోతే {
		qfw_read_నమోదు(qfw_dev, FW_CFG_INITRD_దత్తాంశం,
			       le32_గమ్యం_cpu(initrd_కొలత), దత్తాంశం_addr);
		దత్తాంశం_addr += le32_గమ్యం_cpu(initrd_కొలత);
		env_సమితి_hex("filesize", le32_గమ్యం_cpu(initrd_కొలత));
	}

	qfw_read_నమోదు(qfw_dev, FW_CFG_CMDLINE_కొలత, 4, &cmdline_కొలత);
	ఒకవేళ (cmdline_కొలత) {
		qfw_read_నమోదు(qfw_dev, FW_CFG_CMDLINE_దత్తాంశం,
			       le32_గమ్యం_cpu(cmdline_కొలత), దత్తాంశం_addr);
		/*
		 * if kernel cmdline only contains '\0', (e.g. no -append
		 * when invoking qemu), do not update bootargs
		 */
		ఒకవేళ (*దత్తాంశం_addr) {
			ఒకవేళ (env_సమితి("bootargs", దత్తాంశం_addr) < 0)
				రాయి("warning: unable to change bootargs\n");
		}
	}

	రాయి("loading kernel to address %lx size %x", ఎక్కించు_addr,
	       le32_గమ్యం_cpu(kernel_కొలత));
	ఒకవేళ (initrd_కొలత)
		రాయి(" initrd %lx size %x\n", initrd_addr,
		       le32_గమ్యం_cpu(initrd_కొలత));
	లేకపోతే
		రాయి("\n");

	తిరిగి 0;
}
