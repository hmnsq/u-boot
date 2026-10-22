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

#அந்தர்பாவ <env.h>
#அந்தர்பாவ <malloc.h>         /* malloc, free, realloc*/
#அந்தர்பாவ <linux/ctype.h>    /* isalpha, isdigit */
#அந்தர்பாவ <console.h>
#அந்தர்பாவ <bootretry.h>
#அந்தர்பாவ <cli.h>
#அந்தர்பாவ <cli_hush.h>
#அந்தர்பாவ <command.h>        /* find_cmd */
#அந்தர்பாவ <asm/global_data.h>

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
#எனில் CONFIG_IS_ENABLED(X86) || CONFIG_IS_ENABLED(X86_64) || CONFIG_IS_ENABLED(MIPS)
/* add other arches which benefit from this... */
வகைவரை signed எழுத்து smallint;
வகைவரை unsigned எழுத்து smalluint;
#இல்லையெனில்
/* for arches where byte accesses generate larger code: */
வகைவரை முழு smallint;
வகைவரை unsigned smalluint;
#endif

/*
 * Alignment defines used by BusyBox.
 */
#define ALIGN1			__attribute__((aligned(1)))
#define ALIGN2			__attribute__((aligned(2)))
#define ALIGN4			__attribute__((aligned(4)))
#define ALIGN8			__attribute__((aligned(8)))
#define ALIGN_PTR		__attribute__((aligned(அளவு(வெறுமை*))))

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

நிலையான வெறுமை __noreturn xfunc_die(வெறுமை)
{
	panic("HUSH died!");
}

#define bb_error_செய்தி_and_die(format, ...) செய் { \
panic("HUSH: " format, __VA_ARGS__); \
} போது (0);

#define bb_simple_error_செய்தி_and_die(செய்தி) செய் { \
panic_str("HUSH: " செய்தி); \
} போது (0);

/* fdprintf() is used for debug output. */
நிலையான முழு __maybe_unused fdprintf(முழு fd, மாறிலி எழுத்து *format, ...)
{
	va_list args;
	uint i;

	ப்ரதிஜ்ஞா(fd == 2);

	va_start(args, format);
	i = vprintf(format, args);
	va_end(args);

	திரும்பு i;
}

நிலையான வெறுமை bb_verror_செய்தி(மாறிலி எழுத்து *s, va_list p, மாறிலி எழுத்து* strerr)
{
	/* TODO: what to do with strerr arg? */
	vprintf(s, p);
}

நிலையான வெறுமை bb_error_செய்தி(மாறிலி எழுத்து *s, ...)
{
	va_list p;

	va_start(p, s);
	bb_verror_செய்தி(s, p, வெற்று);
	va_end(p);
}

நிலையான வெறுமை bb_simple_error_செய்தி(மாறிலி எழுத்து *s)
{
	bb_error_செய்தி("%s", s);
}

நிலையான வெறுமை *xmalloc(size_t size)
{
	வெறுமை *p = வெற்று;
	எனில் (!(p = malloc(size)))
		panic("out of memory");
	திரும்பு p;
}

நிலையான வெறுமை *xzalloc(size_t size)
{
	வெறுமை *p = xmalloc(size);
	memset(p, 0, size);
	திரும்பு p;
}

நிலையான வெறுமை *xrealloc(வெறுமை *ptr, size_t size)
{
	வெறுமை *p = வெற்று;
	எனில் (!(p = realloc(ptr, size)))
		panic("out of memory");
	திரும்பு p;
}

நிலையான வெறுமை *xmemdup(மாறிலி வெறுமை *s, முழு n)
{
	திரும்பு memcpy(xmalloc(n), s, n);
}

#define xstrdup		strdup
#define xstrndup	strndup

நிலையான வெறுமை *mempcpy(வெறுமை *dest, மாறிலி வெறுமை *src, size_t len)
{
	திரும்பு memcpy(dest, src, len) + len;
}

/* Like strcpy but can copy overlapping strings. */
நிலையான வெறுமை overlapping_strcpy(எழுத்து *dst, மாறிலி எழுத்து *src)
{
	/*
	 * Cheap optimization for dst == src case -
	 * better to have it here than in many callers.
	 */
	எனில் (dst != src) {
		போது ((*dst = *src) != '\0') {
			dst++;
			src++;
		}
	}
}

நிலையான எழுத்து* skip_whitespace(மாறிலி எழுத்து *s)
{
	/*
	 * In POSIX/C locale (the only locale we care about: do we REALLY want
	 * to allow Unicode whitespace in, say, .conf files? nuts!)
	 * isspace is only these chars: "\t\n\v\f\r" and space.
	 * "\t\n\v\f\r" happen to have ASCII codes 9,10,11,12,13.
	 * Use that.
	 */
	போது (*s == ' ' || (unsigned எழுத்து)(*s - 9) <= (13 - 9))
		s++;

	திரும்பு (எழுத்து *) s;
}

நிலையான எழுத்து* skip_non_whitespace(மாறிலி எழுத்து *s)
{
	போது (*s != '\0' && *s != ' ' && (unsigned எழுத்து)(*s - 9) > (13 - 9))
		s++;

	திரும்பு (எழுத்து *) s;
}

#define is_name(c)	((c) == '_' || isalpha((unsigned எழுத்து)(c)))
#define is_in_name(c)	((c) == '_' || isalnum((unsigned எழுத்து)(c)))

நிலையான மாறிலி எழுத்து* endofname(மாறிலி எழுத்து *name)
{
	எனில் (!is_name(*name))
		திரும்பு name;
	போது (*++name) {
		எனில் (!is_in_name(*name))
			நிறுத்து;
	}
	திரும்பு name;
}

/**
 * list_size() - returns the number of elements in char ** before NULL.
 *
 * Argument must contain NULL to signalize its end.
 *
 * @list The list to count the number of element.
 * @return The number of element in list.
 */
நிலையான size_t list_size(எழுத்து **list)
{
	size_t size;

	க்கு (size = 0; list[size] != வெற்று; size++);

	திரும்பு size;
}

நிலையான முழு varcmp(மாறிலி எழுத்து *p, மாறிலி எழுத்து *q)
{
	முழு c, d;

	போது ((c = *p) == (d = *q)) {
		எனில் (c == '\0' || c == '=')
			செல் out;
		p++;
		q++;
	}
	எனில் (c == '=')
		c = '\0';
	எனில் (d == '=')
		d = '\0';
out:
	திரும்பு c - d;
}

கட்டமைப்பு in_str;
நிலையான முழு u_boot_cli_readline(கட்டமைப்பு in_str *i);

கட்டமைப்பு in_str;
நிலையான முழு u_boot_cli_readline(கட்டமைப்பு in_str *i);

/*
 * BusyBox globals which are needed for hush.
 */
நிலையான uint8_t xfunc_error_retval;

நிலையான மாறிலி எழுத்து defifsvar[] __aligned(1) = "IFS= \t\n";
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
#அந்தர்பாவ "cli_hush_upstream.c"
/*
 * A                                     A
 * +-- Include of the upstream sources --+ *
 *
 */

முழு u_boot_hush_start_modern(வெறுமை)
{
	INIT_G();
	திரும்பு 0;
}

நிலையான முழு u_boot_cli_readline(கட்டமைப்பு in_str *i)
{
	எழுத்து *prompt;
	எழுத்து __maybe_unused *ps_prompt = வெற்று;

	எனில் (!G.promptmode)
		prompt = CONFIG_SYS_PROMPT;
#ifdef CONFIG_SYS_PROMPT_HUSH_PS2
	இல்லையெனில்
		prompt = CONFIG_SYS_PROMPT_HUSH_PS2;
#இல்லையெனில்
	/* TODO: default value? */
	#error "SYS_PROMPT_HUSH_PS2 is not defined!"
#endif

	எனில் (CONFIG_IS_ENABLED(CMDLINE_PS_SUPPORT)) {
		எனில் (!G.promptmode)
			ps_prompt = env_get("PS1");
		இல்லையெனில்
			ps_prompt = env_get("PS2");

		எனில் (ps_prompt)
			prompt = ps_prompt;
	}

	திரும்பு cli_readline(prompt);
}
