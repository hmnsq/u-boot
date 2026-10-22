// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2021, Foundries.IO
 *
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <scp03.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <tee.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <tee/optee_ta_scp03.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/string.h>

ਸਥਿਰ ਪੂਰਨ scp03_enable(bool provision)
{
	ਅਚਲ ਬਣਤਰ tee_optee_ta_uuid uuid = PTA_SCP03_UUID;
	ਬਣਤਰ tee_open_session_arg session;
	ਬਣਤਰ tee_invoke_arg invoke;
	ਬਣਤਰ tee_param param;
	ਬਣਤਰ udevice *tee = ਨਿਰਾ;

	tee = tee_ਲੱਭੋ_device(tee, ਨਿਰਾ, ਨਿਰਾ, ਨਿਰਾ);
	ਜੇ (!tee)
		ਵਾਪਸ -ENODEV;

	memset(&session, 0, ਆਕਾਰ(session));
	tee_optee_ta_uuid_ਮੰਜ਼ਿਲ_octets(session.uuid, &uuid);
	ਜੇ (tee_open_session(tee, &session, 0, ਨਿਰਾ))
		ਵਾਪਸ -ENXIO;

	memset(&param, 0, ਆਕਾਰ(param));
	param.attr = TEE_PARAM_ATTR_TYPE_ਮੁੱਲ_ਨਿਵੇਸ਼;
	param.u.ਮੁੱਲ.a = provision;

	memset(&invoke, 0, ਆਕਾਰ(invoke));
	invoke.func = PTA_CMD_ENABLE_SCP03;
	invoke.session = session.session;

	ਜੇ (tee_invoke_func(tee, &invoke, 1, &param))
		ਵਾਪਸ -EIO;

	tee_close_session(tee, session.session);

	ਵਾਪਸ 0;
}

ਪੂਰਨ tee_enable_scp03(ਖਾਲੀ)
{
	ਵਾਪਸ scp03_enable(false);
}

ਪੂਰਨ tee_provision_scp03(ਖਾਲੀ)
{
	ਵਾਪਸ scp03_enable(true);
}
