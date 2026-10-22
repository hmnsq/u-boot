// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2021, Foundries.IO
 *
 */

#অন্তর্ভাৱ <scp03.h>
#অন্তর্ভাৱ <tee.h>
#অন্তর্ভাৱ <tee/optee_ta_scp03.h>
#অন্তর্ভাৱ <linux/errno.h>
#অন্তর্ভাৱ <linux/string.h>

স্থির পূর্ণাঙ্ক scp03_enable(bool provision)
{
	ধ্রুৱ সংরচনা tee_optee_ta_uuid uuid = PTA_SCP03_UUID;
	সংরচনা tee_open_session_arg session;
	সংরচনা tee_invoke_arg invoke;
	সংরচনা tee_param param;
	সংরচনা udevice *tee = রিক্ত;

	tee = tee_find_device(tee, রিক্ত, রিক্ত, রিক্ত);
	যদি (!tee)
		ঘূৰাই -ENODEV;

	memset(&session, 0, পরিমাণ(session));
	tee_optee_ta_uuid_to_octets(session.uuid, &uuid);
	যদি (tee_open_session(tee, &session, 0, রিক্ত))
		ঘূৰাই -ENXIO;

	memset(&param, 0, পরিমাণ(param));
	param.attr = TEE_PARAM_ATTR_TYPE_মান_প্ৰবেশ;
	param.u.মান.a = provision;

	memset(&invoke, 0, পরিমাণ(invoke));
	invoke.func = PTA_CMD_ENABLE_SCP03;
	invoke.session = session.session;

	যদি (tee_invoke_func(tee, &invoke, 1, &param))
		ঘূৰাই -EIO;

	tee_close_session(tee, session.session);

	ঘূৰাই 0;
}

পূর্ণাঙ্ক tee_enable_scp03(শূন্য)
{
	ঘূৰাই scp03_enable(false);
}

পূর্ণাঙ্ক tee_provision_scp03(শূন্য)
{
	ঘূৰাই scp03_enable(true);
}
