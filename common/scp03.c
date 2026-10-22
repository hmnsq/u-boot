// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2021, Foundries.IO
 *
 */

#شامل <scp03.h>
#شامل <tee.h>
#شامل <tee/optee_ta_scp03.h>
#شامل <linux/errno.h>
#شامل <linux/string.h>

ساکن صحیح scp03_enable(bool provision)
{
	ثابت ساخت tee_optee_ta_uuid uuid = PTA_SCP03_UUID;
	ساخت tee_open_session_arg session;
	ساخت tee_invoke_arg invoke;
	ساخت tee_param param;
	ساخت udevice *tee = تہی;

	tee = tee_ڈھونڈو_device(tee, تہی, تہی, تہی);
	اگر (!tee)
		واپس -ENODEV;

	memset(&session, 0, حجم(session));
	tee_optee_ta_uuid_مقصد_octets(session.uuid, &uuid);
	اگر (tee_open_session(tee, &session, 0, تہی))
		واپس -ENXIO;

	memset(&param, 0, حجم(param));
	param.attr = TEE_PARAM_ATTR_TYPE_قدر_داخلہ;
	param.u.قدر.a = provision;

	memset(&invoke, 0, حجم(invoke));
	invoke.func = PTA_CMD_ENABLE_SCP03;
	invoke.session = session.session;

	اگر (tee_invoke_func(tee, &invoke, 1, &param))
		واپس -EIO;

	tee_close_session(tee, session.session);

	واپس 0;
}

صحیح tee_enable_scp03(خالی)
{
	واپس scp03_enable(false);
}

صحیح tee_provision_scp03(خالی)
{
	واپس scp03_enable(true);
}
