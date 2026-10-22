#شامل <command.h>
#شامل <exports.h>
#شامل <malloc.h>
#شامل <spi.h>
#شامل <i2c.h>
#شامل <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

unsigned long get_version(تهی)
{
	بازگشت XF_VERSION;
}

#define EXPORT_FUNC(f, a, x, ...)  gd->jt->x = f;

صحیح jumptable_init(تهی)
{
	gd->jt = malloc(اندازه(ساختار jt_funcs));
#شامل <_exports.h>

	بازگشت 0;
}
