#அந்தர்பாவ <command.h>
#அந்தர்பாவ <exports.h>
#அந்தர்பாவ <malloc.h>
#அந்தர்பாவ <spi.h>
#அந்தர்பாவ <i2c.h>
#அந்தர்பாவ <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

unsigned long get_version(வெறுமை)
{
	திரும்பு XF_VERSION;
}

#define EXPORT_FUNC(f, a, x, ...)  gd->jt->x = f;

முழு jumptable_init(வெறுமை)
{
	gd->jt = malloc(அளவு(கட்டமைப்பு jt_funcs));
#அந்தர்பாவ <_exports.h>

	திரும்பு 0;
}
