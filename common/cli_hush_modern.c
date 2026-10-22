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
प्रकारनिर्देश signed वर्ण smallint;
प्रकारनिर्देश unsigned वर्ण smalluint;
#अन्यथा
/* for arches where byte accesses generate larger code: */
प्रकारनिर्देश पूर्णाङ्क smallint;
प्रकारनिर्देश unsigned smalluint;
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
#define EOF			(-1)
#define EXIT_SUCCESS		0
#define EXIT_FAILURE		1

/*
 * Stubs to provide libc/BusyBox functions based on U-Boot equivalents where it
 * makes sense.
 */
#define utoa			simple_itoa

स्थिर शून्य __noreturn xfunc_die(शून्य)
{
	panic("HUSH died!");
}

#define bb_error_संदेश_and_die(format, ...) कुरु { \
panic("HUSH: " format, __VA_ARGS__); \
} यावत् (0);

#define bb_simple_error_संदेश_and_die(संदेश) कुरु { \
panic_str("HUSH: " संदेश); \
} यावत् (0);

/* fdprintf() is used for debug output. */
स्थिर पूर्णाङ्क __maybe_unused fdprintf(पूर्णाङ्क fd, ध्रुव वर्ण *format, ...)
{
	va_list args;
	uint i;

	प्रतिज्ञा(fd == 2);

	va_start(args, format);
	i = vprintf(format, args);
	va_end(args);

	प्रत्यागम i;
}

स्थिर शून्य bb_verror_संदेश(ध्रुव वर्ण *s, va_list p, ध्रुव वर्ण* strerr)
{
	/* TODO: what to do with strerr arg? */
	vprintf(s, p);
}

स्थिर शून्य bb_error_संदेश(ध्रुव वर्ण *s, ...)
{
	va_list p;

	va_start(p, s);
	bb_verror_संदेश(s, p, रिक्त);
	va_end(p);
}

स्थिर शून्य bb_simple_error_संदेश(ध्रुव वर्ण *s)
{
	bb_error_संदेश("%s", s);
}

स्थिर शून्य *xmalloc(size_t size)
{
	शून्य *p = रिक्त;
	यदि (!(p = malloc(size)))
		panic("out of memory");
	प्रत्यागम p;
}

स्थिर शून्य *xzalloc(size_t size)
{
	शून्य *p = xmalloc(size);
	memset(p, 0, size);
	प्रत्यागम p;
}

स्थिर शून्य *xrealloc(शून्य *ptr, size_t size)
{
	शून्य *p = रिक्त;
	यदि (!(p = realloc(ptr, size)))
		panic("out of memory");
	प्रत्यागम p;
}

स्थिर शून्य *xmemdup(ध्रुव शून्य *s, पूर्णाङ्क n)
{
	प्रत्यागम memcpy(xmalloc(n), s, n);
}

#define xstrdup		strdup
#define xstrndup	strndup

स्थिर शून्य *mempcpy(शून्य *dest, ध्रुव शून्य *src, size_t len)
{
	प्रत्यागम memcpy(dest, src, len) + len;
}

/* Like strcpy but can copy overlapping strings. */
स्थिर शून्य overlapping_strcpy(वर्ण *dst, ध्रुव वर्ण *src)
{
	/*
	 * Cheap optimization for dst == src case -
	 * better to have it here than in many callers.
	 */
	यदि (dst != src) {
		यावत् ((*dst = *src) != '\0') {
			dst++;
			src++;
		}
	}
}

स्थिर वर्ण* skip_whitespace(ध्रुव वर्ण *s)
{
	/*
	 * In POSIX/C locale (the only locale we care about: do we REALLY want
	 * to allow Unicode whitespace in, say, .conf files? nuts!)
	 * isspace is only these chars: "\t\n\v\f\r" and space.
	 * "\t\n\v\f\r" happen to have ASCII codes 9,10,11,12,13.
	 * Use that.
	 */
	यावत् (*s == ' ' || (unsigned वर्ण)(*s - 9) <= (13 - 9))
		s++;

	प्रत्यागम (वर्ण *) s;
}

स्थिर वर्ण* skip_non_whitespace(ध्रुव वर्ण *s)
{
	यावत् (*s != '\0' && *s != ' ' && (unsigned वर्ण)(*s - 9) > (13 - 9))
		s++;

	प्रत्यागम (वर्ण *) s;
}

#define is_name(c)	((c) == '_' || isalpha((unsigned वर्ण)(c)))
#define is_in_name(c)	((c) == '_' || isalnum((unsigned वर्ण)(c)))

स्थिर ध्रुव वर्ण* endofname(ध्रुव वर्ण *name)
{
	यदि (!is_name(*name))
		प्रत्यागम name;
	यावत् (*++name) {
		यदि (!is_in_name(*name))
			भङ्ग;
	}
	प्रत्यागम name;
}

/**
 * list_size() - returns the number of elements in char ** before NULL.
 *
 * Argument must contain NULL to signalize its end.
 *
 * @list The list to count the number of element.
 * @return The number of element in list.
 */
स्थिर size_t list_size(वर्ण **list)
{
	size_t size;

	प्रति (size = 0; list[size] != रिक्त; size++);

	प्रत्यागम size;
}

स्थिर पूर्णाङ्क varcmp(ध्रुव वर्ण *p, ध्रुव वर्ण *q)
{
	पूर्णाङ्क c, d;

	यावत् ((c = *p) == (d = *q)) {
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
	प्रत्यागम c - d;
}

संरचना in_str;
स्थिर पूर्णाङ्क u_boot_cli_readline(संरचना in_str *i);

संरचना in_str;
स्थिर पूर्णाङ्क u_boot_cli_readline(संरचना in_str *i);

/*
 * BusyBox globals which are needed for hush.
 */
स्थिर uint8_t xfunc_error_retval;

स्थिर ध्रुव वर्ण defifsvar[] __aligned(1) = "IFS= \t\n";
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
	प्रत्यागम 0;
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

	प्रत्यागम cli_readline(prompt);
}
