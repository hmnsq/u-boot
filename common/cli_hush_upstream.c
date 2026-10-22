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
#എങ്കിൽ !(defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) \
	|| defined(__APPLE__) \
    )
# include <malloc.h>   /* for malloc_trim */
#endif
#അന്തര്ഭാവ <glob.h>
/* #include <dmalloc.h> */
#എങ്കിൽ ENABLE_HUSH_CASE
# include <fnmatch.h>
#endif
#അന്തര്ഭാവ <sys/times.h>
#അന്തര്ഭാവ <sys/utsname.h> /* for setting $HOSTNAME */

#അന്തര്ഭാവ "busybox.h"  /* for APPLET_IS_NOFORK/NOEXEC */
#അന്തര്ഭാവ "unicode.h"
#അന്തര്ഭാവ "shell_common.h"
#അന്തര്ഭാവ "math.h"
#അന്തര്ഭാവ "match.h"
#എങ്കിൽ ENABLE_HUSH_RANDOM_SUPPORT
# include "random.h"
#അല്ലെങ്കിൽ
# define CLEAR_RANDOM_T(rnd) ((ശൂന്യം)0)
#endif
#ifndef O_CLOEXEC
# define O_CLOEXEC 0
#endif
#ifndef F_DUPFD_CLOEXEC
# define F_DUPFD_CLOEXEC F_DUPFD
#endif

#എങ്കിൽ ENABLE_FEATURE_SH_EMBEDDED_SCRIPTS && !ENABLE_SHELL_ASH
# include "embedded_scripts.h"
#അല്ലെങ്കിൽ
# define അക്കം_SCRIPTS 0
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

#എങ്കിൽ BUILD_AS_NOMMU
# undef BB_MMU
# undef USE_FOR_NOMMU
# undef USE_FOR_MMU
# define BB_MMU 0
# define USE_FOR_NOMMU(...) __VA_ARGS__
# define USE_FOR_MMU(...)
#endif

#ifndef __U_BOOT__
#അന്തര്ഭാവ "NUM_APPLETS.h"
#എങ്കിൽ അക്കം_APPLETS == 1
/* STANDALONE does not make sense, and won't compile */
# undef ENABLE_FEATURE_SH_STANDALONE
# undef IF_FEATURE_SH_STANDALONE
# undef IF_NOT_FEATURE_SH_STANDALONE
# define ENABLE_FEATURE_SH_STANDALONE 0
# define IF_FEATURE_SH_STANDALONE(...)
# define IF_NOT_FEATURE_SH_STANDALONE(...) __VA_ARGS__
#endif
#endif /* __U_BOOT__ */

#എങ്കിൽ !ENABLE_HUSH_INTERACTIVE
# undef ENABLE_FEATURE_EDITING
# define ENABLE_FEATURE_EDITING 0
# undef ENABLE_FEATURE_EDITING_FANCY_PROMPT
# define ENABLE_FEATURE_EDITING_FANCY_PROMPT 0
# undef ENABLE_FEATURE_EDITING_SAVE_ON_EXIT
# define ENABLE_FEATURE_EDITING_SAVE_ON_EXIT 0
#endif

/* Do we support ANY keywords? */
#എങ്കിൽ ENABLE_HUSH_IF || ENABLE_HUSH_LOOPS || ENABLE_HUSH_CASE
# define HAS_KEYWORDS 1
# define IF_HAS_KEYWORDS(...) __VA_ARGS__
# define IF_HAS_NO_KEYWORDS(...)
#അല്ലെങ്കിൽ
# define HAS_KEYWORDS 0
# define IF_HAS_KEYWORDS(...)
# define IF_HAS_NO_KEYWORDS(...) __VA_ARGS__
#endif

/* If you comment out one of these below, it will be #defined later
 * to perform debug printfs to stderr: */
#define debug_printf(...)         ചെയ്യുക {} ആകുമ്പോൾ (0)
/* Finer-grained debug switches */
#define debug_printf_parse(...)   ചെയ്യുക {} ആകുമ്പോൾ (0)
#define debug_printf_heredoc(...) ചെയ്യുക {} ആകുമ്പോൾ (0)
#define debug_print_വൃക്ഷം(a, b)    ചെയ്യുക {} ആകുമ്പോൾ (0)
#define debug_printf_exec(...)    ചെയ്യുക {} ആകുമ്പോൾ (0)
#define debug_printf_env(...)     ചെയ്യുക {} ആകുമ്പോൾ (0)
#define debug_printf_jobs(...)    ചെയ്യുക {} ആകുമ്പോൾ (0)
#define debug_printf_expand(...)  ചെയ്യുക {} ആകുമ്പോൾ (0)
#define debug_printf_varexp(...)  ചെയ്യുക {} ആകുമ്പോൾ (0)
#define debug_printf_glob(...)    ചെയ്യുക {} ആകുമ്പോൾ (0)
#define debug_printf_redir(...)   ചെയ്യുക {} ആകുമ്പോൾ (0)
#define debug_printf_list(...)    ചെയ്യുക {} ആകുമ്പോൾ (0)
#define debug_printf_subst(...)   ചെയ്യുക {} ആകുമ്പോൾ (0)
#define debug_printf_prompt(...)  ചെയ്യുക {} ആകുമ്പോൾ (0)
#define debug_printf_clean(...)   ചെയ്യുക {} ആകുമ്പോൾ (0)

#define ERR_PTR ((ശൂന്യം*)(long)1)

#define JOB_STATUS_FORMAT    "[%u] %-22s %.40s\n"

#define _SPECIAL_VARS_STR     "_*@$!?#-"
#ifndef __U_BOOT__
#define SPECIAL_VARS_STR     ("_*@$!?#-" + 1)
#define NUMERIC_SPECVARS_STR ("_*@$!?#-" + 3)
#അല്ലെങ്കിൽ /* __U_BOOT__ */
#define SPECIAL_VARS_STR     "*@$!?#-"
#define NUMERIC_SPECVARS_STR "$!?#-"
#endif /* __U_BOOT__ */
#എങ്കിൽ BASH_PATTERN_SUBST
/* Support / and // replace ops */
/* Note that // is stored as \ in "encoded" string representation */
# define VAR_ENCODED_SUBST_OPS      "\\/%#:-=+?"
# define VAR_SUBST_OPS             ("\\/%#:-=+?" + 1)
# define MINUS_PLUS_EQUAL_QUESTION ("\\/%#:-=+?" + 5)
#അല്ലെങ്കിൽ
# define VAR_ENCODED_SUBST_OPS      "%#:-=+?"
# define VAR_SUBST_OPS              "%#:-=+?"
# define MINUS_PLUS_EQUAL_QUESTION ("%#:-=+?" + 3)
#endif

#define SPECIAL_VAR_ചിഹ്നം_STR "\3"
#define SPECIAL_VAR_ചിഹ്നം       3
/* The "variable" with name "\1" emits string "\3". Testcase: "echo ^C" */
#define SPECIAL_VAR_QUOTED_SVS   1

ഘടന variable;

സ്ഥിതം സ്ഥിരം അക്ഷരം hush_version_str[] ALIGN1 = "HUSH_VERSION="BB_VER;

/* This supports saving pointers malloced in vfork child,
 * to be freed in the parent.
 */
#എങ്കിൽ !BB_MMU
തരനിർവചനം ഘടന nommu_save_t {
	ഘടന variable *old_vars;
	അക്ഷരം **argv;
	അക്ഷരം **argv_from_re_execing;
} nommu_save_t;
#endif

എണ്ണൽ {
	RES_NONE  = 0,
#എങ്കിൽ ENABLE_HUSH_IF
	RES_IF    ,
	RES_THEN  ,
	RES_ELIF  ,
	RES_ELSE  ,
	RES_FI    ,
#endif
#എങ്കിൽ ENABLE_HUSH_LOOPS
	RES_FOR   ,
	RES_WHILE ,
	RES_UNTIL ,
	RES_DO    ,
	RES_DONE  ,
#endif
#എങ്കിൽ ENABLE_HUSH_LOOPS || ENABLE_HUSH_CASE
	RES_IN    ,
#endif
#എങ്കിൽ ENABLE_HUSH_CASE
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

തരനിർവചനം ഘടന o_string {
	അക്ഷരം *data;
	പൂർണ്ണസംഖ്യ length; /* position where data is appended */
	പൂർണ്ണസംഖ്യ maxlen;
	പൂർണ്ണസംഖ്യ o_expflags;
	/* At least some part of the string was inside '' or "",
	 * possibly empty one: word"", wo''rd etc. */
	smallint has_quoted_part;
	smallint has_empty_slot;
	smallint ended_in_ifs;
} o_string;
എണ്ണൽ {
	EXP_FLAG_SINGLEWORD     = 0x80, /* must be 0x80 */
	EXP_FLAG_GLOB           = 0x2,
	/* Protect newly added chars against globbing
	 * by prepending \ to *, ?, [, \ */
	EXP_FLAG_ESC_GLOB_CHARS = 0x1,
};
/* Used for initialization: o_string foo = NULL_O_STRING; */
#define NULL_O_STRING { ശൂന്യ }

#ifndef debug_printf_parse
സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം assignment_flag[] ALIGN_PTR = {
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
തരനിർവചനം ഘടന HFILE {
	അക്ഷരം *cur;
	അക്ഷരം *end;
	ഘടന HFILE *next_hfile;
	പൂർണ്ണസംഖ്യ fd;
	അക്ഷരം buf[1024];
} HFILE;

തരനിർവചനം ഘടന in_str {
	സ്ഥിരം അക്ഷരം *p;
	പൂർണ്ണസംഖ്യ peek_buf[2];
	പൂർണ്ണസംഖ്യ last_char;
	HFILE *file;
} in_str;

#ifndef __U_BOOT__
/* The descrip member of this structure is only used to make
 * debugging output pretty */
സ്ഥിതം സ്ഥിരം ഘടന {
	int32_t mode;
	signed അക്ഷരം default_fd;
	അക്ഷരം descrip[3];
} redir_table[] ALIGN4 = {
	{ O_RDONLY,                  0, "<"  },
	{ O_CREAT|O_TRUNC|O_WRONLY,  1, ">"  },
	{ O_CREAT|O_APPEND|O_WRONLY, 1, ">>" },
	{ O_CREAT|O_RDWR,            1, "<>" },
	{ O_RDONLY,                  0, "<<" },
/* Should not be needed. Bogus default_fd helps in debugging */
/*	{ O_RDONLY,                 77, "<<" }, */
};

ഘടന redir_struct {
	ഘടന redir_struct *next;
	അക്ഷരം *rd_filename;          /* filename */
	പൂർണ്ണസംഖ്യ rd_fd;                  /* fd to redirect */
	/* fd to redirect to, or -3 if rd_fd is to be closed (n>&-) */
	പൂർണ്ണസംഖ്യ rd_dup;
	smallint rd_type;           /* (enum redir_type) */
	/* note: for heredocs, rd_filename contains heredoc delimiter,
	 * and subsequently heredoc itself; and rd_dup is a bitmask:
	 * bit 0: do we need to trim leading tabs?
	 * bit 1: is heredoc quoted (<<'delim' syntax) ?
	 */
};
തരനിർവചനം എണ്ണൽ redir_type {
	REDIRECT_നിവേശം     = 0,
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

ഘടന command {
#ifndef __U_BOOT__
	pid_t pid;                  /* 0 if exited */
#endif /* !__U_BOOT__ */
	unsigned assignment_cnt;    /* how many argv[i] are assignments? */
#എങ്കിൽ ENABLE_HUSH_LINENO_VAR
	unsigned lineno;
#endif
	smallint cmd_type;          /* CMD_xxx */
#define CMD_NORMAL   0
#define CMD_SUBSHELL 1
#എങ്കിൽ BASH_TEST2
/* used for "[[ EXPR ]]" */
# define CMD_TEST2_SINGLEWORD_NOGLOB 2
#endif
#എങ്കിൽ BASH_TEST2 || ENABLE_HUSH_LOCAL || ENABLE_HUSH_EXPORT || ENABLE_HUSH_READONLY
/* used to prevent word splitting and globbing in "export v=t*" */
# define CMD_SINGLEWORD_NOGLOB 3
#endif
#എങ്കിൽ ENABLE_HUSH_FUNCTIONS
# define CMD_FUNCDEF 4
#endif

	smalluint cmd_exitcode;
	/* if non-NULL, this "command" is { list }, ( list ), or a compound statement */
	ഘടന pipe *group;
#എങ്കിൽ !BB_MMU
	അക്ഷരം *group_as_string;
#endif
#എങ്കിൽ ENABLE_HUSH_FUNCTIONS
	ഘടന function *child_func;
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
	പൂർണ്ണസംഖ്യ argc; /* number of program arguments */
#endif
	അക്ഷരം **argv;                /* command name and arguments */
/* argv vector may contain variable references (^Cvar^C, ^C0^C etc)
 * and on execution these are substituted with their values.
 * Substitution can make _several_ words out of one argv[n]!
 * Example: argv[0]=='.^C*^C.' here: echo .$*.
 * References of the form ^C`cmd arg^C are `cmd arg` substitutions.
 */
#ifndef __U_BOOT__
	ഘടന redir_struct *redirects; /* I/O redirections */
#endif /* !__U_BOOT__ */
};
/* Is there anything in this command at all? */
#ifndef __U_BOOT__
#define IS_NULL_CMD(cmd) \
	(!(cmd)->group && !(cmd)->argv && !(cmd)->redirects)

#അല്ലെങ്കിൽ /* __U_BOOT__ */
#define IS_NULL_CMD(cmd) \
	(!(cmd)->group && !(cmd)->argv)
#endif /* __U_BOOT__ */
ഘടന pipe {
	ഘടന pipe *next;
	പൂർണ്ണസംഖ്യ അക്കം_cmds;               /* total number of commands in pipe */
#ifndef __U_BOOT__
	പൂർണ്ണസംഖ്യ alive_cmds;             /* number of commands running (not exited) */
	പൂർണ്ണസംഖ്യ stopped_cmds;           /* number of commands alive, but stopped */
#എങ്കിൽ ENABLE_HUSH_JOB
	unsigned jobid;             /* job number */
	pid_t pgrp;                 /* process group ID for the job */
	അക്ഷരം *cmdtext;              /* name of job */
#endif
#endif /* !__U_BOOT__ */
	ഘടന command *cmds;       /* array of commands in pipe */
	smallint followup;          /* PIPE_BG, PIPE_SEQ, PIPE_OR, PIPE_AND */
	IF_HAS_KEYWORDS(smallint pi_inverted;) /* "! cmd | cmd" */
	IF_HAS_KEYWORDS(smallint res_word;) /* needed for if, for, while, until... */
};
തരനിർവചനം എണ്ണൽ pipe_style {
	PIPE_SEQ = 0,
	PIPE_AND = 1,
	PIPE_OR  = 2,
	PIPE_BG  = 3,
} pipe_style;
/* Is there anything in this pipe at all? */
#define IS_NULL_PIPE(pi) \
	((pi)->അക്കം_cmds == 0 IF_HAS_KEYWORDS( && (pi)->res_word == RES_NONE))

/* This holds pointers to the various results of parsing */
ഘടന parse_context {
	/* linked list of pipes */
	ഘടന pipe *list_head;
	/* last pipe (being constructed right now) */
	ഘടന pipe *pipe;
	/* last command in pipe (being constructed right now) */
	ഘടന command *command;
#ifndef __U_BOOT__
	/* last redirect in command->redirects list */
	ഘടന redir_struct *pending_redirect;
#endif /* !__U_BOOT__ */
	o_string word;
#എങ്കിൽ !BB_MMU
	o_string as_string;
#endif
	smallint is_assignment; /* 0:maybe, 1:yes, 2:no, 3:keyword */
#എങ്കിൽ HAS_KEYWORDS
	smallint ctx_res_w;
	smallint ctx_inverted; /* "! cmd | cmd" */
#എങ്കിൽ ENABLE_HUSH_CASE
	smallint ctx_dsemicolon; /* ";;" seen */
#endif
	/* bitmask of FLAG_xxx, for figuring out valid reserved words */
	പൂർണ്ണസംഖ്യ old_flag;
	/* group we are enclosed in:
	 * example: "if pipe1; pipe2; then pipe3; fi"
	 * when we see "if" or "then", we malloc and copy current context,
	 * and make ->stack point to it. then we parse pipeN.
	 * when closing "then" / fi" / whatever is found,
	 * we move list_head into ->stack->command->group,
	 * copy ->stack into current context, and delete ->stack.
	 * (parsing of { list } and ( list ) doesn't use this method)
	 */
	ഘടന parse_context *അട്ടി;
#endif
};
എണ്ണൽ {
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
ഘടന variable {
	ഘടന variable *next;
	അക്ഷരം *varstr;        /* points to "name=" portion */
	പൂർണ്ണസംഖ്യ max_len;         /* if > 0, name is part of initial env; else name is malloced */
#ifndef __U_BOOT__
	uint16_t var_nest_level;
	smallint flg_export; /* putenv should be done on this var */
	smallint flg_read_only;
#endif /* !__U_BOOT__ */
};

എണ്ണൽ {
	BC_BREAK = 1,
	BC_CONTINUE = 2,
};

#എങ്കിൽ ENABLE_HUSH_FUNCTIONS
ഘടന function {
	ഘടന function *next;
	അക്ഷരം *name;
	ഘടന command *parent_cmd;
	ഘടന pipe *body;
# എങ്കിൽ !BB_MMU
	അക്ഷരം *body_as_string;
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
സ്ഥിതം സ്ഥിരം അക്ഷരം o_opt_strings[] ALIGN1 =
	"pipefail\0"
	"noexec\0"
	"errexit\0"
#എങ്കിൽ ENABLE_HUSH_MODE_X
	"xtrace\0"
#endif
	;
എണ്ണൽ {
	OPT_O_PIPEFAIL,
	OPT_O_NOEXEC,
	OPT_O_ERREXIT,
#എങ്കിൽ ENABLE_HUSH_MODE_X
	OPT_O_XTRACE,
#endif
	അക്കം_OPT_O
};

/* "Globals" within this file */
/* Sorted roughly by size (smaller offsets == smaller code) */
ഘടന globals {
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
#എങ്കിൽ ENABLE_HUSH_INTERACTIVE
	/* 'interactive_fd' is a fd# open to ctty, if we have one
	 * _AND_ if we decided to act interactively */
	പൂർണ്ണസംഖ്യ interactive_fd;
	IF_NOT_FEATURE_EDITING_FANCY_PROMPT(അക്ഷരം *PS1;)
# define G_interactive_fd (G.interactive_fd)
#അല്ലെങ്കിൽ
# define G_interactive_fd 0
#endif
#അല്ലെങ്കിൽ /* __U_BOOT__ */
# define G_interactive_fd 0
#endif /* __U_BOOT__ */
#ifndef __U_BOOT__
#എങ്കിൽ ENABLE_FEATURE_EDITING
	വരി_നിവേശം_t *വരി_നിവേശം_state;
#endif
	pid_t വേര്_pid;
	pid_t വേര്_ppid;
	pid_t last_bg_pid;
#എങ്കിൽ ENABLE_HUSH_RANDOM_SUPPORT
	random_t random_gen;
#endif
#എങ്കിൽ ENABLE_HUSH_JOB
	പൂർണ്ണസംഖ്യ run_list_level;
	unsigned last_jobid;
	pid_t saved_tty_pgrp;
	ഘടന pipe *job_list;
# define G_saved_tty_pgrp (G.saved_tty_pgrp)
#അല്ലെങ്കിൽ
# define G_saved_tty_pgrp 0
#endif
#endif /* !__U_BOOT__ */
	/* How deeply are we in context where "set -e" is ignored */
	പൂർണ്ണസംഖ്യ errexit_depth;
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

	അക്ഷരം o_opt[അക്കം_OPT_O];
#എങ്കിൽ ENABLE_HUSH_MODE_X
# define G_x_mode (G.o_opt[OPT_O_XTRACE])
#അല്ലെങ്കിൽ
# define G_x_mode 0
#endif
	അക്ഷരം opt_s;
	അക്ഷരം opt_c;
#endif /* !__U_BOOT__ */
#എങ്കിൽ ENABLE_HUSH_INTERACTIVE
	smallint promptmode; /* 0: PS1, 1: PS2 */
#endif
	/* set by signal handler if SIGINT is received _and_ its trap is not set */
	smallint flag_SIGINT;
#ifndef __U_BOOT__
#എങ്കിൽ ENABLE_HUSH_LOOPS
	smallint flag_break_continue;
#endif
#endif /* !__U_BOOT__ */
#എങ്കിൽ ENABLE_HUSH_FUNCTIONS
	/* 0: outside of a function (or sourced file)
	 * -1: inside of a function, ok to use return builtin
	 * 1: return is invoked, skip all till end of func
	 */
	smallint flag_return_in_progress;
# define G_flag_return_in_progress (G.flag_return_in_progress)
#അല്ലെങ്കിൽ
# define G_flag_return_in_progress 0
#endif
	smallint exiting; /* used to prevent EXIT trap recursion */
	/* These support $? */
	smalluint last_exitcode;
	smalluint expand_exitcode;
	smalluint last_bg_pid_exitcode;
#ifndef __U_BOOT__
#എങ്കിൽ ENABLE_HUSH_SET
	/* are global_argv and global_argv[1..n] malloced? (note: not [0]) */
	smalluint global_args_malloced;
# define G_global_args_malloced (G.global_args_malloced)
#അല്ലെങ്കിൽ
# define G_global_args_malloced 0
#endif
#എങ്കിൽ ENABLE_HUSH_BASH_COMPAT
	പൂർണ്ണസംഖ്യ dead_job_exitcode; /* for "wait -n" */
#endif
#endif /* !__U_BOOT__ */
	/* how many non-NULL argv's we have. NB: $# + 1 */
	പൂർണ്ണസംഖ്യ global_argc;
	അക്ഷരം **global_argv;
#എങ്കിൽ !BB_MMU
	അക്ഷരം *argv0_for_re_execing;
#endif
#എങ്കിൽ ENABLE_HUSH_LOOPS
#ifndef __U_BOOT__
	unsigned depth_break_continue;
#endif /* !__U_BOOT__ */
	unsigned depth_of_loop;
#endif
#ifndef __U_BOOT__
#എങ്കിൽ ENABLE_HUSH_GETOPTS
	unsigned getopt_എണ്ണം;
#endif
#endif /* !__U_BOOT__ */
	സ്ഥിരം അക്ഷരം *ifs;
#ifdef __U_BOOT__
	പൂർണ്ണസംഖ്യ flag_repeat;
	പൂർണ്ണസംഖ്യ do_repeat;
	പൂർണ്ണസംഖ്യ run_command_flags;
#endif /* __U_BOOT__ */
	അക്ഷരം *ifs_whitespace; /* = G.ifs or malloced */
#ifndef __U_BOOT__
	സ്ഥിരം അക്ഷരം *cwd;
#endif /* !__U_BOOT__ */
	ഘടന variable *മുകൾ_var;
	അക്ഷരം **expanded_assignments;
	ഘടന variable **shadowed_vars_pp;
	unsigned var_nest_level;
#ifndef __U_BOOT__
#എങ്കിൽ ENABLE_HUSH_FUNCTIONS
# എങ്കിൽ ENABLE_HUSH_LOCAL
	unsigned func_nest_level; /* solely to prevent "local v" in non-functions */
# endif
	ഘടന function *മുകൾ_func;
#endif
	/* Signal and trap handling */
#എങ്കിൽ ENABLE_HUSH_FAST
	unsigned എണ്ണം_SIGCHLD;
	unsigned handled_SIGCHLD;
	smallint we_have_children;
#endif
#എങ്കിൽ ENABLE_HUSH_LINENO_VAR
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
#എങ്കിൽ ENABLE_HUSH_JOB
	unsigned fatal_sig_mask;
# define G_fatal_sig_mask (G.fatal_sig_mask)
#അല്ലെങ്കിൽ
# define G_fatal_sig_mask 0
#endif
#എങ്കിൽ ENABLE_HUSH_TRAP
	പൂർണ്ണസംഖ്യ pre_trap_exitcode;
# എങ്കിൽ ENABLE_HUSH_FUNCTIONS
	പൂർണ്ണസംഖ്യ return_exitcode;
# endif
	അക്ഷരം **traps; /* char *traps[NSIG] */
# define G_traps G.traps
#അല്ലെങ്കിൽ
# define G_traps ((അക്ഷരം**)ശൂന്യ)
#endif
	sigset_t pending_set;
#എങ്കിൽ ENABLE_HUSH_MEMLEAK
	unsigned long memleak_മൂല്യം;
#endif
#എങ്കിൽ ENABLE_HUSH_MODE_X
	unsigned x_mode_depth;
	/* "set -x" output should not be redirectable with subsequent 2>FILE.
	 * We dup fd#2 to x_mode_fd when "set -x" is executed, and use it
	 * for all subsequent output.
	 */
	പൂർണ്ണസംഖ്യ x_mode_fd;
	o_string x_mode_buf;
#endif
#endif /* !__U_BOOT__ */
#എങ്കിൽ HUSH_DEBUG >= 2
	പൂർണ്ണസംഖ്യ debug_indent;
#endif
#ifndef __U_BOOT__
	ഘടന sigaction sa;
	അക്ഷരം optstring_buf[വലുപ്പം("eixcs")];
#എങ്കിൽ BASH_EPOCH_VARS
	അക്ഷരം epoch_buf[വലുപ്പം("%llu.nnnnnn") + വലുപ്പം(long long)*3];
#endif
#എങ്കിൽ ENABLE_FEATURE_EDITING
	അക്ഷരം user_നിവേശം_buf[CONFIG_FEATURE_EDITING_MAX_LEN];
#endif
#endif /* !__U_BOOT__ */
};
#ifdef __U_BOOT__
ഘടന globals *ptr_to_globals;
#endif /* __U_BOOT__ */
#define G (*ptr_to_globals)
/* Not #defining name to G.name - this quickly gets unwieldy
 * (too many defines). Also, I actually prefer to see when a variable
 * is global, thus "G." prefix is a useful hint */
#ifdef __U_BOOT__
#define SET_PTR_TO_GLOBALS(x) ചെയ്യുക { \
	(*(ഘടന globals**)&ptr_to_globals) = (ശൂന്യം*)(x); \
	barrier(); \
} ആകുമ്പോൾ (0)
#define INIT_G() ചെയ്യുക { \
	SET_PTR_TO_GLOBALS(xzalloc(വലുപ്പം(G))); \
	G.promptmode = 1; \
} ആകുമ്പോൾ (0)
#അല്ലെങ്കിൽ /* !__U_BOOT__ */
#define INIT_G() ചെയ്യുക { \
	SET_PTR_TO_GLOBALS(xzalloc(വലുപ്പം(G))); \
	/* memset(&G.sa, 0, sizeof(G.sa)); */  \
	sigfillset(&G.sa.sa_mask); \
	G.sa.sa_flags = SA_RESTART; \
} ആകുമ്പോൾ (0)
#endif /* !__U_BOOT__ */

#ifndef __U_BOOT__
/* Function prototypes for builtins */
സ്ഥിതം പൂർണ്ണസംഖ്യ builtin_cd(അക്ഷരം **argv) FAST_FUNC;
#എങ്കിൽ ENABLE_HUSH_ECHO
സ്ഥിതം പൂർണ്ണസംഖ്യ builtin_echo(അക്ഷരം **argv) FAST_FUNC;
#endif
സ്ഥിതം പൂർണ്ണസംഖ്യ builtin_eval(അക്ഷരം **argv) FAST_FUNC;
സ്ഥിതം പൂർണ്ണസംഖ്യ builtin_exec(അക്ഷരം **argv) FAST_FUNC;
സ്ഥിതം പൂർണ്ണസംഖ്യ builtin_exit(അക്ഷരം **argv) FAST_FUNC;
#എങ്കിൽ ENABLE_HUSH_EXPORT
സ്ഥിതം പൂർണ്ണസംഖ്യ builtin_export(അക്ഷരം **argv) FAST_FUNC;
#endif
#എങ്കിൽ ENABLE_HUSH_READONLY
സ്ഥിതം പൂർണ്ണസംഖ്യ builtin_readonly(അക്ഷരം **argv) FAST_FUNC;
#endif
സ്ഥിതം പൂർണ്ണസംഖ്യ builtin_false(അക്ഷരം **argv) FAST_FUNC;
#എങ്കിൽ ENABLE_HUSH_JOB
സ്ഥിതം പൂർണ്ണസംഖ്യ builtin_fg_bg(അക്ഷരം **argv) FAST_FUNC;
സ്ഥിതം പൂർണ്ണസംഖ്യ builtin_jobs(അക്ഷരം **argv) FAST_FUNC;
#endif
#എങ്കിൽ ENABLE_HUSH_GETOPTS
സ്ഥിതം പൂർണ്ണസംഖ്യ builtin_getopts(അക്ഷരം **argv) FAST_FUNC;
#endif
#എങ്കിൽ ENABLE_HUSH_HELP
സ്ഥിതം പൂർണ്ണസംഖ്യ builtin_help(അക്ഷരം **argv) FAST_FUNC;
#endif
#എങ്കിൽ MAX_HISTORY && ENABLE_FEATURE_EDITING
സ്ഥിതം പൂർണ്ണസംഖ്യ builtin_history(അക്ഷരം **argv) FAST_FUNC;
#endif
#എങ്കിൽ ENABLE_HUSH_LOCAL
സ്ഥിതം പൂർണ്ണസംഖ്യ builtin_local(അക്ഷരം **argv) FAST_FUNC;
#endif
#എങ്കിൽ ENABLE_HUSH_MEMLEAK
സ്ഥിതം പൂർണ്ണസംഖ്യ builtin_memleak(അക്ഷരം **argv) FAST_FUNC;
#endif
#എങ്കിൽ ENABLE_HUSH_PRINTF
സ്ഥിതം പൂർണ്ണസംഖ്യ builtin_printf(അക്ഷരം **argv) FAST_FUNC;
#endif
സ്ഥിതം പൂർണ്ണസംഖ്യ builtin_pwd(അക്ഷരം **argv) FAST_FUNC;
#എങ്കിൽ ENABLE_HUSH_READ
സ്ഥിതം പൂർണ്ണസംഖ്യ builtin_read(അക്ഷരം **argv) FAST_FUNC;
#endif
#എങ്കിൽ ENABLE_HUSH_SET
സ്ഥിതം പൂർണ്ണസംഖ്യ builtin_set(അക്ഷരം **argv) FAST_FUNC;
#endif
സ്ഥിതം പൂർണ്ണസംഖ്യ builtin_shift(അക്ഷരം **argv) FAST_FUNC;
സ്ഥിതം പൂർണ്ണസംഖ്യ builtin_source(അക്ഷരം **argv) FAST_FUNC;
#എങ്കിൽ ENABLE_HUSH_TEST || BASH_TEST2
സ്ഥിതം പൂർണ്ണസംഖ്യ builtin_test(അക്ഷരം **argv) FAST_FUNC;
#endif
#എങ്കിൽ ENABLE_HUSH_TRAP
സ്ഥിതം പൂർണ്ണസംഖ്യ builtin_trap(അക്ഷരം **argv) FAST_FUNC;
#endif
#എങ്കിൽ ENABLE_HUSH_TYPE
സ്ഥിതം പൂർണ്ണസംഖ്യ builtin_type(അക്ഷരം **argv) FAST_FUNC;
#endif
#എങ്കിൽ ENABLE_HUSH_TIMES
സ്ഥിതം പൂർണ്ണസംഖ്യ builtin_times(അക്ഷരം **argv) FAST_FUNC;
#endif
സ്ഥിതം പൂർണ്ണസംഖ്യ builtin_true(അക്ഷരം **argv) FAST_FUNC;
#എങ്കിൽ ENABLE_HUSH_UMASK
സ്ഥിതം പൂർണ്ണസംഖ്യ builtin_umask(അക്ഷരം **argv) FAST_FUNC;
#endif
#എങ്കിൽ ENABLE_HUSH_UNSET
സ്ഥിതം പൂർണ്ണസംഖ്യ builtin_unset(അക്ഷരം **argv) FAST_FUNC;
#endif
#എങ്കിൽ ENABLE_HUSH_KILL
സ്ഥിതം പൂർണ്ണസംഖ്യ builtin_kill(അക്ഷരം **argv) FAST_FUNC;
#endif
#എങ്കിൽ ENABLE_HUSH_WAIT
സ്ഥിതം പൂർണ്ണസംഖ്യ builtin_wait(അക്ഷരം **argv) FAST_FUNC;
#endif
#എങ്കിൽ ENABLE_HUSH_LOOPS
സ്ഥിതം പൂർണ്ണസംഖ്യ builtin_break(അക്ഷരം **argv) FAST_FUNC;
സ്ഥിതം പൂർണ്ണസംഖ്യ builtin_continue(അക്ഷരം **argv) FAST_FUNC;
#endif
#എങ്കിൽ ENABLE_HUSH_FUNCTIONS
സ്ഥിതം പൂർണ്ണസംഖ്യ builtin_return(അക്ഷരം **argv) FAST_FUNC;
#endif

/* Table of built-in functions.  They can be forked or not, depending on
 * context: within pipes, they fork.  As simple commands, they do not.
 * When used in non-forking context, they can change global variables
 * in the parent shell process.  If forked, of course they cannot.
 * For example, 'unset foo | whatever' will parse and run, but foo will
 * still be set at the end. */
ഘടന built_in_command {
	സ്ഥിരം അക്ഷരം *b_cmd;
	പൂർണ്ണസംഖ്യ (*b_function)(അക്ഷരം **argv) FAST_FUNC;
#എങ്കിൽ ENABLE_HUSH_HELP
	സ്ഥിരം അക്ഷരം *b_descr;
# define BLTIN(cmd, func, help) { cmd, func, help }
#അല്ലെങ്കിൽ
# define BLTIN(cmd, func, help) { cmd, func }
#endif
};

സ്ഥിതം സ്ഥിരം ഘടന built_in_command bltins1[] ALIGN_PTR = {
	BLTIN("."        , builtin_source  , "Run commands in file"),
	BLTIN(":"        , builtin_true    , ശൂന്യ),
#എങ്കിൽ ENABLE_HUSH_JOB
	BLTIN("bg"       , builtin_fg_bg   , "Resume job in background"),
#endif
#എങ്കിൽ ENABLE_HUSH_LOOPS
	BLTIN("break"    , builtin_break   , "Exit loop"),
#endif
	BLTIN("cd"       , builtin_cd      , "Change directory"),
#എങ്കിൽ ENABLE_HUSH_LOOPS
	BLTIN("continue" , builtin_continue, "Start new loop iteration"),
#endif
	BLTIN("eval"     , builtin_eval    , "Construct and run shell command"),
	BLTIN("exec"     , builtin_exec    , "Execute command, don't return to shell"),
	BLTIN("exit"     , builtin_exit    , ശൂന്യ),
#എങ്കിൽ ENABLE_HUSH_EXPORT
	BLTIN("export"   , builtin_export  , "Set environment variables"),
#endif
	BLTIN("false"    , builtin_false   , ശൂന്യ),
#എങ്കിൽ ENABLE_HUSH_JOB
	BLTIN("fg"       , builtin_fg_bg   , "Bring job to foreground"),
#endif
#എങ്കിൽ ENABLE_HUSH_GETOPTS
	BLTIN("getopts"  , builtin_getopts , ശൂന്യ),
#endif
#എങ്കിൽ ENABLE_HUSH_HELP
	BLTIN("help"     , builtin_help    , ശൂന്യ),
#endif
#എങ്കിൽ MAX_HISTORY && ENABLE_FEATURE_EDITING
	BLTIN("history"  , builtin_history , "Show history"),
#endif
#എങ്കിൽ ENABLE_HUSH_JOB
	BLTIN("jobs"     , builtin_jobs    , "List jobs"),
#endif
#എങ്കിൽ ENABLE_HUSH_KILL
	BLTIN("kill"     , builtin_kill    , "Send signals to processes"),
#endif
#എങ്കിൽ ENABLE_HUSH_LOCAL
	BLTIN("local"    , builtin_local   , "Set local variables"),
#endif
#എങ്കിൽ ENABLE_HUSH_MEMLEAK
	BLTIN("memleak"  , builtin_memleak , ശൂന്യ),
#endif
#എങ്കിൽ ENABLE_HUSH_READ
	BLTIN("read"     , builtin_read    , "Input into variable"),
#endif
#എങ്കിൽ ENABLE_HUSH_READONLY
	BLTIN("readonly" , builtin_readonly, "Make variables read-only"),
#endif
#എങ്കിൽ ENABLE_HUSH_FUNCTIONS
	BLTIN("return"   , builtin_return  , "Return from function"),
#endif
#എങ്കിൽ ENABLE_HUSH_SET
	BLTIN("set"      , builtin_set     , "Set positional parameters"),
#endif
	BLTIN("shift"    , builtin_shift   , "Shift positional parameters"),
#എങ്കിൽ BASH_SOURCE
	BLTIN("source"   , builtin_source  , ശൂന്യ),
#endif
#എങ്കിൽ ENABLE_HUSH_TIMES
	BLTIN("times"    , builtin_times   , ശൂന്യ),
#endif
#എങ്കിൽ ENABLE_HUSH_TRAP
	BLTIN("trap"     , builtin_trap    , "Trap signals"),
#endif
	BLTIN("true"     , builtin_true    , ശൂന്യ),
#എങ്കിൽ ENABLE_HUSH_TYPE
	BLTIN("type"     , builtin_type    , "Show command type"),
#endif
#എങ്കിൽ ENABLE_HUSH_ULIMIT
	BLTIN("ulimit"   , shell_builtin_ulimit, "Control resource limits"),
#endif
#എങ്കിൽ ENABLE_HUSH_UMASK
	BLTIN("umask"    , builtin_umask   , "Set file creation mask"),
#endif
#എങ്കിൽ ENABLE_HUSH_UNSET
	BLTIN("unset"    , builtin_unset   , "Unset variables"),
#endif
#എങ്കിൽ ENABLE_HUSH_WAIT
	BLTIN("wait"     , builtin_wait    , "Wait for process to finish"),
#endif
};
/* These builtins won't be used if we are on NOMMU and need to re-exec
 * (it's cheaper to run an external program in this case):
 */
സ്ഥിതം സ്ഥിരം ഘടന built_in_command bltins2[] ALIGN_PTR = {
#എങ്കിൽ ENABLE_HUSH_TEST
	BLTIN("["        , builtin_test    , ശൂന്യ),
#endif
#എങ്കിൽ BASH_TEST2
	BLTIN("[["       , builtin_test    , ശൂന്യ),
#endif
#എങ്കിൽ ENABLE_HUSH_ECHO
	BLTIN("echo"     , builtin_echo    , ശൂന്യ),
#endif
#എങ്കിൽ ENABLE_HUSH_PRINTF
	BLTIN("printf"   , builtin_printf  , ശൂന്യ),
#endif
	BLTIN("pwd"      , builtin_pwd     , ശൂന്യ),
#എങ്കിൽ ENABLE_HUSH_TEST
	BLTIN("test"     , builtin_test    , ശൂന്യ),
#endif
};

#endif /* !__U_BOOT__ */

/* Debug printouts.
 */
#എങ്കിൽ HUSH_DEBUG >= 2
/* prevent disasters with G.debug_indent < 0 */
# define indent() fdprintf(2, "%*s", (G.debug_indent * 2) & 0xff, "")
# define debug_enter() (G.debug_indent++)
# define debug_leave() (G.debug_indent--)
#അല്ലെങ്കിൽ
# define indent()      ((ശൂന്യം)0)
# define debug_enter() ((ശൂന്യം)0)
# define debug_leave() ((ശൂന്യം)0)
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
#അല്ലെങ്കിൽ
# define DEBUG_JOBS 0
#endif

#ifndef debug_printf_expand
# define debug_printf_expand(...) (indent(), fdprintf(2, __VA_ARGS__))
# define DEBUG_EXPAND 1
#അല്ലെങ്കിൽ
# define DEBUG_EXPAND 0
#endif

#ifndef debug_printf_varexp
# define debug_printf_varexp(...) (indent(), fdprintf(2, __VA_ARGS__))
#endif

#ifndef debug_printf_glob
# define debug_printf_glob(...) (indent(), fdprintf(2, __VA_ARGS__))
# define DEBUG_GLOB 1
#അല്ലെങ്കിൽ
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
#അല്ലെങ്കിൽ
# define DEBUG_CLEAN 0
#endif

#എങ്കിൽ DEBUG_EXPAND
സ്ഥിതം ശൂന്യം debug_print_strings(സ്ഥിരം അക്ഷരം *prefix, അക്ഷരം **vv)
{
	indent();
	fdprintf(2, "%s:\n", prefix);
	ആകുമ്പോൾ (*vv)
		fdprintf(2, " '%s'\n", *vv++);
}
#അല്ലെങ്കിൽ
# define debug_print_strings(prefix, vv) ((ശൂന്യം)0)
#endif

/* Leak hunting. Use hush_leaktool.sh for post-processing.
 */
#എങ്കിൽ LEAK_HUNTING
സ്ഥിതം ശൂന്യം *xxmalloc(പൂർണ്ണസംഖ്യ lineno, size_t size)
{
	ശൂന്യം *ptr = xmalloc((size + 0xff) & ~0xff);
	fdprintf(2, "line %d: malloc %p\n", lineno, ptr);
	തിരികെ ptr;
}
സ്ഥിതം ശൂന്യം *xxrealloc(പൂർണ്ണസംഖ്യ lineno, ശൂന്യം *ptr, size_t size)
{
	ptr = xrealloc(ptr, (size + 0xff) & ~0xff);
	fdprintf(2, "line %d: realloc %p\n", lineno, ptr);
	തിരികെ ptr;
}
സ്ഥിതം അക്ഷരം *xxstrdup(പൂർണ്ണസംഖ്യ lineno, സ്ഥിരം അക്ഷരം *str)
{
	അക്ഷരം *ptr = xstrdup(str);
	fdprintf(2, "line %d: strdup %p\n", lineno, ptr);
	തിരികെ ptr;
}
സ്ഥിതം ശൂന്യം xxfree(ശൂന്യം *ptr)
{
	fdprintf(2, "free %p\n", ptr);
	സ്വതന്ത്രം(ptr);
}
# define xmalloc(s)     xxmalloc(__LINE__, s)
# define xrealloc(p, s) xxrealloc(__LINE__, p, s)
# define xstrdup(s)     xxstrdup(__LINE__, s)
# define സ്വതന്ത്രം(p)        xxfree(p)
#endif

/* Syntax and runtime errors. They always abort scripts.
 * In interactive use they usually discard unparsed and/or unexecuted commands
 * and return to the prompt.
 * HUSH_DEBUG >= 2 prints line number in this file where it was detected.
 */
#എങ്കിൽ HUSH_DEBUG < 2
# define സന്ദേശം_and_die_if_script(lineno, ...)     സന്ദേശം_and_die_if_script(__VA_ARGS__)
# define syntax_error(lineno, സന്ദേശം)              syntax_error(സന്ദേശം)
# define syntax_error_at(lineno, സന്ദേശം)           syntax_error_at(സന്ദേശം)
# define syntax_error_unterm_ch(lineno, ch)     syntax_error_unterm_ch(ch)
# define syntax_error_unterm_str(lineno, s)     syntax_error_unterm_str(s)
# define syntax_error_unexpected_ch(lineno, ch) syntax_error_unexpected_ch(ch)
#endif

സ്ഥിതം ശൂന്യം die_if_script(ശൂന്യം)
{
	എങ്കിൽ (!G_interactive_fd) {
		എങ്കിൽ (G.last_exitcode) /* sometines it's 2, not 1 (bash compat) */
			xfunc_error_retval = G.last_exitcode;
		xfunc_die();
	}
}

#ifdef __U_BOOT__
സ്ഥിതം ശൂന്യം __maybe_unused സന്ദേശം_and_die_if_script(unsigned lineno, സ്ഥിരം അക്ഷരം *fmt, ...)
#അല്ലെങ്കിൽ /* !__U_BOOT__ */
സ്ഥിതം ശൂന്യം സന്ദേശം_and_die_if_script(unsigned lineno, സ്ഥിരം അക്ഷരം *fmt, ...)
#endif /* !__U_BOOT__ */
{
	va_list p;

#എങ്കിൽ HUSH_DEBUG >= 2
	bb_error_സന്ദേശം("hush.c:%u", lineno);
#endif
	va_start(p, fmt);
	bb_verror_സന്ദേശം(fmt, p, ശൂന്യ);
	va_end(p);
	die_if_script();
}

സ്ഥിതം ശൂന്യം syntax_error(unsigned lineno UNUSED_PARAM, സ്ഥിരം അക്ഷരം *സന്ദേശം)
{
	എങ്കിൽ (സന്ദേശം)
		bb_error_സന്ദേശം("syntax error: %s", സന്ദേശം);
	അല്ലെങ്കിൽ
		bb_simple_error_സന്ദേശം("syntax error");
	die_if_script();
}

സ്ഥിതം ശൂന്യം syntax_error_at(unsigned lineno UNUSED_PARAM, സ്ഥിരം അക്ഷരം *സന്ദേശം)
{
	bb_error_സന്ദേശം("syntax error at '%s'", സന്ദേശം);
	die_if_script();
}

സ്ഥിതം ശൂന്യം syntax_error_unterm_str(unsigned lineno UNUSED_PARAM, സ്ഥിരം അക്ഷരം *s)
{
	bb_error_സന്ദേശം("syntax error: unterminated %s", s);
//? source4.tests fails: in bash, echo ${^} in script does not terminate the script
// (but bash --posix, or if bash is run as "sh", does terminate in script, so maybe uncomment this?)
//	die_if_script();
}

സ്ഥിതം ശൂന്യം syntax_error_unterm_ch(unsigned lineno, അക്ഷരം ch)
{
	അക്ഷരം സന്ദേശം[2] = { ch, '\0' };
	syntax_error_unterm_str(lineno, സന്ദേശം);
}

സ്ഥിതം ശൂന്യം syntax_error_unexpected_ch(unsigned lineno UNUSED_PARAM, പൂർണ്ണസംഖ്യ ch)
{
	അക്ഷരം സന്ദേശം[2];
	സന്ദേശം[0] = ch;
	സന്ദേശം[1] = '\0';
#എങ്കിൽ HUSH_DEBUG >= 2
	bb_error_സന്ദേശം("hush.c:%u", lineno);
#endif
	bb_error_സന്ദേശം("syntax error: unexpected %s", ch == EOF ? "EOF" : സന്ദേശം);
	die_if_script();
}

#എങ്കിൽ HUSH_DEBUG < 2
# undef സന്ദേശം_and_die_if_script
# undef syntax_error
# undef syntax_error_at
# undef syntax_error_unterm_ch
# undef syntax_error_unterm_str
# undef syntax_error_unexpected_ch
#അല്ലെങ്കിൽ
# define സന്ദേശം_and_die_if_script(...)     സന്ദേശം_and_die_if_script(__LINE__, __VA_ARGS__)
# define syntax_error(സന്ദേശം)              syntax_error(__LINE__, സന്ദേശം)
# define syntax_error_at(സന്ദേശം)           syntax_error_at(__LINE__, സന്ദേശം)
# define syntax_error_unterm_ch(ch)     syntax_error_unterm_ch(__LINE__, ch)
# define syntax_error_unterm_str(s)     syntax_error_unterm_str(__LINE__, s)
# define syntax_error_unexpected_ch(ch) syntax_error_unexpected_ch(__LINE__, ch)
#endif

/* Utility functions
 */
/* Replace each \x with x in place, return ptr past NUL. */
സ്ഥിതം അക്ഷരം *unbackslash(അക്ഷരം *src)
{
#ifdef __U_BOOT__
	അക്ഷരം *dst = src = (അക്ഷരം *)strchrnul(src, '\\');
#അല്ലെങ്കിൽ /* !__U_BOOT__ */
	അക്ഷരം *dst = src = strchrnul(src, '\\');
#endif /* !__U_BOOT__ */
	ആകുമ്പോൾ (1) {
		എങ്കിൽ (*src == '\\') {
			src++;
			എങ്കിൽ (*src != '\0') {
				/* \x -> x */
				*dst++ = *src++;
				തുടരുക;
			}
			/* else: "\<nul>". Do not delete this backslash.
			 * Testcase: eval 'echo ok\'
			 */
			*dst++ = '\\';
			/* fallthrough */
		}
		എങ്കിൽ ((*dst++ = *src++) == '\0')
			നിർത്തുക;
	}
	തിരികെ dst;
}

സ്ഥിതം അക്ഷരം **add_strings_to_strings(അക്ഷരം **strings, അക്ഷരം **add, പൂർണ്ണസംഖ്യ need_to_dup)
{
	പൂർണ്ണസംഖ്യ i;
	unsigned count1;
	unsigned count2;
	അക്ഷരം **v;

	v = strings;
	count1 = 0;
	എങ്കിൽ (v) {
		ആകുമ്പോൾ (*v) {
			count1++;
			v++;
		}
	}
	count2 = 0;
	v = add;
	ആകുമ്പോൾ (*v) {
		count2++;
		v++;
	}
	v = xrealloc(strings, (count1 + count2 + 1) * വലുപ്പം(അക്ഷരം*));
	v[count1 + count2] = ശൂന്യ;
	i = count2;
	ആകുമ്പോൾ (--i >= 0)
		v[count1 + i] = (need_to_dup ? xstrdup(add[i]) : add[i]);
	തിരികെ v;
}
#എങ്കിൽ LEAK_HUNTING
സ്ഥിതം അക്ഷരം **xx_add_strings_to_strings(പൂർണ്ണസംഖ്യ lineno, അക്ഷരം **strings, അക്ഷരം **add, പൂർണ്ണസംഖ്യ need_to_dup)
{
	അക്ഷരം **ptr = add_strings_to_strings(strings, add, need_to_dup);
	fdprintf(2, "line %d: add_strings_to_strings %p\n", lineno, ptr);
	തിരികെ ptr;
}
#define add_strings_to_strings(strings, add, need_to_dup) \
	xx_add_strings_to_strings(__LINE__, strings, add, need_to_dup)
#endif

/* Note: takes ownership of "add" ptr (it is not strdup'ed) */
സ്ഥിതം അക്ഷരം **add_string_to_strings(അക്ഷരം **strings, അക്ഷരം *add)
{
	അക്ഷരം *v[2];
	v[0] = add;
	v[1] = ശൂന്യ;
	തിരികെ add_strings_to_strings(strings, v, /*dup:*/ 0);
}

#എങ്കിൽ LEAK_HUNTING
സ്ഥിതം അക്ഷരം **xx_add_string_to_strings(പൂർണ്ണസംഖ്യ lineno, അക്ഷരം **strings, അക്ഷരം *add)
{
	അക്ഷരം **ptr = add_string_to_strings(strings, add);
	fdprintf(2, "line %d: add_string_to_strings %p\n", lineno, ptr);
	തിരികെ ptr;
}
#define add_string_to_strings(strings, add) \
	xx_add_string_to_strings(__LINE__, strings, add)
#endif

സ്ഥിതം ശൂന്യം free_strings(അക്ഷരം **strings)
{
	അക്ഷരം **v;

	എങ്കിൽ (!strings)
		തിരികെ;
	v = strings;
	ആകുമ്പോൾ (*v) {
		സ്വതന്ത്രം(*v);
		v++;
	}
	സ്വതന്ത്രം(strings);
}

#ifndef __U_BOOT__
സ്ഥിതം പൂർണ്ണസംഖ്യ dup_CLOEXEC(പൂർണ്ണസംഖ്യ fd, പൂർണ്ണസംഖ്യ avoid_fd)
{
	പൂർണ്ണസംഖ്യ newfd;
 repeat:
	newfd = fcntl(fd, F_DUPFD_CLOEXEC, avoid_fd + 1);
	എങ്കിൽ (newfd >= 0) {
		എങ്കിൽ (F_DUPFD_CLOEXEC == F_DUPFD) /* if old libc (w/o F_DUPFD_CLOEXEC) */
			close_on_exec_on(newfd);
	} അല്ലെങ്കിൽ { /* newfd < 0 */
		എങ്കിൽ (errno == EBUSY)
			പോകുക repeat;
		എങ്കിൽ (errno == EINTR)
			പോകുക repeat;
		എങ്കിൽ (errno != EBADF) {
			/* "echo >&9999" gets EINVAL trying to save fd 1 to above 9999.
			 * We could try saving it _below_ 9999 instead (how?), but
			 * this probably means that dup2(9999,1) to effectuate >&9999
			 * would also not work: fd 9999 can't exist.
			 * (This differs from "echo >&99" where saving works, but
			 * subsequent dup2(99,1) fails if fd 99 is not open).
			 */
			bb_perror_സന്ദേശം("fcntl(%d,F_DUPFD,%d)", fd, avoid_fd + 1);
		}
	}
	തിരികെ newfd;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ xdup_CLOEXEC_and_close(പൂർണ്ണസംഖ്യ fd, പൂർണ്ണസംഖ്യ avoid_fd)
{
	പൂർണ്ണസംഖ്യ newfd;
 repeat:
	newfd = fcntl(fd, F_DUPFD_CLOEXEC, avoid_fd + 1);
	എങ്കിൽ (newfd < 0) {
		എങ്കിൽ (errno == EBUSY)
			പോകുക repeat;
		എങ്കിൽ (errno == EINTR)
			പോകുക repeat;
		/* fd was not open? */
		എങ്കിൽ (errno == EBADF)
			തിരികെ fd;
		xfunc_die();
	}
	എങ്കിൽ (F_DUPFD_CLOEXEC == F_DUPFD) /* if old libc (w/o F_DUPFD_CLOEXEC) */
		close_on_exec_on(newfd);
	close(fd);
	തിരികെ newfd;
}

/* Manipulating HFILEs */
സ്ഥിതം HFILE *hfopen(സ്ഥിരം അക്ഷരം *name)
{
	HFILE *fp;
	പൂർണ്ണസംഖ്യ fd;

	fd = STDIN_FILENO;
	എങ്കിൽ (name) {
		fd = open(name, O_RDONLY | O_CLOEXEC);
		എങ്കിൽ (fd < 0)
			തിരികെ ശൂന്യ;
		എങ്കിൽ (O_CLOEXEC == 0) /* ancient libc */
			close_on_exec_on(fd);
	}

	fp = xmalloc(വലുപ്പം(*fp));
	എങ്കിൽ (name == ശൂന്യ)
		G.HFILE_stdin = fp;
	fp->fd = fd;
	fp->cur = fp->end = fp->buf;
	fp->next_hfile = G.HFILE_list;
	G.HFILE_list = fp;
	തിരികെ fp;
}
സ്ഥിതം ശൂന്യം hfclose(HFILE *fp)
{
	HFILE **pp = &G.HFILE_list;
	ആകുമ്പോൾ (*pp) {
		HFILE *cur = *pp;
		എങ്കിൽ (cur == fp) {
			*pp = cur->next_hfile;
			നിർത്തുക;
		}
		pp = &cur->next_hfile;
	}
	എങ്കിൽ (fp->fd >= 0)
		close(fp->fd);
	സ്വതന്ത്രം(fp);
}
സ്ഥിതം പൂർണ്ണസംഖ്യ refill_HFILE_and_getc(HFILE *fp)
{
	പൂർണ്ണസംഖ്യ n;

	എങ്കിൽ (fp->fd < 0) {
		/* Already saw EOF */
		തിരികെ EOF;
	}
#എങ്കിൽ ENABLE_HUSH_INTERACTIVE && !ENABLE_FEATURE_EDITING
	/* If user presses ^C, read() restarts after SIGINT (we use SA_RESTART).
	 * IOW: ^C will not immediately stop line input.
	 * But poll() is different: it does NOT restart after signals.
	 */
	എങ്കിൽ (fp == G.HFILE_stdin) {
		ഘടന pollfd pfd[1];
		pfd[0].fd = fp->fd;
		pfd[0].events = POLLIN;
		n = poll(pfd, 1, -1);
		എങ്കിൽ (n < 0
		 /*&& errno == EINTR - assumed true */
		 && sigismember(&G.pending_set, SIGINT)
		) {
			തിരികെ '\0';
		}
	}
#അല്ലെങ്കിൽ
/* if FEATURE_EDITING=y, we do not use this routine for interactive input */
#endif
	/* Try to buffer more input */
	n = safe_read(fp->fd, fp->buf, വലുപ്പം(fp->buf));
	എങ്കിൽ (n < 0) {
		bb_simple_perror_സന്ദേശം("read error");
		n = 0;
	}
	fp->cur = fp->buf;
	fp->end = fp->buf + n;
	എങ്കിൽ (n == 0) {
		/* EOF/error */
		close(fp->fd);
		fp->fd = -1;
		തിരികെ EOF;
	}
	തിരികെ (unsigned അക്ഷരം)(*fp->cur++);
}
/* Inlined for common case of non-empty buffer.
 */
സ്ഥിതം ALWAYS_INLINE പൂർണ്ണസംഖ്യ hfgetc(HFILE *fp)
{
	എങ്കിൽ (fp->cur < fp->end)
		തിരികെ (unsigned അക്ഷരം)(*fp->cur++);
	/* Buffer empty */
	തിരികെ refill_HFILE_and_getc(fp);
}
സ്ഥിതം പൂർണ്ണസംഖ്യ move_HFILEs_on_redirect(പൂർണ്ണസംഖ്യ fd, പൂർണ്ണസംഖ്യ avoid_fd)
{
	HFILE *fl = G.HFILE_list;
	ആകുമ്പോൾ (fl) {
		എങ്കിൽ (fd == fl->fd) {
			/* We use it only on script files, they are all CLOEXEC */
			fl->fd = xdup_CLOEXEC_and_close(fd, avoid_fd);
			debug_printf_redir("redirect_fd %d: matches a script fd, moving it to %d\n", fd, fl->fd);
			തിരികെ 1; /* "found and moved" */
		}
		fl = fl->next_hfile;
	}
#എങ്കിൽ ENABLE_HUSH_MODE_X
	എങ്കിൽ (G.x_mode_fd > 0 && fd == G.x_mode_fd) {
		G.x_mode_fd = xdup_CLOEXEC_and_close(fd, avoid_fd);
		തിരികെ 1; /* "found and moved" */
	}
#endif
	തിരികെ 0; /* "not in the list" */
}
#എങ്കിൽ ENABLE_FEATURE_SH_STANDALONE && BB_MMU
സ്ഥിതം ശൂന്യം close_all_HFILE_list(ശൂന്യം)
{
	HFILE *fl = G.HFILE_list;
	ആകുമ്പോൾ (fl) {
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
		എങ്കിൽ (fl->fd > 0)
			/*hfclose(fl); - unsafe */
			close(fl->fd);
		fl = fl->next_hfile;
	}
}
#endif
സ്ഥിതം പൂർണ്ണസംഖ്യ fd_in_HFILEs(പൂർണ്ണസംഖ്യ fd)
{
	HFILE *fl = G.HFILE_list;
	ആകുമ്പോൾ (fl) {
		എങ്കിൽ (fl->fd == fd)
			തിരികെ 1;
		fl = fl->next_hfile;
	}
	തിരികെ 0;
}

#endif /* !__U_BOOT__ */

/* Helpers for setting new $n and restoring them back
 */
തരനിർവചനം ഘടന save_arg_t {
	അക്ഷരം *sv_argv0;
	അക്ഷരം **sv_g_argv;
	പൂർണ്ണസംഖ്യ sv_g_argc;
#ifndef __U_BOOT__
	IF_HUSH_SET(smallint sv_g_malloced;)
#endif /* !__U_BOOT__ */
} save_arg_t;

#ifndef __U_BOOT__
സ്ഥിതം ശൂന്യം save_and_replace_G_args(save_arg_t *sv, അക്ഷരം **argv)
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

സ്ഥിതം ശൂന്യം restore_G_args(save_arg_t *sv, അക്ഷരം **argv)
{
#എങ്കിൽ ENABLE_HUSH_SET
	എങ്കിൽ (G.global_args_malloced) {
		/* someone ran "set -- arg1 arg2 ...", undo */
		അക്ഷരം **pp = G.global_argv;
		ആകുമ്പോൾ (*++pp) /* note: does not free $0 */
			സ്വതന്ത്രം(*pp);
		സ്വതന്ത്രം(G.global_argv);
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
എണ്ണൽ {
	SPECIAL_INTERACTIVE_SIGS = 0
		| (1 << SIGTERM)
		| (1 << SIGINT)
		| (1 << SIGHUP)
		,
	SPECIAL_JOBSTOP_SIGS = 0
#എങ്കിൽ ENABLE_HUSH_JOB
		| (1 << SIGTTIN)
		| (1 << SIGTTOU)
		| (1 << SIGTSTP)
#endif
		,
};

സ്ഥിതം ശൂന്യം record_pending_signo(പൂർണ്ണസംഖ്യ sig)
{
	sigaddset(&G.pending_set, sig);
#എങ്കിൽ ENABLE_FEATURE_EDITING
	എങ്കിൽ (sig != SIGCHLD
	 || (G_traps && G_traps[SIGCHLD] && G_traps[SIGCHLD][0])
	 /* ^^^ if SIGCHLD, interrupt line reading only if it has a trap */
	) {
		bb_got_signal = sig; /* for read_line_input: "we got a signal" */
	}
#endif
#എങ്കിൽ ENABLE_HUSH_FAST
	എങ്കിൽ (sig == SIGCHLD) {
		G.എണ്ണം_SIGCHLD++;
//bb_error_msg("[%d] SIGCHLD_handler: G.count_SIGCHLD:%d G.handled_SIGCHLD:%d", getpid(), G.count_SIGCHLD, G.handled_SIGCHLD);
	}
#endif
}

സ്ഥിതം sighandler_t install_sighandler(പൂർണ്ണസംഖ്യ sig, sighandler_t handler)
{
	ഘടന sigaction old_sa;

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
	തിരികെ old_sa.sa_handler;
}
#endif /* !__U_BOOT__ */

#ifndef __U_BOOT__
സ്ഥിതം ശൂന്യം hush_exit(പൂർണ്ണസംഖ്യ exitcode) NORETURN;

സ്ഥിതം ശൂന്യം restore_ttypgrp_and__exit(ശൂന്യം) NORETURN;
സ്ഥിതം ശൂന്യം restore_ttypgrp_and__exit(ശൂന്യം)
{
	/* xfunc has failed! die die die */
	/* no EXIT traps, this is an escape hatch! */
	G.exiting = 1;
	hush_exit(xfunc_error_retval);
}

#എങ്കിൽ ENABLE_HUSH_JOB

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
സ്ഥിതം ശൂന്യം fflush_and__exit(ശൂന്യം) NORETURN;
സ്ഥിതം ശൂന്യം fflush_and__exit(ശൂന്യം)
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
സ്ഥിതം ശൂന്യം sigexit(പൂർണ്ണസംഖ്യ sig) NORETURN;
സ്ഥിതം ശൂന്യം sigexit(പൂർണ്ണസംഖ്യ sig)
{
	/* Careful: we can end up here after [v]fork. Do not restore
	 * tty pgrp then, only top-level shell process does that */
	എങ്കിൽ (G_saved_tty_pgrp && getpid() == G.വേര്_pid) {
		/* Disable all signals: job control, SIGPIPE, etc.
		 * Mostly paranoid measure, to prevent infinite SIGTTOU.
		 */
		sigprocmask_allsigs(SIG_BLOCK);
		tcsetpgrp(G_interactive_fd, G_saved_tty_pgrp);
	}

	/* Not a signal, just exit */
	എങ്കിൽ (sig <= 0)
		_exit(- sig);

	kill_myself_with_sig(sig); /* does not return */
}
#അല്ലെങ്കിൽ

# define disable_restore_tty_pgrp_on_exit() ((ശൂന്യം)0)
# define enable_restore_tty_pgrp_on_exit()  ((ശൂന്യം)0)

#endif
#endif /* !__U_BOOT__ */

#ifndef __U_BOOT__
സ്ഥിതം sighandler_t pick_sighandler(unsigned sig)
{
	sighandler_t handler = SIG_DFL;
	എങ്കിൽ (sig < വലുപ്പം(unsigned)*8) {
		unsigned sigmask = (1 << sig);

#എങ്കിൽ ENABLE_HUSH_JOB
		/* is sig fatal? */
		എങ്കിൽ (G_fatal_sig_mask & sigmask)
			handler = sigexit;
		അല്ലെങ്കിൽ
#endif
		/* sig has special handling? */
		എങ്കിൽ (G.special_sig_mask & sigmask) {
			handler = record_pending_signo;
			/* TTIN/TTOU/TSTP can't be set to record_pending_signo
			 * in order to ignore them: they will be raised
			 * in an endless loop when we try to do some
			 * terminal ioctls! We do have to _ignore_ these.
			 */
			എങ്കിൽ (SPECIAL_JOBSTOP_SIGS & sigmask)
				handler = SIG_IGN;
		}
	}
	തിരികെ handler;
}
#endif /* !__U_BOOT__ */

#ifndef __U_BOOT__
/* Restores tty foreground process group, and exits. */
സ്ഥിതം ശൂന്യം hush_exit(പൂർണ്ണസംഖ്യ exitcode)
{
#എങ്കിൽ ENABLE_FEATURE_EDITING_SAVE_ON_EXIT
	save_history(G.വരി_നിവേശം_state); /* may be NULL */
#endif

	fflush_all();
	എങ്കിൽ (G.exiting <= 0 && G_traps && G_traps[0] && G_traps[0][0]) {
		അക്ഷരം *argv[3];
		/* argv[0] is unused */
		argv[1] = xstrdup(G_traps[0]); /* copy, since EXIT trap handler may modify G_traps[0] */
		argv[2] = ശൂന്യ;
		G.exiting = 1; /* prevent EXIT trap recursion */
		/* Note: G_traps[0] is not cleared!
		 * "trap" will still show it, if executed
		 * in the handler */
		builtin_eval(argv);
	}

#എങ്കിൽ ENABLE_FEATURE_CLEAN_UP
	{
		ഘടന variable *cur_var;
		എങ്കിൽ (G.cwd != bb_സന്ദേശം_unknown)
			സ്വതന്ത്രം((അക്ഷരം*)G.cwd);
		cur_var = G.മുകൾ_var;
		ആകുമ്പോൾ (cur_var) {
			ഘടന variable *tmp = cur_var;
			എങ്കിൽ (!cur_var->max_len)
				സ്വതന്ത്രം(cur_var->varstr);
			cur_var = cur_var->next;
			സ്വതന്ത്രം(tmp);
		}
	}
#endif

	fflush_all();
#എങ്കിൽ ENABLE_HUSH_JOB
	sigexit(- (exitcode & 0xff));
#അല്ലെങ്കിൽ
	_exit(exitcode);
#endif
}

//TODO: return a mask of ALL handled sigs?
സ്ഥിതം പൂർണ്ണസംഖ്യ check_and_run_traps(ശൂന്യം)
{
	പൂർണ്ണസംഖ്യ last_sig = 0;

	ആകുമ്പോൾ (1) {
		പൂർണ്ണസംഖ്യ sig;

		എങ്കിൽ (sigisemptyset(&G.pending_set))
			നിർത്തുക;
		sig = 0;
		ചെയ്യുക {
			sig++;
			എങ്കിൽ (sigismember(&G.pending_set, sig)) {
				sigdelset(&G.pending_set, sig);
				പോകുക got_sig;
			}
		} ആകുമ്പോൾ (sig < NSIG);
		നിർത്തുക;
 got_sig:
#എങ്കിൽ ENABLE_HUSH_TRAP
		എങ്കിൽ (G_traps && G_traps[sig]) {
			debug_printf_exec("%s: sig:%d handler:'%s'\n", __func__, sig, G.traps[sig]);
			എങ്കിൽ (G_traps[sig][0]) {
				/* We have user-defined handler */
				smalluint save_rcode;
				പൂർണ്ണസംഖ്യ save_pre;
				അക്ഷരം *argv[3];
				/* argv[0] is unused */
				argv[1] = xstrdup(G_traps[sig]);
				/* why strdup? trap can modify itself: trap 'trap "echo oops" INT' INT */
				argv[2] = ശൂന്യ;
				save_pre = G.pre_trap_exitcode;
				G.pre_trap_exitcode = save_rcode = G.last_exitcode;
				builtin_eval(argv);
				സ്വതന്ത്രം(argv[1]);
				G.pre_trap_exitcode = save_pre;
				G.last_exitcode = save_rcode;
# എങ്കിൽ ENABLE_HUSH_FUNCTIONS
				എങ്കിൽ (G.return_exitcode >= 0) {
					debug_printf_exec("trap exitcode:%d\n", G.return_exitcode);
					G.last_exitcode = G.return_exitcode;
				}
# endif
				last_sig = sig;
			} /* else: "" trap, ignoring signal */
			തുടരുക;
		}
#endif
		/* not a trap: special action */
		തിരഞ്ഞെടുപ്പ് (sig) {
		അവസ്ഥ SIGINT:
			debug_printf_exec("%s: sig:%d default SIGINT handler\n", __func__, sig);
			G.flag_SIGINT = 1;
			last_sig = sig;
			നിർത്തുക;
#എങ്കിൽ ENABLE_HUSH_JOB
		അവസ്ഥ SIGHUP: {
			/* if (G_interactive_fd) - no need to check, the handler
			 * is only installed if we *are* interactive */
			{
				/* bash compat: "Before exiting, an interactive
				 * shell resends the SIGHUP to all jobs, running
				 * or stopped.  Stopped jobs are sent SIGCONT
				 * to ensure that they receive the SIGHUP."
				 */
				ഘടന pipe *job;
				debug_printf_exec("%s: sig:%d default SIGHUP handler\n", __func__, sig);
				/* bash is observed to signal whole process groups,
				 * not individual processes */
				വേണ്ടി (job = G.job_list; job; job = job->next) {
					എങ്കിൽ (job->pgrp <= 0)
						തുടരുക;
					debug_printf_exec("HUPing pgrp %d\n", job->pgrp);
					എങ്കിൽ (kill(- job->pgrp, SIGHUP) == 0)
						kill(- job->pgrp, SIGCONT);
				}
			}
			/* this restores tty pgrp, then kills us with SIGHUP */
			sigexit(SIGHUP);
		}
#endif
#എങ്കിൽ ENABLE_HUSH_FAST
		അവസ്ഥ SIGCHLD:
			debug_printf_exec("%s: sig:%d default SIGCHLD handler\n", __func__, sig);
			G.എണ്ണം_SIGCHLD++;
//bb_error_msg("[%d] check_and_run_traps: G.count_SIGCHLD:%d G.handled_SIGCHLD:%d", getpid(), G.count_SIGCHLD, G.handled_SIGCHLD);
			/* Note:
			 * We don't do 'last_sig = sig' here -> NOT returning this sig.
			 * This simplifies wait builtin a bit.
			 */
			നിർത്തുക;
#endif
		സ്ഥിരസ്ഥിതി: /* ignored: */
			debug_printf_exec("%s: sig:%d default handling is to ignore\n", __func__, sig);
			/* SIGTERM, SIGQUIT, SIGTTIN, SIGTTOU, SIGTSTP */
			/* Note:
			 * We don't do 'last_sig = sig' here -> NOT returning this sig.
			 * Example: wait is not interrupted by TERM
			 * in interactive shell, because TERM is ignored.
			 */
			നിർത്തുക;
		}
	}
	തിരികെ last_sig;
}

സ്ഥിതം സ്ഥിരം അക്ഷരം *get_cwd(പൂർണ്ണസംഖ്യ force)
{
	എങ്കിൽ (force || G.cwd == ശൂന്യ) {
		/* xrealloc_getcwd_or_warn(arg) calls free(arg),
		 * we must not try to free(bb_msg_unknown) */
		എങ്കിൽ (G.cwd == bb_സന്ദേശം_unknown)
			G.cwd = ശൂന്യ;
		G.cwd = xrealloc_getcwd_or_warn((അക്ഷരം *)G.cwd);
		എങ്കിൽ (!G.cwd)
			G.cwd = bb_സന്ദേശം_unknown;
	}
	തിരികെ G.cwd;
}

#endif /* !__U_BOOT__ */

/*
 * Shell and environment variable support
 */
സ്ഥിതം ഘടന variable **get_ptr_to_local_var(സ്ഥിരം അക്ഷരം *name)
{
	ഘടന variable **pp;
	ഘടന variable *cur;

	pp = &G.മുകൾ_var;
	ആകുമ്പോൾ ((cur = *pp) != ശൂന്യ) {
		എങ്കിൽ (varcmp(cur->varstr, name) == 0)
			തിരികെ pp;
		pp = &cur->next;
	}
	തിരികെ ശൂന്യ;
}

സ്ഥിതം സ്ഥിരം അക്ഷരം* FAST_FUNC get_local_var_മൂല്യം(സ്ഥിരം അക്ഷരം *name)
{
	ഘടന variable **vpp;

	എങ്കിൽ (G.expanded_assignments) {
		അക്ഷരം **cpp = G.expanded_assignments;
		ആകുമ്പോൾ (*cpp) {
			അക്ഷരം *cp = *cpp;
			എങ്കിൽ (varcmp(cp, name) == 0)
				തിരികെ strchr(cp, '=') + 1;
			cpp++;
		}
	}

	vpp = get_ptr_to_local_var(name);
	എങ്കിൽ (vpp)
		തിരികെ strchr((*vpp)->varstr, '=') + 1;

#ifndef __U_BOOT__
	എങ്കിൽ (strcmp(name, "PPID") == 0)
		തിരികെ utoa(G.വേര്_ppid);
#endif /* !__U_BOOT__ */
	// bash compat: UID? EUID?
#എങ്കിൽ ENABLE_HUSH_RANDOM_SUPPORT
	എങ്കിൽ (strcmp(name, "RANDOM") == 0)
		തിരികെ utoa(next_random(&G.random_gen));
#endif
#എങ്കിൽ ENABLE_HUSH_LINENO_VAR
	എങ്കിൽ (strcmp(name, "LINENO") == 0)
		തിരികെ utoa(G.execute_lineno);
#endif
#എങ്കിൽ BASH_EPOCH_VARS
	{
		സ്ഥിരം അക്ഷരം *fmt = ശൂന്യ;
		എങ്കിൽ (strcmp(name, "EPOCHSECONDS") == 0)
			fmt = "%llu";
		അല്ലെങ്കിൽ എങ്കിൽ (strcmp(name, "EPOCHREALTIME") == 0)
			fmt = "%llu.%06u";
		എങ്കിൽ (fmt) {
			ഘടന timeval tv;
			xgettimeofday(&tv);
			sprintf(G.epoch_buf, fmt, (unsigned long long)tv.tv_sec,
					(unsigned)tv.tv_usec);
			തിരികെ G.epoch_buf;
		}
	}
#endif
	തിരികെ ശൂന്യ;
}

#ifndef __U_BOOT__
#എങ്കിൽ ENABLE_HUSH_GETOPTS
സ്ഥിതം ശൂന്യം handle_changed_special_names(സ്ഥിരം അക്ഷരം *name)
{
	എങ്കിൽ (varcmp(name, "OPTIND") == 0) {
		G.getopt_എണ്ണം = 0;
		തിരികെ;
	}
}
#അല്ലെങ്കിൽ
/* Do not even bother evaluating arguments */
# define handle_changed_special_names(...) ((ശൂന്യം)0)
#endif
#അല്ലെങ്കിൽ /* __U_BOOT__ */
/* Do not even bother evaluating arguments */
# define handle_changed_special_names(...) ((ശൂന്യം)0)
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
സ്ഥിതം പൂർണ്ണസംഖ്യ set_local_var(അക്ഷരം *str, unsigned flags)
#അല്ലെങ്കിൽ /* __U_BOOT__ */
പൂർണ്ണസംഖ്യ set_local_var_modern(അക്ഷരം *str, പൂർണ്ണസംഖ്യ flags)
#endif /* __U_BOOT__ */
{
	ഘടന variable **cur_pp;
	ഘടന variable *cur;
	അക്ഷരം *free_me = ശൂന്യ;
	അക്ഷരം *eq_sign;
	പൂർണ്ണസംഖ്യ name_len;
	പൂർണ്ണസംഖ്യ retval;
#ifndef __U_BOOT__
	unsigned local_lvl = (flags >> SETFLAG_VARLVL_SHIFT);
#endif /* !__U_BOOT__ */

	eq_sign = strchr(str, '=');
	എങ്കിൽ (HUSH_DEBUG && !eq_sign)
		bb_simple_error_സന്ദേശം_and_die("BUG in setvar");

	name_len = eq_sign - str + 1; /* including '=' */
	cur_pp = &G.മുകൾ_var;
	ആകുമ്പോൾ ((cur = *cur_pp) != ശൂന്യ) {
		എങ്കിൽ (strncmp(cur->varstr, str, name_len) != 0) {
			cur_pp = &cur->next;
			തുടരുക;
		}

#ifndef __U_BOOT__
		/* We found an existing var with this name */
		എങ്കിൽ (cur->flg_read_only) {
			bb_error_സന്ദേശം("%s: readonly variable", str);
			സ്വതന്ത്രം(str);
//NOTE: in bash, assignment in "export READONLY_VAR=Z" fails, and sets $?=1,
//but export per se succeeds (does put the var in env). We don't mimic that.
			തിരികെ -1;
		}
		എങ്കിൽ (flags & SETFLAG_UNEXPORT) { // && cur->flg_export ?
			debug_printf_env("%s: unsetenv '%s'\n", __func__, str);
			*eq_sign = '\0';
			unsetenv(str);
			*eq_sign = '=';
		}
		എങ്കിൽ (cur->var_nest_level < local_lvl) {
			/* bash 3.2.33(1) and exported vars:
			 * # export z=z
			 * # f() { local z=a; env | grep ^z; }
			 * # f
			 * z=a
			 * # env | grep ^z
			 * z=z
			 */
			എങ്കിൽ (cur->flg_export)
				flags |= SETFLAG_EXPORT;
			/* New variable is local ("local VAR=VAL" or
			 * "VAR=VAL cmd")
			 * and existing one is global, or local
			 * on a lower level that new one.
			 * Remove it from global variable list:
			 */
			*cur_pp = cur->next;
			എങ്കിൽ (G.shadowed_vars_pp) {
				/* Save in "shadowed" list */
				debug_printf_env("shadowing %s'%s'/%u by '%s'/%u\n",
					cur->flg_export ? "exported " : "",
					cur->varstr, cur->var_nest_level, str, local_lvl
				);
				cur->next = *G.shadowed_vars_pp;
				*G.shadowed_vars_pp = cur;
			} അല്ലെങ്കിൽ {
				/* Came from pseudo_exec_argv(), no need to save: delete it */
				debug_printf_env("shadow-deleting %s'%s'/%u by '%s'/%u\n",
					cur->flg_export ? "exported " : "",
					cur->varstr, cur->var_nest_level, str, local_lvl
				);
				എങ്കിൽ (cur->max_len == 0) /* allocated "VAR=VAL"? */
					free_me = cur->varstr; /* then free it later */
				സ്വതന്ത്രം(cur);
			}
			നിർത്തുക;
		}
#endif /* !__U_BOOT__ */

		എങ്കിൽ (strcmp(cur->varstr + name_len, eq_sign + 1) == 0) {
			debug_printf_env("assignement '%s' does not change anything\n", str);
 free_and_exp:
			സ്വതന്ത്രം(str);
			പോകുക exp;
		}

		/* Replace the value in the found "struct variable" */
		എങ്കിൽ (cur->max_len != 0) {
			എങ്കിൽ (cur->max_len >= strnlen(str, cur->max_len + 1)) {
				/* This one is from startup env, reuse space */
				debug_printf_env("reusing startup env for '%s'\n", str);
				strcpy(cur->varstr, str);
				പോകുക free_and_exp;
			}
			/* Can't reuse */
			cur->max_len = 0;
			പോകുക set_str_and_exp;
		}
		/* max_len == 0 signifies "malloced" var, which we can
		 * (and have to) free. But we can't free(cur->varstr) here:
		 * if cur->flg_export is 1, it is in the environment.
		 * We should either unsetenv+free, or wait until putenv,
		 * then putenv(new)+free(old).
		 */
		free_me = cur->varstr;
		പോകുക set_str_and_exp;
	}

	/* Not found or shadowed - create new variable struct */
#ifndef __U_BOOT__
	debug_printf_env("%s: alloc new var '%s'/%u\n", __func__, str, local_lvl);
#അല്ലെങ്കിൽ /* __U_BOOT__ */
	debug_printf_env("%s: alloc new var '%s'\n", __func__, str);
#endif /* __U_BOOT__ */
	cur = xzalloc(വലുപ്പം(*cur));
#ifndef __U_BOOT__
	cur->var_nest_level = local_lvl;
#endif /* !__U_BOOT__ */
	cur->next = *cur_pp;
	*cur_pp = cur;

 set_str_and_exp:
	cur->varstr = str;
 exp:
#ifndef __U_BOOT__
#എങ്കിൽ !BB_MMU || ENABLE_HUSH_READONLY
	എങ്കിൽ (flags & SETFLAG_MAKE_RO) {
		cur->flg_read_only = 1;
	}
#endif
	എങ്കിൽ (flags & SETFLAG_EXPORT)
		cur->flg_export = 1;
#endif /* !__U_BOOT__ */
	retval = 0;
#ifndef __U_BOOT__
	എങ്കിൽ (cur->flg_export) {
		എങ്കിൽ (flags & SETFLAG_UNEXPORT) {
			cur->flg_export = 0;
			/* unsetenv was already done */
		} അല്ലെങ്കിൽ {
			debug_printf_env("%s: putenv '%s'/%u\n", __func__, cur->varstr, cur->var_nest_level);
			retval = putenv(cur->varstr);
			/* fall through to "free(free_me)" -
			 * only now we can free old exported malloced string
			 */
		}
	}
#endif /* !__U_BOOT__ */
	സ്വതന്ത്രം(free_me);

	handle_changed_special_names(cur->varstr);

	തിരികെ retval;
}

#ifndef __U_BOOT__
സ്ഥിതം പൂർണ്ണസംഖ്യ set_local_var0(അക്ഷരം *str)
{
	തിരികെ set_local_var(str, 0);
}

സ്ഥിതം ശൂന്യം FAST_FUNC set_local_var_from_halves(സ്ഥിരം അക്ഷരം *name, സ്ഥിരം അക്ഷരം *val)
{
	അക്ഷരം *var = xasprintf("%s=%s", name, val);
	set_local_var0(var);
}

/* Used at startup and after each cd */
സ്ഥിതം ശൂന്യം set_pwd_var(unsigned flag)
{
	set_local_var(xasprintf("PWD=%s", get_cwd(/*force:*/ 1)), flag);
}
#endif /* !__U_BOOT__ */

#എങ്കിൽ ENABLE_HUSH_UNSET || ENABLE_HUSH_GETOPTS
സ്ഥിതം പൂർണ്ണസംഖ്യ unset_local_var(സ്ഥിരം അക്ഷരം *name)
{
	ഘടന variable *cur;
	ഘടന variable **cur_pp;

	cur_pp = &G.മുകൾ_var;
	ആകുമ്പോൾ ((cur = *cur_pp) != ശൂന്യ) {
		എങ്കിൽ (varcmp(cur->varstr, name) == 0) {
			എങ്കിൽ (cur->flg_read_only) {
				bb_error_സന്ദേശം("%s: readonly variable", name);
				തിരികെ EXIT_FAILURE;
			}

			*cur_pp = cur->next;
			debug_printf_env("%s: unsetenv '%s'\n", __func__, cur->varstr);
			bb_unsetenv(cur->varstr);
			എങ്കിൽ (!cur->max_len)
				സ്വതന്ത്രം(cur->varstr);
			സ്വതന്ത്രം(cur);

			നിർത്തുക;
		}
		cur_pp = &cur->next;
	}

	/* Handle "unset LINENO" et al even if did not find the variable to unset */
	handle_changed_special_names(name);

	തിരികെ EXIT_SUCCESS;
}
#endif

#ifndef __U_BOOT__
/*
 * Helpers for "var1=val1 var2=val2 cmd" feature
 */
സ്ഥിതം ശൂന്യം add_vars(ഘടന variable *var)
{
	ഘടന variable *next;

	ആകുമ്പോൾ (var) {
		next = var->next;
		var->next = G.മുകൾ_var;
		G.മുകൾ_var = var;
		എങ്കിൽ (var->flg_export) {
			debug_printf_env("%s: restoring exported '%s'/%u\n", __func__, var->varstr, var->var_nest_level);
			putenv(var->varstr);
		} അല്ലെങ്കിൽ {
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
സ്ഥിതം ശൂന്യം set_vars_and_save_old(അക്ഷരം **strings)
{
	അക്ഷരം **s;

	എങ്കിൽ (!strings)
		തിരികെ;

	s = strings;
	ആകുമ്പോൾ (*s) {
		ഘടന variable *var_p;
		ഘടന variable **var_pp;
		അക്ഷരം *eq;

		eq = strchr(*s, '=');
		എങ്കിൽ (HUSH_DEBUG && !eq)
			bb_simple_error_സന്ദേശം_and_die("BUG in varexp4");
		var_pp = get_ptr_to_local_var(*s);
		എങ്കിൽ (var_pp) {
			var_p = *var_pp;
			എങ്കിൽ (var_p->flg_read_only) {
				അക്ഷരം **p;
				bb_error_സന്ദേശം("%s: readonly variable", *s);
				/*
				 * "VAR=V BLTIN" unsets VARs after BLTIN completes.
				 * If VAR is readonly, leaving it in the list
				 * after asssignment error (msg above)
				 * causes doubled error message later, on unset.
				 */
				debug_printf_env("removing/freeing '%s' element\n", *s);
				സ്വതന്ത്രം(*s);
				p = s;
				ചെയ്യുക { *p = p[1]; p++; } ആകുമ്പോൾ (*p);
				പോകുക next;
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
	സ്വതന്ത്രം(strings);
}

/*
 * Unicode helper
 */
സ്ഥിതം ശൂന്യം reinit_unicode_for_hush(ശൂന്യം)
{
	/* Unicode support should be activated even if LANG is set
	 * _during_ shell execution, not only if it was set when
	 * shell was started. Therefore, re-check LANG every time:
	 */
	എങ്കിൽ (ENABLE_FEATURE_CHECK_UNICODE_IN_ENV
	 || ENABLE_UNICODE_USING_LOCALE
	) {
		സ്ഥിരം അക്ഷരം *s = get_local_var_മൂല്യം("LC_ALL");
		എങ്കിൽ (!s) s = get_local_var_മൂല്യം("LC_CTYPE");
		എങ്കിൽ (!s) s = get_local_var_മൂല്യം("LANG");
		reinit_unicode(s);
	}
}

#endif /* !__U_BOOT__ */
/*
 * in_str support (strings, and "strings" read from files).
 */

#എങ്കിൽ ENABLE_HUSH_INTERACTIVE
#ifndef __U_BOOT__
/* To test correct lineedit/interactive behavior, type from command line:
 *	echo $P\
 *	\
 *	AT\
 *	H\
 *	\
 * It exercises a lot of corner cases.
 */
സ്ഥിതം സ്ഥിരം അക്ഷരം *setup_prompt_string(ശൂന്യം)
{
	സ്ഥിരം അക്ഷരം *prompt_str;

	debug_printf_prompt("%s promptmode:%d\n", __func__, G.promptmode);

# എങ്കിൽ ENABLE_FEATURE_EDITING_FANCY_PROMPT
	prompt_str = get_local_var_മൂല്യം(G.promptmode == 0 ? "PS1" : "PS2");
	എങ്കിൽ (!prompt_str)
		prompt_str = "";
# അല്ലെങ്കിൽ
	prompt_str = "> "; /* if PS2, else... */
	എങ്കിൽ (G.promptmode == 0) { /* PS1 */
		/* No fancy prompts supported, (re)generate "CURDIR $ " by hand */
		സ്വതന്ത്രം(G.PS1);
		/* bash uses $PWD value, even if it is set by user.
		 * It uses current dir only if PWD is unset.
		 * We always use current dir. */
		prompt_str = G.PS1 = xasprintf("%s %c ", get_cwd(0), (geteuid() != 0) ? '$' : '#');
	}
# endif
	debug_printf("prompt_str '%s'\n", prompt_str);
	തിരികെ prompt_str;
}
#endif /* !__U_BOOT__ */

#ifndef __U_BOOT__
സ്ഥിതം പൂർണ്ണസംഖ്യ get_user_നിവേശം(ഘടന in_str *i)
#അല്ലെങ്കിൽ /* __U_BOOT__ */
സ്ഥിതം ശൂന്യം get_user_നിവേശം(ഘടന in_str *i)
#endif /* __U_BOOT__ */
{
#ifndef __U_BOOT__
# എങ്കിൽ ENABLE_FEATURE_EDITING
	/* In EDITING case, this function reads next input line,
	 * saves it in i->p, then returns 1st char of it.
	 */
	പൂർണ്ണസംഖ്യ r;
	സ്ഥിരം അക്ഷരം *prompt_str;

	prompt_str = setup_prompt_string();
	വേണ്ടി (;;) {
		reinit_unicode_for_hush();
		G.flag_SIGINT = 0;

		bb_got_signal = 0;
		എങ്കിൽ (!sigisemptyset(&G.pending_set)) {
			/* Whoops, already got a signal, do not call read_line_input */
			bb_got_signal = r = -1;
		} അല്ലെങ്കിൽ {
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
			r = read_വരി_നിവേശം(G.വരി_നിവേശം_state, prompt_str,
				G.user_നിവേശം_buf, CONFIG_FEATURE_EDITING_MAX_LEN-1
			);
			/* read_line_input intercepts ^C, "convert" it to SIGINT */
			എങ്കിൽ (r == 0)
				ഉയർത്തുക(SIGINT);
		}
		/* bash prints ^C (before running a trap, if any)
		 * both on keyboard ^C and on real SIGINT (non-kbd generated).
		 */
		എങ്കിൽ (sigismember(&G.pending_set, SIGINT)) {
			ലിഖ(STDOUT_FILENO, "^C\n", 3);
			G.last_exitcode = 128 | SIGINT;
		}
		check_and_run_traps();
		എങ്കിൽ (r == 0) /* keyboard ^C? */
			തുടരുക; /* go back, read another input line */
		എങ്കിൽ (r > 0) /* normal input? (no ^C, no ^D, no signals) */
			നിർത്തുക;
		എങ്കിൽ (!bb_got_signal) {
			/* r < 0: ^D/EOF/error detected (but not signal) */
			/* ^D on interactive input goes to next line before exiting: */
			ലിഖ(STDOUT_FILENO, "\n", 1);
			i->p = ശൂന്യ;
			i->peek_buf[0] = r = EOF;
			തിരികെ r;
		}
		/* it was a signal: go back, read another input line */
	}
	i->p = G.user_നിവേശം_buf;
	തിരികെ (unsigned അക്ഷരം)*i->p++;
# അല്ലെങ്കിൽ
	/* In !EDITING case, this function gets called for every char.
	 * Buffering happens deeper in the call chain, in hfgetc(i->file).
	 */
	പൂർണ്ണസംഖ്യ r;

	വേണ്ടി (;;) {
		G.flag_SIGINT = 0;
		എങ്കിൽ (i->last_char == '\0' || i->last_char == '\n') {
			സ്ഥിരം അക്ഷരം *prompt_str = setup_prompt_string();
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
		എങ്കിൽ (r != '\0' && !G.flag_SIGINT)
			നിർത്തുക;
		എങ്കിൽ (G.flag_SIGINT) {
			/* ^C or SIGINT: repeat */
			/* bash prints ^C even on real SIGINT (non-kbd generated) */
			/* kernel prints "^C" itself, just print newline: */
			ലിഖ(STDOUT_FILENO, "\n", 1);
			G.last_exitcode = 128 | SIGINT;
		}
	}
	തിരികെ r;
# endif
#അല്ലെങ്കിൽ /* __U_BOOT__ */
	പൂർണ്ണസംഖ്യ n;
	പൂർണ്ണസംഖ്യ promptme;
	സ്ഥിതം അക്ഷരം the_command[CONFIG_SYS_CBSIZE + 1];

	bootretry_reset_cmd_timeout();
	promptme = 1;
	n = u_boot_cli_readline(i);

# ifdef CONFIG_BOOT_RETRY_TIME
	എങ്കിൽ (n == -2) {
		puts("\nTimeout waiting for command\n");
#  ifdef CONFIG_RESET_TO_RETRY
		do_reset(ശൂന്യ, 0, 0, ശൂന്യ);
#  elif IS_ENABLED(CONFIG_RETRY_BOOTCMD)
	strcpy(console_buffer, "run bootcmd\n");
# അല്ലെങ്കിൽ
#	error "This only works with CONFIG_RESET_TO_RETRY or CONFIG_BOOT_RETRY_COMMAND enabled"
#  endif
	}
# endif
	എങ്കിൽ (n == -1 ) {
		G.flag_repeat = 0;
		promptme = 0;
	}
	n = strlen(console_buffer);
	console_buffer[n] = '\n';
	console_buffer[n+1]= '\0';
	എങ്കിൽ (had_ctrlc())
		G.flag_repeat = 0;
	clear_ctrlc();
	G.do_repeat = 0;
#ifndef __U_BOOT__
	എങ്കിൽ (G.promptmode == 1) {
#അല്ലെങ്കിൽ /* __U_BOOT__ */
	എങ്കിൽ (!G.promptmode) {
#endif /* __U_BOOT__ */
		എങ്കിൽ (console_buffer[0] == '\n'&& G.flag_repeat == 0) {
			strcpy(the_command, console_buffer);
		}
		അല്ലെങ്കിൽ {
			എങ്കിൽ (console_buffer[0] != '\n') {
				strcpy(the_command, console_buffer);
				G.flag_repeat = 1;
			}
			അല്ലെങ്കിൽ {
				G.do_repeat = 1;
			}
		}
		i->p = the_command;
	}
	അല്ലെങ്കിൽ {
		എങ്കിൽ (console_buffer[0] != '\n') {
			എങ്കിൽ (strlen(the_command) + strlen(console_buffer)
				< CONFIG_SYS_CBSIZE) {
				n = strlen(the_command);
#ifdef __U_BOOT__
				/*
				 * To avoid writing to bad places, we check if
				 * n is greater than 0.
				 * This bug was found by Harald Seiler.
				 */
				എങ്കിൽ (n > 0)
					the_command[n-1] = ' ';
				strcpy(&the_command[n], console_buffer);
#അല്ലെങ്കിൽ /* !__U_BOOT__ */
			the_command[n-1] = ' ';
			strcpy(&the_command[n], console_buffer);
#endif /* !__U_BOOT__ */
				}
				അല്ലെങ്കിൽ {
					the_command[0] = '\n';
					the_command[1] = '\0';
					G.flag_repeat = 0;
				}
		}
		എങ്കിൽ (promptme == 0) {
			the_command[0] = '\n';
			the_command[1] = '\0';
		}
		i->p = console_buffer;
	}
#endif /* __U_BOOT__ */
}
/* This is the magic location that prints prompts
 * and gets data back from the user */
സ്ഥിതം പൂർണ്ണസംഖ്യ fgetc_interactive(ഘടന in_str *i)
{
	പൂർണ്ണസംഖ്യ ch;
#ifndef __U_BOOT__
	/* If it's interactive stdin, get new line. */
	എങ്കിൽ (G_interactive_fd && i->file == G.HFILE_stdin) {
#endif /* !__U_BOOT__ */
#ifndef __U_BOOT__
		/* Returns first char (or EOF), the rest is in i->p[] */
		ch = get_user_നിവേശം(i);
#അല്ലെങ്കിൽ /* __U_BOOT__ */
		/* Avoid garbage value and make clang happy. */
		ch = 0;
		/*
		 * get_user_input() does not return anything when used in
		 * U-Boot.
		 * So, we need to take the read character from i->p[].
		 */
		get_user_നിവേശം(i);
		എങ്കിൽ (i->p && *i->p) {
			ch = *i->p++;
		}
#endif /* __U_BOOT__ */
		G.promptmode = 1; /* PS2 */
		debug_printf_prompt("%s promptmode=%d\n", __func__, G.promptmode);
#ifndef __U_BOOT__
	} അല്ലെങ്കിൽ {
		/* Not stdin: script file, sourced file, etc */
		ചെയ്യുക ch = hfgetc(i->file); ആകുമ്പോൾ (ch == '\0');
	}
#endif /* !__U_BOOT__ */
	തിരികെ ch;
}
#അല്ലെങ്കിൽ  /* !INTERACTIVE */
#ifndef __U_BOOT__
സ്ഥിതം ALWAYS_INLINE പൂർണ്ണസംഖ്യ fgetc_interactive(ഘടന in_str *i)
{
	പൂർണ്ണസംഖ്യ ch;
	ചെയ്യുക ch = hfgetc(i->file); ആകുമ്പോൾ (ch == '\0');
	തിരികെ ch;
}
#endif /* !__U_BOOT__ */
#endif  /* !INTERACTIVE */

സ്ഥിതം പൂർണ്ണസംഖ്യ i_getch(ഘടന in_str *i)
{
	പൂർണ്ണസംഖ്യ ch;

#ifndef __U_BOOT__
	എങ്കിൽ (!i->file) {
		/* string-based in_str */
		ch = (unsigned അക്ഷരം)*i->p;
		എങ്കിൽ (ch != '\0') {
			i->p++;
			i->last_char = ch;
#എങ്കിൽ ENABLE_HUSH_LINENO_VAR
			എങ്കിൽ (ch == '\n') {
				G.parse_lineno++;
				debug_printf_parse("G.parse_lineno++ = %u\n", G.parse_lineno);
			}
#endif
			തിരികെ ch;
		}
		തിരികെ EOF;
	}

#endif /* !__U_BOOT__ */
	/* FILE-based in_str */

#എങ്കിൽ ENABLE_FEATURE_EDITING
	/* This can be stdin, check line editing char[] buffer */
	എങ്കിൽ (i->p && *i->p != '\0') {
		ch = (unsigned അക്ഷരം)*i->p++;
		പോകുക out;
#ifndef __U_BOOT__
	}
#അല്ലെങ്കിൽ /* __U_BOOT__ */
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
	} അല്ലെങ്കിൽ എങ്കിൽ (i->p && *i->p == '\0' && G.run_command_flags){
		തിരികെ EOF;
	}
#endif /* __U_BOOT__ */
#endif
#ifndef __U_BOOT__
	/* peek_buf[] is an int array, not char. Can contain EOF. */
	ch = i->peek_buf[0];
	എങ്കിൽ (ch != 0) {
		പൂർണ്ണസംഖ്യ ch2 = i->peek_buf[1];
		i->peek_buf[0] = ch2;
		എങ്കിൽ (ch2 == 0) /* very likely, avoid redundant write */
			പോകുക out;
		i->peek_buf[1] = 0;
		പോകുക out;
	}

#endif /* !__U_BOOT__ */
	ch = fgetc_interactive(i);
 out:
	debug_printf("file_get: got '%c' %d\n", ch, ch);
	i->last_char = ch;
#എങ്കിൽ ENABLE_HUSH_LINENO_VAR
	എങ്കിൽ (ch == '\n') {
		G.parse_lineno++;
		debug_printf_parse("G.parse_lineno++ = %u\n", G.parse_lineno);
	}
#endif
	തിരികെ ch;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ i_peek(ഘടന in_str *i)
{
#ifndef __U_BOOT__
	പൂർണ്ണസംഖ്യ ch;

	എങ്കിൽ (!i->file) {
		/* string-based in_str */
		/* Doesn't report EOF on NUL. None of the callers care. */
		തിരികെ (unsigned അക്ഷരം)*i->p;
	}

	/* FILE-based in_str */

#എങ്കിൽ ENABLE_FEATURE_EDITING && ENABLE_HUSH_INTERACTIVE
	/* This can be stdin, check line editing char[] buffer */
	എങ്കിൽ (i->p && *i->p != '\0')
		തിരികെ (unsigned അക്ഷരം)*i->p;
#endif
	/* peek_buf[] is an int array, not char. Can contain EOF. */
	ch = i->peek_buf[0];
	എങ്കിൽ (ch != 0)
		തിരികെ ch;

	/* Need to get a new char */
	ch = fgetc_interactive(i);
	debug_printf("file_peek: got '%c' %d\n", ch, ch);

	/* Save it by either rolling back line editing buffer, or in i->peek_buf[0] */
#എങ്കിൽ ENABLE_FEATURE_EDITING && ENABLE_HUSH_INTERACTIVE
	എങ്കിൽ (i->p) {
		i->p -= 1;
		തിരികെ ch;
	}
#endif
	i->peek_buf[0] = ch;
	/*i->peek_buf[1] = 0; - already is */
	തിരികെ ch;
#അല്ലെങ്കിൽ /* __U_BOOT__ */
	/* string-based in_str */
	/* Doesn't report EOF on NUL. None of the callers care. */
	തിരികെ (unsigned അക്ഷരം)*i->p;
#endif /* __U_BOOT__ */
}

/* Only ever called if i_peek() was called, and did not return EOF.
 * IOW: we know the previous peek saw an ordinary char, not EOF, not NUL,
 * not end-of-line. Therefore we never need to read a new editing line here.
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ i_peek2(ഘടന in_str *i)
{
#ifndef __U_BOOT__
	പൂർണ്ണസംഖ്യ ch;
#endif /* !__U_BOOT__ */

	/* There are two cases when i->p[] buffer exists.
	 * (1) it's a string in_str.
	 * (2) It's a file, and we have a saved line editing buffer.
	 * In both cases, we know that i->p[0] exists and not NUL, and
	 * the peek2 result is in i->p[1].
	 */
	എങ്കിൽ (i->p)
		തിരികെ (unsigned അക്ഷരം)i->p[1];

#ifndef __U_BOOT__
	/* Now we know it is a file-based in_str. */

	/* peek_buf[] is an int array, not char. Can contain EOF. */
	/* Is there 2nd char? */
	ch = i->peek_buf[1];
	എങ്കിൽ (ch == 0) {
		/* We did not read it yet, get it now */
		ചെയ്യുക ch = hfgetc(i->file); ആകുമ്പോൾ (ch == '\0');
		i->peek_buf[1] = ch;
	}

	debug_printf("file_peek2: got '%c' %d\n", ch, ch);
	തിരികെ ch;
#അല്ലെങ്കിൽ
	തിരികെ 0;
#endif /* __U_BOOT__ */
}

സ്ഥിതം പൂർണ്ണസംഖ്യ i_getch_and_eat_bkslash_nl(ഘടന in_str *നിവേശം)
{
	വേണ്ടി (;;) {
		പൂർണ്ണസംഖ്യ ch, ch2;

		ch = i_getch(നിവേശം);
		എങ്കിൽ (ch != '\\')
			തിരികെ ch;
		ch2 = i_peek(നിവേശം);
		എങ്കിൽ (ch2 != '\n')
			തിരികെ ch;
		/* backslash+newline, skip it */
		i_getch(നിവേശം);
	}
}

/* Note: this function _eats_ \<newline> pairs, safe to use plain
 * i_getch() after it instead of i_getch_and_eat_bkslash_nl().
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ i_peek_and_eat_bkslash_nl(ഘടന in_str *നിവേശം)
{
	വേണ്ടി (;;) {
		പൂർണ്ണസംഖ്യ ch, ch2;

		ch = i_peek(നിവേശം);
		എങ്കിൽ (ch != '\\')
			തിരികെ ch;
		ch2 = i_peek2(നിവേശം);
		എങ്കിൽ (ch2 != '\n')
			തിരികെ ch;
		/* backslash+newline, skip it */
		i_getch(നിവേശം);
		i_getch(നിവേശം);
	}
}

#ifndef __U_BOOT__
സ്ഥിതം ശൂന്യം setup_file_in_str(ഘടന in_str *i, HFILE *fp)
#അല്ലെങ്കിൽ /* __U_BOOT__ */
സ്ഥിതം ശൂന്യം setup_file_in_str(ഘടന in_str *i)
#endif /* __U_BOOT__ */
{
	memset(i, 0, വലുപ്പം(*i));
#ifndef __U_BOOT__
	i->file = fp;
	/* i->p = NULL; */
#endif /* !__U_BOOT__ */
}

സ്ഥിതം ശൂന്യം setup_string_in_str(ഘടന in_str *i, സ്ഥിരം അക്ഷരം *s)
{
	memset(i, 0, വലുപ്പം(*i));
	/*i->file = NULL */;
	i->p = s;
}

/*
 * o_string support
 */
#define B_CHUNK  (32 * വലുപ്പം(അക്ഷരം*))

സ്ഥിതം ശൂന്യം o_reset_to_empty_unquoted(o_string *o)
{
	o->length = 0;
	o->has_quoted_part = 0;
	എങ്കിൽ (o->data)
		o->data[0] = '\0';
}

സ്ഥിതം ശൂന്യം o_free_and_set_NULL(o_string *o)
{
	സ്വതന്ത്രം(o->data);
	memset(o, 0, വലുപ്പം(*o));
}

സ്ഥിതം ALWAYS_INLINE ശൂന്യം o_free(o_string *o)
{
	സ്വതന്ത്രം(o->data);
}

സ്ഥിതം ശൂന്യം o_grow_by(o_string *o, പൂർണ്ണസംഖ്യ len)
{
	എങ്കിൽ (o->length + len > o->maxlen) {
		o->maxlen += (2 * len) | (B_CHUNK-1);
		o->data = xrealloc(o->data, 1 + o->maxlen);
	}
}

സ്ഥിതം ശൂന്യം o_addchr(o_string *o, പൂർണ്ണസംഖ്യ ch)
{
	debug_printf("o_addchr: '%c' o->length=%d o=%p\n", ch, o->length, o);
	എങ്കിൽ (o->length < o->maxlen) {
		/* likely. avoid o_grow_by() call */
 add:
		o->data[o->length] = ch;
		o->length++;
		o->data[o->length] = '\0';
		തിരികെ;
	}
	o_grow_by(o, 1);
	പോകുക add;
}

#എങ്കിൽ 0
/* Valid only if we know o_string is not empty */
സ്ഥിതം ശൂന്യം o_delchr(o_string *o)
{
	o->length--;
	o->data[o->length] = '\0';
}
#endif

സ്ഥിതം ശൂന്യം o_addblock(o_string *o, സ്ഥിരം അക്ഷരം *str, പൂർണ്ണസംഖ്യ len)
{
	o_grow_by(o, len);
	((അക്ഷരം*)mempcpy(&o->data[o->length], str, len))[0] = '\0';
	o->length += len;
}

സ്ഥിതം ശൂന്യം o_addstr(o_string *o, സ്ഥിരം അക്ഷരം *str)
{
	o_addblock(o, str, strlen(str));
}

#ifndef __U_BOOT__
സ്ഥിതം ശൂന്യം o_addstr_with_NUL(o_string *o, സ്ഥിരം അക്ഷരം *str)
{
	o_addblock(o, str, strlen(str) + 1);
}
#endif /* !__U_BOOT__ */

#എങ്കിൽ !BB_MMU
സ്ഥിതം ശൂന്യം nommu_addchr(o_string *o, പൂർണ്ണസംഖ്യ ch)
{
	എങ്കിൽ (o)
		o_addchr(o, ch);
}
#അല്ലെങ്കിൽ
# define nommu_addchr(o, str) ((ശൂന്യം)0)
#endif

#ifndef __U_BOOT__
#എങ്കിൽ ENABLE_HUSH_MODE_X
സ്ഥിതം ശൂന്യം x_mode_addchr(പൂർണ്ണസംഖ്യ ch)
{
	o_addchr(&G.x_mode_buf, ch);
}
സ്ഥിതം ശൂന്യം x_mode_addstr(സ്ഥിരം അക്ഷരം *str)
{
	o_addstr(&G.x_mode_buf, str);
}
സ്ഥിതം ശൂന്യം x_mode_addblock(സ്ഥിരം അക്ഷരം *str, പൂർണ്ണസംഖ്യ len)
{
	o_addblock(&G.x_mode_buf, str, len);
}
സ്ഥിതം ശൂന്യം x_mode_prefix(ശൂന്യം)
{
	പൂർണ്ണസംഖ്യ n = G.x_mode_depth;
	ചെയ്യുക x_mode_addchr('+'); ആകുമ്പോൾ (--n >= 0);
}
സ്ഥിതം ശൂന്യം x_mode_flush(ശൂന്യം)
{
	പൂർണ്ണസംഖ്യ len = G.x_mode_buf.length;
	എങ്കിൽ (len <= 0)
		തിരികെ;
	എങ്കിൽ (G.x_mode_fd > 0) {
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

#എങ്കിൽ ENABLE_HUSH_BRACE_EXPANSION
# define MAYBE_BRACES "{}"
#അല്ലെങ്കിൽ
# define MAYBE_BRACES ""
#endif

/* My analysis of quoting semantics tells me that state information
 * is associated with a destination, not a source.
 */
സ്ഥിതം ശൂന്യം o_addqchr(o_string *o, പൂർണ്ണസംഖ്യ ch)
{
	പൂർണ്ണസംഖ്യ sz = 1;
	/* '-' is included because of this case:
	 * >filename0 >filename1 >filename9; v='-'; echo filename[0"$v"9]
	 */
	അക്ഷരം *found = strchr("*?[-\\" MAYBE_BRACES, ch);
	എങ്കിൽ (found)
		sz++;
	o_grow_by(o, sz);
	എങ്കിൽ (found) {
		o->data[o->length] = '\\';
		o->length++;
	}
	o->data[o->length] = ch;
	o->length++;
	o->data[o->length] = '\0';
}

സ്ഥിതം ശൂന്യം o_addQchr(o_string *o, പൂർണ്ണസംഖ്യ ch)
{
	പൂർണ്ണസംഖ്യ sz = 1;
	എങ്കിൽ ((o->o_expflags & EXP_FLAG_ESC_GLOB_CHARS)
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

സ്ഥിതം ശൂന്യം o_addqblock(o_string *o, സ്ഥിരം അക്ഷരം *str, പൂർണ്ണസംഖ്യ len)
{
	ആകുമ്പോൾ (len) {
		അക്ഷരം ch;
		പൂർണ്ണസംഖ്യ sz;
		പൂർണ്ണസംഖ്യ ordinary_cnt = strcspn(str, "*?[-\\" MAYBE_BRACES);
		എങ്കിൽ (ordinary_cnt > len) /* paranoia */
			ordinary_cnt = len;
		o_addblock(o, str, ordinary_cnt);
		എങ്കിൽ (ordinary_cnt == len)
			തിരികെ; /* NUL is already added by o_addblock */
		str += ordinary_cnt;
		len -= ordinary_cnt + 1; /* we are processing + 1 char below */

		ch = *str++;
		sz = 1;
		എങ്കിൽ (ch) { /* it is necessarily one of "*?[-\\" MAYBE_BRACES */
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

സ്ഥിതം ശൂന്യം o_addQblock(o_string *o, സ്ഥിരം അക്ഷരം *str, പൂർണ്ണസംഖ്യ len)
{
	എങ്കിൽ (!(o->o_expflags & EXP_FLAG_ESC_GLOB_CHARS)) {
		o_addblock(o, str, len);
		തിരികെ;
	}
	o_addqblock(o, str, len);
}

സ്ഥിതം ശൂന്യം o_addQstr(o_string *o, സ്ഥിരം അക്ഷരം *str)
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
#എങ്കിൽ DEBUG_EXPAND || DEBUG_GLOB
സ്ഥിതം ശൂന്യം debug_print_list(സ്ഥിരം അക്ഷരം *prefix, o_string *o, പൂർണ്ണസംഖ്യ n)
{
	അക്ഷരം **list = (അക്ഷരം**)o->data;
	പൂർണ്ണസംഖ്യ string_start = ((n + 0xf) & ~0xf) * വലുപ്പം(list[0]);
	പൂർണ്ണസംഖ്യ i = 0;

	indent();
	fdprintf(2, "%s: list:%p n:%d string_start:%d length:%d maxlen:%d glob:%d quoted:%d escape:%d\n",
			prefix, list, n, string_start, o->length, o->maxlen,
			!!(o->o_expflags & EXP_FLAG_GLOB),
			o->has_quoted_part,
			!!(o->o_expflags & EXP_FLAG_ESC_GLOB_CHARS));
	ആകുമ്പോൾ (i < n) {
		indent();
		fdprintf(2, " list[%d]=%d '%s' %p\n", i, (പൂർണ്ണസംഖ്യ)(uintptr_t)list[i],
				o->data + (പൂർണ്ണസംഖ്യ)(uintptr_t)list[i] + string_start,
				o->data + (പൂർണ്ണസംഖ്യ)(uintptr_t)list[i] + string_start);
		i++;
	}
	എങ്കിൽ (n) {
		സ്ഥിരം അക്ഷരം *p = o->data + (പൂർണ്ണസംഖ്യ)(uintptr_t)list[n - 1] + string_start;
		indent();
#ifndef __U_BOOT__
		fdprintf(2, " total_sz:%ld\n", (long)((p + strlen(p) + 1) - o->data));
#അല്ലെങ്കിൽ /* __U_BOOT__ */
		printf(" total_sz:%ld\n", (long)((p + strlen(p) + 1) - o->data));
#endif /* __U_BOOT__ */
	}
}
#അല്ലെങ്കിൽ
# define debug_print_list(prefix, o, n) ((ശൂന്യം)0)
#endif

/* n = o_save_ptr_helper(str, n) "starts new string" by storing an index value
 * in list[n] so that it points past last stored byte so far.
 * It returns n+1. */
സ്ഥിതം പൂർണ്ണസംഖ്യ o_save_ptr_helper(o_string *o, പൂർണ്ണസംഖ്യ n)
{
	അക്ഷരം **list = (അക്ഷരം**)o->data;
	പൂർണ്ണസംഖ്യ string_start;
	പൂർണ്ണസംഖ്യ string_len;

	എങ്കിൽ (!o->has_empty_slot) {
		string_start = ((n + 0xf) & ~0xf) * വലുപ്പം(list[0]);
		string_len = o->length - string_start;
		എങ്കിൽ (!(n & 0xf)) { /* 0, 0x10, 0x20...? */
			debug_printf_list("list[%d]=%d string_start=%d (growing)\n", n, string_len, string_start);
			/* list[n] points to string_start, make space for 16 more pointers */
			o->maxlen += 0x10 * വലുപ്പം(list[0]);
			o->data = xrealloc(o->data, o->maxlen + 1);
			list = (അക്ഷരം**)o->data;
			memmove(list + n + 0x10, list + n, string_len);
			/*
			 * expand_on_ifs() has a "previous argv[] ends in IFS?"
			 * check. (grep for -prev-ifs-check-).
			 * Ensure that argv[-1][last] is not garbage
			 * but zero bytes, to save index check there.
			 */
			list[n + 0x10 - 1] = 0;
			o->length += 0x10 * വലുപ്പം(list[0]);
		} അല്ലെങ്കിൽ {
			debug_printf_list("list[%d]=%d string_start=%d\n",
					n, string_len, string_start);
		}
	} അല്ലെങ്കിൽ {
		/* We have empty slot at list[n], reuse without growth */
		string_start = ((n+1 + 0xf) & ~0xf) * വലുപ്പം(list[0]); /* NB: n+1! */
		string_len = o->length - string_start;
		debug_printf_list("list[%d]=%d string_start=%d (empty slot)\n",
				n, string_len, string_start);
		o->has_empty_slot = 0;
	}
	o->has_quoted_part = 0;
	list[n] = (അക്ഷരം*)(uintptr_t)string_len;
	തിരികെ n + 1;
}

/* "What was our last o_save_ptr'ed position (byte offset relative o->data)?" */
സ്ഥിതം പൂർണ്ണസംഖ്യ o_get_last_ptr(o_string *o, പൂർണ്ണസംഖ്യ n)
{
	അക്ഷരം **list = (അക്ഷരം**)o->data;
	പൂർണ്ണസംഖ്യ string_start = ((n + 0xf) & ~0xf) * വലുപ്പം(list[0]);

	തിരികെ ((പൂർണ്ണസംഖ്യ)(uintptr_t)list[n-1]) + string_start;
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

#എങ്കിൽ ENABLE_HUSH_BRACE_EXPANSION
/* There in a GNU extension, GLOB_BRACE, but it is not usable:
 * first, it processes even {a} (no commas), second,
 * I didn't manage to make it return strings when they don't match
 * existing files. Need to re-implement it.
 */

/* Helper */
സ്ഥിതം പൂർണ്ണസംഖ്യ glob_needed(സ്ഥിരം അക്ഷരം *s)
{
	ആകുമ്പോൾ (*s) {
		എങ്കിൽ (*s == '\\') {
			എങ്കിൽ (!s[1])
				തിരികെ 0;
			s += 2;
			തുടരുക;
		}
		എങ്കിൽ (*s == '*' || *s == '[' || *s == '?' || *s == '{')
			തിരികെ 1;
		s++;
	}
	തിരികെ 0;
}
/* Return pointer to next closing brace or to comma */
സ്ഥിതം സ്ഥിരം അക്ഷരം *next_brace_sub(സ്ഥിരം അക്ഷരം *cp)
{
	unsigned depth = 0;
	cp++;
	ആകുമ്പോൾ (*cp != '\0') {
		എങ്കിൽ (*cp == '\\') {
			എങ്കിൽ (*++cp == '\0')
				നിർത്തുക;
			cp++;
			തുടരുക;
		}
		എങ്കിൽ ((*cp == '}' && depth-- == 0) || (*cp == ',' && depth == 0))
			നിർത്തുക;
		എങ്കിൽ (*cp++ == '{')
			depth++;
	}

	തിരികെ *cp != '\0' ? cp : ശൂന്യ;
}
/* Recursive brace globber. Note: may garble pattern[]. */
സ്ഥിതം പൂർണ്ണസംഖ്യ glob_brace(അക്ഷരം *pattern, o_string *o, പൂർണ്ണസംഖ്യ n)
{
	അക്ഷരം *new_pattern_buf;
	സ്ഥിരം അക്ഷരം *begin;
	സ്ഥിരം അക്ഷരം *next;
	സ്ഥിരം അക്ഷരം *rest;
	സ്ഥിരം അക്ഷരം *p;
	size_t rest_len;

	debug_printf_glob("glob_brace('%s')\n", pattern);

	begin = pattern;
	ആകുമ്പോൾ (1) {
		എങ്കിൽ (*begin == '\0')
			പോകുക simple_glob;
		എങ്കിൽ (*begin == '{') {
			/* Find the first sub-pattern and at the same time
			 * find the rest after the closing brace */
			next = next_brace_sub(begin);
			എങ്കിൽ (next == ശൂന്യ) {
				/* An illegal expression */
				പോകുക simple_glob;
			}
			എങ്കിൽ (*next == '}') {
				/* "{abc}" with no commas - illegal
				 * brace expr, disregard and skip it */
				begin = next + 1;
				തുടരുക;
			}
			നിർത്തുക;
		}
		എങ്കിൽ (*begin == '\\' && begin[1] != '\0')
			begin++;
		begin++;
	}
	debug_printf_glob("begin:%s\n", begin);
	debug_printf_glob("next:%s\n", next);

	/* Now find the end of the whole brace expression */
	rest = next;
	ആകുമ്പോൾ (*rest != '}') {
		rest = next_brace_sub(rest);
		എങ്കിൽ (rest == ശൂന്യ) {
			/* An illegal expression */
			പോകുക simple_glob;
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
	ആകുമ്പോൾ (1) {
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
		എങ്കിൽ (*next == '}') {
			/* We saw the last entry */
			നിർത്തുക;
		}
		p = next + 1;
		next = next_brace_sub(next);
	}
	സ്വതന്ത്രം(new_pattern_buf);
	തിരികെ n;

 simple_glob:
	{
		പൂർണ്ണസംഖ്യ gr;
		glob_t globdata;

		memset(&globdata, 0, വലുപ്പം(globdata));
		gr = glob(pattern, 0, ശൂന്യ, &globdata);
		debug_printf_glob("glob('%s'):%d\n", pattern, gr);
		എങ്കിൽ (gr != 0) {
			എങ്കിൽ (gr == GLOB_NOMATCH) {
				globfree(&globdata);
				/* NB: garbles parameter */
				unbackslash(pattern);
				o_addstr_with_NUL(o, pattern);
				debug_printf_glob("glob pattern '%s' is literal\n", pattern);
				തിരികെ o_save_ptr_helper(o, n);
			}
			എങ്കിൽ (gr == GLOB_NOSPACE)
				bb_die_memory_exhausted();
			/* GLOB_ABORTED? Only happens with GLOB_ERR flag,
			 * but we didn't specify it. Paranoia again. */
			bb_error_സന്ദേശം_and_die("glob error %d on '%s'", gr, pattern);
		}
		എങ്കിൽ (globdata.gl_pathv && globdata.gl_pathv[0]) {
			അക്ഷരം **argv = globdata.gl_pathv;
			ആകുമ്പോൾ (1) {
				o_addstr_with_NUL(o, *argv);
				n = o_save_ptr_helper(o, n);
				argv++;
				എങ്കിൽ (!*argv)
					നിർത്തുക;
			}
		}
		globfree(&globdata);
	}
	തിരികെ n;
}
/* Performs globbing on last list[],
 * saving each result as a new list[].
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ perform_glob(o_string *o, പൂർണ്ണസംഖ്യ n)
{
	അക്ഷരം *pattern, *copy;

	debug_printf_glob("start perform_glob: n:%d o->data:%p\n", n, o->data);
	എങ്കിൽ (!o->data)
		തിരികെ o_save_ptr_helper(o, n);
	pattern = o->data + o_get_last_ptr(o, n);
	debug_printf_glob("glob pattern '%s'\n", pattern);
	എങ്കിൽ (!glob_needed(pattern)) {
		/* unbackslash last string in o in place, fix length */
		o->length = unbackslash(pattern) - o->data;
		debug_printf_glob("glob pattern '%s' is literal\n", pattern);
		തിരികെ o_save_ptr_helper(o, n);
	}

	copy = xstrdup(pattern);
	/* "forget" pattern in o */
	o->length = pattern - o->data;
	n = glob_brace(copy, o, n);
	സ്വതന്ത്രം(copy);
	എങ്കിൽ (DEBUG_GLOB)
		debug_print_list("perform_glob returning", o, n);
	തിരികെ n;
}

#അല്ലെങ്കിൽ /* !HUSH_BRACE_EXPANSION */

/* Helper */
സ്ഥിതം പൂർണ്ണസംഖ്യ glob_needed(സ്ഥിരം അക്ഷരം *s)
{
	ആകുമ്പോൾ (*s) {
		എങ്കിൽ (*s == '\\') {
			എങ്കിൽ (!s[1])
				തിരികെ 0;
			s += 2;
			തുടരുക;
		}
		എങ്കിൽ (*s == '*' || *s == '[' || *s == '?')
			തിരികെ 1;
		s++;
	}
	തിരികെ 0;
}
/* Performs globbing on last list[],
 * saving each result as a new list[].
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ perform_glob(o_string *o, പൂർണ്ണസംഖ്യ n)
{
#ifndef __U_BOOT__
	glob_t globdata;
	പൂർണ്ണസംഖ്യ gr;
#endif /* __U_BOOT__ */
	അക്ഷരം *pattern;

	debug_printf_glob("start perform_glob: n:%d o->data:%p\n", n, o->data);
	എങ്കിൽ (!o->data)
		തിരികെ o_save_ptr_helper(o, n);
	pattern = o->data + o_get_last_ptr(o, n);
	debug_printf_glob("glob pattern '%s'\n", pattern);
	എങ്കിൽ (!glob_needed(pattern)) {
#ifndef __U_BOOT__
 literal:
#endif /* __U_BOOT__ */
		/* unbackslash last string in o in place, fix length */
		o->length = unbackslash(pattern) - o->data;
		debug_printf_glob("glob pattern '%s' is literal\n", pattern);
		തിരികെ o_save_ptr_helper(o, n);
	}

#ifndef __U_BOOT__
	memset(&globdata, 0, വലുപ്പം(globdata));
	/* Can't use GLOB_NOCHECK: it does not unescape the string.
	 * If we glob "*.\*" and don't find anything, we need
	 * to fall back to using literal "*.*", but GLOB_NOCHECK
	 * will return "*.\*"!
	 */
	gr = glob(pattern, 0, ശൂന്യ, &globdata);
	debug_printf_glob("glob('%s'):%d\n", pattern, gr);
	എങ്കിൽ (gr != 0) {
		എങ്കിൽ (gr == GLOB_NOMATCH) {
			globfree(&globdata);
			പോകുക literal;
		}
		എങ്കിൽ (gr == GLOB_NOSPACE)
			bb_die_memory_exhausted();
		/* GLOB_ABORTED? Only happens with GLOB_ERR flag,
		 * but we didn't specify it. Paranoia again. */
		bb_error_സന്ദേശം_and_die("glob error %d on '%s'", gr, pattern);
	}
	എങ്കിൽ (globdata.gl_pathv && globdata.gl_pathv[0]) {
		അക്ഷരം **argv = globdata.gl_pathv;
		/* "forget" pattern in o */
		o->length = pattern - o->data;
		ആകുമ്പോൾ (1) {
			o_addstr_with_NUL(o, *argv);
			n = o_save_ptr_helper(o, n);
			argv++;
			എങ്കിൽ (!*argv)
				നിർത്തുക;
		}
	}
	globfree(&globdata);
	എങ്കിൽ (DEBUG_GLOB)
		debug_print_list("perform_glob returning", o, n);
	തിരികെ n;
#അല്ലെങ്കിൽ /* __U_BOOT__ */
	/*
	 * NOTE We only use perform glob to call unbackslash to remove backslash
	 * from string once expanded.
	 * So, it seems OK to return this if no previous return was done.
	 */
	തിരികെ o_save_ptr_helper(o, n);
#endif /* __U_BOOT__ */
}

#endif /* !HUSH_BRACE_EXPANSION */

/* If o->o_expflags & EXP_FLAG_GLOB, glob the string so far remembered.
 * Otherwise, just finish current list[] and start new */
സ്ഥിതം പൂർണ്ണസംഖ്യ o_save_ptr(o_string *o, പൂർണ്ണസംഖ്യ n)
{
	എങ്കിൽ (o->o_expflags & EXP_FLAG_GLOB) {
		/* If o->has_empty_slot, list[n] was already globbed
		 * (if it was requested back then when it was filled)
		 * so don't do that again! */
		എങ്കിൽ (!o->has_empty_slot)
			തിരികെ perform_glob(o, n); /* o_save_ptr_helper is inside */
	}
	തിരികെ o_save_ptr_helper(o, n);
}

/* "Please convert list[n] to real char* ptrs, and NULL terminate it." */
സ്ഥിതം അക്ഷരം **o_finalize_list(o_string *o, പൂർണ്ണസംഖ്യ n)
{
	അക്ഷരം **list;
	പൂർണ്ണസംഖ്യ string_start;

	എങ്കിൽ (DEBUG_EXPAND)
		debug_print_list("finalized", o, n);
	debug_printf_expand("finalized n:%d\n", n);
	list = (അക്ഷരം**)o->data;
	string_start = ((n + 0xf) & ~0xf) * വലുപ്പം(list[0]);
	list[--n] = ശൂന്യ;
	ആകുമ്പോൾ (n) {
		n--;
		list[n] = o->data + (പൂർണ്ണസംഖ്യ)(uintptr_t)list[n] + string_start;
	}
	തിരികെ list;
}

സ്ഥിതം ശൂന്യം free_pipe_list(ഘടന pipe *pi);

/* Returns pi->next - next pipe in the list */
സ്ഥിതം ഘടന pipe *free_pipe(ഘടന pipe *pi)
{
	ഘടന pipe *next;
	പൂർണ്ണസംഖ്യ i;

#ifndef __U_BOOT__
	debug_printf_clean("free_pipe (pid %d)\n", getpid());
#endif /* !__U_BOOT__ */
	വേണ്ടി (i = 0; i < pi->അക്കം_cmds; i++) {
		ഘടന command *command;
#ifndef __U_BOOT__
		ഘടന redir_struct *r, *rnext;
#endif /* !__U_BOOT__ */

		command = &pi->cmds[i];
		debug_printf_clean("  command %d:\n", i);
		എങ്കിൽ (command->argv) {
			എങ്കിൽ (DEBUG_CLEAN) {
				പൂർണ്ണസംഖ്യ a;
				അക്ഷരം **p;
				വേണ്ടി (a = 0, p = command->argv; *p; a++, p++) {
					debug_printf_clean("   argv[%d] = %s\n", a, *p);
				}
			}
			free_strings(command->argv);
			//command->argv = NULL;
		}
		/* not "else if": on syntax error, we may have both! */
		എങ്കിൽ (command->group) {
			debug_printf_clean("   begin group (cmd_type:%d)\n",
					command->cmd_type);
			free_pipe_list(command->group);
			debug_printf_clean("   end group\n");
			//command->group = NULL;
		}
		/* else is crucial here.
		 * If group != NULL, child_func is meaningless */
#എങ്കിൽ ENABLE_HUSH_FUNCTIONS
		അല്ലെങ്കിൽ എങ്കിൽ (command->child_func) {
			debug_printf_exec("cmd %p releases child func at %p\n", command, command->child_func);
			command->child_func->parent_cmd = ശൂന്യ;
		}
#endif
#എങ്കിൽ !BB_MMU
		സ്വതന്ത്രം(command->group_as_string);
		//command->group_as_string = NULL;
#endif
#ifndef __U_BOOT__
		വേണ്ടി (r = command->redirects; r; r = rnext) {
			debug_printf_clean("   redirect %d%s",
					r->rd_fd, redir_table[r->rd_type].descrip);
			/* guard against the case >$FOO, where foo is unset or blank */
			എങ്കിൽ (r->rd_filename) {
				debug_printf_clean(" fname:'%s'\n", r->rd_filename);
				സ്വതന്ത്രം(r->rd_filename);
				//r->rd_filename = NULL;
			}
			debug_printf_clean(" rd_dup:%d\n", r->rd_dup);
			rnext = r->next;
			സ്വതന്ത്രം(r);
		}
		//command->redirects = NULL;
#endif /* !__U_BOOT__ */
	}
	സ്വതന്ത്രം(pi->cmds);   /* children are an array, they get freed all at once */
	//pi->cmds = NULL;
#ifndef __U_BOOT__
#എങ്കിൽ ENABLE_HUSH_JOB
	സ്വതന്ത്രം(pi->cmdtext);
	//pi->cmdtext = NULL;
#endif
#endif /* !__U_BOOT__ */

	next = pi->next;
	സ്വതന്ത്രം(pi);
	തിരികെ next;
}

സ്ഥിതം ശൂന്യം free_pipe_list(ഘടന pipe *pi)
{
	ആകുമ്പോൾ (pi) {
#എങ്കിൽ HAS_KEYWORDS
		debug_printf_clean("pipe reserved word %d\n", pi->res_word);
#endif
		debug_printf_clean("pipe followup code %d\n", pi->followup);
		pi = free_pipe(pi);
	}
}

/*** Parsing routines ***/

#ifndef debug_print_വൃക്ഷം
സ്ഥിതം ശൂന്യം debug_print_വൃക്ഷം(ഘടന pipe *pi, പൂർണ്ണസംഖ്യ lvl)
{
	സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം PIPE[] ALIGN_PTR = {
		[PIPE_SEQ] = "SEQ",
		[PIPE_AND] = "AND",
		[PIPE_OR ] = "OR" ,
		[PIPE_BG ] = "BG" ,
	};
	സ്ഥിതം സ്ഥിരം അക്ഷരം *RES[] = {
		[RES_NONE ] = "NONE" ,
# എങ്കിൽ ENABLE_HUSH_IF
		[RES_IF   ] = "IF"   ,
		[RES_THEN ] = "THEN" ,
		[RES_ELIF ] = "ELIF" ,
		[RES_ELSE ] = "ELSE" ,
		[RES_FI   ] = "FI"   ,
# endif
# എങ്കിൽ ENABLE_HUSH_LOOPS
		[RES_FOR  ] = "FOR"  ,
		[RES_WHILE] = "WHILE",
		[RES_UNTIL] = "UNTIL",
		[RES_DO   ] = "DO"   ,
		[RES_DONE ] = "DONE" ,
# endif
# എങ്കിൽ ENABLE_HUSH_LOOPS || ENABLE_HUSH_CASE
		[RES_IN   ] = "IN"   ,
# endif
# എങ്കിൽ ENABLE_HUSH_CASE
		[RES_CASE ] = "CASE" ,
		[RES_CASE_IN ] = "CASE_IN" ,
		[RES_MATCH] = "MATCH",
		[RES_CASE_BODY] = "CASE_BODY",
		[RES_ESAC ] = "ESAC" ,
# endif
		[RES_XXXX ] = "XXXX" ,
		[RES_SNTX ] = "SNTX" ,
	};
	സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം CMDTYPE[] ALIGN_PTR = {
		"{}",
		"()",
		"[noglob]",
# എങ്കിൽ ENABLE_HUSH_FUNCTIONS
		"func()",
# endif
	};

	പൂർണ്ണസംഖ്യ pin, prn;

	pin = 0;
	ആകുമ്പോൾ (pi) {
		fdprintf(2, "%*spipe %d #cmds:%d %sres_word=%s followup=%d %s\n",
				lvl*2, "",
				pin,
				pi->അക്കം_cmds,
				(IF_HAS_KEYWORDS(pi->pi_inverted ? "! " :) ""),
				RES[pi->res_word],
				pi->followup, PIPE[pi->followup]
		);
		prn = 0;
		ആകുമ്പോൾ (prn < pi->അക്കം_cmds) {
			ഘടന command *command = &pi->cmds[prn];
			അക്ഷരം **argv = command->argv;

			fdprintf(2, "%*s cmd %d assignment_cnt:%d",
					lvl*2, "", prn,
					command->assignment_cnt);
# എങ്കിൽ ENABLE_HUSH_LINENO_VAR
			fdprintf(2, " LINENO:%u", command->lineno);
# endif
			എങ്കിൽ (command->group) {
				fdprintf(2, " group %s: (argv=%p)%s%s\n",
						CMDTYPE[command->cmd_type],
						argv
# എങ്കിൽ !BB_MMU
						, " group_as_string:", command->group_as_string
# അല്ലെങ്കിൽ
						, "", ""
# endif
				);
				debug_print_വൃക്ഷം(command->group, lvl+1);
				prn++;
				തുടരുക;
			}
			എങ്കിൽ (argv) ആകുമ്പോൾ (*argv) {
				fdprintf(2, " '%s'", *argv);
				argv++;
			}
#ifndef __U_BOOT__
			എങ്കിൽ (command->redirects)
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

സ്ഥിതം ഘടന pipe *new_pipe(ശൂന്യം)
{
	ഘടന pipe *pi;
	pi = xzalloc(വലുപ്പം(ഘടന pipe));
	/*pi->res_word = RES_NONE; - RES_NONE is 0 anyway */
	തിരികെ pi;
}

/* Command (member of a pipe) is complete, or we start a new pipe
 * if ctx->command is NULL.
 * No errors possible here.
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ done_command(ഘടന parse_context *ctx)
{
	/* The command is really already in the pipe structure, so
	 * advance the pipe counter and make a new, null command. */
	ഘടന pipe *pi = ctx->pipe;
	ഘടന command *command = ctx->command;

#എങ്കിൽ 0	/* Instead we emit error message at run time */
	എങ്കിൽ (ctx->pending_redirect) {
		/* For example, "cmd >" (no filename to redirect to) */
		syntax_error("invalid redirect");
		ctx->pending_redirect = ശൂന്യ;
	}
#endif

	എങ്കിൽ (command) {
		എങ്കിൽ (IS_NULL_CMD(command)) {
			debug_printf_parse("done_command: skipping null cmd, num_cmds=%d\n", pi->അക്കം_cmds);
			പോകുക clear_and_ret;
		}
		pi->അക്കം_cmds++;
		debug_printf_parse("done_command: ++num_cmds=%d\n", pi->അക്കം_cmds);
		//debug_print_tree(ctx->list_head, 20);
	} അല്ലെങ്കിൽ {
		debug_printf_parse("done_command: initializing, num_cmds=%d\n", pi->അക്കം_cmds);
	}

	/* Only real trickiness here is that the uncommitted
	 * command structure is not counted in pi->num_cmds. */
	pi->cmds = xrealloc(pi->cmds, വലുപ്പം(*pi->cmds) * (pi->അക്കം_cmds+1));
	ctx->command = command = &pi->cmds[pi->അക്കം_cmds];
 clear_and_ret:
	memset(command, 0, വലുപ്പം(*command));
#എങ്കിൽ ENABLE_HUSH_LINENO_VAR
	command->lineno = G.parse_lineno;
	debug_printf_parse("command->lineno = G.parse_lineno (%u)\n", G.parse_lineno);
#endif
	തിരികെ pi->അക്കം_cmds; /* used only for 0/nonzero check */
}

സ്ഥിതം ശൂന്യം done_pipe(ഘടന parse_context *ctx, pipe_style type)
{
	പൂർണ്ണസംഖ്യ not_null;

	debug_printf_parse("done_pipe entered, followup %d\n", type);
	/* Close previous command */
	not_null = done_command(ctx);
#എങ്കിൽ HAS_KEYWORDS
	ctx->pipe->pi_inverted = ctx->ctx_inverted;
	ctx->ctx_inverted = 0;
	ctx->pipe->res_word = ctx->ctx_res_w;
#endif
	എങ്കിൽ (type == PIPE_BG && ctx->list_head != ctx->pipe) {
		/* Necessary since && and || have precedence over &:
		 * "cmd1 && cmd2 &" must spawn both cmds, not only cmd2,
		 * in a backgrounded subshell.
		 */
		ഘടന pipe *pi;
		ഘടന command *command;

		/* Is this actually this construct, all pipes end with && or ||? */
		pi = ctx->list_head;
		ആകുമ്പോൾ (pi != ctx->pipe) {
			എങ്കിൽ (pi->followup != PIPE_AND && pi->followup != PIPE_OR)
				പോകുക no_conv;
			pi = pi->next;
		}

		debug_printf_parse("BG with more than one pipe, converting to { p1 &&...pN; } &\n");
		pi->followup = PIPE_SEQ; /* close pN _not_ with "&"! */
		pi = xzalloc(വലുപ്പം(*pi));
		pi->followup = PIPE_BG;
		pi->അക്കം_cmds = 1;
		pi->cmds = xzalloc(വലുപ്പം(pi->cmds[0]));
		command = &pi->cmds[0];
		എങ്കിൽ (CMD_NORMAL != 0) /* "if xzalloc didn't do that already" */
			command->cmd_type = CMD_NORMAL;
		command->group = ctx->list_head;
#എങ്കിൽ !BB_MMU
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
	} അല്ലെങ്കിൽ {
 no_conv:
		ctx->pipe->followup = type;
	}

	/* Without this check, even just <enter> on command line generates
	 * tree of three NOPs (!). Which is harmless but annoying.
	 * IOW: it is safe to do it unconditionally. */
	എങ്കിൽ (not_null
#എങ്കിൽ ENABLE_HUSH_IF
	 || ctx->ctx_res_w == RES_FI
#endif
#എങ്കിൽ ENABLE_HUSH_LOOPS
	 || ctx->ctx_res_w == RES_DONE
	 || ctx->ctx_res_w == RES_FOR
	 || ctx->ctx_res_w == RES_IN
#endif
#എങ്കിൽ ENABLE_HUSH_CASE
	 || ctx->ctx_res_w == RES_ESAC
#endif
	) {
		ഘടന pipe *new_p;
		debug_printf_parse("done_pipe: adding new pipe: "
#ifndef __U_BOOT__
				"not_null:%d ctx->ctx_res_w:%d\n",
				not_null, ctx->ctx_res_w);
#അല്ലെങ്കിൽ /* __U_BOOT__ */
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
#എങ്കിൽ ENABLE_HUSH_LOOPS
		എങ്കിൽ (ctx->ctx_res_w == RES_FOR
		 || ctx->ctx_res_w == RES_IN)
			ctx->ctx_res_w = RES_NONE;
#endif
#എങ്കിൽ ENABLE_HUSH_CASE
		എങ്കിൽ (ctx->ctx_res_w == RES_MATCH)
			ctx->ctx_res_w = RES_CASE_BODY;
		എങ്കിൽ (ctx->ctx_res_w == RES_CASE)
			ctx->ctx_res_w = RES_CASE_IN;
#endif
		ctx->command = ശൂന്യ; /* trick done_command below */
		/* Create the memory for command, roughly:
		 * ctx->pipe->cmds = new struct command;
		 * ctx->command = &ctx->pipe->cmds[0];
		 */
		done_command(ctx);
		//debug_print_tree(ctx->list_head, 10);
	}
	debug_printf_parse("done_pipe return\n");
}

സ്ഥിതം ശൂന്യം initialize_context(ഘടന parse_context *ctx)
{
	memset(ctx, 0, വലുപ്പം(*ctx));
	എങ്കിൽ (MAYBE_ASSIGNMENT != 0)
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
#എങ്കിൽ HAS_KEYWORDS
ഘടന reserved_combo {
	അക്ഷരം literal[6];
	unsigned അക്ഷരം res;
	unsigned അക്ഷരം assignment_flag;
	uint32_t flag;
};
എണ്ണൽ {
	FLAG_END   = (1 << RES_NONE ),
# എങ്കിൽ ENABLE_HUSH_IF
	FLAG_IF    = (1 << RES_IF   ),
	FLAG_THEN  = (1 << RES_THEN ),
	FLAG_ELIF  = (1 << RES_ELIF ),
	FLAG_ELSE  = (1 << RES_ELSE ),
	FLAG_FI    = (1 << RES_FI   ),
# endif
# എങ്കിൽ ENABLE_HUSH_LOOPS
	FLAG_FOR   = (1 << RES_FOR  ),
	FLAG_WHILE = (1 << RES_WHILE),
	FLAG_UNTIL = (1 << RES_UNTIL),
	FLAG_DO    = (1 << RES_DO   ),
	FLAG_DONE  = (1 << RES_DONE ),
	FLAG_IN    = (1 << RES_IN   ),
# endif
# എങ്കിൽ ENABLE_HUSH_CASE
	FLAG_MATCH = (1 << RES_MATCH),
	FLAG_ESAC  = (1 << RES_ESAC ),
# endif
	FLAG_START = (1 << RES_XXXX ),
};

സ്ഥിതം സ്ഥിരം ഘടന reserved_combo* match_reserved_word(o_string *word)
{
	/* Mostly a list of accepted follow-up reserved words.
	 * FLAG_END means we are done with the sequence, and are ready
	 * to turn the compound list into a command.
	 * FLAG_START means the word must start a new compound list.
	 */
	സ്ഥിതം സ്ഥിരം ഘടന reserved_combo reserved_list[] ALIGN4 = {
# എങ്കിൽ ENABLE_HUSH_IF
		{ "!",     RES_NONE,  NOT_ASSIGNMENT  , 0 },
		{ "if",    RES_IF,    MAYBE_ASSIGNMENT, FLAG_THEN | FLAG_START },
		{ "then",  RES_THEN,  MAYBE_ASSIGNMENT, FLAG_ELIF | FLAG_ELSE | FLAG_FI },
		{ "elif",  RES_ELIF,  MAYBE_ASSIGNMENT, FLAG_THEN },
		{ "else",  RES_ELSE,  MAYBE_ASSIGNMENT, FLAG_FI   },
		{ "fi",    RES_FI,    NOT_ASSIGNMENT  , FLAG_END  },
# endif
# എങ്കിൽ ENABLE_HUSH_LOOPS
		{ "for",   RES_FOR,   NOT_ASSIGNMENT  , FLAG_IN | FLAG_DO | FLAG_START },
		{ "while", RES_WHILE, MAYBE_ASSIGNMENT, FLAG_DO | FLAG_START },
		{ "until", RES_UNTIL, MAYBE_ASSIGNMENT, FLAG_DO | FLAG_START },
		{ "in",    RES_IN,    NOT_ASSIGNMENT  , FLAG_DO   },
		{ "do",    RES_DO,    MAYBE_ASSIGNMENT, FLAG_DONE },
		{ "done",  RES_DONE,  NOT_ASSIGNMENT  , FLAG_END  },
# endif
# എങ്കിൽ ENABLE_HUSH_CASE
		{ "case",  RES_CASE,  NOT_ASSIGNMENT  , FLAG_MATCH | FLAG_START },
		{ "esac",  RES_ESAC,  NOT_ASSIGNMENT  , FLAG_END  },
# endif
	};
	സ്ഥിരം ഘടന reserved_combo *r;

	വേണ്ടി (r = reserved_list; r < reserved_list + ARRAY_SIZE(reserved_list); r++) {
		എങ്കിൽ (strcmp(word->data, r->literal) == 0)
			തിരികെ r;
	}
	തിരികെ ശൂന്യ;
}
/* Return NULL: not a keyword, else: keyword
 */
സ്ഥിതം സ്ഥിരം ഘടന reserved_combo* reserved_word(ഘടന parse_context *ctx)
{
# എങ്കിൽ ENABLE_HUSH_CASE
	സ്ഥിതം സ്ഥിരം ഘടന reserved_combo reserved_match = {
		"",        RES_MATCH, NOT_ASSIGNMENT , FLAG_MATCH | FLAG_ESAC
	};
# endif
	സ്ഥിരം ഘടന reserved_combo *r;

	എങ്കിൽ (ctx->word.has_quoted_part)
		തിരികെ 0;
	r = match_reserved_word(&ctx->word);
	എങ്കിൽ (!r)
		തിരികെ r; /* NULL */

	debug_printf("found reserved word %s, res %d\n", r->literal, r->res);
# എങ്കിൽ ENABLE_HUSH_CASE
	എങ്കിൽ (r->res == RES_IN && ctx->ctx_res_w == RES_CASE_IN) {
		/* "case word IN ..." - IN part starts first MATCH part */
		r = &reserved_match;
	} അല്ലെങ്കിൽ
# endif
	എങ്കിൽ (r->flag == 0) { /* '!' */
		എങ്കിൽ (ctx->ctx_inverted) { /* bash doesn't accept '! ! true' */
			syntax_error("! ! command");
			ctx->ctx_res_w = RES_SNTX;
		}
		ctx->ctx_inverted = 1;
		തിരികെ r;
	}
	എങ്കിൽ (r->flag & FLAG_START) {
		ഘടന parse_context *old;

		old = xmemdup(ctx, വലുപ്പം(*ctx));
		debug_printf_parse("push stack %p\n", old);
		initialize_context(ctx);
		ctx->അട്ടി = old;
	} അല്ലെങ്കിൽ എങ്കിൽ (/*ctx->ctx_res_w == RES_NONE ||*/ !(ctx->old_flag & (1 << r->res))) {
		syntax_error_at(ctx->word.data);
		ctx->ctx_res_w = RES_SNTX;
		തിരികെ r;
	} അല്ലെങ്കിൽ {
		/* "{...} fi" is ok. "{...} if" is not
		 * Example:
		 * if { echo foo; } then { echo bar; } fi */
		എങ്കിൽ (ctx->command->group)
			done_pipe(ctx, PIPE_SEQ);
	}

	ctx->ctx_res_w = r->res;
	ctx->old_flag = r->flag;
	ctx->is_assignment = r->assignment_flag;
	debug_printf_parse("ctx->is_assignment='%s'\n", assignment_flag[ctx->is_assignment]);

	എങ്കിൽ (ctx->old_flag & FLAG_END) {
		ഘടന parse_context *old;

		done_pipe(ctx, PIPE_SEQ);
		debug_printf_parse("pop stack %p\n", ctx->അട്ടി);
		old = ctx->അട്ടി;
		old->command->group = ctx->list_head;
		old->command->cmd_type = CMD_NORMAL;
# എങ്കിൽ !BB_MMU
		/* At this point, the compound command's string is in
		 * ctx->as_string... except for the leading keyword!
		 * Consider this example: "echo a | if true; then echo a; fi"
		 * ctx->as_string will contain "true; then echo a; fi",
		 * with "if " remaining in old->as_string!
		 */
		{
			അക്ഷരം *str;
			പൂർണ്ണസംഖ്യ len = old->as_string.length;
			/* Concatenate halves */
			o_addstr(&old->as_string, ctx->as_string.data);
			o_free(&ctx->as_string);
			/* Find where leading keyword starts in first half */
			str = old->as_string.data + len;
			എങ്കിൽ (str > old->as_string.data)
				str--; /* skip whitespace after keyword */
			ആകുമ്പോൾ (str > old->as_string.data && isalpha(str[-1]))
				str--;
			/* Ugh, we're done with this horrid hack */
			old->command->group_as_string = xstrdup(str);
			debug_printf_parse("pop, remembering as:'%s'\n",
					old->command->group_as_string);
		}
# endif
		*ctx = *old;   /* physical copy */
		സ്വതന്ത്രം(old);
	}
	തിരികെ r;
}
#endif /* HAS_KEYWORDS */

/* Word is complete, look at it and update parsing context.
 * Normal return is 0. Syntax errors return 1.
 * Note: on return, word is reset, but not o_free'd!
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ done_word(ഘടന parse_context *ctx)
{
	ഘടന command *command = ctx->command;

	debug_printf_parse("done_word entered: '%s' %p\n", ctx->word.data, command);
	എങ്കിൽ (ctx->word.length == 0 && !ctx->word.has_quoted_part) {
		debug_printf_parse("done_word return 0: true null, ignored\n");
		തിരികെ 0;
	}

#ifndef __U_BOOT__
	എങ്കിൽ (ctx->pending_redirect) {
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
		എങ്കിൽ (ctx->pending_redirect->rd_type == REDIRECT_HEREDOC) {
			unbackslash(ctx->pending_redirect->rd_filename);
			/* Is it <<"HEREDOC"? */
			എങ്കിൽ (ctx->word.has_quoted_part) {
				ctx->pending_redirect->rd_dup |= HEREDOC_QUOTED;
			}
		}
		debug_printf_parse("word stored in rd_filename: '%s'\n", ctx->word.data);
		ctx->pending_redirect = ശൂന്യ;
	} അല്ലെങ്കിൽ {
#endif /* !__U_BOOT__ */
#എങ്കിൽ HAS_KEYWORDS
# എങ്കിൽ ENABLE_HUSH_CASE
		എങ്കിൽ (ctx->ctx_dsemicolon
		 && strcmp(ctx->word.data, "esac") != 0 /* not "... pattern) cmd;; esac" */
		) {
			/* already done when ctx_dsemicolon was set to 1: */
			/* ctx->ctx_res_w = RES_MATCH; */
			ctx->ctx_dsemicolon = 0;
		} അല്ലെങ്കിൽ
# endif
# എങ്കിൽ defined(CMD_TEST2_SINGLEWORD_NOGLOB)
		എങ്കിൽ (command->cmd_type == CMD_TEST2_SINGLEWORD_NOGLOB
		 && strcmp(ctx->word.data, "]]") == 0
		) {
			/* allow "[[ ]] >file" etc */
			command->cmd_type = CMD_SINGLEWORD_NOGLOB;
		} അല്ലെങ്കിൽ
# endif
		എങ്കിൽ (!command->argv /* if it's the first word... */
# എങ്കിൽ ENABLE_HUSH_LOOPS
		 && ctx->ctx_res_w != RES_FOR /* ...not after FOR or IN */
		 && ctx->ctx_res_w != RES_IN
# endif
# എങ്കിൽ ENABLE_HUSH_CASE
		 && ctx->ctx_res_w != RES_CASE
# endif
		) {
			സ്ഥിരം ഘടന reserved_combo *reserved;
			reserved = reserved_word(ctx);
			debug_printf_parse("checking for reserved-ness: %d\n", !!reserved);
			എങ്കിൽ (reserved) {
# എങ്കിൽ ENABLE_HUSH_LINENO_VAR
/* Case:
 * "while ...; do
 *	cmd ..."
 * If we don't close the pipe _now_, immediately after "do", lineno logic
 * sees "cmd" as starting at "do" - i.e., at the previous line.
 */
				എങ്കിൽ (0
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
				തിരികെ (ctx->ctx_res_w == RES_SNTX);
			}
# എങ്കിൽ defined(CMD_TEST2_SINGLEWORD_NOGLOB)
			എങ്കിൽ (strcmp(ctx->word.data, "[[") == 0) {
				command->cmd_type = CMD_TEST2_SINGLEWORD_NOGLOB;
			} അല്ലെങ്കിൽ
# endif
# എങ്കിൽ defined(CMD_SINGLEWORD_NOGLOB)
			എങ്കിൽ (0
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
# അല്ലെങ്കിൽ
			{ /* empty block to pair "if ... else" */ }
# endif
		}
#endif /* HAS_KEYWORDS */

		എങ്കിൽ (command->group) {
			/* "{ echo foo; } echo bar" - bad */
			syntax_error_at(ctx->word.data);
			debug_printf_parse("done_word return 1: syntax error, "
					"groups and arglists don't mix\n");
			തിരികെ 1;
		}

		/* If this word wasn't an assignment, next ones definitely
		 * can't be assignments. Even if they look like ones. */
		എങ്കിൽ (ctx->is_assignment != DEFINITELY_ASSIGNMENT
		 && ctx->is_assignment != WORD_IS_KEYWORD
		) {
			ctx->is_assignment = NOT_ASSIGNMENT;
		} അല്ലെങ്കിൽ {
			എങ്കിൽ (ctx->is_assignment == DEFINITELY_ASSIGNMENT) {
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

#എങ്കിൽ ENABLE_HUSH_LOOPS
	എങ്കിൽ (ctx->ctx_res_w == RES_FOR) {
		എങ്കിൽ (ctx->word.has_quoted_part
		 || endofname(command->argv[0])[0] != '\0'
		) {
			/* bash says just "not a valid identifier" */
			syntax_error("bad for loop variable");
			തിരികെ 1;
		}
		/* Force FOR to have just one word (variable name) */
		/* NB: basically, this makes hush see "for v in ..."
		 * syntax as if it is "for v; in ...". FOR and IN become
		 * two pipe structs in parse tree. */
		done_pipe(ctx, PIPE_SEQ);
	}
#endif
#എങ്കിൽ ENABLE_HUSH_CASE
	/* Force CASE to have just one word */
	എങ്കിൽ (ctx->ctx_res_w == RES_CASE) {
		done_pipe(ctx, PIPE_SEQ);
	}
#endif

	o_reset_to_empty_unquoted(&ctx->word);

	debug_printf_parse("done_word return 0\n");
	തിരികെ 0;
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
#എങ്കിൽ BB_MMU
#define parse_redir_വലത്_fd(as_string, നിവേശം) \
	parse_redir_വലത്_fd(നിവേശം)
#endif
സ്ഥിതം പൂർണ്ണസംഖ്യ parse_redir_വലത്_fd(o_string *as_string, ഘടന in_str *നിവേശം)
{
	പൂർണ്ണസംഖ്യ ch, d, ok;

	ch = i_peek(നിവേശം);
	എങ്കിൽ (ch != '&')
		തിരികെ REDIRFD_TO_FILE;

	ch = i_getch(നിവേശം);  /* get the & */
	nommu_addchr(as_string, ch);
	ch = i_peek(നിവേശം);
	എങ്കിൽ (ch == '-') {
		ch = i_getch(നിവേശം);
		nommu_addchr(as_string, ch);
		തിരികെ REDIRFD_CLOSE;
	}
	d = 0;
	ok = 0;
	ആകുമ്പോൾ (ch != EOF && isdigit(ch)) {
		d = d*10 + (ch-'0');
		ok = 1;
		ch = i_getch(നിവേശം);
		nommu_addchr(as_string, ch);
		ch = i_peek(നിവേശം);
	}
	എങ്കിൽ (ok) തിരികെ d;

//TODO: this is the place to catch ">&file" bashism (redirect both fd 1 and 2)

	bb_simple_error_സന്ദേശം("ambiguous redirect");
	തിരികെ REDIRFD_SYNTAX_ERR;
}

/* Return code is 0 normal, 1 if a syntax error is detected
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ parse_redirect(ഘടന parse_context *ctx,
		പൂർണ്ണസംഖ്യ fd,
		redir_type style,
		ഘടന in_str *നിവേശം)
{
	ഘടന command *command = ctx->command;
	ഘടന redir_struct *redir;
	ഘടന redir_struct **redirp;
	പൂർണ്ണസംഖ്യ dup_അക്കം;

	dup_അക്കം = REDIRFD_TO_FILE;
	എങ്കിൽ (style != REDIRECT_HEREDOC) {
		/* Check for a '>&1' type redirect */
		dup_അക്കം = parse_redir_വലത്_fd(&ctx->as_string, നിവേശം);
		എങ്കിൽ (dup_അക്കം == REDIRFD_SYNTAX_ERR)
			തിരികെ 1;
	} അല്ലെങ്കിൽ {
		പൂർണ്ണസംഖ്യ ch = i_peek_and_eat_bkslash_nl(നിവേശം);
		dup_അക്കം = (ch == '-'); /* HEREDOC_SKIPTABS bit is 1 */
		എങ്കിൽ (dup_അക്കം) { /* <<-... */
			ch = i_getch(നിവേശം);
			nommu_addchr(&ctx->as_string, ch);
			ch = i_peek(നിവേശം);
		}
	}

	എങ്കിൽ (style == REDIRECT_OVERWRITE && dup_അക്കം == REDIRFD_TO_FILE) {
		പൂർണ്ണസംഖ്യ ch = i_peek_and_eat_bkslash_nl(നിവേശം);
		എങ്കിൽ (ch == '|') {
			/* >|FILE redirect ("clobbering" >).
			 * Since we do not support "set -o noclobber" yet,
			 * >| and > are the same for now. Just eat |.
			 */
			ch = i_getch(നിവേശം);
			nommu_addchr(&ctx->as_string, ch);
		}
	}

	/* Create a new redir_struct and append it to the linked list */
	redirp = &command->redirects;
	ആകുമ്പോൾ ((redir = *redirp) != ശൂന്യ) {
		redirp = &(redir->next);
	}
	*redirp = redir = xzalloc(വലുപ്പം(*redir));
	/* redir->next = NULL; */
	/* redir->rd_filename = NULL; */
	redir->rd_type = style;
	redir->rd_fd = (fd == -1) ? redir_table[style].default_fd : fd;

	debug_printf_parse("redirect type %d %s\n", redir->rd_fd,
				redir_table[style].descrip);

	redir->rd_dup = dup_അക്കം;
	എങ്കിൽ (style != REDIRECT_HEREDOC && dup_അക്കം != REDIRFD_TO_FILE) {
		/* Erik had a check here that the file descriptor in question
		 * is legit; I postpone that to "run time"
		 * A "-" representation of "close me" shows up as a -3 here */
		debug_printf_parse("duplicating redirect '%d>&%d'\n",
				redir->rd_fd, redir->rd_dup);
	} അല്ലെങ്കിൽ {
#എങ്കിൽ 0		/* Instead we emit error message at run time */
		എങ്കിൽ (ctx->pending_redirect) {
			/* For example, "cmd > <file" */
			syntax_error("invalid redirect");
		}
#endif
		/* Set ctx->pending_redirect, so we know what to do at the
		 * end of the next parsed word. */
		ctx->pending_redirect = redir;
	}
	തിരികെ 0;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ redirect_opt_അക്കം(o_string *o)
{
	പൂർണ്ണസംഖ്യ അക്കം;

	എങ്കിൽ (o->data == ശൂന്യ)
		തിരികെ -1;
	അക്കം = bb_strtou(o->data, ശൂന്യ, 10);
	എങ്കിൽ (errno || അക്കം < 0)
		തിരികെ -1;
	o_reset_to_empty_unquoted(o);
	തിരികെ അക്കം;
}

#എങ്കിൽ BB_MMU
#define fetch_till_str(as_string, നിവേശം, word, skip_tabs) \
	fetch_till_str(നിവേശം, word, skip_tabs)
#endif
സ്ഥിതം അക്ഷരം *fetch_till_str(o_string *as_string,
		ഘടന in_str *നിവേശം,
		സ്ഥിരം അക്ഷരം *word,
		പൂർണ്ണസംഖ്യ heredoc_flags)
{
	o_string heredoc = NULL_O_STRING;
	unsigned past_EOL;
	പൂർണ്ണസംഖ്യ prev = 0; /* not \ */
	പൂർണ്ണസംഖ്യ ch;

	/* Starting with "" is necessary for this case:
	 * cat <<EOF
	 *
	 * xxx
	 * EOF
	 */
	heredoc.data = xzalloc(1); /* start as "", not as NULL */

	പോകുക jump_in;

	ആകുമ്പോൾ (1) {
		ch = i_getch(നിവേശം);
		എങ്കിൽ (ch != EOF)
			nommu_addchr(as_string, ch);
		എങ്കിൽ (ch == '\n' || ch == EOF) {
 check_heredoc_end:
#ifndef __U_BOOT__
			എങ്കിൽ ((heredoc_flags & HEREDOC_QUOTED) || prev != '\\') {
#അല്ലെങ്കിൽ /* __U_BOOT__ */
			എങ്കിൽ (prev != '\\') {
#endif
				/* End-of-line, and not a line continuation */
				എങ്കിൽ (strcmp(heredoc.data + past_EOL, word) == 0) {
					heredoc.data[past_EOL] = '\0';
					debug_printf_heredoc("parsed '%s' heredoc '%s'\n", word, heredoc.data);
					തിരികെ heredoc.data;
				}
				എങ്കിൽ (ch == '\n') {
					/* This is a new line.
					 * Remember position and backslash-escaping status.
					 */
					o_addchr(&heredoc, ch);
					prev = ch;
 jump_in:
					past_EOL = heredoc.length;
					/* Get 1st char of next line, possibly skipping leading tabs */
					ചെയ്യുക {
						ch = i_getch(നിവേശം);
						എങ്കിൽ (ch != EOF)
							nommu_addchr(as_string, ch);
#ifndef __U_BOOT__
					} ആകുമ്പോൾ ((heredoc_flags & HEREDOC_SKIPTABS) && ch == '\t');
#അല്ലെങ്കിൽ /* __U_BOOT__ */
				} ആകുമ്പോൾ (ch == '\t');
#endif
					/* If this immediately ended the line,
					 * go back to end-of-line checks.
					 */
					എങ്കിൽ (ch == '\n')
						പോകുക check_heredoc_end;
				}
			} അല്ലെങ്കിൽ {
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
				തുടരുക;
			}
		}
		എങ്കിൽ (ch == EOF) {
			o_free(&heredoc);
			തിരികെ ശൂന്യ; /* error */
		}
		o_addchr(&heredoc, ch);
		nommu_addchr(as_string, ch);
		എങ്കിൽ (prev == '\\' && ch == '\\')
			/* Correctly handle foo\\<eol> (not a line cont.) */
			prev = 0; /* not '\' */
		അല്ലെങ്കിൽ
			prev = ch;
	}
}
#endif /* !__U_BOOT__ */

/* Look at entire parse tree for not-yet-loaded REDIRECT_HEREDOCs
 * and load them all. There should be exactly heredoc_cnt of them.
 */
#എങ്കിൽ BB_MMU
#define fetch_heredocs(as_string, pi, heredoc_cnt, നിവേശം) \
	fetch_heredocs(pi, heredoc_cnt, നിവേശം)
#endif
സ്ഥിതം പൂർണ്ണസംഖ്യ fetch_heredocs(o_string *as_string, ഘടന pipe *pi, പൂർണ്ണസംഖ്യ heredoc_cnt, ഘടന in_str *നിവേശം)
{
	ആകുമ്പോൾ (pi && heredoc_cnt) {
		പൂർണ്ണസംഖ്യ i;
		ഘടന command *cmd = pi->cmds;

		debug_printf_heredoc("fetch_heredocs: num_cmds:%d cmd argv0:'%s'\n",
				pi->അക്കം_cmds,
				cmd->argv ? cmd->argv[0] : "NONE"
		);
		വേണ്ടി (i = 0; i < pi->അക്കം_cmds; i++) {
#ifndef __U_BOOT__
			ഘടന redir_struct *redir = cmd->redirects;

#endif /* !__U_BOOT__ */
			debug_printf_heredoc("fetch_heredocs: %d cmd argv0:'%s'\n",
					i, cmd->argv ? cmd->argv[0] : "NONE");
#ifndef __U_BOOT__
			ആകുമ്പോൾ (redir) {
				എങ്കിൽ (redir->rd_type == REDIRECT_HEREDOC) {
					അക്ഷരം *p;

					redir->rd_type = REDIRECT_HEREDOC2;
					/* redir->rd_dup is (ab)used to indicate <<- */
					p = fetch_till_str(as_string, നിവേശം,
							redir->rd_filename, redir->rd_dup);
					എങ്കിൽ (!p) {
						syntax_error("unexpected EOF in here document");
						തിരികെ -1;
					}
					സ്വതന്ത്രം(redir->rd_filename);
					redir->rd_filename = p;
					heredoc_cnt--;
				}
				redir = redir->next;
			}
#endif /* !__U_BOOT__ */
			എങ്കിൽ (cmd->group) {
				//bb_error_msg("%s:%u heredoc_cnt:%d", __func__, __LINE__, heredoc_cnt);
				heredoc_cnt = fetch_heredocs(as_string, cmd->group, heredoc_cnt, നിവേശം);
				//bb_error_msg("%s:%u heredoc_cnt:%d", __func__, __LINE__, heredoc_cnt);
				എങ്കിൽ (heredoc_cnt < 0)
					തിരികെ heredoc_cnt; /* error */
			}
			cmd++;
		}
		pi = pi->next;
	}
	തിരികെ heredoc_cnt;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ run_list(ഘടന pipe *pi);
#എങ്കിൽ BB_MMU
#define parse_stream(pstring, heredoc_cnt_ptr, നിവേശം, end_trigger) \
	parse_stream(heredoc_cnt_ptr, നിവേശം, end_trigger)
#endif
സ്ഥിതം ഘടന pipe *parse_stream(അക്ഷരം **pstring,
		പൂർണ്ണസംഖ്യ *heredoc_cnt_ptr,
		ഘടന in_str *നിവേശം,
		പൂർണ്ണസംഖ്യ end_trigger);

/* Returns number of heredocs not yet consumed,
 * or -1 on error.
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ parse_group(ഘടന parse_context *ctx,
		ഘടന in_str *നിവേശം, പൂർണ്ണസംഖ്യ ch)
{
	/* ctx->word contains characters seen prior to ( or {.
	 * Typically it's empty, but for function defs,
	 * it contains function name (without '()'). */
#എങ്കിൽ BB_MMU
# define as_string ശൂന്യ
#അല്ലെങ്കിൽ
	അക്ഷരം *as_string = ശൂന്യ;
#endif
	ഘടന pipe *pipe_list;
	പൂർണ്ണസംഖ്യ heredoc_cnt = 0;
	പൂർണ്ണസംഖ്യ endch;
	ഘടന command *command = ctx->command;

	debug_printf_parse("parse_group entered\n");
#എങ്കിൽ ENABLE_HUSH_FUNCTIONS
	എങ്കിൽ (ch == '(' && !ctx->word.has_quoted_part) {
		എങ്കിൽ (ctx->word.length)
			എങ്കിൽ (done_word(ctx))
				തിരികെ -1;
		എങ്കിൽ (!command->argv)
			പോകുക skip; /* (... */
		എങ്കിൽ (command->argv[1]) { /* word word ... (... */
			syntax_error_unexpected_ch('(');
			തിരികെ -1;
		}
		/* it is "word(..." or "word (..." */
		ചെയ്യുക
			ch = i_getch(നിവേശം);
		ആകുമ്പോൾ (ch == ' ' || ch == '\t');
		എങ്കിൽ (ch != ')') {
			syntax_error_unexpected_ch(ch);
			തിരികെ -1;
		}
		nommu_addchr(&ctx->as_string, ch);
		ചെയ്യുക
			ch = i_getch(നിവേശം);
		ആകുമ്പോൾ (ch == ' ' || ch == '\t' || ch == '\n');
		എങ്കിൽ (ch != '{' && ch != '(') {
			syntax_error_unexpected_ch(ch);
			തിരികെ -1;
		}
//bash allows functions named "123", "..", "return"!
//		if (endofname(command->argv[0])[0] != '\0') {
//			syntax_error("bad function name");
//			return -1;
//		}
		nommu_addchr(&ctx->as_string, ch);
		command->cmd_type = CMD_FUNCDEF;
		പോകുക skip;
	}
#endif

#എങ്കിൽ 0 /* Prevented by caller */
	എങ്കിൽ (command->argv /* word [word]{... */
	 || ctx->word.length /* word{... */
	 || ctx->word.has_quoted_part /* ""{... */
	) {
		syntax_error(ശൂന്യ);
		debug_printf_parse("parse_group return -1: "
			"syntax error, groups and arglists don't mix\n");
		തിരികെ -1;
	}
#endif

#ifndef __U_BOOT__
 IF_HUSH_FUNCTIONS(skip:)
#endif /* !__U_BOOT__ */

	endch = '}';
	എങ്കിൽ (ch == '(') {
		endch = ')';
#ifndef __U_BOOT__
		IF_HUSH_FUNCTIONS(എങ്കിൽ (command->cmd_type != CMD_FUNCDEF))
			command->cmd_type = CMD_SUBSHELL;
#endif /* !__U_BOOT__ */
	} അല്ലെങ്കിൽ {
		/* bash does not allow "{echo...", requires whitespace */
		ch = i_peek(നിവേശം);
		എങ്കിൽ (ch != ' ' && ch != '\t' && ch != '\n'
		 && ch != '('	/* but "{(..." is allowed (without whitespace) */
		) {
			syntax_error_unexpected_ch(ch);
			തിരികെ -1;
		}
		എങ്കിൽ (ch != '(') {
			ch = i_getch(നിവേശം);
			nommu_addchr(&ctx->as_string, ch);
		}
	}

	debug_printf_heredoc("calling parse_stream, heredoc_cnt:%d\n", heredoc_cnt);
	pipe_list = parse_stream(&as_string, &heredoc_cnt, നിവേശം, endch);
	debug_printf_heredoc("parse_stream returned: heredoc_cnt:%d\n", heredoc_cnt);
#എങ്കിൽ !BB_MMU
	എങ്കിൽ (as_string)
		o_addstr(&ctx->as_string, as_string);
#endif

	/* empty ()/{} or parse error? */
	എങ്കിൽ (!pipe_list || pipe_list == ERR_PTR) {
		/* parse_stream already emitted error msg */
		എങ്കിൽ (!BB_MMU)
			സ്വതന്ത്രം(as_string);
		debug_printf_parse("parse_group return -1: "
			"parse_stream returned %p\n", pipe_list);
		തിരികെ -1;
	}
#എങ്കിൽ !BB_MMU
	as_string[strlen(as_string) - 1] = '\0'; /* plink ')' or '}' */
	command->group_as_string = as_string;
	debug_printf_parse("end of group, remembering as:'%s'\n",
			command->group_as_string);
#endif

#എങ്കിൽ ENABLE_HUSH_FUNCTIONS
	/* Convert "f() (cmds)" to "f() {(cmds)}" */
	എങ്കിൽ (command->cmd_type == CMD_FUNCDEF && endch == ')') {
		ഘടന command *cmd2;

		cmd2 = xzalloc(വലുപ്പം(*cmd2));
		cmd2->cmd_type = CMD_SUBSHELL;
		cmd2->group = pipe_list;
# എങ്കിൽ !BB_MMU
//UNTESTED!
		cmd2->group_as_string = command->group_as_string;
		command->group_as_string = xasprintf("(%s)", command->group_as_string);
# endif

		pipe_list = new_pipe();
		pipe_list->cmds = cmd2;
		pipe_list->അക്കം_cmds = 1;
	}
#endif

	command->group = pipe_list;

	debug_printf_parse("parse_group return %d\n", heredoc_cnt);
	തിരികെ heredoc_cnt;
	/* command remains "open", available for possible redirects */
#undef as_string
}

#ifndef __U_BOOT__
#എങ്കിൽ ENABLE_HUSH_TICK || ENABLE_FEATURE_SH_MATH || ENABLE_HUSH_DOLLAR_OPS
/* Subroutines for copying $(...) and `...` things */
/* '...' */
സ്ഥിതം പൂർണ്ണസംഖ്യ add_till_single_quote(o_string *dest, ഘടന in_str *നിവേശം)
{
	ആകുമ്പോൾ (1) {
		പൂർണ്ണസംഖ്യ ch = i_getch(നിവേശം);
		എങ്കിൽ (ch == EOF) {
			syntax_error_unterm_ch('\'');
			തിരികെ 0;
		}
		എങ്കിൽ (ch == '\'')
			തിരികെ 1;
		o_addchr(dest, ch);
	}
}
സ്ഥിതം പൂർണ്ണസംഖ്യ add_till_single_quote_dquoted(o_string *dest, ഘടന in_str *നിവേശം)
{
	ആകുമ്പോൾ (1) {
		പൂർണ്ണസംഖ്യ ch = i_getch(നിവേശം);
		എങ്കിൽ (ch == EOF) {
			syntax_error_unterm_ch('\'');
			തിരികെ 0;
		}
		എങ്കിൽ (ch == '\'')
			തിരികെ 1;
		o_addqchr(dest, ch);
	}
}

/* "...\"...`..`...." - do we need to handle "...$(..)..." too? */
സ്ഥിതം പൂർണ്ണസംഖ്യ add_till_backquote(o_string *dest, ഘടന in_str *നിവേശം, പൂർണ്ണസംഖ്യ in_dquote);
സ്ഥിതം പൂർണ്ണസംഖ്യ add_till_double_quote(o_string *dest, ഘടന in_str *നിവേശം)
{
	ആകുമ്പോൾ (1) {
		പൂർണ്ണസംഖ്യ ch = i_getch(നിവേശം);
		എങ്കിൽ (ch == EOF) {
			syntax_error_unterm_ch('"');
			തിരികെ 0;
		}
		എങ്കിൽ (ch == '"')
			തിരികെ 1;
		എങ്കിൽ (ch == '\\') {  /* \x. Copy both chars. */
			o_addchr(dest, ch);
			ch = i_getch(നിവേശം);
		}
		o_addchr(dest, ch);
		എങ്കിൽ (ch == '`') {
			എങ്കിൽ (!add_till_backquote(dest, നിവേശം, /*in_dquote:*/ 1))
				തിരികെ 0;
			o_addchr(dest, ch);
			തുടരുക;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ add_till_backquote(o_string *dest, ഘടന in_str *നിവേശം, പൂർണ്ണസംഖ്യ in_dquote)
{
	ആകുമ്പോൾ (1) {
		പൂർണ്ണസംഖ്യ ch = i_getch(നിവേശം);
		എങ്കിൽ (ch == '`')
			തിരികെ 1;
		എങ്കിൽ (ch == '\\') {
			/* \x. Copy both unless it is \`, \$, \\ and maybe \" */
			ch = i_getch(നിവേശം);
			എങ്കിൽ (ch != '`'
			 && ch != '$'
			 && ch != '\\'
			 && (!in_dquote || ch != '"')
			) {
				o_addchr(dest, '\\');
			}
		}
		എങ്കിൽ (ch == EOF) {
			syntax_error_unterm_ch('`');
			തിരികെ 0;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ add_till_closing_bracket(o_string *dest, ഘടന in_str *നിവേശം, unsigned end_ch)
{
	പൂർണ്ണസംഖ്യ ch;
	അക്ഷരം dbl = end_ch & DOUBLE_CLOSE_CHAR_FLAG;
# എങ്കിൽ BASH_SUBSTR || BASH_PATTERN_SUBST
	അക്ഷരം end_char2 = end_ch >> 8;
# endif
	end_ch &= (DOUBLE_CLOSE_CHAR_FLAG - 1);

# എങ്കിൽ ENABLE_HUSH_INTERACTIVE
	G.promptmode = 1; /* PS2 */
# endif
	debug_printf_prompt("%s promptmode=%d\n", __func__, G.promptmode);

	ആകുമ്പോൾ (1) {
		ch = i_getch(നിവേശം);
		എങ്കിൽ (ch == EOF) {
			syntax_error_unterm_ch(end_ch);
			തിരികെ 0;
		}
		എങ്കിൽ (ch == end_ch
# എങ്കിൽ BASH_SUBSTR || BASH_PATTERN_SUBST
		 || ch == end_char2
# endif
		) {
			എങ്കിൽ (!dbl)
				നിർത്തുക;
			/* we look for closing )) of $((EXPR)) */
			എങ്കിൽ (i_peek_and_eat_bkslash_nl(നിവേശം) == end_ch) {
				i_getch(നിവേശം); /* eat second ')' */
				നിർത്തുക;
			}
		}
		o_addchr(dest, ch);
		//bb_error_msg("%s:o_addchr('%c')", __func__, ch);
		എങ്കിൽ (ch == '(' || ch == '{') {
			ch = (ch == '(' ? ')' : '}');
			എങ്കിൽ (!add_till_closing_bracket(dest, നിവേശം, ch))
				തിരികെ 0;
			o_addchr(dest, ch);
			തുടരുക;
		}
		എങ്കിൽ (ch == '\'') {
			എങ്കിൽ (!add_till_single_quote(dest, നിവേശം))
				തിരികെ 0;
			o_addchr(dest, ch);
			തുടരുക;
		}
		എങ്കിൽ (ch == '"') {
			എങ്കിൽ (!add_till_double_quote(dest, നിവേശം))
				തിരികെ 0;
			o_addchr(dest, ch);
			തുടരുക;
		}
		എങ്കിൽ (ch == '`') {
			എങ്കിൽ (!add_till_backquote(dest, നിവേശം, /*in_dquote:*/ 0))
				തിരികെ 0;
			o_addchr(dest, ch);
			തുടരുക;
		}
		എങ്കിൽ (ch == '\\') {
			/* \x. Copy verbatim. Important for  \(, \) */
			ch = i_getch(നിവേശം);
			എങ്കിൽ (ch == EOF) {
				syntax_error_unterm_ch(end_ch);
				തിരികെ 0;
			}
# എങ്കിൽ 0
			എങ്കിൽ (ch == '\n') {
				/* "backslash+newline", ignore both */
				o_delchr(dest); /* undo insertion of '\' */
				തുടരുക;
			}
# endif
			o_addchr(dest, ch);
			//bb_error_msg("%s:o_addchr('%c') after '\\'", __func__, ch);
			തുടരുക;
		}
	}
	debug_printf_parse("%s return '%s' ch:'%c'\n", __func__, dest->data, ch);
	തിരികെ ch;
}
#endif /* ENABLE_HUSH_TICK || ENABLE_FEATURE_SH_MATH || ENABLE_HUSH_DOLLAR_OPS */

#എങ്കിൽ BASH_DOLLAR_SQUOTE
/* Return code: 1 for "found and parsed", 0 for "seen something else" */
# എങ്കിൽ BB_MMU
#define parse_dollar_squote(as_string, dest, നിവേശം) \
	parse_dollar_squote(dest, നിവേശം)
#define as_string ശൂന്യ
# endif
സ്ഥിതം പൂർണ്ണസംഖ്യ parse_dollar_squote(o_string *as_string, o_string *dest, ഘടന in_str *നിവേശം)
{
	പൂർണ്ണസംഖ്യ start;
	പൂർണ്ണസംഖ്യ ch = i_peek_and_eat_bkslash_nl(നിവേശം);  /* first character after the $ */
	debug_printf_parse("parse_dollar_squote entered: ch='%c'\n", ch);
	എങ്കിൽ (ch != '\'')
		തിരികെ 0;

	dest->has_quoted_part = 1;
	start = dest->length;

	ch = i_getch(നിവേശം); /* eat ' */
	nommu_addchr(as_string, ch);
	ആകുമ്പോൾ (1) {
		ch = i_getch(നിവേശം);
		nommu_addchr(as_string, ch);
		എങ്കിൽ (ch == EOF) {
			syntax_error_unterm_ch('\'');
			തിരികെ 0;
		}
		എങ്കിൽ (ch == '\'')
			നിർത്തുക;
		എങ്കിൽ (ch == SPECIAL_VAR_ചിഹ്നം) {
			/* Convert raw ^C to corresponding special variable reference */
			o_addchr(dest, SPECIAL_VAR_ചിഹ്നം);
			o_addchr(dest, SPECIAL_VAR_QUOTED_SVS);
			/* will addchr() another SPECIAL_VAR_SYMBOL (see after the if() block) */
		} അല്ലെങ്കിൽ എങ്കിൽ (ch == '\\') {
			സ്ഥിതം സ്ഥിരം അക്ഷരം C_escapes[] ALIGN1 = "nrbtfav""x\\01234567";

			ch = i_getch(നിവേശം);
			nommu_addchr(as_string, ch);
			എങ്കിൽ (strchr(C_escapes, ch)) {
				അക്ഷരം buf[4];
				അക്ഷരം *p = buf;
				പൂർണ്ണസംഖ്യ cnt = 2;

				buf[0] = ch;
				എങ്കിൽ ((unsigned അക്ഷരം)(ch - '0') <= 7) { /* \ooo */
					ചെയ്യുക {
						ch = i_peek(നിവേശം);
						എങ്കിൽ ((unsigned അക്ഷരം)(ch - '0') > 7)
							നിർത്തുക;
						*++p = ch = i_getch(നിവേശം);
						nommu_addchr(as_string, ch);
					} ആകുമ്പോൾ (--cnt != 0);
				} അല്ലെങ്കിൽ എങ്കിൽ (ch == 'x') { /* \xHH */
					ചെയ്യുക {
						ch = i_peek(നിവേശം);
						എങ്കിൽ (!isxdigit(ch))
							നിർത്തുക;
						*++p = ch = i_getch(നിവേശം);
						nommu_addchr(as_string, ch);
					} ആകുമ്പോൾ (--cnt != 0);
					എങ്കിൽ (cnt == 2) { /* \x but next char is "bad" */
						ch = 'x';
						പോകുക unrecognized;
					}
				} /* else simple seq like \\ or \t */
				*++p = '\0';
				p = buf;
				ch = bb_process_escape_sequence((ശൂന്യം*)&p);
				//bb_error_msg("buf:'%s' ch:%x", buf, ch);
				എങ്കിൽ (ch == '\0')
					തുടരുക; /* bash compat: $'...\0...' emits nothing */
			} അല്ലെങ്കിൽ { /* unrecognized "\z": encode both chars unless ' or " */
				എങ്കിൽ (ch != '\'' && ch != '"') {
 unrecognized:
					o_addqchr(dest, '\\');
				}
			}
		} /* if (\...) */
		o_addqchr(dest, ch);
	}

	എങ്കിൽ (dest->length == start) {
		/* $'', $'\0', $'\000\x00' and the like */
		o_addchr(dest, SPECIAL_VAR_ചിഹ്നം);
		o_addchr(dest, SPECIAL_VAR_ചിഹ്നം);
	}

	തിരികെ 1;
# undef as_string
}
#അല്ലെങ്കിൽ
# define parse_dollar_squote(as_string, dest, നിവേശം) 0
#endif /* BASH_DOLLAR_SQUOTE */
#endif /* !__U_BOOT__ */

/* Return code: 0 for OK, 1 for syntax error */
#എങ്കിൽ BB_MMU
#define parse_dollar(as_string, dest, നിവേശം, quote_mask) \
	parse_dollar(dest, നിവേശം, quote_mask)
#define as_string ശൂന്യ
#endif
സ്ഥിതം പൂർണ്ണസംഖ്യ parse_dollar(o_string *as_string,
		o_string *dest,
		ഘടന in_str *നിവേശം, unsigned അക്ഷരം quote_mask)
{
	പൂർണ്ണസംഖ്യ ch = i_peek_and_eat_bkslash_nl(നിവേശം);  /* first character after the $ */

	debug_printf_parse("parse_dollar entered: ch='%c' quote_mask:0x%x\n", ch, quote_mask);
	എങ്കിൽ (isalpha(ch)) {
 make_var:
		ch = i_getch(നിവേശം);
		nommu_addchr(as_string, ch);
 /*make_var1:*/
		o_addchr(dest, SPECIAL_VAR_ചിഹ്നം);
		ആകുമ്പോൾ (1) {
			debug_printf_parse(": '%c'\n", ch);
			o_addchr(dest, ch | quote_mask);
			quote_mask = 0;
			ch = i_peek_and_eat_bkslash_nl(നിവേശം);
			എങ്കിൽ (!isalnum(ch) && ch != '_') {
				/* End of variable name reached */
				നിർത്തുക;
			}
			ch = i_getch(നിവേശം);
			nommu_addchr(as_string, ch);
		}
		o_addchr(dest, SPECIAL_VAR_ചിഹ്നം);
	} അല്ലെങ്കിൽ എങ്കിൽ (isdigit(ch)) {
 make_one_char_var:
		ch = i_getch(നിവേശം);
		nommu_addchr(as_string, ch);
		o_addchr(dest, SPECIAL_VAR_ചിഹ്നം);
		debug_printf_parse(": '%c'\n", ch);
		o_addchr(dest, ch | quote_mask);
		o_addchr(dest, SPECIAL_VAR_ചിഹ്നം);
	} അല്ലെങ്കിൽ തിരഞ്ഞെടുപ്പ് (ch) {
#ifndef __U_BOOT__
	അവസ്ഥ '$': /* pid */
	അവസ്ഥ '!': /* last bg pid */
#endif /* !__U_BOOT__ */
	അവസ്ഥ '?': /* last exit code */
	അവസ്ഥ '#': /* number of args */
	അവസ്ഥ '*': /* args */
	അവസ്ഥ '@': /* args */
	അവസ്ഥ '-': /* $- option flags set by set builtin or shell options (-i etc) */
		പോകുക make_one_char_var;
	അവസ്ഥ '{': {
		അക്ഷരം len_single_ch;

		o_addchr(dest, SPECIAL_VAR_ചിഹ്നം);

		ch = i_getch(നിവേശം); /* eat '{' */
		nommu_addchr(as_string, ch);

		ch = i_getch_and_eat_bkslash_nl(നിവേശം); /* first char after '{' */
		/* It should be ${?}, or ${#var},
		 * or even ${?+subst} - operator acting on a special variable,
		 * or the beginning of variable name.
		 */
		എങ്കിൽ (ch == EOF
		 || (!strchr(_SPECIAL_VARS_STR, ch) && !isalnum(ch)) /* not one of those */
		) {
 bad_dollar_syntax:
			syntax_error_unterm_str("${name}");
			debug_printf_parse("parse_dollar return 0: unterminated ${name}\n");
			തിരികെ 0;
		}
		nommu_addchr(as_string, ch);
		len_single_ch = ch;
		ch |= quote_mask;

		/* It's possible to just call add_till_closing_bracket() at this point.
		 * However, this regresses some of our testsuite cases
		 * which check invalid constructs like ${%}.
		 * Oh well... let's check that the var name part is fine... */

		എങ്കിൽ (isdigit(len_single_ch)
		 || (len_single_ch == '#' && isdigit(i_peek_and_eat_bkslash_nl(നിവേശം)))
		) {
			/* Execution engine uses plain xatoi_positive()
			 * to interpret ${NNN} and {#NNN},
			 * check syntax here in the parser.
			 * (bash does not support expressions in ${#NN},
			 * e.g. ${#$var} and {#1:+WORD} are not supported).
			 */
			unsigned cnt = 9; /* max 9 digits for ${NN} and 8 for {#NN} */
			ആകുമ്പോൾ (1) {
				o_addchr(dest, ch);
				debug_printf_parse(": '%c'\n", ch);
				ch = i_getch_and_eat_bkslash_nl(നിവേശം);
				nommu_addchr(as_string, ch);
				എങ്കിൽ (ch == '}')
					നിർത്തുക;
				എങ്കിൽ (--cnt == 0)
					പോകുക bad_dollar_syntax;
				എങ്കിൽ (len_single_ch != '#' && strchr(VAR_SUBST_OPS, ch))
					/* ${NN<op>...} is valid */
					പോകുക eat_until_closing;
				എങ്കിൽ (!isdigit(ch))
					പോകുക bad_dollar_syntax;
			}
		} അല്ലെങ്കിൽ
		ആകുമ്പോൾ (1) {
			unsigned pos;

			o_addchr(dest, ch);
			debug_printf_parse(": '%c'\n", ch);

			ch = i_getch(നിവേശം);
			nommu_addchr(as_string, ch);
			എങ്കിൽ (ch == '}')
				നിർത്തുക;
#ifndef __U_BOOT__
			എങ്കിൽ (!isalnum(ch) && ch != '_') {
#അല്ലെങ്കിൽ /* __U_BOOT__ */
			/*
			 * In several places in U-Boot, we use variable like
			 * foo# (e.g. serial#), particularly in env.
			 * So, we need to authorize # to appear inside
			 * variable name and then expand this variable.
			 * NOTE Having # in variable name is not permitted in
			 * upstream hush but expansion will be done (even though
			 * the result will be empty).
			 */
			എങ്കിൽ (!isalnum(ch) && ch != '_' && ch != '#') {
#endif /* __U_BOOT__ */
				unsigned end_ch;
#ifndef __U_BOOT__
				unsigned അക്ഷരം last_ch;
#endif /* !__U_BOOT__ */
				/* handle parameter expansions
				 * http://www.opengroup.org/onlinepubs/009695399/utilities/xcu_chap02.html#tag_02_06_02
				 */
				എങ്കിൽ (!strchr(VAR_SUBST_OPS, ch)) { /* ${var<bad_char>... */
					എങ്കിൽ (len_single_ch != '#'
					/*|| !strchr(SPECIAL_VARS_STR, ch) - disallow errors like ${#+} ? */
					 || i_peek(നിവേശം) != '}'
					) {
						പോകുക bad_dollar_syntax;
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
				എങ്കിൽ (BASH_SUBSTR
				 && ch == ':'
				 && !strchr(MINUS_PLUS_EQUAL_QUESTION, i_peek(നിവേശം))
				) {
					/* It's ${var:N[:M]} thing */
					end_ch = '}' * 0x100 + ':';
				}
				എങ്കിൽ (BASH_PATTERN_SUBST
				 && ch == '/'
				) {
					/* It's ${var/[/]pattern[/repl]} thing */
					എങ്കിൽ (i_peek(നിവേശം) == '/') { /* ${var//pattern[/repl]}? */
						i_getch(നിവേശം);
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
				എങ്കിൽ (i_peek(നിവേശം) == '/') {
					o_addchr(dest, i_getch(നിവേശം));
				}
#ifndef __U_BOOT__
 again:
#endif /* !__U_BOOT__ */
				എങ്കിൽ (!BB_MMU)
					pos = dest->length;
#എങ്കിൽ ENABLE_HUSH_DOLLAR_OPS
#ifndef __U_BOOT__
				last_ch = add_till_closing_bracket(dest, നിവേശം, end_ch);
				എങ്കിൽ (last_ch == 0) /* error? */
					തിരികെ 0;
#endif /* !__U_BOOT__ */
#അല്ലെങ്കിൽ
# error Simple code to only allow ${var} is not implemented
#endif
				എങ്കിൽ (as_string) {
					o_addstr(as_string, dest->data + pos);
#ifndef __U_BOOT__
					o_addchr(as_string, last_ch);
#endif /* !__U_BOOT__ */
				}

#ifndef __U_BOOT__
				എങ്കിൽ ((BASH_SUBSTR || BASH_PATTERN_SUBST)
					 && (end_ch & 0xff00)
				) {
					/* close the first block: */
					o_addchr(dest, SPECIAL_VAR_ചിഹ്നം);
					/* while parsing N from ${var:N[:M]}
					 * or pattern from ${var/[/]pattern[/repl]} */
					എങ്കിൽ ((end_ch & 0xff) == last_ch) {
						/* got ':' or '/'- parse the rest */
						end_ch = '}';
						പോകുക again;
					}
					/* got '}' */
					എങ്കിൽ (BASH_SUBSTR && end_ch == '}' * 0x100 + ':') {
						/* it's ${var:N} - emulate :999999999 */
						o_addstr(dest, "999999999");
					} /* else: it's ${var/[/]pattern} */
				}
#endif /* !__U_BOOT__ */
				നിർത്തുക;
			}
			len_single_ch = 0; /* it can't be ${#C} op */
		}
		o_addchr(dest, SPECIAL_VAR_ചിഹ്നം);
		നിർത്തുക;
	}
#എങ്കിൽ ENABLE_FEATURE_SH_MATH || ENABLE_HUSH_TICK
	അവസ്ഥ '(': {
		unsigned pos;

		ch = i_getch(നിവേശം);
		nommu_addchr(as_string, ch);
# എങ്കിൽ ENABLE_FEATURE_SH_MATH
		എങ്കിൽ (i_peek_and_eat_bkslash_nl(നിവേശം) == '(') {
			ch = i_getch(നിവേശം);
			nommu_addchr(as_string, ch);
			o_addchr(dest, SPECIAL_VAR_ചിഹ്നം);
			o_addchr(dest, quote_mask | '+');
			എങ്കിൽ (!BB_MMU)
				pos = dest->length;
			എങ്കിൽ (!add_till_closing_bracket(dest, നിവേശം, ')' | DOUBLE_CLOSE_CHAR_FLAG))
				തിരികെ 0; /* error */
			എങ്കിൽ (as_string) {
				o_addstr(as_string, dest->data + pos);
				o_addchr(as_string, ')');
				o_addchr(as_string, ')');
			}
			o_addchr(dest, SPECIAL_VAR_ചിഹ്നം);
			നിർത്തുക;
		}
# endif
# എങ്കിൽ ENABLE_HUSH_TICK
		o_addchr(dest, SPECIAL_VAR_ചിഹ്നം);
		o_addchr(dest, quote_mask | '`');
		എങ്കിൽ (!BB_MMU)
			pos = dest->length;
		എങ്കിൽ (!add_till_closing_bracket(dest, നിവേശം, ')'))
			തിരികെ 0; /* error */
		എങ്കിൽ (as_string) {
			o_addstr(as_string, dest->data + pos);
			o_addchr(as_string, ')');
		}
		o_addchr(dest, SPECIAL_VAR_ചിഹ്നം);
# endif
		നിർത്തുക;
	}
#endif
	അവസ്ഥ '_':
		പോകുക make_var;
#എങ്കിൽ 0
	/* TODO: $_: */
	/* $_ Shell or shell script name; or last argument of last command
	 * (if last command wasn't a pipe; if it was, bash sets $_ to "");
	 * but in command's env, set to full pathname used to invoke it */
		ch = i_getch(നിവേശം);
		nommu_addchr(as_string, ch);
		ch = i_peek_and_eat_bkslash_nl(നിവേശം);
		എങ്കിൽ (isalnum(ch)) { /* it's $_name or $_123 */
			ch = '_';
			പോകുക make_var1;
		}
		/* else: it's $_ */
#endif
	സ്ഥിരസ്ഥിതി:
		o_addQchr(dest, '$');
	}
	debug_printf_parse("parse_dollar return 1 (ok)\n");
	തിരികെ 1;
#undef as_string
}

#എങ്കിൽ BB_MMU
#define encode_string(as_string, dest, നിവേശം, dquote_end) \
	encode_string(dest, നിവേശം, dquote_end)
#define as_string ശൂന്യ
#endif
സ്ഥിതം പൂർണ്ണസംഖ്യ encode_string(o_string *as_string,
		o_string *dest,
		ഘടന in_str *നിവേശം,
		പൂർണ്ണസംഖ്യ dquote_end)
{
	പൂർണ്ണസംഖ്യ ch;
	പൂർണ്ണസംഖ്യ next;

 again:
	ch = i_getch(നിവേശം);
	എങ്കിൽ (ch != EOF)
		nommu_addchr(as_string, ch);
	എങ്കിൽ (ch == dquote_end) { /* may be only '"' or EOF */
		debug_printf_parse("encode_string return 1 (ok)\n");
		തിരികെ 1;
	}
	/* note: can't move it above ch == dquote_end check! */
	എങ്കിൽ (ch == EOF) {
		syntax_error_unterm_ch('"');
		തിരികെ 0; /* error */
	}
	next = '\0';
	എങ്കിൽ (ch != '\n') {
		next = i_peek(നിവേശം);
	}
	debug_printf_parse("\" ch=%c (%d) escape=%d\n",
			ch, ch, !!(dest->o_expflags & EXP_FLAG_ESC_GLOB_CHARS));
	എങ്കിൽ (ch == '\\') {
		എങ്കിൽ (next == EOF) {
			/* Testcase: in interactive shell a file with
			 *  echo "unterminated string\<eof>
			 * is sourced.
			 */
			syntax_error_unterm_ch('"');
			തിരികെ 0; /* error */
		}
		/* bash:
		 * "The backslash retains its special meaning [in "..."]
		 * only when followed by one of the following characters:
		 * $, `, ", \, or <newline>.  A double quote may be quoted
		 * within double quotes by preceding it with a backslash."
		 * NB: in (unquoted) heredoc, above does not apply to ",
		 * therefore we check for it by "next == dquote_end" cond.
		 */
		എങ്കിൽ (next == dquote_end || strchr("$`\\\n", next)) {
			ch = i_getch(നിവേശം); /* eat next */
			എങ്കിൽ (ch == '\n')
				പോകുക again; /* skip \<newline> */
		} /* else: ch remains == '\\', and we double it below: */
		o_addqchr(dest, ch); /* \c if c is a glob char, else just c */
		nommu_addchr(as_string, ch);
		പോകുക again;
	}
	എങ്കിൽ (ch == '$') {
		//if (parse_dollar_squote(as_string, dest, input))
		//	goto again;
		എങ്കിൽ (!parse_dollar(as_string, dest, നിവേശം, /*quote_mask:*/ 0x80)) {
			debug_printf_parse("encode_string return 0: "
					"parse_dollar returned 0 (error)\n");
			തിരികെ 0;
		}
		പോകുക again;
	}
#എങ്കിൽ ENABLE_HUSH_TICK
	എങ്കിൽ (ch == '`') {
		//unsigned pos = dest->length;
		o_addchr(dest, SPECIAL_VAR_ചിഹ്നം);
		o_addchr(dest, 0x80 | '`');
		എങ്കിൽ (!add_till_backquote(dest, നിവേശം, /*in_dquote:*/ dquote_end == '"'))
			തിരികെ 0; /* error */
		o_addchr(dest, SPECIAL_VAR_ചിഹ്നം);
		//debug_printf_subst("SUBST RES3 '%s'\n", dest->data + pos);
		പോകുക again;
	}
#endif
	o_addQchr(dest, ch);
	എങ്കിൽ (ch == SPECIAL_VAR_ചിഹ്നം) {
		/* Convert "^C" to corresponding special variable reference */
		o_addchr(dest, SPECIAL_VAR_QUOTED_SVS);
		o_addchr(dest, SPECIAL_VAR_ചിഹ്നം);
	}
	പോകുക again;
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
സ്ഥിതം ഘടന pipe *parse_stream(അക്ഷരം **pstring,
		പൂർണ്ണസംഖ്യ *heredoc_cnt_ptr,
		ഘടന in_str *നിവേശം,
		പൂർണ്ണസംഖ്യ end_trigger)
{
	ഘടന parse_context ctx;
	പൂർണ്ണസംഖ്യ heredoc_cnt;

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
	ആകുമ്പോൾ (1) {
		സ്ഥിരം അക്ഷരം *is_blank;
		സ്ഥിരം അക്ഷരം *is_special;
		പൂർണ്ണസംഖ്യ ch;
		പൂർണ്ണസംഖ്യ next;
#ifndef __U_BOOT__
		പൂർണ്ണസംഖ്യ redir_fd;
		redir_type redir_style;
#endif /* !__U_BOOT__ */

		ch = i_getch(നിവേശം);
		debug_printf_parse(": ch=%c (%d) escape=%d\n",
				ch, ch, !!(ctx.word.o_expflags & EXP_FLAG_ESC_GLOB_CHARS));
		എങ്കിൽ (ch == EOF) {
			ഘടന pipe *pi;

			എങ്കിൽ (heredoc_cnt) {
				syntax_error_unterm_str("here document");
				പോകുക parse_error_exitcode1;
			}
			എങ്കിൽ (end_trigger == ')') {
				syntax_error_unterm_ch('(');
				പോകുക parse_error_exitcode1;
			}
			എങ്കിൽ (end_trigger == '}') {
				syntax_error_unterm_ch('{');
				പോകുക parse_error_exitcode1;
			}

			എങ്കിൽ (done_word(&ctx)) {
				പോകുക parse_error_exitcode1;
			}
			o_free_and_set_NULL(&ctx.word);
			done_pipe(&ctx, PIPE_SEQ);

			/* Do we sit inside of any if's, loops or case's? */
			എങ്കിൽ (HAS_KEYWORDS
			IF_HAS_KEYWORDS(&& (ctx.ctx_res_w != RES_NONE || ctx.old_flag != 0))
			) {
				syntax_error_unterm_str("compound statement");
				പോകുക parse_error_exitcode1;
			}

			pi = ctx.list_head;
			/* If we got nothing... */
			/* (this makes bare "&" cmd a no-op.
			 * bash says: "syntax error near unexpected token '&'") */
			എങ്കിൽ (pi->അക്കം_cmds == 0
			IF_HAS_KEYWORDS(&& pi->res_word == RES_NONE)
			) {
				free_pipe_list(pi);
				pi = ശൂന്യ;
			}
#എങ്കിൽ !BB_MMU
			debug_printf_parse("as_string1 '%s'\n", ctx.as_string.data);
			എങ്കിൽ (pstring)
				*pstring = ctx.as_string.data;
			അല്ലെങ്കിൽ
				o_free(&ctx.as_string);
#endif
			// heredoc_cnt must be 0 here anyway
			//if (heredoc_cnt_ptr)
			//	*heredoc_cnt_ptr = heredoc_cnt;
			debug_leave();
			debug_printf_heredoc("parse_stream return heredoc_cnt:%d\n", heredoc_cnt);
			debug_printf_parse("parse_stream return %p: EOF\n", pi);
			തിരികെ pi;
		}

		/* Handle "'" and "\" first, as they won't play nice with
		 * i_peek_and_eat_bkslash_nl() anyway:
		 *   echo z\\
		 * and
		 *   echo '\
		 *   '
		 * would break.
		 */
		എങ്കിൽ (ch == '\\') {
			ch = i_getch(നിവേശം);
			എങ്കിൽ (ch == '\n')
				തുടരുക; /* drop \<newline>, get next char */
			nommu_addchr(&ctx.as_string, '\\');
			എങ്കിൽ (ch == SPECIAL_VAR_ചിഹ്നം) {
				nommu_addchr(&ctx.as_string, ch);
				/* Convert \^C to corresponding special variable reference */
				പോകുക case_SPECIAL_VAR_ചിഹ്നം;
			}
			o_addchr(&ctx.word, '\\');
			എങ്കിൽ (ch == EOF) {
				/* Testcase: eval 'echo Ok\' */
				/* bash-4.3.43 was removing backslash,
				 * but 4.4.19 retains it, most other shells too
				 */
				തുടരുക; /* get next char */
			}
			/* Example: echo Hello \2>file
			 * we need to know that word 2 is quoted
			 */
			ctx.word.has_quoted_part = 1;
			nommu_addchr(&ctx.as_string, ch);
			o_addchr(&ctx.word, ch);
			തുടരുക; /* get next char */
		}
		nommu_addchr(&ctx.as_string, ch);
		എങ്കിൽ (ch == '\'') {
			ctx.word.has_quoted_part = 1;
			next = i_getch(നിവേശം);
#ifndef __U_BOOT__
			എങ്കിൽ (next == '\'' && !ctx.pending_redirect)
				പോകുക തിരുകുക_empty_quoted_str_marker;
#endif /* !__U_BOOT__ */

			ch = next;
			ആകുമ്പോൾ (1) {
				എങ്കിൽ (ch == EOF) {
					syntax_error_unterm_ch('\'');
					പോകുക parse_error_exitcode1;
				}
				nommu_addchr(&ctx.as_string, ch);
				എങ്കിൽ (ch == '\'')
					നിർത്തുക;
				എങ്കിൽ (ch == SPECIAL_VAR_ചിഹ്നം) {
					/* Convert raw ^C to corresponding special variable reference */
					o_addchr(&ctx.word, SPECIAL_VAR_ചിഹ്നം);
					o_addchr(&ctx.word, SPECIAL_VAR_QUOTED_SVS);
				}
				o_addqchr(&ctx.word, ch);
				ch = i_getch(നിവേശം);
			}
			തുടരുക; /* get next char */
		}

		next = '\0';
		എങ്കിൽ (ch != '\n')
			next = i_peek_and_eat_bkslash_nl(നിവേശം);

		is_special = "{}<>&|();#" /* special outside of "str" */
#ifndef __U_BOOT__
				"$\"" IF_HUSH_TICK("`") /* always special */
#അല്ലെങ്കിൽ /* __U_BOOT__ */
				"$\""
#endif /* __U_BOOT__ */
				SPECIAL_VAR_ചിഹ്നം_STR;
#എങ്കിൽ defined(CMD_TEST2_SINGLEWORD_NOGLOB)
		എങ്കിൽ (ctx.command->cmd_type == CMD_TEST2_SINGLEWORD_NOGLOB) {
			/* In [[ ]], {}<>&|() are not special */
			is_special += 8;
		} അല്ലെങ്കിൽ
#endif
		/* Are { and } special here? */
		എങ്കിൽ (ctx.command->argv /* word [word]{... - non-special */
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

		എങ്കിൽ (!is_special && !is_blank) { /* ordinary char */
 ordinary_char:
			o_addQchr(&ctx.word, ch);
			എങ്കിൽ ((ctx.is_assignment == MAYBE_ASSIGNMENT
			    || ctx.is_assignment == WORD_IS_KEYWORD)
			 && ch == '='
			 && endofname(ctx.word.data)[0] == '='
			) {
				ctx.is_assignment = DEFINITELY_ASSIGNMENT;
				debug_printf_parse("ctx.is_assignment='%s'\n", assignment_flag[ctx.is_assignment]);
			}
			തുടരുക;
		}

		എങ്കിൽ (is_blank) {
#എങ്കിൽ ENABLE_HUSH_LINENO_VAR
/* Case:
 * "while ...; do<whitespace><newline>
 *	cmd ..."
 * would think that "cmd" starts in <whitespace> -
 * i.e., at the previous line.
 * We need to skip all whitespace before newlines.
 */
			ആകുമ്പോൾ (ch != '\n') {
				next = i_peek(നിവേശം);
				എങ്കിൽ (next != ' ' && next != '\t' && next != '\n')
					നിർത്തുക; /* next char is not ws */
				ch = i_getch(നിവേശം);
			}
			/* ch == last eaten whitespace char */
#endif
			എങ്കിൽ (done_word(&ctx)) {
				പോകുക parse_error_exitcode1;
			}
			എങ്കിൽ (ch == '\n') {
				/* Is this a case when newline is simply ignored?
				 * Some examples:
				 * "cmd | <newline> cmd ..."
				 * "case ... in <newline> word) ..."
				 */
				എങ്കിൽ (IS_NULL_CMD(ctx.command)
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
					ഘടന pipe *pi = ctx.list_head;
					എങ്കിൽ (pi->അക്കം_cmds != 0       /* check #1 */
					 && pi->followup != PIPE_BG /* check #2 */
					) {
						തുടരുക;
					}
				}
				/* Treat newline as a command separator. */
				done_pipe(&ctx, PIPE_SEQ);
				debug_printf_heredoc("heredoc_cnt:%d\n", heredoc_cnt);
				എങ്കിൽ (heredoc_cnt) {
					heredoc_cnt = fetch_heredocs(&ctx.as_string, ctx.list_head, heredoc_cnt, നിവേശം);
					എങ്കിൽ (heredoc_cnt != 0)
						പോകുക parse_error_exitcode1;
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
		എങ്കിൽ (ch == '}') {
#അല്ലെങ്കിൽ /* __U_BOOT__ */
		എങ്കിൽ (ch == '}' || ch == ')') {
#endif /* __U_BOOT__ */
			എങ്കിൽ (ctx.word.length != 0 /* word} */
			 || ctx.word.has_quoted_part    /* ""} */
			) {
				പോകുക ordinary_char;
			}
			എങ്കിൽ (!IS_NULL_CMD(ctx.command)) { /* cmd } */
				/* Generally, there should be semicolon: "cmd; }"
				 * However, bash allows to omit it if "cmd" is
				 * a group. Examples:
				 * { { echo 1; } }
				 * {(echo 1)}
				 * { echo 0 >&2 | { echo 1; } }
				 * { while false; do :; done }
				 * { case a in b) ;; esac }
				 */
				എങ്കിൽ (ctx.command->group)
					പോകുക term_group;
				പോകുക ordinary_char;
			}
			എങ്കിൽ (!IS_NULL_PIPE(ctx.pipe)) /* cmd | } */
				/* Can't be an end of {cmd}, skip the check */
				പോകുക skip_end_trigger;
			/* else: } does terminate a group */
		}
 term_group:
		എങ്കിൽ (end_trigger && end_trigger == ch
		 && (ch != ';' || heredoc_cnt == 0)
#എങ്കിൽ ENABLE_HUSH_CASE
		 && (ch != ')'
		    || ctx.ctx_res_w != RES_MATCH
		    || (!ctx.word.has_quoted_part && strcmp(ctx.word.data, "esac") == 0)
		    )
#endif
		) {
			എങ്കിൽ (done_word(&ctx)) {
				പോകുക parse_error_exitcode1;
			}
			done_pipe(&ctx, PIPE_SEQ);
			ctx.is_assignment = MAYBE_ASSIGNMENT;
			debug_printf_parse("ctx.is_assignment='%s'\n", assignment_flag[ctx.is_assignment]);
			/* Do we sit outside of any if's, loops or case's? */
			എങ്കിൽ (!HAS_KEYWORDS
			IF_HAS_KEYWORDS(|| (ctx.ctx_res_w == RES_NONE && ctx.old_flag == 0))
			) {
				o_free_and_set_NULL(&ctx.word);
#എങ്കിൽ !BB_MMU
				debug_printf_parse("as_string2 '%s'\n", ctx.as_string.data);
				എങ്കിൽ (pstring)
					*pstring = ctx.as_string.data;
				അല്ലെങ്കിൽ
					o_free(&ctx.as_string);
#endif
				എങ്കിൽ (ch != ';' && IS_NULL_PIPE(ctx.list_head)) {
					/* Example: bare "{ }", "()" */
					G.last_exitcode = 2; /* bash compat */
					syntax_error_unexpected_ch(ch);
					പോകുക parse_error;
				}
				എങ്കിൽ (heredoc_cnt_ptr)
					*heredoc_cnt_ptr = heredoc_cnt;
				debug_printf_heredoc("parse_stream return heredoc_cnt:%d\n", heredoc_cnt);
				debug_printf_parse("parse_stream return %p: "
						"end_trigger char found\n",
						ctx.list_head);
				debug_leave();
				തിരികെ ctx.list_head;
			}
		}

		എങ്കിൽ (is_blank)
			തുടരുക;

		/* Catch <, > before deciding whether this word is
		 * an assignment. a=1 2>z b=2: b=2 is still assignment */
		തിരഞ്ഞെടുപ്പ് (ch) {
#ifndef __U_BOOT__
		അവസ്ഥ '>':
			redir_fd = redirect_opt_അക്കം(&ctx.word);
			എങ്കിൽ (done_word(&ctx)) {
				പോകുക parse_error_exitcode1;
			}
			redir_style = REDIRECT_OVERWRITE;
			എങ്കിൽ (next == '>') {
				redir_style = REDIRECT_APPEND;
				ch = i_getch(നിവേശം);
				nommu_addchr(&ctx.as_string, ch);
			}
#എങ്കിൽ 0
			അല്ലെങ്കിൽ എങ്കിൽ (next == '(') {
				syntax_error(">(process) not supported");
				പോകുക parse_error_exitcode1;
			}
#endif
			എങ്കിൽ (parse_redirect(&ctx, redir_fd, redir_style, നിവേശം))
				പോകുക parse_error_exitcode1;
			തുടരുക; /* get next char */
		അവസ്ഥ '<':
			redir_fd = redirect_opt_അക്കം(&ctx.word);
			എങ്കിൽ (done_word(&ctx)) {
				പോകുക parse_error_exitcode1;
			}
			redir_style = REDIRECT_നിവേശം;
			എങ്കിൽ (next == '<') {
				redir_style = REDIRECT_HEREDOC;
				heredoc_cnt++;
				debug_printf_heredoc("++heredoc_cnt=%d\n", heredoc_cnt);
				ch = i_getch(നിവേശം);
				nommu_addchr(&ctx.as_string, ch);
			} അല്ലെങ്കിൽ എങ്കിൽ (next == '>') {
				redir_style = REDIRECT_IO;
				ch = i_getch(നിവേശം);
				nommu_addchr(&ctx.as_string, ch);
			}
#എങ്കിൽ 0
			അല്ലെങ്കിൽ എങ്കിൽ (next == '(') {
				syntax_error("<(process) not supported");
				പോകുക parse_error_exitcode1;
			}
#endif
			എങ്കിൽ (parse_redirect(&ctx, redir_fd, redir_style, നിവേശം))
				പോകുക parse_error_exitcode1;
			തുടരുക; /* get next char */
#അല്ലെങ്കിൽ /* __U_BOOT__ */
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
			അവസ്ഥ '>':
				fallthrough;
			അവസ്ഥ '<':
				o_addQchr(&ctx.word, ch);
				തുടരുക;
#endif /* __U_BOOT__ */
		അവസ്ഥ '#':
			എങ്കിൽ (ctx.word.length == 0 && !ctx.word.has_quoted_part) {
				/* skip "#comment" */
				/* note: we do not add it to &ctx.as_string */
/* TODO: in bash:
 * comment inside $() goes to the next \n, even inside quoted string (!):
 * cmd "$(cmd2 #comment)" - syntax error
 * cmd "`cmd2 #comment`" - ok
 * We accept both (comment ends where command subst ends, in both cases).
 */
				ആകുമ്പോൾ (1) {
					ch = i_peek(നിവേശം);
					എങ്കിൽ (ch == '\n') {
						nommu_addchr(&ctx.as_string, '\n');
						നിർത്തുക;
					}
					ch = i_getch(നിവേശം);
					എങ്കിൽ (ch == EOF)
						നിർത്തുക;
				}
				തുടരുക; /* get next char */
			}
			നിർത്തുക;
		}
 skip_end_trigger:

		എങ്കിൽ (ctx.is_assignment == MAYBE_ASSIGNMENT
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

		തിരഞ്ഞെടുപ്പ് (ch) {
		case_SPECIAL_VAR_ചിഹ്നം:
		അവസ്ഥ SPECIAL_VAR_ചിഹ്നം:
			/* Convert raw ^C to corresponding special variable reference */
			o_addchr(&ctx.word, SPECIAL_VAR_ചിഹ്നം);
			o_addchr(&ctx.word, SPECIAL_VAR_QUOTED_SVS);
			/* fall through */
		അവസ്ഥ '#':
			/* non-comment #: "echo a#b" etc */
			o_addchr(&ctx.word, ch);
			തുടരുക; /* get next char */
		അവസ്ഥ '$':
#ifndef __U_BOOT__
			എങ്കിൽ (parse_dollar_squote(&ctx.as_string, &ctx.word, നിവേശം))
				തുടരുക; /* get next char */
#endif /* !__U_BOOT__ */
			എങ്കിൽ (!parse_dollar(&ctx.as_string, &ctx.word, നിവേശം, /*quote_mask:*/ 0)) {
				debug_printf_parse("parse_stream parse error: "
					"parse_dollar returned 0 (error)\n");
				പോകുക parse_error_exitcode1;
			}
			തുടരുക; /* get next char */
		അവസ്ഥ '"':
			ctx.word.has_quoted_part = 1;
#ifndef __U_BOOT__
			എങ്കിൽ (next == '"' && !ctx.pending_redirect) {
#അല്ലെങ്കിൽ /* __U_BOOT__ */
			എങ്കിൽ (next == '"') {
#endif /* __U_BOOT__ */
				i_getch(നിവേശം); /* eat second " */
#ifndef __U_BOOT__
 തിരുകുക_empty_quoted_str_marker:
#endif /* !__U_BOOT__ */
				nommu_addchr(&ctx.as_string, next);
				o_addchr(&ctx.word, SPECIAL_VAR_ചിഹ്നം);
				o_addchr(&ctx.word, SPECIAL_VAR_ചിഹ്നം);
				തുടരുക; /* get next char */
			}
			എങ്കിൽ (ctx.is_assignment == NOT_ASSIGNMENT)
				ctx.word.o_expflags |= EXP_FLAG_ESC_GLOB_CHARS;
			എങ്കിൽ (!encode_string(&ctx.as_string, &ctx.word, നിവേശം, '"'))
				പോകുക parse_error_exitcode1;
			ctx.word.o_expflags &= ~EXP_FLAG_ESC_GLOB_CHARS;
			തുടരുക; /* get next char */
#എങ്കിൽ ENABLE_HUSH_TICK
		അവസ്ഥ '`': {
			USE_FOR_NOMMU(unsigned pos;)

			o_addchr(&ctx.word, SPECIAL_VAR_ചിഹ്നം);
			o_addchr(&ctx.word, '`');
			USE_FOR_NOMMU(pos = ctx.word.length;)
			എങ്കിൽ (!add_till_backquote(&ctx.word, നിവേശം, /*in_dquote:*/ 0))
				പോകുക parse_error_exitcode1;
# എങ്കിൽ !BB_MMU
			o_addstr(&ctx.as_string, ctx.word.data + pos);
			o_addchr(&ctx.as_string, '`');
# endif
			o_addchr(&ctx.word, SPECIAL_VAR_ചിഹ്നം);
			//debug_printf_subst("SUBST RES3 '%s'\n", ctx.word.data + pos);
			തുടരുക; /* get next char */
		}
#endif
		അവസ്ഥ ';':
#എങ്കിൽ ENABLE_HUSH_CASE
 case_semi:
#endif
			എങ്കിൽ (done_word(&ctx)) {
				പോകുക parse_error_exitcode1;
			}
			done_pipe(&ctx, PIPE_SEQ);
#എങ്കിൽ ENABLE_HUSH_CASE
			/* Eat multiple semicolons, detect
			 * whether it means something special */
			ആകുമ്പോൾ (1) {
				ch = i_peek_and_eat_bkslash_nl(നിവേശം);
				എങ്കിൽ (ch != ';')
					നിർത്തുക;
				ch = i_getch(നിവേശം);
				nommu_addchr(&ctx.as_string, ch);
				എങ്കിൽ (ctx.ctx_res_w == RES_CASE_BODY) {
					ctx.ctx_dsemicolon = 1;
					ctx.ctx_res_w = RES_MATCH;
					നിർത്തുക;
				}
			}
#endif
 new_cmd:
			/* We just finished a cmd. New one may start
			 * with an assignment */
			ctx.is_assignment = MAYBE_ASSIGNMENT;
			debug_printf_parse("ctx.is_assignment='%s'\n", assignment_flag[ctx.is_assignment]);
			തുടരുക; /* get next char */
		അവസ്ഥ '&':
			എങ്കിൽ (done_word(&ctx)) {
				പോകുക parse_error_exitcode1;
			}
			എങ്കിൽ (next == '&') {
				ch = i_getch(നിവേശം);
				nommu_addchr(&ctx.as_string, ch);
				done_pipe(&ctx, PIPE_AND);
			} അല്ലെങ്കിൽ {
				done_pipe(&ctx, PIPE_BG);
			}
			പോകുക new_cmd;
		അവസ്ഥ '|':
			എങ്കിൽ (done_word(&ctx)) {
				പോകുക parse_error_exitcode1;
			}
#എങ്കിൽ ENABLE_HUSH_CASE
			എങ്കിൽ (ctx.ctx_res_w == RES_MATCH)
				നിർത്തുക; /* we are in case's "word | word)" */
#endif
			എങ്കിൽ (next == '|') { /* || */
				ch = i_getch(നിവേശം);
				nommu_addchr(&ctx.as_string, ch);
				done_pipe(&ctx, PIPE_OR);
			} അല്ലെങ്കിൽ {
				/* we could pick up a file descriptor choice here
				 * with redirect_opt_num(), but bash doesn't do it.
				 * "echo foo 2| cat" yields "foo 2". */
				done_command(&ctx);
			}
			പോകുക new_cmd;
		അവസ്ഥ '(':
#എങ്കിൽ ENABLE_HUSH_CASE
			/* "case... in [(]word)..." - skip '(' */
			എങ്കിൽ (ctx.ctx_res_w == RES_MATCH
			 && ctx.command->argv == ശൂന്യ /* not (word|(... */
			 && ctx.word.length == 0 /* not word(... */
			 && ctx.word.has_quoted_part == 0 /* not ""(... */
			) {
				തുടരുക; /* get next char */
			}
#endif
			/* fall through */
		അവസ്ഥ '{': {
			പൂർണ്ണസംഖ്യ n = parse_group(&ctx, നിവേശം, ch);
			എങ്കിൽ (n < 0) {
				പോകുക parse_error_exitcode1;
			}
			debug_printf_heredoc("parse_group done, needs heredocs:%d\n", n);
			heredoc_cnt += n;
			പോകുക new_cmd;
		}
		അവസ്ഥ ')':
#എങ്കിൽ ENABLE_HUSH_CASE
			എങ്കിൽ (ctx.ctx_res_w == RES_MATCH)
				പോകുക case_semi;
#endif
		അവസ്ഥ '}':
			/* proper use of this character is caught by end_trigger:
			 * if we see {, we call parse_group(..., end_trigger='}')
			 * and it will match } earlier (not here). */
			G.last_exitcode = 2;
			syntax_error_unexpected_ch(ch);
			പോകുക parse_error;
		സ്ഥിരസ്ഥിതി:
			എങ്കിൽ (HUSH_DEBUG)
				bb_error_സന്ദേശം_and_die("BUG: unexpected %c", ch);
		}
	} /* while (1) */

 parse_error_exitcode1:
	G.last_exitcode = 1;
 parse_error:
	{
		ഘടന parse_context *pctx;
		IF_HAS_KEYWORDS(ഘടന parse_context *p2;)

		/* Clean up allocated tree.
		 * Sample for finding leaks on syntax error recovery path.
		 * Run it from interactive shell, watch pmap `pidof hush`.
		 * while if false; then false; fi; do break; fi
		 * Samples to catch leaks at execution:
		 * while if (true | { true;}); then echo ok; fi; do break; done
		 * while if (true | { true;}); then echo ok; fi; do (if echo ok; break; then :; fi) | cat; break; done
		 */
		pctx = &ctx;
		ചെയ്യുക {
			/* Update pipe/command counts,
			 * otherwise freeing may miss some */
			done_pipe(pctx, PIPE_SEQ);
			debug_printf_clean("freeing list %p from ctx %p\n",
					pctx->list_head, pctx);
			debug_print_വൃക്ഷം(pctx->list_head, 0);
			free_pipe_list(pctx->list_head);
			debug_printf_clean("freed list %p\n", pctx->list_head);
#എങ്കിൽ !BB_MMU
			o_free(&pctx->as_string);
#endif
			IF_HAS_KEYWORDS(p2 = pctx->അട്ടി;)
			എങ്കിൽ (pctx != &ctx) {
				സ്വതന്ത്രം(pctx);
			}
			IF_HAS_KEYWORDS(pctx = p2;)
		} ആകുമ്പോൾ (HAS_KEYWORDS && pctx);

		o_free(&ctx.word);
#എങ്കിൽ !BB_MMU
		എങ്കിൽ (pstring)
			*pstring = ശൂന്യ;
#endif
		debug_leave();
		തിരികെ ERR_PTR;
	}
}

/*** Execution routines ***/

/* Expansion can recurse, need forward decls: */
#എങ്കിൽ !BASH_PATTERN_SUBST && !ENABLE_HUSH_CASE
#define expand_string_to_string(str, EXP_flags, do_unbackslash) \
	expand_string_to_string(str)
#endif
സ്ഥിതം അക്ഷരം *expand_string_to_string(സ്ഥിരം അക്ഷരം *str, പൂർണ്ണസംഖ്യ EXP_flags, പൂർണ്ണസംഖ്യ do_unbackslash);
#എങ്കിൽ ENABLE_HUSH_TICK
സ്ഥിതം പൂർണ്ണസംഖ്യ process_command_subs(o_string *dest, സ്ഥിരം അക്ഷരം *s);
#endif
സ്ഥിതം പൂർണ്ണസംഖ്യ expand_vars_to_list(o_string *output, പൂർണ്ണസംഖ്യ n, അക്ഷരം *arg);

/* expand_strvec_to_strvec() takes a list of strings, expands
 * all variable references within and returns a pointer to
 * a list of expanded strings, possibly with larger number
 * of strings. (Think VAR="a b"; echo $VAR).
 * This new list is allocated as a single malloc block.
 * NULL-terminated list of char* pointers is at the beginning of it,
 * followed by strings themselves.
 * Caller can deallocate entire list by single free(list). */

/* A horde of its helpers come first: */

സ്ഥിതം ശൂന്യം o_addblock_duplicate_backslash(o_string *o, സ്ഥിരം അക്ഷരം *str, പൂർണ്ണസംഖ്യ len)
{
	ആകുമ്പോൾ (--len >= 0) {
		അക്ഷരം c = *str++;

#എങ്കിൽ ENABLE_HUSH_BRACE_EXPANSION
		എങ്കിൽ (c == '{' || c == '}') {
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
		എങ്കിൽ (c == '\\') {
			/* \z -> \\\z; \<eol> -> \\<eol> */
			o_addchr(o, '\\');
			എങ്കിൽ (len) {
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
സ്ഥിതം പൂർണ്ണസംഖ്യ expand_on_ifs(o_string *output, പൂർണ്ണസംഖ്യ n, സ്ഥിരം അക്ഷരം *str)
{
	പൂർണ്ണസംഖ്യ last_is_ifs = 0;

	ആകുമ്പോൾ (1) {
		പൂർണ്ണസംഖ്യ word_len;

		എങ്കിൽ (!*str)  /* EOL - do not finalize word */
			നിർത്തുക;
		word_len = strcspn(str, G.ifs);
		എങ്കിൽ (word_len) {
			/* We have WORD_LEN leading non-IFS chars */
			എങ്കിൽ (!(output->o_expflags & EXP_FLAG_GLOB)) {
				o_addblock(output, str, word_len);
			} അല്ലെങ്കിൽ {
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
			എങ്കിൽ (!*str)  /* EOL - do not finalize word */
				നിർത്തുക;
		}

		/* We know str here points to at least one IFS char */
		last_is_ifs = 1;
		str += strspn(str, G.ifs_whitespace); /* skip IFS whitespace chars */
		എങ്കിൽ (!*str)  /* EOL - do not finalize word */
			നിർത്തുക;

		എങ്കിൽ (G.ifs_whitespace != G.ifs /* usually false ($IFS is usually all whitespace), */
		 && strchr(G.ifs, *str)       /* the second check would fail */
		) {
			/* This is a non-whitespace $IFS char */
			/* Skip it and IFS whitespace chars, start new word */
			str++;
			str += strspn(str, G.ifs_whitespace);
			പോകുക new_word;
		}

		/* Start new word... but not always! */
		/* Case "v=' a'; echo ''$v": we do need to finalize empty word: */
		എങ്കിൽ (output->has_quoted_part
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
	തിരികെ n;
}

#ifndef __U_BOOT__
/* Helper to expand $((...)) and heredoc body. These act as if
 * they are in double quotes, with the exception that they are not :).
 * Just the rules are similar: "expand only $var and `cmd`"
 *
 * Returns malloced string.
 * As an optimization, we return NULL if expansion is not needed.
 */
സ്ഥിതം അക്ഷരം *encode_then_expand_string(സ്ഥിരം അക്ഷരം *str)
{
	അക്ഷരം *exp_str;
	ഘടന in_str നിവേശം;
	o_string dest = NULL_O_STRING;
	സ്ഥിരം അക്ഷരം *cp;

	cp = str;
	വേണ്ടി (;;) {
		എങ്കിൽ (!*cp) തിരികെ ശൂന്യ; /* string has no special chars */
		എങ്കിൽ (*cp == '$') നിർത്തുക;
		എങ്കിൽ (*cp == '\\') നിർത്തുക;
#എങ്കിൽ ENABLE_HUSH_TICK
		എങ്കിൽ (*cp == '`') നിർത്തുക;
#endif
		cp++;
	}

	/* We need to expand. Example:
	 * echo $(($a + `echo 1`)) $((1 + $((2)) ))
	 */
	setup_string_in_str(&നിവേശം, str);
	encode_string(ശൂന്യ, &dest, &നിവേശം, EOF);
//TODO: error check (encode_string returns 0 on error)?
	//bb_error_msg("'%s' -> '%s'", str, dest.data);
	exp_str = expand_string_to_string(dest.data,
			EXP_FLAG_ESC_GLOB_CHARS,
			/*unbackslash:*/ 1
	);
	//bb_error_msg("'%s' -> '%s'", dest.data, exp_str);
	o_free(&dest);
	തിരികെ exp_str;
}

സ്ഥിതം സ്ഥിരം അക്ഷരം *first_special_char_in_vararg(സ്ഥിരം അക്ഷരം *cp)
{
	വേണ്ടി (;;) {
		എങ്കിൽ (!*cp) തിരികെ ശൂന്യ; /* string has no special chars */
		എങ്കിൽ (*cp == '$') തിരികെ cp;
		എങ്കിൽ (*cp == '\\') തിരികെ cp;
		എങ്കിൽ (*cp == '\'') തിരികെ cp;
		എങ്കിൽ (*cp == '"') തിരികെ cp;
#എങ്കിൽ ENABLE_HUSH_TICK
		എങ്കിൽ (*cp == '`') തിരികെ cp;
#endif
		/* dquoted "${x:+ARG}" should not glob, therefore
		 * '*' et al require some non-literal processing: */
		എങ്കിൽ (*cp == '*') തിരികെ cp;
		എങ്കിൽ (*cp == '?') തിരികെ cp;
		എങ്കിൽ (*cp == '[') തിരികെ cp;
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
#എങ്കിൽ !BASH_PATTERN_SUBST
#define encode_then_expand_vararg(str, handle_squotes, do_unbackslash) \
	encode_then_expand_vararg(str, handle_squotes)
#endif
സ്ഥിതം അക്ഷരം *encode_then_expand_vararg(സ്ഥിരം അക്ഷരം *str, പൂർണ്ണസംഖ്യ handle_squotes, പൂർണ്ണസംഖ്യ do_unbackslash)
{
#എങ്കിൽ !BASH_PATTERN_SUBST && ENABLE_HUSH_CASE
	സ്ഥിരം പൂർണ്ണസംഖ്യ do_unbackslash = 0;
#endif
	അക്ഷരം *exp_str;
	ഘടന in_str നിവേശം;
	o_string dest = NULL_O_STRING;

	എങ്കിൽ (!first_special_char_in_vararg(str)) {
		/* string has no special chars */
		തിരികെ ശൂന്യ;
	}

	setup_string_in_str(&നിവേശം, str);
	dest.data = xzalloc(1); /* start as "", not as NULL */
	exp_str = ശൂന്യ;

	വേണ്ടി (;;) {
		പൂർണ്ണസംഖ്യ ch;

		ch = i_getch(&നിവേശം);
		debug_printf_parse("%s: ch=%c (%d) escape=%d\n",
				__func__, ch, ch, !!dest.o_expflags);

		എങ്കിൽ (!dest.o_expflags) {
			എങ്കിൽ (ch == EOF)
				നിർത്തുക;
			എങ്കിൽ (handle_squotes && ch == '\'') {
				എങ്കിൽ (!add_till_single_quote_dquoted(&dest, &നിവേശം))
					പോകുക ret; /* error */
				തുടരുക;
			}
		}
		എങ്കിൽ (ch == EOF) {
			syntax_error_unterm_ch('"');
			പോകുക ret; /* error */
		}
		എങ്കിൽ (ch == '"') {
			dest.o_expflags ^= EXP_FLAG_ESC_GLOB_CHARS;
			തുടരുക;
		}
		എങ്കിൽ (ch == '\\') {
			ch = i_getch(&നിവേശം);
			എങ്കിൽ (ch == EOF) {
//example? error message?	syntax_error_unterm_ch('"');
				debug_printf_parse("%s: error: \\<eof>\n", __func__);
				പോകുക ret;
			}
			o_addqchr(&dest, ch);
			തുടരുക;
		}
		എങ്കിൽ (ch == '$') {
			എങ്കിൽ (parse_dollar_squote(ശൂന്യ, &dest, &നിവേശം))
				തുടരുക;
			എങ്കിൽ (!parse_dollar(ശൂന്യ, &dest, &നിവേശം, /*quote_mask:*/ 0x80)) {
				debug_printf_parse("%s: error: parse_dollar returned 0 (error)\n", __func__);
				പോകുക ret;
			}
			തുടരുക;
		}
#എങ്കിൽ ENABLE_HUSH_TICK
		എങ്കിൽ (ch == '`') {
			//unsigned pos = dest->length;
			o_addchr(&dest, SPECIAL_VAR_ചിഹ്നം);
			o_addchr(&dest, 0x80 | '`');
			എങ്കിൽ (!add_till_backquote(&dest, &നിവേശം,
					/*in_dquote:*/ dest.o_expflags /* nonzero if EXP_FLAG_ESC_GLOB_CHARS set */
				)
			) {
				പോകുക ret; /* error */
			}
			o_addchr(&dest, SPECIAL_VAR_ചിഹ്നം);
			//debug_printf_subst("SUBST RES3 '%s'\n", dest->data + pos);
			തുടരുക;
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
	തിരികെ exp_str;
}

/* Expanding ARG in ${var+ARG}, ${var-ARG}
 */
സ്ഥിതം NOINLINE പൂർണ്ണസംഖ്യ encode_then_append_var_plusminus(o_string *output, പൂർണ്ണസംഖ്യ n,
		അക്ഷരം *str, പൂർണ്ണസംഖ്യ dquoted)
{
	ഘടന in_str നിവേശം;
	o_string dest = NULL_O_STRING;

	എങ്കിൽ (!first_special_char_in_vararg(str)
	 && '\0' == str[strcspn(str, G.ifs)]
	) {
		/* string has no special chars
		 * && string has no $IFS chars
		 */
		എങ്കിൽ (dquoted) {
			/* Prints 1 (quoted expansion is a "" word, not nothing):
			 * set -- "${notexist-}"; echo $#
			 */
			output->has_quoted_part = 1;
		}
		തിരികെ expand_vars_to_list(output, n, str);
	}

	setup_string_in_str(&നിവേശം, str);

	വേണ്ടി (;;) {
		പൂർണ്ണസംഖ്യ ch;

		ch = i_getch(&നിവേശം);
		debug_printf_parse("%s: ch=%c (%d) escape=%x\n",
				__func__, ch, ch, dest.o_expflags);

		എങ്കിൽ (!dest.o_expflags) {
			എങ്കിൽ (ch == EOF)
				നിർത്തുക;
			എങ്കിൽ (!dquoted && !(output->o_expflags & EXP_FLAG_SINGLEWORD) && strchr(G.ifs, ch)) {
				/* PREFIX${x:d${e}f ...} and we met space: expand "d${e}f" and start new word.
				 * do not assume we are at the start of the word (PREFIX above).
				 */
				എങ്കിൽ (dest.data) {
					n = expand_vars_to_list(output, n, dest.data);
					o_free_and_set_NULL(&dest);
					o_addchr(output, '\0');
					n = o_save_ptr(output, n); /* create next word */
				} അല്ലെങ്കിൽ
				എങ്കിൽ (output->length != o_get_last_ptr(output, n)
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
				തുടരുക;
			}
			എങ്കിൽ (!dquoted && ch == '\'') {
				എങ്കിൽ (!add_till_single_quote_dquoted(&dest, &നിവേശം))
					പോകുക ret; /* error */
				o_addchr(&dest, SPECIAL_VAR_ചിഹ്നം);
				o_addchr(&dest, SPECIAL_VAR_ചിഹ്നം);
				തുടരുക;
			}
		}
		എങ്കിൽ (ch == EOF) {
			syntax_error_unterm_ch('"');
			പോകുക ret; /* error */
		}
		എങ്കിൽ (ch == '"') {
			dest.o_expflags ^= EXP_FLAG_ESC_GLOB_CHARS;
			എങ്കിൽ (dest.o_expflags) {
				o_addchr(&dest, SPECIAL_VAR_ചിഹ്നം);
				o_addchr(&dest, SPECIAL_VAR_ചിഹ്നം);
			}
			തുടരുക;
		}
		എങ്കിൽ (ch == '\\') {
			ch = i_getch(&നിവേശം);
			എങ്കിൽ (ch == EOF) {
//example? error message?	syntax_error_unterm_ch('"');
				debug_printf_parse("%s: error: \\<eof>\n", __func__);
				പോകുക ret;
			}
			o_addqchr(&dest, ch);
			തുടരുക;
		}
		എങ്കിൽ (ch == '$') {
			എങ്കിൽ (!parse_dollar(ശൂന്യ, &dest, &നിവേശം, /*quote_mask:*/ (dest.o_expflags || dquoted) ? 0x80 : 0)) {
				debug_printf_parse("%s: error: parse_dollar returned 0 (error)\n", __func__);
				പോകുക ret;
			}
			തുടരുക;
		}
#എങ്കിൽ ENABLE_HUSH_TICK
		എങ്കിൽ (ch == '`') {
			//unsigned pos = dest->length;
			o_addchr(&dest, SPECIAL_VAR_ചിഹ്നം);
			o_addchr(&dest, (dest.o_expflags || dquoted) ? 0x80 | '`' : '`');
			എങ്കിൽ (!add_till_backquote(&dest, &നിവേശം,
					/*in_dquote:*/ dest.o_expflags /* nonzero if EXP_FLAG_ESC_GLOB_CHARS set */
				)
			) {
				പോകുക ret; /* error */
			}
			o_addchr(&dest, SPECIAL_VAR_ചിഹ്നം);
			//debug_printf_subst("SUBST RES3 '%s'\n", dest->data + pos);
			തുടരുക;
		}
#endif
		എങ്കിൽ (dquoted) {
			/* Always glob-protect if in dquotes:
			 * x=x; echo "${x:+/bin/c*}" - prints: /bin/c*
			 * x=x; echo "${x:+"/bin/c*"}" - prints: /bin/c*
			 */
			o_addqchr(&dest, ch);
		} അല്ലെങ്കിൽ {
			/* Glob-protect only if char is quoted:
			 * x=x; echo ${x:+/bin/c*} - prints many filenames
			 * x=x; echo ${x:+"/bin/c*"} - prints: /bin/c*
			 */
			o_addQchr(&dest, ch);
		}
	} /* for (;;) */

	എങ്കിൽ (dest.data) {
		n = expand_vars_to_list(output, n, dest.data);
	}
 ret:
	o_free(&dest);
	തിരികെ n;
}
#endif /* !__U_BOOT__ */

#ifndef __U_BOOT__
#എങ്കിൽ ENABLE_FEATURE_SH_MATH
സ്ഥിതം arith_t expand_and_evaluate_arith(സ്ഥിരം അക്ഷരം *arg, സ്ഥിരം അക്ഷരം **errmsg_p)
{
	arith_state_t math_state;
	arith_t res;
	അക്ഷരം *exp_str;

	math_state.lookupvar = get_local_var_മൂല്യം;
	math_state.setvar = set_local_var_from_halves;
	//math_state.endofname = endofname;
	exp_str = encode_then_expand_string(arg);
	res = arith(&math_state, exp_str ? exp_str : arg);
	സ്വതന്ത്രം(exp_str);
	എങ്കിൽ (errmsg_p)
		*errmsg_p = math_state.errmsg;
	എങ്കിൽ (math_state.errmsg)
		സന്ദേശം_and_die_if_script(math_state.errmsg);
	തിരികെ res;
}
#endif
#endif /* !__U_BOOT__ */

#ifndef __U_BOOT__
#എങ്കിൽ BASH_PATTERN_SUBST
/* ${var/[/]pattern[/repl]} helpers */
സ്ഥിതം അക്ഷരം *strstr_pattern(അക്ഷരം *val, സ്ഥിരം അക്ഷരം *pattern, പൂർണ്ണസംഖ്യ *size)
{
	പൂർണ്ണസംഖ്യ first_escaped = (pattern[0] == '\\' && pattern[1]);
	/* "first_escaped" trick allows to treat e.g. "\*no_glob_chars"
	 * as literal too (as it is semi-common, and easy to accomodate
	 * by just using str + 1).
	 */
	പൂർണ്ണസംഖ്യ sz = strcspn(pattern + first_escaped * 2, "*?[\\");
	എങ്കിൽ ((pattern + first_escaped * 2)[sz] == '\0') {
		/* Optimization for trivial patterns.
		 * Testcase for very slow replace (performs about 22k replaces):
		 * x=::::::::::::::::::::::
		 * x=$x$x;x=$x$x;x=$x$x;x=$x$x;x=$x$x;x=$x$x;x=$x$x;x=$x$x;x=$x$x;x=$x$x;echo ${#x}
		 * echo "${x//:/|}"
		 */
		*size = sz + first_escaped;
		തിരികെ strstr(val, pattern + first_escaped);
	}

	ആകുമ്പോൾ (1) {
		അക്ഷരം *end = scan_and_match(val, pattern, SCAN_MOVE_FROM_വലത് + SCAN_MATCH_ഇടത്_HALF);
		debug_printf_varexp("val:'%s' pattern:'%s' end:'%s'\n", val, pattern, end);
		എങ്കിൽ (end) {
			*size = end - val;
			തിരികെ val;
		}
		എങ്കിൽ (*val == '\0')
			തിരികെ ശൂന്യ;
		/* Optimization: if "*pat" did not match the start of "string",
		 * we know that "tring", "ring" etc will not match too:
		 */
		എങ്കിൽ (pattern[0] == '*')
			തിരികെ ശൂന്യ;
		val++;
	}
}
സ്ഥിതം അക്ഷരം *replace_pattern(അക്ഷരം *val, സ്ഥിരം അക്ഷരം *pattern, സ്ഥിരം അക്ഷരം *repl, അക്ഷരം exp_op)
{
	അക്ഷരം *result = ശൂന്യ;
	unsigned res_len = 0;
	unsigned repl_len = strlen(repl);

	/* Null pattern never matches, including if "var" is empty */
	എങ്കിൽ (!pattern[0])
		തിരികെ result; /* NULL, no replaces happened */

	ആകുമ്പോൾ (1) {
		പൂർണ്ണസംഖ്യ size;
		അക്ഷരം *s = strstr_pattern(val, pattern, &size);
		എങ്കിൽ (!s)
			നിർത്തുക;

		result = xrealloc(result, res_len + (s - val) + repl_len + 1);
		strcpy(mempcpy(result + res_len, val, s - val), repl);
		res_len += (s - val) + repl_len;
		debug_printf_varexp("val:'%s' s:'%s' result:'%s'\n", val, s, result);

		val = s + size;
		എങ്കിൽ (exp_op == '/')
			നിർത്തുക;
	}
	എങ്കിൽ (*val && result) {
		result = xrealloc(result, res_len + strlen(val) + 1);
		strcpy(result + res_len, val);
		debug_printf_varexp("val:'%s' result:'%s'\n", val, result);
	}
	debug_printf_varexp("result:'%s'\n", result);
	തിരികെ result;
}
#endif /* BASH_PATTERN_SUBST */
#endif /* !__U_BOOT__ */

സ്ഥിതം പൂർണ്ണസംഖ്യ append_str_maybe_ifs_split(o_string *output, പൂർണ്ണസംഖ്യ n,
		പൂർണ്ണസംഖ്യ first_ch, സ്ഥിരം അക്ഷരം *val)
{
	എങ്കിൽ (!(first_ch & 0x80)) { /* unquoted $VAR */
		debug_printf_expand("unquoted '%s', output->o_escape:%d\n", val,
				!!(output->o_expflags & EXP_FLAG_ESC_GLOB_CHARS));
		എങ്കിൽ (val && val[0])
			n = expand_on_ifs(output, n, val);
	} അല്ലെങ്കിൽ { /* quoted "$VAR" */
		output->has_quoted_part = 1;
		debug_printf_expand("quoted '%s', output->o_escape:%d\n", val,
				!!(output->o_expflags & EXP_FLAG_ESC_GLOB_CHARS));
		എങ്കിൽ (val && val[0])
			o_addQstr(output, val);
	}
	തിരികെ n;
}

/* Handle <SPECIAL_VAR_SYMBOL>varname...<SPECIAL_VAR_SYMBOL> construct.
 */
സ്ഥിതം NOINLINE പൂർണ്ണസംഖ്യ expand_one_var(o_string *output, പൂർണ്ണസംഖ്യ n,
		പൂർണ്ണസംഖ്യ first_ch, അക്ഷരം *arg, അക്ഷരം **pp)
{
	സ്ഥിരം അക്ഷരം *val;
	അക്ഷരം *to_be_freed;
	അക്ഷരം *p;
	അക്ഷരം *var;
	അക്ഷരം exp_op;
	അക്ഷരം exp_save = exp_save; /* for compiler */
	അക്ഷരം *exp_saveptr; /* points to expansion operator */
	അക്ഷരം *exp_word = exp_word; /* for compiler */
	അക്ഷരം arg0;

	val = ശൂന്യ;
	to_be_freed = ശൂന്യ;
	p = *pp;
	*p = '\0'; /* replace trailing SPECIAL_VAR_SYMBOL */
	var = arg;
	exp_saveptr = arg[1] ? strchr(VAR_ENCODED_SUBST_OPS, arg[1]) : ശൂന്യ;
	arg0 = arg[0];
	arg[0] = (arg0 & 0x7f);
	exp_op = 0;

	എങ്കിൽ (arg[0] == '#' && arg[1] /* ${#...} but not ${#} */
	 && (!exp_saveptr               /* and ( not(${#<op_char>...}) */
	    || (arg[2] == '\0' && strchr(SPECIAL_VARS_STR, arg[1])) /* or ${#C} "len of $C" ) */
	    )		/* NB: skipping ^^^specvar check mishandles ${#::2} */
	) {
		/* It must be length operator: ${#var} */
		var++;
		exp_op = 'L';
	} അല്ലെങ്കിൽ {
		/* Maybe handle parameter expansion */
		എങ്കിൽ (exp_saveptr /* if 2nd char is one of expansion operators */
		 && strchr(NUMERIC_SPECVARS_STR, arg[0]) /* 1st char is special variable */
		) {
			/* ${?:0}, ${#[:]%0} etc */
			exp_saveptr = var + 1;
		} അല്ലെങ്കിൽ {
			/* ${?}, ${var}, ${var:0}, ${var[:]%0} etc */
			exp_saveptr = var+1 + strcspn(var+1, VAR_ENCODED_SUBST_OPS);
		}
		exp_op = exp_save = *exp_saveptr;
#ifndef __U_BOOT__
		എങ്കിൽ (exp_op) {
			exp_word = exp_saveptr + 1;
			എങ്കിൽ (exp_op == ':') {
				exp_op = *exp_word++;
//TODO: try ${var:} and ${var:bogus} in non-bash config
				എങ്കിൽ (BASH_SUBSTR
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
	എങ്കിൽ (isdigit(var[0])) {
		/* parse_dollar should have vetted var for us */
#ifndef __U_BOOT__
		പൂർണ്ണസംഖ്യ nn = xatoi_positive(var);
#അല്ലെങ്കിൽ /* __U_BOOT__ */
		പൂർണ്ണസംഖ്യ nn = simple_strtoul(var, ശൂന്യ, 10);
#endif /* __U_BOOT__ */
		എങ്കിൽ (nn < G.global_argc)
			val = G.global_argv[nn];
		/* else val remains NULL: $N with too big N */
	} അല്ലെങ്കിൽ {
		തിരഞ്ഞെടുപ്പ് (var[0]) {
#ifndef __U_BOOT__
		അവസ്ഥ '$': /* pid */
			val = utoa(G.വേര്_pid);
			നിർത്തുക;
		അവസ്ഥ '!': /* bg pid */
			val = G.last_bg_pid ? utoa(G.last_bg_pid) : "";
			നിർത്തുക;
#endif /* !__U_BOOT__ */
		അവസ്ഥ '?': /* exitcode */
			val = utoa(G.last_exitcode);
			നിർത്തുക;
		അവസ്ഥ '#': /* argc */
			val = utoa(G.global_argc ? G.global_argc-1 : 0);
			നിർത്തുക;
#ifndef __U_BOOT__
		അവസ്ഥ '-': { /* active options */
			/* Check set_mode() to see what option chars we support */
			അക്ഷരം *cp;
			val = cp = G.optstring_buf;
			എങ്കിൽ (G.o_opt[OPT_O_ERREXIT])
				*cp++ = 'e';
			എങ്കിൽ (G_interactive_fd)
				*cp++ = 'i';
			എങ്കിൽ (G_x_mode)
				*cp++ = 'x';
			/* If G.o_opt[OPT_O_NOEXEC] is true,
			 * commands read but are not executed,
			 * so $- can not execute too, 'n' is never seen in $-.
			 */
			എങ്കിൽ (G.opt_c)
				*cp++ = 'c';
			എങ്കിൽ (G.opt_s)
				*cp++ = 's';
			*cp = '\0';
			നിർത്തുക;
		}
#endif /* !__U_BOOT__ */
		സ്ഥിരസ്ഥിതി:
#ifndef __U_BOOT__
			val = get_local_var_മൂല്യം(var);
#അല്ലെങ്കിൽ /* __U_BOOT__ */
			/*
			 * Environment variable set with setenv* have to be
			 * expanded.
			 * So, we first search if the variable exists in
			 * environment, if this is not the case, we default to
			 * local value.
			 */
			val = env_get(var);
			എങ്കിൽ (!val)
				val = get_local_var_മൂല്യം(var);
#endif /* __U_BOOT__ */
		}
	}

#ifndef __U_BOOT__
	/* Handle any expansions */
	എങ്കിൽ (exp_op == 'L') {
		reinit_unicode_for_hush();
		debug_printf_expand("expand: length(%s)=", val);
		val = utoa(val ? unicode_strlen(val) : 0);
		debug_printf_expand("%s\n", val);
	} അല്ലെങ്കിൽ എങ്കിൽ (exp_op) {
		എങ്കിൽ (exp_op == '%' || exp_op == '#') {
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
			എങ്കിൽ (val && val[0]) {
				അക്ഷരം *t;
				അക്ഷരം *exp_exp_word;
				അക്ഷരം *loc;
				unsigned scan_flags = pick_scan(exp_op, *exp_word);
				എങ്കിൽ (exp_op == *exp_word)  /* ## or %% */
					exp_word++;
				debug_printf_expand("expand: exp_word:'%s'\n", exp_word);
				exp_exp_word = encode_then_expand_vararg(exp_word, /*handle_squotes:*/ 1, /*unbackslash:*/ 0);
				എങ്കിൽ (exp_exp_word)
					exp_word = exp_exp_word;
				debug_printf_expand("expand: exp_word:'%s'\n", exp_word);
				/*
				 * HACK ALERT. We depend here on the fact that
				 * G.global_argv and results of utoa and get_local_var_value
				 * are actually in writable memory:
				 * scan_and_match momentarily stores NULs there.
				 */
				t = (അക്ഷരം*)val;
				loc = scan_and_match(t, exp_word, scan_flags);
				debug_printf_expand("op:%c str:'%s' pat:'%s' res:'%s'\n", exp_op, t, exp_word, loc);
				സ്വതന്ത്രം(exp_exp_word);
				എങ്കിൽ (loc) { /* match was found */
					എങ്കിൽ (scan_flags & SCAN_MATCH_ഇടത്_HALF) /* #[#] */
						val = loc; /* take right part */
					അല്ലെങ്കിൽ /* %[%] */
						val = to_be_freed = xstrndup(val, loc - val); /*ഇടത്*/
				}
			}
		}
#എങ്കിൽ BASH_PATTERN_SUBST
		അല്ലെങ്കിൽ എങ്കിൽ (exp_op == '/' || exp_op == '\\') {
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
			എങ്കിൽ (val /*&& val[0]*/) {
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
				അക്ഷരം *pattern, *repl, *t;
				pattern = encode_then_expand_vararg(exp_word, /*handle_squotes:*/ 1, /*unbackslash:*/ 0);
				എങ്കിൽ (!pattern)
					pattern = xstrdup(exp_word);
				debug_printf_varexp("pattern:'%s'->'%s'\n", exp_word, pattern);
				*p++ = SPECIAL_VAR_ചിഹ്നം;
				exp_word = p;
				p = strchr(p, SPECIAL_VAR_ചിഹ്നം);
				*p = '\0';
				repl = encode_then_expand_vararg(exp_word, /*handle_squotes:*/ 1, /*unbackslash:*/ 1);
				debug_printf_varexp("repl:'%s'->'%s'\n", exp_word, repl);
				/* HACK ALERT. We depend here on the fact that
				 * G.global_argv and results of utoa and get_local_var_value
				 * are actually in writable memory:
				 * replace_pattern momentarily stores NULs there. */
				t = (അക്ഷരം*)val;
				to_be_freed = replace_pattern(t,
						pattern,
						(repl ? repl : exp_word),
						exp_op);
				എങ്കിൽ (to_be_freed) /* at least one replace happened */
					val = to_be_freed;
				സ്വതന്ത്രം(pattern);
				സ്വതന്ത്രം(repl);
			} അല്ലെങ്കിൽ {
				/* Unset variable always gives nothing */
				//  a=; echo ${a/*/w}      # "w"
				//  unset a; echo ${a/*/w} # ""
				/* Just skip "replace" part */
				*p++ = SPECIAL_VAR_ചിഹ്നം;
				p = strchr(p, SPECIAL_VAR_ചിഹ്നം);
				*p = '\0';
			}
		}
#endif /* BASH_PATTERN_SUBST */
		അല്ലെങ്കിൽ എങ്കിൽ (exp_op == ':') {
#എങ്കിൽ BASH_SUBSTR && ENABLE_FEATURE_SH_MATH
			/* It's ${var:N[:M]} bashism.
			 * Note that in encoded form it has TWO parts:
			 * var:N<SPECIAL_VAR_SYMBOL>M<SPECIAL_VAR_SYMBOL>
			 */
			arith_t beg, len;
			unsigned vallen;
			സ്ഥിരം അക്ഷരം *errmsg;

			beg = expand_and_evaluate_arith(exp_word, &errmsg);
			എങ്കിൽ (errmsg)
				പോകുക empty_result;
			debug_printf_varexp("beg:'%s'=%lld\n", exp_word, (long long)beg);
			*p++ = SPECIAL_VAR_ചിഹ്നം;
			exp_word = p;
			p = strchr(p, SPECIAL_VAR_ചിഹ്നം);
			*p = '\0';
			vallen = val ? strlen(val) : 0;
			എങ്കിൽ (beg < 0) {
				/* negative beg counts from the end */
				beg = (arith_t)vallen + beg;
			}
			/* If expansion will be empty, do not even evaluate len */
			എങ്കിൽ (!val || beg < 0 || beg > vallen) {
				/* Why > vallen, not >=? bash:
				 * unset b; a=ab; : ${a:2:${b=3}}; echo $b  # "", b=3 (!!!)
				 * unset b; a=a; : ${a:2:${b=3}}; echo $b   # "", b not set
				 */
				പോകുക empty_result;
			}
			len = expand_and_evaluate_arith(exp_word, &errmsg);
			എങ്കിൽ (errmsg)
				പോകുക empty_result;
			debug_printf_varexp("len:'%s'=%lld\n", exp_word, (long long)len);
			debug_printf_varexp("from val:'%s'\n", val);
			എങ്കിൽ (len < 0) {
				/* in bash, len=-n means strlen()-n */
				len = (arith_t)vallen - beg + len;
				എങ്കിൽ (len < 0) /* bash compat */
					സന്ദേശം_and_die_if_script("%s: substring expression < 0", var);
			}
			എങ്കിൽ (len <= 0 || !val /*|| beg >= vallen*/) {
 empty_result:
				val = ശൂന്യ;
			} അല്ലെങ്കിൽ {
				/* Paranoia. What if user entered 9999999999999
				 * which fits in arith_t but not int? */
				എങ്കിൽ (len > INT_MAX)
					len = INT_MAX;
				val = to_be_freed = xstrndup(val + beg, len);
			}
			debug_printf_varexp("val:'%s'\n", val);
#അല്ലെങ്കിൽ /* not (HUSH_SUBSTR_EXPANSION && FEATURE_SH_MATH) */
			സന്ദേശം_and_die_if_script("malformed ${%s:...}", var);
			val = ശൂന്യ;
#endif
		} അല്ലെങ്കിൽ { /* one of "-=+?" */
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
			പൂർണ്ണസംഖ്യ use_word = (!val || ((exp_save == ':') && !val[0]));
			എങ്കിൽ (exp_op == '+')
				use_word = !use_word;
			debug_printf_expand("expand: op:%c (null:%s) test:%i\n", exp_op,
					(exp_save == ':') ? "true" : "false", use_word);
			എങ്കിൽ (use_word) {
				എങ്കിൽ (exp_op == '+' || exp_op == '-') {
					/* ${var+word} - use alternative value */
					/* ${var-word} - use default value */
					n = encode_then_append_var_plusminus(output, n, exp_word,
							/*dquoted:*/ (arg0 & 0x80)
					);
					val = ശൂന്യ;
				} അല്ലെങ്കിൽ {
					/* ${var?word} - indicate error if unset */
					/* ${var=word} - assign and use default value */
					to_be_freed = encode_then_expand_vararg(exp_word,
							/*handle_squotes:*/ !(arg0 & 0x80),
							/*unbackslash:*/ 0
					);
					എങ്കിൽ (to_be_freed)
						exp_word = to_be_freed;
					എങ്കിൽ (exp_op == '?') {
						/* mimic bash message */
						സന്ദേശം_and_die_if_script("%s: %s",
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
					} അല്ലെങ്കിൽ {
						val = exp_word;
					}
					എങ്കിൽ (exp_op == '=') {
						/* ${var=[word]} or ${var:=[word]} */
						എങ്കിൽ (isdigit(var[0]) || var[0] == '#') {
							/* mimic bash message */
							സന്ദേശം_and_die_if_script("$%s: cannot assign in this way", var);
							val = ശൂന്യ;
						} അല്ലെങ്കിൽ {
							അക്ഷരം *new_var = xasprintf("%s=%s", var, val);
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

	സ്വതന്ത്രം(to_be_freed);
	തിരികെ n;
}

/* Expand all variable references in given string, adding words to list[]
 * at n, n+1,... positions. Return updated n (so that list[n] is next one
 * to be filled). This routine is extremely tricky: has to deal with
 * variables/parameters with whitespace, $* and $@, and constructs like
 * 'echo -$*-'. If you play here, you must run testsuite afterwards! */
സ്ഥിതം NOINLINE പൂർണ്ണസംഖ്യ expand_vars_to_list(o_string *output, പൂർണ്ണസംഖ്യ n, അക്ഷരം *arg)
{
	/* output->o_expflags & EXP_FLAG_SINGLEWORD (0x80) if we are in
	 * expansion of right-hand side of assignment == 1-element expand.
	 */
	അക്ഷരം cant_be_null = 0; /* only bit 0x80 matters */
	അക്ഷരം *p;

	debug_printf_expand("expand_vars_to_list: arg:'%s' singleword:%x\n", arg,
			!!(output->o_expflags & EXP_FLAG_SINGLEWORD));
	debug_print_list("expand_vars_to_list[0]", output, n);

	ആകുമ്പോൾ ((p = strchr(arg, SPECIAL_VAR_ചിഹ്നം)) != ശൂന്യ) {
		അക്ഷരം first_ch;
#എങ്കിൽ ENABLE_FEATURE_SH_MATH
		അക്ഷരം arith_buf[വലുപ്പം(arith_t)*3 + 2];
#endif

		എങ്കിൽ (output->ended_in_ifs) {
			o_addchr(output, '\0');
			n = o_save_ptr(output, n);
			output->ended_in_ifs = 0;
		}

		o_addblock(output, arg, p - arg);
		debug_print_list("expand_vars_to_list[1]", output, n);
		arg = ++p;
		p = strchr(p, SPECIAL_VAR_ചിഹ്നം);

		/* Fetch special var name (if it is indeed one of them)
		 * and quote bit, force the bit on if singleword expansion -
		 * important for not getting v=$@ expand to many words. */
		first_ch = arg[0] | (output->o_expflags & EXP_FLAG_SINGLEWORD);

		/* Is this variable quoted and thus expansion can't be null?
		 * "$@" is special. Even if quoted, it can still
		 * expand to nothing (not even an empty string),
		 * thus it is excluded. */
		എങ്കിൽ ((first_ch & 0x7f) != '@')
			cant_be_null |= first_ch;

		തിരഞ്ഞെടുപ്പ് (first_ch & 0x7f) {
		/* Highest bit in first_ch indicates that var is double-quoted */
		അവസ്ഥ '*':
		അവസ്ഥ '@': {
			പൂർണ്ണസംഖ്യ i;
#ifndef __U_BOOT__
			എങ്കിൽ (!G.global_argv[1])
#അല്ലെങ്കിൽ /* __U_BOOT__ */
			എങ്കിൽ (!G.global_argv || !G.global_argv[1])
#endif /* __U_BOOT__ */
				നിർത്തുക;
			i = 1;
			cant_be_null |= first_ch; /* do it for "$@" _now_, when we know it's not empty */
			എങ്കിൽ (!(first_ch & 0x80)) { /* unquoted $* or $@ */
				ആകുമ്പോൾ (G.global_argv[i]) {
					n = expand_on_ifs(output, n, G.global_argv[i]);
					debug_printf_expand("expand_vars_to_list: argv %d (last %d)\n", i, G.global_argc - 1);
					എങ്കിൽ (G.global_argv[i++][0] && G.global_argv[i]) {
						/* this argv[] is not empty and not last:
						 * put terminating NUL, start new word */
						o_addchr(output, '\0');
						debug_print_list("expand_vars_to_list[2]", output, n);
						n = o_save_ptr(output, n);
						debug_print_list("expand_vars_to_list[3]", output, n);
					}
				}
			} അല്ലെങ്കിൽ
			/* If EXP_FLAG_SINGLEWORD, we handle assignment 'a=....$@.....'
			 * and in this case should treat it like '$*' - see 'else...' below */
			എങ്കിൽ (first_ch == (അക്ഷരം)('@'|0x80)  /* quoted $@ */
			 && !(output->o_expflags & EXP_FLAG_SINGLEWORD) /* not v="$@" case */
			) {
				ആകുമ്പോൾ (1) {
					o_addQstr(output, G.global_argv[i]);
					എങ്കിൽ (++i >= G.global_argc)
						നിർത്തുക;
					o_addchr(output, '\0');
					debug_print_list("expand_vars_to_list[4]", output, n);
					n = o_save_ptr(output, n);
				}
			} അല്ലെങ്കിൽ { /* quoted $* (or v="$@" case): add as one word */
				ആകുമ്പോൾ (1) {
					o_addQstr(output, G.global_argv[i]);
					എങ്കിൽ (!G.global_argv[++i])
						നിർത്തുക;
					എങ്കിൽ (G.ifs[0])
						o_addchr(output, G.ifs[0]);
				}
				output->has_quoted_part = 1;
			}
			നിർത്തുക;
		}
		അവസ്ഥ SPECIAL_VAR_ചിഹ്നം: {
			/* <SPECIAL_VAR_SYMBOL><SPECIAL_VAR_SYMBOL> */
			/* "Empty variable", used to make "" etc to not disappear */
			output->has_quoted_part = 1;
			cant_be_null = 0x80;
			arg++;
			നിർത്തുക;
		}
		അവസ്ഥ SPECIAL_VAR_QUOTED_SVS:
			/* <SPECIAL_VAR_SYMBOL><SPECIAL_VAR_QUOTED_SVS><SPECIAL_VAR_SYMBOL> */
			/* "^C variable", represents literal ^C char (possible in scripts) */
			o_addchr(output, SPECIAL_VAR_ചിഹ്നം);
			arg++;
			നിർത്തുക;
#എങ്കിൽ ENABLE_HUSH_TICK
		അവസ്ഥ '`': {
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
			നിർത്തുക;
		}
#endif
#എങ്കിൽ ENABLE_FEATURE_SH_MATH
		അവസ്ഥ '+': {
			/* <SPECIAL_VAR_SYMBOL>+arith<SPECIAL_VAR_SYMBOL> */
			arith_t res;

			arg++; /* skip '+' */
			*p = '\0'; /* replace trailing <SPECIAL_VAR_SYMBOL> */
			debug_printf_subst("ARITH '%s' first_ch %x\n", arg, first_ch);
			res = expand_and_evaluate_arith(arg, ശൂന്യ);
			debug_printf_subst("ARITH RES '"ARITH_FMT"'\n", res);
			sprintf(arith_buf, ARITH_FMT, res);
			എങ്കിൽ (res < 0
			 && first_ch == (അക്ഷരം)('+'|0x80)
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
			നിർത്തുക;
		}
#endif
		സ്ഥിരസ്ഥിതി:
			/* <SPECIAL_VAR_SYMBOL>varname[ops]<SPECIAL_VAR_SYMBOL> */
			n = expand_one_var(output, n, first_ch, arg, &p);
			നിർത്തുക;
		} /* switch (char after <SPECIAL_VAR_SYMBOL>) */

		/* Restore NULL'ed SPECIAL_VAR_SYMBOL.
		 * Do the check to avoid writing to a const string. */
		എങ്കിൽ (*p != SPECIAL_VAR_ചിഹ്നം)
			*p = SPECIAL_VAR_ചിഹ്നം;
		arg = ++p;
	} /* end of "while (SPECIAL_VAR_SYMBOL is found) ..." */

	എങ്കിൽ (*arg) {
		/* handle trailing string */
		എങ്കിൽ (output->ended_in_ifs) {
			o_addchr(output, '\0');
			n = o_save_ptr(output, n);
		}
		debug_print_list("expand_vars_to_list[a]", output, n);
		/* this part is literal, and it was already pre-quoted
		 * if needed (much earlier), do not use o_addQstr here!
		 */
		o_addstr(output, arg);
		debug_print_list("expand_vars_to_list[b]", output, n);
	} അല്ലെങ്കിൽ
	എങ്കിൽ (output->length == o_get_last_ptr(output, n) /* expansion is empty */
	 && !(cant_be_null & 0x80)   /* and all vars were not quoted */
	 && !output->has_quoted_part
	) {
		n--;
		/* allow to reuse list[n] later without re-growth */
		output->has_empty_slot = 1;
	}

	തിരികെ n;
}

സ്ഥിതം അക്ഷരം **expand_variables(അക്ഷരം **argv, unsigned expflags)
{
	പൂർണ്ണസംഖ്യ n;
	അക്ഷരം **list;
	o_string output = NULL_O_STRING;

	output.o_expflags = expflags;

	n = 0;
	വേണ്ടി (;;) {
		/* go to next list[n] */
		output.ended_in_ifs = 0;
		n = o_save_ptr(&output, n);

		എങ്കിൽ (!*argv)
			നിർത്തുക;

		/* expand argv[i] */
		n = expand_vars_to_list(&output, n, *argv++);
		/* if (!output->has_empty_slot) -- need this?? */
			o_addchr(&output, '\0');
	}
	debug_print_list("expand_variables", &output, n);

	/* output.data (malloced in one block) gets returned in "list" */
	list = o_finalize_list(&output, n);
	debug_print_strings("expand_variables[1]", list);
	തിരികെ list;
}

സ്ഥിതം അക്ഷരം **expand_strvec_to_strvec(അക്ഷരം **argv)
{
	തിരികെ expand_variables(argv, EXP_FLAG_GLOB | EXP_FLAG_ESC_GLOB_CHARS);
}

#എങ്കിൽ defined(CMD_SINGLEWORD_NOGLOB) || defined(CMD_TEST2_SINGLEWORD_NOGLOB)
സ്ഥിതം അക്ഷരം **expand_strvec_to_strvec_singleword_noglob(അക്ഷരം **argv)
{
	തിരികെ expand_variables(argv, EXP_FLAG_SINGLEWORD);
}
#endif

/* Used for expansion of right hand of assignments,
 * $((...)), heredocs, variable expansion parts.
 *
 * NB: should NOT do globbing!
 * "export v=/bin/c*; env | grep ^v=" outputs "v=/bin/c*"
 */
സ്ഥിതം അക്ഷരം *expand_string_to_string(സ്ഥിരം അക്ഷരം *str, പൂർണ്ണസംഖ്യ EXP_flags, പൂർണ്ണസംഖ്യ do_unbackslash)
{
#എങ്കിൽ !BASH_PATTERN_SUBST && !ENABLE_HUSH_CASE
	സ്ഥിരം പൂർണ്ണസംഖ്യ do_unbackslash = 1;
	സ്ഥിരം പൂർണ്ണസംഖ്യ EXP_flags = EXP_FLAG_ESC_GLOB_CHARS;
#endif
	അക്ഷരം *argv[2], **list;

	debug_printf_expand("string_to_string<='%s'\n", str);
	/* This is generally an optimization, but it also
	 * handles "", which otherwise trips over !list[0] check below.
	 * (is this ever happens that we actually get str="" here?)
	 */
	എങ്കിൽ (!strchr(str, SPECIAL_VAR_ചിഹ്നം) && !strchr(str, '\\')) {
		//TODO: Can use on strings with \ too, just unbackslash() them?
		debug_printf_expand("string_to_string(fast)=>'%s'\n", str);
		തിരികെ xstrdup(str);
	}

	argv[0] = (അക്ഷരം*)str;
	argv[1] = ശൂന്യ;
	list = expand_variables(argv, EXP_flags | EXP_FLAG_SINGLEWORD);
	എങ്കിൽ (!list[0]) {
		/* Example where it happens:
		 * x=; echo ${x:-"$@"}
		 */
		((അക്ഷരം*)list)[0] = '\0';
	} അല്ലെങ്കിൽ {
		എങ്കിൽ (HUSH_DEBUG)
			എങ്കിൽ (list[1])
				bb_simple_error_സന്ദേശം_and_die("BUG in varexp2");
		/* actually, just move string 2*sizeof(char*) bytes back */
		overlapping_strcpy((അക്ഷരം*)list, list[0]);
		എങ്കിൽ (do_unbackslash)
			unbackslash((അക്ഷരം*)list);
	}
	debug_printf_expand("string_to_string=>'%s'\n", (അക്ഷരം*)list);
	തിരികെ (അക്ഷരം*)list;
}

#എങ്കിൽ 0
സ്ഥിതം അക്ഷരം* expand_strvec_to_string(അക്ഷരം **argv)
{
	അക്ഷരം **list;

	list = expand_variables(argv, EXP_FLAG_SINGLEWORD);
	/* Convert all NULs to spaces */
	എങ്കിൽ (list[0]) {
		പൂർണ്ണസംഖ്യ n = 1;
		ആകുമ്പോൾ (list[n]) {
			എങ്കിൽ (HUSH_DEBUG)
				എങ്കിൽ (list[n-1] + strlen(list[n-1]) + 1 != list[n])
					bb_error_സന്ദേശം_and_die("BUG in varexp3");
			/* bash uses ' ' regardless of $IFS contents */
			list[n][-1] = ' ';
			n++;
		}
	}
	overlapping_strcpy((അക്ഷരം*)list, list[0] ? list[0] : "");
	debug_printf_expand("strvec_to_string='%s'\n", (അക്ഷരം*)list);
	തിരികെ (അക്ഷരം*)list;
}
#endif

#ifndef __U_BOOT__
സ്ഥിതം അക്ഷരം **expand_assignments(അക്ഷരം **argv, പൂർണ്ണസംഖ്യ എണ്ണം)
{
	പൂർണ്ണസംഖ്യ i;
	അക്ഷരം **p;

	G.expanded_assignments = p = ശൂന്യ;
	/* Expand assignments into one string each */
	വേണ്ടി (i = 0; i < എണ്ണം; i++) {
		p = add_string_to_strings(p,
			expand_string_to_string(argv[i],
				EXP_FLAG_ESC_GLOB_CHARS,
				/*unbackslash:*/ 1
			)
		);
		G.expanded_assignments = p;
	}
	G.expanded_assignments = ശൂന്യ;
	തിരികെ p;
}

സ്ഥിതം ശൂന്യം switch_off_special_sigs(unsigned mask)
{
	unsigned sig = 0;
	ആകുമ്പോൾ ((mask >>= 1) != 0) {
		sig++;
		എങ്കിൽ (!(mask & 1))
			തുടരുക;
#എങ്കിൽ ENABLE_HUSH_TRAP
		എങ്കിൽ (G_traps) {
			എങ്കിൽ (G_traps[sig] && !G_traps[sig][0])
				/* trap is '', has to remain SIG_IGN */
				തുടരുക;
			സ്വതന്ത്രം(G_traps[sig]);
			G_traps[sig] = ശൂന്യ;
		}
#endif
		/* We are here only if no trap or trap was not '' */
		install_sighandler(sig, SIG_DFL);
	}
}
#endif /* !__U_BOOT__ */

#ifndef __U_BOOT__
#എങ്കിൽ BB_MMU
/* never called */
ശൂന്യം re_execute_shell(അക്ഷരം ***to_free, സ്ഥിരം അക്ഷരം *s,
		അക്ഷരം *g_argv0, അക്ഷരം **g_argv,
		അക്ഷരം **builtin_argv) NORETURN;

സ്ഥിതം ശൂന്യം reset_traps_to_defaults(ശൂന്യം)
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
	എങ്കിൽ (!G_traps && !mask)
		തിരികെ; /* already no traps and no special sigs */

	/* Switch off special sigs */
	switch_off_special_sigs(mask);
# എങ്കിൽ ENABLE_HUSH_JOB
	G_fatal_sig_mask = 0;
# endif
	G.special_sig_mask &= ~SPECIAL_INTERACTIVE_SIGS;
	/* SIGQUIT,SIGCHLD and maybe SPECIAL_JOBSTOP_SIGS
	 * remain set in G.special_sig_mask */

# എങ്കിൽ ENABLE_HUSH_TRAP
	എങ്കിൽ (!G_traps)
		തിരികെ;

	/* Reset all sigs to default except ones with empty traps */
	വേണ്ടി (sig = 0; sig < NSIG; sig++) {
		എങ്കിൽ (!G_traps[sig])
			തുടരുക; /* no trap: nothing to do */
		എങ്കിൽ (!G_traps[sig][0])
			തുടരുക; /* empty trap: has to remain SIG_IGN */
		/* sig has non-empty trap, reset it: */
		സ്വതന്ത്രം(G_traps[sig]);
		G_traps[sig] = ശൂന്യ;
		/* There is no signal for trap 0 (EXIT) */
		എങ്കിൽ (sig == 0)
			തുടരുക;
		install_sighandler(sig, pick_sighandler(sig));
	}
# endif
}

#അല്ലെങ്കിൽ /* !BB_MMU */

സ്ഥിതം ശൂന്യം re_execute_shell(അക്ഷരം ***to_free, സ്ഥിരം അക്ഷരം *s,
		അക്ഷരം *g_argv0, അക്ഷരം **g_argv,
		അക്ഷരം **builtin_argv) NORETURN;
സ്ഥിതം ശൂന്യം re_execute_shell(അക്ഷരം ***to_free, സ്ഥിരം അക്ഷരം *s,
		അക്ഷരം *g_argv0, അക്ഷരം **g_argv,
		അക്ഷരം **builtin_argv)
{
# define NOMMU_HACK_FMT ("-$%x:%x:%x:%x:%x:%llx" IF_HUSH_LOOPS(":%x"))
	/* delims + 2 * (number of bytes in printed hex numbers) */
	അക്ഷരം param_buf[വലുപ്പം(NOMMU_HACK_FMT) + 2 * (വലുപ്പം(പൂർണ്ണസംഖ്യ)*6 + വലുപ്പം(long long)*1)];
	അക്ഷരം *heredoc_argv[4];
	ഘടന variable *cur;
# എങ്കിൽ ENABLE_HUSH_FUNCTIONS
	ഘടന function *funcp;
# endif
	അക്ഷരം **argv, **pp;
	unsigned cnt;
	unsigned long long empty_trap_mask;

	എങ്കിൽ (!g_argv0) { /* heredoc */
		argv = heredoc_argv;
		argv[0] = (അക്ഷരം *) G.argv0_for_re_execing;
		argv[1] = (അക്ഷരം *) "-<";
		argv[2] = (അക്ഷരം *) s;
		argv[3] = ശൂന്യ;
		pp = &argv[3]; /* used as pointer to empty environment */
		പോകുക do_exec;
	}

	cnt = 0;
	pp = builtin_argv;
	എങ്കിൽ (pp) ആകുമ്പോൾ (*pp++)
		cnt++;

	empty_trap_mask = 0;
	എങ്കിൽ (G_traps) {
		പൂർണ്ണസംഖ്യ sig;
		വേണ്ടി (sig = 1; sig < NSIG; sig++) {
			എങ്കിൽ (G_traps[sig] && !G_traps[sig][0])
				empty_trap_mask |= 1LL << sig;
		}
	}

	sprintf(param_buf, NOMMU_HACK_FMT
			, (unsigned) G.വേര്_pid
			, (unsigned) G.വേര്_ppid
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
	വേണ്ടി (cur = G.മുകൾ_var; cur; cur = cur->next) {
		എങ്കിൽ (!cur->flg_export || cur->flg_read_only)
			cnt += 2;
	}
# എങ്കിൽ ENABLE_HUSH_LINENO_VAR
	cnt += 2;
# endif
# എങ്കിൽ ENABLE_HUSH_FUNCTIONS
	വേണ്ടി (funcp = G.മുകൾ_func; funcp; funcp = funcp->next)
		cnt += 3;
# endif
	pp = g_argv;
	ആകുമ്പോൾ (*pp++)
		cnt++;
	*to_free = argv = pp = xzalloc(വലുപ്പം(argv[0]) * cnt);
	*pp++ = (അക്ഷരം *) G.argv0_for_re_execing;
	*pp++ = param_buf;
	വേണ്ടി (cur = G.മുകൾ_var; cur; cur = cur->next) {
		എങ്കിൽ (strcmp(cur->varstr, hush_version_str) == 0)
			തുടരുക;
		എങ്കിൽ (cur->flg_read_only) {
			*pp++ = (അക്ഷരം *) "-R";
			*pp++ = cur->varstr;
		} അല്ലെങ്കിൽ എങ്കിൽ (!cur->flg_export) {
			*pp++ = (അക്ഷരം *) "-V";
			*pp++ = cur->varstr;
		}
	}
# എങ്കിൽ ENABLE_HUSH_LINENO_VAR
	*pp++ = (അക്ഷരം *) "-L";
	*pp++ = utoa(G.execute_lineno);
# endif
# എങ്കിൽ ENABLE_HUSH_FUNCTIONS
	വേണ്ടി (funcp = G.മുകൾ_func; funcp; funcp = funcp->next) {
		*pp++ = (അക്ഷരം *) "-F";
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
	*pp++ = (അക്ഷരം *) "-c";
	*pp++ = (അക്ഷരം *) s;
	എങ്കിൽ (builtin_argv) {
		ആകുമ്പോൾ (*++builtin_argv)
			*pp++ = *builtin_argv;
		*pp++ = (അക്ഷരം *) "";
	}
	*pp++ = g_argv0;
	ആകുമ്പോൾ (*g_argv)
		*pp++ = *g_argv++;
	/* *pp = NULL; - is already there */
	pp = environ;

 do_exec:
	debug_printf_exec("re_execute_shell pid:%d cmd:'%s'\n", getpid(), s);
	/* Don't propagate SIG_IGN to the child */
	എങ്കിൽ (SPECIAL_JOBSTOP_SIGS != 0)
		switch_off_special_sigs(G.special_sig_mask & SPECIAL_JOBSTOP_SIGS);
	execve(bb_busybox_exec_path, argv, pp);
	/* Fallback. Useful for init=/bin/hush usage etc */
	എങ്കിൽ (argv[0][0] == '/')
		execve(argv[0], argv, pp);
	xfunc_error_retval = 127;
	bb_simple_error_സന്ദേശം_and_die("can't re-execute the shell");
}
#endif  /* !BB_MMU */

#endif /* !__U_BOOT__ */

സ്ഥിതം പൂർണ്ണസംഖ്യ run_and_free_list(ഘടന pipe *pi);

/* Executing from string: eval, sh -c '...'
 *          or from file: /etc/profile, . file, sh <script>, sh (intereactive)
 * end_trigger controls how often we stop parsing
 * NUL: parse all, execute, return
 * ';': parse till ';' or newline, execute, repeat till EOF
 */
#ifndef __U_BOOT__
സ്ഥിതം ശൂന്യം parse_and_run_stream(ഘടന in_str *inp, പൂർണ്ണസംഖ്യ end_trigger)
#അല്ലെങ്കിൽ /* __U_BOOT__ */
സ്ഥിതം പൂർണ്ണസംഖ്യ parse_and_run_stream(ഘടന in_str *inp, പൂർണ്ണസംഖ്യ end_trigger)
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
	ആകുമ്പോൾ (1) {
#അല്ലെങ്കിൽ /* __U_BOOT__ */
	ചെയ്യുക {
#endif /* __U_BOOT__ */
		ഘടന pipe *pipe_list;

#എങ്കിൽ ENABLE_HUSH_INTERACTIVE
		എങ്കിൽ (end_trigger == ';') {
			G.promptmode = 0; /* PS1 */
			debug_printf_prompt("%s promptmode=%d\n", __func__, G.promptmode);
		}
#endif
		pipe_list = parse_stream(ശൂന്യ, ശൂന്യ, inp, end_trigger);
		എങ്കിൽ (!pipe_list || pipe_list == ERR_PTR) { /* EOF/error */
			/* If we are in "big" script
			 * (not in `cmd` or something similar)...
			 */
			എങ്കിൽ (pipe_list == ERR_PTR && end_trigger == ';') {
				/* Discard cached input (rest of line) */
				പൂർണ്ണസംഖ്യ ch = inp->last_char;
				ആകുമ്പോൾ (ch != EOF && ch != '\n') {
					//bb_error_msg("Discarded:'%c'", ch);
					ch = i_getch(inp);
				}
				/* Force prompt */
				inp->p = ശൂന്യ;
				/* This stream isn't empty */
				empty = 0;
				തുടരുക;
			}
			എങ്കിൽ (!pipe_list && empty)
				G.last_exitcode = 0;
			നിർത്തുക;
		}
		debug_print_വൃക്ഷം(pipe_list, 0);
		debug_printf_exec("parse_and_run_stream: run_and_free_list\n");
#ifndef __U_BOOT__
		run_and_free_list(pipe_list);
#അല്ലെങ്കിൽ /* __U_BOOT__ */
		പൂർണ്ണസംഖ്യ rcode = run_and_free_list(pipe_list);
		/*
		 * We reset input string to not run the following command, so running
		 * 'exit; echo foo' does not print foo.
		 */
		എങ്കിൽ (rcode <= EXIT_RET_CODE)
			setup_file_in_str(inp);
#endif /* __U_BOOT__ */
		empty = 0;
		എങ്കിൽ (G_flag_return_in_progress == 1)
			നിർത്തുക;
#ifndef __U_BOOT__
	}
#അല്ലെങ്കിൽ /* __U_BOOT__ */
	/*
	 * This do/while is needed by run_command to avoid looping on a command
	 * with syntax error.
	 */
	} ആകുമ്പോൾ (!(G.run_command_flags & FLAG_EXIT_FROM_LOOP));

	തിരികെ G.last_exitcode;
#endif /* __U_BOOT__ */
}

#ifndef __U_BOOT__
സ്ഥിതം ശൂന്യം parse_and_run_string(സ്ഥിരം അക്ഷരം *s)
#അല്ലെങ്കിൽ /* __U_BOOT__ */
സ്ഥിതം പൂർണ്ണസംഖ്യ parse_and_run_string(സ്ഥിരം അക്ഷരം *s)
#endif /* __U_BOOT__ */
{
	ഘടന in_str നിവേശം;
#ifndef __U_BOOT__
	IF_HUSH_LINENO_VAR(unsigned sv = G.parse_lineno;)
#അല്ലെങ്കിൽ /* __U_BOOT__ */
	//IF_HUSH_LINENO_VAR(unsigned sv = G.parse_lineno;)
#endif /* __U_BOOT__ */

	setup_string_in_str(&നിവേശം, s);
#ifndef __U_BOOT__
	parse_and_run_stream(&നിവേശം, '\0');
#അല്ലെങ്കിൽ /* __U_BOOT__ */
	തിരികെ parse_and_run_stream(&നിവേശം, '\0');
#endif /* __U_BOOT__ */
#ifndef __U_BOOT__
	IF_HUSH_LINENO_VAR(unsigned sv = G.parse_lineno;)
#അല്ലെങ്കിൽ /* __U_BOOT__ */
	//IF_HUSH_LINENO_VAR(unsigned sv = G.parse_lineno;)
#endif /* __U_BOOT__ */
}

#ifdef __U_BOOT__
പൂർണ്ണസംഖ്യ parse_string_outer_modern(സ്ഥിരം അക്ഷരം *cmd, പൂർണ്ണസംഖ്യ flags)
{
	പൂർണ്ണസംഖ്യ ret;
	പൂർണ്ണസംഖ്യ old_flags;

	/*
	 * Keep old values of run_command to be able to restore them once
	 * command was executed.
	 */
	old_flags = G.run_command_flags;
	G.run_command_flags = flags;

	ret = parse_and_run_string(cmd);

	G.run_command_flags = old_flags;

	തിരികെ ret;
}
#endif /* __U_BOOT__ */
#ifndef __U_BOOT__
സ്ഥിതം ശൂന്യം parse_and_run_file(HFILE *fp)
#അല്ലെങ്കിൽ /* __U_BOOT__ */
ശൂന്യം parse_and_run_file(ശൂന്യം)
#endif /* __U_BOOT__ */
{
	ഘടന in_str നിവേശം;
#ifndef __U_BOOT__
	IF_HUSH_LINENO_VAR(unsigned sv = G.parse_lineno;)

	IF_HUSH_LINENO_VAR(G.parse_lineno = 1;)
	setup_file_in_str(&നിവേശം, fp);
#അല്ലെങ്കിൽ /* __U_BOOT__ */
	setup_file_in_str(&നിവേശം);
#endif /* __U_BOOT__ */
	parse_and_run_stream(&നിവേശം, ';');
#ifndef __U_BOOT__
	IF_HUSH_LINENO_VAR(G.parse_lineno = sv;)
#endif /* !__U_BOOT__ */
}

#ifndef __U_BOOT__
#എങ്കിൽ ENABLE_HUSH_TICK
സ്ഥിതം പൂർണ്ണസംഖ്യ generate_stream_from_string(സ്ഥിരം അക്ഷരം *s, pid_t *pid_p)
{
	pid_t pid;
	പൂർണ്ണസംഖ്യ channel[2];
# എങ്കിൽ !BB_MMU
	അക്ഷരം **to_free = ശൂന്യ;
# endif

	xpipe(channel);
	pid = BB_MMU ? xfork() : xvfork();
	എങ്കിൽ (pid == 0) { /* child */
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
# എങ്കിൽ ENABLE_HUSH_TRAP
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
		എങ്കിൽ (is_prefixed_with(s, "trap")
		 && skip_whitespace(s + 4)[0] == '\0'
		) {
			സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം argv[] ALIGN_PTR = { ശൂന്യ, ശൂന്യ };
			builtin_trap((അക്ഷരം**)argv);
			fflush_all(); /* important */
			_exit(0);
		}
# endif
# എങ്കിൽ BB_MMU
		/* Prevent it from trying to handle ctrl-z etc */
		IF_HUSH_JOB(G.run_list_level = 1;)
		CLEAR_RANDOM_T(&G.random_gen); /* or else $RANDOM repeats in child */
		reset_traps_to_defaults();
		IF_HUSH_MODE_X(G.x_mode_depth++;)
		//bb_error_msg("%s: ++x_mode_depth=%d", __func__, G.x_mode_depth);
		parse_and_run_string(s);
		_exit(G.last_exitcode);
# അല്ലെങ്കിൽ
	/* We re-execute after vfork on NOMMU. This makes this script safe:
	 * yes "0123456789012345678901234567890" | dd bs=32 count=64k >BIG
	 * huge=`cat BIG` # was blocking here forever
	 * echo OK
	 */
		re_execute_shell(&to_free,
				s,
				G.global_argv[0],
				G.global_argv + 1,
				ശൂന്യ);
# endif
	}

	/* parent */
	*pid_p = pid;
# എങ്കിൽ ENABLE_HUSH_FAST
	G.എണ്ണം_SIGCHLD++;
//bb_error_msg("[%d] fork in generate_stream_from_string:"
//		" G.count_SIGCHLD:%d G.handled_SIGCHLD:%d",
//		getpid(), G.count_SIGCHLD, G.handled_SIGCHLD);
# endif
	enable_restore_tty_pgrp_on_exit();
# എങ്കിൽ !BB_MMU
	സ്വതന്ത്രം(to_free);
# endif
	close(channel[1]);
	തിരികെ channel[0];
}

/* Return code is exit status of the process that is run. */
സ്ഥിതം പൂർണ്ണസംഖ്യ process_command_subs(o_string *dest, സ്ഥിരം അക്ഷരം *s)
{
	FILE *fp;
	pid_t pid;
	പൂർണ്ണസംഖ്യ status, ch, eol_cnt;

	fp = xfdopen_for_read(generate_stream_from_string(s, &pid));

	/* Now send results of command back into original context */
	eol_cnt = 0;
	ആകുമ്പോൾ ((ch = getc(fp)) != EOF) {
		എങ്കിൽ (ch == '\0')
			തുടരുക;
		എങ്കിൽ (ch == '\n') {
			eol_cnt++;
			തുടരുക;
		}
		ആകുമ്പോൾ (eol_cnt) {
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
	തിരികെ WEXITSTATUS(status);
}
#endif /* ENABLE_HUSH_TICK */

സ്ഥിതം ശൂന്യം setup_heredoc(ഘടന redir_struct *redir)
{
	ഘടന fd_pair pair;
	pid_t pid;
	പൂർണ്ണസംഖ്യ len, written;
	/* the _body_ of heredoc (misleading field name) */
	സ്ഥിരം അക്ഷരം *heredoc = redir->rd_filename;
	അക്ഷരം *expanded;
#എങ്കിൽ !BB_MMU
	അക്ഷരം **to_free;
#endif

	expanded = ശൂന്യ;
	എങ്കിൽ (!(redir->rd_dup & HEREDOC_QUOTED)) {
		expanded = encode_then_expand_string(heredoc);
		എങ്കിൽ (expanded)
			heredoc = expanded;
	}
	len = strlen(heredoc);

	close(redir->rd_fd); /* often saves dup2+close in xmove_fd */
	xpiped_pair(pair);
	xmove_fd(pair.rd, redir->rd_fd);

	/* Try writing without forking. Newer kernels have
	 * dynamically growing pipes. Must use non-blocking write! */
	ndelay_on(pair.wr);
	ആകുമ്പോൾ (1) {
		written = ലിഖ(pair.wr, heredoc, len);
		എങ്കിൽ (written <= 0)
			നിർത്തുക;
		len -= written;
		എങ്കിൽ (len == 0) {
			close(pair.wr);
			സ്വതന്ത്രം(expanded);
			തിരികെ;
		}
		heredoc += written;
	}
	ndelay_off(pair.wr);

	/* Okay, pipe buffer was not big enough */
	/* Note: we must not create a stray child (bastard? :)
	 * for the unsuspecting parent process. Child creates a grandchild
	 * and exits before parent execs the process which consumes heredoc
	 * (that exec happens after we return from this function) */
#എങ്കിൽ !BB_MMU
	to_free = ശൂന്യ;
#endif
	pid = xvfork();
	എങ്കിൽ (pid == 0) {
		/* child */
		disable_restore_tty_pgrp_on_exit();
		pid = BB_MMU ? xfork() : xvfork();
		എങ്കിൽ (pid != 0)
			_exit(0);
		/* grandchild */
		close(redir->rd_fd); /* read side of the pipe */
#എങ്കിൽ BB_MMU
		full_write(pair.wr, heredoc, len); /* may loop or block */
		_exit(0);
#അല്ലെങ്കിൽ
		/* Delegate blocking writes to another process */
		xmove_fd(pair.wr, STDOUT_FILENO);
		re_execute_shell(&to_free, heredoc, ശൂന്യ, ശൂന്യ, ശൂന്യ);
#endif
	}
	/* parent */
#എങ്കിൽ ENABLE_HUSH_FAST
	G.എണ്ണം_SIGCHLD++;
//bb_error_msg("[%d] fork in setup_heredoc: G.count_SIGCHLD:%d G.handled_SIGCHLD:%d", getpid(), G.count_SIGCHLD, G.handled_SIGCHLD);
#endif
	enable_restore_tty_pgrp_on_exit();
#എങ്കിൽ !BB_MMU
	സ്വതന്ത്രം(to_free);
#endif
	close(pair.wr);
	സ്വതന്ത്രം(expanded);
	wait(ശൂന്യ); /* wait till child has died */
}

ഘടന squirrel {
	പൂർണ്ണസംഖ്യ orig_fd;
	പൂർണ്ണസംഖ്യ moved_to;
	/* moved_to = n: fd was moved to n; restore back to orig_fd after redir */
	/* moved_to = -1: fd was opened by redirect; close orig_fd after redir */
};

സ്ഥിതം ഘടന squirrel *append_squirrel(ഘടന squirrel *sq, പൂർണ്ണസംഖ്യ i, പൂർണ്ണസംഖ്യ orig, പൂർണ്ണസംഖ്യ moved)
{
	sq = xrealloc(sq, (i + 2) * വലുപ്പം(sq[0]));
	sq[i].orig_fd = orig;
	sq[i].moved_to = moved;
	sq[i+1].orig_fd = -1; /* end marker */
	തിരികെ sq;
}

സ്ഥിതം ഘടന squirrel *add_squirrel(ഘടന squirrel *sq, പൂർണ്ണസംഖ്യ fd, പൂർണ്ണസംഖ്യ avoid_fd)
{
	പൂർണ്ണസംഖ്യ moved_to;
	പൂർണ്ണസംഖ്യ i;

	i = 0;
	എങ്കിൽ (sq) വേണ്ടി (; sq[i].orig_fd >= 0; i++) {
		/* If we collide with an already moved fd... */
		എങ്കിൽ (fd == sq[i].moved_to) {
			moved_to = dup_CLOEXEC(sq[i].moved_to, avoid_fd);
			debug_printf_redir("redirect_fd %d: already busy, moving to %d\n", fd, moved_to);
			എങ്കിൽ (moved_to < 0) {
				/* "echo 2>/dev/tty 10>&9999" testcase:
				 * We move fd 2 to 10, then discover we need to move fd 10
				 * (and not hit 9999) and the latter fails.
				 */
				തിരികെ ശൂന്യ; /* fcntl failed */
			}
			sq[i].moved_to = moved_to;
			തിരികെ sq;
		}
		എങ്കിൽ (fd == sq[i].orig_fd) {
			/* Example: echo Hello >/dev/null 1>&2 */
			debug_printf_redir("redirect_fd %d: already moved\n", fd);
			തിരികെ sq;
		}
	}

	/* If this fd is open, we move and remember it; if it's closed, moved_to = -1 */
	moved_to = dup_CLOEXEC(fd, avoid_fd);
	debug_printf_redir("redirect_fd %d: previous fd is moved to %d (-1 if it was closed)\n", fd, moved_to);
	എങ്കിൽ (moved_to < 0 && errno != EBADF)
		തിരികെ ശൂന്യ; /* fcntl failed (not because fd is closed) */
	തിരികെ append_squirrel(sq, i, fd, moved_to);
}

സ്ഥിതം ഘടന squirrel *add_squirrel_closed(ഘടന squirrel *sq, പൂർണ്ണസംഖ്യ fd)
{
	പൂർണ്ണസംഖ്യ i;

	i = 0;
	എങ്കിൽ (sq) വേണ്ടി (; sq[i].orig_fd >= 0; i++) {
		/* If we collide with an already moved fd... */
		എങ്കിൽ (fd == sq[i].orig_fd) {
			/* Examples:
			 * "echo 3>FILE 3>&- 3>FILE"
			 * "echo 3>&- 3>FILE"
			 * No need for last redirect to insert
			 * another "need to close 3" indicator.
			 */
			debug_printf_redir("redirect_fd %d: already moved or closed\n", fd);
			തിരികെ sq;
		}
	}

	debug_printf_redir("redirect_fd %d: previous fd was closed\n", fd);
	തിരികെ append_squirrel(sq, i, fd, -1);
}

/* fd: redirect wants this fd to be used (e.g. 3>file).
 * Move all conflicting internally used fds,
 * and remember them so that we can restore them later.
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ save_fd_on_redirect(പൂർണ്ണസംഖ്യ fd, പൂർണ്ണസംഖ്യ avoid_fd, ഘടന squirrel **sqp)
{
	ഘടന squirrel *new_squirrel;

	എങ്കിൽ (avoid_fd < 9) /* the important case here is that it can be -1 */
		avoid_fd = 9;

#എങ്കിൽ ENABLE_HUSH_INTERACTIVE
	എങ്കിൽ (fd != 0 /* don't trigger for G_interactive_fd == 0 (that's "not interactive" flag) */
	 && fd == G_interactive_fd
	) {
		/* Testcase: "ls -l /proc/$$/fd 255>&-" should work */
		G_interactive_fd = xdup_CLOEXEC_and_close(G_interactive_fd, avoid_fd);
		debug_printf_redir("redirect_fd %d: matches interactive_fd, moving it to %d\n", fd, G_interactive_fd);
		തിരികെ 1; /* "we closed fd" */
	}
#endif
	/* Are we called from setup_redirects(squirrel==NULL)
	 * in redirect in a [v]forked child?
	 */
	എങ്കിൽ (sqp == ശൂന്യ) {
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
		തിരികെ 0;
	}

	/* If this one of script's fds? */
	എങ്കിൽ (move_HFILEs_on_redirect(fd, avoid_fd))
		തിരികെ 1; /* yes. "we closed fd" (actually moved it) */

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
	എങ്കിൽ (sqp == ERR_PTR) {
		/* Don't preserve redirected fds: exec is _meant_ to change these */
		//bb_error_msg("sqp == ERR_PTR: exec >FILE");
		തിരികെ 0;
	}

	/* Check whether it collides with any open fds (e.g. stdio), save fds as needed */
	new_squirrel = add_squirrel(*sqp, fd, avoid_fd);
	എങ്കിൽ (!new_squirrel)
		തിരികെ -1; /* redirect error */
	*sqp = new_squirrel;
	തിരികെ 0; /* "we did not close fd" */
}

സ്ഥിതം ശൂന്യം restore_redirects(ഘടന squirrel *sq)
{
	എങ്കിൽ (sq) {
		പൂർണ്ണസംഖ്യ i;
		വേണ്ടി (i = 0; sq[i].orig_fd >= 0; i++) {
			എങ്കിൽ (sq[i].moved_to >= 0) {
				/* We simply die on error */
				debug_printf_redir("restoring redirected fd from %d to %d\n", sq[i].moved_to, sq[i].orig_fd);
				xmove_fd(sq[i].moved_to, sq[i].orig_fd);
			} അല്ലെങ്കിൽ {
				/* cmd1 9>FILE; cmd2_should_see_fd9_closed */
				debug_printf_redir("restoring redirected fd %d: closing it\n", sq[i].orig_fd);
				close(sq[i].orig_fd);
			}
		}
		സ്വതന്ത്രം(sq);
	}
	എങ്കിൽ (G.HFILE_stdin
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

#എങ്കിൽ ENABLE_FEATURE_SH_STANDALONE && BB_MMU
സ്ഥിതം ശൂന്യം close_saved_fds_and_FILE_fds(ശൂന്യം)
{
	എങ്കിൽ (G_interactive_fd)
		close(G_interactive_fd);
	close_all_HFILE_list();
}
#endif

സ്ഥിതം പൂർണ്ണസംഖ്യ internally_opened_fd(പൂർണ്ണസംഖ്യ fd, ഘടന squirrel *sq)
{
	പൂർണ്ണസംഖ്യ i;

#എങ്കിൽ ENABLE_HUSH_INTERACTIVE
	എങ്കിൽ (fd == G_interactive_fd)
		തിരികെ 1;
#endif
	/* If this one of script's fds? */
	എങ്കിൽ (fd_in_HFILEs(fd))
		തിരികെ 1;

	എങ്കിൽ (sq) വേണ്ടി (i = 0; sq[i].orig_fd >= 0; i++) {
		എങ്കിൽ (fd == sq[i].moved_to)
			തിരികെ 1;
	}
	തിരികെ 0;
}

/* sqp != NULL means we squirrel away copies of stdin, stdout,
 * and stderr if they are redirected.
 * If redirection fails, return 1. This will make caller
 * skip command execution and restore already created redirect fds.
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ setup_redirects(ഘടന command *prog, ഘടന squirrel **sqp)
{
	ഘടന redir_struct *redir;

	വേണ്ടി (redir = prog->redirects; redir; redir = redir->next) {
		പൂർണ്ണസംഖ്യ newfd;
		പൂർണ്ണസംഖ്യ closed;

		എങ്കിൽ (redir->rd_type == REDIRECT_HEREDOC2) {
			/* "rd_fd<<HERE" case */
			എങ്കിൽ (save_fd_on_redirect(redir->rd_fd, /*avoid:*/ 0, sqp) < 0)
				തിരികെ 1;
			/* for REDIRECT_HEREDOC2, rd_filename holds _contents_
			 * of the heredoc */
			debug_printf_redir("set heredoc '%s'\n",
					redir->rd_filename);
			setup_heredoc(redir);
			തുടരുക;
		}

		എങ്കിൽ (redir->rd_dup == REDIRFD_TO_FILE) {
			/* "rd_fd<*>file" case (<*> is <,>,>>,<>) */
			അക്ഷരം *p;
			പൂർണ്ണസംഖ്യ mode;

			എങ്കിൽ (redir->rd_filename == ശൂന്യ) {
				/* Examples:
				 * "cmd >" (no filename)
				 * "cmd > <file" (2nd redirect starts too early)
				 */
				syntax_error("invalid redirect");
				തിരികെ 1;
			}
			mode = redir_table[redir->rd_type].mode;
			p = expand_string_to_string(redir->rd_filename,
				EXP_FLAG_ESC_GLOB_CHARS, /*unbackslash:*/ 1);
			newfd = open_or_warn(p, mode);
			സ്വതന്ത്രം(p);
			എങ്കിൽ (newfd < 0) {
				/* Error message from open_or_warn can be lost
				 * if stderr has been redirected, but bash
				 * and ash both lose it as well
				 * (though zsh doesn't!)
				 */
				തിരികെ 1;
			}
			എങ്കിൽ (newfd == redir->rd_fd && sqp
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
		} അല്ലെങ്കിൽ {
			/* "rd_fd>&rd_dup" or "rd_fd>&-" case */
			newfd = redir->rd_dup;
		}

		എങ്കിൽ (newfd == redir->rd_fd)
			തുടരുക;

		/* if "N>FILE": move newfd to redir->rd_fd */
		/* if "N>&M": dup newfd to redir->rd_fd */
		/* if "N>&-": close redir->rd_fd (newfd is REDIRFD_CLOSE) */

		closed = save_fd_on_redirect(redir->rd_fd, /*avoid:*/ newfd, sqp);
		എങ്കിൽ (closed < 0)
			തിരികെ 1; /* error */
		എങ്കിൽ (newfd == REDIRFD_CLOSE) {
			/* "N>&-" means "close me" */
			എങ്കിൽ (!closed) {
				/* ^^^ optimization: saving may already
				 * have closed it. If not... */
				close(redir->rd_fd);
			}
			/* Sometimes we do another close on restore, getting EBADF.
			 * Consider "echo 3>FILE 3>&-"
			 * first redirect remembers "need to close 3",
			 * and second redirect closes 3! Restore code then closes 3 again.
			 */
		} അല്ലെങ്കിൽ {
			/* if newfd is a script fd or saved fd, do not allow to use it */
			എങ്കിൽ (internally_opened_fd(newfd, sqp && sqp != ERR_PTR ? *sqp : ശൂന്യ)) {
				bb_error_സന്ദേശം("fd#%d is not open", newfd);
				തിരികെ 1;
			}
			എങ്കിൽ (dup2(newfd, redir->rd_fd) < 0) {
				/* "echo >&99" testcase */
				bb_perror_സന്ദേശം("dup2(%d,%d)", newfd, redir->rd_fd);
				തിരികെ 1;
			}
			എങ്കിൽ (redir->rd_dup == REDIRFD_TO_FILE)
				/* "rd_fd > FILE" */
				close(newfd);
			/* else: "rd_fd > rd_dup" */
		}
	}
	തിരികെ 0;
}

സ്ഥിതം അക്ഷരം *find_in_path(സ്ഥിരം അക്ഷരം *arg)
{
	അക്ഷരം *ret = ശൂന്യ;
	സ്ഥിരം അക്ഷരം *PATH = get_local_var_മൂല്യം("PATH");

	എങ്കിൽ (!PATH)
		തിരികെ ശൂന്യ;

	ആകുമ്പോൾ (1) {
		സ്ഥിരം അക്ഷരം *end = strchrnul(PATH, ':');
		പൂർണ്ണസംഖ്യ sz = end - PATH; /* must be int! */

		സ്വതന്ത്രം(ret);
		എങ്കിൽ (sz != 0) {
			ret = xasprintf("%.*s/%s", sz, PATH, arg);
		} അല്ലെങ്കിൽ {
			/* We have xxx::yyyy in $PATH,
			 * it means "use current dir" */
			ret = xstrdup(arg);
		}
		എങ്കിൽ (access(ret, F_OK) == 0)
			നിർത്തുക;

		എങ്കിൽ (*end == '\0') {
			സ്വതന്ത്രം(ret);
			തിരികെ ശൂന്യ;
		}
		PATH = end + 1;
	}

	തിരികെ ret;
}

സ്ഥിതം സ്ഥിരം ഘടന built_in_command *find_builtin_helper(സ്ഥിരം അക്ഷരം *name,
		സ്ഥിരം ഘടന built_in_command *x,
		സ്ഥിരം ഘടന built_in_command *end)
{
	ആകുമ്പോൾ (x != end) {
		എങ്കിൽ (strcmp(name, x->b_cmd) != 0) {
			x++;
			തുടരുക;
		}
		debug_printf_exec("found builtin '%s'\n", name);
		തിരികെ x;
	}
	തിരികെ ശൂന്യ;
}
സ്ഥിതം സ്ഥിരം ഘടന built_in_command *find_builtin1(സ്ഥിരം അക്ഷരം *name)
{
	തിരികെ find_builtin_helper(name, bltins1, &bltins1[ARRAY_SIZE(bltins1)]);
}
സ്ഥിതം സ്ഥിരം ഘടന built_in_command *find_builtin(സ്ഥിരം അക്ഷരം *name)
{
	സ്ഥിരം ഘടന built_in_command *x = find_builtin1(name);
	എങ്കിൽ (x)
		തിരികെ x;
	തിരികെ find_builtin_helper(name, bltins2, &bltins2[ARRAY_SIZE(bltins2)]);
}

#എങ്കിൽ ENABLE_HUSH_JOB && ENABLE_FEATURE_TAB_COMPLETION
സ്ഥിതം സ്ഥിരം അക്ഷരം * FAST_FUNC hush_command_name(പൂർണ്ണസംഖ്യ i)
{
	എങ്കിൽ (/*i >= 0 && */ i < ARRAY_SIZE(bltins1)) {
		തിരികെ bltins1[i].b_cmd;
	}
	i -= ARRAY_SIZE(bltins1);
	എങ്കിൽ (i < ARRAY_SIZE(bltins2)) {
		തിരികെ bltins2[i].b_cmd;
	}
# എങ്കിൽ ENABLE_HUSH_FUNCTIONS
	{
		ഘടന function *funcp;
		i -= ARRAY_SIZE(bltins2);
		വേണ്ടി (funcp = G.മുകൾ_func; funcp; funcp = funcp->next) {
			എങ്കിൽ (--i < 0)
				തിരികെ funcp->name;
		}
	}
# endif
	തിരികെ ശൂന്യ;
}
#endif
#endif /* !__U_BOOT__ */

#ifndef __U_BOOT__
സ്ഥിതം ശൂന്യം remove_nested_vars(ശൂന്യം)
{
	ഘടന variable *cur;
	ഘടന variable **cur_pp;

	cur_pp = &G.മുകൾ_var;
	ആകുമ്പോൾ ((cur = *cur_pp) != ശൂന്യ) {
		എങ്കിൽ (cur->var_nest_level <= G.var_nest_level) {
			cur_pp = &cur->next;
			തുടരുക;
		}
		/* Unexport */
		എങ്കിൽ (cur->flg_export) {
			debug_printf_env("unexporting nested '%s'/%u\n", cur->varstr, cur->var_nest_level);
			bb_unsetenv(cur->varstr);
		}
		/* Remove from global list */
		*cur_pp = cur->next;
		/* Free */
		എങ്കിൽ (!cur->max_len) {
			debug_printf_env("freeing nested '%s'/%u\n", cur->varstr, cur->var_nest_level);
			സ്വതന്ത്രം(cur->varstr);
		}
		സ്വതന്ത്രം(cur);
	}
}

സ്ഥിതം ശൂന്യം enter_var_nest_level(ശൂന്യം)
{
	G.var_nest_level++;
	debug_printf_env("var_nest_level++ %u\n", G.var_nest_level);

	/* Try:	f() { echo -n .; f; }; f
	 * struct variable::var_nest_level is uint16_t,
	 * thus limiting recursion to < 2^16.
	 * In any case, with 8 Mbyte stack SEGV happens
	 * not too long after 2^16 recursions anyway.
	 */
	എങ്കിൽ (G.var_nest_level > 0xff00)
		bb_error_സന്ദേശം_and_die("fatal recursion (depth %u)", G.var_nest_level);
}

സ്ഥിതം ശൂന്യം leave_var_nest_level(ശൂന്യം)
{
	G.var_nest_level--;
	debug_printf_env("var_nest_level-- %u\n", G.var_nest_level);
	എങ്കിൽ (HUSH_DEBUG && (പൂർണ്ണസംഖ്യ)G.var_nest_level < 0)
		bb_simple_error_സന്ദേശം_and_die("BUG: nesting underflow");

	remove_nested_vars();
}
#endif /* __U_BOOT__ */

#എങ്കിൽ ENABLE_HUSH_FUNCTIONS
സ്ഥിതം ഘടന function **find_function_slot(സ്ഥിരം അക്ഷരം *name)
{
	ഘടന function *funcp;
	ഘടന function **funcpp = &G.മുകൾ_func;

	ആകുമ്പോൾ ((funcp = *funcpp) != ശൂന്യ) {
		എങ്കിൽ (strcmp(name, funcp->name) == 0) {
			debug_printf_exec("found function '%s'\n", name);
			നിർത്തുക;
		}
		funcpp = &funcp->next;
	}
	തിരികെ funcpp;
}

സ്ഥിതം ALWAYS_INLINE സ്ഥിരം ഘടന function *find_function(സ്ഥിരം അക്ഷരം *name)
{
	സ്ഥിരം ഘടന function *funcp = *find_function_slot(name);
	തിരികെ funcp;
}

/* Note: takes ownership on name ptr */
സ്ഥിതം ഘടന function *new_function(അക്ഷരം *name)
{
	ഘടന function **funcpp = find_function_slot(name);
	ഘടന function *funcp = *funcpp;

	എങ്കിൽ (funcp != ശൂന്യ) {
		ഘടന command *cmd = funcp->parent_cmd;
		debug_printf_exec("func %p parent_cmd %p\n", funcp, cmd);
		എങ്കിൽ (!cmd) {
			debug_printf_exec("freeing & replacing function '%s'\n", funcp->name);
			സ്വതന്ത്രം(funcp->name);
			/* Note: if !funcp->body, do not free body_as_string!
			 * This is a special case of "-F name body" function:
			 * body_as_string was not malloced! */
			എങ്കിൽ (funcp->body) {
				free_pipe_list(funcp->body);
# എങ്കിൽ !BB_MMU
				സ്വതന്ത്രം(funcp->body_as_string);
# endif
			}
		} അല്ലെങ്കിൽ {
			debug_printf_exec("reinserting in tree & replacing function '%s'\n", funcp->name);
			cmd->argv[0] = funcp->name;
			cmd->group = funcp->body;
# എങ്കിൽ !BB_MMU
			cmd->group_as_string = funcp->body_as_string;
# endif
		}
	} അല്ലെങ്കിൽ {
		debug_printf_exec("remembering new function '%s'\n", name);
		funcp = *funcpp = xzalloc(വലുപ്പം(*funcp));
		/*funcp->next = NULL;*/
	}

	funcp->name = name;
	തിരികെ funcp;
}

# എങ്കിൽ ENABLE_HUSH_UNSET
സ്ഥിതം ശൂന്യം unset_func(സ്ഥിരം അക്ഷരം *name)
{
	ഘടന function **funcpp = find_function_slot(name);
	ഘടന function *funcp = *funcpp;

	എങ്കിൽ (funcp != ശൂന്യ) {
		debug_printf_exec("freeing function '%s'\n", funcp->name);
		*funcpp = funcp->next;
		/* funcp is unlinked now, deleting it.
		 * Note: if !funcp->body, the function was created by
		 * "-F name body", do not free ->body_as_string
		 * and ->name as they were not malloced. */
		എങ്കിൽ (funcp->body) {
			free_pipe_list(funcp->body);
			സ്വതന്ത്രം(funcp->name);
#  എങ്കിൽ !BB_MMU
			സ്വതന്ത്രം(funcp->body_as_string);
#  endif
		}
		സ്വതന്ത്രം(funcp);
	}
}
# endif

# എങ്കിൽ BB_MMU
#define exec_function(to_free, funcp, argv) \
	exec_function(funcp, argv)
# endif
സ്ഥിതം ശൂന്യം exec_function(അക്ഷരം ***to_free,
		സ്ഥിരം ഘടന function *funcp,
		അക്ഷരം **argv) NORETURN;
സ്ഥിതം ശൂന്യം exec_function(അക്ഷരം ***to_free,
		സ്ഥിരം ഘടന function *funcp,
		അക്ഷരം **argv)
{
# എങ്കിൽ BB_MMU
	പൂർണ്ണസംഖ്യ n;

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
# അല്ലെങ്കിൽ
//?	close_saved_fds_and_FILE_fds();

//TODO: check whether "true | func_with_return" works

	re_execute_shell(to_free,
			funcp->body_as_string,
			G.global_argv[0],
			argv + 1,
			ശൂന്യ);
# endif
}

സ്ഥിതം പൂർണ്ണസംഖ്യ run_function(സ്ഥിരം ഘടന function *funcp, അക്ഷരം **argv)
{
	പൂർണ്ണസംഖ്യ rc;
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
# എങ്കിൽ !BB_MMU
	എങ്കിൽ (!funcp->body) {
		/* Function defined by -F */
		parse_and_run_string(funcp->body_as_string);
		rc = G.last_exitcode;
	} അല്ലെങ്കിൽ
# endif
	{
		rc = run_list(funcp->body);
	}

	IF_HUSH_LOCAL(G.func_nest_level--;)
	IF_HUSH_LOCAL(leave_var_nest_level();)

	G_flag_return_in_progress = sv_flg;
# എങ്കിൽ ENABLE_HUSH_TRAP
	debug_printf_exec("G.return_exitcode=-1\n");
	G.return_exitcode = -1; /* invalidate stashed return value */
# endif

	restore_G_args(&sv, argv);

	തിരികെ rc;
}
#endif /* ENABLE_HUSH_FUNCTIONS */

#ifndef __U_BOOT__
#എങ്കിൽ BB_MMU
#define exec_builtin(to_free, x, argv) \
	exec_builtin(x, argv)
#അല്ലെങ്കിൽ
#define exec_builtin(to_free, x, argv) \
	exec_builtin(to_free, argv)
#endif
സ്ഥിതം ശൂന്യം exec_builtin(അക്ഷരം ***to_free,
		സ്ഥിരം ഘടന built_in_command *x,
		അക്ഷരം **argv) NORETURN;
സ്ഥിതം ശൂന്യം exec_builtin(അക്ഷരം ***to_free,
		സ്ഥിരം ഘടന built_in_command *x,
		അക്ഷരം **argv)
{
#എങ്കിൽ BB_MMU
	പൂർണ്ണസംഖ്യ rcode;
//?	close_saved_fds_and_FILE_fds();
	rcode = x->b_function(argv);
	fflush_all();
	_exit(rcode);
#അല്ലെങ്കിൽ
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
സ്ഥിതം ശൂന്യം execvp_or_die(അക്ഷരം **argv) NORETURN;
സ്ഥിതം ശൂന്യം execvp_or_die(അക്ഷരം **argv)
{
	പൂർണ്ണസംഖ്യ e;
	debug_printf_exec("execing '%s'\n", argv[0]);
	/* Don't propagate SIG_IGN to the child */
	എങ്കിൽ (SPECIAL_JOBSTOP_SIGS != 0)
		switch_off_special_sigs(G.special_sig_mask & SPECIAL_JOBSTOP_SIGS);
	execvp(argv[0], argv);
	e = 2;
	എങ്കിൽ (errno == EACCES) e = 126;
	എങ്കിൽ (errno == ENOENT) e = 127;
	bb_perror_സന്ദേശം("can't execute '%s'", argv[0]);
	_exit(e);
}

#എങ്കിൽ ENABLE_HUSH_MODE_X
സ്ഥിതം ശൂന്യം x_mode_print_optionally_squoted(സ്ഥിരം അക്ഷരം *str)
{
	unsigned len;
	സ്ഥിരം അക്ഷരം *cp;

	cp = str;

	/* the set of chars which-cause-string-to-be-squoted mimics bash */
	/* test a char with: bash -c 'set -x; echo "CH"' */
	എങ്കിൽ (str[strcspn(str, "\\\"'`$(){}[]<>;#&|~*?!^"
			" " "\001\002\003\004\005\006\007"
			"\010\011\012\013\014\015\016\017"
			"\020\021\022\023\024\025\026\027"
			"\030\031\032\033\034\035\036\037"
			)
		] == '\0'
	) {
		/* string has no special chars */
		x_mode_addstr(str);
		തിരികെ;
	}

	cp = str;
	വേണ്ടി (;;) {
		/* print '....' up to EOL or first squote */
		len = (പൂർണ്ണസംഖ്യ)(strchrnul(cp, '\'') - cp);
		എങ്കിൽ (len != 0) {
			x_mode_addchr('\'');
			x_mode_addblock(cp, len);
			x_mode_addchr('\'');
			cp += len;
		}
		എങ്കിൽ (*cp == '\0')
			നിർത്തുക;
		/* string contains squote(s), print them as \' */
		x_mode_addchr('\\');
		x_mode_addchr('\'');
		cp++;
	}
}
സ്ഥിതം ശൂന്യം dump_cmd_in_x_mode(അക്ഷരം **argv)
{
	എങ്കിൽ (G_x_mode && argv) {
		unsigned n;

		/* "+[+++...][ cmd...]\n\0" */
		x_mode_prefix();
		n = 0;
		ആകുമ്പോൾ (argv[n]) {
			x_mode_addchr(' ');
			എങ്കിൽ (argv[n][0] == '\0') {
				x_mode_addchr('\'');
				x_mode_addchr('\'');
			} അല്ലെങ്കിൽ {
				x_mode_print_optionally_squoted(argv[n]);
			}
			n++;
		}
		x_mode_flush();
	}
}
#അല്ലെങ്കിൽ
# define dump_cmd_in_x_mode(argv) ((ശൂന്യം)0)
#endif
#endif /* !__U_BOOT__ */

#ifndef __U_BOOT__
#എങ്കിൽ ENABLE_HUSH_COMMAND
സ്ഥിതം ശൂന്യം if_command_vV_print_and_exit(അക്ഷരം opt_vV, അക്ഷരം *cmd, സ്ഥിരം അക്ഷരം *explanation)
{
	അക്ഷരം *to_free;

	എങ്കിൽ (!opt_vV)
		തിരികെ;

	to_free = ശൂന്യ;
	എങ്കിൽ (!explanation) {
		അക്ഷരം *path = getenv("PATH");
		explanation = to_free = find_executable(cmd, &path); /* path == NULL is ok */
		എങ്കിൽ (!explanation)
			_exit(1); /* PROG was not found */
		എങ്കിൽ (opt_vV != 'V')
			cmd = to_free; /* -v PROG prints "/path/to/PROG" */
	}
	printf((opt_vV == 'V') ? "%s is %s\n" : "%s\n", cmd, explanation);
	സ്വതന്ത്രം(to_free);
	fflush_all();
	_exit(0);
}
#അല്ലെങ്കിൽ
# define if_command_vV_print_and_exit(a,b,c) ((ശൂന്യം)0)
#endif
#endif /* !__U_BOOT__ */

#എങ്കിൽ BB_MMU
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
സ്ഥിതം ശൂന്യം pseudo_exec_argv(nommu_save_t *nommu_save,
		അക്ഷരം **argv, പൂർണ്ണസംഖ്യ assignment_cnt,
		അക്ഷരം **argv_expanded) NORETURN;
സ്ഥിതം NOINLINE ശൂന്യം pseudo_exec_argv(nommu_save_t *nommu_save,
		അക്ഷരം **argv, പൂർണ്ണസംഖ്യ assignment_cnt,
		അക്ഷരം **argv_expanded)
{
	സ്ഥിരം ഘടന built_in_command *x;
	ഘടന variable **sv_shadowed;
	അക്ഷരം **new_env;
	IF_HUSH_COMMAND(അക്ഷരം opt_vV = 0;)
	IF_HUSH_FUNCTIONS(സ്ഥിരം ഘടന function *funcp;)

	new_env = expand_assignments(argv, assignment_cnt);
	dump_cmd_in_x_mode(new_env);

	എങ്കിൽ (!argv[assignment_cnt]) {
		/* Case when we are here: ... | var=val | ...
		 * (note that we do not exit early, i.e., do not optimize out
		 * expand_assignments(): think about ... | var=`sleep 1` | ...
		 */
		free_strings(new_env);
		_exit_SUCCESS();
	}

	sv_shadowed = G.shadowed_vars_pp;
#എങ്കിൽ BB_MMU
	G.shadowed_vars_pp = ശൂന്യ; /* "don't save, free them instead" */
#അല്ലെങ്കിൽ
	G.shadowed_vars_pp = &nommu_save->old_vars;
	G.var_nest_level++;
#endif
	set_vars_and_save_old(new_env);
	G.shadowed_vars_pp = sv_shadowed;

	എങ്കിൽ (argv_expanded) {
		argv = argv_expanded;
	} അല്ലെങ്കിൽ {
		argv = expand_strvec_to_strvec(argv + assignment_cnt);
#എങ്കിൽ !BB_MMU
		nommu_save->argv = argv;
#endif
	}
	dump_cmd_in_x_mode(argv);

#എങ്കിൽ ENABLE_FEATURE_SH_STANDALONE || BB_MMU
	എങ്കിൽ (strchr(argv[0], '/') != ശൂന്യ)
		പോകുക skip;
#endif

#എങ്കിൽ ENABLE_HUSH_FUNCTIONS
	/* Check if the command matches any functions (this goes before bltins) */
	funcp = find_function(argv[0]);
	എങ്കിൽ (funcp)
		exec_function(&nommu_save->argv_from_re_execing, funcp, argv);
#endif

#എങ്കിൽ ENABLE_HUSH_COMMAND
	/* "command BAR": run BAR without looking it up among functions
	 * "command -v BAR": print "BAR" or "/path/to/BAR"; or exit 1
	 * "command -V BAR": print "BAR is {a function,a shell builtin,/path/to/BAR}"
	 */
	ആകുമ്പോൾ (strcmp(argv[0], "command") == 0 && argv[1]) {
		അക്ഷരം *p;

		argv++;
		p = *argv;
		എങ്കിൽ (p[0] != '-' || !p[1])
			തുടരുക; /* bash allows "command command command [-OPT] BAR" */

		വേണ്ടി (;;) {
			p++;
			തിരഞ്ഞെടുപ്പ് (*p) {
			അവസ്ഥ '\0':
				argv++;
				p = *argv;
				എങ്കിൽ (p[0] != '-' || !p[1])
					പോകുക after_opts;
				തുടരുക; /* next arg is also -opts, process it too */
			അവസ്ഥ 'v':
			അവസ്ഥ 'V':
				opt_vV = *p;
				തുടരുക;
			സ്ഥിരസ്ഥിതി:
				bb_error_സന്ദേശം_and_die("%s: %s: invalid option", "command", argv[0]);
			}
		}
	}
 after_opts:
# എങ്കിൽ ENABLE_HUSH_FUNCTIONS
	എങ്കിൽ (opt_vV && find_function(argv[0]))
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
	എങ്കിൽ (x) {
		if_command_vV_print_and_exit(opt_vV, argv[0], "a shell builtin");
		exec_builtin(&nommu_save->argv_from_re_execing, x, argv);
	}

#എങ്കിൽ ENABLE_FEATURE_SH_STANDALONE
	/* Check if the command matches any busybox applets */
	{
		പൂർണ്ണസംഖ്യ a = find_applet_by_name(argv[0]);
		എങ്കിൽ (a >= 0) {
			if_command_vV_print_and_exit(opt_vV, argv[0], "an applet");
# എങ്കിൽ BB_MMU /* see above why on NOMMU it is not allowed */
			എങ്കിൽ (APPLET_IS_NOEXEC(a)) {
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
			എങ്കിൽ (SPECIAL_JOBSTOP_SIGS != 0)
				switch_off_special_sigs(G.special_sig_mask & SPECIAL_JOBSTOP_SIGS);
			execv(bb_busybox_exec_path, argv);
			/* If they called chroot or otherwise made the binary no longer
			 * executable, fall through */
		}
	}
#endif

#എങ്കിൽ ENABLE_FEATURE_SH_STANDALONE || BB_MMU
 skip:
#endif
	if_command_vV_print_and_exit(opt_vV, argv[0], ശൂന്യ);
	execvp_or_die(argv);
}

/* Called after [v]fork() in run_pipe
 */
സ്ഥിതം ശൂന്യം pseudo_exec(nommu_save_t *nommu_save,
		ഘടന command *command,
		അക്ഷരം **argv_expanded) NORETURN;
സ്ഥിതം ശൂന്യം pseudo_exec(nommu_save_t *nommu_save,
		ഘടന command *command,
		അക്ഷരം **argv_expanded)
{
#എങ്കിൽ ENABLE_HUSH_FUNCTIONS
	എങ്കിൽ (command->cmd_type == CMD_FUNCDEF) {
		/* Ignore funcdefs in pipes:
		 * true | f() { cmd }
		 */
		_exit(0);
	}
#endif

	എങ്കിൽ (command->argv) {
		pseudo_exec_argv(nommu_save, command->argv,
				command->assignment_cnt, argv_expanded);
	}

	എങ്കിൽ (command->group) {
		/* Cases when we are here:
		 * ( list )
		 * { list } &
		 * ... | ( list ) | ...
		 * ... | { list } | ...
		 */
#എങ്കിൽ BB_MMU
		പൂർണ്ണസംഖ്യ rcode;
		debug_printf_exec("pseudo_exec: run_list\n");
		reset_traps_to_defaults();
		rcode = run_list(command->group);
		/* OK to leak memory by not calling free_pipe_list,
		 * since this process is about to exit */
		_exit(rcode);
#അല്ലെങ്കിൽ
		re_execute_shell(&nommu_save->argv_from_re_execing,
				command->group_as_string,
				G.global_argv[0],
				G.global_argv + 1,
				ശൂന്യ);
#endif
	}

	/* Case when we are here: ... | >file */
	debug_printf_exec("pseudo_exec'ed null command\n");
	_exit_SUCCESS();
}

#എങ്കിൽ ENABLE_HUSH_JOB
സ്ഥിതം സ്ഥിരം അക്ഷരം *get_cmdtext(ഘടന pipe *pi)
{
	അക്ഷരം **argv;
	അക്ഷരം *p;
	പൂർണ്ണസംഖ്യ len;

	/* This is subtle. ->cmdtext is created only on first backgrounding.
	 * (Think "cat, <ctrl-z>, fg, <ctrl-z>, fg, <ctrl-z>...." here...)
	 * On subsequent bg argv is trashed, but we won't use it */
	എങ്കിൽ (pi->cmdtext)
		തിരികെ pi->cmdtext;

	argv = pi->cmds[0].argv;
	എങ്കിൽ (!argv) {
		pi->cmdtext = xzalloc(1);
		തിരികെ pi->cmdtext;
	}
	len = 0;
	ചെയ്യുക {
		len += strlen(*argv) + 1;
	} ആകുമ്പോൾ (*++argv);
	p = xmalloc(len);
	pi->cmdtext = p;
	argv = pi->cmds[0].argv;
	ചെയ്യുക {
		p = stpcpy(p, *argv);
		*p++ = ' ';
	} ആകുമ്പോൾ (*++argv);
	p[-1] = '\0';
	തിരികെ pi->cmdtext;
}

സ്ഥിതം ശൂന്യം remove_job_from_table(ഘടന pipe *pi)
{
	ഘടന pipe *prev_pipe;

	എങ്കിൽ (pi == G.job_list) {
		G.job_list = pi->next;
	} അല്ലെങ്കിൽ {
		prev_pipe = G.job_list;
		ആകുമ്പോൾ (prev_pipe->next != pi)
			prev_pipe = prev_pipe->next;
		prev_pipe->next = pi->next;
	}
	G.last_jobid = 0;
	എങ്കിൽ (G.job_list)
		G.last_jobid = G.job_list->jobid;
}

സ്ഥിതം ശൂന്യം delete_finished_job(ഘടന pipe *pi)
{
	remove_job_from_table(pi);
	free_pipe(pi);
}

സ്ഥിതം ശൂന്യം clean_up_last_dead_job(ശൂന്യം)
{
	എങ്കിൽ (G.job_list && !G.job_list->alive_cmds)
		delete_finished_job(G.job_list);
}

സ്ഥിതം ശൂന്യം തിരുകുക_job_into_table(ഘടന pipe *pi)
{
	ഘടന pipe *job, **jobp;
	പൂർണ്ണസംഖ്യ i;

	clean_up_last_dead_job();

	/* Find the end of the list, and find next job ID to use */
	i = 0;
	jobp = &G.job_list;
	ആകുമ്പോൾ ((job = *jobp) != ശൂന്യ) {
		എങ്കിൽ (job->jobid > i)
			i = job->jobid;
		jobp = &job->next;
	}
	pi->jobid = i + 1;

	/* Create a new job struct at the end */
	job = *jobp = xmemdup(pi, വലുപ്പം(*pi));
	job->next = ശൂന്യ;
	job->cmds = xzalloc(വലുപ്പം(pi->cmds[0]) * pi->അക്കം_cmds);
	/* Cannot copy entire pi->cmds[] vector! This causes double frees */
	വേണ്ടി (i = 0; i < pi->അക്കം_cmds; i++) {
		job->cmds[i].pid = pi->cmds[i].pid;
		/* all other fields are not used and stay zero */
	}
	job->cmdtext = xstrdup(get_cmdtext(pi));

	എങ്കിൽ (G_interactive_fd)
		printf("[%u] %u %s\n", job->jobid, (unsigned)job->cmds[0].pid, job->cmdtext);
	G.last_jobid = job->jobid;
}
#endif /* JOB */

സ്ഥിതം പൂർണ്ണസംഖ്യ job_exited_or_stopped(ഘടന pipe *pi)
{
	പൂർണ്ണസംഖ്യ rcode, i;

	എങ്കിൽ (pi->alive_cmds != pi->stopped_cmds)
		തിരികെ -1;

	/* All processes in fg pipe have exited or stopped */
	rcode = 0;
	i = pi->അക്കം_cmds;
	ആകുമ്പോൾ (--i >= 0) {
		rcode = pi->cmds[i].cmd_exitcode;
		/* usually last process gives overall exitstatus,
		 * but with "set -o pipefail", last *failed* process does */
		എങ്കിൽ (G.o_opt[OPT_O_PIPEFAIL] == 0 || rcode != 0)
			നിർത്തുക;
	}
	IF_HAS_KEYWORDS(എങ്കിൽ (pi->pi_inverted) rcode = !rcode;)
	തിരികെ rcode;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ process_wait_result(ഘടന pipe *fg_pipe, pid_t childpid, പൂർണ്ണസംഖ്യ status)
{
#എങ്കിൽ ENABLE_HUSH_JOB
	ഘടന pipe *pi;
#endif
	പൂർണ്ണസംഖ്യ i, dead;

	dead = WIFEXITED(status) || WIFSIGNALED(status);

#എങ്കിൽ DEBUG_JOBS
	എങ്കിൽ (WIFSTOPPED(status))
		debug_printf_jobs("pid %d stopped by sig %d (exitcode %d)\n",
				childpid, WSTOPSIG(status), WEXITSTATUS(status));
	എങ്കിൽ (WIFSIGNALED(status))
		debug_printf_jobs("pid %d killed by sig %d (exitcode %d)\n",
				childpid, WTERMSIG(status), WEXITSTATUS(status));
	എങ്കിൽ (WIFEXITED(status))
		debug_printf_jobs("pid %d exited, exitcode %d\n",
				childpid, WEXITSTATUS(status));
#endif
	/* Were we asked to wait for a fg pipe? */
	എങ്കിൽ (fg_pipe) {
		i = fg_pipe->അക്കം_cmds;

		ആകുമ്പോൾ (--i >= 0) {
			പൂർണ്ണസംഖ്യ rcode;

			debug_printf_jobs("check pid %d\n", fg_pipe->cmds[i].pid);
			എങ്കിൽ (fg_pipe->cmds[i].pid != childpid)
				തുടരുക;
			എങ്കിൽ (dead) {
				പൂർണ്ണസംഖ്യ ex;
				fg_pipe->cmds[i].pid = 0;
				fg_pipe->alive_cmds--;
				ex = WEXITSTATUS(status);
				/* bash prints killer signal's name for *last*
				 * process in pipe (prints just newline for SIGINT/SIGPIPE).
				 * Mimic this. Example: "sleep 5" + (^\ or kill -QUIT)
				 */
				എങ്കിൽ (WIFSIGNALED(status)) {
					പൂർണ്ണസംഖ്യ sig = WTERMSIG(status);
#എങ്കിൽ ENABLE_HUSH_JOB
					എങ്കിൽ (G.run_list_level == 1
					/* ^^^^^ Do not print in nested contexts, example:
					 * echo `sleep 1; sh -c 'kill -9 $$'` - prints "137", NOT "Killed 137"
					 */
					 && i == fg_pipe->അക്കം_cmds-1
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
			} അല്ലെങ്കിൽ {
				fg_pipe->stopped_cmds++;
			}
			debug_printf_jobs("fg_pipe: alive_cmds %d stopped_cmds %d\n",
					fg_pipe->alive_cmds, fg_pipe->stopped_cmds);
			rcode = job_exited_or_stopped(fg_pipe);
			എങ്കിൽ (rcode >= 0) {
/* Note: *non-interactive* bash does not continue if all processes in fg pipe
 * are stopped. Testcase: "cat | cat" in a script (not on command line!)
 * and "killall -STOP cat" */
				എങ്കിൽ (G_interactive_fd) {
#എങ്കിൽ ENABLE_HUSH_JOB
					എങ്കിൽ (fg_pipe->alive_cmds != 0)
						തിരുകുക_job_into_table(fg_pipe);
#endif
					തിരികെ rcode;
				}
				എങ്കിൽ (fg_pipe->alive_cmds == 0)
					തിരികെ rcode;
			}
			/* There are still running processes in the fg_pipe */
			തിരികെ -1;
		}
		/* It wasn't in fg_pipe, look for process in bg pipes */
	}

#എങ്കിൽ ENABLE_HUSH_JOB
	/* We were asked to wait for bg or orphaned children */
	/* No need to remember exitcode in this case */
	വേണ്ടി (pi = G.job_list; pi; pi = pi->next) {
		വേണ്ടി (i = 0; i < pi->അക്കം_cmds; i++) {
			എങ്കിൽ (pi->cmds[i].pid == childpid)
				പോകുക found_pi_and_prognum;
		}
	}
	/* Happens when shell is used as init process (init=/bin/sh) */
	debug_printf("checkjobs: pid %d was not in our list!\n", childpid);
	തിരികെ -1; /* this wasn't a process from fg_pipe */

 found_pi_and_prognum:
	എങ്കിൽ (dead) {
		/* child exited */
		പൂർണ്ണസംഖ്യ rcode = WEXITSTATUS(status);
		എങ്കിൽ (WIFSIGNALED(status))
			/* NB: not 128 + sig, MIPS has sig 128 */
			rcode = 128 | WTERMSIG(status);
		pi->cmds[i].cmd_exitcode = rcode;
		എങ്കിൽ (G.last_bg_pid == pi->cmds[i].pid)
			G.last_bg_pid_exitcode = rcode;
		pi->cmds[i].pid = 0;
		pi->alive_cmds--;
		എങ്കിൽ (!pi->alive_cmds) {
# എങ്കിൽ ENABLE_HUSH_BASH_COMPAT
			G.dead_job_exitcode = job_exited_or_stopped(pi);
# endif
			എങ്കിൽ (G_interactive_fd) {
				printf(JOB_STATUS_FORMAT, pi->jobid,
						"Done", pi->cmdtext);
				delete_finished_job(pi);
			} അല്ലെങ്കിൽ {
/*
 * bash deletes finished jobs from job table only in interactive mode,
 * after "jobs" cmd, or if pid of a new process matches one of the old ones
 * (see cleanup_dead_jobs(), delete_old_job(), J_NOTIFIED in bash source).
 * Testcase script: "(exit 3) & sleep 1; wait %1; echo $?" prints 3 in bash.
 * We only retain one "dead" job, if it's the single job on the list.
 * This covers most of real-world scenarios where this is useful.
 */
				എങ്കിൽ (pi != G.job_list)
					delete_finished_job(pi);
			}
		}
	} അല്ലെങ്കിൽ {
		/* child stopped */
		pi->stopped_cmds++;
	}
#endif
	തിരികെ -1; /* this wasn't a process from fg_pipe */
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
സ്ഥിതം പൂർണ്ണസംഖ്യ checkjobs(ഘടന pipe *fg_pipe, pid_t waitfor_pid)
{
	പൂർണ്ണസംഖ്യ attributes;
	പൂർണ്ണസംഖ്യ status;
	പൂർണ്ണസംഖ്യ rcode = 0;

	debug_printf_jobs("checkjobs %p\n", fg_pipe);

	attributes = WUNTRACED;
	എങ്കിൽ (fg_pipe == ശൂന്യ)
		attributes |= WNOHANG;

	errno = 0;
#എങ്കിൽ ENABLE_HUSH_FAST
	എങ്കിൽ (G.handled_SIGCHLD == G.എണ്ണം_SIGCHLD) {
//bb_error_msg("[%d] checkjobs: G.count_SIGCHLD:%d G.handled_SIGCHLD:%d children?:%d fg_pipe:%p",
//getpid(), G.count_SIGCHLD, G.handled_SIGCHLD, G.we_have_children, fg_pipe);
		/* There was neither fork nor SIGCHLD since last waitpid */
		/* Avoid doing waitpid syscall if possible */
		എങ്കിൽ (!G.we_have_children) {
			errno = ECHILD;
			തിരികെ -1;
		}
		എങ്കിൽ (fg_pipe == ശൂന്യ) { /* is WNOHANG set? */
			/* We have children, but they did not exit
			 * or stop yet (we saw no SIGCHLD) */
			തിരികെ 0;
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
	ആകുമ്പോൾ (1) {
		pid_t childpid;
#എങ്കിൽ ENABLE_HUSH_FAST
		പൂർണ്ണസംഖ്യ i;
		i = G.എണ്ണം_SIGCHLD;
#endif
		childpid = waitpid(-1, &status, attributes);
		എങ്കിൽ (childpid <= 0) {
			എങ്കിൽ (childpid && errno != ECHILD)
				bb_simple_perror_സന്ദേശം("waitpid");
#എങ്കിൽ ENABLE_HUSH_FAST
			അല്ലെങ്കിൽ { /* Until next SIGCHLD, waitpid's are useless */
				G.we_have_children = (childpid == 0);
				G.handled_SIGCHLD = i;
//bb_error_msg("[%d] checkjobs: waitpid returned <= 0, G.count_SIGCHLD:%d G.handled_SIGCHLD:%d", getpid(), G.count_SIGCHLD, G.handled_SIGCHLD);
			}
#endif
			/* ECHILD (no children), or 0 (no change in children status) */
			rcode = childpid;
			നിർത്തുക;
		}
		rcode = process_wait_result(fg_pipe, childpid, status);
		എങ്കിൽ (rcode >= 0) {
			/* fg_pipe exited or stopped */
			നിർത്തുക;
		}
		എങ്കിൽ (childpid == waitfor_pid) { /* "wait PID" */
			debug_printf_exec("childpid==waitfor_pid:%d status:0x%08x\n", childpid, status);
			rcode = WEXITSTATUS(status);
			എങ്കിൽ (WIFSIGNALED(status))
				rcode = 128 | WTERMSIG(status);
			എങ്കിൽ (WIFSTOPPED(status))
				/* bash: "cmd & wait $!" and cmd stops: $? = 128 | stopsig */
				rcode = 128 | WSTOPSIG(status);
			rcode++;
			നിർത്തുക; /* "wait PID" called us, give it exitcode+1 */
		}
#എങ്കിൽ ENABLE_HUSH_BASH_COMPAT
		എങ്കിൽ (-1 == waitfor_pid /* "wait -n" (wait for any one job) */
		 && G.dead_job_exitcode >= 0 /* some job did finish */
		) {
			debug_printf_exec("waitfor_pid:-1\n");
			rcode = G.dead_job_exitcode + 1;
			നിർത്തുക;
		}
#endif
		/* This wasn't one of our processes, or */
		/* fg_pipe still has running processes, do waitpid again */
	} /* while (waitpid succeeds)... */

	തിരികെ rcode;
}

#എങ്കിൽ ENABLE_HUSH_JOB
സ്ഥിതം പൂർണ്ണസംഖ്യ checkjobs_and_fg_shell(ഘടന pipe *fg_pipe)
{
	pid_t p;
	പൂർണ്ണസംഖ്യ rcode = checkjobs(fg_pipe, 0 /*(no pid to wait for)*/);
	എങ്കിൽ (G_saved_tty_pgrp) {
		/* Job finished, move the shell to the foreground */
		p = getpgrp(); /* our process group id */
		debug_printf_jobs("fg'ing ourself: getpgrp()=%d\n", (പൂർണ്ണസംഖ്യ)p);
		tcsetpgrp(G_interactive_fd, p);
	}
	തിരികെ rcode;
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
സ്ഥിതം ശൂന്യം set_G_ifs(ശൂന്യം)
{
	/* Testcase: set -- q w e; (IFS='' echo "$*"; IFS=''; echo "$*"); echo "$*"
	 * Result should be 3 lines: q w e, qwe, q w e
	 */
	എങ്കിൽ (G.ifs_whitespace != G.ifs)
		സ്വതന്ത്രം(G.ifs_whitespace);
	G.ifs = get_local_var_മൂല്യം("IFS");
	എങ്കിൽ (G.ifs) {
		അക്ഷരം *p;
		G.ifs_whitespace = (അക്ഷരം*)G.ifs;
		p = skip_whitespace(G.ifs);
		എങ്കിൽ (*p) {
			/* Not all $IFS is whitespace */
			അക്ഷരം *d;
			പൂർണ്ണസംഖ്യ len = p - G.ifs;
			p = skip_non_whitespace(p);
			G.ifs_whitespace = xmalloc(len + strlen(p) + 1); /* can overestimate */
			d = mempcpy(G.ifs_whitespace, G.ifs, len);
			ആകുമ്പോൾ (*p) {
				എങ്കിൽ (isspace(*p))
					*d++ = *p;
				p++;
			}
			*d = '\0';
		}
	} അല്ലെങ്കിൽ {
		G.ifs = defifs;
		G.ifs_whitespace = (അക്ഷരം*)G.ifs;
	}
}
#ifndef __U_BOOT__
#എങ്കിൽ !ENABLE_HUSH_MODE_X
#define redirect_and_varexp_helper(command, sqp, argv_expanded) \
	redirect_and_varexp_helper(command, sqp)
#endif
സ്ഥിതം പൂർണ്ണസംഖ്യ redirect_and_varexp_helper(
		ഘടന command *command,
		ഘടന squirrel **sqp,
		അക്ഷരം **argv_expanded)
{
	/* Assignments occur before redirects. Try:
	 * a=`sleep 1` sleep 2 3>/qwe/rty
	 */

	അക്ഷരം **new_env = expand_assignments(command->argv, command->assignment_cnt);
	dump_cmd_in_x_mode(new_env);
	dump_cmd_in_x_mode(argv_expanded);
	/* this takes ownership of new_env[i] elements, and frees new_env: */
	set_vars_and_save_old(new_env);

	തിരികെ setup_redirects(command, sqp);
}
#endif /* !__U_BOOT__ */

സ്ഥിതം NOINLINE പൂർണ്ണസംഖ്യ run_pipe(ഘടന pipe *pi)
{
	സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം null_ptr = ശൂന്യ;

	പൂർണ്ണസംഖ്യ cmd_no;
#ifndef __U_BOOT__
	പൂർണ്ണസംഖ്യ next_infd;
#endif /* !__U_BOOT__ */
	ഘടന command *command;
	അക്ഷരം **argv_expanded;
	അക്ഷരം **argv;
#ifndef __U_BOOT__
	ഘടന squirrel *squirrel = ശൂന്യ;
#endif /* !__U_BOOT__ */
	പൂർണ്ണസംഖ്യ rcode;

#ifdef __U_BOOT__
	/*
	 * Set rcode here to avoid returning a garbage value in the middle of
	 * the function.
	 * Also, if an error occurs, rcode value would be changed and last
	 * return will signal the error.
	 */
	rcode = 0;
#endif /* __U_BOOT__ */

	debug_printf_exec("run_pipe start: members:%d\n", pi->അക്കം_cmds);
	debug_enter();

	set_G_ifs();

#ifndef __U_BOOT__
	IF_HUSH_JOB(pi->pgrp = -1;)
	pi->stopped_cmds = 0;
#endif /* !__U_BOOT__ */
	command = &pi->cmds[0];
	argv_expanded = ശൂന്യ;

#ifndef __U_BOOT__
	എങ്കിൽ (pi->അക്കം_cmds != 1
	 || pi->followup == PIPE_BG
	 || command->cmd_type == CMD_SUBSHELL
	) {
		പോകുക must_fork;
	}

	pi->alive_cmds = 1;
#endif /* !__U_BOOT__ */

	debug_printf_exec(": group:%p argv:'%s'\n",
		command->group, command->argv ? command->argv[0] : "NONE");

	എങ്കിൽ (command->group) {
#എങ്കിൽ ENABLE_HUSH_FUNCTIONS
		എങ്കിൽ (command->cmd_type == CMD_FUNCDEF) {
			/* "executing" func () { list } */
			ഘടന function *funcp;

			funcp = new_function(command->argv[0]);
			/* funcp->name is already set to argv[0] */
			funcp->body = command->group;
# എങ്കിൽ !BB_MMU
			funcp->body_as_string = command->group_as_string;
			command->group_as_string = ശൂന്യ;
# endif
			command->group = ശൂന്യ;
			command->argv[0] = ശൂന്യ;
			debug_printf_exec("cmd %p has child func at %p\n", command, funcp);
			funcp->parent_cmd = command;
			command->child_func = funcp;

			debug_printf_exec("run_pipe: return EXIT_SUCCESS\n");
			debug_leave();
			തിരികെ EXIT_SUCCESS;
		}
#endif
		/* { list } */
		debug_printf_exec("non-subshell group\n");
		rcode = 1; /* exitcode if redir failed */
#ifndef __U_BOOT__
		എങ്കിൽ (setup_redirects(command, &squirrel) == 0) {
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
		IF_HAS_KEYWORDS(എങ്കിൽ (pi->pi_inverted) rcode = !rcode;)
		debug_leave();
		debug_printf_exec("run_pipe: return %d\n", rcode);
		തിരികെ rcode;
	}

	argv = command->argv ? command->argv : (അക്ഷരം **) &null_ptr;
	{
#ifndef __U_BOOT__
		സ്ഥിരം ഘടന built_in_command *x;
		IF_HUSH_FUNCTIONS(സ്ഥിരം ഘടന function *funcp;)
		IF_NOT_HUSH_FUNCTIONS(എണ്ണൽ { funcp = 0 };)
		ഘടന variable **sv_shadowed;
#endif /* !__U_BOOT__ */
		ഘടന variable *old_vars;

#എങ്കിൽ ENABLE_HUSH_LINENO_VAR
		G.execute_lineno = command->lineno;
#endif

		എങ്കിൽ (argv[command->assignment_cnt] == ശൂന്യ) {
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
			ആകുമ്പോൾ (i < command->assignment_cnt) {
				അക്ഷരം *p = expand_string_to_string(argv[i],
						EXP_FLAG_ESC_GLOB_CHARS,
						/*unbackslash:*/ 1
				);
#എങ്കിൽ ENABLE_HUSH_MODE_X
				എങ്കിൽ (G_x_mode) {
					അക്ഷരം *eq;
					എങ്കിൽ (i == 0)
						x_mode_prefix();
					x_mode_addchr(' ');
					eq = strchrnul(p, '=');
					എങ്കിൽ (*eq) eq++;
					x_mode_addblock(p, (eq - p));
					x_mode_print_optionally_squoted(eq);
					x_mode_flush();
				}
#endif
				debug_printf_env("set shell var:'%s'->'%s'\n", *argv, p);
#ifndef __U_BOOT__
				എങ്കിൽ (set_local_var0(p)) {
#അല്ലെങ്കിൽ /* __U_BOOT__ */
				എങ്കിൽ (set_local_var_modern(p, /*flag:*/ 0)) {
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
			എങ്കിൽ (rcode == 0)
				rcode = G.expand_exitcode;
			IF_HAS_KEYWORDS(എങ്കിൽ (pi->pi_inverted) rcode = !rcode;)
			debug_leave();
			debug_printf_exec("run_pipe: return %d\n", rcode);
			തിരികെ rcode;
		}

		/* Expand the rest into (possibly) many strings each */
#എങ്കിൽ defined(CMD_TEST2_SINGLEWORD_NOGLOB)
		എങ്കിൽ (command->cmd_type == CMD_TEST2_SINGLEWORD_NOGLOB)
			argv_expanded = expand_strvec_to_strvec_singleword_noglob(argv + command->assignment_cnt);
		അല്ലെങ്കിൽ
#endif
#എങ്കിൽ defined(CMD_SINGLEWORD_NOGLOB)
		എങ്കിൽ (command->cmd_type == CMD_SINGLEWORD_NOGLOB)
			argv_expanded = expand_strvec_to_strvec_singleword_noglob(argv + command->assignment_cnt);
		അല്ലെങ്കിൽ
#endif
			argv_expanded = expand_strvec_to_strvec(argv + command->assignment_cnt);

		/* If someone gives us an empty string: `cmd with empty output` */
		എങ്കിൽ (!argv_expanded[0]) {
			സ്വതന്ത്രം(argv_expanded);
			/* `false` still has to set exitcode 1 */
			G.expand_exitcode = G.last_exitcode;
			പോകുക only_assignments;
		}

		old_vars = ശൂന്യ;
#ifndef __U_BOOT__
		sv_shadowed = G.shadowed_vars_pp;

		/* Check if argv[0] matches any functions (this goes before bltins) */
		IF_HUSH_FUNCTIONS(funcp = find_function(argv_expanded[0]);)
		IF_HUSH_FUNCTIONS(x = ശൂന്യ;)
		IF_HUSH_FUNCTIONS(എങ്കിൽ (!funcp))
			x = find_builtin(argv_expanded[0]);
		എങ്കിൽ (x || funcp) {
			എങ്കിൽ (x && x->b_function == builtin_exec && argv_expanded[1] == ശൂന്യ) {
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

				പോകുക clean_up_and_ret1;
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
			എങ്കിൽ (rcode == 0) {
				എങ്കിൽ (!funcp) {
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
#എങ്കിൽ ENABLE_HUSH_FUNCTIONS
				അല്ലെങ്കിൽ {
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
		} അല്ലെങ്കിൽ
		എങ്കിൽ (ENABLE_FEATURE_SH_NOFORK && അക്കം_APPLETS > 1) {
			പൂർണ്ണസംഖ്യ n = find_applet_by_name(argv_expanded[0]);
			എങ്കിൽ (n < 0 || !APPLET_IS_NOFORK(n))
				പോകുക must_fork;

			enter_var_nest_level();
			/* Collect all variables "shadowed" by helper into old_vars list */
			G.shadowed_vars_pp = &old_vars;
			rcode = redirect_and_varexp_helper(command, &squirrel, argv_expanded);
			G.shadowed_vars_pp = sv_shadowed;

			എങ്കിൽ (rcode == 0) {
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
		} അല്ലെങ്കിൽ
			പോകുക must_fork;

		restore_redirects(squirrel);
 clean_up_and_ret1:
		leave_var_nest_level();
		add_vars(old_vars);

		/*
		 * Try "usleep 99999999" + ^C + "echo $?"
		 * with FEATURE_SH_NOFORK=y.
		 */
		എങ്കിൽ (!funcp) {
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
			എങ്കിൽ (sigismember(&G.pending_set, SIGINT))
				rcode = 128 | SIGINT;
		}
		സ്വതന്ത്രം(argv_expanded);
		IF_HAS_KEYWORDS(എങ്കിൽ (pi->pi_inverted) rcode = !rcode;)
		debug_leave();
		debug_printf_exec("run_pipe return %d\n", rcode);
		തിരികെ rcode;
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
	ആകുമ്പോൾ (cmd_no < pi->അക്കം_cmds) {
#ifndef __U_BOOT__
		ഘടന fd_pair pipefds;
#എങ്കിൽ !BB_MMU
		പൂർണ്ണസംഖ്യ sv_var_nest_level = G.var_nest_level;
		അസ്ഥിര nommu_save_t nommu_save;
		nommu_save.old_vars = ശൂന്യ;
		nommu_save.argv = ശൂന്യ;
		nommu_save.argv_from_re_execing = ശൂന്യ;
#endif
#endif /* !__U_BOOT__ */
		command = &pi->cmds[cmd_no];
		cmd_no++;

#ifdef __U_BOOT__
		/* Replace argv and argc by expanded if it exists. */
		എങ്കിൽ (argv_expanded) {
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
			എങ്കിൽ (command->argv) {
			debug_printf_exec(": pipe member '%s' '%s'...\n",
					command->argv[0], command->argv[1]);
		} അല്ലെങ്കിൽ {
			debug_printf_exec(": pipe member with no argv\n");
		}

#ifndef __U_BOOT__
		/* pipes are inserted between pairs of commands */
		pipefds.rd = 0;
		pipefds.wr = 1;
		എങ്കിൽ (cmd_no < pi->അക്കം_cmds)
			xpiped_pair(pipefds);

#എങ്കിൽ ENABLE_HUSH_LINENO_VAR
		G.execute_lineno = command->lineno;
#endif

		command->pid = BB_MMU ? fork() : vfork();
		എങ്കിൽ (!command->pid) { /* child */
#എങ്കിൽ ENABLE_HUSH_JOB
			disable_restore_tty_pgrp_on_exit();
			CLEAR_RANDOM_T(&G.random_gen); /* or else $RANDOM repeats in child */

			/* Every child adds itself to new process group
			 * with pgid == pid_of_first_child_in_pipe */
			എങ്കിൽ (G.run_list_level == 1 && G_interactive_fd) {
				pid_t pgrp;
				pgrp = pi->pgrp;
				എങ്കിൽ (pgrp < 0) /* true for 1st process only */
					pgrp = getpid();
				എങ്കിൽ (setpgid(0, pgrp) == 0
				 && pi->followup != PIPE_BG
				 && G_saved_tty_pgrp /* we have ctty */
				) {
					/* We do it in *every* child, not just first,
					 * to avoid races */
					tcsetpgrp(G_interactive_fd, pgrp);
				}
			}
#endif
			എങ്കിൽ (pi->alive_cmds == 0 && pi->followup == PIPE_BG) {
				/* 1st cmd in backgrounded pipe
				 * should have its stdin /dev/null'ed */
				close(0);
				എങ്കിൽ (open(bb_dev_null, O_RDONLY))
					xopen("/", O_RDONLY);
			} അല്ലെങ്കിൽ {
				xmove_fd(next_infd, 0);
			}
			xmove_fd(pipefds.wr, 1);
			എങ്കിൽ (pipefds.rd > 1)
				close(pipefds.rd);
			/* Like bash, explicit redirects override pipes,
			 * and the pipe fd (fd#1) is available for dup'ing:
			 * "cmd1 2>&1 | cmd2": fd#1 is duped to fd#2, thus stderr
			 * of cmd1 goes into pipe.
			 */
			എങ്കിൽ (setup_redirects(command, ശൂന്യ)) {
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
#എങ്കിൽ ENABLE_HUSH_FAST
		G.എണ്ണം_SIGCHLD++;
//bb_error_msg("[%d] fork in run_pipe: G.count_SIGCHLD:%d G.handled_SIGCHLD:%d", getpid(), G.count_SIGCHLD, G.handled_SIGCHLD);
#endif
		enable_restore_tty_pgrp_on_exit();
#എങ്കിൽ !BB_MMU
		/* Clean up after vforked child */
		സ്വതന്ത്രം(nommu_save.argv);
		സ്വതന്ത്രം(nommu_save.argv_from_re_execing);
		G.var_nest_level = sv_var_nest_level;
		remove_nested_vars();
		add_vars(nommu_save.old_vars);
#endif
		സ്വതന്ത്രം(argv_expanded);
		argv_expanded = ശൂന്യ;
		എങ്കിൽ (command->pid < 0) { /* [v]fork failed */
			/* Clearly indicate, was it fork or vfork */
			bb_simple_perror_സന്ദേശം(BB_MMU ? "vfork"+1 : "vfork");
		} അല്ലെങ്കിൽ {
			pi->alive_cmds++;
#എങ്കിൽ ENABLE_HUSH_JOB
			/* Second and next children need to know pid of first one */
			എങ്കിൽ (pi->pgrp < 0)
				pi->pgrp = command->pid;
#endif
		}

		എങ്കിൽ (cmd_no > 1)
			close(next_infd);
		എങ്കിൽ (cmd_no < pi->അക്കം_cmds)
			close(pipefds.wr);
		/* Pass read (output) pipe end to next iteration */
		next_infd = pipefds.rd;
#അല്ലെങ്കിൽ /* __U_BOOT__ */
		/* Process the command */
		rcode = cmd_process(G.do_repeat ? CMD_FLAG_REPEAT : 0,
				    command->argc, command->argv,
				    &(G.flag_repeat), ശൂന്യ);

		എങ്കിൽ (argv_expanded) {
			/*
			 * expand_strvec_to_strvec() allocates memory to expand
			 * argv, we need to free it.
			 */
			സ്വതന്ത്രം(argv_expanded);

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
	എങ്കിൽ (!pi->alive_cmds) {
		debug_leave();
		debug_printf_exec("run_pipe return 1 (all forks failed, no children)\n");
		തിരികെ 1;
	}
#endif /* __U_BOOT__ */

	debug_leave();
#ifndef __U_BOOT__
	debug_printf_exec("run_pipe return -1 (%u children started)\n", pi->alive_cmds);
	തിരികെ -1;
#അല്ലെങ്കിൽ /* __U_BOOT__ */
	IF_HAS_KEYWORDS(എങ്കിൽ (pi->pi_inverted) rcode = !rcode;)
	debug_printf_exec("run_pipe return %d\n", rcode);
	തിരികെ rcode;
#endif /* __U_BOOT__ */
}

/* NB: called by pseudo_exec, and therefore must not modify any
 * global data until exec/_exit (we can be a child after vfork!) */
സ്ഥിതം പൂർണ്ണസംഖ്യ run_list(ഘടന pipe *pi)
{
#എങ്കിൽ ENABLE_HUSH_CASE
	അക്ഷരം *case_word = ശൂന്യ;
#endif
#എങ്കിൽ ENABLE_HUSH_LOOPS
	ഘടന pipe *loop_മുകൾ = ശൂന്യ;
	അക്ഷരം **for_lcur = ശൂന്യ;
	അക്ഷരം **for_list = ശൂന്യ;
#endif
	smallint last_followup;
	smalluint rcode;
#എങ്കിൽ ENABLE_HUSH_IF || ENABLE_HUSH_CASE
	smalluint cond_code = 0;
#അല്ലെങ്കിൽ
	എണ്ണൽ { cond_code = 0 };
#endif
#എങ്കിൽ HAS_KEYWORDS
	smallint rword;      /* RES_foo */
	smallint last_rword; /* ditto */
#endif

#ifndef __U_BOOT__
	debug_printf_exec("run_list lvl %d start\n", G.run_list_level);
	debug_enter();
#endif /* !__U_BOOT__ */

	set_G_ifs();

#എങ്കിൽ ENABLE_HUSH_LOOPS
	/* Check syntax for "for" */
	{
		ഘടന pipe *cpipe;
		വേണ്ടി (cpipe = pi; cpipe; cpipe = cpipe->next) {
			എങ്കിൽ (cpipe->res_word != RES_FOR && cpipe->res_word != RES_IN)
				തുടരുക;
			/* current word is FOR or IN (BOLD in comments below) */
			എങ്കിൽ (cpipe->next == ശൂന്യ) {
				syntax_error("malformed for");
				debug_leave();
				debug_printf_exec("run_list lvl %d return 1\n", G.run_list_level);
				തിരികെ 1;
			}
			/* "FOR v; do ..." and "for v IN a b; do..." are ok */
			എങ്കിൽ (cpipe->next->res_word == RES_DO)
				തുടരുക;
			/* next word is not "do". It must be "in" then ("FOR v in ...") */
			എങ്കിൽ (cpipe->res_word == RES_IN /* "for v IN a b; not_do..."? */
			 || cpipe->next->res_word != RES_IN /* FOR v not_do_and_not_in..."? */
			) {
				syntax_error("malformed for");
				debug_leave();
				debug_printf_exec("run_list lvl %d return 1\n", G.run_list_level);
				തിരികെ 1;
			}
		}
	}
#endif

	/* Past this point, all code paths should jump to ret: label
	 * in order to return, no direct "return" statements please.
	 * This helps to ensure that no memory is leaked. */

#എങ്കിൽ ENABLE_HUSH_JOB
	G.run_list_level++;
#endif

#എങ്കിൽ HAS_KEYWORDS
	rword = RES_NONE;
	last_rword = RES_XXXX;
#endif
	last_followup = PIPE_SEQ;
	rcode = G.last_exitcode;

	/* Go through list of pipes, (maybe) executing them. */
#ifndef __U_BOOT__
	വേണ്ടി (; pi; pi = IF_HUSH_LOOPS(rword == RES_DONE ? loop_മുകൾ : ) pi->next) {
#അല്ലെങ്കിൽ /* __U_BOOT__ */
	വേണ്ടി (; pi; pi = rword == RES_DONE ? loop_മുകൾ : pi->next) {
#endif /* __U_BOOT__ */
		പൂർണ്ണസംഖ്യ r;
		പൂർണ്ണസംഖ്യ sv_errexit_depth;

#ifndef __U_BOOT__
		എങ്കിൽ (G.flag_SIGINT)
			നിർത്തുക;
		എങ്കിൽ (G_flag_return_in_progress == 1)
			നിർത്തുക;
#endif /* !__U_BOOT__ */

		IF_HAS_KEYWORDS(rword = pi->res_word;)
		debug_printf_exec(": rword:%d cond_code:%d last_rword:%d\n",
				rword, cond_code, last_rword);

		sv_errexit_depth = G.errexit_depth;
		എങ്കിൽ (
#എങ്കിൽ ENABLE_HUSH_IF
		    rword == RES_IF || rword == RES_ELIF ||
#endif
		    pi->followup != PIPE_SEQ
		) {
			G.errexit_depth++;
		}
#എങ്കിൽ ENABLE_HUSH_LOOPS
		എങ്കിൽ ((rword == RES_WHILE || rword == RES_UNTIL || rword == RES_FOR)
		 && loop_മുകൾ == ശൂന്യ /* avoid bumping G.depth_of_loop twice */
		) {
			/* start of a loop: remember where loop starts */
			loop_മുകൾ = pi;
			G.depth_of_loop++;
		}
#endif
		/* Still in the same "if...", "then..." or "do..." branch? */
		എങ്കിൽ (IF_HAS_KEYWORDS(rword == last_rword &&) 1) {
			എങ്കിൽ ((rcode == 0 && last_followup == PIPE_OR)
			 || (rcode != 0 && last_followup == PIPE_AND)
			) {
				/* It is "<true> || CMD" or "<false> && CMD"
				 * and we should not execute CMD */
				debug_printf_exec("skipped cmd because of || or &&\n");
				last_followup = pi->followup;
				പോകുക dont_check_jobs_but_continue;
			}
		}
		last_followup = pi->followup;
#എങ്കിൽ ENABLE_HUSH_IF
		എങ്കിൽ (cond_code != 0) {
			എങ്കിൽ (rword == RES_THEN) {
				/* if false; then ... fi has exitcode 0! */
				G.last_exitcode = rcode = EXIT_SUCCESS;
				/* "if <false> THEN cmd": skip cmd */
				debug_printf_exec("skipped THEN cmd because IF condition was false\n");
				last_rword = rword;
				തുടരുക;
			}
		} അല്ലെങ്കിൽ {
			എങ്കിൽ (rword == RES_ELSE
			 || (rword == RES_ELIF && last_rword != RES_ELIF)
			) {
				/* "if <true> then ... ELSE/ELIF cmd":
				 * skip cmd and all following ones */
				debug_printf_exec("skipped ELSE/ELIF branch because IF condition was true\n");
				നിർത്തുക;
			}
			//if (rword == RES_THEN): "if <true> THEN cmd", run cmd (fall through)
		}
#endif
		IF_HAS_KEYWORDS(last_rword = rword;)
#എങ്കിൽ ENABLE_HUSH_LOOPS
		എങ്കിൽ (rword == RES_FOR) { /* && pi->num_cmds - always == 1 */
			എങ്കിൽ (!for_lcur) {
				/* first loop through for */

				സ്ഥിതം സ്ഥിരം അക്ഷരം encoded_dollar_at[] ALIGN1 = {
					SPECIAL_VAR_ചിഹ്നം, '@' | 0x80, SPECIAL_VAR_ചിഹ്നം, '\0'
				}; /* encoded representation of "$@" */
				സ്ഥിതം സ്ഥിരം അക്ഷരം *സ്ഥിരം encoded_dollar_at_argv[] ALIGN_PTR = {
					encoded_dollar_at, ശൂന്യ
				}; /* argv list with one element: "$@" */
				അക്ഷരം **vals;

				G.last_exitcode = rcode = EXIT_SUCCESS;
				vals = (അക്ഷരം**)encoded_dollar_at_argv;
				എങ്കിൽ (pi->next->res_word == RES_IN) {
					/* if no variable values after "in" we skip "for" */
					എങ്കിൽ (!pi->next->cmds[0].argv) {
						debug_printf_exec(": null FOR: exitcode EXIT_SUCCESS\n");
						നിർത്തുക;
					}
					vals = pi->next->cmds[0].argv;
				} /* else: "for var; do..." -> assume "$@" list */
				/* create list of variable values */
				debug_print_strings("for_list made from", vals);
				for_list = expand_strvec_to_strvec(vals);
				for_lcur = for_list;
				debug_print_strings("for_list", for_list);
			}
			എങ്കിൽ (!*for_lcur) {
				/* "for" loop is over, clean up */
				സ്വതന്ത്രം(for_list);
				for_list = ശൂന്യ;
				for_lcur = ശൂന്യ;
				നിർത്തുക;
			}
			/* Insert next value from for_lcur */
			/* note: *for_lcur already has quotes removed, $var expanded, etc */
#ifndef __U_BOOT__
			set_local_var_from_halves(pi->cmds[0].argv[0], *for_lcur++);
#അല്ലെങ്കിൽ /* __U_BOOT__ */
			/* We cannot use xasprintf, so we emulate it. */
			അക്ഷരം *full_var;
			അക്ഷരം *var = pi->cmds[0].argv[0];
			അക്ഷരം *val = *for_lcur++;

			/* + 1 to take into account =. */
			full_var = xmalloc(strlen(var) + strlen(val) + 1);
			sprintf(full_var, "%s=%s", var, val);

			set_local_var_modern(full_var, /*flag:*/ 0);
#endif /* __U_BOOT__ */
			തുടരുക;
		}
		എങ്കിൽ (rword == RES_IN) {
			തുടരുക; /* "for v IN list;..." - "in" has no cmds anyway */
		}
		എങ്കിൽ (rword == RES_DONE) {
			തുടരുക; /* "done" has no cmds too */
		}
#endif
#എങ്കിൽ ENABLE_HUSH_CASE
		എങ്കിൽ (rword == RES_CASE) {
			debug_printf_exec("CASE cond_code:%d\n", cond_code);
			case_word = expand_string_to_string(pi->cmds->argv[0],
				EXP_FLAG_ESC_GLOB_CHARS, /*unbackslash:*/ 1);
			debug_printf_exec("CASE word1:'%s'\n", case_word);
			//unbackslash(case_word);
			//debug_printf_exec("CASE word2:'%s'\n", case_word);
			തുടരുക;
		}
		എങ്കിൽ (rword == RES_MATCH) {
			അക്ഷരം **argv;

			debug_printf_exec("MATCH cond_code:%d\n", cond_code);
			എങ്കിൽ (!case_word) /* "case ... matched_word) ... WORD)": we executed selected branch, stop */
				നിർത്തുക;
			/* all prev words didn't match, does this one match? */
			argv = pi->cmds->argv;
			ആകുമ്പോൾ (*argv) {
				അക്ഷരം *pattern;
				debug_printf_exec("expand_string_to_string('%s')\n", *argv);
				pattern = expand_string_to_string(*argv,
						EXP_FLAG_ESC_GLOB_CHARS,
						/*unbackslash:*/ 0
				);
				/* TODO: which FNM_xxx flags to use? */
				cond_code = (fnmatch(pattern, case_word, /*flags:*/ 0) != 0);
				debug_printf_exec("cond_code=fnmatch(pattern:'%s',str:'%s'):%d\n",
						pattern, case_word, cond_code);
				സ്വതന്ത്രം(pattern);
				എങ്കിൽ (cond_code == 0) {
					/* match! we will execute this branch */
					സ്വതന്ത്രം(case_word);
					case_word = ശൂന്യ; /* make future "word)" stop */
					നിർത്തുക;
				}
				argv++;
			}
			തുടരുക;
		}
		എങ്കിൽ (rword == RES_CASE_BODY) { /* inside of a case branch */
			debug_printf_exec("CASE_BODY cond_code:%d\n", cond_code);
			എങ്കിൽ (cond_code != 0)
				തുടരുക; /* not matched yet, skip this pipe */
		}
		എങ്കിൽ (rword == RES_ESAC) {
			debug_printf_exec("ESAC cond_code:%d\n", cond_code);
			എങ്കിൽ (case_word) {
				/* "case" did not match anything: still set $? (to 0) */
				G.last_exitcode = rcode = EXIT_SUCCESS;
			}
		}
#endif
		/* Just pressing <enter> in shell should check for jobs.
		 * OTOH, in non-interactive shell this is useless
		 * and only leads to extra job checks */
		എങ്കിൽ (pi->അക്കം_cmds == 0) {
#ifndef __U_BOOT__
			എങ്കിൽ (G_interactive_fd)
				പോകുക check_jobs_and_continue;
#endif /* !__U_BOOT__ */
			തുടരുക;
		}

		/* After analyzing all keywords and conditions, we decided
		 * to execute this pipe. NB: have to do checkjobs(NULL)
		 * after run_pipe to collect any background children,
		 * even if list execution is to be stopped. */
		debug_printf_exec(": run_pipe with %d members\n", pi->അക്കം_cmds);
#ifndef __U_BOOT__
#എങ്കിൽ ENABLE_HUSH_LOOPS
		G.flag_break_continue = 0;
#endif
#endif /* !__U_BOOT__ */
#ifndef __U_BOOT__
		rcode = r = G.o_opt[OPT_O_NOEXEC] ? 0 : run_pipe(pi);
		/* NB: rcode is a smalluint, r is int */
#അല്ലെങ്കിൽ /* __U_BOOT__ */
		rcode = r = run_pipe(pi); /* NB: rcode is a smalluint, r is int */
		എങ്കിൽ (r <= EXIT_RET_CODE) {
			പൂർണ്ണസംഖ്യ previous_rcode = G.last_exitcode;
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
			എങ്കിൽ (previous_rcode != r) {
				/*
				 * If run from run_command, run_command_flags will be set, so we check
				 * this to know if we are in main input shell.
				 */
				എങ്കിൽ (!G.run_command_flags)
					printf("exit not allowed from main input shell.\n");

				നിർത്തുക;
			}
			തുടരുക;
		}
#endif /* __U_BOOT__ */
		എങ്കിൽ (r != -1) {
			/* We ran a builtin, function, or group.
			 * rcode is already known
			 * and we don't need to wait for anything. */
			debug_printf_exec(": builtin/func exitcode %d\n", rcode);
			G.last_exitcode = rcode;
#ifndef __U_BOOT__
			check_and_run_traps();
#endif /* !__U_BOOT__ */
#എങ്കിൽ ENABLE_HUSH_TRAP && ENABLE_HUSH_FUNCTIONS
			rcode = G.last_exitcode; /* "return" in trap can change it, read back */
#endif
#ifndef __U_BOOT__
#എങ്കിൽ ENABLE_HUSH_LOOPS
			/* Was it "break" or "continue"? */
			എങ്കിൽ (G.flag_break_continue) {
				smallint fbc = G.flag_break_continue;
				/* We might fall into outer *loop*,
				 * don't want to break it too */
				എങ്കിൽ (loop_മുകൾ) {
					G.depth_break_continue--;
					എങ്കിൽ (G.depth_break_continue == 0)
						G.flag_break_continue = 0;
					/* else: e.g. "continue 2" should *break* once, *then* continue */
				} /* else: "while... do... { we are here (innermost list is not a loop!) };...done" */
				എങ്കിൽ (G.depth_break_continue != 0 || fbc == BC_BREAK) {
					checkjobs(ശൂന്യ, 0 /*(no pid to wait for)*/);
					നിർത്തുക;
				}
				/* "continue": simulate end of loop */
				rword = RES_DONE;
				തുടരുക;
			}
#endif
			എങ്കിൽ (G_flag_return_in_progress == 1) {
				checkjobs(ശൂന്യ, 0 /*(no pid to wait for)*/);
				നിർത്തുക;
			}

		} അല്ലെങ്കിൽ എങ്കിൽ (pi->followup == PIPE_BG) {
			/* What does bash do with attempts to background builtins? */
			/* even bash 3.2 doesn't do that well with nested bg:
			 * try "{ { sleep 10; echo DEEP; } & echo HERE; } &".
			 * I'm NOT treating inner &'s as jobs */
#എങ്കിൽ ENABLE_HUSH_JOB
			എങ്കിൽ (G.run_list_level == 1)
				തിരുകുക_job_into_table(pi);
#endif
			/* Last command's pid goes to $! */
			G.last_bg_pid = pi->cmds[pi->അക്കം_cmds - 1].pid;
			G.last_bg_pid_exitcode = 0;
			debug_printf_exec(": cmd&: exitcode EXIT_SUCCESS\n");
/* Check pi->pi_inverted? "! sleep 1 & echo $?": bash says 1. dash and ash say 0 */
			rcode = EXIT_SUCCESS;
			പോകുക check_traps;
		} അല്ലെങ്കിൽ {
#എങ്കിൽ ENABLE_HUSH_JOB
			എങ്കിൽ (G.run_list_level == 1 && G_interactive_fd) {
				/* Waits for completion, then fg's main shell */
				rcode = checkjobs_and_fg_shell(pi);
				debug_printf_exec(": checkjobs_and_fg_shell exitcode %d\n", rcode);
				പോകുക check_traps;
			}
#endif
			/* This one just waits for completion */
			rcode = checkjobs(pi, 0 /*(no pid to wait for)*/);
			debug_printf_exec(": checkjobs exitcode %d\n", rcode);
 check_traps:
			G.last_exitcode = rcode;
			check_and_run_traps();
#എങ്കിൽ ENABLE_HUSH_TRAP && ENABLE_HUSH_FUNCTIONS
			rcode = G.last_exitcode; /* "return" in trap can change it, read back */
#endif
		}
#endif /* !__U_BOOT__ */

#ifndef __U_BOOT__
		/* Handle "set -e" */
		എങ്കിൽ (rcode != 0 && G.o_opt[OPT_O_ERREXIT]) {
			debug_printf_exec("ERREXIT:1 errexit_depth:%d\n", G.errexit_depth);
			എങ്കിൽ (G.errexit_depth == 0)
				hush_exit(rcode);
		}
#അല്ലെങ്കിൽ /* __U_BOOT__ */
		} /* if (r != -1) */
#endif /* __U_BOOT__ */
		G.errexit_depth = sv_errexit_depth;

		/* Analyze how result affects subsequent commands */
#എങ്കിൽ ENABLE_HUSH_IF
		എങ്കിൽ (rword == RES_IF || rword == RES_ELIF) {
			debug_printf_exec("cond_code=rcode:%d\n", rcode);
			cond_code = rcode;
		}
#endif
#ifndef __U_BOOT__
 check_jobs_and_continue:
		checkjobs(ശൂന്യ, 0 /*(no pid to wait for)*/);
#endif /* !__U_BOOT__ */
 dont_check_jobs_but_continue: ;
#എങ്കിൽ ENABLE_HUSH_LOOPS
		/* Beware of "while false; true; do ..."! */
		എങ്കിൽ (pi->next
		 && (pi->next->res_word == RES_DO || pi->next->res_word == RES_DONE)
		 /* check for RES_DONE is needed for "while ...; do \n done" case */
		) {
			എങ്കിൽ (rword == RES_WHILE) {
				എങ്കിൽ (rcode) {
					/* "while false; do...done" - exitcode 0 */
					G.last_exitcode = rcode = EXIT_SUCCESS;
					debug_printf_exec(": while expr is false: breaking (exitcode:EXIT_SUCCESS)\n");
					നിർത്തുക;
				}
			}
			എങ്കിൽ (rword == RES_UNTIL) {
				എങ്കിൽ (!rcode) {
					debug_printf_exec(": until expr is true: breaking\n");
					നിർത്തുക;
				}
			}
		}
#endif
	} /* for (pi) */

#എങ്കിൽ ENABLE_HUSH_JOB
	G.run_list_level--;
#endif
#എങ്കിൽ ENABLE_HUSH_LOOPS
	എങ്കിൽ (loop_മുകൾ)
		G.depth_of_loop--;
	സ്വതന്ത്രം(for_list);
#endif
#എങ്കിൽ ENABLE_HUSH_CASE
	സ്വതന്ത്രം(case_word);
#endif
#ifndef __U_BOOT__
	debug_leave();
	debug_printf_exec("run_list lvl %d return %d\n", G.run_list_level, rcode);
#endif /* !__U_BOOT__ */
	തിരികെ rcode;
}

/* Select which version we will use */
സ്ഥിതം പൂർണ്ണസംഖ്യ run_and_free_list(ഘടന pipe *pi)
{
	പൂർണ്ണസംഖ്യ rcode = 0;
	debug_printf_exec("run_and_free_list entered\n");
#ifndef __U_BOOT__
	എങ്കിൽ (!G.o_opt[OPT_O_NOEXEC]) {
#endif /* !__U_BOOT__ */
		debug_printf_exec(": run_list: 1st pipe with %d cmds\n", pi->അക്കം_cmds);
		rcode = run_list(pi);
#ifndef __U_BOOT__
	}
#endif /* !__U_BOOT__ */
	/* free_pipe_list has the side effect of clearing memory.
	 * In the long run that function can be merged with run_list,
	 * but doing that now would hobble the debugging effort. */
	free_pipe_list(pi);
	debug_printf_exec("run_and_free_list return %d\n", rcode);
	തിരികെ rcode;
}

#ifndef __U_BOOT__
സ്ഥിതം ശൂന്യം install_sighandlers(unsigned mask)
{
	sighandler_t old_handler;
	unsigned sig = 0;
	ആകുമ്പോൾ ((mask >>= 1) != 0) {
		sig++;
		എങ്കിൽ (!(mask & 1))
			തുടരുക;
		old_handler = install_sighandler(sig, pick_sighandler(sig));
		/* POSIX allows shell to re-enable SIGCHLD
		 * even if it was SIG_IGN on entry.
		 * Therefore we skip IGN check for it:
		 */
		എങ്കിൽ (sig == SIGCHLD)
			തുടരുക;
		/* Interactive bash re-enables SIGHUP which is SIG_IGNed on entry.
		 * Try:
		 * trap '' hup; bash; echo RET  # type "kill -hup $$", see SIGHUP having effect
		 * trap '' hup; bash -c 'kill -hup $$; echo ALIVE'  # here SIGHUP is SIG_IGNed
		 */
		എങ്കിൽ (sig == SIGHUP && G_interactive_fd)
			തുടരുക;
		/* Unless one of the above signals, is it SIG_IGN? */
		എങ്കിൽ (old_handler == SIG_IGN) {
			/* oops... restore back to IGN, and record this fact */
			install_sighandler(sig, old_handler);
#എങ്കിൽ ENABLE_HUSH_TRAP
			എങ്കിൽ (!G_traps)
				G_traps = xzalloc(വലുപ്പം(G_traps[0]) * NSIG);
			സ്വതന്ത്രം(G_traps[sig]);
			G_traps[sig] = xzalloc(1); /* == xstrdup(""); */
#endif
		}
	}
}

/* Called a few times only (or even once if "sh -c") */
സ്ഥിതം ശൂന്യം install_special_sighandlers(ശൂന്യം)
{
	unsigned mask;

	/* Which signals are shell-special? */
	mask = (1 << SIGQUIT) | (1 << SIGCHLD);
	എങ്കിൽ (G_interactive_fd) {
		mask |= SPECIAL_INTERACTIVE_SIGS;
		എങ്കിൽ (G_saved_tty_pgrp) /* we have ctty, job control sigs work */
			mask |= SPECIAL_JOBSTOP_SIGS;
	}
	/* Careful, do not re-install handlers we already installed */
	എങ്കിൽ (G.special_sig_mask != mask) {
		unsigned diff = mask & ~G.special_sig_mask;
		G.special_sig_mask = mask;
		install_sighandlers(diff);
	}
}

#എങ്കിൽ ENABLE_HUSH_JOB
/* helper */
/* Set handlers to restore tty pgrp and exit */
സ്ഥിതം ശൂന്യം install_fatal_sighandlers(ശൂന്യം)
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

സ്ഥിതം പൂർണ്ണസംഖ്യ set_mode(പൂർണ്ണസംഖ്യ state, അക്ഷരം mode, സ്ഥിരം അക്ഷരം *o_opt)
{
	പൂർണ്ണസംഖ്യ idx;
	തിരഞ്ഞെടുപ്പ് (mode) {
	അവസ്ഥ 'n':
		/* set -n has no effect in interactive shell */
		/* Try: while set -n; do echo $-; done */
		എങ്കിൽ (!G_interactive_fd)
			G.o_opt[OPT_O_NOEXEC] = state;
		നിർത്തുക;
	അവസ്ഥ 'x':
		IF_HUSH_MODE_X(G_x_mode = state;)
		IF_HUSH_MODE_X(എങ്കിൽ (G.x_mode_fd <= 0) G.x_mode_fd = dup_CLOEXEC(2, 10);)
		നിർത്തുക;
	അവസ്ഥ 'e':
		G.o_opt[OPT_O_ERREXIT] = state;
		നിർത്തുക;
	അവസ്ഥ 'o':
		എങ്കിൽ (!o_opt) {
			/* "set -o" or "set +o" without parameter.
			 * in bash, set -o produces this output:
			 *  pipefail        off
			 * and set +o:
			 *  set +o pipefail
			 * We always use the second form.
			 */
			സ്ഥിരം അക്ഷരം *p = o_opt_strings;
			idx = 0;
			ആകുമ്പോൾ (*p) {
				printf("set %co %s\n", (G.o_opt[idx] ? '-' : '+'), p);
				idx++;
				p += strlen(p) + 1;
			}
			നിർത്തുക;
		}
		idx = index_in_strings(o_opt_strings, o_opt);
		എങ്കിൽ (idx >= 0) {
			G.o_opt[idx] = state;
			നിർത്തുക;
		}
		/* fall through to error */
	സ്ഥിരസ്ഥിതി:
		തിരികെ EXIT_FAILURE;
	}
	തിരികെ EXIT_SUCCESS;
}

പൂർണ്ണസംഖ്യ hush_main(പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv) MAIN_EXTERNALLY_VISIBLE;
പൂർണ്ണസംഖ്യ hush_main(പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv)
{
	pid_t cached_getpid;
	എണ്ണൽ {
		OPT_login = (1 << 0),
	};
	unsigned flags;
#എങ്കിൽ !BB_MMU
	unsigned builtin_argc = 0;
#endif
	അക്ഷരം **e;
	ഘടന variable *cur_var;
	ഘടന variable *shell_ver;

	INIT_G();
	എങ്കിൽ (EXIT_SUCCESS != 0) /* if EXIT_SUCCESS == 0, it is already done */
		G.last_exitcode = EXIT_SUCCESS;
#എങ്കിൽ !BB_MMU
	/* "Big heredoc" support via "sh -< STRING" invocation.
	 * Check it first (do not bother to run the usual init code,
	 * it is not needed for this case).
	 */
	എങ്കിൽ (argv[1]
	 && argv[1][0] == '-' && argv[1][1] == '<' /*&& !argv[1][2]*/
	 /*&& argv[2] && !argv[3] - we don't check some conditions */
	) {
		full_write1_str(argv[2]);
		_exit(0);
	}
	G.argv0_for_re_execing = argv[0];
#endif
#എങ്കിൽ ENABLE_HUSH_TRAP
# എങ്കിൽ ENABLE_HUSH_FUNCTIONS
	G.return_exitcode = -1;
# endif
	G.pre_trap_exitcode = -1;
#endif

#എങ്കിൽ ENABLE_HUSH_FAST
	G.എണ്ണം_SIGCHLD++; /* ensure it is != G.handled_SIGCHLD */
#endif

	cached_getpid = getpid();   /* for tcsetpgrp() during init */
	G.വേര്_pid = cached_getpid; /* for $PID  (NOMMU can override via -$HEXPID:HEXPPID:...) */
	G.വേര്_ppid = getppid();    /* for $PPID (NOMMU can override) */

	/* Deal with HUSH_VERSION */
	debug_printf_env("unsetenv '%s'\n", "HUSH_VERSION");
	unsetenv("HUSH_VERSION"); /* in case it exists in initial env */
	shell_ver = xzalloc(വലുപ്പം(*shell_ver));
	shell_ver->flg_export = 1;
	shell_ver->flg_read_only = 1;
	/* Code which handles ${var<op>...} needs writable values for all variables,
	 * therefore we xstrdup: */
	shell_ver->varstr = xstrdup(hush_version_str);

	/* Create shell local variables from the values
	 * currently living in the environment */
	G.മുകൾ_var = shell_ver;
	cur_var = G.മുകൾ_var;
	e = environ;
	എങ്കിൽ (e) ആകുമ്പോൾ (*e) {
		അക്ഷരം *മൂല്യം = strchr(*e, '=');
		എങ്കിൽ (മൂല്യം) { /* paranoia */
			cur_var->next = xzalloc(വലുപ്പം(*cur_var));
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

#എങ്കിൽ BASH_HOSTNAME_VAR
	/* Set (but not export) HOSTNAME unless already set */
	എങ്കിൽ (!get_local_var_മൂല്യം("HOSTNAME")) {
		ഘടന utsname uts;
		uname(&uts);
		set_local_var_from_halves("HOSTNAME", uts.nodename);
	}
#endif
	/* IFS is not inherited from the parent environment */
	set_local_var_from_halves("IFS", defifs);

	എങ്കിൽ (!get_local_var_മൂല്യം("PATH"))
		set_local_var_from_halves("PATH", bb_default_വേര്_path);

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

#എങ്കിൽ അക്കം_SCRIPTS > 0
	എങ്കിൽ (argc < 0) {
		അക്ഷരം *script = get_script_content(-argc - 1);
		G.global_argv = argv;
		G.global_argc = string_array_len(argv);
		//install_special_sighandlers(); - needed?
		parse_and_run_string(script);
		പോകുക final_return;
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
	ആകുമ്പോൾ (1) {
		പൂർണ്ണസംഖ്യ opt = getopt(argc, argv, "+" /* stop at 1st non-option */
				"cexinsl"
#എങ്കിൽ !BB_MMU
				"$:R:V:"
# എങ്കിൽ ENABLE_HUSH_LINENO_VAR
				"L:"
# endif
# എങ്കിൽ ENABLE_HUSH_FUNCTIONS
				"F:"
# endif
#endif
		);
		എങ്കിൽ (opt <= 0)
			നിർത്തുക;
		തിരഞ്ഞെടുപ്പ് (opt) {
		അവസ്ഥ 'c':
			/* Note: -c is not an option with param!
			 * "hush -c -l SCRIPT" is valid. "hush -cSCRIPT" is not.
			 */
			G.opt_c = 1;
			നിർത്തുക;
		അവസ്ഥ 'i':
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
			നിർത്തുക;
		അവസ്ഥ 's':
			G.opt_s = 1;
			നിർത്തുക;
		അവസ്ഥ 'l':
			flags |= OPT_login;
			നിർത്തുക;
#എങ്കിൽ !BB_MMU
		അവസ്ഥ '$': {
			unsigned long long empty_trap_mask;

			G.വേര്_pid = bb_strtou(optarg, &optarg, 16);
			optarg++;
			G.വേര്_ppid = bb_strtou(optarg, &optarg, 16);
			optarg++;
			G.last_bg_pid = bb_strtou(optarg, &optarg, 16);
			optarg++;
			G.last_exitcode = bb_strtou(optarg, &optarg, 16);
			optarg++;
			builtin_argc = bb_strtou(optarg, &optarg, 16);
			optarg++;
			empty_trap_mask = bb_strtoull(optarg, &optarg, 16);
			എങ്കിൽ (empty_trap_mask != 0) {
				IF_HUSH_TRAP(പൂർണ്ണസംഖ്യ sig;)
				install_special_sighandlers();
# എങ്കിൽ ENABLE_HUSH_TRAP
				G_traps = xzalloc(വലുപ്പം(G_traps[0]) * NSIG);
				വേണ്ടി (sig = 1; sig < NSIG; sig++) {
					എങ്കിൽ (empty_trap_mask & (1LL << sig)) {
						G_traps[sig] = xzalloc(1); /* == xstrdup(""); */
						install_sighandler(sig, SIG_IGN);
					}
				}
# endif
			}
# എങ്കിൽ ENABLE_HUSH_LOOPS
			optarg++;
			G.depth_of_loop = bb_strtou(optarg, &optarg, 16);
# endif
			/* Suppress "killed by signal" message, -$ hack is used
			 * for subshells: echo `sh -c 'kill -9 $$'`
			 * should be silent.
			 */
			IF_HUSH_JOB(G.run_list_level = 1;)
# എങ്കിൽ ENABLE_HUSH_FUNCTIONS
			/* nommu uses re-exec trick for "... | func | ...",
			 * should allow "return".
			 * This accidentally allows returns in subshells.
			 */
			G_flag_return_in_progress = -1;
# endif
			നിർത്തുക;
		}
		അവസ്ഥ 'R':
		അവസ്ഥ 'V':
			set_local_var(xstrdup(optarg), opt == 'R' ? SETFLAG_MAKE_RO : 0);
			നിർത്തുക;
# എങ്കിൽ ENABLE_HUSH_LINENO_VAR
		അവസ്ഥ 'L':
			G.parse_lineno = xatou(optarg);
			നിർത്തുക;
# endif
# എങ്കിൽ ENABLE_HUSH_FUNCTIONS
		അവസ്ഥ 'F': {
			ഘടന function *funcp = new_function(optarg);
			/* funcp->name is already set to optarg */
			/* funcp->body is set to NULL. It's a special case. */
			funcp->body_as_string = argv[optind];
			optind++;
			നിർത്തുക;
		}
# endif
#endif
		/*case '?': invalid option encountered (set_mode('?') will fail) */
		/*case 'n':*/
		/*case 'x':*/
		/*case 'e':*/
		സ്ഥിരസ്ഥിതി:
			എങ്കിൽ (set_mode(1, opt, ശൂന്യ) == 0) /* no error */
				നിർത്തുക;
			bb_show_usage();
		}
	} /* option parsing loop */

	/* Skip options. Try "hush -l": $1 should not be "-l"! */
	G.global_argc = argc - (optind - 1);
	G.global_argv = argv + (optind - 1);
	G.global_argv[0] = argv[0];

	/* If we are login shell... */
	എങ്കിൽ (flags & OPT_login) {
		സ്ഥിരം അക്ഷരം *hp = ശൂന്യ;
		HFILE *നിവേശം;

		debug_printf("sourcing /etc/profile\n");
		നിവേശം = hfopen("/etc/profile");
 run_profile:
		എങ്കിൽ (നിവേശം != ശൂന്യ) {
			install_special_sighandlers();
			parse_and_run_file(നിവേശം);
			hfclose(നിവേശം);
		}
		/* bash: after sourcing /etc/profile,
		 * tries to source (in the given order):
		 * ~/.bash_profile, ~/.bash_login, ~/.profile,
		 * stopping on first found. --noprofile turns this off.
		 * bash also sources ~/.bash_logout on exit.
		 * If called as sh, skips .bash_XXX files.
		 */
		എങ്കിൽ (!hp) { /* unless we looped on the "goto" already */
			hp = get_local_var_മൂല്യം("HOME");
			എങ്കിൽ (hp && hp[0]) {
				debug_printf("sourcing ~/.profile\n");
				hp = concat_path_file(hp, ".profile");
				നിവേശം = hfopen(hp);
				സ്വതന്ത്രം((അക്ഷരം*)hp);
				പോകുക run_profile;
			}
		}
	}

#ifndef __U_BOOT__
	/* -c takes effect *after* -l */
	എങ്കിൽ (G.opt_c) {
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
		അക്ഷരം *script;

		install_special_sighandlers();

		G.global_argc--;
		G.global_argv++;
#എങ്കിൽ !BB_MMU
		എങ്കിൽ (builtin_argc) {
			/* -c 'builtin' [BARGV...] "" ARG0 [ARG1...] */
			സ്ഥിരം ഘടന built_in_command *x;
			x = find_builtin(G.global_argv[0]);
			എങ്കിൽ (x) { /* paranoia */
				argv = G.global_argv;
				G.global_argc -= builtin_argc + 1; /* skip [BARGV...] "" */
				G.global_argv += builtin_argc + 1;
				G.global_argv[-1] = ശൂന്യ; /* replace "" */
				G.last_exitcode = x->b_function(argv);
			}
			പോകുക final_return;
		}
#endif

		script = G.global_argv[0];
		എങ്കിൽ (!script)
			bb_error_സന്ദേശം_and_die(bb_സന്ദേശം_requires_arg, "-c");
		എങ്കിൽ (!G.global_argv[1]) {
			/* -c 'script' (no params): prevent empty $0 */
			G.global_argv[0] = argv[0];
		} അല്ലെങ്കിൽ { /* else -c 'script' ARG0 [ARG1...]: $0 is ARG0 */
			G.global_argc--;
			G.global_argv++;
		}
		parse_and_run_string(script);
		പോകുക final_return;
	}

	/* -s is: hush -s ARGV1 ARGV2 (no SCRIPT) */
	എങ്കിൽ (!G.opt_s && G.global_argv[1]) {
		HFILE *നിവേശം;
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
		നിവേശം = hfopen(G.global_argv[0]);
		എങ്കിൽ (!നിവേശം) {
			bb_simple_perror_സന്ദേശം_and_die(G.global_argv[0]);
		}
		xfunc_error_retval = 1;
		install_special_sighandlers();
		parse_and_run_file(നിവേശം);
#എങ്കിൽ ENABLE_FEATURE_CLEAN_UP
		hfclose(നിവേശം);
#endif
		പോകുക final_return;
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
#എങ്കിൽ ENABLE_HUSH_JOB
	എങ്കിൽ (isatty(STDIN_FILENO) && isatty(STDOUT_FILENO)) {
		G_saved_tty_pgrp = tcgetpgrp(STDIN_FILENO);
		debug_printf("saved_tty_pgrp:%d\n", G_saved_tty_pgrp);
		എങ്കിൽ (G_saved_tty_pgrp < 0)
			G_saved_tty_pgrp = 0;

		/* try to dup stdin to high fd#, >= 255 */
		G_interactive_fd = dup_CLOEXEC(STDIN_FILENO, 254);
		എങ്കിൽ (G_interactive_fd < 0) {
			/* try to dup to any fd */
			G_interactive_fd = dup_CLOEXEC(STDIN_FILENO, -1);
			എങ്കിൽ (G_interactive_fd < 0) {
				/* give up */
				G_interactive_fd = 0;
				G_saved_tty_pgrp = 0;
			}
		}
	}
	debug_printf("interactive_fd:%d\n", G_interactive_fd);
	എങ്കിൽ (G_interactive_fd) {
		എങ്കിൽ (G_saved_tty_pgrp) {
			/* If we were run as 'hush &', sleep until we are
			 * in the foreground (tty pgrp == our pgrp).
			 * If we get started under a job aware app (like bash),
			 * make sure we are now in charge so we don't fight over
			 * who gets the foreground */
			ആകുമ്പോൾ (1) {
				pid_t shell_pgrp = getpgrp();
				G_saved_tty_pgrp = tcgetpgrp(G_interactive_fd);
				എങ്കിൽ (G_saved_tty_pgrp == shell_pgrp)
					നിർത്തുക;
				/* send TTIN to ourself (should stop us) */
				kill(- shell_pgrp, SIGTTIN);
			}
		}

		/* Install more signal handlers */
		install_special_sighandlers();

		എങ്കിൽ (G_saved_tty_pgrp) {
			/* Set other signals to restore saved_tty_pgrp */
			install_fatal_sighandlers();
			/* Put ourselves in our own process group
			 * (bash, too, does this only if ctty is available) */
			bb_setpgrp(); /* is the same as setpgid(our_pid, our_pid); */
			/* Grab control of the terminal */
			tcsetpgrp(G_interactive_fd, cached_getpid);
		}
		enable_restore_tty_pgrp_on_exit();

# എങ്കിൽ ENABLE_FEATURE_EDITING
		G.വരി_നിവേശം_state = new_വരി_നിവേശം_t(FOR_SHELL);
#  എങ്കിൽ ENABLE_FEATURE_TAB_COMPLETION
		G.വരി_നിവേശം_state->get_exe_name = hush_command_name;
#  endif
#  എങ്കിൽ EDITING_HAS_sh_get_var
		G.വരി_നിവേശം_state->sh_get_var = get_local_var_മൂല്യം;
#  endif
# endif
# എങ്കിൽ ENABLE_HUSH_SAVEHISTORY && MAX_HISTORY > 0
		{
			സ്ഥിരം അക്ഷരം *hp = get_local_var_മൂല്യം("HISTFILE");
			എങ്കിൽ (!hp) {
				hp = get_local_var_മൂല്യം("HOME");
				എങ്കിൽ (hp)
					hp = concat_path_file(hp, ".hush_history");
			} അല്ലെങ്കിൽ {
				hp = xstrdup(hp);
			}
			എങ്കിൽ (hp) {
				G.വരി_നിവേശം_state->hist_file = hp;
				//set_local_var(xasprintf("HISTFILE=%s", ...));
			}
#  എങ്കിൽ ENABLE_FEATURE_SH_HISTFILESIZE
			hp = get_local_var_മൂല്യം("HISTFILESIZE");
			G.വരി_നിവേശം_state->max_history = size_from_HISTFILESIZE(hp);
#  endif
		}
# endif
	} അല്ലെങ്കിൽ {
		install_special_sighandlers();
	}
#അല്ലെങ്കിലെങ്കിൽ ENABLE_HUSH_INTERACTIVE
	/* No job control compiled in, only prompt/line editing */
	എങ്കിൽ (isatty(STDIN_FILENO) && isatty(STDOUT_FILENO)) {
		G_interactive_fd = dup_CLOEXEC(STDIN_FILENO, 254);
		എങ്കിൽ (G_interactive_fd < 0) {
			/* try to dup to any fd */
			G_interactive_fd = dup_CLOEXEC(STDIN_FILENO, -1);
			എങ്കിൽ (G_interactive_fd < 0)
				/* give up */
				G_interactive_fd = 0;
		}
	}
	install_special_sighandlers();
#അല്ലെങ്കിൽ
	/* We have interactiveness code disabled */
	install_special_sighandlers();
#endif
	/* bash:
	 * if interactive but not a login shell, sources ~/.bashrc
	 * (--norc turns this off, --rcfile <file> overrides)
	 */

	എങ്കിൽ (G_interactive_fd) {
#എങ്കിൽ ENABLE_HUSH_INTERACTIVE && ENABLE_FEATURE_EDITING_FANCY_PROMPT
		/* Set (but not export) PS1/2 unless already set */
		എങ്കിൽ (!get_local_var_മൂല്യം("PS1"))
			set_local_var_from_halves("PS1", "\\w \\$ ");
		എങ്കിൽ (!get_local_var_മൂല്യം("PS2"))
			set_local_var_from_halves("PS2", "> ");
#endif
		എങ്കിൽ (!ENABLE_FEATURE_SH_EXTRA_QUIET) {
			/* note: ash and hush share this string */
			printf("\n\n%s %s\n"
				IF_HUSH_HELP("Enter 'help' for a list of built-in commands.\n")
				"\n",
				bb_banner,
				"hush - the humble shell"
			);
		}
	}

	parse_and_run_file(hfopen(ശൂന്യ)); /* stdin */

 final_return:
	hush_exit(G.last_exitcode);
}

/*
 * Built-ins
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ FAST_FUNC builtin_true(അക്ഷരം **argv UNUSED_PARAM)
{
	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ FAST_FUNC builtin_false(അക്ഷരം **argv UNUSED_PARAM)
{
	തിരികെ 1;
}

#എങ്കിൽ ENABLE_HUSH_TEST || ENABLE_HUSH_ECHO || ENABLE_HUSH_PRINTF || ENABLE_HUSH_KILL
സ്ഥിതം NOINLINE പൂർണ്ണസംഖ്യ run_applet_main(അക്ഷരം **argv, പൂർണ്ണസംഖ്യ (*applet_main_func)(പൂർണ്ണസംഖ്യ argc, അക്ഷരം **argv))
{
	പൂർണ്ണസംഖ്യ argc = string_array_len(argv);
	തിരികെ applet_main_func(argc, argv);
}
#endif
#എങ്കിൽ ENABLE_HUSH_TEST || BASH_TEST2
സ്ഥിതം പൂർണ്ണസംഖ്യ FAST_FUNC builtin_test(അക്ഷരം **argv)
{
	തിരികെ run_applet_main(argv, test_main);
}
#endif
#എങ്കിൽ ENABLE_HUSH_ECHO
സ്ഥിതം പൂർണ്ണസംഖ്യ FAST_FUNC builtin_echo(അക്ഷരം **argv)
{
	തിരികെ run_applet_main(argv, echo_main);
}
#endif
#എങ്കിൽ ENABLE_HUSH_PRINTF
സ്ഥിതം പൂർണ്ണസംഖ്യ FAST_FUNC builtin_printf(അക്ഷരം **argv)
{
	തിരികെ run_applet_main(argv, printf_main);
}
#endif

#എങ്കിൽ ENABLE_HUSH_HELP
സ്ഥിതം പൂർണ്ണസംഖ്യ FAST_FUNC builtin_help(അക്ഷരം **argv UNUSED_PARAM)
{
	സ്ഥിരം ഘടന built_in_command *x;

	printf(
		"Built-in commands:\n"
		"------------------\n");
	വേണ്ടി (x = bltins1; x != &bltins1[ARRAY_SIZE(bltins1)]; x++) {
		എങ്കിൽ (x->b_descr)
			printf("%-10s%s\n", x->b_cmd, x->b_descr);
	}
	തിരികെ EXIT_SUCCESS;
}
#endif

#എങ്കിൽ MAX_HISTORY && ENABLE_FEATURE_EDITING
സ്ഥിതം പൂർണ്ണസംഖ്യ FAST_FUNC builtin_history(അക്ഷരം **argv UNUSED_PARAM)
{
	show_history(G.വരി_നിവേശം_state);
	തിരികെ EXIT_SUCCESS;
}
#endif

സ്ഥിതം പൂർണ്ണസംഖ്യ FAST_FUNC builtin_cd(അക്ഷരം **argv)
{
	സ്ഥിരം അക്ഷരം *newdir;

	argv = skip_dash_dash(argv);
	newdir = argv[0];
	എങ്കിൽ (newdir == ശൂന്യ) {
		/* bash does nothing (exitcode 0) if HOME is ""; if it's unset,
		 * bash says "bash: cd: HOME not set" and does nothing
		 * (exitcode 1)
		 */
		സ്ഥിരം അക്ഷരം *home = get_local_var_മൂല്യം("HOME");
		newdir = home ? home : "/";
	}
	എങ്കിൽ (chdir(newdir)) {
		/* Mimic bash message exactly */
		bb_perror_സന്ദേശം("cd: %s", newdir);
		തിരികെ EXIT_FAILURE;
	}
	/* Read current dir (get_cwd(1) is inside) and set PWD.
	 * Note: do not enforce exporting. If PWD was unset or unexported,
	 * set it again, but do not export. bash does the same.
	 */
	set_pwd_var(/*flag:*/ 0);
	തിരികെ EXIT_SUCCESS;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ FAST_FUNC builtin_pwd(അക്ഷരം **argv UNUSED_PARAM)
{
	puts(get_cwd(0));
	തിരികെ EXIT_SUCCESS;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ FAST_FUNC builtin_eval(അക്ഷരം **argv)
{
	argv = skip_dash_dash(argv);

	എങ്കിൽ (!argv[0])
		തിരികെ EXIT_SUCCESS;

	IF_HUSH_MODE_X(G.x_mode_depth++;)
	//bb_error_msg("%s: ++x_mode_depth=%d", __func__, G.x_mode_depth);
	എങ്കിൽ (!argv[1]) {
		/* bash:
		 * eval "echo Hi; done" ("done" is syntax error):
		 * "echo Hi" will not execute too.
		 */
		parse_and_run_string(argv[0]);
	} അല്ലെങ്കിൽ {
		/* "The eval utility shall construct a command by
		 * concatenating arguments together, separating
		 * each with a <space> character."
		 */
		അക്ഷരം *str, *p;
		unsigned len = 0;
		അക്ഷരം **pp = argv;
		ചെയ്യുക
			len += strlen(*pp) + 1;
		ആകുമ്പോൾ (*++pp);
		str = p = xmalloc(len);
		pp = argv;
		വേണ്ടി (;;) {
			p = stpcpy(p, *pp);
			pp++;
			എങ്കിൽ (!*pp)
				നിർത്തുക;
			*p++ = ' ';
		}
		parse_and_run_string(str);
		സ്വതന്ത്രം(str);
	}
	IF_HUSH_MODE_X(G.x_mode_depth--;)
	//bb_error_msg("%s: --x_mode_depth=%d", __func__, G.x_mode_depth);
	തിരികെ G.last_exitcode;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ FAST_FUNC builtin_exec(അക്ഷരം **argv)
{
	argv = skip_dash_dash(argv);
	എങ്കിൽ (argv[0] == ശൂന്യ)
		തിരികെ EXIT_SUCCESS; /* bash does this */

	/* Careful: we can end up here after [v]fork. Do not restore
	 * tty pgrp then, only top-level shell process does that */
	എങ്കിൽ (G_saved_tty_pgrp && getpid() == G.വേര്_pid)
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

സ്ഥിതം പൂർണ്ണസംഖ്യ FAST_FUNC builtin_exit(അക്ഷരം **argv)
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
	എങ്കിൽ (argv[0] == ശൂന്യ) {
#എങ്കിൽ ENABLE_HUSH_TRAP
		എങ്കിൽ (G.pre_trap_exitcode >= 0) /* "exit" in trap uses $? from before the trap */
			hush_exit(G.pre_trap_exitcode);
#endif
		hush_exit(G.last_exitcode);
	}
	/* mimic bash: exit 123abc == exit 255 + error msg */
	xfunc_error_retval = 255;
	/* bash: exit -2 == exit 254, no error msg */
	hush_exit(xatoi(argv[0]) & 0xff);
}

#എങ്കിൽ ENABLE_HUSH_TYPE
/* http://www.opengroup.org/onlinepubs/9699919799/utilities/type.html */
സ്ഥിതം പൂർണ്ണസംഖ്യ FAST_FUNC builtin_type(അക്ഷരം **argv)
{
	പൂർണ്ണസംഖ്യ ret = EXIT_SUCCESS;

	ആകുമ്പോൾ (*++argv) {
		സ്ഥിരം അക്ഷരം *type;
		അക്ഷരം *path = ശൂന്യ;

		എങ്കിൽ (0) {} /* make conditional compile easier below */
		/*else if (find_alias(*argv))
			type = "an alias";*/
# എങ്കിൽ ENABLE_HUSH_FUNCTIONS
		അല്ലെങ്കിൽ എങ്കിൽ (find_function(*argv))
			type = "a function";
# endif
		അല്ലെങ്കിൽ എങ്കിൽ (find_builtin(*argv))
			type = "a shell builtin";
		അല്ലെങ്കിൽ എങ്കിൽ ((path = find_in_path(*argv)) != ശൂന്യ)
			type = path;
		അല്ലെങ്കിൽ {
			bb_error_സന്ദേശം("type: %s: not found", *argv);
			ret = EXIT_FAILURE;
			തുടരുക;
		}

		printf("%s is %s\n", *argv, type);
		സ്വതന്ത്രം(path);
	}

	തിരികെ ret;
}
#endif

#എങ്കിൽ ENABLE_HUSH_READ
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
സ്ഥിതം പൂർണ്ണസംഖ്യ FAST_FUNC builtin_read(അക്ഷരം **argv)
{
	സ്ഥിരം അക്ഷരം *r;
	ഘടന builtin_read_params params;

	memset(&params, 0, വലുപ്പം(params));

	/* "!": do not abort on errors.
	 * Option string must start with "sr" to match BUILTIN_READ_xxx
	 */
	params.read_flags = getopt32(argv,
# എങ്കിൽ BASH_READ_D
		IF_NOT_HUSH_BASH_COMPAT("^")
		"!srn:p:t:u:d:" IF_NOT_HUSH_BASH_COMPAT("\0" "-1"/*min 1 arg*/),
		&params.opt_n, &params.opt_p, &params.opt_t, &params.opt_u, &params.opt_d
# അല്ലെങ്കിൽ
		IF_NOT_HUSH_BASH_COMPAT("^")
		"!srn:p:t:u:" IF_NOT_HUSH_BASH_COMPAT("\0" "-1"/*min 1 arg*/),
		&params.opt_n, &params.opt_p, &params.opt_t, &params.opt_u
# endif
//TODO: print "read: need variable name"
//for the case of !BASH "read" with no args (now it fails silently)
//(or maybe extend getopt32() to emit a message if "-1" fails)
	);
	എങ്കിൽ ((uint32_t)params.read_flags == (uint32_t)-1)
		തിരികെ EXIT_FAILURE;
	argv += optind;
	params.argv = argv;
	params.setvar = set_local_var_from_halves;
	params.ifs = get_local_var_മൂല്യം("IFS"); /* can be NULL */

 again:
	r = shell_builtin_read(&params);

	എങ്കിൽ ((uintptr_t)r == 1 && errno == EINTR) {
		unsigned sig = check_and_run_traps();
		എങ്കിൽ (sig != SIGINT)
			പോകുക again;
	}

	എങ്കിൽ ((uintptr_t)r > 1) {
		bb_simple_error_സന്ദേശം(r);
		r = (അക്ഷരം*)(uintptr_t)1;
	}

	തിരികെ (uintptr_t)r;
}
#endif

#എങ്കിൽ ENABLE_HUSH_UMASK
സ്ഥിതം പൂർണ്ണസംഖ്യ FAST_FUNC builtin_umask(അക്ഷരം **argv)
{
	പൂർണ്ണസംഖ്യ rc;
	mode_t mask;

	rc = 1;
	mask = umask(0);
	argv = skip_dash_dash(argv);
	എങ്കിൽ (argv[0]) {
		mode_t old_mask = mask;

		/* numeric umasks are taken as-is */
		/* symbolic umasks are inverted: "umask a=rx" calls umask(222) */
		എങ്കിൽ (!isdigit(argv[0][0]))
			mask ^= 0777;
		mask = bb_parse_mode(argv[0], mask);
		എങ്കിൽ (!isdigit(argv[0][0]))
			mask ^= 0777;
		എങ്കിൽ ((unsigned)mask > 0777) {
			mask = old_mask;
			/* bash messages:
			 * bash: umask: 'q': invalid symbolic mode operator
			 * bash: umask: 999: octal number out of range
			 */
			bb_error_സന്ദേശം("%s: invalid mode '%s'", "umask", argv[0]);
			rc = 0;
		}
	} അല്ലെങ്കിൽ {
		/* Mimic bash */
		printf("%04o\n", (unsigned) mask);
		/* fall through and restore mask which we set to 0 */
	}
	umask(mask);

	തിരികെ !rc; /* rc != 0 - success */
}
#endif

#എങ്കിൽ ENABLE_HUSH_EXPORT || ENABLE_HUSH_READONLY || ENABLE_HUSH_SET || ENABLE_HUSH_TRAP
സ്ഥിതം ശൂന്യം print_escaped(സ്ഥിരം അക്ഷരം *s)
{
//TODO? bash "set" does not quote variables which contain only alnums and "%+,-./:=@_~",
// (but "export" quotes all variables, even with only these chars).
// I think quoting strings with %+,=~ looks better
// (example: "set" printing var== instead of var='=' looks strange)
// IOW: do not quote "-./:@_": / is used in pathnames, : in PATH, -._ often in file names, @ in emails

	എങ്കിൽ (*s == '\'')
		പോകുക squote;
	ചെയ്യുക {
		സ്ഥിരം അക്ഷരം *p = strchrnul(s, '\'');
		/* print 'xxxx', possibly just '' */
		printf("'%.*s'", (പൂർണ്ണസംഖ്യ)(p - s), s);
		എങ്കിൽ (*p == '\0')
			നിർത്തുക;
		s = p;
 squote:
		/* s points to '; print "'''...'''" */
		putchar('"');
		ചെയ്യുക putchar('\''); ആകുമ്പോൾ (*++s == '\'');
		putchar('"');
	} ആകുമ്പോൾ (*s);
}
#endif

#എങ്കിൽ ENABLE_HUSH_EXPORT || ENABLE_HUSH_LOCAL || ENABLE_HUSH_READONLY
സ്ഥിതം പൂർണ്ണസംഖ്യ helper_export_local(അക്ഷരം **argv, unsigned flags)
{
	ചെയ്യുക {
		അക്ഷരം *name = *argv;
		സ്ഥിരം അക്ഷരം *name_end = endofname(name);

		എങ്കിൽ (*name_end == '\0') {
			ഘടന variable *var, **vpp;

			vpp = get_ptr_to_local_var(name);
			var = vpp ? *vpp : ശൂന്യ;

			എങ്കിൽ (flags & SETFLAG_UNEXPORT) {
				/* export -n NAME (without =VALUE) */
				എങ്കിൽ (var) {
					var->flg_export = 0;
					debug_printf_env("%s: unsetenv '%s'\n", __func__, name);
					unsetenv(name);
				} /* else: export -n NOT_EXISTING_VAR: no-op */
				തുടരുക;
			}
			എങ്കിൽ (flags & SETFLAG_EXPORT) {
				/* export NAME (without =VALUE) */
				എങ്കിൽ (var) {
					var->flg_export = 1;
					debug_printf_env("%s: putenv '%s'\n", __func__, var->varstr);
					putenv(var->varstr);
					തുടരുക;
				}
			}
			എങ്കിൽ (flags & SETFLAG_MAKE_RO) {
				/* readonly NAME (without =VALUE) */
				എങ്കിൽ (var) {
					var->flg_read_only = 1;
					തുടരുക;
				}
			}
# എങ്കിൽ ENABLE_HUSH_LOCAL
			/* Is this "local" bltin? */
			എങ്കിൽ (!(flags & (SETFLAG_EXPORT|SETFLAG_UNEXPORT|SETFLAG_MAKE_RO))) {
				unsigned lvl = flags >> SETFLAG_VARLVL_SHIFT;
				എങ്കിൽ (var && var->var_nest_level == lvl) {
					/* "local x=abc; ...; local x" - ignore second local decl */
					തുടരുക;
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
		} അല്ലെങ്കിൽ {
			എങ്കിൽ (*name_end != '=') {
				bb_error_സന്ദേശം("'%s': bad variable name", name);
				/* do not parse following argv[]s: */
				തിരികെ 1;
			}
			/* (Un)exporting/making local NAME=VALUE */
			name = xstrdup(name);
			/* Testcase: export PS1='\w \$ ' */
			unbackslash(name);
		}
		debug_printf_env("%s: set_local_var('%s')\n", __func__, name);
		എങ്കിൽ (set_local_var(name, flags))
			തിരികെ EXIT_FAILURE;
	} ആകുമ്പോൾ (*++argv);
	തിരികെ EXIT_SUCCESS;
}
#endif

#എങ്കിൽ ENABLE_HUSH_EXPORT
സ്ഥിതം പൂർണ്ണസംഖ്യ FAST_FUNC builtin_export(അക്ഷരം **argv)
{
	unsigned opt_unexport;

# എങ്കിൽ ENABLE_HUSH_EXPORT_N
	/* "!": do not abort on errors */
	opt_unexport = getopt32(argv, "!n");
	എങ്കിൽ (opt_unexport == (uint32_t)-1)
		തിരികെ EXIT_FAILURE;
	argv += optind;
# അല്ലെങ്കിൽ
	opt_unexport = 0;
	argv++;
# endif

	എങ്കിൽ (argv[0] == ശൂന്യ) {
		അക്ഷരം **e = environ;
		എങ്കിൽ (e) {
			ആകുമ്പോൾ (*e) {
# എങ്കിൽ 0
				puts(*e++);
# അല്ലെങ്കിൽ
				/* ash emits: export VAR='VAL'
				 * bash: declare -x VAR="VAL"
				 * we follow ash example */
				സ്ഥിരം അക്ഷരം *s = *e++;
				സ്ഥിരം അക്ഷരം *p = strchr(s, '=');

				എങ്കിൽ (!p) /* wtf? take next variable */
					തുടരുക;
				/* "export VAR=" */
				printf("%s %.*s", "export", (പൂർണ്ണസംഖ്യ)(p - s) + 1, s);
				print_escaped(p + 1);
				putchar('\n');
# endif
			}
			/*fflush_all(); - done after each builtin anyway */
		}
		തിരികെ EXIT_SUCCESS;
	}

	തിരികെ helper_export_local(argv, opt_unexport ? SETFLAG_UNEXPORT : SETFLAG_EXPORT);
}
#endif

#എങ്കിൽ ENABLE_HUSH_LOCAL
സ്ഥിതം പൂർണ്ണസംഖ്യ FAST_FUNC builtin_local(അക്ഷരം **argv)
{
	എങ്കിൽ (G.func_nest_level == 0) {
		bb_error_സന്ദേശം("%s: not in a function", argv[0]);
		തിരികെ EXIT_FAILURE; /* bash compat */
	}
//TODO? ash and bash support "local -" special form,
//which saves/restores $- around function call (including async returns, such as ^C)
//(IOW: it makes "set +/-..." effects local)
	argv++;
	/* Since all builtins run in a nested variable level,
	 * need to use level - 1 here. Or else the variable will be removed at once
	 * after builtin returns.
	 */
	തിരികെ helper_export_local(argv, (G.var_nest_level - 1) << SETFLAG_VARLVL_SHIFT);
}
#endif

#എങ്കിൽ ENABLE_HUSH_READONLY
സ്ഥിതം പൂർണ്ണസംഖ്യ FAST_FUNC builtin_readonly(അക്ഷരം **argv)
{
	argv++;
	എങ്കിൽ (*argv == ശൂന്യ) {
		/* bash: readonly [-p]: list all readonly VARs
		 * (-p has no effect in bash)
		 */
		ഘടന variable *e;
		വേണ്ടി (e = G.മുകൾ_var; e; e = e->next) {
			എങ്കിൽ (e->flg_read_only) {
				സ്ഥിരം അക്ഷരം *s = e->varstr;
				സ്ഥിരം അക്ഷരം *p = strchr(s, '=');

				എങ്കിൽ (!p) /* wtf? take next variable */
					തുടരുക;
				/* "readonly VAR=" */
				printf("%s %.*s", "readonly", (പൂർണ്ണസംഖ്യ)(p - s) + 1, s);
				print_escaped(p + 1);
				putchar('\n');
			}
		}
		തിരികെ EXIT_SUCCESS;
	}
	തിരികെ helper_export_local(argv, SETFLAG_MAKE_RO);
}
#endif

#എങ്കിൽ ENABLE_HUSH_UNSET
/* http://www.opengroup.org/onlinepubs/9699919799/utilities/V3_chap02.html#unset */
സ്ഥിതം പൂർണ്ണസംഖ്യ FAST_FUNC builtin_unset(അക്ഷരം **argv)
{
	പൂർണ്ണസംഖ്യ ret;
	unsigned opts;

	/* "!": do not abort on errors */
	/* "+": stop at 1st non-option */
	opts = getopt32(argv, "!+vf");
	എങ്കിൽ (opts == (unsigned)-1)
		തിരികെ EXIT_FAILURE;
	എങ്കിൽ (opts == 3) {
		bb_simple_error_സന്ദേശം("unset: -v and -f are exclusive");
		തിരികെ EXIT_FAILURE;
	}
	argv += optind;

	ret = EXIT_SUCCESS;
	ആകുമ്പോൾ (*argv) {
		എങ്കിൽ (!(opts & 2)) { /* not -f */
			എങ്കിൽ (unset_local_var(*argv)) {
				/* unset <nonexistent_var> doesn't fail.
				 * Error is when one tries to unset RO var.
				 * Message was printed by unset_local_var. */
				ret = EXIT_FAILURE;
			}
		}
# എങ്കിൽ ENABLE_HUSH_FUNCTIONS
		അല്ലെങ്കിൽ {
			unset_func(*argv);
		}
# endif
		argv++;
	}
	തിരികെ ret;
}
#endif

#എങ്കിൽ ENABLE_HUSH_SET
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
സ്ഥിതം പൂർണ്ണസംഖ്യ FAST_FUNC builtin_set(അക്ഷരം **argv)
{
	പൂർണ്ണസംഖ്യ n;
	അക്ഷരം **pp, **g_argv;
	അക്ഷരം *arg = *++argv;

	എങ്കിൽ (arg == ശൂന്യ) {
		ഘടന variable *e;
		വേണ്ടി (e = G.മുകൾ_var; e; e = e->next) {
			സ്ഥിരം അക്ഷരം *s = e->varstr;
			സ്ഥിരം അക്ഷരം *p = strchr(s, '=');

			എങ്കിൽ (!p) /* wtf? take next variable */
				തുടരുക;
			/* var= */
			printf("%.*s", (പൂർണ്ണസംഖ്യ)(p - s) + 1, s);
			print_escaped(p + 1);
			putchar('\n');
		}
		തിരികെ EXIT_SUCCESS;
	}

	ചെയ്യുക {
		എങ്കിൽ (strcmp(arg, "--") == 0) {
			++argv;
			പോകുക set_argv;
		}
		എങ്കിൽ (arg[0] != '+' && arg[0] != '-')
			നിർത്തുക;
		വേണ്ടി (n = 1; arg[n]; ++n) {
			എങ്കിൽ (set_mode((arg[0] == '-'), arg[n], argv[1])) {
				bb_error_സന്ദേശം("%s: %s: invalid option", "set", arg);
				തിരികെ EXIT_FAILURE;
			}
			എങ്കിൽ (arg[n] == 'o' && argv[1])
				argv++;
		}
	} ആകുമ്പോൾ ((arg = *++argv) != ശൂന്യ);
	/* Now argv[0] is 1st argument */

	എങ്കിൽ (arg == ശൂന്യ)
		തിരികെ EXIT_SUCCESS;
 set_argv:

	/* NB: G.global_argv[0] ($0) is never freed/changed */
	g_argv = G.global_argv;
	എങ്കിൽ (G.global_args_malloced) {
		pp = g_argv;
		ആകുമ്പോൾ (*++pp)
			സ്വതന്ത്രം(*pp);
		g_argv[1] = ശൂന്യ;
	} അല്ലെങ്കിൽ {
		G.global_args_malloced = 1;
		pp = xzalloc(വലുപ്പം(pp[0]) * 2);
		pp[0] = g_argv[0]; /* retain $0 */
		g_argv = pp;
	}
	/* This realloc's G.global_argv */
	G.global_argv = pp = add_strings_to_strings(g_argv, argv, /*dup:*/ 1);

	G.global_argc = 1 + string_array_len(pp + 1);

	തിരികെ EXIT_SUCCESS;
}
#endif

സ്ഥിതം പൂർണ്ണസംഖ്യ FAST_FUNC builtin_shift(അക്ഷരം **argv)
{
	പൂർണ്ണസംഖ്യ n = 1;
	argv = skip_dash_dash(argv);
	എങ്കിൽ (argv[0]) {
		n = bb_strtou(argv[0], ശൂന്യ, 10);
		എങ്കിൽ (errno || n < 0) {
			/* shared string with ash.c */
			bb_error_സന്ദേശം("Illegal number: %s", argv[0]);
			/*
			 * ash aborts in this case.
			 * bash prints error message and set $? to 1.
			 * Interestingly, for "shift 99999" bash does not
			 * print error message, but does set $? to 1
			 * (and does no shifting at all).
			 */
		}
	}
	എങ്കിൽ (n >= 0 && n < G.global_argc) {
		എങ്കിൽ (G_global_args_malloced) {
			പൂർണ്ണസംഖ്യ m = 1;
			ആകുമ്പോൾ (m <= n)
				സ്വതന്ത്രം(G.global_argv[m++]);
		}
		G.global_argc -= n;
		memmove(&G.global_argv[1], &G.global_argv[n+1],
				G.global_argc * വലുപ്പം(G.global_argv[0]));
		തിരികെ EXIT_SUCCESS;
	}
	തിരികെ EXIT_FAILURE;
}

#എങ്കിൽ ENABLE_HUSH_GETOPTS
സ്ഥിതം പൂർണ്ണസംഖ്യ FAST_FUNC builtin_getopts(അക്ഷരം **argv)
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
	അക്ഷരം cbuf[2];
	സ്ഥിരം അക്ഷരം *cp, *optstring, *var;
	പൂർണ്ണസംഖ്യ c, n, exitcode, my_opterr;
	unsigned എണ്ണം;

	optstring = *++argv;
	എങ്കിൽ (!optstring || !(var = *++argv)) {
		bb_simple_error_സന്ദേശം("usage: getopts OPTSTRING VAR [ARGS]");
		തിരികെ EXIT_FAILURE;
	}

	എങ്കിൽ (argv[1])
		argv[0] = G.global_argv[0]; /* for error messages in getopt() */
	അല്ലെങ്കിൽ
		argv = G.global_argv;
	cbuf[1] = '\0';

	my_opterr = 0;
	എങ്കിൽ (optstring[0] != ':') {
		cp = get_local_var_മൂല്യം("OPTERR");
		/* 0 if "OPTERR=0", 1 otherwise */
		my_opterr = (!cp || NOT_LONE_CHAR(cp, '0'));
	}

	/* getopts stops on first non-option. Add "+" to force that */
	/*if (optstring[0] != '+')*/ {
		അക്ഷരം *s = alloca(strlen(optstring) + 2);
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
	എണ്ണം = 0;
	n = string_array_len(argv);
	ചെയ്യുക {
		optarg = ശൂന്യ;
		opterr = (എണ്ണം < G.getopt_എണ്ണം) ? 0 : my_opterr;
		c = getopt(n, argv, optstring);
		എങ്കിൽ (c < 0)
			നിർത്തുക;
		എണ്ണം++;
	} ആകുമ്പോൾ (എണ്ണം <= G.getopt_എണ്ണം);

	/* Set OPTIND. Prevent resetting of the magic counter! */
	set_local_var_from_halves("OPTIND", utoa(optind));
	G.getopt_എണ്ണം = എണ്ണം; /* "next time, give me N+1'th result" */
	GETOPT_RESET(); /* just in case */

	/* Set OPTARG */
	/* Always set or unset, never left as-is, even on exit/error:
	 * "If no option was found, or if the option that was found
	 * does not have an option-argument, OPTARG shall be unset."
	 */
	cp = optarg;
	എങ്കിൽ (c == '?') {
		/* If ":optstring" and unknown option is seen,
		 * it is stored to OPTARG.
		 */
		എങ്കിൽ (optstring[1] == ':') {
			cbuf[0] = optopt;
			cp = cbuf;
		}
	}
	എങ്കിൽ (cp)
		set_local_var_from_halves("OPTARG", cp);
	അല്ലെങ്കിൽ
		unset_local_var("OPTARG");

	/* Convert -1 to "?" */
	exitcode = EXIT_SUCCESS;
	എങ്കിൽ (c < 0) { /* -1: end of options */
		exitcode = EXIT_FAILURE;
		c = '?';
	}

	/* Set VAR */
	cbuf[0] = c;
	set_local_var_from_halves(var, cbuf);

	തിരികെ exitcode;
}
#endif

സ്ഥിതം പൂർണ്ണസംഖ്യ FAST_FUNC builtin_source(അക്ഷരം **argv)
{
	അക്ഷരം *arg_path, *filename;
	HFILE *നിവേശം;
	save_arg_t sv;
	അക്ഷരം *args_need_save;
#എങ്കിൽ ENABLE_HUSH_FUNCTIONS
	smallint sv_flg;
#endif

	argv = skip_dash_dash(argv);
	filename = argv[0];
	എങ്കിൽ (!filename) {
		/* bash says: "bash: .: filename argument required" */
		തിരികെ 2; /* bash compat */
	}
	arg_path = ശൂന്യ;
	എങ്കിൽ (!strchr(filename, '/')) {
		arg_path = find_in_path(filename);
		എങ്കിൽ (arg_path)
			filename = arg_path;
		അല്ലെങ്കിൽ എങ്കിൽ (!ENABLE_HUSH_BASH_SOURCE_CURDIR) {
			errno = ENOENT;
			bb_simple_perror_സന്ദേശം(filename);
			തിരികെ EXIT_FAILURE;
		}
	}
	നിവേശം = hfopen(filename);
	സ്വതന്ത്രം(arg_path);
	എങ്കിൽ (!നിവേശം) {
		bb_perror_സന്ദേശം("%s", filename);
		/* POSIX: non-interactive shell should abort here,
		 * not merely fail. So far no one complained :)
		 */
		തിരികെ EXIT_FAILURE;
	}

#എങ്കിൽ ENABLE_HUSH_FUNCTIONS
	sv_flg = G_flag_return_in_progress;
	/* "we are inside sourced file, ok to use return" */
	G_flag_return_in_progress = -1;
#endif
	args_need_save = argv[1]; /* used as a boolean variable */
	എങ്കിൽ (args_need_save)
		save_and_replace_G_args(&sv, argv);

	/* "false; . ./empty_line; echo Zero:$?" should print 0 */
	G.last_exitcode = 0;
	parse_and_run_file(നിവേശം);
	hfclose(നിവേശം);

	എങ്കിൽ (args_need_save) /* can't use argv[1] instead: "shift" can mangle it */
		restore_G_args(&sv, argv);
#എങ്കിൽ ENABLE_HUSH_FUNCTIONS
	G_flag_return_in_progress = sv_flg;
#endif

	തിരികെ G.last_exitcode;
}

#എങ്കിൽ ENABLE_HUSH_TRAP
സ്ഥിതം പൂർണ്ണസംഖ്യ FAST_FUNC builtin_trap(അക്ഷരം **argv)
{
	പൂർണ്ണസംഖ്യ sig;
	അക്ഷരം *new_cmd;

	എങ്കിൽ (!G_traps)
		G_traps = xzalloc(വലുപ്പം(G_traps[0]) * NSIG);

	argv++;
	എങ്കിൽ (!*argv) {
		പൂർണ്ണസംഖ്യ i;
		/* No args: print all trapped */
		വേണ്ടി (i = 0; i < NSIG; ++i) {
			എങ്കിൽ (G_traps[i]) {
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
		തിരികെ EXIT_SUCCESS;
	}

	new_cmd = ശൂന്യ;
	/* If first arg is a number: reset all specified signals */
	sig = bb_strtou(*argv, ശൂന്യ, 10);
	എങ്കിൽ (errno == 0) {
		പൂർണ്ണസംഖ്യ ret;
 process_sig_list:
		ret = EXIT_SUCCESS;
		ആകുമ്പോൾ (*argv) {
			sighandler_t handler;

			sig = get_signum(*argv++);
			എങ്കിൽ (sig < 0) {
				ret = EXIT_FAILURE;
				/* Mimic bash message exactly */
				bb_error_സന്ദേശം("trap: %s: invalid signal specification", argv[-1]);
				തുടരുക;
			}

			സ്വതന്ത്രം(G_traps[sig]);
			G_traps[sig] = xstrdup(new_cmd);

			debug_printf("trap: setting SIG%s (%i) to '%s'\n",
				get_signame(sig), sig, G_traps[sig]);

			/* There is no signal for 0 (EXIT) */
			എങ്കിൽ (sig == 0)
				തുടരുക;

			എങ്കിൽ (new_cmd)
				handler = (new_cmd[0] ? record_pending_signo : SIG_IGN);
			അല്ലെങ്കിൽ
				/* We are removing trap handler */
				handler = pick_sighandler(sig);
			install_sighandler(sig, handler);
		}
		തിരികെ ret;
	}

	എങ്കിൽ (!argv[1]) { /* no second arg */
		bb_simple_error_സന്ദേശം("trap: invalid arguments");
		തിരികെ EXIT_FAILURE;
	}

	/* First arg is "-": reset all specified to default */
	/* First arg is "--": skip it, the rest is "handler SIGs..." */
	/* Everything else: set arg as signal handler
	 * (includes "" case, which ignores signal) */
	എങ്കിൽ (argv[0][0] == '-') {
		എങ്കിൽ (argv[0][1] == '\0') { /* "-" */
			/* new_cmd remains NULL: "reset these sigs" */
			പോകുക reset_traps;
		}
		എങ്കിൽ (argv[0][1] == '-' && argv[0][2] == '\0') { /* "--" */
			argv++;
		}
		/* else: "-something", no special meaning */
	}
	new_cmd = *argv;
 reset_traps:
	argv++;
	പോകുക process_sig_list;
}
#endif

#എങ്കിൽ ENABLE_HUSH_JOB
സ്ഥിതം ഘടന pipe *parse_jobspec(സ്ഥിരം അക്ഷരം *str)
{
	ഘടന pipe *pi;
	unsigned jobnum;

	എങ്കിൽ (sscanf(str, "%%%u", &jobnum) != 1) {
		എങ്കിൽ (str[0] != '%'
		 || (str[1] != '%' && str[1] != '+' && str[1] != '\0')
		) {
			bb_error_സന്ദേശം("bad argument '%s'", str);
			തിരികെ ശൂന്യ;
		}
		/* It is "%%", "%+" or "%" - current job */
		jobnum = G.last_jobid;
		എങ്കിൽ (jobnum == 0) {
			bb_simple_error_സന്ദേശം("no current job");
			തിരികെ ശൂന്യ;
		}
	}
	വേണ്ടി (pi = G.job_list; pi; pi = pi->next) {
		എങ്കിൽ (pi->jobid == jobnum) {
			തിരികെ pi;
		}
	}
	bb_error_സന്ദേശം("%u: no such job", jobnum);
	തിരികെ ശൂന്യ;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ FAST_FUNC builtin_jobs(അക്ഷരം **argv UNUSED_PARAM)
{
	ഘടന pipe *job;
	സ്ഥിരം അക്ഷരം *status_string;

	checkjobs(ശൂന്യ, 0 /*(no pid to wait for)*/);
	വേണ്ടി (job = G.job_list; job; job = job->next) {
		എങ്കിൽ (job->alive_cmds == job->stopped_cmds)
			status_string = "Stopped";
		അല്ലെങ്കിൽ
			status_string = "Running";

		printf(JOB_STATUS_FORMAT, job->jobid, status_string, job->cmdtext);
	}

	clean_up_last_dead_job();

	തിരികെ EXIT_SUCCESS;
}

/* built-in 'fg' and 'bg' handler */
സ്ഥിതം പൂർണ്ണസംഖ്യ FAST_FUNC builtin_fg_bg(അക്ഷരം **argv)
{
	പൂർണ്ണസംഖ്യ i;
	ഘടന pipe *pi;

	എങ്കിൽ (!G_interactive_fd)
		തിരികെ EXIT_FAILURE;

	/* If they gave us no args, assume they want the last backgrounded task */
	എങ്കിൽ (!argv[1]) {
		വേണ്ടി (pi = G.job_list; pi; pi = pi->next) {
			എങ്കിൽ (pi->jobid == G.last_jobid) {
				പോകുക found;
			}
		}
		bb_error_സന്ദേശം("%s: no current job", argv[0]);
		തിരികെ EXIT_FAILURE;
	}

	pi = parse_jobspec(argv[1]);
	എങ്കിൽ (!pi)
		തിരികെ EXIT_FAILURE;
 found:
	/* TODO: bash prints a string representation
	 * of job being foregrounded (like "sleep 1 | cat") */
	എങ്കിൽ (argv[0][0] == 'f' && G_saved_tty_pgrp) {
		/* Put the job into the foreground. */
		tcsetpgrp(G_interactive_fd, pi->pgrp);
	}

	/* Restart the processes in the job */
	debug_printf_jobs("reviving %d procs, pgrp %d\n", pi->അക്കം_cmds, pi->pgrp);
	വേണ്ടി (i = 0; i < pi->അക്കം_cmds; i++) {
		debug_printf_jobs("reviving pid %d\n", pi->cmds[i].pid);
	}
	pi->stopped_cmds = 0;

	i = kill(- pi->pgrp, SIGCONT);
	എങ്കിൽ (i < 0) {
		എങ്കിൽ (errno == ESRCH) {
			delete_finished_job(pi);
			തിരികെ EXIT_SUCCESS;
		}
		bb_simple_perror_സന്ദേശം("kill (SIGCONT)");
	}

	എങ്കിൽ (argv[0][0] == 'f') {
		remove_job_from_table(pi); /* FG job shouldn't be in job table */
		തിരികെ checkjobs_and_fg_shell(pi);
	}
	തിരികെ EXIT_SUCCESS;
}
#endif

#എങ്കിൽ ENABLE_HUSH_KILL
സ്ഥിതം പൂർണ്ണസംഖ്യ FAST_FUNC builtin_kill(അക്ഷരം **argv)
{
	പൂർണ്ണസംഖ്യ ret = 0;

# എങ്കിൽ ENABLE_HUSH_JOB
	എങ്കിൽ (argv[1] && strcmp(argv[1], "-l") != 0) {
		പൂർണ്ണസംഖ്യ i = 1;

		ചെയ്യുക {
			ഘടന pipe *pi;
			അക്ഷരം *dst;
			പൂർണ്ണസംഖ്യ j, n;

			എങ്കിൽ (argv[i][0] != '%')
				തുടരുക;
			/*
			 * "kill %N" - job kill
			 * Converting to pgrp / pid kill
			 */
			pi = parse_jobspec(argv[i]);
			എങ്കിൽ (!pi) {
				/* Eat bad jobspec */
				j = i;
				ചെയ്യുക {
					j++;
					argv[j - 1] = argv[j];
				} ആകുമ്പോൾ (argv[j]);
				ret = 1;
				i--;
				തുടരുക;
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
			n = G_interactive_fd ? 1 : pi->അക്കം_cmds;
			dst = alloca(n * വലുപ്പം(പൂർണ്ണസംഖ്യ)*4);
			argv[i] = dst;
			എങ്കിൽ (G_interactive_fd)
				dst += sprintf(dst, " -%u", (പൂർണ്ണസംഖ്യ)pi->pgrp);
			അല്ലെങ്കിൽ വേണ്ടി (j = 0; j < n; j++) {
				ഘടന command *cmd = &pi->cmds[j];
				/* Skip exited members of the job */
				എങ്കിൽ (cmd->pid == 0)
					തുടരുക;
				/*
				 * kill_main has matching code to expect
				 * leading space. Needed to not confuse
				 * negative pids with "kill -SIGNAL_NO" syntax
				 */
				dst += sprintf(dst, " %u", (പൂർണ്ണസംഖ്യ)cmd->pid);
			}
			*dst = '\0';
		} ആകുമ്പോൾ (argv[++i]);
	}
# endif

	എങ്കിൽ (argv[1] || ret == 0) {
		ret = run_applet_main(argv, kill_main);
	}
	/* else: ret = 1, "kill %bad_jobspec" case */
	തിരികെ ret;
}
#endif

#എങ്കിൽ ENABLE_HUSH_WAIT
/* http://www.opengroup.org/onlinepubs/9699919799/utilities/wait.html */
# എങ്കിൽ !ENABLE_HUSH_JOB
#  define wait_for_child_or_signal(pipe,pid) wait_for_child_or_signal(pid)
# endif
സ്ഥിതം പൂർണ്ണസംഖ്യ wait_for_child_or_signal(ഘടന pipe *waitfor_pipe, pid_t waitfor_pid)
{
	പൂർണ്ണസംഖ്യ ret = 0;
	വേണ്ടി (;;) {
		പൂർണ്ണസംഖ്യ sig;
		sigset_t oldset;

		എങ്കിൽ (!sigisemptyset(&G.pending_set))
			പോകുക check_sig;

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

		എങ്കിൽ (!sigisemptyset(&G.pending_set)) {
			/* Crap! we raced with some signal! */
			പോകുക restore;
		}

		/*errno = 0; - checkjobs does this */
/* Can't pass waitfor_pipe into checkjobs(): it won't be interruptible */
		ret = checkjobs(ശൂന്യ, waitfor_pid); /* waitpid(WNOHANG) inside */
		debug_printf_exec("checkjobs:%d\n", ret);
# എങ്കിൽ ENABLE_HUSH_JOB
		എങ്കിൽ (waitfor_pipe) {
			പൂർണ്ണസംഖ്യ rcode = job_exited_or_stopped(waitfor_pipe);
			debug_printf_exec("job_exited_or_stopped:%d\n", rcode);
			എങ്കിൽ (rcode >= 0) {
				ret = rcode;
				sigprocmask(SIG_SETMASK, &oldset, ശൂന്യ);
				നിർത്തുക;
			}
		}
# endif
		/* if ECHILD, there are no children (ret is -1 or 0) */
		/* if ret == 0, no children changed state */
		/* if ret != 0, it's exitcode+1 of exited waitfor_pid child */
		എങ്കിൽ (errno == ECHILD || ret) {
			ret--;
			എങ്കിൽ (ret < 0) /* if ECHILD, may need to fix "ret" */
				ret = 0;
# എങ്കിൽ ENABLE_HUSH_BASH_COMPAT
			എങ്കിൽ (waitfor_pid == -1 && errno == ECHILD) {
				/* exitcode of "wait -n" with no children is 127, not 0 */
				ret = 127;
			}
# endif
			sigprocmask(SIG_SETMASK, &oldset, ശൂന്യ);
			നിർത്തുക;
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
		sigprocmask(SIG_SETMASK, &oldset, ശൂന്യ);
 check_sig:
		/* So, did we get a signal? */
		sig = check_and_run_traps();
		എങ്കിൽ (sig /*&& sig != SIGCHLD - always true */) {
			/* Do this for any (non-ignored) signal, not only for ^C */
			ret = 128 | sig;
			നിർത്തുക;
		}
		/* SIGCHLD, or no signal, or ignored one, such as SIGQUIT. Repeat */
	}
	തിരികെ ret;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ FAST_FUNC builtin_wait(അക്ഷരം **argv)
{
	പൂർണ്ണസംഖ്യ ret;
	പൂർണ്ണസംഖ്യ status;

	argv = skip_dash_dash(argv);
# എങ്കിൽ ENABLE_HUSH_BASH_COMPAT
	എങ്കിൽ (argv[0] && strcmp(argv[0], "-n") == 0) {
		/* wait -n */
		/* (bash accepts "wait -n PID" too and ignores PID) */
		G.dead_job_exitcode = -1;
		തിരികെ wait_for_child_or_signal(ശൂന്യ, -1 /*no job, wait for one job*/);
	}
# endif
	എങ്കിൽ (argv[0] == ശൂന്യ) {
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
		തിരികെ wait_for_child_or_signal(ശൂന്യ, 0 /*no job and no pid to wait for*/);
	}

	ചെയ്യുക {
		pid_t pid = bb_strtou(*argv, ശൂന്യ, 10);
		എങ്കിൽ (errno || pid <= 0) {
# എങ്കിൽ ENABLE_HUSH_JOB
			എങ്കിൽ (argv[0][0] == '%') {
				ഘടന pipe *wait_pipe;
				ret = 127; /* bash compat for bad jobspecs */
				wait_pipe = parse_jobspec(*argv);
				എങ്കിൽ (wait_pipe) {
					ret = job_exited_or_stopped(wait_pipe);
					എങ്കിൽ (ret < 0) {
						ret = wait_for_child_or_signal(wait_pipe, 0);
					} അല്ലെങ്കിൽ {
						/* waiting on "last dead job" removes it */
						clean_up_last_dead_job();
					}
				}
				/* else: parse_jobspec() already emitted error msg */
				തുടരുക;
			}
# endif
			/* mimic bash message */
			bb_error_സന്ദേശം("wait: '%s': not a pid or valid job spec", *argv);
			ret = EXIT_FAILURE;
			തുടരുക; /* bash checks all argv[] */
		}

		/* Do we have such child? */
		ret = waitpid(pid, &status, WNOHANG);
		എങ്കിൽ (ret < 0) {
			/* No */
			ret = 127;
			എങ്കിൽ (errno == ECHILD) {
				എങ്കിൽ (pid == G.last_bg_pid) {
					/* "wait $!" but last bg task has already exited. Try:
					 * (sleep 1; exit 3) & sleep 2; echo $?; wait $!; echo $?
					 * In bash it prints exitcode 0, then 3.
					 * In dash, it is 127.
					 */
					ret = G.last_bg_pid_exitcode;
				} അല്ലെങ്കിൽ {
					/* Example: "wait 1". mimic bash message */
					bb_error_സന്ദേശം("wait: pid %u is not a child of this shell", (unsigned)pid);
				}
			} അല്ലെങ്കിൽ {
				/* ??? */
				bb_perror_സന്ദേശം("wait %s", *argv);
			}
			തുടരുക; /* bash checks all argv[] */
		}
		എങ്കിൽ (ret == 0) {
			/* Yes, and it still runs */
			ret = wait_for_child_or_signal(ശൂന്യ, pid);
		} അല്ലെങ്കിൽ {
			/* Yes, and it just exited */
			process_wait_result(ശൂന്യ, pid, status);
			ret = WEXITSTATUS(status);
			എങ്കിൽ (WIFSIGNALED(status))
				ret = 128 | WTERMSIG(status);
		}
	} ആകുമ്പോൾ (*++argv);

	തിരികെ ret;
}
#endif

#എങ്കിൽ ENABLE_HUSH_LOOPS || ENABLE_HUSH_FUNCTIONS
സ്ഥിതം unsigned parse_numeric_argv1(അക്ഷരം **argv, unsigned def, unsigned def_min)
{
	എങ്കിൽ (argv[1]) {
		def = bb_strtou(argv[1], ശൂന്യ, 10);
		എങ്കിൽ (errno || def < def_min || argv[2]) {
			bb_error_സന്ദേശം("%s: bad arguments", argv[0]);
			def = UINT_MAX;
		}
	}
	തിരികെ def;
}
#endif

#എങ്കിൽ ENABLE_HUSH_LOOPS
സ്ഥിതം പൂർണ്ണസംഖ്യ FAST_FUNC builtin_break(അക്ഷരം **argv)
{
	unsigned depth;
	എങ്കിൽ (G.depth_of_loop == 0) {
		bb_error_സന്ദേശം("%s: only meaningful in a loop", argv[0]);
		/* if we came from builtin_continue(), need to undo "= 1" */
		G.flag_break_continue = 0;
		തിരികെ EXIT_SUCCESS; /* bash compat */
	}
	G.flag_break_continue++; /* BC_BREAK = 1, or BC_CONTINUE = 2 */

	G.depth_break_continue = depth = parse_numeric_argv1(argv, 1, 1);
	എങ്കിൽ (depth == UINT_MAX)
		G.flag_break_continue = BC_BREAK;
	എങ്കിൽ (G.depth_of_loop < depth)
		G.depth_break_continue = G.depth_of_loop;

	തിരികെ EXIT_SUCCESS;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ FAST_FUNC builtin_continue(അക്ഷരം **argv)
{
	G.flag_break_continue = 1; /* BC_CONTINUE = 2 = 1+1 */
	തിരികെ builtin_break(argv);
}
#endif

#എങ്കിൽ ENABLE_HUSH_FUNCTIONS
സ്ഥിതം പൂർണ്ണസംഖ്യ FAST_FUNC builtin_return(അക്ഷരം **argv)
{
	പൂർണ്ണസംഖ്യ rc;

	എങ്കിൽ (G_flag_return_in_progress != -1) {
		bb_error_സന്ദേശം("%s: not in a function or sourced script", argv[0]);
		തിരികെ EXIT_FAILURE; /* bash compat */
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
# എങ്കിൽ ENABLE_HUSH_TRAP
	എങ്കിൽ (argv[1]) { /* "return ARG" inside a running trap sets $? */
		debug_printf_exec("G.return_exitcode=%d\n", rc);
		G.return_exitcode = rc;
	}
# endif
	തിരികെ rc;
}
#endif

#എങ്കിൽ ENABLE_HUSH_TIMES
സ്ഥിതം പൂർണ്ണസംഖ്യ FAST_FUNC builtin_times(അക്ഷരം **argv UNUSED_PARAM)
{
	സ്ഥിതം സ്ഥിരം uint8_t times_tbl[] ALIGN1 = {
		' ',  offsetof(ഘടന tms, tms_utime),
		'\n', offsetof(ഘടന tms, tms_stime),
		' ',  offsetof(ഘടന tms, tms_cutime),
		'\n', offsetof(ഘടന tms, tms_cstime),
		0
	};
	സ്ഥിരം uint8_t *p;
	unsigned clk_tck;
	ഘടന tms buf;

	clk_tck = bb_clk_tck();

	times(&buf);
	p = times_tbl;
	ചെയ്യുക {
		unsigned sec, frac;
		unsigned long t;
		t = *(clock_t *)(((അക്ഷരം *) &buf) + p[1]);
		sec = t / clk_tck;
		frac = t % clk_tck;
		printf("%um%u.%03us%c",
			sec / 60, sec % 60,
			(frac * 1000) / clk_tck,
			p[0]);
		p += 2;
	} ആകുമ്പോൾ (*p);

	തിരികെ EXIT_SUCCESS;
}
#endif

#എങ്കിൽ ENABLE_HUSH_MEMLEAK
സ്ഥിതം പൂർണ്ണസംഖ്യ FAST_FUNC builtin_memleak(അക്ഷരം **argv UNUSED_PARAM)
{
	ശൂന്യം *p;
	unsigned long l;

# ifdef M_TRIM_THRESHOLD
	/* Optional. Reduces probability of false positives */
	malloc_trim(0);
# endif
	/* Crude attempt to find where "free memory" starts,
	 * sans fragmentation. */
	p = malloc(240);
	l = (unsigned long)p;
	സ്വതന്ത്രം(p);
	p = malloc(3400);
	എങ്കിൽ (l < (unsigned long)p) l = (unsigned long)p;
	സ്വതന്ത്രം(p);

# എങ്കിൽ 0  /* debug */
	{
		ഘടന mallinfo mi = mallinfo();
		printf("top alloc:0x%lx malloced:%d+%d=%d\n", l,
			mi.arena, mi.hblkhd, mi.arena + mi.hblkhd);
	}
# endif

	എങ്കിൽ (!G.memleak_മൂല്യം)
		G.memleak_മൂല്യം = l;

	l -= G.memleak_മൂല്യം;
	എങ്കിൽ ((long)l < 0)
		l = 0;
	l /= 1024;
	എങ്കിൽ (l > 127)
		l = 127;

	/* Exitcode is "how many kilobytes we leaked since 1st call" */
	തിരികെ l;
}
#endif
#endif /* !__U_BOOT__ */
