// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2021, Foundries.IO
 *
 */

#ضمن <scp03.h>
#ضمن <tee.h>
#ضمن <tee/optee_ta_scp03.h>
#ضمن <linux/errno.h>
#ضمن <linux/string.h>

ساكن صحيح scp03_enable(bool provision)
{
	ثابت بنية tee_optee_ta_uuid uuid = PTA_SCP03_UUID;
	بنية tee_open_session_arg session;
	بنية tee_invoke_arg invoke;
	بنية tee_param param;
	بنية udevice *tee = عدم;

	tee = tee_find_device(tee, عدم, عدم, عدم);
	إذا (!tee)
		أرجع -ENODEV;

	memset(&session, 0, حجم(session));
	tee_optee_ta_uuid_to_octets(session.uuid, &uuid);
	إذا (tee_open_session(tee, &session, 0, عدم))
		أرجع -ENXIO;

	memset(&param, 0, حجم(param));
	param.attr = TEE_PARAM_ATTR_TYPE_قىممەت_كىرگۈزۈش;
	param.u.قىممەت.a = provision;

	memset(&invoke, 0, حجم(invoke));
	invoke.func = PTA_CMD_ENABLE_SCP03;
	invoke.session = session.session;

	إذا (tee_invoke_func(tee, &invoke, 1, &param))
		أرجع -EIO;

	tee_close_session(tee, session.session);

	أرجع 0;
}

صحيح tee_enable_scp03(فراغ)
{
	أرجع scp03_enable(false);
}

صحيح tee_provision_scp03(فراغ)
{
	أرجع scp03_enable(true);
}
