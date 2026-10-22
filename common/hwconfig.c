// SPDX-License-Identifier: GPL-2.0+
/*
 * An inteface for configuring a hardware via u-boot environment.
 *
 * Copyright (c) 2009  MontaVista Software, Inc.
 * Copyright 2011 Freescale Semiconductor, Inc.
 *
 * Author: Anton Vorontsov <avorontsov@ru.mvista.com>
 */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ HWCONFIG_TEST
#ಅನ್ತರ್ಭಾವ <config.h>
#ಅನ್ತರ್ಭಾವ <env.h>
#ಅನ್ತರ್ಭಾವ <exports.h>
#ಅನ್ತರ್ಭಾವ <hwconfig.h>
#ಅನ್ತರ್ಭಾವ <log.h>
#ಅನ್ತರ್ಭಾವ <asm/global_data.h>
#ಅನ್ತರ್ಭಾವ <linux/types.h>
#ಅನ್ತರ್ಭಾವ <linux/string.h>
#ಇಲ್ಲದಿದ್ದರೆ
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <assert.h>
#ವ್ಯಾಖ್ಯಾನಿಸು min(a, b) (((a) < (b)) ? (a) : (b))
#ಆದರೆ_ಅಂತ್ಯ /* HWCONFIG_TEST */

DECLARE_GLOBAL_ದತ್ತಾಂಶ_PTR;

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *hwconfig_parse(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *opts, size_t maxlen,
				  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *opt, ಅಕ್ಷರ *stopchs, ಅಕ್ಷರ eqch,
				  size_t *arglen)
{
	size_t optlen = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(opt);
	ಅಕ್ಷರ *str;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಆರಂಭ = opts;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಕೊನೆ_2;

ಮುಂದಿನ:
	str = ಅಕ್ಷರಸರಣಿ_ಹುಡುಕು(opts, opt);
	ಕೊನೆ_2 = str + optlen;
	ಆದರೆ (ಕೊನೆ_2 - ಆರಂಭ > maxlen)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	ಆದರೆ (str && (str == opts || strpbrk(str - 1, stopchs) == str - 1) &&
			(strpbrk(ಕೊನೆ_2, stopchs) == ಕೊನೆ_2 || *ಕೊನೆ_2 == eqch ||
			 *ಕೊನೆ_2 == '\0')) {
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *arg_ಕೊನೆ;

		ಆದರೆ (!arglen)
			ಹಿಂತಿರುಗಿ str;

		ಆದರೆ (*ಕೊನೆ_2 != eqch)
			ಹಿಂತಿರುಗಿ ಖಾಲಿ;

		arg_ಕೊನೆ = strpbrk(str, stopchs);
		ಆದರೆ (!arg_ಕೊನೆ)
			*arglen = min(maxlen, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(str)) - optlen - 1;
		ಇಲ್ಲದಿದ್ದರೆ
			*arglen = arg_ಕೊನೆ - ಕೊನೆ_2 - 1;

		ಹಿಂತಿರುಗಿ ಕೊನೆ_2 + 1;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (str) {
		opts = ಕೊನೆ_2;
		ಹೋಗು ಮುಂದಿನ;
	}
	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ cpu_hwconfig[] __attribute__((weak)) = "";
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ board_hwconfig[] __attribute__((weak)) = "";

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *__hwconfig(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *opt, size_t *arglen,
			      ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *env_hwconfig)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ret;

	/* if we are passed a buffer use it, otherwise try the environment */
	ಆದರೆ (!env_hwconfig) {
#ಆದರೆ CONFIG_IS_ENABLED(ENV_SUPPORT)
		ಆದರೆ (!(gd->flags & GD_FLG_ENV_READY) &&
		    gd->env_valid != ENV_VALID)
#ಇಲ್ಲದಿದ್ದರೆ
		ಆದರೆ (true)
#ಆದರೆ_ಅಂತ್ಯ
		{
			ಬರೆ("WARNING: Calling __hwconfig without a buffer "
					"and before environment is ready\n");
			ಹಿಂತಿರುಗಿ ಖಾಲಿ;
		}
#ಆದರೆ CONFIG_IS_ENABLED(ENV_SUPPORT)
		env_hwconfig = env_get("hwconfig");
#ಆದರೆ_ಅಂತ್ಯ
	}

	ಆದರೆ (env_hwconfig) {
		ret = hwconfig_parse(env_hwconfig, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(env_hwconfig),
				      opt, ";", ':', arglen);
		ಆದರೆ (ret)
			ಹಿಂತಿರುಗಿ ret;
	}

	ret = hwconfig_parse(board_hwconfig, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(board_hwconfig),
			opt, ";", ':', arglen);
	ಆದರೆ (ret)
		ಹಿಂತಿರುಗಿ ret;

	ಹಿಂತಿರುಗಿ hwconfig_parse(cpu_hwconfig, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(cpu_hwconfig),
			opt, ";", ':', arglen);
}

/*
 * hwconfig_f - query if a particular hwconfig option is specified
 * @opt:	a string representing an option
 * @buf:	if non-NULL use this buffer to parse, otherwise try env
 *
 * This call can be used to find out whether U-Boot should configure
 * a particular hardware option.
 *
 * Returns non-zero value if the hardware option can be used and thus
 * should be configured, 0 otherwise.
 *
 * This function also returns non-zero value if CONFIG_HWCONFIG is
 * undefined.
 *
 * Returning non-zero value without CONFIG_HWCONFIG has its crucial
 * purpose: the hwconfig() call should be a "transparent" interface,
 * e.g. if a board doesn't need hwconfig facility, then we assume
 * that the board file only calls things that are actually used, so
 * hwconfig() will always return true result.
 */
ಪೂರ್ಣಾಂಕ hwconfig_f(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *opt, ಅಕ್ಷರ *buf)
{
	ಹಿಂತಿರುಗಿ !!__hwconfig(opt, ಖಾಲಿ, buf);
}

/*
 * hwconfig_arg_f - get hwconfig option's argument
 * @opt:	a string representing an option
 * @arglen:	a pointer to an allocated size_t variable
 * @buf:	if non-NULL use this buffer to parse, otherwise try env
 *
 * Unlike hwconfig_f() function, this function returns a pointer to the
 * start of the hwconfig arguments, if option is not found or it has
 * no specified arguments, the function returns NULL pointer.
 *
 * If CONFIG_HWCONFIG is undefined, the function returns "", and
 * arglen is set to 0.
 */
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *hwconfig_arg_f(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *opt, size_t *arglen, ಅಕ್ಷರ *buf)
{
	ಹಿಂತಿರುಗಿ __hwconfig(opt, arglen, buf);
}

/*
 * hwconfig_arg_cmp_f - compare hwconfig option's argument
 * @opt:	a string representing an option
 * @arg:	a string for comparing an option's argument
 * @buf:	if non-NULL use this buffer to parse, otherwise try env
 *
 * This call is similar to hwconfig_arg_f, but instead of returning
 * hwconfig argument and its length, it is comparing it to @arg.
 *
 * Returns non-zero value if @arg matches, 0 otherwise.
 *
 * If CONFIG_HWCONFIG is undefined, the function returns a non-zero
 * value, i.e. the argument matches.
 */
ಪೂರ್ಣಾಂಕ hwconfig_arg_cmp_f(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *opt, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *arg, ಅಕ್ಷರ *buf)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *argstr;
	size_t arglen;

	argstr = hwconfig_arg_f(opt, &arglen, buf);
	ಆದರೆ (!argstr || arglen != ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(arg))
		ಹಿಂತಿರುಗಿ 0;

	ಹಿಂತಿರುಗಿ !ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(argstr, arg, arglen);
}

/*
 * hwconfig_sub_f - query if a particular hwconfig sub-option is specified
 * @opt:	a string representing an option
 * @subopt:	a string representing a sub-option
 * @buf:	if non-NULL use this buffer to parse, otherwise try env
 *
 * This call is similar to hwconfig_f(), except that it takes additional
 * argument @subopt. In this example:
 *	"dr_usb:mode=peripheral"
 * "dr_usb" is an option, "mode" is a sub-option, and "peripheral" is its
 * argument.
 */
ಪೂರ್ಣಾಂಕ hwconfig_sub_f(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *opt, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *subopt, ಅಕ್ಷರ *buf)
{
	size_t arglen;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *arg;

	arg = __hwconfig(opt, &arglen, buf);
	ಆದರೆ (!arg)
		ಹಿಂತಿರುಗಿ 0;
	ಹಿಂತಿರುಗಿ !!hwconfig_parse(arg, arglen, subopt, ",;", '=', ಖಾಲಿ);
}

/*
 * hwconfig_subarg_f - get hwconfig sub-option's argument
 * @opt:	a string representing an option
 * @subopt:	a string representing a sub-option
 * @subarglen:	a pointer to an allocated size_t variable
 * @buf:	if non-NULL use this buffer to parse, otherwise try env
 *
 * This call is similar to hwconfig_arg_f(), except that it takes an
 * additional argument @subopt, and so works with sub-options.
 */
ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *hwconfig_subarg_f(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *opt, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *subopt,
			      size_t *subarglen, ಅಕ್ಷರ *buf)
{
	size_t arglen;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *arg;

	arg = __hwconfig(opt, &arglen, buf);
	ಆದರೆ (!arg)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	ಹಿಂತಿರುಗಿ hwconfig_parse(arg, arglen, subopt, ",;", '=', subarglen);
}

/*
 * hwconfig_arg_cmp_f - compare hwconfig sub-option's argument
 * @opt:	a string representing an option
 * @subopt:	a string representing a sub-option
 * @subarg:	a string for comparing an sub-option's argument
 * @buf:	if non-NULL use this buffer to parse, otherwise try env
 *
 * This call is similar to hwconfig_arg_cmp_f, except that it takes an
 * additional argument @subopt, and so works with sub-options.
 */
ಪೂರ್ಣಾಂಕ hwconfig_subarg_cmp_f(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *opt, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *subopt,
			  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *subarg, ಅಕ್ಷರ *buf)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *argstr;
	size_t arglen;

	argstr = hwconfig_subarg_f(opt, subopt, &arglen, buf);
	ಆದರೆ (!argstr || arglen != ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(subarg))
		ಹಿಂತಿರುಗಿ 0;

	ಹಿಂತಿರುಗಿ !ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(argstr, subarg, arglen);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ HWCONFIG_TEST
ಪೂರ್ಣಾಂಕ ಮುಖ್ಯ()
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ret;
	size_t len;

	env_ಗಣ("hwconfig", "key1:subkey1=value1,subkey2=value2;key2:value3;;;;"
			   "key3;:,:=;key4", 1);

	ret = hwconfig_arg("key1", &len);
	ಬರೆ("%zd %.*s\n", len, (ಪೂರ್ಣಾಂಕ)len, ret);
	ಪ್ರತಿಜ್ಞಾ(len == 29);
	ಪ್ರತಿಜ್ಞಾ(hwconfig_arg_cmp("key1", "subkey1=value1,subkey2=value2"));
	ಪ್ರತಿಜ್ಞಾ(!ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(ret, "subkey1=value1,subkey2=value2", len));

	ret = hwconfig_subarg("key1", "subkey1", &len);
	ಬರೆ("%zd %.*s\n", len, (ಪೂರ್ಣಾಂಕ)len, ret);
	ಪ್ರತಿಜ್ಞಾ(len == 6);
	ಪ್ರತಿಜ್ಞಾ(hwconfig_subarg_cmp("key1", "subkey1", "value1"));
	ಪ್ರತಿಜ್ಞಾ(!ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(ret, "value1", len));

	ret = hwconfig_subarg("key1", "subkey2", &len);
	ಬರೆ("%zd %.*s\n", len, (ಪೂರ್ಣಾಂಕ)len, ret);
	ಪ್ರತಿಜ್ಞಾ(len == 6);
	ಪ್ರತಿಜ್ಞಾ(hwconfig_subarg_cmp("key1", "subkey2", "value2"));
	ಪ್ರತಿಜ್ಞಾ(!ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(ret, "value2", len));

	ret = hwconfig_arg("key2", &len);
	ಬರೆ("%zd %.*s\n", len, (ಪೂರ್ಣಾಂಕ)len, ret);
	ಪ್ರತಿಜ್ಞಾ(len == 6);
	ಪ್ರತಿಜ್ಞಾ(hwconfig_arg_cmp("key2", "value3"));
	ಪ್ರತಿಜ್ಞಾ(!ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(ret, "value3", len));

	ಪ್ರತಿಜ್ಞಾ(hwconfig("key3"));
	ಪ್ರತಿಜ್ಞಾ(hwconfig_arg("key4", &len) == ಖಾಲಿ);
	ಪ್ರತಿಜ್ಞಾ(hwconfig_arg("bogus", &len) == ಖಾಲಿ);

	unenv_ಗಣ("hwconfig");

	ಪ್ರತಿಜ್ಞಾ(hwconfig(ಖಾಲಿ) == 0);
	ಪ್ರತಿಜ್ಞಾ(hwconfig("") == 0);
	ಪ್ರತಿಜ್ಞಾ(hwconfig("key3") == 0);

	ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ /* HWCONFIG_TEST */
