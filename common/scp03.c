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

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ scp03_enable(bool provision)
{
	ಸ್ಥಿರಾಂಕ ರಚನೆ tee_optee_ta_uuid uuid = PTA_SCP03_UUID;
	ರಚನೆ tee_open_session_arg session;
	ರಚನೆ tee_invoke_arg invoke;
	ರಚನೆ tee_param param;
	ರಚನೆ udevice *tee = ಖಾಲಿ;

	tee = tee_ಕಂಡುಹಿಡಿ_device(tee, ಖಾಲಿ, ಖಾಲಿ, ಖಾಲಿ);
	ಆದರೆ (!tee)
		ಹಿಂತಿರುಗಿ -ENODEV;

	memset(&session, 0, ಗಾತ್ರ(session));
	tee_optee_ta_uuid_ಗಮ್ಯ_octets(session.uuid, &uuid);
	ಆದರೆ (tee_open_session(tee, &session, 0, ಖಾಲಿ))
		ಹಿಂತಿರುಗಿ -ENXIO;

	memset(&param, 0, ಗಾತ್ರ(param));
	param.attr = TEE_PARAM_ATTR_TYPE_ಮೌಲ್ಯ_ಒಳಹರಿವು;
	param.u.ಮೌಲ್ಯ.a = provision;

	memset(&invoke, 0, ಗಾತ್ರ(invoke));
	invoke.func = PTA_CMD_ENABLE_SCP03;
	invoke.session = session.session;

	ಆದರೆ (tee_invoke_func(tee, &invoke, 1, &param))
		ಹಿಂತಿರುಗಿ -EIO;

	tee_close_session(tee, session.session);

	ಹಿಂತಿರುಗಿ 0;
}

ಪೂರ್ಣಾಂಕ tee_enable_scp03(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ scp03_enable(false);
}

ಪೂರ್ಣಾಂಕ tee_provision_scp03(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ scp03_enable(true);
}
