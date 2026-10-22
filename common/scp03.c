// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2021, Foundries.IO
 *
 */

#ଅନ୍ତର୍ଭାଵ <scp03.h>
#ଅନ୍ତର୍ଭାଵ <tee.h>
#ଅନ୍ତର୍ଭାଵ <tee/optee_ta_scp03.h>
#ଅନ୍ତର୍ଭାଵ <linux/errno.h>
#ଅନ୍ତର୍ଭାଵ <linux/string.h>

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା scp03_enable(bool provision)
{
	ଅଚଳ ସଂରଚନା tee_optee_ta_uuid uuid = PTA_SCP03_UUID;
	ସଂରଚନା tee_open_session_arg session;
	ସଂରଚନା tee_invoke_arg invoke;
	ସଂରଚନା tee_param param;
	ସଂରଚନା udevice *tee = ଖାଲି;

	tee = tee_find_device(tee, ଖାଲି, ଖାଲି, ଖାଲି);
	ଯଦି (!tee)
		ଫେରନ୍ତୁ -ENODEV;

	memset(&session, 0, ଆକାର(session));
	tee_optee_ta_uuid_to_octets(session.uuid, &uuid);
	ଯଦି (tee_open_session(tee, &session, 0, ଖାଲି))
		ଫେରନ୍ତୁ -ENXIO;

	memset(&param, 0, ଆକାର(param));
	param.attr = TEE_PARAM_ATTR_TYPE_ମୂଲ୍ୟ_ନିବେଶ;
	param.u.ମୂଲ୍ୟ.a = provision;

	memset(&invoke, 0, ଆକାର(invoke));
	invoke.func = PTA_CMD_ENABLE_SCP03;
	invoke.session = session.session;

	ଯଦି (tee_invoke_func(tee, &invoke, 1, &param))
		ଫେରନ୍ତୁ -EIO;

	tee_close_session(tee, session.session);

	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା tee_enable_scp03(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ scp03_enable(false);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା tee_provision_scp03(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ scp03_enable(true);
}
