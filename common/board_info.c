// SPDX-License-Identifier: GPL-2.0+

#అన్తర్భావ <dm.h>
#అన్తర్భావ <init.h>
#అన్తర్భావ <sysinfo.h>
#అన్తర్భావ <asm/global_data.h>
#అన్తర్భావ <linux/libfdt.h>
#అన్తర్భావ <linux/compiler.h>

DECLARE_GLOBAL_దత్తాంశం_PTR;

పూర్ణాంకం __weak checkboard(శూన్య)
{
	తిరిగి 0;
}

స్థిర స్థిరరాశి నిర్మాణం గమ్యం_show {
	స్థిరరాశి అక్షరం *name;
	గణన sysinfo_id id;
} గమ్యం_show[] = {
	{ "Manufacturer", SYSID_BOARD_MANUFACTURER},
	{ "Prior-stage version", SYSID_పూర్వ_STAGE_సంస్కరణ },
	{ "Prior-stage date", SYSID_పూర్వ_STAGE_DATE },
	{ /* sentinel */ }
};

స్థిర పూర్ణాంకం try_sysinfo(శూన్య)
{
	నిర్మాణం udevice *dev;
	అక్షరం str[80];
	పూర్ణాంకం ret;

	/* This might provide more detail */
	ret = sysinfo_get(&dev);
	ఒకవేళ (ret)
		తిరిగి ret;

	ret = sysinfo_detect(dev);
	ఒకవేళ (ret)
		తిరిగి ret;

	ret = sysinfo_get_str(dev, SYSID_BOARD_MODEL, పరిమాణం(str), str);
	ఒకవేళ (ret)
		తిరిగి ret;
	రాయి("Model: %s\n", str);

	ఒకవేళ (IS_ENABLED(CONFIG_SYSINFO_EXTRA)) {
		స్థిరరాశి నిర్మాణం గమ్యం_show *అంశం;

		కొరకు (అంశం = గమ్యం_show; అంశం->id; అంశం++) {
			ret = sysinfo_get_str(dev, అంశం->id, పరిమాణం(str), str);
			ఒకవేళ (!ret)
				రాయి("%s: %s\n", అంశం->name, str);
		}
	}

	తిరిగి 0;
}

పూర్ణాంకం show_board_info(శూన్య)
{
	ఒకవేళ (IS_ENABLED(CONFIG_యొక్క_CONTROL)) {
		పూర్ణాంకం ret = -ENOSYS;

		ఒకవేళ (IS_ENABLED(CONFIG_SYSINFO))
			ret = try_sysinfo();

		/* Fail back to the main 'model' if available */
		ఒకవేళ (ret) {
			స్థిరరాశి అక్షరం *model;

			model = fdt_getprop(gd->fdt_blob, 0, "model", ఖాళీ);
			ఒకవేళ (model)
				రాయి("Model: %s\n", model);
		}
	}

	తిరిగి checkboard();
}
