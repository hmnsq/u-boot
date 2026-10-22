#ضمن <command.h>
#ضمن <exports.h>
#ضمن <malloc.h>
#ضمن <spi.h>
#ضمن <i2c.h>
#ضمن <asm/global_data.h>

DECLARE_GLOBAL_انکڑے_PTR;

بنانشانی لما get_نسخہ(فراغ)
{
	أرجع XF_نسخہ;
}

#تعریف_کرو EXPORT_FUNC(f, a, x, ...)  gd->jt->x = f;

صحيح jumptable_init(فراغ)
{
	gd->jt = حافظہ_دیو(حجم(بنية jt_funcs));
#ضمن <_exports.h>

	أرجع 0;
}
