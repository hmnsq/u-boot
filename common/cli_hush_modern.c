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

#ଅନ୍ତର୍ଭାଵ <env.h>
#ଅନ୍ତର୍ଭାଵ <malloc.h>         /* malloc, free, realloc*/
#ଅନ୍ତର୍ଭାଵ <linux/ctype.h>    /* isalpha, isdigit */
#ଅନ୍ତର୍ଭାଵ <console.h>
#ଅନ୍ତର୍ଭାଵ <bootretry.h>
#ଅନ୍ତର୍ଭାଵ <cli.h>
#ଅନ୍ତର୍ଭାଵ <cli_hush.h>
#ଅନ୍ତର୍ଭାଵ <command.h>        /* find_cmd */
#ଅନ୍ତର୍ଭାଵ <asm/global_data.h>

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
#ଯଦି CONFIG_IS_ENABLED(X86) || CONFIG_IS_ENABLED(X86_64) || CONFIG_IS_ENABLED(MIPS)
/* add other arches which benefit from this... */
ପ୍ରକାରବ୍ୟାଖ୍ୟା signed ଅକ୍ଷର smallint;
ପ୍ରକାରବ୍ୟାଖ୍ୟା unsigned ଅକ୍ଷର smalluint;
#ଅନ୍ୟଥା
/* for arches where byte accesses generate larger code: */
ପ୍ରକାରବ୍ୟାଖ୍ୟା ପୂର୍ଣ୍ଣସଂଖ୍ୟା smallint;
ପ୍ରକାରବ୍ୟାଖ୍ୟା unsigned smalluint;
#endif

/*
 * Alignment defines used by BusyBox.
 */
#define ALIGN1			__attribute__((aligned(1)))
#define ALIGN2			__attribute__((aligned(2)))
#define ALIGN4			__attribute__((aligned(4)))
#define ALIGN8			__attribute__((aligned(8)))
#define ALIGN_PTR		__attribute__((aligned(ଆକାର(ଶୂନ୍ୟ*))))

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

ସ୍ଥିର ଶୂନ୍ୟ __noreturn xfunc_die(ଶୂନ୍ୟ)
{
	panic("HUSH died!");
}

#define bb_error_ସନ୍ଦେଶ_and_die(format, ...) କର { \
panic("HUSH: " format, __VA_ARGS__); \
} ଯେତେବେଳେ (0);

#define bb_simple_error_ସନ୍ଦେଶ_and_die(ସନ୍ଦେଶ) କର { \
panic_str("HUSH: " ସନ୍ଦେଶ); \
} ଯେତେବେଳେ (0);

/* fdprintf() is used for debug output. */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା __maybe_unused fdprintf(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd, ଅଚଳ ଅକ୍ଷର *format, ...)
{
	va_list args;
	uint i;

	ପ୍ରତିଜ୍ଞା(fd == 2);

	va_start(args, format);
	i = vprintf(format, args);
	va_end(args);

	ଫେରନ୍ତୁ i;
}

ସ୍ଥିର ଶୂନ୍ୟ bb_verror_ସନ୍ଦେଶ(ଅଚଳ ଅକ୍ଷର *s, va_list p, ଅଚଳ ଅକ୍ଷର* strerr)
{
	/* TODO: what to do with strerr arg? */
	vprintf(s, p);
}

ସ୍ଥିର ଶୂନ୍ୟ bb_error_ସନ୍ଦେଶ(ଅଚଳ ଅକ୍ଷର *s, ...)
{
	va_list p;

	va_start(p, s);
	bb_verror_ସନ୍ଦେଶ(s, p, ଖାଲି);
	va_end(p);
}

ସ୍ଥିର ଶୂନ୍ୟ bb_simple_error_ସନ୍ଦେଶ(ଅଚଳ ଅକ୍ଷର *s)
{
	bb_error_ସନ୍ଦେଶ("%s", s);
}

ସ୍ଥିର ଶୂନ୍ୟ *xmalloc(size_t size)
{
	ଶୂନ୍ୟ *p = ଖାଲି;
	ଯଦି (!(p = malloc(size)))
		panic("out of memory");
	ଫେରନ୍ତୁ p;
}

ସ୍ଥିର ଶୂନ୍ୟ *xzalloc(size_t size)
{
	ଶୂନ୍ୟ *p = xmalloc(size);
	memset(p, 0, size);
	ଫେରନ୍ତୁ p;
}

ସ୍ଥିର ଶୂନ୍ୟ *xrealloc(ଶୂନ୍ୟ *ptr, size_t size)
{
	ଶୂନ୍ୟ *p = ଖାଲି;
	ଯଦି (!(p = realloc(ptr, size)))
		panic("out of memory");
	ଫେରନ୍ତୁ p;
}

ସ୍ଥିର ଶୂନ୍ୟ *xmemdup(ଅଚଳ ଶୂନ୍ୟ *s, ପୂର୍ଣ୍ଣସଂଖ୍ୟା n)
{
	ଫେରନ୍ତୁ memcpy(xmalloc(n), s, n);
}

#define xstrdup		strdup
#define xstrndup	strndup

ସ୍ଥିର ଶୂନ୍ୟ *mempcpy(ଶୂନ୍ୟ *dest, ଅଚଳ ଶୂନ୍ୟ *src, size_t len)
{
	ଫେରନ୍ତୁ memcpy(dest, src, len) + len;
}

/* Like strcpy but can copy overlapping strings. */
ସ୍ଥିର ଶୂନ୍ୟ overlapping_strcpy(ଅକ୍ଷର *dst, ଅଚଳ ଅକ୍ଷର *src)
{
	/*
	 * Cheap optimization for dst == src case -
	 * better to have it here than in many callers.
	 */
	ଯଦି (dst != src) {
		ଯେତେବେଳେ ((*dst = *src) != '\0') {
			dst++;
			src++;
		}
	}
}

ସ୍ଥିର ଅକ୍ଷର* skip_whitespace(ଅଚଳ ଅକ୍ଷର *s)
{
	/*
	 * In POSIX/C locale (the only locale we care about: do we REALLY want
	 * to allow Unicode whitespace in, say, .conf files? nuts!)
	 * isspace is only these chars: "\t\n\v\f\r" and space.
	 * "\t\n\v\f\r" happen to have ASCII codes 9,10,11,12,13.
	 * Use that.
	 */
	ଯେତେବେଳେ (*s == ' ' || (unsigned ଅକ୍ଷର)(*s - 9) <= (13 - 9))
		s++;

	ଫେରନ୍ତୁ (ଅକ୍ଷର *) s;
}

ସ୍ଥିର ଅକ୍ଷର* skip_non_whitespace(ଅଚଳ ଅକ୍ଷର *s)
{
	ଯେତେବେଳେ (*s != '\0' && *s != ' ' && (unsigned ଅକ୍ଷର)(*s - 9) > (13 - 9))
		s++;

	ଫେରନ୍ତୁ (ଅକ୍ଷର *) s;
}

#define is_name(c)	((c) == '_' || isalpha((unsigned ଅକ୍ଷର)(c)))
#define is_in_name(c)	((c) == '_' || isalnum((unsigned ଅକ୍ଷର)(c)))

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର* endofname(ଅଚଳ ଅକ୍ଷର *name)
{
	ଯଦି (!is_name(*name))
		ଫେରନ୍ତୁ name;
	ଯେତେବେଳେ (*++name) {
		ଯଦି (!is_in_name(*name))
			ଭାଙ୍ଗ;
	}
	ଫେରନ୍ତୁ name;
}

/**
 * list_size() - returns the number of elements in char ** before NULL.
 *
 * Argument must contain NULL to signalize its end.
 *
 * @list The list to count the number of element.
 * @return The number of element in list.
 */
ସ୍ଥିର size_t list_size(ଅକ୍ଷର **list)
{
	size_t size;

	ପାଇଁ (size = 0; list[size] != ଖାଲି; size++);

	ଫେରନ୍ତୁ size;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା varcmp(ଅଚଳ ଅକ୍ଷର *p, ଅଚଳ ଅକ୍ଷର *q)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା c, d;

	ଯେତେବେଳେ ((c = *p) == (d = *q)) {
		ଯଦି (c == '\0' || c == '=')
			ଯାଅ out;
		p++;
		q++;
	}
	ଯଦି (c == '=')
		c = '\0';
	ଯଦି (d == '=')
		d = '\0';
out:
	ଫେରନ୍ତୁ c - d;
}

ସଂରଚନା in_str;
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା u_boot_cli_readline(ସଂରଚନା in_str *i);

ସଂରଚନା in_str;
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା u_boot_cli_readline(ସଂରଚନା in_str *i);

/*
 * BusyBox globals which are needed for hush.
 */
ସ୍ଥିର uint8_t xfunc_error_retval;

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର defifsvar[] __aligned(1) = "IFS= \t\n";
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
#ଅନ୍ତର୍ଭାଵ "cli_hush_upstream.c"
/*
 * A                                     A
 * +-- Include of the upstream sources --+ *
 *
 */

ପୂର୍ଣ୍ଣସଂଖ୍ୟା u_boot_hush_start_modern(ଶୂନ୍ୟ)
{
	INIT_G();
	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା u_boot_cli_readline(ସଂରଚନା in_str *i)
{
	ଅକ୍ଷର *prompt;
	ଅକ୍ଷର __maybe_unused *ps_prompt = ଖାଲି;

	ଯଦି (!G.promptmode)
		prompt = CONFIG_SYS_PROMPT;
#ifdef CONFIG_SYS_PROMPT_HUSH_PS2
	ଅନ୍ୟଥା
		prompt = CONFIG_SYS_PROMPT_HUSH_PS2;
#ଅନ୍ୟଥା
	/* TODO: default value? */
	#error "SYS_PROMPT_HUSH_PS2 is not defined!"
#endif

	ଯଦି (CONFIG_IS_ENABLED(CMDLINE_PS_SUPPORT)) {
		ଯଦି (!G.promptmode)
			ps_prompt = env_get("PS1");
		ଅନ୍ୟଥା
			ps_prompt = env_get("PS2");

		ଯଦି (ps_prompt)
			prompt = ps_prompt;
	}

	ଫେରନ୍ତୁ cli_readline(prompt);
}
