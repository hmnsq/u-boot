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

#ಅನ್ತರ್ಭಾವ <env.h>
#ಅನ್ತರ್ಭಾವ <malloc.h>         /* malloc, free, realloc*/
#ಅನ್ತರ್ಭಾವ <linux/ctype.h>    /* isalpha, isdigit */
#ಅನ್ತರ್ಭಾವ <console.h>
#ಅನ್ತರ್ಭಾವ <bootretry.h>
#ಅನ್ತರ್ಭಾವ <cli.h>
#ಅನ್ತರ್ಭಾವ <cli_hush.h>
#ಅನ್ತರ್ಭಾವ <command.h>        /* find_cmd */
#ಅನ್ತರ್ಭಾವ <asm/global_data.h>

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
#ಯದಿ CONFIG_IS_ENABLED(X86) || CONFIG_IS_ENABLED(X86_64) || CONFIG_IS_ENABLED(MIPS)
/* add other arches which benefit from this... */
ಪ್ರಕಾರನಿರ್ದೇಶ signed ವರ್ಣ smallint;
ಪ್ರಕಾರನಿರ್ದೇಶ unsigned ವರ್ಣ smalluint;
#ಅನ್ಯಥಾ
/* for arches where byte accesses generate larger code: */
ಪ್ರಕಾರನಿರ್ದೇಶ ಪೂರ್ಣಾಙ್ಕ smallint;
ಪ್ರಕಾರನಿರ್ದೇಶ unsigned smalluint;
#endif

/*
 * Alignment defines used by BusyBox.
 */
#define ALIGN1			__attribute__((aligned(1)))
#define ALIGN2			__attribute__((aligned(2)))
#define ALIGN4			__attribute__((aligned(4)))
#define ALIGN8			__attribute__((aligned(8)))
#define ALIGN_PTR		__attribute__((aligned(ಪರಿಮಾಣ(ಶೂನ್ಯ*))))

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

ಸ್ಥಿರ ಶೂನ್ಯ __noreturn xfunc_die(ಶೂನ್ಯ)
{
	panic("HUSH died!");
}

#define bb_error_ಸಂದೇಶ_and_die(format, ...) ಕುರು { \
panic("HUSH: " format, __VA_ARGS__); \
} ಯಾವತ್ (0);

#define bb_simple_error_ಸಂದೇಶ_and_die(ಸಂದೇಶ) ಕುರು { \
panic_str("HUSH: " ಸಂದೇಶ); \
} ಯಾವತ್ (0);

/* fdprintf() is used for debug output. */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ __maybe_unused fdprintf(ಪೂರ್ಣಾಙ್ಕ fd, ಧ್ರುವ ವರ್ಣ *format, ...)
{
	va_list args;
	uint i;

	ಪ್ರತಿಜ್ಞಾ(fd == 2);

	va_start(args, format);
	i = vprintf(format, args);
	va_end(args);

	ಪ್ರತ್ಯಾಗಮ i;
}

ಸ್ಥಿರ ಶೂನ್ಯ bb_verror_ಸಂದೇಶ(ಧ್ರುವ ವರ್ಣ *s, va_list p, ಧ್ರುವ ವರ್ಣ* strerr)
{
	/* TODO: what to do with strerr arg? */
	vprintf(s, p);
}

ಸ್ಥಿರ ಶೂನ್ಯ bb_error_ಸಂದೇಶ(ಧ್ರುವ ವರ್ಣ *s, ...)
{
	va_list p;

	va_start(p, s);
	bb_verror_ಸಂದೇಶ(s, p, ರಿಕ್ತ);
	va_end(p);
}

ಸ್ಥಿರ ಶೂನ್ಯ bb_simple_error_ಸಂದೇಶ(ಧ್ರುವ ವರ್ಣ *s)
{
	bb_error_ಸಂದೇಶ("%s", s);
}

ಸ್ಥಿರ ಶೂನ್ಯ *xmalloc(size_t size)
{
	ಶೂನ್ಯ *p = ರಿಕ್ತ;
	ಯದಿ (!(p = malloc(size)))
		panic("out of memory");
	ಪ್ರತ್ಯಾಗಮ p;
}

ಸ್ಥಿರ ಶೂನ್ಯ *xzalloc(size_t size)
{
	ಶೂನ್ಯ *p = xmalloc(size);
	memset(p, 0, size);
	ಪ್ರತ್ಯಾಗಮ p;
}

ಸ್ಥಿರ ಶೂನ್ಯ *xrealloc(ಶೂನ್ಯ *ptr, size_t size)
{
	ಶೂನ್ಯ *p = ರಿಕ್ತ;
	ಯದಿ (!(p = realloc(ptr, size)))
		panic("out of memory");
	ಪ್ರತ್ಯಾಗಮ p;
}

ಸ್ಥಿರ ಶೂನ್ಯ *xmemdup(ಧ್ರುವ ಶೂನ್ಯ *s, ಪೂರ್ಣಾಙ್ಕ n)
{
	ಪ್ರತ್ಯಾಗಮ memcpy(xmalloc(n), s, n);
}

#define xstrdup		strdup
#define xstrndup	strndup

ಸ್ಥಿರ ಶೂನ್ಯ *mempcpy(ಶೂನ್ಯ *dest, ಧ್ರುವ ಶೂನ್ಯ *src, size_t len)
{
	ಪ್ರತ್ಯಾಗಮ memcpy(dest, src, len) + len;
}

/* Like strcpy but can copy overlapping strings. */
ಸ್ಥಿರ ಶೂನ್ಯ overlapping_strcpy(ವರ್ಣ *dst, ಧ್ರುವ ವರ್ಣ *src)
{
	/*
	 * Cheap optimization for dst == src case -
	 * better to have it here than in many callers.
	 */
	ಯದಿ (dst != src) {
		ಯಾವತ್ ((*dst = *src) != '\0') {
			dst++;
			src++;
		}
	}
}

ಸ್ಥಿರ ವರ್ಣ* skip_whitespace(ಧ್ರುವ ವರ್ಣ *s)
{
	/*
	 * In POSIX/C locale (the only locale we care about: do we REALLY want
	 * to allow Unicode whitespace in, say, .conf files? nuts!)
	 * isspace is only these chars: "\t\n\v\f\r" and space.
	 * "\t\n\v\f\r" happen to have ASCII codes 9,10,11,12,13.
	 * Use that.
	 */
	ಯಾವತ್ (*s == ' ' || (unsigned ವರ್ಣ)(*s - 9) <= (13 - 9))
		s++;

	ಪ್ರತ್ಯಾಗಮ (ವರ್ಣ *) s;
}

ಸ್ಥಿರ ವರ್ಣ* skip_non_whitespace(ಧ್ರುವ ವರ್ಣ *s)
{
	ಯಾವತ್ (*s != '\0' && *s != ' ' && (unsigned ವರ್ಣ)(*s - 9) > (13 - 9))
		s++;

	ಪ್ರತ್ಯಾಗಮ (ವರ್ಣ *) s;
}

#define is_name(c)	((c) == '_' || isalpha((unsigned ವರ್ಣ)(c)))
#define is_in_name(c)	((c) == '_' || isalnum((unsigned ವರ್ಣ)(c)))

ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ* endofname(ಧ್ರುವ ವರ್ಣ *name)
{
	ಯದಿ (!is_name(*name))
		ಪ್ರತ್ಯಾಗಮ name;
	ಯಾವತ್ (*++name) {
		ಯದಿ (!is_in_name(*name))
			ಭಙ್ಗ;
	}
	ಪ್ರತ್ಯಾಗಮ name;
}

/**
 * list_size() - returns the number of elements in char ** before NULL.
 *
 * Argument must contain NULL to signalize its end.
 *
 * @list The list to count the number of element.
 * @return The number of element in list.
 */
ಸ್ಥಿರ size_t list_size(ವರ್ಣ **list)
{
	size_t size;

	ಪ್ರತಿ (size = 0; list[size] != ರಿಕ್ತ; size++);

	ಪ್ರತ್ಯಾಗಮ size;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ varcmp(ಧ್ರುವ ವರ್ಣ *p, ಧ್ರುವ ವರ್ಣ *q)
{
	ಪೂರ್ಣಾಙ್ಕ c, d;

	ಯಾವತ್ ((c = *p) == (d = *q)) {
		ಯದಿ (c == '\0' || c == '=')
			ಗಚ್ಛ out;
		p++;
		q++;
	}
	ಯದಿ (c == '=')
		c = '\0';
	ಯದಿ (d == '=')
		d = '\0';
out:
	ಪ್ರತ್ಯಾಗಮ c - d;
}

ಸಂರಚನಾ in_str;
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ u_boot_cli_readline(ಸಂರಚನಾ in_str *i);

ಸಂರಚನಾ in_str;
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ u_boot_cli_readline(ಸಂರಚನಾ in_str *i);

/*
 * BusyBox globals which are needed for hush.
 */
ಸ್ಥಿರ uint8_t xfunc_error_retval;

ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ defifsvar[] __aligned(1) = "IFS= \t\n";
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
#ಅನ್ತರ್ಭಾವ "cli_hush_upstream.c"
/*
 * A                                     A
 * +-- Include of the upstream sources --+ *
 *
 */

ಪೂರ್ಣಾಙ್ಕ u_boot_hush_start_modern(ಶೂನ್ಯ)
{
	INIT_G();
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ u_boot_cli_readline(ಸಂರಚನಾ in_str *i)
{
	ವರ್ಣ *prompt;
	ವರ್ಣ __maybe_unused *ps_prompt = ರಿಕ್ತ;

	ಯದಿ (!G.promptmode)
		prompt = CONFIG_SYS_PROMPT;
#ifdef CONFIG_SYS_PROMPT_HUSH_PS2
	ಅನ್ಯಥಾ
		prompt = CONFIG_SYS_PROMPT_HUSH_PS2;
#ಅನ್ಯಥಾ
	/* TODO: default value? */
	#error "SYS_PROMPT_HUSH_PS2 is not defined!"
#endif

	ಯದಿ (CONFIG_IS_ENABLED(CMDLINE_PS_SUPPORT)) {
		ಯದಿ (!G.promptmode)
			ps_prompt = env_get("PS1");
		ಅನ್ಯಥಾ
			ps_prompt = env_get("PS2");

		ಯದಿ (ps_prompt)
			prompt = ps_prompt;
	}

	ಪ್ರತ್ಯಾಗಮ cli_readline(prompt);
}
