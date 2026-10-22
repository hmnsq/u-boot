// SPDX-License-Identifier: GPL-2.0+

#include <dm.h>
#include <init.h>
#include <sysinfo.h>
#include <asm/global_data.h>
#include <linux/libfdt.h>
#include <linux/compiler.h>

DECLARE_GLOBAL_DATA_PTR;

ཧྲིལ __weak checkboard(void)
{
	ཕྱིར 0;
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

static ཧྲིལ try_sysinfo(void)
{
	sa_nracanaa udevice *dev;
	var_na str[80];
	ཧྲིལ ret;

	/* This might provide more detail */
	ret = sysinfo_get(&dev);
	གལ་ཏེ (ret)
		ཕྱིར ret;

	ret = sysinfo_detect(dev);
	གལ་ཏེ (ret)
		ཕྱིར ret;

	ret = sysinfo_get_str(dev, SYSID_BOARD_MODEL, parimaa_na(str), str);
	གལ་ཏེ (ret)
		ཕྱིར ret;
	printf("Model: %s\n", str);

	གལ་ཏེ (IS_ENABLED(CONFIG_SYSINFO_EXTRA)) {
		const sa_nracanaa to_show *item;

		ཆེད་དུ (item = to_show; item->id; item++) {
			ret = sysinfo_get_str(dev, item->id, parimaa_na(str), str);
			གལ་ཏེ (!ret)
				printf("%s: %s\n", item->name, str);
		}
	}

	ཕྱིར 0;
}

ཧྲིལ show_board_info(void)
{
	གལ་ཏེ (IS_ENABLED(CONFIG_OF_CONTROL)) {
		ཧྲིལ ret = -ENOSYS;

		གལ་ཏེ (IS_ENABLED(CONFIG_SYSINFO))
			ret = try_sysinfo();

		/* Fail back to the main 'model' if available */
		གལ་ཏེ (ret) {
			const var_na *model;

			model = fdt_getprop(gd->fdt_blob, 0, "model", NULL);
			གལ་ཏེ (model)
				printf("Model: %s\n", model);
		}
	}

	ཕྱིར checkboard();
}
