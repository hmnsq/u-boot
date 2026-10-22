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
#include <config.h>
#include <env.h>
#include <exports.h>
#include <hwconfig.h>
#include <log.h>
#include <asm/global_data.h>
#include <linux/types.h>
#include <linux/string.h>
#иначе
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif /* HWCONFIG_TEST */

DECLARE_GLOBAL_DATA_PTR;

static const char *hwconfig_parse(const char *opts, size_t maxlen,
				  const char *opt, char *stopchs, char eqch,
				  size_t *arglen)
{
	size_t optlen = strlen(opt);
	char *str;
	const char *start = opts;
	const char *end;

next:
	str = strstr(opts, opt);
	end = str + optlen;
	ако (end - start > maxlen)
		върни NULL;

	ако (str && (str == opts || strpbrk(str - 1, stopchs) == str - 1) &&
			(strpbrk(end, stopchs) == end || *end == eqch ||
			 *end == '\0')) {
		const char *arg_end;

		ако (!arglen)
			върни str;

		ако (*end != eqch)
			върни NULL;

		arg_end = strpbrk(str, stopchs);
		ако (!arg_end)
			*arglen = min(maxlen, strlen(str)) - optlen - 1;
		иначе
			*arglen = arg_end - end - 1;

		върни end + 1;
	} иначе ако (str) {
		opts = end;
		goto next;
	}
	върни NULL;
}

const char cpu_hwconfig[] __attribute__((weak)) = "";
const char board_hwconfig[] __attribute__((weak)) = "";

static const char *__hwconfig(const char *opt, size_t *arglen,
			      const char *env_hwconfig)
{
	const char *ret;

	/* if we are passed a buffer use it, otherwise try the environment */
	ако (!env_hwconfig) {
#ако CONFIG_IS_ENABLED(ENV_SUPPORT)
		ако (!(gd->flags & GD_FLG_ENV_READY) &&
		    gd->env_valid != ENV_VALID)
#иначе
		ако (true)
#endif
		{
			printf("WARNING: Calling __hwconfig without a buffer "
					"and before environment is ready\n");
			върни NULL;
		}
#ако CONFIG_IS_ENABLED(ENV_SUPPORT)
		env_hwconfig = env_get("hwconfig");
#endif
	}

	ако (env_hwconfig) {
		ret = hwconfig_parse(env_hwconfig, strlen(env_hwconfig),
				      opt, ";", ':', arglen);
		ако (ret)
			върни ret;
	}

	ret = hwconfig_parse(board_hwconfig, strlen(board_hwconfig),
			opt, ";", ':', arglen);
	ако (ret)
		върни ret;

	върни hwconfig_parse(cpu_hwconfig, strlen(cpu_hwconfig),
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
цяло hwconfig_f(const char *opt, char *buf)
{
	върни !!__hwconfig(opt, NULL, buf);
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
const char *hwconfig_arg_f(const char *opt, size_t *arglen, char *buf)
{
	върни __hwconfig(opt, arglen, buf);
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
цяло hwconfig_arg_cmp_f(const char *opt, const char *arg, char *buf)
{
	const char *argstr;
	size_t arglen;

	argstr = hwconfig_arg_f(opt, &arglen, buf);
	ако (!argstr || arglen != strlen(arg))
		върни 0;

	върни !strncmp(argstr, arg, arglen);
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
цяло hwconfig_sub_f(const char *opt, const char *subopt, char *buf)
{
	size_t arglen;
	const char *arg;

	arg = __hwconfig(opt, &arglen, buf);
	ако (!arg)
		върни 0;
	върни !!hwconfig_parse(arg, arglen, subopt, ",;", '=', NULL);
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
const char *hwconfig_subarg_f(const char *opt, const char *subopt,
			      size_t *subarglen, char *buf)
{
	size_t arglen;
	const char *arg;

	arg = __hwconfig(opt, &arglen, buf);
	ако (!arg)
		върни NULL;
	върни hwconfig_parse(arg, arglen, subopt, ",;", '=', subarglen);
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
цяло hwconfig_subarg_cmp_f(const char *opt, const char *subopt,
			  const char *subarg, char *buf)
{
	const char *argstr;
	size_t arglen;

	argstr = hwconfig_subarg_f(opt, subopt, &arglen, buf);
	ако (!argstr || arglen != strlen(subarg))
		върни 0;

	върни !strncmp(argstr, subarg, arglen);
}

#ifdef HWCONFIG_TEST
цяло main()
{
	const char *ret;
	size_t len;

	env_set("hwconfig", "key1:subkey1=value1,subkey2=value2;key2:value3;;;;"
			   "key3;:,:=;key4", 1);

	ret = hwconfig_arg("key1", &len);
	printf("%zd %.*s\n", len, (цяло)len, ret);
	assert(len == 29);
	assert(hwconfig_arg_cmp("key1", "subkey1=value1,subkey2=value2"));
	assert(!strncmp(ret, "subkey1=value1,subkey2=value2", len));

	ret = hwconfig_subarg("key1", "subkey1", &len);
	printf("%zd %.*s\n", len, (цяло)len, ret);
	assert(len == 6);
	assert(hwconfig_subarg_cmp("key1", "subkey1", "value1"));
	assert(!strncmp(ret, "value1", len));

	ret = hwconfig_subarg("key1", "subkey2", &len);
	printf("%zd %.*s\n", len, (цяло)len, ret);
	assert(len == 6);
	assert(hwconfig_subarg_cmp("key1", "subkey2", "value2"));
	assert(!strncmp(ret, "value2", len));

	ret = hwconfig_arg("key2", &len);
	printf("%zd %.*s\n", len, (цяло)len, ret);
	assert(len == 6);
	assert(hwconfig_arg_cmp("key2", "value3"));
	assert(!strncmp(ret, "value3", len));

	assert(hwconfig("key3"));
	assert(hwconfig_arg("key4", &len) == NULL);
	assert(hwconfig_arg("bogus", &len) == NULL);

	unenv_set("hwconfig");

	assert(hwconfig(NULL) == 0);
	assert(hwconfig("") == 0);
	assert(hwconfig("key3") == 0);

	върни 0;
}
#endif /* HWCONFIG_TEST */
