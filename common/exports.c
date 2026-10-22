#অন্তর্ভাব <command.h>
#অন্তর্ভাব <exports.h>
#অন্তর্ভাব <malloc.h>
#অন্তর্ভাব <spi.h>
#অন্তর্ভাব <i2c.h>
#অন্তর্ভাব <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

unsigned long get_version(শূন্য)
{
	প্রত্যাগম XF_VERSION;
}

#define EXPORT_FUNC(f, a, x, ...)  gd->jt->x = f;

পূর্ণাঙ্ক jumptable_init(শূন্য)
{
	gd->jt = malloc(পরিমাণ(সংরচনা jt_funcs));
#অন্তর্ভাব <_exports.h>

	প্রত্যাগম 0;
}
