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
	أرجع uclass_لومړی_device_err(UCLASS_QFW, devp);
}

صحيح qfw_online_cpus(بنية udevice *dev)
{
	u16 nb_cpus;

	qfw_read_ثبت(dev, FW_CFG_NB_CPUS, 2, &nb_cpus);

	أرجع le16_منزل_cpu(nb_cpus);
}

صحيح qfw_read_firmware_list(بنية udevice *dev)
{
	صحيح i;
	u32 شمېر;
	بنية fw_file *file;
	بنية list_سر *ثبت;

	بنية qfw_dev *qdev = dev_get_uclass_priv(dev);

	/* don't read it twice */
	إذا (!list_تشه(&qdev->fw_list))
		أرجع 0;

	qfw_read_ثبت(dev, FW_CFG_FILE_DIR, 4, &شمېر);
	إذا (!شمېر)
		أرجع 0;

	شمېر = be32_منزل_cpu(شمېر);
	لكل (i = 0; i < شمېر; i++) {
		file = حافظه_ورکړه(حجم(*file));
		إذا (!file) {
			چاپ_کړه("error: allocating resource\n");
			اذهب err;
		}
		qfw_read_ثبت(dev, FW_CFG_INVALID,
			       حجم(بنية fw_cfg_file), &file->cfg);
		file->addr = 0;
		list_جمع_کړه_لکۍ(&file->list, &qdev->fw_list);
	}

	أرجع 0;

err:
	list_for_each(ثبت, &qdev->fw_list) {
		file = list_ثبت(ثبت, بنية fw_file, list);
		حرر(file);
	}

	أرجع -ENOMEM;
}

بنية fw_file *qfw_پیدا_کړه_file(بنية udevice *dev, ثابت حرف *name)
{
	بنية list_سر *ثبت;
	بنية fw_file *file;

	بنية qfw_dev *qdev = dev_get_uclass_priv(dev);

	list_for_each(ثبت, &qdev->fw_list) {
		file = list_ثبت(ثبت, بنية fw_file, list);
		إذا (!strcmp(file->cfg.name, name))
			أرجع file;
	}

	أرجع عدم;
}

بنية fw_file *qfw_file_iter_init(بنية udevice *dev,
				   بنية fw_cfg_file_iter *iter)
{
	بنية qfw_dev *qdev = dev_get_uclass_priv(dev);

	iter->ثبت = qdev->fw_list.بل;
	iter->پای_ټکی_2 = &qdev->fw_list;
	أرجع list_ثبت((بنية list_سر *)iter->ثبت,
			  بنية fw_file, list);
}

بنية fw_file *qfw_file_iter_بل(بنية fw_cfg_file_iter *iter)
{
	iter->ثبت = ((بنية list_سر *)iter->ثبت)->بل;
	أرجع list_ثبت((بنية list_سر *)iter->ثبت,
			  بنية fw_file, list);
}

bool qfw_file_iter_پای_ټکی(بنية fw_cfg_file_iter *iter)
{
	أرجع iter->ثبت == iter->پای_ټکی_2;
}

صحيح qemu_fwcfg_تیاری_kernel(بنية udevice *qfw_dev, ulong بارول_addr,
			    ulong initrd_addr)
{
	حرف *معلومات_addr;
	u32 تیاری_اندازه = 0, kernel_اندازه = 0, cmdline_اندازه = 0, initrd_اندازه = 0;

	qfw_read_ثبت(qfw_dev, FW_CFG_تیاری_اندازه, 4, &تیاری_اندازه);
	qfw_read_ثبت(qfw_dev, FW_CFG_KERNEL_اندازه, 4, &kernel_اندازه);

	إذا (!kernel_اندازه) {
		چاپ_کړه("fatal: no kernel available\n");
		أرجع -ENOENT;
	}

	معلومات_addr = map_sysmem(بارول_addr, 0);
	إذا (تیاری_اندازه) {
		qfw_read_ثبت(qfw_dev, FW_CFG_تیاری_معلومات,
			       le32_منزل_cpu(تیاری_اندازه), معلومات_addr);
		معلومات_addr += le32_منزل_cpu(تیاری_اندازه);
	}

	qfw_read_ثبت(qfw_dev, FW_CFG_KERNEL_معلومات,
		       le32_منزل_cpu(kernel_اندازه), معلومات_addr);
	معلومات_addr += le32_منزل_cpu(kernel_اندازه);
	env_سېټ_hex("filesize", le32_منزل_cpu(kernel_اندازه));

	معلومات_addr = map_sysmem(initrd_addr, 0);
	qfw_read_ثبت(qfw_dev, FW_CFG_INITRD_اندازه, 4, &initrd_اندازه);
	إذا (!initrd_اندازه) {
		چاپ_کړه("warning: no initrd available\n");
	} وإلا {
		qfw_read_ثبت(qfw_dev, FW_CFG_INITRD_معلومات,
			       le32_منزل_cpu(initrd_اندازه), معلومات_addr);
		معلومات_addr += le32_منزل_cpu(initrd_اندازه);
		env_سېټ_hex("filesize", le32_منزل_cpu(initrd_اندازه));
	}

	qfw_read_ثبت(qfw_dev, FW_CFG_CMDLINE_اندازه, 4, &cmdline_اندازه);
	إذا (cmdline_اندازه) {
		qfw_read_ثبت(qfw_dev, FW_CFG_CMDLINE_معلومات,
			       le32_منزل_cpu(cmdline_اندازه), معلومات_addr);
		/*
		 * if kernel cmdline only contains '\0', (e.g. no -append
		 * when invoking qemu), do not update bootargs
		 */
		إذا (*معلومات_addr) {
			إذا (env_سېټ("bootargs", معلومات_addr) < 0)
				چاپ_کړه("warning: unable to change bootargs\n");
		}
	}

	چاپ_کړه("loading kernel to address %lx size %x", بارول_addr,
	       le32_منزل_cpu(kernel_اندازه));
	إذا (initrd_اندازه)
		چاپ_کړه(" initrd %lx size %x\n", initrd_addr,
		       le32_منزل_cpu(initrd_اندازه));
	وإلا
		چاپ_کړه("\n");

	أرجع 0;
}
