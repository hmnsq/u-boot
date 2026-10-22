#అన్తర్భావ <command.h>
#అన్తర్భావ <exports.h>
#అన్తర్భావ <malloc.h>
#అన్తర్భావ <spi.h>
#అన్తర్భావ <i2c.h>
#అన్తర్భావ <asm/global_data.h>

DECLARE_GLOBAL_దత్తాంశం_PTR;

గుర్తులేని దీర్ఘ get_సంస్కరణ(శూన్య)
{
	తిరిగి XF_సంస్కరణ;
}

#నిర్వచించు EXPORT_FUNC(f, a, x, ...)  gd->jt->x = f;

పూర్ణాంకం jumptable_init(శూన్య)
{
	gd->jt = జ్ఞాపకం_ఇవ్వు(పరిమాణం(నిర్మాణం jt_funcs));
#అన్తర్భావ <_exports.h>

	తిరిగి 0;
}
