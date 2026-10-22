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

#অন্তর্ভাৱ <env.h>
#অন্তর্ভাৱ <malloc.h>         /* malloc, free, realloc*/
#অন্তর্ভাৱ <linux/ctype.h>    /* isalpha, isdigit */
#অন্তর্ভাৱ <console.h>
#অন্তর্ভাৱ <bootretry.h>
#অন্তর্ভাৱ <cli.h>
#অন্তর্ভাৱ <cli_hush.h>
#অন্তর্ভাৱ <command.h>        /* find_cmd */
#অন্তর্ভাৱ <asm/global_data.h>

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
#যদি CONFIG_IS_ENABLED(X86) || CONFIG_IS_ENABLED(X86_64) || CONFIG_IS_ENABLED(MIPS)
/* add other arches which benefit from this... */
প্রকারনির্দেশ signed ৱর্ণ smallint;
প্রকারনির্দেশ unsigned ৱর্ণ smalluint;
#অন্যথা
/* for arches where byte accesses generate larger code: */
প্রকারনির্দেশ পূর্ণাঙ্ক smallint;
প্রকারনির্দেশ unsigned smalluint;
#endif

/*
 * Alignment defines used by BusyBox.
 */
#define ALIGN1			__attribute__((aligned(1)))
#define ALIGN2			__attribute__((aligned(2)))
#define ALIGN4			__attribute__((aligned(4)))
#define ALIGN8			__attribute__((aligned(8)))
#define ALIGN_PTR		__attribute__((aligned(পরিমাণ(শূন্য*))))

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

স্থির শূন্য __noreturn xfunc_die(শূন্য)
{
	panic("HUSH died!");
}

#define bb_error_বাৰ্তা_and_die(format, ...) কৰা { \
panic("HUSH: " format, __VA_ARGS__); \
} যেতিয়ালৈ (0);

#define bb_simple_error_বাৰ্তা_and_die(বাৰ্তা) কৰা { \
panic_str("HUSH: " বাৰ্তা); \
} যেতিয়ালৈ (0);

/* fdprintf() is used for debug output. */
স্থির পূর্ণাঙ্ক __maybe_unused fdprintf(পূর্ণাঙ্ক fd, ধ্রুৱ ৱর্ণ *format, ...)
{
	va_list args;
	uint i;

	প্রতিজ্ঞা(fd == 2);

	va_start(args, format);
	i = vprintf(format, args);
	va_end(args);

	ঘূৰাই i;
}

স্থির শূন্য bb_verror_বাৰ্তা(ধ্রুৱ ৱর্ণ *s, va_list p, ধ্রুৱ ৱর্ণ* strerr)
{
	/* TODO: what to do with strerr arg? */
	vprintf(s, p);
}

স্থির শূন্য bb_error_বাৰ্তা(ধ্রুৱ ৱর্ণ *s, ...)
{
	va_list p;

	va_start(p, s);
	bb_verror_বাৰ্তা(s, p, রিক্ত);
	va_end(p);
}

স্থির শূন্য bb_simple_error_বাৰ্তা(ধ্রুৱ ৱর্ণ *s)
{
	bb_error_বাৰ্তা("%s", s);
}

স্থির শূন্য *xmalloc(size_t size)
{
	শূন্য *p = রিক্ত;
	যদি (!(p = malloc(size)))
		panic("out of memory");
	ঘূৰাই p;
}

স্থির শূন্য *xzalloc(size_t size)
{
	শূন্য *p = xmalloc(size);
	memset(p, 0, size);
	ঘূৰাই p;
}

স্থির শূন্য *xrealloc(শূন্য *ptr, size_t size)
{
	শূন্য *p = রিক্ত;
	যদি (!(p = realloc(ptr, size)))
		panic("out of memory");
	ঘূৰাই p;
}

স্থির শূন্য *xmemdup(ধ্রুৱ শূন্য *s, পূর্ণাঙ্ক n)
{
	ঘূৰাই memcpy(xmalloc(n), s, n);
}

#define xstrdup		strdup
#define xstrndup	strndup

স্থির শূন্য *mempcpy(শূন্য *dest, ধ্রুৱ শূন্য *src, size_t len)
{
	ঘূৰাই memcpy(dest, src, len) + len;
}

/* Like strcpy but can copy overlapping strings. */
স্থির শূন্য overlapping_strcpy(ৱর্ণ *dst, ধ্রুৱ ৱর্ণ *src)
{
	/*
	 * Cheap optimization for dst == src case -
	 * better to have it here than in many callers.
	 */
	যদি (dst != src) {
		যেতিয়ালৈ ((*dst = *src) != '\0') {
			dst++;
			src++;
		}
	}
}

স্থির ৱর্ণ* skip_whitespace(ধ্রুৱ ৱর্ণ *s)
{
	/*
	 * In POSIX/C locale (the only locale we care about: do we REALLY want
	 * to allow Unicode whitespace in, say, .conf files? nuts!)
	 * isspace is only these chars: "\t\n\v\f\r" and space.
	 * "\t\n\v\f\r" happen to have ASCII codes 9,10,11,12,13.
	 * Use that.
	 */
	যেতিয়ালৈ (*s == ' ' || (unsigned ৱর্ণ)(*s - 9) <= (13 - 9))
		s++;

	ঘূৰাই (ৱর্ণ *) s;
}

স্থির ৱর্ণ* skip_non_whitespace(ধ্রুৱ ৱর্ণ *s)
{
	যেতিয়ালৈ (*s != '\0' && *s != ' ' && (unsigned ৱর্ণ)(*s - 9) > (13 - 9))
		s++;

	ঘূৰাই (ৱর্ণ *) s;
}

#define is_name(c)	((c) == '_' || isalpha((unsigned ৱর্ণ)(c)))
#define is_in_name(c)	((c) == '_' || isalnum((unsigned ৱর্ণ)(c)))

স্থির ধ্রুৱ ৱর্ণ* endofname(ধ্রুৱ ৱর্ণ *name)
{
	যদি (!is_name(*name))
		ঘূৰাই name;
	যেতিয়ালৈ (*++name) {
		যদি (!is_in_name(*name))
			ভঙ্গ;
	}
	ঘূৰাই name;
}

/**
 * list_size() - returns the number of elements in char ** before NULL.
 *
 * Argument must contain NULL to signalize its end.
 *
 * @list The list to count the number of element.
 * @return The number of element in list.
 */
স্থির size_t list_size(ৱর্ণ **list)
{
	size_t size;

	বাবে (size = 0; list[size] != রিক্ত; size++);

	ঘূৰাই size;
}

স্থির পূর্ণাঙ্ক varcmp(ধ্রুৱ ৱর্ণ *p, ধ্রুৱ ৱর্ণ *q)
{
	পূর্ণাঙ্ক c, d;

	যেতিয়ালৈ ((c = *p) == (d = *q)) {
		যদি (c == '\0' || c == '=')
			গচ্ছ out;
		p++;
		q++;
	}
	যদি (c == '=')
		c = '\0';
	যদি (d == '=')
		d = '\0';
out:
	ঘূৰাই c - d;
}

সংরচনা in_str;
স্থির পূর্ণাঙ্ক u_boot_cli_readline(সংরচনা in_str *i);

সংরচনা in_str;
স্থির পূর্ণাঙ্ক u_boot_cli_readline(সংরচনা in_str *i);

/*
 * BusyBox globals which are needed for hush.
 */
স্থির uint8_t xfunc_error_retval;

স্থির ধ্রুৱ ৱর্ণ defifsvar[] __aligned(1) = "IFS= \t\n";
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
#অন্তর্ভাৱ "cli_hush_upstream.c"
/*
 * A                                     A
 * +-- Include of the upstream sources --+ *
 *
 */

পূর্ণাঙ্ক u_boot_hush_start_modern(শূন্য)
{
	INIT_G();
	ঘূৰাই 0;
}

স্থির পূর্ণাঙ্ক u_boot_cli_readline(সংরচনা in_str *i)
{
	ৱর্ণ *prompt;
	ৱর্ণ __maybe_unused *ps_prompt = রিক্ত;

	যদি (!G.promptmode)
		prompt = CONFIG_SYS_PROMPT;
#ifdef CONFIG_SYS_PROMPT_HUSH_PS2
	অন্যথা
		prompt = CONFIG_SYS_PROMPT_HUSH_PS2;
#অন্যথা
	/* TODO: default value? */
	#error "SYS_PROMPT_HUSH_PS2 is not defined!"
#endif

	যদি (CONFIG_IS_ENABLED(CMDLINE_PS_SUPPORT)) {
		যদি (!G.promptmode)
			ps_prompt = env_get("PS1");
		অন্যথা
			ps_prompt = env_get("PS2");

		যদি (ps_prompt)
			prompt = ps_prompt;
	}

	ঘূৰাই cli_readline(prompt);
}
