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
	أرجع uclass_پہلا_device_err(UCLASS_QFW, devp);
}

صحيح qfw_online_cpus(بنية udevice *dev)
{
	u16 nb_cpus;

	qfw_read_اندراج(dev, FW_CFG_NB_CPUS, 2, &nb_cpus);

	أرجع le16_منزل_cpu(nb_cpus);
}

صحيح qfw_read_firmware_list(بنية udevice *dev)
{
	صحيح i;
	u32 گنتی;
	بنية fw_file *file;
	بنية list_سر *اندراج;

	بنية qfw_dev *qdev = dev_get_uclass_priv(dev);

	/* don't read it twice */
	إذا (!list_خالی_تھاواں(&qdev->fw_list))
		أرجع 0;

	qfw_read_اندراج(dev, FW_CFG_FILE_DIR, 4, &گنتی);
	إذا (!گنتی)
		أرجع 0;

	گنتی = be32_منزل_cpu(گنتی);
	لكل (i = 0; i < گنتی; i++) {
		file = حافظہ_دیو(حجم(*file));
		إذا (!file) {
			لکھو("error: allocating resource\n");
			اذهب err;
		}
		qfw_read_اندراج(dev, FW_CFG_INVALID,
			       حجم(بنية fw_cfg_file), &file->cfg);
		file->addr = 0;
		list_جوڑو_پوچھ(&file->list, &qdev->fw_list);
	}

	أرجع 0;

err:
	list_for_each(اندراج, &qdev->fw_list) {
		file = list_اندراج(اندراج, بنية fw_file, list);
		حرر(file);
	}

	أرجع -ENOMEM;
}

بنية fw_file *qfw_لبھو_file(بنية udevice *dev, ثابت حرف *name)
{
	بنية list_سر *اندراج;
	بنية fw_file *file;

	بنية qfw_dev *qdev = dev_get_uclass_priv(dev);

	list_for_each(اندراج, &qdev->fw_list) {
		file = list_اندراج(اندراج, بنية fw_file, list);
		إذا (!strcmp(file->cfg.name, name))
			أرجع file;
	}

	أرجع عدم;
}

بنية fw_file *qfw_file_iter_init(بنية udevice *dev,
				   بنية fw_cfg_file_iter *iter)
{
	بنية qfw_dev *qdev = dev_get_uclass_priv(dev);

	iter->اندراج = qdev->fw_list.اگلا;
	iter->سرا_2 = &qdev->fw_list;
	أرجع list_اندراج((بنية list_سر *)iter->اندراج,
			  بنية fw_file, list);
}

بنية fw_file *qfw_file_iter_اگلا(بنية fw_cfg_file_iter *iter)
{
	iter->اندراج = ((بنية list_سر *)iter->اندراج)->اگلا;
	أرجع list_اندراج((بنية list_سر *)iter->اندراج,
			  بنية fw_file, list);
}

bool qfw_file_iter_سرا(بنية fw_cfg_file_iter *iter)
{
	أرجع iter->اندراج == iter->سرا_2;
}

صحيح qemu_fwcfg_تیاری_kernel(بنية udevice *qfw_dev, ulong لدو_addr,
			    ulong initrd_addr)
{
	حرف *انکڑے_addr;
	u32 تیاری_ناپ = 0, kernel_ناپ = 0, cmdline_ناپ = 0, initrd_ناپ = 0;

	qfw_read_اندراج(qfw_dev, FW_CFG_تیاری_ناپ, 4, &تیاری_ناپ);
	qfw_read_اندراج(qfw_dev, FW_CFG_KERNEL_ناپ, 4, &kernel_ناپ);

	إذا (!kernel_ناپ) {
		لکھو("fatal: no kernel available\n");
		أرجع -ENOENT;
	}

	انکڑے_addr = map_sysmem(لدو_addr, 0);
	إذا (تیاری_ناپ) {
		qfw_read_اندراج(qfw_dev, FW_CFG_تیاری_انکڑے,
			       le32_منزل_cpu(تیاری_ناپ), انکڑے_addr);
		انکڑے_addr += le32_منزل_cpu(تیاری_ناپ);
	}

	qfw_read_اندراج(qfw_dev, FW_CFG_KERNEL_انکڑے,
		       le32_منزل_cpu(kernel_ناپ), انکڑے_addr);
	انکڑے_addr += le32_منزل_cpu(kernel_ناپ);
	env_سیٹ_hex("filesize", le32_منزل_cpu(kernel_ناپ));

	انکڑے_addr = map_sysmem(initrd_addr, 0);
	qfw_read_اندراج(qfw_dev, FW_CFG_INITRD_ناپ, 4, &initrd_ناپ);
	إذا (!initrd_ناپ) {
		لکھو("warning: no initrd available\n");
	} وإلا {
		qfw_read_اندراج(qfw_dev, FW_CFG_INITRD_انکڑے,
			       le32_منزل_cpu(initrd_ناپ), انکڑے_addr);
		انکڑے_addr += le32_منزل_cpu(initrd_ناپ);
		env_سیٹ_hex("filesize", le32_منزل_cpu(initrd_ناپ));
	}

	qfw_read_اندراج(qfw_dev, FW_CFG_CMDLINE_ناپ, 4, &cmdline_ناپ);
	إذا (cmdline_ناپ) {
		qfw_read_اندراج(qfw_dev, FW_CFG_CMDLINE_انکڑے,
			       le32_منزل_cpu(cmdline_ناپ), انکڑے_addr);
		/*
		 * if kernel cmdline only contains '\0', (e.g. no -append
		 * when invoking qemu), do not update bootargs
		 */
		إذا (*انکڑے_addr) {
			إذا (env_سیٹ("bootargs", انکڑے_addr) < 0)
				لکھو("warning: unable to change bootargs\n");
		}
	}

	لکھو("loading kernel to address %lx size %x", لدو_addr,
	       le32_منزل_cpu(kernel_ناپ));
	إذا (initrd_ناپ)
		لکھو(" initrd %lx size %x\n", initrd_addr,
		       le32_منزل_cpu(initrd_ناپ));
	وإلا
		لکھو("\n");

	أرجع 0;
}
