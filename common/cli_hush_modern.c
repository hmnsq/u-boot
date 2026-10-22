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
#تعریف_کرو BB_VER			"1.37.0.git23da5c4b716b"

/*
 * Define hush features by the names used upstream.
 */
#تعریف_کرو ENABLE_HUSH_INTERACTIVE	1
#تعریف_کرو ENABLE_FEATURE_EDITING	1
#تعریف_کرو ENABLE_HUSH_IF		1
#تعریف_کرو ENABLE_HUSH_LOOPS	1
/* No MMU in U-Boot */
#تعریف_کرو BB_MMU			0
#تعریف_کرو USE_FOR_NOMMU(...)	__VA_ARGS__
#تعریف_کرو USE_FOR_MMU(...)

/*
 * Size-saving "small" ints (arch-dependent)
 */
#اگر CONFIG_IS_ENABLED(X86) || CONFIG_IS_ENABLED(X86_64) || CONFIG_IS_ENABLED(MIPS)
/* add other arches which benefit from this... */
قسمتعریف علامتی حرف smallint;
قسمتعریف غیرعلامتی حرف smalluint;
#ورنہ
/* for arches where byte accesses generate larger code: */
قسمتعریف صحیح smallint;
قسمتعریف غیرعلامتی smalluint;
#اگر_ختم

/*
 * Alignment defines used by BusyBox.
 */
#تعریف_کرو ALIGN1			__attribute__((aligned(1)))
#تعریف_کرو ALIGN2			__attribute__((aligned(2)))
#تعریف_کرو ALIGN4			__attribute__((aligned(4)))
#تعریف_کرو ALIGN8			__attribute__((aligned(8)))
#تعریف_کرو ALIGN_PTR		__attribute__((aligned(حجم(خالی*))))

/*
 * Miscellaneous compiler/platform defines.
 */
#تعریف_کرو FAST_FUNC /* not used in U-Boot */
#تعریف_کرو غیر_مستعمل_PARAM		__always_unused
#تعریف_کرو ALWAYS_INLINE		__always_inline
#تعریف_کرو NOINLINE		noinline

/*
 * Defines to provide equivalents to what libc/BusyBox defines.
 */
#تعریف_کرو EOF			(-1)
#تعریف_کرو EXIT_SUCCESS		0
#تعریف_کرو EXIT_FAILURE		1

/*
 * Stubs to provide libc/BusyBox functions based on U-Boot equivalents where it
 * makes sense.
 */
#تعریف_کرو utoa			simple_itoa

ساکن خالی __noreturn xfunc_die(خالی)
{
	panic("HUSH died!");
}

#تعریف_کرو bb_error_پیغام_and_die(format, ...) کرو { \
panic("HUSH: " format, __VA_ARGS__); \
} جب (0);

#تعریف_کرو bb_simple_error_پیغام_and_die(پیغام) کرو { \
panic_str("HUSH: " پیغام); \
} جب (0);

/* fdprintf() is used for debug output. */
ساکن صحیح __maybe_unused fdprintf(صحیح fd, ثابت حرف *format, ...)
{
	va_list دلائل;
	uint i;

	أكد(fd == 2);

	va_آغاز(دلائل, format);
	i = vprintf(format, دلائل);
	va_انتہا(دلائل);

	واپس i;
}

ساکن خالی bb_verror_پیغام(ثابت حرف *s, va_list p, ثابت حرف* strerr)
{
	/* TODO: what to do with strerr arg? */
	vprintf(s, p);
}

ساکن خالی bb_error_پیغام(ثابت حرف *s, ...)
{
	va_list p;

	va_آغاز(p, s);
	bb_verror_پیغام(s, p, تہی);
	va_انتہا(p);
}

ساکن خالی bb_simple_error_پیغام(ثابت حرف *s)
{
	bb_error_پیغام("%s", s);
}

ساکن خالی *xmalloc(size_t سائز)
{
	خالی *p = تہی;
	اگر (!(p = حافظہ_دو(سائز)))
		panic("out of memory");
	واپس p;
}

ساکن خالی *xzalloc(size_t سائز)
{
	خالی *p = xmalloc(سائز);
	memset(p, 0, سائز);
	واپس p;
}

ساکن خالی *xrealloc(خالی *ptr, size_t سائز)
{
	خالی *p = تہی;
	اگر (!(p = realloc(ptr, سائز)))
		panic("out of memory");
	واپس p;
}

ساکن خالی *xmemdup(ثابت خالی *s, صحیح n)
{
	واپس حافظہ_نقل(xmalloc(n), s, n);
}

#تعریف_کرو xstrdup		strdup
#تعریف_کرو xstrndup	strndup

ساکن خالی *mempcpy(خالی *dest, ثابت خالی *src, size_t len)
{
	واپس حافظہ_نقل(dest, src, len) + len;
}

/* Like strcpy but can copy overlapping strings. */
ساکن خالی overlapping_strcpy(حرف *dst, ثابت حرف *src)
{
	/*
	 * Cheap optimization for dst == src case -
	 * better to have it here than in many callers.
	 */
	اگر (dst != src) {
		جب ((*dst = *src) != '\0') {
			dst++;
			src++;
		}
	}
}

ساکن حرف* skip_whitespace(ثابت حرف *s)
{
	/*
	 * In POSIX/C locale (the only locale we care about: do we REALLY want
	 * to allow Unicode whitespace in, say, .conf files? nuts!)
	 * isspace is only these chars: "\t\n\v\f\r" and space.
	 * "\t\n\v\f\r" happen to have ASCII codes 9,10,11,12,13.
	 * Use that.
	 */
	جب (*s == ' ' || (غیرعلامتی حرف)(*s - 9) <= (13 - 9))
		s++;

	واپس (حرف *) s;
}

ساکن حرف* skip_non_whitespace(ثابت حرف *s)
{
	جب (*s != '\0' && *s != ' ' && (غیرعلامتی حرف)(*s - 9) > (13 - 9))
		s++;

	واپس (حرف *) s;
}

#تعریف_کرو is_name(c)	((c) == '_' || حرف_ہے((غیرعلامتی حرف)(c)))
#تعریف_کرو is_اندر_name(c)	((c) == '_' || حرف_یا_ہندسہ((غیرعلامتی حرف)(c)))

ساکن ثابت حرف* endofname(ثابت حرف *name)
{
	اگر (!is_name(*name))
		واپس name;
	جب (*++name) {
		اگر (!is_اندر_name(*name))
			توڑو;
	}
	واپس name;
}

/**
 * list_size() - returns the number of elements in char ** before NULL.
 *
 * Argument must contain NULL to signalize its end.
 *
 * @list The list to count the number of element.
 * @return The number of element in list.
 */
ساکن size_t list_سائز(حرف **list)
{
	size_t سائز;

	برائے (سائز = 0; list[سائز] != تہی; سائز++);

	واپس سائز;
}

ساکن صحیح varcmp(ثابت حرف *p, ثابت حرف *q)
{
	صحیح c, d;

	جب ((c = *p) == (d = *q)) {
		اگر (c == '\0' || c == '=')
			جاؤ برآمد;
		p++;
		q++;
	}
	اگر (c == '=')
		c = '\0';
	اگر (d == '=')
		d = '\0';
برآمد:
	واپس c - d;
}

ساخت اندر_str;
ساکن صحیح u_boot_cli_readline(ساخت اندر_str *i);

ساخت اندر_str;
ساکن صحیح u_boot_cli_readline(ساخت اندر_str *i);

/*
 * BusyBox globals which are needed for hush.
 */
ساکن uint8_t xfunc_error_retval;

ساکن ثابت حرف defifsvar[] __aligned(1) = "IFS= \t\n";
#تعریف_کرو defifs (defifsvar + 4)

/* This define is used to check if exit command was called. */
#تعریف_کرو EXIT_RET_رمز -2

/*
 * This define is used for changes that need be done directly in the upstream
 * sources still. Ideally, its use should be minimized as much as possible.
 */
#تعریف_کرو __U_BOOT__

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

صحیح u_boot_hush_آغاز_modern(خالی)
{
	INIT_G();
	واپس 0;
}

ساکن صحیح u_boot_cli_readline(ساخت اندر_str *i)
{
	حرف *prompt;
	حرف __maybe_unused *ps_prompt = تہی;

	اگر (!G.promptmode)
		prompt = CONFIG_SYS_PROMPT;
#اگر_تعریف_شدہ CONFIG_SYS_PROMPT_HUSH_PS2
	ورنہ
		prompt = CONFIG_SYS_PROMPT_HUSH_PS2;
#ورنہ
	/* TODO: default value? */
	#غلطی "SYS_PROMPT_HUSH_PS2 is not defined!"
#اگر_ختم

	اگر (CONFIG_IS_ENABLED(CMDLINE_PS_SUPPORT)) {
		اگر (!G.promptmode)
			ps_prompt = env_get("PS1");
		ورنہ
			ps_prompt = env_get("PS2");

		اگر (ps_prompt)
			prompt = ps_prompt;
	}

	واپس cli_readline(prompt);
}
