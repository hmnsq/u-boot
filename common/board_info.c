// SPDX-License-Identifier: GPL-2.0+

#شامل <dm.h>
#شامل <init.h>
#شامل <sysinfo.h>
#شامل <asm/global_data.h>
#شامل <linux/libfdt.h>
#شامل <linux/compiler.h>

DECLARE_GLOBAL_ڈیٹا_PTR;

صحیح __weak checkboard(خالی)
{
	واپس 0;
}

ساکن ثابت ساخت مقصد_show {
	ثابت حرف *name;
	شمار sysinfo_id id;
} مقصد_show[] = {
	{ "Manufacturer", SYSID_BOARD_MANUFACTURER},
	{ "Prior-stage version", SYSID_پیشگی_STAGE_نسخہ },
	{ "Prior-stage date", SYSID_پیشگی_STAGE_DATE },
	{ /* sentinel */ }
};

ساکن صحیح try_sysinfo(خالی)
{
	ساخت udevice *dev;
	حرف str[80];
	صحیح ret;

	/* This might provide more detail */
	ret = sysinfo_get(&dev);
	اگر (ret)
		واپس ret;

	ret = sysinfo_detect(dev);
	اگر (ret)
		واپس ret;

	ret = sysinfo_get_str(dev, SYSID_BOARD_MODEL, حجم(str), str);
	اگر (ret)
		واپس ret;
	لکھو("Model: %s\n", str);

	اگر (IS_ENABLED(CONFIG_SYSINFO_EXTRA)) {
		ثابت ساخت مقصد_show *شے;

		برائے (شے = مقصد_show; شے->id; شے++) {
			ret = sysinfo_get_str(dev, شے->id, حجم(str), str);
			اگر (!ret)
				لکھو("%s: %s\n", شے->name, str);
		}
	}

	واپس 0;
}

صحیح show_board_info(خالی)
{
	اگر (IS_ENABLED(CONFIG_کا_CONTROL)) {
		صحیح ret = -ENOSYS;

		اگر (IS_ENABLED(CONFIG_SYSINFO))
			ret = try_sysinfo();

		/* Fail back to the main 'model' if available */
		اگر (ret) {
			ثابت حرف *model;

			model = fdt_getprop(gd->fdt_blob, 0, "model", تہی);
			اگر (model)
				لکھو("Model: %s\n", model);
		}
	}

	واپس checkboard();
}
