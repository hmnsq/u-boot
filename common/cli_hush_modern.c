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

#अन्तर्भाव <env.h>
#अन्तर्भाव <malloc.h>         /* malloc, free, realloc*/
#अन्तर्भाव <linux/ctype.h>    /* isalpha, isdigit */
#अन्तर्भाव <console.h>
#अन्तर्भाव <bootretry.h>
#अन्तर्भाव <cli.h>
#अन्तर्भाव <cli_hush.h>
#अन्तर्भाव <command.h>        /* find_cmd */
#अन्तर्भाव <asm/global_data.h>

/*
 * BusyBox Version: UPDATE THIS WHEN PULLING NEW UPSTREAM REVISION!
 */
#define BB_VER			"1.37.0.git23da5c4b716b"

/*
 * Define hush features by the names used upstream.
 */
#define ENABLE_HUSH_INTERACTIVE	1
#define ENABLE_FEATURE_EDITING	1
#define ENABLE_HUSH_IF		1
#define ENABLE_HUSH_LOOPS	1
/* No MMU in U-Boot */
#define BB_MMU			0
#define USE_FOR_NOMMU(...)	__VA_ARGS__
#define USE_FOR_MMU(...)

/*
 * Size-saving "small" ints (arch-dependent)
 */
#जर CONFIG_IS_ENABLED(X86) || CONFIG_IS_ENABLED(X86_64) || CONFIG_IS_ENABLED(MIPS)
/* add other arches which benefit from this... */
प्रकारव्याख्या चिन्हित अक्षर smallint;
प्रकारव्याख्या अचिन्हित अक्षर smalluint;
#अन्यथा
/* for arches where byte accesses generate larger code: */
प्रकारव्याख्या पूर्णांक smallint;
प्रकारव्याख्या अचिन्हित smalluint;
#endif

/*
 * Alignment defines used by BusyBox.
 */
#define ALIGN1			__attribute__((aligned(1)))
#define ALIGN2			__attribute__((aligned(2)))
#define ALIGN4			__attribute__((aligned(4)))
#define ALIGN8			__attribute__((aligned(8)))
#define ALIGN_PTR		__attribute__((aligned(आकार(शून्य*))))

/*
 * Miscellaneous compiler/platform defines.
 */
#define FAST_FUNC /* not used in U-Boot */
#define UNUSED_PARAM		__always_unused
#define ALWAYS_INLINE		__always_inline
#define NOINLINE		noinline

/*
 * Defines to provide equivalents to what libc/BusyBox defines.
 */
#define खातापूर्ण			(-1)
#define निकास_सफल		0
#define निकास_त्रुटि		1

/*
 * Stubs to provide libc/BusyBox functions based on U-Boot equivalents where it
 * makes sense.
 */
#define utoa			simple_itoa

स्थिर शून्य __noreturn xfunc_die(शून्य)
{
	panic("HUSH died!");
}

#define bb_error_संदेश_and_die(format, ...) करा { \
panic("HUSH: " format, __VA_ARGS__); \
} पर्यंत (0);

#define bb_simple_error_संदेश_and_die(संदेश) करा { \
panic_str("HUSH: " संदेश); \
} पर्यंत (0);

/* fdprintf() is used for debug output. */
स्थिर पूर्णांक __maybe_unused fdprintf(पूर्णांक fd, स्थिरांक अक्षर *format, ...)
{
	बहु_सूची args;
	uint i;

	प्रतिज्ञा(fd == 2);

	बहु_शुरू(args, format);
	i = भ_लिखो(format, args);
	बहु_पूर्ण(args);

	परत i;
}

स्थिर शून्य bb_verror_संदेश(स्थिरांक अक्षर *s, बहु_सूची p, स्थिरांक अक्षर* strerr)
{
	/* TODO: what to do with strerr arg? */
	भ_लिखो(s, p);
}

स्थिर शून्य bb_error_संदेश(स्थिरांक अक्षर *s, ...)
{
	बहु_सूची p;

	बहु_शुरू(p, s);
	bb_verror_संदेश(s, p, रिक्त);
	बहु_पूर्ण(p);
}

स्थिर शून्य bb_simple_error_संदेश(स्थिरांक अक्षर *s)
{
	bb_error_संदेश("%s", s);
}

स्थिर शून्य *xmalloc(माप_प्रकार size)
{
	शून्य *p = रिक्त;
	जर (!(p = दो_स्मृति(size)))
		panic("out of memory");
	परत p;
}

स्थिर शून्य *xzalloc(माप_प्रकार size)
{
	शून्य *p = xmalloc(size);
	स_रखो(p, 0, size);
	परत p;
}

स्थिर शून्य *xrealloc(शून्य *ptr, माप_प्रकार size)
{
	शून्य *p = रिक्त;
	जर (!(p = पुनः_स्मृति(ptr, size)))
		panic("out of memory");
	परत p;
}

स्थिर शून्य *xmemdup(स्थिरांक शून्य *s, पूर्णांक n)
{
	परत स_नकल(xmalloc(n), s, n);
}

#define xstrdup		strdup
#define xstrndup	strndup

स्थिर शून्य *mempcpy(शून्य *dest, स्थिरांक शून्य *src, माप_प्रकार len)
{
	परत स_नकल(dest, src, len) + len;
}

/* Like strcpy but can copy overlapping strings. */
स्थिर शून्य overlapping_strcpy(अक्षर *dst, स्थिरांक अक्षर *src)
{
	/*
	 * Cheap optimization for dst == src case -
	 * better to have it here than in many callers.
	 */
	जर (dst != src) {
		पर्यंत ((*dst = *src) != '\0') {
			dst++;
			src++;
		}
	}
}

स्थिर अक्षर* skip_whitespace(स्थिरांक अक्षर *s)
{
	/*
	 * In POSIX/C locale (the only locale we care about: do we REALLY want
	 * to allow Unicode whitespace in, say, .conf files? nuts!)
	 * isspace is only these chars: "\t\n\v\f\r" and space.
	 * "\t\n\v\f\r" happen to have ASCII codes 9,10,11,12,13.
	 * Use that.
	 */
	पर्यंत (*s == ' ' || (अचिन्हित अक्षर)(*s - 9) <= (13 - 9))
		s++;

	परत (अक्षर *) s;
}

स्थिर अक्षर* skip_non_whitespace(स्थिरांक अक्षर *s)
{
	पर्यंत (*s != '\0' && *s != ' ' && (अचिन्हित अक्षर)(*s - 9) > (13 - 9))
		s++;

	परत (अक्षर *) s;
}

#define is_name(c)	((c) == '_' || है_अक्षर((अचिन्हित अक्षर)(c)))
#define is_in_name(c)	((c) == '_' || है_अक्षर_अंक((अचिन्हित अक्षर)(c)))

स्थिर स्थिरांक अक्षर* endofname(स्थिरांक अक्षर *name)
{
	जर (!is_name(*name))
		परत name;
	पर्यंत (*++name) {
		जर (!is_in_name(*name))
			थांबा;
	}
	परत name;
}

/**
 * list_size() - returns the number of elements in char ** before NULL.
 *
 * Argument must contain NULL to signalize its end.
 *
 * @list The list to count the number of element.
 * @return The number of element in list.
 */
स्थिर माप_प्रकार list_size(अक्षर **list)
{
	माप_प्रकार size;

	साठी (size = 0; list[size] != रिक्त; size++);

	परत size;
}

स्थिर पूर्णांक varcmp(स्थिरांक अक्षर *p, स्थिरांक अक्षर *q)
{
	पूर्णांक c, d;

	पर्यंत ((c = *p) == (d = *q)) {
		जर (c == '\0' || c == '=')
			जा out;
		p++;
		q++;
	}
	जर (c == '=')
		c = '\0';
	जर (d == '=')
		d = '\0';
out:
	परत c - d;
}

संरचना in_str;
स्थिर पूर्णांक u_boot_cli_readline(संरचना in_str *i);

संरचना in_str;
स्थिर पूर्णांक u_boot_cli_readline(संरचना in_str *i);

/*
 * BusyBox globals which are needed for hush.
 */
स्थिर uint8_t xfunc_error_retval;

स्थिर स्थिरांक अक्षर defifsvar[] __aligned(1) = "IFS= \t\n";
#define defifs (defifsvar + 4)

/* This define is used to check if exit command was called. */
#define EXIT_RET_CODE -2

/*
 * This define is used for changes that need be done directly in the upstream
 * sources still. Ideally, its use should be minimized as much as possible.
 */
#define __U_BOOT__

/*
 *
 * +-- Include of the upstream sources --+ *
 * V                                     V
 */
#अन्तर्भाव "cli_hush_upstream.c"
/*
 * A                                     A
 * +-- Include of the upstream sources --+ *
 *
 */

पूर्णांक u_boot_hush_start_modern(शून्य)
{
	INIT_G();
	परत 0;
}

स्थिर पूर्णांक u_boot_cli_readline(संरचना in_str *i)
{
	अक्षर *prompt;
	अक्षर __maybe_unused *ps_prompt = रिक्त;

	जर (!G.promptmode)
		prompt = CONFIG_SYS_PROMPT;
#ifdef CONFIG_SYS_PROMPT_HUSH_PS2
	अन्यथा
		prompt = CONFIG_SYS_PROMPT_HUSH_PS2;
#अन्यथा
	/* TODO: default value? */
	#error "SYS_PROMPT_HUSH_PS2 is not defined!"
#endif

	जर (CONFIG_IS_ENABLED(CMDLINE_PS_SUPPORT)) {
		जर (!G.promptmode)
			ps_prompt = env_get("PS1");
		अन्यथा
			ps_prompt = env_get("PS2");

		जर (ps_prompt)
			prompt = ps_prompt;
	}

	परत cli_readline(prompt);
}
