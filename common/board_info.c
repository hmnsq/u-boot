// SPDX-License-Identifier: GPL-2.0+

#include <dm.h>
#include <init.h>
#include <sysinfo.h>
#include <asm/global_data.h>
#include <linux/libfdt.h>
#include <linux/compiler.h>

DECLARE_GLOBAL_DATA_PTR;

целое __weak checkboard(void)
{
	вернуть 0;
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

static целое try_sysinfo(void)
{
	struct udevice *dev;
	char str[80];
	целое ret;

	/* This might provide more detail */
	ret = sysinfo_get(&dev);
	если (ret)
		вернуть ret;

	ret = sysinfo_detect(dev);
	если (ret)
		вернуть ret;

	ret = sysinfo_get_str(dev, SYSID_BOARD_MODEL, sizeof(str), str);
	если (ret)
		вернуть ret;
	printf("Model: %s\n", str);

	если (IS_ENABLED(CONFIG_SYSINFO_EXTRA)) {
		const struct to_show *item;

		для (item = to_show; item->id; item++) {
			ret = sysinfo_get_str(dev, item->id, sizeof(str), str);
			если (!ret)
				printf("%s: %s\n", item->name, str);
		}
	}

	вернуть 0;
}

целое show_board_info(void)
{
	если (IS_ENABLED(CONFIG_OF_CONTROL)) {
		целое ret = -ENOSYS;

		если (IS_ENABLED(CONFIG_SYSINFO))
			ret = try_sysinfo();

		/* Fail back to the main 'model' if available */
		если (ret) {
			const char *model;

			model = fdt_getprop(gd->fdt_blob, 0, "model", NULL);
			если (model)
				printf("Model: %s\n", model);
		}
	}

	вернуть checkboard();
}
