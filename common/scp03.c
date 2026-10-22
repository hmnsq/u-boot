// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2021, Foundries.IO
 *
 */

#ಅನ್ತರ್ಭಾವ <scp03.h>
#ಅನ್ತರ್ಭಾವ <tee.h>
#ಅನ್ತರ್ಭಾವ <tee/optee_ta_scp03.h>
#ಅನ್ತರ್ಭಾವ <linux/errno.h>
#ಅನ್ತರ್ಭಾವ <linux/string.h>

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ scp03_enable(bool provision)
{
	ಧ್ರುವ ಸಂರಚನಾ tee_optee_ta_uuid uuid = PTA_SCP03_UUID;
	ಸಂರಚನಾ tee_open_session_arg session;
	ಸಂರಚನಾ tee_invoke_arg invoke;
	ಸಂರಚನಾ tee_param param;
	ಸಂರಚನಾ udevice *tee = ರಿಕ್ತ;

	tee = tee_find_device(tee, ರಿಕ್ತ, ರಿಕ್ತ, ರಿಕ್ತ);
	ಯದಿ (!tee)
		ಪ್ರತ್ಯಾಗಮ -ENODEV;

	memset(&session, 0, ಪರಿಮಾಣ(session));
	tee_optee_ta_uuid_to_octets(session.uuid, &uuid);
	ಯದಿ (tee_open_session(tee, &session, 0, ರಿಕ್ತ))
		ಪ್ರತ್ಯಾಗಮ -ENXIO;

	memset(&param, 0, ಪರಿಮಾಣ(param));
	param.attr = TEE_PARAM_ATTR_TYPE_ಮೌಲ್ಯ_ಒಳಹರಿವು;
	param.u.ಮೌಲ್ಯ.a = provision;

	memset(&invoke, 0, ಪರಿಮಾಣ(invoke));
	invoke.func = PTA_CMD_ENABLE_SCP03;
	invoke.session = session.session;

	ಯದಿ (tee_invoke_func(tee, &invoke, 1, &param))
		ಪ್ರತ್ಯಾಗಮ -EIO;

	tee_close_session(tee, session.session);

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಪೂರ್ಣಾಙ್ಕ tee_enable_scp03(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ scp03_enable(false);
}

ಪೂರ್ಣಾಙ್ಕ tee_provision_scp03(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ scp03_enable(true);
}
