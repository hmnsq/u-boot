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

#અન્તર્ભાવ <env.h>
#અન્તર્ભાવ <malloc.h>         /* malloc, free, realloc*/
#અન્તર્ભાવ <linux/ctype.h>    /* isalpha, isdigit */
#અન્તર્ભાવ <console.h>
#અન્તર્ભાવ <bootretry.h>
#અન્તર્ભાવ <cli.h>
#અન્તર્ભાવ <cli_hush.h>
#અન્તર્ભાવ <command.h>        /* find_cmd */
#અન્તર્ભાવ <asm/global_data.h>

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
#જો CONFIG_IS_ENABLED(X86) || CONFIG_IS_ENABLED(X86_64) || CONFIG_IS_ENABLED(MIPS)
/* add other arches which benefit from this... */
પ્રકારવ્યાખ્યા signed અક્ષર smallint;
પ્રકારવ્યાખ્યા unsigned અક્ષર smalluint;
#નહીંતર
/* for arches where byte accesses generate larger code: */
પ્રકારવ્યાખ્યા પૂર્ણાંક smallint;
પ્રકારવ્યાખ્યા unsigned smalluint;
#endif

/*
 * Alignment defines used by BusyBox.
 */
#define ALIGN1			__attribute__((aligned(1)))
#define ALIGN2			__attribute__((aligned(2)))
#define ALIGN4			__attribute__((aligned(4)))
#define ALIGN8			__attribute__((aligned(8)))
#define ALIGN_PTR		__attribute__((aligned(કદ(શૂન્ય*))))

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

સ્થિર શૂન્ય __noreturn xfunc_die(શૂન્ય)
{
	panic("HUSH died!");
}

#define bb_error_સંદેશ_and_die(format, ...) કરો { \
panic("HUSH: " format, __VA_ARGS__); \
} જ્યારે (0);

#define bb_simple_error_સંદેશ_and_die(સંદેશ) કરો { \
panic_str("HUSH: " સંદેશ); \
} જ્યારે (0);

/* fdprintf() is used for debug output. */
સ્થિર પૂર્ણાંક __maybe_unused fdprintf(પૂર્ણાંક fd, અચલ અક્ષર *format, ...)
{
	va_list args;
	uint i;

	પ્રતિજ્ઞા(fd == 2);

	va_start(args, format);
	i = vprintf(format, args);
	va_end(args);

	પાછા i;
}

સ્થિર શૂન્ય bb_verror_સંદેશ(અચલ અક્ષર *s, va_list p, અચલ અક્ષર* strerr)
{
	/* TODO: what to do with strerr arg? */
	vprintf(s, p);
}

સ્થિર શૂન્ય bb_error_સંદેશ(અચલ અક્ષર *s, ...)
{
	va_list p;

	va_start(p, s);
	bb_verror_સંદેશ(s, p, ખાલી);
	va_end(p);
}

સ્થિર શૂન્ય bb_simple_error_સંદેશ(અચલ અક્ષર *s)
{
	bb_error_સંદેશ("%s", s);
}

સ્થિર શૂન્ય *xmalloc(size_t size)
{
	શૂન્ય *p = ખાલી;
	જો (!(p = malloc(size)))
		panic("out of memory");
	પાછા p;
}

સ્થિર શૂન્ય *xzalloc(size_t size)
{
	શૂન્ય *p = xmalloc(size);
	memset(p, 0, size);
	પાછા p;
}

સ્થિર શૂન્ય *xrealloc(શૂન્ય *ptr, size_t size)
{
	શૂન્ય *p = ખાલી;
	જો (!(p = realloc(ptr, size)))
		panic("out of memory");
	પાછા p;
}

સ્થિર શૂન્ય *xmemdup(અચલ શૂન્ય *s, પૂર્ણાંક n)
{
	પાછા memcpy(xmalloc(n), s, n);
}

#define xstrdup		strdup
#define xstrndup	strndup

સ્થિર શૂન્ય *mempcpy(શૂન્ય *dest, અચલ શૂન્ય *src, size_t len)
{
	પાછા memcpy(dest, src, len) + len;
}

/* Like strcpy but can copy overlapping strings. */
સ્થિર શૂન્ય overlapping_strcpy(અક્ષર *dst, અચલ અક્ષર *src)
{
	/*
	 * Cheap optimization for dst == src case -
	 * better to have it here than in many callers.
	 */
	જો (dst != src) {
		જ્યારે ((*dst = *src) != '\0') {
			dst++;
			src++;
		}
	}
}

સ્થિર અક્ષર* skip_whitespace(અચલ અક્ષર *s)
{
	/*
	 * In POSIX/C locale (the only locale we care about: do we REALLY want
	 * to allow Unicode whitespace in, say, .conf files? nuts!)
	 * isspace is only these chars: "\t\n\v\f\r" and space.
	 * "\t\n\v\f\r" happen to have ASCII codes 9,10,11,12,13.
	 * Use that.
	 */
	જ્યારે (*s == ' ' || (unsigned અક્ષર)(*s - 9) <= (13 - 9))
		s++;

	પાછા (અક્ષર *) s;
}

સ્થિર અક્ષર* skip_non_whitespace(અચલ અક્ષર *s)
{
	જ્યારે (*s != '\0' && *s != ' ' && (unsigned અક્ષર)(*s - 9) > (13 - 9))
		s++;

	પાછા (અક્ષર *) s;
}

#define is_name(c)	((c) == '_' || isalpha((unsigned અક્ષર)(c)))
#define is_in_name(c)	((c) == '_' || isalnum((unsigned અક્ષર)(c)))

સ્થિર અચલ અક્ષર* endofname(અચલ અક્ષર *name)
{
	જો (!is_name(*name))
		પાછા name;
	જ્યારે (*++name) {
		જો (!is_in_name(*name))
			તોડો;
	}
	પાછા name;
}

/**
 * list_size() - returns the number of elements in char ** before NULL.
 *
 * Argument must contain NULL to signalize its end.
 *
 * @list The list to count the number of element.
 * @return The number of element in list.
 */
સ્થિર size_t list_size(અક્ષર **list)
{
	size_t size;

	માટે (size = 0; list[size] != ખાલી; size++);

	પાછા size;
}

સ્થિર પૂર્ણાંક varcmp(અચલ અક્ષર *p, અચલ અક્ષર *q)
{
	પૂર્ણાંક c, d;

	જ્યારે ((c = *p) == (d = *q)) {
		જો (c == '\0' || c == '=')
			જાઓ out;
		p++;
		q++;
	}
	જો (c == '=')
		c = '\0';
	જો (d == '=')
		d = '\0';
out:
	પાછા c - d;
}

સંરચના in_str;
સ્થિર પૂર્ણાંક u_boot_cli_readline(સંરચના in_str *i);

સંરચના in_str;
સ્થિર પૂર્ણાંક u_boot_cli_readline(સંરચના in_str *i);

/*
 * BusyBox globals which are needed for hush.
 */
સ્થિર uint8_t xfunc_error_retval;

સ્થિર અચલ અક્ષર defifsvar[] __aligned(1) = "IFS= \t\n";
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
#અન્તર્ભાવ "cli_hush_upstream.c"
/*
 * A                                     A
 * +-- Include of the upstream sources --+ *
 *
 */

પૂર્ણાંક u_boot_hush_start_modern(શૂન્ય)
{
	INIT_G();
	પાછા 0;
}

સ્થિર પૂર્ણાંક u_boot_cli_readline(સંરચના in_str *i)
{
	અક્ષર *prompt;
	અક્ષર __maybe_unused *ps_prompt = ખાલી;

	જો (!G.promptmode)
		prompt = CONFIG_SYS_PROMPT;
#ifdef CONFIG_SYS_PROMPT_HUSH_PS2
	નહીંતર
		prompt = CONFIG_SYS_PROMPT_HUSH_PS2;
#નહીંતર
	/* TODO: default value? */
	#error "SYS_PROMPT_HUSH_PS2 is not defined!"
#endif

	જો (CONFIG_IS_ENABLED(CMDLINE_PS_SUPPORT)) {
		જો (!G.promptmode)
			ps_prompt = env_get("PS1");
		નહીંતર
			ps_prompt = env_get("PS2");

		જો (ps_prompt)
			prompt = ps_prompt;
	}

	પાછા cli_readline(prompt);
}
