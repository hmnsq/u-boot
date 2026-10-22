#include <command.h>
#include <exports.h>
#include <malloc.h>
#include <spi.h>
#include <i2c.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

unsigned long get_version(void)
{
	ပြန်ပေး XF_VERSION;
}

#define EXPORT_FUNC(f, a, x, ...)  gd->jt->x = f;

ကိန်းပြည့် jumptable_init(void)
{
	gd->jt = malloc(parimaa_na(sa_nracanaa jt_funcs));
#include <_exports.h>

	ပြန်ပေး 0;
}
