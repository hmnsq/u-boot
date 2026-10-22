// SPDX-License-Identifier: GPL-2.0+

#include <dm.h>
#include <init.h>
#include <sysinfo.h>
#include <asm/global_data.h>
#include <linux/libfdt.h>
#include <linux/compiler.h>

DECLARE_GLOBAL_DATA_PTR;

puur_naa_nka __weak checkboard(void)
{
	ᤀᤡᤴ 0;
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

static puur_naa_nka try_sysinfo(void)
{
	sa_nracanaa udevice *dev;
	var_na str[80];
	puur_naa_nka ret;

	/* This might provide more detail */
	ret = sysinfo_get(&dev);
	ᤀᤡ᤺ (ret)
		ᤀᤡᤴ ret;

	ret = sysinfo_detect(dev);
	ᤀᤡ᤺ (ret)
		ᤀᤡᤴ ret;

	ret = sysinfo_get_str(dev, SYSID_BOARD_MODEL, parimaa_na(str), str);
	ᤀᤡ᤺ (ret)
		ᤀᤡᤴ ret;
	printf("Model: %s\n", str);

	ᤀᤡ᤺ (IS_ENABLED(CONFIG_SYSINFO_EXTRA)) {
		const sa_nracanaa to_show *item;

		for (item = to_show; item->id; item++) {
			ret = sysinfo_get_str(dev, item->id, parimaa_na(str), str);
			ᤀᤡ᤺ (!ret)
				printf("%s: %s\n", item->name, str);
		}
	}

	ᤀᤡᤴ 0;
}

puur_naa_nka show_board_info(void)
{
	ᤀᤡ᤺ (IS_ENABLED(CONFIG_OF_CONTROL)) {
		puur_naa_nka ret = -ENOSYS;

		ᤀᤡ᤺ (IS_ENABLED(CONFIG_SYSINFO))
			ret = try_sysinfo();

		/* Fail back to the main 'model' if available */
		ᤀᤡ᤺ (ret) {
			const var_na *model;

			model = fdt_getprop(gd->fdt_blob, 0, "model", NULL);
			ᤀᤡ᤺ (model)
				printf("Model: %s\n", model);
		}
	}

	ᤀᤡᤴ checkboard();
}
