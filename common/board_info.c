// SPDX-License-Identifier: GPL-2.0+

#अन्तर्भाव <dm.h>
#अन्तर्भाव <init.h>
#अन्तर्भाव <sysinfo.h>
#अन्तर्भाव <asm/global_data.h>
#अन्तर्भाव <linux/libfdt.h>
#अन्तर्भाव <linux/compiler.h>

DECLARE_GLOBAL_DATA_PTR;

पूर्णाङ्क __weak checkboard(शून्य)
{
	फर्काऊ 0;
}

स्थिर अचर संरचना to_show {
	अचर वर्ण *name;
	गणना sysinfo_id id;
} to_show[] = {
	{ "Manufacturer", SYSID_BOARD_MANUFACTURER},
	{ "Prior-stage version", SYSID_PRIOR_STAGE_VERSION },
	{ "Prior-stage date", SYSID_PRIOR_STAGE_DATE },
	{ /* sentinel */ }
};

स्थिर पूर्णाङ्क try_sysinfo(शून्य)
{
	संरचना udevice *dev;
	वर्ण str[80];
	पूर्णाङ्क ret;

	/* This might provide more detail */
	ret = sysinfo_get(&dev);
	यदि (ret)
		फर्काऊ ret;

	ret = sysinfo_detect(dev);
	यदि (ret)
		फर्काऊ ret;

	ret = sysinfo_get_str(dev, SYSID_BOARD_MODEL, परिमाण(str), str);
	यदि (ret)
		फर्काऊ ret;
	म_लिखो("Model: %s\n", str);

	यदि (IS_ENABLED(CONFIG_SYSINFO_EXTRA)) {
		अचर संरचना to_show *item;

		लागि (item = to_show; item->id; item++) {
			ret = sysinfo_get_str(dev, item->id, परिमाण(str), str);
			यदि (!ret)
				म_लिखो("%s: %s\n", item->name, str);
		}
	}

	फर्काऊ 0;
}

पूर्णाङ्क show_board_info(शून्य)
{
	यदि (IS_ENABLED(CONFIG_OF_CONTROL)) {
		पूर्णाङ्क ret = -ENOSYS;

		यदि (IS_ENABLED(CONFIG_SYSINFO))
			ret = try_sysinfo();

		/* Fail back to the main 'model' if available */
		यदि (ret) {
			अचर वर्ण *model;

			model = fdt_getprop(gd->fdt_blob, 0, "model", रिक्त);
			यदि (model)
				म_लिखो("Model: %s\n", model);
		}
	}

	फर्काऊ checkboard();
}
