// SPDX-License-Identifier: GPL-2.0+

#include <dm.h>
#include <init.h>
#include <sysinfo.h>
#include <asm/global_data.h>
#include <linux/libfdt.h>
#include <linux/compiler.h>

DECLARE_GLOBAL_DATA_PTR;

цяло __weak checkboard(void)
{
	върни 0;
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

static цяло try_sysinfo(void)
{
	struct udevice *dev;
	char str[80];
	цяло ret;

	/* This might provide more detail */
	ret = sysinfo_get(&dev);
	ако (ret)
		върни ret;

	ret = sysinfo_detect(dev);
	ако (ret)
		върни ret;

	ret = sysinfo_get_str(dev, SYSID_BOARD_MODEL, sizeof(str), str);
	ако (ret)
		върни ret;
	printf("Model: %s\n", str);

	ако (IS_ENABLED(CONFIG_SYSINFO_EXTRA)) {
		const struct to_show *item;

		за (item = to_show; item->id; item++) {
			ret = sysinfo_get_str(dev, item->id, sizeof(str), str);
			ако (!ret)
				printf("%s: %s\n", item->name, str);
		}
	}

	върни 0;
}

цяло show_board_info(void)
{
	ако (IS_ENABLED(CONFIG_OF_CONTROL)) {
		цяло ret = -ENOSYS;

		ако (IS_ENABLED(CONFIG_SYSINFO))
			ret = try_sysinfo();

		/* Fail back to the main 'model' if available */
		ако (ret) {
			const char *model;

			model = fdt_getprop(gd->fdt_blob, 0, "model", NULL);
			ако (model)
				printf("Model: %s\n", model);
		}
	}

	върни checkboard();
}
