// SPDX-License-Identifier: GPL-2.0+

#ಅನ್ತರ್ಭಾವ <dm.h>
#ಅನ್ತರ್ಭಾವ <init.h>
#ಅನ್ತರ್ಭಾವ <sysinfo.h>
#ಅನ್ತರ್ಭಾವ <asm/global_data.h>
#ಅನ್ತರ್ಭಾವ <linux/libfdt.h>
#ಅನ್ತರ್ಭಾವ <linux/compiler.h>

DECLARE_GLOBAL_ದತ್ತಾಂಶ_PTR;

ಪೂರ್ಣಾಂಕ __weak checkboard(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ ಗಮ್ಯ_show {
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name;
	ಎಣಿಕೆ sysinfo_id id;
} ಗಮ್ಯ_show[] = {
	{ "Manufacturer", SYSID_BOARD_MANUFACTURER},
	{ "Prior-stage version", SYSID_ಪೂರ್ವ_STAGE_ಆವೃತ್ತಿ },
	{ "Prior-stage date", SYSID_ಪೂರ್ವ_STAGE_DATE },
	{ /* sentinel */ }
};

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ try_sysinfo(ಶೂನ್ಯ)
{
	ರಚನೆ udevice *dev;
	ಅಕ್ಷರ str[80];
	ಪೂರ್ಣಾಂಕ ret;

	/* This might provide more detail */
	ret = sysinfo_get(&dev);
	ಆದರೆ (ret)
		ಹಿಂತಿರುಗಿ ret;

	ret = sysinfo_detect(dev);
	ಆದರೆ (ret)
		ಹಿಂತಿರುಗಿ ret;

	ret = sysinfo_get_str(dev, SYSID_BOARD_MODEL, ಗಾತ್ರ(str), str);
	ಆದರೆ (ret)
		ಹಿಂತಿರುಗಿ ret;
	ಬರೆ("Model: %s\n", str);

	ಆದರೆ (IS_ENABLED(CONFIG_SYSINFO_EXTRA)) {
		ಸ್ಥಿರಾಂಕ ರಚನೆ ಗಮ್ಯ_show *ಅಂಶ;

		ಗಾಗಿ (ಅಂಶ = ಗಮ್ಯ_show; ಅಂಶ->id; ಅಂಶ++) {
			ret = sysinfo_get_str(dev, ಅಂಶ->id, ಗಾತ್ರ(str), str);
			ಆದರೆ (!ret)
				ಬರೆ("%s: %s\n", ಅಂಶ->name, str);
		}
	}

	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ show_board_info(ಶೂನ್ಯ)
{
	ಆದರೆ (IS_ENABLED(CONFIG_ದ_CONTROL)) {
		ಪೂರ್ಣಾಂಕ ret = -ENOSYS;

		ಆದರೆ (IS_ENABLED(CONFIG_SYSINFO))
			ret = try_sysinfo();

		/* Fail back to the main 'model' if available */
		ಆದರೆ (ret) {
			ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *model;

			model = fdt_getprop(gd->fdt_blob, 0, "model", ಖಾಲಿ);
			ಆದರೆ (model)
				ಬರೆ("Model: %s\n", model);
		}
	}

	ಹಿಂತಿರುಗಿ checkboard();
}
