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
#અન્તર્ભાવ <config.h>
#અન્તર્ભાવ <env.h>
#અન્તર્ભાવ <exports.h>
#અન્તર્ભાવ <hwconfig.h>
#અન્તર્ભાવ <log.h>
#અન્તર્ભાવ <asm/global_data.h>
#અન્તર્ભાવ <linux/types.h>
#અન્તર્ભાવ <linux/string.h>
#નહીંતર
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <stdlib.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <assert.h>
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif /* HWCONFIG_TEST */

DECLARE_GLOBAL_DATA_PTR;

સ્થિર અચલ અક્ષર *hwconfig_parse(અચલ અક્ષર *opts, size_t maxlen,
				  અચલ અક્ષર *opt, અક્ષર *stopchs, અક્ષર eqch,
				  size_t *arglen)
{
	size_t optlen = strlen(opt);
	અક્ષર *str;
	અચલ અક્ષર *start = opts;
	અચલ અક્ષર *end;

next:
	str = strstr(opts, opt);
	end = str + optlen;
	જો (end - start > maxlen)
		પાછા ખાલી;

	જો (str && (str == opts || strpbrk(str - 1, stopchs) == str - 1) &&
			(strpbrk(end, stopchs) == end || *end == eqch ||
			 *end == '\0')) {
		અચલ અક્ષર *arg_end;

		જો (!arglen)
			પાછા str;

		જો (*end != eqch)
			પાછા ખાલી;

		arg_end = strpbrk(str, stopchs);
		જો (!arg_end)
			*arglen = min(maxlen, strlen(str)) - optlen - 1;
		નહીંતર
			*arglen = arg_end - end - 1;

		પાછા end + 1;
	} નહીંતર જો (str) {
		opts = end;
		જાઓ next;
	}
	પાછા ખાલી;
}

અચલ અક્ષર cpu_hwconfig[] __attribute__((weak)) = "";
અચલ અક્ષર board_hwconfig[] __attribute__((weak)) = "";

સ્થિર અચલ અક્ષર *__hwconfig(અચલ અક્ષર *opt, size_t *arglen,
			      અચલ અક્ષર *env_hwconfig)
{
	અચલ અક્ષર *ret;

	/* if we are passed a buffer use it, otherwise try the environment */
	જો (!env_hwconfig) {
#જો CONFIG_IS_ENABLED(ENV_SUPPORT)
		જો (!(gd->flags & GD_FLG_ENV_READY) &&
		    gd->env_valid != ENV_VALID)
#નહીંતર
		જો (true)
#endif
		{
			printf("WARNING: Calling __hwconfig without a buffer "
					"and before environment is ready\n");
			પાછા ખાલી;
		}
#જો CONFIG_IS_ENABLED(ENV_SUPPORT)
		env_hwconfig = env_get("hwconfig");
#endif
	}

	જો (env_hwconfig) {
		ret = hwconfig_parse(env_hwconfig, strlen(env_hwconfig),
				      opt, ";", ':', arglen);
		જો (ret)
			પાછા ret;
	}

	ret = hwconfig_parse(board_hwconfig, strlen(board_hwconfig),
			opt, ";", ':', arglen);
	જો (ret)
		પાછા ret;

	પાછા hwconfig_parse(cpu_hwconfig, strlen(cpu_hwconfig),
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
પૂર્ણાંક hwconfig_f(અચલ અક્ષર *opt, અક્ષર *buf)
{
	પાછા !!__hwconfig(opt, ખાલી, buf);
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
અચલ અક્ષર *hwconfig_arg_f(અચલ અક્ષર *opt, size_t *arglen, અક્ષર *buf)
{
	પાછા __hwconfig(opt, arglen, buf);
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
પૂર્ણાંક hwconfig_arg_cmp_f(અચલ અક્ષર *opt, અચલ અક્ષર *arg, અક્ષર *buf)
{
	અચલ અક્ષર *argstr;
	size_t arglen;

	argstr = hwconfig_arg_f(opt, &arglen, buf);
	જો (!argstr || arglen != strlen(arg))
		પાછા 0;

	પાછા !strncmp(argstr, arg, arglen);
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
પૂર્ણાંક hwconfig_sub_f(અચલ અક્ષર *opt, અચલ અક્ષર *subopt, અક્ષર *buf)
{
	size_t arglen;
	અચલ અક્ષર *arg;

	arg = __hwconfig(opt, &arglen, buf);
	જો (!arg)
		પાછા 0;
	પાછા !!hwconfig_parse(arg, arglen, subopt, ",;", '=', ખાલી);
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
અચલ અક્ષર *hwconfig_subarg_f(અચલ અક્ષર *opt, અચલ અક્ષર *subopt,
			      size_t *subarglen, અક્ષર *buf)
{
	size_t arglen;
	અચલ અક્ષર *arg;

	arg = __hwconfig(opt, &arglen, buf);
	જો (!arg)
		પાછા ખાલી;
	પાછા hwconfig_parse(arg, arglen, subopt, ",;", '=', subarglen);
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
પૂર્ણાંક hwconfig_subarg_cmp_f(અચલ અક્ષર *opt, અચલ અક્ષર *subopt,
			  અચલ અક્ષર *subarg, અક્ષર *buf)
{
	અચલ અક્ષર *argstr;
	size_t arglen;

	argstr = hwconfig_subarg_f(opt, subopt, &arglen, buf);
	જો (!argstr || arglen != strlen(subarg))
		પાછા 0;

	પાછા !strncmp(argstr, subarg, arglen);
}

#ifdef HWCONFIG_TEST
પૂર્ણાંક મુખ્ય()
{
	અચલ અક્ષર *ret;
	size_t len;

	env_set("hwconfig", "key1:subkey1=value1,subkey2=value2;key2:value3;;;;"
			   "key3;:,:=;key4", 1);

	ret = hwconfig_arg("key1", &len);
	printf("%zd %.*s\n", len, (પૂર્ણાંક)len, ret);
	પ્રતિજ્ઞા(len == 29);
	પ્રતિજ્ઞા(hwconfig_arg_cmp("key1", "subkey1=value1,subkey2=value2"));
	પ્રતિજ્ઞા(!strncmp(ret, "subkey1=value1,subkey2=value2", len));

	ret = hwconfig_subarg("key1", "subkey1", &len);
	printf("%zd %.*s\n", len, (પૂર્ણાંક)len, ret);
	પ્રતિજ્ઞા(len == 6);
	પ્રતિજ્ઞા(hwconfig_subarg_cmp("key1", "subkey1", "value1"));
	પ્રતિજ્ઞા(!strncmp(ret, "value1", len));

	ret = hwconfig_subarg("key1", "subkey2", &len);
	printf("%zd %.*s\n", len, (પૂર્ણાંક)len, ret);
	પ્રતિજ્ઞા(len == 6);
	પ્રતિજ્ઞા(hwconfig_subarg_cmp("key1", "subkey2", "value2"));
	પ્રતિજ્ઞા(!strncmp(ret, "value2", len));

	ret = hwconfig_arg("key2", &len);
	printf("%zd %.*s\n", len, (પૂર્ણાંક)len, ret);
	પ્રતિજ્ઞા(len == 6);
	પ્રતિજ્ઞા(hwconfig_arg_cmp("key2", "value3"));
	પ્રતિજ્ઞા(!strncmp(ret, "value3", len));

	પ્રતિજ્ઞા(hwconfig("key3"));
	પ્રતિજ્ઞા(hwconfig_arg("key4", &len) == ખાલી);
	પ્રતિજ્ઞા(hwconfig_arg("bogus", &len) == ખાલી);

	unenv_set("hwconfig");

	પ્રતિજ્ઞા(hwconfig(ખાલી) == 0);
	પ્રતિજ્ઞા(hwconfig("") == 0);
	પ્રતિજ્ઞા(hwconfig("key3") == 0);

	પાછા 0;
}
#endif /* HWCONFIG_TEST */
