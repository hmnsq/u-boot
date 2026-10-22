// SPDX-License-Identifier: GPL-2.0+

#ਅਨ੍ਤਰ੍ਭਾਵ <dm.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <init.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sysinfo.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/global_data.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/libfdt.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/compiler.h>

DECLARE_GLOBAL_ਅੰਕੜੇ_PTR;

ਪੂਰਨ __weak checkboard(ਖਾਲੀ)
{
	ਵਾਪਸ 0;
}

ਸਥਿਰ ਅਚਲ ਬਣਤਰ ਮੰਜ਼ਿਲ_show {
	ਅਚਲ ਅੱਖਰ *name;
	ਗਿਣਤੀ sysinfo_id id;
} ਮੰਜ਼ਿਲ_show[] = {
	{ "Manufacturer", SYSID_BOARD_MANUFACTURER},
	{ "Prior-stage version", SYSID_ਪੂਰਵ_STAGE_ਸੰਸਕਰਣ },
	{ "Prior-stage date", SYSID_ਪੂਰਵ_STAGE_DATE },
	{ /* sentinel */ }
};

ਸਥਿਰ ਪੂਰਨ try_sysinfo(ਖਾਲੀ)
{
	ਬਣਤਰ udevice *dev;
	ਅੱਖਰ str[80];
	ਪੂਰਨ ret;

	/* This might provide more detail */
	ret = sysinfo_get(&dev);
	ਜੇ (ret)
		ਵਾਪਸ ret;

	ret = sysinfo_detect(dev);
	ਜੇ (ret)
		ਵਾਪਸ ret;

	ret = sysinfo_get_str(dev, SYSID_BOARD_MODEL, ਆਕਾਰ(str), str);
	ਜੇ (ret)
		ਵਾਪਸ ret;
	ਲਿਖੋ("Model: %s\n", str);

	ਜੇ (IS_ENABLED(CONFIG_SYSINFO_EXTRA)) {
		ਅਚਲ ਬਣਤਰ ਮੰਜ਼ਿਲ_show *ਵਸਤੂ;

		ਲਈ (ਵਸਤੂ = ਮੰਜ਼ਿਲ_show; ਵਸਤੂ->id; ਵਸਤੂ++) {
			ret = sysinfo_get_str(dev, ਵਸਤੂ->id, ਆਕਾਰ(str), str);
			ਜੇ (!ret)
				ਲਿਖੋ("%s: %s\n", ਵਸਤੂ->name, str);
		}
	}

	ਵਾਪਸ 0;
}

ਪੂਰਨ show_board_info(ਖਾਲੀ)
{
	ਜੇ (IS_ENABLED(CONFIG_ਦਾ_CONTROL)) {
		ਪੂਰਨ ret = -ENOSYS;

		ਜੇ (IS_ENABLED(CONFIG_SYSINFO))
			ret = try_sysinfo();

		/* Fail back to the main 'model' if available */
		ਜੇ (ret) {
			ਅਚਲ ਅੱਖਰ *model;

			model = fdt_getprop(gd->fdt_blob, 0, "model", ਨਿਰਾ);
			ਜੇ (model)
				ਲਿਖੋ("Model: %s\n", model);
		}
	}

	ਵਾਪਸ checkboard();
}
