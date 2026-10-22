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
#যদি !(defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) \
	|| defined(__APPLE__) \
    )
# include <malloc.h>   /* for malloc_trim */
#endif
#অন্তর্ভাব <glob.h>
/* #include <dmalloc.h> */
#যদি ENABLE_HUSH_CASE
# include <fnmatch.h>
#endif
#অন্তর্ভাব <sys/times.h>
#অন্তর্ভাব <sys/utsname.h> /* for setting $HOSTNAME */

#অন্তর্ভাব "busybox.h"  /* for APPLET_IS_NOFORK/NOEXEC */
#অন্তর্ভাব "unicode.h"
#অন্তর্ভাব "shell_common.h"
#অন্তর্ভাব "math.h"
#অন্তর্ভাব "match.h"
#যদি ENABLE_HUSH_RANDOM_SUPPORT
# include "random.h"
#অন্যথা
# define CLEAR_RANDOM_T(rnd) ((শূন্য)0)
#endif
#ifndef O_CLOEXEC
# define O_CLOEXEC 0
#endif
#ifndef F_DUPFD_CLOEXEC
# define F_DUPFD_CLOEXEC F_DUPFD
#endif

#যদি ENABLE_FEATURE_SH_EMBEDDED_SCRIPTS && !ENABLE_SHELL_ASH
# include "embedded_scripts.h"
#অন্যথা
# define অঙ্ক_SCRIPTS 0
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

#যদি BUILD_AS_NOMMU
# undef BB_MMU
# undef USE_FOR_NOMMU
# undef USE_FOR_MMU
# define BB_MMU 0
# define USE_FOR_NOMMU(...) __VA_ARGS__
# define USE_FOR_MMU(...)
#endif

#ifndef __U_BOOT__
#অন্তর্ভাব "NUM_APPLETS.h"
#যদি অঙ্ক_APPLETS == 1
/* STANDALONE does not make sense, and won't compile */
# undef ENABLE_FEATURE_SH_STANDALONE
# undef IF_FEATURE_SH_STANDALONE
# undef IF_NOT_FEATURE_SH_STANDALONE
# define ENABLE_FEATURE_SH_STANDALONE 0
# define IF_FEATURE_SH_STANDALONE(...)
# define IF_NOT_FEATURE_SH_STANDALONE(...) __VA_ARGS__
#endif
#endif /* __U_BOOT__ */

#যদি !ENABLE_HUSH_INTERACTIVE
# undef ENABLE_FEATURE_EDITING
# define ENABLE_FEATURE_EDITING 0
# undef ENABLE_FEATURE_EDITING_FANCY_PROMPT
# define ENABLE_FEATURE_EDITING_FANCY_PROMPT 0
# undef ENABLE_FEATURE_EDITING_SAVE_ON_EXIT
# define ENABLE_FEATURE_EDITING_SAVE_ON_EXIT 0
#endif

/* Do we support ANY keywords? */
#যদি ENABLE_HUSH_IF || ENABLE_HUSH_LOOPS || ENABLE_HUSH_CASE
# define HAS_KEYWORDS 1
# define IF_HAS_KEYWORDS(...) __VA_ARGS__
# define IF_HAS_NO_KEYWORDS(...)
#অন্যথা
# define HAS_KEYWORDS 0
# define IF_HAS_KEYWORDS(...)
# define IF_HAS_NO_KEYWORDS(...) __VA_ARGS__
#endif

/* If you comment out one of these below, it will be #defined later
 * to perform debug printfs to stderr: */
#define debug_printf(...)         করো {} যখন (0)
/* Finer-grained debug switches */
#define debug_printf_parse(...)   করো {} যখন (0)
#define debug_printf_heredoc(...) করো {} যখন (0)
#define debug_print_বৃক্ষ(a, b)    করো {} যখন (0)
#define debug_printf_exec(...)    করো {} যখন (0)
#define debug_printf_env(...)     করো {} যখন (0)
#define debug_printf_jobs(...)    করো {} যখন (0)
#define debug_printf_expand(...)  করো {} যখন (0)
#define debug_printf_varexp(...)  করো {} যখন (0)
#define debug_printf_glob(...)    করো {} যখন (0)
#define debug_printf_redir(...)   করো {} যখন (0)
#define debug_printf_list(...)    করো {} যখন (0)
#define debug_printf_subst(...)   করো {} যখন (0)
#define debug_printf_prompt(...)  করো {} যখন (0)
#define debug_printf_clean(...)   করো {} যখন (0)

#define ERR_PTR ((শূন্য*)(long)1)

#define JOB_STATUS_FORMAT    "[%u] %-22s %.40s\n"

#define _SPECIAL_VARS_STR     "_*@$!?#-"
#ifndef __U_BOOT__
#define SPECIAL_VARS_STR     ("_*@$!?#-" + 1)
#define NUMERIC_SPECVARS_STR ("_*@$!?#-" + 3)
#অন্যথা /* __U_BOOT__ */
#define SPECIAL_VARS_STR     "*@$!?#-"
#define NUMERIC_SPECVARS_STR "$!?#-"
#endif /* __U_BOOT__ */
#যদি BASH_PATTERN_SUBST
/* Support / and // replace ops */
/* Note that // is stored as \ in "encoded" string representation */
# define VAR_ENCODED_SUBST_OPS      "\\/%#:-=+?"
# define VAR_SUBST_OPS             ("\\/%#:-=+?" + 1)
# define MINUS_PLUS_EQUAL_QUESTION ("\\/%#:-=+?" + 5)
#অন্যথা
# define VAR_ENCODED_SUBST_OPS      "%#:-=+?"
# define VAR_SUBST_OPS              "%#:-=+?"
# define MINUS_PLUS_EQUAL_QUESTION ("%#:-=+?" + 3)
#endif

#define SPECIAL_VAR_প্রতীক_STR "\3"
#define SPECIAL_VAR_প্রতীক       3
/* The "variable" with name "\1" emits string "\3". Testcase: "echo ^C" */
#define SPECIAL_VAR_QUOTED_SVS   1

কাঠামো variable;

স্থির ধ্রুবক অক্ষর hush_version_str[] ALIGN1 = "HUSH_VERSION="BB_VER;

/* This supports saving pointers malloced in vfork child,
 * to be freed in the parent.
 */
#যদি !BB_MMU
প্রকারলেখ কাঠামো nommu_save_t {
	কাঠামো variable *old_vars;
	অক্ষর **argv;
	অক্ষর **argv_from_re_execing;
} nommu_save_t;
#endif

গণনা {
	RES_NONE  = 0,
#যদি ENABLE_HUSH_IF
	RES_IF    ,
	RES_THEN  ,
	RES_ELIF  ,
	RES_ELSE  ,
	RES_FI    ,
#endif
#যদি ENABLE_HUSH_LOOPS
	RES_FOR   ,
	RES_WHILE ,
	RES_UNTIL ,
	RES_DO    ,
	RES_DONE  ,
#endif
#যদি ENABLE_HUSH_LOOPS || ENABLE_HUSH_CASE
	RES_IN    ,
#endif
#যদি ENABLE_HUSH_CASE
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

প্রকারলেখ কাঠামো o_string {
	অক্ষর *তথ্য;
	পূর্ণসংখ্যা দৈর্ঘ্য; /* position where data is appended */
	পূর্ণসংখ্যা maxlen;
	পূর্ণসংখ্যা o_expflags;
	/* At least some part of the string was inside '' or "",
	 * possibly empty one: word"", wo''rd etc. */
	smallint has_quoted_part;
	smallint has_empty_slot;
	smallint ended_in_ifs;
} o_string;
গণনা {
	EXP_FLAG_SINGLEWORD     = 0x80, /* must be 0x80 */
	EXP_FLAG_GLOB           = 0x2,
	/* Protect newly added chars against globbing
	 * by prepending \ to *, ?, [, \ */
	EXP_FLAG_ESC_GLOB_CHARS = 0x1,
};
/* Used for initialization: o_string foo = NULL_O_STRING; */
#define NULL_O_STRING { ফাঁকা }

#ifndef debug_printf_parse
স্থির ধ্রুবক অক্ষর *ধ্রুবক assignment_flag[] ALIGN_PTR = {
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
প্রকারলেখ কাঠামো HFILE {
	অক্ষর *cur;
	অক্ষর *end;
	কাঠামো HFILE *পরবর্তী_hfile;
	পূর্ণসংখ্যা fd;
	অক্ষর buf[1024];
} HFILE;

প্রকারলেখ কাঠামো in_str {
	ধ্রুবক অক্ষর *p;
	পূর্ণসংখ্যা peek_buf[2];
	পূর্ণসংখ্যা last_char;
	HFILE *file;
} in_str;

#ifndef __U_BOOT__
/* The descrip member of this structure is only used to make
 * debugging output pretty */
স্থির ধ্রুবক কাঠামো {
	int32_t mode;
	signed অক্ষর default_fd;
	অক্ষর descrip[3];
} redir_table[] ALIGN4 = {
	{ O_RDONLY,                  0, "<"  },
	{ O_CREAT|O_TRUNC|O_WRONLY,  1, ">"  },
	{ O_CREAT|O_APPEND|O_WRONLY, 1, ">>" },
	{ O_CREAT|O_RDWR,            1, "<>" },
	{ O_RDONLY,                  0, "<<" },
/* Should not be needed. Bogus default_fd helps in debugging */
/*	{ O_RDONLY,                 77, "<<" }, */
};

কাঠামো redir_struct {
	কাঠামো redir_struct *পরবর্তী;
	অক্ষর *rd_filename;          /* filename */
	পূর্ণসংখ্যা rd_fd;                  /* fd to redirect */
	/* fd to redirect to, or -3 if rd_fd is to be closed (n>&-) */
	পূর্ণসংখ্যা rd_dup;
	smallint rd_type;           /* (enum redir_type) */
	/* note: for heredocs, rd_filename contains heredoc delimiter,
	 * and subsequently heredoc itself; and rd_dup is a bitmask:
	 * bit 0: do we need to trim leading tabs?
	 * bit 1: is heredoc quoted (<<'delim' syntax) ?
	 */
};
প্রকারলেখ গণনা redir_type {
	REDIRECT_প্রবেশ     = 0,
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

কাঠামো command {
#ifndef __U_BOOT__
	pid_t pid;                  /* 0 if exited */
#endif /* !__U_BOOT__ */
	unsigned assignment_cnt;    /* how many argv[i] are assignments? */
#যদি ENABLE_HUSH_LINENO_VAR
	unsigned lineno;
#endif
	smallint cmd_type;          /* CMD_xxx */
#define CMD_NORMAL   0
#define CMD_SUBSHELL 1
#যদি BASH_TEST2
/* used for "[[ EXPR ]]" */
# define CMD_TEST2_SINGLEWORD_NOGLOB 2
#endif
#যদি BASH_TEST2 || ENABLE_HUSH_LOCAL || ENABLE_HUSH_EXPORT || ENABLE_HUSH_READONLY
/* used to prevent word splitting and globbing in "export v=t*" */
# define CMD_SINGLEWORD_NOGLOB 3
#endif
#যদি ENABLE_HUSH_FUNCTIONS
# define CMD_FUNCDEF 4
#endif

	smalluint cmd_exitcode;
	/* if non-NULL, this "command" is { list }, ( list ), or a compound statement */
	কাঠামো pipe *group;
#যদি !BB_MMU
	অক্ষর *group_as_string;
#endif
#যদি ENABLE_HUSH_FUNCTIONS
	কাঠামো function *child_func;
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
	পূর্ণসংখ্যা argc; /* number of program arguments */
#endif
	অক্ষর **argv;                /* command name and arguments */
/* argv vector may contain variable references (^Cvar^C, ^C0^C etc)
 * and on execution these are substituted with their values.
 * Substitution can make _several_ words out of one argv[n]!
 * Example: argv[0]=='.^C*^C.' here: echo .$*.
 * References of the form ^C`cmd arg^C are `cmd arg` substitutions.
 */
#ifndef __U_BOOT__
	কাঠামো redir_struct *redirects; /* I/O redirections */
#endif /* !__U_BOOT__ */
};
/* Is there anything in this command at all? */
#ifndef __U_BOOT__
#define IS_NULL_CMD(cmd) \
	(!(cmd)->group && !(cmd)->argv && !(cmd)->redirects)

#অন্যথা /* __U_BOOT__ */
#define IS_NULL_CMD(cmd) \
	(!(cmd)->group && !(cmd)->argv)
#endif /* __U_BOOT__ */
কাঠামো pipe {
	কাঠামো pipe *পরবর্তী;
	পূর্ণসংখ্যা অঙ্ক_cmds;               /* total number of commands in pipe */
#ifndef __U_BOOT__
	পূর্ণসংখ্যা alive_cmds;             /* number of commands running (not exited) */
	পূর্ণসংখ্যা stopped_cmds;           /* number of commands alive, but stopped */
#যদি ENABLE_HUSH_JOB
	unsigned jobid;             /* job number */
	pid_t pgrp;                 /* process group ID for the job */
	অক্ষর *cmdtext;              /* name of job */
#endif
#endif /* !__U_BOOT__ */
	কাঠামো command *cmds;       /* array of commands in pipe */
	smallint followup;          /* PIPE_BG, PIPE_SEQ, PIPE_OR, PIPE_AND */
	IF_HAS_KEYWORDS(smallint pi_inverted;) /* "! cmd | cmd" */
	IF_HAS_KEYWORDS(smallint res_word;) /* needed for if, for, while, until... */
};
প্রকারলেখ গণনা pipe_style {
	PIPE_SEQ = 0,
	PIPE_AND = 1,
	PIPE_OR  = 2,
	PIPE_BG  = 3,
} pipe_style;
/* Is there anything in this pipe at all? */
#define IS_NULL_PIPE(pi) \
	((pi)->অঙ্ক_cmds == 0 IF_HAS_KEYWORDS( && (pi)->res_word == RES_NONE))

/* This holds pointers to the various results of parsing */
কাঠামো parse_context {
	/* linked list of pipes */
	কাঠামো pipe *list_শীর্ষ;
	/* last pipe (being constructed right now) */
	কাঠামো pipe *pipe;
	/* last command in pipe (being constructed right now) */
	কাঠামো command *command;
#ifndef __U_BOOT__
	/* last redirect in command->redirects list */
	কাঠামো redir_struct *pending_redirect;
#endif /* !__U_BOOT__ */
	o_string word;
#যদি !BB_MMU
	o_string as_string;
#endif
	smallint is_assignment; /* 0:maybe, 1:yes, 2:no, 3:keyword */
#যদি HAS_KEYWORDS
	smallint ctx_res_w;
	smallint ctx_inverted; /* "! cmd | cmd" */
#যদি ENABLE_HUSH_CASE
	smallint ctx_dsemicolon; /* ";;" seen */
#endif
	/* bitmask of FLAG_xxx, for figuring out valid reserved words */
	পূর্ণসংখ্যা old_flag;
	/* group we are enclosed in:
	 * example: "if pipe1; pipe2; then pipe3; fi"
	 * when we see "if" or "then", we malloc and copy current context,
	 * and make ->stack point to it. then we parse pipeN.
	 * when closing "then" / fi" / whatever is found,
	 * we move list_head into ->stack->command->group,
	 * copy ->stack into current context, and delete ->stack.
	 * (parsing of { list } and ( list ) doesn't use this method)
	 */
	কাঠামো parse_context *স্তূপ;
#endif
};
গণনা {
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
কাঠামো variable {
	কাঠামো variable *পরবর্তী;
	অক্ষর *varstr;        /* points to "name=" portion */
	পূর্ণসংখ্যা max_len;         /* if > 0, name is part of initial env; else name is malloced */
#ifndef __U_BOOT__
	uint16_t var_nest_level;
	smallint flg_export; /* putenv should be done on this var */
	smallint flg_read_only;
#endif /* !__U_BOOT__ */
};

গণনা {
	BC_BREAK = 1,
	BC_CONTINUE = 2,
};

#যদি ENABLE_HUSH_FUNCTIONS
কাঠামো function {
	কাঠামো function *পরবর্তী;
	অক্ষর *name;
	কাঠামো command *parent_cmd;
	কাঠামো pipe *body;
# যদি !BB_MMU
	অক্ষর *body_as_string;
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
স্থির ধ্রুবক অক্ষর o_opt_strings[] ALIGN1 =
	"pipefail\0"
	"noexec\0"
	"errexit\0"
#যদি ENABLE_HUSH_MODE_X
	"xtrace\0"
#endif
	;
গণনা {
	OPT_O_PIPEFAIL,
	OPT_O_NOEXEC,
	OPT_O_ERREXIT,
#যদি ENABLE_HUSH_MODE_X
	OPT_O_XTRACE,
#endif
	অঙ্ক_OPT_O
};

/* "Globals" within this file */
/* Sorted roughly by size (smaller offsets == smaller code) */
কাঠামো globals {
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
#যদি ENABLE_HUSH_INTERACTIVE
	/* 'interactive_fd' is a fd# open to ctty, if we have one
	 * _AND_ if we decided to act interactively */
	পূর্ণসংখ্যা interactive_fd;
	IF_NOT_FEATURE_EDITING_FANCY_PROMPT(অক্ষর *PS1;)
# define G_interactive_fd (G.interactive_fd)
#অন্যথা
# define G_interactive_fd 0
#endif
#অন্যথা /* __U_BOOT__ */
# define G_interactive_fd 0
#endif /* __U_BOOT__ */
#ifndef __U_BOOT__
#যদি ENABLE_FEATURE_EDITING
	সারি_প্রবেশ_t *সারি_প্রবেশ_state;
#endif
	pid_t মূল_pid;
	pid_t মূল_ppid;
	pid_t last_bg_pid;
#যদি ENABLE_HUSH_RANDOM_SUPPORT
	random_t random_gen;
#endif
#যদি ENABLE_HUSH_JOB
	পূর্ণসংখ্যা run_list_level;
	unsigned last_jobid;
	pid_t saved_tty_pgrp;
	কাঠামো pipe *job_list;
# define G_saved_tty_pgrp (G.saved_tty_pgrp)
#অন্যথা
# define G_saved_tty_pgrp 0
#endif
#endif /* !__U_BOOT__ */
	/* How deeply are we in context where "set -e" is ignored */
	পূর্ণসংখ্যা errexit_depth;
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

	অক্ষর o_opt[অঙ্ক_OPT_O];
#যদি ENABLE_HUSH_MODE_X
# define G_x_mode (G.o_opt[OPT_O_XTRACE])
#অন্যথা
# define G_x_mode 0
#endif
	অক্ষর opt_s;
	অক্ষর opt_c;
#endif /* !__U_BOOT__ */
#যদি ENABLE_HUSH_INTERACTIVE
	smallint promptmode; /* 0: PS1, 1: PS2 */
#endif
	/* set by signal handler if SIGINT is received _and_ its trap is not set */
	smallint flag_SIGINT;
#ifndef __U_BOOT__
#যদি ENABLE_HUSH_LOOPS
	smallint flag_break_continue;
#endif
#endif /* !__U_BOOT__ */
#যদি ENABLE_HUSH_FUNCTIONS
	/* 0: outside of a function (or sourced file)
	 * -1: inside of a function, ok to use return builtin
	 * 1: return is invoked, skip all till end of func
	 */
	smallint flag_return_in_progress;
# define G_flag_return_in_progress (G.flag_return_in_progress)
#অন্যথা
# define G_flag_return_in_progress 0
#endif
	smallint exiting; /* used to prevent EXIT trap recursion */
	/* These support $? */
	smalluint last_exitcode;
	smalluint expand_exitcode;
	smalluint last_bg_pid_exitcode;
#ifndef __U_BOOT__
#যদি ENABLE_HUSH_SET
	/* are global_argv and global_argv[1..n] malloced? (note: not [0]) */
	smalluint global_args_malloced;
# define G_global_args_malloced (G.global_args_malloced)
#অন্যথা
# define G_global_args_malloced 0
#endif
#যদি ENABLE_HUSH_BASH_COMPAT
	পূর্ণসংখ্যা dead_job_exitcode; /* for "wait -n" */
#endif
#endif /* !__U_BOOT__ */
	/* how many non-NULL argv's we have. NB: $# + 1 */
	পূর্ণসংখ্যা global_argc;
	অক্ষর **global_argv;
#যদি !BB_MMU
	অক্ষর *argv0_for_re_execing;
#endif
#যদি ENABLE_HUSH_LOOPS
#ifndef __U_BOOT__
	unsigned depth_break_continue;
#endif /* !__U_BOOT__ */
	unsigned depth_of_loop;
#endif
#ifndef __U_BOOT__
#যদি ENABLE_HUSH_GETOPTS
	unsigned getopt_গুনতি;
#endif
#endif /* !__U_BOOT__ */
	ধ্রুবক অক্ষর *ifs;
#ifdef __U_BOOT__
	পূর্ণসংখ্যা flag_repeat;
	পূর্ণসংখ্যা do_repeat;
	পূর্ণসংখ্যা run_command_flags;
#endif /* __U_BOOT__ */
	অক্ষর *ifs_whitespace; /* = G.ifs or malloced */
#ifndef __U_BOOT__
	ধ্রুবক অক্ষর *cwd;
#endif /* !__U_BOOT__ */
	কাঠামো variable *শীর্ষ_var;
	অক্ষর **expanded_assignments;
	কাঠামো variable **shadowed_vars_pp;
	unsigned var_nest_level;
#ifndef __U_BOOT__
#যদি ENABLE_HUSH_FUNCTIONS
# যদি ENABLE_HUSH_LOCAL
	unsigned func_nest_level; /* solely to prevent "local v" in non-functions */
# endif
	কাঠামো function *শীর্ষ_func;
#endif
	/* Signal and trap handling */
#যদি ENABLE_HUSH_FAST
	unsigned গুনতি_SIGCHLD;
	unsigned handled_SIGCHLD;
	smallint we_have_children;
#endif
#যদি ENABLE_HUSH_LINENO_VAR
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
#যদি ENABLE_HUSH_JOB
	unsigned fatal_sig_mask;
# define G_fatal_sig_mask (G.fatal_sig_mask)
#অন্যথা
# define G_fatal_sig_mask 0
#endif
#যদি ENABLE_HUSH_TRAP
	পূর্ণসংখ্যা pre_trap_exitcode;
# যদি ENABLE_HUSH_FUNCTIONS
	পূর্ণসংখ্যা return_exitcode;
# endif
	অক্ষর **traps; /* char *traps[NSIG] */
# define G_traps G.traps
#অন্যথা
# define G_traps ((অক্ষর**)ফাঁকা)
#endif
	sigset_t pending_set;
#যদি ENABLE_HUSH_MEMLEAK
	unsigned long memleak_মান;
#endif
#যদি ENABLE_HUSH_MODE_X
	unsigned x_mode_depth;
	/* "set -x" output should not be redirectable with subsequent 2>FILE.
	 * We dup fd#2 to x_mode_fd when "set -x" is executed, and use it
	 * for all subsequent output.
	 */
	পূর্ণসংখ্যা x_mode_fd;
	o_string x_mode_buf;
#endif
#endif /* !__U_BOOT__ */
#যদি HUSH_DEBUG >= 2
	পূর্ণসংখ্যা debug_indent;
#endif
#ifndef __U_BOOT__
	কাঠামো sigaction sa;
	অক্ষর optstring_buf[আকার("eixcs")];
#যদি BASH_EPOCH_VARS
	অক্ষর epoch_buf[আকার("%llu.nnnnnn") + আকার(long long)*3];
#endif
#যদি ENABLE_FEATURE_EDITING
	অক্ষর user_প্রবেশ_buf[CONFIG_FEATURE_EDITING_MAX_LEN];
#endif
#endif /* !__U_BOOT__ */
};
#ifdef __U_BOOT__
কাঠামো globals *ptr_to_globals;
#endif /* __U_BOOT__ */
#define G (*ptr_to_globals)
/* Not #defining name to G.name - this quickly gets unwieldy
 * (too many defines). Also, I actually prefer to see when a variable
 * is global, thus "G." prefix is a useful hint */
#ifdef __U_BOOT__
#define SET_PTR_TO_GLOBALS(x) করো { \
	(*(কাঠামো globals**)&ptr_to_globals) = (শূন্য*)(x); \
	barrier(); \
} যখন (0)
#define INIT_G() করো { \
	SET_PTR_TO_GLOBALS(xzalloc(আকার(G))); \
	G.promptmode = 1; \
} যখন (0)
#অন্যথা /* !__U_BOOT__ */
#define INIT_G() করো { \
	SET_PTR_TO_GLOBALS(xzalloc(আকার(G))); \
	/* memset(&G.sa, 0, sizeof(G.sa)); */  \
	sigfillset(&G.sa.sa_mask); \
	G.sa.sa_flags = SA_RESTART; \
} যখন (0)
#endif /* !__U_BOOT__ */

#ifndef __U_BOOT__
/* Function prototypes for builtins */
স্থির পূর্ণসংখ্যা builtin_cd(অক্ষর **argv) FAST_FUNC;
#যদি ENABLE_HUSH_ECHO
স্থির পূর্ণসংখ্যা builtin_echo(অক্ষর **argv) FAST_FUNC;
#endif
স্থির পূর্ণসংখ্যা builtin_eval(অক্ষর **argv) FAST_FUNC;
স্থির পূর্ণসংখ্যা builtin_exec(অক্ষর **argv) FAST_FUNC;
স্থির পূর্ণসংখ্যা builtin_exit(অক্ষর **argv) FAST_FUNC;
#যদি ENABLE_HUSH_EXPORT
স্থির পূর্ণসংখ্যা builtin_export(অক্ষর **argv) FAST_FUNC;
#endif
#যদি ENABLE_HUSH_READONLY
স্থির পূর্ণসংখ্যা builtin_readonly(অক্ষর **argv) FAST_FUNC;
#endif
স্থির পূর্ণসংখ্যা builtin_false(অক্ষর **argv) FAST_FUNC;
#যদি ENABLE_HUSH_JOB
স্থির পূর্ণসংখ্যা builtin_fg_bg(অক্ষর **argv) FAST_FUNC;
স্থির পূর্ণসংখ্যা builtin_jobs(অক্ষর **argv) FAST_FUNC;
#endif
#যদি ENABLE_HUSH_GETOPTS
স্থির পূর্ণসংখ্যা builtin_getopts(অক্ষর **argv) FAST_FUNC;
#endif
#যদি ENABLE_HUSH_HELP
স্থির পূর্ণসংখ্যা builtin_help(অক্ষর **argv) FAST_FUNC;
#endif
#যদি MAX_HISTORY && ENABLE_FEATURE_EDITING
স্থির পূর্ণসংখ্যা builtin_history(অক্ষর **argv) FAST_FUNC;
#endif
#যদি ENABLE_HUSH_LOCAL
স্থির পূর্ণসংখ্যা builtin_local(অক্ষর **argv) FAST_FUNC;
#endif
#যদি ENABLE_HUSH_MEMLEAK
স্থির পূর্ণসংখ্যা builtin_memleak(অক্ষর **argv) FAST_FUNC;
#endif
#যদি ENABLE_HUSH_PRINTF
স্থির পূর্ণসংখ্যা builtin_printf(অক্ষর **argv) FAST_FUNC;
#endif
স্থির পূর্ণসংখ্যা builtin_pwd(অক্ষর **argv) FAST_FUNC;
#যদি ENABLE_HUSH_READ
স্থির পূর্ণসংখ্যা builtin_read(অক্ষর **argv) FAST_FUNC;
#endif
#যদি ENABLE_HUSH_SET
স্থির পূর্ণসংখ্যা builtin_set(অক্ষর **argv) FAST_FUNC;
#endif
স্থির পূর্ণসংখ্যা builtin_shift(অক্ষর **argv) FAST_FUNC;
স্থির পূর্ণসংখ্যা builtin_source(অক্ষর **argv) FAST_FUNC;
#যদি ENABLE_HUSH_TEST || BASH_TEST2
স্থির পূর্ণসংখ্যা builtin_test(অক্ষর **argv) FAST_FUNC;
#endif
#যদি ENABLE_HUSH_TRAP
স্থির পূর্ণসংখ্যা builtin_trap(অক্ষর **argv) FAST_FUNC;
#endif
#যদি ENABLE_HUSH_TYPE
স্থির পূর্ণসংখ্যা builtin_type(অক্ষর **argv) FAST_FUNC;
#endif
#যদি ENABLE_HUSH_TIMES
স্থির পূর্ণসংখ্যা builtin_times(অক্ষর **argv) FAST_FUNC;
#endif
স্থির পূর্ণসংখ্যা builtin_true(অক্ষর **argv) FAST_FUNC;
#যদি ENABLE_HUSH_UMASK
স্থির পূর্ণসংখ্যা builtin_umask(অক্ষর **argv) FAST_FUNC;
#endif
#যদি ENABLE_HUSH_UNSET
স্থির পূর্ণসংখ্যা builtin_unset(অক্ষর **argv) FAST_FUNC;
#endif
#যদি ENABLE_HUSH_KILL
স্থির পূর্ণসংখ্যা builtin_kill(অক্ষর **argv) FAST_FUNC;
#endif
#যদি ENABLE_HUSH_WAIT
স্থির পূর্ণসংখ্যা builtin_wait(অক্ষর **argv) FAST_FUNC;
#endif
#যদি ENABLE_HUSH_LOOPS
স্থির পূর্ণসংখ্যা builtin_break(অক্ষর **argv) FAST_FUNC;
স্থির পূর্ণসংখ্যা builtin_continue(অক্ষর **argv) FAST_FUNC;
#endif
#যদি ENABLE_HUSH_FUNCTIONS
স্থির পূর্ণসংখ্যা builtin_return(অক্ষর **argv) FAST_FUNC;
#endif

/* Table of built-in functions.  They can be forked or not, depending on
 * context: within pipes, they fork.  As simple commands, they do not.
 * When used in non-forking context, they can change global variables
 * in the parent shell process.  If forked, of course they cannot.
 * For example, 'unset foo | whatever' will parse and run, but foo will
 * still be set at the end. */
কাঠামো built_in_command {
	ধ্রুবক অক্ষর *b_cmd;
	পূর্ণসংখ্যা (*b_function)(অক্ষর **argv) FAST_FUNC;
#যদি ENABLE_HUSH_HELP
	ধ্রুবক অক্ষর *b_descr;
# define BLTIN(cmd, func, help) { cmd, func, help }
#অন্যথা
# define BLTIN(cmd, func, help) { cmd, func }
#endif
};

স্থির ধ্রুবক কাঠামো built_in_command bltins1[] ALIGN_PTR = {
	BLTIN("."        , builtin_source  , "Run commands in file"),
	BLTIN(":"        , builtin_true    , ফাঁকা),
#যদি ENABLE_HUSH_JOB
	BLTIN("bg"       , builtin_fg_bg   , "Resume job in background"),
#endif
#যদি ENABLE_HUSH_LOOPS
	BLTIN("break"    , builtin_break   , "Exit loop"),
#endif
	BLTIN("cd"       , builtin_cd      , "Change directory"),
#যদি ENABLE_HUSH_LOOPS
	BLTIN("continue" , builtin_continue, "Start new loop iteration"),
#endif
	BLTIN("eval"     , builtin_eval    , "Construct and run shell command"),
	BLTIN("exec"     , builtin_exec    , "Execute command, don't return to shell"),
	BLTIN("exit"     , builtin_exit    , ফাঁকা),
#যদি ENABLE_HUSH_EXPORT
	BLTIN("export"   , builtin_export  , "Set environment variables"),
#endif
	BLTIN("false"    , builtin_false   , ফাঁকা),
#যদি ENABLE_HUSH_JOB
	BLTIN("fg"       , builtin_fg_bg   , "Bring job to foreground"),
#endif
#যদি ENABLE_HUSH_GETOPTS
	BLTIN("getopts"  , builtin_getopts , ফাঁকা),
#endif
#যদি ENABLE_HUSH_HELP
	BLTIN("help"     , builtin_help    , ফাঁকা),
#endif
#যদি MAX_HISTORY && ENABLE_FEATURE_EDITING
	BLTIN("history"  , builtin_history , "Show history"),
#endif
#যদি ENABLE_HUSH_JOB
	BLTIN("jobs"     , builtin_jobs    , "List jobs"),
#endif
#যদি ENABLE_HUSH_KILL
	BLTIN("kill"     , builtin_kill    , "Send signals to processes"),
#endif
#যদি ENABLE_HUSH_LOCAL
	BLTIN("local"    , builtin_local   , "Set local variables"),
#endif
#যদি ENABLE_HUSH_MEMLEAK
	BLTIN("memleak"  , builtin_memleak , ফাঁকা),
#endif
#যদি ENABLE_HUSH_READ
	BLTIN("read"     , builtin_read    , "Input into variable"),
#endif
#যদি ENABLE_HUSH_READONLY
	BLTIN("readonly" , builtin_readonly, "Make variables read-only"),
#endif
#যদি ENABLE_HUSH_FUNCTIONS
	BLTIN("return"   , builtin_return  , "Return from function"),
#endif
#যদি ENABLE_HUSH_SET
	BLTIN("set"      , builtin_set     , "Set positional parameters"),
#endif
	BLTIN("shift"    , builtin_shift   , "Shift positional parameters"),
#যদি BASH_SOURCE
	BLTIN("source"   , builtin_source  , ফাঁকা),
#endif
#যদি ENABLE_HUSH_TIMES
	BLTIN("times"    , builtin_times   , ফাঁকা),
#endif
#যদি ENABLE_HUSH_TRAP
	BLTIN("trap"     , builtin_trap    , "Trap signals"),
#endif
	BLTIN("true"     , builtin_true    , ফাঁকা),
#যদি ENABLE_HUSH_TYPE
	BLTIN("type"     , builtin_type    , "Show command type"),
#endif
#যদি ENABLE_HUSH_ULIMIT
	BLTIN("ulimit"   , shell_builtin_ulimit, "Control resource limits"),
#endif
#যদি ENABLE_HUSH_UMASK
	BLTIN("umask"    , builtin_umask   , "Set file creation mask"),
#endif
#যদি ENABLE_HUSH_UNSET
	BLTIN("unset"    , builtin_unset   , "Unset variables"),
#endif
#যদি ENABLE_HUSH_WAIT
	BLTIN("wait"     , builtin_wait    , "Wait for process to finish"),
#endif
};
/* These builtins won't be used if we are on NOMMU and need to re-exec
 * (it's cheaper to run an external program in this case):
 */
স্থির ধ্রুবক কাঠামো built_in_command bltins2[] ALIGN_PTR = {
#যদি ENABLE_HUSH_TEST
	BLTIN("["        , builtin_test    , ফাঁকা),
#endif
#যদি BASH_TEST2
	BLTIN("[["       , builtin_test    , ফাঁকা),
#endif
#যদি ENABLE_HUSH_ECHO
	BLTIN("echo"     , builtin_echo    , ফাঁকা),
#endif
#যদি ENABLE_HUSH_PRINTF
	BLTIN("printf"   , builtin_printf  , ফাঁকা),
#endif
	BLTIN("pwd"      , builtin_pwd     , ফাঁকা),
#যদি ENABLE_HUSH_TEST
	BLTIN("test"     , builtin_test    , ফাঁকা),
#endif
};

#endif /* !__U_BOOT__ */

/* Debug printouts.
 */
#যদি HUSH_DEBUG >= 2
/* prevent disasters with G.debug_indent < 0 */
# define indent() fdprintf(2, "%*s", (G.debug_indent * 2) & 0xff, "")
# define debug_enter() (G.debug_indent++)
# define debug_leave() (G.debug_indent--)
#অন্যথা
# define indent()      ((শূন্য)0)
# define debug_enter() ((শূন্য)0)
# define debug_leave() ((শূন্য)0)
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
#অন্যথা
# define DEBUG_JOBS 0
#endif

#ifndef debug_printf_expand
# define debug_printf_expand(...) (indent(), fdprintf(2, __VA_ARGS__))
# define DEBUG_EXPAND 1
#অন্যথা
# define DEBUG_EXPAND 0
#endif

#ifndef debug_printf_varexp
# define debug_printf_varexp(...) (indent(), fdprintf(2, __VA_ARGS__))
#endif

#ifndef debug_printf_glob
# define debug_printf_glob(...) (indent(), fdprintf(2, __VA_ARGS__))
# define DEBUG_GLOB 1
#অন্যথা
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
#অন্যথা
# define DEBUG_CLEAN 0
#endif

#যদি DEBUG_EXPAND
স্থির শূন্য debug_print_strings(ধ্রুবক অক্ষর *prefix, অক্ষর **vv)
{
	indent();
	fdprintf(2, "%s:\n", prefix);
	যখন (*vv)
		fdprintf(2, " '%s'\n", *vv++);
}
#অন্যথা
# define debug_print_strings(prefix, vv) ((শূন্য)0)
#endif

/* Leak hunting. Use hush_leaktool.sh for post-processing.
 */
#যদি LEAK_HUNTING
স্থির শূন্য *xxmalloc(পূর্ণসংখ্যা lineno, size_t আকার_2)
{
	শূন্য *ptr = xmalloc((আকার_2 + 0xff) & ~0xff);
	fdprintf(2, "line %d: malloc %p\n", lineno, ptr);
	ফেরত ptr;
}
স্থির শূন্য *xxrealloc(পূর্ণসংখ্যা lineno, শূন্য *ptr, size_t আকার_2)
{
	ptr = xrealloc(ptr, (আকার_2 + 0xff) & ~0xff);
	fdprintf(2, "line %d: realloc %p\n", lineno, ptr);
	ফেরত ptr;
}
স্থির অক্ষর *xxstrdup(পূর্ণসংখ্যা lineno, ধ্রুবক অক্ষর *str)
{
	অক্ষর *ptr = xstrdup(str);
	fdprintf(2, "line %d: strdup %p\n", lineno, ptr);
	ফেরত ptr;
}
স্থির শূন্য xxfree(শূন্য *ptr)
{
	fdprintf(2, "free %p\n", ptr);
	মুক্ত(ptr);
}
# define xmalloc(s)     xxmalloc(__LINE__, s)
# define xrealloc(p, s) xxrealloc(__LINE__, p, s)
# define xstrdup(s)     xxstrdup(__LINE__, s)
# define মুক্ত(p)        xxfree(p)
#endif

/* Syntax and runtime errors. They always abort scripts.
 * In interactive use they usually discard unparsed and/or unexecuted commands
 * and return to the prompt.
 * HUSH_DEBUG >= 2 prints line number in this file where it was detected.
 */
#যদি HUSH_DEBUG < 2
# define বার্তা_and_die_if_script(lineno, ...)     বার্তা_and_die_if_script(__VA_ARGS__)
# define syntax_error(lineno, বার্তা)              syntax_error(বার্তা)
# define syntax_error_at(lineno, বার্তা)           syntax_error_at(বার্তা)
# define syntax_error_unterm_ch(lineno, ch)     syntax_error_unterm_ch(ch)
# define syntax_error_unterm_str(lineno, s)     syntax_error_unterm_str(s)
# define syntax_error_unexpected_ch(lineno, ch) syntax_error_unexpected_ch(ch)
#endif

স্থির শূন্য die_if_script(শূন্য)
{
	যদি (!G_interactive_fd) {
		যদি (G.last_exitcode) /* sometines it's 2, not 1 (bash compat) */
			xfunc_error_retval = G.last_exitcode;
		xfunc_die();
	}
}

#ifdef __U_BOOT__
স্থির শূন্য __maybe_unused বার্তা_and_die_if_script(unsigned lineno, ধ্রুবক অক্ষর *fmt, ...)
#অন্যথা /* !__U_BOOT__ */
স্থির শূন্য বার্তা_and_die_if_script(unsigned lineno, ধ্রুবক অক্ষর *fmt, ...)
#endif /* !__U_BOOT__ */
{
	va_list p;

#যদি HUSH_DEBUG >= 2
	bb_error_বার্তা("hush.c:%u", lineno);
#endif
	va_start(p, fmt);
	bb_verror_বার্তা(fmt, p, ফাঁকা);
	va_end(p);
	die_if_script();
}

স্থির শূন্য syntax_error(unsigned lineno UNUSED_PARAM, ধ্রুবক অক্ষর *বার্তা)
{
	যদি (বার্তা)
		bb_error_বার্তা("syntax error: %s", বার্তা);
	অন্যথা
		bb_simple_error_বার্তা("syntax error");
	die_if_script();
}

স্থির শূন্য syntax_error_at(unsigned lineno UNUSED_PARAM, ধ্রুবক অক্ষর *বার্তা)
{
	bb_error_বার্তা("syntax error at '%s'", বার্তা);
	die_if_script();
}

স্থির শূন্য syntax_error_unterm_str(unsigned lineno UNUSED_PARAM, ধ্রুবক অক্ষর *s)
{
	bb_error_বার্তা("syntax error: unterminated %s", s);
//? source4.tests fails: in bash, echo ${^} in script does not terminate the script
// (but bash --posix, or if bash is run as "sh", does terminate in script, so maybe uncomment this?)
//	die_if_script();
}

স্থির শূন্য syntax_error_unterm_ch(unsigned lineno, অক্ষর ch)
{
	অক্ষর বার্তা[2] = { ch, '\0' };
	syntax_error_unterm_str(lineno, বার্তা);
}

স্থির শূন্য syntax_error_unexpected_ch(unsigned lineno UNUSED_PARAM, পূর্ণসংখ্যা ch)
{
	অক্ষর বার্তা[2];
	বার্তা[0] = ch;
	বার্তা[1] = '\0';
#যদি HUSH_DEBUG >= 2
	bb_error_বার্তা("hush.c:%u", lineno);
#endif
	bb_error_বার্তা("syntax error: unexpected %s", ch == EOF ? "EOF" : বার্তা);
	die_if_script();
}

#যদি HUSH_DEBUG < 2
# undef বার্তা_and_die_if_script
# undef syntax_error
# undef syntax_error_at
# undef syntax_error_unterm_ch
# undef syntax_error_unterm_str
# undef syntax_error_unexpected_ch
#অন্যথা
# define বার্তা_and_die_if_script(...)     বার্তা_and_die_if_script(__LINE__, __VA_ARGS__)
# define syntax_error(বার্তা)              syntax_error(__LINE__, বার্তা)
# define syntax_error_at(বার্তা)           syntax_error_at(__LINE__, বার্তা)
# define syntax_error_unterm_ch(ch)     syntax_error_unterm_ch(__LINE__, ch)
# define syntax_error_unterm_str(s)     syntax_error_unterm_str(__LINE__, s)
# define syntax_error_unexpected_ch(ch) syntax_error_unexpected_ch(__LINE__, ch)
#endif

/* Utility functions
 */
/* Replace each \x with x in place, return ptr past NUL. */
স্থির অক্ষর *unbackslash(অক্ষর *src)
{
#ifdef __U_BOOT__
	অক্ষর *dst = src = (অক্ষর *)strchrnul(src, '\\');
#অন্যথা /* !__U_BOOT__ */
	অক্ষর *dst = src = strchrnul(src, '\\');
#endif /* !__U_BOOT__ */
	যখন (1) {
		যদি (*src == '\\') {
			src++;
			যদি (*src != '\0') {
				/* \x -> x */
				*dst++ = *src++;
				চালু;
			}
			/* else: "\<nul>". Do not delete this backslash.
			 * Testcase: eval 'echo ok\'
			 */
			*dst++ = '\\';
			/* fallthrough */
		}
		যদি ((*dst++ = *src++) == '\0')
			বিরতি;
	}
	ফেরত dst;
}

স্থির অক্ষর **add_strings_to_strings(অক্ষর **strings, অক্ষর **add, পূর্ণসংখ্যা need_to_dup)
{
	পূর্ণসংখ্যা i;
	unsigned count1;
	unsigned count2;
	অক্ষর **v;

	v = strings;
	count1 = 0;
	যদি (v) {
		যখন (*v) {
			count1++;
			v++;
		}
	}
	count2 = 0;
	v = add;
	যখন (*v) {
		count2++;
		v++;
	}
	v = xrealloc(strings, (count1 + count2 + 1) * আকার(অক্ষর*));
	v[count1 + count2] = ফাঁকা;
	i = count2;
	যখন (--i >= 0)
		v[count1 + i] = (need_to_dup ? xstrdup(add[i]) : add[i]);
	ফেরত v;
}
#যদি LEAK_HUNTING
স্থির অক্ষর **xx_add_strings_to_strings(পূর্ণসংখ্যা lineno, অক্ষর **strings, অক্ষর **add, পূর্ণসংখ্যা need_to_dup)
{
	অক্ষর **ptr = add_strings_to_strings(strings, add, need_to_dup);
	fdprintf(2, "line %d: add_strings_to_strings %p\n", lineno, ptr);
	ফেরত ptr;
}
#define add_strings_to_strings(strings, add, need_to_dup) \
	xx_add_strings_to_strings(__LINE__, strings, add, need_to_dup)
#endif

/* Note: takes ownership of "add" ptr (it is not strdup'ed) */
স্থির অক্ষর **add_string_to_strings(অক্ষর **strings, অক্ষর *add)
{
	অক্ষর *v[2];
	v[0] = add;
	v[1] = ফাঁকা;
	ফেরত add_strings_to_strings(strings, v, /*dup:*/ 0);
}

#যদি LEAK_HUNTING
স্থির অক্ষর **xx_add_string_to_strings(পূর্ণসংখ্যা lineno, অক্ষর **strings, অক্ষর *add)
{
	অক্ষর **ptr = add_string_to_strings(strings, add);
	fdprintf(2, "line %d: add_string_to_strings %p\n", lineno, ptr);
	ফেরত ptr;
}
#define add_string_to_strings(strings, add) \
	xx_add_string_to_strings(__LINE__, strings, add)
#endif

স্থির শূন্য free_strings(অক্ষর **strings)
{
	অক্ষর **v;

	যদি (!strings)
		ফেরত;
	v = strings;
	যখন (*v) {
		মুক্ত(*v);
		v++;
	}
	মুক্ত(strings);
}

#ifndef __U_BOOT__
স্থির পূর্ণসংখ্যা dup_CLOEXEC(পূর্ণসংখ্যা fd, পূর্ণসংখ্যা avoid_fd)
{
	পূর্ণসংখ্যা newfd;
 repeat:
	newfd = fcntl(fd, F_DUPFD_CLOEXEC, avoid_fd + 1);
	যদি (newfd >= 0) {
		যদি (F_DUPFD_CLOEXEC == F_DUPFD) /* if old libc (w/o F_DUPFD_CLOEXEC) */
			close_on_exec_on(newfd);
	} অন্যথা { /* newfd < 0 */
		যদি (errno == EBUSY)
			যাও repeat;
		যদি (errno == EINTR)
			যাও repeat;
		যদি (errno != EBADF) {
			/* "echo >&9999" gets EINVAL trying to save fd 1 to above 9999.
			 * We could try saving it _below_ 9999 instead (how?), but
			 * this probably means that dup2(9999,1) to effectuate >&9999
			 * would also not work: fd 9999 can't exist.
			 * (This differs from "echo >&99" where saving works, but
			 * subsequent dup2(99,1) fails if fd 99 is not open).
			 */
			bb_perror_বার্তা("fcntl(%d,F_DUPFD,%d)", fd, avoid_fd + 1);
		}
	}
	ফেরত newfd;
}

স্থির পূর্ণসংখ্যা xdup_CLOEXEC_and_close(পূর্ণসংখ্যা fd, পূর্ণসংখ্যা avoid_fd)
{
	পূর্ণসংখ্যা newfd;
 repeat:
	newfd = fcntl(fd, F_DUPFD_CLOEXEC, avoid_fd + 1);
	যদি (newfd < 0) {
		যদি (errno == EBUSY)
			যাও repeat;
		যদি (errno == EINTR)
			যাও repeat;
		/* fd was not open? */
		যদি (errno == EBADF)
			ফেরত fd;
		xfunc_die();
	}
	যদি (F_DUPFD_CLOEXEC == F_DUPFD) /* if old libc (w/o F_DUPFD_CLOEXEC) */
		close_on_exec_on(newfd);
	close(fd);
	ফেরত newfd;
}

/* Manipulating HFILEs */
স্থির HFILE *hfopen(ধ্রুবক অক্ষর *name)
{
	HFILE *fp;
	পূর্ণসংখ্যা fd;

	fd = STDIN_FILENO;
	যদি (name) {
		fd = open(name, O_RDONLY | O_CLOEXEC);
		যদি (fd < 0)
			ফেরত ফাঁকা;
		যদি (O_CLOEXEC == 0) /* ancient libc */
			close_on_exec_on(fd);
	}

	fp = xmalloc(আকার(*fp));
	যদি (name == ফাঁকা)
		G.HFILE_stdin = fp;
	fp->fd = fd;
	fp->cur = fp->end = fp->buf;
	fp->পরবর্তী_hfile = G.HFILE_list;
	G.HFILE_list = fp;
	ফেরত fp;
}
স্থির শূন্য hfclose(HFILE *fp)
{
	HFILE **pp = &G.HFILE_list;
	যখন (*pp) {
		HFILE *cur = *pp;
		যদি (cur == fp) {
			*pp = cur->পরবর্তী_hfile;
			বিরতি;
		}
		pp = &cur->পরবর্তী_hfile;
	}
	যদি (fp->fd >= 0)
		close(fp->fd);
	মুক্ত(fp);
}
স্থির পূর্ণসংখ্যা refill_HFILE_and_getc(HFILE *fp)
{
	পূর্ণসংখ্যা n;

	যদি (fp->fd < 0) {
		/* Already saw EOF */
		ফেরত EOF;
	}
#যদি ENABLE_HUSH_INTERACTIVE && !ENABLE_FEATURE_EDITING
	/* If user presses ^C, read() restarts after SIGINT (we use SA_RESTART).
	 * IOW: ^C will not immediately stop line input.
	 * But poll() is different: it does NOT restart after signals.
	 */
	যদি (fp == G.HFILE_stdin) {
		কাঠামো pollfd pfd[1];
		pfd[0].fd = fp->fd;
		pfd[0].events = POLLIN;
		n = poll(pfd, 1, -1);
		যদি (n < 0
		 /*&& errno == EINTR - assumed true */
		 && sigismember(&G.pending_set, SIGINT)
		) {
			ফেরত '\0';
		}
	}
#অন্যথা
/* if FEATURE_EDITING=y, we do not use this routine for interactive input */
#endif
	/* Try to buffer more input */
	n = safe_read(fp->fd, fp->buf, আকার(fp->buf));
	যদি (n < 0) {
		bb_simple_perror_বার্তা("read error");
		n = 0;
	}
	fp->cur = fp->buf;
	fp->end = fp->buf + n;
	যদি (n == 0) {
		/* EOF/error */
		close(fp->fd);
		fp->fd = -1;
		ফেরত EOF;
	}
	ফেরত (unsigned অক্ষর)(*fp->cur++);
}
/* Inlined for common case of non-empty buffer.
 */
স্থির ALWAYS_INLINE পূর্ণসংখ্যা hfgetc(HFILE *fp)
{
	যদি (fp->cur < fp->end)
		ফেরত (unsigned অক্ষর)(*fp->cur++);
	/* Buffer empty */
	ফেরত refill_HFILE_and_getc(fp);
}
স্থির পূর্ণসংখ্যা move_HFILEs_on_redirect(পূর্ণসংখ্যা fd, পূর্ণসংখ্যা avoid_fd)
{
	HFILE *fl = G.HFILE_list;
	যখন (fl) {
		যদি (fd == fl->fd) {
			/* We use it only on script files, they are all CLOEXEC */
			fl->fd = xdup_CLOEXEC_and_close(fd, avoid_fd);
			debug_printf_redir("redirect_fd %d: matches a script fd, moving it to %d\n", fd, fl->fd);
			ফেরত 1; /* "found and moved" */
		}
		fl = fl->পরবর্তী_hfile;
	}
#যদি ENABLE_HUSH_MODE_X
	যদি (G.x_mode_fd > 0 && fd == G.x_mode_fd) {
		G.x_mode_fd = xdup_CLOEXEC_and_close(fd, avoid_fd);
		ফেরত 1; /* "found and moved" */
	}
#endif
	ফেরত 0; /* "not in the list" */
}
#যদি ENABLE_FEATURE_SH_STANDALONE && BB_MMU
স্থির শূন্য close_all_HFILE_list(শূন্য)
{
	HFILE *fl = G.HFILE_list;
	যখন (fl) {
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
		যদি (fl->fd > 0)
			/*hfclose(fl); - unsafe */
			close(fl->fd);
		fl = fl->পরবর্তী_hfile;
	}
}
#endif
স্থির পূর্ণসংখ্যা fd_in_HFILEs(পূর্ণসংখ্যা fd)
{
	HFILE *fl = G.HFILE_list;
	যখন (fl) {
		যদি (fl->fd == fd)
			ফেরত 1;
		fl = fl->পরবর্তী_hfile;
	}
	ফেরত 0;
}

#endif /* !__U_BOOT__ */

/* Helpers for setting new $n and restoring them back
 */
প্রকারলেখ কাঠামো save_arg_t {
	অক্ষর *sv_argv0;
	অক্ষর **sv_g_argv;
	পূর্ণসংখ্যা sv_g_argc;
#ifndef __U_BOOT__
	IF_HUSH_SET(smallint sv_g_malloced;)
#endif /* !__U_BOOT__ */
} save_arg_t;

#ifndef __U_BOOT__
স্থির শূন্য save_and_replace_G_args(save_arg_t *sv, অক্ষর **argv)
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

স্থির শূন্য restore_G_args(save_arg_t *sv, অক্ষর **argv)
{
#যদি ENABLE_HUSH_SET
	যদি (G.global_args_malloced) {
		/* someone ran "set -- arg1 arg2 ...", undo */
		অক্ষর **pp = G.global_argv;
		যখন (*++pp) /* note: does not free $0 */
			মুক্ত(*pp);
		মুক্ত(G.global_argv);
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
গণনা {
	SPECIAL_INTERACTIVE_SIGS = 0
		| (1 << SIGTERM)
		| (1 << SIGINT)
		| (1 << SIGHUP)
		,
	SPECIAL_JOBSTOP_SIGS = 0
#যদি ENABLE_HUSH_JOB
		| (1 << SIGTTIN)
		| (1 << SIGTTOU)
		| (1 << SIGTSTP)
#endif
		,
};

স্থির শূন্য record_pending_signo(পূর্ণসংখ্যা sig)
{
	sigaddset(&G.pending_set, sig);
#যদি ENABLE_FEATURE_EDITING
	যদি (sig != SIGCHLD
	 || (G_traps && G_traps[SIGCHLD] && G_traps[SIGCHLD][0])
	 /* ^^^ if SIGCHLD, interrupt line reading only if it has a trap */
	) {
		bb_got_signal = sig; /* for read_line_input: "we got a signal" */
	}
#endif
#যদি ENABLE_HUSH_FAST
	যদি (sig == SIGCHLD) {
		G.গুনতি_SIGCHLD++;
//bb_error_msg("[%d] SIGCHLD_handler: G.count_SIGCHLD:%d G.handled_SIGCHLD:%d", getpid(), G.count_SIGCHLD, G.handled_SIGCHLD);
	}
#endif
}

স্থির sighandler_t install_sighandler(পূর্ণসংখ্যা sig, sighandler_t handler)
{
	কাঠামো sigaction old_sa;

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
	ফেরত old_sa.sa_handler;
}
#endif /* !__U_BOOT__ */

#ifndef __U_BOOT__
স্থির শূন্য hush_exit(পূর্ণসংখ্যা exitcode) NORETURN;

স্থির শূন্য restore_ttypgrp_and__exit(শূন্য) NORETURN;
স্থির শূন্য restore_ttypgrp_and__exit(শূন্য)
{
	/* xfunc has failed! die die die */
	/* no EXIT traps, this is an escape hatch! */
	G.exiting = 1;
	hush_exit(xfunc_error_retval);
}

#যদি ENABLE_HUSH_JOB

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
স্থির শূন্য fflush_and__exit(শূন্য) NORETURN;
স্থির শূন্য fflush_and__exit(শূন্য)
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
স্থির শূন্য sigexit(পূর্ণসংখ্যা sig) NORETURN;
স্থির শূন্য sigexit(পূর্ণসংখ্যা sig)
{
	/* Careful: we can end up here after [v]fork. Do not restore
	 * tty pgrp then, only top-level shell process does that */
	যদি (G_saved_tty_pgrp && getpid() == G.মূল_pid) {
		/* Disable all signals: job control, SIGPIPE, etc.
		 * Mostly paranoid measure, to prevent infinite SIGTTOU.
		 */
		sigprocmask_allsigs(SIG_BLOCK);
		tcsetpgrp(G_interactive_fd, G_saved_tty_pgrp);
	}

	/* Not a signal, just exit */
	যদি (sig <= 0)
		_exit(- sig);

	kill_myself_with_sig(sig); /* does not return */
}
#অন্যথা

# define disable_restore_tty_pgrp_on_exit() ((শূন্য)0)
# define enable_restore_tty_pgrp_on_exit()  ((শূন্য)0)

#endif
#endif /* !__U_BOOT__ */

#ifndef __U_BOOT__
স্থির sighandler_t pick_sighandler(unsigned sig)
{
	sighandler_t handler = SIG_DFL;
	যদি (sig < আকার(unsigned)*8) {
		unsigned sigmask = (1 << sig);

#যদি ENABLE_HUSH_JOB
		/* is sig fatal? */
		যদি (G_fatal_sig_mask & sigmask)
			handler = sigexit;
		অন্যথা
#endif
		/* sig has special handling? */
		যদি (G.special_sig_mask & sigmask) {
			handler = record_pending_signo;
			/* TTIN/TTOU/TSTP can't be set to record_pending_signo
			 * in order to ignore them: they will be raised
			 * in an endless loop when we try to do some
			 * terminal ioctls! We do have to _ignore_ these.
			 */
			যদি (SPECIAL_JOBSTOP_SIGS & sigmask)
				handler = SIG_IGN;
		}
	}
	ফেরত handler;
}
#endif /* !__U_BOOT__ */

#ifndef __U_BOOT__
/* Restores tty foreground process group, and exits. */
স্থির শূন্য hush_exit(পূর্ণসংখ্যা exitcode)
{
#যদি ENABLE_FEATURE_EDITING_SAVE_ON_EXIT
	save_history(G.সারি_প্রবেশ_state); /* may be NULL */
#endif

	fflush_all();
	যদি (G.exiting <= 0 && G_traps && G_traps[0] && G_traps[0][0]) {
		অক্ষর *argv[3];
		/* argv[0] is unused */
		argv[1] = xstrdup(G_traps[0]); /* copy, since EXIT trap handler may modify G_traps[0] */
		argv[2] = ফাঁকা;
		G.exiting = 1; /* prevent EXIT trap recursion */
		/* Note: G_traps[0] is not cleared!
		 * "trap" will still show it, if executed
		 * in the handler */
		builtin_eval(argv);
	}

#যদি ENABLE_FEATURE_CLEAN_UP
	{
		কাঠামো variable *cur_var;
		যদি (G.cwd != bb_বার্তা_unknown)
			মুক্ত((অক্ষর*)G.cwd);
		cur_var = G.শীর্ষ_var;
		যখন (cur_var) {
			কাঠামো variable *tmp = cur_var;
			যদি (!cur_var->max_len)
				মুক্ত(cur_var->varstr);
			cur_var = cur_var->পরবর্তী;
			মুক্ত(tmp);
		}
	}
#endif

	fflush_all();
#যদি ENABLE_HUSH_JOB
	sigexit(- (exitcode & 0xff));
#অন্যথা
	_exit(exitcode);
#endif
}

//TODO: return a mask of ALL handled sigs?
স্থির পূর্ণসংখ্যা check_and_run_traps(শূন্য)
{
	পূর্ণসংখ্যা last_sig = 0;

	যখন (1) {
		পূর্ণসংখ্যা sig;

		যদি (sigisemptyset(&G.pending_set))
			বিরতি;
		sig = 0;
		করো {
			sig++;
			যদি (sigismember(&G.pending_set, sig)) {
				sigdelset(&G.pending_set, sig);
				যাও got_sig;
			}
		} যখন (sig < NSIG);
		বিরতি;
 got_sig:
#যদি ENABLE_HUSH_TRAP
		যদি (G_traps && G_traps[sig]) {
			debug_printf_exec("%s: sig:%d handler:'%s'\n", __func__, sig, G.traps[sig]);
			যদি (G_traps[sig][0]) {
				/* We have user-defined handler */
				smalluint save_rcode;
				পূর্ণসংখ্যা save_pre;
				অক্ষর *argv[3];
				/* argv[0] is unused */
				argv[1] = xstrdup(G_traps[sig]);
				/* why strdup? trap can modify itself: trap 'trap "echo oops" INT' INT */
				argv[2] = ফাঁকা;
				save_pre = G.pre_trap_exitcode;
				G.pre_trap_exitcode = save_rcode = G.last_exitcode;
				builtin_eval(argv);
				মুক্ত(argv[1]);
				G.pre_trap_exitcode = save_pre;
				G.last_exitcode = save_rcode;
# যদি ENABLE_HUSH_FUNCTIONS
				যদি (G.return_exitcode >= 0) {
					debug_printf_exec("trap exitcode:%d\n", G.return_exitcode);
					G.last_exitcode = G.return_exitcode;
				}
# endif
				last_sig = sig;
			} /* else: "" trap, ignoring signal */
			চালু;
		}
#endif
		/* not a trap: special action */
		নির্বাচন (sig) {
		ক্ষেত্র SIGINT:
			debug_printf_exec("%s: sig:%d default SIGINT handler\n", __func__, sig);
			G.flag_SIGINT = 1;
			last_sig = sig;
			বিরতি;
#যদি ENABLE_HUSH_JOB
		ক্ষেত্র SIGHUP: {
			/* if (G_interactive_fd) - no need to check, the handler
			 * is only installed if we *are* interactive */
			{
				/* bash compat: "Before exiting, an interactive
				 * shell resends the SIGHUP to all jobs, running
				 * or stopped.  Stopped jobs are sent SIGCONT
				 * to ensure that they receive the SIGHUP."
				 */
				কাঠামো pipe *job;
				debug_printf_exec("%s: sig:%d default SIGHUP handler\n", __func__, sig);
				/* bash is observed to signal whole process groups,
				 * not individual processes */
				জন্য (job = G.job_list; job; job = job->পরবর্তী) {
					যদি (job->pgrp <= 0)
						চালু;
					debug_printf_exec("HUPing pgrp %d\n", job->pgrp);
					যদি (kill(- job->pgrp, SIGHUP) == 0)
						kill(- job->pgrp, SIGCONT);
				}
			}
			/* this restores tty pgrp, then kills us with SIGHUP */
			sigexit(SIGHUP);
		}
#endif
#যদি ENABLE_HUSH_FAST
		ক্ষেত্র SIGCHLD:
			debug_printf_exec("%s: sig:%d default SIGCHLD handler\n", __func__, sig);
			G.গুনতি_SIGCHLD++;
//bb_error_msg("[%d] check_and_run_traps: G.count_SIGCHLD:%d G.handled_SIGCHLD:%d", getpid(), G.count_SIGCHLD, G.handled_SIGCHLD);
			/* Note:
			 * We don't do 'last_sig = sig' here -> NOT returning this sig.
			 * This simplifies wait builtin a bit.
			 */
			বিরতি;
#endif
		পূর্বনির্ধারিত: /* ignored: */
			debug_printf_exec("%s: sig:%d default handling is to ignore\n", __func__, sig);
			/* SIGTERM, SIGQUIT, SIGTTIN, SIGTTOU, SIGTSTP */
			/* Note:
			 * We don't do 'last_sig = sig' here -> NOT returning this sig.
			 * Example: wait is not interrupted by TERM
			 * in interactive shell, because TERM is ignored.
			 */
			বিরতি;
		}
	}
	ফেরত last_sig;
}

স্থির ধ্রুবক অক্ষর *get_cwd(পূর্ণসংখ্যা force)
{
	যদি (force || G.cwd == ফাঁকা) {
		/* xrealloc_getcwd_or_warn(arg) calls free(arg),
		 * we must not try to free(bb_msg_unknown) */
		যদি (G.cwd == bb_বার্তা_unknown)
			G.cwd = ফাঁকা;
		G.cwd = xrealloc_getcwd_or_warn((অক্ষর *)G.cwd);
		যদি (!G.cwd)
			G.cwd = bb_বার্তা_unknown;
	}
	ফেরত G.cwd;
}

#endif /* !__U_BOOT__ */

/*
 * Shell and environment variable support
 */
স্থির কাঠামো variable **get_ptr_to_local_var(ধ্রুবক অক্ষর *name)
{
	কাঠামো variable **pp;
	কাঠামো variable *cur;

	pp = &G.শীর্ষ_var;
	যখন ((cur = *pp) != ফাঁকা) {
		যদি (varcmp(cur->varstr, name) == 0)
			ফেরত pp;
		pp = &cur->পরবর্তী;
	}
	ফেরত ফাঁকা;
}

স্থির ধ্রুবক অক্ষর* FAST_FUNC get_local_var_মান(ধ্রুবক অক্ষর *name)
{
	কাঠামো variable **vpp;

	যদি (G.expanded_assignments) {
		অক্ষর **cpp = G.expanded_assignments;
		যখন (*cpp) {
			অক্ষর *cp = *cpp;
			যদি (varcmp(cp, name) == 0)
				ফেরত strchr(cp, '=') + 1;
			cpp++;
		}
	}

	vpp = get_ptr_to_local_var(name);
	যদি (vpp)
		ফেরত strchr((*vpp)->varstr, '=') + 1;

#ifndef __U_BOOT__
	যদি (strcmp(name, "PPID") == 0)
		ফেরত utoa(G.মূল_ppid);
#endif /* !__U_BOOT__ */
	// bash compat: UID? EUID?
#যদি ENABLE_HUSH_RANDOM_SUPPORT
	যদি (strcmp(name, "RANDOM") == 0)
		ফেরত utoa(পরবর্তী_random(&G.random_gen));
#endif
#যদি ENABLE_HUSH_LINENO_VAR
	যদি (strcmp(name, "LINENO") == 0)
		ফেরত utoa(G.execute_lineno);
#endif
#যদি BASH_EPOCH_VARS
	{
		ধ্রুবক অক্ষর *fmt = ফাঁকা;
		যদি (strcmp(name, "EPOCHSECONDS") == 0)
			fmt = "%llu";
		অন্যথা যদি (strcmp(name, "EPOCHREALTIME") == 0)
			fmt = "%llu.%06u";
		যদি (fmt) {
			কাঠামো timeval tv;
			xgettimeofday(&tv);
			sprintf(G.epoch_buf, fmt, (unsigned long long)tv.tv_sec,
					(unsigned)tv.tv_usec);
			ফেরত G.epoch_buf;
		}
	}
#endif
	ফেরত ফাঁকা;
}

#ifndef __U_BOOT__
#যদি ENABLE_HUSH_GETOPTS
স্থির শূন্য handle_changed_special_names(ধ্রুবক অক্ষর *name)
{
	যদি (varcmp(name, "OPTIND") == 0) {
		G.getopt_গুনতি = 0;
		ফেরত;
	}
}
#অন্যথা
/* Do not even bother evaluating arguments */
# define handle_changed_special_names(...) ((শূন্য)0)
#endif
#অন্যথা /* __U_BOOT__ */
/* Do not even bother evaluating arguments */
# define handle_changed_special_names(...) ((শূন্য)0)
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
স্থির পূর্ণসংখ্যা set_local_var(অক্ষর *str, unsigned flags)
#অন্যথা /* __U_BOOT__ */
পূর্ণসংখ্যা set_local_var_modern(অক্ষর *str, পূর্ণসংখ্যা flags)
#endif /* __U_BOOT__ */
{
	কাঠামো variable **cur_pp;
	কাঠামো variable *cur;
	অক্ষর *free_me = ফাঁকা;
	অক্ষর *eq_sign;
	পূর্ণসংখ্যা name_len;
	পূর্ণসংখ্যা retval;
#ifndef __U_BOOT__
	unsigned local_lvl = (flags >> SETFLAG_VARLVL_SHIFT);
#endif /* !__U_BOOT__ */

	eq_sign = strchr(str, '=');
	যদি (HUSH_DEBUG && !eq_sign)
		bb_simple_error_বার্তা_and_die("BUG in setvar");

	name_len = eq_sign - str + 1; /* including '=' */
	cur_pp = &G.শীর্ষ_var;
	যখন ((cur = *cur_pp) != ফাঁকা) {
		যদি (strncmp(cur->varstr, str, name_len) != 0) {
			cur_pp = &cur->পরবর্তী;
			চালু;
		}

#ifndef __U_BOOT__
		/* We found an existing var with this name */
		যদি (cur->flg_read_only) {
			bb_error_বার্তা("%s: readonly variable", str);
			মুক্ত(str);
//NOTE: in bash, assignment in "export READONLY_VAR=Z" fails, and sets $?=1,
//but export per se succeeds (does put the var in env). We don't mimic that.
			ফেরত -1;
		}
		যদি (flags & SETFLAG_UNEXPORT) { // && cur->flg_export ?
			debug_printf_env("%s: unsetenv '%s'\n", __func__, str);
			*eq_sign = '\0';
			unsetenv(str);
			*eq_sign = '=';
		}
		যদি (cur->var_nest_level < local_lvl) {
			/* bash 3.2.33(1) and exported vars:
			 * # export z=z
			 * # f() { local z=a; env | grep ^z; }
			 * # f
			 * z=a
			 * # env | grep ^z
			 * z=z
			 */
			যদি (cur->flg_export)
				flags |= SETFLAG_EXPORT;
			/* New variable is local ("local VAR=VAL" or
			 * "VAR=VAL cmd")
			 * and existing one is global, or local
			 * on a lower level that new one.
			 * Remove it from global variable list:
			 */
			*cur_pp = cur->পরবর্তী;
			যদি (G.shadowed_vars_pp) {
				/* Save in "shadowed" list */
				debug_printf_env("shadowing %s'%s'/%u by '%s'/%u\n",
					cur->flg_export ? "exported " : "",
					cur->varstr, cur->var_nest_level, str, local_lvl
				);
				cur->পরবর্তী = *G.shadowed_vars_pp;
				*G.shadowed_vars_pp = cur;
			} অন্যথা {
				/* Came from pseudo_exec_argv(), no need to save: delete it */
				debug_printf_env("shadow-deleting %s'%s'/%u by '%s'/%u\n",
					cur->flg_export ? "exported " : "",
					cur->varstr, cur->var_nest_level, str, local_lvl
				);
				যদি (cur->max_len == 0) /* allocated "VAR=VAL"? */
					free_me = cur->varstr; /* then free it later */
				মুক্ত(cur);
			}
			বিরতি;
		}
#endif /* !__U_BOOT__ */

		যদি (strcmp(cur->varstr + name_len, eq_sign + 1) == 0) {
			debug_printf_env("assignement '%s' does not change anything\n", str);
 free_and_exp:
			মুক্ত(str);
			যাও exp;
		}

		/* Replace the value in the found "struct variable" */
		যদি (cur->max_len != 0) {
			যদি (cur->max_len >= strnlen(str, cur->max_len + 1)) {
				/* This one is from startup env, reuse space */
				debug_printf_env("reusing startup env for '%s'\n", str);
				strcpy(cur->varstr, str);
				যাও free_and_exp;
			}
			/* Can't reuse */
			cur->max_len = 0;
			যাও set_str_and_exp;
		}
		/* max_len == 0 signifies "malloced" var, which we can
		 * (and have to) free. But we can't free(cur->varstr) here:
		 * if cur->flg_export is 1, it is in the environment.
		 * We should either unsetenv+free, or wait until putenv,
		 * then putenv(new)+free(old).
		 */
		free_me = cur->varstr;
		যাও set_str_and_exp;
	}

	/* Not found or shadowed - create new variable struct */
#ifndef __U_BOOT__
	debug_printf_env("%s: alloc new var '%s'/%u\n", __func__, str, local_lvl);
#অন্যথা /* __U_BOOT__ */
	debug_printf_env("%s: alloc new var '%s'\n", __func__, str);
#endif /* __U_BOOT__ */
	cur = xzalloc(আকার(*cur));
#ifndef __U_BOOT__
	cur->var_nest_level = local_lvl;
#endif /* !__U_BOOT__ */
	cur->পরবর্তী = *cur_pp;
	*cur_pp = cur;

 set_str_and_exp:
	cur->varstr = str;
 exp:
#ifndef __U_BOOT__
#যদি !BB_MMU || ENABLE_HUSH_READONLY
	যদি (flags & SETFLAG_MAKE_RO) {
		cur->flg_read_only = 1;
	}
#endif
	যদি (flags & SETFLAG_EXPORT)
		cur->flg_export = 1;
#endif /* !__U_BOOT__ */
	retval = 0;
#ifndef __U_BOOT__
	যদি (cur->flg_export) {
		যদি (flags & SETFLAG_UNEXPORT) {
			cur->flg_export = 0;
			/* unsetenv was already done */
		} অন্যথা {
			debug_printf_env("%s: putenv '%s'/%u\n", __func__, cur->varstr, cur->var_nest_level);
			retval = putenv(cur->varstr);
			/* fall through to "free(free_me)" -
			 * only now we can free old exported malloced string
			 */
		}
	}
#endif /* !__U_BOOT__ */
	মুক্ত(free_me);

	handle_changed_special_names(cur->varstr);

	ফেরত retval;
}

#ifndef __U_BOOT__
স্থির পূর্ণসংখ্যা set_local_var0(অক্ষর *str)
{
	ফেরত set_local_var(str, 0);
}

স্থির শূন্য FAST_FUNC set_local_var_from_halves(ধ্রুবক অক্ষর *name, ধ্রুবক অক্ষর *val)
{
	অক্ষর *var = xasprintf("%s=%s", name, val);
	set_local_var0(var);
}

/* Used at startup and after each cd */
স্থির শূন্য set_pwd_var(unsigned flag)
{
	set_local_var(xasprintf("PWD=%s", get_cwd(/*force:*/ 1)), flag);
}
#endif /* !__U_BOOT__ */

#যদি ENABLE_HUSH_UNSET || ENABLE_HUSH_GETOPTS
স্থির পূর্ণসংখ্যা unset_local_var(ধ্রুবক অক্ষর *name)
{
	কাঠামো variable *cur;
	কাঠামো variable **cur_pp;

	cur_pp = &G.শীর্ষ_var;
	যখন ((cur = *cur_pp) != ফাঁকা) {
		যদি (varcmp(cur->varstr, name) == 0) {
			যদি (cur->flg_read_only) {
				bb_error_বার্তা("%s: readonly variable", name);
				ফেরত EXIT_FAILURE;
			}

			*cur_pp = cur->পরবর্তী;
			debug_printf_env("%s: unsetenv '%s'\n", __func__, cur->varstr);
			bb_unsetenv(cur->varstr);
			যদি (!cur->max_len)
				মুক্ত(cur->varstr);
			মুক্ত(cur);

			বিরতি;
		}
		cur_pp = &cur->পরবর্তী;
	}

	/* Handle "unset LINENO" et al even if did not find the variable to unset */
	handle_changed_special_names(name);

	ফেরত EXIT_SUCCESS;
}
#endif

#ifndef __U_BOOT__
/*
 * Helpers for "var1=val1 var2=val2 cmd" feature
 */
স্থির শূন্য add_vars(কাঠামো variable *var)
{
	কাঠামো variable *পরবর্তী;

	যখন (var) {
		পরবর্তী = var->পরবর্তী;
		var->পরবর্তী = G.শীর্ষ_var;
		G.শীর্ষ_var = var;
		যদি (var->flg_export) {
			debug_printf_env("%s: restoring exported '%s'/%u\n", __func__, var->varstr, var->var_nest_level);
			putenv(var->varstr);
		} অন্যথা {
			debug_printf_env("%s: restoring variable '%s'/%u\n", __func__, var->varstr, var->var_nest_level);
		}
		var = পরবর্তী;
	}
}

/* We put strings[i] into variable table and possibly putenv them.
 * If variable is read only, we can free the strings[i]
 * which attempts to overwrite it.
 * The strings[] vector itself is freed.
 */
স্থির শূন্য set_vars_and_save_old(অক্ষর **strings)
{
	অক্ষর **s;

	যদি (!strings)
		ফেরত;

	s = strings;
	যখন (*s) {
		কাঠামো variable *var_p;
		কাঠামো variable **var_pp;
		অক্ষর *eq;

		eq = strchr(*s, '=');
		যদি (HUSH_DEBUG && !eq)
			bb_simple_error_বার্তা_and_die("BUG in varexp4");
		var_pp = get_ptr_to_local_var(*s);
		যদি (var_pp) {
			var_p = *var_pp;
			যদি (var_p->flg_read_only) {
				অক্ষর **p;
				bb_error_বার্তা("%s: readonly variable", *s);
				/*
				 * "VAR=V BLTIN" unsets VARs after BLTIN completes.
				 * If VAR is readonly, leaving it in the list
				 * after asssignment error (msg above)
				 * causes doubled error message later, on unset.
				 */
				debug_printf_env("removing/freeing '%s' element\n", *s);
				মুক্ত(*s);
				p = s;
				করো { *p = p[1]; p++; } যখন (*p);
				যাও পরবর্তী;
			}
			/* below, set_local_var() with nest level will
			 * "shadow" (remove) this variable from
			 * global linked list.
			 */
		}
		debug_printf_env("%s: env override '%s'/%u\n", __func__, *s, G.var_nest_level);
		set_local_var(*s, (G.var_nest_level << SETFLAG_VARLVL_SHIFT) | SETFLAG_EXPORT);
		s++;
 পরবর্তী: ;
	}
	মুক্ত(strings);
}

/*
 * Unicode helper
 */
স্থির শূন্য reinit_unicode_for_hush(শূন্য)
{
	/* Unicode support should be activated even if LANG is set
	 * _during_ shell execution, not only if it was set when
	 * shell was started. Therefore, re-check LANG every time:
	 */
	যদি (ENABLE_FEATURE_CHECK_UNICODE_IN_ENV
	 || ENABLE_UNICODE_USING_LOCALE
	) {
		ধ্রুবক অক্ষর *s = get_local_var_মান("LC_ALL");
		যদি (!s) s = get_local_var_মান("LC_CTYPE");
		যদি (!s) s = get_local_var_মান("LANG");
		reinit_unicode(s);
	}
}

#endif /* !__U_BOOT__ */
/*
 * in_str support (strings, and "strings" read from files).
 */

#যদি ENABLE_HUSH_INTERACTIVE
#ifndef __U_BOOT__
/* To test correct lineedit/interactive behavior, type from command line:
 *	echo $P\
 *	\
 *	AT\
 *	H\
 *	\
 * It exercises a lot of corner cases.
 */
স্থির ধ্রুবক অক্ষর *setup_prompt_string(শূন্য)
{
	ধ্রুবক অক্ষর *prompt_str;

	debug_printf_prompt("%s promptmode:%d\n", __func__, G.promptmode);

# যদি ENABLE_FEATURE_EDITING_FANCY_PROMPT
	prompt_str = get_local_var_মান(G.promptmode == 0 ? "PS1" : "PS2");
	যদি (!prompt_str)
		prompt_str = "";
# অন্যথা
	prompt_str = "> "; /* if PS2, else... */
	যদি (G.promptmode == 0) { /* PS1 */
		/* No fancy prompts supported, (re)generate "CURDIR $ " by hand */
		মুক্ত(G.PS1);
		/* bash uses $PWD value, even if it is set by user.
		 * It uses current dir only if PWD is unset.
		 * We always use current dir. */
		prompt_str = G.PS1 = xasprintf("%s %c ", get_cwd(0), (geteuid() != 0) ? '$' : '#');
	}
# endif
	debug_printf("prompt_str '%s'\n", prompt_str);
	ফেরত prompt_str;
}
#endif /* !__U_BOOT__ */

#ifndef __U_BOOT__
স্থির পূর্ণসংখ্যা get_user_প্রবেশ(কাঠামো in_str *i)
#অন্যথা /* __U_BOOT__ */
স্থির শূন্য get_user_প্রবেশ(কাঠামো in_str *i)
#endif /* __U_BOOT__ */
{
#ifndef __U_BOOT__
# যদি ENABLE_FEATURE_EDITING
	/* In EDITING case, this function reads next input line,
	 * saves it in i->p, then returns 1st char of it.
	 */
	পূর্ণসংখ্যা r;
	ধ্রুবক অক্ষর *prompt_str;

	prompt_str = setup_prompt_string();
	জন্য (;;) {
		reinit_unicode_for_hush();
		G.flag_SIGINT = 0;

		bb_got_signal = 0;
		যদি (!sigisemptyset(&G.pending_set)) {
			/* Whoops, already got a signal, do not call read_line_input */
			bb_got_signal = r = -1;
		} অন্যথা {
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
			r = read_সারি_প্রবেশ(G.সারি_প্রবেশ_state, prompt_str,
				G.user_প্রবেশ_buf, CONFIG_FEATURE_EDITING_MAX_LEN-1
			);
			/* read_line_input intercepts ^C, "convert" it to SIGINT */
			যদি (r == 0)
				তোলো(SIGINT);
		}
		/* bash prints ^C (before running a trap, if any)
		 * both on keyboard ^C and on real SIGINT (non-kbd generated).
		 */
		যদি (sigismember(&G.pending_set, SIGINT)) {
			লিখ(STDOUT_FILENO, "^C\n", 3);
			G.last_exitcode = 128 | SIGINT;
		}
		check_and_run_traps();
		যদি (r == 0) /* keyboard ^C? */
			চালু; /* go back, read another input line */
		যদি (r > 0) /* normal input? (no ^C, no ^D, no signals) */
			বিরতি;
		যদি (!bb_got_signal) {
			/* r < 0: ^D/EOF/error detected (but not signal) */
			/* ^D on interactive input goes to next line before exiting: */
			লিখ(STDOUT_FILENO, "\n", 1);
			i->p = ফাঁকা;
			i->peek_buf[0] = r = EOF;
			ফেরত r;
		}
		/* it was a signal: go back, read another input line */
	}
	i->p = G.user_প্রবেশ_buf;
	ফেরত (unsigned অক্ষর)*i->p++;
# অন্যথা
	/* In !EDITING case, this function gets called for every char.
	 * Buffering happens deeper in the call chain, in hfgetc(i->file).
	 */
	পূর্ণসংখ্যা r;

	জন্য (;;) {
		G.flag_SIGINT = 0;
		যদি (i->last_char == '\0' || i->last_char == '\n') {
			ধ্রুবক অক্ষর *prompt_str = setup_prompt_string();
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
		যদি (r != '\0' && !G.flag_SIGINT)
			বিরতি;
		যদি (G.flag_SIGINT) {
			/* ^C or SIGINT: repeat */
			/* bash prints ^C even on real SIGINT (non-kbd generated) */
			/* kernel prints "^C" itself, just print newline: */
			লিখ(STDOUT_FILENO, "\n", 1);
			G.last_exitcode = 128 | SIGINT;
		}
	}
	ফেরত r;
# endif
#অন্যথা /* __U_BOOT__ */
	পূর্ণসংখ্যা n;
	পূর্ণসংখ্যা promptme;
	স্থির অক্ষর the_command[CONFIG_SYS_CBSIZE + 1];

	bootretry_reset_cmd_timeout();
	promptme = 1;
	n = u_boot_cli_readline(i);

# ifdef CONFIG_BOOT_RETRY_TIME
	যদি (n == -2) {
		puts("\nTimeout waiting for command\n");
#  ifdef CONFIG_RESET_TO_RETRY
		do_reset(ফাঁকা, 0, 0, ফাঁকা);
#  elif IS_ENABLED(CONFIG_RETRY_BOOTCMD)
	strcpy(console_বাফার, "run bootcmd\n");
# অন্যথা
#	error "This only works with CONFIG_RESET_TO_RETRY or CONFIG_BOOT_RETRY_COMMAND enabled"
#  endif
	}
# endif
	যদি (n == -1 ) {
		G.flag_repeat = 0;
		promptme = 0;
	}
	n = strlen(console_বাফার);
	console_বাফার[n] = '\n';
	console_বাফার[n+1]= '\0';
	যদি (had_ctrlc())
		G.flag_repeat = 0;
	clear_ctrlc();
	G.do_repeat = 0;
#ifndef __U_BOOT__
	যদি (G.promptmode == 1) {
#অন্যথা /* __U_BOOT__ */
	যদি (!G.promptmode) {
#endif /* __U_BOOT__ */
		যদি (console_বাফার[0] == '\n'&& G.flag_repeat == 0) {
			strcpy(the_command, console_বাফার);
		}
		অন্যথা {
			যদি (console_বাফার[0] != '\n') {
				strcpy(the_command, console_বাফার);
				G.flag_repeat = 1;
			}
			অন্যথা {
				G.do_repeat = 1;
			}
		}
		i->p = the_command;
	}
	অন্যথা {
		যদি (console_বাফার[0] != '\n') {
			যদি (strlen(the_command) + strlen(console_বাফার)
				< CONFIG_SYS_CBSIZE) {
				n = strlen(the_command);
#ifdef __U_BOOT__
				/*
				 * To avoid writing to bad places, we check if
				 * n is greater than 0.
				 * This bug was found by Harald Seiler.
				 */
				যদি (n > 0)
					the_command[n-1] = ' ';
				strcpy(&the_command[n], console_বাফার);
#অন্যথা /* !__U_BOOT__ */
			the_command[n-1] = ' ';
			strcpy(&the_command[n], console_বাফার);
#endif /* !__U_BOOT__ */
				}
				অন্যথা {
					the_command[0] = '\n';
					the_command[1] = '\0';
					G.flag_repeat = 0;
				}
		}
		যদি (promptme == 0) {
			the_command[0] = '\n';
			the_command[1] = '\0';
		}
		i->p = console_বাফার;
	}
#endif /* __U_BOOT__ */
}
/* This is the magic location that prints prompts
 * and gets data back from the user */
স্থির পূর্ণসংখ্যা fgetc_interactive(কাঠামো in_str *i)
{
	পূর্ণসংখ্যা ch;
#ifndef __U_BOOT__
	/* If it's interactive stdin, get new line. */
	যদি (G_interactive_fd && i->file == G.HFILE_stdin) {
#endif /* !__U_BOOT__ */
#ifndef __U_BOOT__
		/* Returns first char (or EOF), the rest is in i->p[] */
		ch = get_user_প্রবেশ(i);
#অন্যথা /* __U_BOOT__ */
		/* Avoid garbage value and make clang happy. */
		ch = 0;
		/*
		 * get_user_input() does not return anything when used in
		 * U-Boot.
		 * So, we need to take the read character from i->p[].
		 */
		get_user_প্রবেশ(i);
		যদি (i->p && *i->p) {
			ch = *i->p++;
		}
#endif /* __U_BOOT__ */
		G.promptmode = 1; /* PS2 */
		debug_printf_prompt("%s promptmode=%d\n", __func__, G.promptmode);
#ifndef __U_BOOT__
	} অন্যথা {
		/* Not stdin: script file, sourced file, etc */
		করো ch = hfgetc(i->file); যখন (ch == '\0');
	}
#endif /* !__U_BOOT__ */
	ফেরত ch;
}
#অন্যথা  /* !INTERACTIVE */
#ifndef __U_BOOT__
স্থির ALWAYS_INLINE পূর্ণসংখ্যা fgetc_interactive(কাঠামো in_str *i)
{
	পূর্ণসংখ্যা ch;
	করো ch = hfgetc(i->file); যখন (ch == '\0');
	ফেরত ch;
}
#endif /* !__U_BOOT__ */
#endif  /* !INTERACTIVE */

স্থির পূর্ণসংখ্যা i_getch(কাঠামো in_str *i)
{
	পূর্ণসংখ্যা ch;

#ifndef __U_BOOT__
	যদি (!i->file) {
		/* string-based in_str */
		ch = (unsigned অক্ষর)*i->p;
		যদি (ch != '\0') {
			i->p++;
			i->last_char = ch;
#যদি ENABLE_HUSH_LINENO_VAR
			যদি (ch == '\n') {
				G.parse_lineno++;
				debug_printf_parse("G.parse_lineno++ = %u\n", G.parse_lineno);
			}
#endif
			ফেরত ch;
		}
		ফেরত EOF;
	}

#endif /* !__U_BOOT__ */
	/* FILE-based in_str */

#যদি ENABLE_FEATURE_EDITING
	/* This can be stdin, check line editing char[] buffer */
	যদি (i->p && *i->p != '\0') {
		ch = (unsigned অক্ষর)*i->p++;
		যাও out;
#ifndef __U_BOOT__
	}
#অন্যথা /* __U_BOOT__ */
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
	} অন্যথা যদি (i->p && *i->p == '\0' && G.run_command_flags){
		ফেরত EOF;
	}
#endif /* __U_BOOT__ */
#endif
#ifndef __U_BOOT__
	/* peek_buf[] is an int array, not char. Can contain EOF. */
	ch = i->peek_buf[0];
	যদি (ch != 0) {
		পূর্ণসংখ্যা ch2 = i->peek_buf[1];
		i->peek_buf[0] = ch2;
		যদি (ch2 == 0) /* very likely, avoid redundant write */
			যাও out;
		i->peek_buf[1] = 0;
		যাও out;
	}

#endif /* !__U_BOOT__ */
	ch = fgetc_interactive(i);
 out:
	debug_printf("file_get: got '%c' %d\n", ch, ch);
	i->last_char = ch;
#যদি ENABLE_HUSH_LINENO_VAR
	যদি (ch == '\n') {
		G.parse_lineno++;
		debug_printf_parse("G.parse_lineno++ = %u\n", G.parse_lineno);
	}
#endif
	ফেরত ch;
}

স্থির পূর্ণসংখ্যা i_peek(কাঠামো in_str *i)
{
#ifndef __U_BOOT__
	পূর্ণসংখ্যা ch;

	যদি (!i->file) {
		/* string-based in_str */
		/* Doesn't report EOF on NUL. None of the callers care. */
		ফেরত (unsigned অক্ষর)*i->p;
	}

	/* FILE-based in_str */

#যদি ENABLE_FEATURE_EDITING && ENABLE_HUSH_INTERACTIVE
	/* This can be stdin, check line editing char[] buffer */
	যদি (i->p && *i->p != '\0')
		ফেরত (unsigned অক্ষর)*i->p;
#endif
	/* peek_buf[] is an int array, not char. Can contain EOF. */
	ch = i->peek_buf[0];
	যদি (ch != 0)
		ফেরত ch;

	/* Need to get a new char */
	ch = fgetc_interactive(i);
	debug_printf("file_peek: got '%c' %d\n", ch, ch);

	/* Save it by either rolling back line editing buffer, or in i->peek_buf[0] */
#যদি ENABLE_FEATURE_EDITING && ENABLE_HUSH_INTERACTIVE
	যদি (i->p) {
		i->p -= 1;
		ফেরত ch;
	}
#endif
	i->peek_buf[0] = ch;
	/*i->peek_buf[1] = 0; - already is */
	ফেরত ch;
#অন্যথা /* __U_BOOT__ */
	/* string-based in_str */
	/* Doesn't report EOF on NUL. None of the callers care. */
	ফেরত (unsigned অক্ষর)*i->p;
#endif /* __U_BOOT__ */
}

/* Only ever called if i_peek() was called, and did not return EOF.
 * IOW: we know the previous peek saw an ordinary char, not EOF, not NUL,
 * not end-of-line. Therefore we never need to read a new editing line here.
 */
স্থির পূর্ণসংখ্যা i_peek2(কাঠামো in_str *i)
{
#ifndef __U_BOOT__
	পূর্ণসংখ্যা ch;
#endif /* !__U_BOOT__ */

	/* There are two cases when i->p[] buffer exists.
	 * (1) it's a string in_str.
	 * (2) It's a file, and we have a saved line editing buffer.
	 * In both cases, we know that i->p[0] exists and not NUL, and
	 * the peek2 result is in i->p[1].
	 */
	যদি (i->p)
		ফেরত (unsigned অক্ষর)i->p[1];

#ifndef __U_BOOT__
	/* Now we know it is a file-based in_str. */

	/* peek_buf[] is an int array, not char. Can contain EOF. */
	/* Is there 2nd char? */
	ch = i->peek_buf[1];
	যদি (ch == 0) {
		/* We did not read it yet, get it now */
		করো ch = hfgetc(i->file); যখন (ch == '\0');
		i->peek_buf[1] = ch;
	}

	debug_printf("file_peek2: got '%c' %d\n", ch, ch);
	ফেরত ch;
#অন্যথা
	ফেরত 0;
#endif /* __U_BOOT__ */
}

স্থির পূর্ণসংখ্যা i_getch_and_eat_bkslash_nl(কাঠামো in_str *প্রবেশ)
{
	জন্য (;;) {
		পূর্ণসংখ্যা ch, ch2;

		ch = i_getch(প্রবেশ);
		যদি (ch != '\\')
			ফেরত ch;
		ch2 = i_peek(প্রবেশ);
		যদি (ch2 != '\n')
			ফেরত ch;
		/* backslash+newline, skip it */
		i_getch(প্রবেশ);
	}
}

/* Note: this function _eats_ \<newline> pairs, safe to use plain
 * i_getch() after it instead of i_getch_and_eat_bkslash_nl().
 */
স্থির পূর্ণসংখ্যা i_peek_and_eat_bkslash_nl(কাঠামো in_str *প্রবেশ)
{
	জন্য (;;) {
		পূর্ণসংখ্যা ch, ch2;

		ch = i_peek(প্রবেশ);
		যদি (ch != '\\')
			ফেরত ch;
		ch2 = i_peek2(প্রবেশ);
		যদি (ch2 != '\n')
			ফেরত ch;
		/* backslash+newline, skip it */
		i_getch(প্রবেশ);
		i_getch(প্রবেশ);
	}
}

#ifndef __U_BOOT__
স্থির শূন্য setup_file_in_str(কাঠামো in_str *i, HFILE *fp)
#অন্যথা /* __U_BOOT__ */
স্থির শূন্য setup_file_in_str(কাঠামো in_str *i)
#endif /* __U_BOOT__ */
{
	memset(i, 0, আকার(*i));
#ifndef __U_BOOT__
	i->file = fp;
	/* i->p = NULL; */
#endif /* !__U_BOOT__ */
}

স্থির শূন্য setup_string_in_str(কাঠামো in_str *i, ধ্রুবক অক্ষর *s)
{
	memset(i, 0, আকার(*i));
	/*i->file = NULL */;
	i->p = s;
}

/*
 * o_string support
 */
#define B_CHUNK  (32 * আকার(অক্ষর*))

স্থির শূন্য o_reset_to_empty_unquoted(o_string *o)
{
	o->দৈর্ঘ্য = 0;
	o->has_quoted_part = 0;
	যদি (o->তথ্য)
		o->তথ্য[0] = '\0';
}

স্থির শূন্য o_free_and_set_NULL(o_string *o)
{
	মুক্ত(o->তথ্য);
	memset(o, 0, আকার(*o));
}

স্থির ALWAYS_INLINE শূন্য o_free(o_string *o)
{
	মুক্ত(o->তথ্য);
}

স্থির শূন্য o_grow_by(o_string *o, পূর্ণসংখ্যা len)
{
	যদি (o->দৈর্ঘ্য + len > o->maxlen) {
		o->maxlen += (2 * len) | (B_CHUNK-1);
		o->তথ্য = xrealloc(o->তথ্য, 1 + o->maxlen);
	}
}

স্থির শূন্য o_addchr(o_string *o, পূর্ণসংখ্যা ch)
{
	debug_printf("o_addchr: '%c' o->length=%d o=%p\n", ch, o->দৈর্ঘ্য, o);
	যদি (o->দৈর্ঘ্য < o->maxlen) {
		/* likely. avoid o_grow_by() call */
 add:
		o->তথ্য[o->দৈর্ঘ্য] = ch;
		o->দৈর্ঘ্য++;
		o->তথ্য[o->দৈর্ঘ্য] = '\0';
		ফেরত;
	}
	o_grow_by(o, 1);
	যাও add;
}

#যদি 0
/* Valid only if we know o_string is not empty */
স্থির শূন্য o_delchr(o_string *o)
{
	o->দৈর্ঘ্য--;
	o->তথ্য[o->দৈর্ঘ্য] = '\0';
}
#endif

স্থির শূন্য o_addblock(o_string *o, ধ্রুবক অক্ষর *str, পূর্ণসংখ্যা len)
{
	o_grow_by(o, len);
	((অক্ষর*)mempcpy(&o->তথ্য[o->দৈর্ঘ্য], str, len))[0] = '\0';
	o->দৈর্ঘ্য += len;
}

স্থির শূন্য o_addstr(o_string *o, ধ্রুবক অক্ষর *str)
{
	o_addblock(o, str, strlen(str));
}

#ifndef __U_BOOT__
স্থির শূন্য o_addstr_with_NUL(o_string *o, ধ্রুবক অক্ষর *str)
{
	o_addblock(o, str, strlen(str) + 1);
}
#endif /* !__U_BOOT__ */

#যদি !BB_MMU
স্থির শূন্য nommu_addchr(o_string *o, পূর্ণসংখ্যা ch)
{
	যদি (o)
		o_addchr(o, ch);
}
#অন্যথা
# define nommu_addchr(o, str) ((শূন্য)0)
#endif

#ifndef __U_BOOT__
#যদি ENABLE_HUSH_MODE_X
স্থির শূন্য x_mode_addchr(পূর্ণসংখ্যা ch)
{
	o_addchr(&G.x_mode_buf, ch);
}
স্থির শূন্য x_mode_addstr(ধ্রুবক অক্ষর *str)
{
	o_addstr(&G.x_mode_buf, str);
}
স্থির শূন্য x_mode_addblock(ধ্রুবক অক্ষর *str, পূর্ণসংখ্যা len)
{
	o_addblock(&G.x_mode_buf, str, len);
}
স্থির শূন্য x_mode_prefix(শূন্য)
{
	পূর্ণসংখ্যা n = G.x_mode_depth;
	করো x_mode_addchr('+'); যখন (--n >= 0);
}
স্থির শূন্য x_mode_flush(শূন্য)
{
	পূর্ণসংখ্যা len = G.x_mode_buf.দৈর্ঘ্য;
	যদি (len <= 0)
		ফেরত;
	যদি (G.x_mode_fd > 0) {
		G.x_mode_buf.তথ্য[len] = '\n';
		full_write(G.x_mode_fd, G.x_mode_buf.তথ্য, len + 1);
	}
	G.x_mode_buf.দৈর্ঘ্য = 0;
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

#যদি ENABLE_HUSH_BRACE_EXPANSION
# define MAYBE_BRACES "{}"
#অন্যথা
# define MAYBE_BRACES ""
#endif

/* My analysis of quoting semantics tells me that state information
 * is associated with a destination, not a source.
 */
স্থির শূন্য o_addqchr(o_string *o, পূর্ণসংখ্যা ch)
{
	পূর্ণসংখ্যা sz = 1;
	/* '-' is included because of this case:
	 * >filename0 >filename1 >filename9; v='-'; echo filename[0"$v"9]
	 */
	অক্ষর *found = strchr("*?[-\\" MAYBE_BRACES, ch);
	যদি (found)
		sz++;
	o_grow_by(o, sz);
	যদি (found) {
		o->তথ্য[o->দৈর্ঘ্য] = '\\';
		o->দৈর্ঘ্য++;
	}
	o->তথ্য[o->দৈর্ঘ্য] = ch;
	o->দৈর্ঘ্য++;
	o->তথ্য[o->দৈর্ঘ্য] = '\0';
}

স্থির শূন্য o_addQchr(o_string *o, পূর্ণসংখ্যা ch)
{
	পূর্ণসংখ্যা sz = 1;
	যদি ((o->o_expflags & EXP_FLAG_ESC_GLOB_CHARS)
	 && strchr("*?[-\\" MAYBE_BRACES, ch)
	) {
		sz++;
		o->তথ্য[o->দৈর্ঘ্য] = '\\';
		o->দৈর্ঘ্য++;
	}
	o_grow_by(o, sz);
	o->তথ্য[o->দৈর্ঘ্য] = ch;
	o->দৈর্ঘ্য++;
	o->তথ্য[o->দৈর্ঘ্য] = '\0';
}

স্থির শূন্য o_addqblock(o_string *o, ধ্রুবক অক্ষর *str, পূর্ণসংখ্যা len)
{
	যখন (len) {
		অক্ষর ch;
		পূর্ণসংখ্যা sz;
		পূর্ণসংখ্যা ordinary_cnt = strcspn(str, "*?[-\\" MAYBE_BRACES);
		যদি (ordinary_cnt > len) /* paranoia */
			ordinary_cnt = len;
		o_addblock(o, str, ordinary_cnt);
		যদি (ordinary_cnt == len)
			ফেরত; /* NUL is already added by o_addblock */
		str += ordinary_cnt;
		len -= ordinary_cnt + 1; /* we are processing + 1 char below */

		ch = *str++;
		sz = 1;
		যদি (ch) { /* it is necessarily one of "*?[-\\" MAYBE_BRACES */
			sz++;
			o->তথ্য[o->দৈর্ঘ্য] = '\\';
			o->দৈর্ঘ্য++;
		}
		o_grow_by(o, sz);
		o->তথ্য[o->দৈর্ঘ্য] = ch;
		o->দৈর্ঘ্য++;
	}
	o->তথ্য[o->দৈর্ঘ্য] = '\0';
}

স্থির শূন্য o_addQblock(o_string *o, ধ্রুবক অক্ষর *str, পূর্ণসংখ্যা len)
{
	যদি (!(o->o_expflags & EXP_FLAG_ESC_GLOB_CHARS)) {
		o_addblock(o, str, len);
		ফেরত;
	}
	o_addqblock(o, str, len);
}

স্থির শূন্য o_addQstr(o_string *o, ধ্রুবক অক্ষর *str)
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
#যদি DEBUG_EXPAND || DEBUG_GLOB
স্থির শূন্য debug_print_list(ধ্রুবক অক্ষর *prefix, o_string *o, পূর্ণসংখ্যা n)
{
	অক্ষর **list = (অক্ষর**)o->তথ্য;
	পূর্ণসংখ্যা string_start = ((n + 0xf) & ~0xf) * আকার(list[0]);
	পূর্ণসংখ্যা i = 0;

	indent();
	fdprintf(2, "%s: list:%p n:%d string_start:%d length:%d maxlen:%d glob:%d quoted:%d escape:%d\n",
			prefix, list, n, string_start, o->দৈর্ঘ্য, o->maxlen,
			!!(o->o_expflags & EXP_FLAG_GLOB),
			o->has_quoted_part,
			!!(o->o_expflags & EXP_FLAG_ESC_GLOB_CHARS));
	যখন (i < n) {
		indent();
		fdprintf(2, " list[%d]=%d '%s' %p\n", i, (পূর্ণসংখ্যা)(uintptr_t)list[i],
				o->তথ্য + (পূর্ণসংখ্যা)(uintptr_t)list[i] + string_start,
				o->তথ্য + (পূর্ণসংখ্যা)(uintptr_t)list[i] + string_start);
		i++;
	}
	যদি (n) {
		ধ্রুবক অক্ষর *p = o->তথ্য + (পূর্ণসংখ্যা)(uintptr_t)list[n - 1] + string_start;
		indent();
#ifndef __U_BOOT__
		fdprintf(2, " total_sz:%ld\n", (long)((p + strlen(p) + 1) - o->তথ্য));
#অন্যথা /* __U_BOOT__ */
		printf(" total_sz:%ld\n", (long)((p + strlen(p) + 1) - o->তথ্য));
#endif /* __U_BOOT__ */
	}
}
#অন্যথা
# define debug_print_list(prefix, o, n) ((শূন্য)0)
#endif

/* n = o_save_ptr_helper(str, n) "starts new string" by storing an index value
 * in list[n] so that it points past last stored byte so far.
 * It returns n+1. */
স্থির পূর্ণসংখ্যা o_save_ptr_helper(o_string *o, পূর্ণসংখ্যা n)
{
	অক্ষর **list = (অক্ষর**)o->তথ্য;
	পূর্ণসংখ্যা string_start;
	পূর্ণসংখ্যা string_len;

	যদি (!o->has_empty_slot) {
		string_start = ((n + 0xf) & ~0xf) * আকার(list[0]);
		string_len = o->দৈর্ঘ্য - string_start;
		যদি (!(n & 0xf)) { /* 0, 0x10, 0x20...? */
			debug_printf_list("list[%d]=%d string_start=%d (growing)\n", n, string_len, string_start);
			/* list[n] points to string_start, make space for 16 more pointers */
			o->maxlen += 0x10 * আকার(list[0]);
			o->তথ্য = xrealloc(o->তথ্য, o->maxlen + 1);
			list = (অক্ষর**)o->তথ্য;
			memmove(list + n + 0x10, list + n, string_len);
			/*
			 * expand_on_ifs() has a "previous argv[] ends in IFS?"
			 * check. (grep for -prev-ifs-check-).
			 * Ensure that argv[-1][last] is not garbage
			 * but zero bytes, to save index check there.
			 */
			list[n + 0x10 - 1] = 0;
			o->দৈর্ঘ্য += 0x10 * আকার(list[0]);
		} অন্যথা {
			debug_printf_list("list[%d]=%d string_start=%d\n",
					n, string_len, string_start);
		}
	} অন্যথা {
		/* We have empty slot at list[n], reuse without growth */
		string_start = ((n+1 + 0xf) & ~0xf) * আকার(list[0]); /* NB: n+1! */
		string_len = o->দৈর্ঘ্য - string_start;
		debug_printf_list("list[%d]=%d string_start=%d (empty slot)\n",
				n, string_len, string_start);
		o->has_empty_slot = 0;
	}
	o->has_quoted_part = 0;
	list[n] = (অক্ষর*)(uintptr_t)string_len;
	ফেরত n + 1;
}

/* "What was our last o_save_ptr'ed position (byte offset relative o->data)?" */
স্থির পূর্ণসংখ্যা o_get_last_ptr(o_string *o, পূর্ণসংখ্যা n)
{
	অক্ষর **list = (অক্ষর**)o->তথ্য;
	পূর্ণসংখ্যা string_start = ((n + 0xf) & ~0xf) * আকার(list[0]);

	ফেরত ((পূর্ণসংখ্যা)(uintptr_t)list[n-1]) + string_start;
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

#যদি ENABLE_HUSH_BRACE_EXPANSION
/* There in a GNU extension, GLOB_BRACE, but it is not usable:
 * first, it processes even {a} (no commas), second,
 * I didn't manage to make it return strings when they don't match
 * existing files. Need to re-implement it.
 */

/* Helper */
স্থির পূর্ণসংখ্যা glob_needed(ধ্রুবক অক্ষর *s)
{
	যখন (*s) {
		যদি (*s == '\\') {
			যদি (!s[1])
				ফেরত 0;
			s += 2;
			চালু;
		}
		যদি (*s == '*' || *s == '[' || *s == '?' || *s == '{')
			ফেরত 1;
		s++;
	}
	ফেরত 0;
}
/* Return pointer to next closing brace or to comma */
স্থির ধ্রুবক অক্ষর *পরবর্তী_brace_sub(ধ্রুবক অক্ষর *cp)
{
	unsigned depth = 0;
	cp++;
	যখন (*cp != '\0') {
		যদি (*cp == '\\') {
			যদি (*++cp == '\0')
				বিরতি;
			cp++;
			চালু;
		}
		যদি ((*cp == '}' && depth-- == 0) || (*cp == ',' && depth == 0))
			বিরতি;
		যদি (*cp++ == '{')
			depth++;
	}

	ফেরত *cp != '\0' ? cp : ফাঁকা;
}
/* Recursive brace globber. Note: may garble pattern[]. */
স্থির পূর্ণসংখ্যা glob_brace(অক্ষর *pattern, o_string *o, পূর্ণসংখ্যা n)
{
	অক্ষর *new_pattern_buf;
	ধ্রুবক অক্ষর *begin;
	ধ্রুবক অক্ষর *পরবর্তী;
	ধ্রুবক অক্ষর *rest;
	ধ্রুবক অক্ষর *p;
	size_t rest_len;

	debug_printf_glob("glob_brace('%s')\n", pattern);

	begin = pattern;
	যখন (1) {
		যদি (*begin == '\0')
			যাও simple_glob;
		যদি (*begin == '{') {
			/* Find the first sub-pattern and at the same time
			 * find the rest after the closing brace */
			পরবর্তী = পরবর্তী_brace_sub(begin);
			যদি (পরবর্তী == ফাঁকা) {
				/* An illegal expression */
				যাও simple_glob;
			}
			যদি (*পরবর্তী == '}') {
				/* "{abc}" with no commas - illegal
				 * brace expr, disregard and skip it */
				begin = পরবর্তী + 1;
				চালু;
			}
			বিরতি;
		}
		যদি (*begin == '\\' && begin[1] != '\0')
			begin++;
		begin++;
	}
	debug_printf_glob("begin:%s\n", begin);
	debug_printf_glob("next:%s\n", পরবর্তী);

	/* Now find the end of the whole brace expression */
	rest = পরবর্তী;
	যখন (*rest != '}') {
		rest = পরবর্তী_brace_sub(rest);
		যদি (rest == ফাঁকা) {
			/* An illegal expression */
			যাও simple_glob;
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
	যখন (1) {
		/* Construct the new glob expression */
		memcpy(
			mempcpy(
				mempcpy(new_pattern_buf,
					/* We know the prefix for all sub-patterns */
					pattern, begin - pattern),
				p, পরবর্তী - p),
			rest, rest_len);

		/* Note: glob_brace() may garble new_pattern_buf[].
		 * That's why we re-copy prefix every time (1st memcpy above).
		 */
		n = glob_brace(new_pattern_buf, o, n);
		যদি (*পরবর্তী == '}') {
			/* We saw the last entry */
			বিরতি;
		}
		p = পরবর্তী + 1;
		পরবর্তী = পরবর্তী_brace_sub(পরবর্তী);
	}
	মুক্ত(new_pattern_buf);
	ফেরত n;

 simple_glob:
	{
		পূর্ণসংখ্যা gr;
		glob_t globdata;

		memset(&globdata, 0, আকার(globdata));
		gr = glob(pattern, 0, ফাঁকা, &globdata);
		debug_printf_glob("glob('%s'):%d\n", pattern, gr);
		যদি (gr != 0) {
			যদি (gr == GLOB_NOMATCH) {
				globfree(&globdata);
				/* NB: garbles parameter */
				unbackslash(pattern);
				o_addstr_with_NUL(o, pattern);
				debug_printf_glob("glob pattern '%s' is literal\n", pattern);
				ফেরত o_save_ptr_helper(o, n);
			}
			যদি (gr == GLOB_NOSPACE)
				bb_die_memory_exhausted();
			/* GLOB_ABORTED? Only happens with GLOB_ERR flag,
			 * but we didn't specify it. Paranoia again. */
			bb_error_বার্তা_and_die("glob error %d on '%s'", gr, pattern);
		}
		যদি (globdata.gl_pathv && globdata.gl_pathv[0]) {
			অক্ষর **argv = globdata.gl_pathv;
			যখন (1) {
				o_addstr_with_NUL(o, *argv);
				n = o_save_ptr_helper(o, n);
				argv++;
				যদি (!*argv)
					বিরতি;
			}
		}
		globfree(&globdata);
	}
	ফেরত n;
}
/* Performs globbing on last list[],
 * saving each result as a new list[].
 */
স্থির পূর্ণসংখ্যা perform_glob(o_string *o, পূর্ণসংখ্যা n)
{
	অক্ষর *pattern, *copy;

	debug_printf_glob("start perform_glob: n:%d o->data:%p\n", n, o->তথ্য);
	যদি (!o->তথ্য)
		ফেরত o_save_ptr_helper(o, n);
	pattern = o->তথ্য + o_get_last_ptr(o, n);
	debug_printf_glob("glob pattern '%s'\n", pattern);
	যদি (!glob_needed(pattern)) {
		/* unbackslash last string in o in place, fix length */
		o->দৈর্ঘ্য = unbackslash(pattern) - o->তথ্য;
		debug_printf_glob("glob pattern '%s' is literal\n", pattern);
		ফেরত o_save_ptr_helper(o, n);
	}

	copy = xstrdup(pattern);
	/* "forget" pattern in o */
	o->দৈর্ঘ্য = pattern - o->তথ্য;
	n = glob_brace(copy, o, n);
	মুক্ত(copy);
	যদি (DEBUG_GLOB)
		debug_print_list("perform_glob returning", o, n);
	ফেরত n;
}

#অন্যথা /* !HUSH_BRACE_EXPANSION */

/* Helper */
স্থির পূর্ণসংখ্যা glob_needed(ধ্রুবক অক্ষর *s)
{
	যখন (*s) {
		যদি (*s == '\\') {
			যদি (!s[1])
				ফেরত 0;
			s += 2;
			চালু;
		}
		যদি (*s == '*' || *s == '[' || *s == '?')
			ফেরত 1;
		s++;
	}
	ফেরত 0;
}
/* Performs globbing on last list[],
 * saving each result as a new list[].
 */
স্থির পূর্ণসংখ্যা perform_glob(o_string *o, পূর্ণসংখ্যা n)
{
#ifndef __U_BOOT__
	glob_t globdata;
	পূর্ণসংখ্যা gr;
#endif /* __U_BOOT__ */
	অক্ষর *pattern;

	debug_printf_glob("start perform_glob: n:%d o->data:%p\n", n, o->তথ্য);
	যদি (!o->তথ্য)
		ফেরত o_save_ptr_helper(o, n);
	pattern = o->তথ্য + o_get_last_ptr(o, n);
	debug_printf_glob("glob pattern '%s'\n", pattern);
	যদি (!glob_needed(pattern)) {
#ifndef __U_BOOT__
 literal:
#endif /* __U_BOOT__ */
		/* unbackslash last string in o in place, fix length */
		o->দৈর্ঘ্য = unbackslash(pattern) - o->তথ্য;
		debug_printf_glob("glob pattern '%s' is literal\n", pattern);
		ফেরত o_save_ptr_helper(o, n);
	}

#ifndef __U_BOOT__
	memset(&globdata, 0, আকার(globdata));
	/* Can't use GLOB_NOCHECK: it does not unescape the string.
	 * If we glob "*.\*" and don't find anything, we need
	 * to fall back to using literal "*.*", but GLOB_NOCHECK
	 * will return "*.\*"!
	 */
	gr = glob(pattern, 0, ফাঁকা, &globdata);
	debug_printf_glob("glob('%s'):%d\n", pattern, gr);
	যদি (gr != 0) {
		যদি (gr == GLOB_NOMATCH) {
			globfree(&globdata);
			যাও literal;
		}
		যদি (gr == GLOB_NOSPACE)
			bb_die_memory_exhausted();
		/* GLOB_ABORTED? Only happens with GLOB_ERR flag,
		 * but we didn't specify it. Paranoia again. */
		bb_error_বার্তা_and_die("glob error %d on '%s'", gr, pattern);
	}
	যদি (globdata.gl_pathv && globdata.gl_pathv[0]) {
		অক্ষর **argv = globdata.gl_pathv;
		/* "forget" pattern in o */
		o->দৈর্ঘ্য = pattern - o->তথ্য;
		যখন (1) {
			o_addstr_with_NUL(o, *argv);
			n = o_save_ptr_helper(o, n);
			argv++;
			যদি (!*argv)
				বিরতি;
		}
	}
	globfree(&globdata);
	যদি (DEBUG_GLOB)
		debug_print_list("perform_glob returning", o, n);
	ফেরত n;
#অন্যথা /* __U_BOOT__ */
	/*
	 * NOTE We only use perform glob to call unbackslash to remove backslash
	 * from string once expanded.
	 * So, it seems OK to return this if no previous return was done.
	 */
	ফেরত o_save_ptr_helper(o, n);
#endif /* __U_BOOT__ */
}

#endif /* !HUSH_BRACE_EXPANSION */

/* If o->o_expflags & EXP_FLAG_GLOB, glob the string so far remembered.
 * Otherwise, just finish current list[] and start new */
স্থির পূর্ণসংখ্যা o_save_ptr(o_string *o, পূর্ণসংখ্যা n)
{
	যদি (o->o_expflags & EXP_FLAG_GLOB) {
		/* If o->has_empty_slot, list[n] was already globbed
		 * (if it was requested back then when it was filled)
		 * so don't do that again! */
		যদি (!o->has_empty_slot)
			ফেরত perform_glob(o, n); /* o_save_ptr_helper is inside */
	}
	ফেরত o_save_ptr_helper(o, n);
}

/* "Please convert list[n] to real char* ptrs, and NULL terminate it." */
স্থির অক্ষর **o_finalize_list(o_string *o, পূর্ণসংখ্যা n)
{
	অক্ষর **list;
	পূর্ণসংখ্যা string_start;

	যদি (DEBUG_EXPAND)
		debug_print_list("finalized", o, n);
	debug_printf_expand("finalized n:%d\n", n);
	list = (অক্ষর**)o->তথ্য;
	string_start = ((n + 0xf) & ~0xf) * আকার(list[0]);
	list[--n] = ফাঁকা;
	যখন (n) {
		n--;
		list[n] = o->তথ্য + (পূর্ণসংখ্যা)(uintptr_t)list[n] + string_start;
	}
	ফেরত list;
}

স্থির শূন্য free_pipe_list(কাঠামো pipe *pi);

/* Returns pi->next - next pipe in the list */
স্থির কাঠামো pipe *free_pipe(কাঠামো pipe *pi)
{
	কাঠামো pipe *পরবর্তী;
	পূর্ণসংখ্যা i;

#ifndef __U_BOOT__
	debug_printf_clean("free_pipe (pid %d)\n", getpid());
#endif /* !__U_BOOT__ */
	জন্য (i = 0; i < pi->অঙ্ক_cmds; i++) {
		কাঠামো command *command;
#ifndef __U_BOOT__
		কাঠামো redir_struct *r, *rnext;
#endif /* !__U_BOOT__ */

		command = &pi->cmds[i];
		debug_printf_clean("  command %d:\n", i);
		যদি (command->argv) {
			যদি (DEBUG_CLEAN) {
				পূর্ণসংখ্যা a;
				অক্ষর **p;
				জন্য (a = 0, p = command->argv; *p; a++, p++) {
					debug_printf_clean("   argv[%d] = %s\n", a, *p);
				}
			}
			free_strings(command->argv);
			//command->argv = NULL;
		}
		/* not "else if": on syntax error, we may have both! */
		যদি (command->group) {
			debug_printf_clean("   begin group (cmd_type:%d)\n",
					command->cmd_type);
			free_pipe_list(command->group);
			debug_printf_clean("   end group\n");
			//command->group = NULL;
		}
		/* else is crucial here.
		 * If group != NULL, child_func is meaningless */
#যদি ENABLE_HUSH_FUNCTIONS
		অন্যথা যদি (command->child_func) {
			debug_printf_exec("cmd %p releases child func at %p\n", command, command->child_func);
			command->child_func->parent_cmd = ফাঁকা;
		}
#endif
#যদি !BB_MMU
		মুক্ত(command->group_as_string);
		//command->group_as_string = NULL;
#endif
#ifndef __U_BOOT__
		জন্য (r = command->redirects; r; r = rnext) {
			debug_printf_clean("   redirect %d%s",
					r->rd_fd, redir_table[r->rd_type].descrip);
			/* guard against the case >$FOO, where foo is unset or blank */
			যদি (r->rd_filename) {
				debug_printf_clean(" fname:'%s'\n", r->rd_filename);
				মুক্ত(r->rd_filename);
				//r->rd_filename = NULL;
			}
			debug_printf_clean(" rd_dup:%d\n", r->rd_dup);
			rnext = r->পরবর্তী;
			মুক্ত(r);
		}
		//command->redirects = NULL;
#endif /* !__U_BOOT__ */
	}
	মুক্ত(pi->cmds);   /* children are an array, they get freed all at once */
	//pi->cmds = NULL;
#ifndef __U_BOOT__
#যদি ENABLE_HUSH_JOB
	মুক্ত(pi->cmdtext);
	//pi->cmdtext = NULL;
#endif
#endif /* !__U_BOOT__ */

	পরবর্তী = pi->পরবর্তী;
	মুক্ত(pi);
	ফেরত পরবর্তী;
}

স্থির শূন্য free_pipe_list(কাঠামো pipe *pi)
{
	যখন (pi) {
#যদি HAS_KEYWORDS
		debug_printf_clean("pipe reserved word %d\n", pi->res_word);
#endif
		debug_printf_clean("pipe followup code %d\n", pi->followup);
		pi = free_pipe(pi);
	}
}

/*** Parsing routines ***/

#ifndef debug_print_বৃক্ষ
স্থির শূন্য debug_print_বৃক্ষ(কাঠামো pipe *pi, পূর্ণসংখ্যা lvl)
{
	স্থির ধ্রুবক অক্ষর *ধ্রুবক PIPE[] ALIGN_PTR = {
		[PIPE_SEQ] = "SEQ",
		[PIPE_AND] = "AND",
		[PIPE_OR ] = "OR" ,
		[PIPE_BG ] = "BG" ,
	};
	স্থির ধ্রুবক অক্ষর *RES[] = {
		[RES_NONE ] = "NONE" ,
# যদি ENABLE_HUSH_IF
		[RES_IF   ] = "IF"   ,
		[RES_THEN ] = "THEN" ,
		[RES_ELIF ] = "ELIF" ,
		[RES_ELSE ] = "ELSE" ,
		[RES_FI   ] = "FI"   ,
# endif
# যদি ENABLE_HUSH_LOOPS
		[RES_FOR  ] = "FOR"  ,
		[RES_WHILE] = "WHILE",
		[RES_UNTIL] = "UNTIL",
		[RES_DO   ] = "DO"   ,
		[RES_DONE ] = "DONE" ,
# endif
# যদি ENABLE_HUSH_LOOPS || ENABLE_HUSH_CASE
		[RES_IN   ] = "IN"   ,
# endif
# যদি ENABLE_HUSH_CASE
		[RES_CASE ] = "CASE" ,
		[RES_CASE_IN ] = "CASE_IN" ,
		[RES_MATCH] = "MATCH",
		[RES_CASE_BODY] = "CASE_BODY",
		[RES_ESAC ] = "ESAC" ,
# endif
		[RES_XXXX ] = "XXXX" ,
		[RES_SNTX ] = "SNTX" ,
	};
	স্থির ধ্রুবক অক্ষর *ধ্রুবক CMDTYPE[] ALIGN_PTR = {
		"{}",
		"()",
		"[noglob]",
# যদি ENABLE_HUSH_FUNCTIONS
		"func()",
# endif
	};

	পূর্ণসংখ্যা pin, prn;

	pin = 0;
	যখন (pi) {
		fdprintf(2, "%*spipe %d #cmds:%d %sres_word=%s followup=%d %s\n",
				lvl*2, "",
				pin,
				pi->অঙ্ক_cmds,
				(IF_HAS_KEYWORDS(pi->pi_inverted ? "! " :) ""),
				RES[pi->res_word],
				pi->followup, PIPE[pi->followup]
		);
		prn = 0;
		যখন (prn < pi->অঙ্ক_cmds) {
			কাঠামো command *command = &pi->cmds[prn];
			অক্ষর **argv = command->argv;

			fdprintf(2, "%*s cmd %d assignment_cnt:%d",
					lvl*2, "", prn,
					command->assignment_cnt);
# যদি ENABLE_HUSH_LINENO_VAR
			fdprintf(2, " LINENO:%u", command->lineno);
# endif
			যদি (command->group) {
				fdprintf(2, " group %s: (argv=%p)%s%s\n",
						CMDTYPE[command->cmd_type],
						argv
# যদি !BB_MMU
						, " group_as_string:", command->group_as_string
# অন্যথা
						, "", ""
# endif
				);
				debug_print_বৃক্ষ(command->group, lvl+1);
				prn++;
				চালু;
			}
			যদি (argv) যখন (*argv) {
				fdprintf(2, " '%s'", *argv);
				argv++;
			}
#ifndef __U_BOOT__
			যদি (command->redirects)
				fdprintf(2, " {redir}");
#endif /* __U_BOOT__ */
			fdprintf(2, "\n");
			prn++;
		}
		pi = pi->পরবর্তী;
		pin++;
	}
}
#endif /* debug_print_tree */

স্থির কাঠামো pipe *new_pipe(শূন্য)
{
	কাঠামো pipe *pi;
	pi = xzalloc(আকার(কাঠামো pipe));
	/*pi->res_word = RES_NONE; - RES_NONE is 0 anyway */
	ফেরত pi;
}

/* Command (member of a pipe) is complete, or we start a new pipe
 * if ctx->command is NULL.
 * No errors possible here.
 */
স্থির পূর্ণসংখ্যা done_command(কাঠামো parse_context *ctx)
{
	/* The command is really already in the pipe structure, so
	 * advance the pipe counter and make a new, null command. */
	কাঠামো pipe *pi = ctx->pipe;
	কাঠামো command *command = ctx->command;

#যদি 0	/* Instead we emit error message at run time */
	যদি (ctx->pending_redirect) {
		/* For example, "cmd >" (no filename to redirect to) */
		syntax_error("invalid redirect");
		ctx->pending_redirect = ফাঁকা;
	}
#endif

	যদি (command) {
		যদি (IS_NULL_CMD(command)) {
			debug_printf_parse("done_command: skipping null cmd, num_cmds=%d\n", pi->অঙ্ক_cmds);
			যাও clear_and_ret;
		}
		pi->অঙ্ক_cmds++;
		debug_printf_parse("done_command: ++num_cmds=%d\n", pi->অঙ্ক_cmds);
		//debug_print_tree(ctx->list_head, 20);
	} অন্যথা {
		debug_printf_parse("done_command: initializing, num_cmds=%d\n", pi->অঙ্ক_cmds);
	}

	/* Only real trickiness here is that the uncommitted
	 * command structure is not counted in pi->num_cmds. */
	pi->cmds = xrealloc(pi->cmds, আকার(*pi->cmds) * (pi->অঙ্ক_cmds+1));
	ctx->command = command = &pi->cmds[pi->অঙ্ক_cmds];
 clear_and_ret:
	memset(command, 0, আকার(*command));
#যদি ENABLE_HUSH_LINENO_VAR
	command->lineno = G.parse_lineno;
	debug_printf_parse("command->lineno = G.parse_lineno (%u)\n", G.parse_lineno);
#endif
	ফেরত pi->অঙ্ক_cmds; /* used only for 0/nonzero check */
}

স্থির শূন্য done_pipe(কাঠামো parse_context *ctx, pipe_style type)
{
	পূর্ণসংখ্যা not_null;

	debug_printf_parse("done_pipe entered, followup %d\n", type);
	/* Close previous command */
	not_null = done_command(ctx);
#যদি HAS_KEYWORDS
	ctx->pipe->pi_inverted = ctx->ctx_inverted;
	ctx->ctx_inverted = 0;
	ctx->pipe->res_word = ctx->ctx_res_w;
#endif
	যদি (type == PIPE_BG && ctx->list_শীর্ষ != ctx->pipe) {
		/* Necessary since && and || have precedence over &:
		 * "cmd1 && cmd2 &" must spawn both cmds, not only cmd2,
		 * in a backgrounded subshell.
		 */
		কাঠামো pipe *pi;
		কাঠামো command *command;

		/* Is this actually this construct, all pipes end with && or ||? */
		pi = ctx->list_শীর্ষ;
		যখন (pi != ctx->pipe) {
			যদি (pi->followup != PIPE_AND && pi->followup != PIPE_OR)
				যাও no_conv;
			pi = pi->পরবর্তী;
		}

		debug_printf_parse("BG with more than one pipe, converting to { p1 &&...pN; } &\n");
		pi->followup = PIPE_SEQ; /* close pN _not_ with "&"! */
		pi = xzalloc(আকার(*pi));
		pi->followup = PIPE_BG;
		pi->অঙ্ক_cmds = 1;
		pi->cmds = xzalloc(আকার(pi->cmds[0]));
		command = &pi->cmds[0];
		যদি (CMD_NORMAL != 0) /* "if xzalloc didn't do that already" */
			command->cmd_type = CMD_NORMAL;
		command->group = ctx->list_শীর্ষ;
#যদি !BB_MMU
		command->group_as_string = xstrndup(
			    ctx->as_string.তথ্য,
			    ctx->as_string.দৈর্ঘ্য - 1 /* do not copy last char, "&" */
		);
#endif
		/* Replace all pipes in ctx with one newly created */
		ctx->list_শীর্ষ = ctx->pipe = pi;
		/* for cases like "cmd && &", do not be tricked by last command
		 * being null - the entire {...} & is NOT null! */
		not_null = 1;
	} অন্যথা {
 no_conv:
		ctx->pipe->followup = type;
	}

	/* Without this check, even just <enter> on command line generates
	 * tree of three NOPs (!). Which is harmless but annoying.
	 * IOW: it is safe to do it unconditionally. */
	যদি (not_null
#যদি ENABLE_HUSH_IF
	 || ctx->ctx_res_w == RES_FI
#endif
#যদি ENABLE_HUSH_LOOPS
	 || ctx->ctx_res_w == RES_DONE
	 || ctx->ctx_res_w == RES_FOR
	 || ctx->ctx_res_w == RES_IN
#endif
#যদি ENABLE_HUSH_CASE
	 || ctx->ctx_res_w == RES_ESAC
#endif
	) {
		কাঠামো pipe *new_p;
		debug_printf_parse("done_pipe: adding new pipe: "
#ifndef __U_BOOT__
				"not_null:%d ctx->ctx_res_w:%d\n",
				not_null, ctx->ctx_res_w);
#অন্যথা /* __U_BOOT__ */
				"not_null:%d\n",
				not_null);
#endif /* __U_BOOT__ */
		new_p = new_pipe();
		ctx->pipe->পরবর্তী = new_p;
		ctx->pipe = new_p;
		/* RES_THEN, RES_DO etc are "sticky" -
		 * they remain set for pipes inside if/while.
		 * This is used to control execution.
		 * RES_FOR and RES_IN are NOT sticky (needed to support
		 * cases where variable or value happens to match a keyword):
		 */
#যদি ENABLE_HUSH_LOOPS
		যদি (ctx->ctx_res_w == RES_FOR
		 || ctx->ctx_res_w == RES_IN)
			ctx->ctx_res_w = RES_NONE;
#endif
#যদি ENABLE_HUSH_CASE
		যদি (ctx->ctx_res_w == RES_MATCH)
			ctx->ctx_res_w = RES_CASE_BODY;
		যদি (ctx->ctx_res_w == RES_CASE)
			ctx->ctx_res_w = RES_CASE_IN;
#endif
		ctx->command = ফাঁকা; /* trick done_command below */
		/* Create the memory for command, roughly:
		 * ctx->pipe->cmds = new struct command;
		 * ctx->command = &ctx->pipe->cmds[0];
		 */
		done_command(ctx);
		//debug_print_tree(ctx->list_head, 10);
	}
	debug_printf_parse("done_pipe return\n");
}

স্থির শূন্য initialize_context(কাঠামো parse_context *ctx)
{
	memset(ctx, 0, আকার(*ctx));
	যদি (MAYBE_ASSIGNMENT != 0)
		ctx->is_assignment = MAYBE_ASSIGNMENT;
	ctx->pipe = ctx->list_শীর্ষ = new_pipe();
	/* Create the memory for command, roughly:
	 * ctx->pipe->cmds = new struct command;
	 * ctx->command = &ctx->pipe->cmds[0];
	 */
	done_command(ctx);
}

/* If a reserved word is found and processed, parse context is modified
 * and 1 is returned.
 */
#যদি HAS_KEYWORDS
কাঠামো reserved_combo {
	অক্ষর literal[6];
	unsigned অক্ষর res;
	unsigned অক্ষর assignment_flag;
	uint32_t flag;
};
গণনা {
	FLAG_END   = (1 << RES_NONE ),
# যদি ENABLE_HUSH_IF
	FLAG_IF    = (1 << RES_IF   ),
	FLAG_THEN  = (1 << RES_THEN ),
	FLAG_ELIF  = (1 << RES_ELIF ),
	FLAG_ELSE  = (1 << RES_ELSE ),
	FLAG_FI    = (1 << RES_FI   ),
# endif
# যদি ENABLE_HUSH_LOOPS
	FLAG_FOR   = (1 << RES_FOR  ),
	FLAG_WHILE = (1 << RES_WHILE),
	FLAG_UNTIL = (1 << RES_UNTIL),
	FLAG_DO    = (1 << RES_DO   ),
	FLAG_DONE  = (1 << RES_DONE ),
	FLAG_IN    = (1 << RES_IN   ),
# endif
# যদি ENABLE_HUSH_CASE
	FLAG_MATCH = (1 << RES_MATCH),
	FLAG_ESAC  = (1 << RES_ESAC ),
# endif
	FLAG_START = (1 << RES_XXXX ),
};

স্থির ধ্রুবক কাঠামো reserved_combo* match_reserved_word(o_string *word)
{
	/* Mostly a list of accepted follow-up reserved words.
	 * FLAG_END means we are done with the sequence, and are ready
	 * to turn the compound list into a command.
	 * FLAG_START means the word must start a new compound list.
	 */
	স্থির ধ্রুবক কাঠামো reserved_combo reserved_list[] ALIGN4 = {
# যদি ENABLE_HUSH_IF
		{ "!",     RES_NONE,  NOT_ASSIGNMENT  , 0 },
		{ "if",    RES_IF,    MAYBE_ASSIGNMENT, FLAG_THEN | FLAG_START },
		{ "then",  RES_THEN,  MAYBE_ASSIGNMENT, FLAG_ELIF | FLAG_ELSE | FLAG_FI },
		{ "elif",  RES_ELIF,  MAYBE_ASSIGNMENT, FLAG_THEN },
		{ "else",  RES_ELSE,  MAYBE_ASSIGNMENT, FLAG_FI   },
		{ "fi",    RES_FI,    NOT_ASSIGNMENT  , FLAG_END  },
# endif
# যদি ENABLE_HUSH_LOOPS
		{ "for",   RES_FOR,   NOT_ASSIGNMENT  , FLAG_IN | FLAG_DO | FLAG_START },
		{ "while", RES_WHILE, MAYBE_ASSIGNMENT, FLAG_DO | FLAG_START },
		{ "until", RES_UNTIL, MAYBE_ASSIGNMENT, FLAG_DO | FLAG_START },
		{ "in",    RES_IN,    NOT_ASSIGNMENT  , FLAG_DO   },
		{ "do",    RES_DO,    MAYBE_ASSIGNMENT, FLAG_DONE },
		{ "done",  RES_DONE,  NOT_ASSIGNMENT  , FLAG_END  },
# endif
# যদি ENABLE_HUSH_CASE
		{ "case",  RES_CASE,  NOT_ASSIGNMENT  , FLAG_MATCH | FLAG_START },
		{ "esac",  RES_ESAC,  NOT_ASSIGNMENT  , FLAG_END  },
# endif
	};
	ধ্রুবক কাঠামো reserved_combo *r;

	জন্য (r = reserved_list; r < reserved_list + ARRAY_আকার(reserved_list); r++) {
		যদি (strcmp(word->তথ্য, r->literal) == 0)
			ফেরত r;
	}
	ফেরত ফাঁকা;
}
/* Return NULL: not a keyword, else: keyword
 */
স্থির ধ্রুবক কাঠামো reserved_combo* reserved_word(কাঠামো parse_context *ctx)
{
# যদি ENABLE_HUSH_CASE
	স্থির ধ্রুবক কাঠামো reserved_combo reserved_match = {
		"",        RES_MATCH, NOT_ASSIGNMENT , FLAG_MATCH | FLAG_ESAC
	};
# endif
	ধ্রুবক কাঠামো reserved_combo *r;

	যদি (ctx->word.has_quoted_part)
		ফেরত 0;
	r = match_reserved_word(&ctx->word);
	যদি (!r)
		ফেরত r; /* NULL */

	debug_printf("found reserved word %s, res %d\n", r->literal, r->res);
# যদি ENABLE_HUSH_CASE
	যদি (r->res == RES_IN && ctx->ctx_res_w == RES_CASE_IN) {
		/* "case word IN ..." - IN part starts first MATCH part */
		r = &reserved_match;
	} অন্যথা
# endif
	যদি (r->flag == 0) { /* '!' */
		যদি (ctx->ctx_inverted) { /* bash doesn't accept '! ! true' */
			syntax_error("! ! command");
			ctx->ctx_res_w = RES_SNTX;
		}
		ctx->ctx_inverted = 1;
		ফেরত r;
	}
	যদি (r->flag & FLAG_START) {
		কাঠামো parse_context *old;

		old = xmemdup(ctx, আকার(*ctx));
		debug_printf_parse("push stack %p\n", old);
		initialize_context(ctx);
		ctx->স্তূপ = old;
	} অন্যথা যদি (/*ctx->ctx_res_w == RES_NONE ||*/ !(ctx->old_flag & (1 << r->res))) {
		syntax_error_at(ctx->word.তথ্য);
		ctx->ctx_res_w = RES_SNTX;
		ফেরত r;
	} অন্যথা {
		/* "{...} fi" is ok. "{...} if" is not
		 * Example:
		 * if { echo foo; } then { echo bar; } fi */
		যদি (ctx->command->group)
			done_pipe(ctx, PIPE_SEQ);
	}

	ctx->ctx_res_w = r->res;
	ctx->old_flag = r->flag;
	ctx->is_assignment = r->assignment_flag;
	debug_printf_parse("ctx->is_assignment='%s'\n", assignment_flag[ctx->is_assignment]);

	যদি (ctx->old_flag & FLAG_END) {
		কাঠামো parse_context *old;

		done_pipe(ctx, PIPE_SEQ);
		debug_printf_parse("pop stack %p\n", ctx->স্তূপ);
		old = ctx->স্তূপ;
		old->command->group = ctx->list_শীর্ষ;
		old->command->cmd_type = CMD_NORMAL;
# যদি !BB_MMU
		/* At this point, the compound command's string is in
		 * ctx->as_string... except for the leading keyword!
		 * Consider this example: "echo a | if true; then echo a; fi"
		 * ctx->as_string will contain "true; then echo a; fi",
		 * with "if " remaining in old->as_string!
		 */
		{
			অক্ষর *str;
			পূর্ণসংখ্যা len = old->as_string.দৈর্ঘ্য;
			/* Concatenate halves */
			o_addstr(&old->as_string, ctx->as_string.তথ্য);
			o_free(&ctx->as_string);
			/* Find where leading keyword starts in first half */
			str = old->as_string.তথ্য + len;
			যদি (str > old->as_string.তথ্য)
				str--; /* skip whitespace after keyword */
			যখন (str > old->as_string.তথ্য && isalpha(str[-1]))
				str--;
			/* Ugh, we're done with this horrid hack */
			old->command->group_as_string = xstrdup(str);
			debug_printf_parse("pop, remembering as:'%s'\n",
					old->command->group_as_string);
		}
# endif
		*ctx = *old;   /* physical copy */
		মুক্ত(old);
	}
	ফেরত r;
}
#endif /* HAS_KEYWORDS */

/* Word is complete, look at it and update parsing context.
 * Normal return is 0. Syntax errors return 1.
 * Note: on return, word is reset, but not o_free'd!
 */
স্থির পূর্ণসংখ্যা done_word(কাঠামো parse_context *ctx)
{
	কাঠামো command *command = ctx->command;

	debug_printf_parse("done_word entered: '%s' %p\n", ctx->word.তথ্য, command);
	যদি (ctx->word.দৈর্ঘ্য == 0 && !ctx->word.has_quoted_part) {
		debug_printf_parse("done_word return 0: true null, ignored\n");
		ফেরত 0;
	}

#ifndef __U_BOOT__
	যদি (ctx->pending_redirect) {
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

		ctx->pending_redirect->rd_filename = xstrdup(ctx->word.তথ্য);
		/* Cater for >\file case:
		 * >\a creates file a; >\\a, >"\a", >"\\a" create file \a
		 * Same with heredocs:
		 * for <<\H delim is H; <<\\H, <<"\H", <<"\\H" - \H
		 */
		যদি (ctx->pending_redirect->rd_type == REDIRECT_HEREDOC) {
			unbackslash(ctx->pending_redirect->rd_filename);
			/* Is it <<"HEREDOC"? */
			যদি (ctx->word.has_quoted_part) {
				ctx->pending_redirect->rd_dup |= HEREDOC_QUOTED;
			}
		}
		debug_printf_parse("word stored in rd_filename: '%s'\n", ctx->word.তথ্য);
		ctx->pending_redirect = ফাঁকা;
	} অন্যথা {
#endif /* !__U_BOOT__ */
#যদি HAS_KEYWORDS
# যদি ENABLE_HUSH_CASE
		যদি (ctx->ctx_dsemicolon
		 && strcmp(ctx->word.তথ্য, "esac") != 0 /* not "... pattern) cmd;; esac" */
		) {
			/* already done when ctx_dsemicolon was set to 1: */
			/* ctx->ctx_res_w = RES_MATCH; */
			ctx->ctx_dsemicolon = 0;
		} অন্যথা
# endif
# যদি defined(CMD_TEST2_SINGLEWORD_NOGLOB)
		যদি (command->cmd_type == CMD_TEST2_SINGLEWORD_NOGLOB
		 && strcmp(ctx->word.তথ্য, "]]") == 0
		) {
			/* allow "[[ ]] >file" etc */
			command->cmd_type = CMD_SINGLEWORD_NOGLOB;
		} অন্যথা
# endif
		যদি (!command->argv /* if it's the first word... */
# যদি ENABLE_HUSH_LOOPS
		 && ctx->ctx_res_w != RES_FOR /* ...not after FOR or IN */
		 && ctx->ctx_res_w != RES_IN
# endif
# যদি ENABLE_HUSH_CASE
		 && ctx->ctx_res_w != RES_CASE
# endif
		) {
			ধ্রুবক কাঠামো reserved_combo *reserved;
			reserved = reserved_word(ctx);
			debug_printf_parse("checking for reserved-ness: %d\n", !!reserved);
			যদি (reserved) {
# যদি ENABLE_HUSH_LINENO_VAR
/* Case:
 * "while ...; do
 *	cmd ..."
 * If we don't close the pipe _now_, immediately after "do", lineno logic
 * sees "cmd" as starting at "do" - i.e., at the previous line.
 */
				যদি (0
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
				ফেরত (ctx->ctx_res_w == RES_SNTX);
			}
# যদি defined(CMD_TEST2_SINGLEWORD_NOGLOB)
			যদি (strcmp(ctx->word.তথ্য, "[[") == 0) {
				command->cmd_type = CMD_TEST2_SINGLEWORD_NOGLOB;
			} অন্যথা
# endif
# যদি defined(CMD_SINGLEWORD_NOGLOB)
			যদি (0
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
			 IF_HUSH_LOCAL(   || strcmp(ctx->word.তথ্য, "local") == 0)
			 IF_HUSH_EXPORT(  || strcmp(ctx->word.তথ্য, "export") == 0)
			 IF_HUSH_READONLY(|| strcmp(ctx->word.তথ্য, "readonly") == 0)
			) {
				command->cmd_type = CMD_SINGLEWORD_NOGLOB;
			}
# অন্যথা
			{ /* empty block to pair "if ... else" */ }
# endif
		}
#endif /* HAS_KEYWORDS */

		যদি (command->group) {
			/* "{ echo foo; } echo bar" - bad */
			syntax_error_at(ctx->word.তথ্য);
			debug_printf_parse("done_word return 1: syntax error, "
					"groups and arglists don't mix\n");
			ফেরত 1;
		}

		/* If this word wasn't an assignment, next ones definitely
		 * can't be assignments. Even if they look like ones. */
		যদি (ctx->is_assignment != DEFINITELY_ASSIGNMENT
		 && ctx->is_assignment != WORD_IS_KEYWORD
		) {
			ctx->is_assignment = NOT_ASSIGNMENT;
		} অন্যথা {
			যদি (ctx->is_assignment == DEFINITELY_ASSIGNMENT) {
				command->assignment_cnt++;
				debug_printf_parse("++assignment_cnt=%d\n", command->assignment_cnt);
			}
			debug_printf_parse("ctx->is_assignment was:'%s'\n", assignment_flag[ctx->is_assignment]);
			ctx->is_assignment = MAYBE_ASSIGNMENT;
		}
		debug_printf_parse("ctx->is_assignment='%s'\n", assignment_flag[ctx->is_assignment]);
		command->argv = add_string_to_strings(command->argv, xstrdup(ctx->word.তথ্য));
#ifdef __U_BOOT__
		command->argc++;
#endif /* __U_BOOT__ */
		debug_print_strings("word appended to argv", command->argv);

#ifndef __U_BOOT__
	}
#endif /* !__U_BOOT__ */

#যদি ENABLE_HUSH_LOOPS
	যদি (ctx->ctx_res_w == RES_FOR) {
		যদি (ctx->word.has_quoted_part
		 || endofname(command->argv[0])[0] != '\0'
		) {
			/* bash says just "not a valid identifier" */
			syntax_error("bad for loop variable");
			ফেরত 1;
		}
		/* Force FOR to have just one word (variable name) */
		/* NB: basically, this makes hush see "for v in ..."
		 * syntax as if it is "for v; in ...". FOR and IN become
		 * two pipe structs in parse tree. */
		done_pipe(ctx, PIPE_SEQ);
	}
#endif
#যদি ENABLE_HUSH_CASE
	/* Force CASE to have just one word */
	যদি (ctx->ctx_res_w == RES_CASE) {
		done_pipe(ctx, PIPE_SEQ);
	}
#endif

	o_reset_to_empty_unquoted(&ctx->word);

	debug_printf_parse("done_word return 0\n");
	ফেরত 0;
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
#যদি BB_MMU
#define parse_redir_ডান_fd(as_string, প্রবেশ) \
	parse_redir_ডান_fd(প্রবেশ)
#endif
স্থির পূর্ণসংখ্যা parse_redir_ডান_fd(o_string *as_string, কাঠামো in_str *প্রবেশ)
{
	পূর্ণসংখ্যা ch, d, ok;

	ch = i_peek(প্রবেশ);
	যদি (ch != '&')
		ফেরত REDIRFD_TO_FILE;

	ch = i_getch(প্রবেশ);  /* get the & */
	nommu_addchr(as_string, ch);
	ch = i_peek(প্রবেশ);
	যদি (ch == '-') {
		ch = i_getch(প্রবেশ);
		nommu_addchr(as_string, ch);
		ফেরত REDIRFD_CLOSE;
	}
	d = 0;
	ok = 0;
	যখন (ch != EOF && isdigit(ch)) {
		d = d*10 + (ch-'0');
		ok = 1;
		ch = i_getch(প্রবেশ);
		nommu_addchr(as_string, ch);
		ch = i_peek(প্রবেশ);
	}
	যদি (ok) ফেরত d;

//TODO: this is the place to catch ">&file" bashism (redirect both fd 1 and 2)

	bb_simple_error_বার্তা("ambiguous redirect");
	ফেরত REDIRFD_SYNTAX_ERR;
}

/* Return code is 0 normal, 1 if a syntax error is detected
 */
স্থির পূর্ণসংখ্যা parse_redirect(কাঠামো parse_context *ctx,
		পূর্ণসংখ্যা fd,
		redir_type style,
		কাঠামো in_str *প্রবেশ)
{
	কাঠামো command *command = ctx->command;
	কাঠামো redir_struct *redir;
	কাঠামো redir_struct **redirp;
	পূর্ণসংখ্যা dup_অঙ্ক;

	dup_অঙ্ক = REDIRFD_TO_FILE;
	যদি (style != REDIRECT_HEREDOC) {
		/* Check for a '>&1' type redirect */
		dup_অঙ্ক = parse_redir_ডান_fd(&ctx->as_string, প্রবেশ);
		যদি (dup_অঙ্ক == REDIRFD_SYNTAX_ERR)
			ফেরত 1;
	} অন্যথা {
		পূর্ণসংখ্যা ch = i_peek_and_eat_bkslash_nl(প্রবেশ);
		dup_অঙ্ক = (ch == '-'); /* HEREDOC_SKIPTABS bit is 1 */
		যদি (dup_অঙ্ক) { /* <<-... */
			ch = i_getch(প্রবেশ);
			nommu_addchr(&ctx->as_string, ch);
			ch = i_peek(প্রবেশ);
		}
	}

	যদি (style == REDIRECT_OVERWRITE && dup_অঙ্ক == REDIRFD_TO_FILE) {
		পূর্ণসংখ্যা ch = i_peek_and_eat_bkslash_nl(প্রবেশ);
		যদি (ch == '|') {
			/* >|FILE redirect ("clobbering" >).
			 * Since we do not support "set -o noclobber" yet,
			 * >| and > are the same for now. Just eat |.
			 */
			ch = i_getch(প্রবেশ);
			nommu_addchr(&ctx->as_string, ch);
		}
	}

	/* Create a new redir_struct and append it to the linked list */
	redirp = &command->redirects;
	যখন ((redir = *redirp) != ফাঁকা) {
		redirp = &(redir->পরবর্তী);
	}
	*redirp = redir = xzalloc(আকার(*redir));
	/* redir->next = NULL; */
	/* redir->rd_filename = NULL; */
	redir->rd_type = style;
	redir->rd_fd = (fd == -1) ? redir_table[style].default_fd : fd;

	debug_printf_parse("redirect type %d %s\n", redir->rd_fd,
				redir_table[style].descrip);

	redir->rd_dup = dup_অঙ্ক;
	যদি (style != REDIRECT_HEREDOC && dup_অঙ্ক != REDIRFD_TO_FILE) {
		/* Erik had a check here that the file descriptor in question
		 * is legit; I postpone that to "run time"
		 * A "-" representation of "close me" shows up as a -3 here */
		debug_printf_parse("duplicating redirect '%d>&%d'\n",
				redir->rd_fd, redir->rd_dup);
	} অন্যথা {
#যদি 0		/* Instead we emit error message at run time */
		যদি (ctx->pending_redirect) {
			/* For example, "cmd > <file" */
			syntax_error("invalid redirect");
		}
#endif
		/* Set ctx->pending_redirect, so we know what to do at the
		 * end of the next parsed word. */
		ctx->pending_redirect = redir;
	}
	ফেরত 0;
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
স্থির পূর্ণসংখ্যা redirect_opt_অঙ্ক(o_string *o)
{
	পূর্ণসংখ্যা অঙ্ক;

	যদি (o->তথ্য == ফাঁকা)
		ফেরত -1;
	অঙ্ক = bb_strtou(o->তথ্য, ফাঁকা, 10);
	যদি (errno || অঙ্ক < 0)
		ফেরত -1;
	o_reset_to_empty_unquoted(o);
	ফেরত অঙ্ক;
}

#যদি BB_MMU
#define fetch_till_str(as_string, প্রবেশ, word, skip_tabs) \
	fetch_till_str(প্রবেশ, word, skip_tabs)
#endif
স্থির অক্ষর *fetch_till_str(o_string *as_string,
		কাঠামো in_str *প্রবেশ,
		ধ্রুবক অক্ষর *word,
		পূর্ণসংখ্যা heredoc_flags)
{
	o_string heredoc = NULL_O_STRING;
	unsigned past_EOL;
	পূর্ণসংখ্যা পূর্ব = 0; /* not \ */
	পূর্ণসংখ্যা ch;

	/* Starting with "" is necessary for this case:
	 * cat <<EOF
	 *
	 * xxx
	 * EOF
	 */
	heredoc.তথ্য = xzalloc(1); /* start as "", not as NULL */

	যাও jump_in;

	যখন (1) {
		ch = i_getch(প্রবেশ);
		যদি (ch != EOF)
			nommu_addchr(as_string, ch);
		যদি (ch == '\n' || ch == EOF) {
 check_heredoc_end:
#ifndef __U_BOOT__
			যদি ((heredoc_flags & HEREDOC_QUOTED) || পূর্ব != '\\') {
#অন্যথা /* __U_BOOT__ */
			যদি (পূর্ব != '\\') {
#endif
				/* End-of-line, and not a line continuation */
				যদি (strcmp(heredoc.তথ্য + past_EOL, word) == 0) {
					heredoc.তথ্য[past_EOL] = '\0';
					debug_printf_heredoc("parsed '%s' heredoc '%s'\n", word, heredoc.তথ্য);
					ফেরত heredoc.তথ্য;
				}
				যদি (ch == '\n') {
					/* This is a new line.
					 * Remember position and backslash-escaping status.
					 */
					o_addchr(&heredoc, ch);
					পূর্ব = ch;
 jump_in:
					past_EOL = heredoc.দৈর্ঘ্য;
					/* Get 1st char of next line, possibly skipping leading tabs */
					করো {
						ch = i_getch(প্রবেশ);
						যদি (ch != EOF)
							nommu_addchr(as_string, ch);
#ifndef __U_BOOT__
					} যখন ((heredoc_flags & HEREDOC_SKIPTABS) && ch == '\t');
#অন্যথা /* __U_BOOT__ */
				} যখন (ch == '\t');
#endif
					/* If this immediately ended the line,
					 * go back to end-of-line checks.
					 */
					যদি (ch == '\n')
						যাও check_heredoc_end;
				}
			} অন্যথা {
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
				heredoc.তথ্য[--heredoc.দৈর্ঘ্য] = '\0';
				পূর্ব = 0; /* not '\' */
				চালু;
			}
		}
		যদি (ch == EOF) {
			o_free(&heredoc);
			ফেরত ফাঁকা; /* error */
		}
		o_addchr(&heredoc, ch);
		nommu_addchr(as_string, ch);
		যদি (পূর্ব == '\\' && ch == '\\')
			/* Correctly handle foo\\<eol> (not a line cont.) */
			পূর্ব = 0; /* not '\' */
		অন্যথা
			পূর্ব = ch;
	}
}
#endif /* !__U_BOOT__ */

/* Look at entire parse tree for not-yet-loaded REDIRECT_HEREDOCs
 * and load them all. There should be exactly heredoc_cnt of them.
 */
#যদি BB_MMU
#define fetch_heredocs(as_string, pi, heredoc_cnt, প্রবেশ) \
	fetch_heredocs(pi, heredoc_cnt, প্রবেশ)
#endif
স্থির পূর্ণসংখ্যা fetch_heredocs(o_string *as_string, কাঠামো pipe *pi, পূর্ণসংখ্যা heredoc_cnt, কাঠামো in_str *প্রবেশ)
{
	যখন (pi && heredoc_cnt) {
		পূর্ণসংখ্যা i;
		কাঠামো command *cmd = pi->cmds;

		debug_printf_heredoc("fetch_heredocs: num_cmds:%d cmd argv0:'%s'\n",
				pi->অঙ্ক_cmds,
				cmd->argv ? cmd->argv[0] : "NONE"
		);
		জন্য (i = 0; i < pi->অঙ্ক_cmds; i++) {
#ifndef __U_BOOT__
			কাঠামো redir_struct *redir = cmd->redirects;

#endif /* !__U_BOOT__ */
			debug_printf_heredoc("fetch_heredocs: %d cmd argv0:'%s'\n",
					i, cmd->argv ? cmd->argv[0] : "NONE");
#ifndef __U_BOOT__
			যখন (redir) {
				যদি (redir->rd_type == REDIRECT_HEREDOC) {
					অক্ষর *p;

					redir->rd_type = REDIRECT_HEREDOC2;
					/* redir->rd_dup is (ab)used to indicate <<- */
					p = fetch_till_str(as_string, প্রবেশ,
							redir->rd_filename, redir->rd_dup);
					যদি (!p) {
						syntax_error("unexpected EOF in here document");
						ফেরত -1;
					}
					মুক্ত(redir->rd_filename);
					redir->rd_filename = p;
					heredoc_cnt--;
				}
				redir = redir->পরবর্তী;
			}
#endif /* !__U_BOOT__ */
			যদি (cmd->group) {
				//bb_error_msg("%s:%u heredoc_cnt:%d", __func__, __LINE__, heredoc_cnt);
				heredoc_cnt = fetch_heredocs(as_string, cmd->group, heredoc_cnt, প্রবেশ);
				//bb_error_msg("%s:%u heredoc_cnt:%d", __func__, __LINE__, heredoc_cnt);
				যদি (heredoc_cnt < 0)
					ফেরত heredoc_cnt; /* error */
			}
			cmd++;
		}
		pi = pi->পরবর্তী;
	}
	ফেরত heredoc_cnt;
}

স্থির পূর্ণসংখ্যা run_list(কাঠামো pipe *pi);
#যদি BB_MMU
#define parse_stream(pstring, heredoc_cnt_ptr, প্রবেশ, end_trigger) \
	parse_stream(heredoc_cnt_ptr, প্রবেশ, end_trigger)
#endif
স্থির কাঠামো pipe *parse_stream(অক্ষর **pstring,
		পূর্ণসংখ্যা *heredoc_cnt_ptr,
		কাঠামো in_str *প্রবেশ,
		পূর্ণসংখ্যা end_trigger);

/* Returns number of heredocs not yet consumed,
 * or -1 on error.
 */
স্থির পূর্ণসংখ্যা parse_group(কাঠামো parse_context *ctx,
		কাঠামো in_str *প্রবেশ, পূর্ণসংখ্যা ch)
{
	/* ctx->word contains characters seen prior to ( or {.
	 * Typically it's empty, but for function defs,
	 * it contains function name (without '()'). */
#যদি BB_MMU
# define as_string ফাঁকা
#অন্যথা
	অক্ষর *as_string = ফাঁকা;
#endif
	কাঠামো pipe *pipe_list;
	পূর্ণসংখ্যা heredoc_cnt = 0;
	পূর্ণসংখ্যা endch;
	কাঠামো command *command = ctx->command;

	debug_printf_parse("parse_group entered\n");
#যদি ENABLE_HUSH_FUNCTIONS
	যদি (ch == '(' && !ctx->word.has_quoted_part) {
		যদি (ctx->word.দৈর্ঘ্য)
			যদি (done_word(ctx))
				ফেরত -1;
		যদি (!command->argv)
			যাও skip; /* (... */
		যদি (command->argv[1]) { /* word word ... (... */
			syntax_error_unexpected_ch('(');
			ফেরত -1;
		}
		/* it is "word(..." or "word (..." */
		করো
			ch = i_getch(প্রবেশ);
		যখন (ch == ' ' || ch == '\t');
		যদি (ch != ')') {
			syntax_error_unexpected_ch(ch);
			ফেরত -1;
		}
		nommu_addchr(&ctx->as_string, ch);
		করো
			ch = i_getch(প্রবেশ);
		যখন (ch == ' ' || ch == '\t' || ch == '\n');
		যদি (ch != '{' && ch != '(') {
			syntax_error_unexpected_ch(ch);
			ফেরত -1;
		}
//bash allows functions named "123", "..", "return"!
//		if (endofname(command->argv[0])[0] != '\0') {
//			syntax_error("bad function name");
//			return -1;
//		}
		nommu_addchr(&ctx->as_string, ch);
		command->cmd_type = CMD_FUNCDEF;
		যাও skip;
	}
#endif

#যদি 0 /* Prevented by caller */
	যদি (command->argv /* word [word]{... */
	 || ctx->word.দৈর্ঘ্য /* word{... */
	 || ctx->word.has_quoted_part /* ""{... */
	) {
		syntax_error(ফাঁকা);
		debug_printf_parse("parse_group return -1: "
			"syntax error, groups and arglists don't mix\n");
		ফেরত -1;
	}
#endif

#ifndef __U_BOOT__
 IF_HUSH_FUNCTIONS(skip:)
#endif /* !__U_BOOT__ */

	endch = '}';
	যদি (ch == '(') {
		endch = ')';
#ifndef __U_BOOT__
		IF_HUSH_FUNCTIONS(যদি (command->cmd_type != CMD_FUNCDEF))
			command->cmd_type = CMD_SUBSHELL;
#endif /* !__U_BOOT__ */
	} অন্যথা {
		/* bash does not allow "{echo...", requires whitespace */
		ch = i_peek(প্রবেশ);
		যদি (ch != ' ' && ch != '\t' && ch != '\n'
		 && ch != '('	/* but "{(..." is allowed (without whitespace) */
		) {
			syntax_error_unexpected_ch(ch);
			ফেরত -1;
		}
		যদি (ch != '(') {
			ch = i_getch(প্রবেশ);
			nommu_addchr(&ctx->as_string, ch);
		}
	}

	debug_printf_heredoc("calling parse_stream, heredoc_cnt:%d\n", heredoc_cnt);
	pipe_list = parse_stream(&as_string, &heredoc_cnt, প্রবেশ, endch);
	debug_printf_heredoc("parse_stream returned: heredoc_cnt:%d\n", heredoc_cnt);
#যদি !BB_MMU
	যদি (as_string)
		o_addstr(&ctx->as_string, as_string);
#endif

	/* empty ()/{} or parse error? */
	যদি (!pipe_list || pipe_list == ERR_PTR) {
		/* parse_stream already emitted error msg */
		যদি (!BB_MMU)
			মুক্ত(as_string);
		debug_printf_parse("parse_group return -1: "
			"parse_stream returned %p\n", pipe_list);
		ফেরত -1;
	}
#যদি !BB_MMU
	as_string[strlen(as_string) - 1] = '\0'; /* plink ')' or '}' */
	command->group_as_string = as_string;
	debug_printf_parse("end of group, remembering as:'%s'\n",
			command->group_as_string);
#endif

#যদি ENABLE_HUSH_FUNCTIONS
	/* Convert "f() (cmds)" to "f() {(cmds)}" */
	যদি (command->cmd_type == CMD_FUNCDEF && endch == ')') {
		কাঠামো command *cmd2;

		cmd2 = xzalloc(আকার(*cmd2));
		cmd2->cmd_type = CMD_SUBSHELL;
		cmd2->group = pipe_list;
# যদি !BB_MMU
//UNTESTED!
		cmd2->group_as_string = command->group_as_string;
		command->group_as_string = xasprintf("(%s)", command->group_as_string);
# endif

		pipe_list = new_pipe();
		pipe_list->cmds = cmd2;
		pipe_list->অঙ্ক_cmds = 1;
	}
#endif

	command->group = pipe_list;

	debug_printf_parse("parse_group return %d\n", heredoc_cnt);
	ফেরত heredoc_cnt;
	/* command remains "open", available for possible redirects */
#undef as_string
}

#ifndef __U_BOOT__
#যদি ENABLE_HUSH_TICK || ENABLE_FEATURE_SH_MATH || ENABLE_HUSH_DOLLAR_OPS
/* Subroutines for copying $(...) and `...` things */
/* '...' */
স্থির পূর্ণসংখ্যা add_till_single_quote(o_string *dest, কাঠামো in_str *প্রবেশ)
{
	যখন (1) {
		পূর্ণসংখ্যা ch = i_getch(প্রবেশ);
		যদি (ch == EOF) {
			syntax_error_unterm_ch('\'');
			ফেরত 0;
		}
		যদি (ch == '\'')
			ফেরত 1;
		o_addchr(dest, ch);
	}
}
স্থির পূর্ণসংখ্যা add_till_single_quote_dquoted(o_string *dest, কাঠামো in_str *প্রবেশ)
{
	যখন (1) {
		পূর্ণসংখ্যা ch = i_getch(প্রবেশ);
		যদি (ch == EOF) {
			syntax_error_unterm_ch('\'');
			ফেরত 0;
		}
		যদি (ch == '\'')
			ফেরত 1;
		o_addqchr(dest, ch);
	}
}

/* "...\"...`..`...." - do we need to handle "...$(..)..." too? */
স্থির পূর্ণসংখ্যা add_till_backquote(o_string *dest, কাঠামো in_str *প্রবেশ, পূর্ণসংখ্যা in_dquote);
স্থির পূর্ণসংখ্যা add_till_double_quote(o_string *dest, কাঠামো in_str *প্রবেশ)
{
	যখন (1) {
		পূর্ণসংখ্যা ch = i_getch(প্রবেশ);
		যদি (ch == EOF) {
			syntax_error_unterm_ch('"');
			ফেরত 0;
		}
		যদি (ch == '"')
			ফেরত 1;
		যদি (ch == '\\') {  /* \x. Copy both chars. */
			o_addchr(dest, ch);
			ch = i_getch(প্রবেশ);
		}
		o_addchr(dest, ch);
		যদি (ch == '`') {
			যদি (!add_till_backquote(dest, প্রবেশ, /*in_dquote:*/ 1))
				ফেরত 0;
			o_addchr(dest, ch);
			চালু;
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
স্থির পূর্ণসংখ্যা add_till_backquote(o_string *dest, কাঠামো in_str *প্রবেশ, পূর্ণসংখ্যা in_dquote)
{
	যখন (1) {
		পূর্ণসংখ্যা ch = i_getch(প্রবেশ);
		যদি (ch == '`')
			ফেরত 1;
		যদি (ch == '\\') {
			/* \x. Copy both unless it is \`, \$, \\ and maybe \" */
			ch = i_getch(প্রবেশ);
			যদি (ch != '`'
			 && ch != '$'
			 && ch != '\\'
			 && (!in_dquote || ch != '"')
			) {
				o_addchr(dest, '\\');
			}
		}
		যদি (ch == EOF) {
			syntax_error_unterm_ch('`');
			ফেরত 0;
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
স্থির পূর্ণসংখ্যা add_till_closing_bracket(o_string *dest, কাঠামো in_str *প্রবেশ, unsigned end_ch)
{
	পূর্ণসংখ্যা ch;
	অক্ষর dbl = end_ch & DOUBLE_CLOSE_CHAR_FLAG;
# যদি BASH_SUBSTR || BASH_PATTERN_SUBST
	অক্ষর end_char2 = end_ch >> 8;
# endif
	end_ch &= (DOUBLE_CLOSE_CHAR_FLAG - 1);

# যদি ENABLE_HUSH_INTERACTIVE
	G.promptmode = 1; /* PS2 */
# endif
	debug_printf_prompt("%s promptmode=%d\n", __func__, G.promptmode);

	যখন (1) {
		ch = i_getch(প্রবেশ);
		যদি (ch == EOF) {
			syntax_error_unterm_ch(end_ch);
			ফেরত 0;
		}
		যদি (ch == end_ch
# যদি BASH_SUBSTR || BASH_PATTERN_SUBST
		 || ch == end_char2
# endif
		) {
			যদি (!dbl)
				বিরতি;
			/* we look for closing )) of $((EXPR)) */
			যদি (i_peek_and_eat_bkslash_nl(প্রবেশ) == end_ch) {
				i_getch(প্রবেশ); /* eat second ')' */
				বিরতি;
			}
		}
		o_addchr(dest, ch);
		//bb_error_msg("%s:o_addchr('%c')", __func__, ch);
		যদি (ch == '(' || ch == '{') {
			ch = (ch == '(' ? ')' : '}');
			যদি (!add_till_closing_bracket(dest, প্রবেশ, ch))
				ফেরত 0;
			o_addchr(dest, ch);
			চালু;
		}
		যদি (ch == '\'') {
			যদি (!add_till_single_quote(dest, প্রবেশ))
				ফেরত 0;
			o_addchr(dest, ch);
			চালু;
		}
		যদি (ch == '"') {
			যদি (!add_till_double_quote(dest, প্রবেশ))
				ফেরত 0;
			o_addchr(dest, ch);
			চালু;
		}
		যদি (ch == '`') {
			যদি (!add_till_backquote(dest, প্রবেশ, /*in_dquote:*/ 0))
				ফেরত 0;
			o_addchr(dest, ch);
			চালু;
		}
		যদি (ch == '\\') {
			/* \x. Copy verbatim. Important for  \(, \) */
			ch = i_getch(প্রবেশ);
			যদি (ch == EOF) {
				syntax_error_unterm_ch(end_ch);
				ফেরত 0;
			}
# যদি 0
			যদি (ch == '\n') {
				/* "backslash+newline", ignore both */
				o_delchr(dest); /* undo insertion of '\' */
				চালু;
			}
# endif
			o_addchr(dest, ch);
			//bb_error_msg("%s:o_addchr('%c') after '\\'", __func__, ch);
			চালু;
		}
	}
	debug_printf_parse("%s return '%s' ch:'%c'\n", __func__, dest->তথ্য, ch);
	ফেরত ch;
}
#endif /* ENABLE_HUSH_TICK || ENABLE_FEATURE_SH_MATH || ENABLE_HUSH_DOLLAR_OPS */

#যদি BASH_DOLLAR_SQUOTE
/* Return code: 1 for "found and parsed", 0 for "seen something else" */
# যদি BB_MMU
#define parse_dollar_squote(as_string, dest, প্রবেশ) \
	parse_dollar_squote(dest, প্রবেশ)
#define as_string ফাঁকা
# endif
স্থির পূর্ণসংখ্যা parse_dollar_squote(o_string *as_string, o_string *dest, কাঠামো in_str *প্রবেশ)
{
	পূর্ণসংখ্যা start;
	পূর্ণসংখ্যা ch = i_peek_and_eat_bkslash_nl(প্রবেশ);  /* first character after the $ */
	debug_printf_parse("parse_dollar_squote entered: ch='%c'\n", ch);
	যদি (ch != '\'')
		ফেরত 0;

	dest->has_quoted_part = 1;
	start = dest->দৈর্ঘ্য;

	ch = i_getch(প্রবেশ); /* eat ' */
	nommu_addchr(as_string, ch);
	যখন (1) {
		ch = i_getch(প্রবেশ);
		nommu_addchr(as_string, ch);
		যদি (ch == EOF) {
			syntax_error_unterm_ch('\'');
			ফেরত 0;
		}
		যদি (ch == '\'')
			বিরতি;
		যদি (ch == SPECIAL_VAR_প্রতীক) {
			/* Convert raw ^C to corresponding special variable reference */
			o_addchr(dest, SPECIAL_VAR_প্রতীক);
			o_addchr(dest, SPECIAL_VAR_QUOTED_SVS);
			/* will addchr() another SPECIAL_VAR_SYMBOL (see after the if() block) */
		} অন্যথা যদি (ch == '\\') {
			স্থির ধ্রুবক অক্ষর C_escapes[] ALIGN1 = "nrbtfav""x\\01234567";

			ch = i_getch(প্রবেশ);
			nommu_addchr(as_string, ch);
			যদি (strchr(C_escapes, ch)) {
				অক্ষর buf[4];
				অক্ষর *p = buf;
				পূর্ণসংখ্যা cnt = 2;

				buf[0] = ch;
				যদি ((unsigned অক্ষর)(ch - '0') <= 7) { /* \ooo */
					করো {
						ch = i_peek(প্রবেশ);
						যদি ((unsigned অক্ষর)(ch - '0') > 7)
							বিরতি;
						*++p = ch = i_getch(প্রবেশ);
						nommu_addchr(as_string, ch);
					} যখন (--cnt != 0);
				} অন্যথা যদি (ch == 'x') { /* \xHH */
					করো {
						ch = i_peek(প্রবেশ);
						যদি (!isxdigit(ch))
							বিরতি;
						*++p = ch = i_getch(প্রবেশ);
						nommu_addchr(as_string, ch);
					} যখন (--cnt != 0);
					যদি (cnt == 2) { /* \x but next char is "bad" */
						ch = 'x';
						যাও unrecognized;
					}
				} /* else simple seq like \\ or \t */
				*++p = '\0';
				p = buf;
				ch = bb_process_escape_sequence((শূন্য*)&p);
				//bb_error_msg("buf:'%s' ch:%x", buf, ch);
				যদি (ch == '\0')
					চালু; /* bash compat: $'...\0...' emits nothing */
			} অন্যথা { /* unrecognized "\z": encode both chars unless ' or " */
				যদি (ch != '\'' && ch != '"') {
 unrecognized:
					o_addqchr(dest, '\\');
				}
			}
		} /* if (\...) */
		o_addqchr(dest, ch);
	}

	যদি (dest->দৈর্ঘ্য == start) {
		/* $'', $'\0', $'\000\x00' and the like */
		o_addchr(dest, SPECIAL_VAR_প্রতীক);
		o_addchr(dest, SPECIAL_VAR_প্রতীক);
	}

	ফেরত 1;
# undef as_string
}
#অন্যথা
# define parse_dollar_squote(as_string, dest, প্রবেশ) 0
#endif /* BASH_DOLLAR_SQUOTE */
#endif /* !__U_BOOT__ */

/* Return code: 0 for OK, 1 for syntax error */
#যদি BB_MMU
#define parse_dollar(as_string, dest, প্রবেশ, quote_mask) \
	parse_dollar(dest, প্রবেশ, quote_mask)
#define as_string ফাঁকা
#endif
স্থির পূর্ণসংখ্যা parse_dollar(o_string *as_string,
		o_string *dest,
		কাঠামো in_str *প্রবেশ, unsigned অক্ষর quote_mask)
{
	পূর্ণসংখ্যা ch = i_peek_and_eat_bkslash_nl(প্রবেশ);  /* first character after the $ */

	debug_printf_parse("parse_dollar entered: ch='%c' quote_mask:0x%x\n", ch, quote_mask);
	যদি (isalpha(ch)) {
 make_var:
		ch = i_getch(প্রবেশ);
		nommu_addchr(as_string, ch);
 /*make_var1:*/
		o_addchr(dest, SPECIAL_VAR_প্রতীক);
		যখন (1) {
			debug_printf_parse(": '%c'\n", ch);
			o_addchr(dest, ch | quote_mask);
			quote_mask = 0;
			ch = i_peek_and_eat_bkslash_nl(প্রবেশ);
			যদি (!isalnum(ch) && ch != '_') {
				/* End of variable name reached */
				বিরতি;
			}
			ch = i_getch(প্রবেশ);
			nommu_addchr(as_string, ch);
		}
		o_addchr(dest, SPECIAL_VAR_প্রতীক);
	} অন্যথা যদি (isdigit(ch)) {
 make_one_char_var:
		ch = i_getch(প্রবেশ);
		nommu_addchr(as_string, ch);
		o_addchr(dest, SPECIAL_VAR_প্রতীক);
		debug_printf_parse(": '%c'\n", ch);
		o_addchr(dest, ch | quote_mask);
		o_addchr(dest, SPECIAL_VAR_প্রতীক);
	} অন্যথা নির্বাচন (ch) {
#ifndef __U_BOOT__
	ক্ষেত্র '$': /* pid */
	ক্ষেত্র '!': /* last bg pid */
#endif /* !__U_BOOT__ */
	ক্ষেত্র '?': /* last exit code */
	ক্ষেত্র '#': /* number of args */
	ক্ষেত্র '*': /* args */
	ক্ষেত্র '@': /* args */
	ক্ষেত্র '-': /* $- option flags set by set builtin or shell options (-i etc) */
		যাও make_one_char_var;
	ক্ষেত্র '{': {
		অক্ষর len_single_ch;

		o_addchr(dest, SPECIAL_VAR_প্রতীক);

		ch = i_getch(প্রবেশ); /* eat '{' */
		nommu_addchr(as_string, ch);

		ch = i_getch_and_eat_bkslash_nl(প্রবেশ); /* first char after '{' */
		/* It should be ${?}, or ${#var},
		 * or even ${?+subst} - operator acting on a special variable,
		 * or the beginning of variable name.
		 */
		যদি (ch == EOF
		 || (!strchr(_SPECIAL_VARS_STR, ch) && !isalnum(ch)) /* not one of those */
		) {
 bad_dollar_syntax:
			syntax_error_unterm_str("${name}");
			debug_printf_parse("parse_dollar return 0: unterminated ${name}\n");
			ফেরত 0;
		}
		nommu_addchr(as_string, ch);
		len_single_ch = ch;
		ch |= quote_mask;

		/* It's possible to just call add_till_closing_bracket() at this point.
		 * However, this regresses some of our testsuite cases
		 * which check invalid constructs like ${%}.
		 * Oh well... let's check that the var name part is fine... */

		যদি (isdigit(len_single_ch)
		 || (len_single_ch == '#' && isdigit(i_peek_and_eat_bkslash_nl(প্রবেশ)))
		) {
			/* Execution engine uses plain xatoi_positive()
			 * to interpret ${NNN} and {#NNN},
			 * check syntax here in the parser.
			 * (bash does not support expressions in ${#NN},
			 * e.g. ${#$var} and {#1:+WORD} are not supported).
			 */
			unsigned cnt = 9; /* max 9 digits for ${NN} and 8 for {#NN} */
			যখন (1) {
				o_addchr(dest, ch);
				debug_printf_parse(": '%c'\n", ch);
				ch = i_getch_and_eat_bkslash_nl(প্রবেশ);
				nommu_addchr(as_string, ch);
				যদি (ch == '}')
					বিরতি;
				যদি (--cnt == 0)
					যাও bad_dollar_syntax;
				যদি (len_single_ch != '#' && strchr(VAR_SUBST_OPS, ch))
					/* ${NN<op>...} is valid */
					যাও eat_until_closing;
				যদি (!isdigit(ch))
					যাও bad_dollar_syntax;
			}
		} অন্যথা
		যখন (1) {
			unsigned pos;

			o_addchr(dest, ch);
			debug_printf_parse(": '%c'\n", ch);

			ch = i_getch(প্রবেশ);
			nommu_addchr(as_string, ch);
			যদি (ch == '}')
				বিরতি;
#ifndef __U_BOOT__
			যদি (!isalnum(ch) && ch != '_') {
#অন্যথা /* __U_BOOT__ */
			/*
			 * In several places in U-Boot, we use variable like
			 * foo# (e.g. serial#), particularly in env.
			 * So, we need to authorize # to appear inside
			 * variable name and then expand this variable.
			 * NOTE Having # in variable name is not permitted in
			 * upstream hush but expansion will be done (even though
			 * the result will be empty).
			 */
			যদি (!isalnum(ch) && ch != '_' && ch != '#') {
#endif /* __U_BOOT__ */
				unsigned end_ch;
#ifndef __U_BOOT__
				unsigned অক্ষর last_ch;
#endif /* !__U_BOOT__ */
				/* handle parameter expansions
				 * http://www.opengroup.org/onlinepubs/009695399/utilities/xcu_chap02.html#tag_02_06_02
				 */
				যদি (!strchr(VAR_SUBST_OPS, ch)) { /* ${var<bad_char>... */
					যদি (len_single_ch != '#'
					/*|| !strchr(SPECIAL_VARS_STR, ch) - disallow errors like ${#+} ? */
					 || i_peek(প্রবেশ) != '}'
					) {
						যাও bad_dollar_syntax;
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
				যদি (BASH_SUBSTR
				 && ch == ':'
				 && !strchr(MINUS_PLUS_EQUAL_QUESTION, i_peek(প্রবেশ))
				) {
					/* It's ${var:N[:M]} thing */
					end_ch = '}' * 0x100 + ':';
				}
				যদি (BASH_PATTERN_SUBST
				 && ch == '/'
				) {
					/* It's ${var/[/]pattern[/repl]} thing */
					যদি (i_peek(প্রবেশ) == '/') { /* ${var//pattern[/repl]}? */
						i_getch(প্রবেশ);
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
				যদি (i_peek(প্রবেশ) == '/') {
					o_addchr(dest, i_getch(প্রবেশ));
				}
#ifndef __U_BOOT__
 again:
#endif /* !__U_BOOT__ */
				যদি (!BB_MMU)
					pos = dest->দৈর্ঘ্য;
#যদি ENABLE_HUSH_DOLLAR_OPS
#ifndef __U_BOOT__
				last_ch = add_till_closing_bracket(dest, প্রবেশ, end_ch);
				যদি (last_ch == 0) /* error? */
					ফেরত 0;
#endif /* !__U_BOOT__ */
#অন্যথা
# error Simple code to only allow ${var} is not implemented
#endif
				যদি (as_string) {
					o_addstr(as_string, dest->তথ্য + pos);
#ifndef __U_BOOT__
					o_addchr(as_string, last_ch);
#endif /* !__U_BOOT__ */
				}

#ifndef __U_BOOT__
				যদি ((BASH_SUBSTR || BASH_PATTERN_SUBST)
					 && (end_ch & 0xff00)
				) {
					/* close the first block: */
					o_addchr(dest, SPECIAL_VAR_প্রতীক);
					/* while parsing N from ${var:N[:M]}
					 * or pattern from ${var/[/]pattern[/repl]} */
					যদি ((end_ch & 0xff) == last_ch) {
						/* got ':' or '/'- parse the rest */
						end_ch = '}';
						যাও again;
					}
					/* got '}' */
					যদি (BASH_SUBSTR && end_ch == '}' * 0x100 + ':') {
						/* it's ${var:N} - emulate :999999999 */
						o_addstr(dest, "999999999");
					} /* else: it's ${var/[/]pattern} */
				}
#endif /* !__U_BOOT__ */
				বিরতি;
			}
			len_single_ch = 0; /* it can't be ${#C} op */
		}
		o_addchr(dest, SPECIAL_VAR_প্রতীক);
		বিরতি;
	}
#যদি ENABLE_FEATURE_SH_MATH || ENABLE_HUSH_TICK
	ক্ষেত্র '(': {
		unsigned pos;

		ch = i_getch(প্রবেশ);
		nommu_addchr(as_string, ch);
# যদি ENABLE_FEATURE_SH_MATH
		যদি (i_peek_and_eat_bkslash_nl(প্রবেশ) == '(') {
			ch = i_getch(প্রবেশ);
			nommu_addchr(as_string, ch);
			o_addchr(dest, SPECIAL_VAR_প্রতীক);
			o_addchr(dest, quote_mask | '+');
			যদি (!BB_MMU)
				pos = dest->দৈর্ঘ্য;
			যদি (!add_till_closing_bracket(dest, প্রবেশ, ')' | DOUBLE_CLOSE_CHAR_FLAG))
				ফেরত 0; /* error */
			যদি (as_string) {
				o_addstr(as_string, dest->তথ্য + pos);
				o_addchr(as_string, ')');
				o_addchr(as_string, ')');
			}
			o_addchr(dest, SPECIAL_VAR_প্রতীক);
			বিরতি;
		}
# endif
# যদি ENABLE_HUSH_TICK
		o_addchr(dest, SPECIAL_VAR_প্রতীক);
		o_addchr(dest, quote_mask | '`');
		যদি (!BB_MMU)
			pos = dest->দৈর্ঘ্য;
		যদি (!add_till_closing_bracket(dest, প্রবেশ, ')'))
			ফেরত 0; /* error */
		যদি (as_string) {
			o_addstr(as_string, dest->তথ্য + pos);
			o_addchr(as_string, ')');
		}
		o_addchr(dest, SPECIAL_VAR_প্রতীক);
# endif
		বিরতি;
	}
#endif
	ক্ষেত্র '_':
		যাও make_var;
#যদি 0
	/* TODO: $_: */
	/* $_ Shell or shell script name; or last argument of last command
	 * (if last command wasn't a pipe; if it was, bash sets $_ to "");
	 * but in command's env, set to full pathname used to invoke it */
		ch = i_getch(প্রবেশ);
		nommu_addchr(as_string, ch);
		ch = i_peek_and_eat_bkslash_nl(প্রবেশ);
		যদি (isalnum(ch)) { /* it's $_name or $_123 */
			ch = '_';
			যাও make_var1;
		}
		/* else: it's $_ */
#endif
	পূর্বনির্ধারিত:
		o_addQchr(dest, '$');
	}
	debug_printf_parse("parse_dollar return 1 (ok)\n");
	ফেরত 1;
#undef as_string
}

#যদি BB_MMU
#define encode_string(as_string, dest, প্রবেশ, dquote_end) \
	encode_string(dest, প্রবেশ, dquote_end)
#define as_string ফাঁকা
#endif
স্থির পূর্ণসংখ্যা encode_string(o_string *as_string,
		o_string *dest,
		কাঠামো in_str *প্রবেশ,
		পূর্ণসংখ্যা dquote_end)
{
	পূর্ণসংখ্যা ch;
	পূর্ণসংখ্যা পরবর্তী;

 again:
	ch = i_getch(প্রবেশ);
	যদি (ch != EOF)
		nommu_addchr(as_string, ch);
	যদি (ch == dquote_end) { /* may be only '"' or EOF */
		debug_printf_parse("encode_string return 1 (ok)\n");
		ফেরত 1;
	}
	/* note: can't move it above ch == dquote_end check! */
	যদি (ch == EOF) {
		syntax_error_unterm_ch('"');
		ফেরত 0; /* error */
	}
	পরবর্তী = '\0';
	যদি (ch != '\n') {
		পরবর্তী = i_peek(প্রবেশ);
	}
	debug_printf_parse("\" ch=%c (%d) escape=%d\n",
			ch, ch, !!(dest->o_expflags & EXP_FLAG_ESC_GLOB_CHARS));
	যদি (ch == '\\') {
		যদি (পরবর্তী == EOF) {
			/* Testcase: in interactive shell a file with
			 *  echo "unterminated string\<eof>
			 * is sourced.
			 */
			syntax_error_unterm_ch('"');
			ফেরত 0; /* error */
		}
		/* bash:
		 * "The backslash retains its special meaning [in "..."]
		 * only when followed by one of the following characters:
		 * $, `, ", \, or <newline>.  A double quote may be quoted
		 * within double quotes by preceding it with a backslash."
		 * NB: in (unquoted) heredoc, above does not apply to ",
		 * therefore we check for it by "next == dquote_end" cond.
		 */
		যদি (পরবর্তী == dquote_end || strchr("$`\\\n", পরবর্তী)) {
			ch = i_getch(প্রবেশ); /* eat next */
			যদি (ch == '\n')
				যাও again; /* skip \<newline> */
		} /* else: ch remains == '\\', and we double it below: */
		o_addqchr(dest, ch); /* \c if c is a glob char, else just c */
		nommu_addchr(as_string, ch);
		যাও again;
	}
	যদি (ch == '$') {
		//if (parse_dollar_squote(as_string, dest, input))
		//	goto again;
		যদি (!parse_dollar(as_string, dest, প্রবেশ, /*quote_mask:*/ 0x80)) {
			debug_printf_parse("encode_string return 0: "
					"parse_dollar returned 0 (error)\n");
			ফেরত 0;
		}
		যাও again;
	}
#যদি ENABLE_HUSH_TICK
	যদি (ch == '`') {
		//unsigned pos = dest->length;
		o_addchr(dest, SPECIAL_VAR_প্রতীক);
		o_addchr(dest, 0x80 | '`');
		যদি (!add_till_backquote(dest, প্রবেশ, /*in_dquote:*/ dquote_end == '"'))
			ফেরত 0; /* error */
		o_addchr(dest, SPECIAL_VAR_প্রতীক);
		//debug_printf_subst("SUBST RES3 '%s'\n", dest->data + pos);
		যাও again;
	}
#endif
	o_addQchr(dest, ch);
	যদি (ch == SPECIAL_VAR_প্রতীক) {
		/* Convert "^C" to corresponding special variable reference */
		o_addchr(dest, SPECIAL_VAR_QUOTED_SVS);
		o_addchr(dest, SPECIAL_VAR_প্রতীক);
	}
	যাও again;
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
স্থির কাঠামো pipe *parse_stream(অক্ষর **pstring,
		পূর্ণসংখ্যা *heredoc_cnt_ptr,
		কাঠামো in_str *প্রবেশ,
		পূর্ণসংখ্যা end_trigger)
{
	কাঠামো parse_context ctx;
	পূর্ণসংখ্যা heredoc_cnt;

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
	ctx.word.তথ্য = xzalloc(1); /* start as "", not as NULL */

	/* We used to separate words on $IFS here. This was wrong.
	 * $IFS is used only for word splitting when $var is expanded,
	 * here we should use blank chars as separators, not $IFS
	 */

	heredoc_cnt = 0;
	যখন (1) {
		ধ্রুবক অক্ষর *is_blank;
		ধ্রুবক অক্ষর *is_special;
		পূর্ণসংখ্যা ch;
		পূর্ণসংখ্যা পরবর্তী;
#ifndef __U_BOOT__
		পূর্ণসংখ্যা redir_fd;
		redir_type redir_style;
#endif /* !__U_BOOT__ */

		ch = i_getch(প্রবেশ);
		debug_printf_parse(": ch=%c (%d) escape=%d\n",
				ch, ch, !!(ctx.word.o_expflags & EXP_FLAG_ESC_GLOB_CHARS));
		যদি (ch == EOF) {
			কাঠামো pipe *pi;

			যদি (heredoc_cnt) {
				syntax_error_unterm_str("here document");
				যাও parse_error_exitcode1;
			}
			যদি (end_trigger == ')') {
				syntax_error_unterm_ch('(');
				যাও parse_error_exitcode1;
			}
			যদি (end_trigger == '}') {
				syntax_error_unterm_ch('{');
				যাও parse_error_exitcode1;
			}

			যদি (done_word(&ctx)) {
				যাও parse_error_exitcode1;
			}
			o_free_and_set_NULL(&ctx.word);
			done_pipe(&ctx, PIPE_SEQ);

			/* Do we sit inside of any if's, loops or case's? */
			যদি (HAS_KEYWORDS
			IF_HAS_KEYWORDS(&& (ctx.ctx_res_w != RES_NONE || ctx.old_flag != 0))
			) {
				syntax_error_unterm_str("compound statement");
				যাও parse_error_exitcode1;
			}

			pi = ctx.list_শীর্ষ;
			/* If we got nothing... */
			/* (this makes bare "&" cmd a no-op.
			 * bash says: "syntax error near unexpected token '&'") */
			যদি (pi->অঙ্ক_cmds == 0
			IF_HAS_KEYWORDS(&& pi->res_word == RES_NONE)
			) {
				free_pipe_list(pi);
				pi = ফাঁকা;
			}
#যদি !BB_MMU
			debug_printf_parse("as_string1 '%s'\n", ctx.as_string.তথ্য);
			যদি (pstring)
				*pstring = ctx.as_string.তথ্য;
			অন্যথা
				o_free(&ctx.as_string);
#endif
			// heredoc_cnt must be 0 here anyway
			//if (heredoc_cnt_ptr)
			//	*heredoc_cnt_ptr = heredoc_cnt;
			debug_leave();
			debug_printf_heredoc("parse_stream return heredoc_cnt:%d\n", heredoc_cnt);
			debug_printf_parse("parse_stream return %p: EOF\n", pi);
			ফেরত pi;
		}

		/* Handle "'" and "\" first, as they won't play nice with
		 * i_peek_and_eat_bkslash_nl() anyway:
		 *   echo z\\
		 * and
		 *   echo '\
		 *   '
		 * would break.
		 */
		যদি (ch == '\\') {
			ch = i_getch(প্রবেশ);
			যদি (ch == '\n')
				চালু; /* drop \<newline>, get next char */
			nommu_addchr(&ctx.as_string, '\\');
			যদি (ch == SPECIAL_VAR_প্রতীক) {
				nommu_addchr(&ctx.as_string, ch);
				/* Convert \^C to corresponding special variable reference */
				যাও case_SPECIAL_VAR_প্রতীক;
			}
			o_addchr(&ctx.word, '\\');
			যদি (ch == EOF) {
				/* Testcase: eval 'echo Ok\' */
				/* bash-4.3.43 was removing backslash,
				 * but 4.4.19 retains it, most other shells too
				 */
				চালু; /* get next char */
			}
			/* Example: echo Hello \2>file
			 * we need to know that word 2 is quoted
			 */
			ctx.word.has_quoted_part = 1;
			nommu_addchr(&ctx.as_string, ch);
			o_addchr(&ctx.word, ch);
			চালু; /* get next char */
		}
		nommu_addchr(&ctx.as_string, ch);
		যদি (ch == '\'') {
			ctx.word.has_quoted_part = 1;
			পরবর্তী = i_getch(প্রবেশ);
#ifndef __U_BOOT__
			যদি (পরবর্তী == '\'' && !ctx.pending_redirect)
				যাও ঢোকাও_empty_quoted_str_marker;
#endif /* !__U_BOOT__ */

			ch = পরবর্তী;
			যখন (1) {
				যদি (ch == EOF) {
					syntax_error_unterm_ch('\'');
					যাও parse_error_exitcode1;
				}
				nommu_addchr(&ctx.as_string, ch);
				যদি (ch == '\'')
					বিরতি;
				যদি (ch == SPECIAL_VAR_প্রতীক) {
					/* Convert raw ^C to corresponding special variable reference */
					o_addchr(&ctx.word, SPECIAL_VAR_প্রতীক);
					o_addchr(&ctx.word, SPECIAL_VAR_QUOTED_SVS);
				}
				o_addqchr(&ctx.word, ch);
				ch = i_getch(প্রবেশ);
			}
			চালু; /* get next char */
		}

		পরবর্তী = '\0';
		যদি (ch != '\n')
			পরবর্তী = i_peek_and_eat_bkslash_nl(প্রবেশ);

		is_special = "{}<>&|();#" /* special outside of "str" */
#ifndef __U_BOOT__
				"$\"" IF_HUSH_TICK("`") /* always special */
#অন্যথা /* __U_BOOT__ */
				"$\""
#endif /* __U_BOOT__ */
				SPECIAL_VAR_প্রতীক_STR;
#যদি defined(CMD_TEST2_SINGLEWORD_NOGLOB)
		যদি (ctx.command->cmd_type == CMD_TEST2_SINGLEWORD_NOGLOB) {
			/* In [[ ]], {}<>&|() are not special */
			is_special += 8;
		} অন্যথা
#endif
		/* Are { and } special here? */
		যদি (ctx.command->argv /* word [word]{... - non-special */
		 || ctx.word.দৈর্ঘ্য       /* word{... - non-special */
		 || ctx.word.has_quoted_part     /* ""{... - non-special */
		 || (পরবর্তী != ';'             /* }; - special */
		    && পরবর্তী != ')'           /* }) - special */
		    && পরবর্তী != '('           /* {( - special */
		    && পরবর্তী != '&'           /* }& and }&& ... - special */
		    && পরবর্তী != '|'           /* }|| ... - special */
		    && !strchr(defifs, পরবর্তী) /* {word - non-special */
		    )
		) {
			/* They are not special, skip "{}" */
			is_special += 2;
		}
		is_special = strchr(is_special, ch);
		is_blank = strchr(defifs, ch);

		যদি (!is_special && !is_blank) { /* ordinary char */
 ordinary_char:
			o_addQchr(&ctx.word, ch);
			যদি ((ctx.is_assignment == MAYBE_ASSIGNMENT
			    || ctx.is_assignment == WORD_IS_KEYWORD)
			 && ch == '='
			 && endofname(ctx.word.তথ্য)[0] == '='
			) {
				ctx.is_assignment = DEFINITELY_ASSIGNMENT;
				debug_printf_parse("ctx.is_assignment='%s'\n", assignment_flag[ctx.is_assignment]);
			}
			চালু;
		}

		যদি (is_blank) {
#যদি ENABLE_HUSH_LINENO_VAR
/* Case:
 * "while ...; do<whitespace><newline>
 *	cmd ..."
 * would think that "cmd" starts in <whitespace> -
 * i.e., at the previous line.
 * We need to skip all whitespace before newlines.
 */
			যখন (ch != '\n') {
				পরবর্তী = i_peek(প্রবেশ);
				যদি (পরবর্তী != ' ' && পরবর্তী != '\t' && পরবর্তী != '\n')
					বিরতি; /* next char is not ws */
				ch = i_getch(প্রবেশ);
			}
			/* ch == last eaten whitespace char */
#endif
			যদি (done_word(&ctx)) {
				যাও parse_error_exitcode1;
			}
			যদি (ch == '\n') {
				/* Is this a case when newline is simply ignored?
				 * Some examples:
				 * "cmd | <newline> cmd ..."
				 * "case ... in <newline> word) ..."
				 */
				যদি (IS_NULL_CMD(ctx.command)
				 && ctx.word.দৈর্ঘ্য == 0
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
					কাঠামো pipe *pi = ctx.list_শীর্ষ;
					যদি (pi->অঙ্ক_cmds != 0       /* check #1 */
					 && pi->followup != PIPE_BG /* check #2 */
					) {
						চালু;
					}
				}
				/* Treat newline as a command separator. */
				done_pipe(&ctx, PIPE_SEQ);
				debug_printf_heredoc("heredoc_cnt:%d\n", heredoc_cnt);
				যদি (heredoc_cnt) {
					heredoc_cnt = fetch_heredocs(&ctx.as_string, ctx.list_শীর্ষ, heredoc_cnt, প্রবেশ);
					যদি (heredoc_cnt != 0)
						যাও parse_error_exitcode1;
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
		যদি (ch == '}') {
#অন্যথা /* __U_BOOT__ */
		যদি (ch == '}' || ch == ')') {
#endif /* __U_BOOT__ */
			যদি (ctx.word.দৈর্ঘ্য != 0 /* word} */
			 || ctx.word.has_quoted_part    /* ""} */
			) {
				যাও ordinary_char;
			}
			যদি (!IS_NULL_CMD(ctx.command)) { /* cmd } */
				/* Generally, there should be semicolon: "cmd; }"
				 * However, bash allows to omit it if "cmd" is
				 * a group. Examples:
				 * { { echo 1; } }
				 * {(echo 1)}
				 * { echo 0 >&2 | { echo 1; } }
				 * { while false; do :; done }
				 * { case a in b) ;; esac }
				 */
				যদি (ctx.command->group)
					যাও term_group;
				যাও ordinary_char;
			}
			যদি (!IS_NULL_PIPE(ctx.pipe)) /* cmd | } */
				/* Can't be an end of {cmd}, skip the check */
				যাও skip_end_trigger;
			/* else: } does terminate a group */
		}
 term_group:
		যদি (end_trigger && end_trigger == ch
		 && (ch != ';' || heredoc_cnt == 0)
#যদি ENABLE_HUSH_CASE
		 && (ch != ')'
		    || ctx.ctx_res_w != RES_MATCH
		    || (!ctx.word.has_quoted_part && strcmp(ctx.word.তথ্য, "esac") == 0)
		    )
#endif
		) {
			যদি (done_word(&ctx)) {
				যাও parse_error_exitcode1;
			}
			done_pipe(&ctx, PIPE_SEQ);
			ctx.is_assignment = MAYBE_ASSIGNMENT;
			debug_printf_parse("ctx.is_assignment='%s'\n", assignment_flag[ctx.is_assignment]);
			/* Do we sit outside of any if's, loops or case's? */
			যদি (!HAS_KEYWORDS
			IF_HAS_KEYWORDS(|| (ctx.ctx_res_w == RES_NONE && ctx.old_flag == 0))
			) {
				o_free_and_set_NULL(&ctx.word);
#যদি !BB_MMU
				debug_printf_parse("as_string2 '%s'\n", ctx.as_string.তথ্য);
				যদি (pstring)
					*pstring = ctx.as_string.তথ্য;
				অন্যথা
					o_free(&ctx.as_string);
#endif
				যদি (ch != ';' && IS_NULL_PIPE(ctx.list_শীর্ষ)) {
					/* Example: bare "{ }", "()" */
					G.last_exitcode = 2; /* bash compat */
					syntax_error_unexpected_ch(ch);
					যাও parse_error;
				}
				যদি (heredoc_cnt_ptr)
					*heredoc_cnt_ptr = heredoc_cnt;
				debug_printf_heredoc("parse_stream return heredoc_cnt:%d\n", heredoc_cnt);
				debug_printf_parse("parse_stream return %p: "
						"end_trigger char found\n",
						ctx.list_শীর্ষ);
				debug_leave();
				ফেরত ctx.list_শীর্ষ;
			}
		}

		যদি (is_blank)
			চালু;

		/* Catch <, > before deciding whether this word is
		 * an assignment. a=1 2>z b=2: b=2 is still assignment */
		নির্বাচন (ch) {
#ifndef __U_BOOT__
		ক্ষেত্র '>':
			redir_fd = redirect_opt_অঙ্ক(&ctx.word);
			যদি (done_word(&ctx)) {
				যাও parse_error_exitcode1;
			}
			redir_style = REDIRECT_OVERWRITE;
			যদি (পরবর্তী == '>') {
				redir_style = REDIRECT_APPEND;
				ch = i_getch(প্রবেশ);
				nommu_addchr(&ctx.as_string, ch);
			}
#যদি 0
			অন্যথা যদি (পরবর্তী == '(') {
				syntax_error(">(process) not supported");
				যাও parse_error_exitcode1;
			}
#endif
			যদি (parse_redirect(&ctx, redir_fd, redir_style, প্রবেশ))
				যাও parse_error_exitcode1;
			চালু; /* get next char */
		ক্ষেত্র '<':
			redir_fd = redirect_opt_অঙ্ক(&ctx.word);
			যদি (done_word(&ctx)) {
				যাও parse_error_exitcode1;
			}
			redir_style = REDIRECT_প্রবেশ;
			যদি (পরবর্তী == '<') {
				redir_style = REDIRECT_HEREDOC;
				heredoc_cnt++;
				debug_printf_heredoc("++heredoc_cnt=%d\n", heredoc_cnt);
				ch = i_getch(প্রবেশ);
				nommu_addchr(&ctx.as_string, ch);
			} অন্যথা যদি (পরবর্তী == '>') {
				redir_style = REDIRECT_IO;
				ch = i_getch(প্রবেশ);
				nommu_addchr(&ctx.as_string, ch);
			}
#যদি 0
			অন্যথা যদি (পরবর্তী == '(') {
				syntax_error("<(process) not supported");
				যাও parse_error_exitcode1;
			}
#endif
			যদি (parse_redirect(&ctx, redir_fd, redir_style, প্রবেশ))
				যাও parse_error_exitcode1;
			চালু; /* get next char */
#অন্যথা /* __U_BOOT__ */
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
			ক্ষেত্র '>':
				fallthrough;
			ক্ষেত্র '<':
				o_addQchr(&ctx.word, ch);
				চালু;
#endif /* __U_BOOT__ */
		ক্ষেত্র '#':
			যদি (ctx.word.দৈর্ঘ্য == 0 && !ctx.word.has_quoted_part) {
				/* skip "#comment" */
				/* note: we do not add it to &ctx.as_string */
/* TODO: in bash:
 * comment inside $() goes to the next \n, even inside quoted string (!):
 * cmd "$(cmd2 #comment)" - syntax error
 * cmd "`cmd2 #comment`" - ok
 * We accept both (comment ends where command subst ends, in both cases).
 */
				যখন (1) {
					ch = i_peek(প্রবেশ);
					যদি (ch == '\n') {
						nommu_addchr(&ctx.as_string, '\n');
						বিরতি;
					}
					ch = i_getch(প্রবেশ);
					যদি (ch == EOF)
						বিরতি;
				}
				চালু; /* get next char */
			}
			বিরতি;
		}
 skip_end_trigger:

		যদি (ctx.is_assignment == MAYBE_ASSIGNMENT
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

		নির্বাচন (ch) {
		case_SPECIAL_VAR_প্রতীক:
		ক্ষেত্র SPECIAL_VAR_প্রতীক:
			/* Convert raw ^C to corresponding special variable reference */
			o_addchr(&ctx.word, SPECIAL_VAR_প্রতীক);
			o_addchr(&ctx.word, SPECIAL_VAR_QUOTED_SVS);
			/* fall through */
		ক্ষেত্র '#':
			/* non-comment #: "echo a#b" etc */
			o_addchr(&ctx.word, ch);
			চালু; /* get next char */
		ক্ষেত্র '$':
#ifndef __U_BOOT__
			যদি (parse_dollar_squote(&ctx.as_string, &ctx.word, প্রবেশ))
				চালু; /* get next char */
#endif /* !__U_BOOT__ */
			যদি (!parse_dollar(&ctx.as_string, &ctx.word, প্রবেশ, /*quote_mask:*/ 0)) {
				debug_printf_parse("parse_stream parse error: "
					"parse_dollar returned 0 (error)\n");
				যাও parse_error_exitcode1;
			}
			চালু; /* get next char */
		ক্ষেত্র '"':
			ctx.word.has_quoted_part = 1;
#ifndef __U_BOOT__
			যদি (পরবর্তী == '"' && !ctx.pending_redirect) {
#অন্যথা /* __U_BOOT__ */
			যদি (পরবর্তী == '"') {
#endif /* __U_BOOT__ */
				i_getch(প্রবেশ); /* eat second " */
#ifndef __U_BOOT__
 ঢোকাও_empty_quoted_str_marker:
#endif /* !__U_BOOT__ */
				nommu_addchr(&ctx.as_string, পরবর্তী);
				o_addchr(&ctx.word, SPECIAL_VAR_প্রতীক);
				o_addchr(&ctx.word, SPECIAL_VAR_প্রতীক);
				চালু; /* get next char */
			}
			যদি (ctx.is_assignment == NOT_ASSIGNMENT)
				ctx.word.o_expflags |= EXP_FLAG_ESC_GLOB_CHARS;
			যদি (!encode_string(&ctx.as_string, &ctx.word, প্রবেশ, '"'))
				যাও parse_error_exitcode1;
			ctx.word.o_expflags &= ~EXP_FLAG_ESC_GLOB_CHARS;
			চালু; /* get next char */
#যদি ENABLE_HUSH_TICK
		ক্ষেত্র '`': {
			USE_FOR_NOMMU(unsigned pos;)

			o_addchr(&ctx.word, SPECIAL_VAR_প্রতীক);
			o_addchr(&ctx.word, '`');
			USE_FOR_NOMMU(pos = ctx.word.দৈর্ঘ্য;)
			যদি (!add_till_backquote(&ctx.word, প্রবেশ, /*in_dquote:*/ 0))
				যাও parse_error_exitcode1;
# যদি !BB_MMU
			o_addstr(&ctx.as_string, ctx.word.তথ্য + pos);
			o_addchr(&ctx.as_string, '`');
# endif
			o_addchr(&ctx.word, SPECIAL_VAR_প্রতীক);
			//debug_printf_subst("SUBST RES3 '%s'\n", ctx.word.data + pos);
			চালু; /* get next char */
		}
#endif
		ক্ষেত্র ';':
#যদি ENABLE_HUSH_CASE
 case_semi:
#endif
			যদি (done_word(&ctx)) {
				যাও parse_error_exitcode1;
			}
			done_pipe(&ctx, PIPE_SEQ);
#যদি ENABLE_HUSH_CASE
			/* Eat multiple semicolons, detect
			 * whether it means something special */
			যখন (1) {
				ch = i_peek_and_eat_bkslash_nl(প্রবেশ);
				যদি (ch != ';')
					বিরতি;
				ch = i_getch(প্রবেশ);
				nommu_addchr(&ctx.as_string, ch);
				যদি (ctx.ctx_res_w == RES_CASE_BODY) {
					ctx.ctx_dsemicolon = 1;
					ctx.ctx_res_w = RES_MATCH;
					বিরতি;
				}
			}
#endif
 new_cmd:
			/* We just finished a cmd. New one may start
			 * with an assignment */
			ctx.is_assignment = MAYBE_ASSIGNMENT;
			debug_printf_parse("ctx.is_assignment='%s'\n", assignment_flag[ctx.is_assignment]);
			চালু; /* get next char */
		ক্ষেত্র '&':
			যদি (done_word(&ctx)) {
				যাও parse_error_exitcode1;
			}
			যদি (পরবর্তী == '&') {
				ch = i_getch(প্রবেশ);
				nommu_addchr(&ctx.as_string, ch);
				done_pipe(&ctx, PIPE_AND);
			} অন্যথা {
				done_pipe(&ctx, PIPE_BG);
			}
			যাও new_cmd;
		ক্ষেত্র '|':
			যদি (done_word(&ctx)) {
				যাও parse_error_exitcode1;
			}
#যদি ENABLE_HUSH_CASE
			যদি (ctx.ctx_res_w == RES_MATCH)
				বিরতি; /* we are in case's "word | word)" */
#endif
			যদি (পরবর্তী == '|') { /* || */
				ch = i_getch(প্রবেশ);
				nommu_addchr(&ctx.as_string, ch);
				done_pipe(&ctx, PIPE_OR);
			} অন্যথা {
				/* we could pick up a file descriptor choice here
				 * with redirect_opt_num(), but bash doesn't do it.
				 * "echo foo 2| cat" yields "foo 2". */
				done_command(&ctx);
			}
			যাও new_cmd;
		ক্ষেত্র '(':
#যদি ENABLE_HUSH_CASE
			/* "case... in [(]word)..." - skip '(' */
			যদি (ctx.ctx_res_w == RES_MATCH
			 && ctx.command->argv == ফাঁকা /* not (word|(... */
			 && ctx.word.দৈর্ঘ্য == 0 /* not word(... */
			 && ctx.word.has_quoted_part == 0 /* not ""(... */
			) {
				চালু; /* get next char */
			}
#endif
			/* fall through */
		ক্ষেত্র '{': {
			পূর্ণসংখ্যা n = parse_group(&ctx, প্রবেশ, ch);
			যদি (n < 0) {
				যাও parse_error_exitcode1;
			}
			debug_printf_heredoc("parse_group done, needs heredocs:%d\n", n);
			heredoc_cnt += n;
			যাও new_cmd;
		}
		ক্ষেত্র ')':
#যদি ENABLE_HUSH_CASE
			যদি (ctx.ctx_res_w == RES_MATCH)
				যাও case_semi;
#endif
		ক্ষেত্র '}':
			/* proper use of this character is caught by end_trigger:
			 * if we see {, we call parse_group(..., end_trigger='}')
			 * and it will match } earlier (not here). */
			G.last_exitcode = 2;
			syntax_error_unexpected_ch(ch);
			যাও parse_error;
		পূর্বনির্ধারিত:
			যদি (HUSH_DEBUG)
				bb_error_বার্তা_and_die("BUG: unexpected %c", ch);
		}
	} /* while (1) */

 parse_error_exitcode1:
	G.last_exitcode = 1;
 parse_error:
	{
		কাঠামো parse_context *pctx;
		IF_HAS_KEYWORDS(কাঠামো parse_context *p2;)

		/* Clean up allocated tree.
		 * Sample for finding leaks on syntax error recovery path.
		 * Run it from interactive shell, watch pmap `pidof hush`.
		 * while if false; then false; fi; do break; fi
		 * Samples to catch leaks at execution:
		 * while if (true | { true;}); then echo ok; fi; do break; done
		 * while if (true | { true;}); then echo ok; fi; do (if echo ok; break; then :; fi) | cat; break; done
		 */
		pctx = &ctx;
		করো {
			/* Update pipe/command counts,
			 * otherwise freeing may miss some */
			done_pipe(pctx, PIPE_SEQ);
			debug_printf_clean("freeing list %p from ctx %p\n",
					pctx->list_শীর্ষ, pctx);
			debug_print_বৃক্ষ(pctx->list_শীর্ষ, 0);
			free_pipe_list(pctx->list_শীর্ষ);
			debug_printf_clean("freed list %p\n", pctx->list_শীর্ষ);
#যদি !BB_MMU
			o_free(&pctx->as_string);
#endif
			IF_HAS_KEYWORDS(p2 = pctx->স্তূপ;)
			যদি (pctx != &ctx) {
				মুক্ত(pctx);
			}
			IF_HAS_KEYWORDS(pctx = p2;)
		} যখন (HAS_KEYWORDS && pctx);

		o_free(&ctx.word);
#যদি !BB_MMU
		যদি (pstring)
			*pstring = ফাঁকা;
#endif
		debug_leave();
		ফেরত ERR_PTR;
	}
}

/*** Execution routines ***/

/* Expansion can recurse, need forward decls: */
#যদি !BASH_PATTERN_SUBST && !ENABLE_HUSH_CASE
#define expand_string_to_string(str, EXP_flags, do_unbackslash) \
	expand_string_to_string(str)
#endif
স্থির অক্ষর *expand_string_to_string(ধ্রুবক অক্ষর *str, পূর্ণসংখ্যা EXP_flags, পূর্ণসংখ্যা do_unbackslash);
#যদি ENABLE_HUSH_TICK
স্থির পূর্ণসংখ্যা process_command_subs(o_string *dest, ধ্রুবক অক্ষর *s);
#endif
স্থির পূর্ণসংখ্যা expand_vars_to_list(o_string *output, পূর্ণসংখ্যা n, অক্ষর *arg);

/* expand_strvec_to_strvec() takes a list of strings, expands
 * all variable references within and returns a pointer to
 * a list of expanded strings, possibly with larger number
 * of strings. (Think VAR="a b"; echo $VAR).
 * This new list is allocated as a single malloc block.
 * NULL-terminated list of char* pointers is at the beginning of it,
 * followed by strings themselves.
 * Caller can deallocate entire list by single free(list). */

/* A horde of its helpers come first: */

স্থির শূন্য o_addblock_duplicate_backslash(o_string *o, ধ্রুবক অক্ষর *str, পূর্ণসংখ্যা len)
{
	যখন (--len >= 0) {
		অক্ষর c = *str++;

#যদি ENABLE_HUSH_BRACE_EXPANSION
		যদি (c == '{' || c == '}') {
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
		যদি (c == '\\') {
			/* \z -> \\\z; \<eol> -> \\<eol> */
			o_addchr(o, '\\');
			যদি (len) {
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
স্থির পূর্ণসংখ্যা expand_on_ifs(o_string *output, পূর্ণসংখ্যা n, ধ্রুবক অক্ষর *str)
{
	পূর্ণসংখ্যা last_is_ifs = 0;

	যখন (1) {
		পূর্ণসংখ্যা word_len;

		যদি (!*str)  /* EOL - do not finalize word */
			বিরতি;
		word_len = strcspn(str, G.ifs);
		যদি (word_len) {
			/* We have WORD_LEN leading non-IFS chars */
			যদি (!(output->o_expflags & EXP_FLAG_GLOB)) {
				o_addblock(output, str, word_len);
			} অন্যথা {
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
			যদি (!*str)  /* EOL - do not finalize word */
				বিরতি;
		}

		/* We know str here points to at least one IFS char */
		last_is_ifs = 1;
		str += strspn(str, G.ifs_whitespace); /* skip IFS whitespace chars */
		যদি (!*str)  /* EOL - do not finalize word */
			বিরতি;

		যদি (G.ifs_whitespace != G.ifs /* usually false ($IFS is usually all whitespace), */
		 && strchr(G.ifs, *str)       /* the second check would fail */
		) {
			/* This is a non-whitespace $IFS char */
			/* Skip it and IFS whitespace chars, start new word */
			str++;
			str += strspn(str, G.ifs_whitespace);
			যাও new_word;
		}

		/* Start new word... but not always! */
		/* Case "v=' a'; echo ''$v": we do need to finalize empty word: */
		যদি (output->has_quoted_part
		/*
		 * Case "v=' a'; echo $v":
		 * here nothing precedes the space in $v expansion,
		 * therefore we should not finish the word
		 * (IOW: if there *is* word to finalize, only then do it):
		 * It's okay if this accesses the byte before first argv[]:
		 * past call to o_save_ptr() cleared it to zero byte
		 * (grep for -prev-ifs-check-).
		 */
		 || output->তথ্য[output->দৈর্ঘ্য - 1]
		) {
 new_word:
			o_addchr(output, '\0');
			debug_print_list("expand_on_ifs", output, n);
			n = o_save_ptr(output, n);
		}
	}

	output->ended_in_ifs = last_is_ifs;
	debug_print_list("expand_on_ifs[1]", output, n);
	ফেরত n;
}

#ifndef __U_BOOT__
/* Helper to expand $((...)) and heredoc body. These act as if
 * they are in double quotes, with the exception that they are not :).
 * Just the rules are similar: "expand only $var and `cmd`"
 *
 * Returns malloced string.
 * As an optimization, we return NULL if expansion is not needed.
 */
স্থির অক্ষর *encode_then_expand_string(ধ্রুবক অক্ষর *str)
{
	অক্ষর *exp_str;
	কাঠামো in_str প্রবেশ;
	o_string dest = NULL_O_STRING;
	ধ্রুবক অক্ষর *cp;

	cp = str;
	জন্য (;;) {
		যদি (!*cp) ফেরত ফাঁকা; /* string has no special chars */
		যদি (*cp == '$') বিরতি;
		যদি (*cp == '\\') বিরতি;
#যদি ENABLE_HUSH_TICK
		যদি (*cp == '`') বিরতি;
#endif
		cp++;
	}

	/* We need to expand. Example:
	 * echo $(($a + `echo 1`)) $((1 + $((2)) ))
	 */
	setup_string_in_str(&প্রবেশ, str);
	encode_string(ফাঁকা, &dest, &প্রবেশ, EOF);
//TODO: error check (encode_string returns 0 on error)?
	//bb_error_msg("'%s' -> '%s'", str, dest.data);
	exp_str = expand_string_to_string(dest.তথ্য,
			EXP_FLAG_ESC_GLOB_CHARS,
			/*unbackslash:*/ 1
	);
	//bb_error_msg("'%s' -> '%s'", dest.data, exp_str);
	o_free(&dest);
	ফেরত exp_str;
}

স্থির ধ্রুবক অক্ষর *first_special_char_in_vararg(ধ্রুবক অক্ষর *cp)
{
	জন্য (;;) {
		যদি (!*cp) ফেরত ফাঁকা; /* string has no special chars */
		যদি (*cp == '$') ফেরত cp;
		যদি (*cp == '\\') ফেরত cp;
		যদি (*cp == '\'') ফেরত cp;
		যদি (*cp == '"') ফেরত cp;
#যদি ENABLE_HUSH_TICK
		যদি (*cp == '`') ফেরত cp;
#endif
		/* dquoted "${x:+ARG}" should not glob, therefore
		 * '*' et al require some non-literal processing: */
		যদি (*cp == '*') ফেরত cp;
		যদি (*cp == '?') ফেরত cp;
		যদি (*cp == '[') ফেরত cp;
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
#যদি !BASH_PATTERN_SUBST
#define encode_then_expand_vararg(str, handle_squotes, do_unbackslash) \
	encode_then_expand_vararg(str, handle_squotes)
#endif
স্থির অক্ষর *encode_then_expand_vararg(ধ্রুবক অক্ষর *str, পূর্ণসংখ্যা handle_squotes, পূর্ণসংখ্যা do_unbackslash)
{
#যদি !BASH_PATTERN_SUBST && ENABLE_HUSH_CASE
	ধ্রুবক পূর্ণসংখ্যা do_unbackslash = 0;
#endif
	অক্ষর *exp_str;
	কাঠামো in_str প্রবেশ;
	o_string dest = NULL_O_STRING;

	যদি (!first_special_char_in_vararg(str)) {
		/* string has no special chars */
		ফেরত ফাঁকা;
	}

	setup_string_in_str(&প্রবেশ, str);
	dest.তথ্য = xzalloc(1); /* start as "", not as NULL */
	exp_str = ফাঁকা;

	জন্য (;;) {
		পূর্ণসংখ্যা ch;

		ch = i_getch(&প্রবেশ);
		debug_printf_parse("%s: ch=%c (%d) escape=%d\n",
				__func__, ch, ch, !!dest.o_expflags);

		যদি (!dest.o_expflags) {
			যদি (ch == EOF)
				বিরতি;
			যদি (handle_squotes && ch == '\'') {
				যদি (!add_till_single_quote_dquoted(&dest, &প্রবেশ))
					যাও ret; /* error */
				চালু;
			}
		}
		যদি (ch == EOF) {
			syntax_error_unterm_ch('"');
			যাও ret; /* error */
		}
		যদি (ch == '"') {
			dest.o_expflags ^= EXP_FLAG_ESC_GLOB_CHARS;
			চালু;
		}
		যদি (ch == '\\') {
			ch = i_getch(&প্রবেশ);
			যদি (ch == EOF) {
//example? error message?	syntax_error_unterm_ch('"');
				debug_printf_parse("%s: error: \\<eof>\n", __func__);
				যাও ret;
			}
			o_addqchr(&dest, ch);
			চালু;
		}
		যদি (ch == '$') {
			যদি (parse_dollar_squote(ফাঁকা, &dest, &প্রবেশ))
				চালু;
			যদি (!parse_dollar(ফাঁকা, &dest, &প্রবেশ, /*quote_mask:*/ 0x80)) {
				debug_printf_parse("%s: error: parse_dollar returned 0 (error)\n", __func__);
				যাও ret;
			}
			চালু;
		}
#যদি ENABLE_HUSH_TICK
		যদি (ch == '`') {
			//unsigned pos = dest->length;
			o_addchr(&dest, SPECIAL_VAR_প্রতীক);
			o_addchr(&dest, 0x80 | '`');
			যদি (!add_till_backquote(&dest, &প্রবেশ,
					/*in_dquote:*/ dest.o_expflags /* nonzero if EXP_FLAG_ESC_GLOB_CHARS set */
				)
			) {
				যাও ret; /* error */
			}
			o_addchr(&dest, SPECIAL_VAR_প্রতীক);
			//debug_printf_subst("SUBST RES3 '%s'\n", dest->data + pos);
			চালু;
		}
#endif
		o_addQchr(&dest, ch);
	} /* for (;;) */

	debug_printf_parse("encode: '%s' -> '%s'\n", str, dest.তথ্য);
	exp_str = expand_string_to_string(dest.তথ্য,
			do_unbackslash ? EXP_FLAG_ESC_GLOB_CHARS : 0,
			do_unbackslash
	);
 ret:
	debug_printf_parse("expand: '%s' -> '%s'\n", dest.তথ্য, exp_str);
	o_free(&dest);
	ফেরত exp_str;
}

/* Expanding ARG in ${var+ARG}, ${var-ARG}
 */
স্থির NOINLINE পূর্ণসংখ্যা encode_then_append_var_plusminus(o_string *output, পূর্ণসংখ্যা n,
		অক্ষর *str, পূর্ণসংখ্যা dquoted)
{
	কাঠামো in_str প্রবেশ;
	o_string dest = NULL_O_STRING;

	যদি (!first_special_char_in_vararg(str)
	 && '\0' == str[strcspn(str, G.ifs)]
	) {
		/* string has no special chars
		 * && string has no $IFS chars
		 */
		যদি (dquoted) {
			/* Prints 1 (quoted expansion is a "" word, not nothing):
			 * set -- "${notexist-}"; echo $#
			 */
			output->has_quoted_part = 1;
		}
		ফেরত expand_vars_to_list(output, n, str);
	}

	setup_string_in_str(&প্রবেশ, str);

	জন্য (;;) {
		পূর্ণসংখ্যা ch;

		ch = i_getch(&প্রবেশ);
		debug_printf_parse("%s: ch=%c (%d) escape=%x\n",
				__func__, ch, ch, dest.o_expflags);

		যদি (!dest.o_expflags) {
			যদি (ch == EOF)
				বিরতি;
			যদি (!dquoted && !(output->o_expflags & EXP_FLAG_SINGLEWORD) && strchr(G.ifs, ch)) {
				/* PREFIX${x:d${e}f ...} and we met space: expand "d${e}f" and start new word.
				 * do not assume we are at the start of the word (PREFIX above).
				 */
				যদি (dest.তথ্য) {
					n = expand_vars_to_list(output, n, dest.তথ্য);
					o_free_and_set_NULL(&dest);
					o_addchr(output, '\0');
					n = o_save_ptr(output, n); /* create next word */
				} অন্যথা
				যদি (output->দৈর্ঘ্য != o_get_last_ptr(output, n)
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
				চালু;
			}
			যদি (!dquoted && ch == '\'') {
				যদি (!add_till_single_quote_dquoted(&dest, &প্রবেশ))
					যাও ret; /* error */
				o_addchr(&dest, SPECIAL_VAR_প্রতীক);
				o_addchr(&dest, SPECIAL_VAR_প্রতীক);
				চালু;
			}
		}
		যদি (ch == EOF) {
			syntax_error_unterm_ch('"');
			যাও ret; /* error */
		}
		যদি (ch == '"') {
			dest.o_expflags ^= EXP_FLAG_ESC_GLOB_CHARS;
			যদি (dest.o_expflags) {
				o_addchr(&dest, SPECIAL_VAR_প্রতীক);
				o_addchr(&dest, SPECIAL_VAR_প্রতীক);
			}
			চালু;
		}
		যদি (ch == '\\') {
			ch = i_getch(&প্রবেশ);
			যদি (ch == EOF) {
//example? error message?	syntax_error_unterm_ch('"');
				debug_printf_parse("%s: error: \\<eof>\n", __func__);
				যাও ret;
			}
			o_addqchr(&dest, ch);
			চালু;
		}
		যদি (ch == '$') {
			যদি (!parse_dollar(ফাঁকা, &dest, &প্রবেশ, /*quote_mask:*/ (dest.o_expflags || dquoted) ? 0x80 : 0)) {
				debug_printf_parse("%s: error: parse_dollar returned 0 (error)\n", __func__);
				যাও ret;
			}
			চালু;
		}
#যদি ENABLE_HUSH_TICK
		যদি (ch == '`') {
			//unsigned pos = dest->length;
			o_addchr(&dest, SPECIAL_VAR_প্রতীক);
			o_addchr(&dest, (dest.o_expflags || dquoted) ? 0x80 | '`' : '`');
			যদি (!add_till_backquote(&dest, &প্রবেশ,
					/*in_dquote:*/ dest.o_expflags /* nonzero if EXP_FLAG_ESC_GLOB_CHARS set */
				)
			) {
				যাও ret; /* error */
			}
			o_addchr(&dest, SPECIAL_VAR_প্রতীক);
			//debug_printf_subst("SUBST RES3 '%s'\n", dest->data + pos);
			চালু;
		}
#endif
		যদি (dquoted) {
			/* Always glob-protect if in dquotes:
			 * x=x; echo "${x:+/bin/c*}" - prints: /bin/c*
			 * x=x; echo "${x:+"/bin/c*"}" - prints: /bin/c*
			 */
			o_addqchr(&dest, ch);
		} অন্যথা {
			/* Glob-protect only if char is quoted:
			 * x=x; echo ${x:+/bin/c*} - prints many filenames
			 * x=x; echo ${x:+"/bin/c*"} - prints: /bin/c*
			 */
			o_addQchr(&dest, ch);
		}
	} /* for (;;) */

	যদি (dest.তথ্য) {
		n = expand_vars_to_list(output, n, dest.তথ্য);
	}
 ret:
	o_free(&dest);
	ফেরত n;
}
#endif /* !__U_BOOT__ */

#ifndef __U_BOOT__
#যদি ENABLE_FEATURE_SH_MATH
স্থির arith_t expand_and_evaluate_arith(ধ্রুবক অক্ষর *arg, ধ্রুবক অক্ষর **errmsg_p)
{
	arith_state_t math_state;
	arith_t res;
	অক্ষর *exp_str;

	math_state.lookupvar = get_local_var_মান;
	math_state.setvar = set_local_var_from_halves;
	//math_state.endofname = endofname;
	exp_str = encode_then_expand_string(arg);
	res = arith(&math_state, exp_str ? exp_str : arg);
	মুক্ত(exp_str);
	যদি (errmsg_p)
		*errmsg_p = math_state.errmsg;
	যদি (math_state.errmsg)
		বার্তা_and_die_if_script(math_state.errmsg);
	ফেরত res;
}
#endif
#endif /* !__U_BOOT__ */

#ifndef __U_BOOT__
#যদি BASH_PATTERN_SUBST
/* ${var/[/]pattern[/repl]} helpers */
স্থির অক্ষর *strstr_pattern(অক্ষর *val, ধ্রুবক অক্ষর *pattern, পূর্ণসংখ্যা *আকার_2)
{
	পূর্ণসংখ্যা first_escaped = (pattern[0] == '\\' && pattern[1]);
	/* "first_escaped" trick allows to treat e.g. "\*no_glob_chars"
	 * as literal too (as it is semi-common, and easy to accomodate
	 * by just using str + 1).
	 */
	পূর্ণসংখ্যা sz = strcspn(pattern + first_escaped * 2, "*?[\\");
	যদি ((pattern + first_escaped * 2)[sz] == '\0') {
		/* Optimization for trivial patterns.
		 * Testcase for very slow replace (performs about 22k replaces):
		 * x=::::::::::::::::::::::
		 * x=$x$x;x=$x$x;x=$x$x;x=$x$x;x=$x$x;x=$x$x;x=$x$x;x=$x$x;x=$x$x;x=$x$x;echo ${#x}
		 * echo "${x//:/|}"
		 */
		*আকার_2 = sz + first_escaped;
		ফেরত strstr(val, pattern + first_escaped);
	}

	যখন (1) {
		অক্ষর *end = scan_and_match(val, pattern, SCAN_MOVE_FROM_ডান + SCAN_MATCH_বাম_HALF);
		debug_printf_varexp("val:'%s' pattern:'%s' end:'%s'\n", val, pattern, end);
		যদি (end) {
			*আকার_2 = end - val;
			ফেরত val;
		}
		যদি (*val == '\0')
			ফেরত ফাঁকা;
		/* Optimization: if "*pat" did not match the start of "string",
		 * we know that "tring", "ring" etc will not match too:
		 */
		যদি (pattern[0] == '*')
			ফেরত ফাঁকা;
		val++;
	}
}
স্থির অক্ষর *replace_pattern(অক্ষর *val, ধ্রুবক অক্ষর *pattern, ধ্রুবক অক্ষর *repl, অক্ষর exp_op)
{
	অক্ষর *ফলাফল = ফাঁকা;
	unsigned res_len = 0;
	unsigned repl_len = strlen(repl);

	/* Null pattern never matches, including if "var" is empty */
	যদি (!pattern[0])
		ফেরত ফলাফল; /* NULL, no replaces happened */

	যখন (1) {
		পূর্ণসংখ্যা আকার_2;
		অক্ষর *s = strstr_pattern(val, pattern, &আকার_2);
		যদি (!s)
			বিরতি;

		ফলাফল = xrealloc(ফলাফল, res_len + (s - val) + repl_len + 1);
		strcpy(mempcpy(ফলাফল + res_len, val, s - val), repl);
		res_len += (s - val) + repl_len;
		debug_printf_varexp("val:'%s' s:'%s' result:'%s'\n", val, s, ফলাফল);

		val = s + আকার_2;
		যদি (exp_op == '/')
			বিরতি;
	}
	যদি (*val && ফলাফল) {
		ফলাফল = xrealloc(ফলাফল, res_len + strlen(val) + 1);
		strcpy(ফলাফল + res_len, val);
		debug_printf_varexp("val:'%s' result:'%s'\n", val, ফলাফল);
	}
	debug_printf_varexp("result:'%s'\n", ফলাফল);
	ফেরত ফলাফল;
}
#endif /* BASH_PATTERN_SUBST */
#endif /* !__U_BOOT__ */

স্থির পূর্ণসংখ্যা append_str_maybe_ifs_split(o_string *output, পূর্ণসংখ্যা n,
		পূর্ণসংখ্যা first_ch, ধ্রুবক অক্ষর *val)
{
	যদি (!(first_ch & 0x80)) { /* unquoted $VAR */
		debug_printf_expand("unquoted '%s', output->o_escape:%d\n", val,
				!!(output->o_expflags & EXP_FLAG_ESC_GLOB_CHARS));
		যদি (val && val[0])
			n = expand_on_ifs(output, n, val);
	} অন্যথা { /* quoted "$VAR" */
		output->has_quoted_part = 1;
		debug_printf_expand("quoted '%s', output->o_escape:%d\n", val,
				!!(output->o_expflags & EXP_FLAG_ESC_GLOB_CHARS));
		যদি (val && val[0])
			o_addQstr(output, val);
	}
	ফেরত n;
}

/* Handle <SPECIAL_VAR_SYMBOL>varname...<SPECIAL_VAR_SYMBOL> construct.
 */
স্থির NOINLINE পূর্ণসংখ্যা expand_one_var(o_string *output, পূর্ণসংখ্যা n,
		পূর্ণসংখ্যা first_ch, অক্ষর *arg, অক্ষর **pp)
{
	ধ্রুবক অক্ষর *val;
	অক্ষর *to_be_freed;
	অক্ষর *p;
	অক্ষর *var;
	অক্ষর exp_op;
	অক্ষর exp_save = exp_save; /* for compiler */
	অক্ষর *exp_saveptr; /* points to expansion operator */
	অক্ষর *exp_word = exp_word; /* for compiler */
	অক্ষর arg0;

	val = ফাঁকা;
	to_be_freed = ফাঁকা;
	p = *pp;
	*p = '\0'; /* replace trailing SPECIAL_VAR_SYMBOL */
	var = arg;
	exp_saveptr = arg[1] ? strchr(VAR_ENCODED_SUBST_OPS, arg[1]) : ফাঁকা;
	arg0 = arg[0];
	arg[0] = (arg0 & 0x7f);
	exp_op = 0;

	যদি (arg[0] == '#' && arg[1] /* ${#...} but not ${#} */
	 && (!exp_saveptr               /* and ( not(${#<op_char>...}) */
	    || (arg[2] == '\0' && strchr(SPECIAL_VARS_STR, arg[1])) /* or ${#C} "len of $C" ) */
	    )		/* NB: skipping ^^^specvar check mishandles ${#::2} */
	) {
		/* It must be length operator: ${#var} */
		var++;
		exp_op = 'L';
	} অন্যথা {
		/* Maybe handle parameter expansion */
		যদি (exp_saveptr /* if 2nd char is one of expansion operators */
		 && strchr(NUMERIC_SPECVARS_STR, arg[0]) /* 1st char is special variable */
		) {
			/* ${?:0}, ${#[:]%0} etc */
			exp_saveptr = var + 1;
		} অন্যথা {
			/* ${?}, ${var}, ${var:0}, ${var[:]%0} etc */
			exp_saveptr = var+1 + strcspn(var+1, VAR_ENCODED_SUBST_OPS);
		}
		exp_op = exp_save = *exp_saveptr;
#ifndef __U_BOOT__
		যদি (exp_op) {
			exp_word = exp_saveptr + 1;
			যদি (exp_op == ':') {
				exp_op = *exp_word++;
//TODO: try ${var:} and ${var:bogus} in non-bash config
				যদি (BASH_SUBSTR
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
	যদি (isdigit(var[0])) {
		/* parse_dollar should have vetted var for us */
#ifndef __U_BOOT__
		পূর্ণসংখ্যা nn = xatoi_positive(var);
#অন্যথা /* __U_BOOT__ */
		পূর্ণসংখ্যা nn = simple_strtoul(var, ফাঁকা, 10);
#endif /* __U_BOOT__ */
		যদি (nn < G.global_argc)
			val = G.global_argv[nn];
		/* else val remains NULL: $N with too big N */
	} অন্যথা {
		নির্বাচন (var[0]) {
#ifndef __U_BOOT__
		ক্ষেত্র '$': /* pid */
			val = utoa(G.মূল_pid);
			বিরতি;
		ক্ষেত্র '!': /* bg pid */
			val = G.last_bg_pid ? utoa(G.last_bg_pid) : "";
			বিরতি;
#endif /* !__U_BOOT__ */
		ক্ষেত্র '?': /* exitcode */
			val = utoa(G.last_exitcode);
			বিরতি;
		ক্ষেত্র '#': /* argc */
			val = utoa(G.global_argc ? G.global_argc-1 : 0);
			বিরতি;
#ifndef __U_BOOT__
		ক্ষেত্র '-': { /* active options */
			/* Check set_mode() to see what option chars we support */
			অক্ষর *cp;
			val = cp = G.optstring_buf;
			যদি (G.o_opt[OPT_O_ERREXIT])
				*cp++ = 'e';
			যদি (G_interactive_fd)
				*cp++ = 'i';
			যদি (G_x_mode)
				*cp++ = 'x';
			/* If G.o_opt[OPT_O_NOEXEC] is true,
			 * commands read but are not executed,
			 * so $- can not execute too, 'n' is never seen in $-.
			 */
			যদি (G.opt_c)
				*cp++ = 'c';
			যদি (G.opt_s)
				*cp++ = 's';
			*cp = '\0';
			বিরতি;
		}
#endif /* !__U_BOOT__ */
		পূর্বনির্ধারিত:
#ifndef __U_BOOT__
			val = get_local_var_মান(var);
#অন্যথা /* __U_BOOT__ */
			/*
			 * Environment variable set with setenv* have to be
			 * expanded.
			 * So, we first search if the variable exists in
			 * environment, if this is not the case, we default to
			 * local value.
			 */
			val = env_get(var);
			যদি (!val)
				val = get_local_var_মান(var);
#endif /* __U_BOOT__ */
		}
	}

#ifndef __U_BOOT__
	/* Handle any expansions */
	যদি (exp_op == 'L') {
		reinit_unicode_for_hush();
		debug_printf_expand("expand: length(%s)=", val);
		val = utoa(val ? unicode_strlen(val) : 0);
		debug_printf_expand("%s\n", val);
	} অন্যথা যদি (exp_op) {
		যদি (exp_op == '%' || exp_op == '#') {
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
			যদি (val && val[0]) {
				অক্ষর *t;
				অক্ষর *exp_exp_word;
				অক্ষর *loc;
				unsigned scan_flags = pick_scan(exp_op, *exp_word);
				যদি (exp_op == *exp_word)  /* ## or %% */
					exp_word++;
				debug_printf_expand("expand: exp_word:'%s'\n", exp_word);
				exp_exp_word = encode_then_expand_vararg(exp_word, /*handle_squotes:*/ 1, /*unbackslash:*/ 0);
				যদি (exp_exp_word)
					exp_word = exp_exp_word;
				debug_printf_expand("expand: exp_word:'%s'\n", exp_word);
				/*
				 * HACK ALERT. We depend here on the fact that
				 * G.global_argv and results of utoa and get_local_var_value
				 * are actually in writable memory:
				 * scan_and_match momentarily stores NULs there.
				 */
				t = (অক্ষর*)val;
				loc = scan_and_match(t, exp_word, scan_flags);
				debug_printf_expand("op:%c str:'%s' pat:'%s' res:'%s'\n", exp_op, t, exp_word, loc);
				মুক্ত(exp_exp_word);
				যদি (loc) { /* match was found */
					যদি (scan_flags & SCAN_MATCH_বাম_HALF) /* #[#] */
						val = loc; /* take right part */
					অন্যথা /* %[%] */
						val = to_be_freed = xstrndup(val, loc - val); /*বাম*/
				}
			}
		}
#যদি BASH_PATTERN_SUBST
		অন্যথা যদি (exp_op == '/' || exp_op == '\\') {
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
			যদি (val /*&& val[0]*/) {
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
				অক্ষর *pattern, *repl, *t;
				pattern = encode_then_expand_vararg(exp_word, /*handle_squotes:*/ 1, /*unbackslash:*/ 0);
				যদি (!pattern)
					pattern = xstrdup(exp_word);
				debug_printf_varexp("pattern:'%s'->'%s'\n", exp_word, pattern);
				*p++ = SPECIAL_VAR_প্রতীক;
				exp_word = p;
				p = strchr(p, SPECIAL_VAR_প্রতীক);
				*p = '\0';
				repl = encode_then_expand_vararg(exp_word, /*handle_squotes:*/ 1, /*unbackslash:*/ 1);
				debug_printf_varexp("repl:'%s'->'%s'\n", exp_word, repl);
				/* HACK ALERT. We depend here on the fact that
				 * G.global_argv and results of utoa and get_local_var_value
				 * are actually in writable memory:
				 * replace_pattern momentarily stores NULs there. */
				t = (অক্ষর*)val;
				to_be_freed = replace_pattern(t,
						pattern,
						(repl ? repl : exp_word),
						exp_op);
				যদি (to_be_freed) /* at least one replace happened */
					val = to_be_freed;
				মুক্ত(pattern);
				মুক্ত(repl);
			} অন্যথা {
				/* Unset variable always gives nothing */
				//  a=; echo ${a/*/w}      # "w"
				//  unset a; echo ${a/*/w} # ""
				/* Just skip "replace" part */
				*p++ = SPECIAL_VAR_প্রতীক;
				p = strchr(p, SPECIAL_VAR_প্রতীক);
				*p = '\0';
			}
		}
#endif /* BASH_PATTERN_SUBST */
		অন্যথা যদি (exp_op == ':') {
#যদি BASH_SUBSTR && ENABLE_FEATURE_SH_MATH
			/* It's ${var:N[:M]} bashism.
			 * Note that in encoded form it has TWO parts:
			 * var:N<SPECIAL_VAR_SYMBOL>M<SPECIAL_VAR_SYMBOL>
			 */
			arith_t beg, len;
			unsigned vallen;
			ধ্রুবক অক্ষর *errmsg;

			beg = expand_and_evaluate_arith(exp_word, &errmsg);
			যদি (errmsg)
				যাও empty_ফলাফল;
			debug_printf_varexp("beg:'%s'=%lld\n", exp_word, (long long)beg);
			*p++ = SPECIAL_VAR_প্রতীক;
			exp_word = p;
			p = strchr(p, SPECIAL_VAR_প্রতীক);
			*p = '\0';
			vallen = val ? strlen(val) : 0;
			যদি (beg < 0) {
				/* negative beg counts from the end */
				beg = (arith_t)vallen + beg;
			}
			/* If expansion will be empty, do not even evaluate len */
			যদি (!val || beg < 0 || beg > vallen) {
				/* Why > vallen, not >=? bash:
				 * unset b; a=ab; : ${a:2:${b=3}}; echo $b  # "", b=3 (!!!)
				 * unset b; a=a; : ${a:2:${b=3}}; echo $b   # "", b not set
				 */
				যাও empty_ফলাফল;
			}
			len = expand_and_evaluate_arith(exp_word, &errmsg);
			যদি (errmsg)
				যাও empty_ফলাফল;
			debug_printf_varexp("len:'%s'=%lld\n", exp_word, (long long)len);
			debug_printf_varexp("from val:'%s'\n", val);
			যদি (len < 0) {
				/* in bash, len=-n means strlen()-n */
				len = (arith_t)vallen - beg + len;
				যদি (len < 0) /* bash compat */
					বার্তা_and_die_if_script("%s: substring expression < 0", var);
			}
			যদি (len <= 0 || !val /*|| beg >= vallen*/) {
 empty_ফলাফল:
				val = ফাঁকা;
			} অন্যথা {
				/* Paranoia. What if user entered 9999999999999
				 * which fits in arith_t but not int? */
				যদি (len > INT_MAX)
					len = INT_MAX;
				val = to_be_freed = xstrndup(val + beg, len);
			}
			debug_printf_varexp("val:'%s'\n", val);
#অন্যথা /* not (HUSH_SUBSTR_EXPANSION && FEATURE_SH_MATH) */
			বার্তা_and_die_if_script("malformed ${%s:...}", var);
			val = ফাঁকা;
#endif
		} অন্যথা { /* one of "-=+?" */
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
			পূর্ণসংখ্যা use_word = (!val || ((exp_save == ':') && !val[0]));
			যদি (exp_op == '+')
				use_word = !use_word;
			debug_printf_expand("expand: op:%c (null:%s) test:%i\n", exp_op,
					(exp_save == ':') ? "true" : "false", use_word);
			যদি (use_word) {
				যদি (exp_op == '+' || exp_op == '-') {
					/* ${var+word} - use alternative value */
					/* ${var-word} - use default value */
					n = encode_then_append_var_plusminus(output, n, exp_word,
							/*dquoted:*/ (arg0 & 0x80)
					);
					val = ফাঁকা;
				} অন্যথা {
					/* ${var?word} - indicate error if unset */
					/* ${var=word} - assign and use default value */
					to_be_freed = encode_then_expand_vararg(exp_word,
							/*handle_squotes:*/ !(arg0 & 0x80),
							/*unbackslash:*/ 0
					);
					যদি (to_be_freed)
						exp_word = to_be_freed;
					যদি (exp_op == '?') {
						/* mimic bash message */
						বার্তা_and_die_if_script("%s: %s",
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
					} অন্যথা {
						val = exp_word;
					}
					যদি (exp_op == '=') {
						/* ${var=[word]} or ${var:=[word]} */
						যদি (isdigit(var[0]) || var[0] == '#') {
							/* mimic bash message */
							বার্তা_and_die_if_script("$%s: cannot assign in this way", var);
							val = ফাঁকা;
						} অন্যথা {
							অক্ষর *new_var = xasprintf("%s=%s", var, val);
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

	মুক্ত(to_be_freed);
	ফেরত n;
}

/* Expand all variable references in given string, adding words to list[]
 * at n, n+1,... positions. Return updated n (so that list[n] is next one
 * to be filled). This routine is extremely tricky: has to deal with
 * variables/parameters with whitespace, $* and $@, and constructs like
 * 'echo -$*-'. If you play here, you must run testsuite afterwards! */
স্থির NOINLINE পূর্ণসংখ্যা expand_vars_to_list(o_string *output, পূর্ণসংখ্যা n, অক্ষর *arg)
{
	/* output->o_expflags & EXP_FLAG_SINGLEWORD (0x80) if we are in
	 * expansion of right-hand side of assignment == 1-element expand.
	 */
	অক্ষর cant_be_null = 0; /* only bit 0x80 matters */
	অক্ষর *p;

	debug_printf_expand("expand_vars_to_list: arg:'%s' singleword:%x\n", arg,
			!!(output->o_expflags & EXP_FLAG_SINGLEWORD));
	debug_print_list("expand_vars_to_list[0]", output, n);

	যখন ((p = strchr(arg, SPECIAL_VAR_প্রতীক)) != ফাঁকা) {
		অক্ষর first_ch;
#যদি ENABLE_FEATURE_SH_MATH
		অক্ষর arith_buf[আকার(arith_t)*3 + 2];
#endif

		যদি (output->ended_in_ifs) {
			o_addchr(output, '\0');
			n = o_save_ptr(output, n);
			output->ended_in_ifs = 0;
		}

		o_addblock(output, arg, p - arg);
		debug_print_list("expand_vars_to_list[1]", output, n);
		arg = ++p;
		p = strchr(p, SPECIAL_VAR_প্রতীক);

		/* Fetch special var name (if it is indeed one of them)
		 * and quote bit, force the bit on if singleword expansion -
		 * important for not getting v=$@ expand to many words. */
		first_ch = arg[0] | (output->o_expflags & EXP_FLAG_SINGLEWORD);

		/* Is this variable quoted and thus expansion can't be null?
		 * "$@" is special. Even if quoted, it can still
		 * expand to nothing (not even an empty string),
		 * thus it is excluded. */
		যদি ((first_ch & 0x7f) != '@')
			cant_be_null |= first_ch;

		নির্বাচন (first_ch & 0x7f) {
		/* Highest bit in first_ch indicates that var is double-quoted */
		ক্ষেত্র '*':
		ক্ষেত্র '@': {
			পূর্ণসংখ্যা i;
#ifndef __U_BOOT__
			যদি (!G.global_argv[1])
#অন্যথা /* __U_BOOT__ */
			যদি (!G.global_argv || !G.global_argv[1])
#endif /* __U_BOOT__ */
				বিরতি;
			i = 1;
			cant_be_null |= first_ch; /* do it for "$@" _now_, when we know it's not empty */
			যদি (!(first_ch & 0x80)) { /* unquoted $* or $@ */
				যখন (G.global_argv[i]) {
					n = expand_on_ifs(output, n, G.global_argv[i]);
					debug_printf_expand("expand_vars_to_list: argv %d (last %d)\n", i, G.global_argc - 1);
					যদি (G.global_argv[i++][0] && G.global_argv[i]) {
						/* this argv[] is not empty and not last:
						 * put terminating NUL, start new word */
						o_addchr(output, '\0');
						debug_print_list("expand_vars_to_list[2]", output, n);
						n = o_save_ptr(output, n);
						debug_print_list("expand_vars_to_list[3]", output, n);
					}
				}
			} অন্যথা
			/* If EXP_FLAG_SINGLEWORD, we handle assignment 'a=....$@.....'
			 * and in this case should treat it like '$*' - see 'else...' below */
			যদি (first_ch == (অক্ষর)('@'|0x80)  /* quoted $@ */
			 && !(output->o_expflags & EXP_FLAG_SINGLEWORD) /* not v="$@" case */
			) {
				যখন (1) {
					o_addQstr(output, G.global_argv[i]);
					যদি (++i >= G.global_argc)
						বিরতি;
					o_addchr(output, '\0');
					debug_print_list("expand_vars_to_list[4]", output, n);
					n = o_save_ptr(output, n);
				}
			} অন্যথা { /* quoted $* (or v="$@" case): add as one word */
				যখন (1) {
					o_addQstr(output, G.global_argv[i]);
					যদি (!G.global_argv[++i])
						বিরতি;
					যদি (G.ifs[0])
						o_addchr(output, G.ifs[0]);
				}
				output->has_quoted_part = 1;
			}
			বিরতি;
		}
		ক্ষেত্র SPECIAL_VAR_প্রতীক: {
			/* <SPECIAL_VAR_SYMBOL><SPECIAL_VAR_SYMBOL> */
			/* "Empty variable", used to make "" etc to not disappear */
			output->has_quoted_part = 1;
			cant_be_null = 0x80;
			arg++;
			বিরতি;
		}
		ক্ষেত্র SPECIAL_VAR_QUOTED_SVS:
			/* <SPECIAL_VAR_SYMBOL><SPECIAL_VAR_QUOTED_SVS><SPECIAL_VAR_SYMBOL> */
			/* "^C variable", represents literal ^C char (possible in scripts) */
			o_addchr(output, SPECIAL_VAR_প্রতীক);
			arg++;
			বিরতি;
#যদি ENABLE_HUSH_TICK
		ক্ষেত্র '`': {
			/* <SPECIAL_VAR_SYMBOL>`cmd<SPECIAL_VAR_SYMBOL> */
			o_string subst_ফলাফল = NULL_O_STRING;

			*p = '\0'; /* replace trailing <SPECIAL_VAR_SYMBOL> */
			arg++;
			/* Can't just stuff it into output o_string,
			 * expanded result may need to be globbed
			 * and $IFS-split */
			debug_printf_subst("SUBST '%s' first_ch %x\n", arg, first_ch);
			G.last_exitcode = process_command_subs(&subst_ফলাফল, arg);
			G.expand_exitcode = G.last_exitcode;
			debug_printf_subst("SUBST RES:%d '%s'\n", G.last_exitcode, subst_ফলাফল.তথ্য);
			n = append_str_maybe_ifs_split(output, n, first_ch, subst_ফলাফল.তথ্য);
			o_free(&subst_ফলাফল);
			বিরতি;
		}
#endif
#যদি ENABLE_FEATURE_SH_MATH
		ক্ষেত্র '+': {
			/* <SPECIAL_VAR_SYMBOL>+arith<SPECIAL_VAR_SYMBOL> */
			arith_t res;

			arg++; /* skip '+' */
			*p = '\0'; /* replace trailing <SPECIAL_VAR_SYMBOL> */
			debug_printf_subst("ARITH '%s' first_ch %x\n", arg, first_ch);
			res = expand_and_evaluate_arith(arg, ফাঁকা);
			debug_printf_subst("ARITH RES '"ARITH_FMT"'\n", res);
			sprintf(arith_buf, ARITH_FMT, res);
			যদি (res < 0
			 && first_ch == (অক্ষর)('+'|0x80)
			/* && (output->o_expflags & EXP_FLAG_ESC_GLOB_CHARS) */
			) {
				/* Quoted negative ariths, like filename[0"$((-9))"],
				 * should not be interpreted as glob ranges.
				 * Convert leading '-' to '\-':
				 */
				o_grow_by(output, 1);
				output->তথ্য[output->দৈর্ঘ্য++] = '\\';
			}
			o_addstr(output, arith_buf);
			বিরতি;
		}
#endif
		পূর্বনির্ধারিত:
			/* <SPECIAL_VAR_SYMBOL>varname[ops]<SPECIAL_VAR_SYMBOL> */
			n = expand_one_var(output, n, first_ch, arg, &p);
			বিরতি;
		} /* switch (char after <SPECIAL_VAR_SYMBOL>) */

		/* Restore NULL'ed SPECIAL_VAR_SYMBOL.
		 * Do the check to avoid writing to a const string. */
		যদি (*p != SPECIAL_VAR_প্রতীক)
			*p = SPECIAL_VAR_প্রতীক;
		arg = ++p;
	} /* end of "while (SPECIAL_VAR_SYMBOL is found) ..." */

	যদি (*arg) {
		/* handle trailing string */
		যদি (output->ended_in_ifs) {
			o_addchr(output, '\0');
			n = o_save_ptr(output, n);
		}
		debug_print_list("expand_vars_to_list[a]", output, n);
		/* this part is literal, and it was already pre-quoted
		 * if needed (much earlier), do not use o_addQstr here!
		 */
		o_addstr(output, arg);
		debug_print_list("expand_vars_to_list[b]", output, n);
	} অন্যথা
	যদি (output->দৈর্ঘ্য == o_get_last_ptr(output, n) /* expansion is empty */
	 && !(cant_be_null & 0x80)   /* and all vars were not quoted */
	 && !output->has_quoted_part
	) {
		n--;
		/* allow to reuse list[n] later without re-growth */
		output->has_empty_slot = 1;
	}

	ফেরত n;
}

স্থির অক্ষর **expand_variables(অক্ষর **argv, unsigned expflags)
{
	পূর্ণসংখ্যা n;
	অক্ষর **list;
	o_string output = NULL_O_STRING;

	output.o_expflags = expflags;

	n = 0;
	জন্য (;;) {
		/* go to next list[n] */
		output.ended_in_ifs = 0;
		n = o_save_ptr(&output, n);

		যদি (!*argv)
			বিরতি;

		/* expand argv[i] */
		n = expand_vars_to_list(&output, n, *argv++);
		/* if (!output->has_empty_slot) -- need this?? */
			o_addchr(&output, '\0');
	}
	debug_print_list("expand_variables", &output, n);

	/* output.data (malloced in one block) gets returned in "list" */
	list = o_finalize_list(&output, n);
	debug_print_strings("expand_variables[1]", list);
	ফেরত list;
}

স্থির অক্ষর **expand_strvec_to_strvec(অক্ষর **argv)
{
	ফেরত expand_variables(argv, EXP_FLAG_GLOB | EXP_FLAG_ESC_GLOB_CHARS);
}

#যদি defined(CMD_SINGLEWORD_NOGLOB) || defined(CMD_TEST2_SINGLEWORD_NOGLOB)
স্থির অক্ষর **expand_strvec_to_strvec_singleword_noglob(অক্ষর **argv)
{
	ফেরত expand_variables(argv, EXP_FLAG_SINGLEWORD);
}
#endif

/* Used for expansion of right hand of assignments,
 * $((...)), heredocs, variable expansion parts.
 *
 * NB: should NOT do globbing!
 * "export v=/bin/c*; env | grep ^v=" outputs "v=/bin/c*"
 */
স্থির অক্ষর *expand_string_to_string(ধ্রুবক অক্ষর *str, পূর্ণসংখ্যা EXP_flags, পূর্ণসংখ্যা do_unbackslash)
{
#যদি !BASH_PATTERN_SUBST && !ENABLE_HUSH_CASE
	ধ্রুবক পূর্ণসংখ্যা do_unbackslash = 1;
	ধ্রুবক পূর্ণসংখ্যা EXP_flags = EXP_FLAG_ESC_GLOB_CHARS;
#endif
	অক্ষর *argv[2], **list;

	debug_printf_expand("string_to_string<='%s'\n", str);
	/* This is generally an optimization, but it also
	 * handles "", which otherwise trips over !list[0] check below.
	 * (is this ever happens that we actually get str="" here?)
	 */
	যদি (!strchr(str, SPECIAL_VAR_প্রতীক) && !strchr(str, '\\')) {
		//TODO: Can use on strings with \ too, just unbackslash() them?
		debug_printf_expand("string_to_string(fast)=>'%s'\n", str);
		ফেরত xstrdup(str);
	}

	argv[0] = (অক্ষর*)str;
	argv[1] = ফাঁকা;
	list = expand_variables(argv, EXP_flags | EXP_FLAG_SINGLEWORD);
	যদি (!list[0]) {
		/* Example where it happens:
		 * x=; echo ${x:-"$@"}
		 */
		((অক্ষর*)list)[0] = '\0';
	} অন্যথা {
		যদি (HUSH_DEBUG)
			যদি (list[1])
				bb_simple_error_বার্তা_and_die("BUG in varexp2");
		/* actually, just move string 2*sizeof(char*) bytes back */
		overlapping_strcpy((অক্ষর*)list, list[0]);
		যদি (do_unbackslash)
			unbackslash((অক্ষর*)list);
	}
	debug_printf_expand("string_to_string=>'%s'\n", (অক্ষর*)list);
	ফেরত (অক্ষর*)list;
}

#যদি 0
স্থির অক্ষর* expand_strvec_to_string(অক্ষর **argv)
{
	অক্ষর **list;

	list = expand_variables(argv, EXP_FLAG_SINGLEWORD);
	/* Convert all NULs to spaces */
	যদি (list[0]) {
		পূর্ণসংখ্যা n = 1;
		যখন (list[n]) {
			যদি (HUSH_DEBUG)
				যদি (list[n-1] + strlen(list[n-1]) + 1 != list[n])
					bb_error_বার্তা_and_die("BUG in varexp3");
			/* bash uses ' ' regardless of $IFS contents */
			list[n][-1] = ' ';
			n++;
		}
	}
	overlapping_strcpy((অক্ষর*)list, list[0] ? list[0] : "");
	debug_printf_expand("strvec_to_string='%s'\n", (অক্ষর*)list);
	ফেরত (অক্ষর*)list;
}
#endif

#ifndef __U_BOOT__
স্থির অক্ষর **expand_assignments(অক্ষর **argv, পূর্ণসংখ্যা গুনতি)
{
	পূর্ণসংখ্যা i;
	অক্ষর **p;

	G.expanded_assignments = p = ফাঁকা;
	/* Expand assignments into one string each */
	জন্য (i = 0; i < গুনতি; i++) {
		p = add_string_to_strings(p,
			expand_string_to_string(argv[i],
				EXP_FLAG_ESC_GLOB_CHARS,
				/*unbackslash:*/ 1
			)
		);
		G.expanded_assignments = p;
	}
	G.expanded_assignments = ফাঁকা;
	ফেরত p;
}

স্থির শূন্য switch_off_special_sigs(unsigned mask)
{
	unsigned sig = 0;
	যখন ((mask >>= 1) != 0) {
		sig++;
		যদি (!(mask & 1))
			চালু;
#যদি ENABLE_HUSH_TRAP
		যদি (G_traps) {
			যদি (G_traps[sig] && !G_traps[sig][0])
				/* trap is '', has to remain SIG_IGN */
				চালু;
			মুক্ত(G_traps[sig]);
			G_traps[sig] = ফাঁকা;
		}
#endif
		/* We are here only if no trap or trap was not '' */
		install_sighandler(sig, SIG_DFL);
	}
}
#endif /* !__U_BOOT__ */

#ifndef __U_BOOT__
#যদি BB_MMU
/* never called */
শূন্য re_execute_shell(অক্ষর ***to_free, ধ্রুবক অক্ষর *s,
		অক্ষর *g_argv0, অক্ষর **g_argv,
		অক্ষর **builtin_argv) NORETURN;

স্থির শূন্য reset_traps_to_defaults(শূন্য)
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
	যদি (!G_traps && !mask)
		ফেরত; /* already no traps and no special sigs */

	/* Switch off special sigs */
	switch_off_special_sigs(mask);
# যদি ENABLE_HUSH_JOB
	G_fatal_sig_mask = 0;
# endif
	G.special_sig_mask &= ~SPECIAL_INTERACTIVE_SIGS;
	/* SIGQUIT,SIGCHLD and maybe SPECIAL_JOBSTOP_SIGS
	 * remain set in G.special_sig_mask */

# যদি ENABLE_HUSH_TRAP
	যদি (!G_traps)
		ফেরত;

	/* Reset all sigs to default except ones with empty traps */
	জন্য (sig = 0; sig < NSIG; sig++) {
		যদি (!G_traps[sig])
			চালু; /* no trap: nothing to do */
		যদি (!G_traps[sig][0])
			চালু; /* empty trap: has to remain SIG_IGN */
		/* sig has non-empty trap, reset it: */
		মুক্ত(G_traps[sig]);
		G_traps[sig] = ফাঁকা;
		/* There is no signal for trap 0 (EXIT) */
		যদি (sig == 0)
			চালু;
		install_sighandler(sig, pick_sighandler(sig));
	}
# endif
}

#অন্যথা /* !BB_MMU */

স্থির শূন্য re_execute_shell(অক্ষর ***to_free, ধ্রুবক অক্ষর *s,
		অক্ষর *g_argv0, অক্ষর **g_argv,
		অক্ষর **builtin_argv) NORETURN;
স্থির শূন্য re_execute_shell(অক্ষর ***to_free, ধ্রুবক অক্ষর *s,
		অক্ষর *g_argv0, অক্ষর **g_argv,
		অক্ষর **builtin_argv)
{
# define NOMMU_HACK_FMT ("-$%x:%x:%x:%x:%x:%llx" IF_HUSH_LOOPS(":%x"))
	/* delims + 2 * (number of bytes in printed hex numbers) */
	অক্ষর param_buf[আকার(NOMMU_HACK_FMT) + 2 * (আকার(পূর্ণসংখ্যা)*6 + আকার(long long)*1)];
	অক্ষর *heredoc_argv[4];
	কাঠামো variable *cur;
# যদি ENABLE_HUSH_FUNCTIONS
	কাঠামো function *funcp;
# endif
	অক্ষর **argv, **pp;
	unsigned cnt;
	unsigned long long empty_trap_mask;

	যদি (!g_argv0) { /* heredoc */
		argv = heredoc_argv;
		argv[0] = (অক্ষর *) G.argv0_for_re_execing;
		argv[1] = (অক্ষর *) "-<";
		argv[2] = (অক্ষর *) s;
		argv[3] = ফাঁকা;
		pp = &argv[3]; /* used as pointer to empty environment */
		যাও do_exec;
	}

	cnt = 0;
	pp = builtin_argv;
	যদি (pp) যখন (*pp++)
		cnt++;

	empty_trap_mask = 0;
	যদি (G_traps) {
		পূর্ণসংখ্যা sig;
		জন্য (sig = 1; sig < NSIG; sig++) {
			যদি (G_traps[sig] && !G_traps[sig][0])
				empty_trap_mask |= 1LL << sig;
		}
	}

	sprintf(param_buf, NOMMU_HACK_FMT
			, (unsigned) G.মূল_pid
			, (unsigned) G.মূল_ppid
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
	জন্য (cur = G.শীর্ষ_var; cur; cur = cur->পরবর্তী) {
		যদি (!cur->flg_export || cur->flg_read_only)
			cnt += 2;
	}
# যদি ENABLE_HUSH_LINENO_VAR
	cnt += 2;
# endif
# যদি ENABLE_HUSH_FUNCTIONS
	জন্য (funcp = G.শীর্ষ_func; funcp; funcp = funcp->পরবর্তী)
		cnt += 3;
# endif
	pp = g_argv;
	যখন (*pp++)
		cnt++;
	*to_free = argv = pp = xzalloc(আকার(argv[0]) * cnt);
	*pp++ = (অক্ষর *) G.argv0_for_re_execing;
	*pp++ = param_buf;
	জন্য (cur = G.শীর্ষ_var; cur; cur = cur->পরবর্তী) {
		যদি (strcmp(cur->varstr, hush_version_str) == 0)
			চালু;
		যদি (cur->flg_read_only) {
			*pp++ = (অক্ষর *) "-R";
			*pp++ = cur->varstr;
		} অন্যথা যদি (!cur->flg_export) {
			*pp++ = (অক্ষর *) "-V";
			*pp++ = cur->varstr;
		}
	}
# যদি ENABLE_HUSH_LINENO_VAR
	*pp++ = (অক্ষর *) "-L";
	*pp++ = utoa(G.execute_lineno);
# endif
# যদি ENABLE_HUSH_FUNCTIONS
	জন্য (funcp = G.শীর্ষ_func; funcp; funcp = funcp->পরবর্তী) {
		*pp++ = (অক্ষর *) "-F";
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
	*pp++ = (অক্ষর *) "-c";
	*pp++ = (অক্ষর *) s;
	যদি (builtin_argv) {
		যখন (*++builtin_argv)
			*pp++ = *builtin_argv;
		*pp++ = (অক্ষর *) "";
	}
	*pp++ = g_argv0;
	যখন (*g_argv)
		*pp++ = *g_argv++;
	/* *pp = NULL; - is already there */
	pp = environ;

 do_exec:
	debug_printf_exec("re_execute_shell pid:%d cmd:'%s'\n", getpid(), s);
	/* Don't propagate SIG_IGN to the child */
	যদি (SPECIAL_JOBSTOP_SIGS != 0)
		switch_off_special_sigs(G.special_sig_mask & SPECIAL_JOBSTOP_SIGS);
	execve(bb_busybox_exec_path, argv, pp);
	/* Fallback. Useful for init=/bin/hush usage etc */
	যদি (argv[0][0] == '/')
		execve(argv[0], argv, pp);
	xfunc_error_retval = 127;
	bb_simple_error_বার্তা_and_die("can't re-execute the shell");
}
#endif  /* !BB_MMU */

#endif /* !__U_BOOT__ */

স্থির পূর্ণসংখ্যা run_and_free_list(কাঠামো pipe *pi);

/* Executing from string: eval, sh -c '...'
 *          or from file: /etc/profile, . file, sh <script>, sh (intereactive)
 * end_trigger controls how often we stop parsing
 * NUL: parse all, execute, return
 * ';': parse till ';' or newline, execute, repeat till EOF
 */
#ifndef __U_BOOT__
স্থির শূন্য parse_and_run_stream(কাঠামো in_str *inp, পূর্ণসংখ্যা end_trigger)
#অন্যথা /* __U_BOOT__ */
স্থির পূর্ণসংখ্যা parse_and_run_stream(কাঠামো in_str *inp, পূর্ণসংখ্যা end_trigger)
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
	যখন (1) {
#অন্যথা /* __U_BOOT__ */
	করো {
#endif /* __U_BOOT__ */
		কাঠামো pipe *pipe_list;

#যদি ENABLE_HUSH_INTERACTIVE
		যদি (end_trigger == ';') {
			G.promptmode = 0; /* PS1 */
			debug_printf_prompt("%s promptmode=%d\n", __func__, G.promptmode);
		}
#endif
		pipe_list = parse_stream(ফাঁকা, ফাঁকা, inp, end_trigger);
		যদি (!pipe_list || pipe_list == ERR_PTR) { /* EOF/error */
			/* If we are in "big" script
			 * (not in `cmd` or something similar)...
			 */
			যদি (pipe_list == ERR_PTR && end_trigger == ';') {
				/* Discard cached input (rest of line) */
				পূর্ণসংখ্যা ch = inp->last_char;
				যখন (ch != EOF && ch != '\n') {
					//bb_error_msg("Discarded:'%c'", ch);
					ch = i_getch(inp);
				}
				/* Force prompt */
				inp->p = ফাঁকা;
				/* This stream isn't empty */
				empty = 0;
				চালু;
			}
			যদি (!pipe_list && empty)
				G.last_exitcode = 0;
			বিরতি;
		}
		debug_print_বৃক্ষ(pipe_list, 0);
		debug_printf_exec("parse_and_run_stream: run_and_free_list\n");
#ifndef __U_BOOT__
		run_and_free_list(pipe_list);
#অন্যথা /* __U_BOOT__ */
		পূর্ণসংখ্যা rcode = run_and_free_list(pipe_list);
		/*
		 * We reset input string to not run the following command, so running
		 * 'exit; echo foo' does not print foo.
		 */
		যদি (rcode <= EXIT_RET_CODE)
			setup_file_in_str(inp);
#endif /* __U_BOOT__ */
		empty = 0;
		যদি (G_flag_return_in_progress == 1)
			বিরতি;
#ifndef __U_BOOT__
	}
#অন্যথা /* __U_BOOT__ */
	/*
	 * This do/while is needed by run_command to avoid looping on a command
	 * with syntax error.
	 */
	} যখন (!(G.run_command_flags & FLAG_EXIT_FROM_LOOP));

	ফেরত G.last_exitcode;
#endif /* __U_BOOT__ */
}

#ifndef __U_BOOT__
স্থির শূন্য parse_and_run_string(ধ্রুবক অক্ষর *s)
#অন্যথা /* __U_BOOT__ */
স্থির পূর্ণসংখ্যা parse_and_run_string(ধ্রুবক অক্ষর *s)
#endif /* __U_BOOT__ */
{
	কাঠামো in_str প্রবেশ;
#ifndef __U_BOOT__
	IF_HUSH_LINENO_VAR(unsigned sv = G.parse_lineno;)
#অন্যথা /* __U_BOOT__ */
	//IF_HUSH_LINENO_VAR(unsigned sv = G.parse_lineno;)
#endif /* __U_BOOT__ */

	setup_string_in_str(&প্রবেশ, s);
#ifndef __U_BOOT__
	parse_and_run_stream(&প্রবেশ, '\0');
#অন্যথা /* __U_BOOT__ */
	ফেরত parse_and_run_stream(&প্রবেশ, '\0');
#endif /* __U_BOOT__ */
#ifndef __U_BOOT__
	IF_HUSH_LINENO_VAR(unsigned sv = G.parse_lineno;)
#অন্যথা /* __U_BOOT__ */
	//IF_HUSH_LINENO_VAR(unsigned sv = G.parse_lineno;)
#endif /* __U_BOOT__ */
}

#ifdef __U_BOOT__
পূর্ণসংখ্যা parse_string_outer_modern(ধ্রুবক অক্ষর *cmd, পূর্ণসংখ্যা flags)
{
	পূর্ণসংখ্যা ret;
	পূর্ণসংখ্যা old_flags;

	/*
	 * Keep old values of run_command to be able to restore them once
	 * command was executed.
	 */
	old_flags = G.run_command_flags;
	G.run_command_flags = flags;

	ret = parse_and_run_string(cmd);

	G.run_command_flags = old_flags;

	ফেরত ret;
}
#endif /* __U_BOOT__ */
#ifndef __U_BOOT__
স্থির শূন্য parse_and_run_file(HFILE *fp)
#অন্যথা /* __U_BOOT__ */
শূন্য parse_and_run_file(শূন্য)
#endif /* __U_BOOT__ */
{
	কাঠামো in_str প্রবেশ;
#ifndef __U_BOOT__
	IF_HUSH_LINENO_VAR(unsigned sv = G.parse_lineno;)

	IF_HUSH_LINENO_VAR(G.parse_lineno = 1;)
	setup_file_in_str(&প্রবেশ, fp);
#অন্যথা /* __U_BOOT__ */
	setup_file_in_str(&প্রবেশ);
#endif /* __U_BOOT__ */
	parse_and_run_stream(&প্রবেশ, ';');
#ifndef __U_BOOT__
	IF_HUSH_LINENO_VAR(G.parse_lineno = sv;)
#endif /* !__U_BOOT__ */
}

#ifndef __U_BOOT__
#যদি ENABLE_HUSH_TICK
স্থির পূর্ণসংখ্যা generate_stream_from_string(ধ্রুবক অক্ষর *s, pid_t *pid_p)
{
	pid_t pid;
	পূর্ণসংখ্যা channel[2];
# যদি !BB_MMU
	অক্ষর **to_free = ফাঁকা;
# endif

	xpipe(channel);
	pid = BB_MMU ? xfork() : xvfork();
	যদি (pid == 0) { /* child */
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
# যদি ENABLE_HUSH_TRAP
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
		যদি (is_prefixed_with(s, "trap")
		 && skip_whitespace(s + 4)[0] == '\0'
		) {
			স্থির ধ্রুবক অক্ষর *ধ্রুবক argv[] ALIGN_PTR = { ফাঁকা, ফাঁকা };
			builtin_trap((অক্ষর**)argv);
			fflush_all(); /* important */
			_exit(0);
		}
# endif
# যদি BB_MMU
		/* Prevent it from trying to handle ctrl-z etc */
		IF_HUSH_JOB(G.run_list_level = 1;)
		CLEAR_RANDOM_T(&G.random_gen); /* or else $RANDOM repeats in child */
		reset_traps_to_defaults();
		IF_HUSH_MODE_X(G.x_mode_depth++;)
		//bb_error_msg("%s: ++x_mode_depth=%d", __func__, G.x_mode_depth);
		parse_and_run_string(s);
		_exit(G.last_exitcode);
# অন্যথা
	/* We re-execute after vfork on NOMMU. This makes this script safe:
	 * yes "0123456789012345678901234567890" | dd bs=32 count=64k >BIG
	 * huge=`cat BIG` # was blocking here forever
	 * echo OK
	 */
		re_execute_shell(&to_free,
				s,
				G.global_argv[0],
				G.global_argv + 1,
				ফাঁকা);
# endif
	}

	/* parent */
	*pid_p = pid;
# যদি ENABLE_HUSH_FAST
	G.গুনতি_SIGCHLD++;
//bb_error_msg("[%d] fork in generate_stream_from_string:"
//		" G.count_SIGCHLD:%d G.handled_SIGCHLD:%d",
//		getpid(), G.count_SIGCHLD, G.handled_SIGCHLD);
# endif
	enable_restore_tty_pgrp_on_exit();
# যদি !BB_MMU
	মুক্ত(to_free);
# endif
	close(channel[1]);
	ফেরত channel[0];
}

/* Return code is exit status of the process that is run. */
স্থির পূর্ণসংখ্যা process_command_subs(o_string *dest, ধ্রুবক অক্ষর *s)
{
	FILE *fp;
	pid_t pid;
	পূর্ণসংখ্যা status, ch, eol_cnt;

	fp = xfdopen_for_read(generate_stream_from_string(s, &pid));

	/* Now send results of command back into original context */
	eol_cnt = 0;
	যখন ((ch = getc(fp)) != EOF) {
		যদি (ch == '\0')
			চালু;
		যদি (ch == '\n') {
			eol_cnt++;
			চালু;
		}
		যখন (eol_cnt) {
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
	ফেরত WEXITSTATUS(status);
}
#endif /* ENABLE_HUSH_TICK */

স্থির শূন্য setup_heredoc(কাঠামো redir_struct *redir)
{
	কাঠামো fd_pair pair;
	pid_t pid;
	পূর্ণসংখ্যা len, written;
	/* the _body_ of heredoc (misleading field name) */
	ধ্রুবক অক্ষর *heredoc = redir->rd_filename;
	অক্ষর *expanded;
#যদি !BB_MMU
	অক্ষর **to_free;
#endif

	expanded = ফাঁকা;
	যদি (!(redir->rd_dup & HEREDOC_QUOTED)) {
		expanded = encode_then_expand_string(heredoc);
		যদি (expanded)
			heredoc = expanded;
	}
	len = strlen(heredoc);

	close(redir->rd_fd); /* often saves dup2+close in xmove_fd */
	xpiped_pair(pair);
	xmove_fd(pair.rd, redir->rd_fd);

	/* Try writing without forking. Newer kernels have
	 * dynamically growing pipes. Must use non-blocking write! */
	ndelay_on(pair.wr);
	যখন (1) {
		written = লিখ(pair.wr, heredoc, len);
		যদি (written <= 0)
			বিরতি;
		len -= written;
		যদি (len == 0) {
			close(pair.wr);
			মুক্ত(expanded);
			ফেরত;
		}
		heredoc += written;
	}
	ndelay_off(pair.wr);

	/* Okay, pipe buffer was not big enough */
	/* Note: we must not create a stray child (bastard? :)
	 * for the unsuspecting parent process. Child creates a grandchild
	 * and exits before parent execs the process which consumes heredoc
	 * (that exec happens after we return from this function) */
#যদি !BB_MMU
	to_free = ফাঁকা;
#endif
	pid = xvfork();
	যদি (pid == 0) {
		/* child */
		disable_restore_tty_pgrp_on_exit();
		pid = BB_MMU ? xfork() : xvfork();
		যদি (pid != 0)
			_exit(0);
		/* grandchild */
		close(redir->rd_fd); /* read side of the pipe */
#যদি BB_MMU
		full_write(pair.wr, heredoc, len); /* may loop or block */
		_exit(0);
#অন্যথা
		/* Delegate blocking writes to another process */
		xmove_fd(pair.wr, STDOUT_FILENO);
		re_execute_shell(&to_free, heredoc, ফাঁকা, ফাঁকা, ফাঁকা);
#endif
	}
	/* parent */
#যদি ENABLE_HUSH_FAST
	G.গুনতি_SIGCHLD++;
//bb_error_msg("[%d] fork in setup_heredoc: G.count_SIGCHLD:%d G.handled_SIGCHLD:%d", getpid(), G.count_SIGCHLD, G.handled_SIGCHLD);
#endif
	enable_restore_tty_pgrp_on_exit();
#যদি !BB_MMU
	মুক্ত(to_free);
#endif
	close(pair.wr);
	মুক্ত(expanded);
	wait(ফাঁকা); /* wait till child has died */
}

কাঠামো squirrel {
	পূর্ণসংখ্যা orig_fd;
	পূর্ণসংখ্যা moved_to;
	/* moved_to = n: fd was moved to n; restore back to orig_fd after redir */
	/* moved_to = -1: fd was opened by redirect; close orig_fd after redir */
};

স্থির কাঠামো squirrel *append_squirrel(কাঠামো squirrel *sq, পূর্ণসংখ্যা i, পূর্ণসংখ্যা orig, পূর্ণসংখ্যা moved)
{
	sq = xrealloc(sq, (i + 2) * আকার(sq[0]));
	sq[i].orig_fd = orig;
	sq[i].moved_to = moved;
	sq[i+1].orig_fd = -1; /* end marker */
	ফেরত sq;
}

স্থির কাঠামো squirrel *add_squirrel(কাঠামো squirrel *sq, পূর্ণসংখ্যা fd, পূর্ণসংখ্যা avoid_fd)
{
	পূর্ণসংখ্যা moved_to;
	পূর্ণসংখ্যা i;

	i = 0;
	যদি (sq) জন্য (; sq[i].orig_fd >= 0; i++) {
		/* If we collide with an already moved fd... */
		যদি (fd == sq[i].moved_to) {
			moved_to = dup_CLOEXEC(sq[i].moved_to, avoid_fd);
			debug_printf_redir("redirect_fd %d: already busy, moving to %d\n", fd, moved_to);
			যদি (moved_to < 0) {
				/* "echo 2>/dev/tty 10>&9999" testcase:
				 * We move fd 2 to 10, then discover we need to move fd 10
				 * (and not hit 9999) and the latter fails.
				 */
				ফেরত ফাঁকা; /* fcntl failed */
			}
			sq[i].moved_to = moved_to;
			ফেরত sq;
		}
		যদি (fd == sq[i].orig_fd) {
			/* Example: echo Hello >/dev/null 1>&2 */
			debug_printf_redir("redirect_fd %d: already moved\n", fd);
			ফেরত sq;
		}
	}

	/* If this fd is open, we move and remember it; if it's closed, moved_to = -1 */
	moved_to = dup_CLOEXEC(fd, avoid_fd);
	debug_printf_redir("redirect_fd %d: previous fd is moved to %d (-1 if it was closed)\n", fd, moved_to);
	যদি (moved_to < 0 && errno != EBADF)
		ফেরত ফাঁকা; /* fcntl failed (not because fd is closed) */
	ফেরত append_squirrel(sq, i, fd, moved_to);
}

স্থির কাঠামো squirrel *add_squirrel_closed(কাঠামো squirrel *sq, পূর্ণসংখ্যা fd)
{
	পূর্ণসংখ্যা i;

	i = 0;
	যদি (sq) জন্য (; sq[i].orig_fd >= 0; i++) {
		/* If we collide with an already moved fd... */
		যদি (fd == sq[i].orig_fd) {
			/* Examples:
			 * "echo 3>FILE 3>&- 3>FILE"
			 * "echo 3>&- 3>FILE"
			 * No need for last redirect to insert
			 * another "need to close 3" indicator.
			 */
			debug_printf_redir("redirect_fd %d: already moved or closed\n", fd);
			ফেরত sq;
		}
	}

	debug_printf_redir("redirect_fd %d: previous fd was closed\n", fd);
	ফেরত append_squirrel(sq, i, fd, -1);
}

/* fd: redirect wants this fd to be used (e.g. 3>file).
 * Move all conflicting internally used fds,
 * and remember them so that we can restore them later.
 */
স্থির পূর্ণসংখ্যা save_fd_on_redirect(পূর্ণসংখ্যা fd, পূর্ণসংখ্যা avoid_fd, কাঠামো squirrel **sqp)
{
	কাঠামো squirrel *new_squirrel;

	যদি (avoid_fd < 9) /* the important case here is that it can be -1 */
		avoid_fd = 9;

#যদি ENABLE_HUSH_INTERACTIVE
	যদি (fd != 0 /* don't trigger for G_interactive_fd == 0 (that's "not interactive" flag) */
	 && fd == G_interactive_fd
	) {
		/* Testcase: "ls -l /proc/$$/fd 255>&-" should work */
		G_interactive_fd = xdup_CLOEXEC_and_close(G_interactive_fd, avoid_fd);
		debug_printf_redir("redirect_fd %d: matches interactive_fd, moving it to %d\n", fd, G_interactive_fd);
		ফেরত 1; /* "we closed fd" */
	}
#endif
	/* Are we called from setup_redirects(squirrel==NULL)
	 * in redirect in a [v]forked child?
	 */
	যদি (sqp == ফাঁকা) {
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
		ফেরত 0;
	}

	/* If this one of script's fds? */
	যদি (move_HFILEs_on_redirect(fd, avoid_fd))
		ফেরত 1; /* yes. "we closed fd" (actually moved it) */

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
	যদি (sqp == ERR_PTR) {
		/* Don't preserve redirected fds: exec is _meant_ to change these */
		//bb_error_msg("sqp == ERR_PTR: exec >FILE");
		ফেরত 0;
	}

	/* Check whether it collides with any open fds (e.g. stdio), save fds as needed */
	new_squirrel = add_squirrel(*sqp, fd, avoid_fd);
	যদি (!new_squirrel)
		ফেরত -1; /* redirect error */
	*sqp = new_squirrel;
	ফেরত 0; /* "we did not close fd" */
}

স্থির শূন্য restore_redirects(কাঠামো squirrel *sq)
{
	যদি (sq) {
		পূর্ণসংখ্যা i;
		জন্য (i = 0; sq[i].orig_fd >= 0; i++) {
			যদি (sq[i].moved_to >= 0) {
				/* We simply die on error */
				debug_printf_redir("restoring redirected fd from %d to %d\n", sq[i].moved_to, sq[i].orig_fd);
				xmove_fd(sq[i].moved_to, sq[i].orig_fd);
			} অন্যথা {
				/* cmd1 9>FILE; cmd2_should_see_fd9_closed */
				debug_printf_redir("restoring redirected fd %d: closing it\n", sq[i].orig_fd);
				close(sq[i].orig_fd);
			}
		}
		মুক্ত(sq);
	}
	যদি (G.HFILE_stdin
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

#যদি ENABLE_FEATURE_SH_STANDALONE && BB_MMU
স্থির শূন্য close_saved_fds_and_FILE_fds(শূন্য)
{
	যদি (G_interactive_fd)
		close(G_interactive_fd);
	close_all_HFILE_list();
}
#endif

স্থির পূর্ণসংখ্যা internally_opened_fd(পূর্ণসংখ্যা fd, কাঠামো squirrel *sq)
{
	পূর্ণসংখ্যা i;

#যদি ENABLE_HUSH_INTERACTIVE
	যদি (fd == G_interactive_fd)
		ফেরত 1;
#endif
	/* If this one of script's fds? */
	যদি (fd_in_HFILEs(fd))
		ফেরত 1;

	যদি (sq) জন্য (i = 0; sq[i].orig_fd >= 0; i++) {
		যদি (fd == sq[i].moved_to)
			ফেরত 1;
	}
	ফেরত 0;
}

/* sqp != NULL means we squirrel away copies of stdin, stdout,
 * and stderr if they are redirected.
 * If redirection fails, return 1. This will make caller
 * skip command execution and restore already created redirect fds.
 */
স্থির পূর্ণসংখ্যা setup_redirects(কাঠামো command *prog, কাঠামো squirrel **sqp)
{
	কাঠামো redir_struct *redir;

	জন্য (redir = prog->redirects; redir; redir = redir->পরবর্তী) {
		পূর্ণসংখ্যা newfd;
		পূর্ণসংখ্যা closed;

		যদি (redir->rd_type == REDIRECT_HEREDOC2) {
			/* "rd_fd<<HERE" case */
			যদি (save_fd_on_redirect(redir->rd_fd, /*avoid:*/ 0, sqp) < 0)
				ফেরত 1;
			/* for REDIRECT_HEREDOC2, rd_filename holds _contents_
			 * of the heredoc */
			debug_printf_redir("set heredoc '%s'\n",
					redir->rd_filename);
			setup_heredoc(redir);
			চালু;
		}

		যদি (redir->rd_dup == REDIRFD_TO_FILE) {
			/* "rd_fd<*>file" case (<*> is <,>,>>,<>) */
			অক্ষর *p;
			পূর্ণসংখ্যা mode;

			যদি (redir->rd_filename == ফাঁকা) {
				/* Examples:
				 * "cmd >" (no filename)
				 * "cmd > <file" (2nd redirect starts too early)
				 */
				syntax_error("invalid redirect");
				ফেরত 1;
			}
			mode = redir_table[redir->rd_type].mode;
			p = expand_string_to_string(redir->rd_filename,
				EXP_FLAG_ESC_GLOB_CHARS, /*unbackslash:*/ 1);
			newfd = open_or_warn(p, mode);
			মুক্ত(p);
			যদি (newfd < 0) {
				/* Error message from open_or_warn can be lost
				 * if stderr has been redirected, but bash
				 * and ash both lose it as well
				 * (though zsh doesn't!)
				 */
				ফেরত 1;
			}
			যদি (newfd == redir->rd_fd && sqp
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
		} অন্যথা {
			/* "rd_fd>&rd_dup" or "rd_fd>&-" case */
			newfd = redir->rd_dup;
		}

		যদি (newfd == redir->rd_fd)
			চালু;

		/* if "N>FILE": move newfd to redir->rd_fd */
		/* if "N>&M": dup newfd to redir->rd_fd */
		/* if "N>&-": close redir->rd_fd (newfd is REDIRFD_CLOSE) */

		closed = save_fd_on_redirect(redir->rd_fd, /*avoid:*/ newfd, sqp);
		যদি (closed < 0)
			ফেরত 1; /* error */
		যদি (newfd == REDIRFD_CLOSE) {
			/* "N>&-" means "close me" */
			যদি (!closed) {
				/* ^^^ optimization: saving may already
				 * have closed it. If not... */
				close(redir->rd_fd);
			}
			/* Sometimes we do another close on restore, getting EBADF.
			 * Consider "echo 3>FILE 3>&-"
			 * first redirect remembers "need to close 3",
			 * and second redirect closes 3! Restore code then closes 3 again.
			 */
		} অন্যথা {
			/* if newfd is a script fd or saved fd, do not allow to use it */
			যদি (internally_opened_fd(newfd, sqp && sqp != ERR_PTR ? *sqp : ফাঁকা)) {
				bb_error_বার্তা("fd#%d is not open", newfd);
				ফেরত 1;
			}
			যদি (dup2(newfd, redir->rd_fd) < 0) {
				/* "echo >&99" testcase */
				bb_perror_বার্তা("dup2(%d,%d)", newfd, redir->rd_fd);
				ফেরত 1;
			}
			যদি (redir->rd_dup == REDIRFD_TO_FILE)
				/* "rd_fd > FILE" */
				close(newfd);
			/* else: "rd_fd > rd_dup" */
		}
	}
	ফেরত 0;
}

স্থির অক্ষর *find_in_path(ধ্রুবক অক্ষর *arg)
{
	অক্ষর *ret = ফাঁকা;
	ধ্রুবক অক্ষর *PATH = get_local_var_মান("PATH");

	যদি (!PATH)
		ফেরত ফাঁকা;

	যখন (1) {
		ধ্রুবক অক্ষর *end = strchrnul(PATH, ':');
		পূর্ণসংখ্যা sz = end - PATH; /* must be int! */

		মুক্ত(ret);
		যদি (sz != 0) {
			ret = xasprintf("%.*s/%s", sz, PATH, arg);
		} অন্যথা {
			/* We have xxx::yyyy in $PATH,
			 * it means "use current dir" */
			ret = xstrdup(arg);
		}
		যদি (access(ret, F_OK) == 0)
			বিরতি;

		যদি (*end == '\0') {
			মুক্ত(ret);
			ফেরত ফাঁকা;
		}
		PATH = end + 1;
	}

	ফেরত ret;
}

স্থির ধ্রুবক কাঠামো built_in_command *find_builtin_helper(ধ্রুবক অক্ষর *name,
		ধ্রুবক কাঠামো built_in_command *x,
		ধ্রুবক কাঠামো built_in_command *end)
{
	যখন (x != end) {
		যদি (strcmp(name, x->b_cmd) != 0) {
			x++;
			চালু;
		}
		debug_printf_exec("found builtin '%s'\n", name);
		ফেরত x;
	}
	ফেরত ফাঁকা;
}
স্থির ধ্রুবক কাঠামো built_in_command *find_builtin1(ধ্রুবক অক্ষর *name)
{
	ফেরত find_builtin_helper(name, bltins1, &bltins1[ARRAY_আকার(bltins1)]);
}
স্থির ধ্রুবক কাঠামো built_in_command *find_builtin(ধ্রুবক অক্ষর *name)
{
	ধ্রুবক কাঠামো built_in_command *x = find_builtin1(name);
	যদি (x)
		ফেরত x;
	ফেরত find_builtin_helper(name, bltins2, &bltins2[ARRAY_আকার(bltins2)]);
}

#যদি ENABLE_HUSH_JOB && ENABLE_FEATURE_TAB_COMPLETION
স্থির ধ্রুবক অক্ষর * FAST_FUNC hush_command_name(পূর্ণসংখ্যা i)
{
	যদি (/*i >= 0 && */ i < ARRAY_আকার(bltins1)) {
		ফেরত bltins1[i].b_cmd;
	}
	i -= ARRAY_আকার(bltins1);
	যদি (i < ARRAY_আকার(bltins2)) {
		ফেরত bltins2[i].b_cmd;
	}
# যদি ENABLE_HUSH_FUNCTIONS
	{
		কাঠামো function *funcp;
		i -= ARRAY_আকার(bltins2);
		জন্য (funcp = G.শীর্ষ_func; funcp; funcp = funcp->পরবর্তী) {
			যদি (--i < 0)
				ফেরত funcp->name;
		}
	}
# endif
	ফেরত ফাঁকা;
}
#endif
#endif /* !__U_BOOT__ */

#ifndef __U_BOOT__
স্থির শূন্য remove_nested_vars(শূন্য)
{
	কাঠামো variable *cur;
	কাঠামো variable **cur_pp;

	cur_pp = &G.শীর্ষ_var;
	যখন ((cur = *cur_pp) != ফাঁকা) {
		যদি (cur->var_nest_level <= G.var_nest_level) {
			cur_pp = &cur->পরবর্তী;
			চালু;
		}
		/* Unexport */
		যদি (cur->flg_export) {
			debug_printf_env("unexporting nested '%s'/%u\n", cur->varstr, cur->var_nest_level);
			bb_unsetenv(cur->varstr);
		}
		/* Remove from global list */
		*cur_pp = cur->পরবর্তী;
		/* Free */
		যদি (!cur->max_len) {
			debug_printf_env("freeing nested '%s'/%u\n", cur->varstr, cur->var_nest_level);
			মুক্ত(cur->varstr);
		}
		মুক্ত(cur);
	}
}

স্থির শূন্য enter_var_nest_level(শূন্য)
{
	G.var_nest_level++;
	debug_printf_env("var_nest_level++ %u\n", G.var_nest_level);

	/* Try:	f() { echo -n .; f; }; f
	 * struct variable::var_nest_level is uint16_t,
	 * thus limiting recursion to < 2^16.
	 * In any case, with 8 Mbyte stack SEGV happens
	 * not too long after 2^16 recursions anyway.
	 */
	যদি (G.var_nest_level > 0xff00)
		bb_error_বার্তা_and_die("fatal recursion (depth %u)", G.var_nest_level);
}

স্থির শূন্য leave_var_nest_level(শূন্য)
{
	G.var_nest_level--;
	debug_printf_env("var_nest_level-- %u\n", G.var_nest_level);
	যদি (HUSH_DEBUG && (পূর্ণসংখ্যা)G.var_nest_level < 0)
		bb_simple_error_বার্তা_and_die("BUG: nesting underflow");

	remove_nested_vars();
}
#endif /* __U_BOOT__ */

#যদি ENABLE_HUSH_FUNCTIONS
স্থির কাঠামো function **find_function_slot(ধ্রুবক অক্ষর *name)
{
	কাঠামো function *funcp;
	কাঠামো function **funcpp = &G.শীর্ষ_func;

	যখন ((funcp = *funcpp) != ফাঁকা) {
		যদি (strcmp(name, funcp->name) == 0) {
			debug_printf_exec("found function '%s'\n", name);
			বিরতি;
		}
		funcpp = &funcp->পরবর্তী;
	}
	ফেরত funcpp;
}

স্থির ALWAYS_INLINE ধ্রুবক কাঠামো function *find_function(ধ্রুবক অক্ষর *name)
{
	ধ্রুবক কাঠামো function *funcp = *find_function_slot(name);
	ফেরত funcp;
}

/* Note: takes ownership on name ptr */
স্থির কাঠামো function *new_function(অক্ষর *name)
{
	কাঠামো function **funcpp = find_function_slot(name);
	কাঠামো function *funcp = *funcpp;

	যদি (funcp != ফাঁকা) {
		কাঠামো command *cmd = funcp->parent_cmd;
		debug_printf_exec("func %p parent_cmd %p\n", funcp, cmd);
		যদি (!cmd) {
			debug_printf_exec("freeing & replacing function '%s'\n", funcp->name);
			মুক্ত(funcp->name);
			/* Note: if !funcp->body, do not free body_as_string!
			 * This is a special case of "-F name body" function:
			 * body_as_string was not malloced! */
			যদি (funcp->body) {
				free_pipe_list(funcp->body);
# যদি !BB_MMU
				মুক্ত(funcp->body_as_string);
# endif
			}
		} অন্যথা {
			debug_printf_exec("reinserting in tree & replacing function '%s'\n", funcp->name);
			cmd->argv[0] = funcp->name;
			cmd->group = funcp->body;
# যদি !BB_MMU
			cmd->group_as_string = funcp->body_as_string;
# endif
		}
	} অন্যথা {
		debug_printf_exec("remembering new function '%s'\n", name);
		funcp = *funcpp = xzalloc(আকার(*funcp));
		/*funcp->next = NULL;*/
	}

	funcp->name = name;
	ফেরত funcp;
}

# যদি ENABLE_HUSH_UNSET
স্থির শূন্য unset_func(ধ্রুবক অক্ষর *name)
{
	কাঠামো function **funcpp = find_function_slot(name);
	কাঠামো function *funcp = *funcpp;

	যদি (funcp != ফাঁকা) {
		debug_printf_exec("freeing function '%s'\n", funcp->name);
		*funcpp = funcp->পরবর্তী;
		/* funcp is unlinked now, deleting it.
		 * Note: if !funcp->body, the function was created by
		 * "-F name body", do not free ->body_as_string
		 * and ->name as they were not malloced. */
		যদি (funcp->body) {
			free_pipe_list(funcp->body);
			মুক্ত(funcp->name);
#  যদি !BB_MMU
			মুক্ত(funcp->body_as_string);
#  endif
		}
		মুক্ত(funcp);
	}
}
# endif

# যদি BB_MMU
#define exec_function(to_free, funcp, argv) \
	exec_function(funcp, argv)
# endif
স্থির শূন্য exec_function(অক্ষর ***to_free,
		ধ্রুবক কাঠামো function *funcp,
		অক্ষর **argv) NORETURN;
স্থির শূন্য exec_function(অক্ষর ***to_free,
		ধ্রুবক কাঠামো function *funcp,
		অক্ষর **argv)
{
# যদি BB_MMU
	পূর্ণসংখ্যা n;

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
# অন্যথা
//?	close_saved_fds_and_FILE_fds();

//TODO: check whether "true | func_with_return" works

	re_execute_shell(to_free,
			funcp->body_as_string,
			G.global_argv[0],
			argv + 1,
			ফাঁকা);
# endif
}

স্থির পূর্ণসংখ্যা run_function(ধ্রুবক কাঠামো function *funcp, অক্ষর **argv)
{
	পূর্ণসংখ্যা rc;
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
# যদি !BB_MMU
	যদি (!funcp->body) {
		/* Function defined by -F */
		parse_and_run_string(funcp->body_as_string);
		rc = G.last_exitcode;
	} অন্যথা
# endif
	{
		rc = run_list(funcp->body);
	}

	IF_HUSH_LOCAL(G.func_nest_level--;)
	IF_HUSH_LOCAL(leave_var_nest_level();)

	G_flag_return_in_progress = sv_flg;
# যদি ENABLE_HUSH_TRAP
	debug_printf_exec("G.return_exitcode=-1\n");
	G.return_exitcode = -1; /* invalidate stashed return value */
# endif

	restore_G_args(&sv, argv);

	ফেরত rc;
}
#endif /* ENABLE_HUSH_FUNCTIONS */

#ifndef __U_BOOT__
#যদি BB_MMU
#define exec_builtin(to_free, x, argv) \
	exec_builtin(x, argv)
#অন্যথা
#define exec_builtin(to_free, x, argv) \
	exec_builtin(to_free, argv)
#endif
স্থির শূন্য exec_builtin(অক্ষর ***to_free,
		ধ্রুবক কাঠামো built_in_command *x,
		অক্ষর **argv) NORETURN;
স্থির শূন্য exec_builtin(অক্ষর ***to_free,
		ধ্রুবক কাঠামো built_in_command *x,
		অক্ষর **argv)
{
#যদি BB_MMU
	পূর্ণসংখ্যা rcode;
//?	close_saved_fds_and_FILE_fds();
	rcode = x->b_function(argv);
	fflush_all();
	_exit(rcode);
#অন্যথা
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
স্থির শূন্য execvp_or_die(অক্ষর **argv) NORETURN;
স্থির শূন্য execvp_or_die(অক্ষর **argv)
{
	পূর্ণসংখ্যা e;
	debug_printf_exec("execing '%s'\n", argv[0]);
	/* Don't propagate SIG_IGN to the child */
	যদি (SPECIAL_JOBSTOP_SIGS != 0)
		switch_off_special_sigs(G.special_sig_mask & SPECIAL_JOBSTOP_SIGS);
	execvp(argv[0], argv);
	e = 2;
	যদি (errno == EACCES) e = 126;
	যদি (errno == ENOENT) e = 127;
	bb_perror_বার্তা("can't execute '%s'", argv[0]);
	_exit(e);
}

#যদি ENABLE_HUSH_MODE_X
স্থির শূন্য x_mode_print_optionally_squoted(ধ্রুবক অক্ষর *str)
{
	unsigned len;
	ধ্রুবক অক্ষর *cp;

	cp = str;

	/* the set of chars which-cause-string-to-be-squoted mimics bash */
	/* test a char with: bash -c 'set -x; echo "CH"' */
	যদি (str[strcspn(str, "\\\"'`$(){}[]<>;#&|~*?!^"
			" " "\001\002\003\004\005\006\007"
			"\010\011\012\013\014\015\016\017"
			"\020\021\022\023\024\025\026\027"
			"\030\031\032\033\034\035\036\037"
			)
		] == '\0'
	) {
		/* string has no special chars */
		x_mode_addstr(str);
		ফেরত;
	}

	cp = str;
	জন্য (;;) {
		/* print '....' up to EOL or first squote */
		len = (পূর্ণসংখ্যা)(strchrnul(cp, '\'') - cp);
		যদি (len != 0) {
			x_mode_addchr('\'');
			x_mode_addblock(cp, len);
			x_mode_addchr('\'');
			cp += len;
		}
		যদি (*cp == '\0')
			বিরতি;
		/* string contains squote(s), print them as \' */
		x_mode_addchr('\\');
		x_mode_addchr('\'');
		cp++;
	}
}
স্থির শূন্য dump_cmd_in_x_mode(অক্ষর **argv)
{
	যদি (G_x_mode && argv) {
		unsigned n;

		/* "+[+++...][ cmd...]\n\0" */
		x_mode_prefix();
		n = 0;
		যখন (argv[n]) {
			x_mode_addchr(' ');
			যদি (argv[n][0] == '\0') {
				x_mode_addchr('\'');
				x_mode_addchr('\'');
			} অন্যথা {
				x_mode_print_optionally_squoted(argv[n]);
			}
			n++;
		}
		x_mode_flush();
	}
}
#অন্যথা
# define dump_cmd_in_x_mode(argv) ((শূন্য)0)
#endif
#endif /* !__U_BOOT__ */

#ifndef __U_BOOT__
#যদি ENABLE_HUSH_COMMAND
স্থির শূন্য if_command_vV_print_and_exit(অক্ষর opt_vV, অক্ষর *cmd, ধ্রুবক অক্ষর *explanation)
{
	অক্ষর *to_free;

	যদি (!opt_vV)
		ফেরত;

	to_free = ফাঁকা;
	যদি (!explanation) {
		অক্ষর *path = getenv("PATH");
		explanation = to_free = find_executable(cmd, &path); /* path == NULL is ok */
		যদি (!explanation)
			_exit(1); /* PROG was not found */
		যদি (opt_vV != 'V')
			cmd = to_free; /* -v PROG prints "/path/to/PROG" */
	}
	printf((opt_vV == 'V') ? "%s is %s\n" : "%s\n", cmd, explanation);
	মুক্ত(to_free);
	fflush_all();
	_exit(0);
}
#অন্যথা
# define if_command_vV_print_and_exit(a,b,c) ((শূন্য)0)
#endif
#endif /* !__U_BOOT__ */

#যদি BB_MMU
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
স্থির শূন্য pseudo_exec_argv(nommu_save_t *nommu_save,
		অক্ষর **argv, পূর্ণসংখ্যা assignment_cnt,
		অক্ষর **argv_expanded) NORETURN;
স্থির NOINLINE শূন্য pseudo_exec_argv(nommu_save_t *nommu_save,
		অক্ষর **argv, পূর্ণসংখ্যা assignment_cnt,
		অক্ষর **argv_expanded)
{
	ধ্রুবক কাঠামো built_in_command *x;
	কাঠামো variable **sv_shadowed;
	অক্ষর **new_env;
	IF_HUSH_COMMAND(অক্ষর opt_vV = 0;)
	IF_HUSH_FUNCTIONS(ধ্রুবক কাঠামো function *funcp;)

	new_env = expand_assignments(argv, assignment_cnt);
	dump_cmd_in_x_mode(new_env);

	যদি (!argv[assignment_cnt]) {
		/* Case when we are here: ... | var=val | ...
		 * (note that we do not exit early, i.e., do not optimize out
		 * expand_assignments(): think about ... | var=`sleep 1` | ...
		 */
		free_strings(new_env);
		_exit_SUCCESS();
	}

	sv_shadowed = G.shadowed_vars_pp;
#যদি BB_MMU
	G.shadowed_vars_pp = ফাঁকা; /* "don't save, free them instead" */
#অন্যথা
	G.shadowed_vars_pp = &nommu_save->old_vars;
	G.var_nest_level++;
#endif
	set_vars_and_save_old(new_env);
	G.shadowed_vars_pp = sv_shadowed;

	যদি (argv_expanded) {
		argv = argv_expanded;
	} অন্যথা {
		argv = expand_strvec_to_strvec(argv + assignment_cnt);
#যদি !BB_MMU
		nommu_save->argv = argv;
#endif
	}
	dump_cmd_in_x_mode(argv);

#যদি ENABLE_FEATURE_SH_STANDALONE || BB_MMU
	যদি (strchr(argv[0], '/') != ফাঁকা)
		যাও skip;
#endif

#যদি ENABLE_HUSH_FUNCTIONS
	/* Check if the command matches any functions (this goes before bltins) */
	funcp = find_function(argv[0]);
	যদি (funcp)
		exec_function(&nommu_save->argv_from_re_execing, funcp, argv);
#endif

#যদি ENABLE_HUSH_COMMAND
	/* "command BAR": run BAR without looking it up among functions
	 * "command -v BAR": print "BAR" or "/path/to/BAR"; or exit 1
	 * "command -V BAR": print "BAR is {a function,a shell builtin,/path/to/BAR}"
	 */
	যখন (strcmp(argv[0], "command") == 0 && argv[1]) {
		অক্ষর *p;

		argv++;
		p = *argv;
		যদি (p[0] != '-' || !p[1])
			চালু; /* bash allows "command command command [-OPT] BAR" */

		জন্য (;;) {
			p++;
			নির্বাচন (*p) {
			ক্ষেত্র '\0':
				argv++;
				p = *argv;
				যদি (p[0] != '-' || !p[1])
					যাও after_opts;
				চালু; /* next arg is also -opts, process it too */
			ক্ষেত্র 'v':
			ক্ষেত্র 'V':
				opt_vV = *p;
				চালু;
			পূর্বনির্ধারিত:
				bb_error_বার্তা_and_die("%s: %s: invalid option", "command", argv[0]);
			}
		}
	}
 after_opts:
# যদি ENABLE_HUSH_FUNCTIONS
	যদি (opt_vV && find_function(argv[0]))
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
	যদি (x) {
		if_command_vV_print_and_exit(opt_vV, argv[0], "a shell builtin");
		exec_builtin(&nommu_save->argv_from_re_execing, x, argv);
	}

#যদি ENABLE_FEATURE_SH_STANDALONE
	/* Check if the command matches any busybox applets */
	{
		পূর্ণসংখ্যা a = find_applet_by_name(argv[0]);
		যদি (a >= 0) {
			if_command_vV_print_and_exit(opt_vV, argv[0], "an applet");
# যদি BB_MMU /* see above why on NOMMU it is not allowed */
			যদি (APPLET_IS_NOEXEC(a)) {
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
			যদি (SPECIAL_JOBSTOP_SIGS != 0)
				switch_off_special_sigs(G.special_sig_mask & SPECIAL_JOBSTOP_SIGS);
			execv(bb_busybox_exec_path, argv);
			/* If they called chroot or otherwise made the binary no longer
			 * executable, fall through */
		}
	}
#endif

#যদি ENABLE_FEATURE_SH_STANDALONE || BB_MMU
 skip:
#endif
	if_command_vV_print_and_exit(opt_vV, argv[0], ফাঁকা);
	execvp_or_die(argv);
}

/* Called after [v]fork() in run_pipe
 */
স্থির শূন্য pseudo_exec(nommu_save_t *nommu_save,
		কাঠামো command *command,
		অক্ষর **argv_expanded) NORETURN;
স্থির শূন্য pseudo_exec(nommu_save_t *nommu_save,
		কাঠামো command *command,
		অক্ষর **argv_expanded)
{
#যদি ENABLE_HUSH_FUNCTIONS
	যদি (command->cmd_type == CMD_FUNCDEF) {
		/* Ignore funcdefs in pipes:
		 * true | f() { cmd }
		 */
		_exit(0);
	}
#endif

	যদি (command->argv) {
		pseudo_exec_argv(nommu_save, command->argv,
				command->assignment_cnt, argv_expanded);
	}

	যদি (command->group) {
		/* Cases when we are here:
		 * ( list )
		 * { list } &
		 * ... | ( list ) | ...
		 * ... | { list } | ...
		 */
#যদি BB_MMU
		পূর্ণসংখ্যা rcode;
		debug_printf_exec("pseudo_exec: run_list\n");
		reset_traps_to_defaults();
		rcode = run_list(command->group);
		/* OK to leak memory by not calling free_pipe_list,
		 * since this process is about to exit */
		_exit(rcode);
#অন্যথা
		re_execute_shell(&nommu_save->argv_from_re_execing,
				command->group_as_string,
				G.global_argv[0],
				G.global_argv + 1,
				ফাঁকা);
#endif
	}

	/* Case when we are here: ... | >file */
	debug_printf_exec("pseudo_exec'ed null command\n");
	_exit_SUCCESS();
}

#যদি ENABLE_HUSH_JOB
স্থির ধ্রুবক অক্ষর *get_cmdtext(কাঠামো pipe *pi)
{
	অক্ষর **argv;
	অক্ষর *p;
	পূর্ণসংখ্যা len;

	/* This is subtle. ->cmdtext is created only on first backgrounding.
	 * (Think "cat, <ctrl-z>, fg, <ctrl-z>, fg, <ctrl-z>...." here...)
	 * On subsequent bg argv is trashed, but we won't use it */
	যদি (pi->cmdtext)
		ফেরত pi->cmdtext;

	argv = pi->cmds[0].argv;
	যদি (!argv) {
		pi->cmdtext = xzalloc(1);
		ফেরত pi->cmdtext;
	}
	len = 0;
	করো {
		len += strlen(*argv) + 1;
	} যখন (*++argv);
	p = xmalloc(len);
	pi->cmdtext = p;
	argv = pi->cmds[0].argv;
	করো {
		p = stpcpy(p, *argv);
		*p++ = ' ';
	} যখন (*++argv);
	p[-1] = '\0';
	ফেরত pi->cmdtext;
}

স্থির শূন্য remove_job_from_table(কাঠামো pipe *pi)
{
	কাঠামো pipe *পূর্ব_pipe;

	যদি (pi == G.job_list) {
		G.job_list = pi->পরবর্তী;
	} অন্যথা {
		পূর্ব_pipe = G.job_list;
		যখন (পূর্ব_pipe->পরবর্তী != pi)
			পূর্ব_pipe = পূর্ব_pipe->পরবর্তী;
		পূর্ব_pipe->পরবর্তী = pi->পরবর্তী;
	}
	G.last_jobid = 0;
	যদি (G.job_list)
		G.last_jobid = G.job_list->jobid;
}

স্থির শূন্য delete_finished_job(কাঠামো pipe *pi)
{
	remove_job_from_table(pi);
	free_pipe(pi);
}

স্থির শূন্য clean_up_last_dead_job(শূন্য)
{
	যদি (G.job_list && !G.job_list->alive_cmds)
		delete_finished_job(G.job_list);
}

স্থির শূন্য ঢোকাও_job_into_table(কাঠামো pipe *pi)
{
	কাঠামো pipe *job, **jobp;
	পূর্ণসংখ্যা i;

	clean_up_last_dead_job();

	/* Find the end of the list, and find next job ID to use */
	i = 0;
	jobp = &G.job_list;
	যখন ((job = *jobp) != ফাঁকা) {
		যদি (job->jobid > i)
			i = job->jobid;
		jobp = &job->পরবর্তী;
	}
	pi->jobid = i + 1;

	/* Create a new job struct at the end */
	job = *jobp = xmemdup(pi, আকার(*pi));
	job->পরবর্তী = ফাঁকা;
	job->cmds = xzalloc(আকার(pi->cmds[0]) * pi->অঙ্ক_cmds);
	/* Cannot copy entire pi->cmds[] vector! This causes double frees */
	জন্য (i = 0; i < pi->অঙ্ক_cmds; i++) {
		job->cmds[i].pid = pi->cmds[i].pid;
		/* all other fields are not used and stay zero */
	}
	job->cmdtext = xstrdup(get_cmdtext(pi));

	যদি (G_interactive_fd)
		printf("[%u] %u %s\n", job->jobid, (unsigned)job->cmds[0].pid, job->cmdtext);
	G.last_jobid = job->jobid;
}
#endif /* JOB */

স্থির পূর্ণসংখ্যা job_exited_or_stopped(কাঠামো pipe *pi)
{
	পূর্ণসংখ্যা rcode, i;

	যদি (pi->alive_cmds != pi->stopped_cmds)
		ফেরত -1;

	/* All processes in fg pipe have exited or stopped */
	rcode = 0;
	i = pi->অঙ্ক_cmds;
	যখন (--i >= 0) {
		rcode = pi->cmds[i].cmd_exitcode;
		/* usually last process gives overall exitstatus,
		 * but with "set -o pipefail", last *failed* process does */
		যদি (G.o_opt[OPT_O_PIPEFAIL] == 0 || rcode != 0)
			বিরতি;
	}
	IF_HAS_KEYWORDS(যদি (pi->pi_inverted) rcode = !rcode;)
	ফেরত rcode;
}

স্থির পূর্ণসংখ্যা process_wait_ফলাফল(কাঠামো pipe *fg_pipe, pid_t childpid, পূর্ণসংখ্যা status)
{
#যদি ENABLE_HUSH_JOB
	কাঠামো pipe *pi;
#endif
	পূর্ণসংখ্যা i, dead;

	dead = WIFEXITED(status) || WIFSIGNALED(status);

#যদি DEBUG_JOBS
	যদি (WIFSTOPPED(status))
		debug_printf_jobs("pid %d stopped by sig %d (exitcode %d)\n",
				childpid, WSTOPSIG(status), WEXITSTATUS(status));
	যদি (WIFSIGNALED(status))
		debug_printf_jobs("pid %d killed by sig %d (exitcode %d)\n",
				childpid, WTERMSIG(status), WEXITSTATUS(status));
	যদি (WIFEXITED(status))
		debug_printf_jobs("pid %d exited, exitcode %d\n",
				childpid, WEXITSTATUS(status));
#endif
	/* Were we asked to wait for a fg pipe? */
	যদি (fg_pipe) {
		i = fg_pipe->অঙ্ক_cmds;

		যখন (--i >= 0) {
			পূর্ণসংখ্যা rcode;

			debug_printf_jobs("check pid %d\n", fg_pipe->cmds[i].pid);
			যদি (fg_pipe->cmds[i].pid != childpid)
				চালু;
			যদি (dead) {
				পূর্ণসংখ্যা ex;
				fg_pipe->cmds[i].pid = 0;
				fg_pipe->alive_cmds--;
				ex = WEXITSTATUS(status);
				/* bash prints killer signal's name for *last*
				 * process in pipe (prints just newline for SIGINT/SIGPIPE).
				 * Mimic this. Example: "sleep 5" + (^\ or kill -QUIT)
				 */
				যদি (WIFSIGNALED(status)) {
					পূর্ণসংখ্যা sig = WTERMSIG(status);
#যদি ENABLE_HUSH_JOB
					যদি (G.run_list_level == 1
					/* ^^^^^ Do not print in nested contexts, example:
					 * echo `sleep 1; sh -c 'kill -9 $$'` - prints "137", NOT "Killed 137"
					 */
					 && i == fg_pipe->অঙ্ক_cmds-1
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
			} অন্যথা {
				fg_pipe->stopped_cmds++;
			}
			debug_printf_jobs("fg_pipe: alive_cmds %d stopped_cmds %d\n",
					fg_pipe->alive_cmds, fg_pipe->stopped_cmds);
			rcode = job_exited_or_stopped(fg_pipe);
			যদি (rcode >= 0) {
/* Note: *non-interactive* bash does not continue if all processes in fg pipe
 * are stopped. Testcase: "cat | cat" in a script (not on command line!)
 * and "killall -STOP cat" */
				যদি (G_interactive_fd) {
#যদি ENABLE_HUSH_JOB
					যদি (fg_pipe->alive_cmds != 0)
						ঢোকাও_job_into_table(fg_pipe);
#endif
					ফেরত rcode;
				}
				যদি (fg_pipe->alive_cmds == 0)
					ফেরত rcode;
			}
			/* There are still running processes in the fg_pipe */
			ফেরত -1;
		}
		/* It wasn't in fg_pipe, look for process in bg pipes */
	}

#যদি ENABLE_HUSH_JOB
	/* We were asked to wait for bg or orphaned children */
	/* No need to remember exitcode in this case */
	জন্য (pi = G.job_list; pi; pi = pi->পরবর্তী) {
		জন্য (i = 0; i < pi->অঙ্ক_cmds; i++) {
			যদি (pi->cmds[i].pid == childpid)
				যাও found_pi_and_prognum;
		}
	}
	/* Happens when shell is used as init process (init=/bin/sh) */
	debug_printf("checkjobs: pid %d was not in our list!\n", childpid);
	ফেরত -1; /* this wasn't a process from fg_pipe */

 found_pi_and_prognum:
	যদি (dead) {
		/* child exited */
		পূর্ণসংখ্যা rcode = WEXITSTATUS(status);
		যদি (WIFSIGNALED(status))
			/* NB: not 128 + sig, MIPS has sig 128 */
			rcode = 128 | WTERMSIG(status);
		pi->cmds[i].cmd_exitcode = rcode;
		যদি (G.last_bg_pid == pi->cmds[i].pid)
			G.last_bg_pid_exitcode = rcode;
		pi->cmds[i].pid = 0;
		pi->alive_cmds--;
		যদি (!pi->alive_cmds) {
# যদি ENABLE_HUSH_BASH_COMPAT
			G.dead_job_exitcode = job_exited_or_stopped(pi);
# endif
			যদি (G_interactive_fd) {
				printf(JOB_STATUS_FORMAT, pi->jobid,
						"Done", pi->cmdtext);
				delete_finished_job(pi);
			} অন্যথা {
/*
 * bash deletes finished jobs from job table only in interactive mode,
 * after "jobs" cmd, or if pid of a new process matches one of the old ones
 * (see cleanup_dead_jobs(), delete_old_job(), J_NOTIFIED in bash source).
 * Testcase script: "(exit 3) & sleep 1; wait %1; echo $?" prints 3 in bash.
 * We only retain one "dead" job, if it's the single job on the list.
 * This covers most of real-world scenarios where this is useful.
 */
				যদি (pi != G.job_list)
					delete_finished_job(pi);
			}
		}
	} অন্যথা {
		/* child stopped */
		pi->stopped_cmds++;
	}
#endif
	ফেরত -1; /* this wasn't a process from fg_pipe */
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
স্থির পূর্ণসংখ্যা checkjobs(কাঠামো pipe *fg_pipe, pid_t waitfor_pid)
{
	পূর্ণসংখ্যা attributes;
	পূর্ণসংখ্যা status;
	পূর্ণসংখ্যা rcode = 0;

	debug_printf_jobs("checkjobs %p\n", fg_pipe);

	attributes = WUNTRACED;
	যদি (fg_pipe == ফাঁকা)
		attributes |= WNOHANG;

	errno = 0;
#যদি ENABLE_HUSH_FAST
	যদি (G.handled_SIGCHLD == G.গুনতি_SIGCHLD) {
//bb_error_msg("[%d] checkjobs: G.count_SIGCHLD:%d G.handled_SIGCHLD:%d children?:%d fg_pipe:%p",
//getpid(), G.count_SIGCHLD, G.handled_SIGCHLD, G.we_have_children, fg_pipe);
		/* There was neither fork nor SIGCHLD since last waitpid */
		/* Avoid doing waitpid syscall if possible */
		যদি (!G.we_have_children) {
			errno = ECHILD;
			ফেরত -1;
		}
		যদি (fg_pipe == ফাঁকা) { /* is WNOHANG set? */
			/* We have children, but they did not exit
			 * or stop yet (we saw no SIGCHLD) */
			ফেরত 0;
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
	যখন (1) {
		pid_t childpid;
#যদি ENABLE_HUSH_FAST
		পূর্ণসংখ্যা i;
		i = G.গুনতি_SIGCHLD;
#endif
		childpid = waitpid(-1, &status, attributes);
		যদি (childpid <= 0) {
			যদি (childpid && errno != ECHILD)
				bb_simple_perror_বার্তা("waitpid");
#যদি ENABLE_HUSH_FAST
			অন্যথা { /* Until next SIGCHLD, waitpid's are useless */
				G.we_have_children = (childpid == 0);
				G.handled_SIGCHLD = i;
//bb_error_msg("[%d] checkjobs: waitpid returned <= 0, G.count_SIGCHLD:%d G.handled_SIGCHLD:%d", getpid(), G.count_SIGCHLD, G.handled_SIGCHLD);
			}
#endif
			/* ECHILD (no children), or 0 (no change in children status) */
			rcode = childpid;
			বিরতি;
		}
		rcode = process_wait_ফলাফল(fg_pipe, childpid, status);
		যদি (rcode >= 0) {
			/* fg_pipe exited or stopped */
			বিরতি;
		}
		যদি (childpid == waitfor_pid) { /* "wait PID" */
			debug_printf_exec("childpid==waitfor_pid:%d status:0x%08x\n", childpid, status);
			rcode = WEXITSTATUS(status);
			যদি (WIFSIGNALED(status))
				rcode = 128 | WTERMSIG(status);
			যদি (WIFSTOPPED(status))
				/* bash: "cmd & wait $!" and cmd stops: $? = 128 | stopsig */
				rcode = 128 | WSTOPSIG(status);
			rcode++;
			বিরতি; /* "wait PID" called us, give it exitcode+1 */
		}
#যদি ENABLE_HUSH_BASH_COMPAT
		যদি (-1 == waitfor_pid /* "wait -n" (wait for any one job) */
		 && G.dead_job_exitcode >= 0 /* some job did finish */
		) {
			debug_printf_exec("waitfor_pid:-1\n");
			rcode = G.dead_job_exitcode + 1;
			বিরতি;
		}
#endif
		/* This wasn't one of our processes, or */
		/* fg_pipe still has running processes, do waitpid again */
	} /* while (waitpid succeeds)... */

	ফেরত rcode;
}

#যদি ENABLE_HUSH_JOB
স্থির পূর্ণসংখ্যা checkjobs_and_fg_shell(কাঠামো pipe *fg_pipe)
{
	pid_t p;
	পূর্ণসংখ্যা rcode = checkjobs(fg_pipe, 0 /*(no pid to wait for)*/);
	যদি (G_saved_tty_pgrp) {
		/* Job finished, move the shell to the foreground */
		p = getpgrp(); /* our process group id */
		debug_printf_jobs("fg'ing ourself: getpgrp()=%d\n", (পূর্ণসংখ্যা)p);
		tcsetpgrp(G_interactive_fd, p);
	}
	ফেরত rcode;
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
স্থির শূন্য set_G_ifs(শূন্য)
{
	/* Testcase: set -- q w e; (IFS='' echo "$*"; IFS=''; echo "$*"); echo "$*"
	 * Result should be 3 lines: q w e, qwe, q w e
	 */
	যদি (G.ifs_whitespace != G.ifs)
		মুক্ত(G.ifs_whitespace);
	G.ifs = get_local_var_মান("IFS");
	যদি (G.ifs) {
		অক্ষর *p;
		G.ifs_whitespace = (অক্ষর*)G.ifs;
		p = skip_whitespace(G.ifs);
		যদি (*p) {
			/* Not all $IFS is whitespace */
			অক্ষর *d;
			পূর্ণসংখ্যা len = p - G.ifs;
			p = skip_non_whitespace(p);
			G.ifs_whitespace = xmalloc(len + strlen(p) + 1); /* can overestimate */
			d = mempcpy(G.ifs_whitespace, G.ifs, len);
			যখন (*p) {
				যদি (isspace(*p))
					*d++ = *p;
				p++;
			}
			*d = '\0';
		}
	} অন্যথা {
		G.ifs = defifs;
		G.ifs_whitespace = (অক্ষর*)G.ifs;
	}
}
#ifndef __U_BOOT__
#যদি !ENABLE_HUSH_MODE_X
#define redirect_and_varexp_helper(command, sqp, argv_expanded) \
	redirect_and_varexp_helper(command, sqp)
#endif
স্থির পূর্ণসংখ্যা redirect_and_varexp_helper(
		কাঠামো command *command,
		কাঠামো squirrel **sqp,
		অক্ষর **argv_expanded)
{
	/* Assignments occur before redirects. Try:
	 * a=`sleep 1` sleep 2 3>/qwe/rty
	 */

	অক্ষর **new_env = expand_assignments(command->argv, command->assignment_cnt);
	dump_cmd_in_x_mode(new_env);
	dump_cmd_in_x_mode(argv_expanded);
	/* this takes ownership of new_env[i] elements, and frees new_env: */
	set_vars_and_save_old(new_env);

	ফেরত setup_redirects(command, sqp);
}
#endif /* !__U_BOOT__ */

স্থির NOINLINE পূর্ণসংখ্যা run_pipe(কাঠামো pipe *pi)
{
	স্থির ধ্রুবক অক্ষর *ধ্রুবক null_ptr = ফাঁকা;

	পূর্ণসংখ্যা cmd_no;
#ifndef __U_BOOT__
	পূর্ণসংখ্যা পরবর্তী_infd;
#endif /* !__U_BOOT__ */
	কাঠামো command *command;
	অক্ষর **argv_expanded;
	অক্ষর **argv;
#ifndef __U_BOOT__
	কাঠামো squirrel *squirrel = ফাঁকা;
#endif /* !__U_BOOT__ */
	পূর্ণসংখ্যা rcode;

#ifdef __U_BOOT__
	/*
	 * Set rcode here to avoid returning a garbage value in the middle of
	 * the function.
	 * Also, if an error occurs, rcode value would be changed and last
	 * return will signal the error.
	 */
	rcode = 0;
#endif /* __U_BOOT__ */

	debug_printf_exec("run_pipe start: members:%d\n", pi->অঙ্ক_cmds);
	debug_enter();

	set_G_ifs();

#ifndef __U_BOOT__
	IF_HUSH_JOB(pi->pgrp = -1;)
	pi->stopped_cmds = 0;
#endif /* !__U_BOOT__ */
	command = &pi->cmds[0];
	argv_expanded = ফাঁকা;

#ifndef __U_BOOT__
	যদি (pi->অঙ্ক_cmds != 1
	 || pi->followup == PIPE_BG
	 || command->cmd_type == CMD_SUBSHELL
	) {
		যাও must_fork;
	}

	pi->alive_cmds = 1;
#endif /* !__U_BOOT__ */

	debug_printf_exec(": group:%p argv:'%s'\n",
		command->group, command->argv ? command->argv[0] : "NONE");

	যদি (command->group) {
#যদি ENABLE_HUSH_FUNCTIONS
		যদি (command->cmd_type == CMD_FUNCDEF) {
			/* "executing" func () { list } */
			কাঠামো function *funcp;

			funcp = new_function(command->argv[0]);
			/* funcp->name is already set to argv[0] */
			funcp->body = command->group;
# যদি !BB_MMU
			funcp->body_as_string = command->group_as_string;
			command->group_as_string = ফাঁকা;
# endif
			command->group = ফাঁকা;
			command->argv[0] = ফাঁকা;
			debug_printf_exec("cmd %p has child func at %p\n", command, funcp);
			funcp->parent_cmd = command;
			command->child_func = funcp;

			debug_printf_exec("run_pipe: return EXIT_SUCCESS\n");
			debug_leave();
			ফেরত EXIT_SUCCESS;
		}
#endif
		/* { list } */
		debug_printf_exec("non-subshell group\n");
		rcode = 1; /* exitcode if redir failed */
#ifndef __U_BOOT__
		যদি (setup_redirects(command, &squirrel) == 0) {
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
		IF_HAS_KEYWORDS(যদি (pi->pi_inverted) rcode = !rcode;)
		debug_leave();
		debug_printf_exec("run_pipe: return %d\n", rcode);
		ফেরত rcode;
	}

	argv = command->argv ? command->argv : (অক্ষর **) &null_ptr;
	{
#ifndef __U_BOOT__
		ধ্রুবক কাঠামো built_in_command *x;
		IF_HUSH_FUNCTIONS(ধ্রুবক কাঠামো function *funcp;)
		IF_NOT_HUSH_FUNCTIONS(গণনা { funcp = 0 };)
		কাঠামো variable **sv_shadowed;
#endif /* !__U_BOOT__ */
		কাঠামো variable *old_vars;

#যদি ENABLE_HUSH_LINENO_VAR
		G.execute_lineno = command->lineno;
#endif

		যদি (argv[command->assignment_cnt] == ফাঁকা) {
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
			যখন (i < command->assignment_cnt) {
				অক্ষর *p = expand_string_to_string(argv[i],
						EXP_FLAG_ESC_GLOB_CHARS,
						/*unbackslash:*/ 1
				);
#যদি ENABLE_HUSH_MODE_X
				যদি (G_x_mode) {
					অক্ষর *eq;
					যদি (i == 0)
						x_mode_prefix();
					x_mode_addchr(' ');
					eq = strchrnul(p, '=');
					যদি (*eq) eq++;
					x_mode_addblock(p, (eq - p));
					x_mode_print_optionally_squoted(eq);
					x_mode_flush();
				}
#endif
				debug_printf_env("set shell var:'%s'->'%s'\n", *argv, p);
#ifndef __U_BOOT__
				যদি (set_local_var0(p)) {
#অন্যথা /* __U_BOOT__ */
				যদি (set_local_var_modern(p, /*flag:*/ 0)) {
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
			যদি (rcode == 0)
				rcode = G.expand_exitcode;
			IF_HAS_KEYWORDS(যদি (pi->pi_inverted) rcode = !rcode;)
			debug_leave();
			debug_printf_exec("run_pipe: return %d\n", rcode);
			ফেরত rcode;
		}

		/* Expand the rest into (possibly) many strings each */
#যদি defined(CMD_TEST2_SINGLEWORD_NOGLOB)
		যদি (command->cmd_type == CMD_TEST2_SINGLEWORD_NOGLOB)
			argv_expanded = expand_strvec_to_strvec_singleword_noglob(argv + command->assignment_cnt);
		অন্যথা
#endif
#যদি defined(CMD_SINGLEWORD_NOGLOB)
		যদি (command->cmd_type == CMD_SINGLEWORD_NOGLOB)
			argv_expanded = expand_strvec_to_strvec_singleword_noglob(argv + command->assignment_cnt);
		অন্যথা
#endif
			argv_expanded = expand_strvec_to_strvec(argv + command->assignment_cnt);

		/* If someone gives us an empty string: `cmd with empty output` */
		যদি (!argv_expanded[0]) {
			মুক্ত(argv_expanded);
			/* `false` still has to set exitcode 1 */
			G.expand_exitcode = G.last_exitcode;
			যাও only_assignments;
		}

		old_vars = ফাঁকা;
#ifndef __U_BOOT__
		sv_shadowed = G.shadowed_vars_pp;

		/* Check if argv[0] matches any functions (this goes before bltins) */
		IF_HUSH_FUNCTIONS(funcp = find_function(argv_expanded[0]);)
		IF_HUSH_FUNCTIONS(x = ফাঁকা;)
		IF_HUSH_FUNCTIONS(যদি (!funcp))
			x = find_builtin(argv_expanded[0]);
		যদি (x || funcp) {
			যদি (x && x->b_function == builtin_exec && argv_expanded[1] == ফাঁকা) {
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

				যাও clean_up_and_ret1;
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
			যদি (rcode == 0) {
				যদি (!funcp) {
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
#যদি ENABLE_HUSH_FUNCTIONS
				অন্যথা {
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
		} অন্যথা
		যদি (ENABLE_FEATURE_SH_NOFORK && অঙ্ক_APPLETS > 1) {
			পূর্ণসংখ্যা n = find_applet_by_name(argv_expanded[0]);
			যদি (n < 0 || !APPLET_IS_NOFORK(n))
				যাও must_fork;

			enter_var_nest_level();
			/* Collect all variables "shadowed" by helper into old_vars list */
			G.shadowed_vars_pp = &old_vars;
			rcode = redirect_and_varexp_helper(command, &squirrel, argv_expanded);
			G.shadowed_vars_pp = sv_shadowed;

			যদি (rcode == 0) {
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
		} অন্যথা
			যাও must_fork;

		restore_redirects(squirrel);
 clean_up_and_ret1:
		leave_var_nest_level();
		add_vars(old_vars);

		/*
		 * Try "usleep 99999999" + ^C + "echo $?"
		 * with FEATURE_SH_NOFORK=y.
		 */
		যদি (!funcp) {
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
			যদি (sigismember(&G.pending_set, SIGINT))
				rcode = 128 | SIGINT;
		}
		মুক্ত(argv_expanded);
		IF_HAS_KEYWORDS(যদি (pi->pi_inverted) rcode = !rcode;)
		debug_leave();
		debug_printf_exec("run_pipe return %d\n", rcode);
		ফেরত rcode;
#endif /* !__U_BOOT__ */
	}

#ifndef __U_BOOT__
 must_fork:
	/* NB: argv_expanded may already be created, and that
	 * might include `cmd` runs! Do not rerun it! We *must*
	 * use argv_expanded if it's non-NULL */

	/* Going to fork a child per each pipe member */
	pi->alive_cmds = 0;
	পরবর্তী_infd = 0;
#endif /* !__U_BOOT__ */

	cmd_no = 0;
	যখন (cmd_no < pi->অঙ্ক_cmds) {
#ifndef __U_BOOT__
		কাঠামো fd_pair pipefds;
#যদি !BB_MMU
		পূর্ণসংখ্যা sv_var_nest_level = G.var_nest_level;
		অস্থির nommu_save_t nommu_save;
		nommu_save.old_vars = ফাঁকা;
		nommu_save.argv = ফাঁকা;
		nommu_save.argv_from_re_execing = ফাঁকা;
#endif
#endif /* !__U_BOOT__ */
		command = &pi->cmds[cmd_no];
		cmd_no++;

#ifdef __U_BOOT__
		/* Replace argv and argc by expanded if it exists. */
		যদি (argv_expanded) {
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
			command->argc = list_আকার(argv_expanded);
			command->argv = argv_expanded;
		}
#endif /* __U_BOOT__ */
			যদি (command->argv) {
			debug_printf_exec(": pipe member '%s' '%s'...\n",
					command->argv[0], command->argv[1]);
		} অন্যথা {
			debug_printf_exec(": pipe member with no argv\n");
		}

#ifndef __U_BOOT__
		/* pipes are inserted between pairs of commands */
		pipefds.rd = 0;
		pipefds.wr = 1;
		যদি (cmd_no < pi->অঙ্ক_cmds)
			xpiped_pair(pipefds);

#যদি ENABLE_HUSH_LINENO_VAR
		G.execute_lineno = command->lineno;
#endif

		command->pid = BB_MMU ? fork() : vfork();
		যদি (!command->pid) { /* child */
#যদি ENABLE_HUSH_JOB
			disable_restore_tty_pgrp_on_exit();
			CLEAR_RANDOM_T(&G.random_gen); /* or else $RANDOM repeats in child */

			/* Every child adds itself to new process group
			 * with pgid == pid_of_first_child_in_pipe */
			যদি (G.run_list_level == 1 && G_interactive_fd) {
				pid_t pgrp;
				pgrp = pi->pgrp;
				যদি (pgrp < 0) /* true for 1st process only */
					pgrp = getpid();
				যদি (setpgid(0, pgrp) == 0
				 && pi->followup != PIPE_BG
				 && G_saved_tty_pgrp /* we have ctty */
				) {
					/* We do it in *every* child, not just first,
					 * to avoid races */
					tcsetpgrp(G_interactive_fd, pgrp);
				}
			}
#endif
			যদি (pi->alive_cmds == 0 && pi->followup == PIPE_BG) {
				/* 1st cmd in backgrounded pipe
				 * should have its stdin /dev/null'ed */
				close(0);
				যদি (open(bb_dev_null, O_RDONLY))
					xopen("/", O_RDONLY);
			} অন্যথা {
				xmove_fd(পরবর্তী_infd, 0);
			}
			xmove_fd(pipefds.wr, 1);
			যদি (pipefds.rd > 1)
				close(pipefds.rd);
			/* Like bash, explicit redirects override pipes,
			 * and the pipe fd (fd#1) is available for dup'ing:
			 * "cmd1 2>&1 | cmd2": fd#1 is duped to fd#2, thus stderr
			 * of cmd1 goes into pipe.
			 */
			যদি (setup_redirects(command, ফাঁকা)) {
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
#যদি ENABLE_HUSH_FAST
		G.গুনতি_SIGCHLD++;
//bb_error_msg("[%d] fork in run_pipe: G.count_SIGCHLD:%d G.handled_SIGCHLD:%d", getpid(), G.count_SIGCHLD, G.handled_SIGCHLD);
#endif
		enable_restore_tty_pgrp_on_exit();
#যদি !BB_MMU
		/* Clean up after vforked child */
		মুক্ত(nommu_save.argv);
		মুক্ত(nommu_save.argv_from_re_execing);
		G.var_nest_level = sv_var_nest_level;
		remove_nested_vars();
		add_vars(nommu_save.old_vars);
#endif
		মুক্ত(argv_expanded);
		argv_expanded = ফাঁকা;
		যদি (command->pid < 0) { /* [v]fork failed */
			/* Clearly indicate, was it fork or vfork */
			bb_simple_perror_বার্তা(BB_MMU ? "vfork"+1 : "vfork");
		} অন্যথা {
			pi->alive_cmds++;
#যদি ENABLE_HUSH_JOB
			/* Second and next children need to know pid of first one */
			যদি (pi->pgrp < 0)
				pi->pgrp = command->pid;
#endif
		}

		যদি (cmd_no > 1)
			close(পরবর্তী_infd);
		যদি (cmd_no < pi->অঙ্ক_cmds)
			close(pipefds.wr);
		/* Pass read (output) pipe end to next iteration */
		পরবর্তী_infd = pipefds.rd;
#অন্যথা /* __U_BOOT__ */
		/* Process the command */
		rcode = cmd_process(G.do_repeat ? CMD_FLAG_REPEAT : 0,
				    command->argc, command->argv,
				    &(G.flag_repeat), ফাঁকা);

		যদি (argv_expanded) {
			/*
			 * expand_strvec_to_strvec() allocates memory to expand
			 * argv, we need to free it.
			 */
			মুক্ত(argv_expanded);

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
	যদি (!pi->alive_cmds) {
		debug_leave();
		debug_printf_exec("run_pipe return 1 (all forks failed, no children)\n");
		ফেরত 1;
	}
#endif /* __U_BOOT__ */

	debug_leave();
#ifndef __U_BOOT__
	debug_printf_exec("run_pipe return -1 (%u children started)\n", pi->alive_cmds);
	ফেরত -1;
#অন্যথা /* __U_BOOT__ */
	IF_HAS_KEYWORDS(যদি (pi->pi_inverted) rcode = !rcode;)
	debug_printf_exec("run_pipe return %d\n", rcode);
	ফেরত rcode;
#endif /* __U_BOOT__ */
}

/* NB: called by pseudo_exec, and therefore must not modify any
 * global data until exec/_exit (we can be a child after vfork!) */
স্থির পূর্ণসংখ্যা run_list(কাঠামো pipe *pi)
{
#যদি ENABLE_HUSH_CASE
	অক্ষর *case_word = ফাঁকা;
#endif
#যদি ENABLE_HUSH_LOOPS
	কাঠামো pipe *loop_শীর্ষ = ফাঁকা;
	অক্ষর **for_lcur = ফাঁকা;
	অক্ষর **for_list = ফাঁকা;
#endif
	smallint last_followup;
	smalluint rcode;
#যদি ENABLE_HUSH_IF || ENABLE_HUSH_CASE
	smalluint cond_code = 0;
#অন্যথা
	গণনা { cond_code = 0 };
#endif
#যদি HAS_KEYWORDS
	smallint rword;      /* RES_foo */
	smallint last_rword; /* ditto */
#endif

#ifndef __U_BOOT__
	debug_printf_exec("run_list lvl %d start\n", G.run_list_level);
	debug_enter();
#endif /* !__U_BOOT__ */

	set_G_ifs();

#যদি ENABLE_HUSH_LOOPS
	/* Check syntax for "for" */
	{
		কাঠামো pipe *cpipe;
		জন্য (cpipe = pi; cpipe; cpipe = cpipe->পরবর্তী) {
			যদি (cpipe->res_word != RES_FOR && cpipe->res_word != RES_IN)
				চালু;
			/* current word is FOR or IN (BOLD in comments below) */
			যদি (cpipe->পরবর্তী == ফাঁকা) {
				syntax_error("malformed for");
				debug_leave();
				debug_printf_exec("run_list lvl %d return 1\n", G.run_list_level);
				ফেরত 1;
			}
			/* "FOR v; do ..." and "for v IN a b; do..." are ok */
			যদি (cpipe->পরবর্তী->res_word == RES_DO)
				চালু;
			/* next word is not "do". It must be "in" then ("FOR v in ...") */
			যদি (cpipe->res_word == RES_IN /* "for v IN a b; not_do..."? */
			 || cpipe->পরবর্তী->res_word != RES_IN /* FOR v not_do_and_not_in..."? */
			) {
				syntax_error("malformed for");
				debug_leave();
				debug_printf_exec("run_list lvl %d return 1\n", G.run_list_level);
				ফেরত 1;
			}
		}
	}
#endif

	/* Past this point, all code paths should jump to ret: label
	 * in order to return, no direct "return" statements please.
	 * This helps to ensure that no memory is leaked. */

#যদি ENABLE_HUSH_JOB
	G.run_list_level++;
#endif

#যদি HAS_KEYWORDS
	rword = RES_NONE;
	last_rword = RES_XXXX;
#endif
	last_followup = PIPE_SEQ;
	rcode = G.last_exitcode;

	/* Go through list of pipes, (maybe) executing them. */
#ifndef __U_BOOT__
	জন্য (; pi; pi = IF_HUSH_LOOPS(rword == RES_DONE ? loop_শীর্ষ : ) pi->পরবর্তী) {
#অন্যথা /* __U_BOOT__ */
	জন্য (; pi; pi = rword == RES_DONE ? loop_শীর্ষ : pi->পরবর্তী) {
#endif /* __U_BOOT__ */
		পূর্ণসংখ্যা r;
		পূর্ণসংখ্যা sv_errexit_depth;

#ifndef __U_BOOT__
		যদি (G.flag_SIGINT)
			বিরতি;
		যদি (G_flag_return_in_progress == 1)
			বিরতি;
#endif /* !__U_BOOT__ */

		IF_HAS_KEYWORDS(rword = pi->res_word;)
		debug_printf_exec(": rword:%d cond_code:%d last_rword:%d\n",
				rword, cond_code, last_rword);

		sv_errexit_depth = G.errexit_depth;
		যদি (
#যদি ENABLE_HUSH_IF
		    rword == RES_IF || rword == RES_ELIF ||
#endif
		    pi->followup != PIPE_SEQ
		) {
			G.errexit_depth++;
		}
#যদি ENABLE_HUSH_LOOPS
		যদি ((rword == RES_WHILE || rword == RES_UNTIL || rword == RES_FOR)
		 && loop_শীর্ষ == ফাঁকা /* avoid bumping G.depth_of_loop twice */
		) {
			/* start of a loop: remember where loop starts */
			loop_শীর্ষ = pi;
			G.depth_of_loop++;
		}
#endif
		/* Still in the same "if...", "then..." or "do..." branch? */
		যদি (IF_HAS_KEYWORDS(rword == last_rword &&) 1) {
			যদি ((rcode == 0 && last_followup == PIPE_OR)
			 || (rcode != 0 && last_followup == PIPE_AND)
			) {
				/* It is "<true> || CMD" or "<false> && CMD"
				 * and we should not execute CMD */
				debug_printf_exec("skipped cmd because of || or &&\n");
				last_followup = pi->followup;
				যাও dont_check_jobs_but_continue;
			}
		}
		last_followup = pi->followup;
#যদি ENABLE_HUSH_IF
		যদি (cond_code != 0) {
			যদি (rword == RES_THEN) {
				/* if false; then ... fi has exitcode 0! */
				G.last_exitcode = rcode = EXIT_SUCCESS;
				/* "if <false> THEN cmd": skip cmd */
				debug_printf_exec("skipped THEN cmd because IF condition was false\n");
				last_rword = rword;
				চালু;
			}
		} অন্যথা {
			যদি (rword == RES_ELSE
			 || (rword == RES_ELIF && last_rword != RES_ELIF)
			) {
				/* "if <true> then ... ELSE/ELIF cmd":
				 * skip cmd and all following ones */
				debug_printf_exec("skipped ELSE/ELIF branch because IF condition was true\n");
				বিরতি;
			}
			//if (rword == RES_THEN): "if <true> THEN cmd", run cmd (fall through)
		}
#endif
		IF_HAS_KEYWORDS(last_rword = rword;)
#যদি ENABLE_HUSH_LOOPS
		যদি (rword == RES_FOR) { /* && pi->num_cmds - always == 1 */
			যদি (!for_lcur) {
				/* first loop through for */

				স্থির ধ্রুবক অক্ষর encoded_dollar_at[] ALIGN1 = {
					SPECIAL_VAR_প্রতীক, '@' | 0x80, SPECIAL_VAR_প্রতীক, '\0'
				}; /* encoded representation of "$@" */
				স্থির ধ্রুবক অক্ষর *ধ্রুবক encoded_dollar_at_argv[] ALIGN_PTR = {
					encoded_dollar_at, ফাঁকা
				}; /* argv list with one element: "$@" */
				অক্ষর **vals;

				G.last_exitcode = rcode = EXIT_SUCCESS;
				vals = (অক্ষর**)encoded_dollar_at_argv;
				যদি (pi->পরবর্তী->res_word == RES_IN) {
					/* if no variable values after "in" we skip "for" */
					যদি (!pi->পরবর্তী->cmds[0].argv) {
						debug_printf_exec(": null FOR: exitcode EXIT_SUCCESS\n");
						বিরতি;
					}
					vals = pi->পরবর্তী->cmds[0].argv;
				} /* else: "for var; do..." -> assume "$@" list */
				/* create list of variable values */
				debug_print_strings("for_list made from", vals);
				for_list = expand_strvec_to_strvec(vals);
				for_lcur = for_list;
				debug_print_strings("for_list", for_list);
			}
			যদি (!*for_lcur) {
				/* "for" loop is over, clean up */
				মুক্ত(for_list);
				for_list = ফাঁকা;
				for_lcur = ফাঁকা;
				বিরতি;
			}
			/* Insert next value from for_lcur */
			/* note: *for_lcur already has quotes removed, $var expanded, etc */
#ifndef __U_BOOT__
			set_local_var_from_halves(pi->cmds[0].argv[0], *for_lcur++);
#অন্যথা /* __U_BOOT__ */
			/* We cannot use xasprintf, so we emulate it. */
			অক্ষর *full_var;
			অক্ষর *var = pi->cmds[0].argv[0];
			অক্ষর *val = *for_lcur++;

			/* + 1 to take into account =. */
			full_var = xmalloc(strlen(var) + strlen(val) + 1);
			sprintf(full_var, "%s=%s", var, val);

			set_local_var_modern(full_var, /*flag:*/ 0);
#endif /* __U_BOOT__ */
			চালু;
		}
		যদি (rword == RES_IN) {
			চালু; /* "for v IN list;..." - "in" has no cmds anyway */
		}
		যদি (rword == RES_DONE) {
			চালু; /* "done" has no cmds too */
		}
#endif
#যদি ENABLE_HUSH_CASE
		যদি (rword == RES_CASE) {
			debug_printf_exec("CASE cond_code:%d\n", cond_code);
			case_word = expand_string_to_string(pi->cmds->argv[0],
				EXP_FLAG_ESC_GLOB_CHARS, /*unbackslash:*/ 1);
			debug_printf_exec("CASE word1:'%s'\n", case_word);
			//unbackslash(case_word);
			//debug_printf_exec("CASE word2:'%s'\n", case_word);
			চালু;
		}
		যদি (rword == RES_MATCH) {
			অক্ষর **argv;

			debug_printf_exec("MATCH cond_code:%d\n", cond_code);
			যদি (!case_word) /* "case ... matched_word) ... WORD)": we executed selected branch, stop */
				বিরতি;
			/* all prev words didn't match, does this one match? */
			argv = pi->cmds->argv;
			যখন (*argv) {
				অক্ষর *pattern;
				debug_printf_exec("expand_string_to_string('%s')\n", *argv);
				pattern = expand_string_to_string(*argv,
						EXP_FLAG_ESC_GLOB_CHARS,
						/*unbackslash:*/ 0
				);
				/* TODO: which FNM_xxx flags to use? */
				cond_code = (fnmatch(pattern, case_word, /*flags:*/ 0) != 0);
				debug_printf_exec("cond_code=fnmatch(pattern:'%s',str:'%s'):%d\n",
						pattern, case_word, cond_code);
				মুক্ত(pattern);
				যদি (cond_code == 0) {
					/* match! we will execute this branch */
					মুক্ত(case_word);
					case_word = ফাঁকা; /* make future "word)" stop */
					বিরতি;
				}
				argv++;
			}
			চালু;
		}
		যদি (rword == RES_CASE_BODY) { /* inside of a case branch */
			debug_printf_exec("CASE_BODY cond_code:%d\n", cond_code);
			যদি (cond_code != 0)
				চালু; /* not matched yet, skip this pipe */
		}
		যদি (rword == RES_ESAC) {
			debug_printf_exec("ESAC cond_code:%d\n", cond_code);
			যদি (case_word) {
				/* "case" did not match anything: still set $? (to 0) */
				G.last_exitcode = rcode = EXIT_SUCCESS;
			}
		}
#endif
		/* Just pressing <enter> in shell should check for jobs.
		 * OTOH, in non-interactive shell this is useless
		 * and only leads to extra job checks */
		যদি (pi->অঙ্ক_cmds == 0) {
#ifndef __U_BOOT__
			যদি (G_interactive_fd)
				যাও check_jobs_and_continue;
#endif /* !__U_BOOT__ */
			চালু;
		}

		/* After analyzing all keywords and conditions, we decided
		 * to execute this pipe. NB: have to do checkjobs(NULL)
		 * after run_pipe to collect any background children,
		 * even if list execution is to be stopped. */
		debug_printf_exec(": run_pipe with %d members\n", pi->অঙ্ক_cmds);
#ifndef __U_BOOT__
#যদি ENABLE_HUSH_LOOPS
		G.flag_break_continue = 0;
#endif
#endif /* !__U_BOOT__ */
#ifndef __U_BOOT__
		rcode = r = G.o_opt[OPT_O_NOEXEC] ? 0 : run_pipe(pi);
		/* NB: rcode is a smalluint, r is int */
#অন্যথা /* __U_BOOT__ */
		rcode = r = run_pipe(pi); /* NB: rcode is a smalluint, r is int */
		যদি (r <= EXIT_RET_CODE) {
			পূর্ণসংখ্যা previous_rcode = G.last_exitcode;
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
			যদি (previous_rcode != r) {
				/*
				 * If run from run_command, run_command_flags will be set, so we check
				 * this to know if we are in main input shell.
				 */
				যদি (!G.run_command_flags)
					printf("exit not allowed from main input shell.\n");

				বিরতি;
			}
			চালু;
		}
#endif /* __U_BOOT__ */
		যদি (r != -1) {
			/* We ran a builtin, function, or group.
			 * rcode is already known
			 * and we don't need to wait for anything. */
			debug_printf_exec(": builtin/func exitcode %d\n", rcode);
			G.last_exitcode = rcode;
#ifndef __U_BOOT__
			check_and_run_traps();
#endif /* !__U_BOOT__ */
#যদি ENABLE_HUSH_TRAP && ENABLE_HUSH_FUNCTIONS
			rcode = G.last_exitcode; /* "return" in trap can change it, read back */
#endif
#ifndef __U_BOOT__
#যদি ENABLE_HUSH_LOOPS
			/* Was it "break" or "continue"? */
			যদি (G.flag_break_continue) {
				smallint fbc = G.flag_break_continue;
				/* We might fall into outer *loop*,
				 * don't want to break it too */
				যদি (loop_শীর্ষ) {
					G.depth_break_continue--;
					যদি (G.depth_break_continue == 0)
						G.flag_break_continue = 0;
					/* else: e.g. "continue 2" should *break* once, *then* continue */
				} /* else: "while... do... { we are here (innermost list is not a loop!) };...done" */
				যদি (G.depth_break_continue != 0 || fbc == BC_BREAK) {
					checkjobs(ফাঁকা, 0 /*(no pid to wait for)*/);
					বিরতি;
				}
				/* "continue": simulate end of loop */
				rword = RES_DONE;
				চালু;
			}
#endif
			যদি (G_flag_return_in_progress == 1) {
				checkjobs(ফাঁকা, 0 /*(no pid to wait for)*/);
				বিরতি;
			}

		} অন্যথা যদি (pi->followup == PIPE_BG) {
			/* What does bash do with attempts to background builtins? */
			/* even bash 3.2 doesn't do that well with nested bg:
			 * try "{ { sleep 10; echo DEEP; } & echo HERE; } &".
			 * I'm NOT treating inner &'s as jobs */
#যদি ENABLE_HUSH_JOB
			যদি (G.run_list_level == 1)
				ঢোকাও_job_into_table(pi);
#endif
			/* Last command's pid goes to $! */
			G.last_bg_pid = pi->cmds[pi->অঙ্ক_cmds - 1].pid;
			G.last_bg_pid_exitcode = 0;
			debug_printf_exec(": cmd&: exitcode EXIT_SUCCESS\n");
/* Check pi->pi_inverted? "! sleep 1 & echo $?": bash says 1. dash and ash say 0 */
			rcode = EXIT_SUCCESS;
			যাও check_traps;
		} অন্যথা {
#যদি ENABLE_HUSH_JOB
			যদি (G.run_list_level == 1 && G_interactive_fd) {
				/* Waits for completion, then fg's main shell */
				rcode = checkjobs_and_fg_shell(pi);
				debug_printf_exec(": checkjobs_and_fg_shell exitcode %d\n", rcode);
				যাও check_traps;
			}
#endif
			/* This one just waits for completion */
			rcode = checkjobs(pi, 0 /*(no pid to wait for)*/);
			debug_printf_exec(": checkjobs exitcode %d\n", rcode);
 check_traps:
			G.last_exitcode = rcode;
			check_and_run_traps();
#যদি ENABLE_HUSH_TRAP && ENABLE_HUSH_FUNCTIONS
			rcode = G.last_exitcode; /* "return" in trap can change it, read back */
#endif
		}
#endif /* !__U_BOOT__ */

#ifndef __U_BOOT__
		/* Handle "set -e" */
		যদি (rcode != 0 && G.o_opt[OPT_O_ERREXIT]) {
			debug_printf_exec("ERREXIT:1 errexit_depth:%d\n", G.errexit_depth);
			যদি (G.errexit_depth == 0)
				hush_exit(rcode);
		}
#অন্যথা /* __U_BOOT__ */
		} /* if (r != -1) */
#endif /* __U_BOOT__ */
		G.errexit_depth = sv_errexit_depth;

		/* Analyze how result affects subsequent commands */
#যদি ENABLE_HUSH_IF
		যদি (rword == RES_IF || rword == RES_ELIF) {
			debug_printf_exec("cond_code=rcode:%d\n", rcode);
			cond_code = rcode;
		}
#endif
#ifndef __U_BOOT__
 check_jobs_and_continue:
		checkjobs(ফাঁকা, 0 /*(no pid to wait for)*/);
#endif /* !__U_BOOT__ */
 dont_check_jobs_but_continue: ;
#যদি ENABLE_HUSH_LOOPS
		/* Beware of "while false; true; do ..."! */
		যদি (pi->পরবর্তী
		 && (pi->পরবর্তী->res_word == RES_DO || pi->পরবর্তী->res_word == RES_DONE)
		 /* check for RES_DONE is needed for "while ...; do \n done" case */
		) {
			যদি (rword == RES_WHILE) {
				যদি (rcode) {
					/* "while false; do...done" - exitcode 0 */
					G.last_exitcode = rcode = EXIT_SUCCESS;
					debug_printf_exec(": while expr is false: breaking (exitcode:EXIT_SUCCESS)\n");
					বিরতি;
				}
			}
			যদি (rword == RES_UNTIL) {
				যদি (!rcode) {
					debug_printf_exec(": until expr is true: breaking\n");
					বিরতি;
				}
			}
		}
#endif
	} /* for (pi) */

#যদি ENABLE_HUSH_JOB
	G.run_list_level--;
#endif
#যদি ENABLE_HUSH_LOOPS
	যদি (loop_শীর্ষ)
		G.depth_of_loop--;
	মুক্ত(for_list);
#endif
#যদি ENABLE_HUSH_CASE
	মুক্ত(case_word);
#endif
#ifndef __U_BOOT__
	debug_leave();
	debug_printf_exec("run_list lvl %d return %d\n", G.run_list_level, rcode);
#endif /* !__U_BOOT__ */
	ফেরত rcode;
}

/* Select which version we will use */
স্থির পূর্ণসংখ্যা run_and_free_list(কাঠামো pipe *pi)
{
	পূর্ণসংখ্যা rcode = 0;
	debug_printf_exec("run_and_free_list entered\n");
#ifndef __U_BOOT__
	যদি (!G.o_opt[OPT_O_NOEXEC]) {
#endif /* !__U_BOOT__ */
		debug_printf_exec(": run_list: 1st pipe with %d cmds\n", pi->অঙ্ক_cmds);
		rcode = run_list(pi);
#ifndef __U_BOOT__
	}
#endif /* !__U_BOOT__ */
	/* free_pipe_list has the side effect of clearing memory.
	 * In the long run that function can be merged with run_list,
	 * but doing that now would hobble the debugging effort. */
	free_pipe_list(pi);
	debug_printf_exec("run_and_free_list return %d\n", rcode);
	ফেরত rcode;
}

#ifndef __U_BOOT__
স্থির শূন্য install_sighandlers(unsigned mask)
{
	sighandler_t old_handler;
	unsigned sig = 0;
	যখন ((mask >>= 1) != 0) {
		sig++;
		যদি (!(mask & 1))
			চালু;
		old_handler = install_sighandler(sig, pick_sighandler(sig));
		/* POSIX allows shell to re-enable SIGCHLD
		 * even if it was SIG_IGN on entry.
		 * Therefore we skip IGN check for it:
		 */
		যদি (sig == SIGCHLD)
			চালু;
		/* Interactive bash re-enables SIGHUP which is SIG_IGNed on entry.
		 * Try:
		 * trap '' hup; bash; echo RET  # type "kill -hup $$", see SIGHUP having effect
		 * trap '' hup; bash -c 'kill -hup $$; echo ALIVE'  # here SIGHUP is SIG_IGNed
		 */
		যদি (sig == SIGHUP && G_interactive_fd)
			চালু;
		/* Unless one of the above signals, is it SIG_IGN? */
		যদি (old_handler == SIG_IGN) {
			/* oops... restore back to IGN, and record this fact */
			install_sighandler(sig, old_handler);
#যদি ENABLE_HUSH_TRAP
			যদি (!G_traps)
				G_traps = xzalloc(আকার(G_traps[0]) * NSIG);
			মুক্ত(G_traps[sig]);
			G_traps[sig] = xzalloc(1); /* == xstrdup(""); */
#endif
		}
	}
}

/* Called a few times only (or even once if "sh -c") */
স্থির শূন্য install_special_sighandlers(শূন্য)
{
	unsigned mask;

	/* Which signals are shell-special? */
	mask = (1 << SIGQUIT) | (1 << SIGCHLD);
	যদি (G_interactive_fd) {
		mask |= SPECIAL_INTERACTIVE_SIGS;
		যদি (G_saved_tty_pgrp) /* we have ctty, job control sigs work */
			mask |= SPECIAL_JOBSTOP_SIGS;
	}
	/* Careful, do not re-install handlers we already installed */
	যদি (G.special_sig_mask != mask) {
		unsigned diff = mask & ~G.special_sig_mask;
		G.special_sig_mask = mask;
		install_sighandlers(diff);
	}
}

#যদি ENABLE_HUSH_JOB
/* helper */
/* Set handlers to restore tty pgrp and exit */
স্থির শূন্য install_fatal_sighandlers(শূন্য)
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

স্থির পূর্ণসংখ্যা set_mode(পূর্ণসংখ্যা state, অক্ষর mode, ধ্রুবক অক্ষর *o_opt)
{
	পূর্ণসংখ্যা idx;
	নির্বাচন (mode) {
	ক্ষেত্র 'n':
		/* set -n has no effect in interactive shell */
		/* Try: while set -n; do echo $-; done */
		যদি (!G_interactive_fd)
			G.o_opt[OPT_O_NOEXEC] = state;
		বিরতি;
	ক্ষেত্র 'x':
		IF_HUSH_MODE_X(G_x_mode = state;)
		IF_HUSH_MODE_X(যদি (G.x_mode_fd <= 0) G.x_mode_fd = dup_CLOEXEC(2, 10);)
		বিরতি;
	ক্ষেত্র 'e':
		G.o_opt[OPT_O_ERREXIT] = state;
		বিরতি;
	ক্ষেত্র 'o':
		যদি (!o_opt) {
			/* "set -o" or "set +o" without parameter.
			 * in bash, set -o produces this output:
			 *  pipefail        off
			 * and set +o:
			 *  set +o pipefail
			 * We always use the second form.
			 */
			ধ্রুবক অক্ষর *p = o_opt_strings;
			idx = 0;
			যখন (*p) {
				printf("set %co %s\n", (G.o_opt[idx] ? '-' : '+'), p);
				idx++;
				p += strlen(p) + 1;
			}
			বিরতি;
		}
		idx = সূচক_in_strings(o_opt_strings, o_opt);
		যদি (idx >= 0) {
			G.o_opt[idx] = state;
			বিরতি;
		}
		/* fall through to error */
	পূর্বনির্ধারিত:
		ফেরত EXIT_FAILURE;
	}
	ফেরত EXIT_SUCCESS;
}

পূর্ণসংখ্যা hush_main(পূর্ণসংখ্যা argc, অক্ষর **argv) MAIN_EXTERNALLY_VISIBLE;
পূর্ণসংখ্যা hush_main(পূর্ণসংখ্যা argc, অক্ষর **argv)
{
	pid_t cached_getpid;
	গণনা {
		OPT_login = (1 << 0),
	};
	unsigned flags;
#যদি !BB_MMU
	unsigned builtin_argc = 0;
#endif
	অক্ষর **e;
	কাঠামো variable *cur_var;
	কাঠামো variable *shell_ver;

	INIT_G();
	যদি (EXIT_SUCCESS != 0) /* if EXIT_SUCCESS == 0, it is already done */
		G.last_exitcode = EXIT_SUCCESS;
#যদি !BB_MMU
	/* "Big heredoc" support via "sh -< STRING" invocation.
	 * Check it first (do not bother to run the usual init code,
	 * it is not needed for this case).
	 */
	যদি (argv[1]
	 && argv[1][0] == '-' && argv[1][1] == '<' /*&& !argv[1][2]*/
	 /*&& argv[2] && !argv[3] - we don't check some conditions */
	) {
		full_write1_str(argv[2]);
		_exit(0);
	}
	G.argv0_for_re_execing = argv[0];
#endif
#যদি ENABLE_HUSH_TRAP
# যদি ENABLE_HUSH_FUNCTIONS
	G.return_exitcode = -1;
# endif
	G.pre_trap_exitcode = -1;
#endif

#যদি ENABLE_HUSH_FAST
	G.গুনতি_SIGCHLD++; /* ensure it is != G.handled_SIGCHLD */
#endif

	cached_getpid = getpid();   /* for tcsetpgrp() during init */
	G.মূল_pid = cached_getpid; /* for $PID  (NOMMU can override via -$HEXPID:HEXPPID:...) */
	G.মূল_ppid = getppid();    /* for $PPID (NOMMU can override) */

	/* Deal with HUSH_VERSION */
	debug_printf_env("unsetenv '%s'\n", "HUSH_VERSION");
	unsetenv("HUSH_VERSION"); /* in case it exists in initial env */
	shell_ver = xzalloc(আকার(*shell_ver));
	shell_ver->flg_export = 1;
	shell_ver->flg_read_only = 1;
	/* Code which handles ${var<op>...} needs writable values for all variables,
	 * therefore we xstrdup: */
	shell_ver->varstr = xstrdup(hush_version_str);

	/* Create shell local variables from the values
	 * currently living in the environment */
	G.শীর্ষ_var = shell_ver;
	cur_var = G.শীর্ষ_var;
	e = environ;
	যদি (e) যখন (*e) {
		অক্ষর *মান = strchr(*e, '=');
		যদি (মান) { /* paranoia */
			cur_var->পরবর্তী = xzalloc(আকার(*cur_var));
			cur_var = cur_var->পরবর্তী;
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

#যদি BASH_HOSTNAME_VAR
	/* Set (but not export) HOSTNAME unless already set */
	যদি (!get_local_var_মান("HOSTNAME")) {
		কাঠামো utsname uts;
		uname(&uts);
		set_local_var_from_halves("HOSTNAME", uts.nodename);
	}
#endif
	/* IFS is not inherited from the parent environment */
	set_local_var_from_halves("IFS", defifs);

	যদি (!get_local_var_মান("PATH"))
		set_local_var_from_halves("PATH", bb_default_মূল_path);

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

#যদি অঙ্ক_SCRIPTS > 0
	যদি (argc < 0) {
		অক্ষর *script = get_script_content(-argc - 1);
		G.global_argv = argv;
		G.global_argc = string_array_len(argv);
		//install_special_sighandlers(); - needed?
		parse_and_run_string(script);
		যাও final_return;
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
	যখন (1) {
		পূর্ণসংখ্যা opt = getopt(argc, argv, "+" /* stop at 1st non-option */
				"cexinsl"
#যদি !BB_MMU
				"$:R:V:"
# যদি ENABLE_HUSH_LINENO_VAR
				"L:"
# endif
# যদি ENABLE_HUSH_FUNCTIONS
				"F:"
# endif
#endif
		);
		যদি (opt <= 0)
			বিরতি;
		নির্বাচন (opt) {
		ক্ষেত্র 'c':
			/* Note: -c is not an option with param!
			 * "hush -c -l SCRIPT" is valid. "hush -cSCRIPT" is not.
			 */
			G.opt_c = 1;
			বিরতি;
		ক্ষেত্র 'i':
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
			বিরতি;
		ক্ষেত্র 's':
			G.opt_s = 1;
			বিরতি;
		ক্ষেত্র 'l':
			flags |= OPT_login;
			বিরতি;
#যদি !BB_MMU
		ক্ষেত্র '$': {
			unsigned long long empty_trap_mask;

			G.মূল_pid = bb_strtou(optarg, &optarg, 16);
			optarg++;
			G.মূল_ppid = bb_strtou(optarg, &optarg, 16);
			optarg++;
			G.last_bg_pid = bb_strtou(optarg, &optarg, 16);
			optarg++;
			G.last_exitcode = bb_strtou(optarg, &optarg, 16);
			optarg++;
			builtin_argc = bb_strtou(optarg, &optarg, 16);
			optarg++;
			empty_trap_mask = bb_strtoull(optarg, &optarg, 16);
			যদি (empty_trap_mask != 0) {
				IF_HUSH_TRAP(পূর্ণসংখ্যা sig;)
				install_special_sighandlers();
# যদি ENABLE_HUSH_TRAP
				G_traps = xzalloc(আকার(G_traps[0]) * NSIG);
				জন্য (sig = 1; sig < NSIG; sig++) {
					যদি (empty_trap_mask & (1LL << sig)) {
						G_traps[sig] = xzalloc(1); /* == xstrdup(""); */
						install_sighandler(sig, SIG_IGN);
					}
				}
# endif
			}
# যদি ENABLE_HUSH_LOOPS
			optarg++;
			G.depth_of_loop = bb_strtou(optarg, &optarg, 16);
# endif
			/* Suppress "killed by signal" message, -$ hack is used
			 * for subshells: echo `sh -c 'kill -9 $$'`
			 * should be silent.
			 */
			IF_HUSH_JOB(G.run_list_level = 1;)
# যদি ENABLE_HUSH_FUNCTIONS
			/* nommu uses re-exec trick for "... | func | ...",
			 * should allow "return".
			 * This accidentally allows returns in subshells.
			 */
			G_flag_return_in_progress = -1;
# endif
			বিরতি;
		}
		ক্ষেত্র 'R':
		ক্ষেত্র 'V':
			set_local_var(xstrdup(optarg), opt == 'R' ? SETFLAG_MAKE_RO : 0);
			বিরতি;
# যদি ENABLE_HUSH_LINENO_VAR
		ক্ষেত্র 'L':
			G.parse_lineno = xatou(optarg);
			বিরতি;
# endif
# যদি ENABLE_HUSH_FUNCTIONS
		ক্ষেত্র 'F': {
			কাঠামো function *funcp = new_function(optarg);
			/* funcp->name is already set to optarg */
			/* funcp->body is set to NULL. It's a special case. */
			funcp->body_as_string = argv[optind];
			optind++;
			বিরতি;
		}
# endif
#endif
		/*case '?': invalid option encountered (set_mode('?') will fail) */
		/*case 'n':*/
		/*case 'x':*/
		/*case 'e':*/
		পূর্বনির্ধারিত:
			যদি (set_mode(1, opt, ফাঁকা) == 0) /* no error */
				বিরতি;
			bb_show_usage();
		}
	} /* option parsing loop */

	/* Skip options. Try "hush -l": $1 should not be "-l"! */
	G.global_argc = argc - (optind - 1);
	G.global_argv = argv + (optind - 1);
	G.global_argv[0] = argv[0];

	/* If we are login shell... */
	যদি (flags & OPT_login) {
		ধ্রুবক অক্ষর *hp = ফাঁকা;
		HFILE *প্রবেশ;

		debug_printf("sourcing /etc/profile\n");
		প্রবেশ = hfopen("/etc/profile");
 run_profile:
		যদি (প্রবেশ != ফাঁকা) {
			install_special_sighandlers();
			parse_and_run_file(প্রবেশ);
			hfclose(প্রবেশ);
		}
		/* bash: after sourcing /etc/profile,
		 * tries to source (in the given order):
		 * ~/.bash_profile, ~/.bash_login, ~/.profile,
		 * stopping on first found. --noprofile turns this off.
		 * bash also sources ~/.bash_logout on exit.
		 * If called as sh, skips .bash_XXX files.
		 */
		যদি (!hp) { /* unless we looped on the "goto" already */
			hp = get_local_var_মান("HOME");
			যদি (hp && hp[0]) {
				debug_printf("sourcing ~/.profile\n");
				hp = concat_path_file(hp, ".profile");
				প্রবেশ = hfopen(hp);
				মুক্ত((অক্ষর*)hp);
				যাও run_profile;
			}
		}
	}

#ifndef __U_BOOT__
	/* -c takes effect *after* -l */
	যদি (G.opt_c) {
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
		অক্ষর *script;

		install_special_sighandlers();

		G.global_argc--;
		G.global_argv++;
#যদি !BB_MMU
		যদি (builtin_argc) {
			/* -c 'builtin' [BARGV...] "" ARG0 [ARG1...] */
			ধ্রুবক কাঠামো built_in_command *x;
			x = find_builtin(G.global_argv[0]);
			যদি (x) { /* paranoia */
				argv = G.global_argv;
				G.global_argc -= builtin_argc + 1; /* skip [BARGV...] "" */
				G.global_argv += builtin_argc + 1;
				G.global_argv[-1] = ফাঁকা; /* replace "" */
				G.last_exitcode = x->b_function(argv);
			}
			যাও final_return;
		}
#endif

		script = G.global_argv[0];
		যদি (!script)
			bb_error_বার্তা_and_die(bb_বার্তা_requires_arg, "-c");
		যদি (!G.global_argv[1]) {
			/* -c 'script' (no params): prevent empty $0 */
			G.global_argv[0] = argv[0];
		} অন্যথা { /* else -c 'script' ARG0 [ARG1...]: $0 is ARG0 */
			G.global_argc--;
			G.global_argv++;
		}
		parse_and_run_string(script);
		যাও final_return;
	}

	/* -s is: hush -s ARGV1 ARGV2 (no SCRIPT) */
	যদি (!G.opt_s && G.global_argv[1]) {
		HFILE *প্রবেশ;
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
		প্রবেশ = hfopen(G.global_argv[0]);
		যদি (!প্রবেশ) {
			bb_simple_perror_বার্তা_and_die(G.global_argv[0]);
		}
		xfunc_error_retval = 1;
		install_special_sighandlers();
		parse_and_run_file(প্রবেশ);
#যদি ENABLE_FEATURE_CLEAN_UP
		hfclose(প্রবেশ);
#endif
		যাও final_return;
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
#যদি ENABLE_HUSH_JOB
	যদি (isatty(STDIN_FILENO) && isatty(STDOUT_FILENO)) {
		G_saved_tty_pgrp = tcgetpgrp(STDIN_FILENO);
		debug_printf("saved_tty_pgrp:%d\n", G_saved_tty_pgrp);
		যদি (G_saved_tty_pgrp < 0)
			G_saved_tty_pgrp = 0;

		/* try to dup stdin to high fd#, >= 255 */
		G_interactive_fd = dup_CLOEXEC(STDIN_FILENO, 254);
		যদি (G_interactive_fd < 0) {
			/* try to dup to any fd */
			G_interactive_fd = dup_CLOEXEC(STDIN_FILENO, -1);
			যদি (G_interactive_fd < 0) {
				/* give up */
				G_interactive_fd = 0;
				G_saved_tty_pgrp = 0;
			}
		}
	}
	debug_printf("interactive_fd:%d\n", G_interactive_fd);
	যদি (G_interactive_fd) {
		যদি (G_saved_tty_pgrp) {
			/* If we were run as 'hush &', sleep until we are
			 * in the foreground (tty pgrp == our pgrp).
			 * If we get started under a job aware app (like bash),
			 * make sure we are now in charge so we don't fight over
			 * who gets the foreground */
			যখন (1) {
				pid_t shell_pgrp = getpgrp();
				G_saved_tty_pgrp = tcgetpgrp(G_interactive_fd);
				যদি (G_saved_tty_pgrp == shell_pgrp)
					বিরতি;
				/* send TTIN to ourself (should stop us) */
				kill(- shell_pgrp, SIGTTIN);
			}
		}

		/* Install more signal handlers */
		install_special_sighandlers();

		যদি (G_saved_tty_pgrp) {
			/* Set other signals to restore saved_tty_pgrp */
			install_fatal_sighandlers();
			/* Put ourselves in our own process group
			 * (bash, too, does this only if ctty is available) */
			bb_setpgrp(); /* is the same as setpgid(our_pid, our_pid); */
			/* Grab control of the terminal */
			tcsetpgrp(G_interactive_fd, cached_getpid);
		}
		enable_restore_tty_pgrp_on_exit();

# যদি ENABLE_FEATURE_EDITING
		G.সারি_প্রবেশ_state = new_সারি_প্রবেশ_t(FOR_SHELL);
#  যদি ENABLE_FEATURE_TAB_COMPLETION
		G.সারি_প্রবেশ_state->get_exe_name = hush_command_name;
#  endif
#  যদি EDITING_HAS_sh_get_var
		G.সারি_প্রবেশ_state->sh_get_var = get_local_var_মান;
#  endif
# endif
# যদি ENABLE_HUSH_SAVEHISTORY && MAX_HISTORY > 0
		{
			ধ্রুবক অক্ষর *hp = get_local_var_মান("HISTFILE");
			যদি (!hp) {
				hp = get_local_var_মান("HOME");
				যদি (hp)
					hp = concat_path_file(hp, ".hush_history");
			} অন্যথা {
				hp = xstrdup(hp);
			}
			যদি (hp) {
				G.সারি_প্রবেশ_state->hist_file = hp;
				//set_local_var(xasprintf("HISTFILE=%s", ...));
			}
#  যদি ENABLE_FEATURE_SH_HISTFILESIZE
			hp = get_local_var_মান("HISTFILESIZE");
			G.সারি_প্রবেশ_state->max_history = আকার_from_HISTFILESIZE(hp);
#  endif
		}
# endif
	} অন্যথা {
		install_special_sighandlers();
	}
#নতুবা_যদি ENABLE_HUSH_INTERACTIVE
	/* No job control compiled in, only prompt/line editing */
	যদি (isatty(STDIN_FILENO) && isatty(STDOUT_FILENO)) {
		G_interactive_fd = dup_CLOEXEC(STDIN_FILENO, 254);
		যদি (G_interactive_fd < 0) {
			/* try to dup to any fd */
			G_interactive_fd = dup_CLOEXEC(STDIN_FILENO, -1);
			যদি (G_interactive_fd < 0)
				/* give up */
				G_interactive_fd = 0;
		}
	}
	install_special_sighandlers();
#অন্যথা
	/* We have interactiveness code disabled */
	install_special_sighandlers();
#endif
	/* bash:
	 * if interactive but not a login shell, sources ~/.bashrc
	 * (--norc turns this off, --rcfile <file> overrides)
	 */

	যদি (G_interactive_fd) {
#যদি ENABLE_HUSH_INTERACTIVE && ENABLE_FEATURE_EDITING_FANCY_PROMPT
		/* Set (but not export) PS1/2 unless already set */
		যদি (!get_local_var_মান("PS1"))
			set_local_var_from_halves("PS1", "\\w \\$ ");
		যদি (!get_local_var_মান("PS2"))
			set_local_var_from_halves("PS2", "> ");
#endif
		যদি (!ENABLE_FEATURE_SH_EXTRA_QUIET) {
			/* note: ash and hush share this string */
			printf("\n\n%s %s\n"
				IF_HUSH_HELP("Enter 'help' for a list of built-in commands.\n")
				"\n",
				bb_banner,
				"hush - the humble shell"
			);
		}
	}

	parse_and_run_file(hfopen(ফাঁকা)); /* stdin */

 final_return:
	hush_exit(G.last_exitcode);
}

/*
 * Built-ins
 */
স্থির পূর্ণসংখ্যা FAST_FUNC builtin_true(অক্ষর **argv UNUSED_PARAM)
{
	ফেরত 0;
}

স্থির পূর্ণসংখ্যা FAST_FUNC builtin_false(অক্ষর **argv UNUSED_PARAM)
{
	ফেরত 1;
}

#যদি ENABLE_HUSH_TEST || ENABLE_HUSH_ECHO || ENABLE_HUSH_PRINTF || ENABLE_HUSH_KILL
স্থির NOINLINE পূর্ণসংখ্যা run_applet_main(অক্ষর **argv, পূর্ণসংখ্যা (*applet_main_func)(পূর্ণসংখ্যা argc, অক্ষর **argv))
{
	পূর্ণসংখ্যা argc = string_array_len(argv);
	ফেরত applet_main_func(argc, argv);
}
#endif
#যদি ENABLE_HUSH_TEST || BASH_TEST2
স্থির পূর্ণসংখ্যা FAST_FUNC builtin_test(অক্ষর **argv)
{
	ফেরত run_applet_main(argv, test_main);
}
#endif
#যদি ENABLE_HUSH_ECHO
স্থির পূর্ণসংখ্যা FAST_FUNC builtin_echo(অক্ষর **argv)
{
	ফেরত run_applet_main(argv, echo_main);
}
#endif
#যদি ENABLE_HUSH_PRINTF
স্থির পূর্ণসংখ্যা FAST_FUNC builtin_printf(অক্ষর **argv)
{
	ফেরত run_applet_main(argv, printf_main);
}
#endif

#যদি ENABLE_HUSH_HELP
স্থির পূর্ণসংখ্যা FAST_FUNC builtin_help(অক্ষর **argv UNUSED_PARAM)
{
	ধ্রুবক কাঠামো built_in_command *x;

	printf(
		"Built-in commands:\n"
		"------------------\n");
	জন্য (x = bltins1; x != &bltins1[ARRAY_আকার(bltins1)]; x++) {
		যদি (x->b_descr)
			printf("%-10s%s\n", x->b_cmd, x->b_descr);
	}
	ফেরত EXIT_SUCCESS;
}
#endif

#যদি MAX_HISTORY && ENABLE_FEATURE_EDITING
স্থির পূর্ণসংখ্যা FAST_FUNC builtin_history(অক্ষর **argv UNUSED_PARAM)
{
	show_history(G.সারি_প্রবেশ_state);
	ফেরত EXIT_SUCCESS;
}
#endif

স্থির পূর্ণসংখ্যা FAST_FUNC builtin_cd(অক্ষর **argv)
{
	ধ্রুবক অক্ষর *newdir;

	argv = skip_dash_dash(argv);
	newdir = argv[0];
	যদি (newdir == ফাঁকা) {
		/* bash does nothing (exitcode 0) if HOME is ""; if it's unset,
		 * bash says "bash: cd: HOME not set" and does nothing
		 * (exitcode 1)
		 */
		ধ্রুবক অক্ষর *home = get_local_var_মান("HOME");
		newdir = home ? home : "/";
	}
	যদি (chdir(newdir)) {
		/* Mimic bash message exactly */
		bb_perror_বার্তা("cd: %s", newdir);
		ফেরত EXIT_FAILURE;
	}
	/* Read current dir (get_cwd(1) is inside) and set PWD.
	 * Note: do not enforce exporting. If PWD was unset or unexported,
	 * set it again, but do not export. bash does the same.
	 */
	set_pwd_var(/*flag:*/ 0);
	ফেরত EXIT_SUCCESS;
}

স্থির পূর্ণসংখ্যা FAST_FUNC builtin_pwd(অক্ষর **argv UNUSED_PARAM)
{
	puts(get_cwd(0));
	ফেরত EXIT_SUCCESS;
}

স্থির পূর্ণসংখ্যা FAST_FUNC builtin_eval(অক্ষর **argv)
{
	argv = skip_dash_dash(argv);

	যদি (!argv[0])
		ফেরত EXIT_SUCCESS;

	IF_HUSH_MODE_X(G.x_mode_depth++;)
	//bb_error_msg("%s: ++x_mode_depth=%d", __func__, G.x_mode_depth);
	যদি (!argv[1]) {
		/* bash:
		 * eval "echo Hi; done" ("done" is syntax error):
		 * "echo Hi" will not execute too.
		 */
		parse_and_run_string(argv[0]);
	} অন্যথা {
		/* "The eval utility shall construct a command by
		 * concatenating arguments together, separating
		 * each with a <space> character."
		 */
		অক্ষর *str, *p;
		unsigned len = 0;
		অক্ষর **pp = argv;
		করো
			len += strlen(*pp) + 1;
		যখন (*++pp);
		str = p = xmalloc(len);
		pp = argv;
		জন্য (;;) {
			p = stpcpy(p, *pp);
			pp++;
			যদি (!*pp)
				বিরতি;
			*p++ = ' ';
		}
		parse_and_run_string(str);
		মুক্ত(str);
	}
	IF_HUSH_MODE_X(G.x_mode_depth--;)
	//bb_error_msg("%s: --x_mode_depth=%d", __func__, G.x_mode_depth);
	ফেরত G.last_exitcode;
}

স্থির পূর্ণসংখ্যা FAST_FUNC builtin_exec(অক্ষর **argv)
{
	argv = skip_dash_dash(argv);
	যদি (argv[0] == ফাঁকা)
		ফেরত EXIT_SUCCESS; /* bash does this */

	/* Careful: we can end up here after [v]fork. Do not restore
	 * tty pgrp then, only top-level shell process does that */
	যদি (G_saved_tty_pgrp && getpid() == G.মূল_pid)
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

স্থির পূর্ণসংখ্যা FAST_FUNC builtin_exit(অক্ষর **argv)
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
	যদি (argv[0] == ফাঁকা) {
#যদি ENABLE_HUSH_TRAP
		যদি (G.pre_trap_exitcode >= 0) /* "exit" in trap uses $? from before the trap */
			hush_exit(G.pre_trap_exitcode);
#endif
		hush_exit(G.last_exitcode);
	}
	/* mimic bash: exit 123abc == exit 255 + error msg */
	xfunc_error_retval = 255;
	/* bash: exit -2 == exit 254, no error msg */
	hush_exit(xatoi(argv[0]) & 0xff);
}

#যদি ENABLE_HUSH_TYPE
/* http://www.opengroup.org/onlinepubs/9699919799/utilities/type.html */
স্থির পূর্ণসংখ্যা FAST_FUNC builtin_type(অক্ষর **argv)
{
	পূর্ণসংখ্যা ret = EXIT_SUCCESS;

	যখন (*++argv) {
		ধ্রুবক অক্ষর *type;
		অক্ষর *path = ফাঁকা;

		যদি (0) {} /* make conditional compile easier below */
		/*else if (find_alias(*argv))
			type = "an alias";*/
# যদি ENABLE_HUSH_FUNCTIONS
		অন্যথা যদি (find_function(*argv))
			type = "a function";
# endif
		অন্যথা যদি (find_builtin(*argv))
			type = "a shell builtin";
		অন্যথা যদি ((path = find_in_path(*argv)) != ফাঁকা)
			type = path;
		অন্যথা {
			bb_error_বার্তা("type: %s: not found", *argv);
			ret = EXIT_FAILURE;
			চালু;
		}

		printf("%s is %s\n", *argv, type);
		মুক্ত(path);
	}

	ফেরত ret;
}
#endif

#যদি ENABLE_HUSH_READ
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
স্থির পূর্ণসংখ্যা FAST_FUNC builtin_read(অক্ষর **argv)
{
	ধ্রুবক অক্ষর *r;
	কাঠামো builtin_read_params params;

	memset(&params, 0, আকার(params));

	/* "!": do not abort on errors.
	 * Option string must start with "sr" to match BUILTIN_READ_xxx
	 */
	params.read_flags = getopt32(argv,
# যদি BASH_READ_D
		IF_NOT_HUSH_BASH_COMPAT("^")
		"!srn:p:t:u:d:" IF_NOT_HUSH_BASH_COMPAT("\0" "-1"/*min 1 arg*/),
		&params.opt_n, &params.opt_p, &params.opt_t, &params.opt_u, &params.opt_d
# অন্যথা
		IF_NOT_HUSH_BASH_COMPAT("^")
		"!srn:p:t:u:" IF_NOT_HUSH_BASH_COMPAT("\0" "-1"/*min 1 arg*/),
		&params.opt_n, &params.opt_p, &params.opt_t, &params.opt_u
# endif
//TODO: print "read: need variable name"
//for the case of !BASH "read" with no args (now it fails silently)
//(or maybe extend getopt32() to emit a message if "-1" fails)
	);
	যদি ((uint32_t)params.read_flags == (uint32_t)-1)
		ফেরত EXIT_FAILURE;
	argv += optind;
	params.argv = argv;
	params.setvar = set_local_var_from_halves;
	params.ifs = get_local_var_মান("IFS"); /* can be NULL */

 again:
	r = shell_builtin_read(&params);

	যদি ((uintptr_t)r == 1 && errno == EINTR) {
		unsigned sig = check_and_run_traps();
		যদি (sig != SIGINT)
			যাও again;
	}

	যদি ((uintptr_t)r > 1) {
		bb_simple_error_বার্তা(r);
		r = (অক্ষর*)(uintptr_t)1;
	}

	ফেরত (uintptr_t)r;
}
#endif

#যদি ENABLE_HUSH_UMASK
স্থির পূর্ণসংখ্যা FAST_FUNC builtin_umask(অক্ষর **argv)
{
	পূর্ণসংখ্যা rc;
	mode_t mask;

	rc = 1;
	mask = umask(0);
	argv = skip_dash_dash(argv);
	যদি (argv[0]) {
		mode_t old_mask = mask;

		/* numeric umasks are taken as-is */
		/* symbolic umasks are inverted: "umask a=rx" calls umask(222) */
		যদি (!isdigit(argv[0][0]))
			mask ^= 0777;
		mask = bb_parse_mode(argv[0], mask);
		যদি (!isdigit(argv[0][0]))
			mask ^= 0777;
		যদি ((unsigned)mask > 0777) {
			mask = old_mask;
			/* bash messages:
			 * bash: umask: 'q': invalid symbolic mode operator
			 * bash: umask: 999: octal number out of range
			 */
			bb_error_বার্তা("%s: invalid mode '%s'", "umask", argv[0]);
			rc = 0;
		}
	} অন্যথা {
		/* Mimic bash */
		printf("%04o\n", (unsigned) mask);
		/* fall through and restore mask which we set to 0 */
	}
	umask(mask);

	ফেরত !rc; /* rc != 0 - success */
}
#endif

#যদি ENABLE_HUSH_EXPORT || ENABLE_HUSH_READONLY || ENABLE_HUSH_SET || ENABLE_HUSH_TRAP
স্থির শূন্য print_escaped(ধ্রুবক অক্ষর *s)
{
//TODO? bash "set" does not quote variables which contain only alnums and "%+,-./:=@_~",
// (but "export" quotes all variables, even with only these chars).
// I think quoting strings with %+,=~ looks better
// (example: "set" printing var== instead of var='=' looks strange)
// IOW: do not quote "-./:@_": / is used in pathnames, : in PATH, -._ often in file names, @ in emails

	যদি (*s == '\'')
		যাও squote;
	করো {
		ধ্রুবক অক্ষর *p = strchrnul(s, '\'');
		/* print 'xxxx', possibly just '' */
		printf("'%.*s'", (পূর্ণসংখ্যা)(p - s), s);
		যদি (*p == '\0')
			বিরতি;
		s = p;
 squote:
		/* s points to '; print "'''...'''" */
		putchar('"');
		করো putchar('\''); যখন (*++s == '\'');
		putchar('"');
	} যখন (*s);
}
#endif

#যদি ENABLE_HUSH_EXPORT || ENABLE_HUSH_LOCAL || ENABLE_HUSH_READONLY
স্থির পূর্ণসংখ্যা helper_export_local(অক্ষর **argv, unsigned flags)
{
	করো {
		অক্ষর *name = *argv;
		ধ্রুবক অক্ষর *name_end = endofname(name);

		যদি (*name_end == '\0') {
			কাঠামো variable *var, **vpp;

			vpp = get_ptr_to_local_var(name);
			var = vpp ? *vpp : ফাঁকা;

			যদি (flags & SETFLAG_UNEXPORT) {
				/* export -n NAME (without =VALUE) */
				যদি (var) {
					var->flg_export = 0;
					debug_printf_env("%s: unsetenv '%s'\n", __func__, name);
					unsetenv(name);
				} /* else: export -n NOT_EXISTING_VAR: no-op */
				চালু;
			}
			যদি (flags & SETFLAG_EXPORT) {
				/* export NAME (without =VALUE) */
				যদি (var) {
					var->flg_export = 1;
					debug_printf_env("%s: putenv '%s'\n", __func__, var->varstr);
					putenv(var->varstr);
					চালু;
				}
			}
			যদি (flags & SETFLAG_MAKE_RO) {
				/* readonly NAME (without =VALUE) */
				যদি (var) {
					var->flg_read_only = 1;
					চালু;
				}
			}
# যদি ENABLE_HUSH_LOCAL
			/* Is this "local" bltin? */
			যদি (!(flags & (SETFLAG_EXPORT|SETFLAG_UNEXPORT|SETFLAG_MAKE_RO))) {
				unsigned lvl = flags >> SETFLAG_VARLVL_SHIFT;
				যদি (var && var->var_nest_level == lvl) {
					/* "local x=abc; ...; local x" - ignore second local decl */
					চালু;
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
		} অন্যথা {
			যদি (*name_end != '=') {
				bb_error_বার্তা("'%s': bad variable name", name);
				/* do not parse following argv[]s: */
				ফেরত 1;
			}
			/* (Un)exporting/making local NAME=VALUE */
			name = xstrdup(name);
			/* Testcase: export PS1='\w \$ ' */
			unbackslash(name);
		}
		debug_printf_env("%s: set_local_var('%s')\n", __func__, name);
		যদি (set_local_var(name, flags))
			ফেরত EXIT_FAILURE;
	} যখন (*++argv);
	ফেরত EXIT_SUCCESS;
}
#endif

#যদি ENABLE_HUSH_EXPORT
স্থির পূর্ণসংখ্যা FAST_FUNC builtin_export(অক্ষর **argv)
{
	unsigned opt_unexport;

# যদি ENABLE_HUSH_EXPORT_N
	/* "!": do not abort on errors */
	opt_unexport = getopt32(argv, "!n");
	যদি (opt_unexport == (uint32_t)-1)
		ফেরত EXIT_FAILURE;
	argv += optind;
# অন্যথা
	opt_unexport = 0;
	argv++;
# endif

	যদি (argv[0] == ফাঁকা) {
		অক্ষর **e = environ;
		যদি (e) {
			যখন (*e) {
# যদি 0
				puts(*e++);
# অন্যথা
				/* ash emits: export VAR='VAL'
				 * bash: declare -x VAR="VAL"
				 * we follow ash example */
				ধ্রুবক অক্ষর *s = *e++;
				ধ্রুবক অক্ষর *p = strchr(s, '=');

				যদি (!p) /* wtf? take next variable */
					চালু;
				/* "export VAR=" */
				printf("%s %.*s", "export", (পূর্ণসংখ্যা)(p - s) + 1, s);
				print_escaped(p + 1);
				putchar('\n');
# endif
			}
			/*fflush_all(); - done after each builtin anyway */
		}
		ফেরত EXIT_SUCCESS;
	}

	ফেরত helper_export_local(argv, opt_unexport ? SETFLAG_UNEXPORT : SETFLAG_EXPORT);
}
#endif

#যদি ENABLE_HUSH_LOCAL
স্থির পূর্ণসংখ্যা FAST_FUNC builtin_local(অক্ষর **argv)
{
	যদি (G.func_nest_level == 0) {
		bb_error_বার্তা("%s: not in a function", argv[0]);
		ফেরত EXIT_FAILURE; /* bash compat */
	}
//TODO? ash and bash support "local -" special form,
//which saves/restores $- around function call (including async returns, such as ^C)
//(IOW: it makes "set +/-..." effects local)
	argv++;
	/* Since all builtins run in a nested variable level,
	 * need to use level - 1 here. Or else the variable will be removed at once
	 * after builtin returns.
	 */
	ফেরত helper_export_local(argv, (G.var_nest_level - 1) << SETFLAG_VARLVL_SHIFT);
}
#endif

#যদি ENABLE_HUSH_READONLY
স্থির পূর্ণসংখ্যা FAST_FUNC builtin_readonly(অক্ষর **argv)
{
	argv++;
	যদি (*argv == ফাঁকা) {
		/* bash: readonly [-p]: list all readonly VARs
		 * (-p has no effect in bash)
		 */
		কাঠামো variable *e;
		জন্য (e = G.শীর্ষ_var; e; e = e->পরবর্তী) {
			যদি (e->flg_read_only) {
				ধ্রুবক অক্ষর *s = e->varstr;
				ধ্রুবক অক্ষর *p = strchr(s, '=');

				যদি (!p) /* wtf? take next variable */
					চালু;
				/* "readonly VAR=" */
				printf("%s %.*s", "readonly", (পূর্ণসংখ্যা)(p - s) + 1, s);
				print_escaped(p + 1);
				putchar('\n');
			}
		}
		ফেরত EXIT_SUCCESS;
	}
	ফেরত helper_export_local(argv, SETFLAG_MAKE_RO);
}
#endif

#যদি ENABLE_HUSH_UNSET
/* http://www.opengroup.org/onlinepubs/9699919799/utilities/V3_chap02.html#unset */
স্থির পূর্ণসংখ্যা FAST_FUNC builtin_unset(অক্ষর **argv)
{
	পূর্ণসংখ্যা ret;
	unsigned opts;

	/* "!": do not abort on errors */
	/* "+": stop at 1st non-option */
	opts = getopt32(argv, "!+vf");
	যদি (opts == (unsigned)-1)
		ফেরত EXIT_FAILURE;
	যদি (opts == 3) {
		bb_simple_error_বার্তা("unset: -v and -f are exclusive");
		ফেরত EXIT_FAILURE;
	}
	argv += optind;

	ret = EXIT_SUCCESS;
	যখন (*argv) {
		যদি (!(opts & 2)) { /* not -f */
			যদি (unset_local_var(*argv)) {
				/* unset <nonexistent_var> doesn't fail.
				 * Error is when one tries to unset RO var.
				 * Message was printed by unset_local_var. */
				ret = EXIT_FAILURE;
			}
		}
# যদি ENABLE_HUSH_FUNCTIONS
		অন্যথা {
			unset_func(*argv);
		}
# endif
		argv++;
	}
	ফেরত ret;
}
#endif

#যদি ENABLE_HUSH_SET
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
স্থির পূর্ণসংখ্যা FAST_FUNC builtin_set(অক্ষর **argv)
{
	পূর্ণসংখ্যা n;
	অক্ষর **pp, **g_argv;
	অক্ষর *arg = *++argv;

	যদি (arg == ফাঁকা) {
		কাঠামো variable *e;
		জন্য (e = G.শীর্ষ_var; e; e = e->পরবর্তী) {
			ধ্রুবক অক্ষর *s = e->varstr;
			ধ্রুবক অক্ষর *p = strchr(s, '=');

			যদি (!p) /* wtf? take next variable */
				চালু;
			/* var= */
			printf("%.*s", (পূর্ণসংখ্যা)(p - s) + 1, s);
			print_escaped(p + 1);
			putchar('\n');
		}
		ফেরত EXIT_SUCCESS;
	}

	করো {
		যদি (strcmp(arg, "--") == 0) {
			++argv;
			যাও set_argv;
		}
		যদি (arg[0] != '+' && arg[0] != '-')
			বিরতি;
		জন্য (n = 1; arg[n]; ++n) {
			যদি (set_mode((arg[0] == '-'), arg[n], argv[1])) {
				bb_error_বার্তা("%s: %s: invalid option", "set", arg);
				ফেরত EXIT_FAILURE;
			}
			যদি (arg[n] == 'o' && argv[1])
				argv++;
		}
	} যখন ((arg = *++argv) != ফাঁকা);
	/* Now argv[0] is 1st argument */

	যদি (arg == ফাঁকা)
		ফেরত EXIT_SUCCESS;
 set_argv:

	/* NB: G.global_argv[0] ($0) is never freed/changed */
	g_argv = G.global_argv;
	যদি (G.global_args_malloced) {
		pp = g_argv;
		যখন (*++pp)
			মুক্ত(*pp);
		g_argv[1] = ফাঁকা;
	} অন্যথা {
		G.global_args_malloced = 1;
		pp = xzalloc(আকার(pp[0]) * 2);
		pp[0] = g_argv[0]; /* retain $0 */
		g_argv = pp;
	}
	/* This realloc's G.global_argv */
	G.global_argv = pp = add_strings_to_strings(g_argv, argv, /*dup:*/ 1);

	G.global_argc = 1 + string_array_len(pp + 1);

	ফেরত EXIT_SUCCESS;
}
#endif

স্থির পূর্ণসংখ্যা FAST_FUNC builtin_shift(অক্ষর **argv)
{
	পূর্ণসংখ্যা n = 1;
	argv = skip_dash_dash(argv);
	যদি (argv[0]) {
		n = bb_strtou(argv[0], ফাঁকা, 10);
		যদি (errno || n < 0) {
			/* shared string with ash.c */
			bb_error_বার্তা("Illegal number: %s", argv[0]);
			/*
			 * ash aborts in this case.
			 * bash prints error message and set $? to 1.
			 * Interestingly, for "shift 99999" bash does not
			 * print error message, but does set $? to 1
			 * (and does no shifting at all).
			 */
		}
	}
	যদি (n >= 0 && n < G.global_argc) {
		যদি (G_global_args_malloced) {
			পূর্ণসংখ্যা m = 1;
			যখন (m <= n)
				মুক্ত(G.global_argv[m++]);
		}
		G.global_argc -= n;
		memmove(&G.global_argv[1], &G.global_argv[n+1],
				G.global_argc * আকার(G.global_argv[0]));
		ফেরত EXIT_SUCCESS;
	}
	ফেরত EXIT_FAILURE;
}

#যদি ENABLE_HUSH_GETOPTS
স্থির পূর্ণসংখ্যা FAST_FUNC builtin_getopts(অক্ষর **argv)
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
	অক্ষর cbuf[2];
	ধ্রুবক অক্ষর *cp, *optstring, *var;
	পূর্ণসংখ্যা c, n, exitcode, my_opterr;
	unsigned গুনতি;

	optstring = *++argv;
	যদি (!optstring || !(var = *++argv)) {
		bb_simple_error_বার্তা("usage: getopts OPTSTRING VAR [ARGS]");
		ফেরত EXIT_FAILURE;
	}

	যদি (argv[1])
		argv[0] = G.global_argv[0]; /* for error messages in getopt() */
	অন্যথা
		argv = G.global_argv;
	cbuf[1] = '\0';

	my_opterr = 0;
	যদি (optstring[0] != ':') {
		cp = get_local_var_মান("OPTERR");
		/* 0 if "OPTERR=0", 1 otherwise */
		my_opterr = (!cp || NOT_LONE_CHAR(cp, '0'));
	}

	/* getopts stops on first non-option. Add "+" to force that */
	/*if (optstring[0] != '+')*/ {
		অক্ষর *s = alloca(strlen(optstring) + 2);
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
	গুনতি = 0;
	n = string_array_len(argv);
	করো {
		optarg = ফাঁকা;
		opterr = (গুনতি < G.getopt_গুনতি) ? 0 : my_opterr;
		c = getopt(n, argv, optstring);
		যদি (c < 0)
			বিরতি;
		গুনতি++;
	} যখন (গুনতি <= G.getopt_গুনতি);

	/* Set OPTIND. Prevent resetting of the magic counter! */
	set_local_var_from_halves("OPTIND", utoa(optind));
	G.getopt_গুনতি = গুনতি; /* "next time, give me N+1'th result" */
	GETOPT_RESET(); /* just in case */

	/* Set OPTARG */
	/* Always set or unset, never left as-is, even on exit/error:
	 * "If no option was found, or if the option that was found
	 * does not have an option-argument, OPTARG shall be unset."
	 */
	cp = optarg;
	যদি (c == '?') {
		/* If ":optstring" and unknown option is seen,
		 * it is stored to OPTARG.
		 */
		যদি (optstring[1] == ':') {
			cbuf[0] = optopt;
			cp = cbuf;
		}
	}
	যদি (cp)
		set_local_var_from_halves("OPTARG", cp);
	অন্যথা
		unset_local_var("OPTARG");

	/* Convert -1 to "?" */
	exitcode = EXIT_SUCCESS;
	যদি (c < 0) { /* -1: end of options */
		exitcode = EXIT_FAILURE;
		c = '?';
	}

	/* Set VAR */
	cbuf[0] = c;
	set_local_var_from_halves(var, cbuf);

	ফেরত exitcode;
}
#endif

স্থির পূর্ণসংখ্যা FAST_FUNC builtin_source(অক্ষর **argv)
{
	অক্ষর *arg_path, *filename;
	HFILE *প্রবেশ;
	save_arg_t sv;
	অক্ষর *args_need_save;
#যদি ENABLE_HUSH_FUNCTIONS
	smallint sv_flg;
#endif

	argv = skip_dash_dash(argv);
	filename = argv[0];
	যদি (!filename) {
		/* bash says: "bash: .: filename argument required" */
		ফেরত 2; /* bash compat */
	}
	arg_path = ফাঁকা;
	যদি (!strchr(filename, '/')) {
		arg_path = find_in_path(filename);
		যদি (arg_path)
			filename = arg_path;
		অন্যথা যদি (!ENABLE_HUSH_BASH_SOURCE_CURDIR) {
			errno = ENOENT;
			bb_simple_perror_বার্তা(filename);
			ফেরত EXIT_FAILURE;
		}
	}
	প্রবেশ = hfopen(filename);
	মুক্ত(arg_path);
	যদি (!প্রবেশ) {
		bb_perror_বার্তা("%s", filename);
		/* POSIX: non-interactive shell should abort here,
		 * not merely fail. So far no one complained :)
		 */
		ফেরত EXIT_FAILURE;
	}

#যদি ENABLE_HUSH_FUNCTIONS
	sv_flg = G_flag_return_in_progress;
	/* "we are inside sourced file, ok to use return" */
	G_flag_return_in_progress = -1;
#endif
	args_need_save = argv[1]; /* used as a boolean variable */
	যদি (args_need_save)
		save_and_replace_G_args(&sv, argv);

	/* "false; . ./empty_line; echo Zero:$?" should print 0 */
	G.last_exitcode = 0;
	parse_and_run_file(প্রবেশ);
	hfclose(প্রবেশ);

	যদি (args_need_save) /* can't use argv[1] instead: "shift" can mangle it */
		restore_G_args(&sv, argv);
#যদি ENABLE_HUSH_FUNCTIONS
	G_flag_return_in_progress = sv_flg;
#endif

	ফেরত G.last_exitcode;
}

#যদি ENABLE_HUSH_TRAP
স্থির পূর্ণসংখ্যা FAST_FUNC builtin_trap(অক্ষর **argv)
{
	পূর্ণসংখ্যা sig;
	অক্ষর *new_cmd;

	যদি (!G_traps)
		G_traps = xzalloc(আকার(G_traps[0]) * NSIG);

	argv++;
	যদি (!*argv) {
		পূর্ণসংখ্যা i;
		/* No args: print all trapped */
		জন্য (i = 0; i < NSIG; ++i) {
			যদি (G_traps[i]) {
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
		ফেরত EXIT_SUCCESS;
	}

	new_cmd = ফাঁকা;
	/* If first arg is a number: reset all specified signals */
	sig = bb_strtou(*argv, ফাঁকা, 10);
	যদি (errno == 0) {
		পূর্ণসংখ্যা ret;
 process_sig_list:
		ret = EXIT_SUCCESS;
		যখন (*argv) {
			sighandler_t handler;

			sig = get_signum(*argv++);
			যদি (sig < 0) {
				ret = EXIT_FAILURE;
				/* Mimic bash message exactly */
				bb_error_বার্তা("trap: %s: invalid signal specification", argv[-1]);
				চালু;
			}

			মুক্ত(G_traps[sig]);
			G_traps[sig] = xstrdup(new_cmd);

			debug_printf("trap: setting SIG%s (%i) to '%s'\n",
				get_signame(sig), sig, G_traps[sig]);

			/* There is no signal for 0 (EXIT) */
			যদি (sig == 0)
				চালু;

			যদি (new_cmd)
				handler = (new_cmd[0] ? record_pending_signo : SIG_IGN);
			অন্যথা
				/* We are removing trap handler */
				handler = pick_sighandler(sig);
			install_sighandler(sig, handler);
		}
		ফেরত ret;
	}

	যদি (!argv[1]) { /* no second arg */
		bb_simple_error_বার্তা("trap: invalid arguments");
		ফেরত EXIT_FAILURE;
	}

	/* First arg is "-": reset all specified to default */
	/* First arg is "--": skip it, the rest is "handler SIGs..." */
	/* Everything else: set arg as signal handler
	 * (includes "" case, which ignores signal) */
	যদি (argv[0][0] == '-') {
		যদি (argv[0][1] == '\0') { /* "-" */
			/* new_cmd remains NULL: "reset these sigs" */
			যাও reset_traps;
		}
		যদি (argv[0][1] == '-' && argv[0][2] == '\0') { /* "--" */
			argv++;
		}
		/* else: "-something", no special meaning */
	}
	new_cmd = *argv;
 reset_traps:
	argv++;
	যাও process_sig_list;
}
#endif

#যদি ENABLE_HUSH_JOB
স্থির কাঠামো pipe *parse_jobspec(ধ্রুবক অক্ষর *str)
{
	কাঠামো pipe *pi;
	unsigned jobnum;

	যদি (sscanf(str, "%%%u", &jobnum) != 1) {
		যদি (str[0] != '%'
		 || (str[1] != '%' && str[1] != '+' && str[1] != '\0')
		) {
			bb_error_বার্তা("bad argument '%s'", str);
			ফেরত ফাঁকা;
		}
		/* It is "%%", "%+" or "%" - current job */
		jobnum = G.last_jobid;
		যদি (jobnum == 0) {
			bb_simple_error_বার্তা("no current job");
			ফেরত ফাঁকা;
		}
	}
	জন্য (pi = G.job_list; pi; pi = pi->পরবর্তী) {
		যদি (pi->jobid == jobnum) {
			ফেরত pi;
		}
	}
	bb_error_বার্তা("%u: no such job", jobnum);
	ফেরত ফাঁকা;
}

স্থির পূর্ণসংখ্যা FAST_FUNC builtin_jobs(অক্ষর **argv UNUSED_PARAM)
{
	কাঠামো pipe *job;
	ধ্রুবক অক্ষর *status_string;

	checkjobs(ফাঁকা, 0 /*(no pid to wait for)*/);
	জন্য (job = G.job_list; job; job = job->পরবর্তী) {
		যদি (job->alive_cmds == job->stopped_cmds)
			status_string = "Stopped";
		অন্যথা
			status_string = "Running";

		printf(JOB_STATUS_FORMAT, job->jobid, status_string, job->cmdtext);
	}

	clean_up_last_dead_job();

	ফেরত EXIT_SUCCESS;
}

/* built-in 'fg' and 'bg' handler */
স্থির পূর্ণসংখ্যা FAST_FUNC builtin_fg_bg(অক্ষর **argv)
{
	পূর্ণসংখ্যা i;
	কাঠামো pipe *pi;

	যদি (!G_interactive_fd)
		ফেরত EXIT_FAILURE;

	/* If they gave us no args, assume they want the last backgrounded task */
	যদি (!argv[1]) {
		জন্য (pi = G.job_list; pi; pi = pi->পরবর্তী) {
			যদি (pi->jobid == G.last_jobid) {
				যাও found;
			}
		}
		bb_error_বার্তা("%s: no current job", argv[0]);
		ফেরত EXIT_FAILURE;
	}

	pi = parse_jobspec(argv[1]);
	যদি (!pi)
		ফেরত EXIT_FAILURE;
 found:
	/* TODO: bash prints a string representation
	 * of job being foregrounded (like "sleep 1 | cat") */
	যদি (argv[0][0] == 'f' && G_saved_tty_pgrp) {
		/* Put the job into the foreground. */
		tcsetpgrp(G_interactive_fd, pi->pgrp);
	}

	/* Restart the processes in the job */
	debug_printf_jobs("reviving %d procs, pgrp %d\n", pi->অঙ্ক_cmds, pi->pgrp);
	জন্য (i = 0; i < pi->অঙ্ক_cmds; i++) {
		debug_printf_jobs("reviving pid %d\n", pi->cmds[i].pid);
	}
	pi->stopped_cmds = 0;

	i = kill(- pi->pgrp, SIGCONT);
	যদি (i < 0) {
		যদি (errno == ESRCH) {
			delete_finished_job(pi);
			ফেরত EXIT_SUCCESS;
		}
		bb_simple_perror_বার্তা("kill (SIGCONT)");
	}

	যদি (argv[0][0] == 'f') {
		remove_job_from_table(pi); /* FG job shouldn't be in job table */
		ফেরত checkjobs_and_fg_shell(pi);
	}
	ফেরত EXIT_SUCCESS;
}
#endif

#যদি ENABLE_HUSH_KILL
স্থির পূর্ণসংখ্যা FAST_FUNC builtin_kill(অক্ষর **argv)
{
	পূর্ণসংখ্যা ret = 0;

# যদি ENABLE_HUSH_JOB
	যদি (argv[1] && strcmp(argv[1], "-l") != 0) {
		পূর্ণসংখ্যা i = 1;

		করো {
			কাঠামো pipe *pi;
			অক্ষর *dst;
			পূর্ণসংখ্যা j, n;

			যদি (argv[i][0] != '%')
				চালু;
			/*
			 * "kill %N" - job kill
			 * Converting to pgrp / pid kill
			 */
			pi = parse_jobspec(argv[i]);
			যদি (!pi) {
				/* Eat bad jobspec */
				j = i;
				করো {
					j++;
					argv[j - 1] = argv[j];
				} যখন (argv[j]);
				ret = 1;
				i--;
				চালু;
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
			n = G_interactive_fd ? 1 : pi->অঙ্ক_cmds;
			dst = alloca(n * আকার(পূর্ণসংখ্যা)*4);
			argv[i] = dst;
			যদি (G_interactive_fd)
				dst += sprintf(dst, " -%u", (পূর্ণসংখ্যা)pi->pgrp);
			অন্যথা জন্য (j = 0; j < n; j++) {
				কাঠামো command *cmd = &pi->cmds[j];
				/* Skip exited members of the job */
				যদি (cmd->pid == 0)
					চালু;
				/*
				 * kill_main has matching code to expect
				 * leading space. Needed to not confuse
				 * negative pids with "kill -SIGNAL_NO" syntax
				 */
				dst += sprintf(dst, " %u", (পূর্ণসংখ্যা)cmd->pid);
			}
			*dst = '\0';
		} যখন (argv[++i]);
	}
# endif

	যদি (argv[1] || ret == 0) {
		ret = run_applet_main(argv, kill_main);
	}
	/* else: ret = 1, "kill %bad_jobspec" case */
	ফেরত ret;
}
#endif

#যদি ENABLE_HUSH_WAIT
/* http://www.opengroup.org/onlinepubs/9699919799/utilities/wait.html */
# যদি !ENABLE_HUSH_JOB
#  define wait_for_child_or_signal(pipe,pid) wait_for_child_or_signal(pid)
# endif
স্থির পূর্ণসংখ্যা wait_for_child_or_signal(কাঠামো pipe *waitfor_pipe, pid_t waitfor_pid)
{
	পূর্ণসংখ্যা ret = 0;
	জন্য (;;) {
		পূর্ণসংখ্যা sig;
		sigset_t oldset;

		যদি (!sigisemptyset(&G.pending_set))
			যাও check_sig;

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

		যদি (!sigisemptyset(&G.pending_set)) {
			/* Crap! we raced with some signal! */
			যাও restore;
		}

		/*errno = 0; - checkjobs does this */
/* Can't pass waitfor_pipe into checkjobs(): it won't be interruptible */
		ret = checkjobs(ফাঁকা, waitfor_pid); /* waitpid(WNOHANG) inside */
		debug_printf_exec("checkjobs:%d\n", ret);
# যদি ENABLE_HUSH_JOB
		যদি (waitfor_pipe) {
			পূর্ণসংখ্যা rcode = job_exited_or_stopped(waitfor_pipe);
			debug_printf_exec("job_exited_or_stopped:%d\n", rcode);
			যদি (rcode >= 0) {
				ret = rcode;
				sigprocmask(SIG_SETMASK, &oldset, ফাঁকা);
				বিরতি;
			}
		}
# endif
		/* if ECHILD, there are no children (ret is -1 or 0) */
		/* if ret == 0, no children changed state */
		/* if ret != 0, it's exitcode+1 of exited waitfor_pid child */
		যদি (errno == ECHILD || ret) {
			ret--;
			যদি (ret < 0) /* if ECHILD, may need to fix "ret" */
				ret = 0;
# যদি ENABLE_HUSH_BASH_COMPAT
			যদি (waitfor_pid == -1 && errno == ECHILD) {
				/* exitcode of "wait -n" with no children is 127, not 0 */
				ret = 127;
			}
# endif
			sigprocmask(SIG_SETMASK, &oldset, ফাঁকা);
			বিরতি;
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
		sigprocmask(SIG_SETMASK, &oldset, ফাঁকা);
 check_sig:
		/* So, did we get a signal? */
		sig = check_and_run_traps();
		যদি (sig /*&& sig != SIGCHLD - always true */) {
			/* Do this for any (non-ignored) signal, not only for ^C */
			ret = 128 | sig;
			বিরতি;
		}
		/* SIGCHLD, or no signal, or ignored one, such as SIGQUIT. Repeat */
	}
	ফেরত ret;
}

স্থির পূর্ণসংখ্যা FAST_FUNC builtin_wait(অক্ষর **argv)
{
	পূর্ণসংখ্যা ret;
	পূর্ণসংখ্যা status;

	argv = skip_dash_dash(argv);
# যদি ENABLE_HUSH_BASH_COMPAT
	যদি (argv[0] && strcmp(argv[0], "-n") == 0) {
		/* wait -n */
		/* (bash accepts "wait -n PID" too and ignores PID) */
		G.dead_job_exitcode = -1;
		ফেরত wait_for_child_or_signal(ফাঁকা, -1 /*no job, wait for one job*/);
	}
# endif
	যদি (argv[0] == ফাঁকা) {
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
		ফেরত wait_for_child_or_signal(ফাঁকা, 0 /*no job and no pid to wait for*/);
	}

	করো {
		pid_t pid = bb_strtou(*argv, ফাঁকা, 10);
		যদি (errno || pid <= 0) {
# যদি ENABLE_HUSH_JOB
			যদি (argv[0][0] == '%') {
				কাঠামো pipe *wait_pipe;
				ret = 127; /* bash compat for bad jobspecs */
				wait_pipe = parse_jobspec(*argv);
				যদি (wait_pipe) {
					ret = job_exited_or_stopped(wait_pipe);
					যদি (ret < 0) {
						ret = wait_for_child_or_signal(wait_pipe, 0);
					} অন্যথা {
						/* waiting on "last dead job" removes it */
						clean_up_last_dead_job();
					}
				}
				/* else: parse_jobspec() already emitted error msg */
				চালু;
			}
# endif
			/* mimic bash message */
			bb_error_বার্তা("wait: '%s': not a pid or valid job spec", *argv);
			ret = EXIT_FAILURE;
			চালু; /* bash checks all argv[] */
		}

		/* Do we have such child? */
		ret = waitpid(pid, &status, WNOHANG);
		যদি (ret < 0) {
			/* No */
			ret = 127;
			যদি (errno == ECHILD) {
				যদি (pid == G.last_bg_pid) {
					/* "wait $!" but last bg task has already exited. Try:
					 * (sleep 1; exit 3) & sleep 2; echo $?; wait $!; echo $?
					 * In bash it prints exitcode 0, then 3.
					 * In dash, it is 127.
					 */
					ret = G.last_bg_pid_exitcode;
				} অন্যথা {
					/* Example: "wait 1". mimic bash message */
					bb_error_বার্তা("wait: pid %u is not a child of this shell", (unsigned)pid);
				}
			} অন্যথা {
				/* ??? */
				bb_perror_বার্তা("wait %s", *argv);
			}
			চালু; /* bash checks all argv[] */
		}
		যদি (ret == 0) {
			/* Yes, and it still runs */
			ret = wait_for_child_or_signal(ফাঁকা, pid);
		} অন্যথা {
			/* Yes, and it just exited */
			process_wait_ফলাফল(ফাঁকা, pid, status);
			ret = WEXITSTATUS(status);
			যদি (WIFSIGNALED(status))
				ret = 128 | WTERMSIG(status);
		}
	} যখন (*++argv);

	ফেরত ret;
}
#endif

#যদি ENABLE_HUSH_LOOPS || ENABLE_HUSH_FUNCTIONS
স্থির unsigned parse_numeric_argv1(অক্ষর **argv, unsigned def, unsigned def_min)
{
	যদি (argv[1]) {
		def = bb_strtou(argv[1], ফাঁকা, 10);
		যদি (errno || def < def_min || argv[2]) {
			bb_error_বার্তা("%s: bad arguments", argv[0]);
			def = UINT_MAX;
		}
	}
	ফেরত def;
}
#endif

#যদি ENABLE_HUSH_LOOPS
স্থির পূর্ণসংখ্যা FAST_FUNC builtin_break(অক্ষর **argv)
{
	unsigned depth;
	যদি (G.depth_of_loop == 0) {
		bb_error_বার্তা("%s: only meaningful in a loop", argv[0]);
		/* if we came from builtin_continue(), need to undo "= 1" */
		G.flag_break_continue = 0;
		ফেরত EXIT_SUCCESS; /* bash compat */
	}
	G.flag_break_continue++; /* BC_BREAK = 1, or BC_CONTINUE = 2 */

	G.depth_break_continue = depth = parse_numeric_argv1(argv, 1, 1);
	যদি (depth == UINT_MAX)
		G.flag_break_continue = BC_BREAK;
	যদি (G.depth_of_loop < depth)
		G.depth_break_continue = G.depth_of_loop;

	ফেরত EXIT_SUCCESS;
}

স্থির পূর্ণসংখ্যা FAST_FUNC builtin_continue(অক্ষর **argv)
{
	G.flag_break_continue = 1; /* BC_CONTINUE = 2 = 1+1 */
	ফেরত builtin_break(argv);
}
#endif

#যদি ENABLE_HUSH_FUNCTIONS
স্থির পূর্ণসংখ্যা FAST_FUNC builtin_return(অক্ষর **argv)
{
	পূর্ণসংখ্যা rc;

	যদি (G_flag_return_in_progress != -1) {
		bb_error_বার্তা("%s: not in a function or sourced script", argv[0]);
		ফেরত EXIT_FAILURE; /* bash compat */
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
# যদি ENABLE_HUSH_TRAP
	যদি (argv[1]) { /* "return ARG" inside a running trap sets $? */
		debug_printf_exec("G.return_exitcode=%d\n", rc);
		G.return_exitcode = rc;
	}
# endif
	ফেরত rc;
}
#endif

#যদি ENABLE_HUSH_TIMES
স্থির পূর্ণসংখ্যা FAST_FUNC builtin_times(অক্ষর **argv UNUSED_PARAM)
{
	স্থির ধ্রুবক uint8_t times_tbl[] ALIGN1 = {
		' ',  offsetof(কাঠামো tms, tms_utime),
		'\n', offsetof(কাঠামো tms, tms_stime),
		' ',  offsetof(কাঠামো tms, tms_cutime),
		'\n', offsetof(কাঠামো tms, tms_cstime),
		0
	};
	ধ্রুবক uint8_t *p;
	unsigned clk_tck;
	কাঠামো tms buf;

	clk_tck = bb_clk_tck();

	times(&buf);
	p = times_tbl;
	করো {
		unsigned sec, frac;
		unsigned long t;
		t = *(clock_t *)(((অক্ষর *) &buf) + p[1]);
		sec = t / clk_tck;
		frac = t % clk_tck;
		printf("%um%u.%03us%c",
			sec / 60, sec % 60,
			(frac * 1000) / clk_tck,
			p[0]);
		p += 2;
	} যখন (*p);

	ফেরত EXIT_SUCCESS;
}
#endif

#যদি ENABLE_HUSH_MEMLEAK
স্থির পূর্ণসংখ্যা FAST_FUNC builtin_memleak(অক্ষর **argv UNUSED_PARAM)
{
	শূন্য *p;
	unsigned long l;

# ifdef M_TRIM_THRESHOLD
	/* Optional. Reduces probability of false positives */
	malloc_trim(0);
# endif
	/* Crude attempt to find where "free memory" starts,
	 * sans fragmentation. */
	p = malloc(240);
	l = (unsigned long)p;
	মুক্ত(p);
	p = malloc(3400);
	যদি (l < (unsigned long)p) l = (unsigned long)p;
	মুক্ত(p);

# যদি 0  /* debug */
	{
		কাঠামো mallinfo mi = mallinfo();
		printf("top alloc:0x%lx malloced:%d+%d=%d\n", l,
			mi.arena, mi.hblkhd, mi.arena + mi.hblkhd);
	}
# endif

	যদি (!G.memleak_মান)
		G.memleak_মান = l;

	l -= G.memleak_মান;
	যদি ((long)l < 0)
		l = 0;
	l /= 1024;
	যদি (l > 127)
		l = 127;

	/* Exitcode is "how many kilobytes we leaked since 1st call" */
	ফেরত l;
}
#endif
#endif /* !__U_BOOT__ */
