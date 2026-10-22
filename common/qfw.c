// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2015 Miao Yan <yanmiaobest@gmail.com>
 * (C) Copyright 2021 Asherah Connor <ashe@kivikakk.ee>
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <dm.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <env.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <mapmem.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <qfw.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <dm/uclass.h>

ਪੂਰਨ qfw_get_dev(ਬਣਤਰ udevice **devp)
{
	ਵਾਪਸ uclass_ਪਹਿਲਾ_device_err(UCLASS_QFW, devp);
}

ਪੂਰਨ qfw_online_cpus(ਬਣਤਰ udevice *dev)
{
	u16 nb_cpus;

	qfw_read_ਇੰਦਰਾਜ(dev, FW_CFG_NB_CPUS, 2, &nb_cpus);

	ਵਾਪਸ le16_ਮੰਜ਼ਿਲ_cpu(nb_cpus);
}

ਪੂਰਨ qfw_read_firmware_list(ਬਣਤਰ udevice *dev)
{
	ਪੂਰਨ i;
	u32 ਗਿਣਤੀ_2;
	ਬਣਤਰ fw_file *file;
	ਬਣਤਰ list_ਸਿਰ *ਇੰਦਰਾਜ;

	ਬਣਤਰ qfw_dev *qdev = dev_get_uclass_priv(dev);

	/* don't read it twice */
	ਜੇ (!list_ਖਾਲੀ_ਥਾਵਾਂ(&qdev->fw_list))
		ਵਾਪਸ 0;

	qfw_read_ਇੰਦਰਾਜ(dev, FW_CFG_FILE_DIR, 4, &ਗਿਣਤੀ_2);
	ਜੇ (!ਗਿਣਤੀ_2)
		ਵਾਪਸ 0;

	ਗਿਣਤੀ_2 = be32_ਮੰਜ਼ਿਲ_cpu(ਗਿਣਤੀ_2);
	ਲਈ (i = 0; i < ਗਿਣਤੀ_2; i++) {
		file = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਆਕਾਰ(*file));
		ਜੇ (!file) {
			ਲਿਖੋ("error: allocating resource\n");
			ਜਾਓ err;
		}
		qfw_read_ਇੰਦਰਾਜ(dev, FW_CFG_INVALID,
			       ਆਕਾਰ(ਬਣਤਰ fw_cfg_file), &file->cfg);
		file->addr = 0;
		list_ਜੋੜੋ_ਪੂਛ(&file->list, &qdev->fw_list);
	}

	ਵਾਪਸ 0;

err:
	list_for_each(ਇੰਦਰਾਜ, &qdev->fw_list) {
		file = list_ਇੰਦਰਾਜ(ਇੰਦਰਾਜ, ਬਣਤਰ fw_file, list);
		ਮੁਕਤ(file);
	}

	ਵਾਪਸ -ENOMEM;
}

ਬਣਤਰ fw_file *qfw_ਲੱਭੋ_file(ਬਣਤਰ udevice *dev, ਅਚਲ ਅੱਖਰ *name)
{
	ਬਣਤਰ list_ਸਿਰ *ਇੰਦਰਾਜ;
	ਬਣਤਰ fw_file *file;

	ਬਣਤਰ qfw_dev *qdev = dev_get_uclass_priv(dev);

	list_for_each(ਇੰਦਰਾਜ, &qdev->fw_list) {
		file = list_ਇੰਦਰਾਜ(ਇੰਦਰਾਜ, ਬਣਤਰ fw_file, list);
		ਜੇ (!strcmp(file->cfg.name, name))
			ਵਾਪਸ file;
	}

	ਵਾਪਸ ਨਿਰਾ;
}

ਬਣਤਰ fw_file *qfw_file_iter_init(ਬਣਤਰ udevice *dev,
				   ਬਣਤਰ fw_cfg_file_iter *iter)
{
	ਬਣਤਰ qfw_dev *qdev = dev_get_uclass_priv(dev);

	iter->ਇੰਦਰਾਜ = qdev->fw_list.ਅਗਲਾ;
	iter->ਸਿਰਾ_2 = &qdev->fw_list;
	ਵਾਪਸ list_ਇੰਦਰਾਜ((ਬਣਤਰ list_ਸਿਰ *)iter->ਇੰਦਰਾਜ,
			  ਬਣਤਰ fw_file, list);
}

ਬਣਤਰ fw_file *qfw_file_iter_ਅਗਲਾ(ਬਣਤਰ fw_cfg_file_iter *iter)
{
	iter->ਇੰਦਰਾਜ = ((ਬਣਤਰ list_ਸਿਰ *)iter->ਇੰਦਰਾਜ)->ਅਗਲਾ;
	ਵਾਪਸ list_ਇੰਦਰਾਜ((ਬਣਤਰ list_ਸਿਰ *)iter->ਇੰਦਰਾਜ,
			  ਬਣਤਰ fw_file, list);
}

bool qfw_file_iter_ਸਿਰਾ(ਬਣਤਰ fw_cfg_file_iter *iter)
{
	ਵਾਪਸ iter->ਇੰਦਰਾਜ == iter->ਸਿਰਾ_2;
}

ਪੂਰਨ qemu_fwcfg_ਤਿਆਰੀ_kernel(ਬਣਤਰ udevice *qfw_dev, ulong ਲੱਦੋ_addr,
			    ulong initrd_addr)
{
	ਅੱਖਰ *ਅੰਕੜੇ_addr;
	u32 ਤਿਆਰੀ_ਮਾਪ = 0, kernel_ਮਾਪ = 0, cmdline_ਮਾਪ = 0, initrd_ਮਾਪ = 0;

	qfw_read_ਇੰਦਰਾਜ(qfw_dev, FW_CFG_ਤਿਆਰੀ_ਮਾਪ, 4, &ਤਿਆਰੀ_ਮਾਪ);
	qfw_read_ਇੰਦਰਾਜ(qfw_dev, FW_CFG_KERNEL_ਮਾਪ, 4, &kernel_ਮਾਪ);

	ਜੇ (!kernel_ਮਾਪ) {
		ਲਿਖੋ("fatal: no kernel available\n");
		ਵਾਪਸ -ENOENT;
	}

	ਅੰਕੜੇ_addr = map_sysmem(ਲੱਦੋ_addr, 0);
	ਜੇ (ਤਿਆਰੀ_ਮਾਪ) {
		qfw_read_ਇੰਦਰਾਜ(qfw_dev, FW_CFG_ਤਿਆਰੀ_ਅੰਕੜੇ,
			       le32_ਮੰਜ਼ਿਲ_cpu(ਤਿਆਰੀ_ਮਾਪ), ਅੰਕੜੇ_addr);
		ਅੰਕੜੇ_addr += le32_ਮੰਜ਼ਿਲ_cpu(ਤਿਆਰੀ_ਮਾਪ);
	}

	qfw_read_ਇੰਦਰਾਜ(qfw_dev, FW_CFG_KERNEL_ਅੰਕੜੇ,
		       le32_ਮੰਜ਼ਿਲ_cpu(kernel_ਮਾਪ), ਅੰਕੜੇ_addr);
	ਅੰਕੜੇ_addr += le32_ਮੰਜ਼ਿਲ_cpu(kernel_ਮਾਪ);
	env_ਸੈੱਟ_hex("filesize", le32_ਮੰਜ਼ਿਲ_cpu(kernel_ਮਾਪ));

	ਅੰਕੜੇ_addr = map_sysmem(initrd_addr, 0);
	qfw_read_ਇੰਦਰਾਜ(qfw_dev, FW_CFG_INITRD_ਮਾਪ, 4, &initrd_ਮਾਪ);
	ਜੇ (!initrd_ਮਾਪ) {
		ਲਿਖੋ("warning: no initrd available\n");
	} ਨਹੀਂਤਾਂ {
		qfw_read_ਇੰਦਰਾਜ(qfw_dev, FW_CFG_INITRD_ਅੰਕੜੇ,
			       le32_ਮੰਜ਼ਿਲ_cpu(initrd_ਮਾਪ), ਅੰਕੜੇ_addr);
		ਅੰਕੜੇ_addr += le32_ਮੰਜ਼ਿਲ_cpu(initrd_ਮਾਪ);
		env_ਸੈੱਟ_hex("filesize", le32_ਮੰਜ਼ਿਲ_cpu(initrd_ਮਾਪ));
	}

	qfw_read_ਇੰਦਰਾਜ(qfw_dev, FW_CFG_CMDLINE_ਮਾਪ, 4, &cmdline_ਮਾਪ);
	ਜੇ (cmdline_ਮਾਪ) {
		qfw_read_ਇੰਦਰਾਜ(qfw_dev, FW_CFG_CMDLINE_ਅੰਕੜੇ,
			       le32_ਮੰਜ਼ਿਲ_cpu(cmdline_ਮਾਪ), ਅੰਕੜੇ_addr);
		/*
		 * if kernel cmdline only contains '\0', (e.g. no -append
		 * when invoking qemu), do not update bootargs
		 */
		ਜੇ (*ਅੰਕੜੇ_addr) {
			ਜੇ (env_ਸੈੱਟ("bootargs", ਅੰਕੜੇ_addr) < 0)
				ਲਿਖੋ("warning: unable to change bootargs\n");
		}
	}

	ਲਿਖੋ("loading kernel to address %lx size %x", ਲੱਦੋ_addr,
	       le32_ਮੰਜ਼ਿਲ_cpu(kernel_ਮਾਪ));
	ਜੇ (initrd_ਮਾਪ)
		ਲਿਖੋ(" initrd %lx size %x\n", initrd_addr,
		       le32_ਮੰਜ਼ਿਲ_cpu(initrd_ਮਾਪ));
	ਨਹੀਂਤਾਂ
		ਲਿਖੋ("\n");

	ਵਾਪਸ 0;
}
