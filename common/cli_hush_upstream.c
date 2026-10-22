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
#ако !(defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) \
	|| defined(__APPLE__) \
    )
# include <malloc.h>   /* for malloc_trim */
#endif
#include <glob.h>
/* #include <dmalloc.h> */
#ако ENABLE_HUSH_CASE
# include <fnmatch.h>
#endif
#include <sys/times.h>
#include <sys/utsname.h> /* for setting $HOSTNAME */

#include "busybox.h"  /* for APPLET_IS_NOFORK/NOEXEC */
#include "unicode.h"
#include "shell_common.h"
#include "math.h"
#include "match.h"
#ако ENABLE_HUSH_RANDOM_SUPPORT
# include "random.h"
#иначе
# define CLEAR_RANDOM_T(rnd) ((void)0)
#endif
#ifndef O_CLOEXEC
# define O_CLOEXEC 0
#endif
#ifndef F_DUPFD_CLOEXEC
# define F_DUPFD_CLOEXEC F_DUPFD
#endif

#ако ENABLE_FEATURE_SH_EMBEDDED_SCRIPTS && !ENABLE_SHELL_ASH
# include "embedded_scripts.h"
#иначе
# define NUM_SCRIPTS 0
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

#ако BUILD_AS_NOMMU
# undef BB_MMU
# undef USE_FOR_NOMMU
# undef USE_FOR_MMU
# define BB_MMU 0
# define USE_FOR_NOMMU(...) __VA_ARGS__
# define USE_FOR_MMU(...)
#endif

#ifndef __U_BOOT__
#include "NUM_APPLETS.h"
#ако NUM_APPLETS == 1
/* STANDALONE does not make sense, and won't compile */
# undef ENABLE_FEATURE_SH_STANDALONE
# undef IF_FEATURE_SH_STANDALONE
# undef IF_NOT_FEATURE_SH_STANDALONE
# define ENABLE_FEATURE_SH_STANDALONE 0
# define IF_FEATURE_SH_STANDALONE(...)
# define IF_NOT_FEATURE_SH_STANDALONE(...) __VA_ARGS__
#endif
#endif /* __U_BOOT__ */

#ако !ENABLE_HUSH_INTERACTIVE
# undef ENABLE_FEATURE_EDITING
# define ENABLE_FEATURE_EDITING 0
# undef ENABLE_FEATURE_EDITING_FANCY_PROMPT
# define ENABLE_FEATURE_EDITING_FANCY_PROMPT 0
# undef ENABLE_FEATURE_EDITING_SAVE_ON_EXIT
# define ENABLE_FEATURE_EDITING_SAVE_ON_EXIT 0
#endif

/* Do we support ANY keywords? */
#ако ENABLE_HUSH_IF || ENABLE_HUSH_LOOPS || ENABLE_HUSH_CASE
# define HAS_KEYWORDS 1
# define IF_HAS_KEYWORDS(...) __VA_ARGS__
# define IF_HAS_NO_KEYWORDS(...)
#иначе
# define HAS_KEYWORDS 0
# define IF_HAS_KEYWORDS(...)
# define IF_HAS_NO_KEYWORDS(...) __VA_ARGS__
#endif

/* If you comment out one of these below, it will be #defined later
 * to perform debug printfs to stderr: */
#define debug_printf(...)         do {} докато (0)
/* Finer-grained debug switches */
#define debug_printf_parse(...)   do {} докато (0)
#define debug_printf_heredoc(...) do {} докато (0)
#define debug_print_tree(a, b)    do {} докато (0)
#define debug_printf_exec(...)    do {} докато (0)
#define debug_printf_env(...)     do {} докато (0)
#define debug_printf_jobs(...)    do {} докато (0)
#define debug_printf_expand(...)  do {} докато (0)
#define debug_printf_varexp(...)  do {} докато (0)
#define debug_printf_glob(...)    do {} докато (0)
#define debug_printf_redir(...)   do {} докато (0)
#define debug_printf_list(...)    do {} докато (0)
#define debug_printf_subst(...)   do {} докато (0)
#define debug_printf_prompt(...)  do {} докато (0)
#define debug_printf_clean(...)   do {} докато (0)

#define ERR_PTR ((void*)(long)1)

#define JOB_STATUS_FORMAT    "[%u] %-22s %.40s\n"

#define _SPECIAL_VARS_STR     "_*@$!?#-"
#ifndef __U_BOOT__
#define SPECIAL_VARS_STR     ("_*@$!?#-" + 1)
#define NUMERIC_SPECVARS_STR ("_*@$!?#-" + 3)
#иначе /* __U_BOOT__ */
#define SPECIAL_VARS_STR     "*@$!?#-"
#define NUMERIC_SPECVARS_STR "$!?#-"
#endif /* __U_BOOT__ */
#ако BASH_PATTERN_SUBST
/* Support / and // replace ops */
/* Note that // is stored as \ in "encoded" string representation */
# define VAR_ENCODED_SUBST_OPS      "\\/%#:-=+?"
# define VAR_SUBST_OPS             ("\\/%#:-=+?" + 1)
# define MINUS_PLUS_EQUAL_QUESTION ("\\/%#:-=+?" + 5)
#иначе
# define VAR_ENCODED_SUBST_OPS      "%#:-=+?"
# define VAR_SUBST_OPS              "%#:-=+?"
# define MINUS_PLUS_EQUAL_QUESTION ("%#:-=+?" + 3)
#endif

#define SPECIAL_VAR_SYMBOL_STR "\3"
#define SPECIAL_VAR_SYMBOL       3
/* The "variable" with name "\1" emits string "\3". Testcase: "echo ^C" */
#define SPECIAL_VAR_QUOTED_SVS   1

struct variable;

static const char hush_version_str[] ALIGN1 = "HUSH_VERSION="BB_VER;

/* This supports saving pointers malloced in vfork child,
 * to be freed in the parent.
 */
#ако !BB_MMU
typedef struct nommu_save_t {
	struct variable *old_vars;
	char **argv;
	char **argv_from_re_execing;
} nommu_save_t;
#endif

enum {
	RES_NONE  = 0,
#ако ENABLE_HUSH_IF
	RES_IF    ,
	RES_THEN  ,
	RES_ELIF  ,
	RES_ELSE  ,
	RES_FI    ,
#endif
#ако ENABLE_HUSH_LOOPS
	RES_FOR   ,
	RES_WHILE ,
	RES_UNTIL ,
	RES_DO    ,
	RES_DONE  ,
#endif
#ако ENABLE_HUSH_LOOPS || ENABLE_HUSH_CASE
	RES_IN    ,
#endif
#ако ENABLE_HUSH_CASE
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

typedef struct o_string {
	char *data;
	цяло length; /* position where data is appended */
	цяло maxlen;
	цяло o_expflags;
	/* At least some part of the string was inside '' or "",
	 * possibly empty one: word"", wo''rd etc. */
	smallint has_quoted_part;
	smallint has_empty_slot;
	smallint ended_in_ifs;
} o_string;
enum {
	EXP_FLAG_SINGLEWORD     = 0x80, /* must be 0x80 */
	EXP_FLAG_GLOB           = 0x2,
	/* Protect newly added chars against globbing
	 * by prepending \ to *, ?, [, \ */
	EXP_FLAG_ESC_GLOB_CHARS = 0x1,
};
/* Used for initialization: o_string foo = NULL_O_STRING; */
#define NULL_O_STRING { NULL }

#ifndef debug_printf_parse
static const char *const assignment_flag[] ALIGN_PTR = {
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
typedef struct HFILE {
	char *cur;
	char *end;
	struct HFILE *next_hfile;
	цяло fd;
	char buf[1024];
} HFILE;

typedef struct in_str {
	const char *p;
	цяло peek_buf[2];
	цяло last_char;
	HFILE *file;
} in_str;

#ifndef __U_BOOT__
/* The descrip member of this structure is only used to make
 * debugging output pretty */
static const struct {
	int32_t mode;
	signed char default_fd;
	char descrip[3];
} redir_table[] ALIGN4 = {
	{ O_RDONLY,                  0, "<"  },
	{ O_CREAT|O_TRUNC|O_WRONLY,  1, ">"  },
	{ O_CREAT|O_APPEND|O_WRONLY, 1, ">>" },
	{ O_CREAT|O_RDWR,            1, "<>" },
	{ O_RDONLY,                  0, "<<" },
/* Should not be needed. Bogus default_fd helps in debugging */
/*	{ O_RDONLY,                 77, "<<" }, */
};

struct redir_struct {
	struct redir_struct *next;
	char *rd_filename;          /* filename */
	цяло rd_fd;                  /* fd to redirect */
	/* fd to redirect to, or -3 if rd_fd is to be closed (n>&-) */
	цяло rd_dup;
	smallint rd_type;           /* (enum redir_type) */
	/* note: for heredocs, rd_filename contains heredoc delimiter,
	 * and subsequently heredoc itself; and rd_dup is a bitmask:
	 * bit 0: do we need to trim leading tabs?
	 * bit 1: is heredoc quoted (<<'delim' syntax) ?
	 */
};
typedef enum redir_type {
	REDIRECT_INPUT     = 0,
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

struct command {
#ifndef __U_BOOT__
	pid_t pid;                  /* 0 if exited */
#endif /* !__U_BOOT__ */
	unsigned assignment_cnt;    /* how many argv[i] are assignments? */
#ако ENABLE_HUSH_LINENO_VAR
	unsigned lineno;
#endif
	smallint cmd_type;          /* CMD_xxx */
#define CMD_NORMAL   0
#define CMD_SUBSHELL 1
#ако BASH_TEST2
/* used for "[[ EXPR ]]" */
# define CMD_TEST2_SINGLEWORD_NOGLOB 2
#endif
#ако BASH_TEST2 || ENABLE_HUSH_LOCAL || ENABLE_HUSH_EXPORT || ENABLE_HUSH_READONLY
/* used to prevent word splitting and globbing in "export v=t*" */
# define CMD_SINGLEWORD_NOGLOB 3
#endif
#ако ENABLE_HUSH_FUNCTIONS
# define CMD_FUNCDEF 4
#endif

	smalluint cmd_exitcode;
	/* if non-NULL, this "command" is { list }, ( list ), or a compound statement */
	struct pipe *group;
#ако !BB_MMU
	char *group_as_string;
#endif
#ако ENABLE_HUSH_FUNCTIONS
	struct function *child_func;
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
	цяло argc; /* number of program arguments */
#endif
	char **argv;                /* command name and arguments */
/* argv vector may contain variable references (^Cvar^C, ^C0^C etc)
 * and on execution these are substituted with their values.
 * Substitution can make _several_ words out of one argv[n]!
 * Example: argv[0]=='.^C*^C.' here: echo .$*.
 * References of the form ^C`cmd arg^C are `cmd arg` substitutions.
 */
#ifndef __U_BOOT__
	struct redir_struct *redirects; /* I/O redirections */
#endif /* !__U_BOOT__ */
};
/* Is there anything in this command at all? */
#ifndef __U_BOOT__
#define IS_NULL_CMD(cmd) \
	(!(cmd)->group && !(cmd)->argv && !(cmd)->redirects)

#иначе /* __U_BOOT__ */
#define IS_NULL_CMD(cmd) \
	(!(cmd)->group && !(cmd)->argv)
#endif /* __U_BOOT__ */
struct pipe {
	struct pipe *next;
	цяло num_cmds;               /* total number of commands in pipe */
#ifndef __U_BOOT__
	цяло alive_cmds;             /* number of commands running (not exited) */
	цяло stopped_cmds;           /* number of commands alive, but stopped */
#ако ENABLE_HUSH_JOB
	unsigned jobid;             /* job number */
	pid_t pgrp;                 /* process group ID for the job */
	char *cmdtext;              /* name of job */
#endif
#endif /* !__U_BOOT__ */
	struct command *cmds;       /* array of commands in pipe */
	smallint followup;          /* PIPE_BG, PIPE_SEQ, PIPE_OR, PIPE_AND */
	IF_HAS_KEYWORDS(smallint pi_inverted;) /* "! cmd | cmd" */
	IF_HAS_KEYWORDS(smallint res_word;) /* needed for if, for, while, until... */
};
typedef enum pipe_style {
	PIPE_SEQ = 0,
	PIPE_AND = 1,
	PIPE_OR  = 2,
	PIPE_BG  = 3,
} pipe_style;
/* Is there anything in this pipe at all? */
#define IS_NULL_PIPE(pi) \
	((pi)->num_cmds == 0 IF_HAS_KEYWORDS( && (pi)->res_word == RES_NONE))

/* This holds pointers to the various results of parsing */
struct parse_context {
	/* linked list of pipes */
	struct pipe *list_head;
	/* last pipe (being constructed right now) */
	struct pipe *pipe;
	/* last command in pipe (being constructed right now) */
	struct command *command;
#ifndef __U_BOOT__
	/* last redirect in command->redirects list */
	struct redir_struct *pending_redirect;
#endif /* !__U_BOOT__ */
	o_string word;
#ако !BB_MMU
	o_string as_string;
#endif
	smallint is_assignment; /* 0:maybe, 1:yes, 2:no, 3:keyword */
#ако HAS_KEYWORDS
	smallint ctx_res_w;
	smallint ctx_inverted; /* "! cmd | cmd" */
#ако ENABLE_HUSH_CASE
	smallint ctx_dsemicolon; /* ";;" seen */
#endif
	/* bitmask of FLAG_xxx, for figuring out valid reserved words */
	цяло old_flag;
	/* group we are enclosed in:
	 * example: "if pipe1; pipe2; then pipe3; fi"
	 * when we see "if" or "then", we malloc and copy current context,
	 * and make ->stack point to it. then we parse pipeN.
	 * when closing "then" / fi" / whatever is found,
	 * we move list_head into ->stack->command->group,
	 * copy ->stack into current context, and delete ->stack.
	 * (parsing of { list } and ( list ) doesn't use this method)
	 */
	struct parse_context *stack;
#endif
};
enum {
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
struct variable {
	struct variable *next;
	char *varstr;        /* points to "name=" portion */
	цяло max_len;         /* if > 0, name is part of initial env; else name is malloced */
#ifndef __U_BOOT__
	uint16_t var_nest_level;
	smallint flg_export; /* putenv should be done on this var */
	smallint flg_read_only;
#endif /* !__U_BOOT__ */
};

enum {
	BC_BREAK = 1,
	BC_CONTINUE = 2,
};

#ако ENABLE_HUSH_FUNCTIONS
struct function {
	struct function *next;
	char *name;
	struct command *parent_cmd;
	struct pipe *body;
# ако !BB_MMU
	char *body_as_string;
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
static const char o_opt_strings[] ALIGN1 =
	"pipefail\0"
	"noexec\0"
	"errexit\0"
#ако ENABLE_HUSH_MODE_X
	"xtrace\0"
#endif
	;
enum {
	OPT_O_PIPEFAIL,
	OPT_O_NOEXEC,
	OPT_O_ERREXIT,
#ако ENABLE_HUSH_MODE_X
	OPT_O_XTRACE,
#endif
	NUM_OPT_O
};

/* "Globals" within this file */
/* Sorted roughly by size (smaller offsets == smaller code) */
struct globals {
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
#ако ENABLE_HUSH_INTERACTIVE
	/* 'interactive_fd' is a fd# open to ctty, if we have one
	 * _AND_ if we decided to act interactively */
	цяло interactive_fd;
	IF_NOT_FEATURE_EDITING_FANCY_PROMPT(char *PS1;)
# define G_interactive_fd (G.interactive_fd)
#иначе
# define G_interactive_fd 0
#endif
#иначе /* __U_BOOT__ */
# define G_interactive_fd 0
#endif /* __U_BOOT__ */
#ifndef __U_BOOT__
#ако ENABLE_FEATURE_EDITING
	line_input_t *line_input_state;
#endif
	pid_t root_pid;
	pid_t root_ppid;
	pid_t last_bg_pid;
#ако ENABLE_HUSH_RANDOM_SUPPORT
	random_t random_gen;
#endif
#ако ENABLE_HUSH_JOB
	цяло run_list_level;
	unsigned last_jobid;
	pid_t saved_tty_pgrp;
	struct pipe *job_list;
# define G_saved_tty_pgrp (G.saved_tty_pgrp)
#иначе
# define G_saved_tty_pgrp 0
#endif
#endif /* !__U_BOOT__ */
	/* How deeply are we in context where "set -e" is ignored */
	цяло errexit_depth;
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

	char o_opt[NUM_OPT_O];
#ако ENABLE_HUSH_MODE_X
# define G_x_mode (G.o_opt[OPT_O_XTRACE])
#иначе
# define G_x_mode 0
#endif
	char opt_s;
	char opt_c;
#endif /* !__U_BOOT__ */
#ако ENABLE_HUSH_INTERACTIVE
	smallint promptmode; /* 0: PS1, 1: PS2 */
#endif
	/* set by signal handler if SIGINT is received _and_ its trap is not set */
	smallint flag_SIGINT;
#ifndef __U_BOOT__
#ако ENABLE_HUSH_LOOPS
	smallint flag_break_continue;
#endif
#endif /* !__U_BOOT__ */
#ако ENABLE_HUSH_FUNCTIONS
	/* 0: outside of a function (or sourced file)
	 * -1: inside of a function, ok to use return builtin
	 * 1: return is invoked, skip all till end of func
	 */
	smallint flag_return_in_progress;
# define G_flag_return_in_progress (G.flag_return_in_progress)
#иначе
# define G_flag_return_in_progress 0
#endif
	smallint exiting; /* used to prevent EXIT trap recursion */
	/* These support $? */
	smalluint last_exitcode;
	smalluint expand_exitcode;
	smalluint last_bg_pid_exitcode;
#ifndef __U_BOOT__
#ако ENABLE_HUSH_SET
	/* are global_argv and global_argv[1..n] malloced? (note: not [0]) */
	smalluint global_args_malloced;
# define G_global_args_malloced (G.global_args_malloced)
#иначе
# define G_global_args_malloced 0
#endif
#ако ENABLE_HUSH_BASH_COMPAT
	цяло dead_job_exitcode; /* for "wait -n" */
#endif
#endif /* !__U_BOOT__ */
	/* how many non-NULL argv's we have. NB: $# + 1 */
	цяло global_argc;
	char **global_argv;
#ако !BB_MMU
	char *argv0_for_re_execing;
#endif
#ако ENABLE_HUSH_LOOPS
#ifndef __U_BOOT__
	unsigned depth_break_continue;
#endif /* !__U_BOOT__ */
	unsigned depth_of_loop;
#endif
#ifndef __U_BOOT__
#ако ENABLE_HUSH_GETOPTS
	unsigned getopt_count;
#endif
#endif /* !__U_BOOT__ */
	const char *ifs;
#ifdef __U_BOOT__
	цяло flag_repeat;
	цяло do_repeat;
	цяло run_command_flags;
#endif /* __U_BOOT__ */
	char *ifs_whitespace; /* = G.ifs or malloced */
#ifndef __U_BOOT__
	const char *cwd;
#endif /* !__U_BOOT__ */
	struct variable *top_var;
	char **expanded_assignments;
	struct variable **shadowed_vars_pp;
	unsigned var_nest_level;
#ifndef __U_BOOT__
#ако ENABLE_HUSH_FUNCTIONS
# ако ENABLE_HUSH_LOCAL
	unsigned func_nest_level; /* solely to prevent "local v" in non-functions */
# endif
	struct function *top_func;
#endif
	/* Signal and trap handling */
#ако ENABLE_HUSH_FAST
	unsigned count_SIGCHLD;
	unsigned handled_SIGCHLD;
	smallint we_have_children;
#endif
#ако ENABLE_HUSH_LINENO_VAR
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
#ако ENABLE_HUSH_JOB
	unsigned fatal_sig_mask;
# define G_fatal_sig_mask (G.fatal_sig_mask)
#иначе
# define G_fatal_sig_mask 0
#endif
#ако ENABLE_HUSH_TRAP
	цяло pre_trap_exitcode;
# ако ENABLE_HUSH_FUNCTIONS
	цяло return_exitcode;
# endif
	char **traps; /* char *traps[NSIG] */
# define G_traps G.traps
#иначе
# define G_traps ((char**)NULL)
#endif
	sigset_t pending_set;
#ако ENABLE_HUSH_MEMLEAK
	unsigned long memleak_value;
#endif
#ако ENABLE_HUSH_MODE_X
	unsigned x_mode_depth;
	/* "set -x" output should not be redirectable with subsequent 2>FILE.
	 * We dup fd#2 to x_mode_fd when "set -x" is executed, and use it
	 * for all subsequent output.
	 */
	цяло x_mode_fd;
	o_string x_mode_buf;
#endif
#endif /* !__U_BOOT__ */
#ако HUSH_DEBUG >= 2
	цяло debug_indent;
#endif
#ifndef __U_BOOT__
	struct sigaction sa;
	char optstring_buf[sizeof("eixcs")];
#ако BASH_EPOCH_VARS
	char epoch_buf[sizeof("%llu.nnnnnn") + sizeof(long long)*3];
#endif
#ако ENABLE_FEATURE_EDITING
	char user_input_buf[CONFIG_FEATURE_EDITING_MAX_LEN];
#endif
#endif /* !__U_BOOT__ */
};
#ifdef __U_BOOT__
struct globals *ptr_to_globals;
#endif /* __U_BOOT__ */
#define G (*ptr_to_globals)
/* Not #defining name to G.name - this quickly gets unwieldy
 * (too many defines). Also, I actually prefer to see when a variable
 * is global, thus "G." prefix is a useful hint */
#ifdef __U_BOOT__
#define SET_PTR_TO_GLOBALS(x) do { \
	(*(struct globals**)&ptr_to_globals) = (void*)(x); \
	barrier(); \
} докато (0)
#define INIT_G() do { \
	SET_PTR_TO_GLOBALS(xzalloc(sizeof(G))); \
	G.promptmode = 1; \
} докато (0)
#иначе /* !__U_BOOT__ */
#define INIT_G() do { \
	SET_PTR_TO_GLOBALS(xzalloc(sizeof(G))); \
	/* memset(&G.sa, 0, sizeof(G.sa)); */  \
	sigfillset(&G.sa.sa_mask); \
	G.sa.sa_flags = SA_RESTART; \
} докато (0)
#endif /* !__U_BOOT__ */

#ifndef __U_BOOT__
/* Function prototypes for builtins */
static цяло builtin_cd(char **argv) FAST_FUNC;
#ако ENABLE_HUSH_ECHO
static цяло builtin_echo(char **argv) FAST_FUNC;
#endif
static цяло builtin_eval(char **argv) FAST_FUNC;
static цяло builtin_exec(char **argv) FAST_FUNC;
static цяло builtin_exit(char **argv) FAST_FUNC;
#ако ENABLE_HUSH_EXPORT
static цяло builtin_export(char **argv) FAST_FUNC;
#endif
#ако ENABLE_HUSH_READONLY
static цяло builtin_readonly(char **argv) FAST_FUNC;
#endif
static цяло builtin_false(char **argv) FAST_FUNC;
#ако ENABLE_HUSH_JOB
static цяло builtin_fg_bg(char **argv) FAST_FUNC;
static цяло builtin_jobs(char **argv) FAST_FUNC;
#endif
#ако ENABLE_HUSH_GETOPTS
static цяло builtin_getopts(char **argv) FAST_FUNC;
#endif
#ако ENABLE_HUSH_HELP
static цяло builtin_help(char **argv) FAST_FUNC;
#endif
#ако MAX_HISTORY && ENABLE_FEATURE_EDITING
static цяло builtin_history(char **argv) FAST_FUNC;
#endif
#ако ENABLE_HUSH_LOCAL
static цяло builtin_local(char **argv) FAST_FUNC;
#endif
#ако ENABLE_HUSH_MEMLEAK
static цяло builtin_memleak(char **argv) FAST_FUNC;
#endif
#ако ENABLE_HUSH_PRINTF
static цяло builtin_printf(char **argv) FAST_FUNC;
#endif
static цяло builtin_pwd(char **argv) FAST_FUNC;
#ако ENABLE_HUSH_READ
static цяло builtin_read(char **argv) FAST_FUNC;
#endif
#ако ENABLE_HUSH_SET
static цяло builtin_set(char **argv) FAST_FUNC;
#endif
static цяло builtin_shift(char **argv) FAST_FUNC;
static цяло builtin_source(char **argv) FAST_FUNC;
#ако ENABLE_HUSH_TEST || BASH_TEST2
static цяло builtin_test(char **argv) FAST_FUNC;
#endif
#ако ENABLE_HUSH_TRAP
static цяло builtin_trap(char **argv) FAST_FUNC;
#endif
#ако ENABLE_HUSH_TYPE
static цяло builtin_type(char **argv) FAST_FUNC;
#endif
#ако ENABLE_HUSH_TIMES
static цяло builtin_times(char **argv) FAST_FUNC;
#endif
static цяло builtin_true(char **argv) FAST_FUNC;
#ако ENABLE_HUSH_UMASK
static цяло builtin_umask(char **argv) FAST_FUNC;
#endif
#ако ENABLE_HUSH_UNSET
static цяло builtin_unset(char **argv) FAST_FUNC;
#endif
#ако ENABLE_HUSH_KILL
static цяло builtin_kill(char **argv) FAST_FUNC;
#endif
#ако ENABLE_HUSH_WAIT
static цяло builtin_wait(char **argv) FAST_FUNC;
#endif
#ако ENABLE_HUSH_LOOPS
static цяло builtin_break(char **argv) FAST_FUNC;
static цяло builtin_continue(char **argv) FAST_FUNC;
#endif
#ако ENABLE_HUSH_FUNCTIONS
static цяло builtin_return(char **argv) FAST_FUNC;
#endif

/* Table of built-in functions.  They can be forked or not, depending on
 * context: within pipes, they fork.  As simple commands, they do not.
 * When used in non-forking context, they can change global variables
 * in the parent shell process.  If forked, of course they cannot.
 * For example, 'unset foo | whatever' will parse and run, but foo will
 * still be set at the end. */
struct built_in_command {
	const char *b_cmd;
	цяло (*b_function)(char **argv) FAST_FUNC;
#ако ENABLE_HUSH_HELP
	const char *b_descr;
# define BLTIN(cmd, func, help) { cmd, func, help }
#иначе
# define BLTIN(cmd, func, help) { cmd, func }
#endif
};

static const struct built_in_command bltins1[] ALIGN_PTR = {
	BLTIN("."        , builtin_source  , "Run commands in file"),
	BLTIN(":"        , builtin_true    , NULL),
#ако ENABLE_HUSH_JOB
	BLTIN("bg"       , builtin_fg_bg   , "Resume job in background"),
#endif
#ако ENABLE_HUSH_LOOPS
	BLTIN("break"    , builtin_break   , "Exit loop"),
#endif
	BLTIN("cd"       , builtin_cd      , "Change directory"),
#ако ENABLE_HUSH_LOOPS
	BLTIN("continue" , builtin_continue, "Start new loop iteration"),
#endif
	BLTIN("eval"     , builtin_eval    , "Construct and run shell command"),
	BLTIN("exec"     , builtin_exec    , "Execute command, don't return to shell"),
	BLTIN("exit"     , builtin_exit    , NULL),
#ако ENABLE_HUSH_EXPORT
	BLTIN("export"   , builtin_export  , "Set environment variables"),
#endif
	BLTIN("false"    , builtin_false   , NULL),
#ако ENABLE_HUSH_JOB
	BLTIN("fg"       , builtin_fg_bg   , "Bring job to foreground"),
#endif
#ако ENABLE_HUSH_GETOPTS
	BLTIN("getopts"  , builtin_getopts , NULL),
#endif
#ако ENABLE_HUSH_HELP
	BLTIN("help"     , builtin_help    , NULL),
#endif
#ако MAX_HISTORY && ENABLE_FEATURE_EDITING
	BLTIN("history"  , builtin_history , "Show history"),
#endif
#ако ENABLE_HUSH_JOB
	BLTIN("jobs"     , builtin_jobs    , "List jobs"),
#endif
#ако ENABLE_HUSH_KILL
	BLTIN("kill"     , builtin_kill    , "Send signals to processes"),
#endif
#ако ENABLE_HUSH_LOCAL
	BLTIN("local"    , builtin_local   , "Set local variables"),
#endif
#ако ENABLE_HUSH_MEMLEAK
	BLTIN("memleak"  , builtin_memleak , NULL),
#endif
#ако ENABLE_HUSH_READ
	BLTIN("read"     , builtin_read    , "Input into variable"),
#endif
#ако ENABLE_HUSH_READONLY
	BLTIN("readonly" , builtin_readonly, "Make variables read-only"),
#endif
#ако ENABLE_HUSH_FUNCTIONS
	BLTIN("return"   , builtin_return  , "Return from function"),
#endif
#ако ENABLE_HUSH_SET
	BLTIN("set"      , builtin_set     , "Set positional parameters"),
#endif
	BLTIN("shift"    , builtin_shift   , "Shift positional parameters"),
#ако BASH_SOURCE
	BLTIN("source"   , builtin_source  , NULL),
#endif
#ако ENABLE_HUSH_TIMES
	BLTIN("times"    , builtin_times   , NULL),
#endif
#ако ENABLE_HUSH_TRAP
	BLTIN("trap"     , builtin_trap    , "Trap signals"),
#endif
	BLTIN("true"     , builtin_true    , NULL),
#ако ENABLE_HUSH_TYPE
	BLTIN("type"     , builtin_type    , "Show command type"),
#endif
#ако ENABLE_HUSH_ULIMIT
	BLTIN("ulimit"   , shell_builtin_ulimit, "Control resource limits"),
#endif
#ако ENABLE_HUSH_UMASK
	BLTIN("umask"    , builtin_umask   , "Set file creation mask"),
#endif
#ако ENABLE_HUSH_UNSET
	BLTIN("unset"    , builtin_unset   , "Unset variables"),
#endif
#ако ENABLE_HUSH_WAIT
	BLTIN("wait"     , builtin_wait    , "Wait for process to finish"),
#endif
};
/* These builtins won't be used if we are on NOMMU and need to re-exec
 * (it's cheaper to run an external program in this case):
 */
static const struct built_in_command bltins2[] ALIGN_PTR = {
#ако ENABLE_HUSH_TEST
	BLTIN("["        , builtin_test    , NULL),
#endif
#ако BASH_TEST2
	BLTIN("[["       , builtin_test    , NULL),
#endif
#ако ENABLE_HUSH_ECHO
	BLTIN("echo"     , builtin_echo    , NULL),
#endif
#ако ENABLE_HUSH_PRINTF
	BLTIN("printf"   , builtin_printf  , NULL),
#endif
	BLTIN("pwd"      , builtin_pwd     , NULL),
#ако ENABLE_HUSH_TEST
	BLTIN("test"     , builtin_test    , NULL),
#endif
};

#endif /* !__U_BOOT__ */

/* Debug printouts.
 */
#ако HUSH_DEBUG >= 2
/* prevent disasters with G.debug_indent < 0 */
# define indent() fdprintf(2, "%*s", (G.debug_indent * 2) & 0xff, "")
# define debug_enter() (G.debug_indent++)
# define debug_leave() (G.debug_indent--)
#иначе
# define indent()      ((void)0)
# define debug_enter() ((void)0)
# define debug_leave() ((void)0)
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
#иначе
# define DEBUG_JOBS 0
#endif

#ifndef debug_printf_expand
# define debug_printf_expand(...) (indent(), fdprintf(2, __VA_ARGS__))
# define DEBUG_EXPAND 1
#иначе
# define DEBUG_EXPAND 0
#endif

#ifndef debug_printf_varexp
# define debug_printf_varexp(...) (indent(), fdprintf(2, __VA_ARGS__))
#endif

#ifndef debug_printf_glob
# define debug_printf_glob(...) (indent(), fdprintf(2, __VA_ARGS__))
# define DEBUG_GLOB 1
#иначе
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
#иначе
# define DEBUG_CLEAN 0
#endif

#ако DEBUG_EXPAND
static void debug_print_strings(const char *prefix, char **vv)
{
	indent();
	fdprintf(2, "%s:\n", prefix);
	докато (*vv)
		fdprintf(2, " '%s'\n", *vv++);
}
#иначе
# define debug_print_strings(prefix, vv) ((void)0)
#endif

/* Leak hunting. Use hush_leaktool.sh for post-processing.
 */
#ако LEAK_HUNTING
static void *xxmalloc(цяло lineno, size_t size)
{
	void *ptr = xmalloc((size + 0xff) & ~0xff);
	fdprintf(2, "line %d: malloc %p\n", lineno, ptr);
	върни ptr;
}
static void *xxrealloc(цяло lineno, void *ptr, size_t size)
{
	ptr = xrealloc(ptr, (size + 0xff) & ~0xff);
	fdprintf(2, "line %d: realloc %p\n", lineno, ptr);
	върни ptr;
}
static char *xxstrdup(цяло lineno, const char *str)
{
	char *ptr = xstrdup(str);
	fdprintf(2, "line %d: strdup %p\n", lineno, ptr);
	върни ptr;
}
static void xxfree(void *ptr)
{
	fdprintf(2, "free %p\n", ptr);
	free(ptr);
}
# define xmalloc(s)     xxmalloc(__LINE__, s)
# define xrealloc(p, s) xxrealloc(__LINE__, p, s)
# define xstrdup(s)     xxstrdup(__LINE__, s)
# define free(p)        xxfree(p)
#endif

/* Syntax and runtime errors. They always abort scripts.
 * In interactive use they usually discard unparsed and/or unexecuted commands
 * and return to the prompt.
 * HUSH_DEBUG >= 2 prints line number in this file where it was detected.
 */
#ако HUSH_DEBUG < 2
# define msg_and_die_if_script(lineno, ...)     msg_and_die_if_script(__VA_ARGS__)
# define syntax_error(lineno, msg)              syntax_error(msg)
# define syntax_error_at(lineno, msg)           syntax_error_at(msg)
# define syntax_error_unterm_ch(lineno, ch)     syntax_error_unterm_ch(ch)
# define syntax_error_unterm_str(lineno, s)     syntax_error_unterm_str(s)
# define syntax_error_unexpected_ch(lineno, ch) syntax_error_unexpected_ch(ch)
#endif

static void die_if_script(void)
{
	ако (!G_interactive_fd) {
		ако (G.last_exitcode) /* sometines it's 2, not 1 (bash compat) */
			xfunc_error_retval = G.last_exitcode;
		xfunc_die();
	}
}

#ifdef __U_BOOT__
static void __maybe_unused msg_and_die_if_script(unsigned lineno, const char *fmt, ...)
#иначе /* !__U_BOOT__ */
static void msg_and_die_if_script(unsigned lineno, const char *fmt, ...)
#endif /* !__U_BOOT__ */
{
	va_list p;

#ако HUSH_DEBUG >= 2
	bb_error_msg("hush.c:%u", lineno);
#endif
	va_start(p, fmt);
	bb_verror_msg(fmt, p, NULL);
	va_end(p);
	die_if_script();
}

static void syntax_error(unsigned lineno UNUSED_PARAM, const char *msg)
{
	ако (msg)
		bb_error_msg("syntax error: %s", msg);
	иначе
		bb_simple_error_msg("syntax error");
	die_if_script();
}

static void syntax_error_at(unsigned lineno UNUSED_PARAM, const char *msg)
{
	bb_error_msg("syntax error at '%s'", msg);
	die_if_script();
}

static void syntax_error_unterm_str(unsigned lineno UNUSED_PARAM, const char *s)
{
	bb_error_msg("syntax error: unterminated %s", s);
//? source4.tests fails: in bash, echo ${^} in script does not terminate the script
// (but bash --posix, or if bash is run as "sh", does terminate in script, so maybe uncomment this?)
//	die_if_script();
}

static void syntax_error_unterm_ch(unsigned lineno, char ch)
{
	char msg[2] = { ch, '\0' };
	syntax_error_unterm_str(lineno, msg);
}

static void syntax_error_unexpected_ch(unsigned lineno UNUSED_PARAM, цяло ch)
{
	char msg[2];
	msg[0] = ch;
	msg[1] = '\0';
#ако HUSH_DEBUG >= 2
	bb_error_msg("hush.c:%u", lineno);
#endif
	bb_error_msg("syntax error: unexpected %s", ch == EOF ? "EOF" : msg);
	die_if_script();
}

#ако HUSH_DEBUG < 2
# undef msg_and_die_if_script
# undef syntax_error
# undef syntax_error_at
# undef syntax_error_unterm_ch
# undef syntax_error_unterm_str
# undef syntax_error_unexpected_ch
#иначе
# define msg_and_die_if_script(...)     msg_and_die_if_script(__LINE__, __VA_ARGS__)
# define syntax_error(msg)              syntax_error(__LINE__, msg)
# define syntax_error_at(msg)           syntax_error_at(__LINE__, msg)
# define syntax_error_unterm_ch(ch)     syntax_error_unterm_ch(__LINE__, ch)
# define syntax_error_unterm_str(s)     syntax_error_unterm_str(__LINE__, s)
# define syntax_error_unexpected_ch(ch) syntax_error_unexpected_ch(__LINE__, ch)
#endif

/* Utility functions
 */
/* Replace each \x with x in place, return ptr past NUL. */
static char *unbackslash(char *src)
{
#ifdef __U_BOOT__
	char *dst = src = (char *)strchrnul(src, '\\');
#иначе /* !__U_BOOT__ */
	char *dst = src = strchrnul(src, '\\');
#endif /* !__U_BOOT__ */
	докато (1) {
		ако (*src == '\\') {
			src++;
			ако (*src != '\0') {
				/* \x -> x */
				*dst++ = *src++;
				continue;
			}
			/* else: "\<nul>". Do not delete this backslash.
			 * Testcase: eval 'echo ok\'
			 */
			*dst++ = '\\';
			/* fallthrough */
		}
		ако ((*dst++ = *src++) == '\0')
			break;
	}
	върни dst;
}

static char **add_strings_to_strings(char **strings, char **add, цяло need_to_dup)
{
	цяло i;
	unsigned count1;
	unsigned count2;
	char **v;

	v = strings;
	count1 = 0;
	ако (v) {
		докато (*v) {
			count1++;
			v++;
		}
	}
	count2 = 0;
	v = add;
	докато (*v) {
		count2++;
		v++;
	}
	v = xrealloc(strings, (count1 + count2 + 1) * sizeof(char*));
	v[count1 + count2] = NULL;
	i = count2;
	докато (--i >= 0)
		v[count1 + i] = (need_to_dup ? xstrdup(add[i]) : add[i]);
	върни v;
}
#ако LEAK_HUNTING
static char **xx_add_strings_to_strings(цяло lineno, char **strings, char **add, цяло need_to_dup)
{
	char **ptr = add_strings_to_strings(strings, add, need_to_dup);
	fdprintf(2, "line %d: add_strings_to_strings %p\n", lineno, ptr);
	върни ptr;
}
#define add_strings_to_strings(strings, add, need_to_dup) \
	xx_add_strings_to_strings(__LINE__, strings, add, need_to_dup)
#endif

/* Note: takes ownership of "add" ptr (it is not strdup'ed) */
static char **add_string_to_strings(char **strings, char *add)
{
	char *v[2];
	v[0] = add;
	v[1] = NULL;
	върни add_strings_to_strings(strings, v, /*dup:*/ 0);
}

#ако LEAK_HUNTING
static char **xx_add_string_to_strings(цяло lineno, char **strings, char *add)
{
	char **ptr = add_string_to_strings(strings, add);
	fdprintf(2, "line %d: add_string_to_strings %p\n", lineno, ptr);
	върни ptr;
}
#define add_string_to_strings(strings, add) \
	xx_add_string_to_strings(__LINE__, strings, add)
#endif

static void free_strings(char **strings)
{
	char **v;

	ако (!strings)
		върни;
	v = strings;
	докато (*v) {
		free(*v);
		v++;
	}
	free(strings);
}

#ifndef __U_BOOT__
static цяло dup_CLOEXEC(цяло fd, цяло avoid_fd)
{
	цяло newfd;
 repeat:
	newfd = fcntl(fd, F_DUPFD_CLOEXEC, avoid_fd + 1);
	ако (newfd >= 0) {
		ако (F_DUPFD_CLOEXEC == F_DUPFD) /* if old libc (w/o F_DUPFD_CLOEXEC) */
			close_on_exec_on(newfd);
	} иначе { /* newfd < 0 */
		ако (errno == EBUSY)
			goto repeat;
		ако (errno == EINTR)
			goto repeat;
		ако (errno != EBADF) {
			/* "echo >&9999" gets EINVAL trying to save fd 1 to above 9999.
			 * We could try saving it _below_ 9999 instead (how?), but
			 * this probably means that dup2(9999,1) to effectuate >&9999
			 * would also not work: fd 9999 can't exist.
			 * (This differs from "echo >&99" where saving works, but
			 * subsequent dup2(99,1) fails if fd 99 is not open).
			 */
			bb_perror_msg("fcntl(%d,F_DUPFD,%d)", fd, avoid_fd + 1);
		}
	}
	върни newfd;
}

static цяло xdup_CLOEXEC_and_close(цяло fd, цяло avoid_fd)
{
	цяло newfd;
 repeat:
	newfd = fcntl(fd, F_DUPFD_CLOEXEC, avoid_fd + 1);
	ако (newfd < 0) {
		ако (errno == EBUSY)
			goto repeat;
		ако (errno == EINTR)
			goto repeat;
		/* fd was not open? */
		ако (errno == EBADF)
			върни fd;
		xfunc_die();
	}
	ако (F_DUPFD_CLOEXEC == F_DUPFD) /* if old libc (w/o F_DUPFD_CLOEXEC) */
		close_on_exec_on(newfd);
	close(fd);
	върни newfd;
}

/* Manipulating HFILEs */
static HFILE *hfopen(const char *name)
{
	HFILE *fp;
	цяло fd;

	fd = STDIN_FILENO;
	ако (name) {
		fd = open(name, O_RDONLY | O_CLOEXEC);
		ако (fd < 0)
			върни NULL;
		ако (O_CLOEXEC == 0) /* ancient libc */
			close_on_exec_on(fd);
	}

	fp = xmalloc(sizeof(*fp));
	ако (name == NULL)
		G.HFILE_stdin = fp;
	fp->fd = fd;
	fp->cur = fp->end = fp->buf;
	fp->next_hfile = G.HFILE_list;
	G.HFILE_list = fp;
	върни fp;
}
static void hfclose(HFILE *fp)
{
	HFILE **pp = &G.HFILE_list;
	докато (*pp) {
		HFILE *cur = *pp;
		ако (cur == fp) {
			*pp = cur->next_hfile;
			break;
		}
		pp = &cur->next_hfile;
	}
	ако (fp->fd >= 0)
		close(fp->fd);
	free(fp);
}
static цяло refill_HFILE_and_getc(HFILE *fp)
{
	цяло n;

	ако (fp->fd < 0) {
		/* Already saw EOF */
		върни EOF;
	}
#ако ENABLE_HUSH_INTERACTIVE && !ENABLE_FEATURE_EDITING
	/* If user presses ^C, read() restarts after SIGINT (we use SA_RESTART).
	 * IOW: ^C will not immediately stop line input.
	 * But poll() is different: it does NOT restart after signals.
	 */
	ако (fp == G.HFILE_stdin) {
		struct pollfd pfd[1];
		pfd[0].fd = fp->fd;
		pfd[0].events = POLLIN;
		n = poll(pfd, 1, -1);
		ако (n < 0
		 /*&& errno == EINTR - assumed true */
		 && sigismember(&G.pending_set, SIGINT)
		) {
			върни '\0';
		}
	}
#иначе
/* if FEATURE_EDITING=y, we do not use this routine for interactive input */
#endif
	/* Try to buffer more input */
	n = safe_read(fp->fd, fp->buf, sizeof(fp->buf));
	ако (n < 0) {
		bb_simple_perror_msg("read error");
		n = 0;
	}
	fp->cur = fp->buf;
	fp->end = fp->buf + n;
	ако (n == 0) {
		/* EOF/error */
		close(fp->fd);
		fp->fd = -1;
		върни EOF;
	}
	върни (unsigned char)(*fp->cur++);
}
/* Inlined for common case of non-empty buffer.
 */
static ALWAYS_INLINE цяло hfgetc(HFILE *fp)
{
	ако (fp->cur < fp->end)
		върни (unsigned char)(*fp->cur++);
	/* Buffer empty */
	върни refill_HFILE_and_getc(fp);
}
static цяло move_HFILEs_on_redirect(цяло fd, цяло avoid_fd)
{
	HFILE *fl = G.HFILE_list;
	докато (fl) {
		ако (fd == fl->fd) {
			/* We use it only on script files, they are all CLOEXEC */
			fl->fd = xdup_CLOEXEC_and_close(fd, avoid_fd);
			debug_printf_redir("redirect_fd %d: matches a script fd, moving it to %d\n", fd, fl->fd);
			върни 1; /* "found and moved" */
		}
		fl = fl->next_hfile;
	}
#ако ENABLE_HUSH_MODE_X
	ако (G.x_mode_fd > 0 && fd == G.x_mode_fd) {
		G.x_mode_fd = xdup_CLOEXEC_and_close(fd, avoid_fd);
		върни 1; /* "found and moved" */
	}
#endif
	върни 0; /* "not in the list" */
}
#ако ENABLE_FEATURE_SH_STANDALONE && BB_MMU
static void close_all_HFILE_list(void)
{
	HFILE *fl = G.HFILE_list;
	докато (fl) {
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
		ако (fl->fd > 0)
			/*hfclose(fl); - unsafe */
			close(fl->fd);
		fl = fl->next_hfile;
	}
}
#endif
static цяло fd_in_HFILEs(цяло fd)
{
	HFILE *fl = G.HFILE_list;
	докато (fl) {
		ако (fl->fd == fd)
			върни 1;
		fl = fl->next_hfile;
	}
	върни 0;
}

#endif /* !__U_BOOT__ */

/* Helpers for setting new $n and restoring them back
 */
typedef struct save_arg_t {
	char *sv_argv0;
	char **sv_g_argv;
	цяло sv_g_argc;
#ifndef __U_BOOT__
	IF_HUSH_SET(smallint sv_g_malloced;)
#endif /* !__U_BOOT__ */
} save_arg_t;

#ifndef __U_BOOT__
static void save_and_replace_G_args(save_arg_t *sv, char **argv)
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

static void restore_G_args(save_arg_t *sv, char **argv)
{
#ако ENABLE_HUSH_SET
	ако (G.global_args_malloced) {
		/* someone ran "set -- arg1 arg2 ...", undo */
		char **pp = G.global_argv;
		докато (*++pp) /* note: does not free $0 */
			free(*pp);
		free(G.global_argv);
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
enum {
	SPECIAL_INTERACTIVE_SIGS = 0
		| (1 << SIGTERM)
		| (1 << SIGINT)
		| (1 << SIGHUP)
		,
	SPECIAL_JOBSTOP_SIGS = 0
#ако ENABLE_HUSH_JOB
		| (1 << SIGTTIN)
		| (1 << SIGTTOU)
		| (1 << SIGTSTP)
#endif
		,
};

static void record_pending_signo(цяло sig)
{
	sigaddset(&G.pending_set, sig);
#ако ENABLE_FEATURE_EDITING
	ако (sig != SIGCHLD
	 || (G_traps && G_traps[SIGCHLD] && G_traps[SIGCHLD][0])
	 /* ^^^ if SIGCHLD, interrupt line reading only if it has a trap */
	) {
		bb_got_signal = sig; /* for read_line_input: "we got a signal" */
	}
#endif
#ако ENABLE_HUSH_FAST
	ако (sig == SIGCHLD) {
		G.count_SIGCHLD++;
//bb_error_msg("[%d] SIGCHLD_handler: G.count_SIGCHLD:%d G.handled_SIGCHLD:%d", getpid(), G.count_SIGCHLD, G.handled_SIGCHLD);
	}
#endif
}

static sighandler_t install_sighandler(цяло sig, sighandler_t handler)
{
	struct sigaction old_sa;

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
	върни old_sa.sa_handler;
}
#endif /* !__U_BOOT__ */

#ifndef __U_BOOT__
static void hush_exit(цяло exitcode) NORETURN;

static void restore_ttypgrp_and__exit(void) NORETURN;
static void restore_ttypgrp_and__exit(void)
{
	/* xfunc has failed! die die die */
	/* no EXIT traps, this is an escape hatch! */
	G.exiting = 1;
	hush_exit(xfunc_error_retval);
}

#ако ENABLE_HUSH_JOB

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
static void fflush_and__exit(void) NORETURN;
static void fflush_and__exit(void)
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
static void sigexit(цяло sig) NORETURN;
static void sigexit(цяло sig)
{
	/* Careful: we can end up here after [v]fork. Do not restore
	 * tty pgrp then, only top-level shell process does that */
	ако (G_saved_tty_pgrp && getpid() == G.root_pid) {
		/* Disable all signals: job control, SIGPIPE, etc.
		 * Mostly paranoid measure, to prevent infinite SIGTTOU.
		 */
		sigprocmask_allsigs(SIG_BLOCK);
		tcsetpgrp(G_interactive_fd, G_saved_tty_pgrp);
	}

	/* Not a signal, just exit */
	ако (sig <= 0)
		_exit(- sig);

	kill_myself_with_sig(sig); /* does not return */
}
#иначе

# define disable_restore_tty_pgrp_on_exit() ((void)0)
# define enable_restore_tty_pgrp_on_exit()  ((void)0)

#endif
#endif /* !__U_BOOT__ */

#ifndef __U_BOOT__
static sighandler_t pick_sighandler(unsigned sig)
{
	sighandler_t handler = SIG_DFL;
	ако (sig < sizeof(unsigned)*8) {
		unsigned sigmask = (1 << sig);

#ако ENABLE_HUSH_JOB
		/* is sig fatal? */
		ако (G_fatal_sig_mask & sigmask)
			handler = sigexit;
		иначе
#endif
		/* sig has special handling? */
		ако (G.special_sig_mask & sigmask) {
			handler = record_pending_signo;
			/* TTIN/TTOU/TSTP can't be set to record_pending_signo
			 * in order to ignore them: they will be raised
			 * in an endless loop when we try to do some
			 * terminal ioctls! We do have to _ignore_ these.
			 */
			ако (SPECIAL_JOBSTOP_SIGS & sigmask)
				handler = SIG_IGN;
		}
	}
	върни handler;
}
#endif /* !__U_BOOT__ */

#ifndef __U_BOOT__
/* Restores tty foreground process group, and exits. */
static void hush_exit(цяло exitcode)
{
#ако ENABLE_FEATURE_EDITING_SAVE_ON_EXIT
	save_history(G.line_input_state); /* may be NULL */
#endif

	fflush_all();
	ако (G.exiting <= 0 && G_traps && G_traps[0] && G_traps[0][0]) {
		char *argv[3];
		/* argv[0] is unused */
		argv[1] = xstrdup(G_traps[0]); /* copy, since EXIT trap handler may modify G_traps[0] */
		argv[2] = NULL;
		G.exiting = 1; /* prevent EXIT trap recursion */
		/* Note: G_traps[0] is not cleared!
		 * "trap" will still show it, if executed
		 * in the handler */
		builtin_eval(argv);
	}

#ако ENABLE_FEATURE_CLEAN_UP
	{
		struct variable *cur_var;
		ако (G.cwd != bb_msg_unknown)
			free((char*)G.cwd);
		cur_var = G.top_var;
		докато (cur_var) {
			struct variable *tmp = cur_var;
			ако (!cur_var->max_len)
				free(cur_var->varstr);
			cur_var = cur_var->next;
			free(tmp);
		}
	}
#endif

	fflush_all();
#ако ENABLE_HUSH_JOB
	sigexit(- (exitcode & 0xff));
#иначе
	_exit(exitcode);
#endif
}

//TODO: return a mask of ALL handled sigs?
static цяло check_and_run_traps(void)
{
	цяло last_sig = 0;

	докато (1) {
		цяло sig;

		ако (sigisemptyset(&G.pending_set))
			break;
		sig = 0;
		do {
			sig++;
			ако (sigismember(&G.pending_set, sig)) {
				sigdelset(&G.pending_set, sig);
				goto got_sig;
			}
		} докато (sig < NSIG);
		break;
 got_sig:
#ако ENABLE_HUSH_TRAP
		ако (G_traps && G_traps[sig]) {
			debug_printf_exec("%s: sig:%d handler:'%s'\n", __func__, sig, G.traps[sig]);
			ако (G_traps[sig][0]) {
				/* We have user-defined handler */
				smalluint save_rcode;
				цяло save_pre;
				char *argv[3];
				/* argv[0] is unused */
				argv[1] = xstrdup(G_traps[sig]);
				/* why strdup? trap can modify itself: trap 'trap "echo oops" INT' INT */
				argv[2] = NULL;
				save_pre = G.pre_trap_exitcode;
				G.pre_trap_exitcode = save_rcode = G.last_exitcode;
				builtin_eval(argv);
				free(argv[1]);
				G.pre_trap_exitcode = save_pre;
				G.last_exitcode = save_rcode;
# ако ENABLE_HUSH_FUNCTIONS
				ако (G.return_exitcode >= 0) {
					debug_printf_exec("trap exitcode:%d\n", G.return_exitcode);
					G.last_exitcode = G.return_exitcode;
				}
# endif
				last_sig = sig;
			} /* else: "" trap, ignoring signal */
			continue;
		}
#endif
		/* not a trap: special action */
		switch (sig) {
		case SIGINT:
			debug_printf_exec("%s: sig:%d default SIGINT handler\n", __func__, sig);
			G.flag_SIGINT = 1;
			last_sig = sig;
			break;
#ако ENABLE_HUSH_JOB
		case SIGHUP: {
			/* if (G_interactive_fd) - no need to check, the handler
			 * is only installed if we *are* interactive */
			{
				/* bash compat: "Before exiting, an interactive
				 * shell resends the SIGHUP to all jobs, running
				 * or stopped.  Stopped jobs are sent SIGCONT
				 * to ensure that they receive the SIGHUP."
				 */
				struct pipe *job;
				debug_printf_exec("%s: sig:%d default SIGHUP handler\n", __func__, sig);
				/* bash is observed to signal whole process groups,
				 * not individual processes */
				за (job = G.job_list; job; job = job->next) {
					ако (job->pgrp <= 0)
						continue;
					debug_printf_exec("HUPing pgrp %d\n", job->pgrp);
					ако (kill(- job->pgrp, SIGHUP) == 0)
						kill(- job->pgrp, SIGCONT);
				}
			}
			/* this restores tty pgrp, then kills us with SIGHUP */
			sigexit(SIGHUP);
		}
#endif
#ако ENABLE_HUSH_FAST
		case SIGCHLD:
			debug_printf_exec("%s: sig:%d default SIGCHLD handler\n", __func__, sig);
			G.count_SIGCHLD++;
//bb_error_msg("[%d] check_and_run_traps: G.count_SIGCHLD:%d G.handled_SIGCHLD:%d", getpid(), G.count_SIGCHLD, G.handled_SIGCHLD);
			/* Note:
			 * We don't do 'last_sig = sig' here -> NOT returning this sig.
			 * This simplifies wait builtin a bit.
			 */
			break;
#endif
		default: /* ignored: */
			debug_printf_exec("%s: sig:%d default handling is to ignore\n", __func__, sig);
			/* SIGTERM, SIGQUIT, SIGTTIN, SIGTTOU, SIGTSTP */
			/* Note:
			 * We don't do 'last_sig = sig' here -> NOT returning this sig.
			 * Example: wait is not interrupted by TERM
			 * in interactive shell, because TERM is ignored.
			 */
			break;
		}
	}
	върни last_sig;
}

static const char *get_cwd(цяло force)
{
	ако (force || G.cwd == NULL) {
		/* xrealloc_getcwd_or_warn(arg) calls free(arg),
		 * we must not try to free(bb_msg_unknown) */
		ако (G.cwd == bb_msg_unknown)
			G.cwd = NULL;
		G.cwd = xrealloc_getcwd_or_warn((char *)G.cwd);
		ако (!G.cwd)
			G.cwd = bb_msg_unknown;
	}
	върни G.cwd;
}

#endif /* !__U_BOOT__ */

/*
 * Shell and environment variable support
 */
static struct variable **get_ptr_to_local_var(const char *name)
{
	struct variable **pp;
	struct variable *cur;

	pp = &G.top_var;
	докато ((cur = *pp) != NULL) {
		ако (varcmp(cur->varstr, name) == 0)
			върни pp;
		pp = &cur->next;
	}
	върни NULL;
}

static const char* FAST_FUNC get_local_var_value(const char *name)
{
	struct variable **vpp;

	ако (G.expanded_assignments) {
		char **cpp = G.expanded_assignments;
		докато (*cpp) {
			char *cp = *cpp;
			ако (varcmp(cp, name) == 0)
				върни strchr(cp, '=') + 1;
			cpp++;
		}
	}

	vpp = get_ptr_to_local_var(name);
	ако (vpp)
		върни strchr((*vpp)->varstr, '=') + 1;

#ifndef __U_BOOT__
	ако (strcmp(name, "PPID") == 0)
		върни utoa(G.root_ppid);
#endif /* !__U_BOOT__ */
	// bash compat: UID? EUID?
#ако ENABLE_HUSH_RANDOM_SUPPORT
	ако (strcmp(name, "RANDOM") == 0)
		върни utoa(next_random(&G.random_gen));
#endif
#ако ENABLE_HUSH_LINENO_VAR
	ако (strcmp(name, "LINENO") == 0)
		върни utoa(G.execute_lineno);
#endif
#ако BASH_EPOCH_VARS
	{
		const char *fmt = NULL;
		ако (strcmp(name, "EPOCHSECONDS") == 0)
			fmt = "%llu";
		иначе ако (strcmp(name, "EPOCHREALTIME") == 0)
			fmt = "%llu.%06u";
		ако (fmt) {
			struct timeval tv;
			xgettimeofday(&tv);
			sprintf(G.epoch_buf, fmt, (unsigned long long)tv.tv_sec,
					(unsigned)tv.tv_usec);
			върни G.epoch_buf;
		}
	}
#endif
	върни NULL;
}

#ifndef __U_BOOT__
#ако ENABLE_HUSH_GETOPTS
static void handle_changed_special_names(const char *name)
{
	ако (varcmp(name, "OPTIND") == 0) {
		G.getopt_count = 0;
		върни;
	}
}
#иначе
/* Do not even bother evaluating arguments */
# define handle_changed_special_names(...) ((void)0)
#endif
#иначе /* __U_BOOT__ */
/* Do not even bother evaluating arguments */
# define handle_changed_special_names(...) ((void)0)
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
static цяло set_local_var(char *str, unsigned flags)
#иначе /* __U_BOOT__ */
цяло set_local_var_modern(char *str, цяло flags)
#endif /* __U_BOOT__ */
{
	struct variable **cur_pp;
	struct variable *cur;
	char *free_me = NULL;
	char *eq_sign;
	цяло name_len;
	цяло retval;
#ifndef __U_BOOT__
	unsigned local_lvl = (flags >> SETFLAG_VARLVL_SHIFT);
#endif /* !__U_BOOT__ */

	eq_sign = strchr(str, '=');
	ако (HUSH_DEBUG && !eq_sign)
		bb_simple_error_msg_and_die("BUG in setvar");

	name_len = eq_sign - str + 1; /* including '=' */
	cur_pp = &G.top_var;
	докато ((cur = *cur_pp) != NULL) {
		ако (strncmp(cur->varstr, str, name_len) != 0) {
			cur_pp = &cur->next;
			continue;
		}

#ifndef __U_BOOT__
		/* We found an existing var with this name */
		ако (cur->flg_read_only) {
			bb_error_msg("%s: readonly variable", str);
			free(str);
//NOTE: in bash, assignment in "export READONLY_VAR=Z" fails, and sets $?=1,
//but export per se succeeds (does put the var in env). We don't mimic that.
			върни -1;
		}
		ако (flags & SETFLAG_UNEXPORT) { // && cur->flg_export ?
			debug_printf_env("%s: unsetenv '%s'\n", __func__, str);
			*eq_sign = '\0';
			unsetenv(str);
			*eq_sign = '=';
		}
		ако (cur->var_nest_level < local_lvl) {
			/* bash 3.2.33(1) and exported vars:
			 * # export z=z
			 * # f() { local z=a; env | grep ^z; }
			 * # f
			 * z=a
			 * # env | grep ^z
			 * z=z
			 */
			ако (cur->flg_export)
				flags |= SETFLAG_EXPORT;
			/* New variable is local ("local VAR=VAL" or
			 * "VAR=VAL cmd")
			 * and existing one is global, or local
			 * on a lower level that new one.
			 * Remove it from global variable list:
			 */
			*cur_pp = cur->next;
			ако (G.shadowed_vars_pp) {
				/* Save in "shadowed" list */
				debug_printf_env("shadowing %s'%s'/%u by '%s'/%u\n",
					cur->flg_export ? "exported " : "",
					cur->varstr, cur->var_nest_level, str, local_lvl
				);
				cur->next = *G.shadowed_vars_pp;
				*G.shadowed_vars_pp = cur;
			} иначе {
				/* Came from pseudo_exec_argv(), no need to save: delete it */
				debug_printf_env("shadow-deleting %s'%s'/%u by '%s'/%u\n",
					cur->flg_export ? "exported " : "",
					cur->varstr, cur->var_nest_level, str, local_lvl
				);
				ако (cur->max_len == 0) /* allocated "VAR=VAL"? */
					free_me = cur->varstr; /* then free it later */
				free(cur);
			}
			break;
		}
#endif /* !__U_BOOT__ */

		ако (strcmp(cur->varstr + name_len, eq_sign + 1) == 0) {
			debug_printf_env("assignement '%s' does not change anything\n", str);
 free_and_exp:
			free(str);
			goto exp;
		}

		/* Replace the value in the found "struct variable" */
		ако (cur->max_len != 0) {
			ако (cur->max_len >= strnlen(str, cur->max_len + 1)) {
				/* This one is from startup env, reuse space */
				debug_printf_env("reusing startup env for '%s'\n", str);
				strcpy(cur->varstr, str);
				goto free_and_exp;
			}
			/* Can't reuse */
			cur->max_len = 0;
			goto set_str_and_exp;
		}
		/* max_len == 0 signifies "malloced" var, which we can
		 * (and have to) free. But we can't free(cur->varstr) here:
		 * if cur->flg_export is 1, it is in the environment.
		 * We should either unsetenv+free, or wait until putenv,
		 * then putenv(new)+free(old).
		 */
		free_me = cur->varstr;
		goto set_str_and_exp;
	}

	/* Not found or shadowed - create new variable struct */
#ifndef __U_BOOT__
	debug_printf_env("%s: alloc new var '%s'/%u\n", __func__, str, local_lvl);
#иначе /* __U_BOOT__ */
	debug_printf_env("%s: alloc new var '%s'\n", __func__, str);
#endif /* __U_BOOT__ */
	cur = xzalloc(sizeof(*cur));
#ifndef __U_BOOT__
	cur->var_nest_level = local_lvl;
#endif /* !__U_BOOT__ */
	cur->next = *cur_pp;
	*cur_pp = cur;

 set_str_and_exp:
	cur->varstr = str;
 exp:
#ifndef __U_BOOT__
#ако !BB_MMU || ENABLE_HUSH_READONLY
	ако (flags & SETFLAG_MAKE_RO) {
		cur->flg_read_only = 1;
	}
#endif
	ако (flags & SETFLAG_EXPORT)
		cur->flg_export = 1;
#endif /* !__U_BOOT__ */
	retval = 0;
#ifndef __U_BOOT__
	ако (cur->flg_export) {
		ако (flags & SETFLAG_UNEXPORT) {
			cur->flg_export = 0;
			/* unsetenv was already done */
		} иначе {
			debug_printf_env("%s: putenv '%s'/%u\n", __func__, cur->varstr, cur->var_nest_level);
			retval = putenv(cur->varstr);
			/* fall through to "free(free_me)" -
			 * only now we can free old exported malloced string
			 */
		}
	}
#endif /* !__U_BOOT__ */
	free(free_me);

	handle_changed_special_names(cur->varstr);

	върни retval;
}

#ifndef __U_BOOT__
static цяло set_local_var0(char *str)
{
	върни set_local_var(str, 0);
}

static void FAST_FUNC set_local_var_from_halves(const char *name, const char *val)
{
	char *var = xasprintf("%s=%s", name, val);
	set_local_var0(var);
}

/* Used at startup and after each cd */
static void set_pwd_var(unsigned flag)
{
	set_local_var(xasprintf("PWD=%s", get_cwd(/*force:*/ 1)), flag);
}
#endif /* !__U_BOOT__ */

#ако ENABLE_HUSH_UNSET || ENABLE_HUSH_GETOPTS
static цяло unset_local_var(const char *name)
{
	struct variable *cur;
	struct variable **cur_pp;

	cur_pp = &G.top_var;
	докато ((cur = *cur_pp) != NULL) {
		ако (varcmp(cur->varstr, name) == 0) {
			ако (cur->flg_read_only) {
				bb_error_msg("%s: readonly variable", name);
				върни EXIT_FAILURE;
			}

			*cur_pp = cur->next;
			debug_printf_env("%s: unsetenv '%s'\n", __func__, cur->varstr);
			bb_unsetenv(cur->varstr);
			ако (!cur->max_len)
				free(cur->varstr);
			free(cur);

			break;
		}
		cur_pp = &cur->next;
	}

	/* Handle "unset LINENO" et al even if did not find the variable to unset */
	handle_changed_special_names(name);

	върни EXIT_SUCCESS;
}
#endif

#ifndef __U_BOOT__
/*
 * Helpers for "var1=val1 var2=val2 cmd" feature
 */
static void add_vars(struct variable *var)
{
	struct variable *next;

	докато (var) {
		next = var->next;
		var->next = G.top_var;
		G.top_var = var;
		ако (var->flg_export) {
			debug_printf_env("%s: restoring exported '%s'/%u\n", __func__, var->varstr, var->var_nest_level);
			putenv(var->varstr);
		} иначе {
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
static void set_vars_and_save_old(char **strings)
{
	char **s;

	ако (!strings)
		върни;

	s = strings;
	докато (*s) {
		struct variable *var_p;
		struct variable **var_pp;
		char *eq;

		eq = strchr(*s, '=');
		ако (HUSH_DEBUG && !eq)
			bb_simple_error_msg_and_die("BUG in varexp4");
		var_pp = get_ptr_to_local_var(*s);
		ако (var_pp) {
			var_p = *var_pp;
			ако (var_p->flg_read_only) {
				char **p;
				bb_error_msg("%s: readonly variable", *s);
				/*
				 * "VAR=V BLTIN" unsets VARs after BLTIN completes.
				 * If VAR is readonly, leaving it in the list
				 * after asssignment error (msg above)
				 * causes doubled error message later, on unset.
				 */
				debug_printf_env("removing/freeing '%s' element\n", *s);
				free(*s);
				p = s;
				do { *p = p[1]; p++; } докато (*p);
				goto next;
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
	free(strings);
}

/*
 * Unicode helper
 */
static void reinit_unicode_for_hush(void)
{
	/* Unicode support should be activated even if LANG is set
	 * _during_ shell execution, not only if it was set when
	 * shell was started. Therefore, re-check LANG every time:
	 */
	ако (ENABLE_FEATURE_CHECK_UNICODE_IN_ENV
	 || ENABLE_UNICODE_USING_LOCALE
	) {
		const char *s = get_local_var_value("LC_ALL");
		ако (!s) s = get_local_var_value("LC_CTYPE");
		ако (!s) s = get_local_var_value("LANG");
		reinit_unicode(s);
	}
}

#endif /* !__U_BOOT__ */
/*
 * in_str support (strings, and "strings" read from files).
 */

#ако ENABLE_HUSH_INTERACTIVE
#ifndef __U_BOOT__
/* To test correct lineedit/interactive behavior, type from command line:
 *	echo $P\
 *	\
 *	AT\
 *	H\
 *	\
 * It exercises a lot of corner cases.
 */
static const char *setup_prompt_string(void)
{
	const char *prompt_str;

	debug_printf_prompt("%s promptmode:%d\n", __func__, G.promptmode);

# ако ENABLE_FEATURE_EDITING_FANCY_PROMPT
	prompt_str = get_local_var_value(G.promptmode == 0 ? "PS1" : "PS2");
	ако (!prompt_str)
		prompt_str = "";
# иначе
	prompt_str = "> "; /* if PS2, else... */
	ако (G.promptmode == 0) { /* PS1 */
		/* No fancy prompts supported, (re)generate "CURDIR $ " by hand */
		free(G.PS1);
		/* bash uses $PWD value, even if it is set by user.
		 * It uses current dir only if PWD is unset.
		 * We always use current dir. */
		prompt_str = G.PS1 = xasprintf("%s %c ", get_cwd(0), (geteuid() != 0) ? '$' : '#');
	}
# endif
	debug_printf("prompt_str '%s'\n", prompt_str);
	върни prompt_str;
}
#endif /* !__U_BOOT__ */

#ifndef __U_BOOT__
static цяло get_user_input(struct in_str *i)
#иначе /* __U_BOOT__ */
static void get_user_input(struct in_str *i)
#endif /* __U_BOOT__ */
{
#ifndef __U_BOOT__
# ако ENABLE_FEATURE_EDITING
	/* In EDITING case, this function reads next input line,
	 * saves it in i->p, then returns 1st char of it.
	 */
	цяло r;
	const char *prompt_str;

	prompt_str = setup_prompt_string();
	за (;;) {
		reinit_unicode_for_hush();
		G.flag_SIGINT = 0;

		bb_got_signal = 0;
		ако (!sigisemptyset(&G.pending_set)) {
			/* Whoops, already got a signal, do not call read_line_input */
			bb_got_signal = r = -1;
		} иначе {
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
			r = read_line_input(G.line_input_state, prompt_str,
				G.user_input_buf, CONFIG_FEATURE_EDITING_MAX_LEN-1
			);
			/* read_line_input intercepts ^C, "convert" it to SIGINT */
			ако (r == 0)
				raise(SIGINT);
		}
		/* bash prints ^C (before running a trap, if any)
		 * both on keyboard ^C and on real SIGINT (non-kbd generated).
		 */
		ако (sigismember(&G.pending_set, SIGINT)) {
			write(STDOUT_FILENO, "^C\n", 3);
			G.last_exitcode = 128 | SIGINT;
		}
		check_and_run_traps();
		ако (r == 0) /* keyboard ^C? */
			continue; /* go back, read another input line */
		ако (r > 0) /* normal input? (no ^C, no ^D, no signals) */
			break;
		ако (!bb_got_signal) {
			/* r < 0: ^D/EOF/error detected (but not signal) */
			/* ^D on interactive input goes to next line before exiting: */
			write(STDOUT_FILENO, "\n", 1);
			i->p = NULL;
			i->peek_buf[0] = r = EOF;
			върни r;
		}
		/* it was a signal: go back, read another input line */
	}
	i->p = G.user_input_buf;
	върни (unsigned char)*i->p++;
# иначе
	/* In !EDITING case, this function gets called for every char.
	 * Buffering happens deeper in the call chain, in hfgetc(i->file).
	 */
	цяло r;

	за (;;) {
		G.flag_SIGINT = 0;
		ако (i->last_char == '\0' || i->last_char == '\n') {
			const char *prompt_str = setup_prompt_string();
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
		ако (r != '\0' && !G.flag_SIGINT)
			break;
		ако (G.flag_SIGINT) {
			/* ^C or SIGINT: repeat */
			/* bash prints ^C even on real SIGINT (non-kbd generated) */
			/* kernel prints "^C" itself, just print newline: */
			write(STDOUT_FILENO, "\n", 1);
			G.last_exitcode = 128 | SIGINT;
		}
	}
	върни r;
# endif
#иначе /* __U_BOOT__ */
	цяло n;
	цяло promptme;
	static char the_command[CONFIG_SYS_CBSIZE + 1];

	bootretry_reset_cmd_timeout();
	promptme = 1;
	n = u_boot_cli_readline(i);

# ifdef CONFIG_BOOT_RETRY_TIME
	ако (n == -2) {
		puts("\nTimeout waiting for command\n");
#  ifdef CONFIG_RESET_TO_RETRY
		do_reset(NULL, 0, 0, NULL);
#  elif IS_ENABLED(CONFIG_RETRY_BOOTCMD)
	strcpy(console_buffer, "run bootcmd\n");
# иначе
#	error "This only works with CONFIG_RESET_TO_RETRY or CONFIG_BOOT_RETRY_COMMAND enabled"
#  endif
	}
# endif
	ако (n == -1 ) {
		G.flag_repeat = 0;
		promptme = 0;
	}
	n = strlen(console_buffer);
	console_buffer[n] = '\n';
	console_buffer[n+1]= '\0';
	ако (had_ctrlc())
		G.flag_repeat = 0;
	clear_ctrlc();
	G.do_repeat = 0;
#ifndef __U_BOOT__
	ако (G.promptmode == 1) {
#иначе /* __U_BOOT__ */
	ако (!G.promptmode) {
#endif /* __U_BOOT__ */
		ако (console_buffer[0] == '\n'&& G.flag_repeat == 0) {
			strcpy(the_command, console_buffer);
		}
		иначе {
			ако (console_buffer[0] != '\n') {
				strcpy(the_command, console_buffer);
				G.flag_repeat = 1;
			}
			иначе {
				G.do_repeat = 1;
			}
		}
		i->p = the_command;
	}
	иначе {
		ако (console_buffer[0] != '\n') {
			ако (strlen(the_command) + strlen(console_buffer)
				< CONFIG_SYS_CBSIZE) {
				n = strlen(the_command);
#ifdef __U_BOOT__
				/*
				 * To avoid writing to bad places, we check if
				 * n is greater than 0.
				 * This bug was found by Harald Seiler.
				 */
				ако (n > 0)
					the_command[n-1] = ' ';
				strcpy(&the_command[n], console_buffer);
#иначе /* !__U_BOOT__ */
			the_command[n-1] = ' ';
			strcpy(&the_command[n], console_buffer);
#endif /* !__U_BOOT__ */
				}
				иначе {
					the_command[0] = '\n';
					the_command[1] = '\0';
					G.flag_repeat = 0;
				}
		}
		ако (promptme == 0) {
			the_command[0] = '\n';
			the_command[1] = '\0';
		}
		i->p = console_buffer;
	}
#endif /* __U_BOOT__ */
}
/* This is the magic location that prints prompts
 * and gets data back from the user */
static цяло fgetc_interactive(struct in_str *i)
{
	цяло ch;
#ifndef __U_BOOT__
	/* If it's interactive stdin, get new line. */
	ако (G_interactive_fd && i->file == G.HFILE_stdin) {
#endif /* !__U_BOOT__ */
#ifndef __U_BOOT__
		/* Returns first char (or EOF), the rest is in i->p[] */
		ch = get_user_input(i);
#иначе /* __U_BOOT__ */
		/* Avoid garbage value and make clang happy. */
		ch = 0;
		/*
		 * get_user_input() does not return anything when used in
		 * U-Boot.
		 * So, we need to take the read character from i->p[].
		 */
		get_user_input(i);
		ако (i->p && *i->p) {
			ch = *i->p++;
		}
#endif /* __U_BOOT__ */
		G.promptmode = 1; /* PS2 */
		debug_printf_prompt("%s promptmode=%d\n", __func__, G.promptmode);
#ifndef __U_BOOT__
	} иначе {
		/* Not stdin: script file, sourced file, etc */
		do ch = hfgetc(i->file); докато (ch == '\0');
	}
#endif /* !__U_BOOT__ */
	върни ch;
}
#иначе  /* !INTERACTIVE */
#ifndef __U_BOOT__
static ALWAYS_INLINE цяло fgetc_interactive(struct in_str *i)
{
	цяло ch;
	do ch = hfgetc(i->file); докато (ch == '\0');
	върни ch;
}
#endif /* !__U_BOOT__ */
#endif  /* !INTERACTIVE */

static цяло i_getch(struct in_str *i)
{
	цяло ch;

#ifndef __U_BOOT__
	ако (!i->file) {
		/* string-based in_str */
		ch = (unsigned char)*i->p;
		ако (ch != '\0') {
			i->p++;
			i->last_char = ch;
#ако ENABLE_HUSH_LINENO_VAR
			ако (ch == '\n') {
				G.parse_lineno++;
				debug_printf_parse("G.parse_lineno++ = %u\n", G.parse_lineno);
			}
#endif
			върни ch;
		}
		върни EOF;
	}

#endif /* !__U_BOOT__ */
	/* FILE-based in_str */

#ако ENABLE_FEATURE_EDITING
	/* This can be stdin, check line editing char[] buffer */
	ако (i->p && *i->p != '\0') {
		ch = (unsigned char)*i->p++;
		goto out;
#ifndef __U_BOOT__
	}
#иначе /* __U_BOOT__ */
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
	} иначе ако (i->p && *i->p == '\0' && G.run_command_flags){
		върни EOF;
	}
#endif /* __U_BOOT__ */
#endif
#ifndef __U_BOOT__
	/* peek_buf[] is an int array, not char. Can contain EOF. */
	ch = i->peek_buf[0];
	ако (ch != 0) {
		цяло ch2 = i->peek_buf[1];
		i->peek_buf[0] = ch2;
		ако (ch2 == 0) /* very likely, avoid redundant write */
			goto out;
		i->peek_buf[1] = 0;
		goto out;
	}

#endif /* !__U_BOOT__ */
	ch = fgetc_interactive(i);
 out:
	debug_printf("file_get: got '%c' %d\n", ch, ch);
	i->last_char = ch;
#ако ENABLE_HUSH_LINENO_VAR
	ако (ch == '\n') {
		G.parse_lineno++;
		debug_printf_parse("G.parse_lineno++ = %u\n", G.parse_lineno);
	}
#endif
	върни ch;
}

static цяло i_peek(struct in_str *i)
{
#ifndef __U_BOOT__
	цяло ch;

	ако (!i->file) {
		/* string-based in_str */
		/* Doesn't report EOF on NUL. None of the callers care. */
		върни (unsigned char)*i->p;
	}

	/* FILE-based in_str */

#ако ENABLE_FEATURE_EDITING && ENABLE_HUSH_INTERACTIVE
	/* This can be stdin, check line editing char[] buffer */
	ако (i->p && *i->p != '\0')
		върни (unsigned char)*i->p;
#endif
	/* peek_buf[] is an int array, not char. Can contain EOF. */
	ch = i->peek_buf[0];
	ако (ch != 0)
		върни ch;

	/* Need to get a new char */
	ch = fgetc_interactive(i);
	debug_printf("file_peek: got '%c' %d\n", ch, ch);

	/* Save it by either rolling back line editing buffer, or in i->peek_buf[0] */
#ако ENABLE_FEATURE_EDITING && ENABLE_HUSH_INTERACTIVE
	ако (i->p) {
		i->p -= 1;
		върни ch;
	}
#endif
	i->peek_buf[0] = ch;
	/*i->peek_buf[1] = 0; - already is */
	върни ch;
#иначе /* __U_BOOT__ */
	/* string-based in_str */
	/* Doesn't report EOF on NUL. None of the callers care. */
	върни (unsigned char)*i->p;
#endif /* __U_BOOT__ */
}

/* Only ever called if i_peek() was called, and did not return EOF.
 * IOW: we know the previous peek saw an ordinary char, not EOF, not NUL,
 * not end-of-line. Therefore we never need to read a new editing line here.
 */
static цяло i_peek2(struct in_str *i)
{
#ifndef __U_BOOT__
	цяло ch;
#endif /* !__U_BOOT__ */

	/* There are two cases when i->p[] buffer exists.
	 * (1) it's a string in_str.
	 * (2) It's a file, and we have a saved line editing buffer.
	 * In both cases, we know that i->p[0] exists and not NUL, and
	 * the peek2 result is in i->p[1].
	 */
	ако (i->p)
		върни (unsigned char)i->p[1];

#ifndef __U_BOOT__
	/* Now we know it is a file-based in_str. */

	/* peek_buf[] is an int array, not char. Can contain EOF. */
	/* Is there 2nd char? */
	ch = i->peek_buf[1];
	ако (ch == 0) {
		/* We did not read it yet, get it now */
		do ch = hfgetc(i->file); докато (ch == '\0');
		i->peek_buf[1] = ch;
	}

	debug_printf("file_peek2: got '%c' %d\n", ch, ch);
	върни ch;
#иначе
	върни 0;
#endif /* __U_BOOT__ */
}

static цяло i_getch_and_eat_bkslash_nl(struct in_str *input)
{
	за (;;) {
		цяло ch, ch2;

		ch = i_getch(input);
		ако (ch != '\\')
			върни ch;
		ch2 = i_peek(input);
		ако (ch2 != '\n')
			върни ch;
		/* backslash+newline, skip it */
		i_getch(input);
	}
}

/* Note: this function _eats_ \<newline> pairs, safe to use plain
 * i_getch() after it instead of i_getch_and_eat_bkslash_nl().
 */
static цяло i_peek_and_eat_bkslash_nl(struct in_str *input)
{
	за (;;) {
		цяло ch, ch2;

		ch = i_peek(input);
		ако (ch != '\\')
			върни ch;
		ch2 = i_peek2(input);
		ако (ch2 != '\n')
			върни ch;
		/* backslash+newline, skip it */
		i_getch(input);
		i_getch(input);
	}
}

#ifndef __U_BOOT__
static void setup_file_in_str(struct in_str *i, HFILE *fp)
#иначе /* __U_BOOT__ */
static void setup_file_in_str(struct in_str *i)
#endif /* __U_BOOT__ */
{
	memset(i, 0, sizeof(*i));
#ifndef __U_BOOT__
	i->file = fp;
	/* i->p = NULL; */
#endif /* !__U_BOOT__ */
}

static void setup_string_in_str(struct in_str *i, const char *s)
{
	memset(i, 0, sizeof(*i));
	/*i->file = NULL */;
	i->p = s;
}

/*
 * o_string support
 */
#define B_CHUNK  (32 * sizeof(char*))

static void o_reset_to_empty_unquoted(o_string *o)
{
	o->length = 0;
	o->has_quoted_part = 0;
	ако (o->data)
		o->data[0] = '\0';
}

static void o_free_and_set_NULL(o_string *o)
{
	free(o->data);
	memset(o, 0, sizeof(*o));
}

static ALWAYS_INLINE void o_free(o_string *o)
{
	free(o->data);
}

static void o_grow_by(o_string *o, цяло len)
{
	ако (o->length + len > o->maxlen) {
		o->maxlen += (2 * len) | (B_CHUNK-1);
		o->data = xrealloc(o->data, 1 + o->maxlen);
	}
}

static void o_addchr(o_string *o, цяло ch)
{
	debug_printf("o_addchr: '%c' o->length=%d o=%p\n", ch, o->length, o);
	ако (o->length < o->maxlen) {
		/* likely. avoid o_grow_by() call */
 add:
		o->data[o->length] = ch;
		o->length++;
		o->data[o->length] = '\0';
		върни;
	}
	o_grow_by(o, 1);
	goto add;
}

#ако 0
/* Valid only if we know o_string is not empty */
static void o_delchr(o_string *o)
{
	o->length--;
	o->data[o->length] = '\0';
}
#endif

static void o_addblock(o_string *o, const char *str, цяло len)
{
	o_grow_by(o, len);
	((char*)mempcpy(&o->data[o->length], str, len))[0] = '\0';
	o->length += len;
}

static void o_addstr(o_string *o, const char *str)
{
	o_addblock(o, str, strlen(str));
}

#ifndef __U_BOOT__
static void o_addstr_with_NUL(o_string *o, const char *str)
{
	o_addblock(o, str, strlen(str) + 1);
}
#endif /* !__U_BOOT__ */

#ако !BB_MMU
static void nommu_addchr(o_string *o, цяло ch)
{
	ако (o)
		o_addchr(o, ch);
}
#иначе
# define nommu_addchr(o, str) ((void)0)
#endif

#ifndef __U_BOOT__
#ако ENABLE_HUSH_MODE_X
static void x_mode_addchr(цяло ch)
{
	o_addchr(&G.x_mode_buf, ch);
}
static void x_mode_addstr(const char *str)
{
	o_addstr(&G.x_mode_buf, str);
}
static void x_mode_addblock(const char *str, цяло len)
{
	o_addblock(&G.x_mode_buf, str, len);
}
static void x_mode_prefix(void)
{
	цяло n = G.x_mode_depth;
	do x_mode_addchr('+'); докато (--n >= 0);
}
static void x_mode_flush(void)
{
	цяло len = G.x_mode_buf.length;
	ако (len <= 0)
		върни;
	ако (G.x_mode_fd > 0) {
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

#ако ENABLE_HUSH_BRACE_EXPANSION
# define MAYBE_BRACES "{}"
#иначе
# define MAYBE_BRACES ""
#endif

/* My analysis of quoting semantics tells me that state information
 * is associated with a destination, not a source.
 */
static void o_addqchr(o_string *o, цяло ch)
{
	цяло sz = 1;
	/* '-' is included because of this case:
	 * >filename0 >filename1 >filename9; v='-'; echo filename[0"$v"9]
	 */
	char *found = strchr("*?[-\\" MAYBE_BRACES, ch);
	ако (found)
		sz++;
	o_grow_by(o, sz);
	ако (found) {
		o->data[o->length] = '\\';
		o->length++;
	}
	o->data[o->length] = ch;
	o->length++;
	o->data[o->length] = '\0';
}

static void o_addQchr(o_string *o, цяло ch)
{
	цяло sz = 1;
	ако ((o->o_expflags & EXP_FLAG_ESC_GLOB_CHARS)
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

static void o_addqblock(o_string *o, const char *str, цяло len)
{
	докато (len) {
		char ch;
		цяло sz;
		цяло ordinary_cnt = strcspn(str, "*?[-\\" MAYBE_BRACES);
		ако (ordinary_cnt > len) /* paranoia */
			ordinary_cnt = len;
		o_addblock(o, str, ordinary_cnt);
		ако (ordinary_cnt == len)
			върни; /* NUL is already added by o_addblock */
		str += ordinary_cnt;
		len -= ordinary_cnt + 1; /* we are processing + 1 char below */

		ch = *str++;
		sz = 1;
		ако (ch) { /* it is necessarily one of "*?[-\\" MAYBE_BRACES */
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

static void o_addQblock(o_string *o, const char *str, цяло len)
{
	ако (!(o->o_expflags & EXP_FLAG_ESC_GLOB_CHARS)) {
		o_addblock(o, str, len);
		върни;
	}
	o_addqblock(o, str, len);
}

static void o_addQstr(o_string *o, const char *str)
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
#ако DEBUG_EXPAND || DEBUG_GLOB
static void debug_print_list(const char *prefix, o_string *o, цяло n)
{
	char **list = (char**)o->data;
	цяло string_start = ((n + 0xf) & ~0xf) * sizeof(list[0]);
	цяло i = 0;

	indent();
	fdprintf(2, "%s: list:%p n:%d string_start:%d length:%d maxlen:%d glob:%d quoted:%d escape:%d\n",
			prefix, list, n, string_start, o->length, o->maxlen,
			!!(o->o_expflags & EXP_FLAG_GLOB),
			o->has_quoted_part,
			!!(o->o_expflags & EXP_FLAG_ESC_GLOB_CHARS));
	докато (i < n) {
		indent();
		fdprintf(2, " list[%d]=%d '%s' %p\n", i, (цяло)(uintptr_t)list[i],
				o->data + (цяло)(uintptr_t)list[i] + string_start,
				o->data + (цяло)(uintptr_t)list[i] + string_start);
		i++;
	}
	ако (n) {
		const char *p = o->data + (цяло)(uintptr_t)list[n - 1] + string_start;
		indent();
#ifndef __U_BOOT__
		fdprintf(2, " total_sz:%ld\n", (long)((p + strlen(p) + 1) - o->data));
#иначе /* __U_BOOT__ */
		printf(" total_sz:%ld\n", (long)((p + strlen(p) + 1) - o->data));
#endif /* __U_BOOT__ */
	}
}
#иначе
# define debug_print_list(prefix, o, n) ((void)0)
#endif

/* n = o_save_ptr_helper(str, n) "starts new string" by storing an index value
 * in list[n] so that it points past last stored byte so far.
 * It returns n+1. */
static цяло o_save_ptr_helper(o_string *o, цяло n)
{
	char **list = (char**)o->data;
	цяло string_start;
	цяло string_len;

	ако (!o->has_empty_slot) {
		string_start = ((n + 0xf) & ~0xf) * sizeof(list[0]);
		string_len = o->length - string_start;
		ако (!(n & 0xf)) { /* 0, 0x10, 0x20...? */
			debug_printf_list("list[%d]=%d string_start=%d (growing)\n", n, string_len, string_start);
			/* list[n] points to string_start, make space for 16 more pointers */
			o->maxlen += 0x10 * sizeof(list[0]);
			o->data = xrealloc(o->data, o->maxlen + 1);
			list = (char**)o->data;
			memmove(list + n + 0x10, list + n, string_len);
			/*
			 * expand_on_ifs() has a "previous argv[] ends in IFS?"
			 * check. (grep for -prev-ifs-check-).
			 * Ensure that argv[-1][last] is not garbage
			 * but zero bytes, to save index check there.
			 */
			list[n + 0x10 - 1] = 0;
			o->length += 0x10 * sizeof(list[0]);
		} иначе {
			debug_printf_list("list[%d]=%d string_start=%d\n",
					n, string_len, string_start);
		}
	} иначе {
		/* We have empty slot at list[n], reuse without growth */
		string_start = ((n+1 + 0xf) & ~0xf) * sizeof(list[0]); /* NB: n+1! */
		string_len = o->length - string_start;
		debug_printf_list("list[%d]=%d string_start=%d (empty slot)\n",
				n, string_len, string_start);
		o->has_empty_slot = 0;
	}
	o->has_quoted_part = 0;
	list[n] = (char*)(uintptr_t)string_len;
	върни n + 1;
}

/* "What was our last o_save_ptr'ed position (byte offset relative o->data)?" */
static цяло o_get_last_ptr(o_string *o, цяло n)
{
	char **list = (char**)o->data;
	цяло string_start = ((n + 0xf) & ~0xf) * sizeof(list[0]);

	върни ((цяло)(uintptr_t)list[n-1]) + string_start;
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

#ако ENABLE_HUSH_BRACE_EXPANSION
/* There in a GNU extension, GLOB_BRACE, but it is not usable:
 * first, it processes even {a} (no commas), second,
 * I didn't manage to make it return strings when they don't match
 * existing files. Need to re-implement it.
 */

/* Helper */
static цяло glob_needed(const char *s)
{
	докато (*s) {
		ако (*s == '\\') {
			ако (!s[1])
				върни 0;
			s += 2;
			continue;
		}
		ако (*s == '*' || *s == '[' || *s == '?' || *s == '{')
			върни 1;
		s++;
	}
	върни 0;
}
/* Return pointer to next closing brace or to comma */
static const char *next_brace_sub(const char *cp)
{
	unsigned depth = 0;
	cp++;
	докато (*cp != '\0') {
		ако (*cp == '\\') {
			ако (*++cp == '\0')
				break;
			cp++;
			continue;
		}
		ако ((*cp == '}' && depth-- == 0) || (*cp == ',' && depth == 0))
			break;
		ако (*cp++ == '{')
			depth++;
	}

	върни *cp != '\0' ? cp : NULL;
}
/* Recursive brace globber. Note: may garble pattern[]. */
static цяло glob_brace(char *pattern, o_string *o, цяло n)
{
	char *new_pattern_buf;
	const char *begin;
	const char *next;
	const char *rest;
	const char *p;
	size_t rest_len;

	debug_printf_glob("glob_brace('%s')\n", pattern);

	begin = pattern;
	докато (1) {
		ако (*begin == '\0')
			goto simple_glob;
		ако (*begin == '{') {
			/* Find the first sub-pattern and at the same time
			 * find the rest after the closing brace */
			next = next_brace_sub(begin);
			ако (next == NULL) {
				/* An illegal expression */
				goto simple_glob;
			}
			ако (*next == '}') {
				/* "{abc}" with no commas - illegal
				 * brace expr, disregard and skip it */
				begin = next + 1;
				continue;
			}
			break;
		}
		ако (*begin == '\\' && begin[1] != '\0')
			begin++;
		begin++;
	}
	debug_printf_glob("begin:%s\n", begin);
	debug_printf_glob("next:%s\n", next);

	/* Now find the end of the whole brace expression */
	rest = next;
	докато (*rest != '}') {
		rest = next_brace_sub(rest);
		ако (rest == NULL) {
			/* An illegal expression */
			goto simple_glob;
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
	докато (1) {
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
		ако (*next == '}') {
			/* We saw the last entry */
			break;
		}
		p = next + 1;
		next = next_brace_sub(next);
	}
	free(new_pattern_buf);
	върни n;

 simple_glob:
	{
		цяло gr;
		glob_t globdata;

		memset(&globdata, 0, sizeof(globdata));
		gr = glob(pattern, 0, NULL, &globdata);
		debug_printf_glob("glob('%s'):%d\n", pattern, gr);
		ако (gr != 0) {
			ако (gr == GLOB_NOMATCH) {
				globfree(&globdata);
				/* NB: garbles parameter */
				unbackslash(pattern);
				o_addstr_with_NUL(o, pattern);
				debug_printf_glob("glob pattern '%s' is literal\n", pattern);
				върни o_save_ptr_helper(o, n);
			}
			ако (gr == GLOB_NOSPACE)
				bb_die_memory_exhausted();
			/* GLOB_ABORTED? Only happens with GLOB_ERR flag,
			 * but we didn't specify it. Paranoia again. */
			bb_error_msg_and_die("glob error %d on '%s'", gr, pattern);
		}
		ако (globdata.gl_pathv && globdata.gl_pathv[0]) {
			char **argv = globdata.gl_pathv;
			докато (1) {
				o_addstr_with_NUL(o, *argv);
				n = o_save_ptr_helper(o, n);
				argv++;
				ако (!*argv)
					break;
			}
		}
		globfree(&globdata);
	}
	върни n;
}
/* Performs globbing on last list[],
 * saving each result as a new list[].
 */
static цяло perform_glob(o_string *o, цяло n)
{
	char *pattern, *copy;

	debug_printf_glob("start perform_glob: n:%d o->data:%p\n", n, o->data);
	ако (!o->data)
		върни o_save_ptr_helper(o, n);
	pattern = o->data + o_get_last_ptr(o, n);
	debug_printf_glob("glob pattern '%s'\n", pattern);
	ако (!glob_needed(pattern)) {
		/* unbackslash last string in o in place, fix length */
		o->length = unbackslash(pattern) - o->data;
		debug_printf_glob("glob pattern '%s' is literal\n", pattern);
		върни o_save_ptr_helper(o, n);
	}

	copy = xstrdup(pattern);
	/* "forget" pattern in o */
	o->length = pattern - o->data;
	n = glob_brace(copy, o, n);
	free(copy);
	ако (DEBUG_GLOB)
		debug_print_list("perform_glob returning", o, n);
	върни n;
}

#иначе /* !HUSH_BRACE_EXPANSION */

/* Helper */
static цяло glob_needed(const char *s)
{
	докато (*s) {
		ако (*s == '\\') {
			ако (!s[1])
				върни 0;
			s += 2;
			continue;
		}
		ако (*s == '*' || *s == '[' || *s == '?')
			върни 1;
		s++;
	}
	върни 0;
}
/* Performs globbing on last list[],
 * saving each result as a new list[].
 */
static цяло perform_glob(o_string *o, цяло n)
{
#ifndef __U_BOOT__
	glob_t globdata;
	цяло gr;
#endif /* __U_BOOT__ */
	char *pattern;

	debug_printf_glob("start perform_glob: n:%d o->data:%p\n", n, o->data);
	ако (!o->data)
		върни o_save_ptr_helper(o, n);
	pattern = o->data + o_get_last_ptr(o, n);
	debug_printf_glob("glob pattern '%s'\n", pattern);
	ако (!glob_needed(pattern)) {
#ifndef __U_BOOT__
 literal:
#endif /* __U_BOOT__ */
		/* unbackslash last string in o in place, fix length */
		o->length = unbackslash(pattern) - o->data;
		debug_printf_glob("glob pattern '%s' is literal\n", pattern);
		върни o_save_ptr_helper(o, n);
	}

#ifndef __U_BOOT__
	memset(&globdata, 0, sizeof(globdata));
	/* Can't use GLOB_NOCHECK: it does not unescape the string.
	 * If we glob "*.\*" and don't find anything, we need
	 * to fall back to using literal "*.*", but GLOB_NOCHECK
	 * will return "*.\*"!
	 */
	gr = glob(pattern, 0, NULL, &globdata);
	debug_printf_glob("glob('%s'):%d\n", pattern, gr);
	ако (gr != 0) {
		ако (gr == GLOB_NOMATCH) {
			globfree(&globdata);
			goto literal;
		}
		ако (gr == GLOB_NOSPACE)
			bb_die_memory_exhausted();
		/* GLOB_ABORTED? Only happens with GLOB_ERR flag,
		 * but we didn't specify it. Paranoia again. */
		bb_error_msg_and_die("glob error %d on '%s'", gr, pattern);
	}
	ако (globdata.gl_pathv && globdata.gl_pathv[0]) {
		char **argv = globdata.gl_pathv;
		/* "forget" pattern in o */
		o->length = pattern - o->data;
		докато (1) {
			o_addstr_with_NUL(o, *argv);
			n = o_save_ptr_helper(o, n);
			argv++;
			ако (!*argv)
				break;
		}
	}
	globfree(&globdata);
	ако (DEBUG_GLOB)
		debug_print_list("perform_glob returning", o, n);
	върни n;
#иначе /* __U_BOOT__ */
	/*
	 * NOTE We only use perform glob to call unbackslash to remove backslash
	 * from string once expanded.
	 * So, it seems OK to return this if no previous return was done.
	 */
	върни o_save_ptr_helper(o, n);
#endif /* __U_BOOT__ */
}

#endif /* !HUSH_BRACE_EXPANSION */

/* If o->o_expflags & EXP_FLAG_GLOB, glob the string so far remembered.
 * Otherwise, just finish current list[] and start new */
static цяло o_save_ptr(o_string *o, цяло n)
{
	ако (o->o_expflags & EXP_FLAG_GLOB) {
		/* If o->has_empty_slot, list[n] was already globbed
		 * (if it was requested back then when it was filled)
		 * so don't do that again! */
		ако (!o->has_empty_slot)
			върни perform_glob(o, n); /* o_save_ptr_helper is inside */
	}
	върни o_save_ptr_helper(o, n);
}

/* "Please convert list[n] to real char* ptrs, and NULL terminate it." */
static char **o_finalize_list(o_string *o, цяло n)
{
	char **list;
	цяло string_start;

	ако (DEBUG_EXPAND)
		debug_print_list("finalized", o, n);
	debug_printf_expand("finalized n:%d\n", n);
	list = (char**)o->data;
	string_start = ((n + 0xf) & ~0xf) * sizeof(list[0]);
	list[--n] = NULL;
	докато (n) {
		n--;
		list[n] = o->data + (цяло)(uintptr_t)list[n] + string_start;
	}
	върни list;
}

static void free_pipe_list(struct pipe *pi);

/* Returns pi->next - next pipe in the list */
static struct pipe *free_pipe(struct pipe *pi)
{
	struct pipe *next;
	цяло i;

#ifndef __U_BOOT__
	debug_printf_clean("free_pipe (pid %d)\n", getpid());
#endif /* !__U_BOOT__ */
	за (i = 0; i < pi->num_cmds; i++) {
		struct command *command;
#ifndef __U_BOOT__
		struct redir_struct *r, *rnext;
#endif /* !__U_BOOT__ */

		command = &pi->cmds[i];
		debug_printf_clean("  command %d:\n", i);
		ако (command->argv) {
			ако (DEBUG_CLEAN) {
				цяло a;
				char **p;
				за (a = 0, p = command->argv; *p; a++, p++) {
					debug_printf_clean("   argv[%d] = %s\n", a, *p);
				}
			}
			free_strings(command->argv);
			//command->argv = NULL;
		}
		/* not "else if": on syntax error, we may have both! */
		ако (command->group) {
			debug_printf_clean("   begin group (cmd_type:%d)\n",
					command->cmd_type);
			free_pipe_list(command->group);
			debug_printf_clean("   end group\n");
			//command->group = NULL;
		}
		/* else is crucial here.
		 * If group != NULL, child_func is meaningless */
#ако ENABLE_HUSH_FUNCTIONS
		иначе ако (command->child_func) {
			debug_printf_exec("cmd %p releases child func at %p\n", command, command->child_func);
			command->child_func->parent_cmd = NULL;
		}
#endif
#ако !BB_MMU
		free(command->group_as_string);
		//command->group_as_string = NULL;
#endif
#ifndef __U_BOOT__
		за (r = command->redirects; r; r = rnext) {
			debug_printf_clean("   redirect %d%s",
					r->rd_fd, redir_table[r->rd_type].descrip);
			/* guard against the case >$FOO, where foo is unset or blank */
			ако (r->rd_filename) {
				debug_printf_clean(" fname:'%s'\n", r->rd_filename);
				free(r->rd_filename);
				//r->rd_filename = NULL;
			}
			debug_printf_clean(" rd_dup:%d\n", r->rd_dup);
			rnext = r->next;
			free(r);
		}
		//command->redirects = NULL;
#endif /* !__U_BOOT__ */
	}
	free(pi->cmds);   /* children are an array, they get freed all at once */
	//pi->cmds = NULL;
#ifndef __U_BOOT__
#ако ENABLE_HUSH_JOB
	free(pi->cmdtext);
	//pi->cmdtext = NULL;
#endif
#endif /* !__U_BOOT__ */

	next = pi->next;
	free(pi);
	върни next;
}

static void free_pipe_list(struct pipe *pi)
{
	докато (pi) {
#ако HAS_KEYWORDS
		debug_printf_clean("pipe reserved word %d\n", pi->res_word);
#endif
		debug_printf_clean("pipe followup code %d\n", pi->followup);
		pi = free_pipe(pi);
	}
}

/*** Parsing routines ***/

#ifndef debug_print_tree
static void debug_print_tree(struct pipe *pi, цяло lvl)
{
	static const char *const PIPE[] ALIGN_PTR = {
		[PIPE_SEQ] = "SEQ",
		[PIPE_AND] = "AND",
		[PIPE_OR ] = "OR" ,
		[PIPE_BG ] = "BG" ,
	};
	static const char *RES[] = {
		[RES_NONE ] = "NONE" ,
# ако ENABLE_HUSH_IF
		[RES_IF   ] = "IF"   ,
		[RES_THEN ] = "THEN" ,
		[RES_ELIF ] = "ELIF" ,
		[RES_ELSE ] = "ELSE" ,
		[RES_FI   ] = "FI"   ,
# endif
# ако ENABLE_HUSH_LOOPS
		[RES_FOR  ] = "FOR"  ,
		[RES_WHILE] = "WHILE",
		[RES_UNTIL] = "UNTIL",
		[RES_DO   ] = "DO"   ,
		[RES_DONE ] = "DONE" ,
# endif
# ако ENABLE_HUSH_LOOPS || ENABLE_HUSH_CASE
		[RES_IN   ] = "IN"   ,
# endif
# ако ENABLE_HUSH_CASE
		[RES_CASE ] = "CASE" ,
		[RES_CASE_IN ] = "CASE_IN" ,
		[RES_MATCH] = "MATCH",
		[RES_CASE_BODY] = "CASE_BODY",
		[RES_ESAC ] = "ESAC" ,
# endif
		[RES_XXXX ] = "XXXX" ,
		[RES_SNTX ] = "SNTX" ,
	};
	static const char *const CMDTYPE[] ALIGN_PTR = {
		"{}",
		"()",
		"[noglob]",
# ако ENABLE_HUSH_FUNCTIONS
		"func()",
# endif
	};

	цяло pin, prn;

	pin = 0;
	докато (pi) {
		fdprintf(2, "%*spipe %d #cmds:%d %sres_word=%s followup=%d %s\n",
				lvl*2, "",
				pin,
				pi->num_cmds,
				(IF_HAS_KEYWORDS(pi->pi_inverted ? "! " :) ""),
				RES[pi->res_word],
				pi->followup, PIPE[pi->followup]
		);
		prn = 0;
		докато (prn < pi->num_cmds) {
			struct command *command = &pi->cmds[prn];
			char **argv = command->argv;

			fdprintf(2, "%*s cmd %d assignment_cnt:%d",
					lvl*2, "", prn,
					command->assignment_cnt);
# ако ENABLE_HUSH_LINENO_VAR
			fdprintf(2, " LINENO:%u", command->lineno);
# endif
			ако (command->group) {
				fdprintf(2, " group %s: (argv=%p)%s%s\n",
						CMDTYPE[command->cmd_type],
						argv
# ако !BB_MMU
						, " group_as_string:", command->group_as_string
# иначе
						, "", ""
# endif
				);
				debug_print_tree(command->group, lvl+1);
				prn++;
				continue;
			}
			ако (argv) докато (*argv) {
				fdprintf(2, " '%s'", *argv);
				argv++;
			}
#ifndef __U_BOOT__
			ако (command->redirects)
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

static struct pipe *new_pipe(void)
{
	struct pipe *pi;
	pi = xzalloc(sizeof(struct pipe));
	/*pi->res_word = RES_NONE; - RES_NONE is 0 anyway */
	върни pi;
}

/* Command (member of a pipe) is complete, or we start a new pipe
 * if ctx->command is NULL.
 * No errors possible here.
 */
static цяло done_command(struct parse_context *ctx)
{
	/* The command is really already in the pipe structure, so
	 * advance the pipe counter and make a new, null command. */
	struct pipe *pi = ctx->pipe;
	struct command *command = ctx->command;

#ако 0	/* Instead we emit error message at run time */
	ако (ctx->pending_redirect) {
		/* For example, "cmd >" (no filename to redirect to) */
		syntax_error("invalid redirect");
		ctx->pending_redirect = NULL;
	}
#endif

	ако (command) {
		ако (IS_NULL_CMD(command)) {
			debug_printf_parse("done_command: skipping null cmd, num_cmds=%d\n", pi->num_cmds);
			goto clear_and_ret;
		}
		pi->num_cmds++;
		debug_printf_parse("done_command: ++num_cmds=%d\n", pi->num_cmds);
		//debug_print_tree(ctx->list_head, 20);
	} иначе {
		debug_printf_parse("done_command: initializing, num_cmds=%d\n", pi->num_cmds);
	}

	/* Only real trickiness here is that the uncommitted
	 * command structure is not counted in pi->num_cmds. */
	pi->cmds = xrealloc(pi->cmds, sizeof(*pi->cmds) * (pi->num_cmds+1));
	ctx->command = command = &pi->cmds[pi->num_cmds];
 clear_and_ret:
	memset(command, 0, sizeof(*command));
#ако ENABLE_HUSH_LINENO_VAR
	command->lineno = G.parse_lineno;
	debug_printf_parse("command->lineno = G.parse_lineno (%u)\n", G.parse_lineno);
#endif
	върни pi->num_cmds; /* used only for 0/nonzero check */
}

static void done_pipe(struct parse_context *ctx, pipe_style type)
{
	цяло not_null;

	debug_printf_parse("done_pipe entered, followup %d\n", type);
	/* Close previous command */
	not_null = done_command(ctx);
#ако HAS_KEYWORDS
	ctx->pipe->pi_inverted = ctx->ctx_inverted;
	ctx->ctx_inverted = 0;
	ctx->pipe->res_word = ctx->ctx_res_w;
#endif
	ако (type == PIPE_BG && ctx->list_head != ctx->pipe) {
		/* Necessary since && and || have precedence over &:
		 * "cmd1 && cmd2 &" must spawn both cmds, not only cmd2,
		 * in a backgrounded subshell.
		 */
		struct pipe *pi;
		struct command *command;

		/* Is this actually this construct, all pipes end with && or ||? */
		pi = ctx->list_head;
		докато (pi != ctx->pipe) {
			ако (pi->followup != PIPE_AND && pi->followup != PIPE_OR)
				goto no_conv;
			pi = pi->next;
		}

		debug_printf_parse("BG with more than one pipe, converting to { p1 &&...pN; } &\n");
		pi->followup = PIPE_SEQ; /* close pN _not_ with "&"! */
		pi = xzalloc(sizeof(*pi));
		pi->followup = PIPE_BG;
		pi->num_cmds = 1;
		pi->cmds = xzalloc(sizeof(pi->cmds[0]));
		command = &pi->cmds[0];
		ако (CMD_NORMAL != 0) /* "if xzalloc didn't do that already" */
			command->cmd_type = CMD_NORMAL;
		command->group = ctx->list_head;
#ако !BB_MMU
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
	} иначе {
 no_conv:
		ctx->pipe->followup = type;
	}

	/* Without this check, even just <enter> on command line generates
	 * tree of three NOPs (!). Which is harmless but annoying.
	 * IOW: it is safe to do it unconditionally. */
	ако (not_null
#ако ENABLE_HUSH_IF
	 || ctx->ctx_res_w == RES_FI
#endif
#ако ENABLE_HUSH_LOOPS
	 || ctx->ctx_res_w == RES_DONE
	 || ctx->ctx_res_w == RES_FOR
	 || ctx->ctx_res_w == RES_IN
#endif
#ако ENABLE_HUSH_CASE
	 || ctx->ctx_res_w == RES_ESAC
#endif
	) {
		struct pipe *new_p;
		debug_printf_parse("done_pipe: adding new pipe: "
#ifndef __U_BOOT__
				"not_null:%d ctx->ctx_res_w:%d\n",
				not_null, ctx->ctx_res_w);
#иначе /* __U_BOOT__ */
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
#ако ENABLE_HUSH_LOOPS
		ако (ctx->ctx_res_w == RES_FOR
		 || ctx->ctx_res_w == RES_IN)
			ctx->ctx_res_w = RES_NONE;
#endif
#ако ENABLE_HUSH_CASE
		ако (ctx->ctx_res_w == RES_MATCH)
			ctx->ctx_res_w = RES_CASE_BODY;
		ако (ctx->ctx_res_w == RES_CASE)
			ctx->ctx_res_w = RES_CASE_IN;
#endif
		ctx->command = NULL; /* trick done_command below */
		/* Create the memory for command, roughly:
		 * ctx->pipe->cmds = new struct command;
		 * ctx->command = &ctx->pipe->cmds[0];
		 */
		done_command(ctx);
		//debug_print_tree(ctx->list_head, 10);
	}
	debug_printf_parse("done_pipe return\n");
}

static void initialize_context(struct parse_context *ctx)
{
	memset(ctx, 0, sizeof(*ctx));
	ако (MAYBE_ASSIGNMENT != 0)
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
#ако HAS_KEYWORDS
struct reserved_combo {
	char literal[6];
	unsigned char res;
	unsigned char assignment_flag;
	uint32_t flag;
};
enum {
	FLAG_END   = (1 << RES_NONE ),
# ако ENABLE_HUSH_IF
	FLAG_IF    = (1 << RES_IF   ),
	FLAG_THEN  = (1 << RES_THEN ),
	FLAG_ELIF  = (1 << RES_ELIF ),
	FLAG_ELSE  = (1 << RES_ELSE ),
	FLAG_FI    = (1 << RES_FI   ),
# endif
# ако ENABLE_HUSH_LOOPS
	FLAG_FOR   = (1 << RES_FOR  ),
	FLAG_WHILE = (1 << RES_WHILE),
	FLAG_UNTIL = (1 << RES_UNTIL),
	FLAG_DO    = (1 << RES_DO   ),
	FLAG_DONE  = (1 << RES_DONE ),
	FLAG_IN    = (1 << RES_IN   ),
# endif
# ако ENABLE_HUSH_CASE
	FLAG_MATCH = (1 << RES_MATCH),
	FLAG_ESAC  = (1 << RES_ESAC ),
# endif
	FLAG_START = (1 << RES_XXXX ),
};

static const struct reserved_combo* match_reserved_word(o_string *word)
{
	/* Mostly a list of accepted follow-up reserved words.
	 * FLAG_END means we are done with the sequence, and are ready
	 * to turn the compound list into a command.
	 * FLAG_START means the word must start a new compound list.
	 */
	static const struct reserved_combo reserved_list[] ALIGN4 = {
# ако ENABLE_HUSH_IF
		{ "!",     RES_NONE,  NOT_ASSIGNMENT  , 0 },
		{ "if",    RES_IF,    MAYBE_ASSIGNMENT, FLAG_THEN | FLAG_START },
		{ "then",  RES_THEN,  MAYBE_ASSIGNMENT, FLAG_ELIF | FLAG_ELSE | FLAG_FI },
		{ "elif",  RES_ELIF,  MAYBE_ASSIGNMENT, FLAG_THEN },
		{ "else",  RES_ELSE,  MAYBE_ASSIGNMENT, FLAG_FI   },
		{ "fi",    RES_FI,    NOT_ASSIGNMENT  , FLAG_END  },
# endif
# ако ENABLE_HUSH_LOOPS
		{ "for",   RES_FOR,   NOT_ASSIGNMENT  , FLAG_IN | FLAG_DO | FLAG_START },
		{ "while", RES_WHILE, MAYBE_ASSIGNMENT, FLAG_DO | FLAG_START },
		{ "until", RES_UNTIL, MAYBE_ASSIGNMENT, FLAG_DO | FLAG_START },
		{ "in",    RES_IN,    NOT_ASSIGNMENT  , FLAG_DO   },
		{ "do",    RES_DO,    MAYBE_ASSIGNMENT, FLAG_DONE },
		{ "done",  RES_DONE,  NOT_ASSIGNMENT  , FLAG_END  },
# endif
# ако ENABLE_HUSH_CASE
		{ "case",  RES_CASE,  NOT_ASSIGNMENT  , FLAG_MATCH | FLAG_START },
		{ "esac",  RES_ESAC,  NOT_ASSIGNMENT  , FLAG_END  },
# endif
	};
	const struct reserved_combo *r;

	за (r = reserved_list; r < reserved_list + ARRAY_SIZE(reserved_list); r++) {
		ако (strcmp(word->data, r->literal) == 0)
			върни r;
	}
	върни NULL;
}
/* Return NULL: not a keyword, else: keyword
 */
static const struct reserved_combo* reserved_word(struct parse_context *ctx)
{
# ако ENABLE_HUSH_CASE
	static const struct reserved_combo reserved_match = {
		"",        RES_MATCH, NOT_ASSIGNMENT , FLAG_MATCH | FLAG_ESAC
	};
# endif
	const struct reserved_combo *r;

	ако (ctx->word.has_quoted_part)
		върни 0;
	r = match_reserved_word(&ctx->word);
	ако (!r)
		върни r; /* NULL */

	debug_printf("found reserved word %s, res %d\n", r->literal, r->res);
# ако ENABLE_HUSH_CASE
	ако (r->res == RES_IN && ctx->ctx_res_w == RES_CASE_IN) {
		/* "case word IN ..." - IN part starts first MATCH part */
		r = &reserved_match;
	} иначе
# endif
	ако (r->flag == 0) { /* '!' */
		ако (ctx->ctx_inverted) { /* bash doesn't accept '! ! true' */
			syntax_error("! ! command");
			ctx->ctx_res_w = RES_SNTX;
		}
		ctx->ctx_inverted = 1;
		върни r;
	}
	ако (r->flag & FLAG_START) {
		struct parse_context *old;

		old = xmemdup(ctx, sizeof(*ctx));
		debug_printf_parse("push stack %p\n", old);
		initialize_context(ctx);
		ctx->stack = old;
	} иначе ако (/*ctx->ctx_res_w == RES_NONE ||*/ !(ctx->old_flag & (1 << r->res))) {
		syntax_error_at(ctx->word.data);
		ctx->ctx_res_w = RES_SNTX;
		върни r;
	} иначе {
		/* "{...} fi" is ok. "{...} if" is not
		 * Example:
		 * if { echo foo; } then { echo bar; } fi */
		ако (ctx->command->group)
			done_pipe(ctx, PIPE_SEQ);
	}

	ctx->ctx_res_w = r->res;
	ctx->old_flag = r->flag;
	ctx->is_assignment = r->assignment_flag;
	debug_printf_parse("ctx->is_assignment='%s'\n", assignment_flag[ctx->is_assignment]);

	ако (ctx->old_flag & FLAG_END) {
		struct parse_context *old;

		done_pipe(ctx, PIPE_SEQ);
		debug_printf_parse("pop stack %p\n", ctx->stack);
		old = ctx->stack;
		old->command->group = ctx->list_head;
		old->command->cmd_type = CMD_NORMAL;
# ако !BB_MMU
		/* At this point, the compound command's string is in
		 * ctx->as_string... except for the leading keyword!
		 * Consider this example: "echo a | if true; then echo a; fi"
		 * ctx->as_string will contain "true; then echo a; fi",
		 * with "if " remaining in old->as_string!
		 */
		{
			char *str;
			цяло len = old->as_string.length;
			/* Concatenate halves */
			o_addstr(&old->as_string, ctx->as_string.data);
			o_free(&ctx->as_string);
			/* Find where leading keyword starts in first half */
			str = old->as_string.data + len;
			ако (str > old->as_string.data)
				str--; /* skip whitespace after keyword */
			докато (str > old->as_string.data && isalpha(str[-1]))
				str--;
			/* Ugh, we're done with this horrid hack */
			old->command->group_as_string = xstrdup(str);
			debug_printf_parse("pop, remembering as:'%s'\n",
					old->command->group_as_string);
		}
# endif
		*ctx = *old;   /* physical copy */
		free(old);
	}
	върни r;
}
#endif /* HAS_KEYWORDS */

/* Word is complete, look at it and update parsing context.
 * Normal return is 0. Syntax errors return 1.
 * Note: on return, word is reset, but not o_free'd!
 */
static цяло done_word(struct parse_context *ctx)
{
	struct command *command = ctx->command;

	debug_printf_parse("done_word entered: '%s' %p\n", ctx->word.data, command);
	ако (ctx->word.length == 0 && !ctx->word.has_quoted_part) {
		debug_printf_parse("done_word return 0: true null, ignored\n");
		върни 0;
	}

#ifndef __U_BOOT__
	ако (ctx->pending_redirect) {
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
		ако (ctx->pending_redirect->rd_type == REDIRECT_HEREDOC) {
			unbackslash(ctx->pending_redirect->rd_filename);
			/* Is it <<"HEREDOC"? */
			ако (ctx->word.has_quoted_part) {
				ctx->pending_redirect->rd_dup |= HEREDOC_QUOTED;
			}
		}
		debug_printf_parse("word stored in rd_filename: '%s'\n", ctx->word.data);
		ctx->pending_redirect = NULL;
	} иначе {
#endif /* !__U_BOOT__ */
#ако HAS_KEYWORDS
# ако ENABLE_HUSH_CASE
		ако (ctx->ctx_dsemicolon
		 && strcmp(ctx->word.data, "esac") != 0 /* not "... pattern) cmd;; esac" */
		) {
			/* already done when ctx_dsemicolon was set to 1: */
			/* ctx->ctx_res_w = RES_MATCH; */
			ctx->ctx_dsemicolon = 0;
		} иначе
# endif
# ако defined(CMD_TEST2_SINGLEWORD_NOGLOB)
		ако (command->cmd_type == CMD_TEST2_SINGLEWORD_NOGLOB
		 && strcmp(ctx->word.data, "]]") == 0
		) {
			/* allow "[[ ]] >file" etc */
			command->cmd_type = CMD_SINGLEWORD_NOGLOB;
		} иначе
# endif
		ако (!command->argv /* if it's the first word... */
# ако ENABLE_HUSH_LOOPS
		 && ctx->ctx_res_w != RES_FOR /* ...not after FOR or IN */
		 && ctx->ctx_res_w != RES_IN
# endif
# ако ENABLE_HUSH_CASE
		 && ctx->ctx_res_w != RES_CASE
# endif
		) {
			const struct reserved_combo *reserved;
			reserved = reserved_word(ctx);
			debug_printf_parse("checking for reserved-ness: %d\n", !!reserved);
			ако (reserved) {
# ако ENABLE_HUSH_LINENO_VAR
/* Case:
 * "while ...; do
 *	cmd ..."
 * If we don't close the pipe _now_, immediately after "do", lineno logic
 * sees "cmd" as starting at "do" - i.e., at the previous line.
 */
				ако (0
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
				върни (ctx->ctx_res_w == RES_SNTX);
			}
# ако defined(CMD_TEST2_SINGLEWORD_NOGLOB)
			ако (strcmp(ctx->word.data, "[[") == 0) {
				command->cmd_type = CMD_TEST2_SINGLEWORD_NOGLOB;
			} иначе
# endif
# ако defined(CMD_SINGLEWORD_NOGLOB)
			ако (0
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
# иначе
			{ /* empty block to pair "if ... else" */ }
# endif
		}
#endif /* HAS_KEYWORDS */

		ако (command->group) {
			/* "{ echo foo; } echo bar" - bad */
			syntax_error_at(ctx->word.data);
			debug_printf_parse("done_word return 1: syntax error, "
					"groups and arglists don't mix\n");
			върни 1;
		}

		/* If this word wasn't an assignment, next ones definitely
		 * can't be assignments. Even if they look like ones. */
		ако (ctx->is_assignment != DEFINITELY_ASSIGNMENT
		 && ctx->is_assignment != WORD_IS_KEYWORD
		) {
			ctx->is_assignment = NOT_ASSIGNMENT;
		} иначе {
			ако (ctx->is_assignment == DEFINITELY_ASSIGNMENT) {
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

#ако ENABLE_HUSH_LOOPS
	ако (ctx->ctx_res_w == RES_FOR) {
		ако (ctx->word.has_quoted_part
		 || endofname(command->argv[0])[0] != '\0'
		) {
			/* bash says just "not a valid identifier" */
			syntax_error("bad for loop variable");
			върни 1;
		}
		/* Force FOR to have just one word (variable name) */
		/* NB: basically, this makes hush see "for v in ..."
		 * syntax as if it is "for v; in ...". FOR and IN become
		 * two pipe structs in parse tree. */
		done_pipe(ctx, PIPE_SEQ);
	}
#endif
#ако ENABLE_HUSH_CASE
	/* Force CASE to have just one word */
	ако (ctx->ctx_res_w == RES_CASE) {
		done_pipe(ctx, PIPE_SEQ);
	}
#endif

	o_reset_to_empty_unquoted(&ctx->word);

	debug_printf_parse("done_word return 0\n");
	върни 0;
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
#ако BB_MMU
#define parse_redir_right_fd(as_string, input) \
	parse_redir_right_fd(input)
#endif
static цяло parse_redir_right_fd(o_string *as_string, struct in_str *input)
{
	цяло ch, d, ok;

	ch = i_peek(input);
	ако (ch != '&')
		върни REDIRFD_TO_FILE;

	ch = i_getch(input);  /* get the & */
	nommu_addchr(as_string, ch);
	ch = i_peek(input);
	ако (ch == '-') {
		ch = i_getch(input);
		nommu_addchr(as_string, ch);
		върни REDIRFD_CLOSE;
	}
	d = 0;
	ok = 0;
	докато (ch != EOF && isdigit(ch)) {
		d = d*10 + (ch-'0');
		ok = 1;
		ch = i_getch(input);
		nommu_addchr(as_string, ch);
		ch = i_peek(input);
	}
	ако (ok) върни d;

//TODO: this is the place to catch ">&file" bashism (redirect both fd 1 and 2)

	bb_simple_error_msg("ambiguous redirect");
	върни REDIRFD_SYNTAX_ERR;
}

/* Return code is 0 normal, 1 if a syntax error is detected
 */
static цяло parse_redirect(struct parse_context *ctx,
		цяло fd,
		redir_type style,
		struct in_str *input)
{
	struct command *command = ctx->command;
	struct redir_struct *redir;
	struct redir_struct **redirp;
	цяло dup_num;

	dup_num = REDIRFD_TO_FILE;
	ако (style != REDIRECT_HEREDOC) {
		/* Check for a '>&1' type redirect */
		dup_num = parse_redir_right_fd(&ctx->as_string, input);
		ако (dup_num == REDIRFD_SYNTAX_ERR)
			върни 1;
	} иначе {
		цяло ch = i_peek_and_eat_bkslash_nl(input);
		dup_num = (ch == '-'); /* HEREDOC_SKIPTABS bit is 1 */
		ако (dup_num) { /* <<-... */
			ch = i_getch(input);
			nommu_addchr(&ctx->as_string, ch);
			ch = i_peek(input);
		}
	}

	ако (style == REDIRECT_OVERWRITE && dup_num == REDIRFD_TO_FILE) {
		цяло ch = i_peek_and_eat_bkslash_nl(input);
		ако (ch == '|') {
			/* >|FILE redirect ("clobbering" >).
			 * Since we do not support "set -o noclobber" yet,
			 * >| and > are the same for now. Just eat |.
			 */
			ch = i_getch(input);
			nommu_addchr(&ctx->as_string, ch);
		}
	}

	/* Create a new redir_struct and append it to the linked list */
	redirp = &command->redirects;
	докато ((redir = *redirp) != NULL) {
		redirp = &(redir->next);
	}
	*redirp = redir = xzalloc(sizeof(*redir));
	/* redir->next = NULL; */
	/* redir->rd_filename = NULL; */
	redir->rd_type = style;
	redir->rd_fd = (fd == -1) ? redir_table[style].default_fd : fd;

	debug_printf_parse("redirect type %d %s\n", redir->rd_fd,
				redir_table[style].descrip);

	redir->rd_dup = dup_num;
	ако (style != REDIRECT_HEREDOC && dup_num != REDIRFD_TO_FILE) {
		/* Erik had a check here that the file descriptor in question
		 * is legit; I postpone that to "run time"
		 * A "-" representation of "close me" shows up as a -3 here */
		debug_printf_parse("duplicating redirect '%d>&%d'\n",
				redir->rd_fd, redir->rd_dup);
	} иначе {
#ако 0		/* Instead we emit error message at run time */
		ако (ctx->pending_redirect) {
			/* For example, "cmd > <file" */
			syntax_error("invalid redirect");
		}
#endif
		/* Set ctx->pending_redirect, so we know what to do at the
		 * end of the next parsed word. */
		ctx->pending_redirect = redir;
	}
	върни 0;
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
static цяло redirect_opt_num(o_string *o)
{
	цяло num;

	ако (o->data == NULL)
		върни -1;
	num = bb_strtou(o->data, NULL, 10);
	ако (errno || num < 0)
		върни -1;
	o_reset_to_empty_unquoted(o);
	върни num;
}

#ако BB_MMU
#define fetch_till_str(as_string, input, word, skip_tabs) \
	fetch_till_str(input, word, skip_tabs)
#endif
static char *fetch_till_str(o_string *as_string,
		struct in_str *input,
		const char *word,
		цяло heredoc_flags)
{
	o_string heredoc = NULL_O_STRING;
	unsigned past_EOL;
	цяло prev = 0; /* not \ */
	цяло ch;

	/* Starting with "" is necessary for this case:
	 * cat <<EOF
	 *
	 * xxx
	 * EOF
	 */
	heredoc.data = xzalloc(1); /* start as "", not as NULL */

	goto jump_in;

	докато (1) {
		ch = i_getch(input);
		ако (ch != EOF)
			nommu_addchr(as_string, ch);
		ако (ch == '\n' || ch == EOF) {
 check_heredoc_end:
#ifndef __U_BOOT__
			ако ((heredoc_flags & HEREDOC_QUOTED) || prev != '\\') {
#иначе /* __U_BOOT__ */
			ако (prev != '\\') {
#endif
				/* End-of-line, and not a line continuation */
				ако (strcmp(heredoc.data + past_EOL, word) == 0) {
					heredoc.data[past_EOL] = '\0';
					debug_printf_heredoc("parsed '%s' heredoc '%s'\n", word, heredoc.data);
					върни heredoc.data;
				}
				ако (ch == '\n') {
					/* This is a new line.
					 * Remember position and backslash-escaping status.
					 */
					o_addchr(&heredoc, ch);
					prev = ch;
 jump_in:
					past_EOL = heredoc.length;
					/* Get 1st char of next line, possibly skipping leading tabs */
					do {
						ch = i_getch(input);
						ако (ch != EOF)
							nommu_addchr(as_string, ch);
#ifndef __U_BOOT__
					} докато ((heredoc_flags & HEREDOC_SKIPTABS) && ch == '\t');
#иначе /* __U_BOOT__ */
				} докато (ch == '\t');
#endif
					/* If this immediately ended the line,
					 * go back to end-of-line checks.
					 */
					ако (ch == '\n')
						goto check_heredoc_end;
				}
			} иначе {
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
				continue;
			}
		}
		ако (ch == EOF) {
			o_free(&heredoc);
			върни NULL; /* error */
		}
		o_addchr(&heredoc, ch);
		nommu_addchr(as_string, ch);
		ако (prev == '\\' && ch == '\\')
			/* Correctly handle foo\\<eol> (not a line cont.) */
			prev = 0; /* not '\' */
		иначе
			prev = ch;
	}
}
#endif /* !__U_BOOT__ */

/* Look at entire parse tree for not-yet-loaded REDIRECT_HEREDOCs
 * and load them all. There should be exactly heredoc_cnt of them.
 */
#ако BB_MMU
#define fetch_heredocs(as_string, pi, heredoc_cnt, input) \
	fetch_heredocs(pi, heredoc_cnt, input)
#endif
static цяло fetch_heredocs(o_string *as_string, struct pipe *pi, цяло heredoc_cnt, struct in_str *input)
{
	докато (pi && heredoc_cnt) {
		цяло i;
		struct command *cmd = pi->cmds;

		debug_printf_heredoc("fetch_heredocs: num_cmds:%d cmd argv0:'%s'\n",
				pi->num_cmds,
				cmd->argv ? cmd->argv[0] : "NONE"
		);
		за (i = 0; i < pi->num_cmds; i++) {
#ifndef __U_BOOT__
			struct redir_struct *redir = cmd->redirects;

#endif /* !__U_BOOT__ */
			debug_printf_heredoc("fetch_heredocs: %d cmd argv0:'%s'\n",
					i, cmd->argv ? cmd->argv[0] : "NONE");
#ifndef __U_BOOT__
			докато (redir) {
				ако (redir->rd_type == REDIRECT_HEREDOC) {
					char *p;

					redir->rd_type = REDIRECT_HEREDOC2;
					/* redir->rd_dup is (ab)used to indicate <<- */
					p = fetch_till_str(as_string, input,
							redir->rd_filename, redir->rd_dup);
					ако (!p) {
						syntax_error("unexpected EOF in here document");
						върни -1;
					}
					free(redir->rd_filename);
					redir->rd_filename = p;
					heredoc_cnt--;
				}
				redir = redir->next;
			}
#endif /* !__U_BOOT__ */
			ако (cmd->group) {
				//bb_error_msg("%s:%u heredoc_cnt:%d", __func__, __LINE__, heredoc_cnt);
				heredoc_cnt = fetch_heredocs(as_string, cmd->group, heredoc_cnt, input);
				//bb_error_msg("%s:%u heredoc_cnt:%d", __func__, __LINE__, heredoc_cnt);
				ако (heredoc_cnt < 0)
					върни heredoc_cnt; /* error */
			}
			cmd++;
		}
		pi = pi->next;
	}
	върни heredoc_cnt;
}

static цяло run_list(struct pipe *pi);
#ако BB_MMU
#define parse_stream(pstring, heredoc_cnt_ptr, input, end_trigger) \
	parse_stream(heredoc_cnt_ptr, input, end_trigger)
#endif
static struct pipe *parse_stream(char **pstring,
		цяло *heredoc_cnt_ptr,
		struct in_str *input,
		цяло end_trigger);

/* Returns number of heredocs not yet consumed,
 * or -1 on error.
 */
static цяло parse_group(struct parse_context *ctx,
		struct in_str *input, цяло ch)
{
	/* ctx->word contains characters seen prior to ( or {.
	 * Typically it's empty, but for function defs,
	 * it contains function name (without '()'). */
#ако BB_MMU
# define as_string NULL
#иначе
	char *as_string = NULL;
#endif
	struct pipe *pipe_list;
	цяло heredoc_cnt = 0;
	цяло endch;
	struct command *command = ctx->command;

	debug_printf_parse("parse_group entered\n");
#ако ENABLE_HUSH_FUNCTIONS
	ако (ch == '(' && !ctx->word.has_quoted_part) {
		ако (ctx->word.length)
			ако (done_word(ctx))
				върни -1;
		ако (!command->argv)
			goto skip; /* (... */
		ако (command->argv[1]) { /* word word ... (... */
			syntax_error_unexpected_ch('(');
			върни -1;
		}
		/* it is "word(..." or "word (..." */
		do
			ch = i_getch(input);
		докато (ch == ' ' || ch == '\t');
		ако (ch != ')') {
			syntax_error_unexpected_ch(ch);
			върни -1;
		}
		nommu_addchr(&ctx->as_string, ch);
		do
			ch = i_getch(input);
		докато (ch == ' ' || ch == '\t' || ch == '\n');
		ако (ch != '{' && ch != '(') {
			syntax_error_unexpected_ch(ch);
			върни -1;
		}
//bash allows functions named "123", "..", "return"!
//		if (endofname(command->argv[0])[0] != '\0') {
//			syntax_error("bad function name");
//			return -1;
//		}
		nommu_addchr(&ctx->as_string, ch);
		command->cmd_type = CMD_FUNCDEF;
		goto skip;
	}
#endif

#ако 0 /* Prevented by caller */
	ако (command->argv /* word [word]{... */
	 || ctx->word.length /* word{... */
	 || ctx->word.has_quoted_part /* ""{... */
	) {
		syntax_error(NULL);
		debug_printf_parse("parse_group return -1: "
			"syntax error, groups and arglists don't mix\n");
		върни -1;
	}
#endif

#ifndef __U_BOOT__
 IF_HUSH_FUNCTIONS(skip:)
#endif /* !__U_BOOT__ */

	endch = '}';
	ако (ch == '(') {
		endch = ')';
#ifndef __U_BOOT__
		IF_HUSH_FUNCTIONS(ако (command->cmd_type != CMD_FUNCDEF))
			command->cmd_type = CMD_SUBSHELL;
#endif /* !__U_BOOT__ */
	} иначе {
		/* bash does not allow "{echo...", requires whitespace */
		ch = i_peek(input);
		ако (ch != ' ' && ch != '\t' && ch != '\n'
		 && ch != '('	/* but "{(..." is allowed (without whitespace) */
		) {
			syntax_error_unexpected_ch(ch);
			върни -1;
		}
		ако (ch != '(') {
			ch = i_getch(input);
			nommu_addchr(&ctx->as_string, ch);
		}
	}

	debug_printf_heredoc("calling parse_stream, heredoc_cnt:%d\n", heredoc_cnt);
	pipe_list = parse_stream(&as_string, &heredoc_cnt, input, endch);
	debug_printf_heredoc("parse_stream returned: heredoc_cnt:%d\n", heredoc_cnt);
#ако !BB_MMU
	ако (as_string)
		o_addstr(&ctx->as_string, as_string);
#endif

	/* empty ()/{} or parse error? */
	ако (!pipe_list || pipe_list == ERR_PTR) {
		/* parse_stream already emitted error msg */
		ако (!BB_MMU)
			free(as_string);
		debug_printf_parse("parse_group return -1: "
			"parse_stream returned %p\n", pipe_list);
		върни -1;
	}
#ако !BB_MMU
	as_string[strlen(as_string) - 1] = '\0'; /* plink ')' or '}' */
	command->group_as_string = as_string;
	debug_printf_parse("end of group, remembering as:'%s'\n",
			command->group_as_string);
#endif

#ако ENABLE_HUSH_FUNCTIONS
	/* Convert "f() (cmds)" to "f() {(cmds)}" */
	ако (command->cmd_type == CMD_FUNCDEF && endch == ')') {
		struct command *cmd2;

		cmd2 = xzalloc(sizeof(*cmd2));
		cmd2->cmd_type = CMD_SUBSHELL;
		cmd2->group = pipe_list;
# ако !BB_MMU
//UNTESTED!
		cmd2->group_as_string = command->group_as_string;
		command->group_as_string = xasprintf("(%s)", command->group_as_string);
# endif

		pipe_list = new_pipe();
		pipe_list->cmds = cmd2;
		pipe_list->num_cmds = 1;
	}
#endif

	command->group = pipe_list;

	debug_printf_parse("parse_group return %d\n", heredoc_cnt);
	върни heredoc_cnt;
	/* command remains "open", available for possible redirects */
#undef as_string
}

#ifndef __U_BOOT__
#ако ENABLE_HUSH_TICK || ENABLE_FEATURE_SH_MATH || ENABLE_HUSH_DOLLAR_OPS
/* Subroutines for copying $(...) and `...` things */
/* '...' */
static цяло add_till_single_quote(o_string *dest, struct in_str *input)
{
	докато (1) {
		цяло ch = i_getch(input);
		ако (ch == EOF) {
			syntax_error_unterm_ch('\'');
			върни 0;
		}
		ако (ch == '\'')
			върни 1;
		o_addchr(dest, ch);
	}
}
static цяло add_till_single_quote_dquoted(o_string *dest, struct in_str *input)
{
	докато (1) {
		цяло ch = i_getch(input);
		ако (ch == EOF) {
			syntax_error_unterm_ch('\'');
			върни 0;
		}
		ако (ch == '\'')
			върни 1;
		o_addqchr(dest, ch);
	}
}

/* "...\"...`..`...." - do we need to handle "...$(..)..." too? */
static цяло add_till_backquote(o_string *dest, struct in_str *input, цяло in_dquote);
static цяло add_till_double_quote(o_string *dest, struct in_str *input)
{
	докато (1) {
		цяло ch = i_getch(input);
		ако (ch == EOF) {
			syntax_error_unterm_ch('"');
			върни 0;
		}
		ако (ch == '"')
			върни 1;
		ако (ch == '\\') {  /* \x. Copy both chars. */
			o_addchr(dest, ch);
			ch = i_getch(input);
		}
		o_addchr(dest, ch);
		ако (ch == '`') {
			ако (!add_till_backquote(dest, input, /*in_dquote:*/ 1))
				върни 0;
			o_addchr(dest, ch);
			continue;
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
static цяло add_till_backquote(o_string *dest, struct in_str *input, цяло in_dquote)
{
	докато (1) {
		цяло ch = i_getch(input);
		ако (ch == '`')
			върни 1;
		ако (ch == '\\') {
			/* \x. Copy both unless it is \`, \$, \\ and maybe \" */
			ch = i_getch(input);
			ако (ch != '`'
			 && ch != '$'
			 && ch != '\\'
			 && (!in_dquote || ch != '"')
			) {
				o_addchr(dest, '\\');
			}
		}
		ако (ch == EOF) {
			syntax_error_unterm_ch('`');
			върни 0;
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
static цяло add_till_closing_bracket(o_string *dest, struct in_str *input, unsigned end_ch)
{
	цяло ch;
	char dbl = end_ch & DOUBLE_CLOSE_CHAR_FLAG;
# ако BASH_SUBSTR || BASH_PATTERN_SUBST
	char end_char2 = end_ch >> 8;
# endif
	end_ch &= (DOUBLE_CLOSE_CHAR_FLAG - 1);

# ако ENABLE_HUSH_INTERACTIVE
	G.promptmode = 1; /* PS2 */
# endif
	debug_printf_prompt("%s promptmode=%d\n", __func__, G.promptmode);

	докато (1) {
		ch = i_getch(input);
		ако (ch == EOF) {
			syntax_error_unterm_ch(end_ch);
			върни 0;
		}
		ако (ch == end_ch
# ако BASH_SUBSTR || BASH_PATTERN_SUBST
		 || ch == end_char2
# endif
		) {
			ако (!dbl)
				break;
			/* we look for closing )) of $((EXPR)) */
			ако (i_peek_and_eat_bkslash_nl(input) == end_ch) {
				i_getch(input); /* eat second ')' */
				break;
			}
		}
		o_addchr(dest, ch);
		//bb_error_msg("%s:o_addchr('%c')", __func__, ch);
		ако (ch == '(' || ch == '{') {
			ch = (ch == '(' ? ')' : '}');
			ако (!add_till_closing_bracket(dest, input, ch))
				върни 0;
			o_addchr(dest, ch);
			continue;
		}
		ако (ch == '\'') {
			ако (!add_till_single_quote(dest, input))
				върни 0;
			o_addchr(dest, ch);
			continue;
		}
		ако (ch == '"') {
			ако (!add_till_double_quote(dest, input))
				върни 0;
			o_addchr(dest, ch);
			continue;
		}
		ако (ch == '`') {
			ако (!add_till_backquote(dest, input, /*in_dquote:*/ 0))
				върни 0;
			o_addchr(dest, ch);
			continue;
		}
		ако (ch == '\\') {
			/* \x. Copy verbatim. Important for  \(, \) */
			ch = i_getch(input);
			ако (ch == EOF) {
				syntax_error_unterm_ch(end_ch);
				върни 0;
			}
# ако 0
			ако (ch == '\n') {
				/* "backslash+newline", ignore both */
				o_delchr(dest); /* undo insertion of '\' */
				continue;
			}
# endif
			o_addchr(dest, ch);
			//bb_error_msg("%s:o_addchr('%c') after '\\'", __func__, ch);
			continue;
		}
	}
	debug_printf_parse("%s return '%s' ch:'%c'\n", __func__, dest->data, ch);
	върни ch;
}
#endif /* ENABLE_HUSH_TICK || ENABLE_FEATURE_SH_MATH || ENABLE_HUSH_DOLLAR_OPS */

#ако BASH_DOLLAR_SQUOTE
/* Return code: 1 for "found and parsed", 0 for "seen something else" */
# ако BB_MMU
#define parse_dollar_squote(as_string, dest, input) \
	parse_dollar_squote(dest, input)
#define as_string NULL
# endif
static цяло parse_dollar_squote(o_string *as_string, o_string *dest, struct in_str *input)
{
	цяло start;
	цяло ch = i_peek_and_eat_bkslash_nl(input);  /* first character after the $ */
	debug_printf_parse("parse_dollar_squote entered: ch='%c'\n", ch);
	ако (ch != '\'')
		върни 0;

	dest->has_quoted_part = 1;
	start = dest->length;

	ch = i_getch(input); /* eat ' */
	nommu_addchr(as_string, ch);
	докато (1) {
		ch = i_getch(input);
		nommu_addchr(as_string, ch);
		ако (ch == EOF) {
			syntax_error_unterm_ch('\'');
			върни 0;
		}
		ако (ch == '\'')
			break;
		ако (ch == SPECIAL_VAR_SYMBOL) {
			/* Convert raw ^C to corresponding special variable reference */
			o_addchr(dest, SPECIAL_VAR_SYMBOL);
			o_addchr(dest, SPECIAL_VAR_QUOTED_SVS);
			/* will addchr() another SPECIAL_VAR_SYMBOL (see after the if() block) */
		} иначе ако (ch == '\\') {
			static const char C_escapes[] ALIGN1 = "nrbtfav""x\\01234567";

			ch = i_getch(input);
			nommu_addchr(as_string, ch);
			ако (strchr(C_escapes, ch)) {
				char buf[4];
				char *p = buf;
				цяло cnt = 2;

				buf[0] = ch;
				ако ((unsigned char)(ch - '0') <= 7) { /* \ooo */
					do {
						ch = i_peek(input);
						ако ((unsigned char)(ch - '0') > 7)
							break;
						*++p = ch = i_getch(input);
						nommu_addchr(as_string, ch);
					} докато (--cnt != 0);
				} иначе ако (ch == 'x') { /* \xHH */
					do {
						ch = i_peek(input);
						ако (!isxdigit(ch))
							break;
						*++p = ch = i_getch(input);
						nommu_addchr(as_string, ch);
					} докато (--cnt != 0);
					ако (cnt == 2) { /* \x but next char is "bad" */
						ch = 'x';
						goto unrecognized;
					}
				} /* else simple seq like \\ or \t */
				*++p = '\0';
				p = buf;
				ch = bb_process_escape_sequence((void*)&p);
				//bb_error_msg("buf:'%s' ch:%x", buf, ch);
				ако (ch == '\0')
					continue; /* bash compat: $'...\0...' emits nothing */
			} иначе { /* unrecognized "\z": encode both chars unless ' or " */
				ако (ch != '\'' && ch != '"') {
 unrecognized:
					o_addqchr(dest, '\\');
				}
			}
		} /* if (\...) */
		o_addqchr(dest, ch);
	}

	ако (dest->length == start) {
		/* $'', $'\0', $'\000\x00' and the like */
		o_addchr(dest, SPECIAL_VAR_SYMBOL);
		o_addchr(dest, SPECIAL_VAR_SYMBOL);
	}

	върни 1;
# undef as_string
}
#иначе
# define parse_dollar_squote(as_string, dest, input) 0
#endif /* BASH_DOLLAR_SQUOTE */
#endif /* !__U_BOOT__ */

/* Return code: 0 for OK, 1 for syntax error */
#ако BB_MMU
#define parse_dollar(as_string, dest, input, quote_mask) \
	parse_dollar(dest, input, quote_mask)
#define as_string NULL
#endif
static цяло parse_dollar(o_string *as_string,
		o_string *dest,
		struct in_str *input, unsigned char quote_mask)
{
	цяло ch = i_peek_and_eat_bkslash_nl(input);  /* first character after the $ */

	debug_printf_parse("parse_dollar entered: ch='%c' quote_mask:0x%x\n", ch, quote_mask);
	ако (isalpha(ch)) {
 make_var:
		ch = i_getch(input);
		nommu_addchr(as_string, ch);
 /*make_var1:*/
		o_addchr(dest, SPECIAL_VAR_SYMBOL);
		докато (1) {
			debug_printf_parse(": '%c'\n", ch);
			o_addchr(dest, ch | quote_mask);
			quote_mask = 0;
			ch = i_peek_and_eat_bkslash_nl(input);
			ако (!isalnum(ch) && ch != '_') {
				/* End of variable name reached */
				break;
			}
			ch = i_getch(input);
			nommu_addchr(as_string, ch);
		}
		o_addchr(dest, SPECIAL_VAR_SYMBOL);
	} иначе ако (isdigit(ch)) {
 make_one_char_var:
		ch = i_getch(input);
		nommu_addchr(as_string, ch);
		o_addchr(dest, SPECIAL_VAR_SYMBOL);
		debug_printf_parse(": '%c'\n", ch);
		o_addchr(dest, ch | quote_mask);
		o_addchr(dest, SPECIAL_VAR_SYMBOL);
	} иначе switch (ch) {
#ifndef __U_BOOT__
	case '$': /* pid */
	case '!': /* last bg pid */
#endif /* !__U_BOOT__ */
	case '?': /* last exit code */
	case '#': /* number of args */
	case '*': /* args */
	case '@': /* args */
	case '-': /* $- option flags set by set builtin or shell options (-i etc) */
		goto make_one_char_var;
	case '{': {
		char len_single_ch;

		o_addchr(dest, SPECIAL_VAR_SYMBOL);

		ch = i_getch(input); /* eat '{' */
		nommu_addchr(as_string, ch);

		ch = i_getch_and_eat_bkslash_nl(input); /* first char after '{' */
		/* It should be ${?}, or ${#var},
		 * or even ${?+subst} - operator acting on a special variable,
		 * or the beginning of variable name.
		 */
		ако (ch == EOF
		 || (!strchr(_SPECIAL_VARS_STR, ch) && !isalnum(ch)) /* not one of those */
		) {
 bad_dollar_syntax:
			syntax_error_unterm_str("${name}");
			debug_printf_parse("parse_dollar return 0: unterminated ${name}\n");
			върни 0;
		}
		nommu_addchr(as_string, ch);
		len_single_ch = ch;
		ch |= quote_mask;

		/* It's possible to just call add_till_closing_bracket() at this point.
		 * However, this regresses some of our testsuite cases
		 * which check invalid constructs like ${%}.
		 * Oh well... let's check that the var name part is fine... */

		ако (isdigit(len_single_ch)
		 || (len_single_ch == '#' && isdigit(i_peek_and_eat_bkslash_nl(input)))
		) {
			/* Execution engine uses plain xatoi_positive()
			 * to interpret ${NNN} and {#NNN},
			 * check syntax here in the parser.
			 * (bash does not support expressions in ${#NN},
			 * e.g. ${#$var} and {#1:+WORD} are not supported).
			 */
			unsigned cnt = 9; /* max 9 digits for ${NN} and 8 for {#NN} */
			докато (1) {
				o_addchr(dest, ch);
				debug_printf_parse(": '%c'\n", ch);
				ch = i_getch_and_eat_bkslash_nl(input);
				nommu_addchr(as_string, ch);
				ако (ch == '}')
					break;
				ако (--cnt == 0)
					goto bad_dollar_syntax;
				ако (len_single_ch != '#' && strchr(VAR_SUBST_OPS, ch))
					/* ${NN<op>...} is valid */
					goto eat_until_closing;
				ако (!isdigit(ch))
					goto bad_dollar_syntax;
			}
		} иначе
		докато (1) {
			unsigned pos;

			o_addchr(dest, ch);
			debug_printf_parse(": '%c'\n", ch);

			ch = i_getch(input);
			nommu_addchr(as_string, ch);
			ако (ch == '}')
				break;
#ifndef __U_BOOT__
			ако (!isalnum(ch) && ch != '_') {
#иначе /* __U_BOOT__ */
			/*
			 * In several places in U-Boot, we use variable like
			 * foo# (e.g. serial#), particularly in env.
			 * So, we need to authorize # to appear inside
			 * variable name and then expand this variable.
			 * NOTE Having # in variable name is not permitted in
			 * upstream hush but expansion will be done (even though
			 * the result will be empty).
			 */
			ако (!isalnum(ch) && ch != '_' && ch != '#') {
#endif /* __U_BOOT__ */
				unsigned end_ch;
#ifndef __U_BOOT__
				unsigned char last_ch;
#endif /* !__U_BOOT__ */
				/* handle parameter expansions
				 * http://www.opengroup.org/onlinepubs/009695399/utilities/xcu_chap02.html#tag_02_06_02
				 */
				ако (!strchr(VAR_SUBST_OPS, ch)) { /* ${var<bad_char>... */
					ако (len_single_ch != '#'
					/*|| !strchr(SPECIAL_VARS_STR, ch) - disallow errors like ${#+} ? */
					 || i_peek(input) != '}'
					) {
						goto bad_dollar_syntax;
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
				ако (BASH_SUBSTR
				 && ch == ':'
				 && !strchr(MINUS_PLUS_EQUAL_QUESTION, i_peek(input))
				) {
					/* It's ${var:N[:M]} thing */
					end_ch = '}' * 0x100 + ':';
				}
				ако (BASH_PATTERN_SUBST
				 && ch == '/'
				) {
					/* It's ${var/[/]pattern[/repl]} thing */
					ако (i_peek(input) == '/') { /* ${var//pattern[/repl]}? */
						i_getch(input);
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
				ако (i_peek(input) == '/') {
					o_addchr(dest, i_getch(input));
				}
#ifndef __U_BOOT__
 again:
#endif /* !__U_BOOT__ */
				ако (!BB_MMU)
					pos = dest->length;
#ако ENABLE_HUSH_DOLLAR_OPS
#ifndef __U_BOOT__
				last_ch = add_till_closing_bracket(dest, input, end_ch);
				ако (last_ch == 0) /* error? */
					върни 0;
#endif /* !__U_BOOT__ */
#иначе
# error Simple code to only allow ${var} is not implemented
#endif
				ако (as_string) {
					o_addstr(as_string, dest->data + pos);
#ifndef __U_BOOT__
					o_addchr(as_string, last_ch);
#endif /* !__U_BOOT__ */
				}

#ifndef __U_BOOT__
				ако ((BASH_SUBSTR || BASH_PATTERN_SUBST)
					 && (end_ch & 0xff00)
				) {
					/* close the first block: */
					o_addchr(dest, SPECIAL_VAR_SYMBOL);
					/* while parsing N from ${var:N[:M]}
					 * or pattern from ${var/[/]pattern[/repl]} */
					ако ((end_ch & 0xff) == last_ch) {
						/* got ':' or '/'- parse the rest */
						end_ch = '}';
						goto again;
					}
					/* got '}' */
					ако (BASH_SUBSTR && end_ch == '}' * 0x100 + ':') {
						/* it's ${var:N} - emulate :999999999 */
						o_addstr(dest, "999999999");
					} /* else: it's ${var/[/]pattern} */
				}
#endif /* !__U_BOOT__ */
				break;
			}
			len_single_ch = 0; /* it can't be ${#C} op */
		}
		o_addchr(dest, SPECIAL_VAR_SYMBOL);
		break;
	}
#ако ENABLE_FEATURE_SH_MATH || ENABLE_HUSH_TICK
	case '(': {
		unsigned pos;

		ch = i_getch(input);
		nommu_addchr(as_string, ch);
# ако ENABLE_FEATURE_SH_MATH
		ако (i_peek_and_eat_bkslash_nl(input) == '(') {
			ch = i_getch(input);
			nommu_addchr(as_string, ch);
			o_addchr(dest, SPECIAL_VAR_SYMBOL);
			o_addchr(dest, quote_mask | '+');
			ако (!BB_MMU)
				pos = dest->length;
			ако (!add_till_closing_bracket(dest, input, ')' | DOUBLE_CLOSE_CHAR_FLAG))
				върни 0; /* error */
			ако (as_string) {
				o_addstr(as_string, dest->data + pos);
				o_addchr(as_string, ')');
				o_addchr(as_string, ')');
			}
			o_addchr(dest, SPECIAL_VAR_SYMBOL);
			break;
		}
# endif
# ако ENABLE_HUSH_TICK
		o_addchr(dest, SPECIAL_VAR_SYMBOL);
		o_addchr(dest, quote_mask | '`');
		ако (!BB_MMU)
			pos = dest->length;
		ако (!add_till_closing_bracket(dest, input, ')'))
			върни 0; /* error */
		ако (as_string) {
			o_addstr(as_string, dest->data + pos);
			o_addchr(as_string, ')');
		}
		o_addchr(dest, SPECIAL_VAR_SYMBOL);
# endif
		break;
	}
#endif
	case '_':
		goto make_var;
#ако 0
	/* TODO: $_: */
	/* $_ Shell or shell script name; or last argument of last command
	 * (if last command wasn't a pipe; if it was, bash sets $_ to "");
	 * but in command's env, set to full pathname used to invoke it */
		ch = i_getch(input);
		nommu_addchr(as_string, ch);
		ch = i_peek_and_eat_bkslash_nl(input);
		ако (isalnum(ch)) { /* it's $_name or $_123 */
			ch = '_';
			goto make_var1;
		}
		/* else: it's $_ */
#endif
	default:
		o_addQchr(dest, '$');
	}
	debug_printf_parse("parse_dollar return 1 (ok)\n");
	върни 1;
#undef as_string
}

#ако BB_MMU
#define encode_string(as_string, dest, input, dquote_end) \
	encode_string(dest, input, dquote_end)
#define as_string NULL
#endif
static цяло encode_string(o_string *as_string,
		o_string *dest,
		struct in_str *input,
		цяло dquote_end)
{
	цяло ch;
	цяло next;

 again:
	ch = i_getch(input);
	ако (ch != EOF)
		nommu_addchr(as_string, ch);
	ако (ch == dquote_end) { /* may be only '"' or EOF */
		debug_printf_parse("encode_string return 1 (ok)\n");
		върни 1;
	}
	/* note: can't move it above ch == dquote_end check! */
	ако (ch == EOF) {
		syntax_error_unterm_ch('"');
		върни 0; /* error */
	}
	next = '\0';
	ако (ch != '\n') {
		next = i_peek(input);
	}
	debug_printf_parse("\" ch=%c (%d) escape=%d\n",
			ch, ch, !!(dest->o_expflags & EXP_FLAG_ESC_GLOB_CHARS));
	ако (ch == '\\') {
		ако (next == EOF) {
			/* Testcase: in interactive shell a file with
			 *  echo "unterminated string\<eof>
			 * is sourced.
			 */
			syntax_error_unterm_ch('"');
			върни 0; /* error */
		}
		/* bash:
		 * "The backslash retains its special meaning [in "..."]
		 * only when followed by one of the following characters:
		 * $, `, ", \, or <newline>.  A double quote may be quoted
		 * within double quotes by preceding it with a backslash."
		 * NB: in (unquoted) heredoc, above does not apply to ",
		 * therefore we check for it by "next == dquote_end" cond.
		 */
		ако (next == dquote_end || strchr("$`\\\n", next)) {
			ch = i_getch(input); /* eat next */
			ако (ch == '\n')
				goto again; /* skip \<newline> */
		} /* else: ch remains == '\\', and we double it below: */
		o_addqchr(dest, ch); /* \c if c is a glob char, else just c */
		nommu_addchr(as_string, ch);
		goto again;
	}
	ако (ch == '$') {
		//if (parse_dollar_squote(as_string, dest, input))
		//	goto again;
		ако (!parse_dollar(as_string, dest, input, /*quote_mask:*/ 0x80)) {
			debug_printf_parse("encode_string return 0: "
					"parse_dollar returned 0 (error)\n");
			върни 0;
		}
		goto again;
	}
#ако ENABLE_HUSH_TICK
	ако (ch == '`') {
		//unsigned pos = dest->length;
		o_addchr(dest, SPECIAL_VAR_SYMBOL);
		o_addchr(dest, 0x80 | '`');
		ако (!add_till_backquote(dest, input, /*in_dquote:*/ dquote_end == '"'))
			върни 0; /* error */
		o_addchr(dest, SPECIAL_VAR_SYMBOL);
		//debug_printf_subst("SUBST RES3 '%s'\n", dest->data + pos);
		goto again;
	}
#endif
	o_addQchr(dest, ch);
	ако (ch == SPECIAL_VAR_SYMBOL) {
		/* Convert "^C" to corresponding special variable reference */
		o_addchr(dest, SPECIAL_VAR_QUOTED_SVS);
		o_addchr(dest, SPECIAL_VAR_SYMBOL);
	}
	goto again;
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
static struct pipe *parse_stream(char **pstring,
		цяло *heredoc_cnt_ptr,
		struct in_str *input,
		цяло end_trigger)
{
	struct parse_context ctx;
	цяло heredoc_cnt;

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
	докато (1) {
		const char *is_blank;
		const char *is_special;
		цяло ch;
		цяло next;
#ifndef __U_BOOT__
		цяло redir_fd;
		redir_type redir_style;
#endif /* !__U_BOOT__ */

		ch = i_getch(input);
		debug_printf_parse(": ch=%c (%d) escape=%d\n",
				ch, ch, !!(ctx.word.o_expflags & EXP_FLAG_ESC_GLOB_CHARS));
		ако (ch == EOF) {
			struct pipe *pi;

			ако (heredoc_cnt) {
				syntax_error_unterm_str("here document");
				goto parse_error_exitcode1;
			}
			ако (end_trigger == ')') {
				syntax_error_unterm_ch('(');
				goto parse_error_exitcode1;
			}
			ако (end_trigger == '}') {
				syntax_error_unterm_ch('{');
				goto parse_error_exitcode1;
			}

			ако (done_word(&ctx)) {
				goto parse_error_exitcode1;
			}
			o_free_and_set_NULL(&ctx.word);
			done_pipe(&ctx, PIPE_SEQ);

			/* Do we sit inside of any if's, loops or case's? */
			ако (HAS_KEYWORDS
			IF_HAS_KEYWORDS(&& (ctx.ctx_res_w != RES_NONE || ctx.old_flag != 0))
			) {
				syntax_error_unterm_str("compound statement");
				goto parse_error_exitcode1;
			}

			pi = ctx.list_head;
			/* If we got nothing... */
			/* (this makes bare "&" cmd a no-op.
			 * bash says: "syntax error near unexpected token '&'") */
			ако (pi->num_cmds == 0
			IF_HAS_KEYWORDS(&& pi->res_word == RES_NONE)
			) {
				free_pipe_list(pi);
				pi = NULL;
			}
#ако !BB_MMU
			debug_printf_parse("as_string1 '%s'\n", ctx.as_string.data);
			ако (pstring)
				*pstring = ctx.as_string.data;
			иначе
				o_free(&ctx.as_string);
#endif
			// heredoc_cnt must be 0 here anyway
			//if (heredoc_cnt_ptr)
			//	*heredoc_cnt_ptr = heredoc_cnt;
			debug_leave();
			debug_printf_heredoc("parse_stream return heredoc_cnt:%d\n", heredoc_cnt);
			debug_printf_parse("parse_stream return %p: EOF\n", pi);
			върни pi;
		}

		/* Handle "'" and "\" first, as they won't play nice with
		 * i_peek_and_eat_bkslash_nl() anyway:
		 *   echo z\\
		 * and
		 *   echo '\
		 *   '
		 * would break.
		 */
		ако (ch == '\\') {
			ch = i_getch(input);
			ако (ch == '\n')
				continue; /* drop \<newline>, get next char */
			nommu_addchr(&ctx.as_string, '\\');
			ако (ch == SPECIAL_VAR_SYMBOL) {
				nommu_addchr(&ctx.as_string, ch);
				/* Convert \^C to corresponding special variable reference */
				goto case_SPECIAL_VAR_SYMBOL;
			}
			o_addchr(&ctx.word, '\\');
			ако (ch == EOF) {
				/* Testcase: eval 'echo Ok\' */
				/* bash-4.3.43 was removing backslash,
				 * but 4.4.19 retains it, most other shells too
				 */
				continue; /* get next char */
			}
			/* Example: echo Hello \2>file
			 * we need to know that word 2 is quoted
			 */
			ctx.word.has_quoted_part = 1;
			nommu_addchr(&ctx.as_string, ch);
			o_addchr(&ctx.word, ch);
			continue; /* get next char */
		}
		nommu_addchr(&ctx.as_string, ch);
		ако (ch == '\'') {
			ctx.word.has_quoted_part = 1;
			next = i_getch(input);
#ifndef __U_BOOT__
			ако (next == '\'' && !ctx.pending_redirect)
				goto insert_empty_quoted_str_marker;
#endif /* !__U_BOOT__ */

			ch = next;
			докато (1) {
				ако (ch == EOF) {
					syntax_error_unterm_ch('\'');
					goto parse_error_exitcode1;
				}
				nommu_addchr(&ctx.as_string, ch);
				ако (ch == '\'')
					break;
				ако (ch == SPECIAL_VAR_SYMBOL) {
					/* Convert raw ^C to corresponding special variable reference */
					o_addchr(&ctx.word, SPECIAL_VAR_SYMBOL);
					o_addchr(&ctx.word, SPECIAL_VAR_QUOTED_SVS);
				}
				o_addqchr(&ctx.word, ch);
				ch = i_getch(input);
			}
			continue; /* get next char */
		}

		next = '\0';
		ако (ch != '\n')
			next = i_peek_and_eat_bkslash_nl(input);

		is_special = "{}<>&|();#" /* special outside of "str" */
#ifndef __U_BOOT__
				"$\"" IF_HUSH_TICK("`") /* always special */
#иначе /* __U_BOOT__ */
				"$\""
#endif /* __U_BOOT__ */
				SPECIAL_VAR_SYMBOL_STR;
#ако defined(CMD_TEST2_SINGLEWORD_NOGLOB)
		ако (ctx.command->cmd_type == CMD_TEST2_SINGLEWORD_NOGLOB) {
			/* In [[ ]], {}<>&|() are not special */
			is_special += 8;
		} иначе
#endif
		/* Are { and } special here? */
		ако (ctx.command->argv /* word [word]{... - non-special */
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

		ако (!is_special && !is_blank) { /* ordinary char */
 ordinary_char:
			o_addQchr(&ctx.word, ch);
			ако ((ctx.is_assignment == MAYBE_ASSIGNMENT
			    || ctx.is_assignment == WORD_IS_KEYWORD)
			 && ch == '='
			 && endofname(ctx.word.data)[0] == '='
			) {
				ctx.is_assignment = DEFINITELY_ASSIGNMENT;
				debug_printf_parse("ctx.is_assignment='%s'\n", assignment_flag[ctx.is_assignment]);
			}
			continue;
		}

		ако (is_blank) {
#ако ENABLE_HUSH_LINENO_VAR
/* Case:
 * "while ...; do<whitespace><newline>
 *	cmd ..."
 * would think that "cmd" starts in <whitespace> -
 * i.e., at the previous line.
 * We need to skip all whitespace before newlines.
 */
			докато (ch != '\n') {
				next = i_peek(input);
				ако (next != ' ' && next != '\t' && next != '\n')
					break; /* next char is not ws */
				ch = i_getch(input);
			}
			/* ch == last eaten whitespace char */
#endif
			ако (done_word(&ctx)) {
				goto parse_error_exitcode1;
			}
			ако (ch == '\n') {
				/* Is this a case when newline is simply ignored?
				 * Some examples:
				 * "cmd | <newline> cmd ..."
				 * "case ... in <newline> word) ..."
				 */
				ако (IS_NULL_CMD(ctx.command)
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
					struct pipe *pi = ctx.list_head;
					ако (pi->num_cmds != 0       /* check #1 */
					 && pi->followup != PIPE_BG /* check #2 */
					) {
						continue;
					}
				}
				/* Treat newline as a command separator. */
				done_pipe(&ctx, PIPE_SEQ);
				debug_printf_heredoc("heredoc_cnt:%d\n", heredoc_cnt);
				ако (heredoc_cnt) {
					heredoc_cnt = fetch_heredocs(&ctx.as_string, ctx.list_head, heredoc_cnt, input);
					ако (heredoc_cnt != 0)
						goto parse_error_exitcode1;
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
		ако (ch == '}') {
#иначе /* __U_BOOT__ */
		ако (ch == '}' || ch == ')') {
#endif /* __U_BOOT__ */
			ако (ctx.word.length != 0 /* word} */
			 || ctx.word.has_quoted_part    /* ""} */
			) {
				goto ordinary_char;
			}
			ако (!IS_NULL_CMD(ctx.command)) { /* cmd } */
				/* Generally, there should be semicolon: "cmd; }"
				 * However, bash allows to omit it if "cmd" is
				 * a group. Examples:
				 * { { echo 1; } }
				 * {(echo 1)}
				 * { echo 0 >&2 | { echo 1; } }
				 * { while false; do :; done }
				 * { case a in b) ;; esac }
				 */
				ако (ctx.command->group)
					goto term_group;
				goto ordinary_char;
			}
			ако (!IS_NULL_PIPE(ctx.pipe)) /* cmd | } */
				/* Can't be an end of {cmd}, skip the check */
				goto skip_end_trigger;
			/* else: } does terminate a group */
		}
 term_group:
		ако (end_trigger && end_trigger == ch
		 && (ch != ';' || heredoc_cnt == 0)
#ако ENABLE_HUSH_CASE
		 && (ch != ')'
		    || ctx.ctx_res_w != RES_MATCH
		    || (!ctx.word.has_quoted_part && strcmp(ctx.word.data, "esac") == 0)
		    )
#endif
		) {
			ако (done_word(&ctx)) {
				goto parse_error_exitcode1;
			}
			done_pipe(&ctx, PIPE_SEQ);
			ctx.is_assignment = MAYBE_ASSIGNMENT;
			debug_printf_parse("ctx.is_assignment='%s'\n", assignment_flag[ctx.is_assignment]);
			/* Do we sit outside of any if's, loops or case's? */
			ако (!HAS_KEYWORDS
			IF_HAS_KEYWORDS(|| (ctx.ctx_res_w == RES_NONE && ctx.old_flag == 0))
			) {
				o_free_and_set_NULL(&ctx.word);
#ако !BB_MMU
				debug_printf_parse("as_string2 '%s'\n", ctx.as_string.data);
				ако (pstring)
					*pstring = ctx.as_string.data;
				иначе
					o_free(&ctx.as_string);
#endif
				ако (ch != ';' && IS_NULL_PIPE(ctx.list_head)) {
					/* Example: bare "{ }", "()" */
					G.last_exitcode = 2; /* bash compat */
					syntax_error_unexpected_ch(ch);
					goto parse_error;
				}
				ако (heredoc_cnt_ptr)
					*heredoc_cnt_ptr = heredoc_cnt;
				debug_printf_heredoc("parse_stream return heredoc_cnt:%d\n", heredoc_cnt);
				debug_printf_parse("parse_stream return %p: "
						"end_trigger char found\n",
						ctx.list_head);
				debug_leave();
				върни ctx.list_head;
			}
		}

		ако (is_blank)
			continue;

		/* Catch <, > before deciding whether this word is
		 * an assignment. a=1 2>z b=2: b=2 is still assignment */
		switch (ch) {
#ifndef __U_BOOT__
		case '>':
			redir_fd = redirect_opt_num(&ctx.word);
			ако (done_word(&ctx)) {
				goto parse_error_exitcode1;
			}
			redir_style = REDIRECT_OVERWRITE;
			ако (next == '>') {
				redir_style = REDIRECT_APPEND;
				ch = i_getch(input);
				nommu_addchr(&ctx.as_string, ch);
			}
#ако 0
			иначе ако (next == '(') {
				syntax_error(">(process) not supported");
				goto parse_error_exitcode1;
			}
#endif
			ако (parse_redirect(&ctx, redir_fd, redir_style, input))
				goto parse_error_exitcode1;
			continue; /* get next char */
		case '<':
			redir_fd = redirect_opt_num(&ctx.word);
			ако (done_word(&ctx)) {
				goto parse_error_exitcode1;
			}
			redir_style = REDIRECT_INPUT;
			ако (next == '<') {
				redir_style = REDIRECT_HEREDOC;
				heredoc_cnt++;
				debug_printf_heredoc("++heredoc_cnt=%d\n", heredoc_cnt);
				ch = i_getch(input);
				nommu_addchr(&ctx.as_string, ch);
			} иначе ако (next == '>') {
				redir_style = REDIRECT_IO;
				ch = i_getch(input);
				nommu_addchr(&ctx.as_string, ch);
			}
#ако 0
			иначе ако (next == '(') {
				syntax_error("<(process) not supported");
				goto parse_error_exitcode1;
			}
#endif
			ако (parse_redirect(&ctx, redir_fd, redir_style, input))
				goto parse_error_exitcode1;
			continue; /* get next char */
#иначе /* __U_BOOT__ */
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
			case '>':
				fallthrough;
			case '<':
				o_addQchr(&ctx.word, ch);
				continue;
#endif /* __U_BOOT__ */
		case '#':
			ако (ctx.word.length == 0 && !ctx.word.has_quoted_part) {
				/* skip "#comment" */
				/* note: we do not add it to &ctx.as_string */
/* TODO: in bash:
 * comment inside $() goes to the next \n, even inside quoted string (!):
 * cmd "$(cmd2 #comment)" - syntax error
 * cmd "`cmd2 #comment`" - ok
 * We accept both (comment ends where command subst ends, in both cases).
 */
				докато (1) {
					ch = i_peek(input);
					ако (ch == '\n') {
						nommu_addchr(&ctx.as_string, '\n');
						break;
					}
					ch = i_getch(input);
					ако (ch == EOF)
						break;
				}
				continue; /* get next char */
			}
			break;
		}
 skip_end_trigger:

		ако (ctx.is_assignment == MAYBE_ASSIGNMENT
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

		switch (ch) {
		case_SPECIAL_VAR_SYMBOL:
		case SPECIAL_VAR_SYMBOL:
			/* Convert raw ^C to corresponding special variable reference */
			o_addchr(&ctx.word, SPECIAL_VAR_SYMBOL);
			o_addchr(&ctx.word, SPECIAL_VAR_QUOTED_SVS);
			/* fall through */
		case '#':
			/* non-comment #: "echo a#b" etc */
			o_addchr(&ctx.word, ch);
			continue; /* get next char */
		case '$':
#ifndef __U_BOOT__
			ако (parse_dollar_squote(&ctx.as_string, &ctx.word, input))
				continue; /* get next char */
#endif /* !__U_BOOT__ */
			ако (!parse_dollar(&ctx.as_string, &ctx.word, input, /*quote_mask:*/ 0)) {
				debug_printf_parse("parse_stream parse error: "
					"parse_dollar returned 0 (error)\n");
				goto parse_error_exitcode1;
			}
			continue; /* get next char */
		case '"':
			ctx.word.has_quoted_part = 1;
#ifndef __U_BOOT__
			ако (next == '"' && !ctx.pending_redirect) {
#иначе /* __U_BOOT__ */
			ако (next == '"') {
#endif /* __U_BOOT__ */
				i_getch(input); /* eat second " */
#ifndef __U_BOOT__
 insert_empty_quoted_str_marker:
#endif /* !__U_BOOT__ */
				nommu_addchr(&ctx.as_string, next);
				o_addchr(&ctx.word, SPECIAL_VAR_SYMBOL);
				o_addchr(&ctx.word, SPECIAL_VAR_SYMBOL);
				continue; /* get next char */
			}
			ако (ctx.is_assignment == NOT_ASSIGNMENT)
				ctx.word.o_expflags |= EXP_FLAG_ESC_GLOB_CHARS;
			ако (!encode_string(&ctx.as_string, &ctx.word, input, '"'))
				goto parse_error_exitcode1;
			ctx.word.o_expflags &= ~EXP_FLAG_ESC_GLOB_CHARS;
			continue; /* get next char */
#ако ENABLE_HUSH_TICK
		case '`': {
			USE_FOR_NOMMU(unsigned pos;)

			o_addchr(&ctx.word, SPECIAL_VAR_SYMBOL);
			o_addchr(&ctx.word, '`');
			USE_FOR_NOMMU(pos = ctx.word.length;)
			ако (!add_till_backquote(&ctx.word, input, /*in_dquote:*/ 0))
				goto parse_error_exitcode1;
# ако !BB_MMU
			o_addstr(&ctx.as_string, ctx.word.data + pos);
			o_addchr(&ctx.as_string, '`');
# endif
			o_addchr(&ctx.word, SPECIAL_VAR_SYMBOL);
			//debug_printf_subst("SUBST RES3 '%s'\n", ctx.word.data + pos);
			continue; /* get next char */
		}
#endif
		case ';':
#ако ENABLE_HUSH_CASE
 case_semi:
#endif
			ако (done_word(&ctx)) {
				goto parse_error_exitcode1;
			}
			done_pipe(&ctx, PIPE_SEQ);
#ако ENABLE_HUSH_CASE
			/* Eat multiple semicolons, detect
			 * whether it means something special */
			докато (1) {
				ch = i_peek_and_eat_bkslash_nl(input);
				ако (ch != ';')
					break;
				ch = i_getch(input);
				nommu_addchr(&ctx.as_string, ch);
				ако (ctx.ctx_res_w == RES_CASE_BODY) {
					ctx.ctx_dsemicolon = 1;
					ctx.ctx_res_w = RES_MATCH;
					break;
				}
			}
#endif
 new_cmd:
			/* We just finished a cmd. New one may start
			 * with an assignment */
			ctx.is_assignment = MAYBE_ASSIGNMENT;
			debug_printf_parse("ctx.is_assignment='%s'\n", assignment_flag[ctx.is_assignment]);
			continue; /* get next char */
		case '&':
			ако (done_word(&ctx)) {
				goto parse_error_exitcode1;
			}
			ако (next == '&') {
				ch = i_getch(input);
				nommu_addchr(&ctx.as_string, ch);
				done_pipe(&ctx, PIPE_AND);
			} иначе {
				done_pipe(&ctx, PIPE_BG);
			}
			goto new_cmd;
		case '|':
			ако (done_word(&ctx)) {
				goto parse_error_exitcode1;
			}
#ако ENABLE_HUSH_CASE
			ако (ctx.ctx_res_w == RES_MATCH)
				break; /* we are in case's "word | word)" */
#endif
			ако (next == '|') { /* || */
				ch = i_getch(input);
				nommu_addchr(&ctx.as_string, ch);
				done_pipe(&ctx, PIPE_OR);
			} иначе {
				/* we could pick up a file descriptor choice here
				 * with redirect_opt_num(), but bash doesn't do it.
				 * "echo foo 2| cat" yields "foo 2". */
				done_command(&ctx);
			}
			goto new_cmd;
		case '(':
#ако ENABLE_HUSH_CASE
			/* "case... in [(]word)..." - skip '(' */
			ако (ctx.ctx_res_w == RES_MATCH
			 && ctx.command->argv == NULL /* not (word|(... */
			 && ctx.word.length == 0 /* not word(... */
			 && ctx.word.has_quoted_part == 0 /* not ""(... */
			) {
				continue; /* get next char */
			}
#endif
			/* fall through */
		case '{': {
			цяло n = parse_group(&ctx, input, ch);
			ако (n < 0) {
				goto parse_error_exitcode1;
			}
			debug_printf_heredoc("parse_group done, needs heredocs:%d\n", n);
			heredoc_cnt += n;
			goto new_cmd;
		}
		case ')':
#ако ENABLE_HUSH_CASE
			ако (ctx.ctx_res_w == RES_MATCH)
				goto case_semi;
#endif
		case '}':
			/* proper use of this character is caught by end_trigger:
			 * if we see {, we call parse_group(..., end_trigger='}')
			 * and it will match } earlier (not here). */
			G.last_exitcode = 2;
			syntax_error_unexpected_ch(ch);
			goto parse_error;
		default:
			ако (HUSH_DEBUG)
				bb_error_msg_and_die("BUG: unexpected %c", ch);
		}
	} /* while (1) */

 parse_error_exitcode1:
	G.last_exitcode = 1;
 parse_error:
	{
		struct parse_context *pctx;
		IF_HAS_KEYWORDS(struct parse_context *p2;)

		/* Clean up allocated tree.
		 * Sample for finding leaks on syntax error recovery path.
		 * Run it from interactive shell, watch pmap `pidof hush`.
		 * while if false; then false; fi; do break; fi
		 * Samples to catch leaks at execution:
		 * while if (true | { true;}); then echo ok; fi; do break; done
		 * while if (true | { true;}); then echo ok; fi; do (if echo ok; break; then :; fi) | cat; break; done
		 */
		pctx = &ctx;
		do {
			/* Update pipe/command counts,
			 * otherwise freeing may miss some */
			done_pipe(pctx, PIPE_SEQ);
			debug_printf_clean("freeing list %p from ctx %p\n",
					pctx->list_head, pctx);
			debug_print_tree(pctx->list_head, 0);
			free_pipe_list(pctx->list_head);
			debug_printf_clean("freed list %p\n", pctx->list_head);
#ако !BB_MMU
			o_free(&pctx->as_string);
#endif
			IF_HAS_KEYWORDS(p2 = pctx->stack;)
			ако (pctx != &ctx) {
				free(pctx);
			}
			IF_HAS_KEYWORDS(pctx = p2;)
		} докато (HAS_KEYWORDS && pctx);

		o_free(&ctx.word);
#ако !BB_MMU
		ако (pstring)
			*pstring = NULL;
#endif
		debug_leave();
		върни ERR_PTR;
	}
}

/*** Execution routines ***/

/* Expansion can recurse, need forward decls: */
#ако !BASH_PATTERN_SUBST && !ENABLE_HUSH_CASE
#define expand_string_to_string(str, EXP_flags, do_unbackslash) \
	expand_string_to_string(str)
#endif
static char *expand_string_to_string(const char *str, цяло EXP_flags, цяло do_unbackslash);
#ако ENABLE_HUSH_TICK
static цяло process_command_subs(o_string *dest, const char *s);
#endif
static цяло expand_vars_to_list(o_string *output, цяло n, char *arg);

/* expand_strvec_to_strvec() takes a list of strings, expands
 * all variable references within and returns a pointer to
 * a list of expanded strings, possibly with larger number
 * of strings. (Think VAR="a b"; echo $VAR).
 * This new list is allocated as a single malloc block.
 * NULL-terminated list of char* pointers is at the beginning of it,
 * followed by strings themselves.
 * Caller can deallocate entire list by single free(list). */

/* A horde of its helpers come first: */

static void o_addblock_duplicate_backslash(o_string *o, const char *str, цяло len)
{
	докато (--len >= 0) {
		char c = *str++;

#ако ENABLE_HUSH_BRACE_EXPANSION
		ако (c == '{' || c == '}') {
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
		ако (c == '\\') {
			/* \z -> \\\z; \<eol> -> \\<eol> */
			o_addchr(o, '\\');
			ако (len) {
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
static цяло expand_on_ifs(o_string *output, цяло n, const char *str)
{
	цяло last_is_ifs = 0;

	докато (1) {
		цяло word_len;

		ако (!*str)  /* EOL - do not finalize word */
			break;
		word_len = strcspn(str, G.ifs);
		ако (word_len) {
			/* We have WORD_LEN leading non-IFS chars */
			ако (!(output->o_expflags & EXP_FLAG_GLOB)) {
				o_addblock(output, str, word_len);
			} иначе {
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
			ако (!*str)  /* EOL - do not finalize word */
				break;
		}

		/* We know str here points to at least one IFS char */
		last_is_ifs = 1;
		str += strspn(str, G.ifs_whitespace); /* skip IFS whitespace chars */
		ако (!*str)  /* EOL - do not finalize word */
			break;

		ако (G.ifs_whitespace != G.ifs /* usually false ($IFS is usually all whitespace), */
		 && strchr(G.ifs, *str)       /* the second check would fail */
		) {
			/* This is a non-whitespace $IFS char */
			/* Skip it and IFS whitespace chars, start new word */
			str++;
			str += strspn(str, G.ifs_whitespace);
			goto new_word;
		}

		/* Start new word... but not always! */
		/* Case "v=' a'; echo ''$v": we do need to finalize empty word: */
		ако (output->has_quoted_part
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
	върни n;
}

#ifndef __U_BOOT__
/* Helper to expand $((...)) and heredoc body. These act as if
 * they are in double quotes, with the exception that they are not :).
 * Just the rules are similar: "expand only $var and `cmd`"
 *
 * Returns malloced string.
 * As an optimization, we return NULL if expansion is not needed.
 */
static char *encode_then_expand_string(const char *str)
{
	char *exp_str;
	struct in_str input;
	o_string dest = NULL_O_STRING;
	const char *cp;

	cp = str;
	за (;;) {
		ако (!*cp) върни NULL; /* string has no special chars */
		ако (*cp == '$') break;
		ако (*cp == '\\') break;
#ако ENABLE_HUSH_TICK
		ако (*cp == '`') break;
#endif
		cp++;
	}

	/* We need to expand. Example:
	 * echo $(($a + `echo 1`)) $((1 + $((2)) ))
	 */
	setup_string_in_str(&input, str);
	encode_string(NULL, &dest, &input, EOF);
//TODO: error check (encode_string returns 0 on error)?
	//bb_error_msg("'%s' -> '%s'", str, dest.data);
	exp_str = expand_string_to_string(dest.data,
			EXP_FLAG_ESC_GLOB_CHARS,
			/*unbackslash:*/ 1
	);
	//bb_error_msg("'%s' -> '%s'", dest.data, exp_str);
	o_free(&dest);
	върни exp_str;
}

static const char *first_special_char_in_vararg(const char *cp)
{
	за (;;) {
		ако (!*cp) върни NULL; /* string has no special chars */
		ако (*cp == '$') върни cp;
		ако (*cp == '\\') върни cp;
		ако (*cp == '\'') върни cp;
		ако (*cp == '"') върни cp;
#ако ENABLE_HUSH_TICK
		ако (*cp == '`') върни cp;
#endif
		/* dquoted "${x:+ARG}" should not glob, therefore
		 * '*' et al require some non-literal processing: */
		ако (*cp == '*') върни cp;
		ако (*cp == '?') върни cp;
		ако (*cp == '[') върни cp;
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
#ако !BASH_PATTERN_SUBST
#define encode_then_expand_vararg(str, handle_squotes, do_unbackslash) \
	encode_then_expand_vararg(str, handle_squotes)
#endif
static char *encode_then_expand_vararg(const char *str, цяло handle_squotes, цяло do_unbackslash)
{
#ако !BASH_PATTERN_SUBST && ENABLE_HUSH_CASE
	const цяло do_unbackslash = 0;
#endif
	char *exp_str;
	struct in_str input;
	o_string dest = NULL_O_STRING;

	ако (!first_special_char_in_vararg(str)) {
		/* string has no special chars */
		върни NULL;
	}

	setup_string_in_str(&input, str);
	dest.data = xzalloc(1); /* start as "", not as NULL */
	exp_str = NULL;

	за (;;) {
		цяло ch;

		ch = i_getch(&input);
		debug_printf_parse("%s: ch=%c (%d) escape=%d\n",
				__func__, ch, ch, !!dest.o_expflags);

		ако (!dest.o_expflags) {
			ако (ch == EOF)
				break;
			ако (handle_squotes && ch == '\'') {
				ако (!add_till_single_quote_dquoted(&dest, &input))
					goto ret; /* error */
				continue;
			}
		}
		ако (ch == EOF) {
			syntax_error_unterm_ch('"');
			goto ret; /* error */
		}
		ако (ch == '"') {
			dest.o_expflags ^= EXP_FLAG_ESC_GLOB_CHARS;
			continue;
		}
		ако (ch == '\\') {
			ch = i_getch(&input);
			ако (ch == EOF) {
//example? error message?	syntax_error_unterm_ch('"');
				debug_printf_parse("%s: error: \\<eof>\n", __func__);
				goto ret;
			}
			o_addqchr(&dest, ch);
			continue;
		}
		ако (ch == '$') {
			ако (parse_dollar_squote(NULL, &dest, &input))
				continue;
			ако (!parse_dollar(NULL, &dest, &input, /*quote_mask:*/ 0x80)) {
				debug_printf_parse("%s: error: parse_dollar returned 0 (error)\n", __func__);
				goto ret;
			}
			continue;
		}
#ако ENABLE_HUSH_TICK
		ако (ch == '`') {
			//unsigned pos = dest->length;
			o_addchr(&dest, SPECIAL_VAR_SYMBOL);
			o_addchr(&dest, 0x80 | '`');
			ако (!add_till_backquote(&dest, &input,
					/*in_dquote:*/ dest.o_expflags /* nonzero if EXP_FLAG_ESC_GLOB_CHARS set */
				)
			) {
				goto ret; /* error */
			}
			o_addchr(&dest, SPECIAL_VAR_SYMBOL);
			//debug_printf_subst("SUBST RES3 '%s'\n", dest->data + pos);
			continue;
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
	върни exp_str;
}

/* Expanding ARG in ${var+ARG}, ${var-ARG}
 */
static NOINLINE цяло encode_then_append_var_plusminus(o_string *output, цяло n,
		char *str, цяло dquoted)
{
	struct in_str input;
	o_string dest = NULL_O_STRING;

	ако (!first_special_char_in_vararg(str)
	 && '\0' == str[strcspn(str, G.ifs)]
	) {
		/* string has no special chars
		 * && string has no $IFS chars
		 */
		ако (dquoted) {
			/* Prints 1 (quoted expansion is a "" word, not nothing):
			 * set -- "${notexist-}"; echo $#
			 */
			output->has_quoted_part = 1;
		}
		върни expand_vars_to_list(output, n, str);
	}

	setup_string_in_str(&input, str);

	за (;;) {
		цяло ch;

		ch = i_getch(&input);
		debug_printf_parse("%s: ch=%c (%d) escape=%x\n",
				__func__, ch, ch, dest.o_expflags);

		ако (!dest.o_expflags) {
			ако (ch == EOF)
				break;
			ако (!dquoted && !(output->o_expflags & EXP_FLAG_SINGLEWORD) && strchr(G.ifs, ch)) {
				/* PREFIX${x:d${e}f ...} and we met space: expand "d${e}f" and start new word.
				 * do not assume we are at the start of the word (PREFIX above).
				 */
				ако (dest.data) {
					n = expand_vars_to_list(output, n, dest.data);
					o_free_and_set_NULL(&dest);
					o_addchr(output, '\0');
					n = o_save_ptr(output, n); /* create next word */
				} иначе
				ако (output->length != o_get_last_ptr(output, n)
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
				continue;
			}
			ако (!dquoted && ch == '\'') {
				ако (!add_till_single_quote_dquoted(&dest, &input))
					goto ret; /* error */
				o_addchr(&dest, SPECIAL_VAR_SYMBOL);
				o_addchr(&dest, SPECIAL_VAR_SYMBOL);
				continue;
			}
		}
		ако (ch == EOF) {
			syntax_error_unterm_ch('"');
			goto ret; /* error */
		}
		ако (ch == '"') {
			dest.o_expflags ^= EXP_FLAG_ESC_GLOB_CHARS;
			ако (dest.o_expflags) {
				o_addchr(&dest, SPECIAL_VAR_SYMBOL);
				o_addchr(&dest, SPECIAL_VAR_SYMBOL);
			}
			continue;
		}
		ако (ch == '\\') {
			ch = i_getch(&input);
			ако (ch == EOF) {
//example? error message?	syntax_error_unterm_ch('"');
				debug_printf_parse("%s: error: \\<eof>\n", __func__);
				goto ret;
			}
			o_addqchr(&dest, ch);
			continue;
		}
		ако (ch == '$') {
			ако (!parse_dollar(NULL, &dest, &input, /*quote_mask:*/ (dest.o_expflags || dquoted) ? 0x80 : 0)) {
				debug_printf_parse("%s: error: parse_dollar returned 0 (error)\n", __func__);
				goto ret;
			}
			continue;
		}
#ако ENABLE_HUSH_TICK
		ако (ch == '`') {
			//unsigned pos = dest->length;
			o_addchr(&dest, SPECIAL_VAR_SYMBOL);
			o_addchr(&dest, (dest.o_expflags || dquoted) ? 0x80 | '`' : '`');
			ако (!add_till_backquote(&dest, &input,
					/*in_dquote:*/ dest.o_expflags /* nonzero if EXP_FLAG_ESC_GLOB_CHARS set */
				)
			) {
				goto ret; /* error */
			}
			o_addchr(&dest, SPECIAL_VAR_SYMBOL);
			//debug_printf_subst("SUBST RES3 '%s'\n", dest->data + pos);
			continue;
		}
#endif
		ако (dquoted) {
			/* Always glob-protect if in dquotes:
			 * x=x; echo "${x:+/bin/c*}" - prints: /bin/c*
			 * x=x; echo "${x:+"/bin/c*"}" - prints: /bin/c*
			 */
			o_addqchr(&dest, ch);
		} иначе {
			/* Glob-protect only if char is quoted:
			 * x=x; echo ${x:+/bin/c*} - prints many filenames
			 * x=x; echo ${x:+"/bin/c*"} - prints: /bin/c*
			 */
			o_addQchr(&dest, ch);
		}
	} /* for (;;) */

	ако (dest.data) {
		n = expand_vars_to_list(output, n, dest.data);
	}
 ret:
	o_free(&dest);
	върни n;
}
#endif /* !__U_BOOT__ */

#ifndef __U_BOOT__
#ако ENABLE_FEATURE_SH_MATH
static arith_t expand_and_evaluate_arith(const char *arg, const char **errmsg_p)
{
	arith_state_t math_state;
	arith_t res;
	char *exp_str;

	math_state.lookupvar = get_local_var_value;
	math_state.setvar = set_local_var_from_halves;
	//math_state.endofname = endofname;
	exp_str = encode_then_expand_string(arg);
	res = arith(&math_state, exp_str ? exp_str : arg);
	free(exp_str);
	ако (errmsg_p)
		*errmsg_p = math_state.errmsg;
	ако (math_state.errmsg)
		msg_and_die_if_script(math_state.errmsg);
	върни res;
}
#endif
#endif /* !__U_BOOT__ */

#ifndef __U_BOOT__
#ако BASH_PATTERN_SUBST
/* ${var/[/]pattern[/repl]} helpers */
static char *strstr_pattern(char *val, const char *pattern, цяло *size)
{
	цяло first_escaped = (pattern[0] == '\\' && pattern[1]);
	/* "first_escaped" trick allows to treat e.g. "\*no_glob_chars"
	 * as literal too (as it is semi-common, and easy to accomodate
	 * by just using str + 1).
	 */
	цяло sz = strcspn(pattern + first_escaped * 2, "*?[\\");
	ако ((pattern + first_escaped * 2)[sz] == '\0') {
		/* Optimization for trivial patterns.
		 * Testcase for very slow replace (performs about 22k replaces):
		 * x=::::::::::::::::::::::
		 * x=$x$x;x=$x$x;x=$x$x;x=$x$x;x=$x$x;x=$x$x;x=$x$x;x=$x$x;x=$x$x;x=$x$x;echo ${#x}
		 * echo "${x//:/|}"
		 */
		*size = sz + first_escaped;
		върни strstr(val, pattern + first_escaped);
	}

	докато (1) {
		char *end = scan_and_match(val, pattern, SCAN_MOVE_FROM_RIGHT + SCAN_MATCH_LEFT_HALF);
		debug_printf_varexp("val:'%s' pattern:'%s' end:'%s'\n", val, pattern, end);
		ако (end) {
			*size = end - val;
			върни val;
		}
		ако (*val == '\0')
			върни NULL;
		/* Optimization: if "*pat" did not match the start of "string",
		 * we know that "tring", "ring" etc will not match too:
		 */
		ако (pattern[0] == '*')
			върни NULL;
		val++;
	}
}
static char *replace_pattern(char *val, const char *pattern, const char *repl, char exp_op)
{
	char *result = NULL;
	unsigned res_len = 0;
	unsigned repl_len = strlen(repl);

	/* Null pattern never matches, including if "var" is empty */
	ако (!pattern[0])
		върни result; /* NULL, no replaces happened */

	докато (1) {
		цяло size;
		char *s = strstr_pattern(val, pattern, &size);
		ако (!s)
			break;

		result = xrealloc(result, res_len + (s - val) + repl_len + 1);
		strcpy(mempcpy(result + res_len, val, s - val), repl);
		res_len += (s - val) + repl_len;
		debug_printf_varexp("val:'%s' s:'%s' result:'%s'\n", val, s, result);

		val = s + size;
		ако (exp_op == '/')
			break;
	}
	ако (*val && result) {
		result = xrealloc(result, res_len + strlen(val) + 1);
		strcpy(result + res_len, val);
		debug_printf_varexp("val:'%s' result:'%s'\n", val, result);
	}
	debug_printf_varexp("result:'%s'\n", result);
	върни result;
}
#endif /* BASH_PATTERN_SUBST */
#endif /* !__U_BOOT__ */

static цяло append_str_maybe_ifs_split(o_string *output, цяло n,
		цяло first_ch, const char *val)
{
	ако (!(first_ch & 0x80)) { /* unquoted $VAR */
		debug_printf_expand("unquoted '%s', output->o_escape:%d\n", val,
				!!(output->o_expflags & EXP_FLAG_ESC_GLOB_CHARS));
		ако (val && val[0])
			n = expand_on_ifs(output, n, val);
	} иначе { /* quoted "$VAR" */
		output->has_quoted_part = 1;
		debug_printf_expand("quoted '%s', output->o_escape:%d\n", val,
				!!(output->o_expflags & EXP_FLAG_ESC_GLOB_CHARS));
		ако (val && val[0])
			o_addQstr(output, val);
	}
	върни n;
}

/* Handle <SPECIAL_VAR_SYMBOL>varname...<SPECIAL_VAR_SYMBOL> construct.
 */
static NOINLINE цяло expand_one_var(o_string *output, цяло n,
		цяло first_ch, char *arg, char **pp)
{
	const char *val;
	char *to_be_freed;
	char *p;
	char *var;
	char exp_op;
	char exp_save = exp_save; /* for compiler */
	char *exp_saveptr; /* points to expansion operator */
	char *exp_word = exp_word; /* for compiler */
	char arg0;

	val = NULL;
	to_be_freed = NULL;
	p = *pp;
	*p = '\0'; /* replace trailing SPECIAL_VAR_SYMBOL */
	var = arg;
	exp_saveptr = arg[1] ? strchr(VAR_ENCODED_SUBST_OPS, arg[1]) : NULL;
	arg0 = arg[0];
	arg[0] = (arg0 & 0x7f);
	exp_op = 0;

	ако (arg[0] == '#' && arg[1] /* ${#...} but not ${#} */
	 && (!exp_saveptr               /* and ( not(${#<op_char>...}) */
	    || (arg[2] == '\0' && strchr(SPECIAL_VARS_STR, arg[1])) /* or ${#C} "len of $C" ) */
	    )		/* NB: skipping ^^^specvar check mishandles ${#::2} */
	) {
		/* It must be length operator: ${#var} */
		var++;
		exp_op = 'L';
	} иначе {
		/* Maybe handle parameter expansion */
		ако (exp_saveptr /* if 2nd char is one of expansion operators */
		 && strchr(NUMERIC_SPECVARS_STR, arg[0]) /* 1st char is special variable */
		) {
			/* ${?:0}, ${#[:]%0} etc */
			exp_saveptr = var + 1;
		} иначе {
			/* ${?}, ${var}, ${var:0}, ${var[:]%0} etc */
			exp_saveptr = var+1 + strcspn(var+1, VAR_ENCODED_SUBST_OPS);
		}
		exp_op = exp_save = *exp_saveptr;
#ifndef __U_BOOT__
		ако (exp_op) {
			exp_word = exp_saveptr + 1;
			ако (exp_op == ':') {
				exp_op = *exp_word++;
//TODO: try ${var:} and ${var:bogus} in non-bash config
				ако (BASH_SUBSTR
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
	ако (isdigit(var[0])) {
		/* parse_dollar should have vetted var for us */
#ifndef __U_BOOT__
		цяло nn = xatoi_positive(var);
#иначе /* __U_BOOT__ */
		цяло nn = simple_strtoul(var, NULL, 10);
#endif /* __U_BOOT__ */
		ако (nn < G.global_argc)
			val = G.global_argv[nn];
		/* else val remains NULL: $N with too big N */
	} иначе {
		switch (var[0]) {
#ifndef __U_BOOT__
		case '$': /* pid */
			val = utoa(G.root_pid);
			break;
		case '!': /* bg pid */
			val = G.last_bg_pid ? utoa(G.last_bg_pid) : "";
			break;
#endif /* !__U_BOOT__ */
		case '?': /* exitcode */
			val = utoa(G.last_exitcode);
			break;
		case '#': /* argc */
			val = utoa(G.global_argc ? G.global_argc-1 : 0);
			break;
#ifndef __U_BOOT__
		case '-': { /* active options */
			/* Check set_mode() to see what option chars we support */
			char *cp;
			val = cp = G.optstring_buf;
			ако (G.o_opt[OPT_O_ERREXIT])
				*cp++ = 'e';
			ако (G_interactive_fd)
				*cp++ = 'i';
			ако (G_x_mode)
				*cp++ = 'x';
			/* If G.o_opt[OPT_O_NOEXEC] is true,
			 * commands read but are not executed,
			 * so $- can not execute too, 'n' is never seen in $-.
			 */
			ако (G.opt_c)
				*cp++ = 'c';
			ако (G.opt_s)
				*cp++ = 's';
			*cp = '\0';
			break;
		}
#endif /* !__U_BOOT__ */
		default:
#ifndef __U_BOOT__
			val = get_local_var_value(var);
#иначе /* __U_BOOT__ */
			/*
			 * Environment variable set with setenv* have to be
			 * expanded.
			 * So, we first search if the variable exists in
			 * environment, if this is not the case, we default to
			 * local value.
			 */
			val = env_get(var);
			ако (!val)
				val = get_local_var_value(var);
#endif /* __U_BOOT__ */
		}
	}

#ifndef __U_BOOT__
	/* Handle any expansions */
	ако (exp_op == 'L') {
		reinit_unicode_for_hush();
		debug_printf_expand("expand: length(%s)=", val);
		val = utoa(val ? unicode_strlen(val) : 0);
		debug_printf_expand("%s\n", val);
	} иначе ако (exp_op) {
		ако (exp_op == '%' || exp_op == '#') {
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
			ако (val && val[0]) {
				char *t;
				char *exp_exp_word;
				char *loc;
				unsigned scan_flags = pick_scan(exp_op, *exp_word);
				ако (exp_op == *exp_word)  /* ## or %% */
					exp_word++;
				debug_printf_expand("expand: exp_word:'%s'\n", exp_word);
				exp_exp_word = encode_then_expand_vararg(exp_word, /*handle_squotes:*/ 1, /*unbackslash:*/ 0);
				ако (exp_exp_word)
					exp_word = exp_exp_word;
				debug_printf_expand("expand: exp_word:'%s'\n", exp_word);
				/*
				 * HACK ALERT. We depend here on the fact that
				 * G.global_argv and results of utoa and get_local_var_value
				 * are actually in writable memory:
				 * scan_and_match momentarily stores NULs there.
				 */
				t = (char*)val;
				loc = scan_and_match(t, exp_word, scan_flags);
				debug_printf_expand("op:%c str:'%s' pat:'%s' res:'%s'\n", exp_op, t, exp_word, loc);
				free(exp_exp_word);
				ако (loc) { /* match was found */
					ако (scan_flags & SCAN_MATCH_LEFT_HALF) /* #[#] */
						val = loc; /* take right part */
					иначе /* %[%] */
						val = to_be_freed = xstrndup(val, loc - val); /* left */
				}
			}
		}
#ако BASH_PATTERN_SUBST
		иначе ако (exp_op == '/' || exp_op == '\\') {
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
			ако (val /*&& val[0]*/) {
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
				char *pattern, *repl, *t;
				pattern = encode_then_expand_vararg(exp_word, /*handle_squotes:*/ 1, /*unbackslash:*/ 0);
				ако (!pattern)
					pattern = xstrdup(exp_word);
				debug_printf_varexp("pattern:'%s'->'%s'\n", exp_word, pattern);
				*p++ = SPECIAL_VAR_SYMBOL;
				exp_word = p;
				p = strchr(p, SPECIAL_VAR_SYMBOL);
				*p = '\0';
				repl = encode_then_expand_vararg(exp_word, /*handle_squotes:*/ 1, /*unbackslash:*/ 1);
				debug_printf_varexp("repl:'%s'->'%s'\n", exp_word, repl);
				/* HACK ALERT. We depend here on the fact that
				 * G.global_argv and results of utoa and get_local_var_value
				 * are actually in writable memory:
				 * replace_pattern momentarily stores NULs there. */
				t = (char*)val;
				to_be_freed = replace_pattern(t,
						pattern,
						(repl ? repl : exp_word),
						exp_op);
				ако (to_be_freed) /* at least one replace happened */
					val = to_be_freed;
				free(pattern);
				free(repl);
			} иначе {
				/* Unset variable always gives nothing */
				//  a=; echo ${a/*/w}      # "w"
				//  unset a; echo ${a/*/w} # ""
				/* Just skip "replace" part */
				*p++ = SPECIAL_VAR_SYMBOL;
				p = strchr(p, SPECIAL_VAR_SYMBOL);
				*p = '\0';
			}
		}
#endif /* BASH_PATTERN_SUBST */
		иначе ако (exp_op == ':') {
#ако BASH_SUBSTR && ENABLE_FEATURE_SH_MATH
			/* It's ${var:N[:M]} bashism.
			 * Note that in encoded form it has TWO parts:
			 * var:N<SPECIAL_VAR_SYMBOL>M<SPECIAL_VAR_SYMBOL>
			 */
			arith_t beg, len;
			unsigned vallen;
			const char *errmsg;

			beg = expand_and_evaluate_arith(exp_word, &errmsg);
			ако (errmsg)
				goto empty_result;
			debug_printf_varexp("beg:'%s'=%lld\n", exp_word, (long long)beg);
			*p++ = SPECIAL_VAR_SYMBOL;
			exp_word = p;
			p = strchr(p, SPECIAL_VAR_SYMBOL);
			*p = '\0';
			vallen = val ? strlen(val) : 0;
			ако (beg < 0) {
				/* negative beg counts from the end */
				beg = (arith_t)vallen + beg;
			}
			/* If expansion will be empty, do not even evaluate len */
			ако (!val || beg < 0 || beg > vallen) {
				/* Why > vallen, not >=? bash:
				 * unset b; a=ab; : ${a:2:${b=3}}; echo $b  # "", b=3 (!!!)
				 * unset b; a=a; : ${a:2:${b=3}}; echo $b   # "", b not set
				 */
				goto empty_result;
			}
			len = expand_and_evaluate_arith(exp_word, &errmsg);
			ако (errmsg)
				goto empty_result;
			debug_printf_varexp("len:'%s'=%lld\n", exp_word, (long long)len);
			debug_printf_varexp("from val:'%s'\n", val);
			ако (len < 0) {
				/* in bash, len=-n means strlen()-n */
				len = (arith_t)vallen - beg + len;
				ако (len < 0) /* bash compat */
					msg_and_die_if_script("%s: substring expression < 0", var);
			}
			ако (len <= 0 || !val /*|| beg >= vallen*/) {
 empty_result:
				val = NULL;
			} иначе {
				/* Paranoia. What if user entered 9999999999999
				 * which fits in arith_t but not int? */
				ако (len > INT_MAX)
					len = INT_MAX;
				val = to_be_freed = xstrndup(val + beg, len);
			}
			debug_printf_varexp("val:'%s'\n", val);
#иначе /* not (HUSH_SUBSTR_EXPANSION && FEATURE_SH_MATH) */
			msg_and_die_if_script("malformed ${%s:...}", var);
			val = NULL;
#endif
		} иначе { /* one of "-=+?" */
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
			цяло use_word = (!val || ((exp_save == ':') && !val[0]));
			ако (exp_op == '+')
				use_word = !use_word;
			debug_printf_expand("expand: op:%c (null:%s) test:%i\n", exp_op,
					(exp_save == ':') ? "true" : "false", use_word);
			ако (use_word) {
				ако (exp_op == '+' || exp_op == '-') {
					/* ${var+word} - use alternative value */
					/* ${var-word} - use default value */
					n = encode_then_append_var_plusminus(output, n, exp_word,
							/*dquoted:*/ (arg0 & 0x80)
					);
					val = NULL;
				} иначе {
					/* ${var?word} - indicate error if unset */
					/* ${var=word} - assign and use default value */
					to_be_freed = encode_then_expand_vararg(exp_word,
							/*handle_squotes:*/ !(arg0 & 0x80),
							/*unbackslash:*/ 0
					);
					ако (to_be_freed)
						exp_word = to_be_freed;
					ако (exp_op == '?') {
						/* mimic bash message */
						msg_and_die_if_script("%s: %s",
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
					} иначе {
						val = exp_word;
					}
					ако (exp_op == '=') {
						/* ${var=[word]} or ${var:=[word]} */
						ако (isdigit(var[0]) || var[0] == '#') {
							/* mimic bash message */
							msg_and_die_if_script("$%s: cannot assign in this way", var);
							val = NULL;
						} иначе {
							char *new_var = xasprintf("%s=%s", var, val);
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

	free(to_be_freed);
	върни n;
}

/* Expand all variable references in given string, adding words to list[]
 * at n, n+1,... positions. Return updated n (so that list[n] is next one
 * to be filled). This routine is extremely tricky: has to deal with
 * variables/parameters with whitespace, $* and $@, and constructs like
 * 'echo -$*-'. If you play here, you must run testsuite afterwards! */
static NOINLINE цяло expand_vars_to_list(o_string *output, цяло n, char *arg)
{
	/* output->o_expflags & EXP_FLAG_SINGLEWORD (0x80) if we are in
	 * expansion of right-hand side of assignment == 1-element expand.
	 */
	char cant_be_null = 0; /* only bit 0x80 matters */
	char *p;

	debug_printf_expand("expand_vars_to_list: arg:'%s' singleword:%x\n", arg,
			!!(output->o_expflags & EXP_FLAG_SINGLEWORD));
	debug_print_list("expand_vars_to_list[0]", output, n);

	докато ((p = strchr(arg, SPECIAL_VAR_SYMBOL)) != NULL) {
		char first_ch;
#ако ENABLE_FEATURE_SH_MATH
		char arith_buf[sizeof(arith_t)*3 + 2];
#endif

		ако (output->ended_in_ifs) {
			o_addchr(output, '\0');
			n = o_save_ptr(output, n);
			output->ended_in_ifs = 0;
		}

		o_addblock(output, arg, p - arg);
		debug_print_list("expand_vars_to_list[1]", output, n);
		arg = ++p;
		p = strchr(p, SPECIAL_VAR_SYMBOL);

		/* Fetch special var name (if it is indeed one of them)
		 * and quote bit, force the bit on if singleword expansion -
		 * important for not getting v=$@ expand to many words. */
		first_ch = arg[0] | (output->o_expflags & EXP_FLAG_SINGLEWORD);

		/* Is this variable quoted and thus expansion can't be null?
		 * "$@" is special. Even if quoted, it can still
		 * expand to nothing (not even an empty string),
		 * thus it is excluded. */
		ако ((first_ch & 0x7f) != '@')
			cant_be_null |= first_ch;

		switch (first_ch & 0x7f) {
		/* Highest bit in first_ch indicates that var is double-quoted */
		case '*':
		case '@': {
			цяло i;
#ifndef __U_BOOT__
			ако (!G.global_argv[1])
#иначе /* __U_BOOT__ */
			ако (!G.global_argv || !G.global_argv[1])
#endif /* __U_BOOT__ */
				break;
			i = 1;
			cant_be_null |= first_ch; /* do it for "$@" _now_, when we know it's not empty */
			ако (!(first_ch & 0x80)) { /* unquoted $* or $@ */
				докато (G.global_argv[i]) {
					n = expand_on_ifs(output, n, G.global_argv[i]);
					debug_printf_expand("expand_vars_to_list: argv %d (last %d)\n", i, G.global_argc - 1);
					ако (G.global_argv[i++][0] && G.global_argv[i]) {
						/* this argv[] is not empty and not last:
						 * put terminating NUL, start new word */
						o_addchr(output, '\0');
						debug_print_list("expand_vars_to_list[2]", output, n);
						n = o_save_ptr(output, n);
						debug_print_list("expand_vars_to_list[3]", output, n);
					}
				}
			} иначе
			/* If EXP_FLAG_SINGLEWORD, we handle assignment 'a=....$@.....'
			 * and in this case should treat it like '$*' - see 'else...' below */
			ако (first_ch == (char)('@'|0x80)  /* quoted $@ */
			 && !(output->o_expflags & EXP_FLAG_SINGLEWORD) /* not v="$@" case */
			) {
				докато (1) {
					o_addQstr(output, G.global_argv[i]);
					ако (++i >= G.global_argc)
						break;
					o_addchr(output, '\0');
					debug_print_list("expand_vars_to_list[4]", output, n);
					n = o_save_ptr(output, n);
				}
			} иначе { /* quoted $* (or v="$@" case): add as one word */
				докато (1) {
					o_addQstr(output, G.global_argv[i]);
					ако (!G.global_argv[++i])
						break;
					ако (G.ifs[0])
						o_addchr(output, G.ifs[0]);
				}
				output->has_quoted_part = 1;
			}
			break;
		}
		case SPECIAL_VAR_SYMBOL: {
			/* <SPECIAL_VAR_SYMBOL><SPECIAL_VAR_SYMBOL> */
			/* "Empty variable", used to make "" etc to not disappear */
			output->has_quoted_part = 1;
			cant_be_null = 0x80;
			arg++;
			break;
		}
		case SPECIAL_VAR_QUOTED_SVS:
			/* <SPECIAL_VAR_SYMBOL><SPECIAL_VAR_QUOTED_SVS><SPECIAL_VAR_SYMBOL> */
			/* "^C variable", represents literal ^C char (possible in scripts) */
			o_addchr(output, SPECIAL_VAR_SYMBOL);
			arg++;
			break;
#ако ENABLE_HUSH_TICK
		case '`': {
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
			break;
		}
#endif
#ако ENABLE_FEATURE_SH_MATH
		case '+': {
			/* <SPECIAL_VAR_SYMBOL>+arith<SPECIAL_VAR_SYMBOL> */
			arith_t res;

			arg++; /* skip '+' */
			*p = '\0'; /* replace trailing <SPECIAL_VAR_SYMBOL> */
			debug_printf_subst("ARITH '%s' first_ch %x\n", arg, first_ch);
			res = expand_and_evaluate_arith(arg, NULL);
			debug_printf_subst("ARITH RES '"ARITH_FMT"'\n", res);
			sprintf(arith_buf, ARITH_FMT, res);
			ако (res < 0
			 && first_ch == (char)('+'|0x80)
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
			break;
		}
#endif
		default:
			/* <SPECIAL_VAR_SYMBOL>varname[ops]<SPECIAL_VAR_SYMBOL> */
			n = expand_one_var(output, n, first_ch, arg, &p);
			break;
		} /* switch (char after <SPECIAL_VAR_SYMBOL>) */

		/* Restore NULL'ed SPECIAL_VAR_SYMBOL.
		 * Do the check to avoid writing to a const string. */
		ако (*p != SPECIAL_VAR_SYMBOL)
			*p = SPECIAL_VAR_SYMBOL;
		arg = ++p;
	} /* end of "while (SPECIAL_VAR_SYMBOL is found) ..." */

	ако (*arg) {
		/* handle trailing string */
		ако (output->ended_in_ifs) {
			o_addchr(output, '\0');
			n = o_save_ptr(output, n);
		}
		debug_print_list("expand_vars_to_list[a]", output, n);
		/* this part is literal, and it was already pre-quoted
		 * if needed (much earlier), do not use o_addQstr here!
		 */
		o_addstr(output, arg);
		debug_print_list("expand_vars_to_list[b]", output, n);
	} иначе
	ако (output->length == o_get_last_ptr(output, n) /* expansion is empty */
	 && !(cant_be_null & 0x80)   /* and all vars were not quoted */
	 && !output->has_quoted_part
	) {
		n--;
		/* allow to reuse list[n] later without re-growth */
		output->has_empty_slot = 1;
	}

	върни n;
}

static char **expand_variables(char **argv, unsigned expflags)
{
	цяло n;
	char **list;
	o_string output = NULL_O_STRING;

	output.o_expflags = expflags;

	n = 0;
	за (;;) {
		/* go to next list[n] */
		output.ended_in_ifs = 0;
		n = o_save_ptr(&output, n);

		ако (!*argv)
			break;

		/* expand argv[i] */
		n = expand_vars_to_list(&output, n, *argv++);
		/* if (!output->has_empty_slot) -- need this?? */
			o_addchr(&output, '\0');
	}
	debug_print_list("expand_variables", &output, n);

	/* output.data (malloced in one block) gets returned in "list" */
	list = o_finalize_list(&output, n);
	debug_print_strings("expand_variables[1]", list);
	върни list;
}

static char **expand_strvec_to_strvec(char **argv)
{
	върни expand_variables(argv, EXP_FLAG_GLOB | EXP_FLAG_ESC_GLOB_CHARS);
}

#ако defined(CMD_SINGLEWORD_NOGLOB) || defined(CMD_TEST2_SINGLEWORD_NOGLOB)
static char **expand_strvec_to_strvec_singleword_noglob(char **argv)
{
	върни expand_variables(argv, EXP_FLAG_SINGLEWORD);
}
#endif

/* Used for expansion of right hand of assignments,
 * $((...)), heredocs, variable expansion parts.
 *
 * NB: should NOT do globbing!
 * "export v=/bin/c*; env | grep ^v=" outputs "v=/bin/c*"
 */
static char *expand_string_to_string(const char *str, цяло EXP_flags, цяло do_unbackslash)
{
#ако !BASH_PATTERN_SUBST && !ENABLE_HUSH_CASE
	const цяло do_unbackslash = 1;
	const цяло EXP_flags = EXP_FLAG_ESC_GLOB_CHARS;
#endif
	char *argv[2], **list;

	debug_printf_expand("string_to_string<='%s'\n", str);
	/* This is generally an optimization, but it also
	 * handles "", which otherwise trips over !list[0] check below.
	 * (is this ever happens that we actually get str="" here?)
	 */
	ако (!strchr(str, SPECIAL_VAR_SYMBOL) && !strchr(str, '\\')) {
		//TODO: Can use on strings with \ too, just unbackslash() them?
		debug_printf_expand("string_to_string(fast)=>'%s'\n", str);
		върни xstrdup(str);
	}

	argv[0] = (char*)str;
	argv[1] = NULL;
	list = expand_variables(argv, EXP_flags | EXP_FLAG_SINGLEWORD);
	ако (!list[0]) {
		/* Example where it happens:
		 * x=; echo ${x:-"$@"}
		 */
		((char*)list)[0] = '\0';
	} иначе {
		ако (HUSH_DEBUG)
			ако (list[1])
				bb_simple_error_msg_and_die("BUG in varexp2");
		/* actually, just move string 2*sizeof(char*) bytes back */
		overlapping_strcpy((char*)list, list[0]);
		ако (do_unbackslash)
			unbackslash((char*)list);
	}
	debug_printf_expand("string_to_string=>'%s'\n", (char*)list);
	върни (char*)list;
}

#ако 0
static char* expand_strvec_to_string(char **argv)
{
	char **list;

	list = expand_variables(argv, EXP_FLAG_SINGLEWORD);
	/* Convert all NULs to spaces */
	ако (list[0]) {
		цяло n = 1;
		докато (list[n]) {
			ако (HUSH_DEBUG)
				ако (list[n-1] + strlen(list[n-1]) + 1 != list[n])
					bb_error_msg_and_die("BUG in varexp3");
			/* bash uses ' ' regardless of $IFS contents */
			list[n][-1] = ' ';
			n++;
		}
	}
	overlapping_strcpy((char*)list, list[0] ? list[0] : "");
	debug_printf_expand("strvec_to_string='%s'\n", (char*)list);
	върни (char*)list;
}
#endif

#ifndef __U_BOOT__
static char **expand_assignments(char **argv, цяло count)
{
	цяло i;
	char **p;

	G.expanded_assignments = p = NULL;
	/* Expand assignments into one string each */
	за (i = 0; i < count; i++) {
		p = add_string_to_strings(p,
			expand_string_to_string(argv[i],
				EXP_FLAG_ESC_GLOB_CHARS,
				/*unbackslash:*/ 1
			)
		);
		G.expanded_assignments = p;
	}
	G.expanded_assignments = NULL;
	върни p;
}

static void switch_off_special_sigs(unsigned mask)
{
	unsigned sig = 0;
	докато ((mask >>= 1) != 0) {
		sig++;
		ако (!(mask & 1))
			continue;
#ако ENABLE_HUSH_TRAP
		ако (G_traps) {
			ако (G_traps[sig] && !G_traps[sig][0])
				/* trap is '', has to remain SIG_IGN */
				continue;
			free(G_traps[sig]);
			G_traps[sig] = NULL;
		}
#endif
		/* We are here only if no trap or trap was not '' */
		install_sighandler(sig, SIG_DFL);
	}
}
#endif /* !__U_BOOT__ */

#ifndef __U_BOOT__
#ако BB_MMU
/* never called */
void re_execute_shell(char ***to_free, const char *s,
		char *g_argv0, char **g_argv,
		char **builtin_argv) NORETURN;

static void reset_traps_to_defaults(void)
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
	ако (!G_traps && !mask)
		върни; /* already no traps and no special sigs */

	/* Switch off special sigs */
	switch_off_special_sigs(mask);
# ако ENABLE_HUSH_JOB
	G_fatal_sig_mask = 0;
# endif
	G.special_sig_mask &= ~SPECIAL_INTERACTIVE_SIGS;
	/* SIGQUIT,SIGCHLD and maybe SPECIAL_JOBSTOP_SIGS
	 * remain set in G.special_sig_mask */

# ако ENABLE_HUSH_TRAP
	ако (!G_traps)
		върни;

	/* Reset all sigs to default except ones with empty traps */
	за (sig = 0; sig < NSIG; sig++) {
		ако (!G_traps[sig])
			continue; /* no trap: nothing to do */
		ако (!G_traps[sig][0])
			continue; /* empty trap: has to remain SIG_IGN */
		/* sig has non-empty trap, reset it: */
		free(G_traps[sig]);
		G_traps[sig] = NULL;
		/* There is no signal for trap 0 (EXIT) */
		ако (sig == 0)
			continue;
		install_sighandler(sig, pick_sighandler(sig));
	}
# endif
}

#иначе /* !BB_MMU */

static void re_execute_shell(char ***to_free, const char *s,
		char *g_argv0, char **g_argv,
		char **builtin_argv) NORETURN;
static void re_execute_shell(char ***to_free, const char *s,
		char *g_argv0, char **g_argv,
		char **builtin_argv)
{
# define NOMMU_HACK_FMT ("-$%x:%x:%x:%x:%x:%llx" IF_HUSH_LOOPS(":%x"))
	/* delims + 2 * (number of bytes in printed hex numbers) */
	char param_buf[sizeof(NOMMU_HACK_FMT) + 2 * (sizeof(цяло)*6 + sizeof(long long)*1)];
	char *heredoc_argv[4];
	struct variable *cur;
# ако ENABLE_HUSH_FUNCTIONS
	struct function *funcp;
# endif
	char **argv, **pp;
	unsigned cnt;
	unsigned long long empty_trap_mask;

	ако (!g_argv0) { /* heredoc */
		argv = heredoc_argv;
		argv[0] = (char *) G.argv0_for_re_execing;
		argv[1] = (char *) "-<";
		argv[2] = (char *) s;
		argv[3] = NULL;
		pp = &argv[3]; /* used as pointer to empty environment */
		goto do_exec;
	}

	cnt = 0;
	pp = builtin_argv;
	ако (pp) докато (*pp++)
		cnt++;

	empty_trap_mask = 0;
	ако (G_traps) {
		цяло sig;
		за (sig = 1; sig < NSIG; sig++) {
			ако (G_traps[sig] && !G_traps[sig][0])
				empty_trap_mask |= 1LL << sig;
		}
	}

	sprintf(param_buf, NOMMU_HACK_FMT
			, (unsigned) G.root_pid
			, (unsigned) G.root_ppid
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
	за (cur = G.top_var; cur; cur = cur->next) {
		ако (!cur->flg_export || cur->flg_read_only)
			cnt += 2;
	}
# ако ENABLE_HUSH_LINENO_VAR
	cnt += 2;
# endif
# ако ENABLE_HUSH_FUNCTIONS
	за (funcp = G.top_func; funcp; funcp = funcp->next)
		cnt += 3;
# endif
	pp = g_argv;
	докато (*pp++)
		cnt++;
	*to_free = argv = pp = xzalloc(sizeof(argv[0]) * cnt);
	*pp++ = (char *) G.argv0_for_re_execing;
	*pp++ = param_buf;
	за (cur = G.top_var; cur; cur = cur->next) {
		ако (strcmp(cur->varstr, hush_version_str) == 0)
			continue;
		ако (cur->flg_read_only) {
			*pp++ = (char *) "-R";
			*pp++ = cur->varstr;
		} иначе ако (!cur->flg_export) {
			*pp++ = (char *) "-V";
			*pp++ = cur->varstr;
		}
	}
# ако ENABLE_HUSH_LINENO_VAR
	*pp++ = (char *) "-L";
	*pp++ = utoa(G.execute_lineno);
# endif
# ако ENABLE_HUSH_FUNCTIONS
	за (funcp = G.top_func; funcp; funcp = funcp->next) {
		*pp++ = (char *) "-F";
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
	*pp++ = (char *) "-c";
	*pp++ = (char *) s;
	ако (builtin_argv) {
		докато (*++builtin_argv)
			*pp++ = *builtin_argv;
		*pp++ = (char *) "";
	}
	*pp++ = g_argv0;
	докато (*g_argv)
		*pp++ = *g_argv++;
	/* *pp = NULL; - is already there */
	pp = environ;

 do_exec:
	debug_printf_exec("re_execute_shell pid:%d cmd:'%s'\n", getpid(), s);
	/* Don't propagate SIG_IGN to the child */
	ако (SPECIAL_JOBSTOP_SIGS != 0)
		switch_off_special_sigs(G.special_sig_mask & SPECIAL_JOBSTOP_SIGS);
	execve(bb_busybox_exec_path, argv, pp);
	/* Fallback. Useful for init=/bin/hush usage etc */
	ако (argv[0][0] == '/')
		execve(argv[0], argv, pp);
	xfunc_error_retval = 127;
	bb_simple_error_msg_and_die("can't re-execute the shell");
}
#endif  /* !BB_MMU */

#endif /* !__U_BOOT__ */

static цяло run_and_free_list(struct pipe *pi);

/* Executing from string: eval, sh -c '...'
 *          or from file: /etc/profile, . file, sh <script>, sh (intereactive)
 * end_trigger controls how often we stop parsing
 * NUL: parse all, execute, return
 * ';': parse till ';' or newline, execute, repeat till EOF
 */
#ifndef __U_BOOT__
static void parse_and_run_stream(struct in_str *inp, цяло end_trigger)
#иначе /* __U_BOOT__ */
static цяло parse_and_run_stream(struct in_str *inp, цяло end_trigger)
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
	докато (1) {
#иначе /* __U_BOOT__ */
	do {
#endif /* __U_BOOT__ */
		struct pipe *pipe_list;

#ако ENABLE_HUSH_INTERACTIVE
		ако (end_trigger == ';') {
			G.promptmode = 0; /* PS1 */
			debug_printf_prompt("%s promptmode=%d\n", __func__, G.promptmode);
		}
#endif
		pipe_list = parse_stream(NULL, NULL, inp, end_trigger);
		ако (!pipe_list || pipe_list == ERR_PTR) { /* EOF/error */
			/* If we are in "big" script
			 * (not in `cmd` or something similar)...
			 */
			ако (pipe_list == ERR_PTR && end_trigger == ';') {
				/* Discard cached input (rest of line) */
				цяло ch = inp->last_char;
				докато (ch != EOF && ch != '\n') {
					//bb_error_msg("Discarded:'%c'", ch);
					ch = i_getch(inp);
				}
				/* Force prompt */
				inp->p = NULL;
				/* This stream isn't empty */
				empty = 0;
				continue;
			}
			ако (!pipe_list && empty)
				G.last_exitcode = 0;
			break;
		}
		debug_print_tree(pipe_list, 0);
		debug_printf_exec("parse_and_run_stream: run_and_free_list\n");
#ifndef __U_BOOT__
		run_and_free_list(pipe_list);
#иначе /* __U_BOOT__ */
		цяло rcode = run_and_free_list(pipe_list);
		/*
		 * We reset input string to not run the following command, so running
		 * 'exit; echo foo' does not print foo.
		 */
		ако (rcode <= EXIT_RET_CODE)
			setup_file_in_str(inp);
#endif /* __U_BOOT__ */
		empty = 0;
		ако (G_flag_return_in_progress == 1)
			break;
#ifndef __U_BOOT__
	}
#иначе /* __U_BOOT__ */
	/*
	 * This do/while is needed by run_command to avoid looping on a command
	 * with syntax error.
	 */
	} докато (!(G.run_command_flags & FLAG_EXIT_FROM_LOOP));

	върни G.last_exitcode;
#endif /* __U_BOOT__ */
}

#ifndef __U_BOOT__
static void parse_and_run_string(const char *s)
#иначе /* __U_BOOT__ */
static цяло parse_and_run_string(const char *s)
#endif /* __U_BOOT__ */
{
	struct in_str input;
#ifndef __U_BOOT__
	IF_HUSH_LINENO_VAR(unsigned sv = G.parse_lineno;)
#иначе /* __U_BOOT__ */
	//IF_HUSH_LINENO_VAR(unsigned sv = G.parse_lineno;)
#endif /* __U_BOOT__ */

	setup_string_in_str(&input, s);
#ifndef __U_BOOT__
	parse_and_run_stream(&input, '\0');
#иначе /* __U_BOOT__ */
	върни parse_and_run_stream(&input, '\0');
#endif /* __U_BOOT__ */
#ifndef __U_BOOT__
	IF_HUSH_LINENO_VAR(unsigned sv = G.parse_lineno;)
#иначе /* __U_BOOT__ */
	//IF_HUSH_LINENO_VAR(unsigned sv = G.parse_lineno;)
#endif /* __U_BOOT__ */
}

#ifdef __U_BOOT__
цяло parse_string_outer_modern(const char *cmd, цяло flags)
{
	цяло ret;
	цяло old_flags;

	/*
	 * Keep old values of run_command to be able to restore them once
	 * command was executed.
	 */
	old_flags = G.run_command_flags;
	G.run_command_flags = flags;

	ret = parse_and_run_string(cmd);

	G.run_command_flags = old_flags;

	върни ret;
}
#endif /* __U_BOOT__ */
#ifndef __U_BOOT__
static void parse_and_run_file(HFILE *fp)
#иначе /* __U_BOOT__ */
void parse_and_run_file(void)
#endif /* __U_BOOT__ */
{
	struct in_str input;
#ifndef __U_BOOT__
	IF_HUSH_LINENO_VAR(unsigned sv = G.parse_lineno;)

	IF_HUSH_LINENO_VAR(G.parse_lineno = 1;)
	setup_file_in_str(&input, fp);
#иначе /* __U_BOOT__ */
	setup_file_in_str(&input);
#endif /* __U_BOOT__ */
	parse_and_run_stream(&input, ';');
#ifndef __U_BOOT__
	IF_HUSH_LINENO_VAR(G.parse_lineno = sv;)
#endif /* !__U_BOOT__ */
}

#ifndef __U_BOOT__
#ако ENABLE_HUSH_TICK
static цяло generate_stream_from_string(const char *s, pid_t *pid_p)
{
	pid_t pid;
	цяло channel[2];
# ако !BB_MMU
	char **to_free = NULL;
# endif

	xpipe(channel);
	pid = BB_MMU ? xfork() : xvfork();
	ако (pid == 0) { /* child */
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
# ако ENABLE_HUSH_TRAP
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
		ако (is_prefixed_with(s, "trap")
		 && skip_whitespace(s + 4)[0] == '\0'
		) {
			static const char *const argv[] ALIGN_PTR = { NULL, NULL };
			builtin_trap((char**)argv);
			fflush_all(); /* important */
			_exit(0);
		}
# endif
# ако BB_MMU
		/* Prevent it from trying to handle ctrl-z etc */
		IF_HUSH_JOB(G.run_list_level = 1;)
		CLEAR_RANDOM_T(&G.random_gen); /* or else $RANDOM repeats in child */
		reset_traps_to_defaults();
		IF_HUSH_MODE_X(G.x_mode_depth++;)
		//bb_error_msg("%s: ++x_mode_depth=%d", __func__, G.x_mode_depth);
		parse_and_run_string(s);
		_exit(G.last_exitcode);
# иначе
	/* We re-execute after vfork on NOMMU. This makes this script safe:
	 * yes "0123456789012345678901234567890" | dd bs=32 count=64k >BIG
	 * huge=`cat BIG` # was blocking here forever
	 * echo OK
	 */
		re_execute_shell(&to_free,
				s,
				G.global_argv[0],
				G.global_argv + 1,
				NULL);
# endif
	}

	/* parent */
	*pid_p = pid;
# ако ENABLE_HUSH_FAST
	G.count_SIGCHLD++;
//bb_error_msg("[%d] fork in generate_stream_from_string:"
//		" G.count_SIGCHLD:%d G.handled_SIGCHLD:%d",
//		getpid(), G.count_SIGCHLD, G.handled_SIGCHLD);
# endif
	enable_restore_tty_pgrp_on_exit();
# ако !BB_MMU
	free(to_free);
# endif
	close(channel[1]);
	върни channel[0];
}

/* Return code is exit status of the process that is run. */
static цяло process_command_subs(o_string *dest, const char *s)
{
	FILE *fp;
	pid_t pid;
	цяло status, ch, eol_cnt;

	fp = xfdopen_for_read(generate_stream_from_string(s, &pid));

	/* Now send results of command back into original context */
	eol_cnt = 0;
	докато ((ch = getc(fp)) != EOF) {
		ако (ch == '\0')
			continue;
		ако (ch == '\n') {
			eol_cnt++;
			continue;
		}
		докато (eol_cnt) {
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
	върни WEXITSTATUS(status);
}
#endif /* ENABLE_HUSH_TICK */

static void setup_heredoc(struct redir_struct *redir)
{
	struct fd_pair pair;
	pid_t pid;
	цяло len, written;
	/* the _body_ of heredoc (misleading field name) */
	const char *heredoc = redir->rd_filename;
	char *expanded;
#ако !BB_MMU
	char **to_free;
#endif

	expanded = NULL;
	ако (!(redir->rd_dup & HEREDOC_QUOTED)) {
		expanded = encode_then_expand_string(heredoc);
		ако (expanded)
			heredoc = expanded;
	}
	len = strlen(heredoc);

	close(redir->rd_fd); /* often saves dup2+close in xmove_fd */
	xpiped_pair(pair);
	xmove_fd(pair.rd, redir->rd_fd);

	/* Try writing without forking. Newer kernels have
	 * dynamically growing pipes. Must use non-blocking write! */
	ndelay_on(pair.wr);
	докато (1) {
		written = write(pair.wr, heredoc, len);
		ако (written <= 0)
			break;
		len -= written;
		ако (len == 0) {
			close(pair.wr);
			free(expanded);
			върни;
		}
		heredoc += written;
	}
	ndelay_off(pair.wr);

	/* Okay, pipe buffer was not big enough */
	/* Note: we must not create a stray child (bastard? :)
	 * for the unsuspecting parent process. Child creates a grandchild
	 * and exits before parent execs the process which consumes heredoc
	 * (that exec happens after we return from this function) */
#ако !BB_MMU
	to_free = NULL;
#endif
	pid = xvfork();
	ако (pid == 0) {
		/* child */
		disable_restore_tty_pgrp_on_exit();
		pid = BB_MMU ? xfork() : xvfork();
		ако (pid != 0)
			_exit(0);
		/* grandchild */
		close(redir->rd_fd); /* read side of the pipe */
#ако BB_MMU
		full_write(pair.wr, heredoc, len); /* may loop or block */
		_exit(0);
#иначе
		/* Delegate blocking writes to another process */
		xmove_fd(pair.wr, STDOUT_FILENO);
		re_execute_shell(&to_free, heredoc, NULL, NULL, NULL);
#endif
	}
	/* parent */
#ако ENABLE_HUSH_FAST
	G.count_SIGCHLD++;
//bb_error_msg("[%d] fork in setup_heredoc: G.count_SIGCHLD:%d G.handled_SIGCHLD:%d", getpid(), G.count_SIGCHLD, G.handled_SIGCHLD);
#endif
	enable_restore_tty_pgrp_on_exit();
#ако !BB_MMU
	free(to_free);
#endif
	close(pair.wr);
	free(expanded);
	wait(NULL); /* wait till child has died */
}

struct squirrel {
	цяло orig_fd;
	цяло moved_to;
	/* moved_to = n: fd was moved to n; restore back to orig_fd after redir */
	/* moved_to = -1: fd was opened by redirect; close orig_fd after redir */
};

static struct squirrel *append_squirrel(struct squirrel *sq, цяло i, цяло orig, цяло moved)
{
	sq = xrealloc(sq, (i + 2) * sizeof(sq[0]));
	sq[i].orig_fd = orig;
	sq[i].moved_to = moved;
	sq[i+1].orig_fd = -1; /* end marker */
	върни sq;
}

static struct squirrel *add_squirrel(struct squirrel *sq, цяло fd, цяло avoid_fd)
{
	цяло moved_to;
	цяло i;

	i = 0;
	ако (sq) за (; sq[i].orig_fd >= 0; i++) {
		/* If we collide with an already moved fd... */
		ако (fd == sq[i].moved_to) {
			moved_to = dup_CLOEXEC(sq[i].moved_to, avoid_fd);
			debug_printf_redir("redirect_fd %d: already busy, moving to %d\n", fd, moved_to);
			ако (moved_to < 0) {
				/* "echo 2>/dev/tty 10>&9999" testcase:
				 * We move fd 2 to 10, then discover we need to move fd 10
				 * (and not hit 9999) and the latter fails.
				 */
				върни NULL; /* fcntl failed */
			}
			sq[i].moved_to = moved_to;
			върни sq;
		}
		ако (fd == sq[i].orig_fd) {
			/* Example: echo Hello >/dev/null 1>&2 */
			debug_printf_redir("redirect_fd %d: already moved\n", fd);
			върни sq;
		}
	}

	/* If this fd is open, we move and remember it; if it's closed, moved_to = -1 */
	moved_to = dup_CLOEXEC(fd, avoid_fd);
	debug_printf_redir("redirect_fd %d: previous fd is moved to %d (-1 if it was closed)\n", fd, moved_to);
	ако (moved_to < 0 && errno != EBADF)
		върни NULL; /* fcntl failed (not because fd is closed) */
	върни append_squirrel(sq, i, fd, moved_to);
}

static struct squirrel *add_squirrel_closed(struct squirrel *sq, цяло fd)
{
	цяло i;

	i = 0;
	ако (sq) за (; sq[i].orig_fd >= 0; i++) {
		/* If we collide with an already moved fd... */
		ако (fd == sq[i].orig_fd) {
			/* Examples:
			 * "echo 3>FILE 3>&- 3>FILE"
			 * "echo 3>&- 3>FILE"
			 * No need for last redirect to insert
			 * another "need to close 3" indicator.
			 */
			debug_printf_redir("redirect_fd %d: already moved or closed\n", fd);
			върни sq;
		}
	}

	debug_printf_redir("redirect_fd %d: previous fd was closed\n", fd);
	върни append_squirrel(sq, i, fd, -1);
}

/* fd: redirect wants this fd to be used (e.g. 3>file).
 * Move all conflicting internally used fds,
 * and remember them so that we can restore them later.
 */
static цяло save_fd_on_redirect(цяло fd, цяло avoid_fd, struct squirrel **sqp)
{
	struct squirrel *new_squirrel;

	ако (avoid_fd < 9) /* the important case here is that it can be -1 */
		avoid_fd = 9;

#ако ENABLE_HUSH_INTERACTIVE
	ако (fd != 0 /* don't trigger for G_interactive_fd == 0 (that's "not interactive" flag) */
	 && fd == G_interactive_fd
	) {
		/* Testcase: "ls -l /proc/$$/fd 255>&-" should work */
		G_interactive_fd = xdup_CLOEXEC_and_close(G_interactive_fd, avoid_fd);
		debug_printf_redir("redirect_fd %d: matches interactive_fd, moving it to %d\n", fd, G_interactive_fd);
		върни 1; /* "we closed fd" */
	}
#endif
	/* Are we called from setup_redirects(squirrel==NULL)
	 * in redirect in a [v]forked child?
	 */
	ако (sqp == NULL) {
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
		върни 0;
	}

	/* If this one of script's fds? */
	ако (move_HFILEs_on_redirect(fd, avoid_fd))
		върни 1; /* yes. "we closed fd" (actually moved it) */

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
	ако (sqp == ERR_PTR) {
		/* Don't preserve redirected fds: exec is _meant_ to change these */
		//bb_error_msg("sqp == ERR_PTR: exec >FILE");
		върни 0;
	}

	/* Check whether it collides with any open fds (e.g. stdio), save fds as needed */
	new_squirrel = add_squirrel(*sqp, fd, avoid_fd);
	ако (!new_squirrel)
		върни -1; /* redirect error */
	*sqp = new_squirrel;
	върни 0; /* "we did not close fd" */
}

static void restore_redirects(struct squirrel *sq)
{
	ако (sq) {
		цяло i;
		за (i = 0; sq[i].orig_fd >= 0; i++) {
			ако (sq[i].moved_to >= 0) {
				/* We simply die on error */
				debug_printf_redir("restoring redirected fd from %d to %d\n", sq[i].moved_to, sq[i].orig_fd);
				xmove_fd(sq[i].moved_to, sq[i].orig_fd);
			} иначе {
				/* cmd1 9>FILE; cmd2_should_see_fd9_closed */
				debug_printf_redir("restoring redirected fd %d: closing it\n", sq[i].orig_fd);
				close(sq[i].orig_fd);
			}
		}
		free(sq);
	}
	ако (G.HFILE_stdin
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

#ако ENABLE_FEATURE_SH_STANDALONE && BB_MMU
static void close_saved_fds_and_FILE_fds(void)
{
	ако (G_interactive_fd)
		close(G_interactive_fd);
	close_all_HFILE_list();
}
#endif

static цяло internally_opened_fd(цяло fd, struct squirrel *sq)
{
	цяло i;

#ако ENABLE_HUSH_INTERACTIVE
	ако (fd == G_interactive_fd)
		върни 1;
#endif
	/* If this one of script's fds? */
	ако (fd_in_HFILEs(fd))
		върни 1;

	ако (sq) за (i = 0; sq[i].orig_fd >= 0; i++) {
		ако (fd == sq[i].moved_to)
			върни 1;
	}
	върни 0;
}

/* sqp != NULL means we squirrel away copies of stdin, stdout,
 * and stderr if they are redirected.
 * If redirection fails, return 1. This will make caller
 * skip command execution and restore already created redirect fds.
 */
static цяло setup_redirects(struct command *prog, struct squirrel **sqp)
{
	struct redir_struct *redir;

	за (redir = prog->redirects; redir; redir = redir->next) {
		цяло newfd;
		цяло closed;

		ако (redir->rd_type == REDIRECT_HEREDOC2) {
			/* "rd_fd<<HERE" case */
			ако (save_fd_on_redirect(redir->rd_fd, /*avoid:*/ 0, sqp) < 0)
				върни 1;
			/* for REDIRECT_HEREDOC2, rd_filename holds _contents_
			 * of the heredoc */
			debug_printf_redir("set heredoc '%s'\n",
					redir->rd_filename);
			setup_heredoc(redir);
			continue;
		}

		ако (redir->rd_dup == REDIRFD_TO_FILE) {
			/* "rd_fd<*>file" case (<*> is <,>,>>,<>) */
			char *p;
			цяло mode;

			ако (redir->rd_filename == NULL) {
				/* Examples:
				 * "cmd >" (no filename)
				 * "cmd > <file" (2nd redirect starts too early)
				 */
				syntax_error("invalid redirect");
				върни 1;
			}
			mode = redir_table[redir->rd_type].mode;
			p = expand_string_to_string(redir->rd_filename,
				EXP_FLAG_ESC_GLOB_CHARS, /*unbackslash:*/ 1);
			newfd = open_or_warn(p, mode);
			free(p);
			ако (newfd < 0) {
				/* Error message from open_or_warn can be lost
				 * if stderr has been redirected, but bash
				 * and ash both lose it as well
				 * (though zsh doesn't!)
				 */
				върни 1;
			}
			ако (newfd == redir->rd_fd && sqp
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
		} иначе {
			/* "rd_fd>&rd_dup" or "rd_fd>&-" case */
			newfd = redir->rd_dup;
		}

		ако (newfd == redir->rd_fd)
			continue;

		/* if "N>FILE": move newfd to redir->rd_fd */
		/* if "N>&M": dup newfd to redir->rd_fd */
		/* if "N>&-": close redir->rd_fd (newfd is REDIRFD_CLOSE) */

		closed = save_fd_on_redirect(redir->rd_fd, /*avoid:*/ newfd, sqp);
		ако (closed < 0)
			върни 1; /* error */
		ако (newfd == REDIRFD_CLOSE) {
			/* "N>&-" means "close me" */
			ако (!closed) {
				/* ^^^ optimization: saving may already
				 * have closed it. If not... */
				close(redir->rd_fd);
			}
			/* Sometimes we do another close on restore, getting EBADF.
			 * Consider "echo 3>FILE 3>&-"
			 * first redirect remembers "need to close 3",
			 * and second redirect closes 3! Restore code then closes 3 again.
			 */
		} иначе {
			/* if newfd is a script fd or saved fd, do not allow to use it */
			ако (internally_opened_fd(newfd, sqp && sqp != ERR_PTR ? *sqp : NULL)) {
				bb_error_msg("fd#%d is not open", newfd);
				върни 1;
			}
			ако (dup2(newfd, redir->rd_fd) < 0) {
				/* "echo >&99" testcase */
				bb_perror_msg("dup2(%d,%d)", newfd, redir->rd_fd);
				върни 1;
			}
			ако (redir->rd_dup == REDIRFD_TO_FILE)
				/* "rd_fd > FILE" */
				close(newfd);
			/* else: "rd_fd > rd_dup" */
		}
	}
	върни 0;
}

static char *find_in_path(const char *arg)
{
	char *ret = NULL;
	const char *PATH = get_local_var_value("PATH");

	ако (!PATH)
		върни NULL;

	докато (1) {
		const char *end = strchrnul(PATH, ':');
		цяло sz = end - PATH; /* must be int! */

		free(ret);
		ако (sz != 0) {
			ret = xasprintf("%.*s/%s", sz, PATH, arg);
		} иначе {
			/* We have xxx::yyyy in $PATH,
			 * it means "use current dir" */
			ret = xstrdup(arg);
		}
		ако (access(ret, F_OK) == 0)
			break;

		ако (*end == '\0') {
			free(ret);
			върни NULL;
		}
		PATH = end + 1;
	}

	върни ret;
}

static const struct built_in_command *find_builtin_helper(const char *name,
		const struct built_in_command *x,
		const struct built_in_command *end)
{
	докато (x != end) {
		ако (strcmp(name, x->b_cmd) != 0) {
			x++;
			continue;
		}
		debug_printf_exec("found builtin '%s'\n", name);
		върни x;
	}
	върни NULL;
}
static const struct built_in_command *find_builtin1(const char *name)
{
	върни find_builtin_helper(name, bltins1, &bltins1[ARRAY_SIZE(bltins1)]);
}
static const struct built_in_command *find_builtin(const char *name)
{
	const struct built_in_command *x = find_builtin1(name);
	ако (x)
		върни x;
	върни find_builtin_helper(name, bltins2, &bltins2[ARRAY_SIZE(bltins2)]);
}

#ако ENABLE_HUSH_JOB && ENABLE_FEATURE_TAB_COMPLETION
static const char * FAST_FUNC hush_command_name(цяло i)
{
	ако (/*i >= 0 && */ i < ARRAY_SIZE(bltins1)) {
		върни bltins1[i].b_cmd;
	}
	i -= ARRAY_SIZE(bltins1);
	ако (i < ARRAY_SIZE(bltins2)) {
		върни bltins2[i].b_cmd;
	}
# ако ENABLE_HUSH_FUNCTIONS
	{
		struct function *funcp;
		i -= ARRAY_SIZE(bltins2);
		за (funcp = G.top_func; funcp; funcp = funcp->next) {
			ако (--i < 0)
				върни funcp->name;
		}
	}
# endif
	върни NULL;
}
#endif
#endif /* !__U_BOOT__ */

#ifndef __U_BOOT__
static void remove_nested_vars(void)
{
	struct variable *cur;
	struct variable **cur_pp;

	cur_pp = &G.top_var;
	докато ((cur = *cur_pp) != NULL) {
		ако (cur->var_nest_level <= G.var_nest_level) {
			cur_pp = &cur->next;
			continue;
		}
		/* Unexport */
		ако (cur->flg_export) {
			debug_printf_env("unexporting nested '%s'/%u\n", cur->varstr, cur->var_nest_level);
			bb_unsetenv(cur->varstr);
		}
		/* Remove from global list */
		*cur_pp = cur->next;
		/* Free */
		ако (!cur->max_len) {
			debug_printf_env("freeing nested '%s'/%u\n", cur->varstr, cur->var_nest_level);
			free(cur->varstr);
		}
		free(cur);
	}
}

static void enter_var_nest_level(void)
{
	G.var_nest_level++;
	debug_printf_env("var_nest_level++ %u\n", G.var_nest_level);

	/* Try:	f() { echo -n .; f; }; f
	 * struct variable::var_nest_level is uint16_t,
	 * thus limiting recursion to < 2^16.
	 * In any case, with 8 Mbyte stack SEGV happens
	 * not too long after 2^16 recursions anyway.
	 */
	ако (G.var_nest_level > 0xff00)
		bb_error_msg_and_die("fatal recursion (depth %u)", G.var_nest_level);
}

static void leave_var_nest_level(void)
{
	G.var_nest_level--;
	debug_printf_env("var_nest_level-- %u\n", G.var_nest_level);
	ако (HUSH_DEBUG && (цяло)G.var_nest_level < 0)
		bb_simple_error_msg_and_die("BUG: nesting underflow");

	remove_nested_vars();
}
#endif /* __U_BOOT__ */

#ако ENABLE_HUSH_FUNCTIONS
static struct function **find_function_slot(const char *name)
{
	struct function *funcp;
	struct function **funcpp = &G.top_func;

	докато ((funcp = *funcpp) != NULL) {
		ако (strcmp(name, funcp->name) == 0) {
			debug_printf_exec("found function '%s'\n", name);
			break;
		}
		funcpp = &funcp->next;
	}
	върни funcpp;
}

static ALWAYS_INLINE const struct function *find_function(const char *name)
{
	const struct function *funcp = *find_function_slot(name);
	върни funcp;
}

/* Note: takes ownership on name ptr */
static struct function *new_function(char *name)
{
	struct function **funcpp = find_function_slot(name);
	struct function *funcp = *funcpp;

	ако (funcp != NULL) {
		struct command *cmd = funcp->parent_cmd;
		debug_printf_exec("func %p parent_cmd %p\n", funcp, cmd);
		ако (!cmd) {
			debug_printf_exec("freeing & replacing function '%s'\n", funcp->name);
			free(funcp->name);
			/* Note: if !funcp->body, do not free body_as_string!
			 * This is a special case of "-F name body" function:
			 * body_as_string was not malloced! */
			ако (funcp->body) {
				free_pipe_list(funcp->body);
# ако !BB_MMU
				free(funcp->body_as_string);
# endif
			}
		} иначе {
			debug_printf_exec("reinserting in tree & replacing function '%s'\n", funcp->name);
			cmd->argv[0] = funcp->name;
			cmd->group = funcp->body;
# ако !BB_MMU
			cmd->group_as_string = funcp->body_as_string;
# endif
		}
	} иначе {
		debug_printf_exec("remembering new function '%s'\n", name);
		funcp = *funcpp = xzalloc(sizeof(*funcp));
		/*funcp->next = NULL;*/
	}

	funcp->name = name;
	върни funcp;
}

# ако ENABLE_HUSH_UNSET
static void unset_func(const char *name)
{
	struct function **funcpp = find_function_slot(name);
	struct function *funcp = *funcpp;

	ако (funcp != NULL) {
		debug_printf_exec("freeing function '%s'\n", funcp->name);
		*funcpp = funcp->next;
		/* funcp is unlinked now, deleting it.
		 * Note: if !funcp->body, the function was created by
		 * "-F name body", do not free ->body_as_string
		 * and ->name as they were not malloced. */
		ако (funcp->body) {
			free_pipe_list(funcp->body);
			free(funcp->name);
#  ако !BB_MMU
			free(funcp->body_as_string);
#  endif
		}
		free(funcp);
	}
}
# endif

# ако BB_MMU
#define exec_function(to_free, funcp, argv) \
	exec_function(funcp, argv)
# endif
static void exec_function(char ***to_free,
		const struct function *funcp,
		char **argv) NORETURN;
static void exec_function(char ***to_free,
		const struct function *funcp,
		char **argv)
{
# ако BB_MMU
	цяло n;

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
# иначе
//?	close_saved_fds_and_FILE_fds();

//TODO: check whether "true | func_with_return" works

	re_execute_shell(to_free,
			funcp->body_as_string,
			G.global_argv[0],
			argv + 1,
			NULL);
# endif
}

static цяло run_function(const struct function *funcp, char **argv)
{
	цяло rc;
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
# ако !BB_MMU
	ако (!funcp->body) {
		/* Function defined by -F */
		parse_and_run_string(funcp->body_as_string);
		rc = G.last_exitcode;
	} иначе
# endif
	{
		rc = run_list(funcp->body);
	}

	IF_HUSH_LOCAL(G.func_nest_level--;)
	IF_HUSH_LOCAL(leave_var_nest_level();)

	G_flag_return_in_progress = sv_flg;
# ако ENABLE_HUSH_TRAP
	debug_printf_exec("G.return_exitcode=-1\n");
	G.return_exitcode = -1; /* invalidate stashed return value */
# endif

	restore_G_args(&sv, argv);

	върни rc;
}
#endif /* ENABLE_HUSH_FUNCTIONS */

#ifndef __U_BOOT__
#ако BB_MMU
#define exec_builtin(to_free, x, argv) \
	exec_builtin(x, argv)
#иначе
#define exec_builtin(to_free, x, argv) \
	exec_builtin(to_free, argv)
#endif
static void exec_builtin(char ***to_free,
		const struct built_in_command *x,
		char **argv) NORETURN;
static void exec_builtin(char ***to_free,
		const struct built_in_command *x,
		char **argv)
{
#ако BB_MMU
	цяло rcode;
//?	close_saved_fds_and_FILE_fds();
	rcode = x->b_function(argv);
	fflush_all();
	_exit(rcode);
#иначе
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
static void execvp_or_die(char **argv) NORETURN;
static void execvp_or_die(char **argv)
{
	цяло e;
	debug_printf_exec("execing '%s'\n", argv[0]);
	/* Don't propagate SIG_IGN to the child */
	ако (SPECIAL_JOBSTOP_SIGS != 0)
		switch_off_special_sigs(G.special_sig_mask & SPECIAL_JOBSTOP_SIGS);
	execvp(argv[0], argv);
	e = 2;
	ако (errno == EACCES) e = 126;
	ако (errno == ENOENT) e = 127;
	bb_perror_msg("can't execute '%s'", argv[0]);
	_exit(e);
}

#ако ENABLE_HUSH_MODE_X
static void x_mode_print_optionally_squoted(const char *str)
{
	unsigned len;
	const char *cp;

	cp = str;

	/* the set of chars which-cause-string-to-be-squoted mimics bash */
	/* test a char with: bash -c 'set -x; echo "CH"' */
	ако (str[strcspn(str, "\\\"'`$(){}[]<>;#&|~*?!^"
			" " "\001\002\003\004\005\006\007"
			"\010\011\012\013\014\015\016\017"
			"\020\021\022\023\024\025\026\027"
			"\030\031\032\033\034\035\036\037"
			)
		] == '\0'
	) {
		/* string has no special chars */
		x_mode_addstr(str);
		върни;
	}

	cp = str;
	за (;;) {
		/* print '....' up to EOL or first squote */
		len = (цяло)(strchrnul(cp, '\'') - cp);
		ако (len != 0) {
			x_mode_addchr('\'');
			x_mode_addblock(cp, len);
			x_mode_addchr('\'');
			cp += len;
		}
		ако (*cp == '\0')
			break;
		/* string contains squote(s), print them as \' */
		x_mode_addchr('\\');
		x_mode_addchr('\'');
		cp++;
	}
}
static void dump_cmd_in_x_mode(char **argv)
{
	ако (G_x_mode && argv) {
		unsigned n;

		/* "+[+++...][ cmd...]\n\0" */
		x_mode_prefix();
		n = 0;
		докато (argv[n]) {
			x_mode_addchr(' ');
			ако (argv[n][0] == '\0') {
				x_mode_addchr('\'');
				x_mode_addchr('\'');
			} иначе {
				x_mode_print_optionally_squoted(argv[n]);
			}
			n++;
		}
		x_mode_flush();
	}
}
#иначе
# define dump_cmd_in_x_mode(argv) ((void)0)
#endif
#endif /* !__U_BOOT__ */

#ifndef __U_BOOT__
#ако ENABLE_HUSH_COMMAND
static void if_command_vV_print_and_exit(char opt_vV, char *cmd, const char *explanation)
{
	char *to_free;

	ако (!opt_vV)
		върни;

	to_free = NULL;
	ако (!explanation) {
		char *path = getenv("PATH");
		explanation = to_free = find_executable(cmd, &path); /* path == NULL is ok */
		ако (!explanation)
			_exit(1); /* PROG was not found */
		ако (opt_vV != 'V')
			cmd = to_free; /* -v PROG prints "/path/to/PROG" */
	}
	printf((opt_vV == 'V') ? "%s is %s\n" : "%s\n", cmd, explanation);
	free(to_free);
	fflush_all();
	_exit(0);
}
#иначе
# define if_command_vV_print_and_exit(a,b,c) ((void)0)
#endif
#endif /* !__U_BOOT__ */

#ако BB_MMU
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
static void pseudo_exec_argv(nommu_save_t *nommu_save,
		char **argv, цяло assignment_cnt,
		char **argv_expanded) NORETURN;
static NOINLINE void pseudo_exec_argv(nommu_save_t *nommu_save,
		char **argv, цяло assignment_cnt,
		char **argv_expanded)
{
	const struct built_in_command *x;
	struct variable **sv_shadowed;
	char **new_env;
	IF_HUSH_COMMAND(char opt_vV = 0;)
	IF_HUSH_FUNCTIONS(const struct function *funcp;)

	new_env = expand_assignments(argv, assignment_cnt);
	dump_cmd_in_x_mode(new_env);

	ако (!argv[assignment_cnt]) {
		/* Case when we are here: ... | var=val | ...
		 * (note that we do not exit early, i.e., do not optimize out
		 * expand_assignments(): think about ... | var=`sleep 1` | ...
		 */
		free_strings(new_env);
		_exit_SUCCESS();
	}

	sv_shadowed = G.shadowed_vars_pp;
#ако BB_MMU
	G.shadowed_vars_pp = NULL; /* "don't save, free them instead" */
#иначе
	G.shadowed_vars_pp = &nommu_save->old_vars;
	G.var_nest_level++;
#endif
	set_vars_and_save_old(new_env);
	G.shadowed_vars_pp = sv_shadowed;

	ако (argv_expanded) {
		argv = argv_expanded;
	} иначе {
		argv = expand_strvec_to_strvec(argv + assignment_cnt);
#ако !BB_MMU
		nommu_save->argv = argv;
#endif
	}
	dump_cmd_in_x_mode(argv);

#ако ENABLE_FEATURE_SH_STANDALONE || BB_MMU
	ако (strchr(argv[0], '/') != NULL)
		goto skip;
#endif

#ако ENABLE_HUSH_FUNCTIONS
	/* Check if the command matches any functions (this goes before bltins) */
	funcp = find_function(argv[0]);
	ако (funcp)
		exec_function(&nommu_save->argv_from_re_execing, funcp, argv);
#endif

#ако ENABLE_HUSH_COMMAND
	/* "command BAR": run BAR without looking it up among functions
	 * "command -v BAR": print "BAR" or "/path/to/BAR"; or exit 1
	 * "command -V BAR": print "BAR is {a function,a shell builtin,/path/to/BAR}"
	 */
	докато (strcmp(argv[0], "command") == 0 && argv[1]) {
		char *p;

		argv++;
		p = *argv;
		ако (p[0] != '-' || !p[1])
			continue; /* bash allows "command command command [-OPT] BAR" */

		за (;;) {
			p++;
			switch (*p) {
			case '\0':
				argv++;
				p = *argv;
				ако (p[0] != '-' || !p[1])
					goto after_opts;
				continue; /* next arg is also -opts, process it too */
			case 'v':
			case 'V':
				opt_vV = *p;
				continue;
			default:
				bb_error_msg_and_die("%s: %s: invalid option", "command", argv[0]);
			}
		}
	}
 after_opts:
# ако ENABLE_HUSH_FUNCTIONS
	ако (opt_vV && find_function(argv[0]))
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
	ако (x) {
		if_command_vV_print_and_exit(opt_vV, argv[0], "a shell builtin");
		exec_builtin(&nommu_save->argv_from_re_execing, x, argv);
	}

#ако ENABLE_FEATURE_SH_STANDALONE
	/* Check if the command matches any busybox applets */
	{
		цяло a = find_applet_by_name(argv[0]);
		ако (a >= 0) {
			if_command_vV_print_and_exit(opt_vV, argv[0], "an applet");
# ако BB_MMU /* see above why on NOMMU it is not allowed */
			ако (APPLET_IS_NOEXEC(a)) {
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
			ако (SPECIAL_JOBSTOP_SIGS != 0)
				switch_off_special_sigs(G.special_sig_mask & SPECIAL_JOBSTOP_SIGS);
			execv(bb_busybox_exec_path, argv);
			/* If they called chroot or otherwise made the binary no longer
			 * executable, fall through */
		}
	}
#endif

#ако ENABLE_FEATURE_SH_STANDALONE || BB_MMU
 skip:
#endif
	if_command_vV_print_and_exit(opt_vV, argv[0], NULL);
	execvp_or_die(argv);
}

/* Called after [v]fork() in run_pipe
 */
static void pseudo_exec(nommu_save_t *nommu_save,
		struct command *command,
		char **argv_expanded) NORETURN;
static void pseudo_exec(nommu_save_t *nommu_save,
		struct command *command,
		char **argv_expanded)
{
#ако ENABLE_HUSH_FUNCTIONS
	ако (command->cmd_type == CMD_FUNCDEF) {
		/* Ignore funcdefs in pipes:
		 * true | f() { cmd }
		 */
		_exit(0);
	}
#endif

	ако (command->argv) {
		pseudo_exec_argv(nommu_save, command->argv,
				command->assignment_cnt, argv_expanded);
	}

	ако (command->group) {
		/* Cases when we are here:
		 * ( list )
		 * { list } &
		 * ... | ( list ) | ...
		 * ... | { list } | ...
		 */
#ако BB_MMU
		цяло rcode;
		debug_printf_exec("pseudo_exec: run_list\n");
		reset_traps_to_defaults();
		rcode = run_list(command->group);
		/* OK to leak memory by not calling free_pipe_list,
		 * since this process is about to exit */
		_exit(rcode);
#иначе
		re_execute_shell(&nommu_save->argv_from_re_execing,
				command->group_as_string,
				G.global_argv[0],
				G.global_argv + 1,
				NULL);
#endif
	}

	/* Case when we are here: ... | >file */
	debug_printf_exec("pseudo_exec'ed null command\n");
	_exit_SUCCESS();
}

#ако ENABLE_HUSH_JOB
static const char *get_cmdtext(struct pipe *pi)
{
	char **argv;
	char *p;
	цяло len;

	/* This is subtle. ->cmdtext is created only on first backgrounding.
	 * (Think "cat, <ctrl-z>, fg, <ctrl-z>, fg, <ctrl-z>...." here...)
	 * On subsequent bg argv is trashed, but we won't use it */
	ако (pi->cmdtext)
		върни pi->cmdtext;

	argv = pi->cmds[0].argv;
	ако (!argv) {
		pi->cmdtext = xzalloc(1);
		върни pi->cmdtext;
	}
	len = 0;
	do {
		len += strlen(*argv) + 1;
	} докато (*++argv);
	p = xmalloc(len);
	pi->cmdtext = p;
	argv = pi->cmds[0].argv;
	do {
		p = stpcpy(p, *argv);
		*p++ = ' ';
	} докато (*++argv);
	p[-1] = '\0';
	върни pi->cmdtext;
}

static void remove_job_from_table(struct pipe *pi)
{
	struct pipe *prev_pipe;

	ако (pi == G.job_list) {
		G.job_list = pi->next;
	} иначе {
		prev_pipe = G.job_list;
		докато (prev_pipe->next != pi)
			prev_pipe = prev_pipe->next;
		prev_pipe->next = pi->next;
	}
	G.last_jobid = 0;
	ако (G.job_list)
		G.last_jobid = G.job_list->jobid;
}

static void delete_finished_job(struct pipe *pi)
{
	remove_job_from_table(pi);
	free_pipe(pi);
}

static void clean_up_last_dead_job(void)
{
	ако (G.job_list && !G.job_list->alive_cmds)
		delete_finished_job(G.job_list);
}

static void insert_job_into_table(struct pipe *pi)
{
	struct pipe *job, **jobp;
	цяло i;

	clean_up_last_dead_job();

	/* Find the end of the list, and find next job ID to use */
	i = 0;
	jobp = &G.job_list;
	докато ((job = *jobp) != NULL) {
		ако (job->jobid > i)
			i = job->jobid;
		jobp = &job->next;
	}
	pi->jobid = i + 1;

	/* Create a new job struct at the end */
	job = *jobp = xmemdup(pi, sizeof(*pi));
	job->next = NULL;
	job->cmds = xzalloc(sizeof(pi->cmds[0]) * pi->num_cmds);
	/* Cannot copy entire pi->cmds[] vector! This causes double frees */
	за (i = 0; i < pi->num_cmds; i++) {
		job->cmds[i].pid = pi->cmds[i].pid;
		/* all other fields are not used and stay zero */
	}
	job->cmdtext = xstrdup(get_cmdtext(pi));

	ако (G_interactive_fd)
		printf("[%u] %u %s\n", job->jobid, (unsigned)job->cmds[0].pid, job->cmdtext);
	G.last_jobid = job->jobid;
}
#endif /* JOB */

static цяло job_exited_or_stopped(struct pipe *pi)
{
	цяло rcode, i;

	ако (pi->alive_cmds != pi->stopped_cmds)
		върни -1;

	/* All processes in fg pipe have exited or stopped */
	rcode = 0;
	i = pi->num_cmds;
	докато (--i >= 0) {
		rcode = pi->cmds[i].cmd_exitcode;
		/* usually last process gives overall exitstatus,
		 * but with "set -o pipefail", last *failed* process does */
		ако (G.o_opt[OPT_O_PIPEFAIL] == 0 || rcode != 0)
			break;
	}
	IF_HAS_KEYWORDS(ако (pi->pi_inverted) rcode = !rcode;)
	върни rcode;
}

static цяло process_wait_result(struct pipe *fg_pipe, pid_t childpid, цяло status)
{
#ако ENABLE_HUSH_JOB
	struct pipe *pi;
#endif
	цяло i, dead;

	dead = WIFEXITED(status) || WIFSIGNALED(status);

#ако DEBUG_JOBS
	ако (WIFSTOPPED(status))
		debug_printf_jobs("pid %d stopped by sig %d (exitcode %d)\n",
				childpid, WSTOPSIG(status), WEXITSTATUS(status));
	ако (WIFSIGNALED(status))
		debug_printf_jobs("pid %d killed by sig %d (exitcode %d)\n",
				childpid, WTERMSIG(status), WEXITSTATUS(status));
	ако (WIFEXITED(status))
		debug_printf_jobs("pid %d exited, exitcode %d\n",
				childpid, WEXITSTATUS(status));
#endif
	/* Were we asked to wait for a fg pipe? */
	ако (fg_pipe) {
		i = fg_pipe->num_cmds;

		докато (--i >= 0) {
			цяло rcode;

			debug_printf_jobs("check pid %d\n", fg_pipe->cmds[i].pid);
			ако (fg_pipe->cmds[i].pid != childpid)
				continue;
			ако (dead) {
				цяло ex;
				fg_pipe->cmds[i].pid = 0;
				fg_pipe->alive_cmds--;
				ex = WEXITSTATUS(status);
				/* bash prints killer signal's name for *last*
				 * process in pipe (prints just newline for SIGINT/SIGPIPE).
				 * Mimic this. Example: "sleep 5" + (^\ or kill -QUIT)
				 */
				ако (WIFSIGNALED(status)) {
					цяло sig = WTERMSIG(status);
#ако ENABLE_HUSH_JOB
					ако (G.run_list_level == 1
					/* ^^^^^ Do not print in nested contexts, example:
					 * echo `sleep 1; sh -c 'kill -9 $$'` - prints "137", NOT "Killed 137"
					 */
					 && i == fg_pipe->num_cmds-1
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
			} иначе {
				fg_pipe->stopped_cmds++;
			}
			debug_printf_jobs("fg_pipe: alive_cmds %d stopped_cmds %d\n",
					fg_pipe->alive_cmds, fg_pipe->stopped_cmds);
			rcode = job_exited_or_stopped(fg_pipe);
			ако (rcode >= 0) {
/* Note: *non-interactive* bash does not continue if all processes in fg pipe
 * are stopped. Testcase: "cat | cat" in a script (not on command line!)
 * and "killall -STOP cat" */
				ако (G_interactive_fd) {
#ако ENABLE_HUSH_JOB
					ако (fg_pipe->alive_cmds != 0)
						insert_job_into_table(fg_pipe);
#endif
					върни rcode;
				}
				ако (fg_pipe->alive_cmds == 0)
					върни rcode;
			}
			/* There are still running processes in the fg_pipe */
			върни -1;
		}
		/* It wasn't in fg_pipe, look for process in bg pipes */
	}

#ако ENABLE_HUSH_JOB
	/* We were asked to wait for bg or orphaned children */
	/* No need to remember exitcode in this case */
	за (pi = G.job_list; pi; pi = pi->next) {
		за (i = 0; i < pi->num_cmds; i++) {
			ако (pi->cmds[i].pid == childpid)
				goto found_pi_and_prognum;
		}
	}
	/* Happens when shell is used as init process (init=/bin/sh) */
	debug_printf("checkjobs: pid %d was not in our list!\n", childpid);
	върни -1; /* this wasn't a process from fg_pipe */

 found_pi_and_prognum:
	ако (dead) {
		/* child exited */
		цяло rcode = WEXITSTATUS(status);
		ако (WIFSIGNALED(status))
			/* NB: not 128 + sig, MIPS has sig 128 */
			rcode = 128 | WTERMSIG(status);
		pi->cmds[i].cmd_exitcode = rcode;
		ако (G.last_bg_pid == pi->cmds[i].pid)
			G.last_bg_pid_exitcode = rcode;
		pi->cmds[i].pid = 0;
		pi->alive_cmds--;
		ако (!pi->alive_cmds) {
# ако ENABLE_HUSH_BASH_COMPAT
			G.dead_job_exitcode = job_exited_or_stopped(pi);
# endif
			ако (G_interactive_fd) {
				printf(JOB_STATUS_FORMAT, pi->jobid,
						"Done", pi->cmdtext);
				delete_finished_job(pi);
			} иначе {
/*
 * bash deletes finished jobs from job table only in interactive mode,
 * after "jobs" cmd, or if pid of a new process matches one of the old ones
 * (see cleanup_dead_jobs(), delete_old_job(), J_NOTIFIED in bash source).
 * Testcase script: "(exit 3) & sleep 1; wait %1; echo $?" prints 3 in bash.
 * We only retain one "dead" job, if it's the single job on the list.
 * This covers most of real-world scenarios where this is useful.
 */
				ако (pi != G.job_list)
					delete_finished_job(pi);
			}
		}
	} иначе {
		/* child stopped */
		pi->stopped_cmds++;
	}
#endif
	върни -1; /* this wasn't a process from fg_pipe */
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
static цяло checkjobs(struct pipe *fg_pipe, pid_t waitfor_pid)
{
	цяло attributes;
	цяло status;
	цяло rcode = 0;

	debug_printf_jobs("checkjobs %p\n", fg_pipe);

	attributes = WUNTRACED;
	ако (fg_pipe == NULL)
		attributes |= WNOHANG;

	errno = 0;
#ако ENABLE_HUSH_FAST
	ако (G.handled_SIGCHLD == G.count_SIGCHLD) {
//bb_error_msg("[%d] checkjobs: G.count_SIGCHLD:%d G.handled_SIGCHLD:%d children?:%d fg_pipe:%p",
//getpid(), G.count_SIGCHLD, G.handled_SIGCHLD, G.we_have_children, fg_pipe);
		/* There was neither fork nor SIGCHLD since last waitpid */
		/* Avoid doing waitpid syscall if possible */
		ако (!G.we_have_children) {
			errno = ECHILD;
			върни -1;
		}
		ако (fg_pipe == NULL) { /* is WNOHANG set? */
			/* We have children, but they did not exit
			 * or stop yet (we saw no SIGCHLD) */
			върни 0;
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
	докато (1) {
		pid_t childpid;
#ако ENABLE_HUSH_FAST
		цяло i;
		i = G.count_SIGCHLD;
#endif
		childpid = waitpid(-1, &status, attributes);
		ако (childpid <= 0) {
			ако (childpid && errno != ECHILD)
				bb_simple_perror_msg("waitpid");
#ако ENABLE_HUSH_FAST
			иначе { /* Until next SIGCHLD, waitpid's are useless */
				G.we_have_children = (childpid == 0);
				G.handled_SIGCHLD = i;
//bb_error_msg("[%d] checkjobs: waitpid returned <= 0, G.count_SIGCHLD:%d G.handled_SIGCHLD:%d", getpid(), G.count_SIGCHLD, G.handled_SIGCHLD);
			}
#endif
			/* ECHILD (no children), or 0 (no change in children status) */
			rcode = childpid;
			break;
		}
		rcode = process_wait_result(fg_pipe, childpid, status);
		ако (rcode >= 0) {
			/* fg_pipe exited or stopped */
			break;
		}
		ако (childpid == waitfor_pid) { /* "wait PID" */
			debug_printf_exec("childpid==waitfor_pid:%d status:0x%08x\n", childpid, status);
			rcode = WEXITSTATUS(status);
			ако (WIFSIGNALED(status))
				rcode = 128 | WTERMSIG(status);
			ако (WIFSTOPPED(status))
				/* bash: "cmd & wait $!" and cmd stops: $? = 128 | stopsig */
				rcode = 128 | WSTOPSIG(status);
			rcode++;
			break; /* "wait PID" called us, give it exitcode+1 */
		}
#ако ENABLE_HUSH_BASH_COMPAT
		ако (-1 == waitfor_pid /* "wait -n" (wait for any one job) */
		 && G.dead_job_exitcode >= 0 /* some job did finish */
		) {
			debug_printf_exec("waitfor_pid:-1\n");
			rcode = G.dead_job_exitcode + 1;
			break;
		}
#endif
		/* This wasn't one of our processes, or */
		/* fg_pipe still has running processes, do waitpid again */
	} /* while (waitpid succeeds)... */

	върни rcode;
}

#ако ENABLE_HUSH_JOB
static цяло checkjobs_and_fg_shell(struct pipe *fg_pipe)
{
	pid_t p;
	цяло rcode = checkjobs(fg_pipe, 0 /*(no pid to wait for)*/);
	ако (G_saved_tty_pgrp) {
		/* Job finished, move the shell to the foreground */
		p = getpgrp(); /* our process group id */
		debug_printf_jobs("fg'ing ourself: getpgrp()=%d\n", (цяло)p);
		tcsetpgrp(G_interactive_fd, p);
	}
	върни rcode;
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
static void set_G_ifs(void)
{
	/* Testcase: set -- q w e; (IFS='' echo "$*"; IFS=''; echo "$*"); echo "$*"
	 * Result should be 3 lines: q w e, qwe, q w e
	 */
	ако (G.ifs_whitespace != G.ifs)
		free(G.ifs_whitespace);
	G.ifs = get_local_var_value("IFS");
	ако (G.ifs) {
		char *p;
		G.ifs_whitespace = (char*)G.ifs;
		p = skip_whitespace(G.ifs);
		ако (*p) {
			/* Not all $IFS is whitespace */
			char *d;
			цяло len = p - G.ifs;
			p = skip_non_whitespace(p);
			G.ifs_whitespace = xmalloc(len + strlen(p) + 1); /* can overestimate */
			d = mempcpy(G.ifs_whitespace, G.ifs, len);
			докато (*p) {
				ако (isspace(*p))
					*d++ = *p;
				p++;
			}
			*d = '\0';
		}
	} иначе {
		G.ifs = defifs;
		G.ifs_whitespace = (char*)G.ifs;
	}
}
#ifndef __U_BOOT__
#ако !ENABLE_HUSH_MODE_X
#define redirect_and_varexp_helper(command, sqp, argv_expanded) \
	redirect_and_varexp_helper(command, sqp)
#endif
static цяло redirect_and_varexp_helper(
		struct command *command,
		struct squirrel **sqp,
		char **argv_expanded)
{
	/* Assignments occur before redirects. Try:
	 * a=`sleep 1` sleep 2 3>/qwe/rty
	 */

	char **new_env = expand_assignments(command->argv, command->assignment_cnt);
	dump_cmd_in_x_mode(new_env);
	dump_cmd_in_x_mode(argv_expanded);
	/* this takes ownership of new_env[i] elements, and frees new_env: */
	set_vars_and_save_old(new_env);

	върни setup_redirects(command, sqp);
}
#endif /* !__U_BOOT__ */

static NOINLINE цяло run_pipe(struct pipe *pi)
{
	static const char *const null_ptr = NULL;

	цяло cmd_no;
#ifndef __U_BOOT__
	цяло next_infd;
#endif /* !__U_BOOT__ */
	struct command *command;
	char **argv_expanded;
	char **argv;
#ifndef __U_BOOT__
	struct squirrel *squirrel = NULL;
#endif /* !__U_BOOT__ */
	цяло rcode;

#ifdef __U_BOOT__
	/*
	 * Set rcode here to avoid returning a garbage value in the middle of
	 * the function.
	 * Also, if an error occurs, rcode value would be changed and last
	 * return will signal the error.
	 */
	rcode = 0;
#endif /* __U_BOOT__ */

	debug_printf_exec("run_pipe start: members:%d\n", pi->num_cmds);
	debug_enter();

	set_G_ifs();

#ifndef __U_BOOT__
	IF_HUSH_JOB(pi->pgrp = -1;)
	pi->stopped_cmds = 0;
#endif /* !__U_BOOT__ */
	command = &pi->cmds[0];
	argv_expanded = NULL;

#ifndef __U_BOOT__
	ако (pi->num_cmds != 1
	 || pi->followup == PIPE_BG
	 || command->cmd_type == CMD_SUBSHELL
	) {
		goto must_fork;
	}

	pi->alive_cmds = 1;
#endif /* !__U_BOOT__ */

	debug_printf_exec(": group:%p argv:'%s'\n",
		command->group, command->argv ? command->argv[0] : "NONE");

	ако (command->group) {
#ако ENABLE_HUSH_FUNCTIONS
		ако (command->cmd_type == CMD_FUNCDEF) {
			/* "executing" func () { list } */
			struct function *funcp;

			funcp = new_function(command->argv[0]);
			/* funcp->name is already set to argv[0] */
			funcp->body = command->group;
# ако !BB_MMU
			funcp->body_as_string = command->group_as_string;
			command->group_as_string = NULL;
# endif
			command->group = NULL;
			command->argv[0] = NULL;
			debug_printf_exec("cmd %p has child func at %p\n", command, funcp);
			funcp->parent_cmd = command;
			command->child_func = funcp;

			debug_printf_exec("run_pipe: return EXIT_SUCCESS\n");
			debug_leave();
			върни EXIT_SUCCESS;
		}
#endif
		/* { list } */
		debug_printf_exec("non-subshell group\n");
		rcode = 1; /* exitcode if redir failed */
#ifndef __U_BOOT__
		ако (setup_redirects(command, &squirrel) == 0) {
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
		IF_HAS_KEYWORDS(ако (pi->pi_inverted) rcode = !rcode;)
		debug_leave();
		debug_printf_exec("run_pipe: return %d\n", rcode);
		върни rcode;
	}

	argv = command->argv ? command->argv : (char **) &null_ptr;
	{
#ifndef __U_BOOT__
		const struct built_in_command *x;
		IF_HUSH_FUNCTIONS(const struct function *funcp;)
		IF_NOT_HUSH_FUNCTIONS(enum { funcp = 0 };)
		struct variable **sv_shadowed;
#endif /* !__U_BOOT__ */
		struct variable *old_vars;

#ако ENABLE_HUSH_LINENO_VAR
		G.execute_lineno = command->lineno;
#endif

		ако (argv[command->assignment_cnt] == NULL) {
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
			докато (i < command->assignment_cnt) {
				char *p = expand_string_to_string(argv[i],
						EXP_FLAG_ESC_GLOB_CHARS,
						/*unbackslash:*/ 1
				);
#ако ENABLE_HUSH_MODE_X
				ако (G_x_mode) {
					char *eq;
					ако (i == 0)
						x_mode_prefix();
					x_mode_addchr(' ');
					eq = strchrnul(p, '=');
					ако (*eq) eq++;
					x_mode_addblock(p, (eq - p));
					x_mode_print_optionally_squoted(eq);
					x_mode_flush();
				}
#endif
				debug_printf_env("set shell var:'%s'->'%s'\n", *argv, p);
#ifndef __U_BOOT__
				ако (set_local_var0(p)) {
#иначе /* __U_BOOT__ */
				ако (set_local_var_modern(p, /*flag:*/ 0)) {
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
			ако (rcode == 0)
				rcode = G.expand_exitcode;
			IF_HAS_KEYWORDS(ако (pi->pi_inverted) rcode = !rcode;)
			debug_leave();
			debug_printf_exec("run_pipe: return %d\n", rcode);
			върни rcode;
		}

		/* Expand the rest into (possibly) many strings each */
#ако defined(CMD_TEST2_SINGLEWORD_NOGLOB)
		ако (command->cmd_type == CMD_TEST2_SINGLEWORD_NOGLOB)
			argv_expanded = expand_strvec_to_strvec_singleword_noglob(argv + command->assignment_cnt);
		иначе
#endif
#ако defined(CMD_SINGLEWORD_NOGLOB)
		ако (command->cmd_type == CMD_SINGLEWORD_NOGLOB)
			argv_expanded = expand_strvec_to_strvec_singleword_noglob(argv + command->assignment_cnt);
		иначе
#endif
			argv_expanded = expand_strvec_to_strvec(argv + command->assignment_cnt);

		/* If someone gives us an empty string: `cmd with empty output` */
		ако (!argv_expanded[0]) {
			free(argv_expanded);
			/* `false` still has to set exitcode 1 */
			G.expand_exitcode = G.last_exitcode;
			goto only_assignments;
		}

		old_vars = NULL;
#ifndef __U_BOOT__
		sv_shadowed = G.shadowed_vars_pp;

		/* Check if argv[0] matches any functions (this goes before bltins) */
		IF_HUSH_FUNCTIONS(funcp = find_function(argv_expanded[0]);)
		IF_HUSH_FUNCTIONS(x = NULL;)
		IF_HUSH_FUNCTIONS(ако (!funcp))
			x = find_builtin(argv_expanded[0]);
		ако (x || funcp) {
			ако (x && x->b_function == builtin_exec && argv_expanded[1] == NULL) {
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

				goto clean_up_and_ret1;
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
			ако (rcode == 0) {
				ако (!funcp) {
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
#ако ENABLE_HUSH_FUNCTIONS
				иначе {
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
		} иначе
		ако (ENABLE_FEATURE_SH_NOFORK && NUM_APPLETS > 1) {
			цяло n = find_applet_by_name(argv_expanded[0]);
			ако (n < 0 || !APPLET_IS_NOFORK(n))
				goto must_fork;

			enter_var_nest_level();
			/* Collect all variables "shadowed" by helper into old_vars list */
			G.shadowed_vars_pp = &old_vars;
			rcode = redirect_and_varexp_helper(command, &squirrel, argv_expanded);
			G.shadowed_vars_pp = sv_shadowed;

			ако (rcode == 0) {
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
		} иначе
			goto must_fork;

		restore_redirects(squirrel);
 clean_up_and_ret1:
		leave_var_nest_level();
		add_vars(old_vars);

		/*
		 * Try "usleep 99999999" + ^C + "echo $?"
		 * with FEATURE_SH_NOFORK=y.
		 */
		ако (!funcp) {
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
			ако (sigismember(&G.pending_set, SIGINT))
				rcode = 128 | SIGINT;
		}
		free(argv_expanded);
		IF_HAS_KEYWORDS(ако (pi->pi_inverted) rcode = !rcode;)
		debug_leave();
		debug_printf_exec("run_pipe return %d\n", rcode);
		върни rcode;
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
	докато (cmd_no < pi->num_cmds) {
#ifndef __U_BOOT__
		struct fd_pair pipefds;
#ако !BB_MMU
		цяло sv_var_nest_level = G.var_nest_level;
		volatile nommu_save_t nommu_save;
		nommu_save.old_vars = NULL;
		nommu_save.argv = NULL;
		nommu_save.argv_from_re_execing = NULL;
#endif
#endif /* !__U_BOOT__ */
		command = &pi->cmds[cmd_no];
		cmd_no++;

#ifdef __U_BOOT__
		/* Replace argv and argc by expanded if it exists. */
		ако (argv_expanded) {
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
			ако (command->argv) {
			debug_printf_exec(": pipe member '%s' '%s'...\n",
					command->argv[0], command->argv[1]);
		} иначе {
			debug_printf_exec(": pipe member with no argv\n");
		}

#ifndef __U_BOOT__
		/* pipes are inserted between pairs of commands */
		pipefds.rd = 0;
		pipefds.wr = 1;
		ако (cmd_no < pi->num_cmds)
			xpiped_pair(pipefds);

#ако ENABLE_HUSH_LINENO_VAR
		G.execute_lineno = command->lineno;
#endif

		command->pid = BB_MMU ? fork() : vfork();
		ако (!command->pid) { /* child */
#ако ENABLE_HUSH_JOB
			disable_restore_tty_pgrp_on_exit();
			CLEAR_RANDOM_T(&G.random_gen); /* or else $RANDOM repeats in child */

			/* Every child adds itself to new process group
			 * with pgid == pid_of_first_child_in_pipe */
			ако (G.run_list_level == 1 && G_interactive_fd) {
				pid_t pgrp;
				pgrp = pi->pgrp;
				ако (pgrp < 0) /* true for 1st process only */
					pgrp = getpid();
				ако (setpgid(0, pgrp) == 0
				 && pi->followup != PIPE_BG
				 && G_saved_tty_pgrp /* we have ctty */
				) {
					/* We do it in *every* child, not just first,
					 * to avoid races */
					tcsetpgrp(G_interactive_fd, pgrp);
				}
			}
#endif
			ако (pi->alive_cmds == 0 && pi->followup == PIPE_BG) {
				/* 1st cmd in backgrounded pipe
				 * should have its stdin /dev/null'ed */
				close(0);
				ако (open(bb_dev_null, O_RDONLY))
					xopen("/", O_RDONLY);
			} иначе {
				xmove_fd(next_infd, 0);
			}
			xmove_fd(pipefds.wr, 1);
			ако (pipefds.rd > 1)
				close(pipefds.rd);
			/* Like bash, explicit redirects override pipes,
			 * and the pipe fd (fd#1) is available for dup'ing:
			 * "cmd1 2>&1 | cmd2": fd#1 is duped to fd#2, thus stderr
			 * of cmd1 goes into pipe.
			 */
			ако (setup_redirects(command, NULL)) {
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
#ако ENABLE_HUSH_FAST
		G.count_SIGCHLD++;
//bb_error_msg("[%d] fork in run_pipe: G.count_SIGCHLD:%d G.handled_SIGCHLD:%d", getpid(), G.count_SIGCHLD, G.handled_SIGCHLD);
#endif
		enable_restore_tty_pgrp_on_exit();
#ако !BB_MMU
		/* Clean up after vforked child */
		free(nommu_save.argv);
		free(nommu_save.argv_from_re_execing);
		G.var_nest_level = sv_var_nest_level;
		remove_nested_vars();
		add_vars(nommu_save.old_vars);
#endif
		free(argv_expanded);
		argv_expanded = NULL;
		ако (command->pid < 0) { /* [v]fork failed */
			/* Clearly indicate, was it fork or vfork */
			bb_simple_perror_msg(BB_MMU ? "vfork"+1 : "vfork");
		} иначе {
			pi->alive_cmds++;
#ако ENABLE_HUSH_JOB
			/* Second and next children need to know pid of first one */
			ако (pi->pgrp < 0)
				pi->pgrp = command->pid;
#endif
		}

		ако (cmd_no > 1)
			close(next_infd);
		ако (cmd_no < pi->num_cmds)
			close(pipefds.wr);
		/* Pass read (output) pipe end to next iteration */
		next_infd = pipefds.rd;
#иначе /* __U_BOOT__ */
		/* Process the command */
		rcode = cmd_process(G.do_repeat ? CMD_FLAG_REPEAT : 0,
				    command->argc, command->argv,
				    &(G.flag_repeat), NULL);

		ако (argv_expanded) {
			/*
			 * expand_strvec_to_strvec() allocates memory to expand
			 * argv, we need to free it.
			 */
			free(argv_expanded);

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
	ако (!pi->alive_cmds) {
		debug_leave();
		debug_printf_exec("run_pipe return 1 (all forks failed, no children)\n");
		върни 1;
	}
#endif /* __U_BOOT__ */

	debug_leave();
#ifndef __U_BOOT__
	debug_printf_exec("run_pipe return -1 (%u children started)\n", pi->alive_cmds);
	върни -1;
#иначе /* __U_BOOT__ */
	IF_HAS_KEYWORDS(ако (pi->pi_inverted) rcode = !rcode;)
	debug_printf_exec("run_pipe return %d\n", rcode);
	върни rcode;
#endif /* __U_BOOT__ */
}

/* NB: called by pseudo_exec, and therefore must not modify any
 * global data until exec/_exit (we can be a child after vfork!) */
static цяло run_list(struct pipe *pi)
{
#ако ENABLE_HUSH_CASE
	char *case_word = NULL;
#endif
#ако ENABLE_HUSH_LOOPS
	struct pipe *loop_top = NULL;
	char **for_lcur = NULL;
	char **for_list = NULL;
#endif
	smallint last_followup;
	smalluint rcode;
#ако ENABLE_HUSH_IF || ENABLE_HUSH_CASE
	smalluint cond_code = 0;
#иначе
	enum { cond_code = 0 };
#endif
#ако HAS_KEYWORDS
	smallint rword;      /* RES_foo */
	smallint last_rword; /* ditto */
#endif

#ifndef __U_BOOT__
	debug_printf_exec("run_list lvl %d start\n", G.run_list_level);
	debug_enter();
#endif /* !__U_BOOT__ */

	set_G_ifs();

#ако ENABLE_HUSH_LOOPS
	/* Check syntax for "for" */
	{
		struct pipe *cpipe;
		за (cpipe = pi; cpipe; cpipe = cpipe->next) {
			ако (cpipe->res_word != RES_FOR && cpipe->res_word != RES_IN)
				continue;
			/* current word is FOR or IN (BOLD in comments below) */
			ако (cpipe->next == NULL) {
				syntax_error("malformed for");
				debug_leave();
				debug_printf_exec("run_list lvl %d return 1\n", G.run_list_level);
				върни 1;
			}
			/* "FOR v; do ..." and "for v IN a b; do..." are ok */
			ако (cpipe->next->res_word == RES_DO)
				continue;
			/* next word is not "do". It must be "in" then ("FOR v in ...") */
			ако (cpipe->res_word == RES_IN /* "for v IN a b; not_do..."? */
			 || cpipe->next->res_word != RES_IN /* FOR v not_do_and_not_in..."? */
			) {
				syntax_error("malformed for");
				debug_leave();
				debug_printf_exec("run_list lvl %d return 1\n", G.run_list_level);
				върни 1;
			}
		}
	}
#endif

	/* Past this point, all code paths should jump to ret: label
	 * in order to return, no direct "return" statements please.
	 * This helps to ensure that no memory is leaked. */

#ако ENABLE_HUSH_JOB
	G.run_list_level++;
#endif

#ако HAS_KEYWORDS
	rword = RES_NONE;
	last_rword = RES_XXXX;
#endif
	last_followup = PIPE_SEQ;
	rcode = G.last_exitcode;

	/* Go through list of pipes, (maybe) executing them. */
#ifndef __U_BOOT__
	за (; pi; pi = IF_HUSH_LOOPS(rword == RES_DONE ? loop_top : ) pi->next) {
#иначе /* __U_BOOT__ */
	за (; pi; pi = rword == RES_DONE ? loop_top : pi->next) {
#endif /* __U_BOOT__ */
		цяло r;
		цяло sv_errexit_depth;

#ifndef __U_BOOT__
		ако (G.flag_SIGINT)
			break;
		ако (G_flag_return_in_progress == 1)
			break;
#endif /* !__U_BOOT__ */

		IF_HAS_KEYWORDS(rword = pi->res_word;)
		debug_printf_exec(": rword:%d cond_code:%d last_rword:%d\n",
				rword, cond_code, last_rword);

		sv_errexit_depth = G.errexit_depth;
		ако (
#ако ENABLE_HUSH_IF
		    rword == RES_IF || rword == RES_ELIF ||
#endif
		    pi->followup != PIPE_SEQ
		) {
			G.errexit_depth++;
		}
#ако ENABLE_HUSH_LOOPS
		ако ((rword == RES_WHILE || rword == RES_UNTIL || rword == RES_FOR)
		 && loop_top == NULL /* avoid bumping G.depth_of_loop twice */
		) {
			/* start of a loop: remember where loop starts */
			loop_top = pi;
			G.depth_of_loop++;
		}
#endif
		/* Still in the same "if...", "then..." or "do..." branch? */
		ако (IF_HAS_KEYWORDS(rword == last_rword &&) 1) {
			ако ((rcode == 0 && last_followup == PIPE_OR)
			 || (rcode != 0 && last_followup == PIPE_AND)
			) {
				/* It is "<true> || CMD" or "<false> && CMD"
				 * and we should not execute CMD */
				debug_printf_exec("skipped cmd because of || or &&\n");
				last_followup = pi->followup;
				goto dont_check_jobs_but_continue;
			}
		}
		last_followup = pi->followup;
#ако ENABLE_HUSH_IF
		ако (cond_code != 0) {
			ако (rword == RES_THEN) {
				/* if false; then ... fi has exitcode 0! */
				G.last_exitcode = rcode = EXIT_SUCCESS;
				/* "if <false> THEN cmd": skip cmd */
				debug_printf_exec("skipped THEN cmd because IF condition was false\n");
				last_rword = rword;
				continue;
			}
		} иначе {
			ако (rword == RES_ELSE
			 || (rword == RES_ELIF && last_rword != RES_ELIF)
			) {
				/* "if <true> then ... ELSE/ELIF cmd":
				 * skip cmd and all following ones */
				debug_printf_exec("skipped ELSE/ELIF branch because IF condition was true\n");
				break;
			}
			//if (rword == RES_THEN): "if <true> THEN cmd", run cmd (fall through)
		}
#endif
		IF_HAS_KEYWORDS(last_rword = rword;)
#ако ENABLE_HUSH_LOOPS
		ако (rword == RES_FOR) { /* && pi->num_cmds - always == 1 */
			ако (!for_lcur) {
				/* first loop through for */

				static const char encoded_dollar_at[] ALIGN1 = {
					SPECIAL_VAR_SYMBOL, '@' | 0x80, SPECIAL_VAR_SYMBOL, '\0'
				}; /* encoded representation of "$@" */
				static const char *const encoded_dollar_at_argv[] ALIGN_PTR = {
					encoded_dollar_at, NULL
				}; /* argv list with one element: "$@" */
				char **vals;

				G.last_exitcode = rcode = EXIT_SUCCESS;
				vals = (char**)encoded_dollar_at_argv;
				ако (pi->next->res_word == RES_IN) {
					/* if no variable values after "in" we skip "for" */
					ако (!pi->next->cmds[0].argv) {
						debug_printf_exec(": null FOR: exitcode EXIT_SUCCESS\n");
						break;
					}
					vals = pi->next->cmds[0].argv;
				} /* else: "for var; do..." -> assume "$@" list */
				/* create list of variable values */
				debug_print_strings("for_list made from", vals);
				for_list = expand_strvec_to_strvec(vals);
				for_lcur = for_list;
				debug_print_strings("for_list", for_list);
			}
			ако (!*for_lcur) {
				/* "for" loop is over, clean up */
				free(for_list);
				for_list = NULL;
				for_lcur = NULL;
				break;
			}
			/* Insert next value from for_lcur */
			/* note: *for_lcur already has quotes removed, $var expanded, etc */
#ifndef __U_BOOT__
			set_local_var_from_halves(pi->cmds[0].argv[0], *for_lcur++);
#иначе /* __U_BOOT__ */
			/* We cannot use xasprintf, so we emulate it. */
			char *full_var;
			char *var = pi->cmds[0].argv[0];
			char *val = *for_lcur++;

			/* + 1 to take into account =. */
			full_var = xmalloc(strlen(var) + strlen(val) + 1);
			sprintf(full_var, "%s=%s", var, val);

			set_local_var_modern(full_var, /*flag:*/ 0);
#endif /* __U_BOOT__ */
			continue;
		}
		ако (rword == RES_IN) {
			continue; /* "for v IN list;..." - "in" has no cmds anyway */
		}
		ако (rword == RES_DONE) {
			continue; /* "done" has no cmds too */
		}
#endif
#ако ENABLE_HUSH_CASE
		ако (rword == RES_CASE) {
			debug_printf_exec("CASE cond_code:%d\n", cond_code);
			case_word = expand_string_to_string(pi->cmds->argv[0],
				EXP_FLAG_ESC_GLOB_CHARS, /*unbackslash:*/ 1);
			debug_printf_exec("CASE word1:'%s'\n", case_word);
			//unbackslash(case_word);
			//debug_printf_exec("CASE word2:'%s'\n", case_word);
			continue;
		}
		ако (rword == RES_MATCH) {
			char **argv;

			debug_printf_exec("MATCH cond_code:%d\n", cond_code);
			ако (!case_word) /* "case ... matched_word) ... WORD)": we executed selected branch, stop */
				break;
			/* all prev words didn't match, does this one match? */
			argv = pi->cmds->argv;
			докато (*argv) {
				char *pattern;
				debug_printf_exec("expand_string_to_string('%s')\n", *argv);
				pattern = expand_string_to_string(*argv,
						EXP_FLAG_ESC_GLOB_CHARS,
						/*unbackslash:*/ 0
				);
				/* TODO: which FNM_xxx flags to use? */
				cond_code = (fnmatch(pattern, case_word, /*flags:*/ 0) != 0);
				debug_printf_exec("cond_code=fnmatch(pattern:'%s',str:'%s'):%d\n",
						pattern, case_word, cond_code);
				free(pattern);
				ако (cond_code == 0) {
					/* match! we will execute this branch */
					free(case_word);
					case_word = NULL; /* make future "word)" stop */
					break;
				}
				argv++;
			}
			continue;
		}
		ако (rword == RES_CASE_BODY) { /* inside of a case branch */
			debug_printf_exec("CASE_BODY cond_code:%d\n", cond_code);
			ако (cond_code != 0)
				continue; /* not matched yet, skip this pipe */
		}
		ако (rword == RES_ESAC) {
			debug_printf_exec("ESAC cond_code:%d\n", cond_code);
			ако (case_word) {
				/* "case" did not match anything: still set $? (to 0) */
				G.last_exitcode = rcode = EXIT_SUCCESS;
			}
		}
#endif
		/* Just pressing <enter> in shell should check for jobs.
		 * OTOH, in non-interactive shell this is useless
		 * and only leads to extra job checks */
		ако (pi->num_cmds == 0) {
#ifndef __U_BOOT__
			ако (G_interactive_fd)
				goto check_jobs_and_continue;
#endif /* !__U_BOOT__ */
			continue;
		}

		/* After analyzing all keywords and conditions, we decided
		 * to execute this pipe. NB: have to do checkjobs(NULL)
		 * after run_pipe to collect any background children,
		 * even if list execution is to be stopped. */
		debug_printf_exec(": run_pipe with %d members\n", pi->num_cmds);
#ifndef __U_BOOT__
#ако ENABLE_HUSH_LOOPS
		G.flag_break_continue = 0;
#endif
#endif /* !__U_BOOT__ */
#ifndef __U_BOOT__
		rcode = r = G.o_opt[OPT_O_NOEXEC] ? 0 : run_pipe(pi);
		/* NB: rcode is a smalluint, r is int */
#иначе /* __U_BOOT__ */
		rcode = r = run_pipe(pi); /* NB: rcode is a smalluint, r is int */
		ако (r <= EXIT_RET_CODE) {
			цяло previous_rcode = G.last_exitcode;
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
			ако (previous_rcode != r) {
				/*
				 * If run from run_command, run_command_flags will be set, so we check
				 * this to know if we are in main input shell.
				 */
				ако (!G.run_command_flags)
					printf("exit not allowed from main input shell.\n");

				break;
			}
			continue;
		}
#endif /* __U_BOOT__ */
		ако (r != -1) {
			/* We ran a builtin, function, or group.
			 * rcode is already known
			 * and we don't need to wait for anything. */
			debug_printf_exec(": builtin/func exitcode %d\n", rcode);
			G.last_exitcode = rcode;
#ifndef __U_BOOT__
			check_and_run_traps();
#endif /* !__U_BOOT__ */
#ако ENABLE_HUSH_TRAP && ENABLE_HUSH_FUNCTIONS
			rcode = G.last_exitcode; /* "return" in trap can change it, read back */
#endif
#ifndef __U_BOOT__
#ако ENABLE_HUSH_LOOPS
			/* Was it "break" or "continue"? */
			ако (G.flag_break_continue) {
				smallint fbc = G.flag_break_continue;
				/* We might fall into outer *loop*,
				 * don't want to break it too */
				ако (loop_top) {
					G.depth_break_continue--;
					ако (G.depth_break_continue == 0)
						G.flag_break_continue = 0;
					/* else: e.g. "continue 2" should *break* once, *then* continue */
				} /* else: "while... do... { we are here (innermost list is not a loop!) };...done" */
				ако (G.depth_break_continue != 0 || fbc == BC_BREAK) {
					checkjobs(NULL, 0 /*(no pid to wait for)*/);
					break;
				}
				/* "continue": simulate end of loop */
				rword = RES_DONE;
				continue;
			}
#endif
			ако (G_flag_return_in_progress == 1) {
				checkjobs(NULL, 0 /*(no pid to wait for)*/);
				break;
			}

		} иначе ако (pi->followup == PIPE_BG) {
			/* What does bash do with attempts to background builtins? */
			/* even bash 3.2 doesn't do that well with nested bg:
			 * try "{ { sleep 10; echo DEEP; } & echo HERE; } &".
			 * I'm NOT treating inner &'s as jobs */
#ако ENABLE_HUSH_JOB
			ако (G.run_list_level == 1)
				insert_job_into_table(pi);
#endif
			/* Last command's pid goes to $! */
			G.last_bg_pid = pi->cmds[pi->num_cmds - 1].pid;
			G.last_bg_pid_exitcode = 0;
			debug_printf_exec(": cmd&: exitcode EXIT_SUCCESS\n");
/* Check pi->pi_inverted? "! sleep 1 & echo $?": bash says 1. dash and ash say 0 */
			rcode = EXIT_SUCCESS;
			goto check_traps;
		} иначе {
#ако ENABLE_HUSH_JOB
			ако (G.run_list_level == 1 && G_interactive_fd) {
				/* Waits for completion, then fg's main shell */
				rcode = checkjobs_and_fg_shell(pi);
				debug_printf_exec(": checkjobs_and_fg_shell exitcode %d\n", rcode);
				goto check_traps;
			}
#endif
			/* This one just waits for completion */
			rcode = checkjobs(pi, 0 /*(no pid to wait for)*/);
			debug_printf_exec(": checkjobs exitcode %d\n", rcode);
 check_traps:
			G.last_exitcode = rcode;
			check_and_run_traps();
#ако ENABLE_HUSH_TRAP && ENABLE_HUSH_FUNCTIONS
			rcode = G.last_exitcode; /* "return" in trap can change it, read back */
#endif
		}
#endif /* !__U_BOOT__ */

#ifndef __U_BOOT__
		/* Handle "set -e" */
		ако (rcode != 0 && G.o_opt[OPT_O_ERREXIT]) {
			debug_printf_exec("ERREXIT:1 errexit_depth:%d\n", G.errexit_depth);
			ако (G.errexit_depth == 0)
				hush_exit(rcode);
		}
#иначе /* __U_BOOT__ */
		} /* if (r != -1) */
#endif /* __U_BOOT__ */
		G.errexit_depth = sv_errexit_depth;

		/* Analyze how result affects subsequent commands */
#ако ENABLE_HUSH_IF
		ако (rword == RES_IF || rword == RES_ELIF) {
			debug_printf_exec("cond_code=rcode:%d\n", rcode);
			cond_code = rcode;
		}
#endif
#ifndef __U_BOOT__
 check_jobs_and_continue:
		checkjobs(NULL, 0 /*(no pid to wait for)*/);
#endif /* !__U_BOOT__ */
 dont_check_jobs_but_continue: ;
#ако ENABLE_HUSH_LOOPS
		/* Beware of "while false; true; do ..."! */
		ако (pi->next
		 && (pi->next->res_word == RES_DO || pi->next->res_word == RES_DONE)
		 /* check for RES_DONE is needed for "while ...; do \n done" case */
		) {
			ако (rword == RES_WHILE) {
				ако (rcode) {
					/* "while false; do...done" - exitcode 0 */
					G.last_exitcode = rcode = EXIT_SUCCESS;
					debug_printf_exec(": while expr is false: breaking (exitcode:EXIT_SUCCESS)\n");
					break;
				}
			}
			ако (rword == RES_UNTIL) {
				ако (!rcode) {
					debug_printf_exec(": until expr is true: breaking\n");
					break;
				}
			}
		}
#endif
	} /* for (pi) */

#ако ENABLE_HUSH_JOB
	G.run_list_level--;
#endif
#ако ENABLE_HUSH_LOOPS
	ако (loop_top)
		G.depth_of_loop--;
	free(for_list);
#endif
#ако ENABLE_HUSH_CASE
	free(case_word);
#endif
#ifndef __U_BOOT__
	debug_leave();
	debug_printf_exec("run_list lvl %d return %d\n", G.run_list_level, rcode);
#endif /* !__U_BOOT__ */
	върни rcode;
}

/* Select which version we will use */
static цяло run_and_free_list(struct pipe *pi)
{
	цяло rcode = 0;
	debug_printf_exec("run_and_free_list entered\n");
#ifndef __U_BOOT__
	ако (!G.o_opt[OPT_O_NOEXEC]) {
#endif /* !__U_BOOT__ */
		debug_printf_exec(": run_list: 1st pipe with %d cmds\n", pi->num_cmds);
		rcode = run_list(pi);
#ifndef __U_BOOT__
	}
#endif /* !__U_BOOT__ */
	/* free_pipe_list has the side effect of clearing memory.
	 * In the long run that function can be merged with run_list,
	 * but doing that now would hobble the debugging effort. */
	free_pipe_list(pi);
	debug_printf_exec("run_and_free_list return %d\n", rcode);
	върни rcode;
}

#ifndef __U_BOOT__
static void install_sighandlers(unsigned mask)
{
	sighandler_t old_handler;
	unsigned sig = 0;
	докато ((mask >>= 1) != 0) {
		sig++;
		ако (!(mask & 1))
			continue;
		old_handler = install_sighandler(sig, pick_sighandler(sig));
		/* POSIX allows shell to re-enable SIGCHLD
		 * even if it was SIG_IGN on entry.
		 * Therefore we skip IGN check for it:
		 */
		ако (sig == SIGCHLD)
			continue;
		/* Interactive bash re-enables SIGHUP which is SIG_IGNed on entry.
		 * Try:
		 * trap '' hup; bash; echo RET  # type "kill -hup $$", see SIGHUP having effect
		 * trap '' hup; bash -c 'kill -hup $$; echo ALIVE'  # here SIGHUP is SIG_IGNed
		 */
		ако (sig == SIGHUP && G_interactive_fd)
			continue;
		/* Unless one of the above signals, is it SIG_IGN? */
		ако (old_handler == SIG_IGN) {
			/* oops... restore back to IGN, and record this fact */
			install_sighandler(sig, old_handler);
#ако ENABLE_HUSH_TRAP
			ако (!G_traps)
				G_traps = xzalloc(sizeof(G_traps[0]) * NSIG);
			free(G_traps[sig]);
			G_traps[sig] = xzalloc(1); /* == xstrdup(""); */
#endif
		}
	}
}

/* Called a few times only (or even once if "sh -c") */
static void install_special_sighandlers(void)
{
	unsigned mask;

	/* Which signals are shell-special? */
	mask = (1 << SIGQUIT) | (1 << SIGCHLD);
	ако (G_interactive_fd) {
		mask |= SPECIAL_INTERACTIVE_SIGS;
		ако (G_saved_tty_pgrp) /* we have ctty, job control sigs work */
			mask |= SPECIAL_JOBSTOP_SIGS;
	}
	/* Careful, do not re-install handlers we already installed */
	ако (G.special_sig_mask != mask) {
		unsigned diff = mask & ~G.special_sig_mask;
		G.special_sig_mask = mask;
		install_sighandlers(diff);
	}
}

#ако ENABLE_HUSH_JOB
/* helper */
/* Set handlers to restore tty pgrp and exit */
static void install_fatal_sighandlers(void)
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

static цяло set_mode(цяло state, char mode, const char *o_opt)
{
	цяло idx;
	switch (mode) {
	case 'n':
		/* set -n has no effect in interactive shell */
		/* Try: while set -n; do echo $-; done */
		ако (!G_interactive_fd)
			G.o_opt[OPT_O_NOEXEC] = state;
		break;
	case 'x':
		IF_HUSH_MODE_X(G_x_mode = state;)
		IF_HUSH_MODE_X(ако (G.x_mode_fd <= 0) G.x_mode_fd = dup_CLOEXEC(2, 10);)
		break;
	case 'e':
		G.o_opt[OPT_O_ERREXIT] = state;
		break;
	case 'o':
		ако (!o_opt) {
			/* "set -o" or "set +o" without parameter.
			 * in bash, set -o produces this output:
			 *  pipefail        off
			 * and set +o:
			 *  set +o pipefail
			 * We always use the second form.
			 */
			const char *p = o_opt_strings;
			idx = 0;
			докато (*p) {
				printf("set %co %s\n", (G.o_opt[idx] ? '-' : '+'), p);
				idx++;
				p += strlen(p) + 1;
			}
			break;
		}
		idx = index_in_strings(o_opt_strings, o_opt);
		ако (idx >= 0) {
			G.o_opt[idx] = state;
			break;
		}
		/* fall through to error */
	default:
		върни EXIT_FAILURE;
	}
	върни EXIT_SUCCESS;
}

цяло hush_main(цяло argc, char **argv) MAIN_EXTERNALLY_VISIBLE;
цяло hush_main(цяло argc, char **argv)
{
	pid_t cached_getpid;
	enum {
		OPT_login = (1 << 0),
	};
	unsigned flags;
#ако !BB_MMU
	unsigned builtin_argc = 0;
#endif
	char **e;
	struct variable *cur_var;
	struct variable *shell_ver;

	INIT_G();
	ако (EXIT_SUCCESS != 0) /* if EXIT_SUCCESS == 0, it is already done */
		G.last_exitcode = EXIT_SUCCESS;
#ако !BB_MMU
	/* "Big heredoc" support via "sh -< STRING" invocation.
	 * Check it first (do not bother to run the usual init code,
	 * it is not needed for this case).
	 */
	ако (argv[1]
	 && argv[1][0] == '-' && argv[1][1] == '<' /*&& !argv[1][2]*/
	 /*&& argv[2] && !argv[3] - we don't check some conditions */
	) {
		full_write1_str(argv[2]);
		_exit(0);
	}
	G.argv0_for_re_execing = argv[0];
#endif
#ако ENABLE_HUSH_TRAP
# ако ENABLE_HUSH_FUNCTIONS
	G.return_exitcode = -1;
# endif
	G.pre_trap_exitcode = -1;
#endif

#ако ENABLE_HUSH_FAST
	G.count_SIGCHLD++; /* ensure it is != G.handled_SIGCHLD */
#endif

	cached_getpid = getpid();   /* for tcsetpgrp() during init */
	G.root_pid = cached_getpid; /* for $PID  (NOMMU can override via -$HEXPID:HEXPPID:...) */
	G.root_ppid = getppid();    /* for $PPID (NOMMU can override) */

	/* Deal with HUSH_VERSION */
	debug_printf_env("unsetenv '%s'\n", "HUSH_VERSION");
	unsetenv("HUSH_VERSION"); /* in case it exists in initial env */
	shell_ver = xzalloc(sizeof(*shell_ver));
	shell_ver->flg_export = 1;
	shell_ver->flg_read_only = 1;
	/* Code which handles ${var<op>...} needs writable values for all variables,
	 * therefore we xstrdup: */
	shell_ver->varstr = xstrdup(hush_version_str);

	/* Create shell local variables from the values
	 * currently living in the environment */
	G.top_var = shell_ver;
	cur_var = G.top_var;
	e = environ;
	ако (e) докато (*e) {
		char *value = strchr(*e, '=');
		ако (value) { /* paranoia */
			cur_var->next = xzalloc(sizeof(*cur_var));
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

#ако BASH_HOSTNAME_VAR
	/* Set (but not export) HOSTNAME unless already set */
	ако (!get_local_var_value("HOSTNAME")) {
		struct utsname uts;
		uname(&uts);
		set_local_var_from_halves("HOSTNAME", uts.nodename);
	}
#endif
	/* IFS is not inherited from the parent environment */
	set_local_var_from_halves("IFS", defifs);

	ако (!get_local_var_value("PATH"))
		set_local_var_from_halves("PATH", bb_default_root_path);

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

#ако NUM_SCRIPTS > 0
	ако (argc < 0) {
		char *script = get_script_content(-argc - 1);
		G.global_argv = argv;
		G.global_argc = string_array_len(argv);
		//install_special_sighandlers(); - needed?
		parse_and_run_string(script);
		goto final_return;
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
	докато (1) {
		цяло opt = getopt(argc, argv, "+" /* stop at 1st non-option */
				"cexinsl"
#ако !BB_MMU
				"$:R:V:"
# ако ENABLE_HUSH_LINENO_VAR
				"L:"
# endif
# ако ENABLE_HUSH_FUNCTIONS
				"F:"
# endif
#endif
		);
		ако (opt <= 0)
			break;
		switch (opt) {
		case 'c':
			/* Note: -c is not an option with param!
			 * "hush -c -l SCRIPT" is valid. "hush -cSCRIPT" is not.
			 */
			G.opt_c = 1;
			break;
		case 'i':
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
			break;
		case 's':
			G.opt_s = 1;
			break;
		case 'l':
			flags |= OPT_login;
			break;
#ако !BB_MMU
		case '$': {
			unsigned long long empty_trap_mask;

			G.root_pid = bb_strtou(optarg, &optarg, 16);
			optarg++;
			G.root_ppid = bb_strtou(optarg, &optarg, 16);
			optarg++;
			G.last_bg_pid = bb_strtou(optarg, &optarg, 16);
			optarg++;
			G.last_exitcode = bb_strtou(optarg, &optarg, 16);
			optarg++;
			builtin_argc = bb_strtou(optarg, &optarg, 16);
			optarg++;
			empty_trap_mask = bb_strtoull(optarg, &optarg, 16);
			ако (empty_trap_mask != 0) {
				IF_HUSH_TRAP(цяло sig;)
				install_special_sighandlers();
# ако ENABLE_HUSH_TRAP
				G_traps = xzalloc(sizeof(G_traps[0]) * NSIG);
				за (sig = 1; sig < NSIG; sig++) {
					ако (empty_trap_mask & (1LL << sig)) {
						G_traps[sig] = xzalloc(1); /* == xstrdup(""); */
						install_sighandler(sig, SIG_IGN);
					}
				}
# endif
			}
# ако ENABLE_HUSH_LOOPS
			optarg++;
			G.depth_of_loop = bb_strtou(optarg, &optarg, 16);
# endif
			/* Suppress "killed by signal" message, -$ hack is used
			 * for subshells: echo `sh -c 'kill -9 $$'`
			 * should be silent.
			 */
			IF_HUSH_JOB(G.run_list_level = 1;)
# ако ENABLE_HUSH_FUNCTIONS
			/* nommu uses re-exec trick for "... | func | ...",
			 * should allow "return".
			 * This accidentally allows returns in subshells.
			 */
			G_flag_return_in_progress = -1;
# endif
			break;
		}
		case 'R':
		case 'V':
			set_local_var(xstrdup(optarg), opt == 'R' ? SETFLAG_MAKE_RO : 0);
			break;
# ако ENABLE_HUSH_LINENO_VAR
		case 'L':
			G.parse_lineno = xatou(optarg);
			break;
# endif
# ако ENABLE_HUSH_FUNCTIONS
		case 'F': {
			struct function *funcp = new_function(optarg);
			/* funcp->name is already set to optarg */
			/* funcp->body is set to NULL. It's a special case. */
			funcp->body_as_string = argv[optind];
			optind++;
			break;
		}
# endif
#endif
		/*case '?': invalid option encountered (set_mode('?') will fail) */
		/*case 'n':*/
		/*case 'x':*/
		/*case 'e':*/
		default:
			ако (set_mode(1, opt, NULL) == 0) /* no error */
				break;
			bb_show_usage();
		}
	} /* option parsing loop */

	/* Skip options. Try "hush -l": $1 should not be "-l"! */
	G.global_argc = argc - (optind - 1);
	G.global_argv = argv + (optind - 1);
	G.global_argv[0] = argv[0];

	/* If we are login shell... */
	ако (flags & OPT_login) {
		const char *hp = NULL;
		HFILE *input;

		debug_printf("sourcing /etc/profile\n");
		input = hfopen("/etc/profile");
 run_profile:
		ако (input != NULL) {
			install_special_sighandlers();
			parse_and_run_file(input);
			hfclose(input);
		}
		/* bash: after sourcing /etc/profile,
		 * tries to source (in the given order):
		 * ~/.bash_profile, ~/.bash_login, ~/.profile,
		 * stopping on first found. --noprofile turns this off.
		 * bash also sources ~/.bash_logout on exit.
		 * If called as sh, skips .bash_XXX files.
		 */
		ако (!hp) { /* unless we looped on the "goto" already */
			hp = get_local_var_value("HOME");
			ако (hp && hp[0]) {
				debug_printf("sourcing ~/.profile\n");
				hp = concat_path_file(hp, ".profile");
				input = hfopen(hp);
				free((char*)hp);
				goto run_profile;
			}
		}
	}

#ifndef __U_BOOT__
	/* -c takes effect *after* -l */
	ако (G.opt_c) {
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
		char *script;

		install_special_sighandlers();

		G.global_argc--;
		G.global_argv++;
#ако !BB_MMU
		ако (builtin_argc) {
			/* -c 'builtin' [BARGV...] "" ARG0 [ARG1...] */
			const struct built_in_command *x;
			x = find_builtin(G.global_argv[0]);
			ако (x) { /* paranoia */
				argv = G.global_argv;
				G.global_argc -= builtin_argc + 1; /* skip [BARGV...] "" */
				G.global_argv += builtin_argc + 1;
				G.global_argv[-1] = NULL; /* replace "" */
				G.last_exitcode = x->b_function(argv);
			}
			goto final_return;
		}
#endif

		script = G.global_argv[0];
		ако (!script)
			bb_error_msg_and_die(bb_msg_requires_arg, "-c");
		ако (!G.global_argv[1]) {
			/* -c 'script' (no params): prevent empty $0 */
			G.global_argv[0] = argv[0];
		} иначе { /* else -c 'script' ARG0 [ARG1...]: $0 is ARG0 */
			G.global_argc--;
			G.global_argv++;
		}
		parse_and_run_string(script);
		goto final_return;
	}

	/* -s is: hush -s ARGV1 ARGV2 (no SCRIPT) */
	ако (!G.opt_s && G.global_argv[1]) {
		HFILE *input;
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
		input = hfopen(G.global_argv[0]);
		ако (!input) {
			bb_simple_perror_msg_and_die(G.global_argv[0]);
		}
		xfunc_error_retval = 1;
		install_special_sighandlers();
		parse_and_run_file(input);
#ако ENABLE_FEATURE_CLEAN_UP
		hfclose(input);
#endif
		goto final_return;
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
#ако ENABLE_HUSH_JOB
	ако (isatty(STDIN_FILENO) && isatty(STDOUT_FILENO)) {
		G_saved_tty_pgrp = tcgetpgrp(STDIN_FILENO);
		debug_printf("saved_tty_pgrp:%d\n", G_saved_tty_pgrp);
		ако (G_saved_tty_pgrp < 0)
			G_saved_tty_pgrp = 0;

		/* try to dup stdin to high fd#, >= 255 */
		G_interactive_fd = dup_CLOEXEC(STDIN_FILENO, 254);
		ако (G_interactive_fd < 0) {
			/* try to dup to any fd */
			G_interactive_fd = dup_CLOEXEC(STDIN_FILENO, -1);
			ако (G_interactive_fd < 0) {
				/* give up */
				G_interactive_fd = 0;
				G_saved_tty_pgrp = 0;
			}
		}
	}
	debug_printf("interactive_fd:%d\n", G_interactive_fd);
	ако (G_interactive_fd) {
		ако (G_saved_tty_pgrp) {
			/* If we were run as 'hush &', sleep until we are
			 * in the foreground (tty pgrp == our pgrp).
			 * If we get started under a job aware app (like bash),
			 * make sure we are now in charge so we don't fight over
			 * who gets the foreground */
			докато (1) {
				pid_t shell_pgrp = getpgrp();
				G_saved_tty_pgrp = tcgetpgrp(G_interactive_fd);
				ако (G_saved_tty_pgrp == shell_pgrp)
					break;
				/* send TTIN to ourself (should stop us) */
				kill(- shell_pgrp, SIGTTIN);
			}
		}

		/* Install more signal handlers */
		install_special_sighandlers();

		ако (G_saved_tty_pgrp) {
			/* Set other signals to restore saved_tty_pgrp */
			install_fatal_sighandlers();
			/* Put ourselves in our own process group
			 * (bash, too, does this only if ctty is available) */
			bb_setpgrp(); /* is the same as setpgid(our_pid, our_pid); */
			/* Grab control of the terminal */
			tcsetpgrp(G_interactive_fd, cached_getpid);
		}
		enable_restore_tty_pgrp_on_exit();

# ако ENABLE_FEATURE_EDITING
		G.line_input_state = new_line_input_t(FOR_SHELL);
#  ако ENABLE_FEATURE_TAB_COMPLETION
		G.line_input_state->get_exe_name = hush_command_name;
#  endif
#  ако EDITING_HAS_sh_get_var
		G.line_input_state->sh_get_var = get_local_var_value;
#  endif
# endif
# ако ENABLE_HUSH_SAVEHISTORY && MAX_HISTORY > 0
		{
			const char *hp = get_local_var_value("HISTFILE");
			ако (!hp) {
				hp = get_local_var_value("HOME");
				ако (hp)
					hp = concat_path_file(hp, ".hush_history");
			} иначе {
				hp = xstrdup(hp);
			}
			ако (hp) {
				G.line_input_state->hist_file = hp;
				//set_local_var(xasprintf("HISTFILE=%s", ...));
			}
#  ако ENABLE_FEATURE_SH_HISTFILESIZE
			hp = get_local_var_value("HISTFILESIZE");
			G.line_input_state->max_history = size_from_HISTFILESIZE(hp);
#  endif
		}
# endif
	} иначе {
		install_special_sighandlers();
	}
#elif ENABLE_HUSH_INTERACTIVE
	/* No job control compiled in, only prompt/line editing */
	ако (isatty(STDIN_FILENO) && isatty(STDOUT_FILENO)) {
		G_interactive_fd = dup_CLOEXEC(STDIN_FILENO, 254);
		ако (G_interactive_fd < 0) {
			/* try to dup to any fd */
			G_interactive_fd = dup_CLOEXEC(STDIN_FILENO, -1);
			ако (G_interactive_fd < 0)
				/* give up */
				G_interactive_fd = 0;
		}
	}
	install_special_sighandlers();
#иначе
	/* We have interactiveness code disabled */
	install_special_sighandlers();
#endif
	/* bash:
	 * if interactive but not a login shell, sources ~/.bashrc
	 * (--norc turns this off, --rcfile <file> overrides)
	 */

	ако (G_interactive_fd) {
#ако ENABLE_HUSH_INTERACTIVE && ENABLE_FEATURE_EDITING_FANCY_PROMPT
		/* Set (but not export) PS1/2 unless already set */
		ако (!get_local_var_value("PS1"))
			set_local_var_from_halves("PS1", "\\w \\$ ");
		ако (!get_local_var_value("PS2"))
			set_local_var_from_halves("PS2", "> ");
#endif
		ако (!ENABLE_FEATURE_SH_EXTRA_QUIET) {
			/* note: ash and hush share this string */
			printf("\n\n%s %s\n"
				IF_HUSH_HELP("Enter 'help' for a list of built-in commands.\n")
				"\n",
				bb_banner,
				"hush - the humble shell"
			);
		}
	}

	parse_and_run_file(hfopen(NULL)); /* stdin */

 final_return:
	hush_exit(G.last_exitcode);
}

/*
 * Built-ins
 */
static цяло FAST_FUNC builtin_true(char **argv UNUSED_PARAM)
{
	върни 0;
}

static цяло FAST_FUNC builtin_false(char **argv UNUSED_PARAM)
{
	върни 1;
}

#ако ENABLE_HUSH_TEST || ENABLE_HUSH_ECHO || ENABLE_HUSH_PRINTF || ENABLE_HUSH_KILL
static NOINLINE цяло run_applet_main(char **argv, цяло (*applet_main_func)(цяло argc, char **argv))
{
	цяло argc = string_array_len(argv);
	върни applet_main_func(argc, argv);
}
#endif
#ако ENABLE_HUSH_TEST || BASH_TEST2
static цяло FAST_FUNC builtin_test(char **argv)
{
	върни run_applet_main(argv, test_main);
}
#endif
#ако ENABLE_HUSH_ECHO
static цяло FAST_FUNC builtin_echo(char **argv)
{
	върни run_applet_main(argv, echo_main);
}
#endif
#ако ENABLE_HUSH_PRINTF
static цяло FAST_FUNC builtin_printf(char **argv)
{
	върни run_applet_main(argv, printf_main);
}
#endif

#ако ENABLE_HUSH_HELP
static цяло FAST_FUNC builtin_help(char **argv UNUSED_PARAM)
{
	const struct built_in_command *x;

	printf(
		"Built-in commands:\n"
		"------------------\n");
	за (x = bltins1; x != &bltins1[ARRAY_SIZE(bltins1)]; x++) {
		ако (x->b_descr)
			printf("%-10s%s\n", x->b_cmd, x->b_descr);
	}
	върни EXIT_SUCCESS;
}
#endif

#ако MAX_HISTORY && ENABLE_FEATURE_EDITING
static цяло FAST_FUNC builtin_history(char **argv UNUSED_PARAM)
{
	show_history(G.line_input_state);
	върни EXIT_SUCCESS;
}
#endif

static цяло FAST_FUNC builtin_cd(char **argv)
{
	const char *newdir;

	argv = skip_dash_dash(argv);
	newdir = argv[0];
	ако (newdir == NULL) {
		/* bash does nothing (exitcode 0) if HOME is ""; if it's unset,
		 * bash says "bash: cd: HOME not set" and does nothing
		 * (exitcode 1)
		 */
		const char *home = get_local_var_value("HOME");
		newdir = home ? home : "/";
	}
	ако (chdir(newdir)) {
		/* Mimic bash message exactly */
		bb_perror_msg("cd: %s", newdir);
		върни EXIT_FAILURE;
	}
	/* Read current dir (get_cwd(1) is inside) and set PWD.
	 * Note: do not enforce exporting. If PWD was unset or unexported,
	 * set it again, but do not export. bash does the same.
	 */
	set_pwd_var(/*flag:*/ 0);
	върни EXIT_SUCCESS;
}

static цяло FAST_FUNC builtin_pwd(char **argv UNUSED_PARAM)
{
	puts(get_cwd(0));
	върни EXIT_SUCCESS;
}

static цяло FAST_FUNC builtin_eval(char **argv)
{
	argv = skip_dash_dash(argv);

	ако (!argv[0])
		върни EXIT_SUCCESS;

	IF_HUSH_MODE_X(G.x_mode_depth++;)
	//bb_error_msg("%s: ++x_mode_depth=%d", __func__, G.x_mode_depth);
	ако (!argv[1]) {
		/* bash:
		 * eval "echo Hi; done" ("done" is syntax error):
		 * "echo Hi" will not execute too.
		 */
		parse_and_run_string(argv[0]);
	} иначе {
		/* "The eval utility shall construct a command by
		 * concatenating arguments together, separating
		 * each with a <space> character."
		 */
		char *str, *p;
		unsigned len = 0;
		char **pp = argv;
		do
			len += strlen(*pp) + 1;
		докато (*++pp);
		str = p = xmalloc(len);
		pp = argv;
		за (;;) {
			p = stpcpy(p, *pp);
			pp++;
			ако (!*pp)
				break;
			*p++ = ' ';
		}
		parse_and_run_string(str);
		free(str);
	}
	IF_HUSH_MODE_X(G.x_mode_depth--;)
	//bb_error_msg("%s: --x_mode_depth=%d", __func__, G.x_mode_depth);
	върни G.last_exitcode;
}

static цяло FAST_FUNC builtin_exec(char **argv)
{
	argv = skip_dash_dash(argv);
	ако (argv[0] == NULL)
		върни EXIT_SUCCESS; /* bash does this */

	/* Careful: we can end up here after [v]fork. Do not restore
	 * tty pgrp then, only top-level shell process does that */
	ако (G_saved_tty_pgrp && getpid() == G.root_pid)
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

static цяло FAST_FUNC builtin_exit(char **argv)
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
	ако (argv[0] == NULL) {
#ако ENABLE_HUSH_TRAP
		ако (G.pre_trap_exitcode >= 0) /* "exit" in trap uses $? from before the trap */
			hush_exit(G.pre_trap_exitcode);
#endif
		hush_exit(G.last_exitcode);
	}
	/* mimic bash: exit 123abc == exit 255 + error msg */
	xfunc_error_retval = 255;
	/* bash: exit -2 == exit 254, no error msg */
	hush_exit(xatoi(argv[0]) & 0xff);
}

#ако ENABLE_HUSH_TYPE
/* http://www.opengroup.org/onlinepubs/9699919799/utilities/type.html */
static цяло FAST_FUNC builtin_type(char **argv)
{
	цяло ret = EXIT_SUCCESS;

	докато (*++argv) {
		const char *type;
		char *path = NULL;

		ако (0) {} /* make conditional compile easier below */
		/*else if (find_alias(*argv))
			type = "an alias";*/
# ако ENABLE_HUSH_FUNCTIONS
		иначе ако (find_function(*argv))
			type = "a function";
# endif
		иначе ако (find_builtin(*argv))
			type = "a shell builtin";
		иначе ако ((path = find_in_path(*argv)) != NULL)
			type = path;
		иначе {
			bb_error_msg("type: %s: not found", *argv);
			ret = EXIT_FAILURE;
			continue;
		}

		printf("%s is %s\n", *argv, type);
		free(path);
	}

	върни ret;
}
#endif

#ако ENABLE_HUSH_READ
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
static цяло FAST_FUNC builtin_read(char **argv)
{
	const char *r;
	struct builtin_read_params params;

	memset(&params, 0, sizeof(params));

	/* "!": do not abort on errors.
	 * Option string must start with "sr" to match BUILTIN_READ_xxx
	 */
	params.read_flags = getopt32(argv,
# ако BASH_READ_D
		IF_NOT_HUSH_BASH_COMPAT("^")
		"!srn:p:t:u:d:" IF_NOT_HUSH_BASH_COMPAT("\0" "-1"/*min 1 arg*/),
		&params.opt_n, &params.opt_p, &params.opt_t, &params.opt_u, &params.opt_d
# иначе
		IF_NOT_HUSH_BASH_COMPAT("^")
		"!srn:p:t:u:" IF_NOT_HUSH_BASH_COMPAT("\0" "-1"/*min 1 arg*/),
		&params.opt_n, &params.opt_p, &params.opt_t, &params.opt_u
# endif
//TODO: print "read: need variable name"
//for the case of !BASH "read" with no args (now it fails silently)
//(or maybe extend getopt32() to emit a message if "-1" fails)
	);
	ако ((uint32_t)params.read_flags == (uint32_t)-1)
		върни EXIT_FAILURE;
	argv += optind;
	params.argv = argv;
	params.setvar = set_local_var_from_halves;
	params.ifs = get_local_var_value("IFS"); /* can be NULL */

 again:
	r = shell_builtin_read(&params);

	ако ((uintptr_t)r == 1 && errno == EINTR) {
		unsigned sig = check_and_run_traps();
		ако (sig != SIGINT)
			goto again;
	}

	ако ((uintptr_t)r > 1) {
		bb_simple_error_msg(r);
		r = (char*)(uintptr_t)1;
	}

	върни (uintptr_t)r;
}
#endif

#ако ENABLE_HUSH_UMASK
static цяло FAST_FUNC builtin_umask(char **argv)
{
	цяло rc;
	mode_t mask;

	rc = 1;
	mask = umask(0);
	argv = skip_dash_dash(argv);
	ако (argv[0]) {
		mode_t old_mask = mask;

		/* numeric umasks are taken as-is */
		/* symbolic umasks are inverted: "umask a=rx" calls umask(222) */
		ако (!isdigit(argv[0][0]))
			mask ^= 0777;
		mask = bb_parse_mode(argv[0], mask);
		ако (!isdigit(argv[0][0]))
			mask ^= 0777;
		ако ((unsigned)mask > 0777) {
			mask = old_mask;
			/* bash messages:
			 * bash: umask: 'q': invalid symbolic mode operator
			 * bash: umask: 999: octal number out of range
			 */
			bb_error_msg("%s: invalid mode '%s'", "umask", argv[0]);
			rc = 0;
		}
	} иначе {
		/* Mimic bash */
		printf("%04o\n", (unsigned) mask);
		/* fall through and restore mask which we set to 0 */
	}
	umask(mask);

	върни !rc; /* rc != 0 - success */
}
#endif

#ако ENABLE_HUSH_EXPORT || ENABLE_HUSH_READONLY || ENABLE_HUSH_SET || ENABLE_HUSH_TRAP
static void print_escaped(const char *s)
{
//TODO? bash "set" does not quote variables which contain only alnums and "%+,-./:=@_~",
// (but "export" quotes all variables, even with only these chars).
// I think quoting strings with %+,=~ looks better
// (example: "set" printing var== instead of var='=' looks strange)
// IOW: do not quote "-./:@_": / is used in pathnames, : in PATH, -._ often in file names, @ in emails

	ако (*s == '\'')
		goto squote;
	do {
		const char *p = strchrnul(s, '\'');
		/* print 'xxxx', possibly just '' */
		printf("'%.*s'", (цяло)(p - s), s);
		ако (*p == '\0')
			break;
		s = p;
 squote:
		/* s points to '; print "'''...'''" */
		putchar('"');
		do putchar('\''); докато (*++s == '\'');
		putchar('"');
	} докато (*s);
}
#endif

#ако ENABLE_HUSH_EXPORT || ENABLE_HUSH_LOCAL || ENABLE_HUSH_READONLY
static цяло helper_export_local(char **argv, unsigned flags)
{
	do {
		char *name = *argv;
		const char *name_end = endofname(name);

		ако (*name_end == '\0') {
			struct variable *var, **vpp;

			vpp = get_ptr_to_local_var(name);
			var = vpp ? *vpp : NULL;

			ако (flags & SETFLAG_UNEXPORT) {
				/* export -n NAME (without =VALUE) */
				ако (var) {
					var->flg_export = 0;
					debug_printf_env("%s: unsetenv '%s'\n", __func__, name);
					unsetenv(name);
				} /* else: export -n NOT_EXISTING_VAR: no-op */
				continue;
			}
			ако (flags & SETFLAG_EXPORT) {
				/* export NAME (without =VALUE) */
				ако (var) {
					var->flg_export = 1;
					debug_printf_env("%s: putenv '%s'\n", __func__, var->varstr);
					putenv(var->varstr);
					continue;
				}
			}
			ако (flags & SETFLAG_MAKE_RO) {
				/* readonly NAME (without =VALUE) */
				ако (var) {
					var->flg_read_only = 1;
					continue;
				}
			}
# ако ENABLE_HUSH_LOCAL
			/* Is this "local" bltin? */
			ако (!(flags & (SETFLAG_EXPORT|SETFLAG_UNEXPORT|SETFLAG_MAKE_RO))) {
				unsigned lvl = flags >> SETFLAG_VARLVL_SHIFT;
				ако (var && var->var_nest_level == lvl) {
					/* "local x=abc; ...; local x" - ignore second local decl */
					continue;
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
		} иначе {
			ако (*name_end != '=') {
				bb_error_msg("'%s': bad variable name", name);
				/* do not parse following argv[]s: */
				върни 1;
			}
			/* (Un)exporting/making local NAME=VALUE */
			name = xstrdup(name);
			/* Testcase: export PS1='\w \$ ' */
			unbackslash(name);
		}
		debug_printf_env("%s: set_local_var('%s')\n", __func__, name);
		ако (set_local_var(name, flags))
			върни EXIT_FAILURE;
	} докато (*++argv);
	върни EXIT_SUCCESS;
}
#endif

#ако ENABLE_HUSH_EXPORT
static цяло FAST_FUNC builtin_export(char **argv)
{
	unsigned opt_unexport;

# ако ENABLE_HUSH_EXPORT_N
	/* "!": do not abort on errors */
	opt_unexport = getopt32(argv, "!n");
	ако (opt_unexport == (uint32_t)-1)
		върни EXIT_FAILURE;
	argv += optind;
# иначе
	opt_unexport = 0;
	argv++;
# endif

	ако (argv[0] == NULL) {
		char **e = environ;
		ако (e) {
			докато (*e) {
# ако 0
				puts(*e++);
# иначе
				/* ash emits: export VAR='VAL'
				 * bash: declare -x VAR="VAL"
				 * we follow ash example */
				const char *s = *e++;
				const char *p = strchr(s, '=');

				ако (!p) /* wtf? take next variable */
					continue;
				/* "export VAR=" */
				printf("%s %.*s", "export", (цяло)(p - s) + 1, s);
				print_escaped(p + 1);
				putchar('\n');
# endif
			}
			/*fflush_all(); - done after each builtin anyway */
		}
		върни EXIT_SUCCESS;
	}

	върни helper_export_local(argv, opt_unexport ? SETFLAG_UNEXPORT : SETFLAG_EXPORT);
}
#endif

#ако ENABLE_HUSH_LOCAL
static цяло FAST_FUNC builtin_local(char **argv)
{
	ако (G.func_nest_level == 0) {
		bb_error_msg("%s: not in a function", argv[0]);
		върни EXIT_FAILURE; /* bash compat */
	}
//TODO? ash and bash support "local -" special form,
//which saves/restores $- around function call (including async returns, such as ^C)
//(IOW: it makes "set +/-..." effects local)
	argv++;
	/* Since all builtins run in a nested variable level,
	 * need to use level - 1 here. Or else the variable will be removed at once
	 * after builtin returns.
	 */
	върни helper_export_local(argv, (G.var_nest_level - 1) << SETFLAG_VARLVL_SHIFT);
}
#endif

#ако ENABLE_HUSH_READONLY
static цяло FAST_FUNC builtin_readonly(char **argv)
{
	argv++;
	ако (*argv == NULL) {
		/* bash: readonly [-p]: list all readonly VARs
		 * (-p has no effect in bash)
		 */
		struct variable *e;
		за (e = G.top_var; e; e = e->next) {
			ако (e->flg_read_only) {
				const char *s = e->varstr;
				const char *p = strchr(s, '=');

				ако (!p) /* wtf? take next variable */
					continue;
				/* "readonly VAR=" */
				printf("%s %.*s", "readonly", (цяло)(p - s) + 1, s);
				print_escaped(p + 1);
				putchar('\n');
			}
		}
		върни EXIT_SUCCESS;
	}
	върни helper_export_local(argv, SETFLAG_MAKE_RO);
}
#endif

#ако ENABLE_HUSH_UNSET
/* http://www.opengroup.org/onlinepubs/9699919799/utilities/V3_chap02.html#unset */
static цяло FAST_FUNC builtin_unset(char **argv)
{
	цяло ret;
	unsigned opts;

	/* "!": do not abort on errors */
	/* "+": stop at 1st non-option */
	opts = getopt32(argv, "!+vf");
	ако (opts == (unsigned)-1)
		върни EXIT_FAILURE;
	ако (opts == 3) {
		bb_simple_error_msg("unset: -v and -f are exclusive");
		върни EXIT_FAILURE;
	}
	argv += optind;

	ret = EXIT_SUCCESS;
	докато (*argv) {
		ако (!(opts & 2)) { /* not -f */
			ако (unset_local_var(*argv)) {
				/* unset <nonexistent_var> doesn't fail.
				 * Error is when one tries to unset RO var.
				 * Message was printed by unset_local_var. */
				ret = EXIT_FAILURE;
			}
		}
# ако ENABLE_HUSH_FUNCTIONS
		иначе {
			unset_func(*argv);
		}
# endif
		argv++;
	}
	върни ret;
}
#endif

#ако ENABLE_HUSH_SET
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
static цяло FAST_FUNC builtin_set(char **argv)
{
	цяло n;
	char **pp, **g_argv;
	char *arg = *++argv;

	ако (arg == NULL) {
		struct variable *e;
		за (e = G.top_var; e; e = e->next) {
			const char *s = e->varstr;
			const char *p = strchr(s, '=');

			ако (!p) /* wtf? take next variable */
				continue;
			/* var= */
			printf("%.*s", (цяло)(p - s) + 1, s);
			print_escaped(p + 1);
			putchar('\n');
		}
		върни EXIT_SUCCESS;
	}

	do {
		ако (strcmp(arg, "--") == 0) {
			++argv;
			goto set_argv;
		}
		ако (arg[0] != '+' && arg[0] != '-')
			break;
		за (n = 1; arg[n]; ++n) {
			ако (set_mode((arg[0] == '-'), arg[n], argv[1])) {
				bb_error_msg("%s: %s: invalid option", "set", arg);
				върни EXIT_FAILURE;
			}
			ако (arg[n] == 'o' && argv[1])
				argv++;
		}
	} докато ((arg = *++argv) != NULL);
	/* Now argv[0] is 1st argument */

	ако (arg == NULL)
		върни EXIT_SUCCESS;
 set_argv:

	/* NB: G.global_argv[0] ($0) is never freed/changed */
	g_argv = G.global_argv;
	ако (G.global_args_malloced) {
		pp = g_argv;
		докато (*++pp)
			free(*pp);
		g_argv[1] = NULL;
	} иначе {
		G.global_args_malloced = 1;
		pp = xzalloc(sizeof(pp[0]) * 2);
		pp[0] = g_argv[0]; /* retain $0 */
		g_argv = pp;
	}
	/* This realloc's G.global_argv */
	G.global_argv = pp = add_strings_to_strings(g_argv, argv, /*dup:*/ 1);

	G.global_argc = 1 + string_array_len(pp + 1);

	върни EXIT_SUCCESS;
}
#endif

static цяло FAST_FUNC builtin_shift(char **argv)
{
	цяло n = 1;
	argv = skip_dash_dash(argv);
	ако (argv[0]) {
		n = bb_strtou(argv[0], NULL, 10);
		ако (errno || n < 0) {
			/* shared string with ash.c */
			bb_error_msg("Illegal number: %s", argv[0]);
			/*
			 * ash aborts in this case.
			 * bash prints error message and set $? to 1.
			 * Interestingly, for "shift 99999" bash does not
			 * print error message, but does set $? to 1
			 * (and does no shifting at all).
			 */
		}
	}
	ако (n >= 0 && n < G.global_argc) {
		ако (G_global_args_malloced) {
			цяло m = 1;
			докато (m <= n)
				free(G.global_argv[m++]);
		}
		G.global_argc -= n;
		memmove(&G.global_argv[1], &G.global_argv[n+1],
				G.global_argc * sizeof(G.global_argv[0]));
		върни EXIT_SUCCESS;
	}
	върни EXIT_FAILURE;
}

#ако ENABLE_HUSH_GETOPTS
static цяло FAST_FUNC builtin_getopts(char **argv)
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
	char cbuf[2];
	const char *cp, *optstring, *var;
	цяло c, n, exitcode, my_opterr;
	unsigned count;

	optstring = *++argv;
	ако (!optstring || !(var = *++argv)) {
		bb_simple_error_msg("usage: getopts OPTSTRING VAR [ARGS]");
		върни EXIT_FAILURE;
	}

	ако (argv[1])
		argv[0] = G.global_argv[0]; /* for error messages in getopt() */
	иначе
		argv = G.global_argv;
	cbuf[1] = '\0';

	my_opterr = 0;
	ако (optstring[0] != ':') {
		cp = get_local_var_value("OPTERR");
		/* 0 if "OPTERR=0", 1 otherwise */
		my_opterr = (!cp || NOT_LONE_CHAR(cp, '0'));
	}

	/* getopts stops on first non-option. Add "+" to force that */
	/*if (optstring[0] != '+')*/ {
		char *s = alloca(strlen(optstring) + 2);
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
	count = 0;
	n = string_array_len(argv);
	do {
		optarg = NULL;
		opterr = (count < G.getopt_count) ? 0 : my_opterr;
		c = getopt(n, argv, optstring);
		ако (c < 0)
			break;
		count++;
	} докато (count <= G.getopt_count);

	/* Set OPTIND. Prevent resetting of the magic counter! */
	set_local_var_from_halves("OPTIND", utoa(optind));
	G.getopt_count = count; /* "next time, give me N+1'th result" */
	GETOPT_RESET(); /* just in case */

	/* Set OPTARG */
	/* Always set or unset, never left as-is, even on exit/error:
	 * "If no option was found, or if the option that was found
	 * does not have an option-argument, OPTARG shall be unset."
	 */
	cp = optarg;
	ако (c == '?') {
		/* If ":optstring" and unknown option is seen,
		 * it is stored to OPTARG.
		 */
		ако (optstring[1] == ':') {
			cbuf[0] = optopt;
			cp = cbuf;
		}
	}
	ако (cp)
		set_local_var_from_halves("OPTARG", cp);
	иначе
		unset_local_var("OPTARG");

	/* Convert -1 to "?" */
	exitcode = EXIT_SUCCESS;
	ако (c < 0) { /* -1: end of options */
		exitcode = EXIT_FAILURE;
		c = '?';
	}

	/* Set VAR */
	cbuf[0] = c;
	set_local_var_from_halves(var, cbuf);

	върни exitcode;
}
#endif

static цяло FAST_FUNC builtin_source(char **argv)
{
	char *arg_path, *filename;
	HFILE *input;
	save_arg_t sv;
	char *args_need_save;
#ако ENABLE_HUSH_FUNCTIONS
	smallint sv_flg;
#endif

	argv = skip_dash_dash(argv);
	filename = argv[0];
	ако (!filename) {
		/* bash says: "bash: .: filename argument required" */
		върни 2; /* bash compat */
	}
	arg_path = NULL;
	ако (!strchr(filename, '/')) {
		arg_path = find_in_path(filename);
		ако (arg_path)
			filename = arg_path;
		иначе ако (!ENABLE_HUSH_BASH_SOURCE_CURDIR) {
			errno = ENOENT;
			bb_simple_perror_msg(filename);
			върни EXIT_FAILURE;
		}
	}
	input = hfopen(filename);
	free(arg_path);
	ако (!input) {
		bb_perror_msg("%s", filename);
		/* POSIX: non-interactive shell should abort here,
		 * not merely fail. So far no one complained :)
		 */
		върни EXIT_FAILURE;
	}

#ако ENABLE_HUSH_FUNCTIONS
	sv_flg = G_flag_return_in_progress;
	/* "we are inside sourced file, ok to use return" */
	G_flag_return_in_progress = -1;
#endif
	args_need_save = argv[1]; /* used as a boolean variable */
	ако (args_need_save)
		save_and_replace_G_args(&sv, argv);

	/* "false; . ./empty_line; echo Zero:$?" should print 0 */
	G.last_exitcode = 0;
	parse_and_run_file(input);
	hfclose(input);

	ако (args_need_save) /* can't use argv[1] instead: "shift" can mangle it */
		restore_G_args(&sv, argv);
#ако ENABLE_HUSH_FUNCTIONS
	G_flag_return_in_progress = sv_flg;
#endif

	върни G.last_exitcode;
}

#ако ENABLE_HUSH_TRAP
static цяло FAST_FUNC builtin_trap(char **argv)
{
	цяло sig;
	char *new_cmd;

	ако (!G_traps)
		G_traps = xzalloc(sizeof(G_traps[0]) * NSIG);

	argv++;
	ако (!*argv) {
		цяло i;
		/* No args: print all trapped */
		за (i = 0; i < NSIG; ++i) {
			ако (G_traps[i]) {
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
		върни EXIT_SUCCESS;
	}

	new_cmd = NULL;
	/* If first arg is a number: reset all specified signals */
	sig = bb_strtou(*argv, NULL, 10);
	ако (errno == 0) {
		цяло ret;
 process_sig_list:
		ret = EXIT_SUCCESS;
		докато (*argv) {
			sighandler_t handler;

			sig = get_signum(*argv++);
			ако (sig < 0) {
				ret = EXIT_FAILURE;
				/* Mimic bash message exactly */
				bb_error_msg("trap: %s: invalid signal specification", argv[-1]);
				continue;
			}

			free(G_traps[sig]);
			G_traps[sig] = xstrdup(new_cmd);

			debug_printf("trap: setting SIG%s (%i) to '%s'\n",
				get_signame(sig), sig, G_traps[sig]);

			/* There is no signal for 0 (EXIT) */
			ако (sig == 0)
				continue;

			ако (new_cmd)
				handler = (new_cmd[0] ? record_pending_signo : SIG_IGN);
			иначе
				/* We are removing trap handler */
				handler = pick_sighandler(sig);
			install_sighandler(sig, handler);
		}
		върни ret;
	}

	ако (!argv[1]) { /* no second arg */
		bb_simple_error_msg("trap: invalid arguments");
		върни EXIT_FAILURE;
	}

	/* First arg is "-": reset all specified to default */
	/* First arg is "--": skip it, the rest is "handler SIGs..." */
	/* Everything else: set arg as signal handler
	 * (includes "" case, which ignores signal) */
	ако (argv[0][0] == '-') {
		ако (argv[0][1] == '\0') { /* "-" */
			/* new_cmd remains NULL: "reset these sigs" */
			goto reset_traps;
		}
		ако (argv[0][1] == '-' && argv[0][2] == '\0') { /* "--" */
			argv++;
		}
		/* else: "-something", no special meaning */
	}
	new_cmd = *argv;
 reset_traps:
	argv++;
	goto process_sig_list;
}
#endif

#ако ENABLE_HUSH_JOB
static struct pipe *parse_jobspec(const char *str)
{
	struct pipe *pi;
	unsigned jobnum;

	ако (sscanf(str, "%%%u", &jobnum) != 1) {
		ако (str[0] != '%'
		 || (str[1] != '%' && str[1] != '+' && str[1] != '\0')
		) {
			bb_error_msg("bad argument '%s'", str);
			върни NULL;
		}
		/* It is "%%", "%+" or "%" - current job */
		jobnum = G.last_jobid;
		ако (jobnum == 0) {
			bb_simple_error_msg("no current job");
			върни NULL;
		}
	}
	за (pi = G.job_list; pi; pi = pi->next) {
		ако (pi->jobid == jobnum) {
			върни pi;
		}
	}
	bb_error_msg("%u: no such job", jobnum);
	върни NULL;
}

static цяло FAST_FUNC builtin_jobs(char **argv UNUSED_PARAM)
{
	struct pipe *job;
	const char *status_string;

	checkjobs(NULL, 0 /*(no pid to wait for)*/);
	за (job = G.job_list; job; job = job->next) {
		ако (job->alive_cmds == job->stopped_cmds)
			status_string = "Stopped";
		иначе
			status_string = "Running";

		printf(JOB_STATUS_FORMAT, job->jobid, status_string, job->cmdtext);
	}

	clean_up_last_dead_job();

	върни EXIT_SUCCESS;
}

/* built-in 'fg' and 'bg' handler */
static цяло FAST_FUNC builtin_fg_bg(char **argv)
{
	цяло i;
	struct pipe *pi;

	ако (!G_interactive_fd)
		върни EXIT_FAILURE;

	/* If they gave us no args, assume they want the last backgrounded task */
	ако (!argv[1]) {
		за (pi = G.job_list; pi; pi = pi->next) {
			ако (pi->jobid == G.last_jobid) {
				goto found;
			}
		}
		bb_error_msg("%s: no current job", argv[0]);
		върни EXIT_FAILURE;
	}

	pi = parse_jobspec(argv[1]);
	ако (!pi)
		върни EXIT_FAILURE;
 found:
	/* TODO: bash prints a string representation
	 * of job being foregrounded (like "sleep 1 | cat") */
	ако (argv[0][0] == 'f' && G_saved_tty_pgrp) {
		/* Put the job into the foreground. */
		tcsetpgrp(G_interactive_fd, pi->pgrp);
	}

	/* Restart the processes in the job */
	debug_printf_jobs("reviving %d procs, pgrp %d\n", pi->num_cmds, pi->pgrp);
	за (i = 0; i < pi->num_cmds; i++) {
		debug_printf_jobs("reviving pid %d\n", pi->cmds[i].pid);
	}
	pi->stopped_cmds = 0;

	i = kill(- pi->pgrp, SIGCONT);
	ако (i < 0) {
		ако (errno == ESRCH) {
			delete_finished_job(pi);
			върни EXIT_SUCCESS;
		}
		bb_simple_perror_msg("kill (SIGCONT)");
	}

	ако (argv[0][0] == 'f') {
		remove_job_from_table(pi); /* FG job shouldn't be in job table */
		върни checkjobs_and_fg_shell(pi);
	}
	върни EXIT_SUCCESS;
}
#endif

#ако ENABLE_HUSH_KILL
static цяло FAST_FUNC builtin_kill(char **argv)
{
	цяло ret = 0;

# ако ENABLE_HUSH_JOB
	ако (argv[1] && strcmp(argv[1], "-l") != 0) {
		цяло i = 1;

		do {
			struct pipe *pi;
			char *dst;
			цяло j, n;

			ако (argv[i][0] != '%')
				continue;
			/*
			 * "kill %N" - job kill
			 * Converting to pgrp / pid kill
			 */
			pi = parse_jobspec(argv[i]);
			ако (!pi) {
				/* Eat bad jobspec */
				j = i;
				do {
					j++;
					argv[j - 1] = argv[j];
				} докато (argv[j]);
				ret = 1;
				i--;
				continue;
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
			n = G_interactive_fd ? 1 : pi->num_cmds;
			dst = alloca(n * sizeof(цяло)*4);
			argv[i] = dst;
			ако (G_interactive_fd)
				dst += sprintf(dst, " -%u", (цяло)pi->pgrp);
			иначе за (j = 0; j < n; j++) {
				struct command *cmd = &pi->cmds[j];
				/* Skip exited members of the job */
				ако (cmd->pid == 0)
					continue;
				/*
				 * kill_main has matching code to expect
				 * leading space. Needed to not confuse
				 * negative pids with "kill -SIGNAL_NO" syntax
				 */
				dst += sprintf(dst, " %u", (цяло)cmd->pid);
			}
			*dst = '\0';
		} докато (argv[++i]);
	}
# endif

	ако (argv[1] || ret == 0) {
		ret = run_applet_main(argv, kill_main);
	}
	/* else: ret = 1, "kill %bad_jobspec" case */
	върни ret;
}
#endif

#ако ENABLE_HUSH_WAIT
/* http://www.opengroup.org/onlinepubs/9699919799/utilities/wait.html */
# ако !ENABLE_HUSH_JOB
#  define wait_for_child_or_signal(pipe,pid) wait_for_child_or_signal(pid)
# endif
static цяло wait_for_child_or_signal(struct pipe *waitfor_pipe, pid_t waitfor_pid)
{
	цяло ret = 0;
	за (;;) {
		цяло sig;
		sigset_t oldset;

		ако (!sigisemptyset(&G.pending_set))
			goto check_sig;

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

		ако (!sigisemptyset(&G.pending_set)) {
			/* Crap! we raced with some signal! */
			goto restore;
		}

		/*errno = 0; - checkjobs does this */
/* Can't pass waitfor_pipe into checkjobs(): it won't be interruptible */
		ret = checkjobs(NULL, waitfor_pid); /* waitpid(WNOHANG) inside */
		debug_printf_exec("checkjobs:%d\n", ret);
# ако ENABLE_HUSH_JOB
		ако (waitfor_pipe) {
			цяло rcode = job_exited_or_stopped(waitfor_pipe);
			debug_printf_exec("job_exited_or_stopped:%d\n", rcode);
			ако (rcode >= 0) {
				ret = rcode;
				sigprocmask(SIG_SETMASK, &oldset, NULL);
				break;
			}
		}
# endif
		/* if ECHILD, there are no children (ret is -1 or 0) */
		/* if ret == 0, no children changed state */
		/* if ret != 0, it's exitcode+1 of exited waitfor_pid child */
		ако (errno == ECHILD || ret) {
			ret--;
			ако (ret < 0) /* if ECHILD, may need to fix "ret" */
				ret = 0;
# ако ENABLE_HUSH_BASH_COMPAT
			ако (waitfor_pid == -1 && errno == ECHILD) {
				/* exitcode of "wait -n" with no children is 127, not 0 */
				ret = 127;
			}
# endif
			sigprocmask(SIG_SETMASK, &oldset, NULL);
			break;
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
		sigprocmask(SIG_SETMASK, &oldset, NULL);
 check_sig:
		/* So, did we get a signal? */
		sig = check_and_run_traps();
		ако (sig /*&& sig != SIGCHLD - always true */) {
			/* Do this for any (non-ignored) signal, not only for ^C */
			ret = 128 | sig;
			break;
		}
		/* SIGCHLD, or no signal, or ignored one, such as SIGQUIT. Repeat */
	}
	върни ret;
}

static цяло FAST_FUNC builtin_wait(char **argv)
{
	цяло ret;
	цяло status;

	argv = skip_dash_dash(argv);
# ако ENABLE_HUSH_BASH_COMPAT
	ако (argv[0] && strcmp(argv[0], "-n") == 0) {
		/* wait -n */
		/* (bash accepts "wait -n PID" too and ignores PID) */
		G.dead_job_exitcode = -1;
		върни wait_for_child_or_signal(NULL, -1 /*no job, wait for one job*/);
	}
# endif
	ако (argv[0] == NULL) {
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
		върни wait_for_child_or_signal(NULL, 0 /*no job and no pid to wait for*/);
	}

	do {
		pid_t pid = bb_strtou(*argv, NULL, 10);
		ако (errno || pid <= 0) {
# ако ENABLE_HUSH_JOB
			ако (argv[0][0] == '%') {
				struct pipe *wait_pipe;
				ret = 127; /* bash compat for bad jobspecs */
				wait_pipe = parse_jobspec(*argv);
				ако (wait_pipe) {
					ret = job_exited_or_stopped(wait_pipe);
					ако (ret < 0) {
						ret = wait_for_child_or_signal(wait_pipe, 0);
					} иначе {
						/* waiting on "last dead job" removes it */
						clean_up_last_dead_job();
					}
				}
				/* else: parse_jobspec() already emitted error msg */
				continue;
			}
# endif
			/* mimic bash message */
			bb_error_msg("wait: '%s': not a pid or valid job spec", *argv);
			ret = EXIT_FAILURE;
			continue; /* bash checks all argv[] */
		}

		/* Do we have such child? */
		ret = waitpid(pid, &status, WNOHANG);
		ако (ret < 0) {
			/* No */
			ret = 127;
			ако (errno == ECHILD) {
				ако (pid == G.last_bg_pid) {
					/* "wait $!" but last bg task has already exited. Try:
					 * (sleep 1; exit 3) & sleep 2; echo $?; wait $!; echo $?
					 * In bash it prints exitcode 0, then 3.
					 * In dash, it is 127.
					 */
					ret = G.last_bg_pid_exitcode;
				} иначе {
					/* Example: "wait 1". mimic bash message */
					bb_error_msg("wait: pid %u is not a child of this shell", (unsigned)pid);
				}
			} иначе {
				/* ??? */
				bb_perror_msg("wait %s", *argv);
			}
			continue; /* bash checks all argv[] */
		}
		ако (ret == 0) {
			/* Yes, and it still runs */
			ret = wait_for_child_or_signal(NULL, pid);
		} иначе {
			/* Yes, and it just exited */
			process_wait_result(NULL, pid, status);
			ret = WEXITSTATUS(status);
			ако (WIFSIGNALED(status))
				ret = 128 | WTERMSIG(status);
		}
	} докато (*++argv);

	върни ret;
}
#endif

#ако ENABLE_HUSH_LOOPS || ENABLE_HUSH_FUNCTIONS
static unsigned parse_numeric_argv1(char **argv, unsigned def, unsigned def_min)
{
	ако (argv[1]) {
		def = bb_strtou(argv[1], NULL, 10);
		ако (errno || def < def_min || argv[2]) {
			bb_error_msg("%s: bad arguments", argv[0]);
			def = UINT_MAX;
		}
	}
	върни def;
}
#endif

#ако ENABLE_HUSH_LOOPS
static цяло FAST_FUNC builtin_break(char **argv)
{
	unsigned depth;
	ако (G.depth_of_loop == 0) {
		bb_error_msg("%s: only meaningful in a loop", argv[0]);
		/* if we came from builtin_continue(), need to undo "= 1" */
		G.flag_break_continue = 0;
		върни EXIT_SUCCESS; /* bash compat */
	}
	G.flag_break_continue++; /* BC_BREAK = 1, or BC_CONTINUE = 2 */

	G.depth_break_continue = depth = parse_numeric_argv1(argv, 1, 1);
	ако (depth == UINT_MAX)
		G.flag_break_continue = BC_BREAK;
	ако (G.depth_of_loop < depth)
		G.depth_break_continue = G.depth_of_loop;

	върни EXIT_SUCCESS;
}

static цяло FAST_FUNC builtin_continue(char **argv)
{
	G.flag_break_continue = 1; /* BC_CONTINUE = 2 = 1+1 */
	върни builtin_break(argv);
}
#endif

#ако ENABLE_HUSH_FUNCTIONS
static цяло FAST_FUNC builtin_return(char **argv)
{
	цяло rc;

	ако (G_flag_return_in_progress != -1) {
		bb_error_msg("%s: not in a function or sourced script", argv[0]);
		върни EXIT_FAILURE; /* bash compat */
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
# ако ENABLE_HUSH_TRAP
	ако (argv[1]) { /* "return ARG" inside a running trap sets $? */
		debug_printf_exec("G.return_exitcode=%d\n", rc);
		G.return_exitcode = rc;
	}
# endif
	върни rc;
}
#endif

#ако ENABLE_HUSH_TIMES
static цяло FAST_FUNC builtin_times(char **argv UNUSED_PARAM)
{
	static const uint8_t times_tbl[] ALIGN1 = {
		' ',  offsetof(struct tms, tms_utime),
		'\n', offsetof(struct tms, tms_stime),
		' ',  offsetof(struct tms, tms_cutime),
		'\n', offsetof(struct tms, tms_cstime),
		0
	};
	const uint8_t *p;
	unsigned clk_tck;
	struct tms buf;

	clk_tck = bb_clk_tck();

	times(&buf);
	p = times_tbl;
	do {
		unsigned sec, frac;
		unsigned long t;
		t = *(clock_t *)(((char *) &buf) + p[1]);
		sec = t / clk_tck;
		frac = t % clk_tck;
		printf("%um%u.%03us%c",
			sec / 60, sec % 60,
			(frac * 1000) / clk_tck,
			p[0]);
		p += 2;
	} докато (*p);

	върни EXIT_SUCCESS;
}
#endif

#ако ENABLE_HUSH_MEMLEAK
static цяло FAST_FUNC builtin_memleak(char **argv UNUSED_PARAM)
{
	void *p;
	unsigned long l;

# ifdef M_TRIM_THRESHOLD
	/* Optional. Reduces probability of false positives */
	malloc_trim(0);
# endif
	/* Crude attempt to find where "free memory" starts,
	 * sans fragmentation. */
	p = malloc(240);
	l = (unsigned long)p;
	free(p);
	p = malloc(3400);
	ако (l < (unsigned long)p) l = (unsigned long)p;
	free(p);

# ако 0  /* debug */
	{
		struct mallinfo mi = mallinfo();
		printf("top alloc:0x%lx malloced:%d+%d=%d\n", l,
			mi.arena, mi.hblkhd, mi.arena + mi.hblkhd);
	}
# endif

	ако (!G.memleak_value)
		G.memleak_value = l;

	l -= G.memleak_value;
	ако ((long)l < 0)
		l = 0;
	l /= 1024;
	ако (l > 127)
		l = 127;

	/* Exitcode is "how many kilobytes we leaked since 1st call" */
	върни l;
}
#endif
#endif /* !__U_BOOT__ */
