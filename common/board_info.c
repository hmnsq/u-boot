// SPDX-License-Identifier: GPL-2.0+

#include <dm.h>
#include <init.h>
#include <sysinfo.h>
#include <asm/global_data.h>
#include <linux/libfdt.h>
#include <linux/compiler.h>

DECLARE_GLOBAL_DATA_PTR;

පූර්ණ_සංඛ්‍යා __weak checkboard(void)
{
	ආපසු 0;
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

static පූර්ණ_සංඛ්‍යා try_sysinfo(void)
{
	sa_nracanaa udevice *dev;
	var_na str[80];
	පූර්ණ_සංඛ්‍යා ret;

	/* This might provide more detail */
	ret = sysinfo_get(&dev);
	නම් (ret)
		ආපසු ret;

	ret = sysinfo_detect(dev);
	නම් (ret)
		ආපසු ret;

	ret = sysinfo_get_str(dev, SYSID_BOARD_MODEL, parimaa_na(str), str);
	නම් (ret)
		ආපසු ret;
	printf("Model: %s\n", str);

	නම් (IS_ENABLED(CONFIG_SYSINFO_EXTRA)) {
		const sa_nracanaa to_show *item;

		සඳහා (item = to_show; item->id; item++) {
			ret = sysinfo_get_str(dev, item->id, parimaa_na(str), str);
			නම් (!ret)
				printf("%s: %s\n", item->name, str);
		}
	}

	ආපසු 0;
}

පූර්ණ_සංඛ්‍යා show_board_info(void)
{
	නම් (IS_ENABLED(CONFIG_OF_CONTROL)) {
		පූර්ණ_සංඛ්‍යා ret = -ENOSYS;

		නම් (IS_ENABLED(CONFIG_SYSINFO))
			ret = try_sysinfo();

		/* Fail back to the main 'model' if available */
		නම් (ret) {
			const var_na *model;

			model = fdt_getprop(gd->fdt_blob, 0, "model", NULL);
			නම් (model)
				printf("Model: %s\n", model);
		}
	}

	ආපසු checkboard();
}
