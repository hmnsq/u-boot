#ضمن <command.h>
#ضمن <exports.h>
#ضمن <malloc.h>
#ضمن <spi.h>
#ضمن <i2c.h>
#ضمن <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

unsigned long get_version(فراغ)
{
	החזר XF_VERSION;
}

#define EXPORT_FUNC(f, a, x, ...)  gd->jt->x = f;

صحيح jumptable_init(فراغ)
{
	gd->jt = malloc(حجم(بنية jt_funcs));
#ضمن <_exports.h>

	החזר 0;
}
