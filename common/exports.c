#अन्तर्भाव <command.h>
#अन्तर्भाव <exports.h>
#अन्तर्भाव <malloc.h>
#अन्तर्भाव <spi.h>
#अन्तर्भाव <i2c.h>
#अन्तर्भाव <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

अचिन्हित दीर्घ get_version(शून्य)
{
	फर्काऊ XF_VERSION;
}

#define EXPORT_FUNC(f, a, x, ...)  gd->jt->x = f;

पूर्णाङ्क jumptable_init(शून्य)
{
	gd->jt = दो_स्मृति(परिमाण(संरचना jt_funcs));
#अन्तर्भाव <_exports.h>

	फर्काऊ 0;
}
