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
#अन्तर्भाव <config.h>
#अन्तर्भाव <env.h>
#अन्तर्भाव <exports.h>
#अन्तर्भाव <hwconfig.h>
#अन्तर्भाव <log.h>
#अन्तर्भाव <asm/global_data.h>
#अन्तर्भाव <linux/types.h>
#अन्तर्भाव <linux/string.h>
#अन्यथा
#अन्तर्भाव <stdio.h>
#अन्तर्भाव <stdlib.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <assert.h>
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif /* HWCONFIG_TEST */

DECLARE_GLOBAL_DATA_PTR;

स्थिर ध्रुव वर्ण *hwconfig_parse(ध्रुव वर्ण *opts, size_t maxlen,
				  ध्रुव वर्ण *opt, वर्ण *stopchs, वर्ण eqch,
				  size_t *arglen)
{
	size_t optlen = strlen(opt);
	वर्ण *str;
	ध्रुव वर्ण *start = opts;
	ध्रुव वर्ण *end;

next:
	str = strstr(opts, opt);
	end = str + optlen;
	यदि (end - start > maxlen)
		प्रत्यागम रिक्त;

	यदि (str && (str == opts || strpbrk(str - 1, stopchs) == str - 1) &&
			(strpbrk(end, stopchs) == end || *end == eqch ||
			 *end == '\0')) {
		ध्रुव वर्ण *arg_end;

		यदि (!arglen)
			प्रत्यागम str;

		यदि (*end != eqch)
			प्रत्यागम रिक्त;

		arg_end = strpbrk(str, stopchs);
		यदि (!arg_end)
			*arglen = min(maxlen, strlen(str)) - optlen - 1;
		अन्यथा
			*arglen = arg_end - end - 1;

		प्रत्यागम end + 1;
	} अन्यथा यदि (str) {
		opts = end;
		गच्छ next;
	}
	प्रत्यागम रिक्त;
}

ध्रुव वर्ण cpu_hwconfig[] __attribute__((weak)) = "";
ध्रुव वर्ण board_hwconfig[] __attribute__((weak)) = "";

स्थिर ध्रुव वर्ण *__hwconfig(ध्रुव वर्ण *opt, size_t *arglen,
			      ध्रुव वर्ण *env_hwconfig)
{
	ध्रुव वर्ण *ret;

	/* if we are passed a buffer use it, otherwise try the environment */
	यदि (!env_hwconfig) {
#यदि CONFIG_IS_ENABLED(ENV_SUPPORT)
		यदि (!(gd->flags & GD_FLG_ENV_READY) &&
		    gd->env_valid != ENV_VALID)
#अन्यथा
		यदि (true)
#endif
		{
			printf("WARNING: Calling __hwconfig without a buffer "
					"and before environment is ready\n");
			प्रत्यागम रिक्त;
		}
#यदि CONFIG_IS_ENABLED(ENV_SUPPORT)
		env_hwconfig = env_get("hwconfig");
#endif
	}

	यदि (env_hwconfig) {
		ret = hwconfig_parse(env_hwconfig, strlen(env_hwconfig),
				      opt, ";", ':', arglen);
		यदि (ret)
			प्रत्यागम ret;
	}

	ret = hwconfig_parse(board_hwconfig, strlen(board_hwconfig),
			opt, ";", ':', arglen);
	यदि (ret)
		प्रत्यागम ret;

	प्रत्यागम hwconfig_parse(cpu_hwconfig, strlen(cpu_hwconfig),
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
पूर्णाङ्क hwconfig_f(ध्रुव वर्ण *opt, वर्ण *buf)
{
	प्रत्यागम !!__hwconfig(opt, रिक्त, buf);
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
ध्रुव वर्ण *hwconfig_arg_f(ध्रुव वर्ण *opt, size_t *arglen, वर्ण *buf)
{
	प्रत्यागम __hwconfig(opt, arglen, buf);
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
पूर्णाङ्क hwconfig_arg_cmp_f(ध्रुव वर्ण *opt, ध्रुव वर्ण *arg, वर्ण *buf)
{
	ध्रुव वर्ण *argstr;
	size_t arglen;

	argstr = hwconfig_arg_f(opt, &arglen, buf);
	यदि (!argstr || arglen != strlen(arg))
		प्रत्यागम 0;

	प्रत्यागम !strncmp(argstr, arg, arglen);
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
पूर्णाङ्क hwconfig_sub_f(ध्रुव वर्ण *opt, ध्रुव वर्ण *subopt, वर्ण *buf)
{
	size_t arglen;
	ध्रुव वर्ण *arg;

	arg = __hwconfig(opt, &arglen, buf);
	यदि (!arg)
		प्रत्यागम 0;
	प्रत्यागम !!hwconfig_parse(arg, arglen, subopt, ",;", '=', रिक्त);
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
ध्रुव वर्ण *hwconfig_subarg_f(ध्रुव वर्ण *opt, ध्रुव वर्ण *subopt,
			      size_t *subarglen, वर्ण *buf)
{
	size_t arglen;
	ध्रुव वर्ण *arg;

	arg = __hwconfig(opt, &arglen, buf);
	यदि (!arg)
		प्रत्यागम रिक्त;
	प्रत्यागम hwconfig_parse(arg, arglen, subopt, ",;", '=', subarglen);
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
पूर्णाङ्क hwconfig_subarg_cmp_f(ध्रुव वर्ण *opt, ध्रुव वर्ण *subopt,
			  ध्रुव वर्ण *subarg, वर्ण *buf)
{
	ध्रुव वर्ण *argstr;
	size_t arglen;

	argstr = hwconfig_subarg_f(opt, subopt, &arglen, buf);
	यदि (!argstr || arglen != strlen(subarg))
		प्रत्यागम 0;

	प्रत्यागम !strncmp(argstr, subarg, arglen);
}

#ifdef HWCONFIG_TEST
पूर्णाङ्क main()
{
	ध्रुव वर्ण *ret;
	size_t len;

	env_set("hwconfig", "key1:subkey1=value1,subkey2=value2;key2:value3;;;;"
			   "key3;:,:=;key4", 1);

	ret = hwconfig_arg("key1", &len);
	printf("%zd %.*s\n", len, (पूर्णाङ्क)len, ret);
	प्रतिज्ञा(len == 29);
	प्रतिज्ञा(hwconfig_arg_cmp("key1", "subkey1=value1,subkey2=value2"));
	प्रतिज्ञा(!strncmp(ret, "subkey1=value1,subkey2=value2", len));

	ret = hwconfig_subarg("key1", "subkey1", &len);
	printf("%zd %.*s\n", len, (पूर्णाङ्क)len, ret);
	प्रतिज्ञा(len == 6);
	प्रतिज्ञा(hwconfig_subarg_cmp("key1", "subkey1", "value1"));
	प्रतिज्ञा(!strncmp(ret, "value1", len));

	ret = hwconfig_subarg("key1", "subkey2", &len);
	printf("%zd %.*s\n", len, (पूर्णाङ्क)len, ret);
	प्रतिज्ञा(len == 6);
	प्रतिज्ञा(hwconfig_subarg_cmp("key1", "subkey2", "value2"));
	प्रतिज्ञा(!strncmp(ret, "value2", len));

	ret = hwconfig_arg("key2", &len);
	printf("%zd %.*s\n", len, (पूर्णाङ्क)len, ret);
	प्रतिज्ञा(len == 6);
	प्रतिज्ञा(hwconfig_arg_cmp("key2", "value3"));
	प्रतिज्ञा(!strncmp(ret, "value3", len));

	प्रतिज्ञा(hwconfig("key3"));
	प्रतिज्ञा(hwconfig_arg("key4", &len) == रिक्त);
	प्रतिज्ञा(hwconfig_arg("bogus", &len) == रिक्त);

	unenv_set("hwconfig");

	प्रतिज्ञा(hwconfig(रिक्त) == 0);
	प्रतिज्ञा(hwconfig("") == 0);
	प्रतिज्ञा(hwconfig("key3") == 0);

	प्रत्यागम 0;
}
#endif /* HWCONFIG_TEST */
