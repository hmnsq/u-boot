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

स्थिर स्थिरांक अक्षर *hwconfig_parse(स्थिरांक अक्षर *opts, माप_प्रकार maxlen,
				  स्थिरांक अक्षर *opt, अक्षर *stopchs, अक्षर eqch,
				  माप_प्रकार *arglen)
{
	माप_प्रकार optlen = म_माप(opt);
	अक्षर *str;
	स्थिरांक अक्षर *start = opts;
	स्थिरांक अक्षर *end;

next:
	str = म_माला(opts, opt);
	end = str + optlen;
	जर (end - start > maxlen)
		परत रिक्त;

	जर (str && (str == opts || strpbrk(str - 1, stopchs) == str - 1) &&
			(strpbrk(end, stopchs) == end || *end == eqch ||
			 *end == '\0')) {
		स्थिरांक अक्षर *arg_end;

		जर (!arglen)
			परत str;

		जर (*end != eqch)
			परत रिक्त;

		arg_end = strpbrk(str, stopchs);
		जर (!arg_end)
			*arglen = min(maxlen, म_माप(str)) - optlen - 1;
		अन्यथा
			*arglen = arg_end - end - 1;

		परत end + 1;
	} अन्यथा जर (str) {
		opts = end;
		जा next;
	}
	परत रिक्त;
}

स्थिरांक अक्षर cpu_hwconfig[] __attribute__((weak)) = "";
स्थिरांक अक्षर board_hwconfig[] __attribute__((weak)) = "";

स्थिर स्थिरांक अक्षर *__hwconfig(स्थिरांक अक्षर *opt, माप_प्रकार *arglen,
			      स्थिरांक अक्षर *env_hwconfig)
{
	स्थिरांक अक्षर *ret;

	/* if we are passed a buffer use it, otherwise try the environment */
	जर (!env_hwconfig) {
#जर CONFIG_IS_ENABLED(ENV_SUPPORT)
		जर (!(gd->flags & GD_FLG_ENV_READY) &&
		    gd->env_valid != ENV_VALID)
#अन्यथा
		जर (true)
#endif
		{
			म_लिखो("WARNING: Calling __hwconfig without a buffer "
					"and before environment is ready\n");
			परत रिक्त;
		}
#जर CONFIG_IS_ENABLED(ENV_SUPPORT)
		env_hwconfig = env_get("hwconfig");
#endif
	}

	जर (env_hwconfig) {
		ret = hwconfig_parse(env_hwconfig, म_माप(env_hwconfig),
				      opt, ";", ':', arglen);
		जर (ret)
			परत ret;
	}

	ret = hwconfig_parse(board_hwconfig, म_माप(board_hwconfig),
			opt, ";", ':', arglen);
	जर (ret)
		परत ret;

	परत hwconfig_parse(cpu_hwconfig, म_माप(cpu_hwconfig),
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
पूर्णांक hwconfig_f(स्थिरांक अक्षर *opt, अक्षर *buf)
{
	परत !!__hwconfig(opt, रिक्त, buf);
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
स्थिरांक अक्षर *hwconfig_arg_f(स्थिरांक अक्षर *opt, माप_प्रकार *arglen, अक्षर *buf)
{
	परत __hwconfig(opt, arglen, buf);
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
पूर्णांक hwconfig_arg_cmp_f(स्थिरांक अक्षर *opt, स्थिरांक अक्षर *arg, अक्षर *buf)
{
	स्थिरांक अक्षर *argstr;
	माप_प्रकार arglen;

	argstr = hwconfig_arg_f(opt, &arglen, buf);
	जर (!argstr || arglen != म_माप(arg))
		परत 0;

	परत !म_भेदन(argstr, arg, arglen);
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
पूर्णांक hwconfig_sub_f(स्थिरांक अक्षर *opt, स्थिरांक अक्षर *subopt, अक्षर *buf)
{
	माप_प्रकार arglen;
	स्थिरांक अक्षर *arg;

	arg = __hwconfig(opt, &arglen, buf);
	जर (!arg)
		परत 0;
	परत !!hwconfig_parse(arg, arglen, subopt, ",;", '=', रिक्त);
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
स्थिरांक अक्षर *hwconfig_subarg_f(स्थिरांक अक्षर *opt, स्थिरांक अक्षर *subopt,
			      माप_प्रकार *subarglen, अक्षर *buf)
{
	माप_प्रकार arglen;
	स्थिरांक अक्षर *arg;

	arg = __hwconfig(opt, &arglen, buf);
	जर (!arg)
		परत रिक्त;
	परत hwconfig_parse(arg, arglen, subopt, ",;", '=', subarglen);
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
पूर्णांक hwconfig_subarg_cmp_f(स्थिरांक अक्षर *opt, स्थिरांक अक्षर *subopt,
			  स्थिरांक अक्षर *subarg, अक्षर *buf)
{
	स्थिरांक अक्षर *argstr;
	माप_प्रकार arglen;

	argstr = hwconfig_subarg_f(opt, subopt, &arglen, buf);
	जर (!argstr || arglen != म_माप(subarg))
		परत 0;

	परत !म_भेदन(argstr, subarg, arglen);
}

#ifdef HWCONFIG_TEST
पूर्णांक main()
{
	स्थिरांक अक्षर *ret;
	माप_प्रकार len;

	env_set("hwconfig", "key1:subkey1=value1,subkey2=value2;key2:value3;;;;"
			   "key3;:,:=;key4", 1);

	ret = hwconfig_arg("key1", &len);
	म_लिखो("%zd %.*s\n", len, (पूर्णांक)len, ret);
	प्रतिज्ञा(len == 29);
	प्रतिज्ञा(hwconfig_arg_cmp("key1", "subkey1=value1,subkey2=value2"));
	प्रतिज्ञा(!म_भेदन(ret, "subkey1=value1,subkey2=value2", len));

	ret = hwconfig_subarg("key1", "subkey1", &len);
	म_लिखो("%zd %.*s\n", len, (पूर्णांक)len, ret);
	प्रतिज्ञा(len == 6);
	प्रतिज्ञा(hwconfig_subarg_cmp("key1", "subkey1", "value1"));
	प्रतिज्ञा(!म_भेदन(ret, "value1", len));

	ret = hwconfig_subarg("key1", "subkey2", &len);
	म_लिखो("%zd %.*s\n", len, (पूर्णांक)len, ret);
	प्रतिज्ञा(len == 6);
	प्रतिज्ञा(hwconfig_subarg_cmp("key1", "subkey2", "value2"));
	प्रतिज्ञा(!म_भेदन(ret, "value2", len));

	ret = hwconfig_arg("key2", &len);
	म_लिखो("%zd %.*s\n", len, (पूर्णांक)len, ret);
	प्रतिज्ञा(len == 6);
	प्रतिज्ञा(hwconfig_arg_cmp("key2", "value3"));
	प्रतिज्ञा(!म_भेदन(ret, "value3", len));

	प्रतिज्ञा(hwconfig("key3"));
	प्रतिज्ञा(hwconfig_arg("key4", &len) == रिक्त);
	प्रतिज्ञा(hwconfig_arg("bogus", &len) == रिक्त);

	unenv_set("hwconfig");

	प्रतिज्ञा(hwconfig(रिक्त) == 0);
	प्रतिज्ञा(hwconfig("") == 0);
	प्रतिज्ञा(hwconfig("key3") == 0);

	परत 0;
}
#endif /* HWCONFIG_TEST */
