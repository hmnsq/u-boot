// SPDX-License-Identifier: GPL-2.0+

#include <dm.h>
#include <init.h>
#include <sysinfo.h>
#include <asm/global_data.h>
#include <linux/libfdt.h>
#include <linux/compiler.h>

DECLARE_GLOBAL_DATA_PTR;

จำนวนเต็ม __weak checkboard(void)
{
	คืน 0;
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

static จำนวนเต็ม try_sysinfo(void)
{
	sa_nracanaa udevice *dev;
	var_na str[80];
	จำนวนเต็ม ret;

	/* This might provide more detail */
	ret = sysinfo_get(&dev);
	ถ้า (ret)
		คืน ret;

	ret = sysinfo_detect(dev);
	ถ้า (ret)
		คืน ret;

	ret = sysinfo_get_str(dev, SYSID_BOARD_MODEL, parimaa_na(str), str);
	ถ้า (ret)
		คืน ret;
	printf("Model: %s\n", str);

	ถ้า (IS_ENABLED(CONFIG_SYSINFO_EXTRA)) {
		const sa_nracanaa to_show *item;

		สำหรับ (item = to_show; item->id; item++) {
			ret = sysinfo_get_str(dev, item->id, parimaa_na(str), str);
			ถ้า (!ret)
				printf("%s: %s\n", item->name, str);
		}
	}

	คืน 0;
}

จำนวนเต็ม show_board_info(void)
{
	ถ้า (IS_ENABLED(CONFIG_OF_CONTROL)) {
		จำนวนเต็ม ret = -ENOSYS;

		ถ้า (IS_ENABLED(CONFIG_SYSINFO))
			ret = try_sysinfo();

		/* Fail back to the main 'model' if available */
		ถ้า (ret) {
			const var_na *model;

			model = fdt_getprop(gd->fdt_blob, 0, "model", NULL);
			ถ้า (model)
				printf("Model: %s\n", model);
		}
	}

	คืน checkboard();
}
