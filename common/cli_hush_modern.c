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

#అన్తర్భావ <env.h>
#అన్తర్భావ <malloc.h>         /* malloc, free, realloc*/
#అన్తర్భావ <linux/ctype.h>    /* isalpha, isdigit */
#అన్తర్భావ <console.h>
#అన్తర్భావ <bootretry.h>
#అన్తర్భావ <cli.h>
#అన్తర్భావ <cli_hush.h>
#అన్తర్భావ <command.h>        /* find_cmd */
#అన్తర్భావ <asm/global_data.h>

/*
 * BusyBox Version: UPDATE THIS WHEN PULLING NEW UPSTREAM REVISION!
 */
#నిర్వచించు BB_VER			"1.37.0.git23da5c4b716b"

/*
 * Define hush features by the names used upstream.
 */
#నిర్వచించు ENABLE_HUSH_INTERACTIVE	1
#నిర్వచించు ENABLE_FEATURE_EDITING	1
#నిర్వచించు ENABLE_HUSH_IF		1
#నిర్వచించు ENABLE_HUSH_LOOPS	1
/* No MMU in U-Boot */
#నిర్వచించు BB_MMU			0
#నిర్వచించు USE_FOR_NOMMU(...)	__VA_ARGS__
#నిర్వచించు USE_FOR_MMU(...)

/*
 * Size-saving "small" ints (arch-dependent)
 */
#ఒకవేళ CONFIG_IS_ENABLED(X86) || CONFIG_IS_ENABLED(X86_64) || CONFIG_IS_ENABLED(MIPS)
/* add other arches which benefit from this... */
రకనిర్వచనం గుర్తుగల అక్షరం smallint;
రకనిర్వచనం గుర్తులేని అక్షరం smalluint;
#లేకపోతే
/* for arches where byte accesses generate larger code: */
రకనిర్వచనం పూర్ణాంకం smallint;
రకనిర్వచనం గుర్తులేని smalluint;
#అయితే_ముగింపు

/*
 * Alignment defines used by BusyBox.
 */
#నిర్వచించు ALIGN1			__attribute__((aligned(1)))
#నిర్వచించు ALIGN2			__attribute__((aligned(2)))
#నిర్వచించు ALIGN4			__attribute__((aligned(4)))
#నిర్వచించు ALIGN8			__attribute__((aligned(8)))
#నిర్వచించు ALIGN_PTR		__attribute__((aligned(పరిమాణం(శూన్య*))))

/*
 * Miscellaneous compiler/platform defines.
 */
#నిర్వచించు FAST_FUNC /* not used in U-Boot */
#నిర్వచించు వాడని_PARAM		__always_unused
#నిర్వచించు ALWAYS_INLINE		__always_inline
#నిర్వచించు NOINLINE		noinline

/*
 * Defines to provide equivalents to what libc/BusyBox defines.
 */
#నిర్వచించు EOF			(-1)
#నిర్వచించు EXIT_SUCCESS		0
#నిర్వచించు EXIT_FAILURE		1

/*
 * Stubs to provide libc/BusyBox functions based on U-Boot equivalents where it
 * makes sense.
 */
#నిర్వచించు utoa			simple_itoa

స్థిర శూన్య __noreturn xfunc_die(శూన్య)
{
	panic("HUSH died!");
}

#నిర్వచించు bb_error_సందేశం_and_die(format, ...) చేయి { \
panic("HUSH: " format, __VA_ARGS__); \
} అయితే (0);

#నిర్వచించు bb_simple_error_సందేశం_and_die(సందేశం) చేయి { \
panic_str("HUSH: " సందేశం); \
} అయితే (0);

/* fdprintf() is used for debug output. */
స్థిర పూర్ణాంకం __maybe_unused fdprintf(పూర్ణాంకం fd, స్థిరరాశి అక్షరం *format, ...)
{
	va_list వాదనలు;
	uint i;

	ప్రతిజ్ఞా(fd == 2);

	va_ప్రారంభం(వాదనలు, format);
	i = vprintf(format, వాదనలు);
	va_చివర(వాదనలు);

	తిరిగి i;
}

స్థిర శూన్య bb_verror_సందేశం(స్థిరరాశి అక్షరం *s, va_list p, స్థిరరాశి అక్షరం* strerr)
{
	/* TODO: what to do with strerr arg? */
	vprintf(s, p);
}

స్థిర శూన్య bb_error_సందేశం(స్థిరరాశి అక్షరం *s, ...)
{
	va_list p;

	va_ప్రారంభం(p, s);
	bb_verror_సందేశం(s, p, ఖాళీ);
	va_చివర(p);
}

స్థిర శూన్య bb_simple_error_సందేశం(స్థిరరాశి అక్షరం *s)
{
	bb_error_సందేశం("%s", s);
}

స్థిర శూన్య *xmalloc(size_t కొలత)
{
	శూన్య *p = ఖాళీ;
	ఒకవేళ (!(p = జ్ఞాపకం_ఇవ్వు(కొలత)))
		panic("out of memory");
	తిరిగి p;
}

స్థిర శూన్య *xzalloc(size_t కొలత)
{
	శూన్య *p = xmalloc(కొలత);
	memset(p, 0, కొలత);
	తిరిగి p;
}

స్థిర శూన్య *xrealloc(శూన్య *ptr, size_t కొలత)
{
	శూన్య *p = ఖాళీ;
	ఒకవేళ (!(p = realloc(ptr, కొలత)))
		panic("out of memory");
	తిరిగి p;
}

స్థిర శూన్య *xmemdup(స్థిరరాశి శూన్య *s, పూర్ణాంకం n)
{
	తిరిగి జ్ఞాపకం_నకలు(xmalloc(n), s, n);
}

#నిర్వచించు xstrdup		strdup
#నిర్వచించు xstrndup	strndup

స్థిర శూన్య *mempcpy(శూన్య *dest, స్థిరరాశి శూన్య *src, size_t len)
{
	తిరిగి జ్ఞాపకం_నకలు(dest, src, len) + len;
}

/* Like strcpy but can copy overlapping strings. */
స్థిర శూన్య overlapping_strcpy(అక్షరం *dst, స్థిరరాశి అక్షరం *src)
{
	/*
	 * Cheap optimization for dst == src case -
	 * better to have it here than in many callers.
	 */
	ఒకవేళ (dst != src) {
		అయితే ((*dst = *src) != '\0') {
			dst++;
			src++;
		}
	}
}

స్థిర అక్షరం* skip_whitespace(స్థిరరాశి అక్షరం *s)
{
	/*
	 * In POSIX/C locale (the only locale we care about: do we REALLY want
	 * to allow Unicode whitespace in, say, .conf files? nuts!)
	 * isspace is only these chars: "\t\n\v\f\r" and space.
	 * "\t\n\v\f\r" happen to have ASCII codes 9,10,11,12,13.
	 * Use that.
	 */
	అయితే (*s == ' ' || (గుర్తులేని అక్షరం)(*s - 9) <= (13 - 9))
		s++;

	తిరిగి (అక్షరం *) s;
}

స్థిర అక్షరం* skip_non_whitespace(స్థిరరాశి అక్షరం *s)
{
	అయితే (*s != '\0' && *s != ' ' && (గుర్తులేని అక్షరం)(*s - 9) > (13 - 9))
		s++;

	తిరిగి (అక్షరం *) s;
}

#నిర్వచించు is_name(c)	((c) == '_' || అక్షరమా((గుర్తులేని అక్షరం)(c)))
#నిర్వచించు is_లోపల_name(c)	((c) == '_' || అక్షరమా_అంకెనా((గుర్తులేని అక్షరం)(c)))

స్థిర స్థిరరాశి అక్షరం* endofname(స్థిరరాశి అక్షరం *name)
{
	ఒకవేళ (!is_name(*name))
		తిరిగి name;
	అయితే (*++name) {
		ఒకవేళ (!is_లోపల_name(*name))
			ఆపు;
	}
	తిరిగి name;
}

/**
 * list_size() - returns the number of elements in char ** before NULL.
 *
 * Argument must contain NULL to signalize its end.
 *
 * @list The list to count the number of element.
 * @return The number of element in list.
 */
స్థిర size_t list_కొలత(అక్షరం **list)
{
	size_t కొలత;

	కొరకు (కొలత = 0; list[కొలత] != ఖాళీ; కొలత++);

	తిరిగి కొలత;
}

స్థిర పూర్ణాంకం varcmp(స్థిరరాశి అక్షరం *p, స్థిరరాశి అక్షరం *q)
{
	పూర్ణాంకం c, d;

	అయితే ((c = *p) == (d = *q)) {
		ఒకవేళ (c == '\0' || c == '=')
			వెళ్ళు బహిర్గతం;
		p++;
		q++;
	}
	ఒకవేళ (c == '=')
		c = '\0';
	ఒకవేళ (d == '=')
		d = '\0';
బహిర్గతం:
	తిరిగి c - d;
}

నిర్మాణం లోపల_str;
స్థిర పూర్ణాంకం u_boot_cli_readline(నిర్మాణం లోపల_str *i);

నిర్మాణం లోపల_str;
స్థిర పూర్ణాంకం u_boot_cli_readline(నిర్మాణం లోపల_str *i);

/*
 * BusyBox globals which are needed for hush.
 */
స్థిర uint8_t xfunc_error_retval;

స్థిర స్థిరరాశి అక్షరం defifsvar[] __aligned(1) = "IFS= \t\n";
#నిర్వచించు defifs (defifsvar + 4)

/* This define is used to check if exit command was called. */
#నిర్వచించు EXIT_RET_సంకేతం -2

/*
 * This define is used for changes that need be done directly in the upstream
 * sources still. Ideally, its use should be minimized as much as possible.
 */
#నిర్వచించు __U_BOOT__

/*
 *
 * +-- Include of the upstream sources --+ *
 * V                                     V
 */
#అన్తర్భావ "cli_hush_upstream.c"
/*
 * A                                     A
 * +-- Include of the upstream sources --+ *
 *
 */

పూర్ణాంకం u_boot_hush_ప్రారంభం_modern(శూన్య)
{
	INIT_G();
	తిరిగి 0;
}

స్థిర పూర్ణాంకం u_boot_cli_readline(నిర్మాణం లోపల_str *i)
{
	అక్షరం *prompt;
	అక్షరం __maybe_unused *ps_prompt = ఖాళీ;

	ఒకవేళ (!G.promptmode)
		prompt = CONFIG_SYS_PROMPT;
#నిర్వచితమైతే CONFIG_SYS_PROMPT_HUSH_PS2
	లేకపోతే
		prompt = CONFIG_SYS_PROMPT_HUSH_PS2;
#లేకపోతే
	/* TODO: default value? */
	#దోషం "SYS_PROMPT_HUSH_PS2 is not defined!"
#అయితే_ముగింపు

	ఒకవేళ (CONFIG_IS_ENABLED(CMDLINE_PS_SUPPORT)) {
		ఒకవేళ (!G.promptmode)
			ps_prompt = env_get("PS1");
		లేకపోతే
			ps_prompt = env_get("PS2");

		ఒకవేళ (ps_prompt)
			prompt = ps_prompt;
	}

	తిరిగి cli_readline(prompt);
}
