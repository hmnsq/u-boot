// SPDX-License-Identifier: GPL-2.0+
/*
 * An inteface for configuring a hardware via u-boot environment.
 *
 * Copyright (c) 2009  MontaVista Software, Inc.
 * Copyright 2011 Freescale Semiconductor, Inc.
 *
 * Author: Anton Vorontsov <avorontsov@ru.mvista.com>
 */

#ifndef HWCONFIG_TEST
#ಅನ್ತರ್ಭಾವ <config.h>
#ಅನ್ತರ್ಭಾವ <env.h>
#ಅನ್ತರ್ಭಾವ <exports.h>
#ಅನ್ತರ್ಭಾವ <hwconfig.h>
#ಅನ್ತರ್ಭಾವ <log.h>
#ಅನ್ತರ್ಭಾವ <asm/global_data.h>
#ಅನ್ತರ್ಭಾವ <linux/types.h>
#ಅನ್ತರ್ಭಾವ <linux/string.h>
#ಅನ್ಯಥಾ
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <stdlib.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <assert.h>
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif /* HWCONFIG_TEST */

DECLARE_GLOBAL_DATA_PTR;

ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *hwconfig_parse(ಧ್ರುವ ವರ್ಣ *opts, size_t maxlen,
				  ಧ್ರುವ ವರ್ಣ *opt, ವರ್ಣ *stopchs, ವರ್ಣ eqch,
				  size_t *arglen)
{
	size_t optlen = strlen(opt);
	ವರ್ಣ *str;
	ಧ್ರುವ ವರ್ಣ *start = opts;
	ಧ್ರುವ ವರ್ಣ *end;

next:
	str = strstr(opts, opt);
	end = str + optlen;
	ಯದಿ (end - start > maxlen)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	ಯದಿ (str && (str == opts || strpbrk(str - 1, stopchs) == str - 1) &&
			(strpbrk(end, stopchs) == end || *end == eqch ||
			 *end == '\0')) {
		ಧ್ರುವ ವರ್ಣ *arg_end;

		ಯದಿ (!arglen)
			ಪ್ರತ್ಯಾಗಮ str;

		ಯದಿ (*end != eqch)
			ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

		arg_end = strpbrk(str, stopchs);
		ಯದಿ (!arg_end)
			*arglen = min(maxlen, strlen(str)) - optlen - 1;
		ಅನ್ಯಥಾ
			*arglen = arg_end - end - 1;

		ಪ್ರತ್ಯಾಗಮ end + 1;
	} ಅನ್ಯಥಾ ಯದಿ (str) {
		opts = end;
		ಗಚ್ಛ next;
	}
	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

ಧ್ರುವ ವರ್ಣ cpu_hwconfig[] __attribute__((weak)) = "";
ಧ್ರುವ ವರ್ಣ board_hwconfig[] __attribute__((weak)) = "";

ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *__hwconfig(ಧ್ರುವ ವರ್ಣ *opt, size_t *arglen,
			      ಧ್ರುವ ವರ್ಣ *env_hwconfig)
{
	ಧ್ರುವ ವರ್ಣ *ret;

	/* if we are passed a buffer use it, otherwise try the environment */
	ಯದಿ (!env_hwconfig) {
#ಯದಿ CONFIG_IS_ENABLED(ENV_SUPPORT)
		ಯದಿ (!(gd->flags & GD_FLG_ENV_READY) &&
		    gd->env_valid != ENV_VALID)
#ಅನ್ಯಥಾ
		ಯದಿ (true)
#endif
		{
			printf("WARNING: Calling __hwconfig without a buffer "
					"and before environment is ready\n");
			ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
		}
#ಯದಿ CONFIG_IS_ENABLED(ENV_SUPPORT)
		env_hwconfig = env_get("hwconfig");
#endif
	}

	ಯದಿ (env_hwconfig) {
		ret = hwconfig_parse(env_hwconfig, strlen(env_hwconfig),
				      opt, ";", ':', arglen);
		ಯದಿ (ret)
			ಪ್ರತ್ಯಾಗಮ ret;
	}

	ret = hwconfig_parse(board_hwconfig, strlen(board_hwconfig),
			opt, ";", ':', arglen);
	ಯದಿ (ret)
		ಪ್ರತ್ಯಾಗಮ ret;

	ಪ್ರತ್ಯಾಗಮ hwconfig_parse(cpu_hwconfig, strlen(cpu_hwconfig),
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
ಪೂರ್ಣಾಙ್ಕ hwconfig_f(ಧ್ರುವ ವರ್ಣ *opt, ವರ್ಣ *buf)
{
	ಪ್ರತ್ಯಾಗಮ !!__hwconfig(opt, ರಿಕ್ತ, buf);
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
ಧ್ರುವ ವರ್ಣ *hwconfig_arg_f(ಧ್ರುವ ವರ್ಣ *opt, size_t *arglen, ವರ್ಣ *buf)
{
	ಪ್ರತ್ಯಾಗಮ __hwconfig(opt, arglen, buf);
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
ಪೂರ್ಣಾಙ್ಕ hwconfig_arg_cmp_f(ಧ್ರುವ ವರ್ಣ *opt, ಧ್ರುವ ವರ್ಣ *arg, ವರ್ಣ *buf)
{
	ಧ್ರುವ ವರ್ಣ *argstr;
	size_t arglen;

	argstr = hwconfig_arg_f(opt, &arglen, buf);
	ಯದಿ (!argstr || arglen != strlen(arg))
		ಪ್ರತ್ಯಾಗಮ 0;

	ಪ್ರತ್ಯಾಗಮ !strncmp(argstr, arg, arglen);
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
ಪೂರ್ಣಾಙ್ಕ hwconfig_sub_f(ಧ್ರುವ ವರ್ಣ *opt, ಧ್ರುವ ವರ್ಣ *subopt, ವರ್ಣ *buf)
{
	size_t arglen;
	ಧ್ರುವ ವರ್ಣ *arg;

	arg = __hwconfig(opt, &arglen, buf);
	ಯದಿ (!arg)
		ಪ್ರತ್ಯಾಗಮ 0;
	ಪ್ರತ್ಯಾಗಮ !!hwconfig_parse(arg, arglen, subopt, ",;", '=', ರಿಕ್ತ);
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
ಧ್ರುವ ವರ್ಣ *hwconfig_subarg_f(ಧ್ರುವ ವರ್ಣ *opt, ಧ್ರುವ ವರ್ಣ *subopt,
			      size_t *subarglen, ವರ್ಣ *buf)
{
	size_t arglen;
	ಧ್ರುವ ವರ್ಣ *arg;

	arg = __hwconfig(opt, &arglen, buf);
	ಯದಿ (!arg)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	ಪ್ರತ್ಯಾಗಮ hwconfig_parse(arg, arglen, subopt, ",;", '=', subarglen);
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
ಪೂರ್ಣಾಙ್ಕ hwconfig_subarg_cmp_f(ಧ್ರುವ ವರ್ಣ *opt, ಧ್ರುವ ವರ್ಣ *subopt,
			  ಧ್ರುವ ವರ್ಣ *subarg, ವರ್ಣ *buf)
{
	ಧ್ರುವ ವರ್ಣ *argstr;
	size_t arglen;

	argstr = hwconfig_subarg_f(opt, subopt, &arglen, buf);
	ಯದಿ (!argstr || arglen != strlen(subarg))
		ಪ್ರತ್ಯಾಗಮ 0;

	ಪ್ರತ್ಯಾಗಮ !strncmp(argstr, subarg, arglen);
}

#ifdef HWCONFIG_TEST
ಪೂರ್ಣಾಙ್ಕ main()
{
	ಧ್ರುವ ವರ್ಣ *ret;
	size_t len;

	env_set("hwconfig", "key1:subkey1=value1,subkey2=value2;key2:value3;;;;"
			   "key3;:,:=;key4", 1);

	ret = hwconfig_arg("key1", &len);
	printf("%zd %.*s\n", len, (ಪೂರ್ಣಾಙ್ಕ)len, ret);
	ಪ್ರತಿಜ್ಞಾ(len == 29);
	ಪ್ರತಿಜ್ಞಾ(hwconfig_arg_cmp("key1", "subkey1=value1,subkey2=value2"));
	ಪ್ರತಿಜ್ಞಾ(!strncmp(ret, "subkey1=value1,subkey2=value2", len));

	ret = hwconfig_subarg("key1", "subkey1", &len);
	printf("%zd %.*s\n", len, (ಪೂರ್ಣಾಙ್ಕ)len, ret);
	ಪ್ರತಿಜ್ಞಾ(len == 6);
	ಪ್ರತಿಜ್ಞಾ(hwconfig_subarg_cmp("key1", "subkey1", "value1"));
	ಪ್ರತಿಜ್ಞಾ(!strncmp(ret, "value1", len));

	ret = hwconfig_subarg("key1", "subkey2", &len);
	printf("%zd %.*s\n", len, (ಪೂರ್ಣಾಙ್ಕ)len, ret);
	ಪ್ರತಿಜ್ಞಾ(len == 6);
	ಪ್ರತಿಜ್ಞಾ(hwconfig_subarg_cmp("key1", "subkey2", "value2"));
	ಪ್ರತಿಜ್ಞಾ(!strncmp(ret, "value2", len));

	ret = hwconfig_arg("key2", &len);
	printf("%zd %.*s\n", len, (ಪೂರ್ಣಾಙ್ಕ)len, ret);
	ಪ್ರತಿಜ್ಞಾ(len == 6);
	ಪ್ರತಿಜ್ಞಾ(hwconfig_arg_cmp("key2", "value3"));
	ಪ್ರತಿಜ್ಞಾ(!strncmp(ret, "value3", len));

	ಪ್ರತಿಜ್ಞಾ(hwconfig("key3"));
	ಪ್ರತಿಜ್ಞಾ(hwconfig_arg("key4", &len) == ರಿಕ್ತ);
	ಪ್ರತಿಜ್ಞಾ(hwconfig_arg("bogus", &len) == ರಿಕ್ತ);

	unenv_set("hwconfig");

	ಪ್ರತಿಜ್ಞಾ(hwconfig(ರಿಕ್ತ) == 0);
	ಪ್ರತಿಜ್ಞಾ(hwconfig("") == 0);
	ಪ್ರತಿಜ್ಞಾ(hwconfig("key3") == 0);

	ಪ್ರತ್ಯಾಗಮ 0;
}
#endif /* HWCONFIG_TEST */
