// SPDX-License-Identifier: GPL-2.0+
/*
 * An inteface for configuring a hardware via u-boot environment.
 *
 * Copyright (c) 2009  MontaVista Software, Inc.
 * Copyright 2011 Freescale Semiconductor, Inc.
 *
 * Author: Anton Vorontsov <avorontsov@ru.mvista.com>
 */

#अगर_अघोषित HWCONFIG_TEST
#समावेश <config.h>
#समावेश <env.h>
#समावेश <exports.h>
#समावेश <hwconfig.h>
#समावेश <log.h>
#समावेश <asm/global_data.h>
#समावेश <linux/types.h>
#समावेश <linux/string.h>
#अन्यथा
#समावेश <stdio.h>
#समावेश <stdlib.h>
#समावेश <string.h>
#समावेश <assert.h>
#घोषणा min(a, b) (((a) < (b)) ? (a) : (b))
#पूर्ण_अगर /* HWCONFIG_TEST */

DECLARE_GLOBAL_तथ्य_PTR;

अटल स्थिर अक्षर *hwconfig_parse(स्थिर अक्षर *opts, माप_प्रकार maxlen,
				  स्थिर अक्षर *opt, अक्षर *stopchs, अक्षर eqch,
				  माप_प्रकार *arglen)
{
	माप_प्रकार optlen = म_माप(opt);
	अक्षर *str;
	स्थिर अक्षर *start = opts;
	स्थिर अक्षर *end;

अग्र:
	str = म_माला(opts, opt);
	end = str + optlen;
	अगर (end - start > maxlen)
		वापस शून्य;

	अगर (str && (str == opts || strpbrk(str - 1, stopchs) == str - 1) &&
			(strpbrk(end, stopchs) == end || *end == eqch ||
			 *end == '\0')) {
		स्थिर अक्षर *arg_end;

		अगर (!arglen)
			वापस str;

		अगर (*end != eqch)
			वापस शून्य;

		arg_end = strpbrk(str, stopchs);
		अगर (!arg_end)
			*arglen = min(maxlen, म_माप(str)) - optlen - 1;
		अन्यथा
			*arglen = arg_end - end - 1;

		वापस end + 1;
	} अन्यथा अगर (str) {
		opts = end;
		जाओ अग्र;
	}
	वापस शून्य;
}

स्थिर अक्षर cpu_hwconfig[] __attribute__((weak)) = "";
स्थिर अक्षर board_hwconfig[] __attribute__((weak)) = "";

अटल स्थिर अक्षर *__hwconfig(स्थिर अक्षर *opt, माप_प्रकार *arglen,
			      स्थिर अक्षर *env_hwconfig)
{
	स्थिर अक्षर *ret;

	/* if we are passed a buffer use it, otherwise try the environment */
	अगर (!env_hwconfig) {
#अगर CONFIG_IS_ENABLED(ENV_SUPPORT)
		अगर (!(gd->flags & GD_FLG_ENV_READY) &&
		    gd->env_valid != ENV_VALID)
#अन्यथा
		अगर (true)
#पूर्ण_अगर
		{
			म_लिखो("WARNING: Calling __hwconfig without a buffer "
					"and before environment is ready\n");
			वापस शून्य;
		}
#अगर CONFIG_IS_ENABLED(ENV_SUPPORT)
		env_hwconfig = env_get("hwconfig");
#पूर्ण_अगर
	}

	अगर (env_hwconfig) {
		ret = hwconfig_parse(env_hwconfig, म_माप(env_hwconfig),
				      opt, ";", ':', arglen);
		अगर (ret)
			वापस ret;
	}

	ret = hwconfig_parse(board_hwconfig, म_माप(board_hwconfig),
			opt, ";", ':', arglen);
	अगर (ret)
		वापस ret;

	वापस hwconfig_parse(cpu_hwconfig, म_माप(cpu_hwconfig),
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
पूर्णांक hwconfig_f(स्थिर अक्षर *opt, अक्षर *buf)
{
	वापस !!__hwconfig(opt, शून्य, buf);
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
स्थिर अक्षर *hwconfig_arg_f(स्थिर अक्षर *opt, माप_प्रकार *arglen, अक्षर *buf)
{
	वापस __hwconfig(opt, arglen, buf);
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
पूर्णांक hwconfig_arg_cmp_f(स्थिर अक्षर *opt, स्थिर अक्षर *arg, अक्षर *buf)
{
	स्थिर अक्षर *argstr;
	माप_प्रकार arglen;

	argstr = hwconfig_arg_f(opt, &arglen, buf);
	अगर (!argstr || arglen != म_माप(arg))
		वापस 0;

	वापस !म_भेदन(argstr, arg, arglen);
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
पूर्णांक hwconfig_sub_f(स्थिर अक्षर *opt, स्थिर अक्षर *subopt, अक्षर *buf)
{
	माप_प्रकार arglen;
	स्थिर अक्षर *arg;

	arg = __hwconfig(opt, &arglen, buf);
	अगर (!arg)
		वापस 0;
	वापस !!hwconfig_parse(arg, arglen, subopt, ",;", '=', शून्य);
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
स्थिर अक्षर *hwconfig_subarg_f(स्थिर अक्षर *opt, स्थिर अक्षर *subopt,
			      माप_प्रकार *subarglen, अक्षर *buf)
{
	माप_प्रकार arglen;
	स्थिर अक्षर *arg;

	arg = __hwconfig(opt, &arglen, buf);
	अगर (!arg)
		वापस शून्य;
	वापस hwconfig_parse(arg, arglen, subopt, ",;", '=', subarglen);
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
पूर्णांक hwconfig_subarg_cmp_f(स्थिर अक्षर *opt, स्थिर अक्षर *subopt,
			  स्थिर अक्षर *subarg, अक्षर *buf)
{
	स्थिर अक्षर *argstr;
	माप_प्रकार arglen;

	argstr = hwconfig_subarg_f(opt, subopt, &arglen, buf);
	अगर (!argstr || arglen != म_माप(subarg))
		वापस 0;

	वापस !म_भेदन(argstr, subarg, arglen);
}

#अगर_घोषित HWCONFIG_TEST
पूर्णांक मुख्य()
{
	स्थिर अक्षर *ret;
	माप_प्रकार len;

	env_set("hwconfig", "key1:subkey1=value1,subkey2=value2;key2:value3;;;;"
			   "key3;:,:=;key4", 1);

	ret = hwconfig_arg("key1", &len);
	म_लिखो("%zd %.*s\n", len, (पूर्णांक)len, ret);
	निश्चित(len == 29);
	निश्चित(hwconfig_arg_cmp("key1", "subkey1=value1,subkey2=value2"));
	निश्चित(!म_भेदन(ret, "subkey1=value1,subkey2=value2", len));

	ret = hwconfig_subarg("key1", "subkey1", &len);
	म_लिखो("%zd %.*s\n", len, (पूर्णांक)len, ret);
	निश्चित(len == 6);
	निश्चित(hwconfig_subarg_cmp("key1", "subkey1", "value1"));
	निश्चित(!म_भेदन(ret, "value1", len));

	ret = hwconfig_subarg("key1", "subkey2", &len);
	म_लिखो("%zd %.*s\n", len, (पूर्णांक)len, ret);
	निश्चित(len == 6);
	निश्चित(hwconfig_subarg_cmp("key1", "subkey2", "value2"));
	निश्चित(!म_भेदन(ret, "value2", len));

	ret = hwconfig_arg("key2", &len);
	म_लिखो("%zd %.*s\n", len, (पूर्णांक)len, ret);
	निश्चित(len == 6);
	निश्चित(hwconfig_arg_cmp("key2", "value3"));
	निश्चित(!म_भेदन(ret, "value3", len));

	निश्चित(hwconfig("key3"));
	निश्चित(hwconfig_arg("key4", &len) == शून्य);
	निश्चित(hwconfig_arg("bogus", &len) == शून्य);

	unenv_set("hwconfig");

	निश्चित(hwconfig(शून्य) == 0);
	निश्चित(hwconfig("") == 0);
	निश्चित(hwconfig("key3") == 0);

	वापस 0;
}
#पूर्ण_अगर /* HWCONFIG_TEST */
