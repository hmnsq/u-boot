#ಅನ್ತರ್ಭಾವ <command.h>
#ಅನ್ತರ್ಭಾವ <exports.h>
#ಅನ್ತರ್ಭಾವ <malloc.h>
#ಅನ್ತರ್ಭಾವ <spi.h>
#ಅನ್ತರ್ಭಾವ <i2c.h>
#ಅನ್ತರ್ಭಾವ <asm/global_data.h>

DECLARE_GLOBAL_ದತ್ತಾಂಶ_PTR;

ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ get_ಆವೃತ್ತಿ(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ XF_ಆವೃತ್ತಿ;
}

#ವ್ಯಾಖ್ಯಾನಿಸು EXPORT_FUNC(f, a, x, ...)  gd->jt->x = f;

ಪೂರ್ಣಾಂಕ jumptable_init(ಶೂನ್ಯ)
{
	gd->jt = ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ(ರಚನೆ jt_funcs));
#ಅನ್ತರ್ಭಾವ <_exports.h>

	ಹಿಂತಿರುಗಿ 0;
}
