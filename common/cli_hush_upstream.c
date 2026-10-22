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

#اگر_تعریف_نشده __U_BOOT__
#إذا !(defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) \
	|| defined(__APPLE__) \
    )
# include <malloc.h>   /* for malloc_trim */
#پایان_اگر
#ضمن <glob.h>
/* #include <dmalloc.h> */
#إذا ENABLE_HUSH_CASE
# include <fnmatch.h>
#پایان_اگر
#ضمن <sys/times.h>
#ضمن <sys/utsname.h> /* for setting $HOSTNAME */

#ضمن "busybox.h"  /* for APPLET_IS_NOFORK/NOEXEC */
#ضمن "unicode.h"
#ضمن "shell_common.h"
#ضمن "math.h"
#ضمن "match.h"
#إذا ENABLE_HUSH_RANDOM_SUPPORT
# include "random.h"
#وإلا
# define CLEAR_RANDOM_T(rnd) ((فراغ)0)
#پایان_اگر
#اگر_تعریف_نشده O_CLOEXEC
# define O_CLOEXEC 0
#پایان_اگر
#اگر_تعریف_نشده F_DUPFD_CLOEXEC
# define F_DUPFD_CLOEXEC F_DUPFD
#پایان_اگر

#إذا ENABLE_FEATURE_SH_EMBEDDED_SCRIPTS && !ENABLE_SHELL_ASH
# include "embedded_scripts.h"
#وإلا
# define رقم_SCRIPTS 0
#پایان_اگر
#پایان_اگر /* !__U_BOOT__ */

/* So far, all bash compat is controlled by one config option */
/* Separate defines document which part of code implements what */
#تعریف_کن BASH_الگو_SUBST ENABLE_HUSH_BASH_COMPAT
#تعریف_کن BASH_SUBSTR        ENABLE_HUSH_BASH_COMPAT
#تعریف_کن BASH_منبع        ENABLE_HUSH_BASH_COMPAT
#تعریف_کن BASH_DOLLAR_SQUOTE ENABLE_HUSH_BASH_COMPAT
#تعریف_کن BASH_HOSTNAME_VAR  ENABLE_HUSH_BASH_COMPAT
#تعریف_کن BASH_EPOCH_VARS    ENABLE_HUSH_BASH_COMPAT
#تعریف_کن BASH_TEST2         (ENABLE_HUSH_BASH_COMPAT && ENABLE_HUSH_TEST)
#تعریف_کن BASH_READ_D        ENABLE_HUSH_BASH_COMPAT

/* Build knobs */
#تعریف_کن LEAK_HUNTING 0
#تعریف_کن BUILD_AS_NOMMU 0
/* Enable/disable sanity checks. Ok to enable in production,
 * only adds a bit of bloat. Set to >1 to get non-production level verbosity.
 * Keeping 1 for now even in released versions.
 */
#تعریف_کن HUSH_DEBUG 1
/* Slightly bigger (+200 bytes), but faster hush.
 * So far it only enables a trick with counting SIGCHLDs and forks,
 * which allows us to do fewer waitpid's.
 * (we can detect a case where neither forks were done nor SIGCHLDs happened
 * and therefore waitpid will return the same result as last time)
 */
#تعریف_کن ENABLE_HUSH_FAST 0
/* TODO: implement simplified code for users which do not need ${var%...} ops
 * So far ${var%...} ops are always enabled:
 */
#تعریف_کن ENABLE_HUSH_DOLLAR_OPS 1

#إذا BUILD_AS_NOMMU
# undef BB_MMU
# undef USE_FOR_NOMMU
# undef USE_FOR_MMU
# define BB_MMU 0
# define USE_FOR_NOMMU(...) __VA_ARGS__
# define USE_FOR_MMU(...)
#پایان_اگر

#اگر_تعریف_نشده __U_BOOT__
#ضمن "NUM_APPLETS.h"
#إذا رقم_APPLETS == 1
/* STANDALONE does not make sense, and won't compile */
# undef ENABLE_FEATURE_SH_STANDALONE
# undef IF_FEATURE_SH_STANDALONE
# undef IF_NOT_FEATURE_SH_STANDALONE
# define ENABLE_FEATURE_SH_STANDALONE 0
# define IF_FEATURE_SH_STANDALONE(...)
# define IF_NOT_FEATURE_SH_STANDALONE(...) __VA_ARGS__
#پایان_اگر
#پایان_اگر /* __U_BOOT__ */

#إذا !ENABLE_HUSH_INTERACTIVE
# undef ENABLE_FEATURE_EDITING
# define ENABLE_FEATURE_EDITING 0
# undef ENABLE_FEATURE_EDITING_FANCY_PROMPT
# define ENABLE_FEATURE_EDITING_FANCY_PROMPT 0
# undef ENABLE_FEATURE_EDITING_SAVE_ON_EXIT
# define ENABLE_FEATURE_EDITING_SAVE_ON_EXIT 0
#پایان_اگر

/* Do we support ANY keywords? */
#إذا ENABLE_HUSH_IF || ENABLE_HUSH_LOOPS || ENABLE_HUSH_CASE
# define HAS_KEYWORDS 1
# define IF_HAS_KEYWORDS(...) __VA_ARGS__
# define IF_HAS_NO_KEYWORDS(...)
#وإلا
# define HAS_KEYWORDS 0
# define IF_HAS_KEYWORDS(...)
# define IF_HAS_NO_KEYWORDS(...) __VA_ARGS__
#پایان_اگر

/* If you comment out one of these below, it will be #defined later
 * to perform debug printfs to stderr: */
#تعریف_کن debug_printf(...)         افعل {} طالما (0)
/* Finer-grained debug switches */
#تعریف_کن debug_printf_parse(...)   افعل {} طالما (0)
#تعریف_کن debug_printf_heredoc(...) افعل {} طالما (0)
#تعریف_کن debug_print_درخت(a, b)    افعل {} طالما (0)
#تعریف_کن debug_printf_exec(...)    افعل {} طالما (0)
#تعریف_کن debug_printf_env(...)     افعل {} طالما (0)
#تعریف_کن debug_printf_jobs(...)    افعل {} طالما (0)
#تعریف_کن debug_printf_expand(...)  افعل {} طالما (0)
#تعریف_کن debug_printf_varexp(...)  افعل {} طالما (0)
#تعریف_کن debug_printf_glob(...)    افعل {} طالما (0)
#تعریف_کن debug_printf_redir(...)   افعل {} طالما (0)
#تعریف_کن debug_printf_list(...)    افعل {} طالما (0)
#تعریف_کن debug_printf_subst(...)   افعل {} طالما (0)
#تعریف_کن debug_printf_prompt(...)  افعل {} طالما (0)
#تعریف_کن debug_printf_clean(...)   افعل {} طالما (0)

#تعریف_کن ERR_PTR ((فراغ*)(دراز)1)

#تعریف_کن کار_وضعیت_FORMAT    "[%u] %-22s %.40s\n"

#تعریف_کن _SPECIAL_VARS_STR     "_*@$!?#-"
#اگر_تعریف_نشده __U_BOOT__
#تعریف_کن SPECIAL_VARS_STR     ("_*@$!?#-" + 1)
#تعریف_کن NUMERIC_SPECVARS_STR ("_*@$!?#-" + 3)
#وإلا /* __U_BOOT__ */
#تعریف_کن SPECIAL_VARS_STR     "*@$!?#-"
#تعریف_کن NUMERIC_SPECVARS_STR "$!?#-"
#پایان_اگر /* __U_BOOT__ */
#إذا BASH_الگو_SUBST
/* Support / and // replace ops */
/* Note that // is stored as \ in "encoded" string representation */
# define VAR_ENCODED_SUBST_OPS      "\\/%#:-=+?"
# define VAR_SUBST_OPS             ("\\/%#:-=+?" + 1)
# define MINUS_PLUS_EQUAL_QUESTION ("\\/%#:-=+?" + 5)
#وإلا
# define VAR_ENCODED_SUBST_OPS      "%#:-=+?"
# define VAR_SUBST_OPS              "%#:-=+?"
# define MINUS_PLUS_EQUAL_QUESTION ("%#:-=+?" + 3)
#پایان_اگر

#تعریف_کن SPECIAL_VAR_نماد_STR "\3"
#تعریف_کن SPECIAL_VAR_نماد       3
/* The "variable" with name "\1" emits string "\3". Testcase: "echo ^C" */
#تعریف_کن SPECIAL_VAR_QUOTED_SVS   1

بنية variable;

ساكن ثابت حرف hush_نسخه_str[] ALIGN1 = "HUSH_VERSION="BB_VER;

/* This supports saving pointers malloced in vfork child,
 * to be freed in the parent.
 */
#إذا !BB_MMU
تعريفنوع بنية nommu_save_t {
	بنية variable *old_vars;
	حرف **argv;
	حرف **argv_from_re_execing;
} nommu_save_t;
#پایان_اگر

تعداد {
	RES_NONE  = 0,
#إذا ENABLE_HUSH_IF
	RES_IF    ,
	RES_THEN  ,
	RES_ELIF  ,
	RES_ELSE  ,
	RES_FI    ,
#پایان_اگر
#إذا ENABLE_HUSH_LOOPS
	RES_FOR   ,
	RES_WHILE ,
	RES_UNTIL ,
	RES_DO    ,
	RES_انجامشده  ,
#پایان_اگر
#إذا ENABLE_HUSH_LOOPS || ENABLE_HUSH_CASE
	RES_درون    ,
#پایان_اگر
#إذا ENABLE_HUSH_CASE
	RES_CASE  ,
	/* three pseudo-keywords support contrived "case" syntax: */
	RES_CASE_درون,   /* "case ... IN", turns into RES_MATCH when IN is observed */
	RES_تطابق ,    /* "word)" */
	RES_CASE_BODY, /* "this command is inside CASE" */
	RES_ESAC  ,
#پایان_اگر
	RES_XXXX  ,
	RES_SNTX
};

تعريفنوع بنية o_string {
	حرف *داده;
	صحيح درازی; /* position where data is appended */
	صحيح maxlen;
	صحيح o_expflags;
	/* At least some part of the string was inside '' or "",
	 * possibly empty one: word"", wo''rd etc. */
	smallint has_quoted_part;
	smallint has_خالی_جایگاه;
	smallint ended_درون_ifs;
} o_string;
تعداد {
	EXP_FLAG_SINGLEWORD     = 0x80, /* must be 0x80 */
	EXP_FLAG_GLOB           = 0x2,
	/* Protect newly added chars against globbing
	 * by prepending \ to *, ?, [, \ */
	EXP_FLAG_ESC_GLOB_CHARS = 0x1,
};
/* Used for initialization: o_string foo = NULL_O_STRING; */
#تعریف_کن NULL_O_STRING { عدم }

#اگر_تعریف_نشده debug_printf_parse
ساكن ثابت حرف *ثابت assignment_flag[] ALIGN_PTR = {
	"MAYBE_ASSIGNMENT",
	"DEFINITELY_ASSIGNMENT",
	"NOT_ASSIGNMENT",
	"WORD_IS_KEYWORD",
};
#پایان_اگر

/* We almost can use standard FILE api, but we need an ability to move
 * its fd when redirects coincide with it. No api exists for that
 * (RFE for it at https://sourceware.org/bugzilla/show_bug.cgi?id=21902).
 * HFILE is our internal alternative. Only supports reading.
 * Since we now can, we incorporate linked list of all opened HFILEs
 * into the struct (used to be a separate mini-list).
 */
تعريفنوع بنية HFILE {
	حرف *cur;
	حرف *انتها_2;
	بنية HFILE *بعدی_hfile;
	صحيح fd;
	حرف buf[1024];
} HFILE;

تعريفنوع بنية درون_str {
	ثابت حرف *p;
	صحيح peek_buf[2];
	صحيح آخرین_char;
	HFILE *file;
} درون_str;

#اگر_تعریف_نشده __U_BOOT__
/* The descrip member of this structure is only used to make
 * debugging output pretty */
ساكن ثابت بنية {
	int32_t نما;
	علامتدار حرف default_fd;
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

بنية redir_struct {
	بنية redir_struct *بعدی;
	حرف *rd_filename;          /* filename */
	صحيح rd_fd;                  /* fd to redirect */
	/* fd to redirect to, or -3 if rd_fd is to be closed (n>&-) */
	صحيح rd_dup;
	smallint rd_type;           /* (enum redir_type) */
	/* note: for heredocs, rd_filename contains heredoc delimiter,
	 * and subsequently heredoc itself; and rd_dup is a bitmask:
	 * bit 0: do we need to trim leading tabs?
	 * bit 1: is heredoc quoted (<<'delim' syntax) ?
	 */
};
تعريفنوع تعداد redir_type {
	REDIRECT_درون_داد     = 0,
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

#پایان_اگر /* !__U_BOOT__ */

بنية command {
#اگر_تعریف_نشده __U_BOOT__
	pid_t pid;                  /* 0 if exited */
#پایان_اگر /* !__U_BOOT__ */
	بیعلامت assignment_cnt;    /* how many argv[i] are assignments? */
#إذا ENABLE_HUSH_LINENO_VAR
	بیعلامت lineno;
#پایان_اگر
	smallint cmd_type;          /* CMD_xxx */
#تعریف_کن CMD_نرمال   0
#تعریف_کن CMD_SUBSHELL 1
#إذا BASH_TEST2
/* used for "[[ EXPR ]]" */
# define CMD_TEST2_SINGLEWORD_NOGLOB 2
#پایان_اگر
#إذا BASH_TEST2 || ENABLE_HUSH_LOCAL || ENABLE_HUSH_EXPORT || ENABLE_HUSH_READONLY
/* used to prevent word splitting and globbing in "export v=t*" */
# define CMD_SINGLEWORD_NOGLOB 3
#پایان_اگر
#إذا ENABLE_HUSH_FUNCTIONS
# define CMD_FUNCDEF 4
#پایان_اگر

	smalluint cmd_exitcode;
	/* if non-NULL, this "command" is { list }, ( list ), or a compound statement */
	بنية pipe *گروه;
#إذا !BB_MMU
	حرف *گروه_as_string;
#پایان_اگر
#إذا ENABLE_HUSH_FUNCTIONS
	بنية function *فرزند_func;
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
#پایان_اگر
#اگر_تعریف_شده __U_BOOT__
	صحيح argc; /* number of program arguments */
#پایان_اگر
	حرف **argv;                /* command name and arguments */
/* argv vector may contain variable references (^Cvar^C, ^C0^C etc)
 * and on execution these are substituted with their values.
 * Substitution can make _several_ words out of one argv[n]!
 * Example: argv[0]=='.^C*^C.' here: echo .$*.
 * References of the form ^C`cmd arg^C are `cmd arg` substitutions.
 */
#اگر_تعریف_نشده __U_BOOT__
	بنية redir_struct *redirects; /* I/O redirections */
#پایان_اگر /* !__U_BOOT__ */
};
/* Is there anything in this command at all? */
#اگر_تعریف_نشده __U_BOOT__
#تعریف_کن IS_NULL_CMD(cmd) \
	(!(cmd)->گروه && !(cmd)->argv && !(cmd)->redirects)

#وإلا /* __U_BOOT__ */
#تعریف_کن IS_NULL_CMD(cmd) \
	(!(cmd)->گروه && !(cmd)->argv)
#پایان_اگر /* __U_BOOT__ */
بنية pipe {
	بنية pipe *بعدی;
	صحيح رقم_cmds;               /* total number of commands in pipe */
#اگر_تعریف_نشده __U_BOOT__
	صحيح alive_cmds;             /* number of commands running (not exited) */
	صحيح stopped_cmds;           /* number of commands alive, but stopped */
#إذا ENABLE_HUSH_کار
	بیعلامت jobid;             /* job number */
	pid_t pgrp;                 /* process group ID for the job */
	حرف *cmdtext;              /* name of job */
#پایان_اگر
#پایان_اگر /* !__U_BOOT__ */
	بنية command *cmds;       /* array of commands in pipe */
	smallint followup;          /* PIPE_BG, PIPE_SEQ, PIPE_OR, PIPE_AND */
	IF_HAS_KEYWORDS(smallint pi_inverted;) /* "! cmd | cmd" */
	IF_HAS_KEYWORDS(smallint res_کلمه;) /* needed for if, for, while, until... */
};
تعريفنوع تعداد pipe_style {
	PIPE_SEQ = 0,
	PIPE_AND = 1,
	PIPE_OR  = 2,
	PIPE_BG  = 3,
} pipe_style;
/* Is there anything in this pipe at all? */
#تعریف_کن IS_NULL_PIPE(pi) \
	((pi)->رقم_cmds == 0 IF_HAS_KEYWORDS( && (pi)->res_کلمه == RES_NONE))

/* This holds pointers to the various results of parsing */
بنية parse_context {
	/* linked list of pipes */
	بنية pipe *list_سر;
	/* last pipe (being constructed right now) */
	بنية pipe *pipe;
	/* last command in pipe (being constructed right now) */
	بنية command *command;
#اگر_تعریف_نشده __U_BOOT__
	/* last redirect in command->redirects list */
	بنية redir_struct *pending_redirect;
#پایان_اگر /* !__U_BOOT__ */
	o_string کلمه;
#إذا !BB_MMU
	o_string as_string;
#پایان_اگر
	smallint is_assignment; /* 0:maybe, 1:yes, 2:no, 3:keyword */
#إذا HAS_KEYWORDS
	smallint ctx_res_w;
	smallint ctx_inverted; /* "! cmd | cmd" */
#إذا ENABLE_HUSH_CASE
	smallint ctx_dsemicolon; /* ";;" seen */
#پایان_اگر
	/* bitmask of FLAG_xxx, for figuring out valid reserved words */
	صحيح old_flag;
	/* group we are enclosed in:
	 * example: "if pipe1; pipe2; then pipe3; fi"
	 * when we see "if" or "then", we malloc and copy current context,
	 * and make ->stack point to it. then we parse pipeN.
	 * when closing "then" / fi" / whatever is found,
	 * we move list_head into ->stack->command->group,
	 * copy ->stack into current context, and delete ->stack.
	 * (parsing of { list } and ( list ) doesn't use this method)
	 */
	بنية parse_context *پشته;
#پایان_اگر
};
تعداد {
	MAYBE_ASSIGNMENT      = 0,
	DEFINITELY_ASSIGNMENT = 1,
	NOT_ASSIGNMENT        = 2,
	/* Not an assignment, but next word may be: "if v=xyz cmd;" */
	کلمه_IS_KEYWORD       = 3,
};

#اگر_تعریف_نشده __U_BOOT__
/* On program start, environ points to initial environment.
 * putenv adds new pointers into it, unsetenv removes them.
 * Neither of these (de)allocates the strings.
 * setenv allocates new strings in malloc space and does putenv,
 * and thus setenv is unusable (leaky) for shell's purposes */
#تعریف_کن setenv(...) setenv_is_leaky_dont_use()
#پایان_اگر /* !__U_BOOT__ */
بنية variable {
	بنية variable *بعدی;
	حرف *varstr;        /* points to "name=" portion */
	صحيح max_len;         /* if > 0, name is part of initial env; else name is malloced */
#اگر_تعریف_نشده __U_BOOT__
	uint16_t var_nest_level;
	smallint flg_export; /* putenv should be done on this var */
	smallint flg_read_only;
#پایان_اگر /* !__U_BOOT__ */
};

تعداد {
	BC_BREAK = 1,
	BC_CONTINUE = 2,
};

#إذا ENABLE_HUSH_FUNCTIONS
بنية function {
	بنية function *بعدی;
	حرف *name;
	بنية command *والد_cmd;
	بنية pipe *body;
# إذا !BB_MMU
	حرف *body_as_string;
# endif
};
#پایان_اگر

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
ساكن ثابت حرف o_opt_رشتهها[] ALIGN1 =
	"pipefail\0"
	"noexec\0"
	"errexit\0"
#إذا ENABLE_HUSH_نما_X
	"xtrace\0"
#پایان_اگر
	;
تعداد {
	OPT_O_PIPEFAIL,
	OPT_O_NOEXEC,
	OPT_O_ERREXIT,
#إذا ENABLE_HUSH_نما_X
	OPT_O_XTRACE,
#پایان_اگر
	رقم_OPT_O
};

/* "Globals" within this file */
/* Sorted roughly by size (smaller offsets == smaller code) */
بنية globals {
#اگر_تعریف_نشده __U_BOOT__
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
#إذا ENABLE_HUSH_INTERACTIVE
	/* 'interactive_fd' is a fd# open to ctty, if we have one
	 * _AND_ if we decided to act interactively */
	صحيح interactive_fd;
	IF_NOT_FEATURE_EDITING_FANCY_PROMPT(حرف *PS1;)
# define G_interactive_fd (G.interactive_fd)
#وإلا
# define G_interactive_fd 0
#پایان_اگر
#وإلا /* __U_BOOT__ */
# define G_interactive_fd 0
#پایان_اگر /* __U_BOOT__ */
#اگر_تعریف_نشده __U_BOOT__
#إذا ENABLE_FEATURE_EDITING
	سطر_درون_داد_t *سطر_درون_داد_state;
#پایان_اگر
	pid_t ریشه_pid;
	pid_t ریشه_ppid;
	pid_t آخرین_bg_pid;
#إذا ENABLE_HUSH_RANDOM_SUPPORT
	random_t random_gen;
#پایان_اگر
#إذا ENABLE_HUSH_کار
	صحيح پیاپی_list_level;
	بیعلامت آخرین_jobid;
	pid_t saved_tty_pgrp;
	بنية pipe *کار_list;
# define G_saved_tty_pgrp (G.saved_tty_pgrp)
#وإلا
# define G_saved_tty_pgrp 0
#پایان_اگر
#پایان_اگر /* !__U_BOOT__ */
	/* How deeply are we in context where "set -e" is ignored */
	صحيح errexit_عمق;
#اگر_تعریف_نشده __U_BOOT__
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

	حرف o_opt[رقم_OPT_O];
#إذا ENABLE_HUSH_نما_X
# define G_x_نما (G.o_opt[OPT_O_XTRACE])
#وإلا
# define G_x_نما 0
#پایان_اگر
	حرف opt_s;
	حرف opt_c;
#پایان_اگر /* !__U_BOOT__ */
#إذا ENABLE_HUSH_INTERACTIVE
	smallint promptmode; /* 0: PS1, 1: PS2 */
#پایان_اگر
	/* set by signal handler if SIGINT is received _and_ its trap is not set */
	smallint flag_SIGINT;
#اگر_تعریف_نشده __U_BOOT__
#إذا ENABLE_HUSH_LOOPS
	smallint flag_break_continue;
#پایان_اگر
#پایان_اگر /* !__U_BOOT__ */
#إذا ENABLE_HUSH_FUNCTIONS
	/* 0: outside of a function (or sourced file)
	 * -1: inside of a function, ok to use return builtin
	 * 1: return is invoked, skip all till end of func
	 */
	smallint flag_return_درون_پیشرفت;
# define G_flag_return_درون_پیشرفت (G.flag_return_درون_پیشرفت)
#وإلا
# define G_flag_return_درون_پیشرفت 0
#پایان_اگر
	smallint exiting; /* used to prevent EXIT trap recursion */
	/* These support $? */
	smalluint آخرین_exitcode;
	smalluint expand_exitcode;
	smalluint آخرین_bg_pid_exitcode;
#اگر_تعریف_نشده __U_BOOT__
#إذا ENABLE_HUSH_مجموعه
	/* are global_argv and global_argv[1..n] malloced? (note: not [0]) */
	smalluint global_آرگومانها_malloced;
# define G_global_آرگومانها_malloced (G.global_آرگومانها_malloced)
#وإلا
# define G_global_آرگومانها_malloced 0
#پایان_اگر
#إذا ENABLE_HUSH_BASH_COMPAT
	صحيح dead_کار_exitcode; /* for "wait -n" */
#پایان_اگر
#پایان_اگر /* !__U_BOOT__ */
	/* how many non-NULL argv's we have. NB: $# + 1 */
	صحيح global_argc;
	حرف **global_argv;
#إذا !BB_MMU
	حرف *argv0_for_re_execing;
#پایان_اگر
#إذا ENABLE_HUSH_LOOPS
#اگر_تعریف_نشده __U_BOOT__
	بیعلامت عمق_break_continue;
#پایان_اگر /* !__U_BOOT__ */
	بیعلامت عمق_از_loop;
#پایان_اگر
#اگر_تعریف_نشده __U_BOOT__
#إذا ENABLE_HUSH_GETOPTS
	بیعلامت getopt_شمار;
#پایان_اگر
#پایان_اگر /* !__U_BOOT__ */
	ثابت حرف *ifs;
#اگر_تعریف_شده __U_BOOT__
	صحيح flag_repeat;
	صحيح do_repeat;
	صحيح پیاپی_command_flags;
#پایان_اگر /* __U_BOOT__ */
	حرف *ifs_whitespace; /* = G.ifs or malloced */
#اگر_تعریف_نشده __U_BOOT__
	ثابت حرف *cwd;
#پایان_اگر /* !__U_BOOT__ */
	بنية variable *بالاترین_var;
	حرف **expanded_assignments;
	بنية variable **shadowed_vars_pp;
	بیعلامت var_nest_level;
#اگر_تعریف_نشده __U_BOOT__
#إذا ENABLE_HUSH_FUNCTIONS
# إذا ENABLE_HUSH_LOCAL
	بیعلامت func_nest_level; /* solely to prevent "local v" in non-functions */
# endif
	بنية function *بالاترین_func;
#پایان_اگر
	/* Signal and trap handling */
#إذا ENABLE_HUSH_FAST
	بیعلامت شمار_SIGCHLD;
	بیعلامت handled_SIGCHLD;
	smallint we_have_children;
#پایان_اگر
#إذا ENABLE_HUSH_LINENO_VAR
	بیعلامت parse_lineno;
	بیعلامت execute_lineno;
#پایان_اگر
	HFILE *HFILE_list;
	HFILE *HFILE_stdin;
	/* Which signals have non-DFL handler (even with no traps set)?
	 * Set at the start to:
	 * (SIGQUIT + maybe SPECIAL_INTERACTIVE_SIGS + maybe SPECIAL_JOBSTOP_SIGS)
	 * SPECIAL_INTERACTIVE_SIGS are cleared after fork.
	 * The rest is cleared right before execv syscalls.
	 * Other than these two times, never modified.
	 */
	بیعلامت special_sig_ماسک;
#إذا ENABLE_HUSH_کار
	بیعلامت fatal_sig_ماسک;
# define G_fatal_sig_ماسک (G.fatal_sig_ماسک)
#وإلا
# define G_fatal_sig_ماسک 0
#پایان_اگر
#إذا ENABLE_HUSH_TRAP
	صحيح pre_trap_exitcode;
# إذا ENABLE_HUSH_FUNCTIONS
	صحيح return_exitcode;
# endif
	حرف **traps; /* char *traps[NSIG] */
# define G_traps G.traps
#وإلا
# define G_traps ((حرف**)عدم)
#پایان_اگر
	sigset_t pending_مجموعه;
#إذا ENABLE_HUSH_MEMLEAK
	بیعلامت دراز memleak_مقدار;
#پایان_اگر
#إذا ENABLE_HUSH_نما_X
	بیعلامت x_نما_عمق;
	/* "set -x" output should not be redirectable with subsequent 2>FILE.
	 * We dup fd#2 to x_mode_fd when "set -x" is executed, and use it
	 * for all subsequent output.
	 */
	صحيح x_نما_fd;
	o_string x_نما_buf;
#پایان_اگر
#پایان_اگر /* !__U_BOOT__ */
#إذا HUSH_DEBUG >= 2
	صحيح debug_indent;
#پایان_اگر
#اگر_تعریف_نشده __U_BOOT__
	بنية sigaction sa;
	حرف optstring_buf[حجم("eixcs")];
#إذا BASH_EPOCH_VARS
	حرف epoch_buf[حجم("%llu.nnnnnn") + حجم(دراز دراز)*3];
#پایان_اگر
#إذا ENABLE_FEATURE_EDITING
	حرف user_درون_داد_buf[CONFIG_FEATURE_EDITING_MAX_LEN];
#پایان_اگر
#پایان_اگر /* !__U_BOOT__ */
};
#اگر_تعریف_شده __U_BOOT__
بنية globals *ptr_مقصد_globals;
#پایان_اگر /* __U_BOOT__ */
#تعریف_کن G (*ptr_مقصد_globals)
/* Not #defining name to G.name - this quickly gets unwieldy
 * (too many defines). Also, I actually prefer to see when a variable
 * is global, thus "G." prefix is a useful hint */
#اگر_تعریف_شده __U_BOOT__
#تعریف_کن مجموعه_PTR_مقصد_GLOBALS(x) افعل { \
	(*(بنية globals**)&ptr_مقصد_globals) = (فراغ*)(x); \
	barrier(); \
} طالما (0)
#تعریف_کن INIT_G() افعل { \
	مجموعه_PTR_مقصد_GLOBALS(xzalloc(حجم(G))); \
	G.promptmode = 1; \
} طالما (0)
#وإلا /* !__U_BOOT__ */
#تعریف_کن INIT_G() افعل { \
	مجموعه_PTR_مقصد_GLOBALS(xzalloc(حجم(G))); \
	/* memset(&G.sa, 0, sizeof(G.sa)); */  \
	sigfillset(&G.sa.sa_ماسک); \
	G.sa.sa_flags = SA_RESTART; \
} طالما (0)
#پایان_اگر /* !__U_BOOT__ */

#اگر_تعریف_نشده __U_BOOT__
/* Function prototypes for builtins */
ساكن صحيح builtin_cd(حرف **argv) FAST_FUNC;
#إذا ENABLE_HUSH_ECHO
ساكن صحيح builtin_echo(حرف **argv) FAST_FUNC;
#پایان_اگر
ساكن صحيح builtin_eval(حرف **argv) FAST_FUNC;
ساكن صحيح builtin_exec(حرف **argv) FAST_FUNC;
ساكن صحيح builtin_exit(حرف **argv) FAST_FUNC;
#إذا ENABLE_HUSH_EXPORT
ساكن صحيح builtin_export(حرف **argv) FAST_FUNC;
#پایان_اگر
#إذا ENABLE_HUSH_READONLY
ساكن صحيح builtin_readonly(حرف **argv) FAST_FUNC;
#پایان_اگر
ساكن صحيح builtin_false(حرف **argv) FAST_FUNC;
#إذا ENABLE_HUSH_کار
ساكن صحيح builtin_fg_bg(حرف **argv) FAST_FUNC;
ساكن صحيح builtin_jobs(حرف **argv) FAST_FUNC;
#پایان_اگر
#إذا ENABLE_HUSH_GETOPTS
ساكن صحيح builtin_getopts(حرف **argv) FAST_FUNC;
#پایان_اگر
#إذا ENABLE_HUSH_HELP
ساكن صحيح builtin_help(حرف **argv) FAST_FUNC;
#پایان_اگر
#إذا MAX_HISTORY && ENABLE_FEATURE_EDITING
ساكن صحيح builtin_history(حرف **argv) FAST_FUNC;
#پایان_اگر
#إذا ENABLE_HUSH_LOCAL
ساكن صحيح builtin_local(حرف **argv) FAST_FUNC;
#پایان_اگر
#إذا ENABLE_HUSH_MEMLEAK
ساكن صحيح builtin_memleak(حرف **argv) FAST_FUNC;
#پایان_اگر
#إذا ENABLE_HUSH_PRINTF
ساكن صحيح builtin_printf(حرف **argv) FAST_FUNC;
#پایان_اگر
ساكن صحيح builtin_pwd(حرف **argv) FAST_FUNC;
#إذا ENABLE_HUSH_READ
ساكن صحيح builtin_read(حرف **argv) FAST_FUNC;
#پایان_اگر
#إذا ENABLE_HUSH_مجموعه
ساكن صحيح builtin_مجموعه(حرف **argv) FAST_FUNC;
#پایان_اگر
ساكن صحيح builtin_لغزش(حرف **argv) FAST_FUNC;
ساكن صحيح builtin_منبع(حرف **argv) FAST_FUNC;
#إذا ENABLE_HUSH_TEST || BASH_TEST2
ساكن صحيح builtin_test(حرف **argv) FAST_FUNC;
#پایان_اگر
#إذا ENABLE_HUSH_TRAP
ساكن صحيح builtin_trap(حرف **argv) FAST_FUNC;
#پایان_اگر
#إذا ENABLE_HUSH_TYPE
ساكن صحيح builtin_type(حرف **argv) FAST_FUNC;
#پایان_اگر
#إذا ENABLE_HUSH_TIMES
ساكن صحيح builtin_times(حرف **argv) FAST_FUNC;
#پایان_اگر
ساكن صحيح builtin_true(حرف **argv) FAST_FUNC;
#إذا ENABLE_HUSH_UMASK
ساكن صحيح builtin_umask(حرف **argv) FAST_FUNC;
#پایان_اگر
#إذا ENABLE_HUSH_UNSET
ساكن صحيح builtin_unset(حرف **argv) FAST_FUNC;
#پایان_اگر
#إذا ENABLE_HUSH_KILL
ساكن صحيح builtin_kill(حرف **argv) FAST_FUNC;
#پایان_اگر
#إذا ENABLE_HUSH_WAIT
ساكن صحيح builtin_wait(حرف **argv) FAST_FUNC;
#پایان_اگر
#إذا ENABLE_HUSH_LOOPS
ساكن صحيح builtin_break(حرف **argv) FAST_FUNC;
ساكن صحيح builtin_continue(حرف **argv) FAST_FUNC;
#پایان_اگر
#إذا ENABLE_HUSH_FUNCTIONS
ساكن صحيح builtin_return(حرف **argv) FAST_FUNC;
#پایان_اگر

/* Table of built-in functions.  They can be forked or not, depending on
 * context: within pipes, they fork.  As simple commands, they do not.
 * When used in non-forking context, they can change global variables
 * in the parent shell process.  If forked, of course they cannot.
 * For example, 'unset foo | whatever' will parse and run, but foo will
 * still be set at the end. */
بنية built_درون_command {
	ثابت حرف *b_cmd;
	صحيح (*b_function)(حرف **argv) FAST_FUNC;
#إذا ENABLE_HUSH_HELP
	ثابت حرف *b_descr;
# define BLTIN(cmd, func, help) { cmd, func, help }
#وإلا
# define BLTIN(cmd, func, help) { cmd, func }
#پایان_اگر
};

ساكن ثابت بنية built_درون_command bltins1[] ALIGN_PTR = {
	BLTIN("."        , builtin_منبع  , "Run commands in file"),
	BLTIN(":"        , builtin_true    , عدم),
#إذا ENABLE_HUSH_کار
	BLTIN("bg"       , builtin_fg_bg   , "Resume job in background"),
#پایان_اگر
#إذا ENABLE_HUSH_LOOPS
	BLTIN("break"    , builtin_break   , "Exit loop"),
#پایان_اگر
	BLTIN("cd"       , builtin_cd      , "Change directory"),
#إذا ENABLE_HUSH_LOOPS
	BLTIN("continue" , builtin_continue, "Start new loop iteration"),
#پایان_اگر
	BLTIN("eval"     , builtin_eval    , "Construct and run shell command"),
	BLTIN("exec"     , builtin_exec    , "Execute command, don't return to shell"),
	BLTIN("exit"     , builtin_exit    , عدم),
#إذا ENABLE_HUSH_EXPORT
	BLTIN("export"   , builtin_export  , "Set environment variables"),
#پایان_اگر
	BLTIN("false"    , builtin_false   , عدم),
#إذا ENABLE_HUSH_کار
	BLTIN("fg"       , builtin_fg_bg   , "Bring job to foreground"),
#پایان_اگر
#إذا ENABLE_HUSH_GETOPTS
	BLTIN("getopts"  , builtin_getopts , عدم),
#پایان_اگر
#إذا ENABLE_HUSH_HELP
	BLTIN("help"     , builtin_help    , عدم),
#پایان_اگر
#إذا MAX_HISTORY && ENABLE_FEATURE_EDITING
	BLTIN("history"  , builtin_history , "Show history"),
#پایان_اگر
#إذا ENABLE_HUSH_کار
	BLTIN("jobs"     , builtin_jobs    , "List jobs"),
#پایان_اگر
#إذا ENABLE_HUSH_KILL
	BLTIN("kill"     , builtin_kill    , "Send signals to processes"),
#پایان_اگر
#إذا ENABLE_HUSH_LOCAL
	BLTIN("local"    , builtin_local   , "Set local variables"),
#پایان_اگر
#إذا ENABLE_HUSH_MEMLEAK
	BLTIN("memleak"  , builtin_memleak , عدم),
#پایان_اگر
#إذا ENABLE_HUSH_READ
	BLTIN("read"     , builtin_read    , "Input into variable"),
#پایان_اگر
#إذا ENABLE_HUSH_READONLY
	BLTIN("readonly" , builtin_readonly, "Make variables read-only"),
#پایان_اگر
#إذا ENABLE_HUSH_FUNCTIONS
	BLTIN("return"   , builtin_return  , "Return from function"),
#پایان_اگر
#إذا ENABLE_HUSH_مجموعه
	BLTIN("set"      , builtin_مجموعه     , "Set positional parameters"),
#پایان_اگر
	BLTIN("shift"    , builtin_لغزش   , "Shift positional parameters"),
#إذا BASH_منبع
	BLTIN("source"   , builtin_منبع  , عدم),
#پایان_اگر
#إذا ENABLE_HUSH_TIMES
	BLTIN("times"    , builtin_times   , عدم),
#پایان_اگر
#إذا ENABLE_HUSH_TRAP
	BLTIN("trap"     , builtin_trap    , "Trap signals"),
#پایان_اگر
	BLTIN("true"     , builtin_true    , عدم),
#إذا ENABLE_HUSH_TYPE
	BLTIN("type"     , builtin_type    , "Show command type"),
#پایان_اگر
#إذا ENABLE_HUSH_ULIMIT
	BLTIN("ulimit"   , shell_builtin_ulimit, "Control resource limits"),
#پایان_اگر
#إذا ENABLE_HUSH_UMASK
	BLTIN("umask"    , builtin_umask   , "Set file creation mask"),
#پایان_اگر
#إذا ENABLE_HUSH_UNSET
	BLTIN("unset"    , builtin_unset   , "Unset variables"),
#پایان_اگر
#إذا ENABLE_HUSH_WAIT
	BLTIN("wait"     , builtin_wait    , "Wait for process to finish"),
#پایان_اگر
};
/* These builtins won't be used if we are on NOMMU and need to re-exec
 * (it's cheaper to run an external program in this case):
 */
ساكن ثابت بنية built_درون_command bltins2[] ALIGN_PTR = {
#إذا ENABLE_HUSH_TEST
	BLTIN("["        , builtin_test    , عدم),
#پایان_اگر
#إذا BASH_TEST2
	BLTIN("[["       , builtin_test    , عدم),
#پایان_اگر
#إذا ENABLE_HUSH_ECHO
	BLTIN("echo"     , builtin_echo    , عدم),
#پایان_اگر
#إذا ENABLE_HUSH_PRINTF
	BLTIN("printf"   , builtin_printf  , عدم),
#پایان_اگر
	BLTIN("pwd"      , builtin_pwd     , عدم),
#إذا ENABLE_HUSH_TEST
	BLTIN("test"     , builtin_test    , عدم),
#پایان_اگر
};

#پایان_اگر /* !__U_BOOT__ */

/* Debug printouts.
 */
#إذا HUSH_DEBUG >= 2
/* prevent disasters with G.debug_indent < 0 */
# define indent() fdprintf(2, "%*s", (G.debug_indent * 2) & 0xff, "")
# define debug_enter() (G.debug_indent++)
# define debug_leave() (G.debug_indent--)
#وإلا
# define indent()      ((فراغ)0)
# define debug_enter() ((فراغ)0)
# define debug_leave() ((فراغ)0)
#پایان_اگر

#اگر_تعریف_نشده debug_printf
# define debug_printf(...) (indent(), fdprintf(2, __VA_ARGS__))
#پایان_اگر

#اگر_تعریف_نشده debug_printf_parse
# define debug_printf_parse(...) (indent(), fdprintf(2, __VA_ARGS__))
#پایان_اگر

#اگر_تعریف_نشده debug_printf_heredoc
# define debug_printf_heredoc(...) (indent(), fdprintf(2, __VA_ARGS__))
#پایان_اگر

#اگر_تعریف_نشده debug_printf_exec
#تعریف_کن debug_printf_exec(...) (indent(), fdprintf(2, __VA_ARGS__))
#پایان_اگر

#اگر_تعریف_نشده debug_printf_env
# define debug_printf_env(...) (indent(), fdprintf(2, __VA_ARGS__))
#پایان_اگر

#اگر_تعریف_نشده debug_printf_jobs
# define debug_printf_jobs(...) (indent(), fdprintf(2, __VA_ARGS__))
# define DEBUG_JOBS 1
#وإلا
# define DEBUG_JOBS 0
#پایان_اگر

#اگر_تعریف_نشده debug_printf_expand
# define debug_printf_expand(...) (indent(), fdprintf(2, __VA_ARGS__))
# define DEBUG_EXPAND 1
#وإلا
# define DEBUG_EXPAND 0
#پایان_اگر

#اگر_تعریف_نشده debug_printf_varexp
# define debug_printf_varexp(...) (indent(), fdprintf(2, __VA_ARGS__))
#پایان_اگر

#اگر_تعریف_نشده debug_printf_glob
# define debug_printf_glob(...) (indent(), fdprintf(2, __VA_ARGS__))
# define DEBUG_GLOB 1
#وإلا
# define DEBUG_GLOB 0
#پایان_اگر

#اگر_تعریف_نشده debug_printf_redir
# define debug_printf_redir(...) (indent(), fdprintf(2, __VA_ARGS__))
#پایان_اگر

#اگر_تعریف_نشده debug_printf_list
# define debug_printf_list(...) (indent(), fdprintf(2, __VA_ARGS__))
#پایان_اگر

#اگر_تعریف_نشده debug_printf_subst
# define debug_printf_subst(...) (indent(), fdprintf(2, __VA_ARGS__))
#پایان_اگر

#اگر_تعریف_نشده debug_printf_prompt
# define debug_printf_prompt(...) (indent(), fdprintf(2, __VA_ARGS__))
#پایان_اگر

#اگر_تعریف_نشده debug_printf_clean
# define debug_printf_clean(...) (indent(), fdprintf(2, __VA_ARGS__))
# define DEBUG_CLEAN 1
#وإلا
# define DEBUG_CLEAN 0
#پایان_اگر

#إذا DEBUG_EXPAND
ساكن فراغ debug_print_رشتهها(ثابت حرف *پیشوند, حرف **vv)
{
	indent();
	fdprintf(2, "%s:\n", پیشوند);
	طالما (*vv)
		fdprintf(2, " '%s'\n", *vv++);
}
#وإلا
# define debug_print_رشتهها(پیشوند, vv) ((فراغ)0)
#پایان_اگر

/* Leak hunting. Use hush_leaktool.sh for post-processing.
 */
#إذا LEAK_HUNTING
ساكن فراغ *xxmalloc(صحيح lineno, size_t اندازه)
{
	فراغ *ptr = xmalloc((اندازه + 0xff) & ~0xff);
	fdprintf(2, "line %d: malloc %p\n", lineno, ptr);
	أرجع ptr;
}
ساكن فراغ *xxrealloc(صحيح lineno, فراغ *ptr, size_t اندازه)
{
	ptr = xrealloc(ptr, (اندازه + 0xff) & ~0xff);
	fdprintf(2, "line %d: realloc %p\n", lineno, ptr);
	أرجع ptr;
}
ساكن حرف *xxstrdup(صحيح lineno, ثابت حرف *str)
{
	حرف *ptr = xstrdup(str);
	fdprintf(2, "line %d: strdup %p\n", lineno, ptr);
	أرجع ptr;
}
ساكن فراغ xxfree(فراغ *ptr)
{
	fdprintf(2, "free %p\n", ptr);
	حرر(ptr);
}
# define xmalloc(s)     xxmalloc(__LINE__, s)
# define xrealloc(p, s) xxrealloc(__LINE__, p, s)
# define xstrdup(s)     xxstrdup(__LINE__, s)
# define حرر(p)        xxfree(p)
#پایان_اگر

/* Syntax and runtime errors. They always abort scripts.
 * In interactive use they usually discard unparsed and/or unexecuted commands
 * and return to the prompt.
 * HUSH_DEBUG >= 2 prints line number in this file where it was detected.
 */
#إذا HUSH_DEBUG < 2
# define پیام_and_die_if_script(lineno, ...)     پیام_and_die_if_script(__VA_ARGS__)
# define syntax_error(lineno, پیام)              syntax_error(پیام)
# define syntax_error_at(lineno, پیام)           syntax_error_at(پیام)
# define syntax_error_unterm_ch(lineno, ch)     syntax_error_unterm_ch(ch)
# define syntax_error_unterm_str(lineno, s)     syntax_error_unterm_str(s)
# define syntax_error_unexpected_ch(lineno, ch) syntax_error_unexpected_ch(ch)
#پایان_اگر

ساكن فراغ die_if_script(فراغ)
{
	إذا (!G_interactive_fd) {
		إذا (G.آخرین_exitcode) /* sometines it's 2, not 1 (bash compat) */
			xfunc_error_retval = G.آخرین_exitcode;
		xfunc_die();
	}
}

#اگر_تعریف_شده __U_BOOT__
ساكن فراغ __maybe_unused پیام_and_die_if_script(بیعلامت lineno, ثابت حرف *fmt, ...)
#وإلا /* !__U_BOOT__ */
ساكن فراغ پیام_and_die_if_script(بیعلامت lineno, ثابت حرف *fmt, ...)
#پایان_اگر /* !__U_BOOT__ */
{
	va_list p;

#إذا HUSH_DEBUG >= 2
	bb_error_پیام("hush.c:%u", lineno);
#پایان_اگر
	va_شروع(p, fmt);
	bb_verror_پیام(fmt, p, عدم);
	va_انتها(p);
	die_if_script();
}

ساكن فراغ syntax_error(بیعلامت lineno استفادهنشده_PARAM, ثابت حرف *پیام)
{
	إذا (پیام)
		bb_error_پیام("syntax error: %s", پیام);
	وإلا
		bb_simple_error_پیام("syntax error");
	die_if_script();
}

ساكن فراغ syntax_error_at(بیعلامت lineno استفادهنشده_PARAM, ثابت حرف *پیام)
{
	bb_error_پیام("syntax error at '%s'", پیام);
	die_if_script();
}

ساكن فراغ syntax_error_unterm_str(بیعلامت lineno استفادهنشده_PARAM, ثابت حرف *s)
{
	bb_error_پیام("syntax error: unterminated %s", s);
//? source4.tests fails: in bash, echo ${^} in script does not terminate the script
// (but bash --posix, or if bash is run as "sh", does terminate in script, so maybe uncomment this?)
//	die_if_script();
}

ساكن فراغ syntax_error_unterm_ch(بیعلامت lineno, حرف ch)
{
	حرف پیام[2] = { ch, '\0' };
	syntax_error_unterm_str(lineno, پیام);
}

ساكن فراغ syntax_error_unexpected_ch(بیعلامت lineno استفادهنشده_PARAM, صحيح ch)
{
	حرف پیام[2];
	پیام[0] = ch;
	پیام[1] = '\0';
#إذا HUSH_DEBUG >= 2
	bb_error_پیام("hush.c:%u", lineno);
#پایان_اگر
	bb_error_پیام("syntax error: unexpected %s", ch == EOF ? "EOF" : پیام);
	die_if_script();
}

#إذا HUSH_DEBUG < 2
# undef پیام_and_die_if_script
# undef syntax_error
# undef syntax_error_at
# undef syntax_error_unterm_ch
# undef syntax_error_unterm_str
# undef syntax_error_unexpected_ch
#وإلا
# define پیام_and_die_if_script(...)     پیام_and_die_if_script(__LINE__, __VA_ARGS__)
# define syntax_error(پیام)              syntax_error(__LINE__, پیام)
# define syntax_error_at(پیام)           syntax_error_at(__LINE__, پیام)
# define syntax_error_unterm_ch(ch)     syntax_error_unterm_ch(__LINE__, ch)
# define syntax_error_unterm_str(s)     syntax_error_unterm_str(__LINE__, s)
# define syntax_error_unexpected_ch(ch) syntax_error_unexpected_ch(__LINE__, ch)
#پایان_اگر

/* Utility functions
 */
/* Replace each \x with x in place, return ptr past NUL. */
ساكن حرف *unbackslash(حرف *src)
{
#اگر_تعریف_شده __U_BOOT__
	حرف *dst = src = (حرف *)strchrnul(src, '\\');
#وإلا /* !__U_BOOT__ */
	حرف *dst = src = strchrnul(src, '\\');
#پایان_اگر /* !__U_BOOT__ */
	طالما (1) {
		إذا (*src == '\\') {
			src++;
			إذا (*src != '\0') {
				/* \x -> x */
				*dst++ = *src++;
				استمر;
			}
			/* else: "\<nul>". Do not delete this backslash.
			 * Testcase: eval 'echo ok\'
			 */
			*dst++ = '\\';
			/* fallthrough */
		}
		إذا ((*dst++ = *src++) == '\0')
			اكسر;
	}
	أرجع dst;
}

ساكن حرف **جمع_کن_رشتهها_مقصد_رشتهها(حرف **رشتهها, حرف **جمع_کن, صحيح need_مقصد_dup)
{
	صحيح i;
	بیعلامت count1;
	بیعلامت count2;
	حرف **v;

	v = رشتهها;
	count1 = 0;
	إذا (v) {
		طالما (*v) {
			count1++;
			v++;
		}
	}
	count2 = 0;
	v = جمع_کن;
	طالما (*v) {
		count2++;
		v++;
	}
	v = xrealloc(رشتهها, (count1 + count2 + 1) * حجم(حرف*));
	v[count1 + count2] = عدم;
	i = count2;
	طالما (--i >= 0)
		v[count1 + i] = (need_مقصد_dup ? xstrdup(جمع_کن[i]) : جمع_کن[i]);
	أرجع v;
}
#إذا LEAK_HUNTING
ساكن حرف **xx_جمع_کن_رشتهها_مقصد_رشتهها(صحيح lineno, حرف **رشتهها, حرف **جمع_کن, صحيح need_مقصد_dup)
{
	حرف **ptr = جمع_کن_رشتهها_مقصد_رشتهها(رشتهها, جمع_کن, need_مقصد_dup);
	fdprintf(2, "line %d: add_strings_to_strings %p\n", lineno, ptr);
	أرجع ptr;
}
#تعریف_کن جمع_کن_رشتهها_مقصد_رشتهها(رشتهها, جمع_کن, need_مقصد_dup) \
	xx_جمع_کن_رشتهها_مقصد_رشتهها(__LINE__, رشتهها, جمع_کن, need_مقصد_dup)
#پایان_اگر

/* Note: takes ownership of "add" ptr (it is not strdup'ed) */
ساكن حرف **جمع_کن_string_مقصد_رشتهها(حرف **رشتهها, حرف *جمع_کن)
{
	حرف *v[2];
	v[0] = جمع_کن;
	v[1] = عدم;
	أرجع جمع_کن_رشتهها_مقصد_رشتهها(رشتهها, v, /*dup:*/ 0);
}

#إذا LEAK_HUNTING
ساكن حرف **xx_جمع_کن_string_مقصد_رشتهها(صحيح lineno, حرف **رشتهها, حرف *جمع_کن)
{
	حرف **ptr = جمع_کن_string_مقصد_رشتهها(رشتهها, جمع_کن);
	fdprintf(2, "line %d: add_string_to_strings %p\n", lineno, ptr);
	أرجع ptr;
}
#تعریف_کن جمع_کن_string_مقصد_رشتهها(رشتهها, جمع_کن) \
	xx_جمع_کن_string_مقصد_رشتهها(__LINE__, رشتهها, جمع_کن)
#پایان_اگر

ساكن فراغ free_رشتهها(حرف **رشتهها)
{
	حرف **v;

	إذا (!رشتهها)
		أرجع;
	v = رشتهها;
	طالما (*v) {
		حرر(*v);
		v++;
	}
	حرر(رشتهها);
}

#اگر_تعریف_نشده __U_BOOT__
ساكن صحيح dup_CLOEXEC(صحيح fd, صحيح avoid_fd)
{
	صحيح newfd;
 repeat:
	newfd = fcntl(fd, F_DUPFD_CLOEXEC, avoid_fd + 1);
	إذا (newfd >= 0) {
		إذا (F_DUPFD_CLOEXEC == F_DUPFD) /* if old libc (w/o F_DUPFD_CLOEXEC) */
			close_on_exec_on(newfd);
	} وإلا { /* newfd < 0 */
		إذا (errno == EBUSY)
			اذهب repeat;
		إذا (errno == EINTR)
			اذهب repeat;
		إذا (errno != EBADF) {
			/* "echo >&9999" gets EINVAL trying to save fd 1 to above 9999.
			 * We could try saving it _below_ 9999 instead (how?), but
			 * this probably means that dup2(9999,1) to effectuate >&9999
			 * would also not work: fd 9999 can't exist.
			 * (This differs from "echo >&99" where saving works, but
			 * subsequent dup2(99,1) fails if fd 99 is not open).
			 */
			bb_perror_پیام("fcntl(%d,F_DUPFD,%d)", fd, avoid_fd + 1);
		}
	}
	أرجع newfd;
}

ساكن صحيح xdup_CLOEXEC_and_close(صحيح fd, صحيح avoid_fd)
{
	صحيح newfd;
 repeat:
	newfd = fcntl(fd, F_DUPFD_CLOEXEC, avoid_fd + 1);
	إذا (newfd < 0) {
		إذا (errno == EBUSY)
			اذهب repeat;
		إذا (errno == EINTR)
			اذهب repeat;
		/* fd was not open? */
		إذا (errno == EBADF)
			أرجع fd;
		xfunc_die();
	}
	إذا (F_DUPFD_CLOEXEC == F_DUPFD) /* if old libc (w/o F_DUPFD_CLOEXEC) */
		close_on_exec_on(newfd);
	close(fd);
	أرجع newfd;
}

/* Manipulating HFILEs */
ساكن HFILE *hfopen(ثابت حرف *name)
{
	HFILE *fp;
	صحيح fd;

	fd = STDIN_FILENO;
	إذا (name) {
		fd = open(name, O_RDONLY | O_CLOEXEC);
		إذا (fd < 0)
			أرجع عدم;
		إذا (O_CLOEXEC == 0) /* ancient libc */
			close_on_exec_on(fd);
	}

	fp = xmalloc(حجم(*fp));
	إذا (name == عدم)
		G.HFILE_stdin = fp;
	fp->fd = fd;
	fp->cur = fp->انتها_2 = fp->buf;
	fp->بعدی_hfile = G.HFILE_list;
	G.HFILE_list = fp;
	أرجع fp;
}
ساكن فراغ hfclose(HFILE *fp)
{
	HFILE **pp = &G.HFILE_list;
	طالما (*pp) {
		HFILE *cur = *pp;
		إذا (cur == fp) {
			*pp = cur->بعدی_hfile;
			اكسر;
		}
		pp = &cur->بعدی_hfile;
	}
	إذا (fp->fd >= 0)
		close(fp->fd);
	حرر(fp);
}
ساكن صحيح refill_HFILE_and_getc(HFILE *fp)
{
	صحيح n;

	إذا (fp->fd < 0) {
		/* Already saw EOF */
		أرجع EOF;
	}
#إذا ENABLE_HUSH_INTERACTIVE && !ENABLE_FEATURE_EDITING
	/* If user presses ^C, read() restarts after SIGINT (we use SA_RESTART).
	 * IOW: ^C will not immediately stop line input.
	 * But poll() is different: it does NOT restart after signals.
	 */
	إذا (fp == G.HFILE_stdin) {
		بنية pollfd pfd[1];
		pfd[0].fd = fp->fd;
		pfd[0].events = POLLIN;
		n = poll(pfd, 1, -1);
		إذا (n < 0
		 /*&& errno == EINTR - assumed true */
		 && sigismember(&G.pending_مجموعه, SIGINT)
		) {
			أرجع '\0';
		}
	}
#وإلا
/* if FEATURE_EDITING=y, we do not use this routine for interactive input */
#پایان_اگر
	/* Try to buffer more input */
	n = امن_read(fp->fd, fp->buf, حجم(fp->buf));
	إذا (n < 0) {
		bb_simple_perror_پیام("read error");
		n = 0;
	}
	fp->cur = fp->buf;
	fp->انتها_2 = fp->buf + n;
	إذا (n == 0) {
		/* EOF/error */
		close(fp->fd);
		fp->fd = -1;
		أرجع EOF;
	}
	أرجع (بیعلامت حرف)(*fp->cur++);
}
/* Inlined for common case of non-empty buffer.
 */
ساكن ALWAYS_INLINE صحيح hfgetc(HFILE *fp)
{
	إذا (fp->cur < fp->انتها_2)
		أرجع (بیعلامت حرف)(*fp->cur++);
	/* Buffer empty */
	أرجع refill_HFILE_and_getc(fp);
}
ساكن صحيح move_HFILEs_on_redirect(صحيح fd, صحيح avoid_fd)
{
	HFILE *fl = G.HFILE_list;
	طالما (fl) {
		إذا (fd == fl->fd) {
			/* We use it only on script files, they are all CLOEXEC */
			fl->fd = xdup_CLOEXEC_and_close(fd, avoid_fd);
			debug_printf_redir("redirect_fd %d: matches a script fd, moving it to %d\n", fd, fl->fd);
			أرجع 1; /* "found and moved" */
		}
		fl = fl->بعدی_hfile;
	}
#إذا ENABLE_HUSH_نما_X
	إذا (G.x_نما_fd > 0 && fd == G.x_نما_fd) {
		G.x_نما_fd = xdup_CLOEXEC_and_close(fd, avoid_fd);
		أرجع 1; /* "found and moved" */
	}
#پایان_اگر
	أرجع 0; /* "not in the list" */
}
#إذا ENABLE_FEATURE_SH_STANDALONE && BB_MMU
ساكن فراغ close_all_HFILE_list(فراغ)
{
	HFILE *fl = G.HFILE_list;
	طالما (fl) {
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
		إذا (fl->fd > 0)
			/*hfclose(fl); - unsafe */
			close(fl->fd);
		fl = fl->بعدی_hfile;
	}
}
#پایان_اگر
ساكن صحيح fd_درون_HFILEs(صحيح fd)
{
	HFILE *fl = G.HFILE_list;
	طالما (fl) {
		إذا (fl->fd == fd)
			أرجع 1;
		fl = fl->بعدی_hfile;
	}
	أرجع 0;
}

#پایان_اگر /* !__U_BOOT__ */

/* Helpers for setting new $n and restoring them back
 */
تعريفنوع بنية save_arg_t {
	حرف *sv_argv0;
	حرف **sv_g_argv;
	صحيح sv_g_argc;
#اگر_تعریف_نشده __U_BOOT__
	IF_HUSH_مجموعه(smallint sv_g_malloced;)
#پایان_اگر /* !__U_BOOT__ */
} save_arg_t;

#اگر_تعریف_نشده __U_BOOT__
ساكن فراغ save_and_replace_G_آرگومانها(save_arg_t *sv, حرف **argv)
{
	sv->sv_argv0 = argv[0];
	sv->sv_g_argv = G.global_argv;
	sv->sv_g_argc = G.global_argc;
	IF_HUSH_مجموعه(sv->sv_g_malloced = G.global_آرگومانها_malloced;)

	argv[0] = G.global_argv[0]; /* retain $0 */
	G.global_argv = argv;
	IF_HUSH_مجموعه(G.global_آرگومانها_malloced = 0;)

	G.global_argc = 1 + string_array_len(argv + 1);
}

ساكن فراغ restore_G_آرگومانها(save_arg_t *sv, حرف **argv)
{
#إذا ENABLE_HUSH_مجموعه
	إذا (G.global_آرگومانها_malloced) {
		/* someone ran "set -- arg1 arg2 ...", undo */
		حرف **pp = G.global_argv;
		طالما (*++pp) /* note: does not free $0 */
			حرر(*pp);
		حرر(G.global_argv);
	}
#پایان_اگر
	argv[0] = sv->sv_argv0;
	G.global_argv = sv->sv_g_argv;
	G.global_argc = sv->sv_g_argc;
	IF_HUSH_مجموعه(G.global_آرگومانها_malloced = sv->sv_g_malloced;)
}
#پایان_اگر /* !__U_BOOT__ */

#اگر_تعریف_نشده __U_BOOT__
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
تعداد {
	SPECIAL_INTERACTIVE_SIGS = 0
		| (1 << SIGTERM)
		| (1 << SIGINT)
		| (1 << SIGHUP)
		,
	SPECIAL_JOBSTOP_SIGS = 0
#إذا ENABLE_HUSH_کار
		| (1 << SIGTTIN)
		| (1 << SIGTTOU)
		| (1 << SIGTSTP)
#پایان_اگر
		,
};

ساكن فراغ رکورد_pending_signo(صحيح sig)
{
	sigaddset(&G.pending_مجموعه, sig);
#إذا ENABLE_FEATURE_EDITING
	إذا (sig != SIGCHLD
	 || (G_traps && G_traps[SIGCHLD] && G_traps[SIGCHLD][0])
	 /* ^^^ if SIGCHLD, interrupt line reading only if it has a trap */
	) {
		bb_got_signal = sig; /* for read_line_input: "we got a signal" */
	}
#پایان_اگر
#إذا ENABLE_HUSH_FAST
	إذا (sig == SIGCHLD) {
		G.شمار_SIGCHLD++;
//bb_error_msg("[%d] SIGCHLD_handler: G.count_SIGCHLD:%d G.handled_SIGCHLD:%d", getpid(), G.count_SIGCHLD, G.handled_SIGCHLD);
	}
#پایان_اگر
}

ساكن sighandler_t install_sighandler(صحيح sig, sighandler_t handler)
{
	بنية sigaction old_sa;

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
	أرجع old_sa.sa_handler;
}
#پایان_اگر /* !__U_BOOT__ */

#اگر_تعریف_نشده __U_BOOT__
ساكن فراغ hush_exit(صحيح exitcode) NORETURN;

ساكن فراغ restore_ttypgrp_and__exit(فراغ) NORETURN;
ساكن فراغ restore_ttypgrp_and__exit(فراغ)
{
	/* xfunc has failed! die die die */
	/* no EXIT traps, this is an escape hatch! */
	G.exiting = 1;
	hush_exit(xfunc_error_retval);
}

#إذا ENABLE_HUSH_کار

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
ساكن فراغ fflush_and__exit(فراغ) NORETURN;
ساكن فراغ fflush_and__exit(فراغ)
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
ساكن فراغ sigexit(صحيح sig) NORETURN;
ساكن فراغ sigexit(صحيح sig)
{
	/* Careful: we can end up here after [v]fork. Do not restore
	 * tty pgrp then, only top-level shell process does that */
	إذا (G_saved_tty_pgrp && getpid() == G.ریشه_pid) {
		/* Disable all signals: job control, SIGPIPE, etc.
		 * Mostly paranoid measure, to prevent infinite SIGTTOU.
		 */
		sigprocmask_allsigs(SIG_بلوک);
		tcsetpgrp(G_interactive_fd, G_saved_tty_pgrp);
	}

	/* Not a signal, just exit */
	إذا (sig <= 0)
		_exit(- sig);

	kill_myself_with_sig(sig); /* does not return */
}
#وإلا

# define disable_restore_tty_pgrp_on_exit() ((فراغ)0)
# define enable_restore_tty_pgrp_on_exit()  ((فراغ)0)

#پایان_اگر
#پایان_اگر /* !__U_BOOT__ */

#اگر_تعریف_نشده __U_BOOT__
ساكن sighandler_t pick_sighandler(بیعلامت sig)
{
	sighandler_t handler = SIG_DFL;
	إذا (sig < حجم(بیعلامت)*8) {
		بیعلامت sigmask = (1 << sig);

#إذا ENABLE_HUSH_کار
		/* is sig fatal? */
		إذا (G_fatal_sig_ماسک & sigmask)
			handler = sigexit;
		وإلا
#پایان_اگر
		/* sig has special handling? */
		إذا (G.special_sig_ماسک & sigmask) {
			handler = رکورد_pending_signo;
			/* TTIN/TTOU/TSTP can't be set to record_pending_signo
			 * in order to ignore them: they will be raised
			 * in an endless loop when we try to do some
			 * terminal ioctls! We do have to _ignore_ these.
			 */
			إذا (SPECIAL_JOBSTOP_SIGS & sigmask)
				handler = SIG_IGN;
		}
	}
	أرجع handler;
}
#پایان_اگر /* !__U_BOOT__ */

#اگر_تعریف_نشده __U_BOOT__
/* Restores tty foreground process group, and exits. */
ساكن فراغ hush_exit(صحيح exitcode)
{
#إذا ENABLE_FEATURE_EDITING_SAVE_ON_EXIT
	save_history(G.سطر_درون_داد_state); /* may be NULL */
#پایان_اگر

	fflush_all();
	إذا (G.exiting <= 0 && G_traps && G_traps[0] && G_traps[0][0]) {
		حرف *argv[3];
		/* argv[0] is unused */
		argv[1] = xstrdup(G_traps[0]); /* copy, since EXIT trap handler may modify G_traps[0] */
		argv[2] = عدم;
		G.exiting = 1; /* prevent EXIT trap recursion */
		/* Note: G_traps[0] is not cleared!
		 * "trap" will still show it, if executed
		 * in the handler */
		builtin_eval(argv);
	}

#إذا ENABLE_FEATURE_CLEAN_به_بالا
	{
		بنية variable *cur_var;
		إذا (G.cwd != bb_پیام_unknown)
			حرر((حرف*)G.cwd);
		cur_var = G.بالاترین_var;
		طالما (cur_var) {
			بنية variable *tmp = cur_var;
			إذا (!cur_var->max_len)
				حرر(cur_var->varstr);
			cur_var = cur_var->بعدی;
			حرر(tmp);
		}
	}
#پایان_اگر

	fflush_all();
#إذا ENABLE_HUSH_کار
	sigexit(- (exitcode & 0xff));
#وإلا
	_exit(exitcode);
#پایان_اگر
}

//TODO: return a mask of ALL handled sigs?
ساكن صحيح check_and_پیاپی_traps(فراغ)
{
	صحيح آخرین_sig = 0;

	طالما (1) {
		صحيح sig;

		إذا (sigisemptyset(&G.pending_مجموعه))
			اكسر;
		sig = 0;
		افعل {
			sig++;
			إذا (sigismember(&G.pending_مجموعه, sig)) {
				sigdelset(&G.pending_مجموعه, sig);
				اذهب got_sig;
			}
		} طالما (sig < NSIG);
		اكسر;
 got_sig:
#إذا ENABLE_HUSH_TRAP
		إذا (G_traps && G_traps[sig]) {
			debug_printf_exec("%s: sig:%d handler:'%s'\n", __func__, sig, G.traps[sig]);
			إذا (G_traps[sig][0]) {
				/* We have user-defined handler */
				smalluint save_rcode;
				صحيح save_pre;
				حرف *argv[3];
				/* argv[0] is unused */
				argv[1] = xstrdup(G_traps[sig]);
				/* why strdup? trap can modify itself: trap 'trap "echo oops" INT' INT */
				argv[2] = عدم;
				save_pre = G.pre_trap_exitcode;
				G.pre_trap_exitcode = save_rcode = G.آخرین_exitcode;
				builtin_eval(argv);
				حرر(argv[1]);
				G.pre_trap_exitcode = save_pre;
				G.آخرین_exitcode = save_rcode;
# إذا ENABLE_HUSH_FUNCTIONS
				إذا (G.return_exitcode >= 0) {
					debug_printf_exec("trap exitcode:%d\n", G.return_exitcode);
					G.آخرین_exitcode = G.return_exitcode;
				}
# endif
				آخرین_sig = sig;
			} /* else: "" trap, ignoring signal */
			استمر;
		}
#پایان_اگر
		/* not a trap: special action */
		بدل (sig) {
		حالة SIGINT:
			debug_printf_exec("%s: sig:%d default SIGINT handler\n", __func__, sig);
			G.flag_SIGINT = 1;
			آخرین_sig = sig;
			اكسر;
#إذا ENABLE_HUSH_کار
		حالة SIGHUP: {
			/* if (G_interactive_fd) - no need to check, the handler
			 * is only installed if we *are* interactive */
			{
				/* bash compat: "Before exiting, an interactive
				 * shell resends the SIGHUP to all jobs, running
				 * or stopped.  Stopped jobs are sent SIGCONT
				 * to ensure that they receive the SIGHUP."
				 */
				بنية pipe *کار;
				debug_printf_exec("%s: sig:%d default SIGHUP handler\n", __func__, sig);
				/* bash is observed to signal whole process groups,
				 * not individual processes */
				لكل (کار = G.کار_list; کار; کار = کار->بعدی) {
					إذا (کار->pgrp <= 0)
						استمر;
					debug_printf_exec("HUPing pgrp %d\n", کار->pgrp);
					إذا (kill(- کار->pgrp, SIGHUP) == 0)
						kill(- کار->pgrp, SIGCONT);
				}
			}
			/* this restores tty pgrp, then kills us with SIGHUP */
			sigexit(SIGHUP);
		}
#پایان_اگر
#إذا ENABLE_HUSH_FAST
		حالة SIGCHLD:
			debug_printf_exec("%s: sig:%d default SIGCHLD handler\n", __func__, sig);
			G.شمار_SIGCHLD++;
//bb_error_msg("[%d] check_and_run_traps: G.count_SIGCHLD:%d G.handled_SIGCHLD:%d", getpid(), G.count_SIGCHLD, G.handled_SIGCHLD);
			/* Note:
			 * We don't do 'last_sig = sig' here -> NOT returning this sig.
			 * This simplifies wait builtin a bit.
			 */
			اكسر;
#پایان_اگر
		افتراضي: /* ignored: */
			debug_printf_exec("%s: sig:%d default handling is to ignore\n", __func__, sig);
			/* SIGTERM, SIGQUIT, SIGTTIN, SIGTTOU, SIGTSTP */
			/* Note:
			 * We don't do 'last_sig = sig' here -> NOT returning this sig.
			 * Example: wait is not interrupted by TERM
			 * in interactive shell, because TERM is ignored.
			 */
			اكسر;
		}
	}
	أرجع آخرین_sig;
}

ساكن ثابت حرف *get_cwd(صحيح force)
{
	إذا (force || G.cwd == عدم) {
		/* xrealloc_getcwd_or_warn(arg) calls free(arg),
		 * we must not try to free(bb_msg_unknown) */
		إذا (G.cwd == bb_پیام_unknown)
			G.cwd = عدم;
		G.cwd = xrealloc_getcwd_or_warn((حرف *)G.cwd);
		إذا (!G.cwd)
			G.cwd = bb_پیام_unknown;
	}
	أرجع G.cwd;
}

#پایان_اگر /* !__U_BOOT__ */

/*
 * Shell and environment variable support
 */
ساكن بنية variable **get_ptr_مقصد_local_var(ثابت حرف *name)
{
	بنية variable **pp;
	بنية variable *cur;

	pp = &G.بالاترین_var;
	طالما ((cur = *pp) != عدم) {
		إذا (varcmp(cur->varstr, name) == 0)
			أرجع pp;
		pp = &cur->بعدی;
	}
	أرجع عدم;
}

ساكن ثابت حرف* FAST_FUNC get_local_var_مقدار(ثابت حرف *name)
{
	بنية variable **vpp;

	إذا (G.expanded_assignments) {
		حرف **cpp = G.expanded_assignments;
		طالما (*cpp) {
			حرف *cp = *cpp;
			إذا (varcmp(cp, name) == 0)
				أرجع strchr(cp, '=') + 1;
			cpp++;
		}
	}

	vpp = get_ptr_مقصد_local_var(name);
	إذا (vpp)
		أرجع strchr((*vpp)->varstr, '=') + 1;

#اگر_تعریف_نشده __U_BOOT__
	إذا (strcmp(name, "PPID") == 0)
		أرجع utoa(G.ریشه_ppid);
#پایان_اگر /* !__U_BOOT__ */
	// bash compat: UID? EUID?
#إذا ENABLE_HUSH_RANDOM_SUPPORT
	إذا (strcmp(name, "RANDOM") == 0)
		أرجع utoa(بعدی_random(&G.random_gen));
#پایان_اگر
#إذا ENABLE_HUSH_LINENO_VAR
	إذا (strcmp(name, "LINENO") == 0)
		أرجع utoa(G.execute_lineno);
#پایان_اگر
#إذا BASH_EPOCH_VARS
	{
		ثابت حرف *fmt = عدم;
		إذا (strcmp(name, "EPOCHSECONDS") == 0)
			fmt = "%llu";
		وإلا إذا (strcmp(name, "EPOCHREALTIME") == 0)
			fmt = "%llu.%06u";
		إذا (fmt) {
			بنية timeval tv;
			xgettimeofday(&tv);
			sprintf(G.epoch_buf, fmt, (بیعلامت دراز دراز)tv.tv_sec,
					(بیعلامت)tv.tv_usec);
			أرجع G.epoch_buf;
		}
	}
#پایان_اگر
	أرجع عدم;
}

#اگر_تعریف_نشده __U_BOOT__
#إذا ENABLE_HUSH_GETOPTS
ساكن فراغ handle_تغییریافته_special_نامها(ثابت حرف *name)
{
	إذا (varcmp(name, "OPTIND") == 0) {
		G.getopt_شمار = 0;
		أرجع;
	}
}
#وإلا
/* Do not even bother evaluating arguments */
# define handle_تغییریافته_special_نامها(...) ((فراغ)0)
#پایان_اگر
#وإلا /* __U_BOOT__ */
/* Do not even bother evaluating arguments */
# define handle_تغییریافته_special_نامها(...) ((فراغ)0)
#پایان_اگر /* __U_BOOT__ */

/* str holds "NAME=VAL" and is expected to be malloced.
 * We take ownership of it.
 */
#اگر_تعریف_نشده __U_BOOT__
#تعریف_کن SETFLAG_EXPORT   (1 << 0)
#تعریف_کن SETFLAG_UNEXPORT (1 << 1)
#تعریف_کن SETFLAG_MAKE_RO  (1 << 2)
#پایان_اگر /* !__U_BOOT__ */
#تعریف_کن SETFLAG_VARLVL_لغزش   3
#اگر_تعریف_نشده __U_BOOT__
ساكن صحيح مجموعه_local_var(حرف *str, بیعلامت flags)
#وإلا /* __U_BOOT__ */
صحيح مجموعه_local_var_modern(حرف *str, صحيح flags)
#پایان_اگر /* __U_BOOT__ */
{
	بنية variable **cur_pp;
	بنية variable *cur;
	حرف *free_me = عدم;
	حرف *eq_sign;
	صحيح name_len;
	صحيح retval;
#اگر_تعریف_نشده __U_BOOT__
	بیعلامت local_lvl = (flags >> SETFLAG_VARLVL_لغزش);
#پایان_اگر /* !__U_BOOT__ */

	eq_sign = strchr(str, '=');
	إذا (HUSH_DEBUG && !eq_sign)
		bb_simple_error_پیام_and_die("BUG in setvar");

	name_len = eq_sign - str + 1; /* including '=' */
	cur_pp = &G.بالاترین_var;
	طالما ((cur = *cur_pp) != عدم) {
		إذا (مقایسه_رشته(cur->varstr, str, name_len) != 0) {
			cur_pp = &cur->بعدی;
			استمر;
		}

#اگر_تعریف_نشده __U_BOOT__
		/* We found an existing var with this name */
		إذا (cur->flg_read_only) {
			bb_error_پیام("%s: readonly variable", str);
			حرر(str);
//NOTE: in bash, assignment in "export READONLY_VAR=Z" fails, and sets $?=1,
//but export per se succeeds (does put the var in env). We don't mimic that.
			أرجع -1;
		}
		إذا (flags & SETFLAG_UNEXPORT) { // && cur->flg_export ?
			debug_printf_env("%s: unsetenv '%s'\n", __func__, str);
			*eq_sign = '\0';
			unsetenv(str);
			*eq_sign = '=';
		}
		إذا (cur->var_nest_level < local_lvl) {
			/* bash 3.2.33(1) and exported vars:
			 * # export z=z
			 * # f() { local z=a; env | grep ^z; }
			 * # f
			 * z=a
			 * # env | grep ^z
			 * z=z
			 */
			إذا (cur->flg_export)
				flags |= SETFLAG_EXPORT;
			/* New variable is local ("local VAR=VAL" or
			 * "VAR=VAL cmd")
			 * and existing one is global, or local
			 * on a lower level that new one.
			 * Remove it from global variable list:
			 */
			*cur_pp = cur->بعدی;
			إذا (G.shadowed_vars_pp) {
				/* Save in "shadowed" list */
				debug_printf_env("shadowing %s'%s'/%u by '%s'/%u\n",
					cur->flg_export ? "exported " : "",
					cur->varstr, cur->var_nest_level, str, local_lvl
				);
				cur->بعدی = *G.shadowed_vars_pp;
				*G.shadowed_vars_pp = cur;
			} وإلا {
				/* Came from pseudo_exec_argv(), no need to save: delete it */
				debug_printf_env("shadow-deleting %s'%s'/%u by '%s'/%u\n",
					cur->flg_export ? "exported " : "",
					cur->varstr, cur->var_nest_level, str, local_lvl
				);
				إذا (cur->max_len == 0) /* allocated "VAR=VAL"? */
					free_me = cur->varstr; /* then free it later */
				حرر(cur);
			}
			اكسر;
		}
#پایان_اگر /* !__U_BOOT__ */

		إذا (strcmp(cur->varstr + name_len, eq_sign + 1) == 0) {
			debug_printf_env("assignement '%s' does not change anything\n", str);
 free_and_exp:
			حرر(str);
			اذهب نمایی_تابع;
		}

		/* Replace the value in the found "struct variable" */
		إذا (cur->max_len != 0) {
			إذا (cur->max_len >= strnlen(str, cur->max_len + 1)) {
				/* This one is from startup env, reuse space */
				debug_printf_env("reusing startup env for '%s'\n", str);
				strcpy(cur->varstr, str);
				اذهب free_and_exp;
			}
			/* Can't reuse */
			cur->max_len = 0;
			اذهب مجموعه_str_and_exp;
		}
		/* max_len == 0 signifies "malloced" var, which we can
		 * (and have to) free. But we can't free(cur->varstr) here:
		 * if cur->flg_export is 1, it is in the environment.
		 * We should either unsetenv+free, or wait until putenv,
		 * then putenv(new)+free(old).
		 */
		free_me = cur->varstr;
		اذهب مجموعه_str_and_exp;
	}

	/* Not found or shadowed - create new variable struct */
#اگر_تعریف_نشده __U_BOOT__
	debug_printf_env("%s: alloc new var '%s'/%u\n", __func__, str, local_lvl);
#وإلا /* __U_BOOT__ */
	debug_printf_env("%s: alloc new var '%s'\n", __func__, str);
#پایان_اگر /* __U_BOOT__ */
	cur = xzalloc(حجم(*cur));
#اگر_تعریف_نشده __U_BOOT__
	cur->var_nest_level = local_lvl;
#پایان_اگر /* !__U_BOOT__ */
	cur->بعدی = *cur_pp;
	*cur_pp = cur;

 مجموعه_str_and_exp:
	cur->varstr = str;
 نمایی_تابع:
#اگر_تعریف_نشده __U_BOOT__
#إذا !BB_MMU || ENABLE_HUSH_READONLY
	إذا (flags & SETFLAG_MAKE_RO) {
		cur->flg_read_only = 1;
	}
#پایان_اگر
	إذا (flags & SETFLAG_EXPORT)
		cur->flg_export = 1;
#پایان_اگر /* !__U_BOOT__ */
	retval = 0;
#اگر_تعریف_نشده __U_BOOT__
	إذا (cur->flg_export) {
		إذا (flags & SETFLAG_UNEXPORT) {
			cur->flg_export = 0;
			/* unsetenv was already done */
		} وإلا {
			debug_printf_env("%s: putenv '%s'/%u\n", __func__, cur->varstr, cur->var_nest_level);
			retval = putenv(cur->varstr);
			/* fall through to "free(free_me)" -
			 * only now we can free old exported malloced string
			 */
		}
	}
#پایان_اگر /* !__U_BOOT__ */
	حرر(free_me);

	handle_تغییریافته_special_نامها(cur->varstr);

	أرجع retval;
}

#اگر_تعریف_نشده __U_BOOT__
ساكن صحيح مجموعه_local_var0(حرف *str)
{
	أرجع مجموعه_local_var(str, 0);
}

ساكن فراغ FAST_FUNC مجموعه_local_var_from_halves(ثابت حرف *name, ثابت حرف *val)
{
	حرف *var = xasprintf("%s=%s", name, val);
	مجموعه_local_var0(var);
}

/* Used at startup and after each cd */
ساكن فراغ مجموعه_pwd_var(بیعلامت flag)
{
	مجموعه_local_var(xasprintf("PWD=%s", get_cwd(/*force:*/ 1)), flag);
}
#پایان_اگر /* !__U_BOOT__ */

#إذا ENABLE_HUSH_UNSET || ENABLE_HUSH_GETOPTS
ساكن صحيح unset_local_var(ثابت حرف *name)
{
	بنية variable *cur;
	بنية variable **cur_pp;

	cur_pp = &G.بالاترین_var;
	طالما ((cur = *cur_pp) != عدم) {
		إذا (varcmp(cur->varstr, name) == 0) {
			إذا (cur->flg_read_only) {
				bb_error_پیام("%s: readonly variable", name);
				أرجع EXIT_FAILURE;
			}

			*cur_pp = cur->بعدی;
			debug_printf_env("%s: unsetenv '%s'\n", __func__, cur->varstr);
			bb_unsetenv(cur->varstr);
			إذا (!cur->max_len)
				حرر(cur->varstr);
			حرر(cur);

			اكسر;
		}
		cur_pp = &cur->بعدی;
	}

	/* Handle "unset LINENO" et al even if did not find the variable to unset */
	handle_تغییریافته_special_نامها(name);

	أرجع EXIT_SUCCESS;
}
#پایان_اگر

#اگر_تعریف_نشده __U_BOOT__
/*
 * Helpers for "var1=val1 var2=val2 cmd" feature
 */
ساكن فراغ جمع_کن_vars(بنية variable *var)
{
	بنية variable *بعدی;

	طالما (var) {
		بعدی = var->بعدی;
		var->بعدی = G.بالاترین_var;
		G.بالاترین_var = var;
		إذا (var->flg_export) {
			debug_printf_env("%s: restoring exported '%s'/%u\n", __func__, var->varstr, var->var_nest_level);
			putenv(var->varstr);
		} وإلا {
			debug_printf_env("%s: restoring variable '%s'/%u\n", __func__, var->varstr, var->var_nest_level);
		}
		var = بعدی;
	}
}

/* We put strings[i] into variable table and possibly putenv them.
 * If variable is read only, we can free the strings[i]
 * which attempts to overwrite it.
 * The strings[] vector itself is freed.
 */
ساكن فراغ مجموعه_vars_and_save_old(حرف **رشتهها)
{
	حرف **s;

	إذا (!رشتهها)
		أرجع;

	s = رشتهها;
	طالما (*s) {
		بنية variable *var_p;
		بنية variable **var_pp;
		حرف *eq;

		eq = strchr(*s, '=');
		إذا (HUSH_DEBUG && !eq)
			bb_simple_error_پیام_and_die("BUG in varexp4");
		var_pp = get_ptr_مقصد_local_var(*s);
		إذا (var_pp) {
			var_p = *var_pp;
			إذا (var_p->flg_read_only) {
				حرف **p;
				bb_error_پیام("%s: readonly variable", *s);
				/*
				 * "VAR=V BLTIN" unsets VARs after BLTIN completes.
				 * If VAR is readonly, leaving it in the list
				 * after asssignment error (msg above)
				 * causes doubled error message later, on unset.
				 */
				debug_printf_env("removing/freeing '%s' element\n", *s);
				حرر(*s);
				p = s;
				افعل { *p = p[1]; p++; } طالما (*p);
				اذهب بعدی;
			}
			/* below, set_local_var() with nest level will
			 * "shadow" (remove) this variable from
			 * global linked list.
			 */
		}
		debug_printf_env("%s: env override '%s'/%u\n", __func__, *s, G.var_nest_level);
		مجموعه_local_var(*s, (G.var_nest_level << SETFLAG_VARLVL_لغزش) | SETFLAG_EXPORT);
		s++;
 بعدی: ;
	}
	حرر(رشتهها);
}

/*
 * Unicode helper
 */
ساكن فراغ reinit_unicode_for_hush(فراغ)
{
	/* Unicode support should be activated even if LANG is set
	 * _during_ shell execution, not only if it was set when
	 * shell was started. Therefore, re-check LANG every time:
	 */
	إذا (ENABLE_FEATURE_CHECK_UNICODE_درون_ENV
	 || ENABLE_UNICODE_USING_LOCALE
	) {
		ثابت حرف *s = get_local_var_مقدار("LC_ALL");
		إذا (!s) s = get_local_var_مقدار("LC_CTYPE");
		إذا (!s) s = get_local_var_مقدار("LANG");
		reinit_unicode(s);
	}
}

#پایان_اگر /* !__U_BOOT__ */
/*
 * in_str support (strings, and "strings" read from files).
 */

#إذا ENABLE_HUSH_INTERACTIVE
#اگر_تعریف_نشده __U_BOOT__
/* To test correct lineedit/interactive behavior, type from command line:
 *	echo $P\
 *	\
 *	AT\
 *	H\
 *	\
 * It exercises a lot of corner cases.
 */
ساكن ثابت حرف *راهاندازی_prompt_string(فراغ)
{
	ثابت حرف *prompt_str;

	debug_printf_prompt("%s promptmode:%d\n", __func__, G.promptmode);

# إذا ENABLE_FEATURE_EDITING_FANCY_PROMPT
	prompt_str = get_local_var_مقدار(G.promptmode == 0 ? "PS1" : "PS2");
	إذا (!prompt_str)
		prompt_str = "";
# وإلا
	prompt_str = "> "; /* if PS2, else... */
	إذا (G.promptmode == 0) { /* PS1 */
		/* No fancy prompts supported, (re)generate "CURDIR $ " by hand */
		حرر(G.PS1);
		/* bash uses $PWD value, even if it is set by user.
		 * It uses current dir only if PWD is unset.
		 * We always use current dir. */
		prompt_str = G.PS1 = xasprintf("%s %c ", get_cwd(0), (geteuid() != 0) ? '$' : '#');
	}
# endif
	debug_printf("prompt_str '%s'\n", prompt_str);
	أرجع prompt_str;
}
#پایان_اگر /* !__U_BOOT__ */

#اگر_تعریف_نشده __U_BOOT__
ساكن صحيح get_user_درون_داد(بنية درون_str *i)
#وإلا /* __U_BOOT__ */
ساكن فراغ get_user_درون_داد(بنية درون_str *i)
#پایان_اگر /* __U_BOOT__ */
{
#اگر_تعریف_نشده __U_BOOT__
# إذا ENABLE_FEATURE_EDITING
	/* In EDITING case, this function reads next input line,
	 * saves it in i->p, then returns 1st char of it.
	 */
	صحيح r;
	ثابت حرف *prompt_str;

	prompt_str = راهاندازی_prompt_string();
	لكل (;;) {
		reinit_unicode_for_hush();
		G.flag_SIGINT = 0;

		bb_got_signal = 0;
		إذا (!sigisemptyset(&G.pending_مجموعه)) {
			/* Whoops, already got a signal, do not call read_line_input */
			bb_got_signal = r = -1;
		} وإلا {
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
			r = read_سطر_درون_داد(G.سطر_درون_داد_state, prompt_str,
				G.user_درون_داد_buf, CONFIG_FEATURE_EDITING_MAX_LEN-1
			);
			/* read_line_input intercepts ^C, "convert" it to SIGINT */
			إذا (r == 0)
				ارفع(SIGINT);
		}
		/* bash prints ^C (before running a trap, if any)
		 * both on keyboard ^C and on real SIGINT (non-kbd generated).
		 */
		إذا (sigismember(&G.pending_مجموعه, SIGINT)) {
			اكتب(STDOUT_FILENO, "^C\n", 3);
			G.آخرین_exitcode = 128 | SIGINT;
		}
		check_and_پیاپی_traps();
		إذا (r == 0) /* keyboard ^C? */
			استمر; /* go back, read another input line */
		إذا (r > 0) /* normal input? (no ^C, no ^D, no signals) */
			اكسر;
		إذا (!bb_got_signal) {
			/* r < 0: ^D/EOF/error detected (but not signal) */
			/* ^D on interactive input goes to next line before exiting: */
			اكتب(STDOUT_FILENO, "\n", 1);
			i->p = عدم;
			i->peek_buf[0] = r = EOF;
			أرجع r;
		}
		/* it was a signal: go back, read another input line */
	}
	i->p = G.user_درون_داد_buf;
	أرجع (بیعلامت حرف)*i->p++;
# وإلا
	/* In !EDITING case, this function gets called for every char.
	 * Buffering happens deeper in the call chain, in hfgetc(i->file).
	 */
	صحيح r;

	لكل (;;) {
		G.flag_SIGINT = 0;
		إذا (i->آخرین_char == '\0' || i->آخرین_char == '\n') {
			ثابت حرف *prompt_str = راهاندازی_prompt_string();
			/* Why check_and_run_traps here? Try this interactively:
			 * $ trap 'echo INT' INT; (sleep 2; kill -INT $$) &
			 * $ <[enter], repeatedly...>
			 * Without check_and_run_traps, handler never runs.
			 */
			check_and_پیاپی_traps();
			fputs_stdout(prompt_str);
			fflush_all();
		}
		r = hfgetc(i->file);
		/* In !ENABLE_FEATURE_EDITING we don't use read_line_input,
		 * no ^C masking happens during fgetc, no special code for ^C:
		 * it generates SIGINT as usual.
		 */
		check_and_پیاپی_traps();
		إذا (r != '\0' && !G.flag_SIGINT)
			اكسر;
		إذا (G.flag_SIGINT) {
			/* ^C or SIGINT: repeat */
			/* bash prints ^C even on real SIGINT (non-kbd generated) */
			/* kernel prints "^C" itself, just print newline: */
			اكتب(STDOUT_FILENO, "\n", 1);
			G.آخرین_exitcode = 128 | SIGINT;
		}
	}
	أرجع r;
# endif
#وإلا /* __U_BOOT__ */
	صحيح n;
	صحيح promptme;
	ساكن حرف the_command[CONFIG_SYS_CBSIZE + 1];

	bootretry_reset_cmd_timeout();
	promptme = 1;
	n = u_boot_cli_readline(i);

# ifdef CONFIG_BOOT_RETRY_زمان
	إذا (n == -2) {
		puts("\nTimeout waiting for command\n");
#  ifdef CONFIG_RESET_مقصد_RETRY
		do_reset(عدم, 0, 0, عدم);
#  elif IS_ENABLED(CONFIG_RETRY_BOOTCMD)
	strcpy(console_میانگیر, "run bootcmd\n");
# وإلا
#	error "This only works with CONFIG_RESET_TO_RETRY or CONFIG_BOOT_RETRY_COMMAND enabled"
#  endif
	}
# endif
	إذا (n == -1 ) {
		G.flag_repeat = 0;
		promptme = 0;
	}
	n = طول_رشته(console_میانگیر);
	console_میانگیر[n] = '\n';
	console_میانگیر[n+1]= '\0';
	إذا (had_ctrlc())
		G.flag_repeat = 0;
	clear_ctrlc();
	G.do_repeat = 0;
#اگر_تعریف_نشده __U_BOOT__
	إذا (G.promptmode == 1) {
#وإلا /* __U_BOOT__ */
	إذا (!G.promptmode) {
#پایان_اگر /* __U_BOOT__ */
		إذا (console_میانگیر[0] == '\n'&& G.flag_repeat == 0) {
			strcpy(the_command, console_میانگیر);
		}
		وإلا {
			إذا (console_میانگیر[0] != '\n') {
				strcpy(the_command, console_میانگیر);
				G.flag_repeat = 1;
			}
			وإلا {
				G.do_repeat = 1;
			}
		}
		i->p = the_command;
	}
	وإلا {
		إذا (console_میانگیر[0] != '\n') {
			إذا (طول_رشته(the_command) + طول_رشته(console_میانگیر)
				< CONFIG_SYS_CBSIZE) {
				n = طول_رشته(the_command);
#اگر_تعریف_شده __U_BOOT__
				/*
				 * To avoid writing to bad places, we check if
				 * n is greater than 0.
				 * This bug was found by Harald Seiler.
				 */
				إذا (n > 0)
					the_command[n-1] = ' ';
				strcpy(&the_command[n], console_میانگیر);
#وإلا /* !__U_BOOT__ */
			the_command[n-1] = ' ';
			strcpy(&the_command[n], console_میانگیر);
#پایان_اگر /* !__U_BOOT__ */
				}
				وإلا {
					the_command[0] = '\n';
					the_command[1] = '\0';
					G.flag_repeat = 0;
				}
		}
		إذا (promptme == 0) {
			the_command[0] = '\n';
			the_command[1] = '\0';
		}
		i->p = console_میانگیر;
	}
#پایان_اگر /* __U_BOOT__ */
}
/* This is the magic location that prints prompts
 * and gets data back from the user */
ساكن صحيح fgetc_interactive(بنية درون_str *i)
{
	صحيح ch;
#اگر_تعریف_نشده __U_BOOT__
	/* If it's interactive stdin, get new line. */
	إذا (G_interactive_fd && i->file == G.HFILE_stdin) {
#پایان_اگر /* !__U_BOOT__ */
#اگر_تعریف_نشده __U_BOOT__
		/* Returns first char (or EOF), the rest is in i->p[] */
		ch = get_user_درون_داد(i);
#وإلا /* __U_BOOT__ */
		/* Avoid garbage value and make clang happy. */
		ch = 0;
		/*
		 * get_user_input() does not return anything when used in
		 * U-Boot.
		 * So, we need to take the read character from i->p[].
		 */
		get_user_درون_داد(i);
		إذا (i->p && *i->p) {
			ch = *i->p++;
		}
#پایان_اگر /* __U_BOOT__ */
		G.promptmode = 1; /* PS2 */
		debug_printf_prompt("%s promptmode=%d\n", __func__, G.promptmode);
#اگر_تعریف_نشده __U_BOOT__
	} وإلا {
		/* Not stdin: script file, sourced file, etc */
		افعل ch = hfgetc(i->file); طالما (ch == '\0');
	}
#پایان_اگر /* !__U_BOOT__ */
	أرجع ch;
}
#وإلا  /* !INTERACTIVE */
#اگر_تعریف_نشده __U_BOOT__
ساكن ALWAYS_INLINE صحيح fgetc_interactive(بنية درون_str *i)
{
	صحيح ch;
	افعل ch = hfgetc(i->file); طالما (ch == '\0');
	أرجع ch;
}
#پایان_اگر /* !__U_BOOT__ */
#پایان_اگر  /* !INTERACTIVE */

ساكن صحيح i_getch(بنية درون_str *i)
{
	صحيح ch;

#اگر_تعریف_نشده __U_BOOT__
	إذا (!i->file) {
		/* string-based in_str */
		ch = (بیعلامت حرف)*i->p;
		إذا (ch != '\0') {
			i->p++;
			i->آخرین_char = ch;
#إذا ENABLE_HUSH_LINENO_VAR
			إذا (ch == '\n') {
				G.parse_lineno++;
				debug_printf_parse("G.parse_lineno++ = %u\n", G.parse_lineno);
			}
#پایان_اگر
			أرجع ch;
		}
		أرجع EOF;
	}

#پایان_اگر /* !__U_BOOT__ */
	/* FILE-based in_str */

#إذا ENABLE_FEATURE_EDITING
	/* This can be stdin, check line editing char[] buffer */
	إذا (i->p && *i->p != '\0') {
		ch = (بیعلامت حرف)*i->p++;
		اذهب بیرون;
#اگر_تعریف_نشده __U_BOOT__
	}
#وإلا /* __U_BOOT__ */
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
	} وإلا إذا (i->p && *i->p == '\0' && G.پیاپی_command_flags){
		أرجع EOF;
	}
#پایان_اگر /* __U_BOOT__ */
#پایان_اگر
#اگر_تعریف_نشده __U_BOOT__
	/* peek_buf[] is an int array, not char. Can contain EOF. */
	ch = i->peek_buf[0];
	إذا (ch != 0) {
		صحيح ch2 = i->peek_buf[1];
		i->peek_buf[0] = ch2;
		إذا (ch2 == 0) /* very likely, avoid redundant write */
			اذهب بیرون;
		i->peek_buf[1] = 0;
		اذهب بیرون;
	}

#پایان_اگر /* !__U_BOOT__ */
	ch = fgetc_interactive(i);
 بیرون:
	debug_printf("file_get: got '%c' %d\n", ch, ch);
	i->آخرین_char = ch;
#إذا ENABLE_HUSH_LINENO_VAR
	إذا (ch == '\n') {
		G.parse_lineno++;
		debug_printf_parse("G.parse_lineno++ = %u\n", G.parse_lineno);
	}
#پایان_اگر
	أرجع ch;
}

ساكن صحيح i_peek(بنية درون_str *i)
{
#اگر_تعریف_نشده __U_BOOT__
	صحيح ch;

	إذا (!i->file) {
		/* string-based in_str */
		/* Doesn't report EOF on NUL. None of the callers care. */
		أرجع (بیعلامت حرف)*i->p;
	}

	/* FILE-based in_str */

#إذا ENABLE_FEATURE_EDITING && ENABLE_HUSH_INTERACTIVE
	/* This can be stdin, check line editing char[] buffer */
	إذا (i->p && *i->p != '\0')
		أرجع (بیعلامت حرف)*i->p;
#پایان_اگر
	/* peek_buf[] is an int array, not char. Can contain EOF. */
	ch = i->peek_buf[0];
	إذا (ch != 0)
		أرجع ch;

	/* Need to get a new char */
	ch = fgetc_interactive(i);
	debug_printf("file_peek: got '%c' %d\n", ch, ch);

	/* Save it by either rolling back line editing buffer, or in i->peek_buf[0] */
#إذا ENABLE_FEATURE_EDITING && ENABLE_HUSH_INTERACTIVE
	إذا (i->p) {
		i->p -= 1;
		أرجع ch;
	}
#پایان_اگر
	i->peek_buf[0] = ch;
	/*i->peek_buf[1] = 0; - already is */
	أرجع ch;
#وإلا /* __U_BOOT__ */
	/* string-based in_str */
	/* Doesn't report EOF on NUL. None of the callers care. */
	أرجع (بیعلامت حرف)*i->p;
#پایان_اگر /* __U_BOOT__ */
}

/* Only ever called if i_peek() was called, and did not return EOF.
 * IOW: we know the previous peek saw an ordinary char, not EOF, not NUL,
 * not end-of-line. Therefore we never need to read a new editing line here.
 */
ساكن صحيح i_peek2(بنية درون_str *i)
{
#اگر_تعریف_نشده __U_BOOT__
	صحيح ch;
#پایان_اگر /* !__U_BOOT__ */

	/* There are two cases when i->p[] buffer exists.
	 * (1) it's a string in_str.
	 * (2) It's a file, and we have a saved line editing buffer.
	 * In both cases, we know that i->p[0] exists and not NUL, and
	 * the peek2 result is in i->p[1].
	 */
	إذا (i->p)
		أرجع (بیعلامت حرف)i->p[1];

#اگر_تعریف_نشده __U_BOOT__
	/* Now we know it is a file-based in_str. */

	/* peek_buf[] is an int array, not char. Can contain EOF. */
	/* Is there 2nd char? */
	ch = i->peek_buf[1];
	إذا (ch == 0) {
		/* We did not read it yet, get it now */
		افعل ch = hfgetc(i->file); طالما (ch == '\0');
		i->peek_buf[1] = ch;
	}

	debug_printf("file_peek2: got '%c' %d\n", ch, ch);
	أرجع ch;
#وإلا
	أرجع 0;
#پایان_اگر /* __U_BOOT__ */
}

ساكن صحيح i_getch_and_eat_bkslash_nl(بنية درون_str *درون_داد)
{
	لكل (;;) {
		صحيح ch, ch2;

		ch = i_getch(درون_داد);
		إذا (ch != '\\')
			أرجع ch;
		ch2 = i_peek(درون_داد);
		إذا (ch2 != '\n')
			أرجع ch;
		/* backslash+newline, skip it */
		i_getch(درون_داد);
	}
}

/* Note: this function _eats_ \<newline> pairs, safe to use plain
 * i_getch() after it instead of i_getch_and_eat_bkslash_nl().
 */
ساكن صحيح i_peek_and_eat_bkslash_nl(بنية درون_str *درون_داد)
{
	لكل (;;) {
		صحيح ch, ch2;

		ch = i_peek(درون_داد);
		إذا (ch != '\\')
			أرجع ch;
		ch2 = i_peek2(درون_داد);
		إذا (ch2 != '\n')
			أرجع ch;
		/* backslash+newline, skip it */
		i_getch(درون_داد);
		i_getch(درون_داد);
	}
}

#اگر_تعریف_نشده __U_BOOT__
ساكن فراغ راهاندازی_file_درون_str(بنية درون_str *i, HFILE *fp)
#وإلا /* __U_BOOT__ */
ساكن فراغ راهاندازی_file_درون_str(بنية درون_str *i)
#پایان_اگر /* __U_BOOT__ */
{
	memset(i, 0, حجم(*i));
#اگر_تعریف_نشده __U_BOOT__
	i->file = fp;
	/* i->p = NULL; */
#پایان_اگر /* !__U_BOOT__ */
}

ساكن فراغ راهاندازی_string_درون_str(بنية درون_str *i, ثابت حرف *s)
{
	memset(i, 0, حجم(*i));
	/*i->file = NULL */;
	i->p = s;
}

/*
 * o_string support
 */
#تعریف_کن B_CHUNK  (32 * حجم(حرف*))

ساكن فراغ o_reset_مقصد_خالی_unquoted(o_string *o)
{
	o->درازی = 0;
	o->has_quoted_part = 0;
	إذا (o->داده)
		o->داده[0] = '\0';
}

ساكن فراغ o_free_and_مجموعه_NULL(o_string *o)
{
	حرر(o->داده);
	memset(o, 0, حجم(*o));
}

ساكن ALWAYS_INLINE فراغ o_free(o_string *o)
{
	حرر(o->داده);
}

ساكن فراغ o_grow_by(o_string *o, صحيح len)
{
	إذا (o->درازی + len > o->maxlen) {
		o->maxlen += (2 * len) | (B_CHUNK-1);
		o->داده = xrealloc(o->داده, 1 + o->maxlen);
	}
}

ساكن فراغ o_addchr(o_string *o, صحيح ch)
{
	debug_printf("o_addchr: '%c' o->length=%d o=%p\n", ch, o->درازی, o);
	إذا (o->درازی < o->maxlen) {
		/* likely. avoid o_grow_by() call */
 جمع_کن:
		o->داده[o->درازی] = ch;
		o->درازی++;
		o->داده[o->درازی] = '\0';
		أرجع;
	}
	o_grow_by(o, 1);
	اذهب جمع_کن;
}

#إذا 0
/* Valid only if we know o_string is not empty */
ساكن فراغ o_delchr(o_string *o)
{
	o->درازی--;
	o->داده[o->درازی] = '\0';
}
#پایان_اگر

ساكن فراغ o_addblock(o_string *o, ثابت حرف *str, صحيح len)
{
	o_grow_by(o, len);
	((حرف*)mempcpy(&o->داده[o->درازی], str, len))[0] = '\0';
	o->درازی += len;
}

ساكن فراغ o_addstr(o_string *o, ثابت حرف *str)
{
	o_addblock(o, str, طول_رشته(str));
}

#اگر_تعریف_نشده __U_BOOT__
ساكن فراغ o_addstr_with_NUL(o_string *o, ثابت حرف *str)
{
	o_addblock(o, str, طول_رشته(str) + 1);
}
#پایان_اگر /* !__U_BOOT__ */

#إذا !BB_MMU
ساكن فراغ nommu_addchr(o_string *o, صحيح ch)
{
	إذا (o)
		o_addchr(o, ch);
}
#وإلا
# define nommu_addchr(o, str) ((فراغ)0)
#پایان_اگر

#اگر_تعریف_نشده __U_BOOT__
#إذا ENABLE_HUSH_نما_X
ساكن فراغ x_نما_addchr(صحيح ch)
{
	o_addchr(&G.x_نما_buf, ch);
}
ساكن فراغ x_نما_addstr(ثابت حرف *str)
{
	o_addstr(&G.x_نما_buf, str);
}
ساكن فراغ x_نما_addblock(ثابت حرف *str, صحيح len)
{
	o_addblock(&G.x_نما_buf, str, len);
}
ساكن فراغ x_نما_پیشوند(فراغ)
{
	صحيح n = G.x_نما_عمق;
	افعل x_نما_addchr('+'); طالما (--n >= 0);
}
ساكن فراغ x_نما_flush(فراغ)
{
	صحيح len = G.x_نما_buf.درازی;
	إذا (len <= 0)
		أرجع;
	إذا (G.x_نما_fd > 0) {
		G.x_نما_buf.داده[len] = '\n';
		پر_write(G.x_نما_fd, G.x_نما_buf.داده, len + 1);
	}
	G.x_نما_buf.درازی = 0;
}
#پایان_اگر
#پایان_اگر /* !__U_BOOT__ */

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

#إذا ENABLE_HUSH_BRACE_EXPANSION
# define MAYBE_BRACES "{}"
#وإلا
# define MAYBE_BRACES ""
#پایان_اگر

/* My analysis of quoting semantics tells me that state information
 * is associated with a destination, not a source.
 */
ساكن فراغ o_addqchr(o_string *o, صحيح ch)
{
	صحيح sz = 1;
	/* '-' is included because of this case:
	 * >filename0 >filename1 >filename9; v='-'; echo filename[0"$v"9]
	 */
	حرف *یافت = strchr("*?[-\\" MAYBE_BRACES, ch);
	إذا (یافت)
		sz++;
	o_grow_by(o, sz);
	إذا (یافت) {
		o->داده[o->درازی] = '\\';
		o->درازی++;
	}
	o->داده[o->درازی] = ch;
	o->درازی++;
	o->داده[o->درازی] = '\0';
}

ساكن فراغ o_جمع_کن_Qchr(o_string *o, صحيح ch)
{
	صحيح sz = 1;
	إذا ((o->o_expflags & EXP_FLAG_ESC_GLOB_CHARS)
	 && strchr("*?[-\\" MAYBE_BRACES, ch)
	) {
		sz++;
		o->داده[o->درازی] = '\\';
		o->درازی++;
	}
	o_grow_by(o, sz);
	o->داده[o->درازی] = ch;
	o->درازی++;
	o->داده[o->درازی] = '\0';
}

ساكن فراغ o_addqblock(o_string *o, ثابت حرف *str, صحيح len)
{
	طالما (len) {
		حرف ch;
		صحيح sz;
		صحيح ordinary_cnt = strcspn(str, "*?[-\\" MAYBE_BRACES);
		إذا (ordinary_cnt > len) /* paranoia */
			ordinary_cnt = len;
		o_addblock(o, str, ordinary_cnt);
		إذا (ordinary_cnt == len)
			أرجع; /* NUL is already added by o_addblock */
		str += ordinary_cnt;
		len -= ordinary_cnt + 1; /* we are processing + 1 char below */

		ch = *str++;
		sz = 1;
		إذا (ch) { /* it is necessarily one of "*?[-\\" MAYBE_BRACES */
			sz++;
			o->داده[o->درازی] = '\\';
			o->درازی++;
		}
		o_grow_by(o, sz);
		o->داده[o->درازی] = ch;
		o->درازی++;
	}
	o->داده[o->درازی] = '\0';
}

ساكن فراغ o_جمع_کن_Qblock(o_string *o, ثابت حرف *str, صحيح len)
{
	إذا (!(o->o_expflags & EXP_FLAG_ESC_GLOB_CHARS)) {
		o_addblock(o, str, len);
		أرجع;
	}
	o_addqblock(o, str, len);
}

ساكن فراغ o_جمع_کن_Qstr(o_string *o, ثابت حرف *str)
{
	o_جمع_کن_Qblock(o, str, طول_رشته(str));
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
#إذا DEBUG_EXPAND || DEBUG_GLOB
ساكن فراغ debug_print_list(ثابت حرف *پیشوند, o_string *o, صحيح n)
{
	حرف **list = (حرف**)o->داده;
	صحيح string_شروع = ((n + 0xf) & ~0xf) * حجم(list[0]);
	صحيح i = 0;

	indent();
	fdprintf(2, "%s: list:%p n:%d string_start:%d length:%d maxlen:%d glob:%d quoted:%d escape:%d\n",
			پیشوند, list, n, string_شروع, o->درازی, o->maxlen,
			!!(o->o_expflags & EXP_FLAG_GLOB),
			o->has_quoted_part,
			!!(o->o_expflags & EXP_FLAG_ESC_GLOB_CHARS));
	طالما (i < n) {
		indent();
		fdprintf(2, " list[%d]=%d '%s' %p\n", i, (صحيح)(uintptr_t)list[i],
				o->داده + (صحيح)(uintptr_t)list[i] + string_شروع,
				o->داده + (صحيح)(uintptr_t)list[i] + string_شروع);
		i++;
	}
	إذا (n) {
		ثابت حرف *p = o->داده + (صحيح)(uintptr_t)list[n - 1] + string_شروع;
		indent();
#اگر_تعریف_نشده __U_BOOT__
		fdprintf(2, " total_sz:%ld\n", (دراز)((p + طول_رشته(p) + 1) - o->داده));
#وإلا /* __U_BOOT__ */
		چاپ_کن(" total_sz:%ld\n", (دراز)((p + طول_رشته(p) + 1) - o->داده));
#پایان_اگر /* __U_BOOT__ */
	}
}
#وإلا
# define debug_print_list(پیشوند, o, n) ((فراغ)0)
#پایان_اگر

/* n = o_save_ptr_helper(str, n) "starts new string" by storing an index value
 * in list[n] so that it points past last stored byte so far.
 * It returns n+1. */
ساكن صحيح o_save_ptr_helper(o_string *o, صحيح n)
{
	حرف **list = (حرف**)o->داده;
	صحيح string_شروع;
	صحيح string_len;

	إذا (!o->has_خالی_جایگاه) {
		string_شروع = ((n + 0xf) & ~0xf) * حجم(list[0]);
		string_len = o->درازی - string_شروع;
		إذا (!(n & 0xf)) { /* 0, 0x10, 0x20...? */
			debug_printf_list("list[%d]=%d string_start=%d (growing)\n", n, string_len, string_شروع);
			/* list[n] points to string_start, make space for 16 more pointers */
			o->maxlen += 0x10 * حجم(list[0]);
			o->داده = xrealloc(o->داده, o->maxlen + 1);
			list = (حرف**)o->داده;
			memmove(list + n + 0x10, list + n, string_len);
			/*
			 * expand_on_ifs() has a "previous argv[] ends in IFS?"
			 * check. (grep for -prev-ifs-check-).
			 * Ensure that argv[-1][last] is not garbage
			 * but zero bytes, to save index check there.
			 */
			list[n + 0x10 - 1] = 0;
			o->درازی += 0x10 * حجم(list[0]);
		} وإلا {
			debug_printf_list("list[%d]=%d string_start=%d\n",
					n, string_len, string_شروع);
		}
	} وإلا {
		/* We have empty slot at list[n], reuse without growth */
		string_شروع = ((n+1 + 0xf) & ~0xf) * حجم(list[0]); /* NB: n+1! */
		string_len = o->درازی - string_شروع;
		debug_printf_list("list[%d]=%d string_start=%d (empty slot)\n",
				n, string_len, string_شروع);
		o->has_خالی_جایگاه = 0;
	}
	o->has_quoted_part = 0;
	list[n] = (حرف*)(uintptr_t)string_len;
	أرجع n + 1;
}

/* "What was our last o_save_ptr'ed position (byte offset relative o->data)?" */
ساكن صحيح o_get_آخرین_ptr(o_string *o, صحيح n)
{
	حرف **list = (حرف**)o->داده;
	صحيح string_شروع = ((n + 0xf) & ~0xf) * حجم(list[0]);

	أرجع ((صحيح)(uintptr_t)list[n-1]) + string_شروع;
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

#إذا ENABLE_HUSH_BRACE_EXPANSION
/* There in a GNU extension, GLOB_BRACE, but it is not usable:
 * first, it processes even {a} (no commas), second,
 * I didn't manage to make it return strings when they don't match
 * existing files. Need to re-implement it.
 */

/* Helper */
ساكن صحيح glob_needed(ثابت حرف *s)
{
	طالما (*s) {
		إذا (*s == '\\') {
			إذا (!s[1])
				أرجع 0;
			s += 2;
			استمر;
		}
		إذا (*s == '*' || *s == '[' || *s == '?' || *s == '{')
			أرجع 1;
		s++;
	}
	أرجع 0;
}
/* Return pointer to next closing brace or to comma */
ساكن ثابت حرف *بعدی_brace_sub(ثابت حرف *cp)
{
	بیعلامت عمق = 0;
	cp++;
	طالما (*cp != '\0') {
		إذا (*cp == '\\') {
			إذا (*++cp == '\0')
				اكسر;
			cp++;
			استمر;
		}
		إذا ((*cp == '}' && عمق-- == 0) || (*cp == ',' && عمق == 0))
			اكسر;
		إذا (*cp++ == '{')
			عمق++;
	}

	أرجع *cp != '\0' ? cp : عدم;
}
/* Recursive brace globber. Note: may garble pattern[]. */
ساكن صحيح glob_brace(حرف *الگو, o_string *o, صحيح n)
{
	حرف *new_الگو_buf;
	ثابت حرف *begin;
	ثابت حرف *بعدی;
	ثابت حرف *rest;
	ثابت حرف *p;
	size_t rest_len;

	debug_printf_glob("glob_brace('%s')\n", الگو);

	begin = الگو;
	طالما (1) {
		إذا (*begin == '\0')
			اذهب simple_glob;
		إذا (*begin == '{') {
			/* Find the first sub-pattern and at the same time
			 * find the rest after the closing brace */
			بعدی = بعدی_brace_sub(begin);
			إذا (بعدی == عدم) {
				/* An illegal expression */
				اذهب simple_glob;
			}
			إذا (*بعدی == '}') {
				/* "{abc}" with no commas - illegal
				 * brace expr, disregard and skip it */
				begin = بعدی + 1;
				استمر;
			}
			اكسر;
		}
		إذا (*begin == '\\' && begin[1] != '\0')
			begin++;
		begin++;
	}
	debug_printf_glob("begin:%s\n", begin);
	debug_printf_glob("next:%s\n", بعدی);

	/* Now find the end of the whole brace expression */
	rest = بعدی;
	طالما (*rest != '}') {
		rest = بعدی_brace_sub(rest);
		إذا (rest == عدم) {
			/* An illegal expression */
			اذهب simple_glob;
		}
		debug_printf_glob("rest:%s\n", rest);
	}
	rest_len = طول_رشته(++rest) + 1;

	/* We are sure the brace expression is well-formed */

	/* Allocate working buffer large enough for our work */
	new_الگو_buf = xmalloc(طول_رشته(الگو));

	/* We have a brace expression.  BEGIN points to the opening {,
	 * NEXT points past the terminator of the first element, and REST
	 * points past the final }.  We will accumulate result names from
	 * recursive runs for each brace alternative in the buffer using
	 * GLOB_APPEND. */

	p = begin + 1;
	طالما (1) {
		/* Construct the new glob expression */
		کپی_حافظه(
			mempcpy(
				mempcpy(new_الگو_buf,
					/* We know the prefix for all sub-patterns */
					الگو, begin - الگو),
				p, بعدی - p),
			rest, rest_len);

		/* Note: glob_brace() may garble new_pattern_buf[].
		 * That's why we re-copy prefix every time (1st memcpy above).
		 */
		n = glob_brace(new_الگو_buf, o, n);
		إذا (*بعدی == '}') {
			/* We saw the last entry */
			اكسر;
		}
		p = بعدی + 1;
		بعدی = بعدی_brace_sub(بعدی);
	}
	حرر(new_الگو_buf);
	أرجع n;

 simple_glob:
	{
		صحيح gr;
		glob_t globdata;

		memset(&globdata, 0, حجم(globdata));
		gr = glob(الگو, 0, عدم, &globdata);
		debug_printf_glob("glob('%s'):%d\n", الگو, gr);
		إذا (gr != 0) {
			إذا (gr == GLOB_NOMATCH) {
				globfree(&globdata);
				/* NB: garbles parameter */
				unbackslash(الگو);
				o_addstr_with_NUL(o, الگو);
				debug_printf_glob("glob pattern '%s' is literal\n", الگو);
				أرجع o_save_ptr_helper(o, n);
			}
			إذا (gr == GLOB_NOSPACE)
				bb_die_حافظه_exhausted();
			/* GLOB_ABORTED? Only happens with GLOB_ERR flag,
			 * but we didn't specify it. Paranoia again. */
			bb_error_پیام_and_die("glob error %d on '%s'", gr, الگو);
		}
		إذا (globdata.gl_pathv && globdata.gl_pathv[0]) {
			حرف **argv = globdata.gl_pathv;
			طالما (1) {
				o_addstr_with_NUL(o, *argv);
				n = o_save_ptr_helper(o, n);
				argv++;
				إذا (!*argv)
					اكسر;
			}
		}
		globfree(&globdata);
	}
	أرجع n;
}
/* Performs globbing on last list[],
 * saving each result as a new list[].
 */
ساكن صحيح perform_glob(o_string *o, صحيح n)
{
	حرف *الگو, *copy;

	debug_printf_glob("start perform_glob: n:%d o->data:%p\n", n, o->داده);
	إذا (!o->داده)
		أرجع o_save_ptr_helper(o, n);
	الگو = o->داده + o_get_آخرین_ptr(o, n);
	debug_printf_glob("glob pattern '%s'\n", الگو);
	إذا (!glob_needed(الگو)) {
		/* unbackslash last string in o in place, fix length */
		o->درازی = unbackslash(الگو) - o->داده;
		debug_printf_glob("glob pattern '%s' is literal\n", الگو);
		أرجع o_save_ptr_helper(o, n);
	}

	copy = xstrdup(الگو);
	/* "forget" pattern in o */
	o->درازی = الگو - o->داده;
	n = glob_brace(copy, o, n);
	حرر(copy);
	إذا (DEBUG_GLOB)
		debug_print_list("perform_glob returning", o, n);
	أرجع n;
}

#وإلا /* !HUSH_BRACE_EXPANSION */

/* Helper */
ساكن صحيح glob_needed(ثابت حرف *s)
{
	طالما (*s) {
		إذا (*s == '\\') {
			إذا (!s[1])
				أرجع 0;
			s += 2;
			استمر;
		}
		إذا (*s == '*' || *s == '[' || *s == '?')
			أرجع 1;
		s++;
	}
	أرجع 0;
}
/* Performs globbing on last list[],
 * saving each result as a new list[].
 */
ساكن صحيح perform_glob(o_string *o, صحيح n)
{
#اگر_تعریف_نشده __U_BOOT__
	glob_t globdata;
	صحيح gr;
#پایان_اگر /* __U_BOOT__ */
	حرف *الگو;

	debug_printf_glob("start perform_glob: n:%d o->data:%p\n", n, o->داده);
	إذا (!o->داده)
		أرجع o_save_ptr_helper(o, n);
	الگو = o->داده + o_get_آخرین_ptr(o, n);
	debug_printf_glob("glob pattern '%s'\n", الگو);
	إذا (!glob_needed(الگو)) {
#اگر_تعریف_نشده __U_BOOT__
 literal:
#پایان_اگر /* __U_BOOT__ */
		/* unbackslash last string in o in place, fix length */
		o->درازی = unbackslash(الگو) - o->داده;
		debug_printf_glob("glob pattern '%s' is literal\n", الگو);
		أرجع o_save_ptr_helper(o, n);
	}

#اگر_تعریف_نشده __U_BOOT__
	memset(&globdata, 0, حجم(globdata));
	/* Can't use GLOB_NOCHECK: it does not unescape the string.
	 * If we glob "*.\*" and don't find anything, we need
	 * to fall back to using literal "*.*", but GLOB_NOCHECK
	 * will return "*.\*"!
	 */
	gr = glob(الگو, 0, عدم, &globdata);
	debug_printf_glob("glob('%s'):%d\n", الگو, gr);
	إذا (gr != 0) {
		إذا (gr == GLOB_NOMATCH) {
			globfree(&globdata);
			اذهب literal;
		}
		إذا (gr == GLOB_NOSPACE)
			bb_die_حافظه_exhausted();
		/* GLOB_ABORTED? Only happens with GLOB_ERR flag,
		 * but we didn't specify it. Paranoia again. */
		bb_error_پیام_and_die("glob error %d on '%s'", gr, الگو);
	}
	إذا (globdata.gl_pathv && globdata.gl_pathv[0]) {
		حرف **argv = globdata.gl_pathv;
		/* "forget" pattern in o */
		o->درازی = الگو - o->داده;
		طالما (1) {
			o_addstr_with_NUL(o, *argv);
			n = o_save_ptr_helper(o, n);
			argv++;
			إذا (!*argv)
				اكسر;
		}
	}
	globfree(&globdata);
	إذا (DEBUG_GLOB)
		debug_print_list("perform_glob returning", o, n);
	أرجع n;
#وإلا /* __U_BOOT__ */
	/*
	 * NOTE We only use perform glob to call unbackslash to remove backslash
	 * from string once expanded.
	 * So, it seems OK to return this if no previous return was done.
	 */
	أرجع o_save_ptr_helper(o, n);
#پایان_اگر /* __U_BOOT__ */
}

#پایان_اگر /* !HUSH_BRACE_EXPANSION */

/* If o->o_expflags & EXP_FLAG_GLOB, glob the string so far remembered.
 * Otherwise, just finish current list[] and start new */
ساكن صحيح o_save_ptr(o_string *o, صحيح n)
{
	إذا (o->o_expflags & EXP_FLAG_GLOB) {
		/* If o->has_empty_slot, list[n] was already globbed
		 * (if it was requested back then when it was filled)
		 * so don't do that again! */
		إذا (!o->has_خالی_جایگاه)
			أرجع perform_glob(o, n); /* o_save_ptr_helper is inside */
	}
	أرجع o_save_ptr_helper(o, n);
}

/* "Please convert list[n] to real char* ptrs, and NULL terminate it." */
ساكن حرف **o_finalize_list(o_string *o, صحيح n)
{
	حرف **list;
	صحيح string_شروع;

	إذا (DEBUG_EXPAND)
		debug_print_list("finalized", o, n);
	debug_printf_expand("finalized n:%d\n", n);
	list = (حرف**)o->داده;
	string_شروع = ((n + 0xf) & ~0xf) * حجم(list[0]);
	list[--n] = عدم;
	طالما (n) {
		n--;
		list[n] = o->داده + (صحيح)(uintptr_t)list[n] + string_شروع;
	}
	أرجع list;
}

ساكن فراغ free_pipe_list(بنية pipe *pi);

/* Returns pi->next - next pipe in the list */
ساكن بنية pipe *free_pipe(بنية pipe *pi)
{
	بنية pipe *بعدی;
	صحيح i;

#اگر_تعریف_نشده __U_BOOT__
	debug_printf_clean("free_pipe (pid %d)\n", getpid());
#پایان_اگر /* !__U_BOOT__ */
	لكل (i = 0; i < pi->رقم_cmds; i++) {
		بنية command *command;
#اگر_تعریف_نشده __U_BOOT__
		بنية redir_struct *r, *rnext;
#پایان_اگر /* !__U_BOOT__ */

		command = &pi->cmds[i];
		debug_printf_clean("  command %d:\n", i);
		إذا (command->argv) {
			إذا (DEBUG_CLEAN) {
				صحيح a;
				حرف **p;
				لكل (a = 0, p = command->argv; *p; a++, p++) {
					debug_printf_clean("   argv[%d] = %s\n", a, *p);
				}
			}
			free_رشتهها(command->argv);
			//command->argv = NULL;
		}
		/* not "else if": on syntax error, we may have both! */
		إذا (command->گروه) {
			debug_printf_clean("   begin group (cmd_type:%d)\n",
					command->cmd_type);
			free_pipe_list(command->گروه);
			debug_printf_clean("   end group\n");
			//command->group = NULL;
		}
		/* else is crucial here.
		 * If group != NULL, child_func is meaningless */
#إذا ENABLE_HUSH_FUNCTIONS
		وإلا إذا (command->فرزند_func) {
			debug_printf_exec("cmd %p releases child func at %p\n", command, command->فرزند_func);
			command->فرزند_func->والد_cmd = عدم;
		}
#پایان_اگر
#إذا !BB_MMU
		حرر(command->گروه_as_string);
		//command->group_as_string = NULL;
#پایان_اگر
#اگر_تعریف_نشده __U_BOOT__
		لكل (r = command->redirects; r; r = rnext) {
			debug_printf_clean("   redirect %d%s",
					r->rd_fd, redir_جدول[r->rd_type].descrip);
			/* guard against the case >$FOO, where foo is unset or blank */
			إذا (r->rd_filename) {
				debug_printf_clean(" fname:'%s'\n", r->rd_filename);
				حرر(r->rd_filename);
				//r->rd_filename = NULL;
			}
			debug_printf_clean(" rd_dup:%d\n", r->rd_dup);
			rnext = r->بعدی;
			حرر(r);
		}
		//command->redirects = NULL;
#پایان_اگر /* !__U_BOOT__ */
	}
	حرر(pi->cmds);   /* children are an array, they get freed all at once */
	//pi->cmds = NULL;
#اگر_تعریف_نشده __U_BOOT__
#إذا ENABLE_HUSH_کار
	حرر(pi->cmdtext);
	//pi->cmdtext = NULL;
#پایان_اگر
#پایان_اگر /* !__U_BOOT__ */

	بعدی = pi->بعدی;
	حرر(pi);
	أرجع بعدی;
}

ساكن فراغ free_pipe_list(بنية pipe *pi)
{
	طالما (pi) {
#إذا HAS_KEYWORDS
		debug_printf_clean("pipe reserved word %d\n", pi->res_کلمه);
#پایان_اگر
		debug_printf_clean("pipe followup code %d\n", pi->followup);
		pi = free_pipe(pi);
	}
}

/*** Parsing routines ***/

#اگر_تعریف_نشده debug_print_درخت
ساكن فراغ debug_print_درخت(بنية pipe *pi, صحيح lvl)
{
	ساكن ثابت حرف *ثابت PIPE[] ALIGN_PTR = {
		[PIPE_SEQ] = "SEQ",
		[PIPE_AND] = "AND",
		[PIPE_OR ] = "OR" ,
		[PIPE_BG ] = "BG" ,
	};
	ساكن ثابت حرف *RES[] = {
		[RES_NONE ] = "NONE" ,
# إذا ENABLE_HUSH_IF
		[RES_IF   ] = "IF"   ,
		[RES_THEN ] = "THEN" ,
		[RES_ELIF ] = "ELIF" ,
		[RES_ELSE ] = "ELSE" ,
		[RES_FI   ] = "FI"   ,
# endif
# إذا ENABLE_HUSH_LOOPS
		[RES_FOR  ] = "FOR"  ,
		[RES_WHILE] = "WHILE",
		[RES_UNTIL] = "UNTIL",
		[RES_DO   ] = "DO"   ,
		[RES_انجامشده ] = "DONE" ,
# endif
# إذا ENABLE_HUSH_LOOPS || ENABLE_HUSH_CASE
		[RES_درون   ] = "IN"   ,
# endif
# إذا ENABLE_HUSH_CASE
		[RES_CASE ] = "CASE" ,
		[RES_CASE_درون ] = "CASE_IN" ,
		[RES_تطابق] = "MATCH",
		[RES_CASE_BODY] = "CASE_BODY",
		[RES_ESAC ] = "ESAC" ,
# endif
		[RES_XXXX ] = "XXXX" ,
		[RES_SNTX ] = "SNTX" ,
	};
	ساكن ثابت حرف *ثابت CMDTYPE[] ALIGN_PTR = {
		"{}",
		"()",
		"[noglob]",
# إذا ENABLE_HUSH_FUNCTIONS
		"func()",
# endif
	};

	صحيح pin, prn;

	pin = 0;
	طالما (pi) {
		fdprintf(2, "%*spipe %d #cmds:%d %sres_word=%s followup=%d %s\n",
				lvl*2, "",
				pin,
				pi->رقم_cmds,
				(IF_HAS_KEYWORDS(pi->pi_inverted ? "! " :) ""),
				RES[pi->res_کلمه],
				pi->followup, PIPE[pi->followup]
		);
		prn = 0;
		طالما (prn < pi->رقم_cmds) {
			بنية command *command = &pi->cmds[prn];
			حرف **argv = command->argv;

			fdprintf(2, "%*s cmd %d assignment_cnt:%d",
					lvl*2, "", prn,
					command->assignment_cnt);
# إذا ENABLE_HUSH_LINENO_VAR
			fdprintf(2, " LINENO:%u", command->lineno);
# endif
			إذا (command->گروه) {
				fdprintf(2, " group %s: (argv=%p)%s%s\n",
						CMDTYPE[command->cmd_type],
						argv
# إذا !BB_MMU
						, " group_as_string:", command->گروه_as_string
# وإلا
						, "", ""
# endif
				);
				debug_print_درخت(command->گروه, lvl+1);
				prn++;
				استمر;
			}
			إذا (argv) طالما (*argv) {
				fdprintf(2, " '%s'", *argv);
				argv++;
			}
#اگر_تعریف_نشده __U_BOOT__
			إذا (command->redirects)
				fdprintf(2, " {redir}");
#پایان_اگر /* __U_BOOT__ */
			fdprintf(2, "\n");
			prn++;
		}
		pi = pi->بعدی;
		pin++;
	}
}
#پایان_اگر /* debug_print_tree */

ساكن بنية pipe *new_pipe(فراغ)
{
	بنية pipe *pi;
	pi = xzalloc(حجم(بنية pipe));
	/*pi->res_word = RES_NONE; - RES_NONE is 0 anyway */
	أرجع pi;
}

/* Command (member of a pipe) is complete, or we start a new pipe
 * if ctx->command is NULL.
 * No errors possible here.
 */
ساكن صحيح انجامشده_command(بنية parse_context *ctx)
{
	/* The command is really already in the pipe structure, so
	 * advance the pipe counter and make a new, null command. */
	بنية pipe *pi = ctx->pipe;
	بنية command *command = ctx->command;

#إذا 0	/* Instead we emit error message at run time */
	إذا (ctx->pending_redirect) {
		/* For example, "cmd >" (no filename to redirect to) */
		syntax_error("invalid redirect");
		ctx->pending_redirect = عدم;
	}
#پایان_اگر

	إذا (command) {
		إذا (IS_NULL_CMD(command)) {
			debug_printf_parse("done_command: skipping null cmd, num_cmds=%d\n", pi->رقم_cmds);
			اذهب clear_and_ret;
		}
		pi->رقم_cmds++;
		debug_printf_parse("done_command: ++num_cmds=%d\n", pi->رقم_cmds);
		//debug_print_tree(ctx->list_head, 20);
	} وإلا {
		debug_printf_parse("done_command: initializing, num_cmds=%d\n", pi->رقم_cmds);
	}

	/* Only real trickiness here is that the uncommitted
	 * command structure is not counted in pi->num_cmds. */
	pi->cmds = xrealloc(pi->cmds, حجم(*pi->cmds) * (pi->رقم_cmds+1));
	ctx->command = command = &pi->cmds[pi->رقم_cmds];
 clear_and_ret:
	memset(command, 0, حجم(*command));
#إذا ENABLE_HUSH_LINENO_VAR
	command->lineno = G.parse_lineno;
	debug_printf_parse("command->lineno = G.parse_lineno (%u)\n", G.parse_lineno);
#پایان_اگر
	أرجع pi->رقم_cmds; /* used only for 0/nonzero check */
}

ساكن فراغ انجامشده_pipe(بنية parse_context *ctx, pipe_style type)
{
	صحيح not_null;

	debug_printf_parse("done_pipe entered, followup %d\n", type);
	/* Close previous command */
	not_null = انجامشده_command(ctx);
#إذا HAS_KEYWORDS
	ctx->pipe->pi_inverted = ctx->ctx_inverted;
	ctx->ctx_inverted = 0;
	ctx->pipe->res_کلمه = ctx->ctx_res_w;
#پایان_اگر
	إذا (type == PIPE_BG && ctx->list_سر != ctx->pipe) {
		/* Necessary since && and || have precedence over &:
		 * "cmd1 && cmd2 &" must spawn both cmds, not only cmd2,
		 * in a backgrounded subshell.
		 */
		بنية pipe *pi;
		بنية command *command;

		/* Is this actually this construct, all pipes end with && or ||? */
		pi = ctx->list_سر;
		طالما (pi != ctx->pipe) {
			إذا (pi->followup != PIPE_AND && pi->followup != PIPE_OR)
				اذهب no_conv;
			pi = pi->بعدی;
		}

		debug_printf_parse("BG with more than one pipe, converting to { p1 &&...pN; } &\n");
		pi->followup = PIPE_SEQ; /* close pN _not_ with "&"! */
		pi = xzalloc(حجم(*pi));
		pi->followup = PIPE_BG;
		pi->رقم_cmds = 1;
		pi->cmds = xzalloc(حجم(pi->cmds[0]));
		command = &pi->cmds[0];
		إذا (CMD_نرمال != 0) /* "if xzalloc didn't do that already" */
			command->cmd_type = CMD_نرمال;
		command->گروه = ctx->list_سر;
#إذا !BB_MMU
		command->گروه_as_string = xstrndup(
			    ctx->as_string.داده,
			    ctx->as_string.درازی - 1 /* do not copy last char, "&" */
		);
#پایان_اگر
		/* Replace all pipes in ctx with one newly created */
		ctx->list_سر = ctx->pipe = pi;
		/* for cases like "cmd && &", do not be tricked by last command
		 * being null - the entire {...} & is NOT null! */
		not_null = 1;
	} وإلا {
 no_conv:
		ctx->pipe->followup = type;
	}

	/* Without this check, even just <enter> on command line generates
	 * tree of three NOPs (!). Which is harmless but annoying.
	 * IOW: it is safe to do it unconditionally. */
	إذا (not_null
#إذا ENABLE_HUSH_IF
	 || ctx->ctx_res_w == RES_FI
#پایان_اگر
#إذا ENABLE_HUSH_LOOPS
	 || ctx->ctx_res_w == RES_انجامشده
	 || ctx->ctx_res_w == RES_FOR
	 || ctx->ctx_res_w == RES_درون
#پایان_اگر
#إذا ENABLE_HUSH_CASE
	 || ctx->ctx_res_w == RES_ESAC
#پایان_اگر
	) {
		بنية pipe *new_p;
		debug_printf_parse("done_pipe: adding new pipe: "
#اگر_تعریف_نشده __U_BOOT__
				"not_null:%d ctx->ctx_res_w:%d\n",
				not_null, ctx->ctx_res_w);
#وإلا /* __U_BOOT__ */
				"not_null:%d\n",
				not_null);
#پایان_اگر /* __U_BOOT__ */
		new_p = new_pipe();
		ctx->pipe->بعدی = new_p;
		ctx->pipe = new_p;
		/* RES_THEN, RES_DO etc are "sticky" -
		 * they remain set for pipes inside if/while.
		 * This is used to control execution.
		 * RES_FOR and RES_IN are NOT sticky (needed to support
		 * cases where variable or value happens to match a keyword):
		 */
#إذا ENABLE_HUSH_LOOPS
		إذا (ctx->ctx_res_w == RES_FOR
		 || ctx->ctx_res_w == RES_درون)
			ctx->ctx_res_w = RES_NONE;
#پایان_اگر
#إذا ENABLE_HUSH_CASE
		إذا (ctx->ctx_res_w == RES_تطابق)
			ctx->ctx_res_w = RES_CASE_BODY;
		إذا (ctx->ctx_res_w == RES_CASE)
			ctx->ctx_res_w = RES_CASE_درون;
#پایان_اگر
		ctx->command = عدم; /* trick done_command below */
		/* Create the memory for command, roughly:
		 * ctx->pipe->cmds = new struct command;
		 * ctx->command = &ctx->pipe->cmds[0];
		 */
		انجامشده_command(ctx);
		//debug_print_tree(ctx->list_head, 10);
	}
	debug_printf_parse("done_pipe return\n");
}

ساكن فراغ initialize_context(بنية parse_context *ctx)
{
	memset(ctx, 0, حجم(*ctx));
	إذا (MAYBE_ASSIGNMENT != 0)
		ctx->is_assignment = MAYBE_ASSIGNMENT;
	ctx->pipe = ctx->list_سر = new_pipe();
	/* Create the memory for command, roughly:
	 * ctx->pipe->cmds = new struct command;
	 * ctx->command = &ctx->pipe->cmds[0];
	 */
	انجامشده_command(ctx);
}

/* If a reserved word is found and processed, parse context is modified
 * and 1 is returned.
 */
#إذا HAS_KEYWORDS
بنية reserved_combo {
	حرف literal[6];
	بیعلامت حرف res;
	بیعلامت حرف assignment_flag;
	uint32_t flag;
};
تعداد {
	FLAG_انتها   = (1 << RES_NONE ),
# إذا ENABLE_HUSH_IF
	FLAG_IF    = (1 << RES_IF   ),
	FLAG_THEN  = (1 << RES_THEN ),
	FLAG_ELIF  = (1 << RES_ELIF ),
	FLAG_ELSE  = (1 << RES_ELSE ),
	FLAG_FI    = (1 << RES_FI   ),
# endif
# إذا ENABLE_HUSH_LOOPS
	FLAG_FOR   = (1 << RES_FOR  ),
	FLAG_WHILE = (1 << RES_WHILE),
	FLAG_UNTIL = (1 << RES_UNTIL),
	FLAG_DO    = (1 << RES_DO   ),
	FLAG_انجامشده  = (1 << RES_انجامشده ),
	FLAG_درون    = (1 << RES_درون   ),
# endif
# إذا ENABLE_HUSH_CASE
	FLAG_تطابق = (1 << RES_تطابق),
	FLAG_ESAC  = (1 << RES_ESAC ),
# endif
	FLAG_شروع = (1 << RES_XXXX ),
};

ساكن ثابت بنية reserved_combo* تطابق_reserved_کلمه(o_string *کلمه)
{
	/* Mostly a list of accepted follow-up reserved words.
	 * FLAG_END means we are done with the sequence, and are ready
	 * to turn the compound list into a command.
	 * FLAG_START means the word must start a new compound list.
	 */
	ساكن ثابت بنية reserved_combo reserved_list[] ALIGN4 = {
# إذا ENABLE_HUSH_IF
		{ "!",     RES_NONE,  NOT_ASSIGNMENT  , 0 },
		{ "if",    RES_IF,    MAYBE_ASSIGNMENT, FLAG_THEN | FLAG_شروع },
		{ "then",  RES_THEN,  MAYBE_ASSIGNMENT, FLAG_ELIF | FLAG_ELSE | FLAG_FI },
		{ "elif",  RES_ELIF,  MAYBE_ASSIGNMENT, FLAG_THEN },
		{ "else",  RES_ELSE,  MAYBE_ASSIGNMENT, FLAG_FI   },
		{ "fi",    RES_FI,    NOT_ASSIGNMENT  , FLAG_انتها  },
# endif
# إذا ENABLE_HUSH_LOOPS
		{ "for",   RES_FOR,   NOT_ASSIGNMENT  , FLAG_درون | FLAG_DO | FLAG_شروع },
		{ "while", RES_WHILE, MAYBE_ASSIGNMENT, FLAG_DO | FLAG_شروع },
		{ "until", RES_UNTIL, MAYBE_ASSIGNMENT, FLAG_DO | FLAG_شروع },
		{ "in",    RES_درون,    NOT_ASSIGNMENT  , FLAG_DO   },
		{ "do",    RES_DO,    MAYBE_ASSIGNMENT, FLAG_انجامشده },
		{ "done",  RES_انجامشده,  NOT_ASSIGNMENT  , FLAG_انتها  },
# endif
# إذا ENABLE_HUSH_CASE
		{ "case",  RES_CASE,  NOT_ASSIGNMENT  , FLAG_تطابق | FLAG_شروع },
		{ "esac",  RES_ESAC,  NOT_ASSIGNMENT  , FLAG_انتها  },
# endif
	};
	ثابت بنية reserved_combo *r;

	لكل (r = reserved_list; r < reserved_list + ARRAY_اندازه(reserved_list); r++) {
		إذا (strcmp(کلمه->داده, r->literal) == 0)
			أرجع r;
	}
	أرجع عدم;
}
/* Return NULL: not a keyword, else: keyword
 */
ساكن ثابت بنية reserved_combo* reserved_کلمه(بنية parse_context *ctx)
{
# إذا ENABLE_HUSH_CASE
	ساكن ثابت بنية reserved_combo reserved_تطابق = {
		"",        RES_تطابق, NOT_ASSIGNMENT , FLAG_تطابق | FLAG_ESAC
	};
# endif
	ثابت بنية reserved_combo *r;

	إذا (ctx->کلمه.has_quoted_part)
		أرجع 0;
	r = تطابق_reserved_کلمه(&ctx->کلمه);
	إذا (!r)
		أرجع r; /* NULL */

	debug_printf("found reserved word %s, res %d\n", r->literal, r->res);
# إذا ENABLE_HUSH_CASE
	إذا (r->res == RES_درون && ctx->ctx_res_w == RES_CASE_درون) {
		/* "case word IN ..." - IN part starts first MATCH part */
		r = &reserved_تطابق;
	} وإلا
# endif
	إذا (r->flag == 0) { /* '!' */
		إذا (ctx->ctx_inverted) { /* bash doesn't accept '! ! true' */
			syntax_error("! ! command");
			ctx->ctx_res_w = RES_SNTX;
		}
		ctx->ctx_inverted = 1;
		أرجع r;
	}
	إذا (r->flag & FLAG_شروع) {
		بنية parse_context *old;

		old = xmemdup(ctx, حجم(*ctx));
		debug_printf_parse("push stack %p\n", old);
		initialize_context(ctx);
		ctx->پشته = old;
	} وإلا إذا (/*ctx->ctx_res_w == RES_NONE ||*/ !(ctx->old_flag & (1 << r->res))) {
		syntax_error_at(ctx->کلمه.داده);
		ctx->ctx_res_w = RES_SNTX;
		أرجع r;
	} وإلا {
		/* "{...} fi" is ok. "{...} if" is not
		 * Example:
		 * if { echo foo; } then { echo bar; } fi */
		إذا (ctx->command->گروه)
			انجامشده_pipe(ctx, PIPE_SEQ);
	}

	ctx->ctx_res_w = r->res;
	ctx->old_flag = r->flag;
	ctx->is_assignment = r->assignment_flag;
	debug_printf_parse("ctx->is_assignment='%s'\n", assignment_flag[ctx->is_assignment]);

	إذا (ctx->old_flag & FLAG_انتها) {
		بنية parse_context *old;

		انجامشده_pipe(ctx, PIPE_SEQ);
		debug_printf_parse("pop stack %p\n", ctx->پشته);
		old = ctx->پشته;
		old->command->گروه = ctx->list_سر;
		old->command->cmd_type = CMD_نرمال;
# إذا !BB_MMU
		/* At this point, the compound command's string is in
		 * ctx->as_string... except for the leading keyword!
		 * Consider this example: "echo a | if true; then echo a; fi"
		 * ctx->as_string will contain "true; then echo a; fi",
		 * with "if " remaining in old->as_string!
		 */
		{
			حرف *str;
			صحيح len = old->as_string.درازی;
			/* Concatenate halves */
			o_addstr(&old->as_string, ctx->as_string.داده);
			o_free(&ctx->as_string);
			/* Find where leading keyword starts in first half */
			str = old->as_string.داده + len;
			إذا (str > old->as_string.داده)
				str--; /* skip whitespace after keyword */
			طالما (str > old->as_string.داده && آیا_حرف(str[-1]))
				str--;
			/* Ugh, we're done with this horrid hack */
			old->command->گروه_as_string = xstrdup(str);
			debug_printf_parse("pop, remembering as:'%s'\n",
					old->command->گروه_as_string);
		}
# endif
		*ctx = *old;   /* physical copy */
		حرر(old);
	}
	أرجع r;
}
#پایان_اگر /* HAS_KEYWORDS */

/* Word is complete, look at it and update parsing context.
 * Normal return is 0. Syntax errors return 1.
 * Note: on return, word is reset, but not o_free'd!
 */
ساكن صحيح انجامشده_کلمه(بنية parse_context *ctx)
{
	بنية command *command = ctx->command;

	debug_printf_parse("done_word entered: '%s' %p\n", ctx->کلمه.داده, command);
	إذا (ctx->کلمه.درازی == 0 && !ctx->کلمه.has_quoted_part) {
		debug_printf_parse("done_word return 0: true null, ignored\n");
		أرجع 0;
	}

#اگر_تعریف_نشده __U_BOOT__
	إذا (ctx->pending_redirect) {
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

		ctx->pending_redirect->rd_filename = xstrdup(ctx->کلمه.داده);
		/* Cater for >\file case:
		 * >\a creates file a; >\\a, >"\a", >"\\a" create file \a
		 * Same with heredocs:
		 * for <<\H delim is H; <<\\H, <<"\H", <<"\\H" - \H
		 */
		إذا (ctx->pending_redirect->rd_type == REDIRECT_HEREDOC) {
			unbackslash(ctx->pending_redirect->rd_filename);
			/* Is it <<"HEREDOC"? */
			إذا (ctx->کلمه.has_quoted_part) {
				ctx->pending_redirect->rd_dup |= HEREDOC_QUOTED;
			}
		}
		debug_printf_parse("word stored in rd_filename: '%s'\n", ctx->کلمه.داده);
		ctx->pending_redirect = عدم;
	} وإلا {
#پایان_اگر /* !__U_BOOT__ */
#إذا HAS_KEYWORDS
# إذا ENABLE_HUSH_CASE
		إذا (ctx->ctx_dsemicolon
		 && strcmp(ctx->کلمه.داده, "esac") != 0 /* not "... pattern) cmd;; esac" */
		) {
			/* already done when ctx_dsemicolon was set to 1: */
			/* ctx->ctx_res_w = RES_MATCH; */
			ctx->ctx_dsemicolon = 0;
		} وإلا
# endif
# إذا defined(CMD_TEST2_SINGLEWORD_NOGLOB)
		إذا (command->cmd_type == CMD_TEST2_SINGLEWORD_NOGLOB
		 && strcmp(ctx->کلمه.داده, "]]") == 0
		) {
			/* allow "[[ ]] >file" etc */
			command->cmd_type = CMD_SINGLEWORD_NOGLOB;
		} وإلا
# endif
		إذا (!command->argv /* if it's the first word... */
# إذا ENABLE_HUSH_LOOPS
		 && ctx->ctx_res_w != RES_FOR /* ...not after FOR or IN */
		 && ctx->ctx_res_w != RES_درون
# endif
# إذا ENABLE_HUSH_CASE
		 && ctx->ctx_res_w != RES_CASE
# endif
		) {
			ثابت بنية reserved_combo *reserved;
			reserved = reserved_کلمه(ctx);
			debug_printf_parse("checking for reserved-ness: %d\n", !!reserved);
			إذا (reserved) {
# إذا ENABLE_HUSH_LINENO_VAR
/* Case:
 * "while ...; do
 *	cmd ..."
 * If we don't close the pipe _now_, immediately after "do", lineno logic
 * sees "cmd" as starting at "do" - i.e., at the previous line.
 */
				إذا (0
				 IF_HUSH_IF(|| reserved->res == RES_THEN)
				 IF_HUSH_IF(|| reserved->res == RES_ELIF)
				 IF_HUSH_IF(|| reserved->res == RES_ELSE)
				 IF_HUSH_LOOPS(|| reserved->res == RES_DO)
				) {
					انجامشده_pipe(ctx, PIPE_SEQ);
				}
# endif
				o_reset_مقصد_خالی_unquoted(&ctx->کلمه);
				debug_printf_parse("done_word return %d\n",
						(ctx->ctx_res_w == RES_SNTX));
				أرجع (ctx->ctx_res_w == RES_SNTX);
			}
# إذا defined(CMD_TEST2_SINGLEWORD_NOGLOB)
			إذا (strcmp(ctx->کلمه.داده, "[[") == 0) {
				command->cmd_type = CMD_TEST2_SINGLEWORD_NOGLOB;
			} وإلا
# endif
# إذا defined(CMD_SINGLEWORD_NOGLOB)
			إذا (0
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
			 IF_HUSH_LOCAL(   || strcmp(ctx->کلمه.داده, "local") == 0)
			 IF_HUSH_EXPORT(  || strcmp(ctx->کلمه.داده, "export") == 0)
			 IF_HUSH_READONLY(|| strcmp(ctx->کلمه.داده, "readonly") == 0)
			) {
				command->cmd_type = CMD_SINGLEWORD_NOGLOB;
			}
# وإلا
			{ /* empty block to pair "if ... else" */ }
# endif
		}
#پایان_اگر /* HAS_KEYWORDS */

		إذا (command->گروه) {
			/* "{ echo foo; } echo bar" - bad */
			syntax_error_at(ctx->کلمه.داده);
			debug_printf_parse("done_word return 1: syntax error, "
					"groups and arglists don't mix\n");
			أرجع 1;
		}

		/* If this word wasn't an assignment, next ones definitely
		 * can't be assignments. Even if they look like ones. */
		إذا (ctx->is_assignment != DEFINITELY_ASSIGNMENT
		 && ctx->is_assignment != کلمه_IS_KEYWORD
		) {
			ctx->is_assignment = NOT_ASSIGNMENT;
		} وإلا {
			إذا (ctx->is_assignment == DEFINITELY_ASSIGNMENT) {
				command->assignment_cnt++;
				debug_printf_parse("++assignment_cnt=%d\n", command->assignment_cnt);
			}
			debug_printf_parse("ctx->is_assignment was:'%s'\n", assignment_flag[ctx->is_assignment]);
			ctx->is_assignment = MAYBE_ASSIGNMENT;
		}
		debug_printf_parse("ctx->is_assignment='%s'\n", assignment_flag[ctx->is_assignment]);
		command->argv = جمع_کن_string_مقصد_رشتهها(command->argv, xstrdup(ctx->کلمه.داده));
#اگر_تعریف_شده __U_BOOT__
		command->argc++;
#پایان_اگر /* __U_BOOT__ */
		debug_print_رشتهها("word appended to argv", command->argv);

#اگر_تعریف_نشده __U_BOOT__
	}
#پایان_اگر /* !__U_BOOT__ */

#إذا ENABLE_HUSH_LOOPS
	إذا (ctx->ctx_res_w == RES_FOR) {
		إذا (ctx->کلمه.has_quoted_part
		 || endofname(command->argv[0])[0] != '\0'
		) {
			/* bash says just "not a valid identifier" */
			syntax_error("bad for loop variable");
			أرجع 1;
		}
		/* Force FOR to have just one word (variable name) */
		/* NB: basically, this makes hush see "for v in ..."
		 * syntax as if it is "for v; in ...". FOR and IN become
		 * two pipe structs in parse tree. */
		انجامشده_pipe(ctx, PIPE_SEQ);
	}
#پایان_اگر
#إذا ENABLE_HUSH_CASE
	/* Force CASE to have just one word */
	إذا (ctx->ctx_res_w == RES_CASE) {
		انجامشده_pipe(ctx, PIPE_SEQ);
	}
#پایان_اگر

	o_reset_مقصد_خالی_unquoted(&ctx->کلمه);

	debug_printf_parse("done_word return 0\n");
	أرجع 0;
}

#اگر_تعریف_نشده __U_BOOT__
/* Peek ahead in the input to find out if we have a "&n" construct,
 * as in "2>&1", that represents duplicating a file descriptor.
 * Return:
 * REDIRFD_CLOSE if >&- "close fd" construct is seen,
 * REDIRFD_SYNTAX_ERR if syntax error,
 * REDIRFD_TO_FILE if no & was seen,
 * or the number found.
 */
#إذا BB_MMU
#تعریف_کن parse_redir_راست_fd(as_string, درون_داد) \
	parse_redir_راست_fd(درون_داد)
#پایان_اگر
ساكن صحيح parse_redir_راست_fd(o_string *as_string, بنية درون_str *درون_داد)
{
	صحيح ch, d, ok;

	ch = i_peek(درون_داد);
	إذا (ch != '&')
		أرجع REDIRFD_مقصد_FILE;

	ch = i_getch(درون_داد);  /* get the & */
	nommu_addchr(as_string, ch);
	ch = i_peek(درون_داد);
	إذا (ch == '-') {
		ch = i_getch(درون_داد);
		nommu_addchr(as_string, ch);
		أرجع REDIRFD_CLOSE;
	}
	d = 0;
	ok = 0;
	طالما (ch != EOF && آیا_رقم(ch)) {
		d = d*10 + (ch-'0');
		ok = 1;
		ch = i_getch(درون_داد);
		nommu_addchr(as_string, ch);
		ch = i_peek(درون_داد);
	}
	إذا (ok) أرجع d;

//TODO: this is the place to catch ">&file" bashism (redirect both fd 1 and 2)

	bb_simple_error_پیام("ambiguous redirect");
	أرجع REDIRFD_SYNTAX_ERR;
}

/* Return code is 0 normal, 1 if a syntax error is detected
 */
ساكن صحيح parse_redirect(بنية parse_context *ctx,
		صحيح fd,
		redir_type style,
		بنية درون_str *درون_داد)
{
	بنية command *command = ctx->command;
	بنية redir_struct *redir;
	بنية redir_struct **redirp;
	صحيح dup_رقم;

	dup_رقم = REDIRFD_مقصد_FILE;
	إذا (style != REDIRECT_HEREDOC) {
		/* Check for a '>&1' type redirect */
		dup_رقم = parse_redir_راست_fd(&ctx->as_string, درون_داد);
		إذا (dup_رقم == REDIRFD_SYNTAX_ERR)
			أرجع 1;
	} وإلا {
		صحيح ch = i_peek_and_eat_bkslash_nl(درون_داد);
		dup_رقم = (ch == '-'); /* HEREDOC_SKIPTABS bit is 1 */
		إذا (dup_رقم) { /* <<-... */
			ch = i_getch(درون_داد);
			nommu_addchr(&ctx->as_string, ch);
			ch = i_peek(درون_داد);
		}
	}

	إذا (style == REDIRECT_OVERWRITE && dup_رقم == REDIRFD_مقصد_FILE) {
		صحيح ch = i_peek_and_eat_bkslash_nl(درون_داد);
		إذا (ch == '|') {
			/* >|FILE redirect ("clobbering" >).
			 * Since we do not support "set -o noclobber" yet,
			 * >| and > are the same for now. Just eat |.
			 */
			ch = i_getch(درون_داد);
			nommu_addchr(&ctx->as_string, ch);
		}
	}

	/* Create a new redir_struct and append it to the linked list */
	redirp = &command->redirects;
	طالما ((redir = *redirp) != عدم) {
		redirp = &(redir->بعدی);
	}
	*redirp = redir = xzalloc(حجم(*redir));
	/* redir->next = NULL; */
	/* redir->rd_filename = NULL; */
	redir->rd_type = style;
	redir->rd_fd = (fd == -1) ? redir_جدول[style].default_fd : fd;

	debug_printf_parse("redirect type %d %s\n", redir->rd_fd,
				redir_جدول[style].descrip);

	redir->rd_dup = dup_رقم;
	إذا (style != REDIRECT_HEREDOC && dup_رقم != REDIRFD_مقصد_FILE) {
		/* Erik had a check here that the file descriptor in question
		 * is legit; I postpone that to "run time"
		 * A "-" representation of "close me" shows up as a -3 here */
		debug_printf_parse("duplicating redirect '%d>&%d'\n",
				redir->rd_fd, redir->rd_dup);
	} وإلا {
#إذا 0		/* Instead we emit error message at run time */
		إذا (ctx->pending_redirect) {
			/* For example, "cmd > <file" */
			syntax_error("invalid redirect");
		}
#پایان_اگر
		/* Set ctx->pending_redirect, so we know what to do at the
		 * end of the next parsed word. */
		ctx->pending_redirect = redir;
	}
	أرجع 0;
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
ساكن صحيح redirect_opt_رقم(o_string *o)
{
	صحيح رقم;

	إذا (o->داده == عدم)
		أرجع -1;
	رقم = bb_strtou(o->داده, عدم, 10);
	إذا (errno || رقم < 0)
		أرجع -1;
	o_reset_مقصد_خالی_unquoted(o);
	أرجع رقم;
}

#إذا BB_MMU
#تعریف_کن fetch_till_str(as_string, درون_داد, کلمه, skip_tabs) \
	fetch_till_str(درون_داد, کلمه, skip_tabs)
#پایان_اگر
ساكن حرف *fetch_till_str(o_string *as_string,
		بنية درون_str *درون_داد,
		ثابت حرف *کلمه,
		صحيح heredoc_flags)
{
	o_string heredoc = NULL_O_STRING;
	بیعلامت past_EOL;
	صحيح prev = 0; /* not \ */
	صحيح ch;

	/* Starting with "" is necessary for this case:
	 * cat <<EOF
	 *
	 * xxx
	 * EOF
	 */
	heredoc.داده = xzalloc(1); /* start as "", not as NULL */

	اذهب jump_درون;

	طالما (1) {
		ch = i_getch(درون_داد);
		إذا (ch != EOF)
			nommu_addchr(as_string, ch);
		إذا (ch == '\n' || ch == EOF) {
 check_heredoc_انتها:
#اگر_تعریف_نشده __U_BOOT__
			إذا ((heredoc_flags & HEREDOC_QUOTED) || prev != '\\') {
#وإلا /* __U_BOOT__ */
			إذا (prev != '\\') {
#پایان_اگر
				/* End-of-line, and not a line continuation */
				إذا (strcmp(heredoc.داده + past_EOL, کلمه) == 0) {
					heredoc.داده[past_EOL] = '\0';
					debug_printf_heredoc("parsed '%s' heredoc '%s'\n", کلمه, heredoc.داده);
					أرجع heredoc.داده;
				}
				إذا (ch == '\n') {
					/* This is a new line.
					 * Remember position and backslash-escaping status.
					 */
					o_addchr(&heredoc, ch);
					prev = ch;
 jump_درون:
					past_EOL = heredoc.درازی;
					/* Get 1st char of next line, possibly skipping leading tabs */
					افعل {
						ch = i_getch(درون_داد);
						إذا (ch != EOF)
							nommu_addchr(as_string, ch);
#اگر_تعریف_نشده __U_BOOT__
					} طالما ((heredoc_flags & HEREDOC_SKIPTABS) && ch == '\t');
#وإلا /* __U_BOOT__ */
				} طالما (ch == '\t');
#پایان_اگر
					/* If this immediately ended the line,
					 * go back to end-of-line checks.
					 */
					إذا (ch == '\n')
						اذهب check_heredoc_انتها;
				}
			} وإلا {
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
				heredoc.داده[--heredoc.درازی] = '\0';
				prev = 0; /* not '\' */
				استمر;
			}
		}
		إذا (ch == EOF) {
			o_free(&heredoc);
			أرجع عدم; /* error */
		}
		o_addchr(&heredoc, ch);
		nommu_addchr(as_string, ch);
		إذا (prev == '\\' && ch == '\\')
			/* Correctly handle foo\\<eol> (not a line cont.) */
			prev = 0; /* not '\' */
		وإلا
			prev = ch;
	}
}
#پایان_اگر /* !__U_BOOT__ */

/* Look at entire parse tree for not-yet-loaded REDIRECT_HEREDOCs
 * and load them all. There should be exactly heredoc_cnt of them.
 */
#إذا BB_MMU
#تعریف_کن fetch_heredocs(as_string, pi, heredoc_cnt, درون_داد) \
	fetch_heredocs(pi, heredoc_cnt, درون_داد)
#پایان_اگر
ساكن صحيح fetch_heredocs(o_string *as_string, بنية pipe *pi, صحيح heredoc_cnt, بنية درون_str *درون_داد)
{
	طالما (pi && heredoc_cnt) {
		صحيح i;
		بنية command *cmd = pi->cmds;

		debug_printf_heredoc("fetch_heredocs: num_cmds:%d cmd argv0:'%s'\n",
				pi->رقم_cmds,
				cmd->argv ? cmd->argv[0] : "NONE"
		);
		لكل (i = 0; i < pi->رقم_cmds; i++) {
#اگر_تعریف_نشده __U_BOOT__
			بنية redir_struct *redir = cmd->redirects;

#پایان_اگر /* !__U_BOOT__ */
			debug_printf_heredoc("fetch_heredocs: %d cmd argv0:'%s'\n",
					i, cmd->argv ? cmd->argv[0] : "NONE");
#اگر_تعریف_نشده __U_BOOT__
			طالما (redir) {
				إذا (redir->rd_type == REDIRECT_HEREDOC) {
					حرف *p;

					redir->rd_type = REDIRECT_HEREDOC2;
					/* redir->rd_dup is (ab)used to indicate <<- */
					p = fetch_till_str(as_string, درون_داد,
							redir->rd_filename, redir->rd_dup);
					إذا (!p) {
						syntax_error("unexpected EOF in here document");
						أرجع -1;
					}
					حرر(redir->rd_filename);
					redir->rd_filename = p;
					heredoc_cnt--;
				}
				redir = redir->بعدی;
			}
#پایان_اگر /* !__U_BOOT__ */
			إذا (cmd->گروه) {
				//bb_error_msg("%s:%u heredoc_cnt:%d", __func__, __LINE__, heredoc_cnt);
				heredoc_cnt = fetch_heredocs(as_string, cmd->گروه, heredoc_cnt, درون_داد);
				//bb_error_msg("%s:%u heredoc_cnt:%d", __func__, __LINE__, heredoc_cnt);
				إذا (heredoc_cnt < 0)
					أرجع heredoc_cnt; /* error */
			}
			cmd++;
		}
		pi = pi->بعدی;
	}
	أرجع heredoc_cnt;
}

ساكن صحيح پیاپی_list(بنية pipe *pi);
#إذا BB_MMU
#تعریف_کن parse_جریان(pstring, heredoc_cnt_ptr, درون_داد, انتها_trigger) \
	parse_جریان(heredoc_cnt_ptr, درون_داد, انتها_trigger)
#پایان_اگر
ساكن بنية pipe *parse_جریان(حرف **pstring,
		صحيح *heredoc_cnt_ptr,
		بنية درون_str *درون_داد,
		صحيح انتها_trigger);

/* Returns number of heredocs not yet consumed,
 * or -1 on error.
 */
ساكن صحيح parse_گروه(بنية parse_context *ctx,
		بنية درون_str *درون_داد, صحيح ch)
{
	/* ctx->word contains characters seen prior to ( or {.
	 * Typically it's empty, but for function defs,
	 * it contains function name (without '()'). */
#إذا BB_MMU
# define as_string عدم
#وإلا
	حرف *as_string = عدم;
#پایان_اگر
	بنية pipe *pipe_list;
	صحيح heredoc_cnt = 0;
	صحيح endch;
	بنية command *command = ctx->command;

	debug_printf_parse("parse_group entered\n");
#إذا ENABLE_HUSH_FUNCTIONS
	إذا (ch == '(' && !ctx->کلمه.has_quoted_part) {
		إذا (ctx->کلمه.درازی)
			إذا (انجامشده_کلمه(ctx))
				أرجع -1;
		إذا (!command->argv)
			اذهب skip; /* (... */
		إذا (command->argv[1]) { /* word word ... (... */
			syntax_error_unexpected_ch('(');
			أرجع -1;
		}
		/* it is "word(..." or "word (..." */
		افعل
			ch = i_getch(درون_داد);
		طالما (ch == ' ' || ch == '\t');
		إذا (ch != ')') {
			syntax_error_unexpected_ch(ch);
			أرجع -1;
		}
		nommu_addchr(&ctx->as_string, ch);
		افعل
			ch = i_getch(درون_داد);
		طالما (ch == ' ' || ch == '\t' || ch == '\n');
		إذا (ch != '{' && ch != '(') {
			syntax_error_unexpected_ch(ch);
			أرجع -1;
		}
//bash allows functions named "123", "..", "return"!
//		if (endofname(command->argv[0])[0] != '\0') {
//			syntax_error("bad function name");
//			return -1;
//		}
		nommu_addchr(&ctx->as_string, ch);
		command->cmd_type = CMD_FUNCDEF;
		اذهب skip;
	}
#پایان_اگر

#إذا 0 /* Prevented by caller */
	إذا (command->argv /* word [word]{... */
	 || ctx->کلمه.درازی /* word{... */
	 || ctx->کلمه.has_quoted_part /* ""{... */
	) {
		syntax_error(عدم);
		debug_printf_parse("parse_group return -1: "
			"syntax error, groups and arglists don't mix\n");
		أرجع -1;
	}
#پایان_اگر

#اگر_تعریف_نشده __U_BOOT__
 IF_HUSH_FUNCTIONS(skip:)
#پایان_اگر /* !__U_BOOT__ */

	endch = '}';
	إذا (ch == '(') {
		endch = ')';
#اگر_تعریف_نشده __U_BOOT__
		IF_HUSH_FUNCTIONS(إذا (command->cmd_type != CMD_FUNCDEF))
			command->cmd_type = CMD_SUBSHELL;
#پایان_اگر /* !__U_BOOT__ */
	} وإلا {
		/* bash does not allow "{echo...", requires whitespace */
		ch = i_peek(درون_داد);
		إذا (ch != ' ' && ch != '\t' && ch != '\n'
		 && ch != '('	/* but "{(..." is allowed (without whitespace) */
		) {
			syntax_error_unexpected_ch(ch);
			أرجع -1;
		}
		إذا (ch != '(') {
			ch = i_getch(درون_داد);
			nommu_addchr(&ctx->as_string, ch);
		}
	}

	debug_printf_heredoc("calling parse_stream, heredoc_cnt:%d\n", heredoc_cnt);
	pipe_list = parse_جریان(&as_string, &heredoc_cnt, درون_داد, endch);
	debug_printf_heredoc("parse_stream returned: heredoc_cnt:%d\n", heredoc_cnt);
#إذا !BB_MMU
	إذا (as_string)
		o_addstr(&ctx->as_string, as_string);
#پایان_اگر

	/* empty ()/{} or parse error? */
	إذا (!pipe_list || pipe_list == ERR_PTR) {
		/* parse_stream already emitted error msg */
		إذا (!BB_MMU)
			حرر(as_string);
		debug_printf_parse("parse_group return -1: "
			"parse_stream returned %p\n", pipe_list);
		أرجع -1;
	}
#إذا !BB_MMU
	as_string[طول_رشته(as_string) - 1] = '\0'; /* plink ')' or '}' */
	command->گروه_as_string = as_string;
	debug_printf_parse("end of group, remembering as:'%s'\n",
			command->گروه_as_string);
#پایان_اگر

#إذا ENABLE_HUSH_FUNCTIONS
	/* Convert "f() (cmds)" to "f() {(cmds)}" */
	إذا (command->cmd_type == CMD_FUNCDEF && endch == ')') {
		بنية command *cmd2;

		cmd2 = xzalloc(حجم(*cmd2));
		cmd2->cmd_type = CMD_SUBSHELL;
		cmd2->گروه = pipe_list;
# إذا !BB_MMU
//UNTESTED!
		cmd2->گروه_as_string = command->گروه_as_string;
		command->گروه_as_string = xasprintf("(%s)", command->گروه_as_string);
# endif

		pipe_list = new_pipe();
		pipe_list->cmds = cmd2;
		pipe_list->رقم_cmds = 1;
	}
#پایان_اگر

	command->گروه = pipe_list;

	debug_printf_parse("parse_group return %d\n", heredoc_cnt);
	أرجع heredoc_cnt;
	/* command remains "open", available for possible redirects */
#لغو_تعریف as_string
}

#اگر_تعریف_نشده __U_BOOT__
#إذا ENABLE_HUSH_TICK || ENABLE_FEATURE_SH_MATH || ENABLE_HUSH_DOLLAR_OPS
/* Subroutines for copying $(...) and `...` things */
/* '...' */
ساكن صحيح جمع_کن_till_single_quote(o_string *dest, بنية درون_str *درون_داد)
{
	طالما (1) {
		صحيح ch = i_getch(درون_داد);
		إذا (ch == EOF) {
			syntax_error_unterm_ch('\'');
			أرجع 0;
		}
		إذا (ch == '\'')
			أرجع 1;
		o_addchr(dest, ch);
	}
}
ساكن صحيح جمع_کن_till_single_quote_dquoted(o_string *dest, بنية درون_str *درون_داد)
{
	طالما (1) {
		صحيح ch = i_getch(درون_داد);
		إذا (ch == EOF) {
			syntax_error_unterm_ch('\'');
			أرجع 0;
		}
		إذا (ch == '\'')
			أرجع 1;
		o_addqchr(dest, ch);
	}
}

/* "...\"...`..`...." - do we need to handle "...$(..)..." too? */
ساكن صحيح جمع_کن_till_backquote(o_string *dest, بنية درون_str *درون_داد, صحيح درون_dquote);
ساكن صحيح جمع_کن_till_double_quote(o_string *dest, بنية درون_str *درون_داد)
{
	طالما (1) {
		صحيح ch = i_getch(درون_داد);
		إذا (ch == EOF) {
			syntax_error_unterm_ch('"');
			أرجع 0;
		}
		إذا (ch == '"')
			أرجع 1;
		إذا (ch == '\\') {  /* \x. Copy both chars. */
			o_addchr(dest, ch);
			ch = i_getch(درون_داد);
		}
		o_addchr(dest, ch);
		إذا (ch == '`') {
			إذا (!جمع_کن_till_backquote(dest, درون_داد, /*in_dquote:*/ 1))
				أرجع 0;
			o_addchr(dest, ch);
			استمر;
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
ساكن صحيح جمع_کن_till_backquote(o_string *dest, بنية درون_str *درون_داد, صحيح درون_dquote)
{
	طالما (1) {
		صحيح ch = i_getch(درون_داد);
		إذا (ch == '`')
			أرجع 1;
		إذا (ch == '\\') {
			/* \x. Copy both unless it is \`, \$, \\ and maybe \" */
			ch = i_getch(درون_داد);
			إذا (ch != '`'
			 && ch != '$'
			 && ch != '\\'
			 && (!درون_dquote || ch != '"')
			) {
				o_addchr(dest, '\\');
			}
		}
		إذا (ch == EOF) {
			syntax_error_unterm_ch('`');
			أرجع 0;
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
#تعریف_کن DOUBLE_CLOSE_CHAR_FLAG 0x80
ساكن صحيح جمع_کن_till_closing_bracket(o_string *dest, بنية درون_str *درون_داد, بیعلامت انتها_ch)
{
	صحيح ch;
	حرف dbl = انتها_ch & DOUBLE_CLOSE_CHAR_FLAG;
# إذا BASH_SUBSTR || BASH_الگو_SUBST
	حرف انتها_char2 = انتها_ch >> 8;
# endif
	انتها_ch &= (DOUBLE_CLOSE_CHAR_FLAG - 1);

# إذا ENABLE_HUSH_INTERACTIVE
	G.promptmode = 1; /* PS2 */
# endif
	debug_printf_prompt("%s promptmode=%d\n", __func__, G.promptmode);

	طالما (1) {
		ch = i_getch(درون_داد);
		إذا (ch == EOF) {
			syntax_error_unterm_ch(انتها_ch);
			أرجع 0;
		}
		إذا (ch == انتها_ch
# إذا BASH_SUBSTR || BASH_الگو_SUBST
		 || ch == انتها_char2
# endif
		) {
			إذا (!dbl)
				اكسر;
			/* we look for closing )) of $((EXPR)) */
			إذا (i_peek_and_eat_bkslash_nl(درون_داد) == انتها_ch) {
				i_getch(درون_داد); /* eat second ')' */
				اكسر;
			}
		}
		o_addchr(dest, ch);
		//bb_error_msg("%s:o_addchr('%c')", __func__, ch);
		إذا (ch == '(' || ch == '{') {
			ch = (ch == '(' ? ')' : '}');
			إذا (!جمع_کن_till_closing_bracket(dest, درون_داد, ch))
				أرجع 0;
			o_addchr(dest, ch);
			استمر;
		}
		إذا (ch == '\'') {
			إذا (!جمع_کن_till_single_quote(dest, درون_داد))
				أرجع 0;
			o_addchr(dest, ch);
			استمر;
		}
		إذا (ch == '"') {
			إذا (!جمع_کن_till_double_quote(dest, درون_داد))
				أرجع 0;
			o_addchr(dest, ch);
			استمر;
		}
		إذا (ch == '`') {
			إذا (!جمع_کن_till_backquote(dest, درون_داد, /*in_dquote:*/ 0))
				أرجع 0;
			o_addchr(dest, ch);
			استمر;
		}
		إذا (ch == '\\') {
			/* \x. Copy verbatim. Important for  \(, \) */
			ch = i_getch(درون_داد);
			إذا (ch == EOF) {
				syntax_error_unterm_ch(انتها_ch);
				أرجع 0;
			}
# إذا 0
			إذا (ch == '\n') {
				/* "backslash+newline", ignore both */
				o_delchr(dest); /* undo insertion of '\' */
				استمر;
			}
# endif
			o_addchr(dest, ch);
			//bb_error_msg("%s:o_addchr('%c') after '\\'", __func__, ch);
			استمر;
		}
	}
	debug_printf_parse("%s return '%s' ch:'%c'\n", __func__, dest->داده, ch);
	أرجع ch;
}
#پایان_اگر /* ENABLE_HUSH_TICK || ENABLE_FEATURE_SH_MATH || ENABLE_HUSH_DOLLAR_OPS */

#إذا BASH_DOLLAR_SQUOTE
/* Return code: 1 for "found and parsed", 0 for "seen something else" */
# إذا BB_MMU
#تعریف_کن parse_dollar_squote(as_string, dest, درون_داد) \
	parse_dollar_squote(dest, درون_داد)
#تعریف_کن as_string عدم
# endif
ساكن صحيح parse_dollar_squote(o_string *as_string, o_string *dest, بنية درون_str *درون_داد)
{
	صحيح شروع;
	صحيح ch = i_peek_and_eat_bkslash_nl(درون_داد);  /* first character after the $ */
	debug_printf_parse("parse_dollar_squote entered: ch='%c'\n", ch);
	إذا (ch != '\'')
		أرجع 0;

	dest->has_quoted_part = 1;
	شروع = dest->درازی;

	ch = i_getch(درون_داد); /* eat ' */
	nommu_addchr(as_string, ch);
	طالما (1) {
		ch = i_getch(درون_داد);
		nommu_addchr(as_string, ch);
		إذا (ch == EOF) {
			syntax_error_unterm_ch('\'');
			أرجع 0;
		}
		إذا (ch == '\'')
			اكسر;
		إذا (ch == SPECIAL_VAR_نماد) {
			/* Convert raw ^C to corresponding special variable reference */
			o_addchr(dest, SPECIAL_VAR_نماد);
			o_addchr(dest, SPECIAL_VAR_QUOTED_SVS);
			/* will addchr() another SPECIAL_VAR_SYMBOL (see after the if() block) */
		} وإلا إذا (ch == '\\') {
			ساكن ثابت حرف C_escapes[] ALIGN1 = "nrbtfav""x\\01234567";

			ch = i_getch(درون_داد);
			nommu_addchr(as_string, ch);
			إذا (strchr(C_escapes, ch)) {
				حرف buf[4];
				حرف *p = buf;
				صحيح cnt = 2;

				buf[0] = ch;
				إذا ((بیعلامت حرف)(ch - '0') <= 7) { /* \ooo */
					افعل {
						ch = i_peek(درون_داد);
						إذا ((بیعلامت حرف)(ch - '0') > 7)
							اكسر;
						*++p = ch = i_getch(درون_داد);
						nommu_addchr(as_string, ch);
					} طالما (--cnt != 0);
				} وإلا إذا (ch == 'x') { /* \xHH */
					افعل {
						ch = i_peek(درون_داد);
						إذا (!isxdigit(ch))
							اكسر;
						*++p = ch = i_getch(درون_داد);
						nommu_addchr(as_string, ch);
					} طالما (--cnt != 0);
					إذا (cnt == 2) { /* \x but next char is "bad" */
						ch = 'x';
						اذهب unrecognized;
					}
				} /* else simple seq like \\ or \t */
				*++p = '\0';
				p = buf;
				ch = bb_process_escape_sequence((فراغ*)&p);
				//bb_error_msg("buf:'%s' ch:%x", buf, ch);
				إذا (ch == '\0')
					استمر; /* bash compat: $'...\0...' emits nothing */
			} وإلا { /* unrecognized "\z": encode both chars unless ' or " */
				إذا (ch != '\'' && ch != '"') {
 unrecognized:
					o_addqchr(dest, '\\');
				}
			}
		} /* if (\...) */
		o_addqchr(dest, ch);
	}

	إذا (dest->درازی == شروع) {
		/* $'', $'\0', $'\000\x00' and the like */
		o_addchr(dest, SPECIAL_VAR_نماد);
		o_addchr(dest, SPECIAL_VAR_نماد);
	}

	أرجع 1;
# undef as_string
}
#وإلا
# define parse_dollar_squote(as_string, dest, درون_داد) 0
#پایان_اگر /* BASH_DOLLAR_SQUOTE */
#پایان_اگر /* !__U_BOOT__ */

/* Return code: 0 for OK, 1 for syntax error */
#إذا BB_MMU
#تعریف_کن parse_dollar(as_string, dest, درون_داد, quote_ماسک) \
	parse_dollar(dest, درون_داد, quote_ماسک)
#تعریف_کن as_string عدم
#پایان_اگر
ساكن صحيح parse_dollar(o_string *as_string,
		o_string *dest,
		بنية درون_str *درون_داد, بیعلامت حرف quote_ماسک)
{
	صحيح ch = i_peek_and_eat_bkslash_nl(درون_داد);  /* first character after the $ */

	debug_printf_parse("parse_dollar entered: ch='%c' quote_mask:0x%x\n", ch, quote_ماسک);
	إذا (آیا_حرف(ch)) {
 make_var:
		ch = i_getch(درون_داد);
		nommu_addchr(as_string, ch);
 /*make_var1:*/
		o_addchr(dest, SPECIAL_VAR_نماد);
		طالما (1) {
			debug_printf_parse(": '%c'\n", ch);
			o_addchr(dest, ch | quote_ماسک);
			quote_ماسک = 0;
			ch = i_peek_and_eat_bkslash_nl(درون_داد);
			إذا (!آیا_حرف_یا_رقم(ch) && ch != '_') {
				/* End of variable name reached */
				اكسر;
			}
			ch = i_getch(درون_داد);
			nommu_addchr(as_string, ch);
		}
		o_addchr(dest, SPECIAL_VAR_نماد);
	} وإلا إذا (آیا_رقم(ch)) {
 make_one_char_var:
		ch = i_getch(درون_داد);
		nommu_addchr(as_string, ch);
		o_addchr(dest, SPECIAL_VAR_نماد);
		debug_printf_parse(": '%c'\n", ch);
		o_addchr(dest, ch | quote_ماسک);
		o_addchr(dest, SPECIAL_VAR_نماد);
	} وإلا بدل (ch) {
#اگر_تعریف_نشده __U_BOOT__
	حالة '$': /* pid */
	حالة '!': /* last bg pid */
#پایان_اگر /* !__U_BOOT__ */
	حالة '?': /* last exit code */
	حالة '#': /* number of args */
	حالة '*': /*آرگومانها*/
	حالة '@': /*آرگومانها*/
	حالة '-': /* $- option flags set by set builtin or shell options (-i etc) */
		اذهب make_one_char_var;
	حالة '{': {
		حرف len_single_ch;

		o_addchr(dest, SPECIAL_VAR_نماد);

		ch = i_getch(درون_داد); /* eat '{' */
		nommu_addchr(as_string, ch);

		ch = i_getch_and_eat_bkslash_nl(درون_داد); /* first char after '{' */
		/* It should be ${?}, or ${#var},
		 * or even ${?+subst} - operator acting on a special variable,
		 * or the beginning of variable name.
		 */
		إذا (ch == EOF
		 || (!strchr(_SPECIAL_VARS_STR, ch) && !آیا_حرف_یا_رقم(ch)) /* not one of those */
		) {
 bad_dollar_syntax:
			syntax_error_unterm_str("${name}");
			debug_printf_parse("parse_dollar return 0: unterminated ${name}\n");
			أرجع 0;
		}
		nommu_addchr(as_string, ch);
		len_single_ch = ch;
		ch |= quote_ماسک;

		/* It's possible to just call add_till_closing_bracket() at this point.
		 * However, this regresses some of our testsuite cases
		 * which check invalid constructs like ${%}.
		 * Oh well... let's check that the var name part is fine... */

		إذا (آیا_رقم(len_single_ch)
		 || (len_single_ch == '#' && آیا_رقم(i_peek_and_eat_bkslash_nl(درون_داد)))
		) {
			/* Execution engine uses plain xatoi_positive()
			 * to interpret ${NNN} and {#NNN},
			 * check syntax here in the parser.
			 * (bash does not support expressions in ${#NN},
			 * e.g. ${#$var} and {#1:+WORD} are not supported).
			 */
			بیعلامت cnt = 9; /* max 9 digits for ${NN} and 8 for {#NN} */
			طالما (1) {
				o_addchr(dest, ch);
				debug_printf_parse(": '%c'\n", ch);
				ch = i_getch_and_eat_bkslash_nl(درون_داد);
				nommu_addchr(as_string, ch);
				إذا (ch == '}')
					اكسر;
				إذا (--cnt == 0)
					اذهب bad_dollar_syntax;
				إذا (len_single_ch != '#' && strchr(VAR_SUBST_OPS, ch))
					/* ${NN<op>...} is valid */
					اذهب eat_until_closing;
				إذا (!آیا_رقم(ch))
					اذهب bad_dollar_syntax;
			}
		} وإلا
		طالما (1) {
			بیعلامت pos;

			o_addchr(dest, ch);
			debug_printf_parse(": '%c'\n", ch);

			ch = i_getch(درون_داد);
			nommu_addchr(as_string, ch);
			إذا (ch == '}')
				اكسر;
#اگر_تعریف_نشده __U_BOOT__
			إذا (!آیا_حرف_یا_رقم(ch) && ch != '_') {
#وإلا /* __U_BOOT__ */
			/*
			 * In several places in U-Boot, we use variable like
			 * foo# (e.g. serial#), particularly in env.
			 * So, we need to authorize # to appear inside
			 * variable name and then expand this variable.
			 * NOTE Having # in variable name is not permitted in
			 * upstream hush but expansion will be done (even though
			 * the result will be empty).
			 */
			إذا (!آیا_حرف_یا_رقم(ch) && ch != '_' && ch != '#') {
#پایان_اگر /* __U_BOOT__ */
				بیعلامت انتها_ch;
#اگر_تعریف_نشده __U_BOOT__
				بیعلامت حرف آخرین_ch;
#پایان_اگر /* !__U_BOOT__ */
				/* handle parameter expansions
				 * http://www.opengroup.org/onlinepubs/009695399/utilities/xcu_chap02.html#tag_02_06_02
				 */
				إذا (!strchr(VAR_SUBST_OPS, ch)) { /* ${var<bad_char>... */
					إذا (len_single_ch != '#'
					/*|| !strchr(SPECIAL_VARS_STR, ch) - disallow errors like ${#+} ? */
					 || i_peek(درون_داد) != '}'
					) {
						اذهب bad_dollar_syntax;
					}
					/* else: it's "length of C" ${#C} op,
					 * where C is a single char
					 * special var name, e.g. ${#!}.
					 */
				}
 eat_until_closing:
				/* Eat everything until closing '}' (or ':') */
				انتها_ch = '}';
#اگر_تعریف_نشده __U_BOOT__
				إذا (BASH_SUBSTR
				 && ch == ':'
				 && !strchr(MINUS_PLUS_EQUAL_QUESTION, i_peek(درون_داد))
				) {
					/* It's ${var:N[:M]} thing */
					انتها_ch = '}' * 0x100 + ':';
				}
				إذا (BASH_الگو_SUBST
				 && ch == '/'
				) {
					/* It's ${var/[/]pattern[/repl]} thing */
					إذا (i_peek(درون_داد) == '/') { /* ${var//pattern[/repl]}? */
						i_getch(درون_داد);
						nommu_addchr(as_string, '/');
						ch = '\\';
					}
					انتها_ch = '}' * 0x100 + '/';
				}
#پایان_اگر /* !__U_BOOT__ */
				o_addchr(dest, ch);
				/* The pattern can't be empty.
				 * IOW: if the first char after "${v//" is a slash,
				 * it does not terminate the pattern - it's the first char of the pattern:
				 *  v=/dev/ram; echo ${v////-}  prints -dev-ram (pattern is "/")
				 *  v=/dev/ram; echo ${v///r/-} prints /dev-am  (pattern is "/r")
				 */
				إذا (i_peek(درون_داد) == '/') {
					o_addchr(dest, i_getch(درون_داد));
				}
#اگر_تعریف_نشده __U_BOOT__
 again:
#پایان_اگر /* !__U_BOOT__ */
				إذا (!BB_MMU)
					pos = dest->درازی;
#إذا ENABLE_HUSH_DOLLAR_OPS
#اگر_تعریف_نشده __U_BOOT__
				آخرین_ch = جمع_کن_till_closing_bracket(dest, درون_داد, انتها_ch);
				إذا (آخرین_ch == 0) /* error? */
					أرجع 0;
#پایان_اگر /* !__U_BOOT__ */
#وإلا
# error Simple کد مقصد only allow ${var} is not implemented
#پایان_اگر
				إذا (as_string) {
					o_addstr(as_string, dest->داده + pos);
#اگر_تعریف_نشده __U_BOOT__
					o_addchr(as_string, آخرین_ch);
#پایان_اگر /* !__U_BOOT__ */
				}

#اگر_تعریف_نشده __U_BOOT__
				إذا ((BASH_SUBSTR || BASH_الگو_SUBST)
					 && (انتها_ch & 0xff00)
				) {
					/* close the first block: */
					o_addchr(dest, SPECIAL_VAR_نماد);
					/* while parsing N from ${var:N[:M]}
					 * or pattern from ${var/[/]pattern[/repl]} */
					إذا ((انتها_ch & 0xff) == آخرین_ch) {
						/* got ':' or '/'- parse the rest */
						انتها_ch = '}';
						اذهب again;
					}
					/* got '}' */
					إذا (BASH_SUBSTR && انتها_ch == '}' * 0x100 + ':') {
						/* it's ${var:N} - emulate :999999999 */
						o_addstr(dest, "999999999");
					} /* else: it's ${var/[/]pattern} */
				}
#پایان_اگر /* !__U_BOOT__ */
				اكسر;
			}
			len_single_ch = 0; /* it can't be ${#C} op */
		}
		o_addchr(dest, SPECIAL_VAR_نماد);
		اكسر;
	}
#إذا ENABLE_FEATURE_SH_MATH || ENABLE_HUSH_TICK
	حالة '(': {
		بیعلامت pos;

		ch = i_getch(درون_داد);
		nommu_addchr(as_string, ch);
# إذا ENABLE_FEATURE_SH_MATH
		إذا (i_peek_and_eat_bkslash_nl(درون_داد) == '(') {
			ch = i_getch(درون_داد);
			nommu_addchr(as_string, ch);
			o_addchr(dest, SPECIAL_VAR_نماد);
			o_addchr(dest, quote_ماسک | '+');
			إذا (!BB_MMU)
				pos = dest->درازی;
			إذا (!جمع_کن_till_closing_bracket(dest, درون_داد, ')' | DOUBLE_CLOSE_CHAR_FLAG))
				أرجع 0; /* error */
			إذا (as_string) {
				o_addstr(as_string, dest->داده + pos);
				o_addchr(as_string, ')');
				o_addchr(as_string, ')');
			}
			o_addchr(dest, SPECIAL_VAR_نماد);
			اكسر;
		}
# endif
# إذا ENABLE_HUSH_TICK
		o_addchr(dest, SPECIAL_VAR_نماد);
		o_addchr(dest, quote_ماسک | '`');
		إذا (!BB_MMU)
			pos = dest->درازی;
		إذا (!جمع_کن_till_closing_bracket(dest, درون_داد, ')'))
			أرجع 0; /* error */
		إذا (as_string) {
			o_addstr(as_string, dest->داده + pos);
			o_addchr(as_string, ')');
		}
		o_addchr(dest, SPECIAL_VAR_نماد);
# endif
		اكسر;
	}
#پایان_اگر
	حالة '_':
		اذهب make_var;
#إذا 0
	/* TODO: $_: */
	/* $_ Shell or shell script name; or last argument of last command
	 * (if last command wasn't a pipe; if it was, bash sets $_ to "");
	 * but in command's env, set to full pathname used to invoke it */
		ch = i_getch(درون_داد);
		nommu_addchr(as_string, ch);
		ch = i_peek_and_eat_bkslash_nl(درون_داد);
		إذا (آیا_حرف_یا_رقم(ch)) { /* it's $_name or $_123 */
			ch = '_';
			اذهب make_var1;
		}
		/* else: it's $_ */
#پایان_اگر
	افتراضي:
		o_جمع_کن_Qchr(dest, '$');
	}
	debug_printf_parse("parse_dollar return 1 (ok)\n");
	أرجع 1;
#لغو_تعریف as_string
}

#إذا BB_MMU
#تعریف_کن encode_string(as_string, dest, درون_داد, dquote_انتها) \
	encode_string(dest, درون_داد, dquote_انتها)
#تعریف_کن as_string عدم
#پایان_اگر
ساكن صحيح encode_string(o_string *as_string,
		o_string *dest,
		بنية درون_str *درون_داد,
		صحيح dquote_انتها)
{
	صحيح ch;
	صحيح بعدی;

 again:
	ch = i_getch(درون_داد);
	إذا (ch != EOF)
		nommu_addchr(as_string, ch);
	إذا (ch == dquote_انتها) { /* may be only '"' or EOF */
		debug_printf_parse("encode_string return 1 (ok)\n");
		أرجع 1;
	}
	/* note: can't move it above ch == dquote_end check! */
	إذا (ch == EOF) {
		syntax_error_unterm_ch('"');
		أرجع 0; /* error */
	}
	بعدی = '\0';
	إذا (ch != '\n') {
		بعدی = i_peek(درون_داد);
	}
	debug_printf_parse("\" ch=%c (%d) escape=%d\n",
			ch, ch, !!(dest->o_expflags & EXP_FLAG_ESC_GLOB_CHARS));
	إذا (ch == '\\') {
		إذا (بعدی == EOF) {
			/* Testcase: in interactive shell a file with
			 *  echo "unterminated string\<eof>
			 * is sourced.
			 */
			syntax_error_unterm_ch('"');
			أرجع 0; /* error */
		}
		/* bash:
		 * "The backslash retains its special meaning [in "..."]
		 * only when followed by one of the following characters:
		 * $, `, ", \, or <newline>.  A double quote may be quoted
		 * within double quotes by preceding it with a backslash."
		 * NB: in (unquoted) heredoc, above does not apply to ",
		 * therefore we check for it by "next == dquote_end" cond.
		 */
		إذا (بعدی == dquote_انتها || strchr("$`\\\n", بعدی)) {
			ch = i_getch(درون_داد); /* eat next */
			إذا (ch == '\n')
				اذهب again; /* skip \<newline> */
		} /* else: ch remains == '\\', and we double it below: */
		o_addqchr(dest, ch); /* \c if c is a glob char, else just c */
		nommu_addchr(as_string, ch);
		اذهب again;
	}
	إذا (ch == '$') {
		//if (parse_dollar_squote(as_string, dest, input))
		//	goto again;
		إذا (!parse_dollar(as_string, dest, درون_داد, /*quote_mask:*/ 0x80)) {
			debug_printf_parse("encode_string return 0: "
					"parse_dollar returned 0 (error)\n");
			أرجع 0;
		}
		اذهب again;
	}
#إذا ENABLE_HUSH_TICK
	إذا (ch == '`') {
		//unsigned pos = dest->length;
		o_addchr(dest, SPECIAL_VAR_نماد);
		o_addchr(dest, 0x80 | '`');
		إذا (!جمع_کن_till_backquote(dest, درون_داد, /*in_dquote:*/ dquote_انتها == '"'))
			أرجع 0; /* error */
		o_addchr(dest, SPECIAL_VAR_نماد);
		//debug_printf_subst("SUBST RES3 '%s'\n", dest->data + pos);
		اذهب again;
	}
#پایان_اگر
	o_جمع_کن_Qchr(dest, ch);
	إذا (ch == SPECIAL_VAR_نماد) {
		/* Convert "^C" to corresponding special variable reference */
		o_addchr(dest, SPECIAL_VAR_QUOTED_SVS);
		o_addchr(dest, SPECIAL_VAR_نماد);
	}
	اذهب again;
#لغو_تعریف as_string
}

/*
 * Scan input until EOF or end_trigger char.
 * Return a list of pipes to execute, or NULL on EOF
 * or if end_trigger character is met.
 * On syntax error, exit if shell is not interactive,
 * reset parsing machinery and start parsing anew,
 * or return ERR_PTR.
 */
ساكن بنية pipe *parse_جریان(حرف **pstring,
		صحيح *heredoc_cnt_ptr,
		بنية درون_str *درون_داد,
		صحيح انتها_trigger)
{
	بنية parse_context ctx;
	صحيح heredoc_cnt;

	/* Single-quote triggers a bypass of the main loop until its mate is
	 * found.  When recursing, quote state is passed in via ctx.word.o_expflags.
	 */
	debug_printf_parse("parse_stream entered, end_trigger='%c'\n",
			انتها_trigger ? انتها_trigger : 'X');
	debug_enter();

	initialize_context(&ctx);

	/* If very first arg is "" or '', ctx.word.data may end up NULL.
	 * Preventing this:
	 */
	ctx.کلمه.داده = xzalloc(1); /* start as "", not as NULL */

	/* We used to separate words on $IFS here. This was wrong.
	 * $IFS is used only for word splitting when $var is expanded,
	 * here we should use blank chars as separators, not $IFS
	 */

	heredoc_cnt = 0;
	طالما (1) {
		ثابت حرف *is_blank;
		ثابت حرف *is_special;
		صحيح ch;
		صحيح بعدی;
#اگر_تعریف_نشده __U_BOOT__
		صحيح redir_fd;
		redir_type redir_style;
#پایان_اگر /* !__U_BOOT__ */

		ch = i_getch(درون_داد);
		debug_printf_parse(": ch=%c (%d) escape=%d\n",
				ch, ch, !!(ctx.کلمه.o_expflags & EXP_FLAG_ESC_GLOB_CHARS));
		إذا (ch == EOF) {
			بنية pipe *pi;

			إذا (heredoc_cnt) {
				syntax_error_unterm_str("here document");
				اذهب parse_error_exitcode1;
			}
			إذا (انتها_trigger == ')') {
				syntax_error_unterm_ch('(');
				اذهب parse_error_exitcode1;
			}
			إذا (انتها_trigger == '}') {
				syntax_error_unterm_ch('{');
				اذهب parse_error_exitcode1;
			}

			إذا (انجامشده_کلمه(&ctx)) {
				اذهب parse_error_exitcode1;
			}
			o_free_and_مجموعه_NULL(&ctx.کلمه);
			انجامشده_pipe(&ctx, PIPE_SEQ);

			/* Do we sit inside of any if's, loops or case's? */
			إذا (HAS_KEYWORDS
			IF_HAS_KEYWORDS(&& (ctx.ctx_res_w != RES_NONE || ctx.old_flag != 0))
			) {
				syntax_error_unterm_str("compound statement");
				اذهب parse_error_exitcode1;
			}

			pi = ctx.list_سر;
			/* If we got nothing... */
			/* (this makes bare "&" cmd a no-op.
			 * bash says: "syntax error near unexpected token '&'") */
			إذا (pi->رقم_cmds == 0
			IF_HAS_KEYWORDS(&& pi->res_کلمه == RES_NONE)
			) {
				free_pipe_list(pi);
				pi = عدم;
			}
#إذا !BB_MMU
			debug_printf_parse("as_string1 '%s'\n", ctx.as_string.داده);
			إذا (pstring)
				*pstring = ctx.as_string.داده;
			وإلا
				o_free(&ctx.as_string);
#پایان_اگر
			// heredoc_cnt must be 0 here anyway
			//if (heredoc_cnt_ptr)
			//	*heredoc_cnt_ptr = heredoc_cnt;
			debug_leave();
			debug_printf_heredoc("parse_stream return heredoc_cnt:%d\n", heredoc_cnt);
			debug_printf_parse("parse_stream return %p: EOF\n", pi);
			أرجع pi;
		}

		/* Handle "'" and "\" first, as they won't play nice with
		 * i_peek_and_eat_bkslash_nl() anyway:
		 *   echo z\\
		 * and
		 *   echo '\
		 *   '
		 * would break.
		 */
		إذا (ch == '\\') {
			ch = i_getch(درون_داد);
			إذا (ch == '\n')
				استمر; /* drop \<newline>, get next char */
			nommu_addchr(&ctx.as_string, '\\');
			إذا (ch == SPECIAL_VAR_نماد) {
				nommu_addchr(&ctx.as_string, ch);
				/* Convert \^C to corresponding special variable reference */
				اذهب case_SPECIAL_VAR_نماد;
			}
			o_addchr(&ctx.کلمه, '\\');
			إذا (ch == EOF) {
				/* Testcase: eval 'echo Ok\' */
				/* bash-4.3.43 was removing backslash,
				 * but 4.4.19 retains it, most other shells too
				 */
				استمر; /* get next char */
			}
			/* Example: echo Hello \2>file
			 * we need to know that word 2 is quoted
			 */
			ctx.کلمه.has_quoted_part = 1;
			nommu_addchr(&ctx.as_string, ch);
			o_addchr(&ctx.کلمه, ch);
			استمر; /* get next char */
		}
		nommu_addchr(&ctx.as_string, ch);
		إذا (ch == '\'') {
			ctx.کلمه.has_quoted_part = 1;
			بعدی = i_getch(درون_داد);
#اگر_تعریف_نشده __U_BOOT__
			إذا (بعدی == '\'' && !ctx.pending_redirect)
				اذهب درج_خالی_quoted_str_marker;
#پایان_اگر /* !__U_BOOT__ */

			ch = بعدی;
			طالما (1) {
				إذا (ch == EOF) {
					syntax_error_unterm_ch('\'');
					اذهب parse_error_exitcode1;
				}
				nommu_addchr(&ctx.as_string, ch);
				إذا (ch == '\'')
					اكسر;
				إذا (ch == SPECIAL_VAR_نماد) {
					/* Convert raw ^C to corresponding special variable reference */
					o_addchr(&ctx.کلمه, SPECIAL_VAR_نماد);
					o_addchr(&ctx.کلمه, SPECIAL_VAR_QUOTED_SVS);
				}
				o_addqchr(&ctx.کلمه, ch);
				ch = i_getch(درون_داد);
			}
			استمر; /* get next char */
		}

		بعدی = '\0';
		إذا (ch != '\n')
			بعدی = i_peek_and_eat_bkslash_nl(درون_داد);

		is_special = "{}<>&|();#" /* special outside of "str" */
#اگر_تعریف_نشده __U_BOOT__
				"$\"" IF_HUSH_TICK("`") /* always special */
#وإلا /* __U_BOOT__ */
				"$\""
#پایان_اگر /* __U_BOOT__ */
				SPECIAL_VAR_نماد_STR;
#إذا defined(CMD_TEST2_SINGLEWORD_NOGLOB)
		إذا (ctx.command->cmd_type == CMD_TEST2_SINGLEWORD_NOGLOB) {
			/* In [[ ]], {}<>&|() are not special */
			is_special += 8;
		} وإلا
#پایان_اگر
		/* Are { and } special here? */
		إذا (ctx.command->argv /* word [word]{... - non-special */
		 || ctx.کلمه.درازی       /* word{... - non-special */
		 || ctx.کلمه.has_quoted_part     /* ""{... - non-special */
		 || (بعدی != ';'             /* }; - special */
		    && بعدی != ')'           /* }) - special */
		    && بعدی != '('           /* {( - special */
		    && بعدی != '&'           /* }& and }&& ... - special */
		    && بعدی != '|'           /* }|| ... - special */
		    && !strchr(defifs, بعدی) /* {word - non-special */
		    )
		) {
			/* They are not special, skip "{}" */
			is_special += 2;
		}
		is_special = strchr(is_special, ch);
		is_blank = strchr(defifs, ch);

		إذا (!is_special && !is_blank) { /* ordinary char */
 ordinary_char:
			o_جمع_کن_Qchr(&ctx.کلمه, ch);
			إذا ((ctx.is_assignment == MAYBE_ASSIGNMENT
			    || ctx.is_assignment == کلمه_IS_KEYWORD)
			 && ch == '='
			 && endofname(ctx.کلمه.داده)[0] == '='
			) {
				ctx.is_assignment = DEFINITELY_ASSIGNMENT;
				debug_printf_parse("ctx.is_assignment='%s'\n", assignment_flag[ctx.is_assignment]);
			}
			استمر;
		}

		إذا (is_blank) {
#إذا ENABLE_HUSH_LINENO_VAR
/* Case:
 * "while ...; do<whitespace><newline>
 *	cmd ..."
 * would think that "cmd" starts in <whitespace> -
 * i.e., at the previous line.
 * We need to skip all whitespace before newlines.
 */
			طالما (ch != '\n') {
				بعدی = i_peek(درون_داد);
				إذا (بعدی != ' ' && بعدی != '\t' && بعدی != '\n')
					اكسر; /* next char is not ws */
				ch = i_getch(درون_داد);
			}
			/* ch == last eaten whitespace char */
#پایان_اگر
			إذا (انجامشده_کلمه(&ctx)) {
				اذهب parse_error_exitcode1;
			}
			إذا (ch == '\n') {
				/* Is this a case when newline is simply ignored?
				 * Some examples:
				 * "cmd | <newline> cmd ..."
				 * "case ... in <newline> word) ..."
				 */
				إذا (IS_NULL_CMD(ctx.command)
				 && ctx.کلمه.درازی == 0
				 && !ctx.کلمه.has_quoted_part
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
					بنية pipe *pi = ctx.list_سر;
					إذا (pi->رقم_cmds != 0       /* check #1 */
					 && pi->followup != PIPE_BG /* check #2 */
					) {
						استمر;
					}
				}
				/* Treat newline as a command separator. */
				انجامشده_pipe(&ctx, PIPE_SEQ);
				debug_printf_heredoc("heredoc_cnt:%d\n", heredoc_cnt);
				إذا (heredoc_cnt) {
					heredoc_cnt = fetch_heredocs(&ctx.as_string, ctx.list_سر, heredoc_cnt, درون_داد);
					إذا (heredoc_cnt != 0)
						اذهب parse_error_exitcode1;
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
#اگر_تعریف_نشده __U_BOOT__
		إذا (ch == '}') {
#وإلا /* __U_BOOT__ */
		إذا (ch == '}' || ch == ')') {
#پایان_اگر /* __U_BOOT__ */
			إذا (ctx.کلمه.درازی != 0 /* word} */
			 || ctx.کلمه.has_quoted_part    /* ""} */
			) {
				اذهب ordinary_char;
			}
			إذا (!IS_NULL_CMD(ctx.command)) { /* cmd } */
				/* Generally, there should be semicolon: "cmd; }"
				 * However, bash allows to omit it if "cmd" is
				 * a group. Examples:
				 * { { echo 1; } }
				 * {(echo 1)}
				 * { echo 0 >&2 | { echo 1; } }
				 * { while false; do :; done }
				 * { case a in b) ;; esac }
				 */
				إذا (ctx.command->گروه)
					اذهب جمله_گروه;
				اذهب ordinary_char;
			}
			إذا (!IS_NULL_PIPE(ctx.pipe)) /* cmd | } */
				/* Can't be an end of {cmd}, skip the check */
				اذهب skip_انتها_trigger;
			/* else: } does terminate a group */
		}
 جمله_گروه:
		إذا (انتها_trigger && انتها_trigger == ch
		 && (ch != ';' || heredoc_cnt == 0)
#إذا ENABLE_HUSH_CASE
		 && (ch != ')'
		    || ctx.ctx_res_w != RES_تطابق
		    || (!ctx.کلمه.has_quoted_part && strcmp(ctx.کلمه.داده, "esac") == 0)
		    )
#پایان_اگر
		) {
			إذا (انجامشده_کلمه(&ctx)) {
				اذهب parse_error_exitcode1;
			}
			انجامشده_pipe(&ctx, PIPE_SEQ);
			ctx.is_assignment = MAYBE_ASSIGNMENT;
			debug_printf_parse("ctx.is_assignment='%s'\n", assignment_flag[ctx.is_assignment]);
			/* Do we sit outside of any if's, loops or case's? */
			إذا (!HAS_KEYWORDS
			IF_HAS_KEYWORDS(|| (ctx.ctx_res_w == RES_NONE && ctx.old_flag == 0))
			) {
				o_free_and_مجموعه_NULL(&ctx.کلمه);
#إذا !BB_MMU
				debug_printf_parse("as_string2 '%s'\n", ctx.as_string.داده);
				إذا (pstring)
					*pstring = ctx.as_string.داده;
				وإلا
					o_free(&ctx.as_string);
#پایان_اگر
				إذا (ch != ';' && IS_NULL_PIPE(ctx.list_سر)) {
					/* Example: bare "{ }", "()" */
					G.آخرین_exitcode = 2; /* bash compat */
					syntax_error_unexpected_ch(ch);
					اذهب parse_error;
				}
				إذا (heredoc_cnt_ptr)
					*heredoc_cnt_ptr = heredoc_cnt;
				debug_printf_heredoc("parse_stream return heredoc_cnt:%d\n", heredoc_cnt);
				debug_printf_parse("parse_stream return %p: "
						"end_trigger char found\n",
						ctx.list_سر);
				debug_leave();
				أرجع ctx.list_سر;
			}
		}

		إذا (is_blank)
			استمر;

		/* Catch <, > before deciding whether this word is
		 * an assignment. a=1 2>z b=2: b=2 is still assignment */
		بدل (ch) {
#اگر_تعریف_نشده __U_BOOT__
		حالة '>':
			redir_fd = redirect_opt_رقم(&ctx.کلمه);
			إذا (انجامشده_کلمه(&ctx)) {
				اذهب parse_error_exitcode1;
			}
			redir_style = REDIRECT_OVERWRITE;
			إذا (بعدی == '>') {
				redir_style = REDIRECT_APPEND;
				ch = i_getch(درون_داد);
				nommu_addchr(&ctx.as_string, ch);
			}
#إذا 0
			وإلا إذا (بعدی == '(') {
				syntax_error(">(process) not supported");
				اذهب parse_error_exitcode1;
			}
#پایان_اگر
			إذا (parse_redirect(&ctx, redir_fd, redir_style, درون_داد))
				اذهب parse_error_exitcode1;
			استمر; /* get next char */
		حالة '<':
			redir_fd = redirect_opt_رقم(&ctx.کلمه);
			إذا (انجامشده_کلمه(&ctx)) {
				اذهب parse_error_exitcode1;
			}
			redir_style = REDIRECT_درون_داد;
			إذا (بعدی == '<') {
				redir_style = REDIRECT_HEREDOC;
				heredoc_cnt++;
				debug_printf_heredoc("++heredoc_cnt=%d\n", heredoc_cnt);
				ch = i_getch(درون_داد);
				nommu_addchr(&ctx.as_string, ch);
			} وإلا إذا (بعدی == '>') {
				redir_style = REDIRECT_IO;
				ch = i_getch(درون_داد);
				nommu_addchr(&ctx.as_string, ch);
			}
#إذا 0
			وإلا إذا (بعدی == '(') {
				syntax_error("<(process) not supported");
				اذهب parse_error_exitcode1;
			}
#پایان_اگر
			إذا (parse_redirect(&ctx, redir_fd, redir_style, درون_داد))
				اذهب parse_error_exitcode1;
			استمر; /* get next char */
#وإلا /* __U_BOOT__ */
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
			حالة '>':
				fallthrough;
			حالة '<':
				o_جمع_کن_Qchr(&ctx.کلمه, ch);
				استمر;
#پایان_اگر /* __U_BOOT__ */
		حالة '#':
			إذا (ctx.کلمه.درازی == 0 && !ctx.کلمه.has_quoted_part) {
				/* skip "#comment" */
				/* note: we do not add it to &ctx.as_string */
/* TODO: in bash:
 * comment inside $() goes to the next \n, even inside quoted string (!):
 * cmd "$(cmd2 #comment)" - syntax error
 * cmd "`cmd2 #comment`" - ok
 * We accept both (comment ends where command subst ends, in both cases).
 */
				طالما (1) {
					ch = i_peek(درون_داد);
					إذا (ch == '\n') {
						nommu_addchr(&ctx.as_string, '\n');
						اكسر;
					}
					ch = i_getch(درون_داد);
					إذا (ch == EOF)
						اكسر;
				}
				استمر; /* get next char */
			}
			اكسر;
		}
 skip_انتها_trigger:

		إذا (ctx.is_assignment == MAYBE_ASSIGNMENT
#اگر_تعریف_نشده __U_BOOT__
		 /* check that we are not in word in "a=1 2>word b=1": */
		 && !ctx.pending_redirect
#پایان_اگر /* !__U_BOOT__ */
		) {
			/* ch is a special char and thus this word
			 * cannot be an assignment */
			ctx.is_assignment = NOT_ASSIGNMENT;
			debug_printf_parse("ctx.is_assignment='%s'\n", assignment_flag[ctx.is_assignment]);
		}

		/* Note: nommu_addchr(&ctx.as_string, ch) is already done */

		بدل (ch) {
		case_SPECIAL_VAR_نماد:
		حالة SPECIAL_VAR_نماد:
			/* Convert raw ^C to corresponding special variable reference */
			o_addchr(&ctx.کلمه, SPECIAL_VAR_نماد);
			o_addchr(&ctx.کلمه, SPECIAL_VAR_QUOTED_SVS);
			/* fall through */
		حالة '#':
			/* non-comment #: "echo a#b" etc */
			o_addchr(&ctx.کلمه, ch);
			استمر; /* get next char */
		حالة '$':
#اگر_تعریف_نشده __U_BOOT__
			إذا (parse_dollar_squote(&ctx.as_string, &ctx.کلمه, درون_داد))
				استمر; /* get next char */
#پایان_اگر /* !__U_BOOT__ */
			إذا (!parse_dollar(&ctx.as_string, &ctx.کلمه, درون_داد, /*quote_mask:*/ 0)) {
				debug_printf_parse("parse_stream parse error: "
					"parse_dollar returned 0 (error)\n");
				اذهب parse_error_exitcode1;
			}
			استمر; /* get next char */
		حالة '"':
			ctx.کلمه.has_quoted_part = 1;
#اگر_تعریف_نشده __U_BOOT__
			إذا (بعدی == '"' && !ctx.pending_redirect) {
#وإلا /* __U_BOOT__ */
			إذا (بعدی == '"') {
#پایان_اگر /* __U_BOOT__ */
				i_getch(درون_داد); /* eat second " */
#اگر_تعریف_نشده __U_BOOT__
 درج_خالی_quoted_str_marker:
#پایان_اگر /* !__U_BOOT__ */
				nommu_addchr(&ctx.as_string, بعدی);
				o_addchr(&ctx.کلمه, SPECIAL_VAR_نماد);
				o_addchr(&ctx.کلمه, SPECIAL_VAR_نماد);
				استمر; /* get next char */
			}
			إذا (ctx.is_assignment == NOT_ASSIGNMENT)
				ctx.کلمه.o_expflags |= EXP_FLAG_ESC_GLOB_CHARS;
			إذا (!encode_string(&ctx.as_string, &ctx.کلمه, درون_داد, '"'))
				اذهب parse_error_exitcode1;
			ctx.کلمه.o_expflags &= ~EXP_FLAG_ESC_GLOB_CHARS;
			استمر; /* get next char */
#إذا ENABLE_HUSH_TICK
		حالة '`': {
			USE_FOR_NOMMU(بیعلامت pos;)

			o_addchr(&ctx.کلمه, SPECIAL_VAR_نماد);
			o_addchr(&ctx.کلمه, '`');
			USE_FOR_NOMMU(pos = ctx.کلمه.درازی;)
			إذا (!جمع_کن_till_backquote(&ctx.کلمه, درون_داد, /*in_dquote:*/ 0))
				اذهب parse_error_exitcode1;
# إذا !BB_MMU
			o_addstr(&ctx.as_string, ctx.کلمه.داده + pos);
			o_addchr(&ctx.as_string, '`');
# endif
			o_addchr(&ctx.کلمه, SPECIAL_VAR_نماد);
			//debug_printf_subst("SUBST RES3 '%s'\n", ctx.word.data + pos);
			استمر; /* get next char */
		}
#پایان_اگر
		حالة ';':
#إذا ENABLE_HUSH_CASE
 case_semi:
#پایان_اگر
			إذا (انجامشده_کلمه(&ctx)) {
				اذهب parse_error_exitcode1;
			}
			انجامشده_pipe(&ctx, PIPE_SEQ);
#إذا ENABLE_HUSH_CASE
			/* Eat multiple semicolons, detect
			 * whether it means something special */
			طالما (1) {
				ch = i_peek_and_eat_bkslash_nl(درون_داد);
				إذا (ch != ';')
					اكسر;
				ch = i_getch(درون_داد);
				nommu_addchr(&ctx.as_string, ch);
				إذا (ctx.ctx_res_w == RES_CASE_BODY) {
					ctx.ctx_dsemicolon = 1;
					ctx.ctx_res_w = RES_تطابق;
					اكسر;
				}
			}
#پایان_اگر
 new_cmd:
			/* We just finished a cmd. New one may start
			 * with an assignment */
			ctx.is_assignment = MAYBE_ASSIGNMENT;
			debug_printf_parse("ctx.is_assignment='%s'\n", assignment_flag[ctx.is_assignment]);
			استمر; /* get next char */
		حالة '&':
			إذا (انجامشده_کلمه(&ctx)) {
				اذهب parse_error_exitcode1;
			}
			إذا (بعدی == '&') {
				ch = i_getch(درون_داد);
				nommu_addchr(&ctx.as_string, ch);
				انجامشده_pipe(&ctx, PIPE_AND);
			} وإلا {
				انجامشده_pipe(&ctx, PIPE_BG);
			}
			اذهب new_cmd;
		حالة '|':
			إذا (انجامشده_کلمه(&ctx)) {
				اذهب parse_error_exitcode1;
			}
#إذا ENABLE_HUSH_CASE
			إذا (ctx.ctx_res_w == RES_تطابق)
				اكسر; /* we are in case's "word | word)" */
#پایان_اگر
			إذا (بعدی == '|') { /* || */
				ch = i_getch(درون_داد);
				nommu_addchr(&ctx.as_string, ch);
				انجامشده_pipe(&ctx, PIPE_OR);
			} وإلا {
				/* we could pick up a file descriptor choice here
				 * with redirect_opt_num(), but bash doesn't do it.
				 * "echo foo 2| cat" yields "foo 2". */
				انجامشده_command(&ctx);
			}
			اذهب new_cmd;
		حالة '(':
#إذا ENABLE_HUSH_CASE
			/* "case... in [(]word)..." - skip '(' */
			إذا (ctx.ctx_res_w == RES_تطابق
			 && ctx.command->argv == عدم /* not (word|(... */
			 && ctx.کلمه.درازی == 0 /* not word(... */
			 && ctx.کلمه.has_quoted_part == 0 /* not ""(... */
			) {
				استمر; /* get next char */
			}
#پایان_اگر
			/* fall through */
		حالة '{': {
			صحيح n = parse_گروه(&ctx, درون_داد, ch);
			إذا (n < 0) {
				اذهب parse_error_exitcode1;
			}
			debug_printf_heredoc("parse_group done, needs heredocs:%d\n", n);
			heredoc_cnt += n;
			اذهب new_cmd;
		}
		حالة ')':
#إذا ENABLE_HUSH_CASE
			إذا (ctx.ctx_res_w == RES_تطابق)
				اذهب case_semi;
#پایان_اگر
		حالة '}':
			/* proper use of this character is caught by end_trigger:
			 * if we see {, we call parse_group(..., end_trigger='}')
			 * and it will match } earlier (not here). */
			G.آخرین_exitcode = 2;
			syntax_error_unexpected_ch(ch);
			اذهب parse_error;
		افتراضي:
			إذا (HUSH_DEBUG)
				bb_error_پیام_and_die("BUG: unexpected %c", ch);
		}
	} /* while (1) */

 parse_error_exitcode1:
	G.آخرین_exitcode = 1;
 parse_error:
	{
		بنية parse_context *pctx;
		IF_HAS_KEYWORDS(بنية parse_context *p2;)

		/* Clean up allocated tree.
		 * Sample for finding leaks on syntax error recovery path.
		 * Run it from interactive shell, watch pmap `pidof hush`.
		 * while if false; then false; fi; do break; fi
		 * Samples to catch leaks at execution:
		 * while if (true | { true;}); then echo ok; fi; do break; done
		 * while if (true | { true;}); then echo ok; fi; do (if echo ok; break; then :; fi) | cat; break; done
		 */
		pctx = &ctx;
		افعل {
			/* Update pipe/command counts,
			 * otherwise freeing may miss some */
			انجامشده_pipe(pctx, PIPE_SEQ);
			debug_printf_clean("freeing list %p from ctx %p\n",
					pctx->list_سر, pctx);
			debug_print_درخت(pctx->list_سر, 0);
			free_pipe_list(pctx->list_سر);
			debug_printf_clean("freed list %p\n", pctx->list_سر);
#إذا !BB_MMU
			o_free(&pctx->as_string);
#پایان_اگر
			IF_HAS_KEYWORDS(p2 = pctx->پشته;)
			إذا (pctx != &ctx) {
				حرر(pctx);
			}
			IF_HAS_KEYWORDS(pctx = p2;)
		} طالما (HAS_KEYWORDS && pctx);

		o_free(&ctx.کلمه);
#إذا !BB_MMU
		إذا (pstring)
			*pstring = عدم;
#پایان_اگر
		debug_leave();
		أرجع ERR_PTR;
	}
}

/*** Execution routines ***/

/* Expansion can recurse, need forward decls: */
#إذا !BASH_الگو_SUBST && !ENABLE_HUSH_CASE
#تعریف_کن expand_string_مقصد_string(str, EXP_flags, do_unbackslash) \
	expand_string_مقصد_string(str)
#پایان_اگر
ساكن حرف *expand_string_مقصد_string(ثابت حرف *str, صحيح EXP_flags, صحيح do_unbackslash);
#إذا ENABLE_HUSH_TICK
ساكن صحيح process_command_subs(o_string *dest, ثابت حرف *s);
#پایان_اگر
ساكن صحيح expand_vars_مقصد_list(o_string *output, صحيح n, حرف *arg);

/* expand_strvec_to_strvec() takes a list of strings, expands
 * all variable references within and returns a pointer to
 * a list of expanded strings, possibly with larger number
 * of strings. (Think VAR="a b"; echo $VAR).
 * This new list is allocated as a single malloc block.
 * NULL-terminated list of char* pointers is at the beginning of it,
 * followed by strings themselves.
 * Caller can deallocate entire list by single free(list). */

/* A horde of its helpers come first: */

ساكن فراغ o_addblock_duplicate_backslash(o_string *o, ثابت حرف *str, صحيح len)
{
	طالما (--len >= 0) {
		حرف c = *str++;

#إذا ENABLE_HUSH_BRACE_EXPANSION
		إذا (c == '{' || c == '}') {
			/* { -> \{, } -> \} */
			o_addchr(o, '\\');
			/* And now we want to add { or } and continue:
			 *  o_addchr(o, c);
			 *  continue;
			 * luckily, just falling through achieves this.
			 */
		}
#پایان_اگر
		o_addchr(o, c);
		إذا (c == '\\') {
			/* \z -> \\\z; \<eol> -> \\<eol> */
			o_addchr(o, '\\');
			إذا (len) {
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
ساكن صحيح expand_on_ifs(o_string *output, صحيح n, ثابت حرف *str)
{
	صحيح آخرین_is_ifs = 0;

	طالما (1) {
		صحيح کلمه_len;

		إذا (!*str)  /* EOL - do not finalize word */
			اكسر;
		کلمه_len = strcspn(str, G.ifs);
		إذا (کلمه_len) {
			/* We have WORD_LEN leading non-IFS chars */
			إذا (!(output->o_expflags & EXP_FLAG_GLOB)) {
				o_addblock(output, str, کلمه_len);
			} وإلا {
				/* Protect backslashes against globbing up :)
				 * Example: "v='\*'; echo b$v" prints "b\*"
				 * (and does not try to glob on "*")
				 */
				o_addblock_duplicate_backslash(output, str, کلمه_len);
				/*/ Why can't we do it easier? */
				/*o_addblock(output, str, word_len); - WRONG: "v='\*'; echo Z$v" prints "Z*" instead of "Z\*" */
				/*o_addqblock(output, str, word_len); - WRONG: "v='*'; echo Z$v" prints "Z*" instead of Z* files */
			}
			آخرین_is_ifs = 0;
			str += کلمه_len;
			إذا (!*str)  /* EOL - do not finalize word */
				اكسر;
		}

		/* We know str here points to at least one IFS char */
		آخرین_is_ifs = 1;
		str += strspn(str, G.ifs_whitespace); /* skip IFS whitespace chars */
		إذا (!*str)  /* EOL - do not finalize word */
			اكسر;

		إذا (G.ifs_whitespace != G.ifs /* usually false ($IFS is usually all whitespace), */
		 && strchr(G.ifs, *str)       /* the second check would fail */
		) {
			/* This is a non-whitespace $IFS char */
			/* Skip it and IFS whitespace chars, start new word */
			str++;
			str += strspn(str, G.ifs_whitespace);
			اذهب new_کلمه;
		}

		/* Start new word... but not always! */
		/* Case "v=' a'; echo ''$v": we do need to finalize empty word: */
		إذا (output->has_quoted_part
		/*
		 * Case "v=' a'; echo $v":
		 * here nothing precedes the space in $v expansion,
		 * therefore we should not finish the word
		 * (IOW: if there *is* word to finalize, only then do it):
		 * It's okay if this accesses the byte before first argv[]:
		 * past call to o_save_ptr() cleared it to zero byte
		 * (grep for -prev-ifs-check-).
		 */
		 || output->داده[output->درازی - 1]
		) {
 new_کلمه:
			o_addchr(output, '\0');
			debug_print_list("expand_on_ifs", output, n);
			n = o_save_ptr(output, n);
		}
	}

	output->ended_درون_ifs = آخرین_is_ifs;
	debug_print_list("expand_on_ifs[1]", output, n);
	أرجع n;
}

#اگر_تعریف_نشده __U_BOOT__
/* Helper to expand $((...)) and heredoc body. These act as if
 * they are in double quotes, with the exception that they are not :).
 * Just the rules are similar: "expand only $var and `cmd`"
 *
 * Returns malloced string.
 * As an optimization, we return NULL if expansion is not needed.
 */
ساكن حرف *encode_then_expand_string(ثابت حرف *str)
{
	حرف *exp_str;
	بنية درون_str درون_داد;
	o_string dest = NULL_O_STRING;
	ثابت حرف *cp;

	cp = str;
	لكل (;;) {
		إذا (!*cp) أرجع عدم; /* string has no special chars */
		إذا (*cp == '$') اكسر;
		إذا (*cp == '\\') اكسر;
#إذا ENABLE_HUSH_TICK
		إذا (*cp == '`') اكسر;
#پایان_اگر
		cp++;
	}

	/* We need to expand. Example:
	 * echo $(($a + `echo 1`)) $((1 + $((2)) ))
	 */
	راهاندازی_string_درون_str(&درون_داد, str);
	encode_string(عدم, &dest, &درون_داد, EOF);
//TODO: error check (encode_string returns 0 on error)?
	//bb_error_msg("'%s' -> '%s'", str, dest.data);
	exp_str = expand_string_مقصد_string(dest.داده,
			EXP_FLAG_ESC_GLOB_CHARS,
			/*unbackslash:*/ 1
	);
	//bb_error_msg("'%s' -> '%s'", dest.data, exp_str);
	o_free(&dest);
	أرجع exp_str;
}

ساكن ثابت حرف *اولی_special_char_درون_vararg(ثابت حرف *cp)
{
	لكل (;;) {
		إذا (!*cp) أرجع عدم; /* string has no special chars */
		إذا (*cp == '$') أرجع cp;
		إذا (*cp == '\\') أرجع cp;
		إذا (*cp == '\'') أرجع cp;
		إذا (*cp == '"') أرجع cp;
#إذا ENABLE_HUSH_TICK
		إذا (*cp == '`') أرجع cp;
#پایان_اگر
		/* dquoted "${x:+ARG}" should not glob, therefore
		 * '*' et al require some non-literal processing: */
		إذا (*cp == '*') أرجع cp;
		إذا (*cp == '?') أرجع cp;
		إذا (*cp == '[') أرجع cp;
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
#إذا !BASH_الگو_SUBST
#تعریف_کن encode_then_expand_vararg(str, handle_squotes, do_unbackslash) \
	encode_then_expand_vararg(str, handle_squotes)
#پایان_اگر
ساكن حرف *encode_then_expand_vararg(ثابت حرف *str, صحيح handle_squotes, صحيح do_unbackslash)
{
#إذا !BASH_الگو_SUBST && ENABLE_HUSH_CASE
	ثابت صحيح do_unbackslash = 0;
#پایان_اگر
	حرف *exp_str;
	بنية درون_str درون_داد;
	o_string dest = NULL_O_STRING;

	إذا (!اولی_special_char_درون_vararg(str)) {
		/* string has no special chars */
		أرجع عدم;
	}

	راهاندازی_string_درون_str(&درون_داد, str);
	dest.داده = xzalloc(1); /* start as "", not as NULL */
	exp_str = عدم;

	لكل (;;) {
		صحيح ch;

		ch = i_getch(&درون_داد);
		debug_printf_parse("%s: ch=%c (%d) escape=%d\n",
				__func__, ch, ch, !!dest.o_expflags);

		إذا (!dest.o_expflags) {
			إذا (ch == EOF)
				اكسر;
			إذا (handle_squotes && ch == '\'') {
				إذا (!جمع_کن_till_single_quote_dquoted(&dest, &درون_داد))
					اذهب ret; /* error */
				استمر;
			}
		}
		إذا (ch == EOF) {
			syntax_error_unterm_ch('"');
			اذهب ret; /* error */
		}
		إذا (ch == '"') {
			dest.o_expflags ^= EXP_FLAG_ESC_GLOB_CHARS;
			استمر;
		}
		إذا (ch == '\\') {
			ch = i_getch(&درون_داد);
			إذا (ch == EOF) {
//example? error message?	syntax_error_unterm_ch('"');
				debug_printf_parse("%s: error: \\<eof>\n", __func__);
				اذهب ret;
			}
			o_addqchr(&dest, ch);
			استمر;
		}
		إذا (ch == '$') {
			إذا (parse_dollar_squote(عدم, &dest, &درون_داد))
				استمر;
			إذا (!parse_dollar(عدم, &dest, &درون_داد, /*quote_mask:*/ 0x80)) {
				debug_printf_parse("%s: error: parse_dollar returned 0 (error)\n", __func__);
				اذهب ret;
			}
			استمر;
		}
#إذا ENABLE_HUSH_TICK
		إذا (ch == '`') {
			//unsigned pos = dest->length;
			o_addchr(&dest, SPECIAL_VAR_نماد);
			o_addchr(&dest, 0x80 | '`');
			إذا (!جمع_کن_till_backquote(&dest, &درون_داد,
					/*in_dquote:*/ dest.o_expflags /* nonzero if EXP_FLAG_ESC_GLOB_CHARS set */
				)
			) {
				اذهب ret; /* error */
			}
			o_addchr(&dest, SPECIAL_VAR_نماد);
			//debug_printf_subst("SUBST RES3 '%s'\n", dest->data + pos);
			استمر;
		}
#پایان_اگر
		o_جمع_کن_Qchr(&dest, ch);
	} /* for (;;) */

	debug_printf_parse("encode: '%s' -> '%s'\n", str, dest.داده);
	exp_str = expand_string_مقصد_string(dest.داده,
			do_unbackslash ? EXP_FLAG_ESC_GLOB_CHARS : 0,
			do_unbackslash
	);
 ret:
	debug_printf_parse("expand: '%s' -> '%s'\n", dest.داده, exp_str);
	o_free(&dest);
	أرجع exp_str;
}

/* Expanding ARG in ${var+ARG}, ${var-ARG}
 */
ساكن NOINLINE صحيح encode_then_append_var_plusminus(o_string *output, صحيح n,
		حرف *str, صحيح dquoted)
{
	بنية درون_str درون_داد;
	o_string dest = NULL_O_STRING;

	إذا (!اولی_special_char_درون_vararg(str)
	 && '\0' == str[strcspn(str, G.ifs)]
	) {
		/* string has no special chars
		 * && string has no $IFS chars
		 */
		إذا (dquoted) {
			/* Prints 1 (quoted expansion is a "" word, not nothing):
			 * set -- "${notexist-}"; echo $#
			 */
			output->has_quoted_part = 1;
		}
		أرجع expand_vars_مقصد_list(output, n, str);
	}

	راهاندازی_string_درون_str(&درون_داد, str);

	لكل (;;) {
		صحيح ch;

		ch = i_getch(&درون_داد);
		debug_printf_parse("%s: ch=%c (%d) escape=%x\n",
				__func__, ch, ch, dest.o_expflags);

		إذا (!dest.o_expflags) {
			إذا (ch == EOF)
				اكسر;
			إذا (!dquoted && !(output->o_expflags & EXP_FLAG_SINGLEWORD) && strchr(G.ifs, ch)) {
				/* PREFIX${x:d${e}f ...} and we met space: expand "d${e}f" and start new word.
				 * do not assume we are at the start of the word (PREFIX above).
				 */
				إذا (dest.داده) {
					n = expand_vars_مقصد_list(output, n, dest.داده);
					o_free_and_مجموعه_NULL(&dest);
					o_addchr(output, '\0');
					n = o_save_ptr(output, n); /* create next word */
				} وإلا
				إذا (output->درازی != o_get_آخرین_ptr(output, n)
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
				استمر;
			}
			إذا (!dquoted && ch == '\'') {
				إذا (!جمع_کن_till_single_quote_dquoted(&dest, &درون_داد))
					اذهب ret; /* error */
				o_addchr(&dest, SPECIAL_VAR_نماد);
				o_addchr(&dest, SPECIAL_VAR_نماد);
				استمر;
			}
		}
		إذا (ch == EOF) {
			syntax_error_unterm_ch('"');
			اذهب ret; /* error */
		}
		إذا (ch == '"') {
			dest.o_expflags ^= EXP_FLAG_ESC_GLOB_CHARS;
			إذا (dest.o_expflags) {
				o_addchr(&dest, SPECIAL_VAR_نماد);
				o_addchr(&dest, SPECIAL_VAR_نماد);
			}
			استمر;
		}
		إذا (ch == '\\') {
			ch = i_getch(&درون_داد);
			إذا (ch == EOF) {
//example? error message?	syntax_error_unterm_ch('"');
				debug_printf_parse("%s: error: \\<eof>\n", __func__);
				اذهب ret;
			}
			o_addqchr(&dest, ch);
			استمر;
		}
		إذا (ch == '$') {
			إذا (!parse_dollar(عدم, &dest, &درون_داد, /*quote_mask:*/ (dest.o_expflags || dquoted) ? 0x80 : 0)) {
				debug_printf_parse("%s: error: parse_dollar returned 0 (error)\n", __func__);
				اذهب ret;
			}
			استمر;
		}
#إذا ENABLE_HUSH_TICK
		إذا (ch == '`') {
			//unsigned pos = dest->length;
			o_addchr(&dest, SPECIAL_VAR_نماد);
			o_addchr(&dest, (dest.o_expflags || dquoted) ? 0x80 | '`' : '`');
			إذا (!جمع_کن_till_backquote(&dest, &درون_داد,
					/*in_dquote:*/ dest.o_expflags /* nonzero if EXP_FLAG_ESC_GLOB_CHARS set */
				)
			) {
				اذهب ret; /* error */
			}
			o_addchr(&dest, SPECIAL_VAR_نماد);
			//debug_printf_subst("SUBST RES3 '%s'\n", dest->data + pos);
			استمر;
		}
#پایان_اگر
		إذا (dquoted) {
			/* Always glob-protect if in dquotes:
			 * x=x; echo "${x:+/bin/c*}" - prints: /bin/c*
			 * x=x; echo "${x:+"/bin/c*"}" - prints: /bin/c*
			 */
			o_addqchr(&dest, ch);
		} وإلا {
			/* Glob-protect only if char is quoted:
			 * x=x; echo ${x:+/bin/c*} - prints many filenames
			 * x=x; echo ${x:+"/bin/c*"} - prints: /bin/c*
			 */
			o_جمع_کن_Qchr(&dest, ch);
		}
	} /* for (;;) */

	إذا (dest.داده) {
		n = expand_vars_مقصد_list(output, n, dest.داده);
	}
 ret:
	o_free(&dest);
	أرجع n;
}
#پایان_اگر /* !__U_BOOT__ */

#اگر_تعریف_نشده __U_BOOT__
#إذا ENABLE_FEATURE_SH_MATH
ساكن arith_t expand_and_evaluate_arith(ثابت حرف *arg, ثابت حرف **errmsg_p)
{
	arith_state_t math_state;
	arith_t res;
	حرف *exp_str;

	math_state.lookupvar = get_local_var_مقدار;
	math_state.setvar = مجموعه_local_var_from_halves;
	//math_state.endofname = endofname;
	exp_str = encode_then_expand_string(arg);
	res = arith(&math_state, exp_str ? exp_str : arg);
	حرر(exp_str);
	إذا (errmsg_p)
		*errmsg_p = math_state.errmsg;
	إذا (math_state.errmsg)
		پیام_and_die_if_script(math_state.errmsg);
	أرجع res;
}
#پایان_اگر
#پایان_اگر /* !__U_BOOT__ */

#اگر_تعریف_نشده __U_BOOT__
#إذا BASH_الگو_SUBST
/* ${var/[/]pattern[/repl]} helpers */
ساكن حرف *strstr_الگو(حرف *val, ثابت حرف *الگو, صحيح *اندازه)
{
	صحيح اولی_escaped = (الگو[0] == '\\' && الگو[1]);
	/* "first_escaped" trick allows to treat e.g. "\*no_glob_chars"
	 * as literal too (as it is semi-common, and easy to accomodate
	 * by just using str + 1).
	 */
	صحيح sz = strcspn(الگو + اولی_escaped * 2, "*?[\\");
	إذا ((الگو + اولی_escaped * 2)[sz] == '\0') {
		/* Optimization for trivial patterns.
		 * Testcase for very slow replace (performs about 22k replaces):
		 * x=::::::::::::::::::::::
		 * x=$x$x;x=$x$x;x=$x$x;x=$x$x;x=$x$x;x=$x$x;x=$x$x;x=$x$x;x=$x$x;x=$x$x;echo ${#x}
		 * echo "${x//:/|}"
		 */
		*اندازه = sz + اولی_escaped;
		أرجع جستجوی_رشته(val, الگو + اولی_escaped);
	}

	طالما (1) {
		حرف *انتها_2 = پیمایش_and_تطابق(val, الگو, پیمایش_MOVE_FROM_راست + پیمایش_تطابق_چپ_HALF);
		debug_printf_varexp("val:'%s' pattern:'%s' end:'%s'\n", val, الگو, انتها_2);
		إذا (انتها_2) {
			*اندازه = انتها_2 - val;
			أرجع val;
		}
		إذا (*val == '\0')
			أرجع عدم;
		/* Optimization: if "*pat" did not match the start of "string",
		 * we know that "tring", "ring" etc will not match too:
		 */
		إذا (الگو[0] == '*')
			أرجع عدم;
		val++;
	}
}
ساكن حرف *replace_الگو(حرف *val, ثابت حرف *الگو, ثابت حرف *repl, حرف exp_op)
{
	حرف *نتیجه = عدم;
	بیعلامت res_len = 0;
	بیعلامت repl_len = طول_رشته(repl);

	/* Null pattern never matches, including if "var" is empty */
	إذا (!الگو[0])
		أرجع نتیجه; /* NULL, no replaces happened */

	طالما (1) {
		صحيح اندازه;
		حرف *s = strstr_الگو(val, الگو, &اندازه);
		إذا (!s)
			اكسر;

		نتیجه = xrealloc(نتیجه, res_len + (s - val) + repl_len + 1);
		strcpy(mempcpy(نتیجه + res_len, val, s - val), repl);
		res_len += (s - val) + repl_len;
		debug_printf_varexp("val:'%s' s:'%s' result:'%s'\n", val, s, نتیجه);

		val = s + اندازه;
		إذا (exp_op == '/')
			اكسر;
	}
	إذا (*val && نتیجه) {
		نتیجه = xrealloc(نتیجه, res_len + طول_رشته(val) + 1);
		strcpy(نتیجه + res_len, val);
		debug_printf_varexp("val:'%s' result:'%s'\n", val, نتیجه);
	}
	debug_printf_varexp("result:'%s'\n", نتیجه);
	أرجع نتیجه;
}
#پایان_اگر /* BASH_PATTERN_SUBST */
#پایان_اگر /* !__U_BOOT__ */

ساكن صحيح append_str_maybe_ifs_تقسیم(o_string *output, صحيح n,
		صحيح اولی_ch, ثابت حرف *val)
{
	إذا (!(اولی_ch & 0x80)) { /* unquoted $VAR */
		debug_printf_expand("unquoted '%s', output->o_escape:%d\n", val,
				!!(output->o_expflags & EXP_FLAG_ESC_GLOB_CHARS));
		إذا (val && val[0])
			n = expand_on_ifs(output, n, val);
	} وإلا { /* quoted "$VAR" */
		output->has_quoted_part = 1;
		debug_printf_expand("quoted '%s', output->o_escape:%d\n", val,
				!!(output->o_expflags & EXP_FLAG_ESC_GLOB_CHARS));
		إذا (val && val[0])
			o_جمع_کن_Qstr(output, val);
	}
	أرجع n;
}

/* Handle <SPECIAL_VAR_SYMBOL>varname...<SPECIAL_VAR_SYMBOL> construct.
 */
ساكن NOINLINE صحيح expand_one_var(o_string *output, صحيح n,
		صحيح اولی_ch, حرف *arg, حرف **pp)
{
	ثابت حرف *val;
	حرف *مقصد_be_freed;
	حرف *p;
	حرف *var;
	حرف exp_op;
	حرف exp_save = exp_save; /* for compiler */
	حرف *exp_saveptr; /* points to expansion operator */
	حرف *exp_کلمه = exp_کلمه; /* for compiler */
	حرف arg0;

	val = عدم;
	مقصد_be_freed = عدم;
	p = *pp;
	*p = '\0'; /* replace trailing SPECIAL_VAR_SYMBOL */
	var = arg;
	exp_saveptr = arg[1] ? strchr(VAR_ENCODED_SUBST_OPS, arg[1]) : عدم;
	arg0 = arg[0];
	arg[0] = (arg0 & 0x7f);
	exp_op = 0;

	إذا (arg[0] == '#' && arg[1] /* ${#...} but not ${#} */
	 && (!exp_saveptr               /* and ( not(${#<op_char>...}) */
	    || (arg[2] == '\0' && strchr(SPECIAL_VARS_STR, arg[1])) /* or ${#C} "len of $C" ) */
	    )		/* NB: skipping ^^^specvar check mishandles ${#::2} */
	) {
		/* It must be length operator: ${#var} */
		var++;
		exp_op = 'L';
	} وإلا {
		/* Maybe handle parameter expansion */
		إذا (exp_saveptr /* if 2nd char is one of expansion operators */
		 && strchr(NUMERIC_SPECVARS_STR, arg[0]) /* 1st char is special variable */
		) {
			/* ${?:0}, ${#[:]%0} etc */
			exp_saveptr = var + 1;
		} وإلا {
			/* ${?}, ${var}, ${var:0}, ${var[:]%0} etc */
			exp_saveptr = var+1 + strcspn(var+1, VAR_ENCODED_SUBST_OPS);
		}
		exp_op = exp_save = *exp_saveptr;
#اگر_تعریف_نشده __U_BOOT__
		إذا (exp_op) {
			exp_کلمه = exp_saveptr + 1;
			إذا (exp_op == ':') {
				exp_op = *exp_کلمه++;
//TODO: try ${var:} and ${var:bogus} in non-bash config
				إذا (BASH_SUBSTR
				 && (!exp_op || !strchr(MINUS_PLUS_EQUAL_QUESTION, exp_op))
				) {
					/* oops... it's ${var:N[:M]}, not ${var:?xxx} or some such */
					exp_op = ':';
					exp_کلمه--;
				}
			}
			*exp_saveptr = '\0';
		} /* else: it's not an expansion op, but bare ${var} */
#پایان_اگر /* !__U_BOOT__ */
	}

	/* Look up the variable in question */
	إذا (آیا_رقم(var[0])) {
		/* parse_dollar should have vetted var for us */
#اگر_تعریف_نشده __U_BOOT__
		صحيح nn = xatoi_مثبت(var);
#وإلا /* __U_BOOT__ */
		صحيح nn = simple_strtoul(var, عدم, 10);
#پایان_اگر /* __U_BOOT__ */
		إذا (nn < G.global_argc)
			val = G.global_argv[nn];
		/* else val remains NULL: $N with too big N */
	} وإلا {
		بدل (var[0]) {
#اگر_تعریف_نشده __U_BOOT__
		حالة '$': /* pid */
			val = utoa(G.ریشه_pid);
			اكسر;
		حالة '!': /* bg pid */
			val = G.آخرین_bg_pid ? utoa(G.آخرین_bg_pid) : "";
			اكسر;
#پایان_اگر /* !__U_BOOT__ */
		حالة '?': /* exitcode */
			val = utoa(G.آخرین_exitcode);
			اكسر;
		حالة '#': /* argc */
			val = utoa(G.global_argc ? G.global_argc-1 : 0);
			اكسر;
#اگر_تعریف_نشده __U_BOOT__
		حالة '-': { /* active options */
			/* Check set_mode() to see what option chars we support */
			حرف *cp;
			val = cp = G.optstring_buf;
			إذا (G.o_opt[OPT_O_ERREXIT])
				*cp++ = 'e';
			إذا (G_interactive_fd)
				*cp++ = 'i';
			إذا (G_x_نما)
				*cp++ = 'x';
			/* If G.o_opt[OPT_O_NOEXEC] is true,
			 * commands read but are not executed,
			 * so $- can not execute too, 'n' is never seen in $-.
			 */
			إذا (G.opt_c)
				*cp++ = 'c';
			إذا (G.opt_s)
				*cp++ = 's';
			*cp = '\0';
			اكسر;
		}
#پایان_اگر /* !__U_BOOT__ */
		افتراضي:
#اگر_تعریف_نشده __U_BOOT__
			val = get_local_var_مقدار(var);
#وإلا /* __U_BOOT__ */
			/*
			 * Environment variable set with setenv* have to be
			 * expanded.
			 * So, we first search if the variable exists in
			 * environment, if this is not the case, we default to
			 * local value.
			 */
			val = env_get(var);
			إذا (!val)
				val = get_local_var_مقدار(var);
#پایان_اگر /* __U_BOOT__ */
		}
	}

#اگر_تعریف_نشده __U_BOOT__
	/* Handle any expansions */
	إذا (exp_op == 'L') {
		reinit_unicode_for_hush();
		debug_printf_expand("expand: length(%s)=", val);
		val = utoa(val ? unicode_strlen(val) : 0);
		debug_printf_expand("%s\n", val);
	} وإلا إذا (exp_op) {
		إذا (exp_op == '%' || exp_op == '#') {
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
			إذا (val && val[0]) {
				حرف *t;
				حرف *exp_exp_کلمه;
				حرف *loc;
				بیعلامت پیمایش_flags = pick_پیمایش(exp_op, *exp_کلمه);
				إذا (exp_op == *exp_کلمه)  /* ## or %% */
					exp_کلمه++;
				debug_printf_expand("expand: exp_word:'%s'\n", exp_کلمه);
				exp_exp_کلمه = encode_then_expand_vararg(exp_کلمه, /*handle_squotes:*/ 1, /*unbackslash:*/ 0);
				إذا (exp_exp_کلمه)
					exp_کلمه = exp_exp_کلمه;
				debug_printf_expand("expand: exp_word:'%s'\n", exp_کلمه);
				/*
				 * HACK ALERT. We depend here on the fact that
				 * G.global_argv and results of utoa and get_local_var_value
				 * are actually in writable memory:
				 * scan_and_match momentarily stores NULs there.
				 */
				t = (حرف*)val;
				loc = پیمایش_and_تطابق(t, exp_کلمه, پیمایش_flags);
				debug_printf_expand("op:%c str:'%s' pat:'%s' res:'%s'\n", exp_op, t, exp_کلمه, loc);
				حرر(exp_exp_کلمه);
				إذا (loc) { /* match was found */
					إذا (پیمایش_flags & پیمایش_تطابق_چپ_HALF) /* #[#] */
						val = loc; /* take right part */
					وإلا /* %[%] */
						val = مقصد_be_freed = xstrndup(val, loc - val); /*چپ*/
				}
			}
		}
#إذا BASH_الگو_SUBST
		وإلا إذا (exp_op == '/' || exp_op == '\\') {
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
			إذا (val /*&& val[0]*/) {
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
				حرف *الگو, *repl, *t;
				الگو = encode_then_expand_vararg(exp_کلمه, /*handle_squotes:*/ 1, /*unbackslash:*/ 0);
				إذا (!الگو)
					الگو = xstrdup(exp_کلمه);
				debug_printf_varexp("pattern:'%s'->'%s'\n", exp_کلمه, الگو);
				*p++ = SPECIAL_VAR_نماد;
				exp_کلمه = p;
				p = strchr(p, SPECIAL_VAR_نماد);
				*p = '\0';
				repl = encode_then_expand_vararg(exp_کلمه, /*handle_squotes:*/ 1, /*unbackslash:*/ 1);
				debug_printf_varexp("repl:'%s'->'%s'\n", exp_کلمه, repl);
				/* HACK ALERT. We depend here on the fact that
				 * G.global_argv and results of utoa and get_local_var_value
				 * are actually in writable memory:
				 * replace_pattern momentarily stores NULs there. */
				t = (حرف*)val;
				مقصد_be_freed = replace_الگو(t,
						الگو,
						(repl ? repl : exp_کلمه),
						exp_op);
				إذا (مقصد_be_freed) /* at least one replace happened */
					val = مقصد_be_freed;
				حرر(الگو);
				حرر(repl);
			} وإلا {
				/* Unset variable always gives nothing */
				//  a=; echo ${a/*/w}      # "w"
				//  unset a; echo ${a/*/w} # ""
				/* Just skip "replace" part */
				*p++ = SPECIAL_VAR_نماد;
				p = strchr(p, SPECIAL_VAR_نماد);
				*p = '\0';
			}
		}
#پایان_اگر /* BASH_PATTERN_SUBST */
		وإلا إذا (exp_op == ':') {
#إذا BASH_SUBSTR && ENABLE_FEATURE_SH_MATH
			/* It's ${var:N[:M]} bashism.
			 * Note that in encoded form it has TWO parts:
			 * var:N<SPECIAL_VAR_SYMBOL>M<SPECIAL_VAR_SYMBOL>
			 */
			arith_t beg, len;
			بیعلامت vallen;
			ثابت حرف *errmsg;

			beg = expand_and_evaluate_arith(exp_کلمه, &errmsg);
			إذا (errmsg)
				اذهب خالی_نتیجه;
			debug_printf_varexp("beg:'%s'=%lld\n", exp_کلمه, (دراز دراز)beg);
			*p++ = SPECIAL_VAR_نماد;
			exp_کلمه = p;
			p = strchr(p, SPECIAL_VAR_نماد);
			*p = '\0';
			vallen = val ? طول_رشته(val) : 0;
			إذا (beg < 0) {
				/* negative beg counts from the end */
				beg = (arith_t)vallen + beg;
			}
			/* If expansion will be empty, do not even evaluate len */
			إذا (!val || beg < 0 || beg > vallen) {
				/* Why > vallen, not >=? bash:
				 * unset b; a=ab; : ${a:2:${b=3}}; echo $b  # "", b=3 (!!!)
				 * unset b; a=a; : ${a:2:${b=3}}; echo $b   # "", b not set
				 */
				اذهب خالی_نتیجه;
			}
			len = expand_and_evaluate_arith(exp_کلمه, &errmsg);
			إذا (errmsg)
				اذهب خالی_نتیجه;
			debug_printf_varexp("len:'%s'=%lld\n", exp_کلمه, (دراز دراز)len);
			debug_printf_varexp("from val:'%s'\n", val);
			إذا (len < 0) {
				/* in bash, len=-n means strlen()-n */
				len = (arith_t)vallen - beg + len;
				إذا (len < 0) /* bash compat */
					پیام_and_die_if_script("%s: substring expression < 0", var);
			}
			إذا (len <= 0 || !val /*|| beg >= vallen*/) {
 خالی_نتیجه:
				val = عدم;
			} وإلا {
				/* Paranoia. What if user entered 9999999999999
				 * which fits in arith_t but not int? */
				إذا (len > بیشترین_صحیح)
					len = بیشترین_صحیح;
				val = مقصد_be_freed = xstrndup(val + beg, len);
			}
			debug_printf_varexp("val:'%s'\n", val);
#وإلا /* not (HUSH_SUBSTR_EXPANSION && FEATURE_SH_MATH) */
			پیام_and_die_if_script("malformed ${%s:...}", var);
			val = عدم;
#پایان_اگر
		} وإلا { /* one of "-=+?" */
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
			صحيح use_کلمه = (!val || ((exp_save == ':') && !val[0]));
			إذا (exp_op == '+')
				use_کلمه = !use_کلمه;
			debug_printf_expand("expand: op:%c (null:%s) test:%i\n", exp_op,
					(exp_save == ':') ? "true" : "false", use_کلمه);
			إذا (use_کلمه) {
				إذا (exp_op == '+' || exp_op == '-') {
					/* ${var+word} - use alternative value */
					/* ${var-word} - use default value */
					n = encode_then_append_var_plusminus(output, n, exp_کلمه,
							/*dquoted:*/ (arg0 & 0x80)
					);
					val = عدم;
				} وإلا {
					/* ${var?word} - indicate error if unset */
					/* ${var=word} - assign and use default value */
					مقصد_be_freed = encode_then_expand_vararg(exp_کلمه,
							/*handle_squotes:*/ !(arg0 & 0x80),
							/*unbackslash:*/ 0
					);
					إذا (مقصد_be_freed)
						exp_کلمه = مقصد_be_freed;
					إذا (exp_op == '?') {
						/* mimic bash message */
						پیام_and_die_if_script("%s: %s",
							var,
							exp_کلمه[0]
							? exp_کلمه
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
					} وإلا {
						val = exp_کلمه;
					}
					إذا (exp_op == '=') {
						/* ${var=[word]} or ${var:=[word]} */
						إذا (آیا_رقم(var[0]) || var[0] == '#') {
							/* mimic bash message */
							پیام_and_die_if_script("$%s: cannot assign in this way", var);
							val = عدم;
						} وإلا {
							حرف *new_var = xasprintf("%s=%s", var, val);
							مجموعه_local_var0(new_var);
						}
					}
				}
			}
		} /* one of "-=+?" */

		*exp_saveptr = exp_save;
	} /* if (exp_op) */

#پایان_اگر /* !__U_BOOT__ */
	arg[0] = arg0;
	*pp = p;

	n = append_str_maybe_ifs_تقسیم(output, n, اولی_ch, val);

	حرر(مقصد_be_freed);
	أرجع n;
}

/* Expand all variable references in given string, adding words to list[]
 * at n, n+1,... positions. Return updated n (so that list[n] is next one
 * to be filled). This routine is extremely tricky: has to deal with
 * variables/parameters with whitespace, $* and $@, and constructs like
 * 'echo -$*-'. If you play here, you must run testsuite afterwards! */
ساكن NOINLINE صحيح expand_vars_مقصد_list(o_string *output, صحيح n, حرف *arg)
{
	/* output->o_expflags & EXP_FLAG_SINGLEWORD (0x80) if we are in
	 * expansion of right-hand side of assignment == 1-element expand.
	 */
	حرف cant_be_null = 0; /* only bit 0x80 matters */
	حرف *p;

	debug_printf_expand("expand_vars_to_list: arg:'%s' singleword:%x\n", arg,
			!!(output->o_expflags & EXP_FLAG_SINGLEWORD));
	debug_print_list("expand_vars_to_list[0]", output, n);

	طالما ((p = strchr(arg, SPECIAL_VAR_نماد)) != عدم) {
		حرف اولی_ch;
#إذا ENABLE_FEATURE_SH_MATH
		حرف arith_buf[حجم(arith_t)*3 + 2];
#پایان_اگر

		إذا (output->ended_درون_ifs) {
			o_addchr(output, '\0');
			n = o_save_ptr(output, n);
			output->ended_درون_ifs = 0;
		}

		o_addblock(output, arg, p - arg);
		debug_print_list("expand_vars_to_list[1]", output, n);
		arg = ++p;
		p = strchr(p, SPECIAL_VAR_نماد);

		/* Fetch special var name (if it is indeed one of them)
		 * and quote bit, force the bit on if singleword expansion -
		 * important for not getting v=$@ expand to many words. */
		اولی_ch = arg[0] | (output->o_expflags & EXP_FLAG_SINGLEWORD);

		/* Is this variable quoted and thus expansion can't be null?
		 * "$@" is special. Even if quoted, it can still
		 * expand to nothing (not even an empty string),
		 * thus it is excluded. */
		إذا ((اولی_ch & 0x7f) != '@')
			cant_be_null |= اولی_ch;

		بدل (اولی_ch & 0x7f) {
		/* Highest bit in first_ch indicates that var is double-quoted */
		حالة '*':
		حالة '@': {
			صحيح i;
#اگر_تعریف_نشده __U_BOOT__
			إذا (!G.global_argv[1])
#وإلا /* __U_BOOT__ */
			إذا (!G.global_argv || !G.global_argv[1])
#پایان_اگر /* __U_BOOT__ */
				اكسر;
			i = 1;
			cant_be_null |= اولی_ch; /* do it for "$@" _now_, when we know it's not empty */
			إذا (!(اولی_ch & 0x80)) { /* unquoted $* or $@ */
				طالما (G.global_argv[i]) {
					n = expand_on_ifs(output, n, G.global_argv[i]);
					debug_printf_expand("expand_vars_to_list: argv %d (last %d)\n", i, G.global_argc - 1);
					إذا (G.global_argv[i++][0] && G.global_argv[i]) {
						/* this argv[] is not empty and not last:
						 * put terminating NUL, start new word */
						o_addchr(output, '\0');
						debug_print_list("expand_vars_to_list[2]", output, n);
						n = o_save_ptr(output, n);
						debug_print_list("expand_vars_to_list[3]", output, n);
					}
				}
			} وإلا
			/* If EXP_FLAG_SINGLEWORD, we handle assignment 'a=....$@.....'
			 * and in this case should treat it like '$*' - see 'else...' below */
			إذا (اولی_ch == (حرف)('@'|0x80)  /* quoted $@ */
			 && !(output->o_expflags & EXP_FLAG_SINGLEWORD) /* not v="$@" case */
			) {
				طالما (1) {
					o_جمع_کن_Qstr(output, G.global_argv[i]);
					إذا (++i >= G.global_argc)
						اكسر;
					o_addchr(output, '\0');
					debug_print_list("expand_vars_to_list[4]", output, n);
					n = o_save_ptr(output, n);
				}
			} وإلا { /* quoted $* (or v="$@" case): add as one word */
				طالما (1) {
					o_جمع_کن_Qstr(output, G.global_argv[i]);
					إذا (!G.global_argv[++i])
						اكسر;
					إذا (G.ifs[0])
						o_addchr(output, G.ifs[0]);
				}
				output->has_quoted_part = 1;
			}
			اكسر;
		}
		حالة SPECIAL_VAR_نماد: {
			/* <SPECIAL_VAR_SYMBOL><SPECIAL_VAR_SYMBOL> */
			/* "Empty variable", used to make "" etc to not disappear */
			output->has_quoted_part = 1;
			cant_be_null = 0x80;
			arg++;
			اكسر;
		}
		حالة SPECIAL_VAR_QUOTED_SVS:
			/* <SPECIAL_VAR_SYMBOL><SPECIAL_VAR_QUOTED_SVS><SPECIAL_VAR_SYMBOL> */
			/* "^C variable", represents literal ^C char (possible in scripts) */
			o_addchr(output, SPECIAL_VAR_نماد);
			arg++;
			اكسر;
#إذا ENABLE_HUSH_TICK
		حالة '`': {
			/* <SPECIAL_VAR_SYMBOL>`cmd<SPECIAL_VAR_SYMBOL> */
			o_string subst_نتیجه = NULL_O_STRING;

			*p = '\0'; /* replace trailing <SPECIAL_VAR_SYMBOL> */
			arg++;
			/* Can't just stuff it into output o_string,
			 * expanded result may need to be globbed
			 * and $IFS-split */
			debug_printf_subst("SUBST '%s' first_ch %x\n", arg, اولی_ch);
			G.آخرین_exitcode = process_command_subs(&subst_نتیجه, arg);
			G.expand_exitcode = G.آخرین_exitcode;
			debug_printf_subst("SUBST RES:%d '%s'\n", G.آخرین_exitcode, subst_نتیجه.داده);
			n = append_str_maybe_ifs_تقسیم(output, n, اولی_ch, subst_نتیجه.داده);
			o_free(&subst_نتیجه);
			اكسر;
		}
#پایان_اگر
#إذا ENABLE_FEATURE_SH_MATH
		حالة '+': {
			/* <SPECIAL_VAR_SYMBOL>+arith<SPECIAL_VAR_SYMBOL> */
			arith_t res;

			arg++; /* skip '+' */
			*p = '\0'; /* replace trailing <SPECIAL_VAR_SYMBOL> */
			debug_printf_subst("ARITH '%s' first_ch %x\n", arg, اولی_ch);
			res = expand_and_evaluate_arith(arg, عدم);
			debug_printf_subst("ARITH RES '"ARITH_FMT"'\n", res);
			sprintf(arith_buf, ARITH_FMT, res);
			إذا (res < 0
			 && اولی_ch == (حرف)('+'|0x80)
			/* && (output->o_expflags & EXP_FLAG_ESC_GLOB_CHARS) */
			) {
				/* Quoted negative ariths, like filename[0"$((-9))"],
				 * should not be interpreted as glob ranges.
				 * Convert leading '-' to '\-':
				 */
				o_grow_by(output, 1);
				output->داده[output->درازی++] = '\\';
			}
			o_addstr(output, arith_buf);
			اكسر;
		}
#پایان_اگر
		افتراضي:
			/* <SPECIAL_VAR_SYMBOL>varname[ops]<SPECIAL_VAR_SYMBOL> */
			n = expand_one_var(output, n, اولی_ch, arg, &p);
			اكسر;
		} /* switch (char after <SPECIAL_VAR_SYMBOL>) */

		/* Restore NULL'ed SPECIAL_VAR_SYMBOL.
		 * Do the check to avoid writing to a const string. */
		إذا (*p != SPECIAL_VAR_نماد)
			*p = SPECIAL_VAR_نماد;
		arg = ++p;
	} /* end of "while (SPECIAL_VAR_SYMBOL is found) ..." */

	إذا (*arg) {
		/* handle trailing string */
		إذا (output->ended_درون_ifs) {
			o_addchr(output, '\0');
			n = o_save_ptr(output, n);
		}
		debug_print_list("expand_vars_to_list[a]", output, n);
		/* this part is literal, and it was already pre-quoted
		 * if needed (much earlier), do not use o_addQstr here!
		 */
		o_addstr(output, arg);
		debug_print_list("expand_vars_to_list[b]", output, n);
	} وإلا
	إذا (output->درازی == o_get_آخرین_ptr(output, n) /* expansion is empty */
	 && !(cant_be_null & 0x80)   /* and all vars were not quoted */
	 && !output->has_quoted_part
	) {
		n--;
		/* allow to reuse list[n] later without re-growth */
		output->has_خالی_جایگاه = 1;
	}

	أرجع n;
}

ساكن حرف **expand_variables(حرف **argv, بیعلامت expflags)
{
	صحيح n;
	حرف **list;
	o_string output = NULL_O_STRING;

	output.o_expflags = expflags;

	n = 0;
	لكل (;;) {
		/* go to next list[n] */
		output.ended_درون_ifs = 0;
		n = o_save_ptr(&output, n);

		إذا (!*argv)
			اكسر;

		/* expand argv[i] */
		n = expand_vars_مقصد_list(&output, n, *argv++);
		/* if (!output->has_empty_slot) -- need this?? */
			o_addchr(&output, '\0');
	}
	debug_print_list("expand_variables", &output, n);

	/* output.data (malloced in one block) gets returned in "list" */
	list = o_finalize_list(&output, n);
	debug_print_رشتهها("expand_variables[1]", list);
	أرجع list;
}

ساكن حرف **expand_strvec_مقصد_strvec(حرف **argv)
{
	أرجع expand_variables(argv, EXP_FLAG_GLOB | EXP_FLAG_ESC_GLOB_CHARS);
}

#إذا defined(CMD_SINGLEWORD_NOGLOB) || defined(CMD_TEST2_SINGLEWORD_NOGLOB)
ساكن حرف **expand_strvec_مقصد_strvec_singleword_noglob(حرف **argv)
{
	أرجع expand_variables(argv, EXP_FLAG_SINGLEWORD);
}
#پایان_اگر

/* Used for expansion of right hand of assignments,
 * $((...)), heredocs, variable expansion parts.
 *
 * NB: should NOT do globbing!
 * "export v=/bin/c*; env | grep ^v=" outputs "v=/bin/c*"
 */
ساكن حرف *expand_string_مقصد_string(ثابت حرف *str, صحيح EXP_flags, صحيح do_unbackslash)
{
#إذا !BASH_الگو_SUBST && !ENABLE_HUSH_CASE
	ثابت صحيح do_unbackslash = 1;
	ثابت صحيح EXP_flags = EXP_FLAG_ESC_GLOB_CHARS;
#پایان_اگر
	حرف *argv[2], **list;

	debug_printf_expand("string_to_string<='%s'\n", str);
	/* This is generally an optimization, but it also
	 * handles "", which otherwise trips over !list[0] check below.
	 * (is this ever happens that we actually get str="" here?)
	 */
	إذا (!strchr(str, SPECIAL_VAR_نماد) && !strchr(str, '\\')) {
		//TODO: Can use on strings with \ too, just unbackslash() them?
		debug_printf_expand("string_to_string(fast)=>'%s'\n", str);
		أرجع xstrdup(str);
	}

	argv[0] = (حرف*)str;
	argv[1] = عدم;
	list = expand_variables(argv, EXP_flags | EXP_FLAG_SINGLEWORD);
	إذا (!list[0]) {
		/* Example where it happens:
		 * x=; echo ${x:-"$@"}
		 */
		((حرف*)list)[0] = '\0';
	} وإلا {
		إذا (HUSH_DEBUG)
			إذا (list[1])
				bb_simple_error_پیام_and_die("BUG in varexp2");
		/* actually, just move string 2*sizeof(char*) bytes back */
		overlapping_strcpy((حرف*)list, list[0]);
		إذا (do_unbackslash)
			unbackslash((حرف*)list);
	}
	debug_printf_expand("string_to_string=>'%s'\n", (حرف*)list);
	أرجع (حرف*)list;
}

#إذا 0
ساكن حرف* expand_strvec_مقصد_string(حرف **argv)
{
	حرف **list;

	list = expand_variables(argv, EXP_FLAG_SINGLEWORD);
	/* Convert all NULs to spaces */
	إذا (list[0]) {
		صحيح n = 1;
		طالما (list[n]) {
			إذا (HUSH_DEBUG)
				إذا (list[n-1] + طول_رشته(list[n-1]) + 1 != list[n])
					bb_error_پیام_and_die("BUG in varexp3");
			/* bash uses ' ' regardless of $IFS contents */
			list[n][-1] = ' ';
			n++;
		}
	}
	overlapping_strcpy((حرف*)list, list[0] ? list[0] : "");
	debug_printf_expand("strvec_to_string='%s'\n", (حرف*)list);
	أرجع (حرف*)list;
}
#پایان_اگر

#اگر_تعریف_نشده __U_BOOT__
ساكن حرف **expand_assignments(حرف **argv, صحيح شمار)
{
	صحيح i;
	حرف **p;

	G.expanded_assignments = p = عدم;
	/* Expand assignments into one string each */
	لكل (i = 0; i < شمار; i++) {
		p = جمع_کن_string_مقصد_رشتهها(p,
			expand_string_مقصد_string(argv[i],
				EXP_FLAG_ESC_GLOB_CHARS,
				/*unbackslash:*/ 1
			)
		);
		G.expanded_assignments = p;
	}
	G.expanded_assignments = عدم;
	أرجع p;
}

ساكن فراغ switch_off_special_sigs(بیعلامت ماسک)
{
	بیعلامت sig = 0;
	طالما ((ماسک >>= 1) != 0) {
		sig++;
		إذا (!(ماسک & 1))
			استمر;
#إذا ENABLE_HUSH_TRAP
		إذا (G_traps) {
			إذا (G_traps[sig] && !G_traps[sig][0])
				/* trap is '', has to remain SIG_IGN */
				استمر;
			حرر(G_traps[sig]);
			G_traps[sig] = عدم;
		}
#پایان_اگر
		/* We are here only if no trap or trap was not '' */
		install_sighandler(sig, SIG_DFL);
	}
}
#پایان_اگر /* !__U_BOOT__ */

#اگر_تعریف_نشده __U_BOOT__
#إذا BB_MMU
/* never called */
فراغ re_execute_shell(حرف ***مقصد_free, ثابت حرف *s,
		حرف *g_argv0, حرف **g_argv,
		حرف **builtin_argv) NORETURN;

ساكن فراغ reset_traps_مقصد_defaults(فراغ)
{
	/* This function is always called in a child shell
	 * after fork (not vfork, NOMMU doesn't use this function).
	 */
	IF_HUSH_TRAP(بیعلامت sig;)
	بیعلامت ماسک;

	/* Child shells are not interactive.
	 * SIGTTIN/SIGTTOU/SIGTSTP should not have special handling.
	 * Testcase: (while :; do :; done) + ^Z should background.
	 * Same goes for SIGTERM, SIGHUP, SIGINT.
	 */
	ماسک = (G.special_sig_ماسک & SPECIAL_INTERACTIVE_SIGS) | G_fatal_sig_ماسک;
	إذا (!G_traps && !ماسک)
		أرجع; /* already no traps and no special sigs */

	/* Switch off special sigs */
	switch_off_special_sigs(ماسک);
# إذا ENABLE_HUSH_کار
	G_fatal_sig_ماسک = 0;
# endif
	G.special_sig_ماسک &= ~SPECIAL_INTERACTIVE_SIGS;
	/* SIGQUIT,SIGCHLD and maybe SPECIAL_JOBSTOP_SIGS
	 * remain set in G.special_sig_mask */

# إذا ENABLE_HUSH_TRAP
	إذا (!G_traps)
		أرجع;

	/* Reset all sigs to default except ones with empty traps */
	لكل (sig = 0; sig < NSIG; sig++) {
		إذا (!G_traps[sig])
			استمر; /* no trap: nothing to do */
		إذا (!G_traps[sig][0])
			استمر; /* empty trap: has to remain SIG_IGN */
		/* sig has non-empty trap, reset it: */
		حرر(G_traps[sig]);
		G_traps[sig] = عدم;
		/* There is no signal for trap 0 (EXIT) */
		إذا (sig == 0)
			استمر;
		install_sighandler(sig, pick_sighandler(sig));
	}
# endif
}

#وإلا /* !BB_MMU */

ساكن فراغ re_execute_shell(حرف ***مقصد_free, ثابت حرف *s,
		حرف *g_argv0, حرف **g_argv,
		حرف **builtin_argv) NORETURN;
ساكن فراغ re_execute_shell(حرف ***مقصد_free, ثابت حرف *s,
		حرف *g_argv0, حرف **g_argv,
		حرف **builtin_argv)
{
# define NOMMU_HACK_FMT ("-$%x:%x:%x:%x:%x:%llx" IF_HUSH_LOOPS(":%x"))
	/* delims + 2 * (number of bytes in printed hex numbers) */
	حرف param_buf[حجم(NOMMU_HACK_FMT) + 2 * (حجم(صحيح)*6 + حجم(دراز دراز)*1)];
	حرف *heredoc_argv[4];
	بنية variable *cur;
# إذا ENABLE_HUSH_FUNCTIONS
	بنية function *funcp;
# endif
	حرف **argv, **pp;
	بیعلامت cnt;
	بیعلامت دراز دراز خالی_trap_ماسک;

	إذا (!g_argv0) { /* heredoc */
		argv = heredoc_argv;
		argv[0] = (حرف *) G.argv0_for_re_execing;
		argv[1] = (حرف *) "-<";
		argv[2] = (حرف *) s;
		argv[3] = عدم;
		pp = &argv[3]; /* used as pointer to empty environment */
		اذهب do_exec;
	}

	cnt = 0;
	pp = builtin_argv;
	إذا (pp) طالما (*pp++)
		cnt++;

	خالی_trap_ماسک = 0;
	إذا (G_traps) {
		صحيح sig;
		لكل (sig = 1; sig < NSIG; sig++) {
			إذا (G_traps[sig] && !G_traps[sig][0])
				خالی_trap_ماسک |= 1LL << sig;
		}
	}

	sprintf(param_buf, NOMMU_HACK_FMT
			, (بیعلامت) G.ریشه_pid
			, (بیعلامت) G.ریشه_ppid
			, (بیعلامت) G.آخرین_bg_pid
			, (بیعلامت) G.آخرین_exitcode
			, cnt
			, خالی_trap_ماسک
			IF_HUSH_LOOPS(, G.عمق_از_loop)
			);
# undef NOMMU_HACK_FMT
	/* 1:hush 2:-$<pid>:<pid>:<exitcode>:<etc...> <vars...> <funcs...>
	 * 3:-c 4:<cmd> 5:<arg0> <argN...> 6:NULL
	 */
	cnt += 6;
	لكل (cur = G.بالاترین_var; cur; cur = cur->بعدی) {
		إذا (!cur->flg_export || cur->flg_read_only)
			cnt += 2;
	}
# إذا ENABLE_HUSH_LINENO_VAR
	cnt += 2;
# endif
# إذا ENABLE_HUSH_FUNCTIONS
	لكل (funcp = G.بالاترین_func; funcp; funcp = funcp->بعدی)
		cnt += 3;
# endif
	pp = g_argv;
	طالما (*pp++)
		cnt++;
	*مقصد_free = argv = pp = xzalloc(حجم(argv[0]) * cnt);
	*pp++ = (حرف *) G.argv0_for_re_execing;
	*pp++ = param_buf;
	لكل (cur = G.بالاترین_var; cur; cur = cur->بعدی) {
		إذا (strcmp(cur->varstr, hush_نسخه_str) == 0)
			استمر;
		إذا (cur->flg_read_only) {
			*pp++ = (حرف *) "-R";
			*pp++ = cur->varstr;
		} وإلا إذا (!cur->flg_export) {
			*pp++ = (حرف *) "-V";
			*pp++ = cur->varstr;
		}
	}
# إذا ENABLE_HUSH_LINENO_VAR
	*pp++ = (حرف *) "-L";
	*pp++ = utoa(G.execute_lineno);
# endif
# إذا ENABLE_HUSH_FUNCTIONS
	لكل (funcp = G.بالاترین_func; funcp; funcp = funcp->بعدی) {
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
	إذا (builtin_argv) {
		طالما (*++builtin_argv)
			*pp++ = *builtin_argv;
		*pp++ = (حرف *) "";
	}
	*pp++ = g_argv0;
	طالما (*g_argv)
		*pp++ = *g_argv++;
	/* *pp = NULL; - is already there */
	pp = environ;

 do_exec:
	debug_printf_exec("re_execute_shell pid:%d cmd:'%s'\n", getpid(), s);
	/* Don't propagate SIG_IGN to the child */
	إذا (SPECIAL_JOBSTOP_SIGS != 0)
		switch_off_special_sigs(G.special_sig_ماسک & SPECIAL_JOBSTOP_SIGS);
	execve(bb_busybox_exec_مسیر, argv, pp);
	/* Fallback. Useful for init=/bin/hush usage etc */
	إذا (argv[0][0] == '/')
		execve(argv[0], argv, pp);
	xfunc_error_retval = 127;
	bb_simple_error_پیام_and_die("can't re-execute the shell");
}
#پایان_اگر  /* !BB_MMU */

#پایان_اگر /* !__U_BOOT__ */

ساكن صحيح پیاپی_and_free_list(بنية pipe *pi);

/* Executing from string: eval, sh -c '...'
 *          or from file: /etc/profile, . file, sh <script>, sh (intereactive)
 * end_trigger controls how often we stop parsing
 * NUL: parse all, execute, return
 * ';': parse till ';' or newline, execute, repeat till EOF
 */
#اگر_تعریف_نشده __U_BOOT__
ساكن فراغ parse_and_پیاپی_جریان(بنية درون_str *inp, صحيح انتها_trigger)
#وإلا /* __U_BOOT__ */
ساكن صحيح parse_and_پیاپی_جریان(بنية درون_str *inp, صحيح انتها_trigger)
#پایان_اگر /* __U_BOOT__ */
{
	/* Why we need empty flag?
	 * An obscure corner case "false; ``; echo $?":
	 * empty command in `` should still set $? to 0.
	 * But we can't just set $? to 0 at the start,
	 * this breaks "false; echo `echo $?`" case.
	 */
	bool خالی = 1;
#اگر_تعریف_نشده __U_BOOT__
	طالما (1) {
#وإلا /* __U_BOOT__ */
	افعل {
#پایان_اگر /* __U_BOOT__ */
		بنية pipe *pipe_list;

#إذا ENABLE_HUSH_INTERACTIVE
		إذا (انتها_trigger == ';') {
			G.promptmode = 0; /* PS1 */
			debug_printf_prompt("%s promptmode=%d\n", __func__, G.promptmode);
		}
#پایان_اگر
		pipe_list = parse_جریان(عدم, عدم, inp, انتها_trigger);
		إذا (!pipe_list || pipe_list == ERR_PTR) { /* EOF/error */
			/* If we are in "big" script
			 * (not in `cmd` or something similar)...
			 */
			إذا (pipe_list == ERR_PTR && انتها_trigger == ';') {
				/* Discard cached input (rest of line) */
				صحيح ch = inp->آخرین_char;
				طالما (ch != EOF && ch != '\n') {
					//bb_error_msg("Discarded:'%c'", ch);
					ch = i_getch(inp);
				}
				/* Force prompt */
				inp->p = عدم;
				/* This stream isn't empty */
				خالی = 0;
				استمر;
			}
			إذا (!pipe_list && خالی)
				G.آخرین_exitcode = 0;
			اكسر;
		}
		debug_print_درخت(pipe_list, 0);
		debug_printf_exec("parse_and_run_stream: run_and_free_list\n");
#اگر_تعریف_نشده __U_BOOT__
		پیاپی_and_free_list(pipe_list);
#وإلا /* __U_BOOT__ */
		صحيح rcode = پیاپی_and_free_list(pipe_list);
		/*
		 * We reset input string to not run the following command, so running
		 * 'exit; echo foo' does not print foo.
		 */
		إذا (rcode <= EXIT_RET_کد)
			راهاندازی_file_درون_str(inp);
#پایان_اگر /* __U_BOOT__ */
		خالی = 0;
		إذا (G_flag_return_درون_پیشرفت == 1)
			اكسر;
#اگر_تعریف_نشده __U_BOOT__
	}
#وإلا /* __U_BOOT__ */
	/*
	 * This do/while is needed by run_command to avoid looping on a command
	 * with syntax error.
	 */
	} طالما (!(G.پیاپی_command_flags & FLAG_EXIT_FROM_LOOP));

	أرجع G.آخرین_exitcode;
#پایان_اگر /* __U_BOOT__ */
}

#اگر_تعریف_نشده __U_BOOT__
ساكن فراغ parse_and_پیاپی_string(ثابت حرف *s)
#وإلا /* __U_BOOT__ */
ساكن صحيح parse_and_پیاپی_string(ثابت حرف *s)
#پایان_اگر /* __U_BOOT__ */
{
	بنية درون_str درون_داد;
#اگر_تعریف_نشده __U_BOOT__
	IF_HUSH_LINENO_VAR(بیعلامت sv = G.parse_lineno;)
#وإلا /* __U_BOOT__ */
	//IF_HUSH_LINENO_VAR(unsigned sv = G.parse_lineno;)
#پایان_اگر /* __U_BOOT__ */

	راهاندازی_string_درون_str(&درون_داد, s);
#اگر_تعریف_نشده __U_BOOT__
	parse_and_پیاپی_جریان(&درون_داد, '\0');
#وإلا /* __U_BOOT__ */
	أرجع parse_and_پیاپی_جریان(&درون_داد, '\0');
#پایان_اگر /* __U_BOOT__ */
#اگر_تعریف_نشده __U_BOOT__
	IF_HUSH_LINENO_VAR(بیعلامت sv = G.parse_lineno;)
#وإلا /* __U_BOOT__ */
	//IF_HUSH_LINENO_VAR(unsigned sv = G.parse_lineno;)
#پایان_اگر /* __U_BOOT__ */
}

#اگر_تعریف_شده __U_BOOT__
صحيح parse_string_outer_modern(ثابت حرف *cmd, صحيح flags)
{
	صحيح ret;
	صحيح old_flags;

	/*
	 * Keep old values of run_command to be able to restore them once
	 * command was executed.
	 */
	old_flags = G.پیاپی_command_flags;
	G.پیاپی_command_flags = flags;

	ret = parse_and_پیاپی_string(cmd);

	G.پیاپی_command_flags = old_flags;

	أرجع ret;
}
#پایان_اگر /* __U_BOOT__ */
#اگر_تعریف_نشده __U_BOOT__
ساكن فراغ parse_and_پیاپی_file(HFILE *fp)
#وإلا /* __U_BOOT__ */
فراغ parse_and_پیاپی_file(فراغ)
#پایان_اگر /* __U_BOOT__ */
{
	بنية درون_str درون_داد;
#اگر_تعریف_نشده __U_BOOT__
	IF_HUSH_LINENO_VAR(بیعلامت sv = G.parse_lineno;)

	IF_HUSH_LINENO_VAR(G.parse_lineno = 1;)
	راهاندازی_file_درون_str(&درون_داد, fp);
#وإلا /* __U_BOOT__ */
	راهاندازی_file_درون_str(&درون_داد);
#پایان_اگر /* __U_BOOT__ */
	parse_and_پیاپی_جریان(&درون_داد, ';');
#اگر_تعریف_نشده __U_BOOT__
	IF_HUSH_LINENO_VAR(G.parse_lineno = sv;)
#پایان_اگر /* !__U_BOOT__ */
}

#اگر_تعریف_نشده __U_BOOT__
#إذا ENABLE_HUSH_TICK
ساكن صحيح generate_جریان_from_string(ثابت حرف *s, pid_t *pid_p)
{
	pid_t pid;
	صحيح channel[2];
# إذا !BB_MMU
	حرف **مقصد_free = عدم;
# endif

	xpipe(channel);
	pid = BB_MMU ? xfork() : xvfork();
	إذا (pid == 0) { /*فرزند*/
		disable_restore_tty_pgrp_on_exit();
		/* Process substitution is not considered to be usual
		 * 'command execution'.
		 * SUSv3 says ctrl-Z should be ignored, ctrl-C should not.
		 */
		bb_سیگنالها(0
			+ (1 << SIGTSTP)
			+ (1 << SIGTTIN)
			+ (1 << SIGTTOU)
			, SIG_IGN);
		close(channel[0]); /* NB: close _first_, then move fd! */
		xmove_fd(channel[1], 1);
# إذا ENABLE_HUSH_TRAP
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
		إذا (is_prefixed_with(s, "trap")
		 && skip_whitespace(s + 4)[0] == '\0'
		) {
			ساكن ثابت حرف *ثابت argv[] ALIGN_PTR = { عدم, عدم };
			builtin_trap((حرف**)argv);
			fflush_all(); /* important */
			_exit(0);
		}
# endif
# إذا BB_MMU
		/* Prevent it from trying to handle ctrl-z etc */
		IF_HUSH_کار(G.پیاپی_list_level = 1;)
		CLEAR_RANDOM_T(&G.random_gen); /* or else $RANDOM repeats in child */
		reset_traps_مقصد_defaults();
		IF_HUSH_نما_X(G.x_نما_عمق++;)
		//bb_error_msg("%s: ++x_mode_depth=%d", __func__, G.x_mode_depth);
		parse_and_پیاپی_string(s);
		_exit(G.آخرین_exitcode);
# وإلا
	/* We re-execute after vfork on NOMMU. This makes this script safe:
	 * yes "0123456789012345678901234567890" | dd bs=32 count=64k >BIG
	 * huge=`cat BIG` # was blocking here forever
	 * echo OK
	 */
		re_execute_shell(&مقصد_free,
				s,
				G.global_argv[0],
				G.global_argv + 1,
				عدم);
# endif
	}

	/*والد*/
	*pid_p = pid;
# إذا ENABLE_HUSH_FAST
	G.شمار_SIGCHLD++;
//bb_error_msg("[%d] fork in generate_stream_from_string:"
//		" G.count_SIGCHLD:%d G.handled_SIGCHLD:%d",
//		getpid(), G.count_SIGCHLD, G.handled_SIGCHLD);
# endif
	enable_restore_tty_pgrp_on_exit();
# إذا !BB_MMU
	حرر(مقصد_free);
# endif
	close(channel[1]);
	أرجع channel[0];
}

/* Return code is exit status of the process that is run. */
ساكن صحيح process_command_subs(o_string *dest, ثابت حرف *s)
{
	FILE *fp;
	pid_t pid;
	صحيح وضعیت, ch, eol_cnt;

	fp = xfdopen_for_read(generate_جریان_from_string(s, &pid));

	/* Now send results of command back into original context */
	eol_cnt = 0;
	طالما ((ch = getc(fp)) != EOF) {
		إذا (ch == '\0')
			استمر;
		إذا (ch == '\n') {
			eol_cnt++;
			استمر;
		}
		طالما (eol_cnt) {
			o_addchr(dest, '\n');
			eol_cnt--;
		}
		o_جمع_کن_Qchr(dest, ch);
	}

	debug_printf("done reading from `cmd` pipe, closing it\n");
	fclose(fp);
	/* We need to extract exitcode. Test case
	 * "true; echo `sleep 1; false` $?"
	 * should print 1 */
	امن_waitpid(pid, &وضعیت, 0);
	debug_printf("child exited. returning its exitcode:%d\n", WEXITSTATUS(وضعیت));
	أرجع WEXITSTATUS(وضعیت);
}
#پایان_اگر /* ENABLE_HUSH_TICK */

ساكن فراغ راهاندازی_heredoc(بنية redir_struct *redir)
{
	بنية fd_pair pair;
	pid_t pid;
	صحيح len, written;
	/* the _body_ of heredoc (misleading field name) */
	ثابت حرف *heredoc = redir->rd_filename;
	حرف *expanded;
#إذا !BB_MMU
	حرف **مقصد_free;
#پایان_اگر

	expanded = عدم;
	إذا (!(redir->rd_dup & HEREDOC_QUOTED)) {
		expanded = encode_then_expand_string(heredoc);
		إذا (expanded)
			heredoc = expanded;
	}
	len = طول_رشته(heredoc);

	close(redir->rd_fd); /* often saves dup2+close in xmove_fd */
	xpiped_pair(pair);
	xmove_fd(pair.rd, redir->rd_fd);

	/* Try writing without forking. Newer kernels have
	 * dynamically growing pipes. Must use non-blocking write! */
	ndelay_on(pair.wr);
	طالما (1) {
		written = اكتب(pair.wr, heredoc, len);
		إذا (written <= 0)
			اكسر;
		len -= written;
		إذا (len == 0) {
			close(pair.wr);
			حرر(expanded);
			أرجع;
		}
		heredoc += written;
	}
	ndelay_off(pair.wr);

	/* Okay, pipe buffer was not big enough */
	/* Note: we must not create a stray child (bastard? :)
	 * for the unsuspecting parent process. Child creates a grandchild
	 * and exits before parent execs the process which consumes heredoc
	 * (that exec happens after we return from this function) */
#إذا !BB_MMU
	مقصد_free = عدم;
#پایان_اگر
	pid = xvfork();
	إذا (pid == 0) {
		/*فرزند*/
		disable_restore_tty_pgrp_on_exit();
		pid = BB_MMU ? xfork() : xvfork();
		إذا (pid != 0)
			_exit(0);
		/* grandchild */
		close(redir->rd_fd); /* read side of the pipe */
#إذا BB_MMU
		پر_write(pair.wr, heredoc, len); /* may loop or block */
		_exit(0);
#وإلا
		/* Delegate blocking writes to another process */
		xmove_fd(pair.wr, STDOUT_FILENO);
		re_execute_shell(&مقصد_free, heredoc, عدم, عدم, عدم);
#پایان_اگر
	}
	/*والد*/
#إذا ENABLE_HUSH_FAST
	G.شمار_SIGCHLD++;
//bb_error_msg("[%d] fork in setup_heredoc: G.count_SIGCHLD:%d G.handled_SIGCHLD:%d", getpid(), G.count_SIGCHLD, G.handled_SIGCHLD);
#پایان_اگر
	enable_restore_tty_pgrp_on_exit();
#إذا !BB_MMU
	حرر(مقصد_free);
#پایان_اگر
	close(pair.wr);
	حرر(expanded);
	wait(عدم); /* wait till child has died */
}

بنية squirrel {
	صحيح orig_fd;
	صحيح moved_مقصد;
	/* moved_to = n: fd was moved to n; restore back to orig_fd after redir */
	/* moved_to = -1: fd was opened by redirect; close orig_fd after redir */
};

ساكن بنية squirrel *append_squirrel(بنية squirrel *sq, صحيح i, صحيح orig, صحيح moved)
{
	sq = xrealloc(sq, (i + 2) * حجم(sq[0]));
	sq[i].orig_fd = orig;
	sq[i].moved_مقصد = moved;
	sq[i+1].orig_fd = -1; /* end marker */
	أرجع sq;
}

ساكن بنية squirrel *جمع_کن_squirrel(بنية squirrel *sq, صحيح fd, صحيح avoid_fd)
{
	صحيح moved_مقصد;
	صحيح i;

	i = 0;
	إذا (sq) لكل (; sq[i].orig_fd >= 0; i++) {
		/* If we collide with an already moved fd... */
		إذا (fd == sq[i].moved_مقصد) {
			moved_مقصد = dup_CLOEXEC(sq[i].moved_مقصد, avoid_fd);
			debug_printf_redir("redirect_fd %d: already busy, moving to %d\n", fd, moved_مقصد);
			إذا (moved_مقصد < 0) {
				/* "echo 2>/dev/tty 10>&9999" testcase:
				 * We move fd 2 to 10, then discover we need to move fd 10
				 * (and not hit 9999) and the latter fails.
				 */
				أرجع عدم; /* fcntl failed */
			}
			sq[i].moved_مقصد = moved_مقصد;
			أرجع sq;
		}
		إذا (fd == sq[i].orig_fd) {
			/* Example: echo Hello >/dev/null 1>&2 */
			debug_printf_redir("redirect_fd %d: already moved\n", fd);
			أرجع sq;
		}
	}

	/* If this fd is open, we move and remember it; if it's closed, moved_to = -1 */
	moved_مقصد = dup_CLOEXEC(fd, avoid_fd);
	debug_printf_redir("redirect_fd %d: previous fd is moved to %d (-1 if it was closed)\n", fd, moved_مقصد);
	إذا (moved_مقصد < 0 && errno != EBADF)
		أرجع عدم; /* fcntl failed (not because fd is closed) */
	أرجع append_squirrel(sq, i, fd, moved_مقصد);
}

ساكن بنية squirrel *جمع_کن_squirrel_closed(بنية squirrel *sq, صحيح fd)
{
	صحيح i;

	i = 0;
	إذا (sq) لكل (; sq[i].orig_fd >= 0; i++) {
		/* If we collide with an already moved fd... */
		إذا (fd == sq[i].orig_fd) {
			/* Examples:
			 * "echo 3>FILE 3>&- 3>FILE"
			 * "echo 3>&- 3>FILE"
			 * No need for last redirect to insert
			 * another "need to close 3" indicator.
			 */
			debug_printf_redir("redirect_fd %d: already moved or closed\n", fd);
			أرجع sq;
		}
	}

	debug_printf_redir("redirect_fd %d: previous fd was closed\n", fd);
	أرجع append_squirrel(sq, i, fd, -1);
}

/* fd: redirect wants this fd to be used (e.g. 3>file).
 * Move all conflicting internally used fds,
 * and remember them so that we can restore them later.
 */
ساكن صحيح save_fd_on_redirect(صحيح fd, صحيح avoid_fd, بنية squirrel **sqp)
{
	بنية squirrel *new_squirrel;

	إذا (avoid_fd < 9) /* the important case here is that it can be -1 */
		avoid_fd = 9;

#إذا ENABLE_HUSH_INTERACTIVE
	إذا (fd != 0 /* don't trigger for G_interactive_fd == 0 (that's "not interactive" flag) */
	 && fd == G_interactive_fd
	) {
		/* Testcase: "ls -l /proc/$$/fd 255>&-" should work */
		G_interactive_fd = xdup_CLOEXEC_and_close(G_interactive_fd, avoid_fd);
		debug_printf_redir("redirect_fd %d: matches interactive_fd, moving it to %d\n", fd, G_interactive_fd);
		أرجع 1; /* "we closed fd" */
	}
#پایان_اگر
	/* Are we called from setup_redirects(squirrel==NULL)
	 * in redirect in a [v]forked child?
	 */
	إذا (sqp == عدم) {
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
		أرجع 0;
	}

	/* If this one of script's fds? */
	إذا (move_HFILEs_on_redirect(fd, avoid_fd))
		أرجع 1; /* yes. "we closed fd" (actually moved it) */

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
	إذا (sqp == ERR_PTR) {
		/* Don't preserve redirected fds: exec is _meant_ to change these */
		//bb_error_msg("sqp == ERR_PTR: exec >FILE");
		أرجع 0;
	}

	/* Check whether it collides with any open fds (e.g. stdio), save fds as needed */
	new_squirrel = جمع_کن_squirrel(*sqp, fd, avoid_fd);
	إذا (!new_squirrel)
		أرجع -1; /* redirect error */
	*sqp = new_squirrel;
	أرجع 0; /* "we did not close fd" */
}

ساكن فراغ restore_redirects(بنية squirrel *sq)
{
	إذا (sq) {
		صحيح i;
		لكل (i = 0; sq[i].orig_fd >= 0; i++) {
			إذا (sq[i].moved_مقصد >= 0) {
				/* We simply die on error */
				debug_printf_redir("restoring redirected fd from %d to %d\n", sq[i].moved_مقصد, sq[i].orig_fd);
				xmove_fd(sq[i].moved_مقصد, sq[i].orig_fd);
			} وإلا {
				/* cmd1 9>FILE; cmd2_should_see_fd9_closed */
				debug_printf_redir("restoring redirected fd %d: closing it\n", sq[i].orig_fd);
				close(sq[i].orig_fd);
			}
		}
		حرر(sq);
	}
	إذا (G.HFILE_stdin
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

#إذا ENABLE_FEATURE_SH_STANDALONE && BB_MMU
ساكن فراغ close_saved_fds_and_FILE_fds(فراغ)
{
	إذا (G_interactive_fd)
		close(G_interactive_fd);
	close_all_HFILE_list();
}
#پایان_اگر

ساكن صحيح internally_opened_fd(صحيح fd, بنية squirrel *sq)
{
	صحيح i;

#إذا ENABLE_HUSH_INTERACTIVE
	إذا (fd == G_interactive_fd)
		أرجع 1;
#پایان_اگر
	/* If this one of script's fds? */
	إذا (fd_درون_HFILEs(fd))
		أرجع 1;

	إذا (sq) لكل (i = 0; sq[i].orig_fd >= 0; i++) {
		إذا (fd == sq[i].moved_مقصد)
			أرجع 1;
	}
	أرجع 0;
}

/* sqp != NULL means we squirrel away copies of stdin, stdout,
 * and stderr if they are redirected.
 * If redirection fails, return 1. This will make caller
 * skip command execution and restore already created redirect fds.
 */
ساكن صحيح راهاندازی_redirects(بنية command *prog, بنية squirrel **sqp)
{
	بنية redir_struct *redir;

	لكل (redir = prog->redirects; redir; redir = redir->بعدی) {
		صحيح newfd;
		صحيح closed;

		إذا (redir->rd_type == REDIRECT_HEREDOC2) {
			/* "rd_fd<<HERE" case */
			إذا (save_fd_on_redirect(redir->rd_fd, /*avoid:*/ 0, sqp) < 0)
				أرجع 1;
			/* for REDIRECT_HEREDOC2, rd_filename holds _contents_
			 * of the heredoc */
			debug_printf_redir("set heredoc '%s'\n",
					redir->rd_filename);
			راهاندازی_heredoc(redir);
			استمر;
		}

		إذا (redir->rd_dup == REDIRFD_مقصد_FILE) {
			/* "rd_fd<*>file" case (<*> is <,>,>>,<>) */
			حرف *p;
			صحيح نما;

			إذا (redir->rd_filename == عدم) {
				/* Examples:
				 * "cmd >" (no filename)
				 * "cmd > <file" (2nd redirect starts too early)
				 */
				syntax_error("invalid redirect");
				أرجع 1;
			}
			نما = redir_جدول[redir->rd_type].نما;
			p = expand_string_مقصد_string(redir->rd_filename,
				EXP_FLAG_ESC_GLOB_CHARS, /*unbackslash:*/ 1);
			newfd = open_or_warn(p, نما);
			حرر(p);
			إذا (newfd < 0) {
				/* Error message from open_or_warn can be lost
				 * if stderr has been redirected, but bash
				 * and ash both lose it as well
				 * (though zsh doesn't!)
				 */
				أرجع 1;
			}
			إذا (newfd == redir->rd_fd && sqp
			 && sqp != ERR_PTR /* not a redirect in "exec" */
			) {
				/* open() gave us precisely the fd we wanted.
				 * This means that this fd was not busy
				 * (not opened to anywhere).
				 * Remember to close it on restore:
				 */
				*sqp = جمع_کن_squirrel_closed(*sqp, newfd);
				debug_printf_redir("redir to previously closed fd %d\n", newfd);
			}
		} وإلا {
			/* "rd_fd>&rd_dup" or "rd_fd>&-" case */
			newfd = redir->rd_dup;
		}

		إذا (newfd == redir->rd_fd)
			استمر;

		/* if "N>FILE": move newfd to redir->rd_fd */
		/* if "N>&M": dup newfd to redir->rd_fd */
		/* if "N>&-": close redir->rd_fd (newfd is REDIRFD_CLOSE) */

		closed = save_fd_on_redirect(redir->rd_fd, /*avoid:*/ newfd, sqp);
		إذا (closed < 0)
			أرجع 1; /* error */
		إذا (newfd == REDIRFD_CLOSE) {
			/* "N>&-" means "close me" */
			إذا (!closed) {
				/* ^^^ optimization: saving may already
				 * have closed it. If not... */
				close(redir->rd_fd);
			}
			/* Sometimes we do another close on restore, getting EBADF.
			 * Consider "echo 3>FILE 3>&-"
			 * first redirect remembers "need to close 3",
			 * and second redirect closes 3! Restore code then closes 3 again.
			 */
		} وإلا {
			/* if newfd is a script fd or saved fd, do not allow to use it */
			إذا (internally_opened_fd(newfd, sqp && sqp != ERR_PTR ? *sqp : عدم)) {
				bb_error_پیام("fd#%d is not open", newfd);
				أرجع 1;
			}
			إذا (dup2(newfd, redir->rd_fd) < 0) {
				/* "echo >&99" testcase */
				bb_perror_پیام("dup2(%d,%d)", newfd, redir->rd_fd);
				أرجع 1;
			}
			إذا (redir->rd_dup == REDIRFD_مقصد_FILE)
				/* "rd_fd > FILE" */
				close(newfd);
			/* else: "rd_fd > rd_dup" */
		}
	}
	أرجع 0;
}

ساكن حرف *پیدا_کن_درون_مسیر(ثابت حرف *arg)
{
	حرف *ret = عدم;
	ثابت حرف *مسیر = get_local_var_مقدار("PATH");

	إذا (!مسیر)
		أرجع عدم;

	طالما (1) {
		ثابت حرف *انتها_2 = strchrnul(مسیر, ':');
		صحيح sz = انتها_2 - مسیر; /* must be int! */

		حرر(ret);
		إذا (sz != 0) {
			ret = xasprintf("%.*s/%s", sz, مسیر, arg);
		} وإلا {
			/* We have xxx::yyyy in $PATH,
			 * it means "use current dir" */
			ret = xstrdup(arg);
		}
		إذا (access(ret, F_OK) == 0)
			اكسر;

		إذا (*انتها_2 == '\0') {
			حرر(ret);
			أرجع عدم;
		}
		مسیر = انتها_2 + 1;
	}

	أرجع ret;
}

ساكن ثابت بنية built_درون_command *پیدا_کن_builtin_helper(ثابت حرف *name,
		ثابت بنية built_درون_command *x,
		ثابت بنية built_درون_command *انتها_2)
{
	طالما (x != انتها_2) {
		إذا (strcmp(name, x->b_cmd) != 0) {
			x++;
			استمر;
		}
		debug_printf_exec("found builtin '%s'\n", name);
		أرجع x;
	}
	أرجع عدم;
}
ساكن ثابت بنية built_درون_command *پیدا_کن_builtin1(ثابت حرف *name)
{
	أرجع پیدا_کن_builtin_helper(name, bltins1, &bltins1[ARRAY_اندازه(bltins1)]);
}
ساكن ثابت بنية built_درون_command *پیدا_کن_builtin(ثابت حرف *name)
{
	ثابت بنية built_درون_command *x = پیدا_کن_builtin1(name);
	إذا (x)
		أرجع x;
	أرجع پیدا_کن_builtin_helper(name, bltins2, &bltins2[ARRAY_اندازه(bltins2)]);
}

#إذا ENABLE_HUSH_کار && ENABLE_FEATURE_TAB_COMPLETION
ساكن ثابت حرف * FAST_FUNC hush_command_name(صحيح i)
{
	إذا (/*i >= 0 && */ i < ARRAY_اندازه(bltins1)) {
		أرجع bltins1[i].b_cmd;
	}
	i -= ARRAY_اندازه(bltins1);
	إذا (i < ARRAY_اندازه(bltins2)) {
		أرجع bltins2[i].b_cmd;
	}
# إذا ENABLE_HUSH_FUNCTIONS
	{
		بنية function *funcp;
		i -= ARRAY_اندازه(bltins2);
		لكل (funcp = G.بالاترین_func; funcp; funcp = funcp->بعدی) {
			إذا (--i < 0)
				أرجع funcp->name;
		}
	}
# endif
	أرجع عدم;
}
#پایان_اگر
#پایان_اگر /* !__U_BOOT__ */

#اگر_تعریف_نشده __U_BOOT__
ساكن فراغ remove_nested_vars(فراغ)
{
	بنية variable *cur;
	بنية variable **cur_pp;

	cur_pp = &G.بالاترین_var;
	طالما ((cur = *cur_pp) != عدم) {
		إذا (cur->var_nest_level <= G.var_nest_level) {
			cur_pp = &cur->بعدی;
			استمر;
		}
		/* Unexport */
		إذا (cur->flg_export) {
			debug_printf_env("unexporting nested '%s'/%u\n", cur->varstr, cur->var_nest_level);
			bb_unsetenv(cur->varstr);
		}
		/* Remove from global list */
		*cur_pp = cur->بعدی;
		/* Free */
		إذا (!cur->max_len) {
			debug_printf_env("freeing nested '%s'/%u\n", cur->varstr, cur->var_nest_level);
			حرر(cur->varstr);
		}
		حرر(cur);
	}
}

ساكن فراغ enter_var_nest_level(فراغ)
{
	G.var_nest_level++;
	debug_printf_env("var_nest_level++ %u\n", G.var_nest_level);

	/* Try:	f() { echo -n .; f; }; f
	 * struct variable::var_nest_level is uint16_t,
	 * thus limiting recursion to < 2^16.
	 * In any case, with 8 Mbyte stack SEGV happens
	 * not too long after 2^16 recursions anyway.
	 */
	إذا (G.var_nest_level > 0xff00)
		bb_error_پیام_and_die("fatal recursion (depth %u)", G.var_nest_level);
}

ساكن فراغ leave_var_nest_level(فراغ)
{
	G.var_nest_level--;
	debug_printf_env("var_nest_level-- %u\n", G.var_nest_level);
	إذا (HUSH_DEBUG && (صحيح)G.var_nest_level < 0)
		bb_simple_error_پیام_and_die("BUG: nesting underflow");

	remove_nested_vars();
}
#پایان_اگر /* __U_BOOT__ */

#إذا ENABLE_HUSH_FUNCTIONS
ساكن بنية function **پیدا_کن_function_جایگاه(ثابت حرف *name)
{
	بنية function *funcp;
	بنية function **funcpp = &G.بالاترین_func;

	طالما ((funcp = *funcpp) != عدم) {
		إذا (strcmp(name, funcp->name) == 0) {
			debug_printf_exec("found function '%s'\n", name);
			اكسر;
		}
		funcpp = &funcp->بعدی;
	}
	أرجع funcpp;
}

ساكن ALWAYS_INLINE ثابت بنية function *پیدا_کن_function(ثابت حرف *name)
{
	ثابت بنية function *funcp = *پیدا_کن_function_جایگاه(name);
	أرجع funcp;
}

/* Note: takes ownership on name ptr */
ساكن بنية function *new_function(حرف *name)
{
	بنية function **funcpp = پیدا_کن_function_جایگاه(name);
	بنية function *funcp = *funcpp;

	إذا (funcp != عدم) {
		بنية command *cmd = funcp->والد_cmd;
		debug_printf_exec("func %p parent_cmd %p\n", funcp, cmd);
		إذا (!cmd) {
			debug_printf_exec("freeing & replacing function '%s'\n", funcp->name);
			حرر(funcp->name);
			/* Note: if !funcp->body, do not free body_as_string!
			 * This is a special case of "-F name body" function:
			 * body_as_string was not malloced! */
			إذا (funcp->body) {
				free_pipe_list(funcp->body);
# إذا !BB_MMU
				حرر(funcp->body_as_string);
# endif
			}
		} وإلا {
			debug_printf_exec("reinserting in tree & replacing function '%s'\n", funcp->name);
			cmd->argv[0] = funcp->name;
			cmd->گروه = funcp->body;
# إذا !BB_MMU
			cmd->گروه_as_string = funcp->body_as_string;
# endif
		}
	} وإلا {
		debug_printf_exec("remembering new function '%s'\n", name);
		funcp = *funcpp = xzalloc(حجم(*funcp));
		/*funcp->next = NULL;*/
	}

	funcp->name = name;
	أرجع funcp;
}

# إذا ENABLE_HUSH_UNSET
ساكن فراغ unset_func(ثابت حرف *name)
{
	بنية function **funcpp = پیدا_کن_function_جایگاه(name);
	بنية function *funcp = *funcpp;

	إذا (funcp != عدم) {
		debug_printf_exec("freeing function '%s'\n", funcp->name);
		*funcpp = funcp->بعدی;
		/* funcp is unlinked now, deleting it.
		 * Note: if !funcp->body, the function was created by
		 * "-F name body", do not free ->body_as_string
		 * and ->name as they were not malloced. */
		إذا (funcp->body) {
			free_pipe_list(funcp->body);
			حرر(funcp->name);
#  إذا !BB_MMU
			حرر(funcp->body_as_string);
#  endif
		}
		حرر(funcp);
	}
}
# endif

# إذا BB_MMU
#تعریف_کن exec_function(مقصد_free, funcp, argv) \
	exec_function(funcp, argv)
# endif
ساكن فراغ exec_function(حرف ***مقصد_free,
		ثابت بنية function *funcp,
		حرف **argv) NORETURN;
ساكن فراغ exec_function(حرف ***مقصد_free,
		ثابت بنية function *funcp,
		حرف **argv)
{
# إذا BB_MMU
	صحيح n;

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
	G_flag_return_درون_پیشرفت = -1;
	enter_var_nest_level();
	IF_HUSH_LOCAL(G.func_nest_level++;)

	/* On MMU, funcp->body is always non-NULL */
	n = پیاپی_list(funcp->body);
	_exit(n);
# وإلا
//?	close_saved_fds_and_FILE_fds();

//TODO: check whether "true | func_with_return" works

	re_execute_shell(مقصد_free,
			funcp->body_as_string,
			G.global_argv[0],
			argv + 1,
			عدم);
# endif
}

ساكن صحيح پیاپی_function(ثابت بنية function *funcp, حرف **argv)
{
	صحيح rc;
	save_arg_t sv;
	smallint sv_flg;

	save_and_replace_G_آرگومانها(&sv, argv);

	/* "We are in function, ok to use return" */
	sv_flg = G_flag_return_درون_پیشرفت;
	G_flag_return_درون_پیشرفت = -1;

	/* Make "local" variables properly shadow previous ones */
	IF_HUSH_LOCAL(enter_var_nest_level();)
	IF_HUSH_LOCAL(G.func_nest_level++;)

	/* On MMU, funcp->body is always non-NULL */
# إذا !BB_MMU
	إذا (!funcp->body) {
		/* Function defined by -F */
		parse_and_پیاپی_string(funcp->body_as_string);
		rc = G.آخرین_exitcode;
	} وإلا
# endif
	{
		rc = پیاپی_list(funcp->body);
	}

	IF_HUSH_LOCAL(G.func_nest_level--;)
	IF_HUSH_LOCAL(leave_var_nest_level();)

	G_flag_return_درون_پیشرفت = sv_flg;
# إذا ENABLE_HUSH_TRAP
	debug_printf_exec("G.return_exitcode=-1\n");
	G.return_exitcode = -1; /* invalidate stashed return value */
# endif

	restore_G_آرگومانها(&sv, argv);

	أرجع rc;
}
#پایان_اگر /* ENABLE_HUSH_FUNCTIONS */

#اگر_تعریف_نشده __U_BOOT__
#إذا BB_MMU
#تعریف_کن exec_builtin(مقصد_free, x, argv) \
	exec_builtin(x, argv)
#وإلا
#تعریف_کن exec_builtin(مقصد_free, x, argv) \
	exec_builtin(مقصد_free, argv)
#پایان_اگر
ساكن فراغ exec_builtin(حرف ***مقصد_free,
		ثابت بنية built_درون_command *x,
		حرف **argv) NORETURN;
ساكن فراغ exec_builtin(حرف ***مقصد_free,
		ثابت بنية built_درون_command *x,
		حرف **argv)
{
#إذا BB_MMU
	صحيح rcode;
//?	close_saved_fds_and_FILE_fds();
	rcode = x->b_function(argv);
	fflush_all();
	_exit(rcode);
#وإلا
	fflush_all();
	/* On NOMMU, we must never block!
	 * Example: { sleep 99 | read line; } & echo Ok
	 */
	re_execute_shell(مقصد_free,
			argv[0],
			G.global_argv[0],
			G.global_argv + 1,
			argv);
#پایان_اگر
}
#پایان_اگر /* !__U_BOOT__ */

#اگر_تعریف_نشده __U_BOOT__
ساكن فراغ execvp_or_die(حرف **argv) NORETURN;
ساكن فراغ execvp_or_die(حرف **argv)
{
	صحيح e;
	debug_printf_exec("execing '%s'\n", argv[0]);
	/* Don't propagate SIG_IGN to the child */
	إذا (SPECIAL_JOBSTOP_SIGS != 0)
		switch_off_special_sigs(G.special_sig_ماسک & SPECIAL_JOBSTOP_SIGS);
	execvp(argv[0], argv);
	e = 2;
	إذا (errno == EACCES) e = 126;
	إذا (errno == ENOENT) e = 127;
	bb_perror_پیام("can't execute '%s'", argv[0]);
	_exit(e);
}

#إذا ENABLE_HUSH_نما_X
ساكن فراغ x_نما_print_optionally_squoted(ثابت حرف *str)
{
	بیعلامت len;
	ثابت حرف *cp;

	cp = str;

	/* the set of chars which-cause-string-to-be-squoted mimics bash */
	/* test a char with: bash -c 'set -x; echo "CH"' */
	إذا (str[strcspn(str, "\\\"'`$(){}[]<>;#&|~*?!^"
			" " "\001\002\003\004\005\006\007"
			"\010\011\012\013\014\015\016\017"
			"\020\021\022\023\024\025\026\027"
			"\030\031\032\033\034\035\036\037"
			)
		] == '\0'
	) {
		/* string has no special chars */
		x_نما_addstr(str);
		أرجع;
	}

	cp = str;
	لكل (;;) {
		/* print '....' up to EOL or first squote */
		len = (صحيح)(strchrnul(cp, '\'') - cp);
		إذا (len != 0) {
			x_نما_addchr('\'');
			x_نما_addblock(cp, len);
			x_نما_addchr('\'');
			cp += len;
		}
		إذا (*cp == '\0')
			اكسر;
		/* string contains squote(s), print them as \' */
		x_نما_addchr('\\');
		x_نما_addchr('\'');
		cp++;
	}
}
ساكن فراغ dump_cmd_درون_x_نما(حرف **argv)
{
	إذا (G_x_نما && argv) {
		بیعلامت n;

		/* "+[+++...][ cmd...]\n\0" */
		x_نما_پیشوند();
		n = 0;
		طالما (argv[n]) {
			x_نما_addchr(' ');
			إذا (argv[n][0] == '\0') {
				x_نما_addchr('\'');
				x_نما_addchr('\'');
			} وإلا {
				x_نما_print_optionally_squoted(argv[n]);
			}
			n++;
		}
		x_نما_flush();
	}
}
#وإلا
# define dump_cmd_درون_x_نما(argv) ((فراغ)0)
#پایان_اگر
#پایان_اگر /* !__U_BOOT__ */

#اگر_تعریف_نشده __U_BOOT__
#إذا ENABLE_HUSH_COMMAND
ساكن فراغ if_command_vV_print_and_exit(حرف opt_vV, حرف *cmd, ثابت حرف *explanation)
{
	حرف *مقصد_free;

	إذا (!opt_vV)
		أرجع;

	مقصد_free = عدم;
	إذا (!explanation) {
		حرف *مسیر_2 = getenv("PATH");
		explanation = مقصد_free = پیدا_کن_executable(cmd, &مسیر_2); /* path == NULL is ok */
		إذا (!explanation)
			_exit(1); /* PROG was not found */
		إذا (opt_vV != 'V')
			cmd = مقصد_free; /* -v PROG prints "/path/to/PROG" */
	}
	چاپ_کن((opt_vV == 'V') ? "%s is %s\n" : "%s\n", cmd, explanation);
	حرر(مقصد_free);
	fflush_all();
	_exit(0);
}
#وإلا
# define if_command_vV_print_and_exit(a,b,c) ((فراغ)0)
#پایان_اگر
#پایان_اگر /* !__U_BOOT__ */

#إذا BB_MMU
#تعریف_کن pseudo_exec_argv(nommu_save, argv, assignment_cnt, argv_expanded) \
	pseudo_exec_argv(argv, assignment_cnt, argv_expanded)
#تعریف_کن pseudo_exec(nommu_save, command, argv_expanded) \
	pseudo_exec(command, argv_expanded)
#پایان_اگر

#اگر_تعریف_نشده __U_BOOT__
/* Called after [v]fork() in run_pipe, or from builtin_exec.
 * Never returns.
 * Don't exit() here.  If you don't exec, use _exit instead.
 * The at_exit handlers apparently confuse the calling process,
 * in particular stdin handling. Not sure why? -- because of vfork! (vda)
 */
ساكن فراغ pseudo_exec_argv(nommu_save_t *nommu_save,
		حرف **argv, صحيح assignment_cnt,
		حرف **argv_expanded) NORETURN;
ساكن NOINLINE فراغ pseudo_exec_argv(nommu_save_t *nommu_save,
		حرف **argv, صحيح assignment_cnt,
		حرف **argv_expanded)
{
	ثابت بنية built_درون_command *x;
	بنية variable **sv_shadowed;
	حرف **new_env;
	IF_HUSH_COMMAND(حرف opt_vV = 0;)
	IF_HUSH_FUNCTIONS(ثابت بنية function *funcp;)

	new_env = expand_assignments(argv, assignment_cnt);
	dump_cmd_درون_x_نما(new_env);

	إذا (!argv[assignment_cnt]) {
		/* Case when we are here: ... | var=val | ...
		 * (note that we do not exit early, i.e., do not optimize out
		 * expand_assignments(): think about ... | var=`sleep 1` | ...
		 */
		free_رشتهها(new_env);
		_exit_SUCCESS();
	}

	sv_shadowed = G.shadowed_vars_pp;
#إذا BB_MMU
	G.shadowed_vars_pp = عدم; /* "don't save, free them instead" */
#وإلا
	G.shadowed_vars_pp = &nommu_save->old_vars;
	G.var_nest_level++;
#پایان_اگر
	مجموعه_vars_and_save_old(new_env);
	G.shadowed_vars_pp = sv_shadowed;

	إذا (argv_expanded) {
		argv = argv_expanded;
	} وإلا {
		argv = expand_strvec_مقصد_strvec(argv + assignment_cnt);
#إذا !BB_MMU
		nommu_save->argv = argv;
#پایان_اگر
	}
	dump_cmd_درون_x_نما(argv);

#إذا ENABLE_FEATURE_SH_STANDALONE || BB_MMU
	إذا (strchr(argv[0], '/') != عدم)
		اذهب skip;
#پایان_اگر

#إذا ENABLE_HUSH_FUNCTIONS
	/* Check if the command matches any functions (this goes before bltins) */
	funcp = پیدا_کن_function(argv[0]);
	إذا (funcp)
		exec_function(&nommu_save->argv_from_re_execing, funcp, argv);
#پایان_اگر

#إذا ENABLE_HUSH_COMMAND
	/* "command BAR": run BAR without looking it up among functions
	 * "command -v BAR": print "BAR" or "/path/to/BAR"; or exit 1
	 * "command -V BAR": print "BAR is {a function,a shell builtin,/path/to/BAR}"
	 */
	طالما (strcmp(argv[0], "command") == 0 && argv[1]) {
		حرف *p;

		argv++;
		p = *argv;
		إذا (p[0] != '-' || !p[1])
			استمر; /* bash allows "command command command [-OPT] BAR" */

		لكل (;;) {
			p++;
			بدل (*p) {
			حالة '\0':
				argv++;
				p = *argv;
				إذا (p[0] != '-' || !p[1])
					اذهب after_opts;
				استمر; /* next arg is also -opts, process it too */
			حالة 'v':
			حالة 'V':
				opt_vV = *p;
				استمر;
			افتراضي:
				bb_error_پیام_and_die("%s: %s: invalid option", "command", argv[0]);
			}
		}
	}
 after_opts:
# إذا ENABLE_HUSH_FUNCTIONS
	إذا (opt_vV && پیدا_کن_function(argv[0]))
		if_command_vV_print_and_exit(opt_vV, argv[0], "a function");
# endif
#پایان_اگر

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
	x = BB_MMU ? پیدا_کن_builtin(argv[0]) : پیدا_کن_builtin1(argv[0]);
	إذا (x) {
		if_command_vV_print_and_exit(opt_vV, argv[0], "a shell builtin");
		exec_builtin(&nommu_save->argv_from_re_execing, x, argv);
	}

#إذا ENABLE_FEATURE_SH_STANDALONE
	/* Check if the command matches any busybox applets */
	{
		صحيح a = پیدا_کن_applet_by_name(argv[0]);
		إذا (a >= 0) {
			if_command_vV_print_and_exit(opt_vV, argv[0], "an applet");
# إذا BB_MMU /* see above why on NOMMU it is not allowed */
			إذا (APPLET_IS_NOEXEC(a)) {
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
				switch_off_special_sigs(G.special_sig_ماسک);
				debug_printf_exec("running applet '%s'\n", argv[0]);
				پیاپی_noexec_applet_and_exit(a, argv[0], argv);
			}
# endif
			/* Re-exec ourselves */
			debug_printf_exec("re-execing applet '%s'\n", argv[0]);
			/* Don't propagate SIG_IGN to the child */
			إذا (SPECIAL_JOBSTOP_SIGS != 0)
				switch_off_special_sigs(G.special_sig_ماسک & SPECIAL_JOBSTOP_SIGS);
			execv(bb_busybox_exec_مسیر, argv);
			/* If they called chroot or otherwise made the binary no longer
			 * executable, fall through */
		}
	}
#پایان_اگر

#إذا ENABLE_FEATURE_SH_STANDALONE || BB_MMU
 skip:
#پایان_اگر
	if_command_vV_print_and_exit(opt_vV, argv[0], عدم);
	execvp_or_die(argv);
}

/* Called after [v]fork() in run_pipe
 */
ساكن فراغ pseudo_exec(nommu_save_t *nommu_save,
		بنية command *command,
		حرف **argv_expanded) NORETURN;
ساكن فراغ pseudo_exec(nommu_save_t *nommu_save,
		بنية command *command,
		حرف **argv_expanded)
{
#إذا ENABLE_HUSH_FUNCTIONS
	إذا (command->cmd_type == CMD_FUNCDEF) {
		/* Ignore funcdefs in pipes:
		 * true | f() { cmd }
		 */
		_exit(0);
	}
#پایان_اگر

	إذا (command->argv) {
		pseudo_exec_argv(nommu_save, command->argv,
				command->assignment_cnt, argv_expanded);
	}

	إذا (command->گروه) {
		/* Cases when we are here:
		 * ( list )
		 * { list } &
		 * ... | ( list ) | ...
		 * ... | { list } | ...
		 */
#إذا BB_MMU
		صحيح rcode;
		debug_printf_exec("pseudo_exec: run_list\n");
		reset_traps_مقصد_defaults();
		rcode = پیاپی_list(command->گروه);
		/* OK to leak memory by not calling free_pipe_list,
		 * since this process is about to exit */
		_exit(rcode);
#وإلا
		re_execute_shell(&nommu_save->argv_from_re_execing,
				command->گروه_as_string,
				G.global_argv[0],
				G.global_argv + 1,
				عدم);
#پایان_اگر
	}

	/* Case when we are here: ... | >file */
	debug_printf_exec("pseudo_exec'ed null command\n");
	_exit_SUCCESS();
}

#إذا ENABLE_HUSH_کار
ساكن ثابت حرف *get_cmdtext(بنية pipe *pi)
{
	حرف **argv;
	حرف *p;
	صحيح len;

	/* This is subtle. ->cmdtext is created only on first backgrounding.
	 * (Think "cat, <ctrl-z>, fg, <ctrl-z>, fg, <ctrl-z>...." here...)
	 * On subsequent bg argv is trashed, but we won't use it */
	إذا (pi->cmdtext)
		أرجع pi->cmdtext;

	argv = pi->cmds[0].argv;
	إذا (!argv) {
		pi->cmdtext = xzalloc(1);
		أرجع pi->cmdtext;
	}
	len = 0;
	افعل {
		len += طول_رشته(*argv) + 1;
	} طالما (*++argv);
	p = xmalloc(len);
	pi->cmdtext = p;
	argv = pi->cmds[0].argv;
	افعل {
		p = stpcpy(p, *argv);
		*p++ = ' ';
	} طالما (*++argv);
	p[-1] = '\0';
	أرجع pi->cmdtext;
}

ساكن فراغ remove_کار_from_جدول(بنية pipe *pi)
{
	بنية pipe *prev_pipe;

	إذا (pi == G.کار_list) {
		G.کار_list = pi->بعدی;
	} وإلا {
		prev_pipe = G.کار_list;
		طالما (prev_pipe->بعدی != pi)
			prev_pipe = prev_pipe->بعدی;
		prev_pipe->بعدی = pi->بعدی;
	}
	G.آخرین_jobid = 0;
	إذا (G.کار_list)
		G.آخرین_jobid = G.کار_list->jobid;
}

ساكن فراغ delete_پایانیافته_کار(بنية pipe *pi)
{
	remove_کار_from_جدول(pi);
	free_pipe(pi);
}

ساكن فراغ clean_به_بالا_آخرین_dead_کار(فراغ)
{
	إذا (G.کار_list && !G.کار_list->alive_cmds)
		delete_پایانیافته_کار(G.کار_list);
}

ساكن فراغ درج_کار_into_جدول(بنية pipe *pi)
{
	بنية pipe *کار, **jobp;
	صحيح i;

	clean_به_بالا_آخرین_dead_کار();

	/* Find the end of the list, and find next job ID to use */
	i = 0;
	jobp = &G.کار_list;
	طالما ((کار = *jobp) != عدم) {
		إذا (کار->jobid > i)
			i = کار->jobid;
		jobp = &کار->بعدی;
	}
	pi->jobid = i + 1;

	/* Create a new job struct at the end */
	کار = *jobp = xmemdup(pi, حجم(*pi));
	کار->بعدی = عدم;
	کار->cmds = xzalloc(حجم(pi->cmds[0]) * pi->رقم_cmds);
	/* Cannot copy entire pi->cmds[] vector! This causes double frees */
	لكل (i = 0; i < pi->رقم_cmds; i++) {
		کار->cmds[i].pid = pi->cmds[i].pid;
		/* all other fields are not used and stay zero */
	}
	کار->cmdtext = xstrdup(get_cmdtext(pi));

	إذا (G_interactive_fd)
		چاپ_کن("[%u] %u %s\n", کار->jobid, (بیعلامت)کار->cmds[0].pid, کار->cmdtext);
	G.آخرین_jobid = کار->jobid;
}
#پایان_اگر /*کار*/

ساكن صحيح کار_exited_or_stopped(بنية pipe *pi)
{
	صحيح rcode, i;

	إذا (pi->alive_cmds != pi->stopped_cmds)
		أرجع -1;

	/* All processes in fg pipe have exited or stopped */
	rcode = 0;
	i = pi->رقم_cmds;
	طالما (--i >= 0) {
		rcode = pi->cmds[i].cmd_exitcode;
		/* usually last process gives overall exitstatus,
		 * but with "set -o pipefail", last *failed* process does */
		إذا (G.o_opt[OPT_O_PIPEFAIL] == 0 || rcode != 0)
			اكسر;
	}
	IF_HAS_KEYWORDS(إذا (pi->pi_inverted) rcode = !rcode;)
	أرجع rcode;
}

ساكن صحيح process_wait_نتیجه(بنية pipe *fg_pipe, pid_t childpid, صحيح وضعیت)
{
#إذا ENABLE_HUSH_کار
	بنية pipe *pi;
#پایان_اگر
	صحيح i, dead;

	dead = WIFEXITED(وضعیت) || WIFSIGNALED(وضعیت);

#إذا DEBUG_JOBS
	إذا (WIFSTOPPED(وضعیت))
		debug_printf_jobs("pid %d stopped by sig %d (exitcode %d)\n",
				childpid, WSTOPSIG(وضعیت), WEXITSTATUS(وضعیت));
	إذا (WIFSIGNALED(وضعیت))
		debug_printf_jobs("pid %d killed by sig %d (exitcode %d)\n",
				childpid, WTERMSIG(وضعیت), WEXITSTATUS(وضعیت));
	إذا (WIFEXITED(وضعیت))
		debug_printf_jobs("pid %d exited, exitcode %d\n",
				childpid, WEXITSTATUS(وضعیت));
#پایان_اگر
	/* Were we asked to wait for a fg pipe? */
	إذا (fg_pipe) {
		i = fg_pipe->رقم_cmds;

		طالما (--i >= 0) {
			صحيح rcode;

			debug_printf_jobs("check pid %d\n", fg_pipe->cmds[i].pid);
			إذا (fg_pipe->cmds[i].pid != childpid)
				استمر;
			إذا (dead) {
				صحيح ex;
				fg_pipe->cmds[i].pid = 0;
				fg_pipe->alive_cmds--;
				ex = WEXITSTATUS(وضعیت);
				/* bash prints killer signal's name for *last*
				 * process in pipe (prints just newline for SIGINT/SIGPIPE).
				 * Mimic this. Example: "sleep 5" + (^\ or kill -QUIT)
				 */
				إذا (WIFSIGNALED(وضعیت)) {
					صحيح sig = WTERMSIG(وضعیت);
#إذا ENABLE_HUSH_کار
					إذا (G.پیاپی_list_level == 1
					/* ^^^^^ Do not print in nested contexts, example:
					 * echo `sleep 1; sh -c 'kill -9 $$'` - prints "137", NOT "Killed 137"
					 */
					 && i == fg_pipe->رقم_cmds-1
					) {
						/* strsignal() is for bash compat. ~600 bloat versus bbox's get_signame() */
						puts(sig == SIGINT || sig == SIGPIPE ? "" : strsignal(sig));
					}
#پایان_اگر
					/* TODO: if (WCOREDUMP(status)) + " (core dumped)"; */
					/* MIPS has 128 sigs (1..128), if sig==128,
					 * 128 + sig would result in exitcode 256 -> 0!
					 */
					ex = 128 | sig;
				}
				fg_pipe->cmds[i].cmd_exitcode = ex;
			} وإلا {
				fg_pipe->stopped_cmds++;
			}
			debug_printf_jobs("fg_pipe: alive_cmds %d stopped_cmds %d\n",
					fg_pipe->alive_cmds, fg_pipe->stopped_cmds);
			rcode = کار_exited_or_stopped(fg_pipe);
			إذا (rcode >= 0) {
/* Note: *non-interactive* bash does not continue if all processes in fg pipe
 * are stopped. Testcase: "cat | cat" in a script (not on command line!)
 * and "killall -STOP cat" */
				إذا (G_interactive_fd) {
#إذا ENABLE_HUSH_کار
					إذا (fg_pipe->alive_cmds != 0)
						درج_کار_into_جدول(fg_pipe);
#پایان_اگر
					أرجع rcode;
				}
				إذا (fg_pipe->alive_cmds == 0)
					أرجع rcode;
			}
			/* There are still running processes in the fg_pipe */
			أرجع -1;
		}
		/* It wasn't in fg_pipe, look for process in bg pipes */
	}

#إذا ENABLE_HUSH_کار
	/* We were asked to wait for bg or orphaned children */
	/* No need to remember exitcode in this case */
	لكل (pi = G.کار_list; pi; pi = pi->بعدی) {
		لكل (i = 0; i < pi->رقم_cmds; i++) {
			إذا (pi->cmds[i].pid == childpid)
				اذهب یافت_pi_and_prognum;
		}
	}
	/* Happens when shell is used as init process (init=/bin/sh) */
	debug_printf("checkjobs: pid %d was not in our list!\n", childpid);
	أرجع -1; /* this wasn't a process from fg_pipe */

 یافت_pi_and_prognum:
	إذا (dead) {
		/* child exited */
		صحيح rcode = WEXITSTATUS(وضعیت);
		إذا (WIFSIGNALED(وضعیت))
			/* NB: not 128 + sig, MIPS has sig 128 */
			rcode = 128 | WTERMSIG(وضعیت);
		pi->cmds[i].cmd_exitcode = rcode;
		إذا (G.آخرین_bg_pid == pi->cmds[i].pid)
			G.آخرین_bg_pid_exitcode = rcode;
		pi->cmds[i].pid = 0;
		pi->alive_cmds--;
		إذا (!pi->alive_cmds) {
# إذا ENABLE_HUSH_BASH_COMPAT
			G.dead_کار_exitcode = کار_exited_or_stopped(pi);
# endif
			إذا (G_interactive_fd) {
				چاپ_کن(کار_وضعیت_FORMAT, pi->jobid,
						"Done", pi->cmdtext);
				delete_پایانیافته_کار(pi);
			} وإلا {
/*
 * bash deletes finished jobs from job table only in interactive mode,
 * after "jobs" cmd, or if pid of a new process matches one of the old ones
 * (see cleanup_dead_jobs(), delete_old_job(), J_NOTIFIED in bash source).
 * Testcase script: "(exit 3) & sleep 1; wait %1; echo $?" prints 3 in bash.
 * We only retain one "dead" job, if it's the single job on the list.
 * This covers most of real-world scenarios where this is useful.
 */
				إذا (pi != G.کار_list)
					delete_پایانیافته_کار(pi);
			}
		}
	} وإلا {
		/* child stopped */
		pi->stopped_cmds++;
	}
#پایان_اگر
	أرجع -1; /* this wasn't a process from fg_pipe */
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
ساكن صحيح checkjobs(بنية pipe *fg_pipe, pid_t waitfor_pid)
{
	صحيح صفات;
	صحيح وضعیت;
	صحيح rcode = 0;

	debug_printf_jobs("checkjobs %p\n", fg_pipe);

	صفات = WUNTRACED;
	إذا (fg_pipe == عدم)
		صفات |= WNOHANG;

	errno = 0;
#إذا ENABLE_HUSH_FAST
	إذا (G.handled_SIGCHLD == G.شمار_SIGCHLD) {
//bb_error_msg("[%d] checkjobs: G.count_SIGCHLD:%d G.handled_SIGCHLD:%d children?:%d fg_pipe:%p",
//getpid(), G.count_SIGCHLD, G.handled_SIGCHLD, G.we_have_children, fg_pipe);
		/* There was neither fork nor SIGCHLD since last waitpid */
		/* Avoid doing waitpid syscall if possible */
		إذا (!G.we_have_children) {
			errno = ECHILD;
			أرجع -1;
		}
		إذا (fg_pipe == عدم) { /* is WNOHANG set? */
			/* We have children, but they did not exit
			 * or stop yet (we saw no SIGCHLD) */
			أرجع 0;
		}
		/* else: !WNOHANG, waitpid will block, can't short-circuit */
	}
#پایان_اگر

/* Do we do this right?
 * bash-3.00# sleep 20 | false
 * <ctrl-Z pressed>
 * [3]+  Stopped          sleep 20 | false
 * bash-3.00# echo $?
 * 1   <========== bg pipe is not fully done, but exitcode is already known!
 * [hush 1.14.0: yes we do it right]
 */
	طالما (1) {
		pid_t childpid;
#إذا ENABLE_HUSH_FAST
		صحيح i;
		i = G.شمار_SIGCHLD;
#پایان_اگر
		childpid = waitpid(-1, &وضعیت, صفات);
		إذا (childpid <= 0) {
			إذا (childpid && errno != ECHILD)
				bb_simple_perror_پیام("waitpid");
#إذا ENABLE_HUSH_FAST
			وإلا { /* Until next SIGCHLD, waitpid's are useless */
				G.we_have_children = (childpid == 0);
				G.handled_SIGCHLD = i;
//bb_error_msg("[%d] checkjobs: waitpid returned <= 0, G.count_SIGCHLD:%d G.handled_SIGCHLD:%d", getpid(), G.count_SIGCHLD, G.handled_SIGCHLD);
			}
#پایان_اگر
			/* ECHILD (no children), or 0 (no change in children status) */
			rcode = childpid;
			اكسر;
		}
		rcode = process_wait_نتیجه(fg_pipe, childpid, وضعیت);
		إذا (rcode >= 0) {
			/* fg_pipe exited or stopped */
			اكسر;
		}
		إذا (childpid == waitfor_pid) { /* "wait PID" */
			debug_printf_exec("childpid==waitfor_pid:%d status:0x%08x\n", childpid, وضعیت);
			rcode = WEXITSTATUS(وضعیت);
			إذا (WIFSIGNALED(وضعیت))
				rcode = 128 | WTERMSIG(وضعیت);
			إذا (WIFSTOPPED(وضعیت))
				/* bash: "cmd & wait $!" and cmd stops: $? = 128 | stopsig */
				rcode = 128 | WSTOPSIG(وضعیت);
			rcode++;
			اكسر; /* "wait PID" called us, give it exitcode+1 */
		}
#إذا ENABLE_HUSH_BASH_COMPAT
		إذا (-1 == waitfor_pid /* "wait -n" (wait for any one job) */
		 && G.dead_کار_exitcode >= 0 /* some job did finish */
		) {
			debug_printf_exec("waitfor_pid:-1\n");
			rcode = G.dead_کار_exitcode + 1;
			اكسر;
		}
#پایان_اگر
		/* This wasn't one of our processes, or */
		/* fg_pipe still has running processes, do waitpid again */
	} /* while (waitpid succeeds)... */

	أرجع rcode;
}

#إذا ENABLE_HUSH_کار
ساكن صحيح checkjobs_and_fg_shell(بنية pipe *fg_pipe)
{
	pid_t p;
	صحيح rcode = checkjobs(fg_pipe, 0 /*(no pid to wait for)*/);
	إذا (G_saved_tty_pgrp) {
		/* Job finished, move the shell to the foreground */
		p = getpgrp(); /* our process group id */
		debug_printf_jobs("fg'ing ourself: getpgrp()=%d\n", (صحيح)p);
		tcsetpgrp(G_interactive_fd, p);
	}
	أرجع rcode;
}
#پایان_اگر
#پایان_اگر /* !__U_BOOT__ */

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
ساكن فراغ مجموعه_G_ifs(فراغ)
{
	/* Testcase: set -- q w e; (IFS='' echo "$*"; IFS=''; echo "$*"); echo "$*"
	 * Result should be 3 lines: q w e, qwe, q w e
	 */
	إذا (G.ifs_whitespace != G.ifs)
		حرر(G.ifs_whitespace);
	G.ifs = get_local_var_مقدار("IFS");
	إذا (G.ifs) {
		حرف *p;
		G.ifs_whitespace = (حرف*)G.ifs;
		p = skip_whitespace(G.ifs);
		إذا (*p) {
			/* Not all $IFS is whitespace */
			حرف *d;
			صحيح len = p - G.ifs;
			p = skip_non_whitespace(p);
			G.ifs_whitespace = xmalloc(len + طول_رشته(p) + 1); /* can overestimate */
			d = mempcpy(G.ifs_whitespace, G.ifs, len);
			طالما (*p) {
				إذا (آیا_فاصله(*p))
					*d++ = *p;
				p++;
			}
			*d = '\0';
		}
	} وإلا {
		G.ifs = defifs;
		G.ifs_whitespace = (حرف*)G.ifs;
	}
}
#اگر_تعریف_نشده __U_BOOT__
#إذا !ENABLE_HUSH_نما_X
#تعریف_کن redirect_and_varexp_helper(command, sqp, argv_expanded) \
	redirect_and_varexp_helper(command, sqp)
#پایان_اگر
ساكن صحيح redirect_and_varexp_helper(
		بنية command *command,
		بنية squirrel **sqp,
		حرف **argv_expanded)
{
	/* Assignments occur before redirects. Try:
	 * a=`sleep 1` sleep 2 3>/qwe/rty
	 */

	حرف **new_env = expand_assignments(command->argv, command->assignment_cnt);
	dump_cmd_درون_x_نما(new_env);
	dump_cmd_درون_x_نما(argv_expanded);
	/* this takes ownership of new_env[i] elements, and frees new_env: */
	مجموعه_vars_and_save_old(new_env);

	أرجع راهاندازی_redirects(command, sqp);
}
#پایان_اگر /* !__U_BOOT__ */

ساكن NOINLINE صحيح پیاپی_pipe(بنية pipe *pi)
{
	ساكن ثابت حرف *ثابت null_ptr = عدم;

	صحيح cmd_no;
#اگر_تعریف_نشده __U_BOOT__
	صحيح بعدی_infd;
#پایان_اگر /* !__U_BOOT__ */
	بنية command *command;
	حرف **argv_expanded;
	حرف **argv;
#اگر_تعریف_نشده __U_BOOT__
	بنية squirrel *squirrel = عدم;
#پایان_اگر /* !__U_BOOT__ */
	صحيح rcode;

#اگر_تعریف_شده __U_BOOT__
	/*
	 * Set rcode here to avoid returning a garbage value in the middle of
	 * the function.
	 * Also, if an error occurs, rcode value would be changed and last
	 * return will signal the error.
	 */
	rcode = 0;
#پایان_اگر /* __U_BOOT__ */

	debug_printf_exec("run_pipe start: members:%d\n", pi->رقم_cmds);
	debug_enter();

	مجموعه_G_ifs();

#اگر_تعریف_نشده __U_BOOT__
	IF_HUSH_کار(pi->pgrp = -1;)
	pi->stopped_cmds = 0;
#پایان_اگر /* !__U_BOOT__ */
	command = &pi->cmds[0];
	argv_expanded = عدم;

#اگر_تعریف_نشده __U_BOOT__
	إذا (pi->رقم_cmds != 1
	 || pi->followup == PIPE_BG
	 || command->cmd_type == CMD_SUBSHELL
	) {
		اذهب must_fork;
	}

	pi->alive_cmds = 1;
#پایان_اگر /* !__U_BOOT__ */

	debug_printf_exec(": group:%p argv:'%s'\n",
		command->گروه, command->argv ? command->argv[0] : "NONE");

	إذا (command->گروه) {
#إذا ENABLE_HUSH_FUNCTIONS
		إذا (command->cmd_type == CMD_FUNCDEF) {
			/* "executing" func () { list } */
			بنية function *funcp;

			funcp = new_function(command->argv[0]);
			/* funcp->name is already set to argv[0] */
			funcp->body = command->گروه;
# إذا !BB_MMU
			funcp->body_as_string = command->گروه_as_string;
			command->گروه_as_string = عدم;
# endif
			command->گروه = عدم;
			command->argv[0] = عدم;
			debug_printf_exec("cmd %p has child func at %p\n", command, funcp);
			funcp->والد_cmd = command;
			command->فرزند_func = funcp;

			debug_printf_exec("run_pipe: return EXIT_SUCCESS\n");
			debug_leave();
			أرجع EXIT_SUCCESS;
		}
#پایان_اگر
		/* { list } */
		debug_printf_exec("non-subshell group\n");
		rcode = 1; /* exitcode if redir failed */
#اگر_تعریف_نشده __U_BOOT__
		إذا (راهاندازی_redirects(command, &squirrel) == 0) {
#پایان_اگر /* !__U_BOOT__ */
			debug_printf_exec(": run_list\n");
//FIXME: we need to pass squirrel down into run_list()
//for SH_STANDALONE case, or else this construct:
// { find /proc/self/fd; true; } >FILE; cmd2
//has no way of closing saved fd#1 for "find",
//and in SH_STANDALONE mode, "find" is not execed,
//therefore CLOEXEC on saved fd does not help.
			rcode = پیاپی_list(command->گروه) & 0xff;
#اگر_تعریف_نشده __U_BOOT__
		}
		restore_redirects(squirrel);
#پایان_اگر /* !__U_BOOT__ */
		IF_HAS_KEYWORDS(إذا (pi->pi_inverted) rcode = !rcode;)
		debug_leave();
		debug_printf_exec("run_pipe: return %d\n", rcode);
		أرجع rcode;
	}

	argv = command->argv ? command->argv : (حرف **) &null_ptr;
	{
#اگر_تعریف_نشده __U_BOOT__
		ثابت بنية built_درون_command *x;
		IF_HUSH_FUNCTIONS(ثابت بنية function *funcp;)
		IF_NOT_HUSH_FUNCTIONS(تعداد { funcp = 0 };)
		بنية variable **sv_shadowed;
#پایان_اگر /* !__U_BOOT__ */
		بنية variable *old_vars;

#إذا ENABLE_HUSH_LINENO_VAR
		G.execute_lineno = command->lineno;
#پایان_اگر

		إذا (argv[command->assignment_cnt] == عدم) {
			/* Assignments, but no command.
			 * Ensure redirects take effect (that is, create files).
			 * Try "a=t >file"
			 */
			بیعلامت i;
			G.expand_exitcode = 0;
 only_assignments:
#اگر_تعریف_نشده __U_BOOT__
			rcode = راهاندازی_redirects(command, &squirrel);
			restore_redirects(squirrel);
#پایان_اگر /* !__U_BOOT__ */

			/* Set shell variables */
			i = 0;
			طالما (i < command->assignment_cnt) {
				حرف *p = expand_string_مقصد_string(argv[i],
						EXP_FLAG_ESC_GLOB_CHARS,
						/*unbackslash:*/ 1
				);
#إذا ENABLE_HUSH_نما_X
				إذا (G_x_نما) {
					حرف *eq;
					إذا (i == 0)
						x_نما_پیشوند();
					x_نما_addchr(' ');
					eq = strchrnul(p, '=');
					إذا (*eq) eq++;
					x_نما_addblock(p, (eq - p));
					x_نما_print_optionally_squoted(eq);
					x_نما_flush();
				}
#پایان_اگر
				debug_printf_env("set shell var:'%s'->'%s'\n", *argv, p);
#اگر_تعریف_نشده __U_BOOT__
				إذا (مجموعه_local_var0(p)) {
#وإلا /* __U_BOOT__ */
				إذا (مجموعه_local_var_modern(p, /*flag:*/ 0)) {
#پایان_اگر
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
			إذا (rcode == 0)
				rcode = G.expand_exitcode;
			IF_HAS_KEYWORDS(إذا (pi->pi_inverted) rcode = !rcode;)
			debug_leave();
			debug_printf_exec("run_pipe: return %d\n", rcode);
			أرجع rcode;
		}

		/* Expand the rest into (possibly) many strings each */
#إذا defined(CMD_TEST2_SINGLEWORD_NOGLOB)
		إذا (command->cmd_type == CMD_TEST2_SINGLEWORD_NOGLOB)
			argv_expanded = expand_strvec_مقصد_strvec_singleword_noglob(argv + command->assignment_cnt);
		وإلا
#پایان_اگر
#إذا defined(CMD_SINGLEWORD_NOGLOB)
		إذا (command->cmd_type == CMD_SINGLEWORD_NOGLOB)
			argv_expanded = expand_strvec_مقصد_strvec_singleword_noglob(argv + command->assignment_cnt);
		وإلا
#پایان_اگر
			argv_expanded = expand_strvec_مقصد_strvec(argv + command->assignment_cnt);

		/* If someone gives us an empty string: `cmd with empty output` */
		إذا (!argv_expanded[0]) {
			حرر(argv_expanded);
			/* `false` still has to set exitcode 1 */
			G.expand_exitcode = G.آخرین_exitcode;
			اذهب only_assignments;
		}

		old_vars = عدم;
#اگر_تعریف_نشده __U_BOOT__
		sv_shadowed = G.shadowed_vars_pp;

		/* Check if argv[0] matches any functions (this goes before bltins) */
		IF_HUSH_FUNCTIONS(funcp = پیدا_کن_function(argv_expanded[0]);)
		IF_HUSH_FUNCTIONS(x = عدم;)
		IF_HUSH_FUNCTIONS(إذا (!funcp))
			x = پیدا_کن_builtin(argv_expanded[0]);
		إذا (x || funcp) {
			إذا (x && x->b_function == builtin_exec && argv_expanded[1] == عدم) {
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

				اذهب clean_به_بالا_and_ret1;
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
			إذا (rcode == 0) {
				إذا (!funcp) {
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
#إذا ENABLE_HUSH_FUNCTIONS
				وإلا {
					debug_printf_exec(": function '%s' '%s'...\n",
						funcp->name, argv_expanded[1]);
					rcode = پیاپی_function(funcp, argv_expanded) & 0xff;
					/*
					 * But do collect *to old_vars list* vars shadowed
					 * within function execution. To that end, restore
					 * this pointer _after_ function run:
					 */
					G.shadowed_vars_pp = sv_shadowed;
				}
#پایان_اگر
			}
		} وإلا
		إذا (ENABLE_FEATURE_SH_NOFORK && رقم_APPLETS > 1) {
			صحيح n = پیدا_کن_applet_by_name(argv_expanded[0]);
			إذا (n < 0 || !APPLET_IS_NOFORK(n))
				اذهب must_fork;

			enter_var_nest_level();
			/* Collect all variables "shadowed" by helper into old_vars list */
			G.shadowed_vars_pp = &old_vars;
			rcode = redirect_and_varexp_helper(command, &squirrel, argv_expanded);
			G.shadowed_vars_pp = sv_shadowed;

			إذا (rcode == 0) {
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
				rcode = پیاپی_nofork_applet(n, argv_expanded);
			}
		} وإلا
			اذهب must_fork;

		restore_redirects(squirrel);
 clean_به_بالا_and_ret1:
		leave_var_nest_level();
		جمع_کن_vars(old_vars);

		/*
		 * Try "usleep 99999999" + ^C + "echo $?"
		 * with FEATURE_SH_NOFORK=y.
		 */
		إذا (!funcp) {
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
			إذا (sigismember(&G.pending_مجموعه, SIGINT))
				rcode = 128 | SIGINT;
		}
		حرر(argv_expanded);
		IF_HAS_KEYWORDS(إذا (pi->pi_inverted) rcode = !rcode;)
		debug_leave();
		debug_printf_exec("run_pipe return %d\n", rcode);
		أرجع rcode;
#پایان_اگر /* !__U_BOOT__ */
	}

#اگر_تعریف_نشده __U_BOOT__
 must_fork:
	/* NB: argv_expanded may already be created, and that
	 * might include `cmd` runs! Do not rerun it! We *must*
	 * use argv_expanded if it's non-NULL */

	/* Going to fork a child per each pipe member */
	pi->alive_cmds = 0;
	بعدی_infd = 0;
#پایان_اگر /* !__U_BOOT__ */

	cmd_no = 0;
	طالما (cmd_no < pi->رقم_cmds) {
#اگر_تعریف_نشده __U_BOOT__
		بنية fd_pair pipefds;
#إذا !BB_MMU
		صحيح sv_var_nest_level = G.var_nest_level;
		متطاير nommu_save_t nommu_save;
		nommu_save.old_vars = عدم;
		nommu_save.argv = عدم;
		nommu_save.argv_from_re_execing = عدم;
#پایان_اگر
#پایان_اگر /* !__U_BOOT__ */
		command = &pi->cmds[cmd_no];
		cmd_no++;

#اگر_تعریف_شده __U_BOOT__
		/* Replace argv and argc by expanded if it exists. */
		إذا (argv_expanded) {
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
			command->argc = list_اندازه(argv_expanded);
			command->argv = argv_expanded;
		}
#پایان_اگر /* __U_BOOT__ */
			إذا (command->argv) {
			debug_printf_exec(": pipe member '%s' '%s'...\n",
					command->argv[0], command->argv[1]);
		} وإلا {
			debug_printf_exec(": pipe member with no argv\n");
		}

#اگر_تعریف_نشده __U_BOOT__
		/* pipes are inserted between pairs of commands */
		pipefds.rd = 0;
		pipefds.wr = 1;
		إذا (cmd_no < pi->رقم_cmds)
			xpiped_pair(pipefds);

#إذا ENABLE_HUSH_LINENO_VAR
		G.execute_lineno = command->lineno;
#پایان_اگر

		command->pid = BB_MMU ? fork() : vfork();
		إذا (!command->pid) { /*فرزند*/
#إذا ENABLE_HUSH_کار
			disable_restore_tty_pgrp_on_exit();
			CLEAR_RANDOM_T(&G.random_gen); /* or else $RANDOM repeats in child */

			/* Every child adds itself to new process group
			 * with pgid == pid_of_first_child_in_pipe */
			إذا (G.پیاپی_list_level == 1 && G_interactive_fd) {
				pid_t pgrp;
				pgrp = pi->pgrp;
				إذا (pgrp < 0) /* true for 1st process only */
					pgrp = getpid();
				إذا (setpgid(0, pgrp) == 0
				 && pi->followup != PIPE_BG
				 && G_saved_tty_pgrp /* we have ctty */
				) {
					/* We do it in *every* child, not just first,
					 * to avoid races */
					tcsetpgrp(G_interactive_fd, pgrp);
				}
			}
#پایان_اگر
			إذا (pi->alive_cmds == 0 && pi->followup == PIPE_BG) {
				/* 1st cmd in backgrounded pipe
				 * should have its stdin /dev/null'ed */
				close(0);
				إذا (open(bb_dev_null, O_RDONLY))
					xopen("/", O_RDONLY);
			} وإلا {
				xmove_fd(بعدی_infd, 0);
			}
			xmove_fd(pipefds.wr, 1);
			إذا (pipefds.rd > 1)
				close(pipefds.rd);
			/* Like bash, explicit redirects override pipes,
			 * and the pipe fd (fd#1) is available for dup'ing:
			 * "cmd1 2>&1 | cmd2": fd#1 is duped to fd#2, thus stderr
			 * of cmd1 goes into pipe.
			 */
			إذا (راهاندازی_redirects(command, عدم)) {
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
#إذا ENABLE_HUSH_FAST
		G.شمار_SIGCHLD++;
//bb_error_msg("[%d] fork in run_pipe: G.count_SIGCHLD:%d G.handled_SIGCHLD:%d", getpid(), G.count_SIGCHLD, G.handled_SIGCHLD);
#پایان_اگر
		enable_restore_tty_pgrp_on_exit();
#إذا !BB_MMU
		/* Clean up after vforked child */
		حرر(nommu_save.argv);
		حرر(nommu_save.argv_from_re_execing);
		G.var_nest_level = sv_var_nest_level;
		remove_nested_vars();
		جمع_کن_vars(nommu_save.old_vars);
#پایان_اگر
		حرر(argv_expanded);
		argv_expanded = عدم;
		إذا (command->pid < 0) { /* [v]fork failed */
			/* Clearly indicate, was it fork or vfork */
			bb_simple_perror_پیام(BB_MMU ? "vfork"+1 : "vfork");
		} وإلا {
			pi->alive_cmds++;
#إذا ENABLE_HUSH_کار
			/* Second and next children need to know pid of first one */
			إذا (pi->pgrp < 0)
				pi->pgrp = command->pid;
#پایان_اگر
		}

		إذا (cmd_no > 1)
			close(بعدی_infd);
		إذا (cmd_no < pi->رقم_cmds)
			close(pipefds.wr);
		/* Pass read (output) pipe end to next iteration */
		بعدی_infd = pipefds.rd;
#وإلا /* __U_BOOT__ */
		/* Process the command */
		rcode = cmd_process(G.do_repeat ? CMD_FLAG_REPEAT : 0,
				    command->argc, command->argv,
				    &(G.flag_repeat), عدم);

		إذا (argv_expanded) {
			/*
			 * expand_strvec_to_strvec() allocates memory to expand
			 * argv, we need to free it.
			 */
			حرر(argv_expanded);

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
#پایان_اگر /* __U_BOOT__ */
	}

#اگر_تعریف_نشده __U_BOOT__
	إذا (!pi->alive_cmds) {
		debug_leave();
		debug_printf_exec("run_pipe return 1 (all forks failed, no children)\n");
		أرجع 1;
	}
#پایان_اگر /* __U_BOOT__ */

	debug_leave();
#اگر_تعریف_نشده __U_BOOT__
	debug_printf_exec("run_pipe return -1 (%u children started)\n", pi->alive_cmds);
	أرجع -1;
#وإلا /* __U_BOOT__ */
	IF_HAS_KEYWORDS(إذا (pi->pi_inverted) rcode = !rcode;)
	debug_printf_exec("run_pipe return %d\n", rcode);
	أرجع rcode;
#پایان_اگر /* __U_BOOT__ */
}

/* NB: called by pseudo_exec, and therefore must not modify any
 * global data until exec/_exit (we can be a child after vfork!) */
ساكن صحيح پیاپی_list(بنية pipe *pi)
{
#إذا ENABLE_HUSH_CASE
	حرف *case_کلمه = عدم;
#پایان_اگر
#إذا ENABLE_HUSH_LOOPS
	بنية pipe *loop_بالاترین = عدم;
	حرف **for_lcur = عدم;
	حرف **for_list = عدم;
#پایان_اگر
	smallint آخرین_followup;
	smalluint rcode;
#إذا ENABLE_HUSH_IF || ENABLE_HUSH_CASE
	smalluint cond_کد = 0;
#وإلا
	تعداد { cond_کد = 0 };
#پایان_اگر
#إذا HAS_KEYWORDS
	smallint rword;      /* RES_foo */
	smallint آخرین_rword; /* ditto */
#پایان_اگر

#اگر_تعریف_نشده __U_BOOT__
	debug_printf_exec("run_list lvl %d start\n", G.پیاپی_list_level);
	debug_enter();
#پایان_اگر /* !__U_BOOT__ */

	مجموعه_G_ifs();

#إذا ENABLE_HUSH_LOOPS
	/* Check syntax for "for" */
	{
		بنية pipe *cpipe;
		لكل (cpipe = pi; cpipe; cpipe = cpipe->بعدی) {
			إذا (cpipe->res_کلمه != RES_FOR && cpipe->res_کلمه != RES_درون)
				استمر;
			/* current word is FOR or IN (BOLD in comments below) */
			إذا (cpipe->بعدی == عدم) {
				syntax_error("malformed for");
				debug_leave();
				debug_printf_exec("run_list lvl %d return 1\n", G.پیاپی_list_level);
				أرجع 1;
			}
			/* "FOR v; do ..." and "for v IN a b; do..." are ok */
			إذا (cpipe->بعدی->res_کلمه == RES_DO)
				استمر;
			/* next word is not "do". It must be "in" then ("FOR v in ...") */
			إذا (cpipe->res_کلمه == RES_درون /* "for v IN a b; not_do..."? */
			 || cpipe->بعدی->res_کلمه != RES_درون /* FOR v not_do_and_not_in..."? */
			) {
				syntax_error("malformed for");
				debug_leave();
				debug_printf_exec("run_list lvl %d return 1\n", G.پیاپی_list_level);
				أرجع 1;
			}
		}
	}
#پایان_اگر

	/* Past this point, all code paths should jump to ret: label
	 * in order to return, no direct "return" statements please.
	 * This helps to ensure that no memory is leaked. */

#إذا ENABLE_HUSH_کار
	G.پیاپی_list_level++;
#پایان_اگر

#إذا HAS_KEYWORDS
	rword = RES_NONE;
	آخرین_rword = RES_XXXX;
#پایان_اگر
	آخرین_followup = PIPE_SEQ;
	rcode = G.آخرین_exitcode;

	/* Go through list of pipes, (maybe) executing them. */
#اگر_تعریف_نشده __U_BOOT__
	لكل (; pi; pi = IF_HUSH_LOOPS(rword == RES_انجامشده ? loop_بالاترین : ) pi->بعدی) {
#وإلا /* __U_BOOT__ */
	لكل (; pi; pi = rword == RES_انجامشده ? loop_بالاترین : pi->بعدی) {
#پایان_اگر /* __U_BOOT__ */
		صحيح r;
		صحيح sv_errexit_عمق;

#اگر_تعریف_نشده __U_BOOT__
		إذا (G.flag_SIGINT)
			اكسر;
		إذا (G_flag_return_درون_پیشرفت == 1)
			اكسر;
#پایان_اگر /* !__U_BOOT__ */

		IF_HAS_KEYWORDS(rword = pi->res_کلمه;)
		debug_printf_exec(": rword:%d cond_code:%d last_rword:%d\n",
				rword, cond_کد, آخرین_rword);

		sv_errexit_عمق = G.errexit_عمق;
		إذا (
#إذا ENABLE_HUSH_IF
		    rword == RES_IF || rword == RES_ELIF ||
#پایان_اگر
		    pi->followup != PIPE_SEQ
		) {
			G.errexit_عمق++;
		}
#إذا ENABLE_HUSH_LOOPS
		إذا ((rword == RES_WHILE || rword == RES_UNTIL || rword == RES_FOR)
		 && loop_بالاترین == عدم /* avoid bumping G.depth_of_loop twice */
		) {
			/* start of a loop: remember where loop starts */
			loop_بالاترین = pi;
			G.عمق_از_loop++;
		}
#پایان_اگر
		/* Still in the same "if...", "then..." or "do..." branch? */
		إذا (IF_HAS_KEYWORDS(rword == آخرین_rword &&) 1) {
			إذا ((rcode == 0 && آخرین_followup == PIPE_OR)
			 || (rcode != 0 && آخرین_followup == PIPE_AND)
			) {
				/* It is "<true> || CMD" or "<false> && CMD"
				 * and we should not execute CMD */
				debug_printf_exec("skipped cmd because of || or &&\n");
				آخرین_followup = pi->followup;
				اذهب dont_check_jobs_but_continue;
			}
		}
		آخرین_followup = pi->followup;
#إذا ENABLE_HUSH_IF
		إذا (cond_کد != 0) {
			إذا (rword == RES_THEN) {
				/* if false; then ... fi has exitcode 0! */
				G.آخرین_exitcode = rcode = EXIT_SUCCESS;
				/* "if <false> THEN cmd": skip cmd */
				debug_printf_exec("skipped THEN cmd because IF condition was false\n");
				آخرین_rword = rword;
				استمر;
			}
		} وإلا {
			إذا (rword == RES_ELSE
			 || (rword == RES_ELIF && آخرین_rword != RES_ELIF)
			) {
				/* "if <true> then ... ELSE/ELIF cmd":
				 * skip cmd and all following ones */
				debug_printf_exec("skipped ELSE/ELIF branch because IF condition was true\n");
				اكسر;
			}
			//if (rword == RES_THEN): "if <true> THEN cmd", run cmd (fall through)
		}
#پایان_اگر
		IF_HAS_KEYWORDS(آخرین_rword = rword;)
#إذا ENABLE_HUSH_LOOPS
		إذا (rword == RES_FOR) { /* && pi->num_cmds - always == 1 */
			إذا (!for_lcur) {
				/* first loop through for */

				ساكن ثابت حرف encoded_dollar_at[] ALIGN1 = {
					SPECIAL_VAR_نماد, '@' | 0x80, SPECIAL_VAR_نماد, '\0'
				}; /* encoded representation of "$@" */
				ساكن ثابت حرف *ثابت encoded_dollar_at_argv[] ALIGN_PTR = {
					encoded_dollar_at, عدم
				}; /* argv list with one element: "$@" */
				حرف **vals;

				G.آخرین_exitcode = rcode = EXIT_SUCCESS;
				vals = (حرف**)encoded_dollar_at_argv;
				إذا (pi->بعدی->res_کلمه == RES_درون) {
					/* if no variable values after "in" we skip "for" */
					إذا (!pi->بعدی->cmds[0].argv) {
						debug_printf_exec(": null FOR: exitcode EXIT_SUCCESS\n");
						اكسر;
					}
					vals = pi->بعدی->cmds[0].argv;
				} /* else: "for var; do..." -> assume "$@" list */
				/* create list of variable values */
				debug_print_رشتهها("for_list made from", vals);
				for_list = expand_strvec_مقصد_strvec(vals);
				for_lcur = for_list;
				debug_print_رشتهها("for_list", for_list);
			}
			إذا (!*for_lcur) {
				/* "for" loop is over, clean up */
				حرر(for_list);
				for_list = عدم;
				for_lcur = عدم;
				اكسر;
			}
			/* Insert next value from for_lcur */
			/* note: *for_lcur already has quotes removed, $var expanded, etc */
#اگر_تعریف_نشده __U_BOOT__
			مجموعه_local_var_from_halves(pi->cmds[0].argv[0], *for_lcur++);
#وإلا /* __U_BOOT__ */
			/* We cannot use xasprintf, so we emulate it. */
			حرف *پر_var;
			حرف *var = pi->cmds[0].argv[0];
			حرف *val = *for_lcur++;

			/* + 1 to take into account =. */
			پر_var = xmalloc(طول_رشته(var) + طول_رشته(val) + 1);
			sprintf(پر_var, "%s=%s", var, val);

			مجموعه_local_var_modern(پر_var, /*flag:*/ 0);
#پایان_اگر /* __U_BOOT__ */
			استمر;
		}
		إذا (rword == RES_درون) {
			استمر; /* "for v IN list;..." - "in" has no cmds anyway */
		}
		إذا (rword == RES_انجامشده) {
			استمر; /* "done" has no cmds too */
		}
#پایان_اگر
#إذا ENABLE_HUSH_CASE
		إذا (rword == RES_CASE) {
			debug_printf_exec("CASE cond_code:%d\n", cond_کد);
			case_کلمه = expand_string_مقصد_string(pi->cmds->argv[0],
				EXP_FLAG_ESC_GLOB_CHARS, /*unbackslash:*/ 1);
			debug_printf_exec("CASE word1:'%s'\n", case_کلمه);
			//unbackslash(case_word);
			//debug_printf_exec("CASE word2:'%s'\n", case_word);
			استمر;
		}
		إذا (rword == RES_تطابق) {
			حرف **argv;

			debug_printf_exec("MATCH cond_code:%d\n", cond_کد);
			إذا (!case_کلمه) /* "case ... matched_word) ... WORD)": we executed selected branch, stop */
				اكسر;
			/* all prev words didn't match, does this one match? */
			argv = pi->cmds->argv;
			طالما (*argv) {
				حرف *الگو;
				debug_printf_exec("expand_string_to_string('%s')\n", *argv);
				الگو = expand_string_مقصد_string(*argv,
						EXP_FLAG_ESC_GLOB_CHARS,
						/*unbackslash:*/ 0
				);
				/* TODO: which FNM_xxx flags to use? */
				cond_کد = (fnmatch(الگو, case_کلمه, /*flags:*/ 0) != 0);
				debug_printf_exec("cond_code=fnmatch(pattern:'%s',str:'%s'):%d\n",
						الگو, case_کلمه, cond_کد);
				حرر(الگو);
				إذا (cond_کد == 0) {
					/* match! we will execute this branch */
					حرر(case_کلمه);
					case_کلمه = عدم; /* make future "word)" stop */
					اكسر;
				}
				argv++;
			}
			استمر;
		}
		إذا (rword == RES_CASE_BODY) { /* inside of a case branch */
			debug_printf_exec("CASE_BODY cond_code:%d\n", cond_کد);
			إذا (cond_کد != 0)
				استمر; /* not matched yet, skip this pipe */
		}
		إذا (rword == RES_ESAC) {
			debug_printf_exec("ESAC cond_code:%d\n", cond_کد);
			إذا (case_کلمه) {
				/* "case" did not match anything: still set $? (to 0) */
				G.آخرین_exitcode = rcode = EXIT_SUCCESS;
			}
		}
#پایان_اگر
		/* Just pressing <enter> in shell should check for jobs.
		 * OTOH, in non-interactive shell this is useless
		 * and only leads to extra job checks */
		إذا (pi->رقم_cmds == 0) {
#اگر_تعریف_نشده __U_BOOT__
			إذا (G_interactive_fd)
				اذهب check_jobs_and_continue;
#پایان_اگر /* !__U_BOOT__ */
			استمر;
		}

		/* After analyzing all keywords and conditions, we decided
		 * to execute this pipe. NB: have to do checkjobs(NULL)
		 * after run_pipe to collect any background children,
		 * even if list execution is to be stopped. */
		debug_printf_exec(": run_pipe with %d members\n", pi->رقم_cmds);
#اگر_تعریف_نشده __U_BOOT__
#إذا ENABLE_HUSH_LOOPS
		G.flag_break_continue = 0;
#پایان_اگر
#پایان_اگر /* !__U_BOOT__ */
#اگر_تعریف_نشده __U_BOOT__
		rcode = r = G.o_opt[OPT_O_NOEXEC] ? 0 : پیاپی_pipe(pi);
		/* NB: rcode is a smalluint, r is int */
#وإلا /* __U_BOOT__ */
		rcode = r = پیاپی_pipe(pi); /* NB: rcode is a smalluint, r is int */
		إذا (r <= EXIT_RET_کد) {
			صحيح قبلی_rcode = G.آخرین_exitcode;
			/*
			 * This magic is to get the exit code given by the user.
			 * Contrary to old shell code, we use + EXIT_RET_CODE as EXIT_RET_CODE
			 * equals -2.
			 */
			G.آخرین_exitcode = -r + EXIT_RET_کد;

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
			إذا (قبلی_rcode != r) {
				/*
				 * If run from run_command, run_command_flags will be set, so we check
				 * this to know if we are in main input shell.
				 */
				إذا (!G.پیاپی_command_flags)
					چاپ_کن("exit not allowed from main input shell.\n");

				اكسر;
			}
			استمر;
		}
#پایان_اگر /* __U_BOOT__ */
		إذا (r != -1) {
			/* We ran a builtin, function, or group.
			 * rcode is already known
			 * and we don't need to wait for anything. */
			debug_printf_exec(": builtin/func exitcode %d\n", rcode);
			G.آخرین_exitcode = rcode;
#اگر_تعریف_نشده __U_BOOT__
			check_and_پیاپی_traps();
#پایان_اگر /* !__U_BOOT__ */
#إذا ENABLE_HUSH_TRAP && ENABLE_HUSH_FUNCTIONS
			rcode = G.آخرین_exitcode; /* "return" in trap can change it, read back */
#پایان_اگر
#اگر_تعریف_نشده __U_BOOT__
#إذا ENABLE_HUSH_LOOPS
			/* Was it "break" or "continue"? */
			إذا (G.flag_break_continue) {
				smallint fbc = G.flag_break_continue;
				/* We might fall into outer *loop*,
				 * don't want to break it too */
				إذا (loop_بالاترین) {
					G.عمق_break_continue--;
					إذا (G.عمق_break_continue == 0)
						G.flag_break_continue = 0;
					/* else: e.g. "continue 2" should *break* once, *then* continue */
				} /* else: "while... do... { we are here (innermost list is not a loop!) };...done" */
				إذا (G.عمق_break_continue != 0 || fbc == BC_BREAK) {
					checkjobs(عدم, 0 /*(no pid to wait for)*/);
					اكسر;
				}
				/* "continue": simulate end of loop */
				rword = RES_انجامشده;
				استمر;
			}
#پایان_اگر
			إذا (G_flag_return_درون_پیشرفت == 1) {
				checkjobs(عدم, 0 /*(no pid to wait for)*/);
				اكسر;
			}

		} وإلا إذا (pi->followup == PIPE_BG) {
			/* What does bash do with attempts to background builtins? */
			/* even bash 3.2 doesn't do that well with nested bg:
			 * try "{ { sleep 10; echo DEEP; } & echo HERE; } &".
			 * I'm NOT treating inner &'s as jobs */
#إذا ENABLE_HUSH_کار
			إذا (G.پیاپی_list_level == 1)
				درج_کار_into_جدول(pi);
#پایان_اگر
			/* Last command's pid goes to $! */
			G.آخرین_bg_pid = pi->cmds[pi->رقم_cmds - 1].pid;
			G.آخرین_bg_pid_exitcode = 0;
			debug_printf_exec(": cmd&: exitcode EXIT_SUCCESS\n");
/* Check pi->pi_inverted? "! sleep 1 & echo $?": bash says 1. dash and ash say 0 */
			rcode = EXIT_SUCCESS;
			اذهب check_traps;
		} وإلا {
#إذا ENABLE_HUSH_کار
			إذا (G.پیاپی_list_level == 1 && G_interactive_fd) {
				/* Waits for completion, then fg's main shell */
				rcode = checkjobs_and_fg_shell(pi);
				debug_printf_exec(": checkjobs_and_fg_shell exitcode %d\n", rcode);
				اذهب check_traps;
			}
#پایان_اگر
			/* This one just waits for completion */
			rcode = checkjobs(pi, 0 /*(no pid to wait for)*/);
			debug_printf_exec(": checkjobs exitcode %d\n", rcode);
 check_traps:
			G.آخرین_exitcode = rcode;
			check_and_پیاپی_traps();
#إذا ENABLE_HUSH_TRAP && ENABLE_HUSH_FUNCTIONS
			rcode = G.آخرین_exitcode; /* "return" in trap can change it, read back */
#پایان_اگر
		}
#پایان_اگر /* !__U_BOOT__ */

#اگر_تعریف_نشده __U_BOOT__
		/* Handle "set -e" */
		إذا (rcode != 0 && G.o_opt[OPT_O_ERREXIT]) {
			debug_printf_exec("ERREXIT:1 errexit_depth:%d\n", G.errexit_عمق);
			إذا (G.errexit_عمق == 0)
				hush_exit(rcode);
		}
#وإلا /* __U_BOOT__ */
		} /* if (r != -1) */
#پایان_اگر /* __U_BOOT__ */
		G.errexit_عمق = sv_errexit_عمق;

		/* Analyze how result affects subsequent commands */
#إذا ENABLE_HUSH_IF
		إذا (rword == RES_IF || rword == RES_ELIF) {
			debug_printf_exec("cond_code=rcode:%d\n", rcode);
			cond_کد = rcode;
		}
#پایان_اگر
#اگر_تعریف_نشده __U_BOOT__
 check_jobs_and_continue:
		checkjobs(عدم, 0 /*(no pid to wait for)*/);
#پایان_اگر /* !__U_BOOT__ */
 dont_check_jobs_but_continue: ;
#إذا ENABLE_HUSH_LOOPS
		/* Beware of "while false; true; do ..."! */
		إذا (pi->بعدی
		 && (pi->بعدی->res_کلمه == RES_DO || pi->بعدی->res_کلمه == RES_انجامشده)
		 /* check for RES_DONE is needed for "while ...; do \n done" case */
		) {
			إذا (rword == RES_WHILE) {
				إذا (rcode) {
					/* "while false; do...done" - exitcode 0 */
					G.آخرین_exitcode = rcode = EXIT_SUCCESS;
					debug_printf_exec(": while expr is false: breaking (exitcode:EXIT_SUCCESS)\n");
					اكسر;
				}
			}
			إذا (rword == RES_UNTIL) {
				إذا (!rcode) {
					debug_printf_exec(": until expr is true: breaking\n");
					اكسر;
				}
			}
		}
#پایان_اگر
	} /* for (pi) */

#إذا ENABLE_HUSH_کار
	G.پیاپی_list_level--;
#پایان_اگر
#إذا ENABLE_HUSH_LOOPS
	إذا (loop_بالاترین)
		G.عمق_از_loop--;
	حرر(for_list);
#پایان_اگر
#إذا ENABLE_HUSH_CASE
	حرر(case_کلمه);
#پایان_اگر
#اگر_تعریف_نشده __U_BOOT__
	debug_leave();
	debug_printf_exec("run_list lvl %d return %d\n", G.پیاپی_list_level, rcode);
#پایان_اگر /* !__U_BOOT__ */
	أرجع rcode;
}

/* Select which version we will use */
ساكن صحيح پیاپی_and_free_list(بنية pipe *pi)
{
	صحيح rcode = 0;
	debug_printf_exec("run_and_free_list entered\n");
#اگر_تعریف_نشده __U_BOOT__
	إذا (!G.o_opt[OPT_O_NOEXEC]) {
#پایان_اگر /* !__U_BOOT__ */
		debug_printf_exec(": run_list: 1st pipe with %d cmds\n", pi->رقم_cmds);
		rcode = پیاپی_list(pi);
#اگر_تعریف_نشده __U_BOOT__
	}
#پایان_اگر /* !__U_BOOT__ */
	/* free_pipe_list has the side effect of clearing memory.
	 * In the long run that function can be merged with run_list,
	 * but doing that now would hobble the debugging effort. */
	free_pipe_list(pi);
	debug_printf_exec("run_and_free_list return %d\n", rcode);
	أرجع rcode;
}

#اگر_تعریف_نشده __U_BOOT__
ساكن فراغ install_sighandlers(بیعلامت ماسک)
{
	sighandler_t old_handler;
	بیعلامت sig = 0;
	طالما ((ماسک >>= 1) != 0) {
		sig++;
		إذا (!(ماسک & 1))
			استمر;
		old_handler = install_sighandler(sig, pick_sighandler(sig));
		/* POSIX allows shell to re-enable SIGCHLD
		 * even if it was SIG_IGN on entry.
		 * Therefore we skip IGN check for it:
		 */
		إذا (sig == SIGCHLD)
			استمر;
		/* Interactive bash re-enables SIGHUP which is SIG_IGNed on entry.
		 * Try:
		 * trap '' hup; bash; echo RET  # type "kill -hup $$", see SIGHUP having effect
		 * trap '' hup; bash -c 'kill -hup $$; echo ALIVE'  # here SIGHUP is SIG_IGNed
		 */
		إذا (sig == SIGHUP && G_interactive_fd)
			استمر;
		/* Unless one of the above signals, is it SIG_IGN? */
		إذا (old_handler == SIG_IGN) {
			/* oops... restore back to IGN, and record this fact */
			install_sighandler(sig, old_handler);
#إذا ENABLE_HUSH_TRAP
			إذا (!G_traps)
				G_traps = xzalloc(حجم(G_traps[0]) * NSIG);
			حرر(G_traps[sig]);
			G_traps[sig] = xzalloc(1); /* == xstrdup(""); */
#پایان_اگر
		}
	}
}

/* Called a few times only (or even once if "sh -c") */
ساكن فراغ install_special_sighandlers(فراغ)
{
	بیعلامت ماسک;

	/* Which signals are shell-special? */
	ماسک = (1 << SIGQUIT) | (1 << SIGCHLD);
	إذا (G_interactive_fd) {
		ماسک |= SPECIAL_INTERACTIVE_SIGS;
		إذا (G_saved_tty_pgrp) /* we have ctty, job control sigs work */
			ماسک |= SPECIAL_JOBSTOP_SIGS;
	}
	/* Careful, do not re-install handlers we already installed */
	إذا (G.special_sig_ماسک != ماسک) {
		بیعلامت diff = ماسک & ~G.special_sig_ماسک;
		G.special_sig_ماسک = ماسک;
		install_sighandlers(diff);
	}
}

#إذا ENABLE_HUSH_کار
/* helper */
/* Set handlers to restore tty pgrp and exit */
ساكن فراغ install_fatal_sighandlers(فراغ)
{
	بیعلامت ماسک;

	/* We will restore tty pgrp on these signals */
	ماسک = 0
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
	G_fatal_sig_ماسک = ماسک;

	install_sighandlers(ماسک);
}
#پایان_اگر

ساكن صحيح مجموعه_نما(صحيح state, حرف نما, ثابت حرف *o_opt)
{
	صحيح idx;
	بدل (نما) {
	حالة 'n':
		/* set -n has no effect in interactive shell */
		/* Try: while set -n; do echo $-; done */
		إذا (!G_interactive_fd)
			G.o_opt[OPT_O_NOEXEC] = state;
		اكسر;
	حالة 'x':
		IF_HUSH_نما_X(G_x_نما = state;)
		IF_HUSH_نما_X(إذا (G.x_نما_fd <= 0) G.x_نما_fd = dup_CLOEXEC(2, 10);)
		اكسر;
	حالة 'e':
		G.o_opt[OPT_O_ERREXIT] = state;
		اكسر;
	حالة 'o':
		إذا (!o_opt) {
			/* "set -o" or "set +o" without parameter.
			 * in bash, set -o produces this output:
			 *  pipefail        off
			 * and set +o:
			 *  set +o pipefail
			 * We always use the second form.
			 */
			ثابت حرف *p = o_opt_رشتهها;
			idx = 0;
			طالما (*p) {
				چاپ_کن("set %co %s\n", (G.o_opt[idx] ? '-' : '+'), p);
				idx++;
				p += طول_رشته(p) + 1;
			}
			اكسر;
		}
		idx = شاخص_درون_رشتهها(o_opt_رشتهها, o_opt);
		إذا (idx >= 0) {
			G.o_opt[idx] = state;
			اكسر;
		}
		/* fall through to error */
	افتراضي:
		أرجع EXIT_FAILURE;
	}
	أرجع EXIT_SUCCESS;
}

صحيح hush_main(صحيح argc, حرف **argv) MAIN_EXTERNALLY_VISIBLE;
صحيح hush_main(صحيح argc, حرف **argv)
{
	pid_t cached_getpid;
	تعداد {
		OPT_login = (1 << 0),
	};
	بیعلامت flags;
#إذا !BB_MMU
	بیعلامت builtin_argc = 0;
#پایان_اگر
	حرف **e;
	بنية variable *cur_var;
	بنية variable *shell_ver;

	INIT_G();
	إذا (EXIT_SUCCESS != 0) /* if EXIT_SUCCESS == 0, it is already done */
		G.آخرین_exitcode = EXIT_SUCCESS;
#إذا !BB_MMU
	/* "Big heredoc" support via "sh -< STRING" invocation.
	 * Check it first (do not bother to run the usual init code,
	 * it is not needed for this case).
	 */
	إذا (argv[1]
	 && argv[1][0] == '-' && argv[1][1] == '<' /*&& !argv[1][2]*/
	 /*&& argv[2] && !argv[3] - we don't check some conditions */
	) {
		پر_write1_str(argv[2]);
		_exit(0);
	}
	G.argv0_for_re_execing = argv[0];
#پایان_اگر
#إذا ENABLE_HUSH_TRAP
# إذا ENABLE_HUSH_FUNCTIONS
	G.return_exitcode = -1;
# endif
	G.pre_trap_exitcode = -1;
#پایان_اگر

#إذا ENABLE_HUSH_FAST
	G.شمار_SIGCHLD++; /* ensure it is != G.handled_SIGCHLD */
#پایان_اگر

	cached_getpid = getpid();   /* for tcsetpgrp() during init */
	G.ریشه_pid = cached_getpid; /* for $PID  (NOMMU can override via -$HEXPID:HEXPPID:...) */
	G.ریشه_ppid = getppid();    /* for $PPID (NOMMU can override) */

	/* Deal with HUSH_VERSION */
	debug_printf_env("unsetenv '%s'\n", "HUSH_VERSION");
	unsetenv("HUSH_VERSION"); /* in case it exists in initial env */
	shell_ver = xzalloc(حجم(*shell_ver));
	shell_ver->flg_export = 1;
	shell_ver->flg_read_only = 1;
	/* Code which handles ${var<op>...} needs writable values for all variables,
	 * therefore we xstrdup: */
	shell_ver->varstr = xstrdup(hush_نسخه_str);

	/* Create shell local variables from the values
	 * currently living in the environment */
	G.بالاترین_var = shell_ver;
	cur_var = G.بالاترین_var;
	e = environ;
	إذا (e) طالما (*e) {
		حرف *مقدار = strchr(*e, '=');
		إذا (مقدار) { /* paranoia */
			cur_var->بعدی = xzalloc(حجم(*cur_var));
			cur_var = cur_var->بعدی;
			cur_var->varstr = *e;
			cur_var->max_len = طول_رشته(*e);
			cur_var->flg_export = 1;
		}
		e++;
	}
	/* (Re)insert HUSH_VERSION into env (AFTER we scanned the env!) */
	debug_printf_env("putenv '%s'\n", shell_ver->varstr);
	putenv(shell_ver->varstr);

	/* Export PWD */
	مجموعه_pwd_var(SETFLAG_EXPORT);

#إذا BASH_HOSTNAME_VAR
	/* Set (but not export) HOSTNAME unless already set */
	إذا (!get_local_var_مقدار("HOSTNAME")) {
		بنية utsname uts;
		uname(&uts);
		مجموعه_local_var_from_halves("HOSTNAME", uts.nodename);
	}
#پایان_اگر
	/* IFS is not inherited from the parent environment */
	مجموعه_local_var_from_halves("IFS", defifs);

	إذا (!get_local_var_مقدار("PATH"))
		مجموعه_local_var_from_halves("PATH", bb_default_ریشه_مسیر);

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

#إذا رقم_SCRIPTS > 0
	إذا (argc < 0) {
		حرف *script = get_script_content(-argc - 1);
		G.global_argv = argv;
		G.global_argc = string_array_len(argv);
		//install_special_sighandlers(); - needed?
		parse_and_پیاپی_string(script);
		اذهب final_return;
	}
#پایان_اگر

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
	طالما (1) {
		صحيح opt = getopt(argc, argv, "+" /* stop at 1st non-option */
				"cexinsl"
#إذا !BB_MMU
				"$:R:V:"
# إذا ENABLE_HUSH_LINENO_VAR
				"L:"
# endif
# إذا ENABLE_HUSH_FUNCTIONS
				"F:"
# endif
#پایان_اگر
		);
		إذا (opt <= 0)
			اكسر;
		بدل (opt) {
		حالة 'c':
			/* Note: -c is not an option with param!
			 * "hush -c -l SCRIPT" is valid. "hush -cSCRIPT" is not.
			 */
			G.opt_c = 1;
			اكسر;
		حالة 'i':
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
//اینجا
//(under "bash", it's the opposite: it runs $BASH_ENV file only *without* -i).
//
//ash -i -c 'sleep 3; sleep 3', on ^C, drops into a prompt instead of exiting
//(this may be a bug, bash does not do this).
//(ash -i -c 'sleep 3' won't show this, the last command gets auto-"exec"ed)
//
//None of the above feel like useful features people would rely on.
			اكسر;
		حالة 's':
			G.opt_s = 1;
			اكسر;
		حالة 'l':
			flags |= OPT_login;
			اكسر;
#إذا !BB_MMU
		حالة '$': {
			بیعلامت دراز دراز خالی_trap_ماسک;

			G.ریشه_pid = bb_strtou(optarg, &optarg, 16);
			optarg++;
			G.ریشه_ppid = bb_strtou(optarg, &optarg, 16);
			optarg++;
			G.آخرین_bg_pid = bb_strtou(optarg, &optarg, 16);
			optarg++;
			G.آخرین_exitcode = bb_strtou(optarg, &optarg, 16);
			optarg++;
			builtin_argc = bb_strtou(optarg, &optarg, 16);
			optarg++;
			خالی_trap_ماسک = bb_strtoull(optarg, &optarg, 16);
			إذا (خالی_trap_ماسک != 0) {
				IF_HUSH_TRAP(صحيح sig;)
				install_special_sighandlers();
# إذا ENABLE_HUSH_TRAP
				G_traps = xzalloc(حجم(G_traps[0]) * NSIG);
				لكل (sig = 1; sig < NSIG; sig++) {
					إذا (خالی_trap_ماسک & (1LL << sig)) {
						G_traps[sig] = xzalloc(1); /* == xstrdup(""); */
						install_sighandler(sig, SIG_IGN);
					}
				}
# endif
			}
# إذا ENABLE_HUSH_LOOPS
			optarg++;
			G.عمق_از_loop = bb_strtou(optarg, &optarg, 16);
# endif
			/* Suppress "killed by signal" message, -$ hack is used
			 * for subshells: echo `sh -c 'kill -9 $$'`
			 * should be silent.
			 */
			IF_HUSH_کار(G.پیاپی_list_level = 1;)
# إذا ENABLE_HUSH_FUNCTIONS
			/* nommu uses re-exec trick for "... | func | ...",
			 * should allow "return".
			 * This accidentally allows returns in subshells.
			 */
			G_flag_return_درون_پیشرفت = -1;
# endif
			اكسر;
		}
		حالة 'R':
		حالة 'V':
			مجموعه_local_var(xstrdup(optarg), opt == 'R' ? SETFLAG_MAKE_RO : 0);
			اكسر;
# إذا ENABLE_HUSH_LINENO_VAR
		حالة 'L':
			G.parse_lineno = xatou(optarg);
			اكسر;
# endif
# إذا ENABLE_HUSH_FUNCTIONS
		حالة 'F': {
			بنية function *funcp = new_function(optarg);
			/* funcp->name is already set to optarg */
			/* funcp->body is set to NULL. It's a special case. */
			funcp->body_as_string = argv[optind];
			optind++;
			اكسر;
		}
# endif
#پایان_اگر
		/*case '?': invalid option encountered (set_mode('?') will fail) */
		/*case 'n':*/
		/*case 'x':*/
		/*case 'e':*/
		افتراضي:
			إذا (مجموعه_نما(1, opt, عدم) == 0) /* no error */
				اكسر;
			bb_show_usage();
		}
	} /* option parsing loop */

	/* Skip options. Try "hush -l": $1 should not be "-l"! */
	G.global_argc = argc - (optind - 1);
	G.global_argv = argv + (optind - 1);
	G.global_argv[0] = argv[0];

	/* If we are login shell... */
	إذا (flags & OPT_login) {
		ثابت حرف *hp = عدم;
		HFILE *درون_داد;

		debug_printf("sourcing /etc/profile\n");
		درون_داد = hfopen("/etc/profile");
 پیاپی_profile:
		إذا (درون_داد != عدم) {
			install_special_sighandlers();
			parse_and_پیاپی_file(درون_داد);
			hfclose(درون_داد);
		}
		/* bash: after sourcing /etc/profile,
		 * tries to source (in the given order):
		 * ~/.bash_profile, ~/.bash_login, ~/.profile,
		 * stopping on first found. --noprofile turns this off.
		 * bash also sources ~/.bash_logout on exit.
		 * If called as sh, skips .bash_XXX files.
		 */
		إذا (!hp) { /* unless we looped on the "goto" already */
			hp = get_local_var_مقدار("HOME");
			إذا (hp && hp[0]) {
				debug_printf("sourcing ~/.profile\n");
				hp = concat_مسیر_file(hp, ".profile");
				درون_داد = hfopen(hp);
				حرر((حرف*)hp);
				اذهب پیاپی_profile;
			}
		}
	}

#اگر_تعریف_نشده __U_BOOT__
	/* -c takes effect *after* -l */
	إذا (G.opt_c) {
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
#إذا !BB_MMU
		إذا (builtin_argc) {
			/* -c 'builtin' [BARGV...] "" ARG0 [ARG1...] */
			ثابت بنية built_درون_command *x;
			x = پیدا_کن_builtin(G.global_argv[0]);
			إذا (x) { /* paranoia */
				argv = G.global_argv;
				G.global_argc -= builtin_argc + 1; /* skip [BARGV...] "" */
				G.global_argv += builtin_argc + 1;
				G.global_argv[-1] = عدم; /* replace "" */
				G.آخرین_exitcode = x->b_function(argv);
			}
			اذهب final_return;
		}
#پایان_اگر

		script = G.global_argv[0];
		إذا (!script)
			bb_error_پیام_and_die(bb_پیام_requires_arg, "-c");
		إذا (!G.global_argv[1]) {
			/* -c 'script' (no params): prevent empty $0 */
			G.global_argv[0] = argv[0];
		} وإلا { /* else -c 'script' ARG0 [ARG1...]: $0 is ARG0 */
			G.global_argc--;
			G.global_argv++;
		}
		parse_and_پیاپی_string(script);
		اذهب final_return;
	}

	/* -s is: hush -s ARGV1 ARGV2 (no SCRIPT) */
	إذا (!G.opt_s && G.global_argv[1]) {
		HFILE *درون_داد;
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
		درون_داد = hfopen(G.global_argv[0]);
		إذا (!درون_داد) {
			bb_simple_perror_پیام_and_die(G.global_argv[0]);
		}
		xfunc_error_retval = 1;
		install_special_sighandlers();
		parse_and_پیاپی_file(درون_داد);
#إذا ENABLE_FEATURE_CLEAN_به_بالا
		hfclose(درون_داد);
#پایان_اگر
		اذهب final_return;
	}
	/* "implicit" -s: bare interactive hush shows 's' in $- */
	G.opt_s = 1;

#پایان_اگر /* __U_BOOT__ */
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
#إذا ENABLE_HUSH_کار
	إذا (isatty(STDIN_FILENO) && isatty(STDOUT_FILENO)) {
		G_saved_tty_pgrp = tcgetpgrp(STDIN_FILENO);
		debug_printf("saved_tty_pgrp:%d\n", G_saved_tty_pgrp);
		إذا (G_saved_tty_pgrp < 0)
			G_saved_tty_pgrp = 0;

		/* try to dup stdin to high fd#, >= 255 */
		G_interactive_fd = dup_CLOEXEC(STDIN_FILENO, 254);
		إذا (G_interactive_fd < 0) {
			/* try to dup to any fd */
			G_interactive_fd = dup_CLOEXEC(STDIN_FILENO, -1);
			إذا (G_interactive_fd < 0) {
				/* give up */
				G_interactive_fd = 0;
				G_saved_tty_pgrp = 0;
			}
		}
	}
	debug_printf("interactive_fd:%d\n", G_interactive_fd);
	إذا (G_interactive_fd) {
		إذا (G_saved_tty_pgrp) {
			/* If we were run as 'hush &', sleep until we are
			 * in the foreground (tty pgrp == our pgrp).
			 * If we get started under a job aware app (like bash),
			 * make sure we are now in charge so we don't fight over
			 * who gets the foreground */
			طالما (1) {
				pid_t shell_pgrp = getpgrp();
				G_saved_tty_pgrp = tcgetpgrp(G_interactive_fd);
				إذا (G_saved_tty_pgrp == shell_pgrp)
					اكسر;
				/* send TTIN to ourself (should stop us) */
				kill(- shell_pgrp, SIGTTIN);
			}
		}

		/* Install more signal handlers */
		install_special_sighandlers();

		إذا (G_saved_tty_pgrp) {
			/* Set other signals to restore saved_tty_pgrp */
			install_fatal_sighandlers();
			/* Put ourselves in our own process group
			 * (bash, too, does this only if ctty is available) */
			bb_setpgrp(); /* is the same as setpgid(our_pid, our_pid); */
			/* Grab control of the terminal */
			tcsetpgrp(G_interactive_fd, cached_getpid);
		}
		enable_restore_tty_pgrp_on_exit();

# إذا ENABLE_FEATURE_EDITING
		G.سطر_درون_داد_state = new_سطر_درون_داد_t(FOR_SHELL);
#  إذا ENABLE_FEATURE_TAB_COMPLETION
		G.سطر_درون_داد_state->get_exe_name = hush_command_name;
#  endif
#  إذا EDITING_HAS_sh_get_var
		G.سطر_درون_داد_state->sh_get_var = get_local_var_مقدار;
#  endif
# endif
# إذا ENABLE_HUSH_SAVEHISTORY && MAX_HISTORY > 0
		{
			ثابت حرف *hp = get_local_var_مقدار("HISTFILE");
			إذا (!hp) {
				hp = get_local_var_مقدار("HOME");
				إذا (hp)
					hp = concat_مسیر_file(hp, ".hush_history");
			} وإلا {
				hp = xstrdup(hp);
			}
			إذا (hp) {
				G.سطر_درون_داد_state->hist_file = hp;
				//set_local_var(xasprintf("HISTFILE=%s", ...));
			}
#  إذا ENABLE_FEATURE_SH_HISTFILESIZE
			hp = get_local_var_مقدار("HISTFILESIZE");
			G.سطر_درون_داد_state->max_history = اندازه_from_HISTFILESIZE(hp);
#  endif
		}
# endif
	} وإلا {
		install_special_sighandlers();
	}
#وإلاإذا ENABLE_HUSH_INTERACTIVE
	/* No job control compiled in, only prompt/line editing */
	إذا (isatty(STDIN_FILENO) && isatty(STDOUT_FILENO)) {
		G_interactive_fd = dup_CLOEXEC(STDIN_FILENO, 254);
		إذا (G_interactive_fd < 0) {
			/* try to dup to any fd */
			G_interactive_fd = dup_CLOEXEC(STDIN_FILENO, -1);
			إذا (G_interactive_fd < 0)
				/* give up */
				G_interactive_fd = 0;
		}
	}
	install_special_sighandlers();
#وإلا
	/* We have interactiveness code disabled */
	install_special_sighandlers();
#پایان_اگر
	/* bash:
	 * if interactive but not a login shell, sources ~/.bashrc
	 * (--norc turns this off, --rcfile <file> overrides)
	 */

	إذا (G_interactive_fd) {
#إذا ENABLE_HUSH_INTERACTIVE && ENABLE_FEATURE_EDITING_FANCY_PROMPT
		/* Set (but not export) PS1/2 unless already set */
		إذا (!get_local_var_مقدار("PS1"))
			مجموعه_local_var_from_halves("PS1", "\\w \\$ ");
		إذا (!get_local_var_مقدار("PS2"))
			مجموعه_local_var_from_halves("PS2", "> ");
#پایان_اگر
		إذا (!ENABLE_FEATURE_SH_EXTRA_QUIET) {
			/* note: ash and hush share this string */
			چاپ_کن("\n\n%s %s\n"
				IF_HUSH_HELP("Enter 'help' for a list of built-in commands.\n")
				"\n",
				bb_banner,
				"hush - the humble shell"
			);
		}
	}

	parse_and_پیاپی_file(hfopen(عدم)); /* stdin */

 final_return:
	hush_exit(G.آخرین_exitcode);
}

/*
 * Built-ins
 */
ساكن صحيح FAST_FUNC builtin_true(حرف **argv استفادهنشده_PARAM)
{
	أرجع 0;
}

ساكن صحيح FAST_FUNC builtin_false(حرف **argv استفادهنشده_PARAM)
{
	أرجع 1;
}

#إذا ENABLE_HUSH_TEST || ENABLE_HUSH_ECHO || ENABLE_HUSH_PRINTF || ENABLE_HUSH_KILL
ساكن NOINLINE صحيح پیاپی_applet_main(حرف **argv, صحيح (*applet_main_func)(صحيح argc, حرف **argv))
{
	صحيح argc = string_array_len(argv);
	أرجع applet_main_func(argc, argv);
}
#پایان_اگر
#إذا ENABLE_HUSH_TEST || BASH_TEST2
ساكن صحيح FAST_FUNC builtin_test(حرف **argv)
{
	أرجع پیاپی_applet_main(argv, test_main);
}
#پایان_اگر
#إذا ENABLE_HUSH_ECHO
ساكن صحيح FAST_FUNC builtin_echo(حرف **argv)
{
	أرجع پیاپی_applet_main(argv, echo_main);
}
#پایان_اگر
#إذا ENABLE_HUSH_PRINTF
ساكن صحيح FAST_FUNC builtin_printf(حرف **argv)
{
	أرجع پیاپی_applet_main(argv, printf_main);
}
#پایان_اگر

#إذا ENABLE_HUSH_HELP
ساكن صحيح FAST_FUNC builtin_help(حرف **argv استفادهنشده_PARAM)
{
	ثابت بنية built_درون_command *x;

	چاپ_کن(
		"Built-in commands:\n"
		"------------------\n");
	لكل (x = bltins1; x != &bltins1[ARRAY_اندازه(bltins1)]; x++) {
		إذا (x->b_descr)
			چاپ_کن("%-10s%s\n", x->b_cmd, x->b_descr);
	}
	أرجع EXIT_SUCCESS;
}
#پایان_اگر

#إذا MAX_HISTORY && ENABLE_FEATURE_EDITING
ساكن صحيح FAST_FUNC builtin_history(حرف **argv استفادهنشده_PARAM)
{
	show_history(G.سطر_درون_داد_state);
	أرجع EXIT_SUCCESS;
}
#پایان_اگر

ساكن صحيح FAST_FUNC builtin_cd(حرف **argv)
{
	ثابت حرف *newdir;

	argv = skip_dash_dash(argv);
	newdir = argv[0];
	إذا (newdir == عدم) {
		/* bash does nothing (exitcode 0) if HOME is ""; if it's unset,
		 * bash says "bash: cd: HOME not set" and does nothing
		 * (exitcode 1)
		 */
		ثابت حرف *home = get_local_var_مقدار("HOME");
		newdir = home ? home : "/";
	}
	إذا (chdir(newdir)) {
		/* Mimic bash message exactly */
		bb_perror_پیام("cd: %s", newdir);
		أرجع EXIT_FAILURE;
	}
	/* Read current dir (get_cwd(1) is inside) and set PWD.
	 * Note: do not enforce exporting. If PWD was unset or unexported,
	 * set it again, but do not export. bash does the same.
	 */
	مجموعه_pwd_var(/*flag:*/ 0);
	أرجع EXIT_SUCCESS;
}

ساكن صحيح FAST_FUNC builtin_pwd(حرف **argv استفادهنشده_PARAM)
{
	puts(get_cwd(0));
	أرجع EXIT_SUCCESS;
}

ساكن صحيح FAST_FUNC builtin_eval(حرف **argv)
{
	argv = skip_dash_dash(argv);

	إذا (!argv[0])
		أرجع EXIT_SUCCESS;

	IF_HUSH_نما_X(G.x_نما_عمق++;)
	//bb_error_msg("%s: ++x_mode_depth=%d", __func__, G.x_mode_depth);
	إذا (!argv[1]) {
		/* bash:
		 * eval "echo Hi; done" ("done" is syntax error):
		 * "echo Hi" will not execute too.
		 */
		parse_and_پیاپی_string(argv[0]);
	} وإلا {
		/* "The eval utility shall construct a command by
		 * concatenating arguments together, separating
		 * each with a <space> character."
		 */
		حرف *str, *p;
		بیعلامت len = 0;
		حرف **pp = argv;
		افعل
			len += طول_رشته(*pp) + 1;
		طالما (*++pp);
		str = p = xmalloc(len);
		pp = argv;
		لكل (;;) {
			p = stpcpy(p, *pp);
			pp++;
			إذا (!*pp)
				اكسر;
			*p++ = ' ';
		}
		parse_and_پیاپی_string(str);
		حرر(str);
	}
	IF_HUSH_نما_X(G.x_نما_عمق--;)
	//bb_error_msg("%s: --x_mode_depth=%d", __func__, G.x_mode_depth);
	أرجع G.آخرین_exitcode;
}

ساكن صحيح FAST_FUNC builtin_exec(حرف **argv)
{
	argv = skip_dash_dash(argv);
	إذا (argv[0] == عدم)
		أرجع EXIT_SUCCESS; /* bash does this */

	/* Careful: we can end up here after [v]fork. Do not restore
	 * tty pgrp then, only top-level shell process does that */
	إذا (G_saved_tty_pgrp && getpid() == G.ریشه_pid)
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

ساكن صحيح FAST_FUNC builtin_exit(حرف **argv)
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
	إذا (argv[0] == عدم) {
#إذا ENABLE_HUSH_TRAP
		إذا (G.pre_trap_exitcode >= 0) /* "exit" in trap uses $? from before the trap */
			hush_exit(G.pre_trap_exitcode);
#پایان_اگر
		hush_exit(G.آخرین_exitcode);
	}
	/* mimic bash: exit 123abc == exit 255 + error msg */
	xfunc_error_retval = 255;
	/* bash: exit -2 == exit 254, no error msg */
	hush_exit(xatoi(argv[0]) & 0xff);
}

#إذا ENABLE_HUSH_TYPE
/* http://www.opengroup.org/onlinepubs/9699919799/utilities/type.html */
ساكن صحيح FAST_FUNC builtin_type(حرف **argv)
{
	صحيح ret = EXIT_SUCCESS;

	طالما (*++argv) {
		ثابت حرف *type;
		حرف *مسیر_2 = عدم;

		إذا (0) {} /* make conditional compile easier below */
		/*else if (find_alias(*argv))
			type = "an alias";*/
# إذا ENABLE_HUSH_FUNCTIONS
		وإلا إذا (پیدا_کن_function(*argv))
			type = "a function";
# endif
		وإلا إذا (پیدا_کن_builtin(*argv))
			type = "a shell builtin";
		وإلا إذا ((مسیر_2 = پیدا_کن_درون_مسیر(*argv)) != عدم)
			type = مسیر_2;
		وإلا {
			bb_error_پیام("type: %s: not found", *argv);
			ret = EXIT_FAILURE;
			استمر;
		}

		چاپ_کن("%s is %s\n", *argv, type);
		حرر(مسیر_2);
	}

	أرجع ret;
}
#پایان_اگر

#إذا ENABLE_HUSH_READ
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
ساكن صحيح FAST_FUNC builtin_read(حرف **argv)
{
	ثابت حرف *r;
	بنية builtin_read_params params;

	memset(&params, 0, حجم(params));

	/* "!": do not abort on errors.
	 * Option string must start with "sr" to match BUILTIN_READ_xxx
	 */
	params.read_flags = getopt32(argv,
# إذا BASH_READ_D
		IF_NOT_HUSH_BASH_COMPAT("^")
		"!srn:p:t:u:d:" IF_NOT_HUSH_BASH_COMPAT("\0" "-1"/*min 1 arg*/),
		&params.opt_n, &params.opt_p, &params.opt_t, &params.opt_u, &params.opt_d
# وإلا
		IF_NOT_HUSH_BASH_COMPAT("^")
		"!srn:p:t:u:" IF_NOT_HUSH_BASH_COMPAT("\0" "-1"/*min 1 arg*/),
		&params.opt_n, &params.opt_p, &params.opt_t, &params.opt_u
# endif
//TODO: print "read: need variable name"
//for the case of !BASH "read" with no args (now it fails silently)
//(or maybe extend getopt32() to emit a message if "-1" fails)
	);
	إذا ((uint32_t)params.read_flags == (uint32_t)-1)
		أرجع EXIT_FAILURE;
	argv += optind;
	params.argv = argv;
	params.setvar = مجموعه_local_var_from_halves;
	params.ifs = get_local_var_مقدار("IFS"); /* can be NULL */

 again:
	r = shell_builtin_read(&params);

	إذا ((uintptr_t)r == 1 && errno == EINTR) {
		بیعلامت sig = check_and_پیاپی_traps();
		إذا (sig != SIGINT)
			اذهب again;
	}

	إذا ((uintptr_t)r > 1) {
		bb_simple_error_پیام(r);
		r = (حرف*)(uintptr_t)1;
	}

	أرجع (uintptr_t)r;
}
#پایان_اگر

#إذا ENABLE_HUSH_UMASK
ساكن صحيح FAST_FUNC builtin_umask(حرف **argv)
{
	صحيح rc;
	mode_t ماسک;

	rc = 1;
	ماسک = umask(0);
	argv = skip_dash_dash(argv);
	إذا (argv[0]) {
		mode_t old_ماسک = ماسک;

		/* numeric umasks are taken as-is */
		/* symbolic umasks are inverted: "umask a=rx" calls umask(222) */
		إذا (!آیا_رقم(argv[0][0]))
			ماسک ^= 0777;
		ماسک = bb_parse_نما(argv[0], ماسک);
		إذا (!آیا_رقم(argv[0][0]))
			ماسک ^= 0777;
		إذا ((بیعلامت)ماسک > 0777) {
			ماسک = old_ماسک;
			/* bash messages:
			 * bash: umask: 'q': invalid symbolic mode operator
			 * bash: umask: 999: octal number out of range
			 */
			bb_error_پیام("%s: invalid mode '%s'", "umask", argv[0]);
			rc = 0;
		}
	} وإلا {
		/* Mimic bash */
		چاپ_کن("%04o\n", (بیعلامت) ماسک);
		/* fall through and restore mask which we set to 0 */
	}
	umask(ماسک);

	أرجع !rc; /* rc != 0 - success */
}
#پایان_اگر

#إذا ENABLE_HUSH_EXPORT || ENABLE_HUSH_READONLY || ENABLE_HUSH_مجموعه || ENABLE_HUSH_TRAP
ساكن فراغ print_escaped(ثابت حرف *s)
{
//TODO? bash "set" does not quote variables which contain only alnums and "%+,-./:=@_~",
// (but "export" quotes all variables, even with only these chars).
// I think quoting strings with %+,=~ looks better
// (example: "set" printing var== instead of var='=' looks strange)
// IOW: do not quote "-./:@_": / is used in pathnames, : in PATH, -._ often in file names, @ in emails

	إذا (*s == '\'')
		اذهب squote;
	افعل {
		ثابت حرف *p = strchrnul(s, '\'');
		/* print 'xxxx', possibly just '' */
		چاپ_کن("'%.*s'", (صحيح)(p - s), s);
		إذا (*p == '\0')
			اكسر;
		s = p;
 squote:
		/* s points to '; print "'''...'''" */
		حرف_چاپ_کن('"');
		افعل حرف_چاپ_کن('\''); طالما (*++s == '\'');
		حرف_چاپ_کن('"');
	} طالما (*s);
}
#پایان_اگر

#إذا ENABLE_HUSH_EXPORT || ENABLE_HUSH_LOCAL || ENABLE_HUSH_READONLY
ساكن صحيح helper_export_local(حرف **argv, بیعلامت flags)
{
	افعل {
		حرف *name = *argv;
		ثابت حرف *name_انتها = endofname(name);

		إذا (*name_انتها == '\0') {
			بنية variable *var, **vpp;

			vpp = get_ptr_مقصد_local_var(name);
			var = vpp ? *vpp : عدم;

			إذا (flags & SETFLAG_UNEXPORT) {
				/* export -n NAME (without =VALUE) */
				إذا (var) {
					var->flg_export = 0;
					debug_printf_env("%s: unsetenv '%s'\n", __func__, name);
					unsetenv(name);
				} /* else: export -n NOT_EXISTING_VAR: no-op */
				استمر;
			}
			إذا (flags & SETFLAG_EXPORT) {
				/* export NAME (without =VALUE) */
				إذا (var) {
					var->flg_export = 1;
					debug_printf_env("%s: putenv '%s'\n", __func__, var->varstr);
					putenv(var->varstr);
					استمر;
				}
			}
			إذا (flags & SETFLAG_MAKE_RO) {
				/* readonly NAME (without =VALUE) */
				إذا (var) {
					var->flg_read_only = 1;
					استمر;
				}
			}
# إذا ENABLE_HUSH_LOCAL
			/* Is this "local" bltin? */
			إذا (!(flags & (SETFLAG_EXPORT|SETFLAG_UNEXPORT|SETFLAG_MAKE_RO))) {
				بیعلامت lvl = flags >> SETFLAG_VARLVL_لغزش;
				إذا (var && var->var_nest_level == lvl) {
					/* "local x=abc; ...; local x" - ignore second local decl */
					استمر;
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
		} وإلا {
			إذا (*name_انتها != '=') {
				bb_error_پیام("'%s': bad variable name", name);
				/* do not parse following argv[]s: */
				أرجع 1;
			}
			/* (Un)exporting/making local NAME=VALUE */
			name = xstrdup(name);
			/* Testcase: export PS1='\w \$ ' */
			unbackslash(name);
		}
		debug_printf_env("%s: set_local_var('%s')\n", __func__, name);
		إذا (مجموعه_local_var(name, flags))
			أرجع EXIT_FAILURE;
	} طالما (*++argv);
	أرجع EXIT_SUCCESS;
}
#پایان_اگر

#إذا ENABLE_HUSH_EXPORT
ساكن صحيح FAST_FUNC builtin_export(حرف **argv)
{
	بیعلامت opt_unexport;

# إذا ENABLE_HUSH_EXPORT_N
	/* "!": do not abort on errors */
	opt_unexport = getopt32(argv, "!n");
	إذا (opt_unexport == (uint32_t)-1)
		أرجع EXIT_FAILURE;
	argv += optind;
# وإلا
	opt_unexport = 0;
	argv++;
# endif

	إذا (argv[0] == عدم) {
		حرف **e = environ;
		إذا (e) {
			طالما (*e) {
# إذا 0
				puts(*e++);
# وإلا
				/* ash emits: export VAR='VAL'
				 * bash: declare -x VAR="VAL"
				 * we follow ash example */
				ثابت حرف *s = *e++;
				ثابت حرف *p = strchr(s, '=');

				إذا (!p) /* wtf? take next variable */
					استمر;
				/* "export VAR=" */
				چاپ_کن("%s %.*s", "export", (صحيح)(p - s) + 1, s);
				print_escaped(p + 1);
				حرف_چاپ_کن('\n');
# endif
			}
			/*fflush_all(); - done after each builtin anyway */
		}
		أرجع EXIT_SUCCESS;
	}

	أرجع helper_export_local(argv, opt_unexport ? SETFLAG_UNEXPORT : SETFLAG_EXPORT);
}
#پایان_اگر

#إذا ENABLE_HUSH_LOCAL
ساكن صحيح FAST_FUNC builtin_local(حرف **argv)
{
	إذا (G.func_nest_level == 0) {
		bb_error_پیام("%s: not in a function", argv[0]);
		أرجع EXIT_FAILURE; /* bash compat */
	}
//TODO? ash and bash support "local -" special form,
//which saves/restores $- around function call (including async returns, such as ^C)
//(IOW: it makes "set +/-..." effects local)
	argv++;
	/* Since all builtins run in a nested variable level,
	 * need to use level - 1 here. Or else the variable will be removed at once
	 * after builtin returns.
	 */
	أرجع helper_export_local(argv, (G.var_nest_level - 1) << SETFLAG_VARLVL_لغزش);
}
#پایان_اگر

#إذا ENABLE_HUSH_READONLY
ساكن صحيح FAST_FUNC builtin_readonly(حرف **argv)
{
	argv++;
	إذا (*argv == عدم) {
		/* bash: readonly [-p]: list all readonly VARs
		 * (-p has no effect in bash)
		 */
		بنية variable *e;
		لكل (e = G.بالاترین_var; e; e = e->بعدی) {
			إذا (e->flg_read_only) {
				ثابت حرف *s = e->varstr;
				ثابت حرف *p = strchr(s, '=');

				إذا (!p) /* wtf? take next variable */
					استمر;
				/* "readonly VAR=" */
				چاپ_کن("%s %.*s", "readonly", (صحيح)(p - s) + 1, s);
				print_escaped(p + 1);
				حرف_چاپ_کن('\n');
			}
		}
		أرجع EXIT_SUCCESS;
	}
	أرجع helper_export_local(argv, SETFLAG_MAKE_RO);
}
#پایان_اگر

#إذا ENABLE_HUSH_UNSET
/* http://www.opengroup.org/onlinepubs/9699919799/utilities/V3_chap02.html#unset */
ساكن صحيح FAST_FUNC builtin_unset(حرف **argv)
{
	صحيح ret;
	بیعلامت opts;

	/* "!": do not abort on errors */
	/* "+": stop at 1st non-option */
	opts = getopt32(argv, "!+vf");
	إذا (opts == (بیعلامت)-1)
		أرجع EXIT_FAILURE;
	إذا (opts == 3) {
		bb_simple_error_پیام("unset: -v and -f are exclusive");
		أرجع EXIT_FAILURE;
	}
	argv += optind;

	ret = EXIT_SUCCESS;
	طالما (*argv) {
		إذا (!(opts & 2)) { /* not -f */
			إذا (unset_local_var(*argv)) {
				/* unset <nonexistent_var> doesn't fail.
				 * Error is when one tries to unset RO var.
				 * Message was printed by unset_local_var. */
				ret = EXIT_FAILURE;
			}
		}
# إذا ENABLE_HUSH_FUNCTIONS
		وإلا {
			unset_func(*argv);
		}
# endif
		argv++;
	}
	أرجع ret;
}
#پایان_اگر

#إذا ENABLE_HUSH_مجموعه
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
ساكن صحيح FAST_FUNC builtin_مجموعه(حرف **argv)
{
	صحيح n;
	حرف **pp, **g_argv;
	حرف *arg = *++argv;

	إذا (arg == عدم) {
		بنية variable *e;
		لكل (e = G.بالاترین_var; e; e = e->بعدی) {
			ثابت حرف *s = e->varstr;
			ثابت حرف *p = strchr(s, '=');

			إذا (!p) /* wtf? take next variable */
				استمر;
			/* var= */
			چاپ_کن("%.*s", (صحيح)(p - s) + 1, s);
			print_escaped(p + 1);
			حرف_چاپ_کن('\n');
		}
		أرجع EXIT_SUCCESS;
	}

	افعل {
		إذا (strcmp(arg, "--") == 0) {
			++argv;
			اذهب مجموعه_argv;
		}
		إذا (arg[0] != '+' && arg[0] != '-')
			اكسر;
		لكل (n = 1; arg[n]; ++n) {
			إذا (مجموعه_نما((arg[0] == '-'), arg[n], argv[1])) {
				bb_error_پیام("%s: %s: invalid option", "set", arg);
				أرجع EXIT_FAILURE;
			}
			إذا (arg[n] == 'o' && argv[1])
				argv++;
		}
	} طالما ((arg = *++argv) != عدم);
	/* Now argv[0] is 1st argument */

	إذا (arg == عدم)
		أرجع EXIT_SUCCESS;
 مجموعه_argv:

	/* NB: G.global_argv[0] ($0) is never freed/changed */
	g_argv = G.global_argv;
	إذا (G.global_آرگومانها_malloced) {
		pp = g_argv;
		طالما (*++pp)
			حرر(*pp);
		g_argv[1] = عدم;
	} وإلا {
		G.global_آرگومانها_malloced = 1;
		pp = xzalloc(حجم(pp[0]) * 2);
		pp[0] = g_argv[0]; /* retain $0 */
		g_argv = pp;
	}
	/* This realloc's G.global_argv */
	G.global_argv = pp = جمع_کن_رشتهها_مقصد_رشتهها(g_argv, argv, /*dup:*/ 1);

	G.global_argc = 1 + string_array_len(pp + 1);

	أرجع EXIT_SUCCESS;
}
#پایان_اگر

ساكن صحيح FAST_FUNC builtin_لغزش(حرف **argv)
{
	صحيح n = 1;
	argv = skip_dash_dash(argv);
	إذا (argv[0]) {
		n = bb_strtou(argv[0], عدم, 10);
		إذا (errno || n < 0) {
			/* shared string with ash.c */
			bb_error_پیام("Illegal number: %s", argv[0]);
			/*
			 * ash aborts in this case.
			 * bash prints error message and set $? to 1.
			 * Interestingly, for "shift 99999" bash does not
			 * print error message, but does set $? to 1
			 * (and does no shifting at all).
			 */
		}
	}
	إذا (n >= 0 && n < G.global_argc) {
		إذا (G_global_آرگومانها_malloced) {
			صحيح m = 1;
			طالما (m <= n)
				حرر(G.global_argv[m++]);
		}
		G.global_argc -= n;
		memmove(&G.global_argv[1], &G.global_argv[n+1],
				G.global_argc * حجم(G.global_argv[0]));
		أرجع EXIT_SUCCESS;
	}
	أرجع EXIT_FAILURE;
}

#إذا ENABLE_HUSH_GETOPTS
ساكن صحيح FAST_FUNC builtin_getopts(حرف **argv)
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
	صحيح c, n, exitcode, my_opterr;
	بیعلامت شمار;

	optstring = *++argv;
	إذا (!optstring || !(var = *++argv)) {
		bb_simple_error_پیام("usage: getopts OPTSTRING VAR [ARGS]");
		أرجع EXIT_FAILURE;
	}

	إذا (argv[1])
		argv[0] = G.global_argv[0]; /* for error messages in getopt() */
	وإلا
		argv = G.global_argv;
	cbuf[1] = '\0';

	my_opterr = 0;
	إذا (optstring[0] != ':') {
		cp = get_local_var_مقدار("OPTERR");
		/* 0 if "OPTERR=0", 1 otherwise */
		my_opterr = (!cp || NOT_LONE_CHAR(cp, '0'));
	}

	/* getopts stops on first non-option. Add "+" to force that */
	/*if (optstring[0] != '+')*/ {
		حرف *s = alloca(طول_رشته(optstring) + 2);
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
	شمار = 0;
	n = string_array_len(argv);
	افعل {
		optarg = عدم;
		opterr = (شمار < G.getopt_شمار) ? 0 : my_opterr;
		c = getopt(n, argv, optstring);
		إذا (c < 0)
			اكسر;
		شمار++;
	} طالما (شمار <= G.getopt_شمار);

	/* Set OPTIND. Prevent resetting of the magic counter! */
	مجموعه_local_var_from_halves("OPTIND", utoa(optind));
	G.getopt_شمار = شمار; /* "next time, give me N+1'th result" */
	GETOPT_RESET(); /* just in case */

	/* Set OPTARG */
	/* Always set or unset, never left as-is, even on exit/error:
	 * "If no option was found, or if the option that was found
	 * does not have an option-argument, OPTARG shall be unset."
	 */
	cp = optarg;
	إذا (c == '?') {
		/* If ":optstring" and unknown option is seen,
		 * it is stored to OPTARG.
		 */
		إذا (optstring[1] == ':') {
			cbuf[0] = optopt;
			cp = cbuf;
		}
	}
	إذا (cp)
		مجموعه_local_var_from_halves("OPTARG", cp);
	وإلا
		unset_local_var("OPTARG");

	/* Convert -1 to "?" */
	exitcode = EXIT_SUCCESS;
	إذا (c < 0) { /* -1: end of options */
		exitcode = EXIT_FAILURE;
		c = '?';
	}

	/* Set VAR */
	cbuf[0] = c;
	مجموعه_local_var_from_halves(var, cbuf);

	أرجع exitcode;
}
#پایان_اگر

ساكن صحيح FAST_FUNC builtin_منبع(حرف **argv)
{
	حرف *arg_مسیر, *filename;
	HFILE *درون_داد;
	save_arg_t sv;
	حرف *آرگومانها_need_save;
#إذا ENABLE_HUSH_FUNCTIONS
	smallint sv_flg;
#پایان_اگر

	argv = skip_dash_dash(argv);
	filename = argv[0];
	إذا (!filename) {
		/* bash says: "bash: .: filename argument required" */
		أرجع 2; /* bash compat */
	}
	arg_مسیر = عدم;
	إذا (!strchr(filename, '/')) {
		arg_مسیر = پیدا_کن_درون_مسیر(filename);
		إذا (arg_مسیر)
			filename = arg_مسیر;
		وإلا إذا (!ENABLE_HUSH_BASH_منبع_CURDIR) {
			errno = ENOENT;
			bb_simple_perror_پیام(filename);
			أرجع EXIT_FAILURE;
		}
	}
	درون_داد = hfopen(filename);
	حرر(arg_مسیر);
	إذا (!درون_داد) {
		bb_perror_پیام("%s", filename);
		/* POSIX: non-interactive shell should abort here,
		 * not merely fail. So far no one complained :)
		 */
		أرجع EXIT_FAILURE;
	}

#إذا ENABLE_HUSH_FUNCTIONS
	sv_flg = G_flag_return_درون_پیشرفت;
	/* "we are inside sourced file, ok to use return" */
	G_flag_return_درون_پیشرفت = -1;
#پایان_اگر
	آرگومانها_need_save = argv[1]; /* used as a boolean variable */
	إذا (آرگومانها_need_save)
		save_and_replace_G_آرگومانها(&sv, argv);

	/* "false; . ./empty_line; echo Zero:$?" should print 0 */
	G.آخرین_exitcode = 0;
	parse_and_پیاپی_file(درون_داد);
	hfclose(درون_داد);

	إذا (آرگومانها_need_save) /* can't use argv[1] instead: "shift" can mangle it */
		restore_G_آرگومانها(&sv, argv);
#إذا ENABLE_HUSH_FUNCTIONS
	G_flag_return_درون_پیشرفت = sv_flg;
#پایان_اگر

	أرجع G.آخرین_exitcode;
}

#إذا ENABLE_HUSH_TRAP
ساكن صحيح FAST_FUNC builtin_trap(حرف **argv)
{
	صحيح sig;
	حرف *new_cmd;

	إذا (!G_traps)
		G_traps = xzalloc(حجم(G_traps[0]) * NSIG);

	argv++;
	إذا (!*argv) {
		صحيح i;
		/* No args: print all trapped */
		لكل (i = 0; i < NSIG; ++i) {
			إذا (G_traps[i]) {
				چاپ_کن("trap -- ");
				print_escaped(G_traps[i]);
				/* note: bash adds "SIG", but only if invoked
				 * as "bash". If called as "sh", or if set -o posix,
				 * then it prints short signal names.
				 * We are printing short names: */
				چاپ_کن(" %s\n", get_signame(i));
			}
		}
		/*fflush_all(); - done after each builtin anyway */
		أرجع EXIT_SUCCESS;
	}

	new_cmd = عدم;
	/* If first arg is a number: reset all specified signals */
	sig = bb_strtou(*argv, عدم, 10);
	إذا (errno == 0) {
		صحيح ret;
 process_sig_list:
		ret = EXIT_SUCCESS;
		طالما (*argv) {
			sighandler_t handler;

			sig = get_signum(*argv++);
			إذا (sig < 0) {
				ret = EXIT_FAILURE;
				/* Mimic bash message exactly */
				bb_error_پیام("trap: %s: invalid signal specification", argv[-1]);
				استمر;
			}

			حرر(G_traps[sig]);
			G_traps[sig] = xstrdup(new_cmd);

			debug_printf("trap: setting SIG%s (%i) to '%s'\n",
				get_signame(sig), sig, G_traps[sig]);

			/* There is no signal for 0 (EXIT) */
			إذا (sig == 0)
				استمر;

			إذا (new_cmd)
				handler = (new_cmd[0] ? رکورد_pending_signo : SIG_IGN);
			وإلا
				/* We are removing trap handler */
				handler = pick_sighandler(sig);
			install_sighandler(sig, handler);
		}
		أرجع ret;
	}

	إذا (!argv[1]) { /* no second arg */
		bb_simple_error_پیام("trap: invalid arguments");
		أرجع EXIT_FAILURE;
	}

	/* First arg is "-": reset all specified to default */
	/* First arg is "--": skip it, the rest is "handler SIGs..." */
	/* Everything else: set arg as signal handler
	 * (includes "" case, which ignores signal) */
	إذا (argv[0][0] == '-') {
		إذا (argv[0][1] == '\0') { /* "-" */
			/* new_cmd remains NULL: "reset these sigs" */
			اذهب reset_traps;
		}
		إذا (argv[0][1] == '-' && argv[0][2] == '\0') { /* "--" */
			argv++;
		}
		/* else: "-something", no special meaning */
	}
	new_cmd = *argv;
 reset_traps:
	argv++;
	اذهب process_sig_list;
}
#پایان_اگر

#إذا ENABLE_HUSH_کار
ساكن بنية pipe *parse_jobspec(ثابت حرف *str)
{
	بنية pipe *pi;
	بیعلامت jobnum;

	إذا (از_رشته_بخوان(str, "%%%u", &jobnum) != 1) {
		إذا (str[0] != '%'
		 || (str[1] != '%' && str[1] != '+' && str[1] != '\0')
		) {
			bb_error_پیام("bad argument '%s'", str);
			أرجع عدم;
		}
		/* It is "%%", "%+" or "%" - current job */
		jobnum = G.آخرین_jobid;
		إذا (jobnum == 0) {
			bb_simple_error_پیام("no current job");
			أرجع عدم;
		}
	}
	لكل (pi = G.کار_list; pi; pi = pi->بعدی) {
		إذا (pi->jobid == jobnum) {
			أرجع pi;
		}
	}
	bb_error_پیام("%u: no such job", jobnum);
	أرجع عدم;
}

ساكن صحيح FAST_FUNC builtin_jobs(حرف **argv استفادهنشده_PARAM)
{
	بنية pipe *کار;
	ثابت حرف *وضعیت_string;

	checkjobs(عدم, 0 /*(no pid to wait for)*/);
	لكل (کار = G.کار_list; کار; کار = کار->بعدی) {
		إذا (کار->alive_cmds == کار->stopped_cmds)
			وضعیت_string = "Stopped";
		وإلا
			وضعیت_string = "Running";

		چاپ_کن(کار_وضعیت_FORMAT, کار->jobid, وضعیت_string, کار->cmdtext);
	}

	clean_به_بالا_آخرین_dead_کار();

	أرجع EXIT_SUCCESS;
}

/* built-in 'fg' and 'bg' handler */
ساكن صحيح FAST_FUNC builtin_fg_bg(حرف **argv)
{
	صحيح i;
	بنية pipe *pi;

	إذا (!G_interactive_fd)
		أرجع EXIT_FAILURE;

	/* If they gave us no args, assume they want the last backgrounded task */
	إذا (!argv[1]) {
		لكل (pi = G.کار_list; pi; pi = pi->بعدی) {
			إذا (pi->jobid == G.آخرین_jobid) {
				اذهب یافت;
			}
		}
		bb_error_پیام("%s: no current job", argv[0]);
		أرجع EXIT_FAILURE;
	}

	pi = parse_jobspec(argv[1]);
	إذا (!pi)
		أرجع EXIT_FAILURE;
 یافت:
	/* TODO: bash prints a string representation
	 * of job being foregrounded (like "sleep 1 | cat") */
	إذا (argv[0][0] == 'f' && G_saved_tty_pgrp) {
		/* Put the job into the foreground. */
		tcsetpgrp(G_interactive_fd, pi->pgrp);
	}

	/* Restart the processes in the job */
	debug_printf_jobs("reviving %d procs, pgrp %d\n", pi->رقم_cmds, pi->pgrp);
	لكل (i = 0; i < pi->رقم_cmds; i++) {
		debug_printf_jobs("reviving pid %d\n", pi->cmds[i].pid);
	}
	pi->stopped_cmds = 0;

	i = kill(- pi->pgrp, SIGCONT);
	إذا (i < 0) {
		إذا (errno == ESRCH) {
			delete_پایانیافته_کار(pi);
			أرجع EXIT_SUCCESS;
		}
		bb_simple_perror_پیام("kill (SIGCONT)");
	}

	إذا (argv[0][0] == 'f') {
		remove_کار_from_جدول(pi); /* FG job shouldn't be in job table */
		أرجع checkjobs_and_fg_shell(pi);
	}
	أرجع EXIT_SUCCESS;
}
#پایان_اگر

#إذا ENABLE_HUSH_KILL
ساكن صحيح FAST_FUNC builtin_kill(حرف **argv)
{
	صحيح ret = 0;

# إذا ENABLE_HUSH_کار
	إذا (argv[1] && strcmp(argv[1], "-l") != 0) {
		صحيح i = 1;

		افعل {
			بنية pipe *pi;
			حرف *dst;
			صحيح j, n;

			إذا (argv[i][0] != '%')
				استمر;
			/*
			 * "kill %N" - job kill
			 * Converting to pgrp / pid kill
			 */
			pi = parse_jobspec(argv[i]);
			إذا (!pi) {
				/* Eat bad jobspec */
				j = i;
				افعل {
					j++;
					argv[j - 1] = argv[j];
				} طالما (argv[j]);
				ret = 1;
				i--;
				استمر;
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
			n = G_interactive_fd ? 1 : pi->رقم_cmds;
			dst = alloca(n * حجم(صحيح)*4);
			argv[i] = dst;
			إذا (G_interactive_fd)
				dst += sprintf(dst, " -%u", (صحيح)pi->pgrp);
			وإلا لكل (j = 0; j < n; j++) {
				بنية command *cmd = &pi->cmds[j];
				/* Skip exited members of the job */
				إذا (cmd->pid == 0)
					استمر;
				/*
				 * kill_main has matching code to expect
				 * leading space. Needed to not confuse
				 * negative pids with "kill -SIGNAL_NO" syntax
				 */
				dst += sprintf(dst, " %u", (صحيح)cmd->pid);
			}
			*dst = '\0';
		} طالما (argv[++i]);
	}
# endif

	إذا (argv[1] || ret == 0) {
		ret = پیاپی_applet_main(argv, kill_main);
	}
	/* else: ret = 1, "kill %bad_jobspec" case */
	أرجع ret;
}
#پایان_اگر

#إذا ENABLE_HUSH_WAIT
/* http://www.opengroup.org/onlinepubs/9699919799/utilities/wait.html */
# إذا !ENABLE_HUSH_کار
#  define wait_for_فرزند_or_signal(pipe,pid) wait_for_فرزند_or_signal(pid)
# endif
ساكن صحيح wait_for_فرزند_or_signal(بنية pipe *waitfor_pipe, pid_t waitfor_pid)
{
	صحيح ret = 0;
	لكل (;;) {
		صحيح sig;
		sigset_t oldset;

		إذا (!sigisemptyset(&G.pending_مجموعه))
			اذهب check_sig;

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

		إذا (!sigisemptyset(&G.pending_مجموعه)) {
			/* Crap! we raced with some signal! */
			اذهب restore;
		}

		/*errno = 0; - checkjobs does this */
/* Can't pass waitfor_pipe into checkjobs(): it won't be interruptible */
		ret = checkjobs(عدم, waitfor_pid); /* waitpid(WNOHANG) inside */
		debug_printf_exec("checkjobs:%d\n", ret);
# إذا ENABLE_HUSH_کار
		إذا (waitfor_pipe) {
			صحيح rcode = کار_exited_or_stopped(waitfor_pipe);
			debug_printf_exec("job_exited_or_stopped:%d\n", rcode);
			إذا (rcode >= 0) {
				ret = rcode;
				sigprocmask(SIG_SETMASK, &oldset, عدم);
				اكسر;
			}
		}
# endif
		/* if ECHILD, there are no children (ret is -1 or 0) */
		/* if ret == 0, no children changed state */
		/* if ret != 0, it's exitcode+1 of exited waitfor_pid child */
		إذا (errno == ECHILD || ret) {
			ret--;
			إذا (ret < 0) /* if ECHILD, may need to fix "ret" */
				ret = 0;
# إذا ENABLE_HUSH_BASH_COMPAT
			إذا (waitfor_pid == -1 && errno == ECHILD) {
				/* exitcode of "wait -n" with no children is 127, not 0 */
				ret = 127;
			}
# endif
			sigprocmask(SIG_SETMASK, &oldset, عدم);
			اكسر;
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
		sigprocmask(SIG_SETMASK, &oldset, عدم);
 check_sig:
		/* So, did we get a signal? */
		sig = check_and_پیاپی_traps();
		إذا (sig /*&& sig != SIGCHLD - always true */) {
			/* Do this for any (non-ignored) signal, not only for ^C */
			ret = 128 | sig;
			اكسر;
		}
		/* SIGCHLD, or no signal, or ignored one, such as SIGQUIT. Repeat */
	}
	أرجع ret;
}

ساكن صحيح FAST_FUNC builtin_wait(حرف **argv)
{
	صحيح ret;
	صحيح وضعیت;

	argv = skip_dash_dash(argv);
# إذا ENABLE_HUSH_BASH_COMPAT
	إذا (argv[0] && strcmp(argv[0], "-n") == 0) {
		/* wait -n */
		/* (bash accepts "wait -n PID" too and ignores PID) */
		G.dead_کار_exitcode = -1;
		أرجع wait_for_فرزند_or_signal(عدم, -1 /*no job, wait for one job*/);
	}
# endif
	إذا (argv[0] == عدم) {
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
		أرجع wait_for_فرزند_or_signal(عدم, 0 /*no job and no pid to wait for*/);
	}

	افعل {
		pid_t pid = bb_strtou(*argv, عدم, 10);
		إذا (errno || pid <= 0) {
# إذا ENABLE_HUSH_کار
			إذا (argv[0][0] == '%') {
				بنية pipe *wait_pipe;
				ret = 127; /* bash compat for bad jobspecs */
				wait_pipe = parse_jobspec(*argv);
				إذا (wait_pipe) {
					ret = کار_exited_or_stopped(wait_pipe);
					إذا (ret < 0) {
						ret = wait_for_فرزند_or_signal(wait_pipe, 0);
					} وإلا {
						/* waiting on "last dead job" removes it */
						clean_به_بالا_آخرین_dead_کار();
					}
				}
				/* else: parse_jobspec() already emitted error msg */
				استمر;
			}
# endif
			/* mimic bash message */
			bb_error_پیام("wait: '%s': not a pid or valid job spec", *argv);
			ret = EXIT_FAILURE;
			استمر; /* bash checks all argv[] */
		}

		/* Do we have such child? */
		ret = waitpid(pid, &وضعیت, WNOHANG);
		إذا (ret < 0) {
			/* No */
			ret = 127;
			إذا (errno == ECHILD) {
				إذا (pid == G.آخرین_bg_pid) {
					/* "wait $!" but last bg task has already exited. Try:
					 * (sleep 1; exit 3) & sleep 2; echo $?; wait $!; echo $?
					 * In bash it prints exitcode 0, then 3.
					 * In dash, it is 127.
					 */
					ret = G.آخرین_bg_pid_exitcode;
				} وإلا {
					/* Example: "wait 1". mimic bash message */
					bb_error_پیام("wait: pid %u is not a child of this shell", (بیعلامت)pid);
				}
			} وإلا {
				/* ??? */
				bb_perror_پیام("wait %s", *argv);
			}
			استمر; /* bash checks all argv[] */
		}
		إذا (ret == 0) {
			/* Yes, and it still runs */
			ret = wait_for_فرزند_or_signal(عدم, pid);
		} وإلا {
			/* Yes, and it just exited */
			process_wait_نتیجه(عدم, pid, وضعیت);
			ret = WEXITSTATUS(وضعیت);
			إذا (WIFSIGNALED(وضعیت))
				ret = 128 | WTERMSIG(وضعیت);
		}
	} طالما (*++argv);

	أرجع ret;
}
#پایان_اگر

#إذا ENABLE_HUSH_LOOPS || ENABLE_HUSH_FUNCTIONS
ساكن بیعلامت parse_numeric_argv1(حرف **argv, بیعلامت def, بیعلامت def_min)
{
	إذا (argv[1]) {
		def = bb_strtou(argv[1], عدم, 10);
		إذا (errno || def < def_min || argv[2]) {
			bb_error_پیام("%s: bad arguments", argv[0]);
			def = UINT_MAX;
		}
	}
	أرجع def;
}
#پایان_اگر

#إذا ENABLE_HUSH_LOOPS
ساكن صحيح FAST_FUNC builtin_break(حرف **argv)
{
	بیعلامت عمق;
	إذا (G.عمق_از_loop == 0) {
		bb_error_پیام("%s: only meaningful in a loop", argv[0]);
		/* if we came from builtin_continue(), need to undo "= 1" */
		G.flag_break_continue = 0;
		أرجع EXIT_SUCCESS; /* bash compat */
	}
	G.flag_break_continue++; /* BC_BREAK = 1, or BC_CONTINUE = 2 */

	G.عمق_break_continue = عمق = parse_numeric_argv1(argv, 1, 1);
	إذا (عمق == UINT_MAX)
		G.flag_break_continue = BC_BREAK;
	إذا (G.عمق_از_loop < عمق)
		G.عمق_break_continue = G.عمق_از_loop;

	أرجع EXIT_SUCCESS;
}

ساكن صحيح FAST_FUNC builtin_continue(حرف **argv)
{
	G.flag_break_continue = 1; /* BC_CONTINUE = 2 = 1+1 */
	أرجع builtin_break(argv);
}
#پایان_اگر

#إذا ENABLE_HUSH_FUNCTIONS
ساكن صحيح FAST_FUNC builtin_return(حرف **argv)
{
	صحيح rc;

	إذا (G_flag_return_درون_پیشرفت != -1) {
		bb_error_پیام("%s: not in a function or sourced script", argv[0]);
		أرجع EXIT_FAILURE; /* bash compat */
	}

	G_flag_return_درون_پیشرفت = 1;

	/* bash:
	 * out of range: wraps around at 256, does not error out
	 * non-numeric param:
	 * f() { false; return qwe; }; f; echo $?
	 * bash: return: qwe: numeric argument required  <== we do this
	 * 255  <== we also do this
	 */
	rc = parse_numeric_argv1(argv, G.آخرین_exitcode, 0);
# إذا ENABLE_HUSH_TRAP
	إذا (argv[1]) { /* "return ARG" inside a running trap sets $? */
		debug_printf_exec("G.return_exitcode=%d\n", rc);
		G.return_exitcode = rc;
	}
# endif
	أرجع rc;
}
#پایان_اگر

#إذا ENABLE_HUSH_TIMES
ساكن صحيح FAST_FUNC builtin_times(حرف **argv استفادهنشده_PARAM)
{
	ساكن ثابت uint8_t times_tbl[] ALIGN1 = {
		' ',  offsetof(بنية tms, tms_utime),
		'\n', offsetof(بنية tms, tms_stime),
		' ',  offsetof(بنية tms, tms_cutime),
		'\n', offsetof(بنية tms, tms_cstime),
		0
	};
	ثابت uint8_t *p;
	بیعلامت clk_tck;
	بنية tms buf;

	clk_tck = bb_clk_tck();

	times(&buf);
	p = times_tbl;
	افعل {
		بیعلامت sec, frac;
		بیعلامت دراز t;
		t = *(clock_t *)(((حرف *) &buf) + p[1]);
		sec = t / clk_tck;
		frac = t % clk_tck;
		چاپ_کن("%um%u.%03us%c",
			sec / 60, sec % 60,
			(frac * 1000) / clk_tck,
			p[0]);
		p += 2;
	} طالما (*p);

	أرجع EXIT_SUCCESS;
}
#پایان_اگر

#إذا ENABLE_HUSH_MEMLEAK
ساكن صحيح FAST_FUNC builtin_memleak(حرف **argv استفادهنشده_PARAM)
{
	فراغ *p;
	بیعلامت دراز l;

# ifdef M_TRIM_THRESHOLD
	/* Optional. Reduces probability of false positives */
	malloc_trim(0);
# endif
	/* Crude attempt to find where "free memory" starts,
	 * sans fragmentation. */
	p = حافظه_بده(240);
	l = (بیعلامت دراز)p;
	حرر(p);
	p = حافظه_بده(3400);
	إذا (l < (بیعلامت دراز)p) l = (بیعلامت دراز)p;
	حرر(p);

# إذا 0  /* debug */
	{
		بنية mallinfo mi = mallinfo();
		چاپ_کن("top alloc:0x%lx malloced:%d+%d=%d\n", l,
			mi.arena, mi.hblkhd, mi.arena + mi.hblkhd);
	}
# endif

	إذا (!G.memleak_مقدار)
		G.memleak_مقدار = l;

	l -= G.memleak_مقدار;
	إذا ((دراز)l < 0)
		l = 0;
	l /= 1024;
	إذا (l > 127)
		l = 127;

	/* Exitcode is "how many kilobytes we leaked since 1st call" */
	أرجع l;
}
#پایان_اگر
#پایان_اگر /* !__U_BOOT__ */
