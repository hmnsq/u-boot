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

#अगर_अघोषित __U_BOOT__
#अगर !(defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) \
	|| defined(__APPLE__) \
    )
# include <malloc.h>   /* for malloc_trim */
#पूर्ण_अगर
#समावेश <glob.h>
/* #include <dmalloc.h> */
#अगर ENABLE_HUSH_CASE
# include <fnmatch.h>
#पूर्ण_अगर
#समावेश <sys/times.h>
#समावेश <sys/utsname.h> /* for setting $HOSTNAME */

#समावेश "busybox.h"  /* for APPLET_IS_NOFORK/NOEXEC */
#समावेश "unicode.h"
#समावेश "shell_common.h"
#समावेश "math.h"
#समावेश "match.h"
#अगर ENABLE_HUSH_RANDOM_SUPPORT
# include "random.h"
#अन्यथा
# define CLEAR_RANDOM_T(rnd) ((व्योम)0)
#पूर्ण_अगर
#अगर_अघोषित O_CLOEXEC
# define O_CLOEXEC 0
#पूर्ण_अगर
#अगर_अघोषित F_DUPFD_CLOEXEC
# define F_DUPFD_CLOEXEC F_DUPFD
#पूर्ण_अगर

#अगर ENABLE_FEATURE_SH_EMBEDDED_SCRIPTS && !ENABLE_SHELL_ASH
# include "embedded_scripts.h"
#अन्यथा
# define अंक_SCRIPTS 0
#पूर्ण_अगर
#पूर्ण_अगर /* !__U_BOOT__ */

/* So far, all bash compat is controlled by one config option */
/* Separate defines document which part of code implements what */
#घोषणा BASH_PATTERN_SUBST ENABLE_HUSH_BASH_COMPAT
#घोषणा BASH_SUBSTR        ENABLE_HUSH_BASH_COMPAT
#घोषणा BASH_SOURCE        ENABLE_HUSH_BASH_COMPAT
#घोषणा BASH_DOLLAR_SQUOTE ENABLE_HUSH_BASH_COMPAT
#घोषणा BASH_HOSTNAME_VAR  ENABLE_HUSH_BASH_COMPAT
#घोषणा BASH_EPOCH_VARS    ENABLE_HUSH_BASH_COMPAT
#घोषणा BASH_TEST2         (ENABLE_HUSH_BASH_COMPAT && ENABLE_HUSH_TEST)
#घोषणा BASH_READ_D        ENABLE_HUSH_BASH_COMPAT

/* Build knobs */
#घोषणा LEAK_HUNTING 0
#घोषणा BUILD_AS_NOMMU 0
/* Enable/disable sanity checks. Ok to enable in production,
 * only adds a bit of bloat. Set to >1 to get non-production level verbosity.
 * Keeping 1 for now even in released versions.
 */
#घोषणा HUSH_DEBUG 1
/* Slightly bigger (+200 bytes), but faster hush.
 * So far it only enables a trick with counting SIGCHLDs and forks,
 * which allows us to do fewer waitpid's.
 * (we can detect a case where neither forks were done nor SIGCHLDs happened
 * and therefore waitpid will return the same result as last time)
 */
#घोषणा ENABLE_HUSH_FAST 0
/* TODO: implement simplified code for users which do not need ${var%...} ops
 * So far ${var%...} ops are always enabled:
 */
#घोषणा ENABLE_HUSH_DOLLAR_OPS 1

#अगर BUILD_AS_NOMMU
# undef BB_MMU
# undef USE_FOR_NOMMU
# undef USE_FOR_MMU
# define BB_MMU 0
# define USE_FOR_NOMMU(...) __VA_ARGS__
# define USE_FOR_MMU(...)
#पूर्ण_अगर

#अगर_अघोषित __U_BOOT__
#समावेश "NUM_APPLETS.h"
#अगर अंक_APPLETS == 1
/* STANDALONE does not make sense, and won't compile */
# undef ENABLE_FEATURE_SH_STANDALONE
# undef IF_FEATURE_SH_STANDALONE
# undef IF_NOT_FEATURE_SH_STANDALONE
# define ENABLE_FEATURE_SH_STANDALONE 0
# define IF_FEATURE_SH_STANDALONE(...)
# define IF_NOT_FEATURE_SH_STANDALONE(...) __VA_ARGS__
#पूर्ण_अगर
#पूर्ण_अगर /* __U_BOOT__ */

#अगर !ENABLE_HUSH_INTERACTIVE
# undef ENABLE_FEATURE_EDITING
# define ENABLE_FEATURE_EDITING 0
# undef ENABLE_FEATURE_EDITING_FANCY_PROMPT
# define ENABLE_FEATURE_EDITING_FANCY_PROMPT 0
# undef ENABLE_FEATURE_EDITING_SAVE_ON_EXIT
# define ENABLE_FEATURE_EDITING_SAVE_ON_EXIT 0
#पूर्ण_अगर

/* Do we support ANY keywords? */
#अगर ENABLE_HUSH_IF || ENABLE_HUSH_LOOPS || ENABLE_HUSH_CASE
# define HAS_KEYWORDS 1
# define IF_HAS_KEYWORDS(...) __VA_ARGS__
# define IF_HAS_NO_KEYWORDS(...)
#अन्यथा
# define HAS_KEYWORDS 0
# define IF_HAS_KEYWORDS(...)
# define IF_HAS_NO_KEYWORDS(...) __VA_ARGS__
#पूर्ण_अगर

/* If you comment out one of these below, it will be #defined later
 * to perform debug printfs to stderr: */
#घोषणा debug_printf(...)         करो {} जबतक (0)
/* Finer-grained debug switches */
#घोषणा debug_printf_parse(...)   करो {} जबतक (0)
#घोषणा debug_printf_heredoc(...) करो {} जबतक (0)
#घोषणा debug_print_वृक्ष(a, b)    करो {} जबतक (0)
#घोषणा debug_printf_exec(...)    करो {} जबतक (0)
#घोषणा debug_printf_env(...)     करो {} जबतक (0)
#घोषणा debug_printf_jobs(...)    करो {} जबतक (0)
#घोषणा debug_printf_expand(...)  करो {} जबतक (0)
#घोषणा debug_printf_varexp(...)  करो {} जबतक (0)
#घोषणा debug_printf_glob(...)    करो {} जबतक (0)
#घोषणा debug_printf_redir(...)   करो {} जबतक (0)
#घोषणा debug_printf_list(...)    करो {} जबतक (0)
#घोषणा debug_printf_subst(...)   करो {} जबतक (0)
#घोषणा debug_printf_prompt(...)  करो {} जबतक (0)
#घोषणा debug_printf_clean(...)   करो {} जबतक (0)

#घोषणा ERR_PTR ((व्योम*)(दीर्घ)1)

#घोषणा JOB_STATUS_FORMAT    "[%u] %-22s %.40s\n"

#घोषणा _SPECIAL_VARS_STR     "_*@$!?#-"
#अगर_अघोषित __U_BOOT__
#घोषणा SPECIAL_VARS_STR     ("_*@$!?#-" + 1)
#घोषणा NUMERIC_SPECVARS_STR ("_*@$!?#-" + 3)
#अन्यथा /* __U_BOOT__ */
#घोषणा SPECIAL_VARS_STR     "*@$!?#-"
#घोषणा NUMERIC_SPECVARS_STR "$!?#-"
#पूर्ण_अगर /* __U_BOOT__ */
#अगर BASH_PATTERN_SUBST
/* Support / and // replace ops */
/* Note that // is stored as \ in "encoded" string representation */
# define VAR_ENCODED_SUBST_OPS      "\\/%#:-=+?"
# define VAR_SUBST_OPS             ("\\/%#:-=+?" + 1)
# define MINUS_PLUS_EQUAL_QUESTION ("\\/%#:-=+?" + 5)
#अन्यथा
# define VAR_ENCODED_SUBST_OPS      "%#:-=+?"
# define VAR_SUBST_OPS              "%#:-=+?"
# define MINUS_PLUS_EQUAL_QUESTION ("%#:-=+?" + 3)
#पूर्ण_अगर

#घोषणा SPECIAL_VAR_प्रतीक_STR "\3"
#घोषणा SPECIAL_VAR_प्रतीक       3
/* The "variable" with name "\1" emits string "\3". Testcase: "echo ^C" */
#घोषणा SPECIAL_VAR_QUOTED_SVS   1

काष्ठा variable;

अटल स्थिर अक्षर hush_version_str[] ALIGN1 = "HUSH_VERSION="BB_VER;

/* This supports saving pointers malloced in vfork child,
 * to be freed in the parent.
 */
#अगर !BB_MMU
प्रकार काष्ठा nommu_save_t {
	काष्ठा variable *old_vars;
	अक्षर **argv;
	अक्षर **argv_from_re_execing;
} nommu_save_t;
#पूर्ण_अगर

क्रमागत {
	RES_NONE  = 0,
#अगर ENABLE_HUSH_IF
	RES_IF    ,
	RES_THEN  ,
	RES_ELIF  ,
	RES_ELSE  ,
	RES_FI    ,
#पूर्ण_अगर
#अगर ENABLE_HUSH_LOOPS
	RES_FOR   ,
	RES_WHILE ,
	RES_UNTIL ,
	RES_DO    ,
	RES_DONE  ,
#पूर्ण_अगर
#अगर ENABLE_HUSH_LOOPS || ENABLE_HUSH_CASE
	RES_IN    ,
#पूर्ण_अगर
#अगर ENABLE_HUSH_CASE
	RES_CASE  ,
	/* three pseudo-keywords support contrived "case" syntax: */
	RES_CASE_IN,   /* "case ... IN", turns into RES_MATCH when IN is observed */
	RES_MATCH ,    /* "word)" */
	RES_CASE_BODY, /* "this command is inside CASE" */
	RES_ESAC  ,
#पूर्ण_अगर
	RES_XXXX  ,
	RES_SNTX
};

प्रकार काष्ठा o_string {
	अक्षर *तथ्य;
	पूर्णांक लंबाई; /* position where data is appended */
	पूर्णांक maxlen;
	पूर्णांक o_expflags;
	/* At least some part of the string was inside '' or "",
	 * possibly empty one: word"", wo''rd etc. */
	smallint has_quoted_part;
	smallint has_empty_slot;
	smallint ended_in_ifs;
} o_string;
क्रमागत {
	EXP_FLAG_SINGLEWORD     = 0x80, /* must be 0x80 */
	EXP_FLAG_GLOB           = 0x2,
	/* Protect newly added chars against globbing
	 * by prepending \ to *, ?, [, \ */
	EXP_FLAG_ESC_GLOB_CHARS = 0x1,
};
/* Used for initialization: o_string foo = NULL_O_STRING; */
#घोषणा NULL_O_STRING { शून्य }

#अगर_अघोषित debug_printf_parse
अटल स्थिर अक्षर *स्थिर assignment_flag[] ALIGN_PTR = {
	"MAYBE_ASSIGNMENT",
	"DEFINITELY_ASSIGNMENT",
	"NOT_ASSIGNMENT",
	"WORD_IS_KEYWORD",
};
#पूर्ण_अगर

/* We almost can use standard FILE api, but we need an ability to move
 * its fd when redirects coincide with it. No api exists for that
 * (RFE for it at https://sourceware.org/bugzilla/show_bug.cgi?id=21902).
 * HFILE is our internal alternative. Only supports reading.
 * Since we now can, we incorporate linked list of all opened HFILEs
 * into the struct (used to be a separate mini-list).
 */
प्रकार काष्ठा HFILE {
	अक्षर *cur;
	अक्षर *end;
	काष्ठा HFILE *अग्र_hfile;
	पूर्णांक fd;
	अक्षर buf[1024];
} HFILE;

प्रकार काष्ठा in_str {
	स्थिर अक्षर *p;
	पूर्णांक peek_buf[2];
	पूर्णांक last_char;
	HFILE *file;
} in_str;

#अगर_अघोषित __U_BOOT__
/* The descrip member of this structure is only used to make
 * debugging output pretty */
अटल स्थिर काष्ठा {
	int32_t mode;
	चिन्हित अक्षर default_fd;
	अक्षर descrip[3];
} redir_table[] ALIGN4 = {
	{ O_RDONLY,                  0, "<"  },
	{ O_CREAT|O_TRUNC|O_WRONLY,  1, ">"  },
	{ O_CREAT|O_APPEND|O_WRONLY, 1, ">>" },
	{ O_CREAT|O_RDWR,            1, "<>" },
	{ O_RDONLY,                  0, "<<" },
/* Should not be needed. Bogus default_fd helps in debugging */
/*	{ O_RDONLY,                 77, "<<" }, */
};

काष्ठा redir_struct {
	काष्ठा redir_struct *अग्र;
	अक्षर *rd_filename;          /* filename */
	पूर्णांक rd_fd;                  /* fd to redirect */
	/* fd to redirect to, or -3 if rd_fd is to be closed (n>&-) */
	पूर्णांक rd_dup;
	smallint rd_type;           /* (enum redir_type) */
	/* note: for heredocs, rd_filename contains heredoc delimiter,
	 * and subsequently heredoc itself; and rd_dup is a bitmask:
	 * bit 0: do we need to trim leading tabs?
	 * bit 1: is heredoc quoted (<<'delim' syntax) ?
	 */
};
प्रकार क्रमागत redir_type {
	REDIRECT_निवेश     = 0,
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

#पूर्ण_अगर /* !__U_BOOT__ */

काष्ठा command {
#अगर_अघोषित __U_BOOT__
	pid_t pid;                  /* 0 if exited */
#पूर्ण_अगर /* !__U_BOOT__ */
	अचिन्हित assignment_cnt;    /* how many argv[i] are assignments? */
#अगर ENABLE_HUSH_LINENO_VAR
	अचिन्हित lineno;
#पूर्ण_अगर
	smallint cmd_type;          /* CMD_xxx */
#घोषणा CMD_NORMAL   0
#घोषणा CMD_SUBSHELL 1
#अगर BASH_TEST2
/* used for "[[ EXPR ]]" */
# define CMD_TEST2_SINGLEWORD_NOGLOB 2
#पूर्ण_अगर
#अगर BASH_TEST2 || ENABLE_HUSH_LOCAL || ENABLE_HUSH_EXPORT || ENABLE_HUSH_READONLY
/* used to prevent word splitting and globbing in "export v=t*" */
# define CMD_SINGLEWORD_NOGLOB 3
#पूर्ण_अगर
#अगर ENABLE_HUSH_FUNCTIONS
# define CMD_FUNCDEF 4
#पूर्ण_अगर

	smalluint cmd_exitcode;
	/* if non-NULL, this "command" is { list }, ( list ), or a compound statement */
	काष्ठा pipe *group;
#अगर !BB_MMU
	अक्षर *group_as_string;
#पूर्ण_अगर
#अगर ENABLE_HUSH_FUNCTIONS
	काष्ठा function *child_func;
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
#पूर्ण_अगर
#अगर_घोषित __U_BOOT__
	पूर्णांक argc; /* number of program arguments */
#पूर्ण_अगर
	अक्षर **argv;                /* command name and arguments */
/* argv vector may contain variable references (^Cvar^C, ^C0^C etc)
 * and on execution these are substituted with their values.
 * Substitution can make _several_ words out of one argv[n]!
 * Example: argv[0]=='.^C*^C.' here: echo .$*.
 * References of the form ^C`cmd arg^C are `cmd arg` substitutions.
 */
#अगर_अघोषित __U_BOOT__
	काष्ठा redir_struct *redirects; /* I/O redirections */
#पूर्ण_अगर /* !__U_BOOT__ */
};
/* Is there anything in this command at all? */
#अगर_अघोषित __U_BOOT__
#घोषणा IS_NULL_CMD(cmd) \
	(!(cmd)->group && !(cmd)->argv && !(cmd)->redirects)

#अन्यथा /* __U_BOOT__ */
#घोषणा IS_NULL_CMD(cmd) \
	(!(cmd)->group && !(cmd)->argv)
#पूर्ण_अगर /* __U_BOOT__ */
काष्ठा pipe {
	काष्ठा pipe *अग्र;
	पूर्णांक अंक_cmds;               /* total number of commands in pipe */
#अगर_अघोषित __U_BOOT__
	पूर्णांक alive_cmds;             /* number of commands running (not exited) */
	पूर्णांक stopped_cmds;           /* number of commands alive, but stopped */
#अगर ENABLE_HUSH_JOB
	अचिन्हित jobid;             /* job number */
	pid_t pgrp;                 /* process group ID for the job */
	अक्षर *cmdtext;              /* name of job */
#पूर्ण_अगर
#पूर्ण_अगर /* !__U_BOOT__ */
	काष्ठा command *cmds;       /* array of commands in pipe */
	smallint followup;          /* PIPE_BG, PIPE_SEQ, PIPE_OR, PIPE_AND */
	IF_HAS_KEYWORDS(smallint pi_inverted;) /* "! cmd | cmd" */
	IF_HAS_KEYWORDS(smallint res_word;) /* needed for if, for, while, until... */
};
प्रकार क्रमागत pipe_style {
	PIPE_SEQ = 0,
	PIPE_AND = 1,
	PIPE_OR  = 2,
	PIPE_BG  = 3,
} pipe_style;
/* Is there anything in this pipe at all? */
#घोषणा IS_NULL_PIPE(pi) \
	((pi)->अंक_cmds == 0 IF_HAS_KEYWORDS( && (pi)->res_word == RES_NONE))

/* This holds pointers to the various results of parsing */
काष्ठा parse_context {
	/* linked list of pipes */
	काष्ठा pipe *list_शीर्ष;
	/* last pipe (being constructed right now) */
	काष्ठा pipe *pipe;
	/* last command in pipe (being constructed right now) */
	काष्ठा command *command;
#अगर_अघोषित __U_BOOT__
	/* last redirect in command->redirects list */
	काष्ठा redir_struct *pending_redirect;
#पूर्ण_अगर /* !__U_BOOT__ */
	o_string word;
#अगर !BB_MMU
	o_string as_string;
#पूर्ण_अगर
	smallint is_assignment; /* 0:maybe, 1:yes, 2:no, 3:keyword */
#अगर HAS_KEYWORDS
	smallint ctx_res_w;
	smallint ctx_inverted; /* "! cmd | cmd" */
#अगर ENABLE_HUSH_CASE
	smallint ctx_dsemicolon; /* ";;" seen */
#पूर्ण_अगर
	/* bitmask of FLAG_xxx, for figuring out valid reserved words */
	पूर्णांक old_flag;
	/* group we are enclosed in:
	 * example: "if pipe1; pipe2; then pipe3; fi"
	 * when we see "if" or "then", we malloc and copy current context,
	 * and make ->stack point to it. then we parse pipeN.
	 * when closing "then" / fi" / whatever is found,
	 * we move list_head into ->stack->command->group,
	 * copy ->stack into current context, and delete ->stack.
	 * (parsing of { list } and ( list ) doesn't use this method)
	 */
	काष्ठा parse_context *ढेर;
#पूर्ण_अगर
};
क्रमागत {
	MAYBE_ASSIGNMENT      = 0,
	DEFINITELY_ASSIGNMENT = 1,
	NOT_ASSIGNMENT        = 2,
	/* Not an assignment, but next word may be: "if v=xyz cmd;" */
	WORD_IS_KEYWORD       = 3,
};

#अगर_अघोषित __U_BOOT__
/* On program start, environ points to initial environment.
 * putenv adds new pointers into it, unsetenv removes them.
 * Neither of these (de)allocates the strings.
 * setenv allocates new strings in malloc space and does putenv,
 * and thus setenv is unusable (leaky) for shell's purposes */
#घोषणा setenv(...) setenv_is_leaky_dont_use()
#पूर्ण_अगर /* !__U_BOOT__ */
काष्ठा variable {
	काष्ठा variable *अग्र;
	अक्षर *varstr;        /* points to "name=" portion */
	पूर्णांक max_len;         /* if > 0, name is part of initial env; else name is malloced */
#अगर_अघोषित __U_BOOT__
	uint16_t var_nest_level;
	smallint flg_export; /* putenv should be done on this var */
	smallint flg_read_only;
#पूर्ण_अगर /* !__U_BOOT__ */
};

क्रमागत {
	BC_BREAK = 1,
	BC_CONTINUE = 2,
};

#अगर ENABLE_HUSH_FUNCTIONS
काष्ठा function {
	काष्ठा function *अग्र;
	अक्षर *name;
	काष्ठा command *parent_cmd;
	काष्ठा pipe *body;
# अगर !BB_MMU
	अक्षर *body_as_string;
# endif
};
#पूर्ण_अगर

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
अटल स्थिर अक्षर o_opt_strings[] ALIGN1 =
	"pipefail\0"
	"noexec\0"
	"errexit\0"
#अगर ENABLE_HUSH_MODE_X
	"xtrace\0"
#पूर्ण_अगर
	;
क्रमागत {
	OPT_O_PIPEFAIL,
	OPT_O_NOEXEC,
	OPT_O_ERREXIT,
#अगर ENABLE_HUSH_MODE_X
	OPT_O_XTRACE,
#पूर्ण_अगर
	अंक_OPT_O
};

/* "Globals" within this file */
/* Sorted roughly by size (smaller offsets == smaller code) */
काष्ठा globals {
#अगर_अघोषित __U_BOOT__
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
#अगर ENABLE_HUSH_INTERACTIVE
	/* 'interactive_fd' is a fd# open to ctty, if we have one
	 * _AND_ if we decided to act interactively */
	पूर्णांक interactive_fd;
	IF_NOT_FEATURE_EDITING_FANCY_PROMPT(अक्षर *PS1;)
# define G_interactive_fd (G.interactive_fd)
#अन्यथा
# define G_interactive_fd 0
#पूर्ण_अगर
#अन्यथा /* __U_BOOT__ */
# define G_interactive_fd 0
#पूर्ण_अगर /* __U_BOOT__ */
#अगर_अघोषित __U_BOOT__
#अगर ENABLE_FEATURE_EDITING
	पंक्ति_निवेश_t *पंक्ति_निवेश_state;
#पूर्ण_अगर
	pid_t मूल_pid;
	pid_t मूल_ppid;
	pid_t last_bg_pid;
#अगर ENABLE_HUSH_RANDOM_SUPPORT
	random_t random_gen;
#पूर्ण_अगर
#अगर ENABLE_HUSH_JOB
	पूर्णांक run_list_level;
	अचिन्हित last_jobid;
	pid_t saved_tty_pgrp;
	काष्ठा pipe *job_list;
# define G_saved_tty_pgrp (G.saved_tty_pgrp)
#अन्यथा
# define G_saved_tty_pgrp 0
#पूर्ण_अगर
#पूर्ण_अगर /* !__U_BOOT__ */
	/* How deeply are we in context where "set -e" is ignored */
	पूर्णांक errexit_depth;
#अगर_अघोषित __U_BOOT__
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

	अक्षर o_opt[अंक_OPT_O];
#अगर ENABLE_HUSH_MODE_X
# define G_x_mode (G.o_opt[OPT_O_XTRACE])
#अन्यथा
# define G_x_mode 0
#पूर्ण_अगर
	अक्षर opt_s;
	अक्षर opt_c;
#पूर्ण_अगर /* !__U_BOOT__ */
#अगर ENABLE_HUSH_INTERACTIVE
	smallint promptmode; /* 0: PS1, 1: PS2 */
#पूर्ण_अगर
	/* set by signal handler if SIGINT is received _and_ its trap is not set */
	smallint flag_SIGINT;
#अगर_अघोषित __U_BOOT__
#अगर ENABLE_HUSH_LOOPS
	smallint flag_break_continue;
#पूर्ण_अगर
#पूर्ण_अगर /* !__U_BOOT__ */
#अगर ENABLE_HUSH_FUNCTIONS
	/* 0: outside of a function (or sourced file)
	 * -1: inside of a function, ok to use return builtin
	 * 1: return is invoked, skip all till end of func
	 */
	smallint flag_return_in_progress;
# define G_flag_return_in_progress (G.flag_return_in_progress)
#अन्यथा
# define G_flag_return_in_progress 0
#पूर्ण_अगर
	smallint exiting; /* used to prevent EXIT trap recursion */
	/* These support $? */
	smalluint last_exitcode;
	smalluint expand_exitcode;
	smalluint last_bg_pid_exitcode;
#अगर_अघोषित __U_BOOT__
#अगर ENABLE_HUSH_SET
	/* are global_argv and global_argv[1..n] malloced? (note: not [0]) */
	smalluint global_args_malloced;
# define G_global_args_malloced (G.global_args_malloced)
#अन्यथा
# define G_global_args_malloced 0
#पूर्ण_अगर
#अगर ENABLE_HUSH_BASH_COMPAT
	पूर्णांक dead_job_exitcode; /* for "wait -n" */
#पूर्ण_अगर
#पूर्ण_अगर /* !__U_BOOT__ */
	/* how many non-NULL argv's we have. NB: $# + 1 */
	पूर्णांक global_argc;
	अक्षर **global_argv;
#अगर !BB_MMU
	अक्षर *argv0_for_re_execing;
#पूर्ण_अगर
#अगर ENABLE_HUSH_LOOPS
#अगर_अघोषित __U_BOOT__
	अचिन्हित depth_break_continue;
#पूर्ण_अगर /* !__U_BOOT__ */
	अचिन्हित depth_of_loop;
#पूर्ण_अगर
#अगर_अघोषित __U_BOOT__
#अगर ENABLE_HUSH_GETOPTS
	अचिन्हित getopt_गिनती;
#पूर्ण_अगर
#पूर्ण_अगर /* !__U_BOOT__ */
	स्थिर अक्षर *ifs;
#अगर_घोषित __U_BOOT__
	पूर्णांक flag_repeat;
	पूर्णांक do_repeat;
	पूर्णांक run_command_flags;
#पूर्ण_अगर /* __U_BOOT__ */
	अक्षर *ifs_whitespace; /* = G.ifs or malloced */
#अगर_अघोषित __U_BOOT__
	स्थिर अक्षर *cwd;
#पूर्ण_अगर /* !__U_BOOT__ */
	काष्ठा variable *शीर्ष_var;
	अक्षर **expanded_assignments;
	काष्ठा variable **shadowed_vars_pp;
	अचिन्हित var_nest_level;
#अगर_अघोषित __U_BOOT__
#अगर ENABLE_HUSH_FUNCTIONS
# अगर ENABLE_HUSH_LOCAL
	अचिन्हित func_nest_level; /* solely to prevent "local v" in non-functions */
# endif
	काष्ठा function *शीर्ष_func;
#पूर्ण_अगर
	/* Signal and trap handling */
#अगर ENABLE_HUSH_FAST
	अचिन्हित गिनती_SIGCHLD;
	अचिन्हित handled_SIGCHLD;
	smallint we_have_children;
#पूर्ण_अगर
#अगर ENABLE_HUSH_LINENO_VAR
	अचिन्हित parse_lineno;
	अचिन्हित execute_lineno;
#पूर्ण_अगर
	HFILE *HFILE_list;
	HFILE *HFILE_stdin;
	/* Which signals have non-DFL handler (even with no traps set)?
	 * Set at the start to:
	 * (SIGQUIT + maybe SPECIAL_INTERACTIVE_SIGS + maybe SPECIAL_JOBSTOP_SIGS)
	 * SPECIAL_INTERACTIVE_SIGS are cleared after fork.
	 * The rest is cleared right before execv syscalls.
	 * Other than these two times, never modified.
	 */
	अचिन्हित special_sig_mask;
#अगर ENABLE_HUSH_JOB
	अचिन्हित fatal_sig_mask;
# define G_fatal_sig_mask (G.fatal_sig_mask)
#अन्यथा
# define G_fatal_sig_mask 0
#पूर्ण_अगर
#अगर ENABLE_HUSH_TRAP
	पूर्णांक pre_trap_exitcode;
# अगर ENABLE_HUSH_FUNCTIONS
	पूर्णांक return_exitcode;
# endif
	अक्षर **traps; /* char *traps[NSIG] */
# define G_traps G.traps
#अन्यथा
# define G_traps ((अक्षर**)शून्य)
#पूर्ण_अगर
	sigset_t pending_set;
#अगर ENABLE_HUSH_MEMLEAK
	अचिन्हित दीर्घ memleak_मूल्य;
#पूर्ण_अगर
#अगर ENABLE_HUSH_MODE_X
	अचिन्हित x_mode_depth;
	/* "set -x" output should not be redirectable with subsequent 2>FILE.
	 * We dup fd#2 to x_mode_fd when "set -x" is executed, and use it
	 * for all subsequent output.
	 */
	पूर्णांक x_mode_fd;
	o_string x_mode_buf;
#पूर्ण_अगर
#पूर्ण_अगर /* !__U_BOOT__ */
#अगर HUSH_DEBUG >= 2
	पूर्णांक debug_indent;
#पूर्ण_अगर
#अगर_अघोषित __U_BOOT__
	काष्ठा sigaction sa;
	अक्षर optstring_buf[माप("eixcs")];
#अगर BASH_EPOCH_VARS
	अक्षर epoch_buf[माप("%llu.nnnnnn") + माप(दीर्घ दीर्घ)*3];
#पूर्ण_अगर
#अगर ENABLE_FEATURE_EDITING
	अक्षर user_निवेश_buf[CONFIG_FEATURE_EDITING_MAX_LEN];
#पूर्ण_अगर
#पूर्ण_अगर /* !__U_BOOT__ */
};
#अगर_घोषित __U_BOOT__
काष्ठा globals *ptr_to_globals;
#पूर्ण_अगर /* __U_BOOT__ */
#घोषणा G (*ptr_to_globals)
/* Not #defining name to G.name - this quickly gets unwieldy
 * (too many defines). Also, I actually prefer to see when a variable
 * is global, thus "G." prefix is a useful hint */
#अगर_घोषित __U_BOOT__
#घोषणा SET_PTR_TO_GLOBALS(x) करो { \
	(*(काष्ठा globals**)&ptr_to_globals) = (व्योम*)(x); \
	barrier(); \
} जबतक (0)
#घोषणा INIT_G() करो { \
	SET_PTR_TO_GLOBALS(xzalloc(माप(G))); \
	G.promptmode = 1; \
} जबतक (0)
#अन्यथा /* !__U_BOOT__ */
#घोषणा INIT_G() करो { \
	SET_PTR_TO_GLOBALS(xzalloc(माप(G))); \
	/* memset(&G.sa, 0, sizeof(G.sa)); */  \
	sigfillset(&G.sa.sa_mask); \
	G.sa.sa_flags = SA_RESTART; \
} जबतक (0)
#पूर्ण_अगर /* !__U_BOOT__ */

#अगर_अघोषित __U_BOOT__
/* Function prototypes for builtins */
अटल पूर्णांक builtin_cd(अक्षर **argv) FAST_FUNC;
#अगर ENABLE_HUSH_ECHO
अटल पूर्णांक builtin_echo(अक्षर **argv) FAST_FUNC;
#पूर्ण_अगर
अटल पूर्णांक builtin_eval(अक्षर **argv) FAST_FUNC;
अटल पूर्णांक builtin_exec(अक्षर **argv) FAST_FUNC;
अटल पूर्णांक builtin_exit(अक्षर **argv) FAST_FUNC;
#अगर ENABLE_HUSH_EXPORT
अटल पूर्णांक builtin_export(अक्षर **argv) FAST_FUNC;
#पूर्ण_अगर
#अगर ENABLE_HUSH_READONLY
अटल पूर्णांक builtin_readonly(अक्षर **argv) FAST_FUNC;
#पूर्ण_अगर
अटल पूर्णांक builtin_false(अक्षर **argv) FAST_FUNC;
#अगर ENABLE_HUSH_JOB
अटल पूर्णांक builtin_fg_bg(अक्षर **argv) FAST_FUNC;
अटल पूर्णांक builtin_jobs(अक्षर **argv) FAST_FUNC;
#पूर्ण_अगर
#अगर ENABLE_HUSH_GETOPTS
अटल पूर्णांक builtin_getopts(अक्षर **argv) FAST_FUNC;
#पूर्ण_अगर
#अगर ENABLE_HUSH_HELP
अटल पूर्णांक builtin_help(अक्षर **argv) FAST_FUNC;
#पूर्ण_अगर
#अगर MAX_HISTORY && ENABLE_FEATURE_EDITING
अटल पूर्णांक builtin_history(अक्षर **argv) FAST_FUNC;
#पूर्ण_अगर
#अगर ENABLE_HUSH_LOCAL
अटल पूर्णांक builtin_local(अक्षर **argv) FAST_FUNC;
#पूर्ण_अगर
#अगर ENABLE_HUSH_MEMLEAK
अटल पूर्णांक builtin_memleak(अक्षर **argv) FAST_FUNC;
#पूर्ण_अगर
#अगर ENABLE_HUSH_PRINTF
अटल पूर्णांक builtin_printf(अक्षर **argv) FAST_FUNC;
#पूर्ण_अगर
अटल पूर्णांक builtin_pwd(अक्षर **argv) FAST_FUNC;
#अगर ENABLE_HUSH_READ
अटल पूर्णांक builtin_read(अक्षर **argv) FAST_FUNC;
#पूर्ण_अगर
#अगर ENABLE_HUSH_SET
अटल पूर्णांक builtin_set(अक्षर **argv) FAST_FUNC;
#पूर्ण_अगर
अटल पूर्णांक builtin_shift(अक्षर **argv) FAST_FUNC;
अटल पूर्णांक builtin_source(अक्षर **argv) FAST_FUNC;
#अगर ENABLE_HUSH_TEST || BASH_TEST2
अटल पूर्णांक builtin_test(अक्षर **argv) FAST_FUNC;
#पूर्ण_अगर
#अगर ENABLE_HUSH_TRAP
अटल पूर्णांक builtin_trap(अक्षर **argv) FAST_FUNC;
#पूर्ण_अगर
#अगर ENABLE_HUSH_TYPE
अटल पूर्णांक builtin_type(अक्षर **argv) FAST_FUNC;
#पूर्ण_अगर
#अगर ENABLE_HUSH_TIMES
अटल पूर्णांक builtin_times(अक्षर **argv) FAST_FUNC;
#पूर्ण_अगर
अटल पूर्णांक builtin_true(अक्षर **argv) FAST_FUNC;
#अगर ENABLE_HUSH_UMASK
अटल पूर्णांक builtin_umask(अक्षर **argv) FAST_FUNC;
#पूर्ण_अगर
#अगर ENABLE_HUSH_UNSET
अटल पूर्णांक builtin_unset(अक्षर **argv) FAST_FUNC;
#पूर्ण_अगर
#अगर ENABLE_HUSH_KILL
अटल पूर्णांक builtin_kill(अक्षर **argv) FAST_FUNC;
#पूर्ण_अगर
#अगर ENABLE_HUSH_WAIT
अटल पूर्णांक builtin_wait(अक्षर **argv) FAST_FUNC;
#पूर्ण_अगर
#अगर ENABLE_HUSH_LOOPS
अटल पूर्णांक builtin_break(अक्षर **argv) FAST_FUNC;
अटल पूर्णांक builtin_continue(अक्षर **argv) FAST_FUNC;
#पूर्ण_अगर
#अगर ENABLE_HUSH_FUNCTIONS
अटल पूर्णांक builtin_return(अक्षर **argv) FAST_FUNC;
#पूर्ण_अगर

/* Table of built-in functions.  They can be forked or not, depending on
 * context: within pipes, they fork.  As simple commands, they do not.
 * When used in non-forking context, they can change global variables
 * in the parent shell process.  If forked, of course they cannot.
 * For example, 'unset foo | whatever' will parse and run, but foo will
 * still be set at the end. */
काष्ठा built_in_command {
	स्थिर अक्षर *b_cmd;
	पूर्णांक (*b_function)(अक्षर **argv) FAST_FUNC;
#अगर ENABLE_HUSH_HELP
	स्थिर अक्षर *b_descr;
# define BLTIN(cmd, func, help) { cmd, func, help }
#अन्यथा
# define BLTIN(cmd, func, help) { cmd, func }
#पूर्ण_अगर
};

अटल स्थिर काष्ठा built_in_command bltins1[] ALIGN_PTR = {
	BLTIN("."        , builtin_source  , "Run commands in file"),
	BLTIN(":"        , builtin_true    , शून्य),
#अगर ENABLE_HUSH_JOB
	BLTIN("bg"       , builtin_fg_bg   , "Resume job in background"),
#पूर्ण_अगर
#अगर ENABLE_HUSH_LOOPS
	BLTIN("break"    , builtin_break   , "Exit loop"),
#पूर्ण_अगर
	BLTIN("cd"       , builtin_cd      , "Change directory"),
#अगर ENABLE_HUSH_LOOPS
	BLTIN("continue" , builtin_continue, "Start new loop iteration"),
#पूर्ण_अगर
	BLTIN("eval"     , builtin_eval    , "Construct and run shell command"),
	BLTIN("exec"     , builtin_exec    , "Execute command, don't return to shell"),
	BLTIN("exit"     , builtin_exit    , शून्य),
#अगर ENABLE_HUSH_EXPORT
	BLTIN("export"   , builtin_export  , "Set environment variables"),
#पूर्ण_अगर
	BLTIN("false"    , builtin_false   , शून्य),
#अगर ENABLE_HUSH_JOB
	BLTIN("fg"       , builtin_fg_bg   , "Bring job to foreground"),
#पूर्ण_अगर
#अगर ENABLE_HUSH_GETOPTS
	BLTIN("getopts"  , builtin_getopts , शून्य),
#पूर्ण_अगर
#अगर ENABLE_HUSH_HELP
	BLTIN("help"     , builtin_help    , शून्य),
#पूर्ण_अगर
#अगर MAX_HISTORY && ENABLE_FEATURE_EDITING
	BLTIN("history"  , builtin_history , "Show history"),
#पूर्ण_अगर
#अगर ENABLE_HUSH_JOB
	BLTIN("jobs"     , builtin_jobs    , "List jobs"),
#पूर्ण_अगर
#अगर ENABLE_HUSH_KILL
	BLTIN("kill"     , builtin_kill    , "Send signals to processes"),
#पूर्ण_अगर
#अगर ENABLE_HUSH_LOCAL
	BLTIN("local"    , builtin_local   , "Set local variables"),
#पूर्ण_अगर
#अगर ENABLE_HUSH_MEMLEAK
	BLTIN("memleak"  , builtin_memleak , शून्य),
#पूर्ण_अगर
#अगर ENABLE_HUSH_READ
	BLTIN("read"     , builtin_read    , "Input into variable"),
#पूर्ण_अगर
#अगर ENABLE_HUSH_READONLY
	BLTIN("readonly" , builtin_readonly, "Make variables read-only"),
#पूर्ण_अगर
#अगर ENABLE_HUSH_FUNCTIONS
	BLTIN("return"   , builtin_return  , "Return from function"),
#पूर्ण_अगर
#अगर ENABLE_HUSH_SET
	BLTIN("set"      , builtin_set     , "Set positional parameters"),
#पूर्ण_अगर
	BLTIN("shift"    , builtin_shift   , "Shift positional parameters"),
#अगर BASH_SOURCE
	BLTIN("source"   , builtin_source  , शून्य),
#पूर्ण_अगर
#अगर ENABLE_HUSH_TIMES
	BLTIN("times"    , builtin_times   , शून्य),
#पूर्ण_अगर
#अगर ENABLE_HUSH_TRAP
	BLTIN("trap"     , builtin_trap    , "Trap signals"),
#पूर्ण_अगर
	BLTIN("true"     , builtin_true    , शून्य),
#अगर ENABLE_HUSH_TYPE
	BLTIN("type"     , builtin_type    , "Show command type"),
#पूर्ण_अगर
#अगर ENABLE_HUSH_ULIMIT
	BLTIN("ulimit"   , shell_builtin_ulimit, "Control resource limits"),
#पूर्ण_अगर
#अगर ENABLE_HUSH_UMASK
	BLTIN("umask"    , builtin_umask   , "Set file creation mask"),
#पूर्ण_अगर
#अगर ENABLE_HUSH_UNSET
	BLTIN("unset"    , builtin_unset   , "Unset variables"),
#पूर्ण_अगर
#अगर ENABLE_HUSH_WAIT
	BLTIN("wait"     , builtin_wait    , "Wait for process to finish"),
#पूर्ण_अगर
};
/* These builtins won't be used if we are on NOMMU and need to re-exec
 * (it's cheaper to run an external program in this case):
 */
अटल स्थिर काष्ठा built_in_command bltins2[] ALIGN_PTR = {
#अगर ENABLE_HUSH_TEST
	BLTIN("["        , builtin_test    , शून्य),
#पूर्ण_अगर
#अगर BASH_TEST2
	BLTIN("[["       , builtin_test    , शून्य),
#पूर्ण_अगर
#अगर ENABLE_HUSH_ECHO
	BLTIN("echo"     , builtin_echo    , शून्य),
#पूर्ण_अगर
#अगर ENABLE_HUSH_PRINTF
	BLTIN("printf"   , builtin_printf  , शून्य),
#पूर्ण_अगर
	BLTIN("pwd"      , builtin_pwd     , शून्य),
#अगर ENABLE_HUSH_TEST
	BLTIN("test"     , builtin_test    , शून्य),
#पूर्ण_अगर
};

#पूर्ण_अगर /* !__U_BOOT__ */

/* Debug printouts.
 */
#अगर HUSH_DEBUG >= 2
/* prevent disasters with G.debug_indent < 0 */
# define indent() fdprintf(2, "%*s", (G.debug_indent * 2) & 0xff, "")
# define debug_enter() (G.debug_indent++)
# define debug_leave() (G.debug_indent--)
#अन्यथा
# define indent()      ((व्योम)0)
# define debug_enter() ((व्योम)0)
# define debug_leave() ((व्योम)0)
#पूर्ण_अगर

#अगर_अघोषित debug_printf
# define debug_printf(...) (indent(), fdprintf(2, __VA_ARGS__))
#पूर्ण_अगर

#अगर_अघोषित debug_printf_parse
# define debug_printf_parse(...) (indent(), fdprintf(2, __VA_ARGS__))
#पूर्ण_अगर

#अगर_अघोषित debug_printf_heredoc
# define debug_printf_heredoc(...) (indent(), fdprintf(2, __VA_ARGS__))
#पूर्ण_अगर

#अगर_अघोषित debug_printf_exec
#घोषणा debug_printf_exec(...) (indent(), fdprintf(2, __VA_ARGS__))
#पूर्ण_अगर

#अगर_अघोषित debug_printf_env
# define debug_printf_env(...) (indent(), fdprintf(2, __VA_ARGS__))
#पूर्ण_अगर

#अगर_अघोषित debug_printf_jobs
# define debug_printf_jobs(...) (indent(), fdprintf(2, __VA_ARGS__))
# define DEBUG_JOBS 1
#अन्यथा
# define DEBUG_JOBS 0
#पूर्ण_अगर

#अगर_अघोषित debug_printf_expand
# define debug_printf_expand(...) (indent(), fdprintf(2, __VA_ARGS__))
# define DEBUG_EXPAND 1
#अन्यथा
# define DEBUG_EXPAND 0
#पूर्ण_अगर

#अगर_अघोषित debug_printf_varexp
# define debug_printf_varexp(...) (indent(), fdprintf(2, __VA_ARGS__))
#पूर्ण_अगर

#अगर_अघोषित debug_printf_glob
# define debug_printf_glob(...) (indent(), fdprintf(2, __VA_ARGS__))
# define DEBUG_GLOB 1
#अन्यथा
# define DEBUG_GLOB 0
#पूर्ण_अगर

#अगर_अघोषित debug_printf_redir
# define debug_printf_redir(...) (indent(), fdprintf(2, __VA_ARGS__))
#पूर्ण_अगर

#अगर_अघोषित debug_printf_list
# define debug_printf_list(...) (indent(), fdprintf(2, __VA_ARGS__))
#पूर्ण_अगर

#अगर_अघोषित debug_printf_subst
# define debug_printf_subst(...) (indent(), fdprintf(2, __VA_ARGS__))
#पूर्ण_अगर

#अगर_अघोषित debug_printf_prompt
# define debug_printf_prompt(...) (indent(), fdprintf(2, __VA_ARGS__))
#पूर्ण_अगर

#अगर_अघोषित debug_printf_clean
# define debug_printf_clean(...) (indent(), fdprintf(2, __VA_ARGS__))
# define DEBUG_CLEAN 1
#अन्यथा
# define DEBUG_CLEAN 0
#पूर्ण_अगर

#अगर DEBUG_EXPAND
अटल व्योम debug_print_strings(स्थिर अक्षर *prefix, अक्षर **vv)
{
	indent();
	fdprintf(2, "%s:\n", prefix);
	जबतक (*vv)
		fdprintf(2, " '%s'\n", *vv++);
}
#अन्यथा
# define debug_print_strings(prefix, vv) ((व्योम)0)
#पूर्ण_अगर

/* Leak hunting. Use hush_leaktool.sh for post-processing.
 */
#अगर LEAK_HUNTING
अटल व्योम *xxmalloc(पूर्णांक lineno, माप_प्रकार आकार)
{
	व्योम *ptr = xmalloc((आकार + 0xff) & ~0xff);
	fdprintf(2, "line %d: malloc %p\n", lineno, ptr);
	वापस ptr;
}
अटल व्योम *xxrealloc(पूर्णांक lineno, व्योम *ptr, माप_प्रकार आकार)
{
	ptr = xrealloc(ptr, (आकार + 0xff) & ~0xff);
	fdprintf(2, "line %d: realloc %p\n", lineno, ptr);
	वापस ptr;
}
अटल अक्षर *xxstrdup(पूर्णांक lineno, स्थिर अक्षर *str)
{
	अक्षर *ptr = xstrdup(str);
	fdprintf(2, "line %d: strdup %p\n", lineno, ptr);
	वापस ptr;
}
अटल व्योम xxfree(व्योम *ptr)
{
	fdprintf(2, "free %p\n", ptr);
	मुक्त(ptr);
}
# define xmalloc(s)     xxmalloc(__LINE__, s)
# define xrealloc(p, s) xxrealloc(__LINE__, p, s)
# define xstrdup(s)     xxstrdup(__LINE__, s)
# define मुक्त(p)        xxfree(p)
#पूर्ण_अगर

/* Syntax and runtime errors. They always abort scripts.
 * In interactive use they usually discard unparsed and/or unexecuted commands
 * and return to the prompt.
 * HUSH_DEBUG >= 2 prints line number in this file where it was detected.
 */
#अगर HUSH_DEBUG < 2
# define संदेश_and_die_if_script(lineno, ...)     संदेश_and_die_if_script(__VA_ARGS__)
# define syntax_error(lineno, संदेश)              syntax_error(संदेश)
# define syntax_error_at(lineno, संदेश)           syntax_error_at(संदेश)
# define syntax_error_unterm_ch(lineno, ch)     syntax_error_unterm_ch(ch)
# define syntax_error_unterm_str(lineno, s)     syntax_error_unterm_str(s)
# define syntax_error_unexpected_ch(lineno, ch) syntax_error_unexpected_ch(ch)
#पूर्ण_अगर

अटल व्योम die_if_script(व्योम)
{
	अगर (!G_interactive_fd) {
		अगर (G.last_exitcode) /* sometines it's 2, not 1 (bash compat) */
			xfunc_error_retval = G.last_exitcode;
		xfunc_die();
	}
}

#अगर_घोषित __U_BOOT__
अटल व्योम __maybe_unused संदेश_and_die_if_script(अचिन्हित lineno, स्थिर अक्षर *fmt, ...)
#अन्यथा /* !__U_BOOT__ */
अटल व्योम संदेश_and_die_if_script(अचिन्हित lineno, स्थिर अक्षर *fmt, ...)
#पूर्ण_अगर /* !__U_BOOT__ */
{
	बहु_सूची p;

#अगर HUSH_DEBUG >= 2
	bb_error_संदेश("hush.c:%u", lineno);
#पूर्ण_अगर
	बहु_शुरू(p, fmt);
	bb_verror_संदेश(fmt, p, शून्य);
	बहु_पूर्ण(p);
	die_if_script();
}

अटल व्योम syntax_error(अचिन्हित lineno UNUSED_PARAM, स्थिर अक्षर *संदेश)
{
	अगर (संदेश)
		bb_error_संदेश("syntax error: %s", संदेश);
	अन्यथा
		bb_simple_error_संदेश("syntax error");
	die_if_script();
}

अटल व्योम syntax_error_at(अचिन्हित lineno UNUSED_PARAM, स्थिर अक्षर *संदेश)
{
	bb_error_संदेश("syntax error at '%s'", संदेश);
	die_if_script();
}

अटल व्योम syntax_error_unterm_str(अचिन्हित lineno UNUSED_PARAM, स्थिर अक्षर *s)
{
	bb_error_संदेश("syntax error: unterminated %s", s);
//? source4.tests fails: in bash, echo ${^} in script does not terminate the script
// (but bash --posix, or if bash is run as "sh", does terminate in script, so maybe uncomment this?)
//	die_if_script();
}

अटल व्योम syntax_error_unterm_ch(अचिन्हित lineno, अक्षर ch)
{
	अक्षर संदेश[2] = { ch, '\0' };
	syntax_error_unterm_str(lineno, संदेश);
}

अटल व्योम syntax_error_unexpected_ch(अचिन्हित lineno UNUSED_PARAM, पूर्णांक ch)
{
	अक्षर संदेश[2];
	संदेश[0] = ch;
	संदेश[1] = '\0';
#अगर HUSH_DEBUG >= 2
	bb_error_संदेश("hush.c:%u", lineno);
#पूर्ण_अगर
	bb_error_संदेश("syntax error: unexpected %s", ch == खातापूर्ण ? "EOF" : संदेश);
	die_if_script();
}

#अगर HUSH_DEBUG < 2
# undef संदेश_and_die_if_script
# undef syntax_error
# undef syntax_error_at
# undef syntax_error_unterm_ch
# undef syntax_error_unterm_str
# undef syntax_error_unexpected_ch
#अन्यथा
# define संदेश_and_die_if_script(...)     संदेश_and_die_if_script(__LINE__, __VA_ARGS__)
# define syntax_error(संदेश)              syntax_error(__LINE__, संदेश)
# define syntax_error_at(संदेश)           syntax_error_at(__LINE__, संदेश)
# define syntax_error_unterm_ch(ch)     syntax_error_unterm_ch(__LINE__, ch)
# define syntax_error_unterm_str(s)     syntax_error_unterm_str(__LINE__, s)
# define syntax_error_unexpected_ch(ch) syntax_error_unexpected_ch(__LINE__, ch)
#पूर्ण_अगर

/* Utility functions
 */
/* Replace each \x with x in place, return ptr past NUL. */
अटल अक्षर *unbackslash(अक्षर *src)
{
#अगर_घोषित __U_BOOT__
	अक्षर *dst = src = (अक्षर *)strchrnul(src, '\\');
#अन्यथा /* !__U_BOOT__ */
	अक्षर *dst = src = strchrnul(src, '\\');
#पूर्ण_अगर /* !__U_BOOT__ */
	जबतक (1) {
		अगर (*src == '\\') {
			src++;
			अगर (*src != '\0') {
				/* \x -> x */
				*dst++ = *src++;
				जारी;
			}
			/* else: "\<nul>". Do not delete this backslash.
			 * Testcase: eval 'echo ok\'
			 */
			*dst++ = '\\';
			/* fallthrough */
		}
		अगर ((*dst++ = *src++) == '\0')
			अवरोध;
	}
	वापस dst;
}

अटल अक्षर **add_strings_to_strings(अक्षर **strings, अक्षर **add, पूर्णांक need_to_dup)
{
	पूर्णांक i;
	अचिन्हित count1;
	अचिन्हित count2;
	अक्षर **v;

	v = strings;
	count1 = 0;
	अगर (v) {
		जबतक (*v) {
			count1++;
			v++;
		}
	}
	count2 = 0;
	v = add;
	जबतक (*v) {
		count2++;
		v++;
	}
	v = xrealloc(strings, (count1 + count2 + 1) * माप(अक्षर*));
	v[count1 + count2] = शून्य;
	i = count2;
	जबतक (--i >= 0)
		v[count1 + i] = (need_to_dup ? xstrdup(add[i]) : add[i]);
	वापस v;
}
#अगर LEAK_HUNTING
अटल अक्षर **xx_add_strings_to_strings(पूर्णांक lineno, अक्षर **strings, अक्षर **add, पूर्णांक need_to_dup)
{
	अक्षर **ptr = add_strings_to_strings(strings, add, need_to_dup);
	fdprintf(2, "line %d: add_strings_to_strings %p\n", lineno, ptr);
	वापस ptr;
}
#घोषणा add_strings_to_strings(strings, add, need_to_dup) \
	xx_add_strings_to_strings(__LINE__, strings, add, need_to_dup)
#पूर्ण_अगर

/* Note: takes ownership of "add" ptr (it is not strdup'ed) */
अटल अक्षर **add_string_to_strings(अक्षर **strings, अक्षर *add)
{
	अक्षर *v[2];
	v[0] = add;
	v[1] = शून्य;
	वापस add_strings_to_strings(strings, v, /*dup:*/ 0);
}

#अगर LEAK_HUNTING
अटल अक्षर **xx_add_string_to_strings(पूर्णांक lineno, अक्षर **strings, अक्षर *add)
{
	अक्षर **ptr = add_string_to_strings(strings, add);
	fdprintf(2, "line %d: add_string_to_strings %p\n", lineno, ptr);
	वापस ptr;
}
#घोषणा add_string_to_strings(strings, add) \
	xx_add_string_to_strings(__LINE__, strings, add)
#पूर्ण_अगर

अटल व्योम free_strings(अक्षर **strings)
{
	अक्षर **v;

	अगर (!strings)
		वापस;
	v = strings;
	जबतक (*v) {
		मुक्त(*v);
		v++;
	}
	मुक्त(strings);
}

#अगर_अघोषित __U_BOOT__
अटल पूर्णांक dup_CLOEXEC(पूर्णांक fd, पूर्णांक avoid_fd)
{
	पूर्णांक newfd;
 repeat:
	newfd = fcntl(fd, F_DUPFD_CLOEXEC, avoid_fd + 1);
	अगर (newfd >= 0) {
		अगर (F_DUPFD_CLOEXEC == F_DUPFD) /* if old libc (w/o F_DUPFD_CLOEXEC) */
			close_on_exec_on(newfd);
	} अन्यथा { /* newfd < 0 */
		अगर (त्रुटि_सं == EBUSY)
			जाओ repeat;
		अगर (त्रुटि_सं == EINTR)
			जाओ repeat;
		अगर (त्रुटि_सं != EBADF) {
			/* "echo >&9999" gets EINVAL trying to save fd 1 to above 9999.
			 * We could try saving it _below_ 9999 instead (how?), but
			 * this probably means that dup2(9999,1) to effectuate >&9999
			 * would also not work: fd 9999 can't exist.
			 * (This differs from "echo >&99" where saving works, but
			 * subsequent dup2(99,1) fails if fd 99 is not open).
			 */
			bb_perror_संदेश("fcntl(%d,F_DUPFD,%d)", fd, avoid_fd + 1);
		}
	}
	वापस newfd;
}

अटल पूर्णांक xdup_CLOEXEC_and_close(पूर्णांक fd, पूर्णांक avoid_fd)
{
	पूर्णांक newfd;
 repeat:
	newfd = fcntl(fd, F_DUPFD_CLOEXEC, avoid_fd + 1);
	अगर (newfd < 0) {
		अगर (त्रुटि_सं == EBUSY)
			जाओ repeat;
		अगर (त्रुटि_सं == EINTR)
			जाओ repeat;
		/* fd was not open? */
		अगर (त्रुटि_सं == EBADF)
			वापस fd;
		xfunc_die();
	}
	अगर (F_DUPFD_CLOEXEC == F_DUPFD) /* if old libc (w/o F_DUPFD_CLOEXEC) */
		close_on_exec_on(newfd);
	बंद(fd);
	वापस newfd;
}

/* Manipulating HFILEs */
अटल HFILE *hfopen(स्थिर अक्षर *name)
{
	HFILE *fp;
	पूर्णांक fd;

	fd = STDIN_FILENO;
	अगर (name) {
		fd = खोलो(name, O_RDONLY | O_CLOEXEC);
		अगर (fd < 0)
			वापस शून्य;
		अगर (O_CLOEXEC == 0) /* ancient libc */
			close_on_exec_on(fd);
	}

	fp = xmalloc(माप(*fp));
	अगर (name == शून्य)
		G.HFILE_stdin = fp;
	fp->fd = fd;
	fp->cur = fp->end = fp->buf;
	fp->अग्र_hfile = G.HFILE_list;
	G.HFILE_list = fp;
	वापस fp;
}
अटल व्योम hfclose(HFILE *fp)
{
	HFILE **pp = &G.HFILE_list;
	जबतक (*pp) {
		HFILE *cur = *pp;
		अगर (cur == fp) {
			*pp = cur->अग्र_hfile;
			अवरोध;
		}
		pp = &cur->अग्र_hfile;
	}
	अगर (fp->fd >= 0)
		बंद(fp->fd);
	मुक्त(fp);
}
अटल पूर्णांक refill_HFILE_and_getc(HFILE *fp)
{
	पूर्णांक n;

	अगर (fp->fd < 0) {
		/* Already saw EOF */
		वापस खातापूर्ण;
	}
#अगर ENABLE_HUSH_INTERACTIVE && !ENABLE_FEATURE_EDITING
	/* If user presses ^C, read() restarts after SIGINT (we use SA_RESTART).
	 * IOW: ^C will not immediately stop line input.
	 * But poll() is different: it does NOT restart after signals.
	 */
	अगर (fp == G.HFILE_stdin) {
		काष्ठा pollfd pfd[1];
		pfd[0].fd = fp->fd;
		pfd[0].events = POLLIN;
		n = poll(pfd, 1, -1);
		अगर (n < 0
		 /*&& errno == EINTR - assumed true */
		 && sigismember(&G.pending_set, संक_विघ्न)
		) {
			वापस '\0';
		}
	}
#अन्यथा
/* if FEATURE_EDITING=y, we do not use this routine for interactive input */
#पूर्ण_अगर
	/* Try to buffer more input */
	n = safe_read(fp->fd, fp->buf, माप(fp->buf));
	अगर (n < 0) {
		bb_simple_perror_संदेश("read error");
		n = 0;
	}
	fp->cur = fp->buf;
	fp->end = fp->buf + n;
	अगर (n == 0) {
		/* EOF/error */
		बंद(fp->fd);
		fp->fd = -1;
		वापस खातापूर्ण;
	}
	वापस (अचिन्हित अक्षर)(*fp->cur++);
}
/* Inlined for common case of non-empty buffer.
 */
अटल ALWAYS_INLINE पूर्णांक hfgetc(HFILE *fp)
{
	अगर (fp->cur < fp->end)
		वापस (अचिन्हित अक्षर)(*fp->cur++);
	/* Buffer empty */
	वापस refill_HFILE_and_getc(fp);
}
अटल पूर्णांक move_HFILEs_on_redirect(पूर्णांक fd, पूर्णांक avoid_fd)
{
	HFILE *fl = G.HFILE_list;
	जबतक (fl) {
		अगर (fd == fl->fd) {
			/* We use it only on script files, they are all CLOEXEC */
			fl->fd = xdup_CLOEXEC_and_close(fd, avoid_fd);
			debug_printf_redir("redirect_fd %d: matches a script fd, moving it to %d\n", fd, fl->fd);
			वापस 1; /* "found and moved" */
		}
		fl = fl->अग्र_hfile;
	}
#अगर ENABLE_HUSH_MODE_X
	अगर (G.x_mode_fd > 0 && fd == G.x_mode_fd) {
		G.x_mode_fd = xdup_CLOEXEC_and_close(fd, avoid_fd);
		वापस 1; /* "found and moved" */
	}
#पूर्ण_अगर
	वापस 0; /* "not in the list" */
}
#अगर ENABLE_FEATURE_SH_STANDALONE && BB_MMU
अटल व्योम close_all_HFILE_list(व्योम)
{
	HFILE *fl = G.HFILE_list;
	जबतक (fl) {
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
		अगर (fl->fd > 0)
			/*hfclose(fl); - unsafe */
			बंद(fl->fd);
		fl = fl->अग्र_hfile;
	}
}
#पूर्ण_अगर
अटल पूर्णांक fd_in_HFILEs(पूर्णांक fd)
{
	HFILE *fl = G.HFILE_list;
	जबतक (fl) {
		अगर (fl->fd == fd)
			वापस 1;
		fl = fl->अग्र_hfile;
	}
	वापस 0;
}

#पूर्ण_अगर /* !__U_BOOT__ */

/* Helpers for setting new $n and restoring them back
 */
प्रकार काष्ठा save_arg_t {
	अक्षर *sv_argv0;
	अक्षर **sv_g_argv;
	पूर्णांक sv_g_argc;
#अगर_अघोषित __U_BOOT__
	IF_HUSH_SET(smallint sv_g_malloced;)
#पूर्ण_अगर /* !__U_BOOT__ */
} save_arg_t;

#अगर_अघोषित __U_BOOT__
अटल व्योम save_and_replace_G_args(save_arg_t *sv, अक्षर **argv)
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

अटल व्योम restore_G_args(save_arg_t *sv, अक्षर **argv)
{
#अगर ENABLE_HUSH_SET
	अगर (G.global_args_malloced) {
		/* someone ran "set -- arg1 arg2 ...", undo */
		अक्षर **pp = G.global_argv;
		जबतक (*++pp) /* note: does not free $0 */
			मुक्त(*pp);
		मुक्त(G.global_argv);
	}
#पूर्ण_अगर
	argv[0] = sv->sv_argv0;
	G.global_argv = sv->sv_g_argv;
	G.global_argc = sv->sv_g_argc;
	IF_HUSH_SET(G.global_args_malloced = sv->sv_g_malloced;)
}
#पूर्ण_अगर /* !__U_BOOT__ */

#अगर_अघोषित __U_BOOT__
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
क्रमागत {
	SPECIAL_INTERACTIVE_SIGS = 0
		| (1 << संक_इति)
		| (1 << संक_विघ्न)
		| (1 << SIGHUP)
		,
	SPECIAL_JOBSTOP_SIGS = 0
#अगर ENABLE_HUSH_JOB
		| (1 << SIGTTIN)
		| (1 << SIGTTOU)
		| (1 << SIGTSTP)
#पूर्ण_अगर
		,
};

अटल व्योम record_pending_signo(पूर्णांक sig)
{
	sigaddset(&G.pending_set, sig);
#अगर ENABLE_FEATURE_EDITING
	अगर (sig != SIGCHLD
	 || (G_traps && G_traps[SIGCHLD] && G_traps[SIGCHLD][0])
	 /* ^^^ if SIGCHLD, interrupt line reading only if it has a trap */
	) {
		bb_got_signal = sig; /* for read_line_input: "we got a signal" */
	}
#पूर्ण_अगर
#अगर ENABLE_HUSH_FAST
	अगर (sig == SIGCHLD) {
		G.गिनती_SIGCHLD++;
//bb_error_msg("[%d] SIGCHLD_handler: G.count_SIGCHLD:%d G.handled_SIGCHLD:%d", getpid(), G.count_SIGCHLD, G.handled_SIGCHLD);
	}
#पूर्ण_अगर
}

अटल sighandler_t install_sighandler(पूर्णांक sig, sighandler_t handler)
{
	काष्ठा sigaction old_sa;

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
	वापस old_sa.sa_handler;
}
#पूर्ण_अगर /* !__U_BOOT__ */

#अगर_अघोषित __U_BOOT__
अटल व्योम hush_exit(पूर्णांक exitcode) NORETURN;

अटल व्योम restore_ttypgrp_and__exit(व्योम) NORETURN;
अटल व्योम restore_ttypgrp_and__exit(व्योम)
{
	/* xfunc has failed! die die die */
	/* no EXIT traps, this is an escape hatch! */
	G.exiting = 1;
	hush_exit(xfunc_error_retval);
}

#अगर ENABLE_HUSH_JOB

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
अटल व्योम fflush_and__exit(व्योम) NORETURN;
अटल व्योम fflush_and__exit(व्योम)
{
	fflush_all();
	_निकास(xfunc_error_retval);
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
अटल व्योम sigexit(पूर्णांक sig) NORETURN;
अटल व्योम sigexit(पूर्णांक sig)
{
	/* Careful: we can end up here after [v]fork. Do not restore
	 * tty pgrp then, only top-level shell process does that */
	अगर (G_saved_tty_pgrp && getpid() == G.मूल_pid) {
		/* Disable all signals: job control, SIGPIPE, etc.
		 * Mostly paranoid measure, to prevent infinite SIGTTOU.
		 */
		sigprocmask_allsigs(SIG_BLOCK);
		tcsetpgrp(G_interactive_fd, G_saved_tty_pgrp);
	}

	/* Not a signal, just exit */
	अगर (sig <= 0)
		_निकास(- sig);

	kill_myself_with_sig(sig); /* does not return */
}
#अन्यथा

# define disable_restore_tty_pgrp_on_exit() ((व्योम)0)
# define enable_restore_tty_pgrp_on_exit()  ((व्योम)0)

#पूर्ण_अगर
#पूर्ण_अगर /* !__U_BOOT__ */

#अगर_अघोषित __U_BOOT__
अटल sighandler_t pick_sighandler(अचिन्हित sig)
{
	sighandler_t handler = संक_पूर्व;
	अगर (sig < माप(अचिन्हित)*8) {
		अचिन्हित sigmask = (1 << sig);

#अगर ENABLE_HUSH_JOB
		/* is sig fatal? */
		अगर (G_fatal_sig_mask & sigmask)
			handler = sigexit;
		अन्यथा
#पूर्ण_अगर
		/* sig has special handling? */
		अगर (G.special_sig_mask & sigmask) {
			handler = record_pending_signo;
			/* TTIN/TTOU/TSTP can't be set to record_pending_signo
			 * in order to ignore them: they will be raised
			 * in an endless loop when we try to do some
			 * terminal ioctls! We do have to _ignore_ these.
			 */
			अगर (SPECIAL_JOBSTOP_SIGS & sigmask)
				handler = संक_छोड़ो;
		}
	}
	वापस handler;
}
#पूर्ण_अगर /* !__U_BOOT__ */

#अगर_अघोषित __U_BOOT__
/* Restores tty foreground process group, and exits. */
अटल व्योम hush_exit(पूर्णांक exitcode)
{
#अगर ENABLE_FEATURE_EDITING_SAVE_ON_EXIT
	save_history(G.पंक्ति_निवेश_state); /* may be NULL */
#पूर्ण_अगर

	fflush_all();
	अगर (G.exiting <= 0 && G_traps && G_traps[0] && G_traps[0][0]) {
		अक्षर *argv[3];
		/* argv[0] is unused */
		argv[1] = xstrdup(G_traps[0]); /* copy, since EXIT trap handler may modify G_traps[0] */
		argv[2] = शून्य;
		G.exiting = 1; /* prevent EXIT trap recursion */
		/* Note: G_traps[0] is not cleared!
		 * "trap" will still show it, if executed
		 * in the handler */
		builtin_eval(argv);
	}

#अगर ENABLE_FEATURE_CLEAN_UP
	{
		काष्ठा variable *cur_var;
		अगर (G.cwd != bb_संदेश_unknown)
			मुक्त((अक्षर*)G.cwd);
		cur_var = G.शीर्ष_var;
		जबतक (cur_var) {
			काष्ठा variable *tmp = cur_var;
			अगर (!cur_var->max_len)
				मुक्त(cur_var->varstr);
			cur_var = cur_var->अग्र;
			मुक्त(tmp);
		}
	}
#पूर्ण_अगर

	fflush_all();
#अगर ENABLE_HUSH_JOB
	sigexit(- (exitcode & 0xff));
#अन्यथा
	_निकास(exitcode);
#पूर्ण_अगर
}

//TODO: return a mask of ALL handled sigs?
अटल पूर्णांक check_and_run_traps(व्योम)
{
	पूर्णांक last_sig = 0;

	जबतक (1) {
		पूर्णांक sig;

		अगर (sigisemptyset(&G.pending_set))
			अवरोध;
		sig = 0;
		करो {
			sig++;
			अगर (sigismember(&G.pending_set, sig)) {
				sigdelset(&G.pending_set, sig);
				जाओ got_sig;
			}
		} जबतक (sig < NSIG);
		अवरोध;
 got_sig:
#अगर ENABLE_HUSH_TRAP
		अगर (G_traps && G_traps[sig]) {
			debug_printf_exec("%s: sig:%d handler:'%s'\n", __func__, sig, G.traps[sig]);
			अगर (G_traps[sig][0]) {
				/* We have user-defined handler */
				smalluint save_rcode;
				पूर्णांक save_pre;
				अक्षर *argv[3];
				/* argv[0] is unused */
				argv[1] = xstrdup(G_traps[sig]);
				/* why strdup? trap can modify itself: trap 'trap "echo oops" INT' INT */
				argv[2] = शून्य;
				save_pre = G.pre_trap_exitcode;
				G.pre_trap_exitcode = save_rcode = G.last_exitcode;
				builtin_eval(argv);
				मुक्त(argv[1]);
				G.pre_trap_exitcode = save_pre;
				G.last_exitcode = save_rcode;
# अगर ENABLE_HUSH_FUNCTIONS
				अगर (G.return_exitcode >= 0) {
					debug_printf_exec("trap exitcode:%d\n", G.return_exitcode);
					G.last_exitcode = G.return_exitcode;
				}
# endif
				last_sig = sig;
			} /* else: "" trap, ignoring signal */
			जारी;
		}
#पूर्ण_अगर
		/* not a trap: special action */
		चयन (sig) {
		हाल संक_विघ्न:
			debug_printf_exec("%s: sig:%d default SIGINT handler\n", __func__, sig);
			G.flag_SIGINT = 1;
			last_sig = sig;
			अवरोध;
#अगर ENABLE_HUSH_JOB
		हाल SIGHUP: {
			/* if (G_interactive_fd) - no need to check, the handler
			 * is only installed if we *are* interactive */
			{
				/* bash compat: "Before exiting, an interactive
				 * shell resends the SIGHUP to all jobs, running
				 * or stopped.  Stopped jobs are sent SIGCONT
				 * to ensure that they receive the SIGHUP."
				 */
				काष्ठा pipe *job;
				debug_printf_exec("%s: sig:%d default SIGHUP handler\n", __func__, sig);
				/* bash is observed to signal whole process groups,
				 * not individual processes */
				क्रम (job = G.job_list; job; job = job->अग्र) {
					अगर (job->pgrp <= 0)
						जारी;
					debug_printf_exec("HUPing pgrp %d\n", job->pgrp);
					अगर (समाप्त(- job->pgrp, SIGHUP) == 0)
						समाप्त(- job->pgrp, SIGCONT);
				}
			}
			/* this restores tty pgrp, then kills us with SIGHUP */
			sigexit(SIGHUP);
		}
#पूर्ण_अगर
#अगर ENABLE_HUSH_FAST
		हाल SIGCHLD:
			debug_printf_exec("%s: sig:%d default SIGCHLD handler\n", __func__, sig);
			G.गिनती_SIGCHLD++;
//bb_error_msg("[%d] check_and_run_traps: G.count_SIGCHLD:%d G.handled_SIGCHLD:%d", getpid(), G.count_SIGCHLD, G.handled_SIGCHLD);
			/* Note:
			 * We don't do 'last_sig = sig' here -> NOT returning this sig.
			 * This simplifies wait builtin a bit.
			 */
			अवरोध;
#पूर्ण_अगर
		शेष: /* ignored: */
			debug_printf_exec("%s: sig:%d default handling is to ignore\n", __func__, sig);
			/* SIGTERM, SIGQUIT, SIGTTIN, SIGTTOU, SIGTSTP */
			/* Note:
			 * We don't do 'last_sig = sig' here -> NOT returning this sig.
			 * Example: wait is not interrupted by TERM
			 * in interactive shell, because TERM is ignored.
			 */
			अवरोध;
		}
	}
	वापस last_sig;
}

अटल स्थिर अक्षर *get_cwd(पूर्णांक force)
{
	अगर (force || G.cwd == शून्य) {
		/* xrealloc_getcwd_or_warn(arg) calls free(arg),
		 * we must not try to free(bb_msg_unknown) */
		अगर (G.cwd == bb_संदेश_unknown)
			G.cwd = शून्य;
		G.cwd = xrealloc_getcwd_or_warn((अक्षर *)G.cwd);
		अगर (!G.cwd)
			G.cwd = bb_संदेश_unknown;
	}
	वापस G.cwd;
}

#पूर्ण_अगर /* !__U_BOOT__ */

/*
 * Shell and environment variable support
 */
अटल काष्ठा variable **get_ptr_to_local_var(स्थिर अक्षर *name)
{
	काष्ठा variable **pp;
	काष्ठा variable *cur;

	pp = &G.शीर्ष_var;
	जबतक ((cur = *pp) != शून्य) {
		अगर (varcmp(cur->varstr, name) == 0)
			वापस pp;
		pp = &cur->अग्र;
	}
	वापस शून्य;
}

अटल स्थिर अक्षर* FAST_FUNC get_local_var_मूल्य(स्थिर अक्षर *name)
{
	काष्ठा variable **vpp;

	अगर (G.expanded_assignments) {
		अक्षर **cpp = G.expanded_assignments;
		जबतक (*cpp) {
			अक्षर *cp = *cpp;
			अगर (varcmp(cp, name) == 0)
				वापस म_अक्षर(cp, '=') + 1;
			cpp++;
		}
	}

	vpp = get_ptr_to_local_var(name);
	अगर (vpp)
		वापस म_अक्षर((*vpp)->varstr, '=') + 1;

#अगर_अघोषित __U_BOOT__
	अगर (म_भेद(name, "PPID") == 0)
		वापस utoa(G.मूल_ppid);
#पूर्ण_अगर /* !__U_BOOT__ */
	// bash compat: UID? EUID?
#अगर ENABLE_HUSH_RANDOM_SUPPORT
	अगर (म_भेद(name, "RANDOM") == 0)
		वापस utoa(अग्र_random(&G.random_gen));
#पूर्ण_अगर
#अगर ENABLE_HUSH_LINENO_VAR
	अगर (म_भेद(name, "LINENO") == 0)
		वापस utoa(G.execute_lineno);
#पूर्ण_अगर
#अगर BASH_EPOCH_VARS
	{
		स्थिर अक्षर *fmt = शून्य;
		अगर (म_भेद(name, "EPOCHSECONDS") == 0)
			fmt = "%llu";
		अन्यथा अगर (म_भेद(name, "EPOCHREALTIME") == 0)
			fmt = "%llu.%06u";
		अगर (fmt) {
			काष्ठा timeval tv;
			xgettimeofday(&tv);
			प्र_लिखो(G.epoch_buf, fmt, (अचिन्हित दीर्घ दीर्घ)tv.tv_sec,
					(अचिन्हित)tv.tv_usec);
			वापस G.epoch_buf;
		}
	}
#पूर्ण_अगर
	वापस शून्य;
}

#अगर_अघोषित __U_BOOT__
#अगर ENABLE_HUSH_GETOPTS
अटल व्योम handle_changed_special_names(स्थिर अक्षर *name)
{
	अगर (varcmp(name, "OPTIND") == 0) {
		G.getopt_गिनती = 0;
		वापस;
	}
}
#अन्यथा
/* Do not even bother evaluating arguments */
# define handle_changed_special_names(...) ((व्योम)0)
#पूर्ण_अगर
#अन्यथा /* __U_BOOT__ */
/* Do not even bother evaluating arguments */
# define handle_changed_special_names(...) ((व्योम)0)
#पूर्ण_अगर /* __U_BOOT__ */

/* str holds "NAME=VAL" and is expected to be malloced.
 * We take ownership of it.
 */
#अगर_अघोषित __U_BOOT__
#घोषणा SETFLAG_EXPORT   (1 << 0)
#घोषणा SETFLAG_UNEXPORT (1 << 1)
#घोषणा SETFLAG_MAKE_RO  (1 << 2)
#पूर्ण_अगर /* !__U_BOOT__ */
#घोषणा SETFLAG_VARLVL_SHIFT   3
#अगर_अघोषित __U_BOOT__
अटल पूर्णांक set_local_var(अक्षर *str, अचिन्हित flags)
#अन्यथा /* __U_BOOT__ */
पूर्णांक set_local_var_modern(अक्षर *str, पूर्णांक flags)
#पूर्ण_अगर /* __U_BOOT__ */
{
	काष्ठा variable **cur_pp;
	काष्ठा variable *cur;
	अक्षर *free_me = शून्य;
	अक्षर *eq_sign;
	पूर्णांक name_len;
	पूर्णांक retval;
#अगर_अघोषित __U_BOOT__
	अचिन्हित local_lvl = (flags >> SETFLAG_VARLVL_SHIFT);
#पूर्ण_अगर /* !__U_BOOT__ */

	eq_sign = म_अक्षर(str, '=');
	अगर (HUSH_DEBUG && !eq_sign)
		bb_simple_error_संदेश_and_die("BUG in setvar");

	name_len = eq_sign - str + 1; /* including '=' */
	cur_pp = &G.शीर्ष_var;
	जबतक ((cur = *cur_pp) != शून्य) {
		अगर (म_भेदन(cur->varstr, str, name_len) != 0) {
			cur_pp = &cur->अग्र;
			जारी;
		}

#अगर_अघोषित __U_BOOT__
		/* We found an existing var with this name */
		अगर (cur->flg_read_only) {
			bb_error_संदेश("%s: readonly variable", str);
			मुक्त(str);
//NOTE: in bash, assignment in "export READONLY_VAR=Z" fails, and sets $?=1,
//but export per se succeeds (does put the var in env). We don't mimic that.
			वापस -1;
		}
		अगर (flags & SETFLAG_UNEXPORT) { // && cur->flg_export ?
			debug_printf_env("%s: unsetenv '%s'\n", __func__, str);
			*eq_sign = '\0';
			unsetenv(str);
			*eq_sign = '=';
		}
		अगर (cur->var_nest_level < local_lvl) {
			/* bash 3.2.33(1) and exported vars:
			 * # export z=z
			 * # f() { local z=a; env | grep ^z; }
			 * # f
			 * z=a
			 * # env | grep ^z
			 * z=z
			 */
			अगर (cur->flg_export)
				flags |= SETFLAG_EXPORT;
			/* New variable is local ("local VAR=VAL" or
			 * "VAR=VAL cmd")
			 * and existing one is global, or local
			 * on a lower level that new one.
			 * Remove it from global variable list:
			 */
			*cur_pp = cur->अग्र;
			अगर (G.shadowed_vars_pp) {
				/* Save in "shadowed" list */
				debug_printf_env("shadowing %s'%s'/%u by '%s'/%u\n",
					cur->flg_export ? "exported " : "",
					cur->varstr, cur->var_nest_level, str, local_lvl
				);
				cur->अग्र = *G.shadowed_vars_pp;
				*G.shadowed_vars_pp = cur;
			} अन्यथा {
				/* Came from pseudo_exec_argv(), no need to save: delete it */
				debug_printf_env("shadow-deleting %s'%s'/%u by '%s'/%u\n",
					cur->flg_export ? "exported " : "",
					cur->varstr, cur->var_nest_level, str, local_lvl
				);
				अगर (cur->max_len == 0) /* allocated "VAR=VAL"? */
					free_me = cur->varstr; /* then free it later */
				मुक्त(cur);
			}
			अवरोध;
		}
#पूर्ण_अगर /* !__U_BOOT__ */

		अगर (म_भेद(cur->varstr + name_len, eq_sign + 1) == 0) {
			debug_printf_env("assignement '%s' does not change anything\n", str);
 free_and_exp:
			मुक्त(str);
			जाओ exp;
		}

		/* Replace the value in the found "struct variable" */
		अगर (cur->max_len != 0) {
			अगर (cur->max_len >= strnlen(str, cur->max_len + 1)) {
				/* This one is from startup env, reuse space */
				debug_printf_env("reusing startup env for '%s'\n", str);
				म_नकल(cur->varstr, str);
				जाओ free_and_exp;
			}
			/* Can't reuse */
			cur->max_len = 0;
			जाओ set_str_and_exp;
		}
		/* max_len == 0 signifies "malloced" var, which we can
		 * (and have to) free. But we can't free(cur->varstr) here:
		 * if cur->flg_export is 1, it is in the environment.
		 * We should either unsetenv+free, or wait until putenv,
		 * then putenv(new)+free(old).
		 */
		free_me = cur->varstr;
		जाओ set_str_and_exp;
	}

	/* Not found or shadowed - create new variable struct */
#अगर_अघोषित __U_BOOT__
	debug_printf_env("%s: alloc new var '%s'/%u\n", __func__, str, local_lvl);
#अन्यथा /* __U_BOOT__ */
	debug_printf_env("%s: alloc new var '%s'\n", __func__, str);
#पूर्ण_अगर /* __U_BOOT__ */
	cur = xzalloc(माप(*cur));
#अगर_अघोषित __U_BOOT__
	cur->var_nest_level = local_lvl;
#पूर्ण_अगर /* !__U_BOOT__ */
	cur->अग्र = *cur_pp;
	*cur_pp = cur;

 set_str_and_exp:
	cur->varstr = str;
 exp:
#अगर_अघोषित __U_BOOT__
#अगर !BB_MMU || ENABLE_HUSH_READONLY
	अगर (flags & SETFLAG_MAKE_RO) {
		cur->flg_read_only = 1;
	}
#पूर्ण_अगर
	अगर (flags & SETFLAG_EXPORT)
		cur->flg_export = 1;
#पूर्ण_अगर /* !__U_BOOT__ */
	retval = 0;
#अगर_अघोषित __U_BOOT__
	अगर (cur->flg_export) {
		अगर (flags & SETFLAG_UNEXPORT) {
			cur->flg_export = 0;
			/* unsetenv was already done */
		} अन्यथा {
			debug_printf_env("%s: putenv '%s'/%u\n", __func__, cur->varstr, cur->var_nest_level);
			retval = putenv(cur->varstr);
			/* fall through to "free(free_me)" -
			 * only now we can free old exported malloced string
			 */
		}
	}
#पूर्ण_अगर /* !__U_BOOT__ */
	मुक्त(free_me);

	handle_changed_special_names(cur->varstr);

	वापस retval;
}

#अगर_अघोषित __U_BOOT__
अटल पूर्णांक set_local_var0(अक्षर *str)
{
	वापस set_local_var(str, 0);
}

अटल व्योम FAST_FUNC set_local_var_from_halves(स्थिर अक्षर *name, स्थिर अक्षर *val)
{
	अक्षर *var = xasprintf("%s=%s", name, val);
	set_local_var0(var);
}

/* Used at startup and after each cd */
अटल व्योम set_pwd_var(अचिन्हित flag)
{
	set_local_var(xasprintf("PWD=%s", get_cwd(/*force:*/ 1)), flag);
}
#पूर्ण_अगर /* !__U_BOOT__ */

#अगर ENABLE_HUSH_UNSET || ENABLE_HUSH_GETOPTS
अटल पूर्णांक unset_local_var(स्थिर अक्षर *name)
{
	काष्ठा variable *cur;
	काष्ठा variable **cur_pp;

	cur_pp = &G.शीर्ष_var;
	जबतक ((cur = *cur_pp) != शून्य) {
		अगर (varcmp(cur->varstr, name) == 0) {
			अगर (cur->flg_read_only) {
				bb_error_संदेश("%s: readonly variable", name);
				वापस निकास_त्रुटि;
			}

			*cur_pp = cur->अग्र;
			debug_printf_env("%s: unsetenv '%s'\n", __func__, cur->varstr);
			bb_unsetenv(cur->varstr);
			अगर (!cur->max_len)
				मुक्त(cur->varstr);
			मुक्त(cur);

			अवरोध;
		}
		cur_pp = &cur->अग्र;
	}

	/* Handle "unset LINENO" et al even if did not find the variable to unset */
	handle_changed_special_names(name);

	वापस निकास_सफल;
}
#पूर्ण_अगर

#अगर_अघोषित __U_BOOT__
/*
 * Helpers for "var1=val1 var2=val2 cmd" feature
 */
अटल व्योम add_vars(काष्ठा variable *var)
{
	काष्ठा variable *अग्र;

	जबतक (var) {
		अग्र = var->अग्र;
		var->अग्र = G.शीर्ष_var;
		G.शीर्ष_var = var;
		अगर (var->flg_export) {
			debug_printf_env("%s: restoring exported '%s'/%u\n", __func__, var->varstr, var->var_nest_level);
			putenv(var->varstr);
		} अन्यथा {
			debug_printf_env("%s: restoring variable '%s'/%u\n", __func__, var->varstr, var->var_nest_level);
		}
		var = अग्र;
	}
}

/* We put strings[i] into variable table and possibly putenv them.
 * If variable is read only, we can free the strings[i]
 * which attempts to overwrite it.
 * The strings[] vector itself is freed.
 */
अटल व्योम set_vars_and_save_old(अक्षर **strings)
{
	अक्षर **s;

	अगर (!strings)
		वापस;

	s = strings;
	जबतक (*s) {
		काष्ठा variable *var_p;
		काष्ठा variable **var_pp;
		अक्षर *eq;

		eq = म_अक्षर(*s, '=');
		अगर (HUSH_DEBUG && !eq)
			bb_simple_error_संदेश_and_die("BUG in varexp4");
		var_pp = get_ptr_to_local_var(*s);
		अगर (var_pp) {
			var_p = *var_pp;
			अगर (var_p->flg_read_only) {
				अक्षर **p;
				bb_error_संदेश("%s: readonly variable", *s);
				/*
				 * "VAR=V BLTIN" unsets VARs after BLTIN completes.
				 * If VAR is readonly, leaving it in the list
				 * after asssignment error (msg above)
				 * causes doubled error message later, on unset.
				 */
				debug_printf_env("removing/freeing '%s' element\n", *s);
				मुक्त(*s);
				p = s;
				करो { *p = p[1]; p++; } जबतक (*p);
				जाओ अग्र;
			}
			/* below, set_local_var() with nest level will
			 * "shadow" (remove) this variable from
			 * global linked list.
			 */
		}
		debug_printf_env("%s: env override '%s'/%u\n", __func__, *s, G.var_nest_level);
		set_local_var(*s, (G.var_nest_level << SETFLAG_VARLVL_SHIFT) | SETFLAG_EXPORT);
		s++;
 अग्र: ;
	}
	मुक्त(strings);
}

/*
 * Unicode helper
 */
अटल व्योम reinit_unicode_for_hush(व्योम)
{
	/* Unicode support should be activated even if LANG is set
	 * _during_ shell execution, not only if it was set when
	 * shell was started. Therefore, re-check LANG every time:
	 */
	अगर (ENABLE_FEATURE_CHECK_UNICODE_IN_ENV
	 || ENABLE_UNICODE_USING_LOCALE
	) {
		स्थिर अक्षर *s = get_local_var_मूल्य("LC_ALL");
		अगर (!s) s = get_local_var_मूल्य("LC_CTYPE");
		अगर (!s) s = get_local_var_मूल्य("LANG");
		reinit_unicode(s);
	}
}

#पूर्ण_अगर /* !__U_BOOT__ */
/*
 * in_str support (strings, and "strings" read from files).
 */

#अगर ENABLE_HUSH_INTERACTIVE
#अगर_अघोषित __U_BOOT__
/* To test correct lineedit/interactive behavior, type from command line:
 *	echo $P\
 *	\
 *	AT\
 *	H\
 *	\
 * It exercises a lot of corner cases.
 */
अटल स्थिर अक्षर *setup_prompt_string(व्योम)
{
	स्थिर अक्षर *prompt_str;

	debug_printf_prompt("%s promptmode:%d\n", __func__, G.promptmode);

# अगर ENABLE_FEATURE_EDITING_FANCY_PROMPT
	prompt_str = get_local_var_मूल्य(G.promptmode == 0 ? "PS1" : "PS2");
	अगर (!prompt_str)
		prompt_str = "";
# अन्यथा
	prompt_str = "> "; /* if PS2, else... */
	अगर (G.promptmode == 0) { /* PS1 */
		/* No fancy prompts supported, (re)generate "CURDIR $ " by hand */
		मुक्त(G.PS1);
		/* bash uses $PWD value, even if it is set by user.
		 * It uses current dir only if PWD is unset.
		 * We always use current dir. */
		prompt_str = G.PS1 = xasprintf("%s %c ", get_cwd(0), (geteuid() != 0) ? '$' : '#');
	}
# endif
	debug_printf("prompt_str '%s'\n", prompt_str);
	वापस prompt_str;
}
#पूर्ण_अगर /* !__U_BOOT__ */

#अगर_अघोषित __U_BOOT__
अटल पूर्णांक get_user_निवेश(काष्ठा in_str *i)
#अन्यथा /* __U_BOOT__ */
अटल व्योम get_user_निवेश(काष्ठा in_str *i)
#पूर्ण_अगर /* __U_BOOT__ */
{
#अगर_अघोषित __U_BOOT__
# अगर ENABLE_FEATURE_EDITING
	/* In EDITING case, this function reads next input line,
	 * saves it in i->p, then returns 1st char of it.
	 */
	पूर्णांक r;
	स्थिर अक्षर *prompt_str;

	prompt_str = setup_prompt_string();
	क्रम (;;) {
		reinit_unicode_for_hush();
		G.flag_SIGINT = 0;

		bb_got_signal = 0;
		अगर (!sigisemptyset(&G.pending_set)) {
			/* Whoops, already got a signal, do not call read_line_input */
			bb_got_signal = r = -1;
		} अन्यथा {
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
			r = read_पंक्ति_निवेश(G.पंक्ति_निवेश_state, prompt_str,
				G.user_निवेश_buf, CONFIG_FEATURE_EDITING_MAX_LEN-1
			);
			/* read_line_input intercepts ^C, "convert" it to SIGINT */
			अगर (r == 0)
				उठाओ(संक_विघ्न);
		}
		/* bash prints ^C (before running a trap, if any)
		 * both on keyboard ^C and on real SIGINT (non-kbd generated).
		 */
		अगर (sigismember(&G.pending_set, संक_विघ्न)) {
			ग_लिखो(STDOUT_FILENO, "^C\n", 3);
			G.last_exitcode = 128 | संक_विघ्न;
		}
		check_and_run_traps();
		अगर (r == 0) /* keyboard ^C? */
			जारी; /* go back, read another input line */
		अगर (r > 0) /* normal input? (no ^C, no ^D, no signals) */
			अवरोध;
		अगर (!bb_got_signal) {
			/* r < 0: ^D/EOF/error detected (but not signal) */
			/* ^D on interactive input goes to next line before exiting: */
			ग_लिखो(STDOUT_FILENO, "\n", 1);
			i->p = शून्य;
			i->peek_buf[0] = r = खातापूर्ण;
			वापस r;
		}
		/* it was a signal: go back, read another input line */
	}
	i->p = G.user_निवेश_buf;
	वापस (अचिन्हित अक्षर)*i->p++;
# अन्यथा
	/* In !EDITING case, this function gets called for every char.
	 * Buffering happens deeper in the call chain, in hfgetc(i->file).
	 */
	पूर्णांक r;

	क्रम (;;) {
		G.flag_SIGINT = 0;
		अगर (i->last_char == '\0' || i->last_char == '\n') {
			स्थिर अक्षर *prompt_str = setup_prompt_string();
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
		अगर (r != '\0' && !G.flag_SIGINT)
			अवरोध;
		अगर (G.flag_SIGINT) {
			/* ^C or SIGINT: repeat */
			/* bash prints ^C even on real SIGINT (non-kbd generated) */
			/* kernel prints "^C" itself, just print newline: */
			ग_लिखो(STDOUT_FILENO, "\n", 1);
			G.last_exitcode = 128 | संक_विघ्न;
		}
	}
	वापस r;
# endif
#अन्यथा /* __U_BOOT__ */
	पूर्णांक n;
	पूर्णांक promptme;
	अटल अक्षर the_command[CONFIG_SYS_CBSIZE + 1];

	bootretry_reset_cmd_timeout();
	promptme = 1;
	n = u_boot_cli_readline(i);

# ifdef CONFIG_BOOT_RETRY_TIME
	अगर (n == -2) {
		माला_दो("\nTimeout waiting for command\n");
#  ifdef CONFIG_RESET_TO_RETRY
		do_reset(शून्य, 0, 0, शून्य);
#  elif IS_ENABLED(CONFIG_RETRY_BOOTCMD)
	म_नकल(console_बफर, "run bootcmd\n");
# अन्यथा
#	error "This only works with CONFIG_RESET_TO_RETRY or CONFIG_BOOT_RETRY_COMMAND enabled"
#  endif
	}
# endif
	अगर (n == -1 ) {
		G.flag_repeat = 0;
		promptme = 0;
	}
	n = म_माप(console_बफर);
	console_बफर[n] = '\n';
	console_बफर[n+1]= '\0';
	अगर (had_ctrlc())
		G.flag_repeat = 0;
	clear_ctrlc();
	G.do_repeat = 0;
#अगर_अघोषित __U_BOOT__
	अगर (G.promptmode == 1) {
#अन्यथा /* __U_BOOT__ */
	अगर (!G.promptmode) {
#पूर्ण_अगर /* __U_BOOT__ */
		अगर (console_बफर[0] == '\n'&& G.flag_repeat == 0) {
			म_नकल(the_command, console_बफर);
		}
		अन्यथा {
			अगर (console_बफर[0] != '\n') {
				म_नकल(the_command, console_बफर);
				G.flag_repeat = 1;
			}
			अन्यथा {
				G.do_repeat = 1;
			}
		}
		i->p = the_command;
	}
	अन्यथा {
		अगर (console_बफर[0] != '\n') {
			अगर (म_माप(the_command) + म_माप(console_बफर)
				< CONFIG_SYS_CBSIZE) {
				n = म_माप(the_command);
#अगर_घोषित __U_BOOT__
				/*
				 * To avoid writing to bad places, we check if
				 * n is greater than 0.
				 * This bug was found by Harald Seiler.
				 */
				अगर (n > 0)
					the_command[n-1] = ' ';
				म_नकल(&the_command[n], console_बफर);
#अन्यथा /* !__U_BOOT__ */
			the_command[n-1] = ' ';
			म_नकल(&the_command[n], console_बफर);
#पूर्ण_अगर /* !__U_BOOT__ */
				}
				अन्यथा {
					the_command[0] = '\n';
					the_command[1] = '\0';
					G.flag_repeat = 0;
				}
		}
		अगर (promptme == 0) {
			the_command[0] = '\n';
			the_command[1] = '\0';
		}
		i->p = console_बफर;
	}
#पूर्ण_अगर /* __U_BOOT__ */
}
/* This is the magic location that prints prompts
 * and gets data back from the user */
अटल पूर्णांक fgetc_interactive(काष्ठा in_str *i)
{
	पूर्णांक ch;
#अगर_अघोषित __U_BOOT__
	/* If it's interactive stdin, get new line. */
	अगर (G_interactive_fd && i->file == G.HFILE_stdin) {
#पूर्ण_अगर /* !__U_BOOT__ */
#अगर_अघोषित __U_BOOT__
		/* Returns first char (or EOF), the rest is in i->p[] */
		ch = get_user_निवेश(i);
#अन्यथा /* __U_BOOT__ */
		/* Avoid garbage value and make clang happy. */
		ch = 0;
		/*
		 * get_user_input() does not return anything when used in
		 * U-Boot.
		 * So, we need to take the read character from i->p[].
		 */
		get_user_निवेश(i);
		अगर (i->p && *i->p) {
			ch = *i->p++;
		}
#पूर्ण_अगर /* __U_BOOT__ */
		G.promptmode = 1; /* PS2 */
		debug_printf_prompt("%s promptmode=%d\n", __func__, G.promptmode);
#अगर_अघोषित __U_BOOT__
	} अन्यथा {
		/* Not stdin: script file, sourced file, etc */
		करो ch = hfgetc(i->file); जबतक (ch == '\0');
	}
#पूर्ण_अगर /* !__U_BOOT__ */
	वापस ch;
}
#अन्यथा  /* !INTERACTIVE */
#अगर_अघोषित __U_BOOT__
अटल ALWAYS_INLINE पूर्णांक fgetc_interactive(काष्ठा in_str *i)
{
	पूर्णांक ch;
	करो ch = hfgetc(i->file); जबतक (ch == '\0');
	वापस ch;
}
#पूर्ण_अगर /* !__U_BOOT__ */
#पूर्ण_अगर  /* !INTERACTIVE */

अटल पूर्णांक i_getch(काष्ठा in_str *i)
{
	पूर्णांक ch;

#अगर_अघोषित __U_BOOT__
	अगर (!i->file) {
		/* string-based in_str */
		ch = (अचिन्हित अक्षर)*i->p;
		अगर (ch != '\0') {
			i->p++;
			i->last_char = ch;
#अगर ENABLE_HUSH_LINENO_VAR
			अगर (ch == '\n') {
				G.parse_lineno++;
				debug_printf_parse("G.parse_lineno++ = %u\n", G.parse_lineno);
			}
#पूर्ण_अगर
			वापस ch;
		}
		वापस खातापूर्ण;
	}

#पूर्ण_अगर /* !__U_BOOT__ */
	/* FILE-based in_str */

#अगर ENABLE_FEATURE_EDITING
	/* This can be stdin, check line editing char[] buffer */
	अगर (i->p && *i->p != '\0') {
		ch = (अचिन्हित अक्षर)*i->p++;
		जाओ out;
#अगर_अघोषित __U_BOOT__
	}
#अन्यथा /* __U_BOOT__ */
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
	} अन्यथा अगर (i->p && *i->p == '\0' && G.run_command_flags){
		वापस खातापूर्ण;
	}
#पूर्ण_अगर /* __U_BOOT__ */
#पूर्ण_अगर
#अगर_अघोषित __U_BOOT__
	/* peek_buf[] is an int array, not char. Can contain EOF. */
	ch = i->peek_buf[0];
	अगर (ch != 0) {
		पूर्णांक ch2 = i->peek_buf[1];
		i->peek_buf[0] = ch2;
		अगर (ch2 == 0) /* very likely, avoid redundant write */
			जाओ out;
		i->peek_buf[1] = 0;
		जाओ out;
	}

#पूर्ण_अगर /* !__U_BOOT__ */
	ch = fgetc_interactive(i);
 out:
	debug_printf("file_get: got '%c' %d\n", ch, ch);
	i->last_char = ch;
#अगर ENABLE_HUSH_LINENO_VAR
	अगर (ch == '\n') {
		G.parse_lineno++;
		debug_printf_parse("G.parse_lineno++ = %u\n", G.parse_lineno);
	}
#पूर्ण_अगर
	वापस ch;
}

अटल पूर्णांक i_peek(काष्ठा in_str *i)
{
#अगर_अघोषित __U_BOOT__
	पूर्णांक ch;

	अगर (!i->file) {
		/* string-based in_str */
		/* Doesn't report EOF on NUL. None of the callers care. */
		वापस (अचिन्हित अक्षर)*i->p;
	}

	/* FILE-based in_str */

#अगर ENABLE_FEATURE_EDITING && ENABLE_HUSH_INTERACTIVE
	/* This can be stdin, check line editing char[] buffer */
	अगर (i->p && *i->p != '\0')
		वापस (अचिन्हित अक्षर)*i->p;
#पूर्ण_अगर
	/* peek_buf[] is an int array, not char. Can contain EOF. */
	ch = i->peek_buf[0];
	अगर (ch != 0)
		वापस ch;

	/* Need to get a new char */
	ch = fgetc_interactive(i);
	debug_printf("file_peek: got '%c' %d\n", ch, ch);

	/* Save it by either rolling back line editing buffer, or in i->peek_buf[0] */
#अगर ENABLE_FEATURE_EDITING && ENABLE_HUSH_INTERACTIVE
	अगर (i->p) {
		i->p -= 1;
		वापस ch;
	}
#पूर्ण_अगर
	i->peek_buf[0] = ch;
	/*i->peek_buf[1] = 0; - already is */
	वापस ch;
#अन्यथा /* __U_BOOT__ */
	/* string-based in_str */
	/* Doesn't report EOF on NUL. None of the callers care. */
	वापस (अचिन्हित अक्षर)*i->p;
#पूर्ण_अगर /* __U_BOOT__ */
}

/* Only ever called if i_peek() was called, and did not return EOF.
 * IOW: we know the previous peek saw an ordinary char, not EOF, not NUL,
 * not end-of-line. Therefore we never need to read a new editing line here.
 */
अटल पूर्णांक i_peek2(काष्ठा in_str *i)
{
#अगर_अघोषित __U_BOOT__
	पूर्णांक ch;
#पूर्ण_अगर /* !__U_BOOT__ */

	/* There are two cases when i->p[] buffer exists.
	 * (1) it's a string in_str.
	 * (2) It's a file, and we have a saved line editing buffer.
	 * In both cases, we know that i->p[0] exists and not NUL, and
	 * the peek2 result is in i->p[1].
	 */
	अगर (i->p)
		वापस (अचिन्हित अक्षर)i->p[1];

#अगर_अघोषित __U_BOOT__
	/* Now we know it is a file-based in_str. */

	/* peek_buf[] is an int array, not char. Can contain EOF. */
	/* Is there 2nd char? */
	ch = i->peek_buf[1];
	अगर (ch == 0) {
		/* We did not read it yet, get it now */
		करो ch = hfgetc(i->file); जबतक (ch == '\0');
		i->peek_buf[1] = ch;
	}

	debug_printf("file_peek2: got '%c' %d\n", ch, ch);
	वापस ch;
#अन्यथा
	वापस 0;
#पूर्ण_अगर /* __U_BOOT__ */
}

अटल पूर्णांक i_getch_and_eat_bkslash_nl(काष्ठा in_str *निवेश)
{
	क्रम (;;) {
		पूर्णांक ch, ch2;

		ch = i_getch(निवेश);
		अगर (ch != '\\')
			वापस ch;
		ch2 = i_peek(निवेश);
		अगर (ch2 != '\n')
			वापस ch;
		/* backslash+newline, skip it */
		i_getch(निवेश);
	}
}

/* Note: this function _eats_ \<newline> pairs, safe to use plain
 * i_getch() after it instead of i_getch_and_eat_bkslash_nl().
 */
अटल पूर्णांक i_peek_and_eat_bkslash_nl(काष्ठा in_str *निवेश)
{
	क्रम (;;) {
		पूर्णांक ch, ch2;

		ch = i_peek(निवेश);
		अगर (ch != '\\')
			वापस ch;
		ch2 = i_peek2(निवेश);
		अगर (ch2 != '\n')
			वापस ch;
		/* backslash+newline, skip it */
		i_getch(निवेश);
		i_getch(निवेश);
	}
}

#अगर_अघोषित __U_BOOT__
अटल व्योम setup_file_in_str(काष्ठा in_str *i, HFILE *fp)
#अन्यथा /* __U_BOOT__ */
अटल व्योम setup_file_in_str(काष्ठा in_str *i)
#पूर्ण_अगर /* __U_BOOT__ */
{
	स_रखो(i, 0, माप(*i));
#अगर_अघोषित __U_BOOT__
	i->file = fp;
	/* i->p = NULL; */
#पूर्ण_अगर /* !__U_BOOT__ */
}

अटल व्योम setup_string_in_str(काष्ठा in_str *i, स्थिर अक्षर *s)
{
	स_रखो(i, 0, माप(*i));
	/*i->file = NULL */;
	i->p = s;
}

/*
 * o_string support
 */
#घोषणा B_CHUNK  (32 * माप(अक्षर*))

अटल व्योम o_reset_to_empty_unquoted(o_string *o)
{
	o->लंबाई = 0;
	o->has_quoted_part = 0;
	अगर (o->तथ्य)
		o->तथ्य[0] = '\0';
}

अटल व्योम o_free_and_set_NULL(o_string *o)
{
	मुक्त(o->तथ्य);
	स_रखो(o, 0, माप(*o));
}

अटल ALWAYS_INLINE व्योम o_free(o_string *o)
{
	मुक्त(o->तथ्य);
}

अटल व्योम o_grow_by(o_string *o, पूर्णांक len)
{
	अगर (o->लंबाई + len > o->maxlen) {
		o->maxlen += (2 * len) | (B_CHUNK-1);
		o->तथ्य = xrealloc(o->तथ्य, 1 + o->maxlen);
	}
}

अटल व्योम o_addchr(o_string *o, पूर्णांक ch)
{
	debug_printf("o_addchr: '%c' o->length=%d o=%p\n", ch, o->लंबाई, o);
	अगर (o->लंबाई < o->maxlen) {
		/* likely. avoid o_grow_by() call */
 add:
		o->तथ्य[o->लंबाई] = ch;
		o->लंबाई++;
		o->तथ्य[o->लंबाई] = '\0';
		वापस;
	}
	o_grow_by(o, 1);
	जाओ add;
}

#अगर 0
/* Valid only if we know o_string is not empty */
अटल व्योम o_delchr(o_string *o)
{
	o->लंबाई--;
	o->तथ्य[o->लंबाई] = '\0';
}
#पूर्ण_अगर

अटल व्योम o_addblock(o_string *o, स्थिर अक्षर *str, पूर्णांक len)
{
	o_grow_by(o, len);
	((अक्षर*)mempcpy(&o->तथ्य[o->लंबाई], str, len))[0] = '\0';
	o->लंबाई += len;
}

अटल व्योम o_addstr(o_string *o, स्थिर अक्षर *str)
{
	o_addblock(o, str, म_माप(str));
}

#अगर_अघोषित __U_BOOT__
अटल व्योम o_addstr_with_NUL(o_string *o, स्थिर अक्षर *str)
{
	o_addblock(o, str, म_माप(str) + 1);
}
#पूर्ण_अगर /* !__U_BOOT__ */

#अगर !BB_MMU
अटल व्योम nommu_addchr(o_string *o, पूर्णांक ch)
{
	अगर (o)
		o_addchr(o, ch);
}
#अन्यथा
# define nommu_addchr(o, str) ((व्योम)0)
#पूर्ण_अगर

#अगर_अघोषित __U_BOOT__
#अगर ENABLE_HUSH_MODE_X
अटल व्योम x_mode_addchr(पूर्णांक ch)
{
	o_addchr(&G.x_mode_buf, ch);
}
अटल व्योम x_mode_addstr(स्थिर अक्षर *str)
{
	o_addstr(&G.x_mode_buf, str);
}
अटल व्योम x_mode_addblock(स्थिर अक्षर *str, पूर्णांक len)
{
	o_addblock(&G.x_mode_buf, str, len);
}
अटल व्योम x_mode_prefix(व्योम)
{
	पूर्णांक n = G.x_mode_depth;
	करो x_mode_addchr('+'); जबतक (--n >= 0);
}
अटल व्योम x_mode_flush(व्योम)
{
	पूर्णांक len = G.x_mode_buf.लंबाई;
	अगर (len <= 0)
		वापस;
	अगर (G.x_mode_fd > 0) {
		G.x_mode_buf.तथ्य[len] = '\n';
		full_write(G.x_mode_fd, G.x_mode_buf.तथ्य, len + 1);
	}
	G.x_mode_buf.लंबाई = 0;
}
#पूर्ण_अगर
#पूर्ण_अगर /* !__U_BOOT__ */

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

#अगर ENABLE_HUSH_BRACE_EXPANSION
# define MAYBE_BRACES "{}"
#अन्यथा
# define MAYBE_BRACES ""
#पूर्ण_अगर

/* My analysis of quoting semantics tells me that state information
 * is associated with a destination, not a source.
 */
अटल व्योम o_addqchr(o_string *o, पूर्णांक ch)
{
	पूर्णांक sz = 1;
	/* '-' is included because of this case:
	 * >filename0 >filename1 >filename9; v='-'; echo filename[0"$v"9]
	 */
	अक्षर *found = म_अक्षर("*?[-\\" MAYBE_BRACES, ch);
	अगर (found)
		sz++;
	o_grow_by(o, sz);
	अगर (found) {
		o->तथ्य[o->लंबाई] = '\\';
		o->लंबाई++;
	}
	o->तथ्य[o->लंबाई] = ch;
	o->लंबाई++;
	o->तथ्य[o->लंबाई] = '\0';
}

अटल व्योम o_addQchr(o_string *o, पूर्णांक ch)
{
	पूर्णांक sz = 1;
	अगर ((o->o_expflags & EXP_FLAG_ESC_GLOB_CHARS)
	 && म_अक्षर("*?[-\\" MAYBE_BRACES, ch)
	) {
		sz++;
		o->तथ्य[o->लंबाई] = '\\';
		o->लंबाई++;
	}
	o_grow_by(o, sz);
	o->तथ्य[o->लंबाई] = ch;
	o->लंबाई++;
	o->तथ्य[o->लंबाई] = '\0';
}

अटल व्योम o_addqblock(o_string *o, स्थिर अक्षर *str, पूर्णांक len)
{
	जबतक (len) {
		अक्षर ch;
		पूर्णांक sz;
		पूर्णांक ordinary_cnt = म_खोज(str, "*?[-\\" MAYBE_BRACES);
		अगर (ordinary_cnt > len) /* paranoia */
			ordinary_cnt = len;
		o_addblock(o, str, ordinary_cnt);
		अगर (ordinary_cnt == len)
			वापस; /* NUL is already added by o_addblock */
		str += ordinary_cnt;
		len -= ordinary_cnt + 1; /* we are processing + 1 char below */

		ch = *str++;
		sz = 1;
		अगर (ch) { /* it is necessarily one of "*?[-\\" MAYBE_BRACES */
			sz++;
			o->तथ्य[o->लंबाई] = '\\';
			o->लंबाई++;
		}
		o_grow_by(o, sz);
		o->तथ्य[o->लंबाई] = ch;
		o->लंबाई++;
	}
	o->तथ्य[o->लंबाई] = '\0';
}

अटल व्योम o_addQblock(o_string *o, स्थिर अक्षर *str, पूर्णांक len)
{
	अगर (!(o->o_expflags & EXP_FLAG_ESC_GLOB_CHARS)) {
		o_addblock(o, str, len);
		वापस;
	}
	o_addqblock(o, str, len);
}

अटल व्योम o_addQstr(o_string *o, स्थिर अक्षर *str)
{
	o_addQblock(o, str, म_माप(str));
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
#अगर DEBUG_EXPAND || DEBUG_GLOB
अटल व्योम debug_print_list(स्थिर अक्षर *prefix, o_string *o, पूर्णांक n)
{
	अक्षर **list = (अक्षर**)o->तथ्य;
	पूर्णांक string_start = ((n + 0xf) & ~0xf) * माप(list[0]);
	पूर्णांक i = 0;

	indent();
	fdprintf(2, "%s: list:%p n:%d string_start:%d length:%d maxlen:%d glob:%d quoted:%d escape:%d\n",
			prefix, list, n, string_start, o->लंबाई, o->maxlen,
			!!(o->o_expflags & EXP_FLAG_GLOB),
			o->has_quoted_part,
			!!(o->o_expflags & EXP_FLAG_ESC_GLOB_CHARS));
	जबतक (i < n) {
		indent();
		fdprintf(2, " list[%d]=%d '%s' %p\n", i, (पूर्णांक)(uintptr_t)list[i],
				o->तथ्य + (पूर्णांक)(uintptr_t)list[i] + string_start,
				o->तथ्य + (पूर्णांक)(uintptr_t)list[i] + string_start);
		i++;
	}
	अगर (n) {
		स्थिर अक्षर *p = o->तथ्य + (पूर्णांक)(uintptr_t)list[n - 1] + string_start;
		indent();
#अगर_अघोषित __U_BOOT__
		fdprintf(2, " total_sz:%ld\n", (दीर्घ)((p + म_माप(p) + 1) - o->तथ्य));
#अन्यथा /* __U_BOOT__ */
		म_लिखो(" total_sz:%ld\n", (दीर्घ)((p + म_माप(p) + 1) - o->तथ्य));
#पूर्ण_अगर /* __U_BOOT__ */
	}
}
#अन्यथा
# define debug_print_list(prefix, o, n) ((व्योम)0)
#पूर्ण_अगर

/* n = o_save_ptr_helper(str, n) "starts new string" by storing an index value
 * in list[n] so that it points past last stored byte so far.
 * It returns n+1. */
अटल पूर्णांक o_save_ptr_helper(o_string *o, पूर्णांक n)
{
	अक्षर **list = (अक्षर**)o->तथ्य;
	पूर्णांक string_start;
	पूर्णांक string_len;

	अगर (!o->has_empty_slot) {
		string_start = ((n + 0xf) & ~0xf) * माप(list[0]);
		string_len = o->लंबाई - string_start;
		अगर (!(n & 0xf)) { /* 0, 0x10, 0x20...? */
			debug_printf_list("list[%d]=%d string_start=%d (growing)\n", n, string_len, string_start);
			/* list[n] points to string_start, make space for 16 more pointers */
			o->maxlen += 0x10 * माप(list[0]);
			o->तथ्य = xrealloc(o->तथ्य, o->maxlen + 1);
			list = (अक्षर**)o->तथ्य;
			स_हटाओ(list + n + 0x10, list + n, string_len);
			/*
			 * expand_on_ifs() has a "previous argv[] ends in IFS?"
			 * check. (grep for -prev-ifs-check-).
			 * Ensure that argv[-1][last] is not garbage
			 * but zero bytes, to save index check there.
			 */
			list[n + 0x10 - 1] = 0;
			o->लंबाई += 0x10 * माप(list[0]);
		} अन्यथा {
			debug_printf_list("list[%d]=%d string_start=%d\n",
					n, string_len, string_start);
		}
	} अन्यथा {
		/* We have empty slot at list[n], reuse without growth */
		string_start = ((n+1 + 0xf) & ~0xf) * माप(list[0]); /* NB: n+1! */
		string_len = o->लंबाई - string_start;
		debug_printf_list("list[%d]=%d string_start=%d (empty slot)\n",
				n, string_len, string_start);
		o->has_empty_slot = 0;
	}
	o->has_quoted_part = 0;
	list[n] = (अक्षर*)(uintptr_t)string_len;
	वापस n + 1;
}

/* "What was our last o_save_ptr'ed position (byte offset relative o->data)?" */
अटल पूर्णांक o_get_last_ptr(o_string *o, पूर्णांक n)
{
	अक्षर **list = (अक्षर**)o->तथ्य;
	पूर्णांक string_start = ((n + 0xf) & ~0xf) * माप(list[0]);

	वापस ((पूर्णांक)(uintptr_t)list[n-1]) + string_start;
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

#अगर ENABLE_HUSH_BRACE_EXPANSION
/* There in a GNU extension, GLOB_BRACE, but it is not usable:
 * first, it processes even {a} (no commas), second,
 * I didn't manage to make it return strings when they don't match
 * existing files. Need to re-implement it.
 */

/* Helper */
अटल पूर्णांक glob_needed(स्थिर अक्षर *s)
{
	जबतक (*s) {
		अगर (*s == '\\') {
			अगर (!s[1])
				वापस 0;
			s += 2;
			जारी;
		}
		अगर (*s == '*' || *s == '[' || *s == '?' || *s == '{')
			वापस 1;
		s++;
	}
	वापस 0;
}
/* Return pointer to next closing brace or to comma */
अटल स्थिर अक्षर *अग्र_brace_sub(स्थिर अक्षर *cp)
{
	अचिन्हित depth = 0;
	cp++;
	जबतक (*cp != '\0') {
		अगर (*cp == '\\') {
			अगर (*++cp == '\0')
				अवरोध;
			cp++;
			जारी;
		}
		अगर ((*cp == '}' && depth-- == 0) || (*cp == ',' && depth == 0))
			अवरोध;
		अगर (*cp++ == '{')
			depth++;
	}

	वापस *cp != '\0' ? cp : शून्य;
}
/* Recursive brace globber. Note: may garble pattern[]. */
अटल पूर्णांक glob_brace(अक्षर *pattern, o_string *o, पूर्णांक n)
{
	अक्षर *new_pattern_buf;
	स्थिर अक्षर *begin;
	स्थिर अक्षर *अग्र;
	स्थिर अक्षर *rest;
	स्थिर अक्षर *p;
	माप_प्रकार rest_len;

	debug_printf_glob("glob_brace('%s')\n", pattern);

	begin = pattern;
	जबतक (1) {
		अगर (*begin == '\0')
			जाओ simple_glob;
		अगर (*begin == '{') {
			/* Find the first sub-pattern and at the same time
			 * find the rest after the closing brace */
			अग्र = अग्र_brace_sub(begin);
			अगर (अग्र == शून्य) {
				/* An illegal expression */
				जाओ simple_glob;
			}
			अगर (*अग्र == '}') {
				/* "{abc}" with no commas - illegal
				 * brace expr, disregard and skip it */
				begin = अग्र + 1;
				जारी;
			}
			अवरोध;
		}
		अगर (*begin == '\\' && begin[1] != '\0')
			begin++;
		begin++;
	}
	debug_printf_glob("begin:%s\n", begin);
	debug_printf_glob("next:%s\n", अग्र);

	/* Now find the end of the whole brace expression */
	rest = अग्र;
	जबतक (*rest != '}') {
		rest = अग्र_brace_sub(rest);
		अगर (rest == शून्य) {
			/* An illegal expression */
			जाओ simple_glob;
		}
		debug_printf_glob("rest:%s\n", rest);
	}
	rest_len = म_माप(++rest) + 1;

	/* We are sure the brace expression is well-formed */

	/* Allocate working buffer large enough for our work */
	new_pattern_buf = xmalloc(म_माप(pattern));

	/* We have a brace expression.  BEGIN points to the opening {,
	 * NEXT points past the terminator of the first element, and REST
	 * points past the final }.  We will accumulate result names from
	 * recursive runs for each brace alternative in the buffer using
	 * GLOB_APPEND. */

	p = begin + 1;
	जबतक (1) {
		/* Construct the new glob expression */
		स_नकल(
			mempcpy(
				mempcpy(new_pattern_buf,
					/* We know the prefix for all sub-patterns */
					pattern, begin - pattern),
				p, अग्र - p),
			rest, rest_len);

		/* Note: glob_brace() may garble new_pattern_buf[].
		 * That's why we re-copy prefix every time (1st memcpy above).
		 */
		n = glob_brace(new_pattern_buf, o, n);
		अगर (*अग्र == '}') {
			/* We saw the last entry */
			अवरोध;
		}
		p = अग्र + 1;
		अग्र = अग्र_brace_sub(अग्र);
	}
	मुक्त(new_pattern_buf);
	वापस n;

 simple_glob:
	{
		पूर्णांक gr;
		glob_t globdata;

		स_रखो(&globdata, 0, माप(globdata));
		gr = glob(pattern, 0, शून्य, &globdata);
		debug_printf_glob("glob('%s'):%d\n", pattern, gr);
		अगर (gr != 0) {
			अगर (gr == GLOB_NOMATCH) {
				globfree(&globdata);
				/* NB: garbles parameter */
				unbackslash(pattern);
				o_addstr_with_NUL(o, pattern);
				debug_printf_glob("glob pattern '%s' is literal\n", pattern);
				वापस o_save_ptr_helper(o, n);
			}
			अगर (gr == GLOB_NOSPACE)
				bb_die_memory_exhausted();
			/* GLOB_ABORTED? Only happens with GLOB_ERR flag,
			 * but we didn't specify it. Paranoia again. */
			bb_error_संदेश_and_die("glob error %d on '%s'", gr, pattern);
		}
		अगर (globdata.gl_pathv && globdata.gl_pathv[0]) {
			अक्षर **argv = globdata.gl_pathv;
			जबतक (1) {
				o_addstr_with_NUL(o, *argv);
				n = o_save_ptr_helper(o, n);
				argv++;
				अगर (!*argv)
					अवरोध;
			}
		}
		globfree(&globdata);
	}
	वापस n;
}
/* Performs globbing on last list[],
 * saving each result as a new list[].
 */
अटल पूर्णांक perform_glob(o_string *o, पूर्णांक n)
{
	अक्षर *pattern, *copy;

	debug_printf_glob("start perform_glob: n:%d o->data:%p\n", n, o->तथ्य);
	अगर (!o->तथ्य)
		वापस o_save_ptr_helper(o, n);
	pattern = o->तथ्य + o_get_last_ptr(o, n);
	debug_printf_glob("glob pattern '%s'\n", pattern);
	अगर (!glob_needed(pattern)) {
		/* unbackslash last string in o in place, fix length */
		o->लंबाई = unbackslash(pattern) - o->तथ्य;
		debug_printf_glob("glob pattern '%s' is literal\n", pattern);
		वापस o_save_ptr_helper(o, n);
	}

	copy = xstrdup(pattern);
	/* "forget" pattern in o */
	o->लंबाई = pattern - o->तथ्य;
	n = glob_brace(copy, o, n);
	मुक्त(copy);
	अगर (DEBUG_GLOB)
		debug_print_list("perform_glob returning", o, n);
	वापस n;
}

#अन्यथा /* !HUSH_BRACE_EXPANSION */

/* Helper */
अटल पूर्णांक glob_needed(स्थिर अक्षर *s)
{
	जबतक (*s) {
		अगर (*s == '\\') {
			अगर (!s[1])
				वापस 0;
			s += 2;
			जारी;
		}
		अगर (*s == '*' || *s == '[' || *s == '?')
			वापस 1;
		s++;
	}
	वापस 0;
}
/* Performs globbing on last list[],
 * saving each result as a new list[].
 */
अटल पूर्णांक perform_glob(o_string *o, पूर्णांक n)
{
#अगर_अघोषित __U_BOOT__
	glob_t globdata;
	पूर्णांक gr;
#पूर्ण_अगर /* __U_BOOT__ */
	अक्षर *pattern;

	debug_printf_glob("start perform_glob: n:%d o->data:%p\n", n, o->तथ्य);
	अगर (!o->तथ्य)
		वापस o_save_ptr_helper(o, n);
	pattern = o->तथ्य + o_get_last_ptr(o, n);
	debug_printf_glob("glob pattern '%s'\n", pattern);
	अगर (!glob_needed(pattern)) {
#अगर_अघोषित __U_BOOT__
 literal:
#पूर्ण_अगर /* __U_BOOT__ */
		/* unbackslash last string in o in place, fix length */
		o->लंबाई = unbackslash(pattern) - o->तथ्य;
		debug_printf_glob("glob pattern '%s' is literal\n", pattern);
		वापस o_save_ptr_helper(o, n);
	}

#अगर_अघोषित __U_BOOT__
	स_रखो(&globdata, 0, माप(globdata));
	/* Can't use GLOB_NOCHECK: it does not unescape the string.
	 * If we glob "*.\*" and don't find anything, we need
	 * to fall back to using literal "*.*", but GLOB_NOCHECK
	 * will return "*.\*"!
	 */
	gr = glob(pattern, 0, शून्य, &globdata);
	debug_printf_glob("glob('%s'):%d\n", pattern, gr);
	अगर (gr != 0) {
		अगर (gr == GLOB_NOMATCH) {
			globfree(&globdata);
			जाओ literal;
		}
		अगर (gr == GLOB_NOSPACE)
			bb_die_memory_exhausted();
		/* GLOB_ABORTED? Only happens with GLOB_ERR flag,
		 * but we didn't specify it. Paranoia again. */
		bb_error_संदेश_and_die("glob error %d on '%s'", gr, pattern);
	}
	अगर (globdata.gl_pathv && globdata.gl_pathv[0]) {
		अक्षर **argv = globdata.gl_pathv;
		/* "forget" pattern in o */
		o->लंबाई = pattern - o->तथ्य;
		जबतक (1) {
			o_addstr_with_NUL(o, *argv);
			n = o_save_ptr_helper(o, n);
			argv++;
			अगर (!*argv)
				अवरोध;
		}
	}
	globfree(&globdata);
	अगर (DEBUG_GLOB)
		debug_print_list("perform_glob returning", o, n);
	वापस n;
#अन्यथा /* __U_BOOT__ */
	/*
	 * NOTE We only use perform glob to call unbackslash to remove backslash
	 * from string once expanded.
	 * So, it seems OK to return this if no previous return was done.
	 */
	वापस o_save_ptr_helper(o, n);
#पूर्ण_अगर /* __U_BOOT__ */
}

#पूर्ण_अगर /* !HUSH_BRACE_EXPANSION */

/* If o->o_expflags & EXP_FLAG_GLOB, glob the string so far remembered.
 * Otherwise, just finish current list[] and start new */
अटल पूर्णांक o_save_ptr(o_string *o, पूर्णांक n)
{
	अगर (o->o_expflags & EXP_FLAG_GLOB) {
		/* If o->has_empty_slot, list[n] was already globbed
		 * (if it was requested back then when it was filled)
		 * so don't do that again! */
		अगर (!o->has_empty_slot)
			वापस perform_glob(o, n); /* o_save_ptr_helper is inside */
	}
	वापस o_save_ptr_helper(o, n);
}

/* "Please convert list[n] to real char* ptrs, and NULL terminate it." */
अटल अक्षर **o_finalize_list(o_string *o, पूर्णांक n)
{
	अक्षर **list;
	पूर्णांक string_start;

	अगर (DEBUG_EXPAND)
		debug_print_list("finalized", o, n);
	debug_printf_expand("finalized n:%d\n", n);
	list = (अक्षर**)o->तथ्य;
	string_start = ((n + 0xf) & ~0xf) * माप(list[0]);
	list[--n] = शून्य;
	जबतक (n) {
		n--;
		list[n] = o->तथ्य + (पूर्णांक)(uintptr_t)list[n] + string_start;
	}
	वापस list;
}

अटल व्योम free_pipe_list(काष्ठा pipe *pi);

/* Returns pi->next - next pipe in the list */
अटल काष्ठा pipe *free_pipe(काष्ठा pipe *pi)
{
	काष्ठा pipe *अग्र;
	पूर्णांक i;

#अगर_अघोषित __U_BOOT__
	debug_printf_clean("free_pipe (pid %d)\n", getpid());
#पूर्ण_अगर /* !__U_BOOT__ */
	क्रम (i = 0; i < pi->अंक_cmds; i++) {
		काष्ठा command *command;
#अगर_अघोषित __U_BOOT__
		काष्ठा redir_struct *r, *rnext;
#पूर्ण_अगर /* !__U_BOOT__ */

		command = &pi->cmds[i];
		debug_printf_clean("  command %d:\n", i);
		अगर (command->argv) {
			अगर (DEBUG_CLEAN) {
				पूर्णांक a;
				अक्षर **p;
				क्रम (a = 0, p = command->argv; *p; a++, p++) {
					debug_printf_clean("   argv[%d] = %s\n", a, *p);
				}
			}
			free_strings(command->argv);
			//command->argv = NULL;
		}
		/* not "else if": on syntax error, we may have both! */
		अगर (command->group) {
			debug_printf_clean("   begin group (cmd_type:%d)\n",
					command->cmd_type);
			free_pipe_list(command->group);
			debug_printf_clean("   end group\n");
			//command->group = NULL;
		}
		/* else is crucial here.
		 * If group != NULL, child_func is meaningless */
#अगर ENABLE_HUSH_FUNCTIONS
		अन्यथा अगर (command->child_func) {
			debug_printf_exec("cmd %p releases child func at %p\n", command, command->child_func);
			command->child_func->parent_cmd = शून्य;
		}
#पूर्ण_अगर
#अगर !BB_MMU
		मुक्त(command->group_as_string);
		//command->group_as_string = NULL;
#पूर्ण_अगर
#अगर_अघोषित __U_BOOT__
		क्रम (r = command->redirects; r; r = rnext) {
			debug_printf_clean("   redirect %d%s",
					r->rd_fd, redir_table[r->rd_type].descrip);
			/* guard against the case >$FOO, where foo is unset or blank */
			अगर (r->rd_filename) {
				debug_printf_clean(" fname:'%s'\n", r->rd_filename);
				मुक्त(r->rd_filename);
				//r->rd_filename = NULL;
			}
			debug_printf_clean(" rd_dup:%d\n", r->rd_dup);
			rnext = r->अग्र;
			मुक्त(r);
		}
		//command->redirects = NULL;
#पूर्ण_अगर /* !__U_BOOT__ */
	}
	मुक्त(pi->cmds);   /* children are an array, they get freed all at once */
	//pi->cmds = NULL;
#अगर_अघोषित __U_BOOT__
#अगर ENABLE_HUSH_JOB
	मुक्त(pi->cmdtext);
	//pi->cmdtext = NULL;
#पूर्ण_अगर
#पूर्ण_अगर /* !__U_BOOT__ */

	अग्र = pi->अग्र;
	मुक्त(pi);
	वापस अग्र;
}

अटल व्योम free_pipe_list(काष्ठा pipe *pi)
{
	जबतक (pi) {
#अगर HAS_KEYWORDS
		debug_printf_clean("pipe reserved word %d\n", pi->res_word);
#पूर्ण_अगर
		debug_printf_clean("pipe followup code %d\n", pi->followup);
		pi = free_pipe(pi);
	}
}

/*** Parsing routines ***/

#अगर_अघोषित debug_print_वृक्ष
अटल व्योम debug_print_वृक्ष(काष्ठा pipe *pi, पूर्णांक lvl)
{
	अटल स्थिर अक्षर *स्थिर PIPE[] ALIGN_PTR = {
		[PIPE_SEQ] = "SEQ",
		[PIPE_AND] = "AND",
		[PIPE_OR ] = "OR" ,
		[PIPE_BG ] = "BG" ,
	};
	अटल स्थिर अक्षर *RES[] = {
		[RES_NONE ] = "NONE" ,
# अगर ENABLE_HUSH_IF
		[RES_IF   ] = "IF"   ,
		[RES_THEN ] = "THEN" ,
		[RES_ELIF ] = "ELIF" ,
		[RES_ELSE ] = "ELSE" ,
		[RES_FI   ] = "FI"   ,
# endif
# अगर ENABLE_HUSH_LOOPS
		[RES_FOR  ] = "FOR"  ,
		[RES_WHILE] = "WHILE",
		[RES_UNTIL] = "UNTIL",
		[RES_DO   ] = "DO"   ,
		[RES_DONE ] = "DONE" ,
# endif
# अगर ENABLE_HUSH_LOOPS || ENABLE_HUSH_CASE
		[RES_IN   ] = "IN"   ,
# endif
# अगर ENABLE_HUSH_CASE
		[RES_CASE ] = "CASE" ,
		[RES_CASE_IN ] = "CASE_IN" ,
		[RES_MATCH] = "MATCH",
		[RES_CASE_BODY] = "CASE_BODY",
		[RES_ESAC ] = "ESAC" ,
# endif
		[RES_XXXX ] = "XXXX" ,
		[RES_SNTX ] = "SNTX" ,
	};
	अटल स्थिर अक्षर *स्थिर CMDTYPE[] ALIGN_PTR = {
		"{}",
		"()",
		"[noglob]",
# अगर ENABLE_HUSH_FUNCTIONS
		"func()",
# endif
	};

	पूर्णांक pin, prn;

	pin = 0;
	जबतक (pi) {
		fdprintf(2, "%*spipe %d #cmds:%d %sres_word=%s followup=%d %s\n",
				lvl*2, "",
				pin,
				pi->अंक_cmds,
				(IF_HAS_KEYWORDS(pi->pi_inverted ? "! " :) ""),
				RES[pi->res_word],
				pi->followup, PIPE[pi->followup]
		);
		prn = 0;
		जबतक (prn < pi->अंक_cmds) {
			काष्ठा command *command = &pi->cmds[prn];
			अक्षर **argv = command->argv;

			fdprintf(2, "%*s cmd %d assignment_cnt:%d",
					lvl*2, "", prn,
					command->assignment_cnt);
# अगर ENABLE_HUSH_LINENO_VAR
			fdprintf(2, " LINENO:%u", command->lineno);
# endif
			अगर (command->group) {
				fdprintf(2, " group %s: (argv=%p)%s%s\n",
						CMDTYPE[command->cmd_type],
						argv
# अगर !BB_MMU
						, " group_as_string:", command->group_as_string
# अन्यथा
						, "", ""
# endif
				);
				debug_print_वृक्ष(command->group, lvl+1);
				prn++;
				जारी;
			}
			अगर (argv) जबतक (*argv) {
				fdprintf(2, " '%s'", *argv);
				argv++;
			}
#अगर_अघोषित __U_BOOT__
			अगर (command->redirects)
				fdprintf(2, " {redir}");
#पूर्ण_अगर /* __U_BOOT__ */
			fdprintf(2, "\n");
			prn++;
		}
		pi = pi->अग्र;
		pin++;
	}
}
#पूर्ण_अगर /* debug_print_tree */

अटल काष्ठा pipe *new_pipe(व्योम)
{
	काष्ठा pipe *pi;
	pi = xzalloc(माप(काष्ठा pipe));
	/*pi->res_word = RES_NONE; - RES_NONE is 0 anyway */
	वापस pi;
}

/* Command (member of a pipe) is complete, or we start a new pipe
 * if ctx->command is NULL.
 * No errors possible here.
 */
अटल पूर्णांक done_command(काष्ठा parse_context *ctx)
{
	/* The command is really already in the pipe structure, so
	 * advance the pipe counter and make a new, null command. */
	काष्ठा pipe *pi = ctx->pipe;
	काष्ठा command *command = ctx->command;

#अगर 0	/* Instead we emit error message at run time */
	अगर (ctx->pending_redirect) {
		/* For example, "cmd >" (no filename to redirect to) */
		syntax_error("invalid redirect");
		ctx->pending_redirect = शून्य;
	}
#पूर्ण_अगर

	अगर (command) {
		अगर (IS_NULL_CMD(command)) {
			debug_printf_parse("done_command: skipping null cmd, num_cmds=%d\n", pi->अंक_cmds);
			जाओ clear_and_ret;
		}
		pi->अंक_cmds++;
		debug_printf_parse("done_command: ++num_cmds=%d\n", pi->अंक_cmds);
		//debug_print_tree(ctx->list_head, 20);
	} अन्यथा {
		debug_printf_parse("done_command: initializing, num_cmds=%d\n", pi->अंक_cmds);
	}

	/* Only real trickiness here is that the uncommitted
	 * command structure is not counted in pi->num_cmds. */
	pi->cmds = xrealloc(pi->cmds, माप(*pi->cmds) * (pi->अंक_cmds+1));
	ctx->command = command = &pi->cmds[pi->अंक_cmds];
 clear_and_ret:
	स_रखो(command, 0, माप(*command));
#अगर ENABLE_HUSH_LINENO_VAR
	command->lineno = G.parse_lineno;
	debug_printf_parse("command->lineno = G.parse_lineno (%u)\n", G.parse_lineno);
#पूर्ण_अगर
	वापस pi->अंक_cmds; /* used only for 0/nonzero check */
}

अटल व्योम done_pipe(काष्ठा parse_context *ctx, pipe_style type)
{
	पूर्णांक not_null;

	debug_printf_parse("done_pipe entered, followup %d\n", type);
	/* Close previous command */
	not_null = done_command(ctx);
#अगर HAS_KEYWORDS
	ctx->pipe->pi_inverted = ctx->ctx_inverted;
	ctx->ctx_inverted = 0;
	ctx->pipe->res_word = ctx->ctx_res_w;
#पूर्ण_अगर
	अगर (type == PIPE_BG && ctx->list_शीर्ष != ctx->pipe) {
		/* Necessary since && and || have precedence over &:
		 * "cmd1 && cmd2 &" must spawn both cmds, not only cmd2,
		 * in a backgrounded subshell.
		 */
		काष्ठा pipe *pi;
		काष्ठा command *command;

		/* Is this actually this construct, all pipes end with && or ||? */
		pi = ctx->list_शीर्ष;
		जबतक (pi != ctx->pipe) {
			अगर (pi->followup != PIPE_AND && pi->followup != PIPE_OR)
				जाओ no_conv;
			pi = pi->अग्र;
		}

		debug_printf_parse("BG with more than one pipe, converting to { p1 &&...pN; } &\n");
		pi->followup = PIPE_SEQ; /* close pN _not_ with "&"! */
		pi = xzalloc(माप(*pi));
		pi->followup = PIPE_BG;
		pi->अंक_cmds = 1;
		pi->cmds = xzalloc(माप(pi->cmds[0]));
		command = &pi->cmds[0];
		अगर (CMD_NORMAL != 0) /* "if xzalloc didn't do that already" */
			command->cmd_type = CMD_NORMAL;
		command->group = ctx->list_शीर्ष;
#अगर !BB_MMU
		command->group_as_string = xstrndup(
			    ctx->as_string.तथ्य,
			    ctx->as_string.लंबाई - 1 /* do not copy last char, "&" */
		);
#पूर्ण_अगर
		/* Replace all pipes in ctx with one newly created */
		ctx->list_शीर्ष = ctx->pipe = pi;
		/* for cases like "cmd && &", do not be tricked by last command
		 * being null - the entire {...} & is NOT null! */
		not_null = 1;
	} अन्यथा {
 no_conv:
		ctx->pipe->followup = type;
	}

	/* Without this check, even just <enter> on command line generates
	 * tree of three NOPs (!). Which is harmless but annoying.
	 * IOW: it is safe to do it unconditionally. */
	अगर (not_null
#अगर ENABLE_HUSH_IF
	 || ctx->ctx_res_w == RES_FI
#पूर्ण_अगर
#अगर ENABLE_HUSH_LOOPS
	 || ctx->ctx_res_w == RES_DONE
	 || ctx->ctx_res_w == RES_FOR
	 || ctx->ctx_res_w == RES_IN
#पूर्ण_अगर
#अगर ENABLE_HUSH_CASE
	 || ctx->ctx_res_w == RES_ESAC
#पूर्ण_अगर
	) {
		काष्ठा pipe *new_p;
		debug_printf_parse("done_pipe: adding new pipe: "
#अगर_अघोषित __U_BOOT__
				"not_null:%d ctx->ctx_res_w:%d\n",
				not_null, ctx->ctx_res_w);
#अन्यथा /* __U_BOOT__ */
				"not_null:%d\n",
				not_null);
#पूर्ण_अगर /* __U_BOOT__ */
		new_p = new_pipe();
		ctx->pipe->अग्र = new_p;
		ctx->pipe = new_p;
		/* RES_THEN, RES_DO etc are "sticky" -
		 * they remain set for pipes inside if/while.
		 * This is used to control execution.
		 * RES_FOR and RES_IN are NOT sticky (needed to support
		 * cases where variable or value happens to match a keyword):
		 */
#अगर ENABLE_HUSH_LOOPS
		अगर (ctx->ctx_res_w == RES_FOR
		 || ctx->ctx_res_w == RES_IN)
			ctx->ctx_res_w = RES_NONE;
#पूर्ण_अगर
#अगर ENABLE_HUSH_CASE
		अगर (ctx->ctx_res_w == RES_MATCH)
			ctx->ctx_res_w = RES_CASE_BODY;
		अगर (ctx->ctx_res_w == RES_CASE)
			ctx->ctx_res_w = RES_CASE_IN;
#पूर्ण_अगर
		ctx->command = शून्य; /* trick done_command below */
		/* Create the memory for command, roughly:
		 * ctx->pipe->cmds = new struct command;
		 * ctx->command = &ctx->pipe->cmds[0];
		 */
		done_command(ctx);
		//debug_print_tree(ctx->list_head, 10);
	}
	debug_printf_parse("done_pipe return\n");
}

अटल व्योम initialize_context(काष्ठा parse_context *ctx)
{
	स_रखो(ctx, 0, माप(*ctx));
	अगर (MAYBE_ASSIGNMENT != 0)
		ctx->is_assignment = MAYBE_ASSIGNMENT;
	ctx->pipe = ctx->list_शीर्ष = new_pipe();
	/* Create the memory for command, roughly:
	 * ctx->pipe->cmds = new struct command;
	 * ctx->command = &ctx->pipe->cmds[0];
	 */
	done_command(ctx);
}

/* If a reserved word is found and processed, parse context is modified
 * and 1 is returned.
 */
#अगर HAS_KEYWORDS
काष्ठा reserved_combo {
	अक्षर literal[6];
	अचिन्हित अक्षर res;
	अचिन्हित अक्षर assignment_flag;
	uint32_t flag;
};
क्रमागत {
	FLAG_END   = (1 << RES_NONE ),
# अगर ENABLE_HUSH_IF
	FLAG_IF    = (1 << RES_IF   ),
	FLAG_THEN  = (1 << RES_THEN ),
	FLAG_ELIF  = (1 << RES_ELIF ),
	FLAG_ELSE  = (1 << RES_ELSE ),
	FLAG_FI    = (1 << RES_FI   ),
# endif
# अगर ENABLE_HUSH_LOOPS
	FLAG_FOR   = (1 << RES_FOR  ),
	FLAG_WHILE = (1 << RES_WHILE),
	FLAG_UNTIL = (1 << RES_UNTIL),
	FLAG_DO    = (1 << RES_DO   ),
	FLAG_DONE  = (1 << RES_DONE ),
	FLAG_IN    = (1 << RES_IN   ),
# endif
# अगर ENABLE_HUSH_CASE
	FLAG_MATCH = (1 << RES_MATCH),
	FLAG_ESAC  = (1 << RES_ESAC ),
# endif
	FLAG_START = (1 << RES_XXXX ),
};

अटल स्थिर काष्ठा reserved_combo* match_reserved_word(o_string *word)
{
	/* Mostly a list of accepted follow-up reserved words.
	 * FLAG_END means we are done with the sequence, and are ready
	 * to turn the compound list into a command.
	 * FLAG_START means the word must start a new compound list.
	 */
	अटल स्थिर काष्ठा reserved_combo reserved_list[] ALIGN4 = {
# अगर ENABLE_HUSH_IF
		{ "!",     RES_NONE,  NOT_ASSIGNMENT  , 0 },
		{ "if",    RES_IF,    MAYBE_ASSIGNMENT, FLAG_THEN | FLAG_START },
		{ "then",  RES_THEN,  MAYBE_ASSIGNMENT, FLAG_ELIF | FLAG_ELSE | FLAG_FI },
		{ "elif",  RES_ELIF,  MAYBE_ASSIGNMENT, FLAG_THEN },
		{ "else",  RES_ELSE,  MAYBE_ASSIGNMENT, FLAG_FI   },
		{ "fi",    RES_FI,    NOT_ASSIGNMENT  , FLAG_END  },
# endif
# अगर ENABLE_HUSH_LOOPS
		{ "for",   RES_FOR,   NOT_ASSIGNMENT  , FLAG_IN | FLAG_DO | FLAG_START },
		{ "while", RES_WHILE, MAYBE_ASSIGNMENT, FLAG_DO | FLAG_START },
		{ "until", RES_UNTIL, MAYBE_ASSIGNMENT, FLAG_DO | FLAG_START },
		{ "in",    RES_IN,    NOT_ASSIGNMENT  , FLAG_DO   },
		{ "do",    RES_DO,    MAYBE_ASSIGNMENT, FLAG_DONE },
		{ "done",  RES_DONE,  NOT_ASSIGNMENT  , FLAG_END  },
# endif
# अगर ENABLE_HUSH_CASE
		{ "case",  RES_CASE,  NOT_ASSIGNMENT  , FLAG_MATCH | FLAG_START },
		{ "esac",  RES_ESAC,  NOT_ASSIGNMENT  , FLAG_END  },
# endif
	};
	स्थिर काष्ठा reserved_combo *r;

	क्रम (r = reserved_list; r < reserved_list + ARRAY_आकार(reserved_list); r++) {
		अगर (म_भेद(word->तथ्य, r->literal) == 0)
			वापस r;
	}
	वापस शून्य;
}
/* Return NULL: not a keyword, else: keyword
 */
अटल स्थिर काष्ठा reserved_combo* reserved_word(काष्ठा parse_context *ctx)
{
# अगर ENABLE_HUSH_CASE
	अटल स्थिर काष्ठा reserved_combo reserved_match = {
		"",        RES_MATCH, NOT_ASSIGNMENT , FLAG_MATCH | FLAG_ESAC
	};
# endif
	स्थिर काष्ठा reserved_combo *r;

	अगर (ctx->word.has_quoted_part)
		वापस 0;
	r = match_reserved_word(&ctx->word);
	अगर (!r)
		वापस r; /* NULL */

	debug_printf("found reserved word %s, res %d\n", r->literal, r->res);
# अगर ENABLE_HUSH_CASE
	अगर (r->res == RES_IN && ctx->ctx_res_w == RES_CASE_IN) {
		/* "case word IN ..." - IN part starts first MATCH part */
		r = &reserved_match;
	} अन्यथा
# endif
	अगर (r->flag == 0) { /* '!' */
		अगर (ctx->ctx_inverted) { /* bash doesn't accept '! ! true' */
			syntax_error("! ! command");
			ctx->ctx_res_w = RES_SNTX;
		}
		ctx->ctx_inverted = 1;
		वापस r;
	}
	अगर (r->flag & FLAG_START) {
		काष्ठा parse_context *old;

		old = xmemdup(ctx, माप(*ctx));
		debug_printf_parse("push stack %p\n", old);
		initialize_context(ctx);
		ctx->ढेर = old;
	} अन्यथा अगर (/*ctx->ctx_res_w == RES_NONE ||*/ !(ctx->old_flag & (1 << r->res))) {
		syntax_error_at(ctx->word.तथ्य);
		ctx->ctx_res_w = RES_SNTX;
		वापस r;
	} अन्यथा {
		/* "{...} fi" is ok. "{...} if" is not
		 * Example:
		 * if { echo foo; } then { echo bar; } fi */
		अगर (ctx->command->group)
			done_pipe(ctx, PIPE_SEQ);
	}

	ctx->ctx_res_w = r->res;
	ctx->old_flag = r->flag;
	ctx->is_assignment = r->assignment_flag;
	debug_printf_parse("ctx->is_assignment='%s'\n", assignment_flag[ctx->is_assignment]);

	अगर (ctx->old_flag & FLAG_END) {
		काष्ठा parse_context *old;

		done_pipe(ctx, PIPE_SEQ);
		debug_printf_parse("pop stack %p\n", ctx->ढेर);
		old = ctx->ढेर;
		old->command->group = ctx->list_शीर्ष;
		old->command->cmd_type = CMD_NORMAL;
# अगर !BB_MMU
		/* At this point, the compound command's string is in
		 * ctx->as_string... except for the leading keyword!
		 * Consider this example: "echo a | if true; then echo a; fi"
		 * ctx->as_string will contain "true; then echo a; fi",
		 * with "if " remaining in old->as_string!
		 */
		{
			अक्षर *str;
			पूर्णांक len = old->as_string.लंबाई;
			/* Concatenate halves */
			o_addstr(&old->as_string, ctx->as_string.तथ्य);
			o_free(&ctx->as_string);
			/* Find where leading keyword starts in first half */
			str = old->as_string.तथ्य + len;
			अगर (str > old->as_string.तथ्य)
				str--; /* skip whitespace after keyword */
			जबतक (str > old->as_string.तथ्य && है_अक्षर(str[-1]))
				str--;
			/* Ugh, we're done with this horrid hack */
			old->command->group_as_string = xstrdup(str);
			debug_printf_parse("pop, remembering as:'%s'\n",
					old->command->group_as_string);
		}
# endif
		*ctx = *old;   /* physical copy */
		मुक्त(old);
	}
	वापस r;
}
#पूर्ण_अगर /* HAS_KEYWORDS */

/* Word is complete, look at it and update parsing context.
 * Normal return is 0. Syntax errors return 1.
 * Note: on return, word is reset, but not o_free'd!
 */
अटल पूर्णांक done_word(काष्ठा parse_context *ctx)
{
	काष्ठा command *command = ctx->command;

	debug_printf_parse("done_word entered: '%s' %p\n", ctx->word.तथ्य, command);
	अगर (ctx->word.लंबाई == 0 && !ctx->word.has_quoted_part) {
		debug_printf_parse("done_word return 0: true null, ignored\n");
		वापस 0;
	}

#अगर_अघोषित __U_BOOT__
	अगर (ctx->pending_redirect) {
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

		ctx->pending_redirect->rd_filename = xstrdup(ctx->word.तथ्य);
		/* Cater for >\file case:
		 * >\a creates file a; >\\a, >"\a", >"\\a" create file \a
		 * Same with heredocs:
		 * for <<\H delim is H; <<\\H, <<"\H", <<"\\H" - \H
		 */
		अगर (ctx->pending_redirect->rd_type == REDIRECT_HEREDOC) {
			unbackslash(ctx->pending_redirect->rd_filename);
			/* Is it <<"HEREDOC"? */
			अगर (ctx->word.has_quoted_part) {
				ctx->pending_redirect->rd_dup |= HEREDOC_QUOTED;
			}
		}
		debug_printf_parse("word stored in rd_filename: '%s'\n", ctx->word.तथ्य);
		ctx->pending_redirect = शून्य;
	} अन्यथा {
#पूर्ण_अगर /* !__U_BOOT__ */
#अगर HAS_KEYWORDS
# अगर ENABLE_HUSH_CASE
		अगर (ctx->ctx_dsemicolon
		 && म_भेद(ctx->word.तथ्य, "esac") != 0 /* not "... pattern) cmd;; esac" */
		) {
			/* already done when ctx_dsemicolon was set to 1: */
			/* ctx->ctx_res_w = RES_MATCH; */
			ctx->ctx_dsemicolon = 0;
		} अन्यथा
# endif
# अगर defined(CMD_TEST2_SINGLEWORD_NOGLOB)
		अगर (command->cmd_type == CMD_TEST2_SINGLEWORD_NOGLOB
		 && म_भेद(ctx->word.तथ्य, "]]") == 0
		) {
			/* allow "[[ ]] >file" etc */
			command->cmd_type = CMD_SINGLEWORD_NOGLOB;
		} अन्यथा
# endif
		अगर (!command->argv /* if it's the first word... */
# अगर ENABLE_HUSH_LOOPS
		 && ctx->ctx_res_w != RES_FOR /* ...not after FOR or IN */
		 && ctx->ctx_res_w != RES_IN
# endif
# अगर ENABLE_HUSH_CASE
		 && ctx->ctx_res_w != RES_CASE
# endif
		) {
			स्थिर काष्ठा reserved_combo *reserved;
			reserved = reserved_word(ctx);
			debug_printf_parse("checking for reserved-ness: %d\n", !!reserved);
			अगर (reserved) {
# अगर ENABLE_HUSH_LINENO_VAR
/* Case:
 * "while ...; do
 *	cmd ..."
 * If we don't close the pipe _now_, immediately after "do", lineno logic
 * sees "cmd" as starting at "do" - i.e., at the previous line.
 */
				अगर (0
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
				वापस (ctx->ctx_res_w == RES_SNTX);
			}
# अगर defined(CMD_TEST2_SINGLEWORD_NOGLOB)
			अगर (म_भेद(ctx->word.तथ्य, "[[") == 0) {
				command->cmd_type = CMD_TEST2_SINGLEWORD_NOGLOB;
			} अन्यथा
# endif
# अगर defined(CMD_SINGLEWORD_NOGLOB)
			अगर (0
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
			 IF_HUSH_LOCAL(   || म_भेद(ctx->word.तथ्य, "local") == 0)
			 IF_HUSH_EXPORT(  || म_भेद(ctx->word.तथ्य, "export") == 0)
			 IF_HUSH_READONLY(|| म_भेद(ctx->word.तथ्य, "readonly") == 0)
			) {
				command->cmd_type = CMD_SINGLEWORD_NOGLOB;
			}
# अन्यथा
			{ /* empty block to pair "if ... else" */ }
# endif
		}
#पूर्ण_अगर /* HAS_KEYWORDS */

		अगर (command->group) {
			/* "{ echo foo; } echo bar" - bad */
			syntax_error_at(ctx->word.तथ्य);
			debug_printf_parse("done_word return 1: syntax error, "
					"groups and arglists don't mix\n");
			वापस 1;
		}

		/* If this word wasn't an assignment, next ones definitely
		 * can't be assignments. Even if they look like ones. */
		अगर (ctx->is_assignment != DEFINITELY_ASSIGNMENT
		 && ctx->is_assignment != WORD_IS_KEYWORD
		) {
			ctx->is_assignment = NOT_ASSIGNMENT;
		} अन्यथा {
			अगर (ctx->is_assignment == DEFINITELY_ASSIGNMENT) {
				command->assignment_cnt++;
				debug_printf_parse("++assignment_cnt=%d\n", command->assignment_cnt);
			}
			debug_printf_parse("ctx->is_assignment was:'%s'\n", assignment_flag[ctx->is_assignment]);
			ctx->is_assignment = MAYBE_ASSIGNMENT;
		}
		debug_printf_parse("ctx->is_assignment='%s'\n", assignment_flag[ctx->is_assignment]);
		command->argv = add_string_to_strings(command->argv, xstrdup(ctx->word.तथ्य));
#अगर_घोषित __U_BOOT__
		command->argc++;
#पूर्ण_अगर /* __U_BOOT__ */
		debug_print_strings("word appended to argv", command->argv);

#अगर_अघोषित __U_BOOT__
	}
#पूर्ण_अगर /* !__U_BOOT__ */

#अगर ENABLE_HUSH_LOOPS
	अगर (ctx->ctx_res_w == RES_FOR) {
		अगर (ctx->word.has_quoted_part
		 || endofname(command->argv[0])[0] != '\0'
		) {
			/* bash says just "not a valid identifier" */
			syntax_error("bad for loop variable");
			वापस 1;
		}
		/* Force FOR to have just one word (variable name) */
		/* NB: basically, this makes hush see "for v in ..."
		 * syntax as if it is "for v; in ...". FOR and IN become
		 * two pipe structs in parse tree. */
		done_pipe(ctx, PIPE_SEQ);
	}
#पूर्ण_अगर
#अगर ENABLE_HUSH_CASE
	/* Force CASE to have just one word */
	अगर (ctx->ctx_res_w == RES_CASE) {
		done_pipe(ctx, PIPE_SEQ);
	}
#पूर्ण_अगर

	o_reset_to_empty_unquoted(&ctx->word);

	debug_printf_parse("done_word return 0\n");
	वापस 0;
}

#अगर_अघोषित __U_BOOT__
/* Peek ahead in the input to find out if we have a "&n" construct,
 * as in "2>&1", that represents duplicating a file descriptor.
 * Return:
 * REDIRFD_CLOSE if >&- "close fd" construct is seen,
 * REDIRFD_SYNTAX_ERR if syntax error,
 * REDIRFD_TO_FILE if no & was seen,
 * or the number found.
 */
#अगर BB_MMU
#घोषणा parse_redir_दायाँ_fd(as_string, निवेश) \
	parse_redir_दायाँ_fd(निवेश)
#पूर्ण_अगर
अटल पूर्णांक parse_redir_दायाँ_fd(o_string *as_string, काष्ठा in_str *निवेश)
{
	पूर्णांक ch, d, ok;

	ch = i_peek(निवेश);
	अगर (ch != '&')
		वापस REDIRFD_TO_FILE;

	ch = i_getch(निवेश);  /* get the & */
	nommu_addchr(as_string, ch);
	ch = i_peek(निवेश);
	अगर (ch == '-') {
		ch = i_getch(निवेश);
		nommu_addchr(as_string, ch);
		वापस REDIRFD_CLOSE;
	}
	d = 0;
	ok = 0;
	जबतक (ch != खातापूर्ण && है_अंक(ch)) {
		d = d*10 + (ch-'0');
		ok = 1;
		ch = i_getch(निवेश);
		nommu_addchr(as_string, ch);
		ch = i_peek(निवेश);
	}
	अगर (ok) वापस d;

//TODO: this is the place to catch ">&file" bashism (redirect both fd 1 and 2)

	bb_simple_error_संदेश("ambiguous redirect");
	वापस REDIRFD_SYNTAX_ERR;
}

/* Return code is 0 normal, 1 if a syntax error is detected
 */
अटल पूर्णांक parse_redirect(काष्ठा parse_context *ctx,
		पूर्णांक fd,
		redir_type style,
		काष्ठा in_str *निवेश)
{
	काष्ठा command *command = ctx->command;
	काष्ठा redir_struct *redir;
	काष्ठा redir_struct **redirp;
	पूर्णांक dup_अंक;

	dup_अंक = REDIRFD_TO_FILE;
	अगर (style != REDIRECT_HEREDOC) {
		/* Check for a '>&1' type redirect */
		dup_अंक = parse_redir_दायाँ_fd(&ctx->as_string, निवेश);
		अगर (dup_अंक == REDIRFD_SYNTAX_ERR)
			वापस 1;
	} अन्यथा {
		पूर्णांक ch = i_peek_and_eat_bkslash_nl(निवेश);
		dup_अंक = (ch == '-'); /* HEREDOC_SKIPTABS bit is 1 */
		अगर (dup_अंक) { /* <<-... */
			ch = i_getch(निवेश);
			nommu_addchr(&ctx->as_string, ch);
			ch = i_peek(निवेश);
		}
	}

	अगर (style == REDIRECT_OVERWRITE && dup_अंक == REDIRFD_TO_FILE) {
		पूर्णांक ch = i_peek_and_eat_bkslash_nl(निवेश);
		अगर (ch == '|') {
			/* >|FILE redirect ("clobbering" >).
			 * Since we do not support "set -o noclobber" yet,
			 * >| and > are the same for now. Just eat |.
			 */
			ch = i_getch(निवेश);
			nommu_addchr(&ctx->as_string, ch);
		}
	}

	/* Create a new redir_struct and append it to the linked list */
	redirp = &command->redirects;
	जबतक ((redir = *redirp) != शून्य) {
		redirp = &(redir->अग्र);
	}
	*redirp = redir = xzalloc(माप(*redir));
	/* redir->next = NULL; */
	/* redir->rd_filename = NULL; */
	redir->rd_type = style;
	redir->rd_fd = (fd == -1) ? redir_table[style].default_fd : fd;

	debug_printf_parse("redirect type %d %s\n", redir->rd_fd,
				redir_table[style].descrip);

	redir->rd_dup = dup_अंक;
	अगर (style != REDIRECT_HEREDOC && dup_अंक != REDIRFD_TO_FILE) {
		/* Erik had a check here that the file descriptor in question
		 * is legit; I postpone that to "run time"
		 * A "-" representation of "close me" shows up as a -3 here */
		debug_printf_parse("duplicating redirect '%d>&%d'\n",
				redir->rd_fd, redir->rd_dup);
	} अन्यथा {
#अगर 0		/* Instead we emit error message at run time */
		अगर (ctx->pending_redirect) {
			/* For example, "cmd > <file" */
			syntax_error("invalid redirect");
		}
#पूर्ण_अगर
		/* Set ctx->pending_redirect, so we know what to do at the
		 * end of the next parsed word. */
		ctx->pending_redirect = redir;
	}
	वापस 0;
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
अटल पूर्णांक redirect_opt_अंक(o_string *o)
{
	पूर्णांक अंक;

	अगर (o->तथ्य == शून्य)
		वापस -1;
	अंक = bb_strtou(o->तथ्य, शून्य, 10);
	अगर (त्रुटि_सं || अंक < 0)
		वापस -1;
	o_reset_to_empty_unquoted(o);
	वापस अंक;
}

#अगर BB_MMU
#घोषणा fetch_till_str(as_string, निवेश, word, skip_tabs) \
	fetch_till_str(निवेश, word, skip_tabs)
#पूर्ण_अगर
अटल अक्षर *fetch_till_str(o_string *as_string,
		काष्ठा in_str *निवेश,
		स्थिर अक्षर *word,
		पूर्णांक heredoc_flags)
{
	o_string heredoc = NULL_O_STRING;
	अचिन्हित past_EOL;
	पूर्णांक पूर्व = 0; /* not \ */
	पूर्णांक ch;

	/* Starting with "" is necessary for this case:
	 * cat <<EOF
	 *
	 * xxx
	 * EOF
	 */
	heredoc.तथ्य = xzalloc(1); /* start as "", not as NULL */

	जाओ jump_in;

	जबतक (1) {
		ch = i_getch(निवेश);
		अगर (ch != खातापूर्ण)
			nommu_addchr(as_string, ch);
		अगर (ch == '\n' || ch == खातापूर्ण) {
 check_heredoc_end:
#अगर_अघोषित __U_BOOT__
			अगर ((heredoc_flags & HEREDOC_QUOTED) || पूर्व != '\\') {
#अन्यथा /* __U_BOOT__ */
			अगर (पूर्व != '\\') {
#पूर्ण_अगर
				/* End-of-line, and not a line continuation */
				अगर (म_भेद(heredoc.तथ्य + past_EOL, word) == 0) {
					heredoc.तथ्य[past_EOL] = '\0';
					debug_printf_heredoc("parsed '%s' heredoc '%s'\n", word, heredoc.तथ्य);
					वापस heredoc.तथ्य;
				}
				अगर (ch == '\n') {
					/* This is a new line.
					 * Remember position and backslash-escaping status.
					 */
					o_addchr(&heredoc, ch);
					पूर्व = ch;
 jump_in:
					past_EOL = heredoc.लंबाई;
					/* Get 1st char of next line, possibly skipping leading tabs */
					करो {
						ch = i_getch(निवेश);
						अगर (ch != खातापूर्ण)
							nommu_addchr(as_string, ch);
#अगर_अघोषित __U_BOOT__
					} जबतक ((heredoc_flags & HEREDOC_SKIPTABS) && ch == '\t');
#अन्यथा /* __U_BOOT__ */
				} जबतक (ch == '\t');
#पूर्ण_अगर
					/* If this immediately ended the line,
					 * go back to end-of-line checks.
					 */
					अगर (ch == '\n')
						जाओ check_heredoc_end;
				}
			} अन्यथा {
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
				heredoc.तथ्य[--heredoc.लंबाई] = '\0';
				पूर्व = 0; /* not '\' */
				जारी;
			}
		}
		अगर (ch == खातापूर्ण) {
			o_free(&heredoc);
			वापस शून्य; /* error */
		}
		o_addchr(&heredoc, ch);
		nommu_addchr(as_string, ch);
		अगर (पूर्व == '\\' && ch == '\\')
			/* Correctly handle foo\\<eol> (not a line cont.) */
			पूर्व = 0; /* not '\' */
		अन्यथा
			पूर्व = ch;
	}
}
#पूर्ण_अगर /* !__U_BOOT__ */

/* Look at entire parse tree for not-yet-loaded REDIRECT_HEREDOCs
 * and load them all. There should be exactly heredoc_cnt of them.
 */
#अगर BB_MMU
#घोषणा fetch_heredocs(as_string, pi, heredoc_cnt, निवेश) \
	fetch_heredocs(pi, heredoc_cnt, निवेश)
#पूर्ण_अगर
अटल पूर्णांक fetch_heredocs(o_string *as_string, काष्ठा pipe *pi, पूर्णांक heredoc_cnt, काष्ठा in_str *निवेश)
{
	जबतक (pi && heredoc_cnt) {
		पूर्णांक i;
		काष्ठा command *cmd = pi->cmds;

		debug_printf_heredoc("fetch_heredocs: num_cmds:%d cmd argv0:'%s'\n",
				pi->अंक_cmds,
				cmd->argv ? cmd->argv[0] : "NONE"
		);
		क्रम (i = 0; i < pi->अंक_cmds; i++) {
#अगर_अघोषित __U_BOOT__
			काष्ठा redir_struct *redir = cmd->redirects;

#पूर्ण_अगर /* !__U_BOOT__ */
			debug_printf_heredoc("fetch_heredocs: %d cmd argv0:'%s'\n",
					i, cmd->argv ? cmd->argv[0] : "NONE");
#अगर_अघोषित __U_BOOT__
			जबतक (redir) {
				अगर (redir->rd_type == REDIRECT_HEREDOC) {
					अक्षर *p;

					redir->rd_type = REDIRECT_HEREDOC2;
					/* redir->rd_dup is (ab)used to indicate <<- */
					p = fetch_till_str(as_string, निवेश,
							redir->rd_filename, redir->rd_dup);
					अगर (!p) {
						syntax_error("unexpected EOF in here document");
						वापस -1;
					}
					मुक्त(redir->rd_filename);
					redir->rd_filename = p;
					heredoc_cnt--;
				}
				redir = redir->अग्र;
			}
#पूर्ण_अगर /* !__U_BOOT__ */
			अगर (cmd->group) {
				//bb_error_msg("%s:%u heredoc_cnt:%d", __func__, __LINE__, heredoc_cnt);
				heredoc_cnt = fetch_heredocs(as_string, cmd->group, heredoc_cnt, निवेश);
				//bb_error_msg("%s:%u heredoc_cnt:%d", __func__, __LINE__, heredoc_cnt);
				अगर (heredoc_cnt < 0)
					वापस heredoc_cnt; /* error */
			}
			cmd++;
		}
		pi = pi->अग्र;
	}
	वापस heredoc_cnt;
}

अटल पूर्णांक run_list(काष्ठा pipe *pi);
#अगर BB_MMU
#घोषणा parse_stream(pstring, heredoc_cnt_ptr, निवेश, end_trigger) \
	parse_stream(heredoc_cnt_ptr, निवेश, end_trigger)
#पूर्ण_अगर
अटल काष्ठा pipe *parse_stream(अक्षर **pstring,
		पूर्णांक *heredoc_cnt_ptr,
		काष्ठा in_str *निवेश,
		पूर्णांक end_trigger);

/* Returns number of heredocs not yet consumed,
 * or -1 on error.
 */
अटल पूर्णांक parse_group(काष्ठा parse_context *ctx,
		काष्ठा in_str *निवेश, पूर्णांक ch)
{
	/* ctx->word contains characters seen prior to ( or {.
	 * Typically it's empty, but for function defs,
	 * it contains function name (without '()'). */
#अगर BB_MMU
# define as_string शून्य
#अन्यथा
	अक्षर *as_string = शून्य;
#पूर्ण_अगर
	काष्ठा pipe *pipe_list;
	पूर्णांक heredoc_cnt = 0;
	पूर्णांक endch;
	काष्ठा command *command = ctx->command;

	debug_printf_parse("parse_group entered\n");
#अगर ENABLE_HUSH_FUNCTIONS
	अगर (ch == '(' && !ctx->word.has_quoted_part) {
		अगर (ctx->word.लंबाई)
			अगर (done_word(ctx))
				वापस -1;
		अगर (!command->argv)
			जाओ skip; /* (... */
		अगर (command->argv[1]) { /* word word ... (... */
			syntax_error_unexpected_ch('(');
			वापस -1;
		}
		/* it is "word(..." or "word (..." */
		करो
			ch = i_getch(निवेश);
		जबतक (ch == ' ' || ch == '\t');
		अगर (ch != ')') {
			syntax_error_unexpected_ch(ch);
			वापस -1;
		}
		nommu_addchr(&ctx->as_string, ch);
		करो
			ch = i_getch(निवेश);
		जबतक (ch == ' ' || ch == '\t' || ch == '\n');
		अगर (ch != '{' && ch != '(') {
			syntax_error_unexpected_ch(ch);
			वापस -1;
		}
//bash allows functions named "123", "..", "return"!
//		if (endofname(command->argv[0])[0] != '\0') {
//			syntax_error("bad function name");
//			return -1;
//		}
		nommu_addchr(&ctx->as_string, ch);
		command->cmd_type = CMD_FUNCDEF;
		जाओ skip;
	}
#पूर्ण_अगर

#अगर 0 /* Prevented by caller */
	अगर (command->argv /* word [word]{... */
	 || ctx->word.लंबाई /* word{... */
	 || ctx->word.has_quoted_part /* ""{... */
	) {
		syntax_error(शून्य);
		debug_printf_parse("parse_group return -1: "
			"syntax error, groups and arglists don't mix\n");
		वापस -1;
	}
#पूर्ण_अगर

#अगर_अघोषित __U_BOOT__
 IF_HUSH_FUNCTIONS(skip:)
#पूर्ण_अगर /* !__U_BOOT__ */

	endch = '}';
	अगर (ch == '(') {
		endch = ')';
#अगर_अघोषित __U_BOOT__
		IF_HUSH_FUNCTIONS(अगर (command->cmd_type != CMD_FUNCDEF))
			command->cmd_type = CMD_SUBSHELL;
#पूर्ण_अगर /* !__U_BOOT__ */
	} अन्यथा {
		/* bash does not allow "{echo...", requires whitespace */
		ch = i_peek(निवेश);
		अगर (ch != ' ' && ch != '\t' && ch != '\n'
		 && ch != '('	/* but "{(..." is allowed (without whitespace) */
		) {
			syntax_error_unexpected_ch(ch);
			वापस -1;
		}
		अगर (ch != '(') {
			ch = i_getch(निवेश);
			nommu_addchr(&ctx->as_string, ch);
		}
	}

	debug_printf_heredoc("calling parse_stream, heredoc_cnt:%d\n", heredoc_cnt);
	pipe_list = parse_stream(&as_string, &heredoc_cnt, निवेश, endch);
	debug_printf_heredoc("parse_stream returned: heredoc_cnt:%d\n", heredoc_cnt);
#अगर !BB_MMU
	अगर (as_string)
		o_addstr(&ctx->as_string, as_string);
#पूर्ण_अगर

	/* empty ()/{} or parse error? */
	अगर (!pipe_list || pipe_list == ERR_PTR) {
		/* parse_stream already emitted error msg */
		अगर (!BB_MMU)
			मुक्त(as_string);
		debug_printf_parse("parse_group return -1: "
			"parse_stream returned %p\n", pipe_list);
		वापस -1;
	}
#अगर !BB_MMU
	as_string[म_माप(as_string) - 1] = '\0'; /* plink ')' or '}' */
	command->group_as_string = as_string;
	debug_printf_parse("end of group, remembering as:'%s'\n",
			command->group_as_string);
#पूर्ण_अगर

#अगर ENABLE_HUSH_FUNCTIONS
	/* Convert "f() (cmds)" to "f() {(cmds)}" */
	अगर (command->cmd_type == CMD_FUNCDEF && endch == ')') {
		काष्ठा command *cmd2;

		cmd2 = xzalloc(माप(*cmd2));
		cmd2->cmd_type = CMD_SUBSHELL;
		cmd2->group = pipe_list;
# अगर !BB_MMU
//UNTESTED!
		cmd2->group_as_string = command->group_as_string;
		command->group_as_string = xasprintf("(%s)", command->group_as_string);
# endif

		pipe_list = new_pipe();
		pipe_list->cmds = cmd2;
		pipe_list->अंक_cmds = 1;
	}
#पूर्ण_अगर

	command->group = pipe_list;

	debug_printf_parse("parse_group return %d\n", heredoc_cnt);
	वापस heredoc_cnt;
	/* command remains "open", available for possible redirects */
#अघोषित as_string
}

#अगर_अघोषित __U_BOOT__
#अगर ENABLE_HUSH_TICK || ENABLE_FEATURE_SH_MATH || ENABLE_HUSH_DOLLAR_OPS
/* Subroutines for copying $(...) and `...` things */
/* '...' */
अटल पूर्णांक add_till_single_quote(o_string *dest, काष्ठा in_str *निवेश)
{
	जबतक (1) {
		पूर्णांक ch = i_getch(निवेश);
		अगर (ch == खातापूर्ण) {
			syntax_error_unterm_ch('\'');
			वापस 0;
		}
		अगर (ch == '\'')
			वापस 1;
		o_addchr(dest, ch);
	}
}
अटल पूर्णांक add_till_single_quote_dquoted(o_string *dest, काष्ठा in_str *निवेश)
{
	जबतक (1) {
		पूर्णांक ch = i_getch(निवेश);
		अगर (ch == खातापूर्ण) {
			syntax_error_unterm_ch('\'');
			वापस 0;
		}
		अगर (ch == '\'')
			वापस 1;
		o_addqchr(dest, ch);
	}
}

/* "...\"...`..`...." - do we need to handle "...$(..)..." too? */
अटल पूर्णांक add_till_backquote(o_string *dest, काष्ठा in_str *निवेश, पूर्णांक in_dquote);
अटल पूर्णांक add_till_double_quote(o_string *dest, काष्ठा in_str *निवेश)
{
	जबतक (1) {
		पूर्णांक ch = i_getch(निवेश);
		अगर (ch == खातापूर्ण) {
			syntax_error_unterm_ch('"');
			वापस 0;
		}
		अगर (ch == '"')
			वापस 1;
		अगर (ch == '\\') {  /* \x. Copy both chars. */
			o_addchr(dest, ch);
			ch = i_getch(निवेश);
		}
		o_addchr(dest, ch);
		अगर (ch == '`') {
			अगर (!add_till_backquote(dest, निवेश, /*in_dquote:*/ 1))
				वापस 0;
			o_addchr(dest, ch);
			जारी;
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
अटल पूर्णांक add_till_backquote(o_string *dest, काष्ठा in_str *निवेश, पूर्णांक in_dquote)
{
	जबतक (1) {
		पूर्णांक ch = i_getch(निवेश);
		अगर (ch == '`')
			वापस 1;
		अगर (ch == '\\') {
			/* \x. Copy both unless it is \`, \$, \\ and maybe \" */
			ch = i_getch(निवेश);
			अगर (ch != '`'
			 && ch != '$'
			 && ch != '\\'
			 && (!in_dquote || ch != '"')
			) {
				o_addchr(dest, '\\');
			}
		}
		अगर (ch == खातापूर्ण) {
			syntax_error_unterm_ch('`');
			वापस 0;
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
#घोषणा DOUBLE_CLOSE_CHAR_FLAG 0x80
अटल पूर्णांक add_till_closing_bracket(o_string *dest, काष्ठा in_str *निवेश, अचिन्हित end_ch)
{
	पूर्णांक ch;
	अक्षर dbl = end_ch & DOUBLE_CLOSE_CHAR_FLAG;
# अगर BASH_SUBSTR || BASH_PATTERN_SUBST
	अक्षर end_char2 = end_ch >> 8;
# endif
	end_ch &= (DOUBLE_CLOSE_CHAR_FLAG - 1);

# अगर ENABLE_HUSH_INTERACTIVE
	G.promptmode = 1; /* PS2 */
# endif
	debug_printf_prompt("%s promptmode=%d\n", __func__, G.promptmode);

	जबतक (1) {
		ch = i_getch(निवेश);
		अगर (ch == खातापूर्ण) {
			syntax_error_unterm_ch(end_ch);
			वापस 0;
		}
		अगर (ch == end_ch
# अगर BASH_SUBSTR || BASH_PATTERN_SUBST
		 || ch == end_char2
# endif
		) {
			अगर (!dbl)
				अवरोध;
			/* we look for closing )) of $((EXPR)) */
			अगर (i_peek_and_eat_bkslash_nl(निवेश) == end_ch) {
				i_getch(निवेश); /* eat second ')' */
				अवरोध;
			}
		}
		o_addchr(dest, ch);
		//bb_error_msg("%s:o_addchr('%c')", __func__, ch);
		अगर (ch == '(' || ch == '{') {
			ch = (ch == '(' ? ')' : '}');
			अगर (!add_till_closing_bracket(dest, निवेश, ch))
				वापस 0;
			o_addchr(dest, ch);
			जारी;
		}
		अगर (ch == '\'') {
			अगर (!add_till_single_quote(dest, निवेश))
				वापस 0;
			o_addchr(dest, ch);
			जारी;
		}
		अगर (ch == '"') {
			अगर (!add_till_double_quote(dest, निवेश))
				वापस 0;
			o_addchr(dest, ch);
			जारी;
		}
		अगर (ch == '`') {
			अगर (!add_till_backquote(dest, निवेश, /*in_dquote:*/ 0))
				वापस 0;
			o_addchr(dest, ch);
			जारी;
		}
		अगर (ch == '\\') {
			/* \x. Copy verbatim. Important for  \(, \) */
			ch = i_getch(निवेश);
			अगर (ch == खातापूर्ण) {
				syntax_error_unterm_ch(end_ch);
				वापस 0;
			}
# अगर 0
			अगर (ch == '\n') {
				/* "backslash+newline", ignore both */
				o_delchr(dest); /* undo insertion of '\' */
				जारी;
			}
# endif
			o_addchr(dest, ch);
			//bb_error_msg("%s:o_addchr('%c') after '\\'", __func__, ch);
			जारी;
		}
	}
	debug_printf_parse("%s return '%s' ch:'%c'\n", __func__, dest->तथ्य, ch);
	वापस ch;
}
#पूर्ण_अगर /* ENABLE_HUSH_TICK || ENABLE_FEATURE_SH_MATH || ENABLE_HUSH_DOLLAR_OPS */

#अगर BASH_DOLLAR_SQUOTE
/* Return code: 1 for "found and parsed", 0 for "seen something else" */
# अगर BB_MMU
#घोषणा parse_dollar_squote(as_string, dest, निवेश) \
	parse_dollar_squote(dest, निवेश)
#घोषणा as_string शून्य
# endif
अटल पूर्णांक parse_dollar_squote(o_string *as_string, o_string *dest, काष्ठा in_str *निवेश)
{
	पूर्णांक start;
	पूर्णांक ch = i_peek_and_eat_bkslash_nl(निवेश);  /* first character after the $ */
	debug_printf_parse("parse_dollar_squote entered: ch='%c'\n", ch);
	अगर (ch != '\'')
		वापस 0;

	dest->has_quoted_part = 1;
	start = dest->लंबाई;

	ch = i_getch(निवेश); /* eat ' */
	nommu_addchr(as_string, ch);
	जबतक (1) {
		ch = i_getch(निवेश);
		nommu_addchr(as_string, ch);
		अगर (ch == खातापूर्ण) {
			syntax_error_unterm_ch('\'');
			वापस 0;
		}
		अगर (ch == '\'')
			अवरोध;
		अगर (ch == SPECIAL_VAR_प्रतीक) {
			/* Convert raw ^C to corresponding special variable reference */
			o_addchr(dest, SPECIAL_VAR_प्रतीक);
			o_addchr(dest, SPECIAL_VAR_QUOTED_SVS);
			/* will addchr() another SPECIAL_VAR_SYMBOL (see after the if() block) */
		} अन्यथा अगर (ch == '\\') {
			अटल स्थिर अक्षर C_escapes[] ALIGN1 = "nrbtfav""x\\01234567";

			ch = i_getch(निवेश);
			nommu_addchr(as_string, ch);
			अगर (म_अक्षर(C_escapes, ch)) {
				अक्षर buf[4];
				अक्षर *p = buf;
				पूर्णांक cnt = 2;

				buf[0] = ch;
				अगर ((अचिन्हित अक्षर)(ch - '0') <= 7) { /* \ooo */
					करो {
						ch = i_peek(निवेश);
						अगर ((अचिन्हित अक्षर)(ch - '0') > 7)
							अवरोध;
						*++p = ch = i_getch(निवेश);
						nommu_addchr(as_string, ch);
					} जबतक (--cnt != 0);
				} अन्यथा अगर (ch == 'x') { /* \xHH */
					करो {
						ch = i_peek(निवेश);
						अगर (!है_षष्ठादशक(ch))
							अवरोध;
						*++p = ch = i_getch(निवेश);
						nommu_addchr(as_string, ch);
					} जबतक (--cnt != 0);
					अगर (cnt == 2) { /* \x but next char is "bad" */
						ch = 'x';
						जाओ unrecognized;
					}
				} /* else simple seq like \\ or \t */
				*++p = '\0';
				p = buf;
				ch = bb_process_escape_sequence((व्योम*)&p);
				//bb_error_msg("buf:'%s' ch:%x", buf, ch);
				अगर (ch == '\0')
					जारी; /* bash compat: $'...\0...' emits nothing */
			} अन्यथा { /* unrecognized "\z": encode both chars unless ' or " */
				अगर (ch != '\'' && ch != '"') {
 unrecognized:
					o_addqchr(dest, '\\');
				}
			}
		} /* if (\...) */
		o_addqchr(dest, ch);
	}

	अगर (dest->लंबाई == start) {
		/* $'', $'\0', $'\000\x00' and the like */
		o_addchr(dest, SPECIAL_VAR_प्रतीक);
		o_addchr(dest, SPECIAL_VAR_प्रतीक);
	}

	वापस 1;
# undef as_string
}
#अन्यथा
# define parse_dollar_squote(as_string, dest, निवेश) 0
#पूर्ण_अगर /* BASH_DOLLAR_SQUOTE */
#पूर्ण_अगर /* !__U_BOOT__ */

/* Return code: 0 for OK, 1 for syntax error */
#अगर BB_MMU
#घोषणा parse_dollar(as_string, dest, निवेश, quote_mask) \
	parse_dollar(dest, निवेश, quote_mask)
#घोषणा as_string शून्य
#पूर्ण_अगर
अटल पूर्णांक parse_dollar(o_string *as_string,
		o_string *dest,
		काष्ठा in_str *निवेश, अचिन्हित अक्षर quote_mask)
{
	पूर्णांक ch = i_peek_and_eat_bkslash_nl(निवेश);  /* first character after the $ */

	debug_printf_parse("parse_dollar entered: ch='%c' quote_mask:0x%x\n", ch, quote_mask);
	अगर (है_अक्षर(ch)) {
 make_var:
		ch = i_getch(निवेश);
		nommu_addchr(as_string, ch);
 /*make_var1:*/
		o_addchr(dest, SPECIAL_VAR_प्रतीक);
		जबतक (1) {
			debug_printf_parse(": '%c'\n", ch);
			o_addchr(dest, ch | quote_mask);
			quote_mask = 0;
			ch = i_peek_and_eat_bkslash_nl(निवेश);
			अगर (!है_अक्षर_अंक(ch) && ch != '_') {
				/* End of variable name reached */
				अवरोध;
			}
			ch = i_getch(निवेश);
			nommu_addchr(as_string, ch);
		}
		o_addchr(dest, SPECIAL_VAR_प्रतीक);
	} अन्यथा अगर (है_अंक(ch)) {
 make_one_char_var:
		ch = i_getch(निवेश);
		nommu_addchr(as_string, ch);
		o_addchr(dest, SPECIAL_VAR_प्रतीक);
		debug_printf_parse(": '%c'\n", ch);
		o_addchr(dest, ch | quote_mask);
		o_addchr(dest, SPECIAL_VAR_प्रतीक);
	} अन्यथा चयन (ch) {
#अगर_अघोषित __U_BOOT__
	हाल '$': /* pid */
	हाल '!': /* last bg pid */
#पूर्ण_अगर /* !__U_BOOT__ */
	हाल '?': /* last exit code */
	हाल '#': /* number of args */
	हाल '*': /* args */
	हाल '@': /* args */
	हाल '-': /* $- option flags set by set builtin or shell options (-i etc) */
		जाओ make_one_char_var;
	हाल '{': {
		अक्षर len_single_ch;

		o_addchr(dest, SPECIAL_VAR_प्रतीक);

		ch = i_getch(निवेश); /* eat '{' */
		nommu_addchr(as_string, ch);

		ch = i_getch_and_eat_bkslash_nl(निवेश); /* first char after '{' */
		/* It should be ${?}, or ${#var},
		 * or even ${?+subst} - operator acting on a special variable,
		 * or the beginning of variable name.
		 */
		अगर (ch == खातापूर्ण
		 || (!म_अक्षर(_SPECIAL_VARS_STR, ch) && !है_अक्षर_अंक(ch)) /* not one of those */
		) {
 bad_dollar_syntax:
			syntax_error_unterm_str("${name}");
			debug_printf_parse("parse_dollar return 0: unterminated ${name}\n");
			वापस 0;
		}
		nommu_addchr(as_string, ch);
		len_single_ch = ch;
		ch |= quote_mask;

		/* It's possible to just call add_till_closing_bracket() at this point.
		 * However, this regresses some of our testsuite cases
		 * which check invalid constructs like ${%}.
		 * Oh well... let's check that the var name part is fine... */

		अगर (है_अंक(len_single_ch)
		 || (len_single_ch == '#' && है_अंक(i_peek_and_eat_bkslash_nl(निवेश)))
		) {
			/* Execution engine uses plain xatoi_positive()
			 * to interpret ${NNN} and {#NNN},
			 * check syntax here in the parser.
			 * (bash does not support expressions in ${#NN},
			 * e.g. ${#$var} and {#1:+WORD} are not supported).
			 */
			अचिन्हित cnt = 9; /* max 9 digits for ${NN} and 8 for {#NN} */
			जबतक (1) {
				o_addchr(dest, ch);
				debug_printf_parse(": '%c'\n", ch);
				ch = i_getch_and_eat_bkslash_nl(निवेश);
				nommu_addchr(as_string, ch);
				अगर (ch == '}')
					अवरोध;
				अगर (--cnt == 0)
					जाओ bad_dollar_syntax;
				अगर (len_single_ch != '#' && म_अक्षर(VAR_SUBST_OPS, ch))
					/* ${NN<op>...} is valid */
					जाओ eat_until_closing;
				अगर (!है_अंक(ch))
					जाओ bad_dollar_syntax;
			}
		} अन्यथा
		जबतक (1) {
			अचिन्हित pos;

			o_addchr(dest, ch);
			debug_printf_parse(": '%c'\n", ch);

			ch = i_getch(निवेश);
			nommu_addchr(as_string, ch);
			अगर (ch == '}')
				अवरोध;
#अगर_अघोषित __U_BOOT__
			अगर (!है_अक्षर_अंक(ch) && ch != '_') {
#अन्यथा /* __U_BOOT__ */
			/*
			 * In several places in U-Boot, we use variable like
			 * foo# (e.g. serial#), particularly in env.
			 * So, we need to authorize # to appear inside
			 * variable name and then expand this variable.
			 * NOTE Having # in variable name is not permitted in
			 * upstream hush but expansion will be done (even though
			 * the result will be empty).
			 */
			अगर (!है_अक्षर_अंक(ch) && ch != '_' && ch != '#') {
#पूर्ण_अगर /* __U_BOOT__ */
				अचिन्हित end_ch;
#अगर_अघोषित __U_BOOT__
				अचिन्हित अक्षर last_ch;
#पूर्ण_अगर /* !__U_BOOT__ */
				/* handle parameter expansions
				 * http://www.opengroup.org/onlinepubs/009695399/utilities/xcu_chap02.html#tag_02_06_02
				 */
				अगर (!म_अक्षर(VAR_SUBST_OPS, ch)) { /* ${var<bad_char>... */
					अगर (len_single_ch != '#'
					/*|| !strchr(SPECIAL_VARS_STR, ch) - disallow errors like ${#+} ? */
					 || i_peek(निवेश) != '}'
					) {
						जाओ bad_dollar_syntax;
					}
					/* else: it's "length of C" ${#C} op,
					 * where C is a single char
					 * special var name, e.g. ${#!}.
					 */
				}
 eat_until_closing:
				/* Eat everything until closing '}' (or ':') */
				end_ch = '}';
#अगर_अघोषित __U_BOOT__
				अगर (BASH_SUBSTR
				 && ch == ':'
				 && !म_अक्षर(MINUS_PLUS_EQUAL_QUESTION, i_peek(निवेश))
				) {
					/* It's ${var:N[:M]} thing */
					end_ch = '}' * 0x100 + ':';
				}
				अगर (BASH_PATTERN_SUBST
				 && ch == '/'
				) {
					/* It's ${var/[/]pattern[/repl]} thing */
					अगर (i_peek(निवेश) == '/') { /* ${var//pattern[/repl]}? */
						i_getch(निवेश);
						nommu_addchr(as_string, '/');
						ch = '\\';
					}
					end_ch = '}' * 0x100 + '/';
				}
#पूर्ण_अगर /* !__U_BOOT__ */
				o_addchr(dest, ch);
				/* The pattern can't be empty.
				 * IOW: if the first char after "${v//" is a slash,
				 * it does not terminate the pattern - it's the first char of the pattern:
				 *  v=/dev/ram; echo ${v////-}  prints -dev-ram (pattern is "/")
				 *  v=/dev/ram; echo ${v///r/-} prints /dev-am  (pattern is "/r")
				 */
				अगर (i_peek(निवेश) == '/') {
					o_addchr(dest, i_getch(निवेश));
				}
#अगर_अघोषित __U_BOOT__
 again:
#पूर्ण_अगर /* !__U_BOOT__ */
				अगर (!BB_MMU)
					pos = dest->लंबाई;
#अगर ENABLE_HUSH_DOLLAR_OPS
#अगर_अघोषित __U_BOOT__
				last_ch = add_till_closing_bracket(dest, निवेश, end_ch);
				अगर (last_ch == 0) /* error? */
					वापस 0;
#पूर्ण_अगर /* !__U_BOOT__ */
#अन्यथा
# error Simple code to only allow ${var} is not implemented
#पूर्ण_अगर
				अगर (as_string) {
					o_addstr(as_string, dest->तथ्य + pos);
#अगर_अघोषित __U_BOOT__
					o_addchr(as_string, last_ch);
#पूर्ण_अगर /* !__U_BOOT__ */
				}

#अगर_अघोषित __U_BOOT__
				अगर ((BASH_SUBSTR || BASH_PATTERN_SUBST)
					 && (end_ch & 0xff00)
				) {
					/* close the first block: */
					o_addchr(dest, SPECIAL_VAR_प्रतीक);
					/* while parsing N from ${var:N[:M]}
					 * or pattern from ${var/[/]pattern[/repl]} */
					अगर ((end_ch & 0xff) == last_ch) {
						/* got ':' or '/'- parse the rest */
						end_ch = '}';
						जाओ again;
					}
					/* got '}' */
					अगर (BASH_SUBSTR && end_ch == '}' * 0x100 + ':') {
						/* it's ${var:N} - emulate :999999999 */
						o_addstr(dest, "999999999");
					} /* else: it's ${var/[/]pattern} */
				}
#पूर्ण_अगर /* !__U_BOOT__ */
				अवरोध;
			}
			len_single_ch = 0; /* it can't be ${#C} op */
		}
		o_addchr(dest, SPECIAL_VAR_प्रतीक);
		अवरोध;
	}
#अगर ENABLE_FEATURE_SH_MATH || ENABLE_HUSH_TICK
	हाल '(': {
		अचिन्हित pos;

		ch = i_getch(निवेश);
		nommu_addchr(as_string, ch);
# अगर ENABLE_FEATURE_SH_MATH
		अगर (i_peek_and_eat_bkslash_nl(निवेश) == '(') {
			ch = i_getch(निवेश);
			nommu_addchr(as_string, ch);
			o_addchr(dest, SPECIAL_VAR_प्रतीक);
			o_addchr(dest, quote_mask | '+');
			अगर (!BB_MMU)
				pos = dest->लंबाई;
			अगर (!add_till_closing_bracket(dest, निवेश, ')' | DOUBLE_CLOSE_CHAR_FLAG))
				वापस 0; /* error */
			अगर (as_string) {
				o_addstr(as_string, dest->तथ्य + pos);
				o_addchr(as_string, ')');
				o_addchr(as_string, ')');
			}
			o_addchr(dest, SPECIAL_VAR_प्रतीक);
			अवरोध;
		}
# endif
# अगर ENABLE_HUSH_TICK
		o_addchr(dest, SPECIAL_VAR_प्रतीक);
		o_addchr(dest, quote_mask | '`');
		अगर (!BB_MMU)
			pos = dest->लंबाई;
		अगर (!add_till_closing_bracket(dest, निवेश, ')'))
			वापस 0; /* error */
		अगर (as_string) {
			o_addstr(as_string, dest->तथ्य + pos);
			o_addchr(as_string, ')');
		}
		o_addchr(dest, SPECIAL_VAR_प्रतीक);
# endif
		अवरोध;
	}
#पूर्ण_अगर
	हाल '_':
		जाओ make_var;
#अगर 0
	/* TODO: $_: */
	/* $_ Shell or shell script name; or last argument of last command
	 * (if last command wasn't a pipe; if it was, bash sets $_ to "");
	 * but in command's env, set to full pathname used to invoke it */
		ch = i_getch(निवेश);
		nommu_addchr(as_string, ch);
		ch = i_peek_and_eat_bkslash_nl(निवेश);
		अगर (है_अक्षर_अंक(ch)) { /* it's $_name or $_123 */
			ch = '_';
			जाओ make_var1;
		}
		/* else: it's $_ */
#पूर्ण_अगर
	शेष:
		o_addQchr(dest, '$');
	}
	debug_printf_parse("parse_dollar return 1 (ok)\n");
	वापस 1;
#अघोषित as_string
}

#अगर BB_MMU
#घोषणा encode_string(as_string, dest, निवेश, dquote_end) \
	encode_string(dest, निवेश, dquote_end)
#घोषणा as_string शून्य
#पूर्ण_अगर
अटल पूर्णांक encode_string(o_string *as_string,
		o_string *dest,
		काष्ठा in_str *निवेश,
		पूर्णांक dquote_end)
{
	पूर्णांक ch;
	पूर्णांक अग्र;

 again:
	ch = i_getch(निवेश);
	अगर (ch != खातापूर्ण)
		nommu_addchr(as_string, ch);
	अगर (ch == dquote_end) { /* may be only '"' or EOF */
		debug_printf_parse("encode_string return 1 (ok)\n");
		वापस 1;
	}
	/* note: can't move it above ch == dquote_end check! */
	अगर (ch == खातापूर्ण) {
		syntax_error_unterm_ch('"');
		वापस 0; /* error */
	}
	अग्र = '\0';
	अगर (ch != '\n') {
		अग्र = i_peek(निवेश);
	}
	debug_printf_parse("\" ch=%c (%d) escape=%d\n",
			ch, ch, !!(dest->o_expflags & EXP_FLAG_ESC_GLOB_CHARS));
	अगर (ch == '\\') {
		अगर (अग्र == खातापूर्ण) {
			/* Testcase: in interactive shell a file with
			 *  echo "unterminated string\<eof>
			 * is sourced.
			 */
			syntax_error_unterm_ch('"');
			वापस 0; /* error */
		}
		/* bash:
		 * "The backslash retains its special meaning [in "..."]
		 * only when followed by one of the following characters:
		 * $, `, ", \, or <newline>.  A double quote may be quoted
		 * within double quotes by preceding it with a backslash."
		 * NB: in (unquoted) heredoc, above does not apply to ",
		 * therefore we check for it by "next == dquote_end" cond.
		 */
		अगर (अग्र == dquote_end || म_अक्षर("$`\\\n", अग्र)) {
			ch = i_getch(निवेश); /* eat next */
			अगर (ch == '\n')
				जाओ again; /* skip \<newline> */
		} /* else: ch remains == '\\', and we double it below: */
		o_addqchr(dest, ch); /* \c if c is a glob char, else just c */
		nommu_addchr(as_string, ch);
		जाओ again;
	}
	अगर (ch == '$') {
		//if (parse_dollar_squote(as_string, dest, input))
		//	goto again;
		अगर (!parse_dollar(as_string, dest, निवेश, /*quote_mask:*/ 0x80)) {
			debug_printf_parse("encode_string return 0: "
					"parse_dollar returned 0 (error)\n");
			वापस 0;
		}
		जाओ again;
	}
#अगर ENABLE_HUSH_TICK
	अगर (ch == '`') {
		//unsigned pos = dest->length;
		o_addchr(dest, SPECIAL_VAR_प्रतीक);
		o_addchr(dest, 0x80 | '`');
		अगर (!add_till_backquote(dest, निवेश, /*in_dquote:*/ dquote_end == '"'))
			वापस 0; /* error */
		o_addchr(dest, SPECIAL_VAR_प्रतीक);
		//debug_printf_subst("SUBST RES3 '%s'\n", dest->data + pos);
		जाओ again;
	}
#पूर्ण_अगर
	o_addQchr(dest, ch);
	अगर (ch == SPECIAL_VAR_प्रतीक) {
		/* Convert "^C" to corresponding special variable reference */
		o_addchr(dest, SPECIAL_VAR_QUOTED_SVS);
		o_addchr(dest, SPECIAL_VAR_प्रतीक);
	}
	जाओ again;
#अघोषित as_string
}

/*
 * Scan input until EOF or end_trigger char.
 * Return a list of pipes to execute, or NULL on EOF
 * or if end_trigger character is met.
 * On syntax error, exit if shell is not interactive,
 * reset parsing machinery and start parsing anew,
 * or return ERR_PTR.
 */
अटल काष्ठा pipe *parse_stream(अक्षर **pstring,
		पूर्णांक *heredoc_cnt_ptr,
		काष्ठा in_str *निवेश,
		पूर्णांक end_trigger)
{
	काष्ठा parse_context ctx;
	पूर्णांक heredoc_cnt;

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
	ctx.word.तथ्य = xzalloc(1); /* start as "", not as NULL */

	/* We used to separate words on $IFS here. This was wrong.
	 * $IFS is used only for word splitting when $var is expanded,
	 * here we should use blank chars as separators, not $IFS
	 */

	heredoc_cnt = 0;
	जबतक (1) {
		स्थिर अक्षर *is_blank;
		स्थिर अक्षर *is_special;
		पूर्णांक ch;
		पूर्णांक अग्र;
#अगर_अघोषित __U_BOOT__
		पूर्णांक redir_fd;
		redir_type redir_style;
#पूर्ण_अगर /* !__U_BOOT__ */

		ch = i_getch(निवेश);
		debug_printf_parse(": ch=%c (%d) escape=%d\n",
				ch, ch, !!(ctx.word.o_expflags & EXP_FLAG_ESC_GLOB_CHARS));
		अगर (ch == खातापूर्ण) {
			काष्ठा pipe *pi;

			अगर (heredoc_cnt) {
				syntax_error_unterm_str("here document");
				जाओ parse_error_exitcode1;
			}
			अगर (end_trigger == ')') {
				syntax_error_unterm_ch('(');
				जाओ parse_error_exitcode1;
			}
			अगर (end_trigger == '}') {
				syntax_error_unterm_ch('{');
				जाओ parse_error_exitcode1;
			}

			अगर (done_word(&ctx)) {
				जाओ parse_error_exitcode1;
			}
			o_free_and_set_NULL(&ctx.word);
			done_pipe(&ctx, PIPE_SEQ);

			/* Do we sit inside of any if's, loops or case's? */
			अगर (HAS_KEYWORDS
			IF_HAS_KEYWORDS(&& (ctx.ctx_res_w != RES_NONE || ctx.old_flag != 0))
			) {
				syntax_error_unterm_str("compound statement");
				जाओ parse_error_exitcode1;
			}

			pi = ctx.list_शीर्ष;
			/* If we got nothing... */
			/* (this makes bare "&" cmd a no-op.
			 * bash says: "syntax error near unexpected token '&'") */
			अगर (pi->अंक_cmds == 0
			IF_HAS_KEYWORDS(&& pi->res_word == RES_NONE)
			) {
				free_pipe_list(pi);
				pi = शून्य;
			}
#अगर !BB_MMU
			debug_printf_parse("as_string1 '%s'\n", ctx.as_string.तथ्य);
			अगर (pstring)
				*pstring = ctx.as_string.तथ्य;
			अन्यथा
				o_free(&ctx.as_string);
#पूर्ण_अगर
			// heredoc_cnt must be 0 here anyway
			//if (heredoc_cnt_ptr)
			//	*heredoc_cnt_ptr = heredoc_cnt;
			debug_leave();
			debug_printf_heredoc("parse_stream return heredoc_cnt:%d\n", heredoc_cnt);
			debug_printf_parse("parse_stream return %p: EOF\n", pi);
			वापस pi;
		}

		/* Handle "'" and "\" first, as they won't play nice with
		 * i_peek_and_eat_bkslash_nl() anyway:
		 *   echo z\\
		 * and
		 *   echo '\
		 *   '
		 * would break.
		 */
		अगर (ch == '\\') {
			ch = i_getch(निवेश);
			अगर (ch == '\n')
				जारी; /* drop \<newline>, get next char */
			nommu_addchr(&ctx.as_string, '\\');
			अगर (ch == SPECIAL_VAR_प्रतीक) {
				nommu_addchr(&ctx.as_string, ch);
				/* Convert \^C to corresponding special variable reference */
				जाओ case_SPECIAL_VAR_प्रतीक;
			}
			o_addchr(&ctx.word, '\\');
			अगर (ch == खातापूर्ण) {
				/* Testcase: eval 'echo Ok\' */
				/* bash-4.3.43 was removing backslash,
				 * but 4.4.19 retains it, most other shells too
				 */
				जारी; /* get next char */
			}
			/* Example: echo Hello \2>file
			 * we need to know that word 2 is quoted
			 */
			ctx.word.has_quoted_part = 1;
			nommu_addchr(&ctx.as_string, ch);
			o_addchr(&ctx.word, ch);
			जारी; /* get next char */
		}
		nommu_addchr(&ctx.as_string, ch);
		अगर (ch == '\'') {
			ctx.word.has_quoted_part = 1;
			अग्र = i_getch(निवेश);
#अगर_अघोषित __U_BOOT__
			अगर (अग्र == '\'' && !ctx.pending_redirect)
				जाओ जोड़ो_empty_quoted_str_marker;
#पूर्ण_अगर /* !__U_BOOT__ */

			ch = अग्र;
			जबतक (1) {
				अगर (ch == खातापूर्ण) {
					syntax_error_unterm_ch('\'');
					जाओ parse_error_exitcode1;
				}
				nommu_addchr(&ctx.as_string, ch);
				अगर (ch == '\'')
					अवरोध;
				अगर (ch == SPECIAL_VAR_प्रतीक) {
					/* Convert raw ^C to corresponding special variable reference */
					o_addchr(&ctx.word, SPECIAL_VAR_प्रतीक);
					o_addchr(&ctx.word, SPECIAL_VAR_QUOTED_SVS);
				}
				o_addqchr(&ctx.word, ch);
				ch = i_getch(निवेश);
			}
			जारी; /* get next char */
		}

		अग्र = '\0';
		अगर (ch != '\n')
			अग्र = i_peek_and_eat_bkslash_nl(निवेश);

		is_special = "{}<>&|();#" /* special outside of "str" */
#अगर_अघोषित __U_BOOT__
				"$\"" IF_HUSH_TICK("`") /* always special */
#अन्यथा /* __U_BOOT__ */
				"$\""
#पूर्ण_अगर /* __U_BOOT__ */
				SPECIAL_VAR_प्रतीक_STR;
#अगर defined(CMD_TEST2_SINGLEWORD_NOGLOB)
		अगर (ctx.command->cmd_type == CMD_TEST2_SINGLEWORD_NOGLOB) {
			/* In [[ ]], {}<>&|() are not special */
			is_special += 8;
		} अन्यथा
#पूर्ण_अगर
		/* Are { and } special here? */
		अगर (ctx.command->argv /* word [word]{... - non-special */
		 || ctx.word.लंबाई       /* word{... - non-special */
		 || ctx.word.has_quoted_part     /* ""{... - non-special */
		 || (अग्र != ';'             /* }; - special */
		    && अग्र != ')'           /* }) - special */
		    && अग्र != '('           /* {( - special */
		    && अग्र != '&'           /* }& and }&& ... - special */
		    && अग्र != '|'           /* }|| ... - special */
		    && !म_अक्षर(defifs, अग्र) /* {word - non-special */
		    )
		) {
			/* They are not special, skip "{}" */
			is_special += 2;
		}
		is_special = म_अक्षर(is_special, ch);
		is_blank = म_अक्षर(defifs, ch);

		अगर (!is_special && !is_blank) { /* ordinary char */
 ordinary_char:
			o_addQchr(&ctx.word, ch);
			अगर ((ctx.is_assignment == MAYBE_ASSIGNMENT
			    || ctx.is_assignment == WORD_IS_KEYWORD)
			 && ch == '='
			 && endofname(ctx.word.तथ्य)[0] == '='
			) {
				ctx.is_assignment = DEFINITELY_ASSIGNMENT;
				debug_printf_parse("ctx.is_assignment='%s'\n", assignment_flag[ctx.is_assignment]);
			}
			जारी;
		}

		अगर (is_blank) {
#अगर ENABLE_HUSH_LINENO_VAR
/* Case:
 * "while ...; do<whitespace><newline>
 *	cmd ..."
 * would think that "cmd" starts in <whitespace> -
 * i.e., at the previous line.
 * We need to skip all whitespace before newlines.
 */
			जबतक (ch != '\n') {
				अग्र = i_peek(निवेश);
				अगर (अग्र != ' ' && अग्र != '\t' && अग्र != '\n')
					अवरोध; /* next char is not ws */
				ch = i_getch(निवेश);
			}
			/* ch == last eaten whitespace char */
#पूर्ण_अगर
			अगर (done_word(&ctx)) {
				जाओ parse_error_exitcode1;
			}
			अगर (ch == '\n') {
				/* Is this a case when newline is simply ignored?
				 * Some examples:
				 * "cmd | <newline> cmd ..."
				 * "case ... in <newline> word) ..."
				 */
				अगर (IS_NULL_CMD(ctx.command)
				 && ctx.word.लंबाई == 0
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
					काष्ठा pipe *pi = ctx.list_शीर्ष;
					अगर (pi->अंक_cmds != 0       /* check #1 */
					 && pi->followup != PIPE_BG /* check #2 */
					) {
						जारी;
					}
				}
				/* Treat newline as a command separator. */
				done_pipe(&ctx, PIPE_SEQ);
				debug_printf_heredoc("heredoc_cnt:%d\n", heredoc_cnt);
				अगर (heredoc_cnt) {
					heredoc_cnt = fetch_heredocs(&ctx.as_string, ctx.list_शीर्ष, heredoc_cnt, निवेश);
					अगर (heredoc_cnt != 0)
						जाओ parse_error_exitcode1;
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
#अगर_अघोषित __U_BOOT__
		अगर (ch == '}') {
#अन्यथा /* __U_BOOT__ */
		अगर (ch == '}' || ch == ')') {
#पूर्ण_अगर /* __U_BOOT__ */
			अगर (ctx.word.लंबाई != 0 /* word} */
			 || ctx.word.has_quoted_part    /* ""} */
			) {
				जाओ ordinary_char;
			}
			अगर (!IS_NULL_CMD(ctx.command)) { /* cmd } */
				/* Generally, there should be semicolon: "cmd; }"
				 * However, bash allows to omit it if "cmd" is
				 * a group. Examples:
				 * { { echo 1; } }
				 * {(echo 1)}
				 * { echo 0 >&2 | { echo 1; } }
				 * { while false; do :; done }
				 * { case a in b) ;; esac }
				 */
				अगर (ctx.command->group)
					जाओ term_group;
				जाओ ordinary_char;
			}
			अगर (!IS_NULL_PIPE(ctx.pipe)) /* cmd | } */
				/* Can't be an end of {cmd}, skip the check */
				जाओ skip_end_trigger;
			/* else: } does terminate a group */
		}
 term_group:
		अगर (end_trigger && end_trigger == ch
		 && (ch != ';' || heredoc_cnt == 0)
#अगर ENABLE_HUSH_CASE
		 && (ch != ')'
		    || ctx.ctx_res_w != RES_MATCH
		    || (!ctx.word.has_quoted_part && म_भेद(ctx.word.तथ्य, "esac") == 0)
		    )
#पूर्ण_अगर
		) {
			अगर (done_word(&ctx)) {
				जाओ parse_error_exitcode1;
			}
			done_pipe(&ctx, PIPE_SEQ);
			ctx.is_assignment = MAYBE_ASSIGNMENT;
			debug_printf_parse("ctx.is_assignment='%s'\n", assignment_flag[ctx.is_assignment]);
			/* Do we sit outside of any if's, loops or case's? */
			अगर (!HAS_KEYWORDS
			IF_HAS_KEYWORDS(|| (ctx.ctx_res_w == RES_NONE && ctx.old_flag == 0))
			) {
				o_free_and_set_NULL(&ctx.word);
#अगर !BB_MMU
				debug_printf_parse("as_string2 '%s'\n", ctx.as_string.तथ्य);
				अगर (pstring)
					*pstring = ctx.as_string.तथ्य;
				अन्यथा
					o_free(&ctx.as_string);
#पूर्ण_अगर
				अगर (ch != ';' && IS_NULL_PIPE(ctx.list_शीर्ष)) {
					/* Example: bare "{ }", "()" */
					G.last_exitcode = 2; /* bash compat */
					syntax_error_unexpected_ch(ch);
					जाओ parse_error;
				}
				अगर (heredoc_cnt_ptr)
					*heredoc_cnt_ptr = heredoc_cnt;
				debug_printf_heredoc("parse_stream return heredoc_cnt:%d\n", heredoc_cnt);
				debug_printf_parse("parse_stream return %p: "
						"end_trigger char found\n",
						ctx.list_शीर्ष);
				debug_leave();
				वापस ctx.list_शीर्ष;
			}
		}

		अगर (is_blank)
			जारी;

		/* Catch <, > before deciding whether this word is
		 * an assignment. a=1 2>z b=2: b=2 is still assignment */
		चयन (ch) {
#अगर_अघोषित __U_BOOT__
		हाल '>':
			redir_fd = redirect_opt_अंक(&ctx.word);
			अगर (done_word(&ctx)) {
				जाओ parse_error_exitcode1;
			}
			redir_style = REDIRECT_OVERWRITE;
			अगर (अग्र == '>') {
				redir_style = REDIRECT_APPEND;
				ch = i_getch(निवेश);
				nommu_addchr(&ctx.as_string, ch);
			}
#अगर 0
			अन्यथा अगर (अग्र == '(') {
				syntax_error(">(process) not supported");
				जाओ parse_error_exitcode1;
			}
#पूर्ण_अगर
			अगर (parse_redirect(&ctx, redir_fd, redir_style, निवेश))
				जाओ parse_error_exitcode1;
			जारी; /* get next char */
		हाल '<':
			redir_fd = redirect_opt_अंक(&ctx.word);
			अगर (done_word(&ctx)) {
				जाओ parse_error_exitcode1;
			}
			redir_style = REDIRECT_निवेश;
			अगर (अग्र == '<') {
				redir_style = REDIRECT_HEREDOC;
				heredoc_cnt++;
				debug_printf_heredoc("++heredoc_cnt=%d\n", heredoc_cnt);
				ch = i_getch(निवेश);
				nommu_addchr(&ctx.as_string, ch);
			} अन्यथा अगर (अग्र == '>') {
				redir_style = REDIRECT_IO;
				ch = i_getch(निवेश);
				nommu_addchr(&ctx.as_string, ch);
			}
#अगर 0
			अन्यथा अगर (अग्र == '(') {
				syntax_error("<(process) not supported");
				जाओ parse_error_exitcode1;
			}
#पूर्ण_अगर
			अगर (parse_redirect(&ctx, redir_fd, redir_style, निवेश))
				जाओ parse_error_exitcode1;
			जारी; /* get next char */
#अन्यथा /* __U_BOOT__ */
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
			हाल '>':
				fallthrough;
			हाल '<':
				o_addQchr(&ctx.word, ch);
				जारी;
#पूर्ण_अगर /* __U_BOOT__ */
		हाल '#':
			अगर (ctx.word.लंबाई == 0 && !ctx.word.has_quoted_part) {
				/* skip "#comment" */
				/* note: we do not add it to &ctx.as_string */
/* TODO: in bash:
 * comment inside $() goes to the next \n, even inside quoted string (!):
 * cmd "$(cmd2 #comment)" - syntax error
 * cmd "`cmd2 #comment`" - ok
 * We accept both (comment ends where command subst ends, in both cases).
 */
				जबतक (1) {
					ch = i_peek(निवेश);
					अगर (ch == '\n') {
						nommu_addchr(&ctx.as_string, '\n');
						अवरोध;
					}
					ch = i_getch(निवेश);
					अगर (ch == खातापूर्ण)
						अवरोध;
				}
				जारी; /* get next char */
			}
			अवरोध;
		}
 skip_end_trigger:

		अगर (ctx.is_assignment == MAYBE_ASSIGNMENT
#अगर_अघोषित __U_BOOT__
		 /* check that we are not in word in "a=1 2>word b=1": */
		 && !ctx.pending_redirect
#पूर्ण_अगर /* !__U_BOOT__ */
		) {
			/* ch is a special char and thus this word
			 * cannot be an assignment */
			ctx.is_assignment = NOT_ASSIGNMENT;
			debug_printf_parse("ctx.is_assignment='%s'\n", assignment_flag[ctx.is_assignment]);
		}

		/* Note: nommu_addchr(&ctx.as_string, ch) is already done */

		चयन (ch) {
		case_SPECIAL_VAR_प्रतीक:
		हाल SPECIAL_VAR_प्रतीक:
			/* Convert raw ^C to corresponding special variable reference */
			o_addchr(&ctx.word, SPECIAL_VAR_प्रतीक);
			o_addchr(&ctx.word, SPECIAL_VAR_QUOTED_SVS);
			/* fall through */
		हाल '#':
			/* non-comment #: "echo a#b" etc */
			o_addchr(&ctx.word, ch);
			जारी; /* get next char */
		हाल '$':
#अगर_अघोषित __U_BOOT__
			अगर (parse_dollar_squote(&ctx.as_string, &ctx.word, निवेश))
				जारी; /* get next char */
#पूर्ण_अगर /* !__U_BOOT__ */
			अगर (!parse_dollar(&ctx.as_string, &ctx.word, निवेश, /*quote_mask:*/ 0)) {
				debug_printf_parse("parse_stream parse error: "
					"parse_dollar returned 0 (error)\n");
				जाओ parse_error_exitcode1;
			}
			जारी; /* get next char */
		हाल '"':
			ctx.word.has_quoted_part = 1;
#अगर_अघोषित __U_BOOT__
			अगर (अग्र == '"' && !ctx.pending_redirect) {
#अन्यथा /* __U_BOOT__ */
			अगर (अग्र == '"') {
#पूर्ण_अगर /* __U_BOOT__ */
				i_getch(निवेश); /* eat second " */
#अगर_अघोषित __U_BOOT__
 जोड़ो_empty_quoted_str_marker:
#पूर्ण_अगर /* !__U_BOOT__ */
				nommu_addchr(&ctx.as_string, अग्र);
				o_addchr(&ctx.word, SPECIAL_VAR_प्रतीक);
				o_addchr(&ctx.word, SPECIAL_VAR_प्रतीक);
				जारी; /* get next char */
			}
			अगर (ctx.is_assignment == NOT_ASSIGNMENT)
				ctx.word.o_expflags |= EXP_FLAG_ESC_GLOB_CHARS;
			अगर (!encode_string(&ctx.as_string, &ctx.word, निवेश, '"'))
				जाओ parse_error_exitcode1;
			ctx.word.o_expflags &= ~EXP_FLAG_ESC_GLOB_CHARS;
			जारी; /* get next char */
#अगर ENABLE_HUSH_TICK
		हाल '`': {
			USE_FOR_NOMMU(अचिन्हित pos;)

			o_addchr(&ctx.word, SPECIAL_VAR_प्रतीक);
			o_addchr(&ctx.word, '`');
			USE_FOR_NOMMU(pos = ctx.word.लंबाई;)
			अगर (!add_till_backquote(&ctx.word, निवेश, /*in_dquote:*/ 0))
				जाओ parse_error_exitcode1;
# अगर !BB_MMU
			o_addstr(&ctx.as_string, ctx.word.तथ्य + pos);
			o_addchr(&ctx.as_string, '`');
# endif
			o_addchr(&ctx.word, SPECIAL_VAR_प्रतीक);
			//debug_printf_subst("SUBST RES3 '%s'\n", ctx.word.data + pos);
			जारी; /* get next char */
		}
#पूर्ण_अगर
		हाल ';':
#अगर ENABLE_HUSH_CASE
 case_semi:
#पूर्ण_अगर
			अगर (done_word(&ctx)) {
				जाओ parse_error_exitcode1;
			}
			done_pipe(&ctx, PIPE_SEQ);
#अगर ENABLE_HUSH_CASE
			/* Eat multiple semicolons, detect
			 * whether it means something special */
			जबतक (1) {
				ch = i_peek_and_eat_bkslash_nl(निवेश);
				अगर (ch != ';')
					अवरोध;
				ch = i_getch(निवेश);
				nommu_addchr(&ctx.as_string, ch);
				अगर (ctx.ctx_res_w == RES_CASE_BODY) {
					ctx.ctx_dsemicolon = 1;
					ctx.ctx_res_w = RES_MATCH;
					अवरोध;
				}
			}
#पूर्ण_अगर
 new_cmd:
			/* We just finished a cmd. New one may start
			 * with an assignment */
			ctx.is_assignment = MAYBE_ASSIGNMENT;
			debug_printf_parse("ctx.is_assignment='%s'\n", assignment_flag[ctx.is_assignment]);
			जारी; /* get next char */
		हाल '&':
			अगर (done_word(&ctx)) {
				जाओ parse_error_exitcode1;
			}
			अगर (अग्र == '&') {
				ch = i_getch(निवेश);
				nommu_addchr(&ctx.as_string, ch);
				done_pipe(&ctx, PIPE_AND);
			} अन्यथा {
				done_pipe(&ctx, PIPE_BG);
			}
			जाओ new_cmd;
		हाल '|':
			अगर (done_word(&ctx)) {
				जाओ parse_error_exitcode1;
			}
#अगर ENABLE_HUSH_CASE
			अगर (ctx.ctx_res_w == RES_MATCH)
				अवरोध; /* we are in case's "word | word)" */
#पूर्ण_अगर
			अगर (अग्र == '|') { /* || */
				ch = i_getch(निवेश);
				nommu_addchr(&ctx.as_string, ch);
				done_pipe(&ctx, PIPE_OR);
			} अन्यथा {
				/* we could pick up a file descriptor choice here
				 * with redirect_opt_num(), but bash doesn't do it.
				 * "echo foo 2| cat" yields "foo 2". */
				done_command(&ctx);
			}
			जाओ new_cmd;
		हाल '(':
#अगर ENABLE_HUSH_CASE
			/* "case... in [(]word)..." - skip '(' */
			अगर (ctx.ctx_res_w == RES_MATCH
			 && ctx.command->argv == शून्य /* not (word|(... */
			 && ctx.word.लंबाई == 0 /* not word(... */
			 && ctx.word.has_quoted_part == 0 /* not ""(... */
			) {
				जारी; /* get next char */
			}
#पूर्ण_अगर
			/* fall through */
		हाल '{': {
			पूर्णांक n = parse_group(&ctx, निवेश, ch);
			अगर (n < 0) {
				जाओ parse_error_exitcode1;
			}
			debug_printf_heredoc("parse_group done, needs heredocs:%d\n", n);
			heredoc_cnt += n;
			जाओ new_cmd;
		}
		हाल ')':
#अगर ENABLE_HUSH_CASE
			अगर (ctx.ctx_res_w == RES_MATCH)
				जाओ case_semi;
#पूर्ण_अगर
		हाल '}':
			/* proper use of this character is caught by end_trigger:
			 * if we see {, we call parse_group(..., end_trigger='}')
			 * and it will match } earlier (not here). */
			G.last_exitcode = 2;
			syntax_error_unexpected_ch(ch);
			जाओ parse_error;
		शेष:
			अगर (HUSH_DEBUG)
				bb_error_संदेश_and_die("BUG: unexpected %c", ch);
		}
	} /* while (1) */

 parse_error_exitcode1:
	G.last_exitcode = 1;
 parse_error:
	{
		काष्ठा parse_context *pctx;
		IF_HAS_KEYWORDS(काष्ठा parse_context *p2;)

		/* Clean up allocated tree.
		 * Sample for finding leaks on syntax error recovery path.
		 * Run it from interactive shell, watch pmap `pidof hush`.
		 * while if false; then false; fi; do break; fi
		 * Samples to catch leaks at execution:
		 * while if (true | { true;}); then echo ok; fi; do break; done
		 * while if (true | { true;}); then echo ok; fi; do (if echo ok; break; then :; fi) | cat; break; done
		 */
		pctx = &ctx;
		करो {
			/* Update pipe/command counts,
			 * otherwise freeing may miss some */
			done_pipe(pctx, PIPE_SEQ);
			debug_printf_clean("freeing list %p from ctx %p\n",
					pctx->list_शीर्ष, pctx);
			debug_print_वृक्ष(pctx->list_शीर्ष, 0);
			free_pipe_list(pctx->list_शीर्ष);
			debug_printf_clean("freed list %p\n", pctx->list_शीर्ष);
#अगर !BB_MMU
			o_free(&pctx->as_string);
#पूर्ण_अगर
			IF_HAS_KEYWORDS(p2 = pctx->ढेर;)
			अगर (pctx != &ctx) {
				मुक्त(pctx);
			}
			IF_HAS_KEYWORDS(pctx = p2;)
		} जबतक (HAS_KEYWORDS && pctx);

		o_free(&ctx.word);
#अगर !BB_MMU
		अगर (pstring)
			*pstring = शून्य;
#पूर्ण_अगर
		debug_leave();
		वापस ERR_PTR;
	}
}

/*** Execution routines ***/

/* Expansion can recurse, need forward decls: */
#अगर !BASH_PATTERN_SUBST && !ENABLE_HUSH_CASE
#घोषणा expand_string_to_string(str, EXP_flags, do_unbackslash) \
	expand_string_to_string(str)
#पूर्ण_अगर
अटल अक्षर *expand_string_to_string(स्थिर अक्षर *str, पूर्णांक EXP_flags, पूर्णांक do_unbackslash);
#अगर ENABLE_HUSH_TICK
अटल पूर्णांक process_command_subs(o_string *dest, स्थिर अक्षर *s);
#पूर्ण_अगर
अटल पूर्णांक expand_vars_to_list(o_string *output, पूर्णांक n, अक्षर *arg);

/* expand_strvec_to_strvec() takes a list of strings, expands
 * all variable references within and returns a pointer to
 * a list of expanded strings, possibly with larger number
 * of strings. (Think VAR="a b"; echo $VAR).
 * This new list is allocated as a single malloc block.
 * NULL-terminated list of char* pointers is at the beginning of it,
 * followed by strings themselves.
 * Caller can deallocate entire list by single free(list). */

/* A horde of its helpers come first: */

अटल व्योम o_addblock_duplicate_backslash(o_string *o, स्थिर अक्षर *str, पूर्णांक len)
{
	जबतक (--len >= 0) {
		अक्षर c = *str++;

#अगर ENABLE_HUSH_BRACE_EXPANSION
		अगर (c == '{' || c == '}') {
			/* { -> \{, } -> \} */
			o_addchr(o, '\\');
			/* And now we want to add { or } and continue:
			 *  o_addchr(o, c);
			 *  continue;
			 * luckily, just falling through achieves this.
			 */
		}
#पूर्ण_अगर
		o_addchr(o, c);
		अगर (c == '\\') {
			/* \z -> \\\z; \<eol> -> \\<eol> */
			o_addchr(o, '\\');
			अगर (len) {
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
अटल पूर्णांक expand_on_ifs(o_string *output, पूर्णांक n, स्थिर अक्षर *str)
{
	पूर्णांक last_is_ifs = 0;

	जबतक (1) {
		पूर्णांक word_len;

		अगर (!*str)  /* EOL - do not finalize word */
			अवरोध;
		word_len = म_खोज(str, G.ifs);
		अगर (word_len) {
			/* We have WORD_LEN leading non-IFS chars */
			अगर (!(output->o_expflags & EXP_FLAG_GLOB)) {
				o_addblock(output, str, word_len);
			} अन्यथा {
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
			अगर (!*str)  /* EOL - do not finalize word */
				अवरोध;
		}

		/* We know str here points to at least one IFS char */
		last_is_ifs = 1;
		str += म_अखोज(str, G.ifs_whitespace); /* skip IFS whitespace chars */
		अगर (!*str)  /* EOL - do not finalize word */
			अवरोध;

		अगर (G.ifs_whitespace != G.ifs /* usually false ($IFS is usually all whitespace), */
		 && म_अक्षर(G.ifs, *str)       /* the second check would fail */
		) {
			/* This is a non-whitespace $IFS char */
			/* Skip it and IFS whitespace chars, start new word */
			str++;
			str += म_अखोज(str, G.ifs_whitespace);
			जाओ new_word;
		}

		/* Start new word... but not always! */
		/* Case "v=' a'; echo ''$v": we do need to finalize empty word: */
		अगर (output->has_quoted_part
		/*
		 * Case "v=' a'; echo $v":
		 * here nothing precedes the space in $v expansion,
		 * therefore we should not finish the word
		 * (IOW: if there *is* word to finalize, only then do it):
		 * It's okay if this accesses the byte before first argv[]:
		 * past call to o_save_ptr() cleared it to zero byte
		 * (grep for -prev-ifs-check-).
		 */
		 || output->तथ्य[output->लंबाई - 1]
		) {
 new_word:
			o_addchr(output, '\0');
			debug_print_list("expand_on_ifs", output, n);
			n = o_save_ptr(output, n);
		}
	}

	output->ended_in_ifs = last_is_ifs;
	debug_print_list("expand_on_ifs[1]", output, n);
	वापस n;
}

#अगर_अघोषित __U_BOOT__
/* Helper to expand $((...)) and heredoc body. These act as if
 * they are in double quotes, with the exception that they are not :).
 * Just the rules are similar: "expand only $var and `cmd`"
 *
 * Returns malloced string.
 * As an optimization, we return NULL if expansion is not needed.
 */
अटल अक्षर *encode_then_expand_string(स्थिर अक्षर *str)
{
	अक्षर *exp_str;
	काष्ठा in_str निवेश;
	o_string dest = NULL_O_STRING;
	स्थिर अक्षर *cp;

	cp = str;
	क्रम (;;) {
		अगर (!*cp) वापस शून्य; /* string has no special chars */
		अगर (*cp == '$') अवरोध;
		अगर (*cp == '\\') अवरोध;
#अगर ENABLE_HUSH_TICK
		अगर (*cp == '`') अवरोध;
#पूर्ण_अगर
		cp++;
	}

	/* We need to expand. Example:
	 * echo $(($a + `echo 1`)) $((1 + $((2)) ))
	 */
	setup_string_in_str(&निवेश, str);
	encode_string(शून्य, &dest, &निवेश, खातापूर्ण);
//TODO: error check (encode_string returns 0 on error)?
	//bb_error_msg("'%s' -> '%s'", str, dest.data);
	exp_str = expand_string_to_string(dest.तथ्य,
			EXP_FLAG_ESC_GLOB_CHARS,
			/*unbackslash:*/ 1
	);
	//bb_error_msg("'%s' -> '%s'", dest.data, exp_str);
	o_free(&dest);
	वापस exp_str;
}

अटल स्थिर अक्षर *first_special_char_in_vararg(स्थिर अक्षर *cp)
{
	क्रम (;;) {
		अगर (!*cp) वापस शून्य; /* string has no special chars */
		अगर (*cp == '$') वापस cp;
		अगर (*cp == '\\') वापस cp;
		अगर (*cp == '\'') वापस cp;
		अगर (*cp == '"') वापस cp;
#अगर ENABLE_HUSH_TICK
		अगर (*cp == '`') वापस cp;
#पूर्ण_अगर
		/* dquoted "${x:+ARG}" should not glob, therefore
		 * '*' et al require some non-literal processing: */
		अगर (*cp == '*') वापस cp;
		अगर (*cp == '?') वापस cp;
		अगर (*cp == '[') वापस cp;
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
#अगर !BASH_PATTERN_SUBST
#घोषणा encode_then_expand_vararg(str, handle_squotes, do_unbackslash) \
	encode_then_expand_vararg(str, handle_squotes)
#पूर्ण_अगर
अटल अक्षर *encode_then_expand_vararg(स्थिर अक्षर *str, पूर्णांक handle_squotes, पूर्णांक do_unbackslash)
{
#अगर !BASH_PATTERN_SUBST && ENABLE_HUSH_CASE
	स्थिर पूर्णांक do_unbackslash = 0;
#पूर्ण_अगर
	अक्षर *exp_str;
	काष्ठा in_str निवेश;
	o_string dest = NULL_O_STRING;

	अगर (!first_special_char_in_vararg(str)) {
		/* string has no special chars */
		वापस शून्य;
	}

	setup_string_in_str(&निवेश, str);
	dest.तथ्य = xzalloc(1); /* start as "", not as NULL */
	exp_str = शून्य;

	क्रम (;;) {
		पूर्णांक ch;

		ch = i_getch(&निवेश);
		debug_printf_parse("%s: ch=%c (%d) escape=%d\n",
				__func__, ch, ch, !!dest.o_expflags);

		अगर (!dest.o_expflags) {
			अगर (ch == खातापूर्ण)
				अवरोध;
			अगर (handle_squotes && ch == '\'') {
				अगर (!add_till_single_quote_dquoted(&dest, &निवेश))
					जाओ ret; /* error */
				जारी;
			}
		}
		अगर (ch == खातापूर्ण) {
			syntax_error_unterm_ch('"');
			जाओ ret; /* error */
		}
		अगर (ch == '"') {
			dest.o_expflags ^= EXP_FLAG_ESC_GLOB_CHARS;
			जारी;
		}
		अगर (ch == '\\') {
			ch = i_getch(&निवेश);
			अगर (ch == खातापूर्ण) {
//example? error message?	syntax_error_unterm_ch('"');
				debug_printf_parse("%s: error: \\<eof>\n", __func__);
				जाओ ret;
			}
			o_addqchr(&dest, ch);
			जारी;
		}
		अगर (ch == '$') {
			अगर (parse_dollar_squote(शून्य, &dest, &निवेश))
				जारी;
			अगर (!parse_dollar(शून्य, &dest, &निवेश, /*quote_mask:*/ 0x80)) {
				debug_printf_parse("%s: error: parse_dollar returned 0 (error)\n", __func__);
				जाओ ret;
			}
			जारी;
		}
#अगर ENABLE_HUSH_TICK
		अगर (ch == '`') {
			//unsigned pos = dest->length;
			o_addchr(&dest, SPECIAL_VAR_प्रतीक);
			o_addchr(&dest, 0x80 | '`');
			अगर (!add_till_backquote(&dest, &निवेश,
					/*in_dquote:*/ dest.o_expflags /* nonzero if EXP_FLAG_ESC_GLOB_CHARS set */
				)
			) {
				जाओ ret; /* error */
			}
			o_addchr(&dest, SPECIAL_VAR_प्रतीक);
			//debug_printf_subst("SUBST RES3 '%s'\n", dest->data + pos);
			जारी;
		}
#पूर्ण_अगर
		o_addQchr(&dest, ch);
	} /* for (;;) */

	debug_printf_parse("encode: '%s' -> '%s'\n", str, dest.तथ्य);
	exp_str = expand_string_to_string(dest.तथ्य,
			do_unbackslash ? EXP_FLAG_ESC_GLOB_CHARS : 0,
			do_unbackslash
	);
 ret:
	debug_printf_parse("expand: '%s' -> '%s'\n", dest.तथ्य, exp_str);
	o_free(&dest);
	वापस exp_str;
}

/* Expanding ARG in ${var+ARG}, ${var-ARG}
 */
अटल NOINLINE पूर्णांक encode_then_append_var_plusminus(o_string *output, पूर्णांक n,
		अक्षर *str, पूर्णांक dquoted)
{
	काष्ठा in_str निवेश;
	o_string dest = NULL_O_STRING;

	अगर (!first_special_char_in_vararg(str)
	 && '\0' == str[म_खोज(str, G.ifs)]
	) {
		/* string has no special chars
		 * && string has no $IFS chars
		 */
		अगर (dquoted) {
			/* Prints 1 (quoted expansion is a "" word, not nothing):
			 * set -- "${notexist-}"; echo $#
			 */
			output->has_quoted_part = 1;
		}
		वापस expand_vars_to_list(output, n, str);
	}

	setup_string_in_str(&निवेश, str);

	क्रम (;;) {
		पूर्णांक ch;

		ch = i_getch(&निवेश);
		debug_printf_parse("%s: ch=%c (%d) escape=%x\n",
				__func__, ch, ch, dest.o_expflags);

		अगर (!dest.o_expflags) {
			अगर (ch == खातापूर्ण)
				अवरोध;
			अगर (!dquoted && !(output->o_expflags & EXP_FLAG_SINGLEWORD) && म_अक्षर(G.ifs, ch)) {
				/* PREFIX${x:d${e}f ...} and we met space: expand "d${e}f" and start new word.
				 * do not assume we are at the start of the word (PREFIX above).
				 */
				अगर (dest.तथ्य) {
					n = expand_vars_to_list(output, n, dest.तथ्य);
					o_free_and_set_NULL(&dest);
					o_addchr(output, '\0');
					n = o_save_ptr(output, n); /* create next word */
				} अन्यथा
				अगर (output->लंबाई != o_get_last_ptr(output, n)
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
				जारी;
			}
			अगर (!dquoted && ch == '\'') {
				अगर (!add_till_single_quote_dquoted(&dest, &निवेश))
					जाओ ret; /* error */
				o_addchr(&dest, SPECIAL_VAR_प्रतीक);
				o_addchr(&dest, SPECIAL_VAR_प्रतीक);
				जारी;
			}
		}
		अगर (ch == खातापूर्ण) {
			syntax_error_unterm_ch('"');
			जाओ ret; /* error */
		}
		अगर (ch == '"') {
			dest.o_expflags ^= EXP_FLAG_ESC_GLOB_CHARS;
			अगर (dest.o_expflags) {
				o_addchr(&dest, SPECIAL_VAR_प्रतीक);
				o_addchr(&dest, SPECIAL_VAR_प्रतीक);
			}
			जारी;
		}
		अगर (ch == '\\') {
			ch = i_getch(&निवेश);
			अगर (ch == खातापूर्ण) {
//example? error message?	syntax_error_unterm_ch('"');
				debug_printf_parse("%s: error: \\<eof>\n", __func__);
				जाओ ret;
			}
			o_addqchr(&dest, ch);
			जारी;
		}
		अगर (ch == '$') {
			अगर (!parse_dollar(शून्य, &dest, &निवेश, /*quote_mask:*/ (dest.o_expflags || dquoted) ? 0x80 : 0)) {
				debug_printf_parse("%s: error: parse_dollar returned 0 (error)\n", __func__);
				जाओ ret;
			}
			जारी;
		}
#अगर ENABLE_HUSH_TICK
		अगर (ch == '`') {
			//unsigned pos = dest->length;
			o_addchr(&dest, SPECIAL_VAR_प्रतीक);
			o_addchr(&dest, (dest.o_expflags || dquoted) ? 0x80 | '`' : '`');
			अगर (!add_till_backquote(&dest, &निवेश,
					/*in_dquote:*/ dest.o_expflags /* nonzero if EXP_FLAG_ESC_GLOB_CHARS set */
				)
			) {
				जाओ ret; /* error */
			}
			o_addchr(&dest, SPECIAL_VAR_प्रतीक);
			//debug_printf_subst("SUBST RES3 '%s'\n", dest->data + pos);
			जारी;
		}
#पूर्ण_अगर
		अगर (dquoted) {
			/* Always glob-protect if in dquotes:
			 * x=x; echo "${x:+/bin/c*}" - prints: /bin/c*
			 * x=x; echo "${x:+"/bin/c*"}" - prints: /bin/c*
			 */
			o_addqchr(&dest, ch);
		} अन्यथा {
			/* Glob-protect only if char is quoted:
			 * x=x; echo ${x:+/bin/c*} - prints many filenames
			 * x=x; echo ${x:+"/bin/c*"} - prints: /bin/c*
			 */
			o_addQchr(&dest, ch);
		}
	} /* for (;;) */

	अगर (dest.तथ्य) {
		n = expand_vars_to_list(output, n, dest.तथ्य);
	}
 ret:
	o_free(&dest);
	वापस n;
}
#पूर्ण_अगर /* !__U_BOOT__ */

#अगर_अघोषित __U_BOOT__
#अगर ENABLE_FEATURE_SH_MATH
अटल arith_t expand_and_evaluate_arith(स्थिर अक्षर *arg, स्थिर अक्षर **errmsg_p)
{
	arith_state_t math_state;
	arith_t res;
	अक्षर *exp_str;

	math_state.lookupvar = get_local_var_मूल्य;
	math_state.setvar = set_local_var_from_halves;
	//math_state.endofname = endofname;
	exp_str = encode_then_expand_string(arg);
	res = arith(&math_state, exp_str ? exp_str : arg);
	मुक्त(exp_str);
	अगर (errmsg_p)
		*errmsg_p = math_state.errmsg;
	अगर (math_state.errmsg)
		संदेश_and_die_if_script(math_state.errmsg);
	वापस res;
}
#पूर्ण_अगर
#पूर्ण_अगर /* !__U_BOOT__ */

#अगर_अघोषित __U_BOOT__
#अगर BASH_PATTERN_SUBST
/* ${var/[/]pattern[/repl]} helpers */
अटल अक्षर *strstr_pattern(अक्षर *val, स्थिर अक्षर *pattern, पूर्णांक *आकार)
{
	पूर्णांक first_escaped = (pattern[0] == '\\' && pattern[1]);
	/* "first_escaped" trick allows to treat e.g. "\*no_glob_chars"
	 * as literal too (as it is semi-common, and easy to accomodate
	 * by just using str + 1).
	 */
	पूर्णांक sz = म_खोज(pattern + first_escaped * 2, "*?[\\");
	अगर ((pattern + first_escaped * 2)[sz] == '\0') {
		/* Optimization for trivial patterns.
		 * Testcase for very slow replace (performs about 22k replaces):
		 * x=::::::::::::::::::::::
		 * x=$x$x;x=$x$x;x=$x$x;x=$x$x;x=$x$x;x=$x$x;x=$x$x;x=$x$x;x=$x$x;x=$x$x;echo ${#x}
		 * echo "${x//:/|}"
		 */
		*आकार = sz + first_escaped;
		वापस म_माला(val, pattern + first_escaped);
	}

	जबतक (1) {
		अक्षर *end = scan_and_match(val, pattern, SCAN_MOVE_FROM_दायाँ + SCAN_MATCH_बायाँ_HALF);
		debug_printf_varexp("val:'%s' pattern:'%s' end:'%s'\n", val, pattern, end);
		अगर (end) {
			*आकार = end - val;
			वापस val;
		}
		अगर (*val == '\0')
			वापस शून्य;
		/* Optimization: if "*pat" did not match the start of "string",
		 * we know that "tring", "ring" etc will not match too:
		 */
		अगर (pattern[0] == '*')
			वापस शून्य;
		val++;
	}
}
अटल अक्षर *replace_pattern(अक्षर *val, स्थिर अक्षर *pattern, स्थिर अक्षर *repl, अक्षर exp_op)
{
	अक्षर *परिणाम = शून्य;
	अचिन्हित res_len = 0;
	अचिन्हित repl_len = म_माप(repl);

	/* Null pattern never matches, including if "var" is empty */
	अगर (!pattern[0])
		वापस परिणाम; /* NULL, no replaces happened */

	जबतक (1) {
		पूर्णांक आकार;
		अक्षर *s = strstr_pattern(val, pattern, &आकार);
		अगर (!s)
			अवरोध;

		परिणाम = xrealloc(परिणाम, res_len + (s - val) + repl_len + 1);
		म_नकल(mempcpy(परिणाम + res_len, val, s - val), repl);
		res_len += (s - val) + repl_len;
		debug_printf_varexp("val:'%s' s:'%s' result:'%s'\n", val, s, परिणाम);

		val = s + आकार;
		अगर (exp_op == '/')
			अवरोध;
	}
	अगर (*val && परिणाम) {
		परिणाम = xrealloc(परिणाम, res_len + म_माप(val) + 1);
		म_नकल(परिणाम + res_len, val);
		debug_printf_varexp("val:'%s' result:'%s'\n", val, परिणाम);
	}
	debug_printf_varexp("result:'%s'\n", परिणाम);
	वापस परिणाम;
}
#पूर्ण_अगर /* BASH_PATTERN_SUBST */
#पूर्ण_अगर /* !__U_BOOT__ */

अटल पूर्णांक append_str_maybe_ifs_split(o_string *output, पूर्णांक n,
		पूर्णांक first_ch, स्थिर अक्षर *val)
{
	अगर (!(first_ch & 0x80)) { /* unquoted $VAR */
		debug_printf_expand("unquoted '%s', output->o_escape:%d\n", val,
				!!(output->o_expflags & EXP_FLAG_ESC_GLOB_CHARS));
		अगर (val && val[0])
			n = expand_on_ifs(output, n, val);
	} अन्यथा { /* quoted "$VAR" */
		output->has_quoted_part = 1;
		debug_printf_expand("quoted '%s', output->o_escape:%d\n", val,
				!!(output->o_expflags & EXP_FLAG_ESC_GLOB_CHARS));
		अगर (val && val[0])
			o_addQstr(output, val);
	}
	वापस n;
}

/* Handle <SPECIAL_VAR_SYMBOL>varname...<SPECIAL_VAR_SYMBOL> construct.
 */
अटल NOINLINE पूर्णांक expand_one_var(o_string *output, पूर्णांक n,
		पूर्णांक first_ch, अक्षर *arg, अक्षर **pp)
{
	स्थिर अक्षर *val;
	अक्षर *to_be_freed;
	अक्षर *p;
	अक्षर *var;
	अक्षर exp_op;
	अक्षर exp_save = exp_save; /* for compiler */
	अक्षर *exp_saveptr; /* points to expansion operator */
	अक्षर *exp_word = exp_word; /* for compiler */
	अक्षर arg0;

	val = शून्य;
	to_be_freed = शून्य;
	p = *pp;
	*p = '\0'; /* replace trailing SPECIAL_VAR_SYMBOL */
	var = arg;
	exp_saveptr = arg[1] ? म_अक्षर(VAR_ENCODED_SUBST_OPS, arg[1]) : शून्य;
	arg0 = arg[0];
	arg[0] = (arg0 & 0x7f);
	exp_op = 0;

	अगर (arg[0] == '#' && arg[1] /* ${#...} but not ${#} */
	 && (!exp_saveptr               /* and ( not(${#<op_char>...}) */
	    || (arg[2] == '\0' && म_अक्षर(SPECIAL_VARS_STR, arg[1])) /* or ${#C} "len of $C" ) */
	    )		/* NB: skipping ^^^specvar check mishandles ${#::2} */
	) {
		/* It must be length operator: ${#var} */
		var++;
		exp_op = 'L';
	} अन्यथा {
		/* Maybe handle parameter expansion */
		अगर (exp_saveptr /* if 2nd char is one of expansion operators */
		 && म_अक्षर(NUMERIC_SPECVARS_STR, arg[0]) /* 1st char is special variable */
		) {
			/* ${?:0}, ${#[:]%0} etc */
			exp_saveptr = var + 1;
		} अन्यथा {
			/* ${?}, ${var}, ${var:0}, ${var[:]%0} etc */
			exp_saveptr = var+1 + म_खोज(var+1, VAR_ENCODED_SUBST_OPS);
		}
		exp_op = exp_save = *exp_saveptr;
#अगर_अघोषित __U_BOOT__
		अगर (exp_op) {
			exp_word = exp_saveptr + 1;
			अगर (exp_op == ':') {
				exp_op = *exp_word++;
//TODO: try ${var:} and ${var:bogus} in non-bash config
				अगर (BASH_SUBSTR
				 && (!exp_op || !म_अक्षर(MINUS_PLUS_EQUAL_QUESTION, exp_op))
				) {
					/* oops... it's ${var:N[:M]}, not ${var:?xxx} or some such */
					exp_op = ':';
					exp_word--;
				}
			}
			*exp_saveptr = '\0';
		} /* else: it's not an expansion op, but bare ${var} */
#पूर्ण_अगर /* !__U_BOOT__ */
	}

	/* Look up the variable in question */
	अगर (है_अंक(var[0])) {
		/* parse_dollar should have vetted var for us */
#अगर_अघोषित __U_BOOT__
		पूर्णांक nn = xatoi_positive(var);
#अन्यथा /* __U_BOOT__ */
		पूर्णांक nn = simple_strtoul(var, शून्य, 10);
#पूर्ण_अगर /* __U_BOOT__ */
		अगर (nn < G.global_argc)
			val = G.global_argv[nn];
		/* else val remains NULL: $N with too big N */
	} अन्यथा {
		चयन (var[0]) {
#अगर_अघोषित __U_BOOT__
		हाल '$': /* pid */
			val = utoa(G.मूल_pid);
			अवरोध;
		हाल '!': /* bg pid */
			val = G.last_bg_pid ? utoa(G.last_bg_pid) : "";
			अवरोध;
#पूर्ण_अगर /* !__U_BOOT__ */
		हाल '?': /* exitcode */
			val = utoa(G.last_exitcode);
			अवरोध;
		हाल '#': /* argc */
			val = utoa(G.global_argc ? G.global_argc-1 : 0);
			अवरोध;
#अगर_अघोषित __U_BOOT__
		हाल '-': { /* active options */
			/* Check set_mode() to see what option chars we support */
			अक्षर *cp;
			val = cp = G.optstring_buf;
			अगर (G.o_opt[OPT_O_ERREXIT])
				*cp++ = 'e';
			अगर (G_interactive_fd)
				*cp++ = 'i';
			अगर (G_x_mode)
				*cp++ = 'x';
			/* If G.o_opt[OPT_O_NOEXEC] is true,
			 * commands read but are not executed,
			 * so $- can not execute too, 'n' is never seen in $-.
			 */
			अगर (G.opt_c)
				*cp++ = 'c';
			अगर (G.opt_s)
				*cp++ = 's';
			*cp = '\0';
			अवरोध;
		}
#पूर्ण_अगर /* !__U_BOOT__ */
		शेष:
#अगर_अघोषित __U_BOOT__
			val = get_local_var_मूल्य(var);
#अन्यथा /* __U_BOOT__ */
			/*
			 * Environment variable set with setenv* have to be
			 * expanded.
			 * So, we first search if the variable exists in
			 * environment, if this is not the case, we default to
			 * local value.
			 */
			val = env_get(var);
			अगर (!val)
				val = get_local_var_मूल्य(var);
#पूर्ण_अगर /* __U_BOOT__ */
		}
	}

#अगर_अघोषित __U_BOOT__
	/* Handle any expansions */
	अगर (exp_op == 'L') {
		reinit_unicode_for_hush();
		debug_printf_expand("expand: length(%s)=", val);
		val = utoa(val ? unicode_strlen(val) : 0);
		debug_printf_expand("%s\n", val);
	} अन्यथा अगर (exp_op) {
		अगर (exp_op == '%' || exp_op == '#') {
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
			अगर (val && val[0]) {
				अक्षर *t;
				अक्षर *exp_exp_word;
				अक्षर *loc;
				अचिन्हित scan_flags = pick_scan(exp_op, *exp_word);
				अगर (exp_op == *exp_word)  /* ## or %% */
					exp_word++;
				debug_printf_expand("expand: exp_word:'%s'\n", exp_word);
				exp_exp_word = encode_then_expand_vararg(exp_word, /*handle_squotes:*/ 1, /*unbackslash:*/ 0);
				अगर (exp_exp_word)
					exp_word = exp_exp_word;
				debug_printf_expand("expand: exp_word:'%s'\n", exp_word);
				/*
				 * HACK ALERT. We depend here on the fact that
				 * G.global_argv and results of utoa and get_local_var_value
				 * are actually in writable memory:
				 * scan_and_match momentarily stores NULs there.
				 */
				t = (अक्षर*)val;
				loc = scan_and_match(t, exp_word, scan_flags);
				debug_printf_expand("op:%c str:'%s' pat:'%s' res:'%s'\n", exp_op, t, exp_word, loc);
				मुक्त(exp_exp_word);
				अगर (loc) { /* match was found */
					अगर (scan_flags & SCAN_MATCH_बायाँ_HALF) /* #[#] */
						val = loc; /* take right part */
					अन्यथा /* %[%] */
						val = to_be_freed = xstrndup(val, loc - val); /*बायाँ*/
				}
			}
		}
#अगर BASH_PATTERN_SUBST
		अन्यथा अगर (exp_op == '/' || exp_op == '\\') {
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
			अगर (val /*&& val[0]*/) {
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
				अक्षर *pattern, *repl, *t;
				pattern = encode_then_expand_vararg(exp_word, /*handle_squotes:*/ 1, /*unbackslash:*/ 0);
				अगर (!pattern)
					pattern = xstrdup(exp_word);
				debug_printf_varexp("pattern:'%s'->'%s'\n", exp_word, pattern);
				*p++ = SPECIAL_VAR_प्रतीक;
				exp_word = p;
				p = म_अक्षर(p, SPECIAL_VAR_प्रतीक);
				*p = '\0';
				repl = encode_then_expand_vararg(exp_word, /*handle_squotes:*/ 1, /*unbackslash:*/ 1);
				debug_printf_varexp("repl:'%s'->'%s'\n", exp_word, repl);
				/* HACK ALERT. We depend here on the fact that
				 * G.global_argv and results of utoa and get_local_var_value
				 * are actually in writable memory:
				 * replace_pattern momentarily stores NULs there. */
				t = (अक्षर*)val;
				to_be_freed = replace_pattern(t,
						pattern,
						(repl ? repl : exp_word),
						exp_op);
				अगर (to_be_freed) /* at least one replace happened */
					val = to_be_freed;
				मुक्त(pattern);
				मुक्त(repl);
			} अन्यथा {
				/* Unset variable always gives nothing */
				//  a=; echo ${a/*/w}      # "w"
				//  unset a; echo ${a/*/w} # ""
				/* Just skip "replace" part */
				*p++ = SPECIAL_VAR_प्रतीक;
				p = म_अक्षर(p, SPECIAL_VAR_प्रतीक);
				*p = '\0';
			}
		}
#पूर्ण_अगर /* BASH_PATTERN_SUBST */
		अन्यथा अगर (exp_op == ':') {
#अगर BASH_SUBSTR && ENABLE_FEATURE_SH_MATH
			/* It's ${var:N[:M]} bashism.
			 * Note that in encoded form it has TWO parts:
			 * var:N<SPECIAL_VAR_SYMBOL>M<SPECIAL_VAR_SYMBOL>
			 */
			arith_t beg, len;
			अचिन्हित vallen;
			स्थिर अक्षर *errmsg;

			beg = expand_and_evaluate_arith(exp_word, &errmsg);
			अगर (errmsg)
				जाओ empty_परिणाम;
			debug_printf_varexp("beg:'%s'=%lld\n", exp_word, (दीर्घ दीर्घ)beg);
			*p++ = SPECIAL_VAR_प्रतीक;
			exp_word = p;
			p = म_अक्षर(p, SPECIAL_VAR_प्रतीक);
			*p = '\0';
			vallen = val ? म_माप(val) : 0;
			अगर (beg < 0) {
				/* negative beg counts from the end */
				beg = (arith_t)vallen + beg;
			}
			/* If expansion will be empty, do not even evaluate len */
			अगर (!val || beg < 0 || beg > vallen) {
				/* Why > vallen, not >=? bash:
				 * unset b; a=ab; : ${a:2:${b=3}}; echo $b  # "", b=3 (!!!)
				 * unset b; a=a; : ${a:2:${b=3}}; echo $b   # "", b not set
				 */
				जाओ empty_परिणाम;
			}
			len = expand_and_evaluate_arith(exp_word, &errmsg);
			अगर (errmsg)
				जाओ empty_परिणाम;
			debug_printf_varexp("len:'%s'=%lld\n", exp_word, (दीर्घ दीर्घ)len);
			debug_printf_varexp("from val:'%s'\n", val);
			अगर (len < 0) {
				/* in bash, len=-n means strlen()-n */
				len = (arith_t)vallen - beg + len;
				अगर (len < 0) /* bash compat */
					संदेश_and_die_if_script("%s: substring expression < 0", var);
			}
			अगर (len <= 0 || !val /*|| beg >= vallen*/) {
 empty_परिणाम:
				val = शून्य;
			} अन्यथा {
				/* Paranoia. What if user entered 9999999999999
				 * which fits in arith_t but not int? */
				अगर (len > पूर्णांक_उच्च)
					len = पूर्णांक_उच्च;
				val = to_be_freed = xstrndup(val + beg, len);
			}
			debug_printf_varexp("val:'%s'\n", val);
#अन्यथा /* not (HUSH_SUBSTR_EXPANSION && FEATURE_SH_MATH) */
			संदेश_and_die_if_script("malformed ${%s:...}", var);
			val = शून्य;
#पूर्ण_अगर
		} अन्यथा { /* one of "-=+?" */
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
			पूर्णांक use_word = (!val || ((exp_save == ':') && !val[0]));
			अगर (exp_op == '+')
				use_word = !use_word;
			debug_printf_expand("expand: op:%c (null:%s) test:%i\n", exp_op,
					(exp_save == ':') ? "true" : "false", use_word);
			अगर (use_word) {
				अगर (exp_op == '+' || exp_op == '-') {
					/* ${var+word} - use alternative value */
					/* ${var-word} - use default value */
					n = encode_then_append_var_plusminus(output, n, exp_word,
							/*dquoted:*/ (arg0 & 0x80)
					);
					val = शून्य;
				} अन्यथा {
					/* ${var?word} - indicate error if unset */
					/* ${var=word} - assign and use default value */
					to_be_freed = encode_then_expand_vararg(exp_word,
							/*handle_squotes:*/ !(arg0 & 0x80),
							/*unbackslash:*/ 0
					);
					अगर (to_be_freed)
						exp_word = to_be_freed;
					अगर (exp_op == '?') {
						/* mimic bash message */
						संदेश_and_die_if_script("%s: %s",
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
					} अन्यथा {
						val = exp_word;
					}
					अगर (exp_op == '=') {
						/* ${var=[word]} or ${var:=[word]} */
						अगर (है_अंक(var[0]) || var[0] == '#') {
							/* mimic bash message */
							संदेश_and_die_if_script("$%s: cannot assign in this way", var);
							val = शून्य;
						} अन्यथा {
							अक्षर *new_var = xasprintf("%s=%s", var, val);
							set_local_var0(new_var);
						}
					}
				}
			}
		} /* one of "-=+?" */

		*exp_saveptr = exp_save;
	} /* if (exp_op) */

#पूर्ण_अगर /* !__U_BOOT__ */
	arg[0] = arg0;
	*pp = p;

	n = append_str_maybe_ifs_split(output, n, first_ch, val);

	मुक्त(to_be_freed);
	वापस n;
}

/* Expand all variable references in given string, adding words to list[]
 * at n, n+1,... positions. Return updated n (so that list[n] is next one
 * to be filled). This routine is extremely tricky: has to deal with
 * variables/parameters with whitespace, $* and $@, and constructs like
 * 'echo -$*-'. If you play here, you must run testsuite afterwards! */
अटल NOINLINE पूर्णांक expand_vars_to_list(o_string *output, पूर्णांक n, अक्षर *arg)
{
	/* output->o_expflags & EXP_FLAG_SINGLEWORD (0x80) if we are in
	 * expansion of right-hand side of assignment == 1-element expand.
	 */
	अक्षर cant_be_null = 0; /* only bit 0x80 matters */
	अक्षर *p;

	debug_printf_expand("expand_vars_to_list: arg:'%s' singleword:%x\n", arg,
			!!(output->o_expflags & EXP_FLAG_SINGLEWORD));
	debug_print_list("expand_vars_to_list[0]", output, n);

	जबतक ((p = म_अक्षर(arg, SPECIAL_VAR_प्रतीक)) != शून्य) {
		अक्षर first_ch;
#अगर ENABLE_FEATURE_SH_MATH
		अक्षर arith_buf[माप(arith_t)*3 + 2];
#पूर्ण_अगर

		अगर (output->ended_in_ifs) {
			o_addchr(output, '\0');
			n = o_save_ptr(output, n);
			output->ended_in_ifs = 0;
		}

		o_addblock(output, arg, p - arg);
		debug_print_list("expand_vars_to_list[1]", output, n);
		arg = ++p;
		p = म_अक्षर(p, SPECIAL_VAR_प्रतीक);

		/* Fetch special var name (if it is indeed one of them)
		 * and quote bit, force the bit on if singleword expansion -
		 * important for not getting v=$@ expand to many words. */
		first_ch = arg[0] | (output->o_expflags & EXP_FLAG_SINGLEWORD);

		/* Is this variable quoted and thus expansion can't be null?
		 * "$@" is special. Even if quoted, it can still
		 * expand to nothing (not even an empty string),
		 * thus it is excluded. */
		अगर ((first_ch & 0x7f) != '@')
			cant_be_null |= first_ch;

		चयन (first_ch & 0x7f) {
		/* Highest bit in first_ch indicates that var is double-quoted */
		हाल '*':
		हाल '@': {
			पूर्णांक i;
#अगर_अघोषित __U_BOOT__
			अगर (!G.global_argv[1])
#अन्यथा /* __U_BOOT__ */
			अगर (!G.global_argv || !G.global_argv[1])
#पूर्ण_अगर /* __U_BOOT__ */
				अवरोध;
			i = 1;
			cant_be_null |= first_ch; /* do it for "$@" _now_, when we know it's not empty */
			अगर (!(first_ch & 0x80)) { /* unquoted $* or $@ */
				जबतक (G.global_argv[i]) {
					n = expand_on_ifs(output, n, G.global_argv[i]);
					debug_printf_expand("expand_vars_to_list: argv %d (last %d)\n", i, G.global_argc - 1);
					अगर (G.global_argv[i++][0] && G.global_argv[i]) {
						/* this argv[] is not empty and not last:
						 * put terminating NUL, start new word */
						o_addchr(output, '\0');
						debug_print_list("expand_vars_to_list[2]", output, n);
						n = o_save_ptr(output, n);
						debug_print_list("expand_vars_to_list[3]", output, n);
					}
				}
			} अन्यथा
			/* If EXP_FLAG_SINGLEWORD, we handle assignment 'a=....$@.....'
			 * and in this case should treat it like '$*' - see 'else...' below */
			अगर (first_ch == (अक्षर)('@'|0x80)  /* quoted $@ */
			 && !(output->o_expflags & EXP_FLAG_SINGLEWORD) /* not v="$@" case */
			) {
				जबतक (1) {
					o_addQstr(output, G.global_argv[i]);
					अगर (++i >= G.global_argc)
						अवरोध;
					o_addchr(output, '\0');
					debug_print_list("expand_vars_to_list[4]", output, n);
					n = o_save_ptr(output, n);
				}
			} अन्यथा { /* quoted $* (or v="$@" case): add as one word */
				जबतक (1) {
					o_addQstr(output, G.global_argv[i]);
					अगर (!G.global_argv[++i])
						अवरोध;
					अगर (G.ifs[0])
						o_addchr(output, G.ifs[0]);
				}
				output->has_quoted_part = 1;
			}
			अवरोध;
		}
		हाल SPECIAL_VAR_प्रतीक: {
			/* <SPECIAL_VAR_SYMBOL><SPECIAL_VAR_SYMBOL> */
			/* "Empty variable", used to make "" etc to not disappear */
			output->has_quoted_part = 1;
			cant_be_null = 0x80;
			arg++;
			अवरोध;
		}
		हाल SPECIAL_VAR_QUOTED_SVS:
			/* <SPECIAL_VAR_SYMBOL><SPECIAL_VAR_QUOTED_SVS><SPECIAL_VAR_SYMBOL> */
			/* "^C variable", represents literal ^C char (possible in scripts) */
			o_addchr(output, SPECIAL_VAR_प्रतीक);
			arg++;
			अवरोध;
#अगर ENABLE_HUSH_TICK
		हाल '`': {
			/* <SPECIAL_VAR_SYMBOL>`cmd<SPECIAL_VAR_SYMBOL> */
			o_string subst_परिणाम = NULL_O_STRING;

			*p = '\0'; /* replace trailing <SPECIAL_VAR_SYMBOL> */
			arg++;
			/* Can't just stuff it into output o_string,
			 * expanded result may need to be globbed
			 * and $IFS-split */
			debug_printf_subst("SUBST '%s' first_ch %x\n", arg, first_ch);
			G.last_exitcode = process_command_subs(&subst_परिणाम, arg);
			G.expand_exitcode = G.last_exitcode;
			debug_printf_subst("SUBST RES:%d '%s'\n", G.last_exitcode, subst_परिणाम.तथ्य);
			n = append_str_maybe_ifs_split(output, n, first_ch, subst_परिणाम.तथ्य);
			o_free(&subst_परिणाम);
			अवरोध;
		}
#पूर्ण_अगर
#अगर ENABLE_FEATURE_SH_MATH
		हाल '+': {
			/* <SPECIAL_VAR_SYMBOL>+arith<SPECIAL_VAR_SYMBOL> */
			arith_t res;

			arg++; /* skip '+' */
			*p = '\0'; /* replace trailing <SPECIAL_VAR_SYMBOL> */
			debug_printf_subst("ARITH '%s' first_ch %x\n", arg, first_ch);
			res = expand_and_evaluate_arith(arg, शून्य);
			debug_printf_subst("ARITH RES '"ARITH_FMT"'\n", res);
			प्र_लिखो(arith_buf, ARITH_FMT, res);
			अगर (res < 0
			 && first_ch == (अक्षर)('+'|0x80)
			/* && (output->o_expflags & EXP_FLAG_ESC_GLOB_CHARS) */
			) {
				/* Quoted negative ariths, like filename[0"$((-9))"],
				 * should not be interpreted as glob ranges.
				 * Convert leading '-' to '\-':
				 */
				o_grow_by(output, 1);
				output->तथ्य[output->लंबाई++] = '\\';
			}
			o_addstr(output, arith_buf);
			अवरोध;
		}
#पूर्ण_अगर
		शेष:
			/* <SPECIAL_VAR_SYMBOL>varname[ops]<SPECIAL_VAR_SYMBOL> */
			n = expand_one_var(output, n, first_ch, arg, &p);
			अवरोध;
		} /* switch (char after <SPECIAL_VAR_SYMBOL>) */

		/* Restore NULL'ed SPECIAL_VAR_SYMBOL.
		 * Do the check to avoid writing to a const string. */
		अगर (*p != SPECIAL_VAR_प्रतीक)
			*p = SPECIAL_VAR_प्रतीक;
		arg = ++p;
	} /* end of "while (SPECIAL_VAR_SYMBOL is found) ..." */

	अगर (*arg) {
		/* handle trailing string */
		अगर (output->ended_in_ifs) {
			o_addchr(output, '\0');
			n = o_save_ptr(output, n);
		}
		debug_print_list("expand_vars_to_list[a]", output, n);
		/* this part is literal, and it was already pre-quoted
		 * if needed (much earlier), do not use o_addQstr here!
		 */
		o_addstr(output, arg);
		debug_print_list("expand_vars_to_list[b]", output, n);
	} अन्यथा
	अगर (output->लंबाई == o_get_last_ptr(output, n) /* expansion is empty */
	 && !(cant_be_null & 0x80)   /* and all vars were not quoted */
	 && !output->has_quoted_part
	) {
		n--;
		/* allow to reuse list[n] later without re-growth */
		output->has_empty_slot = 1;
	}

	वापस n;
}

अटल अक्षर **expand_variables(अक्षर **argv, अचिन्हित expflags)
{
	पूर्णांक n;
	अक्षर **list;
	o_string output = NULL_O_STRING;

	output.o_expflags = expflags;

	n = 0;
	क्रम (;;) {
		/* go to next list[n] */
		output.ended_in_ifs = 0;
		n = o_save_ptr(&output, n);

		अगर (!*argv)
			अवरोध;

		/* expand argv[i] */
		n = expand_vars_to_list(&output, n, *argv++);
		/* if (!output->has_empty_slot) -- need this?? */
			o_addchr(&output, '\0');
	}
	debug_print_list("expand_variables", &output, n);

	/* output.data (malloced in one block) gets returned in "list" */
	list = o_finalize_list(&output, n);
	debug_print_strings("expand_variables[1]", list);
	वापस list;
}

अटल अक्षर **expand_strvec_to_strvec(अक्षर **argv)
{
	वापस expand_variables(argv, EXP_FLAG_GLOB | EXP_FLAG_ESC_GLOB_CHARS);
}

#अगर defined(CMD_SINGLEWORD_NOGLOB) || defined(CMD_TEST2_SINGLEWORD_NOGLOB)
अटल अक्षर **expand_strvec_to_strvec_singleword_noglob(अक्षर **argv)
{
	वापस expand_variables(argv, EXP_FLAG_SINGLEWORD);
}
#पूर्ण_अगर

/* Used for expansion of right hand of assignments,
 * $((...)), heredocs, variable expansion parts.
 *
 * NB: should NOT do globbing!
 * "export v=/bin/c*; env | grep ^v=" outputs "v=/bin/c*"
 */
अटल अक्षर *expand_string_to_string(स्थिर अक्षर *str, पूर्णांक EXP_flags, पूर्णांक do_unbackslash)
{
#अगर !BASH_PATTERN_SUBST && !ENABLE_HUSH_CASE
	स्थिर पूर्णांक do_unbackslash = 1;
	स्थिर पूर्णांक EXP_flags = EXP_FLAG_ESC_GLOB_CHARS;
#पूर्ण_अगर
	अक्षर *argv[2], **list;

	debug_printf_expand("string_to_string<='%s'\n", str);
	/* This is generally an optimization, but it also
	 * handles "", which otherwise trips over !list[0] check below.
	 * (is this ever happens that we actually get str="" here?)
	 */
	अगर (!म_अक्षर(str, SPECIAL_VAR_प्रतीक) && !म_अक्षर(str, '\\')) {
		//TODO: Can use on strings with \ too, just unbackslash() them?
		debug_printf_expand("string_to_string(fast)=>'%s'\n", str);
		वापस xstrdup(str);
	}

	argv[0] = (अक्षर*)str;
	argv[1] = शून्य;
	list = expand_variables(argv, EXP_flags | EXP_FLAG_SINGLEWORD);
	अगर (!list[0]) {
		/* Example where it happens:
		 * x=; echo ${x:-"$@"}
		 */
		((अक्षर*)list)[0] = '\0';
	} अन्यथा {
		अगर (HUSH_DEBUG)
			अगर (list[1])
				bb_simple_error_संदेश_and_die("BUG in varexp2");
		/* actually, just move string 2*sizeof(char*) bytes back */
		overlapping_strcpy((अक्षर*)list, list[0]);
		अगर (do_unbackslash)
			unbackslash((अक्षर*)list);
	}
	debug_printf_expand("string_to_string=>'%s'\n", (अक्षर*)list);
	वापस (अक्षर*)list;
}

#अगर 0
अटल अक्षर* expand_strvec_to_string(अक्षर **argv)
{
	अक्षर **list;

	list = expand_variables(argv, EXP_FLAG_SINGLEWORD);
	/* Convert all NULs to spaces */
	अगर (list[0]) {
		पूर्णांक n = 1;
		जबतक (list[n]) {
			अगर (HUSH_DEBUG)
				अगर (list[n-1] + म_माप(list[n-1]) + 1 != list[n])
					bb_error_संदेश_and_die("BUG in varexp3");
			/* bash uses ' ' regardless of $IFS contents */
			list[n][-1] = ' ';
			n++;
		}
	}
	overlapping_strcpy((अक्षर*)list, list[0] ? list[0] : "");
	debug_printf_expand("strvec_to_string='%s'\n", (अक्षर*)list);
	वापस (अक्षर*)list;
}
#पूर्ण_अगर

#अगर_अघोषित __U_BOOT__
अटल अक्षर **expand_assignments(अक्षर **argv, पूर्णांक गिनती)
{
	पूर्णांक i;
	अक्षर **p;

	G.expanded_assignments = p = शून्य;
	/* Expand assignments into one string each */
	क्रम (i = 0; i < गिनती; i++) {
		p = add_string_to_strings(p,
			expand_string_to_string(argv[i],
				EXP_FLAG_ESC_GLOB_CHARS,
				/*unbackslash:*/ 1
			)
		);
		G.expanded_assignments = p;
	}
	G.expanded_assignments = शून्य;
	वापस p;
}

अटल व्योम switch_off_special_sigs(अचिन्हित mask)
{
	अचिन्हित sig = 0;
	जबतक ((mask >>= 1) != 0) {
		sig++;
		अगर (!(mask & 1))
			जारी;
#अगर ENABLE_HUSH_TRAP
		अगर (G_traps) {
			अगर (G_traps[sig] && !G_traps[sig][0])
				/* trap is '', has to remain SIG_IGN */
				जारी;
			मुक्त(G_traps[sig]);
			G_traps[sig] = शून्य;
		}
#पूर्ण_अगर
		/* We are here only if no trap or trap was not '' */
		install_sighandler(sig, संक_पूर्व);
	}
}
#पूर्ण_अगर /* !__U_BOOT__ */

#अगर_अघोषित __U_BOOT__
#अगर BB_MMU
/* never called */
व्योम re_execute_shell(अक्षर ***to_free, स्थिर अक्षर *s,
		अक्षर *g_argv0, अक्षर **g_argv,
		अक्षर **builtin_argv) NORETURN;

अटल व्योम reset_traps_to_defaults(व्योम)
{
	/* This function is always called in a child shell
	 * after fork (not vfork, NOMMU doesn't use this function).
	 */
	IF_HUSH_TRAP(अचिन्हित sig;)
	अचिन्हित mask;

	/* Child shells are not interactive.
	 * SIGTTIN/SIGTTOU/SIGTSTP should not have special handling.
	 * Testcase: (while :; do :; done) + ^Z should background.
	 * Same goes for SIGTERM, SIGHUP, SIGINT.
	 */
	mask = (G.special_sig_mask & SPECIAL_INTERACTIVE_SIGS) | G_fatal_sig_mask;
	अगर (!G_traps && !mask)
		वापस; /* already no traps and no special sigs */

	/* Switch off special sigs */
	switch_off_special_sigs(mask);
# अगर ENABLE_HUSH_JOB
	G_fatal_sig_mask = 0;
# endif
	G.special_sig_mask &= ~SPECIAL_INTERACTIVE_SIGS;
	/* SIGQUIT,SIGCHLD and maybe SPECIAL_JOBSTOP_SIGS
	 * remain set in G.special_sig_mask */

# अगर ENABLE_HUSH_TRAP
	अगर (!G_traps)
		वापस;

	/* Reset all sigs to default except ones with empty traps */
	क्रम (sig = 0; sig < NSIG; sig++) {
		अगर (!G_traps[sig])
			जारी; /* no trap: nothing to do */
		अगर (!G_traps[sig][0])
			जारी; /* empty trap: has to remain SIG_IGN */
		/* sig has non-empty trap, reset it: */
		मुक्त(G_traps[sig]);
		G_traps[sig] = शून्य;
		/* There is no signal for trap 0 (EXIT) */
		अगर (sig == 0)
			जारी;
		install_sighandler(sig, pick_sighandler(sig));
	}
# endif
}

#अन्यथा /* !BB_MMU */

अटल व्योम re_execute_shell(अक्षर ***to_free, स्थिर अक्षर *s,
		अक्षर *g_argv0, अक्षर **g_argv,
		अक्षर **builtin_argv) NORETURN;
अटल व्योम re_execute_shell(अक्षर ***to_free, स्थिर अक्षर *s,
		अक्षर *g_argv0, अक्षर **g_argv,
		अक्षर **builtin_argv)
{
# define NOMMU_HACK_FMT ("-$%x:%x:%x:%x:%x:%llx" IF_HUSH_LOOPS(":%x"))
	/* delims + 2 * (number of bytes in printed hex numbers) */
	अक्षर param_buf[माप(NOMMU_HACK_FMT) + 2 * (माप(पूर्णांक)*6 + माप(दीर्घ दीर्घ)*1)];
	अक्षर *heredoc_argv[4];
	काष्ठा variable *cur;
# अगर ENABLE_HUSH_FUNCTIONS
	काष्ठा function *funcp;
# endif
	अक्षर **argv, **pp;
	अचिन्हित cnt;
	अचिन्हित दीर्घ दीर्घ empty_trap_mask;

	अगर (!g_argv0) { /* heredoc */
		argv = heredoc_argv;
		argv[0] = (अक्षर *) G.argv0_for_re_execing;
		argv[1] = (अक्षर *) "-<";
		argv[2] = (अक्षर *) s;
		argv[3] = शून्य;
		pp = &argv[3]; /* used as pointer to empty environment */
		जाओ do_exec;
	}

	cnt = 0;
	pp = builtin_argv;
	अगर (pp) जबतक (*pp++)
		cnt++;

	empty_trap_mask = 0;
	अगर (G_traps) {
		पूर्णांक sig;
		क्रम (sig = 1; sig < NSIG; sig++) {
			अगर (G_traps[sig] && !G_traps[sig][0])
				empty_trap_mask |= 1LL << sig;
		}
	}

	प्र_लिखो(param_buf, NOMMU_HACK_FMT
			, (अचिन्हित) G.मूल_pid
			, (अचिन्हित) G.मूल_ppid
			, (अचिन्हित) G.last_bg_pid
			, (अचिन्हित) G.last_exitcode
			, cnt
			, empty_trap_mask
			IF_HUSH_LOOPS(, G.depth_of_loop)
			);
# undef NOMMU_HACK_FMT
	/* 1:hush 2:-$<pid>:<pid>:<exitcode>:<etc...> <vars...> <funcs...>
	 * 3:-c 4:<cmd> 5:<arg0> <argN...> 6:NULL
	 */
	cnt += 6;
	क्रम (cur = G.शीर्ष_var; cur; cur = cur->अग्र) {
		अगर (!cur->flg_export || cur->flg_read_only)
			cnt += 2;
	}
# अगर ENABLE_HUSH_LINENO_VAR
	cnt += 2;
# endif
# अगर ENABLE_HUSH_FUNCTIONS
	क्रम (funcp = G.शीर्ष_func; funcp; funcp = funcp->अग्र)
		cnt += 3;
# endif
	pp = g_argv;
	जबतक (*pp++)
		cnt++;
	*to_free = argv = pp = xzalloc(माप(argv[0]) * cnt);
	*pp++ = (अक्षर *) G.argv0_for_re_execing;
	*pp++ = param_buf;
	क्रम (cur = G.शीर्ष_var; cur; cur = cur->अग्र) {
		अगर (म_भेद(cur->varstr, hush_version_str) == 0)
			जारी;
		अगर (cur->flg_read_only) {
			*pp++ = (अक्षर *) "-R";
			*pp++ = cur->varstr;
		} अन्यथा अगर (!cur->flg_export) {
			*pp++ = (अक्षर *) "-V";
			*pp++ = cur->varstr;
		}
	}
# अगर ENABLE_HUSH_LINENO_VAR
	*pp++ = (अक्षर *) "-L";
	*pp++ = utoa(G.execute_lineno);
# endif
# अगर ENABLE_HUSH_FUNCTIONS
	क्रम (funcp = G.शीर्ष_func; funcp; funcp = funcp->अग्र) {
		*pp++ = (अक्षर *) "-F";
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
	*pp++ = (अक्षर *) "-c";
	*pp++ = (अक्षर *) s;
	अगर (builtin_argv) {
		जबतक (*++builtin_argv)
			*pp++ = *builtin_argv;
		*pp++ = (अक्षर *) "";
	}
	*pp++ = g_argv0;
	जबतक (*g_argv)
		*pp++ = *g_argv++;
	/* *pp = NULL; - is already there */
	pp = environ;

 do_exec:
	debug_printf_exec("re_execute_shell pid:%d cmd:'%s'\n", getpid(), s);
	/* Don't propagate SIG_IGN to the child */
	अगर (SPECIAL_JOBSTOP_SIGS != 0)
		switch_off_special_sigs(G.special_sig_mask & SPECIAL_JOBSTOP_SIGS);
	execve(bb_busybox_exec_path, argv, pp);
	/* Fallback. Useful for init=/bin/hush usage etc */
	अगर (argv[0][0] == '/')
		execve(argv[0], argv, pp);
	xfunc_error_retval = 127;
	bb_simple_error_संदेश_and_die("can't re-execute the shell");
}
#पूर्ण_अगर  /* !BB_MMU */

#पूर्ण_अगर /* !__U_BOOT__ */

अटल पूर्णांक run_and_free_list(काष्ठा pipe *pi);

/* Executing from string: eval, sh -c '...'
 *          or from file: /etc/profile, . file, sh <script>, sh (intereactive)
 * end_trigger controls how often we stop parsing
 * NUL: parse all, execute, return
 * ';': parse till ';' or newline, execute, repeat till EOF
 */
#अगर_अघोषित __U_BOOT__
अटल व्योम parse_and_run_stream(काष्ठा in_str *inp, पूर्णांक end_trigger)
#अन्यथा /* __U_BOOT__ */
अटल पूर्णांक parse_and_run_stream(काष्ठा in_str *inp, पूर्णांक end_trigger)
#पूर्ण_अगर /* __U_BOOT__ */
{
	/* Why we need empty flag?
	 * An obscure corner case "false; ``; echo $?":
	 * empty command in `` should still set $? to 0.
	 * But we can't just set $? to 0 at the start,
	 * this breaks "false; echo `echo $?`" case.
	 */
	bool empty = 1;
#अगर_अघोषित __U_BOOT__
	जबतक (1) {
#अन्यथा /* __U_BOOT__ */
	करो {
#पूर्ण_अगर /* __U_BOOT__ */
		काष्ठा pipe *pipe_list;

#अगर ENABLE_HUSH_INTERACTIVE
		अगर (end_trigger == ';') {
			G.promptmode = 0; /* PS1 */
			debug_printf_prompt("%s promptmode=%d\n", __func__, G.promptmode);
		}
#पूर्ण_अगर
		pipe_list = parse_stream(शून्य, शून्य, inp, end_trigger);
		अगर (!pipe_list || pipe_list == ERR_PTR) { /* EOF/error */
			/* If we are in "big" script
			 * (not in `cmd` or something similar)...
			 */
			अगर (pipe_list == ERR_PTR && end_trigger == ';') {
				/* Discard cached input (rest of line) */
				पूर्णांक ch = inp->last_char;
				जबतक (ch != खातापूर्ण && ch != '\n') {
					//bb_error_msg("Discarded:'%c'", ch);
					ch = i_getch(inp);
				}
				/* Force prompt */
				inp->p = शून्य;
				/* This stream isn't empty */
				empty = 0;
				जारी;
			}
			अगर (!pipe_list && empty)
				G.last_exitcode = 0;
			अवरोध;
		}
		debug_print_वृक्ष(pipe_list, 0);
		debug_printf_exec("parse_and_run_stream: run_and_free_list\n");
#अगर_अघोषित __U_BOOT__
		run_and_free_list(pipe_list);
#अन्यथा /* __U_BOOT__ */
		पूर्णांक rcode = run_and_free_list(pipe_list);
		/*
		 * We reset input string to not run the following command, so running
		 * 'exit; echo foo' does not print foo.
		 */
		अगर (rcode <= EXIT_RET_CODE)
			setup_file_in_str(inp);
#पूर्ण_अगर /* __U_BOOT__ */
		empty = 0;
		अगर (G_flag_return_in_progress == 1)
			अवरोध;
#अगर_अघोषित __U_BOOT__
	}
#अन्यथा /* __U_BOOT__ */
	/*
	 * This do/while is needed by run_command to avoid looping on a command
	 * with syntax error.
	 */
	} जबतक (!(G.run_command_flags & FLAG_EXIT_FROM_LOOP));

	वापस G.last_exitcode;
#पूर्ण_अगर /* __U_BOOT__ */
}

#अगर_अघोषित __U_BOOT__
अटल व्योम parse_and_run_string(स्थिर अक्षर *s)
#अन्यथा /* __U_BOOT__ */
अटल पूर्णांक parse_and_run_string(स्थिर अक्षर *s)
#पूर्ण_अगर /* __U_BOOT__ */
{
	काष्ठा in_str निवेश;
#अगर_अघोषित __U_BOOT__
	IF_HUSH_LINENO_VAR(अचिन्हित sv = G.parse_lineno;)
#अन्यथा /* __U_BOOT__ */
	//IF_HUSH_LINENO_VAR(unsigned sv = G.parse_lineno;)
#पूर्ण_अगर /* __U_BOOT__ */

	setup_string_in_str(&निवेश, s);
#अगर_अघोषित __U_BOOT__
	parse_and_run_stream(&निवेश, '\0');
#अन्यथा /* __U_BOOT__ */
	वापस parse_and_run_stream(&निवेश, '\0');
#पूर्ण_अगर /* __U_BOOT__ */
#अगर_अघोषित __U_BOOT__
	IF_HUSH_LINENO_VAR(अचिन्हित sv = G.parse_lineno;)
#अन्यथा /* __U_BOOT__ */
	//IF_HUSH_LINENO_VAR(unsigned sv = G.parse_lineno;)
#पूर्ण_अगर /* __U_BOOT__ */
}

#अगर_घोषित __U_BOOT__
पूर्णांक parse_string_outer_modern(स्थिर अक्षर *cmd, पूर्णांक flags)
{
	पूर्णांक ret;
	पूर्णांक old_flags;

	/*
	 * Keep old values of run_command to be able to restore them once
	 * command was executed.
	 */
	old_flags = G.run_command_flags;
	G.run_command_flags = flags;

	ret = parse_and_run_string(cmd);

	G.run_command_flags = old_flags;

	वापस ret;
}
#पूर्ण_अगर /* __U_BOOT__ */
#अगर_अघोषित __U_BOOT__
अटल व्योम parse_and_run_file(HFILE *fp)
#अन्यथा /* __U_BOOT__ */
व्योम parse_and_run_file(व्योम)
#पूर्ण_अगर /* __U_BOOT__ */
{
	काष्ठा in_str निवेश;
#अगर_अघोषित __U_BOOT__
	IF_HUSH_LINENO_VAR(अचिन्हित sv = G.parse_lineno;)

	IF_HUSH_LINENO_VAR(G.parse_lineno = 1;)
	setup_file_in_str(&निवेश, fp);
#अन्यथा /* __U_BOOT__ */
	setup_file_in_str(&निवेश);
#पूर्ण_अगर /* __U_BOOT__ */
	parse_and_run_stream(&निवेश, ';');
#अगर_अघोषित __U_BOOT__
	IF_HUSH_LINENO_VAR(G.parse_lineno = sv;)
#पूर्ण_अगर /* !__U_BOOT__ */
}

#अगर_अघोषित __U_BOOT__
#अगर ENABLE_HUSH_TICK
अटल पूर्णांक generate_stream_from_string(स्थिर अक्षर *s, pid_t *pid_p)
{
	pid_t pid;
	पूर्णांक channel[2];
# अगर !BB_MMU
	अक्षर **to_free = शून्य;
# endif

	xpipe(channel);
	pid = BB_MMU ? xfork() : xvfork();
	अगर (pid == 0) { /* child */
		disable_restore_tty_pgrp_on_exit();
		/* Process substitution is not considered to be usual
		 * 'command execution'.
		 * SUSv3 says ctrl-Z should be ignored, ctrl-C should not.
		 */
		bb_signals(0
			+ (1 << SIGTSTP)
			+ (1 << SIGTTIN)
			+ (1 << SIGTTOU)
			, संक_छोड़ो);
		बंद(channel[0]); /* NB: close _first_, then move fd! */
		xmove_fd(channel[1], 1);
# अगर ENABLE_HUSH_TRAP
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
		अगर (is_prefixed_with(s, "trap")
		 && skip_whitespace(s + 4)[0] == '\0'
		) {
			अटल स्थिर अक्षर *स्थिर argv[] ALIGN_PTR = { शून्य, शून्य };
			builtin_trap((अक्षर**)argv);
			fflush_all(); /* important */
			_निकास(0);
		}
# endif
# अगर BB_MMU
		/* Prevent it from trying to handle ctrl-z etc */
		IF_HUSH_JOB(G.run_list_level = 1;)
		CLEAR_RANDOM_T(&G.random_gen); /* or else $RANDOM repeats in child */
		reset_traps_to_defaults();
		IF_HUSH_MODE_X(G.x_mode_depth++;)
		//bb_error_msg("%s: ++x_mode_depth=%d", __func__, G.x_mode_depth);
		parse_and_run_string(s);
		_निकास(G.last_exitcode);
# अन्यथा
	/* We re-execute after vfork on NOMMU. This makes this script safe:
	 * yes "0123456789012345678901234567890" | dd bs=32 count=64k >BIG
	 * huge=`cat BIG` # was blocking here forever
	 * echo OK
	 */
		re_execute_shell(&to_free,
				s,
				G.global_argv[0],
				G.global_argv + 1,
				शून्य);
# endif
	}

	/* parent */
	*pid_p = pid;
# अगर ENABLE_HUSH_FAST
	G.गिनती_SIGCHLD++;
//bb_error_msg("[%d] fork in generate_stream_from_string:"
//		" G.count_SIGCHLD:%d G.handled_SIGCHLD:%d",
//		getpid(), G.count_SIGCHLD, G.handled_SIGCHLD);
# endif
	enable_restore_tty_pgrp_on_exit();
# अगर !BB_MMU
	मुक्त(to_free);
# endif
	बंद(channel[1]);
	वापस channel[0];
}

/* Return code is exit status of the process that is run. */
अटल पूर्णांक process_command_subs(o_string *dest, स्थिर अक्षर *s)
{
	खाता *fp;
	pid_t pid;
	पूर्णांक status, ch, eol_cnt;

	fp = xfdopen_for_read(generate_stream_from_string(s, &pid));

	/* Now send results of command back into original context */
	eol_cnt = 0;
	जबतक ((ch = अ_लो(fp)) != खातापूर्ण) {
		अगर (ch == '\0')
			जारी;
		अगर (ch == '\n') {
			eol_cnt++;
			जारी;
		}
		जबतक (eol_cnt) {
			o_addchr(dest, '\n');
			eol_cnt--;
		}
		o_addQchr(dest, ch);
	}

	debug_printf("done reading from `cmd` pipe, closing it\n");
	ख_बंद(fp);
	/* We need to extract exitcode. Test case
	 * "true; echo `sleep 1; false` $?"
	 * should print 1 */
	safe_waitpid(pid, &status, 0);
	debug_printf("child exited. returning its exitcode:%d\n", WEXITSTATUS(status));
	वापस WEXITSTATUS(status);
}
#पूर्ण_अगर /* ENABLE_HUSH_TICK */

अटल व्योम setup_heredoc(काष्ठा redir_struct *redir)
{
	काष्ठा fd_pair pair;
	pid_t pid;
	पूर्णांक len, written;
	/* the _body_ of heredoc (misleading field name) */
	स्थिर अक्षर *heredoc = redir->rd_filename;
	अक्षर *expanded;
#अगर !BB_MMU
	अक्षर **to_free;
#पूर्ण_अगर

	expanded = शून्य;
	अगर (!(redir->rd_dup & HEREDOC_QUOTED)) {
		expanded = encode_then_expand_string(heredoc);
		अगर (expanded)
			heredoc = expanded;
	}
	len = म_माप(heredoc);

	बंद(redir->rd_fd); /* often saves dup2+close in xmove_fd */
	xpiped_pair(pair);
	xmove_fd(pair.rd, redir->rd_fd);

	/* Try writing without forking. Newer kernels have
	 * dynamically growing pipes. Must use non-blocking write! */
	ndelay_on(pair.wr);
	जबतक (1) {
		written = ग_लिखो(pair.wr, heredoc, len);
		अगर (written <= 0)
			अवरोध;
		len -= written;
		अगर (len == 0) {
			बंद(pair.wr);
			मुक्त(expanded);
			वापस;
		}
		heredoc += written;
	}
	ndelay_off(pair.wr);

	/* Okay, pipe buffer was not big enough */
	/* Note: we must not create a stray child (bastard? :)
	 * for the unsuspecting parent process. Child creates a grandchild
	 * and exits before parent execs the process which consumes heredoc
	 * (that exec happens after we return from this function) */
#अगर !BB_MMU
	to_free = शून्य;
#पूर्ण_अगर
	pid = xvfork();
	अगर (pid == 0) {
		/* child */
		disable_restore_tty_pgrp_on_exit();
		pid = BB_MMU ? xfork() : xvfork();
		अगर (pid != 0)
			_निकास(0);
		/* grandchild */
		बंद(redir->rd_fd); /* read side of the pipe */
#अगर BB_MMU
		full_write(pair.wr, heredoc, len); /* may loop or block */
		_निकास(0);
#अन्यथा
		/* Delegate blocking writes to another process */
		xmove_fd(pair.wr, STDOUT_FILENO);
		re_execute_shell(&to_free, heredoc, शून्य, शून्य, शून्य);
#पूर्ण_अगर
	}
	/* parent */
#अगर ENABLE_HUSH_FAST
	G.गिनती_SIGCHLD++;
//bb_error_msg("[%d] fork in setup_heredoc: G.count_SIGCHLD:%d G.handled_SIGCHLD:%d", getpid(), G.count_SIGCHLD, G.handled_SIGCHLD);
#पूर्ण_अगर
	enable_restore_tty_pgrp_on_exit();
#अगर !BB_MMU
	मुक्त(to_free);
#पूर्ण_अगर
	बंद(pair.wr);
	मुक्त(expanded);
	रुको(शून्य); /* wait till child has died */
}

काष्ठा squirrel {
	पूर्णांक orig_fd;
	पूर्णांक moved_to;
	/* moved_to = n: fd was moved to n; restore back to orig_fd after redir */
	/* moved_to = -1: fd was opened by redirect; close orig_fd after redir */
};

अटल काष्ठा squirrel *append_squirrel(काष्ठा squirrel *sq, पूर्णांक i, पूर्णांक orig, पूर्णांक moved)
{
	sq = xrealloc(sq, (i + 2) * माप(sq[0]));
	sq[i].orig_fd = orig;
	sq[i].moved_to = moved;
	sq[i+1].orig_fd = -1; /* end marker */
	वापस sq;
}

अटल काष्ठा squirrel *add_squirrel(काष्ठा squirrel *sq, पूर्णांक fd, पूर्णांक avoid_fd)
{
	पूर्णांक moved_to;
	पूर्णांक i;

	i = 0;
	अगर (sq) क्रम (; sq[i].orig_fd >= 0; i++) {
		/* If we collide with an already moved fd... */
		अगर (fd == sq[i].moved_to) {
			moved_to = dup_CLOEXEC(sq[i].moved_to, avoid_fd);
			debug_printf_redir("redirect_fd %d: already busy, moving to %d\n", fd, moved_to);
			अगर (moved_to < 0) {
				/* "echo 2>/dev/tty 10>&9999" testcase:
				 * We move fd 2 to 10, then discover we need to move fd 10
				 * (and not hit 9999) and the latter fails.
				 */
				वापस शून्य; /* fcntl failed */
			}
			sq[i].moved_to = moved_to;
			वापस sq;
		}
		अगर (fd == sq[i].orig_fd) {
			/* Example: echo Hello >/dev/null 1>&2 */
			debug_printf_redir("redirect_fd %d: already moved\n", fd);
			वापस sq;
		}
	}

	/* If this fd is open, we move and remember it; if it's closed, moved_to = -1 */
	moved_to = dup_CLOEXEC(fd, avoid_fd);
	debug_printf_redir("redirect_fd %d: previous fd is moved to %d (-1 if it was closed)\n", fd, moved_to);
	अगर (moved_to < 0 && त्रुटि_सं != EBADF)
		वापस शून्य; /* fcntl failed (not because fd is closed) */
	वापस append_squirrel(sq, i, fd, moved_to);
}

अटल काष्ठा squirrel *add_squirrel_closed(काष्ठा squirrel *sq, पूर्णांक fd)
{
	पूर्णांक i;

	i = 0;
	अगर (sq) क्रम (; sq[i].orig_fd >= 0; i++) {
		/* If we collide with an already moved fd... */
		अगर (fd == sq[i].orig_fd) {
			/* Examples:
			 * "echo 3>FILE 3>&- 3>FILE"
			 * "echo 3>&- 3>FILE"
			 * No need for last redirect to insert
			 * another "need to close 3" indicator.
			 */
			debug_printf_redir("redirect_fd %d: already moved or closed\n", fd);
			वापस sq;
		}
	}

	debug_printf_redir("redirect_fd %d: previous fd was closed\n", fd);
	वापस append_squirrel(sq, i, fd, -1);
}

/* fd: redirect wants this fd to be used (e.g. 3>file).
 * Move all conflicting internally used fds,
 * and remember them so that we can restore them later.
 */
अटल पूर्णांक save_fd_on_redirect(पूर्णांक fd, पूर्णांक avoid_fd, काष्ठा squirrel **sqp)
{
	काष्ठा squirrel *new_squirrel;

	अगर (avoid_fd < 9) /* the important case here is that it can be -1 */
		avoid_fd = 9;

#अगर ENABLE_HUSH_INTERACTIVE
	अगर (fd != 0 /* don't trigger for G_interactive_fd == 0 (that's "not interactive" flag) */
	 && fd == G_interactive_fd
	) {
		/* Testcase: "ls -l /proc/$$/fd 255>&-" should work */
		G_interactive_fd = xdup_CLOEXEC_and_close(G_interactive_fd, avoid_fd);
		debug_printf_redir("redirect_fd %d: matches interactive_fd, moving it to %d\n", fd, G_interactive_fd);
		वापस 1; /* "we closed fd" */
	}
#पूर्ण_अगर
	/* Are we called from setup_redirects(squirrel==NULL)
	 * in redirect in a [v]forked child?
	 */
	अगर (sqp == शून्य) {
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
		वापस 0;
	}

	/* If this one of script's fds? */
	अगर (move_HFILEs_on_redirect(fd, avoid_fd))
		वापस 1; /* yes. "we closed fd" (actually moved it) */

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
	अगर (sqp == ERR_PTR) {
		/* Don't preserve redirected fds: exec is _meant_ to change these */
		//bb_error_msg("sqp == ERR_PTR: exec >FILE");
		वापस 0;
	}

	/* Check whether it collides with any open fds (e.g. stdio), save fds as needed */
	new_squirrel = add_squirrel(*sqp, fd, avoid_fd);
	अगर (!new_squirrel)
		वापस -1; /* redirect error */
	*sqp = new_squirrel;
	वापस 0; /* "we did not close fd" */
}

अटल व्योम restore_redirects(काष्ठा squirrel *sq)
{
	अगर (sq) {
		पूर्णांक i;
		क्रम (i = 0; sq[i].orig_fd >= 0; i++) {
			अगर (sq[i].moved_to >= 0) {
				/* We simply die on error */
				debug_printf_redir("restoring redirected fd from %d to %d\n", sq[i].moved_to, sq[i].orig_fd);
				xmove_fd(sq[i].moved_to, sq[i].orig_fd);
			} अन्यथा {
				/* cmd1 9>FILE; cmd2_should_see_fd9_closed */
				debug_printf_redir("restoring redirected fd %d: closing it\n", sq[i].orig_fd);
				बंद(sq[i].orig_fd);
			}
		}
		मुक्त(sq);
	}
	अगर (G.HFILE_stdin
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

#अगर ENABLE_FEATURE_SH_STANDALONE && BB_MMU
अटल व्योम close_saved_fds_and_FILE_fds(व्योम)
{
	अगर (G_interactive_fd)
		बंद(G_interactive_fd);
	close_all_HFILE_list();
}
#पूर्ण_अगर

अटल पूर्णांक internally_opened_fd(पूर्णांक fd, काष्ठा squirrel *sq)
{
	पूर्णांक i;

#अगर ENABLE_HUSH_INTERACTIVE
	अगर (fd == G_interactive_fd)
		वापस 1;
#पूर्ण_अगर
	/* If this one of script's fds? */
	अगर (fd_in_HFILEs(fd))
		वापस 1;

	अगर (sq) क्रम (i = 0; sq[i].orig_fd >= 0; i++) {
		अगर (fd == sq[i].moved_to)
			वापस 1;
	}
	वापस 0;
}

/* sqp != NULL means we squirrel away copies of stdin, stdout,
 * and stderr if they are redirected.
 * If redirection fails, return 1. This will make caller
 * skip command execution and restore already created redirect fds.
 */
अटल पूर्णांक setup_redirects(काष्ठा command *prog, काष्ठा squirrel **sqp)
{
	काष्ठा redir_struct *redir;

	क्रम (redir = prog->redirects; redir; redir = redir->अग्र) {
		पूर्णांक newfd;
		पूर्णांक closed;

		अगर (redir->rd_type == REDIRECT_HEREDOC2) {
			/* "rd_fd<<HERE" case */
			अगर (save_fd_on_redirect(redir->rd_fd, /*avoid:*/ 0, sqp) < 0)
				वापस 1;
			/* for REDIRECT_HEREDOC2, rd_filename holds _contents_
			 * of the heredoc */
			debug_printf_redir("set heredoc '%s'\n",
					redir->rd_filename);
			setup_heredoc(redir);
			जारी;
		}

		अगर (redir->rd_dup == REDIRFD_TO_FILE) {
			/* "rd_fd<*>file" case (<*> is <,>,>>,<>) */
			अक्षर *p;
			पूर्णांक mode;

			अगर (redir->rd_filename == शून्य) {
				/* Examples:
				 * "cmd >" (no filename)
				 * "cmd > <file" (2nd redirect starts too early)
				 */
				syntax_error("invalid redirect");
				वापस 1;
			}
			mode = redir_table[redir->rd_type].mode;
			p = expand_string_to_string(redir->rd_filename,
				EXP_FLAG_ESC_GLOB_CHARS, /*unbackslash:*/ 1);
			newfd = open_or_warn(p, mode);
			मुक्त(p);
			अगर (newfd < 0) {
				/* Error message from open_or_warn can be lost
				 * if stderr has been redirected, but bash
				 * and ash both lose it as well
				 * (though zsh doesn't!)
				 */
				वापस 1;
			}
			अगर (newfd == redir->rd_fd && sqp
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
		} अन्यथा {
			/* "rd_fd>&rd_dup" or "rd_fd>&-" case */
			newfd = redir->rd_dup;
		}

		अगर (newfd == redir->rd_fd)
			जारी;

		/* if "N>FILE": move newfd to redir->rd_fd */
		/* if "N>&M": dup newfd to redir->rd_fd */
		/* if "N>&-": close redir->rd_fd (newfd is REDIRFD_CLOSE) */

		closed = save_fd_on_redirect(redir->rd_fd, /*avoid:*/ newfd, sqp);
		अगर (closed < 0)
			वापस 1; /* error */
		अगर (newfd == REDIRFD_CLOSE) {
			/* "N>&-" means "close me" */
			अगर (!closed) {
				/* ^^^ optimization: saving may already
				 * have closed it. If not... */
				बंद(redir->rd_fd);
			}
			/* Sometimes we do another close on restore, getting EBADF.
			 * Consider "echo 3>FILE 3>&-"
			 * first redirect remembers "need to close 3",
			 * and second redirect closes 3! Restore code then closes 3 again.
			 */
		} अन्यथा {
			/* if newfd is a script fd or saved fd, do not allow to use it */
			अगर (internally_opened_fd(newfd, sqp && sqp != ERR_PTR ? *sqp : शून्य)) {
				bb_error_संदेश("fd#%d is not open", newfd);
				वापस 1;
			}
			अगर (dup2(newfd, redir->rd_fd) < 0) {
				/* "echo >&99" testcase */
				bb_perror_संदेश("dup2(%d,%d)", newfd, redir->rd_fd);
				वापस 1;
			}
			अगर (redir->rd_dup == REDIRFD_TO_FILE)
				/* "rd_fd > FILE" */
				बंद(newfd);
			/* else: "rd_fd > rd_dup" */
		}
	}
	वापस 0;
}

अटल अक्षर *find_in_path(स्थिर अक्षर *arg)
{
	अक्षर *ret = शून्य;
	स्थिर अक्षर *PATH = get_local_var_मूल्य("PATH");

	अगर (!PATH)
		वापस शून्य;

	जबतक (1) {
		स्थिर अक्षर *end = strchrnul(PATH, ':');
		पूर्णांक sz = end - PATH; /* must be int! */

		मुक्त(ret);
		अगर (sz != 0) {
			ret = xasprintf("%.*s/%s", sz, PATH, arg);
		} अन्यथा {
			/* We have xxx::yyyy in $PATH,
			 * it means "use current dir" */
			ret = xstrdup(arg);
		}
		अगर (access(ret, F_OK) == 0)
			अवरोध;

		अगर (*end == '\0') {
			मुक्त(ret);
			वापस शून्य;
		}
		PATH = end + 1;
	}

	वापस ret;
}

अटल स्थिर काष्ठा built_in_command *find_builtin_helper(स्थिर अक्षर *name,
		स्थिर काष्ठा built_in_command *x,
		स्थिर काष्ठा built_in_command *end)
{
	जबतक (x != end) {
		अगर (म_भेद(name, x->b_cmd) != 0) {
			x++;
			जारी;
		}
		debug_printf_exec("found builtin '%s'\n", name);
		वापस x;
	}
	वापस शून्य;
}
अटल स्थिर काष्ठा built_in_command *find_builtin1(स्थिर अक्षर *name)
{
	वापस find_builtin_helper(name, bltins1, &bltins1[ARRAY_आकार(bltins1)]);
}
अटल स्थिर काष्ठा built_in_command *find_builtin(स्थिर अक्षर *name)
{
	स्थिर काष्ठा built_in_command *x = find_builtin1(name);
	अगर (x)
		वापस x;
	वापस find_builtin_helper(name, bltins2, &bltins2[ARRAY_आकार(bltins2)]);
}

#अगर ENABLE_HUSH_JOB && ENABLE_FEATURE_TAB_COMPLETION
अटल स्थिर अक्षर * FAST_FUNC hush_command_name(पूर्णांक i)
{
	अगर (/*i >= 0 && */ i < ARRAY_आकार(bltins1)) {
		वापस bltins1[i].b_cmd;
	}
	i -= ARRAY_आकार(bltins1);
	अगर (i < ARRAY_आकार(bltins2)) {
		वापस bltins2[i].b_cmd;
	}
# अगर ENABLE_HUSH_FUNCTIONS
	{
		काष्ठा function *funcp;
		i -= ARRAY_आकार(bltins2);
		क्रम (funcp = G.शीर्ष_func; funcp; funcp = funcp->अग्र) {
			अगर (--i < 0)
				वापस funcp->name;
		}
	}
# endif
	वापस शून्य;
}
#पूर्ण_अगर
#पूर्ण_अगर /* !__U_BOOT__ */

#अगर_अघोषित __U_BOOT__
अटल व्योम remove_nested_vars(व्योम)
{
	काष्ठा variable *cur;
	काष्ठा variable **cur_pp;

	cur_pp = &G.शीर्ष_var;
	जबतक ((cur = *cur_pp) != शून्य) {
		अगर (cur->var_nest_level <= G.var_nest_level) {
			cur_pp = &cur->अग्र;
			जारी;
		}
		/* Unexport */
		अगर (cur->flg_export) {
			debug_printf_env("unexporting nested '%s'/%u\n", cur->varstr, cur->var_nest_level);
			bb_unsetenv(cur->varstr);
		}
		/* Remove from global list */
		*cur_pp = cur->अग्र;
		/* Free */
		अगर (!cur->max_len) {
			debug_printf_env("freeing nested '%s'/%u\n", cur->varstr, cur->var_nest_level);
			मुक्त(cur->varstr);
		}
		मुक्त(cur);
	}
}

अटल व्योम enter_var_nest_level(व्योम)
{
	G.var_nest_level++;
	debug_printf_env("var_nest_level++ %u\n", G.var_nest_level);

	/* Try:	f() { echo -n .; f; }; f
	 * struct variable::var_nest_level is uint16_t,
	 * thus limiting recursion to < 2^16.
	 * In any case, with 8 Mbyte stack SEGV happens
	 * not too long after 2^16 recursions anyway.
	 */
	अगर (G.var_nest_level > 0xff00)
		bb_error_संदेश_and_die("fatal recursion (depth %u)", G.var_nest_level);
}

अटल व्योम leave_var_nest_level(व्योम)
{
	G.var_nest_level--;
	debug_printf_env("var_nest_level-- %u\n", G.var_nest_level);
	अगर (HUSH_DEBUG && (पूर्णांक)G.var_nest_level < 0)
		bb_simple_error_संदेश_and_die("BUG: nesting underflow");

	remove_nested_vars();
}
#पूर्ण_अगर /* __U_BOOT__ */

#अगर ENABLE_HUSH_FUNCTIONS
अटल काष्ठा function **find_function_slot(स्थिर अक्षर *name)
{
	काष्ठा function *funcp;
	काष्ठा function **funcpp = &G.शीर्ष_func;

	जबतक ((funcp = *funcpp) != शून्य) {
		अगर (म_भेद(name, funcp->name) == 0) {
			debug_printf_exec("found function '%s'\n", name);
			अवरोध;
		}
		funcpp = &funcp->अग्र;
	}
	वापस funcpp;
}

अटल ALWAYS_INLINE स्थिर काष्ठा function *find_function(स्थिर अक्षर *name)
{
	स्थिर काष्ठा function *funcp = *find_function_slot(name);
	वापस funcp;
}

/* Note: takes ownership on name ptr */
अटल काष्ठा function *new_function(अक्षर *name)
{
	काष्ठा function **funcpp = find_function_slot(name);
	काष्ठा function *funcp = *funcpp;

	अगर (funcp != शून्य) {
		काष्ठा command *cmd = funcp->parent_cmd;
		debug_printf_exec("func %p parent_cmd %p\n", funcp, cmd);
		अगर (!cmd) {
			debug_printf_exec("freeing & replacing function '%s'\n", funcp->name);
			मुक्त(funcp->name);
			/* Note: if !funcp->body, do not free body_as_string!
			 * This is a special case of "-F name body" function:
			 * body_as_string was not malloced! */
			अगर (funcp->body) {
				free_pipe_list(funcp->body);
# अगर !BB_MMU
				मुक्त(funcp->body_as_string);
# endif
			}
		} अन्यथा {
			debug_printf_exec("reinserting in tree & replacing function '%s'\n", funcp->name);
			cmd->argv[0] = funcp->name;
			cmd->group = funcp->body;
# अगर !BB_MMU
			cmd->group_as_string = funcp->body_as_string;
# endif
		}
	} अन्यथा {
		debug_printf_exec("remembering new function '%s'\n", name);
		funcp = *funcpp = xzalloc(माप(*funcp));
		/*funcp->next = NULL;*/
	}

	funcp->name = name;
	वापस funcp;
}

# अगर ENABLE_HUSH_UNSET
अटल व्योम unset_func(स्थिर अक्षर *name)
{
	काष्ठा function **funcpp = find_function_slot(name);
	काष्ठा function *funcp = *funcpp;

	अगर (funcp != शून्य) {
		debug_printf_exec("freeing function '%s'\n", funcp->name);
		*funcpp = funcp->अग्र;
		/* funcp is unlinked now, deleting it.
		 * Note: if !funcp->body, the function was created by
		 * "-F name body", do not free ->body_as_string
		 * and ->name as they were not malloced. */
		अगर (funcp->body) {
			free_pipe_list(funcp->body);
			मुक्त(funcp->name);
#  अगर !BB_MMU
			मुक्त(funcp->body_as_string);
#  endif
		}
		मुक्त(funcp);
	}
}
# endif

# अगर BB_MMU
#घोषणा exec_function(to_free, funcp, argv) \
	exec_function(funcp, argv)
# endif
अटल व्योम exec_function(अक्षर ***to_free,
		स्थिर काष्ठा function *funcp,
		अक्षर **argv) NORETURN;
अटल व्योम exec_function(अक्षर ***to_free,
		स्थिर काष्ठा function *funcp,
		अक्षर **argv)
{
# अगर BB_MMU
	पूर्णांक n;

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
	_निकास(n);
# अन्यथा
//?	close_saved_fds_and_FILE_fds();

//TODO: check whether "true | func_with_return" works

	re_execute_shell(to_free,
			funcp->body_as_string,
			G.global_argv[0],
			argv + 1,
			शून्य);
# endif
}

अटल पूर्णांक run_function(स्थिर काष्ठा function *funcp, अक्षर **argv)
{
	पूर्णांक rc;
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
# अगर !BB_MMU
	अगर (!funcp->body) {
		/* Function defined by -F */
		parse_and_run_string(funcp->body_as_string);
		rc = G.last_exitcode;
	} अन्यथा
# endif
	{
		rc = run_list(funcp->body);
	}

	IF_HUSH_LOCAL(G.func_nest_level--;)
	IF_HUSH_LOCAL(leave_var_nest_level();)

	G_flag_return_in_progress = sv_flg;
# अगर ENABLE_HUSH_TRAP
	debug_printf_exec("G.return_exitcode=-1\n");
	G.return_exitcode = -1; /* invalidate stashed return value */
# endif

	restore_G_args(&sv, argv);

	वापस rc;
}
#पूर्ण_अगर /* ENABLE_HUSH_FUNCTIONS */

#अगर_अघोषित __U_BOOT__
#अगर BB_MMU
#घोषणा exec_builtin(to_free, x, argv) \
	exec_builtin(x, argv)
#अन्यथा
#घोषणा exec_builtin(to_free, x, argv) \
	exec_builtin(to_free, argv)
#पूर्ण_अगर
अटल व्योम exec_builtin(अक्षर ***to_free,
		स्थिर काष्ठा built_in_command *x,
		अक्षर **argv) NORETURN;
अटल व्योम exec_builtin(अक्षर ***to_free,
		स्थिर काष्ठा built_in_command *x,
		अक्षर **argv)
{
#अगर BB_MMU
	पूर्णांक rcode;
//?	close_saved_fds_and_FILE_fds();
	rcode = x->b_function(argv);
	fflush_all();
	_निकास(rcode);
#अन्यथा
	fflush_all();
	/* On NOMMU, we must never block!
	 * Example: { sleep 99 | read line; } & echo Ok
	 */
	re_execute_shell(to_free,
			argv[0],
			G.global_argv[0],
			G.global_argv + 1,
			argv);
#पूर्ण_अगर
}
#पूर्ण_अगर /* !__U_BOOT__ */

#अगर_अघोषित __U_BOOT__
अटल व्योम execvp_or_die(अक्षर **argv) NORETURN;
अटल व्योम execvp_or_die(अक्षर **argv)
{
	पूर्णांक e;
	debug_printf_exec("execing '%s'\n", argv[0]);
	/* Don't propagate SIG_IGN to the child */
	अगर (SPECIAL_JOBSTOP_SIGS != 0)
		switch_off_special_sigs(G.special_sig_mask & SPECIAL_JOBSTOP_SIGS);
	execvp(argv[0], argv);
	e = 2;
	अगर (त्रुटि_सं == EACCES) e = 126;
	अगर (त्रुटि_सं == ENOENT) e = 127;
	bb_perror_संदेश("can't execute '%s'", argv[0]);
	_निकास(e);
}

#अगर ENABLE_HUSH_MODE_X
अटल व्योम x_mode_print_optionally_squoted(स्थिर अक्षर *str)
{
	अचिन्हित len;
	स्थिर अक्षर *cp;

	cp = str;

	/* the set of chars which-cause-string-to-be-squoted mimics bash */
	/* test a char with: bash -c 'set -x; echo "CH"' */
	अगर (str[म_खोज(str, "\\\"'`$(){}[]<>;#&|~*?!^"
			" " "\001\002\003\004\005\006\007"
			"\010\011\012\013\014\015\016\017"
			"\020\021\022\023\024\025\026\027"
			"\030\031\032\033\034\035\036\037"
			)
		] == '\0'
	) {
		/* string has no special chars */
		x_mode_addstr(str);
		वापस;
	}

	cp = str;
	क्रम (;;) {
		/* print '....' up to EOL or first squote */
		len = (पूर्णांक)(strchrnul(cp, '\'') - cp);
		अगर (len != 0) {
			x_mode_addchr('\'');
			x_mode_addblock(cp, len);
			x_mode_addchr('\'');
			cp += len;
		}
		अगर (*cp == '\0')
			अवरोध;
		/* string contains squote(s), print them as \' */
		x_mode_addchr('\\');
		x_mode_addchr('\'');
		cp++;
	}
}
अटल व्योम dump_cmd_in_x_mode(अक्षर **argv)
{
	अगर (G_x_mode && argv) {
		अचिन्हित n;

		/* "+[+++...][ cmd...]\n\0" */
		x_mode_prefix();
		n = 0;
		जबतक (argv[n]) {
			x_mode_addchr(' ');
			अगर (argv[n][0] == '\0') {
				x_mode_addchr('\'');
				x_mode_addchr('\'');
			} अन्यथा {
				x_mode_print_optionally_squoted(argv[n]);
			}
			n++;
		}
		x_mode_flush();
	}
}
#अन्यथा
# define dump_cmd_in_x_mode(argv) ((व्योम)0)
#पूर्ण_अगर
#पूर्ण_अगर /* !__U_BOOT__ */

#अगर_अघोषित __U_BOOT__
#अगर ENABLE_HUSH_COMMAND
अटल व्योम if_command_vV_print_and_exit(अक्षर opt_vV, अक्षर *cmd, स्थिर अक्षर *explanation)
{
	अक्षर *to_free;

	अगर (!opt_vV)
		वापस;

	to_free = शून्य;
	अगर (!explanation) {
		अक्षर *path = दो_पर्या("PATH");
		explanation = to_free = find_executable(cmd, &path); /* path == NULL is ok */
		अगर (!explanation)
			_निकास(1); /* PROG was not found */
		अगर (opt_vV != 'V')
			cmd = to_free; /* -v PROG prints "/path/to/PROG" */
	}
	म_लिखो((opt_vV == 'V') ? "%s is %s\n" : "%s\n", cmd, explanation);
	मुक्त(to_free);
	fflush_all();
	_निकास(0);
}
#अन्यथा
# define if_command_vV_print_and_exit(a,b,c) ((व्योम)0)
#पूर्ण_अगर
#पूर्ण_अगर /* !__U_BOOT__ */

#अगर BB_MMU
#घोषणा pseudo_exec_argv(nommu_save, argv, assignment_cnt, argv_expanded) \
	pseudo_exec_argv(argv, assignment_cnt, argv_expanded)
#घोषणा pseudo_exec(nommu_save, command, argv_expanded) \
	pseudo_exec(command, argv_expanded)
#पूर्ण_अगर

#अगर_अघोषित __U_BOOT__
/* Called after [v]fork() in run_pipe, or from builtin_exec.
 * Never returns.
 * Don't exit() here.  If you don't exec, use _exit instead.
 * The at_exit handlers apparently confuse the calling process,
 * in particular stdin handling. Not sure why? -- because of vfork! (vda)
 */
अटल व्योम pseudo_exec_argv(nommu_save_t *nommu_save,
		अक्षर **argv, पूर्णांक assignment_cnt,
		अक्षर **argv_expanded) NORETURN;
अटल NOINLINE व्योम pseudo_exec_argv(nommu_save_t *nommu_save,
		अक्षर **argv, पूर्णांक assignment_cnt,
		अक्षर **argv_expanded)
{
	स्थिर काष्ठा built_in_command *x;
	काष्ठा variable **sv_shadowed;
	अक्षर **new_env;
	IF_HUSH_COMMAND(अक्षर opt_vV = 0;)
	IF_HUSH_FUNCTIONS(स्थिर काष्ठा function *funcp;)

	new_env = expand_assignments(argv, assignment_cnt);
	dump_cmd_in_x_mode(new_env);

	अगर (!argv[assignment_cnt]) {
		/* Case when we are here: ... | var=val | ...
		 * (note that we do not exit early, i.e., do not optimize out
		 * expand_assignments(): think about ... | var=`sleep 1` | ...
		 */
		free_strings(new_env);
		_exit_SUCCESS();
	}

	sv_shadowed = G.shadowed_vars_pp;
#अगर BB_MMU
	G.shadowed_vars_pp = शून्य; /* "don't save, free them instead" */
#अन्यथा
	G.shadowed_vars_pp = &nommu_save->old_vars;
	G.var_nest_level++;
#पूर्ण_अगर
	set_vars_and_save_old(new_env);
	G.shadowed_vars_pp = sv_shadowed;

	अगर (argv_expanded) {
		argv = argv_expanded;
	} अन्यथा {
		argv = expand_strvec_to_strvec(argv + assignment_cnt);
#अगर !BB_MMU
		nommu_save->argv = argv;
#पूर्ण_अगर
	}
	dump_cmd_in_x_mode(argv);

#अगर ENABLE_FEATURE_SH_STANDALONE || BB_MMU
	अगर (म_अक्षर(argv[0], '/') != शून्य)
		जाओ skip;
#पूर्ण_अगर

#अगर ENABLE_HUSH_FUNCTIONS
	/* Check if the command matches any functions (this goes before bltins) */
	funcp = find_function(argv[0]);
	अगर (funcp)
		exec_function(&nommu_save->argv_from_re_execing, funcp, argv);
#पूर्ण_अगर

#अगर ENABLE_HUSH_COMMAND
	/* "command BAR": run BAR without looking it up among functions
	 * "command -v BAR": print "BAR" or "/path/to/BAR"; or exit 1
	 * "command -V BAR": print "BAR is {a function,a shell builtin,/path/to/BAR}"
	 */
	जबतक (म_भेद(argv[0], "command") == 0 && argv[1]) {
		अक्षर *p;

		argv++;
		p = *argv;
		अगर (p[0] != '-' || !p[1])
			जारी; /* bash allows "command command command [-OPT] BAR" */

		क्रम (;;) {
			p++;
			चयन (*p) {
			हाल '\0':
				argv++;
				p = *argv;
				अगर (p[0] != '-' || !p[1])
					जाओ after_opts;
				जारी; /* next arg is also -opts, process it too */
			हाल 'v':
			हाल 'V':
				opt_vV = *p;
				जारी;
			शेष:
				bb_error_संदेश_and_die("%s: %s: invalid option", "command", argv[0]);
			}
		}
	}
 after_opts:
# अगर ENABLE_HUSH_FUNCTIONS
	अगर (opt_vV && find_function(argv[0]))
		if_command_vV_print_and_exit(opt_vV, argv[0], "a function");
# endif
#पूर्ण_अगर

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
	अगर (x) {
		if_command_vV_print_and_exit(opt_vV, argv[0], "a shell builtin");
		exec_builtin(&nommu_save->argv_from_re_execing, x, argv);
	}

#अगर ENABLE_FEATURE_SH_STANDALONE
	/* Check if the command matches any busybox applets */
	{
		पूर्णांक a = find_applet_by_name(argv[0]);
		अगर (a >= 0) {
			if_command_vV_print_and_exit(opt_vV, argv[0], "an applet");
# अगर BB_MMU /* see above why on NOMMU it is not allowed */
			अगर (APPLET_IS_NOEXEC(a)) {
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
			अगर (SPECIAL_JOBSTOP_SIGS != 0)
				switch_off_special_sigs(G.special_sig_mask & SPECIAL_JOBSTOP_SIGS);
			execv(bb_busybox_exec_path, argv);
			/* If they called chroot or otherwise made the binary no longer
			 * executable, fall through */
		}
	}
#पूर्ण_अगर

#अगर ENABLE_FEATURE_SH_STANDALONE || BB_MMU
 skip:
#पूर्ण_अगर
	if_command_vV_print_and_exit(opt_vV, argv[0], शून्य);
	execvp_or_die(argv);
}

/* Called after [v]fork() in run_pipe
 */
अटल व्योम pseudo_exec(nommu_save_t *nommu_save,
		काष्ठा command *command,
		अक्षर **argv_expanded) NORETURN;
अटल व्योम pseudo_exec(nommu_save_t *nommu_save,
		काष्ठा command *command,
		अक्षर **argv_expanded)
{
#अगर ENABLE_HUSH_FUNCTIONS
	अगर (command->cmd_type == CMD_FUNCDEF) {
		/* Ignore funcdefs in pipes:
		 * true | f() { cmd }
		 */
		_निकास(0);
	}
#पूर्ण_अगर

	अगर (command->argv) {
		pseudo_exec_argv(nommu_save, command->argv,
				command->assignment_cnt, argv_expanded);
	}

	अगर (command->group) {
		/* Cases when we are here:
		 * ( list )
		 * { list } &
		 * ... | ( list ) | ...
		 * ... | { list } | ...
		 */
#अगर BB_MMU
		पूर्णांक rcode;
		debug_printf_exec("pseudo_exec: run_list\n");
		reset_traps_to_defaults();
		rcode = run_list(command->group);
		/* OK to leak memory by not calling free_pipe_list,
		 * since this process is about to exit */
		_निकास(rcode);
#अन्यथा
		re_execute_shell(&nommu_save->argv_from_re_execing,
				command->group_as_string,
				G.global_argv[0],
				G.global_argv + 1,
				शून्य);
#पूर्ण_अगर
	}

	/* Case when we are here: ... | >file */
	debug_printf_exec("pseudo_exec'ed null command\n");
	_exit_SUCCESS();
}

#अगर ENABLE_HUSH_JOB
अटल स्थिर अक्षर *get_cmdtext(काष्ठा pipe *pi)
{
	अक्षर **argv;
	अक्षर *p;
	पूर्णांक len;

	/* This is subtle. ->cmdtext is created only on first backgrounding.
	 * (Think "cat, <ctrl-z>, fg, <ctrl-z>, fg, <ctrl-z>...." here...)
	 * On subsequent bg argv is trashed, but we won't use it */
	अगर (pi->cmdtext)
		वापस pi->cmdtext;

	argv = pi->cmds[0].argv;
	अगर (!argv) {
		pi->cmdtext = xzalloc(1);
		वापस pi->cmdtext;
	}
	len = 0;
	करो {
		len += म_माप(*argv) + 1;
	} जबतक (*++argv);
	p = xmalloc(len);
	pi->cmdtext = p;
	argv = pi->cmds[0].argv;
	करो {
		p = stpcpy(p, *argv);
		*p++ = ' ';
	} जबतक (*++argv);
	p[-1] = '\0';
	वापस pi->cmdtext;
}

अटल व्योम remove_job_from_table(काष्ठा pipe *pi)
{
	काष्ठा pipe *पूर्व_pipe;

	अगर (pi == G.job_list) {
		G.job_list = pi->अग्र;
	} अन्यथा {
		पूर्व_pipe = G.job_list;
		जबतक (पूर्व_pipe->अग्र != pi)
			पूर्व_pipe = पूर्व_pipe->अग्र;
		पूर्व_pipe->अग्र = pi->अग्र;
	}
	G.last_jobid = 0;
	अगर (G.job_list)
		G.last_jobid = G.job_list->jobid;
}

अटल व्योम delete_finished_job(काष्ठा pipe *pi)
{
	remove_job_from_table(pi);
	free_pipe(pi);
}

अटल व्योम clean_up_last_dead_job(व्योम)
{
	अगर (G.job_list && !G.job_list->alive_cmds)
		delete_finished_job(G.job_list);
}

अटल व्योम जोड़ो_job_into_table(काष्ठा pipe *pi)
{
	काष्ठा pipe *job, **jobp;
	पूर्णांक i;

	clean_up_last_dead_job();

	/* Find the end of the list, and find next job ID to use */
	i = 0;
	jobp = &G.job_list;
	जबतक ((job = *jobp) != शून्य) {
		अगर (job->jobid > i)
			i = job->jobid;
		jobp = &job->अग्र;
	}
	pi->jobid = i + 1;

	/* Create a new job struct at the end */
	job = *jobp = xmemdup(pi, माप(*pi));
	job->अग्र = शून्य;
	job->cmds = xzalloc(माप(pi->cmds[0]) * pi->अंक_cmds);
	/* Cannot copy entire pi->cmds[] vector! This causes double frees */
	क्रम (i = 0; i < pi->अंक_cmds; i++) {
		job->cmds[i].pid = pi->cmds[i].pid;
		/* all other fields are not used and stay zero */
	}
	job->cmdtext = xstrdup(get_cmdtext(pi));

	अगर (G_interactive_fd)
		म_लिखो("[%u] %u %s\n", job->jobid, (अचिन्हित)job->cmds[0].pid, job->cmdtext);
	G.last_jobid = job->jobid;
}
#पूर्ण_अगर /* JOB */

अटल पूर्णांक job_exited_or_stopped(काष्ठा pipe *pi)
{
	पूर्णांक rcode, i;

	अगर (pi->alive_cmds != pi->stopped_cmds)
		वापस -1;

	/* All processes in fg pipe have exited or stopped */
	rcode = 0;
	i = pi->अंक_cmds;
	जबतक (--i >= 0) {
		rcode = pi->cmds[i].cmd_exitcode;
		/* usually last process gives overall exitstatus,
		 * but with "set -o pipefail", last *failed* process does */
		अगर (G.o_opt[OPT_O_PIPEFAIL] == 0 || rcode != 0)
			अवरोध;
	}
	IF_HAS_KEYWORDS(अगर (pi->pi_inverted) rcode = !rcode;)
	वापस rcode;
}

अटल पूर्णांक process_wait_परिणाम(काष्ठा pipe *fg_pipe, pid_t childpid, पूर्णांक status)
{
#अगर ENABLE_HUSH_JOB
	काष्ठा pipe *pi;
#पूर्ण_अगर
	पूर्णांक i, dead;

	dead = WIFEXITED(status) || WIFSIGNALED(status);

#अगर DEBUG_JOBS
	अगर (WIFSTOPPED(status))
		debug_printf_jobs("pid %d stopped by sig %d (exitcode %d)\n",
				childpid, WSTOPSIG(status), WEXITSTATUS(status));
	अगर (WIFSIGNALED(status))
		debug_printf_jobs("pid %d killed by sig %d (exitcode %d)\n",
				childpid, WTERMSIG(status), WEXITSTATUS(status));
	अगर (WIFEXITED(status))
		debug_printf_jobs("pid %d exited, exitcode %d\n",
				childpid, WEXITSTATUS(status));
#पूर्ण_अगर
	/* Were we asked to wait for a fg pipe? */
	अगर (fg_pipe) {
		i = fg_pipe->अंक_cmds;

		जबतक (--i >= 0) {
			पूर्णांक rcode;

			debug_printf_jobs("check pid %d\n", fg_pipe->cmds[i].pid);
			अगर (fg_pipe->cmds[i].pid != childpid)
				जारी;
			अगर (dead) {
				पूर्णांक ex;
				fg_pipe->cmds[i].pid = 0;
				fg_pipe->alive_cmds--;
				ex = WEXITSTATUS(status);
				/* bash prints killer signal's name for *last*
				 * process in pipe (prints just newline for SIGINT/SIGPIPE).
				 * Mimic this. Example: "sleep 5" + (^\ or kill -QUIT)
				 */
				अगर (WIFSIGNALED(status)) {
					पूर्णांक sig = WTERMSIG(status);
#अगर ENABLE_HUSH_JOB
					अगर (G.run_list_level == 1
					/* ^^^^^ Do not print in nested contexts, example:
					 * echo `sleep 1; sh -c 'kill -9 $$'` - prints "137", NOT "Killed 137"
					 */
					 && i == fg_pipe->अंक_cmds-1
					) {
						/* strsignal() is for bash compat. ~600 bloat versus bbox's get_signame() */
						माला_दो(sig == संक_विघ्न || sig == SIGPIPE ? "" : strsignal(sig));
					}
#पूर्ण_अगर
					/* TODO: if (WCOREDUMP(status)) + " (core dumped)"; */
					/* MIPS has 128 sigs (1..128), if sig==128,
					 * 128 + sig would result in exitcode 256 -> 0!
					 */
					ex = 128 | sig;
				}
				fg_pipe->cmds[i].cmd_exitcode = ex;
			} अन्यथा {
				fg_pipe->stopped_cmds++;
			}
			debug_printf_jobs("fg_pipe: alive_cmds %d stopped_cmds %d\n",
					fg_pipe->alive_cmds, fg_pipe->stopped_cmds);
			rcode = job_exited_or_stopped(fg_pipe);
			अगर (rcode >= 0) {
/* Note: *non-interactive* bash does not continue if all processes in fg pipe
 * are stopped. Testcase: "cat | cat" in a script (not on command line!)
 * and "killall -STOP cat" */
				अगर (G_interactive_fd) {
#अगर ENABLE_HUSH_JOB
					अगर (fg_pipe->alive_cmds != 0)
						जोड़ो_job_into_table(fg_pipe);
#पूर्ण_अगर
					वापस rcode;
				}
				अगर (fg_pipe->alive_cmds == 0)
					वापस rcode;
			}
			/* There are still running processes in the fg_pipe */
			वापस -1;
		}
		/* It wasn't in fg_pipe, look for process in bg pipes */
	}

#अगर ENABLE_HUSH_JOB
	/* We were asked to wait for bg or orphaned children */
	/* No need to remember exitcode in this case */
	क्रम (pi = G.job_list; pi; pi = pi->अग्र) {
		क्रम (i = 0; i < pi->अंक_cmds; i++) {
			अगर (pi->cmds[i].pid == childpid)
				जाओ found_pi_and_prognum;
		}
	}
	/* Happens when shell is used as init process (init=/bin/sh) */
	debug_printf("checkjobs: pid %d was not in our list!\n", childpid);
	वापस -1; /* this wasn't a process from fg_pipe */

 found_pi_and_prognum:
	अगर (dead) {
		/* child exited */
		पूर्णांक rcode = WEXITSTATUS(status);
		अगर (WIFSIGNALED(status))
			/* NB: not 128 + sig, MIPS has sig 128 */
			rcode = 128 | WTERMSIG(status);
		pi->cmds[i].cmd_exitcode = rcode;
		अगर (G.last_bg_pid == pi->cmds[i].pid)
			G.last_bg_pid_exitcode = rcode;
		pi->cmds[i].pid = 0;
		pi->alive_cmds--;
		अगर (!pi->alive_cmds) {
# अगर ENABLE_HUSH_BASH_COMPAT
			G.dead_job_exitcode = job_exited_or_stopped(pi);
# endif
			अगर (G_interactive_fd) {
				म_लिखो(JOB_STATUS_FORMAT, pi->jobid,
						"Done", pi->cmdtext);
				delete_finished_job(pi);
			} अन्यथा {
/*
 * bash deletes finished jobs from job table only in interactive mode,
 * after "jobs" cmd, or if pid of a new process matches one of the old ones
 * (see cleanup_dead_jobs(), delete_old_job(), J_NOTIFIED in bash source).
 * Testcase script: "(exit 3) & sleep 1; wait %1; echo $?" prints 3 in bash.
 * We only retain one "dead" job, if it's the single job on the list.
 * This covers most of real-world scenarios where this is useful.
 */
				अगर (pi != G.job_list)
					delete_finished_job(pi);
			}
		}
	} अन्यथा {
		/* child stopped */
		pi->stopped_cmds++;
	}
#पूर्ण_अगर
	वापस -1; /* this wasn't a process from fg_pipe */
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
अटल पूर्णांक checkjobs(काष्ठा pipe *fg_pipe, pid_t waitfor_pid)
{
	पूर्णांक attributes;
	पूर्णांक status;
	पूर्णांक rcode = 0;

	debug_printf_jobs("checkjobs %p\n", fg_pipe);

	attributes = WUNTRACED;
	अगर (fg_pipe == शून्य)
		attributes |= WNOHANG;

	त्रुटि_सं = 0;
#अगर ENABLE_HUSH_FAST
	अगर (G.handled_SIGCHLD == G.गिनती_SIGCHLD) {
//bb_error_msg("[%d] checkjobs: G.count_SIGCHLD:%d G.handled_SIGCHLD:%d children?:%d fg_pipe:%p",
//getpid(), G.count_SIGCHLD, G.handled_SIGCHLD, G.we_have_children, fg_pipe);
		/* There was neither fork nor SIGCHLD since last waitpid */
		/* Avoid doing waitpid syscall if possible */
		अगर (!G.we_have_children) {
			त्रुटि_सं = ECHILD;
			वापस -1;
		}
		अगर (fg_pipe == शून्य) { /* is WNOHANG set? */
			/* We have children, but they did not exit
			 * or stop yet (we saw no SIGCHLD) */
			वापस 0;
		}
		/* else: !WNOHANG, waitpid will block, can't short-circuit */
	}
#पूर्ण_अगर

/* Do we do this right?
 * bash-3.00# sleep 20 | false
 * <ctrl-Z pressed>
 * [3]+  Stopped          sleep 20 | false
 * bash-3.00# echo $?
 * 1   <========== bg pipe is not fully done, but exitcode is already known!
 * [hush 1.14.0: yes we do it right]
 */
	जबतक (1) {
		pid_t childpid;
#अगर ENABLE_HUSH_FAST
		पूर्णांक i;
		i = G.गिनती_SIGCHLD;
#पूर्ण_अगर
		childpid = waitpid(-1, &status, attributes);
		अगर (childpid <= 0) {
			अगर (childpid && त्रुटि_सं != ECHILD)
				bb_simple_perror_संदेश("waitpid");
#अगर ENABLE_HUSH_FAST
			अन्यथा { /* Until next SIGCHLD, waitpid's are useless */
				G.we_have_children = (childpid == 0);
				G.handled_SIGCHLD = i;
//bb_error_msg("[%d] checkjobs: waitpid returned <= 0, G.count_SIGCHLD:%d G.handled_SIGCHLD:%d", getpid(), G.count_SIGCHLD, G.handled_SIGCHLD);
			}
#पूर्ण_अगर
			/* ECHILD (no children), or 0 (no change in children status) */
			rcode = childpid;
			अवरोध;
		}
		rcode = process_wait_परिणाम(fg_pipe, childpid, status);
		अगर (rcode >= 0) {
			/* fg_pipe exited or stopped */
			अवरोध;
		}
		अगर (childpid == waitfor_pid) { /* "wait PID" */
			debug_printf_exec("childpid==waitfor_pid:%d status:0x%08x\n", childpid, status);
			rcode = WEXITSTATUS(status);
			अगर (WIFSIGNALED(status))
				rcode = 128 | WTERMSIG(status);
			अगर (WIFSTOPPED(status))
				/* bash: "cmd & wait $!" and cmd stops: $? = 128 | stopsig */
				rcode = 128 | WSTOPSIG(status);
			rcode++;
			अवरोध; /* "wait PID" called us, give it exitcode+1 */
		}
#अगर ENABLE_HUSH_BASH_COMPAT
		अगर (-1 == waitfor_pid /* "wait -n" (wait for any one job) */
		 && G.dead_job_exitcode >= 0 /* some job did finish */
		) {
			debug_printf_exec("waitfor_pid:-1\n");
			rcode = G.dead_job_exitcode + 1;
			अवरोध;
		}
#पूर्ण_अगर
		/* This wasn't one of our processes, or */
		/* fg_pipe still has running processes, do waitpid again */
	} /* while (waitpid succeeds)... */

	वापस rcode;
}

#अगर ENABLE_HUSH_JOB
अटल पूर्णांक checkjobs_and_fg_shell(काष्ठा pipe *fg_pipe)
{
	pid_t p;
	पूर्णांक rcode = checkjobs(fg_pipe, 0 /*(no pid to wait for)*/);
	अगर (G_saved_tty_pgrp) {
		/* Job finished, move the shell to the foreground */
		p = getpgrp(); /* our process group id */
		debug_printf_jobs("fg'ing ourself: getpgrp()=%d\n", (पूर्णांक)p);
		tcsetpgrp(G_interactive_fd, p);
	}
	वापस rcode;
}
#पूर्ण_अगर
#पूर्ण_अगर /* !__U_BOOT__ */

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
अटल व्योम set_G_ifs(व्योम)
{
	/* Testcase: set -- q w e; (IFS='' echo "$*"; IFS=''; echo "$*"); echo "$*"
	 * Result should be 3 lines: q w e, qwe, q w e
	 */
	अगर (G.ifs_whitespace != G.ifs)
		मुक्त(G.ifs_whitespace);
	G.ifs = get_local_var_मूल्य("IFS");
	अगर (G.ifs) {
		अक्षर *p;
		G.ifs_whitespace = (अक्षर*)G.ifs;
		p = skip_whitespace(G.ifs);
		अगर (*p) {
			/* Not all $IFS is whitespace */
			अक्षर *d;
			पूर्णांक len = p - G.ifs;
			p = skip_non_whitespace(p);
			G.ifs_whitespace = xmalloc(len + म_माप(p) + 1); /* can overestimate */
			d = mempcpy(G.ifs_whitespace, G.ifs, len);
			जबतक (*p) {
				अगर (है_खाली(*p))
					*d++ = *p;
				p++;
			}
			*d = '\0';
		}
	} अन्यथा {
		G.ifs = defifs;
		G.ifs_whitespace = (अक्षर*)G.ifs;
	}
}
#अगर_अघोषित __U_BOOT__
#अगर !ENABLE_HUSH_MODE_X
#घोषणा redirect_and_varexp_helper(command, sqp, argv_expanded) \
	redirect_and_varexp_helper(command, sqp)
#पूर्ण_अगर
अटल पूर्णांक redirect_and_varexp_helper(
		काष्ठा command *command,
		काष्ठा squirrel **sqp,
		अक्षर **argv_expanded)
{
	/* Assignments occur before redirects. Try:
	 * a=`sleep 1` sleep 2 3>/qwe/rty
	 */

	अक्षर **new_env = expand_assignments(command->argv, command->assignment_cnt);
	dump_cmd_in_x_mode(new_env);
	dump_cmd_in_x_mode(argv_expanded);
	/* this takes ownership of new_env[i] elements, and frees new_env: */
	set_vars_and_save_old(new_env);

	वापस setup_redirects(command, sqp);
}
#पूर्ण_अगर /* !__U_BOOT__ */

अटल NOINLINE पूर्णांक run_pipe(काष्ठा pipe *pi)
{
	अटल स्थिर अक्षर *स्थिर null_ptr = शून्य;

	पूर्णांक cmd_no;
#अगर_अघोषित __U_BOOT__
	पूर्णांक अग्र_infd;
#पूर्ण_अगर /* !__U_BOOT__ */
	काष्ठा command *command;
	अक्षर **argv_expanded;
	अक्षर **argv;
#अगर_अघोषित __U_BOOT__
	काष्ठा squirrel *squirrel = शून्य;
#पूर्ण_अगर /* !__U_BOOT__ */
	पूर्णांक rcode;

#अगर_घोषित __U_BOOT__
	/*
	 * Set rcode here to avoid returning a garbage value in the middle of
	 * the function.
	 * Also, if an error occurs, rcode value would be changed and last
	 * return will signal the error.
	 */
	rcode = 0;
#पूर्ण_अगर /* __U_BOOT__ */

	debug_printf_exec("run_pipe start: members:%d\n", pi->अंक_cmds);
	debug_enter();

	set_G_ifs();

#अगर_अघोषित __U_BOOT__
	IF_HUSH_JOB(pi->pgrp = -1;)
	pi->stopped_cmds = 0;
#पूर्ण_अगर /* !__U_BOOT__ */
	command = &pi->cmds[0];
	argv_expanded = शून्य;

#अगर_अघोषित __U_BOOT__
	अगर (pi->अंक_cmds != 1
	 || pi->followup == PIPE_BG
	 || command->cmd_type == CMD_SUBSHELL
	) {
		जाओ must_fork;
	}

	pi->alive_cmds = 1;
#पूर्ण_अगर /* !__U_BOOT__ */

	debug_printf_exec(": group:%p argv:'%s'\n",
		command->group, command->argv ? command->argv[0] : "NONE");

	अगर (command->group) {
#अगर ENABLE_HUSH_FUNCTIONS
		अगर (command->cmd_type == CMD_FUNCDEF) {
			/* "executing" func () { list } */
			काष्ठा function *funcp;

			funcp = new_function(command->argv[0]);
			/* funcp->name is already set to argv[0] */
			funcp->body = command->group;
# अगर !BB_MMU
			funcp->body_as_string = command->group_as_string;
			command->group_as_string = शून्य;
# endif
			command->group = शून्य;
			command->argv[0] = शून्य;
			debug_printf_exec("cmd %p has child func at %p\n", command, funcp);
			funcp->parent_cmd = command;
			command->child_func = funcp;

			debug_printf_exec("run_pipe: return EXIT_SUCCESS\n");
			debug_leave();
			वापस निकास_सफल;
		}
#पूर्ण_अगर
		/* { list } */
		debug_printf_exec("non-subshell group\n");
		rcode = 1; /* exitcode if redir failed */
#अगर_अघोषित __U_BOOT__
		अगर (setup_redirects(command, &squirrel) == 0) {
#पूर्ण_अगर /* !__U_BOOT__ */
			debug_printf_exec(": run_list\n");
//FIXME: we need to pass squirrel down into run_list()
//for SH_STANDALONE case, or else this construct:
// { find /proc/self/fd; true; } >FILE; cmd2
//has no way of closing saved fd#1 for "find",
//and in SH_STANDALONE mode, "find" is not execed,
//therefore CLOEXEC on saved fd does not help.
			rcode = run_list(command->group) & 0xff;
#अगर_अघोषित __U_BOOT__
		}
		restore_redirects(squirrel);
#पूर्ण_अगर /* !__U_BOOT__ */
		IF_HAS_KEYWORDS(अगर (pi->pi_inverted) rcode = !rcode;)
		debug_leave();
		debug_printf_exec("run_pipe: return %d\n", rcode);
		वापस rcode;
	}

	argv = command->argv ? command->argv : (अक्षर **) &null_ptr;
	{
#अगर_अघोषित __U_BOOT__
		स्थिर काष्ठा built_in_command *x;
		IF_HUSH_FUNCTIONS(स्थिर काष्ठा function *funcp;)
		IF_NOT_HUSH_FUNCTIONS(क्रमागत { funcp = 0 };)
		काष्ठा variable **sv_shadowed;
#पूर्ण_अगर /* !__U_BOOT__ */
		काष्ठा variable *old_vars;

#अगर ENABLE_HUSH_LINENO_VAR
		G.execute_lineno = command->lineno;
#पूर्ण_अगर

		अगर (argv[command->assignment_cnt] == शून्य) {
			/* Assignments, but no command.
			 * Ensure redirects take effect (that is, create files).
			 * Try "a=t >file"
			 */
			अचिन्हित i;
			G.expand_exitcode = 0;
 only_assignments:
#अगर_अघोषित __U_BOOT__
			rcode = setup_redirects(command, &squirrel);
			restore_redirects(squirrel);
#पूर्ण_अगर /* !__U_BOOT__ */

			/* Set shell variables */
			i = 0;
			जबतक (i < command->assignment_cnt) {
				अक्षर *p = expand_string_to_string(argv[i],
						EXP_FLAG_ESC_GLOB_CHARS,
						/*unbackslash:*/ 1
				);
#अगर ENABLE_HUSH_MODE_X
				अगर (G_x_mode) {
					अक्षर *eq;
					अगर (i == 0)
						x_mode_prefix();
					x_mode_addchr(' ');
					eq = strchrnul(p, '=');
					अगर (*eq) eq++;
					x_mode_addblock(p, (eq - p));
					x_mode_print_optionally_squoted(eq);
					x_mode_flush();
				}
#पूर्ण_अगर
				debug_printf_env("set shell var:'%s'->'%s'\n", *argv, p);
#अगर_अघोषित __U_BOOT__
				अगर (set_local_var0(p)) {
#अन्यथा /* __U_BOOT__ */
				अगर (set_local_var_modern(p, /*flag:*/ 0)) {
#पूर्ण_अगर
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
			अगर (rcode == 0)
				rcode = G.expand_exitcode;
			IF_HAS_KEYWORDS(अगर (pi->pi_inverted) rcode = !rcode;)
			debug_leave();
			debug_printf_exec("run_pipe: return %d\n", rcode);
			वापस rcode;
		}

		/* Expand the rest into (possibly) many strings each */
#अगर defined(CMD_TEST2_SINGLEWORD_NOGLOB)
		अगर (command->cmd_type == CMD_TEST2_SINGLEWORD_NOGLOB)
			argv_expanded = expand_strvec_to_strvec_singleword_noglob(argv + command->assignment_cnt);
		अन्यथा
#पूर्ण_अगर
#अगर defined(CMD_SINGLEWORD_NOGLOB)
		अगर (command->cmd_type == CMD_SINGLEWORD_NOGLOB)
			argv_expanded = expand_strvec_to_strvec_singleword_noglob(argv + command->assignment_cnt);
		अन्यथा
#पूर्ण_अगर
			argv_expanded = expand_strvec_to_strvec(argv + command->assignment_cnt);

		/* If someone gives us an empty string: `cmd with empty output` */
		अगर (!argv_expanded[0]) {
			मुक्त(argv_expanded);
			/* `false` still has to set exitcode 1 */
			G.expand_exitcode = G.last_exitcode;
			जाओ only_assignments;
		}

		old_vars = शून्य;
#अगर_अघोषित __U_BOOT__
		sv_shadowed = G.shadowed_vars_pp;

		/* Check if argv[0] matches any functions (this goes before bltins) */
		IF_HUSH_FUNCTIONS(funcp = find_function(argv_expanded[0]);)
		IF_HUSH_FUNCTIONS(x = शून्य;)
		IF_HUSH_FUNCTIONS(अगर (!funcp))
			x = find_builtin(argv_expanded[0]);
		अगर (x || funcp) {
			अगर (x && x->b_function == builtin_exec && argv_expanded[1] == शून्य) {
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

				जाओ clean_up_and_ret1;
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
			अगर (rcode == 0) {
				अगर (!funcp) {
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
#अगर ENABLE_HUSH_FUNCTIONS
				अन्यथा {
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
#पूर्ण_अगर
			}
		} अन्यथा
		अगर (ENABLE_FEATURE_SH_NOFORK && अंक_APPLETS > 1) {
			पूर्णांक n = find_applet_by_name(argv_expanded[0]);
			अगर (n < 0 || !APPLET_IS_NOFORK(n))
				जाओ must_fork;

			enter_var_nest_level();
			/* Collect all variables "shadowed" by helper into old_vars list */
			G.shadowed_vars_pp = &old_vars;
			rcode = redirect_and_varexp_helper(command, &squirrel, argv_expanded);
			G.shadowed_vars_pp = sv_shadowed;

			अगर (rcode == 0) {
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
		} अन्यथा
			जाओ must_fork;

		restore_redirects(squirrel);
 clean_up_and_ret1:
		leave_var_nest_level();
		add_vars(old_vars);

		/*
		 * Try "usleep 99999999" + ^C + "echo $?"
		 * with FEATURE_SH_NOFORK=y.
		 */
		अगर (!funcp) {
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
			अगर (sigismember(&G.pending_set, संक_विघ्न))
				rcode = 128 | संक_विघ्न;
		}
		मुक्त(argv_expanded);
		IF_HAS_KEYWORDS(अगर (pi->pi_inverted) rcode = !rcode;)
		debug_leave();
		debug_printf_exec("run_pipe return %d\n", rcode);
		वापस rcode;
#पूर्ण_अगर /* !__U_BOOT__ */
	}

#अगर_अघोषित __U_BOOT__
 must_fork:
	/* NB: argv_expanded may already be created, and that
	 * might include `cmd` runs! Do not rerun it! We *must*
	 * use argv_expanded if it's non-NULL */

	/* Going to fork a child per each pipe member */
	pi->alive_cmds = 0;
	अग्र_infd = 0;
#पूर्ण_अगर /* !__U_BOOT__ */

	cmd_no = 0;
	जबतक (cmd_no < pi->अंक_cmds) {
#अगर_अघोषित __U_BOOT__
		काष्ठा fd_pair pipefds;
#अगर !BB_MMU
		पूर्णांक sv_var_nest_level = G.var_nest_level;
		अस्थिर nommu_save_t nommu_save;
		nommu_save.old_vars = शून्य;
		nommu_save.argv = शून्य;
		nommu_save.argv_from_re_execing = शून्य;
#पूर्ण_अगर
#पूर्ण_अगर /* !__U_BOOT__ */
		command = &pi->cmds[cmd_no];
		cmd_no++;

#अगर_घोषित __U_BOOT__
		/* Replace argv and argc by expanded if it exists. */
		अगर (argv_expanded) {
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
			command->argc = list_आकार(argv_expanded);
			command->argv = argv_expanded;
		}
#पूर्ण_अगर /* __U_BOOT__ */
			अगर (command->argv) {
			debug_printf_exec(": pipe member '%s' '%s'...\n",
					command->argv[0], command->argv[1]);
		} अन्यथा {
			debug_printf_exec(": pipe member with no argv\n");
		}

#अगर_अघोषित __U_BOOT__
		/* pipes are inserted between pairs of commands */
		pipefds.rd = 0;
		pipefds.wr = 1;
		अगर (cmd_no < pi->अंक_cmds)
			xpiped_pair(pipefds);

#अगर ENABLE_HUSH_LINENO_VAR
		G.execute_lineno = command->lineno;
#पूर्ण_अगर

		command->pid = BB_MMU ? विभाजन() : vfork();
		अगर (!command->pid) { /* child */
#अगर ENABLE_HUSH_JOB
			disable_restore_tty_pgrp_on_exit();
			CLEAR_RANDOM_T(&G.random_gen); /* or else $RANDOM repeats in child */

			/* Every child adds itself to new process group
			 * with pgid == pid_of_first_child_in_pipe */
			अगर (G.run_list_level == 1 && G_interactive_fd) {
				pid_t pgrp;
				pgrp = pi->pgrp;
				अगर (pgrp < 0) /* true for 1st process only */
					pgrp = getpid();
				अगर (setpgid(0, pgrp) == 0
				 && pi->followup != PIPE_BG
				 && G_saved_tty_pgrp /* we have ctty */
				) {
					/* We do it in *every* child, not just first,
					 * to avoid races */
					tcsetpgrp(G_interactive_fd, pgrp);
				}
			}
#पूर्ण_अगर
			अगर (pi->alive_cmds == 0 && pi->followup == PIPE_BG) {
				/* 1st cmd in backgrounded pipe
				 * should have its stdin /dev/null'ed */
				बंद(0);
				अगर (खोलो(bb_dev_null, O_RDONLY))
					xopen("/", O_RDONLY);
			} अन्यथा {
				xmove_fd(अग्र_infd, 0);
			}
			xmove_fd(pipefds.wr, 1);
			अगर (pipefds.rd > 1)
				बंद(pipefds.rd);
			/* Like bash, explicit redirects override pipes,
			 * and the pipe fd (fd#1) is available for dup'ing:
			 * "cmd1 2>&1 | cmd2": fd#1 is duped to fd#2, thus stderr
			 * of cmd1 goes into pipe.
			 */
			अगर (setup_redirects(command, शून्य)) {
				/* Happens when redir file can't be opened:
				 * $ hush -c 'echo FOO >&2 | echo BAR 3>/qwe/rty; echo BAZ'
				 * FOO
				 * hush: can't open '/qwe/rty': No such file or directory
				 * BAZ
				 * (echo BAR is not executed, it hits _exit(1) below)
				 */
				_निकास(1);
			}

			/* Stores to nommu_save list of env vars putenv'ed
			 * (NOMMU, on MMU we don't need that) */
			/* cast away volatility... */
			pseudo_exec((nommu_save_t*) &nommu_save, command, argv_expanded);
			/* pseudo_exec() does not return */
		}

		/* parent or error */
#अगर ENABLE_HUSH_FAST
		G.गिनती_SIGCHLD++;
//bb_error_msg("[%d] fork in run_pipe: G.count_SIGCHLD:%d G.handled_SIGCHLD:%d", getpid(), G.count_SIGCHLD, G.handled_SIGCHLD);
#पूर्ण_अगर
		enable_restore_tty_pgrp_on_exit();
#अगर !BB_MMU
		/* Clean up after vforked child */
		मुक्त(nommu_save.argv);
		मुक्त(nommu_save.argv_from_re_execing);
		G.var_nest_level = sv_var_nest_level;
		remove_nested_vars();
		add_vars(nommu_save.old_vars);
#पूर्ण_अगर
		मुक्त(argv_expanded);
		argv_expanded = शून्य;
		अगर (command->pid < 0) { /* [v]fork failed */
			/* Clearly indicate, was it fork or vfork */
			bb_simple_perror_संदेश(BB_MMU ? "vfork"+1 : "vfork");
		} अन्यथा {
			pi->alive_cmds++;
#अगर ENABLE_HUSH_JOB
			/* Second and next children need to know pid of first one */
			अगर (pi->pgrp < 0)
				pi->pgrp = command->pid;
#पूर्ण_अगर
		}

		अगर (cmd_no > 1)
			बंद(अग्र_infd);
		अगर (cmd_no < pi->अंक_cmds)
			बंद(pipefds.wr);
		/* Pass read (output) pipe end to next iteration */
		अग्र_infd = pipefds.rd;
#अन्यथा /* __U_BOOT__ */
		/* Process the command */
		rcode = cmd_process(G.do_repeat ? CMD_FLAG_REPEAT : 0,
				    command->argc, command->argv,
				    &(G.flag_repeat), शून्य);

		अगर (argv_expanded) {
			/*
			 * expand_strvec_to_strvec() allocates memory to expand
			 * argv, we need to free it.
			 */
			मुक्त(argv_expanded);

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
#पूर्ण_अगर /* __U_BOOT__ */
	}

#अगर_अघोषित __U_BOOT__
	अगर (!pi->alive_cmds) {
		debug_leave();
		debug_printf_exec("run_pipe return 1 (all forks failed, no children)\n");
		वापस 1;
	}
#पूर्ण_अगर /* __U_BOOT__ */

	debug_leave();
#अगर_अघोषित __U_BOOT__
	debug_printf_exec("run_pipe return -1 (%u children started)\n", pi->alive_cmds);
	वापस -1;
#अन्यथा /* __U_BOOT__ */
	IF_HAS_KEYWORDS(अगर (pi->pi_inverted) rcode = !rcode;)
	debug_printf_exec("run_pipe return %d\n", rcode);
	वापस rcode;
#पूर्ण_अगर /* __U_BOOT__ */
}

/* NB: called by pseudo_exec, and therefore must not modify any
 * global data until exec/_exit (we can be a child after vfork!) */
अटल पूर्णांक run_list(काष्ठा pipe *pi)
{
#अगर ENABLE_HUSH_CASE
	अक्षर *case_word = शून्य;
#पूर्ण_अगर
#अगर ENABLE_HUSH_LOOPS
	काष्ठा pipe *loop_शीर्ष = शून्य;
	अक्षर **for_lcur = शून्य;
	अक्षर **for_list = शून्य;
#पूर्ण_अगर
	smallint last_followup;
	smalluint rcode;
#अगर ENABLE_HUSH_IF || ENABLE_HUSH_CASE
	smalluint cond_code = 0;
#अन्यथा
	क्रमागत { cond_code = 0 };
#पूर्ण_अगर
#अगर HAS_KEYWORDS
	smallint rword;      /* RES_foo */
	smallint last_rword; /* ditto */
#पूर्ण_अगर

#अगर_अघोषित __U_BOOT__
	debug_printf_exec("run_list lvl %d start\n", G.run_list_level);
	debug_enter();
#पूर्ण_अगर /* !__U_BOOT__ */

	set_G_ifs();

#अगर ENABLE_HUSH_LOOPS
	/* Check syntax for "for" */
	{
		काष्ठा pipe *cpipe;
		क्रम (cpipe = pi; cpipe; cpipe = cpipe->अग्र) {
			अगर (cpipe->res_word != RES_FOR && cpipe->res_word != RES_IN)
				जारी;
			/* current word is FOR or IN (BOLD in comments below) */
			अगर (cpipe->अग्र == शून्य) {
				syntax_error("malformed for");
				debug_leave();
				debug_printf_exec("run_list lvl %d return 1\n", G.run_list_level);
				वापस 1;
			}
			/* "FOR v; do ..." and "for v IN a b; do..." are ok */
			अगर (cpipe->अग्र->res_word == RES_DO)
				जारी;
			/* next word is not "do". It must be "in" then ("FOR v in ...") */
			अगर (cpipe->res_word == RES_IN /* "for v IN a b; not_do..."? */
			 || cpipe->अग्र->res_word != RES_IN /* FOR v not_do_and_not_in..."? */
			) {
				syntax_error("malformed for");
				debug_leave();
				debug_printf_exec("run_list lvl %d return 1\n", G.run_list_level);
				वापस 1;
			}
		}
	}
#पूर्ण_अगर

	/* Past this point, all code paths should jump to ret: label
	 * in order to return, no direct "return" statements please.
	 * This helps to ensure that no memory is leaked. */

#अगर ENABLE_HUSH_JOB
	G.run_list_level++;
#पूर्ण_अगर

#अगर HAS_KEYWORDS
	rword = RES_NONE;
	last_rword = RES_XXXX;
#पूर्ण_अगर
	last_followup = PIPE_SEQ;
	rcode = G.last_exitcode;

	/* Go through list of pipes, (maybe) executing them. */
#अगर_अघोषित __U_BOOT__
	क्रम (; pi; pi = IF_HUSH_LOOPS(rword == RES_DONE ? loop_शीर्ष : ) pi->अग्र) {
#अन्यथा /* __U_BOOT__ */
	क्रम (; pi; pi = rword == RES_DONE ? loop_शीर्ष : pi->अग्र) {
#पूर्ण_अगर /* __U_BOOT__ */
		पूर्णांक r;
		पूर्णांक sv_errexit_depth;

#अगर_अघोषित __U_BOOT__
		अगर (G.flag_SIGINT)
			अवरोध;
		अगर (G_flag_return_in_progress == 1)
			अवरोध;
#पूर्ण_अगर /* !__U_BOOT__ */

		IF_HAS_KEYWORDS(rword = pi->res_word;)
		debug_printf_exec(": rword:%d cond_code:%d last_rword:%d\n",
				rword, cond_code, last_rword);

		sv_errexit_depth = G.errexit_depth;
		अगर (
#अगर ENABLE_HUSH_IF
		    rword == RES_IF || rword == RES_ELIF ||
#पूर्ण_अगर
		    pi->followup != PIPE_SEQ
		) {
			G.errexit_depth++;
		}
#अगर ENABLE_HUSH_LOOPS
		अगर ((rword == RES_WHILE || rword == RES_UNTIL || rword == RES_FOR)
		 && loop_शीर्ष == शून्य /* avoid bumping G.depth_of_loop twice */
		) {
			/* start of a loop: remember where loop starts */
			loop_शीर्ष = pi;
			G.depth_of_loop++;
		}
#पूर्ण_अगर
		/* Still in the same "if...", "then..." or "do..." branch? */
		अगर (IF_HAS_KEYWORDS(rword == last_rword &&) 1) {
			अगर ((rcode == 0 && last_followup == PIPE_OR)
			 || (rcode != 0 && last_followup == PIPE_AND)
			) {
				/* It is "<true> || CMD" or "<false> && CMD"
				 * and we should not execute CMD */
				debug_printf_exec("skipped cmd because of || or &&\n");
				last_followup = pi->followup;
				जाओ dont_check_jobs_but_continue;
			}
		}
		last_followup = pi->followup;
#अगर ENABLE_HUSH_IF
		अगर (cond_code != 0) {
			अगर (rword == RES_THEN) {
				/* if false; then ... fi has exitcode 0! */
				G.last_exitcode = rcode = निकास_सफल;
				/* "if <false> THEN cmd": skip cmd */
				debug_printf_exec("skipped THEN cmd because IF condition was false\n");
				last_rword = rword;
				जारी;
			}
		} अन्यथा {
			अगर (rword == RES_ELSE
			 || (rword == RES_ELIF && last_rword != RES_ELIF)
			) {
				/* "if <true> then ... ELSE/ELIF cmd":
				 * skip cmd and all following ones */
				debug_printf_exec("skipped ELSE/ELIF branch because IF condition was true\n");
				अवरोध;
			}
			//if (rword == RES_THEN): "if <true> THEN cmd", run cmd (fall through)
		}
#पूर्ण_अगर
		IF_HAS_KEYWORDS(last_rword = rword;)
#अगर ENABLE_HUSH_LOOPS
		अगर (rword == RES_FOR) { /* && pi->num_cmds - always == 1 */
			अगर (!for_lcur) {
				/* first loop through for */

				अटल स्थिर अक्षर encoded_dollar_at[] ALIGN1 = {
					SPECIAL_VAR_प्रतीक, '@' | 0x80, SPECIAL_VAR_प्रतीक, '\0'
				}; /* encoded representation of "$@" */
				अटल स्थिर अक्षर *स्थिर encoded_dollar_at_argv[] ALIGN_PTR = {
					encoded_dollar_at, शून्य
				}; /* argv list with one element: "$@" */
				अक्षर **vals;

				G.last_exitcode = rcode = निकास_सफल;
				vals = (अक्षर**)encoded_dollar_at_argv;
				अगर (pi->अग्र->res_word == RES_IN) {
					/* if no variable values after "in" we skip "for" */
					अगर (!pi->अग्र->cmds[0].argv) {
						debug_printf_exec(": null FOR: exitcode EXIT_SUCCESS\n");
						अवरोध;
					}
					vals = pi->अग्र->cmds[0].argv;
				} /* else: "for var; do..." -> assume "$@" list */
				/* create list of variable values */
				debug_print_strings("for_list made from", vals);
				for_list = expand_strvec_to_strvec(vals);
				for_lcur = for_list;
				debug_print_strings("for_list", for_list);
			}
			अगर (!*for_lcur) {
				/* "for" loop is over, clean up */
				मुक्त(for_list);
				for_list = शून्य;
				for_lcur = शून्य;
				अवरोध;
			}
			/* Insert next value from for_lcur */
			/* note: *for_lcur already has quotes removed, $var expanded, etc */
#अगर_अघोषित __U_BOOT__
			set_local_var_from_halves(pi->cmds[0].argv[0], *for_lcur++);
#अन्यथा /* __U_BOOT__ */
			/* We cannot use xasprintf, so we emulate it. */
			अक्षर *full_var;
			अक्षर *var = pi->cmds[0].argv[0];
			अक्षर *val = *for_lcur++;

			/* + 1 to take into account =. */
			full_var = xmalloc(म_माप(var) + म_माप(val) + 1);
			प्र_लिखो(full_var, "%s=%s", var, val);

			set_local_var_modern(full_var, /*flag:*/ 0);
#पूर्ण_अगर /* __U_BOOT__ */
			जारी;
		}
		अगर (rword == RES_IN) {
			जारी; /* "for v IN list;..." - "in" has no cmds anyway */
		}
		अगर (rword == RES_DONE) {
			जारी; /* "done" has no cmds too */
		}
#पूर्ण_अगर
#अगर ENABLE_HUSH_CASE
		अगर (rword == RES_CASE) {
			debug_printf_exec("CASE cond_code:%d\n", cond_code);
			case_word = expand_string_to_string(pi->cmds->argv[0],
				EXP_FLAG_ESC_GLOB_CHARS, /*unbackslash:*/ 1);
			debug_printf_exec("CASE word1:'%s'\n", case_word);
			//unbackslash(case_word);
			//debug_printf_exec("CASE word2:'%s'\n", case_word);
			जारी;
		}
		अगर (rword == RES_MATCH) {
			अक्षर **argv;

			debug_printf_exec("MATCH cond_code:%d\n", cond_code);
			अगर (!case_word) /* "case ... matched_word) ... WORD)": we executed selected branch, stop */
				अवरोध;
			/* all prev words didn't match, does this one match? */
			argv = pi->cmds->argv;
			जबतक (*argv) {
				अक्षर *pattern;
				debug_printf_exec("expand_string_to_string('%s')\n", *argv);
				pattern = expand_string_to_string(*argv,
						EXP_FLAG_ESC_GLOB_CHARS,
						/*unbackslash:*/ 0
				);
				/* TODO: which FNM_xxx flags to use? */
				cond_code = (fnmatch(pattern, case_word, /*flags:*/ 0) != 0);
				debug_printf_exec("cond_code=fnmatch(pattern:'%s',str:'%s'):%d\n",
						pattern, case_word, cond_code);
				मुक्त(pattern);
				अगर (cond_code == 0) {
					/* match! we will execute this branch */
					मुक्त(case_word);
					case_word = शून्य; /* make future "word)" stop */
					अवरोध;
				}
				argv++;
			}
			जारी;
		}
		अगर (rword == RES_CASE_BODY) { /* inside of a case branch */
			debug_printf_exec("CASE_BODY cond_code:%d\n", cond_code);
			अगर (cond_code != 0)
				जारी; /* not matched yet, skip this pipe */
		}
		अगर (rword == RES_ESAC) {
			debug_printf_exec("ESAC cond_code:%d\n", cond_code);
			अगर (case_word) {
				/* "case" did not match anything: still set $? (to 0) */
				G.last_exitcode = rcode = निकास_सफल;
			}
		}
#पूर्ण_अगर
		/* Just pressing <enter> in shell should check for jobs.
		 * OTOH, in non-interactive shell this is useless
		 * and only leads to extra job checks */
		अगर (pi->अंक_cmds == 0) {
#अगर_अघोषित __U_BOOT__
			अगर (G_interactive_fd)
				जाओ check_jobs_and_continue;
#पूर्ण_अगर /* !__U_BOOT__ */
			जारी;
		}

		/* After analyzing all keywords and conditions, we decided
		 * to execute this pipe. NB: have to do checkjobs(NULL)
		 * after run_pipe to collect any background children,
		 * even if list execution is to be stopped. */
		debug_printf_exec(": run_pipe with %d members\n", pi->अंक_cmds);
#अगर_अघोषित __U_BOOT__
#अगर ENABLE_HUSH_LOOPS
		G.flag_break_continue = 0;
#पूर्ण_अगर
#पूर्ण_अगर /* !__U_BOOT__ */
#अगर_अघोषित __U_BOOT__
		rcode = r = G.o_opt[OPT_O_NOEXEC] ? 0 : run_pipe(pi);
		/* NB: rcode is a smalluint, r is int */
#अन्यथा /* __U_BOOT__ */
		rcode = r = run_pipe(pi); /* NB: rcode is a smalluint, r is int */
		अगर (r <= EXIT_RET_CODE) {
			पूर्णांक previous_rcode = G.last_exitcode;
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
			अगर (previous_rcode != r) {
				/*
				 * If run from run_command, run_command_flags will be set, so we check
				 * this to know if we are in main input shell.
				 */
				अगर (!G.run_command_flags)
					म_लिखो("exit not allowed from main input shell.\n");

				अवरोध;
			}
			जारी;
		}
#पूर्ण_अगर /* __U_BOOT__ */
		अगर (r != -1) {
			/* We ran a builtin, function, or group.
			 * rcode is already known
			 * and we don't need to wait for anything. */
			debug_printf_exec(": builtin/func exitcode %d\n", rcode);
			G.last_exitcode = rcode;
#अगर_अघोषित __U_BOOT__
			check_and_run_traps();
#पूर्ण_अगर /* !__U_BOOT__ */
#अगर ENABLE_HUSH_TRAP && ENABLE_HUSH_FUNCTIONS
			rcode = G.last_exitcode; /* "return" in trap can change it, read back */
#पूर्ण_अगर
#अगर_अघोषित __U_BOOT__
#अगर ENABLE_HUSH_LOOPS
			/* Was it "break" or "continue"? */
			अगर (G.flag_break_continue) {
				smallint fbc = G.flag_break_continue;
				/* We might fall into outer *loop*,
				 * don't want to break it too */
				अगर (loop_शीर्ष) {
					G.depth_break_continue--;
					अगर (G.depth_break_continue == 0)
						G.flag_break_continue = 0;
					/* else: e.g. "continue 2" should *break* once, *then* continue */
				} /* else: "while... do... { we are here (innermost list is not a loop!) };...done" */
				अगर (G.depth_break_continue != 0 || fbc == BC_BREAK) {
					checkjobs(शून्य, 0 /*(no pid to wait for)*/);
					अवरोध;
				}
				/* "continue": simulate end of loop */
				rword = RES_DONE;
				जारी;
			}
#पूर्ण_अगर
			अगर (G_flag_return_in_progress == 1) {
				checkjobs(शून्य, 0 /*(no pid to wait for)*/);
				अवरोध;
			}

		} अन्यथा अगर (pi->followup == PIPE_BG) {
			/* What does bash do with attempts to background builtins? */
			/* even bash 3.2 doesn't do that well with nested bg:
			 * try "{ { sleep 10; echo DEEP; } & echo HERE; } &".
			 * I'm NOT treating inner &'s as jobs */
#अगर ENABLE_HUSH_JOB
			अगर (G.run_list_level == 1)
				जोड़ो_job_into_table(pi);
#पूर्ण_अगर
			/* Last command's pid goes to $! */
			G.last_bg_pid = pi->cmds[pi->अंक_cmds - 1].pid;
			G.last_bg_pid_exitcode = 0;
			debug_printf_exec(": cmd&: exitcode EXIT_SUCCESS\n");
/* Check pi->pi_inverted? "! sleep 1 & echo $?": bash says 1. dash and ash say 0 */
			rcode = निकास_सफल;
			जाओ check_traps;
		} अन्यथा {
#अगर ENABLE_HUSH_JOB
			अगर (G.run_list_level == 1 && G_interactive_fd) {
				/* Waits for completion, then fg's main shell */
				rcode = checkjobs_and_fg_shell(pi);
				debug_printf_exec(": checkjobs_and_fg_shell exitcode %d\n", rcode);
				जाओ check_traps;
			}
#पूर्ण_अगर
			/* This one just waits for completion */
			rcode = checkjobs(pi, 0 /*(no pid to wait for)*/);
			debug_printf_exec(": checkjobs exitcode %d\n", rcode);
 check_traps:
			G.last_exitcode = rcode;
			check_and_run_traps();
#अगर ENABLE_HUSH_TRAP && ENABLE_HUSH_FUNCTIONS
			rcode = G.last_exitcode; /* "return" in trap can change it, read back */
#पूर्ण_अगर
		}
#पूर्ण_अगर /* !__U_BOOT__ */

#अगर_अघोषित __U_BOOT__
		/* Handle "set -e" */
		अगर (rcode != 0 && G.o_opt[OPT_O_ERREXIT]) {
			debug_printf_exec("ERREXIT:1 errexit_depth:%d\n", G.errexit_depth);
			अगर (G.errexit_depth == 0)
				hush_exit(rcode);
		}
#अन्यथा /* __U_BOOT__ */
		} /* if (r != -1) */
#पूर्ण_अगर /* __U_BOOT__ */
		G.errexit_depth = sv_errexit_depth;

		/* Analyze how result affects subsequent commands */
#अगर ENABLE_HUSH_IF
		अगर (rword == RES_IF || rword == RES_ELIF) {
			debug_printf_exec("cond_code=rcode:%d\n", rcode);
			cond_code = rcode;
		}
#पूर्ण_अगर
#अगर_अघोषित __U_BOOT__
 check_jobs_and_continue:
		checkjobs(शून्य, 0 /*(no pid to wait for)*/);
#पूर्ण_अगर /* !__U_BOOT__ */
 dont_check_jobs_but_continue: ;
#अगर ENABLE_HUSH_LOOPS
		/* Beware of "while false; true; do ..."! */
		अगर (pi->अग्र
		 && (pi->अग्र->res_word == RES_DO || pi->अग्र->res_word == RES_DONE)
		 /* check for RES_DONE is needed for "while ...; do \n done" case */
		) {
			अगर (rword == RES_WHILE) {
				अगर (rcode) {
					/* "while false; do...done" - exitcode 0 */
					G.last_exitcode = rcode = निकास_सफल;
					debug_printf_exec(": while expr is false: breaking (exitcode:EXIT_SUCCESS)\n");
					अवरोध;
				}
			}
			अगर (rword == RES_UNTIL) {
				अगर (!rcode) {
					debug_printf_exec(": until expr is true: breaking\n");
					अवरोध;
				}
			}
		}
#पूर्ण_अगर
	} /* for (pi) */

#अगर ENABLE_HUSH_JOB
	G.run_list_level--;
#पूर्ण_अगर
#अगर ENABLE_HUSH_LOOPS
	अगर (loop_शीर्ष)
		G.depth_of_loop--;
	मुक्त(for_list);
#पूर्ण_अगर
#अगर ENABLE_HUSH_CASE
	मुक्त(case_word);
#पूर्ण_अगर
#अगर_अघोषित __U_BOOT__
	debug_leave();
	debug_printf_exec("run_list lvl %d return %d\n", G.run_list_level, rcode);
#पूर्ण_अगर /* !__U_BOOT__ */
	वापस rcode;
}

/* Select which version we will use */
अटल पूर्णांक run_and_free_list(काष्ठा pipe *pi)
{
	पूर्णांक rcode = 0;
	debug_printf_exec("run_and_free_list entered\n");
#अगर_अघोषित __U_BOOT__
	अगर (!G.o_opt[OPT_O_NOEXEC]) {
#पूर्ण_अगर /* !__U_BOOT__ */
		debug_printf_exec(": run_list: 1st pipe with %d cmds\n", pi->अंक_cmds);
		rcode = run_list(pi);
#अगर_अघोषित __U_BOOT__
	}
#पूर्ण_अगर /* !__U_BOOT__ */
	/* free_pipe_list has the side effect of clearing memory.
	 * In the long run that function can be merged with run_list,
	 * but doing that now would hobble the debugging effort. */
	free_pipe_list(pi);
	debug_printf_exec("run_and_free_list return %d\n", rcode);
	वापस rcode;
}

#अगर_अघोषित __U_BOOT__
अटल व्योम install_sighandlers(अचिन्हित mask)
{
	sighandler_t old_handler;
	अचिन्हित sig = 0;
	जबतक ((mask >>= 1) != 0) {
		sig++;
		अगर (!(mask & 1))
			जारी;
		old_handler = install_sighandler(sig, pick_sighandler(sig));
		/* POSIX allows shell to re-enable SIGCHLD
		 * even if it was SIG_IGN on entry.
		 * Therefore we skip IGN check for it:
		 */
		अगर (sig == SIGCHLD)
			जारी;
		/* Interactive bash re-enables SIGHUP which is SIG_IGNed on entry.
		 * Try:
		 * trap '' hup; bash; echo RET  # type "kill -hup $$", see SIGHUP having effect
		 * trap '' hup; bash -c 'kill -hup $$; echo ALIVE'  # here SIGHUP is SIG_IGNed
		 */
		अगर (sig == SIGHUP && G_interactive_fd)
			जारी;
		/* Unless one of the above signals, is it SIG_IGN? */
		अगर (old_handler == संक_छोड़ो) {
			/* oops... restore back to IGN, and record this fact */
			install_sighandler(sig, old_handler);
#अगर ENABLE_HUSH_TRAP
			अगर (!G_traps)
				G_traps = xzalloc(माप(G_traps[0]) * NSIG);
			मुक्त(G_traps[sig]);
			G_traps[sig] = xzalloc(1); /* == xstrdup(""); */
#पूर्ण_अगर
		}
	}
}

/* Called a few times only (or even once if "sh -c") */
अटल व्योम install_special_sighandlers(व्योम)
{
	अचिन्हित mask;

	/* Which signals are shell-special? */
	mask = (1 << SIGQUIT) | (1 << SIGCHLD);
	अगर (G_interactive_fd) {
		mask |= SPECIAL_INTERACTIVE_SIGS;
		अगर (G_saved_tty_pgrp) /* we have ctty, job control sigs work */
			mask |= SPECIAL_JOBSTOP_SIGS;
	}
	/* Careful, do not re-install handlers we already installed */
	अगर (G.special_sig_mask != mask) {
		अचिन्हित diff = mask & ~G.special_sig_mask;
		G.special_sig_mask = mask;
		install_sighandlers(diff);
	}
}

#अगर ENABLE_HUSH_JOB
/* helper */
/* Set handlers to restore tty pgrp and exit */
अटल व्योम install_fatal_sighandlers(व्योम)
{
	अचिन्हित mask;

	/* We will restore tty pgrp on these signals */
	mask = 0
		/*+ (1 << SIGILL ) * HUSH_DEBUG*/
		/*+ (1 << SIGFPE ) * HUSH_DEBUG*/
		+ (1 << SIGBUS ) * HUSH_DEBUG
		+ (1 << संक_अंश) * HUSH_DEBUG
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
#पूर्ण_अगर

अटल पूर्णांक set_mode(पूर्णांक state, अक्षर mode, स्थिर अक्षर *o_opt)
{
	पूर्णांक idx;
	चयन (mode) {
	हाल 'n':
		/* set -n has no effect in interactive shell */
		/* Try: while set -n; do echo $-; done */
		अगर (!G_interactive_fd)
			G.o_opt[OPT_O_NOEXEC] = state;
		अवरोध;
	हाल 'x':
		IF_HUSH_MODE_X(G_x_mode = state;)
		IF_HUSH_MODE_X(अगर (G.x_mode_fd <= 0) G.x_mode_fd = dup_CLOEXEC(2, 10);)
		अवरोध;
	हाल 'e':
		G.o_opt[OPT_O_ERREXIT] = state;
		अवरोध;
	हाल 'o':
		अगर (!o_opt) {
			/* "set -o" or "set +o" without parameter.
			 * in bash, set -o produces this output:
			 *  pipefail        off
			 * and set +o:
			 *  set +o pipefail
			 * We always use the second form.
			 */
			स्थिर अक्षर *p = o_opt_strings;
			idx = 0;
			जबतक (*p) {
				म_लिखो("set %co %s\n", (G.o_opt[idx] ? '-' : '+'), p);
				idx++;
				p += म_माप(p) + 1;
			}
			अवरोध;
		}
		idx = सूचकांक_in_strings(o_opt_strings, o_opt);
		अगर (idx >= 0) {
			G.o_opt[idx] = state;
			अवरोध;
		}
		/* fall through to error */
	शेष:
		वापस निकास_त्रुटि;
	}
	वापस निकास_सफल;
}

पूर्णांक hush_main(पूर्णांक argc, अक्षर **argv) MAIN_EXTERNALLY_VISIBLE;
पूर्णांक hush_main(पूर्णांक argc, अक्षर **argv)
{
	pid_t cached_getpid;
	क्रमागत {
		OPT_login = (1 << 0),
	};
	अचिन्हित flags;
#अगर !BB_MMU
	अचिन्हित builtin_argc = 0;
#पूर्ण_अगर
	अक्षर **e;
	काष्ठा variable *cur_var;
	काष्ठा variable *shell_ver;

	INIT_G();
	अगर (निकास_सफल != 0) /* if EXIT_SUCCESS == 0, it is already done */
		G.last_exitcode = निकास_सफल;
#अगर !BB_MMU
	/* "Big heredoc" support via "sh -< STRING" invocation.
	 * Check it first (do not bother to run the usual init code,
	 * it is not needed for this case).
	 */
	अगर (argv[1]
	 && argv[1][0] == '-' && argv[1][1] == '<' /*&& !argv[1][2]*/
	 /*&& argv[2] && !argv[3] - we don't check some conditions */
	) {
		full_write1_str(argv[2]);
		_निकास(0);
	}
	G.argv0_for_re_execing = argv[0];
#पूर्ण_अगर
#अगर ENABLE_HUSH_TRAP
# अगर ENABLE_HUSH_FUNCTIONS
	G.return_exitcode = -1;
# endif
	G.pre_trap_exitcode = -1;
#पूर्ण_अगर

#अगर ENABLE_HUSH_FAST
	G.गिनती_SIGCHLD++; /* ensure it is != G.handled_SIGCHLD */
#पूर्ण_अगर

	cached_getpid = getpid();   /* for tcsetpgrp() during init */
	G.मूल_pid = cached_getpid; /* for $PID  (NOMMU can override via -$HEXPID:HEXPPID:...) */
	G.मूल_ppid = getppid();    /* for $PPID (NOMMU can override) */

	/* Deal with HUSH_VERSION */
	debug_printf_env("unsetenv '%s'\n", "HUSH_VERSION");
	unsetenv("HUSH_VERSION"); /* in case it exists in initial env */
	shell_ver = xzalloc(माप(*shell_ver));
	shell_ver->flg_export = 1;
	shell_ver->flg_read_only = 1;
	/* Code which handles ${var<op>...} needs writable values for all variables,
	 * therefore we xstrdup: */
	shell_ver->varstr = xstrdup(hush_version_str);

	/* Create shell local variables from the values
	 * currently living in the environment */
	G.शीर्ष_var = shell_ver;
	cur_var = G.शीर्ष_var;
	e = environ;
	अगर (e) जबतक (*e) {
		अक्षर *मूल्य = म_अक्षर(*e, '=');
		अगर (मूल्य) { /* paranoia */
			cur_var->अग्र = xzalloc(माप(*cur_var));
			cur_var = cur_var->अग्र;
			cur_var->varstr = *e;
			cur_var->max_len = म_माप(*e);
			cur_var->flg_export = 1;
		}
		e++;
	}
	/* (Re)insert HUSH_VERSION into env (AFTER we scanned the env!) */
	debug_printf_env("putenv '%s'\n", shell_ver->varstr);
	putenv(shell_ver->varstr);

	/* Export PWD */
	set_pwd_var(SETFLAG_EXPORT);

#अगर BASH_HOSTNAME_VAR
	/* Set (but not export) HOSTNAME unless already set */
	अगर (!get_local_var_मूल्य("HOSTNAME")) {
		काष्ठा utsname uts;
		uname(&uts);
		set_local_var_from_halves("HOSTNAME", uts.nodename);
	}
#पूर्ण_अगर
	/* IFS is not inherited from the parent environment */
	set_local_var_from_halves("IFS", defifs);

	अगर (!get_local_var_मूल्य("PATH"))
		set_local_var_from_halves("PATH", bb_default_मूल_path);

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

#अगर अंक_SCRIPTS > 0
	अगर (argc < 0) {
		अक्षर *script = get_script_content(-argc - 1);
		G.global_argv = argv;
		G.global_argc = string_array_len(argv);
		//install_special_sighandlers(); - needed?
		parse_and_run_string(script);
		जाओ final_return;
	}
#पूर्ण_अगर

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
	जबतक (1) {
		पूर्णांक opt = getopt(argc, argv, "+" /* stop at 1st non-option */
				"cexinsl"
#अगर !BB_MMU
				"$:R:V:"
# अगर ENABLE_HUSH_LINENO_VAR
				"L:"
# endif
# अगर ENABLE_HUSH_FUNCTIONS
				"F:"
# endif
#पूर्ण_अगर
		);
		अगर (opt <= 0)
			अवरोध;
		चयन (opt) {
		हाल 'c':
			/* Note: -c is not an option with param!
			 * "hush -c -l SCRIPT" is valid. "hush -cSCRIPT" is not.
			 */
			G.opt_c = 1;
			अवरोध;
		हाल 'i':
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
			अवरोध;
		हाल 's':
			G.opt_s = 1;
			अवरोध;
		हाल 'l':
			flags |= OPT_login;
			अवरोध;
#अगर !BB_MMU
		हाल '$': {
			अचिन्हित दीर्घ दीर्घ empty_trap_mask;

			G.मूल_pid = bb_strtou(optarg, &optarg, 16);
			optarg++;
			G.मूल_ppid = bb_strtou(optarg, &optarg, 16);
			optarg++;
			G.last_bg_pid = bb_strtou(optarg, &optarg, 16);
			optarg++;
			G.last_exitcode = bb_strtou(optarg, &optarg, 16);
			optarg++;
			builtin_argc = bb_strtou(optarg, &optarg, 16);
			optarg++;
			empty_trap_mask = bb_strtoull(optarg, &optarg, 16);
			अगर (empty_trap_mask != 0) {
				IF_HUSH_TRAP(पूर्णांक sig;)
				install_special_sighandlers();
# अगर ENABLE_HUSH_TRAP
				G_traps = xzalloc(माप(G_traps[0]) * NSIG);
				क्रम (sig = 1; sig < NSIG; sig++) {
					अगर (empty_trap_mask & (1LL << sig)) {
						G_traps[sig] = xzalloc(1); /* == xstrdup(""); */
						install_sighandler(sig, संक_छोड़ो);
					}
				}
# endif
			}
# अगर ENABLE_HUSH_LOOPS
			optarg++;
			G.depth_of_loop = bb_strtou(optarg, &optarg, 16);
# endif
			/* Suppress "killed by signal" message, -$ hack is used
			 * for subshells: echo `sh -c 'kill -9 $$'`
			 * should be silent.
			 */
			IF_HUSH_JOB(G.run_list_level = 1;)
# अगर ENABLE_HUSH_FUNCTIONS
			/* nommu uses re-exec trick for "... | func | ...",
			 * should allow "return".
			 * This accidentally allows returns in subshells.
			 */
			G_flag_return_in_progress = -1;
# endif
			अवरोध;
		}
		हाल 'R':
		हाल 'V':
			set_local_var(xstrdup(optarg), opt == 'R' ? SETFLAG_MAKE_RO : 0);
			अवरोध;
# अगर ENABLE_HUSH_LINENO_VAR
		हाल 'L':
			G.parse_lineno = xatou(optarg);
			अवरोध;
# endif
# अगर ENABLE_HUSH_FUNCTIONS
		हाल 'F': {
			काष्ठा function *funcp = new_function(optarg);
			/* funcp->name is already set to optarg */
			/* funcp->body is set to NULL. It's a special case. */
			funcp->body_as_string = argv[optind];
			optind++;
			अवरोध;
		}
# endif
#पूर्ण_अगर
		/*case '?': invalid option encountered (set_mode('?') will fail) */
		/*case 'n':*/
		/*case 'x':*/
		/*case 'e':*/
		शेष:
			अगर (set_mode(1, opt, शून्य) == 0) /* no error */
				अवरोध;
			bb_show_usage();
		}
	} /* option parsing loop */

	/* Skip options. Try "hush -l": $1 should not be "-l"! */
	G.global_argc = argc - (optind - 1);
	G.global_argv = argv + (optind - 1);
	G.global_argv[0] = argv[0];

	/* If we are login shell... */
	अगर (flags & OPT_login) {
		स्थिर अक्षर *hp = शून्य;
		HFILE *निवेश;

		debug_printf("sourcing /etc/profile\n");
		निवेश = hfopen("/etc/profile");
 run_profile:
		अगर (निवेश != शून्य) {
			install_special_sighandlers();
			parse_and_run_file(निवेश);
			hfclose(निवेश);
		}
		/* bash: after sourcing /etc/profile,
		 * tries to source (in the given order):
		 * ~/.bash_profile, ~/.bash_login, ~/.profile,
		 * stopping on first found. --noprofile turns this off.
		 * bash also sources ~/.bash_logout on exit.
		 * If called as sh, skips .bash_XXX files.
		 */
		अगर (!hp) { /* unless we looped on the "goto" already */
			hp = get_local_var_मूल्य("HOME");
			अगर (hp && hp[0]) {
				debug_printf("sourcing ~/.profile\n");
				hp = concat_path_file(hp, ".profile");
				निवेश = hfopen(hp);
				मुक्त((अक्षर*)hp);
				जाओ run_profile;
			}
		}
	}

#अगर_अघोषित __U_BOOT__
	/* -c takes effect *after* -l */
	अगर (G.opt_c) {
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
		अक्षर *script;

		install_special_sighandlers();

		G.global_argc--;
		G.global_argv++;
#अगर !BB_MMU
		अगर (builtin_argc) {
			/* -c 'builtin' [BARGV...] "" ARG0 [ARG1...] */
			स्थिर काष्ठा built_in_command *x;
			x = find_builtin(G.global_argv[0]);
			अगर (x) { /* paranoia */
				argv = G.global_argv;
				G.global_argc -= builtin_argc + 1; /* skip [BARGV...] "" */
				G.global_argv += builtin_argc + 1;
				G.global_argv[-1] = शून्य; /* replace "" */
				G.last_exitcode = x->b_function(argv);
			}
			जाओ final_return;
		}
#पूर्ण_अगर

		script = G.global_argv[0];
		अगर (!script)
			bb_error_संदेश_and_die(bb_संदेश_requires_arg, "-c");
		अगर (!G.global_argv[1]) {
			/* -c 'script' (no params): prevent empty $0 */
			G.global_argv[0] = argv[0];
		} अन्यथा { /* else -c 'script' ARG0 [ARG1...]: $0 is ARG0 */
			G.global_argc--;
			G.global_argv++;
		}
		parse_and_run_string(script);
		जाओ final_return;
	}

	/* -s is: hush -s ARGV1 ARGV2 (no SCRIPT) */
	अगर (!G.opt_s && G.global_argv[1]) {
		HFILE *निवेश;
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
		निवेश = hfopen(G.global_argv[0]);
		अगर (!निवेश) {
			bb_simple_perror_संदेश_and_die(G.global_argv[0]);
		}
		xfunc_error_retval = 1;
		install_special_sighandlers();
		parse_and_run_file(निवेश);
#अगर ENABLE_FEATURE_CLEAN_UP
		hfclose(निवेश);
#पूर्ण_अगर
		जाओ final_return;
	}
	/* "implicit" -s: bare interactive hush shows 's' in $- */
	G.opt_s = 1;

#पूर्ण_अगर /* __U_BOOT__ */
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
#अगर ENABLE_HUSH_JOB
	अगर (isatty(STDIN_FILENO) && isatty(STDOUT_FILENO)) {
		G_saved_tty_pgrp = tcgetpgrp(STDIN_FILENO);
		debug_printf("saved_tty_pgrp:%d\n", G_saved_tty_pgrp);
		अगर (G_saved_tty_pgrp < 0)
			G_saved_tty_pgrp = 0;

		/* try to dup stdin to high fd#, >= 255 */
		G_interactive_fd = dup_CLOEXEC(STDIN_FILENO, 254);
		अगर (G_interactive_fd < 0) {
			/* try to dup to any fd */
			G_interactive_fd = dup_CLOEXEC(STDIN_FILENO, -1);
			अगर (G_interactive_fd < 0) {
				/* give up */
				G_interactive_fd = 0;
				G_saved_tty_pgrp = 0;
			}
		}
	}
	debug_printf("interactive_fd:%d\n", G_interactive_fd);
	अगर (G_interactive_fd) {
		अगर (G_saved_tty_pgrp) {
			/* If we were run as 'hush &', sleep until we are
			 * in the foreground (tty pgrp == our pgrp).
			 * If we get started under a job aware app (like bash),
			 * make sure we are now in charge so we don't fight over
			 * who gets the foreground */
			जबतक (1) {
				pid_t shell_pgrp = getpgrp();
				G_saved_tty_pgrp = tcgetpgrp(G_interactive_fd);
				अगर (G_saved_tty_pgrp == shell_pgrp)
					अवरोध;
				/* send TTIN to ourself (should stop us) */
				समाप्त(- shell_pgrp, SIGTTIN);
			}
		}

		/* Install more signal handlers */
		install_special_sighandlers();

		अगर (G_saved_tty_pgrp) {
			/* Set other signals to restore saved_tty_pgrp */
			install_fatal_sighandlers();
			/* Put ourselves in our own process group
			 * (bash, too, does this only if ctty is available) */
			bb_setpgrp(); /* is the same as setpgid(our_pid, our_pid); */
			/* Grab control of the terminal */
			tcsetpgrp(G_interactive_fd, cached_getpid);
		}
		enable_restore_tty_pgrp_on_exit();

# अगर ENABLE_FEATURE_EDITING
		G.पंक्ति_निवेश_state = new_पंक्ति_निवेश_t(FOR_SHELL);
#  अगर ENABLE_FEATURE_TAB_COMPLETION
		G.पंक्ति_निवेश_state->get_exe_name = hush_command_name;
#  endif
#  अगर EDITING_HAS_sh_get_var
		G.पंक्ति_निवेश_state->sh_get_var = get_local_var_मूल्य;
#  endif
# endif
# अगर ENABLE_HUSH_SAVEHISTORY && MAX_HISTORY > 0
		{
			स्थिर अक्षर *hp = get_local_var_मूल्य("HISTFILE");
			अगर (!hp) {
				hp = get_local_var_मूल्य("HOME");
				अगर (hp)
					hp = concat_path_file(hp, ".hush_history");
			} अन्यथा {
				hp = xstrdup(hp);
			}
			अगर (hp) {
				G.पंक्ति_निवेश_state->hist_file = hp;
				//set_local_var(xasprintf("HISTFILE=%s", ...));
			}
#  अगर ENABLE_FEATURE_SH_HISTFILESIZE
			hp = get_local_var_मूल्य("HISTFILESIZE");
			G.पंक्ति_निवेश_state->max_history = आकार_from_HISTFILESIZE(hp);
#  endif
		}
# endif
	} अन्यथा {
		install_special_sighandlers();
	}
#या_अगर ENABLE_HUSH_INTERACTIVE
	/* No job control compiled in, only prompt/line editing */
	अगर (isatty(STDIN_FILENO) && isatty(STDOUT_FILENO)) {
		G_interactive_fd = dup_CLOEXEC(STDIN_FILENO, 254);
		अगर (G_interactive_fd < 0) {
			/* try to dup to any fd */
			G_interactive_fd = dup_CLOEXEC(STDIN_FILENO, -1);
			अगर (G_interactive_fd < 0)
				/* give up */
				G_interactive_fd = 0;
		}
	}
	install_special_sighandlers();
#अन्यथा
	/* We have interactiveness code disabled */
	install_special_sighandlers();
#पूर्ण_अगर
	/* bash:
	 * if interactive but not a login shell, sources ~/.bashrc
	 * (--norc turns this off, --rcfile <file> overrides)
	 */

	अगर (G_interactive_fd) {
#अगर ENABLE_HUSH_INTERACTIVE && ENABLE_FEATURE_EDITING_FANCY_PROMPT
		/* Set (but not export) PS1/2 unless already set */
		अगर (!get_local_var_मूल्य("PS1"))
			set_local_var_from_halves("PS1", "\\w \\$ ");
		अगर (!get_local_var_मूल्य("PS2"))
			set_local_var_from_halves("PS2", "> ");
#पूर्ण_अगर
		अगर (!ENABLE_FEATURE_SH_EXTRA_QUIET) {
			/* note: ash and hush share this string */
			म_लिखो("\n\n%s %s\n"
				IF_HUSH_HELP("Enter 'help' for a list of built-in commands.\n")
				"\n",
				bb_banner,
				"hush - the humble shell"
			);
		}
	}

	parse_and_run_file(hfopen(शून्य)); /* stdin */

 final_return:
	hush_exit(G.last_exitcode);
}

/*
 * Built-ins
 */
अटल पूर्णांक FAST_FUNC builtin_true(अक्षर **argv UNUSED_PARAM)
{
	वापस 0;
}

अटल पूर्णांक FAST_FUNC builtin_false(अक्षर **argv UNUSED_PARAM)
{
	वापस 1;
}

#अगर ENABLE_HUSH_TEST || ENABLE_HUSH_ECHO || ENABLE_HUSH_PRINTF || ENABLE_HUSH_KILL
अटल NOINLINE पूर्णांक run_applet_main(अक्षर **argv, पूर्णांक (*applet_main_func)(पूर्णांक argc, अक्षर **argv))
{
	पूर्णांक argc = string_array_len(argv);
	वापस applet_main_func(argc, argv);
}
#पूर्ण_अगर
#अगर ENABLE_HUSH_TEST || BASH_TEST2
अटल पूर्णांक FAST_FUNC builtin_test(अक्षर **argv)
{
	वापस run_applet_main(argv, test_main);
}
#पूर्ण_अगर
#अगर ENABLE_HUSH_ECHO
अटल पूर्णांक FAST_FUNC builtin_echo(अक्षर **argv)
{
	वापस run_applet_main(argv, echo_main);
}
#पूर्ण_अगर
#अगर ENABLE_HUSH_PRINTF
अटल पूर्णांक FAST_FUNC builtin_printf(अक्षर **argv)
{
	वापस run_applet_main(argv, printf_main);
}
#पूर्ण_अगर

#अगर ENABLE_HUSH_HELP
अटल पूर्णांक FAST_FUNC builtin_help(अक्षर **argv UNUSED_PARAM)
{
	स्थिर काष्ठा built_in_command *x;

	म_लिखो(
		"Built-in commands:\n"
		"------------------\n");
	क्रम (x = bltins1; x != &bltins1[ARRAY_आकार(bltins1)]; x++) {
		अगर (x->b_descr)
			म_लिखो("%-10s%s\n", x->b_cmd, x->b_descr);
	}
	वापस निकास_सफल;
}
#पूर्ण_अगर

#अगर MAX_HISTORY && ENABLE_FEATURE_EDITING
अटल पूर्णांक FAST_FUNC builtin_history(अक्षर **argv UNUSED_PARAM)
{
	show_history(G.पंक्ति_निवेश_state);
	वापस निकास_सफल;
}
#पूर्ण_अगर

अटल पूर्णांक FAST_FUNC builtin_cd(अक्षर **argv)
{
	स्थिर अक्षर *newdir;

	argv = skip_dash_dash(argv);
	newdir = argv[0];
	अगर (newdir == शून्य) {
		/* bash does nothing (exitcode 0) if HOME is ""; if it's unset,
		 * bash says "bash: cd: HOME not set" and does nothing
		 * (exitcode 1)
		 */
		स्थिर अक्षर *home = get_local_var_मूल्य("HOME");
		newdir = home ? home : "/";
	}
	अगर (स_बदलो(newdir)) {
		/* Mimic bash message exactly */
		bb_perror_संदेश("cd: %s", newdir);
		वापस निकास_त्रुटि;
	}
	/* Read current dir (get_cwd(1) is inside) and set PWD.
	 * Note: do not enforce exporting. If PWD was unset or unexported,
	 * set it again, but do not export. bash does the same.
	 */
	set_pwd_var(/*flag:*/ 0);
	वापस निकास_सफल;
}

अटल पूर्णांक FAST_FUNC builtin_pwd(अक्षर **argv UNUSED_PARAM)
{
	माला_दो(get_cwd(0));
	वापस निकास_सफल;
}

अटल पूर्णांक FAST_FUNC builtin_eval(अक्षर **argv)
{
	argv = skip_dash_dash(argv);

	अगर (!argv[0])
		वापस निकास_सफल;

	IF_HUSH_MODE_X(G.x_mode_depth++;)
	//bb_error_msg("%s: ++x_mode_depth=%d", __func__, G.x_mode_depth);
	अगर (!argv[1]) {
		/* bash:
		 * eval "echo Hi; done" ("done" is syntax error):
		 * "echo Hi" will not execute too.
		 */
		parse_and_run_string(argv[0]);
	} अन्यथा {
		/* "The eval utility shall construct a command by
		 * concatenating arguments together, separating
		 * each with a <space> character."
		 */
		अक्षर *str, *p;
		अचिन्हित len = 0;
		अक्षर **pp = argv;
		करो
			len += म_माप(*pp) + 1;
		जबतक (*++pp);
		str = p = xmalloc(len);
		pp = argv;
		क्रम (;;) {
			p = stpcpy(p, *pp);
			pp++;
			अगर (!*pp)
				अवरोध;
			*p++ = ' ';
		}
		parse_and_run_string(str);
		मुक्त(str);
	}
	IF_HUSH_MODE_X(G.x_mode_depth--;)
	//bb_error_msg("%s: --x_mode_depth=%d", __func__, G.x_mode_depth);
	वापस G.last_exitcode;
}

अटल पूर्णांक FAST_FUNC builtin_exec(अक्षर **argv)
{
	argv = skip_dash_dash(argv);
	अगर (argv[0] == शून्य)
		वापस निकास_सफल; /* bash does this */

	/* Careful: we can end up here after [v]fork. Do not restore
	 * tty pgrp then, only top-level shell process does that */
	अगर (G_saved_tty_pgrp && getpid() == G.मूल_pid)
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

अटल पूर्णांक FAST_FUNC builtin_exit(अक्षर **argv)
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
	अगर (argv[0] == शून्य) {
#अगर ENABLE_HUSH_TRAP
		अगर (G.pre_trap_exitcode >= 0) /* "exit" in trap uses $? from before the trap */
			hush_exit(G.pre_trap_exitcode);
#पूर्ण_अगर
		hush_exit(G.last_exitcode);
	}
	/* mimic bash: exit 123abc == exit 255 + error msg */
	xfunc_error_retval = 255;
	/* bash: exit -2 == exit 254, no error msg */
	hush_exit(xatoi(argv[0]) & 0xff);
}

#अगर ENABLE_HUSH_TYPE
/* http://www.opengroup.org/onlinepubs/9699919799/utilities/type.html */
अटल पूर्णांक FAST_FUNC builtin_type(अक्षर **argv)
{
	पूर्णांक ret = निकास_सफल;

	जबतक (*++argv) {
		स्थिर अक्षर *type;
		अक्षर *path = शून्य;

		अगर (0) {} /* make conditional compile easier below */
		/*else if (find_alias(*argv))
			type = "an alias";*/
# अगर ENABLE_HUSH_FUNCTIONS
		अन्यथा अगर (find_function(*argv))
			type = "a function";
# endif
		अन्यथा अगर (find_builtin(*argv))
			type = "a shell builtin";
		अन्यथा अगर ((path = find_in_path(*argv)) != शून्य)
			type = path;
		अन्यथा {
			bb_error_संदेश("type: %s: not found", *argv);
			ret = निकास_त्रुटि;
			जारी;
		}

		म_लिखो("%s is %s\n", *argv, type);
		मुक्त(path);
	}

	वापस ret;
}
#पूर्ण_अगर

#अगर ENABLE_HUSH_READ
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
अटल पूर्णांक FAST_FUNC builtin_read(अक्षर **argv)
{
	स्थिर अक्षर *r;
	काष्ठा builtin_read_params params;

	स_रखो(&params, 0, माप(params));

	/* "!": do not abort on errors.
	 * Option string must start with "sr" to match BUILTIN_READ_xxx
	 */
	params.read_flags = getopt32(argv,
# अगर BASH_READ_D
		IF_NOT_HUSH_BASH_COMPAT("^")
		"!srn:p:t:u:d:" IF_NOT_HUSH_BASH_COMPAT("\0" "-1"/*min 1 arg*/),
		&params.opt_n, &params.opt_p, &params.opt_t, &params.opt_u, &params.opt_d
# अन्यथा
		IF_NOT_HUSH_BASH_COMPAT("^")
		"!srn:p:t:u:" IF_NOT_HUSH_BASH_COMPAT("\0" "-1"/*min 1 arg*/),
		&params.opt_n, &params.opt_p, &params.opt_t, &params.opt_u
# endif
//TODO: print "read: need variable name"
//for the case of !BASH "read" with no args (now it fails silently)
//(or maybe extend getopt32() to emit a message if "-1" fails)
	);
	अगर ((uint32_t)params.read_flags == (uint32_t)-1)
		वापस निकास_त्रुटि;
	argv += optind;
	params.argv = argv;
	params.setvar = set_local_var_from_halves;
	params.ifs = get_local_var_मूल्य("IFS"); /* can be NULL */

 again:
	r = shell_builtin_read(&params);

	अगर ((uintptr_t)r == 1 && त्रुटि_सं == EINTR) {
		अचिन्हित sig = check_and_run_traps();
		अगर (sig != संक_विघ्न)
			जाओ again;
	}

	अगर ((uintptr_t)r > 1) {
		bb_simple_error_संदेश(r);
		r = (अक्षर*)(uintptr_t)1;
	}

	वापस (uintptr_t)r;
}
#पूर्ण_अगर

#अगर ENABLE_HUSH_UMASK
अटल पूर्णांक FAST_FUNC builtin_umask(अक्षर **argv)
{
	पूर्णांक rc;
	mode_t mask;

	rc = 1;
	mask = umask(0);
	argv = skip_dash_dash(argv);
	अगर (argv[0]) {
		mode_t old_mask = mask;

		/* numeric umasks are taken as-is */
		/* symbolic umasks are inverted: "umask a=rx" calls umask(222) */
		अगर (!है_अंक(argv[0][0]))
			mask ^= 0777;
		mask = bb_parse_mode(argv[0], mask);
		अगर (!है_अंक(argv[0][0]))
			mask ^= 0777;
		अगर ((अचिन्हित)mask > 0777) {
			mask = old_mask;
			/* bash messages:
			 * bash: umask: 'q': invalid symbolic mode operator
			 * bash: umask: 999: octal number out of range
			 */
			bb_error_संदेश("%s: invalid mode '%s'", "umask", argv[0]);
			rc = 0;
		}
	} अन्यथा {
		/* Mimic bash */
		म_लिखो("%04o\n", (अचिन्हित) mask);
		/* fall through and restore mask which we set to 0 */
	}
	umask(mask);

	वापस !rc; /* rc != 0 - success */
}
#पूर्ण_अगर

#अगर ENABLE_HUSH_EXPORT || ENABLE_HUSH_READONLY || ENABLE_HUSH_SET || ENABLE_HUSH_TRAP
अटल व्योम print_escaped(स्थिर अक्षर *s)
{
//TODO? bash "set" does not quote variables which contain only alnums and "%+,-./:=@_~",
// (but "export" quotes all variables, even with only these chars).
// I think quoting strings with %+,=~ looks better
// (example: "set" printing var== instead of var='=' looks strange)
// IOW: do not quote "-./:@_": / is used in pathnames, : in PATH, -._ often in file names, @ in emails

	अगर (*s == '\'')
		जाओ squote;
	करो {
		स्थिर अक्षर *p = strchrnul(s, '\'');
		/* print 'xxxx', possibly just '' */
		म_लिखो("'%.*s'", (पूर्णांक)(p - s), s);
		अगर (*p == '\0')
			अवरोध;
		s = p;
 squote:
		/* s points to '; print "'''...'''" */
		अक्षर_दो('"');
		करो अक्षर_दो('\''); जबतक (*++s == '\'');
		अक्षर_दो('"');
	} जबतक (*s);
}
#पूर्ण_अगर

#अगर ENABLE_HUSH_EXPORT || ENABLE_HUSH_LOCAL || ENABLE_HUSH_READONLY
अटल पूर्णांक helper_export_local(अक्षर **argv, अचिन्हित flags)
{
	करो {
		अक्षर *name = *argv;
		स्थिर अक्षर *name_end = endofname(name);

		अगर (*name_end == '\0') {
			काष्ठा variable *var, **vpp;

			vpp = get_ptr_to_local_var(name);
			var = vpp ? *vpp : शून्य;

			अगर (flags & SETFLAG_UNEXPORT) {
				/* export -n NAME (without =VALUE) */
				अगर (var) {
					var->flg_export = 0;
					debug_printf_env("%s: unsetenv '%s'\n", __func__, name);
					unsetenv(name);
				} /* else: export -n NOT_EXISTING_VAR: no-op */
				जारी;
			}
			अगर (flags & SETFLAG_EXPORT) {
				/* export NAME (without =VALUE) */
				अगर (var) {
					var->flg_export = 1;
					debug_printf_env("%s: putenv '%s'\n", __func__, var->varstr);
					putenv(var->varstr);
					जारी;
				}
			}
			अगर (flags & SETFLAG_MAKE_RO) {
				/* readonly NAME (without =VALUE) */
				अगर (var) {
					var->flg_read_only = 1;
					जारी;
				}
			}
# अगर ENABLE_HUSH_LOCAL
			/* Is this "local" bltin? */
			अगर (!(flags & (SETFLAG_EXPORT|SETFLAG_UNEXPORT|SETFLAG_MAKE_RO))) {
				अचिन्हित lvl = flags >> SETFLAG_VARLVL_SHIFT;
				अगर (var && var->var_nest_level == lvl) {
					/* "local x=abc; ...; local x" - ignore second local decl */
					जारी;
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
		} अन्यथा {
			अगर (*name_end != '=') {
				bb_error_संदेश("'%s': bad variable name", name);
				/* do not parse following argv[]s: */
				वापस 1;
			}
			/* (Un)exporting/making local NAME=VALUE */
			name = xstrdup(name);
			/* Testcase: export PS1='\w \$ ' */
			unbackslash(name);
		}
		debug_printf_env("%s: set_local_var('%s')\n", __func__, name);
		अगर (set_local_var(name, flags))
			वापस निकास_त्रुटि;
	} जबतक (*++argv);
	वापस निकास_सफल;
}
#पूर्ण_अगर

#अगर ENABLE_HUSH_EXPORT
अटल पूर्णांक FAST_FUNC builtin_export(अक्षर **argv)
{
	अचिन्हित opt_unexport;

# अगर ENABLE_HUSH_EXPORT_N
	/* "!": do not abort on errors */
	opt_unexport = getopt32(argv, "!n");
	अगर (opt_unexport == (uint32_t)-1)
		वापस निकास_त्रुटि;
	argv += optind;
# अन्यथा
	opt_unexport = 0;
	argv++;
# endif

	अगर (argv[0] == शून्य) {
		अक्षर **e = environ;
		अगर (e) {
			जबतक (*e) {
# अगर 0
				माला_दो(*e++);
# अन्यथा
				/* ash emits: export VAR='VAL'
				 * bash: declare -x VAR="VAL"
				 * we follow ash example */
				स्थिर अक्षर *s = *e++;
				स्थिर अक्षर *p = म_अक्षर(s, '=');

				अगर (!p) /* wtf? take next variable */
					जारी;
				/* "export VAR=" */
				म_लिखो("%s %.*s", "export", (पूर्णांक)(p - s) + 1, s);
				print_escaped(p + 1);
				अक्षर_दो('\n');
# endif
			}
			/*fflush_all(); - done after each builtin anyway */
		}
		वापस निकास_सफल;
	}

	वापस helper_export_local(argv, opt_unexport ? SETFLAG_UNEXPORT : SETFLAG_EXPORT);
}
#पूर्ण_अगर

#अगर ENABLE_HUSH_LOCAL
अटल पूर्णांक FAST_FUNC builtin_local(अक्षर **argv)
{
	अगर (G.func_nest_level == 0) {
		bb_error_संदेश("%s: not in a function", argv[0]);
		वापस निकास_त्रुटि; /* bash compat */
	}
//TODO? ash and bash support "local -" special form,
//which saves/restores $- around function call (including async returns, such as ^C)
//(IOW: it makes "set +/-..." effects local)
	argv++;
	/* Since all builtins run in a nested variable level,
	 * need to use level - 1 here. Or else the variable will be removed at once
	 * after builtin returns.
	 */
	वापस helper_export_local(argv, (G.var_nest_level - 1) << SETFLAG_VARLVL_SHIFT);
}
#पूर्ण_अगर

#अगर ENABLE_HUSH_READONLY
अटल पूर्णांक FAST_FUNC builtin_readonly(अक्षर **argv)
{
	argv++;
	अगर (*argv == शून्य) {
		/* bash: readonly [-p]: list all readonly VARs
		 * (-p has no effect in bash)
		 */
		काष्ठा variable *e;
		क्रम (e = G.शीर्ष_var; e; e = e->अग्र) {
			अगर (e->flg_read_only) {
				स्थिर अक्षर *s = e->varstr;
				स्थिर अक्षर *p = म_अक्षर(s, '=');

				अगर (!p) /* wtf? take next variable */
					जारी;
				/* "readonly VAR=" */
				म_लिखो("%s %.*s", "readonly", (पूर्णांक)(p - s) + 1, s);
				print_escaped(p + 1);
				अक्षर_दो('\n');
			}
		}
		वापस निकास_सफल;
	}
	वापस helper_export_local(argv, SETFLAG_MAKE_RO);
}
#पूर्ण_अगर

#अगर ENABLE_HUSH_UNSET
/* http://www.opengroup.org/onlinepubs/9699919799/utilities/V3_chap02.html#unset */
अटल पूर्णांक FAST_FUNC builtin_unset(अक्षर **argv)
{
	पूर्णांक ret;
	अचिन्हित opts;

	/* "!": do not abort on errors */
	/* "+": stop at 1st non-option */
	opts = getopt32(argv, "!+vf");
	अगर (opts == (अचिन्हित)-1)
		वापस निकास_त्रुटि;
	अगर (opts == 3) {
		bb_simple_error_संदेश("unset: -v and -f are exclusive");
		वापस निकास_त्रुटि;
	}
	argv += optind;

	ret = निकास_सफल;
	जबतक (*argv) {
		अगर (!(opts & 2)) { /* not -f */
			अगर (unset_local_var(*argv)) {
				/* unset <nonexistent_var> doesn't fail.
				 * Error is when one tries to unset RO var.
				 * Message was printed by unset_local_var. */
				ret = निकास_त्रुटि;
			}
		}
# अगर ENABLE_HUSH_FUNCTIONS
		अन्यथा {
			unset_func(*argv);
		}
# endif
		argv++;
	}
	वापस ret;
}
#पूर्ण_अगर

#अगर ENABLE_HUSH_SET
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
अटल पूर्णांक FAST_FUNC builtin_set(अक्षर **argv)
{
	पूर्णांक n;
	अक्षर **pp, **g_argv;
	अक्षर *arg = *++argv;

	अगर (arg == शून्य) {
		काष्ठा variable *e;
		क्रम (e = G.शीर्ष_var; e; e = e->अग्र) {
			स्थिर अक्षर *s = e->varstr;
			स्थिर अक्षर *p = म_अक्षर(s, '=');

			अगर (!p) /* wtf? take next variable */
				जारी;
			/* var= */
			म_लिखो("%.*s", (पूर्णांक)(p - s) + 1, s);
			print_escaped(p + 1);
			अक्षर_दो('\n');
		}
		वापस निकास_सफल;
	}

	करो {
		अगर (म_भेद(arg, "--") == 0) {
			++argv;
			जाओ set_argv;
		}
		अगर (arg[0] != '+' && arg[0] != '-')
			अवरोध;
		क्रम (n = 1; arg[n]; ++n) {
			अगर (set_mode((arg[0] == '-'), arg[n], argv[1])) {
				bb_error_संदेश("%s: %s: invalid option", "set", arg);
				वापस निकास_त्रुटि;
			}
			अगर (arg[n] == 'o' && argv[1])
				argv++;
		}
	} जबतक ((arg = *++argv) != शून्य);
	/* Now argv[0] is 1st argument */

	अगर (arg == शून्य)
		वापस निकास_सफल;
 set_argv:

	/* NB: G.global_argv[0] ($0) is never freed/changed */
	g_argv = G.global_argv;
	अगर (G.global_args_malloced) {
		pp = g_argv;
		जबतक (*++pp)
			मुक्त(*pp);
		g_argv[1] = शून्य;
	} अन्यथा {
		G.global_args_malloced = 1;
		pp = xzalloc(माप(pp[0]) * 2);
		pp[0] = g_argv[0]; /* retain $0 */
		g_argv = pp;
	}
	/* This realloc's G.global_argv */
	G.global_argv = pp = add_strings_to_strings(g_argv, argv, /*dup:*/ 1);

	G.global_argc = 1 + string_array_len(pp + 1);

	वापस निकास_सफल;
}
#पूर्ण_अगर

अटल पूर्णांक FAST_FUNC builtin_shift(अक्षर **argv)
{
	पूर्णांक n = 1;
	argv = skip_dash_dash(argv);
	अगर (argv[0]) {
		n = bb_strtou(argv[0], शून्य, 10);
		अगर (त्रुटि_सं || n < 0) {
			/* shared string with ash.c */
			bb_error_संदेश("Illegal number: %s", argv[0]);
			/*
			 * ash aborts in this case.
			 * bash prints error message and set $? to 1.
			 * Interestingly, for "shift 99999" bash does not
			 * print error message, but does set $? to 1
			 * (and does no shifting at all).
			 */
		}
	}
	अगर (n >= 0 && n < G.global_argc) {
		अगर (G_global_args_malloced) {
			पूर्णांक m = 1;
			जबतक (m <= n)
				मुक्त(G.global_argv[m++]);
		}
		G.global_argc -= n;
		स_हटाओ(&G.global_argv[1], &G.global_argv[n+1],
				G.global_argc * माप(G.global_argv[0]));
		वापस निकास_सफल;
	}
	वापस निकास_त्रुटि;
}

#अगर ENABLE_HUSH_GETOPTS
अटल पूर्णांक FAST_FUNC builtin_getopts(अक्षर **argv)
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
	अक्षर cbuf[2];
	स्थिर अक्षर *cp, *optstring, *var;
	पूर्णांक c, n, exitcode, my_opterr;
	अचिन्हित गिनती;

	optstring = *++argv;
	अगर (!optstring || !(var = *++argv)) {
		bb_simple_error_संदेश("usage: getopts OPTSTRING VAR [ARGS]");
		वापस निकास_त्रुटि;
	}

	अगर (argv[1])
		argv[0] = G.global_argv[0]; /* for error messages in getopt() */
	अन्यथा
		argv = G.global_argv;
	cbuf[1] = '\0';

	my_opterr = 0;
	अगर (optstring[0] != ':') {
		cp = get_local_var_मूल्य("OPTERR");
		/* 0 if "OPTERR=0", 1 otherwise */
		my_opterr = (!cp || NOT_LONE_CHAR(cp, '0'));
	}

	/* getopts stops on first non-option. Add "+" to force that */
	/*if (optstring[0] != '+')*/ {
		अक्षर *s = alloca(म_माप(optstring) + 2);
		प्र_लिखो(s, "+%s", optstring);
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
	गिनती = 0;
	n = string_array_len(argv);
	करो {
		optarg = शून्य;
		opterr = (गिनती < G.getopt_गिनती) ? 0 : my_opterr;
		c = getopt(n, argv, optstring);
		अगर (c < 0)
			अवरोध;
		गिनती++;
	} जबतक (गिनती <= G.getopt_गिनती);

	/* Set OPTIND. Prevent resetting of the magic counter! */
	set_local_var_from_halves("OPTIND", utoa(optind));
	G.getopt_गिनती = गिनती; /* "next time, give me N+1'th result" */
	GETOPT_RESET(); /* just in case */

	/* Set OPTARG */
	/* Always set or unset, never left as-is, even on exit/error:
	 * "If no option was found, or if the option that was found
	 * does not have an option-argument, OPTARG shall be unset."
	 */
	cp = optarg;
	अगर (c == '?') {
		/* If ":optstring" and unknown option is seen,
		 * it is stored to OPTARG.
		 */
		अगर (optstring[1] == ':') {
			cbuf[0] = optopt;
			cp = cbuf;
		}
	}
	अगर (cp)
		set_local_var_from_halves("OPTARG", cp);
	अन्यथा
		unset_local_var("OPTARG");

	/* Convert -1 to "?" */
	exitcode = निकास_सफल;
	अगर (c < 0) { /* -1: end of options */
		exitcode = निकास_त्रुटि;
		c = '?';
	}

	/* Set VAR */
	cbuf[0] = c;
	set_local_var_from_halves(var, cbuf);

	वापस exitcode;
}
#पूर्ण_अगर

अटल पूर्णांक FAST_FUNC builtin_source(अक्षर **argv)
{
	अक्षर *arg_path, *filename;
	HFILE *निवेश;
	save_arg_t sv;
	अक्षर *args_need_save;
#अगर ENABLE_HUSH_FUNCTIONS
	smallint sv_flg;
#पूर्ण_अगर

	argv = skip_dash_dash(argv);
	filename = argv[0];
	अगर (!filename) {
		/* bash says: "bash: .: filename argument required" */
		वापस 2; /* bash compat */
	}
	arg_path = शून्य;
	अगर (!म_अक्षर(filename, '/')) {
		arg_path = find_in_path(filename);
		अगर (arg_path)
			filename = arg_path;
		अन्यथा अगर (!ENABLE_HUSH_BASH_SOURCE_CURDIR) {
			त्रुटि_सं = ENOENT;
			bb_simple_perror_संदेश(filename);
			वापस निकास_त्रुटि;
		}
	}
	निवेश = hfopen(filename);
	मुक्त(arg_path);
	अगर (!निवेश) {
		bb_perror_संदेश("%s", filename);
		/* POSIX: non-interactive shell should abort here,
		 * not merely fail. So far no one complained :)
		 */
		वापस निकास_त्रुटि;
	}

#अगर ENABLE_HUSH_FUNCTIONS
	sv_flg = G_flag_return_in_progress;
	/* "we are inside sourced file, ok to use return" */
	G_flag_return_in_progress = -1;
#पूर्ण_अगर
	args_need_save = argv[1]; /* used as a boolean variable */
	अगर (args_need_save)
		save_and_replace_G_args(&sv, argv);

	/* "false; . ./empty_line; echo Zero:$?" should print 0 */
	G.last_exitcode = 0;
	parse_and_run_file(निवेश);
	hfclose(निवेश);

	अगर (args_need_save) /* can't use argv[1] instead: "shift" can mangle it */
		restore_G_args(&sv, argv);
#अगर ENABLE_HUSH_FUNCTIONS
	G_flag_return_in_progress = sv_flg;
#पूर्ण_अगर

	वापस G.last_exitcode;
}

#अगर ENABLE_HUSH_TRAP
अटल पूर्णांक FAST_FUNC builtin_trap(अक्षर **argv)
{
	पूर्णांक sig;
	अक्षर *new_cmd;

	अगर (!G_traps)
		G_traps = xzalloc(माप(G_traps[0]) * NSIG);

	argv++;
	अगर (!*argv) {
		पूर्णांक i;
		/* No args: print all trapped */
		क्रम (i = 0; i < NSIG; ++i) {
			अगर (G_traps[i]) {
				म_लिखो("trap -- ");
				print_escaped(G_traps[i]);
				/* note: bash adds "SIG", but only if invoked
				 * as "bash". If called as "sh", or if set -o posix,
				 * then it prints short signal names.
				 * We are printing short names: */
				म_लिखो(" %s\n", get_signame(i));
			}
		}
		/*fflush_all(); - done after each builtin anyway */
		वापस निकास_सफल;
	}

	new_cmd = शून्य;
	/* If first arg is a number: reset all specified signals */
	sig = bb_strtou(*argv, शून्य, 10);
	अगर (त्रुटि_सं == 0) {
		पूर्णांक ret;
 process_sig_list:
		ret = निकास_सफल;
		जबतक (*argv) {
			sighandler_t handler;

			sig = get_signum(*argv++);
			अगर (sig < 0) {
				ret = निकास_त्रुटि;
				/* Mimic bash message exactly */
				bb_error_संदेश("trap: %s: invalid signal specification", argv[-1]);
				जारी;
			}

			मुक्त(G_traps[sig]);
			G_traps[sig] = xstrdup(new_cmd);

			debug_printf("trap: setting SIG%s (%i) to '%s'\n",
				get_signame(sig), sig, G_traps[sig]);

			/* There is no signal for 0 (EXIT) */
			अगर (sig == 0)
				जारी;

			अगर (new_cmd)
				handler = (new_cmd[0] ? record_pending_signo : संक_छोड़ो);
			अन्यथा
				/* We are removing trap handler */
				handler = pick_sighandler(sig);
			install_sighandler(sig, handler);
		}
		वापस ret;
	}

	अगर (!argv[1]) { /* no second arg */
		bb_simple_error_संदेश("trap: invalid arguments");
		वापस निकास_त्रुटि;
	}

	/* First arg is "-": reset all specified to default */
	/* First arg is "--": skip it, the rest is "handler SIGs..." */
	/* Everything else: set arg as signal handler
	 * (includes "" case, which ignores signal) */
	अगर (argv[0][0] == '-') {
		अगर (argv[0][1] == '\0') { /* "-" */
			/* new_cmd remains NULL: "reset these sigs" */
			जाओ reset_traps;
		}
		अगर (argv[0][1] == '-' && argv[0][2] == '\0') { /* "--" */
			argv++;
		}
		/* else: "-something", no special meaning */
	}
	new_cmd = *argv;
 reset_traps:
	argv++;
	जाओ process_sig_list;
}
#पूर्ण_अगर

#अगर ENABLE_HUSH_JOB
अटल काष्ठा pipe *parse_jobspec(स्थिर अक्षर *str)
{
	काष्ठा pipe *pi;
	अचिन्हित jobnum;

	अगर (माला_पूछो(str, "%%%u", &jobnum) != 1) {
		अगर (str[0] != '%'
		 || (str[1] != '%' && str[1] != '+' && str[1] != '\0')
		) {
			bb_error_संदेश("bad argument '%s'", str);
			वापस शून्य;
		}
		/* It is "%%", "%+" or "%" - current job */
		jobnum = G.last_jobid;
		अगर (jobnum == 0) {
			bb_simple_error_संदेश("no current job");
			वापस शून्य;
		}
	}
	क्रम (pi = G.job_list; pi; pi = pi->अग्र) {
		अगर (pi->jobid == jobnum) {
			वापस pi;
		}
	}
	bb_error_संदेश("%u: no such job", jobnum);
	वापस शून्य;
}

अटल पूर्णांक FAST_FUNC builtin_jobs(अक्षर **argv UNUSED_PARAM)
{
	काष्ठा pipe *job;
	स्थिर अक्षर *status_string;

	checkjobs(शून्य, 0 /*(no pid to wait for)*/);
	क्रम (job = G.job_list; job; job = job->अग्र) {
		अगर (job->alive_cmds == job->stopped_cmds)
			status_string = "Stopped";
		अन्यथा
			status_string = "Running";

		म_लिखो(JOB_STATUS_FORMAT, job->jobid, status_string, job->cmdtext);
	}

	clean_up_last_dead_job();

	वापस निकास_सफल;
}

/* built-in 'fg' and 'bg' handler */
अटल पूर्णांक FAST_FUNC builtin_fg_bg(अक्षर **argv)
{
	पूर्णांक i;
	काष्ठा pipe *pi;

	अगर (!G_interactive_fd)
		वापस निकास_त्रुटि;

	/* If they gave us no args, assume they want the last backgrounded task */
	अगर (!argv[1]) {
		क्रम (pi = G.job_list; pi; pi = pi->अग्र) {
			अगर (pi->jobid == G.last_jobid) {
				जाओ found;
			}
		}
		bb_error_संदेश("%s: no current job", argv[0]);
		वापस निकास_त्रुटि;
	}

	pi = parse_jobspec(argv[1]);
	अगर (!pi)
		वापस निकास_त्रुटि;
 found:
	/* TODO: bash prints a string representation
	 * of job being foregrounded (like "sleep 1 | cat") */
	अगर (argv[0][0] == 'f' && G_saved_tty_pgrp) {
		/* Put the job into the foreground. */
		tcsetpgrp(G_interactive_fd, pi->pgrp);
	}

	/* Restart the processes in the job */
	debug_printf_jobs("reviving %d procs, pgrp %d\n", pi->अंक_cmds, pi->pgrp);
	क्रम (i = 0; i < pi->अंक_cmds; i++) {
		debug_printf_jobs("reviving pid %d\n", pi->cmds[i].pid);
	}
	pi->stopped_cmds = 0;

	i = समाप्त(- pi->pgrp, SIGCONT);
	अगर (i < 0) {
		अगर (त्रुटि_सं == ESRCH) {
			delete_finished_job(pi);
			वापस निकास_सफल;
		}
		bb_simple_perror_संदेश("kill (SIGCONT)");
	}

	अगर (argv[0][0] == 'f') {
		remove_job_from_table(pi); /* FG job shouldn't be in job table */
		वापस checkjobs_and_fg_shell(pi);
	}
	वापस निकास_सफल;
}
#पूर्ण_अगर

#अगर ENABLE_HUSH_KILL
अटल पूर्णांक FAST_FUNC builtin_kill(अक्षर **argv)
{
	पूर्णांक ret = 0;

# अगर ENABLE_HUSH_JOB
	अगर (argv[1] && म_भेद(argv[1], "-l") != 0) {
		पूर्णांक i = 1;

		करो {
			काष्ठा pipe *pi;
			अक्षर *dst;
			पूर्णांक j, n;

			अगर (argv[i][0] != '%')
				जारी;
			/*
			 * "kill %N" - job kill
			 * Converting to pgrp / pid kill
			 */
			pi = parse_jobspec(argv[i]);
			अगर (!pi) {
				/* Eat bad jobspec */
				j = i;
				करो {
					j++;
					argv[j - 1] = argv[j];
				} जबतक (argv[j]);
				ret = 1;
				i--;
				जारी;
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
			n = G_interactive_fd ? 1 : pi->अंक_cmds;
			dst = alloca(n * माप(पूर्णांक)*4);
			argv[i] = dst;
			अगर (G_interactive_fd)
				dst += प्र_लिखो(dst, " -%u", (पूर्णांक)pi->pgrp);
			अन्यथा क्रम (j = 0; j < n; j++) {
				काष्ठा command *cmd = &pi->cmds[j];
				/* Skip exited members of the job */
				अगर (cmd->pid == 0)
					जारी;
				/*
				 * kill_main has matching code to expect
				 * leading space. Needed to not confuse
				 * negative pids with "kill -SIGNAL_NO" syntax
				 */
				dst += प्र_लिखो(dst, " %u", (पूर्णांक)cmd->pid);
			}
			*dst = '\0';
		} जबतक (argv[++i]);
	}
# endif

	अगर (argv[1] || ret == 0) {
		ret = run_applet_main(argv, kill_main);
	}
	/* else: ret = 1, "kill %bad_jobspec" case */
	वापस ret;
}
#पूर्ण_अगर

#अगर ENABLE_HUSH_WAIT
/* http://www.opengroup.org/onlinepubs/9699919799/utilities/wait.html */
# अगर !ENABLE_HUSH_JOB
#  define wait_for_child_or_signal(pipe,pid) wait_for_child_or_signal(pid)
# endif
अटल पूर्णांक wait_for_child_or_signal(काष्ठा pipe *waitfor_pipe, pid_t waitfor_pid)
{
	पूर्णांक ret = 0;
	क्रम (;;) {
		पूर्णांक sig;
		sigset_t oldset;

		अगर (!sigisemptyset(&G.pending_set))
			जाओ check_sig;

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

		अगर (!sigisemptyset(&G.pending_set)) {
			/* Crap! we raced with some signal! */
			जाओ restore;
		}

		/*errno = 0; - checkjobs does this */
/* Can't pass waitfor_pipe into checkjobs(): it won't be interruptible */
		ret = checkjobs(शून्य, waitfor_pid); /* waitpid(WNOHANG) inside */
		debug_printf_exec("checkjobs:%d\n", ret);
# अगर ENABLE_HUSH_JOB
		अगर (waitfor_pipe) {
			पूर्णांक rcode = job_exited_or_stopped(waitfor_pipe);
			debug_printf_exec("job_exited_or_stopped:%d\n", rcode);
			अगर (rcode >= 0) {
				ret = rcode;
				sigprocmask(SIG_SETMASK, &oldset, शून्य);
				अवरोध;
			}
		}
# endif
		/* if ECHILD, there are no children (ret is -1 or 0) */
		/* if ret == 0, no children changed state */
		/* if ret != 0, it's exitcode+1 of exited waitfor_pid child */
		अगर (त्रुटि_सं == ECHILD || ret) {
			ret--;
			अगर (ret < 0) /* if ECHILD, may need to fix "ret" */
				ret = 0;
# अगर ENABLE_HUSH_BASH_COMPAT
			अगर (waitfor_pid == -1 && त्रुटि_सं == ECHILD) {
				/* exitcode of "wait -n" with no children is 127, not 0 */
				ret = 127;
			}
# endif
			sigprocmask(SIG_SETMASK, &oldset, शून्य);
			अवरोध;
		}
		/* Wait for SIGCHLD or any other signal */
		/* It is vitally important for sigsuspend that SIGCHLD has non-DFL handler! */
		/* Note: sigsuspend invokes signal handler */
		संक_रोको(&oldset);
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
		sigprocmask(SIG_SETMASK, &oldset, शून्य);
 check_sig:
		/* So, did we get a signal? */
		sig = check_and_run_traps();
		अगर (sig /*&& sig != SIGCHLD - always true */) {
			/* Do this for any (non-ignored) signal, not only for ^C */
			ret = 128 | sig;
			अवरोध;
		}
		/* SIGCHLD, or no signal, or ignored one, such as SIGQUIT. Repeat */
	}
	वापस ret;
}

अटल पूर्णांक FAST_FUNC builtin_wait(अक्षर **argv)
{
	पूर्णांक ret;
	पूर्णांक status;

	argv = skip_dash_dash(argv);
# अगर ENABLE_HUSH_BASH_COMPAT
	अगर (argv[0] && म_भेद(argv[0], "-n") == 0) {
		/* wait -n */
		/* (bash accepts "wait -n PID" too and ignores PID) */
		G.dead_job_exitcode = -1;
		वापस wait_for_child_or_signal(शून्य, -1 /*no job, wait for one job*/);
	}
# endif
	अगर (argv[0] == शून्य) {
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
		वापस wait_for_child_or_signal(शून्य, 0 /*no job and no pid to wait for*/);
	}

	करो {
		pid_t pid = bb_strtou(*argv, शून्य, 10);
		अगर (त्रुटि_सं || pid <= 0) {
# अगर ENABLE_HUSH_JOB
			अगर (argv[0][0] == '%') {
				काष्ठा pipe *wait_pipe;
				ret = 127; /* bash compat for bad jobspecs */
				wait_pipe = parse_jobspec(*argv);
				अगर (wait_pipe) {
					ret = job_exited_or_stopped(wait_pipe);
					अगर (ret < 0) {
						ret = wait_for_child_or_signal(wait_pipe, 0);
					} अन्यथा {
						/* waiting on "last dead job" removes it */
						clean_up_last_dead_job();
					}
				}
				/* else: parse_jobspec() already emitted error msg */
				जारी;
			}
# endif
			/* mimic bash message */
			bb_error_संदेश("wait: '%s': not a pid or valid job spec", *argv);
			ret = निकास_त्रुटि;
			जारी; /* bash checks all argv[] */
		}

		/* Do we have such child? */
		ret = waitpid(pid, &status, WNOHANG);
		अगर (ret < 0) {
			/* No */
			ret = 127;
			अगर (त्रुटि_सं == ECHILD) {
				अगर (pid == G.last_bg_pid) {
					/* "wait $!" but last bg task has already exited. Try:
					 * (sleep 1; exit 3) & sleep 2; echo $?; wait $!; echo $?
					 * In bash it prints exitcode 0, then 3.
					 * In dash, it is 127.
					 */
					ret = G.last_bg_pid_exitcode;
				} अन्यथा {
					/* Example: "wait 1". mimic bash message */
					bb_error_संदेश("wait: pid %u is not a child of this shell", (अचिन्हित)pid);
				}
			} अन्यथा {
				/* ??? */
				bb_perror_संदेश("wait %s", *argv);
			}
			जारी; /* bash checks all argv[] */
		}
		अगर (ret == 0) {
			/* Yes, and it still runs */
			ret = wait_for_child_or_signal(शून्य, pid);
		} अन्यथा {
			/* Yes, and it just exited */
			process_wait_परिणाम(शून्य, pid, status);
			ret = WEXITSTATUS(status);
			अगर (WIFSIGNALED(status))
				ret = 128 | WTERMSIG(status);
		}
	} जबतक (*++argv);

	वापस ret;
}
#पूर्ण_अगर

#अगर ENABLE_HUSH_LOOPS || ENABLE_HUSH_FUNCTIONS
अटल अचिन्हित parse_numeric_argv1(अक्षर **argv, अचिन्हित def, अचिन्हित def_min)
{
	अगर (argv[1]) {
		def = bb_strtou(argv[1], शून्य, 10);
		अगर (त्रुटि_सं || def < def_min || argv[2]) {
			bb_error_संदेश("%s: bad arguments", argv[0]);
			def = अच_पूर्णांक_उच्च;
		}
	}
	वापस def;
}
#पूर्ण_अगर

#अगर ENABLE_HUSH_LOOPS
अटल पूर्णांक FAST_FUNC builtin_break(अक्षर **argv)
{
	अचिन्हित depth;
	अगर (G.depth_of_loop == 0) {
		bb_error_संदेश("%s: only meaningful in a loop", argv[0]);
		/* if we came from builtin_continue(), need to undo "= 1" */
		G.flag_break_continue = 0;
		वापस निकास_सफल; /* bash compat */
	}
	G.flag_break_continue++; /* BC_BREAK = 1, or BC_CONTINUE = 2 */

	G.depth_break_continue = depth = parse_numeric_argv1(argv, 1, 1);
	अगर (depth == अच_पूर्णांक_उच्च)
		G.flag_break_continue = BC_BREAK;
	अगर (G.depth_of_loop < depth)
		G.depth_break_continue = G.depth_of_loop;

	वापस निकास_सफल;
}

अटल पूर्णांक FAST_FUNC builtin_continue(अक्षर **argv)
{
	G.flag_break_continue = 1; /* BC_CONTINUE = 2 = 1+1 */
	वापस builtin_break(argv);
}
#पूर्ण_अगर

#अगर ENABLE_HUSH_FUNCTIONS
अटल पूर्णांक FAST_FUNC builtin_return(अक्षर **argv)
{
	पूर्णांक rc;

	अगर (G_flag_return_in_progress != -1) {
		bb_error_संदेश("%s: not in a function or sourced script", argv[0]);
		वापस निकास_त्रुटि; /* bash compat */
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
# अगर ENABLE_HUSH_TRAP
	अगर (argv[1]) { /* "return ARG" inside a running trap sets $? */
		debug_printf_exec("G.return_exitcode=%d\n", rc);
		G.return_exitcode = rc;
	}
# endif
	वापस rc;
}
#पूर्ण_अगर

#अगर ENABLE_HUSH_TIMES
अटल पूर्णांक FAST_FUNC builtin_times(अक्षर **argv UNUSED_PARAM)
{
	अटल स्थिर uint8_t times_tbl[] ALIGN1 = {
		' ',  दुरत्व(काष्ठा tms, tms_utime),
		'\n', दुरत्व(काष्ठा tms, tms_stime),
		' ',  दुरत्व(काष्ठा tms, tms_cutime),
		'\n', दुरत्व(काष्ठा tms, tms_cstime),
		0
	};
	स्थिर uint8_t *p;
	अचिन्हित clk_tck;
	काष्ठा tms buf;

	clk_tck = bb_clk_tck();

	बार(&buf);
	p = times_tbl;
	करो {
		अचिन्हित sec, frac;
		अचिन्हित दीर्घ t;
		t = *(घड़ी_प्रकार *)(((अक्षर *) &buf) + p[1]);
		sec = t / clk_tck;
		frac = t % clk_tck;
		म_लिखो("%um%u.%03us%c",
			sec / 60, sec % 60,
			(frac * 1000) / clk_tck,
			p[0]);
		p += 2;
	} जबतक (*p);

	वापस निकास_सफल;
}
#पूर्ण_अगर

#अगर ENABLE_HUSH_MEMLEAK
अटल पूर्णांक FAST_FUNC builtin_memleak(अक्षर **argv UNUSED_PARAM)
{
	व्योम *p;
	अचिन्हित दीर्घ l;

# ifdef M_TRIM_THRESHOLD
	/* Optional. Reduces probability of false positives */
	malloc_trim(0);
# endif
	/* Crude attempt to find where "free memory" starts,
	 * sans fragmentation. */
	p = दो_स्मृति(240);
	l = (अचिन्हित दीर्घ)p;
	मुक्त(p);
	p = दो_स्मृति(3400);
	अगर (l < (अचिन्हित दीर्घ)p) l = (अचिन्हित दीर्घ)p;
	मुक्त(p);

# अगर 0  /* debug */
	{
		काष्ठा mallinfo mi = mallinfo();
		म_लिखो("top alloc:0x%lx malloced:%d+%d=%d\n", l,
			mi.arena, mi.hblkhd, mi.arena + mi.hblkhd);
	}
# endif

	अगर (!G.memleak_मूल्य)
		G.memleak_मूल्य = l;

	l -= G.memleak_मूल्य;
	अगर ((दीर्घ)l < 0)
		l = 0;
	l /= 1024;
	अगर (l > 127)
		l = 127;

	/* Exitcode is "how many kilobytes we leaked since 1st call" */
	वापस l;
}
#पूर्ण_अगर
#पूर्ण_अगर /* !__U_BOOT__ */
