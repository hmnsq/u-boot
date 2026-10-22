// SPDX-License-Identifier: GPL-2.0+
/*
 * An inteface for configuring a hardware via u-boot environment.
 *
 * Copyright (c) 2009  MontaVista Software, Inc.
 * Copyright 2011 Freescale Semiconductor, Inc.
 *
 * Author: Anton Vorontsov <avorontsov@ru.mvista.com>
 */

#اگر_غیر_تعریف_شدہ HWCONFIG_TEST
#شامل <config.h>
#شامل <env.h>
#شامل <exports.h>
#شامل <hwconfig.h>
#شامل <log.h>
#شامل <asm/global_data.h>
#شامل <linux/types.h>
#شامل <linux/string.h>
#ورنہ
#شامل <stdio.h>
#شامل <stdlib.h>
#شامل <string.h>
#شامل <assert.h>
#تعریف_کرو min(a, b) (((a) < (b)) ? (a) : (b))
#اگر_ختم /* HWCONFIG_TEST */

DECLARE_GLOBAL_ڈیٹا_PTR;

ساکن ثابت حرف *hwconfig_parse(ثابت حرف *opts, size_t maxlen,
				  ثابت حرف *opt, حرف *stopchs, حرف eqch,
				  size_t *arglen)
{
	size_t optlen = لڑی_لمبائی(opt);
	حرف *str;
	ثابت حرف *آغاز = opts;
	ثابت حرف *انتہا_2;

اگلا:
	str = لڑی_تلاش(opts, opt);
	انتہا_2 = str + optlen;
	اگر (انتہا_2 - آغاز > maxlen)
		واپس تہی;

	اگر (str && (str == opts || strpbrk(str - 1, stopchs) == str - 1) &&
			(strpbrk(انتہا_2, stopchs) == انتہا_2 || *انتہا_2 == eqch ||
			 *انتہا_2 == '\0')) {
		ثابت حرف *arg_انتہا;

		اگر (!arglen)
			واپس str;

		اگر (*انتہا_2 != eqch)
			واپس تہی;

		arg_انتہا = strpbrk(str, stopchs);
		اگر (!arg_انتہا)
			*arglen = min(maxlen, لڑی_لمبائی(str)) - optlen - 1;
		ورنہ
			*arglen = arg_انتہا - انتہا_2 - 1;

		واپس انتہا_2 + 1;
	} ورنہ اگر (str) {
		opts = انتہا_2;
		جاؤ اگلا;
	}
	واپس تہی;
}

ثابت حرف cpu_hwconfig[] __attribute__((weak)) = "";
ثابت حرف board_hwconfig[] __attribute__((weak)) = "";

ساکن ثابت حرف *__hwconfig(ثابت حرف *opt, size_t *arglen,
			      ثابت حرف *env_hwconfig)
{
	ثابت حرف *ret;

	/* if we are passed a buffer use it, otherwise try the environment */
	اگر (!env_hwconfig) {
#اگر CONFIG_IS_ENABLED(ENV_SUPPORT)
		اگر (!(gd->flags & GD_FLG_ENV_READY) &&
		    gd->env_valid != ENV_VALID)
#ورنہ
		اگر (true)
#اگر_ختم
		{
			لکھو("WARNING: Calling __hwconfig without a buffer "
					"and before environment is ready\n");
			واپس تہی;
		}
#اگر CONFIG_IS_ENABLED(ENV_SUPPORT)
		env_hwconfig = env_get("hwconfig");
#اگر_ختم
	}

	اگر (env_hwconfig) {
		ret = hwconfig_parse(env_hwconfig, لڑی_لمبائی(env_hwconfig),
				      opt, ";", ':', arglen);
		اگر (ret)
			واپس ret;
	}

	ret = hwconfig_parse(board_hwconfig, لڑی_لمبائی(board_hwconfig),
			opt, ";", ':', arglen);
	اگر (ret)
		واپس ret;

	واپس hwconfig_parse(cpu_hwconfig, لڑی_لمبائی(cpu_hwconfig),
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
صحیح hwconfig_f(ثابت حرف *opt, حرف *buf)
{
	واپس !!__hwconfig(opt, تہی, buf);
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
ثابت حرف *hwconfig_arg_f(ثابت حرف *opt, size_t *arglen, حرف *buf)
{
	واپس __hwconfig(opt, arglen, buf);
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
صحیح hwconfig_arg_cmp_f(ثابت حرف *opt, ثابت حرف *arg, حرف *buf)
{
	ثابت حرف *argstr;
	size_t arglen;

	argstr = hwconfig_arg_f(opt, &arglen, buf);
	اگر (!argstr || arglen != لڑی_لمبائی(arg))
		واپس 0;

	واپس !لڑی_موازنہ(argstr, arg, arglen);
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
صحیح hwconfig_sub_f(ثابت حرف *opt, ثابت حرف *subopt, حرف *buf)
{
	size_t arglen;
	ثابت حرف *arg;

	arg = __hwconfig(opt, &arglen, buf);
	اگر (!arg)
		واپس 0;
	واپس !!hwconfig_parse(arg, arglen, subopt, ",;", '=', تہی);
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
ثابت حرف *hwconfig_subarg_f(ثابت حرف *opt, ثابت حرف *subopt,
			      size_t *subarglen, حرف *buf)
{
	size_t arglen;
	ثابت حرف *arg;

	arg = __hwconfig(opt, &arglen, buf);
	اگر (!arg)
		واپس تہی;
	واپس hwconfig_parse(arg, arglen, subopt, ",;", '=', subarglen);
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
صحیح hwconfig_subarg_cmp_f(ثابت حرف *opt, ثابت حرف *subopt,
			  ثابت حرف *subarg, حرف *buf)
{
	ثابت حرف *argstr;
	size_t arglen;

	argstr = hwconfig_subarg_f(opt, subopt, &arglen, buf);
	اگر (!argstr || arglen != لڑی_لمبائی(subarg))
		واپس 0;

	واپس !لڑی_موازنہ(argstr, subarg, arglen);
}

#اگر_تعریف_شدہ HWCONFIG_TEST
صحیح مین()
{
	ثابت حرف *ret;
	size_t len;

	env_سیٹ("hwconfig", "key1:subkey1=value1,subkey2=value2;key2:value3;;;;"
			   "key3;:,:=;key4", 1);

	ret = hwconfig_arg("key1", &len);
	لکھو("%zd %.*s\n", len, (صحیح)len, ret);
	أكد(len == 29);
	أكد(hwconfig_arg_cmp("key1", "subkey1=value1,subkey2=value2"));
	أكد(!لڑی_موازنہ(ret, "subkey1=value1,subkey2=value2", len));

	ret = hwconfig_subarg("key1", "subkey1", &len);
	لکھو("%zd %.*s\n", len, (صحیح)len, ret);
	أكد(len == 6);
	أكد(hwconfig_subarg_cmp("key1", "subkey1", "value1"));
	أكد(!لڑی_موازنہ(ret, "value1", len));

	ret = hwconfig_subarg("key1", "subkey2", &len);
	لکھو("%zd %.*s\n", len, (صحیح)len, ret);
	أكد(len == 6);
	أكد(hwconfig_subarg_cmp("key1", "subkey2", "value2"));
	أكد(!لڑی_موازنہ(ret, "value2", len));

	ret = hwconfig_arg("key2", &len);
	لکھو("%zd %.*s\n", len, (صحیح)len, ret);
	أكد(len == 6);
	أكد(hwconfig_arg_cmp("key2", "value3"));
	أكد(!لڑی_موازنہ(ret, "value3", len));

	أكد(hwconfig("key3"));
	أكد(hwconfig_arg("key4", &len) == تہی);
	أكد(hwconfig_arg("bogus", &len) == تہی);

	unenv_سیٹ("hwconfig");

	أكد(hwconfig(تہی) == 0);
	أكد(hwconfig("") == 0);
	أكد(hwconfig("key3") == 0);

	واپس 0;
}
#اگر_ختم /* HWCONFIG_TEST */
