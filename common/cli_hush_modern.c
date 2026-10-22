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

#ضمن <env.h>
#ضمن <malloc.h>         /* malloc, free, realloc*/
#ضمن <linux/ctype.h>    /* isalpha, isdigit */
#ضمن <console.h>
#ضمن <bootretry.h>
#ضمن <cli.h>
#ضمن <cli_hush.h>
#ضمن <command.h>        /* find_cmd */
#ضمن <asm/global_data.h>

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
#إذا CONFIG_IS_ENABLED(X86) || CONFIG_IS_ENABLED(X86_64) || CONFIG_IS_ENABLED(MIPS)
/* add other arches which benefit from this... */
تعريفنوع signed حرف smallint;
تعريفنوع unsigned حرف smalluint;
#وإلا
/* for arches where byte accesses generate larger code: */
تعريفنوع صحيح smallint;
تعريفنوع unsigned smalluint;
#endif

/*
 * Alignment defines used by BusyBox.
 */
#define ALIGN1			__attribute__((aligned(1)))
#define ALIGN2			__attribute__((aligned(2)))
#define ALIGN4			__attribute__((aligned(4)))
#define ALIGN8			__attribute__((aligned(8)))
#define ALIGN_PTR		__attribute__((aligned(حجم(فراغ*))))

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

ساكن فراغ __noreturn xfunc_die(فراغ)
{
	panic("HUSH died!");
}

#define bb_error_پەیام_and_die(format, ...) افعل { \
panic("HUSH: " format, __VA_ARGS__); \
} طالما (0);

#define bb_simple_error_پەیام_and_die(پەیام) افعل { \
panic_str("HUSH: " پەیام); \
} طالما (0);

/* fdprintf() is used for debug output. */
ساكن صحيح __maybe_unused fdprintf(صحيح fd, ثابت حرف *format, ...)
{
	va_list args;
	uint i;

	أكد(fd == 2);

	va_start(args, format);
	i = vprintf(format, args);
	va_end(args);

	أرجع i;
}

ساكن فراغ bb_verror_پەیام(ثابت حرف *s, va_list p, ثابت حرف* strerr)
{
	/* TODO: what to do with strerr arg? */
	vprintf(s, p);
}

ساكن فراغ bb_error_پەیام(ثابت حرف *s, ...)
{
	va_list p;

	va_start(p, s);
	bb_verror_پەیام(s, p, عدم);
	va_end(p);
}

ساكن فراغ bb_simple_error_پەیام(ثابت حرف *s)
{
	bb_error_پەیام("%s", s);
}

ساكن فراغ *xmalloc(size_t size)
{
	فراغ *p = عدم;
	إذا (!(p = malloc(size)))
		panic("out of memory");
	أرجع p;
}

ساكن فراغ *xzalloc(size_t size)
{
	فراغ *p = xmalloc(size);
	memset(p, 0, size);
	أرجع p;
}

ساكن فراغ *xrealloc(فراغ *ptr, size_t size)
{
	فراغ *p = عدم;
	إذا (!(p = realloc(ptr, size)))
		panic("out of memory");
	أرجع p;
}

ساكن فراغ *xmemdup(ثابت فراغ *s, صحيح n)
{
	أرجع memcpy(xmalloc(n), s, n);
}

#define xstrdup		strdup
#define xstrndup	strndup

ساكن فراغ *mempcpy(فراغ *dest, ثابت فراغ *src, size_t len)
{
	أرجع memcpy(dest, src, len) + len;
}

/* Like strcpy but can copy overlapping strings. */
ساكن فراغ overlapping_strcpy(حرف *dst, ثابت حرف *src)
{
	/*
	 * Cheap optimization for dst == src case -
	 * better to have it here than in many callers.
	 */
	إذا (dst != src) {
		طالما ((*dst = *src) != '\0') {
			dst++;
			src++;
		}
	}
}

ساكن حرف* skip_whitespace(ثابت حرف *s)
{
	/*
	 * In POSIX/C locale (the only locale we care about: do we REALLY want
	 * to allow Unicode whitespace in, say, .conf files? nuts!)
	 * isspace is only these chars: "\t\n\v\f\r" and space.
	 * "\t\n\v\f\r" happen to have ASCII codes 9,10,11,12,13.
	 * Use that.
	 */
	طالما (*s == ' ' || (unsigned حرف)(*s - 9) <= (13 - 9))
		s++;

	أرجع (حرف *) s;
}

ساكن حرف* skip_non_whitespace(ثابت حرف *s)
{
	طالما (*s != '\0' && *s != ' ' && (unsigned حرف)(*s - 9) > (13 - 9))
		s++;

	أرجع (حرف *) s;
}

#define is_name(c)	((c) == '_' || isalpha((unsigned حرف)(c)))
#define is_in_name(c)	((c) == '_' || isalnum((unsigned حرف)(c)))

ساكن ثابت حرف* endofname(ثابت حرف *name)
{
	إذا (!is_name(*name))
		أرجع name;
	طالما (*++name) {
		إذا (!is_in_name(*name))
			اكسر;
	}
	أرجع name;
}

/**
 * list_size() - returns the number of elements in char ** before NULL.
 *
 * Argument must contain NULL to signalize its end.
 *
 * @list The list to count the number of element.
 * @return The number of element in list.
 */
ساكن size_t list_size(حرف **list)
{
	size_t size;

	لكل (size = 0; list[size] != عدم; size++);

	أرجع size;
}

ساكن صحيح varcmp(ثابت حرف *p, ثابت حرف *q)
{
	صحيح c, d;

	طالما ((c = *p) == (d = *q)) {
		إذا (c == '\0' || c == '=')
			اذهب out;
		p++;
		q++;
	}
	إذا (c == '=')
		c = '\0';
	إذا (d == '=')
		d = '\0';
out:
	أرجع c - d;
}

بنية in_str;
ساكن صحيح u_boot_cli_readline(بنية in_str *i);

بنية in_str;
ساكن صحيح u_boot_cli_readline(بنية in_str *i);

/*
 * BusyBox globals which are needed for hush.
 */
ساكن uint8_t xfunc_error_retval;

ساكن ثابت حرف defifsvar[] __aligned(1) = "IFS= \t\n";
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
#ضمن "cli_hush_upstream.c"
/*
 * A                                     A
 * +-- Include of the upstream sources --+ *
 *
 */

صحيح u_boot_hush_start_modern(فراغ)
{
	INIT_G();
	أرجع 0;
}

ساكن صحيح u_boot_cli_readline(بنية in_str *i)
{
	حرف *prompt;
	حرف __maybe_unused *ps_prompt = عدم;

	إذا (!G.promptmode)
		prompt = CONFIG_SYS_PROMPT;
#ifdef CONFIG_SYS_PROMPT_HUSH_PS2
	وإلا
		prompt = CONFIG_SYS_PROMPT_HUSH_PS2;
#وإلا
	/* TODO: default value? */
	#error "SYS_PROMPT_HUSH_PS2 is not defined!"
#endif

	إذا (CONFIG_IS_ENABLED(CMDLINE_PS_SUPPORT)) {
		إذا (!G.promptmode)
			ps_prompt = env_get("PS1");
		وإلا
			ps_prompt = env_get("PS2");

		إذا (ps_prompt)
			prompt = ps_prompt;
	}

	أرجع cli_readline(prompt);
}
