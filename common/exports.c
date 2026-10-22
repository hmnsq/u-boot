#समावेश <command.h>
#समावेश <exports.h>
#समावेश <malloc.h>
#समावेश <spi.h>
#समावेश <i2c.h>
#समावेश <asm/global_data.h>

DECLARE_GLOBAL_तथ्य_PTR;

अचिन्हित दीर्घ get_version(व्योम)
{
	वापस XF_VERSION;
}

#घोषणा EXPORT_FUNC(f, a, x, ...)  gd->jt->x = f;

पूर्णांक jumptable_init(व्योम)
{
	gd->jt = दो_स्मृति(माप(काष्ठा jt_funcs));
#समावेश <_exports.h>

	वापस 0;
}
