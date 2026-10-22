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
#यदि CONFIG_IS_ENABLED(X86) || CONFIG_IS_ENABLED(X86_64) || CONFIG_IS_ENABLED(MIPS)
/* add other arches which benefit from this... */
प्रकारनिर्देश चिन्हित वर्ण smallint;
प्रकारनिर्देश अचिन्हित वर्ण smalluint;
#अन्यथा
/* for arches where byte accesses generate larger code: */
प्रकारनिर्देश पूर्णाङ्क smallint;
प्रकारनिर्देश अचिन्हित smalluint;
#endif

/*
 * Alignment defines used by BusyBox.
 */
#define ALIGN1			__attribute__((aligned(1)))
#define ALIGN2			__attribute__((aligned(2)))
#define ALIGN4			__attribute__((aligned(4)))
#define ALIGN8			__attribute__((aligned(8)))
#define ALIGN_PTR		__attribute__((aligned(परिमाण(शून्य*))))

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

#define bb_error_सन्देश_and_die(format, ...) गर { \
panic("HUSH: " format, __VA_ARGS__); \
} जबसम्म (0);

#define bb_simple_error_सन्देश_and_die(सन्देश) गर { \
panic_str("HUSH: " सन्देश); \
} जबसम्म (0);

/* fdprintf() is used for debug output. */
स्थिर पूर्णाङ्क __maybe_unused fdprintf(पूर्णाङ्क fd, अचर वर्ण *format, ...)
{
	बहु_सूची args;
	uint i;

	प्रतिज्ञा(fd == 2);

	बहु_शुरू(args, format);
	i = भ_लिखो(format, args);
	बहु_पूर्ण(args);

	फर्काऊ i;
}

स्थिर शून्य bb_verror_सन्देश(अचर वर्ण *s, बहु_सूची p, अचर वर्ण* strerr)
{
	/* TODO: what to do with strerr arg? */
	भ_लिखो(s, p);
}

स्थिर शून्य bb_error_सन्देश(अचर वर्ण *s, ...)
{
	बहु_सूची p;

	बहु_शुरू(p, s);
	bb_verror_सन्देश(s, p, रिक्त);
	बहु_पूर्ण(p);
}

स्थिर शून्य bb_simple_error_सन्देश(अचर वर्ण *s)
{
	bb_error_सन्देश("%s", s);
}

स्थिर शून्य *xmalloc(माप_प्रकार size)
{
	शून्य *p = रिक्त;
	यदि (!(p = दो_स्मृति(size)))
		panic("out of memory");
	फर्काऊ p;
}

स्थिर शून्य *xzalloc(माप_प्रकार size)
{
	शून्य *p = xmalloc(size);
	स_रखो(p, 0, size);
	फर्काऊ p;
}

स्थिर शून्य *xrealloc(शून्य *ptr, माप_प्रकार size)
{
	शून्य *p = रिक्त;
	यदि (!(p = पुनः_स्मृति(ptr, size)))
		panic("out of memory");
	फर्काऊ p;
}

स्थिर शून्य *xmemdup(अचर शून्य *s, पूर्णाङ्क n)
{
	फर्काऊ स_नकल(xmalloc(n), s, n);
}

#define xstrdup		strdup
#define xstrndup	strndup

स्थिर शून्य *mempcpy(शून्य *dest, अचर शून्य *src, माप_प्रकार len)
{
	फर्काऊ स_नकल(dest, src, len) + len;
}

/* Like strcpy but can copy overlapping strings. */
स्थिर शून्य overlapping_strcpy(वर्ण *dst, अचर वर्ण *src)
{
	/*
	 * Cheap optimization for dst == src case -
	 * better to have it here than in many callers.
	 */
	यदि (dst != src) {
		जबसम्म ((*dst = *src) != '\0') {
			dst++;
			src++;
		}
	}
}

स्थिर वर्ण* skip_whitespace(अचर वर्ण *s)
{
	/*
	 * In POSIX/C locale (the only locale we care about: do we REALLY want
	 * to allow Unicode whitespace in, say, .conf files? nuts!)
	 * isspace is only these chars: "\t\n\v\f\r" and space.
	 * "\t\n\v\f\r" happen to have ASCII codes 9,10,11,12,13.
	 * Use that.
	 */
	जबसम्म (*s == ' ' || (अचिन्हित वर्ण)(*s - 9) <= (13 - 9))
		s++;

	फर्काऊ (वर्ण *) s;
}

स्थिर वर्ण* skip_non_whitespace(अचर वर्ण *s)
{
	जबसम्म (*s != '\0' && *s != ' ' && (अचिन्हित वर्ण)(*s - 9) > (13 - 9))
		s++;

	फर्काऊ (वर्ण *) s;
}

#define is_name(c)	((c) == '_' || है_अक्षर((अचिन्हित वर्ण)(c)))
#define is_in_name(c)	((c) == '_' || है_अक्षर_अंक((अचिन्हित वर्ण)(c)))

स्थिर अचर वर्ण* endofname(अचर वर्ण *name)
{
	यदि (!is_name(*name))
		फर्काऊ name;
	जबसम्म (*++name) {
		यदि (!is_in_name(*name))
			रोक;
	}
	फर्काऊ name;
}

/**
 * list_size() - returns the number of elements in char ** before NULL.
 *
 * Argument must contain NULL to signalize its end.
 *
 * @list The list to count the number of element.
 * @return The number of element in list.
 */
स्थिर माप_प्रकार list_size(वर्ण **list)
{
	माप_प्रकार size;

	लागि (size = 0; list[size] != रिक्त; size++);

	फर्काऊ size;
}

स्थिर पूर्णाङ्क varcmp(अचर वर्ण *p, अचर वर्ण *q)
{
	पूर्णाङ्क c, d;

	जबसम्म ((c = *p) == (d = *q)) {
		यदि (c == '\0' || c == '=')
			गच्छ out;
		p++;
		q++;
	}
	यदि (c == '=')
		c = '\0';
	यदि (d == '=')
		d = '\0';
out:
	फर्काऊ c - d;
}

संरचना in_str;
स्थिर पूर्णाङ्क u_boot_cli_readline(संरचना in_str *i);

संरचना in_str;
स्थिर पूर्णाङ्क u_boot_cli_readline(संरचना in_str *i);

/*
 * BusyBox globals which are needed for hush.
 */
स्थिर uint8_t xfunc_error_retval;

स्थिर अचर वर्ण defifsvar[] __aligned(1) = "IFS= \t\n";
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

पूर्णाङ्क u_boot_hush_start_modern(शून्य)
{
	INIT_G();
	फर्काऊ 0;
}

स्थिर पूर्णाङ्क u_boot_cli_readline(संरचना in_str *i)
{
	वर्ण *prompt;
	वर्ण __maybe_unused *ps_prompt = रिक्त;

	यदि (!G.promptmode)
		prompt = CONFIG_SYS_PROMPT;
#ifdef CONFIG_SYS_PROMPT_HUSH_PS2
	अन्यथा
		prompt = CONFIG_SYS_PROMPT_HUSH_PS2;
#अन्यथा
	/* TODO: default value? */
	#error "SYS_PROMPT_HUSH_PS2 is not defined!"
#endif

	यदि (CONFIG_IS_ENABLED(CMDLINE_PS_SUPPORT)) {
		यदि (!G.promptmode)
			ps_prompt = env_get("PS1");
		अन्यथा
			ps_prompt = env_get("PS2");

		यदि (ps_prompt)
			prompt = ps_prompt;
	}

	फर्काऊ cli_readline(prompt);
}
