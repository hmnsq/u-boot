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
#അന്തര്ഭാവ <config.h>
#അന്തര്ഭാവ <env.h>
#അന്തര്ഭാവ <exports.h>
#അന്തര്ഭാവ <hwconfig.h>
#അന്തര്ഭാവ <log.h>
#അന്തര്ഭാവ <asm/global_data.h>
#അന്തര്ഭാവ <linux/types.h>
#അന്തര്ഭാവ <linux/string.h>
#അല്ലെങ്കിൽ
#അന്തര്ഭാവ <stdio.h>
#അന്തര്ഭാവ <stdlib.h>
#അന്തര്ഭാവ <string.h>
#അന്തര്ഭാവ <assert.h>
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif /* HWCONFIG_TEST */

DECLARE_GLOBAL_DATA_PTR;

സ്ഥിതം സ്ഥിരം അക്ഷരം *hwconfig_parse(സ്ഥിരം അക്ഷരം *opts, size_t maxlen,
				  സ്ഥിരം അക്ഷരം *opt, അക്ഷരം *stopchs, അക്ഷരം eqch,
				  size_t *arglen)
{
	size_t optlen = strlen(opt);
	അക്ഷരം *str;
	സ്ഥിരം അക്ഷരം *start = opts;
	സ്ഥിരം അക്ഷരം *end;

next:
	str = strstr(opts, opt);
	end = str + optlen;
	എങ്കിൽ (end - start > maxlen)
		തിരികെ ശൂന്യ;

	എങ്കിൽ (str && (str == opts || strpbrk(str - 1, stopchs) == str - 1) &&
			(strpbrk(end, stopchs) == end || *end == eqch ||
			 *end == '\0')) {
		സ്ഥിരം അക്ഷരം *arg_end;

		എങ്കിൽ (!arglen)
			തിരികെ str;

		എങ്കിൽ (*end != eqch)
			തിരികെ ശൂന്യ;

		arg_end = strpbrk(str, stopchs);
		എങ്കിൽ (!arg_end)
			*arglen = min(maxlen, strlen(str)) - optlen - 1;
		അല്ലെങ്കിൽ
			*arglen = arg_end - end - 1;

		തിരികെ end + 1;
	} അല്ലെങ്കിൽ എങ്കിൽ (str) {
		opts = end;
		പോകുക next;
	}
	തിരികെ ശൂന്യ;
}

സ്ഥിരം അക്ഷരം cpu_hwconfig[] __attribute__((weak)) = "";
സ്ഥിരം അക്ഷരം board_hwconfig[] __attribute__((weak)) = "";

സ്ഥിതം സ്ഥിരം അക്ഷരം *__hwconfig(സ്ഥിരം അക്ഷരം *opt, size_t *arglen,
			      സ്ഥിരം അക്ഷരം *env_hwconfig)
{
	സ്ഥിരം അക്ഷരം *ret;

	/* if we are passed a buffer use it, otherwise try the environment */
	എങ്കിൽ (!env_hwconfig) {
#എങ്കിൽ CONFIG_IS_ENABLED(ENV_SUPPORT)
		എങ്കിൽ (!(gd->flags & GD_FLG_ENV_READY) &&
		    gd->env_valid != ENV_VALID)
#അല്ലെങ്കിൽ
		എങ്കിൽ (true)
#endif
		{
			printf("WARNING: Calling __hwconfig without a buffer "
					"and before environment is ready\n");
			തിരികെ ശൂന്യ;
		}
#എങ്കിൽ CONFIG_IS_ENABLED(ENV_SUPPORT)
		env_hwconfig = env_get("hwconfig");
#endif
	}

	എങ്കിൽ (env_hwconfig) {
		ret = hwconfig_parse(env_hwconfig, strlen(env_hwconfig),
				      opt, ";", ':', arglen);
		എങ്കിൽ (ret)
			തിരികെ ret;
	}

	ret = hwconfig_parse(board_hwconfig, strlen(board_hwconfig),
			opt, ";", ':', arglen);
	എങ്കിൽ (ret)
		തിരികെ ret;

	തിരികെ hwconfig_parse(cpu_hwconfig, strlen(cpu_hwconfig),
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
പൂർണ്ണസംഖ്യ hwconfig_f(സ്ഥിരം അക്ഷരം *opt, അക്ഷരം *buf)
{
	തിരികെ !!__hwconfig(opt, ശൂന്യ, buf);
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
സ്ഥിരം അക്ഷരം *hwconfig_arg_f(സ്ഥിരം അക്ഷരം *opt, size_t *arglen, അക്ഷരം *buf)
{
	തിരികെ __hwconfig(opt, arglen, buf);
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
പൂർണ്ണസംഖ്യ hwconfig_arg_cmp_f(സ്ഥിരം അക്ഷരം *opt, സ്ഥിരം അക്ഷരം *arg, അക്ഷരം *buf)
{
	സ്ഥിരം അക്ഷരം *argstr;
	size_t arglen;

	argstr = hwconfig_arg_f(opt, &arglen, buf);
	എങ്കിൽ (!argstr || arglen != strlen(arg))
		തിരികെ 0;

	തിരികെ !strncmp(argstr, arg, arglen);
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
പൂർണ്ണസംഖ്യ hwconfig_sub_f(സ്ഥിരം അക്ഷരം *opt, സ്ഥിരം അക്ഷരം *subopt, അക്ഷരം *buf)
{
	size_t arglen;
	സ്ഥിരം അക്ഷരം *arg;

	arg = __hwconfig(opt, &arglen, buf);
	എങ്കിൽ (!arg)
		തിരികെ 0;
	തിരികെ !!hwconfig_parse(arg, arglen, subopt, ",;", '=', ശൂന്യ);
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
സ്ഥിരം അക്ഷരം *hwconfig_subarg_f(സ്ഥിരം അക്ഷരം *opt, സ്ഥിരം അക്ഷരം *subopt,
			      size_t *subarglen, അക്ഷരം *buf)
{
	size_t arglen;
	സ്ഥിരം അക്ഷരം *arg;

	arg = __hwconfig(opt, &arglen, buf);
	എങ്കിൽ (!arg)
		തിരികെ ശൂന്യ;
	തിരികെ hwconfig_parse(arg, arglen, subopt, ",;", '=', subarglen);
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
പൂർണ്ണസംഖ്യ hwconfig_subarg_cmp_f(സ്ഥിരം അക്ഷരം *opt, സ്ഥിരം അക്ഷരം *subopt,
			  സ്ഥിരം അക്ഷരം *subarg, അക്ഷരം *buf)
{
	സ്ഥിരം അക്ഷരം *argstr;
	size_t arglen;

	argstr = hwconfig_subarg_f(opt, subopt, &arglen, buf);
	എങ്കിൽ (!argstr || arglen != strlen(subarg))
		തിരികെ 0;

	തിരികെ !strncmp(argstr, subarg, arglen);
}

#ifdef HWCONFIG_TEST
പൂർണ്ണസംഖ്യ പ്രധാന()
{
	സ്ഥിരം അക്ഷരം *ret;
	size_t len;

	env_set("hwconfig", "key1:subkey1=value1,subkey2=value2;key2:value3;;;;"
			   "key3;:,:=;key4", 1);

	ret = hwconfig_arg("key1", &len);
	printf("%zd %.*s\n", len, (പൂർണ്ണസംഖ്യ)len, ret);
	പ്രതിജ്ഞാ(len == 29);
	പ്രതിജ്ഞാ(hwconfig_arg_cmp("key1", "subkey1=value1,subkey2=value2"));
	പ്രതിജ്ഞാ(!strncmp(ret, "subkey1=value1,subkey2=value2", len));

	ret = hwconfig_subarg("key1", "subkey1", &len);
	printf("%zd %.*s\n", len, (പൂർണ്ണസംഖ്യ)len, ret);
	പ്രതിജ്ഞാ(len == 6);
	പ്രതിജ്ഞാ(hwconfig_subarg_cmp("key1", "subkey1", "value1"));
	പ്രതിജ്ഞാ(!strncmp(ret, "value1", len));

	ret = hwconfig_subarg("key1", "subkey2", &len);
	printf("%zd %.*s\n", len, (പൂർണ്ണസംഖ്യ)len, ret);
	പ്രതിജ്ഞാ(len == 6);
	പ്രതിജ്ഞാ(hwconfig_subarg_cmp("key1", "subkey2", "value2"));
	പ്രതിജ്ഞാ(!strncmp(ret, "value2", len));

	ret = hwconfig_arg("key2", &len);
	printf("%zd %.*s\n", len, (പൂർണ്ണസംഖ്യ)len, ret);
	പ്രതിജ്ഞാ(len == 6);
	പ്രതിജ്ഞാ(hwconfig_arg_cmp("key2", "value3"));
	പ്രതിജ്ഞാ(!strncmp(ret, "value3", len));

	പ്രതിജ്ഞാ(hwconfig("key3"));
	പ്രതിജ്ഞാ(hwconfig_arg("key4", &len) == ശൂന്യ);
	പ്രതിജ്ഞാ(hwconfig_arg("bogus", &len) == ശൂന്യ);

	unenv_set("hwconfig");

	പ്രതിജ്ഞാ(hwconfig(ശൂന്യ) == 0);
	പ്രതിജ്ഞാ(hwconfig("") == 0);
	പ്രതിജ്ഞാ(hwconfig("key3") == 0);

	തിരികെ 0;
}
#endif /* HWCONFIG_TEST */
