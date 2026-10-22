#अन्तर्भाव <command.h>
#अन्तर्भाव <exports.h>
#अन्तर्भाव <malloc.h>
#अन्तर्भाव <spi.h>
#अन्तर्भाव <i2c.h>
#अन्तर्भाव <asm/global_data.h>

DECLARE_GLOBAL_दत्त_PTR;

अचिह्नित दीर्घ get_संस्करणम्(शून्य)
{
	प्रत्यागम XF_संस्करणम्;
}

#परिभाषय EXPORT_FUNC(f, a, x, ...)  gd->jt->x = f;

पूर्णाङ्क jumptable_init(शून्य)
{
	gd->jt = स्मृतिं_देहि(परिमाण(संरचना jt_funcs));
#अन्तर्भाव <_exports.h>

	प्रत्यागम 0;
}
