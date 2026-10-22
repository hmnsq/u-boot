// SPDX-License-Identifier: GPL-2.0+
/*
 * An inteface for configuring a hardware via u-boot environment.
 *
 * Copyright (c) 2009  MontaVista Software, Inc.
 * Copyright 2011 Freescale Semiconductor, Inc.
 *
 * Author: Anton Vorontsov <avorontsov@ru.mvista.com>
 */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ HWCONFIG_TEST
#ਅਨ੍ਤਰ੍ਭਾਵ <config.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <env.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <exports.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <hwconfig.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <log.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/global_data.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/types.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/string.h>
#ਨਹੀਂਤਾਂ
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <assert.h>
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ min(a, b) (((a) < (b)) ? (a) : (b))
#ਜੇ_ਅੰਤ /* HWCONFIG_TEST */

DECLARE_GLOBAL_ਅੰਕੜੇ_PTR;

ਸਥਿਰ ਅਚਲ ਅੱਖਰ *hwconfig_parse(ਅਚਲ ਅੱਖਰ *opts, size_t maxlen,
				  ਅਚਲ ਅੱਖਰ *opt, ਅੱਖਰ *stopchs, ਅੱਖਰ eqch,
				  size_t *arglen)
{
	size_t optlen = ਲੜੀ_ਲੰਬਾਈ(opt);
	ਅੱਖਰ *str;
	ਅਚਲ ਅੱਖਰ *ਸ਼ੁਰੂ = opts;
	ਅਚਲ ਅੱਖਰ *ਸਿਰਾ_2;

ਅਗਲਾ:
	str = ਲੜੀ_ਖੋਜ(opts, opt);
	ਸਿਰਾ_2 = str + optlen;
	ਜੇ (ਸਿਰਾ_2 - ਸ਼ੁਰੂ > maxlen)
		ਵਾਪਸ ਨਿਰਾ;

	ਜੇ (str && (str == opts || strpbrk(str - 1, stopchs) == str - 1) &&
			(strpbrk(ਸਿਰਾ_2, stopchs) == ਸਿਰਾ_2 || *ਸਿਰਾ_2 == eqch ||
			 *ਸਿਰਾ_2 == '\0')) {
		ਅਚਲ ਅੱਖਰ *arg_ਸਿਰਾ;

		ਜੇ (!arglen)
			ਵਾਪਸ str;

		ਜੇ (*ਸਿਰਾ_2 != eqch)
			ਵਾਪਸ ਨਿਰਾ;

		arg_ਸਿਰਾ = strpbrk(str, stopchs);
		ਜੇ (!arg_ਸਿਰਾ)
			*arglen = min(maxlen, ਲੜੀ_ਲੰਬਾਈ(str)) - optlen - 1;
		ਨਹੀਂਤਾਂ
			*arglen = arg_ਸਿਰਾ - ਸਿਰਾ_2 - 1;

		ਵਾਪਸ ਸਿਰਾ_2 + 1;
	} ਨਹੀਂਤਾਂ ਜੇ (str) {
		opts = ਸਿਰਾ_2;
		ਜਾਓ ਅਗਲਾ;
	}
	ਵਾਪਸ ਨਿਰਾ;
}

ਅਚਲ ਅੱਖਰ cpu_hwconfig[] __attribute__((weak)) = "";
ਅਚਲ ਅੱਖਰ board_hwconfig[] __attribute__((weak)) = "";

ਸਥਿਰ ਅਚਲ ਅੱਖਰ *__hwconfig(ਅਚਲ ਅੱਖਰ *opt, size_t *arglen,
			      ਅਚਲ ਅੱਖਰ *env_hwconfig)
{
	ਅਚਲ ਅੱਖਰ *ret;

	/* if we are passed a buffer use it, otherwise try the environment */
	ਜੇ (!env_hwconfig) {
#ਜੇ CONFIG_IS_ENABLED(ENV_SUPPORT)
		ਜੇ (!(gd->flags & GD_FLG_ENV_READY) &&
		    gd->env_valid != ENV_VALID)
#ਨਹੀਂਤਾਂ
		ਜੇ (true)
#ਜੇ_ਅੰਤ
		{
			ਲਿਖੋ("WARNING: Calling __hwconfig without a buffer "
					"and before environment is ready\n");
			ਵਾਪਸ ਨਿਰਾ;
		}
#ਜੇ CONFIG_IS_ENABLED(ENV_SUPPORT)
		env_hwconfig = env_get("hwconfig");
#ਜੇ_ਅੰਤ
	}

	ਜੇ (env_hwconfig) {
		ret = hwconfig_parse(env_hwconfig, ਲੜੀ_ਲੰਬਾਈ(env_hwconfig),
				      opt, ";", ':', arglen);
		ਜੇ (ret)
			ਵਾਪਸ ret;
	}

	ret = hwconfig_parse(board_hwconfig, ਲੜੀ_ਲੰਬਾਈ(board_hwconfig),
			opt, ";", ':', arglen);
	ਜੇ (ret)
		ਵਾਪਸ ret;

	ਵਾਪਸ hwconfig_parse(cpu_hwconfig, ਲੜੀ_ਲੰਬਾਈ(cpu_hwconfig),
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
ਪੂਰਨ hwconfig_f(ਅਚਲ ਅੱਖਰ *opt, ਅੱਖਰ *buf)
{
	ਵਾਪਸ !!__hwconfig(opt, ਨਿਰਾ, buf);
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
ਅਚਲ ਅੱਖਰ *hwconfig_arg_f(ਅਚਲ ਅੱਖਰ *opt, size_t *arglen, ਅੱਖਰ *buf)
{
	ਵਾਪਸ __hwconfig(opt, arglen, buf);
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
ਪੂਰਨ hwconfig_arg_cmp_f(ਅਚਲ ਅੱਖਰ *opt, ਅਚਲ ਅੱਖਰ *arg, ਅੱਖਰ *buf)
{
	ਅਚਲ ਅੱਖਰ *argstr;
	size_t arglen;

	argstr = hwconfig_arg_f(opt, &arglen, buf);
	ਜੇ (!argstr || arglen != ਲੜੀ_ਲੰਬਾਈ(arg))
		ਵਾਪਸ 0;

	ਵਾਪਸ !ਲੜੀ_ਤੁਲਨਾ(argstr, arg, arglen);
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
ਪੂਰਨ hwconfig_sub_f(ਅਚਲ ਅੱਖਰ *opt, ਅਚਲ ਅੱਖਰ *subopt, ਅੱਖਰ *buf)
{
	size_t arglen;
	ਅਚਲ ਅੱਖਰ *arg;

	arg = __hwconfig(opt, &arglen, buf);
	ਜੇ (!arg)
		ਵਾਪਸ 0;
	ਵਾਪਸ !!hwconfig_parse(arg, arglen, subopt, ",;", '=', ਨਿਰਾ);
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
ਅਚਲ ਅੱਖਰ *hwconfig_subarg_f(ਅਚਲ ਅੱਖਰ *opt, ਅਚਲ ਅੱਖਰ *subopt,
			      size_t *subarglen, ਅੱਖਰ *buf)
{
	size_t arglen;
	ਅਚਲ ਅੱਖਰ *arg;

	arg = __hwconfig(opt, &arglen, buf);
	ਜੇ (!arg)
		ਵਾਪਸ ਨਿਰਾ;
	ਵਾਪਸ hwconfig_parse(arg, arglen, subopt, ",;", '=', subarglen);
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
ਪੂਰਨ hwconfig_subarg_cmp_f(ਅਚਲ ਅੱਖਰ *opt, ਅਚਲ ਅੱਖਰ *subopt,
			  ਅਚਲ ਅੱਖਰ *subarg, ਅੱਖਰ *buf)
{
	ਅਚਲ ਅੱਖਰ *argstr;
	size_t arglen;

	argstr = hwconfig_subarg_f(opt, subopt, &arglen, buf);
	ਜੇ (!argstr || arglen != ਲੜੀ_ਲੰਬਾਈ(subarg))
		ਵਾਪਸ 0;

	ਵਾਪਸ !ਲੜੀ_ਤੁਲਨਾ(argstr, subarg, arglen);
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ HWCONFIG_TEST
ਪੂਰਨ ਮੁੱਖ()
{
	ਅਚਲ ਅੱਖਰ *ret;
	size_t len;

	env_ਸੈੱਟ("hwconfig", "key1:subkey1=value1,subkey2=value2;key2:value3;;;;"
			   "key3;:,:=;key4", 1);

	ret = hwconfig_arg("key1", &len);
	ਲਿਖੋ("%zd %.*s\n", len, (ਪੂਰਨ)len, ret);
	ਪ੍ਰਤਿਜ੍ਞਾ(len == 29);
	ਪ੍ਰਤਿਜ੍ਞਾ(hwconfig_arg_cmp("key1", "subkey1=value1,subkey2=value2"));
	ਪ੍ਰਤਿਜ੍ਞਾ(!ਲੜੀ_ਤੁਲਨਾ(ret, "subkey1=value1,subkey2=value2", len));

	ret = hwconfig_subarg("key1", "subkey1", &len);
	ਲਿਖੋ("%zd %.*s\n", len, (ਪੂਰਨ)len, ret);
	ਪ੍ਰਤਿਜ੍ਞਾ(len == 6);
	ਪ੍ਰਤਿਜ੍ਞਾ(hwconfig_subarg_cmp("key1", "subkey1", "value1"));
	ਪ੍ਰਤਿਜ੍ਞਾ(!ਲੜੀ_ਤੁਲਨਾ(ret, "value1", len));

	ret = hwconfig_subarg("key1", "subkey2", &len);
	ਲਿਖੋ("%zd %.*s\n", len, (ਪੂਰਨ)len, ret);
	ਪ੍ਰਤਿਜ੍ਞਾ(len == 6);
	ਪ੍ਰਤਿਜ੍ਞਾ(hwconfig_subarg_cmp("key1", "subkey2", "value2"));
	ਪ੍ਰਤਿਜ੍ਞਾ(!ਲੜੀ_ਤੁਲਨਾ(ret, "value2", len));

	ret = hwconfig_arg("key2", &len);
	ਲਿਖੋ("%zd %.*s\n", len, (ਪੂਰਨ)len, ret);
	ਪ੍ਰਤਿਜ੍ਞਾ(len == 6);
	ਪ੍ਰਤਿਜ੍ਞਾ(hwconfig_arg_cmp("key2", "value3"));
	ਪ੍ਰਤਿਜ੍ਞਾ(!ਲੜੀ_ਤੁਲਨਾ(ret, "value3", len));

	ਪ੍ਰਤਿਜ੍ਞਾ(hwconfig("key3"));
	ਪ੍ਰਤਿਜ੍ਞਾ(hwconfig_arg("key4", &len) == ਨਿਰਾ);
	ਪ੍ਰਤਿਜ੍ਞਾ(hwconfig_arg("bogus", &len) == ਨਿਰਾ);

	unenv_ਸੈੱਟ("hwconfig");

	ਪ੍ਰਤਿਜ੍ਞਾ(hwconfig(ਨਿਰਾ) == 0);
	ਪ੍ਰਤਿਜ੍ਞਾ(hwconfig("") == 0);
	ਪ੍ਰਤਿਜ੍ਞਾ(hwconfig("key3") == 0);

	ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ /* HWCONFIG_TEST */
