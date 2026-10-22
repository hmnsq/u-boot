// SPDX-License-Identifier: GPL-2.0+
/*
 * An inteface for configuring a hardware via u-boot environment.
 *
 * Copyright (c) 2009  MontaVista Software, Inc.
 * Copyright 2011 Freescale Semiconductor, Inc.
 *
 * Author: Anton Vorontsov <avorontsov@ru.mvista.com>
 */

#నిర్వచితం_కాకపోతే HWCONFIG_TEST
#అన్తర్భావ <config.h>
#అన్తర్భావ <env.h>
#అన్తర్భావ <exports.h>
#అన్తర్భావ <hwconfig.h>
#అన్తర్భావ <log.h>
#అన్తర్భావ <asm/global_data.h>
#అన్తర్భావ <linux/types.h>
#అన్తర్భావ <linux/string.h>
#లేకపోతే
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <stdlib.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <assert.h>
#నిర్వచించు min(a, b) (((a) < (b)) ? (a) : (b))
#అయితే_ముగింపు /* HWCONFIG_TEST */

DECLARE_GLOBAL_దత్తాంశం_PTR;

స్థిర స్థిరరాశి అక్షరం *hwconfig_parse(స్థిరరాశి అక్షరం *opts, size_t maxlen,
				  స్థిరరాశి అక్షరం *opt, అక్షరం *stopchs, అక్షరం eqch,
				  size_t *arglen)
{
	size_t optlen = అక్షరశ్రేణి_పొడవు(opt);
	అక్షరం *str;
	స్థిరరాశి అక్షరం *ప్రారంభం = opts;
	స్థిరరాశి అక్షరం *చివర_2;

తదుపరి:
	str = అక్షరశ్రేణి_శోధన(opts, opt);
	చివర_2 = str + optlen;
	ఒకవేళ (చివర_2 - ప్రారంభం > maxlen)
		తిరిగి ఖాళీ;

	ఒకవేళ (str && (str == opts || strpbrk(str - 1, stopchs) == str - 1) &&
			(strpbrk(చివర_2, stopchs) == చివర_2 || *చివర_2 == eqch ||
			 *చివర_2 == '\0')) {
		స్థిరరాశి అక్షరం *arg_చివర;

		ఒకవేళ (!arglen)
			తిరిగి str;

		ఒకవేళ (*చివర_2 != eqch)
			తిరిగి ఖాళీ;

		arg_చివర = strpbrk(str, stopchs);
		ఒకవేళ (!arg_చివర)
			*arglen = min(maxlen, అక్షరశ్రేణి_పొడవు(str)) - optlen - 1;
		లేకపోతే
			*arglen = arg_చివర - చివర_2 - 1;

		తిరిగి చివర_2 + 1;
	} లేకపోతే ఒకవేళ (str) {
		opts = చివర_2;
		వెళ్ళు తదుపరి;
	}
	తిరిగి ఖాళీ;
}

స్థిరరాశి అక్షరం cpu_hwconfig[] __attribute__((weak)) = "";
స్థిరరాశి అక్షరం board_hwconfig[] __attribute__((weak)) = "";

స్థిర స్థిరరాశి అక్షరం *__hwconfig(స్థిరరాశి అక్షరం *opt, size_t *arglen,
			      స్థిరరాశి అక్షరం *env_hwconfig)
{
	స్థిరరాశి అక్షరం *ret;

	/* if we are passed a buffer use it, otherwise try the environment */
	ఒకవేళ (!env_hwconfig) {
#ఒకవేళ CONFIG_IS_ENABLED(ENV_SUPPORT)
		ఒకవేళ (!(gd->flags & GD_FLG_ENV_READY) &&
		    gd->env_valid != ENV_VALID)
#లేకపోతే
		ఒకవేళ (true)
#అయితే_ముగింపు
		{
			రాయి("WARNING: Calling __hwconfig without a buffer "
					"and before environment is ready\n");
			తిరిగి ఖాళీ;
		}
#ఒకవేళ CONFIG_IS_ENABLED(ENV_SUPPORT)
		env_hwconfig = env_get("hwconfig");
#అయితే_ముగింపు
	}

	ఒకవేళ (env_hwconfig) {
		ret = hwconfig_parse(env_hwconfig, అక్షరశ్రేణి_పొడవు(env_hwconfig),
				      opt, ";", ':', arglen);
		ఒకవేళ (ret)
			తిరిగి ret;
	}

	ret = hwconfig_parse(board_hwconfig, అక్షరశ్రేణి_పొడవు(board_hwconfig),
			opt, ";", ':', arglen);
	ఒకవేళ (ret)
		తిరిగి ret;

	తిరిగి hwconfig_parse(cpu_hwconfig, అక్షరశ్రేణి_పొడవు(cpu_hwconfig),
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
పూర్ణాంకం hwconfig_f(స్థిరరాశి అక్షరం *opt, అక్షరం *buf)
{
	తిరిగి !!__hwconfig(opt, ఖాళీ, buf);
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
స్థిరరాశి అక్షరం *hwconfig_arg_f(స్థిరరాశి అక్షరం *opt, size_t *arglen, అక్షరం *buf)
{
	తిరిగి __hwconfig(opt, arglen, buf);
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
పూర్ణాంకం hwconfig_arg_cmp_f(స్థిరరాశి అక్షరం *opt, స్థిరరాశి అక్షరం *arg, అక్షరం *buf)
{
	స్థిరరాశి అక్షరం *argstr;
	size_t arglen;

	argstr = hwconfig_arg_f(opt, &arglen, buf);
	ఒకవేళ (!argstr || arglen != అక్షరశ్రేణి_పొడవు(arg))
		తిరిగి 0;

	తిరిగి !అక్షరశ్రేణి_పోలిక(argstr, arg, arglen);
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
పూర్ణాంకం hwconfig_sub_f(స్థిరరాశి అక్షరం *opt, స్థిరరాశి అక్షరం *subopt, అక్షరం *buf)
{
	size_t arglen;
	స్థిరరాశి అక్షరం *arg;

	arg = __hwconfig(opt, &arglen, buf);
	ఒకవేళ (!arg)
		తిరిగి 0;
	తిరిగి !!hwconfig_parse(arg, arglen, subopt, ",;", '=', ఖాళీ);
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
స్థిరరాశి అక్షరం *hwconfig_subarg_f(స్థిరరాశి అక్షరం *opt, స్థిరరాశి అక్షరం *subopt,
			      size_t *subarglen, అక్షరం *buf)
{
	size_t arglen;
	స్థిరరాశి అక్షరం *arg;

	arg = __hwconfig(opt, &arglen, buf);
	ఒకవేళ (!arg)
		తిరిగి ఖాళీ;
	తిరిగి hwconfig_parse(arg, arglen, subopt, ",;", '=', subarglen);
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
పూర్ణాంకం hwconfig_subarg_cmp_f(స్థిరరాశి అక్షరం *opt, స్థిరరాశి అక్షరం *subopt,
			  స్థిరరాశి అక్షరం *subarg, అక్షరం *buf)
{
	స్థిరరాశి అక్షరం *argstr;
	size_t arglen;

	argstr = hwconfig_subarg_f(opt, subopt, &arglen, buf);
	ఒకవేళ (!argstr || arglen != అక్షరశ్రేణి_పొడవు(subarg))
		తిరిగి 0;

	తిరిగి !అక్షరశ్రేణి_పోలిక(argstr, subarg, arglen);
}

#నిర్వచితమైతే HWCONFIG_TEST
పూర్ణాంకం ప్రధాన()
{
	స్థిరరాశి అక్షరం *ret;
	size_t len;

	env_సమితి("hwconfig", "key1:subkey1=value1,subkey2=value2;key2:value3;;;;"
			   "key3;:,:=;key4", 1);

	ret = hwconfig_arg("key1", &len);
	రాయి("%zd %.*s\n", len, (పూర్ణాంకం)len, ret);
	ప్రతిజ్ఞా(len == 29);
	ప్రతిజ్ఞా(hwconfig_arg_cmp("key1", "subkey1=value1,subkey2=value2"));
	ప్రతిజ్ఞా(!అక్షరశ్రేణి_పోలిక(ret, "subkey1=value1,subkey2=value2", len));

	ret = hwconfig_subarg("key1", "subkey1", &len);
	రాయి("%zd %.*s\n", len, (పూర్ణాంకం)len, ret);
	ప్రతిజ్ఞా(len == 6);
	ప్రతిజ్ఞా(hwconfig_subarg_cmp("key1", "subkey1", "value1"));
	ప్రతిజ్ఞా(!అక్షరశ్రేణి_పోలిక(ret, "value1", len));

	ret = hwconfig_subarg("key1", "subkey2", &len);
	రాయి("%zd %.*s\n", len, (పూర్ణాంకం)len, ret);
	ప్రతిజ్ఞా(len == 6);
	ప్రతిజ్ఞా(hwconfig_subarg_cmp("key1", "subkey2", "value2"));
	ప్రతిజ్ఞా(!అక్షరశ్రేణి_పోలిక(ret, "value2", len));

	ret = hwconfig_arg("key2", &len);
	రాయి("%zd %.*s\n", len, (పూర్ణాంకం)len, ret);
	ప్రతిజ్ఞా(len == 6);
	ప్రతిజ్ఞా(hwconfig_arg_cmp("key2", "value3"));
	ప్రతిజ్ఞా(!అక్షరశ్రేణి_పోలిక(ret, "value3", len));

	ప్రతిజ్ఞా(hwconfig("key3"));
	ప్రతిజ్ఞా(hwconfig_arg("key4", &len) == ఖాళీ);
	ప్రతిజ్ఞా(hwconfig_arg("bogus", &len) == ఖాళీ);

	unenv_సమితి("hwconfig");

	ప్రతిజ్ఞా(hwconfig(ఖాళీ) == 0);
	ప్రతిజ్ఞా(hwconfig("") == 0);
	ప్రతిజ్ఞా(hwconfig("key3") == 0);

	తిరిగి 0;
}
#అయితే_ముగింపు /* HWCONFIG_TEST */
