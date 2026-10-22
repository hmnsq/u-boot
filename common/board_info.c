// SPDX-License-Identifier: GPL-2.0+

#અન્તર્ભાવ <dm.h>
#અન્તર્ભાવ <init.h>
#અન્તર્ભાવ <sysinfo.h>
#અન્તર્ભાવ <asm/global_data.h>
#અન્તર્ભાવ <linux/libfdt.h>
#અન્તર્ભાવ <linux/compiler.h>

DECLARE_GLOBAL_DATA_PTR;

પૂર્ણાંક __weak checkboard(શૂન્ય)
{
	પાછા 0;
}

સ્થિર અચલ સંરચના to_show {
	અચલ અક્ષર *name;
	ગણના sysinfo_id id;
} to_show[] = {
	{ "Manufacturer", SYSID_BOARD_MANUFACTURER},
	{ "Prior-stage version", SYSID_PRIOR_STAGE_VERSION },
	{ "Prior-stage date", SYSID_PRIOR_STAGE_DATE },
	{ /* sentinel */ }
};

સ્થિર પૂર્ણાંક try_sysinfo(શૂન્ય)
{
	સંરચના udevice *dev;
	અક્ષર str[80];
	પૂર્ણાંક ret;

	/* This might provide more detail */
	ret = sysinfo_get(&dev);
	જો (ret)
		પાછા ret;

	ret = sysinfo_detect(dev);
	જો (ret)
		પાછા ret;

	ret = sysinfo_get_str(dev, SYSID_BOARD_MODEL, કદ(str), str);
	જો (ret)
		પાછા ret;
	printf("Model: %s\n", str);

	જો (IS_ENABLED(CONFIG_SYSINFO_EXTRA)) {
		અચલ સંરચના to_show *item;

		માટે (item = to_show; item->id; item++) {
			ret = sysinfo_get_str(dev, item->id, કદ(str), str);
			જો (!ret)
				printf("%s: %s\n", item->name, str);
		}
	}

	પાછા 0;
}

પૂર્ણાંક show_board_info(શૂન્ય)
{
	જો (IS_ENABLED(CONFIG_OF_CONTROL)) {
		પૂર્ણાંક ret = -ENOSYS;

		જો (IS_ENABLED(CONFIG_SYSINFO))
			ret = try_sysinfo();

		/* Fail back to the main 'model' if available */
		જો (ret) {
			અચલ અક્ષર *model;

			model = fdt_getprop(gd->fdt_blob, 0, "model", ખાલી);
			જો (model)
				printf("Model: %s\n", model);
		}
	}

	પાછા checkboard();
}
