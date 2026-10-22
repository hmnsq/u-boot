// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015 Miao Yan <yanmiaobest@gmail.com>
 * (C) Copyright 2021 Asherah Connor <ashe@kivikakk.ee>
 */

#شامل <dm.h>
#شامل <env.h>
#شامل <mapmem.h>
#شامل <qfw.h>
#شامل <stdlib.h>
#شامل <dm/uclass.h>

صحیح qfw_get_dev(ساخت udevice **devp)
{
	واپس uclass_پہلا_device_err(UCLASS_QFW, devp);
}

صحیح qfw_online_cpus(ساخت udevice *dev)
{
	u16 nb_cpus;

	qfw_read_اندراج(dev, FW_CFG_NB_CPUS, 2, &nb_cpus);

	واپس le16_مقصد_cpu(nb_cpus);
}

صحیح qfw_read_firmware_list(ساخت udevice *dev)
{
	صحیح i;
	u32 گنتی;
	ساخت fw_file *file;
	ساخت list_سر *اندراج_2;

	ساخت qfw_dev *qdev = dev_get_uclass_priv(dev);

	/* don't read it twice */
	اگر (!list_خالی_خانے(&qdev->fw_list))
		واپس 0;

	qfw_read_اندراج(dev, FW_CFG_FILE_DIR, 4, &گنتی);
	اگر (!گنتی)
		واپس 0;

	گنتی = be32_مقصد_cpu(گنتی);
	برائے (i = 0; i < گنتی; i++) {
		file = حافظہ_دو(حجم(*file));
		اگر (!file) {
			لکھو("error: allocating resource\n");
			جاؤ err;
		}
		qfw_read_اندراج(dev, FW_CFG_INVALID,
			       حجم(ساخت fw_cfg_file), &file->cfg);
		file->addr = 0;
		list_جوڑو_دم(&file->list, &qdev->fw_list);
	}

	واپس 0;

err:
	list_for_each(اندراج_2, &qdev->fw_list) {
		file = list_اندراج(اندراج_2, ساخت fw_file, list);
		آزاد(file);
	}

	واپس -ENOMEM;
}

ساخت fw_file *qfw_ڈھونڈو_file(ساخت udevice *dev, ثابت حرف *name)
{
	ساخت list_سر *اندراج_2;
	ساخت fw_file *file;

	ساخت qfw_dev *qdev = dev_get_uclass_priv(dev);

	list_for_each(اندراج_2, &qdev->fw_list) {
		file = list_اندراج(اندراج_2, ساخت fw_file, list);
		اگر (!strcmp(file->cfg.name, name))
			واپس file;
	}

	واپس تہی;
}

ساخت fw_file *qfw_file_iter_init(ساخت udevice *dev,
				   ساخت fw_cfg_file_iter *iter)
{
	ساخت qfw_dev *qdev = dev_get_uclass_priv(dev);

	iter->اندراج_2 = qdev->fw_list.اگلا;
	iter->انتہا_2 = &qdev->fw_list;
	واپس list_اندراج((ساخت list_سر *)iter->اندراج_2,
			  ساخت fw_file, list);
}

ساخت fw_file *qfw_file_iter_اگلا(ساخت fw_cfg_file_iter *iter)
{
	iter->اندراج_2 = ((ساخت list_سر *)iter->اندراج_2)->اگلا;
	واپس list_اندراج((ساخت list_سر *)iter->اندراج_2,
			  ساخت fw_file, list);
}

bool qfw_file_iter_انتہا(ساخت fw_cfg_file_iter *iter)
{
	واپس iter->اندراج_2 == iter->انتہا_2;
}

صحیح qemu_fwcfg_تیاری_kernel(ساخت udevice *qfw_dev, ulong بھرو_addr,
			    ulong initrd_addr)
{
	حرف *ڈیٹا_addr;
	u32 تیاری_سائز = 0, kernel_سائز = 0, cmdline_سائز = 0, initrd_سائز = 0;

	qfw_read_اندراج(qfw_dev, FW_CFG_تیاری_سائز, 4, &تیاری_سائز);
	qfw_read_اندراج(qfw_dev, FW_CFG_KERNEL_سائز, 4, &kernel_سائز);

	اگر (!kernel_سائز) {
		لکھو("fatal: no kernel available\n");
		واپس -ENOENT;
	}

	ڈیٹا_addr = map_sysmem(بھرو_addr, 0);
	اگر (تیاری_سائز) {
		qfw_read_اندراج(qfw_dev, FW_CFG_تیاری_ڈیٹا,
			       le32_مقصد_cpu(تیاری_سائز), ڈیٹا_addr);
		ڈیٹا_addr += le32_مقصد_cpu(تیاری_سائز);
	}

	qfw_read_اندراج(qfw_dev, FW_CFG_KERNEL_ڈیٹا,
		       le32_مقصد_cpu(kernel_سائز), ڈیٹا_addr);
	ڈیٹا_addr += le32_مقصد_cpu(kernel_سائز);
	env_سیٹ_hex("filesize", le32_مقصد_cpu(kernel_سائز));

	ڈیٹا_addr = map_sysmem(initrd_addr, 0);
	qfw_read_اندراج(qfw_dev, FW_CFG_INITRD_سائز, 4, &initrd_سائز);
	اگر (!initrd_سائز) {
		لکھو("warning: no initrd available\n");
	} ورنہ {
		qfw_read_اندراج(qfw_dev, FW_CFG_INITRD_ڈیٹا,
			       le32_مقصد_cpu(initrd_سائز), ڈیٹا_addr);
		ڈیٹا_addr += le32_مقصد_cpu(initrd_سائز);
		env_سیٹ_hex("filesize", le32_مقصد_cpu(initrd_سائز));
	}

	qfw_read_اندراج(qfw_dev, FW_CFG_CMDLINE_سائز, 4, &cmdline_سائز);
	اگر (cmdline_سائز) {
		qfw_read_اندراج(qfw_dev, FW_CFG_CMDLINE_ڈیٹا,
			       le32_مقصد_cpu(cmdline_سائز), ڈیٹا_addr);
		/*
		 * if kernel cmdline only contains '\0', (e.g. no -append
		 * when invoking qemu), do not update bootargs
		 */
		اگر (*ڈیٹا_addr) {
			اگر (env_سیٹ("bootargs", ڈیٹا_addr) < 0)
				لکھو("warning: unable to change bootargs\n");
		}
	}

	لکھو("loading kernel to address %lx size %x", بھرو_addr,
	       le32_مقصد_cpu(kernel_سائز));
	اگر (initrd_سائز)
		لکھو(" initrd %lx size %x\n", initrd_addr,
		       le32_مقصد_cpu(initrd_سائز));
	ورنہ
		لکھو("\n");

	واپس 0;
}
