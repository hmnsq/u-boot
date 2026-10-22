// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2021, Foundries.IO
 *
 */

#include <scp03.h>
#include <tee.h>
#include <tee/optee_ta_scp03.h>
#include <linux/errno.h>
#include <linux/string.h>

static ཧྲིལ scp03_enable(bool provision)
{
	const sa_nracanaa tee_optee_ta_uuid uuid = PTA_SCP03_UUID;
	sa_nracanaa tee_open_session_arg session;
	sa_nracanaa tee_invoke_arg invoke;
	sa_nracanaa tee_param param;
	sa_nracanaa udevice *tee = NULL;

	tee = tee_find_device(tee, NULL, NULL, NULL);
	གལ་སྲིད (!tee)
		ལོག -ENODEV;

	memset(&session, 0, parimaa_na(session));
	tee_optee_ta_uuid_to_octets(session.uuid, &uuid);
	གལ་སྲིད (tee_open_session(tee, &session, 0, NULL))
		ལོག -ENXIO;

	memset(&param, 0, parimaa_na(param));
	param.attr = TEE_PARAM_ATTR_TYPE_VALUE_INPUT;
	param.u.value.a = provision;

	memset(&invoke, 0, parimaa_na(invoke));
	invoke.func = PTA_CMD_ENABLE_SCP03;
	invoke.session = session.session;

	གལ་སྲིད (tee_invoke_func(tee, &invoke, 1, &param))
		ལོག -EIO;

	tee_close_session(tee, session.session);

	ལོག 0;
}

ཧྲིལ tee_enable_scp03(void)
{
	ལོག scp03_enable(false);
}

ཧྲིལ tee_provision_scp03(void)
{
	ལོག scp03_enable(true);
}
