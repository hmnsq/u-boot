#অন্তর্ভাৱ <command.h>
#অন্তর্ভাৱ <exports.h>
#অন্তর্ভাৱ <malloc.h>
#অন্তর্ভাৱ <spi.h>
#অন্তর্ভাৱ <i2c.h>
#অন্তর্ভাৱ <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

unsigned long get_version(শূন্য)
{
	ঘূৰাই XF_VERSION;
}

#define EXPORT_FUNC(f, a, x, ...)  gd->jt->x = f;

পূর্ণাঙ্ক jumptable_init(শূন্য)
{
	gd->jt = malloc(পরিমাণ(সংরচনা jt_funcs));
#অন্তর্ভাৱ <_exports.h>

	ঘূৰাই 0;
}
