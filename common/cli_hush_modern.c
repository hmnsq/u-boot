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
#ವ್ಯಾಖ್ಯಾನಿಸು BB_VER			"1.37.0.git23da5c4b716b"

/*
 * Define hush features by the names used upstream.
 */
#ವ್ಯಾಖ್ಯಾನಿಸು ENABLE_HUSH_INTERACTIVE	1
#ವ್ಯಾಖ್ಯಾನಿಸು ENABLE_FEATURE_EDITING	1
#ವ್ಯಾಖ್ಯಾನಿಸು ENABLE_HUSH_IF		1
#ವ್ಯಾಖ್ಯಾನಿಸು ENABLE_HUSH_LOOPS	1
/* No MMU in U-Boot */
#ವ್ಯಾಖ್ಯಾನಿಸು BB_MMU			0
#ವ್ಯಾಖ್ಯಾನಿಸು USE_FOR_NOMMU(...)	__VA_ARGS__
#ವ್ಯಾಖ್ಯಾನಿಸು USE_FOR_MMU(...)

/*
 * Size-saving "small" ints (arch-dependent)
 */
#ಆದರೆ CONFIG_IS_ENABLED(X86) || CONFIG_IS_ENABLED(X86_64) || CONFIG_IS_ENABLED(MIPS)
/* add other arches which benefit from this... */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆಯುಳ್ಳ ಅಕ್ಷರ smallint;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ smalluint;
#ಇಲ್ಲದಿದ್ದರೆ
/* for arches where byte accesses generate larger code: */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಪೂರ್ಣಾಂಕ smallint;
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಚಿಹ್ನೆರಹಿತ smalluint;
#ಆದರೆ_ಅಂತ್ಯ

/*
 * Alignment defines used by BusyBox.
 */
#ವ್ಯಾಖ್ಯಾನಿಸು ALIGN1			__attribute__((aligned(1)))
#ವ್ಯಾಖ್ಯಾನಿಸು ALIGN2			__attribute__((aligned(2)))
#ವ್ಯಾಖ್ಯಾನಿಸು ALIGN4			__attribute__((aligned(4)))
#ವ್ಯಾಖ್ಯಾನಿಸು ALIGN8			__attribute__((aligned(8)))
#ವ್ಯಾಖ್ಯಾನಿಸು ALIGN_PTR		__attribute__((aligned(ಗಾತ್ರ(ಶೂನ್ಯ*))))

/*
 * Miscellaneous compiler/platform defines.
 */
#ವ್ಯಾಖ್ಯಾನಿಸು FAST_FUNC /* not used in U-Boot */
#ವ್ಯಾಖ್ಯಾನಿಸು ಬಳಸದ_PARAM		__always_unused
#ವ್ಯಾಖ್ಯಾನಿಸು ALWAYS_INLINE		__always_inline
#ವ್ಯಾಖ್ಯಾನಿಸು NOINLINE		noinline

/*
 * Defines to provide equivalents to what libc/BusyBox defines.
 */
#ವ್ಯಾಖ್ಯಾನಿಸು EOF			(-1)
#ವ್ಯಾಖ್ಯಾನಿಸು EXIT_SUCCESS		0
#ವ್ಯಾಖ್ಯಾನಿಸು EXIT_FAILURE		1

/*
 * Stubs to provide libc/BusyBox functions based on U-Boot equivalents where it
 * makes sense.
 */
#ವ್ಯಾಖ್ಯಾನಿಸು utoa			simple_itoa

ಸ್ಥಿರ ಶೂನ್ಯ __noreturn xfunc_die(ಶೂನ್ಯ)
{
	panic("HUSH died!");
}

#ವ್ಯಾಖ್ಯಾನಿಸು bb_error_ಸಂದೇಶ_and_die(format, ...) ಮಾಡು { \
panic("HUSH: " format, __VA_ARGS__); \
} ಆಗ (0);

#ವ್ಯಾಖ್ಯಾನಿಸು bb_simple_error_ಸಂದೇಶ_and_die(ಸಂದೇಶ) ಮಾಡು { \
panic_str("HUSH: " ಸಂದೇಶ); \
} ಆಗ (0);

/* fdprintf() is used for debug output. */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ __maybe_unused fdprintf(ಪೂರ್ಣಾಂಕ fd, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *format, ...)
{
	va_list ವಾದಗಳು;
	uint i;

	ಪ್ರತಿಜ್ಞಾ(fd == 2);

	va_ಆರಂಭ(ವಾದಗಳು, format);
	i = vprintf(format, ವಾದಗಳು);
	va_ಕೊನೆ(ವಾದಗಳು);

	ಹಿಂತಿರುಗಿ i;
}

ಸ್ಥಿರ ಶೂನ್ಯ bb_verror_ಸಂದೇಶ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s, va_list p, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* strerr)
{
	/* TODO: what to do with strerr arg? */
	vprintf(s, p);
}

ಸ್ಥಿರ ಶೂನ್ಯ bb_error_ಸಂದೇಶ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s, ...)
{
	va_list p;

	va_ಆರಂಭ(p, s);
	bb_verror_ಸಂದೇಶ(s, p, ಖಾಲಿ);
	va_ಕೊನೆ(p);
}

ಸ್ಥಿರ ಶೂನ್ಯ bb_simple_error_ಸಂದೇಶ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s)
{
	bb_error_ಸಂದೇಶ("%s", s);
}

ಸ್ಥಿರ ಶೂನ್ಯ *xmalloc(size_t ಗಾತ್ರ_2)
{
	ಶೂನ್ಯ *p = ಖಾಲಿ;
	ಆದರೆ (!(p = ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ_2)))
		panic("out of memory");
	ಹಿಂತಿರುಗಿ p;
}

ಸ್ಥಿರ ಶೂನ್ಯ *xzalloc(size_t ಗಾತ್ರ_2)
{
	ಶೂನ್ಯ *p = xmalloc(ಗಾತ್ರ_2);
	memset(p, 0, ಗಾತ್ರ_2);
	ಹಿಂತಿರುಗಿ p;
}

ಸ್ಥಿರ ಶೂನ್ಯ *xrealloc(ಶೂನ್ಯ *ptr, size_t ಗಾತ್ರ_2)
{
	ಶೂನ್ಯ *p = ಖಾಲಿ;
	ಆದರೆ (!(p = realloc(ptr, ಗಾತ್ರ_2)))
		panic("out of memory");
	ಹಿಂತಿರುಗಿ p;
}

ಸ್ಥಿರ ಶೂನ್ಯ *xmemdup(ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *s, ಪೂರ್ಣಾಂಕ n)
{
	ಹಿಂತಿರುಗಿ ಸ್ಮರಣೆ_ನಕಲು(xmalloc(n), s, n);
}

#ವ್ಯಾಖ್ಯಾನಿಸು xstrdup		strdup
#ವ್ಯಾಖ್ಯಾನಿಸು xstrndup	strndup

ಸ್ಥಿರ ಶೂನ್ಯ *mempcpy(ಶೂನ್ಯ *dest, ಸ್ಥಿರಾಂಕ ಶೂನ್ಯ *src, size_t len)
{
	ಹಿಂತಿರುಗಿ ಸ್ಮರಣೆ_ನಕಲು(dest, src, len) + len;
}

/* Like strcpy but can copy overlapping strings. */
ಸ್ಥಿರ ಶೂನ್ಯ overlapping_strcpy(ಅಕ್ಷರ *dst, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *src)
{
	/*
	 * Cheap optimization for dst == src case -
	 * better to have it here than in many callers.
	 */
	ಆದರೆ (dst != src) {
		ಆಗ ((*dst = *src) != '\0') {
			dst++;
			src++;
		}
	}
}

ಸ್ಥಿರ ಅಕ್ಷರ* skip_whitespace(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s)
{
	/*
	 * In POSIX/C locale (the only locale we care about: do we REALLY want
	 * to allow Unicode whitespace in, say, .conf files? nuts!)
	 * isspace is only these chars: "\t\n\v\f\r" and space.
	 * "\t\n\v\f\r" happen to have ASCII codes 9,10,11,12,13.
	 * Use that.
	 */
	ಆಗ (*s == ' ' || (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)(*s - 9) <= (13 - 9))
		s++;

	ಹಿಂತಿರುಗಿ (ಅಕ್ಷರ *) s;
}

ಸ್ಥಿರ ಅಕ್ಷರ* skip_non_whitespace(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s)
{
	ಆಗ (*s != '\0' && *s != ' ' && (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)(*s - 9) > (13 - 9))
		s++;

	ಹಿಂತಿರುಗಿ (ಅಕ್ಷರ *) s;
}

#ವ್ಯಾಖ್ಯಾನಿಸು is_name(c)	((c) == '_' || ಅಕ್ಷರವೇ((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)(c)))
#ವ್ಯಾಖ್ಯಾನಿಸು is_ಒಳ_name(c)	((c) == '_' || ಅಕ್ಷರವೇ_ಅಂಕಿಯೇ((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)(c)))

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* endofname(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name)
{
	ಆದರೆ (!is_name(*name))
		ಹಿಂತಿರುಗಿ name;
	ಆಗ (*++name) {
		ಆದರೆ (!is_ಒಳ_name(*name))
			ನಿಲ್ಲಿಸು;
	}
	ಹಿಂತಿರುಗಿ name;
}

/**
 * list_size() - returns the number of elements in char ** before NULL.
 *
 * Argument must contain NULL to signalize its end.
 *
 * @list The list to count the number of element.
 * @return The number of element in list.
 */
ಸ್ಥಿರ size_t list_ಗಾತ್ರ(ಅಕ್ಷರ **list)
{
	size_t ಗಾತ್ರ_2;

	ಗಾಗಿ (ಗಾತ್ರ_2 = 0; list[ಗಾತ್ರ_2] != ಖಾಲಿ; ಗಾತ್ರ_2++);

	ಹಿಂತಿರುಗಿ ಗಾತ್ರ_2;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ varcmp(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *q)
{
	ಪೂರ್ಣಾಂಕ c, d;

	ಆಗ ((c = *p) == (d = *q)) {
		ಆದರೆ (c == '\0' || c == '=')
			ಹೋಗು ಹೊರಹೋಗು;
		p++;
		q++;
	}
	ಆದರೆ (c == '=')
		c = '\0';
	ಆದರೆ (d == '=')
		d = '\0';
ಹೊರಹೋಗು:
	ಹಿಂತಿರುಗಿ c - d;
}

ರಚನೆ ಒಳ_str;
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ u_boot_cli_readline(ರಚನೆ ಒಳ_str *i);

ರಚನೆ ಒಳ_str;
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ u_boot_cli_readline(ರಚನೆ ಒಳ_str *i);

/*
 * BusyBox globals which are needed for hush.
 */
ಸ್ಥಿರ uint8_t xfunc_error_retval;

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ defifsvar[] __aligned(1) = "IFS= \t\n";
#ವ್ಯಾಖ್ಯಾನಿಸು defifs (defifsvar + 4)

/* This define is used to check if exit command was called. */
#ವ್ಯಾಖ್ಯಾನಿಸು EXIT_RET_ಸಂಕೇತ -2

/*
 * This define is used for changes that need be done directly in the upstream
 * sources still. Ideally, its use should be minimized as much as possible.
 */
#ವ್ಯಾಖ್ಯಾನಿಸು __U_BOOT__

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

ಪೂರ್ಣಾಂಕ u_boot_hush_ಆರಂಭ_modern(ಶೂನ್ಯ)
{
	INIT_G();
	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ u_boot_cli_readline(ರಚನೆ ಒಳ_str *i)
{
	ಅಕ್ಷರ *prompt;
	ಅಕ್ಷರ __maybe_unused *ps_prompt = ಖಾಲಿ;

	ಆದರೆ (!G.promptmode)
		prompt = CONFIG_SYS_PROMPT;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_SYS_PROMPT_HUSH_PS2
	ಇಲ್ಲದಿದ್ದರೆ
		prompt = CONFIG_SYS_PROMPT_HUSH_PS2;
#ಇಲ್ಲದಿದ್ದರೆ
	/* TODO: default value? */
	#ದೋಷ "SYS_PROMPT_HUSH_PS2 is not defined!"
#ಆದರೆ_ಅಂತ್ಯ

	ಆದರೆ (CONFIG_IS_ENABLED(CMDLINE_PS_SUPPORT)) {
		ಆದರೆ (!G.promptmode)
			ps_prompt = env_get("PS1");
		ಇಲ್ಲದಿದ್ದರೆ
			ps_prompt = env_get("PS2");

		ಆದರೆ (ps_prompt)
			prompt = ps_prompt;
	}

	ಹಿಂತಿರುಗಿ cli_readline(prompt);
}
