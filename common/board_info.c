// SPDX-License-Identifier: GPL-2.0+

#ضمن <dm.h>
#ضمن <init.h>
#ضمن <sysinfo.h>
#ضمن <asm/global_data.h>
#ضمن <linux/libfdt.h>
#ضمن <linux/compiler.h>

DECLARE_GLOBAL_داده_PTR;

صحيح __weak checkboard(فراغ)
{
	أرجع 0;
}

ساكن ثابت بنية مقصد_show {
	ثابت حرف *name;
	تعداد sysinfo_id id;
} مقصد_show[] = {
	{ "Manufacturer", SYSID_BOARD_MANUFACTURER},
	{ "Prior-stage version", SYSID_پیشین_STAGE_نسخه },
	{ "Prior-stage date", SYSID_پیشین_STAGE_DATE },
	{ /* sentinel */ }
};

ساكن صحيح try_sysinfo(فراغ)
{
	بنية udevice *dev;
	حرف str[80];
	صحيح ret;

	/* This might provide more detail */
	ret = sysinfo_get(&dev);
	إذا (ret)
		أرجع ret;

	ret = sysinfo_detect(dev);
	إذا (ret)
		أرجع ret;

	ret = sysinfo_get_str(dev, SYSID_BOARD_MODEL, حجم(str), str);
	إذا (ret)
		أرجع ret;
	چاپ_کن("Model: %s\n", str);

	إذا (IS_ENABLED(CONFIG_SYSINFO_EXTRA)) {
		ثابت بنية مقصد_show *قلم;

		لكل (قلم = مقصد_show; قلم->id; قلم++) {
			ret = sysinfo_get_str(dev, قلم->id, حجم(str), str);
			إذا (!ret)
				چاپ_کن("%s: %s\n", قلم->name, str);
		}
	}

	أرجع 0;
}

صحيح show_board_info(فراغ)
{
	إذا (IS_ENABLED(CONFIG_از_CONTROL)) {
		صحيح ret = -ENOSYS;

		إذا (IS_ENABLED(CONFIG_SYSINFO))
			ret = try_sysinfo();

		/* Fail back to the main 'model' if available */
		إذا (ret) {
			ثابت حرف *model;

			model = fdt_getprop(gd->fdt_blob, 0, "model", عدم);
			إذا (model)
				چاپ_کن("Model: %s\n", model);
		}
	}

	أرجع checkboard();
}
