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
#ଅନ୍ତର୍ଭାଵ <config.h>
#ଅନ୍ତର୍ଭାଵ <env.h>
#ଅନ୍ତର୍ଭାଵ <exports.h>
#ଅନ୍ତର୍ଭାଵ <hwconfig.h>
#ଅନ୍ତର୍ଭାଵ <log.h>
#ଅନ୍ତର୍ଭାଵ <asm/global_data.h>
#ଅନ୍ତର୍ଭାଵ <linux/types.h>
#ଅନ୍ତର୍ଭାଵ <linux/string.h>
#ଅନ୍ୟଥା
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <stdlib.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <assert.h>
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif /* HWCONFIG_TEST */

DECLARE_GLOBAL_DATA_PTR;

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *hwconfig_parse(ଅଚଳ ଅକ୍ଷର *opts, size_t maxlen,
				  ଅଚଳ ଅକ୍ଷର *opt, ଅକ୍ଷର *stopchs, ଅକ୍ଷର eqch,
				  size_t *arglen)
{
	size_t optlen = strlen(opt);
	ଅକ୍ଷର *str;
	ଅଚଳ ଅକ୍ଷର *start = opts;
	ଅଚଳ ଅକ୍ଷର *end;

next:
	str = strstr(opts, opt);
	end = str + optlen;
	ଯଦି (end - start > maxlen)
		ଫେରନ୍ତୁ ଖାଲି;

	ଯଦି (str && (str == opts || strpbrk(str - 1, stopchs) == str - 1) &&
			(strpbrk(end, stopchs) == end || *end == eqch ||
			 *end == '\0')) {
		ଅଚଳ ଅକ୍ଷର *arg_end;

		ଯଦି (!arglen)
			ଫେରନ୍ତୁ str;

		ଯଦି (*end != eqch)
			ଫେରନ୍ତୁ ଖାଲି;

		arg_end = strpbrk(str, stopchs);
		ଯଦି (!arg_end)
			*arglen = min(maxlen, strlen(str)) - optlen - 1;
		ଅନ୍ୟଥା
			*arglen = arg_end - end - 1;

		ଫେରନ୍ତୁ end + 1;
	} ଅନ୍ୟଥା ଯଦି (str) {
		opts = end;
		ଯାଅ next;
	}
	ଫେରନ୍ତୁ ଖାଲି;
}

ଅଚଳ ଅକ୍ଷର cpu_hwconfig[] __attribute__((weak)) = "";
ଅଚଳ ଅକ୍ଷର board_hwconfig[] __attribute__((weak)) = "";

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *__hwconfig(ଅଚଳ ଅକ୍ଷର *opt, size_t *arglen,
			      ଅଚଳ ଅକ୍ଷର *env_hwconfig)
{
	ଅଚଳ ଅକ୍ଷର *ret;

	/* if we are passed a buffer use it, otherwise try the environment */
	ଯଦି (!env_hwconfig) {
#ଯଦି CONFIG_IS_ENABLED(ENV_SUPPORT)
		ଯଦି (!(gd->flags & GD_FLG_ENV_READY) &&
		    gd->env_valid != ENV_VALID)
#ଅନ୍ୟଥା
		ଯଦି (true)
#endif
		{
			printf("WARNING: Calling __hwconfig without a buffer "
					"and before environment is ready\n");
			ଫେରନ୍ତୁ ଖାଲି;
		}
#ଯଦି CONFIG_IS_ENABLED(ENV_SUPPORT)
		env_hwconfig = env_get("hwconfig");
#endif
	}

	ଯଦି (env_hwconfig) {
		ret = hwconfig_parse(env_hwconfig, strlen(env_hwconfig),
				      opt, ";", ':', arglen);
		ଯଦି (ret)
			ଫେରନ୍ତୁ ret;
	}

	ret = hwconfig_parse(board_hwconfig, strlen(board_hwconfig),
			opt, ";", ':', arglen);
	ଯଦି (ret)
		ଫେରନ୍ତୁ ret;

	ଫେରନ୍ତୁ hwconfig_parse(cpu_hwconfig, strlen(cpu_hwconfig),
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା hwconfig_f(ଅଚଳ ଅକ୍ଷର *opt, ଅକ୍ଷର *buf)
{
	ଫେରନ୍ତୁ !!__hwconfig(opt, ଖାଲି, buf);
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
ଅଚଳ ଅକ୍ଷର *hwconfig_arg_f(ଅଚଳ ଅକ୍ଷର *opt, size_t *arglen, ଅକ୍ଷର *buf)
{
	ଫେରନ୍ତୁ __hwconfig(opt, arglen, buf);
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା hwconfig_arg_cmp_f(ଅଚଳ ଅକ୍ଷର *opt, ଅଚଳ ଅକ୍ଷର *arg, ଅକ୍ଷର *buf)
{
	ଅଚଳ ଅକ୍ଷର *argstr;
	size_t arglen;

	argstr = hwconfig_arg_f(opt, &arglen, buf);
	ଯଦି (!argstr || arglen != strlen(arg))
		ଫେରନ୍ତୁ 0;

	ଫେରନ୍ତୁ !strncmp(argstr, arg, arglen);
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା hwconfig_sub_f(ଅଚଳ ଅକ୍ଷର *opt, ଅଚଳ ଅକ୍ଷର *subopt, ଅକ୍ଷର *buf)
{
	size_t arglen;
	ଅଚଳ ଅକ୍ଷର *arg;

	arg = __hwconfig(opt, &arglen, buf);
	ଯଦି (!arg)
		ଫେରନ୍ତୁ 0;
	ଫେରନ୍ତୁ !!hwconfig_parse(arg, arglen, subopt, ",;", '=', ଖାଲି);
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
ଅଚଳ ଅକ୍ଷର *hwconfig_subarg_f(ଅଚଳ ଅକ୍ଷର *opt, ଅଚଳ ଅକ୍ଷର *subopt,
			      size_t *subarglen, ଅକ୍ଷର *buf)
{
	size_t arglen;
	ଅଚଳ ଅକ୍ଷର *arg;

	arg = __hwconfig(opt, &arglen, buf);
	ଯଦି (!arg)
		ଫେରନ୍ତୁ ଖାଲି;
	ଫେରନ୍ତୁ hwconfig_parse(arg, arglen, subopt, ",;", '=', subarglen);
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
ପୂର୍ଣ୍ଣସଂଖ୍ୟା hwconfig_subarg_cmp_f(ଅଚଳ ଅକ୍ଷର *opt, ଅଚଳ ଅକ୍ଷର *subopt,
			  ଅଚଳ ଅକ୍ଷର *subarg, ଅକ୍ଷର *buf)
{
	ଅଚଳ ଅକ୍ଷର *argstr;
	size_t arglen;

	argstr = hwconfig_subarg_f(opt, subopt, &arglen, buf);
	ଯଦି (!argstr || arglen != strlen(subarg))
		ଫେରନ୍ତୁ 0;

	ଫେରନ୍ତୁ !strncmp(argstr, subarg, arglen);
}

#ifdef HWCONFIG_TEST
ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୁଖ୍ୟ()
{
	ଅଚଳ ଅକ୍ଷର *ret;
	size_t len;

	env_set("hwconfig", "key1:subkey1=value1,subkey2=value2;key2:value3;;;;"
			   "key3;:,:=;key4", 1);

	ret = hwconfig_arg("key1", &len);
	printf("%zd %.*s\n", len, (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)len, ret);
	ପ୍ରତିଜ୍ଞା(len == 29);
	ପ୍ରତିଜ୍ଞା(hwconfig_arg_cmp("key1", "subkey1=value1,subkey2=value2"));
	ପ୍ରତିଜ୍ଞା(!strncmp(ret, "subkey1=value1,subkey2=value2", len));

	ret = hwconfig_subarg("key1", "subkey1", &len);
	printf("%zd %.*s\n", len, (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)len, ret);
	ପ୍ରତିଜ୍ଞା(len == 6);
	ପ୍ରତିଜ୍ଞା(hwconfig_subarg_cmp("key1", "subkey1", "value1"));
	ପ୍ରତିଜ୍ଞା(!strncmp(ret, "value1", len));

	ret = hwconfig_subarg("key1", "subkey2", &len);
	printf("%zd %.*s\n", len, (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)len, ret);
	ପ୍ରତିଜ୍ଞା(len == 6);
	ପ୍ରତିଜ୍ଞା(hwconfig_subarg_cmp("key1", "subkey2", "value2"));
	ପ୍ରତିଜ୍ଞା(!strncmp(ret, "value2", len));

	ret = hwconfig_arg("key2", &len);
	printf("%zd %.*s\n", len, (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)len, ret);
	ପ୍ରତିଜ୍ଞା(len == 6);
	ପ୍ରତିଜ୍ଞା(hwconfig_arg_cmp("key2", "value3"));
	ପ୍ରତିଜ୍ଞା(!strncmp(ret, "value3", len));

	ପ୍ରତିଜ୍ଞା(hwconfig("key3"));
	ପ୍ରତିଜ୍ଞା(hwconfig_arg("key4", &len) == ଖାଲି);
	ପ୍ରତିଜ୍ଞା(hwconfig_arg("bogus", &len) == ଖାଲି);

	unenv_set("hwconfig");

	ପ୍ରତିଜ୍ଞା(hwconfig(ଖାଲି) == 0);
	ପ୍ରତିଜ୍ଞା(hwconfig("") == 0);
	ପ୍ରତିଜ୍ଞା(hwconfig("key3") == 0);

	ଫେରନ୍ତୁ 0;
}
#endif /* HWCONFIG_TEST */
