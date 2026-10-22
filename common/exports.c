#ضمن <command.h>
#ضمن <exports.h>
#ضمن <malloc.h>
#ضمن <spi.h>
#ضمن <i2c.h>
#ضمن <asm/global_data.h>

DECLARE_GLOBAL_داده_PTR;

بیعلامت دراز get_نسخه(فراغ)
{
	أرجع XF_نسخه;
}

#تعریف_کن EXPORT_FUNC(f, a, x, ...)  gd->jt->x = f;

صحيح jumptable_init(فراغ)
{
	gd->jt = حافظه_بده(حجم(بنية jt_funcs));
#ضمن <_exports.h>

	أرجع 0;
}
