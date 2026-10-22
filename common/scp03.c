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

ایستا صحیح scp03_enable(bool provision)
{
	ثابت ساختار tee_optee_ta_uuid uuid = PTA_SCP03_UUID;
	ساختار tee_open_session_arg session;
	ساختار tee_invoke_arg invoke;
	ساختار tee_param param;
	ساختار udevice *tee = پوچ;

	tee = tee_find_device(tee, پوچ, پوچ, پوچ);
	اگر (!tee)
		بازگشت -ENODEV;

	memset(&session, 0, اندازه(session));
	tee_optee_ta_uuid_to_octets(session.uuid, &uuid);
	اگر (tee_open_session(tee, &session, 0, پوچ))
		بازگشت -ENXIO;

	memset(&param, 0, اندازه(param));
	param.attr = TEE_PARAM_ATTR_TYPE_مقدار_درون_داد;
	param.u.مقدار.a = provision;

	memset(&invoke, 0, اندازه(invoke));
	invoke.func = PTA_CMD_ENABLE_SCP03;
	invoke.session = session.session;

	اگر (tee_invoke_func(tee, &invoke, 1, &param))
		بازگشت -EIO;

	tee_close_session(tee, session.session);

	بازگشت 0;
}

صحیح tee_enable_scp03(تهی)
{
	بازگشت scp03_enable(false);
}

صحیح tee_provision_scp03(تهی)
{
	بازگشت scp03_enable(true);
}
