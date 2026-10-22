// SPDX-License-Identifier: GPL-2.0+

#include <dm.h>
#include <init.h>
#include <sysinfo.h>
#include <asm/global_data.h>
#include <linux/libfdt.h>
#include <linux/compiler.h>

DECLARE_GLOBAL_DATA_PTR;

số_nguyên __weak checkboard(void)
{
	trả_về 0;
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

static số_nguyên try_sysinfo(void)
{
	struct udevice *dev;
	char str[80];
	số_nguyên ret;

	/* This might provide more detail */
	ret = sysinfo_get(&dev);
	nếu (ret)
		trả_về ret;

	ret = sysinfo_detect(dev);
	nếu (ret)
		trả_về ret;

	ret = sysinfo_get_str(dev, SYSID_BOARD_MODEL, sizeof(str), str);
	nếu (ret)
		trả_về ret;
	printf("Model: %s\n", str);

	nếu (IS_ENABLED(CONFIG_SYSINFO_EXTRA)) {
		const struct to_show *item;

		cho (item = to_show; item->id; item++) {
			ret = sysinfo_get_str(dev, item->id, sizeof(str), str);
			nếu (!ret)
				printf("%s: %s\n", item->name, str);
		}
	}

	trả_về 0;
}

số_nguyên show_board_info(void)
{
	nếu (IS_ENABLED(CONFIG_OF_CONTROL)) {
		số_nguyên ret = -ENOSYS;

		nếu (IS_ENABLED(CONFIG_SYSINFO))
			ret = try_sysinfo();

		/* Fail back to the main 'model' if available */
		nếu (ret) {
			const char *model;

			model = fdt_getprop(gd->fdt_blob, 0, "model", NULL);
			nếu (model)
				printf("Model: %s\n", model);
		}
	}

	trả_về checkboard();
}
