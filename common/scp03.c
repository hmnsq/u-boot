// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2021, Foundries.IO
 *
 */

#అన్తర్భావ <scp03.h>
#అన్తర్భావ <tee.h>
#అన్తర్భావ <tee/optee_ta_scp03.h>
#అన్తర్భావ <linux/errno.h>
#అన్తర్భావ <linux/string.h>

స్థిర పూర్ణాంకం scp03_enable(bool provision)
{
	స్థిరరాశి నిర్మాణం tee_optee_ta_uuid uuid = PTA_SCP03_UUID;
	నిర్మాణం tee_open_session_arg session;
	నిర్మాణం tee_invoke_arg invoke;
	నిర్మాణం tee_param param;
	నిర్మాణం udevice *tee = ఖాళీ;

	tee = tee_కనుగొను_device(tee, ఖాళీ, ఖాళీ, ఖాళీ);
	ఒకవేళ (!tee)
		తిరిగి -ENODEV;

	memset(&session, 0, పరిమాణం(session));
	tee_optee_ta_uuid_గమ్యం_octets(session.uuid, &uuid);
	ఒకవేళ (tee_open_session(tee, &session, 0, ఖాళీ))
		తిరిగి -ENXIO;

	memset(&param, 0, పరిమాణం(param));
	param.attr = TEE_PARAM_ATTR_TYPE_విలువ_ఆదానం;
	param.u.విలువ.a = provision;

	memset(&invoke, 0, పరిమాణం(invoke));
	invoke.func = PTA_CMD_ENABLE_SCP03;
	invoke.session = session.session;

	ఒకవేళ (tee_invoke_func(tee, &invoke, 1, &param))
		తిరిగి -EIO;

	tee_close_session(tee, session.session);

	తిరిగి 0;
}

పూర్ణాంకం tee_enable_scp03(శూన్య)
{
	తిరిగి scp03_enable(false);
}

పూర్ణాంకం tee_provision_scp03(శూన్య)
{
	తిరిగి scp03_enable(true);
}
