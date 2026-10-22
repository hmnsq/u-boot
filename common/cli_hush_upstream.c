/* vi: set sw=4 ts=4: */
/*
 * A prototype Bourne shell grammar parser.
 * Intended to follow the original Thompson and Ritchie
 * "small and simple is beautiful" philosophy, which
 * incidentally is a good match to today's BusyBox.
 *
 * Copyright (C) 2000,2001  Larry Doolittle <larry@doolittle.boa.org>
 * Copyright (C) 2008,2009  Denys Vlasenko <vda.linux@googlemail.com>
 *
 * Licensed under GPLv2 or later, see file LICENSE in this source tree.
 *
 * Credits:
 *      The parser routines proper are all original material, first
 *      written Dec 2000 and Jan 2001 by Larry Doolittle.  The
 *      execution engine, the builtins, and much of the underlying
 *      support has been adapted from busybox-0.49pre's lash, which is
 *      Copyright (C) 1999-2004 by Erik Andersen <andersen@codepoet.org>
 *      written by Erik Andersen <andersen@codepoet.org>.  That, in turn,
 *      is based in part on ladsh.c, by Michael K. Johnson and Erik W.
 *      Troan, which they placed in the public domain.  I don't know
 *      how much of the Johnson/Troan code has survived the repeated
 *      rewrites.
 *
 * Other credits:
 *      o_addchr derived from similar w_addchar function in glibc-2.2.
 *      parse_redirect, redirect_opt_num, and big chunks of main
 *      and many builtins derived from contributions by Erik Andersen.
 *      Miscellaneous bugfixes from Matt Kraai.
 *
 * There are two big (and related) architecture differences between
 * this parser and the lash parser.  One is that this version is
 * actually designed from the ground up to understand nearly all
 * of the Bourne grammar.  The second, consequential change is that
 * the parser and input reader have been turned inside out.  Now,
 * the parser is in control, and asks for input as needed.  The old
 * way had the input reader in control, and it asked for parsing to
 * take place as needed.  The new way makes it much easier to properly
 * handle the recursion implicit in the various substitutions, especially
 * across continuation lines.
 *
 * TODOs:
 *      grep for "TODO" and fix (some of them are easy)
 *      make complex ${var%...} constructs support optional
 *      make here documents optional
 *      special variables (done: PWD, PPID, RANDOM)
 *      follow IFS rules more precisely, including update semantics
 *      tilde expansion
 *      aliases
 *      "command" missing features:
 *          command -p CMD: run CMD using default $PATH
 *              (can use this to override standalone shell as well?)
 *          command BLTIN: disables special-ness (e.g. errors do not abort)
 *          command -V CMD1 CMD2 CMD3 (multiple args) (not in standard)
 *      builtins mandated by standards we don't support:
 *          [un]alias, fc:
 *          fc -l[nr] [BEG] [END]: list range of commands in history
 *          fc [-e EDITOR] [BEG] [END]: edit/rerun range of commands
 *          fc -s [PAT=REP] [CMD]: rerun CMD, replacing PAT with REP
 *
 * Bash compat TODO:
 *      redirection of stdout+stderr: &> and >&
 *      reserved words: function select
 *      advanced test: [[ ]]
 *      process substitution: <(list) and >(list)
 *      let EXPR [EXPR...]
 *          Each EXPR is an arithmetic expression (ARITHMETIC EVALUATION)
 *          If the last arg evaluates to 0, let returns 1; 0 otherwise.
 *          NB: let `echo 'a=a + 1'` - error (IOW: multi-word expansion is used)
 *      ((EXPR))
 *          The EXPR is evaluated according to ARITHMETIC EVALUATION.
 *          This is exactly equivalent to let "EXPR".
 *      $[EXPR]: synonym for $((EXPR))
 *      indirect expansion: ${!VAR}
 *      substring op on @: ${@:n:m}
 *
 * Won't do:
 *      Some builtins mandated by standards:
 *          newgrp [GRP]: not a builtin in bash but a suid binary
 *              which spawns a new shell with new group ID
 *
 * Status of [[ support:
 * [[ args ]] are CMD_SINGLEWORD_NOGLOB:
 *   v='a b'; [[ $v = 'a b' ]]; echo 0:$?
 *   [[ /bin/n* ]]; echo 0:$?
 *   = is glob match operator, not equality operator: STR = GLOB
 *   == same as =
 *   =~ is regex match operator: STR =~ REGEX
 * TODO:
 * quoting needs to be considered (-f is an operator, "-f" and ""-f are not; etc)
 * in word = GLOB, quoting should be significant on char-by-char basis: a*cd"*"
 */
//config:config HUSH
//config:	bool "hush (70 kb)"
//config:	default y
//config:	select SHELL_HUSH
//config:	help
//config:	hush is a small shell. It handles the normal flow control
//config:	constructs such as if/then/elif/else/fi, for/in/do/done, while loops,
//config:	case/esac. Redirections, here documents, $((arithmetic))
//config:	and functions are supported.
//config:
//config:	It will compile and work on no-mmu systems.
//config:
//config:	It does not handle select, aliases, tilde expansion,
//config:	&>file and >&file redirection of stdout+stderr.
//config:
// This option is visible (has a description) to make it possible to select
// a "scripted" applet (such as NOLOGIN) but avoid selecting any shells:
//config:config SHELL_HUSH
//config:	bool "Internal shell for embedded script support"
//config:	default n
//config:
//config:# hush options
//config:# It's only needed to get "nice" menuconfig indenting.
//config:if SHELL_HUSH || HUSH || SH_IS_HUSH || BASH_IS_HUSH
//config:
//config:config HUSH_BASH_COMPAT
//config:	bool "bash-compatible extensions"
//config:	default y
//config:	depends on SHELL_HUSH
//config:
//config:config HUSH_BRACE_EXPANSION
//config:	bool "Brace expansion"
//config:	default y
//config:	depends on HUSH_BASH_COMPAT
//config:	help
//config:	Enable {abc,def} extension.
//config:
//config:config HUSH_BASH_SOURCE_CURDIR
//config:	bool "'source' and '.' builtins search current directory after $PATH"
//config:	default n   # do not encourage non-standard behavior
//config:	depends on HUSH_BASH_COMPAT
//config:	help
//config:	This is not compliant with standards. Avoid if possible.
//config:
//config:config HUSH_LINENO_VAR
//config:	bool "$LINENO variable (bashism)"
//config:	default y
//config:	depends on SHELL_HUSH
//config:
//config:config HUSH_INTERACTIVE
//config:	bool "Interactive mode"
//config:	default y
//config:	depends on SHELL_HUSH
//config:	help
//config:	Enable interactive mode (prompt and command editing).
//config:	Without this, hush simply reads and executes commands
//config:	from stdin just like a shell script from a file.
//config:	No prompt, no PS1/PS2 magic shell variables.
//config:
//config:config HUSH_SAVEHISTORY
//config:	bool "Save command history to .hush_history"
//config:	default y
//config:	depends on HUSH_INTERACTIVE && FEATURE_EDITING_SAVEHISTORY
//config:
//config:config HUSH_JOB
//config:	bool "Job control"
//config:	default y
//config:	depends on HUSH_INTERACTIVE
//config:	help
//config:	Enable job control: Ctrl-Z backgrounds, Ctrl-C interrupts current
//config:	command (not entire shell), fg/bg builtins work. Without this option,
//config:	"cmd &" still works by simply spawning a process and immediately
//config:	prompting for next command (or executing next command in a script),
//config:	but no separate process group is formed.
//config:
//config:config HUSH_TICK
//config:	bool "Support command substitution"
//config:	default y
//config:	depends on SHELL_HUSH
//config:	help
//config:	Enable `command` and $(command).
//config:
//config:config HUSH_IF
//config:	bool "Support if/then/elif/else/fi"
//config:	default y
//config:	depends on SHELL_HUSH
//config:
//config:config HUSH_LOOPS
//config:	bool "Support for, while and until loops"
//config:	default y
//config:	depends on SHELL_HUSH
//config:
//config:config HUSH_CASE
//config:	bool "Support case ... esac statement"
//config:	default y
//config:	depends on SHELL_HUSH
//config:	help
//config:	Enable case ... esac statement. +400 bytes.
//config:
//config:config HUSH_FUNCTIONS
//config:	bool "Support funcname() { commands; } syntax"
//config:	default y
//config:	depends on SHELL_HUSH
//config:	help
//config:	Enable support for shell functions. +800 bytes.
//config:
//config:config HUSH_LOCAL
//config:	bool "local builtin"
//config:	default y
//config:	depends on HUSH_FUNCTIONS
//config:	help
//config:	Enable support for local variables in functions.
//config:
//config:config HUSH_RANDOM_SUPPORT
//config:	bool "Pseudorandom generator and $RANDOM variable"
//config:	default y
//config:	depends on SHELL_HUSH
//config:	help
//config:	Enable pseudorandom generator and dynamic variable "$RANDOM".
//config:	Each read of "$RANDOM" will generate a new pseudorandom value.
//config:
//config:config HUSH_MODE_X
//config:	bool "Support 'hush -x' option and 'set -x' command"
//config:	default y
//config:	depends on SHELL_HUSH
//config:	help
//config:	This instructs hush to print commands before execution.
//config:	Adds ~300 bytes.
//config:
//config:config HUSH_ECHO
//config:	bool "echo builtin"
//config:	default y
//config:	depends on SHELL_HUSH
//config:
//config:config HUSH_PRINTF
//config:	bool "printf builtin"
//config:	default y
//config:	depends on SHELL_HUSH
//config:
//config:config HUSH_TEST
//config:	bool "test builtin"
//config:	default y
//config:	depends on SHELL_HUSH
//config:
//config:config HUSH_HELP
//config:	bool "help builtin"
//config:	default y
//config:	depends on SHELL_HUSH
//config:
//config:config HUSH_EXPORT
//config:	bool "export builtin"
//config:	default y
//config:	depends on SHELL_HUSH
//config:
//config:config HUSH_EXPORT_N
//config:	bool "Support 'export -n' option"
//config:	default y
//config:	depends on HUSH_EXPORT
//config:	help
//config:	export -n unexports variables. It is a bash extension.
//config:
//config:config HUSH_READONLY
//config:	bool "readonly builtin"
//config:	default y
//config:	depends on SHELL_HUSH
//config:	help
//config:	Enable support for read-only variables.
//config:
//config:config HUSH_KILL
//config:	bool "kill builtin (supports kill %jobspec)"
//config:	default y
//config:	depends on SHELL_HUSH
//config:
//config:config HUSH_WAIT
//config:	bool "wait builtin"
//config:	default y
//config:	depends on SHELL_HUSH
//config:
//config:config HUSH_COMMAND
//config:	bool "command builtin"
//config:	default y
//config:	depends on SHELL_HUSH
//config:
//config:config HUSH_TRAP
//config:	bool "trap builtin"
//config:	default y
//config:	depends on SHELL_HUSH
//config:
//config:config HUSH_TYPE
//config:	bool "type builtin"
//config:	default y
//config:	depends on SHELL_HUSH
//config:
//config:config HUSH_TIMES
//config:	bool "times builtin"
//config:	default y
//config:	depends on SHELL_HUSH
//config:
//config:config HUSH_READ
//config:	bool "read builtin"
//config:	default y
//config:	depends on SHELL_HUSH
//config:
//config:config HUSH_SET
//config:	bool "set builtin"
//config:	default y
//config:	depends on SHELL_HUSH
//config:
//config:config HUSH_UNSET
//config:	bool "unset builtin"
//config:	default y
//config:	depends on SHELL_HUSH
//config:
//config:config HUSH_ULIMIT
//config:	bool "ulimit builtin"
//config:	default y
//config:	depends on SHELL_HUSH
//config:
//config:config HUSH_UMASK
//config:	bool "umask builtin"
//config:	default y
//config:	depends on SHELL_HUSH
//config:
//config:config HUSH_GETOPTS
//config:	bool "getopts builtin"
//config:	default y
//config:	depends on SHELL_HUSH
//config:
//config:config HUSH_MEMLEAK
//config:	bool "memleak builtin (debugging)"
//config:	default n
//config:	depends on SHELL_HUSH
//config:
//config:endif # hush options

//applet:IF_HUSH(APPLET(hush, BB_DIR_BIN, BB_SUID_DROP))
//                       APPLET_ODDNAME:name  main  location    suid_type     help
//applet:IF_SH_IS_HUSH(  APPLET_ODDNAME(sh,   hush, BB_DIR_BIN, BB_SUID_DROP, hush))
//applet:IF_BASH_IS_HUSH(APPLET_ODDNAME(bash, hush, BB_DIR_BIN, BB_SUID_DROP, hush))

//kbuild:lib-$(CONFIG_SHELL_HUSH) += hush.o match.o shell_common.o
//kbuild:lib-$(CONFIG_HUSH_RANDOM_SUPPORT) += random.o

/* -i (interactive) is also accepted,
 * but does nothing, therefore not shown in help.
 * NOMMU-specific options are not meant to be used by users,
 * therefore we don't show them either.
 */
//usage:#define hush_trivial_usage
//usage:	"[-enxl] [-c 'SCRIPT' [ARG0 ARGS] | FILE ARGS | -s ARGS]"
//usage:#define hush_full_usage "\n\n"
//usage:	"Unix shell interpreter"

#اگر_غیر_تعریف_شدہ __U_BOOT__
#اگر !(defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) \
	|| defined(__APPLE__) \
    )
# include <malloc.h>   /* for malloc_trim */
#اگر_ختم
#شامل <glob.h>
/* #include <dmalloc.h> */
#اگر ENABLE_HUSH_CASE
# include <fnmatch.h>
#اگر_ختم
#شامل <sys/times.h>
#شامل <sys/utsname.h> /* for setting $HOSTNAME */

#شامل "busybox.h"  /* for APPLET_IS_NOFORK/NOEXEC */
#شامل "unicode.h"
#شامل "shell_common.h"
#شامل "math.h"
#شامل "match.h"
#اگر ENABLE_HUSH_RANDOM_SUPPORT
# include "random.h"
#ورنہ
# define CLEAR_RANDOM_T(rnd) ((خالی)0)
#اگر_ختم
#اگر_غیر_تعریف_شدہ O_CLOEXEC
# define O_CLOEXEC 0
#اگر_ختم
#اگر_غیر_تعریف_شدہ F_DUPFD_CLOEXEC
# define F_DUPFD_CLOEXEC F_DUPFD
#اگر_ختم

#اگر ENABLE_FEATURE_SH_EMBEDDED_SCRIPTS && !ENABLE_SHELL_ASH
# include "embedded_scripts.h"
#ورنہ
# define عدد_SCRIPTS 0
#اگر_ختم
#اگر_ختم /* !__U_BOOT__ */

/* So far, all bash compat is controlled by one config option */
/* Separate defines document which part of code implements what */
#تعریف_کرو BASH_نمونہ_SUBST ENABLE_HUSH_BASH_COMPAT
#تعریف_کرو BASH_SUBSTR        ENABLE_HUSH_BASH_COMPAT
#تعریف_کرو BASH_ماخذ        ENABLE_HUSH_BASH_COMPAT
#تعریف_کرو BASH_DOLLAR_SQUOTE ENABLE_HUSH_BASH_COMPAT
#تعریف_کرو BASH_HOSTNAME_VAR  ENABLE_HUSH_BASH_COMPAT
#تعریف_کرو BASH_EPOCH_VARS    ENABLE_HUSH_BASH_COMPAT
#تعریف_کرو BASH_TEST2         (ENABLE_HUSH_BASH_COMPAT && ENABLE_HUSH_TEST)
#تعریف_کرو BASH_READ_D        ENABLE_HUSH_BASH_COMPAT

/* Build knobs */
#تعریف_کرو LEAK_HUNTING 0
#تعریف_کرو BUILD_AS_NOMMU 0
/* Enable/disable sanity checks. Ok to enable in production,
 * only adds a bit of bloat. Set to >1 to get non-production level verbosity.
 * Keeping 1 for now even in released versions.
 */
#تعریف_کرو HUSH_DEBUG 1
/* Slightly bigger (+200 bytes), but faster hush.
 * So far it only enables a trick with counting SIGCHLDs and forks,
 * which allows us to do fewer waitpid's.
 * (we can detect a case where neither forks were done nor SIGCHLDs happened
 * and therefore waitpid will return the same result as last time)
 */
#تعریف_کرو ENABLE_HUSH_FAST 0
/* TODO: implement simplified code for users which do not need ${var%...} ops
 * So far ${var%...} ops are always enabled:
 */
#تعریف_کرو ENABLE_HUSH_DOLLAR_OPS 1

#اگر BUILD_AS_NOMMU
# undef BB_MMU
# undef USE_FOR_NOMMU
# undef USE_FOR_MMU
# define BB_MMU 0
# define USE_FOR_NOMMU(...) __VA_ARGS__
# define USE_FOR_MMU(...)
#اگر_ختم

#اگر_غیر_تعریف_شدہ __U_BOOT__
#شامل "NUM_APPLETS.h"
#اگر عدد_APPLETS == 1
/* STANDALONE does not make sense, and won't compile */
# undef ENABLE_FEATURE_SH_STANDALONE
# undef IF_FEATURE_SH_STANDALONE
# undef IF_NOT_FEATURE_SH_STANDALONE
# define ENABLE_FEATURE_SH_STANDALONE 0
# define IF_FEATURE_SH_STANDALONE(...)
# define IF_NOT_FEATURE_SH_STANDALONE(...) __VA_ARGS__
#اگر_ختم
#اگر_ختم /* __U_BOOT__ */

#اگر !ENABLE_HUSH_INTERACTIVE
# undef ENABLE_FEATURE_EDITING
# define ENABLE_FEATURE_EDITING 0
# undef ENABLE_FEATURE_EDITING_FANCY_PROMPT
# define ENABLE_FEATURE_EDITING_FANCY_PROMPT 0
# undef ENABLE_FEATURE_EDITING_SAVE_ON_EXIT
# define ENABLE_FEATURE_EDITING_SAVE_ON_EXIT 0
#اگر_ختم

/* Do we support ANY keywords? */
#اگر ENABLE_HUSH_IF || ENABLE_HUSH_LOOPS || ENABLE_HUSH_CASE
# define HAS_KEYWORDS 1
# define IF_HAS_KEYWORDS(...) __VA_ARGS__
# define IF_HAS_NO_KEYWORDS(...)
#ورنہ
# define HAS_KEYWORDS 0
# define IF_HAS_KEYWORDS(...)
# define IF_HAS_NO_KEYWORDS(...) __VA_ARGS__
#اگر_ختم

/* If you comment out one of these below, it will be #defined later
 * to perform debug printfs to stderr: */
#تعریف_کرو debug_printf(...)         کرو {} جب (0)
/* Finer-grained debug switches */
#تعریف_کرو debug_printf_parse(...)   کرو {} جب (0)
#تعریف_کرو debug_printf_heredoc(...) کرو {} جب (0)
#تعریف_کرو debug_print_درخت(a, b)    کرو {} جب (0)
#تعریف_کرو debug_printf_exec(...)    کرو {} جب (0)
#تعریف_کرو debug_printf_env(...)     کرو {} جب (0)
#تعریف_کرو debug_printf_jobs(...)    کرو {} جب (0)
#تعریف_کرو debug_printf_expand(...)  کرو {} جب (0)
#تعریف_کرو debug_printf_varexp(...)  کرو {} جب (0)
#تعریف_کرو debug_printf_glob(...)    کرو {} جب (0)
#تعریف_کرو debug_printf_redir(...)   کرو {} جب (0)
#تعریف_کرو debug_printf_list(...)    کرو {} جب (0)
#تعریف_کرو debug_printf_subst(...)   کرو {} جب (0)
#تعریف_کرو debug_printf_prompt(...)  کرو {} جب (0)
#تعریف_کرو debug_printf_clean(...)   کرو {} جب (0)

#تعریف_کرو ERR_PTR ((خالی*)(طویل)1)

#تعریف_کرو کام_حالت_FORMAT    "[%u] %-22s %.40s\n"

#تعریف_کرو _SPECIAL_VARS_STR     "_*@$!?#-"
#اگر_غیر_تعریف_شدہ __U_BOOT__
#تعریف_کرو SPECIAL_VARS_STR     ("_*@$!?#-" + 1)
#تعریف_کرو NUMERIC_SPECVARS_STR ("_*@$!?#-" + 3)
#ورنہ /* __U_BOOT__ */
#تعریف_کرو SPECIAL_VARS_STR     "*@$!?#-"
#تعریف_کرو NUMERIC_SPECVARS_STR "$!?#-"
#اگر_ختم /* __U_BOOT__ */
#اگر BASH_نمونہ_SUBST
/* Support / and // replace ops */
/* Note that // is stored as \ in "encoded" string representation */
# define VAR_ENCODED_SUBST_OPS      "\\/%#:-=+?"
# define VAR_SUBST_OPS             ("\\/%#:-=+?" + 1)
# define MINUS_PLUS_EQUAL_QUESTION ("\\/%#:-=+?" + 5)
#ورنہ
# define VAR_ENCODED_SUBST_OPS      "%#:-=+?"
# define VAR_SUBST_OPS              "%#:-=+?"
# define MINUS_PLUS_EQUAL_QUESTION ("%#:-=+?" + 3)
#اگر_ختم

#تعریف_کرو SPECIAL_VAR_علامت_STR "\3"
#تعریف_کرو SPECIAL_VAR_علامت       3
/* The "variable" with name "\1" emits string "\3". Testcase: "echo ^C" */
#تعریف_کرو SPECIAL_VAR_QUOTED_SVS   1

ساخت variable;

ساکن ثابت حرف hush_نسخہ_str[] ALIGN1 = "HUSH_VERSION="BB_VER;

/* This supports saving pointers malloced in vfork child,
 * to be freed in the parent.
 */
#اگر !BB_MMU
قسمتعریف ساخت nommu_save_t {
	ساخت variable *old_vars;
	حرف **argv;
	حرف **argv_from_re_execing;
} nommu_save_t;
#اگر_ختم

شمار {
	RES_NONE  = 0,
#اگر ENABLE_HUSH_IF
	RES_IF    ,
	RES_THEN  ,
	RES_ELIF  ,
	RES_ELSE  ,
	RES_FI    ,
#اگر_ختم
#اگر ENABLE_HUSH_LOOPS
	RES_FOR   ,
	RES_WHILE ,
	RES_UNTIL ,
	RES_DO    ,
	RES_مکمل  ,
#اگر_ختم
#اگر ENABLE_HUSH_LOOPS || ENABLE_HUSH_CASE
	RES_اندر    ,
#اگر_ختم
#اگر ENABLE_HUSH_CASE
	RES_CASE  ,
	/* three pseudo-keywords support contrived "case" syntax: */
	RES_CASE_اندر,   /* "case ... IN", turns into RES_MATCH when IN is observed */
	RES_مطابقت ,    /* "word)" */
	RES_CASE_BODY, /* "this command is inside CASE" */
	RES_ESAC  ,
#اگر_ختم
	RES_XXXX  ,
	RES_SNTX
};

قسمتعریف ساخت o_string {
	حرف *ڈیٹا;
	صحیح لمبائی; /* position where data is appended */
	صحیح maxlen;
	صحیح o_expflags;
	/* At least some part of the string was inside '' or "",
	 * possibly empty one: word"", wo''rd etc. */
	smallint has_quoted_part;
	smallint has_خالی_خانے_خانہ;
	smallint ended_اندر_ifs;
} o_string;
شمار {
	EXP_FLAG_SINGLEWORD     = 0x80, /* must be 0x80 */
	EXP_FLAG_GLOB           = 0x2,
	/* Protect newly added chars against globbing
	 * by prepending \ to *, ?, [, \ */
	EXP_FLAG_ESC_GLOB_CHARS = 0x1,
};
/* Used for initialization: o_string foo = NULL_O_STRING; */
#تعریف_کرو NULL_O_STRING { تہی }

#اگر_غیر_تعریف_شدہ debug_printf_parse
ساکن ثابت حرف *ثابت assignment_flag[] ALIGN_PTR = {
	"MAYBE_ASSIGNMENT",
	"DEFINITELY_ASSIGNMENT",
	"NOT_ASSIGNMENT",
	"WORD_IS_KEYWORD",
};
#اگر_ختم

/* We almost can use standard FILE api, but we need an ability to move
 * its fd when redirects coincide with it. No api exists for that
 * (RFE for it at https://sourceware.org/bugzilla/show_bug.cgi?id=21902).
 * HFILE is our internal alternative. Only supports reading.
 * Since we now can, we incorporate linked list of all opened HFILEs
 * into the struct (used to be a separate mini-list).
 */
قسمتعریف ساخت HFILE {
	حرف *cur;
	حرف *انتہا_2;
	ساخت HFILE *اگلا_hfile;
	صحیح fd;
	حرف buf[1024];
} HFILE;

قسمتعریف ساخت اندر_str {
	ثابت حرف *p;
	صحیح peek_buf[2];
	صحیح آخری_char;
	HFILE *file;
} اندر_str;

#اگر_غیر_تعریف_شدہ __U_BOOT__
/* The descrip member of this structure is only used to make
 * debugging output pretty */
ساکن ثابت ساخت {
	int32_t منوال;
	علامتی حرف default_fd;
	حرف descrip[3];
} redir_جدول[] ALIGN4 = {
	{ O_RDONLY,                  0, "<"  },
	{ O_CREAT|O_TRUNC|O_WRONLY,  1, ">"  },
	{ O_CREAT|O_APPEND|O_WRONLY, 1, ">>" },
	{ O_CREAT|O_RDWR,            1, "<>" },
	{ O_RDONLY,                  0, "<<" },
/* Should not be needed. Bogus default_fd helps in debugging */
/*	{ O_RDONLY,                 77, "<<" }, */
};

ساخت redir_struct {
	ساخت redir_struct *اگلا;
	حرف *rd_filename;          /* filename */
	صحیح rd_fd;                  /* fd to redirect */
	/* fd to redirect to, or -3 if rd_fd is to be closed (n>&-) */
	صحیح rd_dup;
	smallint rd_type;           /* (enum redir_type) */
	/* note: for heredocs, rd_filename contains heredoc delimiter,
	 * and subsequently heredoc itself; and rd_dup is a bitmask:
	 * bit 0: do we need to trim leading tabs?
	 * bit 1: is heredoc quoted (<<'delim' syntax) ?
	 */
};
قسمتعریف شمار redir_type {
	REDIRECT_داخلہ     = 0,
	REDIRECT_OVERWRITE = 1,
	REDIRECT_APPEND    = 2,
	REDIRECT_IO        = 3,
	REDIRECT_HEREDOC   = 4,
	REDIRECT_HEREDOC2  = 5, /* REDIRECT_HEREDOC after heredoc is loaded */

	REDIRFD_CLOSE      = -3,
	REDIRFD_SYNTAX_ERR = -2,
	REDIRFD_مقصد_FILE    = -1,
	/* otherwise, rd_fd is redirected to rd_dup */

	HEREDOC_SKIPTABS = 1,
	HEREDOC_QUOTED   = 2,
} redir_type;

#اگر_ختم /* !__U_BOOT__ */

ساخت command {
#اگر_غیر_تعریف_شدہ __U_BOOT__
	pid_t pid;                  /* 0 if exited */
#اگر_ختم /* !__U_BOOT__ */
	غیرعلامتی assignment_cnt;    /* how many argv[i] are assignments? */
#اگر ENABLE_HUSH_LINENO_VAR
	غیرعلامتی lineno;
#اگر_ختم
	smallint cmd_type;          /* CMD_xxx */
#تعریف_کرو CMD_معمول   0
#تعریف_کرو CMD_SUBSHELL 1
#اگر BASH_TEST2
/* used for "[[ EXPR ]]" */
# define CMD_TEST2_SINGLEWORD_NOGLOB 2
#اگر_ختم
#اگر BASH_TEST2 || ENABLE_HUSH_LOCAL || ENABLE_HUSH_EXPORT || ENABLE_HUSH_READONLY
/* used to prevent word splitting and globbing in "export v=t*" */
# define CMD_SINGLEWORD_NOGLOB 3
#اگر_ختم
#اگر ENABLE_HUSH_FUNCTIONS
# define CMD_FUNCDEF 4
#اگر_ختم

	smalluint cmd_exitcode;
	/* if non-NULL, this "command" is { list }, ( list ), or a compound statement */
	ساخت pipe *گروہ;
#اگر !BB_MMU
	حرف *گروہ_as_string;
#اگر_ختم
#اگر ENABLE_HUSH_FUNCTIONS
	ساخت function *بچہ_func;
/* This field is used to prevent a bug here:
 * while...do f1() {a;}; f1; f1() {b;}; f1; done
 * When we execute "f1() {a;}" cmd, we create new function and clear
 * cmd->group, cmd->group_as_string, cmd->argv[0].
 * When we execute "f1() {b;}", we notice that f1 exists,
 * and that its "parent cmd" struct is still "alive",
 * we put those fields back into cmd->xxx
 * (struct function has ->parent_cmd ptr to facilitate that).
 * When we loop back, we can execute "f1() {a;}" again and set f1 correctly.
 * Without this trick, loop would execute a;b;b;b;...
 * instead of correct sequence a;b;a;b;...
 * When command is freed, it severs the link
 * (sets ->child_func->parent_cmd to NULL).
 */
#اگر_ختم
#اگر_تعریف_شدہ __U_BOOT__
	صحیح argc; /* number of program arguments */
#اگر_ختم
	حرف **argv;                /* command name and arguments */
/* argv vector may contain variable references (^Cvar^C, ^C0^C etc)
 * and on execution these are substituted with their values.
 * Substitution can make _several_ words out of one argv[n]!
 * Example: argv[0]=='.^C*^C.' here: echo .$*.
 * References of the form ^C`cmd arg^C are `cmd arg` substitutions.
 */
#اگر_غیر_تعریف_شدہ __U_BOOT__
	ساخت redir_struct *redirects; /* I/O redirections */
#اگر_ختم /* !__U_BOOT__ */
};
/* Is there anything in this command at all? */
#اگر_غیر_تعریف_شدہ __U_BOOT__
#تعریف_کرو IS_NULL_CMD(cmd) \
	(!(cmd)->گروہ && !(cmd)->argv && !(cmd)->redirects)

#ورنہ /* __U_BOOT__ */
#تعریف_کرو IS_NULL_CMD(cmd) \
	(!(cmd)->گروہ && !(cmd)->argv)
#اگر_ختم /* __U_BOOT__ */
ساخت pipe {
	ساخت pipe *اگلا;
	صحیح عدد_cmds;               /* total number of commands in pipe */
#اگر_غیر_تعریف_شدہ __U_BOOT__
	صحیح alive_cmds;             /* number of commands running (not exited) */
	صحیح stopped_cmds;           /* number of commands alive, but stopped */
#اگر ENABLE_HUSH_کام
	غیرعلامتی jobid;             /* job number */
	pid_t pgrp;                 /* process group ID for the job */
	حرف *cmdtext;              /* name of job */
#اگر_ختم
#اگر_ختم /* !__U_BOOT__ */
	ساخت command *cmds;       /* array of commands in pipe */
	smallint followup;          /* PIPE_BG, PIPE_SEQ, PIPE_OR, PIPE_AND */
	IF_HAS_KEYWORDS(smallint pi_inverted;) /* "! cmd | cmd" */
	IF_HAS_KEYWORDS(smallint res_لفظ;) /* needed for if, for, while, until... */
};
قسمتعریف شمار pipe_style {
	PIPE_SEQ = 0,
	PIPE_AND = 1,
	PIPE_OR  = 2,
	PIPE_BG  = 3,
} pipe_style;
/* Is there anything in this pipe at all? */
#تعریف_کرو IS_NULL_PIPE(pi) \
	((pi)->عدد_cmds == 0 IF_HAS_KEYWORDS( && (pi)->res_لفظ == RES_NONE))

/* This holds pointers to the various results of parsing */
ساخت parse_context {
	/* linked list of pipes */
	ساخت pipe *list_سر;
	/* last pipe (being constructed right now) */
	ساخت pipe *pipe;
	/* last command in pipe (being constructed right now) */
	ساخت command *command;
#اگر_غیر_تعریف_شدہ __U_BOOT__
	/* last redirect in command->redirects list */
	ساخت redir_struct *pending_redirect;
#اگر_ختم /* !__U_BOOT__ */
	o_string لفظ;
#اگر !BB_MMU
	o_string as_string;
#اگر_ختم
	smallint is_assignment; /* 0:maybe, 1:yes, 2:no, 3:keyword */
#اگر HAS_KEYWORDS
	smallint ctx_res_w;
	smallint ctx_inverted; /* "! cmd | cmd" */
#اگر ENABLE_HUSH_CASE
	smallint ctx_dsemicolon; /* ";;" seen */
#اگر_ختم
	/* bitmask of FLAG_xxx, for figuring out valid reserved words */
	صحیح old_flag;
	/* group we are enclosed in:
	 * example: "if pipe1; pipe2; then pipe3; fi"
	 * when we see "if" or "then", we malloc and copy current context,
	 * and make ->stack point to it. then we parse pipeN.
	 * when closing "then" / fi" / whatever is found,
	 * we move list_head into ->stack->command->group,
	 * copy ->stack into current context, and delete ->stack.
	 * (parsing of { list } and ( list ) doesn't use this method)
	 */
	ساخت parse_context *ڈھیر;
#اگر_ختم
};
شمار {
	MAYBE_ASSIGNMENT      = 0,
	DEFINITELY_ASSIGNMENT = 1,
	NOT_ASSIGNMENT        = 2,
	/* Not an assignment, but next word may be: "if v=xyz cmd;" */
	لفظ_IS_KEYWORD       = 3,
};

#اگر_غیر_تعریف_شدہ __U_BOOT__
/* On program start, environ points to initial environment.
 * putenv adds new pointers into it, unsetenv removes them.
 * Neither of these (de)allocates the strings.
 * setenv allocates new strings in malloc space and does putenv,
 * and thus setenv is unusable (leaky) for shell's purposes */
#تعریف_کرو setenv(...) setenv_is_leaky_dont_use()
#اگر_ختم /* !__U_BOOT__ */
ساخت variable {
	ساخت variable *اگلا;
	حرف *varstr;        /* points to "name=" portion */
	صحیح max_len;         /* if > 0, name is part of initial env; else name is malloced */
#اگر_غیر_تعریف_شدہ __U_BOOT__
	uint16_t var_nest_level;
	smallint flg_export; /* putenv should be done on this var */
	smallint flg_read_only;
#اگر_ختم /* !__U_BOOT__ */
};

شمار {
	BC_BREAK = 1,
	BC_CONTINUE = 2,
};

#اگر ENABLE_HUSH_FUNCTIONS
ساخت function {
	ساخت function *اگلا;
	حرف *name;
	ساخت command *والد_cmd;
	ساخت pipe *body;
# اگر !BB_MMU
	حرف *body_as_string;
# endif
};
#اگر_ختم

/* set -/+o OPT support. (TODO: make it optional)
 * bash supports the following opts:
 * allexport       off
 * braceexpand     on
 * emacs           on
 * errexit         off
 * errtrace        off
 * functrace       off
 * hashall         on
 * histexpand      off
 * history         on
 * ignoreeof       off
 * interactive-comments    on
 * keyword         off
 * monitor         on
 * noclobber       off
 * noexec          off
 * noglob          off
 * nolog           off
 * notify          off
 * nounset         off
 * onecmd          off
 * physical        off
 * pipefail        off
 * posix           off
 * privileged      off
 * verbose         off
 * vi              off
 * xtrace          off
 */
ساکن ثابت حرف o_opt_لڑیاں[] ALIGN1 =
	"pipefail\0"
	"noexec\0"
	"errexit\0"
#اگر ENABLE_HUSH_منوال_X
	"xtrace\0"
#اگر_ختم
	;
شمار {
	OPT_O_PIPEFAIL,
	OPT_O_NOEXEC,
	OPT_O_ERREXIT,
#اگر ENABLE_HUSH_منوال_X
	OPT_O_XTRACE,
#اگر_ختم
	عدد_OPT_O
};

/* "Globals" within this file */
/* Sorted roughly by size (smaller offsets == smaller code) */
ساخت globals {
#اگر_غیر_تعریف_شدہ __U_BOOT__
	/* interactive_fd != 0 means we are an interactive shell.
	 * If we are, then saved_tty_pgrp can also be != 0, meaning
	 * that controlling tty is available. With saved_tty_pgrp == 0,
	 * job control still works, but terminal signals
	 * (^C, ^Z, ^Y, ^\) won't work at all, and background
	 * process groups can only be created with "cmd &".
	 * With saved_tty_pgrp != 0, hush will use tcsetpgrp()
	 * to give tty to the foreground process group,
	 * and will take it back when the group is stopped (^Z)
	 * or killed (^C).
	 */
#اگر ENABLE_HUSH_INTERACTIVE
	/* 'interactive_fd' is a fd# open to ctty, if we have one
	 * _AND_ if we decided to act interactively */
	صحیح interactive_fd;
	IF_NOT_FEATURE_EDITING_FANCY_PROMPT(حرف *PS1;)
# define G_interactive_fd (G.interactive_fd)
#ورنہ
# define G_interactive_fd 0
#اگر_ختم
#ورنہ /* __U_BOOT__ */
# define G_interactive_fd 0
#اگر_ختم /* __U_BOOT__ */
#اگر_غیر_تعریف_شدہ __U_BOOT__
#اگر ENABLE_FEATURE_EDITING
	سطر_داخلہ_t *سطر_داخلہ_state;
#اگر_ختم
	pid_t جڑ_pid;
	pid_t جڑ_ppid;
	pid_t آخری_bg_pid;
#اگر ENABLE_HUSH_RANDOM_SUPPORT
	random_t random_gen;
#اگر_ختم
#اگر ENABLE_HUSH_کام
	صحیح تسلسل_list_level;
	غیرعلامتی آخری_jobid;
	pid_t saved_tty_pgrp;
	ساخت pipe *کام_list;
# define G_saved_tty_pgrp (G.saved_tty_pgrp)
#ورنہ
# define G_saved_tty_pgrp 0
#اگر_ختم
#اگر_ختم /* !__U_BOOT__ */
	/* How deeply are we in context where "set -e" is ignored */
	صحیح errexit_گہرائی;
#اگر_غیر_تعریف_شدہ __U_BOOT__
	/* "set -e" rules (do we follow them correctly?):
	 * Exit if pipe, list, or compound command exits with a non-zero status.
	 * Shell does not exit if failed command is part of condition in
	 * if/while, part of && or || list except the last command, any command
	 * in a pipe but the last, or if the command's return value is being
	 * inverted with !. If a compound command other than a subshell returns a
	 * non-zero status because a command failed while -e was being ignored, the
	 * shell does not exit. A trap on ERR, if set, is executed before the shell
	 * exits [ERR is a bashism].
	 *
	 * If a compound command or function executes in a context where -e is
	 * ignored, none of the commands executed within are affected by the -e
	 * setting. If a compound command or function sets -e while executing in a
	 * context where -e is ignored, that setting does not have any effect until
	 * the compound command or the command containing the function call completes.
	 */

	حرف o_opt[عدد_OPT_O];
#اگر ENABLE_HUSH_منوال_X
# define G_x_منوال (G.o_opt[OPT_O_XTRACE])
#ورنہ
# define G_x_منوال 0
#اگر_ختم
	حرف opt_s;
	حرف opt_c;
#اگر_ختم /* !__U_BOOT__ */
#اگر ENABLE_HUSH_INTERACTIVE
	smallint promptmode; /* 0: PS1, 1: PS2 */
#اگر_ختم
	/* set by signal handler if SIGINT is received _and_ its trap is not set */
	smallint flag_SIGINT;
#اگر_غیر_تعریف_شدہ __U_BOOT__
#اگر ENABLE_HUSH_LOOPS
	smallint flag_break_continue;
#اگر_ختم
#اگر_ختم /* !__U_BOOT__ */
#اگر ENABLE_HUSH_FUNCTIONS
	/* 0: outside of a function (or sourced file)
	 * -1: inside of a function, ok to use return builtin
	 * 1: return is invoked, skip all till end of func
	 */
	smallint flag_return_اندر_پیش_رفت;
# define G_flag_return_اندر_پیش_رفت (G.flag_return_اندر_پیش_رفت)
#ورنہ
# define G_flag_return_اندر_پیش_رفت 0
#اگر_ختم
	smallint exiting; /* used to prevent EXIT trap recursion */
	/* These support $? */
	smalluint آخری_exitcode;
	smalluint expand_exitcode;
	smalluint آخری_bg_pid_exitcode;
#اگر_غیر_تعریف_شدہ __U_BOOT__
#اگر ENABLE_HUSH_سیٹ
	/* are global_argv and global_argv[1..n] malloced? (note: not [0]) */
	smalluint global_دلائل_malloced;
# define G_global_دلائل_malloced (G.global_دلائل_malloced)
#ورنہ
# define G_global_دلائل_malloced 0
#اگر_ختم
#اگر ENABLE_HUSH_BASH_COMPAT
	صحیح dead_کام_exitcode; /* for "wait -n" */
#اگر_ختم
#اگر_ختم /* !__U_BOOT__ */
	/* how many non-NULL argv's we have. NB: $# + 1 */
	صحیح global_argc;
	حرف **global_argv;
#اگر !BB_MMU
	حرف *argv0_for_re_execing;
#اگر_ختم
#اگر ENABLE_HUSH_LOOPS
#اگر_غیر_تعریف_شدہ __U_BOOT__
	غیرعلامتی گہرائی_break_continue;
#اگر_ختم /* !__U_BOOT__ */
	غیرعلامتی گہرائی_کا_loop;
#اگر_ختم
#اگر_غیر_تعریف_شدہ __U_BOOT__
#اگر ENABLE_HUSH_GETOPTS
	غیرعلامتی getopt_گنتی;
#اگر_ختم
#اگر_ختم /* !__U_BOOT__ */
	ثابت حرف *ifs;
#اگر_تعریف_شدہ __U_BOOT__
	صحیح flag_repeat;
	صحیح do_repeat;
	صحیح تسلسل_command_flags;
#اگر_ختم /* __U_BOOT__ */
	حرف *ifs_whitespace; /* = G.ifs or malloced */
#اگر_غیر_تعریف_شدہ __U_BOOT__
	ثابت حرف *cwd;
#اگر_ختم /* !__U_BOOT__ */
	ساخت variable *چوٹی_var;
	حرف **expanded_assignments;
	ساخت variable **shadowed_vars_pp;
	غیرعلامتی var_nest_level;
#اگر_غیر_تعریف_شدہ __U_BOOT__
#اگر ENABLE_HUSH_FUNCTIONS
# اگر ENABLE_HUSH_LOCAL
	غیرعلامتی func_nest_level; /* solely to prevent "local v" in non-functions */
# endif
	ساخت function *چوٹی_func;
#اگر_ختم
	/* Signal and trap handling */
#اگر ENABLE_HUSH_FAST
	غیرعلامتی گنتی_SIGCHLD;
	غیرعلامتی handled_SIGCHLD;
	smallint we_have_children;
#اگر_ختم
#اگر ENABLE_HUSH_LINENO_VAR
	غیرعلامتی parse_lineno;
	غیرعلامتی execute_lineno;
#اگر_ختم
	HFILE *HFILE_list;
	HFILE *HFILE_stdin;
	/* Which signals have non-DFL handler (even with no traps set)?
	 * Set at the start to:
	 * (SIGQUIT + maybe SPECIAL_INTERACTIVE_SIGS + maybe SPECIAL_JOBSTOP_SIGS)
	 * SPECIAL_INTERACTIVE_SIGS are cleared after fork.
	 * The rest is cleared right before execv syscalls.
	 * Other than these two times, never modified.
	 */
	غیرعلامتی special_sig_نقاب;
#اگر ENABLE_HUSH_کام
	غیرعلامتی fatal_sig_نقاب;
# define G_fatal_sig_نقاب (G.fatal_sig_نقاب)
#ورنہ
# define G_fatal_sig_نقاب 0
#اگر_ختم
#اگر ENABLE_HUSH_TRAP
	صحیح pre_trap_exitcode;
# اگر ENABLE_HUSH_FUNCTIONS
	صحیح return_exitcode;
# endif
	حرف **traps; /* char *traps[NSIG] */
# define G_traps G.traps
#ورنہ
# define G_traps ((حرف**)تہی)
#اگر_ختم
	sigset_t pending_سیٹ;
#اگر ENABLE_HUSH_MEMLEAK
	غیرعلامتی طویل memleak_قدر;
#اگر_ختم
#اگر ENABLE_HUSH_منوال_X
	غیرعلامتی x_منوال_گہرائی;
	/* "set -x" output should not be redirectable with subsequent 2>FILE.
	 * We dup fd#2 to x_mode_fd when "set -x" is executed, and use it
	 * for all subsequent output.
	 */
	صحیح x_منوال_fd;
	o_string x_منوال_buf;
#اگر_ختم
#اگر_ختم /* !__U_BOOT__ */
#اگر HUSH_DEBUG >= 2
	صحیح debug_indent;
#اگر_ختم
#اگر_غیر_تعریف_شدہ __U_BOOT__
	ساخت sigaction sa;
	حرف optstring_buf[حجم("eixcs")];
#اگر BASH_EPOCH_VARS
	حرف epoch_buf[حجم("%llu.nnnnnn") + حجم(طویل طویل)*3];
#اگر_ختم
#اگر ENABLE_FEATURE_EDITING
	حرف user_داخلہ_buf[CONFIG_FEATURE_EDITING_MAX_LEN];
#اگر_ختم
#اگر_ختم /* !__U_BOOT__ */
};
#اگر_تعریف_شدہ __U_BOOT__
ساخت globals *ptr_مقصد_globals;
#اگر_ختم /* __U_BOOT__ */
#تعریف_کرو G (*ptr_مقصد_globals)
/* Not #defining name to G.name - this quickly gets unwieldy
 * (too many defines). Also, I actually prefer to see when a variable
 * is global, thus "G." prefix is a useful hint */
#اگر_تعریف_شدہ __U_BOOT__
#تعریف_کرو سیٹ_PTR_مقصد_GLOBALS(x) کرو { \
	(*(ساخت globals**)&ptr_مقصد_globals) = (خالی*)(x); \
	barrier(); \
} جب (0)
#تعریف_کرو INIT_G() کرو { \
	سیٹ_PTR_مقصد_GLOBALS(xzalloc(حجم(G))); \
	G.promptmode = 1; \
} جب (0)
#ورنہ /* !__U_BOOT__ */
#تعریف_کرو INIT_G() کرو { \
	سیٹ_PTR_مقصد_GLOBALS(xzalloc(حجم(G))); \
	/* memset(&G.sa, 0, sizeof(G.sa)); */  \
	sigfillset(&G.sa.sa_نقاب); \
	G.sa.sa_flags = SA_RESTART; \
} جب (0)
#اگر_ختم /* !__U_BOOT__ */

#اگر_غیر_تعریف_شدہ __U_BOOT__
/* Function prototypes for builtins */
ساکن صحیح builtin_cd(حرف **argv) FAST_FUNC;
#اگر ENABLE_HUSH_ECHO
ساکن صحیح builtin_echo(حرف **argv) FAST_FUNC;
#اگر_ختم
ساکن صحیح builtin_eval(حرف **argv) FAST_FUNC;
ساکن صحیح builtin_exec(حرف **argv) FAST_FUNC;
ساکن صحیح builtin_exit(حرف **argv) FAST_FUNC;
#اگر ENABLE_HUSH_EXPORT
ساکن صحیح builtin_export(حرف **argv) FAST_FUNC;
#اگر_ختم
#اگر ENABLE_HUSH_READONLY
ساکن صحیح builtin_readonly(حرف **argv) FAST_FUNC;
#اگر_ختم
ساکن صحیح builtin_false(حرف **argv) FAST_FUNC;
#اگر ENABLE_HUSH_کام
ساکن صحیح builtin_fg_bg(حرف **argv) FAST_FUNC;
ساکن صحیح builtin_jobs(حرف **argv) FAST_FUNC;
#اگر_ختم
#اگر ENABLE_HUSH_GETOPTS
ساکن صحیح builtin_getopts(حرف **argv) FAST_FUNC;
#اگر_ختم
#اگر ENABLE_HUSH_HELP
ساکن صحیح builtin_help(حرف **argv) FAST_FUNC;
#اگر_ختم
#اگر MAX_HISTORY && ENABLE_FEATURE_EDITING
ساکن صحیح builtin_history(حرف **argv) FAST_FUNC;
#اگر_ختم
#اگر ENABLE_HUSH_LOCAL
ساکن صحیح builtin_local(حرف **argv) FAST_FUNC;
#اگر_ختم
#اگر ENABLE_HUSH_MEMLEAK
ساکن صحیح builtin_memleak(حرف **argv) FAST_FUNC;
#اگر_ختم
#اگر ENABLE_HUSH_PRINTF
ساکن صحیح builtin_printf(حرف **argv) FAST_FUNC;
#اگر_ختم
ساکن صحیح builtin_pwd(حرف **argv) FAST_FUNC;
#اگر ENABLE_HUSH_READ
ساکن صحیح builtin_read(حرف **argv) FAST_FUNC;
#اگر_ختم
#اگر ENABLE_HUSH_سیٹ
ساکن صحیح builtin_سیٹ(حرف **argv) FAST_FUNC;
#اگر_ختم
ساکن صحیح builtin_سرکاؤ(حرف **argv) FAST_FUNC;
ساکن صحیح builtin_ماخذ(حرف **argv) FAST_FUNC;
#اگر ENABLE_HUSH_TEST || BASH_TEST2
ساکن صحیح builtin_test(حرف **argv) FAST_FUNC;
#اگر_ختم
#اگر ENABLE_HUSH_TRAP
ساکن صحیح builtin_trap(حرف **argv) FAST_FUNC;
#اگر_ختم
#اگر ENABLE_HUSH_TYPE
ساکن صحیح builtin_type(حرف **argv) FAST_FUNC;
#اگر_ختم
#اگر ENABLE_HUSH_TIMES
ساکن صحیح builtin_times(حرف **argv) FAST_FUNC;
#اگر_ختم
ساکن صحیح builtin_true(حرف **argv) FAST_FUNC;
#اگر ENABLE_HUSH_UMASK
ساکن صحیح builtin_umask(حرف **argv) FAST_FUNC;
#اگر_ختم
#اگر ENABLE_HUSH_UNSET
ساکن صحیح builtin_unset(حرف **argv) FAST_FUNC;
#اگر_ختم
#اگر ENABLE_HUSH_KILL
ساکن صحیح builtin_kill(حرف **argv) FAST_FUNC;
#اگر_ختم
#اگر ENABLE_HUSH_WAIT
ساکن صحیح builtin_wait(حرف **argv) FAST_FUNC;
#اگر_ختم
#اگر ENABLE_HUSH_LOOPS
ساکن صحیح builtin_break(حرف **argv) FAST_FUNC;
ساکن صحیح builtin_continue(حرف **argv) FAST_FUNC;
#اگر_ختم
#اگر ENABLE_HUSH_FUNCTIONS
ساکن صحیح builtin_return(حرف **argv) FAST_FUNC;
#اگر_ختم

/* Table of built-in functions.  They can be forked or not, depending on
 * context: within pipes, they fork.  As simple commands, they do not.
 * When used in non-forking context, they can change global variables
 * in the parent shell process.  If forked, of course they cannot.
 * For example, 'unset foo | whatever' will parse and run, but foo will
 * still be set at the end. */
ساخت built_اندر_command {
	ثابت حرف *b_cmd;
	صحیح (*b_function)(حرف **argv) FAST_FUNC;
#اگر ENABLE_HUSH_HELP
	ثابت حرف *b_descr;
# define BLTIN(cmd, func, help) { cmd, func, help }
#ورنہ
# define BLTIN(cmd, func, help) { cmd, func }
#اگر_ختم
};

ساکن ثابت ساخت built_اندر_command bltins1[] ALIGN_PTR = {
	BLTIN("."        , builtin_ماخذ  , "Run commands in file"),
	BLTIN(":"        , builtin_true    , تہی),
#اگر ENABLE_HUSH_کام
	BLTIN("bg"       , builtin_fg_bg   , "Resume job in background"),
#اگر_ختم
#اگر ENABLE_HUSH_LOOPS
	BLTIN("break"    , builtin_break   , "Exit loop"),
#اگر_ختم
	BLTIN("cd"       , builtin_cd      , "Change directory"),
#اگر ENABLE_HUSH_LOOPS
	BLTIN("continue" , builtin_continue, "Start new loop iteration"),
#اگر_ختم
	BLTIN("eval"     , builtin_eval    , "Construct and run shell command"),
	BLTIN("exec"     , builtin_exec    , "Execute command, don't return to shell"),
	BLTIN("exit"     , builtin_exit    , تہی),
#اگر ENABLE_HUSH_EXPORT
	BLTIN("export"   , builtin_export  , "Set environment variables"),
#اگر_ختم
	BLTIN("false"    , builtin_false   , تہی),
#اگر ENABLE_HUSH_کام
	BLTIN("fg"       , builtin_fg_bg   , "Bring job to foreground"),
#اگر_ختم
#اگر ENABLE_HUSH_GETOPTS
	BLTIN("getopts"  , builtin_getopts , تہی),
#اگر_ختم
#اگر ENABLE_HUSH_HELP
	BLTIN("help"     , builtin_help    , تہی),
#اگر_ختم
#اگر MAX_HISTORY && ENABLE_FEATURE_EDITING
	BLTIN("history"  , builtin_history , "Show history"),
#اگر_ختم
#اگر ENABLE_HUSH_کام
	BLTIN("jobs"     , builtin_jobs    , "List jobs"),
#اگر_ختم
#اگر ENABLE_HUSH_KILL
	BLTIN("kill"     , builtin_kill    , "Send signals to processes"),
#اگر_ختم
#اگر ENABLE_HUSH_LOCAL
	BLTIN("local"    , builtin_local   , "Set local variables"),
#اگر_ختم
#اگر ENABLE_HUSH_MEMLEAK
	BLTIN("memleak"  , builtin_memleak , تہی),
#اگر_ختم
#اگر ENABLE_HUSH_READ
	BLTIN("read"     , builtin_read    , "Input into variable"),
#اگر_ختم
#اگر ENABLE_HUSH_READONLY
	BLTIN("readonly" , builtin_readonly, "Make variables read-only"),
#اگر_ختم
#اگر ENABLE_HUSH_FUNCTIONS
	BLTIN("return"   , builtin_return  , "Return from function"),
#اگر_ختم
#اگر ENABLE_HUSH_سیٹ
	BLTIN("set"      , builtin_سیٹ     , "Set positional parameters"),
#اگر_ختم
	BLTIN("shift"    , builtin_سرکاؤ   , "Shift positional parameters"),
#اگر BASH_ماخذ
	BLTIN("source"   , builtin_ماخذ  , تہی),
#اگر_ختم
#اگر ENABLE_HUSH_TIMES
	BLTIN("times"    , builtin_times   , تہی),
#اگر_ختم
#اگر ENABLE_HUSH_TRAP
	BLTIN("trap"     , builtin_trap    , "Trap signals"),
#اگر_ختم
	BLTIN("true"     , builtin_true    , تہی),
#اگر ENABLE_HUSH_TYPE
	BLTIN("type"     , builtin_type    , "Show command type"),
#اگر_ختم
#اگر ENABLE_HUSH_ULIMIT
	BLTIN("ulimit"   , shell_builtin_ulimit, "Control resource limits"),
#اگر_ختم
#اگر ENABLE_HUSH_UMASK
	BLTIN("umask"    , builtin_umask   , "Set file creation mask"),
#اگر_ختم
#اگر ENABLE_HUSH_UNSET
	BLTIN("unset"    , builtin_unset   , "Unset variables"),
#اگر_ختم
#اگر ENABLE_HUSH_WAIT
	BLTIN("wait"     , builtin_wait    , "Wait for process to finish"),
#اگر_ختم
};
/* These builtins won't be used if we are on NOMMU and need to re-exec
 * (it's cheaper to run an external program in this case):
 */
ساکن ثابت ساخت built_اندر_command bltins2[] ALIGN_PTR = {
#اگر ENABLE_HUSH_TEST
	BLTIN("["        , builtin_test    , تہی),
#اگر_ختم
#اگر BASH_TEST2
	BLTIN("[["       , builtin_test    , تہی),
#اگر_ختم
#اگر ENABLE_HUSH_ECHO
	BLTIN("echo"     , builtin_echo    , تہی),
#اگر_ختم
#اگر ENABLE_HUSH_PRINTF
	BLTIN("printf"   , builtin_printf  , تہی),
#اگر_ختم
	BLTIN("pwd"      , builtin_pwd     , تہی),
#اگر ENABLE_HUSH_TEST
	BLTIN("test"     , builtin_test    , تہی),
#اگر_ختم
};

#اگر_ختم /* !__U_BOOT__ */

/* Debug printouts.
 */
#اگر HUSH_DEBUG >= 2
/* prevent disasters with G.debug_indent < 0 */
# define indent() fdprintf(2, "%*s", (G.debug_indent * 2) & 0xff, "")
# define debug_enter() (G.debug_indent++)
# define debug_leave() (G.debug_indent--)
#ورنہ
# define indent()      ((خالی)0)
# define debug_enter() ((خالی)0)
# define debug_leave() ((خالی)0)
#اگر_ختم

#اگر_غیر_تعریف_شدہ debug_printf
# define debug_printf(...) (indent(), fdprintf(2, __VA_ARGS__))
#اگر_ختم

#اگر_غیر_تعریف_شدہ debug_printf_parse
# define debug_printf_parse(...) (indent(), fdprintf(2, __VA_ARGS__))
#اگر_ختم

#اگر_غیر_تعریف_شدہ debug_printf_heredoc
# define debug_printf_heredoc(...) (indent(), fdprintf(2, __VA_ARGS__))
#اگر_ختم

#اگر_غیر_تعریف_شدہ debug_printf_exec
#تعریف_کرو debug_printf_exec(...) (indent(), fdprintf(2, __VA_ARGS__))
#اگر_ختم

#اگر_غیر_تعریف_شدہ debug_printf_env
# define debug_printf_env(...) (indent(), fdprintf(2, __VA_ARGS__))
#اگر_ختم

#اگر_غیر_تعریف_شدہ debug_printf_jobs
# define debug_printf_jobs(...) (indent(), fdprintf(2, __VA_ARGS__))
# define DEBUG_JOBS 1
#ورنہ
# define DEBUG_JOBS 0
#اگر_ختم

#اگر_غیر_تعریف_شدہ debug_printf_expand
# define debug_printf_expand(...) (indent(), fdprintf(2, __VA_ARGS__))
# define DEBUG_EXPAND 1
#ورنہ
# define DEBUG_EXPAND 0
#اگر_ختم

#اگر_غیر_تعریف_شدہ debug_printf_varexp
# define debug_printf_varexp(...) (indent(), fdprintf(2, __VA_ARGS__))
#اگر_ختم

#اگر_غیر_تعریف_شدہ debug_printf_glob
# define debug_printf_glob(...) (indent(), fdprintf(2, __VA_ARGS__))
# define DEBUG_GLOB 1
#ورنہ
# define DEBUG_GLOB 0
#اگر_ختم

#اگر_غیر_تعریف_شدہ debug_printf_redir
# define debug_printf_redir(...) (indent(), fdprintf(2, __VA_ARGS__))
#اگر_ختم

#اگر_غیر_تعریف_شدہ debug_printf_list
# define debug_printf_list(...) (indent(), fdprintf(2, __VA_ARGS__))
#اگر_ختم

#اگر_غیر_تعریف_شدہ debug_printf_subst
# define debug_printf_subst(...) (indent(), fdprintf(2, __VA_ARGS__))
#اگر_ختم

#اگر_غیر_تعریف_شدہ debug_printf_prompt
# define debug_printf_prompt(...) (indent(), fdprintf(2, __VA_ARGS__))
#اگر_ختم

#اگر_غیر_تعریف_شدہ debug_printf_clean
# define debug_printf_clean(...) (indent(), fdprintf(2, __VA_ARGS__))
# define DEBUG_CLEAN 1
#ورنہ
# define DEBUG_CLEAN 0
#اگر_ختم

#اگر DEBUG_EXPAND
ساکن خالی debug_print_لڑیاں(ثابت حرف *سابقہ, حرف **vv)
{
	indent();
	fdprintf(2, "%s:\n", سابقہ);
	جب (*vv)
		fdprintf(2, " '%s'\n", *vv++);
}
#ورنہ
# define debug_print_لڑیاں(سابقہ, vv) ((خالی)0)
#اگر_ختم

/* Leak hunting. Use hush_leaktool.sh for post-processing.
 */
#اگر LEAK_HUNTING
ساکن خالی *xxmalloc(صحیح lineno, size_t سائز)
{
	خالی *ptr = xmalloc((سائز + 0xff) & ~0xff);
	fdprintf(2, "line %d: malloc %p\n", lineno, ptr);
	واپس ptr;
}
ساکن خالی *xxrealloc(صحیح lineno, خالی *ptr, size_t سائز)
{
	ptr = xrealloc(ptr, (سائز + 0xff) & ~0xff);
	fdprintf(2, "line %d: realloc %p\n", lineno, ptr);
	واپس ptr;
}
ساکن حرف *xxstrdup(صحیح lineno, ثابت حرف *str)
{
	حرف *ptr = xstrdup(str);
	fdprintf(2, "line %d: strdup %p\n", lineno, ptr);
	واپس ptr;
}
ساکن خالی xxfree(خالی *ptr)
{
	fdprintf(2, "free %p\n", ptr);
	آزاد(ptr);
}
# define xmalloc(s)     xxmalloc(__LINE__, s)
# define xrealloc(p, s) xxrealloc(__LINE__, p, s)
# define xstrdup(s)     xxstrdup(__LINE__, s)
# define آزاد(p)        xxfree(p)
#اگر_ختم

/* Syntax and runtime errors. They always abort scripts.
 * In interactive use they usually discard unparsed and/or unexecuted commands
 * and return to the prompt.
 * HUSH_DEBUG >= 2 prints line number in this file where it was detected.
 */
#اگر HUSH_DEBUG < 2
# define پیغام_and_die_if_script(lineno, ...)     پیغام_and_die_if_script(__VA_ARGS__)
# define syntax_error(lineno, پیغام)              syntax_error(پیغام)
# define syntax_error_at(lineno, پیغام)           syntax_error_at(پیغام)
# define syntax_error_unterm_ch(lineno, ch)     syntax_error_unterm_ch(ch)
# define syntax_error_unterm_str(lineno, s)     syntax_error_unterm_str(s)
# define syntax_error_unexpected_ch(lineno, ch) syntax_error_unexpected_ch(ch)
#اگر_ختم

ساکن خالی die_if_script(خالی)
{
	اگر (!G_interactive_fd) {
		اگر (G.آخری_exitcode) /* sometines it's 2, not 1 (bash compat) */
			xfunc_error_retval = G.آخری_exitcode;
		xfunc_die();
	}
}

#اگر_تعریف_شدہ __U_BOOT__
ساکن خالی __maybe_unused پیغام_and_die_if_script(غیرعلامتی lineno, ثابت حرف *fmt, ...)
#ورنہ /* !__U_BOOT__ */
ساکن خالی پیغام_and_die_if_script(غیرعلامتی lineno, ثابت حرف *fmt, ...)
#اگر_ختم /* !__U_BOOT__ */
{
	va_list p;

#اگر HUSH_DEBUG >= 2
	bb_error_پیغام("hush.c:%u", lineno);
#اگر_ختم
	va_آغاز(p, fmt);
	bb_verror_پیغام(fmt, p, تہی);
	va_انتہا(p);
	die_if_script();
}

ساکن خالی syntax_error(غیرعلامتی lineno غیر_مستعمل_PARAM, ثابت حرف *پیغام)
{
	اگر (پیغام)
		bb_error_پیغام("syntax error: %s", پیغام);
	ورنہ
		bb_simple_error_پیغام("syntax error");
	die_if_script();
}

ساکن خالی syntax_error_at(غیرعلامتی lineno غیر_مستعمل_PARAM, ثابت حرف *پیغام)
{
	bb_error_پیغام("syntax error at '%s'", پیغام);
	die_if_script();
}

ساکن خالی syntax_error_unterm_str(غیرعلامتی lineno غیر_مستعمل_PARAM, ثابت حرف *s)
{
	bb_error_پیغام("syntax error: unterminated %s", s);
//? source4.tests fails: in bash, echo ${^} in script does not terminate the script
// (but bash --posix, or if bash is run as "sh", does terminate in script, so maybe uncomment this?)
//	die_if_script();
}

ساکن خالی syntax_error_unterm_ch(غیرعلامتی lineno, حرف ch)
{
	حرف پیغام[2] = { ch, '\0' };
	syntax_error_unterm_str(lineno, پیغام);
}

ساکن خالی syntax_error_unexpected_ch(غیرعلامتی lineno غیر_مستعمل_PARAM, صحیح ch)
{
	حرف پیغام[2];
	پیغام[0] = ch;
	پیغام[1] = '\0';
#اگر HUSH_DEBUG >= 2
	bb_error_پیغام("hush.c:%u", lineno);
#اگر_ختم
	bb_error_پیغام("syntax error: unexpected %s", ch == EOF ? "EOF" : پیغام);
	die_if_script();
}

#اگر HUSH_DEBUG < 2
# undef پیغام_and_die_if_script
# undef syntax_error
# undef syntax_error_at
# undef syntax_error_unterm_ch
# undef syntax_error_unterm_str
# undef syntax_error_unexpected_ch
#ورنہ
# define پیغام_and_die_if_script(...)     پیغام_and_die_if_script(__LINE__, __VA_ARGS__)
# define syntax_error(پیغام)              syntax_error(__LINE__, پیغام)
# define syntax_error_at(پیغام)           syntax_error_at(__LINE__, پیغام)
# define syntax_error_unterm_ch(ch)     syntax_error_unterm_ch(__LINE__, ch)
# define syntax_error_unterm_str(s)     syntax_error_unterm_str(__LINE__, s)
# define syntax_error_unexpected_ch(ch) syntax_error_unexpected_ch(__LINE__, ch)
#اگر_ختم

/* Utility functions
 */
/* Replace each \x with x in place, return ptr past NUL. */
ساکن حرف *unbackslash(حرف *src)
{
#اگر_تعریف_شدہ __U_BOOT__
	حرف *dst = src = (حرف *)strchrnul(src, '\\');
#ورنہ /* !__U_BOOT__ */
	حرف *dst = src = strchrnul(src, '\\');
#اگر_ختم /* !__U_BOOT__ */
	جب (1) {
		اگر (*src == '\\') {
			src++;
			اگر (*src != '\0') {
				/* \x -> x */
				*dst++ = *src++;
				جاری;
			}
			/* else: "\<nul>". Do not delete this backslash.
			 * Testcase: eval 'echo ok\'
			 */
			*dst++ = '\\';
			/* fallthrough */
		}
		اگر ((*dst++ = *src++) == '\0')
			توڑو;
	}
	واپس dst;
}

ساکن حرف **جوڑو_لڑیاں_مقصد_لڑیاں(حرف **لڑیاں, حرف **جوڑو, صحیح need_مقصد_dup)
{
	صحیح i;
	غیرعلامتی count1;
	غیرعلامتی count2;
	حرف **v;

	v = لڑیاں;
	count1 = 0;
	اگر (v) {
		جب (*v) {
			count1++;
			v++;
		}
	}
	count2 = 0;
	v = جوڑو;
	جب (*v) {
		count2++;
		v++;
	}
	v = xrealloc(لڑیاں, (count1 + count2 + 1) * حجم(حرف*));
	v[count1 + count2] = تہی;
	i = count2;
	جب (--i >= 0)
		v[count1 + i] = (need_مقصد_dup ? xstrdup(جوڑو[i]) : جوڑو[i]);
	واپس v;
}
#اگر LEAK_HUNTING
ساکن حرف **xx_جوڑو_لڑیاں_مقصد_لڑیاں(صحیح lineno, حرف **لڑیاں, حرف **جوڑو, صحیح need_مقصد_dup)
{
	حرف **ptr = جوڑو_لڑیاں_مقصد_لڑیاں(لڑیاں, جوڑو, need_مقصد_dup);
	fdprintf(2, "line %d: add_strings_to_strings %p\n", lineno, ptr);
	واپس ptr;
}
#تعریف_کرو جوڑو_لڑیاں_مقصد_لڑیاں(لڑیاں, جوڑو, need_مقصد_dup) \
	xx_جوڑو_لڑیاں_مقصد_لڑیاں(__LINE__, لڑیاں, جوڑو, need_مقصد_dup)
#اگر_ختم

/* Note: takes ownership of "add" ptr (it is not strdup'ed) */
ساکن حرف **جوڑو_string_مقصد_لڑیاں(حرف **لڑیاں, حرف *جوڑو)
{
	حرف *v[2];
	v[0] = جوڑو;
	v[1] = تہی;
	واپس جوڑو_لڑیاں_مقصد_لڑیاں(لڑیاں, v, /*dup:*/ 0);
}

#اگر LEAK_HUNTING
ساکن حرف **xx_جوڑو_string_مقصد_لڑیاں(صحیح lineno, حرف **لڑیاں, حرف *جوڑو)
{
	حرف **ptr = جوڑو_string_مقصد_لڑیاں(لڑیاں, جوڑو);
	fdprintf(2, "line %d: add_string_to_strings %p\n", lineno, ptr);
	واپس ptr;
}
#تعریف_کرو جوڑو_string_مقصد_لڑیاں(لڑیاں, جوڑو) \
	xx_جوڑو_string_مقصد_لڑیاں(__LINE__, لڑیاں, جوڑو)
#اگر_ختم

ساکن خالی free_لڑیاں(حرف **لڑیاں)
{
	حرف **v;

	اگر (!لڑیاں)
		واپس;
	v = لڑیاں;
	جب (*v) {
		آزاد(*v);
		v++;
	}
	آزاد(لڑیاں);
}

#اگر_غیر_تعریف_شدہ __U_BOOT__
ساکن صحیح dup_CLOEXEC(صحیح fd, صحیح avoid_fd)
{
	صحیح newfd;
 repeat:
	newfd = fcntl(fd, F_DUPFD_CLOEXEC, avoid_fd + 1);
	اگر (newfd >= 0) {
		اگر (F_DUPFD_CLOEXEC == F_DUPFD) /* if old libc (w/o F_DUPFD_CLOEXEC) */
			close_on_exec_on(newfd);
	} ورنہ { /* newfd < 0 */
		اگر (errno == EBUSY)
			جاؤ repeat;
		اگر (errno == EINTR)
			جاؤ repeat;
		اگر (errno != EBADF) {
			/* "echo >&9999" gets EINVAL trying to save fd 1 to above 9999.
			 * We could try saving it _below_ 9999 instead (how?), but
			 * this probably means that dup2(9999,1) to effectuate >&9999
			 * would also not work: fd 9999 can't exist.
			 * (This differs from "echo >&99" where saving works, but
			 * subsequent dup2(99,1) fails if fd 99 is not open).
			 */
			bb_perror_پیغام("fcntl(%d,F_DUPFD,%d)", fd, avoid_fd + 1);
		}
	}
	واپس newfd;
}

ساکن صحیح xdup_CLOEXEC_and_close(صحیح fd, صحیح avoid_fd)
{
	صحیح newfd;
 repeat:
	newfd = fcntl(fd, F_DUPFD_CLOEXEC, avoid_fd + 1);
	اگر (newfd < 0) {
		اگر (errno == EBUSY)
			جاؤ repeat;
		اگر (errno == EINTR)
			جاؤ repeat;
		/* fd was not open? */
		اگر (errno == EBADF)
			واپس fd;
		xfunc_die();
	}
	اگر (F_DUPFD_CLOEXEC == F_DUPFD) /* if old libc (w/o F_DUPFD_CLOEXEC) */
		close_on_exec_on(newfd);
	close(fd);
	واپس newfd;
}

/* Manipulating HFILEs */
ساکن HFILE *hfopen(ثابت حرف *name)
{
	HFILE *fp;
	صحیح fd;

	fd = STDIN_FILENO;
	اگر (name) {
		fd = open(name, O_RDONLY | O_CLOEXEC);
		اگر (fd < 0)
			واپس تہی;
		اگر (O_CLOEXEC == 0) /* ancient libc */
			close_on_exec_on(fd);
	}

	fp = xmalloc(حجم(*fp));
	اگر (name == تہی)
		G.HFILE_stdin = fp;
	fp->fd = fd;
	fp->cur = fp->انتہا_2 = fp->buf;
	fp->اگلا_hfile = G.HFILE_list;
	G.HFILE_list = fp;
	واپس fp;
}
ساکن خالی hfclose(HFILE *fp)
{
	HFILE **pp = &G.HFILE_list;
	جب (*pp) {
		HFILE *cur = *pp;
		اگر (cur == fp) {
			*pp = cur->اگلا_hfile;
			توڑو;
		}
		pp = &cur->اگلا_hfile;
	}
	اگر (fp->fd >= 0)
		close(fp->fd);
	آزاد(fp);
}
ساکن صحیح refill_HFILE_and_getc(HFILE *fp)
{
	صحیح n;

	اگر (fp->fd < 0) {
		/* Already saw EOF */
		واپس EOF;
	}
#اگر ENABLE_HUSH_INTERACTIVE && !ENABLE_FEATURE_EDITING
	/* If user presses ^C, read() restarts after SIGINT (we use SA_RESTART).
	 * IOW: ^C will not immediately stop line input.
	 * But poll() is different: it does NOT restart after signals.
	 */
	اگر (fp == G.HFILE_stdin) {
		ساخت pollfd pfd[1];
		pfd[0].fd = fp->fd;
		pfd[0].events = POLLIN;
		n = poll(pfd, 1, -1);
		اگر (n < 0
		 /*&& errno == EINTR - assumed true */
		 && sigismember(&G.pending_سیٹ, SIGINT)
		) {
			واپس '\0';
		}
	}
#ورنہ
/* if FEATURE_EDITING=y, we do not use this routine for interactive input */
#اگر_ختم
	/* Try to buffer more input */
	n = محفوظ_read(fp->fd, fp->buf, حجم(fp->buf));
	اگر (n < 0) {
		bb_simple_perror_پیغام("read error");
		n = 0;
	}
	fp->cur = fp->buf;
	fp->انتہا_2 = fp->buf + n;
	اگر (n == 0) {
		/* EOF/error */
		close(fp->fd);
		fp->fd = -1;
		واپس EOF;
	}
	واپس (غیرعلامتی حرف)(*fp->cur++);
}
/* Inlined for common case of non-empty buffer.
 */
ساکن ALWAYS_INLINE صحیح hfgetc(HFILE *fp)
{
	اگر (fp->cur < fp->انتہا_2)
		واپس (غیرعلامتی حرف)(*fp->cur++);
	/* Buffer empty */
	واپس refill_HFILE_and_getc(fp);
}
ساکن صحیح move_HFILEs_on_redirect(صحیح fd, صحیح avoid_fd)
{
	HFILE *fl = G.HFILE_list;
	جب (fl) {
		اگر (fd == fl->fd) {
			/* We use it only on script files, they are all CLOEXEC */
			fl->fd = xdup_CLOEXEC_and_close(fd, avoid_fd);
			debug_printf_redir("redirect_fd %d: matches a script fd, moving it to %d\n", fd, fl->fd);
			واپس 1; /* "found and moved" */
		}
		fl = fl->اگلا_hfile;
	}
#اگر ENABLE_HUSH_منوال_X
	اگر (G.x_منوال_fd > 0 && fd == G.x_منوال_fd) {
		G.x_منوال_fd = xdup_CLOEXEC_and_close(fd, avoid_fd);
		واپس 1; /* "found and moved" */
	}
#اگر_ختم
	واپس 0; /* "not in the list" */
}
#اگر ENABLE_FEATURE_SH_STANDALONE && BB_MMU
ساکن خالی close_all_HFILE_list(خالی)
{
	HFILE *fl = G.HFILE_list;
	جب (fl) {
		/* hfclose would also free HFILE object.
		 * It is disastrous if we share memory with a vforked parent.
		 * I'm not sure we never come here after vfork.
		 * Therefore just close fd, nothing more.
		 *
		 * ">" instead of ">=": we don't close fd#0,
		 * interactive shell uses hfopen(NULL) as stdin input
		 * which has fl->fd == 0, but fd#0 gets redirected in pipes.
		 * If we'd close it here, then e.g. interactive "set | sort"
		 * with NOFORKed sort, would have sort's input fd closed.
		 */
		اگر (fl->fd > 0)
			/*hfclose(fl); - unsafe */
			close(fl->fd);
		fl = fl->اگلا_hfile;
	}
}
#اگر_ختم
ساکن صحیح fd_اندر_HFILEs(صحیح fd)
{
	HFILE *fl = G.HFILE_list;
	جب (fl) {
		اگر (fl->fd == fd)
			واپس 1;
		fl = fl->اگلا_hfile;
	}
	واپس 0;
}

#اگر_ختم /* !__U_BOOT__ */

/* Helpers for setting new $n and restoring them back
 */
قسمتعریف ساخت save_arg_t {
	حرف *sv_argv0;
	حرف **sv_g_argv;
	صحیح sv_g_argc;
#اگر_غیر_تعریف_شدہ __U_BOOT__
	IF_HUSH_سیٹ(smallint sv_g_malloced;)
#اگر_ختم /* !__U_BOOT__ */
} save_arg_t;

#اگر_غیر_تعریف_شدہ __U_BOOT__
ساکن خالی save_and_replace_G_دلائل(save_arg_t *sv, حرف **argv)
{
	sv->sv_argv0 = argv[0];
	sv->sv_g_argv = G.global_argv;
	sv->sv_g_argc = G.global_argc;
	IF_HUSH_سیٹ(sv->sv_g_malloced = G.global_دلائل_malloced;)

	argv[0] = G.global_argv[0]; /* retain $0 */
	G.global_argv = argv;
	IF_HUSH_سیٹ(G.global_دلائل_malloced = 0;)

	G.global_argc = 1 + string_array_len(argv + 1);
}

ساکن خالی restore_G_دلائل(save_arg_t *sv, حرف **argv)
{
#اگر ENABLE_HUSH_سیٹ
	اگر (G.global_دلائل_malloced) {
		/* someone ran "set -- arg1 arg2 ...", undo */
		حرف **pp = G.global_argv;
		جب (*++pp) /* note: does not free $0 */
			آزاد(*pp);
		آزاد(G.global_argv);
	}
#اگر_ختم
	argv[0] = sv->sv_argv0;
	G.global_argv = sv->sv_g_argv;
	G.global_argc = sv->sv_g_argc;
	IF_HUSH_سیٹ(G.global_دلائل_malloced = sv->sv_g_malloced;)
}
#اگر_ختم /* !__U_BOOT__ */

#اگر_غیر_تعریف_شدہ __U_BOOT__
/* Basic theory of signal handling in shell
 * ========================================
 * This does not describe what hush does, rather, it is current understanding
 * what it _should_ do. If it doesn't, it's a bug.
 * http://www.opengroup.org/onlinepubs/9699919799/utilities/V3_chap02.html#trap
 *
 * Signals are handled only after each pipe ("cmd | cmd | cmd" thing)
 * is finished or backgrounded. It is the same in interactive and
 * non-interactive shells, and is the same regardless of whether
 * a user trap handler is installed or a shell special one is in effect.
 * ^C or ^Z from keyboard seems to execute "at once" because it usually
 * backgrounds (i.e. stops) or kills all members of currently running
 * pipe.
 *
 * Wait builtin is interruptible by signals for which user trap is set
 * or by SIGINT in interactive shell.
 *
 * Trap handlers will execute even within trap handlers. (right?)
 *
 * User trap handlers are forgotten when subshell ("(cmd)") is entered,
 * except for handlers set to '' (empty string).
 *
 * If job control is off, backgrounded commands ("cmd &")
 * have SIGINT, SIGQUIT set to SIG_IGN.
 *
 * Commands which are run in command substitution ("`cmd`")
 * have SIGTTIN, SIGTTOU, SIGTSTP set to SIG_IGN.
 *
 * Ordinary commands have signals set to SIG_IGN/DFL as inherited
 * by the shell from its parent.
 *
 * Signals which differ from SIG_DFL action
 * (note: child (i.e., [v]forked) shell is not an interactive shell):
 *
 * SIGQUIT: ignore
 * SIGTERM (interactive): ignore
 * SIGHUP (interactive):
 *    Send SIGCONT to stopped jobs, send SIGHUP to all jobs and exit.
 *    Kernel would do this for us ("orphaned process group" handling
 *    according to POSIX) if we are a session leader and thus our death
 *    frees the controlling tty, but to be bash-compatible, we also do it
 *    for every interactive shell's death by SIGHUP.
 *    (Also, we need to restore tty pgrp, otherwise e.g. Midnight Commander
 *    backgrounds when hush started from it gets killed by SIGHUP).
 * SIGTTIN, SIGTTOU, SIGTSTP (if job control is on): ignore
 *    Note that ^Z is handled not by trapping SIGTSTP, but by seeing
 *    that all pipe members are stopped. Try this in bash:
 *    while :; do :; done - ^Z does not background it
 *    (while :; do :; done) - ^Z backgrounds it
 * SIGINT (interactive): wait for last pipe, ignore the rest
 *    of the command line, show prompt. NB: ^C does not send SIGINT
 *    to interactive shell while shell is waiting for a pipe,
 *    since shell is bg'ed (is not in foreground process group).
 *    Example 1: this waits 5 sec, but does not execute ls:
 *    "echo $$; sleep 5; ls -l" + "kill -INT <pid>"
 *    Example 2: this does not wait and does not execute ls:
 *    "echo $$; sleep 5 & wait; ls -l" + "kill -INT <pid>"
 *    Example 3: this does not wait 5 sec, but executes ls:
 *    "sleep 5; ls -l" + press ^C
 *    Example 4: this does not wait and does not execute ls:
 *    "sleep 5 & wait; ls -l" + press ^C
 *
 * (What happens to signals which are IGN on shell start?)
 * (What happens with signal mask on shell start?)
 *
 * Old implementation
 * ==================
 * We use in-kernel pending signal mask to determine which signals were sent.
 * We block all signals which we don't want to take action immediately,
 * i.e. we block all signals which need to have special handling as described
 * above, and all signals which have traps set.
 * After each pipe execution, we extract any pending signals via sigtimedwait()
 * and act on them.
 *
 * unsigned special_sig_mask: a mask of such "special" signals
 * sigset_t blocked_set:  current blocked signal set
 *
 * "trap - SIGxxx":
 *    clear bit in blocked_set unless it is also in special_sig_mask
 * "trap 'cmd' SIGxxx":
 *    set bit in blocked_set (even if 'cmd' is '')
 * after [v]fork, if we plan to be a shell:
 *    unblock signals with special interactive handling
 *    (child shell is not interactive),
 *    unset all traps except '' (note: regardless of child shell's type - {}, (), etc)
 * after [v]fork, if we plan to exec:
 *    POSIX says fork clears pending signal mask in child - no need to clear it.
 *    Restore blocked signal set to one inherited by shell just prior to exec.
 *
 * Note: as a result, we do not use signal handlers much. The only uses
 * are to count SIGCHLDs
 * and to restore tty pgrp on signal-induced exit.
 *
 * Note 2 (compat):
 * Standard says "When a subshell is entered, traps that are not being ignored
 * are set to the default actions". bash interprets it so that traps which
 * are set to '' (ignore) are NOT reset to defaults. We do the same.
 *
 * Problem: the above approach makes it unwieldy to catch signals while
 * we are in read builtin, or while we read commands from stdin:
 * masked signals are not visible!
 *
 * New implementation
 * ==================
 * We record each signal we are interested in by installing signal handler
 * for them - a bit like emulating kernel pending signal mask in userspace.
 * We are interested in: signals which need to have special handling
 * as described above, and all signals which have traps set.
 * Signals are recorded in pending_set.
 * After each pipe execution, we extract any pending signals
 * and act on them.
 *
 * unsigned special_sig_mask: a mask of shell-special signals.
 * unsigned fatal_sig_mask: a mask of signals on which we restore tty pgrp.
 * char *traps[sig] if trap for sig is set (even if it's '').
 * sigset_t pending_set: set of sigs we received.
 *
 * "trap - SIGxxx":
 *    if sig is in special_sig_mask, set handler back to:
 *        record_pending_signo, or to IGN if it's a tty stop signal
 *    if sig is in fatal_sig_mask, set handler back to sigexit.
 *    else: set handler back to SIG_DFL
 * "trap 'cmd' SIGxxx":
 *    set handler to record_pending_signo.
 * "trap '' SIGxxx":
 *    set handler to SIG_IGN.
 * after [v]fork, if we plan to be a shell:
 *    set signals with special interactive handling to SIG_DFL
 *    (because child shell is not interactive),
 *    unset all traps except '' (note: regardless of child shell's type - {}, (), etc)
 * after [v]fork, if we plan to exec:
 *    POSIX says fork clears pending signal mask in child - no need to clear it.
 *
 * To make wait builtin interruptible, we handle SIGCHLD as special signal,
 * otherwise (if we leave it SIG_DFL) sigsuspend in wait builtin will not wake up on it.
 *
 * Note (compat):
 * Standard says "When a subshell is entered, traps that are not being ignored
 * are set to the default actions". bash interprets it so that traps which
 * are set to '' (ignore) are NOT reset to defaults. We do the same.
 */
شمار {
	SPECIAL_INTERACTIVE_SIGS = 0
		| (1 << SIGTERM)
		| (1 << SIGINT)
		| (1 << SIGHUP)
		,
	SPECIAL_JOBSTOP_SIGS = 0
#اگر ENABLE_HUSH_کام
		| (1 << SIGTTIN)
		| (1 << SIGTTOU)
		| (1 << SIGTSTP)
#اگر_ختم
		,
};

ساکن خالی ریکارڈ_pending_signo(صحیح sig)
{
	sigaddset(&G.pending_سیٹ, sig);
#اگر ENABLE_FEATURE_EDITING
	اگر (sig != SIGCHLD
	 || (G_traps && G_traps[SIGCHLD] && G_traps[SIGCHLD][0])
	 /* ^^^ if SIGCHLD, interrupt line reading only if it has a trap */
	) {
		bb_got_signal = sig; /* for read_line_input: "we got a signal" */
	}
#اگر_ختم
#اگر ENABLE_HUSH_FAST
	اگر (sig == SIGCHLD) {
		G.گنتی_SIGCHLD++;
//bb_error_msg("[%d] SIGCHLD_handler: G.count_SIGCHLD:%d G.handled_SIGCHLD:%d", getpid(), G.count_SIGCHLD, G.handled_SIGCHLD);
	}
#اگر_ختم
}

ساکن sighandler_t install_sighandler(صحیح sig, sighandler_t handler)
{
	ساخت sigaction old_sa;

	/* We could use signal() to install handlers... almost:
	 * except that we need to mask ALL signals while handlers run.
	 * I saw signal nesting in strace, race window isn't small.
	 * SA_RESTART is also needed, but in Linux, signal()
	 * sets SA_RESTART too.
	 */
	/* memset(&G.sa, 0, sizeof(G.sa)); - already done */
	/* sigfillset(&G.sa.sa_mask);      - already done */
	/* G.sa.sa_flags = SA_RESTART;     - already done */
	G.sa.sa_handler = handler;
	sigaction(sig, &G.sa, &old_sa);
	واپس old_sa.sa_handler;
}
#اگر_ختم /* !__U_BOOT__ */

#اگر_غیر_تعریف_شدہ __U_BOOT__
ساکن خالی hush_exit(صحیح exitcode) NORETURN;

ساکن خالی restore_ttypgrp_and__exit(خالی) NORETURN;
ساکن خالی restore_ttypgrp_and__exit(خالی)
{
	/* xfunc has failed! die die die */
	/* no EXIT traps, this is an escape hatch! */
	G.exiting = 1;
	hush_exit(xfunc_error_retval);
}

#اگر ENABLE_HUSH_کام

/* Needed only on some libc:
 * It was observed that on exit(), fgetc'ed buffered data
 * gets "unwound" via lseek(fd, -NUM, SEEK_CUR).
 * With the net effect that even after fork(), not vfork(),
 * exit() in NOEXECed applet in "sh SCRIPT":
 *	noexec_applet_here
 *	echo END_OF_SCRIPT
 * lseeks fd in input FILE object from EOF to "e" in "echo END_OF_SCRIPT".
 * This makes "echo END_OF_SCRIPT" executed twice.
 * Similar problems can be seen with msg_and_die_if_script() -> xfunc_die()
 * and in `cmd` handling.
 * If set as die_func(), this makes xfunc_die() exit via _exit(), not exit():
 */
ساکن خالی fflush_and__exit(خالی) NORETURN;
ساکن خالی fflush_and__exit(خالی)
{
	fflush_all();
	_exit(xfunc_error_retval);
}

/* After [v]fork, in child: do not restore tty pgrp on xfunc death */
# define disable_restore_tty_pgrp_on_exit() (die_func = fflush_and__exit)
/* After [v]fork, in parent: restore tty pgrp on xfunc death */
# define enable_restore_tty_pgrp_on_exit()  (die_func = restore_ttypgrp_and__exit)

/* Restores tty foreground process group, and exits.
 * May be called as signal handler for fatal signal
 * (will resend signal to itself, producing correct exit state)
 * or called directly with -EXITCODE.
 * We also call it if xfunc is exiting.
 */
ساکن خالی sigexit(صحیح sig) NORETURN;
ساکن خالی sigexit(صحیح sig)
{
	/* Careful: we can end up here after [v]fork. Do not restore
	 * tty pgrp then, only top-level shell process does that */
	اگر (G_saved_tty_pgrp && getpid() == G.جڑ_pid) {
		/* Disable all signals: job control, SIGPIPE, etc.
		 * Mostly paranoid measure, to prevent infinite SIGTTOU.
		 */
		sigprocmask_allsigs(SIG_بلاک);
		tcsetpgrp(G_interactive_fd, G_saved_tty_pgrp);
	}

	/* Not a signal, just exit */
	اگر (sig <= 0)
		_exit(- sig);

	kill_myself_with_sig(sig); /* does not return */
}
#ورنہ

# define disable_restore_tty_pgrp_on_exit() ((خالی)0)
# define enable_restore_tty_pgrp_on_exit()  ((خالی)0)

#اگر_ختم
#اگر_ختم /* !__U_BOOT__ */

#اگر_غیر_تعریف_شدہ __U_BOOT__
ساکن sighandler_t pick_sighandler(غیرعلامتی sig)
{
	sighandler_t handler = SIG_DFL;
	اگر (sig < حجم(غیرعلامتی)*8) {
		غیرعلامتی sigmask = (1 << sig);

#اگر ENABLE_HUSH_کام
		/* is sig fatal? */
		اگر (G_fatal_sig_نقاب & sigmask)
			handler = sigexit;
		ورنہ
#اگر_ختم
		/* sig has special handling? */
		اگر (G.special_sig_نقاب & sigmask) {
			handler = ریکارڈ_pending_signo;
			/* TTIN/TTOU/TSTP can't be set to record_pending_signo
			 * in order to ignore them: they will be raised
			 * in an endless loop when we try to do some
			 * terminal ioctls! We do have to _ignore_ these.
			 */
			اگر (SPECIAL_JOBSTOP_SIGS & sigmask)
				handler = SIG_IGN;
		}
	}
	واپس handler;
}
#اگر_ختم /* !__U_BOOT__ */

#اگر_غیر_تعریف_شدہ __U_BOOT__
/* Restores tty foreground process group, and exits. */
ساکن خالی hush_exit(صحیح exitcode)
{
#اگر ENABLE_FEATURE_EDITING_SAVE_ON_EXIT
	save_history(G.سطر_داخلہ_state); /* may be NULL */
#اگر_ختم

	fflush_all();
	اگر (G.exiting <= 0 && G_traps && G_traps[0] && G_traps[0][0]) {
		حرف *argv[3];
		/* argv[0] is unused */
		argv[1] = xstrdup(G_traps[0]); /* copy, since EXIT trap handler may modify G_traps[0] */
		argv[2] = تہی;
		G.exiting = 1; /* prevent EXIT trap recursion */
		/* Note: G_traps[0] is not cleared!
		 * "trap" will still show it, if executed
		 * in the handler */
		builtin_eval(argv);
	}

#اگر ENABLE_FEATURE_CLEAN_اوپر
	{
		ساخت variable *cur_var;
		اگر (G.cwd != bb_پیغام_unknown)
			آزاد((حرف*)G.cwd);
		cur_var = G.چوٹی_var;
		جب (cur_var) {
			ساخت variable *tmp = cur_var;
			اگر (!cur_var->max_len)
				آزاد(cur_var->varstr);
			cur_var = cur_var->اگلا;
			آزاد(tmp);
		}
	}
#اگر_ختم

	fflush_all();
#اگر ENABLE_HUSH_کام
	sigexit(- (exitcode & 0xff));
#ورنہ
	_exit(exitcode);
#اگر_ختم
}

//TODO: return a mask of ALL handled sigs?
ساکن صحیح check_and_تسلسل_traps(خالی)
{
	صحیح آخری_sig = 0;

	جب (1) {
		صحیح sig;

		اگر (sigisemptyset(&G.pending_سیٹ))
			توڑو;
		sig = 0;
		کرو {
			sig++;
			اگر (sigismember(&G.pending_سیٹ, sig)) {
				sigdelset(&G.pending_سیٹ, sig);
				جاؤ got_sig;
			}
		} جب (sig < NSIG);
		توڑو;
 got_sig:
#اگر ENABLE_HUSH_TRAP
		اگر (G_traps && G_traps[sig]) {
			debug_printf_exec("%s: sig:%d handler:'%s'\n", __func__, sig, G.traps[sig]);
			اگر (G_traps[sig][0]) {
				/* We have user-defined handler */
				smalluint save_rcode;
				صحیح save_pre;
				حرف *argv[3];
				/* argv[0] is unused */
				argv[1] = xstrdup(G_traps[sig]);
				/* why strdup? trap can modify itself: trap 'trap "echo oops" INT' INT */
				argv[2] = تہی;
				save_pre = G.pre_trap_exitcode;
				G.pre_trap_exitcode = save_rcode = G.آخری_exitcode;
				builtin_eval(argv);
				آزاد(argv[1]);
				G.pre_trap_exitcode = save_pre;
				G.آخری_exitcode = save_rcode;
# اگر ENABLE_HUSH_FUNCTIONS
				اگر (G.return_exitcode >= 0) {
					debug_printf_exec("trap exitcode:%d\n", G.return_exitcode);
					G.آخری_exitcode = G.return_exitcode;
				}
# endif
				آخری_sig = sig;
			} /* else: "" trap, ignoring signal */
			جاری;
		}
#اگر_ختم
		/* not a trap: special action */
		انتخاب (sig) {
		حالت SIGINT:
			debug_printf_exec("%s: sig:%d default SIGINT handler\n", __func__, sig);
			G.flag_SIGINT = 1;
			آخری_sig = sig;
			توڑو;
#اگر ENABLE_HUSH_کام
		حالت SIGHUP: {
			/* if (G_interactive_fd) - no need to check, the handler
			 * is only installed if we *are* interactive */
			{
				/* bash compat: "Before exiting, an interactive
				 * shell resends the SIGHUP to all jobs, running
				 * or stopped.  Stopped jobs are sent SIGCONT
				 * to ensure that they receive the SIGHUP."
				 */
				ساخت pipe *کام;
				debug_printf_exec("%s: sig:%d default SIGHUP handler\n", __func__, sig);
				/* bash is observed to signal whole process groups,
				 * not individual processes */
				برائے (کام = G.کام_list; کام; کام = کام->اگلا) {
					اگر (کام->pgrp <= 0)
						جاری;
					debug_printf_exec("HUPing pgrp %d\n", کام->pgrp);
					اگر (kill(- کام->pgrp, SIGHUP) == 0)
						kill(- کام->pgrp, SIGCONT);
				}
			}
			/* this restores tty pgrp, then kills us with SIGHUP */
			sigexit(SIGHUP);
		}
#اگر_ختم
#اگر ENABLE_HUSH_FAST
		حالت SIGCHLD:
			debug_printf_exec("%s: sig:%d default SIGCHLD handler\n", __func__, sig);
			G.گنتی_SIGCHLD++;
//bb_error_msg("[%d] check_and_run_traps: G.count_SIGCHLD:%d G.handled_SIGCHLD:%d", getpid(), G.count_SIGCHLD, G.handled_SIGCHLD);
			/* Note:
			 * We don't do 'last_sig = sig' here -> NOT returning this sig.
			 * This simplifies wait builtin a bit.
			 */
			توڑو;
#اگر_ختم
		طےشدہ: /* ignored: */
			debug_printf_exec("%s: sig:%d default handling is to ignore\n", __func__, sig);
			/* SIGTERM, SIGQUIT, SIGTTIN, SIGTTOU, SIGTSTP */
			/* Note:
			 * We don't do 'last_sig = sig' here -> NOT returning this sig.
			 * Example: wait is not interrupted by TERM
			 * in interactive shell, because TERM is ignored.
			 */
			توڑو;
		}
	}
	واپس آخری_sig;
}

ساکن ثابت حرف *get_cwd(صحیح force)
{
	اگر (force || G.cwd == تہی) {
		/* xrealloc_getcwd_or_warn(arg) calls free(arg),
		 * we must not try to free(bb_msg_unknown) */
		اگر (G.cwd == bb_پیغام_unknown)
			G.cwd = تہی;
		G.cwd = xrealloc_getcwd_or_warn((حرف *)G.cwd);
		اگر (!G.cwd)
			G.cwd = bb_پیغام_unknown;
	}
	واپس G.cwd;
}

#اگر_ختم /* !__U_BOOT__ */

/*
 * Shell and environment variable support
 */
ساکن ساخت variable **get_ptr_مقصد_local_var(ثابت حرف *name)
{
	ساخت variable **pp;
	ساخت variable *cur;

	pp = &G.چوٹی_var;
	جب ((cur = *pp) != تہی) {
		اگر (varcmp(cur->varstr, name) == 0)
			واپس pp;
		pp = &cur->اگلا;
	}
	واپس تہی;
}

ساکن ثابت حرف* FAST_FUNC get_local_var_قدر(ثابت حرف *name)
{
	ساخت variable **vpp;

	اگر (G.expanded_assignments) {
		حرف **cpp = G.expanded_assignments;
		جب (*cpp) {
			حرف *cp = *cpp;
			اگر (varcmp(cp, name) == 0)
				واپس strchr(cp, '=') + 1;
			cpp++;
		}
	}

	vpp = get_ptr_مقصد_local_var(name);
	اگر (vpp)
		واپس strchr((*vpp)->varstr, '=') + 1;

#اگر_غیر_تعریف_شدہ __U_BOOT__
	اگر (strcmp(name, "PPID") == 0)
		واپس utoa(G.جڑ_ppid);
#اگر_ختم /* !__U_BOOT__ */
	// bash compat: UID? EUID?
#اگر ENABLE_HUSH_RANDOM_SUPPORT
	اگر (strcmp(name, "RANDOM") == 0)
		واپس utoa(اگلا_random(&G.random_gen));
#اگر_ختم
#اگر ENABLE_HUSH_LINENO_VAR
	اگر (strcmp(name, "LINENO") == 0)
		واپس utoa(G.execute_lineno);
#اگر_ختم
#اگر BASH_EPOCH_VARS
	{
		ثابت حرف *fmt = تہی;
		اگر (strcmp(name, "EPOCHSECONDS") == 0)
			fmt = "%llu";
		ورنہ اگر (strcmp(name, "EPOCHREALTIME") == 0)
			fmt = "%llu.%06u";
		اگر (fmt) {
			ساخت timeval tv;
			xgettimeofday(&tv);
			sprintf(G.epoch_buf, fmt, (غیرعلامتی طویل طویل)tv.tv_sec,
					(غیرعلامتی)tv.tv_usec);
			واپس G.epoch_buf;
		}
	}
#اگر_ختم
	واپس تہی;
}

#اگر_غیر_تعریف_شدہ __U_BOOT__
#اگر ENABLE_HUSH_GETOPTS
ساکن خالی handle_بدلا_special_نام(ثابت حرف *name)
{
	اگر (varcmp(name, "OPTIND") == 0) {
		G.getopt_گنتی = 0;
		واپس;
	}
}
#ورنہ
/* Do not even bother evaluating arguments */
# define handle_بدلا_special_نام(...) ((خالی)0)
#اگر_ختم
#ورنہ /* __U_BOOT__ */
/* Do not even bother evaluating arguments */
# define handle_بدلا_special_نام(...) ((خالی)0)
#اگر_ختم /* __U_BOOT__ */

/* str holds "NAME=VAL" and is expected to be malloced.
 * We take ownership of it.
 */
#اگر_غیر_تعریف_شدہ __U_BOOT__
#تعریف_کرو SETFLAG_EXPORT   (1 << 0)
#تعریف_کرو SETFLAG_UNEXPORT (1 << 1)
#تعریف_کرو SETFLAG_MAKE_RO  (1 << 2)
#اگر_ختم /* !__U_BOOT__ */
#تعریف_کرو SETFLAG_VARLVL_سرکاؤ   3
#اگر_غیر_تعریف_شدہ __U_BOOT__
ساکن صحیح سیٹ_local_var(حرف *str, غیرعلامتی flags)
#ورنہ /* __U_BOOT__ */
صحیح سیٹ_local_var_modern(حرف *str, صحیح flags)
#اگر_ختم /* __U_BOOT__ */
{
	ساخت variable **cur_pp;
	ساخت variable *cur;
	حرف *free_me = تہی;
	حرف *eq_sign;
	صحیح name_len;
	صحیح retval;
#اگر_غیر_تعریف_شدہ __U_BOOT__
	غیرعلامتی local_lvl = (flags >> SETFLAG_VARLVL_سرکاؤ);
#اگر_ختم /* !__U_BOOT__ */

	eq_sign = strchr(str, '=');
	اگر (HUSH_DEBUG && !eq_sign)
		bb_simple_error_پیغام_and_die("BUG in setvar");

	name_len = eq_sign - str + 1; /* including '=' */
	cur_pp = &G.چوٹی_var;
	جب ((cur = *cur_pp) != تہی) {
		اگر (لڑی_موازنہ(cur->varstr, str, name_len) != 0) {
			cur_pp = &cur->اگلا;
			جاری;
		}

#اگر_غیر_تعریف_شدہ __U_BOOT__
		/* We found an existing var with this name */
		اگر (cur->flg_read_only) {
			bb_error_پیغام("%s: readonly variable", str);
			آزاد(str);
//NOTE: in bash, assignment in "export READONLY_VAR=Z" fails, and sets $?=1,
//but export per se succeeds (does put the var in env). We don't mimic that.
			واپس -1;
		}
		اگر (flags & SETFLAG_UNEXPORT) { // && cur->flg_export ?
			debug_printf_env("%s: unsetenv '%s'\n", __func__, str);
			*eq_sign = '\0';
			unsetenv(str);
			*eq_sign = '=';
		}
		اگر (cur->var_nest_level < local_lvl) {
			/* bash 3.2.33(1) and exported vars:
			 * # export z=z
			 * # f() { local z=a; env | grep ^z; }
			 * # f
			 * z=a
			 * # env | grep ^z
			 * z=z
			 */
			اگر (cur->flg_export)
				flags |= SETFLAG_EXPORT;
			/* New variable is local ("local VAR=VAL" or
			 * "VAR=VAL cmd")
			 * and existing one is global, or local
			 * on a lower level that new one.
			 * Remove it from global variable list:
			 */
			*cur_pp = cur->اگلا;
			اگر (G.shadowed_vars_pp) {
				/* Save in "shadowed" list */
				debug_printf_env("shadowing %s'%s'/%u by '%s'/%u\n",
					cur->flg_export ? "exported " : "",
					cur->varstr, cur->var_nest_level, str, local_lvl
				);
				cur->اگلا = *G.shadowed_vars_pp;
				*G.shadowed_vars_pp = cur;
			} ورنہ {
				/* Came from pseudo_exec_argv(), no need to save: delete it */
				debug_printf_env("shadow-deleting %s'%s'/%u by '%s'/%u\n",
					cur->flg_export ? "exported " : "",
					cur->varstr, cur->var_nest_level, str, local_lvl
				);
				اگر (cur->max_len == 0) /* allocated "VAR=VAL"? */
					free_me = cur->varstr; /* then free it later */
				آزاد(cur);
			}
			توڑو;
		}
#اگر_ختم /* !__U_BOOT__ */

		اگر (strcmp(cur->varstr + name_len, eq_sign + 1) == 0) {
			debug_printf_env("assignement '%s' does not change anything\n", str);
 free_and_exp:
			آزاد(str);
			جاؤ قوت_نما;
		}

		/* Replace the value in the found "struct variable" */
		اگر (cur->max_len != 0) {
			اگر (cur->max_len >= strnlen(str, cur->max_len + 1)) {
				/* This one is from startup env, reuse space */
				debug_printf_env("reusing startup env for '%s'\n", str);
				strcpy(cur->varstr, str);
				جاؤ free_and_exp;
			}
			/* Can't reuse */
			cur->max_len = 0;
			جاؤ سیٹ_str_and_exp;
		}
		/* max_len == 0 signifies "malloced" var, which we can
		 * (and have to) free. But we can't free(cur->varstr) here:
		 * if cur->flg_export is 1, it is in the environment.
		 * We should either unsetenv+free, or wait until putenv,
		 * then putenv(new)+free(old).
		 */
		free_me = cur->varstr;
		جاؤ سیٹ_str_and_exp;
	}

	/* Not found or shadowed - create new variable struct */
#اگر_غیر_تعریف_شدہ __U_BOOT__
	debug_printf_env("%s: alloc new var '%s'/%u\n", __func__, str, local_lvl);
#ورنہ /* __U_BOOT__ */
	debug_printf_env("%s: alloc new var '%s'\n", __func__, str);
#اگر_ختم /* __U_BOOT__ */
	cur = xzalloc(حجم(*cur));
#اگر_غیر_تعریف_شدہ __U_BOOT__
	cur->var_nest_level = local_lvl;
#اگر_ختم /* !__U_BOOT__ */
	cur->اگلا = *cur_pp;
	*cur_pp = cur;

 سیٹ_str_and_exp:
	cur->varstr = str;
 قوت_نما:
#اگر_غیر_تعریف_شدہ __U_BOOT__
#اگر !BB_MMU || ENABLE_HUSH_READONLY
	اگر (flags & SETFLAG_MAKE_RO) {
		cur->flg_read_only = 1;
	}
#اگر_ختم
	اگر (flags & SETFLAG_EXPORT)
		cur->flg_export = 1;
#اگر_ختم /* !__U_BOOT__ */
	retval = 0;
#اگر_غیر_تعریف_شدہ __U_BOOT__
	اگر (cur->flg_export) {
		اگر (flags & SETFLAG_UNEXPORT) {
			cur->flg_export = 0;
			/* unsetenv was already done */
		} ورنہ {
			debug_printf_env("%s: putenv '%s'/%u\n", __func__, cur->varstr, cur->var_nest_level);
			retval = putenv(cur->varstr);
			/* fall through to "free(free_me)" -
			 * only now we can free old exported malloced string
			 */
		}
	}
#اگر_ختم /* !__U_BOOT__ */
	آزاد(free_me);

	handle_بدلا_special_نام(cur->varstr);

	واپس retval;
}

#اگر_غیر_تعریف_شدہ __U_BOOT__
ساکن صحیح سیٹ_local_var0(حرف *str)
{
	واپس سیٹ_local_var(str, 0);
}

ساکن خالی FAST_FUNC سیٹ_local_var_from_halves(ثابت حرف *name, ثابت حرف *val)
{
	حرف *var = xasprintf("%s=%s", name, val);
	سیٹ_local_var0(var);
}

/* Used at startup and after each cd */
ساکن خالی سیٹ_pwd_var(غیرعلامتی flag)
{
	سیٹ_local_var(xasprintf("PWD=%s", get_cwd(/*force:*/ 1)), flag);
}
#اگر_ختم /* !__U_BOOT__ */

#اگر ENABLE_HUSH_UNSET || ENABLE_HUSH_GETOPTS
ساکن صحیح unset_local_var(ثابت حرف *name)
{
	ساخت variable *cur;
	ساخت variable **cur_pp;

	cur_pp = &G.چوٹی_var;
	جب ((cur = *cur_pp) != تہی) {
		اگر (varcmp(cur->varstr, name) == 0) {
			اگر (cur->flg_read_only) {
				bb_error_پیغام("%s: readonly variable", name);
				واپس EXIT_FAILURE;
			}

			*cur_pp = cur->اگلا;
			debug_printf_env("%s: unsetenv '%s'\n", __func__, cur->varstr);
			bb_unsetenv(cur->varstr);
			اگر (!cur->max_len)
				آزاد(cur->varstr);
			آزاد(cur);

			توڑو;
		}
		cur_pp = &cur->اگلا;
	}

	/* Handle "unset LINENO" et al even if did not find the variable to unset */
	handle_بدلا_special_نام(name);

	واپس EXIT_SUCCESS;
}
#اگر_ختم

#اگر_غیر_تعریف_شدہ __U_BOOT__
/*
 * Helpers for "var1=val1 var2=val2 cmd" feature
 */
ساکن خالی جوڑو_vars(ساخت variable *var)
{
	ساخت variable *اگلا;

	جب (var) {
		اگلا = var->اگلا;
		var->اگلا = G.چوٹی_var;
		G.چوٹی_var = var;
		اگر (var->flg_export) {
			debug_printf_env("%s: restoring exported '%s'/%u\n", __func__, var->varstr, var->var_nest_level);
			putenv(var->varstr);
		} ورنہ {
			debug_printf_env("%s: restoring variable '%s'/%u\n", __func__, var->varstr, var->var_nest_level);
		}
		var = اگلا;
	}
}

/* We put strings[i] into variable table and possibly putenv them.
 * If variable is read only, we can free the strings[i]
 * which attempts to overwrite it.
 * The strings[] vector itself is freed.
 */
ساکن خالی سیٹ_vars_and_save_old(حرف **لڑیاں)
{
	حرف **s;

	اگر (!لڑیاں)
		واپس;

	s = لڑیاں;
	جب (*s) {
		ساخت variable *var_p;
		ساخت variable **var_pp;
		حرف *eq;

		eq = strchr(*s, '=');
		اگر (HUSH_DEBUG && !eq)
			bb_simple_error_پیغام_and_die("BUG in varexp4");
		var_pp = get_ptr_مقصد_local_var(*s);
		اگر (var_pp) {
			var_p = *var_pp;
			اگر (var_p->flg_read_only) {
				حرف **p;
				bb_error_پیغام("%s: readonly variable", *s);
				/*
				 * "VAR=V BLTIN" unsets VARs after BLTIN completes.
				 * If VAR is readonly, leaving it in the list
				 * after asssignment error (msg above)
				 * causes doubled error message later, on unset.
				 */
				debug_printf_env("removing/freeing '%s' element\n", *s);
				آزاد(*s);
				p = s;
				کرو { *p = p[1]; p++; } جب (*p);
				جاؤ اگلا;
			}
			/* below, set_local_var() with nest level will
			 * "shadow" (remove) this variable from
			 * global linked list.
			 */
		}
		debug_printf_env("%s: env override '%s'/%u\n", __func__, *s, G.var_nest_level);
		سیٹ_local_var(*s, (G.var_nest_level << SETFLAG_VARLVL_سرکاؤ) | SETFLAG_EXPORT);
		s++;
 اگلا: ;
	}
	آزاد(لڑیاں);
}

/*
 * Unicode helper
 */
ساکن خالی reinit_unicode_for_hush(خالی)
{
	/* Unicode support should be activated even if LANG is set
	 * _during_ shell execution, not only if it was set when
	 * shell was started. Therefore, re-check LANG every time:
	 */
	اگر (ENABLE_FEATURE_CHECK_UNICODE_اندر_ENV
	 || ENABLE_UNICODE_USING_LOCALE
	) {
		ثابت حرف *s = get_local_var_قدر("LC_ALL");
		اگر (!s) s = get_local_var_قدر("LC_CTYPE");
		اگر (!s) s = get_local_var_قدر("LANG");
		reinit_unicode(s);
	}
}

#اگر_ختم /* !__U_BOOT__ */
/*
 * in_str support (strings, and "strings" read from files).
 */

#اگر ENABLE_HUSH_INTERACTIVE
#اگر_غیر_تعریف_شدہ __U_BOOT__
/* To test correct lineedit/interactive behavior, type from command line:
 *	echo $P\
 *	\
 *	AT\
 *	H\
 *	\
 * It exercises a lot of corner cases.
 */
ساکن ثابت حرف *تیاری_prompt_string(خالی)
{
	ثابت حرف *prompt_str;

	debug_printf_prompt("%s promptmode:%d\n", __func__, G.promptmode);

# اگر ENABLE_FEATURE_EDITING_FANCY_PROMPT
	prompt_str = get_local_var_قدر(G.promptmode == 0 ? "PS1" : "PS2");
	اگر (!prompt_str)
		prompt_str = "";
# ورنہ
	prompt_str = "> "; /* if PS2, else... */
	اگر (G.promptmode == 0) { /* PS1 */
		/* No fancy prompts supported, (re)generate "CURDIR $ " by hand */
		آزاد(G.PS1);
		/* bash uses $PWD value, even if it is set by user.
		 * It uses current dir only if PWD is unset.
		 * We always use current dir. */
		prompt_str = G.PS1 = xasprintf("%s %c ", get_cwd(0), (geteuid() != 0) ? '$' : '#');
	}
# endif
	debug_printf("prompt_str '%s'\n", prompt_str);
	واپس prompt_str;
}
#اگر_ختم /* !__U_BOOT__ */

#اگر_غیر_تعریف_شدہ __U_BOOT__
ساکن صحیح get_user_داخلہ(ساخت اندر_str *i)
#ورنہ /* __U_BOOT__ */
ساکن خالی get_user_داخلہ(ساخت اندر_str *i)
#اگر_ختم /* __U_BOOT__ */
{
#اگر_غیر_تعریف_شدہ __U_BOOT__
# اگر ENABLE_FEATURE_EDITING
	/* In EDITING case, this function reads next input line,
	 * saves it in i->p, then returns 1st char of it.
	 */
	صحیح r;
	ثابت حرف *prompt_str;

	prompt_str = تیاری_prompt_string();
	برائے (;;) {
		reinit_unicode_for_hush();
		G.flag_SIGINT = 0;

		bb_got_signal = 0;
		اگر (!sigisemptyset(&G.pending_سیٹ)) {
			/* Whoops, already got a signal, do not call read_line_input */
			bb_got_signal = r = -1;
		} ورنہ {
			/* For shell, LI_INTERRUPTIBLE is set:
			 * read_line_input will abort on either
			 * getting EINTR in poll() and bb_got_signal became != 0,
			 * or if it sees bb_got_signal != 0
			 * (IOW: if signal arrives before poll() is reached).
			 * Interactive testcases:
			 * (while kill -INT $$; do sleep 1; done) &
			 * #^^^ prints ^C, prints prompt, repeats
			 * trap 'echo I' int; (while kill -INT $$; do sleep 1; done) &
			 * #^^^ prints ^C, prints "I", prints prompt, repeats
			 * trap 'echo T' term; (while kill $$; do sleep 1; done) &
			 * #^^^ prints "T", prints prompt, repeats
			 * #(bash 5.0.17 exits after first "T", looks like a bug)
			 */
			r = read_سطر_داخلہ(G.سطر_داخلہ_state, prompt_str,
				G.user_داخلہ_buf, CONFIG_FEATURE_EDITING_MAX_LEN-1
			);
			/* read_line_input intercepts ^C, "convert" it to SIGINT */
			اگر (r == 0)
				اٹھاؤ(SIGINT);
		}
		/* bash prints ^C (before running a trap, if any)
		 * both on keyboard ^C and on real SIGINT (non-kbd generated).
		 */
		اگر (sigismember(&G.pending_سیٹ, SIGINT)) {
			اكتب(STDOUT_FILENO, "^C\n", 3);
			G.آخری_exitcode = 128 | SIGINT;
		}
		check_and_تسلسل_traps();
		اگر (r == 0) /* keyboard ^C? */
			جاری; /* go back, read another input line */
		اگر (r > 0) /* normal input? (no ^C, no ^D, no signals) */
			توڑو;
		اگر (!bb_got_signal) {
			/* r < 0: ^D/EOF/error detected (but not signal) */
			/* ^D on interactive input goes to next line before exiting: */
			اكتب(STDOUT_FILENO, "\n", 1);
			i->p = تہی;
			i->peek_buf[0] = r = EOF;
			واپس r;
		}
		/* it was a signal: go back, read another input line */
	}
	i->p = G.user_داخلہ_buf;
	واپس (غیرعلامتی حرف)*i->p++;
# ورنہ
	/* In !EDITING case, this function gets called for every char.
	 * Buffering happens deeper in the call chain, in hfgetc(i->file).
	 */
	صحیح r;

	برائے (;;) {
		G.flag_SIGINT = 0;
		اگر (i->آخری_char == '\0' || i->آخری_char == '\n') {
			ثابت حرف *prompt_str = تیاری_prompt_string();
			/* Why check_and_run_traps here? Try this interactively:
			 * $ trap 'echo INT' INT; (sleep 2; kill -INT $$) &
			 * $ <[enter], repeatedly...>
			 * Without check_and_run_traps, handler never runs.
			 */
			check_and_تسلسل_traps();
			fputs_stdout(prompt_str);
			fflush_all();
		}
		r = hfgetc(i->file);
		/* In !ENABLE_FEATURE_EDITING we don't use read_line_input,
		 * no ^C masking happens during fgetc, no special code for ^C:
		 * it generates SIGINT as usual.
		 */
		check_and_تسلسل_traps();
		اگر (r != '\0' && !G.flag_SIGINT)
			توڑو;
		اگر (G.flag_SIGINT) {
			/* ^C or SIGINT: repeat */
			/* bash prints ^C even on real SIGINT (non-kbd generated) */
			/* kernel prints "^C" itself, just print newline: */
			اكتب(STDOUT_FILENO, "\n", 1);
			G.آخری_exitcode = 128 | SIGINT;
		}
	}
	واپس r;
# endif
#ورنہ /* __U_BOOT__ */
	صحیح n;
	صحیح promptme;
	ساکن حرف the_command[CONFIG_SYS_CBSIZE + 1];

	bootretry_reset_cmd_timeout();
	promptme = 1;
	n = u_boot_cli_readline(i);

# ifdef CONFIG_BOOT_RETRY_وقت
	اگر (n == -2) {
		puts("\nTimeout waiting for command\n");
#  ifdef CONFIG_RESET_مقصد_RETRY
		do_reset(تہی, 0, 0, تہی);
#  elif IS_ENABLED(CONFIG_RETRY_BOOTCMD)
	strcpy(console_بفر, "run bootcmd\n");
# ورنہ
#	error "This only works with CONFIG_RESET_TO_RETRY or CONFIG_BOOT_RETRY_COMMAND enabled"
#  endif
	}
# endif
	اگر (n == -1 ) {
		G.flag_repeat = 0;
		promptme = 0;
	}
	n = لڑی_لمبائی(console_بفر);
	console_بفر[n] = '\n';
	console_بفر[n+1]= '\0';
	اگر (had_ctrlc())
		G.flag_repeat = 0;
	clear_ctrlc();
	G.do_repeat = 0;
#اگر_غیر_تعریف_شدہ __U_BOOT__
	اگر (G.promptmode == 1) {
#ورنہ /* __U_BOOT__ */
	اگر (!G.promptmode) {
#اگر_ختم /* __U_BOOT__ */
		اگر (console_بفر[0] == '\n'&& G.flag_repeat == 0) {
			strcpy(the_command, console_بفر);
		}
		ورنہ {
			اگر (console_بفر[0] != '\n') {
				strcpy(the_command, console_بفر);
				G.flag_repeat = 1;
			}
			ورنہ {
				G.do_repeat = 1;
			}
		}
		i->p = the_command;
	}
	ورنہ {
		اگر (console_بفر[0] != '\n') {
			اگر (لڑی_لمبائی(the_command) + لڑی_لمبائی(console_بفر)
				< CONFIG_SYS_CBSIZE) {
				n = لڑی_لمبائی(the_command);
#اگر_تعریف_شدہ __U_BOOT__
				/*
				 * To avoid writing to bad places, we check if
				 * n is greater than 0.
				 * This bug was found by Harald Seiler.
				 */
				اگر (n > 0)
					the_command[n-1] = ' ';
				strcpy(&the_command[n], console_بفر);
#ورنہ /* !__U_BOOT__ */
			the_command[n-1] = ' ';
			strcpy(&the_command[n], console_بفر);
#اگر_ختم /* !__U_BOOT__ */
				}
				ورنہ {
					the_command[0] = '\n';
					the_command[1] = '\0';
					G.flag_repeat = 0;
				}
		}
		اگر (promptme == 0) {
			the_command[0] = '\n';
			the_command[1] = '\0';
		}
		i->p = console_بفر;
	}
#اگر_ختم /* __U_BOOT__ */
}
/* This is the magic location that prints prompts
 * and gets data back from the user */
ساکن صحیح fgetc_interactive(ساخت اندر_str *i)
{
	صحیح ch;
#اگر_غیر_تعریف_شدہ __U_BOOT__
	/* If it's interactive stdin, get new line. */
	اگر (G_interactive_fd && i->file == G.HFILE_stdin) {
#اگر_ختم /* !__U_BOOT__ */
#اگر_غیر_تعریف_شدہ __U_BOOT__
		/* Returns first char (or EOF), the rest is in i->p[] */
		ch = get_user_داخلہ(i);
#ورنہ /* __U_BOOT__ */
		/* Avoid garbage value and make clang happy. */
		ch = 0;
		/*
		 * get_user_input() does not return anything when used in
		 * U-Boot.
		 * So, we need to take the read character from i->p[].
		 */
		get_user_داخلہ(i);
		اگر (i->p && *i->p) {
			ch = *i->p++;
		}
#اگر_ختم /* __U_BOOT__ */
		G.promptmode = 1; /* PS2 */
		debug_printf_prompt("%s promptmode=%d\n", __func__, G.promptmode);
#اگر_غیر_تعریف_شدہ __U_BOOT__
	} ورنہ {
		/* Not stdin: script file, sourced file, etc */
		کرو ch = hfgetc(i->file); جب (ch == '\0');
	}
#اگر_ختم /* !__U_BOOT__ */
	واپس ch;
}
#ورنہ  /* !INTERACTIVE */
#اگر_غیر_تعریف_شدہ __U_BOOT__
ساکن ALWAYS_INLINE صحیح fgetc_interactive(ساخت اندر_str *i)
{
	صحیح ch;
	کرو ch = hfgetc(i->file); جب (ch == '\0');
	واپس ch;
}
#اگر_ختم /* !__U_BOOT__ */
#اگر_ختم  /* !INTERACTIVE */

ساکن صحیح i_getch(ساخت اندر_str *i)
{
	صحیح ch;

#اگر_غیر_تعریف_شدہ __U_BOOT__
	اگر (!i->file) {
		/* string-based in_str */
		ch = (غیرعلامتی حرف)*i->p;
		اگر (ch != '\0') {
			i->p++;
			i->آخری_char = ch;
#اگر ENABLE_HUSH_LINENO_VAR
			اگر (ch == '\n') {
				G.parse_lineno++;
				debug_printf_parse("G.parse_lineno++ = %u\n", G.parse_lineno);
			}
#اگر_ختم
			واپس ch;
		}
		واپس EOF;
	}

#اگر_ختم /* !__U_BOOT__ */
	/* FILE-based in_str */

#اگر ENABLE_FEATURE_EDITING
	/* This can be stdin, check line editing char[] buffer */
	اگر (i->p && *i->p != '\0') {
		ch = (غیرعلامتی حرف)*i->p++;
		جاؤ برآمد;
#اگر_غیر_تعریف_شدہ __U_BOOT__
	}
#ورنہ /* __U_BOOT__ */
	/*
	 * There are two ways for command to be called:
	 * 1. The first one is when they are typed by the user.
	 * 2. The second one is through run_command() (NOTE command run
	 * internally calls run_command()).
	 *
	 * In the second case, we do not get input from the user, so once we
	 * get a '\0', it means we need to stop.
	 * NOTE G.run_command_flags is only set on run_command call stack, so
	 * we use this to know if we come from user input or run_command().
	 */
	} ورنہ اگر (i->p && *i->p == '\0' && G.تسلسل_command_flags){
		واپس EOF;
	}
#اگر_ختم /* __U_BOOT__ */
#اگر_ختم
#اگر_غیر_تعریف_شدہ __U_BOOT__
	/* peek_buf[] is an int array, not char. Can contain EOF. */
	ch = i->peek_buf[0];
	اگر (ch != 0) {
		صحیح ch2 = i->peek_buf[1];
		i->peek_buf[0] = ch2;
		اگر (ch2 == 0) /* very likely, avoid redundant write */
			جاؤ برآمد;
		i->peek_buf[1] = 0;
		جاؤ برآمد;
	}

#اگر_ختم /* !__U_BOOT__ */
	ch = fgetc_interactive(i);
 برآمد:
	debug_printf("file_get: got '%c' %d\n", ch, ch);
	i->آخری_char = ch;
#اگر ENABLE_HUSH_LINENO_VAR
	اگر (ch == '\n') {
		G.parse_lineno++;
		debug_printf_parse("G.parse_lineno++ = %u\n", G.parse_lineno);
	}
#اگر_ختم
	واپس ch;
}

ساکن صحیح i_peek(ساخت اندر_str *i)
{
#اگر_غیر_تعریف_شدہ __U_BOOT__
	صحیح ch;

	اگر (!i->file) {
		/* string-based in_str */
		/* Doesn't report EOF on NUL. None of the callers care. */
		واپس (غیرعلامتی حرف)*i->p;
	}

	/* FILE-based in_str */

#اگر ENABLE_FEATURE_EDITING && ENABLE_HUSH_INTERACTIVE
	/* This can be stdin, check line editing char[] buffer */
	اگر (i->p && *i->p != '\0')
		واپس (غیرعلامتی حرف)*i->p;
#اگر_ختم
	/* peek_buf[] is an int array, not char. Can contain EOF. */
	ch = i->peek_buf[0];
	اگر (ch != 0)
		واپس ch;

	/* Need to get a new char */
	ch = fgetc_interactive(i);
	debug_printf("file_peek: got '%c' %d\n", ch, ch);

	/* Save it by either rolling back line editing buffer, or in i->peek_buf[0] */
#اگر ENABLE_FEATURE_EDITING && ENABLE_HUSH_INTERACTIVE
	اگر (i->p) {
		i->p -= 1;
		واپس ch;
	}
#اگر_ختم
	i->peek_buf[0] = ch;
	/*i->peek_buf[1] = 0; - already is */
	واپس ch;
#ورنہ /* __U_BOOT__ */
	/* string-based in_str */
	/* Doesn't report EOF on NUL. None of the callers care. */
	واپس (غیرعلامتی حرف)*i->p;
#اگر_ختم /* __U_BOOT__ */
}

/* Only ever called if i_peek() was called, and did not return EOF.
 * IOW: we know the previous peek saw an ordinary char, not EOF, not NUL,
 * not end-of-line. Therefore we never need to read a new editing line here.
 */
ساکن صحیح i_peek2(ساخت اندر_str *i)
{
#اگر_غیر_تعریف_شدہ __U_BOOT__
	صحیح ch;
#اگر_ختم /* !__U_BOOT__ */

	/* There are two cases when i->p[] buffer exists.
	 * (1) it's a string in_str.
	 * (2) It's a file, and we have a saved line editing buffer.
	 * In both cases, we know that i->p[0] exists and not NUL, and
	 * the peek2 result is in i->p[1].
	 */
	اگر (i->p)
		واپس (غیرعلامتی حرف)i->p[1];

#اگر_غیر_تعریف_شدہ __U_BOOT__
	/* Now we know it is a file-based in_str. */

	/* peek_buf[] is an int array, not char. Can contain EOF. */
	/* Is there 2nd char? */
	ch = i->peek_buf[1];
	اگر (ch == 0) {
		/* We did not read it yet, get it now */
		کرو ch = hfgetc(i->file); جب (ch == '\0');
		i->peek_buf[1] = ch;
	}

	debug_printf("file_peek2: got '%c' %d\n", ch, ch);
	واپس ch;
#ورنہ
	واپس 0;
#اگر_ختم /* __U_BOOT__ */
}

ساکن صحیح i_getch_and_eat_bkslash_nl(ساخت اندر_str *داخلہ)
{
	برائے (;;) {
		صحیح ch, ch2;

		ch = i_getch(داخلہ);
		اگر (ch != '\\')
			واپس ch;
		ch2 = i_peek(داخلہ);
		اگر (ch2 != '\n')
			واپس ch;
		/* backslash+newline, skip it */
		i_getch(داخلہ);
	}
}

/* Note: this function _eats_ \<newline> pairs, safe to use plain
 * i_getch() after it instead of i_getch_and_eat_bkslash_nl().
 */
ساکن صحیح i_peek_and_eat_bkslash_nl(ساخت اندر_str *داخلہ)
{
	برائے (;;) {
		صحیح ch, ch2;

		ch = i_peek(داخلہ);
		اگر (ch != '\\')
			واپس ch;
		ch2 = i_peek2(داخلہ);
		اگر (ch2 != '\n')
			واپس ch;
		/* backslash+newline, skip it */
		i_getch(داخلہ);
		i_getch(داخلہ);
	}
}

#اگر_غیر_تعریف_شدہ __U_BOOT__
ساکن خالی تیاری_file_اندر_str(ساخت اندر_str *i, HFILE *fp)
#ورنہ /* __U_BOOT__ */
ساکن خالی تیاری_file_اندر_str(ساخت اندر_str *i)
#اگر_ختم /* __U_BOOT__ */
{
	memset(i, 0, حجم(*i));
#اگر_غیر_تعریف_شدہ __U_BOOT__
	i->file = fp;
	/* i->p = NULL; */
#اگر_ختم /* !__U_BOOT__ */
}

ساکن خالی تیاری_string_اندر_str(ساخت اندر_str *i, ثابت حرف *s)
{
	memset(i, 0, حجم(*i));
	/*i->file = NULL */;
	i->p = s;
}

/*
 * o_string support
 */
#تعریف_کرو B_CHUNK  (32 * حجم(حرف*))

ساکن خالی o_reset_مقصد_خالی_خانے_unquoted(o_string *o)
{
	o->لمبائی = 0;
	o->has_quoted_part = 0;
	اگر (o->ڈیٹا)
		o->ڈیٹا[0] = '\0';
}

ساکن خالی o_free_and_سیٹ_NULL(o_string *o)
{
	آزاد(o->ڈیٹا);
	memset(o, 0, حجم(*o));
}

ساکن ALWAYS_INLINE خالی o_free(o_string *o)
{
	آزاد(o->ڈیٹا);
}

ساکن خالی o_grow_by(o_string *o, صحیح len)
{
	اگر (o->لمبائی + len > o->maxlen) {
		o->maxlen += (2 * len) | (B_CHUNK-1);
		o->ڈیٹا = xrealloc(o->ڈیٹا, 1 + o->maxlen);
	}
}

ساکن خالی o_addchr(o_string *o, صحیح ch)
{
	debug_printf("o_addchr: '%c' o->length=%d o=%p\n", ch, o->لمبائی, o);
	اگر (o->لمبائی < o->maxlen) {
		/* likely. avoid o_grow_by() call */
 جوڑو:
		o->ڈیٹا[o->لمبائی] = ch;
		o->لمبائی++;
		o->ڈیٹا[o->لمبائی] = '\0';
		واپس;
	}
	o_grow_by(o, 1);
	جاؤ جوڑو;
}

#اگر 0
/* Valid only if we know o_string is not empty */
ساکن خالی o_delchr(o_string *o)
{
	o->لمبائی--;
	o->ڈیٹا[o->لمبائی] = '\0';
}
#اگر_ختم

ساکن خالی o_addblock(o_string *o, ثابت حرف *str, صحیح len)
{
	o_grow_by(o, len);
	((حرف*)mempcpy(&o->ڈیٹا[o->لمبائی], str, len))[0] = '\0';
	o->لمبائی += len;
}

ساکن خالی o_addstr(o_string *o, ثابت حرف *str)
{
	o_addblock(o, str, لڑی_لمبائی(str));
}

#اگر_غیر_تعریف_شدہ __U_BOOT__
ساکن خالی o_addstr_with_NUL(o_string *o, ثابت حرف *str)
{
	o_addblock(o, str, لڑی_لمبائی(str) + 1);
}
#اگر_ختم /* !__U_BOOT__ */

#اگر !BB_MMU
ساکن خالی nommu_addchr(o_string *o, صحیح ch)
{
	اگر (o)
		o_addchr(o, ch);
}
#ورنہ
# define nommu_addchr(o, str) ((خالی)0)
#اگر_ختم

#اگر_غیر_تعریف_شدہ __U_BOOT__
#اگر ENABLE_HUSH_منوال_X
ساکن خالی x_منوال_addchr(صحیح ch)
{
	o_addchr(&G.x_منوال_buf, ch);
}
ساکن خالی x_منوال_addstr(ثابت حرف *str)
{
	o_addstr(&G.x_منوال_buf, str);
}
ساکن خالی x_منوال_addblock(ثابت حرف *str, صحیح len)
{
	o_addblock(&G.x_منوال_buf, str, len);
}
ساکن خالی x_منوال_سابقہ(خالی)
{
	صحیح n = G.x_منوال_گہرائی;
	کرو x_منوال_addchr('+'); جب (--n >= 0);
}
ساکن خالی x_منوال_flush(خالی)
{
	صحیح len = G.x_منوال_buf.لمبائی;
	اگر (len <= 0)
		واپس;
	اگر (G.x_منوال_fd > 0) {
		G.x_منوال_buf.ڈیٹا[len] = '\n';
		بھرا_write(G.x_منوال_fd, G.x_منوال_buf.ڈیٹا, len + 1);
	}
	G.x_منوال_buf.لمبائی = 0;
}
#اگر_ختم
#اگر_ختم /* !__U_BOOT__ */

/*
 * HUSH_BRACE_EXPANSION code needs corresponding quoting on variable expansion side.
 * Currently, "v='{q,w}'; echo $v" erroneously expands braces in $v.
 * Apparently, on unquoted $v bash still does globbing
 * ("v='*.txt'; echo $v" prints all .txt files),
 * but NOT brace expansion! Thus, there should be TWO independent
 * quoting mechanisms on $v expansion side: one protects
 * $v from brace expansion, and other additionally protects "$v" against globbing.
 * We have only second one.
 */

#اگر ENABLE_HUSH_BRACE_EXPANSION
# define MAYBE_BRACES "{}"
#ورنہ
# define MAYBE_BRACES ""
#اگر_ختم

/* My analysis of quoting semantics tells me that state information
 * is associated with a destination, not a source.
 */
ساکن خالی o_addqchr(o_string *o, صحیح ch)
{
	صحیح sz = 1;
	/* '-' is included because of this case:
	 * >filename0 >filename1 >filename9; v='-'; echo filename[0"$v"9]
	 */
	حرف *ملا = strchr("*?[-\\" MAYBE_BRACES, ch);
	اگر (ملا)
		sz++;
	o_grow_by(o, sz);
	اگر (ملا) {
		o->ڈیٹا[o->لمبائی] = '\\';
		o->لمبائی++;
	}
	o->ڈیٹا[o->لمبائی] = ch;
	o->لمبائی++;
	o->ڈیٹا[o->لمبائی] = '\0';
}

ساکن خالی o_جوڑو_Qchr(o_string *o, صحیح ch)
{
	صحیح sz = 1;
	اگر ((o->o_expflags & EXP_FLAG_ESC_GLOB_CHARS)
	 && strchr("*?[-\\" MAYBE_BRACES, ch)
	) {
		sz++;
		o->ڈیٹا[o->لمبائی] = '\\';
		o->لمبائی++;
	}
	o_grow_by(o, sz);
	o->ڈیٹا[o->لمبائی] = ch;
	o->لمبائی++;
	o->ڈیٹا[o->لمبائی] = '\0';
}

ساکن خالی o_addqblock(o_string *o, ثابت حرف *str, صحیح len)
{
	جب (len) {
		حرف ch;
		صحیح sz;
		صحیح ordinary_cnt = strcspn(str, "*?[-\\" MAYBE_BRACES);
		اگر (ordinary_cnt > len) /* paranoia */
			ordinary_cnt = len;
		o_addblock(o, str, ordinary_cnt);
		اگر (ordinary_cnt == len)
			واپس; /* NUL is already added by o_addblock */
		str += ordinary_cnt;
		len -= ordinary_cnt + 1; /* we are processing + 1 char below */

		ch = *str++;
		sz = 1;
		اگر (ch) { /* it is necessarily one of "*?[-\\" MAYBE_BRACES */
			sz++;
			o->ڈیٹا[o->لمبائی] = '\\';
			o->لمبائی++;
		}
		o_grow_by(o, sz);
		o->ڈیٹا[o->لمبائی] = ch;
		o->لمبائی++;
	}
	o->ڈیٹا[o->لمبائی] = '\0';
}

ساکن خالی o_جوڑو_Qblock(o_string *o, ثابت حرف *str, صحیح len)
{
	اگر (!(o->o_expflags & EXP_FLAG_ESC_GLOB_CHARS)) {
		o_addblock(o, str, len);
		واپس;
	}
	o_addqblock(o, str, len);
}

ساکن خالی o_جوڑو_Qstr(o_string *o, ثابت حرف *str)
{
	o_جوڑو_Qblock(o, str, لڑی_لمبائی(str));
}

/* A special kind of o_string for $VAR and `cmd` expansion.
 * It contains char* list[] at the beginning, which is grown in 16 element
 * increments. Actual string data starts at the next multiple of 16 * (char*).
 * list[i] contains an INDEX (int!) into this string data.
 * It means that if list[] needs to grow, data needs to be moved higher up
 * but list[i]'s need not be modified.
 * NB: remembering how many list[i]'s you have there is crucial.
 * o_finalize_list() operation post-processes this structure - calculates
 * and stores actual char* ptrs in list[]. Oh, it NULL terminates it as well.
 */
#اگر DEBUG_EXPAND || DEBUG_GLOB
ساکن خالی debug_print_list(ثابت حرف *سابقہ, o_string *o, صحیح n)
{
	حرف **list = (حرف**)o->ڈیٹا;
	صحیح string_آغاز = ((n + 0xf) & ~0xf) * حجم(list[0]);
	صحیح i = 0;

	indent();
	fdprintf(2, "%s: list:%p n:%d string_start:%d length:%d maxlen:%d glob:%d quoted:%d escape:%d\n",
			سابقہ, list, n, string_آغاز, o->لمبائی, o->maxlen,
			!!(o->o_expflags & EXP_FLAG_GLOB),
			o->has_quoted_part,
			!!(o->o_expflags & EXP_FLAG_ESC_GLOB_CHARS));
	جب (i < n) {
		indent();
		fdprintf(2, " list[%d]=%d '%s' %p\n", i, (صحیح)(uintptr_t)list[i],
				o->ڈیٹا + (صحیح)(uintptr_t)list[i] + string_آغاز,
				o->ڈیٹا + (صحیح)(uintptr_t)list[i] + string_آغاز);
		i++;
	}
	اگر (n) {
		ثابت حرف *p = o->ڈیٹا + (صحیح)(uintptr_t)list[n - 1] + string_آغاز;
		indent();
#اگر_غیر_تعریف_شدہ __U_BOOT__
		fdprintf(2, " total_sz:%ld\n", (طویل)((p + لڑی_لمبائی(p) + 1) - o->ڈیٹا));
#ورنہ /* __U_BOOT__ */
		لکھو(" total_sz:%ld\n", (طویل)((p + لڑی_لمبائی(p) + 1) - o->ڈیٹا));
#اگر_ختم /* __U_BOOT__ */
	}
}
#ورنہ
# define debug_print_list(سابقہ, o, n) ((خالی)0)
#اگر_ختم

/* n = o_save_ptr_helper(str, n) "starts new string" by storing an index value
 * in list[n] so that it points past last stored byte so far.
 * It returns n+1. */
ساکن صحیح o_save_ptr_helper(o_string *o, صحیح n)
{
	حرف **list = (حرف**)o->ڈیٹا;
	صحیح string_آغاز;
	صحیح string_len;

	اگر (!o->has_خالی_خانے_خانہ) {
		string_آغاز = ((n + 0xf) & ~0xf) * حجم(list[0]);
		string_len = o->لمبائی - string_آغاز;
		اگر (!(n & 0xf)) { /* 0, 0x10, 0x20...? */
			debug_printf_list("list[%d]=%d string_start=%d (growing)\n", n, string_len, string_آغاز);
			/* list[n] points to string_start, make space for 16 more pointers */
			o->maxlen += 0x10 * حجم(list[0]);
			o->ڈیٹا = xrealloc(o->ڈیٹا, o->maxlen + 1);
			list = (حرف**)o->ڈیٹا;
			memmove(list + n + 0x10, list + n, string_len);
			/*
			 * expand_on_ifs() has a "previous argv[] ends in IFS?"
			 * check. (grep for -prev-ifs-check-).
			 * Ensure that argv[-1][last] is not garbage
			 * but zero bytes, to save index check there.
			 */
			list[n + 0x10 - 1] = 0;
			o->لمبائی += 0x10 * حجم(list[0]);
		} ورنہ {
			debug_printf_list("list[%d]=%d string_start=%d\n",
					n, string_len, string_آغاز);
		}
	} ورنہ {
		/* We have empty slot at list[n], reuse without growth */
		string_آغاز = ((n+1 + 0xf) & ~0xf) * حجم(list[0]); /* NB: n+1! */
		string_len = o->لمبائی - string_آغاز;
		debug_printf_list("list[%d]=%d string_start=%d (empty slot)\n",
				n, string_len, string_آغاز);
		o->has_خالی_خانے_خانہ = 0;
	}
	o->has_quoted_part = 0;
	list[n] = (حرف*)(uintptr_t)string_len;
	واپس n + 1;
}

/* "What was our last o_save_ptr'ed position (byte offset relative o->data)?" */
ساکن صحیح o_get_آخری_ptr(o_string *o, صحیح n)
{
	حرف **list = (حرف**)o->ڈیٹا;
	صحیح string_آغاز = ((n + 0xf) & ~0xf) * حجم(list[0]);

	واپس ((صحیح)(uintptr_t)list[n-1]) + string_آغاز;
}

/*
 * Globbing routines.
 *
 * Most words in commands need to be globbed, even ones which are
 * (single or double) quoted. This stems from the possiblity of
 * constructs like "abc"* and 'abc'* - these should be globbed.
 * Having a different code path for fully-quoted strings ("abc",
 * 'abc') would only help performance-wise, but we still need
 * code for partially-quoted strings.
 *
 * Unfortunately, if we want to match bash and ash behavior in all cases,
 * the logic can't be "shell-syntax argument is first transformed
 * to a string, then globbed, and if globbing does not match anything,
 * it is used verbatim". Here are two examples where it fails:
 *
 * 	echo 'b\*'?
 *
 * The globbing can't be avoided (because of '?' at the end).
 * The glob pattern is: b\\\*? - IOW, both \ and * are literals
 * and are glob-escaped. If this does not match, bash/ash print b\*?
 * - IOW: they "unbackslash" the glob pattern.
 * Now, look at this:
 *
 * 	v='\\\*'; echo b$v?
 *
 * The glob pattern is the same here: b\\\*? - the unquoted $v expansion
 * should be used as glob pattern with no changes. However, if glob
 * does not match, bash/ash print b\\\*? - NOT THE SAME as first example!
 *
 * ash implements this by having an encoded representation of the word
 * to glob, which IS NOT THE SAME as the glob pattern - it has more data.
 * Glob pattern is derived from it. If glob fails, the decision what result
 * should be is made using that encoded representation. Not glob pattern.
 */

#اگر ENABLE_HUSH_BRACE_EXPANSION
/* There in a GNU extension, GLOB_BRACE, but it is not usable:
 * first, it processes even {a} (no commas), second,
 * I didn't manage to make it return strings when they don't match
 * existing files. Need to re-implement it.
 */

/* Helper */
ساکن صحیح glob_needed(ثابت حرف *s)
{
	جب (*s) {
		اگر (*s == '\\') {
			اگر (!s[1])
				واپس 0;
			s += 2;
			جاری;
		}
		اگر (*s == '*' || *s == '[' || *s == '?' || *s == '{')
			واپس 1;
		s++;
	}
	واپس 0;
}
/* Return pointer to next closing brace or to comma */
ساکن ثابت حرف *اگلا_brace_sub(ثابت حرف *cp)
{
	غیرعلامتی گہرائی = 0;
	cp++;
	جب (*cp != '\0') {
		اگر (*cp == '\\') {
			اگر (*++cp == '\0')
				توڑو;
			cp++;
			جاری;
		}
		اگر ((*cp == '}' && گہرائی-- == 0) || (*cp == ',' && گہرائی == 0))
			توڑو;
		اگر (*cp++ == '{')
			گہرائی++;
	}

	واپس *cp != '\0' ? cp : تہی;
}
/* Recursive brace globber. Note: may garble pattern[]. */
ساکن صحیح glob_brace(حرف *نمونہ, o_string *o, صحیح n)
{
	حرف *new_نمونہ_buf;
	ثابت حرف *begin;
	ثابت حرف *اگلا;
	ثابت حرف *rest;
	ثابت حرف *p;
	size_t rest_len;

	debug_printf_glob("glob_brace('%s')\n", نمونہ);

	begin = نمونہ;
	جب (1) {
		اگر (*begin == '\0')
			جاؤ simple_glob;
		اگر (*begin == '{') {
			/* Find the first sub-pattern and at the same time
			 * find the rest after the closing brace */
			اگلا = اگلا_brace_sub(begin);
			اگر (اگلا == تہی) {
				/* An illegal expression */
				جاؤ simple_glob;
			}
			اگر (*اگلا == '}') {
				/* "{abc}" with no commas - illegal
				 * brace expr, disregard and skip it */
				begin = اگلا + 1;
				جاری;
			}
			توڑو;
		}
		اگر (*begin == '\\' && begin[1] != '\0')
			begin++;
		begin++;
	}
	debug_printf_glob("begin:%s\n", begin);
	debug_printf_glob("next:%s\n", اگلا);

	/* Now find the end of the whole brace expression */
	rest = اگلا;
	جب (*rest != '}') {
		rest = اگلا_brace_sub(rest);
		اگر (rest == تہی) {
			/* An illegal expression */
			جاؤ simple_glob;
		}
		debug_printf_glob("rest:%s\n", rest);
	}
	rest_len = لڑی_لمبائی(++rest) + 1;

	/* We are sure the brace expression is well-formed */

	/* Allocate working buffer large enough for our work */
	new_نمونہ_buf = xmalloc(لڑی_لمبائی(نمونہ));

	/* We have a brace expression.  BEGIN points to the opening {,
	 * NEXT points past the terminator of the first element, and REST
	 * points past the final }.  We will accumulate result names from
	 * recursive runs for each brace alternative in the buffer using
	 * GLOB_APPEND. */

	p = begin + 1;
	جب (1) {
		/* Construct the new glob expression */
		حافظہ_نقل(
			mempcpy(
				mempcpy(new_نمونہ_buf,
					/* We know the prefix for all sub-patterns */
					نمونہ, begin - نمونہ),
				p, اگلا - p),
			rest, rest_len);

		/* Note: glob_brace() may garble new_pattern_buf[].
		 * That's why we re-copy prefix every time (1st memcpy above).
		 */
		n = glob_brace(new_نمونہ_buf, o, n);
		اگر (*اگلا == '}') {
			/* We saw the last entry */
			توڑو;
		}
		p = اگلا + 1;
		اگلا = اگلا_brace_sub(اگلا);
	}
	آزاد(new_نمونہ_buf);
	واپس n;

 simple_glob:
	{
		صحیح gr;
		glob_t globdata;

		memset(&globdata, 0, حجم(globdata));
		gr = glob(نمونہ, 0, تہی, &globdata);
		debug_printf_glob("glob('%s'):%d\n", نمونہ, gr);
		اگر (gr != 0) {
			اگر (gr == GLOB_NOMATCH) {
				globfree(&globdata);
				/* NB: garbles parameter */
				unbackslash(نمونہ);
				o_addstr_with_NUL(o, نمونہ);
				debug_printf_glob("glob pattern '%s' is literal\n", نمونہ);
				واپس o_save_ptr_helper(o, n);
			}
			اگر (gr == GLOB_NOSPACE)
				bb_die_حافظہ_exhausted();
			/* GLOB_ABORTED? Only happens with GLOB_ERR flag,
			 * but we didn't specify it. Paranoia again. */
			bb_error_پیغام_and_die("glob error %d on '%s'", gr, نمونہ);
		}
		اگر (globdata.gl_pathv && globdata.gl_pathv[0]) {
			حرف **argv = globdata.gl_pathv;
			جب (1) {
				o_addstr_with_NUL(o, *argv);
				n = o_save_ptr_helper(o, n);
				argv++;
				اگر (!*argv)
					توڑو;
			}
		}
		globfree(&globdata);
	}
	واپس n;
}
/* Performs globbing on last list[],
 * saving each result as a new list[].
 */
ساکن صحیح perform_glob(o_string *o, صحیح n)
{
	حرف *نمونہ, *copy;

	debug_printf_glob("start perform_glob: n:%d o->data:%p\n", n, o->ڈیٹا);
	اگر (!o->ڈیٹا)
		واپس o_save_ptr_helper(o, n);
	نمونہ = o->ڈیٹا + o_get_آخری_ptr(o, n);
	debug_printf_glob("glob pattern '%s'\n", نمونہ);
	اگر (!glob_needed(نمونہ)) {
		/* unbackslash last string in o in place, fix length */
		o->لمبائی = unbackslash(نمونہ) - o->ڈیٹا;
		debug_printf_glob("glob pattern '%s' is literal\n", نمونہ);
		واپس o_save_ptr_helper(o, n);
	}

	copy = xstrdup(نمونہ);
	/* "forget" pattern in o */
	o->لمبائی = نمونہ - o->ڈیٹا;
	n = glob_brace(copy, o, n);
	آزاد(copy);
	اگر (DEBUG_GLOB)
		debug_print_list("perform_glob returning", o, n);
	واپس n;
}

#ورنہ /* !HUSH_BRACE_EXPANSION */

/* Helper */
ساکن صحیح glob_needed(ثابت حرف *s)
{
	جب (*s) {
		اگر (*s == '\\') {
			اگر (!s[1])
				واپس 0;
			s += 2;
			جاری;
		}
		اگر (*s == '*' || *s == '[' || *s == '?')
			واپس 1;
		s++;
	}
	واپس 0;
}
/* Performs globbing on last list[],
 * saving each result as a new list[].
 */
ساکن صحیح perform_glob(o_string *o, صحیح n)
{
#اگر_غیر_تعریف_شدہ __U_BOOT__
	glob_t globdata;
	صحیح gr;
#اگر_ختم /* __U_BOOT__ */
	حرف *نمونہ;

	debug_printf_glob("start perform_glob: n:%d o->data:%p\n", n, o->ڈیٹا);
	اگر (!o->ڈیٹا)
		واپس o_save_ptr_helper(o, n);
	نمونہ = o->ڈیٹا + o_get_آخری_ptr(o, n);
	debug_printf_glob("glob pattern '%s'\n", نمونہ);
	اگر (!glob_needed(نمونہ)) {
#اگر_غیر_تعریف_شدہ __U_BOOT__
 literal:
#اگر_ختم /* __U_BOOT__ */
		/* unbackslash last string in o in place, fix length */
		o->لمبائی = unbackslash(نمونہ) - o->ڈیٹا;
		debug_printf_glob("glob pattern '%s' is literal\n", نمونہ);
		واپس o_save_ptr_helper(o, n);
	}

#اگر_غیر_تعریف_شدہ __U_BOOT__
	memset(&globdata, 0, حجم(globdata));
	/* Can't use GLOB_NOCHECK: it does not unescape the string.
	 * If we glob "*.\*" and don't find anything, we need
	 * to fall back to using literal "*.*", but GLOB_NOCHECK
	 * will return "*.\*"!
	 */
	gr = glob(نمونہ, 0, تہی, &globdata);
	debug_printf_glob("glob('%s'):%d\n", نمونہ, gr);
	اگر (gr != 0) {
		اگر (gr == GLOB_NOMATCH) {
			globfree(&globdata);
			جاؤ literal;
		}
		اگر (gr == GLOB_NOSPACE)
			bb_die_حافظہ_exhausted();
		/* GLOB_ABORTED? Only happens with GLOB_ERR flag,
		 * but we didn't specify it. Paranoia again. */
		bb_error_پیغام_and_die("glob error %d on '%s'", gr, نمونہ);
	}
	اگر (globdata.gl_pathv && globdata.gl_pathv[0]) {
		حرف **argv = globdata.gl_pathv;
		/* "forget" pattern in o */
		o->لمبائی = نمونہ - o->ڈیٹا;
		جب (1) {
			o_addstr_with_NUL(o, *argv);
			n = o_save_ptr_helper(o, n);
			argv++;
			اگر (!*argv)
				توڑو;
		}
	}
	globfree(&globdata);
	اگر (DEBUG_GLOB)
		debug_print_list("perform_glob returning", o, n);
	واپس n;
#ورنہ /* __U_BOOT__ */
	/*
	 * NOTE We only use perform glob to call unbackslash to remove backslash
	 * from string once expanded.
	 * So, it seems OK to return this if no previous return was done.
	 */
	واپس o_save_ptr_helper(o, n);
#اگر_ختم /* __U_BOOT__ */
}

#اگر_ختم /* !HUSH_BRACE_EXPANSION */

/* If o->o_expflags & EXP_FLAG_GLOB, glob the string so far remembered.
 * Otherwise, just finish current list[] and start new */
ساکن صحیح o_save_ptr(o_string *o, صحیح n)
{
	اگر (o->o_expflags & EXP_FLAG_GLOB) {
		/* If o->has_empty_slot, list[n] was already globbed
		 * (if it was requested back then when it was filled)
		 * so don't do that again! */
		اگر (!o->has_خالی_خانے_خانہ)
			واپس perform_glob(o, n); /* o_save_ptr_helper is inside */
	}
	واپس o_save_ptr_helper(o, n);
}

/* "Please convert list[n] to real char* ptrs, and NULL terminate it." */
ساکن حرف **o_finalize_list(o_string *o, صحیح n)
{
	حرف **list;
	صحیح string_آغاز;

	اگر (DEBUG_EXPAND)
		debug_print_list("finalized", o, n);
	debug_printf_expand("finalized n:%d\n", n);
	list = (حرف**)o->ڈیٹا;
	string_آغاز = ((n + 0xf) & ~0xf) * حجم(list[0]);
	list[--n] = تہی;
	جب (n) {
		n--;
		list[n] = o->ڈیٹا + (صحیح)(uintptr_t)list[n] + string_آغاز;
	}
	واپس list;
}

ساکن خالی free_pipe_list(ساخت pipe *pi);

/* Returns pi->next - next pipe in the list */
ساکن ساخت pipe *free_pipe(ساخت pipe *pi)
{
	ساخت pipe *اگلا;
	صحیح i;

#اگر_غیر_تعریف_شدہ __U_BOOT__
	debug_printf_clean("free_pipe (pid %d)\n", getpid());
#اگر_ختم /* !__U_BOOT__ */
	برائے (i = 0; i < pi->عدد_cmds; i++) {
		ساخت command *command;
#اگر_غیر_تعریف_شدہ __U_BOOT__
		ساخت redir_struct *r, *rnext;
#اگر_ختم /* !__U_BOOT__ */

		command = &pi->cmds[i];
		debug_printf_clean("  command %d:\n", i);
		اگر (command->argv) {
			اگر (DEBUG_CLEAN) {
				صحیح a;
				حرف **p;
				برائے (a = 0, p = command->argv; *p; a++, p++) {
					debug_printf_clean("   argv[%d] = %s\n", a, *p);
				}
			}
			free_لڑیاں(command->argv);
			//command->argv = NULL;
		}
		/* not "else if": on syntax error, we may have both! */
		اگر (command->گروہ) {
			debug_printf_clean("   begin group (cmd_type:%d)\n",
					command->cmd_type);
			free_pipe_list(command->گروہ);
			debug_printf_clean("   end group\n");
			//command->group = NULL;
		}
		/* else is crucial here.
		 * If group != NULL, child_func is meaningless */
#اگر ENABLE_HUSH_FUNCTIONS
		ورنہ اگر (command->بچہ_func) {
			debug_printf_exec("cmd %p releases child func at %p\n", command, command->بچہ_func);
			command->بچہ_func->والد_cmd = تہی;
		}
#اگر_ختم
#اگر !BB_MMU
		آزاد(command->گروہ_as_string);
		//command->group_as_string = NULL;
#اگر_ختم
#اگر_غیر_تعریف_شدہ __U_BOOT__
		برائے (r = command->redirects; r; r = rnext) {
			debug_printf_clean("   redirect %d%s",
					r->rd_fd, redir_جدول[r->rd_type].descrip);
			/* guard against the case >$FOO, where foo is unset or blank */
			اگر (r->rd_filename) {
				debug_printf_clean(" fname:'%s'\n", r->rd_filename);
				آزاد(r->rd_filename);
				//r->rd_filename = NULL;
			}
			debug_printf_clean(" rd_dup:%d\n", r->rd_dup);
			rnext = r->اگلا;
			آزاد(r);
		}
		//command->redirects = NULL;
#اگر_ختم /* !__U_BOOT__ */
	}
	آزاد(pi->cmds);   /* children are an array, they get freed all at once */
	//pi->cmds = NULL;
#اگر_غیر_تعریف_شدہ __U_BOOT__
#اگر ENABLE_HUSH_کام
	آزاد(pi->cmdtext);
	//pi->cmdtext = NULL;
#اگر_ختم
#اگر_ختم /* !__U_BOOT__ */

	اگلا = pi->اگلا;
	آزاد(pi);
	واپس اگلا;
}

ساکن خالی free_pipe_list(ساخت pipe *pi)
{
	جب (pi) {
#اگر HAS_KEYWORDS
		debug_printf_clean("pipe reserved word %d\n", pi->res_لفظ);
#اگر_ختم
		debug_printf_clean("pipe followup code %d\n", pi->followup);
		pi = free_pipe(pi);
	}
}

/*** Parsing routines ***/

#اگر_غیر_تعریف_شدہ debug_print_درخت
ساکن خالی debug_print_درخت(ساخت pipe *pi, صحیح lvl)
{
	ساکن ثابت حرف *ثابت PIPE[] ALIGN_PTR = {
		[PIPE_SEQ] = "SEQ",
		[PIPE_AND] = "AND",
		[PIPE_OR ] = "OR" ,
		[PIPE_BG ] = "BG" ,
	};
	ساکن ثابت حرف *RES[] = {
		[RES_NONE ] = "NONE" ,
# اگر ENABLE_HUSH_IF
		[RES_IF   ] = "IF"   ,
		[RES_THEN ] = "THEN" ,
		[RES_ELIF ] = "ELIF" ,
		[RES_ELSE ] = "ELSE" ,
		[RES_FI   ] = "FI"   ,
# endif
# اگر ENABLE_HUSH_LOOPS
		[RES_FOR  ] = "FOR"  ,
		[RES_WHILE] = "WHILE",
		[RES_UNTIL] = "UNTIL",
		[RES_DO   ] = "DO"   ,
		[RES_مکمل ] = "DONE" ,
# endif
# اگر ENABLE_HUSH_LOOPS || ENABLE_HUSH_CASE
		[RES_اندر   ] = "IN"   ,
# endif
# اگر ENABLE_HUSH_CASE
		[RES_CASE ] = "CASE" ,
		[RES_CASE_اندر ] = "CASE_IN" ,
		[RES_مطابقت] = "MATCH",
		[RES_CASE_BODY] = "CASE_BODY",
		[RES_ESAC ] = "ESAC" ,
# endif
		[RES_XXXX ] = "XXXX" ,
		[RES_SNTX ] = "SNTX" ,
	};
	ساکن ثابت حرف *ثابت CMDTYPE[] ALIGN_PTR = {
		"{}",
		"()",
		"[noglob]",
# اگر ENABLE_HUSH_FUNCTIONS
		"func()",
# endif
	};

	صحیح pin, prn;

	pin = 0;
	جب (pi) {
		fdprintf(2, "%*spipe %d #cmds:%d %sres_word=%s followup=%d %s\n",
				lvl*2, "",
				pin,
				pi->عدد_cmds,
				(IF_HAS_KEYWORDS(pi->pi_inverted ? "! " :) ""),
				RES[pi->res_لفظ],
				pi->followup, PIPE[pi->followup]
		);
		prn = 0;
		جب (prn < pi->عدد_cmds) {
			ساخت command *command = &pi->cmds[prn];
			حرف **argv = command->argv;

			fdprintf(2, "%*s cmd %d assignment_cnt:%d",
					lvl*2, "", prn,
					command->assignment_cnt);
# اگر ENABLE_HUSH_LINENO_VAR
			fdprintf(2, " LINENO:%u", command->lineno);
# endif
			اگر (command->گروہ) {
				fdprintf(2, " group %s: (argv=%p)%s%s\n",
						CMDTYPE[command->cmd_type],
						argv
# اگر !BB_MMU
						, " group_as_string:", command->گروہ_as_string
# ورنہ
						, "", ""
# endif
				);
				debug_print_درخت(command->گروہ, lvl+1);
				prn++;
				جاری;
			}
			اگر (argv) جب (*argv) {
				fdprintf(2, " '%s'", *argv);
				argv++;
			}
#اگر_غیر_تعریف_شدہ __U_BOOT__
			اگر (command->redirects)
				fdprintf(2, " {redir}");
#اگر_ختم /* __U_BOOT__ */
			fdprintf(2, "\n");
			prn++;
		}
		pi = pi->اگلا;
		pin++;
	}
}
#اگر_ختم /* debug_print_tree */

ساکن ساخت pipe *new_pipe(خالی)
{
	ساخت pipe *pi;
	pi = xzalloc(حجم(ساخت pipe));
	/*pi->res_word = RES_NONE; - RES_NONE is 0 anyway */
	واپس pi;
}

/* Command (member of a pipe) is complete, or we start a new pipe
 * if ctx->command is NULL.
 * No errors possible here.
 */
ساکن صحیح مکمل_command(ساخت parse_context *ctx)
{
	/* The command is really already in the pipe structure, so
	 * advance the pipe counter and make a new, null command. */
	ساخت pipe *pi = ctx->pipe;
	ساخت command *command = ctx->command;

#اگر 0	/* Instead we emit error message at run time */
	اگر (ctx->pending_redirect) {
		/* For example, "cmd >" (no filename to redirect to) */
		syntax_error("invalid redirect");
		ctx->pending_redirect = تہی;
	}
#اگر_ختم

	اگر (command) {
		اگر (IS_NULL_CMD(command)) {
			debug_printf_parse("done_command: skipping null cmd, num_cmds=%d\n", pi->عدد_cmds);
			جاؤ clear_and_ret;
		}
		pi->عدد_cmds++;
		debug_printf_parse("done_command: ++num_cmds=%d\n", pi->عدد_cmds);
		//debug_print_tree(ctx->list_head, 20);
	} ورنہ {
		debug_printf_parse("done_command: initializing, num_cmds=%d\n", pi->عدد_cmds);
	}

	/* Only real trickiness here is that the uncommitted
	 * command structure is not counted in pi->num_cmds. */
	pi->cmds = xrealloc(pi->cmds, حجم(*pi->cmds) * (pi->عدد_cmds+1));
	ctx->command = command = &pi->cmds[pi->عدد_cmds];
 clear_and_ret:
	memset(command, 0, حجم(*command));
#اگر ENABLE_HUSH_LINENO_VAR
	command->lineno = G.parse_lineno;
	debug_printf_parse("command->lineno = G.parse_lineno (%u)\n", G.parse_lineno);
#اگر_ختم
	واپس pi->عدد_cmds; /* used only for 0/nonzero check */
}

ساکن خالی مکمل_pipe(ساخت parse_context *ctx, pipe_style type)
{
	صحیح not_null;

	debug_printf_parse("done_pipe entered, followup %d\n", type);
	/* Close previous command */
	not_null = مکمل_command(ctx);
#اگر HAS_KEYWORDS
	ctx->pipe->pi_inverted = ctx->ctx_inverted;
	ctx->ctx_inverted = 0;
	ctx->pipe->res_لفظ = ctx->ctx_res_w;
#اگر_ختم
	اگر (type == PIPE_BG && ctx->list_سر != ctx->pipe) {
		/* Necessary since && and || have precedence over &:
		 * "cmd1 && cmd2 &" must spawn both cmds, not only cmd2,
		 * in a backgrounded subshell.
		 */
		ساخت pipe *pi;
		ساخت command *command;

		/* Is this actually this construct, all pipes end with && or ||? */
		pi = ctx->list_سر;
		جب (pi != ctx->pipe) {
			اگر (pi->followup != PIPE_AND && pi->followup != PIPE_OR)
				جاؤ no_conv;
			pi = pi->اگلا;
		}

		debug_printf_parse("BG with more than one pipe, converting to { p1 &&...pN; } &\n");
		pi->followup = PIPE_SEQ; /* close pN _not_ with "&"! */
		pi = xzalloc(حجم(*pi));
		pi->followup = PIPE_BG;
		pi->عدد_cmds = 1;
		pi->cmds = xzalloc(حجم(pi->cmds[0]));
		command = &pi->cmds[0];
		اگر (CMD_معمول != 0) /* "if xzalloc didn't do that already" */
			command->cmd_type = CMD_معمول;
		command->گروہ = ctx->list_سر;
#اگر !BB_MMU
		command->گروہ_as_string = xstrndup(
			    ctx->as_string.ڈیٹا,
			    ctx->as_string.لمبائی - 1 /* do not copy last char, "&" */
		);
#اگر_ختم
		/* Replace all pipes in ctx with one newly created */
		ctx->list_سر = ctx->pipe = pi;
		/* for cases like "cmd && &", do not be tricked by last command
		 * being null - the entire {...} & is NOT null! */
		not_null = 1;
	} ورنہ {
 no_conv:
		ctx->pipe->followup = type;
	}

	/* Without this check, even just <enter> on command line generates
	 * tree of three NOPs (!). Which is harmless but annoying.
	 * IOW: it is safe to do it unconditionally. */
	اگر (not_null
#اگر ENABLE_HUSH_IF
	 || ctx->ctx_res_w == RES_FI
#اگر_ختم
#اگر ENABLE_HUSH_LOOPS
	 || ctx->ctx_res_w == RES_مکمل
	 || ctx->ctx_res_w == RES_FOR
	 || ctx->ctx_res_w == RES_اندر
#اگر_ختم
#اگر ENABLE_HUSH_CASE
	 || ctx->ctx_res_w == RES_ESAC
#اگر_ختم
	) {
		ساخت pipe *new_p;
		debug_printf_parse("done_pipe: adding new pipe: "
#اگر_غیر_تعریف_شدہ __U_BOOT__
				"not_null:%d ctx->ctx_res_w:%d\n",
				not_null, ctx->ctx_res_w);
#ورنہ /* __U_BOOT__ */
				"not_null:%d\n",
				not_null);
#اگر_ختم /* __U_BOOT__ */
		new_p = new_pipe();
		ctx->pipe->اگلا = new_p;
		ctx->pipe = new_p;
		/* RES_THEN, RES_DO etc are "sticky" -
		 * they remain set for pipes inside if/while.
		 * This is used to control execution.
		 * RES_FOR and RES_IN are NOT sticky (needed to support
		 * cases where variable or value happens to match a keyword):
		 */
#اگر ENABLE_HUSH_LOOPS
		اگر (ctx->ctx_res_w == RES_FOR
		 || ctx->ctx_res_w == RES_اندر)
			ctx->ctx_res_w = RES_NONE;
#اگر_ختم
#اگر ENABLE_HUSH_CASE
		اگر (ctx->ctx_res_w == RES_مطابقت)
			ctx->ctx_res_w = RES_CASE_BODY;
		اگر (ctx->ctx_res_w == RES_CASE)
			ctx->ctx_res_w = RES_CASE_اندر;
#اگر_ختم
		ctx->command = تہی; /* trick done_command below */
		/* Create the memory for command, roughly:
		 * ctx->pipe->cmds = new struct command;
		 * ctx->command = &ctx->pipe->cmds[0];
		 */
		مکمل_command(ctx);
		//debug_print_tree(ctx->list_head, 10);
	}
	debug_printf_parse("done_pipe return\n");
}

ساکن خالی initialize_context(ساخت parse_context *ctx)
{
	memset(ctx, 0, حجم(*ctx));
	اگر (MAYBE_ASSIGNMENT != 0)
		ctx->is_assignment = MAYBE_ASSIGNMENT;
	ctx->pipe = ctx->list_سر = new_pipe();
	/* Create the memory for command, roughly:
	 * ctx->pipe->cmds = new struct command;
	 * ctx->command = &ctx->pipe->cmds[0];
	 */
	مکمل_command(ctx);
}

/* If a reserved word is found and processed, parse context is modified
 * and 1 is returned.
 */
#اگر HAS_KEYWORDS
ساخت reserved_combo {
	حرف literal[6];
	غیرعلامتی حرف res;
	غیرعلامتی حرف assignment_flag;
	uint32_t flag;
};
شمار {
	FLAG_انتہا   = (1 << RES_NONE ),
# اگر ENABLE_HUSH_IF
	FLAG_IF    = (1 << RES_IF   ),
	FLAG_THEN  = (1 << RES_THEN ),
	FLAG_ELIF  = (1 << RES_ELIF ),
	FLAG_ELSE  = (1 << RES_ELSE ),
	FLAG_FI    = (1 << RES_FI   ),
# endif
# اگر ENABLE_HUSH_LOOPS
	FLAG_FOR   = (1 << RES_FOR  ),
	FLAG_WHILE = (1 << RES_WHILE),
	FLAG_UNTIL = (1 << RES_UNTIL),
	FLAG_DO    = (1 << RES_DO   ),
	FLAG_مکمل  = (1 << RES_مکمل ),
	FLAG_اندر    = (1 << RES_اندر   ),
# endif
# اگر ENABLE_HUSH_CASE
	FLAG_مطابقت = (1 << RES_مطابقت),
	FLAG_ESAC  = (1 << RES_ESAC ),
# endif
	FLAG_آغاز = (1 << RES_XXXX ),
};

ساکن ثابت ساخت reserved_combo* مطابقت_reserved_لفظ(o_string *لفظ)
{
	/* Mostly a list of accepted follow-up reserved words.
	 * FLAG_END means we are done with the sequence, and are ready
	 * to turn the compound list into a command.
	 * FLAG_START means the word must start a new compound list.
	 */
	ساکن ثابت ساخت reserved_combo reserved_list[] ALIGN4 = {
# اگر ENABLE_HUSH_IF
		{ "!",     RES_NONE,  NOT_ASSIGNMENT  , 0 },
		{ "if",    RES_IF,    MAYBE_ASSIGNMENT, FLAG_THEN | FLAG_آغاز },
		{ "then",  RES_THEN,  MAYBE_ASSIGNMENT, FLAG_ELIF | FLAG_ELSE | FLAG_FI },
		{ "elif",  RES_ELIF,  MAYBE_ASSIGNMENT, FLAG_THEN },
		{ "else",  RES_ELSE,  MAYBE_ASSIGNMENT, FLAG_FI   },
		{ "fi",    RES_FI,    NOT_ASSIGNMENT  , FLAG_انتہا  },
# endif
# اگر ENABLE_HUSH_LOOPS
		{ "for",   RES_FOR,   NOT_ASSIGNMENT  , FLAG_اندر | FLAG_DO | FLAG_آغاز },
		{ "while", RES_WHILE, MAYBE_ASSIGNMENT, FLAG_DO | FLAG_آغاز },
		{ "until", RES_UNTIL, MAYBE_ASSIGNMENT, FLAG_DO | FLAG_آغاز },
		{ "in",    RES_اندر,    NOT_ASSIGNMENT  , FLAG_DO   },
		{ "do",    RES_DO,    MAYBE_ASSIGNMENT, FLAG_مکمل },
		{ "done",  RES_مکمل,  NOT_ASSIGNMENT  , FLAG_انتہا  },
# endif
# اگر ENABLE_HUSH_CASE
		{ "case",  RES_CASE,  NOT_ASSIGNMENT  , FLAG_مطابقت | FLAG_آغاز },
		{ "esac",  RES_ESAC,  NOT_ASSIGNMENT  , FLAG_انتہا  },
# endif
	};
	ثابت ساخت reserved_combo *r;

	برائے (r = reserved_list; r < reserved_list + ARRAY_سائز(reserved_list); r++) {
		اگر (strcmp(لفظ->ڈیٹا, r->literal) == 0)
			واپس r;
	}
	واپس تہی;
}
/* Return NULL: not a keyword, else: keyword
 */
ساکن ثابت ساخت reserved_combo* reserved_لفظ(ساخت parse_context *ctx)
{
# اگر ENABLE_HUSH_CASE
	ساکن ثابت ساخت reserved_combo reserved_مطابقت = {
		"",        RES_مطابقت, NOT_ASSIGNMENT , FLAG_مطابقت | FLAG_ESAC
	};
# endif
	ثابت ساخت reserved_combo *r;

	اگر (ctx->لفظ.has_quoted_part)
		واپس 0;
	r = مطابقت_reserved_لفظ(&ctx->لفظ);
	اگر (!r)
		واپس r; /* NULL */

	debug_printf("found reserved word %s, res %d\n", r->literal, r->res);
# اگر ENABLE_HUSH_CASE
	اگر (r->res == RES_اندر && ctx->ctx_res_w == RES_CASE_اندر) {
		/* "case word IN ..." - IN part starts first MATCH part */
		r = &reserved_مطابقت;
	} ورنہ
# endif
	اگر (r->flag == 0) { /* '!' */
		اگر (ctx->ctx_inverted) { /* bash doesn't accept '! ! true' */
			syntax_error("! ! command");
			ctx->ctx_res_w = RES_SNTX;
		}
		ctx->ctx_inverted = 1;
		واپس r;
	}
	اگر (r->flag & FLAG_آغاز) {
		ساخت parse_context *old;

		old = xmemdup(ctx, حجم(*ctx));
		debug_printf_parse("push stack %p\n", old);
		initialize_context(ctx);
		ctx->ڈھیر = old;
	} ورنہ اگر (/*ctx->ctx_res_w == RES_NONE ||*/ !(ctx->old_flag & (1 << r->res))) {
		syntax_error_at(ctx->لفظ.ڈیٹا);
		ctx->ctx_res_w = RES_SNTX;
		واپس r;
	} ورنہ {
		/* "{...} fi" is ok. "{...} if" is not
		 * Example:
		 * if { echo foo; } then { echo bar; } fi */
		اگر (ctx->command->گروہ)
			مکمل_pipe(ctx, PIPE_SEQ);
	}

	ctx->ctx_res_w = r->res;
	ctx->old_flag = r->flag;
	ctx->is_assignment = r->assignment_flag;
	debug_printf_parse("ctx->is_assignment='%s'\n", assignment_flag[ctx->is_assignment]);

	اگر (ctx->old_flag & FLAG_انتہا) {
		ساخت parse_context *old;

		مکمل_pipe(ctx, PIPE_SEQ);
		debug_printf_parse("pop stack %p\n", ctx->ڈھیر);
		old = ctx->ڈھیر;
		old->command->گروہ = ctx->list_سر;
		old->command->cmd_type = CMD_معمول;
# اگر !BB_MMU
		/* At this point, the compound command's string is in
		 * ctx->as_string... except for the leading keyword!
		 * Consider this example: "echo a | if true; then echo a; fi"
		 * ctx->as_string will contain "true; then echo a; fi",
		 * with "if " remaining in old->as_string!
		 */
		{
			حرف *str;
			صحیح len = old->as_string.لمبائی;
			/* Concatenate halves */
			o_addstr(&old->as_string, ctx->as_string.ڈیٹا);
			o_free(&ctx->as_string);
			/* Find where leading keyword starts in first half */
			str = old->as_string.ڈیٹا + len;
			اگر (str > old->as_string.ڈیٹا)
				str--; /* skip whitespace after keyword */
			جب (str > old->as_string.ڈیٹا && حرف_ہے(str[-1]))
				str--;
			/* Ugh, we're done with this horrid hack */
			old->command->گروہ_as_string = xstrdup(str);
			debug_printf_parse("pop, remembering as:'%s'\n",
					old->command->گروہ_as_string);
		}
# endif
		*ctx = *old;   /* physical copy */
		آزاد(old);
	}
	واپس r;
}
#اگر_ختم /* HAS_KEYWORDS */

/* Word is complete, look at it and update parsing context.
 * Normal return is 0. Syntax errors return 1.
 * Note: on return, word is reset, but not o_free'd!
 */
ساکن صحیح مکمل_لفظ(ساخت parse_context *ctx)
{
	ساخت command *command = ctx->command;

	debug_printf_parse("done_word entered: '%s' %p\n", ctx->لفظ.ڈیٹا, command);
	اگر (ctx->لفظ.لمبائی == 0 && !ctx->لفظ.has_quoted_part) {
		debug_printf_parse("done_word return 0: true null, ignored\n");
		واپس 0;
	}

#اگر_غیر_تعریف_شدہ __U_BOOT__
	اگر (ctx->pending_redirect) {
		/* We do not glob in e.g. >*.tmp case. bash seems to glob here
		 * only if run as "bash", not "sh" */
		/* http://pubs.opengroup.org/onlinepubs/9699919799/utilities/V3_chap02.html
		 * "2.7 Redirection
		 * If the redirection operator is "<<" or "<<-", the word
		 * that follows the redirection operator shall be
		 * subjected to quote removal; it is unspecified whether
		 * any of the other expansions occur. For the other
		 * redirection operators, the word that follows the
		 * redirection operator shall be subjected to tilde
		 * expansion, parameter expansion, command substitution,
		 * arithmetic expansion, and quote removal.
		 * Pathname expansion shall not be performed
		 * on the word by a non-interactive shell; an interactive
		 * shell may perform it, but shall do so only when
		 * the expansion would result in one word."
		 */
//bash does not do parameter/command substitution or arithmetic expansion
//for _heredoc_ redirection word: these constructs look for exact eof marker
// as written:
// <<EOF$t
// <<EOF$((1))
// <<EOF`true`  [this case also makes heredoc "quoted", a-la <<"EOF". Probably bash-4.3.43 bug]

		ctx->pending_redirect->rd_filename = xstrdup(ctx->لفظ.ڈیٹا);
		/* Cater for >\file case:
		 * >\a creates file a; >\\a, >"\a", >"\\a" create file \a
		 * Same with heredocs:
		 * for <<\H delim is H; <<\\H, <<"\H", <<"\\H" - \H
		 */
		اگر (ctx->pending_redirect->rd_type == REDIRECT_HEREDOC) {
			unbackslash(ctx->pending_redirect->rd_filename);
			/* Is it <<"HEREDOC"? */
			اگر (ctx->لفظ.has_quoted_part) {
				ctx->pending_redirect->rd_dup |= HEREDOC_QUOTED;
			}
		}
		debug_printf_parse("word stored in rd_filename: '%s'\n", ctx->لفظ.ڈیٹا);
		ctx->pending_redirect = تہی;
	} ورنہ {
#اگر_ختم /* !__U_BOOT__ */
#اگر HAS_KEYWORDS
# اگر ENABLE_HUSH_CASE
		اگر (ctx->ctx_dsemicolon
		 && strcmp(ctx->لفظ.ڈیٹا, "esac") != 0 /* not "... pattern) cmd;; esac" */
		) {
			/* already done when ctx_dsemicolon was set to 1: */
			/* ctx->ctx_res_w = RES_MATCH; */
			ctx->ctx_dsemicolon = 0;
		} ورنہ
# endif
# اگر defined(CMD_TEST2_SINGLEWORD_NOGLOB)
		اگر (command->cmd_type == CMD_TEST2_SINGLEWORD_NOGLOB
		 && strcmp(ctx->لفظ.ڈیٹا, "]]") == 0
		) {
			/* allow "[[ ]] >file" etc */
			command->cmd_type = CMD_SINGLEWORD_NOGLOB;
		} ورنہ
# endif
		اگر (!command->argv /* if it's the first word... */
# اگر ENABLE_HUSH_LOOPS
		 && ctx->ctx_res_w != RES_FOR /* ...not after FOR or IN */
		 && ctx->ctx_res_w != RES_اندر
# endif
# اگر ENABLE_HUSH_CASE
		 && ctx->ctx_res_w != RES_CASE
# endif
		) {
			ثابت ساخت reserved_combo *reserved;
			reserved = reserved_لفظ(ctx);
			debug_printf_parse("checking for reserved-ness: %d\n", !!reserved);
			اگر (reserved) {
# اگر ENABLE_HUSH_LINENO_VAR
/* Case:
 * "while ...; do
 *	cmd ..."
 * If we don't close the pipe _now_, immediately after "do", lineno logic
 * sees "cmd" as starting at "do" - i.e., at the previous line.
 */
				اگر (0
				 IF_HUSH_IF(|| reserved->res == RES_THEN)
				 IF_HUSH_IF(|| reserved->res == RES_ELIF)
				 IF_HUSH_IF(|| reserved->res == RES_ELSE)
				 IF_HUSH_LOOPS(|| reserved->res == RES_DO)
				) {
					مکمل_pipe(ctx, PIPE_SEQ);
				}
# endif
				o_reset_مقصد_خالی_خانے_unquoted(&ctx->لفظ);
				debug_printf_parse("done_word return %d\n",
						(ctx->ctx_res_w == RES_SNTX));
				واپس (ctx->ctx_res_w == RES_SNTX);
			}
# اگر defined(CMD_TEST2_SINGLEWORD_NOGLOB)
			اگر (strcmp(ctx->لفظ.ڈیٹا, "[[") == 0) {
				command->cmd_type = CMD_TEST2_SINGLEWORD_NOGLOB;
			} ورنہ
# endif
# اگر defined(CMD_SINGLEWORD_NOGLOB)
			اگر (0
			/* In bash, local/export/readonly are special, args
			 * are assignments and therefore expansion of them
			 * should be "one-word" expansion:
			 *  $ export i=`echo 'a  b'` # one arg: "i=a  b"
			 * compare with:
			 *  $ ls i=`echo 'a  b'`     # two args: "i=a" and "b"
			 *  ls: cannot access i=a: No such file or directory
			 *  ls: cannot access b: No such file or directory
			 * Note: bash 3.2.33(1) does this only if export word
			 * itself is not quoted:
			 *  $ export i=`echo 'aaa  bbb'`; echo "$i"
			 *  aaa  bbb
			 *  $ "export" i=`echo 'aaa  bbb'`; echo "$i"
			 *  aaa
			 */
			 IF_HUSH_LOCAL(   || strcmp(ctx->لفظ.ڈیٹا, "local") == 0)
			 IF_HUSH_EXPORT(  || strcmp(ctx->لفظ.ڈیٹا, "export") == 0)
			 IF_HUSH_READONLY(|| strcmp(ctx->لفظ.ڈیٹا, "readonly") == 0)
			) {
				command->cmd_type = CMD_SINGLEWORD_NOGLOB;
			}
# ورنہ
			{ /* empty block to pair "if ... else" */ }
# endif
		}
#اگر_ختم /* HAS_KEYWORDS */

		اگر (command->گروہ) {
			/* "{ echo foo; } echo bar" - bad */
			syntax_error_at(ctx->لفظ.ڈیٹا);
			debug_printf_parse("done_word return 1: syntax error, "
					"groups and arglists don't mix\n");
			واپس 1;
		}

		/* If this word wasn't an assignment, next ones definitely
		 * can't be assignments. Even if they look like ones. */
		اگر (ctx->is_assignment != DEFINITELY_ASSIGNMENT
		 && ctx->is_assignment != لفظ_IS_KEYWORD
		) {
			ctx->is_assignment = NOT_ASSIGNMENT;
		} ورنہ {
			اگر (ctx->is_assignment == DEFINITELY_ASSIGNMENT) {
				command->assignment_cnt++;
				debug_printf_parse("++assignment_cnt=%d\n", command->assignment_cnt);
			}
			debug_printf_parse("ctx->is_assignment was:'%s'\n", assignment_flag[ctx->is_assignment]);
			ctx->is_assignment = MAYBE_ASSIGNMENT;
		}
		debug_printf_parse("ctx->is_assignment='%s'\n", assignment_flag[ctx->is_assignment]);
		command->argv = جوڑو_string_مقصد_لڑیاں(command->argv, xstrdup(ctx->لفظ.ڈیٹا));
#اگر_تعریف_شدہ __U_BOOT__
		command->argc++;
#اگر_ختم /* __U_BOOT__ */
		debug_print_لڑیاں("word appended to argv", command->argv);

#اگر_غیر_تعریف_شدہ __U_BOOT__
	}
#اگر_ختم /* !__U_BOOT__ */

#اگر ENABLE_HUSH_LOOPS
	اگر (ctx->ctx_res_w == RES_FOR) {
		اگر (ctx->لفظ.has_quoted_part
		 || endofname(command->argv[0])[0] != '\0'
		) {
			/* bash says just "not a valid identifier" */
			syntax_error("bad for loop variable");
			واپس 1;
		}
		/* Force FOR to have just one word (variable name) */
		/* NB: basically, this makes hush see "for v in ..."
		 * syntax as if it is "for v; in ...". FOR and IN become
		 * two pipe structs in parse tree. */
		مکمل_pipe(ctx, PIPE_SEQ);
	}
#اگر_ختم
#اگر ENABLE_HUSH_CASE
	/* Force CASE to have just one word */
	اگر (ctx->ctx_res_w == RES_CASE) {
		مکمل_pipe(ctx, PIPE_SEQ);
	}
#اگر_ختم

	o_reset_مقصد_خالی_خانے_unquoted(&ctx->لفظ);

	debug_printf_parse("done_word return 0\n");
	واپس 0;
}

#اگر_غیر_تعریف_شدہ __U_BOOT__
/* Peek ahead in the input to find out if we have a "&n" construct,
 * as in "2>&1", that represents duplicating a file descriptor.
 * Return:
 * REDIRFD_CLOSE if >&- "close fd" construct is seen,
 * REDIRFD_SYNTAX_ERR if syntax error,
 * REDIRFD_TO_FILE if no & was seen,
 * or the number found.
 */
#اگر BB_MMU
#تعریف_کرو parse_redir_دایاں_fd(as_string, داخلہ) \
	parse_redir_دایاں_fd(داخلہ)
#اگر_ختم
ساکن صحیح parse_redir_دایاں_fd(o_string *as_string, ساخت اندر_str *داخلہ)
{
	صحیح ch, d, ok;

	ch = i_peek(داخلہ);
	اگر (ch != '&')
		واپس REDIRFD_مقصد_FILE;

	ch = i_getch(داخلہ);  /* get the & */
	nommu_addchr(as_string, ch);
	ch = i_peek(داخلہ);
	اگر (ch == '-') {
		ch = i_getch(داخلہ);
		nommu_addchr(as_string, ch);
		واپس REDIRFD_CLOSE;
	}
	d = 0;
	ok = 0;
	جب (ch != EOF && ہندسہ_ہے(ch)) {
		d = d*10 + (ch-'0');
		ok = 1;
		ch = i_getch(داخلہ);
		nommu_addchr(as_string, ch);
		ch = i_peek(داخلہ);
	}
	اگر (ok) واپس d;

//TODO: this is the place to catch ">&file" bashism (redirect both fd 1 and 2)

	bb_simple_error_پیغام("ambiguous redirect");
	واپس REDIRFD_SYNTAX_ERR;
}

/* Return code is 0 normal, 1 if a syntax error is detected
 */
ساکن صحیح parse_redirect(ساخت parse_context *ctx,
		صحیح fd,
		redir_type style,
		ساخت اندر_str *داخلہ)
{
	ساخت command *command = ctx->command;
	ساخت redir_struct *redir;
	ساخت redir_struct **redirp;
	صحیح dup_عدد;

	dup_عدد = REDIRFD_مقصد_FILE;
	اگر (style != REDIRECT_HEREDOC) {
		/* Check for a '>&1' type redirect */
		dup_عدد = parse_redir_دایاں_fd(&ctx->as_string, داخلہ);
		اگر (dup_عدد == REDIRFD_SYNTAX_ERR)
			واپس 1;
	} ورنہ {
		صحیح ch = i_peek_and_eat_bkslash_nl(داخلہ);
		dup_عدد = (ch == '-'); /* HEREDOC_SKIPTABS bit is 1 */
		اگر (dup_عدد) { /* <<-... */
			ch = i_getch(داخلہ);
			nommu_addchr(&ctx->as_string, ch);
			ch = i_peek(داخلہ);
		}
	}

	اگر (style == REDIRECT_OVERWRITE && dup_عدد == REDIRFD_مقصد_FILE) {
		صحیح ch = i_peek_and_eat_bkslash_nl(داخلہ);
		اگر (ch == '|') {
			/* >|FILE redirect ("clobbering" >).
			 * Since we do not support "set -o noclobber" yet,
			 * >| and > are the same for now. Just eat |.
			 */
			ch = i_getch(داخلہ);
			nommu_addchr(&ctx->as_string, ch);
		}
	}

	/* Create a new redir_struct and append it to the linked list */
	redirp = &command->redirects;
	جب ((redir = *redirp) != تہی) {
		redirp = &(redir->اگلا);
	}
	*redirp = redir = xzalloc(حجم(*redir));
	/* redir->next = NULL; */
	/* redir->rd_filename = NULL; */
	redir->rd_type = style;
	redir->rd_fd = (fd == -1) ? redir_جدول[style].default_fd : fd;

	debug_printf_parse("redirect type %d %s\n", redir->rd_fd,
				redir_جدول[style].descrip);

	redir->rd_dup = dup_عدد;
	اگر (style != REDIRECT_HEREDOC && dup_عدد != REDIRFD_مقصد_FILE) {
		/* Erik had a check here that the file descriptor in question
		 * is legit; I postpone that to "run time"
		 * A "-" representation of "close me" shows up as a -3 here */
		debug_printf_parse("duplicating redirect '%d>&%d'\n",
				redir->rd_fd, redir->rd_dup);
	} ورنہ {
#اگر 0		/* Instead we emit error message at run time */
		اگر (ctx->pending_redirect) {
			/* For example, "cmd > <file" */
			syntax_error("invalid redirect");
		}
#اگر_ختم
		/* Set ctx->pending_redirect, so we know what to do at the
		 * end of the next parsed word. */
		ctx->pending_redirect = redir;
	}
	واپس 0;
}

/* If a redirect is immediately preceded by a number, that number is
 * supposed to tell which file descriptor to redirect.  This routine
 * looks for such preceding numbers.  In an ideal world this routine
 * needs to handle all the following classes of redirects...
 *     echo 2>foo     # redirects fd  2 to file "foo", nothing passed to echo
 *     echo 49>foo    # redirects fd 49 to file "foo", nothing passed to echo
 *     echo -2>foo    # redirects fd  1 to file "foo",    "-2" passed to echo
 *     echo 49x>foo   # redirects fd  1 to file "foo",   "49x" passed to echo
 *
 * http://www.opengroup.org/onlinepubs/009695399/utilities/xcu_chap02.html
 * "2.7 Redirection
 * ... If n is quoted, the number shall not be recognized as part of
 * the redirection expression. For example:
 * echo \2>a
 * writes the character 2 into file a"
 * We are getting it right by setting ->has_quoted_part on any \<char>
 *
 * A -1 return means no valid number was found,
 * the caller should use the appropriate default for this redirection.
 */
ساکن صحیح redirect_opt_عدد(o_string *o)
{
	صحیح عدد;

	اگر (o->ڈیٹا == تہی)
		واپس -1;
	عدد = bb_strtou(o->ڈیٹا, تہی, 10);
	اگر (errno || عدد < 0)
		واپس -1;
	o_reset_مقصد_خالی_خانے_unquoted(o);
	واپس عدد;
}

#اگر BB_MMU
#تعریف_کرو fetch_till_str(as_string, داخلہ, لفظ, skip_tabs) \
	fetch_till_str(داخلہ, لفظ, skip_tabs)
#اگر_ختم
ساکن حرف *fetch_till_str(o_string *as_string,
		ساخت اندر_str *داخلہ,
		ثابت حرف *لفظ,
		صحیح heredoc_flags)
{
	o_string heredoc = NULL_O_STRING;
	غیرعلامتی past_EOL;
	صحیح پچھلا = 0; /* not \ */
	صحیح ch;

	/* Starting with "" is necessary for this case:
	 * cat <<EOF
	 *
	 * xxx
	 * EOF
	 */
	heredoc.ڈیٹا = xzalloc(1); /* start as "", not as NULL */

	جاؤ jump_اندر;

	جب (1) {
		ch = i_getch(داخلہ);
		اگر (ch != EOF)
			nommu_addchr(as_string, ch);
		اگر (ch == '\n' || ch == EOF) {
 check_heredoc_انتہا:
#اگر_غیر_تعریف_شدہ __U_BOOT__
			اگر ((heredoc_flags & HEREDOC_QUOTED) || پچھلا != '\\') {
#ورنہ /* __U_BOOT__ */
			اگر (پچھلا != '\\') {
#اگر_ختم
				/* End-of-line, and not a line continuation */
				اگر (strcmp(heredoc.ڈیٹا + past_EOL, لفظ) == 0) {
					heredoc.ڈیٹا[past_EOL] = '\0';
					debug_printf_heredoc("parsed '%s' heredoc '%s'\n", لفظ, heredoc.ڈیٹا);
					واپس heredoc.ڈیٹا;
				}
				اگر (ch == '\n') {
					/* This is a new line.
					 * Remember position and backslash-escaping status.
					 */
					o_addchr(&heredoc, ch);
					پچھلا = ch;
 jump_اندر:
					past_EOL = heredoc.لمبائی;
					/* Get 1st char of next line, possibly skipping leading tabs */
					کرو {
						ch = i_getch(داخلہ);
						اگر (ch != EOF)
							nommu_addchr(as_string, ch);
#اگر_غیر_تعریف_شدہ __U_BOOT__
					} جب ((heredoc_flags & HEREDOC_SKIPTABS) && ch == '\t');
#ورنہ /* __U_BOOT__ */
				} جب (ch == '\t');
#اگر_ختم
					/* If this immediately ended the line,
					 * go back to end-of-line checks.
					 */
					اگر (ch == '\n')
						جاؤ check_heredoc_انتہا;
				}
			} ورنہ {
				/* Backslash-line continuation in an unquoted
				 * heredoc. This does not need special handling
				 * for heredoc body (unquoted heredocs are
				 * expanded on "execution" and that would take
				 * care of this case too), but not the case
				 * of line continuation *in terminator*:
				 *  cat <<EOF
				 *  Ok1
				 *  EO\
				 *  F
				 */
				heredoc.ڈیٹا[--heredoc.لمبائی] = '\0';
				پچھلا = 0; /* not '\' */
				جاری;
			}
		}
		اگر (ch == EOF) {
			o_free(&heredoc);
			واپس تہی; /* error */
		}
		o_addchr(&heredoc, ch);
		nommu_addchr(as_string, ch);
		اگر (پچھلا == '\\' && ch == '\\')
			/* Correctly handle foo\\<eol> (not a line cont.) */
			پچھلا = 0; /* not '\' */
		ورنہ
			پچھلا = ch;
	}
}
#اگر_ختم /* !__U_BOOT__ */

/* Look at entire parse tree for not-yet-loaded REDIRECT_HEREDOCs
 * and load them all. There should be exactly heredoc_cnt of them.
 */
#اگر BB_MMU
#تعریف_کرو fetch_heredocs(as_string, pi, heredoc_cnt, داخلہ) \
	fetch_heredocs(pi, heredoc_cnt, داخلہ)
#اگر_ختم
ساکن صحیح fetch_heredocs(o_string *as_string, ساخت pipe *pi, صحیح heredoc_cnt, ساخت اندر_str *داخلہ)
{
	جب (pi && heredoc_cnt) {
		صحیح i;
		ساخت command *cmd = pi->cmds;

		debug_printf_heredoc("fetch_heredocs: num_cmds:%d cmd argv0:'%s'\n",
				pi->عدد_cmds,
				cmd->argv ? cmd->argv[0] : "NONE"
		);
		برائے (i = 0; i < pi->عدد_cmds; i++) {
#اگر_غیر_تعریف_شدہ __U_BOOT__
			ساخت redir_struct *redir = cmd->redirects;

#اگر_ختم /* !__U_BOOT__ */
			debug_printf_heredoc("fetch_heredocs: %d cmd argv0:'%s'\n",
					i, cmd->argv ? cmd->argv[0] : "NONE");
#اگر_غیر_تعریف_شدہ __U_BOOT__
			جب (redir) {
				اگر (redir->rd_type == REDIRECT_HEREDOC) {
					حرف *p;

					redir->rd_type = REDIRECT_HEREDOC2;
					/* redir->rd_dup is (ab)used to indicate <<- */
					p = fetch_till_str(as_string, داخلہ,
							redir->rd_filename, redir->rd_dup);
					اگر (!p) {
						syntax_error("unexpected EOF in here document");
						واپس -1;
					}
					آزاد(redir->rd_filename);
					redir->rd_filename = p;
					heredoc_cnt--;
				}
				redir = redir->اگلا;
			}
#اگر_ختم /* !__U_BOOT__ */
			اگر (cmd->گروہ) {
				//bb_error_msg("%s:%u heredoc_cnt:%d", __func__, __LINE__, heredoc_cnt);
				heredoc_cnt = fetch_heredocs(as_string, cmd->گروہ, heredoc_cnt, داخلہ);
				//bb_error_msg("%s:%u heredoc_cnt:%d", __func__, __LINE__, heredoc_cnt);
				اگر (heredoc_cnt < 0)
					واپس heredoc_cnt; /* error */
			}
			cmd++;
		}
		pi = pi->اگلا;
	}
	واپس heredoc_cnt;
}

ساکن صحیح تسلسل_list(ساخت pipe *pi);
#اگر BB_MMU
#تعریف_کرو parse_دھارا(pstring, heredoc_cnt_ptr, داخلہ, انتہا_trigger) \
	parse_دھارا(heredoc_cnt_ptr, داخلہ, انتہا_trigger)
#اگر_ختم
ساکن ساخت pipe *parse_دھارا(حرف **pstring,
		صحیح *heredoc_cnt_ptr,
		ساخت اندر_str *داخلہ,
		صحیح انتہا_trigger);

/* Returns number of heredocs not yet consumed,
 * or -1 on error.
 */
ساکن صحیح parse_گروہ(ساخت parse_context *ctx,
		ساخت اندر_str *داخلہ, صحیح ch)
{
	/* ctx->word contains characters seen prior to ( or {.
	 * Typically it's empty, but for function defs,
	 * it contains function name (without '()'). */
#اگر BB_MMU
# define as_string تہی
#ورنہ
	حرف *as_string = تہی;
#اگر_ختم
	ساخت pipe *pipe_list;
	صحیح heredoc_cnt = 0;
	صحیح endch;
	ساخت command *command = ctx->command;

	debug_printf_parse("parse_group entered\n");
#اگر ENABLE_HUSH_FUNCTIONS
	اگر (ch == '(' && !ctx->لفظ.has_quoted_part) {
		اگر (ctx->لفظ.لمبائی)
			اگر (مکمل_لفظ(ctx))
				واپس -1;
		اگر (!command->argv)
			جاؤ skip; /* (... */
		اگر (command->argv[1]) { /* word word ... (... */
			syntax_error_unexpected_ch('(');
			واپس -1;
		}
		/* it is "word(..." or "word (..." */
		کرو
			ch = i_getch(داخلہ);
		جب (ch == ' ' || ch == '\t');
		اگر (ch != ')') {
			syntax_error_unexpected_ch(ch);
			واپس -1;
		}
		nommu_addchr(&ctx->as_string, ch);
		کرو
			ch = i_getch(داخلہ);
		جب (ch == ' ' || ch == '\t' || ch == '\n');
		اگر (ch != '{' && ch != '(') {
			syntax_error_unexpected_ch(ch);
			واپس -1;
		}
//bash allows functions named "123", "..", "return"!
//		if (endofname(command->argv[0])[0] != '\0') {
//			syntax_error("bad function name");
//			return -1;
//		}
		nommu_addchr(&ctx->as_string, ch);
		command->cmd_type = CMD_FUNCDEF;
		جاؤ skip;
	}
#اگر_ختم

#اگر 0 /* Prevented by caller */
	اگر (command->argv /* word [word]{... */
	 || ctx->لفظ.لمبائی /* word{... */
	 || ctx->لفظ.has_quoted_part /* ""{... */
	) {
		syntax_error(تہی);
		debug_printf_parse("parse_group return -1: "
			"syntax error, groups and arglists don't mix\n");
		واپس -1;
	}
#اگر_ختم

#اگر_غیر_تعریف_شدہ __U_BOOT__
 IF_HUSH_FUNCTIONS(skip:)
#اگر_ختم /* !__U_BOOT__ */

	endch = '}';
	اگر (ch == '(') {
		endch = ')';
#اگر_غیر_تعریف_شدہ __U_BOOT__
		IF_HUSH_FUNCTIONS(اگر (command->cmd_type != CMD_FUNCDEF))
			command->cmd_type = CMD_SUBSHELL;
#اگر_ختم /* !__U_BOOT__ */
	} ورنہ {
		/* bash does not allow "{echo...", requires whitespace */
		ch = i_peek(داخلہ);
		اگر (ch != ' ' && ch != '\t' && ch != '\n'
		 && ch != '('	/* but "{(..." is allowed (without whitespace) */
		) {
			syntax_error_unexpected_ch(ch);
			واپس -1;
		}
		اگر (ch != '(') {
			ch = i_getch(داخلہ);
			nommu_addchr(&ctx->as_string, ch);
		}
	}

	debug_printf_heredoc("calling parse_stream, heredoc_cnt:%d\n", heredoc_cnt);
	pipe_list = parse_دھارا(&as_string, &heredoc_cnt, داخلہ, endch);
	debug_printf_heredoc("parse_stream returned: heredoc_cnt:%d\n", heredoc_cnt);
#اگر !BB_MMU
	اگر (as_string)
		o_addstr(&ctx->as_string, as_string);
#اگر_ختم

	/* empty ()/{} or parse error? */
	اگر (!pipe_list || pipe_list == ERR_PTR) {
		/* parse_stream already emitted error msg */
		اگر (!BB_MMU)
			آزاد(as_string);
		debug_printf_parse("parse_group return -1: "
			"parse_stream returned %p\n", pipe_list);
		واپس -1;
	}
#اگر !BB_MMU
	as_string[لڑی_لمبائی(as_string) - 1] = '\0'; /* plink ')' or '}' */
	command->گروہ_as_string = as_string;
	debug_printf_parse("end of group, remembering as:'%s'\n",
			command->گروہ_as_string);
#اگر_ختم

#اگر ENABLE_HUSH_FUNCTIONS
	/* Convert "f() (cmds)" to "f() {(cmds)}" */
	اگر (command->cmd_type == CMD_FUNCDEF && endch == ')') {
		ساخت command *cmd2;

		cmd2 = xzalloc(حجم(*cmd2));
		cmd2->cmd_type = CMD_SUBSHELL;
		cmd2->گروہ = pipe_list;
# اگر !BB_MMU
//UNTESTED!
		cmd2->گروہ_as_string = command->گروہ_as_string;
		command->گروہ_as_string = xasprintf("(%s)", command->گروہ_as_string);
# endif

		pipe_list = new_pipe();
		pipe_list->cmds = cmd2;
		pipe_list->عدد_cmds = 1;
	}
#اگر_ختم

	command->گروہ = pipe_list;

	debug_printf_parse("parse_group return %d\n", heredoc_cnt);
	واپس heredoc_cnt;
	/* command remains "open", available for possible redirects */
#تعریف_منسوخ as_string
}

#اگر_غیر_تعریف_شدہ __U_BOOT__
#اگر ENABLE_HUSH_TICK || ENABLE_FEATURE_SH_MATH || ENABLE_HUSH_DOLLAR_OPS
/* Subroutines for copying $(...) and `...` things */
/* '...' */
ساکن صحیح جوڑو_till_single_quote(o_string *dest, ساخت اندر_str *داخلہ)
{
	جب (1) {
		صحیح ch = i_getch(داخلہ);
		اگر (ch == EOF) {
			syntax_error_unterm_ch('\'');
			واپس 0;
		}
		اگر (ch == '\'')
			واپس 1;
		o_addchr(dest, ch);
	}
}
ساکن صحیح جوڑو_till_single_quote_dquoted(o_string *dest, ساخت اندر_str *داخلہ)
{
	جب (1) {
		صحیح ch = i_getch(داخلہ);
		اگر (ch == EOF) {
			syntax_error_unterm_ch('\'');
			واپس 0;
		}
		اگر (ch == '\'')
			واپس 1;
		o_addqchr(dest, ch);
	}
}

/* "...\"...`..`...." - do we need to handle "...$(..)..." too? */
ساکن صحیح جوڑو_till_backquote(o_string *dest, ساخت اندر_str *داخلہ, صحیح اندر_dquote);
ساکن صحیح جوڑو_till_double_quote(o_string *dest, ساخت اندر_str *داخلہ)
{
	جب (1) {
		صحیح ch = i_getch(داخلہ);
		اگر (ch == EOF) {
			syntax_error_unterm_ch('"');
			واپس 0;
		}
		اگر (ch == '"')
			واپس 1;
		اگر (ch == '\\') {  /* \x. Copy both chars. */
			o_addchr(dest, ch);
			ch = i_getch(داخلہ);
		}
		o_addchr(dest, ch);
		اگر (ch == '`') {
			اگر (!جوڑو_till_backquote(dest, داخلہ, /*in_dquote:*/ 1))
				واپس 0;
			o_addchr(dest, ch);
			جاری;
		}
		//if (ch == '$') ...
	}
}

/* Process `cmd` - copy contents until "`" is seen. Complicated by
 * \` quoting.
 * "Within the backquoted style of command substitution, backslash
 * shall retain its literal meaning, except when followed by: '$', '`', or '\'.
 * The search for the matching backquote shall be satisfied by the first
 * backquote found without a preceding backslash; during this search,
 * if a non-escaped backquote is encountered within a shell comment,
 * a here-document, an embedded command substitution of the $(command)
 * form, or a quoted string, undefined results occur. A single-quoted
 * or double-quoted string that begins, but does not end, within the
 * "`...`" sequence produces undefined results."
 * Example                               Output
 * echo `echo '\'TEST\`echo ZZ\`BEST`    \TESTZZBEST
 */
ساکن صحیح جوڑو_till_backquote(o_string *dest, ساخت اندر_str *داخلہ, صحیح اندر_dquote)
{
	جب (1) {
		صحیح ch = i_getch(داخلہ);
		اگر (ch == '`')
			واپس 1;
		اگر (ch == '\\') {
			/* \x. Copy both unless it is \`, \$, \\ and maybe \" */
			ch = i_getch(داخلہ);
			اگر (ch != '`'
			 && ch != '$'
			 && ch != '\\'
			 && (!اندر_dquote || ch != '"')
			) {
				o_addchr(dest, '\\');
			}
		}
		اگر (ch == EOF) {
			syntax_error_unterm_ch('`');
			واپس 0;
		}
		o_addchr(dest, ch);
	}
}
/* Process $(cmd) - copy contents until ")" is seen. Complicated by
 * quoting and nested ()s.
 * "With the $(command) style of command substitution, all characters
 * following the open parenthesis to the matching closing parenthesis
 * constitute the command. Any valid shell script can be used for command,
 * except a script consisting solely of redirections which produces
 * unspecified results."
 * Example                              Output
 * echo $(echo '(TEST)' BEST)           (TEST) BEST
 * echo $(echo 'TEST)' BEST)            TEST) BEST
 * echo $(echo \(\(TEST\) BEST)         ((TEST) BEST
 *
 * Also adapted to eat ${var%...} and $((...)) constructs, since ... part
 * can contain arbitrary constructs, just like $(cmd).
 * In bash compat mode, it needs to also be able to stop on ':' or '/'
 * for ${var:N[:M]} and ${var/P[/R]} parsing.
 */
#تعریف_کرو DOUBLE_CLOSE_CHAR_FLAG 0x80
ساکن صحیح جوڑو_till_closing_bracket(o_string *dest, ساخت اندر_str *داخلہ, غیرعلامتی انتہا_ch)
{
	صحیح ch;
	حرف dbl = انتہا_ch & DOUBLE_CLOSE_CHAR_FLAG;
# اگر BASH_SUBSTR || BASH_نمونہ_SUBST
	حرف انتہا_char2 = انتہا_ch >> 8;
# endif
	انتہا_ch &= (DOUBLE_CLOSE_CHAR_FLAG - 1);

# اگر ENABLE_HUSH_INTERACTIVE
	G.promptmode = 1; /* PS2 */
# endif
	debug_printf_prompt("%s promptmode=%d\n", __func__, G.promptmode);

	جب (1) {
		ch = i_getch(داخلہ);
		اگر (ch == EOF) {
			syntax_error_unterm_ch(انتہا_ch);
			واپس 0;
		}
		اگر (ch == انتہا_ch
# اگر BASH_SUBSTR || BASH_نمونہ_SUBST
		 || ch == انتہا_char2
# endif
		) {
			اگر (!dbl)
				توڑو;
			/* we look for closing )) of $((EXPR)) */
			اگر (i_peek_and_eat_bkslash_nl(داخلہ) == انتہا_ch) {
				i_getch(داخلہ); /* eat second ')' */
				توڑو;
			}
		}
		o_addchr(dest, ch);
		//bb_error_msg("%s:o_addchr('%c')", __func__, ch);
		اگر (ch == '(' || ch == '{') {
			ch = (ch == '(' ? ')' : '}');
			اگر (!جوڑو_till_closing_bracket(dest, داخلہ, ch))
				واپس 0;
			o_addchr(dest, ch);
			جاری;
		}
		اگر (ch == '\'') {
			اگر (!جوڑو_till_single_quote(dest, داخلہ))
				واپس 0;
			o_addchr(dest, ch);
			جاری;
		}
		اگر (ch == '"') {
			اگر (!جوڑو_till_double_quote(dest, داخلہ))
				واپس 0;
			o_addchr(dest, ch);
			جاری;
		}
		اگر (ch == '`') {
			اگر (!جوڑو_till_backquote(dest, داخلہ, /*in_dquote:*/ 0))
				واپس 0;
			o_addchr(dest, ch);
			جاری;
		}
		اگر (ch == '\\') {
			/* \x. Copy verbatim. Important for  \(, \) */
			ch = i_getch(داخلہ);
			اگر (ch == EOF) {
				syntax_error_unterm_ch(انتہا_ch);
				واپس 0;
			}
# اگر 0
			اگر (ch == '\n') {
				/* "backslash+newline", ignore both */
				o_delchr(dest); /* undo insertion of '\' */
				جاری;
			}
# endif
			o_addchr(dest, ch);
			//bb_error_msg("%s:o_addchr('%c') after '\\'", __func__, ch);
			جاری;
		}
	}
	debug_printf_parse("%s return '%s' ch:'%c'\n", __func__, dest->ڈیٹا, ch);
	واپس ch;
}
#اگر_ختم /* ENABLE_HUSH_TICK || ENABLE_FEATURE_SH_MATH || ENABLE_HUSH_DOLLAR_OPS */

#اگر BASH_DOLLAR_SQUOTE
/* Return code: 1 for "found and parsed", 0 for "seen something else" */
# اگر BB_MMU
#تعریف_کرو parse_dollar_squote(as_string, dest, داخلہ) \
	parse_dollar_squote(dest, داخلہ)
#تعریف_کرو as_string تہی
# endif
ساکن صحیح parse_dollar_squote(o_string *as_string, o_string *dest, ساخت اندر_str *داخلہ)
{
	صحیح آغاز;
	صحیح ch = i_peek_and_eat_bkslash_nl(داخلہ);  /* first character after the $ */
	debug_printf_parse("parse_dollar_squote entered: ch='%c'\n", ch);
	اگر (ch != '\'')
		واپس 0;

	dest->has_quoted_part = 1;
	آغاز = dest->لمبائی;

	ch = i_getch(داخلہ); /* eat ' */
	nommu_addchr(as_string, ch);
	جب (1) {
		ch = i_getch(داخلہ);
		nommu_addchr(as_string, ch);
		اگر (ch == EOF) {
			syntax_error_unterm_ch('\'');
			واپس 0;
		}
		اگر (ch == '\'')
			توڑو;
		اگر (ch == SPECIAL_VAR_علامت) {
			/* Convert raw ^C to corresponding special variable reference */
			o_addchr(dest, SPECIAL_VAR_علامت);
			o_addchr(dest, SPECIAL_VAR_QUOTED_SVS);
			/* will addchr() another SPECIAL_VAR_SYMBOL (see after the if() block) */
		} ورنہ اگر (ch == '\\') {
			ساکن ثابت حرف C_escapes[] ALIGN1 = "nrbtfav""x\\01234567";

			ch = i_getch(داخلہ);
			nommu_addchr(as_string, ch);
			اگر (strchr(C_escapes, ch)) {
				حرف buf[4];
				حرف *p = buf;
				صحیح cnt = 2;

				buf[0] = ch;
				اگر ((غیرعلامتی حرف)(ch - '0') <= 7) { /* \ooo */
					کرو {
						ch = i_peek(داخلہ);
						اگر ((غیرعلامتی حرف)(ch - '0') > 7)
							توڑو;
						*++p = ch = i_getch(داخلہ);
						nommu_addchr(as_string, ch);
					} جب (--cnt != 0);
				} ورنہ اگر (ch == 'x') { /* \xHH */
					کرو {
						ch = i_peek(داخلہ);
						اگر (!isxdigit(ch))
							توڑو;
						*++p = ch = i_getch(داخلہ);
						nommu_addchr(as_string, ch);
					} جب (--cnt != 0);
					اگر (cnt == 2) { /* \x but next char is "bad" */
						ch = 'x';
						جاؤ unrecognized;
					}
				} /* else simple seq like \\ or \t */
				*++p = '\0';
				p = buf;
				ch = bb_process_escape_sequence((خالی*)&p);
				//bb_error_msg("buf:'%s' ch:%x", buf, ch);
				اگر (ch == '\0')
					جاری; /* bash compat: $'...\0...' emits nothing */
			} ورنہ { /* unrecognized "\z": encode both chars unless ' or " */
				اگر (ch != '\'' && ch != '"') {
 unrecognized:
					o_addqchr(dest, '\\');
				}
			}
		} /* if (\...) */
		o_addqchr(dest, ch);
	}

	اگر (dest->لمبائی == آغاز) {
		/* $'', $'\0', $'\000\x00' and the like */
		o_addchr(dest, SPECIAL_VAR_علامت);
		o_addchr(dest, SPECIAL_VAR_علامت);
	}

	واپس 1;
# undef as_string
}
#ورنہ
# define parse_dollar_squote(as_string, dest, داخلہ) 0
#اگر_ختم /* BASH_DOLLAR_SQUOTE */
#اگر_ختم /* !__U_BOOT__ */

/* Return code: 0 for OK, 1 for syntax error */
#اگر BB_MMU
#تعریف_کرو parse_dollar(as_string, dest, داخلہ, quote_نقاب) \
	parse_dollar(dest, داخلہ, quote_نقاب)
#تعریف_کرو as_string تہی
#اگر_ختم
ساکن صحیح parse_dollar(o_string *as_string,
		o_string *dest,
		ساخت اندر_str *داخلہ, غیرعلامتی حرف quote_نقاب)
{
	صحیح ch = i_peek_and_eat_bkslash_nl(داخلہ);  /* first character after the $ */

	debug_printf_parse("parse_dollar entered: ch='%c' quote_mask:0x%x\n", ch, quote_نقاب);
	اگر (حرف_ہے(ch)) {
 make_var:
		ch = i_getch(داخلہ);
		nommu_addchr(as_string, ch);
 /*make_var1:*/
		o_addchr(dest, SPECIAL_VAR_علامت);
		جب (1) {
			debug_printf_parse(": '%c'\n", ch);
			o_addchr(dest, ch | quote_نقاب);
			quote_نقاب = 0;
			ch = i_peek_and_eat_bkslash_nl(داخلہ);
			اگر (!حرف_یا_ہندسہ(ch) && ch != '_') {
				/* End of variable name reached */
				توڑو;
			}
			ch = i_getch(داخلہ);
			nommu_addchr(as_string, ch);
		}
		o_addchr(dest, SPECIAL_VAR_علامت);
	} ورنہ اگر (ہندسہ_ہے(ch)) {
 make_one_char_var:
		ch = i_getch(داخلہ);
		nommu_addchr(as_string, ch);
		o_addchr(dest, SPECIAL_VAR_علامت);
		debug_printf_parse(": '%c'\n", ch);
		o_addchr(dest, ch | quote_نقاب);
		o_addchr(dest, SPECIAL_VAR_علامت);
	} ورنہ انتخاب (ch) {
#اگر_غیر_تعریف_شدہ __U_BOOT__
	حالت '$': /* pid */
	حالت '!': /* last bg pid */
#اگر_ختم /* !__U_BOOT__ */
	حالت '?': /* last exit code */
	حالت '#': /* number of args */
	حالت '*': /*دلائل*/
	حالت '@': /*دلائل*/
	حالت '-': /* $- option flags set by set builtin or shell options (-i etc) */
		جاؤ make_one_char_var;
	حالت '{': {
		حرف len_single_ch;

		o_addchr(dest, SPECIAL_VAR_علامت);

		ch = i_getch(داخلہ); /* eat '{' */
		nommu_addchr(as_string, ch);

		ch = i_getch_and_eat_bkslash_nl(داخلہ); /* first char after '{' */
		/* It should be ${?}, or ${#var},
		 * or even ${?+subst} - operator acting on a special variable,
		 * or the beginning of variable name.
		 */
		اگر (ch == EOF
		 || (!strchr(_SPECIAL_VARS_STR, ch) && !حرف_یا_ہندسہ(ch)) /* not one of those */
		) {
 bad_dollar_syntax:
			syntax_error_unterm_str("${name}");
			debug_printf_parse("parse_dollar return 0: unterminated ${name}\n");
			واپس 0;
		}
		nommu_addchr(as_string, ch);
		len_single_ch = ch;
		ch |= quote_نقاب;

		/* It's possible to just call add_till_closing_bracket() at this point.
		 * However, this regresses some of our testsuite cases
		 * which check invalid constructs like ${%}.
		 * Oh well... let's check that the var name part is fine... */

		اگر (ہندسہ_ہے(len_single_ch)
		 || (len_single_ch == '#' && ہندسہ_ہے(i_peek_and_eat_bkslash_nl(داخلہ)))
		) {
			/* Execution engine uses plain xatoi_positive()
			 * to interpret ${NNN} and {#NNN},
			 * check syntax here in the parser.
			 * (bash does not support expressions in ${#NN},
			 * e.g. ${#$var} and {#1:+WORD} are not supported).
			 */
			غیرعلامتی cnt = 9; /* max 9 digits for ${NN} and 8 for {#NN} */
			جب (1) {
				o_addchr(dest, ch);
				debug_printf_parse(": '%c'\n", ch);
				ch = i_getch_and_eat_bkslash_nl(داخلہ);
				nommu_addchr(as_string, ch);
				اگر (ch == '}')
					توڑو;
				اگر (--cnt == 0)
					جاؤ bad_dollar_syntax;
				اگر (len_single_ch != '#' && strchr(VAR_SUBST_OPS, ch))
					/* ${NN<op>...} is valid */
					جاؤ eat_until_closing;
				اگر (!ہندسہ_ہے(ch))
					جاؤ bad_dollar_syntax;
			}
		} ورنہ
		جب (1) {
			غیرعلامتی pos;

			o_addchr(dest, ch);
			debug_printf_parse(": '%c'\n", ch);

			ch = i_getch(داخلہ);
			nommu_addchr(as_string, ch);
			اگر (ch == '}')
				توڑو;
#اگر_غیر_تعریف_شدہ __U_BOOT__
			اگر (!حرف_یا_ہندسہ(ch) && ch != '_') {
#ورنہ /* __U_BOOT__ */
			/*
			 * In several places in U-Boot, we use variable like
			 * foo# (e.g. serial#), particularly in env.
			 * So, we need to authorize # to appear inside
			 * variable name and then expand this variable.
			 * NOTE Having # in variable name is not permitted in
			 * upstream hush but expansion will be done (even though
			 * the result will be empty).
			 */
			اگر (!حرف_یا_ہندسہ(ch) && ch != '_' && ch != '#') {
#اگر_ختم /* __U_BOOT__ */
				غیرعلامتی انتہا_ch;
#اگر_غیر_تعریف_شدہ __U_BOOT__
				غیرعلامتی حرف آخری_ch;
#اگر_ختم /* !__U_BOOT__ */
				/* handle parameter expansions
				 * http://www.opengroup.org/onlinepubs/009695399/utilities/xcu_chap02.html#tag_02_06_02
				 */
				اگر (!strchr(VAR_SUBST_OPS, ch)) { /* ${var<bad_char>... */
					اگر (len_single_ch != '#'
					/*|| !strchr(SPECIAL_VARS_STR, ch) - disallow errors like ${#+} ? */
					 || i_peek(داخلہ) != '}'
					) {
						جاؤ bad_dollar_syntax;
					}
					/* else: it's "length of C" ${#C} op,
					 * where C is a single char
					 * special var name, e.g. ${#!}.
					 */
				}
 eat_until_closing:
				/* Eat everything until closing '}' (or ':') */
				انتہا_ch = '}';
#اگر_غیر_تعریف_شدہ __U_BOOT__
				اگر (BASH_SUBSTR
				 && ch == ':'
				 && !strchr(MINUS_PLUS_EQUAL_QUESTION, i_peek(داخلہ))
				) {
					/* It's ${var:N[:M]} thing */
					انتہا_ch = '}' * 0x100 + ':';
				}
				اگر (BASH_نمونہ_SUBST
				 && ch == '/'
				) {
					/* It's ${var/[/]pattern[/repl]} thing */
					اگر (i_peek(داخلہ) == '/') { /* ${var//pattern[/repl]}? */
						i_getch(داخلہ);
						nommu_addchr(as_string, '/');
						ch = '\\';
					}
					انتہا_ch = '}' * 0x100 + '/';
				}
#اگر_ختم /* !__U_BOOT__ */
				o_addchr(dest, ch);
				/* The pattern can't be empty.
				 * IOW: if the first char after "${v//" is a slash,
				 * it does not terminate the pattern - it's the first char of the pattern:
				 *  v=/dev/ram; echo ${v////-}  prints -dev-ram (pattern is "/")
				 *  v=/dev/ram; echo ${v///r/-} prints /dev-am  (pattern is "/r")
				 */
				اگر (i_peek(داخلہ) == '/') {
					o_addchr(dest, i_getch(داخلہ));
				}
#اگر_غیر_تعریف_شدہ __U_BOOT__
 again:
#اگر_ختم /* !__U_BOOT__ */
				اگر (!BB_MMU)
					pos = dest->لمبائی;
#اگر ENABLE_HUSH_DOLLAR_OPS
#اگر_غیر_تعریف_شدہ __U_BOOT__
				آخری_ch = جوڑو_till_closing_bracket(dest, داخلہ, انتہا_ch);
				اگر (آخری_ch == 0) /* error? */
					واپس 0;
#اگر_ختم /* !__U_BOOT__ */
#ورنہ
# error Simple رمز مقصد only allow ${var} is not implemented
#اگر_ختم
				اگر (as_string) {
					o_addstr(as_string, dest->ڈیٹا + pos);
#اگر_غیر_تعریف_شدہ __U_BOOT__
					o_addchr(as_string, آخری_ch);
#اگر_ختم /* !__U_BOOT__ */
				}

#اگر_غیر_تعریف_شدہ __U_BOOT__
				اگر ((BASH_SUBSTR || BASH_نمونہ_SUBST)
					 && (انتہا_ch & 0xff00)
				) {
					/* close the first block: */
					o_addchr(dest, SPECIAL_VAR_علامت);
					/* while parsing N from ${var:N[:M]}
					 * or pattern from ${var/[/]pattern[/repl]} */
					اگر ((انتہا_ch & 0xff) == آخری_ch) {
						/* got ':' or '/'- parse the rest */
						انتہا_ch = '}';
						جاؤ again;
					}
					/* got '}' */
					اگر (BASH_SUBSTR && انتہا_ch == '}' * 0x100 + ':') {
						/* it's ${var:N} - emulate :999999999 */
						o_addstr(dest, "999999999");
					} /* else: it's ${var/[/]pattern} */
				}
#اگر_ختم /* !__U_BOOT__ */
				توڑو;
			}
			len_single_ch = 0; /* it can't be ${#C} op */
		}
		o_addchr(dest, SPECIAL_VAR_علامت);
		توڑو;
	}
#اگر ENABLE_FEATURE_SH_MATH || ENABLE_HUSH_TICK
	حالت '(': {
		غیرعلامتی pos;

		ch = i_getch(داخلہ);
		nommu_addchr(as_string, ch);
# اگر ENABLE_FEATURE_SH_MATH
		اگر (i_peek_and_eat_bkslash_nl(داخلہ) == '(') {
			ch = i_getch(داخلہ);
			nommu_addchr(as_string, ch);
			o_addchr(dest, SPECIAL_VAR_علامت);
			o_addchr(dest, quote_نقاب | '+');
			اگر (!BB_MMU)
				pos = dest->لمبائی;
			اگر (!جوڑو_till_closing_bracket(dest, داخلہ, ')' | DOUBLE_CLOSE_CHAR_FLAG))
				واپس 0; /* error */
			اگر (as_string) {
				o_addstr(as_string, dest->ڈیٹا + pos);
				o_addchr(as_string, ')');
				o_addchr(as_string, ')');
			}
			o_addchr(dest, SPECIAL_VAR_علامت);
			توڑو;
		}
# endif
# اگر ENABLE_HUSH_TICK
		o_addchr(dest, SPECIAL_VAR_علامت);
		o_addchr(dest, quote_نقاب | '`');
		اگر (!BB_MMU)
			pos = dest->لمبائی;
		اگر (!جوڑو_till_closing_bracket(dest, داخلہ, ')'))
			واپس 0; /* error */
		اگر (as_string) {
			o_addstr(as_string, dest->ڈیٹا + pos);
			o_addchr(as_string, ')');
		}
		o_addchr(dest, SPECIAL_VAR_علامت);
# endif
		توڑو;
	}
#اگر_ختم
	حالت '_':
		جاؤ make_var;
#اگر 0
	/* TODO: $_: */
	/* $_ Shell or shell script name; or last argument of last command
	 * (if last command wasn't a pipe; if it was, bash sets $_ to "");
	 * but in command's env, set to full pathname used to invoke it */
		ch = i_getch(داخلہ);
		nommu_addchr(as_string, ch);
		ch = i_peek_and_eat_bkslash_nl(داخلہ);
		اگر (حرف_یا_ہندسہ(ch)) { /* it's $_name or $_123 */
			ch = '_';
			جاؤ make_var1;
		}
		/* else: it's $_ */
#اگر_ختم
	طےشدہ:
		o_جوڑو_Qchr(dest, '$');
	}
	debug_printf_parse("parse_dollar return 1 (ok)\n");
	واپس 1;
#تعریف_منسوخ as_string
}

#اگر BB_MMU
#تعریف_کرو encode_string(as_string, dest, داخلہ, dquote_انتہا) \
	encode_string(dest, داخلہ, dquote_انتہا)
#تعریف_کرو as_string تہی
#اگر_ختم
ساکن صحیح encode_string(o_string *as_string,
		o_string *dest,
		ساخت اندر_str *داخلہ,
		صحیح dquote_انتہا)
{
	صحیح ch;
	صحیح اگلا;

 again:
	ch = i_getch(داخلہ);
	اگر (ch != EOF)
		nommu_addchr(as_string, ch);
	اگر (ch == dquote_انتہا) { /* may be only '"' or EOF */
		debug_printf_parse("encode_string return 1 (ok)\n");
		واپس 1;
	}
	/* note: can't move it above ch == dquote_end check! */
	اگر (ch == EOF) {
		syntax_error_unterm_ch('"');
		واپس 0; /* error */
	}
	اگلا = '\0';
	اگر (ch != '\n') {
		اگلا = i_peek(داخلہ);
	}
	debug_printf_parse("\" ch=%c (%d) escape=%d\n",
			ch, ch, !!(dest->o_expflags & EXP_FLAG_ESC_GLOB_CHARS));
	اگر (ch == '\\') {
		اگر (اگلا == EOF) {
			/* Testcase: in interactive shell a file with
			 *  echo "unterminated string\<eof>
			 * is sourced.
			 */
			syntax_error_unterm_ch('"');
			واپس 0; /* error */
		}
		/* bash:
		 * "The backslash retains its special meaning [in "..."]
		 * only when followed by one of the following characters:
		 * $, `, ", \, or <newline>.  A double quote may be quoted
		 * within double quotes by preceding it with a backslash."
		 * NB: in (unquoted) heredoc, above does not apply to ",
		 * therefore we check for it by "next == dquote_end" cond.
		 */
		اگر (اگلا == dquote_انتہا || strchr("$`\\\n", اگلا)) {
			ch = i_getch(داخلہ); /* eat next */
			اگر (ch == '\n')
				جاؤ again; /* skip \<newline> */
		} /* else: ch remains == '\\', and we double it below: */
		o_addqchr(dest, ch); /* \c if c is a glob char, else just c */
		nommu_addchr(as_string, ch);
		جاؤ again;
	}
	اگر (ch == '$') {
		//if (parse_dollar_squote(as_string, dest, input))
		//	goto again;
		اگر (!parse_dollar(as_string, dest, داخلہ, /*quote_mask:*/ 0x80)) {
			debug_printf_parse("encode_string return 0: "
					"parse_dollar returned 0 (error)\n");
			واپس 0;
		}
		جاؤ again;
	}
#اگر ENABLE_HUSH_TICK
	اگر (ch == '`') {
		//unsigned pos = dest->length;
		o_addchr(dest, SPECIAL_VAR_علامت);
		o_addchr(dest, 0x80 | '`');
		اگر (!جوڑو_till_backquote(dest, داخلہ, /*in_dquote:*/ dquote_انتہا == '"'))
			واپس 0; /* error */
		o_addchr(dest, SPECIAL_VAR_علامت);
		//debug_printf_subst("SUBST RES3 '%s'\n", dest->data + pos);
		جاؤ again;
	}
#اگر_ختم
	o_جوڑو_Qchr(dest, ch);
	اگر (ch == SPECIAL_VAR_علامت) {
		/* Convert "^C" to corresponding special variable reference */
		o_addchr(dest, SPECIAL_VAR_QUOTED_SVS);
		o_addchr(dest, SPECIAL_VAR_علامت);
	}
	جاؤ again;
#تعریف_منسوخ as_string
}

/*
 * Scan input until EOF or end_trigger char.
 * Return a list of pipes to execute, or NULL on EOF
 * or if end_trigger character is met.
 * On syntax error, exit if shell is not interactive,
 * reset parsing machinery and start parsing anew,
 * or return ERR_PTR.
 */
ساکن ساخت pipe *parse_دھارا(حرف **pstring,
		صحیح *heredoc_cnt_ptr,
		ساخت اندر_str *داخلہ,
		صحیح انتہا_trigger)
{
	ساخت parse_context ctx;
	صحیح heredoc_cnt;

	/* Single-quote triggers a bypass of the main loop until its mate is
	 * found.  When recursing, quote state is passed in via ctx.word.o_expflags.
	 */
	debug_printf_parse("parse_stream entered, end_trigger='%c'\n",
			انتہا_trigger ? انتہا_trigger : 'X');
	debug_enter();

	initialize_context(&ctx);

	/* If very first arg is "" or '', ctx.word.data may end up NULL.
	 * Preventing this:
	 */
	ctx.لفظ.ڈیٹا = xzalloc(1); /* start as "", not as NULL */

	/* We used to separate words on $IFS here. This was wrong.
	 * $IFS is used only for word splitting when $var is expanded,
	 * here we should use blank chars as separators, not $IFS
	 */

	heredoc_cnt = 0;
	جب (1) {
		ثابت حرف *is_blank;
		ثابت حرف *is_special;
		صحیح ch;
		صحیح اگلا;
#اگر_غیر_تعریف_شدہ __U_BOOT__
		صحیح redir_fd;
		redir_type redir_style;
#اگر_ختم /* !__U_BOOT__ */

		ch = i_getch(داخلہ);
		debug_printf_parse(": ch=%c (%d) escape=%d\n",
				ch, ch, !!(ctx.لفظ.o_expflags & EXP_FLAG_ESC_GLOB_CHARS));
		اگر (ch == EOF) {
			ساخت pipe *pi;

			اگر (heredoc_cnt) {
				syntax_error_unterm_str("here document");
				جاؤ parse_error_exitcode1;
			}
			اگر (انتہا_trigger == ')') {
				syntax_error_unterm_ch('(');
				جاؤ parse_error_exitcode1;
			}
			اگر (انتہا_trigger == '}') {
				syntax_error_unterm_ch('{');
				جاؤ parse_error_exitcode1;
			}

			اگر (مکمل_لفظ(&ctx)) {
				جاؤ parse_error_exitcode1;
			}
			o_free_and_سیٹ_NULL(&ctx.لفظ);
			مکمل_pipe(&ctx, PIPE_SEQ);

			/* Do we sit inside of any if's, loops or case's? */
			اگر (HAS_KEYWORDS
			IF_HAS_KEYWORDS(&& (ctx.ctx_res_w != RES_NONE || ctx.old_flag != 0))
			) {
				syntax_error_unterm_str("compound statement");
				جاؤ parse_error_exitcode1;
			}

			pi = ctx.list_سر;
			/* If we got nothing... */
			/* (this makes bare "&" cmd a no-op.
			 * bash says: "syntax error near unexpected token '&'") */
			اگر (pi->عدد_cmds == 0
			IF_HAS_KEYWORDS(&& pi->res_لفظ == RES_NONE)
			) {
				free_pipe_list(pi);
				pi = تہی;
			}
#اگر !BB_MMU
			debug_printf_parse("as_string1 '%s'\n", ctx.as_string.ڈیٹا);
			اگر (pstring)
				*pstring = ctx.as_string.ڈیٹا;
			ورنہ
				o_free(&ctx.as_string);
#اگر_ختم
			// heredoc_cnt must be 0 here anyway
			//if (heredoc_cnt_ptr)
			//	*heredoc_cnt_ptr = heredoc_cnt;
			debug_leave();
			debug_printf_heredoc("parse_stream return heredoc_cnt:%d\n", heredoc_cnt);
			debug_printf_parse("parse_stream return %p: EOF\n", pi);
			واپس pi;
		}

		/* Handle "'" and "\" first, as they won't play nice with
		 * i_peek_and_eat_bkslash_nl() anyway:
		 *   echo z\\
		 * and
		 *   echo '\
		 *   '
		 * would break.
		 */
		اگر (ch == '\\') {
			ch = i_getch(داخلہ);
			اگر (ch == '\n')
				جاری; /* drop \<newline>, get next char */
			nommu_addchr(&ctx.as_string, '\\');
			اگر (ch == SPECIAL_VAR_علامت) {
				nommu_addchr(&ctx.as_string, ch);
				/* Convert \^C to corresponding special variable reference */
				جاؤ case_SPECIAL_VAR_علامت;
			}
			o_addchr(&ctx.لفظ, '\\');
			اگر (ch == EOF) {
				/* Testcase: eval 'echo Ok\' */
				/* bash-4.3.43 was removing backslash,
				 * but 4.4.19 retains it, most other shells too
				 */
				جاری; /* get next char */
			}
			/* Example: echo Hello \2>file
			 * we need to know that word 2 is quoted
			 */
			ctx.لفظ.has_quoted_part = 1;
			nommu_addchr(&ctx.as_string, ch);
			o_addchr(&ctx.لفظ, ch);
			جاری; /* get next char */
		}
		nommu_addchr(&ctx.as_string, ch);
		اگر (ch == '\'') {
			ctx.لفظ.has_quoted_part = 1;
			اگلا = i_getch(داخلہ);
#اگر_غیر_تعریف_شدہ __U_BOOT__
			اگر (اگلا == '\'' && !ctx.pending_redirect)
				جاؤ داخل_کرو_خالی_خانے_quoted_str_marker;
#اگر_ختم /* !__U_BOOT__ */

			ch = اگلا;
			جب (1) {
				اگر (ch == EOF) {
					syntax_error_unterm_ch('\'');
					جاؤ parse_error_exitcode1;
				}
				nommu_addchr(&ctx.as_string, ch);
				اگر (ch == '\'')
					توڑو;
				اگر (ch == SPECIAL_VAR_علامت) {
					/* Convert raw ^C to corresponding special variable reference */
					o_addchr(&ctx.لفظ, SPECIAL_VAR_علامت);
					o_addchr(&ctx.لفظ, SPECIAL_VAR_QUOTED_SVS);
				}
				o_addqchr(&ctx.لفظ, ch);
				ch = i_getch(داخلہ);
			}
			جاری; /* get next char */
		}

		اگلا = '\0';
		اگر (ch != '\n')
			اگلا = i_peek_and_eat_bkslash_nl(داخلہ);

		is_special = "{}<>&|();#" /* special outside of "str" */
#اگر_غیر_تعریف_شدہ __U_BOOT__
				"$\"" IF_HUSH_TICK("`") /* always special */
#ورنہ /* __U_BOOT__ */
				"$\""
#اگر_ختم /* __U_BOOT__ */
				SPECIAL_VAR_علامت_STR;
#اگر defined(CMD_TEST2_SINGLEWORD_NOGLOB)
		اگر (ctx.command->cmd_type == CMD_TEST2_SINGLEWORD_NOGLOB) {
			/* In [[ ]], {}<>&|() are not special */
			is_special += 8;
		} ورنہ
#اگر_ختم
		/* Are { and } special here? */
		اگر (ctx.command->argv /* word [word]{... - non-special */
		 || ctx.لفظ.لمبائی       /* word{... - non-special */
		 || ctx.لفظ.has_quoted_part     /* ""{... - non-special */
		 || (اگلا != ';'             /* }; - special */
		    && اگلا != ')'           /* }) - special */
		    && اگلا != '('           /* {( - special */
		    && اگلا != '&'           /* }& and }&& ... - special */
		    && اگلا != '|'           /* }|| ... - special */
		    && !strchr(defifs, اگلا) /* {word - non-special */
		    )
		) {
			/* They are not special, skip "{}" */
			is_special += 2;
		}
		is_special = strchr(is_special, ch);
		is_blank = strchr(defifs, ch);

		اگر (!is_special && !is_blank) { /* ordinary char */
 ordinary_char:
			o_جوڑو_Qchr(&ctx.لفظ, ch);
			اگر ((ctx.is_assignment == MAYBE_ASSIGNMENT
			    || ctx.is_assignment == لفظ_IS_KEYWORD)
			 && ch == '='
			 && endofname(ctx.لفظ.ڈیٹا)[0] == '='
			) {
				ctx.is_assignment = DEFINITELY_ASSIGNMENT;
				debug_printf_parse("ctx.is_assignment='%s'\n", assignment_flag[ctx.is_assignment]);
			}
			جاری;
		}

		اگر (is_blank) {
#اگر ENABLE_HUSH_LINENO_VAR
/* Case:
 * "while ...; do<whitespace><newline>
 *	cmd ..."
 * would think that "cmd" starts in <whitespace> -
 * i.e., at the previous line.
 * We need to skip all whitespace before newlines.
 */
			جب (ch != '\n') {
				اگلا = i_peek(داخلہ);
				اگر (اگلا != ' ' && اگلا != '\t' && اگلا != '\n')
					توڑو; /* next char is not ws */
				ch = i_getch(داخلہ);
			}
			/* ch == last eaten whitespace char */
#اگر_ختم
			اگر (مکمل_لفظ(&ctx)) {
				جاؤ parse_error_exitcode1;
			}
			اگر (ch == '\n') {
				/* Is this a case when newline is simply ignored?
				 * Some examples:
				 * "cmd | <newline> cmd ..."
				 * "case ... in <newline> word) ..."
				 */
				اگر (IS_NULL_CMD(ctx.command)
				 && ctx.لفظ.لمبائی == 0
				 && !ctx.لفظ.has_quoted_part
				 && heredoc_cnt == 0
				) {
					/* This newline can be ignored. But...
					 * Without check #1, interactive shell
					 * ignores even bare <newline>,
					 * and shows the continuation prompt:
					 * ps1_prompt$ <enter>
					 * ps2> _   <=== wrong, should be ps1
					 * Without check #2, "cmd & <newline>"
					 * is similarly mistreated.
					 * (BTW, this makes "cmd & cmd"
					 * and "cmd && cmd" non-orthogonal.
					 * Really, ask yourself, why
					 * "cmd && <newline>" doesn't start
					 * cmd but waits for more input?
					 * The only reason is that it might be
					 * a "cmd1 && <nl> cmd2 &" construct,
					 * cmd1 may need to run in BG).
					 */
					ساخت pipe *pi = ctx.list_سر;
					اگر (pi->عدد_cmds != 0       /* check #1 */
					 && pi->followup != PIPE_BG /* check #2 */
					) {
						جاری;
					}
				}
				/* Treat newline as a command separator. */
				مکمل_pipe(&ctx, PIPE_SEQ);
				debug_printf_heredoc("heredoc_cnt:%d\n", heredoc_cnt);
				اگر (heredoc_cnt) {
					heredoc_cnt = fetch_heredocs(&ctx.as_string, ctx.list_سر, heredoc_cnt, داخلہ);
					اگر (heredoc_cnt != 0)
						جاؤ parse_error_exitcode1;
				}
				ctx.is_assignment = MAYBE_ASSIGNMENT;
				debug_printf_parse("ctx.is_assignment='%s'\n", assignment_flag[ctx.is_assignment]);
				ch = ';';
				/* note: if (is_blank) continue;
				 * will still trigger for us */
			}
		}

		/* "cmd}" or "cmd }..." without semicolon or &:
		 * } is an ordinary char in this case, even inside { cmd; }
		 * Pathological example: { ""}; } should exec "}" cmd
		 */
#اگر_غیر_تعریف_شدہ __U_BOOT__
		اگر (ch == '}') {
#ورنہ /* __U_BOOT__ */
		اگر (ch == '}' || ch == ')') {
#اگر_ختم /* __U_BOOT__ */
			اگر (ctx.لفظ.لمبائی != 0 /* word} */
			 || ctx.لفظ.has_quoted_part    /* ""} */
			) {
				جاؤ ordinary_char;
			}
			اگر (!IS_NULL_CMD(ctx.command)) { /* cmd } */
				/* Generally, there should be semicolon: "cmd; }"
				 * However, bash allows to omit it if "cmd" is
				 * a group. Examples:
				 * { { echo 1; } }
				 * {(echo 1)}
				 * { echo 0 >&2 | { echo 1; } }
				 * { while false; do :; done }
				 * { case a in b) ;; esac }
				 */
				اگر (ctx.command->گروہ)
					جاؤ رکن_گروہ;
				جاؤ ordinary_char;
			}
			اگر (!IS_NULL_PIPE(ctx.pipe)) /* cmd | } */
				/* Can't be an end of {cmd}, skip the check */
				جاؤ skip_انتہا_trigger;
			/* else: } does terminate a group */
		}
 رکن_گروہ:
		اگر (انتہا_trigger && انتہا_trigger == ch
		 && (ch != ';' || heredoc_cnt == 0)
#اگر ENABLE_HUSH_CASE
		 && (ch != ')'
		    || ctx.ctx_res_w != RES_مطابقت
		    || (!ctx.لفظ.has_quoted_part && strcmp(ctx.لفظ.ڈیٹا, "esac") == 0)
		    )
#اگر_ختم
		) {
			اگر (مکمل_لفظ(&ctx)) {
				جاؤ parse_error_exitcode1;
			}
			مکمل_pipe(&ctx, PIPE_SEQ);
			ctx.is_assignment = MAYBE_ASSIGNMENT;
			debug_printf_parse("ctx.is_assignment='%s'\n", assignment_flag[ctx.is_assignment]);
			/* Do we sit outside of any if's, loops or case's? */
			اگر (!HAS_KEYWORDS
			IF_HAS_KEYWORDS(|| (ctx.ctx_res_w == RES_NONE && ctx.old_flag == 0))
			) {
				o_free_and_سیٹ_NULL(&ctx.لفظ);
#اگر !BB_MMU
				debug_printf_parse("as_string2 '%s'\n", ctx.as_string.ڈیٹا);
				اگر (pstring)
					*pstring = ctx.as_string.ڈیٹا;
				ورنہ
					o_free(&ctx.as_string);
#اگر_ختم
				اگر (ch != ';' && IS_NULL_PIPE(ctx.list_سر)) {
					/* Example: bare "{ }", "()" */
					G.آخری_exitcode = 2; /* bash compat */
					syntax_error_unexpected_ch(ch);
					جاؤ parse_error;
				}
				اگر (heredoc_cnt_ptr)
					*heredoc_cnt_ptr = heredoc_cnt;
				debug_printf_heredoc("parse_stream return heredoc_cnt:%d\n", heredoc_cnt);
				debug_printf_parse("parse_stream return %p: "
						"end_trigger char found\n",
						ctx.list_سر);
				debug_leave();
				واپس ctx.list_سر;
			}
		}

		اگر (is_blank)
			جاری;

		/* Catch <, > before deciding whether this word is
		 * an assignment. a=1 2>z b=2: b=2 is still assignment */
		انتخاب (ch) {
#اگر_غیر_تعریف_شدہ __U_BOOT__
		حالت '>':
			redir_fd = redirect_opt_عدد(&ctx.لفظ);
			اگر (مکمل_لفظ(&ctx)) {
				جاؤ parse_error_exitcode1;
			}
			redir_style = REDIRECT_OVERWRITE;
			اگر (اگلا == '>') {
				redir_style = REDIRECT_APPEND;
				ch = i_getch(داخلہ);
				nommu_addchr(&ctx.as_string, ch);
			}
#اگر 0
			ورنہ اگر (اگلا == '(') {
				syntax_error(">(process) not supported");
				جاؤ parse_error_exitcode1;
			}
#اگر_ختم
			اگر (parse_redirect(&ctx, redir_fd, redir_style, داخلہ))
				جاؤ parse_error_exitcode1;
			جاری; /* get next char */
		حالت '<':
			redir_fd = redirect_opt_عدد(&ctx.لفظ);
			اگر (مکمل_لفظ(&ctx)) {
				جاؤ parse_error_exitcode1;
			}
			redir_style = REDIRECT_داخلہ;
			اگر (اگلا == '<') {
				redir_style = REDIRECT_HEREDOC;
				heredoc_cnt++;
				debug_printf_heredoc("++heredoc_cnt=%d\n", heredoc_cnt);
				ch = i_getch(داخلہ);
				nommu_addchr(&ctx.as_string, ch);
			} ورنہ اگر (اگلا == '>') {
				redir_style = REDIRECT_IO;
				ch = i_getch(داخلہ);
				nommu_addchr(&ctx.as_string, ch);
			}
#اگر 0
			ورنہ اگر (اگلا == '(') {
				syntax_error("<(process) not supported");
				جاؤ parse_error_exitcode1;
			}
#اگر_ختم
			اگر (parse_redirect(&ctx, redir_fd, redir_style, داخلہ))
				جاؤ parse_error_exitcode1;
			جاری; /* get next char */
#ورنہ /* __U_BOOT__ */
			/*
			 * In U-Boot, '<' and '>' can be used in test command to test if
			 * a string is, alphabetically, before or after another.
			 * In 2021 Busybox hush, we will keep the same behavior and so not treat
			 * them as redirection operator.
			 *
			 * Indeed, in U-Boot, tests are handled by the test command and not by the
			 * shell code.
			 * So, better to give this character as input to test command.
			 *
			 * NOTE In my opinion, when you use '<' or '>' I am almost sure
			 * you wanted to use "-gt" or "-lt" in place, so thinking to
			 * escape these will make you should check your code (sh syntax
			 * at this level is, for me, error prone).
			 */
			حالت '>':
				fallthrough;
			حالت '<':
				o_جوڑو_Qchr(&ctx.لفظ, ch);
				جاری;
#اگر_ختم /* __U_BOOT__ */
		حالت '#':
			اگر (ctx.لفظ.لمبائی == 0 && !ctx.لفظ.has_quoted_part) {
				/* skip "#comment" */
				/* note: we do not add it to &ctx.as_string */
/* TODO: in bash:
 * comment inside $() goes to the next \n, even inside quoted string (!):
 * cmd "$(cmd2 #comment)" - syntax error
 * cmd "`cmd2 #comment`" - ok
 * We accept both (comment ends where command subst ends, in both cases).
 */
				جب (1) {
					ch = i_peek(داخلہ);
					اگر (ch == '\n') {
						nommu_addchr(&ctx.as_string, '\n');
						توڑو;
					}
					ch = i_getch(داخلہ);
					اگر (ch == EOF)
						توڑو;
				}
				جاری; /* get next char */
			}
			توڑو;
		}
 skip_انتہا_trigger:

		اگر (ctx.is_assignment == MAYBE_ASSIGNMENT
#اگر_غیر_تعریف_شدہ __U_BOOT__
		 /* check that we are not in word in "a=1 2>word b=1": */
		 && !ctx.pending_redirect
#اگر_ختم /* !__U_BOOT__ */
		) {
			/* ch is a special char and thus this word
			 * cannot be an assignment */
			ctx.is_assignment = NOT_ASSIGNMENT;
			debug_printf_parse("ctx.is_assignment='%s'\n", assignment_flag[ctx.is_assignment]);
		}

		/* Note: nommu_addchr(&ctx.as_string, ch) is already done */

		انتخاب (ch) {
		case_SPECIAL_VAR_علامت:
		حالت SPECIAL_VAR_علامت:
			/* Convert raw ^C to corresponding special variable reference */
			o_addchr(&ctx.لفظ, SPECIAL_VAR_علامت);
			o_addchr(&ctx.لفظ, SPECIAL_VAR_QUOTED_SVS);
			/* fall through */
		حالت '#':
			/* non-comment #: "echo a#b" etc */
			o_addchr(&ctx.لفظ, ch);
			جاری; /* get next char */
		حالت '$':
#اگر_غیر_تعریف_شدہ __U_BOOT__
			اگر (parse_dollar_squote(&ctx.as_string, &ctx.لفظ, داخلہ))
				جاری; /* get next char */
#اگر_ختم /* !__U_BOOT__ */
			اگر (!parse_dollar(&ctx.as_string, &ctx.لفظ, داخلہ, /*quote_mask:*/ 0)) {
				debug_printf_parse("parse_stream parse error: "
					"parse_dollar returned 0 (error)\n");
				جاؤ parse_error_exitcode1;
			}
			جاری; /* get next char */
		حالت '"':
			ctx.لفظ.has_quoted_part = 1;
#اگر_غیر_تعریف_شدہ __U_BOOT__
			اگر (اگلا == '"' && !ctx.pending_redirect) {
#ورنہ /* __U_BOOT__ */
			اگر (اگلا == '"') {
#اگر_ختم /* __U_BOOT__ */
				i_getch(داخلہ); /* eat second " */
#اگر_غیر_تعریف_شدہ __U_BOOT__
 داخل_کرو_خالی_خانے_quoted_str_marker:
#اگر_ختم /* !__U_BOOT__ */
				nommu_addchr(&ctx.as_string, اگلا);
				o_addchr(&ctx.لفظ, SPECIAL_VAR_علامت);
				o_addchr(&ctx.لفظ, SPECIAL_VAR_علامت);
				جاری; /* get next char */
			}
			اگر (ctx.is_assignment == NOT_ASSIGNMENT)
				ctx.لفظ.o_expflags |= EXP_FLAG_ESC_GLOB_CHARS;
			اگر (!encode_string(&ctx.as_string, &ctx.لفظ, داخلہ, '"'))
				جاؤ parse_error_exitcode1;
			ctx.لفظ.o_expflags &= ~EXP_FLAG_ESC_GLOB_CHARS;
			جاری; /* get next char */
#اگر ENABLE_HUSH_TICK
		حالت '`': {
			USE_FOR_NOMMU(غیرعلامتی pos;)

			o_addchr(&ctx.لفظ, SPECIAL_VAR_علامت);
			o_addchr(&ctx.لفظ, '`');
			USE_FOR_NOMMU(pos = ctx.لفظ.لمبائی;)
			اگر (!جوڑو_till_backquote(&ctx.لفظ, داخلہ, /*in_dquote:*/ 0))
				جاؤ parse_error_exitcode1;
# اگر !BB_MMU
			o_addstr(&ctx.as_string, ctx.لفظ.ڈیٹا + pos);
			o_addchr(&ctx.as_string, '`');
# endif
			o_addchr(&ctx.لفظ, SPECIAL_VAR_علامت);
			//debug_printf_subst("SUBST RES3 '%s'\n", ctx.word.data + pos);
			جاری; /* get next char */
		}
#اگر_ختم
		حالت ';':
#اگر ENABLE_HUSH_CASE
 case_semi:
#اگر_ختم
			اگر (مکمل_لفظ(&ctx)) {
				جاؤ parse_error_exitcode1;
			}
			مکمل_pipe(&ctx, PIPE_SEQ);
#اگر ENABLE_HUSH_CASE
			/* Eat multiple semicolons, detect
			 * whether it means something special */
			جب (1) {
				ch = i_peek_and_eat_bkslash_nl(داخلہ);
				اگر (ch != ';')
					توڑو;
				ch = i_getch(داخلہ);
				nommu_addchr(&ctx.as_string, ch);
				اگر (ctx.ctx_res_w == RES_CASE_BODY) {
					ctx.ctx_dsemicolon = 1;
					ctx.ctx_res_w = RES_مطابقت;
					توڑو;
				}
			}
#اگر_ختم
 new_cmd:
			/* We just finished a cmd. New one may start
			 * with an assignment */
			ctx.is_assignment = MAYBE_ASSIGNMENT;
			debug_printf_parse("ctx.is_assignment='%s'\n", assignment_flag[ctx.is_assignment]);
			جاری; /* get next char */
		حالت '&':
			اگر (مکمل_لفظ(&ctx)) {
				جاؤ parse_error_exitcode1;
			}
			اگر (اگلا == '&') {
				ch = i_getch(داخلہ);
				nommu_addchr(&ctx.as_string, ch);
				مکمل_pipe(&ctx, PIPE_AND);
			} ورنہ {
				مکمل_pipe(&ctx, PIPE_BG);
			}
			جاؤ new_cmd;
		حالت '|':
			اگر (مکمل_لفظ(&ctx)) {
				جاؤ parse_error_exitcode1;
			}
#اگر ENABLE_HUSH_CASE
			اگر (ctx.ctx_res_w == RES_مطابقت)
				توڑو; /* we are in case's "word | word)" */
#اگر_ختم
			اگر (اگلا == '|') { /* || */
				ch = i_getch(داخلہ);
				nommu_addchr(&ctx.as_string, ch);
				مکمل_pipe(&ctx, PIPE_OR);
			} ورنہ {
				/* we could pick up a file descriptor choice here
				 * with redirect_opt_num(), but bash doesn't do it.
				 * "echo foo 2| cat" yields "foo 2". */
				مکمل_command(&ctx);
			}
			جاؤ new_cmd;
		حالت '(':
#اگر ENABLE_HUSH_CASE
			/* "case... in [(]word)..." - skip '(' */
			اگر (ctx.ctx_res_w == RES_مطابقت
			 && ctx.command->argv == تہی /* not (word|(... */
			 && ctx.لفظ.لمبائی == 0 /* not word(... */
			 && ctx.لفظ.has_quoted_part == 0 /* not ""(... */
			) {
				جاری; /* get next char */
			}
#اگر_ختم
			/* fall through */
		حالت '{': {
			صحیح n = parse_گروہ(&ctx, داخلہ, ch);
			اگر (n < 0) {
				جاؤ parse_error_exitcode1;
			}
			debug_printf_heredoc("parse_group done, needs heredocs:%d\n", n);
			heredoc_cnt += n;
			جاؤ new_cmd;
		}
		حالت ')':
#اگر ENABLE_HUSH_CASE
			اگر (ctx.ctx_res_w == RES_مطابقت)
				جاؤ case_semi;
#اگر_ختم
		حالت '}':
			/* proper use of this character is caught by end_trigger:
			 * if we see {, we call parse_group(..., end_trigger='}')
			 * and it will match } earlier (not here). */
			G.آخری_exitcode = 2;
			syntax_error_unexpected_ch(ch);
			جاؤ parse_error;
		طےشدہ:
			اگر (HUSH_DEBUG)
				bb_error_پیغام_and_die("BUG: unexpected %c", ch);
		}
	} /* while (1) */

 parse_error_exitcode1:
	G.آخری_exitcode = 1;
 parse_error:
	{
		ساخت parse_context *pctx;
		IF_HAS_KEYWORDS(ساخت parse_context *p2;)

		/* Clean up allocated tree.
		 * Sample for finding leaks on syntax error recovery path.
		 * Run it from interactive shell, watch pmap `pidof hush`.
		 * while if false; then false; fi; do break; fi
		 * Samples to catch leaks at execution:
		 * while if (true | { true;}); then echo ok; fi; do break; done
		 * while if (true | { true;}); then echo ok; fi; do (if echo ok; break; then :; fi) | cat; break; done
		 */
		pctx = &ctx;
		کرو {
			/* Update pipe/command counts,
			 * otherwise freeing may miss some */
			مکمل_pipe(pctx, PIPE_SEQ);
			debug_printf_clean("freeing list %p from ctx %p\n",
					pctx->list_سر, pctx);
			debug_print_درخت(pctx->list_سر, 0);
			free_pipe_list(pctx->list_سر);
			debug_printf_clean("freed list %p\n", pctx->list_سر);
#اگر !BB_MMU
			o_free(&pctx->as_string);
#اگر_ختم
			IF_HAS_KEYWORDS(p2 = pctx->ڈھیر;)
			اگر (pctx != &ctx) {
				آزاد(pctx);
			}
			IF_HAS_KEYWORDS(pctx = p2;)
		} جب (HAS_KEYWORDS && pctx);

		o_free(&ctx.لفظ);
#اگر !BB_MMU
		اگر (pstring)
			*pstring = تہی;
#اگر_ختم
		debug_leave();
		واپس ERR_PTR;
	}
}

/*** Execution routines ***/

/* Expansion can recurse, need forward decls: */
#اگر !BASH_نمونہ_SUBST && !ENABLE_HUSH_CASE
#تعریف_کرو expand_string_مقصد_string(str, EXP_flags, do_unbackslash) \
	expand_string_مقصد_string(str)
#اگر_ختم
ساکن حرف *expand_string_مقصد_string(ثابت حرف *str, صحیح EXP_flags, صحیح do_unbackslash);
#اگر ENABLE_HUSH_TICK
ساکن صحیح process_command_subs(o_string *dest, ثابت حرف *s);
#اگر_ختم
ساکن صحیح expand_vars_مقصد_list(o_string *output, صحیح n, حرف *arg);

/* expand_strvec_to_strvec() takes a list of strings, expands
 * all variable references within and returns a pointer to
 * a list of expanded strings, possibly with larger number
 * of strings. (Think VAR="a b"; echo $VAR).
 * This new list is allocated as a single malloc block.
 * NULL-terminated list of char* pointers is at the beginning of it,
 * followed by strings themselves.
 * Caller can deallocate entire list by single free(list). */

/* A horde of its helpers come first: */

ساکن خالی o_addblock_duplicate_backslash(o_string *o, ثابت حرف *str, صحیح len)
{
	جب (--len >= 0) {
		حرف c = *str++;

#اگر ENABLE_HUSH_BRACE_EXPANSION
		اگر (c == '{' || c == '}') {
			/* { -> \{, } -> \} */
			o_addchr(o, '\\');
			/* And now we want to add { or } and continue:
			 *  o_addchr(o, c);
			 *  continue;
			 * luckily, just falling through achieves this.
			 */
		}
#اگر_ختم
		o_addchr(o, c);
		اگر (c == '\\') {
			/* \z -> \\\z; \<eol> -> \\<eol> */
			o_addchr(o, '\\');
			اگر (len) {
				len--;
				o_addchr(o, '\\');
				o_addchr(o, *str++);
			}
		}
	}
}

/* Store given string, finalizing the word and starting new one whenever
 * we encounter IFS char(s). This is used for expanding variable values.
 * End-of-string does NOT finalize word: think about 'echo -$VAR-'.
 * Return in output->ended_in_ifs:
 * 1 - ended with IFS char, else 0 (this includes case of empty str).
 */
ساکن صحیح expand_on_ifs(o_string *output, صحیح n, ثابت حرف *str)
{
	صحیح آخری_is_ifs = 0;

	جب (1) {
		صحیح لفظ_len;

		اگر (!*str)  /* EOL - do not finalize word */
			توڑو;
		لفظ_len = strcspn(str, G.ifs);
		اگر (لفظ_len) {
			/* We have WORD_LEN leading non-IFS chars */
			اگر (!(output->o_expflags & EXP_FLAG_GLOB)) {
				o_addblock(output, str, لفظ_len);
			} ورنہ {
				/* Protect backslashes against globbing up :)
				 * Example: "v='\*'; echo b$v" prints "b\*"
				 * (and does not try to glob on "*")
				 */
				o_addblock_duplicate_backslash(output, str, لفظ_len);
				/*/ Why can't we do it easier? */
				/*o_addblock(output, str, word_len); - WRONG: "v='\*'; echo Z$v" prints "Z*" instead of "Z\*" */
				/*o_addqblock(output, str, word_len); - WRONG: "v='*'; echo Z$v" prints "Z*" instead of Z* files */
			}
			آخری_is_ifs = 0;
			str += لفظ_len;
			اگر (!*str)  /* EOL - do not finalize word */
				توڑو;
		}

		/* We know str here points to at least one IFS char */
		آخری_is_ifs = 1;
		str += strspn(str, G.ifs_whitespace); /* skip IFS whitespace chars */
		اگر (!*str)  /* EOL - do not finalize word */
			توڑو;

		اگر (G.ifs_whitespace != G.ifs /* usually false ($IFS is usually all whitespace), */
		 && strchr(G.ifs, *str)       /* the second check would fail */
		) {
			/* This is a non-whitespace $IFS char */
			/* Skip it and IFS whitespace chars, start new word */
			str++;
			str += strspn(str, G.ifs_whitespace);
			جاؤ new_لفظ;
		}

		/* Start new word... but not always! */
		/* Case "v=' a'; echo ''$v": we do need to finalize empty word: */
		اگر (output->has_quoted_part
		/*
		 * Case "v=' a'; echo $v":
		 * here nothing precedes the space in $v expansion,
		 * therefore we should not finish the word
		 * (IOW: if there *is* word to finalize, only then do it):
		 * It's okay if this accesses the byte before first argv[]:
		 * past call to o_save_ptr() cleared it to zero byte
		 * (grep for -prev-ifs-check-).
		 */
		 || output->ڈیٹا[output->لمبائی - 1]
		) {
 new_لفظ:
			o_addchr(output, '\0');
			debug_print_list("expand_on_ifs", output, n);
			n = o_save_ptr(output, n);
		}
	}

	output->ended_اندر_ifs = آخری_is_ifs;
	debug_print_list("expand_on_ifs[1]", output, n);
	واپس n;
}

#اگر_غیر_تعریف_شدہ __U_BOOT__
/* Helper to expand $((...)) and heredoc body. These act as if
 * they are in double quotes, with the exception that they are not :).
 * Just the rules are similar: "expand only $var and `cmd`"
 *
 * Returns malloced string.
 * As an optimization, we return NULL if expansion is not needed.
 */
ساکن حرف *encode_then_expand_string(ثابت حرف *str)
{
	حرف *exp_str;
	ساخت اندر_str داخلہ;
	o_string dest = NULL_O_STRING;
	ثابت حرف *cp;

	cp = str;
	برائے (;;) {
		اگر (!*cp) واپس تہی; /* string has no special chars */
		اگر (*cp == '$') توڑو;
		اگر (*cp == '\\') توڑو;
#اگر ENABLE_HUSH_TICK
		اگر (*cp == '`') توڑو;
#اگر_ختم
		cp++;
	}

	/* We need to expand. Example:
	 * echo $(($a + `echo 1`)) $((1 + $((2)) ))
	 */
	تیاری_string_اندر_str(&داخلہ, str);
	encode_string(تہی, &dest, &داخلہ, EOF);
//TODO: error check (encode_string returns 0 on error)?
	//bb_error_msg("'%s' -> '%s'", str, dest.data);
	exp_str = expand_string_مقصد_string(dest.ڈیٹا,
			EXP_FLAG_ESC_GLOB_CHARS,
			/*unbackslash:*/ 1
	);
	//bb_error_msg("'%s' -> '%s'", dest.data, exp_str);
	o_free(&dest);
	واپس exp_str;
}

ساکن ثابت حرف *پہلا_special_char_اندر_vararg(ثابت حرف *cp)
{
	برائے (;;) {
		اگر (!*cp) واپس تہی; /* string has no special chars */
		اگر (*cp == '$') واپس cp;
		اگر (*cp == '\\') واپس cp;
		اگر (*cp == '\'') واپس cp;
		اگر (*cp == '"') واپس cp;
#اگر ENABLE_HUSH_TICK
		اگر (*cp == '`') واپس cp;
#اگر_ختم
		/* dquoted "${x:+ARG}" should not glob, therefore
		 * '*' et al require some non-literal processing: */
		اگر (*cp == '*') واپس cp;
		اگر (*cp == '?') واپس cp;
		اگر (*cp == '[') واپس cp;
		cp++;
	}
}

/* Expanding ARG in ${var#ARG}, ${var%ARG}, or ${var/ARG/ARG}.
 * These can contain single- and double-quoted strings,
 * and treated as if the ARG string is initially unquoted. IOW:
 * ${var#ARG} and "${var#ARG}" treat ARG the same (ARG can even be
 * a dquoted string: "${var#"zz"}"), the difference only comes later
 * (word splitting and globbing of the ${var...} result).
 */
#اگر !BASH_نمونہ_SUBST
#تعریف_کرو encode_then_expand_vararg(str, handle_squotes, do_unbackslash) \
	encode_then_expand_vararg(str, handle_squotes)
#اگر_ختم
ساکن حرف *encode_then_expand_vararg(ثابت حرف *str, صحیح handle_squotes, صحیح do_unbackslash)
{
#اگر !BASH_نمونہ_SUBST && ENABLE_HUSH_CASE
	ثابت صحیح do_unbackslash = 0;
#اگر_ختم
	حرف *exp_str;
	ساخت اندر_str داخلہ;
	o_string dest = NULL_O_STRING;

	اگر (!پہلا_special_char_اندر_vararg(str)) {
		/* string has no special chars */
		واپس تہی;
	}

	تیاری_string_اندر_str(&داخلہ, str);
	dest.ڈیٹا = xzalloc(1); /* start as "", not as NULL */
	exp_str = تہی;

	برائے (;;) {
		صحیح ch;

		ch = i_getch(&داخلہ);
		debug_printf_parse("%s: ch=%c (%d) escape=%d\n",
				__func__, ch, ch, !!dest.o_expflags);

		اگر (!dest.o_expflags) {
			اگر (ch == EOF)
				توڑو;
			اگر (handle_squotes && ch == '\'') {
				اگر (!جوڑو_till_single_quote_dquoted(&dest, &داخلہ))
					جاؤ ret; /* error */
				جاری;
			}
		}
		اگر (ch == EOF) {
			syntax_error_unterm_ch('"');
			جاؤ ret; /* error */
		}
		اگر (ch == '"') {
			dest.o_expflags ^= EXP_FLAG_ESC_GLOB_CHARS;
			جاری;
		}
		اگر (ch == '\\') {
			ch = i_getch(&داخلہ);
			اگر (ch == EOF) {
//example? error message?	syntax_error_unterm_ch('"');
				debug_printf_parse("%s: error: \\<eof>\n", __func__);
				جاؤ ret;
			}
			o_addqchr(&dest, ch);
			جاری;
		}
		اگر (ch == '$') {
			اگر (parse_dollar_squote(تہی, &dest, &داخلہ))
				جاری;
			اگر (!parse_dollar(تہی, &dest, &داخلہ, /*quote_mask:*/ 0x80)) {
				debug_printf_parse("%s: error: parse_dollar returned 0 (error)\n", __func__);
				جاؤ ret;
			}
			جاری;
		}
#اگر ENABLE_HUSH_TICK
		اگر (ch == '`') {
			//unsigned pos = dest->length;
			o_addchr(&dest, SPECIAL_VAR_علامت);
			o_addchr(&dest, 0x80 | '`');
			اگر (!جوڑو_till_backquote(&dest, &داخلہ,
					/*in_dquote:*/ dest.o_expflags /* nonzero if EXP_FLAG_ESC_GLOB_CHARS set */
				)
			) {
				جاؤ ret; /* error */
			}
			o_addchr(&dest, SPECIAL_VAR_علامت);
			//debug_printf_subst("SUBST RES3 '%s'\n", dest->data + pos);
			جاری;
		}
#اگر_ختم
		o_جوڑو_Qchr(&dest, ch);
	} /* for (;;) */

	debug_printf_parse("encode: '%s' -> '%s'\n", str, dest.ڈیٹا);
	exp_str = expand_string_مقصد_string(dest.ڈیٹا,
			do_unbackslash ? EXP_FLAG_ESC_GLOB_CHARS : 0,
			do_unbackslash
	);
 ret:
	debug_printf_parse("expand: '%s' -> '%s'\n", dest.ڈیٹا, exp_str);
	o_free(&dest);
	واپس exp_str;
}

/* Expanding ARG in ${var+ARG}, ${var-ARG}
 */
ساکن NOINLINE صحیح encode_then_append_var_plusminus(o_string *output, صحیح n,
		حرف *str, صحیح dquoted)
{
	ساخت اندر_str داخلہ;
	o_string dest = NULL_O_STRING;

	اگر (!پہلا_special_char_اندر_vararg(str)
	 && '\0' == str[strcspn(str, G.ifs)]
	) {
		/* string has no special chars
		 * && string has no $IFS chars
		 */
		اگر (dquoted) {
			/* Prints 1 (quoted expansion is a "" word, not nothing):
			 * set -- "${notexist-}"; echo $#
			 */
			output->has_quoted_part = 1;
		}
		واپس expand_vars_مقصد_list(output, n, str);
	}

	تیاری_string_اندر_str(&داخلہ, str);

	برائے (;;) {
		صحیح ch;

		ch = i_getch(&داخلہ);
		debug_printf_parse("%s: ch=%c (%d) escape=%x\n",
				__func__, ch, ch, dest.o_expflags);

		اگر (!dest.o_expflags) {
			اگر (ch == EOF)
				توڑو;
			اگر (!dquoted && !(output->o_expflags & EXP_FLAG_SINGLEWORD) && strchr(G.ifs, ch)) {
				/* PREFIX${x:d${e}f ...} and we met space: expand "d${e}f" and start new word.
				 * do not assume we are at the start of the word (PREFIX above).
				 */
				اگر (dest.ڈیٹا) {
					n = expand_vars_مقصد_list(output, n, dest.ڈیٹا);
					o_free_and_سیٹ_NULL(&dest);
					o_addchr(output, '\0');
					n = o_save_ptr(output, n); /* create next word */
				} ورنہ
				اگر (output->لمبائی != o_get_آخری_ptr(output, n)
				 || output->has_quoted_part
				) {
					/* For these cases:
					 * f() { for i; do echo "|$i|"; done; }; x=x
					 * f a${x:+ }b  # 1st condition
					 * |a|
					 * |b|
					 * f ""${x:+ }b  # 2nd condition
					 * ||
					 * |b|
					 */
					o_addchr(output, '\0');
					n = o_save_ptr(output, n); /* create next word */
				}
				جاری;
			}
			اگر (!dquoted && ch == '\'') {
				اگر (!جوڑو_till_single_quote_dquoted(&dest, &داخلہ))
					جاؤ ret; /* error */
				o_addchr(&dest, SPECIAL_VAR_علامت);
				o_addchr(&dest, SPECIAL_VAR_علامت);
				جاری;
			}
		}
		اگر (ch == EOF) {
			syntax_error_unterm_ch('"');
			جاؤ ret; /* error */
		}
		اگر (ch == '"') {
			dest.o_expflags ^= EXP_FLAG_ESC_GLOB_CHARS;
			اگر (dest.o_expflags) {
				o_addchr(&dest, SPECIAL_VAR_علامت);
				o_addchr(&dest, SPECIAL_VAR_علامت);
			}
			جاری;
		}
		اگر (ch == '\\') {
			ch = i_getch(&داخلہ);
			اگر (ch == EOF) {
//example? error message?	syntax_error_unterm_ch('"');
				debug_printf_parse("%s: error: \\<eof>\n", __func__);
				جاؤ ret;
			}
			o_addqchr(&dest, ch);
			جاری;
		}
		اگر (ch == '$') {
			اگر (!parse_dollar(تہی, &dest, &داخلہ, /*quote_mask:*/ (dest.o_expflags || dquoted) ? 0x80 : 0)) {
				debug_printf_parse("%s: error: parse_dollar returned 0 (error)\n", __func__);
				جاؤ ret;
			}
			جاری;
		}
#اگر ENABLE_HUSH_TICK
		اگر (ch == '`') {
			//unsigned pos = dest->length;
			o_addchr(&dest, SPECIAL_VAR_علامت);
			o_addchr(&dest, (dest.o_expflags || dquoted) ? 0x80 | '`' : '`');
			اگر (!جوڑو_till_backquote(&dest, &داخلہ,
					/*in_dquote:*/ dest.o_expflags /* nonzero if EXP_FLAG_ESC_GLOB_CHARS set */
				)
			) {
				جاؤ ret; /* error */
			}
			o_addchr(&dest, SPECIAL_VAR_علامت);
			//debug_printf_subst("SUBST RES3 '%s'\n", dest->data + pos);
			جاری;
		}
#اگر_ختم
		اگر (dquoted) {
			/* Always glob-protect if in dquotes:
			 * x=x; echo "${x:+/bin/c*}" - prints: /bin/c*
			 * x=x; echo "${x:+"/bin/c*"}" - prints: /bin/c*
			 */
			o_addqchr(&dest, ch);
		} ورنہ {
			/* Glob-protect only if char is quoted:
			 * x=x; echo ${x:+/bin/c*} - prints many filenames
			 * x=x; echo ${x:+"/bin/c*"} - prints: /bin/c*
			 */
			o_جوڑو_Qchr(&dest, ch);
		}
	} /* for (;;) */

	اگر (dest.ڈیٹا) {
		n = expand_vars_مقصد_list(output, n, dest.ڈیٹا);
	}
 ret:
	o_free(&dest);
	واپس n;
}
#اگر_ختم /* !__U_BOOT__ */

#اگر_غیر_تعریف_شدہ __U_BOOT__
#اگر ENABLE_FEATURE_SH_MATH
ساکن arith_t expand_and_evaluate_arith(ثابت حرف *arg, ثابت حرف **errmsg_p)
{
	arith_state_t math_state;
	arith_t res;
	حرف *exp_str;

	math_state.lookupvar = get_local_var_قدر;
	math_state.setvar = سیٹ_local_var_from_halves;
	//math_state.endofname = endofname;
	exp_str = encode_then_expand_string(arg);
	res = arith(&math_state, exp_str ? exp_str : arg);
	آزاد(exp_str);
	اگر (errmsg_p)
		*errmsg_p = math_state.errmsg;
	اگر (math_state.errmsg)
		پیغام_and_die_if_script(math_state.errmsg);
	واپس res;
}
#اگر_ختم
#اگر_ختم /* !__U_BOOT__ */

#اگر_غیر_تعریف_شدہ __U_BOOT__
#اگر BASH_نمونہ_SUBST
/* ${var/[/]pattern[/repl]} helpers */
ساکن حرف *strstr_نمونہ(حرف *val, ثابت حرف *نمونہ, صحیح *سائز)
{
	صحیح پہلا_escaped = (نمونہ[0] == '\\' && نمونہ[1]);
	/* "first_escaped" trick allows to treat e.g. "\*no_glob_chars"
	 * as literal too (as it is semi-common, and easy to accomodate
	 * by just using str + 1).
	 */
	صحیح sz = strcspn(نمونہ + پہلا_escaped * 2, "*?[\\");
	اگر ((نمونہ + پہلا_escaped * 2)[sz] == '\0') {
		/* Optimization for trivial patterns.
		 * Testcase for very slow replace (performs about 22k replaces):
		 * x=::::::::::::::::::::::
		 * x=$x$x;x=$x$x;x=$x$x;x=$x$x;x=$x$x;x=$x$x;x=$x$x;x=$x$x;x=$x$x;x=$x$x;echo ${#x}
		 * echo "${x//:/|}"
		 */
		*سائز = sz + پہلا_escaped;
		واپس لڑی_تلاش(val, نمونہ + پہلا_escaped);
	}

	جب (1) {
		حرف *انتہا_2 = جائزہ_and_مطابقت(val, نمونہ, جائزہ_MOVE_FROM_دایاں + جائزہ_مطابقت_بایاں_HALF);
		debug_printf_varexp("val:'%s' pattern:'%s' end:'%s'\n", val, نمونہ, انتہا_2);
		اگر (انتہا_2) {
			*سائز = انتہا_2 - val;
			واپس val;
		}
		اگر (*val == '\0')
			واپس تہی;
		/* Optimization: if "*pat" did not match the start of "string",
		 * we know that "tring", "ring" etc will not match too:
		 */
		اگر (نمونہ[0] == '*')
			واپس تہی;
		val++;
	}
}
ساکن حرف *replace_نمونہ(حرف *val, ثابت حرف *نمونہ, ثابت حرف *repl, حرف exp_op)
{
	حرف *نتیجہ = تہی;
	غیرعلامتی res_len = 0;
	غیرعلامتی repl_len = لڑی_لمبائی(repl);

	/* Null pattern never matches, including if "var" is empty */
	اگر (!نمونہ[0])
		واپس نتیجہ; /* NULL, no replaces happened */

	جب (1) {
		صحیح سائز;
		حرف *s = strstr_نمونہ(val, نمونہ, &سائز);
		اگر (!s)
			توڑو;

		نتیجہ = xrealloc(نتیجہ, res_len + (s - val) + repl_len + 1);
		strcpy(mempcpy(نتیجہ + res_len, val, s - val), repl);
		res_len += (s - val) + repl_len;
		debug_printf_varexp("val:'%s' s:'%s' result:'%s'\n", val, s, نتیجہ);

		val = s + سائز;
		اگر (exp_op == '/')
			توڑو;
	}
	اگر (*val && نتیجہ) {
		نتیجہ = xrealloc(نتیجہ, res_len + لڑی_لمبائی(val) + 1);
		strcpy(نتیجہ + res_len, val);
		debug_printf_varexp("val:'%s' result:'%s'\n", val, نتیجہ);
	}
	debug_printf_varexp("result:'%s'\n", نتیجہ);
	واپس نتیجہ;
}
#اگر_ختم /* BASH_PATTERN_SUBST */
#اگر_ختم /* !__U_BOOT__ */

ساکن صحیح append_str_maybe_ifs_بانٹو(o_string *output, صحیح n,
		صحیح پہلا_ch, ثابت حرف *val)
{
	اگر (!(پہلا_ch & 0x80)) { /* unquoted $VAR */
		debug_printf_expand("unquoted '%s', output->o_escape:%d\n", val,
				!!(output->o_expflags & EXP_FLAG_ESC_GLOB_CHARS));
		اگر (val && val[0])
			n = expand_on_ifs(output, n, val);
	} ورنہ { /* quoted "$VAR" */
		output->has_quoted_part = 1;
		debug_printf_expand("quoted '%s', output->o_escape:%d\n", val,
				!!(output->o_expflags & EXP_FLAG_ESC_GLOB_CHARS));
		اگر (val && val[0])
			o_جوڑو_Qstr(output, val);
	}
	واپس n;
}

/* Handle <SPECIAL_VAR_SYMBOL>varname...<SPECIAL_VAR_SYMBOL> construct.
 */
ساکن NOINLINE صحیح expand_one_var(o_string *output, صحیح n,
		صحیح پہلا_ch, حرف *arg, حرف **pp)
{
	ثابت حرف *val;
	حرف *مقصد_be_freed;
	حرف *p;
	حرف *var;
	حرف exp_op;
	حرف exp_save = exp_save; /* for compiler */
	حرف *exp_saveptr; /* points to expansion operator */
	حرف *exp_لفظ = exp_لفظ; /* for compiler */
	حرف arg0;

	val = تہی;
	مقصد_be_freed = تہی;
	p = *pp;
	*p = '\0'; /* replace trailing SPECIAL_VAR_SYMBOL */
	var = arg;
	exp_saveptr = arg[1] ? strchr(VAR_ENCODED_SUBST_OPS, arg[1]) : تہی;
	arg0 = arg[0];
	arg[0] = (arg0 & 0x7f);
	exp_op = 0;

	اگر (arg[0] == '#' && arg[1] /* ${#...} but not ${#} */
	 && (!exp_saveptr               /* and ( not(${#<op_char>...}) */
	    || (arg[2] == '\0' && strchr(SPECIAL_VARS_STR, arg[1])) /* or ${#C} "len of $C" ) */
	    )		/* NB: skipping ^^^specvar check mishandles ${#::2} */
	) {
		/* It must be length operator: ${#var} */
		var++;
		exp_op = 'L';
	} ورنہ {
		/* Maybe handle parameter expansion */
		اگر (exp_saveptr /* if 2nd char is one of expansion operators */
		 && strchr(NUMERIC_SPECVARS_STR, arg[0]) /* 1st char is special variable */
		) {
			/* ${?:0}, ${#[:]%0} etc */
			exp_saveptr = var + 1;
		} ورنہ {
			/* ${?}, ${var}, ${var:0}, ${var[:]%0} etc */
			exp_saveptr = var+1 + strcspn(var+1, VAR_ENCODED_SUBST_OPS);
		}
		exp_op = exp_save = *exp_saveptr;
#اگر_غیر_تعریف_شدہ __U_BOOT__
		اگر (exp_op) {
			exp_لفظ = exp_saveptr + 1;
			اگر (exp_op == ':') {
				exp_op = *exp_لفظ++;
//TODO: try ${var:} and ${var:bogus} in non-bash config
				اگر (BASH_SUBSTR
				 && (!exp_op || !strchr(MINUS_PLUS_EQUAL_QUESTION, exp_op))
				) {
					/* oops... it's ${var:N[:M]}, not ${var:?xxx} or some such */
					exp_op = ':';
					exp_لفظ--;
				}
			}
			*exp_saveptr = '\0';
		} /* else: it's not an expansion op, but bare ${var} */
#اگر_ختم /* !__U_BOOT__ */
	}

	/* Look up the variable in question */
	اگر (ہندسہ_ہے(var[0])) {
		/* parse_dollar should have vetted var for us */
#اگر_غیر_تعریف_شدہ __U_BOOT__
		صحیح nn = xatoi_مثبت(var);
#ورنہ /* __U_BOOT__ */
		صحیح nn = simple_strtoul(var, تہی, 10);
#اگر_ختم /* __U_BOOT__ */
		اگر (nn < G.global_argc)
			val = G.global_argv[nn];
		/* else val remains NULL: $N with too big N */
	} ورنہ {
		انتخاب (var[0]) {
#اگر_غیر_تعریف_شدہ __U_BOOT__
		حالت '$': /* pid */
			val = utoa(G.جڑ_pid);
			توڑو;
		حالت '!': /* bg pid */
			val = G.آخری_bg_pid ? utoa(G.آخری_bg_pid) : "";
			توڑو;
#اگر_ختم /* !__U_BOOT__ */
		حالت '?': /* exitcode */
			val = utoa(G.آخری_exitcode);
			توڑو;
		حالت '#': /* argc */
			val = utoa(G.global_argc ? G.global_argc-1 : 0);
			توڑو;
#اگر_غیر_تعریف_شدہ __U_BOOT__
		حالت '-': { /* active options */
			/* Check set_mode() to see what option chars we support */
			حرف *cp;
			val = cp = G.optstring_buf;
			اگر (G.o_opt[OPT_O_ERREXIT])
				*cp++ = 'e';
			اگر (G_interactive_fd)
				*cp++ = 'i';
			اگر (G_x_منوال)
				*cp++ = 'x';
			/* If G.o_opt[OPT_O_NOEXEC] is true,
			 * commands read but are not executed,
			 * so $- can not execute too, 'n' is never seen in $-.
			 */
			اگر (G.opt_c)
				*cp++ = 'c';
			اگر (G.opt_s)
				*cp++ = 's';
			*cp = '\0';
			توڑو;
		}
#اگر_ختم /* !__U_BOOT__ */
		طےشدہ:
#اگر_غیر_تعریف_شدہ __U_BOOT__
			val = get_local_var_قدر(var);
#ورنہ /* __U_BOOT__ */
			/*
			 * Environment variable set with setenv* have to be
			 * expanded.
			 * So, we first search if the variable exists in
			 * environment, if this is not the case, we default to
			 * local value.
			 */
			val = env_get(var);
			اگر (!val)
				val = get_local_var_قدر(var);
#اگر_ختم /* __U_BOOT__ */
		}
	}

#اگر_غیر_تعریف_شدہ __U_BOOT__
	/* Handle any expansions */
	اگر (exp_op == 'L') {
		reinit_unicode_for_hush();
		debug_printf_expand("expand: length(%s)=", val);
		val = utoa(val ? unicode_strlen(val) : 0);
		debug_printf_expand("%s\n", val);
	} ورنہ اگر (exp_op) {
		اگر (exp_op == '%' || exp_op == '#') {
			/* Standard-mandated substring removal ops:
			 * ${parameter%word} - remove smallest suffix pattern
			 * ${parameter%%word} - remove largest suffix pattern
			 * ${parameter#word} - remove smallest prefix pattern
			 * ${parameter##word} - remove largest prefix pattern
			 *
			 * Word is expanded to produce a glob pattern.
			 * Then var's value is matched to it and matching part removed.
			 */
			/* bash compat: if x is "" and no shrinking of it is possible,
			 * inner ${...} is not evaluated. Example:
			 *  unset b; : ${a%${b=B}}; echo $b
			 * assignment b=B only happens if $a is not "".
			 */
			اگر (val && val[0]) {
				حرف *t;
				حرف *exp_exp_لفظ;
				حرف *loc;
				غیرعلامتی جائزہ_flags = pick_جائزہ(exp_op, *exp_لفظ);
				اگر (exp_op == *exp_لفظ)  /* ## or %% */
					exp_لفظ++;
				debug_printf_expand("expand: exp_word:'%s'\n", exp_لفظ);
				exp_exp_لفظ = encode_then_expand_vararg(exp_لفظ, /*handle_squotes:*/ 1, /*unbackslash:*/ 0);
				اگر (exp_exp_لفظ)
					exp_لفظ = exp_exp_لفظ;
				debug_printf_expand("expand: exp_word:'%s'\n", exp_لفظ);
				/*
				 * HACK ALERT. We depend here on the fact that
				 * G.global_argv and results of utoa and get_local_var_value
				 * are actually in writable memory:
				 * scan_and_match momentarily stores NULs there.
				 */
				t = (حرف*)val;
				loc = جائزہ_and_مطابقت(t, exp_لفظ, جائزہ_flags);
				debug_printf_expand("op:%c str:'%s' pat:'%s' res:'%s'\n", exp_op, t, exp_لفظ, loc);
				آزاد(exp_exp_لفظ);
				اگر (loc) { /* match was found */
					اگر (جائزہ_flags & جائزہ_مطابقت_بایاں_HALF) /* #[#] */
						val = loc; /* take right part */
					ورنہ /* %[%] */
						val = مقصد_be_freed = xstrndup(val, loc - val); /*بایاں*/
				}
			}
		}
#اگر BASH_نمونہ_SUBST
		ورنہ اگر (exp_op == '/' || exp_op == '\\') {
			/* It's ${var/[/]pattern[/repl]} thing.
			 * Note that in encoded form it has TWO parts:
			 * var/pattern<SPECIAL_VAR_SYMBOL>repl<SPECIAL_VAR_SYMBOL>
			 * and if // is used, it is encoded as \:
			 * var\pattern<SPECIAL_VAR_SYMBOL>repl<SPECIAL_VAR_SYMBOL>
			 */
			/* bash compat: if var is "", both pattern and repl
			 * are still evaluated, if it is unset, then not:
			 * unset b; a=; : ${a/z/${b=3}}; echo $b      # b=3
			 * unset b; unset a; : ${a/z/${b=3}}; echo $b # b not set
			 */
			اگر (val /*&& val[0]*/) {
				/* pattern uses non-standard expansion.
				 * repl should be unbackslashed and globbed
				 * by the usual expansion rules:
				 *  >az >bz
				 *  v='a bz'; echo "${v/a*z/a*z}" #prints "a*z"
				 *  v='a bz'; echo "${v/a*z/\z}"  #prints "z"
				 *  v='a bz'; echo ${v/a*z/a*z}   #prints "az"
				 *  v='a bz'; echo ${v/a*z/\z}    #prints "z"
				 * (note that a*z _pattern_ is never globbed!)
				 */
				حرف *نمونہ, *repl, *t;
				نمونہ = encode_then_expand_vararg(exp_لفظ, /*handle_squotes:*/ 1, /*unbackslash:*/ 0);
				اگر (!نمونہ)
					نمونہ = xstrdup(exp_لفظ);
				debug_printf_varexp("pattern:'%s'->'%s'\n", exp_لفظ, نمونہ);
				*p++ = SPECIAL_VAR_علامت;
				exp_لفظ = p;
				p = strchr(p, SPECIAL_VAR_علامت);
				*p = '\0';
				repl = encode_then_expand_vararg(exp_لفظ, /*handle_squotes:*/ 1, /*unbackslash:*/ 1);
				debug_printf_varexp("repl:'%s'->'%s'\n", exp_لفظ, repl);
				/* HACK ALERT. We depend here on the fact that
				 * G.global_argv and results of utoa and get_local_var_value
				 * are actually in writable memory:
				 * replace_pattern momentarily stores NULs there. */
				t = (حرف*)val;
				مقصد_be_freed = replace_نمونہ(t,
						نمونہ,
						(repl ? repl : exp_لفظ),
						exp_op);
				اگر (مقصد_be_freed) /* at least one replace happened */
					val = مقصد_be_freed;
				آزاد(نمونہ);
				آزاد(repl);
			} ورنہ {
				/* Unset variable always gives nothing */
				//  a=; echo ${a/*/w}      # "w"
				//  unset a; echo ${a/*/w} # ""
				/* Just skip "replace" part */
				*p++ = SPECIAL_VAR_علامت;
				p = strchr(p, SPECIAL_VAR_علامت);
				*p = '\0';
			}
		}
#اگر_ختم /* BASH_PATTERN_SUBST */
		ورنہ اگر (exp_op == ':') {
#اگر BASH_SUBSTR && ENABLE_FEATURE_SH_MATH
			/* It's ${var:N[:M]} bashism.
			 * Note that in encoded form it has TWO parts:
			 * var:N<SPECIAL_VAR_SYMBOL>M<SPECIAL_VAR_SYMBOL>
			 */
			arith_t beg, len;
			غیرعلامتی vallen;
			ثابت حرف *errmsg;

			beg = expand_and_evaluate_arith(exp_لفظ, &errmsg);
			اگر (errmsg)
				جاؤ خالی_خانے_نتیجہ;
			debug_printf_varexp("beg:'%s'=%lld\n", exp_لفظ, (طویل طویل)beg);
			*p++ = SPECIAL_VAR_علامت;
			exp_لفظ = p;
			p = strchr(p, SPECIAL_VAR_علامت);
			*p = '\0';
			vallen = val ? لڑی_لمبائی(val) : 0;
			اگر (beg < 0) {
				/* negative beg counts from the end */
				beg = (arith_t)vallen + beg;
			}
			/* If expansion will be empty, do not even evaluate len */
			اگر (!val || beg < 0 || beg > vallen) {
				/* Why > vallen, not >=? bash:
				 * unset b; a=ab; : ${a:2:${b=3}}; echo $b  # "", b=3 (!!!)
				 * unset b; a=a; : ${a:2:${b=3}}; echo $b   # "", b not set
				 */
				جاؤ خالی_خانے_نتیجہ;
			}
			len = expand_and_evaluate_arith(exp_لفظ, &errmsg);
			اگر (errmsg)
				جاؤ خالی_خانے_نتیجہ;
			debug_printf_varexp("len:'%s'=%lld\n", exp_لفظ, (طویل طویل)len);
			debug_printf_varexp("from val:'%s'\n", val);
			اگر (len < 0) {
				/* in bash, len=-n means strlen()-n */
				len = (arith_t)vallen - beg + len;
				اگر (len < 0) /* bash compat */
					پیغام_and_die_if_script("%s: substring expression < 0", var);
			}
			اگر (len <= 0 || !val /*|| beg >= vallen*/) {
 خالی_خانے_نتیجہ:
				val = تہی;
			} ورنہ {
				/* Paranoia. What if user entered 9999999999999
				 * which fits in arith_t but not int? */
				اگر (len > زیادہ_سے_زیادہ_عدد)
					len = زیادہ_سے_زیادہ_عدد;
				val = مقصد_be_freed = xstrndup(val + beg, len);
			}
			debug_printf_varexp("val:'%s'\n", val);
#ورنہ /* not (HUSH_SUBSTR_EXPANSION && FEATURE_SH_MATH) */
			پیغام_and_die_if_script("malformed ${%s:...}", var);
			val = تہی;
#اگر_ختم
		} ورنہ { /* one of "-=+?" */
			/* Standard-mandated substitution ops:
			 * ${var?word} - indicate error if unset
			 *      If var is unset, word (or a message indicating it is unset
			 *      if word is null) is written to standard error
			 *      and the shell exits with a non-zero exit status.
			 *      Otherwise, the value of var is substituted.
			 * ${var-word} - use default value
			 *      If var is unset, word is substituted.
			 * ${var=word} - assign and use default value
			 *      If var is unset, word is assigned to var.
			 *      In all cases, final value of var is substituted.
			 * ${var+word} - use alternative value
			 *      If var is unset, null is substituted.
			 *      Otherwise, word is substituted.
			 *
			 * Word is subjected to tilde expansion, parameter expansion,
			 * command substitution, and arithmetic expansion.
			 * If word is not needed, it is not expanded.
			 *
			 * Colon forms (${var:-word}, ${var:=word} etc) do the same,
			 * but also treat null var as if it is unset.
			 *
			 * Word-splitting and single quote behavior:
			 *
			 * $ f() { for i; do echo "|$i|"; done; }
			 *
			 * $ x=; f ${x:?'x y' z}; echo $?
			 * bash: x: x y z       # neither f nor "echo $?" executes
			 * (if interactive, bash does not exit, but merely aborts to prompt. $? is set to 1)
			 * $ x=; f "${x:?'x y' z}"
			 * bash: x: x y z       # dash prints: dash: x: 'x y' z
			 *
			 * $ x=; f ${x:='x y' z}
			 * |x|
			 * |y|
			 * |z|
			 * $ x=; f "${x:='x y' z}"
			 * |'x y' z|
			 *
			 * $ x=x; f ${x:+'x y' z}
			 * |x y|
			 * |z|
			 * $ x=x; f "${x:+'x y' z}"
			 * |'x y' z|
			 *
			 * $ x=; f ${x:-'x y' z}
			 * |x y|
			 * |z|
			 * $ x=; f "${x:-'x y' z}"
			 * |'x y' z|
			 */
			صحیح use_لفظ = (!val || ((exp_save == ':') && !val[0]));
			اگر (exp_op == '+')
				use_لفظ = !use_لفظ;
			debug_printf_expand("expand: op:%c (null:%s) test:%i\n", exp_op,
					(exp_save == ':') ? "true" : "false", use_لفظ);
			اگر (use_لفظ) {
				اگر (exp_op == '+' || exp_op == '-') {
					/* ${var+word} - use alternative value */
					/* ${var-word} - use default value */
					n = encode_then_append_var_plusminus(output, n, exp_لفظ,
							/*dquoted:*/ (arg0 & 0x80)
					);
					val = تہی;
				} ورنہ {
					/* ${var?word} - indicate error if unset */
					/* ${var=word} - assign and use default value */
					مقصد_be_freed = encode_then_expand_vararg(exp_لفظ,
							/*handle_squotes:*/ !(arg0 & 0x80),
							/*unbackslash:*/ 0
					);
					اگر (مقصد_be_freed)
						exp_لفظ = مقصد_be_freed;
					اگر (exp_op == '?') {
						/* mimic bash message */
						پیغام_and_die_if_script("%s: %s",
							var,
							exp_لفظ[0]
							? exp_لفظ
							: "parameter null or not set"
							/* ash has more specific messages, a-la: */
							/*: (exp_save == ':' ? "parameter null or not set" : "parameter not set")*/
						);
//TODO: how interactive bash aborts expansion mid-command?
//It aborts the entire line, returns to prompt:
// $ f() { for i; do echo "|$i|"; done; }; x=; f "${x:?'x y' z}"; echo YO
// bash: x: x y z
// $
// ("echo YO" is not executed, neither the f function call)
					} ورنہ {
						val = exp_لفظ;
					}
					اگر (exp_op == '=') {
						/* ${var=[word]} or ${var:=[word]} */
						اگر (ہندسہ_ہے(var[0]) || var[0] == '#') {
							/* mimic bash message */
							پیغام_and_die_if_script("$%s: cannot assign in this way", var);
							val = تہی;
						} ورنہ {
							حرف *new_var = xasprintf("%s=%s", var, val);
							سیٹ_local_var0(new_var);
						}
					}
				}
			}
		} /* one of "-=+?" */

		*exp_saveptr = exp_save;
	} /* if (exp_op) */

#اگر_ختم /* !__U_BOOT__ */
	arg[0] = arg0;
	*pp = p;

	n = append_str_maybe_ifs_بانٹو(output, n, پہلا_ch, val);

	آزاد(مقصد_be_freed);
	واپس n;
}

/* Expand all variable references in given string, adding words to list[]
 * at n, n+1,... positions. Return updated n (so that list[n] is next one
 * to be filled). This routine is extremely tricky: has to deal with
 * variables/parameters with whitespace, $* and $@, and constructs like
 * 'echo -$*-'. If you play here, you must run testsuite afterwards! */
ساکن NOINLINE صحیح expand_vars_مقصد_list(o_string *output, صحیح n, حرف *arg)
{
	/* output->o_expflags & EXP_FLAG_SINGLEWORD (0x80) if we are in
	 * expansion of right-hand side of assignment == 1-element expand.
	 */
	حرف cant_be_null = 0; /* only bit 0x80 matters */
	حرف *p;

	debug_printf_expand("expand_vars_to_list: arg:'%s' singleword:%x\n", arg,
			!!(output->o_expflags & EXP_FLAG_SINGLEWORD));
	debug_print_list("expand_vars_to_list[0]", output, n);

	جب ((p = strchr(arg, SPECIAL_VAR_علامت)) != تہی) {
		حرف پہلا_ch;
#اگر ENABLE_FEATURE_SH_MATH
		حرف arith_buf[حجم(arith_t)*3 + 2];
#اگر_ختم

		اگر (output->ended_اندر_ifs) {
			o_addchr(output, '\0');
			n = o_save_ptr(output, n);
			output->ended_اندر_ifs = 0;
		}

		o_addblock(output, arg, p - arg);
		debug_print_list("expand_vars_to_list[1]", output, n);
		arg = ++p;
		p = strchr(p, SPECIAL_VAR_علامت);

		/* Fetch special var name (if it is indeed one of them)
		 * and quote bit, force the bit on if singleword expansion -
		 * important for not getting v=$@ expand to many words. */
		پہلا_ch = arg[0] | (output->o_expflags & EXP_FLAG_SINGLEWORD);

		/* Is this variable quoted and thus expansion can't be null?
		 * "$@" is special. Even if quoted, it can still
		 * expand to nothing (not even an empty string),
		 * thus it is excluded. */
		اگر ((پہلا_ch & 0x7f) != '@')
			cant_be_null |= پہلا_ch;

		انتخاب (پہلا_ch & 0x7f) {
		/* Highest bit in first_ch indicates that var is double-quoted */
		حالت '*':
		حالت '@': {
			صحیح i;
#اگر_غیر_تعریف_شدہ __U_BOOT__
			اگر (!G.global_argv[1])
#ورنہ /* __U_BOOT__ */
			اگر (!G.global_argv || !G.global_argv[1])
#اگر_ختم /* __U_BOOT__ */
				توڑو;
			i = 1;
			cant_be_null |= پہلا_ch; /* do it for "$@" _now_, when we know it's not empty */
			اگر (!(پہلا_ch & 0x80)) { /* unquoted $* or $@ */
				جب (G.global_argv[i]) {
					n = expand_on_ifs(output, n, G.global_argv[i]);
					debug_printf_expand("expand_vars_to_list: argv %d (last %d)\n", i, G.global_argc - 1);
					اگر (G.global_argv[i++][0] && G.global_argv[i]) {
						/* this argv[] is not empty and not last:
						 * put terminating NUL, start new word */
						o_addchr(output, '\0');
						debug_print_list("expand_vars_to_list[2]", output, n);
						n = o_save_ptr(output, n);
						debug_print_list("expand_vars_to_list[3]", output, n);
					}
				}
			} ورنہ
			/* If EXP_FLAG_SINGLEWORD, we handle assignment 'a=....$@.....'
			 * and in this case should treat it like '$*' - see 'else...' below */
			اگر (پہلا_ch == (حرف)('@'|0x80)  /* quoted $@ */
			 && !(output->o_expflags & EXP_FLAG_SINGLEWORD) /* not v="$@" case */
			) {
				جب (1) {
					o_جوڑو_Qstr(output, G.global_argv[i]);
					اگر (++i >= G.global_argc)
						توڑو;
					o_addchr(output, '\0');
					debug_print_list("expand_vars_to_list[4]", output, n);
					n = o_save_ptr(output, n);
				}
			} ورنہ { /* quoted $* (or v="$@" case): add as one word */
				جب (1) {
					o_جوڑو_Qstr(output, G.global_argv[i]);
					اگر (!G.global_argv[++i])
						توڑو;
					اگر (G.ifs[0])
						o_addchr(output, G.ifs[0]);
				}
				output->has_quoted_part = 1;
			}
			توڑو;
		}
		حالت SPECIAL_VAR_علامت: {
			/* <SPECIAL_VAR_SYMBOL><SPECIAL_VAR_SYMBOL> */
			/* "Empty variable", used to make "" etc to not disappear */
			output->has_quoted_part = 1;
			cant_be_null = 0x80;
			arg++;
			توڑو;
		}
		حالت SPECIAL_VAR_QUOTED_SVS:
			/* <SPECIAL_VAR_SYMBOL><SPECIAL_VAR_QUOTED_SVS><SPECIAL_VAR_SYMBOL> */
			/* "^C variable", represents literal ^C char (possible in scripts) */
			o_addchr(output, SPECIAL_VAR_علامت);
			arg++;
			توڑو;
#اگر ENABLE_HUSH_TICK
		حالت '`': {
			/* <SPECIAL_VAR_SYMBOL>`cmd<SPECIAL_VAR_SYMBOL> */
			o_string subst_نتیجہ = NULL_O_STRING;

			*p = '\0'; /* replace trailing <SPECIAL_VAR_SYMBOL> */
			arg++;
			/* Can't just stuff it into output o_string,
			 * expanded result may need to be globbed
			 * and $IFS-split */
			debug_printf_subst("SUBST '%s' first_ch %x\n", arg, پہلا_ch);
			G.آخری_exitcode = process_command_subs(&subst_نتیجہ, arg);
			G.expand_exitcode = G.آخری_exitcode;
			debug_printf_subst("SUBST RES:%d '%s'\n", G.آخری_exitcode, subst_نتیجہ.ڈیٹا);
			n = append_str_maybe_ifs_بانٹو(output, n, پہلا_ch, subst_نتیجہ.ڈیٹا);
			o_free(&subst_نتیجہ);
			توڑو;
		}
#اگر_ختم
#اگر ENABLE_FEATURE_SH_MATH
		حالت '+': {
			/* <SPECIAL_VAR_SYMBOL>+arith<SPECIAL_VAR_SYMBOL> */
			arith_t res;

			arg++; /* skip '+' */
			*p = '\0'; /* replace trailing <SPECIAL_VAR_SYMBOL> */
			debug_printf_subst("ARITH '%s' first_ch %x\n", arg, پہلا_ch);
			res = expand_and_evaluate_arith(arg, تہی);
			debug_printf_subst("ARITH RES '"ARITH_FMT"'\n", res);
			sprintf(arith_buf, ARITH_FMT, res);
			اگر (res < 0
			 && پہلا_ch == (حرف)('+'|0x80)
			/* && (output->o_expflags & EXP_FLAG_ESC_GLOB_CHARS) */
			) {
				/* Quoted negative ariths, like filename[0"$((-9))"],
				 * should not be interpreted as glob ranges.
				 * Convert leading '-' to '\-':
				 */
				o_grow_by(output, 1);
				output->ڈیٹا[output->لمبائی++] = '\\';
			}
			o_addstr(output, arith_buf);
			توڑو;
		}
#اگر_ختم
		طےشدہ:
			/* <SPECIAL_VAR_SYMBOL>varname[ops]<SPECIAL_VAR_SYMBOL> */
			n = expand_one_var(output, n, پہلا_ch, arg, &p);
			توڑو;
		} /* switch (char after <SPECIAL_VAR_SYMBOL>) */

		/* Restore NULL'ed SPECIAL_VAR_SYMBOL.
		 * Do the check to avoid writing to a const string. */
		اگر (*p != SPECIAL_VAR_علامت)
			*p = SPECIAL_VAR_علامت;
		arg = ++p;
	} /* end of "while (SPECIAL_VAR_SYMBOL is found) ..." */

	اگر (*arg) {
		/* handle trailing string */
		اگر (output->ended_اندر_ifs) {
			o_addchr(output, '\0');
			n = o_save_ptr(output, n);
		}
		debug_print_list("expand_vars_to_list[a]", output, n);
		/* this part is literal, and it was already pre-quoted
		 * if needed (much earlier), do not use o_addQstr here!
		 */
		o_addstr(output, arg);
		debug_print_list("expand_vars_to_list[b]", output, n);
	} ورنہ
	اگر (output->لمبائی == o_get_آخری_ptr(output, n) /* expansion is empty */
	 && !(cant_be_null & 0x80)   /* and all vars were not quoted */
	 && !output->has_quoted_part
	) {
		n--;
		/* allow to reuse list[n] later without re-growth */
		output->has_خالی_خانے_خانہ = 1;
	}

	واپس n;
}

ساکن حرف **expand_variables(حرف **argv, غیرعلامتی expflags)
{
	صحیح n;
	حرف **list;
	o_string output = NULL_O_STRING;

	output.o_expflags = expflags;

	n = 0;
	برائے (;;) {
		/* go to next list[n] */
		output.ended_اندر_ifs = 0;
		n = o_save_ptr(&output, n);

		اگر (!*argv)
			توڑو;

		/* expand argv[i] */
		n = expand_vars_مقصد_list(&output, n, *argv++);
		/* if (!output->has_empty_slot) -- need this?? */
			o_addchr(&output, '\0');
	}
	debug_print_list("expand_variables", &output, n);

	/* output.data (malloced in one block) gets returned in "list" */
	list = o_finalize_list(&output, n);
	debug_print_لڑیاں("expand_variables[1]", list);
	واپس list;
}

ساکن حرف **expand_strvec_مقصد_strvec(حرف **argv)
{
	واپس expand_variables(argv, EXP_FLAG_GLOB | EXP_FLAG_ESC_GLOB_CHARS);
}

#اگر defined(CMD_SINGLEWORD_NOGLOB) || defined(CMD_TEST2_SINGLEWORD_NOGLOB)
ساکن حرف **expand_strvec_مقصد_strvec_singleword_noglob(حرف **argv)
{
	واپس expand_variables(argv, EXP_FLAG_SINGLEWORD);
}
#اگر_ختم

/* Used for expansion of right hand of assignments,
 * $((...)), heredocs, variable expansion parts.
 *
 * NB: should NOT do globbing!
 * "export v=/bin/c*; env | grep ^v=" outputs "v=/bin/c*"
 */
ساکن حرف *expand_string_مقصد_string(ثابت حرف *str, صحیح EXP_flags, صحیح do_unbackslash)
{
#اگر !BASH_نمونہ_SUBST && !ENABLE_HUSH_CASE
	ثابت صحیح do_unbackslash = 1;
	ثابت صحیح EXP_flags = EXP_FLAG_ESC_GLOB_CHARS;
#اگر_ختم
	حرف *argv[2], **list;

	debug_printf_expand("string_to_string<='%s'\n", str);
	/* This is generally an optimization, but it also
	 * handles "", which otherwise trips over !list[0] check below.
	 * (is this ever happens that we actually get str="" here?)
	 */
	اگر (!strchr(str, SPECIAL_VAR_علامت) && !strchr(str, '\\')) {
		//TODO: Can use on strings with \ too, just unbackslash() them?
		debug_printf_expand("string_to_string(fast)=>'%s'\n", str);
		واپس xstrdup(str);
	}

	argv[0] = (حرف*)str;
	argv[1] = تہی;
	list = expand_variables(argv, EXP_flags | EXP_FLAG_SINGLEWORD);
	اگر (!list[0]) {
		/* Example where it happens:
		 * x=; echo ${x:-"$@"}
		 */
		((حرف*)list)[0] = '\0';
	} ورنہ {
		اگر (HUSH_DEBUG)
			اگر (list[1])
				bb_simple_error_پیغام_and_die("BUG in varexp2");
		/* actually, just move string 2*sizeof(char*) bytes back */
		overlapping_strcpy((حرف*)list, list[0]);
		اگر (do_unbackslash)
			unbackslash((حرف*)list);
	}
	debug_printf_expand("string_to_string=>'%s'\n", (حرف*)list);
	واپس (حرف*)list;
}

#اگر 0
ساکن حرف* expand_strvec_مقصد_string(حرف **argv)
{
	حرف **list;

	list = expand_variables(argv, EXP_FLAG_SINGLEWORD);
	/* Convert all NULs to spaces */
	اگر (list[0]) {
		صحیح n = 1;
		جب (list[n]) {
			اگر (HUSH_DEBUG)
				اگر (list[n-1] + لڑی_لمبائی(list[n-1]) + 1 != list[n])
					bb_error_پیغام_and_die("BUG in varexp3");
			/* bash uses ' ' regardless of $IFS contents */
			list[n][-1] = ' ';
			n++;
		}
	}
	overlapping_strcpy((حرف*)list, list[0] ? list[0] : "");
	debug_printf_expand("strvec_to_string='%s'\n", (حرف*)list);
	واپس (حرف*)list;
}
#اگر_ختم

#اگر_غیر_تعریف_شدہ __U_BOOT__
ساکن حرف **expand_assignments(حرف **argv, صحیح گنتی)
{
	صحیح i;
	حرف **p;

	G.expanded_assignments = p = تہی;
	/* Expand assignments into one string each */
	برائے (i = 0; i < گنتی; i++) {
		p = جوڑو_string_مقصد_لڑیاں(p,
			expand_string_مقصد_string(argv[i],
				EXP_FLAG_ESC_GLOB_CHARS,
				/*unbackslash:*/ 1
			)
		);
		G.expanded_assignments = p;
	}
	G.expanded_assignments = تہی;
	واپس p;
}

ساکن خالی switch_off_special_sigs(غیرعلامتی نقاب)
{
	غیرعلامتی sig = 0;
	جب ((نقاب >>= 1) != 0) {
		sig++;
		اگر (!(نقاب & 1))
			جاری;
#اگر ENABLE_HUSH_TRAP
		اگر (G_traps) {
			اگر (G_traps[sig] && !G_traps[sig][0])
				/* trap is '', has to remain SIG_IGN */
				جاری;
			آزاد(G_traps[sig]);
			G_traps[sig] = تہی;
		}
#اگر_ختم
		/* We are here only if no trap or trap was not '' */
		install_sighandler(sig, SIG_DFL);
	}
}
#اگر_ختم /* !__U_BOOT__ */

#اگر_غیر_تعریف_شدہ __U_BOOT__
#اگر BB_MMU
/* never called */
خالی re_execute_shell(حرف ***مقصد_free, ثابت حرف *s,
		حرف *g_argv0, حرف **g_argv,
		حرف **builtin_argv) NORETURN;

ساکن خالی reset_traps_مقصد_defaults(خالی)
{
	/* This function is always called in a child shell
	 * after fork (not vfork, NOMMU doesn't use this function).
	 */
	IF_HUSH_TRAP(غیرعلامتی sig;)
	غیرعلامتی نقاب;

	/* Child shells are not interactive.
	 * SIGTTIN/SIGTTOU/SIGTSTP should not have special handling.
	 * Testcase: (while :; do :; done) + ^Z should background.
	 * Same goes for SIGTERM, SIGHUP, SIGINT.
	 */
	نقاب = (G.special_sig_نقاب & SPECIAL_INTERACTIVE_SIGS) | G_fatal_sig_نقاب;
	اگر (!G_traps && !نقاب)
		واپس; /* already no traps and no special sigs */

	/* Switch off special sigs */
	switch_off_special_sigs(نقاب);
# اگر ENABLE_HUSH_کام
	G_fatal_sig_نقاب = 0;
# endif
	G.special_sig_نقاب &= ~SPECIAL_INTERACTIVE_SIGS;
	/* SIGQUIT,SIGCHLD and maybe SPECIAL_JOBSTOP_SIGS
	 * remain set in G.special_sig_mask */

# اگر ENABLE_HUSH_TRAP
	اگر (!G_traps)
		واپس;

	/* Reset all sigs to default except ones with empty traps */
	برائے (sig = 0; sig < NSIG; sig++) {
		اگر (!G_traps[sig])
			جاری; /* no trap: nothing to do */
		اگر (!G_traps[sig][0])
			جاری; /* empty trap: has to remain SIG_IGN */
		/* sig has non-empty trap, reset it: */
		آزاد(G_traps[sig]);
		G_traps[sig] = تہی;
		/* There is no signal for trap 0 (EXIT) */
		اگر (sig == 0)
			جاری;
		install_sighandler(sig, pick_sighandler(sig));
	}
# endif
}

#ورنہ /* !BB_MMU */

ساکن خالی re_execute_shell(حرف ***مقصد_free, ثابت حرف *s,
		حرف *g_argv0, حرف **g_argv,
		حرف **builtin_argv) NORETURN;
ساکن خالی re_execute_shell(حرف ***مقصد_free, ثابت حرف *s,
		حرف *g_argv0, حرف **g_argv,
		حرف **builtin_argv)
{
# define NOMMU_HACK_FMT ("-$%x:%x:%x:%x:%x:%llx" IF_HUSH_LOOPS(":%x"))
	/* delims + 2 * (number of bytes in printed hex numbers) */
	حرف param_buf[حجم(NOMMU_HACK_FMT) + 2 * (حجم(صحیح)*6 + حجم(طویل طویل)*1)];
	حرف *heredoc_argv[4];
	ساخت variable *cur;
# اگر ENABLE_HUSH_FUNCTIONS
	ساخت function *funcp;
# endif
	حرف **argv, **pp;
	غیرعلامتی cnt;
	غیرعلامتی طویل طویل خالی_خانے_trap_نقاب;

	اگر (!g_argv0) { /* heredoc */
		argv = heredoc_argv;
		argv[0] = (حرف *) G.argv0_for_re_execing;
		argv[1] = (حرف *) "-<";
		argv[2] = (حرف *) s;
		argv[3] = تہی;
		pp = &argv[3]; /* used as pointer to empty environment */
		جاؤ do_exec;
	}

	cnt = 0;
	pp = builtin_argv;
	اگر (pp) جب (*pp++)
		cnt++;

	خالی_خانے_trap_نقاب = 0;
	اگر (G_traps) {
		صحیح sig;
		برائے (sig = 1; sig < NSIG; sig++) {
			اگر (G_traps[sig] && !G_traps[sig][0])
				خالی_خانے_trap_نقاب |= 1LL << sig;
		}
	}

	sprintf(param_buf, NOMMU_HACK_FMT
			, (غیرعلامتی) G.جڑ_pid
			, (غیرعلامتی) G.جڑ_ppid
			, (غیرعلامتی) G.آخری_bg_pid
			, (غیرعلامتی) G.آخری_exitcode
			, cnt
			, خالی_خانے_trap_نقاب
			IF_HUSH_LOOPS(, G.گہرائی_کا_loop)
			);
# undef NOMMU_HACK_FMT
	/* 1:hush 2:-$<pid>:<pid>:<exitcode>:<etc...> <vars...> <funcs...>
	 * 3:-c 4:<cmd> 5:<arg0> <argN...> 6:NULL
	 */
	cnt += 6;
	برائے (cur = G.چوٹی_var; cur; cur = cur->اگلا) {
		اگر (!cur->flg_export || cur->flg_read_only)
			cnt += 2;
	}
# اگر ENABLE_HUSH_LINENO_VAR
	cnt += 2;
# endif
# اگر ENABLE_HUSH_FUNCTIONS
	برائے (funcp = G.چوٹی_func; funcp; funcp = funcp->اگلا)
		cnt += 3;
# endif
	pp = g_argv;
	جب (*pp++)
		cnt++;
	*مقصد_free = argv = pp = xzalloc(حجم(argv[0]) * cnt);
	*pp++ = (حرف *) G.argv0_for_re_execing;
	*pp++ = param_buf;
	برائے (cur = G.چوٹی_var; cur; cur = cur->اگلا) {
		اگر (strcmp(cur->varstr, hush_نسخہ_str) == 0)
			جاری;
		اگر (cur->flg_read_only) {
			*pp++ = (حرف *) "-R";
			*pp++ = cur->varstr;
		} ورنہ اگر (!cur->flg_export) {
			*pp++ = (حرف *) "-V";
			*pp++ = cur->varstr;
		}
	}
# اگر ENABLE_HUSH_LINENO_VAR
	*pp++ = (حرف *) "-L";
	*pp++ = utoa(G.execute_lineno);
# endif
# اگر ENABLE_HUSH_FUNCTIONS
	برائے (funcp = G.چوٹی_func; funcp; funcp = funcp->اگلا) {
		*pp++ = (حرف *) "-F";
		*pp++ = funcp->name;
		*pp++ = funcp->body_as_string;
	}
# endif
	/* We can pass activated traps here. Say, -Tnn:trap_string
	 *
	 * However, POSIX says that subshells reset signals with traps
	 * to SIG_DFL.
	 * I tested bash-3.2 and it not only does that with true subshells
	 * of the form ( list ), but with any forked children shells.
	 * I set trap "echo W" WINCH; and then tried:
	 *
	 * { echo 1; sleep 20; echo 2; } &
	 * while true; do echo 1; sleep 20; echo 2; break; done &
	 * true | { echo 1; sleep 20; echo 2; } | cat
	 *
	 * In all these cases sending SIGWINCH to the child shell
	 * did not run the trap. If I add trap "echo V" WINCH;
	 * _inside_ group (just before echo 1), it works.
	 *
	 * I conclude it means we don't need to pass active traps here.
	 */
	*pp++ = (حرف *) "-c";
	*pp++ = (حرف *) s;
	اگر (builtin_argv) {
		جب (*++builtin_argv)
			*pp++ = *builtin_argv;
		*pp++ = (حرف *) "";
	}
	*pp++ = g_argv0;
	جب (*g_argv)
		*pp++ = *g_argv++;
	/* *pp = NULL; - is already there */
	pp = environ;

 do_exec:
	debug_printf_exec("re_execute_shell pid:%d cmd:'%s'\n", getpid(), s);
	/* Don't propagate SIG_IGN to the child */
	اگر (SPECIAL_JOBSTOP_SIGS != 0)
		switch_off_special_sigs(G.special_sig_نقاب & SPECIAL_JOBSTOP_SIGS);
	execve(bb_busybox_exec_راستہ, argv, pp);
	/* Fallback. Useful for init=/bin/hush usage etc */
	اگر (argv[0][0] == '/')
		execve(argv[0], argv, pp);
	xfunc_error_retval = 127;
	bb_simple_error_پیغام_and_die("can't re-execute the shell");
}
#اگر_ختم  /* !BB_MMU */

#اگر_ختم /* !__U_BOOT__ */

ساکن صحیح تسلسل_and_free_list(ساخت pipe *pi);

/* Executing from string: eval, sh -c '...'
 *          or from file: /etc/profile, . file, sh <script>, sh (intereactive)
 * end_trigger controls how often we stop parsing
 * NUL: parse all, execute, return
 * ';': parse till ';' or newline, execute, repeat till EOF
 */
#اگر_غیر_تعریف_شدہ __U_BOOT__
ساکن خالی parse_and_تسلسل_دھارا(ساخت اندر_str *inp, صحیح انتہا_trigger)
#ورنہ /* __U_BOOT__ */
ساکن صحیح parse_and_تسلسل_دھارا(ساخت اندر_str *inp, صحیح انتہا_trigger)
#اگر_ختم /* __U_BOOT__ */
{
	/* Why we need empty flag?
	 * An obscure corner case "false; ``; echo $?":
	 * empty command in `` should still set $? to 0.
	 * But we can't just set $? to 0 at the start,
	 * this breaks "false; echo `echo $?`" case.
	 */
	bool خالی_خانے = 1;
#اگر_غیر_تعریف_شدہ __U_BOOT__
	جب (1) {
#ورنہ /* __U_BOOT__ */
	کرو {
#اگر_ختم /* __U_BOOT__ */
		ساخت pipe *pipe_list;

#اگر ENABLE_HUSH_INTERACTIVE
		اگر (انتہا_trigger == ';') {
			G.promptmode = 0; /* PS1 */
			debug_printf_prompt("%s promptmode=%d\n", __func__, G.promptmode);
		}
#اگر_ختم
		pipe_list = parse_دھارا(تہی, تہی, inp, انتہا_trigger);
		اگر (!pipe_list || pipe_list == ERR_PTR) { /* EOF/error */
			/* If we are in "big" script
			 * (not in `cmd` or something similar)...
			 */
			اگر (pipe_list == ERR_PTR && انتہا_trigger == ';') {
				/* Discard cached input (rest of line) */
				صحیح ch = inp->آخری_char;
				جب (ch != EOF && ch != '\n') {
					//bb_error_msg("Discarded:'%c'", ch);
					ch = i_getch(inp);
				}
				/* Force prompt */
				inp->p = تہی;
				/* This stream isn't empty */
				خالی_خانے = 0;
				جاری;
			}
			اگر (!pipe_list && خالی_خانے)
				G.آخری_exitcode = 0;
			توڑو;
		}
		debug_print_درخت(pipe_list, 0);
		debug_printf_exec("parse_and_run_stream: run_and_free_list\n");
#اگر_غیر_تعریف_شدہ __U_BOOT__
		تسلسل_and_free_list(pipe_list);
#ورنہ /* __U_BOOT__ */
		صحیح rcode = تسلسل_and_free_list(pipe_list);
		/*
		 * We reset input string to not run the following command, so running
		 * 'exit; echo foo' does not print foo.
		 */
		اگر (rcode <= EXIT_RET_رمز)
			تیاری_file_اندر_str(inp);
#اگر_ختم /* __U_BOOT__ */
		خالی_خانے = 0;
		اگر (G_flag_return_اندر_پیش_رفت == 1)
			توڑو;
#اگر_غیر_تعریف_شدہ __U_BOOT__
	}
#ورنہ /* __U_BOOT__ */
	/*
	 * This do/while is needed by run_command to avoid looping on a command
	 * with syntax error.
	 */
	} جب (!(G.تسلسل_command_flags & FLAG_EXIT_FROM_LOOP));

	واپس G.آخری_exitcode;
#اگر_ختم /* __U_BOOT__ */
}

#اگر_غیر_تعریف_شدہ __U_BOOT__
ساکن خالی parse_and_تسلسل_string(ثابت حرف *s)
#ورنہ /* __U_BOOT__ */
ساکن صحیح parse_and_تسلسل_string(ثابت حرف *s)
#اگر_ختم /* __U_BOOT__ */
{
	ساخت اندر_str داخلہ;
#اگر_غیر_تعریف_شدہ __U_BOOT__
	IF_HUSH_LINENO_VAR(غیرعلامتی sv = G.parse_lineno;)
#ورنہ /* __U_BOOT__ */
	//IF_HUSH_LINENO_VAR(unsigned sv = G.parse_lineno;)
#اگر_ختم /* __U_BOOT__ */

	تیاری_string_اندر_str(&داخلہ, s);
#اگر_غیر_تعریف_شدہ __U_BOOT__
	parse_and_تسلسل_دھارا(&داخلہ, '\0');
#ورنہ /* __U_BOOT__ */
	واپس parse_and_تسلسل_دھارا(&داخلہ, '\0');
#اگر_ختم /* __U_BOOT__ */
#اگر_غیر_تعریف_شدہ __U_BOOT__
	IF_HUSH_LINENO_VAR(غیرعلامتی sv = G.parse_lineno;)
#ورنہ /* __U_BOOT__ */
	//IF_HUSH_LINENO_VAR(unsigned sv = G.parse_lineno;)
#اگر_ختم /* __U_BOOT__ */
}

#اگر_تعریف_شدہ __U_BOOT__
صحیح parse_string_outer_modern(ثابت حرف *cmd, صحیح flags)
{
	صحیح ret;
	صحیح old_flags;

	/*
	 * Keep old values of run_command to be able to restore them once
	 * command was executed.
	 */
	old_flags = G.تسلسل_command_flags;
	G.تسلسل_command_flags = flags;

	ret = parse_and_تسلسل_string(cmd);

	G.تسلسل_command_flags = old_flags;

	واپس ret;
}
#اگر_ختم /* __U_BOOT__ */
#اگر_غیر_تعریف_شدہ __U_BOOT__
ساکن خالی parse_and_تسلسل_file(HFILE *fp)
#ورنہ /* __U_BOOT__ */
خالی parse_and_تسلسل_file(خالی)
#اگر_ختم /* __U_BOOT__ */
{
	ساخت اندر_str داخلہ;
#اگر_غیر_تعریف_شدہ __U_BOOT__
	IF_HUSH_LINENO_VAR(غیرعلامتی sv = G.parse_lineno;)

	IF_HUSH_LINENO_VAR(G.parse_lineno = 1;)
	تیاری_file_اندر_str(&داخلہ, fp);
#ورنہ /* __U_BOOT__ */
	تیاری_file_اندر_str(&داخلہ);
#اگر_ختم /* __U_BOOT__ */
	parse_and_تسلسل_دھارا(&داخلہ, ';');
#اگر_غیر_تعریف_شدہ __U_BOOT__
	IF_HUSH_LINENO_VAR(G.parse_lineno = sv;)
#اگر_ختم /* !__U_BOOT__ */
}

#اگر_غیر_تعریف_شدہ __U_BOOT__
#اگر ENABLE_HUSH_TICK
ساکن صحیح generate_دھارا_from_string(ثابت حرف *s, pid_t *pid_p)
{
	pid_t pid;
	صحیح channel[2];
# اگر !BB_MMU
	حرف **مقصد_free = تہی;
# endif

	xpipe(channel);
	pid = BB_MMU ? xfork() : xvfork();
	اگر (pid == 0) { /*بچہ*/
		disable_restore_tty_pgrp_on_exit();
		/* Process substitution is not considered to be usual
		 * 'command execution'.
		 * SUSv3 says ctrl-Z should be ignored, ctrl-C should not.
		 */
		bb_اشارات(0
			+ (1 << SIGTSTP)
			+ (1 << SIGTTIN)
			+ (1 << SIGTTOU)
			, SIG_IGN);
		close(channel[0]); /* NB: close _first_, then move fd! */
		xmove_fd(channel[1], 1);
# اگر ENABLE_HUSH_TRAP
		/* Awful hack for `trap` or $(trap).
		 *
		 * http://www.opengroup.org/onlinepubs/009695399/utilities/trap.html
		 * contains an example where "trap" is executed in a subshell:
		 *
		 * save_traps=$(trap)
		 * ...
		 * eval "$save_traps"
		 *
		 * Standard does not say that "trap" in subshell shall print
		 * parent shell's traps. It only says that its output
		 * must have suitable form, but then, in the above example
		 * (which is not supposed to be normative), it implies that.
		 *
		 * bash (and probably other shell) does implement it
		 * (traps are reset to defaults, but "trap" still shows them),
		 * but as a result, "trap" logic is hopelessly messed up:
		 *
		 * # trap
		 * trap -- 'echo Ho' SIGWINCH  <--- we have a handler
		 * # (trap)        <--- trap is in subshell - no output (correct, traps are reset)
		 * # true | trap   <--- trap is in subshell - no output (ditto)
		 * # echo `true | trap`    <--- in subshell - output (but traps are reset!)
		 * trap -- 'echo Ho' SIGWINCH
		 * # echo `(trap)`         <--- in subshell in subshell - output
		 * trap -- 'echo Ho' SIGWINCH
		 * # echo `true | (trap)`  <--- in subshell in subshell in subshell - output!
		 * trap -- 'echo Ho' SIGWINCH
		 *
		 * The rules when to forget and when to not forget traps
		 * get really complex and nonsensical.
		 *
		 * Our solution: ONLY bare $(trap) or `trap` is special.
		 */
		s = skip_whitespace(s);
		اگر (is_prefixed_with(s, "trap")
		 && skip_whitespace(s + 4)[0] == '\0'
		) {
			ساکن ثابت حرف *ثابت argv[] ALIGN_PTR = { تہی, تہی };
			builtin_trap((حرف**)argv);
			fflush_all(); /* important */
			_exit(0);
		}
# endif
# اگر BB_MMU
		/* Prevent it from trying to handle ctrl-z etc */
		IF_HUSH_کام(G.تسلسل_list_level = 1;)
		CLEAR_RANDOM_T(&G.random_gen); /* or else $RANDOM repeats in child */
		reset_traps_مقصد_defaults();
		IF_HUSH_منوال_X(G.x_منوال_گہرائی++;)
		//bb_error_msg("%s: ++x_mode_depth=%d", __func__, G.x_mode_depth);
		parse_and_تسلسل_string(s);
		_exit(G.آخری_exitcode);
# ورنہ
	/* We re-execute after vfork on NOMMU. This makes this script safe:
	 * yes "0123456789012345678901234567890" | dd bs=32 count=64k >BIG
	 * huge=`cat BIG` # was blocking here forever
	 * echo OK
	 */
		re_execute_shell(&مقصد_free,
				s,
				G.global_argv[0],
				G.global_argv + 1,
				تہی);
# endif
	}

	/*والد*/
	*pid_p = pid;
# اگر ENABLE_HUSH_FAST
	G.گنتی_SIGCHLD++;
//bb_error_msg("[%d] fork in generate_stream_from_string:"
//		" G.count_SIGCHLD:%d G.handled_SIGCHLD:%d",
//		getpid(), G.count_SIGCHLD, G.handled_SIGCHLD);
# endif
	enable_restore_tty_pgrp_on_exit();
# اگر !BB_MMU
	آزاد(مقصد_free);
# endif
	close(channel[1]);
	واپس channel[0];
}

/* Return code is exit status of the process that is run. */
ساکن صحیح process_command_subs(o_string *dest, ثابت حرف *s)
{
	FILE *fp;
	pid_t pid;
	صحیح حالت_2, ch, eol_cnt;

	fp = xfdopen_for_read(generate_دھارا_from_string(s, &pid));

	/* Now send results of command back into original context */
	eol_cnt = 0;
	جب ((ch = getc(fp)) != EOF) {
		اگر (ch == '\0')
			جاری;
		اگر (ch == '\n') {
			eol_cnt++;
			جاری;
		}
		جب (eol_cnt) {
			o_addchr(dest, '\n');
			eol_cnt--;
		}
		o_جوڑو_Qchr(dest, ch);
	}

	debug_printf("done reading from `cmd` pipe, closing it\n");
	fclose(fp);
	/* We need to extract exitcode. Test case
	 * "true; echo `sleep 1; false` $?"
	 * should print 1 */
	محفوظ_waitpid(pid, &حالت_2, 0);
	debug_printf("child exited. returning its exitcode:%d\n", WEXITSTATUS(حالت_2));
	واپس WEXITSTATUS(حالت_2);
}
#اگر_ختم /* ENABLE_HUSH_TICK */

ساکن خالی تیاری_heredoc(ساخت redir_struct *redir)
{
	ساخت fd_pair pair;
	pid_t pid;
	صحیح len, written;
	/* the _body_ of heredoc (misleading field name) */
	ثابت حرف *heredoc = redir->rd_filename;
	حرف *expanded;
#اگر !BB_MMU
	حرف **مقصد_free;
#اگر_ختم

	expanded = تہی;
	اگر (!(redir->rd_dup & HEREDOC_QUOTED)) {
		expanded = encode_then_expand_string(heredoc);
		اگر (expanded)
			heredoc = expanded;
	}
	len = لڑی_لمبائی(heredoc);

	close(redir->rd_fd); /* often saves dup2+close in xmove_fd */
	xpiped_pair(pair);
	xmove_fd(pair.rd, redir->rd_fd);

	/* Try writing without forking. Newer kernels have
	 * dynamically growing pipes. Must use non-blocking write! */
	ndelay_on(pair.wr);
	جب (1) {
		written = اكتب(pair.wr, heredoc, len);
		اگر (written <= 0)
			توڑو;
		len -= written;
		اگر (len == 0) {
			close(pair.wr);
			آزاد(expanded);
			واپس;
		}
		heredoc += written;
	}
	ndelay_off(pair.wr);

	/* Okay, pipe buffer was not big enough */
	/* Note: we must not create a stray child (bastard? :)
	 * for the unsuspecting parent process. Child creates a grandchild
	 * and exits before parent execs the process which consumes heredoc
	 * (that exec happens after we return from this function) */
#اگر !BB_MMU
	مقصد_free = تہی;
#اگر_ختم
	pid = xvfork();
	اگر (pid == 0) {
		/*بچہ*/
		disable_restore_tty_pgrp_on_exit();
		pid = BB_MMU ? xfork() : xvfork();
		اگر (pid != 0)
			_exit(0);
		/* grandchild */
		close(redir->rd_fd); /* read side of the pipe */
#اگر BB_MMU
		بھرا_write(pair.wr, heredoc, len); /* may loop or block */
		_exit(0);
#ورنہ
		/* Delegate blocking writes to another process */
		xmove_fd(pair.wr, STDOUT_FILENO);
		re_execute_shell(&مقصد_free, heredoc, تہی, تہی, تہی);
#اگر_ختم
	}
	/*والد*/
#اگر ENABLE_HUSH_FAST
	G.گنتی_SIGCHLD++;
//bb_error_msg("[%d] fork in setup_heredoc: G.count_SIGCHLD:%d G.handled_SIGCHLD:%d", getpid(), G.count_SIGCHLD, G.handled_SIGCHLD);
#اگر_ختم
	enable_restore_tty_pgrp_on_exit();
#اگر !BB_MMU
	آزاد(مقصد_free);
#اگر_ختم
	close(pair.wr);
	آزاد(expanded);
	wait(تہی); /* wait till child has died */
}

ساخت squirrel {
	صحیح orig_fd;
	صحیح moved_مقصد;
	/* moved_to = n: fd was moved to n; restore back to orig_fd after redir */
	/* moved_to = -1: fd was opened by redirect; close orig_fd after redir */
};

ساکن ساخت squirrel *append_squirrel(ساخت squirrel *sq, صحیح i, صحیح orig, صحیح moved)
{
	sq = xrealloc(sq, (i + 2) * حجم(sq[0]));
	sq[i].orig_fd = orig;
	sq[i].moved_مقصد = moved;
	sq[i+1].orig_fd = -1; /* end marker */
	واپس sq;
}

ساکن ساخت squirrel *جوڑو_squirrel(ساخت squirrel *sq, صحیح fd, صحیح avoid_fd)
{
	صحیح moved_مقصد;
	صحیح i;

	i = 0;
	اگر (sq) برائے (; sq[i].orig_fd >= 0; i++) {
		/* If we collide with an already moved fd... */
		اگر (fd == sq[i].moved_مقصد) {
			moved_مقصد = dup_CLOEXEC(sq[i].moved_مقصد, avoid_fd);
			debug_printf_redir("redirect_fd %d: already busy, moving to %d\n", fd, moved_مقصد);
			اگر (moved_مقصد < 0) {
				/* "echo 2>/dev/tty 10>&9999" testcase:
				 * We move fd 2 to 10, then discover we need to move fd 10
				 * (and not hit 9999) and the latter fails.
				 */
				واپس تہی; /* fcntl failed */
			}
			sq[i].moved_مقصد = moved_مقصد;
			واپس sq;
		}
		اگر (fd == sq[i].orig_fd) {
			/* Example: echo Hello >/dev/null 1>&2 */
			debug_printf_redir("redirect_fd %d: already moved\n", fd);
			واپس sq;
		}
	}

	/* If this fd is open, we move and remember it; if it's closed, moved_to = -1 */
	moved_مقصد = dup_CLOEXEC(fd, avoid_fd);
	debug_printf_redir("redirect_fd %d: previous fd is moved to %d (-1 if it was closed)\n", fd, moved_مقصد);
	اگر (moved_مقصد < 0 && errno != EBADF)
		واپس تہی; /* fcntl failed (not because fd is closed) */
	واپس append_squirrel(sq, i, fd, moved_مقصد);
}

ساکن ساخت squirrel *جوڑو_squirrel_closed(ساخت squirrel *sq, صحیح fd)
{
	صحیح i;

	i = 0;
	اگر (sq) برائے (; sq[i].orig_fd >= 0; i++) {
		/* If we collide with an already moved fd... */
		اگر (fd == sq[i].orig_fd) {
			/* Examples:
			 * "echo 3>FILE 3>&- 3>FILE"
			 * "echo 3>&- 3>FILE"
			 * No need for last redirect to insert
			 * another "need to close 3" indicator.
			 */
			debug_printf_redir("redirect_fd %d: already moved or closed\n", fd);
			واپس sq;
		}
	}

	debug_printf_redir("redirect_fd %d: previous fd was closed\n", fd);
	واپس append_squirrel(sq, i, fd, -1);
}

/* fd: redirect wants this fd to be used (e.g. 3>file).
 * Move all conflicting internally used fds,
 * and remember them so that we can restore them later.
 */
ساکن صحیح save_fd_on_redirect(صحیح fd, صحیح avoid_fd, ساخت squirrel **sqp)
{
	ساخت squirrel *new_squirrel;

	اگر (avoid_fd < 9) /* the important case here is that it can be -1 */
		avoid_fd = 9;

#اگر ENABLE_HUSH_INTERACTIVE
	اگر (fd != 0 /* don't trigger for G_interactive_fd == 0 (that's "not interactive" flag) */
	 && fd == G_interactive_fd
	) {
		/* Testcase: "ls -l /proc/$$/fd 255>&-" should work */
		G_interactive_fd = xdup_CLOEXEC_and_close(G_interactive_fd, avoid_fd);
		debug_printf_redir("redirect_fd %d: matches interactive_fd, moving it to %d\n", fd, G_interactive_fd);
		واپس 1; /* "we closed fd" */
	}
#اگر_ختم
	/* Are we called from setup_redirects(squirrel==NULL)
	 * in redirect in a [v]forked child?
	 */
	اگر (sqp == تہی) {
		/* No need to move script fds.
		 * For NOMMU case, it's actively wrong: we'd change ->fd
		 * fields in memory for the parent, but parent's fds
		 * aren't moved, it would use wrong fd!
		 * Reproducer: "cmd 3>FILE" in script.
		 * If we would call move_HFILEs_on_redirect(), child would:
		 *  fcntl64(3, F_DUPFD_CLOEXEC, 10)   = 10
		 *  close(3)                          = 0
		 * and change ->fd to 10 if fd#3 is a script fd. WRONG.
		 */
		//bb_error_msg("sqp == NULL: [v]forked child");
		واپس 0;
	}

	/* If this one of script's fds? */
	اگر (move_HFILEs_on_redirect(fd, avoid_fd))
		واپس 1; /* yes. "we closed fd" (actually moved it) */

	/* Are we called for "exec 3>FILE"? Came through
	 * redirect_and_varexp_helper(squirrel=ERR_PTR) -> setup_redirects(ERR_PTR)
	 * This case used to fail for this script:
	 *  exec 3>FILE
	 *  echo Ok
	 *  ...100000 more lines...
	 *  echo Ok
	 * as follows:
	 *  read(3, "exec 3>FILE\necho Ok\necho Ok"..., 1024) = 1024
	 *  open("FILE", O_WRONLY|O_CREAT|O_TRUNC|O_LARGEFILE, 0666) = 4
	 *  dup2(4, 3)                        = 3
	 *  ^^^^^^^^ oops, we lost fd#3 opened to our script!
	 *  close(4)                          = 0
	 *  write(1, "Ok\n", 3)               = 3
	 *  ...                               = 3
	 *  write(1, "Ok\n", 3)               = 3
	 *  read(3, 0x94fbc08, 1024)          = -1 EBADF (Bad file descriptor)
	 *  ^^^^^^^^ oops, wrong fd!!!
	 * With this case separate from sqp == NULL and *after* move_HFILEs,
	 * it now works:
	 */
	اگر (sqp == ERR_PTR) {
		/* Don't preserve redirected fds: exec is _meant_ to change these */
		//bb_error_msg("sqp == ERR_PTR: exec >FILE");
		واپس 0;
	}

	/* Check whether it collides with any open fds (e.g. stdio), save fds as needed */
	new_squirrel = جوڑو_squirrel(*sqp, fd, avoid_fd);
	اگر (!new_squirrel)
		واپس -1; /* redirect error */
	*sqp = new_squirrel;
	واپس 0; /* "we did not close fd" */
}

ساکن خالی restore_redirects(ساخت squirrel *sq)
{
	اگر (sq) {
		صحیح i;
		برائے (i = 0; sq[i].orig_fd >= 0; i++) {
			اگر (sq[i].moved_مقصد >= 0) {
				/* We simply die on error */
				debug_printf_redir("restoring redirected fd from %d to %d\n", sq[i].moved_مقصد, sq[i].orig_fd);
				xmove_fd(sq[i].moved_مقصد, sq[i].orig_fd);
			} ورنہ {
				/* cmd1 9>FILE; cmd2_should_see_fd9_closed */
				debug_printf_redir("restoring redirected fd %d: closing it\n", sq[i].orig_fd);
				close(sq[i].orig_fd);
			}
		}
		آزاد(sq);
	}
	اگر (G.HFILE_stdin
	 && G.HFILE_stdin->fd > STDIN_FILENO
	/* we compare > STDIN, not == STDIN, since hfgetc()
	 * closes fd and sets ->fd to -1 if EOF is reached.
	 * Testcase: echo 'pwd' | hush
	 */
	) {
		/* Testcase: interactive "read r <FILE; echo $r; read r; echo $r".
		 * Redirect moves ->fd to e.g. 10,
		 * and it is not restored above (we do not restore script fds
		 * after redirects, we just use new, "moved" fds).
		 * However for stdin, get_user_input() -> read_line_input(),
		 * and read builtin, depend on fd == STDIN_FILENO.
		 */
		debug_printf_redir("restoring %d to stdin\n", G.HFILE_stdin->fd);
		xmove_fd(G.HFILE_stdin->fd, STDIN_FILENO);
		G.HFILE_stdin->fd = STDIN_FILENO;
	}

	/* If moved, G_interactive_fd stays on new fd, not restoring it */
}

#اگر ENABLE_FEATURE_SH_STANDALONE && BB_MMU
ساکن خالی close_saved_fds_and_FILE_fds(خالی)
{
	اگر (G_interactive_fd)
		close(G_interactive_fd);
	close_all_HFILE_list();
}
#اگر_ختم

ساکن صحیح internally_opened_fd(صحیح fd, ساخت squirrel *sq)
{
	صحیح i;

#اگر ENABLE_HUSH_INTERACTIVE
	اگر (fd == G_interactive_fd)
		واپس 1;
#اگر_ختم
	/* If this one of script's fds? */
	اگر (fd_اندر_HFILEs(fd))
		واپس 1;

	اگر (sq) برائے (i = 0; sq[i].orig_fd >= 0; i++) {
		اگر (fd == sq[i].moved_مقصد)
			واپس 1;
	}
	واپس 0;
}

/* sqp != NULL means we squirrel away copies of stdin, stdout,
 * and stderr if they are redirected.
 * If redirection fails, return 1. This will make caller
 * skip command execution and restore already created redirect fds.
 */
ساکن صحیح تیاری_redirects(ساخت command *prog, ساخت squirrel **sqp)
{
	ساخت redir_struct *redir;

	برائے (redir = prog->redirects; redir; redir = redir->اگلا) {
		صحیح newfd;
		صحیح closed;

		اگر (redir->rd_type == REDIRECT_HEREDOC2) {
			/* "rd_fd<<HERE" case */
			اگر (save_fd_on_redirect(redir->rd_fd, /*avoid:*/ 0, sqp) < 0)
				واپس 1;
			/* for REDIRECT_HEREDOC2, rd_filename holds _contents_
			 * of the heredoc */
			debug_printf_redir("set heredoc '%s'\n",
					redir->rd_filename);
			تیاری_heredoc(redir);
			جاری;
		}

		اگر (redir->rd_dup == REDIRFD_مقصد_FILE) {
			/* "rd_fd<*>file" case (<*> is <,>,>>,<>) */
			حرف *p;
			صحیح منوال;

			اگر (redir->rd_filename == تہی) {
				/* Examples:
				 * "cmd >" (no filename)
				 * "cmd > <file" (2nd redirect starts too early)
				 */
				syntax_error("invalid redirect");
				واپس 1;
			}
			منوال = redir_جدول[redir->rd_type].منوال;
			p = expand_string_مقصد_string(redir->rd_filename,
				EXP_FLAG_ESC_GLOB_CHARS, /*unbackslash:*/ 1);
			newfd = open_or_warn(p, منوال);
			آزاد(p);
			اگر (newfd < 0) {
				/* Error message from open_or_warn can be lost
				 * if stderr has been redirected, but bash
				 * and ash both lose it as well
				 * (though zsh doesn't!)
				 */
				واپس 1;
			}
			اگر (newfd == redir->rd_fd && sqp
			 && sqp != ERR_PTR /* not a redirect in "exec" */
			) {
				/* open() gave us precisely the fd we wanted.
				 * This means that this fd was not busy
				 * (not opened to anywhere).
				 * Remember to close it on restore:
				 */
				*sqp = جوڑو_squirrel_closed(*sqp, newfd);
				debug_printf_redir("redir to previously closed fd %d\n", newfd);
			}
		} ورنہ {
			/* "rd_fd>&rd_dup" or "rd_fd>&-" case */
			newfd = redir->rd_dup;
		}

		اگر (newfd == redir->rd_fd)
			جاری;

		/* if "N>FILE": move newfd to redir->rd_fd */
		/* if "N>&M": dup newfd to redir->rd_fd */
		/* if "N>&-": close redir->rd_fd (newfd is REDIRFD_CLOSE) */

		closed = save_fd_on_redirect(redir->rd_fd, /*avoid:*/ newfd, sqp);
		اگر (closed < 0)
			واپس 1; /* error */
		اگر (newfd == REDIRFD_CLOSE) {
			/* "N>&-" means "close me" */
			اگر (!closed) {
				/* ^^^ optimization: saving may already
				 * have closed it. If not... */
				close(redir->rd_fd);
			}
			/* Sometimes we do another close on restore, getting EBADF.
			 * Consider "echo 3>FILE 3>&-"
			 * first redirect remembers "need to close 3",
			 * and second redirect closes 3! Restore code then closes 3 again.
			 */
		} ورنہ {
			/* if newfd is a script fd or saved fd, do not allow to use it */
			اگر (internally_opened_fd(newfd, sqp && sqp != ERR_PTR ? *sqp : تہی)) {
				bb_error_پیغام("fd#%d is not open", newfd);
				واپس 1;
			}
			اگر (dup2(newfd, redir->rd_fd) < 0) {
				/* "echo >&99" testcase */
				bb_perror_پیغام("dup2(%d,%d)", newfd, redir->rd_fd);
				واپس 1;
			}
			اگر (redir->rd_dup == REDIRFD_مقصد_FILE)
				/* "rd_fd > FILE" */
				close(newfd);
			/* else: "rd_fd > rd_dup" */
		}
	}
	واپس 0;
}

ساکن حرف *ڈھونڈو_اندر_راستہ(ثابت حرف *arg)
{
	حرف *ret = تہی;
	ثابت حرف *راستہ = get_local_var_قدر("PATH");

	اگر (!راستہ)
		واپس تہی;

	جب (1) {
		ثابت حرف *انتہا_2 = strchrnul(راستہ, ':');
		صحیح sz = انتہا_2 - راستہ; /* must be int! */

		آزاد(ret);
		اگر (sz != 0) {
			ret = xasprintf("%.*s/%s", sz, راستہ, arg);
		} ورنہ {
			/* We have xxx::yyyy in $PATH,
			 * it means "use current dir" */
			ret = xstrdup(arg);
		}
		اگر (access(ret, F_OK) == 0)
			توڑو;

		اگر (*انتہا_2 == '\0') {
			آزاد(ret);
			واپس تہی;
		}
		راستہ = انتہا_2 + 1;
	}

	واپس ret;
}

ساکن ثابت ساخت built_اندر_command *ڈھونڈو_builtin_helper(ثابت حرف *name,
		ثابت ساخت built_اندر_command *x,
		ثابت ساخت built_اندر_command *انتہا_2)
{
	جب (x != انتہا_2) {
		اگر (strcmp(name, x->b_cmd) != 0) {
			x++;
			جاری;
		}
		debug_printf_exec("found builtin '%s'\n", name);
		واپس x;
	}
	واپس تہی;
}
ساکن ثابت ساخت built_اندر_command *ڈھونڈو_builtin1(ثابت حرف *name)
{
	واپس ڈھونڈو_builtin_helper(name, bltins1, &bltins1[ARRAY_سائز(bltins1)]);
}
ساکن ثابت ساخت built_اندر_command *ڈھونڈو_builtin(ثابت حرف *name)
{
	ثابت ساخت built_اندر_command *x = ڈھونڈو_builtin1(name);
	اگر (x)
		واپس x;
	واپس ڈھونڈو_builtin_helper(name, bltins2, &bltins2[ARRAY_سائز(bltins2)]);
}

#اگر ENABLE_HUSH_کام && ENABLE_FEATURE_TAB_COMPLETION
ساکن ثابت حرف * FAST_FUNC hush_command_name(صحیح i)
{
	اگر (/*i >= 0 && */ i < ARRAY_سائز(bltins1)) {
		واپس bltins1[i].b_cmd;
	}
	i -= ARRAY_سائز(bltins1);
	اگر (i < ARRAY_سائز(bltins2)) {
		واپس bltins2[i].b_cmd;
	}
# اگر ENABLE_HUSH_FUNCTIONS
	{
		ساخت function *funcp;
		i -= ARRAY_سائز(bltins2);
		برائے (funcp = G.چوٹی_func; funcp; funcp = funcp->اگلا) {
			اگر (--i < 0)
				واپس funcp->name;
		}
	}
# endif
	واپس تہی;
}
#اگر_ختم
#اگر_ختم /* !__U_BOOT__ */

#اگر_غیر_تعریف_شدہ __U_BOOT__
ساکن خالی remove_nested_vars(خالی)
{
	ساخت variable *cur;
	ساخت variable **cur_pp;

	cur_pp = &G.چوٹی_var;
	جب ((cur = *cur_pp) != تہی) {
		اگر (cur->var_nest_level <= G.var_nest_level) {
			cur_pp = &cur->اگلا;
			جاری;
		}
		/* Unexport */
		اگر (cur->flg_export) {
			debug_printf_env("unexporting nested '%s'/%u\n", cur->varstr, cur->var_nest_level);
			bb_unsetenv(cur->varstr);
		}
		/* Remove from global list */
		*cur_pp = cur->اگلا;
		/* Free */
		اگر (!cur->max_len) {
			debug_printf_env("freeing nested '%s'/%u\n", cur->varstr, cur->var_nest_level);
			آزاد(cur->varstr);
		}
		آزاد(cur);
	}
}

ساکن خالی enter_var_nest_level(خالی)
{
	G.var_nest_level++;
	debug_printf_env("var_nest_level++ %u\n", G.var_nest_level);

	/* Try:	f() { echo -n .; f; }; f
	 * struct variable::var_nest_level is uint16_t,
	 * thus limiting recursion to < 2^16.
	 * In any case, with 8 Mbyte stack SEGV happens
	 * not too long after 2^16 recursions anyway.
	 */
	اگر (G.var_nest_level > 0xff00)
		bb_error_پیغام_and_die("fatal recursion (depth %u)", G.var_nest_level);
}

ساکن خالی leave_var_nest_level(خالی)
{
	G.var_nest_level--;
	debug_printf_env("var_nest_level-- %u\n", G.var_nest_level);
	اگر (HUSH_DEBUG && (صحیح)G.var_nest_level < 0)
		bb_simple_error_پیغام_and_die("BUG: nesting underflow");

	remove_nested_vars();
}
#اگر_ختم /* __U_BOOT__ */

#اگر ENABLE_HUSH_FUNCTIONS
ساکن ساخت function **ڈھونڈو_function_خانہ(ثابت حرف *name)
{
	ساخت function *funcp;
	ساخت function **funcpp = &G.چوٹی_func;

	جب ((funcp = *funcpp) != تہی) {
		اگر (strcmp(name, funcp->name) == 0) {
			debug_printf_exec("found function '%s'\n", name);
			توڑو;
		}
		funcpp = &funcp->اگلا;
	}
	واپس funcpp;
}

ساکن ALWAYS_INLINE ثابت ساخت function *ڈھونڈو_function(ثابت حرف *name)
{
	ثابت ساخت function *funcp = *ڈھونڈو_function_خانہ(name);
	واپس funcp;
}

/* Note: takes ownership on name ptr */
ساکن ساخت function *new_function(حرف *name)
{
	ساخت function **funcpp = ڈھونڈو_function_خانہ(name);
	ساخت function *funcp = *funcpp;

	اگر (funcp != تہی) {
		ساخت command *cmd = funcp->والد_cmd;
		debug_printf_exec("func %p parent_cmd %p\n", funcp, cmd);
		اگر (!cmd) {
			debug_printf_exec("freeing & replacing function '%s'\n", funcp->name);
			آزاد(funcp->name);
			/* Note: if !funcp->body, do not free body_as_string!
			 * This is a special case of "-F name body" function:
			 * body_as_string was not malloced! */
			اگر (funcp->body) {
				free_pipe_list(funcp->body);
# اگر !BB_MMU
				آزاد(funcp->body_as_string);
# endif
			}
		} ورنہ {
			debug_printf_exec("reinserting in tree & replacing function '%s'\n", funcp->name);
			cmd->argv[0] = funcp->name;
			cmd->گروہ = funcp->body;
# اگر !BB_MMU
			cmd->گروہ_as_string = funcp->body_as_string;
# endif
		}
	} ورنہ {
		debug_printf_exec("remembering new function '%s'\n", name);
		funcp = *funcpp = xzalloc(حجم(*funcp));
		/*funcp->next = NULL;*/
	}

	funcp->name = name;
	واپس funcp;
}

# اگر ENABLE_HUSH_UNSET
ساکن خالی unset_func(ثابت حرف *name)
{
	ساخت function **funcpp = ڈھونڈو_function_خانہ(name);
	ساخت function *funcp = *funcpp;

	اگر (funcp != تہی) {
		debug_printf_exec("freeing function '%s'\n", funcp->name);
		*funcpp = funcp->اگلا;
		/* funcp is unlinked now, deleting it.
		 * Note: if !funcp->body, the function was created by
		 * "-F name body", do not free ->body_as_string
		 * and ->name as they were not malloced. */
		اگر (funcp->body) {
			free_pipe_list(funcp->body);
			آزاد(funcp->name);
#  اگر !BB_MMU
			آزاد(funcp->body_as_string);
#  endif
		}
		آزاد(funcp);
	}
}
# endif

# اگر BB_MMU
#تعریف_کرو exec_function(مقصد_free, funcp, argv) \
	exec_function(funcp, argv)
# endif
ساکن خالی exec_function(حرف ***مقصد_free,
		ثابت ساخت function *funcp,
		حرف **argv) NORETURN;
ساکن خالی exec_function(حرف ***مقصد_free,
		ثابت ساخت function *funcp,
		حرف **argv)
{
# اگر BB_MMU
	صحیح n;

	argv[0] = G.global_argv[0];
	G.global_argv = argv;
	G.global_argc = n = 1 + string_array_len(argv + 1);

// Example when we are here: "cmd | func"
// func will run with saved-redirect fds open.
// $ f() { echo /proc/self/fd/*; }
// $ true | f
// /proc/self/fd/0 /proc/self/fd/1 /proc/self/fd/2 /proc/self/fd/255 /proc/self/fd/3
// stdio^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ G_interactive_fd^ DIR fd for glob
// Same in script:
// $ . ./SCRIPT
// /proc/self/fd/0 /proc/self/fd/1 /proc/self/fd/2 /proc/self/fd/255 /proc/self/fd/3 /proc/self/fd/4
// stdio^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ G_interactive_fd^ opened ./SCRIPT DIR fd for glob
// They are CLOEXEC so external programs won't see them, but
// for "more correctness" we might want to close those extra fds here:
//?	close_saved_fds_and_FILE_fds();

	/* "we are in a function, ok to use return" */
	G_flag_return_اندر_پیش_رفت = -1;
	enter_var_nest_level();
	IF_HUSH_LOCAL(G.func_nest_level++;)

	/* On MMU, funcp->body is always non-NULL */
	n = تسلسل_list(funcp->body);
	_exit(n);
# ورنہ
//?	close_saved_fds_and_FILE_fds();

//TODO: check whether "true | func_with_return" works

	re_execute_shell(مقصد_free,
			funcp->body_as_string,
			G.global_argv[0],
			argv + 1,
			تہی);
# endif
}

ساکن صحیح تسلسل_function(ثابت ساخت function *funcp, حرف **argv)
{
	صحیح rc;
	save_arg_t sv;
	smallint sv_flg;

	save_and_replace_G_دلائل(&sv, argv);

	/* "We are in function, ok to use return" */
	sv_flg = G_flag_return_اندر_پیش_رفت;
	G_flag_return_اندر_پیش_رفت = -1;

	/* Make "local" variables properly shadow previous ones */
	IF_HUSH_LOCAL(enter_var_nest_level();)
	IF_HUSH_LOCAL(G.func_nest_level++;)

	/* On MMU, funcp->body is always non-NULL */
# اگر !BB_MMU
	اگر (!funcp->body) {
		/* Function defined by -F */
		parse_and_تسلسل_string(funcp->body_as_string);
		rc = G.آخری_exitcode;
	} ورنہ
# endif
	{
		rc = تسلسل_list(funcp->body);
	}

	IF_HUSH_LOCAL(G.func_nest_level--;)
	IF_HUSH_LOCAL(leave_var_nest_level();)

	G_flag_return_اندر_پیش_رفت = sv_flg;
# اگر ENABLE_HUSH_TRAP
	debug_printf_exec("G.return_exitcode=-1\n");
	G.return_exitcode = -1; /* invalidate stashed return value */
# endif

	restore_G_دلائل(&sv, argv);

	واپس rc;
}
#اگر_ختم /* ENABLE_HUSH_FUNCTIONS */

#اگر_غیر_تعریف_شدہ __U_BOOT__
#اگر BB_MMU
#تعریف_کرو exec_builtin(مقصد_free, x, argv) \
	exec_builtin(x, argv)
#ورنہ
#تعریف_کرو exec_builtin(مقصد_free, x, argv) \
	exec_builtin(مقصد_free, argv)
#اگر_ختم
ساکن خالی exec_builtin(حرف ***مقصد_free,
		ثابت ساخت built_اندر_command *x,
		حرف **argv) NORETURN;
ساکن خالی exec_builtin(حرف ***مقصد_free,
		ثابت ساخت built_اندر_command *x,
		حرف **argv)
{
#اگر BB_MMU
	صحیح rcode;
//?	close_saved_fds_and_FILE_fds();
	rcode = x->b_function(argv);
	fflush_all();
	_exit(rcode);
#ورنہ
	fflush_all();
	/* On NOMMU, we must never block!
	 * Example: { sleep 99 | read line; } & echo Ok
	 */
	re_execute_shell(مقصد_free,
			argv[0],
			G.global_argv[0],
			G.global_argv + 1,
			argv);
#اگر_ختم
}
#اگر_ختم /* !__U_BOOT__ */

#اگر_غیر_تعریف_شدہ __U_BOOT__
ساکن خالی execvp_or_die(حرف **argv) NORETURN;
ساکن خالی execvp_or_die(حرف **argv)
{
	صحیح e;
	debug_printf_exec("execing '%s'\n", argv[0]);
	/* Don't propagate SIG_IGN to the child */
	اگر (SPECIAL_JOBSTOP_SIGS != 0)
		switch_off_special_sigs(G.special_sig_نقاب & SPECIAL_JOBSTOP_SIGS);
	execvp(argv[0], argv);
	e = 2;
	اگر (errno == EACCES) e = 126;
	اگر (errno == ENOENT) e = 127;
	bb_perror_پیغام("can't execute '%s'", argv[0]);
	_exit(e);
}

#اگر ENABLE_HUSH_منوال_X
ساکن خالی x_منوال_print_optionally_squoted(ثابت حرف *str)
{
	غیرعلامتی len;
	ثابت حرف *cp;

	cp = str;

	/* the set of chars which-cause-string-to-be-squoted mimics bash */
	/* test a char with: bash -c 'set -x; echo "CH"' */
	اگر (str[strcspn(str, "\\\"'`$(){}[]<>;#&|~*?!^"
			" " "\001\002\003\004\005\006\007"
			"\010\011\012\013\014\015\016\017"
			"\020\021\022\023\024\025\026\027"
			"\030\031\032\033\034\035\036\037"
			)
		] == '\0'
	) {
		/* string has no special chars */
		x_منوال_addstr(str);
		واپس;
	}

	cp = str;
	برائے (;;) {
		/* print '....' up to EOL or first squote */
		len = (صحیح)(strchrnul(cp, '\'') - cp);
		اگر (len != 0) {
			x_منوال_addchr('\'');
			x_منوال_addblock(cp, len);
			x_منوال_addchr('\'');
			cp += len;
		}
		اگر (*cp == '\0')
			توڑو;
		/* string contains squote(s), print them as \' */
		x_منوال_addchr('\\');
		x_منوال_addchr('\'');
		cp++;
	}
}
ساکن خالی dump_cmd_اندر_x_منوال(حرف **argv)
{
	اگر (G_x_منوال && argv) {
		غیرعلامتی n;

		/* "+[+++...][ cmd...]\n\0" */
		x_منوال_سابقہ();
		n = 0;
		جب (argv[n]) {
			x_منوال_addchr(' ');
			اگر (argv[n][0] == '\0') {
				x_منوال_addchr('\'');
				x_منوال_addchr('\'');
			} ورنہ {
				x_منوال_print_optionally_squoted(argv[n]);
			}
			n++;
		}
		x_منوال_flush();
	}
}
#ورنہ
# define dump_cmd_اندر_x_منوال(argv) ((خالی)0)
#اگر_ختم
#اگر_ختم /* !__U_BOOT__ */

#اگر_غیر_تعریف_شدہ __U_BOOT__
#اگر ENABLE_HUSH_COMMAND
ساکن خالی if_command_vV_print_and_exit(حرف opt_vV, حرف *cmd, ثابت حرف *explanation)
{
	حرف *مقصد_free;

	اگر (!opt_vV)
		واپس;

	مقصد_free = تہی;
	اگر (!explanation) {
		حرف *راستہ_2 = getenv("PATH");
		explanation = مقصد_free = ڈھونڈو_executable(cmd, &راستہ_2); /* path == NULL is ok */
		اگر (!explanation)
			_exit(1); /* PROG was not found */
		اگر (opt_vV != 'V')
			cmd = مقصد_free; /* -v PROG prints "/path/to/PROG" */
	}
	لکھو((opt_vV == 'V') ? "%s is %s\n" : "%s\n", cmd, explanation);
	آزاد(مقصد_free);
	fflush_all();
	_exit(0);
}
#ورنہ
# define if_command_vV_print_and_exit(a,b,c) ((خالی)0)
#اگر_ختم
#اگر_ختم /* !__U_BOOT__ */

#اگر BB_MMU
#تعریف_کرو pseudo_exec_argv(nommu_save, argv, assignment_cnt, argv_expanded) \
	pseudo_exec_argv(argv, assignment_cnt, argv_expanded)
#تعریف_کرو pseudo_exec(nommu_save, command, argv_expanded) \
	pseudo_exec(command, argv_expanded)
#اگر_ختم

#اگر_غیر_تعریف_شدہ __U_BOOT__
/* Called after [v]fork() in run_pipe, or from builtin_exec.
 * Never returns.
 * Don't exit() here.  If you don't exec, use _exit instead.
 * The at_exit handlers apparently confuse the calling process,
 * in particular stdin handling. Not sure why? -- because of vfork! (vda)
 */
ساکن خالی pseudo_exec_argv(nommu_save_t *nommu_save,
		حرف **argv, صحیح assignment_cnt,
		حرف **argv_expanded) NORETURN;
ساکن NOINLINE خالی pseudo_exec_argv(nommu_save_t *nommu_save,
		حرف **argv, صحیح assignment_cnt,
		حرف **argv_expanded)
{
	ثابت ساخت built_اندر_command *x;
	ساخت variable **sv_shadowed;
	حرف **new_env;
	IF_HUSH_COMMAND(حرف opt_vV = 0;)
	IF_HUSH_FUNCTIONS(ثابت ساخت function *funcp;)

	new_env = expand_assignments(argv, assignment_cnt);
	dump_cmd_اندر_x_منوال(new_env);

	اگر (!argv[assignment_cnt]) {
		/* Case when we are here: ... | var=val | ...
		 * (note that we do not exit early, i.e., do not optimize out
		 * expand_assignments(): think about ... | var=`sleep 1` | ...
		 */
		free_لڑیاں(new_env);
		_exit_SUCCESS();
	}

	sv_shadowed = G.shadowed_vars_pp;
#اگر BB_MMU
	G.shadowed_vars_pp = تہی; /* "don't save, free them instead" */
#ورنہ
	G.shadowed_vars_pp = &nommu_save->old_vars;
	G.var_nest_level++;
#اگر_ختم
	سیٹ_vars_and_save_old(new_env);
	G.shadowed_vars_pp = sv_shadowed;

	اگر (argv_expanded) {
		argv = argv_expanded;
	} ورنہ {
		argv = expand_strvec_مقصد_strvec(argv + assignment_cnt);
#اگر !BB_MMU
		nommu_save->argv = argv;
#اگر_ختم
	}
	dump_cmd_اندر_x_منوال(argv);

#اگر ENABLE_FEATURE_SH_STANDALONE || BB_MMU
	اگر (strchr(argv[0], '/') != تہی)
		جاؤ skip;
#اگر_ختم

#اگر ENABLE_HUSH_FUNCTIONS
	/* Check if the command matches any functions (this goes before bltins) */
	funcp = ڈھونڈو_function(argv[0]);
	اگر (funcp)
		exec_function(&nommu_save->argv_from_re_execing, funcp, argv);
#اگر_ختم

#اگر ENABLE_HUSH_COMMAND
	/* "command BAR": run BAR without looking it up among functions
	 * "command -v BAR": print "BAR" or "/path/to/BAR"; or exit 1
	 * "command -V BAR": print "BAR is {a function,a shell builtin,/path/to/BAR}"
	 */
	جب (strcmp(argv[0], "command") == 0 && argv[1]) {
		حرف *p;

		argv++;
		p = *argv;
		اگر (p[0] != '-' || !p[1])
			جاری; /* bash allows "command command command [-OPT] BAR" */

		برائے (;;) {
			p++;
			انتخاب (*p) {
			حالت '\0':
				argv++;
				p = *argv;
				اگر (p[0] != '-' || !p[1])
					جاؤ after_opts;
				جاری; /* next arg is also -opts, process it too */
			حالت 'v':
			حالت 'V':
				opt_vV = *p;
				جاری;
			طےشدہ:
				bb_error_پیغام_and_die("%s: %s: invalid option", "command", argv[0]);
			}
		}
	}
 after_opts:
# اگر ENABLE_HUSH_FUNCTIONS
	اگر (opt_vV && ڈھونڈو_function(argv[0]))
		if_command_vV_print_and_exit(opt_vV, argv[0], "a function");
# endif
#اگر_ختم

	/* Check if the command matches any of the builtins.
	 * Depending on context, this might be redundant.  But it's
	 * easier to waste a few CPU cycles than it is to figure out
	 * if this is one of those cases.
	 */
	/* Why "BB_MMU ? :" difference in logic? -
	 * On NOMMU, it is more expensive to re-execute shell
	 * just in order to run echo or test builtin.
	 * It's better to skip it here and run corresponding
	 * non-builtin later. */
	x = BB_MMU ? ڈھونڈو_builtin(argv[0]) : ڈھونڈو_builtin1(argv[0]);
	اگر (x) {
		if_command_vV_print_and_exit(opt_vV, argv[0], "a shell builtin");
		exec_builtin(&nommu_save->argv_from_re_execing, x, argv);
	}

#اگر ENABLE_FEATURE_SH_STANDALONE
	/* Check if the command matches any busybox applets */
	{
		صحیح a = ڈھونڈو_applet_by_name(argv[0]);
		اگر (a >= 0) {
			if_command_vV_print_and_exit(opt_vV, argv[0], "an applet");
# اگر BB_MMU /* see above why on NOMMU it is not allowed */
			اگر (APPLET_IS_NOEXEC(a)) {
				/* Do not leak open fds from opened script files etc.
				 * Testcase: interactive "ls -l /proc/self/fd"
				 * should not show tty fd open.
				 */
				close_saved_fds_and_FILE_fds();
//FIXME: should also close saved redir fds
//This casuses test failures in
//redir_children_should_not_see_saved_fd_2.tests
//redir_children_should_not_see_saved_fd_3.tests
//if you replace "busybox find" with just "find" in them
				/* Without this, "rm -i FILE" can't be ^C'ed: */
				switch_off_special_sigs(G.special_sig_نقاب);
				debug_printf_exec("running applet '%s'\n", argv[0]);
				تسلسل_noexec_applet_and_exit(a, argv[0], argv);
			}
# endif
			/* Re-exec ourselves */
			debug_printf_exec("re-execing applet '%s'\n", argv[0]);
			/* Don't propagate SIG_IGN to the child */
			اگر (SPECIAL_JOBSTOP_SIGS != 0)
				switch_off_special_sigs(G.special_sig_نقاب & SPECIAL_JOBSTOP_SIGS);
			execv(bb_busybox_exec_راستہ, argv);
			/* If they called chroot or otherwise made the binary no longer
			 * executable, fall through */
		}
	}
#اگر_ختم

#اگر ENABLE_FEATURE_SH_STANDALONE || BB_MMU
 skip:
#اگر_ختم
	if_command_vV_print_and_exit(opt_vV, argv[0], تہی);
	execvp_or_die(argv);
}

/* Called after [v]fork() in run_pipe
 */
ساکن خالی pseudo_exec(nommu_save_t *nommu_save,
		ساخت command *command,
		حرف **argv_expanded) NORETURN;
ساکن خالی pseudo_exec(nommu_save_t *nommu_save,
		ساخت command *command,
		حرف **argv_expanded)
{
#اگر ENABLE_HUSH_FUNCTIONS
	اگر (command->cmd_type == CMD_FUNCDEF) {
		/* Ignore funcdefs in pipes:
		 * true | f() { cmd }
		 */
		_exit(0);
	}
#اگر_ختم

	اگر (command->argv) {
		pseudo_exec_argv(nommu_save, command->argv,
				command->assignment_cnt, argv_expanded);
	}

	اگر (command->گروہ) {
		/* Cases when we are here:
		 * ( list )
		 * { list } &
		 * ... | ( list ) | ...
		 * ... | { list } | ...
		 */
#اگر BB_MMU
		صحیح rcode;
		debug_printf_exec("pseudo_exec: run_list\n");
		reset_traps_مقصد_defaults();
		rcode = تسلسل_list(command->گروہ);
		/* OK to leak memory by not calling free_pipe_list,
		 * since this process is about to exit */
		_exit(rcode);
#ورنہ
		re_execute_shell(&nommu_save->argv_from_re_execing,
				command->گروہ_as_string,
				G.global_argv[0],
				G.global_argv + 1,
				تہی);
#اگر_ختم
	}

	/* Case when we are here: ... | >file */
	debug_printf_exec("pseudo_exec'ed null command\n");
	_exit_SUCCESS();
}

#اگر ENABLE_HUSH_کام
ساکن ثابت حرف *get_cmdtext(ساخت pipe *pi)
{
	حرف **argv;
	حرف *p;
	صحیح len;

	/* This is subtle. ->cmdtext is created only on first backgrounding.
	 * (Think "cat, <ctrl-z>, fg, <ctrl-z>, fg, <ctrl-z>...." here...)
	 * On subsequent bg argv is trashed, but we won't use it */
	اگر (pi->cmdtext)
		واپس pi->cmdtext;

	argv = pi->cmds[0].argv;
	اگر (!argv) {
		pi->cmdtext = xzalloc(1);
		واپس pi->cmdtext;
	}
	len = 0;
	کرو {
		len += لڑی_لمبائی(*argv) + 1;
	} جب (*++argv);
	p = xmalloc(len);
	pi->cmdtext = p;
	argv = pi->cmds[0].argv;
	کرو {
		p = stpcpy(p, *argv);
		*p++ = ' ';
	} جب (*++argv);
	p[-1] = '\0';
	واپس pi->cmdtext;
}

ساکن خالی remove_کام_from_جدول(ساخت pipe *pi)
{
	ساخت pipe *پچھلا_pipe;

	اگر (pi == G.کام_list) {
		G.کام_list = pi->اگلا;
	} ورنہ {
		پچھلا_pipe = G.کام_list;
		جب (پچھلا_pipe->اگلا != pi)
			پچھلا_pipe = پچھلا_pipe->اگلا;
		پچھلا_pipe->اگلا = pi->اگلا;
	}
	G.آخری_jobid = 0;
	اگر (G.کام_list)
		G.آخری_jobid = G.کام_list->jobid;
}

ساکن خالی delete_ختم_شدہ_کام(ساخت pipe *pi)
{
	remove_کام_from_جدول(pi);
	free_pipe(pi);
}

ساکن خالی clean_اوپر_آخری_dead_کام(خالی)
{
	اگر (G.کام_list && !G.کام_list->alive_cmds)
		delete_ختم_شدہ_کام(G.کام_list);
}

ساکن خالی داخل_کرو_کام_into_جدول(ساخت pipe *pi)
{
	ساخت pipe *کام, **jobp;
	صحیح i;

	clean_اوپر_آخری_dead_کام();

	/* Find the end of the list, and find next job ID to use */
	i = 0;
	jobp = &G.کام_list;
	جب ((کام = *jobp) != تہی) {
		اگر (کام->jobid > i)
			i = کام->jobid;
		jobp = &کام->اگلا;
	}
	pi->jobid = i + 1;

	/* Create a new job struct at the end */
	کام = *jobp = xmemdup(pi, حجم(*pi));
	کام->اگلا = تہی;
	کام->cmds = xzalloc(حجم(pi->cmds[0]) * pi->عدد_cmds);
	/* Cannot copy entire pi->cmds[] vector! This causes double frees */
	برائے (i = 0; i < pi->عدد_cmds; i++) {
		کام->cmds[i].pid = pi->cmds[i].pid;
		/* all other fields are not used and stay zero */
	}
	کام->cmdtext = xstrdup(get_cmdtext(pi));

	اگر (G_interactive_fd)
		لکھو("[%u] %u %s\n", کام->jobid, (غیرعلامتی)کام->cmds[0].pid, کام->cmdtext);
	G.آخری_jobid = کام->jobid;
}
#اگر_ختم /*کام*/

ساکن صحیح کام_exited_or_stopped(ساخت pipe *pi)
{
	صحیح rcode, i;

	اگر (pi->alive_cmds != pi->stopped_cmds)
		واپس -1;

	/* All processes in fg pipe have exited or stopped */
	rcode = 0;
	i = pi->عدد_cmds;
	جب (--i >= 0) {
		rcode = pi->cmds[i].cmd_exitcode;
		/* usually last process gives overall exitstatus,
		 * but with "set -o pipefail", last *failed* process does */
		اگر (G.o_opt[OPT_O_PIPEFAIL] == 0 || rcode != 0)
			توڑو;
	}
	IF_HAS_KEYWORDS(اگر (pi->pi_inverted) rcode = !rcode;)
	واپس rcode;
}

ساکن صحیح process_wait_نتیجہ(ساخت pipe *fg_pipe, pid_t childpid, صحیح حالت_2)
{
#اگر ENABLE_HUSH_کام
	ساخت pipe *pi;
#اگر_ختم
	صحیح i, dead;

	dead = WIFEXITED(حالت_2) || WIFSIGNALED(حالت_2);

#اگر DEBUG_JOBS
	اگر (WIFSTOPPED(حالت_2))
		debug_printf_jobs("pid %d stopped by sig %d (exitcode %d)\n",
				childpid, WSTOPSIG(حالت_2), WEXITSTATUS(حالت_2));
	اگر (WIFSIGNALED(حالت_2))
		debug_printf_jobs("pid %d killed by sig %d (exitcode %d)\n",
				childpid, WTERMSIG(حالت_2), WEXITSTATUS(حالت_2));
	اگر (WIFEXITED(حالت_2))
		debug_printf_jobs("pid %d exited, exitcode %d\n",
				childpid, WEXITSTATUS(حالت_2));
#اگر_ختم
	/* Were we asked to wait for a fg pipe? */
	اگر (fg_pipe) {
		i = fg_pipe->عدد_cmds;

		جب (--i >= 0) {
			صحیح rcode;

			debug_printf_jobs("check pid %d\n", fg_pipe->cmds[i].pid);
			اگر (fg_pipe->cmds[i].pid != childpid)
				جاری;
			اگر (dead) {
				صحیح ex;
				fg_pipe->cmds[i].pid = 0;
				fg_pipe->alive_cmds--;
				ex = WEXITSTATUS(حالت_2);
				/* bash prints killer signal's name for *last*
				 * process in pipe (prints just newline for SIGINT/SIGPIPE).
				 * Mimic this. Example: "sleep 5" + (^\ or kill -QUIT)
				 */
				اگر (WIFSIGNALED(حالت_2)) {
					صحیح sig = WTERMSIG(حالت_2);
#اگر ENABLE_HUSH_کام
					اگر (G.تسلسل_list_level == 1
					/* ^^^^^ Do not print in nested contexts, example:
					 * echo `sleep 1; sh -c 'kill -9 $$'` - prints "137", NOT "Killed 137"
					 */
					 && i == fg_pipe->عدد_cmds-1
					) {
						/* strsignal() is for bash compat. ~600 bloat versus bbox's get_signame() */
						puts(sig == SIGINT || sig == SIGPIPE ? "" : strsignal(sig));
					}
#اگر_ختم
					/* TODO: if (WCOREDUMP(status)) + " (core dumped)"; */
					/* MIPS has 128 sigs (1..128), if sig==128,
					 * 128 + sig would result in exitcode 256 -> 0!
					 */
					ex = 128 | sig;
				}
				fg_pipe->cmds[i].cmd_exitcode = ex;
			} ورنہ {
				fg_pipe->stopped_cmds++;
			}
			debug_printf_jobs("fg_pipe: alive_cmds %d stopped_cmds %d\n",
					fg_pipe->alive_cmds, fg_pipe->stopped_cmds);
			rcode = کام_exited_or_stopped(fg_pipe);
			اگر (rcode >= 0) {
/* Note: *non-interactive* bash does not continue if all processes in fg pipe
 * are stopped. Testcase: "cat | cat" in a script (not on command line!)
 * and "killall -STOP cat" */
				اگر (G_interactive_fd) {
#اگر ENABLE_HUSH_کام
					اگر (fg_pipe->alive_cmds != 0)
						داخل_کرو_کام_into_جدول(fg_pipe);
#اگر_ختم
					واپس rcode;
				}
				اگر (fg_pipe->alive_cmds == 0)
					واپس rcode;
			}
			/* There are still running processes in the fg_pipe */
			واپس -1;
		}
		/* It wasn't in fg_pipe, look for process in bg pipes */
	}

#اگر ENABLE_HUSH_کام
	/* We were asked to wait for bg or orphaned children */
	/* No need to remember exitcode in this case */
	برائے (pi = G.کام_list; pi; pi = pi->اگلا) {
		برائے (i = 0; i < pi->عدد_cmds; i++) {
			اگر (pi->cmds[i].pid == childpid)
				جاؤ ملا_pi_and_prognum;
		}
	}
	/* Happens when shell is used as init process (init=/bin/sh) */
	debug_printf("checkjobs: pid %d was not in our list!\n", childpid);
	واپس -1; /* this wasn't a process from fg_pipe */

 ملا_pi_and_prognum:
	اگر (dead) {
		/* child exited */
		صحیح rcode = WEXITSTATUS(حالت_2);
		اگر (WIFSIGNALED(حالت_2))
			/* NB: not 128 + sig, MIPS has sig 128 */
			rcode = 128 | WTERMSIG(حالت_2);
		pi->cmds[i].cmd_exitcode = rcode;
		اگر (G.آخری_bg_pid == pi->cmds[i].pid)
			G.آخری_bg_pid_exitcode = rcode;
		pi->cmds[i].pid = 0;
		pi->alive_cmds--;
		اگر (!pi->alive_cmds) {
# اگر ENABLE_HUSH_BASH_COMPAT
			G.dead_کام_exitcode = کام_exited_or_stopped(pi);
# endif
			اگر (G_interactive_fd) {
				لکھو(کام_حالت_FORMAT, pi->jobid,
						"Done", pi->cmdtext);
				delete_ختم_شدہ_کام(pi);
			} ورنہ {
/*
 * bash deletes finished jobs from job table only in interactive mode,
 * after "jobs" cmd, or if pid of a new process matches one of the old ones
 * (see cleanup_dead_jobs(), delete_old_job(), J_NOTIFIED in bash source).
 * Testcase script: "(exit 3) & sleep 1; wait %1; echo $?" prints 3 in bash.
 * We only retain one "dead" job, if it's the single job on the list.
 * This covers most of real-world scenarios where this is useful.
 */
				اگر (pi != G.کام_list)
					delete_ختم_شدہ_کام(pi);
			}
		}
	} ورنہ {
		/* child stopped */
		pi->stopped_cmds++;
	}
#اگر_ختم
	واپس -1; /* this wasn't a process from fg_pipe */
}

/* Check to see if any processes have exited -- if they have,
 * figure out why and see if a job has completed.
 *
 * If non-NULL fg_pipe: wait for its completion or stop.
 * Return its exitcode or zero if stopped.
 *
 * Alternatively (fg_pipe == NULL, waitfor_pid != 0):
 * waitpid(WNOHANG), if waitfor_pid exits or stops, return exitcode+1,
 * else return <0 if waitpid errors out (e.g. ECHILD: nothing to wait for)
 * or 0 if no children changed status.
 *
 * Alternatively (fg_pipe == NULL, waitfor_pid == 0),
 * return <0 if waitpid errors out (e.g. ECHILD: nothing to wait for)
 * or 0 if no children changed status.
 */
ساکن صحیح checkjobs(ساخت pipe *fg_pipe, pid_t waitfor_pid)
{
	صحیح اوصاف;
	صحیح حالت_2;
	صحیح rcode = 0;

	debug_printf_jobs("checkjobs %p\n", fg_pipe);

	اوصاف = WUNTRACED;
	اگر (fg_pipe == تہی)
		اوصاف |= WNOHANG;

	errno = 0;
#اگر ENABLE_HUSH_FAST
	اگر (G.handled_SIGCHLD == G.گنتی_SIGCHLD) {
//bb_error_msg("[%d] checkjobs: G.count_SIGCHLD:%d G.handled_SIGCHLD:%d children?:%d fg_pipe:%p",
//getpid(), G.count_SIGCHLD, G.handled_SIGCHLD, G.we_have_children, fg_pipe);
		/* There was neither fork nor SIGCHLD since last waitpid */
		/* Avoid doing waitpid syscall if possible */
		اگر (!G.we_have_children) {
			errno = ECHILD;
			واپس -1;
		}
		اگر (fg_pipe == تہی) { /* is WNOHANG set? */
			/* We have children, but they did not exit
			 * or stop yet (we saw no SIGCHLD) */
			واپس 0;
		}
		/* else: !WNOHANG, waitpid will block, can't short-circuit */
	}
#اگر_ختم

/* Do we do this right?
 * bash-3.00# sleep 20 | false
 * <ctrl-Z pressed>
 * [3]+  Stopped          sleep 20 | false
 * bash-3.00# echo $?
 * 1   <========== bg pipe is not fully done, but exitcode is already known!
 * [hush 1.14.0: yes we do it right]
 */
	جب (1) {
		pid_t childpid;
#اگر ENABLE_HUSH_FAST
		صحیح i;
		i = G.گنتی_SIGCHLD;
#اگر_ختم
		childpid = waitpid(-1, &حالت_2, اوصاف);
		اگر (childpid <= 0) {
			اگر (childpid && errno != ECHILD)
				bb_simple_perror_پیغام("waitpid");
#اگر ENABLE_HUSH_FAST
			ورنہ { /* Until next SIGCHLD, waitpid's are useless */
				G.we_have_children = (childpid == 0);
				G.handled_SIGCHLD = i;
//bb_error_msg("[%d] checkjobs: waitpid returned <= 0, G.count_SIGCHLD:%d G.handled_SIGCHLD:%d", getpid(), G.count_SIGCHLD, G.handled_SIGCHLD);
			}
#اگر_ختم
			/* ECHILD (no children), or 0 (no change in children status) */
			rcode = childpid;
			توڑو;
		}
		rcode = process_wait_نتیجہ(fg_pipe, childpid, حالت_2);
		اگر (rcode >= 0) {
			/* fg_pipe exited or stopped */
			توڑو;
		}
		اگر (childpid == waitfor_pid) { /* "wait PID" */
			debug_printf_exec("childpid==waitfor_pid:%d status:0x%08x\n", childpid, حالت_2);
			rcode = WEXITSTATUS(حالت_2);
			اگر (WIFSIGNALED(حالت_2))
				rcode = 128 | WTERMSIG(حالت_2);
			اگر (WIFSTOPPED(حالت_2))
				/* bash: "cmd & wait $!" and cmd stops: $? = 128 | stopsig */
				rcode = 128 | WSTOPSIG(حالت_2);
			rcode++;
			توڑو; /* "wait PID" called us, give it exitcode+1 */
		}
#اگر ENABLE_HUSH_BASH_COMPAT
		اگر (-1 == waitfor_pid /* "wait -n" (wait for any one job) */
		 && G.dead_کام_exitcode >= 0 /* some job did finish */
		) {
			debug_printf_exec("waitfor_pid:-1\n");
			rcode = G.dead_کام_exitcode + 1;
			توڑو;
		}
#اگر_ختم
		/* This wasn't one of our processes, or */
		/* fg_pipe still has running processes, do waitpid again */
	} /* while (waitpid succeeds)... */

	واپس rcode;
}

#اگر ENABLE_HUSH_کام
ساکن صحیح checkjobs_and_fg_shell(ساخت pipe *fg_pipe)
{
	pid_t p;
	صحیح rcode = checkjobs(fg_pipe, 0 /*(no pid to wait for)*/);
	اگر (G_saved_tty_pgrp) {
		/* Job finished, move the shell to the foreground */
		p = getpgrp(); /* our process group id */
		debug_printf_jobs("fg'ing ourself: getpgrp()=%d\n", (صحیح)p);
		tcsetpgrp(G_interactive_fd, p);
	}
	واپس rcode;
}
#اگر_ختم
#اگر_ختم /* !__U_BOOT__ */

/* Start all the jobs, but don't wait for anything to finish.
 * See checkjobs().
 *
 * Return code is normally -1, when the caller has to wait for children
 * to finish to determine the exit status of the pipe.  If the pipe
 * is a simple builtin command, however, the action is done by the
 * time run_pipe returns, and the exit code is provided as the
 * return value.
 *
 * Returns -1 only if started some children. IOW: we have to
 * mask out retvals of builtins etc with 0xff!
 *
 * The only case when we do not need to [v]fork is when the pipe
 * is single, non-backgrounded, non-subshell command. Examples:
 * cmd ; ...   { list } ; ...
 * cmd && ...  { list } && ...
 * cmd || ...  { list } || ...
 * If it is, then we can run cmd as a builtin, NOFORK,
 * or (if SH_STANDALONE) an applet, and we can run the { list }
 * with run_list. If it isn't one of these, we fork and exec cmd.
 *
 * Cases when we must fork:
 * non-single:   cmd | cmd
 * backgrounded: cmd &     { list } &
 * subshell:     ( list ) [&]
 */
ساکن خالی سیٹ_G_ifs(خالی)
{
	/* Testcase: set -- q w e; (IFS='' echo "$*"; IFS=''; echo "$*"); echo "$*"
	 * Result should be 3 lines: q w e, qwe, q w e
	 */
	اگر (G.ifs_whitespace != G.ifs)
		آزاد(G.ifs_whitespace);
	G.ifs = get_local_var_قدر("IFS");
	اگر (G.ifs) {
		حرف *p;
		G.ifs_whitespace = (حرف*)G.ifs;
		p = skip_whitespace(G.ifs);
		اگر (*p) {
			/* Not all $IFS is whitespace */
			حرف *d;
			صحیح len = p - G.ifs;
			p = skip_non_whitespace(p);
			G.ifs_whitespace = xmalloc(len + لڑی_لمبائی(p) + 1); /* can overestimate */
			d = mempcpy(G.ifs_whitespace, G.ifs, len);
			جب (*p) {
				اگر (خلا_ہے(*p))
					*d++ = *p;
				p++;
			}
			*d = '\0';
		}
	} ورنہ {
		G.ifs = defifs;
		G.ifs_whitespace = (حرف*)G.ifs;
	}
}
#اگر_غیر_تعریف_شدہ __U_BOOT__
#اگر !ENABLE_HUSH_منوال_X
#تعریف_کرو redirect_and_varexp_helper(command, sqp, argv_expanded) \
	redirect_and_varexp_helper(command, sqp)
#اگر_ختم
ساکن صحیح redirect_and_varexp_helper(
		ساخت command *command,
		ساخت squirrel **sqp,
		حرف **argv_expanded)
{
	/* Assignments occur before redirects. Try:
	 * a=`sleep 1` sleep 2 3>/qwe/rty
	 */

	حرف **new_env = expand_assignments(command->argv, command->assignment_cnt);
	dump_cmd_اندر_x_منوال(new_env);
	dump_cmd_اندر_x_منوال(argv_expanded);
	/* this takes ownership of new_env[i] elements, and frees new_env: */
	سیٹ_vars_and_save_old(new_env);

	واپس تیاری_redirects(command, sqp);
}
#اگر_ختم /* !__U_BOOT__ */

ساکن NOINLINE صحیح تسلسل_pipe(ساخت pipe *pi)
{
	ساکن ثابت حرف *ثابت null_ptr = تہی;

	صحیح cmd_no;
#اگر_غیر_تعریف_شدہ __U_BOOT__
	صحیح اگلا_infd;
#اگر_ختم /* !__U_BOOT__ */
	ساخت command *command;
	حرف **argv_expanded;
	حرف **argv;
#اگر_غیر_تعریف_شدہ __U_BOOT__
	ساخت squirrel *squirrel = تہی;
#اگر_ختم /* !__U_BOOT__ */
	صحیح rcode;

#اگر_تعریف_شدہ __U_BOOT__
	/*
	 * Set rcode here to avoid returning a garbage value in the middle of
	 * the function.
	 * Also, if an error occurs, rcode value would be changed and last
	 * return will signal the error.
	 */
	rcode = 0;
#اگر_ختم /* __U_BOOT__ */

	debug_printf_exec("run_pipe start: members:%d\n", pi->عدد_cmds);
	debug_enter();

	سیٹ_G_ifs();

#اگر_غیر_تعریف_شدہ __U_BOOT__
	IF_HUSH_کام(pi->pgrp = -1;)
	pi->stopped_cmds = 0;
#اگر_ختم /* !__U_BOOT__ */
	command = &pi->cmds[0];
	argv_expanded = تہی;

#اگر_غیر_تعریف_شدہ __U_BOOT__
	اگر (pi->عدد_cmds != 1
	 || pi->followup == PIPE_BG
	 || command->cmd_type == CMD_SUBSHELL
	) {
		جاؤ must_fork;
	}

	pi->alive_cmds = 1;
#اگر_ختم /* !__U_BOOT__ */

	debug_printf_exec(": group:%p argv:'%s'\n",
		command->گروہ, command->argv ? command->argv[0] : "NONE");

	اگر (command->گروہ) {
#اگر ENABLE_HUSH_FUNCTIONS
		اگر (command->cmd_type == CMD_FUNCDEF) {
			/* "executing" func () { list } */
			ساخت function *funcp;

			funcp = new_function(command->argv[0]);
			/* funcp->name is already set to argv[0] */
			funcp->body = command->گروہ;
# اگر !BB_MMU
			funcp->body_as_string = command->گروہ_as_string;
			command->گروہ_as_string = تہی;
# endif
			command->گروہ = تہی;
			command->argv[0] = تہی;
			debug_printf_exec("cmd %p has child func at %p\n", command, funcp);
			funcp->والد_cmd = command;
			command->بچہ_func = funcp;

			debug_printf_exec("run_pipe: return EXIT_SUCCESS\n");
			debug_leave();
			واپس EXIT_SUCCESS;
		}
#اگر_ختم
		/* { list } */
		debug_printf_exec("non-subshell group\n");
		rcode = 1; /* exitcode if redir failed */
#اگر_غیر_تعریف_شدہ __U_BOOT__
		اگر (تیاری_redirects(command, &squirrel) == 0) {
#اگر_ختم /* !__U_BOOT__ */
			debug_printf_exec(": run_list\n");
//FIXME: we need to pass squirrel down into run_list()
//for SH_STANDALONE case, or else this construct:
// { find /proc/self/fd; true; } >FILE; cmd2
//has no way of closing saved fd#1 for "find",
//and in SH_STANDALONE mode, "find" is not execed,
//therefore CLOEXEC on saved fd does not help.
			rcode = تسلسل_list(command->گروہ) & 0xff;
#اگر_غیر_تعریف_شدہ __U_BOOT__
		}
		restore_redirects(squirrel);
#اگر_ختم /* !__U_BOOT__ */
		IF_HAS_KEYWORDS(اگر (pi->pi_inverted) rcode = !rcode;)
		debug_leave();
		debug_printf_exec("run_pipe: return %d\n", rcode);
		واپس rcode;
	}

	argv = command->argv ? command->argv : (حرف **) &null_ptr;
	{
#اگر_غیر_تعریف_شدہ __U_BOOT__
		ثابت ساخت built_اندر_command *x;
		IF_HUSH_FUNCTIONS(ثابت ساخت function *funcp;)
		IF_NOT_HUSH_FUNCTIONS(شمار { funcp = 0 };)
		ساخت variable **sv_shadowed;
#اگر_ختم /* !__U_BOOT__ */
		ساخت variable *old_vars;

#اگر ENABLE_HUSH_LINENO_VAR
		G.execute_lineno = command->lineno;
#اگر_ختم

		اگر (argv[command->assignment_cnt] == تہی) {
			/* Assignments, but no command.
			 * Ensure redirects take effect (that is, create files).
			 * Try "a=t >file"
			 */
			غیرعلامتی i;
			G.expand_exitcode = 0;
 only_assignments:
#اگر_غیر_تعریف_شدہ __U_BOOT__
			rcode = تیاری_redirects(command, &squirrel);
			restore_redirects(squirrel);
#اگر_ختم /* !__U_BOOT__ */

			/* Set shell variables */
			i = 0;
			جب (i < command->assignment_cnt) {
				حرف *p = expand_string_مقصد_string(argv[i],
						EXP_FLAG_ESC_GLOB_CHARS,
						/*unbackslash:*/ 1
				);
#اگر ENABLE_HUSH_منوال_X
				اگر (G_x_منوال) {
					حرف *eq;
					اگر (i == 0)
						x_منوال_سابقہ();
					x_منوال_addchr(' ');
					eq = strchrnul(p, '=');
					اگر (*eq) eq++;
					x_منوال_addblock(p, (eq - p));
					x_منوال_print_optionally_squoted(eq);
					x_منوال_flush();
				}
#اگر_ختم
				debug_printf_env("set shell var:'%s'->'%s'\n", *argv, p);
#اگر_غیر_تعریف_شدہ __U_BOOT__
				اگر (سیٹ_local_var0(p)) {
#ورنہ /* __U_BOOT__ */
				اگر (سیٹ_local_var_modern(p, /*flag:*/ 0)) {
#اگر_ختم
					/* assignment to readonly var / putenv error? */
					rcode = 1;
				}
				i++;
			}
			/* Redirect error sets $? to 1. Otherwise,
			 * if evaluating assignment value set $?, retain it.
			 * Else, clear $?:
			 *  false; q=`exit 2`; echo $? - should print 2
			 *  false; x=1; echo $? - should print 0
			 * Because of the 2nd case, we can't just use G.last_exitcode.
			 */
			اگر (rcode == 0)
				rcode = G.expand_exitcode;
			IF_HAS_KEYWORDS(اگر (pi->pi_inverted) rcode = !rcode;)
			debug_leave();
			debug_printf_exec("run_pipe: return %d\n", rcode);
			واپس rcode;
		}

		/* Expand the rest into (possibly) many strings each */
#اگر defined(CMD_TEST2_SINGLEWORD_NOGLOB)
		اگر (command->cmd_type == CMD_TEST2_SINGLEWORD_NOGLOB)
			argv_expanded = expand_strvec_مقصد_strvec_singleword_noglob(argv + command->assignment_cnt);
		ورنہ
#اگر_ختم
#اگر defined(CMD_SINGLEWORD_NOGLOB)
		اگر (command->cmd_type == CMD_SINGLEWORD_NOGLOB)
			argv_expanded = expand_strvec_مقصد_strvec_singleword_noglob(argv + command->assignment_cnt);
		ورنہ
#اگر_ختم
			argv_expanded = expand_strvec_مقصد_strvec(argv + command->assignment_cnt);

		/* If someone gives us an empty string: `cmd with empty output` */
		اگر (!argv_expanded[0]) {
			آزاد(argv_expanded);
			/* `false` still has to set exitcode 1 */
			G.expand_exitcode = G.آخری_exitcode;
			جاؤ only_assignments;
		}

		old_vars = تہی;
#اگر_غیر_تعریف_شدہ __U_BOOT__
		sv_shadowed = G.shadowed_vars_pp;

		/* Check if argv[0] matches any functions (this goes before bltins) */
		IF_HUSH_FUNCTIONS(funcp = ڈھونڈو_function(argv_expanded[0]);)
		IF_HUSH_FUNCTIONS(x = تہی;)
		IF_HUSH_FUNCTIONS(اگر (!funcp))
			x = ڈھونڈو_builtin(argv_expanded[0]);
		اگر (x || funcp) {
			اگر (x && x->b_function == builtin_exec && argv_expanded[1] == تہی) {
				debug_printf("exec with redirects only\n");
				/*
				 * Variable assignments are executed, but then "forgotten":
				 *  a=`sleep 1;echo A` exec 3>&-; echo $a
				 * sleeps, but prints nothing.
				 */
				enter_var_nest_level();
				G.shadowed_vars_pp = &old_vars;
				rcode = redirect_and_varexp_helper(command,
					/*squirrel:*/ ERR_PTR,
					argv_expanded
				);
				G.shadowed_vars_pp = sv_shadowed;
				/* rcode=1 can be if redir file can't be opened */

				جاؤ clean_اوپر_and_ret1;
			}

			/* Bump var nesting, or this will leak exported $a:
			 * a=b true; env | grep ^a=
			 */
			enter_var_nest_level();
			/* Collect all variables "shadowed" by helper
			 * (IOW: old vars overridden by "var1=val1 var2=val2 cmd..." syntax)
			 * into old_vars list:
			 */
			G.shadowed_vars_pp = &old_vars;
			rcode = redirect_and_varexp_helper(command, &squirrel, argv_expanded);
			اگر (rcode == 0) {
				اگر (!funcp) {
					/* Do not collect *to old_vars list* vars shadowed
					 * by e.g. "local VAR" builtin (collect them
					 * in the previously nested list instead):
					 * don't want them to be restored immediately
					 * after "local" completes.
					 */
					G.shadowed_vars_pp = sv_shadowed;

					debug_printf_exec(": builtin '%s' '%s'...\n",
						x->b_cmd, argv_expanded[1]);
					fflush_all();
					rcode = x->b_function(argv_expanded) & 0xff;
					fflush_all();
				}
#اگر ENABLE_HUSH_FUNCTIONS
				ورنہ {
					debug_printf_exec(": function '%s' '%s'...\n",
						funcp->name, argv_expanded[1]);
					rcode = تسلسل_function(funcp, argv_expanded) & 0xff;
					/*
					 * But do collect *to old_vars list* vars shadowed
					 * within function execution. To that end, restore
					 * this pointer _after_ function run:
					 */
					G.shadowed_vars_pp = sv_shadowed;
				}
#اگر_ختم
			}
		} ورنہ
		اگر (ENABLE_FEATURE_SH_NOFORK && عدد_APPLETS > 1) {
			صحیح n = ڈھونڈو_applet_by_name(argv_expanded[0]);
			اگر (n < 0 || !APPLET_IS_NOFORK(n))
				جاؤ must_fork;

			enter_var_nest_level();
			/* Collect all variables "shadowed" by helper into old_vars list */
			G.shadowed_vars_pp = &old_vars;
			rcode = redirect_and_varexp_helper(command, &squirrel, argv_expanded);
			G.shadowed_vars_pp = sv_shadowed;

			اگر (rcode == 0) {
				debug_printf_exec(": run_nofork_applet '%s' '%s'...\n",
					argv_expanded[0], argv_expanded[1]);
				/*
				 * Note: signals (^C) can't interrupt here.
				 * We remember them and they will be acted upon
				 * after applet returns.
				 * This makes applets which can run for a long time
				 * and/or wait for user input ineligible for NOFORK:
				 * for example, "yes" or "rm" (rm -i waits for input).
				 */
				rcode = تسلسل_nofork_applet(n, argv_expanded);
			}
		} ورنہ
			جاؤ must_fork;

		restore_redirects(squirrel);
 clean_اوپر_and_ret1:
		leave_var_nest_level();
		جوڑو_vars(old_vars);

		/*
		 * Try "usleep 99999999" + ^C + "echo $?"
		 * with FEATURE_SH_NOFORK=y.
		 */
		اگر (!funcp) {
			/* It was builtin or nofork.
			 * if this would be a real fork/execed program,
			 * it should have died if a fatal sig was received.
			 * But OTOH, there was no separate process,
			 * the sig was sent to _shell_, not to non-existing
			 * child.
			 * Let's just handle ^C only, this one is obvious:
			 * we aren't ok with exitcode 0 when ^C was pressed
			 * during builtin/nofork.
			 */
			اگر (sigismember(&G.pending_سیٹ, SIGINT))
				rcode = 128 | SIGINT;
		}
		آزاد(argv_expanded);
		IF_HAS_KEYWORDS(اگر (pi->pi_inverted) rcode = !rcode;)
		debug_leave();
		debug_printf_exec("run_pipe return %d\n", rcode);
		واپس rcode;
#اگر_ختم /* !__U_BOOT__ */
	}

#اگر_غیر_تعریف_شدہ __U_BOOT__
 must_fork:
	/* NB: argv_expanded may already be created, and that
	 * might include `cmd` runs! Do not rerun it! We *must*
	 * use argv_expanded if it's non-NULL */

	/* Going to fork a child per each pipe member */
	pi->alive_cmds = 0;
	اگلا_infd = 0;
#اگر_ختم /* !__U_BOOT__ */

	cmd_no = 0;
	جب (cmd_no < pi->عدد_cmds) {
#اگر_غیر_تعریف_شدہ __U_BOOT__
		ساخت fd_pair pipefds;
#اگر !BB_MMU
		صحیح sv_var_nest_level = G.var_nest_level;
		متلون nommu_save_t nommu_save;
		nommu_save.old_vars = تہی;
		nommu_save.argv = تہی;
		nommu_save.argv_from_re_execing = تہی;
#اگر_ختم
#اگر_ختم /* !__U_BOOT__ */
		command = &pi->cmds[cmd_no];
		cmd_no++;

#اگر_تعریف_شدہ __U_BOOT__
		/* Replace argv and argc by expanded if it exists. */
		اگر (argv_expanded) {
			/*
			 * We need to save a pointer to argv, we will restore it
			 * later, so it will be freed when pipe is freed.
			 */
			argv = command->argv;

			/*
			 * After expansion, there can be more or less argument, so we need to
			 * update argc, for example:
			 * - More arguments:
			 *   foo='bar quuz'
			 *   echo $foo
			 * - Less arguments:
			 *   echo $foo (if foo was never set)
			 */
			command->argc = list_سائز(argv_expanded);
			command->argv = argv_expanded;
		}
#اگر_ختم /* __U_BOOT__ */
			اگر (command->argv) {
			debug_printf_exec(": pipe member '%s' '%s'...\n",
					command->argv[0], command->argv[1]);
		} ورنہ {
			debug_printf_exec(": pipe member with no argv\n");
		}

#اگر_غیر_تعریف_شدہ __U_BOOT__
		/* pipes are inserted between pairs of commands */
		pipefds.rd = 0;
		pipefds.wr = 1;
		اگر (cmd_no < pi->عدد_cmds)
			xpiped_pair(pipefds);

#اگر ENABLE_HUSH_LINENO_VAR
		G.execute_lineno = command->lineno;
#اگر_ختم

		command->pid = BB_MMU ? fork() : vfork();
		اگر (!command->pid) { /*بچہ*/
#اگر ENABLE_HUSH_کام
			disable_restore_tty_pgrp_on_exit();
			CLEAR_RANDOM_T(&G.random_gen); /* or else $RANDOM repeats in child */

			/* Every child adds itself to new process group
			 * with pgid == pid_of_first_child_in_pipe */
			اگر (G.تسلسل_list_level == 1 && G_interactive_fd) {
				pid_t pgrp;
				pgrp = pi->pgrp;
				اگر (pgrp < 0) /* true for 1st process only */
					pgrp = getpid();
				اگر (setpgid(0, pgrp) == 0
				 && pi->followup != PIPE_BG
				 && G_saved_tty_pgrp /* we have ctty */
				) {
					/* We do it in *every* child, not just first,
					 * to avoid races */
					tcsetpgrp(G_interactive_fd, pgrp);
				}
			}
#اگر_ختم
			اگر (pi->alive_cmds == 0 && pi->followup == PIPE_BG) {
				/* 1st cmd in backgrounded pipe
				 * should have its stdin /dev/null'ed */
				close(0);
				اگر (open(bb_dev_null, O_RDONLY))
					xopen("/", O_RDONLY);
			} ورنہ {
				xmove_fd(اگلا_infd, 0);
			}
			xmove_fd(pipefds.wr, 1);
			اگر (pipefds.rd > 1)
				close(pipefds.rd);
			/* Like bash, explicit redirects override pipes,
			 * and the pipe fd (fd#1) is available for dup'ing:
			 * "cmd1 2>&1 | cmd2": fd#1 is duped to fd#2, thus stderr
			 * of cmd1 goes into pipe.
			 */
			اگر (تیاری_redirects(command, تہی)) {
				/* Happens when redir file can't be opened:
				 * $ hush -c 'echo FOO >&2 | echo BAR 3>/qwe/rty; echo BAZ'
				 * FOO
				 * hush: can't open '/qwe/rty': No such file or directory
				 * BAZ
				 * (echo BAR is not executed, it hits _exit(1) below)
				 */
				_exit(1);
			}

			/* Stores to nommu_save list of env vars putenv'ed
			 * (NOMMU, on MMU we don't need that) */
			/* cast away volatility... */
			pseudo_exec((nommu_save_t*) &nommu_save, command, argv_expanded);
			/* pseudo_exec() does not return */
		}

		/* parent or error */
#اگر ENABLE_HUSH_FAST
		G.گنتی_SIGCHLD++;
//bb_error_msg("[%d] fork in run_pipe: G.count_SIGCHLD:%d G.handled_SIGCHLD:%d", getpid(), G.count_SIGCHLD, G.handled_SIGCHLD);
#اگر_ختم
		enable_restore_tty_pgrp_on_exit();
#اگر !BB_MMU
		/* Clean up after vforked child */
		آزاد(nommu_save.argv);
		آزاد(nommu_save.argv_from_re_execing);
		G.var_nest_level = sv_var_nest_level;
		remove_nested_vars();
		جوڑو_vars(nommu_save.old_vars);
#اگر_ختم
		آزاد(argv_expanded);
		argv_expanded = تہی;
		اگر (command->pid < 0) { /* [v]fork failed */
			/* Clearly indicate, was it fork or vfork */
			bb_simple_perror_پیغام(BB_MMU ? "vfork"+1 : "vfork");
		} ورنہ {
			pi->alive_cmds++;
#اگر ENABLE_HUSH_کام
			/* Second and next children need to know pid of first one */
			اگر (pi->pgrp < 0)
				pi->pgrp = command->pid;
#اگر_ختم
		}

		اگر (cmd_no > 1)
			close(اگلا_infd);
		اگر (cmd_no < pi->عدد_cmds)
			close(pipefds.wr);
		/* Pass read (output) pipe end to next iteration */
		اگلا_infd = pipefds.rd;
#ورنہ /* __U_BOOT__ */
		/* Process the command */
		rcode = cmd_process(G.do_repeat ? CMD_FLAG_REPEAT : 0,
				    command->argc, command->argv,
				    &(G.flag_repeat), تہی);

		اگر (argv_expanded) {
			/*
			 * expand_strvec_to_strvec() allocates memory to expand
			 * argv, we need to free it.
			 */
			آزاد(argv_expanded);

			/*
			 * We also restore command->argv to its original value
			 * so no memory leak happens.
			 */
			command->argv = argv;

			/*
			 * NOTE argc exists only in U-Boot, so argv freeing does
			 * not rely on it as this code exists in BusyBox.
			 */
		}
#اگر_ختم /* __U_BOOT__ */
	}

#اگر_غیر_تعریف_شدہ __U_BOOT__
	اگر (!pi->alive_cmds) {
		debug_leave();
		debug_printf_exec("run_pipe return 1 (all forks failed, no children)\n");
		واپس 1;
	}
#اگر_ختم /* __U_BOOT__ */

	debug_leave();
#اگر_غیر_تعریف_شدہ __U_BOOT__
	debug_printf_exec("run_pipe return -1 (%u children started)\n", pi->alive_cmds);
	واپس -1;
#ورنہ /* __U_BOOT__ */
	IF_HAS_KEYWORDS(اگر (pi->pi_inverted) rcode = !rcode;)
	debug_printf_exec("run_pipe return %d\n", rcode);
	واپس rcode;
#اگر_ختم /* __U_BOOT__ */
}

/* NB: called by pseudo_exec, and therefore must not modify any
 * global data until exec/_exit (we can be a child after vfork!) */
ساکن صحیح تسلسل_list(ساخت pipe *pi)
{
#اگر ENABLE_HUSH_CASE
	حرف *case_لفظ = تہی;
#اگر_ختم
#اگر ENABLE_HUSH_LOOPS
	ساخت pipe *loop_چوٹی = تہی;
	حرف **for_lcur = تہی;
	حرف **for_list = تہی;
#اگر_ختم
	smallint آخری_followup;
	smalluint rcode;
#اگر ENABLE_HUSH_IF || ENABLE_HUSH_CASE
	smalluint cond_رمز = 0;
#ورنہ
	شمار { cond_رمز = 0 };
#اگر_ختم
#اگر HAS_KEYWORDS
	smallint rword;      /* RES_foo */
	smallint آخری_rword; /* ditto */
#اگر_ختم

#اگر_غیر_تعریف_شدہ __U_BOOT__
	debug_printf_exec("run_list lvl %d start\n", G.تسلسل_list_level);
	debug_enter();
#اگر_ختم /* !__U_BOOT__ */

	سیٹ_G_ifs();

#اگر ENABLE_HUSH_LOOPS
	/* Check syntax for "for" */
	{
		ساخت pipe *cpipe;
		برائے (cpipe = pi; cpipe; cpipe = cpipe->اگلا) {
			اگر (cpipe->res_لفظ != RES_FOR && cpipe->res_لفظ != RES_اندر)
				جاری;
			/* current word is FOR or IN (BOLD in comments below) */
			اگر (cpipe->اگلا == تہی) {
				syntax_error("malformed for");
				debug_leave();
				debug_printf_exec("run_list lvl %d return 1\n", G.تسلسل_list_level);
				واپس 1;
			}
			/* "FOR v; do ..." and "for v IN a b; do..." are ok */
			اگر (cpipe->اگلا->res_لفظ == RES_DO)
				جاری;
			/* next word is not "do". It must be "in" then ("FOR v in ...") */
			اگر (cpipe->res_لفظ == RES_اندر /* "for v IN a b; not_do..."? */
			 || cpipe->اگلا->res_لفظ != RES_اندر /* FOR v not_do_and_not_in..."? */
			) {
				syntax_error("malformed for");
				debug_leave();
				debug_printf_exec("run_list lvl %d return 1\n", G.تسلسل_list_level);
				واپس 1;
			}
		}
	}
#اگر_ختم

	/* Past this point, all code paths should jump to ret: label
	 * in order to return, no direct "return" statements please.
	 * This helps to ensure that no memory is leaked. */

#اگر ENABLE_HUSH_کام
	G.تسلسل_list_level++;
#اگر_ختم

#اگر HAS_KEYWORDS
	rword = RES_NONE;
	آخری_rword = RES_XXXX;
#اگر_ختم
	آخری_followup = PIPE_SEQ;
	rcode = G.آخری_exitcode;

	/* Go through list of pipes, (maybe) executing them. */
#اگر_غیر_تعریف_شدہ __U_BOOT__
	برائے (; pi; pi = IF_HUSH_LOOPS(rword == RES_مکمل ? loop_چوٹی : ) pi->اگلا) {
#ورنہ /* __U_BOOT__ */
	برائے (; pi; pi = rword == RES_مکمل ? loop_چوٹی : pi->اگلا) {
#اگر_ختم /* __U_BOOT__ */
		صحیح r;
		صحیح sv_errexit_گہرائی;

#اگر_غیر_تعریف_شدہ __U_BOOT__
		اگر (G.flag_SIGINT)
			توڑو;
		اگر (G_flag_return_اندر_پیش_رفت == 1)
			توڑو;
#اگر_ختم /* !__U_BOOT__ */

		IF_HAS_KEYWORDS(rword = pi->res_لفظ;)
		debug_printf_exec(": rword:%d cond_code:%d last_rword:%d\n",
				rword, cond_رمز, آخری_rword);

		sv_errexit_گہرائی = G.errexit_گہرائی;
		اگر (
#اگر ENABLE_HUSH_IF
		    rword == RES_IF || rword == RES_ELIF ||
#اگر_ختم
		    pi->followup != PIPE_SEQ
		) {
			G.errexit_گہرائی++;
		}
#اگر ENABLE_HUSH_LOOPS
		اگر ((rword == RES_WHILE || rword == RES_UNTIL || rword == RES_FOR)
		 && loop_چوٹی == تہی /* avoid bumping G.depth_of_loop twice */
		) {
			/* start of a loop: remember where loop starts */
			loop_چوٹی = pi;
			G.گہرائی_کا_loop++;
		}
#اگر_ختم
		/* Still in the same "if...", "then..." or "do..." branch? */
		اگر (IF_HAS_KEYWORDS(rword == آخری_rword &&) 1) {
			اگر ((rcode == 0 && آخری_followup == PIPE_OR)
			 || (rcode != 0 && آخری_followup == PIPE_AND)
			) {
				/* It is "<true> || CMD" or "<false> && CMD"
				 * and we should not execute CMD */
				debug_printf_exec("skipped cmd because of || or &&\n");
				آخری_followup = pi->followup;
				جاؤ dont_check_jobs_but_continue;
			}
		}
		آخری_followup = pi->followup;
#اگر ENABLE_HUSH_IF
		اگر (cond_رمز != 0) {
			اگر (rword == RES_THEN) {
				/* if false; then ... fi has exitcode 0! */
				G.آخری_exitcode = rcode = EXIT_SUCCESS;
				/* "if <false> THEN cmd": skip cmd */
				debug_printf_exec("skipped THEN cmd because IF condition was false\n");
				آخری_rword = rword;
				جاری;
			}
		} ورنہ {
			اگر (rword == RES_ELSE
			 || (rword == RES_ELIF && آخری_rword != RES_ELIF)
			) {
				/* "if <true> then ... ELSE/ELIF cmd":
				 * skip cmd and all following ones */
				debug_printf_exec("skipped ELSE/ELIF branch because IF condition was true\n");
				توڑو;
			}
			//if (rword == RES_THEN): "if <true> THEN cmd", run cmd (fall through)
		}
#اگر_ختم
		IF_HAS_KEYWORDS(آخری_rword = rword;)
#اگر ENABLE_HUSH_LOOPS
		اگر (rword == RES_FOR) { /* && pi->num_cmds - always == 1 */
			اگر (!for_lcur) {
				/* first loop through for */

				ساکن ثابت حرف encoded_dollar_at[] ALIGN1 = {
					SPECIAL_VAR_علامت, '@' | 0x80, SPECIAL_VAR_علامت, '\0'
				}; /* encoded representation of "$@" */
				ساکن ثابت حرف *ثابت encoded_dollar_at_argv[] ALIGN_PTR = {
					encoded_dollar_at, تہی
				}; /* argv list with one element: "$@" */
				حرف **vals;

				G.آخری_exitcode = rcode = EXIT_SUCCESS;
				vals = (حرف**)encoded_dollar_at_argv;
				اگر (pi->اگلا->res_لفظ == RES_اندر) {
					/* if no variable values after "in" we skip "for" */
					اگر (!pi->اگلا->cmds[0].argv) {
						debug_printf_exec(": null FOR: exitcode EXIT_SUCCESS\n");
						توڑو;
					}
					vals = pi->اگلا->cmds[0].argv;
				} /* else: "for var; do..." -> assume "$@" list */
				/* create list of variable values */
				debug_print_لڑیاں("for_list made from", vals);
				for_list = expand_strvec_مقصد_strvec(vals);
				for_lcur = for_list;
				debug_print_لڑیاں("for_list", for_list);
			}
			اگر (!*for_lcur) {
				/* "for" loop is over, clean up */
				آزاد(for_list);
				for_list = تہی;
				for_lcur = تہی;
				توڑو;
			}
			/* Insert next value from for_lcur */
			/* note: *for_lcur already has quotes removed, $var expanded, etc */
#اگر_غیر_تعریف_شدہ __U_BOOT__
			سیٹ_local_var_from_halves(pi->cmds[0].argv[0], *for_lcur++);
#ورنہ /* __U_BOOT__ */
			/* We cannot use xasprintf, so we emulate it. */
			حرف *بھرا_var;
			حرف *var = pi->cmds[0].argv[0];
			حرف *val = *for_lcur++;

			/* + 1 to take into account =. */
			بھرا_var = xmalloc(لڑی_لمبائی(var) + لڑی_لمبائی(val) + 1);
			sprintf(بھرا_var, "%s=%s", var, val);

			سیٹ_local_var_modern(بھرا_var, /*flag:*/ 0);
#اگر_ختم /* __U_BOOT__ */
			جاری;
		}
		اگر (rword == RES_اندر) {
			جاری; /* "for v IN list;..." - "in" has no cmds anyway */
		}
		اگر (rword == RES_مکمل) {
			جاری; /* "done" has no cmds too */
		}
#اگر_ختم
#اگر ENABLE_HUSH_CASE
		اگر (rword == RES_CASE) {
			debug_printf_exec("CASE cond_code:%d\n", cond_رمز);
			case_لفظ = expand_string_مقصد_string(pi->cmds->argv[0],
				EXP_FLAG_ESC_GLOB_CHARS, /*unbackslash:*/ 1);
			debug_printf_exec("CASE word1:'%s'\n", case_لفظ);
			//unbackslash(case_word);
			//debug_printf_exec("CASE word2:'%s'\n", case_word);
			جاری;
		}
		اگر (rword == RES_مطابقت) {
			حرف **argv;

			debug_printf_exec("MATCH cond_code:%d\n", cond_رمز);
			اگر (!case_لفظ) /* "case ... matched_word) ... WORD)": we executed selected branch, stop */
				توڑو;
			/* all prev words didn't match, does this one match? */
			argv = pi->cmds->argv;
			جب (*argv) {
				حرف *نمونہ;
				debug_printf_exec("expand_string_to_string('%s')\n", *argv);
				نمونہ = expand_string_مقصد_string(*argv,
						EXP_FLAG_ESC_GLOB_CHARS,
						/*unbackslash:*/ 0
				);
				/* TODO: which FNM_xxx flags to use? */
				cond_رمز = (fnmatch(نمونہ, case_لفظ, /*flags:*/ 0) != 0);
				debug_printf_exec("cond_code=fnmatch(pattern:'%s',str:'%s'):%d\n",
						نمونہ, case_لفظ, cond_رمز);
				آزاد(نمونہ);
				اگر (cond_رمز == 0) {
					/* match! we will execute this branch */
					آزاد(case_لفظ);
					case_لفظ = تہی; /* make future "word)" stop */
					توڑو;
				}
				argv++;
			}
			جاری;
		}
		اگر (rword == RES_CASE_BODY) { /* inside of a case branch */
			debug_printf_exec("CASE_BODY cond_code:%d\n", cond_رمز);
			اگر (cond_رمز != 0)
				جاری; /* not matched yet, skip this pipe */
		}
		اگر (rword == RES_ESAC) {
			debug_printf_exec("ESAC cond_code:%d\n", cond_رمز);
			اگر (case_لفظ) {
				/* "case" did not match anything: still set $? (to 0) */
				G.آخری_exitcode = rcode = EXIT_SUCCESS;
			}
		}
#اگر_ختم
		/* Just pressing <enter> in shell should check for jobs.
		 * OTOH, in non-interactive shell this is useless
		 * and only leads to extra job checks */
		اگر (pi->عدد_cmds == 0) {
#اگر_غیر_تعریف_شدہ __U_BOOT__
			اگر (G_interactive_fd)
				جاؤ check_jobs_and_continue;
#اگر_ختم /* !__U_BOOT__ */
			جاری;
		}

		/* After analyzing all keywords and conditions, we decided
		 * to execute this pipe. NB: have to do checkjobs(NULL)
		 * after run_pipe to collect any background children,
		 * even if list execution is to be stopped. */
		debug_printf_exec(": run_pipe with %d members\n", pi->عدد_cmds);
#اگر_غیر_تعریف_شدہ __U_BOOT__
#اگر ENABLE_HUSH_LOOPS
		G.flag_break_continue = 0;
#اگر_ختم
#اگر_ختم /* !__U_BOOT__ */
#اگر_غیر_تعریف_شدہ __U_BOOT__
		rcode = r = G.o_opt[OPT_O_NOEXEC] ? 0 : تسلسل_pipe(pi);
		/* NB: rcode is a smalluint, r is int */
#ورنہ /* __U_BOOT__ */
		rcode = r = تسلسل_pipe(pi); /* NB: rcode is a smalluint, r is int */
		اگر (r <= EXIT_RET_رمز) {
			صحیح پچھلا_rcode = G.آخری_exitcode;
			/*
			 * This magic is to get the exit code given by the user.
			 * Contrary to old shell code, we use + EXIT_RET_CODE as EXIT_RET_CODE
			 * equals -2.
			 */
			G.آخری_exitcode = -r + EXIT_RET_رمز;

			/*
			 * This case deals with the following:
			 * => setenv inner 'echo entry inner; exit; echo inner done'
			 * => setenv outer 'echo entry outer; run inner; echo outer done'
			 * => run outer
			 * So, if we are in inner, we need to break and not run the other
			 * commands.
			 * Otherwise, we just continue in outer.
			 * As return code are propagated, we use the previous value to check if
			 * exit was just called or was propagated.
			 */
			اگر (پچھلا_rcode != r) {
				/*
				 * If run from run_command, run_command_flags will be set, so we check
				 * this to know if we are in main input shell.
				 */
				اگر (!G.تسلسل_command_flags)
					لکھو("exit not allowed from main input shell.\n");

				توڑو;
			}
			جاری;
		}
#اگر_ختم /* __U_BOOT__ */
		اگر (r != -1) {
			/* We ran a builtin, function, or group.
			 * rcode is already known
			 * and we don't need to wait for anything. */
			debug_printf_exec(": builtin/func exitcode %d\n", rcode);
			G.آخری_exitcode = rcode;
#اگر_غیر_تعریف_شدہ __U_BOOT__
			check_and_تسلسل_traps();
#اگر_ختم /* !__U_BOOT__ */
#اگر ENABLE_HUSH_TRAP && ENABLE_HUSH_FUNCTIONS
			rcode = G.آخری_exitcode; /* "return" in trap can change it, read back */
#اگر_ختم
#اگر_غیر_تعریف_شدہ __U_BOOT__
#اگر ENABLE_HUSH_LOOPS
			/* Was it "break" or "continue"? */
			اگر (G.flag_break_continue) {
				smallint fbc = G.flag_break_continue;
				/* We might fall into outer *loop*,
				 * don't want to break it too */
				اگر (loop_چوٹی) {
					G.گہرائی_break_continue--;
					اگر (G.گہرائی_break_continue == 0)
						G.flag_break_continue = 0;
					/* else: e.g. "continue 2" should *break* once, *then* continue */
				} /* else: "while... do... { we are here (innermost list is not a loop!) };...done" */
				اگر (G.گہرائی_break_continue != 0 || fbc == BC_BREAK) {
					checkjobs(تہی, 0 /*(no pid to wait for)*/);
					توڑو;
				}
				/* "continue": simulate end of loop */
				rword = RES_مکمل;
				جاری;
			}
#اگر_ختم
			اگر (G_flag_return_اندر_پیش_رفت == 1) {
				checkjobs(تہی, 0 /*(no pid to wait for)*/);
				توڑو;
			}

		} ورنہ اگر (pi->followup == PIPE_BG) {
			/* What does bash do with attempts to background builtins? */
			/* even bash 3.2 doesn't do that well with nested bg:
			 * try "{ { sleep 10; echo DEEP; } & echo HERE; } &".
			 * I'm NOT treating inner &'s as jobs */
#اگر ENABLE_HUSH_کام
			اگر (G.تسلسل_list_level == 1)
				داخل_کرو_کام_into_جدول(pi);
#اگر_ختم
			/* Last command's pid goes to $! */
			G.آخری_bg_pid = pi->cmds[pi->عدد_cmds - 1].pid;
			G.آخری_bg_pid_exitcode = 0;
			debug_printf_exec(": cmd&: exitcode EXIT_SUCCESS\n");
/* Check pi->pi_inverted? "! sleep 1 & echo $?": bash says 1. dash and ash say 0 */
			rcode = EXIT_SUCCESS;
			جاؤ check_traps;
		} ورنہ {
#اگر ENABLE_HUSH_کام
			اگر (G.تسلسل_list_level == 1 && G_interactive_fd) {
				/* Waits for completion, then fg's main shell */
				rcode = checkjobs_and_fg_shell(pi);
				debug_printf_exec(": checkjobs_and_fg_shell exitcode %d\n", rcode);
				جاؤ check_traps;
			}
#اگر_ختم
			/* This one just waits for completion */
			rcode = checkjobs(pi, 0 /*(no pid to wait for)*/);
			debug_printf_exec(": checkjobs exitcode %d\n", rcode);
 check_traps:
			G.آخری_exitcode = rcode;
			check_and_تسلسل_traps();
#اگر ENABLE_HUSH_TRAP && ENABLE_HUSH_FUNCTIONS
			rcode = G.آخری_exitcode; /* "return" in trap can change it, read back */
#اگر_ختم
		}
#اگر_ختم /* !__U_BOOT__ */

#اگر_غیر_تعریف_شدہ __U_BOOT__
		/* Handle "set -e" */
		اگر (rcode != 0 && G.o_opt[OPT_O_ERREXIT]) {
			debug_printf_exec("ERREXIT:1 errexit_depth:%d\n", G.errexit_گہرائی);
			اگر (G.errexit_گہرائی == 0)
				hush_exit(rcode);
		}
#ورنہ /* __U_BOOT__ */
		} /* if (r != -1) */
#اگر_ختم /* __U_BOOT__ */
		G.errexit_گہرائی = sv_errexit_گہرائی;

		/* Analyze how result affects subsequent commands */
#اگر ENABLE_HUSH_IF
		اگر (rword == RES_IF || rword == RES_ELIF) {
			debug_printf_exec("cond_code=rcode:%d\n", rcode);
			cond_رمز = rcode;
		}
#اگر_ختم
#اگر_غیر_تعریف_شدہ __U_BOOT__
 check_jobs_and_continue:
		checkjobs(تہی, 0 /*(no pid to wait for)*/);
#اگر_ختم /* !__U_BOOT__ */
 dont_check_jobs_but_continue: ;
#اگر ENABLE_HUSH_LOOPS
		/* Beware of "while false; true; do ..."! */
		اگر (pi->اگلا
		 && (pi->اگلا->res_لفظ == RES_DO || pi->اگلا->res_لفظ == RES_مکمل)
		 /* check for RES_DONE is needed for "while ...; do \n done" case */
		) {
			اگر (rword == RES_WHILE) {
				اگر (rcode) {
					/* "while false; do...done" - exitcode 0 */
					G.آخری_exitcode = rcode = EXIT_SUCCESS;
					debug_printf_exec(": while expr is false: breaking (exitcode:EXIT_SUCCESS)\n");
					توڑو;
				}
			}
			اگر (rword == RES_UNTIL) {
				اگر (!rcode) {
					debug_printf_exec(": until expr is true: breaking\n");
					توڑو;
				}
			}
		}
#اگر_ختم
	} /* for (pi) */

#اگر ENABLE_HUSH_کام
	G.تسلسل_list_level--;
#اگر_ختم
#اگر ENABLE_HUSH_LOOPS
	اگر (loop_چوٹی)
		G.گہرائی_کا_loop--;
	آزاد(for_list);
#اگر_ختم
#اگر ENABLE_HUSH_CASE
	آزاد(case_لفظ);
#اگر_ختم
#اگر_غیر_تعریف_شدہ __U_BOOT__
	debug_leave();
	debug_printf_exec("run_list lvl %d return %d\n", G.تسلسل_list_level, rcode);
#اگر_ختم /* !__U_BOOT__ */
	واپس rcode;
}

/* Select which version we will use */
ساکن صحیح تسلسل_and_free_list(ساخت pipe *pi)
{
	صحیح rcode = 0;
	debug_printf_exec("run_and_free_list entered\n");
#اگر_غیر_تعریف_شدہ __U_BOOT__
	اگر (!G.o_opt[OPT_O_NOEXEC]) {
#اگر_ختم /* !__U_BOOT__ */
		debug_printf_exec(": run_list: 1st pipe with %d cmds\n", pi->عدد_cmds);
		rcode = تسلسل_list(pi);
#اگر_غیر_تعریف_شدہ __U_BOOT__
	}
#اگر_ختم /* !__U_BOOT__ */
	/* free_pipe_list has the side effect of clearing memory.
	 * In the long run that function can be merged with run_list,
	 * but doing that now would hobble the debugging effort. */
	free_pipe_list(pi);
	debug_printf_exec("run_and_free_list return %d\n", rcode);
	واپس rcode;
}

#اگر_غیر_تعریف_شدہ __U_BOOT__
ساکن خالی install_sighandlers(غیرعلامتی نقاب)
{
	sighandler_t old_handler;
	غیرعلامتی sig = 0;
	جب ((نقاب >>= 1) != 0) {
		sig++;
		اگر (!(نقاب & 1))
			جاری;
		old_handler = install_sighandler(sig, pick_sighandler(sig));
		/* POSIX allows shell to re-enable SIGCHLD
		 * even if it was SIG_IGN on entry.
		 * Therefore we skip IGN check for it:
		 */
		اگر (sig == SIGCHLD)
			جاری;
		/* Interactive bash re-enables SIGHUP which is SIG_IGNed on entry.
		 * Try:
		 * trap '' hup; bash; echo RET  # type "kill -hup $$", see SIGHUP having effect
		 * trap '' hup; bash -c 'kill -hup $$; echo ALIVE'  # here SIGHUP is SIG_IGNed
		 */
		اگر (sig == SIGHUP && G_interactive_fd)
			جاری;
		/* Unless one of the above signals, is it SIG_IGN? */
		اگر (old_handler == SIG_IGN) {
			/* oops... restore back to IGN, and record this fact */
			install_sighandler(sig, old_handler);
#اگر ENABLE_HUSH_TRAP
			اگر (!G_traps)
				G_traps = xzalloc(حجم(G_traps[0]) * NSIG);
			آزاد(G_traps[sig]);
			G_traps[sig] = xzalloc(1); /* == xstrdup(""); */
#اگر_ختم
		}
	}
}

/* Called a few times only (or even once if "sh -c") */
ساکن خالی install_special_sighandlers(خالی)
{
	غیرعلامتی نقاب;

	/* Which signals are shell-special? */
	نقاب = (1 << SIGQUIT) | (1 << SIGCHLD);
	اگر (G_interactive_fd) {
		نقاب |= SPECIAL_INTERACTIVE_SIGS;
		اگر (G_saved_tty_pgrp) /* we have ctty, job control sigs work */
			نقاب |= SPECIAL_JOBSTOP_SIGS;
	}
	/* Careful, do not re-install handlers we already installed */
	اگر (G.special_sig_نقاب != نقاب) {
		غیرعلامتی diff = نقاب & ~G.special_sig_نقاب;
		G.special_sig_نقاب = نقاب;
		install_sighandlers(diff);
	}
}

#اگر ENABLE_HUSH_کام
/* helper */
/* Set handlers to restore tty pgrp and exit */
ساکن خالی install_fatal_sighandlers(خالی)
{
	غیرعلامتی نقاب;

	/* We will restore tty pgrp on these signals */
	نقاب = 0
		/*+ (1 << SIGILL ) * HUSH_DEBUG*/
		/*+ (1 << SIGFPE ) * HUSH_DEBUG*/
		+ (1 << SIGBUS ) * HUSH_DEBUG
		+ (1 << SIGSEGV) * HUSH_DEBUG
		/*+ (1 << SIGTRAP) * HUSH_DEBUG*/
		+ (1 << SIGABRT)
	/* bash 3.2 seems to handle these just like 'fatal' ones */
		+ (1 << SIGPIPE)
		+ (1 << SIGALRM)
	/* if we are interactive, SIGHUP, SIGTERM and SIGINT are special sigs.
	 * if we aren't interactive... but in this case
	 * we never want to restore pgrp on exit, and this fn is not called
	 */
		/*+ (1 << SIGHUP )*/
		/*+ (1 << SIGTERM)*/
		/*+ (1 << SIGINT )*/
	;
	G_fatal_sig_نقاب = نقاب;

	install_sighandlers(نقاب);
}
#اگر_ختم

ساکن صحیح سیٹ_منوال(صحیح state, حرف منوال, ثابت حرف *o_opt)
{
	صحیح idx;
	انتخاب (منوال) {
	حالت 'n':
		/* set -n has no effect in interactive shell */
		/* Try: while set -n; do echo $-; done */
		اگر (!G_interactive_fd)
			G.o_opt[OPT_O_NOEXEC] = state;
		توڑو;
	حالت 'x':
		IF_HUSH_منوال_X(G_x_منوال = state;)
		IF_HUSH_منوال_X(اگر (G.x_منوال_fd <= 0) G.x_منوال_fd = dup_CLOEXEC(2, 10);)
		توڑو;
	حالت 'e':
		G.o_opt[OPT_O_ERREXIT] = state;
		توڑو;
	حالت 'o':
		اگر (!o_opt) {
			/* "set -o" or "set +o" without parameter.
			 * in bash, set -o produces this output:
			 *  pipefail        off
			 * and set +o:
			 *  set +o pipefail
			 * We always use the second form.
			 */
			ثابت حرف *p = o_opt_لڑیاں;
			idx = 0;
			جب (*p) {
				لکھو("set %co %s\n", (G.o_opt[idx] ? '-' : '+'), p);
				idx++;
				p += لڑی_لمبائی(p) + 1;
			}
			توڑو;
		}
		idx = اشاریہ_اندر_لڑیاں(o_opt_لڑیاں, o_opt);
		اگر (idx >= 0) {
			G.o_opt[idx] = state;
			توڑو;
		}
		/* fall through to error */
	طےشدہ:
		واپس EXIT_FAILURE;
	}
	واپس EXIT_SUCCESS;
}

صحیح hush_main(صحیح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحیح hush_main(صحیح argc, حرف **argv)
{
	pid_t cached_getpid;
	شمار {
		OPT_login = (1 << 0),
	};
	غیرعلامتی flags;
#اگر !BB_MMU
	غیرعلامتی builtin_argc = 0;
#اگر_ختم
	حرف **e;
	ساخت variable *cur_var;
	ساخت variable *shell_ver;

	INIT_G();
	اگر (EXIT_SUCCESS != 0) /* if EXIT_SUCCESS == 0, it is already done */
		G.آخری_exitcode = EXIT_SUCCESS;
#اگر !BB_MMU
	/* "Big heredoc" support via "sh -< STRING" invocation.
	 * Check it first (do not bother to run the usual init code,
	 * it is not needed for this case).
	 */
	اگر (argv[1]
	 && argv[1][0] == '-' && argv[1][1] == '<' /*&& !argv[1][2]*/
	 /*&& argv[2] && !argv[3] - we don't check some conditions */
	) {
		بھرا_write1_str(argv[2]);
		_exit(0);
	}
	G.argv0_for_re_execing = argv[0];
#اگر_ختم
#اگر ENABLE_HUSH_TRAP
# اگر ENABLE_HUSH_FUNCTIONS
	G.return_exitcode = -1;
# endif
	G.pre_trap_exitcode = -1;
#اگر_ختم

#اگر ENABLE_HUSH_FAST
	G.گنتی_SIGCHLD++; /* ensure it is != G.handled_SIGCHLD */
#اگر_ختم

	cached_getpid = getpid();   /* for tcsetpgrp() during init */
	G.جڑ_pid = cached_getpid; /* for $PID  (NOMMU can override via -$HEXPID:HEXPPID:...) */
	G.جڑ_ppid = getppid();    /* for $PPID (NOMMU can override) */

	/* Deal with HUSH_VERSION */
	debug_printf_env("unsetenv '%s'\n", "HUSH_VERSION");
	unsetenv("HUSH_VERSION"); /* in case it exists in initial env */
	shell_ver = xzalloc(حجم(*shell_ver));
	shell_ver->flg_export = 1;
	shell_ver->flg_read_only = 1;
	/* Code which handles ${var<op>...} needs writable values for all variables,
	 * therefore we xstrdup: */
	shell_ver->varstr = xstrdup(hush_نسخہ_str);

	/* Create shell local variables from the values
	 * currently living in the environment */
	G.چوٹی_var = shell_ver;
	cur_var = G.چوٹی_var;
	e = environ;
	اگر (e) جب (*e) {
		حرف *قدر = strchr(*e, '=');
		اگر (قدر) { /* paranoia */
			cur_var->اگلا = xzalloc(حجم(*cur_var));
			cur_var = cur_var->اگلا;
			cur_var->varstr = *e;
			cur_var->max_len = لڑی_لمبائی(*e);
			cur_var->flg_export = 1;
		}
		e++;
	}
	/* (Re)insert HUSH_VERSION into env (AFTER we scanned the env!) */
	debug_printf_env("putenv '%s'\n", shell_ver->varstr);
	putenv(shell_ver->varstr);

	/* Export PWD */
	سیٹ_pwd_var(SETFLAG_EXPORT);

#اگر BASH_HOSTNAME_VAR
	/* Set (but not export) HOSTNAME unless already set */
	اگر (!get_local_var_قدر("HOSTNAME")) {
		ساخت utsname uts;
		uname(&uts);
		سیٹ_local_var_from_halves("HOSTNAME", uts.nodename);
	}
#اگر_ختم
	/* IFS is not inherited from the parent environment */
	سیٹ_local_var_from_halves("IFS", defifs);

	اگر (!get_local_var_قدر("PATH"))
		سیٹ_local_var_from_halves("PATH", bb_default_جڑ_راستہ);

	/* PS1/PS2 are set later, if we determine that we are interactive */

	/* bash also exports SHLVL and _,
	 * and sets (but doesn't export) the following variables:
	 * BASH=/bin/bash
	 * BASH_VERSINFO=([0]="3" [1]="2" [2]="0" [3]="1" [4]="release" [5]="i386-pc-linux-gnu")
	 * BASH_VERSION='3.2.0(1)-release'
	 * HOSTTYPE=i386
	 * MACHTYPE=i386-pc-linux-gnu
	 * OSTYPE=linux-gnu
	 * PPID=<NNNNN> - we also do it elsewhere
	 * EUID=<NNNNN>
	 * UID=<NNNNN>
	 * GROUPS=()
	 * LINES=<NNN>
	 * COLUMNS=<NNN>
	 * BASH_ARGC=()
	 * BASH_ARGV=()
	 * BASH_LINENO=()
	 * BASH_SOURCE=()
	 * DIRSTACK=()
	 * PIPESTATUS=([0]="0")
	 * HISTFILE=/<xxx>/.bash_history
	 * HISTFILESIZE=500
	 * HISTSIZE=500
	 * MAILCHECK=60
	 * PATH=/usr/gnu/bin:/usr/local/bin:/bin:/usr/bin:.
	 * SHELL=/bin/bash
	 * SHELLOPTS=braceexpand:emacs:hashall:histexpand:history:interactive-comments:monitor
	 * TERM=dumb
	 * OPTERR=1
	 * OPTIND=1
	 * PS4='+ '
	 */

#اگر عدد_SCRIPTS > 0
	اگر (argc < 0) {
		حرف *script = get_script_content(-argc - 1);
		G.global_argv = argv;
		G.global_argc = string_array_len(argv);
		//install_special_sighandlers(); - needed?
		parse_and_تسلسل_string(script);
		جاؤ final_return;
	}
#اگر_ختم

	/* Initialize some more globals to non-zero values */
	die_func = restore_ttypgrp_and__exit;

	/* Shell is non-interactive at first. We need to call
	 * install_special_sighandlers() if we are going to execute "sh <script>",
	 * "sh -c <cmds>" or login shell's /etc/profile and friends.
	 * If we later decide that we are interactive, we run install_special_sighandlers()
	 * in order to intercept (more) signals.
	 */

	/* Parse options */
	/* http://www.opengroup.org/onlinepubs/9699919799/utilities/sh.html */
	flags = (argv[0] && argv[0][0] == '-') ? OPT_login : 0;
	جب (1) {
		صحیح opt = getopt(argc, argv, "+" /* stop at 1st non-option */
				"cexinsl"
#اگر !BB_MMU
				"$:R:V:"
# اگر ENABLE_HUSH_LINENO_VAR
				"L:"
# endif
# اگر ENABLE_HUSH_FUNCTIONS
				"F:"
# endif
#اگر_ختم
		);
		اگر (opt <= 0)
			توڑو;
		انتخاب (opt) {
		حالت 'c':
			/* Note: -c is not an option with param!
			 * "hush -c -l SCRIPT" is valid. "hush -cSCRIPT" is not.
			 */
			G.opt_c = 1;
			توڑو;
		حالت 'i':
			/* Well, we cannot just declare interactiveness,
			 * we have to have some stuff (ctty, etc) */
			/* G_interactive_fd++; */
//There are a few cases where bash -i -c 'SCRIPT'
//has visible effect (differs from bash -c 'SCRIPT'):
//it ignores TERM:
//	bash -i -c 'kill $$; echo ALIVE'
//	ALIVE
//it resets SIG_IGNed HUP to SIG_DFL:
//	trap '' hup; bash -i -c 'kill -hup $$; echo ALIVE'
//	Hangup   [the message is not printed by bash, it's the shell which started it]
//is talkative about jobs and exiting:
//	bash -i -c 'sleep 1 & exit'
//	[1] 16170
//	exit
//includes $ENV file (only if run as "sh"):
//	echo last >/tmp/ENV; ENV=/tmp/ENV sh -i -c 'echo HERE'
//	last: cannot open /var/log/wtmp: No such file or directory
//یہاں
//(under "bash", it's the opposite: it runs $BASH_ENV file only *without* -i).
//
//ash -i -c 'sleep 3; sleep 3', on ^C, drops into a prompt instead of exiting
//(this may be a bug, bash does not do this).
//(ash -i -c 'sleep 3' won't show this, the last command gets auto-"exec"ed)
//
//None of the above feel like useful features people would rely on.
			توڑو;
		حالت 's':
			G.opt_s = 1;
			توڑو;
		حالت 'l':
			flags |= OPT_login;
			توڑو;
#اگر !BB_MMU
		حالت '$': {
			غیرعلامتی طویل طویل خالی_خانے_trap_نقاب;

			G.جڑ_pid = bb_strtou(optarg, &optarg, 16);
			optarg++;
			G.جڑ_ppid = bb_strtou(optarg, &optarg, 16);
			optarg++;
			G.آخری_bg_pid = bb_strtou(optarg, &optarg, 16);
			optarg++;
			G.آخری_exitcode = bb_strtou(optarg, &optarg, 16);
			optarg++;
			builtin_argc = bb_strtou(optarg, &optarg, 16);
			optarg++;
			خالی_خانے_trap_نقاب = bb_strtoull(optarg, &optarg, 16);
			اگر (خالی_خانے_trap_نقاب != 0) {
				IF_HUSH_TRAP(صحیح sig;)
				install_special_sighandlers();
# اگر ENABLE_HUSH_TRAP
				G_traps = xzalloc(حجم(G_traps[0]) * NSIG);
				برائے (sig = 1; sig < NSIG; sig++) {
					اگر (خالی_خانے_trap_نقاب & (1LL << sig)) {
						G_traps[sig] = xzalloc(1); /* == xstrdup(""); */
						install_sighandler(sig, SIG_IGN);
					}
				}
# endif
			}
# اگر ENABLE_HUSH_LOOPS
			optarg++;
			G.گہرائی_کا_loop = bb_strtou(optarg, &optarg, 16);
# endif
			/* Suppress "killed by signal" message, -$ hack is used
			 * for subshells: echo `sh -c 'kill -9 $$'`
			 * should be silent.
			 */
			IF_HUSH_کام(G.تسلسل_list_level = 1;)
# اگر ENABLE_HUSH_FUNCTIONS
			/* nommu uses re-exec trick for "... | func | ...",
			 * should allow "return".
			 * This accidentally allows returns in subshells.
			 */
			G_flag_return_اندر_پیش_رفت = -1;
# endif
			توڑو;
		}
		حالت 'R':
		حالت 'V':
			سیٹ_local_var(xstrdup(optarg), opt == 'R' ? SETFLAG_MAKE_RO : 0);
			توڑو;
# اگر ENABLE_HUSH_LINENO_VAR
		حالت 'L':
			G.parse_lineno = xatou(optarg);
			توڑو;
# endif
# اگر ENABLE_HUSH_FUNCTIONS
		حالت 'F': {
			ساخت function *funcp = new_function(optarg);
			/* funcp->name is already set to optarg */
			/* funcp->body is set to NULL. It's a special case. */
			funcp->body_as_string = argv[optind];
			optind++;
			توڑو;
		}
# endif
#اگر_ختم
		/*case '?': invalid option encountered (set_mode('?') will fail) */
		/*case 'n':*/
		/*case 'x':*/
		/*case 'e':*/
		طےشدہ:
			اگر (سیٹ_منوال(1, opt, تہی) == 0) /* no error */
				توڑو;
			bb_show_usage();
		}
	} /* option parsing loop */

	/* Skip options. Try "hush -l": $1 should not be "-l"! */
	G.global_argc = argc - (optind - 1);
	G.global_argv = argv + (optind - 1);
	G.global_argv[0] = argv[0];

	/* If we are login shell... */
	اگر (flags & OPT_login) {
		ثابت حرف *hp = تہی;
		HFILE *داخلہ;

		debug_printf("sourcing /etc/profile\n");
		داخلہ = hfopen("/etc/profile");
 تسلسل_profile:
		اگر (داخلہ != تہی) {
			install_special_sighandlers();
			parse_and_تسلسل_file(داخلہ);
			hfclose(داخلہ);
		}
		/* bash: after sourcing /etc/profile,
		 * tries to source (in the given order):
		 * ~/.bash_profile, ~/.bash_login, ~/.profile,
		 * stopping on first found. --noprofile turns this off.
		 * bash also sources ~/.bash_logout on exit.
		 * If called as sh, skips .bash_XXX files.
		 */
		اگر (!hp) { /* unless we looped on the "goto" already */
			hp = get_local_var_قدر("HOME");
			اگر (hp && hp[0]) {
				debug_printf("sourcing ~/.profile\n");
				hp = concat_راستہ_file(hp, ".profile");
				داخلہ = hfopen(hp);
				آزاد((حرف*)hp);
				جاؤ تسلسل_profile;
			}
		}
	}

#اگر_غیر_تعریف_شدہ __U_BOOT__
	/* -c takes effect *after* -l */
	اگر (G.opt_c) {
		/* Possibilities:
		 * sh ... -c 'script'
		 * sh ... -c 'script' ARG0 [ARG1...]
		 * On NOMMU, if builtin_argc != 0,
		 * sh ... -c 'builtin' BARGV... "" ARG0 [ARG1...]
		 * "" needs to be replaced with NULL
		 * and BARGV vector fed to builtin function.
		 * Note: the form without ARG0 never happens:
		 * sh ... -c 'builtin' BARGV... ""
		 */
		حرف *script;

		install_special_sighandlers();

		G.global_argc--;
		G.global_argv++;
#اگر !BB_MMU
		اگر (builtin_argc) {
			/* -c 'builtin' [BARGV...] "" ARG0 [ARG1...] */
			ثابت ساخت built_اندر_command *x;
			x = ڈھونڈو_builtin(G.global_argv[0]);
			اگر (x) { /* paranoia */
				argv = G.global_argv;
				G.global_argc -= builtin_argc + 1; /* skip [BARGV...] "" */
				G.global_argv += builtin_argc + 1;
				G.global_argv[-1] = تہی; /* replace "" */
				G.آخری_exitcode = x->b_function(argv);
			}
			جاؤ final_return;
		}
#اگر_ختم

		script = G.global_argv[0];
		اگر (!script)
			bb_error_پیغام_and_die(bb_پیغام_requires_arg, "-c");
		اگر (!G.global_argv[1]) {
			/* -c 'script' (no params): prevent empty $0 */
			G.global_argv[0] = argv[0];
		} ورنہ { /* else -c 'script' ARG0 [ARG1...]: $0 is ARG0 */
			G.global_argc--;
			G.global_argv++;
		}
		parse_and_تسلسل_string(script);
		جاؤ final_return;
	}

	/* -s is: hush -s ARGV1 ARGV2 (no SCRIPT) */
	اگر (!G.opt_s && G.global_argv[1]) {
		HFILE *داخلہ;
		/*
		 * "bash <script>" (which is never interactive (unless -i?))
		 * sources $BASH_ENV here (without scanning $PATH).
		 * If called as sh, does the same but with $ENV.
		 * Also NB, per POSIX, $ENV should undergo parameter expansion.
		 */
		G.global_argc--;
		G.global_argv++;
		debug_printf("running script '%s'\n", G.global_argv[0]);
		xfunc_error_retval = 127; /* for "hush /does/not/exist" case */
		داخلہ = hfopen(G.global_argv[0]);
		اگر (!داخلہ) {
			bb_simple_perror_پیغام_and_die(G.global_argv[0]);
		}
		xfunc_error_retval = 1;
		install_special_sighandlers();
		parse_and_تسلسل_file(داخلہ);
#اگر ENABLE_FEATURE_CLEAN_اوپر
		hfclose(داخلہ);
#اگر_ختم
		جاؤ final_return;
	}
	/* "implicit" -s: bare interactive hush shows 's' in $- */
	G.opt_s = 1;

#اگر_ختم /* __U_BOOT__ */
	/* Up to here, shell was non-interactive. Now it may become one.
	 * NB: don't forget to (re)run install_special_sighandlers() as needed.
	 */

	/* A shell is interactive if the '-i' flag was given,
	 * or if all of the following conditions are met:
	 *    no -c command
	 *    no arguments remaining or the -s flag given
	 *    standard input is a terminal
	 *    standard output is a terminal
	 * Refer to Posix.2, the description of the 'sh' utility.
	 */
#اگر ENABLE_HUSH_کام
	اگر (isatty(STDIN_FILENO) && isatty(STDOUT_FILENO)) {
		G_saved_tty_pgrp = tcgetpgrp(STDIN_FILENO);
		debug_printf("saved_tty_pgrp:%d\n", G_saved_tty_pgrp);
		اگر (G_saved_tty_pgrp < 0)
			G_saved_tty_pgrp = 0;

		/* try to dup stdin to high fd#, >= 255 */
		G_interactive_fd = dup_CLOEXEC(STDIN_FILENO, 254);
		اگر (G_interactive_fd < 0) {
			/* try to dup to any fd */
			G_interactive_fd = dup_CLOEXEC(STDIN_FILENO, -1);
			اگر (G_interactive_fd < 0) {
				/* give up */
				G_interactive_fd = 0;
				G_saved_tty_pgrp = 0;
			}
		}
	}
	debug_printf("interactive_fd:%d\n", G_interactive_fd);
	اگر (G_interactive_fd) {
		اگر (G_saved_tty_pgrp) {
			/* If we were run as 'hush &', sleep until we are
			 * in the foreground (tty pgrp == our pgrp).
			 * If we get started under a job aware app (like bash),
			 * make sure we are now in charge so we don't fight over
			 * who gets the foreground */
			جب (1) {
				pid_t shell_pgrp = getpgrp();
				G_saved_tty_pgrp = tcgetpgrp(G_interactive_fd);
				اگر (G_saved_tty_pgrp == shell_pgrp)
					توڑو;
				/* send TTIN to ourself (should stop us) */
				kill(- shell_pgrp, SIGTTIN);
			}
		}

		/* Install more signal handlers */
		install_special_sighandlers();

		اگر (G_saved_tty_pgrp) {
			/* Set other signals to restore saved_tty_pgrp */
			install_fatal_sighandlers();
			/* Put ourselves in our own process group
			 * (bash, too, does this only if ctty is available) */
			bb_setpgrp(); /* is the same as setpgid(our_pid, our_pid); */
			/* Grab control of the terminal */
			tcsetpgrp(G_interactive_fd, cached_getpid);
		}
		enable_restore_tty_pgrp_on_exit();

# اگر ENABLE_FEATURE_EDITING
		G.سطر_داخلہ_state = new_سطر_داخلہ_t(FOR_SHELL);
#  اگر ENABLE_FEATURE_TAB_COMPLETION
		G.سطر_داخلہ_state->get_exe_name = hush_command_name;
#  endif
#  اگر EDITING_HAS_sh_get_var
		G.سطر_داخلہ_state->sh_get_var = get_local_var_قدر;
#  endif
# endif
# اگر ENABLE_HUSH_SAVEHISTORY && MAX_HISTORY > 0
		{
			ثابت حرف *hp = get_local_var_قدر("HISTFILE");
			اگر (!hp) {
				hp = get_local_var_قدر("HOME");
				اگر (hp)
					hp = concat_راستہ_file(hp, ".hush_history");
			} ورنہ {
				hp = xstrdup(hp);
			}
			اگر (hp) {
				G.سطر_داخلہ_state->hist_file = hp;
				//set_local_var(xasprintf("HISTFILE=%s", ...));
			}
#  اگر ENABLE_FEATURE_SH_HISTFILESIZE
			hp = get_local_var_قدر("HISTFILESIZE");
			G.سطر_داخلہ_state->max_history = سائز_from_HISTFILESIZE(hp);
#  endif
		}
# endif
	} ورنہ {
		install_special_sighandlers();
	}
#وگرنہ_اگر ENABLE_HUSH_INTERACTIVE
	/* No job control compiled in, only prompt/line editing */
	اگر (isatty(STDIN_FILENO) && isatty(STDOUT_FILENO)) {
		G_interactive_fd = dup_CLOEXEC(STDIN_FILENO, 254);
		اگر (G_interactive_fd < 0) {
			/* try to dup to any fd */
			G_interactive_fd = dup_CLOEXEC(STDIN_FILENO, -1);
			اگر (G_interactive_fd < 0)
				/* give up */
				G_interactive_fd = 0;
		}
	}
	install_special_sighandlers();
#ورنہ
	/* We have interactiveness code disabled */
	install_special_sighandlers();
#اگر_ختم
	/* bash:
	 * if interactive but not a login shell, sources ~/.bashrc
	 * (--norc turns this off, --rcfile <file> overrides)
	 */

	اگر (G_interactive_fd) {
#اگر ENABLE_HUSH_INTERACTIVE && ENABLE_FEATURE_EDITING_FANCY_PROMPT
		/* Set (but not export) PS1/2 unless already set */
		اگر (!get_local_var_قدر("PS1"))
			سیٹ_local_var_from_halves("PS1", "\\w \\$ ");
		اگر (!get_local_var_قدر("PS2"))
			سیٹ_local_var_from_halves("PS2", "> ");
#اگر_ختم
		اگر (!ENABLE_FEATURE_SH_EXTRA_QUIET) {
			/* note: ash and hush share this string */
			لکھو("\n\n%s %s\n"
				IF_HUSH_HELP("Enter 'help' for a list of built-in commands.\n")
				"\n",
				bb_banner,
				"hush - the humble shell"
			);
		}
	}

	parse_and_تسلسل_file(hfopen(تہی)); /* stdin */

 final_return:
	hush_exit(G.آخری_exitcode);
}

/*
 * Built-ins
 */
ساکن صحیح FAST_FUNC builtin_true(حرف **argv غیر_مستعمل_PARAM)
{
	واپس 0;
}

ساکن صحیح FAST_FUNC builtin_false(حرف **argv غیر_مستعمل_PARAM)
{
	واپس 1;
}

#اگر ENABLE_HUSH_TEST || ENABLE_HUSH_ECHO || ENABLE_HUSH_PRINTF || ENABLE_HUSH_KILL
ساکن NOINLINE صحیح تسلسل_applet_main(حرف **argv, صحیح (*applet_main_func)(صحیح argc, حرف **argv))
{
	صحیح argc = string_array_len(argv);
	واپس applet_main_func(argc, argv);
}
#اگر_ختم
#اگر ENABLE_HUSH_TEST || BASH_TEST2
ساکن صحیح FAST_FUNC builtin_test(حرف **argv)
{
	واپس تسلسل_applet_main(argv, test_main);
}
#اگر_ختم
#اگر ENABLE_HUSH_ECHO
ساکن صحیح FAST_FUNC builtin_echo(حرف **argv)
{
	واپس تسلسل_applet_main(argv, echo_main);
}
#اگر_ختم
#اگر ENABLE_HUSH_PRINTF
ساکن صحیح FAST_FUNC builtin_printf(حرف **argv)
{
	واپس تسلسل_applet_main(argv, printf_main);
}
#اگر_ختم

#اگر ENABLE_HUSH_HELP
ساکن صحیح FAST_FUNC builtin_help(حرف **argv غیر_مستعمل_PARAM)
{
	ثابت ساخت built_اندر_command *x;

	لکھو(
		"Built-in commands:\n"
		"------------------\n");
	برائے (x = bltins1; x != &bltins1[ARRAY_سائز(bltins1)]; x++) {
		اگر (x->b_descr)
			لکھو("%-10s%s\n", x->b_cmd, x->b_descr);
	}
	واپس EXIT_SUCCESS;
}
#اگر_ختم

#اگر MAX_HISTORY && ENABLE_FEATURE_EDITING
ساکن صحیح FAST_FUNC builtin_history(حرف **argv غیر_مستعمل_PARAM)
{
	show_history(G.سطر_داخلہ_state);
	واپس EXIT_SUCCESS;
}
#اگر_ختم

ساکن صحیح FAST_FUNC builtin_cd(حرف **argv)
{
	ثابت حرف *newdir;

	argv = skip_dash_dash(argv);
	newdir = argv[0];
	اگر (newdir == تہی) {
		/* bash does nothing (exitcode 0) if HOME is ""; if it's unset,
		 * bash says "bash: cd: HOME not set" and does nothing
		 * (exitcode 1)
		 */
		ثابت حرف *home = get_local_var_قدر("HOME");
		newdir = home ? home : "/";
	}
	اگر (chdir(newdir)) {
		/* Mimic bash message exactly */
		bb_perror_پیغام("cd: %s", newdir);
		واپس EXIT_FAILURE;
	}
	/* Read current dir (get_cwd(1) is inside) and set PWD.
	 * Note: do not enforce exporting. If PWD was unset or unexported,
	 * set it again, but do not export. bash does the same.
	 */
	سیٹ_pwd_var(/*flag:*/ 0);
	واپس EXIT_SUCCESS;
}

ساکن صحیح FAST_FUNC builtin_pwd(حرف **argv غیر_مستعمل_PARAM)
{
	puts(get_cwd(0));
	واپس EXIT_SUCCESS;
}

ساکن صحیح FAST_FUNC builtin_eval(حرف **argv)
{
	argv = skip_dash_dash(argv);

	اگر (!argv[0])
		واپس EXIT_SUCCESS;

	IF_HUSH_منوال_X(G.x_منوال_گہرائی++;)
	//bb_error_msg("%s: ++x_mode_depth=%d", __func__, G.x_mode_depth);
	اگر (!argv[1]) {
		/* bash:
		 * eval "echo Hi; done" ("done" is syntax error):
		 * "echo Hi" will not execute too.
		 */
		parse_and_تسلسل_string(argv[0]);
	} ورنہ {
		/* "The eval utility shall construct a command by
		 * concatenating arguments together, separating
		 * each with a <space> character."
		 */
		حرف *str, *p;
		غیرعلامتی len = 0;
		حرف **pp = argv;
		کرو
			len += لڑی_لمبائی(*pp) + 1;
		جب (*++pp);
		str = p = xmalloc(len);
		pp = argv;
		برائے (;;) {
			p = stpcpy(p, *pp);
			pp++;
			اگر (!*pp)
				توڑو;
			*p++ = ' ';
		}
		parse_and_تسلسل_string(str);
		آزاد(str);
	}
	IF_HUSH_منوال_X(G.x_منوال_گہرائی--;)
	//bb_error_msg("%s: --x_mode_depth=%d", __func__, G.x_mode_depth);
	واپس G.آخری_exitcode;
}

ساکن صحیح FAST_FUNC builtin_exec(حرف **argv)
{
	argv = skip_dash_dash(argv);
	اگر (argv[0] == تہی)
		واپس EXIT_SUCCESS; /* bash does this */

	/* Careful: we can end up here after [v]fork. Do not restore
	 * tty pgrp then, only top-level shell process does that */
	اگر (G_saved_tty_pgrp && getpid() == G.جڑ_pid)
		tcsetpgrp(G_interactive_fd, G_saved_tty_pgrp);

	/* Saved-redirect fds, script fds and G_interactive_fd are still
	 * open here. However, they are all CLOEXEC, and execv below
	 * closes them. Try interactive "exec ls -l /proc/self/fd",
	 * it should show no extra open fds in the "ls" process.
	 * If we'd try to run builtins/NOEXECs, this would need improving.
	 */
	//close_saved_fds_and_FILE_fds();

	/* TODO: if exec fails, bash does NOT exit! We do.
	 * We'll need to undo trap cleanup (it's inside execvp_or_die)
	 * and tcsetpgrp, and this is inherently racy.
	 */
	execvp_or_die(argv);
}

ساکن صحیح FAST_FUNC builtin_exit(حرف **argv)
{
	debug_printf_exec("%s()\n", __func__);

	/* interactive bash:
	 * # trap "echo EEE" EXIT
	 * # exit
	 * exit
	 * There are stopped jobs.
	 * (if there are _stopped_ jobs, running ones don't count)
	 * # exit
	 * exit
	 * EEE (then bash exits)
	 *
	 * TODO: we can use G.exiting = -1 as indicator "last cmd was exit"
	 */

	/* note: EXIT trap is run by hush_exit */
	argv = skip_dash_dash(argv);
	اگر (argv[0] == تہی) {
#اگر ENABLE_HUSH_TRAP
		اگر (G.pre_trap_exitcode >= 0) /* "exit" in trap uses $? from before the trap */
			hush_exit(G.pre_trap_exitcode);
#اگر_ختم
		hush_exit(G.آخری_exitcode);
	}
	/* mimic bash: exit 123abc == exit 255 + error msg */
	xfunc_error_retval = 255;
	/* bash: exit -2 == exit 254, no error msg */
	hush_exit(xatoi(argv[0]) & 0xff);
}

#اگر ENABLE_HUSH_TYPE
/* http://www.opengroup.org/onlinepubs/9699919799/utilities/type.html */
ساکن صحیح FAST_FUNC builtin_type(حرف **argv)
{
	صحیح ret = EXIT_SUCCESS;

	جب (*++argv) {
		ثابت حرف *type;
		حرف *راستہ_2 = تہی;

		اگر (0) {} /* make conditional compile easier below */
		/*else if (find_alias(*argv))
			type = "an alias";*/
# اگر ENABLE_HUSH_FUNCTIONS
		ورنہ اگر (ڈھونڈو_function(*argv))
			type = "a function";
# endif
		ورنہ اگر (ڈھونڈو_builtin(*argv))
			type = "a shell builtin";
		ورنہ اگر ((راستہ_2 = ڈھونڈو_اندر_راستہ(*argv)) != تہی)
			type = راستہ_2;
		ورنہ {
			bb_error_پیغام("type: %s: not found", *argv);
			ret = EXIT_FAILURE;
			جاری;
		}

		لکھو("%s is %s\n", *argv, type);
		آزاد(راستہ_2);
	}

	واپس ret;
}
#اگر_ختم

#اگر ENABLE_HUSH_READ
/* Interruptibility of read builtin in bash
 * (tested on bash-4.2.8 by sending signals (not by ^C)):
 *
 * Empty trap makes read ignore corresponding signal, for any signal.
 *
 * SIGINT:
 * - terminates non-interactive shell;
 * - interrupts read in interactive shell;
 * if it has non-empty trap:
 * - executes trap and returns to command prompt in interactive shell;
 * - executes trap and returns to read in non-interactive shell;
 * SIGTERM:
 * - is ignored (does not interrupt) read in interactive shell;
 * - terminates non-interactive shell;
 * if it has non-empty trap:
 * - executes trap and returns to read;
 * SIGHUP:
 * - terminates shell (regardless of interactivity);
 * if it has non-empty trap:
 * - executes trap and returns to read;
 * SIGCHLD from children:
 * - does not interrupt read regardless of interactivity:
 *   try: sleep 1 & read x; echo $x
 */
ساکن صحیح FAST_FUNC builtin_read(حرف **argv)
{
	ثابت حرف *r;
	ساخت builtin_read_params params;

	memset(&params, 0, حجم(params));

	/* "!": do not abort on errors.
	 * Option string must start with "sr" to match BUILTIN_READ_xxx
	 */
	params.read_flags = getopt32(argv,
# اگر BASH_READ_D
		IF_NOT_HUSH_BASH_COMPAT("^")
		"!srn:p:t:u:d:" IF_NOT_HUSH_BASH_COMPAT("\0" "-1"/*min 1 arg*/),
		&params.opt_n, &params.opt_p, &params.opt_t, &params.opt_u, &params.opt_d
# ورنہ
		IF_NOT_HUSH_BASH_COMPAT("^")
		"!srn:p:t:u:" IF_NOT_HUSH_BASH_COMPAT("\0" "-1"/*min 1 arg*/),
		&params.opt_n, &params.opt_p, &params.opt_t, &params.opt_u
# endif
//TODO: print "read: need variable name"
//for the case of !BASH "read" with no args (now it fails silently)
//(or maybe extend getopt32() to emit a message if "-1" fails)
	);
	اگر ((uint32_t)params.read_flags == (uint32_t)-1)
		واپس EXIT_FAILURE;
	argv += optind;
	params.argv = argv;
	params.setvar = سیٹ_local_var_from_halves;
	params.ifs = get_local_var_قدر("IFS"); /* can be NULL */

 again:
	r = shell_builtin_read(&params);

	اگر ((uintptr_t)r == 1 && errno == EINTR) {
		غیرعلامتی sig = check_and_تسلسل_traps();
		اگر (sig != SIGINT)
			جاؤ again;
	}

	اگر ((uintptr_t)r > 1) {
		bb_simple_error_پیغام(r);
		r = (حرف*)(uintptr_t)1;
	}

	واپس (uintptr_t)r;
}
#اگر_ختم

#اگر ENABLE_HUSH_UMASK
ساکن صحیح FAST_FUNC builtin_umask(حرف **argv)
{
	صحیح rc;
	mode_t نقاب;

	rc = 1;
	نقاب = umask(0);
	argv = skip_dash_dash(argv);
	اگر (argv[0]) {
		mode_t old_نقاب = نقاب;

		/* numeric umasks are taken as-is */
		/* symbolic umasks are inverted: "umask a=rx" calls umask(222) */
		اگر (!ہندسہ_ہے(argv[0][0]))
			نقاب ^= 0777;
		نقاب = bb_parse_منوال(argv[0], نقاب);
		اگر (!ہندسہ_ہے(argv[0][0]))
			نقاب ^= 0777;
		اگر ((غیرعلامتی)نقاب > 0777) {
			نقاب = old_نقاب;
			/* bash messages:
			 * bash: umask: 'q': invalid symbolic mode operator
			 * bash: umask: 999: octal number out of range
			 */
			bb_error_پیغام("%s: invalid mode '%s'", "umask", argv[0]);
			rc = 0;
		}
	} ورنہ {
		/* Mimic bash */
		لکھو("%04o\n", (غیرعلامتی) نقاب);
		/* fall through and restore mask which we set to 0 */
	}
	umask(نقاب);

	واپس !rc; /* rc != 0 - success */
}
#اگر_ختم

#اگر ENABLE_HUSH_EXPORT || ENABLE_HUSH_READONLY || ENABLE_HUSH_سیٹ || ENABLE_HUSH_TRAP
ساکن خالی print_escaped(ثابت حرف *s)
{
//TODO? bash "set" does not quote variables which contain only alnums and "%+,-./:=@_~",
// (but "export" quotes all variables, even with only these chars).
// I think quoting strings with %+,=~ looks better
// (example: "set" printing var== instead of var='=' looks strange)
// IOW: do not quote "-./:@_": / is used in pathnames, : in PATH, -._ often in file names, @ in emails

	اگر (*s == '\'')
		جاؤ squote;
	کرو {
		ثابت حرف *p = strchrnul(s, '\'');
		/* print 'xxxx', possibly just '' */
		لکھو("'%.*s'", (صحیح)(p - s), s);
		اگر (*p == '\0')
			توڑو;
		s = p;
 squote:
		/* s points to '; print "'''...'''" */
		حرف_لکھو('"');
		کرو حرف_لکھو('\''); جب (*++s == '\'');
		حرف_لکھو('"');
	} جب (*s);
}
#اگر_ختم

#اگر ENABLE_HUSH_EXPORT || ENABLE_HUSH_LOCAL || ENABLE_HUSH_READONLY
ساکن صحیح helper_export_local(حرف **argv, غیرعلامتی flags)
{
	کرو {
		حرف *name = *argv;
		ثابت حرف *name_انتہا = endofname(name);

		اگر (*name_انتہا == '\0') {
			ساخت variable *var, **vpp;

			vpp = get_ptr_مقصد_local_var(name);
			var = vpp ? *vpp : تہی;

			اگر (flags & SETFLAG_UNEXPORT) {
				/* export -n NAME (without =VALUE) */
				اگر (var) {
					var->flg_export = 0;
					debug_printf_env("%s: unsetenv '%s'\n", __func__, name);
					unsetenv(name);
				} /* else: export -n NOT_EXISTING_VAR: no-op */
				جاری;
			}
			اگر (flags & SETFLAG_EXPORT) {
				/* export NAME (without =VALUE) */
				اگر (var) {
					var->flg_export = 1;
					debug_printf_env("%s: putenv '%s'\n", __func__, var->varstr);
					putenv(var->varstr);
					جاری;
				}
			}
			اگر (flags & SETFLAG_MAKE_RO) {
				/* readonly NAME (without =VALUE) */
				اگر (var) {
					var->flg_read_only = 1;
					جاری;
				}
			}
# اگر ENABLE_HUSH_LOCAL
			/* Is this "local" bltin? */
			اگر (!(flags & (SETFLAG_EXPORT|SETFLAG_UNEXPORT|SETFLAG_MAKE_RO))) {
				غیرعلامتی lvl = flags >> SETFLAG_VARLVL_سرکاؤ;
				اگر (var && var->var_nest_level == lvl) {
					/* "local x=abc; ...; local x" - ignore second local decl */
					جاری;
				}
			}
# endif
			/* Exporting non-existing variable.
			 * bash does not put it in environment,
			 * but remembers that it is exported,
			 * and does put it in env when it is set later.
			 * We just set it to "" and export.
			 */
			/* Or, it's "local NAME" (without =VALUE).
			 * bash sets the value to "".
			 */
			/* Or, it's "readonly NAME" (without =VALUE).
			 * bash remembers NAME and disallows its creation
			 * in the future.
			 */
			name = xasprintf("%s=", name);
		} ورنہ {
			اگر (*name_انتہا != '=') {
				bb_error_پیغام("'%s': bad variable name", name);
				/* do not parse following argv[]s: */
				واپس 1;
			}
			/* (Un)exporting/making local NAME=VALUE */
			name = xstrdup(name);
			/* Testcase: export PS1='\w \$ ' */
			unbackslash(name);
		}
		debug_printf_env("%s: set_local_var('%s')\n", __func__, name);
		اگر (سیٹ_local_var(name, flags))
			واپس EXIT_FAILURE;
	} جب (*++argv);
	واپس EXIT_SUCCESS;
}
#اگر_ختم

#اگر ENABLE_HUSH_EXPORT
ساکن صحیح FAST_FUNC builtin_export(حرف **argv)
{
	غیرعلامتی opt_unexport;

# اگر ENABLE_HUSH_EXPORT_N
	/* "!": do not abort on errors */
	opt_unexport = getopt32(argv, "!n");
	اگر (opt_unexport == (uint32_t)-1)
		واپس EXIT_FAILURE;
	argv += optind;
# ورنہ
	opt_unexport = 0;
	argv++;
# endif

	اگر (argv[0] == تہی) {
		حرف **e = environ;
		اگر (e) {
			جب (*e) {
# اگر 0
				puts(*e++);
# ورنہ
				/* ash emits: export VAR='VAL'
				 * bash: declare -x VAR="VAL"
				 * we follow ash example */
				ثابت حرف *s = *e++;
				ثابت حرف *p = strchr(s, '=');

				اگر (!p) /* wtf? take next variable */
					جاری;
				/* "export VAR=" */
				لکھو("%s %.*s", "export", (صحیح)(p - s) + 1, s);
				print_escaped(p + 1);
				حرف_لکھو('\n');
# endif
			}
			/*fflush_all(); - done after each builtin anyway */
		}
		واپس EXIT_SUCCESS;
	}

	واپس helper_export_local(argv, opt_unexport ? SETFLAG_UNEXPORT : SETFLAG_EXPORT);
}
#اگر_ختم

#اگر ENABLE_HUSH_LOCAL
ساکن صحیح FAST_FUNC builtin_local(حرف **argv)
{
	اگر (G.func_nest_level == 0) {
		bb_error_پیغام("%s: not in a function", argv[0]);
		واپس EXIT_FAILURE; /* bash compat */
	}
//TODO? ash and bash support "local -" special form,
//which saves/restores $- around function call (including async returns, such as ^C)
//(IOW: it makes "set +/-..." effects local)
	argv++;
	/* Since all builtins run in a nested variable level,
	 * need to use level - 1 here. Or else the variable will be removed at once
	 * after builtin returns.
	 */
	واپس helper_export_local(argv, (G.var_nest_level - 1) << SETFLAG_VARLVL_سرکاؤ);
}
#اگر_ختم

#اگر ENABLE_HUSH_READONLY
ساکن صحیح FAST_FUNC builtin_readonly(حرف **argv)
{
	argv++;
	اگر (*argv == تہی) {
		/* bash: readonly [-p]: list all readonly VARs
		 * (-p has no effect in bash)
		 */
		ساخت variable *e;
		برائے (e = G.چوٹی_var; e; e = e->اگلا) {
			اگر (e->flg_read_only) {
				ثابت حرف *s = e->varstr;
				ثابت حرف *p = strchr(s, '=');

				اگر (!p) /* wtf? take next variable */
					جاری;
				/* "readonly VAR=" */
				لکھو("%s %.*s", "readonly", (صحیح)(p - s) + 1, s);
				print_escaped(p + 1);
				حرف_لکھو('\n');
			}
		}
		واپس EXIT_SUCCESS;
	}
	واپس helper_export_local(argv, SETFLAG_MAKE_RO);
}
#اگر_ختم

#اگر ENABLE_HUSH_UNSET
/* http://www.opengroup.org/onlinepubs/9699919799/utilities/V3_chap02.html#unset */
ساکن صحیح FAST_FUNC builtin_unset(حرف **argv)
{
	صحیح ret;
	غیرعلامتی opts;

	/* "!": do not abort on errors */
	/* "+": stop at 1st non-option */
	opts = getopt32(argv, "!+vf");
	اگر (opts == (غیرعلامتی)-1)
		واپس EXIT_FAILURE;
	اگر (opts == 3) {
		bb_simple_error_پیغام("unset: -v and -f are exclusive");
		واپس EXIT_FAILURE;
	}
	argv += optind;

	ret = EXIT_SUCCESS;
	جب (*argv) {
		اگر (!(opts & 2)) { /* not -f */
			اگر (unset_local_var(*argv)) {
				/* unset <nonexistent_var> doesn't fail.
				 * Error is when one tries to unset RO var.
				 * Message was printed by unset_local_var. */
				ret = EXIT_FAILURE;
			}
		}
# اگر ENABLE_HUSH_FUNCTIONS
		ورنہ {
			unset_func(*argv);
		}
# endif
		argv++;
	}
	واپس ret;
}
#اگر_ختم

#اگر ENABLE_HUSH_سیٹ
/* http://www.opengroup.org/onlinepubs/9699919799/utilities/V3_chap02.html#set
 * built-in 'set' handler
 * SUSv3 says:
 * set [-abCefhmnuvx] [-o option] [argument...]
 * set [+abCefhmnuvx] [+o option] [argument...]
 * set -- [argument...]
 * set -o
 * set +o
 * Implementations shall support the options in both their hyphen and
 * plus-sign forms. These options can also be specified as options to sh.
 * Examples:
 * Write out all variables and their values: set
 * Set $1, $2, and $3 and set "$#" to 3: set c a b
 * Turn on the -x and -v options: set -xv
 * Unset all positional parameters: set --
 * Set $1 to the value of x, even if it begins with '-' or '+': set -- "$x"
 * Set the positional parameters to the expansion of x, even if x expands
 * with a leading '-' or '+': set -- $x
 *
 * So far, we only support "set -- [argument...]" and some of the short names.
 */
ساکن صحیح FAST_FUNC builtin_سیٹ(حرف **argv)
{
	صحیح n;
	حرف **pp, **g_argv;
	حرف *arg = *++argv;

	اگر (arg == تہی) {
		ساخت variable *e;
		برائے (e = G.چوٹی_var; e; e = e->اگلا) {
			ثابت حرف *s = e->varstr;
			ثابت حرف *p = strchr(s, '=');

			اگر (!p) /* wtf? take next variable */
				جاری;
			/* var= */
			لکھو("%.*s", (صحیح)(p - s) + 1, s);
			print_escaped(p + 1);
			حرف_لکھو('\n');
		}
		واپس EXIT_SUCCESS;
	}

	کرو {
		اگر (strcmp(arg, "--") == 0) {
			++argv;
			جاؤ سیٹ_argv;
		}
		اگر (arg[0] != '+' && arg[0] != '-')
			توڑو;
		برائے (n = 1; arg[n]; ++n) {
			اگر (سیٹ_منوال((arg[0] == '-'), arg[n], argv[1])) {
				bb_error_پیغام("%s: %s: invalid option", "set", arg);
				واپس EXIT_FAILURE;
			}
			اگر (arg[n] == 'o' && argv[1])
				argv++;
		}
	} جب ((arg = *++argv) != تہی);
	/* Now argv[0] is 1st argument */

	اگر (arg == تہی)
		واپس EXIT_SUCCESS;
 سیٹ_argv:

	/* NB: G.global_argv[0] ($0) is never freed/changed */
	g_argv = G.global_argv;
	اگر (G.global_دلائل_malloced) {
		pp = g_argv;
		جب (*++pp)
			آزاد(*pp);
		g_argv[1] = تہی;
	} ورنہ {
		G.global_دلائل_malloced = 1;
		pp = xzalloc(حجم(pp[0]) * 2);
		pp[0] = g_argv[0]; /* retain $0 */
		g_argv = pp;
	}
	/* This realloc's G.global_argv */
	G.global_argv = pp = جوڑو_لڑیاں_مقصد_لڑیاں(g_argv, argv, /*dup:*/ 1);

	G.global_argc = 1 + string_array_len(pp + 1);

	واپس EXIT_SUCCESS;
}
#اگر_ختم

ساکن صحیح FAST_FUNC builtin_سرکاؤ(حرف **argv)
{
	صحیح n = 1;
	argv = skip_dash_dash(argv);
	اگر (argv[0]) {
		n = bb_strtou(argv[0], تہی, 10);
		اگر (errno || n < 0) {
			/* shared string with ash.c */
			bb_error_پیغام("Illegal number: %s", argv[0]);
			/*
			 * ash aborts in this case.
			 * bash prints error message and set $? to 1.
			 * Interestingly, for "shift 99999" bash does not
			 * print error message, but does set $? to 1
			 * (and does no shifting at all).
			 */
		}
	}
	اگر (n >= 0 && n < G.global_argc) {
		اگر (G_global_دلائل_malloced) {
			صحیح m = 1;
			جب (m <= n)
				آزاد(G.global_argv[m++]);
		}
		G.global_argc -= n;
		memmove(&G.global_argv[1], &G.global_argv[n+1],
				G.global_argc * حجم(G.global_argv[0]));
		واپس EXIT_SUCCESS;
	}
	واپس EXIT_FAILURE;
}

#اگر ENABLE_HUSH_GETOPTS
ساکن صحیح FAST_FUNC builtin_getopts(حرف **argv)
{
/* http://pubs.opengroup.org/onlinepubs/9699919799/utilities/getopts.html

TODO:
If a required argument is not found, and getopts is not silent,
a question mark (?) is placed in VAR, OPTARG is unset, and a
diagnostic message is printed.  If getopts is silent, then a
colon (:) is placed in VAR and OPTARG is set to the option
character found.

Test that VAR is a valid variable name?

"Whenever the shell is invoked, OPTIND shall be initialized to 1"
*/
	حرف cbuf[2];
	ثابت حرف *cp, *optstring, *var;
	صحیح c, n, exitcode, my_opterr;
	غیرعلامتی گنتی;

	optstring = *++argv;
	اگر (!optstring || !(var = *++argv)) {
		bb_simple_error_پیغام("usage: getopts OPTSTRING VAR [ARGS]");
		واپس EXIT_FAILURE;
	}

	اگر (argv[1])
		argv[0] = G.global_argv[0]; /* for error messages in getopt() */
	ورنہ
		argv = G.global_argv;
	cbuf[1] = '\0';

	my_opterr = 0;
	اگر (optstring[0] != ':') {
		cp = get_local_var_قدر("OPTERR");
		/* 0 if "OPTERR=0", 1 otherwise */
		my_opterr = (!cp || NOT_LONE_CHAR(cp, '0'));
	}

	/* getopts stops on first non-option. Add "+" to force that */
	/*if (optstring[0] != '+')*/ {
		حرف *s = alloca(لڑی_لمبائی(optstring) + 2);
		sprintf(s, "+%s", optstring);
		optstring = s;
	}

	/* Naively, now we should just
	 *	cp = get_local_var_value("OPTIND");
	 *	optind = cp ? atoi(cp) : 0;
	 *	optarg = NULL;
	 *	opterr = my_opterr;
	 *	c = getopt(string_array_len(argv), argv, optstring);
	 * and be done? Not so fast...
	 * Unlike normal getopt() usage in C programs, here
	 * each successive call will (usually) have the same argv[] CONTENTS,
	 * but not the ADDRESSES. Worse yet, it's possible that between
	 * invocations of "getopts", there will be calls to shell builtins
	 * which use getopt() internally. Example:
	 *	while getopts "abc" RES -a -bc -abc de; do
	 *		unset -ff func
	 *	done
	 * This would not work correctly: getopt() call inside "unset"
	 * modifies internal libc state which is tracking position in
	 * multi-option strings ("-abc"). At best, it can skip options
	 * or return the same option infinitely. With glibc implementation
	 * of getopt(), it would use outright invalid pointers and return
	 * garbage even _without_ "unset" mangling internal state.
	 *
	 * We resort to resetting getopt() state and calling it N times,
	 * until we get Nth result (or failure).
	 * (N == G.getopt_count is reset to 0 whenever OPTIND is [un]set).
	 */
	GETOPT_RESET();
	گنتی = 0;
	n = string_array_len(argv);
	کرو {
		optarg = تہی;
		opterr = (گنتی < G.getopt_گنتی) ? 0 : my_opterr;
		c = getopt(n, argv, optstring);
		اگر (c < 0)
			توڑو;
		گنتی++;
	} جب (گنتی <= G.getopt_گنتی);

	/* Set OPTIND. Prevent resetting of the magic counter! */
	سیٹ_local_var_from_halves("OPTIND", utoa(optind));
	G.getopt_گنتی = گنتی; /* "next time, give me N+1'th result" */
	GETOPT_RESET(); /* just in case */

	/* Set OPTARG */
	/* Always set or unset, never left as-is, even on exit/error:
	 * "If no option was found, or if the option that was found
	 * does not have an option-argument, OPTARG shall be unset."
	 */
	cp = optarg;
	اگر (c == '?') {
		/* If ":optstring" and unknown option is seen,
		 * it is stored to OPTARG.
		 */
		اگر (optstring[1] == ':') {
			cbuf[0] = optopt;
			cp = cbuf;
		}
	}
	اگر (cp)
		سیٹ_local_var_from_halves("OPTARG", cp);
	ورنہ
		unset_local_var("OPTARG");

	/* Convert -1 to "?" */
	exitcode = EXIT_SUCCESS;
	اگر (c < 0) { /* -1: end of options */
		exitcode = EXIT_FAILURE;
		c = '?';
	}

	/* Set VAR */
	cbuf[0] = c;
	سیٹ_local_var_from_halves(var, cbuf);

	واپس exitcode;
}
#اگر_ختم

ساکن صحیح FAST_FUNC builtin_ماخذ(حرف **argv)
{
	حرف *arg_راستہ, *filename;
	HFILE *داخلہ;
	save_arg_t sv;
	حرف *دلائل_need_save;
#اگر ENABLE_HUSH_FUNCTIONS
	smallint sv_flg;
#اگر_ختم

	argv = skip_dash_dash(argv);
	filename = argv[0];
	اگر (!filename) {
		/* bash says: "bash: .: filename argument required" */
		واپس 2; /* bash compat */
	}
	arg_راستہ = تہی;
	اگر (!strchr(filename, '/')) {
		arg_راستہ = ڈھونڈو_اندر_راستہ(filename);
		اگر (arg_راستہ)
			filename = arg_راستہ;
		ورنہ اگر (!ENABLE_HUSH_BASH_ماخذ_CURDIR) {
			errno = ENOENT;
			bb_simple_perror_پیغام(filename);
			واپس EXIT_FAILURE;
		}
	}
	داخلہ = hfopen(filename);
	آزاد(arg_راستہ);
	اگر (!داخلہ) {
		bb_perror_پیغام("%s", filename);
		/* POSIX: non-interactive shell should abort here,
		 * not merely fail. So far no one complained :)
		 */
		واپس EXIT_FAILURE;
	}

#اگر ENABLE_HUSH_FUNCTIONS
	sv_flg = G_flag_return_اندر_پیش_رفت;
	/* "we are inside sourced file, ok to use return" */
	G_flag_return_اندر_پیش_رفت = -1;
#اگر_ختم
	دلائل_need_save = argv[1]; /* used as a boolean variable */
	اگر (دلائل_need_save)
		save_and_replace_G_دلائل(&sv, argv);

	/* "false; . ./empty_line; echo Zero:$?" should print 0 */
	G.آخری_exitcode = 0;
	parse_and_تسلسل_file(داخلہ);
	hfclose(داخلہ);

	اگر (دلائل_need_save) /* can't use argv[1] instead: "shift" can mangle it */
		restore_G_دلائل(&sv, argv);
#اگر ENABLE_HUSH_FUNCTIONS
	G_flag_return_اندر_پیش_رفت = sv_flg;
#اگر_ختم

	واپس G.آخری_exitcode;
}

#اگر ENABLE_HUSH_TRAP
ساکن صحیح FAST_FUNC builtin_trap(حرف **argv)
{
	صحیح sig;
	حرف *new_cmd;

	اگر (!G_traps)
		G_traps = xzalloc(حجم(G_traps[0]) * NSIG);

	argv++;
	اگر (!*argv) {
		صحیح i;
		/* No args: print all trapped */
		برائے (i = 0; i < NSIG; ++i) {
			اگر (G_traps[i]) {
				لکھو("trap -- ");
				print_escaped(G_traps[i]);
				/* note: bash adds "SIG", but only if invoked
				 * as "bash". If called as "sh", or if set -o posix,
				 * then it prints short signal names.
				 * We are printing short names: */
				لکھو(" %s\n", get_signame(i));
			}
		}
		/*fflush_all(); - done after each builtin anyway */
		واپس EXIT_SUCCESS;
	}

	new_cmd = تہی;
	/* If first arg is a number: reset all specified signals */
	sig = bb_strtou(*argv, تہی, 10);
	اگر (errno == 0) {
		صحیح ret;
 process_sig_list:
		ret = EXIT_SUCCESS;
		جب (*argv) {
			sighandler_t handler;

			sig = get_signum(*argv++);
			اگر (sig < 0) {
				ret = EXIT_FAILURE;
				/* Mimic bash message exactly */
				bb_error_پیغام("trap: %s: invalid signal specification", argv[-1]);
				جاری;
			}

			آزاد(G_traps[sig]);
			G_traps[sig] = xstrdup(new_cmd);

			debug_printf("trap: setting SIG%s (%i) to '%s'\n",
				get_signame(sig), sig, G_traps[sig]);

			/* There is no signal for 0 (EXIT) */
			اگر (sig == 0)
				جاری;

			اگر (new_cmd)
				handler = (new_cmd[0] ? ریکارڈ_pending_signo : SIG_IGN);
			ورنہ
				/* We are removing trap handler */
				handler = pick_sighandler(sig);
			install_sighandler(sig, handler);
		}
		واپس ret;
	}

	اگر (!argv[1]) { /* no second arg */
		bb_simple_error_پیغام("trap: invalid arguments");
		واپس EXIT_FAILURE;
	}

	/* First arg is "-": reset all specified to default */
	/* First arg is "--": skip it, the rest is "handler SIGs..." */
	/* Everything else: set arg as signal handler
	 * (includes "" case, which ignores signal) */
	اگر (argv[0][0] == '-') {
		اگر (argv[0][1] == '\0') { /* "-" */
			/* new_cmd remains NULL: "reset these sigs" */
			جاؤ reset_traps;
		}
		اگر (argv[0][1] == '-' && argv[0][2] == '\0') { /* "--" */
			argv++;
		}
		/* else: "-something", no special meaning */
	}
	new_cmd = *argv;
 reset_traps:
	argv++;
	جاؤ process_sig_list;
}
#اگر_ختم

#اگر ENABLE_HUSH_کام
ساکن ساخت pipe *parse_jobspec(ثابت حرف *str)
{
	ساخت pipe *pi;
	غیرعلامتی jobnum;

	اگر (لڑی_سے_پڑھو(str, "%%%u", &jobnum) != 1) {
		اگر (str[0] != '%'
		 || (str[1] != '%' && str[1] != '+' && str[1] != '\0')
		) {
			bb_error_پیغام("bad argument '%s'", str);
			واپس تہی;
		}
		/* It is "%%", "%+" or "%" - current job */
		jobnum = G.آخری_jobid;
		اگر (jobnum == 0) {
			bb_simple_error_پیغام("no current job");
			واپس تہی;
		}
	}
	برائے (pi = G.کام_list; pi; pi = pi->اگلا) {
		اگر (pi->jobid == jobnum) {
			واپس pi;
		}
	}
	bb_error_پیغام("%u: no such job", jobnum);
	واپس تہی;
}

ساکن صحیح FAST_FUNC builtin_jobs(حرف **argv غیر_مستعمل_PARAM)
{
	ساخت pipe *کام;
	ثابت حرف *حالت_string;

	checkjobs(تہی, 0 /*(no pid to wait for)*/);
	برائے (کام = G.کام_list; کام; کام = کام->اگلا) {
		اگر (کام->alive_cmds == کام->stopped_cmds)
			حالت_string = "Stopped";
		ورنہ
			حالت_string = "Running";

		لکھو(کام_حالت_FORMAT, کام->jobid, حالت_string, کام->cmdtext);
	}

	clean_اوپر_آخری_dead_کام();

	واپس EXIT_SUCCESS;
}

/* built-in 'fg' and 'bg' handler */
ساکن صحیح FAST_FUNC builtin_fg_bg(حرف **argv)
{
	صحیح i;
	ساخت pipe *pi;

	اگر (!G_interactive_fd)
		واپس EXIT_FAILURE;

	/* If they gave us no args, assume they want the last backgrounded task */
	اگر (!argv[1]) {
		برائے (pi = G.کام_list; pi; pi = pi->اگلا) {
			اگر (pi->jobid == G.آخری_jobid) {
				جاؤ ملا;
			}
		}
		bb_error_پیغام("%s: no current job", argv[0]);
		واپس EXIT_FAILURE;
	}

	pi = parse_jobspec(argv[1]);
	اگر (!pi)
		واپس EXIT_FAILURE;
 ملا:
	/* TODO: bash prints a string representation
	 * of job being foregrounded (like "sleep 1 | cat") */
	اگر (argv[0][0] == 'f' && G_saved_tty_pgrp) {
		/* Put the job into the foreground. */
		tcsetpgrp(G_interactive_fd, pi->pgrp);
	}

	/* Restart the processes in the job */
	debug_printf_jobs("reviving %d procs, pgrp %d\n", pi->عدد_cmds, pi->pgrp);
	برائے (i = 0; i < pi->عدد_cmds; i++) {
		debug_printf_jobs("reviving pid %d\n", pi->cmds[i].pid);
	}
	pi->stopped_cmds = 0;

	i = kill(- pi->pgrp, SIGCONT);
	اگر (i < 0) {
		اگر (errno == ESRCH) {
			delete_ختم_شدہ_کام(pi);
			واپس EXIT_SUCCESS;
		}
		bb_simple_perror_پیغام("kill (SIGCONT)");
	}

	اگر (argv[0][0] == 'f') {
		remove_کام_from_جدول(pi); /* FG job shouldn't be in job table */
		واپس checkjobs_and_fg_shell(pi);
	}
	واپس EXIT_SUCCESS;
}
#اگر_ختم

#اگر ENABLE_HUSH_KILL
ساکن صحیح FAST_FUNC builtin_kill(حرف **argv)
{
	صحیح ret = 0;

# اگر ENABLE_HUSH_کام
	اگر (argv[1] && strcmp(argv[1], "-l") != 0) {
		صحیح i = 1;

		کرو {
			ساخت pipe *pi;
			حرف *dst;
			صحیح j, n;

			اگر (argv[i][0] != '%')
				جاری;
			/*
			 * "kill %N" - job kill
			 * Converting to pgrp / pid kill
			 */
			pi = parse_jobspec(argv[i]);
			اگر (!pi) {
				/* Eat bad jobspec */
				j = i;
				کرو {
					j++;
					argv[j - 1] = argv[j];
				} جب (argv[j]);
				ret = 1;
				i--;
				جاری;
			}
			/*
			 * In jobs started under job control, we signal
			 * entire process group by kill -PGRP_ID.
			 * This happens, f.e., in interactive shell.
			 *
			 * Otherwise, we signal each child via
			 * kill PID1 PID2 PID3.
			 * Testcases:
			 * sh -c 'sleep 1|sleep 1 & kill %1'
			 * sh -c 'true|sleep 2 & sleep 1; kill %1'
			 * sh -c 'true|sleep 1 & sleep 2; kill %1'
			 */
			n = G_interactive_fd ? 1 : pi->عدد_cmds;
			dst = alloca(n * حجم(صحیح)*4);
			argv[i] = dst;
			اگر (G_interactive_fd)
				dst += sprintf(dst, " -%u", (صحیح)pi->pgrp);
			ورنہ برائے (j = 0; j < n; j++) {
				ساخت command *cmd = &pi->cmds[j];
				/* Skip exited members of the job */
				اگر (cmd->pid == 0)
					جاری;
				/*
				 * kill_main has matching code to expect
				 * leading space. Needed to not confuse
				 * negative pids with "kill -SIGNAL_NO" syntax
				 */
				dst += sprintf(dst, " %u", (صحیح)cmd->pid);
			}
			*dst = '\0';
		} جب (argv[++i]);
	}
# endif

	اگر (argv[1] || ret == 0) {
		ret = تسلسل_applet_main(argv, kill_main);
	}
	/* else: ret = 1, "kill %bad_jobspec" case */
	واپس ret;
}
#اگر_ختم

#اگر ENABLE_HUSH_WAIT
/* http://www.opengroup.org/onlinepubs/9699919799/utilities/wait.html */
# اگر !ENABLE_HUSH_کام
#  define wait_for_بچہ_or_signal(pipe,pid) wait_for_بچہ_or_signal(pid)
# endif
ساکن صحیح wait_for_بچہ_or_signal(ساخت pipe *waitfor_pipe, pid_t waitfor_pid)
{
	صحیح ret = 0;
	برائے (;;) {
		صحیح sig;
		sigset_t oldset;

		اگر (!sigisemptyset(&G.pending_سیٹ))
			جاؤ check_sig;

		/* waitpid is not interruptible by SA_RESTARTed
		 * signals which we use. Thus, this ugly dance:
		 */

		/* Make sure possible SIGCHLD is stored in kernel's
		 * pending signal mask before we call waitpid.
		 * Or else we may race with SIGCHLD, lose it,
		 * and get stuck in sigsuspend...
		 */
		sigfillset(&oldset); /* block all signals, remember old set */
		sigprocmask2(SIG_SETMASK, &oldset);

		اگر (!sigisemptyset(&G.pending_سیٹ)) {
			/* Crap! we raced with some signal! */
			جاؤ restore;
		}

		/*errno = 0; - checkjobs does this */
/* Can't pass waitfor_pipe into checkjobs(): it won't be interruptible */
		ret = checkjobs(تہی, waitfor_pid); /* waitpid(WNOHANG) inside */
		debug_printf_exec("checkjobs:%d\n", ret);
# اگر ENABLE_HUSH_کام
		اگر (waitfor_pipe) {
			صحیح rcode = کام_exited_or_stopped(waitfor_pipe);
			debug_printf_exec("job_exited_or_stopped:%d\n", rcode);
			اگر (rcode >= 0) {
				ret = rcode;
				sigprocmask(SIG_SETMASK, &oldset, تہی);
				توڑو;
			}
		}
# endif
		/* if ECHILD, there are no children (ret is -1 or 0) */
		/* if ret == 0, no children changed state */
		/* if ret != 0, it's exitcode+1 of exited waitfor_pid child */
		اگر (errno == ECHILD || ret) {
			ret--;
			اگر (ret < 0) /* if ECHILD, may need to fix "ret" */
				ret = 0;
# اگر ENABLE_HUSH_BASH_COMPAT
			اگر (waitfor_pid == -1 && errno == ECHILD) {
				/* exitcode of "wait -n" with no children is 127, not 0 */
				ret = 127;
			}
# endif
			sigprocmask(SIG_SETMASK, &oldset, تہی);
			توڑو;
		}
		/* Wait for SIGCHLD or any other signal */
		/* It is vitally important for sigsuspend that SIGCHLD has non-DFL handler! */
		/* Note: sigsuspend invokes signal handler */
		sigsuspend(&oldset);
		/* ^^^ add "sigdelset(&oldset, SIGCHLD)" before sigsuspend
		 * to make sure SIGCHLD is not masked off?
		 * It was reported that this:
		 *	fn() { : | return; }
		 *	shopt -s lastpipe
		 *	fn
		 *	exec hush SCRIPT
		 * under bash 4.4.23 runs SCRIPT with SIGCHLD masked,
		 * making "wait" commands in SCRIPT block forever.
		 */
 restore:
		sigprocmask(SIG_SETMASK, &oldset, تہی);
 check_sig:
		/* So, did we get a signal? */
		sig = check_and_تسلسل_traps();
		اگر (sig /*&& sig != SIGCHLD - always true */) {
			/* Do this for any (non-ignored) signal, not only for ^C */
			ret = 128 | sig;
			توڑو;
		}
		/* SIGCHLD, or no signal, or ignored one, such as SIGQUIT. Repeat */
	}
	واپس ret;
}

ساکن صحیح FAST_FUNC builtin_wait(حرف **argv)
{
	صحیح ret;
	صحیح حالت_2;

	argv = skip_dash_dash(argv);
# اگر ENABLE_HUSH_BASH_COMPAT
	اگر (argv[0] && strcmp(argv[0], "-n") == 0) {
		/* wait -n */
		/* (bash accepts "wait -n PID" too and ignores PID) */
		G.dead_کام_exitcode = -1;
		واپس wait_for_بچہ_or_signal(تہی, -1 /*no job, wait for one job*/);
	}
# endif
	اگر (argv[0] == تہی) {
		/* Don't care about wait results */
		/* Note 1: must wait until there are no more children */
		/* Note 2: must be interruptible */
		/* Examples:
		 * $ sleep 3 & sleep 6 & wait
		 * [1] 30934 sleep 3
		 * [2] 30935 sleep 6
		 * [1] Done                   sleep 3
		 * [2] Done                   sleep 6
		 * $ sleep 3 & sleep 6 & wait
		 * [1] 30936 sleep 3
		 * [2] 30937 sleep 6
		 * [1] Done                   sleep 3
		 * ^C <-- after ~4 sec from keyboard
		 * $
		 */
		واپس wait_for_بچہ_or_signal(تہی, 0 /*no job and no pid to wait for*/);
	}

	کرو {
		pid_t pid = bb_strtou(*argv, تہی, 10);
		اگر (errno || pid <= 0) {
# اگر ENABLE_HUSH_کام
			اگر (argv[0][0] == '%') {
				ساخت pipe *wait_pipe;
				ret = 127; /* bash compat for bad jobspecs */
				wait_pipe = parse_jobspec(*argv);
				اگر (wait_pipe) {
					ret = کام_exited_or_stopped(wait_pipe);
					اگر (ret < 0) {
						ret = wait_for_بچہ_or_signal(wait_pipe, 0);
					} ورنہ {
						/* waiting on "last dead job" removes it */
						clean_اوپر_آخری_dead_کام();
					}
				}
				/* else: parse_jobspec() already emitted error msg */
				جاری;
			}
# endif
			/* mimic bash message */
			bb_error_پیغام("wait: '%s': not a pid or valid job spec", *argv);
			ret = EXIT_FAILURE;
			جاری; /* bash checks all argv[] */
		}

		/* Do we have such child? */
		ret = waitpid(pid, &حالت_2, WNOHANG);
		اگر (ret < 0) {
			/* No */
			ret = 127;
			اگر (errno == ECHILD) {
				اگر (pid == G.آخری_bg_pid) {
					/* "wait $!" but last bg task has already exited. Try:
					 * (sleep 1; exit 3) & sleep 2; echo $?; wait $!; echo $?
					 * In bash it prints exitcode 0, then 3.
					 * In dash, it is 127.
					 */
					ret = G.آخری_bg_pid_exitcode;
				} ورنہ {
					/* Example: "wait 1". mimic bash message */
					bb_error_پیغام("wait: pid %u is not a child of this shell", (غیرعلامتی)pid);
				}
			} ورنہ {
				/* ??? */
				bb_perror_پیغام("wait %s", *argv);
			}
			جاری; /* bash checks all argv[] */
		}
		اگر (ret == 0) {
			/* Yes, and it still runs */
			ret = wait_for_بچہ_or_signal(تہی, pid);
		} ورنہ {
			/* Yes, and it just exited */
			process_wait_نتیجہ(تہی, pid, حالت_2);
			ret = WEXITSTATUS(حالت_2);
			اگر (WIFSIGNALED(حالت_2))
				ret = 128 | WTERMSIG(حالت_2);
		}
	} جب (*++argv);

	واپس ret;
}
#اگر_ختم

#اگر ENABLE_HUSH_LOOPS || ENABLE_HUSH_FUNCTIONS
ساکن غیرعلامتی parse_numeric_argv1(حرف **argv, غیرعلامتی def, غیرعلامتی def_min)
{
	اگر (argv[1]) {
		def = bb_strtou(argv[1], تہی, 10);
		اگر (errno || def < def_min || argv[2]) {
			bb_error_پیغام("%s: bad arguments", argv[0]);
			def = UINT_MAX;
		}
	}
	واپس def;
}
#اگر_ختم

#اگر ENABLE_HUSH_LOOPS
ساکن صحیح FAST_FUNC builtin_break(حرف **argv)
{
	غیرعلامتی گہرائی;
	اگر (G.گہرائی_کا_loop == 0) {
		bb_error_پیغام("%s: only meaningful in a loop", argv[0]);
		/* if we came from builtin_continue(), need to undo "= 1" */
		G.flag_break_continue = 0;
		واپس EXIT_SUCCESS; /* bash compat */
	}
	G.flag_break_continue++; /* BC_BREAK = 1, or BC_CONTINUE = 2 */

	G.گہرائی_break_continue = گہرائی = parse_numeric_argv1(argv, 1, 1);
	اگر (گہرائی == UINT_MAX)
		G.flag_break_continue = BC_BREAK;
	اگر (G.گہرائی_کا_loop < گہرائی)
		G.گہرائی_break_continue = G.گہرائی_کا_loop;

	واپس EXIT_SUCCESS;
}

ساکن صحیح FAST_FUNC builtin_continue(حرف **argv)
{
	G.flag_break_continue = 1; /* BC_CONTINUE = 2 = 1+1 */
	واپس builtin_break(argv);
}
#اگر_ختم

#اگر ENABLE_HUSH_FUNCTIONS
ساکن صحیح FAST_FUNC builtin_return(حرف **argv)
{
	صحیح rc;

	اگر (G_flag_return_اندر_پیش_رفت != -1) {
		bb_error_پیغام("%s: not in a function or sourced script", argv[0]);
		واپس EXIT_FAILURE; /* bash compat */
	}

	G_flag_return_اندر_پیش_رفت = 1;

	/* bash:
	 * out of range: wraps around at 256, does not error out
	 * non-numeric param:
	 * f() { false; return qwe; }; f; echo $?
	 * bash: return: qwe: numeric argument required  <== we do this
	 * 255  <== we also do this
	 */
	rc = parse_numeric_argv1(argv, G.آخری_exitcode, 0);
# اگر ENABLE_HUSH_TRAP
	اگر (argv[1]) { /* "return ARG" inside a running trap sets $? */
		debug_printf_exec("G.return_exitcode=%d\n", rc);
		G.return_exitcode = rc;
	}
# endif
	واپس rc;
}
#اگر_ختم

#اگر ENABLE_HUSH_TIMES
ساکن صحیح FAST_FUNC builtin_times(حرف **argv غیر_مستعمل_PARAM)
{
	ساکن ثابت uint8_t times_tbl[] ALIGN1 = {
		' ',  offsetof(ساخت tms, tms_utime),
		'\n', offsetof(ساخت tms, tms_stime),
		' ',  offsetof(ساخت tms, tms_cutime),
		'\n', offsetof(ساخت tms, tms_cstime),
		0
	};
	ثابت uint8_t *p;
	غیرعلامتی clk_tck;
	ساخت tms buf;

	clk_tck = bb_clk_tck();

	times(&buf);
	p = times_tbl;
	کرو {
		غیرعلامتی sec, frac;
		غیرعلامتی طویل t;
		t = *(clock_t *)(((حرف *) &buf) + p[1]);
		sec = t / clk_tck;
		frac = t % clk_tck;
		لکھو("%um%u.%03us%c",
			sec / 60, sec % 60,
			(frac * 1000) / clk_tck,
			p[0]);
		p += 2;
	} جب (*p);

	واپس EXIT_SUCCESS;
}
#اگر_ختم

#اگر ENABLE_HUSH_MEMLEAK
ساکن صحیح FAST_FUNC builtin_memleak(حرف **argv غیر_مستعمل_PARAM)
{
	خالی *p;
	غیرعلامتی طویل l;

# ifdef M_TRIM_THRESHOLD
	/* Optional. Reduces probability of false positives */
	malloc_trim(0);
# endif
	/* Crude attempt to find where "free memory" starts,
	 * sans fragmentation. */
	p = حافظہ_دو(240);
	l = (غیرعلامتی طویل)p;
	آزاد(p);
	p = حافظہ_دو(3400);
	اگر (l < (غیرعلامتی طویل)p) l = (غیرعلامتی طویل)p;
	آزاد(p);

# اگر 0  /* debug */
	{
		ساخت mallinfo mi = mallinfo();
		لکھو("top alloc:0x%lx malloced:%d+%d=%d\n", l,
			mi.arena, mi.hblkhd, mi.arena + mi.hblkhd);
	}
# endif

	اگر (!G.memleak_قدر)
		G.memleak_قدر = l;

	l -= G.memleak_قدر;
	اگر ((طویل)l < 0)
		l = 0;
	l /= 1024;
	اگر (l > 127)
		l = 127;

	/* Exitcode is "how many kilobytes we leaked since 1st call" */
	واپس l;
}
#اگر_ختم
#اگر_ختم /* !__U_BOOT__ */
