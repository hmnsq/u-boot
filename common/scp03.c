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

स्थिर पूर्णांक scp03_enable(bool provision)
{
	स्थिरांक संरचना tee_optee_ta_uuid uuid = PTA_SCP03_UUID;
	संरचना tee_open_session_arg session;
	संरचना tee_invoke_arg invoke;
	संरचना tee_param param;
	संरचना udevice *tee = रिक्त;

	tee = tee_find_device(tee, रिक्त, रिक्त, रिक्त);
	जर (!tee)
		परत -ENODEV;

	स_रखो(&session, 0, आकार(session));
	tee_optee_ta_uuid_to_octets(session.uuid, &uuid);
	जर (tee_open_session(tee, &session, 0, रिक्त))
		परत -ENXIO;

	स_रखो(&param, 0, आकार(param));
	param.attr = TEE_PARAM_ATTR_TYPE_मूल्य_आदान;
	param.u.मूल्य.a = provision;

	स_रखो(&invoke, 0, आकार(invoke));
	invoke.func = PTA_CMD_ENABLE_SCP03;
	invoke.session = session.session;

	जर (tee_invoke_func(tee, &invoke, 1, &param))
		परत -EIO;

	tee_close_session(tee, session.session);

	परत 0;
}

पूर्णांक tee_enable_scp03(शून्य)
{
	परत scp03_enable(false);
}

पूर्णांक tee_provision_scp03(शून्य)
{
	परत scp03_enable(true);
}
