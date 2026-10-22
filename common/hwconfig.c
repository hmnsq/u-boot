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
#அந்தர்பாவ <config.h>
#அந்தர்பாவ <env.h>
#அந்தர்பாவ <exports.h>
#அந்தர்பாவ <hwconfig.h>
#அந்தர்பாவ <log.h>
#அந்தர்பாவ <asm/global_data.h>
#அந்தர்பாவ <linux/types.h>
#அந்தர்பாவ <linux/string.h>
#இல்லையெனில்
#அந்தர்பாவ <stdio.h>
#அந்தர்பாவ <stdlib.h>
#அந்தர்பாவ <string.h>
#அந்தர்பாவ <assert.h>
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif /* HWCONFIG_TEST */

DECLARE_GLOBAL_DATA_PTR;

நிலையான மாறிலி எழுத்து *hwconfig_parse(மாறிலி எழுத்து *opts, size_t maxlen,
				  மாறிலி எழுத்து *opt, எழுத்து *stopchs, எழுத்து eqch,
				  size_t *arglen)
{
	size_t optlen = strlen(opt);
	எழுத்து *str;
	மாறிலி எழுத்து *start = opts;
	மாறிலி எழுத்து *end;

next:
	str = strstr(opts, opt);
	end = str + optlen;
	எனில் (end - start > maxlen)
		திரும்பு வெற்று;

	எனில் (str && (str == opts || strpbrk(str - 1, stopchs) == str - 1) &&
			(strpbrk(end, stopchs) == end || *end == eqch ||
			 *end == '\0')) {
		மாறிலி எழுத்து *arg_end;

		எனில் (!arglen)
			திரும்பு str;

		எனில் (*end != eqch)
			திரும்பு வெற்று;

		arg_end = strpbrk(str, stopchs);
		எனில் (!arg_end)
			*arglen = min(maxlen, strlen(str)) - optlen - 1;
		இல்லையெனில்
			*arglen = arg_end - end - 1;

		திரும்பு end + 1;
	} இல்லையெனில் எனில் (str) {
		opts = end;
		செல் next;
	}
	திரும்பு வெற்று;
}

மாறிலி எழுத்து cpu_hwconfig[] __attribute__((weak)) = "";
மாறிலி எழுத்து board_hwconfig[] __attribute__((weak)) = "";

நிலையான மாறிலி எழுத்து *__hwconfig(மாறிலி எழுத்து *opt, size_t *arglen,
			      மாறிலி எழுத்து *env_hwconfig)
{
	மாறிலி எழுத்து *ret;

	/* if we are passed a buffer use it, otherwise try the environment */
	எனில் (!env_hwconfig) {
#எனில் CONFIG_IS_ENABLED(ENV_SUPPORT)
		எனில் (!(gd->flags & GD_FLG_ENV_READY) &&
		    gd->env_valid != ENV_VALID)
#இல்லையெனில்
		எனில் (true)
#endif
		{
			printf("WARNING: Calling __hwconfig without a buffer "
					"and before environment is ready\n");
			திரும்பு வெற்று;
		}
#எனில் CONFIG_IS_ENABLED(ENV_SUPPORT)
		env_hwconfig = env_get("hwconfig");
#endif
	}

	எனில் (env_hwconfig) {
		ret = hwconfig_parse(env_hwconfig, strlen(env_hwconfig),
				      opt, ";", ':', arglen);
		எனில் (ret)
			திரும்பு ret;
	}

	ret = hwconfig_parse(board_hwconfig, strlen(board_hwconfig),
			opt, ";", ':', arglen);
	எனில் (ret)
		திரும்பு ret;

	திரும்பு hwconfig_parse(cpu_hwconfig, strlen(cpu_hwconfig),
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
முழு hwconfig_f(மாறிலி எழுத்து *opt, எழுத்து *buf)
{
	திரும்பு !!__hwconfig(opt, வெற்று, buf);
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
மாறிலி எழுத்து *hwconfig_arg_f(மாறிலி எழுத்து *opt, size_t *arglen, எழுத்து *buf)
{
	திரும்பு __hwconfig(opt, arglen, buf);
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
முழு hwconfig_arg_cmp_f(மாறிலி எழுத்து *opt, மாறிலி எழுத்து *arg, எழுத்து *buf)
{
	மாறிலி எழுத்து *argstr;
	size_t arglen;

	argstr = hwconfig_arg_f(opt, &arglen, buf);
	எனில் (!argstr || arglen != strlen(arg))
		திரும்பு 0;

	திரும்பு !strncmp(argstr, arg, arglen);
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
முழு hwconfig_sub_f(மாறிலி எழுத்து *opt, மாறிலி எழுத்து *subopt, எழுத்து *buf)
{
	size_t arglen;
	மாறிலி எழுத்து *arg;

	arg = __hwconfig(opt, &arglen, buf);
	எனில் (!arg)
		திரும்பு 0;
	திரும்பு !!hwconfig_parse(arg, arglen, subopt, ",;", '=', வெற்று);
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
மாறிலி எழுத்து *hwconfig_subarg_f(மாறிலி எழுத்து *opt, மாறிலி எழுத்து *subopt,
			      size_t *subarglen, எழுத்து *buf)
{
	size_t arglen;
	மாறிலி எழுத்து *arg;

	arg = __hwconfig(opt, &arglen, buf);
	எனில் (!arg)
		திரும்பு வெற்று;
	திரும்பு hwconfig_parse(arg, arglen, subopt, ",;", '=', subarglen);
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
முழு hwconfig_subarg_cmp_f(மாறிலி எழுத்து *opt, மாறிலி எழுத்து *subopt,
			  மாறிலி எழுத்து *subarg, எழுத்து *buf)
{
	மாறிலி எழுத்து *argstr;
	size_t arglen;

	argstr = hwconfig_subarg_f(opt, subopt, &arglen, buf);
	எனில் (!argstr || arglen != strlen(subarg))
		திரும்பு 0;

	திரும்பு !strncmp(argstr, subarg, arglen);
}

#ifdef HWCONFIG_TEST
முழு முதன்மை()
{
	மாறிலி எழுத்து *ret;
	size_t len;

	env_set("hwconfig", "key1:subkey1=value1,subkey2=value2;key2:value3;;;;"
			   "key3;:,:=;key4", 1);

	ret = hwconfig_arg("key1", &len);
	printf("%zd %.*s\n", len, (முழு)len, ret);
	ப்ரதிஜ்ஞா(len == 29);
	ப்ரதிஜ்ஞா(hwconfig_arg_cmp("key1", "subkey1=value1,subkey2=value2"));
	ப்ரதிஜ்ஞா(!strncmp(ret, "subkey1=value1,subkey2=value2", len));

	ret = hwconfig_subarg("key1", "subkey1", &len);
	printf("%zd %.*s\n", len, (முழு)len, ret);
	ப்ரதிஜ்ஞா(len == 6);
	ப்ரதிஜ்ஞா(hwconfig_subarg_cmp("key1", "subkey1", "value1"));
	ப்ரதிஜ்ஞா(!strncmp(ret, "value1", len));

	ret = hwconfig_subarg("key1", "subkey2", &len);
	printf("%zd %.*s\n", len, (முழு)len, ret);
	ப்ரதிஜ்ஞா(len == 6);
	ப்ரதிஜ்ஞா(hwconfig_subarg_cmp("key1", "subkey2", "value2"));
	ப்ரதிஜ்ஞா(!strncmp(ret, "value2", len));

	ret = hwconfig_arg("key2", &len);
	printf("%zd %.*s\n", len, (முழு)len, ret);
	ப்ரதிஜ்ஞா(len == 6);
	ப்ரதிஜ்ஞா(hwconfig_arg_cmp("key2", "value3"));
	ப்ரதிஜ்ஞா(!strncmp(ret, "value3", len));

	ப்ரதிஜ்ஞா(hwconfig("key3"));
	ப்ரதிஜ்ஞா(hwconfig_arg("key4", &len) == வெற்று);
	ப்ரதிஜ்ஞா(hwconfig_arg("bogus", &len) == வெற்று);

	unenv_set("hwconfig");

	ப்ரதிஜ்ஞா(hwconfig(வெற்று) == 0);
	ப்ரதிஜ்ஞா(hwconfig("") == 0);
	ப்ரதிஜ்ஞா(hwconfig("key3") == 0);

	திரும்பு 0;
}
#endif /* HWCONFIG_TEST */
