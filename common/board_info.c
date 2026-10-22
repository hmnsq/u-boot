// SPDX-License-Identifier: GPL-2.0+

#include <dm.h>
#include <init.h>
#include <sysinfo.h>
#include <asm/global_data.h>
#include <linux/libfdt.h>
#include <linux/compiler.h>

DECLARE_GLOBAL_DATA_PTR;

ціле __weak checkboard(void)
{
	повернути 0;
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

static ціле try_sysinfo(void)
{
	struct udevice *dev;
	char str[80];
	ціле ret;

	/* This might provide more detail */
	ret = sysinfo_get(&dev);
	якщо (ret)
		повернути ret;

	ret = sysinfo_detect(dev);
	якщо (ret)
		повернути ret;

	ret = sysinfo_get_str(dev, SYSID_BOARD_MODEL, sizeof(str), str);
	якщо (ret)
		повернути ret;
	printf("Model: %s\n", str);

	якщо (IS_ENABLED(CONFIG_SYSINFO_EXTRA)) {
		const struct to_show *item;

		для (item = to_show; item->id; item++) {
			ret = sysinfo_get_str(dev, item->id, sizeof(str), str);
			якщо (!ret)
				printf("%s: %s\n", item->name, str);
		}
	}

	повернути 0;
}

ціле show_board_info(void)
{
	якщо (IS_ENABLED(CONFIG_OF_CONTROL)) {
		ціле ret = -ENOSYS;

		якщо (IS_ENABLED(CONFIG_SYSINFO))
			ret = try_sysinfo();

		/* Fail back to the main 'model' if available */
		якщо (ret) {
			const char *model;

			model = fdt_getprop(gd->fdt_blob, 0, "model", NULL);
			якщо (model)
				printf("Model: %s\n", model);
		}
	}

	повернути checkboard();
}
