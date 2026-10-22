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

#अन्तर्भाव <env.h>
#अन्तर्भाव <malloc.h>         /* malloc, free, realloc*/
#अन्तर्भाव <linux/ctype.h>    /* isalpha, isdigit */
#अन्तर्भाव <console.h>
#अन्तर्भाव <bootretry.h>
#अन्तर्भाव <cli.h>
#अन्तर्भाव <cli_hush.h>
#अन्तर्भाव <command.h>        /* find_cmd */
#अन्तर्भाव <asm/global_data.h>

/*
 * BusyBox Version: UPDATE THIS WHEN PULLING NEW UPSTREAM REVISION!
 */
#परिभाषय BB_VER			"1.37.0.git23da5c4b716b"

/*
 * Define hush features by the names used upstream.
 */
#परिभाषय ENABLE_HUSH_INTERACTIVE	1
#परिभाषय ENABLE_FEATURE_EDITING	1
#परिभाषय ENABLE_HUSH_IF		1
#परिभाषय ENABLE_HUSH_LOOPS	1
/* No MMU in U-Boot */
#परिभाषय BB_MMU			0
#परिभाषय USE_FOR_NOMMU(...)	__VA_ARGS__
#परिभाषय USE_FOR_MMU(...)

/*
 * Size-saving "small" ints (arch-dependent)
 */
#यदि CONFIG_IS_ENABLED(X86) || CONFIG_IS_ENABLED(X86_64) || CONFIG_IS_ENABLED(MIPS)
/* add other arches which benefit from this... */
प्रकारनिर्देश चिह्नित वर्ण smallint;
प्रकारनिर्देश अचिह्नित वर्ण smalluint;
#अन्यथा
/* for arches where byte accesses generate larger code: */
प्रकारनिर्देश पूर्णाङ्क smallint;
प्रकारनिर्देश अचिह्नित smalluint;
#यदि_अन्तः

/*
 * Alignment defines used by BusyBox.
 */
#परिभाषय ALIGN1			__attribute__((aligned(1)))
#परिभाषय ALIGN2			__attribute__((aligned(2)))
#परिभाषय ALIGN4			__attribute__((aligned(4)))
#परिभाषय ALIGN8			__attribute__((aligned(8)))
#परिभाषय ALIGN_PTR		__attribute__((aligned(परिमाण(शून्य*))))

/*
 * Miscellaneous compiler/platform defines.
 */
#परिभाषय FAST_FUNC /* not used in U-Boot */
#परिभाषय अप्रयुक्तम्_PARAM		__always_unused
#परिभाषय ALWAYS_INLINE		__always_inline
#परिभाषय NOINLINE		noinline

/*
 * Defines to provide equivalents to what libc/BusyBox defines.
 */
#परिभाषय EOF			(-1)
#परिभाषय EXIT_SUCCESS		0
#परिभाषय EXIT_FAILURE		1

/*
 * Stubs to provide libc/BusyBox functions based on U-Boot equivalents where it
 * makes sense.
 */
#परिभाषय utoa			simple_itoa

स्थिर शून्य __noreturn xfunc_die(शून्य)
{
	panic("HUSH died!");
}

#परिभाषय bb_error_सन्देशः_and_die(format, ...) कुरु { \
panic("HUSH: " format, __VA_ARGS__); \
} यावत् (0);

#परिभाषय bb_simple_error_सन्देशः_and_die(सन्देशः) कुरु { \
panic_str("HUSH: " सन्देशः); \
} यावत् (0);

/* fdprintf() is used for debug output. */
स्थिर पूर्णाङ्क __maybe_unused fdprintf(पूर्णाङ्क fd, ध्रुव वर्ण *format, ...)
{
	va_list तर्काः;
	uint i;

	प्रतिज्ञा(fd == 2);

	va_start(तर्काः, format);
	i = vprintf(format, तर्काः);
	va_अन्तः(तर्काः);

	प्रत्यागम i;
}

स्थिर शून्य bb_verror_सन्देशः(ध्रुव वर्ण *s, va_list p, ध्रुव वर्ण* strerr)
{
	/* TODO: what to do with strerr arg? */
	vprintf(s, p);
}

स्थिर शून्य bb_error_सन्देशः(ध्रुव वर्ण *s, ...)
{
	va_list p;

	va_start(p, s);
	bb_verror_सन्देशः(s, p, रिक्त);
	va_अन्तः(p);
}

स्थिर शून्य bb_simple_error_सन्देशः(ध्रुव वर्ण *s)
{
	bb_error_सन्देशः("%s", s);
}

स्थिर शून्य *xmalloc(size_t आकार)
{
	शून्य *p = रिक्त;
	यदि (!(p = स्मृतिं_देहि(आकार)))
		panic("out of memory");
	प्रत्यागम p;
}

स्थिर शून्य *xzalloc(size_t आकार)
{
	शून्य *p = xmalloc(आकार);
	स्मृतिं_पूरय(p, 0, आकार);
	प्रत्यागम p;
}

स्थिर शून्य *xrealloc(शून्य *ptr, size_t आकार)
{
	शून्य *p = रिक्त;
	यदि (!(p = realloc(ptr, आकार)))
		panic("out of memory");
	प्रत्यागम p;
}

स्थिर शून्य *xmemdup(ध्रुव शून्य *s, पूर्णाङ्क n)
{
	प्रत्यागम स्मृतिं_प्रतिलिख(xmalloc(n), s, n);
}

#परिभाषय xstrdup		strdup
#परिभाषय xstrndup	strndup

स्थिर शून्य *mempcpy(शून्य *dest, ध्रुव शून्य *src, size_t len)
{
	प्रत्यागम स्मृतिं_प्रतिलिख(dest, src, len) + len;
}

/* Like strcpy but can copy overlapping strings. */
स्थिर शून्य overlapping_strcpy(वर्ण *dst, ध्रुव वर्ण *src)
{
	/*
	 * Cheap optimization for dst == src case -
	 * better to have it here than in many callers.
	 */
	यदि (dst != src) {
		यावत् ((*dst = *src) != '\0') {
			dst++;
			src++;
		}
	}
}

स्थिर वर्ण* skip_whitespace(ध्रुव वर्ण *s)
{
	/*
	 * In POSIX/C locale (the only locale we care about: do we REALLY want
	 * to allow Unicode whitespace in, say, .conf files? nuts!)
	 * isspace is only these chars: "\t\n\v\f\r" and space.
	 * "\t\n\v\f\r" happen to have ASCII codes 9,10,11,12,13.
	 * Use that.
	 */
	यावत् (*s == ' ' || (अचिह्नित वर्ण)(*s - 9) <= (13 - 9))
		s++;

	प्रत्यागम (वर्ण *) s;
}

स्थिर वर्ण* skip_non_whitespace(ध्रुव वर्ण *s)
{
	यावत् (*s != '\0' && *s != ' ' && (अचिह्नित वर्ण)(*s - 9) > (13 - 9))
		s++;

	प्रत्यागम (वर्ण *) s;
}

#परिभाषय is_name(c)	((c) == '_' || अक्षरम्_वा((अचिह्नित वर्ण)(c)))
#परिभाषय is_अन्तर्_name(c)	((c) == '_' || अक्षरम्_अङ्कः_वा((अचिह्नित वर्ण)(c)))

स्थिर ध्रुव वर्ण* endofname(ध्रुव वर्ण *name)
{
	यदि (!is_name(*name))
		प्रत्यागम name;
	यावत् (*++name) {
		यदि (!is_अन्तर्_name(*name))
			भङ्ग;
	}
	प्रत्यागम name;
}

/**
 * list_size() - returns the number of elements in char ** before NULL.
 *
 * Argument must contain NULL to signalize its end.
 *
 * @list The list to count the number of element.
 * @return The number of element in list.
 */
स्थिर size_t list_आकार(वर्ण **list)
{
	size_t आकार;

	प्रति (आकार = 0; list[आकार] != रिक्त; आकार++);

	प्रत्यागम आकार;
}

स्थिर पूर्णाङ्क varcmp(ध्रुव वर्ण *p, ध्रुव वर्ण *q)
{
	पूर्णाङ्क c, d;

	यावत् ((c = *p) == (d = *q)) {
		यदि (c == '\0' || c == '=')
			गच्छ बहिर्गमनम्;
		p++;
		q++;
	}
	यदि (c == '=')
		c = '\0';
	यदि (d == '=')
		d = '\0';
बहिर्गमनम्:
	प्रत्यागम c - d;
}

संरचना अन्तर्_str;
स्थिर पूर्णाङ्क u_boot_cli_readline(संरचना अन्तर्_str *i);

संरचना अन्तर्_str;
स्थिर पूर्णाङ्क u_boot_cli_readline(संरचना अन्तर्_str *i);

/*
 * BusyBox globals which are needed for hush.
 */
स्थिर uint8_t xfunc_error_retval;

स्थिर ध्रुव वर्ण defifsvar[] __aligned(1) = "IFS= \t\n";
#परिभाषय defifs (defifsvar + 4)

/* This define is used to check if exit command was called. */
#परिभाषय EXIT_RET_संहिता -2

/*
 * This define is used for changes that need be done directly in the upstream
 * sources still. Ideally, its use should be minimized as much as possible.
 */
#परिभाषय __U_BOOT__

/*
 *
 * +-- Include of the upstream sources --+ *
 * V                                     V
 */
#अन्तर्भाव "cli_hush_upstream.c"
/*
 * A                                     A
 * +-- Include of the upstream sources --+ *
 *
 */

पूर्णाङ्क u_boot_hush_start_modern(शून्य)
{
	INIT_G();
	प्रत्यागम 0;
}

स्थिर पूर्णाङ्क u_boot_cli_readline(संरचना अन्तर्_str *i)
{
	वर्ण *prompt;
	वर्ण __maybe_unused *ps_prompt = रिक्त;

	यदि (!G.promptmode)
		prompt = CONFIG_SYS_PROMPT;
#यदि_परिभाषितम् CONFIG_SYS_PROMPT_HUSH_PS2
	अन्यथा
		prompt = CONFIG_SYS_PROMPT_HUSH_PS2;
#अन्यथा
	/* TODO: default value? */
	#दोषः "SYS_PROMPT_HUSH_PS2 is not defined!"
#यदि_अन्तः

	यदि (CONFIG_IS_ENABLED(CMDLINE_PS_SUPPORT)) {
		यदि (!G.promptmode)
			ps_prompt = env_get("PS1");
		अन्यथा
			ps_prompt = env_get("PS2");

		यदि (ps_prompt)
			prompt = ps_prompt;
	}

	प्रत्यागम cli_readline(prompt);
}
