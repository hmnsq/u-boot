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

#شامل <env.h>
#شامل <malloc.h>         /* malloc, free, realloc*/
#شامل <linux/ctype.h>    /* isalpha, isdigit */
#شامل <console.h>
#شامل <bootretry.h>
#شامل <cli.h>
#شامل <cli_hush.h>
#شامل <command.h>        /* find_cmd */
#شامل <asm/global_data.h>

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
#اگر CONFIG_IS_ENABLED(X86) || CONFIG_IS_ENABLED(X86_64) || CONFIG_IS_ENABLED(MIPS)
/* add other arches which benefit from this... */
تعریفنوع signed نویسه smallint;
تعریفنوع unsigned نویسه smalluint;
#وگرنه
/* for arches where byte accesses generate larger code: */
تعریفنوع صحیح smallint;
تعریفنوع unsigned smalluint;
#endif

/*
 * Alignment defines used by BusyBox.
 */
#define ALIGN1			__attribute__((aligned(1)))
#define ALIGN2			__attribute__((aligned(2)))
#define ALIGN4			__attribute__((aligned(4)))
#define ALIGN8			__attribute__((aligned(8)))
#define ALIGN_PTR		__attribute__((aligned(اندازه(تهی*))))

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

ایستا تهی __noreturn xfunc_die(تهی)
{
	panic("HUSH died!");
}

#define bb_error_پیام_and_die(format, ...) انجامبده { \
panic("HUSH: " format, __VA_ARGS__); \
} وقتی (0);

#define bb_simple_error_پیام_and_die(پیام) انجامبده { \
panic_str("HUSH: " پیام); \
} وقتی (0);

/* fdprintf() is used for debug output. */
ایستا صحیح __maybe_unused fdprintf(صحیح fd, ثابت نویسه *format, ...)
{
	va_list args;
	uint i;

	أكد(fd == 2);

	va_start(args, format);
	i = vprintf(format, args);
	va_end(args);

	بازگشت i;
}

ایستا تهی bb_verror_پیام(ثابت نویسه *s, va_list p, ثابت نویسه* strerr)
{
	/* TODO: what to do with strerr arg? */
	vprintf(s, p);
}

ایستا تهی bb_error_پیام(ثابت نویسه *s, ...)
{
	va_list p;

	va_start(p, s);
	bb_verror_پیام(s, p, پوچ);
	va_end(p);
}

ایستا تهی bb_simple_error_پیام(ثابت نویسه *s)
{
	bb_error_پیام("%s", s);
}

ایستا تهی *xmalloc(size_t size)
{
	تهی *p = پوچ;
	اگر (!(p = malloc(size)))
		panic("out of memory");
	بازگشت p;
}

ایستا تهی *xzalloc(size_t size)
{
	تهی *p = xmalloc(size);
	memset(p, 0, size);
	بازگشت p;
}

ایستا تهی *xrealloc(تهی *ptr, size_t size)
{
	تهی *p = پوچ;
	اگر (!(p = realloc(ptr, size)))
		panic("out of memory");
	بازگشت p;
}

ایستا تهی *xmemdup(ثابت تهی *s, صحیح n)
{
	بازگشت memcpy(xmalloc(n), s, n);
}

#define xstrdup		strdup
#define xstrndup	strndup

ایستا تهی *mempcpy(تهی *dest, ثابت تهی *src, size_t len)
{
	بازگشت memcpy(dest, src, len) + len;
}

/* Like strcpy but can copy overlapping strings. */
ایستا تهی overlapping_strcpy(نویسه *dst, ثابت نویسه *src)
{
	/*
	 * Cheap optimization for dst == src case -
	 * better to have it here than in many callers.
	 */
	اگر (dst != src) {
		وقتی ((*dst = *src) != '\0') {
			dst++;
			src++;
		}
	}
}

ایستا نویسه* skip_whitespace(ثابت نویسه *s)
{
	/*
	 * In POSIX/C locale (the only locale we care about: do we REALLY want
	 * to allow Unicode whitespace in, say, .conf files? nuts!)
	 * isspace is only these chars: "\t\n\v\f\r" and space.
	 * "\t\n\v\f\r" happen to have ASCII codes 9,10,11,12,13.
	 * Use that.
	 */
	وقتی (*s == ' ' || (unsigned نویسه)(*s - 9) <= (13 - 9))
		s++;

	بازگشت (نویسه *) s;
}

ایستا نویسه* skip_non_whitespace(ثابت نویسه *s)
{
	وقتی (*s != '\0' && *s != ' ' && (unsigned نویسه)(*s - 9) > (13 - 9))
		s++;

	بازگشت (نویسه *) s;
}

#define is_name(c)	((c) == '_' || isalpha((unsigned نویسه)(c)))
#define is_in_name(c)	((c) == '_' || isalnum((unsigned نویسه)(c)))

ایستا ثابت نویسه* endofname(ثابت نویسه *name)
{
	اگر (!is_name(*name))
		بازگشت name;
	وقتی (*++name) {
		اگر (!is_in_name(*name))
			بشکن;
	}
	بازگشت name;
}

/**
 * list_size() - returns the number of elements in char ** before NULL.
 *
 * Argument must contain NULL to signalize its end.
 *
 * @list The list to count the number of element.
 * @return The number of element in list.
 */
ایستا size_t list_size(نویسه **list)
{
	size_t size;

	برای (size = 0; list[size] != پوچ; size++);

	بازگشت size;
}

ایستا صحیح varcmp(ثابت نویسه *p, ثابت نویسه *q)
{
	صحیح c, d;

	وقتی ((c = *p) == (d = *q)) {
		اگر (c == '\0' || c == '=')
			برو out;
		p++;
		q++;
	}
	اگر (c == '=')
		c = '\0';
	اگر (d == '=')
		d = '\0';
out:
	بازگشت c - d;
}

ساختار in_str;
ایستا صحیح u_boot_cli_readline(ساختار in_str *i);

ساختار in_str;
ایستا صحیح u_boot_cli_readline(ساختار in_str *i);

/*
 * BusyBox globals which are needed for hush.
 */
ایستا uint8_t xfunc_error_retval;

ایستا ثابت نویسه defifsvar[] __aligned(1) = "IFS= \t\n";
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
#شامل "cli_hush_upstream.c"
/*
 * A                                     A
 * +-- Include of the upstream sources --+ *
 *
 */

صحیح u_boot_hush_start_modern(تهی)
{
	INIT_G();
	بازگشت 0;
}

ایستا صحیح u_boot_cli_readline(ساختار in_str *i)
{
	نویسه *prompt;
	نویسه __maybe_unused *ps_prompt = پوچ;

	اگر (!G.promptmode)
		prompt = CONFIG_SYS_PROMPT;
#ifdef CONFIG_SYS_PROMPT_HUSH_PS2
	وگرنه
		prompt = CONFIG_SYS_PROMPT_HUSH_PS2;
#وگرنه
	/* TODO: default value? */
	#error "SYS_PROMPT_HUSH_PS2 is not defined!"
#endif

	اگر (CONFIG_IS_ENABLED(CMDLINE_PS_SUPPORT)) {
		اگر (!G.promptmode)
			ps_prompt = env_get("PS1");
		وگرنه
			ps_prompt = env_get("PS2");

		اگر (ps_prompt)
			prompt = ps_prompt;
	}

	بازگشت cli_readline(prompt);
}
