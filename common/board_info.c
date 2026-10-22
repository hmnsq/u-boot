// SPDX-License-Identifier: GPL-2.0+

#include <dm.h>
#include <init.h>
#include <sysinfo.h>
#include <asm/global_data.h>
#include <linux/libfdt.h>
#include <linux/compiler.h>

DECLARE_GLOBAL_DATA_PTR;

ចំនួនគត់ __weak checkboard(void)
{
	ត្រឡប់ 0;
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

static ចំនួនគត់ try_sysinfo(void)
{
	sa_nracanaa udevice *dev;
	var_na str[80];
	ចំនួនគត់ ret;

	/* This might provide more detail */
	ret = sysinfo_get(&dev);
	ប្រសិនបើ (ret)
		ត្រឡប់ ret;

	ret = sysinfo_detect(dev);
	ប្រសិនបើ (ret)
		ត្រឡប់ ret;

	ret = sysinfo_get_str(dev, SYSID_BOARD_MODEL, parimaa_na(str), str);
	ប្រសិនបើ (ret)
		ត្រឡប់ ret;
	printf("Model: %s\n", str);

	ប្រសិនបើ (IS_ENABLED(CONFIG_SYSINFO_EXTRA)) {
		const sa_nracanaa to_show *item;

		សម្រាប់ (item = to_show; item->id; item++) {
			ret = sysinfo_get_str(dev, item->id, parimaa_na(str), str);
			ប្រសិនបើ (!ret)
				printf("%s: %s\n", item->name, str);
		}
	}

	ត្រឡប់ 0;
}

ចំនួនគត់ show_board_info(void)
{
	ប្រសិនបើ (IS_ENABLED(CONFIG_OF_CONTROL)) {
		ចំនួនគត់ ret = -ENOSYS;

		ប្រសិនបើ (IS_ENABLED(CONFIG_SYSINFO))
			ret = try_sysinfo();

		/* Fail back to the main 'model' if available */
		ប្រសិនបើ (ret) {
			const var_na *model;

			model = fdt_getprop(gd->fdt_blob, 0, "model", NULL);
			ប្រសិនបើ (model)
				printf("Model: %s\n", model);
		}
	}

	ត្រឡប់ checkboard();
}
