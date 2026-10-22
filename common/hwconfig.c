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
#شامل <config.h>
#شامل <env.h>
#شامل <exports.h>
#شامل <hwconfig.h>
#شامل <log.h>
#شامل <asm/global_data.h>
#شامل <linux/types.h>
#شامل <linux/string.h>
#وگرنه
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <string.h>
#شامل <assert.h>
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif /* HWCONFIG_TEST */

DECLARE_GLOBAL_DATA_PTR;

ایستا ثابت نویسه *hwconfig_parse(ثابت نویسه *opts, size_t maxlen,
				  ثابت نویسه *opt, نویسه *stopchs, نویسه eqch,
				  size_t *arglen)
{
	size_t optlen = strlen(opt);
	نویسه *str;
	ثابت نویسه *start = opts;
	ثابت نویسه *end;

next:
	str = strstr(opts, opt);
	end = str + optlen;
	اگر (end - start > maxlen)
		بازگشت پوچ;

	اگر (str && (str == opts || strpbrk(str - 1, stopchs) == str - 1) &&
			(strpbrk(end, stopchs) == end || *end == eqch ||
			 *end == '\0')) {
		ثابت نویسه *arg_end;

		اگر (!arglen)
			بازگشت str;

		اگر (*end != eqch)
			بازگشت پوچ;

		arg_end = strpbrk(str, stopchs);
		اگر (!arg_end)
			*arglen = min(maxlen, strlen(str)) - optlen - 1;
		وگرنه
			*arglen = arg_end - end - 1;

		بازگشت end + 1;
	} وگرنه اگر (str) {
		opts = end;
		برو next;
	}
	بازگشت پوچ;
}

ثابت نویسه cpu_hwconfig[] __attribute__((weak)) = "";
ثابت نویسه board_hwconfig[] __attribute__((weak)) = "";

ایستا ثابت نویسه *__hwconfig(ثابت نویسه *opt, size_t *arglen,
			      ثابت نویسه *env_hwconfig)
{
	ثابت نویسه *ret;

	/* if we are passed a buffer use it, otherwise try the environment */
	اگر (!env_hwconfig) {
#اگر CONFIG_IS_ENABLED(ENV_SUPPORT)
		اگر (!(gd->flags & GD_FLG_ENV_READY) &&
		    gd->env_valid != ENV_VALID)
#وگرنه
		اگر (true)
#endif
		{
			printf("WARNING: Calling __hwconfig without a buffer "
					"and before environment is ready\n");
			بازگشت پوچ;
		}
#اگر CONFIG_IS_ENABLED(ENV_SUPPORT)
		env_hwconfig = env_get("hwconfig");
#endif
	}

	اگر (env_hwconfig) {
		ret = hwconfig_parse(env_hwconfig, strlen(env_hwconfig),
				      opt, ";", ':', arglen);
		اگر (ret)
			بازگشت ret;
	}

	ret = hwconfig_parse(board_hwconfig, strlen(board_hwconfig),
			opt, ";", ':', arglen);
	اگر (ret)
		بازگشت ret;

	بازگشت hwconfig_parse(cpu_hwconfig, strlen(cpu_hwconfig),
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
صحیح hwconfig_f(ثابت نویسه *opt, نویسه *buf)
{
	بازگشت !!__hwconfig(opt, پوچ, buf);
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
ثابت نویسه *hwconfig_arg_f(ثابت نویسه *opt, size_t *arglen, نویسه *buf)
{
	بازگشت __hwconfig(opt, arglen, buf);
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
صحیح hwconfig_arg_cmp_f(ثابت نویسه *opt, ثابت نویسه *arg, نویسه *buf)
{
	ثابت نویسه *argstr;
	size_t arglen;

	argstr = hwconfig_arg_f(opt, &arglen, buf);
	اگر (!argstr || arglen != strlen(arg))
		بازگشت 0;

	بازگشت !strncmp(argstr, arg, arglen);
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
صحیح hwconfig_sub_f(ثابت نویسه *opt, ثابت نویسه *subopt, نویسه *buf)
{
	size_t arglen;
	ثابت نویسه *arg;

	arg = __hwconfig(opt, &arglen, buf);
	اگر (!arg)
		بازگشت 0;
	بازگشت !!hwconfig_parse(arg, arglen, subopt, ",;", '=', پوچ);
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
ثابت نویسه *hwconfig_subarg_f(ثابت نویسه *opt, ثابت نویسه *subopt,
			      size_t *subarglen, نویسه *buf)
{
	size_t arglen;
	ثابت نویسه *arg;

	arg = __hwconfig(opt, &arglen, buf);
	اگر (!arg)
		بازگشت پوچ;
	بازگشت hwconfig_parse(arg, arglen, subopt, ",;", '=', subarglen);
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
صحیح hwconfig_subarg_cmp_f(ثابت نویسه *opt, ثابت نویسه *subopt,
			  ثابت نویسه *subarg, نویسه *buf)
{
	ثابت نویسه *argstr;
	size_t arglen;

	argstr = hwconfig_subarg_f(opt, subopt, &arglen, buf);
	اگر (!argstr || arglen != strlen(subarg))
		بازگشت 0;

	بازگشت !strncmp(argstr, subarg, arglen);
}

#ifdef HWCONFIG_TEST
صحیح اصلی()
{
	ثابت نویسه *ret;
	size_t len;

	env_set("hwconfig", "key1:subkey1=value1,subkey2=value2;key2:value3;;;;"
			   "key3;:,:=;key4", 1);

	ret = hwconfig_arg("key1", &len);
	printf("%zd %.*s\n", len, (صحیح)len, ret);
	أكد(len == 29);
	أكد(hwconfig_arg_cmp("key1", "subkey1=value1,subkey2=value2"));
	أكد(!strncmp(ret, "subkey1=value1,subkey2=value2", len));

	ret = hwconfig_subarg("key1", "subkey1", &len);
	printf("%zd %.*s\n", len, (صحیح)len, ret);
	أكد(len == 6);
	أكد(hwconfig_subarg_cmp("key1", "subkey1", "value1"));
	أكد(!strncmp(ret, "value1", len));

	ret = hwconfig_subarg("key1", "subkey2", &len);
	printf("%zd %.*s\n", len, (صحیح)len, ret);
	أكد(len == 6);
	أكد(hwconfig_subarg_cmp("key1", "subkey2", "value2"));
	أكد(!strncmp(ret, "value2", len));

	ret = hwconfig_arg("key2", &len);
	printf("%zd %.*s\n", len, (صحیح)len, ret);
	أكد(len == 6);
	أكد(hwconfig_arg_cmp("key2", "value3"));
	أكد(!strncmp(ret, "value3", len));

	أكد(hwconfig("key3"));
	أكد(hwconfig_arg("key4", &len) == پوچ);
	أكد(hwconfig_arg("bogus", &len) == پوچ);

	unenv_set("hwconfig");

	أكد(hwconfig(پوچ) == 0);
	أكد(hwconfig("") == 0);
	أكد(hwconfig("key3") == 0);

	بازگشت 0;
}
#endif /* HWCONFIG_TEST */
