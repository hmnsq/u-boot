// SPDX-License-Identifier: GPL-2.0+

#include <dm.h>
#include <init.h>
#include <sysinfo.h>
#include <asm/global_data.h>
#include <linux/libfdt.h>
#include <linux/compiler.h>

DECLARE_GLOBAL_DATA_PTR;

整数 __weak checkboard(void)
{
	返回 0;
}

static const struct to_show {
	const char *name;
	enum sysinfo_id id;
} to_show[] = {
	{ "Manufacturer", SYSID_BOARD_MANUFACTURER},
	{ "Prior-stage version", SYSID_PRIOR_STAGE_VERSION },
	{ "Prior-stage date", SYSID_PRIOR_STAGE_DATE },
	{ /* sentinel */ }
};

static 整数 try_sysinfo(void)
{
	struct udevice *dev;
	char str[80];
	整数 ret;

	/* This might provide more detail */
	ret = sysinfo_get(&dev);
	如果 (ret)
		返回 ret;

	ret = sysinfo_detect(dev);
	如果 (ret)
		返回 ret;

	ret = sysinfo_get_str(dev, SYSID_BOARD_MODEL, sizeof(str), str);
	如果 (ret)
		返回 ret;
	printf("Model: %s\n", str);

	如果 (IS_ENABLED(CONFIG_SYSINFO_EXTRA)) {
		const struct to_show *item;

		为 (item = to_show; item->id; item++) {
			ret = sysinfo_get_str(dev, item->id, sizeof(str), str);
			如果 (!ret)
				printf("%s: %s\n", item->name, str);
		}
	}

	返回 0;
}

整数 show_board_info(void)
{
	如果 (IS_ENABLED(CONFIG_OF_CONTROL)) {
		整数 ret = -ENOSYS;

		如果 (IS_ENABLED(CONFIG_SYSINFO))
			ret = try_sysinfo();

		/* Fail back to the main 'model' if available */
		如果 (ret) {
			const char *model;

			model = fdt_getprop(gd->fdt_blob, 0, "model", NULL);
			如果 (model)
				printf("Model: %s\n", model);
		}
	}

	返回 checkboard();
}
