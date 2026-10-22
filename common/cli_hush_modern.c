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

#include <env.h>
#include <malloc.h>         /* malloc, free, realloc*/
#include <linux/ctype.h>    /* isalpha, isdigit */
#include <console.h>
#include <bootretry.h>
#include <cli.h>
#include <cli_hush.h>
#include <command.h>        /* find_cmd */
#include <asm/global_data.h>

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
#အကယ်၍ CONFIG_IS_ENABLED(X86) || CONFIG_IS_ENABLED(X86_64) || CONFIG_IS_ENABLED(MIPS)
/* add other arches which benefit from this... */
typedef signed var_na smallint;
typedef unsigned var_na smalluint;
#မဟုတ်ပါက
/* for arches where byte accesses generate larger code: */
typedef ကိန်းပြည့် smallint;
typedef unsigned smalluint;
#endif

/*
 * Alignment defines used by BusyBox.
 */
#define ALIGN1			__attribute__((aligned(1)))
#define ALIGN2			__attribute__((aligned(2)))
#define ALIGN4			__attribute__((aligned(4)))
#define ALIGN8			__attribute__((aligned(8)))
#define ALIGN_PTR		__attribute__((aligned(parimaa_na(void*))))

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

static void __noreturn xfunc_die(void)
{
	panic("HUSH died!");
}

#define bb_error_msg_and_die(format, ...) do { \
panic("HUSH: " format, __VA_ARGS__); \
} နေစဉ် (0);

#define bb_simple_error_msg_and_die(msg) do { \
panic_str("HUSH: " msg); \
} နေစဉ် (0);

/* fdprintf() is used for debug output. */
static ကိန်းပြည့် __maybe_unused fdprintf(ကိန်းပြည့် fd, const var_na *format, ...)
{
	va_list args;
	uint i;

	assert(fd == 2);

	va_start(args, format);
	i = vprintf(format, args);
	va_end(args);

	ပြန်ပေး i;
}

static void bb_verror_msg(const var_na *s, va_list p, const var_na* strerr)
{
	/* TODO: what to do with strerr arg? */
	vprintf(s, p);
}

static void bb_error_msg(const var_na *s, ...)
{
	va_list p;

	va_start(p, s);
	bb_verror_msg(s, p, NULL);
	va_end(p);
}

static void bb_simple_error_msg(const var_na *s)
{
	bb_error_msg("%s", s);
}

static void *xmalloc(size_t size)
{
	void *p = NULL;
	အကယ်၍ (!(p = malloc(size)))
		panic("out of memory");
	ပြန်ပေး p;
}

static void *xzalloc(size_t size)
{
	void *p = xmalloc(size);
	memset(p, 0, size);
	ပြန်ပေး p;
}

static void *xrealloc(void *ptr, size_t size)
{
	void *p = NULL;
	အကယ်၍ (!(p = realloc(ptr, size)))
		panic("out of memory");
	ပြန်ပေး p;
}

static void *xmemdup(const void *s, ကိန်းပြည့် n)
{
	ပြန်ပေး memcpy(xmalloc(n), s, n);
}

#define xstrdup		strdup
#define xstrndup	strndup

static void *mempcpy(void *dest, const void *src, size_t len)
{
	ပြန်ပေး memcpy(dest, src, len) + len;
}

/* Like strcpy but can copy overlapping strings. */
static void overlapping_strcpy(var_na *dst, const var_na *src)
{
	/*
	 * Cheap optimization for dst == src case -
	 * better to have it here than in many callers.
	 */
	အကယ်၍ (dst != src) {
		နေစဉ် ((*dst = *src) != '\0') {
			dst++;
			src++;
		}
	}
}

static var_na* skip_whitespace(const var_na *s)
{
	/*
	 * In POSIX/C locale (the only locale we care about: do we REALLY want
	 * to allow Unicode whitespace in, say, .conf files? nuts!)
	 * isspace is only these chars: "\t\n\v\f\r" and space.
	 * "\t\n\v\f\r" happen to have ASCII codes 9,10,11,12,13.
	 * Use that.
	 */
	နေစဉ် (*s == ' ' || (unsigned var_na)(*s - 9) <= (13 - 9))
		s++;

	ပြန်ပေး (var_na *) s;
}

static var_na* skip_non_whitespace(const var_na *s)
{
	နေစဉ် (*s != '\0' && *s != ' ' && (unsigned var_na)(*s - 9) > (13 - 9))
		s++;

	ပြန်ပေး (var_na *) s;
}

#define is_name(c)	((c) == '_' || isalpha((unsigned var_na)(c)))
#define is_in_name(c)	((c) == '_' || isalnum((unsigned var_na)(c)))

static const var_na* endofname(const var_na *name)
{
	အကယ်၍ (!is_name(*name))
		ပြန်ပေး name;
	နေစဉ် (*++name) {
		အကယ်၍ (!is_in_name(*name))
			bha_nga;
	}
	ပြန်ပေး name;
}

/**
 * list_size() - returns the number of elements in char ** before NULL.
 *
 * Argument must contain NULL to signalize its end.
 *
 * @list The list to count the number of element.
 * @return The number of element in list.
 */
static size_t list_size(var_na **list)
{
	size_t size;

	အတွက် (size = 0; list[size] != NULL; size++);

	ပြန်ပေး size;
}

static ကိန်းပြည့် varcmp(const var_na *p, const var_na *q)
{
	ကိန်းပြည့် c, d;

	နေစဉ် ((c = *p) == (d = *q)) {
		အကယ်၍ (c == '\0' || c == '=')
			goto out;
		p++;
		q++;
	}
	အကယ်၍ (c == '=')
		c = '\0';
	အကယ်၍ (d == '=')
		d = '\0';
out:
	ပြန်ပေး c - d;
}

sa_nracanaa in_str;
static ကိန်းပြည့် u_boot_cli_readline(sa_nracanaa in_str *i);

sa_nracanaa in_str;
static ကိန်းပြည့် u_boot_cli_readline(sa_nracanaa in_str *i);

/*
 * BusyBox globals which are needed for hush.
 */
static uint8_t xfunc_error_retval;

static const var_na defifsvar[] __aligned(1) = "IFS= \t\n";
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
#include "cli_hush_upstream.c"
/*
 * A                                     A
 * +-- Include of the upstream sources --+ *
 *
 */

ကိန်းပြည့် u_boot_hush_start_modern(void)
{
	INIT_G();
	ပြန်ပေး 0;
}

static ကိန်းပြည့် u_boot_cli_readline(sa_nracanaa in_str *i)
{
	var_na *prompt;
	var_na __maybe_unused *ps_prompt = NULL;

	အကယ်၍ (!G.promptmode)
		prompt = CONFIG_SYS_PROMPT;
#ifdef CONFIG_SYS_PROMPT_HUSH_PS2
	မဟုတ်ပါက
		prompt = CONFIG_SYS_PROMPT_HUSH_PS2;
#မဟုတ်ပါက
	/* TODO: default value? */
	#error "SYS_PROMPT_HUSH_PS2 is not defined!"
#endif

	အကယ်၍ (CONFIG_IS_ENABLED(CMDLINE_PS_SUPPORT)) {
		အကယ်၍ (!G.promptmode)
			ps_prompt = env_get("PS1");
		မဟုတ်ပါက
			ps_prompt = env_get("PS2");

		အကယ်၍ (ps_prompt)
			prompt = ps_prompt;
	}

	ပြန်ပေး cli_readline(prompt);
}
