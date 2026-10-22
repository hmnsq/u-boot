// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2021, Foundries.IO
 *
 */

#অন্তর্ভাব <scp03.h>
#অন্তর্ভাব <tee.h>
#অন্তর্ভাব <tee/optee_ta_scp03.h>
#অন্তর্ভাব <linux/errno.h>
#অন্তর্ভাব <linux/string.h>

স্থির পূর্ণসংখ্যা scp03_enable(bool provision)
{
	ধ্রুবক কাঠামো tee_optee_ta_uuid uuid = PTA_SCP03_UUID;
	কাঠামো tee_open_session_arg session;
	কাঠামো tee_invoke_arg invoke;
	কাঠামো tee_param param;
	কাঠামো udevice *tee = ফাঁকা;

	tee = tee_find_device(tee, ফাঁকা, ফাঁকা, ফাঁকা);
	যদি (!tee)
		ফেরত -ENODEV;

	memset(&session, 0, আকার(session));
	tee_optee_ta_uuid_to_octets(session.uuid, &uuid);
	যদি (tee_open_session(tee, &session, 0, ফাঁকা))
		ফেরত -ENXIO;

	memset(&param, 0, আকার(param));
	param.attr = TEE_PARAM_ATTR_TYPE_মান_প্রবেশ;
	param.u.মান.a = provision;

	memset(&invoke, 0, আকার(invoke));
	invoke.func = PTA_CMD_ENABLE_SCP03;
	invoke.session = session.session;

	যদি (tee_invoke_func(tee, &invoke, 1, &param))
		ফেরত -EIO;

	tee_close_session(tee, session.session);

	ফেরত 0;
}

পূর্ণসংখ্যা tee_enable_scp03(শূন্য)
{
	ফেরত scp03_enable(false);
}

পূর্ণসংখ্যা tee_provision_scp03(শূন্য)
{
	ফেরত scp03_enable(true);
}
