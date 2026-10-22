// SPDX-License-Identifier: GPL-2.0+

#அந்தர்பாவ <dm.h>
#அந்தர்பாவ <init.h>
#அந்தர்பாவ <sysinfo.h>
#அந்தர்பாவ <asm/global_data.h>
#அந்தர்பாவ <linux/libfdt.h>
#அந்தர்பாவ <linux/compiler.h>

DECLARE_GLOBAL_DATA_PTR;

முழு __weak checkboard(வெறுமை)
{
	திரும்பு 0;
}

நிலையான மாறிலி கட்டமைப்பு to_show {
	மாறிலி எழுத்து *name;
	எண்ணிக்கை sysinfo_id id;
} to_show[] = {
	{ "Manufacturer", SYSID_BOARD_MANUFACTURER},
	{ "Prior-stage version", SYSID_PRIOR_STAGE_VERSION },
	{ "Prior-stage date", SYSID_PRIOR_STAGE_DATE },
	{ /* sentinel */ }
};

நிலையான முழு try_sysinfo(வெறுமை)
{
	கட்டமைப்பு udevice *dev;
	எழுத்து str[80];
	முழு ret;

	/* This might provide more detail */
	ret = sysinfo_get(&dev);
	எனில் (ret)
		திரும்பு ret;

	ret = sysinfo_detect(dev);
	எனில் (ret)
		திரும்பு ret;

	ret = sysinfo_get_str(dev, SYSID_BOARD_MODEL, அளவு(str), str);
	எனில் (ret)
		திரும்பு ret;
	printf("Model: %s\n", str);

	எனில் (IS_ENABLED(CONFIG_SYSINFO_EXTRA)) {
		மாறிலி கட்டமைப்பு to_show *item;

		க்கு (item = to_show; item->id; item++) {
			ret = sysinfo_get_str(dev, item->id, அளவு(str), str);
			எனில் (!ret)
				printf("%s: %s\n", item->name, str);
		}
	}

	திரும்பு 0;
}

முழு show_board_info(வெறுமை)
{
	எனில் (IS_ENABLED(CONFIG_OF_CONTROL)) {
		முழு ret = -ENOSYS;

		எனில் (IS_ENABLED(CONFIG_SYSINFO))
			ret = try_sysinfo();

		/* Fail back to the main 'model' if available */
		எனில் (ret) {
			மாறிலி எழுத்து *model;

			model = fdt_getprop(gd->fdt_blob, 0, "model", வெற்று);
			எனில் (model)
				printf("Model: %s\n", model);
		}
	}

	திரும்பு checkboard();
}
