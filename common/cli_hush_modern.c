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

#ਅਨ੍ਤਰ੍ਭਾਵ <env.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <malloc.h>         /* malloc, free, realloc*/
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/ctype.h>    /* isalpha, isdigit */
#ਅਨ੍ਤਰ੍ਭਾਵ <console.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <bootretry.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <cli.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <cli_hush.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <command.h>        /* find_cmd */
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/global_data.h>

/*
 * BusyBox Version: UPDATE THIS WHEN PULLING NEW UPSTREAM REVISION!
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BB_VER			"1.37.0.git23da5c4b716b"

/*
 * Define hush features by the names used upstream.
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ENABLE_HUSH_INTERACTIVE	1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ENABLE_FEATURE_EDITING	1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ENABLE_HUSH_IF		1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ENABLE_HUSH_LOOPS	1
/* No MMU in U-Boot */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BB_MMU			0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USE_FOR_NOMMU(...)	__VA_ARGS__
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ USE_FOR_MMU(...)

/*
 * Size-saving "small" ints (arch-dependent)
 */
#ਜੇ CONFIG_IS_ENABLED(X86) || CONFIG_IS_ENABLED(X86_64) || CONFIG_IS_ENABLED(MIPS)
/* add other arches which benefit from this... */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਨਿਸ਼ਾਨੀਵਾਲਾ ਅੱਖਰ smallint;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ smalluint;
#ਨਹੀਂਤਾਂ
/* for arches where byte accesses generate larger code: */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਪੂਰਨ smallint;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਿਨਾਨਿਸ਼ਾਨੀ smalluint;
#ਜੇ_ਅੰਤ

/*
 * Alignment defines used by BusyBox.
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ALIGN1			__attribute__((aligned(1)))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ALIGN2			__attribute__((aligned(2)))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ALIGN4			__attribute__((aligned(4)))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ALIGN8			__attribute__((aligned(8)))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ALIGN_PTR		__attribute__((aligned(ਆਕਾਰ(ਖਾਲੀ*))))

/*
 * Miscellaneous compiler/platform defines.
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FAST_FUNC /* not used in U-Boot */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਅਣਵਰਤਿਆ_PARAM		__always_unused
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ALWAYS_INLINE		__always_inline
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NOINLINE		noinline

/*
 * Defines to provide equivalents to what libc/BusyBox defines.
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EOF			(-1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EXIT_SUCCESS		0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EXIT_FAILURE		1

/*
 * Stubs to provide libc/BusyBox functions based on U-Boot equivalents where it
 * makes sense.
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ utoa			simple_itoa

ਸਥਿਰ ਖਾਲੀ __noreturn xfunc_die(ਖਾਲੀ)
{
	panic("HUSH died!");
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ bb_error_ਸੁਨੇਹਾ_and_die(format, ...) ਕਰੋ { \
panic("HUSH: " format, __VA_ARGS__); \
} ਜਦੋਂ (0);

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ bb_simple_error_ਸੁਨੇਹਾ_and_die(ਸੁਨੇਹਾ) ਕਰੋ { \
panic_str("HUSH: " ਸੁਨੇਹਾ); \
} ਜਦੋਂ (0);

/* fdprintf() is used for debug output. */
ਸਥਿਰ ਪੂਰਨ __maybe_unused fdprintf(ਪੂਰਨ fd, ਅਚਲ ਅੱਖਰ *format, ...)
{
	va_list ਦਲੀਲਾਂ;
	uint i;

	ਪ੍ਰਤਿਜ੍ਞਾ(fd == 2);

	va_ਸ਼ੁਰੂ(ਦਲੀਲਾਂ, format);
	i = vprintf(format, ਦਲੀਲਾਂ);
	va_ਸਿਰਾ(ਦਲੀਲਾਂ);

	ਵਾਪਸ i;
}

ਸਥਿਰ ਖਾਲੀ bb_verror_ਸੁਨੇਹਾ(ਅਚਲ ਅੱਖਰ *s, va_list p, ਅਚਲ ਅੱਖਰ* strerr)
{
	/* TODO: what to do with strerr arg? */
	vprintf(s, p);
}

ਸਥਿਰ ਖਾਲੀ bb_error_ਸੁਨੇਹਾ(ਅਚਲ ਅੱਖਰ *s, ...)
{
	va_list p;

	va_ਸ਼ੁਰੂ(p, s);
	bb_verror_ਸੁਨੇਹਾ(s, p, ਨਿਰਾ);
	va_ਸਿਰਾ(p);
}

ਸਥਿਰ ਖਾਲੀ bb_simple_error_ਸੁਨੇਹਾ(ਅਚਲ ਅੱਖਰ *s)
{
	bb_error_ਸੁਨੇਹਾ("%s", s);
}

ਸਥਿਰ ਖਾਲੀ *xmalloc(size_t ਮਾਪ)
{
	ਖਾਲੀ *p = ਨਿਰਾ;
	ਜੇ (!(p = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਮਾਪ)))
		panic("out of memory");
	ਵਾਪਸ p;
}

ਸਥਿਰ ਖਾਲੀ *xzalloc(size_t ਮਾਪ)
{
	ਖਾਲੀ *p = xmalloc(ਮਾਪ);
	memset(p, 0, ਮਾਪ);
	ਵਾਪਸ p;
}

ਸਥਿਰ ਖਾਲੀ *xrealloc(ਖਾਲੀ *ptr, size_t ਮਾਪ)
{
	ਖਾਲੀ *p = ਨਿਰਾ;
	ਜੇ (!(p = realloc(ptr, ਮਾਪ)))
		panic("out of memory");
	ਵਾਪਸ p;
}

ਸਥਿਰ ਖਾਲੀ *xmemdup(ਅਚਲ ਖਾਲੀ *s, ਪੂਰਨ n)
{
	ਵਾਪਸ ਸਮ੍ਰਿਤੀ_ਨਕਲ(xmalloc(n), s, n);
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ xstrdup		strdup
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ xstrndup	strndup

ਸਥਿਰ ਖਾਲੀ *mempcpy(ਖਾਲੀ *dest, ਅਚਲ ਖਾਲੀ *src, size_t len)
{
	ਵਾਪਸ ਸਮ੍ਰਿਤੀ_ਨਕਲ(dest, src, len) + len;
}

/* Like strcpy but can copy overlapping strings. */
ਸਥਿਰ ਖਾਲੀ overlapping_strcpy(ਅੱਖਰ *dst, ਅਚਲ ਅੱਖਰ *src)
{
	/*
	 * Cheap optimization for dst == src case -
	 * better to have it here than in many callers.
	 */
	ਜੇ (dst != src) {
		ਜਦੋਂ ((*dst = *src) != '\0') {
			dst++;
			src++;
		}
	}
}

ਸਥਿਰ ਅੱਖਰ* skip_whitespace(ਅਚਲ ਅੱਖਰ *s)
{
	/*
	 * In POSIX/C locale (the only locale we care about: do we REALLY want
	 * to allow Unicode whitespace in, say, .conf files? nuts!)
	 * isspace is only these chars: "\t\n\v\f\r" and space.
	 * "\t\n\v\f\r" happen to have ASCII codes 9,10,11,12,13.
	 * Use that.
	 */
	ਜਦੋਂ (*s == ' ' || (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)(*s - 9) <= (13 - 9))
		s++;

	ਵਾਪਸ (ਅੱਖਰ *) s;
}

ਸਥਿਰ ਅੱਖਰ* skip_non_whitespace(ਅਚਲ ਅੱਖਰ *s)
{
	ਜਦੋਂ (*s != '\0' && *s != ' ' && (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)(*s - 9) > (13 - 9))
		s++;

	ਵਾਪਸ (ਅੱਖਰ *) s;
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ is_name(c)	((c) == '_' || ਅੱਖਰ_ਹੈ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)(c)))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ is_ਵਿੱਚ_name(c)	((c) == '_' || ਅੱਖਰ_ਜਾਂ_ਅੰਕ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)(c)))

ਸਥਿਰ ਅਚਲ ਅੱਖਰ* endofname(ਅਚਲ ਅੱਖਰ *name)
{
	ਜੇ (!is_name(*name))
		ਵਾਪਸ name;
	ਜਦੋਂ (*++name) {
		ਜੇ (!is_ਵਿੱਚ_name(*name))
			ਰੋਕੋ;
	}
	ਵਾਪਸ name;
}

/**
 * list_size() - returns the number of elements in char ** before NULL.
 *
 * Argument must contain NULL to signalize its end.
 *
 * @list The list to count the number of element.
 * @return The number of element in list.
 */
ਸਥਿਰ size_t list_ਮਾਪ(ਅੱਖਰ **list)
{
	size_t ਮਾਪ;

	ਲਈ (ਮਾਪ = 0; list[ਮਾਪ] != ਨਿਰਾ; ਮਾਪ++);

	ਵਾਪਸ ਮਾਪ;
}

ਸਥਿਰ ਪੂਰਨ varcmp(ਅਚਲ ਅੱਖਰ *p, ਅਚਲ ਅੱਖਰ *q)
{
	ਪੂਰਨ c, d;

	ਜਦੋਂ ((c = *p) == (d = *q)) {
		ਜੇ (c == '\0' || c == '=')
			ਜਾਓ ਬਾਹਰਲਾ;
		p++;
		q++;
	}
	ਜੇ (c == '=')
		c = '\0';
	ਜੇ (d == '=')
		d = '\0';
ਬਾਹਰਲਾ:
	ਵਾਪਸ c - d;
}

ਬਣਤਰ ਵਿੱਚ_str;
ਸਥਿਰ ਪੂਰਨ u_boot_cli_readline(ਬਣਤਰ ਵਿੱਚ_str *i);

ਬਣਤਰ ਵਿੱਚ_str;
ਸਥਿਰ ਪੂਰਨ u_boot_cli_readline(ਬਣਤਰ ਵਿੱਚ_str *i);

/*
 * BusyBox globals which are needed for hush.
 */
ਸਥਿਰ uint8_t xfunc_error_retval;

ਸਥਿਰ ਅਚਲ ਅੱਖਰ defifsvar[] __aligned(1) = "IFS= \t\n";
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ defifs (defifsvar + 4)

/* This define is used to check if exit command was called. */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EXIT_RET_ਕੋਡ -2

/*
 * This define is used for changes that need be done directly in the upstream
 * sources still. Ideally, its use should be minimized as much as possible.
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __U_BOOT__

/*
 *
 * +-- Include of the upstream sources --+ *
 * V                                     V
 */
#ਅਨ੍ਤਰ੍ਭਾਵ "cli_hush_upstream.c"
/*
 * A                                     A
 * +-- Include of the upstream sources --+ *
 *
 */

ਪੂਰਨ u_boot_hush_ਸ਼ੁਰੂ_modern(ਖਾਲੀ)
{
	INIT_G();
	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ u_boot_cli_readline(ਬਣਤਰ ਵਿੱਚ_str *i)
{
	ਅੱਖਰ *prompt;
	ਅੱਖਰ __maybe_unused *ps_prompt = ਨਿਰਾ;

	ਜੇ (!G.promptmode)
		prompt = CONFIG_SYS_PROMPT;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SYS_PROMPT_HUSH_PS2
	ਨਹੀਂਤਾਂ
		prompt = CONFIG_SYS_PROMPT_HUSH_PS2;
#ਨਹੀਂਤਾਂ
	/* TODO: default value? */
	#ਗਲਤੀ "SYS_PROMPT_HUSH_PS2 is not defined!"
#ਜੇ_ਅੰਤ

	ਜੇ (CONFIG_IS_ENABLED(CMDLINE_PS_SUPPORT)) {
		ਜੇ (!G.promptmode)
			ps_prompt = env_get("PS1");
		ਨਹੀਂਤਾਂ
			ps_prompt = env_get("PS2");

		ਜੇ (ps_prompt)
			prompt = ps_prompt;
	}

	ਵਾਪਸ cli_readline(prompt);
}
