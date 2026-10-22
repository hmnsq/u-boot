#અન્તર્ભાવ <command.h>
#અન્તર્ભાવ <exports.h>
#અન્તર્ભાવ <malloc.h>
#અન્તર્ભાવ <spi.h>
#અન્તર્ભાવ <i2c.h>
#અન્તર્ભાવ <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

unsigned long get_version(શૂન્ય)
{
	પાછા XF_VERSION;
}

#define EXPORT_FUNC(f, a, x, ...)  gd->jt->x = f;

પૂર્ણાંક jumptable_init(શૂન્ય)
{
	gd->jt = malloc(કદ(સંરચના jt_funcs));
#અન્તર્ભાવ <_exports.h>

	પાછા 0;
}
