#شامل <command.h>
#شامل <exports.h>
#شامل <malloc.h>
#شامل <spi.h>
#شامل <i2c.h>
#شامل <asm/global_data.h>

DECLARE_GLOBAL_ڈیٹا_PTR;

غیرعلامتی طویل get_نسخہ(خالی)
{
	واپس XF_نسخہ;
}

#تعریف_کرو EXPORT_FUNC(f, a, x, ...)  gd->jt->x = f;

صحیح jumptable_init(خالی)
{
	gd->jt = حافظہ_دو(حجم(ساخت jt_funcs));
#شامل <_exports.h>

	واپس 0;
}
