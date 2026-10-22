// SPDX-License-Identifier: GPL-2.0+

#समावेश <dm.h>
#समावेश <init.h>
#समावेश <sysinfo.h>
#समावेश <asm/global_data.h>
#समावेश <linux/libfdt.h>
#समावेश <linux/compiler.h>

DECLARE_GLOBAL_तथ्य_PTR;

पूर्णांक __weak checkboard(व्योम)
{
	वापस 0;
}

अटल स्थिर काष्ठा to_show {
	स्थिर अक्षर *name;
	क्रमागत sysinfo_id id;
} to_show[] = {
	{ "Manufacturer", SYSID_BOARD_MANUFACTURER},
	{ "Prior-stage version", SYSID_PRIOR_STAGE_VERSION },
	{ "Prior-stage date", SYSID_PRIOR_STAGE_DATE },
	{ /* sentinel */ }
};

अटल पूर्णांक try_sysinfo(व्योम)
{
	काष्ठा udevice *dev;
	अक्षर str[80];
	पूर्णांक ret;

	/* This might provide more detail */
	ret = sysinfo_get(&dev);
	अगर (ret)
		वापस ret;

	ret = sysinfo_detect(dev);
	अगर (ret)
		वापस ret;

	ret = sysinfo_get_str(dev, SYSID_BOARD_MODEL, माप(str), str);
	अगर (ret)
		वापस ret;
	म_लिखो("Model: %s\n", str);

	अगर (IS_ENABLED(CONFIG_SYSINFO_EXTRA)) {
		स्थिर काष्ठा to_show *item;

		क्रम (item = to_show; item->id; item++) {
			ret = sysinfo_get_str(dev, item->id, माप(str), str);
			अगर (!ret)
				म_लिखो("%s: %s\n", item->name, str);
		}
	}

	वापस 0;
}

पूर्णांक show_board_info(व्योम)
{
	अगर (IS_ENABLED(CONFIG_OF_CONTROL)) {
		पूर्णांक ret = -ENOSYS;

		अगर (IS_ENABLED(CONFIG_SYSINFO))
			ret = try_sysinfo();

		/* Fail back to the main 'model' if available */
		अगर (ret) {
			स्थिर अक्षर *model;

			model = fdt_getprop(gd->fdt_blob, 0, "model", शून्य);
			अगर (model)
				म_लिखो("Model: %s\n", model);
		}
	}

	वापस checkboard();
}
