// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2021, Foundries.IO
 *
 */

#अन्तर्भाव <scp03.h>
#अन्तर्भाव <tee.h>
#अन्तर्भाव <tee/optee_ta_scp03.h>
#अन्तर्भाव <linux/errno.h>
#अन्तर्भाव <linux/string.h>

स्थिर पूर्णाङ्क scp03_enable(bool provision)
{
	अचर संरचना tee_optee_ta_uuid uuid = PTA_SCP03_UUID;
	संरचना tee_open_session_arg session;
	संरचना tee_invoke_arg invoke;
	संरचना tee_param param;
	संरचना udevice *tee = रिक्त;

	tee = tee_find_device(tee, रिक्त, रिक्त, रिक्त);
	यदि (!tee)
		फर्काऊ -ENODEV;

	स_रखो(&session, 0, परिमाण(session));
	tee_optee_ta_uuid_to_octets(session.uuid, &uuid);
	यदि (tee_open_session(tee, &session, 0, रिक्त))
		फर्काऊ -ENXIO;

	स_रखो(&param, 0, परिमाण(param));
	param.attr = TEE_PARAM_ATTR_TYPE_मान_आगत;
	param.u.मान.a = provision;

	स_रखो(&invoke, 0, परिमाण(invoke));
	invoke.func = PTA_CMD_ENABLE_SCP03;
	invoke.session = session.session;

	यदि (tee_invoke_func(tee, &invoke, 1, &param))
		फर्काऊ -EIO;

	tee_close_session(tee, session.session);

	फर्काऊ 0;
}

पूर्णाङ्क tee_enable_scp03(शून्य)
{
	फर्काऊ scp03_enable(false);
}

पूर्णाङ्क tee_provision_scp03(शून्य)
{
	फर्काऊ scp03_enable(true);
}
