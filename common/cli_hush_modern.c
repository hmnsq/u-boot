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
#تعریف_کړه BB_VER			"1.37.0.git23da5c4b716b"

/*
 * Define hush features by the names used upstream.
 */
#تعریف_کړه ENABLE_HUSH_INTERACTIVE	1
#تعریف_کړه ENABLE_FEATURE_EDITING	1
#تعریف_کړه ENABLE_HUSH_IF		1
#تعریف_کړه ENABLE_HUSH_LOOPS	1
/* No MMU in U-Boot */
#تعریف_کړه BB_MMU			0
#تعریف_کړه USE_FOR_NOMMU(...)	__VA_ARGS__
#تعریف_کړه USE_FOR_MMU(...)

/*
 * Size-saving "small" ints (arch-dependent)
 */
#إذا CONFIG_IS_ENABLED(X86) || CONFIG_IS_ENABLED(X86_64) || CONFIG_IS_ENABLED(MIPS)
/* add other arches which benefit from this... */
تعريفنوع نښهدار حرف smallint;
تعريفنوع بینښه حرف smalluint;
#وإلا
/* for arches where byte accesses generate larger code: */
تعريفنوع صحيح smallint;
تعريفنوع بینښه smalluint;
#د_که_پای

/*
 * Alignment defines used by BusyBox.
 */
#تعریف_کړه ALIGN1			__attribute__((aligned(1)))
#تعریف_کړه ALIGN2			__attribute__((aligned(2)))
#تعریف_کړه ALIGN4			__attribute__((aligned(4)))
#تعریف_کړه ALIGN8			__attribute__((aligned(8)))
#تعریف_کړه ALIGN_PTR		__attribute__((aligned(حجم(فراغ*))))

/*
 * Miscellaneous compiler/platform defines.
 */
#تعریف_کړه FAST_FUNC /* not used in U-Boot */
#تعریف_کړه ناکارول_شوی_PARAM		__always_unused
#تعریف_کړه ALWAYS_INLINE		__always_inline
#تعریف_کړه NOINLINE		noinline

/*
 * Defines to provide equivalents to what libc/BusyBox defines.
 */
#تعریف_کړه EOF			(-1)
#تعریف_کړه EXIT_SUCCESS		0
#تعریف_کړه EXIT_FAILURE		1

/*
 * Stubs to provide libc/BusyBox functions based on U-Boot equivalents where it
 * makes sense.
 */
#تعریف_کړه utoa			simple_itoa

ساكن فراغ __noreturn xfunc_die(فراغ)
{
	panic("HUSH died!");
}

#تعریف_کړه bb_error_پیغام_and_die(format, ...) افعل { \
panic("HUSH: " format, __VA_ARGS__); \
} طالما (0);

#تعریف_کړه bb_simple_error_پیغام_and_die(پیغام) افعل { \
panic_str("HUSH: " پیغام); \
} طالما (0);

/* fdprintf() is used for debug output. */
ساكن صحيح __maybe_unused fdprintf(صحيح fd, ثابت حرف *format, ...)
{
	va_list دلیلونه;
	uint i;

	أكد(fd == 2);

	va_پیل(دلیلونه, format);
	i = vprintf(format, دلیلونه);
	va_پای_ټکی(دلیلونه);

	أرجع i;
}

ساكن فراغ bb_verror_پیغام(ثابت حرف *s, va_list p, ثابت حرف* strerr)
{
	/* TODO: what to do with strerr arg? */
	vprintf(s, p);
}

ساكن فراغ bb_error_پیغام(ثابت حرف *s, ...)
{
	va_list p;

	va_پیل(p, s);
	bb_verror_پیغام(s, p, عدم);
	va_پای_ټکی(p);
}

ساكن فراغ bb_simple_error_پیغام(ثابت حرف *s)
{
	bb_error_پیغام("%s", s);
}

ساكن فراغ *xmalloc(size_t اندازه)
{
	فراغ *p = عدم;
	إذا (!(p = حافظه_ورکړه(اندازه)))
		panic("out of memory");
	أرجع p;
}

ساكن فراغ *xzalloc(size_t اندازه)
{
	فراغ *p = xmalloc(اندازه);
	memset(p, 0, اندازه);
	أرجع p;
}

ساكن فراغ *xrealloc(فراغ *ptr, size_t اندازه)
{
	فراغ *p = عدم;
	إذا (!(p = realloc(ptr, اندازه)))
		panic("out of memory");
	أرجع p;
}

ساكن فراغ *xmemdup(ثابت فراغ *s, صحيح n)
{
	أرجع حافظه_نقل(xmalloc(n), s, n);
}

#تعریف_کړه xstrdup		strdup
#تعریف_کړه xstrndup	strndup

ساكن فراغ *mempcpy(فراغ *dest, ثابت فراغ *src, size_t len)
{
	أرجع حافظه_نقل(dest, src, len) + len;
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
	طالما (*s == ' ' || (بینښه حرف)(*s - 9) <= (13 - 9))
		s++;

	أرجع (حرف *) s;
}

ساكن حرف* skip_non_whitespace(ثابت حرف *s)
{
	طالما (*s != '\0' && *s != ' ' && (بینښه حرف)(*s - 9) > (13 - 9))
		s++;

	أرجع (حرف *) s;
}

#تعریف_کړه is_name(c)	((c) == '_' || که_توری((بینښه حرف)(c)))
#تعریف_کړه is_دننه_name(c)	((c) == '_' || که_توری_یا_عدد((بینښه حرف)(c)))

ساكن ثابت حرف* endofname(ثابت حرف *name)
{
	إذا (!is_name(*name))
		أرجع name;
	طالما (*++name) {
		إذا (!is_دننه_name(*name))
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
ساكن size_t list_اندازه(حرف **list)
{
	size_t اندازه;

	لكل (اندازه = 0; list[اندازه] != عدم; اندازه++);

	أرجع اندازه;
}

ساكن صحيح varcmp(ثابت حرف *p, ثابت حرف *q)
{
	صحيح c, d;

	طالما ((c = *p) == (d = *q)) {
		إذا (c == '\0' || c == '=')
			اذهب بهر;
		p++;
		q++;
	}
	إذا (c == '=')
		c = '\0';
	إذا (d == '=')
		d = '\0';
بهر:
	أرجع c - d;
}

بنية دننه_str;
ساكن صحيح u_boot_cli_readline(بنية دننه_str *i);

بنية دننه_str;
ساكن صحيح u_boot_cli_readline(بنية دننه_str *i);

/*
 * BusyBox globals which are needed for hush.
 */
ساكن uint8_t xfunc_error_retval;

ساكن ثابت حرف defifsvar[] __aligned(1) = "IFS= \t\n";
#تعریف_کړه defifs (defifsvar + 4)

/* This define is used to check if exit command was called. */
#تعریف_کړه EXIT_RET_کوډ -2

/*
 * This define is used for changes that need be done directly in the upstream
 * sources still. Ideally, its use should be minimized as much as possible.
 */
#تعریف_کړه __U_BOOT__

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

صحيح u_boot_hush_پیل_modern(فراغ)
{
	INIT_G();
	أرجع 0;
}

ساكن صحيح u_boot_cli_readline(بنية دننه_str *i)
{
	حرف *prompt;
	حرف __maybe_unused *ps_prompt = عدم;

	إذا (!G.promptmode)
		prompt = CONFIG_SYS_PROMPT;
#که_تعریف_شوی CONFIG_SYS_PROMPT_HUSH_PS2
	وإلا
		prompt = CONFIG_SYS_PROMPT_HUSH_PS2;
#وإلا
	/* TODO: default value? */
	#تېروتنه "SYS_PROMPT_HUSH_PS2 is not defined!"
#د_که_پای

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
