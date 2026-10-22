// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2021, Foundries.IO
 *
 */

#समावेश <scp03.h>
#समावेश <tee.h>
#समावेश <tee/optee_ta_scp03.h>
#समावेश <linux/errno.h>
#समावेश <linux/string.h>

अटल पूर्णांक scp03_enable(bool provision)
{
	स्थिर काष्ठा tee_optee_ta_uuid uuid = PTA_SCP03_UUID;
	काष्ठा tee_open_session_arg session;
	काष्ठा tee_invoke_arg invoke;
	काष्ठा tee_param param;
	काष्ठा udevice *tee = शून्य;

	tee = tee_find_device(tee, शून्य, शून्य, शून्य);
	अगर (!tee)
		वापस -ENODEV;

	स_रखो(&session, 0, माप(session));
	tee_optee_ta_uuid_to_octets(session.uuid, &uuid);
	अगर (tee_open_session(tee, &session, 0, शून्य))
		वापस -ENXIO;

	स_रखो(&param, 0, माप(param));
	param.attr = TEE_PARAM_ATTR_TYPE_मूल्य_निवेश;
	param.u.मूल्य.a = provision;

	स_रखो(&invoke, 0, माप(invoke));
	invoke.func = PTA_CMD_ENABLE_SCP03;
	invoke.session = session.session;

	अगर (tee_invoke_func(tee, &invoke, 1, &param))
		वापस -EIO;

	tee_close_session(tee, session.session);

	वापस 0;
}

पूर्णांक tee_enable_scp03(व्योम)
{
	वापस scp03_enable(false);
}

पूर्णांक tee_provision_scp03(व्योम)
{
	वापस scp03_enable(true);
}
