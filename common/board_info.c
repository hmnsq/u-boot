// SPDX-License-Identifier: GPL-2.0+

#ضمن <dm.h>
#ضمن <init.h>
#ضمن <sysinfo.h>
#ضمن <asm/global_data.h>
#ضمن <linux/libfdt.h>
#ضمن <linux/compiler.h>

DECLARE_GLOBAL_معلومات_PTR;

صحيح __weak checkboard(فراغ)
{
	أرجع 0;
}

ساكن ثابت بنية منزل_show {
	ثابت حرف *name;
	تعداد sysinfo_id id;
} منزل_show[] = {
	{ "Manufacturer", SYSID_BOARD_MANUFACTURER},
	{ "Prior-stage version", SYSID_اڳواٽ_STAGE_نسخو },
	{ "Prior-stage date", SYSID_اڳواٽ_STAGE_DATE },
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
	لکو("Model: %s\n", str);

	إذا (IS_ENABLED(CONFIG_SYSINFO_EXTRA)) {
		ثابت بنية منزل_show *شي;

		لكل (شي = منزل_show; شي->id; شي++) {
			ret = sysinfo_get_str(dev, شي->id, حجم(str), str);
			إذا (!ret)
				لکو("%s: %s\n", شي->name, str);
		}
	}

	أرجع 0;
}

صحيح show_board_info(فراغ)
{
	إذا (IS_ENABLED(CONFIG_جو_CONTROL)) {
		صحيح ret = -ENOSYS;

		إذا (IS_ENABLED(CONFIG_SYSINFO))
			ret = try_sysinfo();

		/* Fail back to the main 'model' if available */
		إذا (ret) {
			ثابت حرف *model;

			model = fdt_getprop(gd->fdt_blob, 0, "model", عدم);
			إذا (model)
				لکو("Model: %s\n", model);
		}
	}

	أرجع checkboard();
}
