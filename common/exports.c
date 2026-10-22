#অন্তর্ভাব <command.h>
#অন্তর্ভাব <exports.h>
#অন্তর্ভাব <malloc.h>
#অন্তর্ভাব <spi.h>
#অন্তর্ভাব <i2c.h>
#অন্তর্ভাব <asm/global_data.h>

DECLARE_GLOBAL_তথ্য_PTR;

unsigned long get_version(শূন্য)
{
	ফেরত XF_VERSION;
}

#define EXPORT_FUNC(f, a, x, ...)  gd->jt->x = f;

পূর্ণসংখ্যা jumptable_init(শূন্য)
{
	gd->jt = malloc(আকার(কাঠামো jt_funcs));
#অন্তর্ভাব <_exports.h>

	ফেরত 0;
}
