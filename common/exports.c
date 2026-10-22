#include <command.h>
#include <exports.h>
#include <malloc.h>
#include <spi.h>
#include <i2c.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

unsigned long get_version(void)
{
	trả_về XF_VERSION;
}

#define EXPORT_FUNC(f, a, x, ...)  gd->jt->x = f;

số_nguyên jumptable_init(void)
{
	gd->jt = malloc(sizeof(struct jt_funcs));
#include <_exports.h>

	trả_về 0;
}
