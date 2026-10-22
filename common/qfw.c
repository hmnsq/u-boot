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
	أرجع uclass_پهريون_device_err(UCLASS_QFW, devp);
}

صحيح qfw_online_cpus(بنية udevice *dev)
{
	u16 nb_cpus;

	qfw_read_داخلا(dev, FW_CFG_NB_CPUS, 2, &nb_cpus);

	أرجع le16_منزل_cpu(nb_cpus);
}

صحيح qfw_read_firmware_list(بنية udevice *dev)
{
	صحيح i;
	u32 ڳڻپ;
	بنية fw_file *file;
	بنية list_سر *داخلا_2;

	بنية qfw_dev *qdev = dev_get_uclass_priv(dev);

	/* don't read it twice */
	إذا (!list_خالي_جايون(&qdev->fw_list))
		أرجع 0;

	qfw_read_داخلا(dev, FW_CFG_FILE_DIR, 4, &ڳڻپ);
	إذا (!ڳڻپ)
		أرجع 0;

	ڳڻپ = be32_منزل_cpu(ڳڻپ);
	لكل (i = 0; i < ڳڻپ; i++) {
		file = يادگيري_ڏيو(حجم(*file));
		إذا (!file) {
			لکو("error: allocating resource\n");
			اذهب err;
		}
		qfw_read_داخلا(dev, FW_CFG_INVALID,
			       حجم(بنية fw_cfg_file), &file->cfg);
		file->addr = 0;
		list_جوڙيو_پڇ(&file->list, &qdev->fw_list);
	}

	أرجع 0;

err:
	list_for_each(داخلا_2, &qdev->fw_list) {
		file = list_داخلا(داخلا_2, بنية fw_file, list);
		حرر(file);
	}

	أرجع -ENOMEM;
}

بنية fw_file *qfw_ڳوليو_file(بنية udevice *dev, ثابت حرف *name)
{
	بنية list_سر *داخلا_2;
	بنية fw_file *file;

	بنية qfw_dev *qdev = dev_get_uclass_priv(dev);

	list_for_each(داخلا_2, &qdev->fw_list) {
		file = list_داخلا(داخلا_2, بنية fw_file, list);
		إذا (!strcmp(file->cfg.name, name))
			أرجع file;
	}

	أرجع عدم;
}

بنية fw_file *qfw_file_iter_init(بنية udevice *dev,
				   بنية fw_cfg_file_iter *iter)
{
	بنية qfw_dev *qdev = dev_get_uclass_priv(dev);

	iter->داخلا_2 = qdev->fw_list.ايندڙ;
	iter->پڇاڙي_2 = &qdev->fw_list;
	أرجع list_داخلا((بنية list_سر *)iter->داخلا_2,
			  بنية fw_file, list);
}

بنية fw_file *qfw_file_iter_ايندڙ(بنية fw_cfg_file_iter *iter)
{
	iter->داخلا_2 = ((بنية list_سر *)iter->داخلا_2)->ايندڙ;
	أرجع list_داخلا((بنية list_سر *)iter->داخلا_2,
			  بنية fw_file, list);
}

bool qfw_file_iter_پڇاڙي(بنية fw_cfg_file_iter *iter)
{
	أرجع iter->داخلا_2 == iter->پڇاڙي_2;
}

صحيح qemu_fwcfg_تياري_kernel(بنية udevice *qfw_dev, ulong ڀريو_addr,
			    ulong initrd_addr)
{
	حرف *معلومات_addr;
	u32 تياري_سائيز = 0, kernel_سائيز = 0, cmdline_سائيز = 0, initrd_سائيز = 0;

	qfw_read_داخلا(qfw_dev, FW_CFG_تياري_سائيز, 4, &تياري_سائيز);
	qfw_read_داخلا(qfw_dev, FW_CFG_KERNEL_سائيز, 4, &kernel_سائيز);

	إذا (!kernel_سائيز) {
		لکو("fatal: no kernel available\n");
		أرجع -ENOENT;
	}

	معلومات_addr = map_sysmem(ڀريو_addr, 0);
	إذا (تياري_سائيز) {
		qfw_read_داخلا(qfw_dev, FW_CFG_تياري_معلومات,
			       le32_منزل_cpu(تياري_سائيز), معلومات_addr);
		معلومات_addr += le32_منزل_cpu(تياري_سائيز);
	}

	qfw_read_داخلا(qfw_dev, FW_CFG_KERNEL_معلومات,
		       le32_منزل_cpu(kernel_سائيز), معلومات_addr);
	معلومات_addr += le32_منزل_cpu(kernel_سائيز);
	env_سيٽ_hex("filesize", le32_منزل_cpu(kernel_سائيز));

	معلومات_addr = map_sysmem(initrd_addr, 0);
	qfw_read_داخلا(qfw_dev, FW_CFG_INITRD_سائيز, 4, &initrd_سائيز);
	إذا (!initrd_سائيز) {
		لکو("warning: no initrd available\n");
	} وإلا {
		qfw_read_داخلا(qfw_dev, FW_CFG_INITRD_معلومات,
			       le32_منزل_cpu(initrd_سائيز), معلومات_addr);
		معلومات_addr += le32_منزل_cpu(initrd_سائيز);
		env_سيٽ_hex("filesize", le32_منزل_cpu(initrd_سائيز));
	}

	qfw_read_داخلا(qfw_dev, FW_CFG_CMDLINE_سائيز, 4, &cmdline_سائيز);
	إذا (cmdline_سائيز) {
		qfw_read_داخلا(qfw_dev, FW_CFG_CMDLINE_معلومات,
			       le32_منزل_cpu(cmdline_سائيز), معلومات_addr);
		/*
		 * if kernel cmdline only contains '\0', (e.g. no -append
		 * when invoking qemu), do not update bootargs
		 */
		إذا (*معلومات_addr) {
			إذا (env_سيٽ("bootargs", معلومات_addr) < 0)
				لکو("warning: unable to change bootargs\n");
		}
	}

	لکو("loading kernel to address %lx size %x", ڀريو_addr,
	       le32_منزل_cpu(kernel_سائيز));
	إذا (initrd_سائيز)
		لکو(" initrd %lx size %x\n", initrd_addr,
		       le32_منزل_cpu(initrd_سائيز));
	وإلا
		لکو("\n");

	أرجع 0;
}
