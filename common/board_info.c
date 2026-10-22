// SPDX-License-Identifier: GPL-2.0+

#ଅନ୍ତର୍ଭାଵ <dm.h>
#ଅନ୍ତର୍ଭାଵ <init.h>
#ଅନ୍ତର୍ଭାଵ <sysinfo.h>
#ଅନ୍ତର୍ଭାଵ <asm/global_data.h>
#ଅନ୍ତର୍ଭାଵ <linux/libfdt.h>
#ଅନ୍ତର୍ଭାଵ <linux/compiler.h>

DECLARE_GLOBAL_DATA_PTR;

ପୂର୍ଣ୍ଣସଂଖ୍ୟା __weak checkboard(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ଅଚଳ ସଂରଚନା to_show {
	ଅଚଳ ଅକ୍ଷର *name;
	ଗଣନା sysinfo_id id;
} to_show[] = {
	{ "Manufacturer", SYSID_BOARD_MANUFACTURER},
	{ "Prior-stage version", SYSID_PRIOR_STAGE_VERSION },
	{ "Prior-stage date", SYSID_PRIOR_STAGE_DATE },
	{ /* sentinel */ }
};

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା try_sysinfo(ଶୂନ୍ୟ)
{
	ସଂରଚନା udevice *dev;
	ଅକ୍ଷର str[80];
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

	/* This might provide more detail */
	ret = sysinfo_get(&dev);
	ଯଦି (ret)
		ଫେରନ୍ତୁ ret;

	ret = sysinfo_detect(dev);
	ଯଦି (ret)
		ଫେରନ୍ତୁ ret;

	ret = sysinfo_get_str(dev, SYSID_BOARD_MODEL, ଆକାର(str), str);
	ଯଦି (ret)
		ଫେରନ୍ତୁ ret;
	printf("Model: %s\n", str);

	ଯଦି (IS_ENABLED(CONFIG_SYSINFO_EXTRA)) {
		ଅଚଳ ସଂରଚନା to_show *item;

		ପାଇଁ (item = to_show; item->id; item++) {
			ret = sysinfo_get_str(dev, item->id, ଆକାର(str), str);
			ଯଦି (!ret)
				printf("%s: %s\n", item->name, str);
		}
	}

	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା show_board_info(ଶୂନ୍ୟ)
{
	ଯଦି (IS_ENABLED(CONFIG_OF_CONTROL)) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = -ENOSYS;

		ଯଦି (IS_ENABLED(CONFIG_SYSINFO))
			ret = try_sysinfo();

		/* Fail back to the main 'model' if available */
		ଯଦି (ret) {
			ଅଚଳ ଅକ୍ଷର *model;

			model = fdt_getprop(gd->fdt_blob, 0, "model", ଖାଲି);
			ଯଦି (model)
				printf("Model: %s\n", model);
		}
	}

	ଫେରନ୍ତୁ checkboard();
}
