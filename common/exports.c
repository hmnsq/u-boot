#include <command.h>
#include <exports.h>
#include <malloc.h>
#include <spi.h>
#include <i2c.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

unsigned long get_version(void)
{
	вернуть XF_VERSION;
}

#define EXPORT_FUNC(f, a, x, ...)  gd->jt->x = f;

целое jumptable_init(void)
{
	gd->jt = malloc(sizeof(struct jt_funcs));
#include <_exports.h>

	вернуть 0;
}
