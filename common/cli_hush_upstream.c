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

#నిర్వచితం_కాకపోతే __U_BOOT__
#ఒకవేళ !(defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) \
	|| defined(__APPLE__) \
    )
# include <malloc.h>   /* for malloc_trim */
#అయితే_ముగింపు
#అన్తర్భావ <glob.h>
/* #include <dmalloc.h> */
#ఒకవేళ ENABLE_HUSH_CASE
# include <fnmatch.h>
#అయితే_ముగింపు
#అన్తర్భావ <sys/times.h>
#అన్తర్భావ <sys/utsname.h> /* for setting $HOSTNAME */

#అన్తర్భావ "busybox.h"  /* for APPLET_IS_NOFORK/NOEXEC */
#అన్తర్భావ "unicode.h"
#అన్తర్భావ "shell_common.h"
#అన్తర్భావ "math.h"
#అన్తర్భావ "match.h"
#ఒకవేళ ENABLE_HUSH_RANDOM_SUPPORT
# include "random.h"
#లేకపోతే
# define CLEAR_RANDOM_T(rnd) ((శూన్య)0)
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే O_CLOEXEC
# define O_CLOEXEC 0
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే F_DUPFD_CLOEXEC
# define F_DUPFD_CLOEXEC F_DUPFD
#అయితే_ముగింపు

#ఒకవేళ ENABLE_FEATURE_SH_EMBEDDED_SCRIPTS && !ENABLE_SHELL_ASH
# include "embedded_scripts.h"
#లేకపోతే
# define అంకె_SCRIPTS 0
#అయితే_ముగింపు
#అయితే_ముగింపు /* !__U_BOOT__ */

/* So far, all bash compat is controlled by one config option */
/* Separate defines document which part of code implements what */
#నిర్వచించు BASH_నమూనా_SUBST ENABLE_HUSH_BASH_COMPAT
#నిర్వచించు BASH_SUBSTR        ENABLE_HUSH_BASH_COMPAT
#నిర్వచించు BASH_ఆధారం        ENABLE_HUSH_BASH_COMPAT
#నిర్వచించు BASH_DOLLAR_SQUOTE ENABLE_HUSH_BASH_COMPAT
#నిర్వచించు BASH_HOSTNAME_VAR  ENABLE_HUSH_BASH_COMPAT
#నిర్వచించు BASH_EPOCH_VARS    ENABLE_HUSH_BASH_COMPAT
#నిర్వచించు BASH_TEST2         (ENABLE_HUSH_BASH_COMPAT && ENABLE_HUSH_TEST)
#నిర్వచించు BASH_READ_D        ENABLE_HUSH_BASH_COMPAT

/* Build knobs */
#నిర్వచించు LEAK_HUNTING 0
#నిర్వచించు BUILD_AS_NOMMU 0
/* Enable/disable sanity checks. Ok to enable in production,
 * only adds a bit of bloat. Set to >1 to get non-production level verbosity.
 * Keeping 1 for now even in released versions.
 */
#నిర్వచించు HUSH_DEBUG 1
/* Slightly bigger (+200 bytes), but faster hush.
 * So far it only enables a trick with counting SIGCHLDs and forks,
 * which allows us to do fewer waitpid's.
 * (we can detect a case where neither forks were done nor SIGCHLDs happened
 * and therefore waitpid will return the same result as last time)
 */
#నిర్వచించు ENABLE_HUSH_FAST 0
/* TODO: implement simplified code for users which do not need ${var%...} ops
 * So far ${var%...} ops are always enabled:
 */
#నిర్వచించు ENABLE_HUSH_DOLLAR_OPS 1

#ఒకవేళ BUILD_AS_NOMMU
# undef BB_MMU
# undef USE_FOR_NOMMU
# undef USE_FOR_MMU
# define BB_MMU 0
# define USE_FOR_NOMMU(...) __VA_ARGS__
# define USE_FOR_MMU(...)
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే __U_BOOT__
#అన్తర్భావ "NUM_APPLETS.h"
#ఒకవేళ అంకె_APPLETS == 1
/* STANDALONE does not make sense, and won't compile */
# undef ENABLE_FEATURE_SH_STANDALONE
# undef IF_FEATURE_SH_STANDALONE
# undef IF_NOT_FEATURE_SH_STANDALONE
# define ENABLE_FEATURE_SH_STANDALONE 0
# define IF_FEATURE_SH_STANDALONE(...)
# define IF_NOT_FEATURE_SH_STANDALONE(...) __VA_ARGS__
#అయితే_ముగింపు
#అయితే_ముగింపు /* __U_BOOT__ */

#ఒకవేళ !ENABLE_HUSH_INTERACTIVE
# undef ENABLE_FEATURE_EDITING
# define ENABLE_FEATURE_EDITING 0
# undef ENABLE_FEATURE_EDITING_FANCY_PROMPT
# define ENABLE_FEATURE_EDITING_FANCY_PROMPT 0
# undef ENABLE_FEATURE_EDITING_SAVE_ON_EXIT
# define ENABLE_FEATURE_EDITING_SAVE_ON_EXIT 0
#అయితే_ముగింపు

/* Do we support ANY keywords? */
#ఒకవేళ ENABLE_HUSH_IF || ENABLE_HUSH_LOOPS || ENABLE_HUSH_CASE
# define HAS_KEYWORDS 1
# define IF_HAS_KEYWORDS(...) __VA_ARGS__
# define IF_HAS_NO_KEYWORDS(...)
#లేకపోతే
# define HAS_KEYWORDS 0
# define IF_HAS_KEYWORDS(...)
# define IF_HAS_NO_KEYWORDS(...) __VA_ARGS__
#అయితే_ముగింపు

/* If you comment out one of these below, it will be #defined later
 * to perform debug printfs to stderr: */
#నిర్వచించు debug_printf(...)         చేయి {} అయితే (0)
/* Finer-grained debug switches */
#నిర్వచించు debug_printf_parse(...)   చేయి {} అయితే (0)
#నిర్వచించు debug_printf_heredoc(...) చేయి {} అయితే (0)
#నిర్వచించు debug_print_వృక్షం(a, b)    చేయి {} అయితే (0)
#నిర్వచించు debug_printf_exec(...)    చేయి {} అయితే (0)
#నిర్వచించు debug_printf_env(...)     చేయి {} అయితే (0)
#నిర్వచించు debug_printf_jobs(...)    చేయి {} అయితే (0)
#నిర్వచించు debug_printf_expand(...)  చేయి {} అయితే (0)
#నిర్వచించు debug_printf_varexp(...)  చేయి {} అయితే (0)
#నిర్వచించు debug_printf_glob(...)    చేయి {} అయితే (0)
#నిర్వచించు debug_printf_redir(...)   చేయి {} అయితే (0)
#నిర్వచించు debug_printf_list(...)    చేయి {} అయితే (0)
#నిర్వచించు debug_printf_subst(...)   చేయి {} అయితే (0)
#నిర్వచించు debug_printf_prompt(...)  చేయి {} అయితే (0)
#నిర్వచించు debug_printf_clean(...)   చేయి {} అయితే (0)

#నిర్వచించు ERR_PTR ((శూన్య*)(దీర్ఘ)1)

#నిర్వచించు పని_స్థితి_FORMAT    "[%u] %-22s %.40s\n"

#నిర్వచించు _SPECIAL_VARS_STR     "_*@$!?#-"
#నిర్వచితం_కాకపోతే __U_BOOT__
#నిర్వచించు SPECIAL_VARS_STR     ("_*@$!?#-" + 1)
#నిర్వచించు NUMERIC_SPECVARS_STR ("_*@$!?#-" + 3)
#లేకపోతే /* __U_BOOT__ */
#నిర్వచించు SPECIAL_VARS_STR     "*@$!?#-"
#నిర్వచించు NUMERIC_SPECVARS_STR "$!?#-"
#అయితే_ముగింపు /* __U_BOOT__ */
#ఒకవేళ BASH_నమూనా_SUBST
/* Support / and // replace ops */
/* Note that // is stored as \ in "encoded" string representation */
# define VAR_ENCODED_SUBST_OPS      "\\/%#:-=+?"
# define VAR_SUBST_OPS             ("\\/%#:-=+?" + 1)
# define MINUS_PLUS_EQUAL_QUESTION ("\\/%#:-=+?" + 5)
#లేకపోతే
# define VAR_ENCODED_SUBST_OPS      "%#:-=+?"
# define VAR_SUBST_OPS              "%#:-=+?"
# define MINUS_PLUS_EQUAL_QUESTION ("%#:-=+?" + 3)
#అయితే_ముగింపు

#నిర్వచించు SPECIAL_VAR_చిహ్నం_STR "\3"
#నిర్వచించు SPECIAL_VAR_చిహ్నం       3
/* The "variable" with name "\1" emits string "\3". Testcase: "echo ^C" */
#నిర్వచించు SPECIAL_VAR_QUOTED_SVS   1

నిర్మాణం variable;

స్థిర స్థిరరాశి అక్షరం hush_సంస్కరణ_str[] ALIGN1 = "HUSH_VERSION="BB_VER;

/* This supports saving pointers malloced in vfork child,
 * to be freed in the parent.
 */
#ఒకవేళ !BB_MMU
రకనిర్వచనం నిర్మాణం nommu_save_t {
	నిర్మాణం variable *old_vars;
	అక్షరం **argv;
	అక్షరం **argv_from_re_execing;
} nommu_save_t;
#అయితే_ముగింపు

గణన {
	RES_NONE  = 0,
#ఒకవేళ ENABLE_HUSH_IF
	RES_IF    ,
	RES_THEN  ,
	RES_ELIF  ,
	RES_ELSE  ,
	RES_FI    ,
#అయితే_ముగింపు
#ఒకవేళ ENABLE_HUSH_LOOPS
	RES_FOR   ,
	RES_WHILE ,
	RES_UNTIL ,
	RES_DO    ,
	RES_పూర్తి  ,
#అయితే_ముగింపు
#ఒకవేళ ENABLE_HUSH_LOOPS || ENABLE_HUSH_CASE
	RES_లోపల    ,
#అయితే_ముగింపు
#ఒకవేళ ENABLE_HUSH_CASE
	RES_CASE  ,
	/* three pseudo-keywords support contrived "case" syntax: */
	RES_CASE_లోపల,   /* "case ... IN", turns into RES_MATCH when IN is observed */
	RES_సరిపోలిక ,    /* "word)" */
	RES_CASE_BODY, /* "this command is inside CASE" */
	RES_ESAC  ,
#అయితే_ముగింపు
	RES_XXXX  ,
	RES_SNTX
};

రకనిర్వచనం నిర్మాణం o_string {
	అక్షరం *దత్తాంశం;
	పూర్ణాంకం నిడివి; /* position where data is appended */
	పూర్ణాంకం maxlen;
	పూర్ణాంకం o_expflags;
	/* At least some part of the string was inside '' or "",
	 * possibly empty one: word"", wo''rd etc. */
	smallint has_quoted_part;
	smallint has_ఖాళీలు_స్థానం;
	smallint ended_లోపల_ifs;
} o_string;
గణన {
	EXP_FLAG_SINGLEWORD     = 0x80, /* must be 0x80 */
	EXP_FLAG_GLOB           = 0x2,
	/* Protect newly added chars against globbing
	 * by prepending \ to *, ?, [, \ */
	EXP_FLAG_ESC_GLOB_CHARS = 0x1,
};
/* Used for initialization: o_string foo = NULL_O_STRING; */
#నిర్వచించు NULL_O_STRING { ఖాళీ }

#నిర్వచితం_కాకపోతే debug_printf_parse
స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి assignment_flag[] ALIGN_PTR = {
	"MAYBE_ASSIGNMENT",
	"DEFINITELY_ASSIGNMENT",
	"NOT_ASSIGNMENT",
	"WORD_IS_KEYWORD",
};
#అయితే_ముగింపు

/* We almost can use standard FILE api, but we need an ability to move
 * its fd when redirects coincide with it. No api exists for that
 * (RFE for it at https://sourceware.org/bugzilla/show_bug.cgi?id=21902).
 * HFILE is our internal alternative. Only supports reading.
 * Since we now can, we incorporate linked list of all opened HFILEs
 * into the struct (used to be a separate mini-list).
 */
రకనిర్వచనం నిర్మాణం HFILE {
	అక్షరం *cur;
	అక్షరం *చివర_2;
	నిర్మాణం HFILE *తదుపరి_hfile;
	పూర్ణాంకం fd;
	అక్షరం buf[1024];
} HFILE;

రకనిర్వచనం నిర్మాణం లోపల_str {
	స్థిరరాశి అక్షరం *p;
	పూర్ణాంకం peek_buf[2];
	పూర్ణాంకం చివరి_char;
	HFILE *file;
} లోపల_str;

#నిర్వచితం_కాకపోతే __U_BOOT__
/* The descrip member of this structure is only used to make
 * debugging output pretty */
స్థిర స్థిరరాశి నిర్మాణం {
	int32_t బహుళకం;
	గుర్తుగల అక్షరం default_fd;
	అక్షరం descrip[3];
} redir_పట్టిక[] ALIGN4 = {
	{ O_RDONLY,                  0, "<"  },
	{ O_CREAT|O_TRUNC|O_WRONLY,  1, ">"  },
	{ O_CREAT|O_APPEND|O_WRONLY, 1, ">>" },
	{ O_CREAT|O_RDWR,            1, "<>" },
	{ O_RDONLY,                  0, "<<" },
/* Should not be needed. Bogus default_fd helps in debugging */
/*	{ O_RDONLY,                 77, "<<" }, */
};

నిర్మాణం redir_struct {
	నిర్మాణం redir_struct *తదుపరి;
	అక్షరం *rd_filename;          /* filename */
	పూర్ణాంకం rd_fd;                  /* fd to redirect */
	/* fd to redirect to, or -3 if rd_fd is to be closed (n>&-) */
	పూర్ణాంకం rd_dup;
	smallint rd_type;           /* (enum redir_type) */
	/* note: for heredocs, rd_filename contains heredoc delimiter,
	 * and subsequently heredoc itself; and rd_dup is a bitmask:
	 * bit 0: do we need to trim leading tabs?
	 * bit 1: is heredoc quoted (<<'delim' syntax) ?
	 */
};
రకనిర్వచనం గణన redir_type {
	REDIRECT_ఆదానం     = 0,
	REDIRECT_OVERWRITE = 1,
	REDIRECT_APPEND    = 2,
	REDIRECT_IO        = 3,
	REDIRECT_HEREDOC   = 4,
	REDIRECT_HEREDOC2  = 5, /* REDIRECT_HEREDOC after heredoc is loaded */

	REDIRFD_CLOSE      = -3,
	REDIRFD_SYNTAX_ERR = -2,
	REDIRFD_గమ్యం_FILE    = -1,
	/* otherwise, rd_fd is redirected to rd_dup */

	HEREDOC_SKIPTABS = 1,
	HEREDOC_QUOTED   = 2,
} redir_type;

#అయితే_ముగింపు /* !__U_BOOT__ */

నిర్మాణం command {
#నిర్వచితం_కాకపోతే __U_BOOT__
	pid_t pid;                  /* 0 if exited */
#అయితే_ముగింపు /* !__U_BOOT__ */
	గుర్తులేని assignment_cnt;    /* how many argv[i] are assignments? */
#ఒకవేళ ENABLE_HUSH_LINENO_VAR
	గుర్తులేని lineno;
#అయితే_ముగింపు
	smallint cmd_type;          /* CMD_xxx */
#నిర్వచించు CMD_సాధారణ   0
#నిర్వచించు CMD_SUBSHELL 1
#ఒకవేళ BASH_TEST2
/* used for "[[ EXPR ]]" */
# define CMD_TEST2_SINGLEWORD_NOGLOB 2
#అయితే_ముగింపు
#ఒకవేళ BASH_TEST2 || ENABLE_HUSH_LOCAL || ENABLE_HUSH_EXPORT || ENABLE_HUSH_READONLY
/* used to prevent word splitting and globbing in "export v=t*" */
# define CMD_SINGLEWORD_NOGLOB 3
#అయితే_ముగింపు
#ఒకవేళ ENABLE_HUSH_FUNCTIONS
# define CMD_FUNCDEF 4
#అయితే_ముగింపు

	smalluint cmd_exitcode;
	/* if non-NULL, this "command" is { list }, ( list ), or a compound statement */
	నిర్మాణం pipe *సమూహం;
#ఒకవేళ !BB_MMU
	అక్షరం *సమూహం_as_string;
#అయితే_ముగింపు
#ఒకవేళ ENABLE_HUSH_FUNCTIONS
	నిర్మాణం function *బిడ్డ_func;
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
#అయితే_ముగింపు
#నిర్వచితమైతే __U_BOOT__
	పూర్ణాంకం argc; /* number of program arguments */
#అయితే_ముగింపు
	అక్షరం **argv;                /* command name and arguments */
/* argv vector may contain variable references (^Cvar^C, ^C0^C etc)
 * and on execution these are substituted with their values.
 * Substitution can make _several_ words out of one argv[n]!
 * Example: argv[0]=='.^C*^C.' here: echo .$*.
 * References of the form ^C`cmd arg^C are `cmd arg` substitutions.
 */
#నిర్వచితం_కాకపోతే __U_BOOT__
	నిర్మాణం redir_struct *redirects; /* I/O redirections */
#అయితే_ముగింపు /* !__U_BOOT__ */
};
/* Is there anything in this command at all? */
#నిర్వచితం_కాకపోతే __U_BOOT__
#నిర్వచించు IS_NULL_CMD(cmd) \
	(!(cmd)->సమూహం && !(cmd)->argv && !(cmd)->redirects)

#లేకపోతే /* __U_BOOT__ */
#నిర్వచించు IS_NULL_CMD(cmd) \
	(!(cmd)->సమూహం && !(cmd)->argv)
#అయితే_ముగింపు /* __U_BOOT__ */
నిర్మాణం pipe {
	నిర్మాణం pipe *తదుపరి;
	పూర్ణాంకం అంకె_cmds;               /* total number of commands in pipe */
#నిర్వచితం_కాకపోతే __U_BOOT__
	పూర్ణాంకం alive_cmds;             /* number of commands running (not exited) */
	పూర్ణాంకం stopped_cmds;           /* number of commands alive, but stopped */
#ఒకవేళ ENABLE_HUSH_పని
	గుర్తులేని jobid;             /* job number */
	pid_t pgrp;                 /* process group ID for the job */
	అక్షరం *cmdtext;              /* name of job */
#అయితే_ముగింపు
#అయితే_ముగింపు /* !__U_BOOT__ */
	నిర్మాణం command *cmds;       /* array of commands in pipe */
	smallint followup;          /* PIPE_BG, PIPE_SEQ, PIPE_OR, PIPE_AND */
	IF_HAS_KEYWORDS(smallint pi_inverted;) /* "! cmd | cmd" */
	IF_HAS_KEYWORDS(smallint res_మాట;) /* needed for if, for, while, until... */
};
రకనిర్వచనం గణన pipe_style {
	PIPE_SEQ = 0,
	PIPE_AND = 1,
	PIPE_OR  = 2,
	PIPE_BG  = 3,
} pipe_style;
/* Is there anything in this pipe at all? */
#నిర్వచించు IS_NULL_PIPE(pi) \
	((pi)->అంకె_cmds == 0 IF_HAS_KEYWORDS( && (pi)->res_మాట == RES_NONE))

/* This holds pointers to the various results of parsing */
నిర్మాణం parse_context {
	/* linked list of pipes */
	నిర్మాణం pipe *list_తల;
	/* last pipe (being constructed right now) */
	నిర్మాణం pipe *pipe;
	/* last command in pipe (being constructed right now) */
	నిర్మాణం command *command;
#నిర్వచితం_కాకపోతే __U_BOOT__
	/* last redirect in command->redirects list */
	నిర్మాణం redir_struct *pending_redirect;
#అయితే_ముగింపు /* !__U_BOOT__ */
	o_string మాట;
#ఒకవేళ !BB_MMU
	o_string as_string;
#అయితే_ముగింపు
	smallint is_assignment; /* 0:maybe, 1:yes, 2:no, 3:keyword */
#ఒకవేళ HAS_KEYWORDS
	smallint ctx_res_w;
	smallint ctx_inverted; /* "! cmd | cmd" */
#ఒకవేళ ENABLE_HUSH_CASE
	smallint ctx_dsemicolon; /* ";;" seen */
#అయితే_ముగింపు
	/* bitmask of FLAG_xxx, for figuring out valid reserved words */
	పూర్ణాంకం old_flag;
	/* group we are enclosed in:
	 * example: "if pipe1; pipe2; then pipe3; fi"
	 * when we see "if" or "then", we malloc and copy current context,
	 * and make ->stack point to it. then we parse pipeN.
	 * when closing "then" / fi" / whatever is found,
	 * we move list_head into ->stack->command->group,
	 * copy ->stack into current context, and delete ->stack.
	 * (parsing of { list } and ( list ) doesn't use this method)
	 */
	నిర్మాణం parse_context *దొంతర;
#అయితే_ముగింపు
};
గణన {
	MAYBE_ASSIGNMENT      = 0,
	DEFINITELY_ASSIGNMENT = 1,
	NOT_ASSIGNMENT        = 2,
	/* Not an assignment, but next word may be: "if v=xyz cmd;" */
	మాట_IS_KEYWORD       = 3,
};

#నిర్వచితం_కాకపోతే __U_BOOT__
/* On program start, environ points to initial environment.
 * putenv adds new pointers into it, unsetenv removes them.
 * Neither of these (de)allocates the strings.
 * setenv allocates new strings in malloc space and does putenv,
 * and thus setenv is unusable (leaky) for shell's purposes */
#నిర్వచించు setenv(...) setenv_is_leaky_dont_use()
#అయితే_ముగింపు /* !__U_BOOT__ */
నిర్మాణం variable {
	నిర్మాణం variable *తదుపరి;
	అక్షరం *varstr;        /* points to "name=" portion */
	పూర్ణాంకం max_len;         /* if > 0, name is part of initial env; else name is malloced */
#నిర్వచితం_కాకపోతే __U_BOOT__
	uint16_t var_nest_level;
	smallint flg_export; /* putenv should be done on this var */
	smallint flg_read_only;
#అయితే_ముగింపు /* !__U_BOOT__ */
};

గణన {
	BC_BREAK = 1,
	BC_CONTINUE = 2,
};

#ఒకవేళ ENABLE_HUSH_FUNCTIONS
నిర్మాణం function {
	నిర్మాణం function *తదుపరి;
	అక్షరం *name;
	నిర్మాణం command *మాతృ_cmd;
	నిర్మాణం pipe *body;
# ఒకవేళ !BB_MMU
	అక్షరం *body_as_string;
# endif
};
#అయితే_ముగింపు

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
స్థిర స్థిరరాశి అక్షరం o_opt_అక్షరశ్రేణులు[] ALIGN1 =
	"pipefail\0"
	"noexec\0"
	"errexit\0"
#ఒకవేళ ENABLE_HUSH_బహుళకం_X
	"xtrace\0"
#అయితే_ముగింపు
	;
గణన {
	OPT_O_PIPEFAIL,
	OPT_O_NOEXEC,
	OPT_O_ERREXIT,
#ఒకవేళ ENABLE_HUSH_బహుళకం_X
	OPT_O_XTRACE,
#అయితే_ముగింపు
	అంకె_OPT_O
};

/* "Globals" within this file */
/* Sorted roughly by size (smaller offsets == smaller code) */
నిర్మాణం globals {
#నిర్వచితం_కాకపోతే __U_BOOT__
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
#ఒకవేళ ENABLE_HUSH_INTERACTIVE
	/* 'interactive_fd' is a fd# open to ctty, if we have one
	 * _AND_ if we decided to act interactively */
	పూర్ణాంకం interactive_fd;
	IF_NOT_FEATURE_EDITING_FANCY_PROMPT(అక్షరం *PS1;)
# define G_interactive_fd (G.interactive_fd)
#లేకపోతే
# define G_interactive_fd 0
#అయితే_ముగింపు
#లేకపోతే /* __U_BOOT__ */
# define G_interactive_fd 0
#అయితే_ముగింపు /* __U_BOOT__ */
#నిర్వచితం_కాకపోతే __U_BOOT__
#ఒకవేళ ENABLE_FEATURE_EDITING
	పంక్తి_ఆదానం_t *పంక్తి_ఆదానం_state;
#అయితే_ముగింపు
	pid_t మూలం_pid;
	pid_t మూలం_ppid;
	pid_t చివరి_bg_pid;
#ఒకవేళ ENABLE_HUSH_RANDOM_SUPPORT
	random_t random_gen;
#అయితే_ముగింపు
#ఒకవేళ ENABLE_HUSH_పని
	పూర్ణాంకం నిరంతరం_list_level;
	గుర్తులేని చివరి_jobid;
	pid_t saved_tty_pgrp;
	నిర్మాణం pipe *పని_list;
# define G_saved_tty_pgrp (G.saved_tty_pgrp)
#లేకపోతే
# define G_saved_tty_pgrp 0
#అయితే_ముగింపు
#అయితే_ముగింపు /* !__U_BOOT__ */
	/* How deeply are we in context where "set -e" is ignored */
	పూర్ణాంకం errexit_లోతు;
#నిర్వచితం_కాకపోతే __U_BOOT__
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

	అక్షరం o_opt[అంకె_OPT_O];
#ఒకవేళ ENABLE_HUSH_బహుళకం_X
# define G_x_బహుళకం (G.o_opt[OPT_O_XTRACE])
#లేకపోతే
# define G_x_బహుళకం 0
#అయితే_ముగింపు
	అక్షరం opt_s;
	అక్షరం opt_c;
#అయితే_ముగింపు /* !__U_BOOT__ */
#ఒకవేళ ENABLE_HUSH_INTERACTIVE
	smallint promptmode; /* 0: PS1, 1: PS2 */
#అయితే_ముగింపు
	/* set by signal handler if SIGINT is received _and_ its trap is not set */
	smallint flag_SIGINT;
#నిర్వచితం_కాకపోతే __U_BOOT__
#ఒకవేళ ENABLE_HUSH_LOOPS
	smallint flag_break_continue;
#అయితే_ముగింపు
#అయితే_ముగింపు /* !__U_BOOT__ */
#ఒకవేళ ENABLE_HUSH_FUNCTIONS
	/* 0: outside of a function (or sourced file)
	 * -1: inside of a function, ok to use return builtin
	 * 1: return is invoked, skip all till end of func
	 */
	smallint flag_return_లోపల_పురోగతి;
# define G_flag_return_లోపల_పురోగతి (G.flag_return_లోపల_పురోగతి)
#లేకపోతే
# define G_flag_return_లోపల_పురోగతి 0
#అయితే_ముగింపు
	smallint exiting; /* used to prevent EXIT trap recursion */
	/* These support $? */
	smalluint చివరి_exitcode;
	smalluint expand_exitcode;
	smalluint చివరి_bg_pid_exitcode;
#నిర్వచితం_కాకపోతే __U_BOOT__
#ఒకవేళ ENABLE_HUSH_సమితి
	/* are global_argv and global_argv[1..n] malloced? (note: not [0]) */
	smalluint global_వాదనలు_malloced;
# define G_global_వాదనలు_malloced (G.global_వాదనలు_malloced)
#లేకపోతే
# define G_global_వాదనలు_malloced 0
#అయితే_ముగింపు
#ఒకవేళ ENABLE_HUSH_BASH_COMPAT
	పూర్ణాంకం dead_పని_exitcode; /* for "wait -n" */
#అయితే_ముగింపు
#అయితే_ముగింపు /* !__U_BOOT__ */
	/* how many non-NULL argv's we have. NB: $# + 1 */
	పూర్ణాంకం global_argc;
	అక్షరం **global_argv;
#ఒకవేళ !BB_MMU
	అక్షరం *argv0_for_re_execing;
#అయితే_ముగింపు
#ఒకవేళ ENABLE_HUSH_LOOPS
#నిర్వచితం_కాకపోతే __U_BOOT__
	గుర్తులేని లోతు_break_continue;
#అయితే_ముగింపు /* !__U_BOOT__ */
	గుర్తులేని లోతు_యొక్క_loop;
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే __U_BOOT__
#ఒకవేళ ENABLE_HUSH_GETOPTS
	గుర్తులేని getopt_లెక్క;
#అయితే_ముగింపు
#అయితే_ముగింపు /* !__U_BOOT__ */
	స్థిరరాశి అక్షరం *ifs;
#నిర్వచితమైతే __U_BOOT__
	పూర్ణాంకం flag_repeat;
	పూర్ణాంకం do_repeat;
	పూర్ణాంకం నిరంతరం_command_flags;
#అయితే_ముగింపు /* __U_BOOT__ */
	అక్షరం *ifs_whitespace; /* = G.ifs or malloced */
#నిర్వచితం_కాకపోతే __U_BOOT__
	స్థిరరాశి అక్షరం *cwd;
#అయితే_ముగింపు /* !__U_BOOT__ */
	నిర్మాణం variable *శిఖరం_var;
	అక్షరం **expanded_assignments;
	నిర్మాణం variable **shadowed_vars_pp;
	గుర్తులేని var_nest_level;
#నిర్వచితం_కాకపోతే __U_BOOT__
#ఒకవేళ ENABLE_HUSH_FUNCTIONS
# ఒకవేళ ENABLE_HUSH_LOCAL
	గుర్తులేని func_nest_level; /* solely to prevent "local v" in non-functions */
# endif
	నిర్మాణం function *శిఖరం_func;
#అయితే_ముగింపు
	/* Signal and trap handling */
#ఒకవేళ ENABLE_HUSH_FAST
	గుర్తులేని లెక్క_SIGCHLD;
	గుర్తులేని handled_SIGCHLD;
	smallint we_have_children;
#అయితే_ముగింపు
#ఒకవేళ ENABLE_HUSH_LINENO_VAR
	గుర్తులేని parse_lineno;
	గుర్తులేని execute_lineno;
#అయితే_ముగింపు
	HFILE *HFILE_list;
	HFILE *HFILE_stdin;
	/* Which signals have non-DFL handler (even with no traps set)?
	 * Set at the start to:
	 * (SIGQUIT + maybe SPECIAL_INTERACTIVE_SIGS + maybe SPECIAL_JOBSTOP_SIGS)
	 * SPECIAL_INTERACTIVE_SIGS are cleared after fork.
	 * The rest is cleared right before execv syscalls.
	 * Other than these two times, never modified.
	 */
	గుర్తులేని special_sig_ముసుగు;
#ఒకవేళ ENABLE_HUSH_పని
	గుర్తులేని fatal_sig_ముసుగు;
# define G_fatal_sig_ముసుగు (G.fatal_sig_ముసుగు)
#లేకపోతే
# define G_fatal_sig_ముసుగు 0
#అయితే_ముగింపు
#ఒకవేళ ENABLE_HUSH_TRAP
	పూర్ణాంకం pre_trap_exitcode;
# ఒకవేళ ENABLE_HUSH_FUNCTIONS
	పూర్ణాంకం return_exitcode;
# endif
	అక్షరం **traps; /* char *traps[NSIG] */
# define G_traps G.traps
#లేకపోతే
# define G_traps ((అక్షరం**)ఖాళీ)
#అయితే_ముగింపు
	sigset_t pending_సమితి;
#ఒకవేళ ENABLE_HUSH_MEMLEAK
	గుర్తులేని దీర్ఘ memleak_విలువ;
#అయితే_ముగింపు
#ఒకవేళ ENABLE_HUSH_బహుళకం_X
	గుర్తులేని x_బహుళకం_లోతు;
	/* "set -x" output should not be redirectable with subsequent 2>FILE.
	 * We dup fd#2 to x_mode_fd when "set -x" is executed, and use it
	 * for all subsequent output.
	 */
	పూర్ణాంకం x_బహుళకం_fd;
	o_string x_బహుళకం_buf;
#అయితే_ముగింపు
#అయితే_ముగింపు /* !__U_BOOT__ */
#ఒకవేళ HUSH_DEBUG >= 2
	పూర్ణాంకం debug_indent;
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే __U_BOOT__
	నిర్మాణం sigaction sa;
	అక్షరం optstring_buf[పరిమాణం("eixcs")];
#ఒకవేళ BASH_EPOCH_VARS
	అక్షరం epoch_buf[పరిమాణం("%llu.nnnnnn") + పరిమాణం(దీర్ఘ దీర్ఘ)*3];
#అయితే_ముగింపు
#ఒకవేళ ENABLE_FEATURE_EDITING
	అక్షరం user_ఆదానం_buf[CONFIG_FEATURE_EDITING_MAX_LEN];
#అయితే_ముగింపు
#అయితే_ముగింపు /* !__U_BOOT__ */
};
#నిర్వచితమైతే __U_BOOT__
నిర్మాణం globals *ptr_గమ్యం_globals;
#అయితే_ముగింపు /* __U_BOOT__ */
#నిర్వచించు G (*ptr_గమ్యం_globals)
/* Not #defining name to G.name - this quickly gets unwieldy
 * (too many defines). Also, I actually prefer to see when a variable
 * is global, thus "G." prefix is a useful hint */
#నిర్వచితమైతే __U_BOOT__
#నిర్వచించు సమితి_PTR_గమ్యం_GLOBALS(x) చేయి { \
	(*(నిర్మాణం globals**)&ptr_గమ్యం_globals) = (శూన్య*)(x); \
	barrier(); \
} అయితే (0)
#నిర్వచించు INIT_G() చేయి { \
	సమితి_PTR_గమ్యం_GLOBALS(xzalloc(పరిమాణం(G))); \
	G.promptmode = 1; \
} అయితే (0)
#లేకపోతే /* !__U_BOOT__ */
#నిర్వచించు INIT_G() చేయి { \
	సమితి_PTR_గమ్యం_GLOBALS(xzalloc(పరిమాణం(G))); \
	/* memset(&G.sa, 0, sizeof(G.sa)); */  \
	sigfillset(&G.sa.sa_ముసుగు); \
	G.sa.sa_flags = SA_RESTART; \
} అయితే (0)
#అయితే_ముగింపు /* !__U_BOOT__ */

#నిర్వచితం_కాకపోతే __U_BOOT__
/* Function prototypes for builtins */
స్థిర పూర్ణాంకం builtin_cd(అక్షరం **argv) FAST_FUNC;
#ఒకవేళ ENABLE_HUSH_ECHO
స్థిర పూర్ణాంకం builtin_echo(అక్షరం **argv) FAST_FUNC;
#అయితే_ముగింపు
స్థిర పూర్ణాంకం builtin_eval(అక్షరం **argv) FAST_FUNC;
స్థిర పూర్ణాంకం builtin_exec(అక్షరం **argv) FAST_FUNC;
స్థిర పూర్ణాంకం builtin_exit(అక్షరం **argv) FAST_FUNC;
#ఒకవేళ ENABLE_HUSH_EXPORT
స్థిర పూర్ణాంకం builtin_export(అక్షరం **argv) FAST_FUNC;
#అయితే_ముగింపు
#ఒకవేళ ENABLE_HUSH_READONLY
స్థిర పూర్ణాంకం builtin_readonly(అక్షరం **argv) FAST_FUNC;
#అయితే_ముగింపు
స్థిర పూర్ణాంకం builtin_false(అక్షరం **argv) FAST_FUNC;
#ఒకవేళ ENABLE_HUSH_పని
స్థిర పూర్ణాంకం builtin_fg_bg(అక్షరం **argv) FAST_FUNC;
స్థిర పూర్ణాంకం builtin_jobs(అక్షరం **argv) FAST_FUNC;
#అయితే_ముగింపు
#ఒకవేళ ENABLE_HUSH_GETOPTS
స్థిర పూర్ణాంకం builtin_getopts(అక్షరం **argv) FAST_FUNC;
#అయితే_ముగింపు
#ఒకవేళ ENABLE_HUSH_HELP
స్థిర పూర్ణాంకం builtin_help(అక్షరం **argv) FAST_FUNC;
#అయితే_ముగింపు
#ఒకవేళ MAX_HISTORY && ENABLE_FEATURE_EDITING
స్థిర పూర్ణాంకం builtin_history(అక్షరం **argv) FAST_FUNC;
#అయితే_ముగింపు
#ఒకవేళ ENABLE_HUSH_LOCAL
స్థిర పూర్ణాంకం builtin_local(అక్షరం **argv) FAST_FUNC;
#అయితే_ముగింపు
#ఒకవేళ ENABLE_HUSH_MEMLEAK
స్థిర పూర్ణాంకం builtin_memleak(అక్షరం **argv) FAST_FUNC;
#అయితే_ముగింపు
#ఒకవేళ ENABLE_HUSH_PRINTF
స్థిర పూర్ణాంకం builtin_printf(అక్షరం **argv) FAST_FUNC;
#అయితే_ముగింపు
స్థిర పూర్ణాంకం builtin_pwd(అక్షరం **argv) FAST_FUNC;
#ఒకవేళ ENABLE_HUSH_READ
స్థిర పూర్ణాంకం builtin_read(అక్షరం **argv) FAST_FUNC;
#అయితే_ముగింపు
#ఒకవేళ ENABLE_HUSH_సమితి
స్థిర పూర్ణాంకం builtin_సమితి(అక్షరం **argv) FAST_FUNC;
#అయితే_ముగింపు
స్థిర పూర్ణాంకం builtin_జరుపు(అక్షరం **argv) FAST_FUNC;
స్థిర పూర్ణాంకం builtin_ఆధారం(అక్షరం **argv) FAST_FUNC;
#ఒకవేళ ENABLE_HUSH_TEST || BASH_TEST2
స్థిర పూర్ణాంకం builtin_test(అక్షరం **argv) FAST_FUNC;
#అయితే_ముగింపు
#ఒకవేళ ENABLE_HUSH_TRAP
స్థిర పూర్ణాంకం builtin_trap(అక్షరం **argv) FAST_FUNC;
#అయితే_ముగింపు
#ఒకవేళ ENABLE_HUSH_TYPE
స్థిర పూర్ణాంకం builtin_type(అక్షరం **argv) FAST_FUNC;
#అయితే_ముగింపు
#ఒకవేళ ENABLE_HUSH_TIMES
స్థిర పూర్ణాంకం builtin_times(అక్షరం **argv) FAST_FUNC;
#అయితే_ముగింపు
స్థిర పూర్ణాంకం builtin_true(అక్షరం **argv) FAST_FUNC;
#ఒకవేళ ENABLE_HUSH_UMASK
స్థిర పూర్ణాంకం builtin_umask(అక్షరం **argv) FAST_FUNC;
#అయితే_ముగింపు
#ఒకవేళ ENABLE_HUSH_UNSET
స్థిర పూర్ణాంకం builtin_unset(అక్షరం **argv) FAST_FUNC;
#అయితే_ముగింపు
#ఒకవేళ ENABLE_HUSH_KILL
స్థిర పూర్ణాంకం builtin_kill(అక్షరం **argv) FAST_FUNC;
#అయితే_ముగింపు
#ఒకవేళ ENABLE_HUSH_WAIT
స్థిర పూర్ణాంకం builtin_wait(అక్షరం **argv) FAST_FUNC;
#అయితే_ముగింపు
#ఒకవేళ ENABLE_HUSH_LOOPS
స్థిర పూర్ణాంకం builtin_break(అక్షరం **argv) FAST_FUNC;
స్థిర పూర్ణాంకం builtin_continue(అక్షరం **argv) FAST_FUNC;
#అయితే_ముగింపు
#ఒకవేళ ENABLE_HUSH_FUNCTIONS
స్థిర పూర్ణాంకం builtin_return(అక్షరం **argv) FAST_FUNC;
#అయితే_ముగింపు

/* Table of built-in functions.  They can be forked or not, depending on
 * context: within pipes, they fork.  As simple commands, they do not.
 * When used in non-forking context, they can change global variables
 * in the parent shell process.  If forked, of course they cannot.
 * For example, 'unset foo | whatever' will parse and run, but foo will
 * still be set at the end. */
నిర్మాణం built_లోపల_command {
	స్థిరరాశి అక్షరం *b_cmd;
	పూర్ణాంకం (*b_function)(అక్షరం **argv) FAST_FUNC;
#ఒకవేళ ENABLE_HUSH_HELP
	స్థిరరాశి అక్షరం *b_descr;
# define BLTIN(cmd, func, help) { cmd, func, help }
#లేకపోతే
# define BLTIN(cmd, func, help) { cmd, func }
#అయితే_ముగింపు
};

స్థిర స్థిరరాశి నిర్మాణం built_లోపల_command bltins1[] ALIGN_PTR = {
	BLTIN("."        , builtin_ఆధారం  , "Run commands in file"),
	BLTIN(":"        , builtin_true    , ఖాళీ),
#ఒకవేళ ENABLE_HUSH_పని
	BLTIN("bg"       , builtin_fg_bg   , "Resume job in background"),
#అయితే_ముగింపు
#ఒకవేళ ENABLE_HUSH_LOOPS
	BLTIN("break"    , builtin_break   , "Exit loop"),
#అయితే_ముగింపు
	BLTIN("cd"       , builtin_cd      , "Change directory"),
#ఒకవేళ ENABLE_HUSH_LOOPS
	BLTIN("continue" , builtin_continue, "Start new loop iteration"),
#అయితే_ముగింపు
	BLTIN("eval"     , builtin_eval    , "Construct and run shell command"),
	BLTIN("exec"     , builtin_exec    , "Execute command, don't return to shell"),
	BLTIN("exit"     , builtin_exit    , ఖాళీ),
#ఒకవేళ ENABLE_HUSH_EXPORT
	BLTIN("export"   , builtin_export  , "Set environment variables"),
#అయితే_ముగింపు
	BLTIN("false"    , builtin_false   , ఖాళీ),
#ఒకవేళ ENABLE_HUSH_పని
	BLTIN("fg"       , builtin_fg_bg   , "Bring job to foreground"),
#అయితే_ముగింపు
#ఒకవేళ ENABLE_HUSH_GETOPTS
	BLTIN("getopts"  , builtin_getopts , ఖాళీ),
#అయితే_ముగింపు
#ఒకవేళ ENABLE_HUSH_HELP
	BLTIN("help"     , builtin_help    , ఖాళీ),
#అయితే_ముగింపు
#ఒకవేళ MAX_HISTORY && ENABLE_FEATURE_EDITING
	BLTIN("history"  , builtin_history , "Show history"),
#అయితే_ముగింపు
#ఒకవేళ ENABLE_HUSH_పని
	BLTIN("jobs"     , builtin_jobs    , "List jobs"),
#అయితే_ముగింపు
#ఒకవేళ ENABLE_HUSH_KILL
	BLTIN("kill"     , builtin_kill    , "Send signals to processes"),
#అయితే_ముగింపు
#ఒకవేళ ENABLE_HUSH_LOCAL
	BLTIN("local"    , builtin_local   , "Set local variables"),
#అయితే_ముగింపు
#ఒకవేళ ENABLE_HUSH_MEMLEAK
	BLTIN("memleak"  , builtin_memleak , ఖాళీ),
#అయితే_ముగింపు
#ఒకవేళ ENABLE_HUSH_READ
	BLTIN("read"     , builtin_read    , "Input into variable"),
#అయితే_ముగింపు
#ఒకవేళ ENABLE_HUSH_READONLY
	BLTIN("readonly" , builtin_readonly, "Make variables read-only"),
#అయితే_ముగింపు
#ఒకవేళ ENABLE_HUSH_FUNCTIONS
	BLTIN("return"   , builtin_return  , "Return from function"),
#అయితే_ముగింపు
#ఒకవేళ ENABLE_HUSH_సమితి
	BLTIN("set"      , builtin_సమితి     , "Set positional parameters"),
#అయితే_ముగింపు
	BLTIN("shift"    , builtin_జరుపు   , "Shift positional parameters"),
#ఒకవేళ BASH_ఆధారం
	BLTIN("source"   , builtin_ఆధారం  , ఖాళీ),
#అయితే_ముగింపు
#ఒకవేళ ENABLE_HUSH_TIMES
	BLTIN("times"    , builtin_times   , ఖాళీ),
#అయితే_ముగింపు
#ఒకవేళ ENABLE_HUSH_TRAP
	BLTIN("trap"     , builtin_trap    , "Trap signals"),
#అయితే_ముగింపు
	BLTIN("true"     , builtin_true    , ఖాళీ),
#ఒకవేళ ENABLE_HUSH_TYPE
	BLTIN("type"     , builtin_type    , "Show command type"),
#అయితే_ముగింపు
#ఒకవేళ ENABLE_HUSH_ULIMIT
	BLTIN("ulimit"   , shell_builtin_ulimit, "Control resource limits"),
#అయితే_ముగింపు
#ఒకవేళ ENABLE_HUSH_UMASK
	BLTIN("umask"    , builtin_umask   , "Set file creation mask"),
#అయితే_ముగింపు
#ఒకవేళ ENABLE_HUSH_UNSET
	BLTIN("unset"    , builtin_unset   , "Unset variables"),
#అయితే_ముగింపు
#ఒకవేళ ENABLE_HUSH_WAIT
	BLTIN("wait"     , builtin_wait    , "Wait for process to finish"),
#అయితే_ముగింపు
};
/* These builtins won't be used if we are on NOMMU and need to re-exec
 * (it's cheaper to run an external program in this case):
 */
స్థిర స్థిరరాశి నిర్మాణం built_లోపల_command bltins2[] ALIGN_PTR = {
#ఒకవేళ ENABLE_HUSH_TEST
	BLTIN("["        , builtin_test    , ఖాళీ),
#అయితే_ముగింపు
#ఒకవేళ BASH_TEST2
	BLTIN("[["       , builtin_test    , ఖాళీ),
#అయితే_ముగింపు
#ఒకవేళ ENABLE_HUSH_ECHO
	BLTIN("echo"     , builtin_echo    , ఖాళీ),
#అయితే_ముగింపు
#ఒకవేళ ENABLE_HUSH_PRINTF
	BLTIN("printf"   , builtin_printf  , ఖాళీ),
#అయితే_ముగింపు
	BLTIN("pwd"      , builtin_pwd     , ఖాళీ),
#ఒకవేళ ENABLE_HUSH_TEST
	BLTIN("test"     , builtin_test    , ఖాళీ),
#అయితే_ముగింపు
};

#అయితే_ముగింపు /* !__U_BOOT__ */

/* Debug printouts.
 */
#ఒకవేళ HUSH_DEBUG >= 2
/* prevent disasters with G.debug_indent < 0 */
# define indent() fdprintf(2, "%*s", (G.debug_indent * 2) & 0xff, "")
# define debug_enter() (G.debug_indent++)
# define debug_leave() (G.debug_indent--)
#లేకపోతే
# define indent()      ((శూన్య)0)
# define debug_enter() ((శూన్య)0)
# define debug_leave() ((శూన్య)0)
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే debug_printf
# define debug_printf(...) (indent(), fdprintf(2, __VA_ARGS__))
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే debug_printf_parse
# define debug_printf_parse(...) (indent(), fdprintf(2, __VA_ARGS__))
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే debug_printf_heredoc
# define debug_printf_heredoc(...) (indent(), fdprintf(2, __VA_ARGS__))
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే debug_printf_exec
#నిర్వచించు debug_printf_exec(...) (indent(), fdprintf(2, __VA_ARGS__))
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే debug_printf_env
# define debug_printf_env(...) (indent(), fdprintf(2, __VA_ARGS__))
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే debug_printf_jobs
# define debug_printf_jobs(...) (indent(), fdprintf(2, __VA_ARGS__))
# define DEBUG_JOBS 1
#లేకపోతే
# define DEBUG_JOBS 0
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే debug_printf_expand
# define debug_printf_expand(...) (indent(), fdprintf(2, __VA_ARGS__))
# define DEBUG_EXPAND 1
#లేకపోతే
# define DEBUG_EXPAND 0
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే debug_printf_varexp
# define debug_printf_varexp(...) (indent(), fdprintf(2, __VA_ARGS__))
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే debug_printf_glob
# define debug_printf_glob(...) (indent(), fdprintf(2, __VA_ARGS__))
# define DEBUG_GLOB 1
#లేకపోతే
# define DEBUG_GLOB 0
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే debug_printf_redir
# define debug_printf_redir(...) (indent(), fdprintf(2, __VA_ARGS__))
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే debug_printf_list
# define debug_printf_list(...) (indent(), fdprintf(2, __VA_ARGS__))
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే debug_printf_subst
# define debug_printf_subst(...) (indent(), fdprintf(2, __VA_ARGS__))
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే debug_printf_prompt
# define debug_printf_prompt(...) (indent(), fdprintf(2, __VA_ARGS__))
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే debug_printf_clean
# define debug_printf_clean(...) (indent(), fdprintf(2, __VA_ARGS__))
# define DEBUG_CLEAN 1
#లేకపోతే
# define DEBUG_CLEAN 0
#అయితే_ముగింపు

#ఒకవేళ DEBUG_EXPAND
స్థిర శూన్య debug_print_అక్షరశ్రేణులు(స్థిరరాశి అక్షరం *ఉపసర్గ, అక్షరం **vv)
{
	indent();
	fdprintf(2, "%s:\n", ఉపసర్గ);
	అయితే (*vv)
		fdprintf(2, " '%s'\n", *vv++);
}
#లేకపోతే
# define debug_print_అక్షరశ్రేణులు(ఉపసర్గ, vv) ((శూన్య)0)
#అయితే_ముగింపు

/* Leak hunting. Use hush_leaktool.sh for post-processing.
 */
#ఒకవేళ LEAK_HUNTING
స్థిర శూన్య *xxmalloc(పూర్ణాంకం lineno, size_t కొలత)
{
	శూన్య *ptr = xmalloc((కొలత + 0xff) & ~0xff);
	fdprintf(2, "line %d: malloc %p\n", lineno, ptr);
	తిరిగి ptr;
}
స్థిర శూన్య *xxrealloc(పూర్ణాంకం lineno, శూన్య *ptr, size_t కొలత)
{
	ptr = xrealloc(ptr, (కొలత + 0xff) & ~0xff);
	fdprintf(2, "line %d: realloc %p\n", lineno, ptr);
	తిరిగి ptr;
}
స్థిర అక్షరం *xxstrdup(పూర్ణాంకం lineno, స్థిరరాశి అక్షరం *str)
{
	అక్షరం *ptr = xstrdup(str);
	fdprintf(2, "line %d: strdup %p\n", lineno, ptr);
	తిరిగి ptr;
}
స్థిర శూన్య xxfree(శూన్య *ptr)
{
	fdprintf(2, "free %p\n", ptr);
	విడుదల(ptr);
}
# define xmalloc(s)     xxmalloc(__LINE__, s)
# define xrealloc(p, s) xxrealloc(__LINE__, p, s)
# define xstrdup(s)     xxstrdup(__LINE__, s)
# define విడుదల(p)        xxfree(p)
#అయితే_ముగింపు

/* Syntax and runtime errors. They always abort scripts.
 * In interactive use they usually discard unparsed and/or unexecuted commands
 * and return to the prompt.
 * HUSH_DEBUG >= 2 prints line number in this file where it was detected.
 */
#ఒకవేళ HUSH_DEBUG < 2
# define సందేశం_and_die_if_script(lineno, ...)     సందేశం_and_die_if_script(__VA_ARGS__)
# define syntax_error(lineno, సందేశం)              syntax_error(సందేశం)
# define syntax_error_at(lineno, సందేశం)           syntax_error_at(సందేశం)
# define syntax_error_unterm_ch(lineno, ch)     syntax_error_unterm_ch(ch)
# define syntax_error_unterm_str(lineno, s)     syntax_error_unterm_str(s)
# define syntax_error_unexpected_ch(lineno, ch) syntax_error_unexpected_ch(ch)
#అయితే_ముగింపు

స్థిర శూన్య die_if_script(శూన్య)
{
	ఒకవేళ (!G_interactive_fd) {
		ఒకవేళ (G.చివరి_exitcode) /* sometines it's 2, not 1 (bash compat) */
			xfunc_error_retval = G.చివరి_exitcode;
		xfunc_die();
	}
}

#నిర్వచితమైతే __U_BOOT__
స్థిర శూన్య __maybe_unused సందేశం_and_die_if_script(గుర్తులేని lineno, స్థిరరాశి అక్షరం *fmt, ...)
#లేకపోతే /* !__U_BOOT__ */
స్థిర శూన్య సందేశం_and_die_if_script(గుర్తులేని lineno, స్థిరరాశి అక్షరం *fmt, ...)
#అయితే_ముగింపు /* !__U_BOOT__ */
{
	va_list p;

#ఒకవేళ HUSH_DEBUG >= 2
	bb_error_సందేశం("hush.c:%u", lineno);
#అయితే_ముగింపు
	va_ప్రారంభం(p, fmt);
	bb_verror_సందేశం(fmt, p, ఖాళీ);
	va_చివర(p);
	die_if_script();
}

స్థిర శూన్య syntax_error(గుర్తులేని lineno వాడని_PARAM, స్థిరరాశి అక్షరం *సందేశం)
{
	ఒకవేళ (సందేశం)
		bb_error_సందేశం("syntax error: %s", సందేశం);
	లేకపోతే
		bb_simple_error_సందేశం("syntax error");
	die_if_script();
}

స్థిర శూన్య syntax_error_at(గుర్తులేని lineno వాడని_PARAM, స్థిరరాశి అక్షరం *సందేశం)
{
	bb_error_సందేశం("syntax error at '%s'", సందేశం);
	die_if_script();
}

స్థిర శూన్య syntax_error_unterm_str(గుర్తులేని lineno వాడని_PARAM, స్థిరరాశి అక్షరం *s)
{
	bb_error_సందేశం("syntax error: unterminated %s", s);
//? source4.tests fails: in bash, echo ${^} in script does not terminate the script
// (but bash --posix, or if bash is run as "sh", does terminate in script, so maybe uncomment this?)
//	die_if_script();
}

స్థిర శూన్య syntax_error_unterm_ch(గుర్తులేని lineno, అక్షరం ch)
{
	అక్షరం సందేశం[2] = { ch, '\0' };
	syntax_error_unterm_str(lineno, సందేశం);
}

స్థిర శూన్య syntax_error_unexpected_ch(గుర్తులేని lineno వాడని_PARAM, పూర్ణాంకం ch)
{
	అక్షరం సందేశం[2];
	సందేశం[0] = ch;
	సందేశం[1] = '\0';
#ఒకవేళ HUSH_DEBUG >= 2
	bb_error_సందేశం("hush.c:%u", lineno);
#అయితే_ముగింపు
	bb_error_సందేశం("syntax error: unexpected %s", ch == EOF ? "EOF" : సందేశం);
	die_if_script();
}

#ఒకవేళ HUSH_DEBUG < 2
# undef సందేశం_and_die_if_script
# undef syntax_error
# undef syntax_error_at
# undef syntax_error_unterm_ch
# undef syntax_error_unterm_str
# undef syntax_error_unexpected_ch
#లేకపోతే
# define సందేశం_and_die_if_script(...)     సందేశం_and_die_if_script(__LINE__, __VA_ARGS__)
# define syntax_error(సందేశం)              syntax_error(__LINE__, సందేశం)
# define syntax_error_at(సందేశం)           syntax_error_at(__LINE__, సందేశం)
# define syntax_error_unterm_ch(ch)     syntax_error_unterm_ch(__LINE__, ch)
# define syntax_error_unterm_str(s)     syntax_error_unterm_str(__LINE__, s)
# define syntax_error_unexpected_ch(ch) syntax_error_unexpected_ch(__LINE__, ch)
#అయితే_ముగింపు

/* Utility functions
 */
/* Replace each \x with x in place, return ptr past NUL. */
స్థిర అక్షరం *unbackslash(అక్షరం *src)
{
#నిర్వచితమైతే __U_BOOT__
	అక్షరం *dst = src = (అక్షరం *)strchrnul(src, '\\');
#లేకపోతే /* !__U_BOOT__ */
	అక్షరం *dst = src = strchrnul(src, '\\');
#అయితే_ముగింపు /* !__U_BOOT__ */
	అయితే (1) {
		ఒకవేళ (*src == '\\') {
			src++;
			ఒకవేళ (*src != '\0') {
				/* \x -> x */
				*dst++ = *src++;
				కొనసాగు;
			}
			/* else: "\<nul>". Do not delete this backslash.
			 * Testcase: eval 'echo ok\'
			 */
			*dst++ = '\\';
			/* fallthrough */
		}
		ఒకవేళ ((*dst++ = *src++) == '\0')
			ఆపు;
	}
	తిరిగి dst;
}

స్థిర అక్షరం **కూడు_అక్షరశ్రేణులు_గమ్యం_అక్షరశ్రేణులు(అక్షరం **అక్షరశ్రేణులు, అక్షరం **కూడు, పూర్ణాంకం need_గమ్యం_dup)
{
	పూర్ణాంకం i;
	గుర్తులేని count1;
	గుర్తులేని count2;
	అక్షరం **v;

	v = అక్షరశ్రేణులు;
	count1 = 0;
	ఒకవేళ (v) {
		అయితే (*v) {
			count1++;
			v++;
		}
	}
	count2 = 0;
	v = కూడు;
	అయితే (*v) {
		count2++;
		v++;
	}
	v = xrealloc(అక్షరశ్రేణులు, (count1 + count2 + 1) * పరిమాణం(అక్షరం*));
	v[count1 + count2] = ఖాళీ;
	i = count2;
	అయితే (--i >= 0)
		v[count1 + i] = (need_గమ్యం_dup ? xstrdup(కూడు[i]) : కూడు[i]);
	తిరిగి v;
}
#ఒకవేళ LEAK_HUNTING
స్థిర అక్షరం **xx_కూడు_అక్షరశ్రేణులు_గమ్యం_అక్షరశ్రేణులు(పూర్ణాంకం lineno, అక్షరం **అక్షరశ్రేణులు, అక్షరం **కూడు, పూర్ణాంకం need_గమ్యం_dup)
{
	అక్షరం **ptr = కూడు_అక్షరశ్రేణులు_గమ్యం_అక్షరశ్రేణులు(అక్షరశ్రేణులు, కూడు, need_గమ్యం_dup);
	fdprintf(2, "line %d: add_strings_to_strings %p\n", lineno, ptr);
	తిరిగి ptr;
}
#నిర్వచించు కూడు_అక్షరశ్రేణులు_గమ్యం_అక్షరశ్రేణులు(అక్షరశ్రేణులు, కూడు, need_గమ్యం_dup) \
	xx_కూడు_అక్షరశ్రేణులు_గమ్యం_అక్షరశ్రేణులు(__LINE__, అక్షరశ్రేణులు, కూడు, need_గమ్యం_dup)
#అయితే_ముగింపు

/* Note: takes ownership of "add" ptr (it is not strdup'ed) */
స్థిర అక్షరం **కూడు_string_గమ్యం_అక్షరశ్రేణులు(అక్షరం **అక్షరశ్రేణులు, అక్షరం *కూడు)
{
	అక్షరం *v[2];
	v[0] = కూడు;
	v[1] = ఖాళీ;
	తిరిగి కూడు_అక్షరశ్రేణులు_గమ్యం_అక్షరశ్రేణులు(అక్షరశ్రేణులు, v, /*dup:*/ 0);
}

#ఒకవేళ LEAK_HUNTING
స్థిర అక్షరం **xx_కూడు_string_గమ్యం_అక్షరశ్రేణులు(పూర్ణాంకం lineno, అక్షరం **అక్షరశ్రేణులు, అక్షరం *కూడు)
{
	అక్షరం **ptr = కూడు_string_గమ్యం_అక్షరశ్రేణులు(అక్షరశ్రేణులు, కూడు);
	fdprintf(2, "line %d: add_string_to_strings %p\n", lineno, ptr);
	తిరిగి ptr;
}
#నిర్వచించు కూడు_string_గమ్యం_అక్షరశ్రేణులు(అక్షరశ్రేణులు, కూడు) \
	xx_కూడు_string_గమ్యం_అక్షరశ్రేణులు(__LINE__, అక్షరశ్రేణులు, కూడు)
#అయితే_ముగింపు

స్థిర శూన్య free_అక్షరశ్రేణులు(అక్షరం **అక్షరశ్రేణులు)
{
	అక్షరం **v;

	ఒకవేళ (!అక్షరశ్రేణులు)
		తిరిగి;
	v = అక్షరశ్రేణులు;
	అయితే (*v) {
		విడుదల(*v);
		v++;
	}
	విడుదల(అక్షరశ్రేణులు);
}

#నిర్వచితం_కాకపోతే __U_BOOT__
స్థిర పూర్ణాంకం dup_CLOEXEC(పూర్ణాంకం fd, పూర్ణాంకం avoid_fd)
{
	పూర్ణాంకం newfd;
 repeat:
	newfd = fcntl(fd, F_DUPFD_CLOEXEC, avoid_fd + 1);
	ఒకవేళ (newfd >= 0) {
		ఒకవేళ (F_DUPFD_CLOEXEC == F_DUPFD) /* if old libc (w/o F_DUPFD_CLOEXEC) */
			close_on_exec_on(newfd);
	} లేకపోతే { /* newfd < 0 */
		ఒకవేళ (errno == EBUSY)
			వెళ్ళు repeat;
		ఒకవేళ (errno == EINTR)
			వెళ్ళు repeat;
		ఒకవేళ (errno != EBADF) {
			/* "echo >&9999" gets EINVAL trying to save fd 1 to above 9999.
			 * We could try saving it _below_ 9999 instead (how?), but
			 * this probably means that dup2(9999,1) to effectuate >&9999
			 * would also not work: fd 9999 can't exist.
			 * (This differs from "echo >&99" where saving works, but
			 * subsequent dup2(99,1) fails if fd 99 is not open).
			 */
			bb_perror_సందేశం("fcntl(%d,F_DUPFD,%d)", fd, avoid_fd + 1);
		}
	}
	తిరిగి newfd;
}

స్థిర పూర్ణాంకం xdup_CLOEXEC_and_close(పూర్ణాంకం fd, పూర్ణాంకం avoid_fd)
{
	పూర్ణాంకం newfd;
 repeat:
	newfd = fcntl(fd, F_DUPFD_CLOEXEC, avoid_fd + 1);
	ఒకవేళ (newfd < 0) {
		ఒకవేళ (errno == EBUSY)
			వెళ్ళు repeat;
		ఒకవేళ (errno == EINTR)
			వెళ్ళు repeat;
		/* fd was not open? */
		ఒకవేళ (errno == EBADF)
			తిరిగి fd;
		xfunc_die();
	}
	ఒకవేళ (F_DUPFD_CLOEXEC == F_DUPFD) /* if old libc (w/o F_DUPFD_CLOEXEC) */
		close_on_exec_on(newfd);
	close(fd);
	తిరిగి newfd;
}

/* Manipulating HFILEs */
స్థిర HFILE *hfopen(స్థిరరాశి అక్షరం *name)
{
	HFILE *fp;
	పూర్ణాంకం fd;

	fd = STDIN_FILENO;
	ఒకవేళ (name) {
		fd = open(name, O_RDONLY | O_CLOEXEC);
		ఒకవేళ (fd < 0)
			తిరిగి ఖాళీ;
		ఒకవేళ (O_CLOEXEC == 0) /* ancient libc */
			close_on_exec_on(fd);
	}

	fp = xmalloc(పరిమాణం(*fp));
	ఒకవేళ (name == ఖాళీ)
		G.HFILE_stdin = fp;
	fp->fd = fd;
	fp->cur = fp->చివర_2 = fp->buf;
	fp->తదుపరి_hfile = G.HFILE_list;
	G.HFILE_list = fp;
	తిరిగి fp;
}
స్థిర శూన్య hfclose(HFILE *fp)
{
	HFILE **pp = &G.HFILE_list;
	అయితే (*pp) {
		HFILE *cur = *pp;
		ఒకవేళ (cur == fp) {
			*pp = cur->తదుపరి_hfile;
			ఆపు;
		}
		pp = &cur->తదుపరి_hfile;
	}
	ఒకవేళ (fp->fd >= 0)
		close(fp->fd);
	విడుదల(fp);
}
స్థిర పూర్ణాంకం refill_HFILE_and_getc(HFILE *fp)
{
	పూర్ణాంకం n;

	ఒకవేళ (fp->fd < 0) {
		/* Already saw EOF */
		తిరిగి EOF;
	}
#ఒకవేళ ENABLE_HUSH_INTERACTIVE && !ENABLE_FEATURE_EDITING
	/* If user presses ^C, read() restarts after SIGINT (we use SA_RESTART).
	 * IOW: ^C will not immediately stop line input.
	 * But poll() is different: it does NOT restart after signals.
	 */
	ఒకవేళ (fp == G.HFILE_stdin) {
		నిర్మాణం pollfd pfd[1];
		pfd[0].fd = fp->fd;
		pfd[0].events = POLLIN;
		n = poll(pfd, 1, -1);
		ఒకవేళ (n < 0
		 /*&& errno == EINTR - assumed true */
		 && sigismember(&G.pending_సమితి, SIGINT)
		) {
			తిరిగి '\0';
		}
	}
#లేకపోతే
/* if FEATURE_EDITING=y, we do not use this routine for interactive input */
#అయితే_ముగింపు
	/* Try to buffer more input */
	n = సురక్షిత_read(fp->fd, fp->buf, పరిమాణం(fp->buf));
	ఒకవేళ (n < 0) {
		bb_simple_perror_సందేశం("read error");
		n = 0;
	}
	fp->cur = fp->buf;
	fp->చివర_2 = fp->buf + n;
	ఒకవేళ (n == 0) {
		/* EOF/error */
		close(fp->fd);
		fp->fd = -1;
		తిరిగి EOF;
	}
	తిరిగి (గుర్తులేని అక్షరం)(*fp->cur++);
}
/* Inlined for common case of non-empty buffer.
 */
స్థిర ALWAYS_INLINE పూర్ణాంకం hfgetc(HFILE *fp)
{
	ఒకవేళ (fp->cur < fp->చివర_2)
		తిరిగి (గుర్తులేని అక్షరం)(*fp->cur++);
	/* Buffer empty */
	తిరిగి refill_HFILE_and_getc(fp);
}
స్థిర పూర్ణాంకం move_HFILEs_on_redirect(పూర్ణాంకం fd, పూర్ణాంకం avoid_fd)
{
	HFILE *fl = G.HFILE_list;
	అయితే (fl) {
		ఒకవేళ (fd == fl->fd) {
			/* We use it only on script files, they are all CLOEXEC */
			fl->fd = xdup_CLOEXEC_and_close(fd, avoid_fd);
			debug_printf_redir("redirect_fd %d: matches a script fd, moving it to %d\n", fd, fl->fd);
			తిరిగి 1; /* "found and moved" */
		}
		fl = fl->తదుపరి_hfile;
	}
#ఒకవేళ ENABLE_HUSH_బహుళకం_X
	ఒకవేళ (G.x_బహుళకం_fd > 0 && fd == G.x_బహుళకం_fd) {
		G.x_బహుళకం_fd = xdup_CLOEXEC_and_close(fd, avoid_fd);
		తిరిగి 1; /* "found and moved" */
	}
#అయితే_ముగింపు
	తిరిగి 0; /* "not in the list" */
}
#ఒకవేళ ENABLE_FEATURE_SH_STANDALONE && BB_MMU
స్థిర శూన్య close_all_HFILE_list(శూన్య)
{
	HFILE *fl = G.HFILE_list;
	అయితే (fl) {
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
		ఒకవేళ (fl->fd > 0)
			/*hfclose(fl); - unsafe */
			close(fl->fd);
		fl = fl->తదుపరి_hfile;
	}
}
#అయితే_ముగింపు
స్థిర పూర్ణాంకం fd_లోపల_HFILEs(పూర్ణాంకం fd)
{
	HFILE *fl = G.HFILE_list;
	అయితే (fl) {
		ఒకవేళ (fl->fd == fd)
			తిరిగి 1;
		fl = fl->తదుపరి_hfile;
	}
	తిరిగి 0;
}

#అయితే_ముగింపు /* !__U_BOOT__ */

/* Helpers for setting new $n and restoring them back
 */
రకనిర్వచనం నిర్మాణం save_arg_t {
	అక్షరం *sv_argv0;
	అక్షరం **sv_g_argv;
	పూర్ణాంకం sv_g_argc;
#నిర్వచితం_కాకపోతే __U_BOOT__
	IF_HUSH_సమితి(smallint sv_g_malloced;)
#అయితే_ముగింపు /* !__U_BOOT__ */
} save_arg_t;

#నిర్వచితం_కాకపోతే __U_BOOT__
స్థిర శూన్య save_and_replace_G_వాదనలు(save_arg_t *sv, అక్షరం **argv)
{
	sv->sv_argv0 = argv[0];
	sv->sv_g_argv = G.global_argv;
	sv->sv_g_argc = G.global_argc;
	IF_HUSH_సమితి(sv->sv_g_malloced = G.global_వాదనలు_malloced;)

	argv[0] = G.global_argv[0]; /* retain $0 */
	G.global_argv = argv;
	IF_HUSH_సమితి(G.global_వాదనలు_malloced = 0;)

	G.global_argc = 1 + string_array_len(argv + 1);
}

స్థిర శూన్య restore_G_వాదనలు(save_arg_t *sv, అక్షరం **argv)
{
#ఒకవేళ ENABLE_HUSH_సమితి
	ఒకవేళ (G.global_వాదనలు_malloced) {
		/* someone ran "set -- arg1 arg2 ...", undo */
		అక్షరం **pp = G.global_argv;
		అయితే (*++pp) /* note: does not free $0 */
			విడుదల(*pp);
		విడుదల(G.global_argv);
	}
#అయితే_ముగింపు
	argv[0] = sv->sv_argv0;
	G.global_argv = sv->sv_g_argv;
	G.global_argc = sv->sv_g_argc;
	IF_HUSH_సమితి(G.global_వాదనలు_malloced = sv->sv_g_malloced;)
}
#అయితే_ముగింపు /* !__U_BOOT__ */

#నిర్వచితం_కాకపోతే __U_BOOT__
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
గణన {
	SPECIAL_INTERACTIVE_SIGS = 0
		| (1 << SIGTERM)
		| (1 << SIGINT)
		| (1 << SIGHUP)
		,
	SPECIAL_JOBSTOP_SIGS = 0
#ఒకవేళ ENABLE_HUSH_పని
		| (1 << SIGTTIN)
		| (1 << SIGTTOU)
		| (1 << SIGTSTP)
#అయితే_ముగింపు
		,
};

స్థిర శూన్య రికార్డు_pending_signo(పూర్ణాంకం sig)
{
	sigaddset(&G.pending_సమితి, sig);
#ఒకవేళ ENABLE_FEATURE_EDITING
	ఒకవేళ (sig != SIGCHLD
	 || (G_traps && G_traps[SIGCHLD] && G_traps[SIGCHLD][0])
	 /* ^^^ if SIGCHLD, interrupt line reading only if it has a trap */
	) {
		bb_got_signal = sig; /* for read_line_input: "we got a signal" */
	}
#అయితే_ముగింపు
#ఒకవేళ ENABLE_HUSH_FAST
	ఒకవేళ (sig == SIGCHLD) {
		G.లెక్క_SIGCHLD++;
//bb_error_msg("[%d] SIGCHLD_handler: G.count_SIGCHLD:%d G.handled_SIGCHLD:%d", getpid(), G.count_SIGCHLD, G.handled_SIGCHLD);
	}
#అయితే_ముగింపు
}

స్థిర sighandler_t install_sighandler(పూర్ణాంకం sig, sighandler_t handler)
{
	నిర్మాణం sigaction old_sa;

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
	తిరిగి old_sa.sa_handler;
}
#అయితే_ముగింపు /* !__U_BOOT__ */

#నిర్వచితం_కాకపోతే __U_BOOT__
స్థిర శూన్య hush_exit(పూర్ణాంకం exitcode) NORETURN;

స్థిర శూన్య restore_ttypgrp_and__exit(శూన్య) NORETURN;
స్థిర శూన్య restore_ttypgrp_and__exit(శూన్య)
{
	/* xfunc has failed! die die die */
	/* no EXIT traps, this is an escape hatch! */
	G.exiting = 1;
	hush_exit(xfunc_error_retval);
}

#ఒకవేళ ENABLE_HUSH_పని

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
స్థిర శూన్య fflush_and__exit(శూన్య) NORETURN;
స్థిర శూన్య fflush_and__exit(శూన్య)
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
స్థిర శూన్య sigexit(పూర్ణాంకం sig) NORETURN;
స్థిర శూన్య sigexit(పూర్ణాంకం sig)
{
	/* Careful: we can end up here after [v]fork. Do not restore
	 * tty pgrp then, only top-level shell process does that */
	ఒకవేళ (G_saved_tty_pgrp && getpid() == G.మూలం_pid) {
		/* Disable all signals: job control, SIGPIPE, etc.
		 * Mostly paranoid measure, to prevent infinite SIGTTOU.
		 */
		sigprocmask_allsigs(SIG_ఖండం);
		tcsetpgrp(G_interactive_fd, G_saved_tty_pgrp);
	}

	/* Not a signal, just exit */
	ఒకవేళ (sig <= 0)
		_exit(- sig);

	kill_myself_with_sig(sig); /* does not return */
}
#లేకపోతే

# define disable_restore_tty_pgrp_on_exit() ((శూన్య)0)
# define enable_restore_tty_pgrp_on_exit()  ((శూన్య)0)

#అయితే_ముగింపు
#అయితే_ముగింపు /* !__U_BOOT__ */

#నిర్వచితం_కాకపోతే __U_BOOT__
స్థిర sighandler_t pick_sighandler(గుర్తులేని sig)
{
	sighandler_t handler = SIG_DFL;
	ఒకవేళ (sig < పరిమాణం(గుర్తులేని)*8) {
		గుర్తులేని sigmask = (1 << sig);

#ఒకవేళ ENABLE_HUSH_పని
		/* is sig fatal? */
		ఒకవేళ (G_fatal_sig_ముసుగు & sigmask)
			handler = sigexit;
		లేకపోతే
#అయితే_ముగింపు
		/* sig has special handling? */
		ఒకవేళ (G.special_sig_ముసుగు & sigmask) {
			handler = రికార్డు_pending_signo;
			/* TTIN/TTOU/TSTP can't be set to record_pending_signo
			 * in order to ignore them: they will be raised
			 * in an endless loop when we try to do some
			 * terminal ioctls! We do have to _ignore_ these.
			 */
			ఒకవేళ (SPECIAL_JOBSTOP_SIGS & sigmask)
				handler = SIG_IGN;
		}
	}
	తిరిగి handler;
}
#అయితే_ముగింపు /* !__U_BOOT__ */

#నిర్వచితం_కాకపోతే __U_BOOT__
/* Restores tty foreground process group, and exits. */
స్థిర శూన్య hush_exit(పూర్ణాంకం exitcode)
{
#ఒకవేళ ENABLE_FEATURE_EDITING_SAVE_ON_EXIT
	save_history(G.పంక్తి_ఆదానం_state); /* may be NULL */
#అయితే_ముగింపు

	fflush_all();
	ఒకవేళ (G.exiting <= 0 && G_traps && G_traps[0] && G_traps[0][0]) {
		అక్షరం *argv[3];
		/* argv[0] is unused */
		argv[1] = xstrdup(G_traps[0]); /* copy, since EXIT trap handler may modify G_traps[0] */
		argv[2] = ఖాళీ;
		G.exiting = 1; /* prevent EXIT trap recursion */
		/* Note: G_traps[0] is not cleared!
		 * "trap" will still show it, if executed
		 * in the handler */
		builtin_eval(argv);
	}

#ఒకవేళ ENABLE_FEATURE_CLEAN_పైకి
	{
		నిర్మాణం variable *cur_var;
		ఒకవేళ (G.cwd != bb_సందేశం_unknown)
			విడుదల((అక్షరం*)G.cwd);
		cur_var = G.శిఖరం_var;
		అయితే (cur_var) {
			నిర్మాణం variable *tmp = cur_var;
			ఒకవేళ (!cur_var->max_len)
				విడుదల(cur_var->varstr);
			cur_var = cur_var->తదుపరి;
			విడుదల(tmp);
		}
	}
#అయితే_ముగింపు

	fflush_all();
#ఒకవేళ ENABLE_HUSH_పని
	sigexit(- (exitcode & 0xff));
#లేకపోతే
	_exit(exitcode);
#అయితే_ముగింపు
}

//TODO: return a mask of ALL handled sigs?
స్థిర పూర్ణాంకం check_and_నిరంతరం_traps(శూన్య)
{
	పూర్ణాంకం చివరి_sig = 0;

	అయితే (1) {
		పూర్ణాంకం sig;

		ఒకవేళ (sigisemptyset(&G.pending_సమితి))
			ఆపు;
		sig = 0;
		చేయి {
			sig++;
			ఒకవేళ (sigismember(&G.pending_సమితి, sig)) {
				sigdelset(&G.pending_సమితి, sig);
				వెళ్ళు got_sig;
			}
		} అయితే (sig < NSIG);
		ఆపు;
 got_sig:
#ఒకవేళ ENABLE_HUSH_TRAP
		ఒకవేళ (G_traps && G_traps[sig]) {
			debug_printf_exec("%s: sig:%d handler:'%s'\n", __func__, sig, G.traps[sig]);
			ఒకవేళ (G_traps[sig][0]) {
				/* We have user-defined handler */
				smalluint save_rcode;
				పూర్ణాంకం save_pre;
				అక్షరం *argv[3];
				/* argv[0] is unused */
				argv[1] = xstrdup(G_traps[sig]);
				/* why strdup? trap can modify itself: trap 'trap "echo oops" INT' INT */
				argv[2] = ఖాళీ;
				save_pre = G.pre_trap_exitcode;
				G.pre_trap_exitcode = save_rcode = G.చివరి_exitcode;
				builtin_eval(argv);
				విడుదల(argv[1]);
				G.pre_trap_exitcode = save_pre;
				G.చివరి_exitcode = save_rcode;
# ఒకవేళ ENABLE_HUSH_FUNCTIONS
				ఒకవేళ (G.return_exitcode >= 0) {
					debug_printf_exec("trap exitcode:%d\n", G.return_exitcode);
					G.చివరి_exitcode = G.return_exitcode;
				}
# endif
				చివరి_sig = sig;
			} /* else: "" trap, ignoring signal */
			కొనసాగు;
		}
#అయితే_ముగింపు
		/* not a trap: special action */
		ఎంపిక (sig) {
		స్థితి SIGINT:
			debug_printf_exec("%s: sig:%d default SIGINT handler\n", __func__, sig);
			G.flag_SIGINT = 1;
			చివరి_sig = sig;
			ఆపు;
#ఒకవేళ ENABLE_HUSH_పని
		స్థితి SIGHUP: {
			/* if (G_interactive_fd) - no need to check, the handler
			 * is only installed if we *are* interactive */
			{
				/* bash compat: "Before exiting, an interactive
				 * shell resends the SIGHUP to all jobs, running
				 * or stopped.  Stopped jobs are sent SIGCONT
				 * to ensure that they receive the SIGHUP."
				 */
				నిర్మాణం pipe *పని;
				debug_printf_exec("%s: sig:%d default SIGHUP handler\n", __func__, sig);
				/* bash is observed to signal whole process groups,
				 * not individual processes */
				కొరకు (పని = G.పని_list; పని; పని = పని->తదుపరి) {
					ఒకవేళ (పని->pgrp <= 0)
						కొనసాగు;
					debug_printf_exec("HUPing pgrp %d\n", పని->pgrp);
					ఒకవేళ (kill(- పని->pgrp, SIGHUP) == 0)
						kill(- పని->pgrp, SIGCONT);
				}
			}
			/* this restores tty pgrp, then kills us with SIGHUP */
			sigexit(SIGHUP);
		}
#అయితే_ముగింపు
#ఒకవేళ ENABLE_HUSH_FAST
		స్థితి SIGCHLD:
			debug_printf_exec("%s: sig:%d default SIGCHLD handler\n", __func__, sig);
			G.లెక్క_SIGCHLD++;
//bb_error_msg("[%d] check_and_run_traps: G.count_SIGCHLD:%d G.handled_SIGCHLD:%d", getpid(), G.count_SIGCHLD, G.handled_SIGCHLD);
			/* Note:
			 * We don't do 'last_sig = sig' here -> NOT returning this sig.
			 * This simplifies wait builtin a bit.
			 */
			ఆపు;
#అయితే_ముగింపు
		అప్రమేయ: /* ignored: */
			debug_printf_exec("%s: sig:%d default handling is to ignore\n", __func__, sig);
			/* SIGTERM, SIGQUIT, SIGTTIN, SIGTTOU, SIGTSTP */
			/* Note:
			 * We don't do 'last_sig = sig' here -> NOT returning this sig.
			 * Example: wait is not interrupted by TERM
			 * in interactive shell, because TERM is ignored.
			 */
			ఆపు;
		}
	}
	తిరిగి చివరి_sig;
}

స్థిర స్థిరరాశి అక్షరం *get_cwd(పూర్ణాంకం force)
{
	ఒకవేళ (force || G.cwd == ఖాళీ) {
		/* xrealloc_getcwd_or_warn(arg) calls free(arg),
		 * we must not try to free(bb_msg_unknown) */
		ఒకవేళ (G.cwd == bb_సందేశం_unknown)
			G.cwd = ఖాళీ;
		G.cwd = xrealloc_getcwd_or_warn((అక్షరం *)G.cwd);
		ఒకవేళ (!G.cwd)
			G.cwd = bb_సందేశం_unknown;
	}
	తిరిగి G.cwd;
}

#అయితే_ముగింపు /* !__U_BOOT__ */

/*
 * Shell and environment variable support
 */
స్థిర నిర్మాణం variable **get_ptr_గమ్యం_local_var(స్థిరరాశి అక్షరం *name)
{
	నిర్మాణం variable **pp;
	నిర్మాణం variable *cur;

	pp = &G.శిఖరం_var;
	అయితే ((cur = *pp) != ఖాళీ) {
		ఒకవేళ (varcmp(cur->varstr, name) == 0)
			తిరిగి pp;
		pp = &cur->తదుపరి;
	}
	తిరిగి ఖాళీ;
}

స్థిర స్థిరరాశి అక్షరం* FAST_FUNC get_local_var_విలువ(స్థిరరాశి అక్షరం *name)
{
	నిర్మాణం variable **vpp;

	ఒకవేళ (G.expanded_assignments) {
		అక్షరం **cpp = G.expanded_assignments;
		అయితే (*cpp) {
			అక్షరం *cp = *cpp;
			ఒకవేళ (varcmp(cp, name) == 0)
				తిరిగి strchr(cp, '=') + 1;
			cpp++;
		}
	}

	vpp = get_ptr_గమ్యం_local_var(name);
	ఒకవేళ (vpp)
		తిరిగి strchr((*vpp)->varstr, '=') + 1;

#నిర్వచితం_కాకపోతే __U_BOOT__
	ఒకవేళ (strcmp(name, "PPID") == 0)
		తిరిగి utoa(G.మూలం_ppid);
#అయితే_ముగింపు /* !__U_BOOT__ */
	// bash compat: UID? EUID?
#ఒకవేళ ENABLE_HUSH_RANDOM_SUPPORT
	ఒకవేళ (strcmp(name, "RANDOM") == 0)
		తిరిగి utoa(తదుపరి_random(&G.random_gen));
#అయితే_ముగింపు
#ఒకవేళ ENABLE_HUSH_LINENO_VAR
	ఒకవేళ (strcmp(name, "LINENO") == 0)
		తిరిగి utoa(G.execute_lineno);
#అయితే_ముగింపు
#ఒకవేళ BASH_EPOCH_VARS
	{
		స్థిరరాశి అక్షరం *fmt = ఖాళీ;
		ఒకవేళ (strcmp(name, "EPOCHSECONDS") == 0)
			fmt = "%llu";
		లేకపోతే ఒకవేళ (strcmp(name, "EPOCHREALTIME") == 0)
			fmt = "%llu.%06u";
		ఒకవేళ (fmt) {
			నిర్మాణం timeval tv;
			xgettimeofday(&tv);
			sprintf(G.epoch_buf, fmt, (గుర్తులేని దీర్ఘ దీర్ఘ)tv.tv_sec,
					(గుర్తులేని)tv.tv_usec);
			తిరిగి G.epoch_buf;
		}
	}
#అయితే_ముగింపు
	తిరిగి ఖాళీ;
}

#నిర్వచితం_కాకపోతే __U_BOOT__
#ఒకవేళ ENABLE_HUSH_GETOPTS
స్థిర శూన్య handle_మారింది_special_పేర్లు(స్థిరరాశి అక్షరం *name)
{
	ఒకవేళ (varcmp(name, "OPTIND") == 0) {
		G.getopt_లెక్క = 0;
		తిరిగి;
	}
}
#లేకపోతే
/* Do not even bother evaluating arguments */
# define handle_మారింది_special_పేర్లు(...) ((శూన్య)0)
#అయితే_ముగింపు
#లేకపోతే /* __U_BOOT__ */
/* Do not even bother evaluating arguments */
# define handle_మారింది_special_పేర్లు(...) ((శూన్య)0)
#అయితే_ముగింపు /* __U_BOOT__ */

/* str holds "NAME=VAL" and is expected to be malloced.
 * We take ownership of it.
 */
#నిర్వచితం_కాకపోతే __U_BOOT__
#నిర్వచించు SETFLAG_EXPORT   (1 << 0)
#నిర్వచించు SETFLAG_UNEXPORT (1 << 1)
#నిర్వచించు SETFLAG_MAKE_RO  (1 << 2)
#అయితే_ముగింపు /* !__U_BOOT__ */
#నిర్వచించు SETFLAG_VARLVL_జరుపు   3
#నిర్వచితం_కాకపోతే __U_BOOT__
స్థిర పూర్ణాంకం సమితి_local_var(అక్షరం *str, గుర్తులేని flags)
#లేకపోతే /* __U_BOOT__ */
పూర్ణాంకం సమితి_local_var_modern(అక్షరం *str, పూర్ణాంకం flags)
#అయితే_ముగింపు /* __U_BOOT__ */
{
	నిర్మాణం variable **cur_pp;
	నిర్మాణం variable *cur;
	అక్షరం *free_me = ఖాళీ;
	అక్షరం *eq_sign;
	పూర్ణాంకం name_len;
	పూర్ణాంకం retval;
#నిర్వచితం_కాకపోతే __U_BOOT__
	గుర్తులేని local_lvl = (flags >> SETFLAG_VARLVL_జరుపు);
#అయితే_ముగింపు /* !__U_BOOT__ */

	eq_sign = strchr(str, '=');
	ఒకవేళ (HUSH_DEBUG && !eq_sign)
		bb_simple_error_సందేశం_and_die("BUG in setvar");

	name_len = eq_sign - str + 1; /* including '=' */
	cur_pp = &G.శిఖరం_var;
	అయితే ((cur = *cur_pp) != ఖాళీ) {
		ఒకవేళ (అక్షరశ్రేణి_పోలిక(cur->varstr, str, name_len) != 0) {
			cur_pp = &cur->తదుపరి;
			కొనసాగు;
		}

#నిర్వచితం_కాకపోతే __U_BOOT__
		/* We found an existing var with this name */
		ఒకవేళ (cur->flg_read_only) {
			bb_error_సందేశం("%s: readonly variable", str);
			విడుదల(str);
//NOTE: in bash, assignment in "export READONLY_VAR=Z" fails, and sets $?=1,
//but export per se succeeds (does put the var in env). We don't mimic that.
			తిరిగి -1;
		}
		ఒకవేళ (flags & SETFLAG_UNEXPORT) { // && cur->flg_export ?
			debug_printf_env("%s: unsetenv '%s'\n", __func__, str);
			*eq_sign = '\0';
			unsetenv(str);
			*eq_sign = '=';
		}
		ఒకవేళ (cur->var_nest_level < local_lvl) {
			/* bash 3.2.33(1) and exported vars:
			 * # export z=z
			 * # f() { local z=a; env | grep ^z; }
			 * # f
			 * z=a
			 * # env | grep ^z
			 * z=z
			 */
			ఒకవేళ (cur->flg_export)
				flags |= SETFLAG_EXPORT;
			/* New variable is local ("local VAR=VAL" or
			 * "VAR=VAL cmd")
			 * and existing one is global, or local
			 * on a lower level that new one.
			 * Remove it from global variable list:
			 */
			*cur_pp = cur->తదుపరి;
			ఒకవేళ (G.shadowed_vars_pp) {
				/* Save in "shadowed" list */
				debug_printf_env("shadowing %s'%s'/%u by '%s'/%u\n",
					cur->flg_export ? "exported " : "",
					cur->varstr, cur->var_nest_level, str, local_lvl
				);
				cur->తదుపరి = *G.shadowed_vars_pp;
				*G.shadowed_vars_pp = cur;
			} లేకపోతే {
				/* Came from pseudo_exec_argv(), no need to save: delete it */
				debug_printf_env("shadow-deleting %s'%s'/%u by '%s'/%u\n",
					cur->flg_export ? "exported " : "",
					cur->varstr, cur->var_nest_level, str, local_lvl
				);
				ఒకవేళ (cur->max_len == 0) /* allocated "VAR=VAL"? */
					free_me = cur->varstr; /* then free it later */
				విడుదల(cur);
			}
			ఆపు;
		}
#అయితే_ముగింపు /* !__U_BOOT__ */

		ఒకవేళ (strcmp(cur->varstr + name_len, eq_sign + 1) == 0) {
			debug_printf_env("assignement '%s' does not change anything\n", str);
 free_and_exp:
			విడుదల(str);
			వెళ్ళు ఘాతాంకం;
		}

		/* Replace the value in the found "struct variable" */
		ఒకవేళ (cur->max_len != 0) {
			ఒకవేళ (cur->max_len >= strnlen(str, cur->max_len + 1)) {
				/* This one is from startup env, reuse space */
				debug_printf_env("reusing startup env for '%s'\n", str);
				strcpy(cur->varstr, str);
				వెళ్ళు free_and_exp;
			}
			/* Can't reuse */
			cur->max_len = 0;
			వెళ్ళు సమితి_str_and_exp;
		}
		/* max_len == 0 signifies "malloced" var, which we can
		 * (and have to) free. But we can't free(cur->varstr) here:
		 * if cur->flg_export is 1, it is in the environment.
		 * We should either unsetenv+free, or wait until putenv,
		 * then putenv(new)+free(old).
		 */
		free_me = cur->varstr;
		వెళ్ళు సమితి_str_and_exp;
	}

	/* Not found or shadowed - create new variable struct */
#నిర్వచితం_కాకపోతే __U_BOOT__
	debug_printf_env("%s: alloc new var '%s'/%u\n", __func__, str, local_lvl);
#లేకపోతే /* __U_BOOT__ */
	debug_printf_env("%s: alloc new var '%s'\n", __func__, str);
#అయితే_ముగింపు /* __U_BOOT__ */
	cur = xzalloc(పరిమాణం(*cur));
#నిర్వచితం_కాకపోతే __U_BOOT__
	cur->var_nest_level = local_lvl;
#అయితే_ముగింపు /* !__U_BOOT__ */
	cur->తదుపరి = *cur_pp;
	*cur_pp = cur;

 సమితి_str_and_exp:
	cur->varstr = str;
 ఘాతాంకం:
#నిర్వచితం_కాకపోతే __U_BOOT__
#ఒకవేళ !BB_MMU || ENABLE_HUSH_READONLY
	ఒకవేళ (flags & SETFLAG_MAKE_RO) {
		cur->flg_read_only = 1;
	}
#అయితే_ముగింపు
	ఒకవేళ (flags & SETFLAG_EXPORT)
		cur->flg_export = 1;
#అయితే_ముగింపు /* !__U_BOOT__ */
	retval = 0;
#నిర్వచితం_కాకపోతే __U_BOOT__
	ఒకవేళ (cur->flg_export) {
		ఒకవేళ (flags & SETFLAG_UNEXPORT) {
			cur->flg_export = 0;
			/* unsetenv was already done */
		} లేకపోతే {
			debug_printf_env("%s: putenv '%s'/%u\n", __func__, cur->varstr, cur->var_nest_level);
			retval = putenv(cur->varstr);
			/* fall through to "free(free_me)" -
			 * only now we can free old exported malloced string
			 */
		}
	}
#అయితే_ముగింపు /* !__U_BOOT__ */
	విడుదల(free_me);

	handle_మారింది_special_పేర్లు(cur->varstr);

	తిరిగి retval;
}

#నిర్వచితం_కాకపోతే __U_BOOT__
స్థిర పూర్ణాంకం సమితి_local_var0(అక్షరం *str)
{
	తిరిగి సమితి_local_var(str, 0);
}

స్థిర శూన్య FAST_FUNC సమితి_local_var_from_halves(స్థిరరాశి అక్షరం *name, స్థిరరాశి అక్షరం *val)
{
	అక్షరం *var = xasprintf("%s=%s", name, val);
	సమితి_local_var0(var);
}

/* Used at startup and after each cd */
స్థిర శూన్య సమితి_pwd_var(గుర్తులేని flag)
{
	సమితి_local_var(xasprintf("PWD=%s", get_cwd(/*force:*/ 1)), flag);
}
#అయితే_ముగింపు /* !__U_BOOT__ */

#ఒకవేళ ENABLE_HUSH_UNSET || ENABLE_HUSH_GETOPTS
స్థిర పూర్ణాంకం unset_local_var(స్థిరరాశి అక్షరం *name)
{
	నిర్మాణం variable *cur;
	నిర్మాణం variable **cur_pp;

	cur_pp = &G.శిఖరం_var;
	అయితే ((cur = *cur_pp) != ఖాళీ) {
		ఒకవేళ (varcmp(cur->varstr, name) == 0) {
			ఒకవేళ (cur->flg_read_only) {
				bb_error_సందేశం("%s: readonly variable", name);
				తిరిగి EXIT_FAILURE;
			}

			*cur_pp = cur->తదుపరి;
			debug_printf_env("%s: unsetenv '%s'\n", __func__, cur->varstr);
			bb_unsetenv(cur->varstr);
			ఒకవేళ (!cur->max_len)
				విడుదల(cur->varstr);
			విడుదల(cur);

			ఆపు;
		}
		cur_pp = &cur->తదుపరి;
	}

	/* Handle "unset LINENO" et al even if did not find the variable to unset */
	handle_మారింది_special_పేర్లు(name);

	తిరిగి EXIT_SUCCESS;
}
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే __U_BOOT__
/*
 * Helpers for "var1=val1 var2=val2 cmd" feature
 */
స్థిర శూన్య కూడు_vars(నిర్మాణం variable *var)
{
	నిర్మాణం variable *తదుపరి;

	అయితే (var) {
		తదుపరి = var->తదుపరి;
		var->తదుపరి = G.శిఖరం_var;
		G.శిఖరం_var = var;
		ఒకవేళ (var->flg_export) {
			debug_printf_env("%s: restoring exported '%s'/%u\n", __func__, var->varstr, var->var_nest_level);
			putenv(var->varstr);
		} లేకపోతే {
			debug_printf_env("%s: restoring variable '%s'/%u\n", __func__, var->varstr, var->var_nest_level);
		}
		var = తదుపరి;
	}
}

/* We put strings[i] into variable table and possibly putenv them.
 * If variable is read only, we can free the strings[i]
 * which attempts to overwrite it.
 * The strings[] vector itself is freed.
 */
స్థిర శూన్య సమితి_vars_and_save_old(అక్షరం **అక్షరశ్రేణులు)
{
	అక్షరం **s;

	ఒకవేళ (!అక్షరశ్రేణులు)
		తిరిగి;

	s = అక్షరశ్రేణులు;
	అయితే (*s) {
		నిర్మాణం variable *var_p;
		నిర్మాణం variable **var_pp;
		అక్షరం *eq;

		eq = strchr(*s, '=');
		ఒకవేళ (HUSH_DEBUG && !eq)
			bb_simple_error_సందేశం_and_die("BUG in varexp4");
		var_pp = get_ptr_గమ్యం_local_var(*s);
		ఒకవేళ (var_pp) {
			var_p = *var_pp;
			ఒకవేళ (var_p->flg_read_only) {
				అక్షరం **p;
				bb_error_సందేశం("%s: readonly variable", *s);
				/*
				 * "VAR=V BLTIN" unsets VARs after BLTIN completes.
				 * If VAR is readonly, leaving it in the list
				 * after asssignment error (msg above)
				 * causes doubled error message later, on unset.
				 */
				debug_printf_env("removing/freeing '%s' element\n", *s);
				విడుదల(*s);
				p = s;
				చేయి { *p = p[1]; p++; } అయితే (*p);
				వెళ్ళు తదుపరి;
			}
			/* below, set_local_var() with nest level will
			 * "shadow" (remove) this variable from
			 * global linked list.
			 */
		}
		debug_printf_env("%s: env override '%s'/%u\n", __func__, *s, G.var_nest_level);
		సమితి_local_var(*s, (G.var_nest_level << SETFLAG_VARLVL_జరుపు) | SETFLAG_EXPORT);
		s++;
 తదుపరి: ;
	}
	విడుదల(అక్షరశ్రేణులు);
}

/*
 * Unicode helper
 */
స్థిర శూన్య reinit_unicode_for_hush(శూన్య)
{
	/* Unicode support should be activated even if LANG is set
	 * _during_ shell execution, not only if it was set when
	 * shell was started. Therefore, re-check LANG every time:
	 */
	ఒకవేళ (ENABLE_FEATURE_CHECK_UNICODE_లోపల_ENV
	 || ENABLE_UNICODE_USING_LOCALE
	) {
		స్థిరరాశి అక్షరం *s = get_local_var_విలువ("LC_ALL");
		ఒకవేళ (!s) s = get_local_var_విలువ("LC_CTYPE");
		ఒకవేళ (!s) s = get_local_var_విలువ("LANG");
		reinit_unicode(s);
	}
}

#అయితే_ముగింపు /* !__U_BOOT__ */
/*
 * in_str support (strings, and "strings" read from files).
 */

#ఒకవేళ ENABLE_HUSH_INTERACTIVE
#నిర్వచితం_కాకపోతే __U_BOOT__
/* To test correct lineedit/interactive behavior, type from command line:
 *	echo $P\
 *	\
 *	AT\
 *	H\
 *	\
 * It exercises a lot of corner cases.
 */
స్థిర స్థిరరాశి అక్షరం *అమరిక_prompt_string(శూన్య)
{
	స్థిరరాశి అక్షరం *prompt_str;

	debug_printf_prompt("%s promptmode:%d\n", __func__, G.promptmode);

# ఒకవేళ ENABLE_FEATURE_EDITING_FANCY_PROMPT
	prompt_str = get_local_var_విలువ(G.promptmode == 0 ? "PS1" : "PS2");
	ఒకవేళ (!prompt_str)
		prompt_str = "";
# లేకపోతే
	prompt_str = "> "; /* if PS2, else... */
	ఒకవేళ (G.promptmode == 0) { /* PS1 */
		/* No fancy prompts supported, (re)generate "CURDIR $ " by hand */
		విడుదల(G.PS1);
		/* bash uses $PWD value, even if it is set by user.
		 * It uses current dir only if PWD is unset.
		 * We always use current dir. */
		prompt_str = G.PS1 = xasprintf("%s %c ", get_cwd(0), (geteuid() != 0) ? '$' : '#');
	}
# endif
	debug_printf("prompt_str '%s'\n", prompt_str);
	తిరిగి prompt_str;
}
#అయితే_ముగింపు /* !__U_BOOT__ */

#నిర్వచితం_కాకపోతే __U_BOOT__
స్థిర పూర్ణాంకం get_user_ఆదానం(నిర్మాణం లోపల_str *i)
#లేకపోతే /* __U_BOOT__ */
స్థిర శూన్య get_user_ఆదానం(నిర్మాణం లోపల_str *i)
#అయితే_ముగింపు /* __U_BOOT__ */
{
#నిర్వచితం_కాకపోతే __U_BOOT__
# ఒకవేళ ENABLE_FEATURE_EDITING
	/* In EDITING case, this function reads next input line,
	 * saves it in i->p, then returns 1st char of it.
	 */
	పూర్ణాంకం r;
	స్థిరరాశి అక్షరం *prompt_str;

	prompt_str = అమరిక_prompt_string();
	కొరకు (;;) {
		reinit_unicode_for_hush();
		G.flag_SIGINT = 0;

		bb_got_signal = 0;
		ఒకవేళ (!sigisemptyset(&G.pending_సమితి)) {
			/* Whoops, already got a signal, do not call read_line_input */
			bb_got_signal = r = -1;
		} లేకపోతే {
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
			r = read_పంక్తి_ఆదానం(G.పంక్తి_ఆదానం_state, prompt_str,
				G.user_ఆదానం_buf, CONFIG_FEATURE_EDITING_MAX_LEN-1
			);
			/* read_line_input intercepts ^C, "convert" it to SIGINT */
			ఒకవేళ (r == 0)
				లేపు(SIGINT);
		}
		/* bash prints ^C (before running a trap, if any)
		 * both on keyboard ^C and on real SIGINT (non-kbd generated).
		 */
		ఒకవేళ (sigismember(&G.pending_సమితి, SIGINT)) {
			లిఖ(STDOUT_FILENO, "^C\n", 3);
			G.చివరి_exitcode = 128 | SIGINT;
		}
		check_and_నిరంతరం_traps();
		ఒకవేళ (r == 0) /* keyboard ^C? */
			కొనసాగు; /* go back, read another input line */
		ఒకవేళ (r > 0) /* normal input? (no ^C, no ^D, no signals) */
			ఆపు;
		ఒకవేళ (!bb_got_signal) {
			/* r < 0: ^D/EOF/error detected (but not signal) */
			/* ^D on interactive input goes to next line before exiting: */
			లిఖ(STDOUT_FILENO, "\n", 1);
			i->p = ఖాళీ;
			i->peek_buf[0] = r = EOF;
			తిరిగి r;
		}
		/* it was a signal: go back, read another input line */
	}
	i->p = G.user_ఆదానం_buf;
	తిరిగి (గుర్తులేని అక్షరం)*i->p++;
# లేకపోతే
	/* In !EDITING case, this function gets called for every char.
	 * Buffering happens deeper in the call chain, in hfgetc(i->file).
	 */
	పూర్ణాంకం r;

	కొరకు (;;) {
		G.flag_SIGINT = 0;
		ఒకవేళ (i->చివరి_char == '\0' || i->చివరి_char == '\n') {
			స్థిరరాశి అక్షరం *prompt_str = అమరిక_prompt_string();
			/* Why check_and_run_traps here? Try this interactively:
			 * $ trap 'echo INT' INT; (sleep 2; kill -INT $$) &
			 * $ <[enter], repeatedly...>
			 * Without check_and_run_traps, handler never runs.
			 */
			check_and_నిరంతరం_traps();
			fputs_stdout(prompt_str);
			fflush_all();
		}
		r = hfgetc(i->file);
		/* In !ENABLE_FEATURE_EDITING we don't use read_line_input,
		 * no ^C masking happens during fgetc, no special code for ^C:
		 * it generates SIGINT as usual.
		 */
		check_and_నిరంతరం_traps();
		ఒకవేళ (r != '\0' && !G.flag_SIGINT)
			ఆపు;
		ఒకవేళ (G.flag_SIGINT) {
			/* ^C or SIGINT: repeat */
			/* bash prints ^C even on real SIGINT (non-kbd generated) */
			/* kernel prints "^C" itself, just print newline: */
			లిఖ(STDOUT_FILENO, "\n", 1);
			G.చివరి_exitcode = 128 | SIGINT;
		}
	}
	తిరిగి r;
# endif
#లేకపోతే /* __U_BOOT__ */
	పూర్ణాంకం n;
	పూర్ణాంకం promptme;
	స్థిర అక్షరం the_command[CONFIG_SYS_CBSIZE + 1];

	bootretry_reset_cmd_timeout();
	promptme = 1;
	n = u_boot_cli_readline(i);

# ifdef CONFIG_BOOT_RETRY_సమయం
	ఒకవేళ (n == -2) {
		puts("\nTimeout waiting for command\n");
#  ifdef CONFIG_RESET_గమ్యం_RETRY
		do_reset(ఖాళీ, 0, 0, ఖాళీ);
#  elif IS_ENABLED(CONFIG_RETRY_BOOTCMD)
	strcpy(console_బఫరు, "run bootcmd\n");
# లేకపోతే
#	error "This only works with CONFIG_RESET_TO_RETRY or CONFIG_BOOT_RETRY_COMMAND enabled"
#  endif
	}
# endif
	ఒకవేళ (n == -1 ) {
		G.flag_repeat = 0;
		promptme = 0;
	}
	n = అక్షరశ్రేణి_పొడవు(console_బఫరు);
	console_బఫరు[n] = '\n';
	console_బఫరు[n+1]= '\0';
	ఒకవేళ (had_ctrlc())
		G.flag_repeat = 0;
	clear_ctrlc();
	G.do_repeat = 0;
#నిర్వచితం_కాకపోతే __U_BOOT__
	ఒకవేళ (G.promptmode == 1) {
#లేకపోతే /* __U_BOOT__ */
	ఒకవేళ (!G.promptmode) {
#అయితే_ముగింపు /* __U_BOOT__ */
		ఒకవేళ (console_బఫరు[0] == '\n'&& G.flag_repeat == 0) {
			strcpy(the_command, console_బఫరు);
		}
		లేకపోతే {
			ఒకవేళ (console_బఫరు[0] != '\n') {
				strcpy(the_command, console_బఫరు);
				G.flag_repeat = 1;
			}
			లేకపోతే {
				G.do_repeat = 1;
			}
		}
		i->p = the_command;
	}
	లేకపోతే {
		ఒకవేళ (console_బఫరు[0] != '\n') {
			ఒకవేళ (అక్షరశ్రేణి_పొడవు(the_command) + అక్షరశ్రేణి_పొడవు(console_బఫరు)
				< CONFIG_SYS_CBSIZE) {
				n = అక్షరశ్రేణి_పొడవు(the_command);
#నిర్వచితమైతే __U_BOOT__
				/*
				 * To avoid writing to bad places, we check if
				 * n is greater than 0.
				 * This bug was found by Harald Seiler.
				 */
				ఒకవేళ (n > 0)
					the_command[n-1] = ' ';
				strcpy(&the_command[n], console_బఫరు);
#లేకపోతే /* !__U_BOOT__ */
			the_command[n-1] = ' ';
			strcpy(&the_command[n], console_బఫరు);
#అయితే_ముగింపు /* !__U_BOOT__ */
				}
				లేకపోతే {
					the_command[0] = '\n';
					the_command[1] = '\0';
					G.flag_repeat = 0;
				}
		}
		ఒకవేళ (promptme == 0) {
			the_command[0] = '\n';
			the_command[1] = '\0';
		}
		i->p = console_బఫరు;
	}
#అయితే_ముగింపు /* __U_BOOT__ */
}
/* This is the magic location that prints prompts
 * and gets data back from the user */
స్థిర పూర్ణాంకం fgetc_interactive(నిర్మాణం లోపల_str *i)
{
	పూర్ణాంకం ch;
#నిర్వచితం_కాకపోతే __U_BOOT__
	/* If it's interactive stdin, get new line. */
	ఒకవేళ (G_interactive_fd && i->file == G.HFILE_stdin) {
#అయితే_ముగింపు /* !__U_BOOT__ */
#నిర్వచితం_కాకపోతే __U_BOOT__
		/* Returns first char (or EOF), the rest is in i->p[] */
		ch = get_user_ఆదానం(i);
#లేకపోతే /* __U_BOOT__ */
		/* Avoid garbage value and make clang happy. */
		ch = 0;
		/*
		 * get_user_input() does not return anything when used in
		 * U-Boot.
		 * So, we need to take the read character from i->p[].
		 */
		get_user_ఆదానం(i);
		ఒకవేళ (i->p && *i->p) {
			ch = *i->p++;
		}
#అయితే_ముగింపు /* __U_BOOT__ */
		G.promptmode = 1; /* PS2 */
		debug_printf_prompt("%s promptmode=%d\n", __func__, G.promptmode);
#నిర్వచితం_కాకపోతే __U_BOOT__
	} లేకపోతే {
		/* Not stdin: script file, sourced file, etc */
		చేయి ch = hfgetc(i->file); అయితే (ch == '\0');
	}
#అయితే_ముగింపు /* !__U_BOOT__ */
	తిరిగి ch;
}
#లేకపోతే  /* !INTERACTIVE */
#నిర్వచితం_కాకపోతే __U_BOOT__
స్థిర ALWAYS_INLINE పూర్ణాంకం fgetc_interactive(నిర్మాణం లోపల_str *i)
{
	పూర్ణాంకం ch;
	చేయి ch = hfgetc(i->file); అయితే (ch == '\0');
	తిరిగి ch;
}
#అయితే_ముగింపు /* !__U_BOOT__ */
#అయితే_ముగింపు  /* !INTERACTIVE */

స్థిర పూర్ణాంకం i_getch(నిర్మాణం లోపల_str *i)
{
	పూర్ణాంకం ch;

#నిర్వచితం_కాకపోతే __U_BOOT__
	ఒకవేళ (!i->file) {
		/* string-based in_str */
		ch = (గుర్తులేని అక్షరం)*i->p;
		ఒకవేళ (ch != '\0') {
			i->p++;
			i->చివరి_char = ch;
#ఒకవేళ ENABLE_HUSH_LINENO_VAR
			ఒకవేళ (ch == '\n') {
				G.parse_lineno++;
				debug_printf_parse("G.parse_lineno++ = %u\n", G.parse_lineno);
			}
#అయితే_ముగింపు
			తిరిగి ch;
		}
		తిరిగి EOF;
	}

#అయితే_ముగింపు /* !__U_BOOT__ */
	/* FILE-based in_str */

#ఒకవేళ ENABLE_FEATURE_EDITING
	/* This can be stdin, check line editing char[] buffer */
	ఒకవేళ (i->p && *i->p != '\0') {
		ch = (గుర్తులేని అక్షరం)*i->p++;
		వెళ్ళు బహిర్గతం;
#నిర్వచితం_కాకపోతే __U_BOOT__
	}
#లేకపోతే /* __U_BOOT__ */
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
	} లేకపోతే ఒకవేళ (i->p && *i->p == '\0' && G.నిరంతరం_command_flags){
		తిరిగి EOF;
	}
#అయితే_ముగింపు /* __U_BOOT__ */
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే __U_BOOT__
	/* peek_buf[] is an int array, not char. Can contain EOF. */
	ch = i->peek_buf[0];
	ఒకవేళ (ch != 0) {
		పూర్ణాంకం ch2 = i->peek_buf[1];
		i->peek_buf[0] = ch2;
		ఒకవేళ (ch2 == 0) /* very likely, avoid redundant write */
			వెళ్ళు బహిర్గతం;
		i->peek_buf[1] = 0;
		వెళ్ళు బహిర్గతం;
	}

#అయితే_ముగింపు /* !__U_BOOT__ */
	ch = fgetc_interactive(i);
 బహిర్గతం:
	debug_printf("file_get: got '%c' %d\n", ch, ch);
	i->చివరి_char = ch;
#ఒకవేళ ENABLE_HUSH_LINENO_VAR
	ఒకవేళ (ch == '\n') {
		G.parse_lineno++;
		debug_printf_parse("G.parse_lineno++ = %u\n", G.parse_lineno);
	}
#అయితే_ముగింపు
	తిరిగి ch;
}

స్థిర పూర్ణాంకం i_peek(నిర్మాణం లోపల_str *i)
{
#నిర్వచితం_కాకపోతే __U_BOOT__
	పూర్ణాంకం ch;

	ఒకవేళ (!i->file) {
		/* string-based in_str */
		/* Doesn't report EOF on NUL. None of the callers care. */
		తిరిగి (గుర్తులేని అక్షరం)*i->p;
	}

	/* FILE-based in_str */

#ఒకవేళ ENABLE_FEATURE_EDITING && ENABLE_HUSH_INTERACTIVE
	/* This can be stdin, check line editing char[] buffer */
	ఒకవేళ (i->p && *i->p != '\0')
		తిరిగి (గుర్తులేని అక్షరం)*i->p;
#అయితే_ముగింపు
	/* peek_buf[] is an int array, not char. Can contain EOF. */
	ch = i->peek_buf[0];
	ఒకవేళ (ch != 0)
		తిరిగి ch;

	/* Need to get a new char */
	ch = fgetc_interactive(i);
	debug_printf("file_peek: got '%c' %d\n", ch, ch);

	/* Save it by either rolling back line editing buffer, or in i->peek_buf[0] */
#ఒకవేళ ENABLE_FEATURE_EDITING && ENABLE_HUSH_INTERACTIVE
	ఒకవేళ (i->p) {
		i->p -= 1;
		తిరిగి ch;
	}
#అయితే_ముగింపు
	i->peek_buf[0] = ch;
	/*i->peek_buf[1] = 0; - already is */
	తిరిగి ch;
#లేకపోతే /* __U_BOOT__ */
	/* string-based in_str */
	/* Doesn't report EOF on NUL. None of the callers care. */
	తిరిగి (గుర్తులేని అక్షరం)*i->p;
#అయితే_ముగింపు /* __U_BOOT__ */
}

/* Only ever called if i_peek() was called, and did not return EOF.
 * IOW: we know the previous peek saw an ordinary char, not EOF, not NUL,
 * not end-of-line. Therefore we never need to read a new editing line here.
 */
స్థిర పూర్ణాంకం i_peek2(నిర్మాణం లోపల_str *i)
{
#నిర్వచితం_కాకపోతే __U_BOOT__
	పూర్ణాంకం ch;
#అయితే_ముగింపు /* !__U_BOOT__ */

	/* There are two cases when i->p[] buffer exists.
	 * (1) it's a string in_str.
	 * (2) It's a file, and we have a saved line editing buffer.
	 * In both cases, we know that i->p[0] exists and not NUL, and
	 * the peek2 result is in i->p[1].
	 */
	ఒకవేళ (i->p)
		తిరిగి (గుర్తులేని అక్షరం)i->p[1];

#నిర్వచితం_కాకపోతే __U_BOOT__
	/* Now we know it is a file-based in_str. */

	/* peek_buf[] is an int array, not char. Can contain EOF. */
	/* Is there 2nd char? */
	ch = i->peek_buf[1];
	ఒకవేళ (ch == 0) {
		/* We did not read it yet, get it now */
		చేయి ch = hfgetc(i->file); అయితే (ch == '\0');
		i->peek_buf[1] = ch;
	}

	debug_printf("file_peek2: got '%c' %d\n", ch, ch);
	తిరిగి ch;
#లేకపోతే
	తిరిగి 0;
#అయితే_ముగింపు /* __U_BOOT__ */
}

స్థిర పూర్ణాంకం i_getch_and_eat_bkslash_nl(నిర్మాణం లోపల_str *ఆదానం)
{
	కొరకు (;;) {
		పూర్ణాంకం ch, ch2;

		ch = i_getch(ఆదానం);
		ఒకవేళ (ch != '\\')
			తిరిగి ch;
		ch2 = i_peek(ఆదానం);
		ఒకవేళ (ch2 != '\n')
			తిరిగి ch;
		/* backslash+newline, skip it */
		i_getch(ఆదానం);
	}
}

/* Note: this function _eats_ \<newline> pairs, safe to use plain
 * i_getch() after it instead of i_getch_and_eat_bkslash_nl().
 */
స్థిర పూర్ణాంకం i_peek_and_eat_bkslash_nl(నిర్మాణం లోపల_str *ఆదానం)
{
	కొరకు (;;) {
		పూర్ణాంకం ch, ch2;

		ch = i_peek(ఆదానం);
		ఒకవేళ (ch != '\\')
			తిరిగి ch;
		ch2 = i_peek2(ఆదానం);
		ఒకవేళ (ch2 != '\n')
			తిరిగి ch;
		/* backslash+newline, skip it */
		i_getch(ఆదానం);
		i_getch(ఆదానం);
	}
}

#నిర్వచితం_కాకపోతే __U_BOOT__
స్థిర శూన్య అమరిక_file_లోపల_str(నిర్మాణం లోపల_str *i, HFILE *fp)
#లేకపోతే /* __U_BOOT__ */
స్థిర శూన్య అమరిక_file_లోపల_str(నిర్మాణం లోపల_str *i)
#అయితే_ముగింపు /* __U_BOOT__ */
{
	memset(i, 0, పరిమాణం(*i));
#నిర్వచితం_కాకపోతే __U_BOOT__
	i->file = fp;
	/* i->p = NULL; */
#అయితే_ముగింపు /* !__U_BOOT__ */
}

స్థిర శూన్య అమరిక_string_లోపల_str(నిర్మాణం లోపల_str *i, స్థిరరాశి అక్షరం *s)
{
	memset(i, 0, పరిమాణం(*i));
	/*i->file = NULL */;
	i->p = s;
}

/*
 * o_string support
 */
#నిర్వచించు B_CHUNK  (32 * పరిమాణం(అక్షరం*))

స్థిర శూన్య o_reset_గమ్యం_ఖాళీలు_unquoted(o_string *o)
{
	o->నిడివి = 0;
	o->has_quoted_part = 0;
	ఒకవేళ (o->దత్తాంశం)
		o->దత్తాంశం[0] = '\0';
}

స్థిర శూన్య o_free_and_సమితి_NULL(o_string *o)
{
	విడుదల(o->దత్తాంశం);
	memset(o, 0, పరిమాణం(*o));
}

స్థిర ALWAYS_INLINE శూన్య o_free(o_string *o)
{
	విడుదల(o->దత్తాంశం);
}

స్థిర శూన్య o_grow_by(o_string *o, పూర్ణాంకం len)
{
	ఒకవేళ (o->నిడివి + len > o->maxlen) {
		o->maxlen += (2 * len) | (B_CHUNK-1);
		o->దత్తాంశం = xrealloc(o->దత్తాంశం, 1 + o->maxlen);
	}
}

స్థిర శూన్య o_addchr(o_string *o, పూర్ణాంకం ch)
{
	debug_printf("o_addchr: '%c' o->length=%d o=%p\n", ch, o->నిడివి, o);
	ఒకవేళ (o->నిడివి < o->maxlen) {
		/* likely. avoid o_grow_by() call */
 కూడు:
		o->దత్తాంశం[o->నిడివి] = ch;
		o->నిడివి++;
		o->దత్తాంశం[o->నిడివి] = '\0';
		తిరిగి;
	}
	o_grow_by(o, 1);
	వెళ్ళు కూడు;
}

#ఒకవేళ 0
/* Valid only if we know o_string is not empty */
స్థిర శూన్య o_delchr(o_string *o)
{
	o->నిడివి--;
	o->దత్తాంశం[o->నిడివి] = '\0';
}
#అయితే_ముగింపు

స్థిర శూన్య o_addblock(o_string *o, స్థిరరాశి అక్షరం *str, పూర్ణాంకం len)
{
	o_grow_by(o, len);
	((అక్షరం*)mempcpy(&o->దత్తాంశం[o->నిడివి], str, len))[0] = '\0';
	o->నిడివి += len;
}

స్థిర శూన్య o_addstr(o_string *o, స్థిరరాశి అక్షరం *str)
{
	o_addblock(o, str, అక్షరశ్రేణి_పొడవు(str));
}

#నిర్వచితం_కాకపోతే __U_BOOT__
స్థిర శూన్య o_addstr_with_NUL(o_string *o, స్థిరరాశి అక్షరం *str)
{
	o_addblock(o, str, అక్షరశ్రేణి_పొడవు(str) + 1);
}
#అయితే_ముగింపు /* !__U_BOOT__ */

#ఒకవేళ !BB_MMU
స్థిర శూన్య nommu_addchr(o_string *o, పూర్ణాంకం ch)
{
	ఒకవేళ (o)
		o_addchr(o, ch);
}
#లేకపోతే
# define nommu_addchr(o, str) ((శూన్య)0)
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే __U_BOOT__
#ఒకవేళ ENABLE_HUSH_బహుళకం_X
స్థిర శూన్య x_బహుళకం_addchr(పూర్ణాంకం ch)
{
	o_addchr(&G.x_బహుళకం_buf, ch);
}
స్థిర శూన్య x_బహుళకం_addstr(స్థిరరాశి అక్షరం *str)
{
	o_addstr(&G.x_బహుళకం_buf, str);
}
స్థిర శూన్య x_బహుళకం_addblock(స్థిరరాశి అక్షరం *str, పూర్ణాంకం len)
{
	o_addblock(&G.x_బహుళకం_buf, str, len);
}
స్థిర శూన్య x_బహుళకం_ఉపసర్గ(శూన్య)
{
	పూర్ణాంకం n = G.x_బహుళకం_లోతు;
	చేయి x_బహుళకం_addchr('+'); అయితే (--n >= 0);
}
స్థిర శూన్య x_బహుళకం_flush(శూన్య)
{
	పూర్ణాంకం len = G.x_బహుళకం_buf.నిడివి;
	ఒకవేళ (len <= 0)
		తిరిగి;
	ఒకవేళ (G.x_బహుళకం_fd > 0) {
		G.x_బహుళకం_buf.దత్తాంశం[len] = '\n';
		నిండిన_write(G.x_బహుళకం_fd, G.x_బహుళకం_buf.దత్తాంశం, len + 1);
	}
	G.x_బహుళకం_buf.నిడివి = 0;
}
#అయితే_ముగింపు
#అయితే_ముగింపు /* !__U_BOOT__ */

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

#ఒకవేళ ENABLE_HUSH_BRACE_EXPANSION
# define MAYBE_BRACES "{}"
#లేకపోతే
# define MAYBE_BRACES ""
#అయితే_ముగింపు

/* My analysis of quoting semantics tells me that state information
 * is associated with a destination, not a source.
 */
స్థిర శూన్య o_addqchr(o_string *o, పూర్ణాంకం ch)
{
	పూర్ణాంకం sz = 1;
	/* '-' is included because of this case:
	 * >filename0 >filename1 >filename9; v='-'; echo filename[0"$v"9]
	 */
	అక్షరం *దొరికింది = strchr("*?[-\\" MAYBE_BRACES, ch);
	ఒకవేళ (దొరికింది)
		sz++;
	o_grow_by(o, sz);
	ఒకవేళ (దొరికింది) {
		o->దత్తాంశం[o->నిడివి] = '\\';
		o->నిడివి++;
	}
	o->దత్తాంశం[o->నిడివి] = ch;
	o->నిడివి++;
	o->దత్తాంశం[o->నిడివి] = '\0';
}

స్థిర శూన్య o_కూడు_Qchr(o_string *o, పూర్ణాంకం ch)
{
	పూర్ణాంకం sz = 1;
	ఒకవేళ ((o->o_expflags & EXP_FLAG_ESC_GLOB_CHARS)
	 && strchr("*?[-\\" MAYBE_BRACES, ch)
	) {
		sz++;
		o->దత్తాంశం[o->నిడివి] = '\\';
		o->నిడివి++;
	}
	o_grow_by(o, sz);
	o->దత్తాంశం[o->నిడివి] = ch;
	o->నిడివి++;
	o->దత్తాంశం[o->నిడివి] = '\0';
}

స్థిర శూన్య o_addqblock(o_string *o, స్థిరరాశి అక్షరం *str, పూర్ణాంకం len)
{
	అయితే (len) {
		అక్షరం ch;
		పూర్ణాంకం sz;
		పూర్ణాంకం ordinary_cnt = strcspn(str, "*?[-\\" MAYBE_BRACES);
		ఒకవేళ (ordinary_cnt > len) /* paranoia */
			ordinary_cnt = len;
		o_addblock(o, str, ordinary_cnt);
		ఒకవేళ (ordinary_cnt == len)
			తిరిగి; /* NUL is already added by o_addblock */
		str += ordinary_cnt;
		len -= ordinary_cnt + 1; /* we are processing + 1 char below */

		ch = *str++;
		sz = 1;
		ఒకవేళ (ch) { /* it is necessarily one of "*?[-\\" MAYBE_BRACES */
			sz++;
			o->దత్తాంశం[o->నిడివి] = '\\';
			o->నిడివి++;
		}
		o_grow_by(o, sz);
		o->దత్తాంశం[o->నిడివి] = ch;
		o->నిడివి++;
	}
	o->దత్తాంశం[o->నిడివి] = '\0';
}

స్థిర శూన్య o_కూడు_Qblock(o_string *o, స్థిరరాశి అక్షరం *str, పూర్ణాంకం len)
{
	ఒకవేళ (!(o->o_expflags & EXP_FLAG_ESC_GLOB_CHARS)) {
		o_addblock(o, str, len);
		తిరిగి;
	}
	o_addqblock(o, str, len);
}

స్థిర శూన్య o_కూడు_Qstr(o_string *o, స్థిరరాశి అక్షరం *str)
{
	o_కూడు_Qblock(o, str, అక్షరశ్రేణి_పొడవు(str));
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
#ఒకవేళ DEBUG_EXPAND || DEBUG_GLOB
స్థిర శూన్య debug_print_list(స్థిరరాశి అక్షరం *ఉపసర్గ, o_string *o, పూర్ణాంకం n)
{
	అక్షరం **list = (అక్షరం**)o->దత్తాంశం;
	పూర్ణాంకం string_ప్రారంభం = ((n + 0xf) & ~0xf) * పరిమాణం(list[0]);
	పూర్ణాంకం i = 0;

	indent();
	fdprintf(2, "%s: list:%p n:%d string_start:%d length:%d maxlen:%d glob:%d quoted:%d escape:%d\n",
			ఉపసర్గ, list, n, string_ప్రారంభం, o->నిడివి, o->maxlen,
			!!(o->o_expflags & EXP_FLAG_GLOB),
			o->has_quoted_part,
			!!(o->o_expflags & EXP_FLAG_ESC_GLOB_CHARS));
	అయితే (i < n) {
		indent();
		fdprintf(2, " list[%d]=%d '%s' %p\n", i, (పూర్ణాంకం)(uintptr_t)list[i],
				o->దత్తాంశం + (పూర్ణాంకం)(uintptr_t)list[i] + string_ప్రారంభం,
				o->దత్తాంశం + (పూర్ణాంకం)(uintptr_t)list[i] + string_ప్రారంభం);
		i++;
	}
	ఒకవేళ (n) {
		స్థిరరాశి అక్షరం *p = o->దత్తాంశం + (పూర్ణాంకం)(uintptr_t)list[n - 1] + string_ప్రారంభం;
		indent();
#నిర్వచితం_కాకపోతే __U_BOOT__
		fdprintf(2, " total_sz:%ld\n", (దీర్ఘ)((p + అక్షరశ్రేణి_పొడవు(p) + 1) - o->దత్తాంశం));
#లేకపోతే /* __U_BOOT__ */
		రాయి(" total_sz:%ld\n", (దీర్ఘ)((p + అక్షరశ్రేణి_పొడవు(p) + 1) - o->దత్తాంశం));
#అయితే_ముగింపు /* __U_BOOT__ */
	}
}
#లేకపోతే
# define debug_print_list(ఉపసర్గ, o, n) ((శూన్య)0)
#అయితే_ముగింపు

/* n = o_save_ptr_helper(str, n) "starts new string" by storing an index value
 * in list[n] so that it points past last stored byte so far.
 * It returns n+1. */
స్థిర పూర్ణాంకం o_save_ptr_helper(o_string *o, పూర్ణాంకం n)
{
	అక్షరం **list = (అక్షరం**)o->దత్తాంశం;
	పూర్ణాంకం string_ప్రారంభం;
	పూర్ణాంకం string_len;

	ఒకవేళ (!o->has_ఖాళీలు_స్థానం) {
		string_ప్రారంభం = ((n + 0xf) & ~0xf) * పరిమాణం(list[0]);
		string_len = o->నిడివి - string_ప్రారంభం;
		ఒకవేళ (!(n & 0xf)) { /* 0, 0x10, 0x20...? */
			debug_printf_list("list[%d]=%d string_start=%d (growing)\n", n, string_len, string_ప్రారంభం);
			/* list[n] points to string_start, make space for 16 more pointers */
			o->maxlen += 0x10 * పరిమాణం(list[0]);
			o->దత్తాంశం = xrealloc(o->దత్తాంశం, o->maxlen + 1);
			list = (అక్షరం**)o->దత్తాంశం;
			memmove(list + n + 0x10, list + n, string_len);
			/*
			 * expand_on_ifs() has a "previous argv[] ends in IFS?"
			 * check. (grep for -prev-ifs-check-).
			 * Ensure that argv[-1][last] is not garbage
			 * but zero bytes, to save index check there.
			 */
			list[n + 0x10 - 1] = 0;
			o->నిడివి += 0x10 * పరిమాణం(list[0]);
		} లేకపోతే {
			debug_printf_list("list[%d]=%d string_start=%d\n",
					n, string_len, string_ప్రారంభం);
		}
	} లేకపోతే {
		/* We have empty slot at list[n], reuse without growth */
		string_ప్రారంభం = ((n+1 + 0xf) & ~0xf) * పరిమాణం(list[0]); /* NB: n+1! */
		string_len = o->నిడివి - string_ప్రారంభం;
		debug_printf_list("list[%d]=%d string_start=%d (empty slot)\n",
				n, string_len, string_ప్రారంభం);
		o->has_ఖాళీలు_స్థానం = 0;
	}
	o->has_quoted_part = 0;
	list[n] = (అక్షరం*)(uintptr_t)string_len;
	తిరిగి n + 1;
}

/* "What was our last o_save_ptr'ed position (byte offset relative o->data)?" */
స్థిర పూర్ణాంకం o_get_చివరి_ptr(o_string *o, పూర్ణాంకం n)
{
	అక్షరం **list = (అక్షరం**)o->దత్తాంశం;
	పూర్ణాంకం string_ప్రారంభం = ((n + 0xf) & ~0xf) * పరిమాణం(list[0]);

	తిరిగి ((పూర్ణాంకం)(uintptr_t)list[n-1]) + string_ప్రారంభం;
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

#ఒకవేళ ENABLE_HUSH_BRACE_EXPANSION
/* There in a GNU extension, GLOB_BRACE, but it is not usable:
 * first, it processes even {a} (no commas), second,
 * I didn't manage to make it return strings when they don't match
 * existing files. Need to re-implement it.
 */

/* Helper */
స్థిర పూర్ణాంకం glob_needed(స్థిరరాశి అక్షరం *s)
{
	అయితే (*s) {
		ఒకవేళ (*s == '\\') {
			ఒకవేళ (!s[1])
				తిరిగి 0;
			s += 2;
			కొనసాగు;
		}
		ఒకవేళ (*s == '*' || *s == '[' || *s == '?' || *s == '{')
			తిరిగి 1;
		s++;
	}
	తిరిగి 0;
}
/* Return pointer to next closing brace or to comma */
స్థిర స్థిరరాశి అక్షరం *తదుపరి_brace_sub(స్థిరరాశి అక్షరం *cp)
{
	గుర్తులేని లోతు = 0;
	cp++;
	అయితే (*cp != '\0') {
		ఒకవేళ (*cp == '\\') {
			ఒకవేళ (*++cp == '\0')
				ఆపు;
			cp++;
			కొనసాగు;
		}
		ఒకవేళ ((*cp == '}' && లోతు-- == 0) || (*cp == ',' && లోతు == 0))
			ఆపు;
		ఒకవేళ (*cp++ == '{')
			లోతు++;
	}

	తిరిగి *cp != '\0' ? cp : ఖాళీ;
}
/* Recursive brace globber. Note: may garble pattern[]. */
స్థిర పూర్ణాంకం glob_brace(అక్షరం *నమూనా, o_string *o, పూర్ణాంకం n)
{
	అక్షరం *new_నమూనా_buf;
	స్థిరరాశి అక్షరం *begin;
	స్థిరరాశి అక్షరం *తదుపరి;
	స్థిరరాశి అక్షరం *rest;
	స్థిరరాశి అక్షరం *p;
	size_t rest_len;

	debug_printf_glob("glob_brace('%s')\n", నమూనా);

	begin = నమూనా;
	అయితే (1) {
		ఒకవేళ (*begin == '\0')
			వెళ్ళు simple_glob;
		ఒకవేళ (*begin == '{') {
			/* Find the first sub-pattern and at the same time
			 * find the rest after the closing brace */
			తదుపరి = తదుపరి_brace_sub(begin);
			ఒకవేళ (తదుపరి == ఖాళీ) {
				/* An illegal expression */
				వెళ్ళు simple_glob;
			}
			ఒకవేళ (*తదుపరి == '}') {
				/* "{abc}" with no commas - illegal
				 * brace expr, disregard and skip it */
				begin = తదుపరి + 1;
				కొనసాగు;
			}
			ఆపు;
		}
		ఒకవేళ (*begin == '\\' && begin[1] != '\0')
			begin++;
		begin++;
	}
	debug_printf_glob("begin:%s\n", begin);
	debug_printf_glob("next:%s\n", తదుపరి);

	/* Now find the end of the whole brace expression */
	rest = తదుపరి;
	అయితే (*rest != '}') {
		rest = తదుపరి_brace_sub(rest);
		ఒకవేళ (rest == ఖాళీ) {
			/* An illegal expression */
			వెళ్ళు simple_glob;
		}
		debug_printf_glob("rest:%s\n", rest);
	}
	rest_len = అక్షరశ్రేణి_పొడవు(++rest) + 1;

	/* We are sure the brace expression is well-formed */

	/* Allocate working buffer large enough for our work */
	new_నమూనా_buf = xmalloc(అక్షరశ్రేణి_పొడవు(నమూనా));

	/* We have a brace expression.  BEGIN points to the opening {,
	 * NEXT points past the terminator of the first element, and REST
	 * points past the final }.  We will accumulate result names from
	 * recursive runs for each brace alternative in the buffer using
	 * GLOB_APPEND. */

	p = begin + 1;
	అయితే (1) {
		/* Construct the new glob expression */
		జ్ఞాపకం_నకలు(
			mempcpy(
				mempcpy(new_నమూనా_buf,
					/* We know the prefix for all sub-patterns */
					నమూనా, begin - నమూనా),
				p, తదుపరి - p),
			rest, rest_len);

		/* Note: glob_brace() may garble new_pattern_buf[].
		 * That's why we re-copy prefix every time (1st memcpy above).
		 */
		n = glob_brace(new_నమూనా_buf, o, n);
		ఒకవేళ (*తదుపరి == '}') {
			/* We saw the last entry */
			ఆపు;
		}
		p = తదుపరి + 1;
		తదుపరి = తదుపరి_brace_sub(తదుపరి);
	}
	విడుదల(new_నమూనా_buf);
	తిరిగి n;

 simple_glob:
	{
		పూర్ణాంకం gr;
		glob_t globdata;

		memset(&globdata, 0, పరిమాణం(globdata));
		gr = glob(నమూనా, 0, ఖాళీ, &globdata);
		debug_printf_glob("glob('%s'):%d\n", నమూనా, gr);
		ఒకవేళ (gr != 0) {
			ఒకవేళ (gr == GLOB_NOMATCH) {
				globfree(&globdata);
				/* NB: garbles parameter */
				unbackslash(నమూనా);
				o_addstr_with_NUL(o, నమూనా);
				debug_printf_glob("glob pattern '%s' is literal\n", నమూనా);
				తిరిగి o_save_ptr_helper(o, n);
			}
			ఒకవేళ (gr == GLOB_NOSPACE)
				bb_die_జ్ఞాపకం_exhausted();
			/* GLOB_ABORTED? Only happens with GLOB_ERR flag,
			 * but we didn't specify it. Paranoia again. */
			bb_error_సందేశం_and_die("glob error %d on '%s'", gr, నమూనా);
		}
		ఒకవేళ (globdata.gl_pathv && globdata.gl_pathv[0]) {
			అక్షరం **argv = globdata.gl_pathv;
			అయితే (1) {
				o_addstr_with_NUL(o, *argv);
				n = o_save_ptr_helper(o, n);
				argv++;
				ఒకవేళ (!*argv)
					ఆపు;
			}
		}
		globfree(&globdata);
	}
	తిరిగి n;
}
/* Performs globbing on last list[],
 * saving each result as a new list[].
 */
స్థిర పూర్ణాంకం perform_glob(o_string *o, పూర్ణాంకం n)
{
	అక్షరం *నమూనా, *copy;

	debug_printf_glob("start perform_glob: n:%d o->data:%p\n", n, o->దత్తాంశం);
	ఒకవేళ (!o->దత్తాంశం)
		తిరిగి o_save_ptr_helper(o, n);
	నమూనా = o->దత్తాంశం + o_get_చివరి_ptr(o, n);
	debug_printf_glob("glob pattern '%s'\n", నమూనా);
	ఒకవేళ (!glob_needed(నమూనా)) {
		/* unbackslash last string in o in place, fix length */
		o->నిడివి = unbackslash(నమూనా) - o->దత్తాంశం;
		debug_printf_glob("glob pattern '%s' is literal\n", నమూనా);
		తిరిగి o_save_ptr_helper(o, n);
	}

	copy = xstrdup(నమూనా);
	/* "forget" pattern in o */
	o->నిడివి = నమూనా - o->దత్తాంశం;
	n = glob_brace(copy, o, n);
	విడుదల(copy);
	ఒకవేళ (DEBUG_GLOB)
		debug_print_list("perform_glob returning", o, n);
	తిరిగి n;
}

#లేకపోతే /* !HUSH_BRACE_EXPANSION */

/* Helper */
స్థిర పూర్ణాంకం glob_needed(స్థిరరాశి అక్షరం *s)
{
	అయితే (*s) {
		ఒకవేళ (*s == '\\') {
			ఒకవేళ (!s[1])
				తిరిగి 0;
			s += 2;
			కొనసాగు;
		}
		ఒకవేళ (*s == '*' || *s == '[' || *s == '?')
			తిరిగి 1;
		s++;
	}
	తిరిగి 0;
}
/* Performs globbing on last list[],
 * saving each result as a new list[].
 */
స్థిర పూర్ణాంకం perform_glob(o_string *o, పూర్ణాంకం n)
{
#నిర్వచితం_కాకపోతే __U_BOOT__
	glob_t globdata;
	పూర్ణాంకం gr;
#అయితే_ముగింపు /* __U_BOOT__ */
	అక్షరం *నమూనా;

	debug_printf_glob("start perform_glob: n:%d o->data:%p\n", n, o->దత్తాంశం);
	ఒకవేళ (!o->దత్తాంశం)
		తిరిగి o_save_ptr_helper(o, n);
	నమూనా = o->దత్తాంశం + o_get_చివరి_ptr(o, n);
	debug_printf_glob("glob pattern '%s'\n", నమూనా);
	ఒకవేళ (!glob_needed(నమూనా)) {
#నిర్వచితం_కాకపోతే __U_BOOT__
 literal:
#అయితే_ముగింపు /* __U_BOOT__ */
		/* unbackslash last string in o in place, fix length */
		o->నిడివి = unbackslash(నమూనా) - o->దత్తాంశం;
		debug_printf_glob("glob pattern '%s' is literal\n", నమూనా);
		తిరిగి o_save_ptr_helper(o, n);
	}

#నిర్వచితం_కాకపోతే __U_BOOT__
	memset(&globdata, 0, పరిమాణం(globdata));
	/* Can't use GLOB_NOCHECK: it does not unescape the string.
	 * If we glob "*.\*" and don't find anything, we need
	 * to fall back to using literal "*.*", but GLOB_NOCHECK
	 * will return "*.\*"!
	 */
	gr = glob(నమూనా, 0, ఖాళీ, &globdata);
	debug_printf_glob("glob('%s'):%d\n", నమూనా, gr);
	ఒకవేళ (gr != 0) {
		ఒకవేళ (gr == GLOB_NOMATCH) {
			globfree(&globdata);
			వెళ్ళు literal;
		}
		ఒకవేళ (gr == GLOB_NOSPACE)
			bb_die_జ్ఞాపకం_exhausted();
		/* GLOB_ABORTED? Only happens with GLOB_ERR flag,
		 * but we didn't specify it. Paranoia again. */
		bb_error_సందేశం_and_die("glob error %d on '%s'", gr, నమూనా);
	}
	ఒకవేళ (globdata.gl_pathv && globdata.gl_pathv[0]) {
		అక్షరం **argv = globdata.gl_pathv;
		/* "forget" pattern in o */
		o->నిడివి = నమూనా - o->దత్తాంశం;
		అయితే (1) {
			o_addstr_with_NUL(o, *argv);
			n = o_save_ptr_helper(o, n);
			argv++;
			ఒకవేళ (!*argv)
				ఆపు;
		}
	}
	globfree(&globdata);
	ఒకవేళ (DEBUG_GLOB)
		debug_print_list("perform_glob returning", o, n);
	తిరిగి n;
#లేకపోతే /* __U_BOOT__ */
	/*
	 * NOTE We only use perform glob to call unbackslash to remove backslash
	 * from string once expanded.
	 * So, it seems OK to return this if no previous return was done.
	 */
	తిరిగి o_save_ptr_helper(o, n);
#అయితే_ముగింపు /* __U_BOOT__ */
}

#అయితే_ముగింపు /* !HUSH_BRACE_EXPANSION */

/* If o->o_expflags & EXP_FLAG_GLOB, glob the string so far remembered.
 * Otherwise, just finish current list[] and start new */
స్థిర పూర్ణాంకం o_save_ptr(o_string *o, పూర్ణాంకం n)
{
	ఒకవేళ (o->o_expflags & EXP_FLAG_GLOB) {
		/* If o->has_empty_slot, list[n] was already globbed
		 * (if it was requested back then when it was filled)
		 * so don't do that again! */
		ఒకవేళ (!o->has_ఖాళీలు_స్థానం)
			తిరిగి perform_glob(o, n); /* o_save_ptr_helper is inside */
	}
	తిరిగి o_save_ptr_helper(o, n);
}

/* "Please convert list[n] to real char* ptrs, and NULL terminate it." */
స్థిర అక్షరం **o_finalize_list(o_string *o, పూర్ణాంకం n)
{
	అక్షరం **list;
	పూర్ణాంకం string_ప్రారంభం;

	ఒకవేళ (DEBUG_EXPAND)
		debug_print_list("finalized", o, n);
	debug_printf_expand("finalized n:%d\n", n);
	list = (అక్షరం**)o->దత్తాంశం;
	string_ప్రారంభం = ((n + 0xf) & ~0xf) * పరిమాణం(list[0]);
	list[--n] = ఖాళీ;
	అయితే (n) {
		n--;
		list[n] = o->దత్తాంశం + (పూర్ణాంకం)(uintptr_t)list[n] + string_ప్రారంభం;
	}
	తిరిగి list;
}

స్థిర శూన్య free_pipe_list(నిర్మాణం pipe *pi);

/* Returns pi->next - next pipe in the list */
స్థిర నిర్మాణం pipe *free_pipe(నిర్మాణం pipe *pi)
{
	నిర్మాణం pipe *తదుపరి;
	పూర్ణాంకం i;

#నిర్వచితం_కాకపోతే __U_BOOT__
	debug_printf_clean("free_pipe (pid %d)\n", getpid());
#అయితే_ముగింపు /* !__U_BOOT__ */
	కొరకు (i = 0; i < pi->అంకె_cmds; i++) {
		నిర్మాణం command *command;
#నిర్వచితం_కాకపోతే __U_BOOT__
		నిర్మాణం redir_struct *r, *rnext;
#అయితే_ముగింపు /* !__U_BOOT__ */

		command = &pi->cmds[i];
		debug_printf_clean("  command %d:\n", i);
		ఒకవేళ (command->argv) {
			ఒకవేళ (DEBUG_CLEAN) {
				పూర్ణాంకం a;
				అక్షరం **p;
				కొరకు (a = 0, p = command->argv; *p; a++, p++) {
					debug_printf_clean("   argv[%d] = %s\n", a, *p);
				}
			}
			free_అక్షరశ్రేణులు(command->argv);
			//command->argv = NULL;
		}
		/* not "else if": on syntax error, we may have both! */
		ఒకవేళ (command->సమూహం) {
			debug_printf_clean("   begin group (cmd_type:%d)\n",
					command->cmd_type);
			free_pipe_list(command->సమూహం);
			debug_printf_clean("   end group\n");
			//command->group = NULL;
		}
		/* else is crucial here.
		 * If group != NULL, child_func is meaningless */
#ఒకవేళ ENABLE_HUSH_FUNCTIONS
		లేకపోతే ఒకవేళ (command->బిడ్డ_func) {
			debug_printf_exec("cmd %p releases child func at %p\n", command, command->బిడ్డ_func);
			command->బిడ్డ_func->మాతృ_cmd = ఖాళీ;
		}
#అయితే_ముగింపు
#ఒకవేళ !BB_MMU
		విడుదల(command->సమూహం_as_string);
		//command->group_as_string = NULL;
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే __U_BOOT__
		కొరకు (r = command->redirects; r; r = rnext) {
			debug_printf_clean("   redirect %d%s",
					r->rd_fd, redir_పట్టిక[r->rd_type].descrip);
			/* guard against the case >$FOO, where foo is unset or blank */
			ఒకవేళ (r->rd_filename) {
				debug_printf_clean(" fname:'%s'\n", r->rd_filename);
				విడుదల(r->rd_filename);
				//r->rd_filename = NULL;
			}
			debug_printf_clean(" rd_dup:%d\n", r->rd_dup);
			rnext = r->తదుపరి;
			విడుదల(r);
		}
		//command->redirects = NULL;
#అయితే_ముగింపు /* !__U_BOOT__ */
	}
	విడుదల(pi->cmds);   /* children are an array, they get freed all at once */
	//pi->cmds = NULL;
#నిర్వచితం_కాకపోతే __U_BOOT__
#ఒకవేళ ENABLE_HUSH_పని
	విడుదల(pi->cmdtext);
	//pi->cmdtext = NULL;
#అయితే_ముగింపు
#అయితే_ముగింపు /* !__U_BOOT__ */

	తదుపరి = pi->తదుపరి;
	విడుదల(pi);
	తిరిగి తదుపరి;
}

స్థిర శూన్య free_pipe_list(నిర్మాణం pipe *pi)
{
	అయితే (pi) {
#ఒకవేళ HAS_KEYWORDS
		debug_printf_clean("pipe reserved word %d\n", pi->res_మాట);
#అయితే_ముగింపు
		debug_printf_clean("pipe followup code %d\n", pi->followup);
		pi = free_pipe(pi);
	}
}

/*** Parsing routines ***/

#నిర్వచితం_కాకపోతే debug_print_వృక్షం
స్థిర శూన్య debug_print_వృక్షం(నిర్మాణం pipe *pi, పూర్ణాంకం lvl)
{
	స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి PIPE[] ALIGN_PTR = {
		[PIPE_SEQ] = "SEQ",
		[PIPE_AND] = "AND",
		[PIPE_OR ] = "OR" ,
		[PIPE_BG ] = "BG" ,
	};
	స్థిర స్థిరరాశి అక్షరం *RES[] = {
		[RES_NONE ] = "NONE" ,
# ఒకవేళ ENABLE_HUSH_IF
		[RES_IF   ] = "IF"   ,
		[RES_THEN ] = "THEN" ,
		[RES_ELIF ] = "ELIF" ,
		[RES_ELSE ] = "ELSE" ,
		[RES_FI   ] = "FI"   ,
# endif
# ఒకవేళ ENABLE_HUSH_LOOPS
		[RES_FOR  ] = "FOR"  ,
		[RES_WHILE] = "WHILE",
		[RES_UNTIL] = "UNTIL",
		[RES_DO   ] = "DO"   ,
		[RES_పూర్తి ] = "DONE" ,
# endif
# ఒకవేళ ENABLE_HUSH_LOOPS || ENABLE_HUSH_CASE
		[RES_లోపల   ] = "IN"   ,
# endif
# ఒకవేళ ENABLE_HUSH_CASE
		[RES_CASE ] = "CASE" ,
		[RES_CASE_లోపల ] = "CASE_IN" ,
		[RES_సరిపోలిక] = "MATCH",
		[RES_CASE_BODY] = "CASE_BODY",
		[RES_ESAC ] = "ESAC" ,
# endif
		[RES_XXXX ] = "XXXX" ,
		[RES_SNTX ] = "SNTX" ,
	};
	స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి CMDTYPE[] ALIGN_PTR = {
		"{}",
		"()",
		"[noglob]",
# ఒకవేళ ENABLE_HUSH_FUNCTIONS
		"func()",
# endif
	};

	పూర్ణాంకం pin, prn;

	pin = 0;
	అయితే (pi) {
		fdprintf(2, "%*spipe %d #cmds:%d %sres_word=%s followup=%d %s\n",
				lvl*2, "",
				pin,
				pi->అంకె_cmds,
				(IF_HAS_KEYWORDS(pi->pi_inverted ? "! " :) ""),
				RES[pi->res_మాట],
				pi->followup, PIPE[pi->followup]
		);
		prn = 0;
		అయితే (prn < pi->అంకె_cmds) {
			నిర్మాణం command *command = &pi->cmds[prn];
			అక్షరం **argv = command->argv;

			fdprintf(2, "%*s cmd %d assignment_cnt:%d",
					lvl*2, "", prn,
					command->assignment_cnt);
# ఒకవేళ ENABLE_HUSH_LINENO_VAR
			fdprintf(2, " LINENO:%u", command->lineno);
# endif
			ఒకవేళ (command->సమూహం) {
				fdprintf(2, " group %s: (argv=%p)%s%s\n",
						CMDTYPE[command->cmd_type],
						argv
# ఒకవేళ !BB_MMU
						, " group_as_string:", command->సమూహం_as_string
# లేకపోతే
						, "", ""
# endif
				);
				debug_print_వృక్షం(command->సమూహం, lvl+1);
				prn++;
				కొనసాగు;
			}
			ఒకవేళ (argv) అయితే (*argv) {
				fdprintf(2, " '%s'", *argv);
				argv++;
			}
#నిర్వచితం_కాకపోతే __U_BOOT__
			ఒకవేళ (command->redirects)
				fdprintf(2, " {redir}");
#అయితే_ముగింపు /* __U_BOOT__ */
			fdprintf(2, "\n");
			prn++;
		}
		pi = pi->తదుపరి;
		pin++;
	}
}
#అయితే_ముగింపు /* debug_print_tree */

స్థిర నిర్మాణం pipe *new_pipe(శూన్య)
{
	నిర్మాణం pipe *pi;
	pi = xzalloc(పరిమాణం(నిర్మాణం pipe));
	/*pi->res_word = RES_NONE; - RES_NONE is 0 anyway */
	తిరిగి pi;
}

/* Command (member of a pipe) is complete, or we start a new pipe
 * if ctx->command is NULL.
 * No errors possible here.
 */
స్థిర పూర్ణాంకం పూర్తి_command(నిర్మాణం parse_context *ctx)
{
	/* The command is really already in the pipe structure, so
	 * advance the pipe counter and make a new, null command. */
	నిర్మాణం pipe *pi = ctx->pipe;
	నిర్మాణం command *command = ctx->command;

#ఒకవేళ 0	/* Instead we emit error message at run time */
	ఒకవేళ (ctx->pending_redirect) {
		/* For example, "cmd >" (no filename to redirect to) */
		syntax_error("invalid redirect");
		ctx->pending_redirect = ఖాళీ;
	}
#అయితే_ముగింపు

	ఒకవేళ (command) {
		ఒకవేళ (IS_NULL_CMD(command)) {
			debug_printf_parse("done_command: skipping null cmd, num_cmds=%d\n", pi->అంకె_cmds);
			వెళ్ళు clear_and_ret;
		}
		pi->అంకె_cmds++;
		debug_printf_parse("done_command: ++num_cmds=%d\n", pi->అంకె_cmds);
		//debug_print_tree(ctx->list_head, 20);
	} లేకపోతే {
		debug_printf_parse("done_command: initializing, num_cmds=%d\n", pi->అంకె_cmds);
	}

	/* Only real trickiness here is that the uncommitted
	 * command structure is not counted in pi->num_cmds. */
	pi->cmds = xrealloc(pi->cmds, పరిమాణం(*pi->cmds) * (pi->అంకె_cmds+1));
	ctx->command = command = &pi->cmds[pi->అంకె_cmds];
 clear_and_ret:
	memset(command, 0, పరిమాణం(*command));
#ఒకవేళ ENABLE_HUSH_LINENO_VAR
	command->lineno = G.parse_lineno;
	debug_printf_parse("command->lineno = G.parse_lineno (%u)\n", G.parse_lineno);
#అయితే_ముగింపు
	తిరిగి pi->అంకె_cmds; /* used only for 0/nonzero check */
}

స్థిర శూన్య పూర్తి_pipe(నిర్మాణం parse_context *ctx, pipe_style type)
{
	పూర్ణాంకం not_null;

	debug_printf_parse("done_pipe entered, followup %d\n", type);
	/* Close previous command */
	not_null = పూర్తి_command(ctx);
#ఒకవేళ HAS_KEYWORDS
	ctx->pipe->pi_inverted = ctx->ctx_inverted;
	ctx->ctx_inverted = 0;
	ctx->pipe->res_మాట = ctx->ctx_res_w;
#అయితే_ముగింపు
	ఒకవేళ (type == PIPE_BG && ctx->list_తల != ctx->pipe) {
		/* Necessary since && and || have precedence over &:
		 * "cmd1 && cmd2 &" must spawn both cmds, not only cmd2,
		 * in a backgrounded subshell.
		 */
		నిర్మాణం pipe *pi;
		నిర్మాణం command *command;

		/* Is this actually this construct, all pipes end with && or ||? */
		pi = ctx->list_తల;
		అయితే (pi != ctx->pipe) {
			ఒకవేళ (pi->followup != PIPE_AND && pi->followup != PIPE_OR)
				వెళ్ళు no_conv;
			pi = pi->తదుపరి;
		}

		debug_printf_parse("BG with more than one pipe, converting to { p1 &&...pN; } &\n");
		pi->followup = PIPE_SEQ; /* close pN _not_ with "&"! */
		pi = xzalloc(పరిమాణం(*pi));
		pi->followup = PIPE_BG;
		pi->అంకె_cmds = 1;
		pi->cmds = xzalloc(పరిమాణం(pi->cmds[0]));
		command = &pi->cmds[0];
		ఒకవేళ (CMD_సాధారణ != 0) /* "if xzalloc didn't do that already" */
			command->cmd_type = CMD_సాధారణ;
		command->సమూహం = ctx->list_తల;
#ఒకవేళ !BB_MMU
		command->సమూహం_as_string = xstrndup(
			    ctx->as_string.దత్తాంశం,
			    ctx->as_string.నిడివి - 1 /* do not copy last char, "&" */
		);
#అయితే_ముగింపు
		/* Replace all pipes in ctx with one newly created */
		ctx->list_తల = ctx->pipe = pi;
		/* for cases like "cmd && &", do not be tricked by last command
		 * being null - the entire {...} & is NOT null! */
		not_null = 1;
	} లేకపోతే {
 no_conv:
		ctx->pipe->followup = type;
	}

	/* Without this check, even just <enter> on command line generates
	 * tree of three NOPs (!). Which is harmless but annoying.
	 * IOW: it is safe to do it unconditionally. */
	ఒకవేళ (not_null
#ఒకవేళ ENABLE_HUSH_IF
	 || ctx->ctx_res_w == RES_FI
#అయితే_ముగింపు
#ఒకవేళ ENABLE_HUSH_LOOPS
	 || ctx->ctx_res_w == RES_పూర్తి
	 || ctx->ctx_res_w == RES_FOR
	 || ctx->ctx_res_w == RES_లోపల
#అయితే_ముగింపు
#ఒకవేళ ENABLE_HUSH_CASE
	 || ctx->ctx_res_w == RES_ESAC
#అయితే_ముగింపు
	) {
		నిర్మాణం pipe *new_p;
		debug_printf_parse("done_pipe: adding new pipe: "
#నిర్వచితం_కాకపోతే __U_BOOT__
				"not_null:%d ctx->ctx_res_w:%d\n",
				not_null, ctx->ctx_res_w);
#లేకపోతే /* __U_BOOT__ */
				"not_null:%d\n",
				not_null);
#అయితే_ముగింపు /* __U_BOOT__ */
		new_p = new_pipe();
		ctx->pipe->తదుపరి = new_p;
		ctx->pipe = new_p;
		/* RES_THEN, RES_DO etc are "sticky" -
		 * they remain set for pipes inside if/while.
		 * This is used to control execution.
		 * RES_FOR and RES_IN are NOT sticky (needed to support
		 * cases where variable or value happens to match a keyword):
		 */
#ఒకవేళ ENABLE_HUSH_LOOPS
		ఒకవేళ (ctx->ctx_res_w == RES_FOR
		 || ctx->ctx_res_w == RES_లోపల)
			ctx->ctx_res_w = RES_NONE;
#అయితే_ముగింపు
#ఒకవేళ ENABLE_HUSH_CASE
		ఒకవేళ (ctx->ctx_res_w == RES_సరిపోలిక)
			ctx->ctx_res_w = RES_CASE_BODY;
		ఒకవేళ (ctx->ctx_res_w == RES_CASE)
			ctx->ctx_res_w = RES_CASE_లోపల;
#అయితే_ముగింపు
		ctx->command = ఖాళీ; /* trick done_command below */
		/* Create the memory for command, roughly:
		 * ctx->pipe->cmds = new struct command;
		 * ctx->command = &ctx->pipe->cmds[0];
		 */
		పూర్తి_command(ctx);
		//debug_print_tree(ctx->list_head, 10);
	}
	debug_printf_parse("done_pipe return\n");
}

స్థిర శూన్య initialize_context(నిర్మాణం parse_context *ctx)
{
	memset(ctx, 0, పరిమాణం(*ctx));
	ఒకవేళ (MAYBE_ASSIGNMENT != 0)
		ctx->is_assignment = MAYBE_ASSIGNMENT;
	ctx->pipe = ctx->list_తల = new_pipe();
	/* Create the memory for command, roughly:
	 * ctx->pipe->cmds = new struct command;
	 * ctx->command = &ctx->pipe->cmds[0];
	 */
	పూర్తి_command(ctx);
}

/* If a reserved word is found and processed, parse context is modified
 * and 1 is returned.
 */
#ఒకవేళ HAS_KEYWORDS
నిర్మాణం reserved_combo {
	అక్షరం literal[6];
	గుర్తులేని అక్షరం res;
	గుర్తులేని అక్షరం assignment_flag;
	uint32_t flag;
};
గణన {
	FLAG_చివర   = (1 << RES_NONE ),
# ఒకవేళ ENABLE_HUSH_IF
	FLAG_IF    = (1 << RES_IF   ),
	FLAG_THEN  = (1 << RES_THEN ),
	FLAG_ELIF  = (1 << RES_ELIF ),
	FLAG_ELSE  = (1 << RES_ELSE ),
	FLAG_FI    = (1 << RES_FI   ),
# endif
# ఒకవేళ ENABLE_HUSH_LOOPS
	FLAG_FOR   = (1 << RES_FOR  ),
	FLAG_WHILE = (1 << RES_WHILE),
	FLAG_UNTIL = (1 << RES_UNTIL),
	FLAG_DO    = (1 << RES_DO   ),
	FLAG_పూర్తి  = (1 << RES_పూర్తి ),
	FLAG_లోపల    = (1 << RES_లోపల   ),
# endif
# ఒకవేళ ENABLE_HUSH_CASE
	FLAG_సరిపోలిక = (1 << RES_సరిపోలిక),
	FLAG_ESAC  = (1 << RES_ESAC ),
# endif
	FLAG_ప్రారంభం = (1 << RES_XXXX ),
};

స్థిర స్థిరరాశి నిర్మాణం reserved_combo* సరిపోలిక_reserved_మాట(o_string *మాట)
{
	/* Mostly a list of accepted follow-up reserved words.
	 * FLAG_END means we are done with the sequence, and are ready
	 * to turn the compound list into a command.
	 * FLAG_START means the word must start a new compound list.
	 */
	స్థిర స్థిరరాశి నిర్మాణం reserved_combo reserved_list[] ALIGN4 = {
# ఒకవేళ ENABLE_HUSH_IF
		{ "!",     RES_NONE,  NOT_ASSIGNMENT  , 0 },
		{ "if",    RES_IF,    MAYBE_ASSIGNMENT, FLAG_THEN | FLAG_ప్రారంభం },
		{ "then",  RES_THEN,  MAYBE_ASSIGNMENT, FLAG_ELIF | FLAG_ELSE | FLAG_FI },
		{ "elif",  RES_ELIF,  MAYBE_ASSIGNMENT, FLAG_THEN },
		{ "else",  RES_ELSE,  MAYBE_ASSIGNMENT, FLAG_FI   },
		{ "fi",    RES_FI,    NOT_ASSIGNMENT  , FLAG_చివర  },
# endif
# ఒకవేళ ENABLE_HUSH_LOOPS
		{ "for",   RES_FOR,   NOT_ASSIGNMENT  , FLAG_లోపల | FLAG_DO | FLAG_ప్రారంభం },
		{ "while", RES_WHILE, MAYBE_ASSIGNMENT, FLAG_DO | FLAG_ప్రారంభం },
		{ "until", RES_UNTIL, MAYBE_ASSIGNMENT, FLAG_DO | FLAG_ప్రారంభం },
		{ "in",    RES_లోపల,    NOT_ASSIGNMENT  , FLAG_DO   },
		{ "do",    RES_DO,    MAYBE_ASSIGNMENT, FLAG_పూర్తి },
		{ "done",  RES_పూర్తి,  NOT_ASSIGNMENT  , FLAG_చివర  },
# endif
# ఒకవేళ ENABLE_HUSH_CASE
		{ "case",  RES_CASE,  NOT_ASSIGNMENT  , FLAG_సరిపోలిక | FLAG_ప్రారంభం },
		{ "esac",  RES_ESAC,  NOT_ASSIGNMENT  , FLAG_చివర  },
# endif
	};
	స్థిరరాశి నిర్మాణం reserved_combo *r;

	కొరకు (r = reserved_list; r < reserved_list + ARRAY_కొలత(reserved_list); r++) {
		ఒకవేళ (strcmp(మాట->దత్తాంశం, r->literal) == 0)
			తిరిగి r;
	}
	తిరిగి ఖాళీ;
}
/* Return NULL: not a keyword, else: keyword
 */
స్థిర స్థిరరాశి నిర్మాణం reserved_combo* reserved_మాట(నిర్మాణం parse_context *ctx)
{
# ఒకవేళ ENABLE_HUSH_CASE
	స్థిర స్థిరరాశి నిర్మాణం reserved_combo reserved_సరిపోలిక = {
		"",        RES_సరిపోలిక, NOT_ASSIGNMENT , FLAG_సరిపోలిక | FLAG_ESAC
	};
# endif
	స్థిరరాశి నిర్మాణం reserved_combo *r;

	ఒకవేళ (ctx->మాట.has_quoted_part)
		తిరిగి 0;
	r = సరిపోలిక_reserved_మాట(&ctx->మాట);
	ఒకవేళ (!r)
		తిరిగి r; /* NULL */

	debug_printf("found reserved word %s, res %d\n", r->literal, r->res);
# ఒకవేళ ENABLE_HUSH_CASE
	ఒకవేళ (r->res == RES_లోపల && ctx->ctx_res_w == RES_CASE_లోపల) {
		/* "case word IN ..." - IN part starts first MATCH part */
		r = &reserved_సరిపోలిక;
	} లేకపోతే
# endif
	ఒకవేళ (r->flag == 0) { /* '!' */
		ఒకవేళ (ctx->ctx_inverted) { /* bash doesn't accept '! ! true' */
			syntax_error("! ! command");
			ctx->ctx_res_w = RES_SNTX;
		}
		ctx->ctx_inverted = 1;
		తిరిగి r;
	}
	ఒకవేళ (r->flag & FLAG_ప్రారంభం) {
		నిర్మాణం parse_context *old;

		old = xmemdup(ctx, పరిమాణం(*ctx));
		debug_printf_parse("push stack %p\n", old);
		initialize_context(ctx);
		ctx->దొంతర = old;
	} లేకపోతే ఒకవేళ (/*ctx->ctx_res_w == RES_NONE ||*/ !(ctx->old_flag & (1 << r->res))) {
		syntax_error_at(ctx->మాట.దత్తాంశం);
		ctx->ctx_res_w = RES_SNTX;
		తిరిగి r;
	} లేకపోతే {
		/* "{...} fi" is ok. "{...} if" is not
		 * Example:
		 * if { echo foo; } then { echo bar; } fi */
		ఒకవేళ (ctx->command->సమూహం)
			పూర్తి_pipe(ctx, PIPE_SEQ);
	}

	ctx->ctx_res_w = r->res;
	ctx->old_flag = r->flag;
	ctx->is_assignment = r->assignment_flag;
	debug_printf_parse("ctx->is_assignment='%s'\n", assignment_flag[ctx->is_assignment]);

	ఒకవేళ (ctx->old_flag & FLAG_చివర) {
		నిర్మాణం parse_context *old;

		పూర్తి_pipe(ctx, PIPE_SEQ);
		debug_printf_parse("pop stack %p\n", ctx->దొంతర);
		old = ctx->దొంతర;
		old->command->సమూహం = ctx->list_తల;
		old->command->cmd_type = CMD_సాధారణ;
# ఒకవేళ !BB_MMU
		/* At this point, the compound command's string is in
		 * ctx->as_string... except for the leading keyword!
		 * Consider this example: "echo a | if true; then echo a; fi"
		 * ctx->as_string will contain "true; then echo a; fi",
		 * with "if " remaining in old->as_string!
		 */
		{
			అక్షరం *str;
			పూర్ణాంకం len = old->as_string.నిడివి;
			/* Concatenate halves */
			o_addstr(&old->as_string, ctx->as_string.దత్తాంశం);
			o_free(&ctx->as_string);
			/* Find where leading keyword starts in first half */
			str = old->as_string.దత్తాంశం + len;
			ఒకవేళ (str > old->as_string.దత్తాంశం)
				str--; /* skip whitespace after keyword */
			అయితే (str > old->as_string.దత్తాంశం && అక్షరమా(str[-1]))
				str--;
			/* Ugh, we're done with this horrid hack */
			old->command->సమూహం_as_string = xstrdup(str);
			debug_printf_parse("pop, remembering as:'%s'\n",
					old->command->సమూహం_as_string);
		}
# endif
		*ctx = *old;   /* physical copy */
		విడుదల(old);
	}
	తిరిగి r;
}
#అయితే_ముగింపు /* HAS_KEYWORDS */

/* Word is complete, look at it and update parsing context.
 * Normal return is 0. Syntax errors return 1.
 * Note: on return, word is reset, but not o_free'd!
 */
స్థిర పూర్ణాంకం పూర్తి_మాట(నిర్మాణం parse_context *ctx)
{
	నిర్మాణం command *command = ctx->command;

	debug_printf_parse("done_word entered: '%s' %p\n", ctx->మాట.దత్తాంశం, command);
	ఒకవేళ (ctx->మాట.నిడివి == 0 && !ctx->మాట.has_quoted_part) {
		debug_printf_parse("done_word return 0: true null, ignored\n");
		తిరిగి 0;
	}

#నిర్వచితం_కాకపోతే __U_BOOT__
	ఒకవేళ (ctx->pending_redirect) {
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

		ctx->pending_redirect->rd_filename = xstrdup(ctx->మాట.దత్తాంశం);
		/* Cater for >\file case:
		 * >\a creates file a; >\\a, >"\a", >"\\a" create file \a
		 * Same with heredocs:
		 * for <<\H delim is H; <<\\H, <<"\H", <<"\\H" - \H
		 */
		ఒకవేళ (ctx->pending_redirect->rd_type == REDIRECT_HEREDOC) {
			unbackslash(ctx->pending_redirect->rd_filename);
			/* Is it <<"HEREDOC"? */
			ఒకవేళ (ctx->మాట.has_quoted_part) {
				ctx->pending_redirect->rd_dup |= HEREDOC_QUOTED;
			}
		}
		debug_printf_parse("word stored in rd_filename: '%s'\n", ctx->మాట.దత్తాంశం);
		ctx->pending_redirect = ఖాళీ;
	} లేకపోతే {
#అయితే_ముగింపు /* !__U_BOOT__ */
#ఒకవేళ HAS_KEYWORDS
# ఒకవేళ ENABLE_HUSH_CASE
		ఒకవేళ (ctx->ctx_dsemicolon
		 && strcmp(ctx->మాట.దత్తాంశం, "esac") != 0 /* not "... pattern) cmd;; esac" */
		) {
			/* already done when ctx_dsemicolon was set to 1: */
			/* ctx->ctx_res_w = RES_MATCH; */
			ctx->ctx_dsemicolon = 0;
		} లేకపోతే
# endif
# ఒకవేళ defined(CMD_TEST2_SINGLEWORD_NOGLOB)
		ఒకవేళ (command->cmd_type == CMD_TEST2_SINGLEWORD_NOGLOB
		 && strcmp(ctx->మాట.దత్తాంశం, "]]") == 0
		) {
			/* allow "[[ ]] >file" etc */
			command->cmd_type = CMD_SINGLEWORD_NOGLOB;
		} లేకపోతే
# endif
		ఒకవేళ (!command->argv /* if it's the first word... */
# ఒకవేళ ENABLE_HUSH_LOOPS
		 && ctx->ctx_res_w != RES_FOR /* ...not after FOR or IN */
		 && ctx->ctx_res_w != RES_లోపల
# endif
# ఒకవేళ ENABLE_HUSH_CASE
		 && ctx->ctx_res_w != RES_CASE
# endif
		) {
			స్థిరరాశి నిర్మాణం reserved_combo *reserved;
			reserved = reserved_మాట(ctx);
			debug_printf_parse("checking for reserved-ness: %d\n", !!reserved);
			ఒకవేళ (reserved) {
# ఒకవేళ ENABLE_HUSH_LINENO_VAR
/* Case:
 * "while ...; do
 *	cmd ..."
 * If we don't close the pipe _now_, immediately after "do", lineno logic
 * sees "cmd" as starting at "do" - i.e., at the previous line.
 */
				ఒకవేళ (0
				 IF_HUSH_IF(|| reserved->res == RES_THEN)
				 IF_HUSH_IF(|| reserved->res == RES_ELIF)
				 IF_HUSH_IF(|| reserved->res == RES_ELSE)
				 IF_HUSH_LOOPS(|| reserved->res == RES_DO)
				) {
					పూర్తి_pipe(ctx, PIPE_SEQ);
				}
# endif
				o_reset_గమ్యం_ఖాళీలు_unquoted(&ctx->మాట);
				debug_printf_parse("done_word return %d\n",
						(ctx->ctx_res_w == RES_SNTX));
				తిరిగి (ctx->ctx_res_w == RES_SNTX);
			}
# ఒకవేళ defined(CMD_TEST2_SINGLEWORD_NOGLOB)
			ఒకవేళ (strcmp(ctx->మాట.దత్తాంశం, "[[") == 0) {
				command->cmd_type = CMD_TEST2_SINGLEWORD_NOGLOB;
			} లేకపోతే
# endif
# ఒకవేళ defined(CMD_SINGLEWORD_NOGLOB)
			ఒకవేళ (0
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
			 IF_HUSH_LOCAL(   || strcmp(ctx->మాట.దత్తాంశం, "local") == 0)
			 IF_HUSH_EXPORT(  || strcmp(ctx->మాట.దత్తాంశం, "export") == 0)
			 IF_HUSH_READONLY(|| strcmp(ctx->మాట.దత్తాంశం, "readonly") == 0)
			) {
				command->cmd_type = CMD_SINGLEWORD_NOGLOB;
			}
# లేకపోతే
			{ /* empty block to pair "if ... else" */ }
# endif
		}
#అయితే_ముగింపు /* HAS_KEYWORDS */

		ఒకవేళ (command->సమూహం) {
			/* "{ echo foo; } echo bar" - bad */
			syntax_error_at(ctx->మాట.దత్తాంశం);
			debug_printf_parse("done_word return 1: syntax error, "
					"groups and arglists don't mix\n");
			తిరిగి 1;
		}

		/* If this word wasn't an assignment, next ones definitely
		 * can't be assignments. Even if they look like ones. */
		ఒకవేళ (ctx->is_assignment != DEFINITELY_ASSIGNMENT
		 && ctx->is_assignment != మాట_IS_KEYWORD
		) {
			ctx->is_assignment = NOT_ASSIGNMENT;
		} లేకపోతే {
			ఒకవేళ (ctx->is_assignment == DEFINITELY_ASSIGNMENT) {
				command->assignment_cnt++;
				debug_printf_parse("++assignment_cnt=%d\n", command->assignment_cnt);
			}
			debug_printf_parse("ctx->is_assignment was:'%s'\n", assignment_flag[ctx->is_assignment]);
			ctx->is_assignment = MAYBE_ASSIGNMENT;
		}
		debug_printf_parse("ctx->is_assignment='%s'\n", assignment_flag[ctx->is_assignment]);
		command->argv = కూడు_string_గమ్యం_అక్షరశ్రేణులు(command->argv, xstrdup(ctx->మాట.దత్తాంశం));
#నిర్వచితమైతే __U_BOOT__
		command->argc++;
#అయితే_ముగింపు /* __U_BOOT__ */
		debug_print_అక్షరశ్రేణులు("word appended to argv", command->argv);

#నిర్వచితం_కాకపోతే __U_BOOT__
	}
#అయితే_ముగింపు /* !__U_BOOT__ */

#ఒకవేళ ENABLE_HUSH_LOOPS
	ఒకవేళ (ctx->ctx_res_w == RES_FOR) {
		ఒకవేళ (ctx->మాట.has_quoted_part
		 || endofname(command->argv[0])[0] != '\0'
		) {
			/* bash says just "not a valid identifier" */
			syntax_error("bad for loop variable");
			తిరిగి 1;
		}
		/* Force FOR to have just one word (variable name) */
		/* NB: basically, this makes hush see "for v in ..."
		 * syntax as if it is "for v; in ...". FOR and IN become
		 * two pipe structs in parse tree. */
		పూర్తి_pipe(ctx, PIPE_SEQ);
	}
#అయితే_ముగింపు
#ఒకవేళ ENABLE_HUSH_CASE
	/* Force CASE to have just one word */
	ఒకవేళ (ctx->ctx_res_w == RES_CASE) {
		పూర్తి_pipe(ctx, PIPE_SEQ);
	}
#అయితే_ముగింపు

	o_reset_గమ్యం_ఖాళీలు_unquoted(&ctx->మాట);

	debug_printf_parse("done_word return 0\n");
	తిరిగి 0;
}

#నిర్వచితం_కాకపోతే __U_BOOT__
/* Peek ahead in the input to find out if we have a "&n" construct,
 * as in "2>&1", that represents duplicating a file descriptor.
 * Return:
 * REDIRFD_CLOSE if >&- "close fd" construct is seen,
 * REDIRFD_SYNTAX_ERR if syntax error,
 * REDIRFD_TO_FILE if no & was seen,
 * or the number found.
 */
#ఒకవేళ BB_MMU
#నిర్వచించు parse_redir_కుడి_fd(as_string, ఆదానం) \
	parse_redir_కుడి_fd(ఆదానం)
#అయితే_ముగింపు
స్థిర పూర్ణాంకం parse_redir_కుడి_fd(o_string *as_string, నిర్మాణం లోపల_str *ఆదానం)
{
	పూర్ణాంకం ch, d, ok;

	ch = i_peek(ఆదానం);
	ఒకవేళ (ch != '&')
		తిరిగి REDIRFD_గమ్యం_FILE;

	ch = i_getch(ఆదానం);  /* get the & */
	nommu_addchr(as_string, ch);
	ch = i_peek(ఆదానం);
	ఒకవేళ (ch == '-') {
		ch = i_getch(ఆదానం);
		nommu_addchr(as_string, ch);
		తిరిగి REDIRFD_CLOSE;
	}
	d = 0;
	ok = 0;
	అయితే (ch != EOF && అంకెనా(ch)) {
		d = d*10 + (ch-'0');
		ok = 1;
		ch = i_getch(ఆదానం);
		nommu_addchr(as_string, ch);
		ch = i_peek(ఆదానం);
	}
	ఒకవేళ (ok) తిరిగి d;

//TODO: this is the place to catch ">&file" bashism (redirect both fd 1 and 2)

	bb_simple_error_సందేశం("ambiguous redirect");
	తిరిగి REDIRFD_SYNTAX_ERR;
}

/* Return code is 0 normal, 1 if a syntax error is detected
 */
స్థిర పూర్ణాంకం parse_redirect(నిర్మాణం parse_context *ctx,
		పూర్ణాంకం fd,
		redir_type style,
		నిర్మాణం లోపల_str *ఆదానం)
{
	నిర్మాణం command *command = ctx->command;
	నిర్మాణం redir_struct *redir;
	నిర్మాణం redir_struct **redirp;
	పూర్ణాంకం dup_అంకె;

	dup_అంకె = REDIRFD_గమ్యం_FILE;
	ఒకవేళ (style != REDIRECT_HEREDOC) {
		/* Check for a '>&1' type redirect */
		dup_అంకె = parse_redir_కుడి_fd(&ctx->as_string, ఆదానం);
		ఒకవేళ (dup_అంకె == REDIRFD_SYNTAX_ERR)
			తిరిగి 1;
	} లేకపోతే {
		పూర్ణాంకం ch = i_peek_and_eat_bkslash_nl(ఆదానం);
		dup_అంకె = (ch == '-'); /* HEREDOC_SKIPTABS bit is 1 */
		ఒకవేళ (dup_అంకె) { /* <<-... */
			ch = i_getch(ఆదానం);
			nommu_addchr(&ctx->as_string, ch);
			ch = i_peek(ఆదానం);
		}
	}

	ఒకవేళ (style == REDIRECT_OVERWRITE && dup_అంకె == REDIRFD_గమ్యం_FILE) {
		పూర్ణాంకం ch = i_peek_and_eat_bkslash_nl(ఆదానం);
		ఒకవేళ (ch == '|') {
			/* >|FILE redirect ("clobbering" >).
			 * Since we do not support "set -o noclobber" yet,
			 * >| and > are the same for now. Just eat |.
			 */
			ch = i_getch(ఆదానం);
			nommu_addchr(&ctx->as_string, ch);
		}
	}

	/* Create a new redir_struct and append it to the linked list */
	redirp = &command->redirects;
	అయితే ((redir = *redirp) != ఖాళీ) {
		redirp = &(redir->తదుపరి);
	}
	*redirp = redir = xzalloc(పరిమాణం(*redir));
	/* redir->next = NULL; */
	/* redir->rd_filename = NULL; */
	redir->rd_type = style;
	redir->rd_fd = (fd == -1) ? redir_పట్టిక[style].default_fd : fd;

	debug_printf_parse("redirect type %d %s\n", redir->rd_fd,
				redir_పట్టిక[style].descrip);

	redir->rd_dup = dup_అంకె;
	ఒకవేళ (style != REDIRECT_HEREDOC && dup_అంకె != REDIRFD_గమ్యం_FILE) {
		/* Erik had a check here that the file descriptor in question
		 * is legit; I postpone that to "run time"
		 * A "-" representation of "close me" shows up as a -3 here */
		debug_printf_parse("duplicating redirect '%d>&%d'\n",
				redir->rd_fd, redir->rd_dup);
	} లేకపోతే {
#ఒకవేళ 0		/* Instead we emit error message at run time */
		ఒకవేళ (ctx->pending_redirect) {
			/* For example, "cmd > <file" */
			syntax_error("invalid redirect");
		}
#అయితే_ముగింపు
		/* Set ctx->pending_redirect, so we know what to do at the
		 * end of the next parsed word. */
		ctx->pending_redirect = redir;
	}
	తిరిగి 0;
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
స్థిర పూర్ణాంకం redirect_opt_అంకె(o_string *o)
{
	పూర్ణాంకం అంకె;

	ఒకవేళ (o->దత్తాంశం == ఖాళీ)
		తిరిగి -1;
	అంకె = bb_strtou(o->దత్తాంశం, ఖాళీ, 10);
	ఒకవేళ (errno || అంకె < 0)
		తిరిగి -1;
	o_reset_గమ్యం_ఖాళీలు_unquoted(o);
	తిరిగి అంకె;
}

#ఒకవేళ BB_MMU
#నిర్వచించు fetch_till_str(as_string, ఆదానం, మాట, skip_tabs) \
	fetch_till_str(ఆదానం, మాట, skip_tabs)
#అయితే_ముగింపు
స్థిర అక్షరం *fetch_till_str(o_string *as_string,
		నిర్మాణం లోపల_str *ఆదానం,
		స్థిరరాశి అక్షరం *మాట,
		పూర్ణాంకం heredoc_flags)
{
	o_string heredoc = NULL_O_STRING;
	గుర్తులేని past_EOL;
	పూర్ణాంకం prev = 0; /* not \ */
	పూర్ణాంకం ch;

	/* Starting with "" is necessary for this case:
	 * cat <<EOF
	 *
	 * xxx
	 * EOF
	 */
	heredoc.దత్తాంశం = xzalloc(1); /* start as "", not as NULL */

	వెళ్ళు jump_లోపల;

	అయితే (1) {
		ch = i_getch(ఆదానం);
		ఒకవేళ (ch != EOF)
			nommu_addchr(as_string, ch);
		ఒకవేళ (ch == '\n' || ch == EOF) {
 check_heredoc_చివర:
#నిర్వచితం_కాకపోతే __U_BOOT__
			ఒకవేళ ((heredoc_flags & HEREDOC_QUOTED) || prev != '\\') {
#లేకపోతే /* __U_BOOT__ */
			ఒకవేళ (prev != '\\') {
#అయితే_ముగింపు
				/* End-of-line, and not a line continuation */
				ఒకవేళ (strcmp(heredoc.దత్తాంశం + past_EOL, మాట) == 0) {
					heredoc.దత్తాంశం[past_EOL] = '\0';
					debug_printf_heredoc("parsed '%s' heredoc '%s'\n", మాట, heredoc.దత్తాంశం);
					తిరిగి heredoc.దత్తాంశం;
				}
				ఒకవేళ (ch == '\n') {
					/* This is a new line.
					 * Remember position and backslash-escaping status.
					 */
					o_addchr(&heredoc, ch);
					prev = ch;
 jump_లోపల:
					past_EOL = heredoc.నిడివి;
					/* Get 1st char of next line, possibly skipping leading tabs */
					చేయి {
						ch = i_getch(ఆదానం);
						ఒకవేళ (ch != EOF)
							nommu_addchr(as_string, ch);
#నిర్వచితం_కాకపోతే __U_BOOT__
					} అయితే ((heredoc_flags & HEREDOC_SKIPTABS) && ch == '\t');
#లేకపోతే /* __U_BOOT__ */
				} అయితే (ch == '\t');
#అయితే_ముగింపు
					/* If this immediately ended the line,
					 * go back to end-of-line checks.
					 */
					ఒకవేళ (ch == '\n')
						వెళ్ళు check_heredoc_చివర;
				}
			} లేకపోతే {
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
				heredoc.దత్తాంశం[--heredoc.నిడివి] = '\0';
				prev = 0; /* not '\' */
				కొనసాగు;
			}
		}
		ఒకవేళ (ch == EOF) {
			o_free(&heredoc);
			తిరిగి ఖాళీ; /* error */
		}
		o_addchr(&heredoc, ch);
		nommu_addchr(as_string, ch);
		ఒకవేళ (prev == '\\' && ch == '\\')
			/* Correctly handle foo\\<eol> (not a line cont.) */
			prev = 0; /* not '\' */
		లేకపోతే
			prev = ch;
	}
}
#అయితే_ముగింపు /* !__U_BOOT__ */

/* Look at entire parse tree for not-yet-loaded REDIRECT_HEREDOCs
 * and load them all. There should be exactly heredoc_cnt of them.
 */
#ఒకవేళ BB_MMU
#నిర్వచించు fetch_heredocs(as_string, pi, heredoc_cnt, ఆదానం) \
	fetch_heredocs(pi, heredoc_cnt, ఆదానం)
#అయితే_ముగింపు
స్థిర పూర్ణాంకం fetch_heredocs(o_string *as_string, నిర్మాణం pipe *pi, పూర్ణాంకం heredoc_cnt, నిర్మాణం లోపల_str *ఆదానం)
{
	అయితే (pi && heredoc_cnt) {
		పూర్ణాంకం i;
		నిర్మాణం command *cmd = pi->cmds;

		debug_printf_heredoc("fetch_heredocs: num_cmds:%d cmd argv0:'%s'\n",
				pi->అంకె_cmds,
				cmd->argv ? cmd->argv[0] : "NONE"
		);
		కొరకు (i = 0; i < pi->అంకె_cmds; i++) {
#నిర్వచితం_కాకపోతే __U_BOOT__
			నిర్మాణం redir_struct *redir = cmd->redirects;

#అయితే_ముగింపు /* !__U_BOOT__ */
			debug_printf_heredoc("fetch_heredocs: %d cmd argv0:'%s'\n",
					i, cmd->argv ? cmd->argv[0] : "NONE");
#నిర్వచితం_కాకపోతే __U_BOOT__
			అయితే (redir) {
				ఒకవేళ (redir->rd_type == REDIRECT_HEREDOC) {
					అక్షరం *p;

					redir->rd_type = REDIRECT_HEREDOC2;
					/* redir->rd_dup is (ab)used to indicate <<- */
					p = fetch_till_str(as_string, ఆదానం,
							redir->rd_filename, redir->rd_dup);
					ఒకవేళ (!p) {
						syntax_error("unexpected EOF in here document");
						తిరిగి -1;
					}
					విడుదల(redir->rd_filename);
					redir->rd_filename = p;
					heredoc_cnt--;
				}
				redir = redir->తదుపరి;
			}
#అయితే_ముగింపు /* !__U_BOOT__ */
			ఒకవేళ (cmd->సమూహం) {
				//bb_error_msg("%s:%u heredoc_cnt:%d", __func__, __LINE__, heredoc_cnt);
				heredoc_cnt = fetch_heredocs(as_string, cmd->సమూహం, heredoc_cnt, ఆదానం);
				//bb_error_msg("%s:%u heredoc_cnt:%d", __func__, __LINE__, heredoc_cnt);
				ఒకవేళ (heredoc_cnt < 0)
					తిరిగి heredoc_cnt; /* error */
			}
			cmd++;
		}
		pi = pi->తదుపరి;
	}
	తిరిగి heredoc_cnt;
}

స్థిర పూర్ణాంకం నిరంతరం_list(నిర్మాణం pipe *pi);
#ఒకవేళ BB_MMU
#నిర్వచించు parse_ప్రవాహం(pstring, heredoc_cnt_ptr, ఆదానం, చివర_trigger) \
	parse_ప్రవాహం(heredoc_cnt_ptr, ఆదానం, చివర_trigger)
#అయితే_ముగింపు
స్థిర నిర్మాణం pipe *parse_ప్రవాహం(అక్షరం **pstring,
		పూర్ణాంకం *heredoc_cnt_ptr,
		నిర్మాణం లోపల_str *ఆదానం,
		పూర్ణాంకం చివర_trigger);

/* Returns number of heredocs not yet consumed,
 * or -1 on error.
 */
స్థిర పూర్ణాంకం parse_సమూహం(నిర్మాణం parse_context *ctx,
		నిర్మాణం లోపల_str *ఆదానం, పూర్ణాంకం ch)
{
	/* ctx->word contains characters seen prior to ( or {.
	 * Typically it's empty, but for function defs,
	 * it contains function name (without '()'). */
#ఒకవేళ BB_MMU
# define as_string ఖాళీ
#లేకపోతే
	అక్షరం *as_string = ఖాళీ;
#అయితే_ముగింపు
	నిర్మాణం pipe *pipe_list;
	పూర్ణాంకం heredoc_cnt = 0;
	పూర్ణాంకం endch;
	నిర్మాణం command *command = ctx->command;

	debug_printf_parse("parse_group entered\n");
#ఒకవేళ ENABLE_HUSH_FUNCTIONS
	ఒకవేళ (ch == '(' && !ctx->మాట.has_quoted_part) {
		ఒకవేళ (ctx->మాట.నిడివి)
			ఒకవేళ (పూర్తి_మాట(ctx))
				తిరిగి -1;
		ఒకవేళ (!command->argv)
			వెళ్ళు skip; /* (... */
		ఒకవేళ (command->argv[1]) { /* word word ... (... */
			syntax_error_unexpected_ch('(');
			తిరిగి -1;
		}
		/* it is "word(..." or "word (..." */
		చేయి
			ch = i_getch(ఆదానం);
		అయితే (ch == ' ' || ch == '\t');
		ఒకవేళ (ch != ')') {
			syntax_error_unexpected_ch(ch);
			తిరిగి -1;
		}
		nommu_addchr(&ctx->as_string, ch);
		చేయి
			ch = i_getch(ఆదానం);
		అయితే (ch == ' ' || ch == '\t' || ch == '\n');
		ఒకవేళ (ch != '{' && ch != '(') {
			syntax_error_unexpected_ch(ch);
			తిరిగి -1;
		}
//bash allows functions named "123", "..", "return"!
//		if (endofname(command->argv[0])[0] != '\0') {
//			syntax_error("bad function name");
//			return -1;
//		}
		nommu_addchr(&ctx->as_string, ch);
		command->cmd_type = CMD_FUNCDEF;
		వెళ్ళు skip;
	}
#అయితే_ముగింపు

#ఒకవేళ 0 /* Prevented by caller */
	ఒకవేళ (command->argv /* word [word]{... */
	 || ctx->మాట.నిడివి /* word{... */
	 || ctx->మాట.has_quoted_part /* ""{... */
	) {
		syntax_error(ఖాళీ);
		debug_printf_parse("parse_group return -1: "
			"syntax error, groups and arglists don't mix\n");
		తిరిగి -1;
	}
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే __U_BOOT__
 IF_HUSH_FUNCTIONS(skip:)
#అయితే_ముగింపు /* !__U_BOOT__ */

	endch = '}';
	ఒకవేళ (ch == '(') {
		endch = ')';
#నిర్వచితం_కాకపోతే __U_BOOT__
		IF_HUSH_FUNCTIONS(ఒకవేళ (command->cmd_type != CMD_FUNCDEF))
			command->cmd_type = CMD_SUBSHELL;
#అయితే_ముగింపు /* !__U_BOOT__ */
	} లేకపోతే {
		/* bash does not allow "{echo...", requires whitespace */
		ch = i_peek(ఆదానం);
		ఒకవేళ (ch != ' ' && ch != '\t' && ch != '\n'
		 && ch != '('	/* but "{(..." is allowed (without whitespace) */
		) {
			syntax_error_unexpected_ch(ch);
			తిరిగి -1;
		}
		ఒకవేళ (ch != '(') {
			ch = i_getch(ఆదానం);
			nommu_addchr(&ctx->as_string, ch);
		}
	}

	debug_printf_heredoc("calling parse_stream, heredoc_cnt:%d\n", heredoc_cnt);
	pipe_list = parse_ప్రవాహం(&as_string, &heredoc_cnt, ఆదానం, endch);
	debug_printf_heredoc("parse_stream returned: heredoc_cnt:%d\n", heredoc_cnt);
#ఒకవేళ !BB_MMU
	ఒకవేళ (as_string)
		o_addstr(&ctx->as_string, as_string);
#అయితే_ముగింపు

	/* empty ()/{} or parse error? */
	ఒకవేళ (!pipe_list || pipe_list == ERR_PTR) {
		/* parse_stream already emitted error msg */
		ఒకవేళ (!BB_MMU)
			విడుదల(as_string);
		debug_printf_parse("parse_group return -1: "
			"parse_stream returned %p\n", pipe_list);
		తిరిగి -1;
	}
#ఒకవేళ !BB_MMU
	as_string[అక్షరశ్రేణి_పొడవు(as_string) - 1] = '\0'; /* plink ')' or '}' */
	command->సమూహం_as_string = as_string;
	debug_printf_parse("end of group, remembering as:'%s'\n",
			command->సమూహం_as_string);
#అయితే_ముగింపు

#ఒకవేళ ENABLE_HUSH_FUNCTIONS
	/* Convert "f() (cmds)" to "f() {(cmds)}" */
	ఒకవేళ (command->cmd_type == CMD_FUNCDEF && endch == ')') {
		నిర్మాణం command *cmd2;

		cmd2 = xzalloc(పరిమాణం(*cmd2));
		cmd2->cmd_type = CMD_SUBSHELL;
		cmd2->సమూహం = pipe_list;
# ఒకవేళ !BB_MMU
//UNTESTED!
		cmd2->సమూహం_as_string = command->సమూహం_as_string;
		command->సమూహం_as_string = xasprintf("(%s)", command->సమూహం_as_string);
# endif

		pipe_list = new_pipe();
		pipe_list->cmds = cmd2;
		pipe_list->అంకె_cmds = 1;
	}
#అయితే_ముగింపు

	command->సమూహం = pipe_list;

	debug_printf_parse("parse_group return %d\n", heredoc_cnt);
	తిరిగి heredoc_cnt;
	/* command remains "open", available for possible redirects */
#నిర్వచనం_రద్దు as_string
}

#నిర్వచితం_కాకపోతే __U_BOOT__
#ఒకవేళ ENABLE_HUSH_TICK || ENABLE_FEATURE_SH_MATH || ENABLE_HUSH_DOLLAR_OPS
/* Subroutines for copying $(...) and `...` things */
/* '...' */
స్థిర పూర్ణాంకం కూడు_till_single_quote(o_string *dest, నిర్మాణం లోపల_str *ఆదానం)
{
	అయితే (1) {
		పూర్ణాంకం ch = i_getch(ఆదానం);
		ఒకవేళ (ch == EOF) {
			syntax_error_unterm_ch('\'');
			తిరిగి 0;
		}
		ఒకవేళ (ch == '\'')
			తిరిగి 1;
		o_addchr(dest, ch);
	}
}
స్థిర పూర్ణాంకం కూడు_till_single_quote_dquoted(o_string *dest, నిర్మాణం లోపల_str *ఆదానం)
{
	అయితే (1) {
		పూర్ణాంకం ch = i_getch(ఆదానం);
		ఒకవేళ (ch == EOF) {
			syntax_error_unterm_ch('\'');
			తిరిగి 0;
		}
		ఒకవేళ (ch == '\'')
			తిరిగి 1;
		o_addqchr(dest, ch);
	}
}

/* "...\"...`..`...." - do we need to handle "...$(..)..." too? */
స్థిర పూర్ణాంకం కూడు_till_backquote(o_string *dest, నిర్మాణం లోపల_str *ఆదానం, పూర్ణాంకం లోపల_dquote);
స్థిర పూర్ణాంకం కూడు_till_double_quote(o_string *dest, నిర్మాణం లోపల_str *ఆదానం)
{
	అయితే (1) {
		పూర్ణాంకం ch = i_getch(ఆదానం);
		ఒకవేళ (ch == EOF) {
			syntax_error_unterm_ch('"');
			తిరిగి 0;
		}
		ఒకవేళ (ch == '"')
			తిరిగి 1;
		ఒకవేళ (ch == '\\') {  /* \x. Copy both chars. */
			o_addchr(dest, ch);
			ch = i_getch(ఆదానం);
		}
		o_addchr(dest, ch);
		ఒకవేళ (ch == '`') {
			ఒకవేళ (!కూడు_till_backquote(dest, ఆదానం, /*in_dquote:*/ 1))
				తిరిగి 0;
			o_addchr(dest, ch);
			కొనసాగు;
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
స్థిర పూర్ణాంకం కూడు_till_backquote(o_string *dest, నిర్మాణం లోపల_str *ఆదానం, పూర్ణాంకం లోపల_dquote)
{
	అయితే (1) {
		పూర్ణాంకం ch = i_getch(ఆదానం);
		ఒకవేళ (ch == '`')
			తిరిగి 1;
		ఒకవేళ (ch == '\\') {
			/* \x. Copy both unless it is \`, \$, \\ and maybe \" */
			ch = i_getch(ఆదానం);
			ఒకవేళ (ch != '`'
			 && ch != '$'
			 && ch != '\\'
			 && (!లోపల_dquote || ch != '"')
			) {
				o_addchr(dest, '\\');
			}
		}
		ఒకవేళ (ch == EOF) {
			syntax_error_unterm_ch('`');
			తిరిగి 0;
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
#నిర్వచించు DOUBLE_CLOSE_CHAR_FLAG 0x80
స్థిర పూర్ణాంకం కూడు_till_closing_bracket(o_string *dest, నిర్మాణం లోపల_str *ఆదానం, గుర్తులేని చివర_ch)
{
	పూర్ణాంకం ch;
	అక్షరం dbl = చివర_ch & DOUBLE_CLOSE_CHAR_FLAG;
# ఒకవేళ BASH_SUBSTR || BASH_నమూనా_SUBST
	అక్షరం చివర_char2 = చివర_ch >> 8;
# endif
	చివర_ch &= (DOUBLE_CLOSE_CHAR_FLAG - 1);

# ఒకవేళ ENABLE_HUSH_INTERACTIVE
	G.promptmode = 1; /* PS2 */
# endif
	debug_printf_prompt("%s promptmode=%d\n", __func__, G.promptmode);

	అయితే (1) {
		ch = i_getch(ఆదానం);
		ఒకవేళ (ch == EOF) {
			syntax_error_unterm_ch(చివర_ch);
			తిరిగి 0;
		}
		ఒకవేళ (ch == చివర_ch
# ఒకవేళ BASH_SUBSTR || BASH_నమూనా_SUBST
		 || ch == చివర_char2
# endif
		) {
			ఒకవేళ (!dbl)
				ఆపు;
			/* we look for closing )) of $((EXPR)) */
			ఒకవేళ (i_peek_and_eat_bkslash_nl(ఆదానం) == చివర_ch) {
				i_getch(ఆదానం); /* eat second ')' */
				ఆపు;
			}
		}
		o_addchr(dest, ch);
		//bb_error_msg("%s:o_addchr('%c')", __func__, ch);
		ఒకవేళ (ch == '(' || ch == '{') {
			ch = (ch == '(' ? ')' : '}');
			ఒకవేళ (!కూడు_till_closing_bracket(dest, ఆదానం, ch))
				తిరిగి 0;
			o_addchr(dest, ch);
			కొనసాగు;
		}
		ఒకవేళ (ch == '\'') {
			ఒకవేళ (!కూడు_till_single_quote(dest, ఆదానం))
				తిరిగి 0;
			o_addchr(dest, ch);
			కొనసాగు;
		}
		ఒకవేళ (ch == '"') {
			ఒకవేళ (!కూడు_till_double_quote(dest, ఆదానం))
				తిరిగి 0;
			o_addchr(dest, ch);
			కొనసాగు;
		}
		ఒకవేళ (ch == '`') {
			ఒకవేళ (!కూడు_till_backquote(dest, ఆదానం, /*in_dquote:*/ 0))
				తిరిగి 0;
			o_addchr(dest, ch);
			కొనసాగు;
		}
		ఒకవేళ (ch == '\\') {
			/* \x. Copy verbatim. Important for  \(, \) */
			ch = i_getch(ఆదానం);
			ఒకవేళ (ch == EOF) {
				syntax_error_unterm_ch(చివర_ch);
				తిరిగి 0;
			}
# ఒకవేళ 0
			ఒకవేళ (ch == '\n') {
				/* "backslash+newline", ignore both */
				o_delchr(dest); /* undo insertion of '\' */
				కొనసాగు;
			}
# endif
			o_addchr(dest, ch);
			//bb_error_msg("%s:o_addchr('%c') after '\\'", __func__, ch);
			కొనసాగు;
		}
	}
	debug_printf_parse("%s return '%s' ch:'%c'\n", __func__, dest->దత్తాంశం, ch);
	తిరిగి ch;
}
#అయితే_ముగింపు /* ENABLE_HUSH_TICK || ENABLE_FEATURE_SH_MATH || ENABLE_HUSH_DOLLAR_OPS */

#ఒకవేళ BASH_DOLLAR_SQUOTE
/* Return code: 1 for "found and parsed", 0 for "seen something else" */
# ఒకవేళ BB_MMU
#నిర్వచించు parse_dollar_squote(as_string, dest, ఆదానం) \
	parse_dollar_squote(dest, ఆదానం)
#నిర్వచించు as_string ఖాళీ
# endif
స్థిర పూర్ణాంకం parse_dollar_squote(o_string *as_string, o_string *dest, నిర్మాణం లోపల_str *ఆదానం)
{
	పూర్ణాంకం ప్రారంభం;
	పూర్ణాంకం ch = i_peek_and_eat_bkslash_nl(ఆదానం);  /* first character after the $ */
	debug_printf_parse("parse_dollar_squote entered: ch='%c'\n", ch);
	ఒకవేళ (ch != '\'')
		తిరిగి 0;

	dest->has_quoted_part = 1;
	ప్రారంభం = dest->నిడివి;

	ch = i_getch(ఆదానం); /* eat ' */
	nommu_addchr(as_string, ch);
	అయితే (1) {
		ch = i_getch(ఆదానం);
		nommu_addchr(as_string, ch);
		ఒకవేళ (ch == EOF) {
			syntax_error_unterm_ch('\'');
			తిరిగి 0;
		}
		ఒకవేళ (ch == '\'')
			ఆపు;
		ఒకవేళ (ch == SPECIAL_VAR_చిహ్నం) {
			/* Convert raw ^C to corresponding special variable reference */
			o_addchr(dest, SPECIAL_VAR_చిహ్నం);
			o_addchr(dest, SPECIAL_VAR_QUOTED_SVS);
			/* will addchr() another SPECIAL_VAR_SYMBOL (see after the if() block) */
		} లేకపోతే ఒకవేళ (ch == '\\') {
			స్థిర స్థిరరాశి అక్షరం C_escapes[] ALIGN1 = "nrbtfav""x\\01234567";

			ch = i_getch(ఆదానం);
			nommu_addchr(as_string, ch);
			ఒకవేళ (strchr(C_escapes, ch)) {
				అక్షరం buf[4];
				అక్షరం *p = buf;
				పూర్ణాంకం cnt = 2;

				buf[0] = ch;
				ఒకవేళ ((గుర్తులేని అక్షరం)(ch - '0') <= 7) { /* \ooo */
					చేయి {
						ch = i_peek(ఆదానం);
						ఒకవేళ ((గుర్తులేని అక్షరం)(ch - '0') > 7)
							ఆపు;
						*++p = ch = i_getch(ఆదానం);
						nommu_addchr(as_string, ch);
					} అయితే (--cnt != 0);
				} లేకపోతే ఒకవేళ (ch == 'x') { /* \xHH */
					చేయి {
						ch = i_peek(ఆదానం);
						ఒకవేళ (!isxdigit(ch))
							ఆపు;
						*++p = ch = i_getch(ఆదానం);
						nommu_addchr(as_string, ch);
					} అయితే (--cnt != 0);
					ఒకవేళ (cnt == 2) { /* \x but next char is "bad" */
						ch = 'x';
						వెళ్ళు unrecognized;
					}
				} /* else simple seq like \\ or \t */
				*++p = '\0';
				p = buf;
				ch = bb_process_escape_sequence((శూన్య*)&p);
				//bb_error_msg("buf:'%s' ch:%x", buf, ch);
				ఒకవేళ (ch == '\0')
					కొనసాగు; /* bash compat: $'...\0...' emits nothing */
			} లేకపోతే { /* unrecognized "\z": encode both chars unless ' or " */
				ఒకవేళ (ch != '\'' && ch != '"') {
 unrecognized:
					o_addqchr(dest, '\\');
				}
			}
		} /* if (\...) */
		o_addqchr(dest, ch);
	}

	ఒకవేళ (dest->నిడివి == ప్రారంభం) {
		/* $'', $'\0', $'\000\x00' and the like */
		o_addchr(dest, SPECIAL_VAR_చిహ్నం);
		o_addchr(dest, SPECIAL_VAR_చిహ్నం);
	}

	తిరిగి 1;
# undef as_string
}
#లేకపోతే
# define parse_dollar_squote(as_string, dest, ఆదానం) 0
#అయితే_ముగింపు /* BASH_DOLLAR_SQUOTE */
#అయితే_ముగింపు /* !__U_BOOT__ */

/* Return code: 0 for OK, 1 for syntax error */
#ఒకవేళ BB_MMU
#నిర్వచించు parse_dollar(as_string, dest, ఆదానం, quote_ముసుగు) \
	parse_dollar(dest, ఆదానం, quote_ముసుగు)
#నిర్వచించు as_string ఖాళీ
#అయితే_ముగింపు
స్థిర పూర్ణాంకం parse_dollar(o_string *as_string,
		o_string *dest,
		నిర్మాణం లోపల_str *ఆదానం, గుర్తులేని అక్షరం quote_ముసుగు)
{
	పూర్ణాంకం ch = i_peek_and_eat_bkslash_nl(ఆదానం);  /* first character after the $ */

	debug_printf_parse("parse_dollar entered: ch='%c' quote_mask:0x%x\n", ch, quote_ముసుగు);
	ఒకవేళ (అక్షరమా(ch)) {
 make_var:
		ch = i_getch(ఆదానం);
		nommu_addchr(as_string, ch);
 /*make_var1:*/
		o_addchr(dest, SPECIAL_VAR_చిహ్నం);
		అయితే (1) {
			debug_printf_parse(": '%c'\n", ch);
			o_addchr(dest, ch | quote_ముసుగు);
			quote_ముసుగు = 0;
			ch = i_peek_and_eat_bkslash_nl(ఆదానం);
			ఒకవేళ (!అక్షరమా_అంకెనా(ch) && ch != '_') {
				/* End of variable name reached */
				ఆపు;
			}
			ch = i_getch(ఆదానం);
			nommu_addchr(as_string, ch);
		}
		o_addchr(dest, SPECIAL_VAR_చిహ్నం);
	} లేకపోతే ఒకవేళ (అంకెనా(ch)) {
 make_one_char_var:
		ch = i_getch(ఆదానం);
		nommu_addchr(as_string, ch);
		o_addchr(dest, SPECIAL_VAR_చిహ్నం);
		debug_printf_parse(": '%c'\n", ch);
		o_addchr(dest, ch | quote_ముసుగు);
		o_addchr(dest, SPECIAL_VAR_చిహ్నం);
	} లేకపోతే ఎంపిక (ch) {
#నిర్వచితం_కాకపోతే __U_BOOT__
	స్థితి '$': /* pid */
	స్థితి '!': /* last bg pid */
#అయితే_ముగింపు /* !__U_BOOT__ */
	స్థితి '?': /* last exit code */
	స్థితి '#': /* number of args */
	స్థితి '*': /*వాదనలు*/
	స్థితి '@': /*వాదనలు*/
	స్థితి '-': /* $- option flags set by set builtin or shell options (-i etc) */
		వెళ్ళు make_one_char_var;
	స్థితి '{': {
		అక్షరం len_single_ch;

		o_addchr(dest, SPECIAL_VAR_చిహ్నం);

		ch = i_getch(ఆదానం); /* eat '{' */
		nommu_addchr(as_string, ch);

		ch = i_getch_and_eat_bkslash_nl(ఆదానం); /* first char after '{' */
		/* It should be ${?}, or ${#var},
		 * or even ${?+subst} - operator acting on a special variable,
		 * or the beginning of variable name.
		 */
		ఒకవేళ (ch == EOF
		 || (!strchr(_SPECIAL_VARS_STR, ch) && !అక్షరమా_అంకెనా(ch)) /* not one of those */
		) {
 bad_dollar_syntax:
			syntax_error_unterm_str("${name}");
			debug_printf_parse("parse_dollar return 0: unterminated ${name}\n");
			తిరిగి 0;
		}
		nommu_addchr(as_string, ch);
		len_single_ch = ch;
		ch |= quote_ముసుగు;

		/* It's possible to just call add_till_closing_bracket() at this point.
		 * However, this regresses some of our testsuite cases
		 * which check invalid constructs like ${%}.
		 * Oh well... let's check that the var name part is fine... */

		ఒకవేళ (అంకెనా(len_single_ch)
		 || (len_single_ch == '#' && అంకెనా(i_peek_and_eat_bkslash_nl(ఆదానం)))
		) {
			/* Execution engine uses plain xatoi_positive()
			 * to interpret ${NNN} and {#NNN},
			 * check syntax here in the parser.
			 * (bash does not support expressions in ${#NN},
			 * e.g. ${#$var} and {#1:+WORD} are not supported).
			 */
			గుర్తులేని cnt = 9; /* max 9 digits for ${NN} and 8 for {#NN} */
			అయితే (1) {
				o_addchr(dest, ch);
				debug_printf_parse(": '%c'\n", ch);
				ch = i_getch_and_eat_bkslash_nl(ఆదానం);
				nommu_addchr(as_string, ch);
				ఒకవేళ (ch == '}')
					ఆపు;
				ఒకవేళ (--cnt == 0)
					వెళ్ళు bad_dollar_syntax;
				ఒకవేళ (len_single_ch != '#' && strchr(VAR_SUBST_OPS, ch))
					/* ${NN<op>...} is valid */
					వెళ్ళు eat_until_closing;
				ఒకవేళ (!అంకెనా(ch))
					వెళ్ళు bad_dollar_syntax;
			}
		} లేకపోతే
		అయితే (1) {
			గుర్తులేని pos;

			o_addchr(dest, ch);
			debug_printf_parse(": '%c'\n", ch);

			ch = i_getch(ఆదానం);
			nommu_addchr(as_string, ch);
			ఒకవేళ (ch == '}')
				ఆపు;
#నిర్వచితం_కాకపోతే __U_BOOT__
			ఒకవేళ (!అక్షరమా_అంకెనా(ch) && ch != '_') {
#లేకపోతే /* __U_BOOT__ */
			/*
			 * In several places in U-Boot, we use variable like
			 * foo# (e.g. serial#), particularly in env.
			 * So, we need to authorize # to appear inside
			 * variable name and then expand this variable.
			 * NOTE Having # in variable name is not permitted in
			 * upstream hush but expansion will be done (even though
			 * the result will be empty).
			 */
			ఒకవేళ (!అక్షరమా_అంకెనా(ch) && ch != '_' && ch != '#') {
#అయితే_ముగింపు /* __U_BOOT__ */
				గుర్తులేని చివర_ch;
#నిర్వచితం_కాకపోతే __U_BOOT__
				గుర్తులేని అక్షరం చివరి_ch;
#అయితే_ముగింపు /* !__U_BOOT__ */
				/* handle parameter expansions
				 * http://www.opengroup.org/onlinepubs/009695399/utilities/xcu_chap02.html#tag_02_06_02
				 */
				ఒకవేళ (!strchr(VAR_SUBST_OPS, ch)) { /* ${var<bad_char>... */
					ఒకవేళ (len_single_ch != '#'
					/*|| !strchr(SPECIAL_VARS_STR, ch) - disallow errors like ${#+} ? */
					 || i_peek(ఆదానం) != '}'
					) {
						వెళ్ళు bad_dollar_syntax;
					}
					/* else: it's "length of C" ${#C} op,
					 * where C is a single char
					 * special var name, e.g. ${#!}.
					 */
				}
 eat_until_closing:
				/* Eat everything until closing '}' (or ':') */
				చివర_ch = '}';
#నిర్వచితం_కాకపోతే __U_BOOT__
				ఒకవేళ (BASH_SUBSTR
				 && ch == ':'
				 && !strchr(MINUS_PLUS_EQUAL_QUESTION, i_peek(ఆదానం))
				) {
					/* It's ${var:N[:M]} thing */
					చివర_ch = '}' * 0x100 + ':';
				}
				ఒకవేళ (BASH_నమూనా_SUBST
				 && ch == '/'
				) {
					/* It's ${var/[/]pattern[/repl]} thing */
					ఒకవేళ (i_peek(ఆదానం) == '/') { /* ${var//pattern[/repl]}? */
						i_getch(ఆదానం);
						nommu_addchr(as_string, '/');
						ch = '\\';
					}
					చివర_ch = '}' * 0x100 + '/';
				}
#అయితే_ముగింపు /* !__U_BOOT__ */
				o_addchr(dest, ch);
				/* The pattern can't be empty.
				 * IOW: if the first char after "${v//" is a slash,
				 * it does not terminate the pattern - it's the first char of the pattern:
				 *  v=/dev/ram; echo ${v////-}  prints -dev-ram (pattern is "/")
				 *  v=/dev/ram; echo ${v///r/-} prints /dev-am  (pattern is "/r")
				 */
				ఒకవేళ (i_peek(ఆదానం) == '/') {
					o_addchr(dest, i_getch(ఆదానం));
				}
#నిర్వచితం_కాకపోతే __U_BOOT__
 again:
#అయితే_ముగింపు /* !__U_BOOT__ */
				ఒకవేళ (!BB_MMU)
					pos = dest->నిడివి;
#ఒకవేళ ENABLE_HUSH_DOLLAR_OPS
#నిర్వచితం_కాకపోతే __U_BOOT__
				చివరి_ch = కూడు_till_closing_bracket(dest, ఆదానం, చివర_ch);
				ఒకవేళ (చివరి_ch == 0) /* error? */
					తిరిగి 0;
#అయితే_ముగింపు /* !__U_BOOT__ */
#లేకపోతే
# error Simple సంకేతం_2 గమ్యం only allow ${var} is not implemented
#అయితే_ముగింపు
				ఒకవేళ (as_string) {
					o_addstr(as_string, dest->దత్తాంశం + pos);
#నిర్వచితం_కాకపోతే __U_BOOT__
					o_addchr(as_string, చివరి_ch);
#అయితే_ముగింపు /* !__U_BOOT__ */
				}

#నిర్వచితం_కాకపోతే __U_BOOT__
				ఒకవేళ ((BASH_SUBSTR || BASH_నమూనా_SUBST)
					 && (చివర_ch & 0xff00)
				) {
					/* close the first block: */
					o_addchr(dest, SPECIAL_VAR_చిహ్నం);
					/* while parsing N from ${var:N[:M]}
					 * or pattern from ${var/[/]pattern[/repl]} */
					ఒకవేళ ((చివర_ch & 0xff) == చివరి_ch) {
						/* got ':' or '/'- parse the rest */
						చివర_ch = '}';
						వెళ్ళు again;
					}
					/* got '}' */
					ఒకవేళ (BASH_SUBSTR && చివర_ch == '}' * 0x100 + ':') {
						/* it's ${var:N} - emulate :999999999 */
						o_addstr(dest, "999999999");
					} /* else: it's ${var/[/]pattern} */
				}
#అయితే_ముగింపు /* !__U_BOOT__ */
				ఆపు;
			}
			len_single_ch = 0; /* it can't be ${#C} op */
		}
		o_addchr(dest, SPECIAL_VAR_చిహ్నం);
		ఆపు;
	}
#ఒకవేళ ENABLE_FEATURE_SH_MATH || ENABLE_HUSH_TICK
	స్థితి '(': {
		గుర్తులేని pos;

		ch = i_getch(ఆదానం);
		nommu_addchr(as_string, ch);
# ఒకవేళ ENABLE_FEATURE_SH_MATH
		ఒకవేళ (i_peek_and_eat_bkslash_nl(ఆదానం) == '(') {
			ch = i_getch(ఆదానం);
			nommu_addchr(as_string, ch);
			o_addchr(dest, SPECIAL_VAR_చిహ్నం);
			o_addchr(dest, quote_ముసుగు | '+');
			ఒకవేళ (!BB_MMU)
				pos = dest->నిడివి;
			ఒకవేళ (!కూడు_till_closing_bracket(dest, ఆదానం, ')' | DOUBLE_CLOSE_CHAR_FLAG))
				తిరిగి 0; /* error */
			ఒకవేళ (as_string) {
				o_addstr(as_string, dest->దత్తాంశం + pos);
				o_addchr(as_string, ')');
				o_addchr(as_string, ')');
			}
			o_addchr(dest, SPECIAL_VAR_చిహ్నం);
			ఆపు;
		}
# endif
# ఒకవేళ ENABLE_HUSH_TICK
		o_addchr(dest, SPECIAL_VAR_చిహ్నం);
		o_addchr(dest, quote_ముసుగు | '`');
		ఒకవేళ (!BB_MMU)
			pos = dest->నిడివి;
		ఒకవేళ (!కూడు_till_closing_bracket(dest, ఆదానం, ')'))
			తిరిగి 0; /* error */
		ఒకవేళ (as_string) {
			o_addstr(as_string, dest->దత్తాంశం + pos);
			o_addchr(as_string, ')');
		}
		o_addchr(dest, SPECIAL_VAR_చిహ్నం);
# endif
		ఆపు;
	}
#అయితే_ముగింపు
	స్థితి '_':
		వెళ్ళు make_var;
#ఒకవేళ 0
	/* TODO: $_: */
	/* $_ Shell or shell script name; or last argument of last command
	 * (if last command wasn't a pipe; if it was, bash sets $_ to "");
	 * but in command's env, set to full pathname used to invoke it */
		ch = i_getch(ఆదానం);
		nommu_addchr(as_string, ch);
		ch = i_peek_and_eat_bkslash_nl(ఆదానం);
		ఒకవేళ (అక్షరమా_అంకెనా(ch)) { /* it's $_name or $_123 */
			ch = '_';
			వెళ్ళు make_var1;
		}
		/* else: it's $_ */
#అయితే_ముగింపు
	అప్రమేయ:
		o_కూడు_Qchr(dest, '$');
	}
	debug_printf_parse("parse_dollar return 1 (ok)\n");
	తిరిగి 1;
#నిర్వచనం_రద్దు as_string
}

#ఒకవేళ BB_MMU
#నిర్వచించు encode_string(as_string, dest, ఆదానం, dquote_చివర) \
	encode_string(dest, ఆదానం, dquote_చివర)
#నిర్వచించు as_string ఖాళీ
#అయితే_ముగింపు
స్థిర పూర్ణాంకం encode_string(o_string *as_string,
		o_string *dest,
		నిర్మాణం లోపల_str *ఆదానం,
		పూర్ణాంకం dquote_చివర)
{
	పూర్ణాంకం ch;
	పూర్ణాంకం తదుపరి;

 again:
	ch = i_getch(ఆదానం);
	ఒకవేళ (ch != EOF)
		nommu_addchr(as_string, ch);
	ఒకవేళ (ch == dquote_చివర) { /* may be only '"' or EOF */
		debug_printf_parse("encode_string return 1 (ok)\n");
		తిరిగి 1;
	}
	/* note: can't move it above ch == dquote_end check! */
	ఒకవేళ (ch == EOF) {
		syntax_error_unterm_ch('"');
		తిరిగి 0; /* error */
	}
	తదుపరి = '\0';
	ఒకవేళ (ch != '\n') {
		తదుపరి = i_peek(ఆదానం);
	}
	debug_printf_parse("\" ch=%c (%d) escape=%d\n",
			ch, ch, !!(dest->o_expflags & EXP_FLAG_ESC_GLOB_CHARS));
	ఒకవేళ (ch == '\\') {
		ఒకవేళ (తదుపరి == EOF) {
			/* Testcase: in interactive shell a file with
			 *  echo "unterminated string\<eof>
			 * is sourced.
			 */
			syntax_error_unterm_ch('"');
			తిరిగి 0; /* error */
		}
		/* bash:
		 * "The backslash retains its special meaning [in "..."]
		 * only when followed by one of the following characters:
		 * $, `, ", \, or <newline>.  A double quote may be quoted
		 * within double quotes by preceding it with a backslash."
		 * NB: in (unquoted) heredoc, above does not apply to ",
		 * therefore we check for it by "next == dquote_end" cond.
		 */
		ఒకవేళ (తదుపరి == dquote_చివర || strchr("$`\\\n", తదుపరి)) {
			ch = i_getch(ఆదానం); /* eat next */
			ఒకవేళ (ch == '\n')
				వెళ్ళు again; /* skip \<newline> */
		} /* else: ch remains == '\\', and we double it below: */
		o_addqchr(dest, ch); /* \c if c is a glob char, else just c */
		nommu_addchr(as_string, ch);
		వెళ్ళు again;
	}
	ఒకవేళ (ch == '$') {
		//if (parse_dollar_squote(as_string, dest, input))
		//	goto again;
		ఒకవేళ (!parse_dollar(as_string, dest, ఆదానం, /*quote_mask:*/ 0x80)) {
			debug_printf_parse("encode_string return 0: "
					"parse_dollar returned 0 (error)\n");
			తిరిగి 0;
		}
		వెళ్ళు again;
	}
#ఒకవేళ ENABLE_HUSH_TICK
	ఒకవేళ (ch == '`') {
		//unsigned pos = dest->length;
		o_addchr(dest, SPECIAL_VAR_చిహ్నం);
		o_addchr(dest, 0x80 | '`');
		ఒకవేళ (!కూడు_till_backquote(dest, ఆదానం, /*in_dquote:*/ dquote_చివర == '"'))
			తిరిగి 0; /* error */
		o_addchr(dest, SPECIAL_VAR_చిహ్నం);
		//debug_printf_subst("SUBST RES3 '%s'\n", dest->data + pos);
		వెళ్ళు again;
	}
#అయితే_ముగింపు
	o_కూడు_Qchr(dest, ch);
	ఒకవేళ (ch == SPECIAL_VAR_చిహ్నం) {
		/* Convert "^C" to corresponding special variable reference */
		o_addchr(dest, SPECIAL_VAR_QUOTED_SVS);
		o_addchr(dest, SPECIAL_VAR_చిహ్నం);
	}
	వెళ్ళు again;
#నిర్వచనం_రద్దు as_string
}

/*
 * Scan input until EOF or end_trigger char.
 * Return a list of pipes to execute, or NULL on EOF
 * or if end_trigger character is met.
 * On syntax error, exit if shell is not interactive,
 * reset parsing machinery and start parsing anew,
 * or return ERR_PTR.
 */
స్థిర నిర్మాణం pipe *parse_ప్రవాహం(అక్షరం **pstring,
		పూర్ణాంకం *heredoc_cnt_ptr,
		నిర్మాణం లోపల_str *ఆదానం,
		పూర్ణాంకం చివర_trigger)
{
	నిర్మాణం parse_context ctx;
	పూర్ణాంకం heredoc_cnt;

	/* Single-quote triggers a bypass of the main loop until its mate is
	 * found.  When recursing, quote state is passed in via ctx.word.o_expflags.
	 */
	debug_printf_parse("parse_stream entered, end_trigger='%c'\n",
			చివర_trigger ? చివర_trigger : 'X');
	debug_enter();

	initialize_context(&ctx);

	/* If very first arg is "" or '', ctx.word.data may end up NULL.
	 * Preventing this:
	 */
	ctx.మాట.దత్తాంశం = xzalloc(1); /* start as "", not as NULL */

	/* We used to separate words on $IFS here. This was wrong.
	 * $IFS is used only for word splitting when $var is expanded,
	 * here we should use blank chars as separators, not $IFS
	 */

	heredoc_cnt = 0;
	అయితే (1) {
		స్థిరరాశి అక్షరం *is_blank;
		స్థిరరాశి అక్షరం *is_special;
		పూర్ణాంకం ch;
		పూర్ణాంకం తదుపరి;
#నిర్వచితం_కాకపోతే __U_BOOT__
		పూర్ణాంకం redir_fd;
		redir_type redir_style;
#అయితే_ముగింపు /* !__U_BOOT__ */

		ch = i_getch(ఆదానం);
		debug_printf_parse(": ch=%c (%d) escape=%d\n",
				ch, ch, !!(ctx.మాట.o_expflags & EXP_FLAG_ESC_GLOB_CHARS));
		ఒకవేళ (ch == EOF) {
			నిర్మాణం pipe *pi;

			ఒకవేళ (heredoc_cnt) {
				syntax_error_unterm_str("here document");
				వెళ్ళు parse_error_exitcode1;
			}
			ఒకవేళ (చివర_trigger == ')') {
				syntax_error_unterm_ch('(');
				వెళ్ళు parse_error_exitcode1;
			}
			ఒకవేళ (చివర_trigger == '}') {
				syntax_error_unterm_ch('{');
				వెళ్ళు parse_error_exitcode1;
			}

			ఒకవేళ (పూర్తి_మాట(&ctx)) {
				వెళ్ళు parse_error_exitcode1;
			}
			o_free_and_సమితి_NULL(&ctx.మాట);
			పూర్తి_pipe(&ctx, PIPE_SEQ);

			/* Do we sit inside of any if's, loops or case's? */
			ఒకవేళ (HAS_KEYWORDS
			IF_HAS_KEYWORDS(&& (ctx.ctx_res_w != RES_NONE || ctx.old_flag != 0))
			) {
				syntax_error_unterm_str("compound statement");
				వెళ్ళు parse_error_exitcode1;
			}

			pi = ctx.list_తల;
			/* If we got nothing... */
			/* (this makes bare "&" cmd a no-op.
			 * bash says: "syntax error near unexpected token '&'") */
			ఒకవేళ (pi->అంకె_cmds == 0
			IF_HAS_KEYWORDS(&& pi->res_మాట == RES_NONE)
			) {
				free_pipe_list(pi);
				pi = ఖాళీ;
			}
#ఒకవేళ !BB_MMU
			debug_printf_parse("as_string1 '%s'\n", ctx.as_string.దత్తాంశం);
			ఒకవేళ (pstring)
				*pstring = ctx.as_string.దత్తాంశం;
			లేకపోతే
				o_free(&ctx.as_string);
#అయితే_ముగింపు
			// heredoc_cnt must be 0 here anyway
			//if (heredoc_cnt_ptr)
			//	*heredoc_cnt_ptr = heredoc_cnt;
			debug_leave();
			debug_printf_heredoc("parse_stream return heredoc_cnt:%d\n", heredoc_cnt);
			debug_printf_parse("parse_stream return %p: EOF\n", pi);
			తిరిగి pi;
		}

		/* Handle "'" and "\" first, as they won't play nice with
		 * i_peek_and_eat_bkslash_nl() anyway:
		 *   echo z\\
		 * and
		 *   echo '\
		 *   '
		 * would break.
		 */
		ఒకవేళ (ch == '\\') {
			ch = i_getch(ఆదానం);
			ఒకవేళ (ch == '\n')
				కొనసాగు; /* drop \<newline>, get next char */
			nommu_addchr(&ctx.as_string, '\\');
			ఒకవేళ (ch == SPECIAL_VAR_చిహ్నం) {
				nommu_addchr(&ctx.as_string, ch);
				/* Convert \^C to corresponding special variable reference */
				వెళ్ళు case_SPECIAL_VAR_చిహ్నం;
			}
			o_addchr(&ctx.మాట, '\\');
			ఒకవేళ (ch == EOF) {
				/* Testcase: eval 'echo Ok\' */
				/* bash-4.3.43 was removing backslash,
				 * but 4.4.19 retains it, most other shells too
				 */
				కొనసాగు; /* get next char */
			}
			/* Example: echo Hello \2>file
			 * we need to know that word 2 is quoted
			 */
			ctx.మాట.has_quoted_part = 1;
			nommu_addchr(&ctx.as_string, ch);
			o_addchr(&ctx.మాట, ch);
			కొనసాగు; /* get next char */
		}
		nommu_addchr(&ctx.as_string, ch);
		ఒకవేళ (ch == '\'') {
			ctx.మాట.has_quoted_part = 1;
			తదుపరి = i_getch(ఆదానం);
#నిర్వచితం_కాకపోతే __U_BOOT__
			ఒకవేళ (తదుపరి == '\'' && !ctx.pending_redirect)
				వెళ్ళు చొప్పించు_ఖాళీలు_quoted_str_marker;
#అయితే_ముగింపు /* !__U_BOOT__ */

			ch = తదుపరి;
			అయితే (1) {
				ఒకవేళ (ch == EOF) {
					syntax_error_unterm_ch('\'');
					వెళ్ళు parse_error_exitcode1;
				}
				nommu_addchr(&ctx.as_string, ch);
				ఒకవేళ (ch == '\'')
					ఆపు;
				ఒకవేళ (ch == SPECIAL_VAR_చిహ్నం) {
					/* Convert raw ^C to corresponding special variable reference */
					o_addchr(&ctx.మాట, SPECIAL_VAR_చిహ్నం);
					o_addchr(&ctx.మాట, SPECIAL_VAR_QUOTED_SVS);
				}
				o_addqchr(&ctx.మాట, ch);
				ch = i_getch(ఆదానం);
			}
			కొనసాగు; /* get next char */
		}

		తదుపరి = '\0';
		ఒకవేళ (ch != '\n')
			తదుపరి = i_peek_and_eat_bkslash_nl(ఆదానం);

		is_special = "{}<>&|();#" /* special outside of "str" */
#నిర్వచితం_కాకపోతే __U_BOOT__
				"$\"" IF_HUSH_TICK("`") /* always special */
#లేకపోతే /* __U_BOOT__ */
				"$\""
#అయితే_ముగింపు /* __U_BOOT__ */
				SPECIAL_VAR_చిహ్నం_STR;
#ఒకవేళ defined(CMD_TEST2_SINGLEWORD_NOGLOB)
		ఒకవేళ (ctx.command->cmd_type == CMD_TEST2_SINGLEWORD_NOGLOB) {
			/* In [[ ]], {}<>&|() are not special */
			is_special += 8;
		} లేకపోతే
#అయితే_ముగింపు
		/* Are { and } special here? */
		ఒకవేళ (ctx.command->argv /* word [word]{... - non-special */
		 || ctx.మాట.నిడివి       /* word{... - non-special */
		 || ctx.మాట.has_quoted_part     /* ""{... - non-special */
		 || (తదుపరి != ';'             /* }; - special */
		    && తదుపరి != ')'           /* }) - special */
		    && తదుపరి != '('           /* {( - special */
		    && తదుపరి != '&'           /* }& and }&& ... - special */
		    && తదుపరి != '|'           /* }|| ... - special */
		    && !strchr(defifs, తదుపరి) /* {word - non-special */
		    )
		) {
			/* They are not special, skip "{}" */
			is_special += 2;
		}
		is_special = strchr(is_special, ch);
		is_blank = strchr(defifs, ch);

		ఒకవేళ (!is_special && !is_blank) { /* ordinary char */
 ordinary_char:
			o_కూడు_Qchr(&ctx.మాట, ch);
			ఒకవేళ ((ctx.is_assignment == MAYBE_ASSIGNMENT
			    || ctx.is_assignment == మాట_IS_KEYWORD)
			 && ch == '='
			 && endofname(ctx.మాట.దత్తాంశం)[0] == '='
			) {
				ctx.is_assignment = DEFINITELY_ASSIGNMENT;
				debug_printf_parse("ctx.is_assignment='%s'\n", assignment_flag[ctx.is_assignment]);
			}
			కొనసాగు;
		}

		ఒకవేళ (is_blank) {
#ఒకవేళ ENABLE_HUSH_LINENO_VAR
/* Case:
 * "while ...; do<whitespace><newline>
 *	cmd ..."
 * would think that "cmd" starts in <whitespace> -
 * i.e., at the previous line.
 * We need to skip all whitespace before newlines.
 */
			అయితే (ch != '\n') {
				తదుపరి = i_peek(ఆదానం);
				ఒకవేళ (తదుపరి != ' ' && తదుపరి != '\t' && తదుపరి != '\n')
					ఆపు; /* next char is not ws */
				ch = i_getch(ఆదానం);
			}
			/* ch == last eaten whitespace char */
#అయితే_ముగింపు
			ఒకవేళ (పూర్తి_మాట(&ctx)) {
				వెళ్ళు parse_error_exitcode1;
			}
			ఒకవేళ (ch == '\n') {
				/* Is this a case when newline is simply ignored?
				 * Some examples:
				 * "cmd | <newline> cmd ..."
				 * "case ... in <newline> word) ..."
				 */
				ఒకవేళ (IS_NULL_CMD(ctx.command)
				 && ctx.మాట.నిడివి == 0
				 && !ctx.మాట.has_quoted_part
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
					నిర్మాణం pipe *pi = ctx.list_తల;
					ఒకవేళ (pi->అంకె_cmds != 0       /* check #1 */
					 && pi->followup != PIPE_BG /* check #2 */
					) {
						కొనసాగు;
					}
				}
				/* Treat newline as a command separator. */
				పూర్తి_pipe(&ctx, PIPE_SEQ);
				debug_printf_heredoc("heredoc_cnt:%d\n", heredoc_cnt);
				ఒకవేళ (heredoc_cnt) {
					heredoc_cnt = fetch_heredocs(&ctx.as_string, ctx.list_తల, heredoc_cnt, ఆదానం);
					ఒకవేళ (heredoc_cnt != 0)
						వెళ్ళు parse_error_exitcode1;
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
#నిర్వచితం_కాకపోతే __U_BOOT__
		ఒకవేళ (ch == '}') {
#లేకపోతే /* __U_BOOT__ */
		ఒకవేళ (ch == '}' || ch == ')') {
#అయితే_ముగింపు /* __U_BOOT__ */
			ఒకవేళ (ctx.మాట.నిడివి != 0 /* word} */
			 || ctx.మాట.has_quoted_part    /* ""} */
			) {
				వెళ్ళు ordinary_char;
			}
			ఒకవేళ (!IS_NULL_CMD(ctx.command)) { /* cmd } */
				/* Generally, there should be semicolon: "cmd; }"
				 * However, bash allows to omit it if "cmd" is
				 * a group. Examples:
				 * { { echo 1; } }
				 * {(echo 1)}
				 * { echo 0 >&2 | { echo 1; } }
				 * { while false; do :; done }
				 * { case a in b) ;; esac }
				 */
				ఒకవేళ (ctx.command->సమూహం)
					వెళ్ళు పదం_సమూహం;
				వెళ్ళు ordinary_char;
			}
			ఒకవేళ (!IS_NULL_PIPE(ctx.pipe)) /* cmd | } */
				/* Can't be an end of {cmd}, skip the check */
				వెళ్ళు skip_చివర_trigger;
			/* else: } does terminate a group */
		}
 పదం_సమూహం:
		ఒకవేళ (చివర_trigger && చివర_trigger == ch
		 && (ch != ';' || heredoc_cnt == 0)
#ఒకవేళ ENABLE_HUSH_CASE
		 && (ch != ')'
		    || ctx.ctx_res_w != RES_సరిపోలిక
		    || (!ctx.మాట.has_quoted_part && strcmp(ctx.మాట.దత్తాంశం, "esac") == 0)
		    )
#అయితే_ముగింపు
		) {
			ఒకవేళ (పూర్తి_మాట(&ctx)) {
				వెళ్ళు parse_error_exitcode1;
			}
			పూర్తి_pipe(&ctx, PIPE_SEQ);
			ctx.is_assignment = MAYBE_ASSIGNMENT;
			debug_printf_parse("ctx.is_assignment='%s'\n", assignment_flag[ctx.is_assignment]);
			/* Do we sit outside of any if's, loops or case's? */
			ఒకవేళ (!HAS_KEYWORDS
			IF_HAS_KEYWORDS(|| (ctx.ctx_res_w == RES_NONE && ctx.old_flag == 0))
			) {
				o_free_and_సమితి_NULL(&ctx.మాట);
#ఒకవేళ !BB_MMU
				debug_printf_parse("as_string2 '%s'\n", ctx.as_string.దత్తాంశం);
				ఒకవేళ (pstring)
					*pstring = ctx.as_string.దత్తాంశం;
				లేకపోతే
					o_free(&ctx.as_string);
#అయితే_ముగింపు
				ఒకవేళ (ch != ';' && IS_NULL_PIPE(ctx.list_తల)) {
					/* Example: bare "{ }", "()" */
					G.చివరి_exitcode = 2; /* bash compat */
					syntax_error_unexpected_ch(ch);
					వెళ్ళు parse_error;
				}
				ఒకవేళ (heredoc_cnt_ptr)
					*heredoc_cnt_ptr = heredoc_cnt;
				debug_printf_heredoc("parse_stream return heredoc_cnt:%d\n", heredoc_cnt);
				debug_printf_parse("parse_stream return %p: "
						"end_trigger char found\n",
						ctx.list_తల);
				debug_leave();
				తిరిగి ctx.list_తల;
			}
		}

		ఒకవేళ (is_blank)
			కొనసాగు;

		/* Catch <, > before deciding whether this word is
		 * an assignment. a=1 2>z b=2: b=2 is still assignment */
		ఎంపిక (ch) {
#నిర్వచితం_కాకపోతే __U_BOOT__
		స్థితి '>':
			redir_fd = redirect_opt_అంకె(&ctx.మాట);
			ఒకవేళ (పూర్తి_మాట(&ctx)) {
				వెళ్ళు parse_error_exitcode1;
			}
			redir_style = REDIRECT_OVERWRITE;
			ఒకవేళ (తదుపరి == '>') {
				redir_style = REDIRECT_APPEND;
				ch = i_getch(ఆదానం);
				nommu_addchr(&ctx.as_string, ch);
			}
#ఒకవేళ 0
			లేకపోతే ఒకవేళ (తదుపరి == '(') {
				syntax_error(">(process) not supported");
				వెళ్ళు parse_error_exitcode1;
			}
#అయితే_ముగింపు
			ఒకవేళ (parse_redirect(&ctx, redir_fd, redir_style, ఆదానం))
				వెళ్ళు parse_error_exitcode1;
			కొనసాగు; /* get next char */
		స్థితి '<':
			redir_fd = redirect_opt_అంకె(&ctx.మాట);
			ఒకవేళ (పూర్తి_మాట(&ctx)) {
				వెళ్ళు parse_error_exitcode1;
			}
			redir_style = REDIRECT_ఆదానం;
			ఒకవేళ (తదుపరి == '<') {
				redir_style = REDIRECT_HEREDOC;
				heredoc_cnt++;
				debug_printf_heredoc("++heredoc_cnt=%d\n", heredoc_cnt);
				ch = i_getch(ఆదానం);
				nommu_addchr(&ctx.as_string, ch);
			} లేకపోతే ఒకవేళ (తదుపరి == '>') {
				redir_style = REDIRECT_IO;
				ch = i_getch(ఆదానం);
				nommu_addchr(&ctx.as_string, ch);
			}
#ఒకవేళ 0
			లేకపోతే ఒకవేళ (తదుపరి == '(') {
				syntax_error("<(process) not supported");
				వెళ్ళు parse_error_exitcode1;
			}
#అయితే_ముగింపు
			ఒకవేళ (parse_redirect(&ctx, redir_fd, redir_style, ఆదానం))
				వెళ్ళు parse_error_exitcode1;
			కొనసాగు; /* get next char */
#లేకపోతే /* __U_BOOT__ */
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
			స్థితి '>':
				fallthrough;
			స్థితి '<':
				o_కూడు_Qchr(&ctx.మాట, ch);
				కొనసాగు;
#అయితే_ముగింపు /* __U_BOOT__ */
		స్థితి '#':
			ఒకవేళ (ctx.మాట.నిడివి == 0 && !ctx.మాట.has_quoted_part) {
				/* skip "#comment" */
				/* note: we do not add it to &ctx.as_string */
/* TODO: in bash:
 * comment inside $() goes to the next \n, even inside quoted string (!):
 * cmd "$(cmd2 #comment)" - syntax error
 * cmd "`cmd2 #comment`" - ok
 * We accept both (comment ends where command subst ends, in both cases).
 */
				అయితే (1) {
					ch = i_peek(ఆదానం);
					ఒకవేళ (ch == '\n') {
						nommu_addchr(&ctx.as_string, '\n');
						ఆపు;
					}
					ch = i_getch(ఆదానం);
					ఒకవేళ (ch == EOF)
						ఆపు;
				}
				కొనసాగు; /* get next char */
			}
			ఆపు;
		}
 skip_చివర_trigger:

		ఒకవేళ (ctx.is_assignment == MAYBE_ASSIGNMENT
#నిర్వచితం_కాకపోతే __U_BOOT__
		 /* check that we are not in word in "a=1 2>word b=1": */
		 && !ctx.pending_redirect
#అయితే_ముగింపు /* !__U_BOOT__ */
		) {
			/* ch is a special char and thus this word
			 * cannot be an assignment */
			ctx.is_assignment = NOT_ASSIGNMENT;
			debug_printf_parse("ctx.is_assignment='%s'\n", assignment_flag[ctx.is_assignment]);
		}

		/* Note: nommu_addchr(&ctx.as_string, ch) is already done */

		ఎంపిక (ch) {
		case_SPECIAL_VAR_చిహ్నం:
		స్థితి SPECIAL_VAR_చిహ్నం:
			/* Convert raw ^C to corresponding special variable reference */
			o_addchr(&ctx.మాట, SPECIAL_VAR_చిహ్నం);
			o_addchr(&ctx.మాట, SPECIAL_VAR_QUOTED_SVS);
			/* fall through */
		స్థితి '#':
			/* non-comment #: "echo a#b" etc */
			o_addchr(&ctx.మాట, ch);
			కొనసాగు; /* get next char */
		స్థితి '$':
#నిర్వచితం_కాకపోతే __U_BOOT__
			ఒకవేళ (parse_dollar_squote(&ctx.as_string, &ctx.మాట, ఆదానం))
				కొనసాగు; /* get next char */
#అయితే_ముగింపు /* !__U_BOOT__ */
			ఒకవేళ (!parse_dollar(&ctx.as_string, &ctx.మాట, ఆదానం, /*quote_mask:*/ 0)) {
				debug_printf_parse("parse_stream parse error: "
					"parse_dollar returned 0 (error)\n");
				వెళ్ళు parse_error_exitcode1;
			}
			కొనసాగు; /* get next char */
		స్థితి '"':
			ctx.మాట.has_quoted_part = 1;
#నిర్వచితం_కాకపోతే __U_BOOT__
			ఒకవేళ (తదుపరి == '"' && !ctx.pending_redirect) {
#లేకపోతే /* __U_BOOT__ */
			ఒకవేళ (తదుపరి == '"') {
#అయితే_ముగింపు /* __U_BOOT__ */
				i_getch(ఆదానం); /* eat second " */
#నిర్వచితం_కాకపోతే __U_BOOT__
 చొప్పించు_ఖాళీలు_quoted_str_marker:
#అయితే_ముగింపు /* !__U_BOOT__ */
				nommu_addchr(&ctx.as_string, తదుపరి);
				o_addchr(&ctx.మాట, SPECIAL_VAR_చిహ్నం);
				o_addchr(&ctx.మాట, SPECIAL_VAR_చిహ్నం);
				కొనసాగు; /* get next char */
			}
			ఒకవేళ (ctx.is_assignment == NOT_ASSIGNMENT)
				ctx.మాట.o_expflags |= EXP_FLAG_ESC_GLOB_CHARS;
			ఒకవేళ (!encode_string(&ctx.as_string, &ctx.మాట, ఆదానం, '"'))
				వెళ్ళు parse_error_exitcode1;
			ctx.మాట.o_expflags &= ~EXP_FLAG_ESC_GLOB_CHARS;
			కొనసాగు; /* get next char */
#ఒకవేళ ENABLE_HUSH_TICK
		స్థితి '`': {
			USE_FOR_NOMMU(గుర్తులేని pos;)

			o_addchr(&ctx.మాట, SPECIAL_VAR_చిహ్నం);
			o_addchr(&ctx.మాట, '`');
			USE_FOR_NOMMU(pos = ctx.మాట.నిడివి;)
			ఒకవేళ (!కూడు_till_backquote(&ctx.మాట, ఆదానం, /*in_dquote:*/ 0))
				వెళ్ళు parse_error_exitcode1;
# ఒకవేళ !BB_MMU
			o_addstr(&ctx.as_string, ctx.మాట.దత్తాంశం + pos);
			o_addchr(&ctx.as_string, '`');
# endif
			o_addchr(&ctx.మాట, SPECIAL_VAR_చిహ్నం);
			//debug_printf_subst("SUBST RES3 '%s'\n", ctx.word.data + pos);
			కొనసాగు; /* get next char */
		}
#అయితే_ముగింపు
		స్థితి ';':
#ఒకవేళ ENABLE_HUSH_CASE
 case_semi:
#అయితే_ముగింపు
			ఒకవేళ (పూర్తి_మాట(&ctx)) {
				వెళ్ళు parse_error_exitcode1;
			}
			పూర్తి_pipe(&ctx, PIPE_SEQ);
#ఒకవేళ ENABLE_HUSH_CASE
			/* Eat multiple semicolons, detect
			 * whether it means something special */
			అయితే (1) {
				ch = i_peek_and_eat_bkslash_nl(ఆదానం);
				ఒకవేళ (ch != ';')
					ఆపు;
				ch = i_getch(ఆదానం);
				nommu_addchr(&ctx.as_string, ch);
				ఒకవేళ (ctx.ctx_res_w == RES_CASE_BODY) {
					ctx.ctx_dsemicolon = 1;
					ctx.ctx_res_w = RES_సరిపోలిక;
					ఆపు;
				}
			}
#అయితే_ముగింపు
 new_cmd:
			/* We just finished a cmd. New one may start
			 * with an assignment */
			ctx.is_assignment = MAYBE_ASSIGNMENT;
			debug_printf_parse("ctx.is_assignment='%s'\n", assignment_flag[ctx.is_assignment]);
			కొనసాగు; /* get next char */
		స్థితి '&':
			ఒకవేళ (పూర్తి_మాట(&ctx)) {
				వెళ్ళు parse_error_exitcode1;
			}
			ఒకవేళ (తదుపరి == '&') {
				ch = i_getch(ఆదానం);
				nommu_addchr(&ctx.as_string, ch);
				పూర్తి_pipe(&ctx, PIPE_AND);
			} లేకపోతే {
				పూర్తి_pipe(&ctx, PIPE_BG);
			}
			వెళ్ళు new_cmd;
		స్థితి '|':
			ఒకవేళ (పూర్తి_మాట(&ctx)) {
				వెళ్ళు parse_error_exitcode1;
			}
#ఒకవేళ ENABLE_HUSH_CASE
			ఒకవేళ (ctx.ctx_res_w == RES_సరిపోలిక)
				ఆపు; /* we are in case's "word | word)" */
#అయితే_ముగింపు
			ఒకవేళ (తదుపరి == '|') { /* || */
				ch = i_getch(ఆదానం);
				nommu_addchr(&ctx.as_string, ch);
				పూర్తి_pipe(&ctx, PIPE_OR);
			} లేకపోతే {
				/* we could pick up a file descriptor choice here
				 * with redirect_opt_num(), but bash doesn't do it.
				 * "echo foo 2| cat" yields "foo 2". */
				పూర్తి_command(&ctx);
			}
			వెళ్ళు new_cmd;
		స్థితి '(':
#ఒకవేళ ENABLE_HUSH_CASE
			/* "case... in [(]word)..." - skip '(' */
			ఒకవేళ (ctx.ctx_res_w == RES_సరిపోలిక
			 && ctx.command->argv == ఖాళీ /* not (word|(... */
			 && ctx.మాట.నిడివి == 0 /* not word(... */
			 && ctx.మాట.has_quoted_part == 0 /* not ""(... */
			) {
				కొనసాగు; /* get next char */
			}
#అయితే_ముగింపు
			/* fall through */
		స్థితి '{': {
			పూర్ణాంకం n = parse_సమూహం(&ctx, ఆదానం, ch);
			ఒకవేళ (n < 0) {
				వెళ్ళు parse_error_exitcode1;
			}
			debug_printf_heredoc("parse_group done, needs heredocs:%d\n", n);
			heredoc_cnt += n;
			వెళ్ళు new_cmd;
		}
		స్థితి ')':
#ఒకవేళ ENABLE_HUSH_CASE
			ఒకవేళ (ctx.ctx_res_w == RES_సరిపోలిక)
				వెళ్ళు case_semi;
#అయితే_ముగింపు
		స్థితి '}':
			/* proper use of this character is caught by end_trigger:
			 * if we see {, we call parse_group(..., end_trigger='}')
			 * and it will match } earlier (not here). */
			G.చివరి_exitcode = 2;
			syntax_error_unexpected_ch(ch);
			వెళ్ళు parse_error;
		అప్రమేయ:
			ఒకవేళ (HUSH_DEBUG)
				bb_error_సందేశం_and_die("BUG: unexpected %c", ch);
		}
	} /* while (1) */

 parse_error_exitcode1:
	G.చివరి_exitcode = 1;
 parse_error:
	{
		నిర్మాణం parse_context *pctx;
		IF_HAS_KEYWORDS(నిర్మాణం parse_context *p2;)

		/* Clean up allocated tree.
		 * Sample for finding leaks on syntax error recovery path.
		 * Run it from interactive shell, watch pmap `pidof hush`.
		 * while if false; then false; fi; do break; fi
		 * Samples to catch leaks at execution:
		 * while if (true | { true;}); then echo ok; fi; do break; done
		 * while if (true | { true;}); then echo ok; fi; do (if echo ok; break; then :; fi) | cat; break; done
		 */
		pctx = &ctx;
		చేయి {
			/* Update pipe/command counts,
			 * otherwise freeing may miss some */
			పూర్తి_pipe(pctx, PIPE_SEQ);
			debug_printf_clean("freeing list %p from ctx %p\n",
					pctx->list_తల, pctx);
			debug_print_వృక్షం(pctx->list_తల, 0);
			free_pipe_list(pctx->list_తల);
			debug_printf_clean("freed list %p\n", pctx->list_తల);
#ఒకవేళ !BB_MMU
			o_free(&pctx->as_string);
#అయితే_ముగింపు
			IF_HAS_KEYWORDS(p2 = pctx->దొంతర;)
			ఒకవేళ (pctx != &ctx) {
				విడుదల(pctx);
			}
			IF_HAS_KEYWORDS(pctx = p2;)
		} అయితే (HAS_KEYWORDS && pctx);

		o_free(&ctx.మాట);
#ఒకవేళ !BB_MMU
		ఒకవేళ (pstring)
			*pstring = ఖాళీ;
#అయితే_ముగింపు
		debug_leave();
		తిరిగి ERR_PTR;
	}
}

/*** Execution routines ***/

/* Expansion can recurse, need forward decls: */
#ఒకవేళ !BASH_నమూనా_SUBST && !ENABLE_HUSH_CASE
#నిర్వచించు expand_string_గమ్యం_string(str, EXP_flags, do_unbackslash) \
	expand_string_గమ్యం_string(str)
#అయితే_ముగింపు
స్థిర అక్షరం *expand_string_గమ్యం_string(స్థిరరాశి అక్షరం *str, పూర్ణాంకం EXP_flags, పూర్ణాంకం do_unbackslash);
#ఒకవేళ ENABLE_HUSH_TICK
స్థిర పూర్ణాంకం process_command_subs(o_string *dest, స్థిరరాశి అక్షరం *s);
#అయితే_ముగింపు
స్థిర పూర్ణాంకం expand_vars_గమ్యం_list(o_string *output, పూర్ణాంకం n, అక్షరం *arg);

/* expand_strvec_to_strvec() takes a list of strings, expands
 * all variable references within and returns a pointer to
 * a list of expanded strings, possibly with larger number
 * of strings. (Think VAR="a b"; echo $VAR).
 * This new list is allocated as a single malloc block.
 * NULL-terminated list of char* pointers is at the beginning of it,
 * followed by strings themselves.
 * Caller can deallocate entire list by single free(list). */

/* A horde of its helpers come first: */

స్థిర శూన్య o_addblock_duplicate_backslash(o_string *o, స్థిరరాశి అక్షరం *str, పూర్ణాంకం len)
{
	అయితే (--len >= 0) {
		అక్షరం c = *str++;

#ఒకవేళ ENABLE_HUSH_BRACE_EXPANSION
		ఒకవేళ (c == '{' || c == '}') {
			/* { -> \{, } -> \} */
			o_addchr(o, '\\');
			/* And now we want to add { or } and continue:
			 *  o_addchr(o, c);
			 *  continue;
			 * luckily, just falling through achieves this.
			 */
		}
#అయితే_ముగింపు
		o_addchr(o, c);
		ఒకవేళ (c == '\\') {
			/* \z -> \\\z; \<eol> -> \\<eol> */
			o_addchr(o, '\\');
			ఒకవేళ (len) {
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
స్థిర పూర్ణాంకం expand_on_ifs(o_string *output, పూర్ణాంకం n, స్థిరరాశి అక్షరం *str)
{
	పూర్ణాంకం చివరి_is_ifs = 0;

	అయితే (1) {
		పూర్ణాంకం మాట_len;

		ఒకవేళ (!*str)  /* EOL - do not finalize word */
			ఆపు;
		మాట_len = strcspn(str, G.ifs);
		ఒకవేళ (మాట_len) {
			/* We have WORD_LEN leading non-IFS chars */
			ఒకవేళ (!(output->o_expflags & EXP_FLAG_GLOB)) {
				o_addblock(output, str, మాట_len);
			} లేకపోతే {
				/* Protect backslashes against globbing up :)
				 * Example: "v='\*'; echo b$v" prints "b\*"
				 * (and does not try to glob on "*")
				 */
				o_addblock_duplicate_backslash(output, str, మాట_len);
				/*/ Why can't we do it easier? */
				/*o_addblock(output, str, word_len); - WRONG: "v='\*'; echo Z$v" prints "Z*" instead of "Z\*" */
				/*o_addqblock(output, str, word_len); - WRONG: "v='*'; echo Z$v" prints "Z*" instead of Z* files */
			}
			చివరి_is_ifs = 0;
			str += మాట_len;
			ఒకవేళ (!*str)  /* EOL - do not finalize word */
				ఆపు;
		}

		/* We know str here points to at least one IFS char */
		చివరి_is_ifs = 1;
		str += strspn(str, G.ifs_whitespace); /* skip IFS whitespace chars */
		ఒకవేళ (!*str)  /* EOL - do not finalize word */
			ఆపు;

		ఒకవేళ (G.ifs_whitespace != G.ifs /* usually false ($IFS is usually all whitespace), */
		 && strchr(G.ifs, *str)       /* the second check would fail */
		) {
			/* This is a non-whitespace $IFS char */
			/* Skip it and IFS whitespace chars, start new word */
			str++;
			str += strspn(str, G.ifs_whitespace);
			వెళ్ళు new_మాట;
		}

		/* Start new word... but not always! */
		/* Case "v=' a'; echo ''$v": we do need to finalize empty word: */
		ఒకవేళ (output->has_quoted_part
		/*
		 * Case "v=' a'; echo $v":
		 * here nothing precedes the space in $v expansion,
		 * therefore we should not finish the word
		 * (IOW: if there *is* word to finalize, only then do it):
		 * It's okay if this accesses the byte before first argv[]:
		 * past call to o_save_ptr() cleared it to zero byte
		 * (grep for -prev-ifs-check-).
		 */
		 || output->దత్తాంశం[output->నిడివి - 1]
		) {
 new_మాట:
			o_addchr(output, '\0');
			debug_print_list("expand_on_ifs", output, n);
			n = o_save_ptr(output, n);
		}
	}

	output->ended_లోపల_ifs = చివరి_is_ifs;
	debug_print_list("expand_on_ifs[1]", output, n);
	తిరిగి n;
}

#నిర్వచితం_కాకపోతే __U_BOOT__
/* Helper to expand $((...)) and heredoc body. These act as if
 * they are in double quotes, with the exception that they are not :).
 * Just the rules are similar: "expand only $var and `cmd`"
 *
 * Returns malloced string.
 * As an optimization, we return NULL if expansion is not needed.
 */
స్థిర అక్షరం *encode_then_expand_string(స్థిరరాశి అక్షరం *str)
{
	అక్షరం *exp_str;
	నిర్మాణం లోపల_str ఆదానం;
	o_string dest = NULL_O_STRING;
	స్థిరరాశి అక్షరం *cp;

	cp = str;
	కొరకు (;;) {
		ఒకవేళ (!*cp) తిరిగి ఖాళీ; /* string has no special chars */
		ఒకవేళ (*cp == '$') ఆపు;
		ఒకవేళ (*cp == '\\') ఆపు;
#ఒకవేళ ENABLE_HUSH_TICK
		ఒకవేళ (*cp == '`') ఆపు;
#అయితే_ముగింపు
		cp++;
	}

	/* We need to expand. Example:
	 * echo $(($a + `echo 1`)) $((1 + $((2)) ))
	 */
	అమరిక_string_లోపల_str(&ఆదానం, str);
	encode_string(ఖాళీ, &dest, &ఆదానం, EOF);
//TODO: error check (encode_string returns 0 on error)?
	//bb_error_msg("'%s' -> '%s'", str, dest.data);
	exp_str = expand_string_గమ్యం_string(dest.దత్తాంశం,
			EXP_FLAG_ESC_GLOB_CHARS,
			/*unbackslash:*/ 1
	);
	//bb_error_msg("'%s' -> '%s'", dest.data, exp_str);
	o_free(&dest);
	తిరిగి exp_str;
}

స్థిర స్థిరరాశి అక్షరం *మొదటి_special_char_లోపల_vararg(స్థిరరాశి అక్షరం *cp)
{
	కొరకు (;;) {
		ఒకవేళ (!*cp) తిరిగి ఖాళీ; /* string has no special chars */
		ఒకవేళ (*cp == '$') తిరిగి cp;
		ఒకవేళ (*cp == '\\') తిరిగి cp;
		ఒకవేళ (*cp == '\'') తిరిగి cp;
		ఒకవేళ (*cp == '"') తిరిగి cp;
#ఒకవేళ ENABLE_HUSH_TICK
		ఒకవేళ (*cp == '`') తిరిగి cp;
#అయితే_ముగింపు
		/* dquoted "${x:+ARG}" should not glob, therefore
		 * '*' et al require some non-literal processing: */
		ఒకవేళ (*cp == '*') తిరిగి cp;
		ఒకవేళ (*cp == '?') తిరిగి cp;
		ఒకవేళ (*cp == '[') తిరిగి cp;
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
#ఒకవేళ !BASH_నమూనా_SUBST
#నిర్వచించు encode_then_expand_vararg(str, handle_squotes, do_unbackslash) \
	encode_then_expand_vararg(str, handle_squotes)
#అయితే_ముగింపు
స్థిర అక్షరం *encode_then_expand_vararg(స్థిరరాశి అక్షరం *str, పూర్ణాంకం handle_squotes, పూర్ణాంకం do_unbackslash)
{
#ఒకవేళ !BASH_నమూనా_SUBST && ENABLE_HUSH_CASE
	స్థిరరాశి పూర్ణాంకం do_unbackslash = 0;
#అయితే_ముగింపు
	అక్షరం *exp_str;
	నిర్మాణం లోపల_str ఆదానం;
	o_string dest = NULL_O_STRING;

	ఒకవేళ (!మొదటి_special_char_లోపల_vararg(str)) {
		/* string has no special chars */
		తిరిగి ఖాళీ;
	}

	అమరిక_string_లోపల_str(&ఆదానం, str);
	dest.దత్తాంశం = xzalloc(1); /* start as "", not as NULL */
	exp_str = ఖాళీ;

	కొరకు (;;) {
		పూర్ణాంకం ch;

		ch = i_getch(&ఆదానం);
		debug_printf_parse("%s: ch=%c (%d) escape=%d\n",
				__func__, ch, ch, !!dest.o_expflags);

		ఒకవేళ (!dest.o_expflags) {
			ఒకవేళ (ch == EOF)
				ఆపు;
			ఒకవేళ (handle_squotes && ch == '\'') {
				ఒకవేళ (!కూడు_till_single_quote_dquoted(&dest, &ఆదానం))
					వెళ్ళు ret; /* error */
				కొనసాగు;
			}
		}
		ఒకవేళ (ch == EOF) {
			syntax_error_unterm_ch('"');
			వెళ్ళు ret; /* error */
		}
		ఒకవేళ (ch == '"') {
			dest.o_expflags ^= EXP_FLAG_ESC_GLOB_CHARS;
			కొనసాగు;
		}
		ఒకవేళ (ch == '\\') {
			ch = i_getch(&ఆదానం);
			ఒకవేళ (ch == EOF) {
//example? error message?	syntax_error_unterm_ch('"');
				debug_printf_parse("%s: error: \\<eof>\n", __func__);
				వెళ్ళు ret;
			}
			o_addqchr(&dest, ch);
			కొనసాగు;
		}
		ఒకవేళ (ch == '$') {
			ఒకవేళ (parse_dollar_squote(ఖాళీ, &dest, &ఆదానం))
				కొనసాగు;
			ఒకవేళ (!parse_dollar(ఖాళీ, &dest, &ఆదానం, /*quote_mask:*/ 0x80)) {
				debug_printf_parse("%s: error: parse_dollar returned 0 (error)\n", __func__);
				వెళ్ళు ret;
			}
			కొనసాగు;
		}
#ఒకవేళ ENABLE_HUSH_TICK
		ఒకవేళ (ch == '`') {
			//unsigned pos = dest->length;
			o_addchr(&dest, SPECIAL_VAR_చిహ్నం);
			o_addchr(&dest, 0x80 | '`');
			ఒకవేళ (!కూడు_till_backquote(&dest, &ఆదానం,
					/*in_dquote:*/ dest.o_expflags /* nonzero if EXP_FLAG_ESC_GLOB_CHARS set */
				)
			) {
				వెళ్ళు ret; /* error */
			}
			o_addchr(&dest, SPECIAL_VAR_చిహ్నం);
			//debug_printf_subst("SUBST RES3 '%s'\n", dest->data + pos);
			కొనసాగు;
		}
#అయితే_ముగింపు
		o_కూడు_Qchr(&dest, ch);
	} /* for (;;) */

	debug_printf_parse("encode: '%s' -> '%s'\n", str, dest.దత్తాంశం);
	exp_str = expand_string_గమ్యం_string(dest.దత్తాంశం,
			do_unbackslash ? EXP_FLAG_ESC_GLOB_CHARS : 0,
			do_unbackslash
	);
 ret:
	debug_printf_parse("expand: '%s' -> '%s'\n", dest.దత్తాంశం, exp_str);
	o_free(&dest);
	తిరిగి exp_str;
}

/* Expanding ARG in ${var+ARG}, ${var-ARG}
 */
స్థిర NOINLINE పూర్ణాంకం encode_then_append_var_plusminus(o_string *output, పూర్ణాంకం n,
		అక్షరం *str, పూర్ణాంకం dquoted)
{
	నిర్మాణం లోపల_str ఆదానం;
	o_string dest = NULL_O_STRING;

	ఒకవేళ (!మొదటి_special_char_లోపల_vararg(str)
	 && '\0' == str[strcspn(str, G.ifs)]
	) {
		/* string has no special chars
		 * && string has no $IFS chars
		 */
		ఒకవేళ (dquoted) {
			/* Prints 1 (quoted expansion is a "" word, not nothing):
			 * set -- "${notexist-}"; echo $#
			 */
			output->has_quoted_part = 1;
		}
		తిరిగి expand_vars_గమ్యం_list(output, n, str);
	}

	అమరిక_string_లోపల_str(&ఆదానం, str);

	కొరకు (;;) {
		పూర్ణాంకం ch;

		ch = i_getch(&ఆదానం);
		debug_printf_parse("%s: ch=%c (%d) escape=%x\n",
				__func__, ch, ch, dest.o_expflags);

		ఒకవేళ (!dest.o_expflags) {
			ఒకవేళ (ch == EOF)
				ఆపు;
			ఒకవేళ (!dquoted && !(output->o_expflags & EXP_FLAG_SINGLEWORD) && strchr(G.ifs, ch)) {
				/* PREFIX${x:d${e}f ...} and we met space: expand "d${e}f" and start new word.
				 * do not assume we are at the start of the word (PREFIX above).
				 */
				ఒకవేళ (dest.దత్తాంశం) {
					n = expand_vars_గమ్యం_list(output, n, dest.దత్తాంశం);
					o_free_and_సమితి_NULL(&dest);
					o_addchr(output, '\0');
					n = o_save_ptr(output, n); /* create next word */
				} లేకపోతే
				ఒకవేళ (output->నిడివి != o_get_చివరి_ptr(output, n)
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
				కొనసాగు;
			}
			ఒకవేళ (!dquoted && ch == '\'') {
				ఒకవేళ (!కూడు_till_single_quote_dquoted(&dest, &ఆదానం))
					వెళ్ళు ret; /* error */
				o_addchr(&dest, SPECIAL_VAR_చిహ్నం);
				o_addchr(&dest, SPECIAL_VAR_చిహ్నం);
				కొనసాగు;
			}
		}
		ఒకవేళ (ch == EOF) {
			syntax_error_unterm_ch('"');
			వెళ్ళు ret; /* error */
		}
		ఒకవేళ (ch == '"') {
			dest.o_expflags ^= EXP_FLAG_ESC_GLOB_CHARS;
			ఒకవేళ (dest.o_expflags) {
				o_addchr(&dest, SPECIAL_VAR_చిహ్నం);
				o_addchr(&dest, SPECIAL_VAR_చిహ్నం);
			}
			కొనసాగు;
		}
		ఒకవేళ (ch == '\\') {
			ch = i_getch(&ఆదానం);
			ఒకవేళ (ch == EOF) {
//example? error message?	syntax_error_unterm_ch('"');
				debug_printf_parse("%s: error: \\<eof>\n", __func__);
				వెళ్ళు ret;
			}
			o_addqchr(&dest, ch);
			కొనసాగు;
		}
		ఒకవేళ (ch == '$') {
			ఒకవేళ (!parse_dollar(ఖాళీ, &dest, &ఆదానం, /*quote_mask:*/ (dest.o_expflags || dquoted) ? 0x80 : 0)) {
				debug_printf_parse("%s: error: parse_dollar returned 0 (error)\n", __func__);
				వెళ్ళు ret;
			}
			కొనసాగు;
		}
#ఒకవేళ ENABLE_HUSH_TICK
		ఒకవేళ (ch == '`') {
			//unsigned pos = dest->length;
			o_addchr(&dest, SPECIAL_VAR_చిహ్నం);
			o_addchr(&dest, (dest.o_expflags || dquoted) ? 0x80 | '`' : '`');
			ఒకవేళ (!కూడు_till_backquote(&dest, &ఆదానం,
					/*in_dquote:*/ dest.o_expflags /* nonzero if EXP_FLAG_ESC_GLOB_CHARS set */
				)
			) {
				వెళ్ళు ret; /* error */
			}
			o_addchr(&dest, SPECIAL_VAR_చిహ్నం);
			//debug_printf_subst("SUBST RES3 '%s'\n", dest->data + pos);
			కొనసాగు;
		}
#అయితే_ముగింపు
		ఒకవేళ (dquoted) {
			/* Always glob-protect if in dquotes:
			 * x=x; echo "${x:+/bin/c*}" - prints: /bin/c*
			 * x=x; echo "${x:+"/bin/c*"}" - prints: /bin/c*
			 */
			o_addqchr(&dest, ch);
		} లేకపోతే {
			/* Glob-protect only if char is quoted:
			 * x=x; echo ${x:+/bin/c*} - prints many filenames
			 * x=x; echo ${x:+"/bin/c*"} - prints: /bin/c*
			 */
			o_కూడు_Qchr(&dest, ch);
		}
	} /* for (;;) */

	ఒకవేళ (dest.దత్తాంశం) {
		n = expand_vars_గమ్యం_list(output, n, dest.దత్తాంశం);
	}
 ret:
	o_free(&dest);
	తిరిగి n;
}
#అయితే_ముగింపు /* !__U_BOOT__ */

#నిర్వచితం_కాకపోతే __U_BOOT__
#ఒకవేళ ENABLE_FEATURE_SH_MATH
స్థిర arith_t expand_and_evaluate_arith(స్థిరరాశి అక్షరం *arg, స్థిరరాశి అక్షరం **errmsg_p)
{
	arith_state_t math_state;
	arith_t res;
	అక్షరం *exp_str;

	math_state.lookupvar = get_local_var_విలువ;
	math_state.setvar = సమితి_local_var_from_halves;
	//math_state.endofname = endofname;
	exp_str = encode_then_expand_string(arg);
	res = arith(&math_state, exp_str ? exp_str : arg);
	విడుదల(exp_str);
	ఒకవేళ (errmsg_p)
		*errmsg_p = math_state.errmsg;
	ఒకవేళ (math_state.errmsg)
		సందేశం_and_die_if_script(math_state.errmsg);
	తిరిగి res;
}
#అయితే_ముగింపు
#అయితే_ముగింపు /* !__U_BOOT__ */

#నిర్వచితం_కాకపోతే __U_BOOT__
#ఒకవేళ BASH_నమూనా_SUBST
/* ${var/[/]pattern[/repl]} helpers */
స్థిర అక్షరం *strstr_నమూనా(అక్షరం *val, స్థిరరాశి అక్షరం *నమూనా, పూర్ణాంకం *కొలత)
{
	పూర్ణాంకం మొదటి_escaped = (నమూనా[0] == '\\' && నమూనా[1]);
	/* "first_escaped" trick allows to treat e.g. "\*no_glob_chars"
	 * as literal too (as it is semi-common, and easy to accomodate
	 * by just using str + 1).
	 */
	పూర్ణాంకం sz = strcspn(నమూనా + మొదటి_escaped * 2, "*?[\\");
	ఒకవేళ ((నమూనా + మొదటి_escaped * 2)[sz] == '\0') {
		/* Optimization for trivial patterns.
		 * Testcase for very slow replace (performs about 22k replaces):
		 * x=::::::::::::::::::::::
		 * x=$x$x;x=$x$x;x=$x$x;x=$x$x;x=$x$x;x=$x$x;x=$x$x;x=$x$x;x=$x$x;x=$x$x;echo ${#x}
		 * echo "${x//:/|}"
		 */
		*కొలత = sz + మొదటి_escaped;
		తిరిగి అక్షరశ్రేణి_శోధన(val, నమూనా + మొదటి_escaped);
	}

	అయితే (1) {
		అక్షరం *చివర_2 = పరిశీలన_and_సరిపోలిక(val, నమూనా, పరిశీలన_MOVE_FROM_కుడి + పరిశీలన_సరిపోలిక_ఎడమ_HALF);
		debug_printf_varexp("val:'%s' pattern:'%s' end:'%s'\n", val, నమూనా, చివర_2);
		ఒకవేళ (చివర_2) {
			*కొలత = చివర_2 - val;
			తిరిగి val;
		}
		ఒకవేళ (*val == '\0')
			తిరిగి ఖాళీ;
		/* Optimization: if "*pat" did not match the start of "string",
		 * we know that "tring", "ring" etc will not match too:
		 */
		ఒకవేళ (నమూనా[0] == '*')
			తిరిగి ఖాళీ;
		val++;
	}
}
స్థిర అక్షరం *replace_నమూనా(అక్షరం *val, స్థిరరాశి అక్షరం *నమూనా, స్థిరరాశి అక్షరం *repl, అక్షరం exp_op)
{
	అక్షరం *ఫలితం = ఖాళీ;
	గుర్తులేని res_len = 0;
	గుర్తులేని repl_len = అక్షరశ్రేణి_పొడవు(repl);

	/* Null pattern never matches, including if "var" is empty */
	ఒకవేళ (!నమూనా[0])
		తిరిగి ఫలితం; /* NULL, no replaces happened */

	అయితే (1) {
		పూర్ణాంకం కొలత;
		అక్షరం *s = strstr_నమూనా(val, నమూనా, &కొలత);
		ఒకవేళ (!s)
			ఆపు;

		ఫలితం = xrealloc(ఫలితం, res_len + (s - val) + repl_len + 1);
		strcpy(mempcpy(ఫలితం + res_len, val, s - val), repl);
		res_len += (s - val) + repl_len;
		debug_printf_varexp("val:'%s' s:'%s' result:'%s'\n", val, s, ఫలితం);

		val = s + కొలత;
		ఒకవేళ (exp_op == '/')
			ఆపు;
	}
	ఒకవేళ (*val && ఫలితం) {
		ఫలితం = xrealloc(ఫలితం, res_len + అక్షరశ్రేణి_పొడవు(val) + 1);
		strcpy(ఫలితం + res_len, val);
		debug_printf_varexp("val:'%s' result:'%s'\n", val, ఫలితం);
	}
	debug_printf_varexp("result:'%s'\n", ఫలితం);
	తిరిగి ఫలితం;
}
#అయితే_ముగింపు /* BASH_PATTERN_SUBST */
#అయితే_ముగింపు /* !__U_BOOT__ */

స్థిర పూర్ణాంకం append_str_maybe_ifs_విభజించు(o_string *output, పూర్ణాంకం n,
		పూర్ణాంకం మొదటి_ch, స్థిరరాశి అక్షరం *val)
{
	ఒకవేళ (!(మొదటి_ch & 0x80)) { /* unquoted $VAR */
		debug_printf_expand("unquoted '%s', output->o_escape:%d\n", val,
				!!(output->o_expflags & EXP_FLAG_ESC_GLOB_CHARS));
		ఒకవేళ (val && val[0])
			n = expand_on_ifs(output, n, val);
	} లేకపోతే { /* quoted "$VAR" */
		output->has_quoted_part = 1;
		debug_printf_expand("quoted '%s', output->o_escape:%d\n", val,
				!!(output->o_expflags & EXP_FLAG_ESC_GLOB_CHARS));
		ఒకవేళ (val && val[0])
			o_కూడు_Qstr(output, val);
	}
	తిరిగి n;
}

/* Handle <SPECIAL_VAR_SYMBOL>varname...<SPECIAL_VAR_SYMBOL> construct.
 */
స్థిర NOINLINE పూర్ణాంకం expand_one_var(o_string *output, పూర్ణాంకం n,
		పూర్ణాంకం మొదటి_ch, అక్షరం *arg, అక్షరం **pp)
{
	స్థిరరాశి అక్షరం *val;
	అక్షరం *గమ్యం_be_freed;
	అక్షరం *p;
	అక్షరం *var;
	అక్షరం exp_op;
	అక్షరం exp_save = exp_save; /* for compiler */
	అక్షరం *exp_saveptr; /* points to expansion operator */
	అక్షరం *exp_మాట = exp_మాట; /* for compiler */
	అక్షరం arg0;

	val = ఖాళీ;
	గమ్యం_be_freed = ఖాళీ;
	p = *pp;
	*p = '\0'; /* replace trailing SPECIAL_VAR_SYMBOL */
	var = arg;
	exp_saveptr = arg[1] ? strchr(VAR_ENCODED_SUBST_OPS, arg[1]) : ఖాళీ;
	arg0 = arg[0];
	arg[0] = (arg0 & 0x7f);
	exp_op = 0;

	ఒకవేళ (arg[0] == '#' && arg[1] /* ${#...} but not ${#} */
	 && (!exp_saveptr               /* and ( not(${#<op_char>...}) */
	    || (arg[2] == '\0' && strchr(SPECIAL_VARS_STR, arg[1])) /* or ${#C} "len of $C" ) */
	    )		/* NB: skipping ^^^specvar check mishandles ${#::2} */
	) {
		/* It must be length operator: ${#var} */
		var++;
		exp_op = 'L';
	} లేకపోతే {
		/* Maybe handle parameter expansion */
		ఒకవేళ (exp_saveptr /* if 2nd char is one of expansion operators */
		 && strchr(NUMERIC_SPECVARS_STR, arg[0]) /* 1st char is special variable */
		) {
			/* ${?:0}, ${#[:]%0} etc */
			exp_saveptr = var + 1;
		} లేకపోతే {
			/* ${?}, ${var}, ${var:0}, ${var[:]%0} etc */
			exp_saveptr = var+1 + strcspn(var+1, VAR_ENCODED_SUBST_OPS);
		}
		exp_op = exp_save = *exp_saveptr;
#నిర్వచితం_కాకపోతే __U_BOOT__
		ఒకవేళ (exp_op) {
			exp_మాట = exp_saveptr + 1;
			ఒకవేళ (exp_op == ':') {
				exp_op = *exp_మాట++;
//TODO: try ${var:} and ${var:bogus} in non-bash config
				ఒకవేళ (BASH_SUBSTR
				 && (!exp_op || !strchr(MINUS_PLUS_EQUAL_QUESTION, exp_op))
				) {
					/* oops... it's ${var:N[:M]}, not ${var:?xxx} or some such */
					exp_op = ':';
					exp_మాట--;
				}
			}
			*exp_saveptr = '\0';
		} /* else: it's not an expansion op, but bare ${var} */
#అయితే_ముగింపు /* !__U_BOOT__ */
	}

	/* Look up the variable in question */
	ఒకవేళ (అంకెనా(var[0])) {
		/* parse_dollar should have vetted var for us */
#నిర్వచితం_కాకపోతే __U_BOOT__
		పూర్ణాంకం nn = xatoi_ధనాత్మక(var);
#లేకపోతే /* __U_BOOT__ */
		పూర్ణాంకం nn = simple_strtoul(var, ఖాళీ, 10);
#అయితే_ముగింపు /* __U_BOOT__ */
		ఒకవేళ (nn < G.global_argc)
			val = G.global_argv[nn];
		/* else val remains NULL: $N with too big N */
	} లేకపోతే {
		ఎంపిక (var[0]) {
#నిర్వచితం_కాకపోతే __U_BOOT__
		స్థితి '$': /* pid */
			val = utoa(G.మూలం_pid);
			ఆపు;
		స్థితి '!': /* bg pid */
			val = G.చివరి_bg_pid ? utoa(G.చివరి_bg_pid) : "";
			ఆపు;
#అయితే_ముగింపు /* !__U_BOOT__ */
		స్థితి '?': /* exitcode */
			val = utoa(G.చివరి_exitcode);
			ఆపు;
		స్థితి '#': /* argc */
			val = utoa(G.global_argc ? G.global_argc-1 : 0);
			ఆపు;
#నిర్వచితం_కాకపోతే __U_BOOT__
		స్థితి '-': { /* active options */
			/* Check set_mode() to see what option chars we support */
			అక్షరం *cp;
			val = cp = G.optstring_buf;
			ఒకవేళ (G.o_opt[OPT_O_ERREXIT])
				*cp++ = 'e';
			ఒకవేళ (G_interactive_fd)
				*cp++ = 'i';
			ఒకవేళ (G_x_బహుళకం)
				*cp++ = 'x';
			/* If G.o_opt[OPT_O_NOEXEC] is true,
			 * commands read but are not executed,
			 * so $- can not execute too, 'n' is never seen in $-.
			 */
			ఒకవేళ (G.opt_c)
				*cp++ = 'c';
			ఒకవేళ (G.opt_s)
				*cp++ = 's';
			*cp = '\0';
			ఆపు;
		}
#అయితే_ముగింపు /* !__U_BOOT__ */
		అప్రమేయ:
#నిర్వచితం_కాకపోతే __U_BOOT__
			val = get_local_var_విలువ(var);
#లేకపోతే /* __U_BOOT__ */
			/*
			 * Environment variable set with setenv* have to be
			 * expanded.
			 * So, we first search if the variable exists in
			 * environment, if this is not the case, we default to
			 * local value.
			 */
			val = env_get(var);
			ఒకవేళ (!val)
				val = get_local_var_విలువ(var);
#అయితే_ముగింపు /* __U_BOOT__ */
		}
	}

#నిర్వచితం_కాకపోతే __U_BOOT__
	/* Handle any expansions */
	ఒకవేళ (exp_op == 'L') {
		reinit_unicode_for_hush();
		debug_printf_expand("expand: length(%s)=", val);
		val = utoa(val ? unicode_strlen(val) : 0);
		debug_printf_expand("%s\n", val);
	} లేకపోతే ఒకవేళ (exp_op) {
		ఒకవేళ (exp_op == '%' || exp_op == '#') {
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
			ఒకవేళ (val && val[0]) {
				అక్షరం *t;
				అక్షరం *exp_exp_మాట;
				అక్షరం *loc;
				గుర్తులేని పరిశీలన_flags = pick_పరిశీలన(exp_op, *exp_మాట);
				ఒకవేళ (exp_op == *exp_మాట)  /* ## or %% */
					exp_మాట++;
				debug_printf_expand("expand: exp_word:'%s'\n", exp_మాట);
				exp_exp_మాట = encode_then_expand_vararg(exp_మాట, /*handle_squotes:*/ 1, /*unbackslash:*/ 0);
				ఒకవేళ (exp_exp_మాట)
					exp_మాట = exp_exp_మాట;
				debug_printf_expand("expand: exp_word:'%s'\n", exp_మాట);
				/*
				 * HACK ALERT. We depend here on the fact that
				 * G.global_argv and results of utoa and get_local_var_value
				 * are actually in writable memory:
				 * scan_and_match momentarily stores NULs there.
				 */
				t = (అక్షరం*)val;
				loc = పరిశీలన_and_సరిపోలిక(t, exp_మాట, పరిశీలన_flags);
				debug_printf_expand("op:%c str:'%s' pat:'%s' res:'%s'\n", exp_op, t, exp_మాట, loc);
				విడుదల(exp_exp_మాట);
				ఒకవేళ (loc) { /* match was found */
					ఒకవేళ (పరిశీలన_flags & పరిశీలన_సరిపోలిక_ఎడమ_HALF) /* #[#] */
						val = loc; /* take right part */
					లేకపోతే /* %[%] */
						val = గమ్యం_be_freed = xstrndup(val, loc - val); /*ఎడమ*/
				}
			}
		}
#ఒకవేళ BASH_నమూనా_SUBST
		లేకపోతే ఒకవేళ (exp_op == '/' || exp_op == '\\') {
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
			ఒకవేళ (val /*&& val[0]*/) {
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
				అక్షరం *నమూనా, *repl, *t;
				నమూనా = encode_then_expand_vararg(exp_మాట, /*handle_squotes:*/ 1, /*unbackslash:*/ 0);
				ఒకవేళ (!నమూనా)
					నమూనా = xstrdup(exp_మాట);
				debug_printf_varexp("pattern:'%s'->'%s'\n", exp_మాట, నమూనా);
				*p++ = SPECIAL_VAR_చిహ్నం;
				exp_మాట = p;
				p = strchr(p, SPECIAL_VAR_చిహ్నం);
				*p = '\0';
				repl = encode_then_expand_vararg(exp_మాట, /*handle_squotes:*/ 1, /*unbackslash:*/ 1);
				debug_printf_varexp("repl:'%s'->'%s'\n", exp_మాట, repl);
				/* HACK ALERT. We depend here on the fact that
				 * G.global_argv and results of utoa and get_local_var_value
				 * are actually in writable memory:
				 * replace_pattern momentarily stores NULs there. */
				t = (అక్షరం*)val;
				గమ్యం_be_freed = replace_నమూనా(t,
						నమూనా,
						(repl ? repl : exp_మాట),
						exp_op);
				ఒకవేళ (గమ్యం_be_freed) /* at least one replace happened */
					val = గమ్యం_be_freed;
				విడుదల(నమూనా);
				విడుదల(repl);
			} లేకపోతే {
				/* Unset variable always gives nothing */
				//  a=; echo ${a/*/w}      # "w"
				//  unset a; echo ${a/*/w} # ""
				/* Just skip "replace" part */
				*p++ = SPECIAL_VAR_చిహ్నం;
				p = strchr(p, SPECIAL_VAR_చిహ్నం);
				*p = '\0';
			}
		}
#అయితే_ముగింపు /* BASH_PATTERN_SUBST */
		లేకపోతే ఒకవేళ (exp_op == ':') {
#ఒకవేళ BASH_SUBSTR && ENABLE_FEATURE_SH_MATH
			/* It's ${var:N[:M]} bashism.
			 * Note that in encoded form it has TWO parts:
			 * var:N<SPECIAL_VAR_SYMBOL>M<SPECIAL_VAR_SYMBOL>
			 */
			arith_t beg, len;
			గుర్తులేని vallen;
			స్థిరరాశి అక్షరం *errmsg;

			beg = expand_and_evaluate_arith(exp_మాట, &errmsg);
			ఒకవేళ (errmsg)
				వెళ్ళు ఖాళీలు_ఫలితం;
			debug_printf_varexp("beg:'%s'=%lld\n", exp_మాట, (దీర్ఘ దీర్ఘ)beg);
			*p++ = SPECIAL_VAR_చిహ్నం;
			exp_మాట = p;
			p = strchr(p, SPECIAL_VAR_చిహ్నం);
			*p = '\0';
			vallen = val ? అక్షరశ్రేణి_పొడవు(val) : 0;
			ఒకవేళ (beg < 0) {
				/* negative beg counts from the end */
				beg = (arith_t)vallen + beg;
			}
			/* If expansion will be empty, do not even evaluate len */
			ఒకవేళ (!val || beg < 0 || beg > vallen) {
				/* Why > vallen, not >=? bash:
				 * unset b; a=ab; : ${a:2:${b=3}}; echo $b  # "", b=3 (!!!)
				 * unset b; a=a; : ${a:2:${b=3}}; echo $b   # "", b not set
				 */
				వెళ్ళు ఖాళీలు_ఫలితం;
			}
			len = expand_and_evaluate_arith(exp_మాట, &errmsg);
			ఒకవేళ (errmsg)
				వెళ్ళు ఖాళీలు_ఫలితం;
			debug_printf_varexp("len:'%s'=%lld\n", exp_మాట, (దీర్ఘ దీర్ఘ)len);
			debug_printf_varexp("from val:'%s'\n", val);
			ఒకవేళ (len < 0) {
				/* in bash, len=-n means strlen()-n */
				len = (arith_t)vallen - beg + len;
				ఒకవేళ (len < 0) /* bash compat */
					సందేశం_and_die_if_script("%s: substring expression < 0", var);
			}
			ఒకవేళ (len <= 0 || !val /*|| beg >= vallen*/) {
 ఖాళీలు_ఫలితం:
				val = ఖాళీ;
			} లేకపోతే {
				/* Paranoia. What if user entered 9999999999999
				 * which fits in arith_t but not int? */
				ఒకవేళ (len > గరిష్ఠ_పూర్ణాంకం)
					len = గరిష్ఠ_పూర్ణాంకం;
				val = గమ్యం_be_freed = xstrndup(val + beg, len);
			}
			debug_printf_varexp("val:'%s'\n", val);
#లేకపోతే /* not (HUSH_SUBSTR_EXPANSION && FEATURE_SH_MATH) */
			సందేశం_and_die_if_script("malformed ${%s:...}", var);
			val = ఖాళీ;
#అయితే_ముగింపు
		} లేకపోతే { /* one of "-=+?" */
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
			పూర్ణాంకం use_మాట = (!val || ((exp_save == ':') && !val[0]));
			ఒకవేళ (exp_op == '+')
				use_మాట = !use_మాట;
			debug_printf_expand("expand: op:%c (null:%s) test:%i\n", exp_op,
					(exp_save == ':') ? "true" : "false", use_మాట);
			ఒకవేళ (use_మాట) {
				ఒకవేళ (exp_op == '+' || exp_op == '-') {
					/* ${var+word} - use alternative value */
					/* ${var-word} - use default value */
					n = encode_then_append_var_plusminus(output, n, exp_మాట,
							/*dquoted:*/ (arg0 & 0x80)
					);
					val = ఖాళీ;
				} లేకపోతే {
					/* ${var?word} - indicate error if unset */
					/* ${var=word} - assign and use default value */
					గమ్యం_be_freed = encode_then_expand_vararg(exp_మాట,
							/*handle_squotes:*/ !(arg0 & 0x80),
							/*unbackslash:*/ 0
					);
					ఒకవేళ (గమ్యం_be_freed)
						exp_మాట = గమ్యం_be_freed;
					ఒకవేళ (exp_op == '?') {
						/* mimic bash message */
						సందేశం_and_die_if_script("%s: %s",
							var,
							exp_మాట[0]
							? exp_మాట
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
					} లేకపోతే {
						val = exp_మాట;
					}
					ఒకవేళ (exp_op == '=') {
						/* ${var=[word]} or ${var:=[word]} */
						ఒకవేళ (అంకెనా(var[0]) || var[0] == '#') {
							/* mimic bash message */
							సందేశం_and_die_if_script("$%s: cannot assign in this way", var);
							val = ఖాళీ;
						} లేకపోతే {
							అక్షరం *new_var = xasprintf("%s=%s", var, val);
							సమితి_local_var0(new_var);
						}
					}
				}
			}
		} /* one of "-=+?" */

		*exp_saveptr = exp_save;
	} /* if (exp_op) */

#అయితే_ముగింపు /* !__U_BOOT__ */
	arg[0] = arg0;
	*pp = p;

	n = append_str_maybe_ifs_విభజించు(output, n, మొదటి_ch, val);

	విడుదల(గమ్యం_be_freed);
	తిరిగి n;
}

/* Expand all variable references in given string, adding words to list[]
 * at n, n+1,... positions. Return updated n (so that list[n] is next one
 * to be filled). This routine is extremely tricky: has to deal with
 * variables/parameters with whitespace, $* and $@, and constructs like
 * 'echo -$*-'. If you play here, you must run testsuite afterwards! */
స్థిర NOINLINE పూర్ణాంకం expand_vars_గమ్యం_list(o_string *output, పూర్ణాంకం n, అక్షరం *arg)
{
	/* output->o_expflags & EXP_FLAG_SINGLEWORD (0x80) if we are in
	 * expansion of right-hand side of assignment == 1-element expand.
	 */
	అక్షరం cant_be_null = 0; /* only bit 0x80 matters */
	అక్షరం *p;

	debug_printf_expand("expand_vars_to_list: arg:'%s' singleword:%x\n", arg,
			!!(output->o_expflags & EXP_FLAG_SINGLEWORD));
	debug_print_list("expand_vars_to_list[0]", output, n);

	అయితే ((p = strchr(arg, SPECIAL_VAR_చిహ్నం)) != ఖాళీ) {
		అక్షరం మొదటి_ch;
#ఒకవేళ ENABLE_FEATURE_SH_MATH
		అక్షరం arith_buf[పరిమాణం(arith_t)*3 + 2];
#అయితే_ముగింపు

		ఒకవేళ (output->ended_లోపల_ifs) {
			o_addchr(output, '\0');
			n = o_save_ptr(output, n);
			output->ended_లోపల_ifs = 0;
		}

		o_addblock(output, arg, p - arg);
		debug_print_list("expand_vars_to_list[1]", output, n);
		arg = ++p;
		p = strchr(p, SPECIAL_VAR_చిహ్నం);

		/* Fetch special var name (if it is indeed one of them)
		 * and quote bit, force the bit on if singleword expansion -
		 * important for not getting v=$@ expand to many words. */
		మొదటి_ch = arg[0] | (output->o_expflags & EXP_FLAG_SINGLEWORD);

		/* Is this variable quoted and thus expansion can't be null?
		 * "$@" is special. Even if quoted, it can still
		 * expand to nothing (not even an empty string),
		 * thus it is excluded. */
		ఒకవేళ ((మొదటి_ch & 0x7f) != '@')
			cant_be_null |= మొదటి_ch;

		ఎంపిక (మొదటి_ch & 0x7f) {
		/* Highest bit in first_ch indicates that var is double-quoted */
		స్థితి '*':
		స్థితి '@': {
			పూర్ణాంకం i;
#నిర్వచితం_కాకపోతే __U_BOOT__
			ఒకవేళ (!G.global_argv[1])
#లేకపోతే /* __U_BOOT__ */
			ఒకవేళ (!G.global_argv || !G.global_argv[1])
#అయితే_ముగింపు /* __U_BOOT__ */
				ఆపు;
			i = 1;
			cant_be_null |= మొదటి_ch; /* do it for "$@" _now_, when we know it's not empty */
			ఒకవేళ (!(మొదటి_ch & 0x80)) { /* unquoted $* or $@ */
				అయితే (G.global_argv[i]) {
					n = expand_on_ifs(output, n, G.global_argv[i]);
					debug_printf_expand("expand_vars_to_list: argv %d (last %d)\n", i, G.global_argc - 1);
					ఒకవేళ (G.global_argv[i++][0] && G.global_argv[i]) {
						/* this argv[] is not empty and not last:
						 * put terminating NUL, start new word */
						o_addchr(output, '\0');
						debug_print_list("expand_vars_to_list[2]", output, n);
						n = o_save_ptr(output, n);
						debug_print_list("expand_vars_to_list[3]", output, n);
					}
				}
			} లేకపోతే
			/* If EXP_FLAG_SINGLEWORD, we handle assignment 'a=....$@.....'
			 * and in this case should treat it like '$*' - see 'else...' below */
			ఒకవేళ (మొదటి_ch == (అక్షరం)('@'|0x80)  /* quoted $@ */
			 && !(output->o_expflags & EXP_FLAG_SINGLEWORD) /* not v="$@" case */
			) {
				అయితే (1) {
					o_కూడు_Qstr(output, G.global_argv[i]);
					ఒకవేళ (++i >= G.global_argc)
						ఆపు;
					o_addchr(output, '\0');
					debug_print_list("expand_vars_to_list[4]", output, n);
					n = o_save_ptr(output, n);
				}
			} లేకపోతే { /* quoted $* (or v="$@" case): add as one word */
				అయితే (1) {
					o_కూడు_Qstr(output, G.global_argv[i]);
					ఒకవేళ (!G.global_argv[++i])
						ఆపు;
					ఒకవేళ (G.ifs[0])
						o_addchr(output, G.ifs[0]);
				}
				output->has_quoted_part = 1;
			}
			ఆపు;
		}
		స్థితి SPECIAL_VAR_చిహ్నం: {
			/* <SPECIAL_VAR_SYMBOL><SPECIAL_VAR_SYMBOL> */
			/* "Empty variable", used to make "" etc to not disappear */
			output->has_quoted_part = 1;
			cant_be_null = 0x80;
			arg++;
			ఆపు;
		}
		స్థితి SPECIAL_VAR_QUOTED_SVS:
			/* <SPECIAL_VAR_SYMBOL><SPECIAL_VAR_QUOTED_SVS><SPECIAL_VAR_SYMBOL> */
			/* "^C variable", represents literal ^C char (possible in scripts) */
			o_addchr(output, SPECIAL_VAR_చిహ్నం);
			arg++;
			ఆపు;
#ఒకవేళ ENABLE_HUSH_TICK
		స్థితి '`': {
			/* <SPECIAL_VAR_SYMBOL>`cmd<SPECIAL_VAR_SYMBOL> */
			o_string subst_ఫలితం = NULL_O_STRING;

			*p = '\0'; /* replace trailing <SPECIAL_VAR_SYMBOL> */
			arg++;
			/* Can't just stuff it into output o_string,
			 * expanded result may need to be globbed
			 * and $IFS-split */
			debug_printf_subst("SUBST '%s' first_ch %x\n", arg, మొదటి_ch);
			G.చివరి_exitcode = process_command_subs(&subst_ఫలితం, arg);
			G.expand_exitcode = G.చివరి_exitcode;
			debug_printf_subst("SUBST RES:%d '%s'\n", G.చివరి_exitcode, subst_ఫలితం.దత్తాంశం);
			n = append_str_maybe_ifs_విభజించు(output, n, మొదటి_ch, subst_ఫలితం.దత్తాంశం);
			o_free(&subst_ఫలితం);
			ఆపు;
		}
#అయితే_ముగింపు
#ఒకవేళ ENABLE_FEATURE_SH_MATH
		స్థితి '+': {
			/* <SPECIAL_VAR_SYMBOL>+arith<SPECIAL_VAR_SYMBOL> */
			arith_t res;

			arg++; /* skip '+' */
			*p = '\0'; /* replace trailing <SPECIAL_VAR_SYMBOL> */
			debug_printf_subst("ARITH '%s' first_ch %x\n", arg, మొదటి_ch);
			res = expand_and_evaluate_arith(arg, ఖాళీ);
			debug_printf_subst("ARITH RES '"ARITH_FMT"'\n", res);
			sprintf(arith_buf, ARITH_FMT, res);
			ఒకవేళ (res < 0
			 && మొదటి_ch == (అక్షరం)('+'|0x80)
			/* && (output->o_expflags & EXP_FLAG_ESC_GLOB_CHARS) */
			) {
				/* Quoted negative ariths, like filename[0"$((-9))"],
				 * should not be interpreted as glob ranges.
				 * Convert leading '-' to '\-':
				 */
				o_grow_by(output, 1);
				output->దత్తాంశం[output->నిడివి++] = '\\';
			}
			o_addstr(output, arith_buf);
			ఆపు;
		}
#అయితే_ముగింపు
		అప్రమేయ:
			/* <SPECIAL_VAR_SYMBOL>varname[ops]<SPECIAL_VAR_SYMBOL> */
			n = expand_one_var(output, n, మొదటి_ch, arg, &p);
			ఆపు;
		} /* switch (char after <SPECIAL_VAR_SYMBOL>) */

		/* Restore NULL'ed SPECIAL_VAR_SYMBOL.
		 * Do the check to avoid writing to a const string. */
		ఒకవేళ (*p != SPECIAL_VAR_చిహ్నం)
			*p = SPECIAL_VAR_చిహ్నం;
		arg = ++p;
	} /* end of "while (SPECIAL_VAR_SYMBOL is found) ..." */

	ఒకవేళ (*arg) {
		/* handle trailing string */
		ఒకవేళ (output->ended_లోపల_ifs) {
			o_addchr(output, '\0');
			n = o_save_ptr(output, n);
		}
		debug_print_list("expand_vars_to_list[a]", output, n);
		/* this part is literal, and it was already pre-quoted
		 * if needed (much earlier), do not use o_addQstr here!
		 */
		o_addstr(output, arg);
		debug_print_list("expand_vars_to_list[b]", output, n);
	} లేకపోతే
	ఒకవేళ (output->నిడివి == o_get_చివరి_ptr(output, n) /* expansion is empty */
	 && !(cant_be_null & 0x80)   /* and all vars were not quoted */
	 && !output->has_quoted_part
	) {
		n--;
		/* allow to reuse list[n] later without re-growth */
		output->has_ఖాళీలు_స్థానం = 1;
	}

	తిరిగి n;
}

స్థిర అక్షరం **expand_variables(అక్షరం **argv, గుర్తులేని expflags)
{
	పూర్ణాంకం n;
	అక్షరం **list;
	o_string output = NULL_O_STRING;

	output.o_expflags = expflags;

	n = 0;
	కొరకు (;;) {
		/* go to next list[n] */
		output.ended_లోపల_ifs = 0;
		n = o_save_ptr(&output, n);

		ఒకవేళ (!*argv)
			ఆపు;

		/* expand argv[i] */
		n = expand_vars_గమ్యం_list(&output, n, *argv++);
		/* if (!output->has_empty_slot) -- need this?? */
			o_addchr(&output, '\0');
	}
	debug_print_list("expand_variables", &output, n);

	/* output.data (malloced in one block) gets returned in "list" */
	list = o_finalize_list(&output, n);
	debug_print_అక్షరశ్రేణులు("expand_variables[1]", list);
	తిరిగి list;
}

స్థిర అక్షరం **expand_strvec_గమ్యం_strvec(అక్షరం **argv)
{
	తిరిగి expand_variables(argv, EXP_FLAG_GLOB | EXP_FLAG_ESC_GLOB_CHARS);
}

#ఒకవేళ defined(CMD_SINGLEWORD_NOGLOB) || defined(CMD_TEST2_SINGLEWORD_NOGLOB)
స్థిర అక్షరం **expand_strvec_గమ్యం_strvec_singleword_noglob(అక్షరం **argv)
{
	తిరిగి expand_variables(argv, EXP_FLAG_SINGLEWORD);
}
#అయితే_ముగింపు

/* Used for expansion of right hand of assignments,
 * $((...)), heredocs, variable expansion parts.
 *
 * NB: should NOT do globbing!
 * "export v=/bin/c*; env | grep ^v=" outputs "v=/bin/c*"
 */
స్థిర అక్షరం *expand_string_గమ్యం_string(స్థిరరాశి అక్షరం *str, పూర్ణాంకం EXP_flags, పూర్ణాంకం do_unbackslash)
{
#ఒకవేళ !BASH_నమూనా_SUBST && !ENABLE_HUSH_CASE
	స్థిరరాశి పూర్ణాంకం do_unbackslash = 1;
	స్థిరరాశి పూర్ణాంకం EXP_flags = EXP_FLAG_ESC_GLOB_CHARS;
#అయితే_ముగింపు
	అక్షరం *argv[2], **list;

	debug_printf_expand("string_to_string<='%s'\n", str);
	/* This is generally an optimization, but it also
	 * handles "", which otherwise trips over !list[0] check below.
	 * (is this ever happens that we actually get str="" here?)
	 */
	ఒకవేళ (!strchr(str, SPECIAL_VAR_చిహ్నం) && !strchr(str, '\\')) {
		//TODO: Can use on strings with \ too, just unbackslash() them?
		debug_printf_expand("string_to_string(fast)=>'%s'\n", str);
		తిరిగి xstrdup(str);
	}

	argv[0] = (అక్షరం*)str;
	argv[1] = ఖాళీ;
	list = expand_variables(argv, EXP_flags | EXP_FLAG_SINGLEWORD);
	ఒకవేళ (!list[0]) {
		/* Example where it happens:
		 * x=; echo ${x:-"$@"}
		 */
		((అక్షరం*)list)[0] = '\0';
	} లేకపోతే {
		ఒకవేళ (HUSH_DEBUG)
			ఒకవేళ (list[1])
				bb_simple_error_సందేశం_and_die("BUG in varexp2");
		/* actually, just move string 2*sizeof(char*) bytes back */
		overlapping_strcpy((అక్షరం*)list, list[0]);
		ఒకవేళ (do_unbackslash)
			unbackslash((అక్షరం*)list);
	}
	debug_printf_expand("string_to_string=>'%s'\n", (అక్షరం*)list);
	తిరిగి (అక్షరం*)list;
}

#ఒకవేళ 0
స్థిర అక్షరం* expand_strvec_గమ్యం_string(అక్షరం **argv)
{
	అక్షరం **list;

	list = expand_variables(argv, EXP_FLAG_SINGLEWORD);
	/* Convert all NULs to spaces */
	ఒకవేళ (list[0]) {
		పూర్ణాంకం n = 1;
		అయితే (list[n]) {
			ఒకవేళ (HUSH_DEBUG)
				ఒకవేళ (list[n-1] + అక్షరశ్రేణి_పొడవు(list[n-1]) + 1 != list[n])
					bb_error_సందేశం_and_die("BUG in varexp3");
			/* bash uses ' ' regardless of $IFS contents */
			list[n][-1] = ' ';
			n++;
		}
	}
	overlapping_strcpy((అక్షరం*)list, list[0] ? list[0] : "");
	debug_printf_expand("strvec_to_string='%s'\n", (అక్షరం*)list);
	తిరిగి (అక్షరం*)list;
}
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే __U_BOOT__
స్థిర అక్షరం **expand_assignments(అక్షరం **argv, పూర్ణాంకం లెక్క)
{
	పూర్ణాంకం i;
	అక్షరం **p;

	G.expanded_assignments = p = ఖాళీ;
	/* Expand assignments into one string each */
	కొరకు (i = 0; i < లెక్క; i++) {
		p = కూడు_string_గమ్యం_అక్షరశ్రేణులు(p,
			expand_string_గమ్యం_string(argv[i],
				EXP_FLAG_ESC_GLOB_CHARS,
				/*unbackslash:*/ 1
			)
		);
		G.expanded_assignments = p;
	}
	G.expanded_assignments = ఖాళీ;
	తిరిగి p;
}

స్థిర శూన్య switch_off_special_sigs(గుర్తులేని ముసుగు)
{
	గుర్తులేని sig = 0;
	అయితే ((ముసుగు >>= 1) != 0) {
		sig++;
		ఒకవేళ (!(ముసుగు & 1))
			కొనసాగు;
#ఒకవేళ ENABLE_HUSH_TRAP
		ఒకవేళ (G_traps) {
			ఒకవేళ (G_traps[sig] && !G_traps[sig][0])
				/* trap is '', has to remain SIG_IGN */
				కొనసాగు;
			విడుదల(G_traps[sig]);
			G_traps[sig] = ఖాళీ;
		}
#అయితే_ముగింపు
		/* We are here only if no trap or trap was not '' */
		install_sighandler(sig, SIG_DFL);
	}
}
#అయితే_ముగింపు /* !__U_BOOT__ */

#నిర్వచితం_కాకపోతే __U_BOOT__
#ఒకవేళ BB_MMU
/* never called */
శూన్య re_execute_shell(అక్షరం ***గమ్యం_free, స్థిరరాశి అక్షరం *s,
		అక్షరం *g_argv0, అక్షరం **g_argv,
		అక్షరం **builtin_argv) NORETURN;

స్థిర శూన్య reset_traps_గమ్యం_defaults(శూన్య)
{
	/* This function is always called in a child shell
	 * after fork (not vfork, NOMMU doesn't use this function).
	 */
	IF_HUSH_TRAP(గుర్తులేని sig;)
	గుర్తులేని ముసుగు;

	/* Child shells are not interactive.
	 * SIGTTIN/SIGTTOU/SIGTSTP should not have special handling.
	 * Testcase: (while :; do :; done) + ^Z should background.
	 * Same goes for SIGTERM, SIGHUP, SIGINT.
	 */
	ముసుగు = (G.special_sig_ముసుగు & SPECIAL_INTERACTIVE_SIGS) | G_fatal_sig_ముసుగు;
	ఒకవేళ (!G_traps && !ముసుగు)
		తిరిగి; /* already no traps and no special sigs */

	/* Switch off special sigs */
	switch_off_special_sigs(ముసుగు);
# ఒకవేళ ENABLE_HUSH_పని
	G_fatal_sig_ముసుగు = 0;
# endif
	G.special_sig_ముసుగు &= ~SPECIAL_INTERACTIVE_SIGS;
	/* SIGQUIT,SIGCHLD and maybe SPECIAL_JOBSTOP_SIGS
	 * remain set in G.special_sig_mask */

# ఒకవేళ ENABLE_HUSH_TRAP
	ఒకవేళ (!G_traps)
		తిరిగి;

	/* Reset all sigs to default except ones with empty traps */
	కొరకు (sig = 0; sig < NSIG; sig++) {
		ఒకవేళ (!G_traps[sig])
			కొనసాగు; /* no trap: nothing to do */
		ఒకవేళ (!G_traps[sig][0])
			కొనసాగు; /* empty trap: has to remain SIG_IGN */
		/* sig has non-empty trap, reset it: */
		విడుదల(G_traps[sig]);
		G_traps[sig] = ఖాళీ;
		/* There is no signal for trap 0 (EXIT) */
		ఒకవేళ (sig == 0)
			కొనసాగు;
		install_sighandler(sig, pick_sighandler(sig));
	}
# endif
}

#లేకపోతే /* !BB_MMU */

స్థిర శూన్య re_execute_shell(అక్షరం ***గమ్యం_free, స్థిరరాశి అక్షరం *s,
		అక్షరం *g_argv0, అక్షరం **g_argv,
		అక్షరం **builtin_argv) NORETURN;
స్థిర శూన్య re_execute_shell(అక్షరం ***గమ్యం_free, స్థిరరాశి అక్షరం *s,
		అక్షరం *g_argv0, అక్షరం **g_argv,
		అక్షరం **builtin_argv)
{
# define NOMMU_HACK_FMT ("-$%x:%x:%x:%x:%x:%llx" IF_HUSH_LOOPS(":%x"))
	/* delims + 2 * (number of bytes in printed hex numbers) */
	అక్షరం param_buf[పరిమాణం(NOMMU_HACK_FMT) + 2 * (పరిమాణం(పూర్ణాంకం)*6 + పరిమాణం(దీర్ఘ దీర్ఘ)*1)];
	అక్షరం *heredoc_argv[4];
	నిర్మాణం variable *cur;
# ఒకవేళ ENABLE_HUSH_FUNCTIONS
	నిర్మాణం function *funcp;
# endif
	అక్షరం **argv, **pp;
	గుర్తులేని cnt;
	గుర్తులేని దీర్ఘ దీర్ఘ ఖాళీలు_trap_ముసుగు;

	ఒకవేళ (!g_argv0) { /* heredoc */
		argv = heredoc_argv;
		argv[0] = (అక్షరం *) G.argv0_for_re_execing;
		argv[1] = (అక్షరం *) "-<";
		argv[2] = (అక్షరం *) s;
		argv[3] = ఖాళీ;
		pp = &argv[3]; /* used as pointer to empty environment */
		వెళ్ళు do_exec;
	}

	cnt = 0;
	pp = builtin_argv;
	ఒకవేళ (pp) అయితే (*pp++)
		cnt++;

	ఖాళీలు_trap_ముసుగు = 0;
	ఒకవేళ (G_traps) {
		పూర్ణాంకం sig;
		కొరకు (sig = 1; sig < NSIG; sig++) {
			ఒకవేళ (G_traps[sig] && !G_traps[sig][0])
				ఖాళీలు_trap_ముసుగు |= 1LL << sig;
		}
	}

	sprintf(param_buf, NOMMU_HACK_FMT
			, (గుర్తులేని) G.మూలం_pid
			, (గుర్తులేని) G.మూలం_ppid
			, (గుర్తులేని) G.చివరి_bg_pid
			, (గుర్తులేని) G.చివరి_exitcode
			, cnt
			, ఖాళీలు_trap_ముసుగు
			IF_HUSH_LOOPS(, G.లోతు_యొక్క_loop)
			);
# undef NOMMU_HACK_FMT
	/* 1:hush 2:-$<pid>:<pid>:<exitcode>:<etc...> <vars...> <funcs...>
	 * 3:-c 4:<cmd> 5:<arg0> <argN...> 6:NULL
	 */
	cnt += 6;
	కొరకు (cur = G.శిఖరం_var; cur; cur = cur->తదుపరి) {
		ఒకవేళ (!cur->flg_export || cur->flg_read_only)
			cnt += 2;
	}
# ఒకవేళ ENABLE_HUSH_LINENO_VAR
	cnt += 2;
# endif
# ఒకవేళ ENABLE_HUSH_FUNCTIONS
	కొరకు (funcp = G.శిఖరం_func; funcp; funcp = funcp->తదుపరి)
		cnt += 3;
# endif
	pp = g_argv;
	అయితే (*pp++)
		cnt++;
	*గమ్యం_free = argv = pp = xzalloc(పరిమాణం(argv[0]) * cnt);
	*pp++ = (అక్షరం *) G.argv0_for_re_execing;
	*pp++ = param_buf;
	కొరకు (cur = G.శిఖరం_var; cur; cur = cur->తదుపరి) {
		ఒకవేళ (strcmp(cur->varstr, hush_సంస్కరణ_str) == 0)
			కొనసాగు;
		ఒకవేళ (cur->flg_read_only) {
			*pp++ = (అక్షరం *) "-R";
			*pp++ = cur->varstr;
		} లేకపోతే ఒకవేళ (!cur->flg_export) {
			*pp++ = (అక్షరం *) "-V";
			*pp++ = cur->varstr;
		}
	}
# ఒకవేళ ENABLE_HUSH_LINENO_VAR
	*pp++ = (అక్షరం *) "-L";
	*pp++ = utoa(G.execute_lineno);
# endif
# ఒకవేళ ENABLE_HUSH_FUNCTIONS
	కొరకు (funcp = G.శిఖరం_func; funcp; funcp = funcp->తదుపరి) {
		*pp++ = (అక్షరం *) "-F";
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
	*pp++ = (అక్షరం *) "-c";
	*pp++ = (అక్షరం *) s;
	ఒకవేళ (builtin_argv) {
		అయితే (*++builtin_argv)
			*pp++ = *builtin_argv;
		*pp++ = (అక్షరం *) "";
	}
	*pp++ = g_argv0;
	అయితే (*g_argv)
		*pp++ = *g_argv++;
	/* *pp = NULL; - is already there */
	pp = environ;

 do_exec:
	debug_printf_exec("re_execute_shell pid:%d cmd:'%s'\n", getpid(), s);
	/* Don't propagate SIG_IGN to the child */
	ఒకవేళ (SPECIAL_JOBSTOP_SIGS != 0)
		switch_off_special_sigs(G.special_sig_ముసుగు & SPECIAL_JOBSTOP_SIGS);
	execve(bb_busybox_exec_మార్గం, argv, pp);
	/* Fallback. Useful for init=/bin/hush usage etc */
	ఒకవేళ (argv[0][0] == '/')
		execve(argv[0], argv, pp);
	xfunc_error_retval = 127;
	bb_simple_error_సందేశం_and_die("can't re-execute the shell");
}
#అయితే_ముగింపు  /* !BB_MMU */

#అయితే_ముగింపు /* !__U_BOOT__ */

స్థిర పూర్ణాంకం నిరంతరం_and_free_list(నిర్మాణం pipe *pi);

/* Executing from string: eval, sh -c '...'
 *          or from file: /etc/profile, . file, sh <script>, sh (intereactive)
 * end_trigger controls how often we stop parsing
 * NUL: parse all, execute, return
 * ';': parse till ';' or newline, execute, repeat till EOF
 */
#నిర్వచితం_కాకపోతే __U_BOOT__
స్థిర శూన్య parse_and_నిరంతరం_ప్రవాహం(నిర్మాణం లోపల_str *inp, పూర్ణాంకం చివర_trigger)
#లేకపోతే /* __U_BOOT__ */
స్థిర పూర్ణాంకం parse_and_నిరంతరం_ప్రవాహం(నిర్మాణం లోపల_str *inp, పూర్ణాంకం చివర_trigger)
#అయితే_ముగింపు /* __U_BOOT__ */
{
	/* Why we need empty flag?
	 * An obscure corner case "false; ``; echo $?":
	 * empty command in `` should still set $? to 0.
	 * But we can't just set $? to 0 at the start,
	 * this breaks "false; echo `echo $?`" case.
	 */
	bool ఖాళీలు = 1;
#నిర్వచితం_కాకపోతే __U_BOOT__
	అయితే (1) {
#లేకపోతే /* __U_BOOT__ */
	చేయి {
#అయితే_ముగింపు /* __U_BOOT__ */
		నిర్మాణం pipe *pipe_list;

#ఒకవేళ ENABLE_HUSH_INTERACTIVE
		ఒకవేళ (చివర_trigger == ';') {
			G.promptmode = 0; /* PS1 */
			debug_printf_prompt("%s promptmode=%d\n", __func__, G.promptmode);
		}
#అయితే_ముగింపు
		pipe_list = parse_ప్రవాహం(ఖాళీ, ఖాళీ, inp, చివర_trigger);
		ఒకవేళ (!pipe_list || pipe_list == ERR_PTR) { /* EOF/error */
			/* If we are in "big" script
			 * (not in `cmd` or something similar)...
			 */
			ఒకవేళ (pipe_list == ERR_PTR && చివర_trigger == ';') {
				/* Discard cached input (rest of line) */
				పూర్ణాంకం ch = inp->చివరి_char;
				అయితే (ch != EOF && ch != '\n') {
					//bb_error_msg("Discarded:'%c'", ch);
					ch = i_getch(inp);
				}
				/* Force prompt */
				inp->p = ఖాళీ;
				/* This stream isn't empty */
				ఖాళీలు = 0;
				కొనసాగు;
			}
			ఒకవేళ (!pipe_list && ఖాళీలు)
				G.చివరి_exitcode = 0;
			ఆపు;
		}
		debug_print_వృక్షం(pipe_list, 0);
		debug_printf_exec("parse_and_run_stream: run_and_free_list\n");
#నిర్వచితం_కాకపోతే __U_BOOT__
		నిరంతరం_and_free_list(pipe_list);
#లేకపోతే /* __U_BOOT__ */
		పూర్ణాంకం rcode = నిరంతరం_and_free_list(pipe_list);
		/*
		 * We reset input string to not run the following command, so running
		 * 'exit; echo foo' does not print foo.
		 */
		ఒకవేళ (rcode <= EXIT_RET_సంకేతం)
			అమరిక_file_లోపల_str(inp);
#అయితే_ముగింపు /* __U_BOOT__ */
		ఖాళీలు = 0;
		ఒకవేళ (G_flag_return_లోపల_పురోగతి == 1)
			ఆపు;
#నిర్వచితం_కాకపోతే __U_BOOT__
	}
#లేకపోతే /* __U_BOOT__ */
	/*
	 * This do/while is needed by run_command to avoid looping on a command
	 * with syntax error.
	 */
	} అయితే (!(G.నిరంతరం_command_flags & FLAG_EXIT_FROM_LOOP));

	తిరిగి G.చివరి_exitcode;
#అయితే_ముగింపు /* __U_BOOT__ */
}

#నిర్వచితం_కాకపోతే __U_BOOT__
స్థిర శూన్య parse_and_నిరంతరం_string(స్థిరరాశి అక్షరం *s)
#లేకపోతే /* __U_BOOT__ */
స్థిర పూర్ణాంకం parse_and_నిరంతరం_string(స్థిరరాశి అక్షరం *s)
#అయితే_ముగింపు /* __U_BOOT__ */
{
	నిర్మాణం లోపల_str ఆదానం;
#నిర్వచితం_కాకపోతే __U_BOOT__
	IF_HUSH_LINENO_VAR(గుర్తులేని sv = G.parse_lineno;)
#లేకపోతే /* __U_BOOT__ */
	//IF_HUSH_LINENO_VAR(unsigned sv = G.parse_lineno;)
#అయితే_ముగింపు /* __U_BOOT__ */

	అమరిక_string_లోపల_str(&ఆదానం, s);
#నిర్వచితం_కాకపోతే __U_BOOT__
	parse_and_నిరంతరం_ప్రవాహం(&ఆదానం, '\0');
#లేకపోతే /* __U_BOOT__ */
	తిరిగి parse_and_నిరంతరం_ప్రవాహం(&ఆదానం, '\0');
#అయితే_ముగింపు /* __U_BOOT__ */
#నిర్వచితం_కాకపోతే __U_BOOT__
	IF_HUSH_LINENO_VAR(గుర్తులేని sv = G.parse_lineno;)
#లేకపోతే /* __U_BOOT__ */
	//IF_HUSH_LINENO_VAR(unsigned sv = G.parse_lineno;)
#అయితే_ముగింపు /* __U_BOOT__ */
}

#నిర్వచితమైతే __U_BOOT__
పూర్ణాంకం parse_string_outer_modern(స్థిరరాశి అక్షరం *cmd, పూర్ణాంకం flags)
{
	పూర్ణాంకం ret;
	పూర్ణాంకం old_flags;

	/*
	 * Keep old values of run_command to be able to restore them once
	 * command was executed.
	 */
	old_flags = G.నిరంతరం_command_flags;
	G.నిరంతరం_command_flags = flags;

	ret = parse_and_నిరంతరం_string(cmd);

	G.నిరంతరం_command_flags = old_flags;

	తిరిగి ret;
}
#అయితే_ముగింపు /* __U_BOOT__ */
#నిర్వచితం_కాకపోతే __U_BOOT__
స్థిర శూన్య parse_and_నిరంతరం_file(HFILE *fp)
#లేకపోతే /* __U_BOOT__ */
శూన్య parse_and_నిరంతరం_file(శూన్య)
#అయితే_ముగింపు /* __U_BOOT__ */
{
	నిర్మాణం లోపల_str ఆదానం;
#నిర్వచితం_కాకపోతే __U_BOOT__
	IF_HUSH_LINENO_VAR(గుర్తులేని sv = G.parse_lineno;)

	IF_HUSH_LINENO_VAR(G.parse_lineno = 1;)
	అమరిక_file_లోపల_str(&ఆదానం, fp);
#లేకపోతే /* __U_BOOT__ */
	అమరిక_file_లోపల_str(&ఆదానం);
#అయితే_ముగింపు /* __U_BOOT__ */
	parse_and_నిరంతరం_ప్రవాహం(&ఆదానం, ';');
#నిర్వచితం_కాకపోతే __U_BOOT__
	IF_HUSH_LINENO_VAR(G.parse_lineno = sv;)
#అయితే_ముగింపు /* !__U_BOOT__ */
}

#నిర్వచితం_కాకపోతే __U_BOOT__
#ఒకవేళ ENABLE_HUSH_TICK
స్థిర పూర్ణాంకం generate_ప్రవాహం_from_string(స్థిరరాశి అక్షరం *s, pid_t *pid_p)
{
	pid_t pid;
	పూర్ణాంకం channel[2];
# ఒకవేళ !BB_MMU
	అక్షరం **గమ్యం_free = ఖాళీ;
# endif

	xpipe(channel);
	pid = BB_MMU ? xfork() : xvfork();
	ఒకవేళ (pid == 0) { /*బిడ్డ*/
		disable_restore_tty_pgrp_on_exit();
		/* Process substitution is not considered to be usual
		 * 'command execution'.
		 * SUSv3 says ctrl-Z should be ignored, ctrl-C should not.
		 */
		bb_సంకేతాలు(0
			+ (1 << SIGTSTP)
			+ (1 << SIGTTIN)
			+ (1 << SIGTTOU)
			, SIG_IGN);
		close(channel[0]); /* NB: close _first_, then move fd! */
		xmove_fd(channel[1], 1);
# ఒకవేళ ENABLE_HUSH_TRAP
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
		ఒకవేళ (is_prefixed_with(s, "trap")
		 && skip_whitespace(s + 4)[0] == '\0'
		) {
			స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి argv[] ALIGN_PTR = { ఖాళీ, ఖాళీ };
			builtin_trap((అక్షరం**)argv);
			fflush_all(); /* important */
			_exit(0);
		}
# endif
# ఒకవేళ BB_MMU
		/* Prevent it from trying to handle ctrl-z etc */
		IF_HUSH_పని(G.నిరంతరం_list_level = 1;)
		CLEAR_RANDOM_T(&G.random_gen); /* or else $RANDOM repeats in child */
		reset_traps_గమ్యం_defaults();
		IF_HUSH_బహుళకం_X(G.x_బహుళకం_లోతు++;)
		//bb_error_msg("%s: ++x_mode_depth=%d", __func__, G.x_mode_depth);
		parse_and_నిరంతరం_string(s);
		_exit(G.చివరి_exitcode);
# లేకపోతే
	/* We re-execute after vfork on NOMMU. This makes this script safe:
	 * yes "0123456789012345678901234567890" | dd bs=32 count=64k >BIG
	 * huge=`cat BIG` # was blocking here forever
	 * echo OK
	 */
		re_execute_shell(&గమ్యం_free,
				s,
				G.global_argv[0],
				G.global_argv + 1,
				ఖాళీ);
# endif
	}

	/*మాతృ*/
	*pid_p = pid;
# ఒకవేళ ENABLE_HUSH_FAST
	G.లెక్క_SIGCHLD++;
//bb_error_msg("[%d] fork in generate_stream_from_string:"
//		" G.count_SIGCHLD:%d G.handled_SIGCHLD:%d",
//		getpid(), G.count_SIGCHLD, G.handled_SIGCHLD);
# endif
	enable_restore_tty_pgrp_on_exit();
# ఒకవేళ !BB_MMU
	విడుదల(గమ్యం_free);
# endif
	close(channel[1]);
	తిరిగి channel[0];
}

/* Return code is exit status of the process that is run. */
స్థిర పూర్ణాంకం process_command_subs(o_string *dest, స్థిరరాశి అక్షరం *s)
{
	FILE *fp;
	pid_t pid;
	పూర్ణాంకం స్థితి_2, ch, eol_cnt;

	fp = xfdopen_for_read(generate_ప్రవాహం_from_string(s, &pid));

	/* Now send results of command back into original context */
	eol_cnt = 0;
	అయితే ((ch = getc(fp)) != EOF) {
		ఒకవేళ (ch == '\0')
			కొనసాగు;
		ఒకవేళ (ch == '\n') {
			eol_cnt++;
			కొనసాగు;
		}
		అయితే (eol_cnt) {
			o_addchr(dest, '\n');
			eol_cnt--;
		}
		o_కూడు_Qchr(dest, ch);
	}

	debug_printf("done reading from `cmd` pipe, closing it\n");
	fclose(fp);
	/* We need to extract exitcode. Test case
	 * "true; echo `sleep 1; false` $?"
	 * should print 1 */
	సురక్షిత_waitpid(pid, &స్థితి_2, 0);
	debug_printf("child exited. returning its exitcode:%d\n", WEXITSTATUS(స్థితి_2));
	తిరిగి WEXITSTATUS(స్థితి_2);
}
#అయితే_ముగింపు /* ENABLE_HUSH_TICK */

స్థిర శూన్య అమరిక_heredoc(నిర్మాణం redir_struct *redir)
{
	నిర్మాణం fd_pair pair;
	pid_t pid;
	పూర్ణాంకం len, written;
	/* the _body_ of heredoc (misleading field name) */
	స్థిరరాశి అక్షరం *heredoc = redir->rd_filename;
	అక్షరం *expanded;
#ఒకవేళ !BB_MMU
	అక్షరం **గమ్యం_free;
#అయితే_ముగింపు

	expanded = ఖాళీ;
	ఒకవేళ (!(redir->rd_dup & HEREDOC_QUOTED)) {
		expanded = encode_then_expand_string(heredoc);
		ఒకవేళ (expanded)
			heredoc = expanded;
	}
	len = అక్షరశ్రేణి_పొడవు(heredoc);

	close(redir->rd_fd); /* often saves dup2+close in xmove_fd */
	xpiped_pair(pair);
	xmove_fd(pair.rd, redir->rd_fd);

	/* Try writing without forking. Newer kernels have
	 * dynamically growing pipes. Must use non-blocking write! */
	ndelay_on(pair.wr);
	అయితే (1) {
		written = లిఖ(pair.wr, heredoc, len);
		ఒకవేళ (written <= 0)
			ఆపు;
		len -= written;
		ఒకవేళ (len == 0) {
			close(pair.wr);
			విడుదల(expanded);
			తిరిగి;
		}
		heredoc += written;
	}
	ndelay_off(pair.wr);

	/* Okay, pipe buffer was not big enough */
	/* Note: we must not create a stray child (bastard? :)
	 * for the unsuspecting parent process. Child creates a grandchild
	 * and exits before parent execs the process which consumes heredoc
	 * (that exec happens after we return from this function) */
#ఒకవేళ !BB_MMU
	గమ్యం_free = ఖాళీ;
#అయితే_ముగింపు
	pid = xvfork();
	ఒకవేళ (pid == 0) {
		/*బిడ్డ*/
		disable_restore_tty_pgrp_on_exit();
		pid = BB_MMU ? xfork() : xvfork();
		ఒకవేళ (pid != 0)
			_exit(0);
		/* grandchild */
		close(redir->rd_fd); /* read side of the pipe */
#ఒకవేళ BB_MMU
		నిండిన_write(pair.wr, heredoc, len); /* may loop or block */
		_exit(0);
#లేకపోతే
		/* Delegate blocking writes to another process */
		xmove_fd(pair.wr, STDOUT_FILENO);
		re_execute_shell(&గమ్యం_free, heredoc, ఖాళీ, ఖాళీ, ఖాళీ);
#అయితే_ముగింపు
	}
	/*మాతృ*/
#ఒకవేళ ENABLE_HUSH_FAST
	G.లెక్క_SIGCHLD++;
//bb_error_msg("[%d] fork in setup_heredoc: G.count_SIGCHLD:%d G.handled_SIGCHLD:%d", getpid(), G.count_SIGCHLD, G.handled_SIGCHLD);
#అయితే_ముగింపు
	enable_restore_tty_pgrp_on_exit();
#ఒకవేళ !BB_MMU
	విడుదల(గమ్యం_free);
#అయితే_ముగింపు
	close(pair.wr);
	విడుదల(expanded);
	wait(ఖాళీ); /* wait till child has died */
}

నిర్మాణం squirrel {
	పూర్ణాంకం orig_fd;
	పూర్ణాంకం moved_గమ్యం;
	/* moved_to = n: fd was moved to n; restore back to orig_fd after redir */
	/* moved_to = -1: fd was opened by redirect; close orig_fd after redir */
};

స్థిర నిర్మాణం squirrel *append_squirrel(నిర్మాణం squirrel *sq, పూర్ణాంకం i, పూర్ణాంకం orig, పూర్ణాంకం moved)
{
	sq = xrealloc(sq, (i + 2) * పరిమాణం(sq[0]));
	sq[i].orig_fd = orig;
	sq[i].moved_గమ్యం = moved;
	sq[i+1].orig_fd = -1; /* end marker */
	తిరిగి sq;
}

స్థిర నిర్మాణం squirrel *కూడు_squirrel(నిర్మాణం squirrel *sq, పూర్ణాంకం fd, పూర్ణాంకం avoid_fd)
{
	పూర్ణాంకం moved_గమ్యం;
	పూర్ణాంకం i;

	i = 0;
	ఒకవేళ (sq) కొరకు (; sq[i].orig_fd >= 0; i++) {
		/* If we collide with an already moved fd... */
		ఒకవేళ (fd == sq[i].moved_గమ్యం) {
			moved_గమ్యం = dup_CLOEXEC(sq[i].moved_గమ్యం, avoid_fd);
			debug_printf_redir("redirect_fd %d: already busy, moving to %d\n", fd, moved_గమ్యం);
			ఒకవేళ (moved_గమ్యం < 0) {
				/* "echo 2>/dev/tty 10>&9999" testcase:
				 * We move fd 2 to 10, then discover we need to move fd 10
				 * (and not hit 9999) and the latter fails.
				 */
				తిరిగి ఖాళీ; /* fcntl failed */
			}
			sq[i].moved_గమ్యం = moved_గమ్యం;
			తిరిగి sq;
		}
		ఒకవేళ (fd == sq[i].orig_fd) {
			/* Example: echo Hello >/dev/null 1>&2 */
			debug_printf_redir("redirect_fd %d: already moved\n", fd);
			తిరిగి sq;
		}
	}

	/* If this fd is open, we move and remember it; if it's closed, moved_to = -1 */
	moved_గమ్యం = dup_CLOEXEC(fd, avoid_fd);
	debug_printf_redir("redirect_fd %d: previous fd is moved to %d (-1 if it was closed)\n", fd, moved_గమ్యం);
	ఒకవేళ (moved_గమ్యం < 0 && errno != EBADF)
		తిరిగి ఖాళీ; /* fcntl failed (not because fd is closed) */
	తిరిగి append_squirrel(sq, i, fd, moved_గమ్యం);
}

స్థిర నిర్మాణం squirrel *కూడు_squirrel_closed(నిర్మాణం squirrel *sq, పూర్ణాంకం fd)
{
	పూర్ణాంకం i;

	i = 0;
	ఒకవేళ (sq) కొరకు (; sq[i].orig_fd >= 0; i++) {
		/* If we collide with an already moved fd... */
		ఒకవేళ (fd == sq[i].orig_fd) {
			/* Examples:
			 * "echo 3>FILE 3>&- 3>FILE"
			 * "echo 3>&- 3>FILE"
			 * No need for last redirect to insert
			 * another "need to close 3" indicator.
			 */
			debug_printf_redir("redirect_fd %d: already moved or closed\n", fd);
			తిరిగి sq;
		}
	}

	debug_printf_redir("redirect_fd %d: previous fd was closed\n", fd);
	తిరిగి append_squirrel(sq, i, fd, -1);
}

/* fd: redirect wants this fd to be used (e.g. 3>file).
 * Move all conflicting internally used fds,
 * and remember them so that we can restore them later.
 */
స్థిర పూర్ణాంకం save_fd_on_redirect(పూర్ణాంకం fd, పూర్ణాంకం avoid_fd, నిర్మాణం squirrel **sqp)
{
	నిర్మాణం squirrel *new_squirrel;

	ఒకవేళ (avoid_fd < 9) /* the important case here is that it can be -1 */
		avoid_fd = 9;

#ఒకవేళ ENABLE_HUSH_INTERACTIVE
	ఒకవేళ (fd != 0 /* don't trigger for G_interactive_fd == 0 (that's "not interactive" flag) */
	 && fd == G_interactive_fd
	) {
		/* Testcase: "ls -l /proc/$$/fd 255>&-" should work */
		G_interactive_fd = xdup_CLOEXEC_and_close(G_interactive_fd, avoid_fd);
		debug_printf_redir("redirect_fd %d: matches interactive_fd, moving it to %d\n", fd, G_interactive_fd);
		తిరిగి 1; /* "we closed fd" */
	}
#అయితే_ముగింపు
	/* Are we called from setup_redirects(squirrel==NULL)
	 * in redirect in a [v]forked child?
	 */
	ఒకవేళ (sqp == ఖాళీ) {
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
		తిరిగి 0;
	}

	/* If this one of script's fds? */
	ఒకవేళ (move_HFILEs_on_redirect(fd, avoid_fd))
		తిరిగి 1; /* yes. "we closed fd" (actually moved it) */

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
	ఒకవేళ (sqp == ERR_PTR) {
		/* Don't preserve redirected fds: exec is _meant_ to change these */
		//bb_error_msg("sqp == ERR_PTR: exec >FILE");
		తిరిగి 0;
	}

	/* Check whether it collides with any open fds (e.g. stdio), save fds as needed */
	new_squirrel = కూడు_squirrel(*sqp, fd, avoid_fd);
	ఒకవేళ (!new_squirrel)
		తిరిగి -1; /* redirect error */
	*sqp = new_squirrel;
	తిరిగి 0; /* "we did not close fd" */
}

స్థిర శూన్య restore_redirects(నిర్మాణం squirrel *sq)
{
	ఒకవేళ (sq) {
		పూర్ణాంకం i;
		కొరకు (i = 0; sq[i].orig_fd >= 0; i++) {
			ఒకవేళ (sq[i].moved_గమ్యం >= 0) {
				/* We simply die on error */
				debug_printf_redir("restoring redirected fd from %d to %d\n", sq[i].moved_గమ్యం, sq[i].orig_fd);
				xmove_fd(sq[i].moved_గమ్యం, sq[i].orig_fd);
			} లేకపోతే {
				/* cmd1 9>FILE; cmd2_should_see_fd9_closed */
				debug_printf_redir("restoring redirected fd %d: closing it\n", sq[i].orig_fd);
				close(sq[i].orig_fd);
			}
		}
		విడుదల(sq);
	}
	ఒకవేళ (G.HFILE_stdin
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

#ఒకవేళ ENABLE_FEATURE_SH_STANDALONE && BB_MMU
స్థిర శూన్య close_saved_fds_and_FILE_fds(శూన్య)
{
	ఒకవేళ (G_interactive_fd)
		close(G_interactive_fd);
	close_all_HFILE_list();
}
#అయితే_ముగింపు

స్థిర పూర్ణాంకం internally_opened_fd(పూర్ణాంకం fd, నిర్మాణం squirrel *sq)
{
	పూర్ణాంకం i;

#ఒకవేళ ENABLE_HUSH_INTERACTIVE
	ఒకవేళ (fd == G_interactive_fd)
		తిరిగి 1;
#అయితే_ముగింపు
	/* If this one of script's fds? */
	ఒకవేళ (fd_లోపల_HFILEs(fd))
		తిరిగి 1;

	ఒకవేళ (sq) కొరకు (i = 0; sq[i].orig_fd >= 0; i++) {
		ఒకవేళ (fd == sq[i].moved_గమ్యం)
			తిరిగి 1;
	}
	తిరిగి 0;
}

/* sqp != NULL means we squirrel away copies of stdin, stdout,
 * and stderr if they are redirected.
 * If redirection fails, return 1. This will make caller
 * skip command execution and restore already created redirect fds.
 */
స్థిర పూర్ణాంకం అమరిక_redirects(నిర్మాణం command *prog, నిర్మాణం squirrel **sqp)
{
	నిర్మాణం redir_struct *redir;

	కొరకు (redir = prog->redirects; redir; redir = redir->తదుపరి) {
		పూర్ణాంకం newfd;
		పూర్ణాంకం closed;

		ఒకవేళ (redir->rd_type == REDIRECT_HEREDOC2) {
			/* "rd_fd<<HERE" case */
			ఒకవేళ (save_fd_on_redirect(redir->rd_fd, /*avoid:*/ 0, sqp) < 0)
				తిరిగి 1;
			/* for REDIRECT_HEREDOC2, rd_filename holds _contents_
			 * of the heredoc */
			debug_printf_redir("set heredoc '%s'\n",
					redir->rd_filename);
			అమరిక_heredoc(redir);
			కొనసాగు;
		}

		ఒకవేళ (redir->rd_dup == REDIRFD_గమ్యం_FILE) {
			/* "rd_fd<*>file" case (<*> is <,>,>>,<>) */
			అక్షరం *p;
			పూర్ణాంకం బహుళకం;

			ఒకవేళ (redir->rd_filename == ఖాళీ) {
				/* Examples:
				 * "cmd >" (no filename)
				 * "cmd > <file" (2nd redirect starts too early)
				 */
				syntax_error("invalid redirect");
				తిరిగి 1;
			}
			బహుళకం = redir_పట్టిక[redir->rd_type].బహుళకం;
			p = expand_string_గమ్యం_string(redir->rd_filename,
				EXP_FLAG_ESC_GLOB_CHARS, /*unbackslash:*/ 1);
			newfd = open_or_warn(p, బహుళకం);
			విడుదల(p);
			ఒకవేళ (newfd < 0) {
				/* Error message from open_or_warn can be lost
				 * if stderr has been redirected, but bash
				 * and ash both lose it as well
				 * (though zsh doesn't!)
				 */
				తిరిగి 1;
			}
			ఒకవేళ (newfd == redir->rd_fd && sqp
			 && sqp != ERR_PTR /* not a redirect in "exec" */
			) {
				/* open() gave us precisely the fd we wanted.
				 * This means that this fd was not busy
				 * (not opened to anywhere).
				 * Remember to close it on restore:
				 */
				*sqp = కూడు_squirrel_closed(*sqp, newfd);
				debug_printf_redir("redir to previously closed fd %d\n", newfd);
			}
		} లేకపోతే {
			/* "rd_fd>&rd_dup" or "rd_fd>&-" case */
			newfd = redir->rd_dup;
		}

		ఒకవేళ (newfd == redir->rd_fd)
			కొనసాగు;

		/* if "N>FILE": move newfd to redir->rd_fd */
		/* if "N>&M": dup newfd to redir->rd_fd */
		/* if "N>&-": close redir->rd_fd (newfd is REDIRFD_CLOSE) */

		closed = save_fd_on_redirect(redir->rd_fd, /*avoid:*/ newfd, sqp);
		ఒకవేళ (closed < 0)
			తిరిగి 1; /* error */
		ఒకవేళ (newfd == REDIRFD_CLOSE) {
			/* "N>&-" means "close me" */
			ఒకవేళ (!closed) {
				/* ^^^ optimization: saving may already
				 * have closed it. If not... */
				close(redir->rd_fd);
			}
			/* Sometimes we do another close on restore, getting EBADF.
			 * Consider "echo 3>FILE 3>&-"
			 * first redirect remembers "need to close 3",
			 * and second redirect closes 3! Restore code then closes 3 again.
			 */
		} లేకపోతే {
			/* if newfd is a script fd or saved fd, do not allow to use it */
			ఒకవేళ (internally_opened_fd(newfd, sqp && sqp != ERR_PTR ? *sqp : ఖాళీ)) {
				bb_error_సందేశం("fd#%d is not open", newfd);
				తిరిగి 1;
			}
			ఒకవేళ (dup2(newfd, redir->rd_fd) < 0) {
				/* "echo >&99" testcase */
				bb_perror_సందేశం("dup2(%d,%d)", newfd, redir->rd_fd);
				తిరిగి 1;
			}
			ఒకవేళ (redir->rd_dup == REDIRFD_గమ్యం_FILE)
				/* "rd_fd > FILE" */
				close(newfd);
			/* else: "rd_fd > rd_dup" */
		}
	}
	తిరిగి 0;
}

స్థిర అక్షరం *కనుగొను_లోపల_మార్గం(స్థిరరాశి అక్షరం *arg)
{
	అక్షరం *ret = ఖాళీ;
	స్థిరరాశి అక్షరం *మార్గం = get_local_var_విలువ("PATH");

	ఒకవేళ (!మార్గం)
		తిరిగి ఖాళీ;

	అయితే (1) {
		స్థిరరాశి అక్షరం *చివర_2 = strchrnul(మార్గం, ':');
		పూర్ణాంకం sz = చివర_2 - మార్గం; /* must be int! */

		విడుదల(ret);
		ఒకవేళ (sz != 0) {
			ret = xasprintf("%.*s/%s", sz, మార్గం, arg);
		} లేకపోతే {
			/* We have xxx::yyyy in $PATH,
			 * it means "use current dir" */
			ret = xstrdup(arg);
		}
		ఒకవేళ (access(ret, F_OK) == 0)
			ఆపు;

		ఒకవేళ (*చివర_2 == '\0') {
			విడుదల(ret);
			తిరిగి ఖాళీ;
		}
		మార్గం = చివర_2 + 1;
	}

	తిరిగి ret;
}

స్థిర స్థిరరాశి నిర్మాణం built_లోపల_command *కనుగొను_builtin_helper(స్థిరరాశి అక్షరం *name,
		స్థిరరాశి నిర్మాణం built_లోపల_command *x,
		స్థిరరాశి నిర్మాణం built_లోపల_command *చివర_2)
{
	అయితే (x != చివర_2) {
		ఒకవేళ (strcmp(name, x->b_cmd) != 0) {
			x++;
			కొనసాగు;
		}
		debug_printf_exec("found builtin '%s'\n", name);
		తిరిగి x;
	}
	తిరిగి ఖాళీ;
}
స్థిర స్థిరరాశి నిర్మాణం built_లోపల_command *కనుగొను_builtin1(స్థిరరాశి అక్షరం *name)
{
	తిరిగి కనుగొను_builtin_helper(name, bltins1, &bltins1[ARRAY_కొలత(bltins1)]);
}
స్థిర స్థిరరాశి నిర్మాణం built_లోపల_command *కనుగొను_builtin(స్థిరరాశి అక్షరం *name)
{
	స్థిరరాశి నిర్మాణం built_లోపల_command *x = కనుగొను_builtin1(name);
	ఒకవేళ (x)
		తిరిగి x;
	తిరిగి కనుగొను_builtin_helper(name, bltins2, &bltins2[ARRAY_కొలత(bltins2)]);
}

#ఒకవేళ ENABLE_HUSH_పని && ENABLE_FEATURE_TAB_COMPLETION
స్థిర స్థిరరాశి అక్షరం * FAST_FUNC hush_command_name(పూర్ణాంకం i)
{
	ఒకవేళ (/*i >= 0 && */ i < ARRAY_కొలత(bltins1)) {
		తిరిగి bltins1[i].b_cmd;
	}
	i -= ARRAY_కొలత(bltins1);
	ఒకవేళ (i < ARRAY_కొలత(bltins2)) {
		తిరిగి bltins2[i].b_cmd;
	}
# ఒకవేళ ENABLE_HUSH_FUNCTIONS
	{
		నిర్మాణం function *funcp;
		i -= ARRAY_కొలత(bltins2);
		కొరకు (funcp = G.శిఖరం_func; funcp; funcp = funcp->తదుపరి) {
			ఒకవేళ (--i < 0)
				తిరిగి funcp->name;
		}
	}
# endif
	తిరిగి ఖాళీ;
}
#అయితే_ముగింపు
#అయితే_ముగింపు /* !__U_BOOT__ */

#నిర్వచితం_కాకపోతే __U_BOOT__
స్థిర శూన్య remove_nested_vars(శూన్య)
{
	నిర్మాణం variable *cur;
	నిర్మాణం variable **cur_pp;

	cur_pp = &G.శిఖరం_var;
	అయితే ((cur = *cur_pp) != ఖాళీ) {
		ఒకవేళ (cur->var_nest_level <= G.var_nest_level) {
			cur_pp = &cur->తదుపరి;
			కొనసాగు;
		}
		/* Unexport */
		ఒకవేళ (cur->flg_export) {
			debug_printf_env("unexporting nested '%s'/%u\n", cur->varstr, cur->var_nest_level);
			bb_unsetenv(cur->varstr);
		}
		/* Remove from global list */
		*cur_pp = cur->తదుపరి;
		/* Free */
		ఒకవేళ (!cur->max_len) {
			debug_printf_env("freeing nested '%s'/%u\n", cur->varstr, cur->var_nest_level);
			విడుదల(cur->varstr);
		}
		విడుదల(cur);
	}
}

స్థిర శూన్య enter_var_nest_level(శూన్య)
{
	G.var_nest_level++;
	debug_printf_env("var_nest_level++ %u\n", G.var_nest_level);

	/* Try:	f() { echo -n .; f; }; f
	 * struct variable::var_nest_level is uint16_t,
	 * thus limiting recursion to < 2^16.
	 * In any case, with 8 Mbyte stack SEGV happens
	 * not too long after 2^16 recursions anyway.
	 */
	ఒకవేళ (G.var_nest_level > 0xff00)
		bb_error_సందేశం_and_die("fatal recursion (depth %u)", G.var_nest_level);
}

స్థిర శూన్య leave_var_nest_level(శూన్య)
{
	G.var_nest_level--;
	debug_printf_env("var_nest_level-- %u\n", G.var_nest_level);
	ఒకవేళ (HUSH_DEBUG && (పూర్ణాంకం)G.var_nest_level < 0)
		bb_simple_error_సందేశం_and_die("BUG: nesting underflow");

	remove_nested_vars();
}
#అయితే_ముగింపు /* __U_BOOT__ */

#ఒకవేళ ENABLE_HUSH_FUNCTIONS
స్థిర నిర్మాణం function **కనుగొను_function_స్థానం(స్థిరరాశి అక్షరం *name)
{
	నిర్మాణం function *funcp;
	నిర్మాణం function **funcpp = &G.శిఖరం_func;

	అయితే ((funcp = *funcpp) != ఖాళీ) {
		ఒకవేళ (strcmp(name, funcp->name) == 0) {
			debug_printf_exec("found function '%s'\n", name);
			ఆపు;
		}
		funcpp = &funcp->తదుపరి;
	}
	తిరిగి funcpp;
}

స్థిర ALWAYS_INLINE స్థిరరాశి నిర్మాణం function *కనుగొను_function(స్థిరరాశి అక్షరం *name)
{
	స్థిరరాశి నిర్మాణం function *funcp = *కనుగొను_function_స్థానం(name);
	తిరిగి funcp;
}

/* Note: takes ownership on name ptr */
స్థిర నిర్మాణం function *new_function(అక్షరం *name)
{
	నిర్మాణం function **funcpp = కనుగొను_function_స్థానం(name);
	నిర్మాణం function *funcp = *funcpp;

	ఒకవేళ (funcp != ఖాళీ) {
		నిర్మాణం command *cmd = funcp->మాతృ_cmd;
		debug_printf_exec("func %p parent_cmd %p\n", funcp, cmd);
		ఒకవేళ (!cmd) {
			debug_printf_exec("freeing & replacing function '%s'\n", funcp->name);
			విడుదల(funcp->name);
			/* Note: if !funcp->body, do not free body_as_string!
			 * This is a special case of "-F name body" function:
			 * body_as_string was not malloced! */
			ఒకవేళ (funcp->body) {
				free_pipe_list(funcp->body);
# ఒకవేళ !BB_MMU
				విడుదల(funcp->body_as_string);
# endif
			}
		} లేకపోతే {
			debug_printf_exec("reinserting in tree & replacing function '%s'\n", funcp->name);
			cmd->argv[0] = funcp->name;
			cmd->సమూహం = funcp->body;
# ఒకవేళ !BB_MMU
			cmd->సమూహం_as_string = funcp->body_as_string;
# endif
		}
	} లేకపోతే {
		debug_printf_exec("remembering new function '%s'\n", name);
		funcp = *funcpp = xzalloc(పరిమాణం(*funcp));
		/*funcp->next = NULL;*/
	}

	funcp->name = name;
	తిరిగి funcp;
}

# ఒకవేళ ENABLE_HUSH_UNSET
స్థిర శూన్య unset_func(స్థిరరాశి అక్షరం *name)
{
	నిర్మాణం function **funcpp = కనుగొను_function_స్థానం(name);
	నిర్మాణం function *funcp = *funcpp;

	ఒకవేళ (funcp != ఖాళీ) {
		debug_printf_exec("freeing function '%s'\n", funcp->name);
		*funcpp = funcp->తదుపరి;
		/* funcp is unlinked now, deleting it.
		 * Note: if !funcp->body, the function was created by
		 * "-F name body", do not free ->body_as_string
		 * and ->name as they were not malloced. */
		ఒకవేళ (funcp->body) {
			free_pipe_list(funcp->body);
			విడుదల(funcp->name);
#  ఒకవేళ !BB_MMU
			విడుదల(funcp->body_as_string);
#  endif
		}
		విడుదల(funcp);
	}
}
# endif

# ఒకవేళ BB_MMU
#నిర్వచించు exec_function(గమ్యం_free, funcp, argv) \
	exec_function(funcp, argv)
# endif
స్థిర శూన్య exec_function(అక్షరం ***గమ్యం_free,
		స్థిరరాశి నిర్మాణం function *funcp,
		అక్షరం **argv) NORETURN;
స్థిర శూన్య exec_function(అక్షరం ***గమ్యం_free,
		స్థిరరాశి నిర్మాణం function *funcp,
		అక్షరం **argv)
{
# ఒకవేళ BB_MMU
	పూర్ణాంకం n;

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
	G_flag_return_లోపల_పురోగతి = -1;
	enter_var_nest_level();
	IF_HUSH_LOCAL(G.func_nest_level++;)

	/* On MMU, funcp->body is always non-NULL */
	n = నిరంతరం_list(funcp->body);
	_exit(n);
# లేకపోతే
//?	close_saved_fds_and_FILE_fds();

//TODO: check whether "true | func_with_return" works

	re_execute_shell(గమ్యం_free,
			funcp->body_as_string,
			G.global_argv[0],
			argv + 1,
			ఖాళీ);
# endif
}

స్థిర పూర్ణాంకం నిరంతరం_function(స్థిరరాశి నిర్మాణం function *funcp, అక్షరం **argv)
{
	పూర్ణాంకం rc;
	save_arg_t sv;
	smallint sv_flg;

	save_and_replace_G_వాదనలు(&sv, argv);

	/* "We are in function, ok to use return" */
	sv_flg = G_flag_return_లోపల_పురోగతి;
	G_flag_return_లోపల_పురోగతి = -1;

	/* Make "local" variables properly shadow previous ones */
	IF_HUSH_LOCAL(enter_var_nest_level();)
	IF_HUSH_LOCAL(G.func_nest_level++;)

	/* On MMU, funcp->body is always non-NULL */
# ఒకవేళ !BB_MMU
	ఒకవేళ (!funcp->body) {
		/* Function defined by -F */
		parse_and_నిరంతరం_string(funcp->body_as_string);
		rc = G.చివరి_exitcode;
	} లేకపోతే
# endif
	{
		rc = నిరంతరం_list(funcp->body);
	}

	IF_HUSH_LOCAL(G.func_nest_level--;)
	IF_HUSH_LOCAL(leave_var_nest_level();)

	G_flag_return_లోపల_పురోగతి = sv_flg;
# ఒకవేళ ENABLE_HUSH_TRAP
	debug_printf_exec("G.return_exitcode=-1\n");
	G.return_exitcode = -1; /* invalidate stashed return value */
# endif

	restore_G_వాదనలు(&sv, argv);

	తిరిగి rc;
}
#అయితే_ముగింపు /* ENABLE_HUSH_FUNCTIONS */

#నిర్వచితం_కాకపోతే __U_BOOT__
#ఒకవేళ BB_MMU
#నిర్వచించు exec_builtin(గమ్యం_free, x, argv) \
	exec_builtin(x, argv)
#లేకపోతే
#నిర్వచించు exec_builtin(గమ్యం_free, x, argv) \
	exec_builtin(గమ్యం_free, argv)
#అయితే_ముగింపు
స్థిర శూన్య exec_builtin(అక్షరం ***గమ్యం_free,
		స్థిరరాశి నిర్మాణం built_లోపల_command *x,
		అక్షరం **argv) NORETURN;
స్థిర శూన్య exec_builtin(అక్షరం ***గమ్యం_free,
		స్థిరరాశి నిర్మాణం built_లోపల_command *x,
		అక్షరం **argv)
{
#ఒకవేళ BB_MMU
	పూర్ణాంకం rcode;
//?	close_saved_fds_and_FILE_fds();
	rcode = x->b_function(argv);
	fflush_all();
	_exit(rcode);
#లేకపోతే
	fflush_all();
	/* On NOMMU, we must never block!
	 * Example: { sleep 99 | read line; } & echo Ok
	 */
	re_execute_shell(గమ్యం_free,
			argv[0],
			G.global_argv[0],
			G.global_argv + 1,
			argv);
#అయితే_ముగింపు
}
#అయితే_ముగింపు /* !__U_BOOT__ */

#నిర్వచితం_కాకపోతే __U_BOOT__
స్థిర శూన్య execvp_or_die(అక్షరం **argv) NORETURN;
స్థిర శూన్య execvp_or_die(అక్షరం **argv)
{
	పూర్ణాంకం e;
	debug_printf_exec("execing '%s'\n", argv[0]);
	/* Don't propagate SIG_IGN to the child */
	ఒకవేళ (SPECIAL_JOBSTOP_SIGS != 0)
		switch_off_special_sigs(G.special_sig_ముసుగు & SPECIAL_JOBSTOP_SIGS);
	execvp(argv[0], argv);
	e = 2;
	ఒకవేళ (errno == EACCES) e = 126;
	ఒకవేళ (errno == ENOENT) e = 127;
	bb_perror_సందేశం("can't execute '%s'", argv[0]);
	_exit(e);
}

#ఒకవేళ ENABLE_HUSH_బహుళకం_X
స్థిర శూన్య x_బహుళకం_print_optionally_squoted(స్థిరరాశి అక్షరం *str)
{
	గుర్తులేని len;
	స్థిరరాశి అక్షరం *cp;

	cp = str;

	/* the set of chars which-cause-string-to-be-squoted mimics bash */
	/* test a char with: bash -c 'set -x; echo "CH"' */
	ఒకవేళ (str[strcspn(str, "\\\"'`$(){}[]<>;#&|~*?!^"
			" " "\001\002\003\004\005\006\007"
			"\010\011\012\013\014\015\016\017"
			"\020\021\022\023\024\025\026\027"
			"\030\031\032\033\034\035\036\037"
			)
		] == '\0'
	) {
		/* string has no special chars */
		x_బహుళకం_addstr(str);
		తిరిగి;
	}

	cp = str;
	కొరకు (;;) {
		/* print '....' up to EOL or first squote */
		len = (పూర్ణాంకం)(strchrnul(cp, '\'') - cp);
		ఒకవేళ (len != 0) {
			x_బహుళకం_addchr('\'');
			x_బహుళకం_addblock(cp, len);
			x_బహుళకం_addchr('\'');
			cp += len;
		}
		ఒకవేళ (*cp == '\0')
			ఆపు;
		/* string contains squote(s), print them as \' */
		x_బహుళకం_addchr('\\');
		x_బహుళకం_addchr('\'');
		cp++;
	}
}
స్థిర శూన్య dump_cmd_లోపల_x_బహుళకం(అక్షరం **argv)
{
	ఒకవేళ (G_x_బహుళకం && argv) {
		గుర్తులేని n;

		/* "+[+++...][ cmd...]\n\0" */
		x_బహుళకం_ఉపసర్గ();
		n = 0;
		అయితే (argv[n]) {
			x_బహుళకం_addchr(' ');
			ఒకవేళ (argv[n][0] == '\0') {
				x_బహుళకం_addchr('\'');
				x_బహుళకం_addchr('\'');
			} లేకపోతే {
				x_బహుళకం_print_optionally_squoted(argv[n]);
			}
			n++;
		}
		x_బహుళకం_flush();
	}
}
#లేకపోతే
# define dump_cmd_లోపల_x_బహుళకం(argv) ((శూన్య)0)
#అయితే_ముగింపు
#అయితే_ముగింపు /* !__U_BOOT__ */

#నిర్వచితం_కాకపోతే __U_BOOT__
#ఒకవేళ ENABLE_HUSH_COMMAND
స్థిర శూన్య if_command_vV_print_and_exit(అక్షరం opt_vV, అక్షరం *cmd, స్థిరరాశి అక్షరం *explanation)
{
	అక్షరం *గమ్యం_free;

	ఒకవేళ (!opt_vV)
		తిరిగి;

	గమ్యం_free = ఖాళీ;
	ఒకవేళ (!explanation) {
		అక్షరం *మార్గం_2 = getenv("PATH");
		explanation = గమ్యం_free = కనుగొను_executable(cmd, &మార్గం_2); /* path == NULL is ok */
		ఒకవేళ (!explanation)
			_exit(1); /* PROG was not found */
		ఒకవేళ (opt_vV != 'V')
			cmd = గమ్యం_free; /* -v PROG prints "/path/to/PROG" */
	}
	రాయి((opt_vV == 'V') ? "%s is %s\n" : "%s\n", cmd, explanation);
	విడుదల(గమ్యం_free);
	fflush_all();
	_exit(0);
}
#లేకపోతే
# define if_command_vV_print_and_exit(a,b,c) ((శూన్య)0)
#అయితే_ముగింపు
#అయితే_ముగింపు /* !__U_BOOT__ */

#ఒకవేళ BB_MMU
#నిర్వచించు pseudo_exec_argv(nommu_save, argv, assignment_cnt, argv_expanded) \
	pseudo_exec_argv(argv, assignment_cnt, argv_expanded)
#నిర్వచించు pseudo_exec(nommu_save, command, argv_expanded) \
	pseudo_exec(command, argv_expanded)
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే __U_BOOT__
/* Called after [v]fork() in run_pipe, or from builtin_exec.
 * Never returns.
 * Don't exit() here.  If you don't exec, use _exit instead.
 * The at_exit handlers apparently confuse the calling process,
 * in particular stdin handling. Not sure why? -- because of vfork! (vda)
 */
స్థిర శూన్య pseudo_exec_argv(nommu_save_t *nommu_save,
		అక్షరం **argv, పూర్ణాంకం assignment_cnt,
		అక్షరం **argv_expanded) NORETURN;
స్థిర NOINLINE శూన్య pseudo_exec_argv(nommu_save_t *nommu_save,
		అక్షరం **argv, పూర్ణాంకం assignment_cnt,
		అక్షరం **argv_expanded)
{
	స్థిరరాశి నిర్మాణం built_లోపల_command *x;
	నిర్మాణం variable **sv_shadowed;
	అక్షరం **new_env;
	IF_HUSH_COMMAND(అక్షరం opt_vV = 0;)
	IF_HUSH_FUNCTIONS(స్థిరరాశి నిర్మాణం function *funcp;)

	new_env = expand_assignments(argv, assignment_cnt);
	dump_cmd_లోపల_x_బహుళకం(new_env);

	ఒకవేళ (!argv[assignment_cnt]) {
		/* Case when we are here: ... | var=val | ...
		 * (note that we do not exit early, i.e., do not optimize out
		 * expand_assignments(): think about ... | var=`sleep 1` | ...
		 */
		free_అక్షరశ్రేణులు(new_env);
		_exit_SUCCESS();
	}

	sv_shadowed = G.shadowed_vars_pp;
#ఒకవేళ BB_MMU
	G.shadowed_vars_pp = ఖాళీ; /* "don't save, free them instead" */
#లేకపోతే
	G.shadowed_vars_pp = &nommu_save->old_vars;
	G.var_nest_level++;
#అయితే_ముగింపు
	సమితి_vars_and_save_old(new_env);
	G.shadowed_vars_pp = sv_shadowed;

	ఒకవేళ (argv_expanded) {
		argv = argv_expanded;
	} లేకపోతే {
		argv = expand_strvec_గమ్యం_strvec(argv + assignment_cnt);
#ఒకవేళ !BB_MMU
		nommu_save->argv = argv;
#అయితే_ముగింపు
	}
	dump_cmd_లోపల_x_బహుళకం(argv);

#ఒకవేళ ENABLE_FEATURE_SH_STANDALONE || BB_MMU
	ఒకవేళ (strchr(argv[0], '/') != ఖాళీ)
		వెళ్ళు skip;
#అయితే_ముగింపు

#ఒకవేళ ENABLE_HUSH_FUNCTIONS
	/* Check if the command matches any functions (this goes before bltins) */
	funcp = కనుగొను_function(argv[0]);
	ఒకవేళ (funcp)
		exec_function(&nommu_save->argv_from_re_execing, funcp, argv);
#అయితే_ముగింపు

#ఒకవేళ ENABLE_HUSH_COMMAND
	/* "command BAR": run BAR without looking it up among functions
	 * "command -v BAR": print "BAR" or "/path/to/BAR"; or exit 1
	 * "command -V BAR": print "BAR is {a function,a shell builtin,/path/to/BAR}"
	 */
	అయితే (strcmp(argv[0], "command") == 0 && argv[1]) {
		అక్షరం *p;

		argv++;
		p = *argv;
		ఒకవేళ (p[0] != '-' || !p[1])
			కొనసాగు; /* bash allows "command command command [-OPT] BAR" */

		కొరకు (;;) {
			p++;
			ఎంపిక (*p) {
			స్థితి '\0':
				argv++;
				p = *argv;
				ఒకవేళ (p[0] != '-' || !p[1])
					వెళ్ళు after_opts;
				కొనసాగు; /* next arg is also -opts, process it too */
			స్థితి 'v':
			స్థితి 'V':
				opt_vV = *p;
				కొనసాగు;
			అప్రమేయ:
				bb_error_సందేశం_and_die("%s: %s: invalid option", "command", argv[0]);
			}
		}
	}
 after_opts:
# ఒకవేళ ENABLE_HUSH_FUNCTIONS
	ఒకవేళ (opt_vV && కనుగొను_function(argv[0]))
		if_command_vV_print_and_exit(opt_vV, argv[0], "a function");
# endif
#అయితే_ముగింపు

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
	x = BB_MMU ? కనుగొను_builtin(argv[0]) : కనుగొను_builtin1(argv[0]);
	ఒకవేళ (x) {
		if_command_vV_print_and_exit(opt_vV, argv[0], "a shell builtin");
		exec_builtin(&nommu_save->argv_from_re_execing, x, argv);
	}

#ఒకవేళ ENABLE_FEATURE_SH_STANDALONE
	/* Check if the command matches any busybox applets */
	{
		పూర్ణాంకం a = కనుగొను_applet_by_name(argv[0]);
		ఒకవేళ (a >= 0) {
			if_command_vV_print_and_exit(opt_vV, argv[0], "an applet");
# ఒకవేళ BB_MMU /* see above why on NOMMU it is not allowed */
			ఒకవేళ (APPLET_IS_NOEXEC(a)) {
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
				switch_off_special_sigs(G.special_sig_ముసుగు);
				debug_printf_exec("running applet '%s'\n", argv[0]);
				నిరంతరం_noexec_applet_and_exit(a, argv[0], argv);
			}
# endif
			/* Re-exec ourselves */
			debug_printf_exec("re-execing applet '%s'\n", argv[0]);
			/* Don't propagate SIG_IGN to the child */
			ఒకవేళ (SPECIAL_JOBSTOP_SIGS != 0)
				switch_off_special_sigs(G.special_sig_ముసుగు & SPECIAL_JOBSTOP_SIGS);
			execv(bb_busybox_exec_మార్గం, argv);
			/* If they called chroot or otherwise made the binary no longer
			 * executable, fall through */
		}
	}
#అయితే_ముగింపు

#ఒకవేళ ENABLE_FEATURE_SH_STANDALONE || BB_MMU
 skip:
#అయితే_ముగింపు
	if_command_vV_print_and_exit(opt_vV, argv[0], ఖాళీ);
	execvp_or_die(argv);
}

/* Called after [v]fork() in run_pipe
 */
స్థిర శూన్య pseudo_exec(nommu_save_t *nommu_save,
		నిర్మాణం command *command,
		అక్షరం **argv_expanded) NORETURN;
స్థిర శూన్య pseudo_exec(nommu_save_t *nommu_save,
		నిర్మాణం command *command,
		అక్షరం **argv_expanded)
{
#ఒకవేళ ENABLE_HUSH_FUNCTIONS
	ఒకవేళ (command->cmd_type == CMD_FUNCDEF) {
		/* Ignore funcdefs in pipes:
		 * true | f() { cmd }
		 */
		_exit(0);
	}
#అయితే_ముగింపు

	ఒకవేళ (command->argv) {
		pseudo_exec_argv(nommu_save, command->argv,
				command->assignment_cnt, argv_expanded);
	}

	ఒకవేళ (command->సమూహం) {
		/* Cases when we are here:
		 * ( list )
		 * { list } &
		 * ... | ( list ) | ...
		 * ... | { list } | ...
		 */
#ఒకవేళ BB_MMU
		పూర్ణాంకం rcode;
		debug_printf_exec("pseudo_exec: run_list\n");
		reset_traps_గమ్యం_defaults();
		rcode = నిరంతరం_list(command->సమూహం);
		/* OK to leak memory by not calling free_pipe_list,
		 * since this process is about to exit */
		_exit(rcode);
#లేకపోతే
		re_execute_shell(&nommu_save->argv_from_re_execing,
				command->సమూహం_as_string,
				G.global_argv[0],
				G.global_argv + 1,
				ఖాళీ);
#అయితే_ముగింపు
	}

	/* Case when we are here: ... | >file */
	debug_printf_exec("pseudo_exec'ed null command\n");
	_exit_SUCCESS();
}

#ఒకవేళ ENABLE_HUSH_పని
స్థిర స్థిరరాశి అక్షరం *get_cmdtext(నిర్మాణం pipe *pi)
{
	అక్షరం **argv;
	అక్షరం *p;
	పూర్ణాంకం len;

	/* This is subtle. ->cmdtext is created only on first backgrounding.
	 * (Think "cat, <ctrl-z>, fg, <ctrl-z>, fg, <ctrl-z>...." here...)
	 * On subsequent bg argv is trashed, but we won't use it */
	ఒకవేళ (pi->cmdtext)
		తిరిగి pi->cmdtext;

	argv = pi->cmds[0].argv;
	ఒకవేళ (!argv) {
		pi->cmdtext = xzalloc(1);
		తిరిగి pi->cmdtext;
	}
	len = 0;
	చేయి {
		len += అక్షరశ్రేణి_పొడవు(*argv) + 1;
	} అయితే (*++argv);
	p = xmalloc(len);
	pi->cmdtext = p;
	argv = pi->cmds[0].argv;
	చేయి {
		p = stpcpy(p, *argv);
		*p++ = ' ';
	} అయితే (*++argv);
	p[-1] = '\0';
	తిరిగి pi->cmdtext;
}

స్థిర శూన్య remove_పని_from_పట్టిక(నిర్మాణం pipe *pi)
{
	నిర్మాణం pipe *prev_pipe;

	ఒకవేళ (pi == G.పని_list) {
		G.పని_list = pi->తదుపరి;
	} లేకపోతే {
		prev_pipe = G.పని_list;
		అయితే (prev_pipe->తదుపరి != pi)
			prev_pipe = prev_pipe->తదుపరి;
		prev_pipe->తదుపరి = pi->తదుపరి;
	}
	G.చివరి_jobid = 0;
	ఒకవేళ (G.పని_list)
		G.చివరి_jobid = G.పని_list->jobid;
}

స్థిర శూన్య delete_ముగిసిన_పని(నిర్మాణం pipe *pi)
{
	remove_పని_from_పట్టిక(pi);
	free_pipe(pi);
}

స్థిర శూన్య clean_పైకి_చివరి_dead_పని(శూన్య)
{
	ఒకవేళ (G.పని_list && !G.పని_list->alive_cmds)
		delete_ముగిసిన_పని(G.పని_list);
}

స్థిర శూన్య చొప్పించు_పని_into_పట్టిక(నిర్మాణం pipe *pi)
{
	నిర్మాణం pipe *పని, **jobp;
	పూర్ణాంకం i;

	clean_పైకి_చివరి_dead_పని();

	/* Find the end of the list, and find next job ID to use */
	i = 0;
	jobp = &G.పని_list;
	అయితే ((పని = *jobp) != ఖాళీ) {
		ఒకవేళ (పని->jobid > i)
			i = పని->jobid;
		jobp = &పని->తదుపరి;
	}
	pi->jobid = i + 1;

	/* Create a new job struct at the end */
	పని = *jobp = xmemdup(pi, పరిమాణం(*pi));
	పని->తదుపరి = ఖాళీ;
	పని->cmds = xzalloc(పరిమాణం(pi->cmds[0]) * pi->అంకె_cmds);
	/* Cannot copy entire pi->cmds[] vector! This causes double frees */
	కొరకు (i = 0; i < pi->అంకె_cmds; i++) {
		పని->cmds[i].pid = pi->cmds[i].pid;
		/* all other fields are not used and stay zero */
	}
	పని->cmdtext = xstrdup(get_cmdtext(pi));

	ఒకవేళ (G_interactive_fd)
		రాయి("[%u] %u %s\n", పని->jobid, (గుర్తులేని)పని->cmds[0].pid, పని->cmdtext);
	G.చివరి_jobid = పని->jobid;
}
#అయితే_ముగింపు /*పని*/

స్థిర పూర్ణాంకం పని_exited_or_stopped(నిర్మాణం pipe *pi)
{
	పూర్ణాంకం rcode, i;

	ఒకవేళ (pi->alive_cmds != pi->stopped_cmds)
		తిరిగి -1;

	/* All processes in fg pipe have exited or stopped */
	rcode = 0;
	i = pi->అంకె_cmds;
	అయితే (--i >= 0) {
		rcode = pi->cmds[i].cmd_exitcode;
		/* usually last process gives overall exitstatus,
		 * but with "set -o pipefail", last *failed* process does */
		ఒకవేళ (G.o_opt[OPT_O_PIPEFAIL] == 0 || rcode != 0)
			ఆపు;
	}
	IF_HAS_KEYWORDS(ఒకవేళ (pi->pi_inverted) rcode = !rcode;)
	తిరిగి rcode;
}

స్థిర పూర్ణాంకం process_wait_ఫలితం(నిర్మాణం pipe *fg_pipe, pid_t childpid, పూర్ణాంకం స్థితి_2)
{
#ఒకవేళ ENABLE_HUSH_పని
	నిర్మాణం pipe *pi;
#అయితే_ముగింపు
	పూర్ణాంకం i, dead;

	dead = WIFEXITED(స్థితి_2) || WIFSIGNALED(స్థితి_2);

#ఒకవేళ DEBUG_JOBS
	ఒకవేళ (WIFSTOPPED(స్థితి_2))
		debug_printf_jobs("pid %d stopped by sig %d (exitcode %d)\n",
				childpid, WSTOPSIG(స్థితి_2), WEXITSTATUS(స్థితి_2));
	ఒకవేళ (WIFSIGNALED(స్థితి_2))
		debug_printf_jobs("pid %d killed by sig %d (exitcode %d)\n",
				childpid, WTERMSIG(స్థితి_2), WEXITSTATUS(స్థితి_2));
	ఒకవేళ (WIFEXITED(స్థితి_2))
		debug_printf_jobs("pid %d exited, exitcode %d\n",
				childpid, WEXITSTATUS(స్థితి_2));
#అయితే_ముగింపు
	/* Were we asked to wait for a fg pipe? */
	ఒకవేళ (fg_pipe) {
		i = fg_pipe->అంకె_cmds;

		అయితే (--i >= 0) {
			పూర్ణాంకం rcode;

			debug_printf_jobs("check pid %d\n", fg_pipe->cmds[i].pid);
			ఒకవేళ (fg_pipe->cmds[i].pid != childpid)
				కొనసాగు;
			ఒకవేళ (dead) {
				పూర్ణాంకం ex;
				fg_pipe->cmds[i].pid = 0;
				fg_pipe->alive_cmds--;
				ex = WEXITSTATUS(స్థితి_2);
				/* bash prints killer signal's name for *last*
				 * process in pipe (prints just newline for SIGINT/SIGPIPE).
				 * Mimic this. Example: "sleep 5" + (^\ or kill -QUIT)
				 */
				ఒకవేళ (WIFSIGNALED(స్థితి_2)) {
					పూర్ణాంకం sig = WTERMSIG(స్థితి_2);
#ఒకవేళ ENABLE_HUSH_పని
					ఒకవేళ (G.నిరంతరం_list_level == 1
					/* ^^^^^ Do not print in nested contexts, example:
					 * echo `sleep 1; sh -c 'kill -9 $$'` - prints "137", NOT "Killed 137"
					 */
					 && i == fg_pipe->అంకె_cmds-1
					) {
						/* strsignal() is for bash compat. ~600 bloat versus bbox's get_signame() */
						puts(sig == SIGINT || sig == SIGPIPE ? "" : strsignal(sig));
					}
#అయితే_ముగింపు
					/* TODO: if (WCOREDUMP(status)) + " (core dumped)"; */
					/* MIPS has 128 sigs (1..128), if sig==128,
					 * 128 + sig would result in exitcode 256 -> 0!
					 */
					ex = 128 | sig;
				}
				fg_pipe->cmds[i].cmd_exitcode = ex;
			} లేకపోతే {
				fg_pipe->stopped_cmds++;
			}
			debug_printf_jobs("fg_pipe: alive_cmds %d stopped_cmds %d\n",
					fg_pipe->alive_cmds, fg_pipe->stopped_cmds);
			rcode = పని_exited_or_stopped(fg_pipe);
			ఒకవేళ (rcode >= 0) {
/* Note: *non-interactive* bash does not continue if all processes in fg pipe
 * are stopped. Testcase: "cat | cat" in a script (not on command line!)
 * and "killall -STOP cat" */
				ఒకవేళ (G_interactive_fd) {
#ఒకవేళ ENABLE_HUSH_పని
					ఒకవేళ (fg_pipe->alive_cmds != 0)
						చొప్పించు_పని_into_పట్టిక(fg_pipe);
#అయితే_ముగింపు
					తిరిగి rcode;
				}
				ఒకవేళ (fg_pipe->alive_cmds == 0)
					తిరిగి rcode;
			}
			/* There are still running processes in the fg_pipe */
			తిరిగి -1;
		}
		/* It wasn't in fg_pipe, look for process in bg pipes */
	}

#ఒకవేళ ENABLE_HUSH_పని
	/* We were asked to wait for bg or orphaned children */
	/* No need to remember exitcode in this case */
	కొరకు (pi = G.పని_list; pi; pi = pi->తదుపరి) {
		కొరకు (i = 0; i < pi->అంకె_cmds; i++) {
			ఒకవేళ (pi->cmds[i].pid == childpid)
				వెళ్ళు దొరికింది_pi_and_prognum;
		}
	}
	/* Happens when shell is used as init process (init=/bin/sh) */
	debug_printf("checkjobs: pid %d was not in our list!\n", childpid);
	తిరిగి -1; /* this wasn't a process from fg_pipe */

 దొరికింది_pi_and_prognum:
	ఒకవేళ (dead) {
		/* child exited */
		పూర్ణాంకం rcode = WEXITSTATUS(స్థితి_2);
		ఒకవేళ (WIFSIGNALED(స్థితి_2))
			/* NB: not 128 + sig, MIPS has sig 128 */
			rcode = 128 | WTERMSIG(స్థితి_2);
		pi->cmds[i].cmd_exitcode = rcode;
		ఒకవేళ (G.చివరి_bg_pid == pi->cmds[i].pid)
			G.చివరి_bg_pid_exitcode = rcode;
		pi->cmds[i].pid = 0;
		pi->alive_cmds--;
		ఒకవేళ (!pi->alive_cmds) {
# ఒకవేళ ENABLE_HUSH_BASH_COMPAT
			G.dead_పని_exitcode = పని_exited_or_stopped(pi);
# endif
			ఒకవేళ (G_interactive_fd) {
				రాయి(పని_స్థితి_FORMAT, pi->jobid,
						"Done", pi->cmdtext);
				delete_ముగిసిన_పని(pi);
			} లేకపోతే {
/*
 * bash deletes finished jobs from job table only in interactive mode,
 * after "jobs" cmd, or if pid of a new process matches one of the old ones
 * (see cleanup_dead_jobs(), delete_old_job(), J_NOTIFIED in bash source).
 * Testcase script: "(exit 3) & sleep 1; wait %1; echo $?" prints 3 in bash.
 * We only retain one "dead" job, if it's the single job on the list.
 * This covers most of real-world scenarios where this is useful.
 */
				ఒకవేళ (pi != G.పని_list)
					delete_ముగిసిన_పని(pi);
			}
		}
	} లేకపోతే {
		/* child stopped */
		pi->stopped_cmds++;
	}
#అయితే_ముగింపు
	తిరిగి -1; /* this wasn't a process from fg_pipe */
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
స్థిర పూర్ణాంకం checkjobs(నిర్మాణం pipe *fg_pipe, pid_t waitfor_pid)
{
	పూర్ణాంకం లక్షణాలు;
	పూర్ణాంకం స్థితి_2;
	పూర్ణాంకం rcode = 0;

	debug_printf_jobs("checkjobs %p\n", fg_pipe);

	లక్షణాలు = WUNTRACED;
	ఒకవేళ (fg_pipe == ఖాళీ)
		లక్షణాలు |= WNOHANG;

	errno = 0;
#ఒకవేళ ENABLE_HUSH_FAST
	ఒకవేళ (G.handled_SIGCHLD == G.లెక్క_SIGCHLD) {
//bb_error_msg("[%d] checkjobs: G.count_SIGCHLD:%d G.handled_SIGCHLD:%d children?:%d fg_pipe:%p",
//getpid(), G.count_SIGCHLD, G.handled_SIGCHLD, G.we_have_children, fg_pipe);
		/* There was neither fork nor SIGCHLD since last waitpid */
		/* Avoid doing waitpid syscall if possible */
		ఒకవేళ (!G.we_have_children) {
			errno = ECHILD;
			తిరిగి -1;
		}
		ఒకవేళ (fg_pipe == ఖాళీ) { /* is WNOHANG set? */
			/* We have children, but they did not exit
			 * or stop yet (we saw no SIGCHLD) */
			తిరిగి 0;
		}
		/* else: !WNOHANG, waitpid will block, can't short-circuit */
	}
#అయితే_ముగింపు

/* Do we do this right?
 * bash-3.00# sleep 20 | false
 * <ctrl-Z pressed>
 * [3]+  Stopped          sleep 20 | false
 * bash-3.00# echo $?
 * 1   <========== bg pipe is not fully done, but exitcode is already known!
 * [hush 1.14.0: yes we do it right]
 */
	అయితే (1) {
		pid_t childpid;
#ఒకవేళ ENABLE_HUSH_FAST
		పూర్ణాంకం i;
		i = G.లెక్క_SIGCHLD;
#అయితే_ముగింపు
		childpid = waitpid(-1, &స్థితి_2, లక్షణాలు);
		ఒకవేళ (childpid <= 0) {
			ఒకవేళ (childpid && errno != ECHILD)
				bb_simple_perror_సందేశం("waitpid");
#ఒకవేళ ENABLE_HUSH_FAST
			లేకపోతే { /* Until next SIGCHLD, waitpid's are useless */
				G.we_have_children = (childpid == 0);
				G.handled_SIGCHLD = i;
//bb_error_msg("[%d] checkjobs: waitpid returned <= 0, G.count_SIGCHLD:%d G.handled_SIGCHLD:%d", getpid(), G.count_SIGCHLD, G.handled_SIGCHLD);
			}
#అయితే_ముగింపు
			/* ECHILD (no children), or 0 (no change in children status) */
			rcode = childpid;
			ఆపు;
		}
		rcode = process_wait_ఫలితం(fg_pipe, childpid, స్థితి_2);
		ఒకవేళ (rcode >= 0) {
			/* fg_pipe exited or stopped */
			ఆపు;
		}
		ఒకవేళ (childpid == waitfor_pid) { /* "wait PID" */
			debug_printf_exec("childpid==waitfor_pid:%d status:0x%08x\n", childpid, స్థితి_2);
			rcode = WEXITSTATUS(స్థితి_2);
			ఒకవేళ (WIFSIGNALED(స్థితి_2))
				rcode = 128 | WTERMSIG(స్థితి_2);
			ఒకవేళ (WIFSTOPPED(స్థితి_2))
				/* bash: "cmd & wait $!" and cmd stops: $? = 128 | stopsig */
				rcode = 128 | WSTOPSIG(స్థితి_2);
			rcode++;
			ఆపు; /* "wait PID" called us, give it exitcode+1 */
		}
#ఒకవేళ ENABLE_HUSH_BASH_COMPAT
		ఒకవేళ (-1 == waitfor_pid /* "wait -n" (wait for any one job) */
		 && G.dead_పని_exitcode >= 0 /* some job did finish */
		) {
			debug_printf_exec("waitfor_pid:-1\n");
			rcode = G.dead_పని_exitcode + 1;
			ఆపు;
		}
#అయితే_ముగింపు
		/* This wasn't one of our processes, or */
		/* fg_pipe still has running processes, do waitpid again */
	} /* while (waitpid succeeds)... */

	తిరిగి rcode;
}

#ఒకవేళ ENABLE_HUSH_పని
స్థిర పూర్ణాంకం checkjobs_and_fg_shell(నిర్మాణం pipe *fg_pipe)
{
	pid_t p;
	పూర్ణాంకం rcode = checkjobs(fg_pipe, 0 /*(no pid to wait for)*/);
	ఒకవేళ (G_saved_tty_pgrp) {
		/* Job finished, move the shell to the foreground */
		p = getpgrp(); /* our process group id */
		debug_printf_jobs("fg'ing ourself: getpgrp()=%d\n", (పూర్ణాంకం)p);
		tcsetpgrp(G_interactive_fd, p);
	}
	తిరిగి rcode;
}
#అయితే_ముగింపు
#అయితే_ముగింపు /* !__U_BOOT__ */

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
స్థిర శూన్య సమితి_G_ifs(శూన్య)
{
	/* Testcase: set -- q w e; (IFS='' echo "$*"; IFS=''; echo "$*"); echo "$*"
	 * Result should be 3 lines: q w e, qwe, q w e
	 */
	ఒకవేళ (G.ifs_whitespace != G.ifs)
		విడుదల(G.ifs_whitespace);
	G.ifs = get_local_var_విలువ("IFS");
	ఒకవేళ (G.ifs) {
		అక్షరం *p;
		G.ifs_whitespace = (అక్షరం*)G.ifs;
		p = skip_whitespace(G.ifs);
		ఒకవేళ (*p) {
			/* Not all $IFS is whitespace */
			అక్షరం *d;
			పూర్ణాంకం len = p - G.ifs;
			p = skip_non_whitespace(p);
			G.ifs_whitespace = xmalloc(len + అక్షరశ్రేణి_పొడవు(p) + 1); /* can overestimate */
			d = mempcpy(G.ifs_whitespace, G.ifs, len);
			అయితే (*p) {
				ఒకవేళ (ఖాళీనా(*p))
					*d++ = *p;
				p++;
			}
			*d = '\0';
		}
	} లేకపోతే {
		G.ifs = defifs;
		G.ifs_whitespace = (అక్షరం*)G.ifs;
	}
}
#నిర్వచితం_కాకపోతే __U_BOOT__
#ఒకవేళ !ENABLE_HUSH_బహుళకం_X
#నిర్వచించు redirect_and_varexp_helper(command, sqp, argv_expanded) \
	redirect_and_varexp_helper(command, sqp)
#అయితే_ముగింపు
స్థిర పూర్ణాంకం redirect_and_varexp_helper(
		నిర్మాణం command *command,
		నిర్మాణం squirrel **sqp,
		అక్షరం **argv_expanded)
{
	/* Assignments occur before redirects. Try:
	 * a=`sleep 1` sleep 2 3>/qwe/rty
	 */

	అక్షరం **new_env = expand_assignments(command->argv, command->assignment_cnt);
	dump_cmd_లోపల_x_బహుళకం(new_env);
	dump_cmd_లోపల_x_బహుళకం(argv_expanded);
	/* this takes ownership of new_env[i] elements, and frees new_env: */
	సమితి_vars_and_save_old(new_env);

	తిరిగి అమరిక_redirects(command, sqp);
}
#అయితే_ముగింపు /* !__U_BOOT__ */

స్థిర NOINLINE పూర్ణాంకం నిరంతరం_pipe(నిర్మాణం pipe *pi)
{
	స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి null_ptr = ఖాళీ;

	పూర్ణాంకం cmd_no;
#నిర్వచితం_కాకపోతే __U_BOOT__
	పూర్ణాంకం తదుపరి_infd;
#అయితే_ముగింపు /* !__U_BOOT__ */
	నిర్మాణం command *command;
	అక్షరం **argv_expanded;
	అక్షరం **argv;
#నిర్వచితం_కాకపోతే __U_BOOT__
	నిర్మాణం squirrel *squirrel = ఖాళీ;
#అయితే_ముగింపు /* !__U_BOOT__ */
	పూర్ణాంకం rcode;

#నిర్వచితమైతే __U_BOOT__
	/*
	 * Set rcode here to avoid returning a garbage value in the middle of
	 * the function.
	 * Also, if an error occurs, rcode value would be changed and last
	 * return will signal the error.
	 */
	rcode = 0;
#అయితే_ముగింపు /* __U_BOOT__ */

	debug_printf_exec("run_pipe start: members:%d\n", pi->అంకె_cmds);
	debug_enter();

	సమితి_G_ifs();

#నిర్వచితం_కాకపోతే __U_BOOT__
	IF_HUSH_పని(pi->pgrp = -1;)
	pi->stopped_cmds = 0;
#అయితే_ముగింపు /* !__U_BOOT__ */
	command = &pi->cmds[0];
	argv_expanded = ఖాళీ;

#నిర్వచితం_కాకపోతే __U_BOOT__
	ఒకవేళ (pi->అంకె_cmds != 1
	 || pi->followup == PIPE_BG
	 || command->cmd_type == CMD_SUBSHELL
	) {
		వెళ్ళు must_fork;
	}

	pi->alive_cmds = 1;
#అయితే_ముగింపు /* !__U_BOOT__ */

	debug_printf_exec(": group:%p argv:'%s'\n",
		command->సమూహం, command->argv ? command->argv[0] : "NONE");

	ఒకవేళ (command->సమూహం) {
#ఒకవేళ ENABLE_HUSH_FUNCTIONS
		ఒకవేళ (command->cmd_type == CMD_FUNCDEF) {
			/* "executing" func () { list } */
			నిర్మాణం function *funcp;

			funcp = new_function(command->argv[0]);
			/* funcp->name is already set to argv[0] */
			funcp->body = command->సమూహం;
# ఒకవేళ !BB_MMU
			funcp->body_as_string = command->సమూహం_as_string;
			command->సమూహం_as_string = ఖాళీ;
# endif
			command->సమూహం = ఖాళీ;
			command->argv[0] = ఖాళీ;
			debug_printf_exec("cmd %p has child func at %p\n", command, funcp);
			funcp->మాతృ_cmd = command;
			command->బిడ్డ_func = funcp;

			debug_printf_exec("run_pipe: return EXIT_SUCCESS\n");
			debug_leave();
			తిరిగి EXIT_SUCCESS;
		}
#అయితే_ముగింపు
		/* { list } */
		debug_printf_exec("non-subshell group\n");
		rcode = 1; /* exitcode if redir failed */
#నిర్వచితం_కాకపోతే __U_BOOT__
		ఒకవేళ (అమరిక_redirects(command, &squirrel) == 0) {
#అయితే_ముగింపు /* !__U_BOOT__ */
			debug_printf_exec(": run_list\n");
//FIXME: we need to pass squirrel down into run_list()
//for SH_STANDALONE case, or else this construct:
// { find /proc/self/fd; true; } >FILE; cmd2
//has no way of closing saved fd#1 for "find",
//and in SH_STANDALONE mode, "find" is not execed,
//therefore CLOEXEC on saved fd does not help.
			rcode = నిరంతరం_list(command->సమూహం) & 0xff;
#నిర్వచితం_కాకపోతే __U_BOOT__
		}
		restore_redirects(squirrel);
#అయితే_ముగింపు /* !__U_BOOT__ */
		IF_HAS_KEYWORDS(ఒకవేళ (pi->pi_inverted) rcode = !rcode;)
		debug_leave();
		debug_printf_exec("run_pipe: return %d\n", rcode);
		తిరిగి rcode;
	}

	argv = command->argv ? command->argv : (అక్షరం **) &null_ptr;
	{
#నిర్వచితం_కాకపోతే __U_BOOT__
		స్థిరరాశి నిర్మాణం built_లోపల_command *x;
		IF_HUSH_FUNCTIONS(స్థిరరాశి నిర్మాణం function *funcp;)
		IF_NOT_HUSH_FUNCTIONS(గణన { funcp = 0 };)
		నిర్మాణం variable **sv_shadowed;
#అయితే_ముగింపు /* !__U_BOOT__ */
		నిర్మాణం variable *old_vars;

#ఒకవేళ ENABLE_HUSH_LINENO_VAR
		G.execute_lineno = command->lineno;
#అయితే_ముగింపు

		ఒకవేళ (argv[command->assignment_cnt] == ఖాళీ) {
			/* Assignments, but no command.
			 * Ensure redirects take effect (that is, create files).
			 * Try "a=t >file"
			 */
			గుర్తులేని i;
			G.expand_exitcode = 0;
 only_assignments:
#నిర్వచితం_కాకపోతే __U_BOOT__
			rcode = అమరిక_redirects(command, &squirrel);
			restore_redirects(squirrel);
#అయితే_ముగింపు /* !__U_BOOT__ */

			/* Set shell variables */
			i = 0;
			అయితే (i < command->assignment_cnt) {
				అక్షరం *p = expand_string_గమ్యం_string(argv[i],
						EXP_FLAG_ESC_GLOB_CHARS,
						/*unbackslash:*/ 1
				);
#ఒకవేళ ENABLE_HUSH_బహుళకం_X
				ఒకవేళ (G_x_బహుళకం) {
					అక్షరం *eq;
					ఒకవేళ (i == 0)
						x_బహుళకం_ఉపసర్గ();
					x_బహుళకం_addchr(' ');
					eq = strchrnul(p, '=');
					ఒకవేళ (*eq) eq++;
					x_బహుళకం_addblock(p, (eq - p));
					x_బహుళకం_print_optionally_squoted(eq);
					x_బహుళకం_flush();
				}
#అయితే_ముగింపు
				debug_printf_env("set shell var:'%s'->'%s'\n", *argv, p);
#నిర్వచితం_కాకపోతే __U_BOOT__
				ఒకవేళ (సమితి_local_var0(p)) {
#లేకపోతే /* __U_BOOT__ */
				ఒకవేళ (సమితి_local_var_modern(p, /*flag:*/ 0)) {
#అయితే_ముగింపు
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
			ఒకవేళ (rcode == 0)
				rcode = G.expand_exitcode;
			IF_HAS_KEYWORDS(ఒకవేళ (pi->pi_inverted) rcode = !rcode;)
			debug_leave();
			debug_printf_exec("run_pipe: return %d\n", rcode);
			తిరిగి rcode;
		}

		/* Expand the rest into (possibly) many strings each */
#ఒకవేళ defined(CMD_TEST2_SINGLEWORD_NOGLOB)
		ఒకవేళ (command->cmd_type == CMD_TEST2_SINGLEWORD_NOGLOB)
			argv_expanded = expand_strvec_గమ్యం_strvec_singleword_noglob(argv + command->assignment_cnt);
		లేకపోతే
#అయితే_ముగింపు
#ఒకవేళ defined(CMD_SINGLEWORD_NOGLOB)
		ఒకవేళ (command->cmd_type == CMD_SINGLEWORD_NOGLOB)
			argv_expanded = expand_strvec_గమ్యం_strvec_singleword_noglob(argv + command->assignment_cnt);
		లేకపోతే
#అయితే_ముగింపు
			argv_expanded = expand_strvec_గమ్యం_strvec(argv + command->assignment_cnt);

		/* If someone gives us an empty string: `cmd with empty output` */
		ఒకవేళ (!argv_expanded[0]) {
			విడుదల(argv_expanded);
			/* `false` still has to set exitcode 1 */
			G.expand_exitcode = G.చివరి_exitcode;
			వెళ్ళు only_assignments;
		}

		old_vars = ఖాళీ;
#నిర్వచితం_కాకపోతే __U_BOOT__
		sv_shadowed = G.shadowed_vars_pp;

		/* Check if argv[0] matches any functions (this goes before bltins) */
		IF_HUSH_FUNCTIONS(funcp = కనుగొను_function(argv_expanded[0]);)
		IF_HUSH_FUNCTIONS(x = ఖాళీ;)
		IF_HUSH_FUNCTIONS(ఒకవేళ (!funcp))
			x = కనుగొను_builtin(argv_expanded[0]);
		ఒకవేళ (x || funcp) {
			ఒకవేళ (x && x->b_function == builtin_exec && argv_expanded[1] == ఖాళీ) {
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

				వెళ్ళు clean_పైకి_and_ret1;
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
			ఒకవేళ (rcode == 0) {
				ఒకవేళ (!funcp) {
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
#ఒకవేళ ENABLE_HUSH_FUNCTIONS
				లేకపోతే {
					debug_printf_exec(": function '%s' '%s'...\n",
						funcp->name, argv_expanded[1]);
					rcode = నిరంతరం_function(funcp, argv_expanded) & 0xff;
					/*
					 * But do collect *to old_vars list* vars shadowed
					 * within function execution. To that end, restore
					 * this pointer _after_ function run:
					 */
					G.shadowed_vars_pp = sv_shadowed;
				}
#అయితే_ముగింపు
			}
		} లేకపోతే
		ఒకవేళ (ENABLE_FEATURE_SH_NOFORK && అంకె_APPLETS > 1) {
			పూర్ణాంకం n = కనుగొను_applet_by_name(argv_expanded[0]);
			ఒకవేళ (n < 0 || !APPLET_IS_NOFORK(n))
				వెళ్ళు must_fork;

			enter_var_nest_level();
			/* Collect all variables "shadowed" by helper into old_vars list */
			G.shadowed_vars_pp = &old_vars;
			rcode = redirect_and_varexp_helper(command, &squirrel, argv_expanded);
			G.shadowed_vars_pp = sv_shadowed;

			ఒకవేళ (rcode == 0) {
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
				rcode = నిరంతరం_nofork_applet(n, argv_expanded);
			}
		} లేకపోతే
			వెళ్ళు must_fork;

		restore_redirects(squirrel);
 clean_పైకి_and_ret1:
		leave_var_nest_level();
		కూడు_vars(old_vars);

		/*
		 * Try "usleep 99999999" + ^C + "echo $?"
		 * with FEATURE_SH_NOFORK=y.
		 */
		ఒకవేళ (!funcp) {
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
			ఒకవేళ (sigismember(&G.pending_సమితి, SIGINT))
				rcode = 128 | SIGINT;
		}
		విడుదల(argv_expanded);
		IF_HAS_KEYWORDS(ఒకవేళ (pi->pi_inverted) rcode = !rcode;)
		debug_leave();
		debug_printf_exec("run_pipe return %d\n", rcode);
		తిరిగి rcode;
#అయితే_ముగింపు /* !__U_BOOT__ */
	}

#నిర్వచితం_కాకపోతే __U_BOOT__
 must_fork:
	/* NB: argv_expanded may already be created, and that
	 * might include `cmd` runs! Do not rerun it! We *must*
	 * use argv_expanded if it's non-NULL */

	/* Going to fork a child per each pipe member */
	pi->alive_cmds = 0;
	తదుపరి_infd = 0;
#అయితే_ముగింపు /* !__U_BOOT__ */

	cmd_no = 0;
	అయితే (cmd_no < pi->అంకె_cmds) {
#నిర్వచితం_కాకపోతే __U_BOOT__
		నిర్మాణం fd_pair pipefds;
#ఒకవేళ !BB_MMU
		పూర్ణాంకం sv_var_nest_level = G.var_nest_level;
		అస్థిర nommu_save_t nommu_save;
		nommu_save.old_vars = ఖాళీ;
		nommu_save.argv = ఖాళీ;
		nommu_save.argv_from_re_execing = ఖాళీ;
#అయితే_ముగింపు
#అయితే_ముగింపు /* !__U_BOOT__ */
		command = &pi->cmds[cmd_no];
		cmd_no++;

#నిర్వచితమైతే __U_BOOT__
		/* Replace argv and argc by expanded if it exists. */
		ఒకవేళ (argv_expanded) {
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
			command->argc = list_కొలత(argv_expanded);
			command->argv = argv_expanded;
		}
#అయితే_ముగింపు /* __U_BOOT__ */
			ఒకవేళ (command->argv) {
			debug_printf_exec(": pipe member '%s' '%s'...\n",
					command->argv[0], command->argv[1]);
		} లేకపోతే {
			debug_printf_exec(": pipe member with no argv\n");
		}

#నిర్వచితం_కాకపోతే __U_BOOT__
		/* pipes are inserted between pairs of commands */
		pipefds.rd = 0;
		pipefds.wr = 1;
		ఒకవేళ (cmd_no < pi->అంకె_cmds)
			xpiped_pair(pipefds);

#ఒకవేళ ENABLE_HUSH_LINENO_VAR
		G.execute_lineno = command->lineno;
#అయితే_ముగింపు

		command->pid = BB_MMU ? fork() : vfork();
		ఒకవేళ (!command->pid) { /*బిడ్డ*/
#ఒకవేళ ENABLE_HUSH_పని
			disable_restore_tty_pgrp_on_exit();
			CLEAR_RANDOM_T(&G.random_gen); /* or else $RANDOM repeats in child */

			/* Every child adds itself to new process group
			 * with pgid == pid_of_first_child_in_pipe */
			ఒకవేళ (G.నిరంతరం_list_level == 1 && G_interactive_fd) {
				pid_t pgrp;
				pgrp = pi->pgrp;
				ఒకవేళ (pgrp < 0) /* true for 1st process only */
					pgrp = getpid();
				ఒకవేళ (setpgid(0, pgrp) == 0
				 && pi->followup != PIPE_BG
				 && G_saved_tty_pgrp /* we have ctty */
				) {
					/* We do it in *every* child, not just first,
					 * to avoid races */
					tcsetpgrp(G_interactive_fd, pgrp);
				}
			}
#అయితే_ముగింపు
			ఒకవేళ (pi->alive_cmds == 0 && pi->followup == PIPE_BG) {
				/* 1st cmd in backgrounded pipe
				 * should have its stdin /dev/null'ed */
				close(0);
				ఒకవేళ (open(bb_dev_null, O_RDONLY))
					xopen("/", O_RDONLY);
			} లేకపోతే {
				xmove_fd(తదుపరి_infd, 0);
			}
			xmove_fd(pipefds.wr, 1);
			ఒకవేళ (pipefds.rd > 1)
				close(pipefds.rd);
			/* Like bash, explicit redirects override pipes,
			 * and the pipe fd (fd#1) is available for dup'ing:
			 * "cmd1 2>&1 | cmd2": fd#1 is duped to fd#2, thus stderr
			 * of cmd1 goes into pipe.
			 */
			ఒకవేళ (అమరిక_redirects(command, ఖాళీ)) {
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
#ఒకవేళ ENABLE_HUSH_FAST
		G.లెక్క_SIGCHLD++;
//bb_error_msg("[%d] fork in run_pipe: G.count_SIGCHLD:%d G.handled_SIGCHLD:%d", getpid(), G.count_SIGCHLD, G.handled_SIGCHLD);
#అయితే_ముగింపు
		enable_restore_tty_pgrp_on_exit();
#ఒకవేళ !BB_MMU
		/* Clean up after vforked child */
		విడుదల(nommu_save.argv);
		విడుదల(nommu_save.argv_from_re_execing);
		G.var_nest_level = sv_var_nest_level;
		remove_nested_vars();
		కూడు_vars(nommu_save.old_vars);
#అయితే_ముగింపు
		విడుదల(argv_expanded);
		argv_expanded = ఖాళీ;
		ఒకవేళ (command->pid < 0) { /* [v]fork failed */
			/* Clearly indicate, was it fork or vfork */
			bb_simple_perror_సందేశం(BB_MMU ? "vfork"+1 : "vfork");
		} లేకపోతే {
			pi->alive_cmds++;
#ఒకవేళ ENABLE_HUSH_పని
			/* Second and next children need to know pid of first one */
			ఒకవేళ (pi->pgrp < 0)
				pi->pgrp = command->pid;
#అయితే_ముగింపు
		}

		ఒకవేళ (cmd_no > 1)
			close(తదుపరి_infd);
		ఒకవేళ (cmd_no < pi->అంకె_cmds)
			close(pipefds.wr);
		/* Pass read (output) pipe end to next iteration */
		తదుపరి_infd = pipefds.rd;
#లేకపోతే /* __U_BOOT__ */
		/* Process the command */
		rcode = cmd_process(G.do_repeat ? CMD_FLAG_REPEAT : 0,
				    command->argc, command->argv,
				    &(G.flag_repeat), ఖాళీ);

		ఒకవేళ (argv_expanded) {
			/*
			 * expand_strvec_to_strvec() allocates memory to expand
			 * argv, we need to free it.
			 */
			విడుదల(argv_expanded);

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
#అయితే_ముగింపు /* __U_BOOT__ */
	}

#నిర్వచితం_కాకపోతే __U_BOOT__
	ఒకవేళ (!pi->alive_cmds) {
		debug_leave();
		debug_printf_exec("run_pipe return 1 (all forks failed, no children)\n");
		తిరిగి 1;
	}
#అయితే_ముగింపు /* __U_BOOT__ */

	debug_leave();
#నిర్వచితం_కాకపోతే __U_BOOT__
	debug_printf_exec("run_pipe return -1 (%u children started)\n", pi->alive_cmds);
	తిరిగి -1;
#లేకపోతే /* __U_BOOT__ */
	IF_HAS_KEYWORDS(ఒకవేళ (pi->pi_inverted) rcode = !rcode;)
	debug_printf_exec("run_pipe return %d\n", rcode);
	తిరిగి rcode;
#అయితే_ముగింపు /* __U_BOOT__ */
}

/* NB: called by pseudo_exec, and therefore must not modify any
 * global data until exec/_exit (we can be a child after vfork!) */
స్థిర పూర్ణాంకం నిరంతరం_list(నిర్మాణం pipe *pi)
{
#ఒకవేళ ENABLE_HUSH_CASE
	అక్షరం *case_మాట = ఖాళీ;
#అయితే_ముగింపు
#ఒకవేళ ENABLE_HUSH_LOOPS
	నిర్మాణం pipe *loop_శిఖరం = ఖాళీ;
	అక్షరం **for_lcur = ఖాళీ;
	అక్షరం **for_list = ఖాళీ;
#అయితే_ముగింపు
	smallint చివరి_followup;
	smalluint rcode;
#ఒకవేళ ENABLE_HUSH_IF || ENABLE_HUSH_CASE
	smalluint cond_సంకేతం = 0;
#లేకపోతే
	గణన { cond_సంకేతం = 0 };
#అయితే_ముగింపు
#ఒకవేళ HAS_KEYWORDS
	smallint rword;      /* RES_foo */
	smallint చివరి_rword; /* ditto */
#అయితే_ముగింపు

#నిర్వచితం_కాకపోతే __U_BOOT__
	debug_printf_exec("run_list lvl %d start\n", G.నిరంతరం_list_level);
	debug_enter();
#అయితే_ముగింపు /* !__U_BOOT__ */

	సమితి_G_ifs();

#ఒకవేళ ENABLE_HUSH_LOOPS
	/* Check syntax for "for" */
	{
		నిర్మాణం pipe *cpipe;
		కొరకు (cpipe = pi; cpipe; cpipe = cpipe->తదుపరి) {
			ఒకవేళ (cpipe->res_మాట != RES_FOR && cpipe->res_మాట != RES_లోపల)
				కొనసాగు;
			/* current word is FOR or IN (BOLD in comments below) */
			ఒకవేళ (cpipe->తదుపరి == ఖాళీ) {
				syntax_error("malformed for");
				debug_leave();
				debug_printf_exec("run_list lvl %d return 1\n", G.నిరంతరం_list_level);
				తిరిగి 1;
			}
			/* "FOR v; do ..." and "for v IN a b; do..." are ok */
			ఒకవేళ (cpipe->తదుపరి->res_మాట == RES_DO)
				కొనసాగు;
			/* next word is not "do". It must be "in" then ("FOR v in ...") */
			ఒకవేళ (cpipe->res_మాట == RES_లోపల /* "for v IN a b; not_do..."? */
			 || cpipe->తదుపరి->res_మాట != RES_లోపల /* FOR v not_do_and_not_in..."? */
			) {
				syntax_error("malformed for");
				debug_leave();
				debug_printf_exec("run_list lvl %d return 1\n", G.నిరంతరం_list_level);
				తిరిగి 1;
			}
		}
	}
#అయితే_ముగింపు

	/* Past this point, all code paths should jump to ret: label
	 * in order to return, no direct "return" statements please.
	 * This helps to ensure that no memory is leaked. */

#ఒకవేళ ENABLE_HUSH_పని
	G.నిరంతరం_list_level++;
#అయితే_ముగింపు

#ఒకవేళ HAS_KEYWORDS
	rword = RES_NONE;
	చివరి_rword = RES_XXXX;
#అయితే_ముగింపు
	చివరి_followup = PIPE_SEQ;
	rcode = G.చివరి_exitcode;

	/* Go through list of pipes, (maybe) executing them. */
#నిర్వచితం_కాకపోతే __U_BOOT__
	కొరకు (; pi; pi = IF_HUSH_LOOPS(rword == RES_పూర్తి ? loop_శిఖరం : ) pi->తదుపరి) {
#లేకపోతే /* __U_BOOT__ */
	కొరకు (; pi; pi = rword == RES_పూర్తి ? loop_శిఖరం : pi->తదుపరి) {
#అయితే_ముగింపు /* __U_BOOT__ */
		పూర్ణాంకం r;
		పూర్ణాంకం sv_errexit_లోతు;

#నిర్వచితం_కాకపోతే __U_BOOT__
		ఒకవేళ (G.flag_SIGINT)
			ఆపు;
		ఒకవేళ (G_flag_return_లోపల_పురోగతి == 1)
			ఆపు;
#అయితే_ముగింపు /* !__U_BOOT__ */

		IF_HAS_KEYWORDS(rword = pi->res_మాట;)
		debug_printf_exec(": rword:%d cond_code:%d last_rword:%d\n",
				rword, cond_సంకేతం, చివరి_rword);

		sv_errexit_లోతు = G.errexit_లోతు;
		ఒకవేళ (
#ఒకవేళ ENABLE_HUSH_IF
		    rword == RES_IF || rword == RES_ELIF ||
#అయితే_ముగింపు
		    pi->followup != PIPE_SEQ
		) {
			G.errexit_లోతు++;
		}
#ఒకవేళ ENABLE_HUSH_LOOPS
		ఒకవేళ ((rword == RES_WHILE || rword == RES_UNTIL || rword == RES_FOR)
		 && loop_శిఖరం == ఖాళీ /* avoid bumping G.depth_of_loop twice */
		) {
			/* start of a loop: remember where loop starts */
			loop_శిఖరం = pi;
			G.లోతు_యొక్క_loop++;
		}
#అయితే_ముగింపు
		/* Still in the same "if...", "then..." or "do..." branch? */
		ఒకవేళ (IF_HAS_KEYWORDS(rword == చివరి_rword &&) 1) {
			ఒకవేళ ((rcode == 0 && చివరి_followup == PIPE_OR)
			 || (rcode != 0 && చివరి_followup == PIPE_AND)
			) {
				/* It is "<true> || CMD" or "<false> && CMD"
				 * and we should not execute CMD */
				debug_printf_exec("skipped cmd because of || or &&\n");
				చివరి_followup = pi->followup;
				వెళ్ళు dont_check_jobs_but_continue;
			}
		}
		చివరి_followup = pi->followup;
#ఒకవేళ ENABLE_HUSH_IF
		ఒకవేళ (cond_సంకేతం != 0) {
			ఒకవేళ (rword == RES_THEN) {
				/* if false; then ... fi has exitcode 0! */
				G.చివరి_exitcode = rcode = EXIT_SUCCESS;
				/* "if <false> THEN cmd": skip cmd */
				debug_printf_exec("skipped THEN cmd because IF condition was false\n");
				చివరి_rword = rword;
				కొనసాగు;
			}
		} లేకపోతే {
			ఒకవేళ (rword == RES_ELSE
			 || (rword == RES_ELIF && చివరి_rword != RES_ELIF)
			) {
				/* "if <true> then ... ELSE/ELIF cmd":
				 * skip cmd and all following ones */
				debug_printf_exec("skipped ELSE/ELIF branch because IF condition was true\n");
				ఆపు;
			}
			//if (rword == RES_THEN): "if <true> THEN cmd", run cmd (fall through)
		}
#అయితే_ముగింపు
		IF_HAS_KEYWORDS(చివరి_rword = rword;)
#ఒకవేళ ENABLE_HUSH_LOOPS
		ఒకవేళ (rword == RES_FOR) { /* && pi->num_cmds - always == 1 */
			ఒకవేళ (!for_lcur) {
				/* first loop through for */

				స్థిర స్థిరరాశి అక్షరం encoded_dollar_at[] ALIGN1 = {
					SPECIAL_VAR_చిహ్నం, '@' | 0x80, SPECIAL_VAR_చిహ్నం, '\0'
				}; /* encoded representation of "$@" */
				స్థిర స్థిరరాశి అక్షరం *స్థిరరాశి encoded_dollar_at_argv[] ALIGN_PTR = {
					encoded_dollar_at, ఖాళీ
				}; /* argv list with one element: "$@" */
				అక్షరం **vals;

				G.చివరి_exitcode = rcode = EXIT_SUCCESS;
				vals = (అక్షరం**)encoded_dollar_at_argv;
				ఒకవేళ (pi->తదుపరి->res_మాట == RES_లోపల) {
					/* if no variable values after "in" we skip "for" */
					ఒకవేళ (!pi->తదుపరి->cmds[0].argv) {
						debug_printf_exec(": null FOR: exitcode EXIT_SUCCESS\n");
						ఆపు;
					}
					vals = pi->తదుపరి->cmds[0].argv;
				} /* else: "for var; do..." -> assume "$@" list */
				/* create list of variable values */
				debug_print_అక్షరశ్రేణులు("for_list made from", vals);
				for_list = expand_strvec_గమ్యం_strvec(vals);
				for_lcur = for_list;
				debug_print_అక్షరశ్రేణులు("for_list", for_list);
			}
			ఒకవేళ (!*for_lcur) {
				/* "for" loop is over, clean up */
				విడుదల(for_list);
				for_list = ఖాళీ;
				for_lcur = ఖాళీ;
				ఆపు;
			}
			/* Insert next value from for_lcur */
			/* note: *for_lcur already has quotes removed, $var expanded, etc */
#నిర్వచితం_కాకపోతే __U_BOOT__
			సమితి_local_var_from_halves(pi->cmds[0].argv[0], *for_lcur++);
#లేకపోతే /* __U_BOOT__ */
			/* We cannot use xasprintf, so we emulate it. */
			అక్షరం *నిండిన_var;
			అక్షరం *var = pi->cmds[0].argv[0];
			అక్షరం *val = *for_lcur++;

			/* + 1 to take into account =. */
			నిండిన_var = xmalloc(అక్షరశ్రేణి_పొడవు(var) + అక్షరశ్రేణి_పొడవు(val) + 1);
			sprintf(నిండిన_var, "%s=%s", var, val);

			సమితి_local_var_modern(నిండిన_var, /*flag:*/ 0);
#అయితే_ముగింపు /* __U_BOOT__ */
			కొనసాగు;
		}
		ఒకవేళ (rword == RES_లోపల) {
			కొనసాగు; /* "for v IN list;..." - "in" has no cmds anyway */
		}
		ఒకవేళ (rword == RES_పూర్తి) {
			కొనసాగు; /* "done" has no cmds too */
		}
#అయితే_ముగింపు
#ఒకవేళ ENABLE_HUSH_CASE
		ఒకవేళ (rword == RES_CASE) {
			debug_printf_exec("CASE cond_code:%d\n", cond_సంకేతం);
			case_మాట = expand_string_గమ్యం_string(pi->cmds->argv[0],
				EXP_FLAG_ESC_GLOB_CHARS, /*unbackslash:*/ 1);
			debug_printf_exec("CASE word1:'%s'\n", case_మాట);
			//unbackslash(case_word);
			//debug_printf_exec("CASE word2:'%s'\n", case_word);
			కొనసాగు;
		}
		ఒకవేళ (rword == RES_సరిపోలిక) {
			అక్షరం **argv;

			debug_printf_exec("MATCH cond_code:%d\n", cond_సంకేతం);
			ఒకవేళ (!case_మాట) /* "case ... matched_word) ... WORD)": we executed selected branch, stop */
				ఆపు;
			/* all prev words didn't match, does this one match? */
			argv = pi->cmds->argv;
			అయితే (*argv) {
				అక్షరం *నమూనా;
				debug_printf_exec("expand_string_to_string('%s')\n", *argv);
				నమూనా = expand_string_గమ్యం_string(*argv,
						EXP_FLAG_ESC_GLOB_CHARS,
						/*unbackslash:*/ 0
				);
				/* TODO: which FNM_xxx flags to use? */
				cond_సంకేతం = (fnmatch(నమూనా, case_మాట, /*flags:*/ 0) != 0);
				debug_printf_exec("cond_code=fnmatch(pattern:'%s',str:'%s'):%d\n",
						నమూనా, case_మాట, cond_సంకేతం);
				విడుదల(నమూనా);
				ఒకవేళ (cond_సంకేతం == 0) {
					/* match! we will execute this branch */
					విడుదల(case_మాట);
					case_మాట = ఖాళీ; /* make future "word)" stop */
					ఆపు;
				}
				argv++;
			}
			కొనసాగు;
		}
		ఒకవేళ (rword == RES_CASE_BODY) { /* inside of a case branch */
			debug_printf_exec("CASE_BODY cond_code:%d\n", cond_సంకేతం);
			ఒకవేళ (cond_సంకేతం != 0)
				కొనసాగు; /* not matched yet, skip this pipe */
		}
		ఒకవేళ (rword == RES_ESAC) {
			debug_printf_exec("ESAC cond_code:%d\n", cond_సంకేతం);
			ఒకవేళ (case_మాట) {
				/* "case" did not match anything: still set $? (to 0) */
				G.చివరి_exitcode = rcode = EXIT_SUCCESS;
			}
		}
#అయితే_ముగింపు
		/* Just pressing <enter> in shell should check for jobs.
		 * OTOH, in non-interactive shell this is useless
		 * and only leads to extra job checks */
		ఒకవేళ (pi->అంకె_cmds == 0) {
#నిర్వచితం_కాకపోతే __U_BOOT__
			ఒకవేళ (G_interactive_fd)
				వెళ్ళు check_jobs_and_continue;
#అయితే_ముగింపు /* !__U_BOOT__ */
			కొనసాగు;
		}

		/* After analyzing all keywords and conditions, we decided
		 * to execute this pipe. NB: have to do checkjobs(NULL)
		 * after run_pipe to collect any background children,
		 * even if list execution is to be stopped. */
		debug_printf_exec(": run_pipe with %d members\n", pi->అంకె_cmds);
#నిర్వచితం_కాకపోతే __U_BOOT__
#ఒకవేళ ENABLE_HUSH_LOOPS
		G.flag_break_continue = 0;
#అయితే_ముగింపు
#అయితే_ముగింపు /* !__U_BOOT__ */
#నిర్వచితం_కాకపోతే __U_BOOT__
		rcode = r = G.o_opt[OPT_O_NOEXEC] ? 0 : నిరంతరం_pipe(pi);
		/* NB: rcode is a smalluint, r is int */
#లేకపోతే /* __U_BOOT__ */
		rcode = r = నిరంతరం_pipe(pi); /* NB: rcode is a smalluint, r is int */
		ఒకవేళ (r <= EXIT_RET_సంకేతం) {
			పూర్ణాంకం మునుపటి_rcode = G.చివరి_exitcode;
			/*
			 * This magic is to get the exit code given by the user.
			 * Contrary to old shell code, we use + EXIT_RET_CODE as EXIT_RET_CODE
			 * equals -2.
			 */
			G.చివరి_exitcode = -r + EXIT_RET_సంకేతం;

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
			ఒకవేళ (మునుపటి_rcode != r) {
				/*
				 * If run from run_command, run_command_flags will be set, so we check
				 * this to know if we are in main input shell.
				 */
				ఒకవేళ (!G.నిరంతరం_command_flags)
					రాయి("exit not allowed from main input shell.\n");

				ఆపు;
			}
			కొనసాగు;
		}
#అయితే_ముగింపు /* __U_BOOT__ */
		ఒకవేళ (r != -1) {
			/* We ran a builtin, function, or group.
			 * rcode is already known
			 * and we don't need to wait for anything. */
			debug_printf_exec(": builtin/func exitcode %d\n", rcode);
			G.చివరి_exitcode = rcode;
#నిర్వచితం_కాకపోతే __U_BOOT__
			check_and_నిరంతరం_traps();
#అయితే_ముగింపు /* !__U_BOOT__ */
#ఒకవేళ ENABLE_HUSH_TRAP && ENABLE_HUSH_FUNCTIONS
			rcode = G.చివరి_exitcode; /* "return" in trap can change it, read back */
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే __U_BOOT__
#ఒకవేళ ENABLE_HUSH_LOOPS
			/* Was it "break" or "continue"? */
			ఒకవేళ (G.flag_break_continue) {
				smallint fbc = G.flag_break_continue;
				/* We might fall into outer *loop*,
				 * don't want to break it too */
				ఒకవేళ (loop_శిఖరం) {
					G.లోతు_break_continue--;
					ఒకవేళ (G.లోతు_break_continue == 0)
						G.flag_break_continue = 0;
					/* else: e.g. "continue 2" should *break* once, *then* continue */
				} /* else: "while... do... { we are here (innermost list is not a loop!) };...done" */
				ఒకవేళ (G.లోతు_break_continue != 0 || fbc == BC_BREAK) {
					checkjobs(ఖాళీ, 0 /*(no pid to wait for)*/);
					ఆపు;
				}
				/* "continue": simulate end of loop */
				rword = RES_పూర్తి;
				కొనసాగు;
			}
#అయితే_ముగింపు
			ఒకవేళ (G_flag_return_లోపల_పురోగతి == 1) {
				checkjobs(ఖాళీ, 0 /*(no pid to wait for)*/);
				ఆపు;
			}

		} లేకపోతే ఒకవేళ (pi->followup == PIPE_BG) {
			/* What does bash do with attempts to background builtins? */
			/* even bash 3.2 doesn't do that well with nested bg:
			 * try "{ { sleep 10; echo DEEP; } & echo HERE; } &".
			 * I'm NOT treating inner &'s as jobs */
#ఒకవేళ ENABLE_HUSH_పని
			ఒకవేళ (G.నిరంతరం_list_level == 1)
				చొప్పించు_పని_into_పట్టిక(pi);
#అయితే_ముగింపు
			/* Last command's pid goes to $! */
			G.చివరి_bg_pid = pi->cmds[pi->అంకె_cmds - 1].pid;
			G.చివరి_bg_pid_exitcode = 0;
			debug_printf_exec(": cmd&: exitcode EXIT_SUCCESS\n");
/* Check pi->pi_inverted? "! sleep 1 & echo $?": bash says 1. dash and ash say 0 */
			rcode = EXIT_SUCCESS;
			వెళ్ళు check_traps;
		} లేకపోతే {
#ఒకవేళ ENABLE_HUSH_పని
			ఒకవేళ (G.నిరంతరం_list_level == 1 && G_interactive_fd) {
				/* Waits for completion, then fg's main shell */
				rcode = checkjobs_and_fg_shell(pi);
				debug_printf_exec(": checkjobs_and_fg_shell exitcode %d\n", rcode);
				వెళ్ళు check_traps;
			}
#అయితే_ముగింపు
			/* This one just waits for completion */
			rcode = checkjobs(pi, 0 /*(no pid to wait for)*/);
			debug_printf_exec(": checkjobs exitcode %d\n", rcode);
 check_traps:
			G.చివరి_exitcode = rcode;
			check_and_నిరంతరం_traps();
#ఒకవేళ ENABLE_HUSH_TRAP && ENABLE_HUSH_FUNCTIONS
			rcode = G.చివరి_exitcode; /* "return" in trap can change it, read back */
#అయితే_ముగింపు
		}
#అయితే_ముగింపు /* !__U_BOOT__ */

#నిర్వచితం_కాకపోతే __U_BOOT__
		/* Handle "set -e" */
		ఒకవేళ (rcode != 0 && G.o_opt[OPT_O_ERREXIT]) {
			debug_printf_exec("ERREXIT:1 errexit_depth:%d\n", G.errexit_లోతు);
			ఒకవేళ (G.errexit_లోతు == 0)
				hush_exit(rcode);
		}
#లేకపోతే /* __U_BOOT__ */
		} /* if (r != -1) */
#అయితే_ముగింపు /* __U_BOOT__ */
		G.errexit_లోతు = sv_errexit_లోతు;

		/* Analyze how result affects subsequent commands */
#ఒకవేళ ENABLE_HUSH_IF
		ఒకవేళ (rword == RES_IF || rword == RES_ELIF) {
			debug_printf_exec("cond_code=rcode:%d\n", rcode);
			cond_సంకేతం = rcode;
		}
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే __U_BOOT__
 check_jobs_and_continue:
		checkjobs(ఖాళీ, 0 /*(no pid to wait for)*/);
#అయితే_ముగింపు /* !__U_BOOT__ */
 dont_check_jobs_but_continue: ;
#ఒకవేళ ENABLE_HUSH_LOOPS
		/* Beware of "while false; true; do ..."! */
		ఒకవేళ (pi->తదుపరి
		 && (pi->తదుపరి->res_మాట == RES_DO || pi->తదుపరి->res_మాట == RES_పూర్తి)
		 /* check for RES_DONE is needed for "while ...; do \n done" case */
		) {
			ఒకవేళ (rword == RES_WHILE) {
				ఒకవేళ (rcode) {
					/* "while false; do...done" - exitcode 0 */
					G.చివరి_exitcode = rcode = EXIT_SUCCESS;
					debug_printf_exec(": while expr is false: breaking (exitcode:EXIT_SUCCESS)\n");
					ఆపు;
				}
			}
			ఒకవేళ (rword == RES_UNTIL) {
				ఒకవేళ (!rcode) {
					debug_printf_exec(": until expr is true: breaking\n");
					ఆపు;
				}
			}
		}
#అయితే_ముగింపు
	} /* for (pi) */

#ఒకవేళ ENABLE_HUSH_పని
	G.నిరంతరం_list_level--;
#అయితే_ముగింపు
#ఒకవేళ ENABLE_HUSH_LOOPS
	ఒకవేళ (loop_శిఖరం)
		G.లోతు_యొక్క_loop--;
	విడుదల(for_list);
#అయితే_ముగింపు
#ఒకవేళ ENABLE_HUSH_CASE
	విడుదల(case_మాట);
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే __U_BOOT__
	debug_leave();
	debug_printf_exec("run_list lvl %d return %d\n", G.నిరంతరం_list_level, rcode);
#అయితే_ముగింపు /* !__U_BOOT__ */
	తిరిగి rcode;
}

/* Select which version we will use */
స్థిర పూర్ణాంకం నిరంతరం_and_free_list(నిర్మాణం pipe *pi)
{
	పూర్ణాంకం rcode = 0;
	debug_printf_exec("run_and_free_list entered\n");
#నిర్వచితం_కాకపోతే __U_BOOT__
	ఒకవేళ (!G.o_opt[OPT_O_NOEXEC]) {
#అయితే_ముగింపు /* !__U_BOOT__ */
		debug_printf_exec(": run_list: 1st pipe with %d cmds\n", pi->అంకె_cmds);
		rcode = నిరంతరం_list(pi);
#నిర్వచితం_కాకపోతే __U_BOOT__
	}
#అయితే_ముగింపు /* !__U_BOOT__ */
	/* free_pipe_list has the side effect of clearing memory.
	 * In the long run that function can be merged with run_list,
	 * but doing that now would hobble the debugging effort. */
	free_pipe_list(pi);
	debug_printf_exec("run_and_free_list return %d\n", rcode);
	తిరిగి rcode;
}

#నిర్వచితం_కాకపోతే __U_BOOT__
స్థిర శూన్య install_sighandlers(గుర్తులేని ముసుగు)
{
	sighandler_t old_handler;
	గుర్తులేని sig = 0;
	అయితే ((ముసుగు >>= 1) != 0) {
		sig++;
		ఒకవేళ (!(ముసుగు & 1))
			కొనసాగు;
		old_handler = install_sighandler(sig, pick_sighandler(sig));
		/* POSIX allows shell to re-enable SIGCHLD
		 * even if it was SIG_IGN on entry.
		 * Therefore we skip IGN check for it:
		 */
		ఒకవేళ (sig == SIGCHLD)
			కొనసాగు;
		/* Interactive bash re-enables SIGHUP which is SIG_IGNed on entry.
		 * Try:
		 * trap '' hup; bash; echo RET  # type "kill -hup $$", see SIGHUP having effect
		 * trap '' hup; bash -c 'kill -hup $$; echo ALIVE'  # here SIGHUP is SIG_IGNed
		 */
		ఒకవేళ (sig == SIGHUP && G_interactive_fd)
			కొనసాగు;
		/* Unless one of the above signals, is it SIG_IGN? */
		ఒకవేళ (old_handler == SIG_IGN) {
			/* oops... restore back to IGN, and record this fact */
			install_sighandler(sig, old_handler);
#ఒకవేళ ENABLE_HUSH_TRAP
			ఒకవేళ (!G_traps)
				G_traps = xzalloc(పరిమాణం(G_traps[0]) * NSIG);
			విడుదల(G_traps[sig]);
			G_traps[sig] = xzalloc(1); /* == xstrdup(""); */
#అయితే_ముగింపు
		}
	}
}

/* Called a few times only (or even once if "sh -c") */
స్థిర శూన్య install_special_sighandlers(శూన్య)
{
	గుర్తులేని ముసుగు;

	/* Which signals are shell-special? */
	ముసుగు = (1 << SIGQUIT) | (1 << SIGCHLD);
	ఒకవేళ (G_interactive_fd) {
		ముసుగు |= SPECIAL_INTERACTIVE_SIGS;
		ఒకవేళ (G_saved_tty_pgrp) /* we have ctty, job control sigs work */
			ముసుగు |= SPECIAL_JOBSTOP_SIGS;
	}
	/* Careful, do not re-install handlers we already installed */
	ఒకవేళ (G.special_sig_ముసుగు != ముసుగు) {
		గుర్తులేని diff = ముసుగు & ~G.special_sig_ముసుగు;
		G.special_sig_ముసుగు = ముసుగు;
		install_sighandlers(diff);
	}
}

#ఒకవేళ ENABLE_HUSH_పని
/* helper */
/* Set handlers to restore tty pgrp and exit */
స్థిర శూన్య install_fatal_sighandlers(శూన్య)
{
	గుర్తులేని ముసుగు;

	/* We will restore tty pgrp on these signals */
	ముసుగు = 0
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
	G_fatal_sig_ముసుగు = ముసుగు;

	install_sighandlers(ముసుగు);
}
#అయితే_ముగింపు

స్థిర పూర్ణాంకం సమితి_బహుళకం(పూర్ణాంకం state, అక్షరం బహుళకం, స్థిరరాశి అక్షరం *o_opt)
{
	పూర్ణాంకం idx;
	ఎంపిక (బహుళకం) {
	స్థితి 'n':
		/* set -n has no effect in interactive shell */
		/* Try: while set -n; do echo $-; done */
		ఒకవేళ (!G_interactive_fd)
			G.o_opt[OPT_O_NOEXEC] = state;
		ఆపు;
	స్థితి 'x':
		IF_HUSH_బహుళకం_X(G_x_బహుళకం = state;)
		IF_HUSH_బహుళకం_X(ఒకవేళ (G.x_బహుళకం_fd <= 0) G.x_బహుళకం_fd = dup_CLOEXEC(2, 10);)
		ఆపు;
	స్థితి 'e':
		G.o_opt[OPT_O_ERREXIT] = state;
		ఆపు;
	స్థితి 'o':
		ఒకవేళ (!o_opt) {
			/* "set -o" or "set +o" without parameter.
			 * in bash, set -o produces this output:
			 *  pipefail        off
			 * and set +o:
			 *  set +o pipefail
			 * We always use the second form.
			 */
			స్థిరరాశి అక్షరం *p = o_opt_అక్షరశ్రేణులు;
			idx = 0;
			అయితే (*p) {
				రాయి("set %co %s\n", (G.o_opt[idx] ? '-' : '+'), p);
				idx++;
				p += అక్షరశ్రేణి_పొడవు(p) + 1;
			}
			ఆపు;
		}
		idx = సూచీ_లోపల_అక్షరశ్రేణులు(o_opt_అక్షరశ్రేణులు, o_opt);
		ఒకవేళ (idx >= 0) {
			G.o_opt[idx] = state;
			ఆపు;
		}
		/* fall through to error */
	అప్రమేయ:
		తిరిగి EXIT_FAILURE;
	}
	తిరిగి EXIT_SUCCESS;
}

పూర్ణాంకం hush_main(పూర్ణాంకం argc, అక్షరం **argv) MAIN_EXTERNALLY_VISIBLE;
పూర్ణాంకం hush_main(పూర్ణాంకం argc, అక్షరం **argv)
{
	pid_t cached_getpid;
	గణన {
		OPT_login = (1 << 0),
	};
	గుర్తులేని flags;
#ఒకవేళ !BB_MMU
	గుర్తులేని builtin_argc = 0;
#అయితే_ముగింపు
	అక్షరం **e;
	నిర్మాణం variable *cur_var;
	నిర్మాణం variable *shell_ver;

	INIT_G();
	ఒకవేళ (EXIT_SUCCESS != 0) /* if EXIT_SUCCESS == 0, it is already done */
		G.చివరి_exitcode = EXIT_SUCCESS;
#ఒకవేళ !BB_MMU
	/* "Big heredoc" support via "sh -< STRING" invocation.
	 * Check it first (do not bother to run the usual init code,
	 * it is not needed for this case).
	 */
	ఒకవేళ (argv[1]
	 && argv[1][0] == '-' && argv[1][1] == '<' /*&& !argv[1][2]*/
	 /*&& argv[2] && !argv[3] - we don't check some conditions */
	) {
		నిండిన_write1_str(argv[2]);
		_exit(0);
	}
	G.argv0_for_re_execing = argv[0];
#అయితే_ముగింపు
#ఒకవేళ ENABLE_HUSH_TRAP
# ఒకవేళ ENABLE_HUSH_FUNCTIONS
	G.return_exitcode = -1;
# endif
	G.pre_trap_exitcode = -1;
#అయితే_ముగింపు

#ఒకవేళ ENABLE_HUSH_FAST
	G.లెక్క_SIGCHLD++; /* ensure it is != G.handled_SIGCHLD */
#అయితే_ముగింపు

	cached_getpid = getpid();   /* for tcsetpgrp() during init */
	G.మూలం_pid = cached_getpid; /* for $PID  (NOMMU can override via -$HEXPID:HEXPPID:...) */
	G.మూలం_ppid = getppid();    /* for $PPID (NOMMU can override) */

	/* Deal with HUSH_VERSION */
	debug_printf_env("unsetenv '%s'\n", "HUSH_VERSION");
	unsetenv("HUSH_VERSION"); /* in case it exists in initial env */
	shell_ver = xzalloc(పరిమాణం(*shell_ver));
	shell_ver->flg_export = 1;
	shell_ver->flg_read_only = 1;
	/* Code which handles ${var<op>...} needs writable values for all variables,
	 * therefore we xstrdup: */
	shell_ver->varstr = xstrdup(hush_సంస్కరణ_str);

	/* Create shell local variables from the values
	 * currently living in the environment */
	G.శిఖరం_var = shell_ver;
	cur_var = G.శిఖరం_var;
	e = environ;
	ఒకవేళ (e) అయితే (*e) {
		అక్షరం *విలువ = strchr(*e, '=');
		ఒకవేళ (విలువ) { /* paranoia */
			cur_var->తదుపరి = xzalloc(పరిమాణం(*cur_var));
			cur_var = cur_var->తదుపరి;
			cur_var->varstr = *e;
			cur_var->max_len = అక్షరశ్రేణి_పొడవు(*e);
			cur_var->flg_export = 1;
		}
		e++;
	}
	/* (Re)insert HUSH_VERSION into env (AFTER we scanned the env!) */
	debug_printf_env("putenv '%s'\n", shell_ver->varstr);
	putenv(shell_ver->varstr);

	/* Export PWD */
	సమితి_pwd_var(SETFLAG_EXPORT);

#ఒకవేళ BASH_HOSTNAME_VAR
	/* Set (but not export) HOSTNAME unless already set */
	ఒకవేళ (!get_local_var_విలువ("HOSTNAME")) {
		నిర్మాణం utsname uts;
		uname(&uts);
		సమితి_local_var_from_halves("HOSTNAME", uts.nodename);
	}
#అయితే_ముగింపు
	/* IFS is not inherited from the parent environment */
	సమితి_local_var_from_halves("IFS", defifs);

	ఒకవేళ (!get_local_var_విలువ("PATH"))
		సమితి_local_var_from_halves("PATH", bb_default_మూలం_మార్గం);

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

#ఒకవేళ అంకె_SCRIPTS > 0
	ఒకవేళ (argc < 0) {
		అక్షరం *script = get_script_content(-argc - 1);
		G.global_argv = argv;
		G.global_argc = string_array_len(argv);
		//install_special_sighandlers(); - needed?
		parse_and_నిరంతరం_string(script);
		వెళ్ళు final_return;
	}
#అయితే_ముగింపు

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
	అయితే (1) {
		పూర్ణాంకం opt = getopt(argc, argv, "+" /* stop at 1st non-option */
				"cexinsl"
#ఒకవేళ !BB_MMU
				"$:R:V:"
# ఒకవేళ ENABLE_HUSH_LINENO_VAR
				"L:"
# endif
# ఒకవేళ ENABLE_HUSH_FUNCTIONS
				"F:"
# endif
#అయితే_ముగింపు
		);
		ఒకవేళ (opt <= 0)
			ఆపు;
		ఎంపిక (opt) {
		స్థితి 'c':
			/* Note: -c is not an option with param!
			 * "hush -c -l SCRIPT" is valid. "hush -cSCRIPT" is not.
			 */
			G.opt_c = 1;
			ఆపు;
		స్థితి 'i':
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
//ఇక్కడ
//(under "bash", it's the opposite: it runs $BASH_ENV file only *without* -i).
//
//ash -i -c 'sleep 3; sleep 3', on ^C, drops into a prompt instead of exiting
//(this may be a bug, bash does not do this).
//(ash -i -c 'sleep 3' won't show this, the last command gets auto-"exec"ed)
//
//None of the above feel like useful features people would rely on.
			ఆపు;
		స్థితి 's':
			G.opt_s = 1;
			ఆపు;
		స్థితి 'l':
			flags |= OPT_login;
			ఆపు;
#ఒకవేళ !BB_MMU
		స్థితి '$': {
			గుర్తులేని దీర్ఘ దీర్ఘ ఖాళీలు_trap_ముసుగు;

			G.మూలం_pid = bb_strtou(optarg, &optarg, 16);
			optarg++;
			G.మూలం_ppid = bb_strtou(optarg, &optarg, 16);
			optarg++;
			G.చివరి_bg_pid = bb_strtou(optarg, &optarg, 16);
			optarg++;
			G.చివరి_exitcode = bb_strtou(optarg, &optarg, 16);
			optarg++;
			builtin_argc = bb_strtou(optarg, &optarg, 16);
			optarg++;
			ఖాళీలు_trap_ముసుగు = bb_strtoull(optarg, &optarg, 16);
			ఒకవేళ (ఖాళీలు_trap_ముసుగు != 0) {
				IF_HUSH_TRAP(పూర్ణాంకం sig;)
				install_special_sighandlers();
# ఒకవేళ ENABLE_HUSH_TRAP
				G_traps = xzalloc(పరిమాణం(G_traps[0]) * NSIG);
				కొరకు (sig = 1; sig < NSIG; sig++) {
					ఒకవేళ (ఖాళీలు_trap_ముసుగు & (1LL << sig)) {
						G_traps[sig] = xzalloc(1); /* == xstrdup(""); */
						install_sighandler(sig, SIG_IGN);
					}
				}
# endif
			}
# ఒకవేళ ENABLE_HUSH_LOOPS
			optarg++;
			G.లోతు_యొక్క_loop = bb_strtou(optarg, &optarg, 16);
# endif
			/* Suppress "killed by signal" message, -$ hack is used
			 * for subshells: echo `sh -c 'kill -9 $$'`
			 * should be silent.
			 */
			IF_HUSH_పని(G.నిరంతరం_list_level = 1;)
# ఒకవేళ ENABLE_HUSH_FUNCTIONS
			/* nommu uses re-exec trick for "... | func | ...",
			 * should allow "return".
			 * This accidentally allows returns in subshells.
			 */
			G_flag_return_లోపల_పురోగతి = -1;
# endif
			ఆపు;
		}
		స్థితి 'R':
		స్థితి 'V':
			సమితి_local_var(xstrdup(optarg), opt == 'R' ? SETFLAG_MAKE_RO : 0);
			ఆపు;
# ఒకవేళ ENABLE_HUSH_LINENO_VAR
		స్థితి 'L':
			G.parse_lineno = xatou(optarg);
			ఆపు;
# endif
# ఒకవేళ ENABLE_HUSH_FUNCTIONS
		స్థితి 'F': {
			నిర్మాణం function *funcp = new_function(optarg);
			/* funcp->name is already set to optarg */
			/* funcp->body is set to NULL. It's a special case. */
			funcp->body_as_string = argv[optind];
			optind++;
			ఆపు;
		}
# endif
#అయితే_ముగింపు
		/*case '?': invalid option encountered (set_mode('?') will fail) */
		/*case 'n':*/
		/*case 'x':*/
		/*case 'e':*/
		అప్రమేయ:
			ఒకవేళ (సమితి_బహుళకం(1, opt, ఖాళీ) == 0) /* no error */
				ఆపు;
			bb_show_usage();
		}
	} /* option parsing loop */

	/* Skip options. Try "hush -l": $1 should not be "-l"! */
	G.global_argc = argc - (optind - 1);
	G.global_argv = argv + (optind - 1);
	G.global_argv[0] = argv[0];

	/* If we are login shell... */
	ఒకవేళ (flags & OPT_login) {
		స్థిరరాశి అక్షరం *hp = ఖాళీ;
		HFILE *ఆదానం;

		debug_printf("sourcing /etc/profile\n");
		ఆదానం = hfopen("/etc/profile");
 నిరంతరం_profile:
		ఒకవేళ (ఆదానం != ఖాళీ) {
			install_special_sighandlers();
			parse_and_నిరంతరం_file(ఆదానం);
			hfclose(ఆదానం);
		}
		/* bash: after sourcing /etc/profile,
		 * tries to source (in the given order):
		 * ~/.bash_profile, ~/.bash_login, ~/.profile,
		 * stopping on first found. --noprofile turns this off.
		 * bash also sources ~/.bash_logout on exit.
		 * If called as sh, skips .bash_XXX files.
		 */
		ఒకవేళ (!hp) { /* unless we looped on the "goto" already */
			hp = get_local_var_విలువ("HOME");
			ఒకవేళ (hp && hp[0]) {
				debug_printf("sourcing ~/.profile\n");
				hp = concat_మార్గం_file(hp, ".profile");
				ఆదానం = hfopen(hp);
				విడుదల((అక్షరం*)hp);
				వెళ్ళు నిరంతరం_profile;
			}
		}
	}

#నిర్వచితం_కాకపోతే __U_BOOT__
	/* -c takes effect *after* -l */
	ఒకవేళ (G.opt_c) {
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
		అక్షరం *script;

		install_special_sighandlers();

		G.global_argc--;
		G.global_argv++;
#ఒకవేళ !BB_MMU
		ఒకవేళ (builtin_argc) {
			/* -c 'builtin' [BARGV...] "" ARG0 [ARG1...] */
			స్థిరరాశి నిర్మాణం built_లోపల_command *x;
			x = కనుగొను_builtin(G.global_argv[0]);
			ఒకవేళ (x) { /* paranoia */
				argv = G.global_argv;
				G.global_argc -= builtin_argc + 1; /* skip [BARGV...] "" */
				G.global_argv += builtin_argc + 1;
				G.global_argv[-1] = ఖాళీ; /* replace "" */
				G.చివరి_exitcode = x->b_function(argv);
			}
			వెళ్ళు final_return;
		}
#అయితే_ముగింపు

		script = G.global_argv[0];
		ఒకవేళ (!script)
			bb_error_సందేశం_and_die(bb_సందేశం_requires_arg, "-c");
		ఒకవేళ (!G.global_argv[1]) {
			/* -c 'script' (no params): prevent empty $0 */
			G.global_argv[0] = argv[0];
		} లేకపోతే { /* else -c 'script' ARG0 [ARG1...]: $0 is ARG0 */
			G.global_argc--;
			G.global_argv++;
		}
		parse_and_నిరంతరం_string(script);
		వెళ్ళు final_return;
	}

	/* -s is: hush -s ARGV1 ARGV2 (no SCRIPT) */
	ఒకవేళ (!G.opt_s && G.global_argv[1]) {
		HFILE *ఆదానం;
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
		ఆదానం = hfopen(G.global_argv[0]);
		ఒకవేళ (!ఆదానం) {
			bb_simple_perror_సందేశం_and_die(G.global_argv[0]);
		}
		xfunc_error_retval = 1;
		install_special_sighandlers();
		parse_and_నిరంతరం_file(ఆదానం);
#ఒకవేళ ENABLE_FEATURE_CLEAN_పైకి
		hfclose(ఆదానం);
#అయితే_ముగింపు
		వెళ్ళు final_return;
	}
	/* "implicit" -s: bare interactive hush shows 's' in $- */
	G.opt_s = 1;

#అయితే_ముగింపు /* __U_BOOT__ */
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
#ఒకవేళ ENABLE_HUSH_పని
	ఒకవేళ (isatty(STDIN_FILENO) && isatty(STDOUT_FILENO)) {
		G_saved_tty_pgrp = tcgetpgrp(STDIN_FILENO);
		debug_printf("saved_tty_pgrp:%d\n", G_saved_tty_pgrp);
		ఒకవేళ (G_saved_tty_pgrp < 0)
			G_saved_tty_pgrp = 0;

		/* try to dup stdin to high fd#, >= 255 */
		G_interactive_fd = dup_CLOEXEC(STDIN_FILENO, 254);
		ఒకవేళ (G_interactive_fd < 0) {
			/* try to dup to any fd */
			G_interactive_fd = dup_CLOEXEC(STDIN_FILENO, -1);
			ఒకవేళ (G_interactive_fd < 0) {
				/* give up */
				G_interactive_fd = 0;
				G_saved_tty_pgrp = 0;
			}
		}
	}
	debug_printf("interactive_fd:%d\n", G_interactive_fd);
	ఒకవేళ (G_interactive_fd) {
		ఒకవేళ (G_saved_tty_pgrp) {
			/* If we were run as 'hush &', sleep until we are
			 * in the foreground (tty pgrp == our pgrp).
			 * If we get started under a job aware app (like bash),
			 * make sure we are now in charge so we don't fight over
			 * who gets the foreground */
			అయితే (1) {
				pid_t shell_pgrp = getpgrp();
				G_saved_tty_pgrp = tcgetpgrp(G_interactive_fd);
				ఒకవేళ (G_saved_tty_pgrp == shell_pgrp)
					ఆపు;
				/* send TTIN to ourself (should stop us) */
				kill(- shell_pgrp, SIGTTIN);
			}
		}

		/* Install more signal handlers */
		install_special_sighandlers();

		ఒకవేళ (G_saved_tty_pgrp) {
			/* Set other signals to restore saved_tty_pgrp */
			install_fatal_sighandlers();
			/* Put ourselves in our own process group
			 * (bash, too, does this only if ctty is available) */
			bb_setpgrp(); /* is the same as setpgid(our_pid, our_pid); */
			/* Grab control of the terminal */
			tcsetpgrp(G_interactive_fd, cached_getpid);
		}
		enable_restore_tty_pgrp_on_exit();

# ఒకవేళ ENABLE_FEATURE_EDITING
		G.పంక్తి_ఆదానం_state = new_పంక్తి_ఆదానం_t(FOR_SHELL);
#  ఒకవేళ ENABLE_FEATURE_TAB_COMPLETION
		G.పంక్తి_ఆదానం_state->get_exe_name = hush_command_name;
#  endif
#  ఒకవేళ EDITING_HAS_sh_get_var
		G.పంక్తి_ఆదానం_state->sh_get_var = get_local_var_విలువ;
#  endif
# endif
# ఒకవేళ ENABLE_HUSH_SAVEHISTORY && MAX_HISTORY > 0
		{
			స్థిరరాశి అక్షరం *hp = get_local_var_విలువ("HISTFILE");
			ఒకవేళ (!hp) {
				hp = get_local_var_విలువ("HOME");
				ఒకవేళ (hp)
					hp = concat_మార్గం_file(hp, ".hush_history");
			} లేకపోతే {
				hp = xstrdup(hp);
			}
			ఒకవేళ (hp) {
				G.పంక్తి_ఆదానం_state->hist_file = hp;
				//set_local_var(xasprintf("HISTFILE=%s", ...));
			}
#  ఒకవేళ ENABLE_FEATURE_SH_HISTFILESIZE
			hp = get_local_var_విలువ("HISTFILESIZE");
			G.పంక్తి_ఆదానం_state->max_history = కొలత_from_HISTFILESIZE(hp);
#  endif
		}
# endif
	} లేకపోతే {
		install_special_sighandlers();
	}
#లేదావేళ ENABLE_HUSH_INTERACTIVE
	/* No job control compiled in, only prompt/line editing */
	ఒకవేళ (isatty(STDIN_FILENO) && isatty(STDOUT_FILENO)) {
		G_interactive_fd = dup_CLOEXEC(STDIN_FILENO, 254);
		ఒకవేళ (G_interactive_fd < 0) {
			/* try to dup to any fd */
			G_interactive_fd = dup_CLOEXEC(STDIN_FILENO, -1);
			ఒకవేళ (G_interactive_fd < 0)
				/* give up */
				G_interactive_fd = 0;
		}
	}
	install_special_sighandlers();
#లేకపోతే
	/* We have interactiveness code disabled */
	install_special_sighandlers();
#అయితే_ముగింపు
	/* bash:
	 * if interactive but not a login shell, sources ~/.bashrc
	 * (--norc turns this off, --rcfile <file> overrides)
	 */

	ఒకవేళ (G_interactive_fd) {
#ఒకవేళ ENABLE_HUSH_INTERACTIVE && ENABLE_FEATURE_EDITING_FANCY_PROMPT
		/* Set (but not export) PS1/2 unless already set */
		ఒకవేళ (!get_local_var_విలువ("PS1"))
			సమితి_local_var_from_halves("PS1", "\\w \\$ ");
		ఒకవేళ (!get_local_var_విలువ("PS2"))
			సమితి_local_var_from_halves("PS2", "> ");
#అయితే_ముగింపు
		ఒకవేళ (!ENABLE_FEATURE_SH_EXTRA_QUIET) {
			/* note: ash and hush share this string */
			రాయి("\n\n%s %s\n"
				IF_HUSH_HELP("Enter 'help' for a list of built-in commands.\n")
				"\n",
				bb_banner,
				"hush - the humble shell"
			);
		}
	}

	parse_and_నిరంతరం_file(hfopen(ఖాళీ)); /* stdin */

 final_return:
	hush_exit(G.చివరి_exitcode);
}

/*
 * Built-ins
 */
స్థిర పూర్ణాంకం FAST_FUNC builtin_true(అక్షరం **argv వాడని_PARAM)
{
	తిరిగి 0;
}

స్థిర పూర్ణాంకం FAST_FUNC builtin_false(అక్షరం **argv వాడని_PARAM)
{
	తిరిగి 1;
}

#ఒకవేళ ENABLE_HUSH_TEST || ENABLE_HUSH_ECHO || ENABLE_HUSH_PRINTF || ENABLE_HUSH_KILL
స్థిర NOINLINE పూర్ణాంకం నిరంతరం_applet_main(అక్షరం **argv, పూర్ణాంకం (*applet_main_func)(పూర్ణాంకం argc, అక్షరం **argv))
{
	పూర్ణాంకం argc = string_array_len(argv);
	తిరిగి applet_main_func(argc, argv);
}
#అయితే_ముగింపు
#ఒకవేళ ENABLE_HUSH_TEST || BASH_TEST2
స్థిర పూర్ణాంకం FAST_FUNC builtin_test(అక్షరం **argv)
{
	తిరిగి నిరంతరం_applet_main(argv, test_main);
}
#అయితే_ముగింపు
#ఒకవేళ ENABLE_HUSH_ECHO
స్థిర పూర్ణాంకం FAST_FUNC builtin_echo(అక్షరం **argv)
{
	తిరిగి నిరంతరం_applet_main(argv, echo_main);
}
#అయితే_ముగింపు
#ఒకవేళ ENABLE_HUSH_PRINTF
స్థిర పూర్ణాంకం FAST_FUNC builtin_printf(అక్షరం **argv)
{
	తిరిగి నిరంతరం_applet_main(argv, printf_main);
}
#అయితే_ముగింపు

#ఒకవేళ ENABLE_HUSH_HELP
స్థిర పూర్ణాంకం FAST_FUNC builtin_help(అక్షరం **argv వాడని_PARAM)
{
	స్థిరరాశి నిర్మాణం built_లోపల_command *x;

	రాయి(
		"Built-in commands:\n"
		"------------------\n");
	కొరకు (x = bltins1; x != &bltins1[ARRAY_కొలత(bltins1)]; x++) {
		ఒకవేళ (x->b_descr)
			రాయి("%-10s%s\n", x->b_cmd, x->b_descr);
	}
	తిరిగి EXIT_SUCCESS;
}
#అయితే_ముగింపు

#ఒకవేళ MAX_HISTORY && ENABLE_FEATURE_EDITING
స్థిర పూర్ణాంకం FAST_FUNC builtin_history(అక్షరం **argv వాడని_PARAM)
{
	show_history(G.పంక్తి_ఆదానం_state);
	తిరిగి EXIT_SUCCESS;
}
#అయితే_ముగింపు

స్థిర పూర్ణాంకం FAST_FUNC builtin_cd(అక్షరం **argv)
{
	స్థిరరాశి అక్షరం *newdir;

	argv = skip_dash_dash(argv);
	newdir = argv[0];
	ఒకవేళ (newdir == ఖాళీ) {
		/* bash does nothing (exitcode 0) if HOME is ""; if it's unset,
		 * bash says "bash: cd: HOME not set" and does nothing
		 * (exitcode 1)
		 */
		స్థిరరాశి అక్షరం *home = get_local_var_విలువ("HOME");
		newdir = home ? home : "/";
	}
	ఒకవేళ (chdir(newdir)) {
		/* Mimic bash message exactly */
		bb_perror_సందేశం("cd: %s", newdir);
		తిరిగి EXIT_FAILURE;
	}
	/* Read current dir (get_cwd(1) is inside) and set PWD.
	 * Note: do not enforce exporting. If PWD was unset or unexported,
	 * set it again, but do not export. bash does the same.
	 */
	సమితి_pwd_var(/*flag:*/ 0);
	తిరిగి EXIT_SUCCESS;
}

స్థిర పూర్ణాంకం FAST_FUNC builtin_pwd(అక్షరం **argv వాడని_PARAM)
{
	puts(get_cwd(0));
	తిరిగి EXIT_SUCCESS;
}

స్థిర పూర్ణాంకం FAST_FUNC builtin_eval(అక్షరం **argv)
{
	argv = skip_dash_dash(argv);

	ఒకవేళ (!argv[0])
		తిరిగి EXIT_SUCCESS;

	IF_HUSH_బహుళకం_X(G.x_బహుళకం_లోతు++;)
	//bb_error_msg("%s: ++x_mode_depth=%d", __func__, G.x_mode_depth);
	ఒకవేళ (!argv[1]) {
		/* bash:
		 * eval "echo Hi; done" ("done" is syntax error):
		 * "echo Hi" will not execute too.
		 */
		parse_and_నిరంతరం_string(argv[0]);
	} లేకపోతే {
		/* "The eval utility shall construct a command by
		 * concatenating arguments together, separating
		 * each with a <space> character."
		 */
		అక్షరం *str, *p;
		గుర్తులేని len = 0;
		అక్షరం **pp = argv;
		చేయి
			len += అక్షరశ్రేణి_పొడవు(*pp) + 1;
		అయితే (*++pp);
		str = p = xmalloc(len);
		pp = argv;
		కొరకు (;;) {
			p = stpcpy(p, *pp);
			pp++;
			ఒకవేళ (!*pp)
				ఆపు;
			*p++ = ' ';
		}
		parse_and_నిరంతరం_string(str);
		విడుదల(str);
	}
	IF_HUSH_బహుళకం_X(G.x_బహుళకం_లోతు--;)
	//bb_error_msg("%s: --x_mode_depth=%d", __func__, G.x_mode_depth);
	తిరిగి G.చివరి_exitcode;
}

స్థిర పూర్ణాంకం FAST_FUNC builtin_exec(అక్షరం **argv)
{
	argv = skip_dash_dash(argv);
	ఒకవేళ (argv[0] == ఖాళీ)
		తిరిగి EXIT_SUCCESS; /* bash does this */

	/* Careful: we can end up here after [v]fork. Do not restore
	 * tty pgrp then, only top-level shell process does that */
	ఒకవేళ (G_saved_tty_pgrp && getpid() == G.మూలం_pid)
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

స్థిర పూర్ణాంకం FAST_FUNC builtin_exit(అక్షరం **argv)
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
	ఒకవేళ (argv[0] == ఖాళీ) {
#ఒకవేళ ENABLE_HUSH_TRAP
		ఒకవేళ (G.pre_trap_exitcode >= 0) /* "exit" in trap uses $? from before the trap */
			hush_exit(G.pre_trap_exitcode);
#అయితే_ముగింపు
		hush_exit(G.చివరి_exitcode);
	}
	/* mimic bash: exit 123abc == exit 255 + error msg */
	xfunc_error_retval = 255;
	/* bash: exit -2 == exit 254, no error msg */
	hush_exit(xatoi(argv[0]) & 0xff);
}

#ఒకవేళ ENABLE_HUSH_TYPE
/* http://www.opengroup.org/onlinepubs/9699919799/utilities/type.html */
స్థిర పూర్ణాంకం FAST_FUNC builtin_type(అక్షరం **argv)
{
	పూర్ణాంకం ret = EXIT_SUCCESS;

	అయితే (*++argv) {
		స్థిరరాశి అక్షరం *type;
		అక్షరం *మార్గం_2 = ఖాళీ;

		ఒకవేళ (0) {} /* make conditional compile easier below */
		/*else if (find_alias(*argv))
			type = "an alias";*/
# ఒకవేళ ENABLE_HUSH_FUNCTIONS
		లేకపోతే ఒకవేళ (కనుగొను_function(*argv))
			type = "a function";
# endif
		లేకపోతే ఒకవేళ (కనుగొను_builtin(*argv))
			type = "a shell builtin";
		లేకపోతే ఒకవేళ ((మార్గం_2 = కనుగొను_లోపల_మార్గం(*argv)) != ఖాళీ)
			type = మార్గం_2;
		లేకపోతే {
			bb_error_సందేశం("type: %s: not found", *argv);
			ret = EXIT_FAILURE;
			కొనసాగు;
		}

		రాయి("%s is %s\n", *argv, type);
		విడుదల(మార్గం_2);
	}

	తిరిగి ret;
}
#అయితే_ముగింపు

#ఒకవేళ ENABLE_HUSH_READ
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
స్థిర పూర్ణాంకం FAST_FUNC builtin_read(అక్షరం **argv)
{
	స్థిరరాశి అక్షరం *r;
	నిర్మాణం builtin_read_params params;

	memset(&params, 0, పరిమాణం(params));

	/* "!": do not abort on errors.
	 * Option string must start with "sr" to match BUILTIN_READ_xxx
	 */
	params.read_flags = getopt32(argv,
# ఒకవేళ BASH_READ_D
		IF_NOT_HUSH_BASH_COMPAT("^")
		"!srn:p:t:u:d:" IF_NOT_HUSH_BASH_COMPAT("\0" "-1"/*min 1 arg*/),
		&params.opt_n, &params.opt_p, &params.opt_t, &params.opt_u, &params.opt_d
# లేకపోతే
		IF_NOT_HUSH_BASH_COMPAT("^")
		"!srn:p:t:u:" IF_NOT_HUSH_BASH_COMPAT("\0" "-1"/*min 1 arg*/),
		&params.opt_n, &params.opt_p, &params.opt_t, &params.opt_u
# endif
//TODO: print "read: need variable name"
//for the case of !BASH "read" with no args (now it fails silently)
//(or maybe extend getopt32() to emit a message if "-1" fails)
	);
	ఒకవేళ ((uint32_t)params.read_flags == (uint32_t)-1)
		తిరిగి EXIT_FAILURE;
	argv += optind;
	params.argv = argv;
	params.setvar = సమితి_local_var_from_halves;
	params.ifs = get_local_var_విలువ("IFS"); /* can be NULL */

 again:
	r = shell_builtin_read(&params);

	ఒకవేళ ((uintptr_t)r == 1 && errno == EINTR) {
		గుర్తులేని sig = check_and_నిరంతరం_traps();
		ఒకవేళ (sig != SIGINT)
			వెళ్ళు again;
	}

	ఒకవేళ ((uintptr_t)r > 1) {
		bb_simple_error_సందేశం(r);
		r = (అక్షరం*)(uintptr_t)1;
	}

	తిరిగి (uintptr_t)r;
}
#అయితే_ముగింపు

#ఒకవేళ ENABLE_HUSH_UMASK
స్థిర పూర్ణాంకం FAST_FUNC builtin_umask(అక్షరం **argv)
{
	పూర్ణాంకం rc;
	mode_t ముసుగు;

	rc = 1;
	ముసుగు = umask(0);
	argv = skip_dash_dash(argv);
	ఒకవేళ (argv[0]) {
		mode_t old_ముసుగు = ముసుగు;

		/* numeric umasks are taken as-is */
		/* symbolic umasks are inverted: "umask a=rx" calls umask(222) */
		ఒకవేళ (!అంకెనా(argv[0][0]))
			ముసుగు ^= 0777;
		ముసుగు = bb_parse_బహుళకం(argv[0], ముసుగు);
		ఒకవేళ (!అంకెనా(argv[0][0]))
			ముసుగు ^= 0777;
		ఒకవేళ ((గుర్తులేని)ముసుగు > 0777) {
			ముసుగు = old_ముసుగు;
			/* bash messages:
			 * bash: umask: 'q': invalid symbolic mode operator
			 * bash: umask: 999: octal number out of range
			 */
			bb_error_సందేశం("%s: invalid mode '%s'", "umask", argv[0]);
			rc = 0;
		}
	} లేకపోతే {
		/* Mimic bash */
		రాయి("%04o\n", (గుర్తులేని) ముసుగు);
		/* fall through and restore mask which we set to 0 */
	}
	umask(ముసుగు);

	తిరిగి !rc; /* rc != 0 - success */
}
#అయితే_ముగింపు

#ఒకవేళ ENABLE_HUSH_EXPORT || ENABLE_HUSH_READONLY || ENABLE_HUSH_సమితి || ENABLE_HUSH_TRAP
స్థిర శూన్య print_escaped(స్థిరరాశి అక్షరం *s)
{
//TODO? bash "set" does not quote variables which contain only alnums and "%+,-./:=@_~",
// (but "export" quotes all variables, even with only these chars).
// I think quoting strings with %+,=~ looks better
// (example: "set" printing var== instead of var='=' looks strange)
// IOW: do not quote "-./:@_": / is used in pathnames, : in PATH, -._ often in file names, @ in emails

	ఒకవేళ (*s == '\'')
		వెళ్ళు squote;
	చేయి {
		స్థిరరాశి అక్షరం *p = strchrnul(s, '\'');
		/* print 'xxxx', possibly just '' */
		రాయి("'%.*s'", (పూర్ణాంకం)(p - s), s);
		ఒకవేళ (*p == '\0')
			ఆపు;
		s = p;
 squote:
		/* s points to '; print "'''...'''" */
		అక్షరం_రాయి('"');
		చేయి అక్షరం_రాయి('\''); అయితే (*++s == '\'');
		అక్షరం_రాయి('"');
	} అయితే (*s);
}
#అయితే_ముగింపు

#ఒకవేళ ENABLE_HUSH_EXPORT || ENABLE_HUSH_LOCAL || ENABLE_HUSH_READONLY
స్థిర పూర్ణాంకం helper_export_local(అక్షరం **argv, గుర్తులేని flags)
{
	చేయి {
		అక్షరం *name = *argv;
		స్థిరరాశి అక్షరం *name_చివర = endofname(name);

		ఒకవేళ (*name_చివర == '\0') {
			నిర్మాణం variable *var, **vpp;

			vpp = get_ptr_గమ్యం_local_var(name);
			var = vpp ? *vpp : ఖాళీ;

			ఒకవేళ (flags & SETFLAG_UNEXPORT) {
				/* export -n NAME (without =VALUE) */
				ఒకవేళ (var) {
					var->flg_export = 0;
					debug_printf_env("%s: unsetenv '%s'\n", __func__, name);
					unsetenv(name);
				} /* else: export -n NOT_EXISTING_VAR: no-op */
				కొనసాగు;
			}
			ఒకవేళ (flags & SETFLAG_EXPORT) {
				/* export NAME (without =VALUE) */
				ఒకవేళ (var) {
					var->flg_export = 1;
					debug_printf_env("%s: putenv '%s'\n", __func__, var->varstr);
					putenv(var->varstr);
					కొనసాగు;
				}
			}
			ఒకవేళ (flags & SETFLAG_MAKE_RO) {
				/* readonly NAME (without =VALUE) */
				ఒకవేళ (var) {
					var->flg_read_only = 1;
					కొనసాగు;
				}
			}
# ఒకవేళ ENABLE_HUSH_LOCAL
			/* Is this "local" bltin? */
			ఒకవేళ (!(flags & (SETFLAG_EXPORT|SETFLAG_UNEXPORT|SETFLAG_MAKE_RO))) {
				గుర్తులేని lvl = flags >> SETFLAG_VARLVL_జరుపు;
				ఒకవేళ (var && var->var_nest_level == lvl) {
					/* "local x=abc; ...; local x" - ignore second local decl */
					కొనసాగు;
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
		} లేకపోతే {
			ఒకవేళ (*name_చివర != '=') {
				bb_error_సందేశం("'%s': bad variable name", name);
				/* do not parse following argv[]s: */
				తిరిగి 1;
			}
			/* (Un)exporting/making local NAME=VALUE */
			name = xstrdup(name);
			/* Testcase: export PS1='\w \$ ' */
			unbackslash(name);
		}
		debug_printf_env("%s: set_local_var('%s')\n", __func__, name);
		ఒకవేళ (సమితి_local_var(name, flags))
			తిరిగి EXIT_FAILURE;
	} అయితే (*++argv);
	తిరిగి EXIT_SUCCESS;
}
#అయితే_ముగింపు

#ఒకవేళ ENABLE_HUSH_EXPORT
స్థిర పూర్ణాంకం FAST_FUNC builtin_export(అక్షరం **argv)
{
	గుర్తులేని opt_unexport;

# ఒకవేళ ENABLE_HUSH_EXPORT_N
	/* "!": do not abort on errors */
	opt_unexport = getopt32(argv, "!n");
	ఒకవేళ (opt_unexport == (uint32_t)-1)
		తిరిగి EXIT_FAILURE;
	argv += optind;
# లేకపోతే
	opt_unexport = 0;
	argv++;
# endif

	ఒకవేళ (argv[0] == ఖాళీ) {
		అక్షరం **e = environ;
		ఒకవేళ (e) {
			అయితే (*e) {
# ఒకవేళ 0
				puts(*e++);
# లేకపోతే
				/* ash emits: export VAR='VAL'
				 * bash: declare -x VAR="VAL"
				 * we follow ash example */
				స్థిరరాశి అక్షరం *s = *e++;
				స్థిరరాశి అక్షరం *p = strchr(s, '=');

				ఒకవేళ (!p) /* wtf? take next variable */
					కొనసాగు;
				/* "export VAR=" */
				రాయి("%s %.*s", "export", (పూర్ణాంకం)(p - s) + 1, s);
				print_escaped(p + 1);
				అక్షరం_రాయి('\n');
# endif
			}
			/*fflush_all(); - done after each builtin anyway */
		}
		తిరిగి EXIT_SUCCESS;
	}

	తిరిగి helper_export_local(argv, opt_unexport ? SETFLAG_UNEXPORT : SETFLAG_EXPORT);
}
#అయితే_ముగింపు

#ఒకవేళ ENABLE_HUSH_LOCAL
స్థిర పూర్ణాంకం FAST_FUNC builtin_local(అక్షరం **argv)
{
	ఒకవేళ (G.func_nest_level == 0) {
		bb_error_సందేశం("%s: not in a function", argv[0]);
		తిరిగి EXIT_FAILURE; /* bash compat */
	}
//TODO? ash and bash support "local -" special form,
//which saves/restores $- around function call (including async returns, such as ^C)
//(IOW: it makes "set +/-..." effects local)
	argv++;
	/* Since all builtins run in a nested variable level,
	 * need to use level - 1 here. Or else the variable will be removed at once
	 * after builtin returns.
	 */
	తిరిగి helper_export_local(argv, (G.var_nest_level - 1) << SETFLAG_VARLVL_జరుపు);
}
#అయితే_ముగింపు

#ఒకవేళ ENABLE_HUSH_READONLY
స్థిర పూర్ణాంకం FAST_FUNC builtin_readonly(అక్షరం **argv)
{
	argv++;
	ఒకవేళ (*argv == ఖాళీ) {
		/* bash: readonly [-p]: list all readonly VARs
		 * (-p has no effect in bash)
		 */
		నిర్మాణం variable *e;
		కొరకు (e = G.శిఖరం_var; e; e = e->తదుపరి) {
			ఒకవేళ (e->flg_read_only) {
				స్థిరరాశి అక్షరం *s = e->varstr;
				స్థిరరాశి అక్షరం *p = strchr(s, '=');

				ఒకవేళ (!p) /* wtf? take next variable */
					కొనసాగు;
				/* "readonly VAR=" */
				రాయి("%s %.*s", "readonly", (పూర్ణాంకం)(p - s) + 1, s);
				print_escaped(p + 1);
				అక్షరం_రాయి('\n');
			}
		}
		తిరిగి EXIT_SUCCESS;
	}
	తిరిగి helper_export_local(argv, SETFLAG_MAKE_RO);
}
#అయితే_ముగింపు

#ఒకవేళ ENABLE_HUSH_UNSET
/* http://www.opengroup.org/onlinepubs/9699919799/utilities/V3_chap02.html#unset */
స్థిర పూర్ణాంకం FAST_FUNC builtin_unset(అక్షరం **argv)
{
	పూర్ణాంకం ret;
	గుర్తులేని opts;

	/* "!": do not abort on errors */
	/* "+": stop at 1st non-option */
	opts = getopt32(argv, "!+vf");
	ఒకవేళ (opts == (గుర్తులేని)-1)
		తిరిగి EXIT_FAILURE;
	ఒకవేళ (opts == 3) {
		bb_simple_error_సందేశం("unset: -v and -f are exclusive");
		తిరిగి EXIT_FAILURE;
	}
	argv += optind;

	ret = EXIT_SUCCESS;
	అయితే (*argv) {
		ఒకవేళ (!(opts & 2)) { /* not -f */
			ఒకవేళ (unset_local_var(*argv)) {
				/* unset <nonexistent_var> doesn't fail.
				 * Error is when one tries to unset RO var.
				 * Message was printed by unset_local_var. */
				ret = EXIT_FAILURE;
			}
		}
# ఒకవేళ ENABLE_HUSH_FUNCTIONS
		లేకపోతే {
			unset_func(*argv);
		}
# endif
		argv++;
	}
	తిరిగి ret;
}
#అయితే_ముగింపు

#ఒకవేళ ENABLE_HUSH_సమితి
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
స్థిర పూర్ణాంకం FAST_FUNC builtin_సమితి(అక్షరం **argv)
{
	పూర్ణాంకం n;
	అక్షరం **pp, **g_argv;
	అక్షరం *arg = *++argv;

	ఒకవేళ (arg == ఖాళీ) {
		నిర్మాణం variable *e;
		కొరకు (e = G.శిఖరం_var; e; e = e->తదుపరి) {
			స్థిరరాశి అక్షరం *s = e->varstr;
			స్థిరరాశి అక్షరం *p = strchr(s, '=');

			ఒకవేళ (!p) /* wtf? take next variable */
				కొనసాగు;
			/* var= */
			రాయి("%.*s", (పూర్ణాంకం)(p - s) + 1, s);
			print_escaped(p + 1);
			అక్షరం_రాయి('\n');
		}
		తిరిగి EXIT_SUCCESS;
	}

	చేయి {
		ఒకవేళ (strcmp(arg, "--") == 0) {
			++argv;
			వెళ్ళు సమితి_argv;
		}
		ఒకవేళ (arg[0] != '+' && arg[0] != '-')
			ఆపు;
		కొరకు (n = 1; arg[n]; ++n) {
			ఒకవేళ (సమితి_బహుళకం((arg[0] == '-'), arg[n], argv[1])) {
				bb_error_సందేశం("%s: %s: invalid option", "set", arg);
				తిరిగి EXIT_FAILURE;
			}
			ఒకవేళ (arg[n] == 'o' && argv[1])
				argv++;
		}
	} అయితే ((arg = *++argv) != ఖాళీ);
	/* Now argv[0] is 1st argument */

	ఒకవేళ (arg == ఖాళీ)
		తిరిగి EXIT_SUCCESS;
 సమితి_argv:

	/* NB: G.global_argv[0] ($0) is never freed/changed */
	g_argv = G.global_argv;
	ఒకవేళ (G.global_వాదనలు_malloced) {
		pp = g_argv;
		అయితే (*++pp)
			విడుదల(*pp);
		g_argv[1] = ఖాళీ;
	} లేకపోతే {
		G.global_వాదనలు_malloced = 1;
		pp = xzalloc(పరిమాణం(pp[0]) * 2);
		pp[0] = g_argv[0]; /* retain $0 */
		g_argv = pp;
	}
	/* This realloc's G.global_argv */
	G.global_argv = pp = కూడు_అక్షరశ్రేణులు_గమ్యం_అక్షరశ్రేణులు(g_argv, argv, /*dup:*/ 1);

	G.global_argc = 1 + string_array_len(pp + 1);

	తిరిగి EXIT_SUCCESS;
}
#అయితే_ముగింపు

స్థిర పూర్ణాంకం FAST_FUNC builtin_జరుపు(అక్షరం **argv)
{
	పూర్ణాంకం n = 1;
	argv = skip_dash_dash(argv);
	ఒకవేళ (argv[0]) {
		n = bb_strtou(argv[0], ఖాళీ, 10);
		ఒకవేళ (errno || n < 0) {
			/* shared string with ash.c */
			bb_error_సందేశం("Illegal number: %s", argv[0]);
			/*
			 * ash aborts in this case.
			 * bash prints error message and set $? to 1.
			 * Interestingly, for "shift 99999" bash does not
			 * print error message, but does set $? to 1
			 * (and does no shifting at all).
			 */
		}
	}
	ఒకవేళ (n >= 0 && n < G.global_argc) {
		ఒకవేళ (G_global_వాదనలు_malloced) {
			పూర్ణాంకం m = 1;
			అయితే (m <= n)
				విడుదల(G.global_argv[m++]);
		}
		G.global_argc -= n;
		memmove(&G.global_argv[1], &G.global_argv[n+1],
				G.global_argc * పరిమాణం(G.global_argv[0]));
		తిరిగి EXIT_SUCCESS;
	}
	తిరిగి EXIT_FAILURE;
}

#ఒకవేళ ENABLE_HUSH_GETOPTS
స్థిర పూర్ణాంకం FAST_FUNC builtin_getopts(అక్షరం **argv)
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
	అక్షరం cbuf[2];
	స్థిరరాశి అక్షరం *cp, *optstring, *var;
	పూర్ణాంకం c, n, exitcode, my_opterr;
	గుర్తులేని లెక్క;

	optstring = *++argv;
	ఒకవేళ (!optstring || !(var = *++argv)) {
		bb_simple_error_సందేశం("usage: getopts OPTSTRING VAR [ARGS]");
		తిరిగి EXIT_FAILURE;
	}

	ఒకవేళ (argv[1])
		argv[0] = G.global_argv[0]; /* for error messages in getopt() */
	లేకపోతే
		argv = G.global_argv;
	cbuf[1] = '\0';

	my_opterr = 0;
	ఒకవేళ (optstring[0] != ':') {
		cp = get_local_var_విలువ("OPTERR");
		/* 0 if "OPTERR=0", 1 otherwise */
		my_opterr = (!cp || NOT_LONE_CHAR(cp, '0'));
	}

	/* getopts stops on first non-option. Add "+" to force that */
	/*if (optstring[0] != '+')*/ {
		అక్షరం *s = alloca(అక్షరశ్రేణి_పొడవు(optstring) + 2);
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
	లెక్క = 0;
	n = string_array_len(argv);
	చేయి {
		optarg = ఖాళీ;
		opterr = (లెక్క < G.getopt_లెక్క) ? 0 : my_opterr;
		c = getopt(n, argv, optstring);
		ఒకవేళ (c < 0)
			ఆపు;
		లెక్క++;
	} అయితే (లెక్క <= G.getopt_లెక్క);

	/* Set OPTIND. Prevent resetting of the magic counter! */
	సమితి_local_var_from_halves("OPTIND", utoa(optind));
	G.getopt_లెక్క = లెక్క; /* "next time, give me N+1'th result" */
	GETOPT_RESET(); /* just in case */

	/* Set OPTARG */
	/* Always set or unset, never left as-is, even on exit/error:
	 * "If no option was found, or if the option that was found
	 * does not have an option-argument, OPTARG shall be unset."
	 */
	cp = optarg;
	ఒకవేళ (c == '?') {
		/* If ":optstring" and unknown option is seen,
		 * it is stored to OPTARG.
		 */
		ఒకవేళ (optstring[1] == ':') {
			cbuf[0] = optopt;
			cp = cbuf;
		}
	}
	ఒకవేళ (cp)
		సమితి_local_var_from_halves("OPTARG", cp);
	లేకపోతే
		unset_local_var("OPTARG");

	/* Convert -1 to "?" */
	exitcode = EXIT_SUCCESS;
	ఒకవేళ (c < 0) { /* -1: end of options */
		exitcode = EXIT_FAILURE;
		c = '?';
	}

	/* Set VAR */
	cbuf[0] = c;
	సమితి_local_var_from_halves(var, cbuf);

	తిరిగి exitcode;
}
#అయితే_ముగింపు

స్థిర పూర్ణాంకం FAST_FUNC builtin_ఆధారం(అక్షరం **argv)
{
	అక్షరం *arg_మార్గం, *filename;
	HFILE *ఆదానం;
	save_arg_t sv;
	అక్షరం *వాదనలు_need_save;
#ఒకవేళ ENABLE_HUSH_FUNCTIONS
	smallint sv_flg;
#అయితే_ముగింపు

	argv = skip_dash_dash(argv);
	filename = argv[0];
	ఒకవేళ (!filename) {
		/* bash says: "bash: .: filename argument required" */
		తిరిగి 2; /* bash compat */
	}
	arg_మార్గం = ఖాళీ;
	ఒకవేళ (!strchr(filename, '/')) {
		arg_మార్గం = కనుగొను_లోపల_మార్గం(filename);
		ఒకవేళ (arg_మార్గం)
			filename = arg_మార్గం;
		లేకపోతే ఒకవేళ (!ENABLE_HUSH_BASH_ఆధారం_CURDIR) {
			errno = ENOENT;
			bb_simple_perror_సందేశం(filename);
			తిరిగి EXIT_FAILURE;
		}
	}
	ఆదానం = hfopen(filename);
	విడుదల(arg_మార్గం);
	ఒకవేళ (!ఆదానం) {
		bb_perror_సందేశం("%s", filename);
		/* POSIX: non-interactive shell should abort here,
		 * not merely fail. So far no one complained :)
		 */
		తిరిగి EXIT_FAILURE;
	}

#ఒకవేళ ENABLE_HUSH_FUNCTIONS
	sv_flg = G_flag_return_లోపల_పురోగతి;
	/* "we are inside sourced file, ok to use return" */
	G_flag_return_లోపల_పురోగతి = -1;
#అయితే_ముగింపు
	వాదనలు_need_save = argv[1]; /* used as a boolean variable */
	ఒకవేళ (వాదనలు_need_save)
		save_and_replace_G_వాదనలు(&sv, argv);

	/* "false; . ./empty_line; echo Zero:$?" should print 0 */
	G.చివరి_exitcode = 0;
	parse_and_నిరంతరం_file(ఆదానం);
	hfclose(ఆదానం);

	ఒకవేళ (వాదనలు_need_save) /* can't use argv[1] instead: "shift" can mangle it */
		restore_G_వాదనలు(&sv, argv);
#ఒకవేళ ENABLE_HUSH_FUNCTIONS
	G_flag_return_లోపల_పురోగతి = sv_flg;
#అయితే_ముగింపు

	తిరిగి G.చివరి_exitcode;
}

#ఒకవేళ ENABLE_HUSH_TRAP
స్థిర పూర్ణాంకం FAST_FUNC builtin_trap(అక్షరం **argv)
{
	పూర్ణాంకం sig;
	అక్షరం *new_cmd;

	ఒకవేళ (!G_traps)
		G_traps = xzalloc(పరిమాణం(G_traps[0]) * NSIG);

	argv++;
	ఒకవేళ (!*argv) {
		పూర్ణాంకం i;
		/* No args: print all trapped */
		కొరకు (i = 0; i < NSIG; ++i) {
			ఒకవేళ (G_traps[i]) {
				రాయి("trap -- ");
				print_escaped(G_traps[i]);
				/* note: bash adds "SIG", but only if invoked
				 * as "bash". If called as "sh", or if set -o posix,
				 * then it prints short signal names.
				 * We are printing short names: */
				రాయి(" %s\n", get_signame(i));
			}
		}
		/*fflush_all(); - done after each builtin anyway */
		తిరిగి EXIT_SUCCESS;
	}

	new_cmd = ఖాళీ;
	/* If first arg is a number: reset all specified signals */
	sig = bb_strtou(*argv, ఖాళీ, 10);
	ఒకవేళ (errno == 0) {
		పూర్ణాంకం ret;
 process_sig_list:
		ret = EXIT_SUCCESS;
		అయితే (*argv) {
			sighandler_t handler;

			sig = get_signum(*argv++);
			ఒకవేళ (sig < 0) {
				ret = EXIT_FAILURE;
				/* Mimic bash message exactly */
				bb_error_సందేశం("trap: %s: invalid signal specification", argv[-1]);
				కొనసాగు;
			}

			విడుదల(G_traps[sig]);
			G_traps[sig] = xstrdup(new_cmd);

			debug_printf("trap: setting SIG%s (%i) to '%s'\n",
				get_signame(sig), sig, G_traps[sig]);

			/* There is no signal for 0 (EXIT) */
			ఒకవేళ (sig == 0)
				కొనసాగు;

			ఒకవేళ (new_cmd)
				handler = (new_cmd[0] ? రికార్డు_pending_signo : SIG_IGN);
			లేకపోతే
				/* We are removing trap handler */
				handler = pick_sighandler(sig);
			install_sighandler(sig, handler);
		}
		తిరిగి ret;
	}

	ఒకవేళ (!argv[1]) { /* no second arg */
		bb_simple_error_సందేశం("trap: invalid arguments");
		తిరిగి EXIT_FAILURE;
	}

	/* First arg is "-": reset all specified to default */
	/* First arg is "--": skip it, the rest is "handler SIGs..." */
	/* Everything else: set arg as signal handler
	 * (includes "" case, which ignores signal) */
	ఒకవేళ (argv[0][0] == '-') {
		ఒకవేళ (argv[0][1] == '\0') { /* "-" */
			/* new_cmd remains NULL: "reset these sigs" */
			వెళ్ళు reset_traps;
		}
		ఒకవేళ (argv[0][1] == '-' && argv[0][2] == '\0') { /* "--" */
			argv++;
		}
		/* else: "-something", no special meaning */
	}
	new_cmd = *argv;
 reset_traps:
	argv++;
	వెళ్ళు process_sig_list;
}
#అయితే_ముగింపు

#ఒకవేళ ENABLE_HUSH_పని
స్థిర నిర్మాణం pipe *parse_jobspec(స్థిరరాశి అక్షరం *str)
{
	నిర్మాణం pipe *pi;
	గుర్తులేని jobnum;

	ఒకవేళ (అక్షరశ్రేణి_చదువు(str, "%%%u", &jobnum) != 1) {
		ఒకవేళ (str[0] != '%'
		 || (str[1] != '%' && str[1] != '+' && str[1] != '\0')
		) {
			bb_error_సందేశం("bad argument '%s'", str);
			తిరిగి ఖాళీ;
		}
		/* It is "%%", "%+" or "%" - current job */
		jobnum = G.చివరి_jobid;
		ఒకవేళ (jobnum == 0) {
			bb_simple_error_సందేశం("no current job");
			తిరిగి ఖాళీ;
		}
	}
	కొరకు (pi = G.పని_list; pi; pi = pi->తదుపరి) {
		ఒకవేళ (pi->jobid == jobnum) {
			తిరిగి pi;
		}
	}
	bb_error_సందేశం("%u: no such job", jobnum);
	తిరిగి ఖాళీ;
}

స్థిర పూర్ణాంకం FAST_FUNC builtin_jobs(అక్షరం **argv వాడని_PARAM)
{
	నిర్మాణం pipe *పని;
	స్థిరరాశి అక్షరం *స్థితి_string;

	checkjobs(ఖాళీ, 0 /*(no pid to wait for)*/);
	కొరకు (పని = G.పని_list; పని; పని = పని->తదుపరి) {
		ఒకవేళ (పని->alive_cmds == పని->stopped_cmds)
			స్థితి_string = "Stopped";
		లేకపోతే
			స్థితి_string = "Running";

		రాయి(పని_స్థితి_FORMAT, పని->jobid, స్థితి_string, పని->cmdtext);
	}

	clean_పైకి_చివరి_dead_పని();

	తిరిగి EXIT_SUCCESS;
}

/* built-in 'fg' and 'bg' handler */
స్థిర పూర్ణాంకం FAST_FUNC builtin_fg_bg(అక్షరం **argv)
{
	పూర్ణాంకం i;
	నిర్మాణం pipe *pi;

	ఒకవేళ (!G_interactive_fd)
		తిరిగి EXIT_FAILURE;

	/* If they gave us no args, assume they want the last backgrounded task */
	ఒకవేళ (!argv[1]) {
		కొరకు (pi = G.పని_list; pi; pi = pi->తదుపరి) {
			ఒకవేళ (pi->jobid == G.చివరి_jobid) {
				వెళ్ళు దొరికింది;
			}
		}
		bb_error_సందేశం("%s: no current job", argv[0]);
		తిరిగి EXIT_FAILURE;
	}

	pi = parse_jobspec(argv[1]);
	ఒకవేళ (!pi)
		తిరిగి EXIT_FAILURE;
 దొరికింది:
	/* TODO: bash prints a string representation
	 * of job being foregrounded (like "sleep 1 | cat") */
	ఒకవేళ (argv[0][0] == 'f' && G_saved_tty_pgrp) {
		/* Put the job into the foreground. */
		tcsetpgrp(G_interactive_fd, pi->pgrp);
	}

	/* Restart the processes in the job */
	debug_printf_jobs("reviving %d procs, pgrp %d\n", pi->అంకె_cmds, pi->pgrp);
	కొరకు (i = 0; i < pi->అంకె_cmds; i++) {
		debug_printf_jobs("reviving pid %d\n", pi->cmds[i].pid);
	}
	pi->stopped_cmds = 0;

	i = kill(- pi->pgrp, SIGCONT);
	ఒకవేళ (i < 0) {
		ఒకవేళ (errno == ESRCH) {
			delete_ముగిసిన_పని(pi);
			తిరిగి EXIT_SUCCESS;
		}
		bb_simple_perror_సందేశం("kill (SIGCONT)");
	}

	ఒకవేళ (argv[0][0] == 'f') {
		remove_పని_from_పట్టిక(pi); /* FG job shouldn't be in job table */
		తిరిగి checkjobs_and_fg_shell(pi);
	}
	తిరిగి EXIT_SUCCESS;
}
#అయితే_ముగింపు

#ఒకవేళ ENABLE_HUSH_KILL
స్థిర పూర్ణాంకం FAST_FUNC builtin_kill(అక్షరం **argv)
{
	పూర్ణాంకం ret = 0;

# ఒకవేళ ENABLE_HUSH_పని
	ఒకవేళ (argv[1] && strcmp(argv[1], "-l") != 0) {
		పూర్ణాంకం i = 1;

		చేయి {
			నిర్మాణం pipe *pi;
			అక్షరం *dst;
			పూర్ణాంకం j, n;

			ఒకవేళ (argv[i][0] != '%')
				కొనసాగు;
			/*
			 * "kill %N" - job kill
			 * Converting to pgrp / pid kill
			 */
			pi = parse_jobspec(argv[i]);
			ఒకవేళ (!pi) {
				/* Eat bad jobspec */
				j = i;
				చేయి {
					j++;
					argv[j - 1] = argv[j];
				} అయితే (argv[j]);
				ret = 1;
				i--;
				కొనసాగు;
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
			n = G_interactive_fd ? 1 : pi->అంకె_cmds;
			dst = alloca(n * పరిమాణం(పూర్ణాంకం)*4);
			argv[i] = dst;
			ఒకవేళ (G_interactive_fd)
				dst += sprintf(dst, " -%u", (పూర్ణాంకం)pi->pgrp);
			లేకపోతే కొరకు (j = 0; j < n; j++) {
				నిర్మాణం command *cmd = &pi->cmds[j];
				/* Skip exited members of the job */
				ఒకవేళ (cmd->pid == 0)
					కొనసాగు;
				/*
				 * kill_main has matching code to expect
				 * leading space. Needed to not confuse
				 * negative pids with "kill -SIGNAL_NO" syntax
				 */
				dst += sprintf(dst, " %u", (పూర్ణాంకం)cmd->pid);
			}
			*dst = '\0';
		} అయితే (argv[++i]);
	}
# endif

	ఒకవేళ (argv[1] || ret == 0) {
		ret = నిరంతరం_applet_main(argv, kill_main);
	}
	/* else: ret = 1, "kill %bad_jobspec" case */
	తిరిగి ret;
}
#అయితే_ముగింపు

#ఒకవేళ ENABLE_HUSH_WAIT
/* http://www.opengroup.org/onlinepubs/9699919799/utilities/wait.html */
# ఒకవేళ !ENABLE_HUSH_పని
#  define wait_for_బిడ్డ_or_signal(pipe,pid) wait_for_బిడ్డ_or_signal(pid)
# endif
స్థిర పూర్ణాంకం wait_for_బిడ్డ_or_signal(నిర్మాణం pipe *waitfor_pipe, pid_t waitfor_pid)
{
	పూర్ణాంకం ret = 0;
	కొరకు (;;) {
		పూర్ణాంకం sig;
		sigset_t oldset;

		ఒకవేళ (!sigisemptyset(&G.pending_సమితి))
			వెళ్ళు check_sig;

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

		ఒకవేళ (!sigisemptyset(&G.pending_సమితి)) {
			/* Crap! we raced with some signal! */
			వెళ్ళు restore;
		}

		/*errno = 0; - checkjobs does this */
/* Can't pass waitfor_pipe into checkjobs(): it won't be interruptible */
		ret = checkjobs(ఖాళీ, waitfor_pid); /* waitpid(WNOHANG) inside */
		debug_printf_exec("checkjobs:%d\n", ret);
# ఒకవేళ ENABLE_HUSH_పని
		ఒకవేళ (waitfor_pipe) {
			పూర్ణాంకం rcode = పని_exited_or_stopped(waitfor_pipe);
			debug_printf_exec("job_exited_or_stopped:%d\n", rcode);
			ఒకవేళ (rcode >= 0) {
				ret = rcode;
				sigprocmask(SIG_SETMASK, &oldset, ఖాళీ);
				ఆపు;
			}
		}
# endif
		/* if ECHILD, there are no children (ret is -1 or 0) */
		/* if ret == 0, no children changed state */
		/* if ret != 0, it's exitcode+1 of exited waitfor_pid child */
		ఒకవేళ (errno == ECHILD || ret) {
			ret--;
			ఒకవేళ (ret < 0) /* if ECHILD, may need to fix "ret" */
				ret = 0;
# ఒకవేళ ENABLE_HUSH_BASH_COMPAT
			ఒకవేళ (waitfor_pid == -1 && errno == ECHILD) {
				/* exitcode of "wait -n" with no children is 127, not 0 */
				ret = 127;
			}
# endif
			sigprocmask(SIG_SETMASK, &oldset, ఖాళీ);
			ఆపు;
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
		sigprocmask(SIG_SETMASK, &oldset, ఖాళీ);
 check_sig:
		/* So, did we get a signal? */
		sig = check_and_నిరంతరం_traps();
		ఒకవేళ (sig /*&& sig != SIGCHLD - always true */) {
			/* Do this for any (non-ignored) signal, not only for ^C */
			ret = 128 | sig;
			ఆపు;
		}
		/* SIGCHLD, or no signal, or ignored one, such as SIGQUIT. Repeat */
	}
	తిరిగి ret;
}

స్థిర పూర్ణాంకం FAST_FUNC builtin_wait(అక్షరం **argv)
{
	పూర్ణాంకం ret;
	పూర్ణాంకం స్థితి_2;

	argv = skip_dash_dash(argv);
# ఒకవేళ ENABLE_HUSH_BASH_COMPAT
	ఒకవేళ (argv[0] && strcmp(argv[0], "-n") == 0) {
		/* wait -n */
		/* (bash accepts "wait -n PID" too and ignores PID) */
		G.dead_పని_exitcode = -1;
		తిరిగి wait_for_బిడ్డ_or_signal(ఖాళీ, -1 /*no job, wait for one job*/);
	}
# endif
	ఒకవేళ (argv[0] == ఖాళీ) {
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
		తిరిగి wait_for_బిడ్డ_or_signal(ఖాళీ, 0 /*no job and no pid to wait for*/);
	}

	చేయి {
		pid_t pid = bb_strtou(*argv, ఖాళీ, 10);
		ఒకవేళ (errno || pid <= 0) {
# ఒకవేళ ENABLE_HUSH_పని
			ఒకవేళ (argv[0][0] == '%') {
				నిర్మాణం pipe *wait_pipe;
				ret = 127; /* bash compat for bad jobspecs */
				wait_pipe = parse_jobspec(*argv);
				ఒకవేళ (wait_pipe) {
					ret = పని_exited_or_stopped(wait_pipe);
					ఒకవేళ (ret < 0) {
						ret = wait_for_బిడ్డ_or_signal(wait_pipe, 0);
					} లేకపోతే {
						/* waiting on "last dead job" removes it */
						clean_పైకి_చివరి_dead_పని();
					}
				}
				/* else: parse_jobspec() already emitted error msg */
				కొనసాగు;
			}
# endif
			/* mimic bash message */
			bb_error_సందేశం("wait: '%s': not a pid or valid job spec", *argv);
			ret = EXIT_FAILURE;
			కొనసాగు; /* bash checks all argv[] */
		}

		/* Do we have such child? */
		ret = waitpid(pid, &స్థితి_2, WNOHANG);
		ఒకవేళ (ret < 0) {
			/* No */
			ret = 127;
			ఒకవేళ (errno == ECHILD) {
				ఒకవేళ (pid == G.చివరి_bg_pid) {
					/* "wait $!" but last bg task has already exited. Try:
					 * (sleep 1; exit 3) & sleep 2; echo $?; wait $!; echo $?
					 * In bash it prints exitcode 0, then 3.
					 * In dash, it is 127.
					 */
					ret = G.చివరి_bg_pid_exitcode;
				} లేకపోతే {
					/* Example: "wait 1". mimic bash message */
					bb_error_సందేశం("wait: pid %u is not a child of this shell", (గుర్తులేని)pid);
				}
			} లేకపోతే {
				/* ??? */
				bb_perror_సందేశం("wait %s", *argv);
			}
			కొనసాగు; /* bash checks all argv[] */
		}
		ఒకవేళ (ret == 0) {
			/* Yes, and it still runs */
			ret = wait_for_బిడ్డ_or_signal(ఖాళీ, pid);
		} లేకపోతే {
			/* Yes, and it just exited */
			process_wait_ఫలితం(ఖాళీ, pid, స్థితి_2);
			ret = WEXITSTATUS(స్థితి_2);
			ఒకవేళ (WIFSIGNALED(స్థితి_2))
				ret = 128 | WTERMSIG(స్థితి_2);
		}
	} అయితే (*++argv);

	తిరిగి ret;
}
#అయితే_ముగింపు

#ఒకవేళ ENABLE_HUSH_LOOPS || ENABLE_HUSH_FUNCTIONS
స్థిర గుర్తులేని parse_numeric_argv1(అక్షరం **argv, గుర్తులేని def, గుర్తులేని def_min)
{
	ఒకవేళ (argv[1]) {
		def = bb_strtou(argv[1], ఖాళీ, 10);
		ఒకవేళ (errno || def < def_min || argv[2]) {
			bb_error_సందేశం("%s: bad arguments", argv[0]);
			def = UINT_MAX;
		}
	}
	తిరిగి def;
}
#అయితే_ముగింపు

#ఒకవేళ ENABLE_HUSH_LOOPS
స్థిర పూర్ణాంకం FAST_FUNC builtin_break(అక్షరం **argv)
{
	గుర్తులేని లోతు;
	ఒకవేళ (G.లోతు_యొక్క_loop == 0) {
		bb_error_సందేశం("%s: only meaningful in a loop", argv[0]);
		/* if we came from builtin_continue(), need to undo "= 1" */
		G.flag_break_continue = 0;
		తిరిగి EXIT_SUCCESS; /* bash compat */
	}
	G.flag_break_continue++; /* BC_BREAK = 1, or BC_CONTINUE = 2 */

	G.లోతు_break_continue = లోతు = parse_numeric_argv1(argv, 1, 1);
	ఒకవేళ (లోతు == UINT_MAX)
		G.flag_break_continue = BC_BREAK;
	ఒకవేళ (G.లోతు_యొక్క_loop < లోతు)
		G.లోతు_break_continue = G.లోతు_యొక్క_loop;

	తిరిగి EXIT_SUCCESS;
}

స్థిర పూర్ణాంకం FAST_FUNC builtin_continue(అక్షరం **argv)
{
	G.flag_break_continue = 1; /* BC_CONTINUE = 2 = 1+1 */
	తిరిగి builtin_break(argv);
}
#అయితే_ముగింపు

#ఒకవేళ ENABLE_HUSH_FUNCTIONS
స్థిర పూర్ణాంకం FAST_FUNC builtin_return(అక్షరం **argv)
{
	పూర్ణాంకం rc;

	ఒకవేళ (G_flag_return_లోపల_పురోగతి != -1) {
		bb_error_సందేశం("%s: not in a function or sourced script", argv[0]);
		తిరిగి EXIT_FAILURE; /* bash compat */
	}

	G_flag_return_లోపల_పురోగతి = 1;

	/* bash:
	 * out of range: wraps around at 256, does not error out
	 * non-numeric param:
	 * f() { false; return qwe; }; f; echo $?
	 * bash: return: qwe: numeric argument required  <== we do this
	 * 255  <== we also do this
	 */
	rc = parse_numeric_argv1(argv, G.చివరి_exitcode, 0);
# ఒకవేళ ENABLE_HUSH_TRAP
	ఒకవేళ (argv[1]) { /* "return ARG" inside a running trap sets $? */
		debug_printf_exec("G.return_exitcode=%d\n", rc);
		G.return_exitcode = rc;
	}
# endif
	తిరిగి rc;
}
#అయితే_ముగింపు

#ఒకవేళ ENABLE_HUSH_TIMES
స్థిర పూర్ణాంకం FAST_FUNC builtin_times(అక్షరం **argv వాడని_PARAM)
{
	స్థిర స్థిరరాశి uint8_t times_tbl[] ALIGN1 = {
		' ',  offsetof(నిర్మాణం tms, tms_utime),
		'\n', offsetof(నిర్మాణం tms, tms_stime),
		' ',  offsetof(నిర్మాణం tms, tms_cutime),
		'\n', offsetof(నిర్మాణం tms, tms_cstime),
		0
	};
	స్థిరరాశి uint8_t *p;
	గుర్తులేని clk_tck;
	నిర్మాణం tms buf;

	clk_tck = bb_clk_tck();

	times(&buf);
	p = times_tbl;
	చేయి {
		గుర్తులేని sec, frac;
		గుర్తులేని దీర్ఘ t;
		t = *(clock_t *)(((అక్షరం *) &buf) + p[1]);
		sec = t / clk_tck;
		frac = t % clk_tck;
		రాయి("%um%u.%03us%c",
			sec / 60, sec % 60,
			(frac * 1000) / clk_tck,
			p[0]);
		p += 2;
	} అయితే (*p);

	తిరిగి EXIT_SUCCESS;
}
#అయితే_ముగింపు

#ఒకవేళ ENABLE_HUSH_MEMLEAK
స్థిర పూర్ణాంకం FAST_FUNC builtin_memleak(అక్షరం **argv వాడని_PARAM)
{
	శూన్య *p;
	గుర్తులేని దీర్ఘ l;

# ifdef M_TRIM_THRESHOLD
	/* Optional. Reduces probability of false positives */
	malloc_trim(0);
# endif
	/* Crude attempt to find where "free memory" starts,
	 * sans fragmentation. */
	p = జ్ఞాపకం_ఇవ్వు(240);
	l = (గుర్తులేని దీర్ఘ)p;
	విడుదల(p);
	p = జ్ఞాపకం_ఇవ్వు(3400);
	ఒకవేళ (l < (గుర్తులేని దీర్ఘ)p) l = (గుర్తులేని దీర్ఘ)p;
	విడుదల(p);

# ఒకవేళ 0  /* debug */
	{
		నిర్మాణం mallinfo mi = mallinfo();
		రాయి("top alloc:0x%lx malloced:%d+%d=%d\n", l,
			mi.arena, mi.hblkhd, mi.arena + mi.hblkhd);
	}
# endif

	ఒకవేళ (!G.memleak_విలువ)
		G.memleak_విలువ = l;

	l -= G.memleak_విలువ;
	ఒకవేళ ((దీర్ఘ)l < 0)
		l = 0;
	l /= 1024;
	ఒకవేళ (l > 127)
		l = 127;

	/* Exitcode is "how many kilobytes we leaked since 1st call" */
	తిరిగి l;
}
#అయితే_ముగింపు
#అయితే_ముగింపు /* !__U_BOOT__ */
