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

#അന്തര്ഭാവ <env.h>
#അന്തര്ഭാവ <malloc.h>         /* malloc, free, realloc*/
#അന്തര്ഭാവ <linux/ctype.h>    /* isalpha, isdigit */
#അന്തര്ഭാവ <console.h>
#അന്തര്ഭാവ <bootretry.h>
#അന്തര്ഭാവ <cli.h>
#അന്തര്ഭാവ <cli_hush.h>
#അന്തര്ഭാവ <command.h>        /* find_cmd */
#അന്തര്ഭാവ <asm/global_data.h>

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
#എങ്കിൽ CONFIG_IS_ENABLED(X86) || CONFIG_IS_ENABLED(X86_64) || CONFIG_IS_ENABLED(MIPS)
/* add other arches which benefit from this... */
തരനിർവചനം signed അക്ഷരം smallint;
തരനിർവചനം unsigned അക്ഷരം smalluint;
#അല്ലെങ്കിൽ
/* for arches where byte accesses generate larger code: */
തരനിർവചനം പൂർണ്ണസംഖ്യ smallint;
തരനിർവചനം unsigned smalluint;
#endif

/*
 * Alignment defines used by BusyBox.
 */
#define ALIGN1			__attribute__((aligned(1)))
#define ALIGN2			__attribute__((aligned(2)))
#define ALIGN4			__attribute__((aligned(4)))
#define ALIGN8			__attribute__((aligned(8)))
#define ALIGN_PTR		__attribute__((aligned(വലുപ്പം(ശൂന്യം*))))

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

സ്ഥിതം ശൂന്യം __noreturn xfunc_die(ശൂന്യം)
{
	panic("HUSH died!");
}

#define bb_error_സന്ദേശം_and_die(format, ...) ചെയ്യുക { \
panic("HUSH: " format, __VA_ARGS__); \
} ആകുമ്പോൾ (0);

#define bb_simple_error_സന്ദേശം_and_die(സന്ദേശം) ചെയ്യുക { \
panic_str("HUSH: " സന്ദേശം); \
} ആകുമ്പോൾ (0);

/* fdprintf() is used for debug output. */
സ്ഥിതം പൂർണ്ണസംഖ്യ __maybe_unused fdprintf(പൂർണ്ണസംഖ്യ fd, സ്ഥിരം അക്ഷരം *format, ...)
{
	va_list args;
	uint i;

	പ്രതിജ്ഞാ(fd == 2);

	va_start(args, format);
	i = vprintf(format, args);
	va_end(args);

	തിരികെ i;
}

സ്ഥിതം ശൂന്യം bb_verror_സന്ദേശം(സ്ഥിരം അക്ഷരം *s, va_list p, സ്ഥിരം അക്ഷരം* strerr)
{
	/* TODO: what to do with strerr arg? */
	vprintf(s, p);
}

സ്ഥിതം ശൂന്യം bb_error_സന്ദേശം(സ്ഥിരം അക്ഷരം *s, ...)
{
	va_list p;

	va_start(p, s);
	bb_verror_സന്ദേശം(s, p, ശൂന്യ);
	va_end(p);
}

സ്ഥിതം ശൂന്യം bb_simple_error_സന്ദേശം(സ്ഥിരം അക്ഷരം *s)
{
	bb_error_സന്ദേശം("%s", s);
}

സ്ഥിതം ശൂന്യം *xmalloc(size_t size)
{
	ശൂന്യം *p = ശൂന്യ;
	എങ്കിൽ (!(p = malloc(size)))
		panic("out of memory");
	തിരികെ p;
}

സ്ഥിതം ശൂന്യം *xzalloc(size_t size)
{
	ശൂന്യം *p = xmalloc(size);
	memset(p, 0, size);
	തിരികെ p;
}

സ്ഥിതം ശൂന്യം *xrealloc(ശൂന്യം *ptr, size_t size)
{
	ശൂന്യം *p = ശൂന്യ;
	എങ്കിൽ (!(p = realloc(ptr, size)))
		panic("out of memory");
	തിരികെ p;
}

സ്ഥിതം ശൂന്യം *xmemdup(സ്ഥിരം ശൂന്യം *s, പൂർണ്ണസംഖ്യ n)
{
	തിരികെ memcpy(xmalloc(n), s, n);
}

#define xstrdup		strdup
#define xstrndup	strndup

സ്ഥിതം ശൂന്യം *mempcpy(ശൂന്യം *dest, സ്ഥിരം ശൂന്യം *src, size_t len)
{
	തിരികെ memcpy(dest, src, len) + len;
}

/* Like strcpy but can copy overlapping strings. */
സ്ഥിതം ശൂന്യം overlapping_strcpy(അക്ഷരം *dst, സ്ഥിരം അക്ഷരം *src)
{
	/*
	 * Cheap optimization for dst == src case -
	 * better to have it here than in many callers.
	 */
	എങ്കിൽ (dst != src) {
		ആകുമ്പോൾ ((*dst = *src) != '\0') {
			dst++;
			src++;
		}
	}
}

സ്ഥിതം അക്ഷരം* skip_whitespace(സ്ഥിരം അക്ഷരം *s)
{
	/*
	 * In POSIX/C locale (the only locale we care about: do we REALLY want
	 * to allow Unicode whitespace in, say, .conf files? nuts!)
	 * isspace is only these chars: "\t\n\v\f\r" and space.
	 * "\t\n\v\f\r" happen to have ASCII codes 9,10,11,12,13.
	 * Use that.
	 */
	ആകുമ്പോൾ (*s == ' ' || (unsigned അക്ഷരം)(*s - 9) <= (13 - 9))
		s++;

	തിരികെ (അക്ഷരം *) s;
}

സ്ഥിതം അക്ഷരം* skip_non_whitespace(സ്ഥിരം അക്ഷരം *s)
{
	ആകുമ്പോൾ (*s != '\0' && *s != ' ' && (unsigned അക്ഷരം)(*s - 9) > (13 - 9))
		s++;

	തിരികെ (അക്ഷരം *) s;
}

#define is_name(c)	((c) == '_' || isalpha((unsigned അക്ഷരം)(c)))
#define is_in_name(c)	((c) == '_' || isalnum((unsigned അക്ഷരം)(c)))

സ്ഥിതം സ്ഥിരം അക്ഷരം* endofname(സ്ഥിരം അക്ഷരം *name)
{
	എങ്കിൽ (!is_name(*name))
		തിരികെ name;
	ആകുമ്പോൾ (*++name) {
		എങ്കിൽ (!is_in_name(*name))
			നിർത്തുക;
	}
	തിരികെ name;
}

/**
 * list_size() - returns the number of elements in char ** before NULL.
 *
 * Argument must contain NULL to signalize its end.
 *
 * @list The list to count the number of element.
 * @return The number of element in list.
 */
സ്ഥിതം size_t list_size(അക്ഷരം **list)
{
	size_t size;

	വേണ്ടി (size = 0; list[size] != ശൂന്യ; size++);

	തിരികെ size;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ varcmp(സ്ഥിരം അക്ഷരം *p, സ്ഥിരം അക്ഷരം *q)
{
	പൂർണ്ണസംഖ്യ c, d;

	ആകുമ്പോൾ ((c = *p) == (d = *q)) {
		എങ്കിൽ (c == '\0' || c == '=')
			പോകുക out;
		p++;
		q++;
	}
	എങ്കിൽ (c == '=')
		c = '\0';
	എങ്കിൽ (d == '=')
		d = '\0';
out:
	തിരികെ c - d;
}

ഘടന in_str;
സ്ഥിതം പൂർണ്ണസംഖ്യ u_boot_cli_readline(ഘടന in_str *i);

ഘടന in_str;
സ്ഥിതം പൂർണ്ണസംഖ്യ u_boot_cli_readline(ഘടന in_str *i);

/*
 * BusyBox globals which are needed for hush.
 */
സ്ഥിതം uint8_t xfunc_error_retval;

സ്ഥിതം സ്ഥിരം അക്ഷരം defifsvar[] __aligned(1) = "IFS= \t\n";
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
#അന്തര്ഭാവ "cli_hush_upstream.c"
/*
 * A                                     A
 * +-- Include of the upstream sources --+ *
 *
 */

പൂർണ്ണസംഖ്യ u_boot_hush_start_modern(ശൂന്യം)
{
	INIT_G();
	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ u_boot_cli_readline(ഘടന in_str *i)
{
	അക്ഷരം *prompt;
	അക്ഷരം __maybe_unused *ps_prompt = ശൂന്യ;

	എങ്കിൽ (!G.promptmode)
		prompt = CONFIG_SYS_PROMPT;
#ifdef CONFIG_SYS_PROMPT_HUSH_PS2
	അല്ലെങ്കിൽ
		prompt = CONFIG_SYS_PROMPT_HUSH_PS2;
#അല്ലെങ്കിൽ
	/* TODO: default value? */
	#error "SYS_PROMPT_HUSH_PS2 is not defined!"
#endif

	എങ്കിൽ (CONFIG_IS_ENABLED(CMDLINE_PS_SUPPORT)) {
		എങ്കിൽ (!G.promptmode)
			ps_prompt = env_get("PS1");
		അല്ലെങ്കിൽ
			ps_prompt = env_get("PS2");

		എങ്കിൽ (ps_prompt)
			prompt = ps_prompt;
	}

	തിരികെ cli_readline(prompt);
}
