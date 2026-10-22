// SPDX-License-Identifier: GPL-2.0+

#अन्तर्भाव <dm.h>
#अन्तर्भाव <init.h>
#अन्तर्भाव <sysinfo.h>
#अन्तर्भाव <asm/global_data.h>
#अन्तर्भाव <linux/libfdt.h>
#अन्तर्भाव <linux/compiler.h>

DECLARE_GLOBAL_DATA_PTR;

पूर्णांक __weak checkboard(शून्य)
{
	परत 0;
}

स्थिर स्थिरांक संरचना to_show {
	स्थिरांक अक्षर *name;
	गणना sysinfo_id id;
} to_show[] = {
	{ "Manufacturer", SYSID_BOARD_MANUFACTURER},
	{ "Prior-stage version", SYSID_PRIOR_STAGE_VERSION },
	{ "Prior-stage date", SYSID_PRIOR_STAGE_DATE },
	{ /* sentinel */ }
};

स्थिर पूर्णांक try_sysinfo(शून्य)
{
	संरचना udevice *dev;
	अक्षर str[80];
	पूर्णांक ret;

	/* This might provide more detail */
	ret = sysinfo_get(&dev);
	जर (ret)
		परत ret;

	ret = sysinfo_detect(dev);
	जर (ret)
		परत ret;

	ret = sysinfo_get_str(dev, SYSID_BOARD_MODEL, आकार(str), str);
	जर (ret)
		परत ret;
	म_लिखो("Model: %s\n", str);

	जर (IS_ENABLED(CONFIG_SYSINFO_EXTRA)) {
		स्थिरांक संरचना to_show *item;

		साठी (item = to_show; item->id; item++) {
			ret = sysinfo_get_str(dev, item->id, आकार(str), str);
			जर (!ret)
				म_लिखो("%s: %s\n", item->name, str);
		}
	}

	परत 0;
}

पूर्णांक show_board_info(शून्य)
{
	जर (IS_ENABLED(CONFIG_OF_CONTROL)) {
		पूर्णांक ret = -ENOSYS;

		जर (IS_ENABLED(CONFIG_SYSINFO))
			ret = try_sysinfo();

		/* Fail back to the main 'model' if available */
		जर (ret) {
			स्थिरांक अक्षर *model;

			model = fdt_getprop(gd->fdt_blob, 0, "model", रिक्त);
			जर (model)
				म_लिखो("Model: %s\n", model);
		}
	}

	परत checkboard();
}
