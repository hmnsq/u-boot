#ضمن <command.h>
#ضمن <exports.h>
#ضمن <malloc.h>
#ضمن <spi.h>
#ضمن <i2c.h>
#ضمن <asm/global_data.h>

DECLARE_GLOBAL_معلومات_PTR;

بنانشان ڊگهو get_نسخو(فراغ)
{
	أرجع XF_نسخو;
}

#وصف_ڪريو EXPORT_FUNC(f, a, x, ...)  gd->jt->x = f;

صحيح jumptable_init(فراغ)
{
	gd->jt = يادگيري_ڏيو(حجم(بنية jt_funcs));
#ضمن <_exports.h>

	أرجع 0;
}
