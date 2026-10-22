#ಅನ್ತರ್ಭಾವ <command.h>
#ಅನ್ತರ್ಭಾವ <exports.h>
#ಅನ್ತರ್ಭಾವ <malloc.h>
#ಅನ್ತರ್ಭಾವ <spi.h>
#ಅನ್ತರ್ಭಾವ <i2c.h>
#ಅನ್ತರ್ಭಾವ <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

unsigned long get_version(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ XF_VERSION;
}

#define EXPORT_FUNC(f, a, x, ...)  gd->jt->x = f;

ಪೂರ್ಣಾಙ್ಕ jumptable_init(ಶೂನ್ಯ)
{
	gd->jt = malloc(ಪರಿಮಾಣ(ಸಂರಚನಾ jt_funcs));
#ಅನ್ತರ್ಭಾವ <_exports.h>

	ಪ್ರತ್ಯಾಗಮ 0;
}
