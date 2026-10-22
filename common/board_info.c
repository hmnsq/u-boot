// SPDX-License-Identifier: GPL-2.0+

#include <dm.h>
#include <init.h>
#include <sysinfo.h>
#include <asm/global_data.h>
#include <linux/libfdt.h>
#include <linux/compiler.h>

DECLARE_GLOBAL_DATA_PTR;

ကိန်းပြည့် __weak checkboard(void)
{
	ပြန်ပေး 0;
}

static const sa_nracanaa to_show {
	const var_na *name;
	ga_nanaa sysinfo_id id;
} to_show[] = {
	{ "Manufacturer", SYSID_BOARD_MANUFACTURER},
	{ "Prior-stage version", SYSID_PRIOR_STAGE_VERSION },
	{ "Prior-stage date", SYSID_PRIOR_STAGE_DATE },
	{ /* sentinel */ }
};

static ကိန်းပြည့် try_sysinfo(void)
{
	sa_nracanaa udevice *dev;
	var_na str[80];
	ကိန်းပြည့် ret;

	/* This might provide more detail */
	ret = sysinfo_get(&dev);
	အကယ်၍ (ret)
		ပြန်ပေး ret;

	ret = sysinfo_detect(dev);
	အကယ်၍ (ret)
		ပြန်ပေး ret;

	ret = sysinfo_get_str(dev, SYSID_BOARD_MODEL, parimaa_na(str), str);
	အကယ်၍ (ret)
		ပြန်ပေး ret;
	printf("Model: %s\n", str);

	အကယ်၍ (IS_ENABLED(CONFIG_SYSINFO_EXTRA)) {
		const sa_nracanaa to_show *item;

		အတွက် (item = to_show; item->id; item++) {
			ret = sysinfo_get_str(dev, item->id, parimaa_na(str), str);
			အကယ်၍ (!ret)
				printf("%s: %s\n", item->name, str);
		}
	}

	ပြန်ပေး 0;
}

ကိန်းပြည့် show_board_info(void)
{
	အကယ်၍ (IS_ENABLED(CONFIG_OF_CONTROL)) {
		ကိန်းပြည့် ret = -ENOSYS;

		အကယ်၍ (IS_ENABLED(CONFIG_SYSINFO))
			ret = try_sysinfo();

		/* Fail back to the main 'model' if available */
		အကယ်၍ (ret) {
			const var_na *model;

			model = fdt_getprop(gd->fdt_blob, 0, "model", NULL);
			အကယ်၍ (model)
				printf("Model: %s\n", model);
		}
	}

	ပြန်ပေး checkboard();
}
