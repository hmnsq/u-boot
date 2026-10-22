// SPDX-License-Identifier: GPL-2.0+
/*
 * This file defines the compilation unit for the new hush shell version.  The
 * actual implementation from upstream BusyBox can be found in
 * `cli_hush_upstream.c` which is included at the end of this file.
 *
 * This "wrapper" technique is used to keep the changes to the upstream version
 * as minmal as possible.  Instead, all defines and redefines necessary are done
 * here, outside the upstream sources.  This will hopefully make upgrades to
 * newer revisions much easier.
 *
 * Copyright (c) 2021, Harald Seiler, DENX Software Engineering, hws@denx.de
 */

#समावेश <env.h>
#समावेश <malloc.h>         /* malloc, free, realloc*/
#समावेश <linux/ctype.h>    /* isalpha, isdigit */
#समावेश <console.h>
#समावेश <bootretry.h>
#समावेश <cli.h>
#समावेश <cli_hush.h>
#समावेश <command.h>        /* find_cmd */
#समावेश <asm/global_data.h>

/*
 * BusyBox Version: UPDATE THIS WHEN PULLING NEW UPSTREAM REVISION!
 */
#घोषणा BB_VER			"1.37.0.git23da5c4b716b"

/*
 * Define hush features by the names used upstream.
 */
#घोषणा ENABLE_HUSH_INTERACTIVE	1
#घोषणा ENABLE_FEATURE_EDITING	1
#घोषणा ENABLE_HUSH_IF		1
#घोषणा ENABLE_HUSH_LOOPS	1
/* No MMU in U-Boot */
#घोषणा BB_MMU			0
#घोषणा USE_FOR_NOMMU(...)	__VA_ARGS__
#घोषणा USE_FOR_MMU(...)

/*
 * Size-saving "small" ints (arch-dependent)
 */
#अगर CONFIG_IS_ENABLED(X86) || CONFIG_IS_ENABLED(X86_64) || CONFIG_IS_ENABLED(MIPS)
/* add other arches which benefit from this... */
प्रकार चिन्हित अक्षर smallint;
प्रकार अचिन्हित अक्षर smalluint;
#अन्यथा
/* for arches where byte accesses generate larger code: */
प्रकार पूर्णांक smallint;
प्रकार अचिन्हित smalluint;
#पूर्ण_अगर

/*
 * Alignment defines used by BusyBox.
 */
#घोषणा ALIGN1			__attribute__((aligned(1)))
#घोषणा ALIGN2			__attribute__((aligned(2)))
#घोषणा ALIGN4			__attribute__((aligned(4)))
#घोषणा ALIGN8			__attribute__((aligned(8)))
#घोषणा ALIGN_PTR		__attribute__((aligned(माप(व्योम*))))

/*
 * Miscellaneous compiler/platform defines.
 */
#घोषणा FAST_FUNC /* not used in U-Boot */
#घोषणा UNUSED_PARAM		__always_unused
#घोषणा ALWAYS_INLINE		__always_inline
#घोषणा NOINLINE		noinline

/*
 * Defines to provide equivalents to what libc/BusyBox defines.
 */
#घोषणा खातापूर्ण			(-1)
#घोषणा निकास_सफल		0
#घोषणा निकास_त्रुटि		1

/*
 * Stubs to provide libc/BusyBox functions based on U-Boot equivalents where it
 * makes sense.
 */
#घोषणा utoa			simple_itoa

अटल व्योम __noreturn xfunc_die(व्योम)
{
	panic("HUSH died!");
}

#घोषणा bb_error_संदेश_and_die(format, ...) करो { \
panic("HUSH: " format, __VA_ARGS__); \
} जबतक (0);

#घोषणा bb_simple_error_संदेश_and_die(संदेश) करो { \
panic_str("HUSH: " संदेश); \
} जबतक (0);

/* fdprintf() is used for debug output. */
अटल पूर्णांक __maybe_unused fdprintf(पूर्णांक fd, स्थिर अक्षर *format, ...)
{
	बहु_सूची args;
	uint i;

	निश्चित(fd == 2);

	बहु_शुरू(args, format);
	i = भ_लिखो(format, args);
	बहु_पूर्ण(args);

	वापस i;
}

अटल व्योम bb_verror_संदेश(स्थिर अक्षर *s, बहु_सूची p, स्थिर अक्षर* strerr)
{
	/* TODO: what to do with strerr arg? */
	भ_लिखो(s, p);
}

अटल व्योम bb_error_संदेश(स्थिर अक्षर *s, ...)
{
	बहु_सूची p;

	बहु_शुरू(p, s);
	bb_verror_संदेश(s, p, शून्य);
	बहु_पूर्ण(p);
}

अटल व्योम bb_simple_error_संदेश(स्थिर अक्षर *s)
{
	bb_error_संदेश("%s", s);
}

अटल व्योम *xmalloc(माप_प्रकार आकार)
{
	व्योम *p = शून्य;
	अगर (!(p = दो_स्मृति(आकार)))
		panic("out of memory");
	वापस p;
}

अटल व्योम *xzalloc(माप_प्रकार आकार)
{
	व्योम *p = xmalloc(आकार);
	स_रखो(p, 0, आकार);
	वापस p;
}

अटल व्योम *xrealloc(व्योम *ptr, माप_प्रकार आकार)
{
	व्योम *p = शून्य;
	अगर (!(p = पुनः_स्मृति(ptr, आकार)))
		panic("out of memory");
	वापस p;
}

अटल व्योम *xmemdup(स्थिर व्योम *s, पूर्णांक n)
{
	वापस स_नकल(xmalloc(n), s, n);
}

#घोषणा xstrdup		strdup
#घोषणा xstrndup	strndup

अटल व्योम *mempcpy(व्योम *dest, स्थिर व्योम *src, माप_प्रकार len)
{
	वापस स_नकल(dest, src, len) + len;
}

/* Like strcpy but can copy overlapping strings. */
अटल व्योम overlapping_strcpy(अक्षर *dst, स्थिर अक्षर *src)
{
	/*
	 * Cheap optimization for dst == src case -
	 * better to have it here than in many callers.
	 */
	अगर (dst != src) {
		जबतक ((*dst = *src) != '\0') {
			dst++;
			src++;
		}
	}
}

अटल अक्षर* skip_whitespace(स्थिर अक्षर *s)
{
	/*
	 * In POSIX/C locale (the only locale we care about: do we REALLY want
	 * to allow Unicode whitespace in, say, .conf files? nuts!)
	 * isspace is only these chars: "\t\n\v\f\r" and space.
	 * "\t\n\v\f\r" happen to have ASCII codes 9,10,11,12,13.
	 * Use that.
	 */
	जबतक (*s == ' ' || (अचिन्हित अक्षर)(*s - 9) <= (13 - 9))
		s++;

	वापस (अक्षर *) s;
}

अटल अक्षर* skip_non_whitespace(स्थिर अक्षर *s)
{
	जबतक (*s != '\0' && *s != ' ' && (अचिन्हित अक्षर)(*s - 9) > (13 - 9))
		s++;

	वापस (अक्षर *) s;
}

#घोषणा is_name(c)	((c) == '_' || है_अक्षर((अचिन्हित अक्षर)(c)))
#घोषणा is_in_name(c)	((c) == '_' || है_अक्षर_अंक((अचिन्हित अक्षर)(c)))

अटल स्थिर अक्षर* endofname(स्थिर अक्षर *name)
{
	अगर (!is_name(*name))
		वापस name;
	जबतक (*++name) {
		अगर (!is_in_name(*name))
			अवरोध;
	}
	वापस name;
}

/**
 * list_size() - returns the number of elements in char ** before NULL.
 *
 * Argument must contain NULL to signalize its end.
 *
 * @list The list to count the number of element.
 * @return The number of element in list.
 */
अटल माप_प्रकार list_आकार(अक्षर **list)
{
	माप_प्रकार आकार;

	क्रम (आकार = 0; list[आकार] != शून्य; आकार++);

	वापस आकार;
}

अटल पूर्णांक varcmp(स्थिर अक्षर *p, स्थिर अक्षर *q)
{
	पूर्णांक c, d;

	जबतक ((c = *p) == (d = *q)) {
		अगर (c == '\0' || c == '=')
			जाओ out;
		p++;
		q++;
	}
	अगर (c == '=')
		c = '\0';
	अगर (d == '=')
		d = '\0';
out:
	वापस c - d;
}

काष्ठा in_str;
अटल पूर्णांक u_boot_cli_readline(काष्ठा in_str *i);

काष्ठा in_str;
अटल पूर्णांक u_boot_cli_readline(काष्ठा in_str *i);

/*
 * BusyBox globals which are needed for hush.
 */
अटल uint8_t xfunc_error_retval;

अटल स्थिर अक्षर defifsvar[] __aligned(1) = "IFS= \t\n";
#घोषणा defifs (defifsvar + 4)

/* This define is used to check if exit command was called. */
#घोषणा EXIT_RET_CODE -2

/*
 * This define is used for changes that need be done directly in the upstream
 * sources still. Ideally, its use should be minimized as much as possible.
 */
#घोषणा __U_BOOT__

/*
 *
 * +-- Include of the upstream sources --+ *
 * V                                     V
 */
#समावेश "cli_hush_upstream.c"
/*
 * A                                     A
 * +-- Include of the upstream sources --+ *
 *
 */

पूर्णांक u_boot_hush_start_modern(व्योम)
{
	INIT_G();
	वापस 0;
}

अटल पूर्णांक u_boot_cli_readline(काष्ठा in_str *i)
{
	अक्षर *prompt;
	अक्षर __maybe_unused *ps_prompt = शून्य;

	अगर (!G.promptmode)
		prompt = CONFIG_SYS_PROMPT;
#अगर_घोषित CONFIG_SYS_PROMPT_HUSH_PS2
	अन्यथा
		prompt = CONFIG_SYS_PROMPT_HUSH_PS2;
#अन्यथा
	/* TODO: default value? */
	#त्रुटि "SYS_PROMPT_HUSH_PS2 is not defined!"
#पूर्ण_अगर

	अगर (CONFIG_IS_ENABLED(CMDLINE_PS_SUPPORT)) {
		अगर (!G.promptmode)
			ps_prompt = env_get("PS1");
		अन्यथा
			ps_prompt = env_get("PS2");

		अगर (ps_prompt)
			prompt = ps_prompt;
	}

	वापस cli_readline(prompt);
}
