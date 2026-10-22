// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2021, Foundries.IO
 *
 */

#അന്തര്ഭാവ <scp03.h>
#അന്തര്ഭാവ <tee.h>
#അന്തര്ഭാവ <tee/optee_ta_scp03.h>
#അന്തര്ഭാവ <linux/errno.h>
#അന്തര്ഭാവ <linux/string.h>

സ്ഥിതം പൂർണ്ണസംഖ്യ scp03_enable(bool provision)
{
	സ്ഥിരം ഘടന tee_optee_ta_uuid uuid = PTA_SCP03_UUID;
	ഘടന tee_open_session_arg session;
	ഘടന tee_invoke_arg invoke;
	ഘടന tee_param param;
	ഘടന udevice *tee = ശൂന്യ;

	tee = tee_find_device(tee, ശൂന്യ, ശൂന്യ, ശൂന്യ);
	എങ്കിൽ (!tee)
		തിരികെ -ENODEV;

	memset(&session, 0, വലുപ്പം(session));
	tee_optee_ta_uuid_to_octets(session.uuid, &uuid);
	എങ്കിൽ (tee_open_session(tee, &session, 0, ശൂന്യ))
		തിരികെ -ENXIO;

	memset(&param, 0, വലുപ്പം(param));
	param.attr = TEE_PARAM_ATTR_TYPE_മൂല്യം_നിവേശം;
	param.u.മൂല്യം.a = provision;

	memset(&invoke, 0, വലുപ്പം(invoke));
	invoke.func = PTA_CMD_ENABLE_SCP03;
	invoke.session = session.session;

	എങ്കിൽ (tee_invoke_func(tee, &invoke, 1, &param))
		തിരികെ -EIO;

	tee_close_session(tee, session.session);

	തിരികെ 0;
}

പൂർണ്ണസംഖ്യ tee_enable_scp03(ശൂന്യം)
{
	തിരികെ scp03_enable(false);
}

പൂർണ്ണസംഖ്യ tee_provision_scp03(ശൂന്യം)
{
	തിരികെ scp03_enable(true);
}
