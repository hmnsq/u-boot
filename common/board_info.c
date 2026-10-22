// SPDX-License-Identifier: GPL-2.0+

#अन्तर्भाव <dm.h>
#अन्तर्भाव <init.h>
#अन्तर्भाव <sysinfo.h>
#अन्तर्भाव <asm/global_data.h>
#अन्तर्भाव <linux/libfdt.h>
#अन्तर्भाव <linux/compiler.h>

DECLARE_GLOBAL_दत्त_PTR;

पूर्णाङ्क __weak checkboard(शून्य)
{
	प्रत्यागम 0;
}

स्थिर ध्रुव संरचना गन्तव्यम्_show {
	ध्रुव वर्ण *name;
	गणना sysinfo_id id;
} गन्तव्यम्_show[] = {
	{ "Manufacturer", SYSID_BOARD_MANUFACTURER},
	{ "Prior-stage version", SYSID_प्राक्_STAGE_संस्करणम् },
	{ "Prior-stage date", SYSID_प्राक्_STAGE_DATE },
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
		प्रत्यागम ret;

	ret = sysinfo_detect(dev);
	यदि (ret)
		प्रत्यागम ret;

	ret = sysinfo_get_str(dev, SYSID_BOARD_MODEL, परिमाण(str), str);
	यदि (ret)
		प्रत्यागम ret;
	प्रारूप_लिख("Model: %s\n", str);

	यदि (IS_ENABLED(CONFIG_SYSINFO_EXTRA)) {
		ध्रुव संरचना गन्तव्यम्_show *item;

		प्रति (item = गन्तव्यम्_show; item->id; item++) {
			ret = sysinfo_get_str(dev, item->id, परिमाण(str), str);
			यदि (!ret)
				प्रारूप_लिख("%s: %s\n", item->name, str);
		}
	}

	प्रत्यागम 0;
}

पूर्णाङ्क show_board_info(शून्य)
{
	यदि (IS_ENABLED(CONFIG_सम्बन्धि_CONTROL)) {
		पूर्णाङ्क ret = -ENOSYS;

		यदि (IS_ENABLED(CONFIG_SYSINFO))
			ret = try_sysinfo();

		/* Fail back to the main 'model' if available */
		यदि (ret) {
			ध्रुव वर्ण *model;

			model = fdt_getprop(gd->fdt_blob, 0, "model", रिक्त);
			यदि (model)
				प्रारूप_लिख("Model: %s\n", model);
		}
	}

	प्रत्यागम checkboard();
}
