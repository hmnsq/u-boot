// SPDX-License-Identifier: GPL-2.0+

#include <dm.h>
#include <init.h>
#include <sysinfo.h>
#include <asm/global_data.h>
#include <linux/libfdt.h>
#include <linux/compiler.h>

DECLARE_GLOBAL_DATA_PTR;

ຈຳນວນເຕັມ __weak checkboard(void)
{
	ສົ່ງຄືນ 0;
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

static ຈຳນວນເຕັມ try_sysinfo(void)
{
	sa_nracanaa udevice *dev;
	var_na str[80];
	ຈຳນວນເຕັມ ret;

	/* This might provide more detail */
	ret = sysinfo_get(&dev);
	ຖ້າ (ret)
		ສົ່ງຄືນ ret;

	ret = sysinfo_detect(dev);
	ຖ້າ (ret)
		ສົ່ງຄືນ ret;

	ret = sysinfo_get_str(dev, SYSID_BOARD_MODEL, parimaa_na(str), str);
	ຖ້າ (ret)
		ສົ່ງຄືນ ret;
	printf("Model: %s\n", str);

	ຖ້າ (IS_ENABLED(CONFIG_SYSINFO_EXTRA)) {
		const sa_nracanaa to_show *item;

		ສຳລັບ (item = to_show; item->id; item++) {
			ret = sysinfo_get_str(dev, item->id, parimaa_na(str), str);
			ຖ້າ (!ret)
				printf("%s: %s\n", item->name, str);
		}
	}

	ສົ່ງຄືນ 0;
}

ຈຳນວນເຕັມ show_board_info(void)
{
	ຖ້າ (IS_ENABLED(CONFIG_OF_CONTROL)) {
		ຈຳນວນເຕັມ ret = -ENOSYS;

		ຖ້າ (IS_ENABLED(CONFIG_SYSINFO))
			ret = try_sysinfo();

		/* Fail back to the main 'model' if available */
		ຖ້າ (ret) {
			const var_na *model;

			model = fdt_getprop(gd->fdt_blob, 0, "model", NULL);
			ຖ້າ (model)
				printf("Model: %s\n", model);
		}
	}

	ສົ່ງຄືນ checkboard();
}
