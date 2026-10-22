// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2021, Foundries.IO
 *
 */

#અન્તર્ભાવ <scp03.h>
#અન્તર્ભાવ <tee.h>
#અન્તર્ભાવ <tee/optee_ta_scp03.h>
#અન્તર્ભાવ <linux/errno.h>
#અન્તર્ભાવ <linux/string.h>

સ્થિર પૂર્ણાંક scp03_enable(bool provision)
{
	અચલ સંરચના tee_optee_ta_uuid uuid = PTA_SCP03_UUID;
	સંરચના tee_open_session_arg session;
	સંરચના tee_invoke_arg invoke;
	સંરચના tee_param param;
	સંરચના udevice *tee = ખાલી;

	tee = tee_find_device(tee, ખાલી, ખાલી, ખાલી);
	જો (!tee)
		પાછા -ENODEV;

	memset(&session, 0, કદ(session));
	tee_optee_ta_uuid_to_octets(session.uuid, &uuid);
	જો (tee_open_session(tee, &session, 0, ખાલી))
		પાછા -ENXIO;

	memset(&param, 0, કદ(param));
	param.attr = TEE_PARAM_ATTR_TYPE_કિંમત_આગત;
	param.u.કિંમત.a = provision;

	memset(&invoke, 0, કદ(invoke));
	invoke.func = PTA_CMD_ENABLE_SCP03;
	invoke.session = session.session;

	જો (tee_invoke_func(tee, &invoke, 1, &param))
		પાછા -EIO;

	tee_close_session(tee, session.session);

	પાછા 0;
}

પૂર્ણાંક tee_enable_scp03(શૂન્ય)
{
	પાછા scp03_enable(false);
}

પૂર્ણાંક tee_provision_scp03(શૂન્ય)
{
	પાછા scp03_enable(true);
}
