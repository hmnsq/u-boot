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
#જો !(defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) \
	|| defined(__APPLE__) \
    )
# include <malloc.h>   /* for malloc_trim */
#endif
#અન્તર્ભાવ <glob.h>
/* #include <dmalloc.h> */
#જો ENABLE_HUSH_CASE
# include <fnmatch.h>
#endif
#અન્તર્ભાવ <sys/times.h>
#અન્તર્ભાવ <sys/utsname.h> /* for setting $HOSTNAME */

#અન્તર્ભાવ "busybox.h"  /* for APPLET_IS_NOFORK/NOEXEC */
#અન્તર્ભાવ "unicode.h"
#અન્તર્ભાવ "shell_common.h"
#અન્તર્ભાવ "math.h"
#અન્તર્ભાવ "match.h"
#જો ENABLE_HUSH_RANDOM_SUPPORT
# include "random.h"
#નહીંતર
# define CLEAR_RANDOM_T(rnd) ((શૂન્ય)0)
#endif
#ifndef O_CLOEXEC
# define O_CLOEXEC 0
#endif
#ifndef F_DUPFD_CLOEXEC
# define F_DUPFD_CLOEXEC F_DUPFD
#endif

#જો ENABLE_FEATURE_SH_EMBEDDED_SCRIPTS && !ENABLE_SHELL_ASH
# include "embedded_scripts.h"
#નહીંતર
# define અંક_SCRIPTS 0
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

#જો BUILD_AS_NOMMU
# undef BB_MMU
# undef USE_FOR_NOMMU
# undef USE_FOR_MMU
# define BB_MMU 0
# define USE_FOR_NOMMU(...) __VA_ARGS__
# define USE_FOR_MMU(...)
#endif

#ifndef __U_BOOT__
#અન્તર્ભાવ "NUM_APPLETS.h"
#જો અંક_APPLETS == 1
/* STANDALONE does not make sense, and won't compile */
# undef ENABLE_FEATURE_SH_STANDALONE
# undef IF_FEATURE_SH_STANDALONE
# undef IF_NOT_FEATURE_SH_STANDALONE
# define ENABLE_FEATURE_SH_STANDALONE 0
# define IF_FEATURE_SH_STANDALONE(...)
# define IF_NOT_FEATURE_SH_STANDALONE(...) __VA_ARGS__
#endif
#endif /* __U_BOOT__ */

#જો !ENABLE_HUSH_INTERACTIVE
# undef ENABLE_FEATURE_EDITING
# define ENABLE_FEATURE_EDITING 0
# undef ENABLE_FEATURE_EDITING_FANCY_PROMPT
# define ENABLE_FEATURE_EDITING_FANCY_PROMPT 0
# undef ENABLE_FEATURE_EDITING_SAVE_ON_EXIT
# define ENABLE_FEATURE_EDITING_SAVE_ON_EXIT 0
#endif

/* Do we support ANY keywords? */
#જો ENABLE_HUSH_IF || ENABLE_HUSH_LOOPS || ENABLE_HUSH_CASE
# define HAS_KEYWORDS 1
# define IF_HAS_KEYWORDS(...) __VA_ARGS__
# define IF_HAS_NO_KEYWORDS(...)
#નહીંતર
# define HAS_KEYWORDS 0
# define IF_HAS_KEYWORDS(...)
# define IF_HAS_NO_KEYWORDS(...) __VA_ARGS__
#endif

/* If you comment out one of these below, it will be #defined later
 * to perform debug printfs to stderr: */
#define debug_printf(...)         કરો {} જ્યારે (0)
/* Finer-grained debug switches */
#define debug_printf_parse(...)   કરો {} જ્યારે (0)
#define debug_printf_heredoc(...) કરો {} જ્યારે (0)
#define debug_print_વૃક્ષ(a, b)    કરો {} જ્યારે (0)
#define debug_printf_exec(...)    કરો {} જ્યારે (0)
#define debug_printf_env(...)     કરો {} જ્યારે (0)
#define debug_printf_jobs(...)    કરો {} જ્યારે (0)
#define debug_printf_expand(...)  કરો {} જ્યારે (0)
#define debug_printf_varexp(...)  કરો {} જ્યારે (0)
#define debug_printf_glob(...)    કરો {} જ્યારે (0)
#define debug_printf_redir(...)   કરો {} જ્યારે (0)
#define debug_printf_list(...)    કરો {} જ્યારે (0)
#define debug_printf_subst(...)   કરો {} જ્યારે (0)
#define debug_printf_prompt(...)  કરો {} જ્યારે (0)
#define debug_printf_clean(...)   કરો {} જ્યારે (0)

#define ERR_PTR ((શૂન્ય*)(long)1)

#define JOB_STATUS_FORMAT    "[%u] %-22s %.40s\n"

#define _SPECIAL_VARS_STR     "_*@$!?#-"
#ifndef __U_BOOT__
#define SPECIAL_VARS_STR     ("_*@$!?#-" + 1)
#define NUMERIC_SPECVARS_STR ("_*@$!?#-" + 3)
#નહીંતર /* __U_BOOT__ */
#define SPECIAL_VARS_STR     "*@$!?#-"
#define NUMERIC_SPECVARS_STR "$!?#-"
#endif /* __U_BOOT__ */
#જો BASH_PATTERN_SUBST
/* Support / and // replace ops */
/* Note that // is stored as \ in "encoded" string representation */
# define VAR_ENCODED_SUBST_OPS      "\\/%#:-=+?"
# define VAR_SUBST_OPS             ("\\/%#:-=+?" + 1)
# define MINUS_PLUS_EQUAL_QUESTION ("\\/%#:-=+?" + 5)
#નહીંતર
# define VAR_ENCODED_SUBST_OPS      "%#:-=+?"
# define VAR_SUBST_OPS              "%#:-=+?"
# define MINUS_PLUS_EQUAL_QUESTION ("%#:-=+?" + 3)
#endif

#define SPECIAL_VAR_પ્રતીક_STR "\3"
#define SPECIAL_VAR_પ્રતીક       3
/* The "variable" with name "\1" emits string "\3". Testcase: "echo ^C" */
#define SPECIAL_VAR_QUOTED_SVS   1

સંરચના variable;

સ્થિર અચલ અક્ષર hush_version_str[] ALIGN1 = "HUSH_VERSION="BB_VER;

/* This supports saving pointers malloced in vfork child,
 * to be freed in the parent.
 */
#જો !BB_MMU
પ્રકારવ્યાખ્યા સંરચના nommu_save_t {
	સંરચના variable *old_vars;
	અક્ષર **argv;
	અક્ષર **argv_from_re_execing;
} nommu_save_t;
#endif

ગણના {
	RES_NONE  = 0,
#જો ENABLE_HUSH_IF
	RES_IF    ,
	RES_THEN  ,
	RES_ELIF  ,
	RES_ELSE  ,
	RES_FI    ,
#endif
#જો ENABLE_HUSH_LOOPS
	RES_FOR   ,
	RES_WHILE ,
	RES_UNTIL ,
	RES_DO    ,
	RES_DONE  ,
#endif
#જો ENABLE_HUSH_LOOPS || ENABLE_HUSH_CASE
	RES_IN    ,
#endif
#જો ENABLE_HUSH_CASE
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

પ્રકારવ્યાખ્યા સંરચના o_string {
	અક્ષર *data;
	પૂર્ણાંક length; /* position where data is appended */
	પૂર્ણાંક maxlen;
	પૂર્ણાંક o_expflags;
	/* At least some part of the string was inside '' or "",
	 * possibly empty one: word"", wo''rd etc. */
	smallint has_quoted_part;
	smallint has_empty_slot;
	smallint ended_in_ifs;
} o_string;
ગણના {
	EXP_FLAG_SINGLEWORD     = 0x80, /* must be 0x80 */
	EXP_FLAG_GLOB           = 0x2,
	/* Protect newly added chars against globbing
	 * by prepending \ to *, ?, [, \ */
	EXP_FLAG_ESC_GLOB_CHARS = 0x1,
};
/* Used for initialization: o_string foo = NULL_O_STRING; */
#define NULL_O_STRING { ખાલી }

#ifndef debug_printf_parse
સ્થિર અચલ અક્ષર *અચલ assignment_flag[] ALIGN_PTR = {
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
પ્રકારવ્યાખ્યા સંરચના HFILE {
	અક્ષર *cur;
	અક્ષર *end;
	સંરચના HFILE *next_hfile;
	પૂર્ણાંક fd;
	અક્ષર buf[1024];
} HFILE;

પ્રકારવ્યાખ્યા સંરચના in_str {
	અચલ અક્ષર *p;
	પૂર્ણાંક peek_buf[2];
	પૂર્ણાંક last_char;
	HFILE *file;
} in_str;

#ifndef __U_BOOT__
/* The descrip member of this structure is only used to make
 * debugging output pretty */
સ્થિર અચલ સંરચના {
	int32_t mode;
	signed અક્ષર default_fd;
	અક્ષર descrip[3];
} redir_table[] ALIGN4 = {
	{ O_RDONLY,                  0, "<"  },
	{ O_CREAT|O_TRUNC|O_WRONLY,  1, ">"  },
	{ O_CREAT|O_APPEND|O_WRONLY, 1, ">>" },
	{ O_CREAT|O_RDWR,            1, "<>" },
	{ O_RDONLY,                  0, "<<" },
/* Should not be needed. Bogus default_fd helps in debugging */
/*	{ O_RDONLY,                 77, "<<" }, */
};

સંરચના redir_struct {
	સંરચના redir_struct *next;
	અક્ષર *rd_filename;          /* filename */
	પૂર્ણાંક rd_fd;                  /* fd to redirect */
	/* fd to redirect to, or -3 if rd_fd is to be closed (n>&-) */
	પૂર્ણાંક rd_dup;
	smallint rd_type;           /* (enum redir_type) */
	/* note: for heredocs, rd_filename contains heredoc delimiter,
	 * and subsequently heredoc itself; and rd_dup is a bitmask:
	 * bit 0: do we need to trim leading tabs?
	 * bit 1: is heredoc quoted (<<'delim' syntax) ?
	 */
};
પ્રકારવ્યાખ્યા ગણના redir_type {
	REDIRECT_આગત     = 0,
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

સંરચના command {
#ifndef __U_BOOT__
	pid_t pid;                  /* 0 if exited */
#endif /* !__U_BOOT__ */
	unsigned assignment_cnt;    /* how many argv[i] are assignments? */
#જો ENABLE_HUSH_LINENO_VAR
	unsigned lineno;
#endif
	smallint cmd_type;          /* CMD_xxx */
#define CMD_NORMAL   0
#define CMD_SUBSHELL 1
#જો BASH_TEST2
/* used for "[[ EXPR ]]" */
# define CMD_TEST2_SINGLEWORD_NOGLOB 2
#endif
#જો BASH_TEST2 || ENABLE_HUSH_LOCAL || ENABLE_HUSH_EXPORT || ENABLE_HUSH_READONLY
/* used to prevent word splitting and globbing in "export v=t*" */
# define CMD_SINGLEWORD_NOGLOB 3
#endif
#જો ENABLE_HUSH_FUNCTIONS
# define CMD_FUNCDEF 4
#endif

	smalluint cmd_exitcode;
	/* if non-NULL, this "command" is { list }, ( list ), or a compound statement */
	સંરચના pipe *group;
#જો !BB_MMU
	અક્ષર *group_as_string;
#endif
#જો ENABLE_HUSH_FUNCTIONS
	સંરચના function *child_func;
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
	પૂર્ણાંક argc; /* number of program arguments */
#endif
	અક્ષર **argv;                /* command name and arguments */
/* argv vector may contain variable references (^Cvar^C, ^C0^C etc)
 * and on execution these are substituted with their values.
 * Substitution can make _several_ words out of one argv[n]!
 * Example: argv[0]=='.^C*^C.' here: echo .$*.
 * References of the form ^C`cmd arg^C are `cmd arg` substitutions.
 */
#ifndef __U_BOOT__
	સંરચના redir_struct *redirects; /* I/O redirections */
#endif /* !__U_BOOT__ */
};
/* Is there anything in this command at all? */
#ifndef __U_BOOT__
#define IS_NULL_CMD(cmd) \
	(!(cmd)->group && !(cmd)->argv && !(cmd)->redirects)

#નહીંતર /* __U_BOOT__ */
#define IS_NULL_CMD(cmd) \
	(!(cmd)->group && !(cmd)->argv)
#endif /* __U_BOOT__ */
સંરચના pipe {
	સંરચના pipe *next;
	પૂર્ણાંક અંક_cmds;               /* total number of commands in pipe */
#ifndef __U_BOOT__
	પૂર્ણાંક alive_cmds;             /* number of commands running (not exited) */
	પૂર્ણાંક stopped_cmds;           /* number of commands alive, but stopped */
#જો ENABLE_HUSH_JOB
	unsigned jobid;             /* job number */
	pid_t pgrp;                 /* process group ID for the job */
	અક્ષર *cmdtext;              /* name of job */
#endif
#endif /* !__U_BOOT__ */
	સંરચના command *cmds;       /* array of commands in pipe */
	smallint followup;          /* PIPE_BG, PIPE_SEQ, PIPE_OR, PIPE_AND */
	IF_HAS_KEYWORDS(smallint pi_inverted;) /* "! cmd | cmd" */
	IF_HAS_KEYWORDS(smallint res_word;) /* needed for if, for, while, until... */
};
પ્રકારવ્યાખ્યા ગણના pipe_style {
	PIPE_SEQ = 0,
	PIPE_AND = 1,
	PIPE_OR  = 2,
	PIPE_BG  = 3,
} pipe_style;
/* Is there anything in this pipe at all? */
#define IS_NULL_PIPE(pi) \
	((pi)->અંક_cmds == 0 IF_HAS_KEYWORDS( && (pi)->res_word == RES_NONE))

/* This holds pointers to the various results of parsing */
સંરચના parse_context {
	/* linked list of pipes */
	સંરચના pipe *list_head;
	/* last pipe (being constructed right now) */
	સંરચના pipe *pipe;
	/* last command in pipe (being constructed right now) */
	સંરચના command *command;
#ifndef __U_BOOT__
	/* last redirect in command->redirects list */
	સંરચના redir_struct *pending_redirect;
#endif /* !__U_BOOT__ */
	o_string word;
#જો !BB_MMU
	o_string as_string;
#endif
	smallint is_assignment; /* 0:maybe, 1:yes, 2:no, 3:keyword */
#જો HAS_KEYWORDS
	smallint ctx_res_w;
	smallint ctx_inverted; /* "! cmd | cmd" */
#જો ENABLE_HUSH_CASE
	smallint ctx_dsemicolon; /* ";;" seen */
#endif
	/* bitmask of FLAG_xxx, for figuring out valid reserved words */
	પૂર્ણાંક old_flag;
	/* group we are enclosed in:
	 * example: "if pipe1; pipe2; then pipe3; fi"
	 * when we see "if" or "then", we malloc and copy current context,
	 * and make ->stack point to it. then we parse pipeN.
	 * when closing "then" / fi" / whatever is found,
	 * we move list_head into ->stack->command->group,
	 * copy ->stack into current context, and delete ->stack.
	 * (parsing of { list } and ( list ) doesn't use this method)
	 */
	સંરચના parse_context *ઢગલો;
#endif
};
ગણના {
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
સંરચના variable {
	સંરચના variable *next;
	અક્ષર *varstr;        /* points to "name=" portion */
	પૂર્ણાંક max_len;         /* if > 0, name is part of initial env; else name is malloced */
#ifndef __U_BOOT__
	uint16_t var_nest_level;
	smallint flg_export; /* putenv should be done on this var */
	smallint flg_read_only;
#endif /* !__U_BOOT__ */
};

ગણના {
	BC_BREAK = 1,
	BC_CONTINUE = 2,
};

#જો ENABLE_HUSH_FUNCTIONS
સંરચના function {
	સંરચના function *next;
	અક્ષર *name;
	સંરચના command *parent_cmd;
	સંરચના pipe *body;
# જો !BB_MMU
	અક્ષર *body_as_string;
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
સ્થિર અચલ અક્ષર o_opt_strings[] ALIGN1 =
	"pipefail\0"
	"noexec\0"
	"errexit\0"
#જો ENABLE_HUSH_MODE_X
	"xtrace\0"
#endif
	;
ગણના {
	OPT_O_PIPEFAIL,
	OPT_O_NOEXEC,
	OPT_O_ERREXIT,
#જો ENABLE_HUSH_MODE_X
	OPT_O_XTRACE,
#endif
	અંક_OPT_O
};

/* "Globals" within this file */
/* Sorted roughly by size (smaller offsets == smaller code) */
સંરચના globals {
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
#જો ENABLE_HUSH_INTERACTIVE
	/* 'interactive_fd' is a fd# open to ctty, if we have one
	 * _AND_ if we decided to act interactively */
	પૂર્ણાંક interactive_fd;
	IF_NOT_FEATURE_EDITING_FANCY_PROMPT(અક્ષર *PS1;)
# define G_interactive_fd (G.interactive_fd)
#નહીંતર
# define G_interactive_fd 0
#endif
#નહીંતર /* __U_BOOT__ */
# define G_interactive_fd 0
#endif /* __U_BOOT__ */
#ifndef __U_BOOT__
#જો ENABLE_FEATURE_EDITING
	લીટી_આગત_t *લીટી_આગત_state;
#endif
	pid_t મૂળ_pid;
	pid_t મૂળ_ppid;
	pid_t last_bg_pid;
#જો ENABLE_HUSH_RANDOM_SUPPORT
	random_t random_gen;
#endif
#જો ENABLE_HUSH_JOB
	પૂર્ણાંક run_list_level;
	unsigned last_jobid;
	pid_t saved_tty_pgrp;
	સંરચના pipe *job_list;
# define G_saved_tty_pgrp (G.saved_tty_pgrp)
#નહીંતર
# define G_saved_tty_pgrp 0
#endif
#endif /* !__U_BOOT__ */
	/* How deeply are we in context where "set -e" is ignored */
	પૂર્ણાંક errexit_depth;
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

	અક્ષર o_opt[અંક_OPT_O];
#જો ENABLE_HUSH_MODE_X
# define G_x_mode (G.o_opt[OPT_O_XTRACE])
#નહીંતર
# define G_x_mode 0
#endif
	અક્ષર opt_s;
	અક્ષર opt_c;
#endif /* !__U_BOOT__ */
#જો ENABLE_HUSH_INTERACTIVE
	smallint promptmode; /* 0: PS1, 1: PS2 */
#endif
	/* set by signal handler if SIGINT is received _and_ its trap is not set */
	smallint flag_SIGINT;
#ifndef __U_BOOT__
#જો ENABLE_HUSH_LOOPS
	smallint flag_break_continue;
#endif
#endif /* !__U_BOOT__ */
#જો ENABLE_HUSH_FUNCTIONS
	/* 0: outside of a function (or sourced file)
	 * -1: inside of a function, ok to use return builtin
	 * 1: return is invoked, skip all till end of func
	 */
	smallint flag_return_in_progress;
# define G_flag_return_in_progress (G.flag_return_in_progress)
#નહીંતર
# define G_flag_return_in_progress 0
#endif
	smallint exiting; /* used to prevent EXIT trap recursion */
	/* These support $? */
	smalluint last_exitcode;
	smalluint expand_exitcode;
	smalluint last_bg_pid_exitcode;
#ifndef __U_BOOT__
#જો ENABLE_HUSH_SET
	/* are global_argv and global_argv[1..n] malloced? (note: not [0]) */
	smalluint global_args_malloced;
# define G_global_args_malloced (G.global_args_malloced)
#નહીંતર
# define G_global_args_malloced 0
#endif
#જો ENABLE_HUSH_BASH_COMPAT
	પૂર્ણાંક dead_job_exitcode; /* for "wait -n" */
#endif
#endif /* !__U_BOOT__ */
	/* how many non-NULL argv's we have. NB: $# + 1 */
	પૂર્ણાંક global_argc;
	અક્ષર **global_argv;
#જો !BB_MMU
	અક્ષર *argv0_for_re_execing;
#endif
#જો ENABLE_HUSH_LOOPS
#ifndef __U_BOOT__
	unsigned depth_break_continue;
#endif /* !__U_BOOT__ */
	unsigned depth_of_loop;
#endif
#ifndef __U_BOOT__
#જો ENABLE_HUSH_GETOPTS
	unsigned getopt_ગણતરી;
#endif
#endif /* !__U_BOOT__ */
	અચલ અક્ષર *ifs;
#ifdef __U_BOOT__
	પૂર્ણાંક flag_repeat;
	પૂર્ણાંક do_repeat;
	પૂર્ણાંક run_command_flags;
#endif /* __U_BOOT__ */
	અક્ષર *ifs_whitespace; /* = G.ifs or malloced */
#ifndef __U_BOOT__
	અચલ અક્ષર *cwd;
#endif /* !__U_BOOT__ */
	સંરચના variable *ટોચ_var;
	અક્ષર **expanded_assignments;
	સંરચના variable **shadowed_vars_pp;
	unsigned var_nest_level;
#ifndef __U_BOOT__
#જો ENABLE_HUSH_FUNCTIONS
# જો ENABLE_HUSH_LOCAL
	unsigned func_nest_level; /* solely to prevent "local v" in non-functions */
# endif
	સંરચના function *ટોચ_func;
#endif
	/* Signal and trap handling */
#જો ENABLE_HUSH_FAST
	unsigned ગણતરી_SIGCHLD;
	unsigned handled_SIGCHLD;
	smallint we_have_children;
#endif
#જો ENABLE_HUSH_LINENO_VAR
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
#જો ENABLE_HUSH_JOB
	unsigned fatal_sig_mask;
# define G_fatal_sig_mask (G.fatal_sig_mask)
#નહીંતર
# define G_fatal_sig_mask 0
#endif
#જો ENABLE_HUSH_TRAP
	પૂર્ણાંક pre_trap_exitcode;
# જો ENABLE_HUSH_FUNCTIONS
	પૂર્ણાંક return_exitcode;
# endif
	અક્ષર **traps; /* char *traps[NSIG] */
# define G_traps G.traps
#નહીંતર
# define G_traps ((અક્ષર**)ખાલી)
#endif
	sigset_t pending_set;
#જો ENABLE_HUSH_MEMLEAK
	unsigned long memleak_કિંમત;
#endif
#જો ENABLE_HUSH_MODE_X
	unsigned x_mode_depth;
	/* "set -x" output should not be redirectable with subsequent 2>FILE.
	 * We dup fd#2 to x_mode_fd when "set -x" is executed, and use it
	 * for all subsequent output.
	 */
	પૂર્ણાંક x_mode_fd;
	o_string x_mode_buf;
#endif
#endif /* !__U_BOOT__ */
#જો HUSH_DEBUG >= 2
	પૂર્ણાંક debug_indent;
#endif
#ifndef __U_BOOT__
	સંરચના sigaction sa;
	અક્ષર optstring_buf[કદ("eixcs")];
#જો BASH_EPOCH_VARS
	અક્ષર epoch_buf[કદ("%llu.nnnnnn") + કદ(long long)*3];
#endif
#જો ENABLE_FEATURE_EDITING
	અક્ષર user_આગત_buf[CONFIG_FEATURE_EDITING_MAX_LEN];
#endif
#endif /* !__U_BOOT__ */
};
#ifdef __U_BOOT__
સંરચના globals *ptr_to_globals;
#endif /* __U_BOOT__ */
#define G (*ptr_to_globals)
/* Not #defining name to G.name - this quickly gets unwieldy
 * (too many defines). Also, I actually prefer to see when a variable
 * is global, thus "G." prefix is a useful hint */
#ifdef __U_BOOT__
#define SET_PTR_TO_GLOBALS(x) કરો { \
	(*(સંરચના globals**)&ptr_to_globals) = (શૂન્ય*)(x); \
	barrier(); \
} જ્યારે (0)
#define INIT_G() કરો { \
	SET_PTR_TO_GLOBALS(xzalloc(કદ(G))); \
	G.promptmode = 1; \
} જ્યારે (0)
#નહીંતર /* !__U_BOOT__ */
#define INIT_G() કરો { \
	SET_PTR_TO_GLOBALS(xzalloc(કદ(G))); \
	/* memset(&G.sa, 0, sizeof(G.sa)); */  \
	sigfillset(&G.sa.sa_mask); \
	G.sa.sa_flags = SA_RESTART; \
} જ્યારે (0)
#endif /* !__U_BOOT__ */

#ifndef __U_BOOT__
/* Function prototypes for builtins */
સ્થિર પૂર્ણાંક builtin_cd(અક્ષર **argv) FAST_FUNC;
#જો ENABLE_HUSH_ECHO
સ્થિર પૂર્ણાંક builtin_echo(અક્ષર **argv) FAST_FUNC;
#endif
સ્થિર પૂર્ણાંક builtin_eval(અક્ષર **argv) FAST_FUNC;
સ્થિર પૂર્ણાંક builtin_exec(અક્ષર **argv) FAST_FUNC;
સ્થિર પૂર્ણાંક builtin_exit(અક્ષર **argv) FAST_FUNC;
#જો ENABLE_HUSH_EXPORT
સ્થિર પૂર્ણાંક builtin_export(અક્ષર **argv) FAST_FUNC;
#endif
#જો ENABLE_HUSH_READONLY
સ્થિર પૂર્ણાંક builtin_readonly(અક્ષર **argv) FAST_FUNC;
#endif
સ્થિર પૂર્ણાંક builtin_false(અક્ષર **argv) FAST_FUNC;
#જો ENABLE_HUSH_JOB
સ્થિર પૂર્ણાંક builtin_fg_bg(અક્ષર **argv) FAST_FUNC;
સ્થિર પૂર્ણાંક builtin_jobs(અક્ષર **argv) FAST_FUNC;
#endif
#જો ENABLE_HUSH_GETOPTS
સ્થિર પૂર્ણાંક builtin_getopts(અક્ષર **argv) FAST_FUNC;
#endif
#જો ENABLE_HUSH_HELP
સ્થિર પૂર્ણાંક builtin_help(અક્ષર **argv) FAST_FUNC;
#endif
#જો MAX_HISTORY && ENABLE_FEATURE_EDITING
સ્થિર પૂર્ણાંક builtin_history(અક્ષર **argv) FAST_FUNC;
#endif
#જો ENABLE_HUSH_LOCAL
સ્થિર પૂર્ણાંક builtin_local(અક્ષર **argv) FAST_FUNC;
#endif
#જો ENABLE_HUSH_MEMLEAK
સ્થિર પૂર્ણાંક builtin_memleak(અક્ષર **argv) FAST_FUNC;
#endif
#જો ENABLE_HUSH_PRINTF
સ્થિર પૂર્ણાંક builtin_printf(અક્ષર **argv) FAST_FUNC;
#endif
સ્થિર પૂર્ણાંક builtin_pwd(અક્ષર **argv) FAST_FUNC;
#જો ENABLE_HUSH_READ
સ્થિર પૂર્ણાંક builtin_read(અક્ષર **argv) FAST_FUNC;
#endif
#જો ENABLE_HUSH_SET
સ્થિર પૂર્ણાંક builtin_set(અક્ષર **argv) FAST_FUNC;
#endif
સ્થિર પૂર્ણાંક builtin_shift(અક્ષર **argv) FAST_FUNC;
સ્થિર પૂર્ણાંક builtin_source(અક્ષર **argv) FAST_FUNC;
#જો ENABLE_HUSH_TEST || BASH_TEST2
સ્થિર પૂર્ણાંક builtin_test(અક્ષર **argv) FAST_FUNC;
#endif
#જો ENABLE_HUSH_TRAP
સ્થિર પૂર્ણાંક builtin_trap(અક્ષર **argv) FAST_FUNC;
#endif
#જો ENABLE_HUSH_TYPE
સ્થિર પૂર્ણાંક builtin_type(અક્ષર **argv) FAST_FUNC;
#endif
#જો ENABLE_HUSH_TIMES
સ્થિર પૂર્ણાંક builtin_times(અક્ષર **argv) FAST_FUNC;
#endif
સ્થિર પૂર્ણાંક builtin_true(અક્ષર **argv) FAST_FUNC;
#જો ENABLE_HUSH_UMASK
સ્થિર પૂર્ણાંક builtin_umask(અક્ષર **argv) FAST_FUNC;
#endif
#જો ENABLE_HUSH_UNSET
સ્થિર પૂર્ણાંક builtin_unset(અક્ષર **argv) FAST_FUNC;
#endif
#જો ENABLE_HUSH_KILL
સ્થિર પૂર્ણાંક builtin_kill(અક્ષર **argv) FAST_FUNC;
#endif
#જો ENABLE_HUSH_WAIT
સ્થિર પૂર્ણાંક builtin_wait(અક્ષર **argv) FAST_FUNC;
#endif
#જો ENABLE_HUSH_LOOPS
સ્થિર પૂર્ણાંક builtin_break(અક્ષર **argv) FAST_FUNC;
સ્થિર પૂર્ણાંક builtin_continue(અક્ષર **argv) FAST_FUNC;
#endif
#જો ENABLE_HUSH_FUNCTIONS
સ્થિર પૂર્ણાંક builtin_return(અક્ષર **argv) FAST_FUNC;
#endif

/* Table of built-in functions.  They can be forked or not, depending on
 * context: within pipes, they fork.  As simple commands, they do not.
 * When used in non-forking context, they can change global variables
 * in the parent shell process.  If forked, of course they cannot.
 * For example, 'unset foo | whatever' will parse and run, but foo will
 * still be set at the end. */
સંરચના built_in_command {
	અચલ અક્ષર *b_cmd;
	પૂર્ણાંક (*b_function)(અક્ષર **argv) FAST_FUNC;
#જો ENABLE_HUSH_HELP
	અચલ અક્ષર *b_descr;
# define BLTIN(cmd, func, help) { cmd, func, help }
#નહીંતર
# define BLTIN(cmd, func, help) { cmd, func }
#endif
};

સ્થિર અચલ સંરચના built_in_command bltins1[] ALIGN_PTR = {
	BLTIN("."        , builtin_source  , "Run commands in file"),
	BLTIN(":"        , builtin_true    , ખાલી),
#જો ENABLE_HUSH_JOB
	BLTIN("bg"       , builtin_fg_bg   , "Resume job in background"),
#endif
#જો ENABLE_HUSH_LOOPS
	BLTIN("break"    , builtin_break   , "Exit loop"),
#endif
	BLTIN("cd"       , builtin_cd      , "Change directory"),
#જો ENABLE_HUSH_LOOPS
	BLTIN("continue" , builtin_continue, "Start new loop iteration"),
#endif
	BLTIN("eval"     , builtin_eval    , "Construct and run shell command"),
	BLTIN("exec"     , builtin_exec    , "Execute command, don't return to shell"),
	BLTIN("exit"     , builtin_exit    , ખાલી),
#જો ENABLE_HUSH_EXPORT
	BLTIN("export"   , builtin_export  , "Set environment variables"),
#endif
	BLTIN("false"    , builtin_false   , ખાલી),
#જો ENABLE_HUSH_JOB
	BLTIN("fg"       , builtin_fg_bg   , "Bring job to foreground"),
#endif
#જો ENABLE_HUSH_GETOPTS
	BLTIN("getopts"  , builtin_getopts , ખાલી),
#endif
#જો ENABLE_HUSH_HELP
	BLTIN("help"     , builtin_help    , ખાલી),
#endif
#જો MAX_HISTORY && ENABLE_FEATURE_EDITING
	BLTIN("history"  , builtin_history , "Show history"),
#endif
#જો ENABLE_HUSH_JOB
	BLTIN("jobs"     , builtin_jobs    , "List jobs"),
#endif
#જો ENABLE_HUSH_KILL
	BLTIN("kill"     , builtin_kill    , "Send signals to processes"),
#endif
#જો ENABLE_HUSH_LOCAL
	BLTIN("local"    , builtin_local   , "Set local variables"),
#endif
#જો ENABLE_HUSH_MEMLEAK
	BLTIN("memleak"  , builtin_memleak , ખાલી),
#endif
#જો ENABLE_HUSH_READ
	BLTIN("read"     , builtin_read    , "Input into variable"),
#endif
#જો ENABLE_HUSH_READONLY
	BLTIN("readonly" , builtin_readonly, "Make variables read-only"),
#endif
#જો ENABLE_HUSH_FUNCTIONS
	BLTIN("return"   , builtin_return  , "Return from function"),
#endif
#જો ENABLE_HUSH_SET
	BLTIN("set"      , builtin_set     , "Set positional parameters"),
#endif
	BLTIN("shift"    , builtin_shift   , "Shift positional parameters"),
#જો BASH_SOURCE
	BLTIN("source"   , builtin_source  , ખાલી),
#endif
#જો ENABLE_HUSH_TIMES
	BLTIN("times"    , builtin_times   , ખાલી),
#endif
#જો ENABLE_HUSH_TRAP
	BLTIN("trap"     , builtin_trap    , "Trap signals"),
#endif
	BLTIN("true"     , builtin_true    , ખાલી),
#જો ENABLE_HUSH_TYPE
	BLTIN("type"     , builtin_type    , "Show command type"),
#endif
#જો ENABLE_HUSH_ULIMIT
	BLTIN("ulimit"   , shell_builtin_ulimit, "Control resource limits"),
#endif
#જો ENABLE_HUSH_UMASK
	BLTIN("umask"    , builtin_umask   , "Set file creation mask"),
#endif
#જો ENABLE_HUSH_UNSET
	BLTIN("unset"    , builtin_unset   , "Unset variables"),
#endif
#જો ENABLE_HUSH_WAIT
	BLTIN("wait"     , builtin_wait    , "Wait for process to finish"),
#endif
};
/* These builtins won't be used if we are on NOMMU and need to re-exec
 * (it's cheaper to run an external program in this case):
 */
સ્થિર અચલ સંરચના built_in_command bltins2[] ALIGN_PTR = {
#જો ENABLE_HUSH_TEST
	BLTIN("["        , builtin_test    , ખાલી),
#endif
#જો BASH_TEST2
	BLTIN("[["       , builtin_test    , ખાલી),
#endif
#જો ENABLE_HUSH_ECHO
	BLTIN("echo"     , builtin_echo    , ખાલી),
#endif
#જો ENABLE_HUSH_PRINTF
	BLTIN("printf"   , builtin_printf  , ખાલી),
#endif
	BLTIN("pwd"      , builtin_pwd     , ખાલી),
#જો ENABLE_HUSH_TEST
	BLTIN("test"     , builtin_test    , ખાલી),
#endif
};

#endif /* !__U_BOOT__ */

/* Debug printouts.
 */
#જો HUSH_DEBUG >= 2
/* prevent disasters with G.debug_indent < 0 */
# define indent() fdprintf(2, "%*s", (G.debug_indent * 2) & 0xff, "")
# define debug_enter() (G.debug_indent++)
# define debug_leave() (G.debug_indent--)
#નહીંતર
# define indent()      ((શૂન્ય)0)
# define debug_enter() ((શૂન્ય)0)
# define debug_leave() ((શૂન્ય)0)
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
#નહીંતર
# define DEBUG_JOBS 0
#endif

#ifndef debug_printf_expand
# define debug_printf_expand(...) (indent(), fdprintf(2, __VA_ARGS__))
# define DEBUG_EXPAND 1
#નહીંતર
# define DEBUG_EXPAND 0
#endif

#ifndef debug_printf_varexp
# define debug_printf_varexp(...) (indent(), fdprintf(2, __VA_ARGS__))
#endif

#ifndef debug_printf_glob
# define debug_printf_glob(...) (indent(), fdprintf(2, __VA_ARGS__))
# define DEBUG_GLOB 1
#નહીંતર
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
#નહીંતર
# define DEBUG_CLEAN 0
#endif

#જો DEBUG_EXPAND
સ્થિર શૂન્ય debug_print_strings(અચલ અક્ષર *prefix, અક્ષર **vv)
{
	indent();
	fdprintf(2, "%s:\n", prefix);
	જ્યારે (*vv)
		fdprintf(2, " '%s'\n", *vv++);
}
#નહીંતર
# define debug_print_strings(prefix, vv) ((શૂન્ય)0)
#endif

/* Leak hunting. Use hush_leaktool.sh for post-processing.
 */
#જો LEAK_HUNTING
સ્થિર શૂન્ય *xxmalloc(પૂર્ણાંક lineno, size_t size)
{
	શૂન્ય *ptr = xmalloc((size + 0xff) & ~0xff);
	fdprintf(2, "line %d: malloc %p\n", lineno, ptr);
	પાછા ptr;
}
સ્થિર શૂન્ય *xxrealloc(પૂર્ણાંક lineno, શૂન્ય *ptr, size_t size)
{
	ptr = xrealloc(ptr, (size + 0xff) & ~0xff);
	fdprintf(2, "line %d: realloc %p\n", lineno, ptr);
	પાછા ptr;
}
સ્થિર અક્ષર *xxstrdup(પૂર્ણાંક lineno, અચલ અક્ષર *str)
{
	અક્ષર *ptr = xstrdup(str);
	fdprintf(2, "line %d: strdup %p\n", lineno, ptr);
	પાછા ptr;
}
સ્થિર શૂન્ય xxfree(શૂન્ય *ptr)
{
	fdprintf(2, "free %p\n", ptr);
	મુક્ત(ptr);
}
# define xmalloc(s)     xxmalloc(__LINE__, s)
# define xrealloc(p, s) xxrealloc(__LINE__, p, s)
# define xstrdup(s)     xxstrdup(__LINE__, s)
# define મુક્ત(p)        xxfree(p)
#endif

/* Syntax and runtime errors. They always abort scripts.
 * In interactive use they usually discard unparsed and/or unexecuted commands
 * and return to the prompt.
 * HUSH_DEBUG >= 2 prints line number in this file where it was detected.
 */
#જો HUSH_DEBUG < 2
# define સંદેશ_and_die_if_script(lineno, ...)     સંદેશ_and_die_if_script(__VA_ARGS__)
# define syntax_error(lineno, સંદેશ)              syntax_error(સંદેશ)
# define syntax_error_at(lineno, સંદેશ)           syntax_error_at(સંદેશ)
# define syntax_error_unterm_ch(lineno, ch)     syntax_error_unterm_ch(ch)
# define syntax_error_unterm_str(lineno, s)     syntax_error_unterm_str(s)
# define syntax_error_unexpected_ch(lineno, ch) syntax_error_unexpected_ch(ch)
#endif

સ્થિર શૂન્ય die_if_script(શૂન્ય)
{
	જો (!G_interactive_fd) {
		જો (G.last_exitcode) /* sometines it's 2, not 1 (bash compat) */
			xfunc_error_retval = G.last_exitcode;
		xfunc_die();
	}
}

#ifdef __U_BOOT__
સ્થિર શૂન્ય __maybe_unused સંદેશ_and_die_if_script(unsigned lineno, અચલ અક્ષર *fmt, ...)
#નહીંતર /* !__U_BOOT__ */
સ્થિર શૂન્ય સંદેશ_and_die_if_script(unsigned lineno, અચલ અક્ષર *fmt, ...)
#endif /* !__U_BOOT__ */
{
	va_list p;

#જો HUSH_DEBUG >= 2
	bb_error_સંદેશ("hush.c:%u", lineno);
#endif
	va_start(p, fmt);
	bb_verror_સંદેશ(fmt, p, ખાલી);
	va_end(p);
	die_if_script();
}

સ્થિર શૂન્ય syntax_error(unsigned lineno UNUSED_PARAM, અચલ અક્ષર *સંદેશ)
{
	જો (સંદેશ)
		bb_error_સંદેશ("syntax error: %s", સંદેશ);
	નહીંતર
		bb_simple_error_સંદેશ("syntax error");
	die_if_script();
}

સ્થિર શૂન્ય syntax_error_at(unsigned lineno UNUSED_PARAM, અચલ અક્ષર *સંદેશ)
{
	bb_error_સંદેશ("syntax error at '%s'", સંદેશ);
	die_if_script();
}

સ્થિર શૂન્ય syntax_error_unterm_str(unsigned lineno UNUSED_PARAM, અચલ અક્ષર *s)
{
	bb_error_સંદેશ("syntax error: unterminated %s", s);
//? source4.tests fails: in bash, echo ${^} in script does not terminate the script
// (but bash --posix, or if bash is run as "sh", does terminate in script, so maybe uncomment this?)
//	die_if_script();
}

સ્થિર શૂન્ય syntax_error_unterm_ch(unsigned lineno, અક્ષર ch)
{
	અક્ષર સંદેશ[2] = { ch, '\0' };
	syntax_error_unterm_str(lineno, સંદેશ);
}

સ્થિર શૂન્ય syntax_error_unexpected_ch(unsigned lineno UNUSED_PARAM, પૂર્ણાંક ch)
{
	અક્ષર સંદેશ[2];
	સંદેશ[0] = ch;
	સંદેશ[1] = '\0';
#જો HUSH_DEBUG >= 2
	bb_error_સંદેશ("hush.c:%u", lineno);
#endif
	bb_error_સંદેશ("syntax error: unexpected %s", ch == EOF ? "EOF" : સંદેશ);
	die_if_script();
}

#જો HUSH_DEBUG < 2
# undef સંદેશ_and_die_if_script
# undef syntax_error
# undef syntax_error_at
# undef syntax_error_unterm_ch
# undef syntax_error_unterm_str
# undef syntax_error_unexpected_ch
#નહીંતર
# define સંદેશ_and_die_if_script(...)     સંદેશ_and_die_if_script(__LINE__, __VA_ARGS__)
# define syntax_error(સંદેશ)              syntax_error(__LINE__, સંદેશ)
# define syntax_error_at(સંદેશ)           syntax_error_at(__LINE__, સંદેશ)
# define syntax_error_unterm_ch(ch)     syntax_error_unterm_ch(__LINE__, ch)
# define syntax_error_unterm_str(s)     syntax_error_unterm_str(__LINE__, s)
# define syntax_error_unexpected_ch(ch) syntax_error_unexpected_ch(__LINE__, ch)
#endif

/* Utility functions
 */
/* Replace each \x with x in place, return ptr past NUL. */
સ્થિર અક્ષર *unbackslash(અક્ષર *src)
{
#ifdef __U_BOOT__
	અક્ષર *dst = src = (અક્ષર *)strchrnul(src, '\\');
#નહીંતર /* !__U_BOOT__ */
	અક્ષર *dst = src = strchrnul(src, '\\');
#endif /* !__U_BOOT__ */
	જ્યારે (1) {
		જો (*src == '\\') {
			src++;
			જો (*src != '\0') {
				/* \x -> x */
				*dst++ = *src++;
				ચાલુ;
			}
			/* else: "\<nul>". Do not delete this backslash.
			 * Testcase: eval 'echo ok\'
			 */
			*dst++ = '\\';
			/* fallthrough */
		}
		જો ((*dst++ = *src++) == '\0')
			તોડો;
	}
	પાછા dst;
}

સ્થિર અક્ષર **add_strings_to_strings(અક્ષર **strings, અક્ષર **add, પૂર્ણાંક need_to_dup)
{
	પૂર્ણાંક i;
	unsigned count1;
	unsigned count2;
	અક્ષર **v;

	v = strings;
	count1 = 0;
	જો (v) {
		જ્યારે (*v) {
			count1++;
			v++;
		}
	}
	count2 = 0;
	v = add;
	જ્યારે (*v) {
		count2++;
		v++;
	}
	v = xrealloc(strings, (count1 + count2 + 1) * કદ(અક્ષર*));
	v[count1 + count2] = ખાલી;
	i = count2;
	જ્યારે (--i >= 0)
		v[count1 + i] = (need_to_dup ? xstrdup(add[i]) : add[i]);
	પાછા v;
}
#જો LEAK_HUNTING
સ્થિર અક્ષર **xx_add_strings_to_strings(પૂર્ણાંક lineno, અક્ષર **strings, અક્ષર **add, પૂર્ણાંક need_to_dup)
{
	અક્ષર **ptr = add_strings_to_strings(strings, add, need_to_dup);
	fdprintf(2, "line %d: add_strings_to_strings %p\n", lineno, ptr);
	પાછા ptr;
}
#define add_strings_to_strings(strings, add, need_to_dup) \
	xx_add_strings_to_strings(__LINE__, strings, add, need_to_dup)
#endif

/* Note: takes ownership of "add" ptr (it is not strdup'ed) */
સ્થિર અક્ષર **add_string_to_strings(અક્ષર **strings, અક્ષર *add)
{
	અક્ષર *v[2];
	v[0] = add;
	v[1] = ખાલી;
	પાછા add_strings_to_strings(strings, v, /*dup:*/ 0);
}

#જો LEAK_HUNTING
સ્થિર અક્ષર **xx_add_string_to_strings(પૂર્ણાંક lineno, અક્ષર **strings, અક્ષર *add)
{
	અક્ષર **ptr = add_string_to_strings(strings, add);
	fdprintf(2, "line %d: add_string_to_strings %p\n", lineno, ptr);
	પાછા ptr;
}
#define add_string_to_strings(strings, add) \
	xx_add_string_to_strings(__LINE__, strings, add)
#endif

સ્થિર શૂન્ય free_strings(અક્ષર **strings)
{
	અક્ષર **v;

	જો (!strings)
		પાછા;
	v = strings;
	જ્યારે (*v) {
		મુક્ત(*v);
		v++;
	}
	મુક્ત(strings);
}

#ifndef __U_BOOT__
સ્થિર પૂર્ણાંક dup_CLOEXEC(પૂર્ણાંક fd, પૂર્ણાંક avoid_fd)
{
	પૂર્ણાંક newfd;
 repeat:
	newfd = fcntl(fd, F_DUPFD_CLOEXEC, avoid_fd + 1);
	જો (newfd >= 0) {
		જો (F_DUPFD_CLOEXEC == F_DUPFD) /* if old libc (w/o F_DUPFD_CLOEXEC) */
			close_on_exec_on(newfd);
	} નહીંતર { /* newfd < 0 */
		જો (errno == EBUSY)
			જાઓ repeat;
		જો (errno == EINTR)
			જાઓ repeat;
		જો (errno != EBADF) {
			/* "echo >&9999" gets EINVAL trying to save fd 1 to above 9999.
			 * We could try saving it _below_ 9999 instead (how?), but
			 * this probably means that dup2(9999,1) to effectuate >&9999
			 * would also not work: fd 9999 can't exist.
			 * (This differs from "echo >&99" where saving works, but
			 * subsequent dup2(99,1) fails if fd 99 is not open).
			 */
			bb_perror_સંદેશ("fcntl(%d,F_DUPFD,%d)", fd, avoid_fd + 1);
		}
	}
	પાછા newfd;
}

સ્થિર પૂર્ણાંક xdup_CLOEXEC_and_close(પૂર્ણાંક fd, પૂર્ણાંક avoid_fd)
{
	પૂર્ણાંક newfd;
 repeat:
	newfd = fcntl(fd, F_DUPFD_CLOEXEC, avoid_fd + 1);
	જો (newfd < 0) {
		જો (errno == EBUSY)
			જાઓ repeat;
		જો (errno == EINTR)
			જાઓ repeat;
		/* fd was not open? */
		જો (errno == EBADF)
			પાછા fd;
		xfunc_die();
	}
	જો (F_DUPFD_CLOEXEC == F_DUPFD) /* if old libc (w/o F_DUPFD_CLOEXEC) */
		close_on_exec_on(newfd);
	close(fd);
	પાછા newfd;
}

/* Manipulating HFILEs */
સ્થિર HFILE *hfopen(અચલ અક્ષર *name)
{
	HFILE *fp;
	પૂર્ણાંક fd;

	fd = STDIN_FILENO;
	જો (name) {
		fd = open(name, O_RDONLY | O_CLOEXEC);
		જો (fd < 0)
			પાછા ખાલી;
		જો (O_CLOEXEC == 0) /* ancient libc */
			close_on_exec_on(fd);
	}

	fp = xmalloc(કદ(*fp));
	જો (name == ખાલી)
		G.HFILE_stdin = fp;
	fp->fd = fd;
	fp->cur = fp->end = fp->buf;
	fp->next_hfile = G.HFILE_list;
	G.HFILE_list = fp;
	પાછા fp;
}
સ્થિર શૂન્ય hfclose(HFILE *fp)
{
	HFILE **pp = &G.HFILE_list;
	જ્યારે (*pp) {
		HFILE *cur = *pp;
		જો (cur == fp) {
			*pp = cur->next_hfile;
			તોડો;
		}
		pp = &cur->next_hfile;
	}
	જો (fp->fd >= 0)
		close(fp->fd);
	મુક્ત(fp);
}
સ્થિર પૂર્ણાંક refill_HFILE_and_getc(HFILE *fp)
{
	પૂર્ણાંક n;

	જો (fp->fd < 0) {
		/* Already saw EOF */
		પાછા EOF;
	}
#જો ENABLE_HUSH_INTERACTIVE && !ENABLE_FEATURE_EDITING
	/* If user presses ^C, read() restarts after SIGINT (we use SA_RESTART).
	 * IOW: ^C will not immediately stop line input.
	 * But poll() is different: it does NOT restart after signals.
	 */
	જો (fp == G.HFILE_stdin) {
		સંરચના pollfd pfd[1];
		pfd[0].fd = fp->fd;
		pfd[0].events = POLLIN;
		n = poll(pfd, 1, -1);
		જો (n < 0
		 /*&& errno == EINTR - assumed true */
		 && sigismember(&G.pending_set, SIGINT)
		) {
			પાછા '\0';
		}
	}
#નહીંતર
/* if FEATURE_EDITING=y, we do not use this routine for interactive input */
#endif
	/* Try to buffer more input */
	n = safe_read(fp->fd, fp->buf, કદ(fp->buf));
	જો (n < 0) {
		bb_simple_perror_સંદેશ("read error");
		n = 0;
	}
	fp->cur = fp->buf;
	fp->end = fp->buf + n;
	જો (n == 0) {
		/* EOF/error */
		close(fp->fd);
		fp->fd = -1;
		પાછા EOF;
	}
	પાછા (unsigned અક્ષર)(*fp->cur++);
}
/* Inlined for common case of non-empty buffer.
 */
સ્થિર ALWAYS_INLINE પૂર્ણાંક hfgetc(HFILE *fp)
{
	જો (fp->cur < fp->end)
		પાછા (unsigned અક્ષર)(*fp->cur++);
	/* Buffer empty */
	પાછા refill_HFILE_and_getc(fp);
}
સ્થિર પૂર્ણાંક move_HFILEs_on_redirect(પૂર્ણાંક fd, પૂર્ણાંક avoid_fd)
{
	HFILE *fl = G.HFILE_list;
	જ્યારે (fl) {
		જો (fd == fl->fd) {
			/* We use it only on script files, they are all CLOEXEC */
			fl->fd = xdup_CLOEXEC_and_close(fd, avoid_fd);
			debug_printf_redir("redirect_fd %d: matches a script fd, moving it to %d\n", fd, fl->fd);
			પાછા 1; /* "found and moved" */
		}
		fl = fl->next_hfile;
	}
#જો ENABLE_HUSH_MODE_X
	જો (G.x_mode_fd > 0 && fd == G.x_mode_fd) {
		G.x_mode_fd = xdup_CLOEXEC_and_close(fd, avoid_fd);
		પાછા 1; /* "found and moved" */
	}
#endif
	પાછા 0; /* "not in the list" */
}
#જો ENABLE_FEATURE_SH_STANDALONE && BB_MMU
સ્થિર શૂન્ય close_all_HFILE_list(શૂન્ય)
{
	HFILE *fl = G.HFILE_list;
	જ્યારે (fl) {
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
		જો (fl->fd > 0)
			/*hfclose(fl); - unsafe */
			close(fl->fd);
		fl = fl->next_hfile;
	}
}
#endif
સ્થિર પૂર્ણાંક fd_in_HFILEs(પૂર્ણાંક fd)
{
	HFILE *fl = G.HFILE_list;
	જ્યારે (fl) {
		જો (fl->fd == fd)
			પાછા 1;
		fl = fl->next_hfile;
	}
	પાછા 0;
}

#endif /* !__U_BOOT__ */

/* Helpers for setting new $n and restoring them back
 */
પ્રકારવ્યાખ્યા સંરચના save_arg_t {
	અક્ષર *sv_argv0;
	અક્ષર **sv_g_argv;
	પૂર્ણાંક sv_g_argc;
#ifndef __U_BOOT__
	IF_HUSH_SET(smallint sv_g_malloced;)
#endif /* !__U_BOOT__ */
} save_arg_t;

#ifndef __U_BOOT__
સ્થિર શૂન્ય save_and_replace_G_args(save_arg_t *sv, અક્ષર **argv)
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

સ્થિર શૂન્ય restore_G_args(save_arg_t *sv, અક્ષર **argv)
{
#જો ENABLE_HUSH_SET
	જો (G.global_args_malloced) {
		/* someone ran "set -- arg1 arg2 ...", undo */
		અક્ષર **pp = G.global_argv;
		જ્યારે (*++pp) /* note: does not free $0 */
			મુક્ત(*pp);
		મુક્ત(G.global_argv);
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
ગણના {
	SPECIAL_INTERACTIVE_SIGS = 0
		| (1 << SIGTERM)
		| (1 << SIGINT)
		| (1 << SIGHUP)
		,
	SPECIAL_JOBSTOP_SIGS = 0
#જો ENABLE_HUSH_JOB
		| (1 << SIGTTIN)
		| (1 << SIGTTOU)
		| (1 << SIGTSTP)
#endif
		,
};

સ્થિર શૂન્ય record_pending_signo(પૂર્ણાંક sig)
{
	sigaddset(&G.pending_set, sig);
#જો ENABLE_FEATURE_EDITING
	જો (sig != SIGCHLD
	 || (G_traps && G_traps[SIGCHLD] && G_traps[SIGCHLD][0])
	 /* ^^^ if SIGCHLD, interrupt line reading only if it has a trap */
	) {
		bb_got_signal = sig; /* for read_line_input: "we got a signal" */
	}
#endif
#જો ENABLE_HUSH_FAST
	જો (sig == SIGCHLD) {
		G.ગણતરી_SIGCHLD++;
//bb_error_msg("[%d] SIGCHLD_handler: G.count_SIGCHLD:%d G.handled_SIGCHLD:%d", getpid(), G.count_SIGCHLD, G.handled_SIGCHLD);
	}
#endif
}

સ્થિર sighandler_t install_sighandler(પૂર્ણાંક sig, sighandler_t handler)
{
	સંરચના sigaction old_sa;

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
	પાછા old_sa.sa_handler;
}
#endif /* !__U_BOOT__ */

#ifndef __U_BOOT__
સ્થિર શૂન્ય hush_exit(પૂર્ણાંક exitcode) NORETURN;

સ્થિર શૂન્ય restore_ttypgrp_and__exit(શૂન્ય) NORETURN;
સ્થિર શૂન્ય restore_ttypgrp_and__exit(શૂન્ય)
{
	/* xfunc has failed! die die die */
	/* no EXIT traps, this is an escape hatch! */
	G.exiting = 1;
	hush_exit(xfunc_error_retval);
}

#જો ENABLE_HUSH_JOB

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
સ્થિર શૂન્ય fflush_and__exit(શૂન્ય) NORETURN;
સ્થિર શૂન્ય fflush_and__exit(શૂન્ય)
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
સ્થિર શૂન્ય sigexit(પૂર્ણાંક sig) NORETURN;
સ્થિર શૂન્ય sigexit(પૂર્ણાંક sig)
{
	/* Careful: we can end up here after [v]fork. Do not restore
	 * tty pgrp then, only top-level shell process does that */
	જો (G_saved_tty_pgrp && getpid() == G.મૂળ_pid) {
		/* Disable all signals: job control, SIGPIPE, etc.
		 * Mostly paranoid measure, to prevent infinite SIGTTOU.
		 */
		sigprocmask_allsigs(SIG_BLOCK);
		tcsetpgrp(G_interactive_fd, G_saved_tty_pgrp);
	}

	/* Not a signal, just exit */
	જો (sig <= 0)
		_exit(- sig);

	kill_myself_with_sig(sig); /* does not return */
}
#નહીંતર

# define disable_restore_tty_pgrp_on_exit() ((શૂન્ય)0)
# define enable_restore_tty_pgrp_on_exit()  ((શૂન્ય)0)

#endif
#endif /* !__U_BOOT__ */

#ifndef __U_BOOT__
સ્થિર sighandler_t pick_sighandler(unsigned sig)
{
	sighandler_t handler = SIG_DFL;
	જો (sig < કદ(unsigned)*8) {
		unsigned sigmask = (1 << sig);

#જો ENABLE_HUSH_JOB
		/* is sig fatal? */
		જો (G_fatal_sig_mask & sigmask)
			handler = sigexit;
		નહીંતર
#endif
		/* sig has special handling? */
		જો (G.special_sig_mask & sigmask) {
			handler = record_pending_signo;
			/* TTIN/TTOU/TSTP can't be set to record_pending_signo
			 * in order to ignore them: they will be raised
			 * in an endless loop when we try to do some
			 * terminal ioctls! We do have to _ignore_ these.
			 */
			જો (SPECIAL_JOBSTOP_SIGS & sigmask)
				handler = SIG_IGN;
		}
	}
	પાછા handler;
}
#endif /* !__U_BOOT__ */

#ifndef __U_BOOT__
/* Restores tty foreground process group, and exits. */
સ્થિર શૂન્ય hush_exit(પૂર્ણાંક exitcode)
{
#જો ENABLE_FEATURE_EDITING_SAVE_ON_EXIT
	save_history(G.લીટી_આગત_state); /* may be NULL */
#endif

	fflush_all();
	જો (G.exiting <= 0 && G_traps && G_traps[0] && G_traps[0][0]) {
		અક્ષર *argv[3];
		/* argv[0] is unused */
		argv[1] = xstrdup(G_traps[0]); /* copy, since EXIT trap handler may modify G_traps[0] */
		argv[2] = ખાલી;
		G.exiting = 1; /* prevent EXIT trap recursion */
		/* Note: G_traps[0] is not cleared!
		 * "trap" will still show it, if executed
		 * in the handler */
		builtin_eval(argv);
	}

#જો ENABLE_FEATURE_CLEAN_UP
	{
		સંરચના variable *cur_var;
		જો (G.cwd != bb_સંદેશ_unknown)
			મુક્ત((અક્ષર*)G.cwd);
		cur_var = G.ટોચ_var;
		જ્યારે (cur_var) {
			સંરચના variable *tmp = cur_var;
			જો (!cur_var->max_len)
				મુક્ત(cur_var->varstr);
			cur_var = cur_var->next;
			મુક્ત(tmp);
		}
	}
#endif

	fflush_all();
#જો ENABLE_HUSH_JOB
	sigexit(- (exitcode & 0xff));
#નહીંતર
	_exit(exitcode);
#endif
}

//TODO: return a mask of ALL handled sigs?
સ્થિર પૂર્ણાંક check_and_run_traps(શૂન્ય)
{
	પૂર્ણાંક last_sig = 0;

	જ્યારે (1) {
		પૂર્ણાંક sig;

		જો (sigisemptyset(&G.pending_set))
			તોડો;
		sig = 0;
		કરો {
			sig++;
			જો (sigismember(&G.pending_set, sig)) {
				sigdelset(&G.pending_set, sig);
				જાઓ got_sig;
			}
		} જ્યારે (sig < NSIG);
		તોડો;
 got_sig:
#જો ENABLE_HUSH_TRAP
		જો (G_traps && G_traps[sig]) {
			debug_printf_exec("%s: sig:%d handler:'%s'\n", __func__, sig, G.traps[sig]);
			જો (G_traps[sig][0]) {
				/* We have user-defined handler */
				smalluint save_rcode;
				પૂર્ણાંક save_pre;
				અક્ષર *argv[3];
				/* argv[0] is unused */
				argv[1] = xstrdup(G_traps[sig]);
				/* why strdup? trap can modify itself: trap 'trap "echo oops" INT' INT */
				argv[2] = ખાલી;
				save_pre = G.pre_trap_exitcode;
				G.pre_trap_exitcode = save_rcode = G.last_exitcode;
				builtin_eval(argv);
				મુક્ત(argv[1]);
				G.pre_trap_exitcode = save_pre;
				G.last_exitcode = save_rcode;
# જો ENABLE_HUSH_FUNCTIONS
				જો (G.return_exitcode >= 0) {
					debug_printf_exec("trap exitcode:%d\n", G.return_exitcode);
					G.last_exitcode = G.return_exitcode;
				}
# endif
				last_sig = sig;
			} /* else: "" trap, ignoring signal */
			ચાલુ;
		}
#endif
		/* not a trap: special action */
		પસંદગી (sig) {
		સ્થિતિ SIGINT:
			debug_printf_exec("%s: sig:%d default SIGINT handler\n", __func__, sig);
			G.flag_SIGINT = 1;
			last_sig = sig;
			તોડો;
#જો ENABLE_HUSH_JOB
		સ્થિતિ SIGHUP: {
			/* if (G_interactive_fd) - no need to check, the handler
			 * is only installed if we *are* interactive */
			{
				/* bash compat: "Before exiting, an interactive
				 * shell resends the SIGHUP to all jobs, running
				 * or stopped.  Stopped jobs are sent SIGCONT
				 * to ensure that they receive the SIGHUP."
				 */
				સંરચના pipe *job;
				debug_printf_exec("%s: sig:%d default SIGHUP handler\n", __func__, sig);
				/* bash is observed to signal whole process groups,
				 * not individual processes */
				માટે (job = G.job_list; job; job = job->next) {
					જો (job->pgrp <= 0)
						ચાલુ;
					debug_printf_exec("HUPing pgrp %d\n", job->pgrp);
					જો (kill(- job->pgrp, SIGHUP) == 0)
						kill(- job->pgrp, SIGCONT);
				}
			}
			/* this restores tty pgrp, then kills us with SIGHUP */
			sigexit(SIGHUP);
		}
#endif
#જો ENABLE_HUSH_FAST
		સ્થિતિ SIGCHLD:
			debug_printf_exec("%s: sig:%d default SIGCHLD handler\n", __func__, sig);
			G.ગણતરી_SIGCHLD++;
//bb_error_msg("[%d] check_and_run_traps: G.count_SIGCHLD:%d G.handled_SIGCHLD:%d", getpid(), G.count_SIGCHLD, G.handled_SIGCHLD);
			/* Note:
			 * We don't do 'last_sig = sig' here -> NOT returning this sig.
			 * This simplifies wait builtin a bit.
			 */
			તોડો;
#endif
		મૂળભૂત: /* ignored: */
			debug_printf_exec("%s: sig:%d default handling is to ignore\n", __func__, sig);
			/* SIGTERM, SIGQUIT, SIGTTIN, SIGTTOU, SIGTSTP */
			/* Note:
			 * We don't do 'last_sig = sig' here -> NOT returning this sig.
			 * Example: wait is not interrupted by TERM
			 * in interactive shell, because TERM is ignored.
			 */
			તોડો;
		}
	}
	પાછા last_sig;
}

સ્થિર અચલ અક્ષર *get_cwd(પૂર્ણાંક force)
{
	જો (force || G.cwd == ખાલી) {
		/* xrealloc_getcwd_or_warn(arg) calls free(arg),
		 * we must not try to free(bb_msg_unknown) */
		જો (G.cwd == bb_સંદેશ_unknown)
			G.cwd = ખાલી;
		G.cwd = xrealloc_getcwd_or_warn((અક્ષર *)G.cwd);
		જો (!G.cwd)
			G.cwd = bb_સંદેશ_unknown;
	}
	પાછા G.cwd;
}

#endif /* !__U_BOOT__ */

/*
 * Shell and environment variable support
 */
સ્થિર સંરચના variable **get_ptr_to_local_var(અચલ અક્ષર *name)
{
	સંરચના variable **pp;
	સંરચના variable *cur;

	pp = &G.ટોચ_var;
	જ્યારે ((cur = *pp) != ખાલી) {
		જો (varcmp(cur->varstr, name) == 0)
			પાછા pp;
		pp = &cur->next;
	}
	પાછા ખાલી;
}

સ્થિર અચલ અક્ષર* FAST_FUNC get_local_var_કિંમત(અચલ અક્ષર *name)
{
	સંરચના variable **vpp;

	જો (G.expanded_assignments) {
		અક્ષર **cpp = G.expanded_assignments;
		જ્યારે (*cpp) {
			અક્ષર *cp = *cpp;
			જો (varcmp(cp, name) == 0)
				પાછા strchr(cp, '=') + 1;
			cpp++;
		}
	}

	vpp = get_ptr_to_local_var(name);
	જો (vpp)
		પાછા strchr((*vpp)->varstr, '=') + 1;

#ifndef __U_BOOT__
	જો (strcmp(name, "PPID") == 0)
		પાછા utoa(G.મૂળ_ppid);
#endif /* !__U_BOOT__ */
	// bash compat: UID? EUID?
#જો ENABLE_HUSH_RANDOM_SUPPORT
	જો (strcmp(name, "RANDOM") == 0)
		પાછા utoa(next_random(&G.random_gen));
#endif
#જો ENABLE_HUSH_LINENO_VAR
	જો (strcmp(name, "LINENO") == 0)
		પાછા utoa(G.execute_lineno);
#endif
#જો BASH_EPOCH_VARS
	{
		અચલ અક્ષર *fmt = ખાલી;
		જો (strcmp(name, "EPOCHSECONDS") == 0)
			fmt = "%llu";
		નહીંતર જો (strcmp(name, "EPOCHREALTIME") == 0)
			fmt = "%llu.%06u";
		જો (fmt) {
			સંરચના timeval tv;
			xgettimeofday(&tv);
			sprintf(G.epoch_buf, fmt, (unsigned long long)tv.tv_sec,
					(unsigned)tv.tv_usec);
			પાછા G.epoch_buf;
		}
	}
#endif
	પાછા ખાલી;
}

#ifndef __U_BOOT__
#જો ENABLE_HUSH_GETOPTS
સ્થિર શૂન્ય handle_changed_special_names(અચલ અક્ષર *name)
{
	જો (varcmp(name, "OPTIND") == 0) {
		G.getopt_ગણતરી = 0;
		પાછા;
	}
}
#નહીંતર
/* Do not even bother evaluating arguments */
# define handle_changed_special_names(...) ((શૂન્ય)0)
#endif
#નહીંતર /* __U_BOOT__ */
/* Do not even bother evaluating arguments */
# define handle_changed_special_names(...) ((શૂન્ય)0)
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
સ્થિર પૂર્ણાંક set_local_var(અક્ષર *str, unsigned flags)
#નહીંતર /* __U_BOOT__ */
પૂર્ણાંક set_local_var_modern(અક્ષર *str, પૂર્ણાંક flags)
#endif /* __U_BOOT__ */
{
	સંરચના variable **cur_pp;
	સંરચના variable *cur;
	અક્ષર *free_me = ખાલી;
	અક્ષર *eq_sign;
	પૂર્ણાંક name_len;
	પૂર્ણાંક retval;
#ifndef __U_BOOT__
	unsigned local_lvl = (flags >> SETFLAG_VARLVL_SHIFT);
#endif /* !__U_BOOT__ */

	eq_sign = strchr(str, '=');
	જો (HUSH_DEBUG && !eq_sign)
		bb_simple_error_સંદેશ_and_die("BUG in setvar");

	name_len = eq_sign - str + 1; /* including '=' */
	cur_pp = &G.ટોચ_var;
	જ્યારે ((cur = *cur_pp) != ખાલી) {
		જો (strncmp(cur->varstr, str, name_len) != 0) {
			cur_pp = &cur->next;
			ચાલુ;
		}

#ifndef __U_BOOT__
		/* We found an existing var with this name */
		જો (cur->flg_read_only) {
			bb_error_સંદેશ("%s: readonly variable", str);
			મુક્ત(str);
//NOTE: in bash, assignment in "export READONLY_VAR=Z" fails, and sets $?=1,
//but export per se succeeds (does put the var in env). We don't mimic that.
			પાછા -1;
		}
		જો (flags & SETFLAG_UNEXPORT) { // && cur->flg_export ?
			debug_printf_env("%s: unsetenv '%s'\n", __func__, str);
			*eq_sign = '\0';
			unsetenv(str);
			*eq_sign = '=';
		}
		જો (cur->var_nest_level < local_lvl) {
			/* bash 3.2.33(1) and exported vars:
			 * # export z=z
			 * # f() { local z=a; env | grep ^z; }
			 * # f
			 * z=a
			 * # env | grep ^z
			 * z=z
			 */
			જો (cur->flg_export)
				flags |= SETFLAG_EXPORT;
			/* New variable is local ("local VAR=VAL" or
			 * "VAR=VAL cmd")
			 * and existing one is global, or local
			 * on a lower level that new one.
			 * Remove it from global variable list:
			 */
			*cur_pp = cur->next;
			જો (G.shadowed_vars_pp) {
				/* Save in "shadowed" list */
				debug_printf_env("shadowing %s'%s'/%u by '%s'/%u\n",
					cur->flg_export ? "exported " : "",
					cur->varstr, cur->var_nest_level, str, local_lvl
				);
				cur->next = *G.shadowed_vars_pp;
				*G.shadowed_vars_pp = cur;
			} નહીંતર {
				/* Came from pseudo_exec_argv(), no need to save: delete it */
				debug_printf_env("shadow-deleting %s'%s'/%u by '%s'/%u\n",
					cur->flg_export ? "exported " : "",
					cur->varstr, cur->var_nest_level, str, local_lvl
				);
				જો (cur->max_len == 0) /* allocated "VAR=VAL"? */
					free_me = cur->varstr; /* then free it later */
				મુક્ત(cur);
			}
			તોડો;
		}
#endif /* !__U_BOOT__ */

		જો (strcmp(cur->varstr + name_len, eq_sign + 1) == 0) {
			debug_printf_env("assignement '%s' does not change anything\n", str);
 free_and_exp:
			મુક્ત(str);
			જાઓ exp;
		}

		/* Replace the value in the found "struct variable" */
		જો (cur->max_len != 0) {
			જો (cur->max_len >= strnlen(str, cur->max_len + 1)) {
				/* This one is from startup env, reuse space */
				debug_printf_env("reusing startup env for '%s'\n", str);
				strcpy(cur->varstr, str);
				જાઓ free_and_exp;
			}
			/* Can't reuse */
			cur->max_len = 0;
			જાઓ set_str_and_exp;
		}
		/* max_len == 0 signifies "malloced" var, which we can
		 * (and have to) free. But we can't free(cur->varstr) here:
		 * if cur->flg_export is 1, it is in the environment.
		 * We should either unsetenv+free, or wait until putenv,
		 * then putenv(new)+free(old).
		 */
		free_me = cur->varstr;
		જાઓ set_str_and_exp;
	}

	/* Not found or shadowed - create new variable struct */
#ifndef __U_BOOT__
	debug_printf_env("%s: alloc new var '%s'/%u\n", __func__, str, local_lvl);
#નહીંતર /* __U_BOOT__ */
	debug_printf_env("%s: alloc new var '%s'\n", __func__, str);
#endif /* __U_BOOT__ */
	cur = xzalloc(કદ(*cur));
#ifndef __U_BOOT__
	cur->var_nest_level = local_lvl;
#endif /* !__U_BOOT__ */
	cur->next = *cur_pp;
	*cur_pp = cur;

 set_str_and_exp:
	cur->varstr = str;
 exp:
#ifndef __U_BOOT__
#જો !BB_MMU || ENABLE_HUSH_READONLY
	જો (flags & SETFLAG_MAKE_RO) {
		cur->flg_read_only = 1;
	}
#endif
	જો (flags & SETFLAG_EXPORT)
		cur->flg_export = 1;
#endif /* !__U_BOOT__ */
	retval = 0;
#ifndef __U_BOOT__
	જો (cur->flg_export) {
		જો (flags & SETFLAG_UNEXPORT) {
			cur->flg_export = 0;
			/* unsetenv was already done */
		} નહીંતર {
			debug_printf_env("%s: putenv '%s'/%u\n", __func__, cur->varstr, cur->var_nest_level);
			retval = putenv(cur->varstr);
			/* fall through to "free(free_me)" -
			 * only now we can free old exported malloced string
			 */
		}
	}
#endif /* !__U_BOOT__ */
	મુક્ત(free_me);

	handle_changed_special_names(cur->varstr);

	પાછા retval;
}

#ifndef __U_BOOT__
સ્થિર પૂર્ણાંક set_local_var0(અક્ષર *str)
{
	પાછા set_local_var(str, 0);
}

સ્થિર શૂન્ય FAST_FUNC set_local_var_from_halves(અચલ અક્ષર *name, અચલ અક્ષર *val)
{
	અક્ષર *var = xasprintf("%s=%s", name, val);
	set_local_var0(var);
}

/* Used at startup and after each cd */
સ્થિર શૂન્ય set_pwd_var(unsigned flag)
{
	set_local_var(xasprintf("PWD=%s", get_cwd(/*force:*/ 1)), flag);
}
#endif /* !__U_BOOT__ */

#જો ENABLE_HUSH_UNSET || ENABLE_HUSH_GETOPTS
સ્થિર પૂર્ણાંક unset_local_var(અચલ અક્ષર *name)
{
	સંરચના variable *cur;
	સંરચના variable **cur_pp;

	cur_pp = &G.ટોચ_var;
	જ્યારે ((cur = *cur_pp) != ખાલી) {
		જો (varcmp(cur->varstr, name) == 0) {
			જો (cur->flg_read_only) {
				bb_error_સંદેશ("%s: readonly variable", name);
				પાછા EXIT_FAILURE;
			}

			*cur_pp = cur->next;
			debug_printf_env("%s: unsetenv '%s'\n", __func__, cur->varstr);
			bb_unsetenv(cur->varstr);
			જો (!cur->max_len)
				મુક્ત(cur->varstr);
			મુક્ત(cur);

			તોડો;
		}
		cur_pp = &cur->next;
	}

	/* Handle "unset LINENO" et al even if did not find the variable to unset */
	handle_changed_special_names(name);

	પાછા EXIT_SUCCESS;
}
#endif

#ifndef __U_BOOT__
/*
 * Helpers for "var1=val1 var2=val2 cmd" feature
 */
સ્થિર શૂન્ય add_vars(સંરચના variable *var)
{
	સંરચના variable *next;

	જ્યારે (var) {
		next = var->next;
		var->next = G.ટોચ_var;
		G.ટોચ_var = var;
		જો (var->flg_export) {
			debug_printf_env("%s: restoring exported '%s'/%u\n", __func__, var->varstr, var->var_nest_level);
			putenv(var->varstr);
		} નહીંતર {
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
સ્થિર શૂન્ય set_vars_and_save_old(અક્ષર **strings)
{
	અક્ષર **s;

	જો (!strings)
		પાછા;

	s = strings;
	જ્યારે (*s) {
		સંરચના variable *var_p;
		સંરચના variable **var_pp;
		અક્ષર *eq;

		eq = strchr(*s, '=');
		જો (HUSH_DEBUG && !eq)
			bb_simple_error_સંદેશ_and_die("BUG in varexp4");
		var_pp = get_ptr_to_local_var(*s);
		જો (var_pp) {
			var_p = *var_pp;
			જો (var_p->flg_read_only) {
				અક્ષર **p;
				bb_error_સંદેશ("%s: readonly variable", *s);
				/*
				 * "VAR=V BLTIN" unsets VARs after BLTIN completes.
				 * If VAR is readonly, leaving it in the list
				 * after asssignment error (msg above)
				 * causes doubled error message later, on unset.
				 */
				debug_printf_env("removing/freeing '%s' element\n", *s);
				મુક્ત(*s);
				p = s;
				કરો { *p = p[1]; p++; } જ્યારે (*p);
				જાઓ next;
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
	મુક્ત(strings);
}

/*
 * Unicode helper
 */
સ્થિર શૂન્ય reinit_unicode_for_hush(શૂન્ય)
{
	/* Unicode support should be activated even if LANG is set
	 * _during_ shell execution, not only if it was set when
	 * shell was started. Therefore, re-check LANG every time:
	 */
	જો (ENABLE_FEATURE_CHECK_UNICODE_IN_ENV
	 || ENABLE_UNICODE_USING_LOCALE
	) {
		અચલ અક્ષર *s = get_local_var_કિંમત("LC_ALL");
		જો (!s) s = get_local_var_કિંમત("LC_CTYPE");
		જો (!s) s = get_local_var_કિંમત("LANG");
		reinit_unicode(s);
	}
}

#endif /* !__U_BOOT__ */
/*
 * in_str support (strings, and "strings" read from files).
 */

#જો ENABLE_HUSH_INTERACTIVE
#ifndef __U_BOOT__
/* To test correct lineedit/interactive behavior, type from command line:
 *	echo $P\
 *	\
 *	AT\
 *	H\
 *	\
 * It exercises a lot of corner cases.
 */
સ્થિર અચલ અક્ષર *setup_prompt_string(શૂન્ય)
{
	અચલ અક્ષર *prompt_str;

	debug_printf_prompt("%s promptmode:%d\n", __func__, G.promptmode);

# જો ENABLE_FEATURE_EDITING_FANCY_PROMPT
	prompt_str = get_local_var_કિંમત(G.promptmode == 0 ? "PS1" : "PS2");
	જો (!prompt_str)
		prompt_str = "";
# નહીંતર
	prompt_str = "> "; /* if PS2, else... */
	જો (G.promptmode == 0) { /* PS1 */
		/* No fancy prompts supported, (re)generate "CURDIR $ " by hand */
		મુક્ત(G.PS1);
		/* bash uses $PWD value, even if it is set by user.
		 * It uses current dir only if PWD is unset.
		 * We always use current dir. */
		prompt_str = G.PS1 = xasprintf("%s %c ", get_cwd(0), (geteuid() != 0) ? '$' : '#');
	}
# endif
	debug_printf("prompt_str '%s'\n", prompt_str);
	પાછા prompt_str;
}
#endif /* !__U_BOOT__ */

#ifndef __U_BOOT__
સ્થિર પૂર્ણાંક get_user_આગત(સંરચના in_str *i)
#નહીંતર /* __U_BOOT__ */
સ્થિર શૂન્ય get_user_આગત(સંરચના in_str *i)
#endif /* __U_BOOT__ */
{
#ifndef __U_BOOT__
# જો ENABLE_FEATURE_EDITING
	/* In EDITING case, this function reads next input line,
	 * saves it in i->p, then returns 1st char of it.
	 */
	પૂર્ણાંક r;
	અચલ અક્ષર *prompt_str;

	prompt_str = setup_prompt_string();
	માટે (;;) {
		reinit_unicode_for_hush();
		G.flag_SIGINT = 0;

		bb_got_signal = 0;
		જો (!sigisemptyset(&G.pending_set)) {
			/* Whoops, already got a signal, do not call read_line_input */
			bb_got_signal = r = -1;
		} નહીંતર {
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
			r = read_લીટી_આગત(G.લીટી_આગત_state, prompt_str,
				G.user_આગત_buf, CONFIG_FEATURE_EDITING_MAX_LEN-1
			);
			/* read_line_input intercepts ^C, "convert" it to SIGINT */
			જો (r == 0)
				ઉઠાવો(SIGINT);
		}
		/* bash prints ^C (before running a trap, if any)
		 * both on keyboard ^C and on real SIGINT (non-kbd generated).
		 */
		જો (sigismember(&G.pending_set, SIGINT)) {
			લિખ(STDOUT_FILENO, "^C\n", 3);
			G.last_exitcode = 128 | SIGINT;
		}
		check_and_run_traps();
		જો (r == 0) /* keyboard ^C? */
			ચાલુ; /* go back, read another input line */
		જો (r > 0) /* normal input? (no ^C, no ^D, no signals) */
			તોડો;
		જો (!bb_got_signal) {
			/* r < 0: ^D/EOF/error detected (but not signal) */
			/* ^D on interactive input goes to next line before exiting: */
			લિખ(STDOUT_FILENO, "\n", 1);
			i->p = ખાલી;
			i->peek_buf[0] = r = EOF;
			પાછા r;
		}
		/* it was a signal: go back, read another input line */
	}
	i->p = G.user_આગત_buf;
	પાછા (unsigned અક્ષર)*i->p++;
# નહીંતર
	/* In !EDITING case, this function gets called for every char.
	 * Buffering happens deeper in the call chain, in hfgetc(i->file).
	 */
	પૂર્ણાંક r;

	માટે (;;) {
		G.flag_SIGINT = 0;
		જો (i->last_char == '\0' || i->last_char == '\n') {
			અચલ અક્ષર *prompt_str = setup_prompt_string();
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
		જો (r != '\0' && !G.flag_SIGINT)
			તોડો;
		જો (G.flag_SIGINT) {
			/* ^C or SIGINT: repeat */
			/* bash prints ^C even on real SIGINT (non-kbd generated) */
			/* kernel prints "^C" itself, just print newline: */
			લિખ(STDOUT_FILENO, "\n", 1);
			G.last_exitcode = 128 | SIGINT;
		}
	}
	પાછા r;
# endif
#નહીંતર /* __U_BOOT__ */
	પૂર્ણાંક n;
	પૂર્ણાંક promptme;
	સ્થિર અક્ષર the_command[CONFIG_SYS_CBSIZE + 1];

	bootretry_reset_cmd_timeout();
	promptme = 1;
	n = u_boot_cli_readline(i);

# ifdef CONFIG_BOOT_RETRY_TIME
	જો (n == -2) {
		puts("\nTimeout waiting for command\n");
#  ifdef CONFIG_RESET_TO_RETRY
		do_reset(ખાલી, 0, 0, ખાલી);
#  elif IS_ENABLED(CONFIG_RETRY_BOOTCMD)
	strcpy(console_buffer, "run bootcmd\n");
# નહીંતર
#	error "This only works with CONFIG_RESET_TO_RETRY or CONFIG_BOOT_RETRY_COMMAND enabled"
#  endif
	}
# endif
	જો (n == -1 ) {
		G.flag_repeat = 0;
		promptme = 0;
	}
	n = strlen(console_buffer);
	console_buffer[n] = '\n';
	console_buffer[n+1]= '\0';
	જો (had_ctrlc())
		G.flag_repeat = 0;
	clear_ctrlc();
	G.do_repeat = 0;
#ifndef __U_BOOT__
	જો (G.promptmode == 1) {
#નહીંતર /* __U_BOOT__ */
	જો (!G.promptmode) {
#endif /* __U_BOOT__ */
		જો (console_buffer[0] == '\n'&& G.flag_repeat == 0) {
			strcpy(the_command, console_buffer);
		}
		નહીંતર {
			જો (console_buffer[0] != '\n') {
				strcpy(the_command, console_buffer);
				G.flag_repeat = 1;
			}
			નહીંતર {
				G.do_repeat = 1;
			}
		}
		i->p = the_command;
	}
	નહીંતર {
		જો (console_buffer[0] != '\n') {
			જો (strlen(the_command) + strlen(console_buffer)
				< CONFIG_SYS_CBSIZE) {
				n = strlen(the_command);
#ifdef __U_BOOT__
				/*
				 * To avoid writing to bad places, we check if
				 * n is greater than 0.
				 * This bug was found by Harald Seiler.
				 */
				જો (n > 0)
					the_command[n-1] = ' ';
				strcpy(&the_command[n], console_buffer);
#નહીંતર /* !__U_BOOT__ */
			the_command[n-1] = ' ';
			strcpy(&the_command[n], console_buffer);
#endif /* !__U_BOOT__ */
				}
				નહીંતર {
					the_command[0] = '\n';
					the_command[1] = '\0';
					G.flag_repeat = 0;
				}
		}
		જો (promptme == 0) {
			the_command[0] = '\n';
			the_command[1] = '\0';
		}
		i->p = console_buffer;
	}
#endif /* __U_BOOT__ */
}
/* This is the magic location that prints prompts
 * and gets data back from the user */
સ્થિર પૂર્ણાંક fgetc_interactive(સંરચના in_str *i)
{
	પૂર્ણાંક ch;
#ifndef __U_BOOT__
	/* If it's interactive stdin, get new line. */
	જો (G_interactive_fd && i->file == G.HFILE_stdin) {
#endif /* !__U_BOOT__ */
#ifndef __U_BOOT__
		/* Returns first char (or EOF), the rest is in i->p[] */
		ch = get_user_આગત(i);
#નહીંતર /* __U_BOOT__ */
		/* Avoid garbage value and make clang happy. */
		ch = 0;
		/*
		 * get_user_input() does not return anything when used in
		 * U-Boot.
		 * So, we need to take the read character from i->p[].
		 */
		get_user_આગત(i);
		જો (i->p && *i->p) {
			ch = *i->p++;
		}
#endif /* __U_BOOT__ */
		G.promptmode = 1; /* PS2 */
		debug_printf_prompt("%s promptmode=%d\n", __func__, G.promptmode);
#ifndef __U_BOOT__
	} નહીંતર {
		/* Not stdin: script file, sourced file, etc */
		કરો ch = hfgetc(i->file); જ્યારે (ch == '\0');
	}
#endif /* !__U_BOOT__ */
	પાછા ch;
}
#નહીંતર  /* !INTERACTIVE */
#ifndef __U_BOOT__
સ્થિર ALWAYS_INLINE પૂર્ણાંક fgetc_interactive(સંરચના in_str *i)
{
	પૂર્ણાંક ch;
	કરો ch = hfgetc(i->file); જ્યારે (ch == '\0');
	પાછા ch;
}
#endif /* !__U_BOOT__ */
#endif  /* !INTERACTIVE */

સ્થિર પૂર્ણાંક i_getch(સંરચના in_str *i)
{
	પૂર્ણાંક ch;

#ifndef __U_BOOT__
	જો (!i->file) {
		/* string-based in_str */
		ch = (unsigned અક્ષર)*i->p;
		જો (ch != '\0') {
			i->p++;
			i->last_char = ch;
#જો ENABLE_HUSH_LINENO_VAR
			જો (ch == '\n') {
				G.parse_lineno++;
				debug_printf_parse("G.parse_lineno++ = %u\n", G.parse_lineno);
			}
#endif
			પાછા ch;
		}
		પાછા EOF;
	}

#endif /* !__U_BOOT__ */
	/* FILE-based in_str */

#જો ENABLE_FEATURE_EDITING
	/* This can be stdin, check line editing char[] buffer */
	જો (i->p && *i->p != '\0') {
		ch = (unsigned અક્ષર)*i->p++;
		જાઓ out;
#ifndef __U_BOOT__
	}
#નહીંતર /* __U_BOOT__ */
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
	} નહીંતર જો (i->p && *i->p == '\0' && G.run_command_flags){
		પાછા EOF;
	}
#endif /* __U_BOOT__ */
#endif
#ifndef __U_BOOT__
	/* peek_buf[] is an int array, not char. Can contain EOF. */
	ch = i->peek_buf[0];
	જો (ch != 0) {
		પૂર્ણાંક ch2 = i->peek_buf[1];
		i->peek_buf[0] = ch2;
		જો (ch2 == 0) /* very likely, avoid redundant write */
			જાઓ out;
		i->peek_buf[1] = 0;
		જાઓ out;
	}

#endif /* !__U_BOOT__ */
	ch = fgetc_interactive(i);
 out:
	debug_printf("file_get: got '%c' %d\n", ch, ch);
	i->last_char = ch;
#જો ENABLE_HUSH_LINENO_VAR
	જો (ch == '\n') {
		G.parse_lineno++;
		debug_printf_parse("G.parse_lineno++ = %u\n", G.parse_lineno);
	}
#endif
	પાછા ch;
}

સ્થિર પૂર્ણાંક i_peek(સંરચના in_str *i)
{
#ifndef __U_BOOT__
	પૂર્ણાંક ch;

	જો (!i->file) {
		/* string-based in_str */
		/* Doesn't report EOF on NUL. None of the callers care. */
		પાછા (unsigned અક્ષર)*i->p;
	}

	/* FILE-based in_str */

#જો ENABLE_FEATURE_EDITING && ENABLE_HUSH_INTERACTIVE
	/* This can be stdin, check line editing char[] buffer */
	જો (i->p && *i->p != '\0')
		પાછા (unsigned અક્ષર)*i->p;
#endif
	/* peek_buf[] is an int array, not char. Can contain EOF. */
	ch = i->peek_buf[0];
	જો (ch != 0)
		પાછા ch;

	/* Need to get a new char */
	ch = fgetc_interactive(i);
	debug_printf("file_peek: got '%c' %d\n", ch, ch);

	/* Save it by either rolling back line editing buffer, or in i->peek_buf[0] */
#જો ENABLE_FEATURE_EDITING && ENABLE_HUSH_INTERACTIVE
	જો (i->p) {
		i->p -= 1;
		પાછા ch;
	}
#endif
	i->peek_buf[0] = ch;
	/*i->peek_buf[1] = 0; - already is */
	પાછા ch;
#નહીંતર /* __U_BOOT__ */
	/* string-based in_str */
	/* Doesn't report EOF on NUL. None of the callers care. */
	પાછા (unsigned અક્ષર)*i->p;
#endif /* __U_BOOT__ */
}

/* Only ever called if i_peek() was called, and did not return EOF.
 * IOW: we know the previous peek saw an ordinary char, not EOF, not NUL,
 * not end-of-line. Therefore we never need to read a new editing line here.
 */
સ્થિર પૂર્ણાંક i_peek2(સંરચના in_str *i)
{
#ifndef __U_BOOT__
	પૂર્ણાંક ch;
#endif /* !__U_BOOT__ */

	/* There are two cases when i->p[] buffer exists.
	 * (1) it's a string in_str.
	 * (2) It's a file, and we have a saved line editing buffer.
	 * In both cases, we know that i->p[0] exists and not NUL, and
	 * the peek2 result is in i->p[1].
	 */
	જો (i->p)
		પાછા (unsigned અક્ષર)i->p[1];

#ifndef __U_BOOT__
	/* Now we know it is a file-based in_str. */

	/* peek_buf[] is an int array, not char. Can contain EOF. */
	/* Is there 2nd char? */
	ch = i->peek_buf[1];
	જો (ch == 0) {
		/* We did not read it yet, get it now */
		કરો ch = hfgetc(i->file); જ્યારે (ch == '\0');
		i->peek_buf[1] = ch;
	}

	debug_printf("file_peek2: got '%c' %d\n", ch, ch);
	પાછા ch;
#નહીંતર
	પાછા 0;
#endif /* __U_BOOT__ */
}

સ્થિર પૂર્ણાંક i_getch_and_eat_bkslash_nl(સંરચના in_str *આગત)
{
	માટે (;;) {
		પૂર્ણાંક ch, ch2;

		ch = i_getch(આગત);
		જો (ch != '\\')
			પાછા ch;
		ch2 = i_peek(આગત);
		જો (ch2 != '\n')
			પાછા ch;
		/* backslash+newline, skip it */
		i_getch(આગત);
	}
}

/* Note: this function _eats_ \<newline> pairs, safe to use plain
 * i_getch() after it instead of i_getch_and_eat_bkslash_nl().
 */
સ્થિર પૂર્ણાંક i_peek_and_eat_bkslash_nl(સંરચના in_str *આગત)
{
	માટે (;;) {
		પૂર્ણાંક ch, ch2;

		ch = i_peek(આગત);
		જો (ch != '\\')
			પાછા ch;
		ch2 = i_peek2(આગત);
		જો (ch2 != '\n')
			પાછા ch;
		/* backslash+newline, skip it */
		i_getch(આગત);
		i_getch(આગત);
	}
}

#ifndef __U_BOOT__
સ્થિર શૂન્ય setup_file_in_str(સંરચના in_str *i, HFILE *fp)
#નહીંતર /* __U_BOOT__ */
સ્થિર શૂન્ય setup_file_in_str(સંરચના in_str *i)
#endif /* __U_BOOT__ */
{
	memset(i, 0, કદ(*i));
#ifndef __U_BOOT__
	i->file = fp;
	/* i->p = NULL; */
#endif /* !__U_BOOT__ */
}

સ્થિર શૂન્ય setup_string_in_str(સંરચના in_str *i, અચલ અક્ષર *s)
{
	memset(i, 0, કદ(*i));
	/*i->file = NULL */;
	i->p = s;
}

/*
 * o_string support
 */
#define B_CHUNK  (32 * કદ(અક્ષર*))

સ્થિર શૂન્ય o_reset_to_empty_unquoted(o_string *o)
{
	o->length = 0;
	o->has_quoted_part = 0;
	જો (o->data)
		o->data[0] = '\0';
}

સ્થિર શૂન્ય o_free_and_set_NULL(o_string *o)
{
	મુક્ત(o->data);
	memset(o, 0, કદ(*o));
}

સ્થિર ALWAYS_INLINE શૂન્ય o_free(o_string *o)
{
	મુક્ત(o->data);
}

સ્થિર શૂન્ય o_grow_by(o_string *o, પૂર્ણાંક len)
{
	જો (o->length + len > o->maxlen) {
		o->maxlen += (2 * len) | (B_CHUNK-1);
		o->data = xrealloc(o->data, 1 + o->maxlen);
	}
}

સ્થિર શૂન્ય o_addchr(o_string *o, પૂર્ણાંક ch)
{
	debug_printf("o_addchr: '%c' o->length=%d o=%p\n", ch, o->length, o);
	જો (o->length < o->maxlen) {
		/* likely. avoid o_grow_by() call */
 add:
		o->data[o->length] = ch;
		o->length++;
		o->data[o->length] = '\0';
		પાછા;
	}
	o_grow_by(o, 1);
	જાઓ add;
}

#જો 0
/* Valid only if we know o_string is not empty */
સ્થિર શૂન્ય o_delchr(o_string *o)
{
	o->length--;
	o->data[o->length] = '\0';
}
#endif

સ્થિર શૂન્ય o_addblock(o_string *o, અચલ અક્ષર *str, પૂર્ણાંક len)
{
	o_grow_by(o, len);
	((અક્ષર*)mempcpy(&o->data[o->length], str, len))[0] = '\0';
	o->length += len;
}

સ્થિર શૂન્ય o_addstr(o_string *o, અચલ અક્ષર *str)
{
	o_addblock(o, str, strlen(str));
}

#ifndef __U_BOOT__
સ્થિર શૂન્ય o_addstr_with_NUL(o_string *o, અચલ અક્ષર *str)
{
	o_addblock(o, str, strlen(str) + 1);
}
#endif /* !__U_BOOT__ */

#જો !BB_MMU
સ્થિર શૂન્ય nommu_addchr(o_string *o, પૂર્ણાંક ch)
{
	જો (o)
		o_addchr(o, ch);
}
#નહીંતર
# define nommu_addchr(o, str) ((શૂન્ય)0)
#endif

#ifndef __U_BOOT__
#જો ENABLE_HUSH_MODE_X
સ્થિર શૂન્ય x_mode_addchr(પૂર્ણાંક ch)
{
	o_addchr(&G.x_mode_buf, ch);
}
સ્થિર શૂન્ય x_mode_addstr(અચલ અક્ષર *str)
{
	o_addstr(&G.x_mode_buf, str);
}
સ્થિર શૂન્ય x_mode_addblock(અચલ અક્ષર *str, પૂર્ણાંક len)
{
	o_addblock(&G.x_mode_buf, str, len);
}
સ્થિર શૂન્ય x_mode_prefix(શૂન્ય)
{
	પૂર્ણાંક n = G.x_mode_depth;
	કરો x_mode_addchr('+'); જ્યારે (--n >= 0);
}
સ્થિર શૂન્ય x_mode_flush(શૂન્ય)
{
	પૂર્ણાંક len = G.x_mode_buf.length;
	જો (len <= 0)
		પાછા;
	જો (G.x_mode_fd > 0) {
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

#જો ENABLE_HUSH_BRACE_EXPANSION
# define MAYBE_BRACES "{}"
#નહીંતર
# define MAYBE_BRACES ""
#endif

/* My analysis of quoting semantics tells me that state information
 * is associated with a destination, not a source.
 */
સ્થિર શૂન્ય o_addqchr(o_string *o, પૂર્ણાંક ch)
{
	પૂર્ણાંક sz = 1;
	/* '-' is included because of this case:
	 * >filename0 >filename1 >filename9; v='-'; echo filename[0"$v"9]
	 */
	અક્ષર *found = strchr("*?[-\\" MAYBE_BRACES, ch);
	જો (found)
		sz++;
	o_grow_by(o, sz);
	જો (found) {
		o->data[o->length] = '\\';
		o->length++;
	}
	o->data[o->length] = ch;
	o->length++;
	o->data[o->length] = '\0';
}

સ્થિર શૂન્ય o_addQchr(o_string *o, પૂર્ણાંક ch)
{
	પૂર્ણાંક sz = 1;
	જો ((o->o_expflags & EXP_FLAG_ESC_GLOB_CHARS)
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

સ્થિર શૂન્ય o_addqblock(o_string *o, અચલ અક્ષર *str, પૂર્ણાંક len)
{
	જ્યારે (len) {
		અક્ષર ch;
		પૂર્ણાંક sz;
		પૂર્ણાંક ordinary_cnt = strcspn(str, "*?[-\\" MAYBE_BRACES);
		જો (ordinary_cnt > len) /* paranoia */
			ordinary_cnt = len;
		o_addblock(o, str, ordinary_cnt);
		જો (ordinary_cnt == len)
			પાછા; /* NUL is already added by o_addblock */
		str += ordinary_cnt;
		len -= ordinary_cnt + 1; /* we are processing + 1 char below */

		ch = *str++;
		sz = 1;
		જો (ch) { /* it is necessarily one of "*?[-\\" MAYBE_BRACES */
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

સ્થિર શૂન્ય o_addQblock(o_string *o, અચલ અક્ષર *str, પૂર્ણાંક len)
{
	જો (!(o->o_expflags & EXP_FLAG_ESC_GLOB_CHARS)) {
		o_addblock(o, str, len);
		પાછા;
	}
	o_addqblock(o, str, len);
}

સ્થિર શૂન્ય o_addQstr(o_string *o, અચલ અક્ષર *str)
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
#જો DEBUG_EXPAND || DEBUG_GLOB
સ્થિર શૂન્ય debug_print_list(અચલ અક્ષર *prefix, o_string *o, પૂર્ણાંક n)
{
	અક્ષર **list = (અક્ષર**)o->data;
	પૂર્ણાંક string_start = ((n + 0xf) & ~0xf) * કદ(list[0]);
	પૂર્ણાંક i = 0;

	indent();
	fdprintf(2, "%s: list:%p n:%d string_start:%d length:%d maxlen:%d glob:%d quoted:%d escape:%d\n",
			prefix, list, n, string_start, o->length, o->maxlen,
			!!(o->o_expflags & EXP_FLAG_GLOB),
			o->has_quoted_part,
			!!(o->o_expflags & EXP_FLAG_ESC_GLOB_CHARS));
	જ્યારે (i < n) {
		indent();
		fdprintf(2, " list[%d]=%d '%s' %p\n", i, (પૂર્ણાંક)(uintptr_t)list[i],
				o->data + (પૂર્ણાંક)(uintptr_t)list[i] + string_start,
				o->data + (પૂર્ણાંક)(uintptr_t)list[i] + string_start);
		i++;
	}
	જો (n) {
		અચલ અક્ષર *p = o->data + (પૂર્ણાંક)(uintptr_t)list[n - 1] + string_start;
		indent();
#ifndef __U_BOOT__
		fdprintf(2, " total_sz:%ld\n", (long)((p + strlen(p) + 1) - o->data));
#નહીંતર /* __U_BOOT__ */
		printf(" total_sz:%ld\n", (long)((p + strlen(p) + 1) - o->data));
#endif /* __U_BOOT__ */
	}
}
#નહીંતર
# define debug_print_list(prefix, o, n) ((શૂન્ય)0)
#endif

/* n = o_save_ptr_helper(str, n) "starts new string" by storing an index value
 * in list[n] so that it points past last stored byte so far.
 * It returns n+1. */
સ્થિર પૂર્ણાંક o_save_ptr_helper(o_string *o, પૂર્ણાંક n)
{
	અક્ષર **list = (અક્ષર**)o->data;
	પૂર્ણાંક string_start;
	પૂર્ણાંક string_len;

	જો (!o->has_empty_slot) {
		string_start = ((n + 0xf) & ~0xf) * કદ(list[0]);
		string_len = o->length - string_start;
		જો (!(n & 0xf)) { /* 0, 0x10, 0x20...? */
			debug_printf_list("list[%d]=%d string_start=%d (growing)\n", n, string_len, string_start);
			/* list[n] points to string_start, make space for 16 more pointers */
			o->maxlen += 0x10 * કદ(list[0]);
			o->data = xrealloc(o->data, o->maxlen + 1);
			list = (અક્ષર**)o->data;
			memmove(list + n + 0x10, list + n, string_len);
			/*
			 * expand_on_ifs() has a "previous argv[] ends in IFS?"
			 * check. (grep for -prev-ifs-check-).
			 * Ensure that argv[-1][last] is not garbage
			 * but zero bytes, to save index check there.
			 */
			list[n + 0x10 - 1] = 0;
			o->length += 0x10 * કદ(list[0]);
		} નહીંતર {
			debug_printf_list("list[%d]=%d string_start=%d\n",
					n, string_len, string_start);
		}
	} નહીંતર {
		/* We have empty slot at list[n], reuse without growth */
		string_start = ((n+1 + 0xf) & ~0xf) * કદ(list[0]); /* NB: n+1! */
		string_len = o->length - string_start;
		debug_printf_list("list[%d]=%d string_start=%d (empty slot)\n",
				n, string_len, string_start);
		o->has_empty_slot = 0;
	}
	o->has_quoted_part = 0;
	list[n] = (અક્ષર*)(uintptr_t)string_len;
	પાછા n + 1;
}

/* "What was our last o_save_ptr'ed position (byte offset relative o->data)?" */
સ્થિર પૂર્ણાંક o_get_last_ptr(o_string *o, પૂર્ણાંક n)
{
	અક્ષર **list = (અક્ષર**)o->data;
	પૂર્ણાંક string_start = ((n + 0xf) & ~0xf) * કદ(list[0]);

	પાછા ((પૂર્ણાંક)(uintptr_t)list[n-1]) + string_start;
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

#જો ENABLE_HUSH_BRACE_EXPANSION
/* There in a GNU extension, GLOB_BRACE, but it is not usable:
 * first, it processes even {a} (no commas), second,
 * I didn't manage to make it return strings when they don't match
 * existing files. Need to re-implement it.
 */

/* Helper */
સ્થિર પૂર્ણાંક glob_needed(અચલ અક્ષર *s)
{
	જ્યારે (*s) {
		જો (*s == '\\') {
			જો (!s[1])
				પાછા 0;
			s += 2;
			ચાલુ;
		}
		જો (*s == '*' || *s == '[' || *s == '?' || *s == '{')
			પાછા 1;
		s++;
	}
	પાછા 0;
}
/* Return pointer to next closing brace or to comma */
સ્થિર અચલ અક્ષર *next_brace_sub(અચલ અક્ષર *cp)
{
	unsigned depth = 0;
	cp++;
	જ્યારે (*cp != '\0') {
		જો (*cp == '\\') {
			જો (*++cp == '\0')
				તોડો;
			cp++;
			ચાલુ;
		}
		જો ((*cp == '}' && depth-- == 0) || (*cp == ',' && depth == 0))
			તોડો;
		જો (*cp++ == '{')
			depth++;
	}

	પાછા *cp != '\0' ? cp : ખાલી;
}
/* Recursive brace globber. Note: may garble pattern[]. */
સ્થિર પૂર્ણાંક glob_brace(અક્ષર *pattern, o_string *o, પૂર્ણાંક n)
{
	અક્ષર *new_pattern_buf;
	અચલ અક્ષર *begin;
	અચલ અક્ષર *next;
	અચલ અક્ષર *rest;
	અચલ અક્ષર *p;
	size_t rest_len;

	debug_printf_glob("glob_brace('%s')\n", pattern);

	begin = pattern;
	જ્યારે (1) {
		જો (*begin == '\0')
			જાઓ simple_glob;
		જો (*begin == '{') {
			/* Find the first sub-pattern and at the same time
			 * find the rest after the closing brace */
			next = next_brace_sub(begin);
			જો (next == ખાલી) {
				/* An illegal expression */
				જાઓ simple_glob;
			}
			જો (*next == '}') {
				/* "{abc}" with no commas - illegal
				 * brace expr, disregard and skip it */
				begin = next + 1;
				ચાલુ;
			}
			તોડો;
		}
		જો (*begin == '\\' && begin[1] != '\0')
			begin++;
		begin++;
	}
	debug_printf_glob("begin:%s\n", begin);
	debug_printf_glob("next:%s\n", next);

	/* Now find the end of the whole brace expression */
	rest = next;
	જ્યારે (*rest != '}') {
		rest = next_brace_sub(rest);
		જો (rest == ખાલી) {
			/* An illegal expression */
			જાઓ simple_glob;
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
	જ્યારે (1) {
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
		જો (*next == '}') {
			/* We saw the last entry */
			તોડો;
		}
		p = next + 1;
		next = next_brace_sub(next);
	}
	મુક્ત(new_pattern_buf);
	પાછા n;

 simple_glob:
	{
		પૂર્ણાંક gr;
		glob_t globdata;

		memset(&globdata, 0, કદ(globdata));
		gr = glob(pattern, 0, ખાલી, &globdata);
		debug_printf_glob("glob('%s'):%d\n", pattern, gr);
		જો (gr != 0) {
			જો (gr == GLOB_NOMATCH) {
				globfree(&globdata);
				/* NB: garbles parameter */
				unbackslash(pattern);
				o_addstr_with_NUL(o, pattern);
				debug_printf_glob("glob pattern '%s' is literal\n", pattern);
				પાછા o_save_ptr_helper(o, n);
			}
			જો (gr == GLOB_NOSPACE)
				bb_die_memory_exhausted();
			/* GLOB_ABORTED? Only happens with GLOB_ERR flag,
			 * but we didn't specify it. Paranoia again. */
			bb_error_સંદેશ_and_die("glob error %d on '%s'", gr, pattern);
		}
		જો (globdata.gl_pathv && globdata.gl_pathv[0]) {
			અક્ષર **argv = globdata.gl_pathv;
			જ્યારે (1) {
				o_addstr_with_NUL(o, *argv);
				n = o_save_ptr_helper(o, n);
				argv++;
				જો (!*argv)
					તોડો;
			}
		}
		globfree(&globdata);
	}
	પાછા n;
}
/* Performs globbing on last list[],
 * saving each result as a new list[].
 */
સ્થિર પૂર્ણાંક perform_glob(o_string *o, પૂર્ણાંક n)
{
	અક્ષર *pattern, *copy;

	debug_printf_glob("start perform_glob: n:%d o->data:%p\n", n, o->data);
	જો (!o->data)
		પાછા o_save_ptr_helper(o, n);
	pattern = o->data + o_get_last_ptr(o, n);
	debug_printf_glob("glob pattern '%s'\n", pattern);
	જો (!glob_needed(pattern)) {
		/* unbackslash last string in o in place, fix length */
		o->length = unbackslash(pattern) - o->data;
		debug_printf_glob("glob pattern '%s' is literal\n", pattern);
		પાછા o_save_ptr_helper(o, n);
	}

	copy = xstrdup(pattern);
	/* "forget" pattern in o */
	o->length = pattern - o->data;
	n = glob_brace(copy, o, n);
	મુક્ત(copy);
	જો (DEBUG_GLOB)
		debug_print_list("perform_glob returning", o, n);
	પાછા n;
}

#નહીંતર /* !HUSH_BRACE_EXPANSION */

/* Helper */
સ્થિર પૂર્ણાંક glob_needed(અચલ અક્ષર *s)
{
	જ્યારે (*s) {
		જો (*s == '\\') {
			જો (!s[1])
				પાછા 0;
			s += 2;
			ચાલુ;
		}
		જો (*s == '*' || *s == '[' || *s == '?')
			પાછા 1;
		s++;
	}
	પાછા 0;
}
/* Performs globbing on last list[],
 * saving each result as a new list[].
 */
સ્થિર પૂર્ણાંક perform_glob(o_string *o, પૂર્ણાંક n)
{
#ifndef __U_BOOT__
	glob_t globdata;
	પૂર્ણાંક gr;
#endif /* __U_BOOT__ */
	અક્ષર *pattern;

	debug_printf_glob("start perform_glob: n:%d o->data:%p\n", n, o->data);
	જો (!o->data)
		પાછા o_save_ptr_helper(o, n);
	pattern = o->data + o_get_last_ptr(o, n);
	debug_printf_glob("glob pattern '%s'\n", pattern);
	જો (!glob_needed(pattern)) {
#ifndef __U_BOOT__
 literal:
#endif /* __U_BOOT__ */
		/* unbackslash last string in o in place, fix length */
		o->length = unbackslash(pattern) - o->data;
		debug_printf_glob("glob pattern '%s' is literal\n", pattern);
		પાછા o_save_ptr_helper(o, n);
	}

#ifndef __U_BOOT__
	memset(&globdata, 0, કદ(globdata));
	/* Can't use GLOB_NOCHECK: it does not unescape the string.
	 * If we glob "*.\*" and don't find anything, we need
	 * to fall back to using literal "*.*", but GLOB_NOCHECK
	 * will return "*.\*"!
	 */
	gr = glob(pattern, 0, ખાલી, &globdata);
	debug_printf_glob("glob('%s'):%d\n", pattern, gr);
	જો (gr != 0) {
		જો (gr == GLOB_NOMATCH) {
			globfree(&globdata);
			જાઓ literal;
		}
		જો (gr == GLOB_NOSPACE)
			bb_die_memory_exhausted();
		/* GLOB_ABORTED? Only happens with GLOB_ERR flag,
		 * but we didn't specify it. Paranoia again. */
		bb_error_સંદેશ_and_die("glob error %d on '%s'", gr, pattern);
	}
	જો (globdata.gl_pathv && globdata.gl_pathv[0]) {
		અક્ષર **argv = globdata.gl_pathv;
		/* "forget" pattern in o */
		o->length = pattern - o->data;
		જ્યારે (1) {
			o_addstr_with_NUL(o, *argv);
			n = o_save_ptr_helper(o, n);
			argv++;
			જો (!*argv)
				તોડો;
		}
	}
	globfree(&globdata);
	જો (DEBUG_GLOB)
		debug_print_list("perform_glob returning", o, n);
	પાછા n;
#નહીંતર /* __U_BOOT__ */
	/*
	 * NOTE We only use perform glob to call unbackslash to remove backslash
	 * from string once expanded.
	 * So, it seems OK to return this if no previous return was done.
	 */
	પાછા o_save_ptr_helper(o, n);
#endif /* __U_BOOT__ */
}

#endif /* !HUSH_BRACE_EXPANSION */

/* If o->o_expflags & EXP_FLAG_GLOB, glob the string so far remembered.
 * Otherwise, just finish current list[] and start new */
સ્થિર પૂર્ણાંક o_save_ptr(o_string *o, પૂર્ણાંક n)
{
	જો (o->o_expflags & EXP_FLAG_GLOB) {
		/* If o->has_empty_slot, list[n] was already globbed
		 * (if it was requested back then when it was filled)
		 * so don't do that again! */
		જો (!o->has_empty_slot)
			પાછા perform_glob(o, n); /* o_save_ptr_helper is inside */
	}
	પાછા o_save_ptr_helper(o, n);
}

/* "Please convert list[n] to real char* ptrs, and NULL terminate it." */
સ્થિર અક્ષર **o_finalize_list(o_string *o, પૂર્ણાંક n)
{
	અક્ષર **list;
	પૂર્ણાંક string_start;

	જો (DEBUG_EXPAND)
		debug_print_list("finalized", o, n);
	debug_printf_expand("finalized n:%d\n", n);
	list = (અક્ષર**)o->data;
	string_start = ((n + 0xf) & ~0xf) * કદ(list[0]);
	list[--n] = ખાલી;
	જ્યારે (n) {
		n--;
		list[n] = o->data + (પૂર્ણાંક)(uintptr_t)list[n] + string_start;
	}
	પાછા list;
}

સ્થિર શૂન્ય free_pipe_list(સંરચના pipe *pi);

/* Returns pi->next - next pipe in the list */
સ્થિર સંરચના pipe *free_pipe(સંરચના pipe *pi)
{
	સંરચના pipe *next;
	પૂર્ણાંક i;

#ifndef __U_BOOT__
	debug_printf_clean("free_pipe (pid %d)\n", getpid());
#endif /* !__U_BOOT__ */
	માટે (i = 0; i < pi->અંક_cmds; i++) {
		સંરચના command *command;
#ifndef __U_BOOT__
		સંરચના redir_struct *r, *rnext;
#endif /* !__U_BOOT__ */

		command = &pi->cmds[i];
		debug_printf_clean("  command %d:\n", i);
		જો (command->argv) {
			જો (DEBUG_CLEAN) {
				પૂર્ણાંક a;
				અક્ષર **p;
				માટે (a = 0, p = command->argv; *p; a++, p++) {
					debug_printf_clean("   argv[%d] = %s\n", a, *p);
				}
			}
			free_strings(command->argv);
			//command->argv = NULL;
		}
		/* not "else if": on syntax error, we may have both! */
		જો (command->group) {
			debug_printf_clean("   begin group (cmd_type:%d)\n",
					command->cmd_type);
			free_pipe_list(command->group);
			debug_printf_clean("   end group\n");
			//command->group = NULL;
		}
		/* else is crucial here.
		 * If group != NULL, child_func is meaningless */
#જો ENABLE_HUSH_FUNCTIONS
		નહીંતર જો (command->child_func) {
			debug_printf_exec("cmd %p releases child func at %p\n", command, command->child_func);
			command->child_func->parent_cmd = ખાલી;
		}
#endif
#જો !BB_MMU
		મુક્ત(command->group_as_string);
		//command->group_as_string = NULL;
#endif
#ifndef __U_BOOT__
		માટે (r = command->redirects; r; r = rnext) {
			debug_printf_clean("   redirect %d%s",
					r->rd_fd, redir_table[r->rd_type].descrip);
			/* guard against the case >$FOO, where foo is unset or blank */
			જો (r->rd_filename) {
				debug_printf_clean(" fname:'%s'\n", r->rd_filename);
				મુક્ત(r->rd_filename);
				//r->rd_filename = NULL;
			}
			debug_printf_clean(" rd_dup:%d\n", r->rd_dup);
			rnext = r->next;
			મુક્ત(r);
		}
		//command->redirects = NULL;
#endif /* !__U_BOOT__ */
	}
	મુક્ત(pi->cmds);   /* children are an array, they get freed all at once */
	//pi->cmds = NULL;
#ifndef __U_BOOT__
#જો ENABLE_HUSH_JOB
	મુક્ત(pi->cmdtext);
	//pi->cmdtext = NULL;
#endif
#endif /* !__U_BOOT__ */

	next = pi->next;
	મુક્ત(pi);
	પાછા next;
}

સ્થિર શૂન્ય free_pipe_list(સંરચના pipe *pi)
{
	જ્યારે (pi) {
#જો HAS_KEYWORDS
		debug_printf_clean("pipe reserved word %d\n", pi->res_word);
#endif
		debug_printf_clean("pipe followup code %d\n", pi->followup);
		pi = free_pipe(pi);
	}
}

/*** Parsing routines ***/

#ifndef debug_print_વૃક્ષ
સ્થિર શૂન્ય debug_print_વૃક્ષ(સંરચના pipe *pi, પૂર્ણાંક lvl)
{
	સ્થિર અચલ અક્ષર *અચલ PIPE[] ALIGN_PTR = {
		[PIPE_SEQ] = "SEQ",
		[PIPE_AND] = "AND",
		[PIPE_OR ] = "OR" ,
		[PIPE_BG ] = "BG" ,
	};
	સ્થિર અચલ અક્ષર *RES[] = {
		[RES_NONE ] = "NONE" ,
# જો ENABLE_HUSH_IF
		[RES_IF   ] = "IF"   ,
		[RES_THEN ] = "THEN" ,
		[RES_ELIF ] = "ELIF" ,
		[RES_ELSE ] = "ELSE" ,
		[RES_FI   ] = "FI"   ,
# endif
# જો ENABLE_HUSH_LOOPS
		[RES_FOR  ] = "FOR"  ,
		[RES_WHILE] = "WHILE",
		[RES_UNTIL] = "UNTIL",
		[RES_DO   ] = "DO"   ,
		[RES_DONE ] = "DONE" ,
# endif
# જો ENABLE_HUSH_LOOPS || ENABLE_HUSH_CASE
		[RES_IN   ] = "IN"   ,
# endif
# જો ENABLE_HUSH_CASE
		[RES_CASE ] = "CASE" ,
		[RES_CASE_IN ] = "CASE_IN" ,
		[RES_MATCH] = "MATCH",
		[RES_CASE_BODY] = "CASE_BODY",
		[RES_ESAC ] = "ESAC" ,
# endif
		[RES_XXXX ] = "XXXX" ,
		[RES_SNTX ] = "SNTX" ,
	};
	સ્થિર અચલ અક્ષર *અચલ CMDTYPE[] ALIGN_PTR = {
		"{}",
		"()",
		"[noglob]",
# જો ENABLE_HUSH_FUNCTIONS
		"func()",
# endif
	};

	પૂર્ણાંક pin, prn;

	pin = 0;
	જ્યારે (pi) {
		fdprintf(2, "%*spipe %d #cmds:%d %sres_word=%s followup=%d %s\n",
				lvl*2, "",
				pin,
				pi->અંક_cmds,
				(IF_HAS_KEYWORDS(pi->pi_inverted ? "! " :) ""),
				RES[pi->res_word],
				pi->followup, PIPE[pi->followup]
		);
		prn = 0;
		જ્યારે (prn < pi->અંક_cmds) {
			સંરચના command *command = &pi->cmds[prn];
			અક્ષર **argv = command->argv;

			fdprintf(2, "%*s cmd %d assignment_cnt:%d",
					lvl*2, "", prn,
					command->assignment_cnt);
# જો ENABLE_HUSH_LINENO_VAR
			fdprintf(2, " LINENO:%u", command->lineno);
# endif
			જો (command->group) {
				fdprintf(2, " group %s: (argv=%p)%s%s\n",
						CMDTYPE[command->cmd_type],
						argv
# જો !BB_MMU
						, " group_as_string:", command->group_as_string
# નહીંતર
						, "", ""
# endif
				);
				debug_print_વૃક્ષ(command->group, lvl+1);
				prn++;
				ચાલુ;
			}
			જો (argv) જ્યારે (*argv) {
				fdprintf(2, " '%s'", *argv);
				argv++;
			}
#ifndef __U_BOOT__
			જો (command->redirects)
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

સ્થિર સંરચના pipe *new_pipe(શૂન્ય)
{
	સંરચના pipe *pi;
	pi = xzalloc(કદ(સંરચના pipe));
	/*pi->res_word = RES_NONE; - RES_NONE is 0 anyway */
	પાછા pi;
}

/* Command (member of a pipe) is complete, or we start a new pipe
 * if ctx->command is NULL.
 * No errors possible here.
 */
સ્થિર પૂર્ણાંક done_command(સંરચના parse_context *ctx)
{
	/* The command is really already in the pipe structure, so
	 * advance the pipe counter and make a new, null command. */
	સંરચના pipe *pi = ctx->pipe;
	સંરચના command *command = ctx->command;

#જો 0	/* Instead we emit error message at run time */
	જો (ctx->pending_redirect) {
		/* For example, "cmd >" (no filename to redirect to) */
		syntax_error("invalid redirect");
		ctx->pending_redirect = ખાલી;
	}
#endif

	જો (command) {
		જો (IS_NULL_CMD(command)) {
			debug_printf_parse("done_command: skipping null cmd, num_cmds=%d\n", pi->અંક_cmds);
			જાઓ clear_and_ret;
		}
		pi->અંક_cmds++;
		debug_printf_parse("done_command: ++num_cmds=%d\n", pi->અંક_cmds);
		//debug_print_tree(ctx->list_head, 20);
	} નહીંતર {
		debug_printf_parse("done_command: initializing, num_cmds=%d\n", pi->અંક_cmds);
	}

	/* Only real trickiness here is that the uncommitted
	 * command structure is not counted in pi->num_cmds. */
	pi->cmds = xrealloc(pi->cmds, કદ(*pi->cmds) * (pi->અંક_cmds+1));
	ctx->command = command = &pi->cmds[pi->અંક_cmds];
 clear_and_ret:
	memset(command, 0, કદ(*command));
#જો ENABLE_HUSH_LINENO_VAR
	command->lineno = G.parse_lineno;
	debug_printf_parse("command->lineno = G.parse_lineno (%u)\n", G.parse_lineno);
#endif
	પાછા pi->અંક_cmds; /* used only for 0/nonzero check */
}

સ્થિર શૂન્ય done_pipe(સંરચના parse_context *ctx, pipe_style type)
{
	પૂર્ણાંક not_null;

	debug_printf_parse("done_pipe entered, followup %d\n", type);
	/* Close previous command */
	not_null = done_command(ctx);
#જો HAS_KEYWORDS
	ctx->pipe->pi_inverted = ctx->ctx_inverted;
	ctx->ctx_inverted = 0;
	ctx->pipe->res_word = ctx->ctx_res_w;
#endif
	જો (type == PIPE_BG && ctx->list_head != ctx->pipe) {
		/* Necessary since && and || have precedence over &:
		 * "cmd1 && cmd2 &" must spawn both cmds, not only cmd2,
		 * in a backgrounded subshell.
		 */
		સંરચના pipe *pi;
		સંરચના command *command;

		/* Is this actually this construct, all pipes end with && or ||? */
		pi = ctx->list_head;
		જ્યારે (pi != ctx->pipe) {
			જો (pi->followup != PIPE_AND && pi->followup != PIPE_OR)
				જાઓ no_conv;
			pi = pi->next;
		}

		debug_printf_parse("BG with more than one pipe, converting to { p1 &&...pN; } &\n");
		pi->followup = PIPE_SEQ; /* close pN _not_ with "&"! */
		pi = xzalloc(કદ(*pi));
		pi->followup = PIPE_BG;
		pi->અંક_cmds = 1;
		pi->cmds = xzalloc(કદ(pi->cmds[0]));
		command = &pi->cmds[0];
		જો (CMD_NORMAL != 0) /* "if xzalloc didn't do that already" */
			command->cmd_type = CMD_NORMAL;
		command->group = ctx->list_head;
#જો !BB_MMU
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
	} નહીંતર {
 no_conv:
		ctx->pipe->followup = type;
	}

	/* Without this check, even just <enter> on command line generates
	 * tree of three NOPs (!). Which is harmless but annoying.
	 * IOW: it is safe to do it unconditionally. */
	જો (not_null
#જો ENABLE_HUSH_IF
	 || ctx->ctx_res_w == RES_FI
#endif
#જો ENABLE_HUSH_LOOPS
	 || ctx->ctx_res_w == RES_DONE
	 || ctx->ctx_res_w == RES_FOR
	 || ctx->ctx_res_w == RES_IN
#endif
#જો ENABLE_HUSH_CASE
	 || ctx->ctx_res_w == RES_ESAC
#endif
	) {
		સંરચના pipe *new_p;
		debug_printf_parse("done_pipe: adding new pipe: "
#ifndef __U_BOOT__
				"not_null:%d ctx->ctx_res_w:%d\n",
				not_null, ctx->ctx_res_w);
#નહીંતર /* __U_BOOT__ */
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
#જો ENABLE_HUSH_LOOPS
		જો (ctx->ctx_res_w == RES_FOR
		 || ctx->ctx_res_w == RES_IN)
			ctx->ctx_res_w = RES_NONE;
#endif
#જો ENABLE_HUSH_CASE
		જો (ctx->ctx_res_w == RES_MATCH)
			ctx->ctx_res_w = RES_CASE_BODY;
		જો (ctx->ctx_res_w == RES_CASE)
			ctx->ctx_res_w = RES_CASE_IN;
#endif
		ctx->command = ખાલી; /* trick done_command below */
		/* Create the memory for command, roughly:
		 * ctx->pipe->cmds = new struct command;
		 * ctx->command = &ctx->pipe->cmds[0];
		 */
		done_command(ctx);
		//debug_print_tree(ctx->list_head, 10);
	}
	debug_printf_parse("done_pipe return\n");
}

સ્થિર શૂન્ય initialize_context(સંરચના parse_context *ctx)
{
	memset(ctx, 0, કદ(*ctx));
	જો (MAYBE_ASSIGNMENT != 0)
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
#જો HAS_KEYWORDS
સંરચના reserved_combo {
	અક્ષર literal[6];
	unsigned અક્ષર res;
	unsigned અક્ષર assignment_flag;
	uint32_t flag;
};
ગણના {
	FLAG_END   = (1 << RES_NONE ),
# જો ENABLE_HUSH_IF
	FLAG_IF    = (1 << RES_IF   ),
	FLAG_THEN  = (1 << RES_THEN ),
	FLAG_ELIF  = (1 << RES_ELIF ),
	FLAG_ELSE  = (1 << RES_ELSE ),
	FLAG_FI    = (1 << RES_FI   ),
# endif
# જો ENABLE_HUSH_LOOPS
	FLAG_FOR   = (1 << RES_FOR  ),
	FLAG_WHILE = (1 << RES_WHILE),
	FLAG_UNTIL = (1 << RES_UNTIL),
	FLAG_DO    = (1 << RES_DO   ),
	FLAG_DONE  = (1 << RES_DONE ),
	FLAG_IN    = (1 << RES_IN   ),
# endif
# જો ENABLE_HUSH_CASE
	FLAG_MATCH = (1 << RES_MATCH),
	FLAG_ESAC  = (1 << RES_ESAC ),
# endif
	FLAG_START = (1 << RES_XXXX ),
};

સ્થિર અચલ સંરચના reserved_combo* match_reserved_word(o_string *word)
{
	/* Mostly a list of accepted follow-up reserved words.
	 * FLAG_END means we are done with the sequence, and are ready
	 * to turn the compound list into a command.
	 * FLAG_START means the word must start a new compound list.
	 */
	સ્થિર અચલ સંરચના reserved_combo reserved_list[] ALIGN4 = {
# જો ENABLE_HUSH_IF
		{ "!",     RES_NONE,  NOT_ASSIGNMENT  , 0 },
		{ "if",    RES_IF,    MAYBE_ASSIGNMENT, FLAG_THEN | FLAG_START },
		{ "then",  RES_THEN,  MAYBE_ASSIGNMENT, FLAG_ELIF | FLAG_ELSE | FLAG_FI },
		{ "elif",  RES_ELIF,  MAYBE_ASSIGNMENT, FLAG_THEN },
		{ "else",  RES_ELSE,  MAYBE_ASSIGNMENT, FLAG_FI   },
		{ "fi",    RES_FI,    NOT_ASSIGNMENT  , FLAG_END  },
# endif
# જો ENABLE_HUSH_LOOPS
		{ "for",   RES_FOR,   NOT_ASSIGNMENT  , FLAG_IN | FLAG_DO | FLAG_START },
		{ "while", RES_WHILE, MAYBE_ASSIGNMENT, FLAG_DO | FLAG_START },
		{ "until", RES_UNTIL, MAYBE_ASSIGNMENT, FLAG_DO | FLAG_START },
		{ "in",    RES_IN,    NOT_ASSIGNMENT  , FLAG_DO   },
		{ "do",    RES_DO,    MAYBE_ASSIGNMENT, FLAG_DONE },
		{ "done",  RES_DONE,  NOT_ASSIGNMENT  , FLAG_END  },
# endif
# જો ENABLE_HUSH_CASE
		{ "case",  RES_CASE,  NOT_ASSIGNMENT  , FLAG_MATCH | FLAG_START },
		{ "esac",  RES_ESAC,  NOT_ASSIGNMENT  , FLAG_END  },
# endif
	};
	અચલ સંરચના reserved_combo *r;

	માટે (r = reserved_list; r < reserved_list + ARRAY_SIZE(reserved_list); r++) {
		જો (strcmp(word->data, r->literal) == 0)
			પાછા r;
	}
	પાછા ખાલી;
}
/* Return NULL: not a keyword, else: keyword
 */
સ્થિર અચલ સંરચના reserved_combo* reserved_word(સંરચના parse_context *ctx)
{
# જો ENABLE_HUSH_CASE
	સ્થિર અચલ સંરચના reserved_combo reserved_match = {
		"",        RES_MATCH, NOT_ASSIGNMENT , FLAG_MATCH | FLAG_ESAC
	};
# endif
	અચલ સંરચના reserved_combo *r;

	જો (ctx->word.has_quoted_part)
		પાછા 0;
	r = match_reserved_word(&ctx->word);
	જો (!r)
		પાછા r; /* NULL */

	debug_printf("found reserved word %s, res %d\n", r->literal, r->res);
# જો ENABLE_HUSH_CASE
	જો (r->res == RES_IN && ctx->ctx_res_w == RES_CASE_IN) {
		/* "case word IN ..." - IN part starts first MATCH part */
		r = &reserved_match;
	} નહીંતર
# endif
	જો (r->flag == 0) { /* '!' */
		જો (ctx->ctx_inverted) { /* bash doesn't accept '! ! true' */
			syntax_error("! ! command");
			ctx->ctx_res_w = RES_SNTX;
		}
		ctx->ctx_inverted = 1;
		પાછા r;
	}
	જો (r->flag & FLAG_START) {
		સંરચના parse_context *old;

		old = xmemdup(ctx, કદ(*ctx));
		debug_printf_parse("push stack %p\n", old);
		initialize_context(ctx);
		ctx->ઢગલો = old;
	} નહીંતર જો (/*ctx->ctx_res_w == RES_NONE ||*/ !(ctx->old_flag & (1 << r->res))) {
		syntax_error_at(ctx->word.data);
		ctx->ctx_res_w = RES_SNTX;
		પાછા r;
	} નહીંતર {
		/* "{...} fi" is ok. "{...} if" is not
		 * Example:
		 * if { echo foo; } then { echo bar; } fi */
		જો (ctx->command->group)
			done_pipe(ctx, PIPE_SEQ);
	}

	ctx->ctx_res_w = r->res;
	ctx->old_flag = r->flag;
	ctx->is_assignment = r->assignment_flag;
	debug_printf_parse("ctx->is_assignment='%s'\n", assignment_flag[ctx->is_assignment]);

	જો (ctx->old_flag & FLAG_END) {
		સંરચના parse_context *old;

		done_pipe(ctx, PIPE_SEQ);
		debug_printf_parse("pop stack %p\n", ctx->ઢગલો);
		old = ctx->ઢગલો;
		old->command->group = ctx->list_head;
		old->command->cmd_type = CMD_NORMAL;
# જો !BB_MMU
		/* At this point, the compound command's string is in
		 * ctx->as_string... except for the leading keyword!
		 * Consider this example: "echo a | if true; then echo a; fi"
		 * ctx->as_string will contain "true; then echo a; fi",
		 * with "if " remaining in old->as_string!
		 */
		{
			અક્ષર *str;
			પૂર્ણાંક len = old->as_string.length;
			/* Concatenate halves */
			o_addstr(&old->as_string, ctx->as_string.data);
			o_free(&ctx->as_string);
			/* Find where leading keyword starts in first half */
			str = old->as_string.data + len;
			જો (str > old->as_string.data)
				str--; /* skip whitespace after keyword */
			જ્યારે (str > old->as_string.data && isalpha(str[-1]))
				str--;
			/* Ugh, we're done with this horrid hack */
			old->command->group_as_string = xstrdup(str);
			debug_printf_parse("pop, remembering as:'%s'\n",
					old->command->group_as_string);
		}
# endif
		*ctx = *old;   /* physical copy */
		મુક્ત(old);
	}
	પાછા r;
}
#endif /* HAS_KEYWORDS */

/* Word is complete, look at it and update parsing context.
 * Normal return is 0. Syntax errors return 1.
 * Note: on return, word is reset, but not o_free'd!
 */
સ્થિર પૂર્ણાંક done_word(સંરચના parse_context *ctx)
{
	સંરચના command *command = ctx->command;

	debug_printf_parse("done_word entered: '%s' %p\n", ctx->word.data, command);
	જો (ctx->word.length == 0 && !ctx->word.has_quoted_part) {
		debug_printf_parse("done_word return 0: true null, ignored\n");
		પાછા 0;
	}

#ifndef __U_BOOT__
	જો (ctx->pending_redirect) {
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
		જો (ctx->pending_redirect->rd_type == REDIRECT_HEREDOC) {
			unbackslash(ctx->pending_redirect->rd_filename);
			/* Is it <<"HEREDOC"? */
			જો (ctx->word.has_quoted_part) {
				ctx->pending_redirect->rd_dup |= HEREDOC_QUOTED;
			}
		}
		debug_printf_parse("word stored in rd_filename: '%s'\n", ctx->word.data);
		ctx->pending_redirect = ખાલી;
	} નહીંતર {
#endif /* !__U_BOOT__ */
#જો HAS_KEYWORDS
# જો ENABLE_HUSH_CASE
		જો (ctx->ctx_dsemicolon
		 && strcmp(ctx->word.data, "esac") != 0 /* not "... pattern) cmd;; esac" */
		) {
			/* already done when ctx_dsemicolon was set to 1: */
			/* ctx->ctx_res_w = RES_MATCH; */
			ctx->ctx_dsemicolon = 0;
		} નહીંતર
# endif
# જો defined(CMD_TEST2_SINGLEWORD_NOGLOB)
		જો (command->cmd_type == CMD_TEST2_SINGLEWORD_NOGLOB
		 && strcmp(ctx->word.data, "]]") == 0
		) {
			/* allow "[[ ]] >file" etc */
			command->cmd_type = CMD_SINGLEWORD_NOGLOB;
		} નહીંતર
# endif
		જો (!command->argv /* if it's the first word... */
# જો ENABLE_HUSH_LOOPS
		 && ctx->ctx_res_w != RES_FOR /* ...not after FOR or IN */
		 && ctx->ctx_res_w != RES_IN
# endif
# જો ENABLE_HUSH_CASE
		 && ctx->ctx_res_w != RES_CASE
# endif
		) {
			અચલ સંરચના reserved_combo *reserved;
			reserved = reserved_word(ctx);
			debug_printf_parse("checking for reserved-ness: %d\n", !!reserved);
			જો (reserved) {
# જો ENABLE_HUSH_LINENO_VAR
/* Case:
 * "while ...; do
 *	cmd ..."
 * If we don't close the pipe _now_, immediately after "do", lineno logic
 * sees "cmd" as starting at "do" - i.e., at the previous line.
 */
				જો (0
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
				પાછા (ctx->ctx_res_w == RES_SNTX);
			}
# જો defined(CMD_TEST2_SINGLEWORD_NOGLOB)
			જો (strcmp(ctx->word.data, "[[") == 0) {
				command->cmd_type = CMD_TEST2_SINGLEWORD_NOGLOB;
			} નહીંતર
# endif
# જો defined(CMD_SINGLEWORD_NOGLOB)
			જો (0
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
# નહીંતર
			{ /* empty block to pair "if ... else" */ }
# endif
		}
#endif /* HAS_KEYWORDS */

		જો (command->group) {
			/* "{ echo foo; } echo bar" - bad */
			syntax_error_at(ctx->word.data);
			debug_printf_parse("done_word return 1: syntax error, "
					"groups and arglists don't mix\n");
			પાછા 1;
		}

		/* If this word wasn't an assignment, next ones definitely
		 * can't be assignments. Even if they look like ones. */
		જો (ctx->is_assignment != DEFINITELY_ASSIGNMENT
		 && ctx->is_assignment != WORD_IS_KEYWORD
		) {
			ctx->is_assignment = NOT_ASSIGNMENT;
		} નહીંતર {
			જો (ctx->is_assignment == DEFINITELY_ASSIGNMENT) {
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

#જો ENABLE_HUSH_LOOPS
	જો (ctx->ctx_res_w == RES_FOR) {
		જો (ctx->word.has_quoted_part
		 || endofname(command->argv[0])[0] != '\0'
		) {
			/* bash says just "not a valid identifier" */
			syntax_error("bad for loop variable");
			પાછા 1;
		}
		/* Force FOR to have just one word (variable name) */
		/* NB: basically, this makes hush see "for v in ..."
		 * syntax as if it is "for v; in ...". FOR and IN become
		 * two pipe structs in parse tree. */
		done_pipe(ctx, PIPE_SEQ);
	}
#endif
#જો ENABLE_HUSH_CASE
	/* Force CASE to have just one word */
	જો (ctx->ctx_res_w == RES_CASE) {
		done_pipe(ctx, PIPE_SEQ);
	}
#endif

	o_reset_to_empty_unquoted(&ctx->word);

	debug_printf_parse("done_word return 0\n");
	પાછા 0;
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
#જો BB_MMU
#define parse_redir_જમણું_fd(as_string, આગત) \
	parse_redir_જમણું_fd(આગત)
#endif
સ્થિર પૂર્ણાંક parse_redir_જમણું_fd(o_string *as_string, સંરચના in_str *આગત)
{
	પૂર્ણાંક ch, d, ok;

	ch = i_peek(આગત);
	જો (ch != '&')
		પાછા REDIRFD_TO_FILE;

	ch = i_getch(આગત);  /* get the & */
	nommu_addchr(as_string, ch);
	ch = i_peek(આગત);
	જો (ch == '-') {
		ch = i_getch(આગત);
		nommu_addchr(as_string, ch);
		પાછા REDIRFD_CLOSE;
	}
	d = 0;
	ok = 0;
	જ્યારે (ch != EOF && isdigit(ch)) {
		d = d*10 + (ch-'0');
		ok = 1;
		ch = i_getch(આગત);
		nommu_addchr(as_string, ch);
		ch = i_peek(આગત);
	}
	જો (ok) પાછા d;

//TODO: this is the place to catch ">&file" bashism (redirect both fd 1 and 2)

	bb_simple_error_સંદેશ("ambiguous redirect");
	પાછા REDIRFD_SYNTAX_ERR;
}

/* Return code is 0 normal, 1 if a syntax error is detected
 */
સ્થિર પૂર્ણાંક parse_redirect(સંરચના parse_context *ctx,
		પૂર્ણાંક fd,
		redir_type style,
		સંરચના in_str *આગત)
{
	સંરચના command *command = ctx->command;
	સંરચના redir_struct *redir;
	સંરચના redir_struct **redirp;
	પૂર્ણાંક dup_અંક;

	dup_અંક = REDIRFD_TO_FILE;
	જો (style != REDIRECT_HEREDOC) {
		/* Check for a '>&1' type redirect */
		dup_અંક = parse_redir_જમણું_fd(&ctx->as_string, આગત);
		જો (dup_અંક == REDIRFD_SYNTAX_ERR)
			પાછા 1;
	} નહીંતર {
		પૂર્ણાંક ch = i_peek_and_eat_bkslash_nl(આગત);
		dup_અંક = (ch == '-'); /* HEREDOC_SKIPTABS bit is 1 */
		જો (dup_અંક) { /* <<-... */
			ch = i_getch(આગત);
			nommu_addchr(&ctx->as_string, ch);
			ch = i_peek(આગત);
		}
	}

	જો (style == REDIRECT_OVERWRITE && dup_અંક == REDIRFD_TO_FILE) {
		પૂર્ણાંક ch = i_peek_and_eat_bkslash_nl(આગત);
		જો (ch == '|') {
			/* >|FILE redirect ("clobbering" >).
			 * Since we do not support "set -o noclobber" yet,
			 * >| and > are the same for now. Just eat |.
			 */
			ch = i_getch(આગત);
			nommu_addchr(&ctx->as_string, ch);
		}
	}

	/* Create a new redir_struct and append it to the linked list */
	redirp = &command->redirects;
	જ્યારે ((redir = *redirp) != ખાલી) {
		redirp = &(redir->next);
	}
	*redirp = redir = xzalloc(કદ(*redir));
	/* redir->next = NULL; */
	/* redir->rd_filename = NULL; */
	redir->rd_type = style;
	redir->rd_fd = (fd == -1) ? redir_table[style].default_fd : fd;

	debug_printf_parse("redirect type %d %s\n", redir->rd_fd,
				redir_table[style].descrip);

	redir->rd_dup = dup_અંક;
	જો (style != REDIRECT_HEREDOC && dup_અંક != REDIRFD_TO_FILE) {
		/* Erik had a check here that the file descriptor in question
		 * is legit; I postpone that to "run time"
		 * A "-" representation of "close me" shows up as a -3 here */
		debug_printf_parse("duplicating redirect '%d>&%d'\n",
				redir->rd_fd, redir->rd_dup);
	} નહીંતર {
#જો 0		/* Instead we emit error message at run time */
		જો (ctx->pending_redirect) {
			/* For example, "cmd > <file" */
			syntax_error("invalid redirect");
		}
#endif
		/* Set ctx->pending_redirect, so we know what to do at the
		 * end of the next parsed word. */
		ctx->pending_redirect = redir;
	}
	પાછા 0;
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
સ્થિર પૂર્ણાંક redirect_opt_અંક(o_string *o)
{
	પૂર્ણાંક અંક;

	જો (o->data == ખાલી)
		પાછા -1;
	અંક = bb_strtou(o->data, ખાલી, 10);
	જો (errno || અંક < 0)
		પાછા -1;
	o_reset_to_empty_unquoted(o);
	પાછા અંક;
}

#જો BB_MMU
#define fetch_till_str(as_string, આગત, word, skip_tabs) \
	fetch_till_str(આગત, word, skip_tabs)
#endif
સ્થિર અક્ષર *fetch_till_str(o_string *as_string,
		સંરચના in_str *આગત,
		અચલ અક્ષર *word,
		પૂર્ણાંક heredoc_flags)
{
	o_string heredoc = NULL_O_STRING;
	unsigned past_EOL;
	પૂર્ણાંક prev = 0; /* not \ */
	પૂર્ણાંક ch;

	/* Starting with "" is necessary for this case:
	 * cat <<EOF
	 *
	 * xxx
	 * EOF
	 */
	heredoc.data = xzalloc(1); /* start as "", not as NULL */

	જાઓ jump_in;

	જ્યારે (1) {
		ch = i_getch(આગત);
		જો (ch != EOF)
			nommu_addchr(as_string, ch);
		જો (ch == '\n' || ch == EOF) {
 check_heredoc_end:
#ifndef __U_BOOT__
			જો ((heredoc_flags & HEREDOC_QUOTED) || prev != '\\') {
#નહીંતર /* __U_BOOT__ */
			જો (prev != '\\') {
#endif
				/* End-of-line, and not a line continuation */
				જો (strcmp(heredoc.data + past_EOL, word) == 0) {
					heredoc.data[past_EOL] = '\0';
					debug_printf_heredoc("parsed '%s' heredoc '%s'\n", word, heredoc.data);
					પાછા heredoc.data;
				}
				જો (ch == '\n') {
					/* This is a new line.
					 * Remember position and backslash-escaping status.
					 */
					o_addchr(&heredoc, ch);
					prev = ch;
 jump_in:
					past_EOL = heredoc.length;
					/* Get 1st char of next line, possibly skipping leading tabs */
					કરો {
						ch = i_getch(આગત);
						જો (ch != EOF)
							nommu_addchr(as_string, ch);
#ifndef __U_BOOT__
					} જ્યારે ((heredoc_flags & HEREDOC_SKIPTABS) && ch == '\t');
#નહીંતર /* __U_BOOT__ */
				} જ્યારે (ch == '\t');
#endif
					/* If this immediately ended the line,
					 * go back to end-of-line checks.
					 */
					જો (ch == '\n')
						જાઓ check_heredoc_end;
				}
			} નહીંતર {
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
				ચાલુ;
			}
		}
		જો (ch == EOF) {
			o_free(&heredoc);
			પાછા ખાલી; /* error */
		}
		o_addchr(&heredoc, ch);
		nommu_addchr(as_string, ch);
		જો (prev == '\\' && ch == '\\')
			/* Correctly handle foo\\<eol> (not a line cont.) */
			prev = 0; /* not '\' */
		નહીંતર
			prev = ch;
	}
}
#endif /* !__U_BOOT__ */

/* Look at entire parse tree for not-yet-loaded REDIRECT_HEREDOCs
 * and load them all. There should be exactly heredoc_cnt of them.
 */
#જો BB_MMU
#define fetch_heredocs(as_string, pi, heredoc_cnt, આગત) \
	fetch_heredocs(pi, heredoc_cnt, આગત)
#endif
સ્થિર પૂર્ણાંક fetch_heredocs(o_string *as_string, સંરચના pipe *pi, પૂર્ણાંક heredoc_cnt, સંરચના in_str *આગત)
{
	જ્યારે (pi && heredoc_cnt) {
		પૂર્ણાંક i;
		સંરચના command *cmd = pi->cmds;

		debug_printf_heredoc("fetch_heredocs: num_cmds:%d cmd argv0:'%s'\n",
				pi->અંક_cmds,
				cmd->argv ? cmd->argv[0] : "NONE"
		);
		માટે (i = 0; i < pi->અંક_cmds; i++) {
#ifndef __U_BOOT__
			સંરચના redir_struct *redir = cmd->redirects;

#endif /* !__U_BOOT__ */
			debug_printf_heredoc("fetch_heredocs: %d cmd argv0:'%s'\n",
					i, cmd->argv ? cmd->argv[0] : "NONE");
#ifndef __U_BOOT__
			જ્યારે (redir) {
				જો (redir->rd_type == REDIRECT_HEREDOC) {
					અક્ષર *p;

					redir->rd_type = REDIRECT_HEREDOC2;
					/* redir->rd_dup is (ab)used to indicate <<- */
					p = fetch_till_str(as_string, આગત,
							redir->rd_filename, redir->rd_dup);
					જો (!p) {
						syntax_error("unexpected EOF in here document");
						પાછા -1;
					}
					મુક્ત(redir->rd_filename);
					redir->rd_filename = p;
					heredoc_cnt--;
				}
				redir = redir->next;
			}
#endif /* !__U_BOOT__ */
			જો (cmd->group) {
				//bb_error_msg("%s:%u heredoc_cnt:%d", __func__, __LINE__, heredoc_cnt);
				heredoc_cnt = fetch_heredocs(as_string, cmd->group, heredoc_cnt, આગત);
				//bb_error_msg("%s:%u heredoc_cnt:%d", __func__, __LINE__, heredoc_cnt);
				જો (heredoc_cnt < 0)
					પાછા heredoc_cnt; /* error */
			}
			cmd++;
		}
		pi = pi->next;
	}
	પાછા heredoc_cnt;
}

સ્થિર પૂર્ણાંક run_list(સંરચના pipe *pi);
#જો BB_MMU
#define parse_stream(pstring, heredoc_cnt_ptr, આગત, end_trigger) \
	parse_stream(heredoc_cnt_ptr, આગત, end_trigger)
#endif
સ્થિર સંરચના pipe *parse_stream(અક્ષર **pstring,
		પૂર્ણાંક *heredoc_cnt_ptr,
		સંરચના in_str *આગત,
		પૂર્ણાંક end_trigger);

/* Returns number of heredocs not yet consumed,
 * or -1 on error.
 */
સ્થિર પૂર્ણાંક parse_group(સંરચના parse_context *ctx,
		સંરચના in_str *આગત, પૂર્ણાંક ch)
{
	/* ctx->word contains characters seen prior to ( or {.
	 * Typically it's empty, but for function defs,
	 * it contains function name (without '()'). */
#જો BB_MMU
# define as_string ખાલી
#નહીંતર
	અક્ષર *as_string = ખાલી;
#endif
	સંરચના pipe *pipe_list;
	પૂર્ણાંક heredoc_cnt = 0;
	પૂર્ણાંક endch;
	સંરચના command *command = ctx->command;

	debug_printf_parse("parse_group entered\n");
#જો ENABLE_HUSH_FUNCTIONS
	જો (ch == '(' && !ctx->word.has_quoted_part) {
		જો (ctx->word.length)
			જો (done_word(ctx))
				પાછા -1;
		જો (!command->argv)
			જાઓ skip; /* (... */
		જો (command->argv[1]) { /* word word ... (... */
			syntax_error_unexpected_ch('(');
			પાછા -1;
		}
		/* it is "word(..." or "word (..." */
		કરો
			ch = i_getch(આગત);
		જ્યારે (ch == ' ' || ch == '\t');
		જો (ch != ')') {
			syntax_error_unexpected_ch(ch);
			પાછા -1;
		}
		nommu_addchr(&ctx->as_string, ch);
		કરો
			ch = i_getch(આગત);
		જ્યારે (ch == ' ' || ch == '\t' || ch == '\n');
		જો (ch != '{' && ch != '(') {
			syntax_error_unexpected_ch(ch);
			પાછા -1;
		}
//bash allows functions named "123", "..", "return"!
//		if (endofname(command->argv[0])[0] != '\0') {
//			syntax_error("bad function name");
//			return -1;
//		}
		nommu_addchr(&ctx->as_string, ch);
		command->cmd_type = CMD_FUNCDEF;
		જાઓ skip;
	}
#endif

#જો 0 /* Prevented by caller */
	જો (command->argv /* word [word]{... */
	 || ctx->word.length /* word{... */
	 || ctx->word.has_quoted_part /* ""{... */
	) {
		syntax_error(ખાલી);
		debug_printf_parse("parse_group return -1: "
			"syntax error, groups and arglists don't mix\n");
		પાછા -1;
	}
#endif

#ifndef __U_BOOT__
 IF_HUSH_FUNCTIONS(skip:)
#endif /* !__U_BOOT__ */

	endch = '}';
	જો (ch == '(') {
		endch = ')';
#ifndef __U_BOOT__
		IF_HUSH_FUNCTIONS(જો (command->cmd_type != CMD_FUNCDEF))
			command->cmd_type = CMD_SUBSHELL;
#endif /* !__U_BOOT__ */
	} નહીંતર {
		/* bash does not allow "{echo...", requires whitespace */
		ch = i_peek(આગત);
		જો (ch != ' ' && ch != '\t' && ch != '\n'
		 && ch != '('	/* but "{(..." is allowed (without whitespace) */
		) {
			syntax_error_unexpected_ch(ch);
			પાછા -1;
		}
		જો (ch != '(') {
			ch = i_getch(આગત);
			nommu_addchr(&ctx->as_string, ch);
		}
	}

	debug_printf_heredoc("calling parse_stream, heredoc_cnt:%d\n", heredoc_cnt);
	pipe_list = parse_stream(&as_string, &heredoc_cnt, આગત, endch);
	debug_printf_heredoc("parse_stream returned: heredoc_cnt:%d\n", heredoc_cnt);
#જો !BB_MMU
	જો (as_string)
		o_addstr(&ctx->as_string, as_string);
#endif

	/* empty ()/{} or parse error? */
	જો (!pipe_list || pipe_list == ERR_PTR) {
		/* parse_stream already emitted error msg */
		જો (!BB_MMU)
			મુક્ત(as_string);
		debug_printf_parse("parse_group return -1: "
			"parse_stream returned %p\n", pipe_list);
		પાછા -1;
	}
#જો !BB_MMU
	as_string[strlen(as_string) - 1] = '\0'; /* plink ')' or '}' */
	command->group_as_string = as_string;
	debug_printf_parse("end of group, remembering as:'%s'\n",
			command->group_as_string);
#endif

#જો ENABLE_HUSH_FUNCTIONS
	/* Convert "f() (cmds)" to "f() {(cmds)}" */
	જો (command->cmd_type == CMD_FUNCDEF && endch == ')') {
		સંરચના command *cmd2;

		cmd2 = xzalloc(કદ(*cmd2));
		cmd2->cmd_type = CMD_SUBSHELL;
		cmd2->group = pipe_list;
# જો !BB_MMU
//UNTESTED!
		cmd2->group_as_string = command->group_as_string;
		command->group_as_string = xasprintf("(%s)", command->group_as_string);
# endif

		pipe_list = new_pipe();
		pipe_list->cmds = cmd2;
		pipe_list->અંક_cmds = 1;
	}
#endif

	command->group = pipe_list;

	debug_printf_parse("parse_group return %d\n", heredoc_cnt);
	પાછા heredoc_cnt;
	/* command remains "open", available for possible redirects */
#undef as_string
}

#ifndef __U_BOOT__
#જો ENABLE_HUSH_TICK || ENABLE_FEATURE_SH_MATH || ENABLE_HUSH_DOLLAR_OPS
/* Subroutines for copying $(...) and `...` things */
/* '...' */
સ્થિર પૂર્ણાંક add_till_single_quote(o_string *dest, સંરચના in_str *આગત)
{
	જ્યારે (1) {
		પૂર્ણાંક ch = i_getch(આગત);
		જો (ch == EOF) {
			syntax_error_unterm_ch('\'');
			પાછા 0;
		}
		જો (ch == '\'')
			પાછા 1;
		o_addchr(dest, ch);
	}
}
સ્થિર પૂર્ણાંક add_till_single_quote_dquoted(o_string *dest, સંરચના in_str *આગત)
{
	જ્યારે (1) {
		પૂર્ણાંક ch = i_getch(આગત);
		જો (ch == EOF) {
			syntax_error_unterm_ch('\'');
			પાછા 0;
		}
		જો (ch == '\'')
			પાછા 1;
		o_addqchr(dest, ch);
	}
}

/* "...\"...`..`...." - do we need to handle "...$(..)..." too? */
સ્થિર પૂર્ણાંક add_till_backquote(o_string *dest, સંરચના in_str *આગત, પૂર્ણાંક in_dquote);
સ્થિર પૂર્ણાંક add_till_double_quote(o_string *dest, સંરચના in_str *આગત)
{
	જ્યારે (1) {
		પૂર્ણાંક ch = i_getch(આગત);
		જો (ch == EOF) {
			syntax_error_unterm_ch('"');
			પાછા 0;
		}
		જો (ch == '"')
			પાછા 1;
		જો (ch == '\\') {  /* \x. Copy both chars. */
			o_addchr(dest, ch);
			ch = i_getch(આગત);
		}
		o_addchr(dest, ch);
		જો (ch == '`') {
			જો (!add_till_backquote(dest, આગત, /*in_dquote:*/ 1))
				પાછા 0;
			o_addchr(dest, ch);
			ચાલુ;
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
સ્થિર પૂર્ણાંક add_till_backquote(o_string *dest, સંરચના in_str *આગત, પૂર્ણાંક in_dquote)
{
	જ્યારે (1) {
		પૂર્ણાંક ch = i_getch(આગત);
		જો (ch == '`')
			પાછા 1;
		જો (ch == '\\') {
			/* \x. Copy both unless it is \`, \$, \\ and maybe \" */
			ch = i_getch(આગત);
			જો (ch != '`'
			 && ch != '$'
			 && ch != '\\'
			 && (!in_dquote || ch != '"')
			) {
				o_addchr(dest, '\\');
			}
		}
		જો (ch == EOF) {
			syntax_error_unterm_ch('`');
			પાછા 0;
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
સ્થિર પૂર્ણાંક add_till_closing_bracket(o_string *dest, સંરચના in_str *આગત, unsigned end_ch)
{
	પૂર્ણાંક ch;
	અક્ષર dbl = end_ch & DOUBLE_CLOSE_CHAR_FLAG;
# જો BASH_SUBSTR || BASH_PATTERN_SUBST
	અક્ષર end_char2 = end_ch >> 8;
# endif
	end_ch &= (DOUBLE_CLOSE_CHAR_FLAG - 1);

# જો ENABLE_HUSH_INTERACTIVE
	G.promptmode = 1; /* PS2 */
# endif
	debug_printf_prompt("%s promptmode=%d\n", __func__, G.promptmode);

	જ્યારે (1) {
		ch = i_getch(આગત);
		જો (ch == EOF) {
			syntax_error_unterm_ch(end_ch);
			પાછા 0;
		}
		જો (ch == end_ch
# જો BASH_SUBSTR || BASH_PATTERN_SUBST
		 || ch == end_char2
# endif
		) {
			જો (!dbl)
				તોડો;
			/* we look for closing )) of $((EXPR)) */
			જો (i_peek_and_eat_bkslash_nl(આગત) == end_ch) {
				i_getch(આગત); /* eat second ')' */
				તોડો;
			}
		}
		o_addchr(dest, ch);
		//bb_error_msg("%s:o_addchr('%c')", __func__, ch);
		જો (ch == '(' || ch == '{') {
			ch = (ch == '(' ? ')' : '}');
			જો (!add_till_closing_bracket(dest, આગત, ch))
				પાછા 0;
			o_addchr(dest, ch);
			ચાલુ;
		}
		જો (ch == '\'') {
			જો (!add_till_single_quote(dest, આગત))
				પાછા 0;
			o_addchr(dest, ch);
			ચાલુ;
		}
		જો (ch == '"') {
			જો (!add_till_double_quote(dest, આગત))
				પાછા 0;
			o_addchr(dest, ch);
			ચાલુ;
		}
		જો (ch == '`') {
			જો (!add_till_backquote(dest, આગત, /*in_dquote:*/ 0))
				પાછા 0;
			o_addchr(dest, ch);
			ચાલુ;
		}
		જો (ch == '\\') {
			/* \x. Copy verbatim. Important for  \(, \) */
			ch = i_getch(આગત);
			જો (ch == EOF) {
				syntax_error_unterm_ch(end_ch);
				પાછા 0;
			}
# જો 0
			જો (ch == '\n') {
				/* "backslash+newline", ignore both */
				o_delchr(dest); /* undo insertion of '\' */
				ચાલુ;
			}
# endif
			o_addchr(dest, ch);
			//bb_error_msg("%s:o_addchr('%c') after '\\'", __func__, ch);
			ચાલુ;
		}
	}
	debug_printf_parse("%s return '%s' ch:'%c'\n", __func__, dest->data, ch);
	પાછા ch;
}
#endif /* ENABLE_HUSH_TICK || ENABLE_FEATURE_SH_MATH || ENABLE_HUSH_DOLLAR_OPS */

#જો BASH_DOLLAR_SQUOTE
/* Return code: 1 for "found and parsed", 0 for "seen something else" */
# જો BB_MMU
#define parse_dollar_squote(as_string, dest, આગત) \
	parse_dollar_squote(dest, આગત)
#define as_string ખાલી
# endif
સ્થિર પૂર્ણાંક parse_dollar_squote(o_string *as_string, o_string *dest, સંરચના in_str *આગત)
{
	પૂર્ણાંક start;
	પૂર્ણાંક ch = i_peek_and_eat_bkslash_nl(આગત);  /* first character after the $ */
	debug_printf_parse("parse_dollar_squote entered: ch='%c'\n", ch);
	જો (ch != '\'')
		પાછા 0;

	dest->has_quoted_part = 1;
	start = dest->length;

	ch = i_getch(આગત); /* eat ' */
	nommu_addchr(as_string, ch);
	જ્યારે (1) {
		ch = i_getch(આગત);
		nommu_addchr(as_string, ch);
		જો (ch == EOF) {
			syntax_error_unterm_ch('\'');
			પાછા 0;
		}
		જો (ch == '\'')
			તોડો;
		જો (ch == SPECIAL_VAR_પ્રતીક) {
			/* Convert raw ^C to corresponding special variable reference */
			o_addchr(dest, SPECIAL_VAR_પ્રતીક);
			o_addchr(dest, SPECIAL_VAR_QUOTED_SVS);
			/* will addchr() another SPECIAL_VAR_SYMBOL (see after the if() block) */
		} નહીંતર જો (ch == '\\') {
			સ્થિર અચલ અક્ષર C_escapes[] ALIGN1 = "nrbtfav""x\\01234567";

			ch = i_getch(આગત);
			nommu_addchr(as_string, ch);
			જો (strchr(C_escapes, ch)) {
				અક્ષર buf[4];
				અક્ષર *p = buf;
				પૂર્ણાંક cnt = 2;

				buf[0] = ch;
				જો ((unsigned અક્ષર)(ch - '0') <= 7) { /* \ooo */
					કરો {
						ch = i_peek(આગત);
						જો ((unsigned અક્ષર)(ch - '0') > 7)
							તોડો;
						*++p = ch = i_getch(આગત);
						nommu_addchr(as_string, ch);
					} જ્યારે (--cnt != 0);
				} નહીંતર જો (ch == 'x') { /* \xHH */
					કરો {
						ch = i_peek(આગત);
						જો (!isxdigit(ch))
							તોડો;
						*++p = ch = i_getch(આગત);
						nommu_addchr(as_string, ch);
					} જ્યારે (--cnt != 0);
					જો (cnt == 2) { /* \x but next char is "bad" */
						ch = 'x';
						જાઓ unrecognized;
					}
				} /* else simple seq like \\ or \t */
				*++p = '\0';
				p = buf;
				ch = bb_process_escape_sequence((શૂન્ય*)&p);
				//bb_error_msg("buf:'%s' ch:%x", buf, ch);
				જો (ch == '\0')
					ચાલુ; /* bash compat: $'...\0...' emits nothing */
			} નહીંતર { /* unrecognized "\z": encode both chars unless ' or " */
				જો (ch != '\'' && ch != '"') {
 unrecognized:
					o_addqchr(dest, '\\');
				}
			}
		} /* if (\...) */
		o_addqchr(dest, ch);
	}

	જો (dest->length == start) {
		/* $'', $'\0', $'\000\x00' and the like */
		o_addchr(dest, SPECIAL_VAR_પ્રતીક);
		o_addchr(dest, SPECIAL_VAR_પ્રતીક);
	}

	પાછા 1;
# undef as_string
}
#નહીંતર
# define parse_dollar_squote(as_string, dest, આગત) 0
#endif /* BASH_DOLLAR_SQUOTE */
#endif /* !__U_BOOT__ */

/* Return code: 0 for OK, 1 for syntax error */
#જો BB_MMU
#define parse_dollar(as_string, dest, આગત, quote_mask) \
	parse_dollar(dest, આગત, quote_mask)
#define as_string ખાલી
#endif
સ્થિર પૂર્ણાંક parse_dollar(o_string *as_string,
		o_string *dest,
		સંરચના in_str *આગત, unsigned અક્ષર quote_mask)
{
	પૂર્ણાંક ch = i_peek_and_eat_bkslash_nl(આગત);  /* first character after the $ */

	debug_printf_parse("parse_dollar entered: ch='%c' quote_mask:0x%x\n", ch, quote_mask);
	જો (isalpha(ch)) {
 make_var:
		ch = i_getch(આગત);
		nommu_addchr(as_string, ch);
 /*make_var1:*/
		o_addchr(dest, SPECIAL_VAR_પ્રતીક);
		જ્યારે (1) {
			debug_printf_parse(": '%c'\n", ch);
			o_addchr(dest, ch | quote_mask);
			quote_mask = 0;
			ch = i_peek_and_eat_bkslash_nl(આગત);
			જો (!isalnum(ch) && ch != '_') {
				/* End of variable name reached */
				તોડો;
			}
			ch = i_getch(આગત);
			nommu_addchr(as_string, ch);
		}
		o_addchr(dest, SPECIAL_VAR_પ્રતીક);
	} નહીંતર જો (isdigit(ch)) {
 make_one_char_var:
		ch = i_getch(આગત);
		nommu_addchr(as_string, ch);
		o_addchr(dest, SPECIAL_VAR_પ્રતીક);
		debug_printf_parse(": '%c'\n", ch);
		o_addchr(dest, ch | quote_mask);
		o_addchr(dest, SPECIAL_VAR_પ્રતીક);
	} નહીંતર પસંદગી (ch) {
#ifndef __U_BOOT__
	સ્થિતિ '$': /* pid */
	સ્થિતિ '!': /* last bg pid */
#endif /* !__U_BOOT__ */
	સ્થિતિ '?': /* last exit code */
	સ્થિતિ '#': /* number of args */
	સ્થિતિ '*': /* args */
	સ્થિતિ '@': /* args */
	સ્થિતિ '-': /* $- option flags set by set builtin or shell options (-i etc) */
		જાઓ make_one_char_var;
	સ્થિતિ '{': {
		અક્ષર len_single_ch;

		o_addchr(dest, SPECIAL_VAR_પ્રતીક);

		ch = i_getch(આગત); /* eat '{' */
		nommu_addchr(as_string, ch);

		ch = i_getch_and_eat_bkslash_nl(આગત); /* first char after '{' */
		/* It should be ${?}, or ${#var},
		 * or even ${?+subst} - operator acting on a special variable,
		 * or the beginning of variable name.
		 */
		જો (ch == EOF
		 || (!strchr(_SPECIAL_VARS_STR, ch) && !isalnum(ch)) /* not one of those */
		) {
 bad_dollar_syntax:
			syntax_error_unterm_str("${name}");
			debug_printf_parse("parse_dollar return 0: unterminated ${name}\n");
			પાછા 0;
		}
		nommu_addchr(as_string, ch);
		len_single_ch = ch;
		ch |= quote_mask;

		/* It's possible to just call add_till_closing_bracket() at this point.
		 * However, this regresses some of our testsuite cases
		 * which check invalid constructs like ${%}.
		 * Oh well... let's check that the var name part is fine... */

		જો (isdigit(len_single_ch)
		 || (len_single_ch == '#' && isdigit(i_peek_and_eat_bkslash_nl(આગત)))
		) {
			/* Execution engine uses plain xatoi_positive()
			 * to interpret ${NNN} and {#NNN},
			 * check syntax here in the parser.
			 * (bash does not support expressions in ${#NN},
			 * e.g. ${#$var} and {#1:+WORD} are not supported).
			 */
			unsigned cnt = 9; /* max 9 digits for ${NN} and 8 for {#NN} */
			જ્યારે (1) {
				o_addchr(dest, ch);
				debug_printf_parse(": '%c'\n", ch);
				ch = i_getch_and_eat_bkslash_nl(આગત);
				nommu_addchr(as_string, ch);
				જો (ch == '}')
					તોડો;
				જો (--cnt == 0)
					જાઓ bad_dollar_syntax;
				જો (len_single_ch != '#' && strchr(VAR_SUBST_OPS, ch))
					/* ${NN<op>...} is valid */
					જાઓ eat_until_closing;
				જો (!isdigit(ch))
					જાઓ bad_dollar_syntax;
			}
		} નહીંતર
		જ્યારે (1) {
			unsigned pos;

			o_addchr(dest, ch);
			debug_printf_parse(": '%c'\n", ch);

			ch = i_getch(આગત);
			nommu_addchr(as_string, ch);
			જો (ch == '}')
				તોડો;
#ifndef __U_BOOT__
			જો (!isalnum(ch) && ch != '_') {
#નહીંતર /* __U_BOOT__ */
			/*
			 * In several places in U-Boot, we use variable like
			 * foo# (e.g. serial#), particularly in env.
			 * So, we need to authorize # to appear inside
			 * variable name and then expand this variable.
			 * NOTE Having # in variable name is not permitted in
			 * upstream hush but expansion will be done (even though
			 * the result will be empty).
			 */
			જો (!isalnum(ch) && ch != '_' && ch != '#') {
#endif /* __U_BOOT__ */
				unsigned end_ch;
#ifndef __U_BOOT__
				unsigned અક્ષર last_ch;
#endif /* !__U_BOOT__ */
				/* handle parameter expansions
				 * http://www.opengroup.org/onlinepubs/009695399/utilities/xcu_chap02.html#tag_02_06_02
				 */
				જો (!strchr(VAR_SUBST_OPS, ch)) { /* ${var<bad_char>... */
					જો (len_single_ch != '#'
					/*|| !strchr(SPECIAL_VARS_STR, ch) - disallow errors like ${#+} ? */
					 || i_peek(આગત) != '}'
					) {
						જાઓ bad_dollar_syntax;
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
				જો (BASH_SUBSTR
				 && ch == ':'
				 && !strchr(MINUS_PLUS_EQUAL_QUESTION, i_peek(આગત))
				) {
					/* It's ${var:N[:M]} thing */
					end_ch = '}' * 0x100 + ':';
				}
				જો (BASH_PATTERN_SUBST
				 && ch == '/'
				) {
					/* It's ${var/[/]pattern[/repl]} thing */
					જો (i_peek(આગત) == '/') { /* ${var//pattern[/repl]}? */
						i_getch(આગત);
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
				જો (i_peek(આગત) == '/') {
					o_addchr(dest, i_getch(આગત));
				}
#ifndef __U_BOOT__
 again:
#endif /* !__U_BOOT__ */
				જો (!BB_MMU)
					pos = dest->length;
#જો ENABLE_HUSH_DOLLAR_OPS
#ifndef __U_BOOT__
				last_ch = add_till_closing_bracket(dest, આગત, end_ch);
				જો (last_ch == 0) /* error? */
					પાછા 0;
#endif /* !__U_BOOT__ */
#નહીંતર
# error Simple code to only allow ${var} is not implemented
#endif
				જો (as_string) {
					o_addstr(as_string, dest->data + pos);
#ifndef __U_BOOT__
					o_addchr(as_string, last_ch);
#endif /* !__U_BOOT__ */
				}

#ifndef __U_BOOT__
				જો ((BASH_SUBSTR || BASH_PATTERN_SUBST)
					 && (end_ch & 0xff00)
				) {
					/* close the first block: */
					o_addchr(dest, SPECIAL_VAR_પ્રતીક);
					/* while parsing N from ${var:N[:M]}
					 * or pattern from ${var/[/]pattern[/repl]} */
					જો ((end_ch & 0xff) == last_ch) {
						/* got ':' or '/'- parse the rest */
						end_ch = '}';
						જાઓ again;
					}
					/* got '}' */
					જો (BASH_SUBSTR && end_ch == '}' * 0x100 + ':') {
						/* it's ${var:N} - emulate :999999999 */
						o_addstr(dest, "999999999");
					} /* else: it's ${var/[/]pattern} */
				}
#endif /* !__U_BOOT__ */
				તોડો;
			}
			len_single_ch = 0; /* it can't be ${#C} op */
		}
		o_addchr(dest, SPECIAL_VAR_પ્રતીક);
		તોડો;
	}
#જો ENABLE_FEATURE_SH_MATH || ENABLE_HUSH_TICK
	સ્થિતિ '(': {
		unsigned pos;

		ch = i_getch(આગત);
		nommu_addchr(as_string, ch);
# જો ENABLE_FEATURE_SH_MATH
		જો (i_peek_and_eat_bkslash_nl(આગત) == '(') {
			ch = i_getch(આગત);
			nommu_addchr(as_string, ch);
			o_addchr(dest, SPECIAL_VAR_પ્રતીક);
			o_addchr(dest, quote_mask | '+');
			જો (!BB_MMU)
				pos = dest->length;
			જો (!add_till_closing_bracket(dest, આગત, ')' | DOUBLE_CLOSE_CHAR_FLAG))
				પાછા 0; /* error */
			જો (as_string) {
				o_addstr(as_string, dest->data + pos);
				o_addchr(as_string, ')');
				o_addchr(as_string, ')');
			}
			o_addchr(dest, SPECIAL_VAR_પ્રતીક);
			તોડો;
		}
# endif
# જો ENABLE_HUSH_TICK
		o_addchr(dest, SPECIAL_VAR_પ્રતીક);
		o_addchr(dest, quote_mask | '`');
		જો (!BB_MMU)
			pos = dest->length;
		જો (!add_till_closing_bracket(dest, આગત, ')'))
			પાછા 0; /* error */
		જો (as_string) {
			o_addstr(as_string, dest->data + pos);
			o_addchr(as_string, ')');
		}
		o_addchr(dest, SPECIAL_VAR_પ્રતીક);
# endif
		તોડો;
	}
#endif
	સ્થિતિ '_':
		જાઓ make_var;
#જો 0
	/* TODO: $_: */
	/* $_ Shell or shell script name; or last argument of last command
	 * (if last command wasn't a pipe; if it was, bash sets $_ to "");
	 * but in command's env, set to full pathname used to invoke it */
		ch = i_getch(આગત);
		nommu_addchr(as_string, ch);
		ch = i_peek_and_eat_bkslash_nl(આગત);
		જો (isalnum(ch)) { /* it's $_name or $_123 */
			ch = '_';
			જાઓ make_var1;
		}
		/* else: it's $_ */
#endif
	મૂળભૂત:
		o_addQchr(dest, '$');
	}
	debug_printf_parse("parse_dollar return 1 (ok)\n");
	પાછા 1;
#undef as_string
}

#જો BB_MMU
#define encode_string(as_string, dest, આગત, dquote_end) \
	encode_string(dest, આગત, dquote_end)
#define as_string ખાલી
#endif
સ્થિર પૂર્ણાંક encode_string(o_string *as_string,
		o_string *dest,
		સંરચના in_str *આગત,
		પૂર્ણાંક dquote_end)
{
	પૂર્ણાંક ch;
	પૂર્ણાંક next;

 again:
	ch = i_getch(આગત);
	જો (ch != EOF)
		nommu_addchr(as_string, ch);
	જો (ch == dquote_end) { /* may be only '"' or EOF */
		debug_printf_parse("encode_string return 1 (ok)\n");
		પાછા 1;
	}
	/* note: can't move it above ch == dquote_end check! */
	જો (ch == EOF) {
		syntax_error_unterm_ch('"');
		પાછા 0; /* error */
	}
	next = '\0';
	જો (ch != '\n') {
		next = i_peek(આગત);
	}
	debug_printf_parse("\" ch=%c (%d) escape=%d\n",
			ch, ch, !!(dest->o_expflags & EXP_FLAG_ESC_GLOB_CHARS));
	જો (ch == '\\') {
		જો (next == EOF) {
			/* Testcase: in interactive shell a file with
			 *  echo "unterminated string\<eof>
			 * is sourced.
			 */
			syntax_error_unterm_ch('"');
			પાછા 0; /* error */
		}
		/* bash:
		 * "The backslash retains its special meaning [in "..."]
		 * only when followed by one of the following characters:
		 * $, `, ", \, or <newline>.  A double quote may be quoted
		 * within double quotes by preceding it with a backslash."
		 * NB: in (unquoted) heredoc, above does not apply to ",
		 * therefore we check for it by "next == dquote_end" cond.
		 */
		જો (next == dquote_end || strchr("$`\\\n", next)) {
			ch = i_getch(આગત); /* eat next */
			જો (ch == '\n')
				જાઓ again; /* skip \<newline> */
		} /* else: ch remains == '\\', and we double it below: */
		o_addqchr(dest, ch); /* \c if c is a glob char, else just c */
		nommu_addchr(as_string, ch);
		જાઓ again;
	}
	જો (ch == '$') {
		//if (parse_dollar_squote(as_string, dest, input))
		//	goto again;
		જો (!parse_dollar(as_string, dest, આગત, /*quote_mask:*/ 0x80)) {
			debug_printf_parse("encode_string return 0: "
					"parse_dollar returned 0 (error)\n");
			પાછા 0;
		}
		જાઓ again;
	}
#જો ENABLE_HUSH_TICK
	જો (ch == '`') {
		//unsigned pos = dest->length;
		o_addchr(dest, SPECIAL_VAR_પ્રતીક);
		o_addchr(dest, 0x80 | '`');
		જો (!add_till_backquote(dest, આગત, /*in_dquote:*/ dquote_end == '"'))
			પાછા 0; /* error */
		o_addchr(dest, SPECIAL_VAR_પ્રતીક);
		//debug_printf_subst("SUBST RES3 '%s'\n", dest->data + pos);
		જાઓ again;
	}
#endif
	o_addQchr(dest, ch);
	જો (ch == SPECIAL_VAR_પ્રતીક) {
		/* Convert "^C" to corresponding special variable reference */
		o_addchr(dest, SPECIAL_VAR_QUOTED_SVS);
		o_addchr(dest, SPECIAL_VAR_પ્રતીક);
	}
	જાઓ again;
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
સ્થિર સંરચના pipe *parse_stream(અક્ષર **pstring,
		પૂર્ણાંક *heredoc_cnt_ptr,
		સંરચના in_str *આગત,
		પૂર્ણાંક end_trigger)
{
	સંરચના parse_context ctx;
	પૂર્ણાંક heredoc_cnt;

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
	જ્યારે (1) {
		અચલ અક્ષર *is_blank;
		અચલ અક્ષર *is_special;
		પૂર્ણાંક ch;
		પૂર્ણાંક next;
#ifndef __U_BOOT__
		પૂર્ણાંક redir_fd;
		redir_type redir_style;
#endif /* !__U_BOOT__ */

		ch = i_getch(આગત);
		debug_printf_parse(": ch=%c (%d) escape=%d\n",
				ch, ch, !!(ctx.word.o_expflags & EXP_FLAG_ESC_GLOB_CHARS));
		જો (ch == EOF) {
			સંરચના pipe *pi;

			જો (heredoc_cnt) {
				syntax_error_unterm_str("here document");
				જાઓ parse_error_exitcode1;
			}
			જો (end_trigger == ')') {
				syntax_error_unterm_ch('(');
				જાઓ parse_error_exitcode1;
			}
			જો (end_trigger == '}') {
				syntax_error_unterm_ch('{');
				જાઓ parse_error_exitcode1;
			}

			જો (done_word(&ctx)) {
				જાઓ parse_error_exitcode1;
			}
			o_free_and_set_NULL(&ctx.word);
			done_pipe(&ctx, PIPE_SEQ);

			/* Do we sit inside of any if's, loops or case's? */
			જો (HAS_KEYWORDS
			IF_HAS_KEYWORDS(&& (ctx.ctx_res_w != RES_NONE || ctx.old_flag != 0))
			) {
				syntax_error_unterm_str("compound statement");
				જાઓ parse_error_exitcode1;
			}

			pi = ctx.list_head;
			/* If we got nothing... */
			/* (this makes bare "&" cmd a no-op.
			 * bash says: "syntax error near unexpected token '&'") */
			જો (pi->અંક_cmds == 0
			IF_HAS_KEYWORDS(&& pi->res_word == RES_NONE)
			) {
				free_pipe_list(pi);
				pi = ખાલી;
			}
#જો !BB_MMU
			debug_printf_parse("as_string1 '%s'\n", ctx.as_string.data);
			જો (pstring)
				*pstring = ctx.as_string.data;
			નહીંતર
				o_free(&ctx.as_string);
#endif
			// heredoc_cnt must be 0 here anyway
			//if (heredoc_cnt_ptr)
			//	*heredoc_cnt_ptr = heredoc_cnt;
			debug_leave();
			debug_printf_heredoc("parse_stream return heredoc_cnt:%d\n", heredoc_cnt);
			debug_printf_parse("parse_stream return %p: EOF\n", pi);
			પાછા pi;
		}

		/* Handle "'" and "\" first, as they won't play nice with
		 * i_peek_and_eat_bkslash_nl() anyway:
		 *   echo z\\
		 * and
		 *   echo '\
		 *   '
		 * would break.
		 */
		જો (ch == '\\') {
			ch = i_getch(આગત);
			જો (ch == '\n')
				ચાલુ; /* drop \<newline>, get next char */
			nommu_addchr(&ctx.as_string, '\\');
			જો (ch == SPECIAL_VAR_પ્રતીક) {
				nommu_addchr(&ctx.as_string, ch);
				/* Convert \^C to corresponding special variable reference */
				જાઓ case_SPECIAL_VAR_પ્રતીક;
			}
			o_addchr(&ctx.word, '\\');
			જો (ch == EOF) {
				/* Testcase: eval 'echo Ok\' */
				/* bash-4.3.43 was removing backslash,
				 * but 4.4.19 retains it, most other shells too
				 */
				ચાલુ; /* get next char */
			}
			/* Example: echo Hello \2>file
			 * we need to know that word 2 is quoted
			 */
			ctx.word.has_quoted_part = 1;
			nommu_addchr(&ctx.as_string, ch);
			o_addchr(&ctx.word, ch);
			ચાલુ; /* get next char */
		}
		nommu_addchr(&ctx.as_string, ch);
		જો (ch == '\'') {
			ctx.word.has_quoted_part = 1;
			next = i_getch(આગત);
#ifndef __U_BOOT__
			જો (next == '\'' && !ctx.pending_redirect)
				જાઓ ઉમેરો_empty_quoted_str_marker;
#endif /* !__U_BOOT__ */

			ch = next;
			જ્યારે (1) {
				જો (ch == EOF) {
					syntax_error_unterm_ch('\'');
					જાઓ parse_error_exitcode1;
				}
				nommu_addchr(&ctx.as_string, ch);
				જો (ch == '\'')
					તોડો;
				જો (ch == SPECIAL_VAR_પ્રતીક) {
					/* Convert raw ^C to corresponding special variable reference */
					o_addchr(&ctx.word, SPECIAL_VAR_પ્રતીક);
					o_addchr(&ctx.word, SPECIAL_VAR_QUOTED_SVS);
				}
				o_addqchr(&ctx.word, ch);
				ch = i_getch(આગત);
			}
			ચાલુ; /* get next char */
		}

		next = '\0';
		જો (ch != '\n')
			next = i_peek_and_eat_bkslash_nl(આગત);

		is_special = "{}<>&|();#" /* special outside of "str" */
#ifndef __U_BOOT__
				"$\"" IF_HUSH_TICK("`") /* always special */
#નહીંતર /* __U_BOOT__ */
				"$\""
#endif /* __U_BOOT__ */
				SPECIAL_VAR_પ્રતીક_STR;
#જો defined(CMD_TEST2_SINGLEWORD_NOGLOB)
		જો (ctx.command->cmd_type == CMD_TEST2_SINGLEWORD_NOGLOB) {
			/* In [[ ]], {}<>&|() are not special */
			is_special += 8;
		} નહીંતર
#endif
		/* Are { and } special here? */
		જો (ctx.command->argv /* word [word]{... - non-special */
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

		જો (!is_special && !is_blank) { /* ordinary char */
 ordinary_char:
			o_addQchr(&ctx.word, ch);
			જો ((ctx.is_assignment == MAYBE_ASSIGNMENT
			    || ctx.is_assignment == WORD_IS_KEYWORD)
			 && ch == '='
			 && endofname(ctx.word.data)[0] == '='
			) {
				ctx.is_assignment = DEFINITELY_ASSIGNMENT;
				debug_printf_parse("ctx.is_assignment='%s'\n", assignment_flag[ctx.is_assignment]);
			}
			ચાલુ;
		}

		જો (is_blank) {
#જો ENABLE_HUSH_LINENO_VAR
/* Case:
 * "while ...; do<whitespace><newline>
 *	cmd ..."
 * would think that "cmd" starts in <whitespace> -
 * i.e., at the previous line.
 * We need to skip all whitespace before newlines.
 */
			જ્યારે (ch != '\n') {
				next = i_peek(આગત);
				જો (next != ' ' && next != '\t' && next != '\n')
					તોડો; /* next char is not ws */
				ch = i_getch(આગત);
			}
			/* ch == last eaten whitespace char */
#endif
			જો (done_word(&ctx)) {
				જાઓ parse_error_exitcode1;
			}
			જો (ch == '\n') {
				/* Is this a case when newline is simply ignored?
				 * Some examples:
				 * "cmd | <newline> cmd ..."
				 * "case ... in <newline> word) ..."
				 */
				જો (IS_NULL_CMD(ctx.command)
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
					સંરચના pipe *pi = ctx.list_head;
					જો (pi->અંક_cmds != 0       /* check #1 */
					 && pi->followup != PIPE_BG /* check #2 */
					) {
						ચાલુ;
					}
				}
				/* Treat newline as a command separator. */
				done_pipe(&ctx, PIPE_SEQ);
				debug_printf_heredoc("heredoc_cnt:%d\n", heredoc_cnt);
				જો (heredoc_cnt) {
					heredoc_cnt = fetch_heredocs(&ctx.as_string, ctx.list_head, heredoc_cnt, આગત);
					જો (heredoc_cnt != 0)
						જાઓ parse_error_exitcode1;
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
		જો (ch == '}') {
#નહીંતર /* __U_BOOT__ */
		જો (ch == '}' || ch == ')') {
#endif /* __U_BOOT__ */
			જો (ctx.word.length != 0 /* word} */
			 || ctx.word.has_quoted_part    /* ""} */
			) {
				જાઓ ordinary_char;
			}
			જો (!IS_NULL_CMD(ctx.command)) { /* cmd } */
				/* Generally, there should be semicolon: "cmd; }"
				 * However, bash allows to omit it if "cmd" is
				 * a group. Examples:
				 * { { echo 1; } }
				 * {(echo 1)}
				 * { echo 0 >&2 | { echo 1; } }
				 * { while false; do :; done }
				 * { case a in b) ;; esac }
				 */
				જો (ctx.command->group)
					જાઓ term_group;
				જાઓ ordinary_char;
			}
			જો (!IS_NULL_PIPE(ctx.pipe)) /* cmd | } */
				/* Can't be an end of {cmd}, skip the check */
				જાઓ skip_end_trigger;
			/* else: } does terminate a group */
		}
 term_group:
		જો (end_trigger && end_trigger == ch
		 && (ch != ';' || heredoc_cnt == 0)
#જો ENABLE_HUSH_CASE
		 && (ch != ')'
		    || ctx.ctx_res_w != RES_MATCH
		    || (!ctx.word.has_quoted_part && strcmp(ctx.word.data, "esac") == 0)
		    )
#endif
		) {
			જો (done_word(&ctx)) {
				જાઓ parse_error_exitcode1;
			}
			done_pipe(&ctx, PIPE_SEQ);
			ctx.is_assignment = MAYBE_ASSIGNMENT;
			debug_printf_parse("ctx.is_assignment='%s'\n", assignment_flag[ctx.is_assignment]);
			/* Do we sit outside of any if's, loops or case's? */
			જો (!HAS_KEYWORDS
			IF_HAS_KEYWORDS(|| (ctx.ctx_res_w == RES_NONE && ctx.old_flag == 0))
			) {
				o_free_and_set_NULL(&ctx.word);
#જો !BB_MMU
				debug_printf_parse("as_string2 '%s'\n", ctx.as_string.data);
				જો (pstring)
					*pstring = ctx.as_string.data;
				નહીંતર
					o_free(&ctx.as_string);
#endif
				જો (ch != ';' && IS_NULL_PIPE(ctx.list_head)) {
					/* Example: bare "{ }", "()" */
					G.last_exitcode = 2; /* bash compat */
					syntax_error_unexpected_ch(ch);
					જાઓ parse_error;
				}
				જો (heredoc_cnt_ptr)
					*heredoc_cnt_ptr = heredoc_cnt;
				debug_printf_heredoc("parse_stream return heredoc_cnt:%d\n", heredoc_cnt);
				debug_printf_parse("parse_stream return %p: "
						"end_trigger char found\n",
						ctx.list_head);
				debug_leave();
				પાછા ctx.list_head;
			}
		}

		જો (is_blank)
			ચાલુ;

		/* Catch <, > before deciding whether this word is
		 * an assignment. a=1 2>z b=2: b=2 is still assignment */
		પસંદગી (ch) {
#ifndef __U_BOOT__
		સ્થિતિ '>':
			redir_fd = redirect_opt_અંક(&ctx.word);
			જો (done_word(&ctx)) {
				જાઓ parse_error_exitcode1;
			}
			redir_style = REDIRECT_OVERWRITE;
			જો (next == '>') {
				redir_style = REDIRECT_APPEND;
				ch = i_getch(આગત);
				nommu_addchr(&ctx.as_string, ch);
			}
#જો 0
			નહીંતર જો (next == '(') {
				syntax_error(">(process) not supported");
				જાઓ parse_error_exitcode1;
			}
#endif
			જો (parse_redirect(&ctx, redir_fd, redir_style, આગત))
				જાઓ parse_error_exitcode1;
			ચાલુ; /* get next char */
		સ્થિતિ '<':
			redir_fd = redirect_opt_અંક(&ctx.word);
			જો (done_word(&ctx)) {
				જાઓ parse_error_exitcode1;
			}
			redir_style = REDIRECT_આગત;
			જો (next == '<') {
				redir_style = REDIRECT_HEREDOC;
				heredoc_cnt++;
				debug_printf_heredoc("++heredoc_cnt=%d\n", heredoc_cnt);
				ch = i_getch(આગત);
				nommu_addchr(&ctx.as_string, ch);
			} નહીંતર જો (next == '>') {
				redir_style = REDIRECT_IO;
				ch = i_getch(આગત);
				nommu_addchr(&ctx.as_string, ch);
			}
#જો 0
			નહીંતર જો (next == '(') {
				syntax_error("<(process) not supported");
				જાઓ parse_error_exitcode1;
			}
#endif
			જો (parse_redirect(&ctx, redir_fd, redir_style, આગત))
				જાઓ parse_error_exitcode1;
			ચાલુ; /* get next char */
#નહીંતર /* __U_BOOT__ */
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
			સ્થિતિ '>':
				fallthrough;
			સ્થિતિ '<':
				o_addQchr(&ctx.word, ch);
				ચાલુ;
#endif /* __U_BOOT__ */
		સ્થિતિ '#':
			જો (ctx.word.length == 0 && !ctx.word.has_quoted_part) {
				/* skip "#comment" */
				/* note: we do not add it to &ctx.as_string */
/* TODO: in bash:
 * comment inside $() goes to the next \n, even inside quoted string (!):
 * cmd "$(cmd2 #comment)" - syntax error
 * cmd "`cmd2 #comment`" - ok
 * We accept both (comment ends where command subst ends, in both cases).
 */
				જ્યારે (1) {
					ch = i_peek(આગત);
					જો (ch == '\n') {
						nommu_addchr(&ctx.as_string, '\n');
						તોડો;
					}
					ch = i_getch(આગત);
					જો (ch == EOF)
						તોડો;
				}
				ચાલુ; /* get next char */
			}
			તોડો;
		}
 skip_end_trigger:

		જો (ctx.is_assignment == MAYBE_ASSIGNMENT
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

		પસંદગી (ch) {
		case_SPECIAL_VAR_પ્રતીક:
		સ્થિતિ SPECIAL_VAR_પ્રતીક:
			/* Convert raw ^C to corresponding special variable reference */
			o_addchr(&ctx.word, SPECIAL_VAR_પ્રતીક);
			o_addchr(&ctx.word, SPECIAL_VAR_QUOTED_SVS);
			/* fall through */
		સ્થિતિ '#':
			/* non-comment #: "echo a#b" etc */
			o_addchr(&ctx.word, ch);
			ચાલુ; /* get next char */
		સ્થિતિ '$':
#ifndef __U_BOOT__
			જો (parse_dollar_squote(&ctx.as_string, &ctx.word, આગત))
				ચાલુ; /* get next char */
#endif /* !__U_BOOT__ */
			જો (!parse_dollar(&ctx.as_string, &ctx.word, આગત, /*quote_mask:*/ 0)) {
				debug_printf_parse("parse_stream parse error: "
					"parse_dollar returned 0 (error)\n");
				જાઓ parse_error_exitcode1;
			}
			ચાલુ; /* get next char */
		સ્થિતિ '"':
			ctx.word.has_quoted_part = 1;
#ifndef __U_BOOT__
			જો (next == '"' && !ctx.pending_redirect) {
#નહીંતર /* __U_BOOT__ */
			જો (next == '"') {
#endif /* __U_BOOT__ */
				i_getch(આગત); /* eat second " */
#ifndef __U_BOOT__
 ઉમેરો_empty_quoted_str_marker:
#endif /* !__U_BOOT__ */
				nommu_addchr(&ctx.as_string, next);
				o_addchr(&ctx.word, SPECIAL_VAR_પ્રતીક);
				o_addchr(&ctx.word, SPECIAL_VAR_પ્રતીક);
				ચાલુ; /* get next char */
			}
			જો (ctx.is_assignment == NOT_ASSIGNMENT)
				ctx.word.o_expflags |= EXP_FLAG_ESC_GLOB_CHARS;
			જો (!encode_string(&ctx.as_string, &ctx.word, આગત, '"'))
				જાઓ parse_error_exitcode1;
			ctx.word.o_expflags &= ~EXP_FLAG_ESC_GLOB_CHARS;
			ચાલુ; /* get next char */
#જો ENABLE_HUSH_TICK
		સ્થિતિ '`': {
			USE_FOR_NOMMU(unsigned pos;)

			o_addchr(&ctx.word, SPECIAL_VAR_પ્રતીક);
			o_addchr(&ctx.word, '`');
			USE_FOR_NOMMU(pos = ctx.word.length;)
			જો (!add_till_backquote(&ctx.word, આગત, /*in_dquote:*/ 0))
				જાઓ parse_error_exitcode1;
# જો !BB_MMU
			o_addstr(&ctx.as_string, ctx.word.data + pos);
			o_addchr(&ctx.as_string, '`');
# endif
			o_addchr(&ctx.word, SPECIAL_VAR_પ્રતીક);
			//debug_printf_subst("SUBST RES3 '%s'\n", ctx.word.data + pos);
			ચાલુ; /* get next char */
		}
#endif
		સ્થિતિ ';':
#જો ENABLE_HUSH_CASE
 case_semi:
#endif
			જો (done_word(&ctx)) {
				જાઓ parse_error_exitcode1;
			}
			done_pipe(&ctx, PIPE_SEQ);
#જો ENABLE_HUSH_CASE
			/* Eat multiple semicolons, detect
			 * whether it means something special */
			જ્યારે (1) {
				ch = i_peek_and_eat_bkslash_nl(આગત);
				જો (ch != ';')
					તોડો;
				ch = i_getch(આગત);
				nommu_addchr(&ctx.as_string, ch);
				જો (ctx.ctx_res_w == RES_CASE_BODY) {
					ctx.ctx_dsemicolon = 1;
					ctx.ctx_res_w = RES_MATCH;
					તોડો;
				}
			}
#endif
 new_cmd:
			/* We just finished a cmd. New one may start
			 * with an assignment */
			ctx.is_assignment = MAYBE_ASSIGNMENT;
			debug_printf_parse("ctx.is_assignment='%s'\n", assignment_flag[ctx.is_assignment]);
			ચાલુ; /* get next char */
		સ્થિતિ '&':
			જો (done_word(&ctx)) {
				જાઓ parse_error_exitcode1;
			}
			જો (next == '&') {
				ch = i_getch(આગત);
				nommu_addchr(&ctx.as_string, ch);
				done_pipe(&ctx, PIPE_AND);
			} નહીંતર {
				done_pipe(&ctx, PIPE_BG);
			}
			જાઓ new_cmd;
		સ્થિતિ '|':
			જો (done_word(&ctx)) {
				જાઓ parse_error_exitcode1;
			}
#જો ENABLE_HUSH_CASE
			જો (ctx.ctx_res_w == RES_MATCH)
				તોડો; /* we are in case's "word | word)" */
#endif
			જો (next == '|') { /* || */
				ch = i_getch(આગત);
				nommu_addchr(&ctx.as_string, ch);
				done_pipe(&ctx, PIPE_OR);
			} નહીંતર {
				/* we could pick up a file descriptor choice here
				 * with redirect_opt_num(), but bash doesn't do it.
				 * "echo foo 2| cat" yields "foo 2". */
				done_command(&ctx);
			}
			જાઓ new_cmd;
		સ્થિતિ '(':
#જો ENABLE_HUSH_CASE
			/* "case... in [(]word)..." - skip '(' */
			જો (ctx.ctx_res_w == RES_MATCH
			 && ctx.command->argv == ખાલી /* not (word|(... */
			 && ctx.word.length == 0 /* not word(... */
			 && ctx.word.has_quoted_part == 0 /* not ""(... */
			) {
				ચાલુ; /* get next char */
			}
#endif
			/* fall through */
		સ્થિતિ '{': {
			પૂર્ણાંક n = parse_group(&ctx, આગત, ch);
			જો (n < 0) {
				જાઓ parse_error_exitcode1;
			}
			debug_printf_heredoc("parse_group done, needs heredocs:%d\n", n);
			heredoc_cnt += n;
			જાઓ new_cmd;
		}
		સ્થિતિ ')':
#જો ENABLE_HUSH_CASE
			જો (ctx.ctx_res_w == RES_MATCH)
				જાઓ case_semi;
#endif
		સ્થિતિ '}':
			/* proper use of this character is caught by end_trigger:
			 * if we see {, we call parse_group(..., end_trigger='}')
			 * and it will match } earlier (not here). */
			G.last_exitcode = 2;
			syntax_error_unexpected_ch(ch);
			જાઓ parse_error;
		મૂળભૂત:
			જો (HUSH_DEBUG)
				bb_error_સંદેશ_and_die("BUG: unexpected %c", ch);
		}
	} /* while (1) */

 parse_error_exitcode1:
	G.last_exitcode = 1;
 parse_error:
	{
		સંરચના parse_context *pctx;
		IF_HAS_KEYWORDS(સંરચના parse_context *p2;)

		/* Clean up allocated tree.
		 * Sample for finding leaks on syntax error recovery path.
		 * Run it from interactive shell, watch pmap `pidof hush`.
		 * while if false; then false; fi; do break; fi
		 * Samples to catch leaks at execution:
		 * while if (true | { true;}); then echo ok; fi; do break; done
		 * while if (true | { true;}); then echo ok; fi; do (if echo ok; break; then :; fi) | cat; break; done
		 */
		pctx = &ctx;
		કરો {
			/* Update pipe/command counts,
			 * otherwise freeing may miss some */
			done_pipe(pctx, PIPE_SEQ);
			debug_printf_clean("freeing list %p from ctx %p\n",
					pctx->list_head, pctx);
			debug_print_વૃક્ષ(pctx->list_head, 0);
			free_pipe_list(pctx->list_head);
			debug_printf_clean("freed list %p\n", pctx->list_head);
#જો !BB_MMU
			o_free(&pctx->as_string);
#endif
			IF_HAS_KEYWORDS(p2 = pctx->ઢગલો;)
			જો (pctx != &ctx) {
				મુક્ત(pctx);
			}
			IF_HAS_KEYWORDS(pctx = p2;)
		} જ્યારે (HAS_KEYWORDS && pctx);

		o_free(&ctx.word);
#જો !BB_MMU
		જો (pstring)
			*pstring = ખાલી;
#endif
		debug_leave();
		પાછા ERR_PTR;
	}
}

/*** Execution routines ***/

/* Expansion can recurse, need forward decls: */
#જો !BASH_PATTERN_SUBST && !ENABLE_HUSH_CASE
#define expand_string_to_string(str, EXP_flags, do_unbackslash) \
	expand_string_to_string(str)
#endif
સ્થિર અક્ષર *expand_string_to_string(અચલ અક્ષર *str, પૂર્ણાંક EXP_flags, પૂર્ણાંક do_unbackslash);
#જો ENABLE_HUSH_TICK
સ્થિર પૂર્ણાંક process_command_subs(o_string *dest, અચલ અક્ષર *s);
#endif
સ્થિર પૂર્ણાંક expand_vars_to_list(o_string *output, પૂર્ણાંક n, અક્ષર *arg);

/* expand_strvec_to_strvec() takes a list of strings, expands
 * all variable references within and returns a pointer to
 * a list of expanded strings, possibly with larger number
 * of strings. (Think VAR="a b"; echo $VAR).
 * This new list is allocated as a single malloc block.
 * NULL-terminated list of char* pointers is at the beginning of it,
 * followed by strings themselves.
 * Caller can deallocate entire list by single free(list). */

/* A horde of its helpers come first: */

સ્થિર શૂન્ય o_addblock_duplicate_backslash(o_string *o, અચલ અક્ષર *str, પૂર્ણાંક len)
{
	જ્યારે (--len >= 0) {
		અક્ષર c = *str++;

#જો ENABLE_HUSH_BRACE_EXPANSION
		જો (c == '{' || c == '}') {
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
		જો (c == '\\') {
			/* \z -> \\\z; \<eol> -> \\<eol> */
			o_addchr(o, '\\');
			જો (len) {
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
સ્થિર પૂર્ણાંક expand_on_ifs(o_string *output, પૂર્ણાંક n, અચલ અક્ષર *str)
{
	પૂર્ણાંક last_is_ifs = 0;

	જ્યારે (1) {
		પૂર્ણાંક word_len;

		જો (!*str)  /* EOL - do not finalize word */
			તોડો;
		word_len = strcspn(str, G.ifs);
		જો (word_len) {
			/* We have WORD_LEN leading non-IFS chars */
			જો (!(output->o_expflags & EXP_FLAG_GLOB)) {
				o_addblock(output, str, word_len);
			} નહીંતર {
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
			જો (!*str)  /* EOL - do not finalize word */
				તોડો;
		}

		/* We know str here points to at least one IFS char */
		last_is_ifs = 1;
		str += strspn(str, G.ifs_whitespace); /* skip IFS whitespace chars */
		જો (!*str)  /* EOL - do not finalize word */
			તોડો;

		જો (G.ifs_whitespace != G.ifs /* usually false ($IFS is usually all whitespace), */
		 && strchr(G.ifs, *str)       /* the second check would fail */
		) {
			/* This is a non-whitespace $IFS char */
			/* Skip it and IFS whitespace chars, start new word */
			str++;
			str += strspn(str, G.ifs_whitespace);
			જાઓ new_word;
		}

		/* Start new word... but not always! */
		/* Case "v=' a'; echo ''$v": we do need to finalize empty word: */
		જો (output->has_quoted_part
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
	પાછા n;
}

#ifndef __U_BOOT__
/* Helper to expand $((...)) and heredoc body. These act as if
 * they are in double quotes, with the exception that they are not :).
 * Just the rules are similar: "expand only $var and `cmd`"
 *
 * Returns malloced string.
 * As an optimization, we return NULL if expansion is not needed.
 */
સ્થિર અક્ષર *encode_then_expand_string(અચલ અક્ષર *str)
{
	અક્ષર *exp_str;
	સંરચના in_str આગત;
	o_string dest = NULL_O_STRING;
	અચલ અક્ષર *cp;

	cp = str;
	માટે (;;) {
		જો (!*cp) પાછા ખાલી; /* string has no special chars */
		જો (*cp == '$') તોડો;
		જો (*cp == '\\') તોડો;
#જો ENABLE_HUSH_TICK
		જો (*cp == '`') તોડો;
#endif
		cp++;
	}

	/* We need to expand. Example:
	 * echo $(($a + `echo 1`)) $((1 + $((2)) ))
	 */
	setup_string_in_str(&આગત, str);
	encode_string(ખાલી, &dest, &આગત, EOF);
//TODO: error check (encode_string returns 0 on error)?
	//bb_error_msg("'%s' -> '%s'", str, dest.data);
	exp_str = expand_string_to_string(dest.data,
			EXP_FLAG_ESC_GLOB_CHARS,
			/*unbackslash:*/ 1
	);
	//bb_error_msg("'%s' -> '%s'", dest.data, exp_str);
	o_free(&dest);
	પાછા exp_str;
}

સ્થિર અચલ અક્ષર *first_special_char_in_vararg(અચલ અક્ષર *cp)
{
	માટે (;;) {
		જો (!*cp) પાછા ખાલી; /* string has no special chars */
		જો (*cp == '$') પાછા cp;
		જો (*cp == '\\') પાછા cp;
		જો (*cp == '\'') પાછા cp;
		જો (*cp == '"') પાછા cp;
#જો ENABLE_HUSH_TICK
		જો (*cp == '`') પાછા cp;
#endif
		/* dquoted "${x:+ARG}" should not glob, therefore
		 * '*' et al require some non-literal processing: */
		જો (*cp == '*') પાછા cp;
		જો (*cp == '?') પાછા cp;
		જો (*cp == '[') પાછા cp;
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
#જો !BASH_PATTERN_SUBST
#define encode_then_expand_vararg(str, handle_squotes, do_unbackslash) \
	encode_then_expand_vararg(str, handle_squotes)
#endif
સ્થિર અક્ષર *encode_then_expand_vararg(અચલ અક્ષર *str, પૂર્ણાંક handle_squotes, પૂર્ણાંક do_unbackslash)
{
#જો !BASH_PATTERN_SUBST && ENABLE_HUSH_CASE
	અચલ પૂર્ણાંક do_unbackslash = 0;
#endif
	અક્ષર *exp_str;
	સંરચના in_str આગત;
	o_string dest = NULL_O_STRING;

	જો (!first_special_char_in_vararg(str)) {
		/* string has no special chars */
		પાછા ખાલી;
	}

	setup_string_in_str(&આગત, str);
	dest.data = xzalloc(1); /* start as "", not as NULL */
	exp_str = ખાલી;

	માટે (;;) {
		પૂર્ણાંક ch;

		ch = i_getch(&આગત);
		debug_printf_parse("%s: ch=%c (%d) escape=%d\n",
				__func__, ch, ch, !!dest.o_expflags);

		જો (!dest.o_expflags) {
			જો (ch == EOF)
				તોડો;
			જો (handle_squotes && ch == '\'') {
				જો (!add_till_single_quote_dquoted(&dest, &આગત))
					જાઓ ret; /* error */
				ચાલુ;
			}
		}
		જો (ch == EOF) {
			syntax_error_unterm_ch('"');
			જાઓ ret; /* error */
		}
		જો (ch == '"') {
			dest.o_expflags ^= EXP_FLAG_ESC_GLOB_CHARS;
			ચાલુ;
		}
		જો (ch == '\\') {
			ch = i_getch(&આગત);
			જો (ch == EOF) {
//example? error message?	syntax_error_unterm_ch('"');
				debug_printf_parse("%s: error: \\<eof>\n", __func__);
				જાઓ ret;
			}
			o_addqchr(&dest, ch);
			ચાલુ;
		}
		જો (ch == '$') {
			જો (parse_dollar_squote(ખાલી, &dest, &આગત))
				ચાલુ;
			જો (!parse_dollar(ખાલી, &dest, &આગત, /*quote_mask:*/ 0x80)) {
				debug_printf_parse("%s: error: parse_dollar returned 0 (error)\n", __func__);
				જાઓ ret;
			}
			ચાલુ;
		}
#જો ENABLE_HUSH_TICK
		જો (ch == '`') {
			//unsigned pos = dest->length;
			o_addchr(&dest, SPECIAL_VAR_પ્રતીક);
			o_addchr(&dest, 0x80 | '`');
			જો (!add_till_backquote(&dest, &આગત,
					/*in_dquote:*/ dest.o_expflags /* nonzero if EXP_FLAG_ESC_GLOB_CHARS set */
				)
			) {
				જાઓ ret; /* error */
			}
			o_addchr(&dest, SPECIAL_VAR_પ્રતીક);
			//debug_printf_subst("SUBST RES3 '%s'\n", dest->data + pos);
			ચાલુ;
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
	પાછા exp_str;
}

/* Expanding ARG in ${var+ARG}, ${var-ARG}
 */
સ્થિર NOINLINE પૂર્ણાંક encode_then_append_var_plusminus(o_string *output, પૂર્ણાંક n,
		અક્ષર *str, પૂર્ણાંક dquoted)
{
	સંરચના in_str આગત;
	o_string dest = NULL_O_STRING;

	જો (!first_special_char_in_vararg(str)
	 && '\0' == str[strcspn(str, G.ifs)]
	) {
		/* string has no special chars
		 * && string has no $IFS chars
		 */
		જો (dquoted) {
			/* Prints 1 (quoted expansion is a "" word, not nothing):
			 * set -- "${notexist-}"; echo $#
			 */
			output->has_quoted_part = 1;
		}
		પાછા expand_vars_to_list(output, n, str);
	}

	setup_string_in_str(&આગત, str);

	માટે (;;) {
		પૂર્ણાંક ch;

		ch = i_getch(&આગત);
		debug_printf_parse("%s: ch=%c (%d) escape=%x\n",
				__func__, ch, ch, dest.o_expflags);

		જો (!dest.o_expflags) {
			જો (ch == EOF)
				તોડો;
			જો (!dquoted && !(output->o_expflags & EXP_FLAG_SINGLEWORD) && strchr(G.ifs, ch)) {
				/* PREFIX${x:d${e}f ...} and we met space: expand "d${e}f" and start new word.
				 * do not assume we are at the start of the word (PREFIX above).
				 */
				જો (dest.data) {
					n = expand_vars_to_list(output, n, dest.data);
					o_free_and_set_NULL(&dest);
					o_addchr(output, '\0');
					n = o_save_ptr(output, n); /* create next word */
				} નહીંતર
				જો (output->length != o_get_last_ptr(output, n)
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
				ચાલુ;
			}
			જો (!dquoted && ch == '\'') {
				જો (!add_till_single_quote_dquoted(&dest, &આગત))
					જાઓ ret; /* error */
				o_addchr(&dest, SPECIAL_VAR_પ્રતીક);
				o_addchr(&dest, SPECIAL_VAR_પ્રતીક);
				ચાલુ;
			}
		}
		જો (ch == EOF) {
			syntax_error_unterm_ch('"');
			જાઓ ret; /* error */
		}
		જો (ch == '"') {
			dest.o_expflags ^= EXP_FLAG_ESC_GLOB_CHARS;
			જો (dest.o_expflags) {
				o_addchr(&dest, SPECIAL_VAR_પ્રતીક);
				o_addchr(&dest, SPECIAL_VAR_પ્રતીક);
			}
			ચાલુ;
		}
		જો (ch == '\\') {
			ch = i_getch(&આગત);
			જો (ch == EOF) {
//example? error message?	syntax_error_unterm_ch('"');
				debug_printf_parse("%s: error: \\<eof>\n", __func__);
				જાઓ ret;
			}
			o_addqchr(&dest, ch);
			ચાલુ;
		}
		જો (ch == '$') {
			જો (!parse_dollar(ખાલી, &dest, &આગત, /*quote_mask:*/ (dest.o_expflags || dquoted) ? 0x80 : 0)) {
				debug_printf_parse("%s: error: parse_dollar returned 0 (error)\n", __func__);
				જાઓ ret;
			}
			ચાલુ;
		}
#જો ENABLE_HUSH_TICK
		જો (ch == '`') {
			//unsigned pos = dest->length;
			o_addchr(&dest, SPECIAL_VAR_પ્રતીક);
			o_addchr(&dest, (dest.o_expflags || dquoted) ? 0x80 | '`' : '`');
			જો (!add_till_backquote(&dest, &આગત,
					/*in_dquote:*/ dest.o_expflags /* nonzero if EXP_FLAG_ESC_GLOB_CHARS set */
				)
			) {
				જાઓ ret; /* error */
			}
			o_addchr(&dest, SPECIAL_VAR_પ્રતીક);
			//debug_printf_subst("SUBST RES3 '%s'\n", dest->data + pos);
			ચાલુ;
		}
#endif
		જો (dquoted) {
			/* Always glob-protect if in dquotes:
			 * x=x; echo "${x:+/bin/c*}" - prints: /bin/c*
			 * x=x; echo "${x:+"/bin/c*"}" - prints: /bin/c*
			 */
			o_addqchr(&dest, ch);
		} નહીંતર {
			/* Glob-protect only if char is quoted:
			 * x=x; echo ${x:+/bin/c*} - prints many filenames
			 * x=x; echo ${x:+"/bin/c*"} - prints: /bin/c*
			 */
			o_addQchr(&dest, ch);
		}
	} /* for (;;) */

	જો (dest.data) {
		n = expand_vars_to_list(output, n, dest.data);
	}
 ret:
	o_free(&dest);
	પાછા n;
}
#endif /* !__U_BOOT__ */

#ifndef __U_BOOT__
#જો ENABLE_FEATURE_SH_MATH
સ્થિર arith_t expand_and_evaluate_arith(અચલ અક્ષર *arg, અચલ અક્ષર **errmsg_p)
{
	arith_state_t math_state;
	arith_t res;
	અક્ષર *exp_str;

	math_state.lookupvar = get_local_var_કિંમત;
	math_state.setvar = set_local_var_from_halves;
	//math_state.endofname = endofname;
	exp_str = encode_then_expand_string(arg);
	res = arith(&math_state, exp_str ? exp_str : arg);
	મુક્ત(exp_str);
	જો (errmsg_p)
		*errmsg_p = math_state.errmsg;
	જો (math_state.errmsg)
		સંદેશ_and_die_if_script(math_state.errmsg);
	પાછા res;
}
#endif
#endif /* !__U_BOOT__ */

#ifndef __U_BOOT__
#જો BASH_PATTERN_SUBST
/* ${var/[/]pattern[/repl]} helpers */
સ્થિર અક્ષર *strstr_pattern(અક્ષર *val, અચલ અક્ષર *pattern, પૂર્ણાંક *size)
{
	પૂર્ણાંક first_escaped = (pattern[0] == '\\' && pattern[1]);
	/* "first_escaped" trick allows to treat e.g. "\*no_glob_chars"
	 * as literal too (as it is semi-common, and easy to accomodate
	 * by just using str + 1).
	 */
	પૂર્ણાંક sz = strcspn(pattern + first_escaped * 2, "*?[\\");
	જો ((pattern + first_escaped * 2)[sz] == '\0') {
		/* Optimization for trivial patterns.
		 * Testcase for very slow replace (performs about 22k replaces):
		 * x=::::::::::::::::::::::
		 * x=$x$x;x=$x$x;x=$x$x;x=$x$x;x=$x$x;x=$x$x;x=$x$x;x=$x$x;x=$x$x;x=$x$x;echo ${#x}
		 * echo "${x//:/|}"
		 */
		*size = sz + first_escaped;
		પાછા strstr(val, pattern + first_escaped);
	}

	જ્યારે (1) {
		અક્ષર *end = scan_and_match(val, pattern, SCAN_MOVE_FROM_જમણું + SCAN_MATCH_ડાબું_HALF);
		debug_printf_varexp("val:'%s' pattern:'%s' end:'%s'\n", val, pattern, end);
		જો (end) {
			*size = end - val;
			પાછા val;
		}
		જો (*val == '\0')
			પાછા ખાલી;
		/* Optimization: if "*pat" did not match the start of "string",
		 * we know that "tring", "ring" etc will not match too:
		 */
		જો (pattern[0] == '*')
			પાછા ખાલી;
		val++;
	}
}
સ્થિર અક્ષર *replace_pattern(અક્ષર *val, અચલ અક્ષર *pattern, અચલ અક્ષર *repl, અક્ષર exp_op)
{
	અક્ષર *result = ખાલી;
	unsigned res_len = 0;
	unsigned repl_len = strlen(repl);

	/* Null pattern never matches, including if "var" is empty */
	જો (!pattern[0])
		પાછા result; /* NULL, no replaces happened */

	જ્યારે (1) {
		પૂર્ણાંક size;
		અક્ષર *s = strstr_pattern(val, pattern, &size);
		જો (!s)
			તોડો;

		result = xrealloc(result, res_len + (s - val) + repl_len + 1);
		strcpy(mempcpy(result + res_len, val, s - val), repl);
		res_len += (s - val) + repl_len;
		debug_printf_varexp("val:'%s' s:'%s' result:'%s'\n", val, s, result);

		val = s + size;
		જો (exp_op == '/')
			તોડો;
	}
	જો (*val && result) {
		result = xrealloc(result, res_len + strlen(val) + 1);
		strcpy(result + res_len, val);
		debug_printf_varexp("val:'%s' result:'%s'\n", val, result);
	}
	debug_printf_varexp("result:'%s'\n", result);
	પાછા result;
}
#endif /* BASH_PATTERN_SUBST */
#endif /* !__U_BOOT__ */

સ્થિર પૂર્ણાંક append_str_maybe_ifs_split(o_string *output, પૂર્ણાંક n,
		પૂર્ણાંક first_ch, અચલ અક્ષર *val)
{
	જો (!(first_ch & 0x80)) { /* unquoted $VAR */
		debug_printf_expand("unquoted '%s', output->o_escape:%d\n", val,
				!!(output->o_expflags & EXP_FLAG_ESC_GLOB_CHARS));
		જો (val && val[0])
			n = expand_on_ifs(output, n, val);
	} નહીંતર { /* quoted "$VAR" */
		output->has_quoted_part = 1;
		debug_printf_expand("quoted '%s', output->o_escape:%d\n", val,
				!!(output->o_expflags & EXP_FLAG_ESC_GLOB_CHARS));
		જો (val && val[0])
			o_addQstr(output, val);
	}
	પાછા n;
}

/* Handle <SPECIAL_VAR_SYMBOL>varname...<SPECIAL_VAR_SYMBOL> construct.
 */
સ્થિર NOINLINE પૂર્ણાંક expand_one_var(o_string *output, પૂર્ણાંક n,
		પૂર્ણાંક first_ch, અક્ષર *arg, અક્ષર **pp)
{
	અચલ અક્ષર *val;
	અક્ષર *to_be_freed;
	અક્ષર *p;
	અક્ષર *var;
	અક્ષર exp_op;
	અક્ષર exp_save = exp_save; /* for compiler */
	અક્ષર *exp_saveptr; /* points to expansion operator */
	અક્ષર *exp_word = exp_word; /* for compiler */
	અક્ષર arg0;

	val = ખાલી;
	to_be_freed = ખાલી;
	p = *pp;
	*p = '\0'; /* replace trailing SPECIAL_VAR_SYMBOL */
	var = arg;
	exp_saveptr = arg[1] ? strchr(VAR_ENCODED_SUBST_OPS, arg[1]) : ખાલી;
	arg0 = arg[0];
	arg[0] = (arg0 & 0x7f);
	exp_op = 0;

	જો (arg[0] == '#' && arg[1] /* ${#...} but not ${#} */
	 && (!exp_saveptr               /* and ( not(${#<op_char>...}) */
	    || (arg[2] == '\0' && strchr(SPECIAL_VARS_STR, arg[1])) /* or ${#C} "len of $C" ) */
	    )		/* NB: skipping ^^^specvar check mishandles ${#::2} */
	) {
		/* It must be length operator: ${#var} */
		var++;
		exp_op = 'L';
	} નહીંતર {
		/* Maybe handle parameter expansion */
		જો (exp_saveptr /* if 2nd char is one of expansion operators */
		 && strchr(NUMERIC_SPECVARS_STR, arg[0]) /* 1st char is special variable */
		) {
			/* ${?:0}, ${#[:]%0} etc */
			exp_saveptr = var + 1;
		} નહીંતર {
			/* ${?}, ${var}, ${var:0}, ${var[:]%0} etc */
			exp_saveptr = var+1 + strcspn(var+1, VAR_ENCODED_SUBST_OPS);
		}
		exp_op = exp_save = *exp_saveptr;
#ifndef __U_BOOT__
		જો (exp_op) {
			exp_word = exp_saveptr + 1;
			જો (exp_op == ':') {
				exp_op = *exp_word++;
//TODO: try ${var:} and ${var:bogus} in non-bash config
				જો (BASH_SUBSTR
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
	જો (isdigit(var[0])) {
		/* parse_dollar should have vetted var for us */
#ifndef __U_BOOT__
		પૂર્ણાંક nn = xatoi_positive(var);
#નહીંતર /* __U_BOOT__ */
		પૂર્ણાંક nn = simple_strtoul(var, ખાલી, 10);
#endif /* __U_BOOT__ */
		જો (nn < G.global_argc)
			val = G.global_argv[nn];
		/* else val remains NULL: $N with too big N */
	} નહીંતર {
		પસંદગી (var[0]) {
#ifndef __U_BOOT__
		સ્થિતિ '$': /* pid */
			val = utoa(G.મૂળ_pid);
			તોડો;
		સ્થિતિ '!': /* bg pid */
			val = G.last_bg_pid ? utoa(G.last_bg_pid) : "";
			તોડો;
#endif /* !__U_BOOT__ */
		સ્થિતિ '?': /* exitcode */
			val = utoa(G.last_exitcode);
			તોડો;
		સ્થિતિ '#': /* argc */
			val = utoa(G.global_argc ? G.global_argc-1 : 0);
			તોડો;
#ifndef __U_BOOT__
		સ્થિતિ '-': { /* active options */
			/* Check set_mode() to see what option chars we support */
			અક્ષર *cp;
			val = cp = G.optstring_buf;
			જો (G.o_opt[OPT_O_ERREXIT])
				*cp++ = 'e';
			જો (G_interactive_fd)
				*cp++ = 'i';
			જો (G_x_mode)
				*cp++ = 'x';
			/* If G.o_opt[OPT_O_NOEXEC] is true,
			 * commands read but are not executed,
			 * so $- can not execute too, 'n' is never seen in $-.
			 */
			જો (G.opt_c)
				*cp++ = 'c';
			જો (G.opt_s)
				*cp++ = 's';
			*cp = '\0';
			તોડો;
		}
#endif /* !__U_BOOT__ */
		મૂળભૂત:
#ifndef __U_BOOT__
			val = get_local_var_કિંમત(var);
#નહીંતર /* __U_BOOT__ */
			/*
			 * Environment variable set with setenv* have to be
			 * expanded.
			 * So, we first search if the variable exists in
			 * environment, if this is not the case, we default to
			 * local value.
			 */
			val = env_get(var);
			જો (!val)
				val = get_local_var_કિંમત(var);
#endif /* __U_BOOT__ */
		}
	}

#ifndef __U_BOOT__
	/* Handle any expansions */
	જો (exp_op == 'L') {
		reinit_unicode_for_hush();
		debug_printf_expand("expand: length(%s)=", val);
		val = utoa(val ? unicode_strlen(val) : 0);
		debug_printf_expand("%s\n", val);
	} નહીંતર જો (exp_op) {
		જો (exp_op == '%' || exp_op == '#') {
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
			જો (val && val[0]) {
				અક્ષર *t;
				અક્ષર *exp_exp_word;
				અક્ષર *loc;
				unsigned scan_flags = pick_scan(exp_op, *exp_word);
				જો (exp_op == *exp_word)  /* ## or %% */
					exp_word++;
				debug_printf_expand("expand: exp_word:'%s'\n", exp_word);
				exp_exp_word = encode_then_expand_vararg(exp_word, /*handle_squotes:*/ 1, /*unbackslash:*/ 0);
				જો (exp_exp_word)
					exp_word = exp_exp_word;
				debug_printf_expand("expand: exp_word:'%s'\n", exp_word);
				/*
				 * HACK ALERT. We depend here on the fact that
				 * G.global_argv and results of utoa and get_local_var_value
				 * are actually in writable memory:
				 * scan_and_match momentarily stores NULs there.
				 */
				t = (અક્ષર*)val;
				loc = scan_and_match(t, exp_word, scan_flags);
				debug_printf_expand("op:%c str:'%s' pat:'%s' res:'%s'\n", exp_op, t, exp_word, loc);
				મુક્ત(exp_exp_word);
				જો (loc) { /* match was found */
					જો (scan_flags & SCAN_MATCH_ડાબું_HALF) /* #[#] */
						val = loc; /* take right part */
					નહીંતર /* %[%] */
						val = to_be_freed = xstrndup(val, loc - val); /*ડાબું*/
				}
			}
		}
#જો BASH_PATTERN_SUBST
		નહીંતર જો (exp_op == '/' || exp_op == '\\') {
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
			જો (val /*&& val[0]*/) {
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
				અક્ષર *pattern, *repl, *t;
				pattern = encode_then_expand_vararg(exp_word, /*handle_squotes:*/ 1, /*unbackslash:*/ 0);
				જો (!pattern)
					pattern = xstrdup(exp_word);
				debug_printf_varexp("pattern:'%s'->'%s'\n", exp_word, pattern);
				*p++ = SPECIAL_VAR_પ્રતીક;
				exp_word = p;
				p = strchr(p, SPECIAL_VAR_પ્રતીક);
				*p = '\0';
				repl = encode_then_expand_vararg(exp_word, /*handle_squotes:*/ 1, /*unbackslash:*/ 1);
				debug_printf_varexp("repl:'%s'->'%s'\n", exp_word, repl);
				/* HACK ALERT. We depend here on the fact that
				 * G.global_argv and results of utoa and get_local_var_value
				 * are actually in writable memory:
				 * replace_pattern momentarily stores NULs there. */
				t = (અક્ષર*)val;
				to_be_freed = replace_pattern(t,
						pattern,
						(repl ? repl : exp_word),
						exp_op);
				જો (to_be_freed) /* at least one replace happened */
					val = to_be_freed;
				મુક્ત(pattern);
				મુક્ત(repl);
			} નહીંતર {
				/* Unset variable always gives nothing */
				//  a=; echo ${a/*/w}      # "w"
				//  unset a; echo ${a/*/w} # ""
				/* Just skip "replace" part */
				*p++ = SPECIAL_VAR_પ્રતીક;
				p = strchr(p, SPECIAL_VAR_પ્રતીક);
				*p = '\0';
			}
		}
#endif /* BASH_PATTERN_SUBST */
		નહીંતર જો (exp_op == ':') {
#જો BASH_SUBSTR && ENABLE_FEATURE_SH_MATH
			/* It's ${var:N[:M]} bashism.
			 * Note that in encoded form it has TWO parts:
			 * var:N<SPECIAL_VAR_SYMBOL>M<SPECIAL_VAR_SYMBOL>
			 */
			arith_t beg, len;
			unsigned vallen;
			અચલ અક્ષર *errmsg;

			beg = expand_and_evaluate_arith(exp_word, &errmsg);
			જો (errmsg)
				જાઓ empty_result;
			debug_printf_varexp("beg:'%s'=%lld\n", exp_word, (long long)beg);
			*p++ = SPECIAL_VAR_પ્રતીક;
			exp_word = p;
			p = strchr(p, SPECIAL_VAR_પ્રતીક);
			*p = '\0';
			vallen = val ? strlen(val) : 0;
			જો (beg < 0) {
				/* negative beg counts from the end */
				beg = (arith_t)vallen + beg;
			}
			/* If expansion will be empty, do not even evaluate len */
			જો (!val || beg < 0 || beg > vallen) {
				/* Why > vallen, not >=? bash:
				 * unset b; a=ab; : ${a:2:${b=3}}; echo $b  # "", b=3 (!!!)
				 * unset b; a=a; : ${a:2:${b=3}}; echo $b   # "", b not set
				 */
				જાઓ empty_result;
			}
			len = expand_and_evaluate_arith(exp_word, &errmsg);
			જો (errmsg)
				જાઓ empty_result;
			debug_printf_varexp("len:'%s'=%lld\n", exp_word, (long long)len);
			debug_printf_varexp("from val:'%s'\n", val);
			જો (len < 0) {
				/* in bash, len=-n means strlen()-n */
				len = (arith_t)vallen - beg + len;
				જો (len < 0) /* bash compat */
					સંદેશ_and_die_if_script("%s: substring expression < 0", var);
			}
			જો (len <= 0 || !val /*|| beg >= vallen*/) {
 empty_result:
				val = ખાલી;
			} નહીંતર {
				/* Paranoia. What if user entered 9999999999999
				 * which fits in arith_t but not int? */
				જો (len > INT_MAX)
					len = INT_MAX;
				val = to_be_freed = xstrndup(val + beg, len);
			}
			debug_printf_varexp("val:'%s'\n", val);
#નહીંતર /* not (HUSH_SUBSTR_EXPANSION && FEATURE_SH_MATH) */
			સંદેશ_and_die_if_script("malformed ${%s:...}", var);
			val = ખાલી;
#endif
		} નહીંતર { /* one of "-=+?" */
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
			પૂર્ણાંક use_word = (!val || ((exp_save == ':') && !val[0]));
			જો (exp_op == '+')
				use_word = !use_word;
			debug_printf_expand("expand: op:%c (null:%s) test:%i\n", exp_op,
					(exp_save == ':') ? "true" : "false", use_word);
			જો (use_word) {
				જો (exp_op == '+' || exp_op == '-') {
					/* ${var+word} - use alternative value */
					/* ${var-word} - use default value */
					n = encode_then_append_var_plusminus(output, n, exp_word,
							/*dquoted:*/ (arg0 & 0x80)
					);
					val = ખાલી;
				} નહીંતર {
					/* ${var?word} - indicate error if unset */
					/* ${var=word} - assign and use default value */
					to_be_freed = encode_then_expand_vararg(exp_word,
							/*handle_squotes:*/ !(arg0 & 0x80),
							/*unbackslash:*/ 0
					);
					જો (to_be_freed)
						exp_word = to_be_freed;
					જો (exp_op == '?') {
						/* mimic bash message */
						સંદેશ_and_die_if_script("%s: %s",
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
					} નહીંતર {
						val = exp_word;
					}
					જો (exp_op == '=') {
						/* ${var=[word]} or ${var:=[word]} */
						જો (isdigit(var[0]) || var[0] == '#') {
							/* mimic bash message */
							સંદેશ_and_die_if_script("$%s: cannot assign in this way", var);
							val = ખાલી;
						} નહીંતર {
							અક્ષર *new_var = xasprintf("%s=%s", var, val);
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

	મુક્ત(to_be_freed);
	પાછા n;
}

/* Expand all variable references in given string, adding words to list[]
 * at n, n+1,... positions. Return updated n (so that list[n] is next one
 * to be filled). This routine is extremely tricky: has to deal with
 * variables/parameters with whitespace, $* and $@, and constructs like
 * 'echo -$*-'. If you play here, you must run testsuite afterwards! */
સ્થિર NOINLINE પૂર્ણાંક expand_vars_to_list(o_string *output, પૂર્ણાંક n, અક્ષર *arg)
{
	/* output->o_expflags & EXP_FLAG_SINGLEWORD (0x80) if we are in
	 * expansion of right-hand side of assignment == 1-element expand.
	 */
	અક્ષર cant_be_null = 0; /* only bit 0x80 matters */
	અક્ષર *p;

	debug_printf_expand("expand_vars_to_list: arg:'%s' singleword:%x\n", arg,
			!!(output->o_expflags & EXP_FLAG_SINGLEWORD));
	debug_print_list("expand_vars_to_list[0]", output, n);

	જ્યારે ((p = strchr(arg, SPECIAL_VAR_પ્રતીક)) != ખાલી) {
		અક્ષર first_ch;
#જો ENABLE_FEATURE_SH_MATH
		અક્ષર arith_buf[કદ(arith_t)*3 + 2];
#endif

		જો (output->ended_in_ifs) {
			o_addchr(output, '\0');
			n = o_save_ptr(output, n);
			output->ended_in_ifs = 0;
		}

		o_addblock(output, arg, p - arg);
		debug_print_list("expand_vars_to_list[1]", output, n);
		arg = ++p;
		p = strchr(p, SPECIAL_VAR_પ્રતીક);

		/* Fetch special var name (if it is indeed one of them)
		 * and quote bit, force the bit on if singleword expansion -
		 * important for not getting v=$@ expand to many words. */
		first_ch = arg[0] | (output->o_expflags & EXP_FLAG_SINGLEWORD);

		/* Is this variable quoted and thus expansion can't be null?
		 * "$@" is special. Even if quoted, it can still
		 * expand to nothing (not even an empty string),
		 * thus it is excluded. */
		જો ((first_ch & 0x7f) != '@')
			cant_be_null |= first_ch;

		પસંદગી (first_ch & 0x7f) {
		/* Highest bit in first_ch indicates that var is double-quoted */
		સ્થિતિ '*':
		સ્થિતિ '@': {
			પૂર્ણાંક i;
#ifndef __U_BOOT__
			જો (!G.global_argv[1])
#નહીંતર /* __U_BOOT__ */
			જો (!G.global_argv || !G.global_argv[1])
#endif /* __U_BOOT__ */
				તોડો;
			i = 1;
			cant_be_null |= first_ch; /* do it for "$@" _now_, when we know it's not empty */
			જો (!(first_ch & 0x80)) { /* unquoted $* or $@ */
				જ્યારે (G.global_argv[i]) {
					n = expand_on_ifs(output, n, G.global_argv[i]);
					debug_printf_expand("expand_vars_to_list: argv %d (last %d)\n", i, G.global_argc - 1);
					જો (G.global_argv[i++][0] && G.global_argv[i]) {
						/* this argv[] is not empty and not last:
						 * put terminating NUL, start new word */
						o_addchr(output, '\0');
						debug_print_list("expand_vars_to_list[2]", output, n);
						n = o_save_ptr(output, n);
						debug_print_list("expand_vars_to_list[3]", output, n);
					}
				}
			} નહીંતર
			/* If EXP_FLAG_SINGLEWORD, we handle assignment 'a=....$@.....'
			 * and in this case should treat it like '$*' - see 'else...' below */
			જો (first_ch == (અક્ષર)('@'|0x80)  /* quoted $@ */
			 && !(output->o_expflags & EXP_FLAG_SINGLEWORD) /* not v="$@" case */
			) {
				જ્યારે (1) {
					o_addQstr(output, G.global_argv[i]);
					જો (++i >= G.global_argc)
						તોડો;
					o_addchr(output, '\0');
					debug_print_list("expand_vars_to_list[4]", output, n);
					n = o_save_ptr(output, n);
				}
			} નહીંતર { /* quoted $* (or v="$@" case): add as one word */
				જ્યારે (1) {
					o_addQstr(output, G.global_argv[i]);
					જો (!G.global_argv[++i])
						તોડો;
					જો (G.ifs[0])
						o_addchr(output, G.ifs[0]);
				}
				output->has_quoted_part = 1;
			}
			તોડો;
		}
		સ્થિતિ SPECIAL_VAR_પ્રતીક: {
			/* <SPECIAL_VAR_SYMBOL><SPECIAL_VAR_SYMBOL> */
			/* "Empty variable", used to make "" etc to not disappear */
			output->has_quoted_part = 1;
			cant_be_null = 0x80;
			arg++;
			તોડો;
		}
		સ્થિતિ SPECIAL_VAR_QUOTED_SVS:
			/* <SPECIAL_VAR_SYMBOL><SPECIAL_VAR_QUOTED_SVS><SPECIAL_VAR_SYMBOL> */
			/* "^C variable", represents literal ^C char (possible in scripts) */
			o_addchr(output, SPECIAL_VAR_પ્રતીક);
			arg++;
			તોડો;
#જો ENABLE_HUSH_TICK
		સ્થિતિ '`': {
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
			તોડો;
		}
#endif
#જો ENABLE_FEATURE_SH_MATH
		સ્થિતિ '+': {
			/* <SPECIAL_VAR_SYMBOL>+arith<SPECIAL_VAR_SYMBOL> */
			arith_t res;

			arg++; /* skip '+' */
			*p = '\0'; /* replace trailing <SPECIAL_VAR_SYMBOL> */
			debug_printf_subst("ARITH '%s' first_ch %x\n", arg, first_ch);
			res = expand_and_evaluate_arith(arg, ખાલી);
			debug_printf_subst("ARITH RES '"ARITH_FMT"'\n", res);
			sprintf(arith_buf, ARITH_FMT, res);
			જો (res < 0
			 && first_ch == (અક્ષર)('+'|0x80)
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
			તોડો;
		}
#endif
		મૂળભૂત:
			/* <SPECIAL_VAR_SYMBOL>varname[ops]<SPECIAL_VAR_SYMBOL> */
			n = expand_one_var(output, n, first_ch, arg, &p);
			તોડો;
		} /* switch (char after <SPECIAL_VAR_SYMBOL>) */

		/* Restore NULL'ed SPECIAL_VAR_SYMBOL.
		 * Do the check to avoid writing to a const string. */
		જો (*p != SPECIAL_VAR_પ્રતીક)
			*p = SPECIAL_VAR_પ્રતીક;
		arg = ++p;
	} /* end of "while (SPECIAL_VAR_SYMBOL is found) ..." */

	જો (*arg) {
		/* handle trailing string */
		જો (output->ended_in_ifs) {
			o_addchr(output, '\0');
			n = o_save_ptr(output, n);
		}
		debug_print_list("expand_vars_to_list[a]", output, n);
		/* this part is literal, and it was already pre-quoted
		 * if needed (much earlier), do not use o_addQstr here!
		 */
		o_addstr(output, arg);
		debug_print_list("expand_vars_to_list[b]", output, n);
	} નહીંતર
	જો (output->length == o_get_last_ptr(output, n) /* expansion is empty */
	 && !(cant_be_null & 0x80)   /* and all vars were not quoted */
	 && !output->has_quoted_part
	) {
		n--;
		/* allow to reuse list[n] later without re-growth */
		output->has_empty_slot = 1;
	}

	પાછા n;
}

સ્થિર અક્ષર **expand_variables(અક્ષર **argv, unsigned expflags)
{
	પૂર્ણાંક n;
	અક્ષર **list;
	o_string output = NULL_O_STRING;

	output.o_expflags = expflags;

	n = 0;
	માટે (;;) {
		/* go to next list[n] */
		output.ended_in_ifs = 0;
		n = o_save_ptr(&output, n);

		જો (!*argv)
			તોડો;

		/* expand argv[i] */
		n = expand_vars_to_list(&output, n, *argv++);
		/* if (!output->has_empty_slot) -- need this?? */
			o_addchr(&output, '\0');
	}
	debug_print_list("expand_variables", &output, n);

	/* output.data (malloced in one block) gets returned in "list" */
	list = o_finalize_list(&output, n);
	debug_print_strings("expand_variables[1]", list);
	પાછા list;
}

સ્થિર અક્ષર **expand_strvec_to_strvec(અક્ષર **argv)
{
	પાછા expand_variables(argv, EXP_FLAG_GLOB | EXP_FLAG_ESC_GLOB_CHARS);
}

#જો defined(CMD_SINGLEWORD_NOGLOB) || defined(CMD_TEST2_SINGLEWORD_NOGLOB)
સ્થિર અક્ષર **expand_strvec_to_strvec_singleword_noglob(અક્ષર **argv)
{
	પાછા expand_variables(argv, EXP_FLAG_SINGLEWORD);
}
#endif

/* Used for expansion of right hand of assignments,
 * $((...)), heredocs, variable expansion parts.
 *
 * NB: should NOT do globbing!
 * "export v=/bin/c*; env | grep ^v=" outputs "v=/bin/c*"
 */
સ્થિર અક્ષર *expand_string_to_string(અચલ અક્ષર *str, પૂર્ણાંક EXP_flags, પૂર્ણાંક do_unbackslash)
{
#જો !BASH_PATTERN_SUBST && !ENABLE_HUSH_CASE
	અચલ પૂર્ણાંક do_unbackslash = 1;
	અચલ પૂર્ણાંક EXP_flags = EXP_FLAG_ESC_GLOB_CHARS;
#endif
	અક્ષર *argv[2], **list;

	debug_printf_expand("string_to_string<='%s'\n", str);
	/* This is generally an optimization, but it also
	 * handles "", which otherwise trips over !list[0] check below.
	 * (is this ever happens that we actually get str="" here?)
	 */
	જો (!strchr(str, SPECIAL_VAR_પ્રતીક) && !strchr(str, '\\')) {
		//TODO: Can use on strings with \ too, just unbackslash() them?
		debug_printf_expand("string_to_string(fast)=>'%s'\n", str);
		પાછા xstrdup(str);
	}

	argv[0] = (અક્ષર*)str;
	argv[1] = ખાલી;
	list = expand_variables(argv, EXP_flags | EXP_FLAG_SINGLEWORD);
	જો (!list[0]) {
		/* Example where it happens:
		 * x=; echo ${x:-"$@"}
		 */
		((અક્ષર*)list)[0] = '\0';
	} નહીંતર {
		જો (HUSH_DEBUG)
			જો (list[1])
				bb_simple_error_સંદેશ_and_die("BUG in varexp2");
		/* actually, just move string 2*sizeof(char*) bytes back */
		overlapping_strcpy((અક્ષર*)list, list[0]);
		જો (do_unbackslash)
			unbackslash((અક્ષર*)list);
	}
	debug_printf_expand("string_to_string=>'%s'\n", (અક્ષર*)list);
	પાછા (અક્ષર*)list;
}

#જો 0
સ્થિર અક્ષર* expand_strvec_to_string(અક્ષર **argv)
{
	અક્ષર **list;

	list = expand_variables(argv, EXP_FLAG_SINGLEWORD);
	/* Convert all NULs to spaces */
	જો (list[0]) {
		પૂર્ણાંક n = 1;
		જ્યારે (list[n]) {
			જો (HUSH_DEBUG)
				જો (list[n-1] + strlen(list[n-1]) + 1 != list[n])
					bb_error_સંદેશ_and_die("BUG in varexp3");
			/* bash uses ' ' regardless of $IFS contents */
			list[n][-1] = ' ';
			n++;
		}
	}
	overlapping_strcpy((અક્ષર*)list, list[0] ? list[0] : "");
	debug_printf_expand("strvec_to_string='%s'\n", (અક્ષર*)list);
	પાછા (અક્ષર*)list;
}
#endif

#ifndef __U_BOOT__
સ્થિર અક્ષર **expand_assignments(અક્ષર **argv, પૂર્ણાંક ગણતરી)
{
	પૂર્ણાંક i;
	અક્ષર **p;

	G.expanded_assignments = p = ખાલી;
	/* Expand assignments into one string each */
	માટે (i = 0; i < ગણતરી; i++) {
		p = add_string_to_strings(p,
			expand_string_to_string(argv[i],
				EXP_FLAG_ESC_GLOB_CHARS,
				/*unbackslash:*/ 1
			)
		);
		G.expanded_assignments = p;
	}
	G.expanded_assignments = ખાલી;
	પાછા p;
}

સ્થિર શૂન્ય switch_off_special_sigs(unsigned mask)
{
	unsigned sig = 0;
	જ્યારે ((mask >>= 1) != 0) {
		sig++;
		જો (!(mask & 1))
			ચાલુ;
#જો ENABLE_HUSH_TRAP
		જો (G_traps) {
			જો (G_traps[sig] && !G_traps[sig][0])
				/* trap is '', has to remain SIG_IGN */
				ચાલુ;
			મુક્ત(G_traps[sig]);
			G_traps[sig] = ખાલી;
		}
#endif
		/* We are here only if no trap or trap was not '' */
		install_sighandler(sig, SIG_DFL);
	}
}
#endif /* !__U_BOOT__ */

#ifndef __U_BOOT__
#જો BB_MMU
/* never called */
શૂન્ય re_execute_shell(અક્ષર ***to_free, અચલ અક્ષર *s,
		અક્ષર *g_argv0, અક્ષર **g_argv,
		અક્ષર **builtin_argv) NORETURN;

સ્થિર શૂન્ય reset_traps_to_defaults(શૂન્ય)
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
	જો (!G_traps && !mask)
		પાછા; /* already no traps and no special sigs */

	/* Switch off special sigs */
	switch_off_special_sigs(mask);
# જો ENABLE_HUSH_JOB
	G_fatal_sig_mask = 0;
# endif
	G.special_sig_mask &= ~SPECIAL_INTERACTIVE_SIGS;
	/* SIGQUIT,SIGCHLD and maybe SPECIAL_JOBSTOP_SIGS
	 * remain set in G.special_sig_mask */

# જો ENABLE_HUSH_TRAP
	જો (!G_traps)
		પાછા;

	/* Reset all sigs to default except ones with empty traps */
	માટે (sig = 0; sig < NSIG; sig++) {
		જો (!G_traps[sig])
			ચાલુ; /* no trap: nothing to do */
		જો (!G_traps[sig][0])
			ચાલુ; /* empty trap: has to remain SIG_IGN */
		/* sig has non-empty trap, reset it: */
		મુક્ત(G_traps[sig]);
		G_traps[sig] = ખાલી;
		/* There is no signal for trap 0 (EXIT) */
		જો (sig == 0)
			ચાલુ;
		install_sighandler(sig, pick_sighandler(sig));
	}
# endif
}

#નહીંતર /* !BB_MMU */

સ્થિર શૂન્ય re_execute_shell(અક્ષર ***to_free, અચલ અક્ષર *s,
		અક્ષર *g_argv0, અક્ષર **g_argv,
		અક્ષર **builtin_argv) NORETURN;
સ્થિર શૂન્ય re_execute_shell(અક્ષર ***to_free, અચલ અક્ષર *s,
		અક્ષર *g_argv0, અક્ષર **g_argv,
		અક્ષર **builtin_argv)
{
# define NOMMU_HACK_FMT ("-$%x:%x:%x:%x:%x:%llx" IF_HUSH_LOOPS(":%x"))
	/* delims + 2 * (number of bytes in printed hex numbers) */
	અક્ષર param_buf[કદ(NOMMU_HACK_FMT) + 2 * (કદ(પૂર્ણાંક)*6 + કદ(long long)*1)];
	અક્ષર *heredoc_argv[4];
	સંરચના variable *cur;
# જો ENABLE_HUSH_FUNCTIONS
	સંરચના function *funcp;
# endif
	અક્ષર **argv, **pp;
	unsigned cnt;
	unsigned long long empty_trap_mask;

	જો (!g_argv0) { /* heredoc */
		argv = heredoc_argv;
		argv[0] = (અક્ષર *) G.argv0_for_re_execing;
		argv[1] = (અક્ષર *) "-<";
		argv[2] = (અક્ષર *) s;
		argv[3] = ખાલી;
		pp = &argv[3]; /* used as pointer to empty environment */
		જાઓ do_exec;
	}

	cnt = 0;
	pp = builtin_argv;
	જો (pp) જ્યારે (*pp++)
		cnt++;

	empty_trap_mask = 0;
	જો (G_traps) {
		પૂર્ણાંક sig;
		માટે (sig = 1; sig < NSIG; sig++) {
			જો (G_traps[sig] && !G_traps[sig][0])
				empty_trap_mask |= 1LL << sig;
		}
	}

	sprintf(param_buf, NOMMU_HACK_FMT
			, (unsigned) G.મૂળ_pid
			, (unsigned) G.મૂળ_ppid
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
	માટે (cur = G.ટોચ_var; cur; cur = cur->next) {
		જો (!cur->flg_export || cur->flg_read_only)
			cnt += 2;
	}
# જો ENABLE_HUSH_LINENO_VAR
	cnt += 2;
# endif
# જો ENABLE_HUSH_FUNCTIONS
	માટે (funcp = G.ટોચ_func; funcp; funcp = funcp->next)
		cnt += 3;
# endif
	pp = g_argv;
	જ્યારે (*pp++)
		cnt++;
	*to_free = argv = pp = xzalloc(કદ(argv[0]) * cnt);
	*pp++ = (અક્ષર *) G.argv0_for_re_execing;
	*pp++ = param_buf;
	માટે (cur = G.ટોચ_var; cur; cur = cur->next) {
		જો (strcmp(cur->varstr, hush_version_str) == 0)
			ચાલુ;
		જો (cur->flg_read_only) {
			*pp++ = (અક્ષર *) "-R";
			*pp++ = cur->varstr;
		} નહીંતર જો (!cur->flg_export) {
			*pp++ = (અક્ષર *) "-V";
			*pp++ = cur->varstr;
		}
	}
# જો ENABLE_HUSH_LINENO_VAR
	*pp++ = (અક્ષર *) "-L";
	*pp++ = utoa(G.execute_lineno);
# endif
# જો ENABLE_HUSH_FUNCTIONS
	માટે (funcp = G.ટોચ_func; funcp; funcp = funcp->next) {
		*pp++ = (અક્ષર *) "-F";
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
	*pp++ = (અક્ષર *) "-c";
	*pp++ = (અક્ષર *) s;
	જો (builtin_argv) {
		જ્યારે (*++builtin_argv)
			*pp++ = *builtin_argv;
		*pp++ = (અક્ષર *) "";
	}
	*pp++ = g_argv0;
	જ્યારે (*g_argv)
		*pp++ = *g_argv++;
	/* *pp = NULL; - is already there */
	pp = environ;

 do_exec:
	debug_printf_exec("re_execute_shell pid:%d cmd:'%s'\n", getpid(), s);
	/* Don't propagate SIG_IGN to the child */
	જો (SPECIAL_JOBSTOP_SIGS != 0)
		switch_off_special_sigs(G.special_sig_mask & SPECIAL_JOBSTOP_SIGS);
	execve(bb_busybox_exec_path, argv, pp);
	/* Fallback. Useful for init=/bin/hush usage etc */
	જો (argv[0][0] == '/')
		execve(argv[0], argv, pp);
	xfunc_error_retval = 127;
	bb_simple_error_સંદેશ_and_die("can't re-execute the shell");
}
#endif  /* !BB_MMU */

#endif /* !__U_BOOT__ */

સ્થિર પૂર્ણાંક run_and_free_list(સંરચના pipe *pi);

/* Executing from string: eval, sh -c '...'
 *          or from file: /etc/profile, . file, sh <script>, sh (intereactive)
 * end_trigger controls how often we stop parsing
 * NUL: parse all, execute, return
 * ';': parse till ';' or newline, execute, repeat till EOF
 */
#ifndef __U_BOOT__
સ્થિર શૂન્ય parse_and_run_stream(સંરચના in_str *inp, પૂર્ણાંક end_trigger)
#નહીંતર /* __U_BOOT__ */
સ્થિર પૂર્ણાંક parse_and_run_stream(સંરચના in_str *inp, પૂર્ણાંક end_trigger)
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
	જ્યારે (1) {
#નહીંતર /* __U_BOOT__ */
	કરો {
#endif /* __U_BOOT__ */
		સંરચના pipe *pipe_list;

#જો ENABLE_HUSH_INTERACTIVE
		જો (end_trigger == ';') {
			G.promptmode = 0; /* PS1 */
			debug_printf_prompt("%s promptmode=%d\n", __func__, G.promptmode);
		}
#endif
		pipe_list = parse_stream(ખાલી, ખાલી, inp, end_trigger);
		જો (!pipe_list || pipe_list == ERR_PTR) { /* EOF/error */
			/* If we are in "big" script
			 * (not in `cmd` or something similar)...
			 */
			જો (pipe_list == ERR_PTR && end_trigger == ';') {
				/* Discard cached input (rest of line) */
				પૂર્ણાંક ch = inp->last_char;
				જ્યારે (ch != EOF && ch != '\n') {
					//bb_error_msg("Discarded:'%c'", ch);
					ch = i_getch(inp);
				}
				/* Force prompt */
				inp->p = ખાલી;
				/* This stream isn't empty */
				empty = 0;
				ચાલુ;
			}
			જો (!pipe_list && empty)
				G.last_exitcode = 0;
			તોડો;
		}
		debug_print_વૃક્ષ(pipe_list, 0);
		debug_printf_exec("parse_and_run_stream: run_and_free_list\n");
#ifndef __U_BOOT__
		run_and_free_list(pipe_list);
#નહીંતર /* __U_BOOT__ */
		પૂર્ણાંક rcode = run_and_free_list(pipe_list);
		/*
		 * We reset input string to not run the following command, so running
		 * 'exit; echo foo' does not print foo.
		 */
		જો (rcode <= EXIT_RET_CODE)
			setup_file_in_str(inp);
#endif /* __U_BOOT__ */
		empty = 0;
		જો (G_flag_return_in_progress == 1)
			તોડો;
#ifndef __U_BOOT__
	}
#નહીંતર /* __U_BOOT__ */
	/*
	 * This do/while is needed by run_command to avoid looping on a command
	 * with syntax error.
	 */
	} જ્યારે (!(G.run_command_flags & FLAG_EXIT_FROM_LOOP));

	પાછા G.last_exitcode;
#endif /* __U_BOOT__ */
}

#ifndef __U_BOOT__
સ્થિર શૂન્ય parse_and_run_string(અચલ અક્ષર *s)
#નહીંતર /* __U_BOOT__ */
સ્થિર પૂર્ણાંક parse_and_run_string(અચલ અક્ષર *s)
#endif /* __U_BOOT__ */
{
	સંરચના in_str આગત;
#ifndef __U_BOOT__
	IF_HUSH_LINENO_VAR(unsigned sv = G.parse_lineno;)
#નહીંતર /* __U_BOOT__ */
	//IF_HUSH_LINENO_VAR(unsigned sv = G.parse_lineno;)
#endif /* __U_BOOT__ */

	setup_string_in_str(&આગત, s);
#ifndef __U_BOOT__
	parse_and_run_stream(&આગત, '\0');
#નહીંતર /* __U_BOOT__ */
	પાછા parse_and_run_stream(&આગત, '\0');
#endif /* __U_BOOT__ */
#ifndef __U_BOOT__
	IF_HUSH_LINENO_VAR(unsigned sv = G.parse_lineno;)
#નહીંતર /* __U_BOOT__ */
	//IF_HUSH_LINENO_VAR(unsigned sv = G.parse_lineno;)
#endif /* __U_BOOT__ */
}

#ifdef __U_BOOT__
પૂર્ણાંક parse_string_outer_modern(અચલ અક્ષર *cmd, પૂર્ણાંક flags)
{
	પૂર્ણાંક ret;
	પૂર્ણાંક old_flags;

	/*
	 * Keep old values of run_command to be able to restore them once
	 * command was executed.
	 */
	old_flags = G.run_command_flags;
	G.run_command_flags = flags;

	ret = parse_and_run_string(cmd);

	G.run_command_flags = old_flags;

	પાછા ret;
}
#endif /* __U_BOOT__ */
#ifndef __U_BOOT__
સ્થિર શૂન્ય parse_and_run_file(HFILE *fp)
#નહીંતર /* __U_BOOT__ */
શૂન્ય parse_and_run_file(શૂન્ય)
#endif /* __U_BOOT__ */
{
	સંરચના in_str આગત;
#ifndef __U_BOOT__
	IF_HUSH_LINENO_VAR(unsigned sv = G.parse_lineno;)

	IF_HUSH_LINENO_VAR(G.parse_lineno = 1;)
	setup_file_in_str(&આગત, fp);
#નહીંતર /* __U_BOOT__ */
	setup_file_in_str(&આગત);
#endif /* __U_BOOT__ */
	parse_and_run_stream(&આગત, ';');
#ifndef __U_BOOT__
	IF_HUSH_LINENO_VAR(G.parse_lineno = sv;)
#endif /* !__U_BOOT__ */
}

#ifndef __U_BOOT__
#જો ENABLE_HUSH_TICK
સ્થિર પૂર્ણાંક generate_stream_from_string(અચલ અક્ષર *s, pid_t *pid_p)
{
	pid_t pid;
	પૂર્ણાંક channel[2];
# જો !BB_MMU
	અક્ષર **to_free = ખાલી;
# endif

	xpipe(channel);
	pid = BB_MMU ? xfork() : xvfork();
	જો (pid == 0) { /* child */
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
# જો ENABLE_HUSH_TRAP
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
		જો (is_prefixed_with(s, "trap")
		 && skip_whitespace(s + 4)[0] == '\0'
		) {
			સ્થિર અચલ અક્ષર *અચલ argv[] ALIGN_PTR = { ખાલી, ખાલી };
			builtin_trap((અક્ષર**)argv);
			fflush_all(); /* important */
			_exit(0);
		}
# endif
# જો BB_MMU
		/* Prevent it from trying to handle ctrl-z etc */
		IF_HUSH_JOB(G.run_list_level = 1;)
		CLEAR_RANDOM_T(&G.random_gen); /* or else $RANDOM repeats in child */
		reset_traps_to_defaults();
		IF_HUSH_MODE_X(G.x_mode_depth++;)
		//bb_error_msg("%s: ++x_mode_depth=%d", __func__, G.x_mode_depth);
		parse_and_run_string(s);
		_exit(G.last_exitcode);
# નહીંતર
	/* We re-execute after vfork on NOMMU. This makes this script safe:
	 * yes "0123456789012345678901234567890" | dd bs=32 count=64k >BIG
	 * huge=`cat BIG` # was blocking here forever
	 * echo OK
	 */
		re_execute_shell(&to_free,
				s,
				G.global_argv[0],
				G.global_argv + 1,
				ખાલી);
# endif
	}

	/* parent */
	*pid_p = pid;
# જો ENABLE_HUSH_FAST
	G.ગણતરી_SIGCHLD++;
//bb_error_msg("[%d] fork in generate_stream_from_string:"
//		" G.count_SIGCHLD:%d G.handled_SIGCHLD:%d",
//		getpid(), G.count_SIGCHLD, G.handled_SIGCHLD);
# endif
	enable_restore_tty_pgrp_on_exit();
# જો !BB_MMU
	મુક્ત(to_free);
# endif
	close(channel[1]);
	પાછા channel[0];
}

/* Return code is exit status of the process that is run. */
સ્થિર પૂર્ણાંક process_command_subs(o_string *dest, અચલ અક્ષર *s)
{
	FILE *fp;
	pid_t pid;
	પૂર્ણાંક status, ch, eol_cnt;

	fp = xfdopen_for_read(generate_stream_from_string(s, &pid));

	/* Now send results of command back into original context */
	eol_cnt = 0;
	જ્યારે ((ch = getc(fp)) != EOF) {
		જો (ch == '\0')
			ચાલુ;
		જો (ch == '\n') {
			eol_cnt++;
			ચાલુ;
		}
		જ્યારે (eol_cnt) {
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
	પાછા WEXITSTATUS(status);
}
#endif /* ENABLE_HUSH_TICK */

સ્થિર શૂન્ય setup_heredoc(સંરચના redir_struct *redir)
{
	સંરચના fd_pair pair;
	pid_t pid;
	પૂર્ણાંક len, written;
	/* the _body_ of heredoc (misleading field name) */
	અચલ અક્ષર *heredoc = redir->rd_filename;
	અક્ષર *expanded;
#જો !BB_MMU
	અક્ષર **to_free;
#endif

	expanded = ખાલી;
	જો (!(redir->rd_dup & HEREDOC_QUOTED)) {
		expanded = encode_then_expand_string(heredoc);
		જો (expanded)
			heredoc = expanded;
	}
	len = strlen(heredoc);

	close(redir->rd_fd); /* often saves dup2+close in xmove_fd */
	xpiped_pair(pair);
	xmove_fd(pair.rd, redir->rd_fd);

	/* Try writing without forking. Newer kernels have
	 * dynamically growing pipes. Must use non-blocking write! */
	ndelay_on(pair.wr);
	જ્યારે (1) {
		written = લિખ(pair.wr, heredoc, len);
		જો (written <= 0)
			તોડો;
		len -= written;
		જો (len == 0) {
			close(pair.wr);
			મુક્ત(expanded);
			પાછા;
		}
		heredoc += written;
	}
	ndelay_off(pair.wr);

	/* Okay, pipe buffer was not big enough */
	/* Note: we must not create a stray child (bastard? :)
	 * for the unsuspecting parent process. Child creates a grandchild
	 * and exits before parent execs the process which consumes heredoc
	 * (that exec happens after we return from this function) */
#જો !BB_MMU
	to_free = ખાલી;
#endif
	pid = xvfork();
	જો (pid == 0) {
		/* child */
		disable_restore_tty_pgrp_on_exit();
		pid = BB_MMU ? xfork() : xvfork();
		જો (pid != 0)
			_exit(0);
		/* grandchild */
		close(redir->rd_fd); /* read side of the pipe */
#જો BB_MMU
		full_write(pair.wr, heredoc, len); /* may loop or block */
		_exit(0);
#નહીંતર
		/* Delegate blocking writes to another process */
		xmove_fd(pair.wr, STDOUT_FILENO);
		re_execute_shell(&to_free, heredoc, ખાલી, ખાલી, ખાલી);
#endif
	}
	/* parent */
#જો ENABLE_HUSH_FAST
	G.ગણતરી_SIGCHLD++;
//bb_error_msg("[%d] fork in setup_heredoc: G.count_SIGCHLD:%d G.handled_SIGCHLD:%d", getpid(), G.count_SIGCHLD, G.handled_SIGCHLD);
#endif
	enable_restore_tty_pgrp_on_exit();
#જો !BB_MMU
	મુક્ત(to_free);
#endif
	close(pair.wr);
	મુક્ત(expanded);
	wait(ખાલી); /* wait till child has died */
}

સંરચના squirrel {
	પૂર્ણાંક orig_fd;
	પૂર્ણાંક moved_to;
	/* moved_to = n: fd was moved to n; restore back to orig_fd after redir */
	/* moved_to = -1: fd was opened by redirect; close orig_fd after redir */
};

સ્થિર સંરચના squirrel *append_squirrel(સંરચના squirrel *sq, પૂર્ણાંક i, પૂર્ણાંક orig, પૂર્ણાંક moved)
{
	sq = xrealloc(sq, (i + 2) * કદ(sq[0]));
	sq[i].orig_fd = orig;
	sq[i].moved_to = moved;
	sq[i+1].orig_fd = -1; /* end marker */
	પાછા sq;
}

સ્થિર સંરચના squirrel *add_squirrel(સંરચના squirrel *sq, પૂર્ણાંક fd, પૂર્ણાંક avoid_fd)
{
	પૂર્ણાંક moved_to;
	પૂર્ણાંક i;

	i = 0;
	જો (sq) માટે (; sq[i].orig_fd >= 0; i++) {
		/* If we collide with an already moved fd... */
		જો (fd == sq[i].moved_to) {
			moved_to = dup_CLOEXEC(sq[i].moved_to, avoid_fd);
			debug_printf_redir("redirect_fd %d: already busy, moving to %d\n", fd, moved_to);
			જો (moved_to < 0) {
				/* "echo 2>/dev/tty 10>&9999" testcase:
				 * We move fd 2 to 10, then discover we need to move fd 10
				 * (and not hit 9999) and the latter fails.
				 */
				પાછા ખાલી; /* fcntl failed */
			}
			sq[i].moved_to = moved_to;
			પાછા sq;
		}
		જો (fd == sq[i].orig_fd) {
			/* Example: echo Hello >/dev/null 1>&2 */
			debug_printf_redir("redirect_fd %d: already moved\n", fd);
			પાછા sq;
		}
	}

	/* If this fd is open, we move and remember it; if it's closed, moved_to = -1 */
	moved_to = dup_CLOEXEC(fd, avoid_fd);
	debug_printf_redir("redirect_fd %d: previous fd is moved to %d (-1 if it was closed)\n", fd, moved_to);
	જો (moved_to < 0 && errno != EBADF)
		પાછા ખાલી; /* fcntl failed (not because fd is closed) */
	પાછા append_squirrel(sq, i, fd, moved_to);
}

સ્થિર સંરચના squirrel *add_squirrel_closed(સંરચના squirrel *sq, પૂર્ણાંક fd)
{
	પૂર્ણાંક i;

	i = 0;
	જો (sq) માટે (; sq[i].orig_fd >= 0; i++) {
		/* If we collide with an already moved fd... */
		જો (fd == sq[i].orig_fd) {
			/* Examples:
			 * "echo 3>FILE 3>&- 3>FILE"
			 * "echo 3>&- 3>FILE"
			 * No need for last redirect to insert
			 * another "need to close 3" indicator.
			 */
			debug_printf_redir("redirect_fd %d: already moved or closed\n", fd);
			પાછા sq;
		}
	}

	debug_printf_redir("redirect_fd %d: previous fd was closed\n", fd);
	પાછા append_squirrel(sq, i, fd, -1);
}

/* fd: redirect wants this fd to be used (e.g. 3>file).
 * Move all conflicting internally used fds,
 * and remember them so that we can restore them later.
 */
સ્થિર પૂર્ણાંક save_fd_on_redirect(પૂર્ણાંક fd, પૂર્ણાંક avoid_fd, સંરચના squirrel **sqp)
{
	સંરચના squirrel *new_squirrel;

	જો (avoid_fd < 9) /* the important case here is that it can be -1 */
		avoid_fd = 9;

#જો ENABLE_HUSH_INTERACTIVE
	જો (fd != 0 /* don't trigger for G_interactive_fd == 0 (that's "not interactive" flag) */
	 && fd == G_interactive_fd
	) {
		/* Testcase: "ls -l /proc/$$/fd 255>&-" should work */
		G_interactive_fd = xdup_CLOEXEC_and_close(G_interactive_fd, avoid_fd);
		debug_printf_redir("redirect_fd %d: matches interactive_fd, moving it to %d\n", fd, G_interactive_fd);
		પાછા 1; /* "we closed fd" */
	}
#endif
	/* Are we called from setup_redirects(squirrel==NULL)
	 * in redirect in a [v]forked child?
	 */
	જો (sqp == ખાલી) {
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
		પાછા 0;
	}

	/* If this one of script's fds? */
	જો (move_HFILEs_on_redirect(fd, avoid_fd))
		પાછા 1; /* yes. "we closed fd" (actually moved it) */

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
	જો (sqp == ERR_PTR) {
		/* Don't preserve redirected fds: exec is _meant_ to change these */
		//bb_error_msg("sqp == ERR_PTR: exec >FILE");
		પાછા 0;
	}

	/* Check whether it collides with any open fds (e.g. stdio), save fds as needed */
	new_squirrel = add_squirrel(*sqp, fd, avoid_fd);
	જો (!new_squirrel)
		પાછા -1; /* redirect error */
	*sqp = new_squirrel;
	પાછા 0; /* "we did not close fd" */
}

સ્થિર શૂન્ય restore_redirects(સંરચના squirrel *sq)
{
	જો (sq) {
		પૂર્ણાંક i;
		માટે (i = 0; sq[i].orig_fd >= 0; i++) {
			જો (sq[i].moved_to >= 0) {
				/* We simply die on error */
				debug_printf_redir("restoring redirected fd from %d to %d\n", sq[i].moved_to, sq[i].orig_fd);
				xmove_fd(sq[i].moved_to, sq[i].orig_fd);
			} નહીંતર {
				/* cmd1 9>FILE; cmd2_should_see_fd9_closed */
				debug_printf_redir("restoring redirected fd %d: closing it\n", sq[i].orig_fd);
				close(sq[i].orig_fd);
			}
		}
		મુક્ત(sq);
	}
	જો (G.HFILE_stdin
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

#જો ENABLE_FEATURE_SH_STANDALONE && BB_MMU
સ્થિર શૂન્ય close_saved_fds_and_FILE_fds(શૂન્ય)
{
	જો (G_interactive_fd)
		close(G_interactive_fd);
	close_all_HFILE_list();
}
#endif

સ્થિર પૂર્ણાંક internally_opened_fd(પૂર્ણાંક fd, સંરચના squirrel *sq)
{
	પૂર્ણાંક i;

#જો ENABLE_HUSH_INTERACTIVE
	જો (fd == G_interactive_fd)
		પાછા 1;
#endif
	/* If this one of script's fds? */
	જો (fd_in_HFILEs(fd))
		પાછા 1;

	જો (sq) માટે (i = 0; sq[i].orig_fd >= 0; i++) {
		જો (fd == sq[i].moved_to)
			પાછા 1;
	}
	પાછા 0;
}

/* sqp != NULL means we squirrel away copies of stdin, stdout,
 * and stderr if they are redirected.
 * If redirection fails, return 1. This will make caller
 * skip command execution and restore already created redirect fds.
 */
સ્થિર પૂર્ણાંક setup_redirects(સંરચના command *prog, સંરચના squirrel **sqp)
{
	સંરચના redir_struct *redir;

	માટે (redir = prog->redirects; redir; redir = redir->next) {
		પૂર્ણાંક newfd;
		પૂર્ણાંક closed;

		જો (redir->rd_type == REDIRECT_HEREDOC2) {
			/* "rd_fd<<HERE" case */
			જો (save_fd_on_redirect(redir->rd_fd, /*avoid:*/ 0, sqp) < 0)
				પાછા 1;
			/* for REDIRECT_HEREDOC2, rd_filename holds _contents_
			 * of the heredoc */
			debug_printf_redir("set heredoc '%s'\n",
					redir->rd_filename);
			setup_heredoc(redir);
			ચાલુ;
		}

		જો (redir->rd_dup == REDIRFD_TO_FILE) {
			/* "rd_fd<*>file" case (<*> is <,>,>>,<>) */
			અક્ષર *p;
			પૂર્ણાંક mode;

			જો (redir->rd_filename == ખાલી) {
				/* Examples:
				 * "cmd >" (no filename)
				 * "cmd > <file" (2nd redirect starts too early)
				 */
				syntax_error("invalid redirect");
				પાછા 1;
			}
			mode = redir_table[redir->rd_type].mode;
			p = expand_string_to_string(redir->rd_filename,
				EXP_FLAG_ESC_GLOB_CHARS, /*unbackslash:*/ 1);
			newfd = open_or_warn(p, mode);
			મુક્ત(p);
			જો (newfd < 0) {
				/* Error message from open_or_warn can be lost
				 * if stderr has been redirected, but bash
				 * and ash both lose it as well
				 * (though zsh doesn't!)
				 */
				પાછા 1;
			}
			જો (newfd == redir->rd_fd && sqp
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
		} નહીંતર {
			/* "rd_fd>&rd_dup" or "rd_fd>&-" case */
			newfd = redir->rd_dup;
		}

		જો (newfd == redir->rd_fd)
			ચાલુ;

		/* if "N>FILE": move newfd to redir->rd_fd */
		/* if "N>&M": dup newfd to redir->rd_fd */
		/* if "N>&-": close redir->rd_fd (newfd is REDIRFD_CLOSE) */

		closed = save_fd_on_redirect(redir->rd_fd, /*avoid:*/ newfd, sqp);
		જો (closed < 0)
			પાછા 1; /* error */
		જો (newfd == REDIRFD_CLOSE) {
			/* "N>&-" means "close me" */
			જો (!closed) {
				/* ^^^ optimization: saving may already
				 * have closed it. If not... */
				close(redir->rd_fd);
			}
			/* Sometimes we do another close on restore, getting EBADF.
			 * Consider "echo 3>FILE 3>&-"
			 * first redirect remembers "need to close 3",
			 * and second redirect closes 3! Restore code then closes 3 again.
			 */
		} નહીંતર {
			/* if newfd is a script fd or saved fd, do not allow to use it */
			જો (internally_opened_fd(newfd, sqp && sqp != ERR_PTR ? *sqp : ખાલી)) {
				bb_error_સંદેશ("fd#%d is not open", newfd);
				પાછા 1;
			}
			જો (dup2(newfd, redir->rd_fd) < 0) {
				/* "echo >&99" testcase */
				bb_perror_સંદેશ("dup2(%d,%d)", newfd, redir->rd_fd);
				પાછા 1;
			}
			જો (redir->rd_dup == REDIRFD_TO_FILE)
				/* "rd_fd > FILE" */
				close(newfd);
			/* else: "rd_fd > rd_dup" */
		}
	}
	પાછા 0;
}

સ્થિર અક્ષર *find_in_path(અચલ અક્ષર *arg)
{
	અક્ષર *ret = ખાલી;
	અચલ અક્ષર *PATH = get_local_var_કિંમત("PATH");

	જો (!PATH)
		પાછા ખાલી;

	જ્યારે (1) {
		અચલ અક્ષર *end = strchrnul(PATH, ':');
		પૂર્ણાંક sz = end - PATH; /* must be int! */

		મુક્ત(ret);
		જો (sz != 0) {
			ret = xasprintf("%.*s/%s", sz, PATH, arg);
		} નહીંતર {
			/* We have xxx::yyyy in $PATH,
			 * it means "use current dir" */
			ret = xstrdup(arg);
		}
		જો (access(ret, F_OK) == 0)
			તોડો;

		જો (*end == '\0') {
			મુક્ત(ret);
			પાછા ખાલી;
		}
		PATH = end + 1;
	}

	પાછા ret;
}

સ્થિર અચલ સંરચના built_in_command *find_builtin_helper(અચલ અક્ષર *name,
		અચલ સંરચના built_in_command *x,
		અચલ સંરચના built_in_command *end)
{
	જ્યારે (x != end) {
		જો (strcmp(name, x->b_cmd) != 0) {
			x++;
			ચાલુ;
		}
		debug_printf_exec("found builtin '%s'\n", name);
		પાછા x;
	}
	પાછા ખાલી;
}
સ્થિર અચલ સંરચના built_in_command *find_builtin1(અચલ અક્ષર *name)
{
	પાછા find_builtin_helper(name, bltins1, &bltins1[ARRAY_SIZE(bltins1)]);
}
સ્થિર અચલ સંરચના built_in_command *find_builtin(અચલ અક્ષર *name)
{
	અચલ સંરચના built_in_command *x = find_builtin1(name);
	જો (x)
		પાછા x;
	પાછા find_builtin_helper(name, bltins2, &bltins2[ARRAY_SIZE(bltins2)]);
}

#જો ENABLE_HUSH_JOB && ENABLE_FEATURE_TAB_COMPLETION
સ્થિર અચલ અક્ષર * FAST_FUNC hush_command_name(પૂર્ણાંક i)
{
	જો (/*i >= 0 && */ i < ARRAY_SIZE(bltins1)) {
		પાછા bltins1[i].b_cmd;
	}
	i -= ARRAY_SIZE(bltins1);
	જો (i < ARRAY_SIZE(bltins2)) {
		પાછા bltins2[i].b_cmd;
	}
# જો ENABLE_HUSH_FUNCTIONS
	{
		સંરચના function *funcp;
		i -= ARRAY_SIZE(bltins2);
		માટે (funcp = G.ટોચ_func; funcp; funcp = funcp->next) {
			જો (--i < 0)
				પાછા funcp->name;
		}
	}
# endif
	પાછા ખાલી;
}
#endif
#endif /* !__U_BOOT__ */

#ifndef __U_BOOT__
સ્થિર શૂન્ય remove_nested_vars(શૂન્ય)
{
	સંરચના variable *cur;
	સંરચના variable **cur_pp;

	cur_pp = &G.ટોચ_var;
	જ્યારે ((cur = *cur_pp) != ખાલી) {
		જો (cur->var_nest_level <= G.var_nest_level) {
			cur_pp = &cur->next;
			ચાલુ;
		}
		/* Unexport */
		જો (cur->flg_export) {
			debug_printf_env("unexporting nested '%s'/%u\n", cur->varstr, cur->var_nest_level);
			bb_unsetenv(cur->varstr);
		}
		/* Remove from global list */
		*cur_pp = cur->next;
		/* Free */
		જો (!cur->max_len) {
			debug_printf_env("freeing nested '%s'/%u\n", cur->varstr, cur->var_nest_level);
			મુક્ત(cur->varstr);
		}
		મુક્ત(cur);
	}
}

સ્થિર શૂન્ય enter_var_nest_level(શૂન્ય)
{
	G.var_nest_level++;
	debug_printf_env("var_nest_level++ %u\n", G.var_nest_level);

	/* Try:	f() { echo -n .; f; }; f
	 * struct variable::var_nest_level is uint16_t,
	 * thus limiting recursion to < 2^16.
	 * In any case, with 8 Mbyte stack SEGV happens
	 * not too long after 2^16 recursions anyway.
	 */
	જો (G.var_nest_level > 0xff00)
		bb_error_સંદેશ_and_die("fatal recursion (depth %u)", G.var_nest_level);
}

સ્થિર શૂન્ય leave_var_nest_level(શૂન્ય)
{
	G.var_nest_level--;
	debug_printf_env("var_nest_level-- %u\n", G.var_nest_level);
	જો (HUSH_DEBUG && (પૂર્ણાંક)G.var_nest_level < 0)
		bb_simple_error_સંદેશ_and_die("BUG: nesting underflow");

	remove_nested_vars();
}
#endif /* __U_BOOT__ */

#જો ENABLE_HUSH_FUNCTIONS
સ્થિર સંરચના function **find_function_slot(અચલ અક્ષર *name)
{
	સંરચના function *funcp;
	સંરચના function **funcpp = &G.ટોચ_func;

	જ્યારે ((funcp = *funcpp) != ખાલી) {
		જો (strcmp(name, funcp->name) == 0) {
			debug_printf_exec("found function '%s'\n", name);
			તોડો;
		}
		funcpp = &funcp->next;
	}
	પાછા funcpp;
}

સ્થિર ALWAYS_INLINE અચલ સંરચના function *find_function(અચલ અક્ષર *name)
{
	અચલ સંરચના function *funcp = *find_function_slot(name);
	પાછા funcp;
}

/* Note: takes ownership on name ptr */
સ્થિર સંરચના function *new_function(અક્ષર *name)
{
	સંરચના function **funcpp = find_function_slot(name);
	સંરચના function *funcp = *funcpp;

	જો (funcp != ખાલી) {
		સંરચના command *cmd = funcp->parent_cmd;
		debug_printf_exec("func %p parent_cmd %p\n", funcp, cmd);
		જો (!cmd) {
			debug_printf_exec("freeing & replacing function '%s'\n", funcp->name);
			મુક્ત(funcp->name);
			/* Note: if !funcp->body, do not free body_as_string!
			 * This is a special case of "-F name body" function:
			 * body_as_string was not malloced! */
			જો (funcp->body) {
				free_pipe_list(funcp->body);
# જો !BB_MMU
				મુક્ત(funcp->body_as_string);
# endif
			}
		} નહીંતર {
			debug_printf_exec("reinserting in tree & replacing function '%s'\n", funcp->name);
			cmd->argv[0] = funcp->name;
			cmd->group = funcp->body;
# જો !BB_MMU
			cmd->group_as_string = funcp->body_as_string;
# endif
		}
	} નહીંતર {
		debug_printf_exec("remembering new function '%s'\n", name);
		funcp = *funcpp = xzalloc(કદ(*funcp));
		/*funcp->next = NULL;*/
	}

	funcp->name = name;
	પાછા funcp;
}

# જો ENABLE_HUSH_UNSET
સ્થિર શૂન્ય unset_func(અચલ અક્ષર *name)
{
	સંરચના function **funcpp = find_function_slot(name);
	સંરચના function *funcp = *funcpp;

	જો (funcp != ખાલી) {
		debug_printf_exec("freeing function '%s'\n", funcp->name);
		*funcpp = funcp->next;
		/* funcp is unlinked now, deleting it.
		 * Note: if !funcp->body, the function was created by
		 * "-F name body", do not free ->body_as_string
		 * and ->name as they were not malloced. */
		જો (funcp->body) {
			free_pipe_list(funcp->body);
			મુક્ત(funcp->name);
#  જો !BB_MMU
			મુક્ત(funcp->body_as_string);
#  endif
		}
		મુક્ત(funcp);
	}
}
# endif

# જો BB_MMU
#define exec_function(to_free, funcp, argv) \
	exec_function(funcp, argv)
# endif
સ્થિર શૂન્ય exec_function(અક્ષર ***to_free,
		અચલ સંરચના function *funcp,
		અક્ષર **argv) NORETURN;
સ્થિર શૂન્ય exec_function(અક્ષર ***to_free,
		અચલ સંરચના function *funcp,
		અક્ષર **argv)
{
# જો BB_MMU
	પૂર્ણાંક n;

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
# નહીંતર
//?	close_saved_fds_and_FILE_fds();

//TODO: check whether "true | func_with_return" works

	re_execute_shell(to_free,
			funcp->body_as_string,
			G.global_argv[0],
			argv + 1,
			ખાલી);
# endif
}

સ્થિર પૂર્ણાંક run_function(અચલ સંરચના function *funcp, અક્ષર **argv)
{
	પૂર્ણાંક rc;
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
# જો !BB_MMU
	જો (!funcp->body) {
		/* Function defined by -F */
		parse_and_run_string(funcp->body_as_string);
		rc = G.last_exitcode;
	} નહીંતર
# endif
	{
		rc = run_list(funcp->body);
	}

	IF_HUSH_LOCAL(G.func_nest_level--;)
	IF_HUSH_LOCAL(leave_var_nest_level();)

	G_flag_return_in_progress = sv_flg;
# જો ENABLE_HUSH_TRAP
	debug_printf_exec("G.return_exitcode=-1\n");
	G.return_exitcode = -1; /* invalidate stashed return value */
# endif

	restore_G_args(&sv, argv);

	પાછા rc;
}
#endif /* ENABLE_HUSH_FUNCTIONS */

#ifndef __U_BOOT__
#જો BB_MMU
#define exec_builtin(to_free, x, argv) \
	exec_builtin(x, argv)
#નહીંતર
#define exec_builtin(to_free, x, argv) \
	exec_builtin(to_free, argv)
#endif
સ્થિર શૂન્ય exec_builtin(અક્ષર ***to_free,
		અચલ સંરચના built_in_command *x,
		અક્ષર **argv) NORETURN;
સ્થિર શૂન્ય exec_builtin(અક્ષર ***to_free,
		અચલ સંરચના built_in_command *x,
		અક્ષર **argv)
{
#જો BB_MMU
	પૂર્ણાંક rcode;
//?	close_saved_fds_and_FILE_fds();
	rcode = x->b_function(argv);
	fflush_all();
	_exit(rcode);
#નહીંતર
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
સ્થિર શૂન્ય execvp_or_die(અક્ષર **argv) NORETURN;
સ્થિર શૂન્ય execvp_or_die(અક્ષર **argv)
{
	પૂર્ણાંક e;
	debug_printf_exec("execing '%s'\n", argv[0]);
	/* Don't propagate SIG_IGN to the child */
	જો (SPECIAL_JOBSTOP_SIGS != 0)
		switch_off_special_sigs(G.special_sig_mask & SPECIAL_JOBSTOP_SIGS);
	execvp(argv[0], argv);
	e = 2;
	જો (errno == EACCES) e = 126;
	જો (errno == ENOENT) e = 127;
	bb_perror_સંદેશ("can't execute '%s'", argv[0]);
	_exit(e);
}

#જો ENABLE_HUSH_MODE_X
સ્થિર શૂન્ય x_mode_print_optionally_squoted(અચલ અક્ષર *str)
{
	unsigned len;
	અચલ અક્ષર *cp;

	cp = str;

	/* the set of chars which-cause-string-to-be-squoted mimics bash */
	/* test a char with: bash -c 'set -x; echo "CH"' */
	જો (str[strcspn(str, "\\\"'`$(){}[]<>;#&|~*?!^"
			" " "\001\002\003\004\005\006\007"
			"\010\011\012\013\014\015\016\017"
			"\020\021\022\023\024\025\026\027"
			"\030\031\032\033\034\035\036\037"
			)
		] == '\0'
	) {
		/* string has no special chars */
		x_mode_addstr(str);
		પાછા;
	}

	cp = str;
	માટે (;;) {
		/* print '....' up to EOL or first squote */
		len = (પૂર્ણાંક)(strchrnul(cp, '\'') - cp);
		જો (len != 0) {
			x_mode_addchr('\'');
			x_mode_addblock(cp, len);
			x_mode_addchr('\'');
			cp += len;
		}
		જો (*cp == '\0')
			તોડો;
		/* string contains squote(s), print them as \' */
		x_mode_addchr('\\');
		x_mode_addchr('\'');
		cp++;
	}
}
સ્થિર શૂન્ય dump_cmd_in_x_mode(અક્ષર **argv)
{
	જો (G_x_mode && argv) {
		unsigned n;

		/* "+[+++...][ cmd...]\n\0" */
		x_mode_prefix();
		n = 0;
		જ્યારે (argv[n]) {
			x_mode_addchr(' ');
			જો (argv[n][0] == '\0') {
				x_mode_addchr('\'');
				x_mode_addchr('\'');
			} નહીંતર {
				x_mode_print_optionally_squoted(argv[n]);
			}
			n++;
		}
		x_mode_flush();
	}
}
#નહીંતર
# define dump_cmd_in_x_mode(argv) ((શૂન્ય)0)
#endif
#endif /* !__U_BOOT__ */

#ifndef __U_BOOT__
#જો ENABLE_HUSH_COMMAND
સ્થિર શૂન્ય if_command_vV_print_and_exit(અક્ષર opt_vV, અક્ષર *cmd, અચલ અક્ષર *explanation)
{
	અક્ષર *to_free;

	જો (!opt_vV)
		પાછા;

	to_free = ખાલી;
	જો (!explanation) {
		અક્ષર *path = getenv("PATH");
		explanation = to_free = find_executable(cmd, &path); /* path == NULL is ok */
		જો (!explanation)
			_exit(1); /* PROG was not found */
		જો (opt_vV != 'V')
			cmd = to_free; /* -v PROG prints "/path/to/PROG" */
	}
	printf((opt_vV == 'V') ? "%s is %s\n" : "%s\n", cmd, explanation);
	મુક્ત(to_free);
	fflush_all();
	_exit(0);
}
#નહીંતર
# define if_command_vV_print_and_exit(a,b,c) ((શૂન્ય)0)
#endif
#endif /* !__U_BOOT__ */

#જો BB_MMU
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
સ્થિર શૂન્ય pseudo_exec_argv(nommu_save_t *nommu_save,
		અક્ષર **argv, પૂર્ણાંક assignment_cnt,
		અક્ષર **argv_expanded) NORETURN;
સ્થિર NOINLINE શૂન્ય pseudo_exec_argv(nommu_save_t *nommu_save,
		અક્ષર **argv, પૂર્ણાંક assignment_cnt,
		અક્ષર **argv_expanded)
{
	અચલ સંરચના built_in_command *x;
	સંરચના variable **sv_shadowed;
	અક્ષર **new_env;
	IF_HUSH_COMMAND(અક્ષર opt_vV = 0;)
	IF_HUSH_FUNCTIONS(અચલ સંરચના function *funcp;)

	new_env = expand_assignments(argv, assignment_cnt);
	dump_cmd_in_x_mode(new_env);

	જો (!argv[assignment_cnt]) {
		/* Case when we are here: ... | var=val | ...
		 * (note that we do not exit early, i.e., do not optimize out
		 * expand_assignments(): think about ... | var=`sleep 1` | ...
		 */
		free_strings(new_env);
		_exit_SUCCESS();
	}

	sv_shadowed = G.shadowed_vars_pp;
#જો BB_MMU
	G.shadowed_vars_pp = ખાલી; /* "don't save, free them instead" */
#નહીંતર
	G.shadowed_vars_pp = &nommu_save->old_vars;
	G.var_nest_level++;
#endif
	set_vars_and_save_old(new_env);
	G.shadowed_vars_pp = sv_shadowed;

	જો (argv_expanded) {
		argv = argv_expanded;
	} નહીંતર {
		argv = expand_strvec_to_strvec(argv + assignment_cnt);
#જો !BB_MMU
		nommu_save->argv = argv;
#endif
	}
	dump_cmd_in_x_mode(argv);

#જો ENABLE_FEATURE_SH_STANDALONE || BB_MMU
	જો (strchr(argv[0], '/') != ખાલી)
		જાઓ skip;
#endif

#જો ENABLE_HUSH_FUNCTIONS
	/* Check if the command matches any functions (this goes before bltins) */
	funcp = find_function(argv[0]);
	જો (funcp)
		exec_function(&nommu_save->argv_from_re_execing, funcp, argv);
#endif

#જો ENABLE_HUSH_COMMAND
	/* "command BAR": run BAR without looking it up among functions
	 * "command -v BAR": print "BAR" or "/path/to/BAR"; or exit 1
	 * "command -V BAR": print "BAR is {a function,a shell builtin,/path/to/BAR}"
	 */
	જ્યારે (strcmp(argv[0], "command") == 0 && argv[1]) {
		અક્ષર *p;

		argv++;
		p = *argv;
		જો (p[0] != '-' || !p[1])
			ચાલુ; /* bash allows "command command command [-OPT] BAR" */

		માટે (;;) {
			p++;
			પસંદગી (*p) {
			સ્થિતિ '\0':
				argv++;
				p = *argv;
				જો (p[0] != '-' || !p[1])
					જાઓ after_opts;
				ચાલુ; /* next arg is also -opts, process it too */
			સ્થિતિ 'v':
			સ્થિતિ 'V':
				opt_vV = *p;
				ચાલુ;
			મૂળભૂત:
				bb_error_સંદેશ_and_die("%s: %s: invalid option", "command", argv[0]);
			}
		}
	}
 after_opts:
# જો ENABLE_HUSH_FUNCTIONS
	જો (opt_vV && find_function(argv[0]))
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
	જો (x) {
		if_command_vV_print_and_exit(opt_vV, argv[0], "a shell builtin");
		exec_builtin(&nommu_save->argv_from_re_execing, x, argv);
	}

#જો ENABLE_FEATURE_SH_STANDALONE
	/* Check if the command matches any busybox applets */
	{
		પૂર્ણાંક a = find_applet_by_name(argv[0]);
		જો (a >= 0) {
			if_command_vV_print_and_exit(opt_vV, argv[0], "an applet");
# જો BB_MMU /* see above why on NOMMU it is not allowed */
			જો (APPLET_IS_NOEXEC(a)) {
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
			જો (SPECIAL_JOBSTOP_SIGS != 0)
				switch_off_special_sigs(G.special_sig_mask & SPECIAL_JOBSTOP_SIGS);
			execv(bb_busybox_exec_path, argv);
			/* If they called chroot or otherwise made the binary no longer
			 * executable, fall through */
		}
	}
#endif

#જો ENABLE_FEATURE_SH_STANDALONE || BB_MMU
 skip:
#endif
	if_command_vV_print_and_exit(opt_vV, argv[0], ખાલી);
	execvp_or_die(argv);
}

/* Called after [v]fork() in run_pipe
 */
સ્થિર શૂન્ય pseudo_exec(nommu_save_t *nommu_save,
		સંરચના command *command,
		અક્ષર **argv_expanded) NORETURN;
સ્થિર શૂન્ય pseudo_exec(nommu_save_t *nommu_save,
		સંરચના command *command,
		અક્ષર **argv_expanded)
{
#જો ENABLE_HUSH_FUNCTIONS
	જો (command->cmd_type == CMD_FUNCDEF) {
		/* Ignore funcdefs in pipes:
		 * true | f() { cmd }
		 */
		_exit(0);
	}
#endif

	જો (command->argv) {
		pseudo_exec_argv(nommu_save, command->argv,
				command->assignment_cnt, argv_expanded);
	}

	જો (command->group) {
		/* Cases when we are here:
		 * ( list )
		 * { list } &
		 * ... | ( list ) | ...
		 * ... | { list } | ...
		 */
#જો BB_MMU
		પૂર્ણાંક rcode;
		debug_printf_exec("pseudo_exec: run_list\n");
		reset_traps_to_defaults();
		rcode = run_list(command->group);
		/* OK to leak memory by not calling free_pipe_list,
		 * since this process is about to exit */
		_exit(rcode);
#નહીંતર
		re_execute_shell(&nommu_save->argv_from_re_execing,
				command->group_as_string,
				G.global_argv[0],
				G.global_argv + 1,
				ખાલી);
#endif
	}

	/* Case when we are here: ... | >file */
	debug_printf_exec("pseudo_exec'ed null command\n");
	_exit_SUCCESS();
}

#જો ENABLE_HUSH_JOB
સ્થિર અચલ અક્ષર *get_cmdtext(સંરચના pipe *pi)
{
	અક્ષર **argv;
	અક્ષર *p;
	પૂર્ણાંક len;

	/* This is subtle. ->cmdtext is created only on first backgrounding.
	 * (Think "cat, <ctrl-z>, fg, <ctrl-z>, fg, <ctrl-z>...." here...)
	 * On subsequent bg argv is trashed, but we won't use it */
	જો (pi->cmdtext)
		પાછા pi->cmdtext;

	argv = pi->cmds[0].argv;
	જો (!argv) {
		pi->cmdtext = xzalloc(1);
		પાછા pi->cmdtext;
	}
	len = 0;
	કરો {
		len += strlen(*argv) + 1;
	} જ્યારે (*++argv);
	p = xmalloc(len);
	pi->cmdtext = p;
	argv = pi->cmds[0].argv;
	કરો {
		p = stpcpy(p, *argv);
		*p++ = ' ';
	} જ્યારે (*++argv);
	p[-1] = '\0';
	પાછા pi->cmdtext;
}

સ્થિર શૂન્ય remove_job_from_table(સંરચના pipe *pi)
{
	સંરચના pipe *prev_pipe;

	જો (pi == G.job_list) {
		G.job_list = pi->next;
	} નહીંતર {
		prev_pipe = G.job_list;
		જ્યારે (prev_pipe->next != pi)
			prev_pipe = prev_pipe->next;
		prev_pipe->next = pi->next;
	}
	G.last_jobid = 0;
	જો (G.job_list)
		G.last_jobid = G.job_list->jobid;
}

સ્થિર શૂન્ય delete_finished_job(સંરચના pipe *pi)
{
	remove_job_from_table(pi);
	free_pipe(pi);
}

સ્થિર શૂન્ય clean_up_last_dead_job(શૂન્ય)
{
	જો (G.job_list && !G.job_list->alive_cmds)
		delete_finished_job(G.job_list);
}

સ્થિર શૂન્ય ઉમેરો_job_into_table(સંરચના pipe *pi)
{
	સંરચના pipe *job, **jobp;
	પૂર્ણાંક i;

	clean_up_last_dead_job();

	/* Find the end of the list, and find next job ID to use */
	i = 0;
	jobp = &G.job_list;
	જ્યારે ((job = *jobp) != ખાલી) {
		જો (job->jobid > i)
			i = job->jobid;
		jobp = &job->next;
	}
	pi->jobid = i + 1;

	/* Create a new job struct at the end */
	job = *jobp = xmemdup(pi, કદ(*pi));
	job->next = ખાલી;
	job->cmds = xzalloc(કદ(pi->cmds[0]) * pi->અંક_cmds);
	/* Cannot copy entire pi->cmds[] vector! This causes double frees */
	માટે (i = 0; i < pi->અંક_cmds; i++) {
		job->cmds[i].pid = pi->cmds[i].pid;
		/* all other fields are not used and stay zero */
	}
	job->cmdtext = xstrdup(get_cmdtext(pi));

	જો (G_interactive_fd)
		printf("[%u] %u %s\n", job->jobid, (unsigned)job->cmds[0].pid, job->cmdtext);
	G.last_jobid = job->jobid;
}
#endif /* JOB */

સ્થિર પૂર્ણાંક job_exited_or_stopped(સંરચના pipe *pi)
{
	પૂર્ણાંક rcode, i;

	જો (pi->alive_cmds != pi->stopped_cmds)
		પાછા -1;

	/* All processes in fg pipe have exited or stopped */
	rcode = 0;
	i = pi->અંક_cmds;
	જ્યારે (--i >= 0) {
		rcode = pi->cmds[i].cmd_exitcode;
		/* usually last process gives overall exitstatus,
		 * but with "set -o pipefail", last *failed* process does */
		જો (G.o_opt[OPT_O_PIPEFAIL] == 0 || rcode != 0)
			તોડો;
	}
	IF_HAS_KEYWORDS(જો (pi->pi_inverted) rcode = !rcode;)
	પાછા rcode;
}

સ્થિર પૂર્ણાંક process_wait_result(સંરચના pipe *fg_pipe, pid_t childpid, પૂર્ણાંક status)
{
#જો ENABLE_HUSH_JOB
	સંરચના pipe *pi;
#endif
	પૂર્ણાંક i, dead;

	dead = WIFEXITED(status) || WIFSIGNALED(status);

#જો DEBUG_JOBS
	જો (WIFSTOPPED(status))
		debug_printf_jobs("pid %d stopped by sig %d (exitcode %d)\n",
				childpid, WSTOPSIG(status), WEXITSTATUS(status));
	જો (WIFSIGNALED(status))
		debug_printf_jobs("pid %d killed by sig %d (exitcode %d)\n",
				childpid, WTERMSIG(status), WEXITSTATUS(status));
	જો (WIFEXITED(status))
		debug_printf_jobs("pid %d exited, exitcode %d\n",
				childpid, WEXITSTATUS(status));
#endif
	/* Were we asked to wait for a fg pipe? */
	જો (fg_pipe) {
		i = fg_pipe->અંક_cmds;

		જ્યારે (--i >= 0) {
			પૂર્ણાંક rcode;

			debug_printf_jobs("check pid %d\n", fg_pipe->cmds[i].pid);
			જો (fg_pipe->cmds[i].pid != childpid)
				ચાલુ;
			જો (dead) {
				પૂર્ણાંક ex;
				fg_pipe->cmds[i].pid = 0;
				fg_pipe->alive_cmds--;
				ex = WEXITSTATUS(status);
				/* bash prints killer signal's name for *last*
				 * process in pipe (prints just newline for SIGINT/SIGPIPE).
				 * Mimic this. Example: "sleep 5" + (^\ or kill -QUIT)
				 */
				જો (WIFSIGNALED(status)) {
					પૂર્ણાંક sig = WTERMSIG(status);
#જો ENABLE_HUSH_JOB
					જો (G.run_list_level == 1
					/* ^^^^^ Do not print in nested contexts, example:
					 * echo `sleep 1; sh -c 'kill -9 $$'` - prints "137", NOT "Killed 137"
					 */
					 && i == fg_pipe->અંક_cmds-1
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
			} નહીંતર {
				fg_pipe->stopped_cmds++;
			}
			debug_printf_jobs("fg_pipe: alive_cmds %d stopped_cmds %d\n",
					fg_pipe->alive_cmds, fg_pipe->stopped_cmds);
			rcode = job_exited_or_stopped(fg_pipe);
			જો (rcode >= 0) {
/* Note: *non-interactive* bash does not continue if all processes in fg pipe
 * are stopped. Testcase: "cat | cat" in a script (not on command line!)
 * and "killall -STOP cat" */
				જો (G_interactive_fd) {
#જો ENABLE_HUSH_JOB
					જો (fg_pipe->alive_cmds != 0)
						ઉમેરો_job_into_table(fg_pipe);
#endif
					પાછા rcode;
				}
				જો (fg_pipe->alive_cmds == 0)
					પાછા rcode;
			}
			/* There are still running processes in the fg_pipe */
			પાછા -1;
		}
		/* It wasn't in fg_pipe, look for process in bg pipes */
	}

#જો ENABLE_HUSH_JOB
	/* We were asked to wait for bg or orphaned children */
	/* No need to remember exitcode in this case */
	માટે (pi = G.job_list; pi; pi = pi->next) {
		માટે (i = 0; i < pi->અંક_cmds; i++) {
			જો (pi->cmds[i].pid == childpid)
				જાઓ found_pi_and_prognum;
		}
	}
	/* Happens when shell is used as init process (init=/bin/sh) */
	debug_printf("checkjobs: pid %d was not in our list!\n", childpid);
	પાછા -1; /* this wasn't a process from fg_pipe */

 found_pi_and_prognum:
	જો (dead) {
		/* child exited */
		પૂર્ણાંક rcode = WEXITSTATUS(status);
		જો (WIFSIGNALED(status))
			/* NB: not 128 + sig, MIPS has sig 128 */
			rcode = 128 | WTERMSIG(status);
		pi->cmds[i].cmd_exitcode = rcode;
		જો (G.last_bg_pid == pi->cmds[i].pid)
			G.last_bg_pid_exitcode = rcode;
		pi->cmds[i].pid = 0;
		pi->alive_cmds--;
		જો (!pi->alive_cmds) {
# જો ENABLE_HUSH_BASH_COMPAT
			G.dead_job_exitcode = job_exited_or_stopped(pi);
# endif
			જો (G_interactive_fd) {
				printf(JOB_STATUS_FORMAT, pi->jobid,
						"Done", pi->cmdtext);
				delete_finished_job(pi);
			} નહીંતર {
/*
 * bash deletes finished jobs from job table only in interactive mode,
 * after "jobs" cmd, or if pid of a new process matches one of the old ones
 * (see cleanup_dead_jobs(), delete_old_job(), J_NOTIFIED in bash source).
 * Testcase script: "(exit 3) & sleep 1; wait %1; echo $?" prints 3 in bash.
 * We only retain one "dead" job, if it's the single job on the list.
 * This covers most of real-world scenarios where this is useful.
 */
				જો (pi != G.job_list)
					delete_finished_job(pi);
			}
		}
	} નહીંતર {
		/* child stopped */
		pi->stopped_cmds++;
	}
#endif
	પાછા -1; /* this wasn't a process from fg_pipe */
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
સ્થિર પૂર્ણાંક checkjobs(સંરચના pipe *fg_pipe, pid_t waitfor_pid)
{
	પૂર્ણાંક attributes;
	પૂર્ણાંક status;
	પૂર્ણાંક rcode = 0;

	debug_printf_jobs("checkjobs %p\n", fg_pipe);

	attributes = WUNTRACED;
	જો (fg_pipe == ખાલી)
		attributes |= WNOHANG;

	errno = 0;
#જો ENABLE_HUSH_FAST
	જો (G.handled_SIGCHLD == G.ગણતરી_SIGCHLD) {
//bb_error_msg("[%d] checkjobs: G.count_SIGCHLD:%d G.handled_SIGCHLD:%d children?:%d fg_pipe:%p",
//getpid(), G.count_SIGCHLD, G.handled_SIGCHLD, G.we_have_children, fg_pipe);
		/* There was neither fork nor SIGCHLD since last waitpid */
		/* Avoid doing waitpid syscall if possible */
		જો (!G.we_have_children) {
			errno = ECHILD;
			પાછા -1;
		}
		જો (fg_pipe == ખાલી) { /* is WNOHANG set? */
			/* We have children, but they did not exit
			 * or stop yet (we saw no SIGCHLD) */
			પાછા 0;
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
	જ્યારે (1) {
		pid_t childpid;
#જો ENABLE_HUSH_FAST
		પૂર્ણાંક i;
		i = G.ગણતરી_SIGCHLD;
#endif
		childpid = waitpid(-1, &status, attributes);
		જો (childpid <= 0) {
			જો (childpid && errno != ECHILD)
				bb_simple_perror_સંદેશ("waitpid");
#જો ENABLE_HUSH_FAST
			નહીંતર { /* Until next SIGCHLD, waitpid's are useless */
				G.we_have_children = (childpid == 0);
				G.handled_SIGCHLD = i;
//bb_error_msg("[%d] checkjobs: waitpid returned <= 0, G.count_SIGCHLD:%d G.handled_SIGCHLD:%d", getpid(), G.count_SIGCHLD, G.handled_SIGCHLD);
			}
#endif
			/* ECHILD (no children), or 0 (no change in children status) */
			rcode = childpid;
			તોડો;
		}
		rcode = process_wait_result(fg_pipe, childpid, status);
		જો (rcode >= 0) {
			/* fg_pipe exited or stopped */
			તોડો;
		}
		જો (childpid == waitfor_pid) { /* "wait PID" */
			debug_printf_exec("childpid==waitfor_pid:%d status:0x%08x\n", childpid, status);
			rcode = WEXITSTATUS(status);
			જો (WIFSIGNALED(status))
				rcode = 128 | WTERMSIG(status);
			જો (WIFSTOPPED(status))
				/* bash: "cmd & wait $!" and cmd stops: $? = 128 | stopsig */
				rcode = 128 | WSTOPSIG(status);
			rcode++;
			તોડો; /* "wait PID" called us, give it exitcode+1 */
		}
#જો ENABLE_HUSH_BASH_COMPAT
		જો (-1 == waitfor_pid /* "wait -n" (wait for any one job) */
		 && G.dead_job_exitcode >= 0 /* some job did finish */
		) {
			debug_printf_exec("waitfor_pid:-1\n");
			rcode = G.dead_job_exitcode + 1;
			તોડો;
		}
#endif
		/* This wasn't one of our processes, or */
		/* fg_pipe still has running processes, do waitpid again */
	} /* while (waitpid succeeds)... */

	પાછા rcode;
}

#જો ENABLE_HUSH_JOB
સ્થિર પૂર્ણાંક checkjobs_and_fg_shell(સંરચના pipe *fg_pipe)
{
	pid_t p;
	પૂર્ણાંક rcode = checkjobs(fg_pipe, 0 /*(no pid to wait for)*/);
	જો (G_saved_tty_pgrp) {
		/* Job finished, move the shell to the foreground */
		p = getpgrp(); /* our process group id */
		debug_printf_jobs("fg'ing ourself: getpgrp()=%d\n", (પૂર્ણાંક)p);
		tcsetpgrp(G_interactive_fd, p);
	}
	પાછા rcode;
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
સ્થિર શૂન્ય set_G_ifs(શૂન્ય)
{
	/* Testcase: set -- q w e; (IFS='' echo "$*"; IFS=''; echo "$*"); echo "$*"
	 * Result should be 3 lines: q w e, qwe, q w e
	 */
	જો (G.ifs_whitespace != G.ifs)
		મુક્ત(G.ifs_whitespace);
	G.ifs = get_local_var_કિંમત("IFS");
	જો (G.ifs) {
		અક્ષર *p;
		G.ifs_whitespace = (અક્ષર*)G.ifs;
		p = skip_whitespace(G.ifs);
		જો (*p) {
			/* Not all $IFS is whitespace */
			અક્ષર *d;
			પૂર્ણાંક len = p - G.ifs;
			p = skip_non_whitespace(p);
			G.ifs_whitespace = xmalloc(len + strlen(p) + 1); /* can overestimate */
			d = mempcpy(G.ifs_whitespace, G.ifs, len);
			જ્યારે (*p) {
				જો (isspace(*p))
					*d++ = *p;
				p++;
			}
			*d = '\0';
		}
	} નહીંતર {
		G.ifs = defifs;
		G.ifs_whitespace = (અક્ષર*)G.ifs;
	}
}
#ifndef __U_BOOT__
#જો !ENABLE_HUSH_MODE_X
#define redirect_and_varexp_helper(command, sqp, argv_expanded) \
	redirect_and_varexp_helper(command, sqp)
#endif
સ્થિર પૂર્ણાંક redirect_and_varexp_helper(
		સંરચના command *command,
		સંરચના squirrel **sqp,
		અક્ષર **argv_expanded)
{
	/* Assignments occur before redirects. Try:
	 * a=`sleep 1` sleep 2 3>/qwe/rty
	 */

	અક્ષર **new_env = expand_assignments(command->argv, command->assignment_cnt);
	dump_cmd_in_x_mode(new_env);
	dump_cmd_in_x_mode(argv_expanded);
	/* this takes ownership of new_env[i] elements, and frees new_env: */
	set_vars_and_save_old(new_env);

	પાછા setup_redirects(command, sqp);
}
#endif /* !__U_BOOT__ */

સ્થિર NOINLINE પૂર્ણાંક run_pipe(સંરચના pipe *pi)
{
	સ્થિર અચલ અક્ષર *અચલ null_ptr = ખાલી;

	પૂર્ણાંક cmd_no;
#ifndef __U_BOOT__
	પૂર્ણાંક next_infd;
#endif /* !__U_BOOT__ */
	સંરચના command *command;
	અક્ષર **argv_expanded;
	અક્ષર **argv;
#ifndef __U_BOOT__
	સંરચના squirrel *squirrel = ખાલી;
#endif /* !__U_BOOT__ */
	પૂર્ણાંક rcode;

#ifdef __U_BOOT__
	/*
	 * Set rcode here to avoid returning a garbage value in the middle of
	 * the function.
	 * Also, if an error occurs, rcode value would be changed and last
	 * return will signal the error.
	 */
	rcode = 0;
#endif /* __U_BOOT__ */

	debug_printf_exec("run_pipe start: members:%d\n", pi->અંક_cmds);
	debug_enter();

	set_G_ifs();

#ifndef __U_BOOT__
	IF_HUSH_JOB(pi->pgrp = -1;)
	pi->stopped_cmds = 0;
#endif /* !__U_BOOT__ */
	command = &pi->cmds[0];
	argv_expanded = ખાલી;

#ifndef __U_BOOT__
	જો (pi->અંક_cmds != 1
	 || pi->followup == PIPE_BG
	 || command->cmd_type == CMD_SUBSHELL
	) {
		જાઓ must_fork;
	}

	pi->alive_cmds = 1;
#endif /* !__U_BOOT__ */

	debug_printf_exec(": group:%p argv:'%s'\n",
		command->group, command->argv ? command->argv[0] : "NONE");

	જો (command->group) {
#જો ENABLE_HUSH_FUNCTIONS
		જો (command->cmd_type == CMD_FUNCDEF) {
			/* "executing" func () { list } */
			સંરચના function *funcp;

			funcp = new_function(command->argv[0]);
			/* funcp->name is already set to argv[0] */
			funcp->body = command->group;
# જો !BB_MMU
			funcp->body_as_string = command->group_as_string;
			command->group_as_string = ખાલી;
# endif
			command->group = ખાલી;
			command->argv[0] = ખાલી;
			debug_printf_exec("cmd %p has child func at %p\n", command, funcp);
			funcp->parent_cmd = command;
			command->child_func = funcp;

			debug_printf_exec("run_pipe: return EXIT_SUCCESS\n");
			debug_leave();
			પાછા EXIT_SUCCESS;
		}
#endif
		/* { list } */
		debug_printf_exec("non-subshell group\n");
		rcode = 1; /* exitcode if redir failed */
#ifndef __U_BOOT__
		જો (setup_redirects(command, &squirrel) == 0) {
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
		IF_HAS_KEYWORDS(જો (pi->pi_inverted) rcode = !rcode;)
		debug_leave();
		debug_printf_exec("run_pipe: return %d\n", rcode);
		પાછા rcode;
	}

	argv = command->argv ? command->argv : (અક્ષર **) &null_ptr;
	{
#ifndef __U_BOOT__
		અચલ સંરચના built_in_command *x;
		IF_HUSH_FUNCTIONS(અચલ સંરચના function *funcp;)
		IF_NOT_HUSH_FUNCTIONS(ગણના { funcp = 0 };)
		સંરચના variable **sv_shadowed;
#endif /* !__U_BOOT__ */
		સંરચના variable *old_vars;

#જો ENABLE_HUSH_LINENO_VAR
		G.execute_lineno = command->lineno;
#endif

		જો (argv[command->assignment_cnt] == ખાલી) {
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
			જ્યારે (i < command->assignment_cnt) {
				અક્ષર *p = expand_string_to_string(argv[i],
						EXP_FLAG_ESC_GLOB_CHARS,
						/*unbackslash:*/ 1
				);
#જો ENABLE_HUSH_MODE_X
				જો (G_x_mode) {
					અક્ષર *eq;
					જો (i == 0)
						x_mode_prefix();
					x_mode_addchr(' ');
					eq = strchrnul(p, '=');
					જો (*eq) eq++;
					x_mode_addblock(p, (eq - p));
					x_mode_print_optionally_squoted(eq);
					x_mode_flush();
				}
#endif
				debug_printf_env("set shell var:'%s'->'%s'\n", *argv, p);
#ifndef __U_BOOT__
				જો (set_local_var0(p)) {
#નહીંતર /* __U_BOOT__ */
				જો (set_local_var_modern(p, /*flag:*/ 0)) {
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
			જો (rcode == 0)
				rcode = G.expand_exitcode;
			IF_HAS_KEYWORDS(જો (pi->pi_inverted) rcode = !rcode;)
			debug_leave();
			debug_printf_exec("run_pipe: return %d\n", rcode);
			પાછા rcode;
		}

		/* Expand the rest into (possibly) many strings each */
#જો defined(CMD_TEST2_SINGLEWORD_NOGLOB)
		જો (command->cmd_type == CMD_TEST2_SINGLEWORD_NOGLOB)
			argv_expanded = expand_strvec_to_strvec_singleword_noglob(argv + command->assignment_cnt);
		નહીંતર
#endif
#જો defined(CMD_SINGLEWORD_NOGLOB)
		જો (command->cmd_type == CMD_SINGLEWORD_NOGLOB)
			argv_expanded = expand_strvec_to_strvec_singleword_noglob(argv + command->assignment_cnt);
		નહીંતર
#endif
			argv_expanded = expand_strvec_to_strvec(argv + command->assignment_cnt);

		/* If someone gives us an empty string: `cmd with empty output` */
		જો (!argv_expanded[0]) {
			મુક્ત(argv_expanded);
			/* `false` still has to set exitcode 1 */
			G.expand_exitcode = G.last_exitcode;
			જાઓ only_assignments;
		}

		old_vars = ખાલી;
#ifndef __U_BOOT__
		sv_shadowed = G.shadowed_vars_pp;

		/* Check if argv[0] matches any functions (this goes before bltins) */
		IF_HUSH_FUNCTIONS(funcp = find_function(argv_expanded[0]);)
		IF_HUSH_FUNCTIONS(x = ખાલી;)
		IF_HUSH_FUNCTIONS(જો (!funcp))
			x = find_builtin(argv_expanded[0]);
		જો (x || funcp) {
			જો (x && x->b_function == builtin_exec && argv_expanded[1] == ખાલી) {
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

				જાઓ clean_up_and_ret1;
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
			જો (rcode == 0) {
				જો (!funcp) {
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
#જો ENABLE_HUSH_FUNCTIONS
				નહીંતર {
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
		} નહીંતર
		જો (ENABLE_FEATURE_SH_NOFORK && અંક_APPLETS > 1) {
			પૂર્ણાંક n = find_applet_by_name(argv_expanded[0]);
			જો (n < 0 || !APPLET_IS_NOFORK(n))
				જાઓ must_fork;

			enter_var_nest_level();
			/* Collect all variables "shadowed" by helper into old_vars list */
			G.shadowed_vars_pp = &old_vars;
			rcode = redirect_and_varexp_helper(command, &squirrel, argv_expanded);
			G.shadowed_vars_pp = sv_shadowed;

			જો (rcode == 0) {
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
		} નહીંતર
			જાઓ must_fork;

		restore_redirects(squirrel);
 clean_up_and_ret1:
		leave_var_nest_level();
		add_vars(old_vars);

		/*
		 * Try "usleep 99999999" + ^C + "echo $?"
		 * with FEATURE_SH_NOFORK=y.
		 */
		જો (!funcp) {
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
			જો (sigismember(&G.pending_set, SIGINT))
				rcode = 128 | SIGINT;
		}
		મુક્ત(argv_expanded);
		IF_HAS_KEYWORDS(જો (pi->pi_inverted) rcode = !rcode;)
		debug_leave();
		debug_printf_exec("run_pipe return %d\n", rcode);
		પાછા rcode;
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
	જ્યારે (cmd_no < pi->અંક_cmds) {
#ifndef __U_BOOT__
		સંરચના fd_pair pipefds;
#જો !BB_MMU
		પૂર્ણાંક sv_var_nest_level = G.var_nest_level;
		અસ્થિર nommu_save_t nommu_save;
		nommu_save.old_vars = ખાલી;
		nommu_save.argv = ખાલી;
		nommu_save.argv_from_re_execing = ખાલી;
#endif
#endif /* !__U_BOOT__ */
		command = &pi->cmds[cmd_no];
		cmd_no++;

#ifdef __U_BOOT__
		/* Replace argv and argc by expanded if it exists. */
		જો (argv_expanded) {
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
			જો (command->argv) {
			debug_printf_exec(": pipe member '%s' '%s'...\n",
					command->argv[0], command->argv[1]);
		} નહીંતર {
			debug_printf_exec(": pipe member with no argv\n");
		}

#ifndef __U_BOOT__
		/* pipes are inserted between pairs of commands */
		pipefds.rd = 0;
		pipefds.wr = 1;
		જો (cmd_no < pi->અંક_cmds)
			xpiped_pair(pipefds);

#જો ENABLE_HUSH_LINENO_VAR
		G.execute_lineno = command->lineno;
#endif

		command->pid = BB_MMU ? fork() : vfork();
		જો (!command->pid) { /* child */
#જો ENABLE_HUSH_JOB
			disable_restore_tty_pgrp_on_exit();
			CLEAR_RANDOM_T(&G.random_gen); /* or else $RANDOM repeats in child */

			/* Every child adds itself to new process group
			 * with pgid == pid_of_first_child_in_pipe */
			જો (G.run_list_level == 1 && G_interactive_fd) {
				pid_t pgrp;
				pgrp = pi->pgrp;
				જો (pgrp < 0) /* true for 1st process only */
					pgrp = getpid();
				જો (setpgid(0, pgrp) == 0
				 && pi->followup != PIPE_BG
				 && G_saved_tty_pgrp /* we have ctty */
				) {
					/* We do it in *every* child, not just first,
					 * to avoid races */
					tcsetpgrp(G_interactive_fd, pgrp);
				}
			}
#endif
			જો (pi->alive_cmds == 0 && pi->followup == PIPE_BG) {
				/* 1st cmd in backgrounded pipe
				 * should have its stdin /dev/null'ed */
				close(0);
				જો (open(bb_dev_null, O_RDONLY))
					xopen("/", O_RDONLY);
			} નહીંતર {
				xmove_fd(next_infd, 0);
			}
			xmove_fd(pipefds.wr, 1);
			જો (pipefds.rd > 1)
				close(pipefds.rd);
			/* Like bash, explicit redirects override pipes,
			 * and the pipe fd (fd#1) is available for dup'ing:
			 * "cmd1 2>&1 | cmd2": fd#1 is duped to fd#2, thus stderr
			 * of cmd1 goes into pipe.
			 */
			જો (setup_redirects(command, ખાલી)) {
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
#જો ENABLE_HUSH_FAST
		G.ગણતરી_SIGCHLD++;
//bb_error_msg("[%d] fork in run_pipe: G.count_SIGCHLD:%d G.handled_SIGCHLD:%d", getpid(), G.count_SIGCHLD, G.handled_SIGCHLD);
#endif
		enable_restore_tty_pgrp_on_exit();
#જો !BB_MMU
		/* Clean up after vforked child */
		મુક્ત(nommu_save.argv);
		મુક્ત(nommu_save.argv_from_re_execing);
		G.var_nest_level = sv_var_nest_level;
		remove_nested_vars();
		add_vars(nommu_save.old_vars);
#endif
		મુક્ત(argv_expanded);
		argv_expanded = ખાલી;
		જો (command->pid < 0) { /* [v]fork failed */
			/* Clearly indicate, was it fork or vfork */
			bb_simple_perror_સંદેશ(BB_MMU ? "vfork"+1 : "vfork");
		} નહીંતર {
			pi->alive_cmds++;
#જો ENABLE_HUSH_JOB
			/* Second and next children need to know pid of first one */
			જો (pi->pgrp < 0)
				pi->pgrp = command->pid;
#endif
		}

		જો (cmd_no > 1)
			close(next_infd);
		જો (cmd_no < pi->અંક_cmds)
			close(pipefds.wr);
		/* Pass read (output) pipe end to next iteration */
		next_infd = pipefds.rd;
#નહીંતર /* __U_BOOT__ */
		/* Process the command */
		rcode = cmd_process(G.do_repeat ? CMD_FLAG_REPEAT : 0,
				    command->argc, command->argv,
				    &(G.flag_repeat), ખાલી);

		જો (argv_expanded) {
			/*
			 * expand_strvec_to_strvec() allocates memory to expand
			 * argv, we need to free it.
			 */
			મુક્ત(argv_expanded);

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
	જો (!pi->alive_cmds) {
		debug_leave();
		debug_printf_exec("run_pipe return 1 (all forks failed, no children)\n");
		પાછા 1;
	}
#endif /* __U_BOOT__ */

	debug_leave();
#ifndef __U_BOOT__
	debug_printf_exec("run_pipe return -1 (%u children started)\n", pi->alive_cmds);
	પાછા -1;
#નહીંતર /* __U_BOOT__ */
	IF_HAS_KEYWORDS(જો (pi->pi_inverted) rcode = !rcode;)
	debug_printf_exec("run_pipe return %d\n", rcode);
	પાછા rcode;
#endif /* __U_BOOT__ */
}

/* NB: called by pseudo_exec, and therefore must not modify any
 * global data until exec/_exit (we can be a child after vfork!) */
સ્થિર પૂર્ણાંક run_list(સંરચના pipe *pi)
{
#જો ENABLE_HUSH_CASE
	અક્ષર *case_word = ખાલી;
#endif
#જો ENABLE_HUSH_LOOPS
	સંરચના pipe *loop_ટોચ = ખાલી;
	અક્ષર **for_lcur = ખાલી;
	અક્ષર **for_list = ખાલી;
#endif
	smallint last_followup;
	smalluint rcode;
#જો ENABLE_HUSH_IF || ENABLE_HUSH_CASE
	smalluint cond_code = 0;
#નહીંતર
	ગણના { cond_code = 0 };
#endif
#જો HAS_KEYWORDS
	smallint rword;      /* RES_foo */
	smallint last_rword; /* ditto */
#endif

#ifndef __U_BOOT__
	debug_printf_exec("run_list lvl %d start\n", G.run_list_level);
	debug_enter();
#endif /* !__U_BOOT__ */

	set_G_ifs();

#જો ENABLE_HUSH_LOOPS
	/* Check syntax for "for" */
	{
		સંરચના pipe *cpipe;
		માટે (cpipe = pi; cpipe; cpipe = cpipe->next) {
			જો (cpipe->res_word != RES_FOR && cpipe->res_word != RES_IN)
				ચાલુ;
			/* current word is FOR or IN (BOLD in comments below) */
			જો (cpipe->next == ખાલી) {
				syntax_error("malformed for");
				debug_leave();
				debug_printf_exec("run_list lvl %d return 1\n", G.run_list_level);
				પાછા 1;
			}
			/* "FOR v; do ..." and "for v IN a b; do..." are ok */
			જો (cpipe->next->res_word == RES_DO)
				ચાલુ;
			/* next word is not "do". It must be "in" then ("FOR v in ...") */
			જો (cpipe->res_word == RES_IN /* "for v IN a b; not_do..."? */
			 || cpipe->next->res_word != RES_IN /* FOR v not_do_and_not_in..."? */
			) {
				syntax_error("malformed for");
				debug_leave();
				debug_printf_exec("run_list lvl %d return 1\n", G.run_list_level);
				પાછા 1;
			}
		}
	}
#endif

	/* Past this point, all code paths should jump to ret: label
	 * in order to return, no direct "return" statements please.
	 * This helps to ensure that no memory is leaked. */

#જો ENABLE_HUSH_JOB
	G.run_list_level++;
#endif

#જો HAS_KEYWORDS
	rword = RES_NONE;
	last_rword = RES_XXXX;
#endif
	last_followup = PIPE_SEQ;
	rcode = G.last_exitcode;

	/* Go through list of pipes, (maybe) executing them. */
#ifndef __U_BOOT__
	માટે (; pi; pi = IF_HUSH_LOOPS(rword == RES_DONE ? loop_ટોચ : ) pi->next) {
#નહીંતર /* __U_BOOT__ */
	માટે (; pi; pi = rword == RES_DONE ? loop_ટોચ : pi->next) {
#endif /* __U_BOOT__ */
		પૂર્ણાંક r;
		પૂર્ણાંક sv_errexit_depth;

#ifndef __U_BOOT__
		જો (G.flag_SIGINT)
			તોડો;
		જો (G_flag_return_in_progress == 1)
			તોડો;
#endif /* !__U_BOOT__ */

		IF_HAS_KEYWORDS(rword = pi->res_word;)
		debug_printf_exec(": rword:%d cond_code:%d last_rword:%d\n",
				rword, cond_code, last_rword);

		sv_errexit_depth = G.errexit_depth;
		જો (
#જો ENABLE_HUSH_IF
		    rword == RES_IF || rword == RES_ELIF ||
#endif
		    pi->followup != PIPE_SEQ
		) {
			G.errexit_depth++;
		}
#જો ENABLE_HUSH_LOOPS
		જો ((rword == RES_WHILE || rword == RES_UNTIL || rword == RES_FOR)
		 && loop_ટોચ == ખાલી /* avoid bumping G.depth_of_loop twice */
		) {
			/* start of a loop: remember where loop starts */
			loop_ટોચ = pi;
			G.depth_of_loop++;
		}
#endif
		/* Still in the same "if...", "then..." or "do..." branch? */
		જો (IF_HAS_KEYWORDS(rword == last_rword &&) 1) {
			જો ((rcode == 0 && last_followup == PIPE_OR)
			 || (rcode != 0 && last_followup == PIPE_AND)
			) {
				/* It is "<true> || CMD" or "<false> && CMD"
				 * and we should not execute CMD */
				debug_printf_exec("skipped cmd because of || or &&\n");
				last_followup = pi->followup;
				જાઓ dont_check_jobs_but_continue;
			}
		}
		last_followup = pi->followup;
#જો ENABLE_HUSH_IF
		જો (cond_code != 0) {
			જો (rword == RES_THEN) {
				/* if false; then ... fi has exitcode 0! */
				G.last_exitcode = rcode = EXIT_SUCCESS;
				/* "if <false> THEN cmd": skip cmd */
				debug_printf_exec("skipped THEN cmd because IF condition was false\n");
				last_rword = rword;
				ચાલુ;
			}
		} નહીંતર {
			જો (rword == RES_ELSE
			 || (rword == RES_ELIF && last_rword != RES_ELIF)
			) {
				/* "if <true> then ... ELSE/ELIF cmd":
				 * skip cmd and all following ones */
				debug_printf_exec("skipped ELSE/ELIF branch because IF condition was true\n");
				તોડો;
			}
			//if (rword == RES_THEN): "if <true> THEN cmd", run cmd (fall through)
		}
#endif
		IF_HAS_KEYWORDS(last_rword = rword;)
#જો ENABLE_HUSH_LOOPS
		જો (rword == RES_FOR) { /* && pi->num_cmds - always == 1 */
			જો (!for_lcur) {
				/* first loop through for */

				સ્થિર અચલ અક્ષર encoded_dollar_at[] ALIGN1 = {
					SPECIAL_VAR_પ્રતીક, '@' | 0x80, SPECIAL_VAR_પ્રતીક, '\0'
				}; /* encoded representation of "$@" */
				સ્થિર અચલ અક્ષર *અચલ encoded_dollar_at_argv[] ALIGN_PTR = {
					encoded_dollar_at, ખાલી
				}; /* argv list with one element: "$@" */
				અક્ષર **vals;

				G.last_exitcode = rcode = EXIT_SUCCESS;
				vals = (અક્ષર**)encoded_dollar_at_argv;
				જો (pi->next->res_word == RES_IN) {
					/* if no variable values after "in" we skip "for" */
					જો (!pi->next->cmds[0].argv) {
						debug_printf_exec(": null FOR: exitcode EXIT_SUCCESS\n");
						તોડો;
					}
					vals = pi->next->cmds[0].argv;
				} /* else: "for var; do..." -> assume "$@" list */
				/* create list of variable values */
				debug_print_strings("for_list made from", vals);
				for_list = expand_strvec_to_strvec(vals);
				for_lcur = for_list;
				debug_print_strings("for_list", for_list);
			}
			જો (!*for_lcur) {
				/* "for" loop is over, clean up */
				મુક્ત(for_list);
				for_list = ખાલી;
				for_lcur = ખાલી;
				તોડો;
			}
			/* Insert next value from for_lcur */
			/* note: *for_lcur already has quotes removed, $var expanded, etc */
#ifndef __U_BOOT__
			set_local_var_from_halves(pi->cmds[0].argv[0], *for_lcur++);
#નહીંતર /* __U_BOOT__ */
			/* We cannot use xasprintf, so we emulate it. */
			અક્ષર *full_var;
			અક્ષર *var = pi->cmds[0].argv[0];
			અક્ષર *val = *for_lcur++;

			/* + 1 to take into account =. */
			full_var = xmalloc(strlen(var) + strlen(val) + 1);
			sprintf(full_var, "%s=%s", var, val);

			set_local_var_modern(full_var, /*flag:*/ 0);
#endif /* __U_BOOT__ */
			ચાલુ;
		}
		જો (rword == RES_IN) {
			ચાલુ; /* "for v IN list;..." - "in" has no cmds anyway */
		}
		જો (rword == RES_DONE) {
			ચાલુ; /* "done" has no cmds too */
		}
#endif
#જો ENABLE_HUSH_CASE
		જો (rword == RES_CASE) {
			debug_printf_exec("CASE cond_code:%d\n", cond_code);
			case_word = expand_string_to_string(pi->cmds->argv[0],
				EXP_FLAG_ESC_GLOB_CHARS, /*unbackslash:*/ 1);
			debug_printf_exec("CASE word1:'%s'\n", case_word);
			//unbackslash(case_word);
			//debug_printf_exec("CASE word2:'%s'\n", case_word);
			ચાલુ;
		}
		જો (rword == RES_MATCH) {
			અક્ષર **argv;

			debug_printf_exec("MATCH cond_code:%d\n", cond_code);
			જો (!case_word) /* "case ... matched_word) ... WORD)": we executed selected branch, stop */
				તોડો;
			/* all prev words didn't match, does this one match? */
			argv = pi->cmds->argv;
			જ્યારે (*argv) {
				અક્ષર *pattern;
				debug_printf_exec("expand_string_to_string('%s')\n", *argv);
				pattern = expand_string_to_string(*argv,
						EXP_FLAG_ESC_GLOB_CHARS,
						/*unbackslash:*/ 0
				);
				/* TODO: which FNM_xxx flags to use? */
				cond_code = (fnmatch(pattern, case_word, /*flags:*/ 0) != 0);
				debug_printf_exec("cond_code=fnmatch(pattern:'%s',str:'%s'):%d\n",
						pattern, case_word, cond_code);
				મુક્ત(pattern);
				જો (cond_code == 0) {
					/* match! we will execute this branch */
					મુક્ત(case_word);
					case_word = ખાલી; /* make future "word)" stop */
					તોડો;
				}
				argv++;
			}
			ચાલુ;
		}
		જો (rword == RES_CASE_BODY) { /* inside of a case branch */
			debug_printf_exec("CASE_BODY cond_code:%d\n", cond_code);
			જો (cond_code != 0)
				ચાલુ; /* not matched yet, skip this pipe */
		}
		જો (rword == RES_ESAC) {
			debug_printf_exec("ESAC cond_code:%d\n", cond_code);
			જો (case_word) {
				/* "case" did not match anything: still set $? (to 0) */
				G.last_exitcode = rcode = EXIT_SUCCESS;
			}
		}
#endif
		/* Just pressing <enter> in shell should check for jobs.
		 * OTOH, in non-interactive shell this is useless
		 * and only leads to extra job checks */
		જો (pi->અંક_cmds == 0) {
#ifndef __U_BOOT__
			જો (G_interactive_fd)
				જાઓ check_jobs_and_continue;
#endif /* !__U_BOOT__ */
			ચાલુ;
		}

		/* After analyzing all keywords and conditions, we decided
		 * to execute this pipe. NB: have to do checkjobs(NULL)
		 * after run_pipe to collect any background children,
		 * even if list execution is to be stopped. */
		debug_printf_exec(": run_pipe with %d members\n", pi->અંક_cmds);
#ifndef __U_BOOT__
#જો ENABLE_HUSH_LOOPS
		G.flag_break_continue = 0;
#endif
#endif /* !__U_BOOT__ */
#ifndef __U_BOOT__
		rcode = r = G.o_opt[OPT_O_NOEXEC] ? 0 : run_pipe(pi);
		/* NB: rcode is a smalluint, r is int */
#નહીંતર /* __U_BOOT__ */
		rcode = r = run_pipe(pi); /* NB: rcode is a smalluint, r is int */
		જો (r <= EXIT_RET_CODE) {
			પૂર્ણાંક previous_rcode = G.last_exitcode;
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
			જો (previous_rcode != r) {
				/*
				 * If run from run_command, run_command_flags will be set, so we check
				 * this to know if we are in main input shell.
				 */
				જો (!G.run_command_flags)
					printf("exit not allowed from main input shell.\n");

				તોડો;
			}
			ચાલુ;
		}
#endif /* __U_BOOT__ */
		જો (r != -1) {
			/* We ran a builtin, function, or group.
			 * rcode is already known
			 * and we don't need to wait for anything. */
			debug_printf_exec(": builtin/func exitcode %d\n", rcode);
			G.last_exitcode = rcode;
#ifndef __U_BOOT__
			check_and_run_traps();
#endif /* !__U_BOOT__ */
#જો ENABLE_HUSH_TRAP && ENABLE_HUSH_FUNCTIONS
			rcode = G.last_exitcode; /* "return" in trap can change it, read back */
#endif
#ifndef __U_BOOT__
#જો ENABLE_HUSH_LOOPS
			/* Was it "break" or "continue"? */
			જો (G.flag_break_continue) {
				smallint fbc = G.flag_break_continue;
				/* We might fall into outer *loop*,
				 * don't want to break it too */
				જો (loop_ટોચ) {
					G.depth_break_continue--;
					જો (G.depth_break_continue == 0)
						G.flag_break_continue = 0;
					/* else: e.g. "continue 2" should *break* once, *then* continue */
				} /* else: "while... do... { we are here (innermost list is not a loop!) };...done" */
				જો (G.depth_break_continue != 0 || fbc == BC_BREAK) {
					checkjobs(ખાલી, 0 /*(no pid to wait for)*/);
					તોડો;
				}
				/* "continue": simulate end of loop */
				rword = RES_DONE;
				ચાલુ;
			}
#endif
			જો (G_flag_return_in_progress == 1) {
				checkjobs(ખાલી, 0 /*(no pid to wait for)*/);
				તોડો;
			}

		} નહીંતર જો (pi->followup == PIPE_BG) {
			/* What does bash do with attempts to background builtins? */
			/* even bash 3.2 doesn't do that well with nested bg:
			 * try "{ { sleep 10; echo DEEP; } & echo HERE; } &".
			 * I'm NOT treating inner &'s as jobs */
#જો ENABLE_HUSH_JOB
			જો (G.run_list_level == 1)
				ઉમેરો_job_into_table(pi);
#endif
			/* Last command's pid goes to $! */
			G.last_bg_pid = pi->cmds[pi->અંક_cmds - 1].pid;
			G.last_bg_pid_exitcode = 0;
			debug_printf_exec(": cmd&: exitcode EXIT_SUCCESS\n");
/* Check pi->pi_inverted? "! sleep 1 & echo $?": bash says 1. dash and ash say 0 */
			rcode = EXIT_SUCCESS;
			જાઓ check_traps;
		} નહીંતર {
#જો ENABLE_HUSH_JOB
			જો (G.run_list_level == 1 && G_interactive_fd) {
				/* Waits for completion, then fg's main shell */
				rcode = checkjobs_and_fg_shell(pi);
				debug_printf_exec(": checkjobs_and_fg_shell exitcode %d\n", rcode);
				જાઓ check_traps;
			}
#endif
			/* This one just waits for completion */
			rcode = checkjobs(pi, 0 /*(no pid to wait for)*/);
			debug_printf_exec(": checkjobs exitcode %d\n", rcode);
 check_traps:
			G.last_exitcode = rcode;
			check_and_run_traps();
#જો ENABLE_HUSH_TRAP && ENABLE_HUSH_FUNCTIONS
			rcode = G.last_exitcode; /* "return" in trap can change it, read back */
#endif
		}
#endif /* !__U_BOOT__ */

#ifndef __U_BOOT__
		/* Handle "set -e" */
		જો (rcode != 0 && G.o_opt[OPT_O_ERREXIT]) {
			debug_printf_exec("ERREXIT:1 errexit_depth:%d\n", G.errexit_depth);
			જો (G.errexit_depth == 0)
				hush_exit(rcode);
		}
#નહીંતર /* __U_BOOT__ */
		} /* if (r != -1) */
#endif /* __U_BOOT__ */
		G.errexit_depth = sv_errexit_depth;

		/* Analyze how result affects subsequent commands */
#જો ENABLE_HUSH_IF
		જો (rword == RES_IF || rword == RES_ELIF) {
			debug_printf_exec("cond_code=rcode:%d\n", rcode);
			cond_code = rcode;
		}
#endif
#ifndef __U_BOOT__
 check_jobs_and_continue:
		checkjobs(ખાલી, 0 /*(no pid to wait for)*/);
#endif /* !__U_BOOT__ */
 dont_check_jobs_but_continue: ;
#જો ENABLE_HUSH_LOOPS
		/* Beware of "while false; true; do ..."! */
		જો (pi->next
		 && (pi->next->res_word == RES_DO || pi->next->res_word == RES_DONE)
		 /* check for RES_DONE is needed for "while ...; do \n done" case */
		) {
			જો (rword == RES_WHILE) {
				જો (rcode) {
					/* "while false; do...done" - exitcode 0 */
					G.last_exitcode = rcode = EXIT_SUCCESS;
					debug_printf_exec(": while expr is false: breaking (exitcode:EXIT_SUCCESS)\n");
					તોડો;
				}
			}
			જો (rword == RES_UNTIL) {
				જો (!rcode) {
					debug_printf_exec(": until expr is true: breaking\n");
					તોડો;
				}
			}
		}
#endif
	} /* for (pi) */

#જો ENABLE_HUSH_JOB
	G.run_list_level--;
#endif
#જો ENABLE_HUSH_LOOPS
	જો (loop_ટોચ)
		G.depth_of_loop--;
	મુક્ત(for_list);
#endif
#જો ENABLE_HUSH_CASE
	મુક્ત(case_word);
#endif
#ifndef __U_BOOT__
	debug_leave();
	debug_printf_exec("run_list lvl %d return %d\n", G.run_list_level, rcode);
#endif /* !__U_BOOT__ */
	પાછા rcode;
}

/* Select which version we will use */
સ્થિર પૂર્ણાંક run_and_free_list(સંરચના pipe *pi)
{
	પૂર્ણાંક rcode = 0;
	debug_printf_exec("run_and_free_list entered\n");
#ifndef __U_BOOT__
	જો (!G.o_opt[OPT_O_NOEXEC]) {
#endif /* !__U_BOOT__ */
		debug_printf_exec(": run_list: 1st pipe with %d cmds\n", pi->અંક_cmds);
		rcode = run_list(pi);
#ifndef __U_BOOT__
	}
#endif /* !__U_BOOT__ */
	/* free_pipe_list has the side effect of clearing memory.
	 * In the long run that function can be merged with run_list,
	 * but doing that now would hobble the debugging effort. */
	free_pipe_list(pi);
	debug_printf_exec("run_and_free_list return %d\n", rcode);
	પાછા rcode;
}

#ifndef __U_BOOT__
સ્થિર શૂન્ય install_sighandlers(unsigned mask)
{
	sighandler_t old_handler;
	unsigned sig = 0;
	જ્યારે ((mask >>= 1) != 0) {
		sig++;
		જો (!(mask & 1))
			ચાલુ;
		old_handler = install_sighandler(sig, pick_sighandler(sig));
		/* POSIX allows shell to re-enable SIGCHLD
		 * even if it was SIG_IGN on entry.
		 * Therefore we skip IGN check for it:
		 */
		જો (sig == SIGCHLD)
			ચાલુ;
		/* Interactive bash re-enables SIGHUP which is SIG_IGNed on entry.
		 * Try:
		 * trap '' hup; bash; echo RET  # type "kill -hup $$", see SIGHUP having effect
		 * trap '' hup; bash -c 'kill -hup $$; echo ALIVE'  # here SIGHUP is SIG_IGNed
		 */
		જો (sig == SIGHUP && G_interactive_fd)
			ચાલુ;
		/* Unless one of the above signals, is it SIG_IGN? */
		જો (old_handler == SIG_IGN) {
			/* oops... restore back to IGN, and record this fact */
			install_sighandler(sig, old_handler);
#જો ENABLE_HUSH_TRAP
			જો (!G_traps)
				G_traps = xzalloc(કદ(G_traps[0]) * NSIG);
			મુક્ત(G_traps[sig]);
			G_traps[sig] = xzalloc(1); /* == xstrdup(""); */
#endif
		}
	}
}

/* Called a few times only (or even once if "sh -c") */
સ્થિર શૂન્ય install_special_sighandlers(શૂન્ય)
{
	unsigned mask;

	/* Which signals are shell-special? */
	mask = (1 << SIGQUIT) | (1 << SIGCHLD);
	જો (G_interactive_fd) {
		mask |= SPECIAL_INTERACTIVE_SIGS;
		જો (G_saved_tty_pgrp) /* we have ctty, job control sigs work */
			mask |= SPECIAL_JOBSTOP_SIGS;
	}
	/* Careful, do not re-install handlers we already installed */
	જો (G.special_sig_mask != mask) {
		unsigned diff = mask & ~G.special_sig_mask;
		G.special_sig_mask = mask;
		install_sighandlers(diff);
	}
}

#જો ENABLE_HUSH_JOB
/* helper */
/* Set handlers to restore tty pgrp and exit */
સ્થિર શૂન્ય install_fatal_sighandlers(શૂન્ય)
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

સ્થિર પૂર્ણાંક set_mode(પૂર્ણાંક state, અક્ષર mode, અચલ અક્ષર *o_opt)
{
	પૂર્ણાંક idx;
	પસંદગી (mode) {
	સ્થિતિ 'n':
		/* set -n has no effect in interactive shell */
		/* Try: while set -n; do echo $-; done */
		જો (!G_interactive_fd)
			G.o_opt[OPT_O_NOEXEC] = state;
		તોડો;
	સ્થિતિ 'x':
		IF_HUSH_MODE_X(G_x_mode = state;)
		IF_HUSH_MODE_X(જો (G.x_mode_fd <= 0) G.x_mode_fd = dup_CLOEXEC(2, 10);)
		તોડો;
	સ્થિતિ 'e':
		G.o_opt[OPT_O_ERREXIT] = state;
		તોડો;
	સ્થિતિ 'o':
		જો (!o_opt) {
			/* "set -o" or "set +o" without parameter.
			 * in bash, set -o produces this output:
			 *  pipefail        off
			 * and set +o:
			 *  set +o pipefail
			 * We always use the second form.
			 */
			અચલ અક્ષર *p = o_opt_strings;
			idx = 0;
			જ્યારે (*p) {
				printf("set %co %s\n", (G.o_opt[idx] ? '-' : '+'), p);
				idx++;
				p += strlen(p) + 1;
			}
			તોડો;
		}
		idx = index_in_strings(o_opt_strings, o_opt);
		જો (idx >= 0) {
			G.o_opt[idx] = state;
			તોડો;
		}
		/* fall through to error */
	મૂળભૂત:
		પાછા EXIT_FAILURE;
	}
	પાછા EXIT_SUCCESS;
}

પૂર્ણાંક hush_main(પૂર્ણાંક argc, અક્ષર **argv) MAIN_EXTERNALLY_VISIBLE;
પૂર્ણાંક hush_main(પૂર્ણાંક argc, અક્ષર **argv)
{
	pid_t cached_getpid;
	ગણના {
		OPT_login = (1 << 0),
	};
	unsigned flags;
#જો !BB_MMU
	unsigned builtin_argc = 0;
#endif
	અક્ષર **e;
	સંરચના variable *cur_var;
	સંરચના variable *shell_ver;

	INIT_G();
	જો (EXIT_SUCCESS != 0) /* if EXIT_SUCCESS == 0, it is already done */
		G.last_exitcode = EXIT_SUCCESS;
#જો !BB_MMU
	/* "Big heredoc" support via "sh -< STRING" invocation.
	 * Check it first (do not bother to run the usual init code,
	 * it is not needed for this case).
	 */
	જો (argv[1]
	 && argv[1][0] == '-' && argv[1][1] == '<' /*&& !argv[1][2]*/
	 /*&& argv[2] && !argv[3] - we don't check some conditions */
	) {
		full_write1_str(argv[2]);
		_exit(0);
	}
	G.argv0_for_re_execing = argv[0];
#endif
#જો ENABLE_HUSH_TRAP
# જો ENABLE_HUSH_FUNCTIONS
	G.return_exitcode = -1;
# endif
	G.pre_trap_exitcode = -1;
#endif

#જો ENABLE_HUSH_FAST
	G.ગણતરી_SIGCHLD++; /* ensure it is != G.handled_SIGCHLD */
#endif

	cached_getpid = getpid();   /* for tcsetpgrp() during init */
	G.મૂળ_pid = cached_getpid; /* for $PID  (NOMMU can override via -$HEXPID:HEXPPID:...) */
	G.મૂળ_ppid = getppid();    /* for $PPID (NOMMU can override) */

	/* Deal with HUSH_VERSION */
	debug_printf_env("unsetenv '%s'\n", "HUSH_VERSION");
	unsetenv("HUSH_VERSION"); /* in case it exists in initial env */
	shell_ver = xzalloc(કદ(*shell_ver));
	shell_ver->flg_export = 1;
	shell_ver->flg_read_only = 1;
	/* Code which handles ${var<op>...} needs writable values for all variables,
	 * therefore we xstrdup: */
	shell_ver->varstr = xstrdup(hush_version_str);

	/* Create shell local variables from the values
	 * currently living in the environment */
	G.ટોચ_var = shell_ver;
	cur_var = G.ટોચ_var;
	e = environ;
	જો (e) જ્યારે (*e) {
		અક્ષર *કિંમત = strchr(*e, '=');
		જો (કિંમત) { /* paranoia */
			cur_var->next = xzalloc(કદ(*cur_var));
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

#જો BASH_HOSTNAME_VAR
	/* Set (but not export) HOSTNAME unless already set */
	જો (!get_local_var_કિંમત("HOSTNAME")) {
		સંરચના utsname uts;
		uname(&uts);
		set_local_var_from_halves("HOSTNAME", uts.nodename);
	}
#endif
	/* IFS is not inherited from the parent environment */
	set_local_var_from_halves("IFS", defifs);

	જો (!get_local_var_કિંમત("PATH"))
		set_local_var_from_halves("PATH", bb_default_મૂળ_path);

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

#જો અંક_SCRIPTS > 0
	જો (argc < 0) {
		અક્ષર *script = get_script_content(-argc - 1);
		G.global_argv = argv;
		G.global_argc = string_array_len(argv);
		//install_special_sighandlers(); - needed?
		parse_and_run_string(script);
		જાઓ final_return;
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
	જ્યારે (1) {
		પૂર્ણાંક opt = getopt(argc, argv, "+" /* stop at 1st non-option */
				"cexinsl"
#જો !BB_MMU
				"$:R:V:"
# જો ENABLE_HUSH_LINENO_VAR
				"L:"
# endif
# જો ENABLE_HUSH_FUNCTIONS
				"F:"
# endif
#endif
		);
		જો (opt <= 0)
			તોડો;
		પસંદગી (opt) {
		સ્થિતિ 'c':
			/* Note: -c is not an option with param!
			 * "hush -c -l SCRIPT" is valid. "hush -cSCRIPT" is not.
			 */
			G.opt_c = 1;
			તોડો;
		સ્થિતિ 'i':
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
			તોડો;
		સ્થિતિ 's':
			G.opt_s = 1;
			તોડો;
		સ્થિતિ 'l':
			flags |= OPT_login;
			તોડો;
#જો !BB_MMU
		સ્થિતિ '$': {
			unsigned long long empty_trap_mask;

			G.મૂળ_pid = bb_strtou(optarg, &optarg, 16);
			optarg++;
			G.મૂળ_ppid = bb_strtou(optarg, &optarg, 16);
			optarg++;
			G.last_bg_pid = bb_strtou(optarg, &optarg, 16);
			optarg++;
			G.last_exitcode = bb_strtou(optarg, &optarg, 16);
			optarg++;
			builtin_argc = bb_strtou(optarg, &optarg, 16);
			optarg++;
			empty_trap_mask = bb_strtoull(optarg, &optarg, 16);
			જો (empty_trap_mask != 0) {
				IF_HUSH_TRAP(પૂર્ણાંક sig;)
				install_special_sighandlers();
# જો ENABLE_HUSH_TRAP
				G_traps = xzalloc(કદ(G_traps[0]) * NSIG);
				માટે (sig = 1; sig < NSIG; sig++) {
					જો (empty_trap_mask & (1LL << sig)) {
						G_traps[sig] = xzalloc(1); /* == xstrdup(""); */
						install_sighandler(sig, SIG_IGN);
					}
				}
# endif
			}
# જો ENABLE_HUSH_LOOPS
			optarg++;
			G.depth_of_loop = bb_strtou(optarg, &optarg, 16);
# endif
			/* Suppress "killed by signal" message, -$ hack is used
			 * for subshells: echo `sh -c 'kill -9 $$'`
			 * should be silent.
			 */
			IF_HUSH_JOB(G.run_list_level = 1;)
# જો ENABLE_HUSH_FUNCTIONS
			/* nommu uses re-exec trick for "... | func | ...",
			 * should allow "return".
			 * This accidentally allows returns in subshells.
			 */
			G_flag_return_in_progress = -1;
# endif
			તોડો;
		}
		સ્થિતિ 'R':
		સ્થિતિ 'V':
			set_local_var(xstrdup(optarg), opt == 'R' ? SETFLAG_MAKE_RO : 0);
			તોડો;
# જો ENABLE_HUSH_LINENO_VAR
		સ્થિતિ 'L':
			G.parse_lineno = xatou(optarg);
			તોડો;
# endif
# જો ENABLE_HUSH_FUNCTIONS
		સ્થિતિ 'F': {
			સંરચના function *funcp = new_function(optarg);
			/* funcp->name is already set to optarg */
			/* funcp->body is set to NULL. It's a special case. */
			funcp->body_as_string = argv[optind];
			optind++;
			તોડો;
		}
# endif
#endif
		/*case '?': invalid option encountered (set_mode('?') will fail) */
		/*case 'n':*/
		/*case 'x':*/
		/*case 'e':*/
		મૂળભૂત:
			જો (set_mode(1, opt, ખાલી) == 0) /* no error */
				તોડો;
			bb_show_usage();
		}
	} /* option parsing loop */

	/* Skip options. Try "hush -l": $1 should not be "-l"! */
	G.global_argc = argc - (optind - 1);
	G.global_argv = argv + (optind - 1);
	G.global_argv[0] = argv[0];

	/* If we are login shell... */
	જો (flags & OPT_login) {
		અચલ અક્ષર *hp = ખાલી;
		HFILE *આગત;

		debug_printf("sourcing /etc/profile\n");
		આગત = hfopen("/etc/profile");
 run_profile:
		જો (આગત != ખાલી) {
			install_special_sighandlers();
			parse_and_run_file(આગત);
			hfclose(આગત);
		}
		/* bash: after sourcing /etc/profile,
		 * tries to source (in the given order):
		 * ~/.bash_profile, ~/.bash_login, ~/.profile,
		 * stopping on first found. --noprofile turns this off.
		 * bash also sources ~/.bash_logout on exit.
		 * If called as sh, skips .bash_XXX files.
		 */
		જો (!hp) { /* unless we looped on the "goto" already */
			hp = get_local_var_કિંમત("HOME");
			જો (hp && hp[0]) {
				debug_printf("sourcing ~/.profile\n");
				hp = concat_path_file(hp, ".profile");
				આગત = hfopen(hp);
				મુક્ત((અક્ષર*)hp);
				જાઓ run_profile;
			}
		}
	}

#ifndef __U_BOOT__
	/* -c takes effect *after* -l */
	જો (G.opt_c) {
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
		અક્ષર *script;

		install_special_sighandlers();

		G.global_argc--;
		G.global_argv++;
#જો !BB_MMU
		જો (builtin_argc) {
			/* -c 'builtin' [BARGV...] "" ARG0 [ARG1...] */
			અચલ સંરચના built_in_command *x;
			x = find_builtin(G.global_argv[0]);
			જો (x) { /* paranoia */
				argv = G.global_argv;
				G.global_argc -= builtin_argc + 1; /* skip [BARGV...] "" */
				G.global_argv += builtin_argc + 1;
				G.global_argv[-1] = ખાલી; /* replace "" */
				G.last_exitcode = x->b_function(argv);
			}
			જાઓ final_return;
		}
#endif

		script = G.global_argv[0];
		જો (!script)
			bb_error_સંદેશ_and_die(bb_સંદેશ_requires_arg, "-c");
		જો (!G.global_argv[1]) {
			/* -c 'script' (no params): prevent empty $0 */
			G.global_argv[0] = argv[0];
		} નહીંતર { /* else -c 'script' ARG0 [ARG1...]: $0 is ARG0 */
			G.global_argc--;
			G.global_argv++;
		}
		parse_and_run_string(script);
		જાઓ final_return;
	}

	/* -s is: hush -s ARGV1 ARGV2 (no SCRIPT) */
	જો (!G.opt_s && G.global_argv[1]) {
		HFILE *આગત;
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
		આગત = hfopen(G.global_argv[0]);
		જો (!આગત) {
			bb_simple_perror_સંદેશ_and_die(G.global_argv[0]);
		}
		xfunc_error_retval = 1;
		install_special_sighandlers();
		parse_and_run_file(આગત);
#જો ENABLE_FEATURE_CLEAN_UP
		hfclose(આગત);
#endif
		જાઓ final_return;
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
#જો ENABLE_HUSH_JOB
	જો (isatty(STDIN_FILENO) && isatty(STDOUT_FILENO)) {
		G_saved_tty_pgrp = tcgetpgrp(STDIN_FILENO);
		debug_printf("saved_tty_pgrp:%d\n", G_saved_tty_pgrp);
		જો (G_saved_tty_pgrp < 0)
			G_saved_tty_pgrp = 0;

		/* try to dup stdin to high fd#, >= 255 */
		G_interactive_fd = dup_CLOEXEC(STDIN_FILENO, 254);
		જો (G_interactive_fd < 0) {
			/* try to dup to any fd */
			G_interactive_fd = dup_CLOEXEC(STDIN_FILENO, -1);
			જો (G_interactive_fd < 0) {
				/* give up */
				G_interactive_fd = 0;
				G_saved_tty_pgrp = 0;
			}
		}
	}
	debug_printf("interactive_fd:%d\n", G_interactive_fd);
	જો (G_interactive_fd) {
		જો (G_saved_tty_pgrp) {
			/* If we were run as 'hush &', sleep until we are
			 * in the foreground (tty pgrp == our pgrp).
			 * If we get started under a job aware app (like bash),
			 * make sure we are now in charge so we don't fight over
			 * who gets the foreground */
			જ્યારે (1) {
				pid_t shell_pgrp = getpgrp();
				G_saved_tty_pgrp = tcgetpgrp(G_interactive_fd);
				જો (G_saved_tty_pgrp == shell_pgrp)
					તોડો;
				/* send TTIN to ourself (should stop us) */
				kill(- shell_pgrp, SIGTTIN);
			}
		}

		/* Install more signal handlers */
		install_special_sighandlers();

		જો (G_saved_tty_pgrp) {
			/* Set other signals to restore saved_tty_pgrp */
			install_fatal_sighandlers();
			/* Put ourselves in our own process group
			 * (bash, too, does this only if ctty is available) */
			bb_setpgrp(); /* is the same as setpgid(our_pid, our_pid); */
			/* Grab control of the terminal */
			tcsetpgrp(G_interactive_fd, cached_getpid);
		}
		enable_restore_tty_pgrp_on_exit();

# જો ENABLE_FEATURE_EDITING
		G.લીટી_આગત_state = new_લીટી_આગત_t(FOR_SHELL);
#  જો ENABLE_FEATURE_TAB_COMPLETION
		G.લીટી_આગત_state->get_exe_name = hush_command_name;
#  endif
#  જો EDITING_HAS_sh_get_var
		G.લીટી_આગત_state->sh_get_var = get_local_var_કિંમત;
#  endif
# endif
# જો ENABLE_HUSH_SAVEHISTORY && MAX_HISTORY > 0
		{
			અચલ અક્ષર *hp = get_local_var_કિંમત("HISTFILE");
			જો (!hp) {
				hp = get_local_var_કિંમત("HOME");
				જો (hp)
					hp = concat_path_file(hp, ".hush_history");
			} નહીંતર {
				hp = xstrdup(hp);
			}
			જો (hp) {
				G.લીટી_આગત_state->hist_file = hp;
				//set_local_var(xasprintf("HISTFILE=%s", ...));
			}
#  જો ENABLE_FEATURE_SH_HISTFILESIZE
			hp = get_local_var_કિંમત("HISTFILESIZE");
			G.લીટી_આગત_state->max_history = size_from_HISTFILESIZE(hp);
#  endif
		}
# endif
	} નહીંતર {
		install_special_sighandlers();
	}
#અથવાજો ENABLE_HUSH_INTERACTIVE
	/* No job control compiled in, only prompt/line editing */
	જો (isatty(STDIN_FILENO) && isatty(STDOUT_FILENO)) {
		G_interactive_fd = dup_CLOEXEC(STDIN_FILENO, 254);
		જો (G_interactive_fd < 0) {
			/* try to dup to any fd */
			G_interactive_fd = dup_CLOEXEC(STDIN_FILENO, -1);
			જો (G_interactive_fd < 0)
				/* give up */
				G_interactive_fd = 0;
		}
	}
	install_special_sighandlers();
#નહીંતર
	/* We have interactiveness code disabled */
	install_special_sighandlers();
#endif
	/* bash:
	 * if interactive but not a login shell, sources ~/.bashrc
	 * (--norc turns this off, --rcfile <file> overrides)
	 */

	જો (G_interactive_fd) {
#જો ENABLE_HUSH_INTERACTIVE && ENABLE_FEATURE_EDITING_FANCY_PROMPT
		/* Set (but not export) PS1/2 unless already set */
		જો (!get_local_var_કિંમત("PS1"))
			set_local_var_from_halves("PS1", "\\w \\$ ");
		જો (!get_local_var_કિંમત("PS2"))
			set_local_var_from_halves("PS2", "> ");
#endif
		જો (!ENABLE_FEATURE_SH_EXTRA_QUIET) {
			/* note: ash and hush share this string */
			printf("\n\n%s %s\n"
				IF_HUSH_HELP("Enter 'help' for a list of built-in commands.\n")
				"\n",
				bb_banner,
				"hush - the humble shell"
			);
		}
	}

	parse_and_run_file(hfopen(ખાલી)); /* stdin */

 final_return:
	hush_exit(G.last_exitcode);
}

/*
 * Built-ins
 */
સ્થિર પૂર્ણાંક FAST_FUNC builtin_true(અક્ષર **argv UNUSED_PARAM)
{
	પાછા 0;
}

સ્થિર પૂર્ણાંક FAST_FUNC builtin_false(અક્ષર **argv UNUSED_PARAM)
{
	પાછા 1;
}

#જો ENABLE_HUSH_TEST || ENABLE_HUSH_ECHO || ENABLE_HUSH_PRINTF || ENABLE_HUSH_KILL
સ્થિર NOINLINE પૂર્ણાંક run_applet_main(અક્ષર **argv, પૂર્ણાંક (*applet_main_func)(પૂર્ણાંક argc, અક્ષર **argv))
{
	પૂર્ણાંક argc = string_array_len(argv);
	પાછા applet_main_func(argc, argv);
}
#endif
#જો ENABLE_HUSH_TEST || BASH_TEST2
સ્થિર પૂર્ણાંક FAST_FUNC builtin_test(અક્ષર **argv)
{
	પાછા run_applet_main(argv, test_main);
}
#endif
#જો ENABLE_HUSH_ECHO
સ્થિર પૂર્ણાંક FAST_FUNC builtin_echo(અક્ષર **argv)
{
	પાછા run_applet_main(argv, echo_main);
}
#endif
#જો ENABLE_HUSH_PRINTF
સ્થિર પૂર્ણાંક FAST_FUNC builtin_printf(અક્ષર **argv)
{
	પાછા run_applet_main(argv, printf_main);
}
#endif

#જો ENABLE_HUSH_HELP
સ્થિર પૂર્ણાંક FAST_FUNC builtin_help(અક્ષર **argv UNUSED_PARAM)
{
	અચલ સંરચના built_in_command *x;

	printf(
		"Built-in commands:\n"
		"------------------\n");
	માટે (x = bltins1; x != &bltins1[ARRAY_SIZE(bltins1)]; x++) {
		જો (x->b_descr)
			printf("%-10s%s\n", x->b_cmd, x->b_descr);
	}
	પાછા EXIT_SUCCESS;
}
#endif

#જો MAX_HISTORY && ENABLE_FEATURE_EDITING
સ્થિર પૂર્ણાંક FAST_FUNC builtin_history(અક્ષર **argv UNUSED_PARAM)
{
	show_history(G.લીટી_આગત_state);
	પાછા EXIT_SUCCESS;
}
#endif

સ્થિર પૂર્ણાંક FAST_FUNC builtin_cd(અક્ષર **argv)
{
	અચલ અક્ષર *newdir;

	argv = skip_dash_dash(argv);
	newdir = argv[0];
	જો (newdir == ખાલી) {
		/* bash does nothing (exitcode 0) if HOME is ""; if it's unset,
		 * bash says "bash: cd: HOME not set" and does nothing
		 * (exitcode 1)
		 */
		અચલ અક્ષર *home = get_local_var_કિંમત("HOME");
		newdir = home ? home : "/";
	}
	જો (chdir(newdir)) {
		/* Mimic bash message exactly */
		bb_perror_સંદેશ("cd: %s", newdir);
		પાછા EXIT_FAILURE;
	}
	/* Read current dir (get_cwd(1) is inside) and set PWD.
	 * Note: do not enforce exporting. If PWD was unset or unexported,
	 * set it again, but do not export. bash does the same.
	 */
	set_pwd_var(/*flag:*/ 0);
	પાછા EXIT_SUCCESS;
}

સ્થિર પૂર્ણાંક FAST_FUNC builtin_pwd(અક્ષર **argv UNUSED_PARAM)
{
	puts(get_cwd(0));
	પાછા EXIT_SUCCESS;
}

સ્થિર પૂર્ણાંક FAST_FUNC builtin_eval(અક્ષર **argv)
{
	argv = skip_dash_dash(argv);

	જો (!argv[0])
		પાછા EXIT_SUCCESS;

	IF_HUSH_MODE_X(G.x_mode_depth++;)
	//bb_error_msg("%s: ++x_mode_depth=%d", __func__, G.x_mode_depth);
	જો (!argv[1]) {
		/* bash:
		 * eval "echo Hi; done" ("done" is syntax error):
		 * "echo Hi" will not execute too.
		 */
		parse_and_run_string(argv[0]);
	} નહીંતર {
		/* "The eval utility shall construct a command by
		 * concatenating arguments together, separating
		 * each with a <space> character."
		 */
		અક્ષર *str, *p;
		unsigned len = 0;
		અક્ષર **pp = argv;
		કરો
			len += strlen(*pp) + 1;
		જ્યારે (*++pp);
		str = p = xmalloc(len);
		pp = argv;
		માટે (;;) {
			p = stpcpy(p, *pp);
			pp++;
			જો (!*pp)
				તોડો;
			*p++ = ' ';
		}
		parse_and_run_string(str);
		મુક્ત(str);
	}
	IF_HUSH_MODE_X(G.x_mode_depth--;)
	//bb_error_msg("%s: --x_mode_depth=%d", __func__, G.x_mode_depth);
	પાછા G.last_exitcode;
}

સ્થિર પૂર્ણાંક FAST_FUNC builtin_exec(અક્ષર **argv)
{
	argv = skip_dash_dash(argv);
	જો (argv[0] == ખાલી)
		પાછા EXIT_SUCCESS; /* bash does this */

	/* Careful: we can end up here after [v]fork. Do not restore
	 * tty pgrp then, only top-level shell process does that */
	જો (G_saved_tty_pgrp && getpid() == G.મૂળ_pid)
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

સ્થિર પૂર્ણાંક FAST_FUNC builtin_exit(અક્ષર **argv)
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
	જો (argv[0] == ખાલી) {
#જો ENABLE_HUSH_TRAP
		જો (G.pre_trap_exitcode >= 0) /* "exit" in trap uses $? from before the trap */
			hush_exit(G.pre_trap_exitcode);
#endif
		hush_exit(G.last_exitcode);
	}
	/* mimic bash: exit 123abc == exit 255 + error msg */
	xfunc_error_retval = 255;
	/* bash: exit -2 == exit 254, no error msg */
	hush_exit(xatoi(argv[0]) & 0xff);
}

#જો ENABLE_HUSH_TYPE
/* http://www.opengroup.org/onlinepubs/9699919799/utilities/type.html */
સ્થિર પૂર્ણાંક FAST_FUNC builtin_type(અક્ષર **argv)
{
	પૂર્ણાંક ret = EXIT_SUCCESS;

	જ્યારે (*++argv) {
		અચલ અક્ષર *type;
		અક્ષર *path = ખાલી;

		જો (0) {} /* make conditional compile easier below */
		/*else if (find_alias(*argv))
			type = "an alias";*/
# જો ENABLE_HUSH_FUNCTIONS
		નહીંતર જો (find_function(*argv))
			type = "a function";
# endif
		નહીંતર જો (find_builtin(*argv))
			type = "a shell builtin";
		નહીંતર જો ((path = find_in_path(*argv)) != ખાલી)
			type = path;
		નહીંતર {
			bb_error_સંદેશ("type: %s: not found", *argv);
			ret = EXIT_FAILURE;
			ચાલુ;
		}

		printf("%s is %s\n", *argv, type);
		મુક્ત(path);
	}

	પાછા ret;
}
#endif

#જો ENABLE_HUSH_READ
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
સ્થિર પૂર્ણાંક FAST_FUNC builtin_read(અક્ષર **argv)
{
	અચલ અક્ષર *r;
	સંરચના builtin_read_params params;

	memset(&params, 0, કદ(params));

	/* "!": do not abort on errors.
	 * Option string must start with "sr" to match BUILTIN_READ_xxx
	 */
	params.read_flags = getopt32(argv,
# જો BASH_READ_D
		IF_NOT_HUSH_BASH_COMPAT("^")
		"!srn:p:t:u:d:" IF_NOT_HUSH_BASH_COMPAT("\0" "-1"/*min 1 arg*/),
		&params.opt_n, &params.opt_p, &params.opt_t, &params.opt_u, &params.opt_d
# નહીંતર
		IF_NOT_HUSH_BASH_COMPAT("^")
		"!srn:p:t:u:" IF_NOT_HUSH_BASH_COMPAT("\0" "-1"/*min 1 arg*/),
		&params.opt_n, &params.opt_p, &params.opt_t, &params.opt_u
# endif
//TODO: print "read: need variable name"
//for the case of !BASH "read" with no args (now it fails silently)
//(or maybe extend getopt32() to emit a message if "-1" fails)
	);
	જો ((uint32_t)params.read_flags == (uint32_t)-1)
		પાછા EXIT_FAILURE;
	argv += optind;
	params.argv = argv;
	params.setvar = set_local_var_from_halves;
	params.ifs = get_local_var_કિંમત("IFS"); /* can be NULL */

 again:
	r = shell_builtin_read(&params);

	જો ((uintptr_t)r == 1 && errno == EINTR) {
		unsigned sig = check_and_run_traps();
		જો (sig != SIGINT)
			જાઓ again;
	}

	જો ((uintptr_t)r > 1) {
		bb_simple_error_સંદેશ(r);
		r = (અક્ષર*)(uintptr_t)1;
	}

	પાછા (uintptr_t)r;
}
#endif

#જો ENABLE_HUSH_UMASK
સ્થિર પૂર્ણાંક FAST_FUNC builtin_umask(અક્ષર **argv)
{
	પૂર્ણાંક rc;
	mode_t mask;

	rc = 1;
	mask = umask(0);
	argv = skip_dash_dash(argv);
	જો (argv[0]) {
		mode_t old_mask = mask;

		/* numeric umasks are taken as-is */
		/* symbolic umasks are inverted: "umask a=rx" calls umask(222) */
		જો (!isdigit(argv[0][0]))
			mask ^= 0777;
		mask = bb_parse_mode(argv[0], mask);
		જો (!isdigit(argv[0][0]))
			mask ^= 0777;
		જો ((unsigned)mask > 0777) {
			mask = old_mask;
			/* bash messages:
			 * bash: umask: 'q': invalid symbolic mode operator
			 * bash: umask: 999: octal number out of range
			 */
			bb_error_સંદેશ("%s: invalid mode '%s'", "umask", argv[0]);
			rc = 0;
		}
	} નહીંતર {
		/* Mimic bash */
		printf("%04o\n", (unsigned) mask);
		/* fall through and restore mask which we set to 0 */
	}
	umask(mask);

	પાછા !rc; /* rc != 0 - success */
}
#endif

#જો ENABLE_HUSH_EXPORT || ENABLE_HUSH_READONLY || ENABLE_HUSH_SET || ENABLE_HUSH_TRAP
સ્થિર શૂન્ય print_escaped(અચલ અક્ષર *s)
{
//TODO? bash "set" does not quote variables which contain only alnums and "%+,-./:=@_~",
// (but "export" quotes all variables, even with only these chars).
// I think quoting strings with %+,=~ looks better
// (example: "set" printing var== instead of var='=' looks strange)
// IOW: do not quote "-./:@_": / is used in pathnames, : in PATH, -._ often in file names, @ in emails

	જો (*s == '\'')
		જાઓ squote;
	કરો {
		અચલ અક્ષર *p = strchrnul(s, '\'');
		/* print 'xxxx', possibly just '' */
		printf("'%.*s'", (પૂર્ણાંક)(p - s), s);
		જો (*p == '\0')
			તોડો;
		s = p;
 squote:
		/* s points to '; print "'''...'''" */
		putchar('"');
		કરો putchar('\''); જ્યારે (*++s == '\'');
		putchar('"');
	} જ્યારે (*s);
}
#endif

#જો ENABLE_HUSH_EXPORT || ENABLE_HUSH_LOCAL || ENABLE_HUSH_READONLY
સ્થિર પૂર્ણાંક helper_export_local(અક્ષર **argv, unsigned flags)
{
	કરો {
		અક્ષર *name = *argv;
		અચલ અક્ષર *name_end = endofname(name);

		જો (*name_end == '\0') {
			સંરચના variable *var, **vpp;

			vpp = get_ptr_to_local_var(name);
			var = vpp ? *vpp : ખાલી;

			જો (flags & SETFLAG_UNEXPORT) {
				/* export -n NAME (without =VALUE) */
				જો (var) {
					var->flg_export = 0;
					debug_printf_env("%s: unsetenv '%s'\n", __func__, name);
					unsetenv(name);
				} /* else: export -n NOT_EXISTING_VAR: no-op */
				ચાલુ;
			}
			જો (flags & SETFLAG_EXPORT) {
				/* export NAME (without =VALUE) */
				જો (var) {
					var->flg_export = 1;
					debug_printf_env("%s: putenv '%s'\n", __func__, var->varstr);
					putenv(var->varstr);
					ચાલુ;
				}
			}
			જો (flags & SETFLAG_MAKE_RO) {
				/* readonly NAME (without =VALUE) */
				જો (var) {
					var->flg_read_only = 1;
					ચાલુ;
				}
			}
# જો ENABLE_HUSH_LOCAL
			/* Is this "local" bltin? */
			જો (!(flags & (SETFLAG_EXPORT|SETFLAG_UNEXPORT|SETFLAG_MAKE_RO))) {
				unsigned lvl = flags >> SETFLAG_VARLVL_SHIFT;
				જો (var && var->var_nest_level == lvl) {
					/* "local x=abc; ...; local x" - ignore second local decl */
					ચાલુ;
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
		} નહીંતર {
			જો (*name_end != '=') {
				bb_error_સંદેશ("'%s': bad variable name", name);
				/* do not parse following argv[]s: */
				પાછા 1;
			}
			/* (Un)exporting/making local NAME=VALUE */
			name = xstrdup(name);
			/* Testcase: export PS1='\w \$ ' */
			unbackslash(name);
		}
		debug_printf_env("%s: set_local_var('%s')\n", __func__, name);
		જો (set_local_var(name, flags))
			પાછા EXIT_FAILURE;
	} જ્યારે (*++argv);
	પાછા EXIT_SUCCESS;
}
#endif

#જો ENABLE_HUSH_EXPORT
સ્થિર પૂર્ણાંક FAST_FUNC builtin_export(અક્ષર **argv)
{
	unsigned opt_unexport;

# જો ENABLE_HUSH_EXPORT_N
	/* "!": do not abort on errors */
	opt_unexport = getopt32(argv, "!n");
	જો (opt_unexport == (uint32_t)-1)
		પાછા EXIT_FAILURE;
	argv += optind;
# નહીંતર
	opt_unexport = 0;
	argv++;
# endif

	જો (argv[0] == ખાલી) {
		અક્ષર **e = environ;
		જો (e) {
			જ્યારે (*e) {
# જો 0
				puts(*e++);
# નહીંતર
				/* ash emits: export VAR='VAL'
				 * bash: declare -x VAR="VAL"
				 * we follow ash example */
				અચલ અક્ષર *s = *e++;
				અચલ અક્ષર *p = strchr(s, '=');

				જો (!p) /* wtf? take next variable */
					ચાલુ;
				/* "export VAR=" */
				printf("%s %.*s", "export", (પૂર્ણાંક)(p - s) + 1, s);
				print_escaped(p + 1);
				putchar('\n');
# endif
			}
			/*fflush_all(); - done after each builtin anyway */
		}
		પાછા EXIT_SUCCESS;
	}

	પાછા helper_export_local(argv, opt_unexport ? SETFLAG_UNEXPORT : SETFLAG_EXPORT);
}
#endif

#જો ENABLE_HUSH_LOCAL
સ્થિર પૂર્ણાંક FAST_FUNC builtin_local(અક્ષર **argv)
{
	જો (G.func_nest_level == 0) {
		bb_error_સંદેશ("%s: not in a function", argv[0]);
		પાછા EXIT_FAILURE; /* bash compat */
	}
//TODO? ash and bash support "local -" special form,
//which saves/restores $- around function call (including async returns, such as ^C)
//(IOW: it makes "set +/-..." effects local)
	argv++;
	/* Since all builtins run in a nested variable level,
	 * need to use level - 1 here. Or else the variable will be removed at once
	 * after builtin returns.
	 */
	પાછા helper_export_local(argv, (G.var_nest_level - 1) << SETFLAG_VARLVL_SHIFT);
}
#endif

#જો ENABLE_HUSH_READONLY
સ્થિર પૂર્ણાંક FAST_FUNC builtin_readonly(અક્ષર **argv)
{
	argv++;
	જો (*argv == ખાલી) {
		/* bash: readonly [-p]: list all readonly VARs
		 * (-p has no effect in bash)
		 */
		સંરચના variable *e;
		માટે (e = G.ટોચ_var; e; e = e->next) {
			જો (e->flg_read_only) {
				અચલ અક્ષર *s = e->varstr;
				અચલ અક્ષર *p = strchr(s, '=');

				જો (!p) /* wtf? take next variable */
					ચાલુ;
				/* "readonly VAR=" */
				printf("%s %.*s", "readonly", (પૂર્ણાંક)(p - s) + 1, s);
				print_escaped(p + 1);
				putchar('\n');
			}
		}
		પાછા EXIT_SUCCESS;
	}
	પાછા helper_export_local(argv, SETFLAG_MAKE_RO);
}
#endif

#જો ENABLE_HUSH_UNSET
/* http://www.opengroup.org/onlinepubs/9699919799/utilities/V3_chap02.html#unset */
સ્થિર પૂર્ણાંક FAST_FUNC builtin_unset(અક્ષર **argv)
{
	પૂર્ણાંક ret;
	unsigned opts;

	/* "!": do not abort on errors */
	/* "+": stop at 1st non-option */
	opts = getopt32(argv, "!+vf");
	જો (opts == (unsigned)-1)
		પાછા EXIT_FAILURE;
	જો (opts == 3) {
		bb_simple_error_સંદેશ("unset: -v and -f are exclusive");
		પાછા EXIT_FAILURE;
	}
	argv += optind;

	ret = EXIT_SUCCESS;
	જ્યારે (*argv) {
		જો (!(opts & 2)) { /* not -f */
			જો (unset_local_var(*argv)) {
				/* unset <nonexistent_var> doesn't fail.
				 * Error is when one tries to unset RO var.
				 * Message was printed by unset_local_var. */
				ret = EXIT_FAILURE;
			}
		}
# જો ENABLE_HUSH_FUNCTIONS
		નહીંતર {
			unset_func(*argv);
		}
# endif
		argv++;
	}
	પાછા ret;
}
#endif

#જો ENABLE_HUSH_SET
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
સ્થિર પૂર્ણાંક FAST_FUNC builtin_set(અક્ષર **argv)
{
	પૂર્ણાંક n;
	અક્ષર **pp, **g_argv;
	અક્ષર *arg = *++argv;

	જો (arg == ખાલી) {
		સંરચના variable *e;
		માટે (e = G.ટોચ_var; e; e = e->next) {
			અચલ અક્ષર *s = e->varstr;
			અચલ અક્ષર *p = strchr(s, '=');

			જો (!p) /* wtf? take next variable */
				ચાલુ;
			/* var= */
			printf("%.*s", (પૂર્ણાંક)(p - s) + 1, s);
			print_escaped(p + 1);
			putchar('\n');
		}
		પાછા EXIT_SUCCESS;
	}

	કરો {
		જો (strcmp(arg, "--") == 0) {
			++argv;
			જાઓ set_argv;
		}
		જો (arg[0] != '+' && arg[0] != '-')
			તોડો;
		માટે (n = 1; arg[n]; ++n) {
			જો (set_mode((arg[0] == '-'), arg[n], argv[1])) {
				bb_error_સંદેશ("%s: %s: invalid option", "set", arg);
				પાછા EXIT_FAILURE;
			}
			જો (arg[n] == 'o' && argv[1])
				argv++;
		}
	} જ્યારે ((arg = *++argv) != ખાલી);
	/* Now argv[0] is 1st argument */

	જો (arg == ખાલી)
		પાછા EXIT_SUCCESS;
 set_argv:

	/* NB: G.global_argv[0] ($0) is never freed/changed */
	g_argv = G.global_argv;
	જો (G.global_args_malloced) {
		pp = g_argv;
		જ્યારે (*++pp)
			મુક્ત(*pp);
		g_argv[1] = ખાલી;
	} નહીંતર {
		G.global_args_malloced = 1;
		pp = xzalloc(કદ(pp[0]) * 2);
		pp[0] = g_argv[0]; /* retain $0 */
		g_argv = pp;
	}
	/* This realloc's G.global_argv */
	G.global_argv = pp = add_strings_to_strings(g_argv, argv, /*dup:*/ 1);

	G.global_argc = 1 + string_array_len(pp + 1);

	પાછા EXIT_SUCCESS;
}
#endif

સ્થિર પૂર્ણાંક FAST_FUNC builtin_shift(અક્ષર **argv)
{
	પૂર્ણાંક n = 1;
	argv = skip_dash_dash(argv);
	જો (argv[0]) {
		n = bb_strtou(argv[0], ખાલી, 10);
		જો (errno || n < 0) {
			/* shared string with ash.c */
			bb_error_સંદેશ("Illegal number: %s", argv[0]);
			/*
			 * ash aborts in this case.
			 * bash prints error message and set $? to 1.
			 * Interestingly, for "shift 99999" bash does not
			 * print error message, but does set $? to 1
			 * (and does no shifting at all).
			 */
		}
	}
	જો (n >= 0 && n < G.global_argc) {
		જો (G_global_args_malloced) {
			પૂર્ણાંક m = 1;
			જ્યારે (m <= n)
				મુક્ત(G.global_argv[m++]);
		}
		G.global_argc -= n;
		memmove(&G.global_argv[1], &G.global_argv[n+1],
				G.global_argc * કદ(G.global_argv[0]));
		પાછા EXIT_SUCCESS;
	}
	પાછા EXIT_FAILURE;
}

#જો ENABLE_HUSH_GETOPTS
સ્થિર પૂર્ણાંક FAST_FUNC builtin_getopts(અક્ષર **argv)
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
	અક્ષર cbuf[2];
	અચલ અક્ષર *cp, *optstring, *var;
	પૂર્ણાંક c, n, exitcode, my_opterr;
	unsigned ગણતરી;

	optstring = *++argv;
	જો (!optstring || !(var = *++argv)) {
		bb_simple_error_સંદેશ("usage: getopts OPTSTRING VAR [ARGS]");
		પાછા EXIT_FAILURE;
	}

	જો (argv[1])
		argv[0] = G.global_argv[0]; /* for error messages in getopt() */
	નહીંતર
		argv = G.global_argv;
	cbuf[1] = '\0';

	my_opterr = 0;
	જો (optstring[0] != ':') {
		cp = get_local_var_કિંમત("OPTERR");
		/* 0 if "OPTERR=0", 1 otherwise */
		my_opterr = (!cp || NOT_LONE_CHAR(cp, '0'));
	}

	/* getopts stops on first non-option. Add "+" to force that */
	/*if (optstring[0] != '+')*/ {
		અક્ષર *s = alloca(strlen(optstring) + 2);
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
	ગણતરી = 0;
	n = string_array_len(argv);
	કરો {
		optarg = ખાલી;
		opterr = (ગણતરી < G.getopt_ગણતરી) ? 0 : my_opterr;
		c = getopt(n, argv, optstring);
		જો (c < 0)
			તોડો;
		ગણતરી++;
	} જ્યારે (ગણતરી <= G.getopt_ગણતરી);

	/* Set OPTIND. Prevent resetting of the magic counter! */
	set_local_var_from_halves("OPTIND", utoa(optind));
	G.getopt_ગણતરી = ગણતરી; /* "next time, give me N+1'th result" */
	GETOPT_RESET(); /* just in case */

	/* Set OPTARG */
	/* Always set or unset, never left as-is, even on exit/error:
	 * "If no option was found, or if the option that was found
	 * does not have an option-argument, OPTARG shall be unset."
	 */
	cp = optarg;
	જો (c == '?') {
		/* If ":optstring" and unknown option is seen,
		 * it is stored to OPTARG.
		 */
		જો (optstring[1] == ':') {
			cbuf[0] = optopt;
			cp = cbuf;
		}
	}
	જો (cp)
		set_local_var_from_halves("OPTARG", cp);
	નહીંતર
		unset_local_var("OPTARG");

	/* Convert -1 to "?" */
	exitcode = EXIT_SUCCESS;
	જો (c < 0) { /* -1: end of options */
		exitcode = EXIT_FAILURE;
		c = '?';
	}

	/* Set VAR */
	cbuf[0] = c;
	set_local_var_from_halves(var, cbuf);

	પાછા exitcode;
}
#endif

સ્થિર પૂર્ણાંક FAST_FUNC builtin_source(અક્ષર **argv)
{
	અક્ષર *arg_path, *filename;
	HFILE *આગત;
	save_arg_t sv;
	અક્ષર *args_need_save;
#જો ENABLE_HUSH_FUNCTIONS
	smallint sv_flg;
#endif

	argv = skip_dash_dash(argv);
	filename = argv[0];
	જો (!filename) {
		/* bash says: "bash: .: filename argument required" */
		પાછા 2; /* bash compat */
	}
	arg_path = ખાલી;
	જો (!strchr(filename, '/')) {
		arg_path = find_in_path(filename);
		જો (arg_path)
			filename = arg_path;
		નહીંતર જો (!ENABLE_HUSH_BASH_SOURCE_CURDIR) {
			errno = ENOENT;
			bb_simple_perror_સંદેશ(filename);
			પાછા EXIT_FAILURE;
		}
	}
	આગત = hfopen(filename);
	મુક્ત(arg_path);
	જો (!આગત) {
		bb_perror_સંદેશ("%s", filename);
		/* POSIX: non-interactive shell should abort here,
		 * not merely fail. So far no one complained :)
		 */
		પાછા EXIT_FAILURE;
	}

#જો ENABLE_HUSH_FUNCTIONS
	sv_flg = G_flag_return_in_progress;
	/* "we are inside sourced file, ok to use return" */
	G_flag_return_in_progress = -1;
#endif
	args_need_save = argv[1]; /* used as a boolean variable */
	જો (args_need_save)
		save_and_replace_G_args(&sv, argv);

	/* "false; . ./empty_line; echo Zero:$?" should print 0 */
	G.last_exitcode = 0;
	parse_and_run_file(આગત);
	hfclose(આગત);

	જો (args_need_save) /* can't use argv[1] instead: "shift" can mangle it */
		restore_G_args(&sv, argv);
#જો ENABLE_HUSH_FUNCTIONS
	G_flag_return_in_progress = sv_flg;
#endif

	પાછા G.last_exitcode;
}

#જો ENABLE_HUSH_TRAP
સ્થિર પૂર્ણાંક FAST_FUNC builtin_trap(અક્ષર **argv)
{
	પૂર્ણાંક sig;
	અક્ષર *new_cmd;

	જો (!G_traps)
		G_traps = xzalloc(કદ(G_traps[0]) * NSIG);

	argv++;
	જો (!*argv) {
		પૂર્ણાંક i;
		/* No args: print all trapped */
		માટે (i = 0; i < NSIG; ++i) {
			જો (G_traps[i]) {
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
		પાછા EXIT_SUCCESS;
	}

	new_cmd = ખાલી;
	/* If first arg is a number: reset all specified signals */
	sig = bb_strtou(*argv, ખાલી, 10);
	જો (errno == 0) {
		પૂર્ણાંક ret;
 process_sig_list:
		ret = EXIT_SUCCESS;
		જ્યારે (*argv) {
			sighandler_t handler;

			sig = get_signum(*argv++);
			જો (sig < 0) {
				ret = EXIT_FAILURE;
				/* Mimic bash message exactly */
				bb_error_સંદેશ("trap: %s: invalid signal specification", argv[-1]);
				ચાલુ;
			}

			મુક્ત(G_traps[sig]);
			G_traps[sig] = xstrdup(new_cmd);

			debug_printf("trap: setting SIG%s (%i) to '%s'\n",
				get_signame(sig), sig, G_traps[sig]);

			/* There is no signal for 0 (EXIT) */
			જો (sig == 0)
				ચાલુ;

			જો (new_cmd)
				handler = (new_cmd[0] ? record_pending_signo : SIG_IGN);
			નહીંતર
				/* We are removing trap handler */
				handler = pick_sighandler(sig);
			install_sighandler(sig, handler);
		}
		પાછા ret;
	}

	જો (!argv[1]) { /* no second arg */
		bb_simple_error_સંદેશ("trap: invalid arguments");
		પાછા EXIT_FAILURE;
	}

	/* First arg is "-": reset all specified to default */
	/* First arg is "--": skip it, the rest is "handler SIGs..." */
	/* Everything else: set arg as signal handler
	 * (includes "" case, which ignores signal) */
	જો (argv[0][0] == '-') {
		જો (argv[0][1] == '\0') { /* "-" */
			/* new_cmd remains NULL: "reset these sigs" */
			જાઓ reset_traps;
		}
		જો (argv[0][1] == '-' && argv[0][2] == '\0') { /* "--" */
			argv++;
		}
		/* else: "-something", no special meaning */
	}
	new_cmd = *argv;
 reset_traps:
	argv++;
	જાઓ process_sig_list;
}
#endif

#જો ENABLE_HUSH_JOB
સ્થિર સંરચના pipe *parse_jobspec(અચલ અક્ષર *str)
{
	સંરચના pipe *pi;
	unsigned jobnum;

	જો (sscanf(str, "%%%u", &jobnum) != 1) {
		જો (str[0] != '%'
		 || (str[1] != '%' && str[1] != '+' && str[1] != '\0')
		) {
			bb_error_સંદેશ("bad argument '%s'", str);
			પાછા ખાલી;
		}
		/* It is "%%", "%+" or "%" - current job */
		jobnum = G.last_jobid;
		જો (jobnum == 0) {
			bb_simple_error_સંદેશ("no current job");
			પાછા ખાલી;
		}
	}
	માટે (pi = G.job_list; pi; pi = pi->next) {
		જો (pi->jobid == jobnum) {
			પાછા pi;
		}
	}
	bb_error_સંદેશ("%u: no such job", jobnum);
	પાછા ખાલી;
}

સ્થિર પૂર્ણાંક FAST_FUNC builtin_jobs(અક્ષર **argv UNUSED_PARAM)
{
	સંરચના pipe *job;
	અચલ અક્ષર *status_string;

	checkjobs(ખાલી, 0 /*(no pid to wait for)*/);
	માટે (job = G.job_list; job; job = job->next) {
		જો (job->alive_cmds == job->stopped_cmds)
			status_string = "Stopped";
		નહીંતર
			status_string = "Running";

		printf(JOB_STATUS_FORMAT, job->jobid, status_string, job->cmdtext);
	}

	clean_up_last_dead_job();

	પાછા EXIT_SUCCESS;
}

/* built-in 'fg' and 'bg' handler */
સ્થિર પૂર્ણાંક FAST_FUNC builtin_fg_bg(અક્ષર **argv)
{
	પૂર્ણાંક i;
	સંરચના pipe *pi;

	જો (!G_interactive_fd)
		પાછા EXIT_FAILURE;

	/* If they gave us no args, assume they want the last backgrounded task */
	જો (!argv[1]) {
		માટે (pi = G.job_list; pi; pi = pi->next) {
			જો (pi->jobid == G.last_jobid) {
				જાઓ found;
			}
		}
		bb_error_સંદેશ("%s: no current job", argv[0]);
		પાછા EXIT_FAILURE;
	}

	pi = parse_jobspec(argv[1]);
	જો (!pi)
		પાછા EXIT_FAILURE;
 found:
	/* TODO: bash prints a string representation
	 * of job being foregrounded (like "sleep 1 | cat") */
	જો (argv[0][0] == 'f' && G_saved_tty_pgrp) {
		/* Put the job into the foreground. */
		tcsetpgrp(G_interactive_fd, pi->pgrp);
	}

	/* Restart the processes in the job */
	debug_printf_jobs("reviving %d procs, pgrp %d\n", pi->અંક_cmds, pi->pgrp);
	માટે (i = 0; i < pi->અંક_cmds; i++) {
		debug_printf_jobs("reviving pid %d\n", pi->cmds[i].pid);
	}
	pi->stopped_cmds = 0;

	i = kill(- pi->pgrp, SIGCONT);
	જો (i < 0) {
		જો (errno == ESRCH) {
			delete_finished_job(pi);
			પાછા EXIT_SUCCESS;
		}
		bb_simple_perror_સંદેશ("kill (SIGCONT)");
	}

	જો (argv[0][0] == 'f') {
		remove_job_from_table(pi); /* FG job shouldn't be in job table */
		પાછા checkjobs_and_fg_shell(pi);
	}
	પાછા EXIT_SUCCESS;
}
#endif

#જો ENABLE_HUSH_KILL
સ્થિર પૂર્ણાંક FAST_FUNC builtin_kill(અક્ષર **argv)
{
	પૂર્ણાંક ret = 0;

# જો ENABLE_HUSH_JOB
	જો (argv[1] && strcmp(argv[1], "-l") != 0) {
		પૂર્ણાંક i = 1;

		કરો {
			સંરચના pipe *pi;
			અક્ષર *dst;
			પૂર્ણાંક j, n;

			જો (argv[i][0] != '%')
				ચાલુ;
			/*
			 * "kill %N" - job kill
			 * Converting to pgrp / pid kill
			 */
			pi = parse_jobspec(argv[i]);
			જો (!pi) {
				/* Eat bad jobspec */
				j = i;
				કરો {
					j++;
					argv[j - 1] = argv[j];
				} જ્યારે (argv[j]);
				ret = 1;
				i--;
				ચાલુ;
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
			n = G_interactive_fd ? 1 : pi->અંક_cmds;
			dst = alloca(n * કદ(પૂર્ણાંક)*4);
			argv[i] = dst;
			જો (G_interactive_fd)
				dst += sprintf(dst, " -%u", (પૂર્ણાંક)pi->pgrp);
			નહીંતર માટે (j = 0; j < n; j++) {
				સંરચના command *cmd = &pi->cmds[j];
				/* Skip exited members of the job */
				જો (cmd->pid == 0)
					ચાલુ;
				/*
				 * kill_main has matching code to expect
				 * leading space. Needed to not confuse
				 * negative pids with "kill -SIGNAL_NO" syntax
				 */
				dst += sprintf(dst, " %u", (પૂર્ણાંક)cmd->pid);
			}
			*dst = '\0';
		} જ્યારે (argv[++i]);
	}
# endif

	જો (argv[1] || ret == 0) {
		ret = run_applet_main(argv, kill_main);
	}
	/* else: ret = 1, "kill %bad_jobspec" case */
	પાછા ret;
}
#endif

#જો ENABLE_HUSH_WAIT
/* http://www.opengroup.org/onlinepubs/9699919799/utilities/wait.html */
# જો !ENABLE_HUSH_JOB
#  define wait_for_child_or_signal(pipe,pid) wait_for_child_or_signal(pid)
# endif
સ્થિર પૂર્ણાંક wait_for_child_or_signal(સંરચના pipe *waitfor_pipe, pid_t waitfor_pid)
{
	પૂર્ણાંક ret = 0;
	માટે (;;) {
		પૂર્ણાંક sig;
		sigset_t oldset;

		જો (!sigisemptyset(&G.pending_set))
			જાઓ check_sig;

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

		જો (!sigisemptyset(&G.pending_set)) {
			/* Crap! we raced with some signal! */
			જાઓ restore;
		}

		/*errno = 0; - checkjobs does this */
/* Can't pass waitfor_pipe into checkjobs(): it won't be interruptible */
		ret = checkjobs(ખાલી, waitfor_pid); /* waitpid(WNOHANG) inside */
		debug_printf_exec("checkjobs:%d\n", ret);
# જો ENABLE_HUSH_JOB
		જો (waitfor_pipe) {
			પૂર્ણાંક rcode = job_exited_or_stopped(waitfor_pipe);
			debug_printf_exec("job_exited_or_stopped:%d\n", rcode);
			જો (rcode >= 0) {
				ret = rcode;
				sigprocmask(SIG_SETMASK, &oldset, ખાલી);
				તોડો;
			}
		}
# endif
		/* if ECHILD, there are no children (ret is -1 or 0) */
		/* if ret == 0, no children changed state */
		/* if ret != 0, it's exitcode+1 of exited waitfor_pid child */
		જો (errno == ECHILD || ret) {
			ret--;
			જો (ret < 0) /* if ECHILD, may need to fix "ret" */
				ret = 0;
# જો ENABLE_HUSH_BASH_COMPAT
			જો (waitfor_pid == -1 && errno == ECHILD) {
				/* exitcode of "wait -n" with no children is 127, not 0 */
				ret = 127;
			}
# endif
			sigprocmask(SIG_SETMASK, &oldset, ખાલી);
			તોડો;
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
		sigprocmask(SIG_SETMASK, &oldset, ખાલી);
 check_sig:
		/* So, did we get a signal? */
		sig = check_and_run_traps();
		જો (sig /*&& sig != SIGCHLD - always true */) {
			/* Do this for any (non-ignored) signal, not only for ^C */
			ret = 128 | sig;
			તોડો;
		}
		/* SIGCHLD, or no signal, or ignored one, such as SIGQUIT. Repeat */
	}
	પાછા ret;
}

સ્થિર પૂર્ણાંક FAST_FUNC builtin_wait(અક્ષર **argv)
{
	પૂર્ણાંક ret;
	પૂર્ણાંક status;

	argv = skip_dash_dash(argv);
# જો ENABLE_HUSH_BASH_COMPAT
	જો (argv[0] && strcmp(argv[0], "-n") == 0) {
		/* wait -n */
		/* (bash accepts "wait -n PID" too and ignores PID) */
		G.dead_job_exitcode = -1;
		પાછા wait_for_child_or_signal(ખાલી, -1 /*no job, wait for one job*/);
	}
# endif
	જો (argv[0] == ખાલી) {
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
		પાછા wait_for_child_or_signal(ખાલી, 0 /*no job and no pid to wait for*/);
	}

	કરો {
		pid_t pid = bb_strtou(*argv, ખાલી, 10);
		જો (errno || pid <= 0) {
# જો ENABLE_HUSH_JOB
			જો (argv[0][0] == '%') {
				સંરચના pipe *wait_pipe;
				ret = 127; /* bash compat for bad jobspecs */
				wait_pipe = parse_jobspec(*argv);
				જો (wait_pipe) {
					ret = job_exited_or_stopped(wait_pipe);
					જો (ret < 0) {
						ret = wait_for_child_or_signal(wait_pipe, 0);
					} નહીંતર {
						/* waiting on "last dead job" removes it */
						clean_up_last_dead_job();
					}
				}
				/* else: parse_jobspec() already emitted error msg */
				ચાલુ;
			}
# endif
			/* mimic bash message */
			bb_error_સંદેશ("wait: '%s': not a pid or valid job spec", *argv);
			ret = EXIT_FAILURE;
			ચાલુ; /* bash checks all argv[] */
		}

		/* Do we have such child? */
		ret = waitpid(pid, &status, WNOHANG);
		જો (ret < 0) {
			/* No */
			ret = 127;
			જો (errno == ECHILD) {
				જો (pid == G.last_bg_pid) {
					/* "wait $!" but last bg task has already exited. Try:
					 * (sleep 1; exit 3) & sleep 2; echo $?; wait $!; echo $?
					 * In bash it prints exitcode 0, then 3.
					 * In dash, it is 127.
					 */
					ret = G.last_bg_pid_exitcode;
				} નહીંતર {
					/* Example: "wait 1". mimic bash message */
					bb_error_સંદેશ("wait: pid %u is not a child of this shell", (unsigned)pid);
				}
			} નહીંતર {
				/* ??? */
				bb_perror_સંદેશ("wait %s", *argv);
			}
			ચાલુ; /* bash checks all argv[] */
		}
		જો (ret == 0) {
			/* Yes, and it still runs */
			ret = wait_for_child_or_signal(ખાલી, pid);
		} નહીંતર {
			/* Yes, and it just exited */
			process_wait_result(ખાલી, pid, status);
			ret = WEXITSTATUS(status);
			જો (WIFSIGNALED(status))
				ret = 128 | WTERMSIG(status);
		}
	} જ્યારે (*++argv);

	પાછા ret;
}
#endif

#જો ENABLE_HUSH_LOOPS || ENABLE_HUSH_FUNCTIONS
સ્થિર unsigned parse_numeric_argv1(અક્ષર **argv, unsigned def, unsigned def_min)
{
	જો (argv[1]) {
		def = bb_strtou(argv[1], ખાલી, 10);
		જો (errno || def < def_min || argv[2]) {
			bb_error_સંદેશ("%s: bad arguments", argv[0]);
			def = UINT_MAX;
		}
	}
	પાછા def;
}
#endif

#જો ENABLE_HUSH_LOOPS
સ્થિર પૂર્ણાંક FAST_FUNC builtin_break(અક્ષર **argv)
{
	unsigned depth;
	જો (G.depth_of_loop == 0) {
		bb_error_સંદેશ("%s: only meaningful in a loop", argv[0]);
		/* if we came from builtin_continue(), need to undo "= 1" */
		G.flag_break_continue = 0;
		પાછા EXIT_SUCCESS; /* bash compat */
	}
	G.flag_break_continue++; /* BC_BREAK = 1, or BC_CONTINUE = 2 */

	G.depth_break_continue = depth = parse_numeric_argv1(argv, 1, 1);
	જો (depth == UINT_MAX)
		G.flag_break_continue = BC_BREAK;
	જો (G.depth_of_loop < depth)
		G.depth_break_continue = G.depth_of_loop;

	પાછા EXIT_SUCCESS;
}

સ્થિર પૂર્ણાંક FAST_FUNC builtin_continue(અક્ષર **argv)
{
	G.flag_break_continue = 1; /* BC_CONTINUE = 2 = 1+1 */
	પાછા builtin_break(argv);
}
#endif

#જો ENABLE_HUSH_FUNCTIONS
સ્થિર પૂર્ણાંક FAST_FUNC builtin_return(અક્ષર **argv)
{
	પૂર્ણાંક rc;

	જો (G_flag_return_in_progress != -1) {
		bb_error_સંદેશ("%s: not in a function or sourced script", argv[0]);
		પાછા EXIT_FAILURE; /* bash compat */
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
# જો ENABLE_HUSH_TRAP
	જો (argv[1]) { /* "return ARG" inside a running trap sets $? */
		debug_printf_exec("G.return_exitcode=%d\n", rc);
		G.return_exitcode = rc;
	}
# endif
	પાછા rc;
}
#endif

#જો ENABLE_HUSH_TIMES
સ્થિર પૂર્ણાંક FAST_FUNC builtin_times(અક્ષર **argv UNUSED_PARAM)
{
	સ્થિર અચલ uint8_t times_tbl[] ALIGN1 = {
		' ',  offsetof(સંરચના tms, tms_utime),
		'\n', offsetof(સંરચના tms, tms_stime),
		' ',  offsetof(સંરચના tms, tms_cutime),
		'\n', offsetof(સંરચના tms, tms_cstime),
		0
	};
	અચલ uint8_t *p;
	unsigned clk_tck;
	સંરચના tms buf;

	clk_tck = bb_clk_tck();

	times(&buf);
	p = times_tbl;
	કરો {
		unsigned sec, frac;
		unsigned long t;
		t = *(clock_t *)(((અક્ષર *) &buf) + p[1]);
		sec = t / clk_tck;
		frac = t % clk_tck;
		printf("%um%u.%03us%c",
			sec / 60, sec % 60,
			(frac * 1000) / clk_tck,
			p[0]);
		p += 2;
	} જ્યારે (*p);

	પાછા EXIT_SUCCESS;
}
#endif

#જો ENABLE_HUSH_MEMLEAK
સ્થિર પૂર્ણાંક FAST_FUNC builtin_memleak(અક્ષર **argv UNUSED_PARAM)
{
	શૂન્ય *p;
	unsigned long l;

# ifdef M_TRIM_THRESHOLD
	/* Optional. Reduces probability of false positives */
	malloc_trim(0);
# endif
	/* Crude attempt to find where "free memory" starts,
	 * sans fragmentation. */
	p = malloc(240);
	l = (unsigned long)p;
	મુક્ત(p);
	p = malloc(3400);
	જો (l < (unsigned long)p) l = (unsigned long)p;
	મુક્ત(p);

# જો 0  /* debug */
	{
		સંરચના mallinfo mi = mallinfo();
		printf("top alloc:0x%lx malloced:%d+%d=%d\n", l,
			mi.arena, mi.hblkhd, mi.arena + mi.hblkhd);
	}
# endif

	જો (!G.memleak_કિંમત)
		G.memleak_કિંમત = l;

	l -= G.memleak_કિંમત;
	જો ((long)l < 0)
		l = 0;
	l /= 1024;
	જો (l > 127)
		l = 127;

	/* Exitcode is "how many kilobytes we leaked since 1st call" */
	પાછા l;
}
#endif
#endif /* !__U_BOOT__ */
