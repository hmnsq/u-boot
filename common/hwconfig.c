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
#অন্তর্ভাব <config.h>
#অন্তর্ভাব <env.h>
#অন্তর্ভাব <exports.h>
#অন্তর্ভাব <hwconfig.h>
#অন্তর্ভাব <log.h>
#অন্তর্ভাব <asm/global_data.h>
#অন্তর্ভাব <linux/types.h>
#অন্তর্ভাব <linux/string.h>
#অন্যথা
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <stdlib.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <assert.h>
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif /* HWCONFIG_TEST */

DECLARE_GLOBAL_তথ্য_PTR;

স্থির ধ্রুবক অক্ষর *hwconfig_parse(ধ্রুবক অক্ষর *opts, size_t maxlen,
				  ধ্রুবক অক্ষর *opt, অক্ষর *stopchs, অক্ষর eqch,
				  size_t *arglen)
{
	size_t optlen = strlen(opt);
	অক্ষর *str;
	ধ্রুবক অক্ষর *start = opts;
	ধ্রুবক অক্ষর *end;

পরবর্তী:
	str = strstr(opts, opt);
	end = str + optlen;
	যদি (end - start > maxlen)
		ফেরত ফাঁকা;

	যদি (str && (str == opts || strpbrk(str - 1, stopchs) == str - 1) &&
			(strpbrk(end, stopchs) == end || *end == eqch ||
			 *end == '\0')) {
		ধ্রুবক অক্ষর *arg_end;

		যদি (!arglen)
			ফেরত str;

		যদি (*end != eqch)
			ফেরত ফাঁকা;

		arg_end = strpbrk(str, stopchs);
		যদি (!arg_end)
			*arglen = min(maxlen, strlen(str)) - optlen - 1;
		অন্যথা
			*arglen = arg_end - end - 1;

		ফেরত end + 1;
	} অন্যথা যদি (str) {
		opts = end;
		যাও পরবর্তী;
	}
	ফেরত ফাঁকা;
}

ধ্রুবক অক্ষর cpu_hwconfig[] __attribute__((weak)) = "";
ধ্রুবক অক্ষর board_hwconfig[] __attribute__((weak)) = "";

স্থির ধ্রুবক অক্ষর *__hwconfig(ধ্রুবক অক্ষর *opt, size_t *arglen,
			      ধ্রুবক অক্ষর *env_hwconfig)
{
	ধ্রুবক অক্ষর *ret;

	/* if we are passed a buffer use it, otherwise try the environment */
	যদি (!env_hwconfig) {
#যদি CONFIG_IS_ENABLED(ENV_SUPPORT)
		যদি (!(gd->flags & GD_FLG_ENV_READY) &&
		    gd->env_valid != ENV_VALID)
#অন্যথা
		যদি (true)
#endif
		{
			printf("WARNING: Calling __hwconfig without a buffer "
					"and before environment is ready\n");
			ফেরত ফাঁকা;
		}
#যদি CONFIG_IS_ENABLED(ENV_SUPPORT)
		env_hwconfig = env_get("hwconfig");
#endif
	}

	যদি (env_hwconfig) {
		ret = hwconfig_parse(env_hwconfig, strlen(env_hwconfig),
				      opt, ";", ':', arglen);
		যদি (ret)
			ফেরত ret;
	}

	ret = hwconfig_parse(board_hwconfig, strlen(board_hwconfig),
			opt, ";", ':', arglen);
	যদি (ret)
		ফেরত ret;

	ফেরত hwconfig_parse(cpu_hwconfig, strlen(cpu_hwconfig),
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
পূর্ণসংখ্যা hwconfig_f(ধ্রুবক অক্ষর *opt, অক্ষর *buf)
{
	ফেরত !!__hwconfig(opt, ফাঁকা, buf);
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
ধ্রুবক অক্ষর *hwconfig_arg_f(ধ্রুবক অক্ষর *opt, size_t *arglen, অক্ষর *buf)
{
	ফেরত __hwconfig(opt, arglen, buf);
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
পূর্ণসংখ্যা hwconfig_arg_cmp_f(ধ্রুবক অক্ষর *opt, ধ্রুবক অক্ষর *arg, অক্ষর *buf)
{
	ধ্রুবক অক্ষর *argstr;
	size_t arglen;

	argstr = hwconfig_arg_f(opt, &arglen, buf);
	যদি (!argstr || arglen != strlen(arg))
		ফেরত 0;

	ফেরত !strncmp(argstr, arg, arglen);
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
পূর্ণসংখ্যা hwconfig_sub_f(ধ্রুবক অক্ষর *opt, ধ্রুবক অক্ষর *subopt, অক্ষর *buf)
{
	size_t arglen;
	ধ্রুবক অক্ষর *arg;

	arg = __hwconfig(opt, &arglen, buf);
	যদি (!arg)
		ফেরত 0;
	ফেরত !!hwconfig_parse(arg, arglen, subopt, ",;", '=', ফাঁকা);
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
ধ্রুবক অক্ষর *hwconfig_subarg_f(ধ্রুবক অক্ষর *opt, ধ্রুবক অক্ষর *subopt,
			      size_t *subarglen, অক্ষর *buf)
{
	size_t arglen;
	ধ্রুবক অক্ষর *arg;

	arg = __hwconfig(opt, &arglen, buf);
	যদি (!arg)
		ফেরত ফাঁকা;
	ফেরত hwconfig_parse(arg, arglen, subopt, ",;", '=', subarglen);
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
পূর্ণসংখ্যা hwconfig_subarg_cmp_f(ধ্রুবক অক্ষর *opt, ধ্রুবক অক্ষর *subopt,
			  ধ্রুবক অক্ষর *subarg, অক্ষর *buf)
{
	ধ্রুবক অক্ষর *argstr;
	size_t arglen;

	argstr = hwconfig_subarg_f(opt, subopt, &arglen, buf);
	যদি (!argstr || arglen != strlen(subarg))
		ফেরত 0;

	ফেরত !strncmp(argstr, subarg, arglen);
}

#ifdef HWCONFIG_TEST
পূর্ণসংখ্যা প্রধান()
{
	ধ্রুবক অক্ষর *ret;
	size_t len;

	env_set("hwconfig", "key1:subkey1=value1,subkey2=value2;key2:value3;;;;"
			   "key3;:,:=;key4", 1);

	ret = hwconfig_arg("key1", &len);
	printf("%zd %.*s\n", len, (পূর্ণসংখ্যা)len, ret);
	প্রতিজ্ঞা(len == 29);
	প্রতিজ্ঞা(hwconfig_arg_cmp("key1", "subkey1=value1,subkey2=value2"));
	প্রতিজ্ঞা(!strncmp(ret, "subkey1=value1,subkey2=value2", len));

	ret = hwconfig_subarg("key1", "subkey1", &len);
	printf("%zd %.*s\n", len, (পূর্ণসংখ্যা)len, ret);
	প্রতিজ্ঞা(len == 6);
	প্রতিজ্ঞা(hwconfig_subarg_cmp("key1", "subkey1", "value1"));
	প্রতিজ্ঞা(!strncmp(ret, "value1", len));

	ret = hwconfig_subarg("key1", "subkey2", &len);
	printf("%zd %.*s\n", len, (পূর্ণসংখ্যা)len, ret);
	প্রতিজ্ঞা(len == 6);
	প্রতিজ্ঞা(hwconfig_subarg_cmp("key1", "subkey2", "value2"));
	প্রতিজ্ঞা(!strncmp(ret, "value2", len));

	ret = hwconfig_arg("key2", &len);
	printf("%zd %.*s\n", len, (পূর্ণসংখ্যা)len, ret);
	প্রতিজ্ঞা(len == 6);
	প্রতিজ্ঞা(hwconfig_arg_cmp("key2", "value3"));
	প্রতিজ্ঞা(!strncmp(ret, "value3", len));

	প্রতিজ্ঞা(hwconfig("key3"));
	প্রতিজ্ঞা(hwconfig_arg("key4", &len) == ফাঁকা);
	প্রতিজ্ঞা(hwconfig_arg("bogus", &len) == ফাঁকা);

	unenv_set("hwconfig");

	প্রতিজ্ঞা(hwconfig(ফাঁকা) == 0);
	প্রতিজ্ঞা(hwconfig("") == 0);
	প্রতিজ্ঞা(hwconfig("key3") == 0);

	ফেরত 0;
}
#endif /* HWCONFIG_TEST */
