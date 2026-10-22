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
	أرجع uclass_اولی_device_err(UCLASS_QFW, devp);
}

صحيح qfw_online_cpus(بنية udevice *dev)
{
	u16 nb_cpus;

	qfw_read_مدخل(dev, FW_CFG_NB_CPUS, 2, &nb_cpus);

	أرجع le16_مقصد_cpu(nb_cpus);
}

صحيح qfw_read_firmware_list(بنية udevice *dev)
{
	صحيح i;
	u32 شمار;
	بنية fw_file *file;
	بنية list_سر *مدخل;

	بنية qfw_dev *qdev = dev_get_uclass_priv(dev);

	/* don't read it twice */
	إذا (!list_خالی(&qdev->fw_list))
		أرجع 0;

	qfw_read_مدخل(dev, FW_CFG_FILE_DIR, 4, &شمار);
	إذا (!شمار)
		أرجع 0;

	شمار = be32_مقصد_cpu(شمار);
	لكل (i = 0; i < شمار; i++) {
		file = حافظه_بده(حجم(*file));
		إذا (!file) {
			چاپ_کن("error: allocating resource\n");
			اذهب err;
		}
		qfw_read_مدخل(dev, FW_CFG_INVALID,
			       حجم(بنية fw_cfg_file), &file->cfg);
		file->addr = 0;
		list_جمع_کن_دم(&file->list, &qdev->fw_list);
	}

	أرجع 0;

err:
	list_for_each(مدخل, &qdev->fw_list) {
		file = list_مدخل(مدخل, بنية fw_file, list);
		حرر(file);
	}

	أرجع -ENOMEM;
}

بنية fw_file *qfw_پیدا_کن_file(بنية udevice *dev, ثابت حرف *name)
{
	بنية list_سر *مدخل;
	بنية fw_file *file;

	بنية qfw_dev *qdev = dev_get_uclass_priv(dev);

	list_for_each(مدخل, &qdev->fw_list) {
		file = list_مدخل(مدخل, بنية fw_file, list);
		إذا (!strcmp(file->cfg.name, name))
			أرجع file;
	}

	أرجع عدم;
}

بنية fw_file *qfw_file_iter_init(بنية udevice *dev,
				   بنية fw_cfg_file_iter *iter)
{
	بنية qfw_dev *qdev = dev_get_uclass_priv(dev);

	iter->مدخل = qdev->fw_list.بعدی;
	iter->انتها_2 = &qdev->fw_list;
	أرجع list_مدخل((بنية list_سر *)iter->مدخل,
			  بنية fw_file, list);
}

بنية fw_file *qfw_file_iter_بعدی(بنية fw_cfg_file_iter *iter)
{
	iter->مدخل = ((بنية list_سر *)iter->مدخل)->بعدی;
	أرجع list_مدخل((بنية list_سر *)iter->مدخل,
			  بنية fw_file, list);
}

bool qfw_file_iter_انتها(بنية fw_cfg_file_iter *iter)
{
	أرجع iter->مدخل == iter->انتها_2;
}

صحيح qemu_fwcfg_راهاندازی_kernel(بنية udevice *qfw_dev, ulong بارگیری_addr,
			    ulong initrd_addr)
{
	حرف *داده_addr;
	u32 راهاندازی_اندازه = 0, kernel_اندازه = 0, cmdline_اندازه = 0, initrd_اندازه = 0;

	qfw_read_مدخل(qfw_dev, FW_CFG_راهاندازی_اندازه, 4, &راهاندازی_اندازه);
	qfw_read_مدخل(qfw_dev, FW_CFG_KERNEL_اندازه, 4, &kernel_اندازه);

	إذا (!kernel_اندازه) {
		چاپ_کن("fatal: no kernel available\n");
		أرجع -ENOENT;
	}

	داده_addr = map_sysmem(بارگیری_addr, 0);
	إذا (راهاندازی_اندازه) {
		qfw_read_مدخل(qfw_dev, FW_CFG_راهاندازی_داده,
			       le32_مقصد_cpu(راهاندازی_اندازه), داده_addr);
		داده_addr += le32_مقصد_cpu(راهاندازی_اندازه);
	}

	qfw_read_مدخل(qfw_dev, FW_CFG_KERNEL_داده,
		       le32_مقصد_cpu(kernel_اندازه), داده_addr);
	داده_addr += le32_مقصد_cpu(kernel_اندازه);
	env_مجموعه_hex("filesize", le32_مقصد_cpu(kernel_اندازه));

	داده_addr = map_sysmem(initrd_addr, 0);
	qfw_read_مدخل(qfw_dev, FW_CFG_INITRD_اندازه, 4, &initrd_اندازه);
	إذا (!initrd_اندازه) {
		چاپ_کن("warning: no initrd available\n");
	} وإلا {
		qfw_read_مدخل(qfw_dev, FW_CFG_INITRD_داده,
			       le32_مقصد_cpu(initrd_اندازه), داده_addr);
		داده_addr += le32_مقصد_cpu(initrd_اندازه);
		env_مجموعه_hex("filesize", le32_مقصد_cpu(initrd_اندازه));
	}

	qfw_read_مدخل(qfw_dev, FW_CFG_CMDLINE_اندازه, 4, &cmdline_اندازه);
	إذا (cmdline_اندازه) {
		qfw_read_مدخل(qfw_dev, FW_CFG_CMDLINE_داده,
			       le32_مقصد_cpu(cmdline_اندازه), داده_addr);
		/*
		 * if kernel cmdline only contains '\0', (e.g. no -append
		 * when invoking qemu), do not update bootargs
		 */
		إذا (*داده_addr) {
			إذا (env_مجموعه("bootargs", داده_addr) < 0)
				چاپ_کن("warning: unable to change bootargs\n");
		}
	}

	چاپ_کن("loading kernel to address %lx size %x", بارگیری_addr,
	       le32_مقصد_cpu(kernel_اندازه));
	إذا (initrd_اندازه)
		چاپ_کن(" initrd %lx size %x\n", initrd_addr,
		       le32_مقصد_cpu(initrd_اندازه));
	وإلا
		چاپ_کن("\n");

	أرجع 0;
}
