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

#ifndef __U_BOOT__
#ଯଦି !(defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) \
	|| defined(__APPLE__) \
    )
# include <malloc.h>   /* for malloc_trim */
#endif
#ଅନ୍ତର୍ଭାଵ <glob.h>
/* #include <dmalloc.h> */
#ଯଦି ENABLE_HUSH_CASE
# include <fnmatch.h>
#endif
#ଅନ୍ତର୍ଭାଵ <sys/times.h>
#ଅନ୍ତର୍ଭାଵ <sys/utsname.h> /* for setting $HOSTNAME */

#ଅନ୍ତର୍ଭାଵ "busybox.h"  /* for APPLET_IS_NOFORK/NOEXEC */
#ଅନ୍ତର୍ଭାଵ "unicode.h"
#ଅନ୍ତର୍ଭାଵ "shell_common.h"
#ଅନ୍ତର୍ଭାଵ "math.h"
#ଅନ୍ତର୍ଭାଵ "match.h"
#ଯଦି ENABLE_HUSH_RANDOM_SUPPORT
# include "random.h"
#ଅନ୍ୟଥା
# define CLEAR_RANDOM_T(rnd) ((ଶୂନ୍ୟ)0)
#endif
#ifndef O_CLOEXEC
# define O_CLOEXEC 0
#endif
#ifndef F_DUPFD_CLOEXEC
# define F_DUPFD_CLOEXEC F_DUPFD
#endif

#ଯଦି ENABLE_FEATURE_SH_EMBEDDED_SCRIPTS && !ENABLE_SHELL_ASH
# include "embedded_scripts.h"
#ଅନ୍ୟଥା
# define ଅଙ୍କ_SCRIPTS 0
#endif
#endif /* !__U_BOOT__ */

/* So far, all bash compat is controlled by one config option */
/* Separate defines document which part of code implements what */
#define BASH_PATTERN_SUBST ENABLE_HUSH_BASH_COMPAT
#define BASH_SUBSTR        ENABLE_HUSH_BASH_COMPAT
#define BASH_SOURCE        ENABLE_HUSH_BASH_COMPAT
#define BASH_DOLLAR_SQUOTE ENABLE_HUSH_BASH_COMPAT
#define BASH_HOSTNAME_VAR  ENABLE_HUSH_BASH_COMPAT
#define BASH_EPOCH_VARS    ENABLE_HUSH_BASH_COMPAT
#define BASH_TEST2         (ENABLE_HUSH_BASH_COMPAT && ENABLE_HUSH_TEST)
#define BASH_READ_D        ENABLE_HUSH_BASH_COMPAT

/* Build knobs */
#define LEAK_HUNTING 0
#define BUILD_AS_NOMMU 0
/* Enable/disable sanity checks. Ok to enable in production,
 * only adds a bit of bloat. Set to >1 to get non-production level verbosity.
 * Keeping 1 for now even in released versions.
 */
#define HUSH_DEBUG 1
/* Slightly bigger (+200 bytes), but faster hush.
 * So far it only enables a trick with counting SIGCHLDs and forks,
 * which allows us to do fewer waitpid's.
 * (we can detect a case where neither forks were done nor SIGCHLDs happened
 * and therefore waitpid will return the same result as last time)
 */
#define ENABLE_HUSH_FAST 0
/* TODO: implement simplified code for users which do not need ${var%...} ops
 * So far ${var%...} ops are always enabled:
 */
#define ENABLE_HUSH_DOLLAR_OPS 1

#ଯଦି BUILD_AS_NOMMU
# undef BB_MMU
# undef USE_FOR_NOMMU
# undef USE_FOR_MMU
# define BB_MMU 0
# define USE_FOR_NOMMU(...) __VA_ARGS__
# define USE_FOR_MMU(...)
#endif

#ifndef __U_BOOT__
#ଅନ୍ତର୍ଭାଵ "NUM_APPLETS.h"
#ଯଦି ଅଙ୍କ_APPLETS == 1
/* STANDALONE does not make sense, and won't compile */
# undef ENABLE_FEATURE_SH_STANDALONE
# undef IF_FEATURE_SH_STANDALONE
# undef IF_NOT_FEATURE_SH_STANDALONE
# define ENABLE_FEATURE_SH_STANDALONE 0
# define IF_FEATURE_SH_STANDALONE(...)
# define IF_NOT_FEATURE_SH_STANDALONE(...) __VA_ARGS__
#endif
#endif /* __U_BOOT__ */

#ଯଦି !ENABLE_HUSH_INTERACTIVE
# undef ENABLE_FEATURE_EDITING
# define ENABLE_FEATURE_EDITING 0
# undef ENABLE_FEATURE_EDITING_FANCY_PROMPT
# define ENABLE_FEATURE_EDITING_FANCY_PROMPT 0
# undef ENABLE_FEATURE_EDITING_SAVE_ON_EXIT
# define ENABLE_FEATURE_EDITING_SAVE_ON_EXIT 0
#endif

/* Do we support ANY keywords? */
#ଯଦି ENABLE_HUSH_IF || ENABLE_HUSH_LOOPS || ENABLE_HUSH_CASE
# define HAS_KEYWORDS 1
# define IF_HAS_KEYWORDS(...) __VA_ARGS__
# define IF_HAS_NO_KEYWORDS(...)
#ଅନ୍ୟଥା
# define HAS_KEYWORDS 0
# define IF_HAS_KEYWORDS(...)
# define IF_HAS_NO_KEYWORDS(...) __VA_ARGS__
#endif

/* If you comment out one of these below, it will be #defined later
 * to perform debug printfs to stderr: */
#define debug_printf(...)         କର {} ଯେତେବେଳେ (0)
/* Finer-grained debug switches */
#define debug_printf_parse(...)   କର {} ଯେତେବେଳେ (0)
#define debug_printf_heredoc(...) କର {} ଯେତେବେଳେ (0)
#define debug_print_ବୃକ୍ଷ(a, b)    କର {} ଯେତେବେଳେ (0)
#define debug_printf_exec(...)    କର {} ଯେତେବେଳେ (0)
#define debug_printf_env(...)     କର {} ଯେତେବେଳେ (0)
#define debug_printf_jobs(...)    କର {} ଯେତେବେଳେ (0)
#define debug_printf_expand(...)  କର {} ଯେତେବେଳେ (0)
#define debug_printf_varexp(...)  କର {} ଯେତେବେଳେ (0)
#define debug_printf_glob(...)    କର {} ଯେତେବେଳେ (0)
#define debug_printf_redir(...)   କର {} ଯେତେବେଳେ (0)
#define debug_printf_list(...)    କର {} ଯେତେବେଳେ (0)
#define debug_printf_subst(...)   କର {} ଯେତେବେଳେ (0)
#define debug_printf_prompt(...)  କର {} ଯେତେବେଳେ (0)
#define debug_printf_clean(...)   କର {} ଯେତେବେଳେ (0)

#define ERR_PTR ((ଶୂନ୍ୟ*)(long)1)

#define JOB_STATUS_FORMAT    "[%u] %-22s %.40s\n"

#define _SPECIAL_VARS_STR     "_*@$!?#-"
#ifndef __U_BOOT__
#define SPECIAL_VARS_STR     ("_*@$!?#-" + 1)
#define NUMERIC_SPECVARS_STR ("_*@$!?#-" + 3)
#ଅନ୍ୟଥା /* __U_BOOT__ */
#define SPECIAL_VARS_STR     "*@$!?#-"
#define NUMERIC_SPECVARS_STR "$!?#-"
#endif /* __U_BOOT__ */
#ଯଦି BASH_PATTERN_SUBST
/* Support / and // replace ops */
/* Note that // is stored as \ in "encoded" string representation */
# define VAR_ENCODED_SUBST_OPS      "\\/%#:-=+?"
# define VAR_SUBST_OPS             ("\\/%#:-=+?" + 1)
# define MINUS_PLUS_EQUAL_QUESTION ("\\/%#:-=+?" + 5)
#ଅନ୍ୟଥା
# define VAR_ENCODED_SUBST_OPS      "%#:-=+?"
# define VAR_SUBST_OPS              "%#:-=+?"
# define MINUS_PLUS_EQUAL_QUESTION ("%#:-=+?" + 3)
#endif

#define SPECIAL_VAR_ପ୍ରତୀକ_STR "\3"
#define SPECIAL_VAR_ପ୍ରତୀକ       3
/* The "variable" with name "\1" emits string "\3". Testcase: "echo ^C" */
#define SPECIAL_VAR_QUOTED_SVS   1

ସଂରଚନା variable;

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର hush_version_str[] ALIGN1 = "HUSH_VERSION="BB_VER;

/* This supports saving pointers malloced in vfork child,
 * to be freed in the parent.
 */
#ଯଦି !BB_MMU
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା nommu_save_t {
	ସଂରଚନା variable *old_vars;
	ଅକ୍ଷର **argv;
	ଅକ୍ଷର **argv_from_re_execing;
} nommu_save_t;
#endif

ଗଣନା {
	RES_NONE  = 0,
#ଯଦି ENABLE_HUSH_IF
	RES_IF    ,
	RES_THEN  ,
	RES_ELIF  ,
	RES_ELSE  ,
	RES_FI    ,
#endif
#ଯଦି ENABLE_HUSH_LOOPS
	RES_FOR   ,
	RES_WHILE ,
	RES_UNTIL ,
	RES_DO    ,
	RES_DONE  ,
#endif
#ଯଦି ENABLE_HUSH_LOOPS || ENABLE_HUSH_CASE
	RES_IN    ,
#endif
#ଯଦି ENABLE_HUSH_CASE
	RES_CASE  ,
	/* three pseudo-keywords support contrived "case" syntax: */
	RES_CASE_IN,   /* "case ... IN", turns into RES_MATCH when IN is observed */
	RES_MATCH ,    /* "word)" */
	RES_CASE_BODY, /* "this command is inside CASE" */
	RES_ESAC  ,
#endif
	RES_XXXX  ,
	RES_SNTX
};

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା o_string {
	ଅକ୍ଷର *data;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା length; /* position where data is appended */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା maxlen;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା o_expflags;
	/* At least some part of the string was inside '' or "",
	 * possibly empty one: word"", wo''rd etc. */
	smallint has_quoted_part;
	smallint has_empty_slot;
	smallint ended_in_ifs;
} o_string;
ଗଣନା {
	EXP_FLAG_SINGLEWORD     = 0x80, /* must be 0x80 */
	EXP_FLAG_GLOB           = 0x2,
	/* Protect newly added chars against globbing
	 * by prepending \ to *, ?, [, \ */
	EXP_FLAG_ESC_GLOB_CHARS = 0x1,
};
/* Used for initialization: o_string foo = NULL_O_STRING; */
#define NULL_O_STRING { ଖାଲି }

#ifndef debug_printf_parse
ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ assignment_flag[] ALIGN_PTR = {
	"MAYBE_ASSIGNMENT",
	"DEFINITELY_ASSIGNMENT",
	"NOT_ASSIGNMENT",
	"WORD_IS_KEYWORD",
};
#endif

/* We almost can use standard FILE api, but we need an ability to move
 * its fd when redirects coincide with it. No api exists for that
 * (RFE for it at https://sourceware.org/bugzilla/show_bug.cgi?id=21902).
 * HFILE is our internal alternative. Only supports reading.
 * Since we now can, we incorporate linked list of all opened HFILEs
 * into the struct (used to be a separate mini-list).
 */
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା HFILE {
	ଅକ୍ଷର *cur;
	ଅକ୍ଷର *end;
	ସଂରଚନା HFILE *next_hfile;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd;
	ଅକ୍ଷର buf[1024];
} HFILE;

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା in_str {
	ଅଚଳ ଅକ୍ଷର *p;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା peek_buf[2];
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା last_char;
	HFILE *file;
} in_str;

#ifndef __U_BOOT__
/* The descrip member of this structure is only used to make
 * debugging output pretty */
ସ୍ଥିର ଅଚଳ ସଂରଚନା {
	int32_t mode;
	signed ଅକ୍ଷର default_fd;
	ଅକ୍ଷର descrip[3];
} redir_table[] ALIGN4 = {
	{ O_RDONLY,                  0, "<"  },
	{ O_CREAT|O_TRUNC|O_WRONLY,  1, ">"  },
	{ O_CREAT|O_APPEND|O_WRONLY, 1, ">>" },
	{ O_CREAT|O_RDWR,            1, "<>" },
	{ O_RDONLY,                  0, "<<" },
/* Should not be needed. Bogus default_fd helps in debugging */
/*	{ O_RDONLY,                 77, "<<" }, */
};

ସଂରଚନା redir_struct {
	ସଂରଚନା redir_struct *next;
	ଅକ୍ଷର *rd_filename;          /* filename */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rd_fd;                  /* fd to redirect */
	/* fd to redirect to, or -3 if rd_fd is to be closed (n>&-) */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rd_dup;
	smallint rd_type;           /* (enum redir_type) */
	/* note: for heredocs, rd_filename contains heredoc delimiter,
	 * and subsequently heredoc itself; and rd_dup is a bitmask:
	 * bit 0: do we need to trim leading tabs?
	 * bit 1: is heredoc quoted (<<'delim' syntax) ?
	 */
};
ପ୍ରକାରବ୍ୟାଖ୍ୟା ଗଣନା redir_type {
	REDIRECT_ନିବେଶ     = 0,
	REDIRECT_OVERWRITE = 1,
	REDIRECT_APPEND    = 2,
	REDIRECT_IO        = 3,
	REDIRECT_HEREDOC   = 4,
	REDIRECT_HEREDOC2  = 5, /* REDIRECT_HEREDOC after heredoc is loaded */

	REDIRFD_CLOSE      = -3,
	REDIRFD_SYNTAX_ERR = -2,
	REDIRFD_TO_FILE    = -1,
	/* otherwise, rd_fd is redirected to rd_dup */

	HEREDOC_SKIPTABS = 1,
	HEREDOC_QUOTED   = 2,
} redir_type;

#endif /* !__U_BOOT__ */

ସଂରଚନା command {
#ifndef __U_BOOT__
	pid_t pid;                  /* 0 if exited */
#endif /* !__U_BOOT__ */
	unsigned assignment_cnt;    /* how many argv[i] are assignments? */
#ଯଦି ENABLE_HUSH_LINENO_VAR
	unsigned lineno;
#endif
	smallint cmd_type;          /* CMD_xxx */
#define CMD_NORMAL   0
#define CMD_SUBSHELL 1
#ଯଦି BASH_TEST2
/* used for "[[ EXPR ]]" */
# define CMD_TEST2_SINGLEWORD_NOGLOB 2
#endif
#ଯଦି BASH_TEST2 || ENABLE_HUSH_LOCAL || ENABLE_HUSH_EXPORT || ENABLE_HUSH_READONLY
/* used to prevent word splitting and globbing in "export v=t*" */
# define CMD_SINGLEWORD_NOGLOB 3
#endif
#ଯଦି ENABLE_HUSH_FUNCTIONS
# define CMD_FUNCDEF 4
#endif

	smalluint cmd_exitcode;
	/* if non-NULL, this "command" is { list }, ( list ), or a compound statement */
	ସଂରଚନା pipe *group;
#ଯଦି !BB_MMU
	ଅକ୍ଷର *group_as_string;
#endif
#ଯଦି ENABLE_HUSH_FUNCTIONS
	ସଂରଚନା function *child_func;
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
#endif
#ifdef __U_BOOT__
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc; /* number of program arguments */
#endif
	ଅକ୍ଷର **argv;                /* command name and arguments */
/* argv vector may contain variable references (^Cvar^C, ^C0^C etc)
 * and on execution these are substituted with their values.
 * Substitution can make _several_ words out of one argv[n]!
 * Example: argv[0]=='.^C*^C.' here: echo .$*.
 * References of the form ^C`cmd arg^C are `cmd arg` substitutions.
 */
#ifndef __U_BOOT__
	ସଂରଚନା redir_struct *redirects; /* I/O redirections */
#endif /* !__U_BOOT__ */
};
/* Is there anything in this command at all? */
#ifndef __U_BOOT__
#define IS_NULL_CMD(cmd) \
	(!(cmd)->group && !(cmd)->argv && !(cmd)->redirects)

#ଅନ୍ୟଥା /* __U_BOOT__ */
#define IS_NULL_CMD(cmd) \
	(!(cmd)->group && !(cmd)->argv)
#endif /* __U_BOOT__ */
ସଂରଚନା pipe {
	ସଂରଚନା pipe *next;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଅଙ୍କ_cmds;               /* total number of commands in pipe */
#ifndef __U_BOOT__
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା alive_cmds;             /* number of commands running (not exited) */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା stopped_cmds;           /* number of commands alive, but stopped */
#ଯଦି ENABLE_HUSH_JOB
	unsigned jobid;             /* job number */
	pid_t pgrp;                 /* process group ID for the job */
	ଅକ୍ଷର *cmdtext;              /* name of job */
#endif
#endif /* !__U_BOOT__ */
	ସଂରଚନା command *cmds;       /* array of commands in pipe */
	smallint followup;          /* PIPE_BG, PIPE_SEQ, PIPE_OR, PIPE_AND */
	IF_HAS_KEYWORDS(smallint pi_inverted;) /* "! cmd | cmd" */
	IF_HAS_KEYWORDS(smallint res_word;) /* needed for if, for, while, until... */
};
ପ୍ରକାରବ୍ୟାଖ୍ୟା ଗଣନା pipe_style {
	PIPE_SEQ = 0,
	PIPE_AND = 1,
	PIPE_OR  = 2,
	PIPE_BG  = 3,
} pipe_style;
/* Is there anything in this pipe at all? */
#define IS_NULL_PIPE(pi) \
	((pi)->ଅଙ୍କ_cmds == 0 IF_HAS_KEYWORDS( && (pi)->res_word == RES_NONE))

/* This holds pointers to the various results of parsing */
ସଂରଚନା parse_context {
	/* linked list of pipes */
	ସଂରଚନା pipe *list_head;
	/* last pipe (being constructed right now) */
	ସଂରଚନା pipe *pipe;
	/* last command in pipe (being constructed right now) */
	ସଂରଚନା command *command;
#ifndef __U_BOOT__
	/* last redirect in command->redirects list */
	ସଂରଚନା redir_struct *pending_redirect;
#endif /* !__U_BOOT__ */
	o_string word;
#ଯଦି !BB_MMU
	o_string as_string;
#endif
	smallint is_assignment; /* 0:maybe, 1:yes, 2:no, 3:keyword */
#ଯଦି HAS_KEYWORDS
	smallint ctx_res_w;
	smallint ctx_inverted; /* "! cmd | cmd" */
#ଯଦି ENABLE_HUSH_CASE
	smallint ctx_dsemicolon; /* ";;" seen */
#endif
	/* bitmask of FLAG_xxx, for figuring out valid reserved words */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା old_flag;
	/* group we are enclosed in:
	 * example: "if pipe1; pipe2; then pipe3; fi"
	 * when we see "if" or "then", we malloc and copy current context,
	 * and make ->stack point to it. then we parse pipeN.
	 * when closing "then" / fi" / whatever is found,
	 * we move list_head into ->stack->command->group,
	 * copy ->stack into current context, and delete ->stack.
	 * (parsing of { list } and ( list ) doesn't use this method)
	 */
	ସଂରଚନା parse_context *ଗଦା;
#endif
};
ଗଣନା {
	MAYBE_ASSIGNMENT      = 0,
	DEFINITELY_ASSIGNMENT = 1,
	NOT_ASSIGNMENT        = 2,
	/* Not an assignment, but next word may be: "if v=xyz cmd;" */
	WORD_IS_KEYWORD       = 3,
};

#ifndef __U_BOOT__
/* On program start, environ points to initial environment.
 * putenv adds new pointers into it, unsetenv removes them.
 * Neither of these (de)allocates the strings.
 * setenv allocates new strings in malloc space and does putenv,
 * and thus setenv is unusable (leaky) for shell's purposes */
#define setenv(...) setenv_is_leaky_dont_use()
#endif /* !__U_BOOT__ */
ସଂରଚନା variable {
	ସଂରଚନା variable *next;
	ଅକ୍ଷର *varstr;        /* points to "name=" portion */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା max_len;         /* if > 0, name is part of initial env; else name is malloced */
#ifndef __U_BOOT__
	uint16_t var_nest_level;
	smallint flg_export; /* putenv should be done on this var */
	smallint flg_read_only;
#endif /* !__U_BOOT__ */
};

ଗଣନା {
	BC_BREAK = 1,
	BC_CONTINUE = 2,
};

#ଯଦି ENABLE_HUSH_FUNCTIONS
ସଂରଚନା function {
	ସଂରଚନା function *next;
	ଅକ୍ଷର *name;
	ସଂରଚନା command *parent_cmd;
	ସଂରଚନା pipe *body;
# ଯଦି !BB_MMU
	ଅକ୍ଷର *body_as_string;
# endif
};
#endif

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
ସ୍ଥିର ଅଚଳ ଅକ୍ଷର o_opt_strings[] ALIGN1 =
	"pipefail\0"
	"noexec\0"
	"errexit\0"
#ଯଦି ENABLE_HUSH_MODE_X
	"xtrace\0"
#endif
	;
ଗଣନା {
	OPT_O_PIPEFAIL,
	OPT_O_NOEXEC,
	OPT_O_ERREXIT,
#ଯଦି ENABLE_HUSH_MODE_X
	OPT_O_XTRACE,
#endif
	ଅଙ୍କ_OPT_O
};

/* "Globals" within this file */
/* Sorted roughly by size (smaller offsets == smaller code) */
ସଂରଚନା globals {
#ifndef __U_BOOT__
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
#ଯଦି ENABLE_HUSH_INTERACTIVE
	/* 'interactive_fd' is a fd# open to ctty, if we have one
	 * _AND_ if we decided to act interactively */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା interactive_fd;
	IF_NOT_FEATURE_EDITING_FANCY_PROMPT(ଅକ୍ଷର *PS1;)
# define G_interactive_fd (G.interactive_fd)
#ଅନ୍ୟଥା
# define G_interactive_fd 0
#endif
#ଅନ୍ୟଥା /* __U_BOOT__ */
# define G_interactive_fd 0
#endif /* __U_BOOT__ */
#ifndef __U_BOOT__
#ଯଦି ENABLE_FEATURE_EDITING
	ଧାଡ଼ି_ନିବେଶ_t *ଧାଡ଼ି_ନିବେଶ_state;
#endif
	pid_t ମୂଳ_pid;
	pid_t ମୂଳ_ppid;
	pid_t last_bg_pid;
#ଯଦି ENABLE_HUSH_RANDOM_SUPPORT
	random_t random_gen;
#endif
#ଯଦି ENABLE_HUSH_JOB
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା run_list_level;
	unsigned last_jobid;
	pid_t saved_tty_pgrp;
	ସଂରଚନା pipe *job_list;
# define G_saved_tty_pgrp (G.saved_tty_pgrp)
#ଅନ୍ୟଥା
# define G_saved_tty_pgrp 0
#endif
#endif /* !__U_BOOT__ */
	/* How deeply are we in context where "set -e" is ignored */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା errexit_depth;
#ifndef __U_BOOT__
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

	ଅକ୍ଷର o_opt[ଅଙ୍କ_OPT_O];
#ଯଦି ENABLE_HUSH_MODE_X
# define G_x_mode (G.o_opt[OPT_O_XTRACE])
#ଅନ୍ୟଥା
# define G_x_mode 0
#endif
	ଅକ୍ଷର opt_s;
	ଅକ୍ଷର opt_c;
#endif /* !__U_BOOT__ */
#ଯଦି ENABLE_HUSH_INTERACTIVE
	smallint promptmode; /* 0: PS1, 1: PS2 */
#endif
	/* set by signal handler if SIGINT is received _and_ its trap is not set */
	smallint flag_SIGINT;
#ifndef __U_BOOT__
#ଯଦି ENABLE_HUSH_LOOPS
	smallint flag_break_continue;
#endif
#endif /* !__U_BOOT__ */
#ଯଦି ENABLE_HUSH_FUNCTIONS
	/* 0: outside of a function (or sourced file)
	 * -1: inside of a function, ok to use return builtin
	 * 1: return is invoked, skip all till end of func
	 */
	smallint flag_return_in_progress;
# define G_flag_return_in_progress (G.flag_return_in_progress)
#ଅନ୍ୟଥା
# define G_flag_return_in_progress 0
#endif
	smallint exiting; /* used to prevent EXIT trap recursion */
	/* These support $? */
	smalluint last_exitcode;
	smalluint expand_exitcode;
	smalluint last_bg_pid_exitcode;
#ifndef __U_BOOT__
#ଯଦି ENABLE_HUSH_SET
	/* are global_argv and global_argv[1..n] malloced? (note: not [0]) */
	smalluint global_args_malloced;
# define G_global_args_malloced (G.global_args_malloced)
#ଅନ୍ୟଥା
# define G_global_args_malloced 0
#endif
#ଯଦି ENABLE_HUSH_BASH_COMPAT
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା dead_job_exitcode; /* for "wait -n" */
#endif
#endif /* !__U_BOOT__ */
	/* how many non-NULL argv's we have. NB: $# + 1 */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା global_argc;
	ଅକ୍ଷର **global_argv;
#ଯଦି !BB_MMU
	ଅକ୍ଷର *argv0_for_re_execing;
#endif
#ଯଦି ENABLE_HUSH_LOOPS
#ifndef __U_BOOT__
	unsigned depth_break_continue;
#endif /* !__U_BOOT__ */
	unsigned depth_of_loop;
#endif
#ifndef __U_BOOT__
#ଯଦି ENABLE_HUSH_GETOPTS
	unsigned getopt_ଗଣନା;
#endif
#endif /* !__U_BOOT__ */
	ଅଚଳ ଅକ୍ଷର *ifs;
#ifdef __U_BOOT__
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା flag_repeat;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା do_repeat;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା run_command_flags;
#endif /* __U_BOOT__ */
	ଅକ୍ଷର *ifs_whitespace; /* = G.ifs or malloced */
#ifndef __U_BOOT__
	ଅଚଳ ଅକ୍ଷର *cwd;
#endif /* !__U_BOOT__ */
	ସଂରଚନା variable *ଶୀର୍ଷ_var;
	ଅକ୍ଷର **expanded_assignments;
	ସଂରଚନା variable **shadowed_vars_pp;
	unsigned var_nest_level;
#ifndef __U_BOOT__
#ଯଦି ENABLE_HUSH_FUNCTIONS
# ଯଦି ENABLE_HUSH_LOCAL
	unsigned func_nest_level; /* solely to prevent "local v" in non-functions */
# endif
	ସଂରଚନା function *ଶୀର୍ଷ_func;
#endif
	/* Signal and trap handling */
#ଯଦି ENABLE_HUSH_FAST
	unsigned ଗଣନା_SIGCHLD;
	unsigned handled_SIGCHLD;
	smallint we_have_children;
#endif
#ଯଦି ENABLE_HUSH_LINENO_VAR
	unsigned parse_lineno;
	unsigned execute_lineno;
#endif
	HFILE *HFILE_list;
	HFILE *HFILE_stdin;
	/* Which signals have non-DFL handler (even with no traps set)?
	 * Set at the start to:
	 * (SIGQUIT + maybe SPECIAL_INTERACTIVE_SIGS + maybe SPECIAL_JOBSTOP_SIGS)
	 * SPECIAL_INTERACTIVE_SIGS are cleared after fork.
	 * The rest is cleared right before execv syscalls.
	 * Other than these two times, never modified.
	 */
	unsigned special_sig_mask;
#ଯଦି ENABLE_HUSH_JOB
	unsigned fatal_sig_mask;
# define G_fatal_sig_mask (G.fatal_sig_mask)
#ଅନ୍ୟଥା
# define G_fatal_sig_mask 0
#endif
#ଯଦି ENABLE_HUSH_TRAP
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା pre_trap_exitcode;
# ଯଦି ENABLE_HUSH_FUNCTIONS
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା return_exitcode;
# endif
	ଅକ୍ଷର **traps; /* char *traps[NSIG] */
# define G_traps G.traps
#ଅନ୍ୟଥା
# define G_traps ((ଅକ୍ଷର**)ଖାଲି)
#endif
	sigset_t pending_set;
#ଯଦି ENABLE_HUSH_MEMLEAK
	unsigned long memleak_ମୂଲ୍ୟ;
#endif
#ଯଦି ENABLE_HUSH_MODE_X
	unsigned x_mode_depth;
	/* "set -x" output should not be redirectable with subsequent 2>FILE.
	 * We dup fd#2 to x_mode_fd when "set -x" is executed, and use it
	 * for all subsequent output.
	 */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା x_mode_fd;
	o_string x_mode_buf;
#endif
#endif /* !__U_BOOT__ */
#ଯଦି HUSH_DEBUG >= 2
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା debug_indent;
#endif
#ifndef __U_BOOT__
	ସଂରଚନା sigaction sa;
	ଅକ୍ଷର optstring_buf[ଆକାର("eixcs")];
#ଯଦି BASH_EPOCH_VARS
	ଅକ୍ଷର epoch_buf[ଆକାର("%llu.nnnnnn") + ଆକାର(long long)*3];
#endif
#ଯଦି ENABLE_FEATURE_EDITING
	ଅକ୍ଷର user_ନିବେଶ_buf[CONFIG_FEATURE_EDITING_MAX_LEN];
#endif
#endif /* !__U_BOOT__ */
};
#ifdef __U_BOOT__
ସଂରଚନା globals *ptr_to_globals;
#endif /* __U_BOOT__ */
#define G (*ptr_to_globals)
/* Not #defining name to G.name - this quickly gets unwieldy
 * (too many defines). Also, I actually prefer to see when a variable
 * is global, thus "G." prefix is a useful hint */
#ifdef __U_BOOT__
#define SET_PTR_TO_GLOBALS(x) କର { \
	(*(ସଂରଚନା globals**)&ptr_to_globals) = (ଶୂନ୍ୟ*)(x); \
	barrier(); \
} ଯେତେବେଳେ (0)
#define INIT_G() କର { \
	SET_PTR_TO_GLOBALS(xzalloc(ଆକାର(G))); \
	G.promptmode = 1; \
} ଯେତେବେଳେ (0)
#ଅନ୍ୟଥା /* !__U_BOOT__ */
#define INIT_G() କର { \
	SET_PTR_TO_GLOBALS(xzalloc(ଆକାର(G))); \
	/* memset(&G.sa, 0, sizeof(G.sa)); */  \
	sigfillset(&G.sa.sa_mask); \
	G.sa.sa_flags = SA_RESTART; \
} ଯେତେବେଳେ (0)
#endif /* !__U_BOOT__ */

#ifndef __U_BOOT__
/* Function prototypes for builtins */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା builtin_cd(ଅକ୍ଷର **argv) FAST_FUNC;
#ଯଦି ENABLE_HUSH_ECHO
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା builtin_echo(ଅକ୍ଷର **argv) FAST_FUNC;
#endif
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା builtin_eval(ଅକ୍ଷର **argv) FAST_FUNC;
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା builtin_exec(ଅକ୍ଷର **argv) FAST_FUNC;
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା builtin_exit(ଅକ୍ଷର **argv) FAST_FUNC;
#ଯଦି ENABLE_HUSH_EXPORT
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା builtin_export(ଅକ୍ଷର **argv) FAST_FUNC;
#endif
#ଯଦି ENABLE_HUSH_READONLY
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା builtin_readonly(ଅକ୍ଷର **argv) FAST_FUNC;
#endif
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା builtin_false(ଅକ୍ଷର **argv) FAST_FUNC;
#ଯଦି ENABLE_HUSH_JOB
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା builtin_fg_bg(ଅକ୍ଷର **argv) FAST_FUNC;
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା builtin_jobs(ଅକ୍ଷର **argv) FAST_FUNC;
#endif
#ଯଦି ENABLE_HUSH_GETOPTS
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା builtin_getopts(ଅକ୍ଷର **argv) FAST_FUNC;
#endif
#ଯଦି ENABLE_HUSH_HELP
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା builtin_help(ଅକ୍ଷର **argv) FAST_FUNC;
#endif
#ଯଦି MAX_HISTORY && ENABLE_FEATURE_EDITING
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା builtin_history(ଅକ୍ଷର **argv) FAST_FUNC;
#endif
#ଯଦି ENABLE_HUSH_LOCAL
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା builtin_local(ଅକ୍ଷର **argv) FAST_FUNC;
#endif
#ଯଦି ENABLE_HUSH_MEMLEAK
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା builtin_memleak(ଅକ୍ଷର **argv) FAST_FUNC;
#endif
#ଯଦି ENABLE_HUSH_PRINTF
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା builtin_printf(ଅକ୍ଷର **argv) FAST_FUNC;
#endif
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା builtin_pwd(ଅକ୍ଷର **argv) FAST_FUNC;
#ଯଦି ENABLE_HUSH_READ
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା builtin_read(ଅକ୍ଷର **argv) FAST_FUNC;
#endif
#ଯଦି ENABLE_HUSH_SET
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା builtin_set(ଅକ୍ଷର **argv) FAST_FUNC;
#endif
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା builtin_shift(ଅକ୍ଷର **argv) FAST_FUNC;
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା builtin_source(ଅକ୍ଷର **argv) FAST_FUNC;
#ଯଦି ENABLE_HUSH_TEST || BASH_TEST2
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା builtin_test(ଅକ୍ଷର **argv) FAST_FUNC;
#endif
#ଯଦି ENABLE_HUSH_TRAP
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା builtin_trap(ଅକ୍ଷର **argv) FAST_FUNC;
#endif
#ଯଦି ENABLE_HUSH_TYPE
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା builtin_type(ଅକ୍ଷର **argv) FAST_FUNC;
#endif
#ଯଦି ENABLE_HUSH_TIMES
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା builtin_times(ଅକ୍ଷର **argv) FAST_FUNC;
#endif
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା builtin_true(ଅକ୍ଷର **argv) FAST_FUNC;
#ଯଦି ENABLE_HUSH_UMASK
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା builtin_umask(ଅକ୍ଷର **argv) FAST_FUNC;
#endif
#ଯଦି ENABLE_HUSH_UNSET
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା builtin_unset(ଅକ୍ଷର **argv) FAST_FUNC;
#endif
#ଯଦି ENABLE_HUSH_KILL
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା builtin_kill(ଅକ୍ଷର **argv) FAST_FUNC;
#endif
#ଯଦି ENABLE_HUSH_WAIT
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା builtin_wait(ଅକ୍ଷର **argv) FAST_FUNC;
#endif
#ଯଦି ENABLE_HUSH_LOOPS
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା builtin_break(ଅକ୍ଷର **argv) FAST_FUNC;
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା builtin_continue(ଅକ୍ଷର **argv) FAST_FUNC;
#endif
#ଯଦି ENABLE_HUSH_FUNCTIONS
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା builtin_return(ଅକ୍ଷର **argv) FAST_FUNC;
#endif

/* Table of built-in functions.  They can be forked or not, depending on
 * context: within pipes, they fork.  As simple commands, they do not.
 * When used in non-forking context, they can change global variables
 * in the parent shell process.  If forked, of course they cannot.
 * For example, 'unset foo | whatever' will parse and run, but foo will
 * still be set at the end. */
ସଂରଚନା built_in_command {
	ଅଚଳ ଅକ୍ଷର *b_cmd;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*b_function)(ଅକ୍ଷର **argv) FAST_FUNC;
#ଯଦି ENABLE_HUSH_HELP
	ଅଚଳ ଅକ୍ଷର *b_descr;
# define BLTIN(cmd, func, help) { cmd, func, help }
#ଅନ୍ୟଥା
# define BLTIN(cmd, func, help) { cmd, func }
#endif
};

ସ୍ଥିର ଅଚଳ ସଂରଚନା built_in_command bltins1[] ALIGN_PTR = {
	BLTIN("."        , builtin_source  , "Run commands in file"),
	BLTIN(":"        , builtin_true    , ଖାଲି),
#ଯଦି ENABLE_HUSH_JOB
	BLTIN("bg"       , builtin_fg_bg   , "Resume job in background"),
#endif
#ଯଦି ENABLE_HUSH_LOOPS
	BLTIN("break"    , builtin_break   , "Exit loop"),
#endif
	BLTIN("cd"       , builtin_cd      , "Change directory"),
#ଯଦି ENABLE_HUSH_LOOPS
	BLTIN("continue" , builtin_continue, "Start new loop iteration"),
#endif
	BLTIN("eval"     , builtin_eval    , "Construct and run shell command"),
	BLTIN("exec"     , builtin_exec    , "Execute command, don't return to shell"),
	BLTIN("exit"     , builtin_exit    , ଖାଲି),
#ଯଦି ENABLE_HUSH_EXPORT
	BLTIN("export"   , builtin_export  , "Set environment variables"),
#endif
	BLTIN("false"    , builtin_false   , ଖାଲି),
#ଯଦି ENABLE_HUSH_JOB
	BLTIN("fg"       , builtin_fg_bg   , "Bring job to foreground"),
#endif
#ଯଦି ENABLE_HUSH_GETOPTS
	BLTIN("getopts"  , builtin_getopts , ଖାଲି),
#endif
#ଯଦି ENABLE_HUSH_HELP
	BLTIN("help"     , builtin_help    , ଖାଲି),
#endif
#ଯଦି MAX_HISTORY && ENABLE_FEATURE_EDITING
	BLTIN("history"  , builtin_history , "Show history"),
#endif
#ଯଦି ENABLE_HUSH_JOB
	BLTIN("jobs"     , builtin_jobs    , "List jobs"),
#endif
#ଯଦି ENABLE_HUSH_KILL
	BLTIN("kill"     , builtin_kill    , "Send signals to processes"),
#endif
#ଯଦି ENABLE_HUSH_LOCAL
	BLTIN("local"    , builtin_local   , "Set local variables"),
#endif
#ଯଦି ENABLE_HUSH_MEMLEAK
	BLTIN("memleak"  , builtin_memleak , ଖାଲି),
#endif
#ଯଦି ENABLE_HUSH_READ
	BLTIN("read"     , builtin_read    , "Input into variable"),
#endif
#ଯଦି ENABLE_HUSH_READONLY
	BLTIN("readonly" , builtin_readonly, "Make variables read-only"),
#endif
#ଯଦି ENABLE_HUSH_FUNCTIONS
	BLTIN("return"   , builtin_return  , "Return from function"),
#endif
#ଯଦି ENABLE_HUSH_SET
	BLTIN("set"      , builtin_set     , "Set positional parameters"),
#endif
	BLTIN("shift"    , builtin_shift   , "Shift positional parameters"),
#ଯଦି BASH_SOURCE
	BLTIN("source"   , builtin_source  , ଖାଲି),
#endif
#ଯଦି ENABLE_HUSH_TIMES
	BLTIN("times"    , builtin_times   , ଖାଲି),
#endif
#ଯଦି ENABLE_HUSH_TRAP
	BLTIN("trap"     , builtin_trap    , "Trap signals"),
#endif
	BLTIN("true"     , builtin_true    , ଖାଲି),
#ଯଦି ENABLE_HUSH_TYPE
	BLTIN("type"     , builtin_type    , "Show command type"),
#endif
#ଯଦି ENABLE_HUSH_ULIMIT
	BLTIN("ulimit"   , shell_builtin_ulimit, "Control resource limits"),
#endif
#ଯଦି ENABLE_HUSH_UMASK
	BLTIN("umask"    , builtin_umask   , "Set file creation mask"),
#endif
#ଯଦି ENABLE_HUSH_UNSET
	BLTIN("unset"    , builtin_unset   , "Unset variables"),
#endif
#ଯଦି ENABLE_HUSH_WAIT
	BLTIN("wait"     , builtin_wait    , "Wait for process to finish"),
#endif
};
/* These builtins won't be used if we are on NOMMU and need to re-exec
 * (it's cheaper to run an external program in this case):
 */
ସ୍ଥିର ଅଚଳ ସଂରଚନା built_in_command bltins2[] ALIGN_PTR = {
#ଯଦି ENABLE_HUSH_TEST
	BLTIN("["        , builtin_test    , ଖାଲି),
#endif
#ଯଦି BASH_TEST2
	BLTIN("[["       , builtin_test    , ଖାଲି),
#endif
#ଯଦି ENABLE_HUSH_ECHO
	BLTIN("echo"     , builtin_echo    , ଖାଲି),
#endif
#ଯଦି ENABLE_HUSH_PRINTF
	BLTIN("printf"   , builtin_printf  , ଖାଲି),
#endif
	BLTIN("pwd"      , builtin_pwd     , ଖାଲି),
#ଯଦି ENABLE_HUSH_TEST
	BLTIN("test"     , builtin_test    , ଖାଲି),
#endif
};

#endif /* !__U_BOOT__ */

/* Debug printouts.
 */
#ଯଦି HUSH_DEBUG >= 2
/* prevent disasters with G.debug_indent < 0 */
# define indent() fdprintf(2, "%*s", (G.debug_indent * 2) & 0xff, "")
# define debug_enter() (G.debug_indent++)
# define debug_leave() (G.debug_indent--)
#ଅନ୍ୟଥା
# define indent()      ((ଶୂନ୍ୟ)0)
# define debug_enter() ((ଶୂନ୍ୟ)0)
# define debug_leave() ((ଶୂନ୍ୟ)0)
#endif

#ifndef debug_printf
# define debug_printf(...) (indent(), fdprintf(2, __VA_ARGS__))
#endif

#ifndef debug_printf_parse
# define debug_printf_parse(...) (indent(), fdprintf(2, __VA_ARGS__))
#endif

#ifndef debug_printf_heredoc
# define debug_printf_heredoc(...) (indent(), fdprintf(2, __VA_ARGS__))
#endif

#ifndef debug_printf_exec
#define debug_printf_exec(...) (indent(), fdprintf(2, __VA_ARGS__))
#endif

#ifndef debug_printf_env
# define debug_printf_env(...) (indent(), fdprintf(2, __VA_ARGS__))
#endif

#ifndef debug_printf_jobs
# define debug_printf_jobs(...) (indent(), fdprintf(2, __VA_ARGS__))
# define DEBUG_JOBS 1
#ଅନ୍ୟଥା
# define DEBUG_JOBS 0
#endif

#ifndef debug_printf_expand
# define debug_printf_expand(...) (indent(), fdprintf(2, __VA_ARGS__))
# define DEBUG_EXPAND 1
#ଅନ୍ୟଥା
# define DEBUG_EXPAND 0
#endif

#ifndef debug_printf_varexp
# define debug_printf_varexp(...) (indent(), fdprintf(2, __VA_ARGS__))
#endif

#ifndef debug_printf_glob
# define debug_printf_glob(...) (indent(), fdprintf(2, __VA_ARGS__))
# define DEBUG_GLOB 1
#ଅନ୍ୟଥା
# define DEBUG_GLOB 0
#endif

#ifndef debug_printf_redir
# define debug_printf_redir(...) (indent(), fdprintf(2, __VA_ARGS__))
#endif

#ifndef debug_printf_list
# define debug_printf_list(...) (indent(), fdprintf(2, __VA_ARGS__))
#endif

#ifndef debug_printf_subst
# define debug_printf_subst(...) (indent(), fdprintf(2, __VA_ARGS__))
#endif

#ifndef debug_printf_prompt
# define debug_printf_prompt(...) (indent(), fdprintf(2, __VA_ARGS__))
#endif

#ifndef debug_printf_clean
# define debug_printf_clean(...) (indent(), fdprintf(2, __VA_ARGS__))
# define DEBUG_CLEAN 1
#ଅନ୍ୟଥା
# define DEBUG_CLEAN 0
#endif

#ଯଦି DEBUG_EXPAND
ସ୍ଥିର ଶୂନ୍ୟ debug_print_strings(ଅଚଳ ଅକ୍ଷର *prefix, ଅକ୍ଷର **vv)
{
	indent();
	fdprintf(2, "%s:\n", prefix);
	ଯେତେବେଳେ (*vv)
		fdprintf(2, " '%s'\n", *vv++);
}
#ଅନ୍ୟଥା
# define debug_print_strings(prefix, vv) ((ଶୂନ୍ୟ)0)
#endif

/* Leak hunting. Use hush_leaktool.sh for post-processing.
 */
#ଯଦି LEAK_HUNTING
ସ୍ଥିର ଶୂନ୍ୟ *xxmalloc(ପୂର୍ଣ୍ଣସଂଖ୍ୟା lineno, size_t size)
{
	ଶୂନ୍ୟ *ptr = xmalloc((size + 0xff) & ~0xff);
	fdprintf(2, "line %d: malloc %p\n", lineno, ptr);
	ଫେରନ୍ତୁ ptr;
}
ସ୍ଥିର ଶୂନ୍ୟ *xxrealloc(ପୂର୍ଣ୍ଣସଂଖ୍ୟା lineno, ଶୂନ୍ୟ *ptr, size_t size)
{
	ptr = xrealloc(ptr, (size + 0xff) & ~0xff);
	fdprintf(2, "line %d: realloc %p\n", lineno, ptr);
	ଫେରନ୍ତୁ ptr;
}
ସ୍ଥିର ଅକ୍ଷର *xxstrdup(ପୂର୍ଣ୍ଣସଂଖ୍ୟା lineno, ଅଚଳ ଅକ୍ଷର *str)
{
	ଅକ୍ଷର *ptr = xstrdup(str);
	fdprintf(2, "line %d: strdup %p\n", lineno, ptr);
	ଫେରନ୍ତୁ ptr;
}
ସ୍ଥିର ଶୂନ୍ୟ xxfree(ଶୂନ୍ୟ *ptr)
{
	fdprintf(2, "free %p\n", ptr);
	ମୁକ୍ତ(ptr);
}
# define xmalloc(s)     xxmalloc(__LINE__, s)
# define xrealloc(p, s) xxrealloc(__LINE__, p, s)
# define xstrdup(s)     xxstrdup(__LINE__, s)
# define ମୁକ୍ତ(p)        xxfree(p)
#endif

/* Syntax and runtime errors. They always abort scripts.
 * In interactive use they usually discard unparsed and/or unexecuted commands
 * and return to the prompt.
 * HUSH_DEBUG >= 2 prints line number in this file where it was detected.
 */
#ଯଦି HUSH_DEBUG < 2
# define ସନ୍ଦେଶ_and_die_if_script(lineno, ...)     ସନ୍ଦେଶ_and_die_if_script(__VA_ARGS__)
# define syntax_error(lineno, ସନ୍ଦେଶ)              syntax_error(ସନ୍ଦେଶ)
# define syntax_error_at(lineno, ସନ୍ଦେଶ)           syntax_error_at(ସନ୍ଦେଶ)
# define syntax_error_unterm_ch(lineno, ch)     syntax_error_unterm_ch(ch)
# define syntax_error_unterm_str(lineno, s)     syntax_error_unterm_str(s)
# define syntax_error_unexpected_ch(lineno, ch) syntax_error_unexpected_ch(ch)
#endif

ସ୍ଥିର ଶୂନ୍ୟ die_if_script(ଶୂନ୍ୟ)
{
	ଯଦି (!G_interactive_fd) {
		ଯଦି (G.last_exitcode) /* sometines it's 2, not 1 (bash compat) */
			xfunc_error_retval = G.last_exitcode;
		xfunc_die();
	}
}

#ifdef __U_BOOT__
ସ୍ଥିର ଶୂନ୍ୟ __maybe_unused ସନ୍ଦେଶ_and_die_if_script(unsigned lineno, ଅଚଳ ଅକ୍ଷର *fmt, ...)
#ଅନ୍ୟଥା /* !__U_BOOT__ */
ସ୍ଥିର ଶୂନ୍ୟ ସନ୍ଦେଶ_and_die_if_script(unsigned lineno, ଅଚଳ ଅକ୍ଷର *fmt, ...)
#endif /* !__U_BOOT__ */
{
	va_list p;

#ଯଦି HUSH_DEBUG >= 2
	bb_error_ସନ୍ଦେଶ("hush.c:%u", lineno);
#endif
	va_start(p, fmt);
	bb_verror_ସନ୍ଦେଶ(fmt, p, ଖାଲି);
	va_end(p);
	die_if_script();
}

ସ୍ଥିର ଶୂନ୍ୟ syntax_error(unsigned lineno UNUSED_PARAM, ଅଚଳ ଅକ୍ଷର *ସନ୍ଦେଶ)
{
	ଯଦି (ସନ୍ଦେଶ)
		bb_error_ସନ୍ଦେଶ("syntax error: %s", ସନ୍ଦେଶ);
	ଅନ୍ୟଥା
		bb_simple_error_ସନ୍ଦେଶ("syntax error");
	die_if_script();
}

ସ୍ଥିର ଶୂନ୍ୟ syntax_error_at(unsigned lineno UNUSED_PARAM, ଅଚଳ ଅକ୍ଷର *ସନ୍ଦେଶ)
{
	bb_error_ସନ୍ଦେଶ("syntax error at '%s'", ସନ୍ଦେଶ);
	die_if_script();
}

ସ୍ଥିର ଶୂନ୍ୟ syntax_error_unterm_str(unsigned lineno UNUSED_PARAM, ଅଚଳ ଅକ୍ଷର *s)
{
	bb_error_ସନ୍ଦେଶ("syntax error: unterminated %s", s);
//? source4.tests fails: in bash, echo ${^} in script does not terminate the script
// (but bash --posix, or if bash is run as "sh", does terminate in script, so maybe uncomment this?)
//	die_if_script();
}

ସ୍ଥିର ଶୂନ୍ୟ syntax_error_unterm_ch(unsigned lineno, ଅକ୍ଷର ch)
{
	ଅକ୍ଷର ସନ୍ଦେଶ[2] = { ch, '\0' };
	syntax_error_unterm_str(lineno, ସନ୍ଦେଶ);
}

ସ୍ଥିର ଶୂନ୍ୟ syntax_error_unexpected_ch(unsigned lineno UNUSED_PARAM, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ch)
{
	ଅକ୍ଷର ସନ୍ଦେଶ[2];
	ସନ୍ଦେଶ[0] = ch;
	ସନ୍ଦେଶ[1] = '\0';
#ଯଦି HUSH_DEBUG >= 2
	bb_error_ସନ୍ଦେଶ("hush.c:%u", lineno);
#endif
	bb_error_ସନ୍ଦେଶ("syntax error: unexpected %s", ch == EOF ? "EOF" : ସନ୍ଦେଶ);
	die_if_script();
}

#ଯଦି HUSH_DEBUG < 2
# undef ସନ୍ଦେଶ_and_die_if_script
# undef syntax_error
# undef syntax_error_at
# undef syntax_error_unterm_ch
# undef syntax_error_unterm_str
# undef syntax_error_unexpected_ch
#ଅନ୍ୟଥା
# define ସନ୍ଦେଶ_and_die_if_script(...)     ସନ୍ଦେଶ_and_die_if_script(__LINE__, __VA_ARGS__)
# define syntax_error(ସନ୍ଦେଶ)              syntax_error(__LINE__, ସନ୍ଦେଶ)
# define syntax_error_at(ସନ୍ଦେଶ)           syntax_error_at(__LINE__, ସନ୍ଦେଶ)
# define syntax_error_unterm_ch(ch)     syntax_error_unterm_ch(__LINE__, ch)
# define syntax_error_unterm_str(s)     syntax_error_unterm_str(__LINE__, s)
# define syntax_error_unexpected_ch(ch) syntax_error_unexpected_ch(__LINE__, ch)
#endif

/* Utility functions
 */
/* Replace each \x with x in place, return ptr past NUL. */
ସ୍ଥିର ଅକ୍ଷର *unbackslash(ଅକ୍ଷର *src)
{
#ifdef __U_BOOT__
	ଅକ୍ଷର *dst = src = (ଅକ୍ଷର *)strchrnul(src, '\\');
#ଅନ୍ୟଥା /* !__U_BOOT__ */
	ଅକ୍ଷର *dst = src = strchrnul(src, '\\');
#endif /* !__U_BOOT__ */
	ଯେତେବେଳେ (1) {
		ଯଦି (*src == '\\') {
			src++;
			ଯଦି (*src != '\0') {
				/* \x -> x */
				*dst++ = *src++;
				ଜାରି;
			}
			/* else: "\<nul>". Do not delete this backslash.
			 * Testcase: eval 'echo ok\'
			 */
			*dst++ = '\\';
			/* fallthrough */
		}
		ଯଦି ((*dst++ = *src++) == '\0')
			ଭାଙ୍ଗ;
	}
	ଫେରନ୍ତୁ dst;
}

ସ୍ଥିର ଅକ୍ଷର **add_strings_to_strings(ଅକ୍ଷର **strings, ଅକ୍ଷର **add, ପୂର୍ଣ୍ଣସଂଖ୍ୟା need_to_dup)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	unsigned count1;
	unsigned count2;
	ଅକ୍ଷର **v;

	v = strings;
	count1 = 0;
	ଯଦି (v) {
		ଯେତେବେଳେ (*v) {
			count1++;
			v++;
		}
	}
	count2 = 0;
	v = add;
	ଯେତେବେଳେ (*v) {
		count2++;
		v++;
	}
	v = xrealloc(strings, (count1 + count2 + 1) * ଆକାର(ଅକ୍ଷର*));
	v[count1 + count2] = ଖାଲି;
	i = count2;
	ଯେତେବେଳେ (--i >= 0)
		v[count1 + i] = (need_to_dup ? xstrdup(add[i]) : add[i]);
	ଫେରନ୍ତୁ v;
}
#ଯଦି LEAK_HUNTING
ସ୍ଥିର ଅକ୍ଷର **xx_add_strings_to_strings(ପୂର୍ଣ୍ଣସଂଖ୍ୟା lineno, ଅକ୍ଷର **strings, ଅକ୍ଷର **add, ପୂର୍ଣ୍ଣସଂଖ୍ୟା need_to_dup)
{
	ଅକ୍ଷର **ptr = add_strings_to_strings(strings, add, need_to_dup);
	fdprintf(2, "line %d: add_strings_to_strings %p\n", lineno, ptr);
	ଫେରନ୍ତୁ ptr;
}
#define add_strings_to_strings(strings, add, need_to_dup) \
	xx_add_strings_to_strings(__LINE__, strings, add, need_to_dup)
#endif

/* Note: takes ownership of "add" ptr (it is not strdup'ed) */
ସ୍ଥିର ଅକ୍ଷର **add_string_to_strings(ଅକ୍ଷର **strings, ଅକ୍ଷର *add)
{
	ଅକ୍ଷର *v[2];
	v[0] = add;
	v[1] = ଖାଲି;
	ଫେରନ୍ତୁ add_strings_to_strings(strings, v, /*dup:*/ 0);
}

#ଯଦି LEAK_HUNTING
ସ୍ଥିର ଅକ୍ଷର **xx_add_string_to_strings(ପୂର୍ଣ୍ଣସଂଖ୍ୟା lineno, ଅକ୍ଷର **strings, ଅକ୍ଷର *add)
{
	ଅକ୍ଷର **ptr = add_string_to_strings(strings, add);
	fdprintf(2, "line %d: add_string_to_strings %p\n", lineno, ptr);
	ଫେରନ୍ତୁ ptr;
}
#define add_string_to_strings(strings, add) \
	xx_add_string_to_strings(__LINE__, strings, add)
#endif

ସ୍ଥିର ଶୂନ୍ୟ free_strings(ଅକ୍ଷର **strings)
{
	ଅକ୍ଷର **v;

	ଯଦି (!strings)
		ଫେରନ୍ତୁ;
	v = strings;
	ଯେତେବେଳେ (*v) {
		ମୁକ୍ତ(*v);
		v++;
	}
	ମୁକ୍ତ(strings);
}

#ifndef __U_BOOT__
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା dup_CLOEXEC(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd, ପୂର୍ଣ୍ଣସଂଖ୍ୟା avoid_fd)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା newfd;
 repeat:
	newfd = fcntl(fd, F_DUPFD_CLOEXEC, avoid_fd + 1);
	ଯଦି (newfd >= 0) {
		ଯଦି (F_DUPFD_CLOEXEC == F_DUPFD) /* if old libc (w/o F_DUPFD_CLOEXEC) */
			close_on_exec_on(newfd);
	} ଅନ୍ୟଥା { /* newfd < 0 */
		ଯଦି (errno == EBUSY)
			ଯାଅ repeat;
		ଯଦି (errno == EINTR)
			ଯାଅ repeat;
		ଯଦି (errno != EBADF) {
			/* "echo >&9999" gets EINVAL trying to save fd 1 to above 9999.
			 * We could try saving it _below_ 9999 instead (how?), but
			 * this probably means that dup2(9999,1) to effectuate >&9999
			 * would also not work: fd 9999 can't exist.
			 * (This differs from "echo >&99" where saving works, but
			 * subsequent dup2(99,1) fails if fd 99 is not open).
			 */
			bb_perror_ସନ୍ଦେଶ("fcntl(%d,F_DUPFD,%d)", fd, avoid_fd + 1);
		}
	}
	ଫେରନ୍ତୁ newfd;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା xdup_CLOEXEC_and_close(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd, ପୂର୍ଣ୍ଣସଂଖ୍ୟା avoid_fd)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା newfd;
 repeat:
	newfd = fcntl(fd, F_DUPFD_CLOEXEC, avoid_fd + 1);
	ଯଦି (newfd < 0) {
		ଯଦି (errno == EBUSY)
			ଯାଅ repeat;
		ଯଦି (errno == EINTR)
			ଯାଅ repeat;
		/* fd was not open? */
		ଯଦି (errno == EBADF)
			ଫେରନ୍ତୁ fd;
		xfunc_die();
	}
	ଯଦି (F_DUPFD_CLOEXEC == F_DUPFD) /* if old libc (w/o F_DUPFD_CLOEXEC) */
		close_on_exec_on(newfd);
	close(fd);
	ଫେରନ୍ତୁ newfd;
}

/* Manipulating HFILEs */
ସ୍ଥିର HFILE *hfopen(ଅଚଳ ଅକ୍ଷର *name)
{
	HFILE *fp;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd;

	fd = STDIN_FILENO;
	ଯଦି (name) {
		fd = open(name, O_RDONLY | O_CLOEXEC);
		ଯଦି (fd < 0)
			ଫେରନ୍ତୁ ଖାଲି;
		ଯଦି (O_CLOEXEC == 0) /* ancient libc */
			close_on_exec_on(fd);
	}

	fp = xmalloc(ଆକାର(*fp));
	ଯଦି (name == ଖାଲି)
		G.HFILE_stdin = fp;
	fp->fd = fd;
	fp->cur = fp->end = fp->buf;
	fp->next_hfile = G.HFILE_list;
	G.HFILE_list = fp;
	ଫେରନ୍ତୁ fp;
}
ସ୍ଥିର ଶୂନ୍ୟ hfclose(HFILE *fp)
{
	HFILE **pp = &G.HFILE_list;
	ଯେତେବେଳେ (*pp) {
		HFILE *cur = *pp;
		ଯଦି (cur == fp) {
			*pp = cur->next_hfile;
			ଭାଙ୍ଗ;
		}
		pp = &cur->next_hfile;
	}
	ଯଦି (fp->fd >= 0)
		close(fp->fd);
	ମୁକ୍ତ(fp);
}
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା refill_HFILE_and_getc(HFILE *fp)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା n;

	ଯଦି (fp->fd < 0) {
		/* Already saw EOF */
		ଫେରନ୍ତୁ EOF;
	}
#ଯଦି ENABLE_HUSH_INTERACTIVE && !ENABLE_FEATURE_EDITING
	/* If user presses ^C, read() restarts after SIGINT (we use SA_RESTART).
	 * IOW: ^C will not immediately stop line input.
	 * But poll() is different: it does NOT restart after signals.
	 */
	ଯଦି (fp == G.HFILE_stdin) {
		ସଂରଚନା pollfd pfd[1];
		pfd[0].fd = fp->fd;
		pfd[0].events = POLLIN;
		n = poll(pfd, 1, -1);
		ଯଦି (n < 0
		 /*&& errno == EINTR - assumed true */
		 && sigismember(&G.pending_set, SIGINT)
		) {
			ଫେରନ୍ତୁ '\0';
		}
	}
#ଅନ୍ୟଥା
/* if FEATURE_EDITING=y, we do not use this routine for interactive input */
#endif
	/* Try to buffer more input */
	n = safe_read(fp->fd, fp->buf, ଆକାର(fp->buf));
	ଯଦି (n < 0) {
		bb_simple_perror_ସନ୍ଦେଶ("read error");
		n = 0;
	}
	fp->cur = fp->buf;
	fp->end = fp->buf + n;
	ଯଦି (n == 0) {
		/* EOF/error */
		close(fp->fd);
		fp->fd = -1;
		ଫେରନ୍ତୁ EOF;
	}
	ଫେରନ୍ତୁ (unsigned ଅକ୍ଷର)(*fp->cur++);
}
/* Inlined for common case of non-empty buffer.
 */
ସ୍ଥିର ALWAYS_INLINE ପୂର୍ଣ୍ଣସଂଖ୍ୟା hfgetc(HFILE *fp)
{
	ଯଦି (fp->cur < fp->end)
		ଫେରନ୍ତୁ (unsigned ଅକ୍ଷର)(*fp->cur++);
	/* Buffer empty */
	ଫେରନ୍ତୁ refill_HFILE_and_getc(fp);
}
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା move_HFILEs_on_redirect(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd, ପୂର୍ଣ୍ଣସଂଖ୍ୟା avoid_fd)
{
	HFILE *fl = G.HFILE_list;
	ଯେତେବେଳେ (fl) {
		ଯଦି (fd == fl->fd) {
			/* We use it only on script files, they are all CLOEXEC */
			fl->fd = xdup_CLOEXEC_and_close(fd, avoid_fd);
			debug_printf_redir("redirect_fd %d: matches a script fd, moving it to %d\n", fd, fl->fd);
			ଫେରନ୍ତୁ 1; /* "found and moved" */
		}
		fl = fl->next_hfile;
	}
#ଯଦି ENABLE_HUSH_MODE_X
	ଯଦି (G.x_mode_fd > 0 && fd == G.x_mode_fd) {
		G.x_mode_fd = xdup_CLOEXEC_and_close(fd, avoid_fd);
		ଫେରନ୍ତୁ 1; /* "found and moved" */
	}
#endif
	ଫେରନ୍ତୁ 0; /* "not in the list" */
}
#ଯଦି ENABLE_FEATURE_SH_STANDALONE && BB_MMU
ସ୍ଥିର ଶୂନ୍ୟ close_all_HFILE_list(ଶୂନ୍ୟ)
{
	HFILE *fl = G.HFILE_list;
	ଯେତେବେଳେ (fl) {
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
		ଯଦି (fl->fd > 0)
			/*hfclose(fl); - unsafe */
			close(fl->fd);
		fl = fl->next_hfile;
	}
}
#endif
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd_in_HFILEs(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd)
{
	HFILE *fl = G.HFILE_list;
	ଯେତେବେଳେ (fl) {
		ଯଦି (fl->fd == fd)
			ଫେରନ୍ତୁ 1;
		fl = fl->next_hfile;
	}
	ଫେରନ୍ତୁ 0;
}

#endif /* !__U_BOOT__ */

/* Helpers for setting new $n and restoring them back
 */
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା save_arg_t {
	ଅକ୍ଷର *sv_argv0;
	ଅକ୍ଷର **sv_g_argv;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା sv_g_argc;
#ifndef __U_BOOT__
	IF_HUSH_SET(smallint sv_g_malloced;)
#endif /* !__U_BOOT__ */
} save_arg_t;

#ifndef __U_BOOT__
ସ୍ଥିର ଶୂନ୍ୟ save_and_replace_G_args(save_arg_t *sv, ଅକ୍ଷର **argv)
{
	sv->sv_argv0 = argv[0];
	sv->sv_g_argv = G.global_argv;
	sv->sv_g_argc = G.global_argc;
	IF_HUSH_SET(sv->sv_g_malloced = G.global_args_malloced;)

	argv[0] = G.global_argv[0]; /* retain $0 */
	G.global_argv = argv;
	IF_HUSH_SET(G.global_args_malloced = 0;)

	G.global_argc = 1 + string_array_len(argv + 1);
}

ସ୍ଥିର ଶୂନ୍ୟ restore_G_args(save_arg_t *sv, ଅକ୍ଷର **argv)
{
#ଯଦି ENABLE_HUSH_SET
	ଯଦି (G.global_args_malloced) {
		/* someone ran "set -- arg1 arg2 ...", undo */
		ଅକ୍ଷର **pp = G.global_argv;
		ଯେତେବେଳେ (*++pp) /* note: does not free $0 */
			ମୁକ୍ତ(*pp);
		ମୁକ୍ତ(G.global_argv);
	}
#endif
	argv[0] = sv->sv_argv0;
	G.global_argv = sv->sv_g_argv;
	G.global_argc = sv->sv_g_argc;
	IF_HUSH_SET(G.global_args_malloced = sv->sv_g_malloced;)
}
#endif /* !__U_BOOT__ */

#ifndef __U_BOOT__
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
ଗଣନା {
	SPECIAL_INTERACTIVE_SIGS = 0
		| (1 << SIGTERM)
		| (1 << SIGINT)
		| (1 << SIGHUP)
		,
	SPECIAL_JOBSTOP_SIGS = 0
#ଯଦି ENABLE_HUSH_JOB
		| (1 << SIGTTIN)
		| (1 << SIGTTOU)
		| (1 << SIGTSTP)
#endif
		,
};

ସ୍ଥିର ଶୂନ୍ୟ record_pending_signo(ପୂର୍ଣ୍ଣସଂଖ୍ୟା sig)
{
	sigaddset(&G.pending_set, sig);
#ଯଦି ENABLE_FEATURE_EDITING
	ଯଦି (sig != SIGCHLD
	 || (G_traps && G_traps[SIGCHLD] && G_traps[SIGCHLD][0])
	 /* ^^^ if SIGCHLD, interrupt line reading only if it has a trap */
	) {
		bb_got_signal = sig; /* for read_line_input: "we got a signal" */
	}
#endif
#ଯଦି ENABLE_HUSH_FAST
	ଯଦି (sig == SIGCHLD) {
		G.ଗଣନା_SIGCHLD++;
//bb_error_msg("[%d] SIGCHLD_handler: G.count_SIGCHLD:%d G.handled_SIGCHLD:%d", getpid(), G.count_SIGCHLD, G.handled_SIGCHLD);
	}
#endif
}

ସ୍ଥିର sighandler_t install_sighandler(ପୂର୍ଣ୍ଣସଂଖ୍ୟା sig, sighandler_t handler)
{
	ସଂରଚନା sigaction old_sa;

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
	ଫେରନ୍ତୁ old_sa.sa_handler;
}
#endif /* !__U_BOOT__ */

#ifndef __U_BOOT__
ସ୍ଥିର ଶୂନ୍ୟ hush_exit(ପୂର୍ଣ୍ଣସଂଖ୍ୟା exitcode) NORETURN;

ସ୍ଥିର ଶୂନ୍ୟ restore_ttypgrp_and__exit(ଶୂନ୍ୟ) NORETURN;
ସ୍ଥିର ଶୂନ୍ୟ restore_ttypgrp_and__exit(ଶୂନ୍ୟ)
{
	/* xfunc has failed! die die die */
	/* no EXIT traps, this is an escape hatch! */
	G.exiting = 1;
	hush_exit(xfunc_error_retval);
}

#ଯଦି ENABLE_HUSH_JOB

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
ସ୍ଥିର ଶୂନ୍ୟ fflush_and__exit(ଶୂନ୍ୟ) NORETURN;
ସ୍ଥିର ଶୂନ୍ୟ fflush_and__exit(ଶୂନ୍ୟ)
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
ସ୍ଥିର ଶୂନ୍ୟ sigexit(ପୂର୍ଣ୍ଣସଂଖ୍ୟା sig) NORETURN;
ସ୍ଥିର ଶୂନ୍ୟ sigexit(ପୂର୍ଣ୍ଣସଂଖ୍ୟା sig)
{
	/* Careful: we can end up here after [v]fork. Do not restore
	 * tty pgrp then, only top-level shell process does that */
	ଯଦି (G_saved_tty_pgrp && getpid() == G.ମୂଳ_pid) {
		/* Disable all signals: job control, SIGPIPE, etc.
		 * Mostly paranoid measure, to prevent infinite SIGTTOU.
		 */
		sigprocmask_allsigs(SIG_BLOCK);
		tcsetpgrp(G_interactive_fd, G_saved_tty_pgrp);
	}

	/* Not a signal, just exit */
	ଯଦି (sig <= 0)
		_exit(- sig);

	kill_myself_with_sig(sig); /* does not return */
}
#ଅନ୍ୟଥା

# define disable_restore_tty_pgrp_on_exit() ((ଶୂନ୍ୟ)0)
# define enable_restore_tty_pgrp_on_exit()  ((ଶୂନ୍ୟ)0)

#endif
#endif /* !__U_BOOT__ */

#ifndef __U_BOOT__
ସ୍ଥିର sighandler_t pick_sighandler(unsigned sig)
{
	sighandler_t handler = SIG_DFL;
	ଯଦି (sig < ଆକାର(unsigned)*8) {
		unsigned sigmask = (1 << sig);

#ଯଦି ENABLE_HUSH_JOB
		/* is sig fatal? */
		ଯଦି (G_fatal_sig_mask & sigmask)
			handler = sigexit;
		ଅନ୍ୟଥା
#endif
		/* sig has special handling? */
		ଯଦି (G.special_sig_mask & sigmask) {
			handler = record_pending_signo;
			/* TTIN/TTOU/TSTP can't be set to record_pending_signo
			 * in order to ignore them: they will be raised
			 * in an endless loop when we try to do some
			 * terminal ioctls! We do have to _ignore_ these.
			 */
			ଯଦି (SPECIAL_JOBSTOP_SIGS & sigmask)
				handler = SIG_IGN;
		}
	}
	ଫେରନ୍ତୁ handler;
}
#endif /* !__U_BOOT__ */

#ifndef __U_BOOT__
/* Restores tty foreground process group, and exits. */
ସ୍ଥିର ଶୂନ୍ୟ hush_exit(ପୂର୍ଣ୍ଣସଂଖ୍ୟା exitcode)
{
#ଯଦି ENABLE_FEATURE_EDITING_SAVE_ON_EXIT
	save_history(G.ଧାଡ଼ି_ନିବେଶ_state); /* may be NULL */
#endif

	fflush_all();
	ଯଦି (G.exiting <= 0 && G_traps && G_traps[0] && G_traps[0][0]) {
		ଅକ୍ଷର *argv[3];
		/* argv[0] is unused */
		argv[1] = xstrdup(G_traps[0]); /* copy, since EXIT trap handler may modify G_traps[0] */
		argv[2] = ଖାଲି;
		G.exiting = 1; /* prevent EXIT trap recursion */
		/* Note: G_traps[0] is not cleared!
		 * "trap" will still show it, if executed
		 * in the handler */
		builtin_eval(argv);
	}

#ଯଦି ENABLE_FEATURE_CLEAN_UP
	{
		ସଂରଚନା variable *cur_var;
		ଯଦି (G.cwd != bb_ସନ୍ଦେଶ_unknown)
			ମୁକ୍ତ((ଅକ୍ଷର*)G.cwd);
		cur_var = G.ଶୀର୍ଷ_var;
		ଯେତେବେଳେ (cur_var) {
			ସଂରଚନା variable *tmp = cur_var;
			ଯଦି (!cur_var->max_len)
				ମୁକ୍ତ(cur_var->varstr);
			cur_var = cur_var->next;
			ମୁକ୍ତ(tmp);
		}
	}
#endif

	fflush_all();
#ଯଦି ENABLE_HUSH_JOB
	sigexit(- (exitcode & 0xff));
#ଅନ୍ୟଥା
	_exit(exitcode);
#endif
}

//TODO: return a mask of ALL handled sigs?
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା check_and_run_traps(ଶୂନ୍ୟ)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା last_sig = 0;

	ଯେତେବେଳେ (1) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା sig;

		ଯଦି (sigisemptyset(&G.pending_set))
			ଭାଙ୍ଗ;
		sig = 0;
		କର {
			sig++;
			ଯଦି (sigismember(&G.pending_set, sig)) {
				sigdelset(&G.pending_set, sig);
				ଯାଅ got_sig;
			}
		} ଯେତେବେଳେ (sig < NSIG);
		ଭାଙ୍ଗ;
 got_sig:
#ଯଦି ENABLE_HUSH_TRAP
		ଯଦି (G_traps && G_traps[sig]) {
			debug_printf_exec("%s: sig:%d handler:'%s'\n", __func__, sig, G.traps[sig]);
			ଯଦି (G_traps[sig][0]) {
				/* We have user-defined handler */
				smalluint save_rcode;
				ପୂର୍ଣ୍ଣସଂଖ୍ୟା save_pre;
				ଅକ୍ଷର *argv[3];
				/* argv[0] is unused */
				argv[1] = xstrdup(G_traps[sig]);
				/* why strdup? trap can modify itself: trap 'trap "echo oops" INT' INT */
				argv[2] = ଖାଲି;
				save_pre = G.pre_trap_exitcode;
				G.pre_trap_exitcode = save_rcode = G.last_exitcode;
				builtin_eval(argv);
				ମୁକ୍ତ(argv[1]);
				G.pre_trap_exitcode = save_pre;
				G.last_exitcode = save_rcode;
# ଯଦି ENABLE_HUSH_FUNCTIONS
				ଯଦି (G.return_exitcode >= 0) {
					debug_printf_exec("trap exitcode:%d\n", G.return_exitcode);
					G.last_exitcode = G.return_exitcode;
				}
# endif
				last_sig = sig;
			} /* else: "" trap, ignoring signal */
			ଜାରି;
		}
#endif
		/* not a trap: special action */
		ବାଛ (sig) {
		ସ୍ଥିତି SIGINT:
			debug_printf_exec("%s: sig:%d default SIGINT handler\n", __func__, sig);
			G.flag_SIGINT = 1;
			last_sig = sig;
			ଭାଙ୍ଗ;
#ଯଦି ENABLE_HUSH_JOB
		ସ୍ଥିତି SIGHUP: {
			/* if (G_interactive_fd) - no need to check, the handler
			 * is only installed if we *are* interactive */
			{
				/* bash compat: "Before exiting, an interactive
				 * shell resends the SIGHUP to all jobs, running
				 * or stopped.  Stopped jobs are sent SIGCONT
				 * to ensure that they receive the SIGHUP."
				 */
				ସଂରଚନା pipe *job;
				debug_printf_exec("%s: sig:%d default SIGHUP handler\n", __func__, sig);
				/* bash is observed to signal whole process groups,
				 * not individual processes */
				ପାଇଁ (job = G.job_list; job; job = job->next) {
					ଯଦି (job->pgrp <= 0)
						ଜାରି;
					debug_printf_exec("HUPing pgrp %d\n", job->pgrp);
					ଯଦି (kill(- job->pgrp, SIGHUP) == 0)
						kill(- job->pgrp, SIGCONT);
				}
			}
			/* this restores tty pgrp, then kills us with SIGHUP */
			sigexit(SIGHUP);
		}
#endif
#ଯଦି ENABLE_HUSH_FAST
		ସ୍ଥିତି SIGCHLD:
			debug_printf_exec("%s: sig:%d default SIGCHLD handler\n", __func__, sig);
			G.ଗଣନା_SIGCHLD++;
//bb_error_msg("[%d] check_and_run_traps: G.count_SIGCHLD:%d G.handled_SIGCHLD:%d", getpid(), G.count_SIGCHLD, G.handled_SIGCHLD);
			/* Note:
			 * We don't do 'last_sig = sig' here -> NOT returning this sig.
			 * This simplifies wait builtin a bit.
			 */
			ଭାଙ୍ଗ;
#endif
		ପୂର୍ବନିର୍ଦ୍ଧାରିତ: /* ignored: */
			debug_printf_exec("%s: sig:%d default handling is to ignore\n", __func__, sig);
			/* SIGTERM, SIGQUIT, SIGTTIN, SIGTTOU, SIGTSTP */
			/* Note:
			 * We don't do 'last_sig = sig' here -> NOT returning this sig.
			 * Example: wait is not interrupted by TERM
			 * in interactive shell, because TERM is ignored.
			 */
			ଭାଙ୍ଗ;
		}
	}
	ଫେରନ୍ତୁ last_sig;
}

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *get_cwd(ପୂର୍ଣ୍ଣସଂଖ୍ୟା force)
{
	ଯଦି (force || G.cwd == ଖାଲି) {
		/* xrealloc_getcwd_or_warn(arg) calls free(arg),
		 * we must not try to free(bb_msg_unknown) */
		ଯଦି (G.cwd == bb_ସନ୍ଦେଶ_unknown)
			G.cwd = ଖାଲି;
		G.cwd = xrealloc_getcwd_or_warn((ଅକ୍ଷର *)G.cwd);
		ଯଦି (!G.cwd)
			G.cwd = bb_ସନ୍ଦେଶ_unknown;
	}
	ଫେରନ୍ତୁ G.cwd;
}

#endif /* !__U_BOOT__ */

/*
 * Shell and environment variable support
 */
ସ୍ଥିର ସଂରଚନା variable **get_ptr_to_local_var(ଅଚଳ ଅକ୍ଷର *name)
{
	ସଂରଚନା variable **pp;
	ସଂରଚନା variable *cur;

	pp = &G.ଶୀର୍ଷ_var;
	ଯେତେବେଳେ ((cur = *pp) != ଖାଲି) {
		ଯଦି (varcmp(cur->varstr, name) == 0)
			ଫେରନ୍ତୁ pp;
		pp = &cur->next;
	}
	ଫେରନ୍ତୁ ଖାଲି;
}

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର* FAST_FUNC get_local_var_ମୂଲ୍ୟ(ଅଚଳ ଅକ୍ଷର *name)
{
	ସଂରଚନା variable **vpp;

	ଯଦି (G.expanded_assignments) {
		ଅକ୍ଷର **cpp = G.expanded_assignments;
		ଯେତେବେଳେ (*cpp) {
			ଅକ୍ଷର *cp = *cpp;
			ଯଦି (varcmp(cp, name) == 0)
				ଫେରନ୍ତୁ strchr(cp, '=') + 1;
			cpp++;
		}
	}

	vpp = get_ptr_to_local_var(name);
	ଯଦି (vpp)
		ଫେରନ୍ତୁ strchr((*vpp)->varstr, '=') + 1;

#ifndef __U_BOOT__
	ଯଦି (strcmp(name, "PPID") == 0)
		ଫେରନ୍ତୁ utoa(G.ମୂଳ_ppid);
#endif /* !__U_BOOT__ */
	// bash compat: UID? EUID?
#ଯଦି ENABLE_HUSH_RANDOM_SUPPORT
	ଯଦି (strcmp(name, "RANDOM") == 0)
		ଫେରନ୍ତୁ utoa(next_random(&G.random_gen));
#endif
#ଯଦି ENABLE_HUSH_LINENO_VAR
	ଯଦି (strcmp(name, "LINENO") == 0)
		ଫେରନ୍ତୁ utoa(G.execute_lineno);
#endif
#ଯଦି BASH_EPOCH_VARS
	{
		ଅଚଳ ଅକ୍ଷର *fmt = ଖାଲି;
		ଯଦି (strcmp(name, "EPOCHSECONDS") == 0)
			fmt = "%llu";
		ଅନ୍ୟଥା ଯଦି (strcmp(name, "EPOCHREALTIME") == 0)
			fmt = "%llu.%06u";
		ଯଦି (fmt) {
			ସଂରଚନା timeval tv;
			xgettimeofday(&tv);
			sprintf(G.epoch_buf, fmt, (unsigned long long)tv.tv_sec,
					(unsigned)tv.tv_usec);
			ଫେରନ୍ତୁ G.epoch_buf;
		}
	}
#endif
	ଫେରନ୍ତୁ ଖାଲି;
}

#ifndef __U_BOOT__
#ଯଦି ENABLE_HUSH_GETOPTS
ସ୍ଥିର ଶୂନ୍ୟ handle_changed_special_names(ଅଚଳ ଅକ୍ଷର *name)
{
	ଯଦି (varcmp(name, "OPTIND") == 0) {
		G.getopt_ଗଣନା = 0;
		ଫେରନ୍ତୁ;
	}
}
#ଅନ୍ୟଥା
/* Do not even bother evaluating arguments */
# define handle_changed_special_names(...) ((ଶୂନ୍ୟ)0)
#endif
#ଅନ୍ୟଥା /* __U_BOOT__ */
/* Do not even bother evaluating arguments */
# define handle_changed_special_names(...) ((ଶୂନ୍ୟ)0)
#endif /* __U_BOOT__ */

/* str holds "NAME=VAL" and is expected to be malloced.
 * We take ownership of it.
 */
#ifndef __U_BOOT__
#define SETFLAG_EXPORT   (1 << 0)
#define SETFLAG_UNEXPORT (1 << 1)
#define SETFLAG_MAKE_RO  (1 << 2)
#endif /* !__U_BOOT__ */
#define SETFLAG_VARLVL_SHIFT   3
#ifndef __U_BOOT__
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା set_local_var(ଅକ୍ଷର *str, unsigned flags)
#ଅନ୍ୟଥା /* __U_BOOT__ */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା set_local_var_modern(ଅକ୍ଷର *str, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags)
#endif /* __U_BOOT__ */
{
	ସଂରଚନା variable **cur_pp;
	ସଂରଚନା variable *cur;
	ଅକ୍ଷର *free_me = ଖାଲି;
	ଅକ୍ଷର *eq_sign;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା name_len;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା retval;
#ifndef __U_BOOT__
	unsigned local_lvl = (flags >> SETFLAG_VARLVL_SHIFT);
#endif /* !__U_BOOT__ */

	eq_sign = strchr(str, '=');
	ଯଦି (HUSH_DEBUG && !eq_sign)
		bb_simple_error_ସନ୍ଦେଶ_and_die("BUG in setvar");

	name_len = eq_sign - str + 1; /* including '=' */
	cur_pp = &G.ଶୀର୍ଷ_var;
	ଯେତେବେଳେ ((cur = *cur_pp) != ଖାଲି) {
		ଯଦି (strncmp(cur->varstr, str, name_len) != 0) {
			cur_pp = &cur->next;
			ଜାରି;
		}

#ifndef __U_BOOT__
		/* We found an existing var with this name */
		ଯଦି (cur->flg_read_only) {
			bb_error_ସନ୍ଦେଶ("%s: readonly variable", str);
			ମୁକ୍ତ(str);
//NOTE: in bash, assignment in "export READONLY_VAR=Z" fails, and sets $?=1,
//but export per se succeeds (does put the var in env). We don't mimic that.
			ଫେରନ୍ତୁ -1;
		}
		ଯଦି (flags & SETFLAG_UNEXPORT) { // && cur->flg_export ?
			debug_printf_env("%s: unsetenv '%s'\n", __func__, str);
			*eq_sign = '\0';
			unsetenv(str);
			*eq_sign = '=';
		}
		ଯଦି (cur->var_nest_level < local_lvl) {
			/* bash 3.2.33(1) and exported vars:
			 * # export z=z
			 * # f() { local z=a; env | grep ^z; }
			 * # f
			 * z=a
			 * # env | grep ^z
			 * z=z
			 */
			ଯଦି (cur->flg_export)
				flags |= SETFLAG_EXPORT;
			/* New variable is local ("local VAR=VAL" or
			 * "VAR=VAL cmd")
			 * and existing one is global, or local
			 * on a lower level that new one.
			 * Remove it from global variable list:
			 */
			*cur_pp = cur->next;
			ଯଦି (G.shadowed_vars_pp) {
				/* Save in "shadowed" list */
				debug_printf_env("shadowing %s'%s'/%u by '%s'/%u\n",
					cur->flg_export ? "exported " : "",
					cur->varstr, cur->var_nest_level, str, local_lvl
				);
				cur->next = *G.shadowed_vars_pp;
				*G.shadowed_vars_pp = cur;
			} ଅନ୍ୟଥା {
				/* Came from pseudo_exec_argv(), no need to save: delete it */
				debug_printf_env("shadow-deleting %s'%s'/%u by '%s'/%u\n",
					cur->flg_export ? "exported " : "",
					cur->varstr, cur->var_nest_level, str, local_lvl
				);
				ଯଦି (cur->max_len == 0) /* allocated "VAR=VAL"? */
					free_me = cur->varstr; /* then free it later */
				ମୁକ୍ତ(cur);
			}
			ଭାଙ୍ଗ;
		}
#endif /* !__U_BOOT__ */

		ଯଦି (strcmp(cur->varstr + name_len, eq_sign + 1) == 0) {
			debug_printf_env("assignement '%s' does not change anything\n", str);
 free_and_exp:
			ମୁକ୍ତ(str);
			ଯାଅ exp;
		}

		/* Replace the value in the found "struct variable" */
		ଯଦି (cur->max_len != 0) {
			ଯଦି (cur->max_len >= strnlen(str, cur->max_len + 1)) {
				/* This one is from startup env, reuse space */
				debug_printf_env("reusing startup env for '%s'\n", str);
				strcpy(cur->varstr, str);
				ଯାଅ free_and_exp;
			}
			/* Can't reuse */
			cur->max_len = 0;
			ଯାଅ set_str_and_exp;
		}
		/* max_len == 0 signifies "malloced" var, which we can
		 * (and have to) free. But we can't free(cur->varstr) here:
		 * if cur->flg_export is 1, it is in the environment.
		 * We should either unsetenv+free, or wait until putenv,
		 * then putenv(new)+free(old).
		 */
		free_me = cur->varstr;
		ଯାଅ set_str_and_exp;
	}

	/* Not found or shadowed - create new variable struct */
#ifndef __U_BOOT__
	debug_printf_env("%s: alloc new var '%s'/%u\n", __func__, str, local_lvl);
#ଅନ୍ୟଥା /* __U_BOOT__ */
	debug_printf_env("%s: alloc new var '%s'\n", __func__, str);
#endif /* __U_BOOT__ */
	cur = xzalloc(ଆକାର(*cur));
#ifndef __U_BOOT__
	cur->var_nest_level = local_lvl;
#endif /* !__U_BOOT__ */
	cur->next = *cur_pp;
	*cur_pp = cur;

 set_str_and_exp:
	cur->varstr = str;
 exp:
#ifndef __U_BOOT__
#ଯଦି !BB_MMU || ENABLE_HUSH_READONLY
	ଯଦି (flags & SETFLAG_MAKE_RO) {
		cur->flg_read_only = 1;
	}
#endif
	ଯଦି (flags & SETFLAG_EXPORT)
		cur->flg_export = 1;
#endif /* !__U_BOOT__ */
	retval = 0;
#ifndef __U_BOOT__
	ଯଦି (cur->flg_export) {
		ଯଦି (flags & SETFLAG_UNEXPORT) {
			cur->flg_export = 0;
			/* unsetenv was already done */
		} ଅନ୍ୟଥା {
			debug_printf_env("%s: putenv '%s'/%u\n", __func__, cur->varstr, cur->var_nest_level);
			retval = putenv(cur->varstr);
			/* fall through to "free(free_me)" -
			 * only now we can free old exported malloced string
			 */
		}
	}
#endif /* !__U_BOOT__ */
	ମୁକ୍ତ(free_me);

	handle_changed_special_names(cur->varstr);

	ଫେରନ୍ତୁ retval;
}

#ifndef __U_BOOT__
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା set_local_var0(ଅକ୍ଷର *str)
{
	ଫେରନ୍ତୁ set_local_var(str, 0);
}

ସ୍ଥିର ଶୂନ୍ୟ FAST_FUNC set_local_var_from_halves(ଅଚଳ ଅକ୍ଷର *name, ଅଚଳ ଅକ୍ଷର *val)
{
	ଅକ୍ଷର *var = xasprintf("%s=%s", name, val);
	set_local_var0(var);
}

/* Used at startup and after each cd */
ସ୍ଥିର ଶୂନ୍ୟ set_pwd_var(unsigned flag)
{
	set_local_var(xasprintf("PWD=%s", get_cwd(/*force:*/ 1)), flag);
}
#endif /* !__U_BOOT__ */

#ଯଦି ENABLE_HUSH_UNSET || ENABLE_HUSH_GETOPTS
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା unset_local_var(ଅଚଳ ଅକ୍ଷର *name)
{
	ସଂରଚନା variable *cur;
	ସଂରଚନା variable **cur_pp;

	cur_pp = &G.ଶୀର୍ଷ_var;
	ଯେତେବେଳେ ((cur = *cur_pp) != ଖାଲି) {
		ଯଦି (varcmp(cur->varstr, name) == 0) {
			ଯଦି (cur->flg_read_only) {
				bb_error_ସନ୍ଦେଶ("%s: readonly variable", name);
				ଫେରନ୍ତୁ EXIT_FAILURE;
			}

			*cur_pp = cur->next;
			debug_printf_env("%s: unsetenv '%s'\n", __func__, cur->varstr);
			bb_unsetenv(cur->varstr);
			ଯଦି (!cur->max_len)
				ମୁକ୍ତ(cur->varstr);
			ମୁକ୍ତ(cur);

			ଭାଙ୍ଗ;
		}
		cur_pp = &cur->next;
	}

	/* Handle "unset LINENO" et al even if did not find the variable to unset */
	handle_changed_special_names(name);

	ଫେରନ୍ତୁ EXIT_SUCCESS;
}
#endif

#ifndef __U_BOOT__
/*
 * Helpers for "var1=val1 var2=val2 cmd" feature
 */
ସ୍ଥିର ଶୂନ୍ୟ add_vars(ସଂରଚନା variable *var)
{
	ସଂରଚନା variable *next;

	ଯେତେବେଳେ (var) {
		next = var->next;
		var->next = G.ଶୀର୍ଷ_var;
		G.ଶୀର୍ଷ_var = var;
		ଯଦି (var->flg_export) {
			debug_printf_env("%s: restoring exported '%s'/%u\n", __func__, var->varstr, var->var_nest_level);
			putenv(var->varstr);
		} ଅନ୍ୟଥା {
			debug_printf_env("%s: restoring variable '%s'/%u\n", __func__, var->varstr, var->var_nest_level);
		}
		var = next;
	}
}

/* We put strings[i] into variable table and possibly putenv them.
 * If variable is read only, we can free the strings[i]
 * which attempts to overwrite it.
 * The strings[] vector itself is freed.
 */
ସ୍ଥିର ଶୂନ୍ୟ set_vars_and_save_old(ଅକ୍ଷର **strings)
{
	ଅକ୍ଷର **s;

	ଯଦି (!strings)
		ଫେରନ୍ତୁ;

	s = strings;
	ଯେତେବେଳେ (*s) {
		ସଂରଚନା variable *var_p;
		ସଂରଚନା variable **var_pp;
		ଅକ୍ଷର *eq;

		eq = strchr(*s, '=');
		ଯଦି (HUSH_DEBUG && !eq)
			bb_simple_error_ସନ୍ଦେଶ_and_die("BUG in varexp4");
		var_pp = get_ptr_to_local_var(*s);
		ଯଦି (var_pp) {
			var_p = *var_pp;
			ଯଦି (var_p->flg_read_only) {
				ଅକ୍ଷର **p;
				bb_error_ସନ୍ଦେଶ("%s: readonly variable", *s);
				/*
				 * "VAR=V BLTIN" unsets VARs after BLTIN completes.
				 * If VAR is readonly, leaving it in the list
				 * after asssignment error (msg above)
				 * causes doubled error message later, on unset.
				 */
				debug_printf_env("removing/freeing '%s' element\n", *s);
				ମୁକ୍ତ(*s);
				p = s;
				କର { *p = p[1]; p++; } ଯେତେବେଳେ (*p);
				ଯାଅ next;
			}
			/* below, set_local_var() with nest level will
			 * "shadow" (remove) this variable from
			 * global linked list.
			 */
		}
		debug_printf_env("%s: env override '%s'/%u\n", __func__, *s, G.var_nest_level);
		set_local_var(*s, (G.var_nest_level << SETFLAG_VARLVL_SHIFT) | SETFLAG_EXPORT);
		s++;
 next: ;
	}
	ମୁକ୍ତ(strings);
}

/*
 * Unicode helper
 */
ସ୍ଥିର ଶୂନ୍ୟ reinit_unicode_for_hush(ଶୂନ୍ୟ)
{
	/* Unicode support should be activated even if LANG is set
	 * _during_ shell execution, not only if it was set when
	 * shell was started. Therefore, re-check LANG every time:
	 */
	ଯଦି (ENABLE_FEATURE_CHECK_UNICODE_IN_ENV
	 || ENABLE_UNICODE_USING_LOCALE
	) {
		ଅଚଳ ଅକ୍ଷର *s = get_local_var_ମୂଲ୍ୟ("LC_ALL");
		ଯଦି (!s) s = get_local_var_ମୂଲ୍ୟ("LC_CTYPE");
		ଯଦି (!s) s = get_local_var_ମୂଲ୍ୟ("LANG");
		reinit_unicode(s);
	}
}

#endif /* !__U_BOOT__ */
/*
 * in_str support (strings, and "strings" read from files).
 */

#ଯଦି ENABLE_HUSH_INTERACTIVE
#ifndef __U_BOOT__
/* To test correct lineedit/interactive behavior, type from command line:
 *	echo $P\
 *	\
 *	AT\
 *	H\
 *	\
 * It exercises a lot of corner cases.
 */
ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *setup_prompt_string(ଶୂନ୍ୟ)
{
	ଅଚଳ ଅକ୍ଷର *prompt_str;

	debug_printf_prompt("%s promptmode:%d\n", __func__, G.promptmode);

# ଯଦି ENABLE_FEATURE_EDITING_FANCY_PROMPT
	prompt_str = get_local_var_ମୂଲ୍ୟ(G.promptmode == 0 ? "PS1" : "PS2");
	ଯଦି (!prompt_str)
		prompt_str = "";
# ଅନ୍ୟଥା
	prompt_str = "> "; /* if PS2, else... */
	ଯଦି (G.promptmode == 0) { /* PS1 */
		/* No fancy prompts supported, (re)generate "CURDIR $ " by hand */
		ମୁକ୍ତ(G.PS1);
		/* bash uses $PWD value, even if it is set by user.
		 * It uses current dir only if PWD is unset.
		 * We always use current dir. */
		prompt_str = G.PS1 = xasprintf("%s %c ", get_cwd(0), (geteuid() != 0) ? '$' : '#');
	}
# endif
	debug_printf("prompt_str '%s'\n", prompt_str);
	ଫେରନ୍ତୁ prompt_str;
}
#endif /* !__U_BOOT__ */

#ifndef __U_BOOT__
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା get_user_ନିବେଶ(ସଂରଚନା in_str *i)
#ଅନ୍ୟଥା /* __U_BOOT__ */
ସ୍ଥିର ଶୂନ୍ୟ get_user_ନିବେଶ(ସଂରଚନା in_str *i)
#endif /* __U_BOOT__ */
{
#ifndef __U_BOOT__
# ଯଦି ENABLE_FEATURE_EDITING
	/* In EDITING case, this function reads next input line,
	 * saves it in i->p, then returns 1st char of it.
	 */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା r;
	ଅଚଳ ଅକ୍ଷର *prompt_str;

	prompt_str = setup_prompt_string();
	ପାଇଁ (;;) {
		reinit_unicode_for_hush();
		G.flag_SIGINT = 0;

		bb_got_signal = 0;
		ଯଦି (!sigisemptyset(&G.pending_set)) {
			/* Whoops, already got a signal, do not call read_line_input */
			bb_got_signal = r = -1;
		} ଅନ୍ୟଥା {
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
			r = read_ଧାଡ଼ି_ନିବେଶ(G.ଧାଡ଼ି_ନିବେଶ_state, prompt_str,
				G.user_ନିବେଶ_buf, CONFIG_FEATURE_EDITING_MAX_LEN-1
			);
			/* read_line_input intercepts ^C, "convert" it to SIGINT */
			ଯଦି (r == 0)
				ଉଠାଅ(SIGINT);
		}
		/* bash prints ^C (before running a trap, if any)
		 * both on keyboard ^C and on real SIGINT (non-kbd generated).
		 */
		ଯଦି (sigismember(&G.pending_set, SIGINT)) {
			ଲିଖ(STDOUT_FILENO, "^C\n", 3);
			G.last_exitcode = 128 | SIGINT;
		}
		check_and_run_traps();
		ଯଦି (r == 0) /* keyboard ^C? */
			ଜାରି; /* go back, read another input line */
		ଯଦି (r > 0) /* normal input? (no ^C, no ^D, no signals) */
			ଭାଙ୍ଗ;
		ଯଦି (!bb_got_signal) {
			/* r < 0: ^D/EOF/error detected (but not signal) */
			/* ^D on interactive input goes to next line before exiting: */
			ଲିଖ(STDOUT_FILENO, "\n", 1);
			i->p = ଖାଲି;
			i->peek_buf[0] = r = EOF;
			ଫେରନ୍ତୁ r;
		}
		/* it was a signal: go back, read another input line */
	}
	i->p = G.user_ନିବେଶ_buf;
	ଫେରନ୍ତୁ (unsigned ଅକ୍ଷର)*i->p++;
# ଅନ୍ୟଥା
	/* In !EDITING case, this function gets called for every char.
	 * Buffering happens deeper in the call chain, in hfgetc(i->file).
	 */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା r;

	ପାଇଁ (;;) {
		G.flag_SIGINT = 0;
		ଯଦି (i->last_char == '\0' || i->last_char == '\n') {
			ଅଚଳ ଅକ୍ଷର *prompt_str = setup_prompt_string();
			/* Why check_and_run_traps here? Try this interactively:
			 * $ trap 'echo INT' INT; (sleep 2; kill -INT $$) &
			 * $ <[enter], repeatedly...>
			 * Without check_and_run_traps, handler never runs.
			 */
			check_and_run_traps();
			fputs_stdout(prompt_str);
			fflush_all();
		}
		r = hfgetc(i->file);
		/* In !ENABLE_FEATURE_EDITING we don't use read_line_input,
		 * no ^C masking happens during fgetc, no special code for ^C:
		 * it generates SIGINT as usual.
		 */
		check_and_run_traps();
		ଯଦି (r != '\0' && !G.flag_SIGINT)
			ଭାଙ୍ଗ;
		ଯଦି (G.flag_SIGINT) {
			/* ^C or SIGINT: repeat */
			/* bash prints ^C even on real SIGINT (non-kbd generated) */
			/* kernel prints "^C" itself, just print newline: */
			ଲିଖ(STDOUT_FILENO, "\n", 1);
			G.last_exitcode = 128 | SIGINT;
		}
	}
	ଫେରନ୍ତୁ r;
# endif
#ଅନ୍ୟଥା /* __U_BOOT__ */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା n;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା promptme;
	ସ୍ଥିର ଅକ୍ଷର the_command[CONFIG_SYS_CBSIZE + 1];

	bootretry_reset_cmd_timeout();
	promptme = 1;
	n = u_boot_cli_readline(i);

# ifdef CONFIG_BOOT_RETRY_TIME
	ଯଦି (n == -2) {
		puts("\nTimeout waiting for command\n");
#  ifdef CONFIG_RESET_TO_RETRY
		do_reset(ଖାଲି, 0, 0, ଖାଲି);
#  elif IS_ENABLED(CONFIG_RETRY_BOOTCMD)
	strcpy(console_buffer, "run bootcmd\n");
# ଅନ୍ୟଥା
#	error "This only works with CONFIG_RESET_TO_RETRY or CONFIG_BOOT_RETRY_COMMAND enabled"
#  endif
	}
# endif
	ଯଦି (n == -1 ) {
		G.flag_repeat = 0;
		promptme = 0;
	}
	n = strlen(console_buffer);
	console_buffer[n] = '\n';
	console_buffer[n+1]= '\0';
	ଯଦି (had_ctrlc())
		G.flag_repeat = 0;
	clear_ctrlc();
	G.do_repeat = 0;
#ifndef __U_BOOT__
	ଯଦି (G.promptmode == 1) {
#ଅନ୍ୟଥା /* __U_BOOT__ */
	ଯଦି (!G.promptmode) {
#endif /* __U_BOOT__ */
		ଯଦି (console_buffer[0] == '\n'&& G.flag_repeat == 0) {
			strcpy(the_command, console_buffer);
		}
		ଅନ୍ୟଥା {
			ଯଦି (console_buffer[0] != '\n') {
				strcpy(the_command, console_buffer);
				G.flag_repeat = 1;
			}
			ଅନ୍ୟଥା {
				G.do_repeat = 1;
			}
		}
		i->p = the_command;
	}
	ଅନ୍ୟଥା {
		ଯଦି (console_buffer[0] != '\n') {
			ଯଦି (strlen(the_command) + strlen(console_buffer)
				< CONFIG_SYS_CBSIZE) {
				n = strlen(the_command);
#ifdef __U_BOOT__
				/*
				 * To avoid writing to bad places, we check if
				 * n is greater than 0.
				 * This bug was found by Harald Seiler.
				 */
				ଯଦି (n > 0)
					the_command[n-1] = ' ';
				strcpy(&the_command[n], console_buffer);
#ଅନ୍ୟଥା /* !__U_BOOT__ */
			the_command[n-1] = ' ';
			strcpy(&the_command[n], console_buffer);
#endif /* !__U_BOOT__ */
				}
				ଅନ୍ୟଥା {
					the_command[0] = '\n';
					the_command[1] = '\0';
					G.flag_repeat = 0;
				}
		}
		ଯଦି (promptme == 0) {
			the_command[0] = '\n';
			the_command[1] = '\0';
		}
		i->p = console_buffer;
	}
#endif /* __U_BOOT__ */
}
/* This is the magic location that prints prompts
 * and gets data back from the user */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fgetc_interactive(ସଂରଚନା in_str *i)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ch;
#ifndef __U_BOOT__
	/* If it's interactive stdin, get new line. */
	ଯଦି (G_interactive_fd && i->file == G.HFILE_stdin) {
#endif /* !__U_BOOT__ */
#ifndef __U_BOOT__
		/* Returns first char (or EOF), the rest is in i->p[] */
		ch = get_user_ନିବେଶ(i);
#ଅନ୍ୟଥା /* __U_BOOT__ */
		/* Avoid garbage value and make clang happy. */
		ch = 0;
		/*
		 * get_user_input() does not return anything when used in
		 * U-Boot.
		 * So, we need to take the read character from i->p[].
		 */
		get_user_ନିବେଶ(i);
		ଯଦି (i->p && *i->p) {
			ch = *i->p++;
		}
#endif /* __U_BOOT__ */
		G.promptmode = 1; /* PS2 */
		debug_printf_prompt("%s promptmode=%d\n", __func__, G.promptmode);
#ifndef __U_BOOT__
	} ଅନ୍ୟଥା {
		/* Not stdin: script file, sourced file, etc */
		କର ch = hfgetc(i->file); ଯେତେବେଳେ (ch == '\0');
	}
#endif /* !__U_BOOT__ */
	ଫେରନ୍ତୁ ch;
}
#ଅନ୍ୟଥା  /* !INTERACTIVE */
#ifndef __U_BOOT__
ସ୍ଥିର ALWAYS_INLINE ପୂର୍ଣ୍ଣସଂଖ୍ୟା fgetc_interactive(ସଂରଚନା in_str *i)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ch;
	କର ch = hfgetc(i->file); ଯେତେବେଳେ (ch == '\0');
	ଫେରନ୍ତୁ ch;
}
#endif /* !__U_BOOT__ */
#endif  /* !INTERACTIVE */

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା i_getch(ସଂରଚନା in_str *i)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ch;

#ifndef __U_BOOT__
	ଯଦି (!i->file) {
		/* string-based in_str */
		ch = (unsigned ଅକ୍ଷର)*i->p;
		ଯଦି (ch != '\0') {
			i->p++;
			i->last_char = ch;
#ଯଦି ENABLE_HUSH_LINENO_VAR
			ଯଦି (ch == '\n') {
				G.parse_lineno++;
				debug_printf_parse("G.parse_lineno++ = %u\n", G.parse_lineno);
			}
#endif
			ଫେରନ୍ତୁ ch;
		}
		ଫେରନ୍ତୁ EOF;
	}

#endif /* !__U_BOOT__ */
	/* FILE-based in_str */

#ଯଦି ENABLE_FEATURE_EDITING
	/* This can be stdin, check line editing char[] buffer */
	ଯଦି (i->p && *i->p != '\0') {
		ch = (unsigned ଅକ୍ଷର)*i->p++;
		ଯାଅ out;
#ifndef __U_BOOT__
	}
#ଅନ୍ୟଥା /* __U_BOOT__ */
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
	} ଅନ୍ୟଥା ଯଦି (i->p && *i->p == '\0' && G.run_command_flags){
		ଫେରନ୍ତୁ EOF;
	}
#endif /* __U_BOOT__ */
#endif
#ifndef __U_BOOT__
	/* peek_buf[] is an int array, not char. Can contain EOF. */
	ch = i->peek_buf[0];
	ଯଦି (ch != 0) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା ch2 = i->peek_buf[1];
		i->peek_buf[0] = ch2;
		ଯଦି (ch2 == 0) /* very likely, avoid redundant write */
			ଯାଅ out;
		i->peek_buf[1] = 0;
		ଯାଅ out;
	}

#endif /* !__U_BOOT__ */
	ch = fgetc_interactive(i);
 out:
	debug_printf("file_get: got '%c' %d\n", ch, ch);
	i->last_char = ch;
#ଯଦି ENABLE_HUSH_LINENO_VAR
	ଯଦି (ch == '\n') {
		G.parse_lineno++;
		debug_printf_parse("G.parse_lineno++ = %u\n", G.parse_lineno);
	}
#endif
	ଫେରନ୍ତୁ ch;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା i_peek(ସଂରଚନା in_str *i)
{
#ifndef __U_BOOT__
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ch;

	ଯଦି (!i->file) {
		/* string-based in_str */
		/* Doesn't report EOF on NUL. None of the callers care. */
		ଫେରନ୍ତୁ (unsigned ଅକ୍ଷର)*i->p;
	}

	/* FILE-based in_str */

#ଯଦି ENABLE_FEATURE_EDITING && ENABLE_HUSH_INTERACTIVE
	/* This can be stdin, check line editing char[] buffer */
	ଯଦି (i->p && *i->p != '\0')
		ଫେରନ୍ତୁ (unsigned ଅକ୍ଷର)*i->p;
#endif
	/* peek_buf[] is an int array, not char. Can contain EOF. */
	ch = i->peek_buf[0];
	ଯଦି (ch != 0)
		ଫେରନ୍ତୁ ch;

	/* Need to get a new char */
	ch = fgetc_interactive(i);
	debug_printf("file_peek: got '%c' %d\n", ch, ch);

	/* Save it by either rolling back line editing buffer, or in i->peek_buf[0] */
#ଯଦି ENABLE_FEATURE_EDITING && ENABLE_HUSH_INTERACTIVE
	ଯଦି (i->p) {
		i->p -= 1;
		ଫେରନ୍ତୁ ch;
	}
#endif
	i->peek_buf[0] = ch;
	/*i->peek_buf[1] = 0; - already is */
	ଫେରନ୍ତୁ ch;
#ଅନ୍ୟଥା /* __U_BOOT__ */
	/* string-based in_str */
	/* Doesn't report EOF on NUL. None of the callers care. */
	ଫେରନ୍ତୁ (unsigned ଅକ୍ଷର)*i->p;
#endif /* __U_BOOT__ */
}

/* Only ever called if i_peek() was called, and did not return EOF.
 * IOW: we know the previous peek saw an ordinary char, not EOF, not NUL,
 * not end-of-line. Therefore we never need to read a new editing line here.
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା i_peek2(ସଂରଚନା in_str *i)
{
#ifndef __U_BOOT__
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ch;
#endif /* !__U_BOOT__ */

	/* There are two cases when i->p[] buffer exists.
	 * (1) it's a string in_str.
	 * (2) It's a file, and we have a saved line editing buffer.
	 * In both cases, we know that i->p[0] exists and not NUL, and
	 * the peek2 result is in i->p[1].
	 */
	ଯଦି (i->p)
		ଫେରନ୍ତୁ (unsigned ଅକ୍ଷର)i->p[1];

#ifndef __U_BOOT__
	/* Now we know it is a file-based in_str. */

	/* peek_buf[] is an int array, not char. Can contain EOF. */
	/* Is there 2nd char? */
	ch = i->peek_buf[1];
	ଯଦି (ch == 0) {
		/* We did not read it yet, get it now */
		କର ch = hfgetc(i->file); ଯେତେବେଳେ (ch == '\0');
		i->peek_buf[1] = ch;
	}

	debug_printf("file_peek2: got '%c' %d\n", ch, ch);
	ଫେରନ୍ତୁ ch;
#ଅନ୍ୟଥା
	ଫେରନ୍ତୁ 0;
#endif /* __U_BOOT__ */
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା i_getch_and_eat_bkslash_nl(ସଂରଚନା in_str *ନିବେଶ)
{
	ପାଇଁ (;;) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା ch, ch2;

		ch = i_getch(ନିବେଶ);
		ଯଦି (ch != '\\')
			ଫେରନ୍ତୁ ch;
		ch2 = i_peek(ନିବେଶ);
		ଯଦି (ch2 != '\n')
			ଫେରନ୍ତୁ ch;
		/* backslash+newline, skip it */
		i_getch(ନିବେଶ);
	}
}

/* Note: this function _eats_ \<newline> pairs, safe to use plain
 * i_getch() after it instead of i_getch_and_eat_bkslash_nl().
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା i_peek_and_eat_bkslash_nl(ସଂରଚନା in_str *ନିବେଶ)
{
	ପାଇଁ (;;) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା ch, ch2;

		ch = i_peek(ନିବେଶ);
		ଯଦି (ch != '\\')
			ଫେରନ୍ତୁ ch;
		ch2 = i_peek2(ନିବେଶ);
		ଯଦି (ch2 != '\n')
			ଫେରନ୍ତୁ ch;
		/* backslash+newline, skip it */
		i_getch(ନିବେଶ);
		i_getch(ନିବେଶ);
	}
}

#ifndef __U_BOOT__
ସ୍ଥିର ଶୂନ୍ୟ setup_file_in_str(ସଂରଚନା in_str *i, HFILE *fp)
#ଅନ୍ୟଥା /* __U_BOOT__ */
ସ୍ଥିର ଶୂନ୍ୟ setup_file_in_str(ସଂରଚନା in_str *i)
#endif /* __U_BOOT__ */
{
	memset(i, 0, ଆକାର(*i));
#ifndef __U_BOOT__
	i->file = fp;
	/* i->p = NULL; */
#endif /* !__U_BOOT__ */
}

ସ୍ଥିର ଶୂନ୍ୟ setup_string_in_str(ସଂରଚନା in_str *i, ଅଚଳ ଅକ୍ଷର *s)
{
	memset(i, 0, ଆକାର(*i));
	/*i->file = NULL */;
	i->p = s;
}

/*
 * o_string support
 */
#define B_CHUNK  (32 * ଆକାର(ଅକ୍ଷର*))

ସ୍ଥିର ଶୂନ୍ୟ o_reset_to_empty_unquoted(o_string *o)
{
	o->length = 0;
	o->has_quoted_part = 0;
	ଯଦି (o->data)
		o->data[0] = '\0';
}

ସ୍ଥିର ଶୂନ୍ୟ o_free_and_set_NULL(o_string *o)
{
	ମୁକ୍ତ(o->data);
	memset(o, 0, ଆକାର(*o));
}

ସ୍ଥିର ALWAYS_INLINE ଶୂନ୍ୟ o_free(o_string *o)
{
	ମୁକ୍ତ(o->data);
}

ସ୍ଥିର ଶୂନ୍ୟ o_grow_by(o_string *o, ପୂର୍ଣ୍ଣସଂଖ୍ୟା len)
{
	ଯଦି (o->length + len > o->maxlen) {
		o->maxlen += (2 * len) | (B_CHUNK-1);
		o->data = xrealloc(o->data, 1 + o->maxlen);
	}
}

ସ୍ଥିର ଶୂନ୍ୟ o_addchr(o_string *o, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ch)
{
	debug_printf("o_addchr: '%c' o->length=%d o=%p\n", ch, o->length, o);
	ଯଦି (o->length < o->maxlen) {
		/* likely. avoid o_grow_by() call */
 add:
		o->data[o->length] = ch;
		o->length++;
		o->data[o->length] = '\0';
		ଫେରନ୍ତୁ;
	}
	o_grow_by(o, 1);
	ଯାଅ add;
}

#ଯଦି 0
/* Valid only if we know o_string is not empty */
ସ୍ଥିର ଶୂନ୍ୟ o_delchr(o_string *o)
{
	o->length--;
	o->data[o->length] = '\0';
}
#endif

ସ୍ଥିର ଶୂନ୍ୟ o_addblock(o_string *o, ଅଚଳ ଅକ୍ଷର *str, ପୂର୍ଣ୍ଣସଂଖ୍ୟା len)
{
	o_grow_by(o, len);
	((ଅକ୍ଷର*)mempcpy(&o->data[o->length], str, len))[0] = '\0';
	o->length += len;
}

ସ୍ଥିର ଶୂନ୍ୟ o_addstr(o_string *o, ଅଚଳ ଅକ୍ଷର *str)
{
	o_addblock(o, str, strlen(str));
}

#ifndef __U_BOOT__
ସ୍ଥିର ଶୂନ୍ୟ o_addstr_with_NUL(o_string *o, ଅଚଳ ଅକ୍ଷର *str)
{
	o_addblock(o, str, strlen(str) + 1);
}
#endif /* !__U_BOOT__ */

#ଯଦି !BB_MMU
ସ୍ଥିର ଶୂନ୍ୟ nommu_addchr(o_string *o, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ch)
{
	ଯଦି (o)
		o_addchr(o, ch);
}
#ଅନ୍ୟଥା
# define nommu_addchr(o, str) ((ଶୂନ୍ୟ)0)
#endif

#ifndef __U_BOOT__
#ଯଦି ENABLE_HUSH_MODE_X
ସ୍ଥିର ଶୂନ୍ୟ x_mode_addchr(ପୂର୍ଣ୍ଣସଂଖ୍ୟା ch)
{
	o_addchr(&G.x_mode_buf, ch);
}
ସ୍ଥିର ଶୂନ୍ୟ x_mode_addstr(ଅଚଳ ଅକ୍ଷର *str)
{
	o_addstr(&G.x_mode_buf, str);
}
ସ୍ଥିର ଶୂନ୍ୟ x_mode_addblock(ଅଚଳ ଅକ୍ଷର *str, ପୂର୍ଣ୍ଣସଂଖ୍ୟା len)
{
	o_addblock(&G.x_mode_buf, str, len);
}
ସ୍ଥିର ଶୂନ୍ୟ x_mode_prefix(ଶୂନ୍ୟ)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = G.x_mode_depth;
	କର x_mode_addchr('+'); ଯେତେବେଳେ (--n >= 0);
}
ସ୍ଥିର ଶୂନ୍ୟ x_mode_flush(ଶୂନ୍ୟ)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା len = G.x_mode_buf.length;
	ଯଦି (len <= 0)
		ଫେରନ୍ତୁ;
	ଯଦି (G.x_mode_fd > 0) {
		G.x_mode_buf.data[len] = '\n';
		full_write(G.x_mode_fd, G.x_mode_buf.data, len + 1);
	}
	G.x_mode_buf.length = 0;
}
#endif
#endif /* !__U_BOOT__ */

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

#ଯଦି ENABLE_HUSH_BRACE_EXPANSION
# define MAYBE_BRACES "{}"
#ଅନ୍ୟଥା
# define MAYBE_BRACES ""
#endif

/* My analysis of quoting semantics tells me that state information
 * is associated with a destination, not a source.
 */
ସ୍ଥିର ଶୂନ୍ୟ o_addqchr(o_string *o, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ch)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା sz = 1;
	/* '-' is included because of this case:
	 * >filename0 >filename1 >filename9; v='-'; echo filename[0"$v"9]
	 */
	ଅକ୍ଷର *found = strchr("*?[-\\" MAYBE_BRACES, ch);
	ଯଦି (found)
		sz++;
	o_grow_by(o, sz);
	ଯଦି (found) {
		o->data[o->length] = '\\';
		o->length++;
	}
	o->data[o->length] = ch;
	o->length++;
	o->data[o->length] = '\0';
}

ସ୍ଥିର ଶୂନ୍ୟ o_addQchr(o_string *o, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ch)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା sz = 1;
	ଯଦି ((o->o_expflags & EXP_FLAG_ESC_GLOB_CHARS)
	 && strchr("*?[-\\" MAYBE_BRACES, ch)
	) {
		sz++;
		o->data[o->length] = '\\';
		o->length++;
	}
	o_grow_by(o, sz);
	o->data[o->length] = ch;
	o->length++;
	o->data[o->length] = '\0';
}

ସ୍ଥିର ଶୂନ୍ୟ o_addqblock(o_string *o, ଅଚଳ ଅକ୍ଷର *str, ପୂର୍ଣ୍ଣସଂଖ୍ୟା len)
{
	ଯେତେବେଳେ (len) {
		ଅକ୍ଷର ch;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା sz;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା ordinary_cnt = strcspn(str, "*?[-\\" MAYBE_BRACES);
		ଯଦି (ordinary_cnt > len) /* paranoia */
			ordinary_cnt = len;
		o_addblock(o, str, ordinary_cnt);
		ଯଦି (ordinary_cnt == len)
			ଫେରନ୍ତୁ; /* NUL is already added by o_addblock */
		str += ordinary_cnt;
		len -= ordinary_cnt + 1; /* we are processing + 1 char below */

		ch = *str++;
		sz = 1;
		ଯଦି (ch) { /* it is necessarily one of "*?[-\\" MAYBE_BRACES */
			sz++;
			o->data[o->length] = '\\';
			o->length++;
		}
		o_grow_by(o, sz);
		o->data[o->length] = ch;
		o->length++;
	}
	o->data[o->length] = '\0';
}

ସ୍ଥିର ଶୂନ୍ୟ o_addQblock(o_string *o, ଅଚଳ ଅକ୍ଷର *str, ପୂର୍ଣ୍ଣସଂଖ୍ୟା len)
{
	ଯଦି (!(o->o_expflags & EXP_FLAG_ESC_GLOB_CHARS)) {
		o_addblock(o, str, len);
		ଫେରନ୍ତୁ;
	}
	o_addqblock(o, str, len);
}

ସ୍ଥିର ଶୂନ୍ୟ o_addQstr(o_string *o, ଅଚଳ ଅକ୍ଷର *str)
{
	o_addQblock(o, str, strlen(str));
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
#ଯଦି DEBUG_EXPAND || DEBUG_GLOB
ସ୍ଥିର ଶୂନ୍ୟ debug_print_list(ଅଚଳ ଅକ୍ଷର *prefix, o_string *o, ପୂର୍ଣ୍ଣସଂଖ୍ୟା n)
{
	ଅକ୍ଷର **list = (ଅକ୍ଷର**)o->data;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା string_start = ((n + 0xf) & ~0xf) * ଆକାର(list[0]);
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = 0;

	indent();
	fdprintf(2, "%s: list:%p n:%d string_start:%d length:%d maxlen:%d glob:%d quoted:%d escape:%d\n",
			prefix, list, n, string_start, o->length, o->maxlen,
			!!(o->o_expflags & EXP_FLAG_GLOB),
			o->has_quoted_part,
			!!(o->o_expflags & EXP_FLAG_ESC_GLOB_CHARS));
	ଯେତେବେଳେ (i < n) {
		indent();
		fdprintf(2, " list[%d]=%d '%s' %p\n", i, (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(uintptr_t)list[i],
				o->data + (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(uintptr_t)list[i] + string_start,
				o->data + (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(uintptr_t)list[i] + string_start);
		i++;
	}
	ଯଦି (n) {
		ଅଚଳ ଅକ୍ଷର *p = o->data + (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(uintptr_t)list[n - 1] + string_start;
		indent();
#ifndef __U_BOOT__
		fdprintf(2, " total_sz:%ld\n", (long)((p + strlen(p) + 1) - o->data));
#ଅନ୍ୟଥା /* __U_BOOT__ */
		printf(" total_sz:%ld\n", (long)((p + strlen(p) + 1) - o->data));
#endif /* __U_BOOT__ */
	}
}
#ଅନ୍ୟଥା
# define debug_print_list(prefix, o, n) ((ଶୂନ୍ୟ)0)
#endif

/* n = o_save_ptr_helper(str, n) "starts new string" by storing an index value
 * in list[n] so that it points past last stored byte so far.
 * It returns n+1. */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା o_save_ptr_helper(o_string *o, ପୂର୍ଣ୍ଣସଂଖ୍ୟା n)
{
	ଅକ୍ଷର **list = (ଅକ୍ଷର**)o->data;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା string_start;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା string_len;

	ଯଦି (!o->has_empty_slot) {
		string_start = ((n + 0xf) & ~0xf) * ଆକାର(list[0]);
		string_len = o->length - string_start;
		ଯଦି (!(n & 0xf)) { /* 0, 0x10, 0x20...? */
			debug_printf_list("list[%d]=%d string_start=%d (growing)\n", n, string_len, string_start);
			/* list[n] points to string_start, make space for 16 more pointers */
			o->maxlen += 0x10 * ଆକାର(list[0]);
			o->data = xrealloc(o->data, o->maxlen + 1);
			list = (ଅକ୍ଷର**)o->data;
			memmove(list + n + 0x10, list + n, string_len);
			/*
			 * expand_on_ifs() has a "previous argv[] ends in IFS?"
			 * check. (grep for -prev-ifs-check-).
			 * Ensure that argv[-1][last] is not garbage
			 * but zero bytes, to save index check there.
			 */
			list[n + 0x10 - 1] = 0;
			o->length += 0x10 * ଆକାର(list[0]);
		} ଅନ୍ୟଥା {
			debug_printf_list("list[%d]=%d string_start=%d\n",
					n, string_len, string_start);
		}
	} ଅନ୍ୟଥା {
		/* We have empty slot at list[n], reuse without growth */
		string_start = ((n+1 + 0xf) & ~0xf) * ଆକାର(list[0]); /* NB: n+1! */
		string_len = o->length - string_start;
		debug_printf_list("list[%d]=%d string_start=%d (empty slot)\n",
				n, string_len, string_start);
		o->has_empty_slot = 0;
	}
	o->has_quoted_part = 0;
	list[n] = (ଅକ୍ଷର*)(uintptr_t)string_len;
	ଫେରନ୍ତୁ n + 1;
}

/* "What was our last o_save_ptr'ed position (byte offset relative o->data)?" */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା o_get_last_ptr(o_string *o, ପୂର୍ଣ୍ଣସଂଖ୍ୟା n)
{
	ଅକ୍ଷର **list = (ଅକ୍ଷର**)o->data;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା string_start = ((n + 0xf) & ~0xf) * ଆକାର(list[0]);

	ଫେରନ୍ତୁ ((ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(uintptr_t)list[n-1]) + string_start;
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

#ଯଦି ENABLE_HUSH_BRACE_EXPANSION
/* There in a GNU extension, GLOB_BRACE, but it is not usable:
 * first, it processes even {a} (no commas), second,
 * I didn't manage to make it return strings when they don't match
 * existing files. Need to re-implement it.
 */

/* Helper */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା glob_needed(ଅଚଳ ଅକ୍ଷର *s)
{
	ଯେତେବେଳେ (*s) {
		ଯଦି (*s == '\\') {
			ଯଦି (!s[1])
				ଫେରନ୍ତୁ 0;
			s += 2;
			ଜାରି;
		}
		ଯଦି (*s == '*' || *s == '[' || *s == '?' || *s == '{')
			ଫେରନ୍ତୁ 1;
		s++;
	}
	ଫେରନ୍ତୁ 0;
}
/* Return pointer to next closing brace or to comma */
ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *next_brace_sub(ଅଚଳ ଅକ୍ଷର *cp)
{
	unsigned depth = 0;
	cp++;
	ଯେତେବେଳେ (*cp != '\0') {
		ଯଦି (*cp == '\\') {
			ଯଦି (*++cp == '\0')
				ଭାଙ୍ଗ;
			cp++;
			ଜାରି;
		}
		ଯଦି ((*cp == '}' && depth-- == 0) || (*cp == ',' && depth == 0))
			ଭାଙ୍ଗ;
		ଯଦି (*cp++ == '{')
			depth++;
	}

	ଫେରନ୍ତୁ *cp != '\0' ? cp : ଖାଲି;
}
/* Recursive brace globber. Note: may garble pattern[]. */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା glob_brace(ଅକ୍ଷର *pattern, o_string *o, ପୂର୍ଣ୍ଣସଂଖ୍ୟା n)
{
	ଅକ୍ଷର *new_pattern_buf;
	ଅଚଳ ଅକ୍ଷର *begin;
	ଅଚଳ ଅକ୍ଷର *next;
	ଅଚଳ ଅକ୍ଷର *rest;
	ଅଚଳ ଅକ୍ଷର *p;
	size_t rest_len;

	debug_printf_glob("glob_brace('%s')\n", pattern);

	begin = pattern;
	ଯେତେବେଳେ (1) {
		ଯଦି (*begin == '\0')
			ଯାଅ simple_glob;
		ଯଦି (*begin == '{') {
			/* Find the first sub-pattern and at the same time
			 * find the rest after the closing brace */
			next = next_brace_sub(begin);
			ଯଦି (next == ଖାଲି) {
				/* An illegal expression */
				ଯାଅ simple_glob;
			}
			ଯଦି (*next == '}') {
				/* "{abc}" with no commas - illegal
				 * brace expr, disregard and skip it */
				begin = next + 1;
				ଜାରି;
			}
			ଭାଙ୍ଗ;
		}
		ଯଦି (*begin == '\\' && begin[1] != '\0')
			begin++;
		begin++;
	}
	debug_printf_glob("begin:%s\n", begin);
	debug_printf_glob("next:%s\n", next);

	/* Now find the end of the whole brace expression */
	rest = next;
	ଯେତେବେଳେ (*rest != '}') {
		rest = next_brace_sub(rest);
		ଯଦି (rest == ଖାଲି) {
			/* An illegal expression */
			ଯାଅ simple_glob;
		}
		debug_printf_glob("rest:%s\n", rest);
	}
	rest_len = strlen(++rest) + 1;

	/* We are sure the brace expression is well-formed */

	/* Allocate working buffer large enough for our work */
	new_pattern_buf = xmalloc(strlen(pattern));

	/* We have a brace expression.  BEGIN points to the opening {,
	 * NEXT points past the terminator of the first element, and REST
	 * points past the final }.  We will accumulate result names from
	 * recursive runs for each brace alternative in the buffer using
	 * GLOB_APPEND. */

	p = begin + 1;
	ଯେତେବେଳେ (1) {
		/* Construct the new glob expression */
		memcpy(
			mempcpy(
				mempcpy(new_pattern_buf,
					/* We know the prefix for all sub-patterns */
					pattern, begin - pattern),
				p, next - p),
			rest, rest_len);

		/* Note: glob_brace() may garble new_pattern_buf[].
		 * That's why we re-copy prefix every time (1st memcpy above).
		 */
		n = glob_brace(new_pattern_buf, o, n);
		ଯଦି (*next == '}') {
			/* We saw the last entry */
			ଭାଙ୍ଗ;
		}
		p = next + 1;
		next = next_brace_sub(next);
	}
	ମୁକ୍ତ(new_pattern_buf);
	ଫେରନ୍ତୁ n;

 simple_glob:
	{
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା gr;
		glob_t globdata;

		memset(&globdata, 0, ଆକାର(globdata));
		gr = glob(pattern, 0, ଖାଲି, &globdata);
		debug_printf_glob("glob('%s'):%d\n", pattern, gr);
		ଯଦି (gr != 0) {
			ଯଦି (gr == GLOB_NOMATCH) {
				globfree(&globdata);
				/* NB: garbles parameter */
				unbackslash(pattern);
				o_addstr_with_NUL(o, pattern);
				debug_printf_glob("glob pattern '%s' is literal\n", pattern);
				ଫେରନ୍ତୁ o_save_ptr_helper(o, n);
			}
			ଯଦି (gr == GLOB_NOSPACE)
				bb_die_memory_exhausted();
			/* GLOB_ABORTED? Only happens with GLOB_ERR flag,
			 * but we didn't specify it. Paranoia again. */
			bb_error_ସନ୍ଦେଶ_and_die("glob error %d on '%s'", gr, pattern);
		}
		ଯଦି (globdata.gl_pathv && globdata.gl_pathv[0]) {
			ଅକ୍ଷର **argv = globdata.gl_pathv;
			ଯେତେବେଳେ (1) {
				o_addstr_with_NUL(o, *argv);
				n = o_save_ptr_helper(o, n);
				argv++;
				ଯଦି (!*argv)
					ଭାଙ୍ଗ;
			}
		}
		globfree(&globdata);
	}
	ଫେରନ୍ତୁ n;
}
/* Performs globbing on last list[],
 * saving each result as a new list[].
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା perform_glob(o_string *o, ପୂର୍ଣ୍ଣସଂଖ୍ୟା n)
{
	ଅକ୍ଷର *pattern, *copy;

	debug_printf_glob("start perform_glob: n:%d o->data:%p\n", n, o->data);
	ଯଦି (!o->data)
		ଫେରନ୍ତୁ o_save_ptr_helper(o, n);
	pattern = o->data + o_get_last_ptr(o, n);
	debug_printf_glob("glob pattern '%s'\n", pattern);
	ଯଦି (!glob_needed(pattern)) {
		/* unbackslash last string in o in place, fix length */
		o->length = unbackslash(pattern) - o->data;
		debug_printf_glob("glob pattern '%s' is literal\n", pattern);
		ଫେରନ୍ତୁ o_save_ptr_helper(o, n);
	}

	copy = xstrdup(pattern);
	/* "forget" pattern in o */
	o->length = pattern - o->data;
	n = glob_brace(copy, o, n);
	ମୁକ୍ତ(copy);
	ଯଦି (DEBUG_GLOB)
		debug_print_list("perform_glob returning", o, n);
	ଫେରନ୍ତୁ n;
}

#ଅନ୍ୟଥା /* !HUSH_BRACE_EXPANSION */

/* Helper */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା glob_needed(ଅଚଳ ଅକ୍ଷର *s)
{
	ଯେତେବେଳେ (*s) {
		ଯଦି (*s == '\\') {
			ଯଦି (!s[1])
				ଫେରନ୍ତୁ 0;
			s += 2;
			ଜାରି;
		}
		ଯଦି (*s == '*' || *s == '[' || *s == '?')
			ଫେରନ୍ତୁ 1;
		s++;
	}
	ଫେରନ୍ତୁ 0;
}
/* Performs globbing on last list[],
 * saving each result as a new list[].
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା perform_glob(o_string *o, ପୂର୍ଣ୍ଣସଂଖ୍ୟା n)
{
#ifndef __U_BOOT__
	glob_t globdata;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା gr;
#endif /* __U_BOOT__ */
	ଅକ୍ଷର *pattern;

	debug_printf_glob("start perform_glob: n:%d o->data:%p\n", n, o->data);
	ଯଦି (!o->data)
		ଫେରନ୍ତୁ o_save_ptr_helper(o, n);
	pattern = o->data + o_get_last_ptr(o, n);
	debug_printf_glob("glob pattern '%s'\n", pattern);
	ଯଦି (!glob_needed(pattern)) {
#ifndef __U_BOOT__
 literal:
#endif /* __U_BOOT__ */
		/* unbackslash last string in o in place, fix length */
		o->length = unbackslash(pattern) - o->data;
		debug_printf_glob("glob pattern '%s' is literal\n", pattern);
		ଫେରନ୍ତୁ o_save_ptr_helper(o, n);
	}

#ifndef __U_BOOT__
	memset(&globdata, 0, ଆକାର(globdata));
	/* Can't use GLOB_NOCHECK: it does not unescape the string.
	 * If we glob "*.\*" and don't find anything, we need
	 * to fall back to using literal "*.*", but GLOB_NOCHECK
	 * will return "*.\*"!
	 */
	gr = glob(pattern, 0, ଖାଲି, &globdata);
	debug_printf_glob("glob('%s'):%d\n", pattern, gr);
	ଯଦି (gr != 0) {
		ଯଦି (gr == GLOB_NOMATCH) {
			globfree(&globdata);
			ଯାଅ literal;
		}
		ଯଦି (gr == GLOB_NOSPACE)
			bb_die_memory_exhausted();
		/* GLOB_ABORTED? Only happens with GLOB_ERR flag,
		 * but we didn't specify it. Paranoia again. */
		bb_error_ସନ୍ଦେଶ_and_die("glob error %d on '%s'", gr, pattern);
	}
	ଯଦି (globdata.gl_pathv && globdata.gl_pathv[0]) {
		ଅକ୍ଷର **argv = globdata.gl_pathv;
		/* "forget" pattern in o */
		o->length = pattern - o->data;
		ଯେତେବେଳେ (1) {
			o_addstr_with_NUL(o, *argv);
			n = o_save_ptr_helper(o, n);
			argv++;
			ଯଦି (!*argv)
				ଭାଙ୍ଗ;
		}
	}
	globfree(&globdata);
	ଯଦି (DEBUG_GLOB)
		debug_print_list("perform_glob returning", o, n);
	ଫେରନ୍ତୁ n;
#ଅନ୍ୟଥା /* __U_BOOT__ */
	/*
	 * NOTE We only use perform glob to call unbackslash to remove backslash
	 * from string once expanded.
	 * So, it seems OK to return this if no previous return was done.
	 */
	ଫେରନ୍ତୁ o_save_ptr_helper(o, n);
#endif /* __U_BOOT__ */
}

#endif /* !HUSH_BRACE_EXPANSION */

/* If o->o_expflags & EXP_FLAG_GLOB, glob the string so far remembered.
 * Otherwise, just finish current list[] and start new */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା o_save_ptr(o_string *o, ପୂର୍ଣ୍ଣସଂଖ୍ୟା n)
{
	ଯଦି (o->o_expflags & EXP_FLAG_GLOB) {
		/* If o->has_empty_slot, list[n] was already globbed
		 * (if it was requested back then when it was filled)
		 * so don't do that again! */
		ଯଦି (!o->has_empty_slot)
			ଫେରନ୍ତୁ perform_glob(o, n); /* o_save_ptr_helper is inside */
	}
	ଫେରନ୍ତୁ o_save_ptr_helper(o, n);
}

/* "Please convert list[n] to real char* ptrs, and NULL terminate it." */
ସ୍ଥିର ଅକ୍ଷର **o_finalize_list(o_string *o, ପୂର୍ଣ୍ଣସଂଖ୍ୟା n)
{
	ଅକ୍ଷର **list;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା string_start;

	ଯଦି (DEBUG_EXPAND)
		debug_print_list("finalized", o, n);
	debug_printf_expand("finalized n:%d\n", n);
	list = (ଅକ୍ଷର**)o->data;
	string_start = ((n + 0xf) & ~0xf) * ଆକାର(list[0]);
	list[--n] = ଖାଲି;
	ଯେତେବେଳେ (n) {
		n--;
		list[n] = o->data + (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(uintptr_t)list[n] + string_start;
	}
	ଫେରନ୍ତୁ list;
}

ସ୍ଥିର ଶୂନ୍ୟ free_pipe_list(ସଂରଚନା pipe *pi);

/* Returns pi->next - next pipe in the list */
ସ୍ଥିର ସଂରଚନା pipe *free_pipe(ସଂରଚନା pipe *pi)
{
	ସଂରଚନା pipe *next;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

#ifndef __U_BOOT__
	debug_printf_clean("free_pipe (pid %d)\n", getpid());
#endif /* !__U_BOOT__ */
	ପାଇଁ (i = 0; i < pi->ଅଙ୍କ_cmds; i++) {
		ସଂରଚନା command *command;
#ifndef __U_BOOT__
		ସଂରଚନା redir_struct *r, *rnext;
#endif /* !__U_BOOT__ */

		command = &pi->cmds[i];
		debug_printf_clean("  command %d:\n", i);
		ଯଦି (command->argv) {
			ଯଦି (DEBUG_CLEAN) {
				ପୂର୍ଣ୍ଣସଂଖ୍ୟା a;
				ଅକ୍ଷର **p;
				ପାଇଁ (a = 0, p = command->argv; *p; a++, p++) {
					debug_printf_clean("   argv[%d] = %s\n", a, *p);
				}
			}
			free_strings(command->argv);
			//command->argv = NULL;
		}
		/* not "else if": on syntax error, we may have both! */
		ଯଦି (command->group) {
			debug_printf_clean("   begin group (cmd_type:%d)\n",
					command->cmd_type);
			free_pipe_list(command->group);
			debug_printf_clean("   end group\n");
			//command->group = NULL;
		}
		/* else is crucial here.
		 * If group != NULL, child_func is meaningless */
#ଯଦି ENABLE_HUSH_FUNCTIONS
		ଅନ୍ୟଥା ଯଦି (command->child_func) {
			debug_printf_exec("cmd %p releases child func at %p\n", command, command->child_func);
			command->child_func->parent_cmd = ଖାଲି;
		}
#endif
#ଯଦି !BB_MMU
		ମୁକ୍ତ(command->group_as_string);
		//command->group_as_string = NULL;
#endif
#ifndef __U_BOOT__
		ପାଇଁ (r = command->redirects; r; r = rnext) {
			debug_printf_clean("   redirect %d%s",
					r->rd_fd, redir_table[r->rd_type].descrip);
			/* guard against the case >$FOO, where foo is unset or blank */
			ଯଦି (r->rd_filename) {
				debug_printf_clean(" fname:'%s'\n", r->rd_filename);
				ମୁକ୍ତ(r->rd_filename);
				//r->rd_filename = NULL;
			}
			debug_printf_clean(" rd_dup:%d\n", r->rd_dup);
			rnext = r->next;
			ମୁକ୍ତ(r);
		}
		//command->redirects = NULL;
#endif /* !__U_BOOT__ */
	}
	ମୁକ୍ତ(pi->cmds);   /* children are an array, they get freed all at once */
	//pi->cmds = NULL;
#ifndef __U_BOOT__
#ଯଦି ENABLE_HUSH_JOB
	ମୁକ୍ତ(pi->cmdtext);
	//pi->cmdtext = NULL;
#endif
#endif /* !__U_BOOT__ */

	next = pi->next;
	ମୁକ୍ତ(pi);
	ଫେରନ୍ତୁ next;
}

ସ୍ଥିର ଶୂନ୍ୟ free_pipe_list(ସଂରଚନା pipe *pi)
{
	ଯେତେବେଳେ (pi) {
#ଯଦି HAS_KEYWORDS
		debug_printf_clean("pipe reserved word %d\n", pi->res_word);
#endif
		debug_printf_clean("pipe followup code %d\n", pi->followup);
		pi = free_pipe(pi);
	}
}

/*** Parsing routines ***/

#ifndef debug_print_ବୃକ୍ଷ
ସ୍ଥିର ଶୂନ୍ୟ debug_print_ବୃକ୍ଷ(ସଂରଚନା pipe *pi, ପୂର୍ଣ୍ଣସଂଖ୍ୟା lvl)
{
	ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ PIPE[] ALIGN_PTR = {
		[PIPE_SEQ] = "SEQ",
		[PIPE_AND] = "AND",
		[PIPE_OR ] = "OR" ,
		[PIPE_BG ] = "BG" ,
	};
	ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *RES[] = {
		[RES_NONE ] = "NONE" ,
# ଯଦି ENABLE_HUSH_IF
		[RES_IF   ] = "IF"   ,
		[RES_THEN ] = "THEN" ,
		[RES_ELIF ] = "ELIF" ,
		[RES_ELSE ] = "ELSE" ,
		[RES_FI   ] = "FI"   ,
# endif
# ଯଦି ENABLE_HUSH_LOOPS
		[RES_FOR  ] = "FOR"  ,
		[RES_WHILE] = "WHILE",
		[RES_UNTIL] = "UNTIL",
		[RES_DO   ] = "DO"   ,
		[RES_DONE ] = "DONE" ,
# endif
# ଯଦି ENABLE_HUSH_LOOPS || ENABLE_HUSH_CASE
		[RES_IN   ] = "IN"   ,
# endif
# ଯଦି ENABLE_HUSH_CASE
		[RES_CASE ] = "CASE" ,
		[RES_CASE_IN ] = "CASE_IN" ,
		[RES_MATCH] = "MATCH",
		[RES_CASE_BODY] = "CASE_BODY",
		[RES_ESAC ] = "ESAC" ,
# endif
		[RES_XXXX ] = "XXXX" ,
		[RES_SNTX ] = "SNTX" ,
	};
	ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ CMDTYPE[] ALIGN_PTR = {
		"{}",
		"()",
		"[noglob]",
# ଯଦି ENABLE_HUSH_FUNCTIONS
		"func()",
# endif
	};

	ପୂର୍ଣ୍ଣସଂଖ୍ୟା pin, prn;

	pin = 0;
	ଯେତେବେଳେ (pi) {
		fdprintf(2, "%*spipe %d #cmds:%d %sres_word=%s followup=%d %s\n",
				lvl*2, "",
				pin,
				pi->ଅଙ୍କ_cmds,
				(IF_HAS_KEYWORDS(pi->pi_inverted ? "! " :) ""),
				RES[pi->res_word],
				pi->followup, PIPE[pi->followup]
		);
		prn = 0;
		ଯେତେବେଳେ (prn < pi->ଅଙ୍କ_cmds) {
			ସଂରଚନା command *command = &pi->cmds[prn];
			ଅକ୍ଷର **argv = command->argv;

			fdprintf(2, "%*s cmd %d assignment_cnt:%d",
					lvl*2, "", prn,
					command->assignment_cnt);
# ଯଦି ENABLE_HUSH_LINENO_VAR
			fdprintf(2, " LINENO:%u", command->lineno);
# endif
			ଯଦି (command->group) {
				fdprintf(2, " group %s: (argv=%p)%s%s\n",
						CMDTYPE[command->cmd_type],
						argv
# ଯଦି !BB_MMU
						, " group_as_string:", command->group_as_string
# ଅନ୍ୟଥା
						, "", ""
# endif
				);
				debug_print_ବୃକ୍ଷ(command->group, lvl+1);
				prn++;
				ଜାରି;
			}
			ଯଦି (argv) ଯେତେବେଳେ (*argv) {
				fdprintf(2, " '%s'", *argv);
				argv++;
			}
#ifndef __U_BOOT__
			ଯଦି (command->redirects)
				fdprintf(2, " {redir}");
#endif /* __U_BOOT__ */
			fdprintf(2, "\n");
			prn++;
		}
		pi = pi->next;
		pin++;
	}
}
#endif /* debug_print_tree */

ସ୍ଥିର ସଂରଚନା pipe *new_pipe(ଶୂନ୍ୟ)
{
	ସଂରଚନା pipe *pi;
	pi = xzalloc(ଆକାର(ସଂରଚନା pipe));
	/*pi->res_word = RES_NONE; - RES_NONE is 0 anyway */
	ଫେରନ୍ତୁ pi;
}

/* Command (member of a pipe) is complete, or we start a new pipe
 * if ctx->command is NULL.
 * No errors possible here.
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା done_command(ସଂରଚନା parse_context *ctx)
{
	/* The command is really already in the pipe structure, so
	 * advance the pipe counter and make a new, null command. */
	ସଂରଚନା pipe *pi = ctx->pipe;
	ସଂରଚନା command *command = ctx->command;

#ଯଦି 0	/* Instead we emit error message at run time */
	ଯଦି (ctx->pending_redirect) {
		/* For example, "cmd >" (no filename to redirect to) */
		syntax_error("invalid redirect");
		ctx->pending_redirect = ଖାଲି;
	}
#endif

	ଯଦି (command) {
		ଯଦି (IS_NULL_CMD(command)) {
			debug_printf_parse("done_command: skipping null cmd, num_cmds=%d\n", pi->ଅଙ୍କ_cmds);
			ଯାଅ clear_and_ret;
		}
		pi->ଅଙ୍କ_cmds++;
		debug_printf_parse("done_command: ++num_cmds=%d\n", pi->ଅଙ୍କ_cmds);
		//debug_print_tree(ctx->list_head, 20);
	} ଅନ୍ୟଥା {
		debug_printf_parse("done_command: initializing, num_cmds=%d\n", pi->ଅଙ୍କ_cmds);
	}

	/* Only real trickiness here is that the uncommitted
	 * command structure is not counted in pi->num_cmds. */
	pi->cmds = xrealloc(pi->cmds, ଆକାର(*pi->cmds) * (pi->ଅଙ୍କ_cmds+1));
	ctx->command = command = &pi->cmds[pi->ଅଙ୍କ_cmds];
 clear_and_ret:
	memset(command, 0, ଆକାର(*command));
#ଯଦି ENABLE_HUSH_LINENO_VAR
	command->lineno = G.parse_lineno;
	debug_printf_parse("command->lineno = G.parse_lineno (%u)\n", G.parse_lineno);
#endif
	ଫେରନ୍ତୁ pi->ଅଙ୍କ_cmds; /* used only for 0/nonzero check */
}

ସ୍ଥିର ଶୂନ୍ୟ done_pipe(ସଂରଚନା parse_context *ctx, pipe_style type)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା not_null;

	debug_printf_parse("done_pipe entered, followup %d\n", type);
	/* Close previous command */
	not_null = done_command(ctx);
#ଯଦି HAS_KEYWORDS
	ctx->pipe->pi_inverted = ctx->ctx_inverted;
	ctx->ctx_inverted = 0;
	ctx->pipe->res_word = ctx->ctx_res_w;
#endif
	ଯଦି (type == PIPE_BG && ctx->list_head != ctx->pipe) {
		/* Necessary since && and || have precedence over &:
		 * "cmd1 && cmd2 &" must spawn both cmds, not only cmd2,
		 * in a backgrounded subshell.
		 */
		ସଂରଚନା pipe *pi;
		ସଂରଚନା command *command;

		/* Is this actually this construct, all pipes end with && or ||? */
		pi = ctx->list_head;
		ଯେତେବେଳେ (pi != ctx->pipe) {
			ଯଦି (pi->followup != PIPE_AND && pi->followup != PIPE_OR)
				ଯାଅ no_conv;
			pi = pi->next;
		}

		debug_printf_parse("BG with more than one pipe, converting to { p1 &&...pN; } &\n");
		pi->followup = PIPE_SEQ; /* close pN _not_ with "&"! */
		pi = xzalloc(ଆକାର(*pi));
		pi->followup = PIPE_BG;
		pi->ଅଙ୍କ_cmds = 1;
		pi->cmds = xzalloc(ଆକାର(pi->cmds[0]));
		command = &pi->cmds[0];
		ଯଦି (CMD_NORMAL != 0) /* "if xzalloc didn't do that already" */
			command->cmd_type = CMD_NORMAL;
		command->group = ctx->list_head;
#ଯଦି !BB_MMU
		command->group_as_string = xstrndup(
			    ctx->as_string.data,
			    ctx->as_string.length - 1 /* do not copy last char, "&" */
		);
#endif
		/* Replace all pipes in ctx with one newly created */
		ctx->list_head = ctx->pipe = pi;
		/* for cases like "cmd && &", do not be tricked by last command
		 * being null - the entire {...} & is NOT null! */
		not_null = 1;
	} ଅନ୍ୟଥା {
 no_conv:
		ctx->pipe->followup = type;
	}

	/* Without this check, even just <enter> on command line generates
	 * tree of three NOPs (!). Which is harmless but annoying.
	 * IOW: it is safe to do it unconditionally. */
	ଯଦି (not_null
#ଯଦି ENABLE_HUSH_IF
	 || ctx->ctx_res_w == RES_FI
#endif
#ଯଦି ENABLE_HUSH_LOOPS
	 || ctx->ctx_res_w == RES_DONE
	 || ctx->ctx_res_w == RES_FOR
	 || ctx->ctx_res_w == RES_IN
#endif
#ଯଦି ENABLE_HUSH_CASE
	 || ctx->ctx_res_w == RES_ESAC
#endif
	) {
		ସଂରଚନା pipe *new_p;
		debug_printf_parse("done_pipe: adding new pipe: "
#ifndef __U_BOOT__
				"not_null:%d ctx->ctx_res_w:%d\n",
				not_null, ctx->ctx_res_w);
#ଅନ୍ୟଥା /* __U_BOOT__ */
				"not_null:%d\n",
				not_null);
#endif /* __U_BOOT__ */
		new_p = new_pipe();
		ctx->pipe->next = new_p;
		ctx->pipe = new_p;
		/* RES_THEN, RES_DO etc are "sticky" -
		 * they remain set for pipes inside if/while.
		 * This is used to control execution.
		 * RES_FOR and RES_IN are NOT sticky (needed to support
		 * cases where variable or value happens to match a keyword):
		 */
#ଯଦି ENABLE_HUSH_LOOPS
		ଯଦି (ctx->ctx_res_w == RES_FOR
		 || ctx->ctx_res_w == RES_IN)
			ctx->ctx_res_w = RES_NONE;
#endif
#ଯଦି ENABLE_HUSH_CASE
		ଯଦି (ctx->ctx_res_w == RES_MATCH)
			ctx->ctx_res_w = RES_CASE_BODY;
		ଯଦି (ctx->ctx_res_w == RES_CASE)
			ctx->ctx_res_w = RES_CASE_IN;
#endif
		ctx->command = ଖାଲି; /* trick done_command below */
		/* Create the memory for command, roughly:
		 * ctx->pipe->cmds = new struct command;
		 * ctx->command = &ctx->pipe->cmds[0];
		 */
		done_command(ctx);
		//debug_print_tree(ctx->list_head, 10);
	}
	debug_printf_parse("done_pipe return\n");
}

ସ୍ଥିର ଶୂନ୍ୟ initialize_context(ସଂରଚନା parse_context *ctx)
{
	memset(ctx, 0, ଆକାର(*ctx));
	ଯଦି (MAYBE_ASSIGNMENT != 0)
		ctx->is_assignment = MAYBE_ASSIGNMENT;
	ctx->pipe = ctx->list_head = new_pipe();
	/* Create the memory for command, roughly:
	 * ctx->pipe->cmds = new struct command;
	 * ctx->command = &ctx->pipe->cmds[0];
	 */
	done_command(ctx);
}

/* If a reserved word is found and processed, parse context is modified
 * and 1 is returned.
 */
#ଯଦି HAS_KEYWORDS
ସଂରଚନା reserved_combo {
	ଅକ୍ଷର literal[6];
	unsigned ଅକ୍ଷର res;
	unsigned ଅକ୍ଷର assignment_flag;
	uint32_t flag;
};
ଗଣନା {
	FLAG_END   = (1 << RES_NONE ),
# ଯଦି ENABLE_HUSH_IF
	FLAG_IF    = (1 << RES_IF   ),
	FLAG_THEN  = (1 << RES_THEN ),
	FLAG_ELIF  = (1 << RES_ELIF ),
	FLAG_ELSE  = (1 << RES_ELSE ),
	FLAG_FI    = (1 << RES_FI   ),
# endif
# ଯଦି ENABLE_HUSH_LOOPS
	FLAG_FOR   = (1 << RES_FOR  ),
	FLAG_WHILE = (1 << RES_WHILE),
	FLAG_UNTIL = (1 << RES_UNTIL),
	FLAG_DO    = (1 << RES_DO   ),
	FLAG_DONE  = (1 << RES_DONE ),
	FLAG_IN    = (1 << RES_IN   ),
# endif
# ଯଦି ENABLE_HUSH_CASE
	FLAG_MATCH = (1 << RES_MATCH),
	FLAG_ESAC  = (1 << RES_ESAC ),
# endif
	FLAG_START = (1 << RES_XXXX ),
};

ସ୍ଥିର ଅଚଳ ସଂରଚନା reserved_combo* match_reserved_word(o_string *word)
{
	/* Mostly a list of accepted follow-up reserved words.
	 * FLAG_END means we are done with the sequence, and are ready
	 * to turn the compound list into a command.
	 * FLAG_START means the word must start a new compound list.
	 */
	ସ୍ଥିର ଅଚଳ ସଂରଚନା reserved_combo reserved_list[] ALIGN4 = {
# ଯଦି ENABLE_HUSH_IF
		{ "!",     RES_NONE,  NOT_ASSIGNMENT  , 0 },
		{ "if",    RES_IF,    MAYBE_ASSIGNMENT, FLAG_THEN | FLAG_START },
		{ "then",  RES_THEN,  MAYBE_ASSIGNMENT, FLAG_ELIF | FLAG_ELSE | FLAG_FI },
		{ "elif",  RES_ELIF,  MAYBE_ASSIGNMENT, FLAG_THEN },
		{ "else",  RES_ELSE,  MAYBE_ASSIGNMENT, FLAG_FI   },
		{ "fi",    RES_FI,    NOT_ASSIGNMENT  , FLAG_END  },
# endif
# ଯଦି ENABLE_HUSH_LOOPS
		{ "for",   RES_FOR,   NOT_ASSIGNMENT  , FLAG_IN | FLAG_DO | FLAG_START },
		{ "while", RES_WHILE, MAYBE_ASSIGNMENT, FLAG_DO | FLAG_START },
		{ "until", RES_UNTIL, MAYBE_ASSIGNMENT, FLAG_DO | FLAG_START },
		{ "in",    RES_IN,    NOT_ASSIGNMENT  , FLAG_DO   },
		{ "do",    RES_DO,    MAYBE_ASSIGNMENT, FLAG_DONE },
		{ "done",  RES_DONE,  NOT_ASSIGNMENT  , FLAG_END  },
# endif
# ଯଦି ENABLE_HUSH_CASE
		{ "case",  RES_CASE,  NOT_ASSIGNMENT  , FLAG_MATCH | FLAG_START },
		{ "esac",  RES_ESAC,  NOT_ASSIGNMENT  , FLAG_END  },
# endif
	};
	ଅଚଳ ସଂରଚନା reserved_combo *r;

	ପାଇଁ (r = reserved_list; r < reserved_list + ARRAY_SIZE(reserved_list); r++) {
		ଯଦି (strcmp(word->data, r->literal) == 0)
			ଫେରନ୍ତୁ r;
	}
	ଫେରନ୍ତୁ ଖାଲି;
}
/* Return NULL: not a keyword, else: keyword
 */
ସ୍ଥିର ଅଚଳ ସଂରଚନା reserved_combo* reserved_word(ସଂରଚନା parse_context *ctx)
{
# ଯଦି ENABLE_HUSH_CASE
	ସ୍ଥିର ଅଚଳ ସଂରଚନା reserved_combo reserved_match = {
		"",        RES_MATCH, NOT_ASSIGNMENT , FLAG_MATCH | FLAG_ESAC
	};
# endif
	ଅଚଳ ସଂରଚନା reserved_combo *r;

	ଯଦି (ctx->word.has_quoted_part)
		ଫେରନ୍ତୁ 0;
	r = match_reserved_word(&ctx->word);
	ଯଦି (!r)
		ଫେରନ୍ତୁ r; /* NULL */

	debug_printf("found reserved word %s, res %d\n", r->literal, r->res);
# ଯଦି ENABLE_HUSH_CASE
	ଯଦି (r->res == RES_IN && ctx->ctx_res_w == RES_CASE_IN) {
		/* "case word IN ..." - IN part starts first MATCH part */
		r = &reserved_match;
	} ଅନ୍ୟଥା
# endif
	ଯଦି (r->flag == 0) { /* '!' */
		ଯଦି (ctx->ctx_inverted) { /* bash doesn't accept '! ! true' */
			syntax_error("! ! command");
			ctx->ctx_res_w = RES_SNTX;
		}
		ctx->ctx_inverted = 1;
		ଫେରନ୍ତୁ r;
	}
	ଯଦି (r->flag & FLAG_START) {
		ସଂରଚନା parse_context *old;

		old = xmemdup(ctx, ଆକାର(*ctx));
		debug_printf_parse("push stack %p\n", old);
		initialize_context(ctx);
		ctx->ଗଦା = old;
	} ଅନ୍ୟଥା ଯଦି (/*ctx->ctx_res_w == RES_NONE ||*/ !(ctx->old_flag & (1 << r->res))) {
		syntax_error_at(ctx->word.data);
		ctx->ctx_res_w = RES_SNTX;
		ଫେରନ୍ତୁ r;
	} ଅନ୍ୟଥା {
		/* "{...} fi" is ok. "{...} if" is not
		 * Example:
		 * if { echo foo; } then { echo bar; } fi */
		ଯଦି (ctx->command->group)
			done_pipe(ctx, PIPE_SEQ);
	}

	ctx->ctx_res_w = r->res;
	ctx->old_flag = r->flag;
	ctx->is_assignment = r->assignment_flag;
	debug_printf_parse("ctx->is_assignment='%s'\n", assignment_flag[ctx->is_assignment]);

	ଯଦି (ctx->old_flag & FLAG_END) {
		ସଂରଚନା parse_context *old;

		done_pipe(ctx, PIPE_SEQ);
		debug_printf_parse("pop stack %p\n", ctx->ଗଦା);
		old = ctx->ଗଦା;
		old->command->group = ctx->list_head;
		old->command->cmd_type = CMD_NORMAL;
# ଯଦି !BB_MMU
		/* At this point, the compound command's string is in
		 * ctx->as_string... except for the leading keyword!
		 * Consider this example: "echo a | if true; then echo a; fi"
		 * ctx->as_string will contain "true; then echo a; fi",
		 * with "if " remaining in old->as_string!
		 */
		{
			ଅକ୍ଷର *str;
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା len = old->as_string.length;
			/* Concatenate halves */
			o_addstr(&old->as_string, ctx->as_string.data);
			o_free(&ctx->as_string);
			/* Find where leading keyword starts in first half */
			str = old->as_string.data + len;
			ଯଦି (str > old->as_string.data)
				str--; /* skip whitespace after keyword */
			ଯେତେବେଳେ (str > old->as_string.data && isalpha(str[-1]))
				str--;
			/* Ugh, we're done with this horrid hack */
			old->command->group_as_string = xstrdup(str);
			debug_printf_parse("pop, remembering as:'%s'\n",
					old->command->group_as_string);
		}
# endif
		*ctx = *old;   /* physical copy */
		ମୁକ୍ତ(old);
	}
	ଫେରନ୍ତୁ r;
}
#endif /* HAS_KEYWORDS */

/* Word is complete, look at it and update parsing context.
 * Normal return is 0. Syntax errors return 1.
 * Note: on return, word is reset, but not o_free'd!
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା done_word(ସଂରଚନା parse_context *ctx)
{
	ସଂରଚନା command *command = ctx->command;

	debug_printf_parse("done_word entered: '%s' %p\n", ctx->word.data, command);
	ଯଦି (ctx->word.length == 0 && !ctx->word.has_quoted_part) {
		debug_printf_parse("done_word return 0: true null, ignored\n");
		ଫେରନ୍ତୁ 0;
	}

#ifndef __U_BOOT__
	ଯଦି (ctx->pending_redirect) {
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

		ctx->pending_redirect->rd_filename = xstrdup(ctx->word.data);
		/* Cater for >\file case:
		 * >\a creates file a; >\\a, >"\a", >"\\a" create file \a
		 * Same with heredocs:
		 * for <<\H delim is H; <<\\H, <<"\H", <<"\\H" - \H
		 */
		ଯଦି (ctx->pending_redirect->rd_type == REDIRECT_HEREDOC) {
			unbackslash(ctx->pending_redirect->rd_filename);
			/* Is it <<"HEREDOC"? */
			ଯଦି (ctx->word.has_quoted_part) {
				ctx->pending_redirect->rd_dup |= HEREDOC_QUOTED;
			}
		}
		debug_printf_parse("word stored in rd_filename: '%s'\n", ctx->word.data);
		ctx->pending_redirect = ଖାଲି;
	} ଅନ୍ୟଥା {
#endif /* !__U_BOOT__ */
#ଯଦି HAS_KEYWORDS
# ଯଦି ENABLE_HUSH_CASE
		ଯଦି (ctx->ctx_dsemicolon
		 && strcmp(ctx->word.data, "esac") != 0 /* not "... pattern) cmd;; esac" */
		) {
			/* already done when ctx_dsemicolon was set to 1: */
			/* ctx->ctx_res_w = RES_MATCH; */
			ctx->ctx_dsemicolon = 0;
		} ଅନ୍ୟଥା
# endif
# ଯଦି defined(CMD_TEST2_SINGLEWORD_NOGLOB)
		ଯଦି (command->cmd_type == CMD_TEST2_SINGLEWORD_NOGLOB
		 && strcmp(ctx->word.data, "]]") == 0
		) {
			/* allow "[[ ]] >file" etc */
			command->cmd_type = CMD_SINGLEWORD_NOGLOB;
		} ଅନ୍ୟଥା
# endif
		ଯଦି (!command->argv /* if it's the first word... */
# ଯଦି ENABLE_HUSH_LOOPS
		 && ctx->ctx_res_w != RES_FOR /* ...not after FOR or IN */
		 && ctx->ctx_res_w != RES_IN
# endif
# ଯଦି ENABLE_HUSH_CASE
		 && ctx->ctx_res_w != RES_CASE
# endif
		) {
			ଅଚଳ ସଂରଚନା reserved_combo *reserved;
			reserved = reserved_word(ctx);
			debug_printf_parse("checking for reserved-ness: %d\n", !!reserved);
			ଯଦି (reserved) {
# ଯଦି ENABLE_HUSH_LINENO_VAR
/* Case:
 * "while ...; do
 *	cmd ..."
 * If we don't close the pipe _now_, immediately after "do", lineno logic
 * sees "cmd" as starting at "do" - i.e., at the previous line.
 */
				ଯଦି (0
				 IF_HUSH_IF(|| reserved->res == RES_THEN)
				 IF_HUSH_IF(|| reserved->res == RES_ELIF)
				 IF_HUSH_IF(|| reserved->res == RES_ELSE)
				 IF_HUSH_LOOPS(|| reserved->res == RES_DO)
				) {
					done_pipe(ctx, PIPE_SEQ);
				}
# endif
				o_reset_to_empty_unquoted(&ctx->word);
				debug_printf_parse("done_word return %d\n",
						(ctx->ctx_res_w == RES_SNTX));
				ଫେରନ୍ତୁ (ctx->ctx_res_w == RES_SNTX);
			}
# ଯଦି defined(CMD_TEST2_SINGLEWORD_NOGLOB)
			ଯଦି (strcmp(ctx->word.data, "[[") == 0) {
				command->cmd_type = CMD_TEST2_SINGLEWORD_NOGLOB;
			} ଅନ୍ୟଥା
# endif
# ଯଦି defined(CMD_SINGLEWORD_NOGLOB)
			ଯଦି (0
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
			 IF_HUSH_LOCAL(   || strcmp(ctx->word.data, "local") == 0)
			 IF_HUSH_EXPORT(  || strcmp(ctx->word.data, "export") == 0)
			 IF_HUSH_READONLY(|| strcmp(ctx->word.data, "readonly") == 0)
			) {
				command->cmd_type = CMD_SINGLEWORD_NOGLOB;
			}
# ଅନ୍ୟଥା
			{ /* empty block to pair "if ... else" */ }
# endif
		}
#endif /* HAS_KEYWORDS */

		ଯଦି (command->group) {
			/* "{ echo foo; } echo bar" - bad */
			syntax_error_at(ctx->word.data);
			debug_printf_parse("done_word return 1: syntax error, "
					"groups and arglists don't mix\n");
			ଫେରନ୍ତୁ 1;
		}

		/* If this word wasn't an assignment, next ones definitely
		 * can't be assignments. Even if they look like ones. */
		ଯଦି (ctx->is_assignment != DEFINITELY_ASSIGNMENT
		 && ctx->is_assignment != WORD_IS_KEYWORD
		) {
			ctx->is_assignment = NOT_ASSIGNMENT;
		} ଅନ୍ୟଥା {
			ଯଦି (ctx->is_assignment == DEFINITELY_ASSIGNMENT) {
				command->assignment_cnt++;
				debug_printf_parse("++assignment_cnt=%d\n", command->assignment_cnt);
			}
			debug_printf_parse("ctx->is_assignment was:'%s'\n", assignment_flag[ctx->is_assignment]);
			ctx->is_assignment = MAYBE_ASSIGNMENT;
		}
		debug_printf_parse("ctx->is_assignment='%s'\n", assignment_flag[ctx->is_assignment]);
		command->argv = add_string_to_strings(command->argv, xstrdup(ctx->word.data));
#ifdef __U_BOOT__
		command->argc++;
#endif /* __U_BOOT__ */
		debug_print_strings("word appended to argv", command->argv);

#ifndef __U_BOOT__
	}
#endif /* !__U_BOOT__ */

#ଯଦି ENABLE_HUSH_LOOPS
	ଯଦି (ctx->ctx_res_w == RES_FOR) {
		ଯଦି (ctx->word.has_quoted_part
		 || endofname(command->argv[0])[0] != '\0'
		) {
			/* bash says just "not a valid identifier" */
			syntax_error("bad for loop variable");
			ଫେରନ୍ତୁ 1;
		}
		/* Force FOR to have just one word (variable name) */
		/* NB: basically, this makes hush see "for v in ..."
		 * syntax as if it is "for v; in ...". FOR and IN become
		 * two pipe structs in parse tree. */
		done_pipe(ctx, PIPE_SEQ);
	}
#endif
#ଯଦି ENABLE_HUSH_CASE
	/* Force CASE to have just one word */
	ଯଦି (ctx->ctx_res_w == RES_CASE) {
		done_pipe(ctx, PIPE_SEQ);
	}
#endif

	o_reset_to_empty_unquoted(&ctx->word);

	debug_printf_parse("done_word return 0\n");
	ଫେରନ୍ତୁ 0;
}

#ifndef __U_BOOT__
/* Peek ahead in the input to find out if we have a "&n" construct,
 * as in "2>&1", that represents duplicating a file descriptor.
 * Return:
 * REDIRFD_CLOSE if >&- "close fd" construct is seen,
 * REDIRFD_SYNTAX_ERR if syntax error,
 * REDIRFD_TO_FILE if no & was seen,
 * or the number found.
 */
#ଯଦି BB_MMU
#define parse_redir_ଡାହାଣ_fd(as_string, ନିବେଶ) \
	parse_redir_ଡାହାଣ_fd(ନିବେଶ)
#endif
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା parse_redir_ଡାହାଣ_fd(o_string *as_string, ସଂରଚନା in_str *ନିବେଶ)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ch, d, ok;

	ch = i_peek(ନିବେଶ);
	ଯଦି (ch != '&')
		ଫେରନ୍ତୁ REDIRFD_TO_FILE;

	ch = i_getch(ନିବେଶ);  /* get the & */
	nommu_addchr(as_string, ch);
	ch = i_peek(ନିବେଶ);
	ଯଦି (ch == '-') {
		ch = i_getch(ନିବେଶ);
		nommu_addchr(as_string, ch);
		ଫେରନ୍ତୁ REDIRFD_CLOSE;
	}
	d = 0;
	ok = 0;
	ଯେତେବେଳେ (ch != EOF && isdigit(ch)) {
		d = d*10 + (ch-'0');
		ok = 1;
		ch = i_getch(ନିବେଶ);
		nommu_addchr(as_string, ch);
		ch = i_peek(ନିବେଶ);
	}
	ଯଦି (ok) ଫେରନ୍ତୁ d;

//TODO: this is the place to catch ">&file" bashism (redirect both fd 1 and 2)

	bb_simple_error_ସନ୍ଦେଶ("ambiguous redirect");
	ଫେରନ୍ତୁ REDIRFD_SYNTAX_ERR;
}

/* Return code is 0 normal, 1 if a syntax error is detected
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା parse_redirect(ସଂରଚନା parse_context *ctx,
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd,
		redir_type style,
		ସଂରଚନା in_str *ନିବେଶ)
{
	ସଂରଚନା command *command = ctx->command;
	ସଂରଚନା redir_struct *redir;
	ସଂରଚନା redir_struct **redirp;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା dup_ଅଙ୍କ;

	dup_ଅଙ୍କ = REDIRFD_TO_FILE;
	ଯଦି (style != REDIRECT_HEREDOC) {
		/* Check for a '>&1' type redirect */
		dup_ଅଙ୍କ = parse_redir_ଡାହାଣ_fd(&ctx->as_string, ନିବେଶ);
		ଯଦି (dup_ଅଙ୍କ == REDIRFD_SYNTAX_ERR)
			ଫେରନ୍ତୁ 1;
	} ଅନ୍ୟଥା {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା ch = i_peek_and_eat_bkslash_nl(ନିବେଶ);
		dup_ଅଙ୍କ = (ch == '-'); /* HEREDOC_SKIPTABS bit is 1 */
		ଯଦି (dup_ଅଙ୍କ) { /* <<-... */
			ch = i_getch(ନିବେଶ);
			nommu_addchr(&ctx->as_string, ch);
			ch = i_peek(ନିବେଶ);
		}
	}

	ଯଦି (style == REDIRECT_OVERWRITE && dup_ଅଙ୍କ == REDIRFD_TO_FILE) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା ch = i_peek_and_eat_bkslash_nl(ନିବେଶ);
		ଯଦି (ch == '|') {
			/* >|FILE redirect ("clobbering" >).
			 * Since we do not support "set -o noclobber" yet,
			 * >| and > are the same for now. Just eat |.
			 */
			ch = i_getch(ନିବେଶ);
			nommu_addchr(&ctx->as_string, ch);
		}
	}

	/* Create a new redir_struct and append it to the linked list */
	redirp = &command->redirects;
	ଯେତେବେଳେ ((redir = *redirp) != ଖାଲି) {
		redirp = &(redir->next);
	}
	*redirp = redir = xzalloc(ଆକାର(*redir));
	/* redir->next = NULL; */
	/* redir->rd_filename = NULL; */
	redir->rd_type = style;
	redir->rd_fd = (fd == -1) ? redir_table[style].default_fd : fd;

	debug_printf_parse("redirect type %d %s\n", redir->rd_fd,
				redir_table[style].descrip);

	redir->rd_dup = dup_ଅଙ୍କ;
	ଯଦି (style != REDIRECT_HEREDOC && dup_ଅଙ୍କ != REDIRFD_TO_FILE) {
		/* Erik had a check here that the file descriptor in question
		 * is legit; I postpone that to "run time"
		 * A "-" representation of "close me" shows up as a -3 here */
		debug_printf_parse("duplicating redirect '%d>&%d'\n",
				redir->rd_fd, redir->rd_dup);
	} ଅନ୍ୟଥା {
#ଯଦି 0		/* Instead we emit error message at run time */
		ଯଦି (ctx->pending_redirect) {
			/* For example, "cmd > <file" */
			syntax_error("invalid redirect");
		}
#endif
		/* Set ctx->pending_redirect, so we know what to do at the
		 * end of the next parsed word. */
		ctx->pending_redirect = redir;
	}
	ଫେରନ୍ତୁ 0;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା redirect_opt_ଅଙ୍କ(o_string *o)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଅଙ୍କ;

	ଯଦି (o->data == ଖାଲି)
		ଫେରନ୍ତୁ -1;
	ଅଙ୍କ = bb_strtou(o->data, ଖାଲି, 10);
	ଯଦି (errno || ଅଙ୍କ < 0)
		ଫେରନ୍ତୁ -1;
	o_reset_to_empty_unquoted(o);
	ଫେରନ୍ତୁ ଅଙ୍କ;
}

#ଯଦି BB_MMU
#define fetch_till_str(as_string, ନିବେଶ, word, skip_tabs) \
	fetch_till_str(ନିବେଶ, word, skip_tabs)
#endif
ସ୍ଥିର ଅକ୍ଷର *fetch_till_str(o_string *as_string,
		ସଂରଚନା in_str *ନିବେଶ,
		ଅଚଳ ଅକ୍ଷର *word,
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା heredoc_flags)
{
	o_string heredoc = NULL_O_STRING;
	unsigned past_EOL;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା prev = 0; /* not \ */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ch;

	/* Starting with "" is necessary for this case:
	 * cat <<EOF
	 *
	 * xxx
	 * EOF
	 */
	heredoc.data = xzalloc(1); /* start as "", not as NULL */

	ଯାଅ jump_in;

	ଯେତେବେଳେ (1) {
		ch = i_getch(ନିବେଶ);
		ଯଦି (ch != EOF)
			nommu_addchr(as_string, ch);
		ଯଦି (ch == '\n' || ch == EOF) {
 check_heredoc_end:
#ifndef __U_BOOT__
			ଯଦି ((heredoc_flags & HEREDOC_QUOTED) || prev != '\\') {
#ଅନ୍ୟଥା /* __U_BOOT__ */
			ଯଦି (prev != '\\') {
#endif
				/* End-of-line, and not a line continuation */
				ଯଦି (strcmp(heredoc.data + past_EOL, word) == 0) {
					heredoc.data[past_EOL] = '\0';
					debug_printf_heredoc("parsed '%s' heredoc '%s'\n", word, heredoc.data);
					ଫେରନ୍ତୁ heredoc.data;
				}
				ଯଦି (ch == '\n') {
					/* This is a new line.
					 * Remember position and backslash-escaping status.
					 */
					o_addchr(&heredoc, ch);
					prev = ch;
 jump_in:
					past_EOL = heredoc.length;
					/* Get 1st char of next line, possibly skipping leading tabs */
					କର {
						ch = i_getch(ନିବେଶ);
						ଯଦି (ch != EOF)
							nommu_addchr(as_string, ch);
#ifndef __U_BOOT__
					} ଯେତେବେଳେ ((heredoc_flags & HEREDOC_SKIPTABS) && ch == '\t');
#ଅନ୍ୟଥା /* __U_BOOT__ */
				} ଯେତେବେଳେ (ch == '\t');
#endif
					/* If this immediately ended the line,
					 * go back to end-of-line checks.
					 */
					ଯଦି (ch == '\n')
						ଯାଅ check_heredoc_end;
				}
			} ଅନ୍ୟଥା {
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
				heredoc.data[--heredoc.length] = '\0';
				prev = 0; /* not '\' */
				ଜାରି;
			}
		}
		ଯଦି (ch == EOF) {
			o_free(&heredoc);
			ଫେରନ୍ତୁ ଖାଲି; /* error */
		}
		o_addchr(&heredoc, ch);
		nommu_addchr(as_string, ch);
		ଯଦି (prev == '\\' && ch == '\\')
			/* Correctly handle foo\\<eol> (not a line cont.) */
			prev = 0; /* not '\' */
		ଅନ୍ୟଥା
			prev = ch;
	}
}
#endif /* !__U_BOOT__ */

/* Look at entire parse tree for not-yet-loaded REDIRECT_HEREDOCs
 * and load them all. There should be exactly heredoc_cnt of them.
 */
#ଯଦି BB_MMU
#define fetch_heredocs(as_string, pi, heredoc_cnt, ନିବେଶ) \
	fetch_heredocs(pi, heredoc_cnt, ନିବେଶ)
#endif
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fetch_heredocs(o_string *as_string, ସଂରଚନା pipe *pi, ପୂର୍ଣ୍ଣସଂଖ୍ୟା heredoc_cnt, ସଂରଚନା in_str *ନିବେଶ)
{
	ଯେତେବେଳେ (pi && heredoc_cnt) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
		ସଂରଚନା command *cmd = pi->cmds;

		debug_printf_heredoc("fetch_heredocs: num_cmds:%d cmd argv0:'%s'\n",
				pi->ଅଙ୍କ_cmds,
				cmd->argv ? cmd->argv[0] : "NONE"
		);
		ପାଇଁ (i = 0; i < pi->ଅଙ୍କ_cmds; i++) {
#ifndef __U_BOOT__
			ସଂରଚନା redir_struct *redir = cmd->redirects;

#endif /* !__U_BOOT__ */
			debug_printf_heredoc("fetch_heredocs: %d cmd argv0:'%s'\n",
					i, cmd->argv ? cmd->argv[0] : "NONE");
#ifndef __U_BOOT__
			ଯେତେବେଳେ (redir) {
				ଯଦି (redir->rd_type == REDIRECT_HEREDOC) {
					ଅକ୍ଷର *p;

					redir->rd_type = REDIRECT_HEREDOC2;
					/* redir->rd_dup is (ab)used to indicate <<- */
					p = fetch_till_str(as_string, ନିବେଶ,
							redir->rd_filename, redir->rd_dup);
					ଯଦି (!p) {
						syntax_error("unexpected EOF in here document");
						ଫେରନ୍ତୁ -1;
					}
					ମୁକ୍ତ(redir->rd_filename);
					redir->rd_filename = p;
					heredoc_cnt--;
				}
				redir = redir->next;
			}
#endif /* !__U_BOOT__ */
			ଯଦି (cmd->group) {
				//bb_error_msg("%s:%u heredoc_cnt:%d", __func__, __LINE__, heredoc_cnt);
				heredoc_cnt = fetch_heredocs(as_string, cmd->group, heredoc_cnt, ନିବେଶ);
				//bb_error_msg("%s:%u heredoc_cnt:%d", __func__, __LINE__, heredoc_cnt);
				ଯଦି (heredoc_cnt < 0)
					ଫେରନ୍ତୁ heredoc_cnt; /* error */
			}
			cmd++;
		}
		pi = pi->next;
	}
	ଫେରନ୍ତୁ heredoc_cnt;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା run_list(ସଂରଚନା pipe *pi);
#ଯଦି BB_MMU
#define parse_stream(pstring, heredoc_cnt_ptr, ନିବେଶ, end_trigger) \
	parse_stream(heredoc_cnt_ptr, ନିବେଶ, end_trigger)
#endif
ସ୍ଥିର ସଂରଚନା pipe *parse_stream(ଅକ୍ଷର **pstring,
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା *heredoc_cnt_ptr,
		ସଂରଚନା in_str *ନିବେଶ,
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା end_trigger);

/* Returns number of heredocs not yet consumed,
 * or -1 on error.
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା parse_group(ସଂରଚନା parse_context *ctx,
		ସଂରଚନା in_str *ନିବେଶ, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ch)
{
	/* ctx->word contains characters seen prior to ( or {.
	 * Typically it's empty, but for function defs,
	 * it contains function name (without '()'). */
#ଯଦି BB_MMU
# define as_string ଖାଲି
#ଅନ୍ୟଥା
	ଅକ୍ଷର *as_string = ଖାଲି;
#endif
	ସଂରଚନା pipe *pipe_list;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା heredoc_cnt = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା endch;
	ସଂରଚନା command *command = ctx->command;

	debug_printf_parse("parse_group entered\n");
#ଯଦି ENABLE_HUSH_FUNCTIONS
	ଯଦି (ch == '(' && !ctx->word.has_quoted_part) {
		ଯଦି (ctx->word.length)
			ଯଦି (done_word(ctx))
				ଫେରନ୍ତୁ -1;
		ଯଦି (!command->argv)
			ଯାଅ skip; /* (... */
		ଯଦି (command->argv[1]) { /* word word ... (... */
			syntax_error_unexpected_ch('(');
			ଫେରନ୍ତୁ -1;
		}
		/* it is "word(..." or "word (..." */
		କର
			ch = i_getch(ନିବେଶ);
		ଯେତେବେଳେ (ch == ' ' || ch == '\t');
		ଯଦି (ch != ')') {
			syntax_error_unexpected_ch(ch);
			ଫେରନ୍ତୁ -1;
		}
		nommu_addchr(&ctx->as_string, ch);
		କର
			ch = i_getch(ନିବେଶ);
		ଯେତେବେଳେ (ch == ' ' || ch == '\t' || ch == '\n');
		ଯଦି (ch != '{' && ch != '(') {
			syntax_error_unexpected_ch(ch);
			ଫେରନ୍ତୁ -1;
		}
//bash allows functions named "123", "..", "return"!
//		if (endofname(command->argv[0])[0] != '\0') {
//			syntax_error("bad function name");
//			return -1;
//		}
		nommu_addchr(&ctx->as_string, ch);
		command->cmd_type = CMD_FUNCDEF;
		ଯାଅ skip;
	}
#endif

#ଯଦି 0 /* Prevented by caller */
	ଯଦି (command->argv /* word [word]{... */
	 || ctx->word.length /* word{... */
	 || ctx->word.has_quoted_part /* ""{... */
	) {
		syntax_error(ଖାଲି);
		debug_printf_parse("parse_group return -1: "
			"syntax error, groups and arglists don't mix\n");
		ଫେରନ୍ତୁ -1;
	}
#endif

#ifndef __U_BOOT__
 IF_HUSH_FUNCTIONS(skip:)
#endif /* !__U_BOOT__ */

	endch = '}';
	ଯଦି (ch == '(') {
		endch = ')';
#ifndef __U_BOOT__
		IF_HUSH_FUNCTIONS(ଯଦି (command->cmd_type != CMD_FUNCDEF))
			command->cmd_type = CMD_SUBSHELL;
#endif /* !__U_BOOT__ */
	} ଅନ୍ୟଥା {
		/* bash does not allow "{echo...", requires whitespace */
		ch = i_peek(ନିବେଶ);
		ଯଦି (ch != ' ' && ch != '\t' && ch != '\n'
		 && ch != '('	/* but "{(..." is allowed (without whitespace) */
		) {
			syntax_error_unexpected_ch(ch);
			ଫେରନ୍ତୁ -1;
		}
		ଯଦି (ch != '(') {
			ch = i_getch(ନିବେଶ);
			nommu_addchr(&ctx->as_string, ch);
		}
	}

	debug_printf_heredoc("calling parse_stream, heredoc_cnt:%d\n", heredoc_cnt);
	pipe_list = parse_stream(&as_string, &heredoc_cnt, ନିବେଶ, endch);
	debug_printf_heredoc("parse_stream returned: heredoc_cnt:%d\n", heredoc_cnt);
#ଯଦି !BB_MMU
	ଯଦି (as_string)
		o_addstr(&ctx->as_string, as_string);
#endif

	/* empty ()/{} or parse error? */
	ଯଦି (!pipe_list || pipe_list == ERR_PTR) {
		/* parse_stream already emitted error msg */
		ଯଦି (!BB_MMU)
			ମୁକ୍ତ(as_string);
		debug_printf_parse("parse_group return -1: "
			"parse_stream returned %p\n", pipe_list);
		ଫେରନ୍ତୁ -1;
	}
#ଯଦି !BB_MMU
	as_string[strlen(as_string) - 1] = '\0'; /* plink ')' or '}' */
	command->group_as_string = as_string;
	debug_printf_parse("end of group, remembering as:'%s'\n",
			command->group_as_string);
#endif

#ଯଦି ENABLE_HUSH_FUNCTIONS
	/* Convert "f() (cmds)" to "f() {(cmds)}" */
	ଯଦି (command->cmd_type == CMD_FUNCDEF && endch == ')') {
		ସଂରଚନା command *cmd2;

		cmd2 = xzalloc(ଆକାର(*cmd2));
		cmd2->cmd_type = CMD_SUBSHELL;
		cmd2->group = pipe_list;
# ଯଦି !BB_MMU
//UNTESTED!
		cmd2->group_as_string = command->group_as_string;
		command->group_as_string = xasprintf("(%s)", command->group_as_string);
# endif

		pipe_list = new_pipe();
		pipe_list->cmds = cmd2;
		pipe_list->ଅଙ୍କ_cmds = 1;
	}
#endif

	command->group = pipe_list;

	debug_printf_parse("parse_group return %d\n", heredoc_cnt);
	ଫେରନ୍ତୁ heredoc_cnt;
	/* command remains "open", available for possible redirects */
#undef as_string
}

#ifndef __U_BOOT__
#ଯଦି ENABLE_HUSH_TICK || ENABLE_FEATURE_SH_MATH || ENABLE_HUSH_DOLLAR_OPS
/* Subroutines for copying $(...) and `...` things */
/* '...' */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା add_till_single_quote(o_string *dest, ସଂରଚନା in_str *ନିବେଶ)
{
	ଯେତେବେଳେ (1) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା ch = i_getch(ନିବେଶ);
		ଯଦି (ch == EOF) {
			syntax_error_unterm_ch('\'');
			ଫେରନ୍ତୁ 0;
		}
		ଯଦି (ch == '\'')
			ଫେରନ୍ତୁ 1;
		o_addchr(dest, ch);
	}
}
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା add_till_single_quote_dquoted(o_string *dest, ସଂରଚନା in_str *ନିବେଶ)
{
	ଯେତେବେଳେ (1) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା ch = i_getch(ନିବେଶ);
		ଯଦି (ch == EOF) {
			syntax_error_unterm_ch('\'');
			ଫେରନ୍ତୁ 0;
		}
		ଯଦି (ch == '\'')
			ଫେରନ୍ତୁ 1;
		o_addqchr(dest, ch);
	}
}

/* "...\"...`..`...." - do we need to handle "...$(..)..." too? */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା add_till_backquote(o_string *dest, ସଂରଚନା in_str *ନିବେଶ, ପୂର୍ଣ୍ଣସଂଖ୍ୟା in_dquote);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା add_till_double_quote(o_string *dest, ସଂରଚନା in_str *ନିବେଶ)
{
	ଯେତେବେଳେ (1) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା ch = i_getch(ନିବେଶ);
		ଯଦି (ch == EOF) {
			syntax_error_unterm_ch('"');
			ଫେରନ୍ତୁ 0;
		}
		ଯଦି (ch == '"')
			ଫେରନ୍ତୁ 1;
		ଯଦି (ch == '\\') {  /* \x. Copy both chars. */
			o_addchr(dest, ch);
			ch = i_getch(ନିବେଶ);
		}
		o_addchr(dest, ch);
		ଯଦି (ch == '`') {
			ଯଦି (!add_till_backquote(dest, ନିବେଶ, /*in_dquote:*/ 1))
				ଫେରନ୍ତୁ 0;
			o_addchr(dest, ch);
			ଜାରି;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା add_till_backquote(o_string *dest, ସଂରଚନା in_str *ନିବେଶ, ପୂର୍ଣ୍ଣସଂଖ୍ୟା in_dquote)
{
	ଯେତେବେଳେ (1) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା ch = i_getch(ନିବେଶ);
		ଯଦି (ch == '`')
			ଫେରନ୍ତୁ 1;
		ଯଦି (ch == '\\') {
			/* \x. Copy both unless it is \`, \$, \\ and maybe \" */
			ch = i_getch(ନିବେଶ);
			ଯଦି (ch != '`'
			 && ch != '$'
			 && ch != '\\'
			 && (!in_dquote || ch != '"')
			) {
				o_addchr(dest, '\\');
			}
		}
		ଯଦି (ch == EOF) {
			syntax_error_unterm_ch('`');
			ଫେରନ୍ତୁ 0;
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
#define DOUBLE_CLOSE_CHAR_FLAG 0x80
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା add_till_closing_bracket(o_string *dest, ସଂରଚନା in_str *ନିବେଶ, unsigned end_ch)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ch;
	ଅକ୍ଷର dbl = end_ch & DOUBLE_CLOSE_CHAR_FLAG;
# ଯଦି BASH_SUBSTR || BASH_PATTERN_SUBST
	ଅକ୍ଷର end_char2 = end_ch >> 8;
# endif
	end_ch &= (DOUBLE_CLOSE_CHAR_FLAG - 1);

# ଯଦି ENABLE_HUSH_INTERACTIVE
	G.promptmode = 1; /* PS2 */
# endif
	debug_printf_prompt("%s promptmode=%d\n", __func__, G.promptmode);

	ଯେତେବେଳେ (1) {
		ch = i_getch(ନିବେଶ);
		ଯଦି (ch == EOF) {
			syntax_error_unterm_ch(end_ch);
			ଫେରନ୍ତୁ 0;
		}
		ଯଦି (ch == end_ch
# ଯଦି BASH_SUBSTR || BASH_PATTERN_SUBST
		 || ch == end_char2
# endif
		) {
			ଯଦି (!dbl)
				ଭାଙ୍ଗ;
			/* we look for closing )) of $((EXPR)) */
			ଯଦି (i_peek_and_eat_bkslash_nl(ନିବେଶ) == end_ch) {
				i_getch(ନିବେଶ); /* eat second ')' */
				ଭାଙ୍ଗ;
			}
		}
		o_addchr(dest, ch);
		//bb_error_msg("%s:o_addchr('%c')", __func__, ch);
		ଯଦି (ch == '(' || ch == '{') {
			ch = (ch == '(' ? ')' : '}');
			ଯଦି (!add_till_closing_bracket(dest, ନିବେଶ, ch))
				ଫେରନ୍ତୁ 0;
			o_addchr(dest, ch);
			ଜାରି;
		}
		ଯଦି (ch == '\'') {
			ଯଦି (!add_till_single_quote(dest, ନିବେଶ))
				ଫେରନ୍ତୁ 0;
			o_addchr(dest, ch);
			ଜାରି;
		}
		ଯଦି (ch == '"') {
			ଯଦି (!add_till_double_quote(dest, ନିବେଶ))
				ଫେରନ୍ତୁ 0;
			o_addchr(dest, ch);
			ଜାରି;
		}
		ଯଦି (ch == '`') {
			ଯଦି (!add_till_backquote(dest, ନିବେଶ, /*in_dquote:*/ 0))
				ଫେରନ୍ତୁ 0;
			o_addchr(dest, ch);
			ଜାରି;
		}
		ଯଦି (ch == '\\') {
			/* \x. Copy verbatim. Important for  \(, \) */
			ch = i_getch(ନିବେଶ);
			ଯଦି (ch == EOF) {
				syntax_error_unterm_ch(end_ch);
				ଫେରନ୍ତୁ 0;
			}
# ଯଦି 0
			ଯଦି (ch == '\n') {
				/* "backslash+newline", ignore both */
				o_delchr(dest); /* undo insertion of '\' */
				ଜାରି;
			}
# endif
			o_addchr(dest, ch);
			//bb_error_msg("%s:o_addchr('%c') after '\\'", __func__, ch);
			ଜାରି;
		}
	}
	debug_printf_parse("%s return '%s' ch:'%c'\n", __func__, dest->data, ch);
	ଫେରନ୍ତୁ ch;
}
#endif /* ENABLE_HUSH_TICK || ENABLE_FEATURE_SH_MATH || ENABLE_HUSH_DOLLAR_OPS */

#ଯଦି BASH_DOLLAR_SQUOTE
/* Return code: 1 for "found and parsed", 0 for "seen something else" */
# ଯଦି BB_MMU
#define parse_dollar_squote(as_string, dest, ନିବେଶ) \
	parse_dollar_squote(dest, ନିବେଶ)
#define as_string ଖାଲି
# endif
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା parse_dollar_squote(o_string *as_string, o_string *dest, ସଂରଚନା in_str *ନିବେଶ)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା start;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ch = i_peek_and_eat_bkslash_nl(ନିବେଶ);  /* first character after the $ */
	debug_printf_parse("parse_dollar_squote entered: ch='%c'\n", ch);
	ଯଦି (ch != '\'')
		ଫେରନ୍ତୁ 0;

	dest->has_quoted_part = 1;
	start = dest->length;

	ch = i_getch(ନିବେଶ); /* eat ' */
	nommu_addchr(as_string, ch);
	ଯେତେବେଳେ (1) {
		ch = i_getch(ନିବେଶ);
		nommu_addchr(as_string, ch);
		ଯଦି (ch == EOF) {
			syntax_error_unterm_ch('\'');
			ଫେରନ୍ତୁ 0;
		}
		ଯଦି (ch == '\'')
			ଭାଙ୍ଗ;
		ଯଦି (ch == SPECIAL_VAR_ପ୍ରତୀକ) {
			/* Convert raw ^C to corresponding special variable reference */
			o_addchr(dest, SPECIAL_VAR_ପ୍ରତୀକ);
			o_addchr(dest, SPECIAL_VAR_QUOTED_SVS);
			/* will addchr() another SPECIAL_VAR_SYMBOL (see after the if() block) */
		} ଅନ୍ୟଥା ଯଦି (ch == '\\') {
			ସ୍ଥିର ଅଚଳ ଅକ୍ଷର C_escapes[] ALIGN1 = "nrbtfav""x\\01234567";

			ch = i_getch(ନିବେଶ);
			nommu_addchr(as_string, ch);
			ଯଦି (strchr(C_escapes, ch)) {
				ଅକ୍ଷର buf[4];
				ଅକ୍ଷର *p = buf;
				ପୂର୍ଣ୍ଣସଂଖ୍ୟା cnt = 2;

				buf[0] = ch;
				ଯଦି ((unsigned ଅକ୍ଷର)(ch - '0') <= 7) { /* \ooo */
					କର {
						ch = i_peek(ନିବେଶ);
						ଯଦି ((unsigned ଅକ୍ଷର)(ch - '0') > 7)
							ଭାଙ୍ଗ;
						*++p = ch = i_getch(ନିବେଶ);
						nommu_addchr(as_string, ch);
					} ଯେତେବେଳେ (--cnt != 0);
				} ଅନ୍ୟଥା ଯଦି (ch == 'x') { /* \xHH */
					କର {
						ch = i_peek(ନିବେଶ);
						ଯଦି (!isxdigit(ch))
							ଭାଙ୍ଗ;
						*++p = ch = i_getch(ନିବେଶ);
						nommu_addchr(as_string, ch);
					} ଯେତେବେଳେ (--cnt != 0);
					ଯଦି (cnt == 2) { /* \x but next char is "bad" */
						ch = 'x';
						ଯାଅ unrecognized;
					}
				} /* else simple seq like \\ or \t */
				*++p = '\0';
				p = buf;
				ch = bb_process_escape_sequence((ଶୂନ୍ୟ*)&p);
				//bb_error_msg("buf:'%s' ch:%x", buf, ch);
				ଯଦି (ch == '\0')
					ଜାରି; /* bash compat: $'...\0...' emits nothing */
			} ଅନ୍ୟଥା { /* unrecognized "\z": encode both chars unless ' or " */
				ଯଦି (ch != '\'' && ch != '"') {
 unrecognized:
					o_addqchr(dest, '\\');
				}
			}
		} /* if (\...) */
		o_addqchr(dest, ch);
	}

	ଯଦି (dest->length == start) {
		/* $'', $'\0', $'\000\x00' and the like */
		o_addchr(dest, SPECIAL_VAR_ପ୍ରତୀକ);
		o_addchr(dest, SPECIAL_VAR_ପ୍ରତୀକ);
	}

	ଫେରନ୍ତୁ 1;
# undef as_string
}
#ଅନ୍ୟଥା
# define parse_dollar_squote(as_string, dest, ନିବେଶ) 0
#endif /* BASH_DOLLAR_SQUOTE */
#endif /* !__U_BOOT__ */

/* Return code: 0 for OK, 1 for syntax error */
#ଯଦି BB_MMU
#define parse_dollar(as_string, dest, ନିବେଶ, quote_mask) \
	parse_dollar(dest, ନିବେଶ, quote_mask)
#define as_string ଖାଲି
#endif
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା parse_dollar(o_string *as_string,
		o_string *dest,
		ସଂରଚନା in_str *ନିବେଶ, unsigned ଅକ୍ଷର quote_mask)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ch = i_peek_and_eat_bkslash_nl(ନିବେଶ);  /* first character after the $ */

	debug_printf_parse("parse_dollar entered: ch='%c' quote_mask:0x%x\n", ch, quote_mask);
	ଯଦି (isalpha(ch)) {
 make_var:
		ch = i_getch(ନିବେଶ);
		nommu_addchr(as_string, ch);
 /*make_var1:*/
		o_addchr(dest, SPECIAL_VAR_ପ୍ରତୀକ);
		ଯେତେବେଳେ (1) {
			debug_printf_parse(": '%c'\n", ch);
			o_addchr(dest, ch | quote_mask);
			quote_mask = 0;
			ch = i_peek_and_eat_bkslash_nl(ନିବେଶ);
			ଯଦି (!isalnum(ch) && ch != '_') {
				/* End of variable name reached */
				ଭାଙ୍ଗ;
			}
			ch = i_getch(ନିବେଶ);
			nommu_addchr(as_string, ch);
		}
		o_addchr(dest, SPECIAL_VAR_ପ୍ରତୀକ);
	} ଅନ୍ୟଥା ଯଦି (isdigit(ch)) {
 make_one_char_var:
		ch = i_getch(ନିବେଶ);
		nommu_addchr(as_string, ch);
		o_addchr(dest, SPECIAL_VAR_ପ୍ରତୀକ);
		debug_printf_parse(": '%c'\n", ch);
		o_addchr(dest, ch | quote_mask);
		o_addchr(dest, SPECIAL_VAR_ପ୍ରତୀକ);
	} ଅନ୍ୟଥା ବାଛ (ch) {
#ifndef __U_BOOT__
	ସ୍ଥିତି '$': /* pid */
	ସ୍ଥିତି '!': /* last bg pid */
#endif /* !__U_BOOT__ */
	ସ୍ଥିତି '?': /* last exit code */
	ସ୍ଥିତି '#': /* number of args */
	ସ୍ଥିତି '*': /* args */
	ସ୍ଥିତି '@': /* args */
	ସ୍ଥିତି '-': /* $- option flags set by set builtin or shell options (-i etc) */
		ଯାଅ make_one_char_var;
	ସ୍ଥିତି '{': {
		ଅକ୍ଷର len_single_ch;

		o_addchr(dest, SPECIAL_VAR_ପ୍ରତୀକ);

		ch = i_getch(ନିବେଶ); /* eat '{' */
		nommu_addchr(as_string, ch);

		ch = i_getch_and_eat_bkslash_nl(ନିବେଶ); /* first char after '{' */
		/* It should be ${?}, or ${#var},
		 * or even ${?+subst} - operator acting on a special variable,
		 * or the beginning of variable name.
		 */
		ଯଦି (ch == EOF
		 || (!strchr(_SPECIAL_VARS_STR, ch) && !isalnum(ch)) /* not one of those */
		) {
 bad_dollar_syntax:
			syntax_error_unterm_str("${name}");
			debug_printf_parse("parse_dollar return 0: unterminated ${name}\n");
			ଫେରନ୍ତୁ 0;
		}
		nommu_addchr(as_string, ch);
		len_single_ch = ch;
		ch |= quote_mask;

		/* It's possible to just call add_till_closing_bracket() at this point.
		 * However, this regresses some of our testsuite cases
		 * which check invalid constructs like ${%}.
		 * Oh well... let's check that the var name part is fine... */

		ଯଦି (isdigit(len_single_ch)
		 || (len_single_ch == '#' && isdigit(i_peek_and_eat_bkslash_nl(ନିବେଶ)))
		) {
			/* Execution engine uses plain xatoi_positive()
			 * to interpret ${NNN} and {#NNN},
			 * check syntax here in the parser.
			 * (bash does not support expressions in ${#NN},
			 * e.g. ${#$var} and {#1:+WORD} are not supported).
			 */
			unsigned cnt = 9; /* max 9 digits for ${NN} and 8 for {#NN} */
			ଯେତେବେଳେ (1) {
				o_addchr(dest, ch);
				debug_printf_parse(": '%c'\n", ch);
				ch = i_getch_and_eat_bkslash_nl(ନିବେଶ);
				nommu_addchr(as_string, ch);
				ଯଦି (ch == '}')
					ଭାଙ୍ଗ;
				ଯଦି (--cnt == 0)
					ଯାଅ bad_dollar_syntax;
				ଯଦି (len_single_ch != '#' && strchr(VAR_SUBST_OPS, ch))
					/* ${NN<op>...} is valid */
					ଯାଅ eat_until_closing;
				ଯଦି (!isdigit(ch))
					ଯାଅ bad_dollar_syntax;
			}
		} ଅନ୍ୟଥା
		ଯେତେବେଳେ (1) {
			unsigned pos;

			o_addchr(dest, ch);
			debug_printf_parse(": '%c'\n", ch);

			ch = i_getch(ନିବେଶ);
			nommu_addchr(as_string, ch);
			ଯଦି (ch == '}')
				ଭାଙ୍ଗ;
#ifndef __U_BOOT__
			ଯଦି (!isalnum(ch) && ch != '_') {
#ଅନ୍ୟଥା /* __U_BOOT__ */
			/*
			 * In several places in U-Boot, we use variable like
			 * foo# (e.g. serial#), particularly in env.
			 * So, we need to authorize # to appear inside
			 * variable name and then expand this variable.
			 * NOTE Having # in variable name is not permitted in
			 * upstream hush but expansion will be done (even though
			 * the result will be empty).
			 */
			ଯଦି (!isalnum(ch) && ch != '_' && ch != '#') {
#endif /* __U_BOOT__ */
				unsigned end_ch;
#ifndef __U_BOOT__
				unsigned ଅକ୍ଷର last_ch;
#endif /* !__U_BOOT__ */
				/* handle parameter expansions
				 * http://www.opengroup.org/onlinepubs/009695399/utilities/xcu_chap02.html#tag_02_06_02
				 */
				ଯଦି (!strchr(VAR_SUBST_OPS, ch)) { /* ${var<bad_char>... */
					ଯଦି (len_single_ch != '#'
					/*|| !strchr(SPECIAL_VARS_STR, ch) - disallow errors like ${#+} ? */
					 || i_peek(ନିବେଶ) != '}'
					) {
						ଯାଅ bad_dollar_syntax;
					}
					/* else: it's "length of C" ${#C} op,
					 * where C is a single char
					 * special var name, e.g. ${#!}.
					 */
				}
 eat_until_closing:
				/* Eat everything until closing '}' (or ':') */
				end_ch = '}';
#ifndef __U_BOOT__
				ଯଦି (BASH_SUBSTR
				 && ch == ':'
				 && !strchr(MINUS_PLUS_EQUAL_QUESTION, i_peek(ନିବେଶ))
				) {
					/* It's ${var:N[:M]} thing */
					end_ch = '}' * 0x100 + ':';
				}
				ଯଦି (BASH_PATTERN_SUBST
				 && ch == '/'
				) {
					/* It's ${var/[/]pattern[/repl]} thing */
					ଯଦି (i_peek(ନିବେଶ) == '/') { /* ${var//pattern[/repl]}? */
						i_getch(ନିବେଶ);
						nommu_addchr(as_string, '/');
						ch = '\\';
					}
					end_ch = '}' * 0x100 + '/';
				}
#endif /* !__U_BOOT__ */
				o_addchr(dest, ch);
				/* The pattern can't be empty.
				 * IOW: if the first char after "${v//" is a slash,
				 * it does not terminate the pattern - it's the first char of the pattern:
				 *  v=/dev/ram; echo ${v////-}  prints -dev-ram (pattern is "/")
				 *  v=/dev/ram; echo ${v///r/-} prints /dev-am  (pattern is "/r")
				 */
				ଯଦି (i_peek(ନିବେଶ) == '/') {
					o_addchr(dest, i_getch(ନିବେଶ));
				}
#ifndef __U_BOOT__
 again:
#endif /* !__U_BOOT__ */
				ଯଦି (!BB_MMU)
					pos = dest->length;
#ଯଦି ENABLE_HUSH_DOLLAR_OPS
#ifndef __U_BOOT__
				last_ch = add_till_closing_bracket(dest, ନିବେଶ, end_ch);
				ଯଦି (last_ch == 0) /* error? */
					ଫେରନ୍ତୁ 0;
#endif /* !__U_BOOT__ */
#ଅନ୍ୟଥା
# error Simple code to only allow ${var} is not implemented
#endif
				ଯଦି (as_string) {
					o_addstr(as_string, dest->data + pos);
#ifndef __U_BOOT__
					o_addchr(as_string, last_ch);
#endif /* !__U_BOOT__ */
				}

#ifndef __U_BOOT__
				ଯଦି ((BASH_SUBSTR || BASH_PATTERN_SUBST)
					 && (end_ch & 0xff00)
				) {
					/* close the first block: */
					o_addchr(dest, SPECIAL_VAR_ପ୍ରତୀକ);
					/* while parsing N from ${var:N[:M]}
					 * or pattern from ${var/[/]pattern[/repl]} */
					ଯଦି ((end_ch & 0xff) == last_ch) {
						/* got ':' or '/'- parse the rest */
						end_ch = '}';
						ଯାଅ again;
					}
					/* got '}' */
					ଯଦି (BASH_SUBSTR && end_ch == '}' * 0x100 + ':') {
						/* it's ${var:N} - emulate :999999999 */
						o_addstr(dest, "999999999");
					} /* else: it's ${var/[/]pattern} */
				}
#endif /* !__U_BOOT__ */
				ଭାଙ୍ଗ;
			}
			len_single_ch = 0; /* it can't be ${#C} op */
		}
		o_addchr(dest, SPECIAL_VAR_ପ୍ରତୀକ);
		ଭାଙ୍ଗ;
	}
#ଯଦି ENABLE_FEATURE_SH_MATH || ENABLE_HUSH_TICK
	ସ୍ଥିତି '(': {
		unsigned pos;

		ch = i_getch(ନିବେଶ);
		nommu_addchr(as_string, ch);
# ଯଦି ENABLE_FEATURE_SH_MATH
		ଯଦି (i_peek_and_eat_bkslash_nl(ନିବେଶ) == '(') {
			ch = i_getch(ନିବେଶ);
			nommu_addchr(as_string, ch);
			o_addchr(dest, SPECIAL_VAR_ପ୍ରତୀକ);
			o_addchr(dest, quote_mask | '+');
			ଯଦି (!BB_MMU)
				pos = dest->length;
			ଯଦି (!add_till_closing_bracket(dest, ନିବେଶ, ')' | DOUBLE_CLOSE_CHAR_FLAG))
				ଫେରନ୍ତୁ 0; /* error */
			ଯଦି (as_string) {
				o_addstr(as_string, dest->data + pos);
				o_addchr(as_string, ')');
				o_addchr(as_string, ')');
			}
			o_addchr(dest, SPECIAL_VAR_ପ୍ରତୀକ);
			ଭାଙ୍ଗ;
		}
# endif
# ଯଦି ENABLE_HUSH_TICK
		o_addchr(dest, SPECIAL_VAR_ପ୍ରତୀକ);
		o_addchr(dest, quote_mask | '`');
		ଯଦି (!BB_MMU)
			pos = dest->length;
		ଯଦି (!add_till_closing_bracket(dest, ନିବେଶ, ')'))
			ଫେରନ୍ତୁ 0; /* error */
		ଯଦି (as_string) {
			o_addstr(as_string, dest->data + pos);
			o_addchr(as_string, ')');
		}
		o_addchr(dest, SPECIAL_VAR_ପ୍ରତୀକ);
# endif
		ଭାଙ୍ଗ;
	}
#endif
	ସ୍ଥିତି '_':
		ଯାଅ make_var;
#ଯଦି 0
	/* TODO: $_: */
	/* $_ Shell or shell script name; or last argument of last command
	 * (if last command wasn't a pipe; if it was, bash sets $_ to "");
	 * but in command's env, set to full pathname used to invoke it */
		ch = i_getch(ନିବେଶ);
		nommu_addchr(as_string, ch);
		ch = i_peek_and_eat_bkslash_nl(ନିବେଶ);
		ଯଦି (isalnum(ch)) { /* it's $_name or $_123 */
			ch = '_';
			ଯାଅ make_var1;
		}
		/* else: it's $_ */
#endif
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		o_addQchr(dest, '$');
	}
	debug_printf_parse("parse_dollar return 1 (ok)\n");
	ଫେରନ୍ତୁ 1;
#undef as_string
}

#ଯଦି BB_MMU
#define encode_string(as_string, dest, ନିବେଶ, dquote_end) \
	encode_string(dest, ନିବେଶ, dquote_end)
#define as_string ଖାଲି
#endif
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା encode_string(o_string *as_string,
		o_string *dest,
		ସଂରଚନା in_str *ନିବେଶ,
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା dquote_end)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ch;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା next;

 again:
	ch = i_getch(ନିବେଶ);
	ଯଦି (ch != EOF)
		nommu_addchr(as_string, ch);
	ଯଦି (ch == dquote_end) { /* may be only '"' or EOF */
		debug_printf_parse("encode_string return 1 (ok)\n");
		ଫେରନ୍ତୁ 1;
	}
	/* note: can't move it above ch == dquote_end check! */
	ଯଦି (ch == EOF) {
		syntax_error_unterm_ch('"');
		ଫେରନ୍ତୁ 0; /* error */
	}
	next = '\0';
	ଯଦି (ch != '\n') {
		next = i_peek(ନିବେଶ);
	}
	debug_printf_parse("\" ch=%c (%d) escape=%d\n",
			ch, ch, !!(dest->o_expflags & EXP_FLAG_ESC_GLOB_CHARS));
	ଯଦି (ch == '\\') {
		ଯଦି (next == EOF) {
			/* Testcase: in interactive shell a file with
			 *  echo "unterminated string\<eof>
			 * is sourced.
			 */
			syntax_error_unterm_ch('"');
			ଫେରନ୍ତୁ 0; /* error */
		}
		/* bash:
		 * "The backslash retains its special meaning [in "..."]
		 * only when followed by one of the following characters:
		 * $, `, ", \, or <newline>.  A double quote may be quoted
		 * within double quotes by preceding it with a backslash."
		 * NB: in (unquoted) heredoc, above does not apply to ",
		 * therefore we check for it by "next == dquote_end" cond.
		 */
		ଯଦି (next == dquote_end || strchr("$`\\\n", next)) {
			ch = i_getch(ନିବେଶ); /* eat next */
			ଯଦି (ch == '\n')
				ଯାଅ again; /* skip \<newline> */
		} /* else: ch remains == '\\', and we double it below: */
		o_addqchr(dest, ch); /* \c if c is a glob char, else just c */
		nommu_addchr(as_string, ch);
		ଯାଅ again;
	}
	ଯଦି (ch == '$') {
		//if (parse_dollar_squote(as_string, dest, input))
		//	goto again;
		ଯଦି (!parse_dollar(as_string, dest, ନିବେଶ, /*quote_mask:*/ 0x80)) {
			debug_printf_parse("encode_string return 0: "
					"parse_dollar returned 0 (error)\n");
			ଫେରନ୍ତୁ 0;
		}
		ଯାଅ again;
	}
#ଯଦି ENABLE_HUSH_TICK
	ଯଦି (ch == '`') {
		//unsigned pos = dest->length;
		o_addchr(dest, SPECIAL_VAR_ପ୍ରତୀକ);
		o_addchr(dest, 0x80 | '`');
		ଯଦି (!add_till_backquote(dest, ନିବେଶ, /*in_dquote:*/ dquote_end == '"'))
			ଫେରନ୍ତୁ 0; /* error */
		o_addchr(dest, SPECIAL_VAR_ପ୍ରତୀକ);
		//debug_printf_subst("SUBST RES3 '%s'\n", dest->data + pos);
		ଯାଅ again;
	}
#endif
	o_addQchr(dest, ch);
	ଯଦି (ch == SPECIAL_VAR_ପ୍ରତୀକ) {
		/* Convert "^C" to corresponding special variable reference */
		o_addchr(dest, SPECIAL_VAR_QUOTED_SVS);
		o_addchr(dest, SPECIAL_VAR_ପ୍ରତୀକ);
	}
	ଯାଅ again;
#undef as_string
}

/*
 * Scan input until EOF or end_trigger char.
 * Return a list of pipes to execute, or NULL on EOF
 * or if end_trigger character is met.
 * On syntax error, exit if shell is not interactive,
 * reset parsing machinery and start parsing anew,
 * or return ERR_PTR.
 */
ସ୍ଥିର ସଂରଚନା pipe *parse_stream(ଅକ୍ଷର **pstring,
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା *heredoc_cnt_ptr,
		ସଂରଚନା in_str *ନିବେଶ,
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା end_trigger)
{
	ସଂରଚନା parse_context ctx;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା heredoc_cnt;

	/* Single-quote triggers a bypass of the main loop until its mate is
	 * found.  When recursing, quote state is passed in via ctx.word.o_expflags.
	 */
	debug_printf_parse("parse_stream entered, end_trigger='%c'\n",
			end_trigger ? end_trigger : 'X');
	debug_enter();

	initialize_context(&ctx);

	/* If very first arg is "" or '', ctx.word.data may end up NULL.
	 * Preventing this:
	 */
	ctx.word.data = xzalloc(1); /* start as "", not as NULL */

	/* We used to separate words on $IFS here. This was wrong.
	 * $IFS is used only for word splitting when $var is expanded,
	 * here we should use blank chars as separators, not $IFS
	 */

	heredoc_cnt = 0;
	ଯେତେବେଳେ (1) {
		ଅଚଳ ଅକ୍ଷର *is_blank;
		ଅଚଳ ଅକ୍ଷର *is_special;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା ch;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା next;
#ifndef __U_BOOT__
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା redir_fd;
		redir_type redir_style;
#endif /* !__U_BOOT__ */

		ch = i_getch(ନିବେଶ);
		debug_printf_parse(": ch=%c (%d) escape=%d\n",
				ch, ch, !!(ctx.word.o_expflags & EXP_FLAG_ESC_GLOB_CHARS));
		ଯଦି (ch == EOF) {
			ସଂରଚନା pipe *pi;

			ଯଦି (heredoc_cnt) {
				syntax_error_unterm_str("here document");
				ଯାଅ parse_error_exitcode1;
			}
			ଯଦି (end_trigger == ')') {
				syntax_error_unterm_ch('(');
				ଯାଅ parse_error_exitcode1;
			}
			ଯଦି (end_trigger == '}') {
				syntax_error_unterm_ch('{');
				ଯାଅ parse_error_exitcode1;
			}

			ଯଦି (done_word(&ctx)) {
				ଯାଅ parse_error_exitcode1;
			}
			o_free_and_set_NULL(&ctx.word);
			done_pipe(&ctx, PIPE_SEQ);

			/* Do we sit inside of any if's, loops or case's? */
			ଯଦି (HAS_KEYWORDS
			IF_HAS_KEYWORDS(&& (ctx.ctx_res_w != RES_NONE || ctx.old_flag != 0))
			) {
				syntax_error_unterm_str("compound statement");
				ଯାଅ parse_error_exitcode1;
			}

			pi = ctx.list_head;
			/* If we got nothing... */
			/* (this makes bare "&" cmd a no-op.
			 * bash says: "syntax error near unexpected token '&'") */
			ଯଦି (pi->ଅଙ୍କ_cmds == 0
			IF_HAS_KEYWORDS(&& pi->res_word == RES_NONE)
			) {
				free_pipe_list(pi);
				pi = ଖାଲି;
			}
#ଯଦି !BB_MMU
			debug_printf_parse("as_string1 '%s'\n", ctx.as_string.data);
			ଯଦି (pstring)
				*pstring = ctx.as_string.data;
			ଅନ୍ୟଥା
				o_free(&ctx.as_string);
#endif
			// heredoc_cnt must be 0 here anyway
			//if (heredoc_cnt_ptr)
			//	*heredoc_cnt_ptr = heredoc_cnt;
			debug_leave();
			debug_printf_heredoc("parse_stream return heredoc_cnt:%d\n", heredoc_cnt);
			debug_printf_parse("parse_stream return %p: EOF\n", pi);
			ଫେରନ୍ତୁ pi;
		}

		/* Handle "'" and "\" first, as they won't play nice with
		 * i_peek_and_eat_bkslash_nl() anyway:
		 *   echo z\\
		 * and
		 *   echo '\
		 *   '
		 * would break.
		 */
		ଯଦି (ch == '\\') {
			ch = i_getch(ନିବେଶ);
			ଯଦି (ch == '\n')
				ଜାରି; /* drop \<newline>, get next char */
			nommu_addchr(&ctx.as_string, '\\');
			ଯଦି (ch == SPECIAL_VAR_ପ୍ରତୀକ) {
				nommu_addchr(&ctx.as_string, ch);
				/* Convert \^C to corresponding special variable reference */
				ଯାଅ case_SPECIAL_VAR_ପ୍ରତୀକ;
			}
			o_addchr(&ctx.word, '\\');
			ଯଦି (ch == EOF) {
				/* Testcase: eval 'echo Ok\' */
				/* bash-4.3.43 was removing backslash,
				 * but 4.4.19 retains it, most other shells too
				 */
				ଜାରି; /* get next char */
			}
			/* Example: echo Hello \2>file
			 * we need to know that word 2 is quoted
			 */
			ctx.word.has_quoted_part = 1;
			nommu_addchr(&ctx.as_string, ch);
			o_addchr(&ctx.word, ch);
			ଜାରି; /* get next char */
		}
		nommu_addchr(&ctx.as_string, ch);
		ଯଦି (ch == '\'') {
			ctx.word.has_quoted_part = 1;
			next = i_getch(ନିବେଶ);
#ifndef __U_BOOT__
			ଯଦି (next == '\'' && !ctx.pending_redirect)
				ଯାଅ ଭର୍ତ୍ତି_empty_quoted_str_marker;
#endif /* !__U_BOOT__ */

			ch = next;
			ଯେତେବେଳେ (1) {
				ଯଦି (ch == EOF) {
					syntax_error_unterm_ch('\'');
					ଯାଅ parse_error_exitcode1;
				}
				nommu_addchr(&ctx.as_string, ch);
				ଯଦି (ch == '\'')
					ଭାଙ୍ଗ;
				ଯଦି (ch == SPECIAL_VAR_ପ୍ରତୀକ) {
					/* Convert raw ^C to corresponding special variable reference */
					o_addchr(&ctx.word, SPECIAL_VAR_ପ୍ରତୀକ);
					o_addchr(&ctx.word, SPECIAL_VAR_QUOTED_SVS);
				}
				o_addqchr(&ctx.word, ch);
				ch = i_getch(ନିବେଶ);
			}
			ଜାରି; /* get next char */
		}

		next = '\0';
		ଯଦି (ch != '\n')
			next = i_peek_and_eat_bkslash_nl(ନିବେଶ);

		is_special = "{}<>&|();#" /* special outside of "str" */
#ifndef __U_BOOT__
				"$\"" IF_HUSH_TICK("`") /* always special */
#ଅନ୍ୟଥା /* __U_BOOT__ */
				"$\""
#endif /* __U_BOOT__ */
				SPECIAL_VAR_ପ୍ରତୀକ_STR;
#ଯଦି defined(CMD_TEST2_SINGLEWORD_NOGLOB)
		ଯଦି (ctx.command->cmd_type == CMD_TEST2_SINGLEWORD_NOGLOB) {
			/* In [[ ]], {}<>&|() are not special */
			is_special += 8;
		} ଅନ୍ୟଥା
#endif
		/* Are { and } special here? */
		ଯଦି (ctx.command->argv /* word [word]{... - non-special */
		 || ctx.word.length       /* word{... - non-special */
		 || ctx.word.has_quoted_part     /* ""{... - non-special */
		 || (next != ';'             /* }; - special */
		    && next != ')'           /* }) - special */
		    && next != '('           /* {( - special */
		    && next != '&'           /* }& and }&& ... - special */
		    && next != '|'           /* }|| ... - special */
		    && !strchr(defifs, next) /* {word - non-special */
		    )
		) {
			/* They are not special, skip "{}" */
			is_special += 2;
		}
		is_special = strchr(is_special, ch);
		is_blank = strchr(defifs, ch);

		ଯଦି (!is_special && !is_blank) { /* ordinary char */
 ordinary_char:
			o_addQchr(&ctx.word, ch);
			ଯଦି ((ctx.is_assignment == MAYBE_ASSIGNMENT
			    || ctx.is_assignment == WORD_IS_KEYWORD)
			 && ch == '='
			 && endofname(ctx.word.data)[0] == '='
			) {
				ctx.is_assignment = DEFINITELY_ASSIGNMENT;
				debug_printf_parse("ctx.is_assignment='%s'\n", assignment_flag[ctx.is_assignment]);
			}
			ଜାରି;
		}

		ଯଦି (is_blank) {
#ଯଦି ENABLE_HUSH_LINENO_VAR
/* Case:
 * "while ...; do<whitespace><newline>
 *	cmd ..."
 * would think that "cmd" starts in <whitespace> -
 * i.e., at the previous line.
 * We need to skip all whitespace before newlines.
 */
			ଯେତେବେଳେ (ch != '\n') {
				next = i_peek(ନିବେଶ);
				ଯଦି (next != ' ' && next != '\t' && next != '\n')
					ଭାଙ୍ଗ; /* next char is not ws */
				ch = i_getch(ନିବେଶ);
			}
			/* ch == last eaten whitespace char */
#endif
			ଯଦି (done_word(&ctx)) {
				ଯାଅ parse_error_exitcode1;
			}
			ଯଦି (ch == '\n') {
				/* Is this a case when newline is simply ignored?
				 * Some examples:
				 * "cmd | <newline> cmd ..."
				 * "case ... in <newline> word) ..."
				 */
				ଯଦି (IS_NULL_CMD(ctx.command)
				 && ctx.word.length == 0
				 && !ctx.word.has_quoted_part
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
					ସଂରଚନା pipe *pi = ctx.list_head;
					ଯଦି (pi->ଅଙ୍କ_cmds != 0       /* check #1 */
					 && pi->followup != PIPE_BG /* check #2 */
					) {
						ଜାରି;
					}
				}
				/* Treat newline as a command separator. */
				done_pipe(&ctx, PIPE_SEQ);
				debug_printf_heredoc("heredoc_cnt:%d\n", heredoc_cnt);
				ଯଦି (heredoc_cnt) {
					heredoc_cnt = fetch_heredocs(&ctx.as_string, ctx.list_head, heredoc_cnt, ନିବେଶ);
					ଯଦି (heredoc_cnt != 0)
						ଯାଅ parse_error_exitcode1;
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
#ifndef __U_BOOT__
		ଯଦି (ch == '}') {
#ଅନ୍ୟଥା /* __U_BOOT__ */
		ଯଦି (ch == '}' || ch == ')') {
#endif /* __U_BOOT__ */
			ଯଦି (ctx.word.length != 0 /* word} */
			 || ctx.word.has_quoted_part    /* ""} */
			) {
				ଯାଅ ordinary_char;
			}
			ଯଦି (!IS_NULL_CMD(ctx.command)) { /* cmd } */
				/* Generally, there should be semicolon: "cmd; }"
				 * However, bash allows to omit it if "cmd" is
				 * a group. Examples:
				 * { { echo 1; } }
				 * {(echo 1)}
				 * { echo 0 >&2 | { echo 1; } }
				 * { while false; do :; done }
				 * { case a in b) ;; esac }
				 */
				ଯଦି (ctx.command->group)
					ଯାଅ term_group;
				ଯାଅ ordinary_char;
			}
			ଯଦି (!IS_NULL_PIPE(ctx.pipe)) /* cmd | } */
				/* Can't be an end of {cmd}, skip the check */
				ଯାଅ skip_end_trigger;
			/* else: } does terminate a group */
		}
 term_group:
		ଯଦି (end_trigger && end_trigger == ch
		 && (ch != ';' || heredoc_cnt == 0)
#ଯଦି ENABLE_HUSH_CASE
		 && (ch != ')'
		    || ctx.ctx_res_w != RES_MATCH
		    || (!ctx.word.has_quoted_part && strcmp(ctx.word.data, "esac") == 0)
		    )
#endif
		) {
			ଯଦି (done_word(&ctx)) {
				ଯାଅ parse_error_exitcode1;
			}
			done_pipe(&ctx, PIPE_SEQ);
			ctx.is_assignment = MAYBE_ASSIGNMENT;
			debug_printf_parse("ctx.is_assignment='%s'\n", assignment_flag[ctx.is_assignment]);
			/* Do we sit outside of any if's, loops or case's? */
			ଯଦି (!HAS_KEYWORDS
			IF_HAS_KEYWORDS(|| (ctx.ctx_res_w == RES_NONE && ctx.old_flag == 0))
			) {
				o_free_and_set_NULL(&ctx.word);
#ଯଦି !BB_MMU
				debug_printf_parse("as_string2 '%s'\n", ctx.as_string.data);
				ଯଦି (pstring)
					*pstring = ctx.as_string.data;
				ଅନ୍ୟଥା
					o_free(&ctx.as_string);
#endif
				ଯଦି (ch != ';' && IS_NULL_PIPE(ctx.list_head)) {
					/* Example: bare "{ }", "()" */
					G.last_exitcode = 2; /* bash compat */
					syntax_error_unexpected_ch(ch);
					ଯାଅ parse_error;
				}
				ଯଦି (heredoc_cnt_ptr)
					*heredoc_cnt_ptr = heredoc_cnt;
				debug_printf_heredoc("parse_stream return heredoc_cnt:%d\n", heredoc_cnt);
				debug_printf_parse("parse_stream return %p: "
						"end_trigger char found\n",
						ctx.list_head);
				debug_leave();
				ଫେରନ୍ତୁ ctx.list_head;
			}
		}

		ଯଦି (is_blank)
			ଜାରି;

		/* Catch <, > before deciding whether this word is
		 * an assignment. a=1 2>z b=2: b=2 is still assignment */
		ବାଛ (ch) {
#ifndef __U_BOOT__
		ସ୍ଥିତି '>':
			redir_fd = redirect_opt_ଅଙ୍କ(&ctx.word);
			ଯଦି (done_word(&ctx)) {
				ଯାଅ parse_error_exitcode1;
			}
			redir_style = REDIRECT_OVERWRITE;
			ଯଦି (next == '>') {
				redir_style = REDIRECT_APPEND;
				ch = i_getch(ନିବେଶ);
				nommu_addchr(&ctx.as_string, ch);
			}
#ଯଦି 0
			ଅନ୍ୟଥା ଯଦି (next == '(') {
				syntax_error(">(process) not supported");
				ଯାଅ parse_error_exitcode1;
			}
#endif
			ଯଦି (parse_redirect(&ctx, redir_fd, redir_style, ନିବେଶ))
				ଯାଅ parse_error_exitcode1;
			ଜାରି; /* get next char */
		ସ୍ଥିତି '<':
			redir_fd = redirect_opt_ଅଙ୍କ(&ctx.word);
			ଯଦି (done_word(&ctx)) {
				ଯାଅ parse_error_exitcode1;
			}
			redir_style = REDIRECT_ନିବେଶ;
			ଯଦି (next == '<') {
				redir_style = REDIRECT_HEREDOC;
				heredoc_cnt++;
				debug_printf_heredoc("++heredoc_cnt=%d\n", heredoc_cnt);
				ch = i_getch(ନିବେଶ);
				nommu_addchr(&ctx.as_string, ch);
			} ଅନ୍ୟଥା ଯଦି (next == '>') {
				redir_style = REDIRECT_IO;
				ch = i_getch(ନିବେଶ);
				nommu_addchr(&ctx.as_string, ch);
			}
#ଯଦି 0
			ଅନ୍ୟଥା ଯଦି (next == '(') {
				syntax_error("<(process) not supported");
				ଯାଅ parse_error_exitcode1;
			}
#endif
			ଯଦି (parse_redirect(&ctx, redir_fd, redir_style, ନିବେଶ))
				ଯାଅ parse_error_exitcode1;
			ଜାରି; /* get next char */
#ଅନ୍ୟଥା /* __U_BOOT__ */
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
			ସ୍ଥିତି '>':
				fallthrough;
			ସ୍ଥିତି '<':
				o_addQchr(&ctx.word, ch);
				ଜାରି;
#endif /* __U_BOOT__ */
		ସ୍ଥିତି '#':
			ଯଦି (ctx.word.length == 0 && !ctx.word.has_quoted_part) {
				/* skip "#comment" */
				/* note: we do not add it to &ctx.as_string */
/* TODO: in bash:
 * comment inside $() goes to the next \n, even inside quoted string (!):
 * cmd "$(cmd2 #comment)" - syntax error
 * cmd "`cmd2 #comment`" - ok
 * We accept both (comment ends where command subst ends, in both cases).
 */
				ଯେତେବେଳେ (1) {
					ch = i_peek(ନିବେଶ);
					ଯଦି (ch == '\n') {
						nommu_addchr(&ctx.as_string, '\n');
						ଭାଙ୍ଗ;
					}
					ch = i_getch(ନିବେଶ);
					ଯଦି (ch == EOF)
						ଭାଙ୍ଗ;
				}
				ଜାରି; /* get next char */
			}
			ଭାଙ୍ଗ;
		}
 skip_end_trigger:

		ଯଦି (ctx.is_assignment == MAYBE_ASSIGNMENT
#ifndef __U_BOOT__
		 /* check that we are not in word in "a=1 2>word b=1": */
		 && !ctx.pending_redirect
#endif /* !__U_BOOT__ */
		) {
			/* ch is a special char and thus this word
			 * cannot be an assignment */
			ctx.is_assignment = NOT_ASSIGNMENT;
			debug_printf_parse("ctx.is_assignment='%s'\n", assignment_flag[ctx.is_assignment]);
		}

		/* Note: nommu_addchr(&ctx.as_string, ch) is already done */

		ବାଛ (ch) {
		case_SPECIAL_VAR_ପ୍ରତୀକ:
		ସ୍ଥିତି SPECIAL_VAR_ପ୍ରତୀକ:
			/* Convert raw ^C to corresponding special variable reference */
			o_addchr(&ctx.word, SPECIAL_VAR_ପ୍ରତୀକ);
			o_addchr(&ctx.word, SPECIAL_VAR_QUOTED_SVS);
			/* fall through */
		ସ୍ଥିତି '#':
			/* non-comment #: "echo a#b" etc */
			o_addchr(&ctx.word, ch);
			ଜାରି; /* get next char */
		ସ୍ଥିତି '$':
#ifndef __U_BOOT__
			ଯଦି (parse_dollar_squote(&ctx.as_string, &ctx.word, ନିବେଶ))
				ଜାରି; /* get next char */
#endif /* !__U_BOOT__ */
			ଯଦି (!parse_dollar(&ctx.as_string, &ctx.word, ନିବେଶ, /*quote_mask:*/ 0)) {
				debug_printf_parse("parse_stream parse error: "
					"parse_dollar returned 0 (error)\n");
				ଯାଅ parse_error_exitcode1;
			}
			ଜାରି; /* get next char */
		ସ୍ଥିତି '"':
			ctx.word.has_quoted_part = 1;
#ifndef __U_BOOT__
			ଯଦି (next == '"' && !ctx.pending_redirect) {
#ଅନ୍ୟଥା /* __U_BOOT__ */
			ଯଦି (next == '"') {
#endif /* __U_BOOT__ */
				i_getch(ନିବେଶ); /* eat second " */
#ifndef __U_BOOT__
 ଭର୍ତ୍ତି_empty_quoted_str_marker:
#endif /* !__U_BOOT__ */
				nommu_addchr(&ctx.as_string, next);
				o_addchr(&ctx.word, SPECIAL_VAR_ପ୍ରତୀକ);
				o_addchr(&ctx.word, SPECIAL_VAR_ପ୍ରତୀକ);
				ଜାରି; /* get next char */
			}
			ଯଦି (ctx.is_assignment == NOT_ASSIGNMENT)
				ctx.word.o_expflags |= EXP_FLAG_ESC_GLOB_CHARS;
			ଯଦି (!encode_string(&ctx.as_string, &ctx.word, ନିବେଶ, '"'))
				ଯାଅ parse_error_exitcode1;
			ctx.word.o_expflags &= ~EXP_FLAG_ESC_GLOB_CHARS;
			ଜାରି; /* get next char */
#ଯଦି ENABLE_HUSH_TICK
		ସ୍ଥିତି '`': {
			USE_FOR_NOMMU(unsigned pos;)

			o_addchr(&ctx.word, SPECIAL_VAR_ପ୍ରତୀକ);
			o_addchr(&ctx.word, '`');
			USE_FOR_NOMMU(pos = ctx.word.length;)
			ଯଦି (!add_till_backquote(&ctx.word, ନିବେଶ, /*in_dquote:*/ 0))
				ଯାଅ parse_error_exitcode1;
# ଯଦି !BB_MMU
			o_addstr(&ctx.as_string, ctx.word.data + pos);
			o_addchr(&ctx.as_string, '`');
# endif
			o_addchr(&ctx.word, SPECIAL_VAR_ପ୍ରତୀକ);
			//debug_printf_subst("SUBST RES3 '%s'\n", ctx.word.data + pos);
			ଜାରି; /* get next char */
		}
#endif
		ସ୍ଥିତି ';':
#ଯଦି ENABLE_HUSH_CASE
 case_semi:
#endif
			ଯଦି (done_word(&ctx)) {
				ଯାଅ parse_error_exitcode1;
			}
			done_pipe(&ctx, PIPE_SEQ);
#ଯଦି ENABLE_HUSH_CASE
			/* Eat multiple semicolons, detect
			 * whether it means something special */
			ଯେତେବେଳେ (1) {
				ch = i_peek_and_eat_bkslash_nl(ନିବେଶ);
				ଯଦି (ch != ';')
					ଭାଙ୍ଗ;
				ch = i_getch(ନିବେଶ);
				nommu_addchr(&ctx.as_string, ch);
				ଯଦି (ctx.ctx_res_w == RES_CASE_BODY) {
					ctx.ctx_dsemicolon = 1;
					ctx.ctx_res_w = RES_MATCH;
					ଭାଙ୍ଗ;
				}
			}
#endif
 new_cmd:
			/* We just finished a cmd. New one may start
			 * with an assignment */
			ctx.is_assignment = MAYBE_ASSIGNMENT;
			debug_printf_parse("ctx.is_assignment='%s'\n", assignment_flag[ctx.is_assignment]);
			ଜାରି; /* get next char */
		ସ୍ଥିତି '&':
			ଯଦି (done_word(&ctx)) {
				ଯାଅ parse_error_exitcode1;
			}
			ଯଦି (next == '&') {
				ch = i_getch(ନିବେଶ);
				nommu_addchr(&ctx.as_string, ch);
				done_pipe(&ctx, PIPE_AND);
			} ଅନ୍ୟଥା {
				done_pipe(&ctx, PIPE_BG);
			}
			ଯାଅ new_cmd;
		ସ୍ଥିତି '|':
			ଯଦି (done_word(&ctx)) {
				ଯାଅ parse_error_exitcode1;
			}
#ଯଦି ENABLE_HUSH_CASE
			ଯଦି (ctx.ctx_res_w == RES_MATCH)
				ଭାଙ୍ଗ; /* we are in case's "word | word)" */
#endif
			ଯଦି (next == '|') { /* || */
				ch = i_getch(ନିବେଶ);
				nommu_addchr(&ctx.as_string, ch);
				done_pipe(&ctx, PIPE_OR);
			} ଅନ୍ୟଥା {
				/* we could pick up a file descriptor choice here
				 * with redirect_opt_num(), but bash doesn't do it.
				 * "echo foo 2| cat" yields "foo 2". */
				done_command(&ctx);
			}
			ଯାଅ new_cmd;
		ସ୍ଥିତି '(':
#ଯଦି ENABLE_HUSH_CASE
			/* "case... in [(]word)..." - skip '(' */
			ଯଦି (ctx.ctx_res_w == RES_MATCH
			 && ctx.command->argv == ଖାଲି /* not (word|(... */
			 && ctx.word.length == 0 /* not word(... */
			 && ctx.word.has_quoted_part == 0 /* not ""(... */
			) {
				ଜାରି; /* get next char */
			}
#endif
			/* fall through */
		ସ୍ଥିତି '{': {
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = parse_group(&ctx, ନିବେଶ, ch);
			ଯଦି (n < 0) {
				ଯାଅ parse_error_exitcode1;
			}
			debug_printf_heredoc("parse_group done, needs heredocs:%d\n", n);
			heredoc_cnt += n;
			ଯାଅ new_cmd;
		}
		ସ୍ଥିତି ')':
#ଯଦି ENABLE_HUSH_CASE
			ଯଦି (ctx.ctx_res_w == RES_MATCH)
				ଯାଅ case_semi;
#endif
		ସ୍ଥିତି '}':
			/* proper use of this character is caught by end_trigger:
			 * if we see {, we call parse_group(..., end_trigger='}')
			 * and it will match } earlier (not here). */
			G.last_exitcode = 2;
			syntax_error_unexpected_ch(ch);
			ଯାଅ parse_error;
		ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
			ଯଦି (HUSH_DEBUG)
				bb_error_ସନ୍ଦେଶ_and_die("BUG: unexpected %c", ch);
		}
	} /* while (1) */

 parse_error_exitcode1:
	G.last_exitcode = 1;
 parse_error:
	{
		ସଂରଚନା parse_context *pctx;
		IF_HAS_KEYWORDS(ସଂରଚନା parse_context *p2;)

		/* Clean up allocated tree.
		 * Sample for finding leaks on syntax error recovery path.
		 * Run it from interactive shell, watch pmap `pidof hush`.
		 * while if false; then false; fi; do break; fi
		 * Samples to catch leaks at execution:
		 * while if (true | { true;}); then echo ok; fi; do break; done
		 * while if (true | { true;}); then echo ok; fi; do (if echo ok; break; then :; fi) | cat; break; done
		 */
		pctx = &ctx;
		କର {
			/* Update pipe/command counts,
			 * otherwise freeing may miss some */
			done_pipe(pctx, PIPE_SEQ);
			debug_printf_clean("freeing list %p from ctx %p\n",
					pctx->list_head, pctx);
			debug_print_ବୃକ୍ଷ(pctx->list_head, 0);
			free_pipe_list(pctx->list_head);
			debug_printf_clean("freed list %p\n", pctx->list_head);
#ଯଦି !BB_MMU
			o_free(&pctx->as_string);
#endif
			IF_HAS_KEYWORDS(p2 = pctx->ଗଦା;)
			ଯଦି (pctx != &ctx) {
				ମୁକ୍ତ(pctx);
			}
			IF_HAS_KEYWORDS(pctx = p2;)
		} ଯେତେବେଳେ (HAS_KEYWORDS && pctx);

		o_free(&ctx.word);
#ଯଦି !BB_MMU
		ଯଦି (pstring)
			*pstring = ଖାଲି;
#endif
		debug_leave();
		ଫେରନ୍ତୁ ERR_PTR;
	}
}

/*** Execution routines ***/

/* Expansion can recurse, need forward decls: */
#ଯଦି !BASH_PATTERN_SUBST && !ENABLE_HUSH_CASE
#define expand_string_to_string(str, EXP_flags, do_unbackslash) \
	expand_string_to_string(str)
#endif
ସ୍ଥିର ଅକ୍ଷର *expand_string_to_string(ଅଚଳ ଅକ୍ଷର *str, ପୂର୍ଣ୍ଣସଂଖ୍ୟା EXP_flags, ପୂର୍ଣ୍ଣସଂଖ୍ୟା do_unbackslash);
#ଯଦି ENABLE_HUSH_TICK
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା process_command_subs(o_string *dest, ଅଚଳ ଅକ୍ଷର *s);
#endif
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା expand_vars_to_list(o_string *output, ପୂର୍ଣ୍ଣସଂଖ୍ୟା n, ଅକ୍ଷର *arg);

/* expand_strvec_to_strvec() takes a list of strings, expands
 * all variable references within and returns a pointer to
 * a list of expanded strings, possibly with larger number
 * of strings. (Think VAR="a b"; echo $VAR).
 * This new list is allocated as a single malloc block.
 * NULL-terminated list of char* pointers is at the beginning of it,
 * followed by strings themselves.
 * Caller can deallocate entire list by single free(list). */

/* A horde of its helpers come first: */

ସ୍ଥିର ଶୂନ୍ୟ o_addblock_duplicate_backslash(o_string *o, ଅଚଳ ଅକ୍ଷର *str, ପୂର୍ଣ୍ଣସଂଖ୍ୟା len)
{
	ଯେତେବେଳେ (--len >= 0) {
		ଅକ୍ଷର c = *str++;

#ଯଦି ENABLE_HUSH_BRACE_EXPANSION
		ଯଦି (c == '{' || c == '}') {
			/* { -> \{, } -> \} */
			o_addchr(o, '\\');
			/* And now we want to add { or } and continue:
			 *  o_addchr(o, c);
			 *  continue;
			 * luckily, just falling through achieves this.
			 */
		}
#endif
		o_addchr(o, c);
		ଯଦି (c == '\\') {
			/* \z -> \\\z; \<eol> -> \\<eol> */
			o_addchr(o, '\\');
			ଯଦି (len) {
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା expand_on_ifs(o_string *output, ପୂର୍ଣ୍ଣସଂଖ୍ୟା n, ଅଚଳ ଅକ୍ଷର *str)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା last_is_ifs = 0;

	ଯେତେବେଳେ (1) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା word_len;

		ଯଦି (!*str)  /* EOL - do not finalize word */
			ଭାଙ୍ଗ;
		word_len = strcspn(str, G.ifs);
		ଯଦି (word_len) {
			/* We have WORD_LEN leading non-IFS chars */
			ଯଦି (!(output->o_expflags & EXP_FLAG_GLOB)) {
				o_addblock(output, str, word_len);
			} ଅନ୍ୟଥା {
				/* Protect backslashes against globbing up :)
				 * Example: "v='\*'; echo b$v" prints "b\*"
				 * (and does not try to glob on "*")
				 */
				o_addblock_duplicate_backslash(output, str, word_len);
				/*/ Why can't we do it easier? */
				/*o_addblock(output, str, word_len); - WRONG: "v='\*'; echo Z$v" prints "Z*" instead of "Z\*" */
				/*o_addqblock(output, str, word_len); - WRONG: "v='*'; echo Z$v" prints "Z*" instead of Z* files */
			}
			last_is_ifs = 0;
			str += word_len;
			ଯଦି (!*str)  /* EOL - do not finalize word */
				ଭାଙ୍ଗ;
		}

		/* We know str here points to at least one IFS char */
		last_is_ifs = 1;
		str += strspn(str, G.ifs_whitespace); /* skip IFS whitespace chars */
		ଯଦି (!*str)  /* EOL - do not finalize word */
			ଭାଙ୍ଗ;

		ଯଦି (G.ifs_whitespace != G.ifs /* usually false ($IFS is usually all whitespace), */
		 && strchr(G.ifs, *str)       /* the second check would fail */
		) {
			/* This is a non-whitespace $IFS char */
			/* Skip it and IFS whitespace chars, start new word */
			str++;
			str += strspn(str, G.ifs_whitespace);
			ଯାଅ new_word;
		}

		/* Start new word... but not always! */
		/* Case "v=' a'; echo ''$v": we do need to finalize empty word: */
		ଯଦି (output->has_quoted_part
		/*
		 * Case "v=' a'; echo $v":
		 * here nothing precedes the space in $v expansion,
		 * therefore we should not finish the word
		 * (IOW: if there *is* word to finalize, only then do it):
		 * It's okay if this accesses the byte before first argv[]:
		 * past call to o_save_ptr() cleared it to zero byte
		 * (grep for -prev-ifs-check-).
		 */
		 || output->data[output->length - 1]
		) {
 new_word:
			o_addchr(output, '\0');
			debug_print_list("expand_on_ifs", output, n);
			n = o_save_ptr(output, n);
		}
	}

	output->ended_in_ifs = last_is_ifs;
	debug_print_list("expand_on_ifs[1]", output, n);
	ଫେରନ୍ତୁ n;
}

#ifndef __U_BOOT__
/* Helper to expand $((...)) and heredoc body. These act as if
 * they are in double quotes, with the exception that they are not :).
 * Just the rules are similar: "expand only $var and `cmd`"
 *
 * Returns malloced string.
 * As an optimization, we return NULL if expansion is not needed.
 */
ସ୍ଥିର ଅକ୍ଷର *encode_then_expand_string(ଅଚଳ ଅକ୍ଷର *str)
{
	ଅକ୍ଷର *exp_str;
	ସଂରଚନା in_str ନିବେଶ;
	o_string dest = NULL_O_STRING;
	ଅଚଳ ଅକ୍ଷର *cp;

	cp = str;
	ପାଇଁ (;;) {
		ଯଦି (!*cp) ଫେରନ୍ତୁ ଖାଲି; /* string has no special chars */
		ଯଦି (*cp == '$') ଭାଙ୍ଗ;
		ଯଦି (*cp == '\\') ଭାଙ୍ଗ;
#ଯଦି ENABLE_HUSH_TICK
		ଯଦି (*cp == '`') ଭାଙ୍ଗ;
#endif
		cp++;
	}

	/* We need to expand. Example:
	 * echo $(($a + `echo 1`)) $((1 + $((2)) ))
	 */
	setup_string_in_str(&ନିବେଶ, str);
	encode_string(ଖାଲି, &dest, &ନିବେଶ, EOF);
//TODO: error check (encode_string returns 0 on error)?
	//bb_error_msg("'%s' -> '%s'", str, dest.data);
	exp_str = expand_string_to_string(dest.data,
			EXP_FLAG_ESC_GLOB_CHARS,
			/*unbackslash:*/ 1
	);
	//bb_error_msg("'%s' -> '%s'", dest.data, exp_str);
	o_free(&dest);
	ଫେରନ୍ତୁ exp_str;
}

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *first_special_char_in_vararg(ଅଚଳ ଅକ୍ଷର *cp)
{
	ପାଇଁ (;;) {
		ଯଦି (!*cp) ଫେରନ୍ତୁ ଖାଲି; /* string has no special chars */
		ଯଦି (*cp == '$') ଫେରନ୍ତୁ cp;
		ଯଦି (*cp == '\\') ଫେରନ୍ତୁ cp;
		ଯଦି (*cp == '\'') ଫେରନ୍ତୁ cp;
		ଯଦି (*cp == '"') ଫେରନ୍ତୁ cp;
#ଯଦି ENABLE_HUSH_TICK
		ଯଦି (*cp == '`') ଫେରନ୍ତୁ cp;
#endif
		/* dquoted "${x:+ARG}" should not glob, therefore
		 * '*' et al require some non-literal processing: */
		ଯଦି (*cp == '*') ଫେରନ୍ତୁ cp;
		ଯଦି (*cp == '?') ଫେରନ୍ତୁ cp;
		ଯଦି (*cp == '[') ଫେରନ୍ତୁ cp;
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
#ଯଦି !BASH_PATTERN_SUBST
#define encode_then_expand_vararg(str, handle_squotes, do_unbackslash) \
	encode_then_expand_vararg(str, handle_squotes)
#endif
ସ୍ଥିର ଅକ୍ଷର *encode_then_expand_vararg(ଅଚଳ ଅକ୍ଷର *str, ପୂର୍ଣ୍ଣସଂଖ୍ୟା handle_squotes, ପୂର୍ଣ୍ଣସଂଖ୍ୟା do_unbackslash)
{
#ଯଦି !BASH_PATTERN_SUBST && ENABLE_HUSH_CASE
	ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା do_unbackslash = 0;
#endif
	ଅକ୍ଷର *exp_str;
	ସଂରଚନା in_str ନିବେଶ;
	o_string dest = NULL_O_STRING;

	ଯଦି (!first_special_char_in_vararg(str)) {
		/* string has no special chars */
		ଫେରନ୍ତୁ ଖାଲି;
	}

	setup_string_in_str(&ନିବେଶ, str);
	dest.data = xzalloc(1); /* start as "", not as NULL */
	exp_str = ଖାଲି;

	ପାଇଁ (;;) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା ch;

		ch = i_getch(&ନିବେଶ);
		debug_printf_parse("%s: ch=%c (%d) escape=%d\n",
				__func__, ch, ch, !!dest.o_expflags);

		ଯଦି (!dest.o_expflags) {
			ଯଦି (ch == EOF)
				ଭାଙ୍ଗ;
			ଯଦି (handle_squotes && ch == '\'') {
				ଯଦି (!add_till_single_quote_dquoted(&dest, &ନିବେଶ))
					ଯାଅ ret; /* error */
				ଜାରି;
			}
		}
		ଯଦି (ch == EOF) {
			syntax_error_unterm_ch('"');
			ଯାଅ ret; /* error */
		}
		ଯଦି (ch == '"') {
			dest.o_expflags ^= EXP_FLAG_ESC_GLOB_CHARS;
			ଜାରି;
		}
		ଯଦି (ch == '\\') {
			ch = i_getch(&ନିବେଶ);
			ଯଦି (ch == EOF) {
//example? error message?	syntax_error_unterm_ch('"');
				debug_printf_parse("%s: error: \\<eof>\n", __func__);
				ଯାଅ ret;
			}
			o_addqchr(&dest, ch);
			ଜାରି;
		}
		ଯଦି (ch == '$') {
			ଯଦି (parse_dollar_squote(ଖାଲି, &dest, &ନିବେଶ))
				ଜାରି;
			ଯଦି (!parse_dollar(ଖାଲି, &dest, &ନିବେଶ, /*quote_mask:*/ 0x80)) {
				debug_printf_parse("%s: error: parse_dollar returned 0 (error)\n", __func__);
				ଯାଅ ret;
			}
			ଜାରି;
		}
#ଯଦି ENABLE_HUSH_TICK
		ଯଦି (ch == '`') {
			//unsigned pos = dest->length;
			o_addchr(&dest, SPECIAL_VAR_ପ୍ରତୀକ);
			o_addchr(&dest, 0x80 | '`');
			ଯଦି (!add_till_backquote(&dest, &ନିବେଶ,
					/*in_dquote:*/ dest.o_expflags /* nonzero if EXP_FLAG_ESC_GLOB_CHARS set */
				)
			) {
				ଯାଅ ret; /* error */
			}
			o_addchr(&dest, SPECIAL_VAR_ପ୍ରତୀକ);
			//debug_printf_subst("SUBST RES3 '%s'\n", dest->data + pos);
			ଜାରି;
		}
#endif
		o_addQchr(&dest, ch);
	} /* for (;;) */

	debug_printf_parse("encode: '%s' -> '%s'\n", str, dest.data);
	exp_str = expand_string_to_string(dest.data,
			do_unbackslash ? EXP_FLAG_ESC_GLOB_CHARS : 0,
			do_unbackslash
	);
 ret:
	debug_printf_parse("expand: '%s' -> '%s'\n", dest.data, exp_str);
	o_free(&dest);
	ଫେରନ୍ତୁ exp_str;
}

/* Expanding ARG in ${var+ARG}, ${var-ARG}
 */
ସ୍ଥିର NOINLINE ପୂର୍ଣ୍ଣସଂଖ୍ୟା encode_then_append_var_plusminus(o_string *output, ପୂର୍ଣ୍ଣସଂଖ୍ୟା n,
		ଅକ୍ଷର *str, ପୂର୍ଣ୍ଣସଂଖ୍ୟା dquoted)
{
	ସଂରଚନା in_str ନିବେଶ;
	o_string dest = NULL_O_STRING;

	ଯଦି (!first_special_char_in_vararg(str)
	 && '\0' == str[strcspn(str, G.ifs)]
	) {
		/* string has no special chars
		 * && string has no $IFS chars
		 */
		ଯଦି (dquoted) {
			/* Prints 1 (quoted expansion is a "" word, not nothing):
			 * set -- "${notexist-}"; echo $#
			 */
			output->has_quoted_part = 1;
		}
		ଫେରନ୍ତୁ expand_vars_to_list(output, n, str);
	}

	setup_string_in_str(&ନିବେଶ, str);

	ପାଇଁ (;;) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା ch;

		ch = i_getch(&ନିବେଶ);
		debug_printf_parse("%s: ch=%c (%d) escape=%x\n",
				__func__, ch, ch, dest.o_expflags);

		ଯଦି (!dest.o_expflags) {
			ଯଦି (ch == EOF)
				ଭାଙ୍ଗ;
			ଯଦି (!dquoted && !(output->o_expflags & EXP_FLAG_SINGLEWORD) && strchr(G.ifs, ch)) {
				/* PREFIX${x:d${e}f ...} and we met space: expand "d${e}f" and start new word.
				 * do not assume we are at the start of the word (PREFIX above).
				 */
				ଯଦି (dest.data) {
					n = expand_vars_to_list(output, n, dest.data);
					o_free_and_set_NULL(&dest);
					o_addchr(output, '\0');
					n = o_save_ptr(output, n); /* create next word */
				} ଅନ୍ୟଥା
				ଯଦି (output->length != o_get_last_ptr(output, n)
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
				ଜାରି;
			}
			ଯଦି (!dquoted && ch == '\'') {
				ଯଦି (!add_till_single_quote_dquoted(&dest, &ନିବେଶ))
					ଯାଅ ret; /* error */
				o_addchr(&dest, SPECIAL_VAR_ପ୍ରତୀକ);
				o_addchr(&dest, SPECIAL_VAR_ପ୍ରତୀକ);
				ଜାରି;
			}
		}
		ଯଦି (ch == EOF) {
			syntax_error_unterm_ch('"');
			ଯାଅ ret; /* error */
		}
		ଯଦି (ch == '"') {
			dest.o_expflags ^= EXP_FLAG_ESC_GLOB_CHARS;
			ଯଦି (dest.o_expflags) {
				o_addchr(&dest, SPECIAL_VAR_ପ୍ରତୀକ);
				o_addchr(&dest, SPECIAL_VAR_ପ୍ରତୀକ);
			}
			ଜାରି;
		}
		ଯଦି (ch == '\\') {
			ch = i_getch(&ନିବେଶ);
			ଯଦି (ch == EOF) {
//example? error message?	syntax_error_unterm_ch('"');
				debug_printf_parse("%s: error: \\<eof>\n", __func__);
				ଯାଅ ret;
			}
			o_addqchr(&dest, ch);
			ଜାରି;
		}
		ଯଦି (ch == '$') {
			ଯଦି (!parse_dollar(ଖାଲି, &dest, &ନିବେଶ, /*quote_mask:*/ (dest.o_expflags || dquoted) ? 0x80 : 0)) {
				debug_printf_parse("%s: error: parse_dollar returned 0 (error)\n", __func__);
				ଯାଅ ret;
			}
			ଜାରି;
		}
#ଯଦି ENABLE_HUSH_TICK
		ଯଦି (ch == '`') {
			//unsigned pos = dest->length;
			o_addchr(&dest, SPECIAL_VAR_ପ୍ରତୀକ);
			o_addchr(&dest, (dest.o_expflags || dquoted) ? 0x80 | '`' : '`');
			ଯଦି (!add_till_backquote(&dest, &ନିବେଶ,
					/*in_dquote:*/ dest.o_expflags /* nonzero if EXP_FLAG_ESC_GLOB_CHARS set */
				)
			) {
				ଯାଅ ret; /* error */
			}
			o_addchr(&dest, SPECIAL_VAR_ପ୍ରତୀକ);
			//debug_printf_subst("SUBST RES3 '%s'\n", dest->data + pos);
			ଜାରି;
		}
#endif
		ଯଦି (dquoted) {
			/* Always glob-protect if in dquotes:
			 * x=x; echo "${x:+/bin/c*}" - prints: /bin/c*
			 * x=x; echo "${x:+"/bin/c*"}" - prints: /bin/c*
			 */
			o_addqchr(&dest, ch);
		} ଅନ୍ୟଥା {
			/* Glob-protect only if char is quoted:
			 * x=x; echo ${x:+/bin/c*} - prints many filenames
			 * x=x; echo ${x:+"/bin/c*"} - prints: /bin/c*
			 */
			o_addQchr(&dest, ch);
		}
	} /* for (;;) */

	ଯଦି (dest.data) {
		n = expand_vars_to_list(output, n, dest.data);
	}
 ret:
	o_free(&dest);
	ଫେରନ୍ତୁ n;
}
#endif /* !__U_BOOT__ */

#ifndef __U_BOOT__
#ଯଦି ENABLE_FEATURE_SH_MATH
ସ୍ଥିର arith_t expand_and_evaluate_arith(ଅଚଳ ଅକ୍ଷର *arg, ଅଚଳ ଅକ୍ଷର **errmsg_p)
{
	arith_state_t math_state;
	arith_t res;
	ଅକ୍ଷର *exp_str;

	math_state.lookupvar = get_local_var_ମୂଲ୍ୟ;
	math_state.setvar = set_local_var_from_halves;
	//math_state.endofname = endofname;
	exp_str = encode_then_expand_string(arg);
	res = arith(&math_state, exp_str ? exp_str : arg);
	ମୁକ୍ତ(exp_str);
	ଯଦି (errmsg_p)
		*errmsg_p = math_state.errmsg;
	ଯଦି (math_state.errmsg)
		ସନ୍ଦେଶ_and_die_if_script(math_state.errmsg);
	ଫେରନ୍ତୁ res;
}
#endif
#endif /* !__U_BOOT__ */

#ifndef __U_BOOT__
#ଯଦି BASH_PATTERN_SUBST
/* ${var/[/]pattern[/repl]} helpers */
ସ୍ଥିର ଅକ୍ଷର *strstr_pattern(ଅକ୍ଷର *val, ଅଚଳ ଅକ୍ଷର *pattern, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *size)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା first_escaped = (pattern[0] == '\\' && pattern[1]);
	/* "first_escaped" trick allows to treat e.g. "\*no_glob_chars"
	 * as literal too (as it is semi-common, and easy to accomodate
	 * by just using str + 1).
	 */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା sz = strcspn(pattern + first_escaped * 2, "*?[\\");
	ଯଦି ((pattern + first_escaped * 2)[sz] == '\0') {
		/* Optimization for trivial patterns.
		 * Testcase for very slow replace (performs about 22k replaces):
		 * x=::::::::::::::::::::::
		 * x=$x$x;x=$x$x;x=$x$x;x=$x$x;x=$x$x;x=$x$x;x=$x$x;x=$x$x;x=$x$x;x=$x$x;echo ${#x}
		 * echo "${x//:/|}"
		 */
		*size = sz + first_escaped;
		ଫେରନ୍ତୁ strstr(val, pattern + first_escaped);
	}

	ଯେତେବେଳେ (1) {
		ଅକ୍ଷର *end = scan_and_match(val, pattern, SCAN_MOVE_FROM_ଡାହାଣ + SCAN_MATCH_ବାମ_HALF);
		debug_printf_varexp("val:'%s' pattern:'%s' end:'%s'\n", val, pattern, end);
		ଯଦି (end) {
			*size = end - val;
			ଫେରନ୍ତୁ val;
		}
		ଯଦି (*val == '\0')
			ଫେରନ୍ତୁ ଖାଲି;
		/* Optimization: if "*pat" did not match the start of "string",
		 * we know that "tring", "ring" etc will not match too:
		 */
		ଯଦି (pattern[0] == '*')
			ଫେରନ୍ତୁ ଖାଲି;
		val++;
	}
}
ସ୍ଥିର ଅକ୍ଷର *replace_pattern(ଅକ୍ଷର *val, ଅଚଳ ଅକ୍ଷର *pattern, ଅଚଳ ଅକ୍ଷର *repl, ଅକ୍ଷର exp_op)
{
	ଅକ୍ଷର *result = ଖାଲି;
	unsigned res_len = 0;
	unsigned repl_len = strlen(repl);

	/* Null pattern never matches, including if "var" is empty */
	ଯଦି (!pattern[0])
		ଫେରନ୍ତୁ result; /* NULL, no replaces happened */

	ଯେତେବେଳେ (1) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା size;
		ଅକ୍ଷର *s = strstr_pattern(val, pattern, &size);
		ଯଦି (!s)
			ଭାଙ୍ଗ;

		result = xrealloc(result, res_len + (s - val) + repl_len + 1);
		strcpy(mempcpy(result + res_len, val, s - val), repl);
		res_len += (s - val) + repl_len;
		debug_printf_varexp("val:'%s' s:'%s' result:'%s'\n", val, s, result);

		val = s + size;
		ଯଦି (exp_op == '/')
			ଭାଙ୍ଗ;
	}
	ଯଦି (*val && result) {
		result = xrealloc(result, res_len + strlen(val) + 1);
		strcpy(result + res_len, val);
		debug_printf_varexp("val:'%s' result:'%s'\n", val, result);
	}
	debug_printf_varexp("result:'%s'\n", result);
	ଫେରନ୍ତୁ result;
}
#endif /* BASH_PATTERN_SUBST */
#endif /* !__U_BOOT__ */

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା append_str_maybe_ifs_split(o_string *output, ପୂର୍ଣ୍ଣସଂଖ୍ୟା n,
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା first_ch, ଅଚଳ ଅକ୍ଷର *val)
{
	ଯଦି (!(first_ch & 0x80)) { /* unquoted $VAR */
		debug_printf_expand("unquoted '%s', output->o_escape:%d\n", val,
				!!(output->o_expflags & EXP_FLAG_ESC_GLOB_CHARS));
		ଯଦି (val && val[0])
			n = expand_on_ifs(output, n, val);
	} ଅନ୍ୟଥା { /* quoted "$VAR" */
		output->has_quoted_part = 1;
		debug_printf_expand("quoted '%s', output->o_escape:%d\n", val,
				!!(output->o_expflags & EXP_FLAG_ESC_GLOB_CHARS));
		ଯଦି (val && val[0])
			o_addQstr(output, val);
	}
	ଫେରନ୍ତୁ n;
}

/* Handle <SPECIAL_VAR_SYMBOL>varname...<SPECIAL_VAR_SYMBOL> construct.
 */
ସ୍ଥିର NOINLINE ପୂର୍ଣ୍ଣସଂଖ୍ୟା expand_one_var(o_string *output, ପୂର୍ଣ୍ଣସଂଖ୍ୟା n,
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା first_ch, ଅକ୍ଷର *arg, ଅକ୍ଷର **pp)
{
	ଅଚଳ ଅକ୍ଷର *val;
	ଅକ୍ଷର *to_be_freed;
	ଅକ୍ଷର *p;
	ଅକ୍ଷର *var;
	ଅକ୍ଷର exp_op;
	ଅକ୍ଷର exp_save = exp_save; /* for compiler */
	ଅକ୍ଷର *exp_saveptr; /* points to expansion operator */
	ଅକ୍ଷର *exp_word = exp_word; /* for compiler */
	ଅକ୍ଷର arg0;

	val = ଖାଲି;
	to_be_freed = ଖାଲି;
	p = *pp;
	*p = '\0'; /* replace trailing SPECIAL_VAR_SYMBOL */
	var = arg;
	exp_saveptr = arg[1] ? strchr(VAR_ENCODED_SUBST_OPS, arg[1]) : ଖାଲି;
	arg0 = arg[0];
	arg[0] = (arg0 & 0x7f);
	exp_op = 0;

	ଯଦି (arg[0] == '#' && arg[1] /* ${#...} but not ${#} */
	 && (!exp_saveptr               /* and ( not(${#<op_char>...}) */
	    || (arg[2] == '\0' && strchr(SPECIAL_VARS_STR, arg[1])) /* or ${#C} "len of $C" ) */
	    )		/* NB: skipping ^^^specvar check mishandles ${#::2} */
	) {
		/* It must be length operator: ${#var} */
		var++;
		exp_op = 'L';
	} ଅନ୍ୟଥା {
		/* Maybe handle parameter expansion */
		ଯଦି (exp_saveptr /* if 2nd char is one of expansion operators */
		 && strchr(NUMERIC_SPECVARS_STR, arg[0]) /* 1st char is special variable */
		) {
			/* ${?:0}, ${#[:]%0} etc */
			exp_saveptr = var + 1;
		} ଅନ୍ୟଥା {
			/* ${?}, ${var}, ${var:0}, ${var[:]%0} etc */
			exp_saveptr = var+1 + strcspn(var+1, VAR_ENCODED_SUBST_OPS);
		}
		exp_op = exp_save = *exp_saveptr;
#ifndef __U_BOOT__
		ଯଦି (exp_op) {
			exp_word = exp_saveptr + 1;
			ଯଦି (exp_op == ':') {
				exp_op = *exp_word++;
//TODO: try ${var:} and ${var:bogus} in non-bash config
				ଯଦି (BASH_SUBSTR
				 && (!exp_op || !strchr(MINUS_PLUS_EQUAL_QUESTION, exp_op))
				) {
					/* oops... it's ${var:N[:M]}, not ${var:?xxx} or some such */
					exp_op = ':';
					exp_word--;
				}
			}
			*exp_saveptr = '\0';
		} /* else: it's not an expansion op, but bare ${var} */
#endif /* !__U_BOOT__ */
	}

	/* Look up the variable in question */
	ଯଦି (isdigit(var[0])) {
		/* parse_dollar should have vetted var for us */
#ifndef __U_BOOT__
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା nn = xatoi_positive(var);
#ଅନ୍ୟଥା /* __U_BOOT__ */
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା nn = simple_strtoul(var, ଖାଲି, 10);
#endif /* __U_BOOT__ */
		ଯଦି (nn < G.global_argc)
			val = G.global_argv[nn];
		/* else val remains NULL: $N with too big N */
	} ଅନ୍ୟଥା {
		ବାଛ (var[0]) {
#ifndef __U_BOOT__
		ସ୍ଥିତି '$': /* pid */
			val = utoa(G.ମୂଳ_pid);
			ଭାଙ୍ଗ;
		ସ୍ଥିତି '!': /* bg pid */
			val = G.last_bg_pid ? utoa(G.last_bg_pid) : "";
			ଭାଙ୍ଗ;
#endif /* !__U_BOOT__ */
		ସ୍ଥିତି '?': /* exitcode */
			val = utoa(G.last_exitcode);
			ଭାଙ୍ଗ;
		ସ୍ଥିତି '#': /* argc */
			val = utoa(G.global_argc ? G.global_argc-1 : 0);
			ଭାଙ୍ଗ;
#ifndef __U_BOOT__
		ସ୍ଥିତି '-': { /* active options */
			/* Check set_mode() to see what option chars we support */
			ଅକ୍ଷର *cp;
			val = cp = G.optstring_buf;
			ଯଦି (G.o_opt[OPT_O_ERREXIT])
				*cp++ = 'e';
			ଯଦି (G_interactive_fd)
				*cp++ = 'i';
			ଯଦି (G_x_mode)
				*cp++ = 'x';
			/* If G.o_opt[OPT_O_NOEXEC] is true,
			 * commands read but are not executed,
			 * so $- can not execute too, 'n' is never seen in $-.
			 */
			ଯଦି (G.opt_c)
				*cp++ = 'c';
			ଯଦି (G.opt_s)
				*cp++ = 's';
			*cp = '\0';
			ଭାଙ୍ଗ;
		}
#endif /* !__U_BOOT__ */
		ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
#ifndef __U_BOOT__
			val = get_local_var_ମୂଲ୍ୟ(var);
#ଅନ୍ୟଥା /* __U_BOOT__ */
			/*
			 * Environment variable set with setenv* have to be
			 * expanded.
			 * So, we first search if the variable exists in
			 * environment, if this is not the case, we default to
			 * local value.
			 */
			val = env_get(var);
			ଯଦି (!val)
				val = get_local_var_ମୂଲ୍ୟ(var);
#endif /* __U_BOOT__ */
		}
	}

#ifndef __U_BOOT__
	/* Handle any expansions */
	ଯଦି (exp_op == 'L') {
		reinit_unicode_for_hush();
		debug_printf_expand("expand: length(%s)=", val);
		val = utoa(val ? unicode_strlen(val) : 0);
		debug_printf_expand("%s\n", val);
	} ଅନ୍ୟଥା ଯଦି (exp_op) {
		ଯଦି (exp_op == '%' || exp_op == '#') {
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
			ଯଦି (val && val[0]) {
				ଅକ୍ଷର *t;
				ଅକ୍ଷର *exp_exp_word;
				ଅକ୍ଷର *loc;
				unsigned scan_flags = pick_scan(exp_op, *exp_word);
				ଯଦି (exp_op == *exp_word)  /* ## or %% */
					exp_word++;
				debug_printf_expand("expand: exp_word:'%s'\n", exp_word);
				exp_exp_word = encode_then_expand_vararg(exp_word, /*handle_squotes:*/ 1, /*unbackslash:*/ 0);
				ଯଦି (exp_exp_word)
					exp_word = exp_exp_word;
				debug_printf_expand("expand: exp_word:'%s'\n", exp_word);
				/*
				 * HACK ALERT. We depend here on the fact that
				 * G.global_argv and results of utoa and get_local_var_value
				 * are actually in writable memory:
				 * scan_and_match momentarily stores NULs there.
				 */
				t = (ଅକ୍ଷର*)val;
				loc = scan_and_match(t, exp_word, scan_flags);
				debug_printf_expand("op:%c str:'%s' pat:'%s' res:'%s'\n", exp_op, t, exp_word, loc);
				ମୁକ୍ତ(exp_exp_word);
				ଯଦି (loc) { /* match was found */
					ଯଦି (scan_flags & SCAN_MATCH_ବାମ_HALF) /* #[#] */
						val = loc; /* take right part */
					ଅନ୍ୟଥା /* %[%] */
						val = to_be_freed = xstrndup(val, loc - val); /*ବାମ*/
				}
			}
		}
#ଯଦି BASH_PATTERN_SUBST
		ଅନ୍ୟଥା ଯଦି (exp_op == '/' || exp_op == '\\') {
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
			ଯଦି (val /*&& val[0]*/) {
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
				ଅକ୍ଷର *pattern, *repl, *t;
				pattern = encode_then_expand_vararg(exp_word, /*handle_squotes:*/ 1, /*unbackslash:*/ 0);
				ଯଦି (!pattern)
					pattern = xstrdup(exp_word);
				debug_printf_varexp("pattern:'%s'->'%s'\n", exp_word, pattern);
				*p++ = SPECIAL_VAR_ପ୍ରତୀକ;
				exp_word = p;
				p = strchr(p, SPECIAL_VAR_ପ୍ରତୀକ);
				*p = '\0';
				repl = encode_then_expand_vararg(exp_word, /*handle_squotes:*/ 1, /*unbackslash:*/ 1);
				debug_printf_varexp("repl:'%s'->'%s'\n", exp_word, repl);
				/* HACK ALERT. We depend here on the fact that
				 * G.global_argv and results of utoa and get_local_var_value
				 * are actually in writable memory:
				 * replace_pattern momentarily stores NULs there. */
				t = (ଅକ୍ଷର*)val;
				to_be_freed = replace_pattern(t,
						pattern,
						(repl ? repl : exp_word),
						exp_op);
				ଯଦି (to_be_freed) /* at least one replace happened */
					val = to_be_freed;
				ମୁକ୍ତ(pattern);
				ମୁକ୍ତ(repl);
			} ଅନ୍ୟଥା {
				/* Unset variable always gives nothing */
				//  a=; echo ${a/*/w}      # "w"
				//  unset a; echo ${a/*/w} # ""
				/* Just skip "replace" part */
				*p++ = SPECIAL_VAR_ପ୍ରତୀକ;
				p = strchr(p, SPECIAL_VAR_ପ୍ରତୀକ);
				*p = '\0';
			}
		}
#endif /* BASH_PATTERN_SUBST */
		ଅନ୍ୟଥା ଯଦି (exp_op == ':') {
#ଯଦି BASH_SUBSTR && ENABLE_FEATURE_SH_MATH
			/* It's ${var:N[:M]} bashism.
			 * Note that in encoded form it has TWO parts:
			 * var:N<SPECIAL_VAR_SYMBOL>M<SPECIAL_VAR_SYMBOL>
			 */
			arith_t beg, len;
			unsigned vallen;
			ଅଚଳ ଅକ୍ଷର *errmsg;

			beg = expand_and_evaluate_arith(exp_word, &errmsg);
			ଯଦି (errmsg)
				ଯାଅ empty_result;
			debug_printf_varexp("beg:'%s'=%lld\n", exp_word, (long long)beg);
			*p++ = SPECIAL_VAR_ପ୍ରତୀକ;
			exp_word = p;
			p = strchr(p, SPECIAL_VAR_ପ୍ରତୀକ);
			*p = '\0';
			vallen = val ? strlen(val) : 0;
			ଯଦି (beg < 0) {
				/* negative beg counts from the end */
				beg = (arith_t)vallen + beg;
			}
			/* If expansion will be empty, do not even evaluate len */
			ଯଦି (!val || beg < 0 || beg > vallen) {
				/* Why > vallen, not >=? bash:
				 * unset b; a=ab; : ${a:2:${b=3}}; echo $b  # "", b=3 (!!!)
				 * unset b; a=a; : ${a:2:${b=3}}; echo $b   # "", b not set
				 */
				ଯାଅ empty_result;
			}
			len = expand_and_evaluate_arith(exp_word, &errmsg);
			ଯଦି (errmsg)
				ଯାଅ empty_result;
			debug_printf_varexp("len:'%s'=%lld\n", exp_word, (long long)len);
			debug_printf_varexp("from val:'%s'\n", val);
			ଯଦି (len < 0) {
				/* in bash, len=-n means strlen()-n */
				len = (arith_t)vallen - beg + len;
				ଯଦି (len < 0) /* bash compat */
					ସନ୍ଦେଶ_and_die_if_script("%s: substring expression < 0", var);
			}
			ଯଦି (len <= 0 || !val /*|| beg >= vallen*/) {
 empty_result:
				val = ଖାଲି;
			} ଅନ୍ୟଥା {
				/* Paranoia. What if user entered 9999999999999
				 * which fits in arith_t but not int? */
				ଯଦି (len > INT_MAX)
					len = INT_MAX;
				val = to_be_freed = xstrndup(val + beg, len);
			}
			debug_printf_varexp("val:'%s'\n", val);
#ଅନ୍ୟଥା /* not (HUSH_SUBSTR_EXPANSION && FEATURE_SH_MATH) */
			ସନ୍ଦେଶ_and_die_if_script("malformed ${%s:...}", var);
			val = ଖାଲି;
#endif
		} ଅନ୍ୟଥା { /* one of "-=+?" */
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
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା use_word = (!val || ((exp_save == ':') && !val[0]));
			ଯଦି (exp_op == '+')
				use_word = !use_word;
			debug_printf_expand("expand: op:%c (null:%s) test:%i\n", exp_op,
					(exp_save == ':') ? "true" : "false", use_word);
			ଯଦି (use_word) {
				ଯଦି (exp_op == '+' || exp_op == '-') {
					/* ${var+word} - use alternative value */
					/* ${var-word} - use default value */
					n = encode_then_append_var_plusminus(output, n, exp_word,
							/*dquoted:*/ (arg0 & 0x80)
					);
					val = ଖାଲି;
				} ଅନ୍ୟଥା {
					/* ${var?word} - indicate error if unset */
					/* ${var=word} - assign and use default value */
					to_be_freed = encode_then_expand_vararg(exp_word,
							/*handle_squotes:*/ !(arg0 & 0x80),
							/*unbackslash:*/ 0
					);
					ଯଦି (to_be_freed)
						exp_word = to_be_freed;
					ଯଦି (exp_op == '?') {
						/* mimic bash message */
						ସନ୍ଦେଶ_and_die_if_script("%s: %s",
							var,
							exp_word[0]
							? exp_word
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
					} ଅନ୍ୟଥା {
						val = exp_word;
					}
					ଯଦି (exp_op == '=') {
						/* ${var=[word]} or ${var:=[word]} */
						ଯଦି (isdigit(var[0]) || var[0] == '#') {
							/* mimic bash message */
							ସନ୍ଦେଶ_and_die_if_script("$%s: cannot assign in this way", var);
							val = ଖାଲି;
						} ଅନ୍ୟଥା {
							ଅକ୍ଷର *new_var = xasprintf("%s=%s", var, val);
							set_local_var0(new_var);
						}
					}
				}
			}
		} /* one of "-=+?" */

		*exp_saveptr = exp_save;
	} /* if (exp_op) */

#endif /* !__U_BOOT__ */
	arg[0] = arg0;
	*pp = p;

	n = append_str_maybe_ifs_split(output, n, first_ch, val);

	ମୁକ୍ତ(to_be_freed);
	ଫେରନ୍ତୁ n;
}

/* Expand all variable references in given string, adding words to list[]
 * at n, n+1,... positions. Return updated n (so that list[n] is next one
 * to be filled). This routine is extremely tricky: has to deal with
 * variables/parameters with whitespace, $* and $@, and constructs like
 * 'echo -$*-'. If you play here, you must run testsuite afterwards! */
ସ୍ଥିର NOINLINE ପୂର୍ଣ୍ଣସଂଖ୍ୟା expand_vars_to_list(o_string *output, ପୂର୍ଣ୍ଣସଂଖ୍ୟା n, ଅକ୍ଷର *arg)
{
	/* output->o_expflags & EXP_FLAG_SINGLEWORD (0x80) if we are in
	 * expansion of right-hand side of assignment == 1-element expand.
	 */
	ଅକ୍ଷର cant_be_null = 0; /* only bit 0x80 matters */
	ଅକ୍ଷର *p;

	debug_printf_expand("expand_vars_to_list: arg:'%s' singleword:%x\n", arg,
			!!(output->o_expflags & EXP_FLAG_SINGLEWORD));
	debug_print_list("expand_vars_to_list[0]", output, n);

	ଯେତେବେଳେ ((p = strchr(arg, SPECIAL_VAR_ପ୍ରତୀକ)) != ଖାଲି) {
		ଅକ୍ଷର first_ch;
#ଯଦି ENABLE_FEATURE_SH_MATH
		ଅକ୍ଷର arith_buf[ଆକାର(arith_t)*3 + 2];
#endif

		ଯଦି (output->ended_in_ifs) {
			o_addchr(output, '\0');
			n = o_save_ptr(output, n);
			output->ended_in_ifs = 0;
		}

		o_addblock(output, arg, p - arg);
		debug_print_list("expand_vars_to_list[1]", output, n);
		arg = ++p;
		p = strchr(p, SPECIAL_VAR_ପ୍ରତୀକ);

		/* Fetch special var name (if it is indeed one of them)
		 * and quote bit, force the bit on if singleword expansion -
		 * important for not getting v=$@ expand to many words. */
		first_ch = arg[0] | (output->o_expflags & EXP_FLAG_SINGLEWORD);

		/* Is this variable quoted and thus expansion can't be null?
		 * "$@" is special. Even if quoted, it can still
		 * expand to nothing (not even an empty string),
		 * thus it is excluded. */
		ଯଦି ((first_ch & 0x7f) != '@')
			cant_be_null |= first_ch;

		ବାଛ (first_ch & 0x7f) {
		/* Highest bit in first_ch indicates that var is double-quoted */
		ସ୍ଥିତି '*':
		ସ୍ଥିତି '@': {
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
#ifndef __U_BOOT__
			ଯଦି (!G.global_argv[1])
#ଅନ୍ୟଥା /* __U_BOOT__ */
			ଯଦି (!G.global_argv || !G.global_argv[1])
#endif /* __U_BOOT__ */
				ଭାଙ୍ଗ;
			i = 1;
			cant_be_null |= first_ch; /* do it for "$@" _now_, when we know it's not empty */
			ଯଦି (!(first_ch & 0x80)) { /* unquoted $* or $@ */
				ଯେତେବେଳେ (G.global_argv[i]) {
					n = expand_on_ifs(output, n, G.global_argv[i]);
					debug_printf_expand("expand_vars_to_list: argv %d (last %d)\n", i, G.global_argc - 1);
					ଯଦି (G.global_argv[i++][0] && G.global_argv[i]) {
						/* this argv[] is not empty and not last:
						 * put terminating NUL, start new word */
						o_addchr(output, '\0');
						debug_print_list("expand_vars_to_list[2]", output, n);
						n = o_save_ptr(output, n);
						debug_print_list("expand_vars_to_list[3]", output, n);
					}
				}
			} ଅନ୍ୟଥା
			/* If EXP_FLAG_SINGLEWORD, we handle assignment 'a=....$@.....'
			 * and in this case should treat it like '$*' - see 'else...' below */
			ଯଦି (first_ch == (ଅକ୍ଷର)('@'|0x80)  /* quoted $@ */
			 && !(output->o_expflags & EXP_FLAG_SINGLEWORD) /* not v="$@" case */
			) {
				ଯେତେବେଳେ (1) {
					o_addQstr(output, G.global_argv[i]);
					ଯଦି (++i >= G.global_argc)
						ଭାଙ୍ଗ;
					o_addchr(output, '\0');
					debug_print_list("expand_vars_to_list[4]", output, n);
					n = o_save_ptr(output, n);
				}
			} ଅନ୍ୟଥା { /* quoted $* (or v="$@" case): add as one word */
				ଯେତେବେଳେ (1) {
					o_addQstr(output, G.global_argv[i]);
					ଯଦି (!G.global_argv[++i])
						ଭାଙ୍ଗ;
					ଯଦି (G.ifs[0])
						o_addchr(output, G.ifs[0]);
				}
				output->has_quoted_part = 1;
			}
			ଭାଙ୍ଗ;
		}
		ସ୍ଥିତି SPECIAL_VAR_ପ୍ରତୀକ: {
			/* <SPECIAL_VAR_SYMBOL><SPECIAL_VAR_SYMBOL> */
			/* "Empty variable", used to make "" etc to not disappear */
			output->has_quoted_part = 1;
			cant_be_null = 0x80;
			arg++;
			ଭାଙ୍ଗ;
		}
		ସ୍ଥିତି SPECIAL_VAR_QUOTED_SVS:
			/* <SPECIAL_VAR_SYMBOL><SPECIAL_VAR_QUOTED_SVS><SPECIAL_VAR_SYMBOL> */
			/* "^C variable", represents literal ^C char (possible in scripts) */
			o_addchr(output, SPECIAL_VAR_ପ୍ରତୀକ);
			arg++;
			ଭାଙ୍ଗ;
#ଯଦି ENABLE_HUSH_TICK
		ସ୍ଥିତି '`': {
			/* <SPECIAL_VAR_SYMBOL>`cmd<SPECIAL_VAR_SYMBOL> */
			o_string subst_result = NULL_O_STRING;

			*p = '\0'; /* replace trailing <SPECIAL_VAR_SYMBOL> */
			arg++;
			/* Can't just stuff it into output o_string,
			 * expanded result may need to be globbed
			 * and $IFS-split */
			debug_printf_subst("SUBST '%s' first_ch %x\n", arg, first_ch);
			G.last_exitcode = process_command_subs(&subst_result, arg);
			G.expand_exitcode = G.last_exitcode;
			debug_printf_subst("SUBST RES:%d '%s'\n", G.last_exitcode, subst_result.data);
			n = append_str_maybe_ifs_split(output, n, first_ch, subst_result.data);
			o_free(&subst_result);
			ଭାଙ୍ଗ;
		}
#endif
#ଯଦି ENABLE_FEATURE_SH_MATH
		ସ୍ଥିତି '+': {
			/* <SPECIAL_VAR_SYMBOL>+arith<SPECIAL_VAR_SYMBOL> */
			arith_t res;

			arg++; /* skip '+' */
			*p = '\0'; /* replace trailing <SPECIAL_VAR_SYMBOL> */
			debug_printf_subst("ARITH '%s' first_ch %x\n", arg, first_ch);
			res = expand_and_evaluate_arith(arg, ଖାଲି);
			debug_printf_subst("ARITH RES '"ARITH_FMT"'\n", res);
			sprintf(arith_buf, ARITH_FMT, res);
			ଯଦି (res < 0
			 && first_ch == (ଅକ୍ଷର)('+'|0x80)
			/* && (output->o_expflags & EXP_FLAG_ESC_GLOB_CHARS) */
			) {
				/* Quoted negative ariths, like filename[0"$((-9))"],
				 * should not be interpreted as glob ranges.
				 * Convert leading '-' to '\-':
				 */
				o_grow_by(output, 1);
				output->data[output->length++] = '\\';
			}
			o_addstr(output, arith_buf);
			ଭାଙ୍ଗ;
		}
#endif
		ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
			/* <SPECIAL_VAR_SYMBOL>varname[ops]<SPECIAL_VAR_SYMBOL> */
			n = expand_one_var(output, n, first_ch, arg, &p);
			ଭାଙ୍ଗ;
		} /* switch (char after <SPECIAL_VAR_SYMBOL>) */

		/* Restore NULL'ed SPECIAL_VAR_SYMBOL.
		 * Do the check to avoid writing to a const string. */
		ଯଦି (*p != SPECIAL_VAR_ପ୍ରତୀକ)
			*p = SPECIAL_VAR_ପ୍ରତୀକ;
		arg = ++p;
	} /* end of "while (SPECIAL_VAR_SYMBOL is found) ..." */

	ଯଦି (*arg) {
		/* handle trailing string */
		ଯଦି (output->ended_in_ifs) {
			o_addchr(output, '\0');
			n = o_save_ptr(output, n);
		}
		debug_print_list("expand_vars_to_list[a]", output, n);
		/* this part is literal, and it was already pre-quoted
		 * if needed (much earlier), do not use o_addQstr here!
		 */
		o_addstr(output, arg);
		debug_print_list("expand_vars_to_list[b]", output, n);
	} ଅନ୍ୟଥା
	ଯଦି (output->length == o_get_last_ptr(output, n) /* expansion is empty */
	 && !(cant_be_null & 0x80)   /* and all vars were not quoted */
	 && !output->has_quoted_part
	) {
		n--;
		/* allow to reuse list[n] later without re-growth */
		output->has_empty_slot = 1;
	}

	ଫେରନ୍ତୁ n;
}

ସ୍ଥିର ଅକ୍ଷର **expand_variables(ଅକ୍ଷର **argv, unsigned expflags)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା n;
	ଅକ୍ଷର **list;
	o_string output = NULL_O_STRING;

	output.o_expflags = expflags;

	n = 0;
	ପାଇଁ (;;) {
		/* go to next list[n] */
		output.ended_in_ifs = 0;
		n = o_save_ptr(&output, n);

		ଯଦି (!*argv)
			ଭାଙ୍ଗ;

		/* expand argv[i] */
		n = expand_vars_to_list(&output, n, *argv++);
		/* if (!output->has_empty_slot) -- need this?? */
			o_addchr(&output, '\0');
	}
	debug_print_list("expand_variables", &output, n);

	/* output.data (malloced in one block) gets returned in "list" */
	list = o_finalize_list(&output, n);
	debug_print_strings("expand_variables[1]", list);
	ଫେରନ୍ତୁ list;
}

ସ୍ଥିର ଅକ୍ଷର **expand_strvec_to_strvec(ଅକ୍ଷର **argv)
{
	ଫେରନ୍ତୁ expand_variables(argv, EXP_FLAG_GLOB | EXP_FLAG_ESC_GLOB_CHARS);
}

#ଯଦି defined(CMD_SINGLEWORD_NOGLOB) || defined(CMD_TEST2_SINGLEWORD_NOGLOB)
ସ୍ଥିର ଅକ୍ଷର **expand_strvec_to_strvec_singleword_noglob(ଅକ୍ଷର **argv)
{
	ଫେରନ୍ତୁ expand_variables(argv, EXP_FLAG_SINGLEWORD);
}
#endif

/* Used for expansion of right hand of assignments,
 * $((...)), heredocs, variable expansion parts.
 *
 * NB: should NOT do globbing!
 * "export v=/bin/c*; env | grep ^v=" outputs "v=/bin/c*"
 */
ସ୍ଥିର ଅକ୍ଷର *expand_string_to_string(ଅଚଳ ଅକ୍ଷର *str, ପୂର୍ଣ୍ଣସଂଖ୍ୟା EXP_flags, ପୂର୍ଣ୍ଣସଂଖ୍ୟା do_unbackslash)
{
#ଯଦି !BASH_PATTERN_SUBST && !ENABLE_HUSH_CASE
	ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା do_unbackslash = 1;
	ଅଚଳ ପୂର୍ଣ୍ଣସଂଖ୍ୟା EXP_flags = EXP_FLAG_ESC_GLOB_CHARS;
#endif
	ଅକ୍ଷର *argv[2], **list;

	debug_printf_expand("string_to_string<='%s'\n", str);
	/* This is generally an optimization, but it also
	 * handles "", which otherwise trips over !list[0] check below.
	 * (is this ever happens that we actually get str="" here?)
	 */
	ଯଦି (!strchr(str, SPECIAL_VAR_ପ୍ରତୀକ) && !strchr(str, '\\')) {
		//TODO: Can use on strings with \ too, just unbackslash() them?
		debug_printf_expand("string_to_string(fast)=>'%s'\n", str);
		ଫେରନ୍ତୁ xstrdup(str);
	}

	argv[0] = (ଅକ୍ଷର*)str;
	argv[1] = ଖାଲି;
	list = expand_variables(argv, EXP_flags | EXP_FLAG_SINGLEWORD);
	ଯଦି (!list[0]) {
		/* Example where it happens:
		 * x=; echo ${x:-"$@"}
		 */
		((ଅକ୍ଷର*)list)[0] = '\0';
	} ଅନ୍ୟଥା {
		ଯଦି (HUSH_DEBUG)
			ଯଦି (list[1])
				bb_simple_error_ସନ୍ଦେଶ_and_die("BUG in varexp2");
		/* actually, just move string 2*sizeof(char*) bytes back */
		overlapping_strcpy((ଅକ୍ଷର*)list, list[0]);
		ଯଦି (do_unbackslash)
			unbackslash((ଅକ୍ଷର*)list);
	}
	debug_printf_expand("string_to_string=>'%s'\n", (ଅକ୍ଷର*)list);
	ଫେରନ୍ତୁ (ଅକ୍ଷର*)list;
}

#ଯଦି 0
ସ୍ଥିର ଅକ୍ଷର* expand_strvec_to_string(ଅକ୍ଷର **argv)
{
	ଅକ୍ଷର **list;

	list = expand_variables(argv, EXP_FLAG_SINGLEWORD);
	/* Convert all NULs to spaces */
	ଯଦି (list[0]) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = 1;
		ଯେତେବେଳେ (list[n]) {
			ଯଦି (HUSH_DEBUG)
				ଯଦି (list[n-1] + strlen(list[n-1]) + 1 != list[n])
					bb_error_ସନ୍ଦେଶ_and_die("BUG in varexp3");
			/* bash uses ' ' regardless of $IFS contents */
			list[n][-1] = ' ';
			n++;
		}
	}
	overlapping_strcpy((ଅକ୍ଷର*)list, list[0] ? list[0] : "");
	debug_printf_expand("strvec_to_string='%s'\n", (ଅକ୍ଷର*)list);
	ଫେରନ୍ତୁ (ଅକ୍ଷର*)list;
}
#endif

#ifndef __U_BOOT__
ସ୍ଥିର ଅକ୍ଷର **expand_assignments(ଅକ୍ଷର **argv, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	ଅକ୍ଷର **p;

	G.expanded_assignments = p = ଖାଲି;
	/* Expand assignments into one string each */
	ପାଇଁ (i = 0; i < ଗଣନା_2; i++) {
		p = add_string_to_strings(p,
			expand_string_to_string(argv[i],
				EXP_FLAG_ESC_GLOB_CHARS,
				/*unbackslash:*/ 1
			)
		);
		G.expanded_assignments = p;
	}
	G.expanded_assignments = ଖାଲି;
	ଫେରନ୍ତୁ p;
}

ସ୍ଥିର ଶୂନ୍ୟ switch_off_special_sigs(unsigned mask)
{
	unsigned sig = 0;
	ଯେତେବେଳେ ((mask >>= 1) != 0) {
		sig++;
		ଯଦି (!(mask & 1))
			ଜାରି;
#ଯଦି ENABLE_HUSH_TRAP
		ଯଦି (G_traps) {
			ଯଦି (G_traps[sig] && !G_traps[sig][0])
				/* trap is '', has to remain SIG_IGN */
				ଜାରି;
			ମୁକ୍ତ(G_traps[sig]);
			G_traps[sig] = ଖାଲି;
		}
#endif
		/* We are here only if no trap or trap was not '' */
		install_sighandler(sig, SIG_DFL);
	}
}
#endif /* !__U_BOOT__ */

#ifndef __U_BOOT__
#ଯଦି BB_MMU
/* never called */
ଶୂନ୍ୟ re_execute_shell(ଅକ୍ଷର ***to_free, ଅଚଳ ଅକ୍ଷର *s,
		ଅକ୍ଷର *g_argv0, ଅକ୍ଷର **g_argv,
		ଅକ୍ଷର **builtin_argv) NORETURN;

ସ୍ଥିର ଶୂନ୍ୟ reset_traps_to_defaults(ଶୂନ୍ୟ)
{
	/* This function is always called in a child shell
	 * after fork (not vfork, NOMMU doesn't use this function).
	 */
	IF_HUSH_TRAP(unsigned sig;)
	unsigned mask;

	/* Child shells are not interactive.
	 * SIGTTIN/SIGTTOU/SIGTSTP should not have special handling.
	 * Testcase: (while :; do :; done) + ^Z should background.
	 * Same goes for SIGTERM, SIGHUP, SIGINT.
	 */
	mask = (G.special_sig_mask & SPECIAL_INTERACTIVE_SIGS) | G_fatal_sig_mask;
	ଯଦି (!G_traps && !mask)
		ଫେରନ୍ତୁ; /* already no traps and no special sigs */

	/* Switch off special sigs */
	switch_off_special_sigs(mask);
# ଯଦି ENABLE_HUSH_JOB
	G_fatal_sig_mask = 0;
# endif
	G.special_sig_mask &= ~SPECIAL_INTERACTIVE_SIGS;
	/* SIGQUIT,SIGCHLD and maybe SPECIAL_JOBSTOP_SIGS
	 * remain set in G.special_sig_mask */

# ଯଦି ENABLE_HUSH_TRAP
	ଯଦି (!G_traps)
		ଫେରନ୍ତୁ;

	/* Reset all sigs to default except ones with empty traps */
	ପାଇଁ (sig = 0; sig < NSIG; sig++) {
		ଯଦି (!G_traps[sig])
			ଜାରି; /* no trap: nothing to do */
		ଯଦି (!G_traps[sig][0])
			ଜାରି; /* empty trap: has to remain SIG_IGN */
		/* sig has non-empty trap, reset it: */
		ମୁକ୍ତ(G_traps[sig]);
		G_traps[sig] = ଖାଲି;
		/* There is no signal for trap 0 (EXIT) */
		ଯଦି (sig == 0)
			ଜାରି;
		install_sighandler(sig, pick_sighandler(sig));
	}
# endif
}

#ଅନ୍ୟଥା /* !BB_MMU */

ସ୍ଥିର ଶୂନ୍ୟ re_execute_shell(ଅକ୍ଷର ***to_free, ଅଚଳ ଅକ୍ଷର *s,
		ଅକ୍ଷର *g_argv0, ଅକ୍ଷର **g_argv,
		ଅକ୍ଷର **builtin_argv) NORETURN;
ସ୍ଥିର ଶୂନ୍ୟ re_execute_shell(ଅକ୍ଷର ***to_free, ଅଚଳ ଅକ୍ଷର *s,
		ଅକ୍ଷର *g_argv0, ଅକ୍ଷର **g_argv,
		ଅକ୍ଷର **builtin_argv)
{
# define NOMMU_HACK_FMT ("-$%x:%x:%x:%x:%x:%llx" IF_HUSH_LOOPS(":%x"))
	/* delims + 2 * (number of bytes in printed hex numbers) */
	ଅକ୍ଷର param_buf[ଆକାର(NOMMU_HACK_FMT) + 2 * (ଆକାର(ପୂର୍ଣ୍ଣସଂଖ୍ୟା)*6 + ଆକାର(long long)*1)];
	ଅକ୍ଷର *heredoc_argv[4];
	ସଂରଚନା variable *cur;
# ଯଦି ENABLE_HUSH_FUNCTIONS
	ସଂରଚନା function *funcp;
# endif
	ଅକ୍ଷର **argv, **pp;
	unsigned cnt;
	unsigned long long empty_trap_mask;

	ଯଦି (!g_argv0) { /* heredoc */
		argv = heredoc_argv;
		argv[0] = (ଅକ୍ଷର *) G.argv0_for_re_execing;
		argv[1] = (ଅକ୍ଷର *) "-<";
		argv[2] = (ଅକ୍ଷର *) s;
		argv[3] = ଖାଲି;
		pp = &argv[3]; /* used as pointer to empty environment */
		ଯାଅ do_exec;
	}

	cnt = 0;
	pp = builtin_argv;
	ଯଦି (pp) ଯେତେବେଳେ (*pp++)
		cnt++;

	empty_trap_mask = 0;
	ଯଦି (G_traps) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା sig;
		ପାଇଁ (sig = 1; sig < NSIG; sig++) {
			ଯଦି (G_traps[sig] && !G_traps[sig][0])
				empty_trap_mask |= 1LL << sig;
		}
	}

	sprintf(param_buf, NOMMU_HACK_FMT
			, (unsigned) G.ମୂଳ_pid
			, (unsigned) G.ମୂଳ_ppid
			, (unsigned) G.last_bg_pid
			, (unsigned) G.last_exitcode
			, cnt
			, empty_trap_mask
			IF_HUSH_LOOPS(, G.depth_of_loop)
			);
# undef NOMMU_HACK_FMT
	/* 1:hush 2:-$<pid>:<pid>:<exitcode>:<etc...> <vars...> <funcs...>
	 * 3:-c 4:<cmd> 5:<arg0> <argN...> 6:NULL
	 */
	cnt += 6;
	ପାଇଁ (cur = G.ଶୀର୍ଷ_var; cur; cur = cur->next) {
		ଯଦି (!cur->flg_export || cur->flg_read_only)
			cnt += 2;
	}
# ଯଦି ENABLE_HUSH_LINENO_VAR
	cnt += 2;
# endif
# ଯଦି ENABLE_HUSH_FUNCTIONS
	ପାଇଁ (funcp = G.ଶୀର୍ଷ_func; funcp; funcp = funcp->next)
		cnt += 3;
# endif
	pp = g_argv;
	ଯେତେବେଳେ (*pp++)
		cnt++;
	*to_free = argv = pp = xzalloc(ଆକାର(argv[0]) * cnt);
	*pp++ = (ଅକ୍ଷର *) G.argv0_for_re_execing;
	*pp++ = param_buf;
	ପାଇଁ (cur = G.ଶୀର୍ଷ_var; cur; cur = cur->next) {
		ଯଦି (strcmp(cur->varstr, hush_version_str) == 0)
			ଜାରି;
		ଯଦି (cur->flg_read_only) {
			*pp++ = (ଅକ୍ଷର *) "-R";
			*pp++ = cur->varstr;
		} ଅନ୍ୟଥା ଯଦି (!cur->flg_export) {
			*pp++ = (ଅକ୍ଷର *) "-V";
			*pp++ = cur->varstr;
		}
	}
# ଯଦି ENABLE_HUSH_LINENO_VAR
	*pp++ = (ଅକ୍ଷର *) "-L";
	*pp++ = utoa(G.execute_lineno);
# endif
# ଯଦି ENABLE_HUSH_FUNCTIONS
	ପାଇଁ (funcp = G.ଶୀର୍ଷ_func; funcp; funcp = funcp->next) {
		*pp++ = (ଅକ୍ଷର *) "-F";
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
	*pp++ = (ଅକ୍ଷର *) "-c";
	*pp++ = (ଅକ୍ଷର *) s;
	ଯଦି (builtin_argv) {
		ଯେତେବେଳେ (*++builtin_argv)
			*pp++ = *builtin_argv;
		*pp++ = (ଅକ୍ଷର *) "";
	}
	*pp++ = g_argv0;
	ଯେତେବେଳେ (*g_argv)
		*pp++ = *g_argv++;
	/* *pp = NULL; - is already there */
	pp = environ;

 do_exec:
	debug_printf_exec("re_execute_shell pid:%d cmd:'%s'\n", getpid(), s);
	/* Don't propagate SIG_IGN to the child */
	ଯଦି (SPECIAL_JOBSTOP_SIGS != 0)
		switch_off_special_sigs(G.special_sig_mask & SPECIAL_JOBSTOP_SIGS);
	execve(bb_busybox_exec_path, argv, pp);
	/* Fallback. Useful for init=/bin/hush usage etc */
	ଯଦି (argv[0][0] == '/')
		execve(argv[0], argv, pp);
	xfunc_error_retval = 127;
	bb_simple_error_ସନ୍ଦେଶ_and_die("can't re-execute the shell");
}
#endif  /* !BB_MMU */

#endif /* !__U_BOOT__ */

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା run_and_free_list(ସଂରଚନା pipe *pi);

/* Executing from string: eval, sh -c '...'
 *          or from file: /etc/profile, . file, sh <script>, sh (intereactive)
 * end_trigger controls how often we stop parsing
 * NUL: parse all, execute, return
 * ';': parse till ';' or newline, execute, repeat till EOF
 */
#ifndef __U_BOOT__
ସ୍ଥିର ଶୂନ୍ୟ parse_and_run_stream(ସଂରଚନା in_str *inp, ପୂର୍ଣ୍ଣସଂଖ୍ୟା end_trigger)
#ଅନ୍ୟଥା /* __U_BOOT__ */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା parse_and_run_stream(ସଂରଚନା in_str *inp, ପୂର୍ଣ୍ଣସଂଖ୍ୟା end_trigger)
#endif /* __U_BOOT__ */
{
	/* Why we need empty flag?
	 * An obscure corner case "false; ``; echo $?":
	 * empty command in `` should still set $? to 0.
	 * But we can't just set $? to 0 at the start,
	 * this breaks "false; echo `echo $?`" case.
	 */
	bool empty = 1;
#ifndef __U_BOOT__
	ଯେତେବେଳେ (1) {
#ଅନ୍ୟଥା /* __U_BOOT__ */
	କର {
#endif /* __U_BOOT__ */
		ସଂରଚନା pipe *pipe_list;

#ଯଦି ENABLE_HUSH_INTERACTIVE
		ଯଦି (end_trigger == ';') {
			G.promptmode = 0; /* PS1 */
			debug_printf_prompt("%s promptmode=%d\n", __func__, G.promptmode);
		}
#endif
		pipe_list = parse_stream(ଖାଲି, ଖାଲି, inp, end_trigger);
		ଯଦି (!pipe_list || pipe_list == ERR_PTR) { /* EOF/error */
			/* If we are in "big" script
			 * (not in `cmd` or something similar)...
			 */
			ଯଦି (pipe_list == ERR_PTR && end_trigger == ';') {
				/* Discard cached input (rest of line) */
				ପୂର୍ଣ୍ଣସଂଖ୍ୟା ch = inp->last_char;
				ଯେତେବେଳେ (ch != EOF && ch != '\n') {
					//bb_error_msg("Discarded:'%c'", ch);
					ch = i_getch(inp);
				}
				/* Force prompt */
				inp->p = ଖାଲି;
				/* This stream isn't empty */
				empty = 0;
				ଜାରି;
			}
			ଯଦି (!pipe_list && empty)
				G.last_exitcode = 0;
			ଭାଙ୍ଗ;
		}
		debug_print_ବୃକ୍ଷ(pipe_list, 0);
		debug_printf_exec("parse_and_run_stream: run_and_free_list\n");
#ifndef __U_BOOT__
		run_and_free_list(pipe_list);
#ଅନ୍ୟଥା /* __U_BOOT__ */
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା rcode = run_and_free_list(pipe_list);
		/*
		 * We reset input string to not run the following command, so running
		 * 'exit; echo foo' does not print foo.
		 */
		ଯଦି (rcode <= EXIT_RET_CODE)
			setup_file_in_str(inp);
#endif /* __U_BOOT__ */
		empty = 0;
		ଯଦି (G_flag_return_in_progress == 1)
			ଭାଙ୍ଗ;
#ifndef __U_BOOT__
	}
#ଅନ୍ୟଥା /* __U_BOOT__ */
	/*
	 * This do/while is needed by run_command to avoid looping on a command
	 * with syntax error.
	 */
	} ଯେତେବେଳେ (!(G.run_command_flags & FLAG_EXIT_FROM_LOOP));

	ଫେରନ୍ତୁ G.last_exitcode;
#endif /* __U_BOOT__ */
}

#ifndef __U_BOOT__
ସ୍ଥିର ଶୂନ୍ୟ parse_and_run_string(ଅଚଳ ଅକ୍ଷର *s)
#ଅନ୍ୟଥା /* __U_BOOT__ */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା parse_and_run_string(ଅଚଳ ଅକ୍ଷର *s)
#endif /* __U_BOOT__ */
{
	ସଂରଚନା in_str ନିବେଶ;
#ifndef __U_BOOT__
	IF_HUSH_LINENO_VAR(unsigned sv = G.parse_lineno;)
#ଅନ୍ୟଥା /* __U_BOOT__ */
	//IF_HUSH_LINENO_VAR(unsigned sv = G.parse_lineno;)
#endif /* __U_BOOT__ */

	setup_string_in_str(&ନିବେଶ, s);
#ifndef __U_BOOT__
	parse_and_run_stream(&ନିବେଶ, '\0');
#ଅନ୍ୟଥା /* __U_BOOT__ */
	ଫେରନ୍ତୁ parse_and_run_stream(&ନିବେଶ, '\0');
#endif /* __U_BOOT__ */
#ifndef __U_BOOT__
	IF_HUSH_LINENO_VAR(unsigned sv = G.parse_lineno;)
#ଅନ୍ୟଥା /* __U_BOOT__ */
	//IF_HUSH_LINENO_VAR(unsigned sv = G.parse_lineno;)
#endif /* __U_BOOT__ */
}

#ifdef __U_BOOT__
ପୂର୍ଣ୍ଣସଂଖ୍ୟା parse_string_outer_modern(ଅଚଳ ଅକ୍ଷର *cmd, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା old_flags;

	/*
	 * Keep old values of run_command to be able to restore them once
	 * command was executed.
	 */
	old_flags = G.run_command_flags;
	G.run_command_flags = flags;

	ret = parse_and_run_string(cmd);

	G.run_command_flags = old_flags;

	ଫେରନ୍ତୁ ret;
}
#endif /* __U_BOOT__ */
#ifndef __U_BOOT__
ସ୍ଥିର ଶୂନ୍ୟ parse_and_run_file(HFILE *fp)
#ଅନ୍ୟଥା /* __U_BOOT__ */
ଶୂନ୍ୟ parse_and_run_file(ଶୂନ୍ୟ)
#endif /* __U_BOOT__ */
{
	ସଂରଚନା in_str ନିବେଶ;
#ifndef __U_BOOT__
	IF_HUSH_LINENO_VAR(unsigned sv = G.parse_lineno;)

	IF_HUSH_LINENO_VAR(G.parse_lineno = 1;)
	setup_file_in_str(&ନିବେଶ, fp);
#ଅନ୍ୟଥା /* __U_BOOT__ */
	setup_file_in_str(&ନିବେଶ);
#endif /* __U_BOOT__ */
	parse_and_run_stream(&ନିବେଶ, ';');
#ifndef __U_BOOT__
	IF_HUSH_LINENO_VAR(G.parse_lineno = sv;)
#endif /* !__U_BOOT__ */
}

#ifndef __U_BOOT__
#ଯଦି ENABLE_HUSH_TICK
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା generate_stream_from_string(ଅଚଳ ଅକ୍ଷର *s, pid_t *pid_p)
{
	pid_t pid;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା channel[2];
# ଯଦି !BB_MMU
	ଅକ୍ଷର **to_free = ଖାଲି;
# endif

	xpipe(channel);
	pid = BB_MMU ? xfork() : xvfork();
	ଯଦି (pid == 0) { /* child */
		disable_restore_tty_pgrp_on_exit();
		/* Process substitution is not considered to be usual
		 * 'command execution'.
		 * SUSv3 says ctrl-Z should be ignored, ctrl-C should not.
		 */
		bb_signals(0
			+ (1 << SIGTSTP)
			+ (1 << SIGTTIN)
			+ (1 << SIGTTOU)
			, SIG_IGN);
		close(channel[0]); /* NB: close _first_, then move fd! */
		xmove_fd(channel[1], 1);
# ଯଦି ENABLE_HUSH_TRAP
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
		ଯଦି (is_prefixed_with(s, "trap")
		 && skip_whitespace(s + 4)[0] == '\0'
		) {
			ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ argv[] ALIGN_PTR = { ଖାଲି, ଖାଲି };
			builtin_trap((ଅକ୍ଷର**)argv);
			fflush_all(); /* important */
			_exit(0);
		}
# endif
# ଯଦି BB_MMU
		/* Prevent it from trying to handle ctrl-z etc */
		IF_HUSH_JOB(G.run_list_level = 1;)
		CLEAR_RANDOM_T(&G.random_gen); /* or else $RANDOM repeats in child */
		reset_traps_to_defaults();
		IF_HUSH_MODE_X(G.x_mode_depth++;)
		//bb_error_msg("%s: ++x_mode_depth=%d", __func__, G.x_mode_depth);
		parse_and_run_string(s);
		_exit(G.last_exitcode);
# ଅନ୍ୟଥା
	/* We re-execute after vfork on NOMMU. This makes this script safe:
	 * yes "0123456789012345678901234567890" | dd bs=32 count=64k >BIG
	 * huge=`cat BIG` # was blocking here forever
	 * echo OK
	 */
		re_execute_shell(&to_free,
				s,
				G.global_argv[0],
				G.global_argv + 1,
				ଖାଲି);
# endif
	}

	/* parent */
	*pid_p = pid;
# ଯଦି ENABLE_HUSH_FAST
	G.ଗଣନା_SIGCHLD++;
//bb_error_msg("[%d] fork in generate_stream_from_string:"
//		" G.count_SIGCHLD:%d G.handled_SIGCHLD:%d",
//		getpid(), G.count_SIGCHLD, G.handled_SIGCHLD);
# endif
	enable_restore_tty_pgrp_on_exit();
# ଯଦି !BB_MMU
	ମୁକ୍ତ(to_free);
# endif
	close(channel[1]);
	ଫେରନ୍ତୁ channel[0];
}

/* Return code is exit status of the process that is run. */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା process_command_subs(o_string *dest, ଅଚଳ ଅକ୍ଷର *s)
{
	FILE *fp;
	pid_t pid;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା status, ch, eol_cnt;

	fp = xfdopen_for_read(generate_stream_from_string(s, &pid));

	/* Now send results of command back into original context */
	eol_cnt = 0;
	ଯେତେବେଳେ ((ch = getc(fp)) != EOF) {
		ଯଦି (ch == '\0')
			ଜାରି;
		ଯଦି (ch == '\n') {
			eol_cnt++;
			ଜାରି;
		}
		ଯେତେବେଳେ (eol_cnt) {
			o_addchr(dest, '\n');
			eol_cnt--;
		}
		o_addQchr(dest, ch);
	}

	debug_printf("done reading from `cmd` pipe, closing it\n");
	fclose(fp);
	/* We need to extract exitcode. Test case
	 * "true; echo `sleep 1; false` $?"
	 * should print 1 */
	safe_waitpid(pid, &status, 0);
	debug_printf("child exited. returning its exitcode:%d\n", WEXITSTATUS(status));
	ଫେରନ୍ତୁ WEXITSTATUS(status);
}
#endif /* ENABLE_HUSH_TICK */

ସ୍ଥିର ଶୂନ୍ୟ setup_heredoc(ସଂରଚନା redir_struct *redir)
{
	ସଂରଚନା fd_pair pair;
	pid_t pid;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା len, written;
	/* the _body_ of heredoc (misleading field name) */
	ଅଚଳ ଅକ୍ଷର *heredoc = redir->rd_filename;
	ଅକ୍ଷର *expanded;
#ଯଦି !BB_MMU
	ଅକ୍ଷର **to_free;
#endif

	expanded = ଖାଲି;
	ଯଦି (!(redir->rd_dup & HEREDOC_QUOTED)) {
		expanded = encode_then_expand_string(heredoc);
		ଯଦି (expanded)
			heredoc = expanded;
	}
	len = strlen(heredoc);

	close(redir->rd_fd); /* often saves dup2+close in xmove_fd */
	xpiped_pair(pair);
	xmove_fd(pair.rd, redir->rd_fd);

	/* Try writing without forking. Newer kernels have
	 * dynamically growing pipes. Must use non-blocking write! */
	ndelay_on(pair.wr);
	ଯେତେବେଳେ (1) {
		written = ଲିଖ(pair.wr, heredoc, len);
		ଯଦି (written <= 0)
			ଭାଙ୍ଗ;
		len -= written;
		ଯଦି (len == 0) {
			close(pair.wr);
			ମୁକ୍ତ(expanded);
			ଫେରନ୍ତୁ;
		}
		heredoc += written;
	}
	ndelay_off(pair.wr);

	/* Okay, pipe buffer was not big enough */
	/* Note: we must not create a stray child (bastard? :)
	 * for the unsuspecting parent process. Child creates a grandchild
	 * and exits before parent execs the process which consumes heredoc
	 * (that exec happens after we return from this function) */
#ଯଦି !BB_MMU
	to_free = ଖାଲି;
#endif
	pid = xvfork();
	ଯଦି (pid == 0) {
		/* child */
		disable_restore_tty_pgrp_on_exit();
		pid = BB_MMU ? xfork() : xvfork();
		ଯଦି (pid != 0)
			_exit(0);
		/* grandchild */
		close(redir->rd_fd); /* read side of the pipe */
#ଯଦି BB_MMU
		full_write(pair.wr, heredoc, len); /* may loop or block */
		_exit(0);
#ଅନ୍ୟଥା
		/* Delegate blocking writes to another process */
		xmove_fd(pair.wr, STDOUT_FILENO);
		re_execute_shell(&to_free, heredoc, ଖାଲି, ଖାଲି, ଖାଲି);
#endif
	}
	/* parent */
#ଯଦି ENABLE_HUSH_FAST
	G.ଗଣନା_SIGCHLD++;
//bb_error_msg("[%d] fork in setup_heredoc: G.count_SIGCHLD:%d G.handled_SIGCHLD:%d", getpid(), G.count_SIGCHLD, G.handled_SIGCHLD);
#endif
	enable_restore_tty_pgrp_on_exit();
#ଯଦି !BB_MMU
	ମୁକ୍ତ(to_free);
#endif
	close(pair.wr);
	ମୁକ୍ତ(expanded);
	wait(ଖାଲି); /* wait till child has died */
}

ସଂରଚନା squirrel {
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା orig_fd;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା moved_to;
	/* moved_to = n: fd was moved to n; restore back to orig_fd after redir */
	/* moved_to = -1: fd was opened by redirect; close orig_fd after redir */
};

ସ୍ଥିର ସଂରଚନା squirrel *append_squirrel(ସଂରଚନା squirrel *sq, ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, ପୂର୍ଣ୍ଣସଂଖ୍ୟା orig, ପୂର୍ଣ୍ଣସଂଖ୍ୟା moved)
{
	sq = xrealloc(sq, (i + 2) * ଆକାର(sq[0]));
	sq[i].orig_fd = orig;
	sq[i].moved_to = moved;
	sq[i+1].orig_fd = -1; /* end marker */
	ଫେରନ୍ତୁ sq;
}

ସ୍ଥିର ସଂରଚନା squirrel *add_squirrel(ସଂରଚନା squirrel *sq, ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd, ପୂର୍ଣ୍ଣସଂଖ୍ୟା avoid_fd)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା moved_to;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

	i = 0;
	ଯଦି (sq) ପାଇଁ (; sq[i].orig_fd >= 0; i++) {
		/* If we collide with an already moved fd... */
		ଯଦି (fd == sq[i].moved_to) {
			moved_to = dup_CLOEXEC(sq[i].moved_to, avoid_fd);
			debug_printf_redir("redirect_fd %d: already busy, moving to %d\n", fd, moved_to);
			ଯଦି (moved_to < 0) {
				/* "echo 2>/dev/tty 10>&9999" testcase:
				 * We move fd 2 to 10, then discover we need to move fd 10
				 * (and not hit 9999) and the latter fails.
				 */
				ଫେରନ୍ତୁ ଖାଲି; /* fcntl failed */
			}
			sq[i].moved_to = moved_to;
			ଫେରନ୍ତୁ sq;
		}
		ଯଦି (fd == sq[i].orig_fd) {
			/* Example: echo Hello >/dev/null 1>&2 */
			debug_printf_redir("redirect_fd %d: already moved\n", fd);
			ଫେରନ୍ତୁ sq;
		}
	}

	/* If this fd is open, we move and remember it; if it's closed, moved_to = -1 */
	moved_to = dup_CLOEXEC(fd, avoid_fd);
	debug_printf_redir("redirect_fd %d: previous fd is moved to %d (-1 if it was closed)\n", fd, moved_to);
	ଯଦି (moved_to < 0 && errno != EBADF)
		ଫେରନ୍ତୁ ଖାଲି; /* fcntl failed (not because fd is closed) */
	ଫେରନ୍ତୁ append_squirrel(sq, i, fd, moved_to);
}

ସ୍ଥିର ସଂରଚନା squirrel *add_squirrel_closed(ସଂରଚନା squirrel *sq, ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

	i = 0;
	ଯଦି (sq) ପାଇଁ (; sq[i].orig_fd >= 0; i++) {
		/* If we collide with an already moved fd... */
		ଯଦି (fd == sq[i].orig_fd) {
			/* Examples:
			 * "echo 3>FILE 3>&- 3>FILE"
			 * "echo 3>&- 3>FILE"
			 * No need for last redirect to insert
			 * another "need to close 3" indicator.
			 */
			debug_printf_redir("redirect_fd %d: already moved or closed\n", fd);
			ଫେରନ୍ତୁ sq;
		}
	}

	debug_printf_redir("redirect_fd %d: previous fd was closed\n", fd);
	ଫେରନ୍ତୁ append_squirrel(sq, i, fd, -1);
}

/* fd: redirect wants this fd to be used (e.g. 3>file).
 * Move all conflicting internally used fds,
 * and remember them so that we can restore them later.
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା save_fd_on_redirect(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd, ପୂର୍ଣ୍ଣସଂଖ୍ୟା avoid_fd, ସଂରଚନା squirrel **sqp)
{
	ସଂରଚନା squirrel *new_squirrel;

	ଯଦି (avoid_fd < 9) /* the important case here is that it can be -1 */
		avoid_fd = 9;

#ଯଦି ENABLE_HUSH_INTERACTIVE
	ଯଦି (fd != 0 /* don't trigger for G_interactive_fd == 0 (that's "not interactive" flag) */
	 && fd == G_interactive_fd
	) {
		/* Testcase: "ls -l /proc/$$/fd 255>&-" should work */
		G_interactive_fd = xdup_CLOEXEC_and_close(G_interactive_fd, avoid_fd);
		debug_printf_redir("redirect_fd %d: matches interactive_fd, moving it to %d\n", fd, G_interactive_fd);
		ଫେରନ୍ତୁ 1; /* "we closed fd" */
	}
#endif
	/* Are we called from setup_redirects(squirrel==NULL)
	 * in redirect in a [v]forked child?
	 */
	ଯଦି (sqp == ଖାଲି) {
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
		ଫେରନ୍ତୁ 0;
	}

	/* If this one of script's fds? */
	ଯଦି (move_HFILEs_on_redirect(fd, avoid_fd))
		ଫେରନ୍ତୁ 1; /* yes. "we closed fd" (actually moved it) */

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
	ଯଦି (sqp == ERR_PTR) {
		/* Don't preserve redirected fds: exec is _meant_ to change these */
		//bb_error_msg("sqp == ERR_PTR: exec >FILE");
		ଫେରନ୍ତୁ 0;
	}

	/* Check whether it collides with any open fds (e.g. stdio), save fds as needed */
	new_squirrel = add_squirrel(*sqp, fd, avoid_fd);
	ଯଦି (!new_squirrel)
		ଫେରନ୍ତୁ -1; /* redirect error */
	*sqp = new_squirrel;
	ଫେରନ୍ତୁ 0; /* "we did not close fd" */
}

ସ୍ଥିର ଶୂନ୍ୟ restore_redirects(ସଂରଚନା squirrel *sq)
{
	ଯଦି (sq) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
		ପାଇଁ (i = 0; sq[i].orig_fd >= 0; i++) {
			ଯଦି (sq[i].moved_to >= 0) {
				/* We simply die on error */
				debug_printf_redir("restoring redirected fd from %d to %d\n", sq[i].moved_to, sq[i].orig_fd);
				xmove_fd(sq[i].moved_to, sq[i].orig_fd);
			} ଅନ୍ୟଥା {
				/* cmd1 9>FILE; cmd2_should_see_fd9_closed */
				debug_printf_redir("restoring redirected fd %d: closing it\n", sq[i].orig_fd);
				close(sq[i].orig_fd);
			}
		}
		ମୁକ୍ତ(sq);
	}
	ଯଦି (G.HFILE_stdin
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

#ଯଦି ENABLE_FEATURE_SH_STANDALONE && BB_MMU
ସ୍ଥିର ଶୂନ୍ୟ close_saved_fds_and_FILE_fds(ଶୂନ୍ୟ)
{
	ଯଦି (G_interactive_fd)
		close(G_interactive_fd);
	close_all_HFILE_list();
}
#endif

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା internally_opened_fd(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd, ସଂରଚନା squirrel *sq)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

#ଯଦି ENABLE_HUSH_INTERACTIVE
	ଯଦି (fd == G_interactive_fd)
		ଫେରନ୍ତୁ 1;
#endif
	/* If this one of script's fds? */
	ଯଦି (fd_in_HFILEs(fd))
		ଫେରନ୍ତୁ 1;

	ଯଦି (sq) ପାଇଁ (i = 0; sq[i].orig_fd >= 0; i++) {
		ଯଦି (fd == sq[i].moved_to)
			ଫେରନ୍ତୁ 1;
	}
	ଫେରନ୍ତୁ 0;
}

/* sqp != NULL means we squirrel away copies of stdin, stdout,
 * and stderr if they are redirected.
 * If redirection fails, return 1. This will make caller
 * skip command execution and restore already created redirect fds.
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା setup_redirects(ସଂରଚନା command *prog, ସଂରଚନା squirrel **sqp)
{
	ସଂରଚନା redir_struct *redir;

	ପାଇଁ (redir = prog->redirects; redir; redir = redir->next) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା newfd;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା closed;

		ଯଦି (redir->rd_type == REDIRECT_HEREDOC2) {
			/* "rd_fd<<HERE" case */
			ଯଦି (save_fd_on_redirect(redir->rd_fd, /*avoid:*/ 0, sqp) < 0)
				ଫେରନ୍ତୁ 1;
			/* for REDIRECT_HEREDOC2, rd_filename holds _contents_
			 * of the heredoc */
			debug_printf_redir("set heredoc '%s'\n",
					redir->rd_filename);
			setup_heredoc(redir);
			ଜାରି;
		}

		ଯଦି (redir->rd_dup == REDIRFD_TO_FILE) {
			/* "rd_fd<*>file" case (<*> is <,>,>>,<>) */
			ଅକ୍ଷର *p;
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା mode;

			ଯଦି (redir->rd_filename == ଖାଲି) {
				/* Examples:
				 * "cmd >" (no filename)
				 * "cmd > <file" (2nd redirect starts too early)
				 */
				syntax_error("invalid redirect");
				ଫେରନ୍ତୁ 1;
			}
			mode = redir_table[redir->rd_type].mode;
			p = expand_string_to_string(redir->rd_filename,
				EXP_FLAG_ESC_GLOB_CHARS, /*unbackslash:*/ 1);
			newfd = open_or_warn(p, mode);
			ମୁକ୍ତ(p);
			ଯଦି (newfd < 0) {
				/* Error message from open_or_warn can be lost
				 * if stderr has been redirected, but bash
				 * and ash both lose it as well
				 * (though zsh doesn't!)
				 */
				ଫେରନ୍ତୁ 1;
			}
			ଯଦି (newfd == redir->rd_fd && sqp
			 && sqp != ERR_PTR /* not a redirect in "exec" */
			) {
				/* open() gave us precisely the fd we wanted.
				 * This means that this fd was not busy
				 * (not opened to anywhere).
				 * Remember to close it on restore:
				 */
				*sqp = add_squirrel_closed(*sqp, newfd);
				debug_printf_redir("redir to previously closed fd %d\n", newfd);
			}
		} ଅନ୍ୟଥା {
			/* "rd_fd>&rd_dup" or "rd_fd>&-" case */
			newfd = redir->rd_dup;
		}

		ଯଦି (newfd == redir->rd_fd)
			ଜାରି;

		/* if "N>FILE": move newfd to redir->rd_fd */
		/* if "N>&M": dup newfd to redir->rd_fd */
		/* if "N>&-": close redir->rd_fd (newfd is REDIRFD_CLOSE) */

		closed = save_fd_on_redirect(redir->rd_fd, /*avoid:*/ newfd, sqp);
		ଯଦି (closed < 0)
			ଫେରନ୍ତୁ 1; /* error */
		ଯଦି (newfd == REDIRFD_CLOSE) {
			/* "N>&-" means "close me" */
			ଯଦି (!closed) {
				/* ^^^ optimization: saving may already
				 * have closed it. If not... */
				close(redir->rd_fd);
			}
			/* Sometimes we do another close on restore, getting EBADF.
			 * Consider "echo 3>FILE 3>&-"
			 * first redirect remembers "need to close 3",
			 * and second redirect closes 3! Restore code then closes 3 again.
			 */
		} ଅନ୍ୟଥା {
			/* if newfd is a script fd or saved fd, do not allow to use it */
			ଯଦି (internally_opened_fd(newfd, sqp && sqp != ERR_PTR ? *sqp : ଖାଲି)) {
				bb_error_ସନ୍ଦେଶ("fd#%d is not open", newfd);
				ଫେରନ୍ତୁ 1;
			}
			ଯଦି (dup2(newfd, redir->rd_fd) < 0) {
				/* "echo >&99" testcase */
				bb_perror_ସନ୍ଦେଶ("dup2(%d,%d)", newfd, redir->rd_fd);
				ଫେରନ୍ତୁ 1;
			}
			ଯଦି (redir->rd_dup == REDIRFD_TO_FILE)
				/* "rd_fd > FILE" */
				close(newfd);
			/* else: "rd_fd > rd_dup" */
		}
	}
	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ଅକ୍ଷର *find_in_path(ଅଚଳ ଅକ୍ଷର *arg)
{
	ଅକ୍ଷର *ret = ଖାଲି;
	ଅଚଳ ଅକ୍ଷର *PATH = get_local_var_ମୂଲ୍ୟ("PATH");

	ଯଦି (!PATH)
		ଫେରନ୍ତୁ ଖାଲି;

	ଯେତେବେଳେ (1) {
		ଅଚଳ ଅକ୍ଷର *end = strchrnul(PATH, ':');
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା sz = end - PATH; /* must be int! */

		ମୁକ୍ତ(ret);
		ଯଦି (sz != 0) {
			ret = xasprintf("%.*s/%s", sz, PATH, arg);
		} ଅନ୍ୟଥା {
			/* We have xxx::yyyy in $PATH,
			 * it means "use current dir" */
			ret = xstrdup(arg);
		}
		ଯଦି (access(ret, F_OK) == 0)
			ଭାଙ୍ଗ;

		ଯଦି (*end == '\0') {
			ମୁକ୍ତ(ret);
			ଫେରନ୍ତୁ ଖାଲି;
		}
		PATH = end + 1;
	}

	ଫେରନ୍ତୁ ret;
}

ସ୍ଥିର ଅଚଳ ସଂରଚନା built_in_command *find_builtin_helper(ଅଚଳ ଅକ୍ଷର *name,
		ଅଚଳ ସଂରଚନା built_in_command *x,
		ଅଚଳ ସଂରଚନା built_in_command *end)
{
	ଯେତେବେଳେ (x != end) {
		ଯଦି (strcmp(name, x->b_cmd) != 0) {
			x++;
			ଜାରି;
		}
		debug_printf_exec("found builtin '%s'\n", name);
		ଫେରନ୍ତୁ x;
	}
	ଫେରନ୍ତୁ ଖାଲି;
}
ସ୍ଥିର ଅଚଳ ସଂରଚନା built_in_command *find_builtin1(ଅଚଳ ଅକ୍ଷର *name)
{
	ଫେରନ୍ତୁ find_builtin_helper(name, bltins1, &bltins1[ARRAY_SIZE(bltins1)]);
}
ସ୍ଥିର ଅଚଳ ସଂରଚନା built_in_command *find_builtin(ଅଚଳ ଅକ୍ଷର *name)
{
	ଅଚଳ ସଂରଚନା built_in_command *x = find_builtin1(name);
	ଯଦି (x)
		ଫେରନ୍ତୁ x;
	ଫେରନ୍ତୁ find_builtin_helper(name, bltins2, &bltins2[ARRAY_SIZE(bltins2)]);
}

#ଯଦି ENABLE_HUSH_JOB && ENABLE_FEATURE_TAB_COMPLETION
ସ୍ଥିର ଅଚଳ ଅକ୍ଷର * FAST_FUNC hush_command_name(ପୂର୍ଣ୍ଣସଂଖ୍ୟା i)
{
	ଯଦି (/*i >= 0 && */ i < ARRAY_SIZE(bltins1)) {
		ଫେରନ୍ତୁ bltins1[i].b_cmd;
	}
	i -= ARRAY_SIZE(bltins1);
	ଯଦି (i < ARRAY_SIZE(bltins2)) {
		ଫେରନ୍ତୁ bltins2[i].b_cmd;
	}
# ଯଦି ENABLE_HUSH_FUNCTIONS
	{
		ସଂରଚନା function *funcp;
		i -= ARRAY_SIZE(bltins2);
		ପାଇଁ (funcp = G.ଶୀର୍ଷ_func; funcp; funcp = funcp->next) {
			ଯଦି (--i < 0)
				ଫେରନ୍ତୁ funcp->name;
		}
	}
# endif
	ଫେରନ୍ତୁ ଖାଲି;
}
#endif
#endif /* !__U_BOOT__ */

#ifndef __U_BOOT__
ସ୍ଥିର ଶୂନ୍ୟ remove_nested_vars(ଶୂନ୍ୟ)
{
	ସଂରଚନା variable *cur;
	ସଂରଚନା variable **cur_pp;

	cur_pp = &G.ଶୀର୍ଷ_var;
	ଯେତେବେଳେ ((cur = *cur_pp) != ଖାଲି) {
		ଯଦି (cur->var_nest_level <= G.var_nest_level) {
			cur_pp = &cur->next;
			ଜାରି;
		}
		/* Unexport */
		ଯଦି (cur->flg_export) {
			debug_printf_env("unexporting nested '%s'/%u\n", cur->varstr, cur->var_nest_level);
			bb_unsetenv(cur->varstr);
		}
		/* Remove from global list */
		*cur_pp = cur->next;
		/* Free */
		ଯଦି (!cur->max_len) {
			debug_printf_env("freeing nested '%s'/%u\n", cur->varstr, cur->var_nest_level);
			ମୁକ୍ତ(cur->varstr);
		}
		ମୁକ୍ତ(cur);
	}
}

ସ୍ଥିର ଶୂନ୍ୟ enter_var_nest_level(ଶୂନ୍ୟ)
{
	G.var_nest_level++;
	debug_printf_env("var_nest_level++ %u\n", G.var_nest_level);

	/* Try:	f() { echo -n .; f; }; f
	 * struct variable::var_nest_level is uint16_t,
	 * thus limiting recursion to < 2^16.
	 * In any case, with 8 Mbyte stack SEGV happens
	 * not too long after 2^16 recursions anyway.
	 */
	ଯଦି (G.var_nest_level > 0xff00)
		bb_error_ସନ୍ଦେଶ_and_die("fatal recursion (depth %u)", G.var_nest_level);
}

ସ୍ଥିର ଶୂନ୍ୟ leave_var_nest_level(ଶୂନ୍ୟ)
{
	G.var_nest_level--;
	debug_printf_env("var_nest_level-- %u\n", G.var_nest_level);
	ଯଦି (HUSH_DEBUG && (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)G.var_nest_level < 0)
		bb_simple_error_ସନ୍ଦେଶ_and_die("BUG: nesting underflow");

	remove_nested_vars();
}
#endif /* __U_BOOT__ */

#ଯଦି ENABLE_HUSH_FUNCTIONS
ସ୍ଥିର ସଂରଚନା function **find_function_slot(ଅଚଳ ଅକ୍ଷର *name)
{
	ସଂରଚନା function *funcp;
	ସଂରଚନା function **funcpp = &G.ଶୀର୍ଷ_func;

	ଯେତେବେଳେ ((funcp = *funcpp) != ଖାଲି) {
		ଯଦି (strcmp(name, funcp->name) == 0) {
			debug_printf_exec("found function '%s'\n", name);
			ଭାଙ୍ଗ;
		}
		funcpp = &funcp->next;
	}
	ଫେରନ୍ତୁ funcpp;
}

ସ୍ଥିର ALWAYS_INLINE ଅଚଳ ସଂରଚନା function *find_function(ଅଚଳ ଅକ୍ଷର *name)
{
	ଅଚଳ ସଂରଚନା function *funcp = *find_function_slot(name);
	ଫେରନ୍ତୁ funcp;
}

/* Note: takes ownership on name ptr */
ସ୍ଥିର ସଂରଚନା function *new_function(ଅକ୍ଷର *name)
{
	ସଂରଚନା function **funcpp = find_function_slot(name);
	ସଂରଚନା function *funcp = *funcpp;

	ଯଦି (funcp != ଖାଲି) {
		ସଂରଚନା command *cmd = funcp->parent_cmd;
		debug_printf_exec("func %p parent_cmd %p\n", funcp, cmd);
		ଯଦି (!cmd) {
			debug_printf_exec("freeing & replacing function '%s'\n", funcp->name);
			ମୁକ୍ତ(funcp->name);
			/* Note: if !funcp->body, do not free body_as_string!
			 * This is a special case of "-F name body" function:
			 * body_as_string was not malloced! */
			ଯଦି (funcp->body) {
				free_pipe_list(funcp->body);
# ଯଦି !BB_MMU
				ମୁକ୍ତ(funcp->body_as_string);
# endif
			}
		} ଅନ୍ୟଥା {
			debug_printf_exec("reinserting in tree & replacing function '%s'\n", funcp->name);
			cmd->argv[0] = funcp->name;
			cmd->group = funcp->body;
# ଯଦି !BB_MMU
			cmd->group_as_string = funcp->body_as_string;
# endif
		}
	} ଅନ୍ୟଥା {
		debug_printf_exec("remembering new function '%s'\n", name);
		funcp = *funcpp = xzalloc(ଆକାର(*funcp));
		/*funcp->next = NULL;*/
	}

	funcp->name = name;
	ଫେରନ୍ତୁ funcp;
}

# ଯଦି ENABLE_HUSH_UNSET
ସ୍ଥିର ଶୂନ୍ୟ unset_func(ଅଚଳ ଅକ୍ଷର *name)
{
	ସଂରଚନା function **funcpp = find_function_slot(name);
	ସଂରଚନା function *funcp = *funcpp;

	ଯଦି (funcp != ଖାଲି) {
		debug_printf_exec("freeing function '%s'\n", funcp->name);
		*funcpp = funcp->next;
		/* funcp is unlinked now, deleting it.
		 * Note: if !funcp->body, the function was created by
		 * "-F name body", do not free ->body_as_string
		 * and ->name as they were not malloced. */
		ଯଦି (funcp->body) {
			free_pipe_list(funcp->body);
			ମୁକ୍ତ(funcp->name);
#  ଯଦି !BB_MMU
			ମୁକ୍ତ(funcp->body_as_string);
#  endif
		}
		ମୁକ୍ତ(funcp);
	}
}
# endif

# ଯଦି BB_MMU
#define exec_function(to_free, funcp, argv) \
	exec_function(funcp, argv)
# endif
ସ୍ଥିର ଶୂନ୍ୟ exec_function(ଅକ୍ଷର ***to_free,
		ଅଚଳ ସଂରଚନା function *funcp,
		ଅକ୍ଷର **argv) NORETURN;
ସ୍ଥିର ଶୂନ୍ୟ exec_function(ଅକ୍ଷର ***to_free,
		ଅଚଳ ସଂରଚନା function *funcp,
		ଅକ୍ଷର **argv)
{
# ଯଦି BB_MMU
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା n;

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
	G_flag_return_in_progress = -1;
	enter_var_nest_level();
	IF_HUSH_LOCAL(G.func_nest_level++;)

	/* On MMU, funcp->body is always non-NULL */
	n = run_list(funcp->body);
	_exit(n);
# ଅନ୍ୟଥା
//?	close_saved_fds_and_FILE_fds();

//TODO: check whether "true | func_with_return" works

	re_execute_shell(to_free,
			funcp->body_as_string,
			G.global_argv[0],
			argv + 1,
			ଖାଲି);
# endif
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା run_function(ଅଚଳ ସଂରଚନା function *funcp, ଅକ୍ଷର **argv)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
	save_arg_t sv;
	smallint sv_flg;

	save_and_replace_G_args(&sv, argv);

	/* "We are in function, ok to use return" */
	sv_flg = G_flag_return_in_progress;
	G_flag_return_in_progress = -1;

	/* Make "local" variables properly shadow previous ones */
	IF_HUSH_LOCAL(enter_var_nest_level();)
	IF_HUSH_LOCAL(G.func_nest_level++;)

	/* On MMU, funcp->body is always non-NULL */
# ଯଦି !BB_MMU
	ଯଦି (!funcp->body) {
		/* Function defined by -F */
		parse_and_run_string(funcp->body_as_string);
		rc = G.last_exitcode;
	} ଅନ୍ୟଥା
# endif
	{
		rc = run_list(funcp->body);
	}

	IF_HUSH_LOCAL(G.func_nest_level--;)
	IF_HUSH_LOCAL(leave_var_nest_level();)

	G_flag_return_in_progress = sv_flg;
# ଯଦି ENABLE_HUSH_TRAP
	debug_printf_exec("G.return_exitcode=-1\n");
	G.return_exitcode = -1; /* invalidate stashed return value */
# endif

	restore_G_args(&sv, argv);

	ଫେରନ୍ତୁ rc;
}
#endif /* ENABLE_HUSH_FUNCTIONS */

#ifndef __U_BOOT__
#ଯଦି BB_MMU
#define exec_builtin(to_free, x, argv) \
	exec_builtin(x, argv)
#ଅନ୍ୟଥା
#define exec_builtin(to_free, x, argv) \
	exec_builtin(to_free, argv)
#endif
ସ୍ଥିର ଶୂନ୍ୟ exec_builtin(ଅକ୍ଷର ***to_free,
		ଅଚଳ ସଂରଚନା built_in_command *x,
		ଅକ୍ଷର **argv) NORETURN;
ସ୍ଥିର ଶୂନ୍ୟ exec_builtin(ଅକ୍ଷର ***to_free,
		ଅଚଳ ସଂରଚନା built_in_command *x,
		ଅକ୍ଷର **argv)
{
#ଯଦି BB_MMU
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rcode;
//?	close_saved_fds_and_FILE_fds();
	rcode = x->b_function(argv);
	fflush_all();
	_exit(rcode);
#ଅନ୍ୟଥା
	fflush_all();
	/* On NOMMU, we must never block!
	 * Example: { sleep 99 | read line; } & echo Ok
	 */
	re_execute_shell(to_free,
			argv[0],
			G.global_argv[0],
			G.global_argv + 1,
			argv);
#endif
}
#endif /* !__U_BOOT__ */

#ifndef __U_BOOT__
ସ୍ଥିର ଶୂନ୍ୟ execvp_or_die(ଅକ୍ଷର **argv) NORETURN;
ସ୍ଥିର ଶୂନ୍ୟ execvp_or_die(ଅକ୍ଷର **argv)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା e;
	debug_printf_exec("execing '%s'\n", argv[0]);
	/* Don't propagate SIG_IGN to the child */
	ଯଦି (SPECIAL_JOBSTOP_SIGS != 0)
		switch_off_special_sigs(G.special_sig_mask & SPECIAL_JOBSTOP_SIGS);
	execvp(argv[0], argv);
	e = 2;
	ଯଦି (errno == EACCES) e = 126;
	ଯଦି (errno == ENOENT) e = 127;
	bb_perror_ସନ୍ଦେଶ("can't execute '%s'", argv[0]);
	_exit(e);
}

#ଯଦି ENABLE_HUSH_MODE_X
ସ୍ଥିର ଶୂନ୍ୟ x_mode_print_optionally_squoted(ଅଚଳ ଅକ୍ଷର *str)
{
	unsigned len;
	ଅଚଳ ଅକ୍ଷର *cp;

	cp = str;

	/* the set of chars which-cause-string-to-be-squoted mimics bash */
	/* test a char with: bash -c 'set -x; echo "CH"' */
	ଯଦି (str[strcspn(str, "\\\"'`$(){}[]<>;#&|~*?!^"
			" " "\001\002\003\004\005\006\007"
			"\010\011\012\013\014\015\016\017"
			"\020\021\022\023\024\025\026\027"
			"\030\031\032\033\034\035\036\037"
			)
		] == '\0'
	) {
		/* string has no special chars */
		x_mode_addstr(str);
		ଫେରନ୍ତୁ;
	}

	cp = str;
	ପାଇଁ (;;) {
		/* print '....' up to EOL or first squote */
		len = (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(strchrnul(cp, '\'') - cp);
		ଯଦି (len != 0) {
			x_mode_addchr('\'');
			x_mode_addblock(cp, len);
			x_mode_addchr('\'');
			cp += len;
		}
		ଯଦି (*cp == '\0')
			ଭାଙ୍ଗ;
		/* string contains squote(s), print them as \' */
		x_mode_addchr('\\');
		x_mode_addchr('\'');
		cp++;
	}
}
ସ୍ଥିର ଶୂନ୍ୟ dump_cmd_in_x_mode(ଅକ୍ଷର **argv)
{
	ଯଦି (G_x_mode && argv) {
		unsigned n;

		/* "+[+++...][ cmd...]\n\0" */
		x_mode_prefix();
		n = 0;
		ଯେତେବେଳେ (argv[n]) {
			x_mode_addchr(' ');
			ଯଦି (argv[n][0] == '\0') {
				x_mode_addchr('\'');
				x_mode_addchr('\'');
			} ଅନ୍ୟଥା {
				x_mode_print_optionally_squoted(argv[n]);
			}
			n++;
		}
		x_mode_flush();
	}
}
#ଅନ୍ୟଥା
# define dump_cmd_in_x_mode(argv) ((ଶୂନ୍ୟ)0)
#endif
#endif /* !__U_BOOT__ */

#ifndef __U_BOOT__
#ଯଦି ENABLE_HUSH_COMMAND
ସ୍ଥିର ଶୂନ୍ୟ if_command_vV_print_and_exit(ଅକ୍ଷର opt_vV, ଅକ୍ଷର *cmd, ଅଚଳ ଅକ୍ଷର *explanation)
{
	ଅକ୍ଷର *to_free;

	ଯଦି (!opt_vV)
		ଫେରନ୍ତୁ;

	to_free = ଖାଲି;
	ଯଦି (!explanation) {
		ଅକ୍ଷର *path = getenv("PATH");
		explanation = to_free = find_executable(cmd, &path); /* path == NULL is ok */
		ଯଦି (!explanation)
			_exit(1); /* PROG was not found */
		ଯଦି (opt_vV != 'V')
			cmd = to_free; /* -v PROG prints "/path/to/PROG" */
	}
	printf((opt_vV == 'V') ? "%s is %s\n" : "%s\n", cmd, explanation);
	ମୁକ୍ତ(to_free);
	fflush_all();
	_exit(0);
}
#ଅନ୍ୟଥା
# define if_command_vV_print_and_exit(a,b,c) ((ଶୂନ୍ୟ)0)
#endif
#endif /* !__U_BOOT__ */

#ଯଦି BB_MMU
#define pseudo_exec_argv(nommu_save, argv, assignment_cnt, argv_expanded) \
	pseudo_exec_argv(argv, assignment_cnt, argv_expanded)
#define pseudo_exec(nommu_save, command, argv_expanded) \
	pseudo_exec(command, argv_expanded)
#endif

#ifndef __U_BOOT__
/* Called after [v]fork() in run_pipe, or from builtin_exec.
 * Never returns.
 * Don't exit() here.  If you don't exec, use _exit instead.
 * The at_exit handlers apparently confuse the calling process,
 * in particular stdin handling. Not sure why? -- because of vfork! (vda)
 */
ସ୍ଥିର ଶୂନ୍ୟ pseudo_exec_argv(nommu_save_t *nommu_save,
		ଅକ୍ଷର **argv, ପୂର୍ଣ୍ଣସଂଖ୍ୟା assignment_cnt,
		ଅକ୍ଷର **argv_expanded) NORETURN;
ସ୍ଥିର NOINLINE ଶୂନ୍ୟ pseudo_exec_argv(nommu_save_t *nommu_save,
		ଅକ୍ଷର **argv, ପୂର୍ଣ୍ଣସଂଖ୍ୟା assignment_cnt,
		ଅକ୍ଷର **argv_expanded)
{
	ଅଚଳ ସଂରଚନା built_in_command *x;
	ସଂରଚନା variable **sv_shadowed;
	ଅକ୍ଷର **new_env;
	IF_HUSH_COMMAND(ଅକ୍ଷର opt_vV = 0;)
	IF_HUSH_FUNCTIONS(ଅଚଳ ସଂରଚନା function *funcp;)

	new_env = expand_assignments(argv, assignment_cnt);
	dump_cmd_in_x_mode(new_env);

	ଯଦି (!argv[assignment_cnt]) {
		/* Case when we are here: ... | var=val | ...
		 * (note that we do not exit early, i.e., do not optimize out
		 * expand_assignments(): think about ... | var=`sleep 1` | ...
		 */
		free_strings(new_env);
		_exit_SUCCESS();
	}

	sv_shadowed = G.shadowed_vars_pp;
#ଯଦି BB_MMU
	G.shadowed_vars_pp = ଖାଲି; /* "don't save, free them instead" */
#ଅନ୍ୟଥା
	G.shadowed_vars_pp = &nommu_save->old_vars;
	G.var_nest_level++;
#endif
	set_vars_and_save_old(new_env);
	G.shadowed_vars_pp = sv_shadowed;

	ଯଦି (argv_expanded) {
		argv = argv_expanded;
	} ଅନ୍ୟଥା {
		argv = expand_strvec_to_strvec(argv + assignment_cnt);
#ଯଦି !BB_MMU
		nommu_save->argv = argv;
#endif
	}
	dump_cmd_in_x_mode(argv);

#ଯଦି ENABLE_FEATURE_SH_STANDALONE || BB_MMU
	ଯଦି (strchr(argv[0], '/') != ଖାଲି)
		ଯାଅ skip;
#endif

#ଯଦି ENABLE_HUSH_FUNCTIONS
	/* Check if the command matches any functions (this goes before bltins) */
	funcp = find_function(argv[0]);
	ଯଦି (funcp)
		exec_function(&nommu_save->argv_from_re_execing, funcp, argv);
#endif

#ଯଦି ENABLE_HUSH_COMMAND
	/* "command BAR": run BAR without looking it up among functions
	 * "command -v BAR": print "BAR" or "/path/to/BAR"; or exit 1
	 * "command -V BAR": print "BAR is {a function,a shell builtin,/path/to/BAR}"
	 */
	ଯେତେବେଳେ (strcmp(argv[0], "command") == 0 && argv[1]) {
		ଅକ୍ଷର *p;

		argv++;
		p = *argv;
		ଯଦି (p[0] != '-' || !p[1])
			ଜାରି; /* bash allows "command command command [-OPT] BAR" */

		ପାଇଁ (;;) {
			p++;
			ବାଛ (*p) {
			ସ୍ଥିତି '\0':
				argv++;
				p = *argv;
				ଯଦି (p[0] != '-' || !p[1])
					ଯାଅ after_opts;
				ଜାରି; /* next arg is also -opts, process it too */
			ସ୍ଥିତି 'v':
			ସ୍ଥିତି 'V':
				opt_vV = *p;
				ଜାରି;
			ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
				bb_error_ସନ୍ଦେଶ_and_die("%s: %s: invalid option", "command", argv[0]);
			}
		}
	}
 after_opts:
# ଯଦି ENABLE_HUSH_FUNCTIONS
	ଯଦି (opt_vV && find_function(argv[0]))
		if_command_vV_print_and_exit(opt_vV, argv[0], "a function");
# endif
#endif

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
	x = BB_MMU ? find_builtin(argv[0]) : find_builtin1(argv[0]);
	ଯଦି (x) {
		if_command_vV_print_and_exit(opt_vV, argv[0], "a shell builtin");
		exec_builtin(&nommu_save->argv_from_re_execing, x, argv);
	}

#ଯଦି ENABLE_FEATURE_SH_STANDALONE
	/* Check if the command matches any busybox applets */
	{
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା a = find_applet_by_name(argv[0]);
		ଯଦି (a >= 0) {
			if_command_vV_print_and_exit(opt_vV, argv[0], "an applet");
# ଯଦି BB_MMU /* see above why on NOMMU it is not allowed */
			ଯଦି (APPLET_IS_NOEXEC(a)) {
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
				switch_off_special_sigs(G.special_sig_mask);
				debug_printf_exec("running applet '%s'\n", argv[0]);
				run_noexec_applet_and_exit(a, argv[0], argv);
			}
# endif
			/* Re-exec ourselves */
			debug_printf_exec("re-execing applet '%s'\n", argv[0]);
			/* Don't propagate SIG_IGN to the child */
			ଯଦି (SPECIAL_JOBSTOP_SIGS != 0)
				switch_off_special_sigs(G.special_sig_mask & SPECIAL_JOBSTOP_SIGS);
			execv(bb_busybox_exec_path, argv);
			/* If they called chroot or otherwise made the binary no longer
			 * executable, fall through */
		}
	}
#endif

#ଯଦି ENABLE_FEATURE_SH_STANDALONE || BB_MMU
 skip:
#endif
	if_command_vV_print_and_exit(opt_vV, argv[0], ଖାଲି);
	execvp_or_die(argv);
}

/* Called after [v]fork() in run_pipe
 */
ସ୍ଥିର ଶୂନ୍ୟ pseudo_exec(nommu_save_t *nommu_save,
		ସଂରଚନା command *command,
		ଅକ୍ଷର **argv_expanded) NORETURN;
ସ୍ଥିର ଶୂନ୍ୟ pseudo_exec(nommu_save_t *nommu_save,
		ସଂରଚନା command *command,
		ଅକ୍ଷର **argv_expanded)
{
#ଯଦି ENABLE_HUSH_FUNCTIONS
	ଯଦି (command->cmd_type == CMD_FUNCDEF) {
		/* Ignore funcdefs in pipes:
		 * true | f() { cmd }
		 */
		_exit(0);
	}
#endif

	ଯଦି (command->argv) {
		pseudo_exec_argv(nommu_save, command->argv,
				command->assignment_cnt, argv_expanded);
	}

	ଯଦି (command->group) {
		/* Cases when we are here:
		 * ( list )
		 * { list } &
		 * ... | ( list ) | ...
		 * ... | { list } | ...
		 */
#ଯଦି BB_MMU
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା rcode;
		debug_printf_exec("pseudo_exec: run_list\n");
		reset_traps_to_defaults();
		rcode = run_list(command->group);
		/* OK to leak memory by not calling free_pipe_list,
		 * since this process is about to exit */
		_exit(rcode);
#ଅନ୍ୟଥା
		re_execute_shell(&nommu_save->argv_from_re_execing,
				command->group_as_string,
				G.global_argv[0],
				G.global_argv + 1,
				ଖାଲି);
#endif
	}

	/* Case when we are here: ... | >file */
	debug_printf_exec("pseudo_exec'ed null command\n");
	_exit_SUCCESS();
}

#ଯଦି ENABLE_HUSH_JOB
ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *get_cmdtext(ସଂରଚନା pipe *pi)
{
	ଅକ୍ଷର **argv;
	ଅକ୍ଷର *p;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା len;

	/* This is subtle. ->cmdtext is created only on first backgrounding.
	 * (Think "cat, <ctrl-z>, fg, <ctrl-z>, fg, <ctrl-z>...." here...)
	 * On subsequent bg argv is trashed, but we won't use it */
	ଯଦି (pi->cmdtext)
		ଫେରନ୍ତୁ pi->cmdtext;

	argv = pi->cmds[0].argv;
	ଯଦି (!argv) {
		pi->cmdtext = xzalloc(1);
		ଫେରନ୍ତୁ pi->cmdtext;
	}
	len = 0;
	କର {
		len += strlen(*argv) + 1;
	} ଯେତେବେଳେ (*++argv);
	p = xmalloc(len);
	pi->cmdtext = p;
	argv = pi->cmds[0].argv;
	କର {
		p = stpcpy(p, *argv);
		*p++ = ' ';
	} ଯେତେବେଳେ (*++argv);
	p[-1] = '\0';
	ଫେରନ୍ତୁ pi->cmdtext;
}

ସ୍ଥିର ଶୂନ୍ୟ remove_job_from_table(ସଂରଚନା pipe *pi)
{
	ସଂରଚନା pipe *prev_pipe;

	ଯଦି (pi == G.job_list) {
		G.job_list = pi->next;
	} ଅନ୍ୟଥା {
		prev_pipe = G.job_list;
		ଯେତେବେଳେ (prev_pipe->next != pi)
			prev_pipe = prev_pipe->next;
		prev_pipe->next = pi->next;
	}
	G.last_jobid = 0;
	ଯଦି (G.job_list)
		G.last_jobid = G.job_list->jobid;
}

ସ୍ଥିର ଶୂନ୍ୟ delete_finished_job(ସଂରଚନା pipe *pi)
{
	remove_job_from_table(pi);
	free_pipe(pi);
}

ସ୍ଥିର ଶୂନ୍ୟ clean_up_last_dead_job(ଶୂନ୍ୟ)
{
	ଯଦି (G.job_list && !G.job_list->alive_cmds)
		delete_finished_job(G.job_list);
}

ସ୍ଥିର ଶୂନ୍ୟ ଭର୍ତ୍ତି_job_into_table(ସଂରଚନା pipe *pi)
{
	ସଂରଚନା pipe *job, **jobp;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

	clean_up_last_dead_job();

	/* Find the end of the list, and find next job ID to use */
	i = 0;
	jobp = &G.job_list;
	ଯେତେବେଳେ ((job = *jobp) != ଖାଲି) {
		ଯଦି (job->jobid > i)
			i = job->jobid;
		jobp = &job->next;
	}
	pi->jobid = i + 1;

	/* Create a new job struct at the end */
	job = *jobp = xmemdup(pi, ଆକାର(*pi));
	job->next = ଖାଲି;
	job->cmds = xzalloc(ଆକାର(pi->cmds[0]) * pi->ଅଙ୍କ_cmds);
	/* Cannot copy entire pi->cmds[] vector! This causes double frees */
	ପାଇଁ (i = 0; i < pi->ଅଙ୍କ_cmds; i++) {
		job->cmds[i].pid = pi->cmds[i].pid;
		/* all other fields are not used and stay zero */
	}
	job->cmdtext = xstrdup(get_cmdtext(pi));

	ଯଦି (G_interactive_fd)
		printf("[%u] %u %s\n", job->jobid, (unsigned)job->cmds[0].pid, job->cmdtext);
	G.last_jobid = job->jobid;
}
#endif /* JOB */

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା job_exited_or_stopped(ସଂରଚନା pipe *pi)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rcode, i;

	ଯଦି (pi->alive_cmds != pi->stopped_cmds)
		ଫେରନ୍ତୁ -1;

	/* All processes in fg pipe have exited or stopped */
	rcode = 0;
	i = pi->ଅଙ୍କ_cmds;
	ଯେତେବେଳେ (--i >= 0) {
		rcode = pi->cmds[i].cmd_exitcode;
		/* usually last process gives overall exitstatus,
		 * but with "set -o pipefail", last *failed* process does */
		ଯଦି (G.o_opt[OPT_O_PIPEFAIL] == 0 || rcode != 0)
			ଭାଙ୍ଗ;
	}
	IF_HAS_KEYWORDS(ଯଦି (pi->pi_inverted) rcode = !rcode;)
	ଫେରନ୍ତୁ rcode;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା process_wait_result(ସଂରଚନା pipe *fg_pipe, pid_t childpid, ପୂର୍ଣ୍ଣସଂଖ୍ୟା status)
{
#ଯଦି ENABLE_HUSH_JOB
	ସଂରଚନା pipe *pi;
#endif
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, dead;

	dead = WIFEXITED(status) || WIFSIGNALED(status);

#ଯଦି DEBUG_JOBS
	ଯଦି (WIFSTOPPED(status))
		debug_printf_jobs("pid %d stopped by sig %d (exitcode %d)\n",
				childpid, WSTOPSIG(status), WEXITSTATUS(status));
	ଯଦି (WIFSIGNALED(status))
		debug_printf_jobs("pid %d killed by sig %d (exitcode %d)\n",
				childpid, WTERMSIG(status), WEXITSTATUS(status));
	ଯଦି (WIFEXITED(status))
		debug_printf_jobs("pid %d exited, exitcode %d\n",
				childpid, WEXITSTATUS(status));
#endif
	/* Were we asked to wait for a fg pipe? */
	ଯଦି (fg_pipe) {
		i = fg_pipe->ଅଙ୍କ_cmds;

		ଯେତେବେଳେ (--i >= 0) {
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା rcode;

			debug_printf_jobs("check pid %d\n", fg_pipe->cmds[i].pid);
			ଯଦି (fg_pipe->cmds[i].pid != childpid)
				ଜାରି;
			ଯଦି (dead) {
				ପୂର୍ଣ୍ଣସଂଖ୍ୟା ex;
				fg_pipe->cmds[i].pid = 0;
				fg_pipe->alive_cmds--;
				ex = WEXITSTATUS(status);
				/* bash prints killer signal's name for *last*
				 * process in pipe (prints just newline for SIGINT/SIGPIPE).
				 * Mimic this. Example: "sleep 5" + (^\ or kill -QUIT)
				 */
				ଯଦି (WIFSIGNALED(status)) {
					ପୂର୍ଣ୍ଣସଂଖ୍ୟା sig = WTERMSIG(status);
#ଯଦି ENABLE_HUSH_JOB
					ଯଦି (G.run_list_level == 1
					/* ^^^^^ Do not print in nested contexts, example:
					 * echo `sleep 1; sh -c 'kill -9 $$'` - prints "137", NOT "Killed 137"
					 */
					 && i == fg_pipe->ଅଙ୍କ_cmds-1
					) {
						/* strsignal() is for bash compat. ~600 bloat versus bbox's get_signame() */
						puts(sig == SIGINT || sig == SIGPIPE ? "" : strsignal(sig));
					}
#endif
					/* TODO: if (WCOREDUMP(status)) + " (core dumped)"; */
					/* MIPS has 128 sigs (1..128), if sig==128,
					 * 128 + sig would result in exitcode 256 -> 0!
					 */
					ex = 128 | sig;
				}
				fg_pipe->cmds[i].cmd_exitcode = ex;
			} ଅନ୍ୟଥା {
				fg_pipe->stopped_cmds++;
			}
			debug_printf_jobs("fg_pipe: alive_cmds %d stopped_cmds %d\n",
					fg_pipe->alive_cmds, fg_pipe->stopped_cmds);
			rcode = job_exited_or_stopped(fg_pipe);
			ଯଦି (rcode >= 0) {
/* Note: *non-interactive* bash does not continue if all processes in fg pipe
 * are stopped. Testcase: "cat | cat" in a script (not on command line!)
 * and "killall -STOP cat" */
				ଯଦି (G_interactive_fd) {
#ଯଦି ENABLE_HUSH_JOB
					ଯଦି (fg_pipe->alive_cmds != 0)
						ଭର୍ତ୍ତି_job_into_table(fg_pipe);
#endif
					ଫେରନ୍ତୁ rcode;
				}
				ଯଦି (fg_pipe->alive_cmds == 0)
					ଫେରନ୍ତୁ rcode;
			}
			/* There are still running processes in the fg_pipe */
			ଫେରନ୍ତୁ -1;
		}
		/* It wasn't in fg_pipe, look for process in bg pipes */
	}

#ଯଦି ENABLE_HUSH_JOB
	/* We were asked to wait for bg or orphaned children */
	/* No need to remember exitcode in this case */
	ପାଇଁ (pi = G.job_list; pi; pi = pi->next) {
		ପାଇଁ (i = 0; i < pi->ଅଙ୍କ_cmds; i++) {
			ଯଦି (pi->cmds[i].pid == childpid)
				ଯାଅ found_pi_and_prognum;
		}
	}
	/* Happens when shell is used as init process (init=/bin/sh) */
	debug_printf("checkjobs: pid %d was not in our list!\n", childpid);
	ଫେରନ୍ତୁ -1; /* this wasn't a process from fg_pipe */

 found_pi_and_prognum:
	ଯଦି (dead) {
		/* child exited */
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା rcode = WEXITSTATUS(status);
		ଯଦି (WIFSIGNALED(status))
			/* NB: not 128 + sig, MIPS has sig 128 */
			rcode = 128 | WTERMSIG(status);
		pi->cmds[i].cmd_exitcode = rcode;
		ଯଦି (G.last_bg_pid == pi->cmds[i].pid)
			G.last_bg_pid_exitcode = rcode;
		pi->cmds[i].pid = 0;
		pi->alive_cmds--;
		ଯଦି (!pi->alive_cmds) {
# ଯଦି ENABLE_HUSH_BASH_COMPAT
			G.dead_job_exitcode = job_exited_or_stopped(pi);
# endif
			ଯଦି (G_interactive_fd) {
				printf(JOB_STATUS_FORMAT, pi->jobid,
						"Done", pi->cmdtext);
				delete_finished_job(pi);
			} ଅନ୍ୟଥା {
/*
 * bash deletes finished jobs from job table only in interactive mode,
 * after "jobs" cmd, or if pid of a new process matches one of the old ones
 * (see cleanup_dead_jobs(), delete_old_job(), J_NOTIFIED in bash source).
 * Testcase script: "(exit 3) & sleep 1; wait %1; echo $?" prints 3 in bash.
 * We only retain one "dead" job, if it's the single job on the list.
 * This covers most of real-world scenarios where this is useful.
 */
				ଯଦି (pi != G.job_list)
					delete_finished_job(pi);
			}
		}
	} ଅନ୍ୟଥା {
		/* child stopped */
		pi->stopped_cmds++;
	}
#endif
	ଫେରନ୍ତୁ -1; /* this wasn't a process from fg_pipe */
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା checkjobs(ସଂରଚନା pipe *fg_pipe, pid_t waitfor_pid)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା attributes;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା status;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rcode = 0;

	debug_printf_jobs("checkjobs %p\n", fg_pipe);

	attributes = WUNTRACED;
	ଯଦି (fg_pipe == ଖାଲି)
		attributes |= WNOHANG;

	errno = 0;
#ଯଦି ENABLE_HUSH_FAST
	ଯଦି (G.handled_SIGCHLD == G.ଗଣନା_SIGCHLD) {
//bb_error_msg("[%d] checkjobs: G.count_SIGCHLD:%d G.handled_SIGCHLD:%d children?:%d fg_pipe:%p",
//getpid(), G.count_SIGCHLD, G.handled_SIGCHLD, G.we_have_children, fg_pipe);
		/* There was neither fork nor SIGCHLD since last waitpid */
		/* Avoid doing waitpid syscall if possible */
		ଯଦି (!G.we_have_children) {
			errno = ECHILD;
			ଫେରନ୍ତୁ -1;
		}
		ଯଦି (fg_pipe == ଖାଲି) { /* is WNOHANG set? */
			/* We have children, but they did not exit
			 * or stop yet (we saw no SIGCHLD) */
			ଫେରନ୍ତୁ 0;
		}
		/* else: !WNOHANG, waitpid will block, can't short-circuit */
	}
#endif

/* Do we do this right?
 * bash-3.00# sleep 20 | false
 * <ctrl-Z pressed>
 * [3]+  Stopped          sleep 20 | false
 * bash-3.00# echo $?
 * 1   <========== bg pipe is not fully done, but exitcode is already known!
 * [hush 1.14.0: yes we do it right]
 */
	ଯେତେବେଳେ (1) {
		pid_t childpid;
#ଯଦି ENABLE_HUSH_FAST
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
		i = G.ଗଣନା_SIGCHLD;
#endif
		childpid = waitpid(-1, &status, attributes);
		ଯଦି (childpid <= 0) {
			ଯଦି (childpid && errno != ECHILD)
				bb_simple_perror_ସନ୍ଦେଶ("waitpid");
#ଯଦି ENABLE_HUSH_FAST
			ଅନ୍ୟଥା { /* Until next SIGCHLD, waitpid's are useless */
				G.we_have_children = (childpid == 0);
				G.handled_SIGCHLD = i;
//bb_error_msg("[%d] checkjobs: waitpid returned <= 0, G.count_SIGCHLD:%d G.handled_SIGCHLD:%d", getpid(), G.count_SIGCHLD, G.handled_SIGCHLD);
			}
#endif
			/* ECHILD (no children), or 0 (no change in children status) */
			rcode = childpid;
			ଭାଙ୍ଗ;
		}
		rcode = process_wait_result(fg_pipe, childpid, status);
		ଯଦି (rcode >= 0) {
			/* fg_pipe exited or stopped */
			ଭାଙ୍ଗ;
		}
		ଯଦି (childpid == waitfor_pid) { /* "wait PID" */
			debug_printf_exec("childpid==waitfor_pid:%d status:0x%08x\n", childpid, status);
			rcode = WEXITSTATUS(status);
			ଯଦି (WIFSIGNALED(status))
				rcode = 128 | WTERMSIG(status);
			ଯଦି (WIFSTOPPED(status))
				/* bash: "cmd & wait $!" and cmd stops: $? = 128 | stopsig */
				rcode = 128 | WSTOPSIG(status);
			rcode++;
			ଭାଙ୍ଗ; /* "wait PID" called us, give it exitcode+1 */
		}
#ଯଦି ENABLE_HUSH_BASH_COMPAT
		ଯଦି (-1 == waitfor_pid /* "wait -n" (wait for any one job) */
		 && G.dead_job_exitcode >= 0 /* some job did finish */
		) {
			debug_printf_exec("waitfor_pid:-1\n");
			rcode = G.dead_job_exitcode + 1;
			ଭାଙ୍ଗ;
		}
#endif
		/* This wasn't one of our processes, or */
		/* fg_pipe still has running processes, do waitpid again */
	} /* while (waitpid succeeds)... */

	ଫେରନ୍ତୁ rcode;
}

#ଯଦି ENABLE_HUSH_JOB
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା checkjobs_and_fg_shell(ସଂରଚନା pipe *fg_pipe)
{
	pid_t p;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rcode = checkjobs(fg_pipe, 0 /*(no pid to wait for)*/);
	ଯଦି (G_saved_tty_pgrp) {
		/* Job finished, move the shell to the foreground */
		p = getpgrp(); /* our process group id */
		debug_printf_jobs("fg'ing ourself: getpgrp()=%d\n", (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)p);
		tcsetpgrp(G_interactive_fd, p);
	}
	ଫେରନ୍ତୁ rcode;
}
#endif
#endif /* !__U_BOOT__ */

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
ସ୍ଥିର ଶୂନ୍ୟ set_G_ifs(ଶୂନ୍ୟ)
{
	/* Testcase: set -- q w e; (IFS='' echo "$*"; IFS=''; echo "$*"); echo "$*"
	 * Result should be 3 lines: q w e, qwe, q w e
	 */
	ଯଦି (G.ifs_whitespace != G.ifs)
		ମୁକ୍ତ(G.ifs_whitespace);
	G.ifs = get_local_var_ମୂଲ୍ୟ("IFS");
	ଯଦି (G.ifs) {
		ଅକ୍ଷର *p;
		G.ifs_whitespace = (ଅକ୍ଷର*)G.ifs;
		p = skip_whitespace(G.ifs);
		ଯଦି (*p) {
			/* Not all $IFS is whitespace */
			ଅକ୍ଷର *d;
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା len = p - G.ifs;
			p = skip_non_whitespace(p);
			G.ifs_whitespace = xmalloc(len + strlen(p) + 1); /* can overestimate */
			d = mempcpy(G.ifs_whitespace, G.ifs, len);
			ଯେତେବେଳେ (*p) {
				ଯଦି (isspace(*p))
					*d++ = *p;
				p++;
			}
			*d = '\0';
		}
	} ଅନ୍ୟଥା {
		G.ifs = defifs;
		G.ifs_whitespace = (ଅକ୍ଷର*)G.ifs;
	}
}
#ifndef __U_BOOT__
#ଯଦି !ENABLE_HUSH_MODE_X
#define redirect_and_varexp_helper(command, sqp, argv_expanded) \
	redirect_and_varexp_helper(command, sqp)
#endif
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା redirect_and_varexp_helper(
		ସଂରଚନା command *command,
		ସଂରଚନା squirrel **sqp,
		ଅକ୍ଷର **argv_expanded)
{
	/* Assignments occur before redirects. Try:
	 * a=`sleep 1` sleep 2 3>/qwe/rty
	 */

	ଅକ୍ଷର **new_env = expand_assignments(command->argv, command->assignment_cnt);
	dump_cmd_in_x_mode(new_env);
	dump_cmd_in_x_mode(argv_expanded);
	/* this takes ownership of new_env[i] elements, and frees new_env: */
	set_vars_and_save_old(new_env);

	ଫେରନ୍ତୁ setup_redirects(command, sqp);
}
#endif /* !__U_BOOT__ */

ସ୍ଥିର NOINLINE ପୂର୍ଣ୍ଣସଂଖ୍ୟା run_pipe(ସଂରଚନା pipe *pi)
{
	ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ null_ptr = ଖାଲି;

	ପୂର୍ଣ୍ଣସଂଖ୍ୟା cmd_no;
#ifndef __U_BOOT__
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା next_infd;
#endif /* !__U_BOOT__ */
	ସଂରଚନା command *command;
	ଅକ୍ଷର **argv_expanded;
	ଅକ୍ଷର **argv;
#ifndef __U_BOOT__
	ସଂରଚନା squirrel *squirrel = ଖାଲି;
#endif /* !__U_BOOT__ */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rcode;

#ifdef __U_BOOT__
	/*
	 * Set rcode here to avoid returning a garbage value in the middle of
	 * the function.
	 * Also, if an error occurs, rcode value would be changed and last
	 * return will signal the error.
	 */
	rcode = 0;
#endif /* __U_BOOT__ */

	debug_printf_exec("run_pipe start: members:%d\n", pi->ଅଙ୍କ_cmds);
	debug_enter();

	set_G_ifs();

#ifndef __U_BOOT__
	IF_HUSH_JOB(pi->pgrp = -1;)
	pi->stopped_cmds = 0;
#endif /* !__U_BOOT__ */
	command = &pi->cmds[0];
	argv_expanded = ଖାଲି;

#ifndef __U_BOOT__
	ଯଦି (pi->ଅଙ୍କ_cmds != 1
	 || pi->followup == PIPE_BG
	 || command->cmd_type == CMD_SUBSHELL
	) {
		ଯାଅ must_fork;
	}

	pi->alive_cmds = 1;
#endif /* !__U_BOOT__ */

	debug_printf_exec(": group:%p argv:'%s'\n",
		command->group, command->argv ? command->argv[0] : "NONE");

	ଯଦି (command->group) {
#ଯଦି ENABLE_HUSH_FUNCTIONS
		ଯଦି (command->cmd_type == CMD_FUNCDEF) {
			/* "executing" func () { list } */
			ସଂରଚନା function *funcp;

			funcp = new_function(command->argv[0]);
			/* funcp->name is already set to argv[0] */
			funcp->body = command->group;
# ଯଦି !BB_MMU
			funcp->body_as_string = command->group_as_string;
			command->group_as_string = ଖାଲି;
# endif
			command->group = ଖାଲି;
			command->argv[0] = ଖାଲି;
			debug_printf_exec("cmd %p has child func at %p\n", command, funcp);
			funcp->parent_cmd = command;
			command->child_func = funcp;

			debug_printf_exec("run_pipe: return EXIT_SUCCESS\n");
			debug_leave();
			ଫେରନ୍ତୁ EXIT_SUCCESS;
		}
#endif
		/* { list } */
		debug_printf_exec("non-subshell group\n");
		rcode = 1; /* exitcode if redir failed */
#ifndef __U_BOOT__
		ଯଦି (setup_redirects(command, &squirrel) == 0) {
#endif /* !__U_BOOT__ */
			debug_printf_exec(": run_list\n");
//FIXME: we need to pass squirrel down into run_list()
//for SH_STANDALONE case, or else this construct:
// { find /proc/self/fd; true; } >FILE; cmd2
//has no way of closing saved fd#1 for "find",
//and in SH_STANDALONE mode, "find" is not execed,
//therefore CLOEXEC on saved fd does not help.
			rcode = run_list(command->group) & 0xff;
#ifndef __U_BOOT__
		}
		restore_redirects(squirrel);
#endif /* !__U_BOOT__ */
		IF_HAS_KEYWORDS(ଯଦି (pi->pi_inverted) rcode = !rcode;)
		debug_leave();
		debug_printf_exec("run_pipe: return %d\n", rcode);
		ଫେରନ୍ତୁ rcode;
	}

	argv = command->argv ? command->argv : (ଅକ୍ଷର **) &null_ptr;
	{
#ifndef __U_BOOT__
		ଅଚଳ ସଂରଚନା built_in_command *x;
		IF_HUSH_FUNCTIONS(ଅଚଳ ସଂରଚନା function *funcp;)
		IF_NOT_HUSH_FUNCTIONS(ଗଣନା { funcp = 0 };)
		ସଂରଚନା variable **sv_shadowed;
#endif /* !__U_BOOT__ */
		ସଂରଚନା variable *old_vars;

#ଯଦି ENABLE_HUSH_LINENO_VAR
		G.execute_lineno = command->lineno;
#endif

		ଯଦି (argv[command->assignment_cnt] == ଖାଲି) {
			/* Assignments, but no command.
			 * Ensure redirects take effect (that is, create files).
			 * Try "a=t >file"
			 */
			unsigned i;
			G.expand_exitcode = 0;
 only_assignments:
#ifndef __U_BOOT__
			rcode = setup_redirects(command, &squirrel);
			restore_redirects(squirrel);
#endif /* !__U_BOOT__ */

			/* Set shell variables */
			i = 0;
			ଯେତେବେଳେ (i < command->assignment_cnt) {
				ଅକ୍ଷର *p = expand_string_to_string(argv[i],
						EXP_FLAG_ESC_GLOB_CHARS,
						/*unbackslash:*/ 1
				);
#ଯଦି ENABLE_HUSH_MODE_X
				ଯଦି (G_x_mode) {
					ଅକ୍ଷର *eq;
					ଯଦି (i == 0)
						x_mode_prefix();
					x_mode_addchr(' ');
					eq = strchrnul(p, '=');
					ଯଦି (*eq) eq++;
					x_mode_addblock(p, (eq - p));
					x_mode_print_optionally_squoted(eq);
					x_mode_flush();
				}
#endif
				debug_printf_env("set shell var:'%s'->'%s'\n", *argv, p);
#ifndef __U_BOOT__
				ଯଦି (set_local_var0(p)) {
#ଅନ୍ୟଥା /* __U_BOOT__ */
				ଯଦି (set_local_var_modern(p, /*flag:*/ 0)) {
#endif
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
			ଯଦି (rcode == 0)
				rcode = G.expand_exitcode;
			IF_HAS_KEYWORDS(ଯଦି (pi->pi_inverted) rcode = !rcode;)
			debug_leave();
			debug_printf_exec("run_pipe: return %d\n", rcode);
			ଫେରନ୍ତୁ rcode;
		}

		/* Expand the rest into (possibly) many strings each */
#ଯଦି defined(CMD_TEST2_SINGLEWORD_NOGLOB)
		ଯଦି (command->cmd_type == CMD_TEST2_SINGLEWORD_NOGLOB)
			argv_expanded = expand_strvec_to_strvec_singleword_noglob(argv + command->assignment_cnt);
		ଅନ୍ୟଥା
#endif
#ଯଦି defined(CMD_SINGLEWORD_NOGLOB)
		ଯଦି (command->cmd_type == CMD_SINGLEWORD_NOGLOB)
			argv_expanded = expand_strvec_to_strvec_singleword_noglob(argv + command->assignment_cnt);
		ଅନ୍ୟଥା
#endif
			argv_expanded = expand_strvec_to_strvec(argv + command->assignment_cnt);

		/* If someone gives us an empty string: `cmd with empty output` */
		ଯଦି (!argv_expanded[0]) {
			ମୁକ୍ତ(argv_expanded);
			/* `false` still has to set exitcode 1 */
			G.expand_exitcode = G.last_exitcode;
			ଯାଅ only_assignments;
		}

		old_vars = ଖାଲି;
#ifndef __U_BOOT__
		sv_shadowed = G.shadowed_vars_pp;

		/* Check if argv[0] matches any functions (this goes before bltins) */
		IF_HUSH_FUNCTIONS(funcp = find_function(argv_expanded[0]);)
		IF_HUSH_FUNCTIONS(x = ଖାଲି;)
		IF_HUSH_FUNCTIONS(ଯଦି (!funcp))
			x = find_builtin(argv_expanded[0]);
		ଯଦି (x || funcp) {
			ଯଦି (x && x->b_function == builtin_exec && argv_expanded[1] == ଖାଲି) {
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

				ଯାଅ clean_up_and_ret1;
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
			ଯଦି (rcode == 0) {
				ଯଦି (!funcp) {
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
#ଯଦି ENABLE_HUSH_FUNCTIONS
				ଅନ୍ୟଥା {
					debug_printf_exec(": function '%s' '%s'...\n",
						funcp->name, argv_expanded[1]);
					rcode = run_function(funcp, argv_expanded) & 0xff;
					/*
					 * But do collect *to old_vars list* vars shadowed
					 * within function execution. To that end, restore
					 * this pointer _after_ function run:
					 */
					G.shadowed_vars_pp = sv_shadowed;
				}
#endif
			}
		} ଅନ୍ୟଥା
		ଯଦି (ENABLE_FEATURE_SH_NOFORK && ଅଙ୍କ_APPLETS > 1) {
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = find_applet_by_name(argv_expanded[0]);
			ଯଦି (n < 0 || !APPLET_IS_NOFORK(n))
				ଯାଅ must_fork;

			enter_var_nest_level();
			/* Collect all variables "shadowed" by helper into old_vars list */
			G.shadowed_vars_pp = &old_vars;
			rcode = redirect_and_varexp_helper(command, &squirrel, argv_expanded);
			G.shadowed_vars_pp = sv_shadowed;

			ଯଦି (rcode == 0) {
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
				rcode = run_nofork_applet(n, argv_expanded);
			}
		} ଅନ୍ୟଥା
			ଯାଅ must_fork;

		restore_redirects(squirrel);
 clean_up_and_ret1:
		leave_var_nest_level();
		add_vars(old_vars);

		/*
		 * Try "usleep 99999999" + ^C + "echo $?"
		 * with FEATURE_SH_NOFORK=y.
		 */
		ଯଦି (!funcp) {
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
			ଯଦି (sigismember(&G.pending_set, SIGINT))
				rcode = 128 | SIGINT;
		}
		ମୁକ୍ତ(argv_expanded);
		IF_HAS_KEYWORDS(ଯଦି (pi->pi_inverted) rcode = !rcode;)
		debug_leave();
		debug_printf_exec("run_pipe return %d\n", rcode);
		ଫେରନ୍ତୁ rcode;
#endif /* !__U_BOOT__ */
	}

#ifndef __U_BOOT__
 must_fork:
	/* NB: argv_expanded may already be created, and that
	 * might include `cmd` runs! Do not rerun it! We *must*
	 * use argv_expanded if it's non-NULL */

	/* Going to fork a child per each pipe member */
	pi->alive_cmds = 0;
	next_infd = 0;
#endif /* !__U_BOOT__ */

	cmd_no = 0;
	ଯେତେବେଳେ (cmd_no < pi->ଅଙ୍କ_cmds) {
#ifndef __U_BOOT__
		ସଂରଚନା fd_pair pipefds;
#ଯଦି !BB_MMU
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା sv_var_nest_level = G.var_nest_level;
		ଅସ୍ଥିର nommu_save_t nommu_save;
		nommu_save.old_vars = ଖାଲି;
		nommu_save.argv = ଖାଲି;
		nommu_save.argv_from_re_execing = ଖାଲି;
#endif
#endif /* !__U_BOOT__ */
		command = &pi->cmds[cmd_no];
		cmd_no++;

#ifdef __U_BOOT__
		/* Replace argv and argc by expanded if it exists. */
		ଯଦି (argv_expanded) {
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
			command->argc = list_size(argv_expanded);
			command->argv = argv_expanded;
		}
#endif /* __U_BOOT__ */
			ଯଦି (command->argv) {
			debug_printf_exec(": pipe member '%s' '%s'...\n",
					command->argv[0], command->argv[1]);
		} ଅନ୍ୟଥା {
			debug_printf_exec(": pipe member with no argv\n");
		}

#ifndef __U_BOOT__
		/* pipes are inserted between pairs of commands */
		pipefds.rd = 0;
		pipefds.wr = 1;
		ଯଦି (cmd_no < pi->ଅଙ୍କ_cmds)
			xpiped_pair(pipefds);

#ଯଦି ENABLE_HUSH_LINENO_VAR
		G.execute_lineno = command->lineno;
#endif

		command->pid = BB_MMU ? fork() : vfork();
		ଯଦି (!command->pid) { /* child */
#ଯଦି ENABLE_HUSH_JOB
			disable_restore_tty_pgrp_on_exit();
			CLEAR_RANDOM_T(&G.random_gen); /* or else $RANDOM repeats in child */

			/* Every child adds itself to new process group
			 * with pgid == pid_of_first_child_in_pipe */
			ଯଦି (G.run_list_level == 1 && G_interactive_fd) {
				pid_t pgrp;
				pgrp = pi->pgrp;
				ଯଦି (pgrp < 0) /* true for 1st process only */
					pgrp = getpid();
				ଯଦି (setpgid(0, pgrp) == 0
				 && pi->followup != PIPE_BG
				 && G_saved_tty_pgrp /* we have ctty */
				) {
					/* We do it in *every* child, not just first,
					 * to avoid races */
					tcsetpgrp(G_interactive_fd, pgrp);
				}
			}
#endif
			ଯଦି (pi->alive_cmds == 0 && pi->followup == PIPE_BG) {
				/* 1st cmd in backgrounded pipe
				 * should have its stdin /dev/null'ed */
				close(0);
				ଯଦି (open(bb_dev_null, O_RDONLY))
					xopen("/", O_RDONLY);
			} ଅନ୍ୟଥା {
				xmove_fd(next_infd, 0);
			}
			xmove_fd(pipefds.wr, 1);
			ଯଦି (pipefds.rd > 1)
				close(pipefds.rd);
			/* Like bash, explicit redirects override pipes,
			 * and the pipe fd (fd#1) is available for dup'ing:
			 * "cmd1 2>&1 | cmd2": fd#1 is duped to fd#2, thus stderr
			 * of cmd1 goes into pipe.
			 */
			ଯଦି (setup_redirects(command, ଖାଲି)) {
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
#ଯଦି ENABLE_HUSH_FAST
		G.ଗଣନା_SIGCHLD++;
//bb_error_msg("[%d] fork in run_pipe: G.count_SIGCHLD:%d G.handled_SIGCHLD:%d", getpid(), G.count_SIGCHLD, G.handled_SIGCHLD);
#endif
		enable_restore_tty_pgrp_on_exit();
#ଯଦି !BB_MMU
		/* Clean up after vforked child */
		ମୁକ୍ତ(nommu_save.argv);
		ମୁକ୍ତ(nommu_save.argv_from_re_execing);
		G.var_nest_level = sv_var_nest_level;
		remove_nested_vars();
		add_vars(nommu_save.old_vars);
#endif
		ମୁକ୍ତ(argv_expanded);
		argv_expanded = ଖାଲି;
		ଯଦି (command->pid < 0) { /* [v]fork failed */
			/* Clearly indicate, was it fork or vfork */
			bb_simple_perror_ସନ୍ଦେଶ(BB_MMU ? "vfork"+1 : "vfork");
		} ଅନ୍ୟଥା {
			pi->alive_cmds++;
#ଯଦି ENABLE_HUSH_JOB
			/* Second and next children need to know pid of first one */
			ଯଦି (pi->pgrp < 0)
				pi->pgrp = command->pid;
#endif
		}

		ଯଦି (cmd_no > 1)
			close(next_infd);
		ଯଦି (cmd_no < pi->ଅଙ୍କ_cmds)
			close(pipefds.wr);
		/* Pass read (output) pipe end to next iteration */
		next_infd = pipefds.rd;
#ଅନ୍ୟଥା /* __U_BOOT__ */
		/* Process the command */
		rcode = cmd_process(G.do_repeat ? CMD_FLAG_REPEAT : 0,
				    command->argc, command->argv,
				    &(G.flag_repeat), ଖାଲି);

		ଯଦି (argv_expanded) {
			/*
			 * expand_strvec_to_strvec() allocates memory to expand
			 * argv, we need to free it.
			 */
			ମୁକ୍ତ(argv_expanded);

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
#endif /* __U_BOOT__ */
	}

#ifndef __U_BOOT__
	ଯଦି (!pi->alive_cmds) {
		debug_leave();
		debug_printf_exec("run_pipe return 1 (all forks failed, no children)\n");
		ଫେରନ୍ତୁ 1;
	}
#endif /* __U_BOOT__ */

	debug_leave();
#ifndef __U_BOOT__
	debug_printf_exec("run_pipe return -1 (%u children started)\n", pi->alive_cmds);
	ଫେରନ୍ତୁ -1;
#ଅନ୍ୟଥା /* __U_BOOT__ */
	IF_HAS_KEYWORDS(ଯଦି (pi->pi_inverted) rcode = !rcode;)
	debug_printf_exec("run_pipe return %d\n", rcode);
	ଫେରନ୍ତୁ rcode;
#endif /* __U_BOOT__ */
}

/* NB: called by pseudo_exec, and therefore must not modify any
 * global data until exec/_exit (we can be a child after vfork!) */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା run_list(ସଂରଚନା pipe *pi)
{
#ଯଦି ENABLE_HUSH_CASE
	ଅକ୍ଷର *case_word = ଖାଲି;
#endif
#ଯଦି ENABLE_HUSH_LOOPS
	ସଂରଚନା pipe *loop_ଶୀର୍ଷ = ଖାଲି;
	ଅକ୍ଷର **for_lcur = ଖାଲି;
	ଅକ୍ଷର **for_list = ଖାଲି;
#endif
	smallint last_followup;
	smalluint rcode;
#ଯଦି ENABLE_HUSH_IF || ENABLE_HUSH_CASE
	smalluint cond_code = 0;
#ଅନ୍ୟଥା
	ଗଣନା { cond_code = 0 };
#endif
#ଯଦି HAS_KEYWORDS
	smallint rword;      /* RES_foo */
	smallint last_rword; /* ditto */
#endif

#ifndef __U_BOOT__
	debug_printf_exec("run_list lvl %d start\n", G.run_list_level);
	debug_enter();
#endif /* !__U_BOOT__ */

	set_G_ifs();

#ଯଦି ENABLE_HUSH_LOOPS
	/* Check syntax for "for" */
	{
		ସଂରଚନା pipe *cpipe;
		ପାଇଁ (cpipe = pi; cpipe; cpipe = cpipe->next) {
			ଯଦି (cpipe->res_word != RES_FOR && cpipe->res_word != RES_IN)
				ଜାରି;
			/* current word is FOR or IN (BOLD in comments below) */
			ଯଦି (cpipe->next == ଖାଲି) {
				syntax_error("malformed for");
				debug_leave();
				debug_printf_exec("run_list lvl %d return 1\n", G.run_list_level);
				ଫେରନ୍ତୁ 1;
			}
			/* "FOR v; do ..." and "for v IN a b; do..." are ok */
			ଯଦି (cpipe->next->res_word == RES_DO)
				ଜାରି;
			/* next word is not "do". It must be "in" then ("FOR v in ...") */
			ଯଦି (cpipe->res_word == RES_IN /* "for v IN a b; not_do..."? */
			 || cpipe->next->res_word != RES_IN /* FOR v not_do_and_not_in..."? */
			) {
				syntax_error("malformed for");
				debug_leave();
				debug_printf_exec("run_list lvl %d return 1\n", G.run_list_level);
				ଫେରନ୍ତୁ 1;
			}
		}
	}
#endif

	/* Past this point, all code paths should jump to ret: label
	 * in order to return, no direct "return" statements please.
	 * This helps to ensure that no memory is leaked. */

#ଯଦି ENABLE_HUSH_JOB
	G.run_list_level++;
#endif

#ଯଦି HAS_KEYWORDS
	rword = RES_NONE;
	last_rword = RES_XXXX;
#endif
	last_followup = PIPE_SEQ;
	rcode = G.last_exitcode;

	/* Go through list of pipes, (maybe) executing them. */
#ifndef __U_BOOT__
	ପାଇଁ (; pi; pi = IF_HUSH_LOOPS(rword == RES_DONE ? loop_ଶୀର୍ଷ : ) pi->next) {
#ଅନ୍ୟଥା /* __U_BOOT__ */
	ପାଇଁ (; pi; pi = rword == RES_DONE ? loop_ଶୀର୍ଷ : pi->next) {
#endif /* __U_BOOT__ */
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା r;
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା sv_errexit_depth;

#ifndef __U_BOOT__
		ଯଦି (G.flag_SIGINT)
			ଭାଙ୍ଗ;
		ଯଦି (G_flag_return_in_progress == 1)
			ଭାଙ୍ଗ;
#endif /* !__U_BOOT__ */

		IF_HAS_KEYWORDS(rword = pi->res_word;)
		debug_printf_exec(": rword:%d cond_code:%d last_rword:%d\n",
				rword, cond_code, last_rword);

		sv_errexit_depth = G.errexit_depth;
		ଯଦି (
#ଯଦି ENABLE_HUSH_IF
		    rword == RES_IF || rword == RES_ELIF ||
#endif
		    pi->followup != PIPE_SEQ
		) {
			G.errexit_depth++;
		}
#ଯଦି ENABLE_HUSH_LOOPS
		ଯଦି ((rword == RES_WHILE || rword == RES_UNTIL || rword == RES_FOR)
		 && loop_ଶୀର୍ଷ == ଖାଲି /* avoid bumping G.depth_of_loop twice */
		) {
			/* start of a loop: remember where loop starts */
			loop_ଶୀର୍ଷ = pi;
			G.depth_of_loop++;
		}
#endif
		/* Still in the same "if...", "then..." or "do..." branch? */
		ଯଦି (IF_HAS_KEYWORDS(rword == last_rword &&) 1) {
			ଯଦି ((rcode == 0 && last_followup == PIPE_OR)
			 || (rcode != 0 && last_followup == PIPE_AND)
			) {
				/* It is "<true> || CMD" or "<false> && CMD"
				 * and we should not execute CMD */
				debug_printf_exec("skipped cmd because of || or &&\n");
				last_followup = pi->followup;
				ଯାଅ dont_check_jobs_but_continue;
			}
		}
		last_followup = pi->followup;
#ଯଦି ENABLE_HUSH_IF
		ଯଦି (cond_code != 0) {
			ଯଦି (rword == RES_THEN) {
				/* if false; then ... fi has exitcode 0! */
				G.last_exitcode = rcode = EXIT_SUCCESS;
				/* "if <false> THEN cmd": skip cmd */
				debug_printf_exec("skipped THEN cmd because IF condition was false\n");
				last_rword = rword;
				ଜାରି;
			}
		} ଅନ୍ୟଥା {
			ଯଦି (rword == RES_ELSE
			 || (rword == RES_ELIF && last_rword != RES_ELIF)
			) {
				/* "if <true> then ... ELSE/ELIF cmd":
				 * skip cmd and all following ones */
				debug_printf_exec("skipped ELSE/ELIF branch because IF condition was true\n");
				ଭାଙ୍ଗ;
			}
			//if (rword == RES_THEN): "if <true> THEN cmd", run cmd (fall through)
		}
#endif
		IF_HAS_KEYWORDS(last_rword = rword;)
#ଯଦି ENABLE_HUSH_LOOPS
		ଯଦି (rword == RES_FOR) { /* && pi->num_cmds - always == 1 */
			ଯଦି (!for_lcur) {
				/* first loop through for */

				ସ୍ଥିର ଅଚଳ ଅକ୍ଷର encoded_dollar_at[] ALIGN1 = {
					SPECIAL_VAR_ପ୍ରତୀକ, '@' | 0x80, SPECIAL_VAR_ପ୍ରତୀକ, '\0'
				}; /* encoded representation of "$@" */
				ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *ଅଚଳ encoded_dollar_at_argv[] ALIGN_PTR = {
					encoded_dollar_at, ଖାଲି
				}; /* argv list with one element: "$@" */
				ଅକ୍ଷର **vals;

				G.last_exitcode = rcode = EXIT_SUCCESS;
				vals = (ଅକ୍ଷର**)encoded_dollar_at_argv;
				ଯଦି (pi->next->res_word == RES_IN) {
					/* if no variable values after "in" we skip "for" */
					ଯଦି (!pi->next->cmds[0].argv) {
						debug_printf_exec(": null FOR: exitcode EXIT_SUCCESS\n");
						ଭାଙ୍ଗ;
					}
					vals = pi->next->cmds[0].argv;
				} /* else: "for var; do..." -> assume "$@" list */
				/* create list of variable values */
				debug_print_strings("for_list made from", vals);
				for_list = expand_strvec_to_strvec(vals);
				for_lcur = for_list;
				debug_print_strings("for_list", for_list);
			}
			ଯଦି (!*for_lcur) {
				/* "for" loop is over, clean up */
				ମୁକ୍ତ(for_list);
				for_list = ଖାଲି;
				for_lcur = ଖାଲି;
				ଭାଙ୍ଗ;
			}
			/* Insert next value from for_lcur */
			/* note: *for_lcur already has quotes removed, $var expanded, etc */
#ifndef __U_BOOT__
			set_local_var_from_halves(pi->cmds[0].argv[0], *for_lcur++);
#ଅନ୍ୟଥା /* __U_BOOT__ */
			/* We cannot use xasprintf, so we emulate it. */
			ଅକ୍ଷର *full_var;
			ଅକ୍ଷର *var = pi->cmds[0].argv[0];
			ଅକ୍ଷର *val = *for_lcur++;

			/* + 1 to take into account =. */
			full_var = xmalloc(strlen(var) + strlen(val) + 1);
			sprintf(full_var, "%s=%s", var, val);

			set_local_var_modern(full_var, /*flag:*/ 0);
#endif /* __U_BOOT__ */
			ଜାରି;
		}
		ଯଦି (rword == RES_IN) {
			ଜାରି; /* "for v IN list;..." - "in" has no cmds anyway */
		}
		ଯଦି (rword == RES_DONE) {
			ଜାରି; /* "done" has no cmds too */
		}
#endif
#ଯଦି ENABLE_HUSH_CASE
		ଯଦି (rword == RES_CASE) {
			debug_printf_exec("CASE cond_code:%d\n", cond_code);
			case_word = expand_string_to_string(pi->cmds->argv[0],
				EXP_FLAG_ESC_GLOB_CHARS, /*unbackslash:*/ 1);
			debug_printf_exec("CASE word1:'%s'\n", case_word);
			//unbackslash(case_word);
			//debug_printf_exec("CASE word2:'%s'\n", case_word);
			ଜାରି;
		}
		ଯଦି (rword == RES_MATCH) {
			ଅକ୍ଷର **argv;

			debug_printf_exec("MATCH cond_code:%d\n", cond_code);
			ଯଦି (!case_word) /* "case ... matched_word) ... WORD)": we executed selected branch, stop */
				ଭାଙ୍ଗ;
			/* all prev words didn't match, does this one match? */
			argv = pi->cmds->argv;
			ଯେତେବେଳେ (*argv) {
				ଅକ୍ଷର *pattern;
				debug_printf_exec("expand_string_to_string('%s')\n", *argv);
				pattern = expand_string_to_string(*argv,
						EXP_FLAG_ESC_GLOB_CHARS,
						/*unbackslash:*/ 0
				);
				/* TODO: which FNM_xxx flags to use? */
				cond_code = (fnmatch(pattern, case_word, /*flags:*/ 0) != 0);
				debug_printf_exec("cond_code=fnmatch(pattern:'%s',str:'%s'):%d\n",
						pattern, case_word, cond_code);
				ମୁକ୍ତ(pattern);
				ଯଦି (cond_code == 0) {
					/* match! we will execute this branch */
					ମୁକ୍ତ(case_word);
					case_word = ଖାଲି; /* make future "word)" stop */
					ଭାଙ୍ଗ;
				}
				argv++;
			}
			ଜାରି;
		}
		ଯଦି (rword == RES_CASE_BODY) { /* inside of a case branch */
			debug_printf_exec("CASE_BODY cond_code:%d\n", cond_code);
			ଯଦି (cond_code != 0)
				ଜାରି; /* not matched yet, skip this pipe */
		}
		ଯଦି (rword == RES_ESAC) {
			debug_printf_exec("ESAC cond_code:%d\n", cond_code);
			ଯଦି (case_word) {
				/* "case" did not match anything: still set $? (to 0) */
				G.last_exitcode = rcode = EXIT_SUCCESS;
			}
		}
#endif
		/* Just pressing <enter> in shell should check for jobs.
		 * OTOH, in non-interactive shell this is useless
		 * and only leads to extra job checks */
		ଯଦି (pi->ଅଙ୍କ_cmds == 0) {
#ifndef __U_BOOT__
			ଯଦି (G_interactive_fd)
				ଯାଅ check_jobs_and_continue;
#endif /* !__U_BOOT__ */
			ଜାରି;
		}

		/* After analyzing all keywords and conditions, we decided
		 * to execute this pipe. NB: have to do checkjobs(NULL)
		 * after run_pipe to collect any background children,
		 * even if list execution is to be stopped. */
		debug_printf_exec(": run_pipe with %d members\n", pi->ଅଙ୍କ_cmds);
#ifndef __U_BOOT__
#ଯଦି ENABLE_HUSH_LOOPS
		G.flag_break_continue = 0;
#endif
#endif /* !__U_BOOT__ */
#ifndef __U_BOOT__
		rcode = r = G.o_opt[OPT_O_NOEXEC] ? 0 : run_pipe(pi);
		/* NB: rcode is a smalluint, r is int */
#ଅନ୍ୟଥା /* __U_BOOT__ */
		rcode = r = run_pipe(pi); /* NB: rcode is a smalluint, r is int */
		ଯଦି (r <= EXIT_RET_CODE) {
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା previous_rcode = G.last_exitcode;
			/*
			 * This magic is to get the exit code given by the user.
			 * Contrary to old shell code, we use + EXIT_RET_CODE as EXIT_RET_CODE
			 * equals -2.
			 */
			G.last_exitcode = -r + EXIT_RET_CODE;

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
			ଯଦି (previous_rcode != r) {
				/*
				 * If run from run_command, run_command_flags will be set, so we check
				 * this to know if we are in main input shell.
				 */
				ଯଦି (!G.run_command_flags)
					printf("exit not allowed from main input shell.\n");

				ଭାଙ୍ଗ;
			}
			ଜାରି;
		}
#endif /* __U_BOOT__ */
		ଯଦି (r != -1) {
			/* We ran a builtin, function, or group.
			 * rcode is already known
			 * and we don't need to wait for anything. */
			debug_printf_exec(": builtin/func exitcode %d\n", rcode);
			G.last_exitcode = rcode;
#ifndef __U_BOOT__
			check_and_run_traps();
#endif /* !__U_BOOT__ */
#ଯଦି ENABLE_HUSH_TRAP && ENABLE_HUSH_FUNCTIONS
			rcode = G.last_exitcode; /* "return" in trap can change it, read back */
#endif
#ifndef __U_BOOT__
#ଯଦି ENABLE_HUSH_LOOPS
			/* Was it "break" or "continue"? */
			ଯଦି (G.flag_break_continue) {
				smallint fbc = G.flag_break_continue;
				/* We might fall into outer *loop*,
				 * don't want to break it too */
				ଯଦି (loop_ଶୀର୍ଷ) {
					G.depth_break_continue--;
					ଯଦି (G.depth_break_continue == 0)
						G.flag_break_continue = 0;
					/* else: e.g. "continue 2" should *break* once, *then* continue */
				} /* else: "while... do... { we are here (innermost list is not a loop!) };...done" */
				ଯଦି (G.depth_break_continue != 0 || fbc == BC_BREAK) {
					checkjobs(ଖାଲି, 0 /*(no pid to wait for)*/);
					ଭାଙ୍ଗ;
				}
				/* "continue": simulate end of loop */
				rword = RES_DONE;
				ଜାରି;
			}
#endif
			ଯଦି (G_flag_return_in_progress == 1) {
				checkjobs(ଖାଲି, 0 /*(no pid to wait for)*/);
				ଭାଙ୍ଗ;
			}

		} ଅନ୍ୟଥା ଯଦି (pi->followup == PIPE_BG) {
			/* What does bash do with attempts to background builtins? */
			/* even bash 3.2 doesn't do that well with nested bg:
			 * try "{ { sleep 10; echo DEEP; } & echo HERE; } &".
			 * I'm NOT treating inner &'s as jobs */
#ଯଦି ENABLE_HUSH_JOB
			ଯଦି (G.run_list_level == 1)
				ଭର୍ତ୍ତି_job_into_table(pi);
#endif
			/* Last command's pid goes to $! */
			G.last_bg_pid = pi->cmds[pi->ଅଙ୍କ_cmds - 1].pid;
			G.last_bg_pid_exitcode = 0;
			debug_printf_exec(": cmd&: exitcode EXIT_SUCCESS\n");
/* Check pi->pi_inverted? "! sleep 1 & echo $?": bash says 1. dash and ash say 0 */
			rcode = EXIT_SUCCESS;
			ଯାଅ check_traps;
		} ଅନ୍ୟଥା {
#ଯଦି ENABLE_HUSH_JOB
			ଯଦି (G.run_list_level == 1 && G_interactive_fd) {
				/* Waits for completion, then fg's main shell */
				rcode = checkjobs_and_fg_shell(pi);
				debug_printf_exec(": checkjobs_and_fg_shell exitcode %d\n", rcode);
				ଯାଅ check_traps;
			}
#endif
			/* This one just waits for completion */
			rcode = checkjobs(pi, 0 /*(no pid to wait for)*/);
			debug_printf_exec(": checkjobs exitcode %d\n", rcode);
 check_traps:
			G.last_exitcode = rcode;
			check_and_run_traps();
#ଯଦି ENABLE_HUSH_TRAP && ENABLE_HUSH_FUNCTIONS
			rcode = G.last_exitcode; /* "return" in trap can change it, read back */
#endif
		}
#endif /* !__U_BOOT__ */

#ifndef __U_BOOT__
		/* Handle "set -e" */
		ଯଦି (rcode != 0 && G.o_opt[OPT_O_ERREXIT]) {
			debug_printf_exec("ERREXIT:1 errexit_depth:%d\n", G.errexit_depth);
			ଯଦି (G.errexit_depth == 0)
				hush_exit(rcode);
		}
#ଅନ୍ୟଥା /* __U_BOOT__ */
		} /* if (r != -1) */
#endif /* __U_BOOT__ */
		G.errexit_depth = sv_errexit_depth;

		/* Analyze how result affects subsequent commands */
#ଯଦି ENABLE_HUSH_IF
		ଯଦି (rword == RES_IF || rword == RES_ELIF) {
			debug_printf_exec("cond_code=rcode:%d\n", rcode);
			cond_code = rcode;
		}
#endif
#ifndef __U_BOOT__
 check_jobs_and_continue:
		checkjobs(ଖାଲି, 0 /*(no pid to wait for)*/);
#endif /* !__U_BOOT__ */
 dont_check_jobs_but_continue: ;
#ଯଦି ENABLE_HUSH_LOOPS
		/* Beware of "while false; true; do ..."! */
		ଯଦି (pi->next
		 && (pi->next->res_word == RES_DO || pi->next->res_word == RES_DONE)
		 /* check for RES_DONE is needed for "while ...; do \n done" case */
		) {
			ଯଦି (rword == RES_WHILE) {
				ଯଦି (rcode) {
					/* "while false; do...done" - exitcode 0 */
					G.last_exitcode = rcode = EXIT_SUCCESS;
					debug_printf_exec(": while expr is false: breaking (exitcode:EXIT_SUCCESS)\n");
					ଭାଙ୍ଗ;
				}
			}
			ଯଦି (rword == RES_UNTIL) {
				ଯଦି (!rcode) {
					debug_printf_exec(": until expr is true: breaking\n");
					ଭାଙ୍ଗ;
				}
			}
		}
#endif
	} /* for (pi) */

#ଯଦି ENABLE_HUSH_JOB
	G.run_list_level--;
#endif
#ଯଦି ENABLE_HUSH_LOOPS
	ଯଦି (loop_ଶୀର୍ଷ)
		G.depth_of_loop--;
	ମୁକ୍ତ(for_list);
#endif
#ଯଦି ENABLE_HUSH_CASE
	ମୁକ୍ତ(case_word);
#endif
#ifndef __U_BOOT__
	debug_leave();
	debug_printf_exec("run_list lvl %d return %d\n", G.run_list_level, rcode);
#endif /* !__U_BOOT__ */
	ଫେରନ୍ତୁ rcode;
}

/* Select which version we will use */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା run_and_free_list(ସଂରଚନା pipe *pi)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rcode = 0;
	debug_printf_exec("run_and_free_list entered\n");
#ifndef __U_BOOT__
	ଯଦି (!G.o_opt[OPT_O_NOEXEC]) {
#endif /* !__U_BOOT__ */
		debug_printf_exec(": run_list: 1st pipe with %d cmds\n", pi->ଅଙ୍କ_cmds);
		rcode = run_list(pi);
#ifndef __U_BOOT__
	}
#endif /* !__U_BOOT__ */
	/* free_pipe_list has the side effect of clearing memory.
	 * In the long run that function can be merged with run_list,
	 * but doing that now would hobble the debugging effort. */
	free_pipe_list(pi);
	debug_printf_exec("run_and_free_list return %d\n", rcode);
	ଫେରନ୍ତୁ rcode;
}

#ifndef __U_BOOT__
ସ୍ଥିର ଶୂନ୍ୟ install_sighandlers(unsigned mask)
{
	sighandler_t old_handler;
	unsigned sig = 0;
	ଯେତେବେଳେ ((mask >>= 1) != 0) {
		sig++;
		ଯଦି (!(mask & 1))
			ଜାରି;
		old_handler = install_sighandler(sig, pick_sighandler(sig));
		/* POSIX allows shell to re-enable SIGCHLD
		 * even if it was SIG_IGN on entry.
		 * Therefore we skip IGN check for it:
		 */
		ଯଦି (sig == SIGCHLD)
			ଜାରି;
		/* Interactive bash re-enables SIGHUP which is SIG_IGNed on entry.
		 * Try:
		 * trap '' hup; bash; echo RET  # type "kill -hup $$", see SIGHUP having effect
		 * trap '' hup; bash -c 'kill -hup $$; echo ALIVE'  # here SIGHUP is SIG_IGNed
		 */
		ଯଦି (sig == SIGHUP && G_interactive_fd)
			ଜାରି;
		/* Unless one of the above signals, is it SIG_IGN? */
		ଯଦି (old_handler == SIG_IGN) {
			/* oops... restore back to IGN, and record this fact */
			install_sighandler(sig, old_handler);
#ଯଦି ENABLE_HUSH_TRAP
			ଯଦି (!G_traps)
				G_traps = xzalloc(ଆକାର(G_traps[0]) * NSIG);
			ମୁକ୍ତ(G_traps[sig]);
			G_traps[sig] = xzalloc(1); /* == xstrdup(""); */
#endif
		}
	}
}

/* Called a few times only (or even once if "sh -c") */
ସ୍ଥିର ଶୂନ୍ୟ install_special_sighandlers(ଶୂନ୍ୟ)
{
	unsigned mask;

	/* Which signals are shell-special? */
	mask = (1 << SIGQUIT) | (1 << SIGCHLD);
	ଯଦି (G_interactive_fd) {
		mask |= SPECIAL_INTERACTIVE_SIGS;
		ଯଦି (G_saved_tty_pgrp) /* we have ctty, job control sigs work */
			mask |= SPECIAL_JOBSTOP_SIGS;
	}
	/* Careful, do not re-install handlers we already installed */
	ଯଦି (G.special_sig_mask != mask) {
		unsigned diff = mask & ~G.special_sig_mask;
		G.special_sig_mask = mask;
		install_sighandlers(diff);
	}
}

#ଯଦି ENABLE_HUSH_JOB
/* helper */
/* Set handlers to restore tty pgrp and exit */
ସ୍ଥିର ଶୂନ୍ୟ install_fatal_sighandlers(ଶୂନ୍ୟ)
{
	unsigned mask;

	/* We will restore tty pgrp on these signals */
	mask = 0
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
	G_fatal_sig_mask = mask;

	install_sighandlers(mask);
}
#endif

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା set_mode(ପୂର୍ଣ୍ଣସଂଖ୍ୟା state, ଅକ୍ଷର mode, ଅଚଳ ଅକ୍ଷର *o_opt)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା idx;
	ବାଛ (mode) {
	ସ୍ଥିତି 'n':
		/* set -n has no effect in interactive shell */
		/* Try: while set -n; do echo $-; done */
		ଯଦି (!G_interactive_fd)
			G.o_opt[OPT_O_NOEXEC] = state;
		ଭାଙ୍ଗ;
	ସ୍ଥିତି 'x':
		IF_HUSH_MODE_X(G_x_mode = state;)
		IF_HUSH_MODE_X(ଯଦି (G.x_mode_fd <= 0) G.x_mode_fd = dup_CLOEXEC(2, 10);)
		ଭାଙ୍ଗ;
	ସ୍ଥିତି 'e':
		G.o_opt[OPT_O_ERREXIT] = state;
		ଭାଙ୍ଗ;
	ସ୍ଥିତି 'o':
		ଯଦି (!o_opt) {
			/* "set -o" or "set +o" without parameter.
			 * in bash, set -o produces this output:
			 *  pipefail        off
			 * and set +o:
			 *  set +o pipefail
			 * We always use the second form.
			 */
			ଅଚଳ ଅକ୍ଷର *p = o_opt_strings;
			idx = 0;
			ଯେତେବେଳେ (*p) {
				printf("set %co %s\n", (G.o_opt[idx] ? '-' : '+'), p);
				idx++;
				p += strlen(p) + 1;
			}
			ଭାଙ୍ଗ;
		}
		idx = index_in_strings(o_opt_strings, o_opt);
		ଯଦି (idx >= 0) {
			G.o_opt[idx] = state;
			ଭାଙ୍ଗ;
		}
		/* fall through to error */
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		ଫେରନ୍ତୁ EXIT_FAILURE;
	}
	ଫେରନ୍ତୁ EXIT_SUCCESS;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା hush_main(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର **argv) MAIN_EXTERNALLY_VISIBLE;
ପୂର୍ଣ୍ଣସଂଖ୍ୟା hush_main(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର **argv)
{
	pid_t cached_getpid;
	ଗଣନା {
		OPT_login = (1 << 0),
	};
	unsigned flags;
#ଯଦି !BB_MMU
	unsigned builtin_argc = 0;
#endif
	ଅକ୍ଷର **e;
	ସଂରଚନା variable *cur_var;
	ସଂରଚନା variable *shell_ver;

	INIT_G();
	ଯଦି (EXIT_SUCCESS != 0) /* if EXIT_SUCCESS == 0, it is already done */
		G.last_exitcode = EXIT_SUCCESS;
#ଯଦି !BB_MMU
	/* "Big heredoc" support via "sh -< STRING" invocation.
	 * Check it first (do not bother to run the usual init code,
	 * it is not needed for this case).
	 */
	ଯଦି (argv[1]
	 && argv[1][0] == '-' && argv[1][1] == '<' /*&& !argv[1][2]*/
	 /*&& argv[2] && !argv[3] - we don't check some conditions */
	) {
		full_write1_str(argv[2]);
		_exit(0);
	}
	G.argv0_for_re_execing = argv[0];
#endif
#ଯଦି ENABLE_HUSH_TRAP
# ଯଦି ENABLE_HUSH_FUNCTIONS
	G.return_exitcode = -1;
# endif
	G.pre_trap_exitcode = -1;
#endif

#ଯଦି ENABLE_HUSH_FAST
	G.ଗଣନା_SIGCHLD++; /* ensure it is != G.handled_SIGCHLD */
#endif

	cached_getpid = getpid();   /* for tcsetpgrp() during init */
	G.ମୂଳ_pid = cached_getpid; /* for $PID  (NOMMU can override via -$HEXPID:HEXPPID:...) */
	G.ମୂଳ_ppid = getppid();    /* for $PPID (NOMMU can override) */

	/* Deal with HUSH_VERSION */
	debug_printf_env("unsetenv '%s'\n", "HUSH_VERSION");
	unsetenv("HUSH_VERSION"); /* in case it exists in initial env */
	shell_ver = xzalloc(ଆକାର(*shell_ver));
	shell_ver->flg_export = 1;
	shell_ver->flg_read_only = 1;
	/* Code which handles ${var<op>...} needs writable values for all variables,
	 * therefore we xstrdup: */
	shell_ver->varstr = xstrdup(hush_version_str);

	/* Create shell local variables from the values
	 * currently living in the environment */
	G.ଶୀର୍ଷ_var = shell_ver;
	cur_var = G.ଶୀର୍ଷ_var;
	e = environ;
	ଯଦି (e) ଯେତେବେଳେ (*e) {
		ଅକ୍ଷର *ମୂଲ୍ୟ = strchr(*e, '=');
		ଯଦି (ମୂଲ୍ୟ) { /* paranoia */
			cur_var->next = xzalloc(ଆକାର(*cur_var));
			cur_var = cur_var->next;
			cur_var->varstr = *e;
			cur_var->max_len = strlen(*e);
			cur_var->flg_export = 1;
		}
		e++;
	}
	/* (Re)insert HUSH_VERSION into env (AFTER we scanned the env!) */
	debug_printf_env("putenv '%s'\n", shell_ver->varstr);
	putenv(shell_ver->varstr);

	/* Export PWD */
	set_pwd_var(SETFLAG_EXPORT);

#ଯଦି BASH_HOSTNAME_VAR
	/* Set (but not export) HOSTNAME unless already set */
	ଯଦି (!get_local_var_ମୂଲ୍ୟ("HOSTNAME")) {
		ସଂରଚନା utsname uts;
		uname(&uts);
		set_local_var_from_halves("HOSTNAME", uts.nodename);
	}
#endif
	/* IFS is not inherited from the parent environment */
	set_local_var_from_halves("IFS", defifs);

	ଯଦି (!get_local_var_ମୂଲ୍ୟ("PATH"))
		set_local_var_from_halves("PATH", bb_default_ମୂଳ_path);

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

#ଯଦି ଅଙ୍କ_SCRIPTS > 0
	ଯଦି (argc < 0) {
		ଅକ୍ଷର *script = get_script_content(-argc - 1);
		G.global_argv = argv;
		G.global_argc = string_array_len(argv);
		//install_special_sighandlers(); - needed?
		parse_and_run_string(script);
		ଯାଅ final_return;
	}
#endif

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
	ଯେତେବେଳେ (1) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା opt = getopt(argc, argv, "+" /* stop at 1st non-option */
				"cexinsl"
#ଯଦି !BB_MMU
				"$:R:V:"
# ଯଦି ENABLE_HUSH_LINENO_VAR
				"L:"
# endif
# ଯଦି ENABLE_HUSH_FUNCTIONS
				"F:"
# endif
#endif
		);
		ଯଦି (opt <= 0)
			ଭାଙ୍ଗ;
		ବାଛ (opt) {
		ସ୍ଥିତି 'c':
			/* Note: -c is not an option with param!
			 * "hush -c -l SCRIPT" is valid. "hush -cSCRIPT" is not.
			 */
			G.opt_c = 1;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'i':
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
//	HERE
//(under "bash", it's the opposite: it runs $BASH_ENV file only *without* -i).
//
//ash -i -c 'sleep 3; sleep 3', on ^C, drops into a prompt instead of exiting
//(this may be a bug, bash does not do this).
//(ash -i -c 'sleep 3' won't show this, the last command gets auto-"exec"ed)
//
//None of the above feel like useful features people would rely on.
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 's':
			G.opt_s = 1;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି 'l':
			flags |= OPT_login;
			ଭାଙ୍ଗ;
#ଯଦି !BB_MMU
		ସ୍ଥିତି '$': {
			unsigned long long empty_trap_mask;

			G.ମୂଳ_pid = bb_strtou(optarg, &optarg, 16);
			optarg++;
			G.ମୂଳ_ppid = bb_strtou(optarg, &optarg, 16);
			optarg++;
			G.last_bg_pid = bb_strtou(optarg, &optarg, 16);
			optarg++;
			G.last_exitcode = bb_strtou(optarg, &optarg, 16);
			optarg++;
			builtin_argc = bb_strtou(optarg, &optarg, 16);
			optarg++;
			empty_trap_mask = bb_strtoull(optarg, &optarg, 16);
			ଯଦି (empty_trap_mask != 0) {
				IF_HUSH_TRAP(ପୂର୍ଣ୍ଣସଂଖ୍ୟା sig;)
				install_special_sighandlers();
# ଯଦି ENABLE_HUSH_TRAP
				G_traps = xzalloc(ଆକାର(G_traps[0]) * NSIG);
				ପାଇଁ (sig = 1; sig < NSIG; sig++) {
					ଯଦି (empty_trap_mask & (1LL << sig)) {
						G_traps[sig] = xzalloc(1); /* == xstrdup(""); */
						install_sighandler(sig, SIG_IGN);
					}
				}
# endif
			}
# ଯଦି ENABLE_HUSH_LOOPS
			optarg++;
			G.depth_of_loop = bb_strtou(optarg, &optarg, 16);
# endif
			/* Suppress "killed by signal" message, -$ hack is used
			 * for subshells: echo `sh -c 'kill -9 $$'`
			 * should be silent.
			 */
			IF_HUSH_JOB(G.run_list_level = 1;)
# ଯଦି ENABLE_HUSH_FUNCTIONS
			/* nommu uses re-exec trick for "... | func | ...",
			 * should allow "return".
			 * This accidentally allows returns in subshells.
			 */
			G_flag_return_in_progress = -1;
# endif
			ଭାଙ୍ଗ;
		}
		ସ୍ଥିତି 'R':
		ସ୍ଥିତି 'V':
			set_local_var(xstrdup(optarg), opt == 'R' ? SETFLAG_MAKE_RO : 0);
			ଭାଙ୍ଗ;
# ଯଦି ENABLE_HUSH_LINENO_VAR
		ସ୍ଥିତି 'L':
			G.parse_lineno = xatou(optarg);
			ଭାଙ୍ଗ;
# endif
# ଯଦି ENABLE_HUSH_FUNCTIONS
		ସ୍ଥିତି 'F': {
			ସଂରଚନା function *funcp = new_function(optarg);
			/* funcp->name is already set to optarg */
			/* funcp->body is set to NULL. It's a special case. */
			funcp->body_as_string = argv[optind];
			optind++;
			ଭାଙ୍ଗ;
		}
# endif
#endif
		/*case '?': invalid option encountered (set_mode('?') will fail) */
		/*case 'n':*/
		/*case 'x':*/
		/*case 'e':*/
		ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
			ଯଦି (set_mode(1, opt, ଖାଲି) == 0) /* no error */
				ଭାଙ୍ଗ;
			bb_show_usage();
		}
	} /* option parsing loop */

	/* Skip options. Try "hush -l": $1 should not be "-l"! */
	G.global_argc = argc - (optind - 1);
	G.global_argv = argv + (optind - 1);
	G.global_argv[0] = argv[0];

	/* If we are login shell... */
	ଯଦି (flags & OPT_login) {
		ଅଚଳ ଅକ୍ଷର *hp = ଖାଲି;
		HFILE *ନିବେଶ;

		debug_printf("sourcing /etc/profile\n");
		ନିବେଶ = hfopen("/etc/profile");
 run_profile:
		ଯଦି (ନିବେଶ != ଖାଲି) {
			install_special_sighandlers();
			parse_and_run_file(ନିବେଶ);
			hfclose(ନିବେଶ);
		}
		/* bash: after sourcing /etc/profile,
		 * tries to source (in the given order):
		 * ~/.bash_profile, ~/.bash_login, ~/.profile,
		 * stopping on first found. --noprofile turns this off.
		 * bash also sources ~/.bash_logout on exit.
		 * If called as sh, skips .bash_XXX files.
		 */
		ଯଦି (!hp) { /* unless we looped on the "goto" already */
			hp = get_local_var_ମୂଲ୍ୟ("HOME");
			ଯଦି (hp && hp[0]) {
				debug_printf("sourcing ~/.profile\n");
				hp = concat_path_file(hp, ".profile");
				ନିବେଶ = hfopen(hp);
				ମୁକ୍ତ((ଅକ୍ଷର*)hp);
				ଯାଅ run_profile;
			}
		}
	}

#ifndef __U_BOOT__
	/* -c takes effect *after* -l */
	ଯଦି (G.opt_c) {
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
		ଅକ୍ଷର *script;

		install_special_sighandlers();

		G.global_argc--;
		G.global_argv++;
#ଯଦି !BB_MMU
		ଯଦି (builtin_argc) {
			/* -c 'builtin' [BARGV...] "" ARG0 [ARG1...] */
			ଅଚଳ ସଂରଚନା built_in_command *x;
			x = find_builtin(G.global_argv[0]);
			ଯଦି (x) { /* paranoia */
				argv = G.global_argv;
				G.global_argc -= builtin_argc + 1; /* skip [BARGV...] "" */
				G.global_argv += builtin_argc + 1;
				G.global_argv[-1] = ଖାଲି; /* replace "" */
				G.last_exitcode = x->b_function(argv);
			}
			ଯାଅ final_return;
		}
#endif

		script = G.global_argv[0];
		ଯଦି (!script)
			bb_error_ସନ୍ଦେଶ_and_die(bb_ସନ୍ଦେଶ_requires_arg, "-c");
		ଯଦି (!G.global_argv[1]) {
			/* -c 'script' (no params): prevent empty $0 */
			G.global_argv[0] = argv[0];
		} ଅନ୍ୟଥା { /* else -c 'script' ARG0 [ARG1...]: $0 is ARG0 */
			G.global_argc--;
			G.global_argv++;
		}
		parse_and_run_string(script);
		ଯାଅ final_return;
	}

	/* -s is: hush -s ARGV1 ARGV2 (no SCRIPT) */
	ଯଦି (!G.opt_s && G.global_argv[1]) {
		HFILE *ନିବେଶ;
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
		ନିବେଶ = hfopen(G.global_argv[0]);
		ଯଦି (!ନିବେଶ) {
			bb_simple_perror_ସନ୍ଦେଶ_and_die(G.global_argv[0]);
		}
		xfunc_error_retval = 1;
		install_special_sighandlers();
		parse_and_run_file(ନିବେଶ);
#ଯଦି ENABLE_FEATURE_CLEAN_UP
		hfclose(ନିବେଶ);
#endif
		ଯାଅ final_return;
	}
	/* "implicit" -s: bare interactive hush shows 's' in $- */
	G.opt_s = 1;

#endif /* __U_BOOT__ */
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
#ଯଦି ENABLE_HUSH_JOB
	ଯଦି (isatty(STDIN_FILENO) && isatty(STDOUT_FILENO)) {
		G_saved_tty_pgrp = tcgetpgrp(STDIN_FILENO);
		debug_printf("saved_tty_pgrp:%d\n", G_saved_tty_pgrp);
		ଯଦି (G_saved_tty_pgrp < 0)
			G_saved_tty_pgrp = 0;

		/* try to dup stdin to high fd#, >= 255 */
		G_interactive_fd = dup_CLOEXEC(STDIN_FILENO, 254);
		ଯଦି (G_interactive_fd < 0) {
			/* try to dup to any fd */
			G_interactive_fd = dup_CLOEXEC(STDIN_FILENO, -1);
			ଯଦି (G_interactive_fd < 0) {
				/* give up */
				G_interactive_fd = 0;
				G_saved_tty_pgrp = 0;
			}
		}
	}
	debug_printf("interactive_fd:%d\n", G_interactive_fd);
	ଯଦି (G_interactive_fd) {
		ଯଦି (G_saved_tty_pgrp) {
			/* If we were run as 'hush &', sleep until we are
			 * in the foreground (tty pgrp == our pgrp).
			 * If we get started under a job aware app (like bash),
			 * make sure we are now in charge so we don't fight over
			 * who gets the foreground */
			ଯେତେବେଳେ (1) {
				pid_t shell_pgrp = getpgrp();
				G_saved_tty_pgrp = tcgetpgrp(G_interactive_fd);
				ଯଦି (G_saved_tty_pgrp == shell_pgrp)
					ଭାଙ୍ଗ;
				/* send TTIN to ourself (should stop us) */
				kill(- shell_pgrp, SIGTTIN);
			}
		}

		/* Install more signal handlers */
		install_special_sighandlers();

		ଯଦି (G_saved_tty_pgrp) {
			/* Set other signals to restore saved_tty_pgrp */
			install_fatal_sighandlers();
			/* Put ourselves in our own process group
			 * (bash, too, does this only if ctty is available) */
			bb_setpgrp(); /* is the same as setpgid(our_pid, our_pid); */
			/* Grab control of the terminal */
			tcsetpgrp(G_interactive_fd, cached_getpid);
		}
		enable_restore_tty_pgrp_on_exit();

# ଯଦି ENABLE_FEATURE_EDITING
		G.ଧାଡ଼ି_ନିବେଶ_state = new_ଧାଡ଼ି_ନିବେଶ_t(FOR_SHELL);
#  ଯଦି ENABLE_FEATURE_TAB_COMPLETION
		G.ଧାଡ଼ି_ନିବେଶ_state->get_exe_name = hush_command_name;
#  endif
#  ଯଦି EDITING_HAS_sh_get_var
		G.ଧାଡ଼ି_ନିବେଶ_state->sh_get_var = get_local_var_ମୂଲ୍ୟ;
#  endif
# endif
# ଯଦି ENABLE_HUSH_SAVEHISTORY && MAX_HISTORY > 0
		{
			ଅଚଳ ଅକ୍ଷର *hp = get_local_var_ମୂଲ୍ୟ("HISTFILE");
			ଯଦି (!hp) {
				hp = get_local_var_ମୂଲ୍ୟ("HOME");
				ଯଦି (hp)
					hp = concat_path_file(hp, ".hush_history");
			} ଅନ୍ୟଥା {
				hp = xstrdup(hp);
			}
			ଯଦି (hp) {
				G.ଧାଡ଼ି_ନିବେଶ_state->hist_file = hp;
				//set_local_var(xasprintf("HISTFILE=%s", ...));
			}
#  ଯଦି ENABLE_FEATURE_SH_HISTFILESIZE
			hp = get_local_var_ମୂଲ୍ୟ("HISTFILESIZE");
			G.ଧାଡ଼ି_ନିବେଶ_state->max_history = size_from_HISTFILESIZE(hp);
#  endif
		}
# endif
	} ଅନ୍ୟଥା {
		install_special_sighandlers();
	}
#ନଚେତଯଦି ENABLE_HUSH_INTERACTIVE
	/* No job control compiled in, only prompt/line editing */
	ଯଦି (isatty(STDIN_FILENO) && isatty(STDOUT_FILENO)) {
		G_interactive_fd = dup_CLOEXEC(STDIN_FILENO, 254);
		ଯଦି (G_interactive_fd < 0) {
			/* try to dup to any fd */
			G_interactive_fd = dup_CLOEXEC(STDIN_FILENO, -1);
			ଯଦି (G_interactive_fd < 0)
				/* give up */
				G_interactive_fd = 0;
		}
	}
	install_special_sighandlers();
#ଅନ୍ୟଥା
	/* We have interactiveness code disabled */
	install_special_sighandlers();
#endif
	/* bash:
	 * if interactive but not a login shell, sources ~/.bashrc
	 * (--norc turns this off, --rcfile <file> overrides)
	 */

	ଯଦି (G_interactive_fd) {
#ଯଦି ENABLE_HUSH_INTERACTIVE && ENABLE_FEATURE_EDITING_FANCY_PROMPT
		/* Set (but not export) PS1/2 unless already set */
		ଯଦି (!get_local_var_ମୂଲ୍ୟ("PS1"))
			set_local_var_from_halves("PS1", "\\w \\$ ");
		ଯଦି (!get_local_var_ମୂଲ୍ୟ("PS2"))
			set_local_var_from_halves("PS2", "> ");
#endif
		ଯଦି (!ENABLE_FEATURE_SH_EXTRA_QUIET) {
			/* note: ash and hush share this string */
			printf("\n\n%s %s\n"
				IF_HUSH_HELP("Enter 'help' for a list of built-in commands.\n")
				"\n",
				bb_banner,
				"hush - the humble shell"
			);
		}
	}

	parse_and_run_file(hfopen(ଖାଲି)); /* stdin */

 final_return:
	hush_exit(G.last_exitcode);
}

/*
 * Built-ins
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା FAST_FUNC builtin_true(ଅକ୍ଷର **argv UNUSED_PARAM)
{
	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା FAST_FUNC builtin_false(ଅକ୍ଷର **argv UNUSED_PARAM)
{
	ଫେରନ୍ତୁ 1;
}

#ଯଦି ENABLE_HUSH_TEST || ENABLE_HUSH_ECHO || ENABLE_HUSH_PRINTF || ENABLE_HUSH_KILL
ସ୍ଥିର NOINLINE ପୂର୍ଣ୍ଣସଂଖ୍ୟା run_applet_main(ଅକ୍ଷର **argv, ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*applet_main_func)(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର **argv))
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc = string_array_len(argv);
	ଫେରନ୍ତୁ applet_main_func(argc, argv);
}
#endif
#ଯଦି ENABLE_HUSH_TEST || BASH_TEST2
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା FAST_FUNC builtin_test(ଅକ୍ଷର **argv)
{
	ଫେରନ୍ତୁ run_applet_main(argv, test_main);
}
#endif
#ଯଦି ENABLE_HUSH_ECHO
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା FAST_FUNC builtin_echo(ଅକ୍ଷର **argv)
{
	ଫେରନ୍ତୁ run_applet_main(argv, echo_main);
}
#endif
#ଯଦି ENABLE_HUSH_PRINTF
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା FAST_FUNC builtin_printf(ଅକ୍ଷର **argv)
{
	ଫେରନ୍ତୁ run_applet_main(argv, printf_main);
}
#endif

#ଯଦି ENABLE_HUSH_HELP
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା FAST_FUNC builtin_help(ଅକ୍ଷର **argv UNUSED_PARAM)
{
	ଅଚଳ ସଂରଚନା built_in_command *x;

	printf(
		"Built-in commands:\n"
		"------------------\n");
	ପାଇଁ (x = bltins1; x != &bltins1[ARRAY_SIZE(bltins1)]; x++) {
		ଯଦି (x->b_descr)
			printf("%-10s%s\n", x->b_cmd, x->b_descr);
	}
	ଫେରନ୍ତୁ EXIT_SUCCESS;
}
#endif

#ଯଦି MAX_HISTORY && ENABLE_FEATURE_EDITING
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା FAST_FUNC builtin_history(ଅକ୍ଷର **argv UNUSED_PARAM)
{
	show_history(G.ଧାଡ଼ି_ନିବେଶ_state);
	ଫେରନ୍ତୁ EXIT_SUCCESS;
}
#endif

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା FAST_FUNC builtin_cd(ଅକ୍ଷର **argv)
{
	ଅଚଳ ଅକ୍ଷର *newdir;

	argv = skip_dash_dash(argv);
	newdir = argv[0];
	ଯଦି (newdir == ଖାଲି) {
		/* bash does nothing (exitcode 0) if HOME is ""; if it's unset,
		 * bash says "bash: cd: HOME not set" and does nothing
		 * (exitcode 1)
		 */
		ଅଚଳ ଅକ୍ଷର *home = get_local_var_ମୂଲ୍ୟ("HOME");
		newdir = home ? home : "/";
	}
	ଯଦି (chdir(newdir)) {
		/* Mimic bash message exactly */
		bb_perror_ସନ୍ଦେଶ("cd: %s", newdir);
		ଫେରନ୍ତୁ EXIT_FAILURE;
	}
	/* Read current dir (get_cwd(1) is inside) and set PWD.
	 * Note: do not enforce exporting. If PWD was unset or unexported,
	 * set it again, but do not export. bash does the same.
	 */
	set_pwd_var(/*flag:*/ 0);
	ଫେରନ୍ତୁ EXIT_SUCCESS;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା FAST_FUNC builtin_pwd(ଅକ୍ଷର **argv UNUSED_PARAM)
{
	puts(get_cwd(0));
	ଫେରନ୍ତୁ EXIT_SUCCESS;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା FAST_FUNC builtin_eval(ଅକ୍ଷର **argv)
{
	argv = skip_dash_dash(argv);

	ଯଦି (!argv[0])
		ଫେରନ୍ତୁ EXIT_SUCCESS;

	IF_HUSH_MODE_X(G.x_mode_depth++;)
	//bb_error_msg("%s: ++x_mode_depth=%d", __func__, G.x_mode_depth);
	ଯଦି (!argv[1]) {
		/* bash:
		 * eval "echo Hi; done" ("done" is syntax error):
		 * "echo Hi" will not execute too.
		 */
		parse_and_run_string(argv[0]);
	} ଅନ୍ୟଥା {
		/* "The eval utility shall construct a command by
		 * concatenating arguments together, separating
		 * each with a <space> character."
		 */
		ଅକ୍ଷର *str, *p;
		unsigned len = 0;
		ଅକ୍ଷର **pp = argv;
		କର
			len += strlen(*pp) + 1;
		ଯେତେବେଳେ (*++pp);
		str = p = xmalloc(len);
		pp = argv;
		ପାଇଁ (;;) {
			p = stpcpy(p, *pp);
			pp++;
			ଯଦି (!*pp)
				ଭାଙ୍ଗ;
			*p++ = ' ';
		}
		parse_and_run_string(str);
		ମୁକ୍ତ(str);
	}
	IF_HUSH_MODE_X(G.x_mode_depth--;)
	//bb_error_msg("%s: --x_mode_depth=%d", __func__, G.x_mode_depth);
	ଫେରନ୍ତୁ G.last_exitcode;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା FAST_FUNC builtin_exec(ଅକ୍ଷର **argv)
{
	argv = skip_dash_dash(argv);
	ଯଦି (argv[0] == ଖାଲି)
		ଫେରନ୍ତୁ EXIT_SUCCESS; /* bash does this */

	/* Careful: we can end up here after [v]fork. Do not restore
	 * tty pgrp then, only top-level shell process does that */
	ଯଦି (G_saved_tty_pgrp && getpid() == G.ମୂଳ_pid)
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

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା FAST_FUNC builtin_exit(ଅକ୍ଷର **argv)
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
	ଯଦି (argv[0] == ଖାଲି) {
#ଯଦି ENABLE_HUSH_TRAP
		ଯଦି (G.pre_trap_exitcode >= 0) /* "exit" in trap uses $? from before the trap */
			hush_exit(G.pre_trap_exitcode);
#endif
		hush_exit(G.last_exitcode);
	}
	/* mimic bash: exit 123abc == exit 255 + error msg */
	xfunc_error_retval = 255;
	/* bash: exit -2 == exit 254, no error msg */
	hush_exit(xatoi(argv[0]) & 0xff);
}

#ଯଦି ENABLE_HUSH_TYPE
/* http://www.opengroup.org/onlinepubs/9699919799/utilities/type.html */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା FAST_FUNC builtin_type(ଅକ୍ଷର **argv)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = EXIT_SUCCESS;

	ଯେତେବେଳେ (*++argv) {
		ଅଚଳ ଅକ୍ଷର *type;
		ଅକ୍ଷର *path = ଖାଲି;

		ଯଦି (0) {} /* make conditional compile easier below */
		/*else if (find_alias(*argv))
			type = "an alias";*/
# ଯଦି ENABLE_HUSH_FUNCTIONS
		ଅନ୍ୟଥା ଯଦି (find_function(*argv))
			type = "a function";
# endif
		ଅନ୍ୟଥା ଯଦି (find_builtin(*argv))
			type = "a shell builtin";
		ଅନ୍ୟଥା ଯଦି ((path = find_in_path(*argv)) != ଖାଲି)
			type = path;
		ଅନ୍ୟଥା {
			bb_error_ସନ୍ଦେଶ("type: %s: not found", *argv);
			ret = EXIT_FAILURE;
			ଜାରି;
		}

		printf("%s is %s\n", *argv, type);
		ମୁକ୍ତ(path);
	}

	ଫେରନ୍ତୁ ret;
}
#endif

#ଯଦି ENABLE_HUSH_READ
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା FAST_FUNC builtin_read(ଅକ୍ଷର **argv)
{
	ଅଚଳ ଅକ୍ଷର *r;
	ସଂରଚନା builtin_read_params params;

	memset(&params, 0, ଆକାର(params));

	/* "!": do not abort on errors.
	 * Option string must start with "sr" to match BUILTIN_READ_xxx
	 */
	params.read_flags = getopt32(argv,
# ଯଦି BASH_READ_D
		IF_NOT_HUSH_BASH_COMPAT("^")
		"!srn:p:t:u:d:" IF_NOT_HUSH_BASH_COMPAT("\0" "-1"/*min 1 arg*/),
		&params.opt_n, &params.opt_p, &params.opt_t, &params.opt_u, &params.opt_d
# ଅନ୍ୟଥା
		IF_NOT_HUSH_BASH_COMPAT("^")
		"!srn:p:t:u:" IF_NOT_HUSH_BASH_COMPAT("\0" "-1"/*min 1 arg*/),
		&params.opt_n, &params.opt_p, &params.opt_t, &params.opt_u
# endif
//TODO: print "read: need variable name"
//for the case of !BASH "read" with no args (now it fails silently)
//(or maybe extend getopt32() to emit a message if "-1" fails)
	);
	ଯଦି ((uint32_t)params.read_flags == (uint32_t)-1)
		ଫେରନ୍ତୁ EXIT_FAILURE;
	argv += optind;
	params.argv = argv;
	params.setvar = set_local_var_from_halves;
	params.ifs = get_local_var_ମୂଲ୍ୟ("IFS"); /* can be NULL */

 again:
	r = shell_builtin_read(&params);

	ଯଦି ((uintptr_t)r == 1 && errno == EINTR) {
		unsigned sig = check_and_run_traps();
		ଯଦି (sig != SIGINT)
			ଯାଅ again;
	}

	ଯଦି ((uintptr_t)r > 1) {
		bb_simple_error_ସନ୍ଦେଶ(r);
		r = (ଅକ୍ଷର*)(uintptr_t)1;
	}

	ଫେରନ୍ତୁ (uintptr_t)r;
}
#endif

#ଯଦି ENABLE_HUSH_UMASK
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା FAST_FUNC builtin_umask(ଅକ୍ଷର **argv)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
	mode_t mask;

	rc = 1;
	mask = umask(0);
	argv = skip_dash_dash(argv);
	ଯଦି (argv[0]) {
		mode_t old_mask = mask;

		/* numeric umasks are taken as-is */
		/* symbolic umasks are inverted: "umask a=rx" calls umask(222) */
		ଯଦି (!isdigit(argv[0][0]))
			mask ^= 0777;
		mask = bb_parse_mode(argv[0], mask);
		ଯଦି (!isdigit(argv[0][0]))
			mask ^= 0777;
		ଯଦି ((unsigned)mask > 0777) {
			mask = old_mask;
			/* bash messages:
			 * bash: umask: 'q': invalid symbolic mode operator
			 * bash: umask: 999: octal number out of range
			 */
			bb_error_ସନ୍ଦେଶ("%s: invalid mode '%s'", "umask", argv[0]);
			rc = 0;
		}
	} ଅନ୍ୟଥା {
		/* Mimic bash */
		printf("%04o\n", (unsigned) mask);
		/* fall through and restore mask which we set to 0 */
	}
	umask(mask);

	ଫେରନ୍ତୁ !rc; /* rc != 0 - success */
}
#endif

#ଯଦି ENABLE_HUSH_EXPORT || ENABLE_HUSH_READONLY || ENABLE_HUSH_SET || ENABLE_HUSH_TRAP
ସ୍ଥିର ଶୂନ୍ୟ print_escaped(ଅଚଳ ଅକ୍ଷର *s)
{
//TODO? bash "set" does not quote variables which contain only alnums and "%+,-./:=@_~",
// (but "export" quotes all variables, even with only these chars).
// I think quoting strings with %+,=~ looks better
// (example: "set" printing var== instead of var='=' looks strange)
// IOW: do not quote "-./:@_": / is used in pathnames, : in PATH, -._ often in file names, @ in emails

	ଯଦି (*s == '\'')
		ଯାଅ squote;
	କର {
		ଅଚଳ ଅକ୍ଷର *p = strchrnul(s, '\'');
		/* print 'xxxx', possibly just '' */
		printf("'%.*s'", (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(p - s), s);
		ଯଦି (*p == '\0')
			ଭାଙ୍ଗ;
		s = p;
 squote:
		/* s points to '; print "'''...'''" */
		putchar('"');
		କର putchar('\''); ଯେତେବେଳେ (*++s == '\'');
		putchar('"');
	} ଯେତେବେଳେ (*s);
}
#endif

#ଯଦି ENABLE_HUSH_EXPORT || ENABLE_HUSH_LOCAL || ENABLE_HUSH_READONLY
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା helper_export_local(ଅକ୍ଷର **argv, unsigned flags)
{
	କର {
		ଅକ୍ଷର *name = *argv;
		ଅଚଳ ଅକ୍ଷର *name_end = endofname(name);

		ଯଦି (*name_end == '\0') {
			ସଂରଚନା variable *var, **vpp;

			vpp = get_ptr_to_local_var(name);
			var = vpp ? *vpp : ଖାଲି;

			ଯଦି (flags & SETFLAG_UNEXPORT) {
				/* export -n NAME (without =VALUE) */
				ଯଦି (var) {
					var->flg_export = 0;
					debug_printf_env("%s: unsetenv '%s'\n", __func__, name);
					unsetenv(name);
				} /* else: export -n NOT_EXISTING_VAR: no-op */
				ଜାରି;
			}
			ଯଦି (flags & SETFLAG_EXPORT) {
				/* export NAME (without =VALUE) */
				ଯଦି (var) {
					var->flg_export = 1;
					debug_printf_env("%s: putenv '%s'\n", __func__, var->varstr);
					putenv(var->varstr);
					ଜାରି;
				}
			}
			ଯଦି (flags & SETFLAG_MAKE_RO) {
				/* readonly NAME (without =VALUE) */
				ଯଦି (var) {
					var->flg_read_only = 1;
					ଜାରି;
				}
			}
# ଯଦି ENABLE_HUSH_LOCAL
			/* Is this "local" bltin? */
			ଯଦି (!(flags & (SETFLAG_EXPORT|SETFLAG_UNEXPORT|SETFLAG_MAKE_RO))) {
				unsigned lvl = flags >> SETFLAG_VARLVL_SHIFT;
				ଯଦି (var && var->var_nest_level == lvl) {
					/* "local x=abc; ...; local x" - ignore second local decl */
					ଜାରି;
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
		} ଅନ୍ୟଥା {
			ଯଦି (*name_end != '=') {
				bb_error_ସନ୍ଦେଶ("'%s': bad variable name", name);
				/* do not parse following argv[]s: */
				ଫେରନ୍ତୁ 1;
			}
			/* (Un)exporting/making local NAME=VALUE */
			name = xstrdup(name);
			/* Testcase: export PS1='\w \$ ' */
			unbackslash(name);
		}
		debug_printf_env("%s: set_local_var('%s')\n", __func__, name);
		ଯଦି (set_local_var(name, flags))
			ଫେରନ୍ତୁ EXIT_FAILURE;
	} ଯେତେବେଳେ (*++argv);
	ଫେରନ୍ତୁ EXIT_SUCCESS;
}
#endif

#ଯଦି ENABLE_HUSH_EXPORT
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା FAST_FUNC builtin_export(ଅକ୍ଷର **argv)
{
	unsigned opt_unexport;

# ଯଦି ENABLE_HUSH_EXPORT_N
	/* "!": do not abort on errors */
	opt_unexport = getopt32(argv, "!n");
	ଯଦି (opt_unexport == (uint32_t)-1)
		ଫେରନ୍ତୁ EXIT_FAILURE;
	argv += optind;
# ଅନ୍ୟଥା
	opt_unexport = 0;
	argv++;
# endif

	ଯଦି (argv[0] == ଖାଲି) {
		ଅକ୍ଷର **e = environ;
		ଯଦି (e) {
			ଯେତେବେଳେ (*e) {
# ଯଦି 0
				puts(*e++);
# ଅନ୍ୟଥା
				/* ash emits: export VAR='VAL'
				 * bash: declare -x VAR="VAL"
				 * we follow ash example */
				ଅଚଳ ଅକ୍ଷର *s = *e++;
				ଅଚଳ ଅକ୍ଷର *p = strchr(s, '=');

				ଯଦି (!p) /* wtf? take next variable */
					ଜାରି;
				/* "export VAR=" */
				printf("%s %.*s", "export", (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(p - s) + 1, s);
				print_escaped(p + 1);
				putchar('\n');
# endif
			}
			/*fflush_all(); - done after each builtin anyway */
		}
		ଫେରନ୍ତୁ EXIT_SUCCESS;
	}

	ଫେରନ୍ତୁ helper_export_local(argv, opt_unexport ? SETFLAG_UNEXPORT : SETFLAG_EXPORT);
}
#endif

#ଯଦି ENABLE_HUSH_LOCAL
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା FAST_FUNC builtin_local(ଅକ୍ଷର **argv)
{
	ଯଦି (G.func_nest_level == 0) {
		bb_error_ସନ୍ଦେଶ("%s: not in a function", argv[0]);
		ଫେରନ୍ତୁ EXIT_FAILURE; /* bash compat */
	}
//TODO? ash and bash support "local -" special form,
//which saves/restores $- around function call (including async returns, such as ^C)
//(IOW: it makes "set +/-..." effects local)
	argv++;
	/* Since all builtins run in a nested variable level,
	 * need to use level - 1 here. Or else the variable will be removed at once
	 * after builtin returns.
	 */
	ଫେରନ୍ତୁ helper_export_local(argv, (G.var_nest_level - 1) << SETFLAG_VARLVL_SHIFT);
}
#endif

#ଯଦି ENABLE_HUSH_READONLY
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା FAST_FUNC builtin_readonly(ଅକ୍ଷର **argv)
{
	argv++;
	ଯଦି (*argv == ଖାଲି) {
		/* bash: readonly [-p]: list all readonly VARs
		 * (-p has no effect in bash)
		 */
		ସଂରଚନା variable *e;
		ପାଇଁ (e = G.ଶୀର୍ଷ_var; e; e = e->next) {
			ଯଦି (e->flg_read_only) {
				ଅଚଳ ଅକ୍ଷର *s = e->varstr;
				ଅଚଳ ଅକ୍ଷର *p = strchr(s, '=');

				ଯଦି (!p) /* wtf? take next variable */
					ଜାରି;
				/* "readonly VAR=" */
				printf("%s %.*s", "readonly", (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(p - s) + 1, s);
				print_escaped(p + 1);
				putchar('\n');
			}
		}
		ଫେରନ୍ତୁ EXIT_SUCCESS;
	}
	ଫେରନ୍ତୁ helper_export_local(argv, SETFLAG_MAKE_RO);
}
#endif

#ଯଦି ENABLE_HUSH_UNSET
/* http://www.opengroup.org/onlinepubs/9699919799/utilities/V3_chap02.html#unset */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା FAST_FUNC builtin_unset(ଅକ୍ଷର **argv)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;
	unsigned opts;

	/* "!": do not abort on errors */
	/* "+": stop at 1st non-option */
	opts = getopt32(argv, "!+vf");
	ଯଦି (opts == (unsigned)-1)
		ଫେରନ୍ତୁ EXIT_FAILURE;
	ଯଦି (opts == 3) {
		bb_simple_error_ସନ୍ଦେଶ("unset: -v and -f are exclusive");
		ଫେରନ୍ତୁ EXIT_FAILURE;
	}
	argv += optind;

	ret = EXIT_SUCCESS;
	ଯେତେବେଳେ (*argv) {
		ଯଦି (!(opts & 2)) { /* not -f */
			ଯଦି (unset_local_var(*argv)) {
				/* unset <nonexistent_var> doesn't fail.
				 * Error is when one tries to unset RO var.
				 * Message was printed by unset_local_var. */
				ret = EXIT_FAILURE;
			}
		}
# ଯଦି ENABLE_HUSH_FUNCTIONS
		ଅନ୍ୟଥା {
			unset_func(*argv);
		}
# endif
		argv++;
	}
	ଫେରନ୍ତୁ ret;
}
#endif

#ଯଦି ENABLE_HUSH_SET
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା FAST_FUNC builtin_set(ଅକ୍ଷର **argv)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା n;
	ଅକ୍ଷର **pp, **g_argv;
	ଅକ୍ଷର *arg = *++argv;

	ଯଦି (arg == ଖାଲି) {
		ସଂରଚନା variable *e;
		ପାଇଁ (e = G.ଶୀର୍ଷ_var; e; e = e->next) {
			ଅଚଳ ଅକ୍ଷର *s = e->varstr;
			ଅଚଳ ଅକ୍ଷର *p = strchr(s, '=');

			ଯଦି (!p) /* wtf? take next variable */
				ଜାରି;
			/* var= */
			printf("%.*s", (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(p - s) + 1, s);
			print_escaped(p + 1);
			putchar('\n');
		}
		ଫେରନ୍ତୁ EXIT_SUCCESS;
	}

	କର {
		ଯଦି (strcmp(arg, "--") == 0) {
			++argv;
			ଯାଅ set_argv;
		}
		ଯଦି (arg[0] != '+' && arg[0] != '-')
			ଭାଙ୍ଗ;
		ପାଇଁ (n = 1; arg[n]; ++n) {
			ଯଦି (set_mode((arg[0] == '-'), arg[n], argv[1])) {
				bb_error_ସନ୍ଦେଶ("%s: %s: invalid option", "set", arg);
				ଫେରନ୍ତୁ EXIT_FAILURE;
			}
			ଯଦି (arg[n] == 'o' && argv[1])
				argv++;
		}
	} ଯେତେବେଳେ ((arg = *++argv) != ଖାଲି);
	/* Now argv[0] is 1st argument */

	ଯଦି (arg == ଖାଲି)
		ଫେରନ୍ତୁ EXIT_SUCCESS;
 set_argv:

	/* NB: G.global_argv[0] ($0) is never freed/changed */
	g_argv = G.global_argv;
	ଯଦି (G.global_args_malloced) {
		pp = g_argv;
		ଯେତେବେଳେ (*++pp)
			ମୁକ୍ତ(*pp);
		g_argv[1] = ଖାଲି;
	} ଅନ୍ୟଥା {
		G.global_args_malloced = 1;
		pp = xzalloc(ଆକାର(pp[0]) * 2);
		pp[0] = g_argv[0]; /* retain $0 */
		g_argv = pp;
	}
	/* This realloc's G.global_argv */
	G.global_argv = pp = add_strings_to_strings(g_argv, argv, /*dup:*/ 1);

	G.global_argc = 1 + string_array_len(pp + 1);

	ଫେରନ୍ତୁ EXIT_SUCCESS;
}
#endif

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା FAST_FUNC builtin_shift(ଅକ୍ଷର **argv)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = 1;
	argv = skip_dash_dash(argv);
	ଯଦି (argv[0]) {
		n = bb_strtou(argv[0], ଖାଲି, 10);
		ଯଦି (errno || n < 0) {
			/* shared string with ash.c */
			bb_error_ସନ୍ଦେଶ("Illegal number: %s", argv[0]);
			/*
			 * ash aborts in this case.
			 * bash prints error message and set $? to 1.
			 * Interestingly, for "shift 99999" bash does not
			 * print error message, but does set $? to 1
			 * (and does no shifting at all).
			 */
		}
	}
	ଯଦି (n >= 0 && n < G.global_argc) {
		ଯଦି (G_global_args_malloced) {
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା m = 1;
			ଯେତେବେଳେ (m <= n)
				ମୁକ୍ତ(G.global_argv[m++]);
		}
		G.global_argc -= n;
		memmove(&G.global_argv[1], &G.global_argv[n+1],
				G.global_argc * ଆକାର(G.global_argv[0]));
		ଫେରନ୍ତୁ EXIT_SUCCESS;
	}
	ଫେରନ୍ତୁ EXIT_FAILURE;
}

#ଯଦି ENABLE_HUSH_GETOPTS
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା FAST_FUNC builtin_getopts(ଅକ୍ଷର **argv)
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
	ଅକ୍ଷର cbuf[2];
	ଅଚଳ ଅକ୍ଷର *cp, *optstring, *var;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା c, n, exitcode, my_opterr;
	unsigned ଗଣନା_2;

	optstring = *++argv;
	ଯଦି (!optstring || !(var = *++argv)) {
		bb_simple_error_ସନ୍ଦେଶ("usage: getopts OPTSTRING VAR [ARGS]");
		ଫେରନ୍ତୁ EXIT_FAILURE;
	}

	ଯଦି (argv[1])
		argv[0] = G.global_argv[0]; /* for error messages in getopt() */
	ଅନ୍ୟଥା
		argv = G.global_argv;
	cbuf[1] = '\0';

	my_opterr = 0;
	ଯଦି (optstring[0] != ':') {
		cp = get_local_var_ମୂଲ୍ୟ("OPTERR");
		/* 0 if "OPTERR=0", 1 otherwise */
		my_opterr = (!cp || NOT_LONE_CHAR(cp, '0'));
	}

	/* getopts stops on first non-option. Add "+" to force that */
	/*if (optstring[0] != '+')*/ {
		ଅକ୍ଷର *s = alloca(strlen(optstring) + 2);
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
	ଗଣନା_2 = 0;
	n = string_array_len(argv);
	କର {
		optarg = ଖାଲି;
		opterr = (ଗଣନା_2 < G.getopt_ଗଣନା) ? 0 : my_opterr;
		c = getopt(n, argv, optstring);
		ଯଦି (c < 0)
			ଭାଙ୍ଗ;
		ଗଣନା_2++;
	} ଯେତେବେଳେ (ଗଣନା_2 <= G.getopt_ଗଣନା);

	/* Set OPTIND. Prevent resetting of the magic counter! */
	set_local_var_from_halves("OPTIND", utoa(optind));
	G.getopt_ଗଣନା = ଗଣନା_2; /* "next time, give me N+1'th result" */
	GETOPT_RESET(); /* just in case */

	/* Set OPTARG */
	/* Always set or unset, never left as-is, even on exit/error:
	 * "If no option was found, or if the option that was found
	 * does not have an option-argument, OPTARG shall be unset."
	 */
	cp = optarg;
	ଯଦି (c == '?') {
		/* If ":optstring" and unknown option is seen,
		 * it is stored to OPTARG.
		 */
		ଯଦି (optstring[1] == ':') {
			cbuf[0] = optopt;
			cp = cbuf;
		}
	}
	ଯଦି (cp)
		set_local_var_from_halves("OPTARG", cp);
	ଅନ୍ୟଥା
		unset_local_var("OPTARG");

	/* Convert -1 to "?" */
	exitcode = EXIT_SUCCESS;
	ଯଦି (c < 0) { /* -1: end of options */
		exitcode = EXIT_FAILURE;
		c = '?';
	}

	/* Set VAR */
	cbuf[0] = c;
	set_local_var_from_halves(var, cbuf);

	ଫେରନ୍ତୁ exitcode;
}
#endif

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା FAST_FUNC builtin_source(ଅକ୍ଷର **argv)
{
	ଅକ୍ଷର *arg_path, *filename;
	HFILE *ନିବେଶ;
	save_arg_t sv;
	ଅକ୍ଷର *args_need_save;
#ଯଦି ENABLE_HUSH_FUNCTIONS
	smallint sv_flg;
#endif

	argv = skip_dash_dash(argv);
	filename = argv[0];
	ଯଦି (!filename) {
		/* bash says: "bash: .: filename argument required" */
		ଫେରନ୍ତୁ 2; /* bash compat */
	}
	arg_path = ଖାଲି;
	ଯଦି (!strchr(filename, '/')) {
		arg_path = find_in_path(filename);
		ଯଦି (arg_path)
			filename = arg_path;
		ଅନ୍ୟଥା ଯଦି (!ENABLE_HUSH_BASH_SOURCE_CURDIR) {
			errno = ENOENT;
			bb_simple_perror_ସନ୍ଦେଶ(filename);
			ଫେରନ୍ତୁ EXIT_FAILURE;
		}
	}
	ନିବେଶ = hfopen(filename);
	ମୁକ୍ତ(arg_path);
	ଯଦି (!ନିବେଶ) {
		bb_perror_ସନ୍ଦେଶ("%s", filename);
		/* POSIX: non-interactive shell should abort here,
		 * not merely fail. So far no one complained :)
		 */
		ଫେରନ୍ତୁ EXIT_FAILURE;
	}

#ଯଦି ENABLE_HUSH_FUNCTIONS
	sv_flg = G_flag_return_in_progress;
	/* "we are inside sourced file, ok to use return" */
	G_flag_return_in_progress = -1;
#endif
	args_need_save = argv[1]; /* used as a boolean variable */
	ଯଦି (args_need_save)
		save_and_replace_G_args(&sv, argv);

	/* "false; . ./empty_line; echo Zero:$?" should print 0 */
	G.last_exitcode = 0;
	parse_and_run_file(ନିବେଶ);
	hfclose(ନିବେଶ);

	ଯଦି (args_need_save) /* can't use argv[1] instead: "shift" can mangle it */
		restore_G_args(&sv, argv);
#ଯଦି ENABLE_HUSH_FUNCTIONS
	G_flag_return_in_progress = sv_flg;
#endif

	ଫେରନ୍ତୁ G.last_exitcode;
}

#ଯଦି ENABLE_HUSH_TRAP
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା FAST_FUNC builtin_trap(ଅକ୍ଷର **argv)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା sig;
	ଅକ୍ଷର *new_cmd;

	ଯଦି (!G_traps)
		G_traps = xzalloc(ଆକାର(G_traps[0]) * NSIG);

	argv++;
	ଯଦି (!*argv) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
		/* No args: print all trapped */
		ପାଇଁ (i = 0; i < NSIG; ++i) {
			ଯଦି (G_traps[i]) {
				printf("trap -- ");
				print_escaped(G_traps[i]);
				/* note: bash adds "SIG", but only if invoked
				 * as "bash". If called as "sh", or if set -o posix,
				 * then it prints short signal names.
				 * We are printing short names: */
				printf(" %s\n", get_signame(i));
			}
		}
		/*fflush_all(); - done after each builtin anyway */
		ଫେରନ୍ତୁ EXIT_SUCCESS;
	}

	new_cmd = ଖାଲି;
	/* If first arg is a number: reset all specified signals */
	sig = bb_strtou(*argv, ଖାଲି, 10);
	ଯଦି (errno == 0) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;
 process_sig_list:
		ret = EXIT_SUCCESS;
		ଯେତେବେଳେ (*argv) {
			sighandler_t handler;

			sig = get_signum(*argv++);
			ଯଦି (sig < 0) {
				ret = EXIT_FAILURE;
				/* Mimic bash message exactly */
				bb_error_ସନ୍ଦେଶ("trap: %s: invalid signal specification", argv[-1]);
				ଜାରି;
			}

			ମୁକ୍ତ(G_traps[sig]);
			G_traps[sig] = xstrdup(new_cmd);

			debug_printf("trap: setting SIG%s (%i) to '%s'\n",
				get_signame(sig), sig, G_traps[sig]);

			/* There is no signal for 0 (EXIT) */
			ଯଦି (sig == 0)
				ଜାରି;

			ଯଦି (new_cmd)
				handler = (new_cmd[0] ? record_pending_signo : SIG_IGN);
			ଅନ୍ୟଥା
				/* We are removing trap handler */
				handler = pick_sighandler(sig);
			install_sighandler(sig, handler);
		}
		ଫେରନ୍ତୁ ret;
	}

	ଯଦି (!argv[1]) { /* no second arg */
		bb_simple_error_ସନ୍ଦେଶ("trap: invalid arguments");
		ଫେରନ୍ତୁ EXIT_FAILURE;
	}

	/* First arg is "-": reset all specified to default */
	/* First arg is "--": skip it, the rest is "handler SIGs..." */
	/* Everything else: set arg as signal handler
	 * (includes "" case, which ignores signal) */
	ଯଦି (argv[0][0] == '-') {
		ଯଦି (argv[0][1] == '\0') { /* "-" */
			/* new_cmd remains NULL: "reset these sigs" */
			ଯାଅ reset_traps;
		}
		ଯଦି (argv[0][1] == '-' && argv[0][2] == '\0') { /* "--" */
			argv++;
		}
		/* else: "-something", no special meaning */
	}
	new_cmd = *argv;
 reset_traps:
	argv++;
	ଯାଅ process_sig_list;
}
#endif

#ଯଦି ENABLE_HUSH_JOB
ସ୍ଥିର ସଂରଚନା pipe *parse_jobspec(ଅଚଳ ଅକ୍ଷର *str)
{
	ସଂରଚନା pipe *pi;
	unsigned jobnum;

	ଯଦି (sscanf(str, "%%%u", &jobnum) != 1) {
		ଯଦି (str[0] != '%'
		 || (str[1] != '%' && str[1] != '+' && str[1] != '\0')
		) {
			bb_error_ସନ୍ଦେଶ("bad argument '%s'", str);
			ଫେରନ୍ତୁ ଖାଲି;
		}
		/* It is "%%", "%+" or "%" - current job */
		jobnum = G.last_jobid;
		ଯଦି (jobnum == 0) {
			bb_simple_error_ସନ୍ଦେଶ("no current job");
			ଫେରନ୍ତୁ ଖାଲି;
		}
	}
	ପାଇଁ (pi = G.job_list; pi; pi = pi->next) {
		ଯଦି (pi->jobid == jobnum) {
			ଫେରନ୍ତୁ pi;
		}
	}
	bb_error_ସନ୍ଦେଶ("%u: no such job", jobnum);
	ଫେରନ୍ତୁ ଖାଲି;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା FAST_FUNC builtin_jobs(ଅକ୍ଷର **argv UNUSED_PARAM)
{
	ସଂରଚନା pipe *job;
	ଅଚଳ ଅକ୍ଷର *status_string;

	checkjobs(ଖାଲି, 0 /*(no pid to wait for)*/);
	ପାଇଁ (job = G.job_list; job; job = job->next) {
		ଯଦି (job->alive_cmds == job->stopped_cmds)
			status_string = "Stopped";
		ଅନ୍ୟଥା
			status_string = "Running";

		printf(JOB_STATUS_FORMAT, job->jobid, status_string, job->cmdtext);
	}

	clean_up_last_dead_job();

	ଫେରନ୍ତୁ EXIT_SUCCESS;
}

/* built-in 'fg' and 'bg' handler */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା FAST_FUNC builtin_fg_bg(ଅକ୍ଷର **argv)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	ସଂରଚନା pipe *pi;

	ଯଦି (!G_interactive_fd)
		ଫେରନ୍ତୁ EXIT_FAILURE;

	/* If they gave us no args, assume they want the last backgrounded task */
	ଯଦି (!argv[1]) {
		ପାଇଁ (pi = G.job_list; pi; pi = pi->next) {
			ଯଦି (pi->jobid == G.last_jobid) {
				ଯାଅ found;
			}
		}
		bb_error_ସନ୍ଦେଶ("%s: no current job", argv[0]);
		ଫେରନ୍ତୁ EXIT_FAILURE;
	}

	pi = parse_jobspec(argv[1]);
	ଯଦି (!pi)
		ଫେରନ୍ତୁ EXIT_FAILURE;
 found:
	/* TODO: bash prints a string representation
	 * of job being foregrounded (like "sleep 1 | cat") */
	ଯଦି (argv[0][0] == 'f' && G_saved_tty_pgrp) {
		/* Put the job into the foreground. */
		tcsetpgrp(G_interactive_fd, pi->pgrp);
	}

	/* Restart the processes in the job */
	debug_printf_jobs("reviving %d procs, pgrp %d\n", pi->ଅଙ୍କ_cmds, pi->pgrp);
	ପାଇଁ (i = 0; i < pi->ଅଙ୍କ_cmds; i++) {
		debug_printf_jobs("reviving pid %d\n", pi->cmds[i].pid);
	}
	pi->stopped_cmds = 0;

	i = kill(- pi->pgrp, SIGCONT);
	ଯଦି (i < 0) {
		ଯଦି (errno == ESRCH) {
			delete_finished_job(pi);
			ଫେରନ୍ତୁ EXIT_SUCCESS;
		}
		bb_simple_perror_ସନ୍ଦେଶ("kill (SIGCONT)");
	}

	ଯଦି (argv[0][0] == 'f') {
		remove_job_from_table(pi); /* FG job shouldn't be in job table */
		ଫେରନ୍ତୁ checkjobs_and_fg_shell(pi);
	}
	ଫେରନ୍ତୁ EXIT_SUCCESS;
}
#endif

#ଯଦି ENABLE_HUSH_KILL
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା FAST_FUNC builtin_kill(ଅକ୍ଷର **argv)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = 0;

# ଯଦି ENABLE_HUSH_JOB
	ଯଦି (argv[1] && strcmp(argv[1], "-l") != 0) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା i = 1;

		କର {
			ସଂରଚନା pipe *pi;
			ଅକ୍ଷର *dst;
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା j, n;

			ଯଦି (argv[i][0] != '%')
				ଜାରି;
			/*
			 * "kill %N" - job kill
			 * Converting to pgrp / pid kill
			 */
			pi = parse_jobspec(argv[i]);
			ଯଦି (!pi) {
				/* Eat bad jobspec */
				j = i;
				କର {
					j++;
					argv[j - 1] = argv[j];
				} ଯେତେବେଳେ (argv[j]);
				ret = 1;
				i--;
				ଜାରି;
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
			n = G_interactive_fd ? 1 : pi->ଅଙ୍କ_cmds;
			dst = alloca(n * ଆକାର(ପୂର୍ଣ୍ଣସଂଖ୍ୟା)*4);
			argv[i] = dst;
			ଯଦି (G_interactive_fd)
				dst += sprintf(dst, " -%u", (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)pi->pgrp);
			ଅନ୍ୟଥା ପାଇଁ (j = 0; j < n; j++) {
				ସଂରଚନା command *cmd = &pi->cmds[j];
				/* Skip exited members of the job */
				ଯଦି (cmd->pid == 0)
					ଜାରି;
				/*
				 * kill_main has matching code to expect
				 * leading space. Needed to not confuse
				 * negative pids with "kill -SIGNAL_NO" syntax
				 */
				dst += sprintf(dst, " %u", (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)cmd->pid);
			}
			*dst = '\0';
		} ଯେତେବେଳେ (argv[++i]);
	}
# endif

	ଯଦି (argv[1] || ret == 0) {
		ret = run_applet_main(argv, kill_main);
	}
	/* else: ret = 1, "kill %bad_jobspec" case */
	ଫେରନ୍ତୁ ret;
}
#endif

#ଯଦି ENABLE_HUSH_WAIT
/* http://www.opengroup.org/onlinepubs/9699919799/utilities/wait.html */
# ଯଦି !ENABLE_HUSH_JOB
#  define wait_for_child_or_signal(pipe,pid) wait_for_child_or_signal(pid)
# endif
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା wait_for_child_or_signal(ସଂରଚନା pipe *waitfor_pipe, pid_t waitfor_pid)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = 0;
	ପାଇଁ (;;) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା sig;
		sigset_t oldset;

		ଯଦି (!sigisemptyset(&G.pending_set))
			ଯାଅ check_sig;

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

		ଯଦି (!sigisemptyset(&G.pending_set)) {
			/* Crap! we raced with some signal! */
			ଯାଅ restore;
		}

		/*errno = 0; - checkjobs does this */
/* Can't pass waitfor_pipe into checkjobs(): it won't be interruptible */
		ret = checkjobs(ଖାଲି, waitfor_pid); /* waitpid(WNOHANG) inside */
		debug_printf_exec("checkjobs:%d\n", ret);
# ଯଦି ENABLE_HUSH_JOB
		ଯଦି (waitfor_pipe) {
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା rcode = job_exited_or_stopped(waitfor_pipe);
			debug_printf_exec("job_exited_or_stopped:%d\n", rcode);
			ଯଦି (rcode >= 0) {
				ret = rcode;
				sigprocmask(SIG_SETMASK, &oldset, ଖାଲି);
				ଭାଙ୍ଗ;
			}
		}
# endif
		/* if ECHILD, there are no children (ret is -1 or 0) */
		/* if ret == 0, no children changed state */
		/* if ret != 0, it's exitcode+1 of exited waitfor_pid child */
		ଯଦି (errno == ECHILD || ret) {
			ret--;
			ଯଦି (ret < 0) /* if ECHILD, may need to fix "ret" */
				ret = 0;
# ଯଦି ENABLE_HUSH_BASH_COMPAT
			ଯଦି (waitfor_pid == -1 && errno == ECHILD) {
				/* exitcode of "wait -n" with no children is 127, not 0 */
				ret = 127;
			}
# endif
			sigprocmask(SIG_SETMASK, &oldset, ଖାଲି);
			ଭାଙ୍ଗ;
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
		sigprocmask(SIG_SETMASK, &oldset, ଖାଲି);
 check_sig:
		/* So, did we get a signal? */
		sig = check_and_run_traps();
		ଯଦି (sig /*&& sig != SIGCHLD - always true */) {
			/* Do this for any (non-ignored) signal, not only for ^C */
			ret = 128 | sig;
			ଭାଙ୍ଗ;
		}
		/* SIGCHLD, or no signal, or ignored one, such as SIGQUIT. Repeat */
	}
	ଫେରନ୍ତୁ ret;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା FAST_FUNC builtin_wait(ଅକ୍ଷର **argv)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା status;

	argv = skip_dash_dash(argv);
# ଯଦି ENABLE_HUSH_BASH_COMPAT
	ଯଦି (argv[0] && strcmp(argv[0], "-n") == 0) {
		/* wait -n */
		/* (bash accepts "wait -n PID" too and ignores PID) */
		G.dead_job_exitcode = -1;
		ଫେରନ୍ତୁ wait_for_child_or_signal(ଖାଲି, -1 /*no job, wait for one job*/);
	}
# endif
	ଯଦି (argv[0] == ଖାଲି) {
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
		ଫେରନ୍ତୁ wait_for_child_or_signal(ଖାଲି, 0 /*no job and no pid to wait for*/);
	}

	କର {
		pid_t pid = bb_strtou(*argv, ଖାଲି, 10);
		ଯଦି (errno || pid <= 0) {
# ଯଦି ENABLE_HUSH_JOB
			ଯଦି (argv[0][0] == '%') {
				ସଂରଚନା pipe *wait_pipe;
				ret = 127; /* bash compat for bad jobspecs */
				wait_pipe = parse_jobspec(*argv);
				ଯଦି (wait_pipe) {
					ret = job_exited_or_stopped(wait_pipe);
					ଯଦି (ret < 0) {
						ret = wait_for_child_or_signal(wait_pipe, 0);
					} ଅନ୍ୟଥା {
						/* waiting on "last dead job" removes it */
						clean_up_last_dead_job();
					}
				}
				/* else: parse_jobspec() already emitted error msg */
				ଜାରି;
			}
# endif
			/* mimic bash message */
			bb_error_ସନ୍ଦେଶ("wait: '%s': not a pid or valid job spec", *argv);
			ret = EXIT_FAILURE;
			ଜାରି; /* bash checks all argv[] */
		}

		/* Do we have such child? */
		ret = waitpid(pid, &status, WNOHANG);
		ଯଦି (ret < 0) {
			/* No */
			ret = 127;
			ଯଦି (errno == ECHILD) {
				ଯଦି (pid == G.last_bg_pid) {
					/* "wait $!" but last bg task has already exited. Try:
					 * (sleep 1; exit 3) & sleep 2; echo $?; wait $!; echo $?
					 * In bash it prints exitcode 0, then 3.
					 * In dash, it is 127.
					 */
					ret = G.last_bg_pid_exitcode;
				} ଅନ୍ୟଥା {
					/* Example: "wait 1". mimic bash message */
					bb_error_ସନ୍ଦେଶ("wait: pid %u is not a child of this shell", (unsigned)pid);
				}
			} ଅନ୍ୟଥା {
				/* ??? */
				bb_perror_ସନ୍ଦେଶ("wait %s", *argv);
			}
			ଜାରି; /* bash checks all argv[] */
		}
		ଯଦି (ret == 0) {
			/* Yes, and it still runs */
			ret = wait_for_child_or_signal(ଖାଲି, pid);
		} ଅନ୍ୟଥା {
			/* Yes, and it just exited */
			process_wait_result(ଖାଲି, pid, status);
			ret = WEXITSTATUS(status);
			ଯଦି (WIFSIGNALED(status))
				ret = 128 | WTERMSIG(status);
		}
	} ଯେତେବେଳେ (*++argv);

	ଫେରନ୍ତୁ ret;
}
#endif

#ଯଦି ENABLE_HUSH_LOOPS || ENABLE_HUSH_FUNCTIONS
ସ୍ଥିର unsigned parse_numeric_argv1(ଅକ୍ଷର **argv, unsigned def, unsigned def_min)
{
	ଯଦି (argv[1]) {
		def = bb_strtou(argv[1], ଖାଲି, 10);
		ଯଦି (errno || def < def_min || argv[2]) {
			bb_error_ସନ୍ଦେଶ("%s: bad arguments", argv[0]);
			def = UINT_MAX;
		}
	}
	ଫେରନ୍ତୁ def;
}
#endif

#ଯଦି ENABLE_HUSH_LOOPS
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା FAST_FUNC builtin_break(ଅକ୍ଷର **argv)
{
	unsigned depth;
	ଯଦି (G.depth_of_loop == 0) {
		bb_error_ସନ୍ଦେଶ("%s: only meaningful in a loop", argv[0]);
		/* if we came from builtin_continue(), need to undo "= 1" */
		G.flag_break_continue = 0;
		ଫେରନ୍ତୁ EXIT_SUCCESS; /* bash compat */
	}
	G.flag_break_continue++; /* BC_BREAK = 1, or BC_CONTINUE = 2 */

	G.depth_break_continue = depth = parse_numeric_argv1(argv, 1, 1);
	ଯଦି (depth == UINT_MAX)
		G.flag_break_continue = BC_BREAK;
	ଯଦି (G.depth_of_loop < depth)
		G.depth_break_continue = G.depth_of_loop;

	ଫେରନ୍ତୁ EXIT_SUCCESS;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା FAST_FUNC builtin_continue(ଅକ୍ଷର **argv)
{
	G.flag_break_continue = 1; /* BC_CONTINUE = 2 = 1+1 */
	ଫେରନ୍ତୁ builtin_break(argv);
}
#endif

#ଯଦି ENABLE_HUSH_FUNCTIONS
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା FAST_FUNC builtin_return(ଅକ୍ଷର **argv)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;

	ଯଦି (G_flag_return_in_progress != -1) {
		bb_error_ସନ୍ଦେଶ("%s: not in a function or sourced script", argv[0]);
		ଫେରନ୍ତୁ EXIT_FAILURE; /* bash compat */
	}

	G_flag_return_in_progress = 1;

	/* bash:
	 * out of range: wraps around at 256, does not error out
	 * non-numeric param:
	 * f() { false; return qwe; }; f; echo $?
	 * bash: return: qwe: numeric argument required  <== we do this
	 * 255  <== we also do this
	 */
	rc = parse_numeric_argv1(argv, G.last_exitcode, 0);
# ଯଦି ENABLE_HUSH_TRAP
	ଯଦି (argv[1]) { /* "return ARG" inside a running trap sets $? */
		debug_printf_exec("G.return_exitcode=%d\n", rc);
		G.return_exitcode = rc;
	}
# endif
	ଫେରନ୍ତୁ rc;
}
#endif

#ଯଦି ENABLE_HUSH_TIMES
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା FAST_FUNC builtin_times(ଅକ୍ଷର **argv UNUSED_PARAM)
{
	ସ୍ଥିର ଅଚଳ uint8_t times_tbl[] ALIGN1 = {
		' ',  offsetof(ସଂରଚନା tms, tms_utime),
		'\n', offsetof(ସଂରଚନା tms, tms_stime),
		' ',  offsetof(ସଂରଚନା tms, tms_cutime),
		'\n', offsetof(ସଂରଚନା tms, tms_cstime),
		0
	};
	ଅଚଳ uint8_t *p;
	unsigned clk_tck;
	ସଂରଚନା tms buf;

	clk_tck = bb_clk_tck();

	times(&buf);
	p = times_tbl;
	କର {
		unsigned sec, frac;
		unsigned long t;
		t = *(clock_t *)(((ଅକ୍ଷର *) &buf) + p[1]);
		sec = t / clk_tck;
		frac = t % clk_tck;
		printf("%um%u.%03us%c",
			sec / 60, sec % 60,
			(frac * 1000) / clk_tck,
			p[0]);
		p += 2;
	} ଯେତେବେଳେ (*p);

	ଫେରନ୍ତୁ EXIT_SUCCESS;
}
#endif

#ଯଦି ENABLE_HUSH_MEMLEAK
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା FAST_FUNC builtin_memleak(ଅକ୍ଷର **argv UNUSED_PARAM)
{
	ଶୂନ୍ୟ *p;
	unsigned long l;

# ifdef M_TRIM_THRESHOLD
	/* Optional. Reduces probability of false positives */
	malloc_trim(0);
# endif
	/* Crude attempt to find where "free memory" starts,
	 * sans fragmentation. */
	p = malloc(240);
	l = (unsigned long)p;
	ମୁକ୍ତ(p);
	p = malloc(3400);
	ଯଦି (l < (unsigned long)p) l = (unsigned long)p;
	ମୁକ୍ତ(p);

# ଯଦି 0  /* debug */
	{
		ସଂରଚନା mallinfo mi = mallinfo();
		printf("top alloc:0x%lx malloced:%d+%d=%d\n", l,
			mi.arena, mi.hblkhd, mi.arena + mi.hblkhd);
	}
# endif

	ଯଦି (!G.memleak_ମୂଲ୍ୟ)
		G.memleak_ମୂଲ୍ୟ = l;

	l -= G.memleak_ମୂଲ୍ୟ;
	ଯଦି ((long)l < 0)
		l = 0;
	l /= 1024;
	ଯଦି (l > 127)
		l = 127;

	/* Exitcode is "how many kilobytes we leaked since 1st call" */
	ଫେରନ୍ତୁ l;
}
#endif
#endif /* !__U_BOOT__ */
