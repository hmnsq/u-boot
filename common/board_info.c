// SPDX-License-Identifier: GPL-2.0+

#അന്തര്ഭാവ <dm.h>
#അന്തര്ഭാവ <init.h>
#അന്തര്ഭാവ <sysinfo.h>
#അന്തര്ഭാവ <asm/global_data.h>
#അന്തര്ഭാവ <linux/libfdt.h>
#അന്തര്ഭാവ <linux/compiler.h>

DECLARE_GLOBAL_DATA_PTR;

പൂർണ്ണസംഖ്യ __weak checkboard(ശൂന്യം)
{
	തിരികെ 0;
}

സ്ഥിതം സ്ഥിരം ഘടന to_show {
	സ്ഥിരം അക്ഷരം *name;
	എണ്ണൽ sysinfo_id id;
} to_show[] = {
	{ "Manufacturer", SYSID_BOARD_MANUFACTURER},
	{ "Prior-stage version", SYSID_PRIOR_STAGE_VERSION },
	{ "Prior-stage date", SYSID_PRIOR_STAGE_DATE },
	{ /* sentinel */ }
};

സ്ഥിതം പൂർണ്ണസംഖ്യ try_sysinfo(ശൂന്യം)
{
	ഘടന udevice *dev;
	അക്ഷരം str[80];
	പൂർണ്ണസംഖ്യ ret;

	/* This might provide more detail */
	ret = sysinfo_get(&dev);
	എങ്കിൽ (ret)
		തിരികെ ret;

	ret = sysinfo_detect(dev);
	എങ്കിൽ (ret)
		തിരികെ ret;

	ret = sysinfo_get_str(dev, SYSID_BOARD_MODEL, വലുപ്പം(str), str);
	എങ്കിൽ (ret)
		തിരികെ ret;
	printf("Model: %s\n", str);

	എങ്കിൽ (IS_ENABLED(CONFIG_SYSINFO_EXTRA)) {
		സ്ഥിരം ഘടന to_show *item;

		വേണ്ടി (item = to_show; item->id; item++) {
			ret = sysinfo_get_str(dev, item->id, വലുപ്പം(str), str);
			എങ്കിൽ (!ret)
				printf("%s: %s\n", item->name, str);
		}
	}

	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ show_board_info(ശൂന്യം)
{
	എങ്കിൽ (IS_ENABLED(CONFIG_OF_CONTROL)) {
		പൂർണ്ണസംഖ്യ ret = -ENOSYS;

		എങ്കിൽ (IS_ENABLED(CONFIG_SYSINFO))
			ret = try_sysinfo();

		/* Fail back to the main 'model' if available */
		എങ്കിൽ (ret) {
			സ്ഥിരം അക്ഷരം *model;

			model = fdt_getprop(gd->fdt_blob, 0, "model", ശൂന്യ);
			എങ്കിൽ (model)
				printf("Model: %s\n", model);
		}
	}

	തിരികെ checkboard();
}
