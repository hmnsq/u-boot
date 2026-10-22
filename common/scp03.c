// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2021, Foundries.IO
 *
 */

#அந்தர்பாவ <scp03.h>
#அந்தர்பாவ <tee.h>
#அந்தர்பாவ <tee/optee_ta_scp03.h>
#அந்தர்பாவ <linux/errno.h>
#அந்தர்பாவ <linux/string.h>

நிலையான முழு scp03_enable(bool provision)
{
	மாறிலி கட்டமைப்பு tee_optee_ta_uuid uuid = PTA_SCP03_UUID;
	கட்டமைப்பு tee_open_session_arg session;
	கட்டமைப்பு tee_invoke_arg invoke;
	கட்டமைப்பு tee_param param;
	கட்டமைப்பு udevice *tee = வெற்று;

	tee = tee_find_device(tee, வெற்று, வெற்று, வெற்று);
	எனில் (!tee)
		திரும்பு -ENODEV;

	memset(&session, 0, அளவு(session));
	tee_optee_ta_uuid_to_octets(session.uuid, &uuid);
	எனில் (tee_open_session(tee, &session, 0, வெற்று))
		திரும்பு -ENXIO;

	memset(&param, 0, அளவு(param));
	param.attr = TEE_PARAM_ATTR_TYPE_மதிப்பு_உள்ளீடு;
	param.u.மதிப்பு.a = provision;

	memset(&invoke, 0, அளவு(invoke));
	invoke.func = PTA_CMD_ENABLE_SCP03;
	invoke.session = session.session;

	எனில் (tee_invoke_func(tee, &invoke, 1, &param))
		திரும்பு -EIO;

	tee_close_session(tee, session.session);

	திரும்பு 0;
}

முழு tee_enable_scp03(வெறுமை)
{
	திரும்பு scp03_enable(false);
}

முழு tee_provision_scp03(வெறுமை)
{
	திரும்பு scp03_enable(true);
}
