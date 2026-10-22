#ଅନ୍ତର୍ଭାଵ <command.h>
#ଅନ୍ତର୍ଭାଵ <exports.h>
#ଅନ୍ତର୍ଭାଵ <malloc.h>
#ଅନ୍ତର୍ଭାଵ <spi.h>
#ଅନ୍ତର୍ଭାଵ <i2c.h>
#ଅନ୍ତର୍ଭାଵ <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

unsigned long get_version(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ XF_VERSION;
}

#define EXPORT_FUNC(f, a, x, ...)  gd->jt->x = f;

ପୂର୍ଣ୍ଣସଂଖ୍ୟା jumptable_init(ଶୂନ୍ୟ)
{
	gd->jt = malloc(ଆକାର(ସଂରଚନା jt_funcs));
#ଅନ୍ତର୍ଭାଵ <_exports.h>

	ଫେରନ୍ତୁ 0;
}
