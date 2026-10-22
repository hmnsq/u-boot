#അന്തര്ഭാവ <command.h>
#അന്തര്ഭാവ <exports.h>
#അന്തര്ഭാവ <malloc.h>
#അന്തര്ഭാവ <spi.h>
#അന്തര്ഭാവ <i2c.h>
#അന്തര്ഭാവ <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

unsigned long get_version(ശൂന്യം)
{
	തിരികെ XF_VERSION;
}

#define EXPORT_FUNC(f, a, x, ...)  gd->jt->x = f;

പൂർണ്ണസംഖ്യ jumptable_init(ശൂന്യം)
{
	gd->jt = malloc(വലുപ്പം(ഘടന jt_funcs));
#അന്തര്ഭാവ <_exports.h>

	തിരികെ 0;
}
