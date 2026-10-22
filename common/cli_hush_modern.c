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

#অন্তর্ভাব <env.h>
#অন্তর্ভাব <malloc.h>         /* malloc, free, realloc*/
#অন্তর্ভাব <linux/ctype.h>    /* isalpha, isdigit */
#অন্তর্ভাব <console.h>
#অন্তর্ভাব <bootretry.h>
#অন্তর্ভাব <cli.h>
#অন্তর্ভাব <cli_hush.h>
#অন্তর্ভাব <command.h>        /* find_cmd */
#অন্তর্ভাব <asm/global_data.h>

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
প্রকারলেখ signed অক্ষর smallint;
প্রকারলেখ unsigned অক্ষর smalluint;
#অন্যথা
/* for arches where byte accesses generate larger code: */
প্রকারলেখ পূর্ণসংখ্যা smallint;
প্রকারলেখ unsigned smalluint;
#endif

/*
 * Alignment defines used by BusyBox.
 */
#define ALIGN1			__attribute__((aligned(1)))
#define ALIGN2			__attribute__((aligned(2)))
#define ALIGN4			__attribute__((aligned(4)))
#define ALIGN8			__attribute__((aligned(8)))
#define ALIGN_PTR		__attribute__((aligned(আকার(শূন্য*))))

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

#define bb_error_বার্তা_and_die(format, ...) করো { \
panic("HUSH: " format, __VA_ARGS__); \
} যখন (0);

#define bb_simple_error_বার্তা_and_die(বার্তা) করো { \
panic_str("HUSH: " বার্তা); \
} যখন (0);

/* fdprintf() is used for debug output. */
স্থির পূর্ণসংখ্যা __maybe_unused fdprintf(পূর্ণসংখ্যা fd, ধ্রুবক অক্ষর *format, ...)
{
	va_list args;
	uint i;

	প্রতিজ্ঞা(fd == 2);

	va_start(args, format);
	i = vprintf(format, args);
	va_end(args);

	ফেরত i;
}

স্থির শূন্য bb_verror_বার্তা(ধ্রুবক অক্ষর *s, va_list p, ধ্রুবক অক্ষর* strerr)
{
	/* TODO: what to do with strerr arg? */
	vprintf(s, p);
}

স্থির শূন্য bb_error_বার্তা(ধ্রুবক অক্ষর *s, ...)
{
	va_list p;

	va_start(p, s);
	bb_verror_বার্তা(s, p, ফাঁকা);
	va_end(p);
}

স্থির শূন্য bb_simple_error_বার্তা(ধ্রুবক অক্ষর *s)
{
	bb_error_বার্তা("%s", s);
}

স্থির শূন্য *xmalloc(size_t আকার_2)
{
	শূন্য *p = ফাঁকা;
	যদি (!(p = malloc(আকার_2)))
		panic("out of memory");
	ফেরত p;
}

স্থির শূন্য *xzalloc(size_t আকার_2)
{
	শূন্য *p = xmalloc(আকার_2);
	memset(p, 0, আকার_2);
	ফেরত p;
}

স্থির শূন্য *xrealloc(শূন্য *ptr, size_t আকার_2)
{
	শূন্য *p = ফাঁকা;
	যদি (!(p = realloc(ptr, আকার_2)))
		panic("out of memory");
	ফেরত p;
}

স্থির শূন্য *xmemdup(ধ্রুবক শূন্য *s, পূর্ণসংখ্যা n)
{
	ফেরত memcpy(xmalloc(n), s, n);
}

#define xstrdup		strdup
#define xstrndup	strndup

স্থির শূন্য *mempcpy(শূন্য *dest, ধ্রুবক শূন্য *src, size_t len)
{
	ফেরত memcpy(dest, src, len) + len;
}

/* Like strcpy but can copy overlapping strings. */
স্থির শূন্য overlapping_strcpy(অক্ষর *dst, ধ্রুবক অক্ষর *src)
{
	/*
	 * Cheap optimization for dst == src case -
	 * better to have it here than in many callers.
	 */
	যদি (dst != src) {
		যখন ((*dst = *src) != '\0') {
			dst++;
			src++;
		}
	}
}

স্থির অক্ষর* skip_whitespace(ধ্রুবক অক্ষর *s)
{
	/*
	 * In POSIX/C locale (the only locale we care about: do we REALLY want
	 * to allow Unicode whitespace in, say, .conf files? nuts!)
	 * isspace is only these chars: "\t\n\v\f\r" and space.
	 * "\t\n\v\f\r" happen to have ASCII codes 9,10,11,12,13.
	 * Use that.
	 */
	যখন (*s == ' ' || (unsigned অক্ষর)(*s - 9) <= (13 - 9))
		s++;

	ফেরত (অক্ষর *) s;
}

স্থির অক্ষর* skip_non_whitespace(ধ্রুবক অক্ষর *s)
{
	যখন (*s != '\0' && *s != ' ' && (unsigned অক্ষর)(*s - 9) > (13 - 9))
		s++;

	ফেরত (অক্ষর *) s;
}

#define is_name(c)	((c) == '_' || isalpha((unsigned অক্ষর)(c)))
#define is_in_name(c)	((c) == '_' || isalnum((unsigned অক্ষর)(c)))

স্থির ধ্রুবক অক্ষর* endofname(ধ্রুবক অক্ষর *name)
{
	যদি (!is_name(*name))
		ফেরত name;
	যখন (*++name) {
		যদি (!is_in_name(*name))
			বিরতি;
	}
	ফেরত name;
}

/**
 * list_size() - returns the number of elements in char ** before NULL.
 *
 * Argument must contain NULL to signalize its end.
 *
 * @list The list to count the number of element.
 * @return The number of element in list.
 */
স্থির size_t list_আকার(অক্ষর **list)
{
	size_t আকার_2;

	জন্য (আকার_2 = 0; list[আকার_2] != ফাঁকা; আকার_2++);

	ফেরত আকার_2;
}

স্থির পূর্ণসংখ্যা varcmp(ধ্রুবক অক্ষর *p, ধ্রুবক অক্ষর *q)
{
	পূর্ণসংখ্যা c, d;

	যখন ((c = *p) == (d = *q)) {
		যদি (c == '\0' || c == '=')
			যাও out;
		p++;
		q++;
	}
	যদি (c == '=')
		c = '\0';
	যদি (d == '=')
		d = '\0';
out:
	ফেরত c - d;
}

কাঠামো in_str;
স্থির পূর্ণসংখ্যা u_boot_cli_readline(কাঠামো in_str *i);

কাঠামো in_str;
স্থির পূর্ণসংখ্যা u_boot_cli_readline(কাঠামো in_str *i);

/*
 * BusyBox globals which are needed for hush.
 */
স্থির uint8_t xfunc_error_retval;

স্থির ধ্রুবক অক্ষর defifsvar[] __aligned(1) = "IFS= \t\n";
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
#অন্তর্ভাব "cli_hush_upstream.c"
/*
 * A                                     A
 * +-- Include of the upstream sources --+ *
 *
 */

পূর্ণসংখ্যা u_boot_hush_start_modern(শূন্য)
{
	INIT_G();
	ফেরত 0;
}

স্থির পূর্ণসংখ্যা u_boot_cli_readline(কাঠামো in_str *i)
{
	অক্ষর *prompt;
	অক্ষর __maybe_unused *ps_prompt = ফাঁকা;

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

	ফেরত cli_readline(prompt);
}
