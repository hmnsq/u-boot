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
#وصف_ڪريو BB_VER			"1.37.0.git23da5c4b716b"

/*
 * Define hush features by the names used upstream.
 */
#وصف_ڪريو ENABLE_HUSH_INTERACTIVE	1
#وصف_ڪريو ENABLE_FEATURE_EDITING	1
#وصف_ڪريو ENABLE_HUSH_IF		1
#وصف_ڪريو ENABLE_HUSH_LOOPS	1
/* No MMU in U-Boot */
#وصف_ڪريو BB_MMU			0
#وصف_ڪريو USE_FOR_NOMMU(...)	__VA_ARGS__
#وصف_ڪريو USE_FOR_MMU(...)

/*
 * Size-saving "small" ints (arch-dependent)
 */
#إذا CONFIG_IS_ENABLED(X86) || CONFIG_IS_ENABLED(X86_64) || CONFIG_IS_ENABLED(MIPS)
/* add other arches which benefit from this... */
تعريفنوع نشانوارو حرف smallint;
تعريفنوع بنانشان حرف smalluint;
#وإلا
/* for arches where byte accesses generate larger code: */
تعريفنوع صحيح smallint;
تعريفنوع بنانشان smalluint;
#جيڪڏهن_پورو

/*
 * Alignment defines used by BusyBox.
 */
#وصف_ڪريو ALIGN1			__attribute__((aligned(1)))
#وصف_ڪريو ALIGN2			__attribute__((aligned(2)))
#وصف_ڪريو ALIGN4			__attribute__((aligned(4)))
#وصف_ڪريو ALIGN8			__attribute__((aligned(8)))
#وصف_ڪريو ALIGN_PTR		__attribute__((aligned(حجم(فراغ*))))

/*
 * Miscellaneous compiler/platform defines.
 */
#وصف_ڪريو FAST_FUNC /* not used in U-Boot */
#وصف_ڪريو اڻ_استعمال_ٿيل_PARAM		__always_unused
#وصف_ڪريو ALWAYS_INLINE		__always_inline
#وصف_ڪريو NOINLINE		noinline

/*
 * Defines to provide equivalents to what libc/BusyBox defines.
 */
#وصف_ڪريو EOF			(-1)
#وصف_ڪريو EXIT_SUCCESS		0
#وصف_ڪريو EXIT_FAILURE		1

/*
 * Stubs to provide libc/BusyBox functions based on U-Boot equivalents where it
 * makes sense.
 */
#وصف_ڪريو utoa			simple_itoa

ساكن فراغ __noreturn xfunc_die(فراغ)
{
	panic("HUSH died!");
}

#وصف_ڪريو bb_error_نياپو_and_die(format, ...) افعل { \
panic("HUSH: " format, __VA_ARGS__); \
} طالما (0);

#وصف_ڪريو bb_simple_error_نياپو_and_die(نياپو) افعل { \
panic_str("HUSH: " نياپو); \
} طالما (0);

/* fdprintf() is used for debug output. */
ساكن صحيح __maybe_unused fdprintf(صحيح fd, ثابت حرف *format, ...)
{
	va_list دليلون;
	uint i;

	أكد(fd == 2);

	va_شروعات(دليلون, format);
	i = vprintf(format, دليلون);
	va_پڇاڙي(دليلون);

	أرجع i;
}

ساكن فراغ bb_verror_نياپو(ثابت حرف *s, va_list p, ثابت حرف* strerr)
{
	/* TODO: what to do with strerr arg? */
	vprintf(s, p);
}

ساكن فراغ bb_error_نياپو(ثابت حرف *s, ...)
{
	va_list p;

	va_شروعات(p, s);
	bb_verror_نياپو(s, p, عدم);
	va_پڇاڙي(p);
}

ساكن فراغ bb_simple_error_نياپو(ثابت حرف *s)
{
	bb_error_نياپو("%s", s);
}

ساكن فراغ *xmalloc(size_t سائيز)
{
	فراغ *p = عدم;
	إذا (!(p = يادگيري_ڏيو(سائيز)))
		panic("out of memory");
	أرجع p;
}

ساكن فراغ *xzalloc(size_t سائيز)
{
	فراغ *p = xmalloc(سائيز);
	memset(p, 0, سائيز);
	أرجع p;
}

ساكن فراغ *xrealloc(فراغ *ptr, size_t سائيز)
{
	فراغ *p = عدم;
	إذا (!(p = realloc(ptr, سائيز)))
		panic("out of memory");
	أرجع p;
}

ساكن فراغ *xmemdup(ثابت فراغ *s, صحيح n)
{
	أرجع يادگيري_نقل(xmalloc(n), s, n);
}

#وصف_ڪريو xstrdup		strdup
#وصف_ڪريو xstrndup	strndup

ساكن فراغ *mempcpy(فراغ *dest, ثابت فراغ *src, size_t len)
{
	أرجع يادگيري_نقل(dest, src, len) + len;
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
	طالما (*s == ' ' || (بنانشان حرف)(*s - 9) <= (13 - 9))
		s++;

	أرجع (حرف *) s;
}

ساكن حرف* skip_non_whitespace(ثابت حرف *s)
{
	طالما (*s != '\0' && *s != ' ' && (بنانشان حرف)(*s - 9) > (13 - 9))
		s++;

	أرجع (حرف *) s;
}

#وصف_ڪريو is_name(c)	((c) == '_' || اکر_آهي((بنانشان حرف)(c)))
#وصف_ڪريو is_اندريون_name(c)	((c) == '_' || اکر_يا_انگ((بنانشان حرف)(c)))

ساكن ثابت حرف* endofname(ثابت حرف *name)
{
	إذا (!is_name(*name))
		أرجع name;
	طالما (*++name) {
		إذا (!is_اندريون_name(*name))
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
ساكن size_t list_سائيز(حرف **list)
{
	size_t سائيز;

	لكل (سائيز = 0; list[سائيز] != عدم; سائيز++);

	أرجع سائيز;
}

ساكن صحيح varcmp(ثابت حرف *p, ثابت حرف *q)
{
	صحيح c, d;

	طالما ((c = *p) == (d = *q)) {
		إذا (c == '\0' || c == '=')
			اذهب ٻاهريون;
		p++;
		q++;
	}
	إذا (c == '=')
		c = '\0';
	إذا (d == '=')
		d = '\0';
ٻاهريون:
	أرجع c - d;
}

بنية اندريون_str;
ساكن صحيح u_boot_cli_readline(بنية اندريون_str *i);

بنية اندريون_str;
ساكن صحيح u_boot_cli_readline(بنية اندريون_str *i);

/*
 * BusyBox globals which are needed for hush.
 */
ساكن uint8_t xfunc_error_retval;

ساكن ثابت حرف defifsvar[] __aligned(1) = "IFS= \t\n";
#وصف_ڪريو defifs (defifsvar + 4)

/* This define is used to check if exit command was called. */
#وصف_ڪريو EXIT_RET_ڪوڊ -2

/*
 * This define is used for changes that need be done directly in the upstream
 * sources still. Ideally, its use should be minimized as much as possible.
 */
#وصف_ڪريو __U_BOOT__

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

صحيح u_boot_hush_شروعات_modern(فراغ)
{
	INIT_G();
	أرجع 0;
}

ساكن صحيح u_boot_cli_readline(بنية اندريون_str *i)
{
	حرف *prompt;
	حرف __maybe_unused *ps_prompt = عدم;

	إذا (!G.promptmode)
		prompt = CONFIG_SYS_PROMPT;
#جيڪڏهن_وصف_ٿيل CONFIG_SYS_PROMPT_HUSH_PS2
	وإلا
		prompt = CONFIG_SYS_PROMPT_HUSH_PS2;
#وإلا
	/* TODO: default value? */
	#غلطي "SYS_PROMPT_HUSH_PS2 is not defined!"
#جيڪڏهن_پورو

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
