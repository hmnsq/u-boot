#ਅਨ੍ਤਰ੍ਭਾਵ <command.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <exports.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <malloc.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <spi.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <i2c.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/global_data.h>

DECLARE_GLOBAL_ਅੰਕੜੇ_PTR;

ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ get_ਸੰਸਕਰਣ(ਖਾਲੀ)
{
	ਵਾਪਸ XF_ਸੰਸਕਰਣ;
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EXPORT_FUNC(f, a, x, ...)  gd->jt->x = f;

ਪੂਰਨ jumptable_init(ਖਾਲੀ)
{
	gd->jt = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਆਕਾਰ(ਬਣਤਰ jt_funcs));
#ਅਨ੍ਤਰ੍ਭਾਵ <_exports.h>

	ਵਾਪਸ 0;
}
