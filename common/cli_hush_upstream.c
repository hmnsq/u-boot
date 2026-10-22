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

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
#ਜੇ !(defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) \
	|| defined(__APPLE__) \
    )
# include <malloc.h>   /* for malloc_trim */
#ਜੇ_ਅੰਤ
#ਅਨ੍ਤਰ੍ਭਾਵ <glob.h>
/* #include <dmalloc.h> */
#ਜੇ ENABLE_HUSH_CASE
# include <fnmatch.h>
#ਜੇ_ਅੰਤ
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/times.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/utsname.h> /* for setting $HOSTNAME */

#ਅਨ੍ਤਰ੍ਭਾਵ "busybox.h"  /* for APPLET_IS_NOFORK/NOEXEC */
#ਅਨ੍ਤਰ੍ਭਾਵ "unicode.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "shell_common.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "math.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "match.h"
#ਜੇ ENABLE_HUSH_RANDOM_SUPPORT
# include "random.h"
#ਨਹੀਂਤਾਂ
# define CLEAR_RANDOM_T(rnd) ((ਖਾਲੀ)0)
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ O_CLOEXEC
# define O_CLOEXEC 0
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ F_DUPFD_CLOEXEC
# define F_DUPFD_CLOEXEC F_DUPFD
#ਜੇ_ਅੰਤ

#ਜੇ ENABLE_FEATURE_SH_EMBEDDED_SCRIPTS && !ENABLE_SHELL_ASH
# include "embedded_scripts.h"
#ਨਹੀਂਤਾਂ
# define ਅੰਕ_SCRIPTS 0
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */

/* So far, all bash compat is controlled by one config option */
/* Separate defines document which part of code implements what */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BASH_ਨਮੂਨਾ_SUBST ENABLE_HUSH_BASH_COMPAT
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BASH_SUBSTR        ENABLE_HUSH_BASH_COMPAT
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BASH_ਸਰੋਤ        ENABLE_HUSH_BASH_COMPAT
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BASH_DOLLAR_SQUOTE ENABLE_HUSH_BASH_COMPAT
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BASH_HOSTNAME_VAR  ENABLE_HUSH_BASH_COMPAT
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BASH_EPOCH_VARS    ENABLE_HUSH_BASH_COMPAT
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BASH_TEST2         (ENABLE_HUSH_BASH_COMPAT && ENABLE_HUSH_TEST)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BASH_READ_D        ENABLE_HUSH_BASH_COMPAT

/* Build knobs */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ LEAK_HUNTING 0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BUILD_AS_NOMMU 0
/* Enable/disable sanity checks. Ok to enable in production,
 * only adds a bit of bloat. Set to >1 to get non-production level verbosity.
 * Keeping 1 for now even in released versions.
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HUSH_DEBUG 1
/* Slightly bigger (+200 bytes), but faster hush.
 * So far it only enables a trick with counting SIGCHLDs and forks,
 * which allows us to do fewer waitpid's.
 * (we can detect a case where neither forks were done nor SIGCHLDs happened
 * and therefore waitpid will return the same result as last time)
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ENABLE_HUSH_FAST 0
/* TODO: implement simplified code for users which do not need ${var%...} ops
 * So far ${var%...} ops are always enabled:
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ENABLE_HUSH_DOLLAR_OPS 1

#ਜੇ BUILD_AS_NOMMU
# undef BB_MMU
# undef USE_FOR_NOMMU
# undef USE_FOR_MMU
# define BB_MMU 0
# define USE_FOR_NOMMU(...) __VA_ARGS__
# define USE_FOR_MMU(...)
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
#ਅਨ੍ਤਰ੍ਭਾਵ "NUM_APPLETS.h"
#ਜੇ ਅੰਕ_APPLETS == 1
/* STANDALONE does not make sense, and won't compile */
# undef ENABLE_FEATURE_SH_STANDALONE
# undef IF_FEATURE_SH_STANDALONE
# undef IF_NOT_FEATURE_SH_STANDALONE
# define ENABLE_FEATURE_SH_STANDALONE 0
# define IF_FEATURE_SH_STANDALONE(...)
# define IF_NOT_FEATURE_SH_STANDALONE(...) __VA_ARGS__
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ /* __U_BOOT__ */

#ਜੇ !ENABLE_HUSH_INTERACTIVE
# undef ENABLE_FEATURE_EDITING
# define ENABLE_FEATURE_EDITING 0
# undef ENABLE_FEATURE_EDITING_FANCY_PROMPT
# define ENABLE_FEATURE_EDITING_FANCY_PROMPT 0
# undef ENABLE_FEATURE_EDITING_SAVE_ON_EXIT
# define ENABLE_FEATURE_EDITING_SAVE_ON_EXIT 0
#ਜੇ_ਅੰਤ

/* Do we support ANY keywords? */
#ਜੇ ENABLE_HUSH_IF || ENABLE_HUSH_LOOPS || ENABLE_HUSH_CASE
# define HAS_KEYWORDS 1
# define IF_HAS_KEYWORDS(...) __VA_ARGS__
# define IF_HAS_NO_KEYWORDS(...)
#ਨਹੀਂਤਾਂ
# define HAS_KEYWORDS 0
# define IF_HAS_KEYWORDS(...)
# define IF_HAS_NO_KEYWORDS(...) __VA_ARGS__
#ਜੇ_ਅੰਤ

/* If you comment out one of these below, it will be #defined later
 * to perform debug printfs to stderr: */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ debug_printf(...)         ਕਰੋ {} ਜਦੋਂ (0)
/* Finer-grained debug switches */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ debug_printf_parse(...)   ਕਰੋ {} ਜਦੋਂ (0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ debug_printf_heredoc(...) ਕਰੋ {} ਜਦੋਂ (0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ debug_print_ਰੁੱਖ(a, b)    ਕਰੋ {} ਜਦੋਂ (0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ debug_printf_exec(...)    ਕਰੋ {} ਜਦੋਂ (0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ debug_printf_env(...)     ਕਰੋ {} ਜਦੋਂ (0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ debug_printf_jobs(...)    ਕਰੋ {} ਜਦੋਂ (0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ debug_printf_expand(...)  ਕਰੋ {} ਜਦੋਂ (0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ debug_printf_varexp(...)  ਕਰੋ {} ਜਦੋਂ (0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ debug_printf_glob(...)    ਕਰੋ {} ਜਦੋਂ (0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ debug_printf_redir(...)   ਕਰੋ {} ਜਦੋਂ (0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ debug_printf_list(...)    ਕਰੋ {} ਜਦੋਂ (0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ debug_printf_subst(...)   ਕਰੋ {} ਜਦੋਂ (0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ debug_printf_prompt(...)  ਕਰੋ {} ਜਦੋਂ (0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ debug_printf_clean(...)   ਕਰੋ {} ਜਦੋਂ (0)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ERR_PTR ((ਖਾਲੀ*)(ਲੰਮਾ)1)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਕੰਮ_ਸਥਿਤੀ_FORMAT    "[%u] %-22s %.40s\n"

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ _SPECIAL_VARS_STR     "_*@$!?#-"
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SPECIAL_VARS_STR     ("_*@$!?#-" + 1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NUMERIC_SPECVARS_STR ("_*@$!?#-" + 3)
#ਨਹੀਂਤਾਂ /* __U_BOOT__ */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SPECIAL_VARS_STR     "*@$!?#-"
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NUMERIC_SPECVARS_STR "$!?#-"
#ਜੇ_ਅੰਤ /* __U_BOOT__ */
#ਜੇ BASH_ਨਮੂਨਾ_SUBST
/* Support / and // replace ops */
/* Note that // is stored as \ in "encoded" string representation */
# define VAR_ENCODED_SUBST_OPS      "\\/%#:-=+?"
# define VAR_SUBST_OPS             ("\\/%#:-=+?" + 1)
# define MINUS_PLUS_EQUAL_QUESTION ("\\/%#:-=+?" + 5)
#ਨਹੀਂਤਾਂ
# define VAR_ENCODED_SUBST_OPS      "%#:-=+?"
# define VAR_SUBST_OPS              "%#:-=+?"
# define MINUS_PLUS_EQUAL_QUESTION ("%#:-=+?" + 3)
#ਜੇ_ਅੰਤ

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SPECIAL_VAR_ਚਿੰਨ੍ਹ_STR "\3"
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SPECIAL_VAR_ਚਿੰਨ੍ਹ       3
/* The "variable" with name "\1" emits string "\3". Testcase: "echo ^C" */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SPECIAL_VAR_QUOTED_SVS   1

ਬਣਤਰ variable;

ਸਥਿਰ ਅਚਲ ਅੱਖਰ hush_ਸੰਸਕਰਣ_str[] ALIGN1 = "HUSH_VERSION="BB_VER;

/* This supports saving pointers malloced in vfork child,
 * to be freed in the parent.
 */
#ਜੇ !BB_MMU
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ nommu_save_t {
	ਬਣਤਰ variable *old_vars;
	ਅੱਖਰ **argv;
	ਅੱਖਰ **argv_from_re_execing;
} nommu_save_t;
#ਜੇ_ਅੰਤ

ਗਿਣਤੀ {
	RES_NONE  = 0,
#ਜੇ ENABLE_HUSH_IF
	RES_IF    ,
	RES_THEN  ,
	RES_ELIF  ,
	RES_ELSE  ,
	RES_FI    ,
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_HUSH_LOOPS
	RES_FOR   ,
	RES_WHILE ,
	RES_UNTIL ,
	RES_DO    ,
	RES_ਪੂਰਾ  ,
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_HUSH_LOOPS || ENABLE_HUSH_CASE
	RES_ਵਿੱਚ    ,
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_HUSH_CASE
	RES_CASE  ,
	/* three pseudo-keywords support contrived "case" syntax: */
	RES_CASE_ਵਿੱਚ,   /* "case ... IN", turns into RES_MATCH when IN is observed */
	RES_ਮੇਲ ,    /* "word)" */
	RES_CASE_BODY, /* "this command is inside CASE" */
	RES_ESAC  ,
#ਜੇ_ਅੰਤ
	RES_XXXX  ,
	RES_SNTX
};

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ o_string {
	ਅੱਖਰ *ਅੰਕੜੇ;
	ਪੂਰਨ ਲੰਮਾਈ; /* position where data is appended */
	ਪੂਰਨ maxlen;
	ਪੂਰਨ o_expflags;
	/* At least some part of the string was inside '' or "",
	 * possibly empty one: word"", wo''rd etc. */
	smallint has_quoted_part;
	smallint has_ਖਾਲੀ_ਥਾਵਾਂ_ਥਾਂ;
	smallint ended_ਵਿੱਚ_ifs;
} o_string;
ਗਿਣਤੀ {
	EXP_FLAG_SINGLEWORD     = 0x80, /* must be 0x80 */
	EXP_FLAG_GLOB           = 0x2,
	/* Protect newly added chars against globbing
	 * by prepending \ to *, ?, [, \ */
	EXP_FLAG_ESC_GLOB_CHARS = 0x1,
};
/* Used for initialization: o_string foo = NULL_O_STRING; */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NULL_O_STRING { ਨਿਰਾ }

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ debug_printf_parse
ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ assignment_flag[] ALIGN_PTR = {
	"MAYBE_ASSIGNMENT",
	"DEFINITELY_ASSIGNMENT",
	"NOT_ASSIGNMENT",
	"WORD_IS_KEYWORD",
};
#ਜੇ_ਅੰਤ

/* We almost can use standard FILE api, but we need an ability to move
 * its fd when redirects coincide with it. No api exists for that
 * (RFE for it at https://sourceware.org/bugzilla/show_bug.cgi?id=21902).
 * HFILE is our internal alternative. Only supports reading.
 * Since we now can, we incorporate linked list of all opened HFILEs
 * into the struct (used to be a separate mini-list).
 */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ HFILE {
	ਅੱਖਰ *cur;
	ਅੱਖਰ *ਸਿਰਾ_2;
	ਬਣਤਰ HFILE *ਅਗਲਾ_hfile;
	ਪੂਰਨ fd;
	ਅੱਖਰ buf[1024];
} HFILE;

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ ਵਿੱਚ_str {
	ਅਚਲ ਅੱਖਰ *p;
	ਪੂਰਨ peek_buf[2];
	ਪੂਰਨ ਆਖਰੀ_char;
	HFILE *file;
} ਵਿੱਚ_str;

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
/* The descrip member of this structure is only used to make
 * debugging output pretty */
ਸਥਿਰ ਅਚਲ ਬਣਤਰ {
	int32_t ਬਹੁਲਕ;
	ਨਿਸ਼ਾਨੀਵਾਲਾ ਅੱਖਰ default_fd;
	ਅੱਖਰ descrip[3];
} redir_ਸਾਰਣੀ[] ALIGN4 = {
	{ O_RDONLY,                  0, "<"  },
	{ O_CREAT|O_TRUNC|O_WRONLY,  1, ">"  },
	{ O_CREAT|O_APPEND|O_WRONLY, 1, ">>" },
	{ O_CREAT|O_RDWR,            1, "<>" },
	{ O_RDONLY,                  0, "<<" },
/* Should not be needed. Bogus default_fd helps in debugging */
/*	{ O_RDONLY,                 77, "<<" }, */
};

ਬਣਤਰ redir_struct {
	ਬਣਤਰ redir_struct *ਅਗਲਾ;
	ਅੱਖਰ *rd_filename;          /* filename */
	ਪੂਰਨ rd_fd;                  /* fd to redirect */
	/* fd to redirect to, or -3 if rd_fd is to be closed (n>&-) */
	ਪੂਰਨ rd_dup;
	smallint rd_type;           /* (enum redir_type) */
	/* note: for heredocs, rd_filename contains heredoc delimiter,
	 * and subsequently heredoc itself; and rd_dup is a bitmask:
	 * bit 0: do we need to trim leading tabs?
	 * bit 1: is heredoc quoted (<<'delim' syntax) ?
	 */
};
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਗਿਣਤੀ redir_type {
	REDIRECT_ਨਿਵੇਸ਼     = 0,
	REDIRECT_OVERWRITE = 1,
	REDIRECT_APPEND    = 2,
	REDIRECT_IO        = 3,
	REDIRECT_HEREDOC   = 4,
	REDIRECT_HEREDOC2  = 5, /* REDIRECT_HEREDOC after heredoc is loaded */

	REDIRFD_CLOSE      = -3,
	REDIRFD_SYNTAX_ERR = -2,
	REDIRFD_ਮੰਜ਼ਿਲ_FILE    = -1,
	/* otherwise, rd_fd is redirected to rd_dup */

	HEREDOC_SKIPTABS = 1,
	HEREDOC_QUOTED   = 2,
} redir_type;

#ਜੇ_ਅੰਤ /* !__U_BOOT__ */

ਬਣਤਰ command {
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	pid_t pid;                  /* 0 if exited */
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */
	ਬਿਨਾਨਿਸ਼ਾਨੀ assignment_cnt;    /* how many argv[i] are assignments? */
#ਜੇ ENABLE_HUSH_LINENO_VAR
	ਬਿਨਾਨਿਸ਼ਾਨੀ lineno;
#ਜੇ_ਅੰਤ
	smallint cmd_type;          /* CMD_xxx */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CMD_ਸਾਧਾਰਨ   0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CMD_SUBSHELL 1
#ਜੇ BASH_TEST2
/* used for "[[ EXPR ]]" */
# define CMD_TEST2_SINGLEWORD_NOGLOB 2
#ਜੇ_ਅੰਤ
#ਜੇ BASH_TEST2 || ENABLE_HUSH_LOCAL || ENABLE_HUSH_EXPORT || ENABLE_HUSH_READONLY
/* used to prevent word splitting and globbing in "export v=t*" */
# define CMD_SINGLEWORD_NOGLOB 3
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_HUSH_FUNCTIONS
# define CMD_FUNCDEF 4
#ਜੇ_ਅੰਤ

	smalluint cmd_exitcode;
	/* if non-NULL, this "command" is { list }, ( list ), or a compound statement */
	ਬਣਤਰ pipe *ਸਮੂਹ;
#ਜੇ !BB_MMU
	ਅੱਖਰ *ਸਮੂਹ_as_string;
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_HUSH_FUNCTIONS
	ਬਣਤਰ function *ਬੱਚਾ_func;
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
#ਜੇ_ਅੰਤ
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	ਪੂਰਨ argc; /* number of program arguments */
#ਜੇ_ਅੰਤ
	ਅੱਖਰ **argv;                /* command name and arguments */
/* argv vector may contain variable references (^Cvar^C, ^C0^C etc)
 * and on execution these are substituted with their values.
 * Substitution can make _several_ words out of one argv[n]!
 * Example: argv[0]=='.^C*^C.' here: echo .$*.
 * References of the form ^C`cmd arg^C are `cmd arg` substitutions.
 */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	ਬਣਤਰ redir_struct *redirects; /* I/O redirections */
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */
};
/* Is there anything in this command at all? */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ IS_NULL_CMD(cmd) \
	(!(cmd)->ਸਮੂਹ && !(cmd)->argv && !(cmd)->redirects)

#ਨਹੀਂਤਾਂ /* __U_BOOT__ */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ IS_NULL_CMD(cmd) \
	(!(cmd)->ਸਮੂਹ && !(cmd)->argv)
#ਜੇ_ਅੰਤ /* __U_BOOT__ */
ਬਣਤਰ pipe {
	ਬਣਤਰ pipe *ਅਗਲਾ;
	ਪੂਰਨ ਅੰਕ_cmds;               /* total number of commands in pipe */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	ਪੂਰਨ alive_cmds;             /* number of commands running (not exited) */
	ਪੂਰਨ stopped_cmds;           /* number of commands alive, but stopped */
#ਜੇ ENABLE_HUSH_ਕੰਮ
	ਬਿਨਾਨਿਸ਼ਾਨੀ jobid;             /* job number */
	pid_t pgrp;                 /* process group ID for the job */
	ਅੱਖਰ *cmdtext;              /* name of job */
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */
	ਬਣਤਰ command *cmds;       /* array of commands in pipe */
	smallint followup;          /* PIPE_BG, PIPE_SEQ, PIPE_OR, PIPE_AND */
	IF_HAS_KEYWORDS(smallint pi_inverted;) /* "! cmd | cmd" */
	IF_HAS_KEYWORDS(smallint res_ਸ਼ਬਦ;) /* needed for if, for, while, until... */
};
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਗਿਣਤੀ pipe_style {
	PIPE_SEQ = 0,
	PIPE_AND = 1,
	PIPE_OR  = 2,
	PIPE_BG  = 3,
} pipe_style;
/* Is there anything in this pipe at all? */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ IS_NULL_PIPE(pi) \
	((pi)->ਅੰਕ_cmds == 0 IF_HAS_KEYWORDS( && (pi)->res_ਸ਼ਬਦ == RES_NONE))

/* This holds pointers to the various results of parsing */
ਬਣਤਰ parse_context {
	/* linked list of pipes */
	ਬਣਤਰ pipe *list_ਸਿਰ;
	/* last pipe (being constructed right now) */
	ਬਣਤਰ pipe *pipe;
	/* last command in pipe (being constructed right now) */
	ਬਣਤਰ command *command;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	/* last redirect in command->redirects list */
	ਬਣਤਰ redir_struct *pending_redirect;
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */
	o_string ਸ਼ਬਦ;
#ਜੇ !BB_MMU
	o_string as_string;
#ਜੇ_ਅੰਤ
	smallint is_assignment; /* 0:maybe, 1:yes, 2:no, 3:keyword */
#ਜੇ HAS_KEYWORDS
	smallint ctx_res_w;
	smallint ctx_inverted; /* "! cmd | cmd" */
#ਜੇ ENABLE_HUSH_CASE
	smallint ctx_dsemicolon; /* ";;" seen */
#ਜੇ_ਅੰਤ
	/* bitmask of FLAG_xxx, for figuring out valid reserved words */
	ਪੂਰਨ old_flag;
	/* group we are enclosed in:
	 * example: "if pipe1; pipe2; then pipe3; fi"
	 * when we see "if" or "then", we malloc and copy current context,
	 * and make ->stack point to it. then we parse pipeN.
	 * when closing "then" / fi" / whatever is found,
	 * we move list_head into ->stack->command->group,
	 * copy ->stack into current context, and delete ->stack.
	 * (parsing of { list } and ( list ) doesn't use this method)
	 */
	ਬਣਤਰ parse_context *ਢੇਰ;
#ਜੇ_ਅੰਤ
};
ਗਿਣਤੀ {
	MAYBE_ASSIGNMENT      = 0,
	DEFINITELY_ASSIGNMENT = 1,
	NOT_ASSIGNMENT        = 2,
	/* Not an assignment, but next word may be: "if v=xyz cmd;" */
	ਸ਼ਬਦ_IS_KEYWORD       = 3,
};

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
/* On program start, environ points to initial environment.
 * putenv adds new pointers into it, unsetenv removes them.
 * Neither of these (de)allocates the strings.
 * setenv allocates new strings in malloc space and does putenv,
 * and thus setenv is unusable (leaky) for shell's purposes */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ setenv(...) setenv_is_leaky_dont_use()
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */
ਬਣਤਰ variable {
	ਬਣਤਰ variable *ਅਗਲਾ;
	ਅੱਖਰ *varstr;        /* points to "name=" portion */
	ਪੂਰਨ max_len;         /* if > 0, name is part of initial env; else name is malloced */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	uint16_t var_nest_level;
	smallint flg_export; /* putenv should be done on this var */
	smallint flg_read_only;
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */
};

ਗਿਣਤੀ {
	BC_BREAK = 1,
	BC_CONTINUE = 2,
};

#ਜੇ ENABLE_HUSH_FUNCTIONS
ਬਣਤਰ function {
	ਬਣਤਰ function *ਅਗਲਾ;
	ਅੱਖਰ *name;
	ਬਣਤਰ command *ਮਾਪਾ_cmd;
	ਬਣਤਰ pipe *body;
# ਜੇ !BB_MMU
	ਅੱਖਰ *body_as_string;
# endif
};
#ਜੇ_ਅੰਤ

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
ਸਥਿਰ ਅਚਲ ਅੱਖਰ o_opt_ਲੜੀਆਂ[] ALIGN1 =
	"pipefail\0"
	"noexec\0"
	"errexit\0"
#ਜੇ ENABLE_HUSH_ਬਹੁਲਕ_X
	"xtrace\0"
#ਜੇ_ਅੰਤ
	;
ਗਿਣਤੀ {
	OPT_O_PIPEFAIL,
	OPT_O_NOEXEC,
	OPT_O_ERREXIT,
#ਜੇ ENABLE_HUSH_ਬਹੁਲਕ_X
	OPT_O_XTRACE,
#ਜੇ_ਅੰਤ
	ਅੰਕ_OPT_O
};

/* "Globals" within this file */
/* Sorted roughly by size (smaller offsets == smaller code) */
ਬਣਤਰ globals {
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
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
#ਜੇ ENABLE_HUSH_INTERACTIVE
	/* 'interactive_fd' is a fd# open to ctty, if we have one
	 * _AND_ if we decided to act interactively */
	ਪੂਰਨ interactive_fd;
	IF_NOT_FEATURE_EDITING_FANCY_PROMPT(ਅੱਖਰ *PS1;)
# define G_interactive_fd (G.interactive_fd)
#ਨਹੀਂਤਾਂ
# define G_interactive_fd 0
#ਜੇ_ਅੰਤ
#ਨਹੀਂਤਾਂ /* __U_BOOT__ */
# define G_interactive_fd 0
#ਜੇ_ਅੰਤ /* __U_BOOT__ */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
#ਜੇ ENABLE_FEATURE_EDITING
	ਸਤਰ_ਨਿਵੇਸ਼_t *ਸਤਰ_ਨਿਵੇਸ਼_state;
#ਜੇ_ਅੰਤ
	pid_t ਜੜ੍ਹ_pid;
	pid_t ਜੜ੍ਹ_ppid;
	pid_t ਆਖਰੀ_bg_pid;
#ਜੇ ENABLE_HUSH_RANDOM_SUPPORT
	random_t random_gen;
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_HUSH_ਕੰਮ
	ਪੂਰਨ ਲੜੀਵਾਰ_list_level;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਆਖਰੀ_jobid;
	pid_t saved_tty_pgrp;
	ਬਣਤਰ pipe *ਕੰਮ_list;
# define G_saved_tty_pgrp (G.saved_tty_pgrp)
#ਨਹੀਂਤਾਂ
# define G_saved_tty_pgrp 0
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */
	/* How deeply are we in context where "set -e" is ignored */
	ਪੂਰਨ errexit_ਡੂੰਘਾਈ;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
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

	ਅੱਖਰ o_opt[ਅੰਕ_OPT_O];
#ਜੇ ENABLE_HUSH_ਬਹੁਲਕ_X
# define G_x_ਬਹੁਲਕ (G.o_opt[OPT_O_XTRACE])
#ਨਹੀਂਤਾਂ
# define G_x_ਬਹੁਲਕ 0
#ਜੇ_ਅੰਤ
	ਅੱਖਰ opt_s;
	ਅੱਖਰ opt_c;
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */
#ਜੇ ENABLE_HUSH_INTERACTIVE
	smallint promptmode; /* 0: PS1, 1: PS2 */
#ਜੇ_ਅੰਤ
	/* set by signal handler if SIGINT is received _and_ its trap is not set */
	smallint flag_SIGINT;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
#ਜੇ ENABLE_HUSH_LOOPS
	smallint flag_break_continue;
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */
#ਜੇ ENABLE_HUSH_FUNCTIONS
	/* 0: outside of a function (or sourced file)
	 * -1: inside of a function, ok to use return builtin
	 * 1: return is invoked, skip all till end of func
	 */
	smallint flag_return_ਵਿੱਚ_ਤਰੱਕੀ;
# define G_flag_return_ਵਿੱਚ_ਤਰੱਕੀ (G.flag_return_ਵਿੱਚ_ਤਰੱਕੀ)
#ਨਹੀਂਤਾਂ
# define G_flag_return_ਵਿੱਚ_ਤਰੱਕੀ 0
#ਜੇ_ਅੰਤ
	smallint exiting; /* used to prevent EXIT trap recursion */
	/* These support $? */
	smalluint ਆਖਰੀ_exitcode;
	smalluint expand_exitcode;
	smalluint ਆਖਰੀ_bg_pid_exitcode;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
#ਜੇ ENABLE_HUSH_ਸੈੱਟ
	/* are global_argv and global_argv[1..n] malloced? (note: not [0]) */
	smalluint global_ਦਲੀਲਾਂ_malloced;
# define G_global_ਦਲੀਲਾਂ_malloced (G.global_ਦਲੀਲਾਂ_malloced)
#ਨਹੀਂਤਾਂ
# define G_global_ਦਲੀਲਾਂ_malloced 0
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_HUSH_BASH_COMPAT
	ਪੂਰਨ dead_ਕੰਮ_exitcode; /* for "wait -n" */
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */
	/* how many non-NULL argv's we have. NB: $# + 1 */
	ਪੂਰਨ global_argc;
	ਅੱਖਰ **global_argv;
#ਜੇ !BB_MMU
	ਅੱਖਰ *argv0_for_re_execing;
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_HUSH_LOOPS
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਡੂੰਘਾਈ_break_continue;
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਡੂੰਘਾਈ_ਦਾ_loop;
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
#ਜੇ ENABLE_HUSH_GETOPTS
	ਬਿਨਾਨਿਸ਼ਾਨੀ getopt_ਗਿਣਤੀ;
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */
	ਅਚਲ ਅੱਖਰ *ifs;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	ਪੂਰਨ flag_repeat;
	ਪੂਰਨ do_repeat;
	ਪੂਰਨ ਲੜੀਵਾਰ_command_flags;
#ਜੇ_ਅੰਤ /* __U_BOOT__ */
	ਅੱਖਰ *ifs_whitespace; /* = G.ifs or malloced */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	ਅਚਲ ਅੱਖਰ *cwd;
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */
	ਬਣਤਰ variable *ਸਿਖਰ_var;
	ਅੱਖਰ **expanded_assignments;
	ਬਣਤਰ variable **shadowed_vars_pp;
	ਬਿਨਾਨਿਸ਼ਾਨੀ var_nest_level;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
#ਜੇ ENABLE_HUSH_FUNCTIONS
# ਜੇ ENABLE_HUSH_LOCAL
	ਬਿਨਾਨਿਸ਼ਾਨੀ func_nest_level; /* solely to prevent "local v" in non-functions */
# endif
	ਬਣਤਰ function *ਸਿਖਰ_func;
#ਜੇ_ਅੰਤ
	/* Signal and trap handling */
#ਜੇ ENABLE_HUSH_FAST
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਗਿਣਤੀ_SIGCHLD;
	ਬਿਨਾਨਿਸ਼ਾਨੀ handled_SIGCHLD;
	smallint we_have_children;
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_HUSH_LINENO_VAR
	ਬਿਨਾਨਿਸ਼ਾਨੀ parse_lineno;
	ਬਿਨਾਨਿਸ਼ਾਨੀ execute_lineno;
#ਜੇ_ਅੰਤ
	HFILE *HFILE_list;
	HFILE *HFILE_stdin;
	/* Which signals have non-DFL handler (even with no traps set)?
	 * Set at the start to:
	 * (SIGQUIT + maybe SPECIAL_INTERACTIVE_SIGS + maybe SPECIAL_JOBSTOP_SIGS)
	 * SPECIAL_INTERACTIVE_SIGS are cleared after fork.
	 * The rest is cleared right before execv syscalls.
	 * Other than these two times, never modified.
	 */
	ਬਿਨਾਨਿਸ਼ਾਨੀ special_sig_ਨਕਾਬ;
#ਜੇ ENABLE_HUSH_ਕੰਮ
	ਬਿਨਾਨਿਸ਼ਾਨੀ fatal_sig_ਨਕਾਬ;
# define G_fatal_sig_ਨਕਾਬ (G.fatal_sig_ਨਕਾਬ)
#ਨਹੀਂਤਾਂ
# define G_fatal_sig_ਨਕਾਬ 0
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_HUSH_TRAP
	ਪੂਰਨ pre_trap_exitcode;
# ਜੇ ENABLE_HUSH_FUNCTIONS
	ਪੂਰਨ return_exitcode;
# endif
	ਅੱਖਰ **traps; /* char *traps[NSIG] */
# define G_traps G.traps
#ਨਹੀਂਤਾਂ
# define G_traps ((ਅੱਖਰ**)ਨਿਰਾ)
#ਜੇ_ਅੰਤ
	sigset_t pending_ਸੈੱਟ;
#ਜੇ ENABLE_HUSH_MEMLEAK
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ memleak_ਮੁੱਲ;
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_HUSH_ਬਹੁਲਕ_X
	ਬਿਨਾਨਿਸ਼ਾਨੀ x_ਬਹੁਲਕ_ਡੂੰਘਾਈ;
	/* "set -x" output should not be redirectable with subsequent 2>FILE.
	 * We dup fd#2 to x_mode_fd when "set -x" is executed, and use it
	 * for all subsequent output.
	 */
	ਪੂਰਨ x_ਬਹੁਲਕ_fd;
	o_string x_ਬਹੁਲਕ_buf;
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */
#ਜੇ HUSH_DEBUG >= 2
	ਪੂਰਨ debug_indent;
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	ਬਣਤਰ sigaction sa;
	ਅੱਖਰ optstring_buf[ਆਕਾਰ("eixcs")];
#ਜੇ BASH_EPOCH_VARS
	ਅੱਖਰ epoch_buf[ਆਕਾਰ("%llu.nnnnnn") + ਆਕਾਰ(ਲੰਮਾ ਲੰਮਾ)*3];
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_FEATURE_EDITING
	ਅੱਖਰ user_ਨਿਵੇਸ਼_buf[CONFIG_FEATURE_EDITING_MAX_LEN];
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */
};
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
ਬਣਤਰ globals *ptr_ਮੰਜ਼ਿਲ_globals;
#ਜੇ_ਅੰਤ /* __U_BOOT__ */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ G (*ptr_ਮੰਜ਼ਿਲ_globals)
/* Not #defining name to G.name - this quickly gets unwieldy
 * (too many defines). Also, I actually prefer to see when a variable
 * is global, thus "G." prefix is a useful hint */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸੈੱਟ_PTR_ਮੰਜ਼ਿਲ_GLOBALS(x) ਕਰੋ { \
	(*(ਬਣਤਰ globals**)&ptr_ਮੰਜ਼ਿਲ_globals) = (ਖਾਲੀ*)(x); \
	barrier(); \
} ਜਦੋਂ (0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ INIT_G() ਕਰੋ { \
	ਸੈੱਟ_PTR_ਮੰਜ਼ਿਲ_GLOBALS(xzalloc(ਆਕਾਰ(G))); \
	G.promptmode = 1; \
} ਜਦੋਂ (0)
#ਨਹੀਂਤਾਂ /* !__U_BOOT__ */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ INIT_G() ਕਰੋ { \
	ਸੈੱਟ_PTR_ਮੰਜ਼ਿਲ_GLOBALS(xzalloc(ਆਕਾਰ(G))); \
	/* memset(&G.sa, 0, sizeof(G.sa)); */  \
	sigfillset(&G.sa.sa_ਨਕਾਬ); \
	G.sa.sa_flags = SA_RESTART; \
} ਜਦੋਂ (0)
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
/* Function prototypes for builtins */
ਸਥਿਰ ਪੂਰਨ builtin_cd(ਅੱਖਰ **argv) FAST_FUNC;
#ਜੇ ENABLE_HUSH_ECHO
ਸਥਿਰ ਪੂਰਨ builtin_echo(ਅੱਖਰ **argv) FAST_FUNC;
#ਜੇ_ਅੰਤ
ਸਥਿਰ ਪੂਰਨ builtin_eval(ਅੱਖਰ **argv) FAST_FUNC;
ਸਥਿਰ ਪੂਰਨ builtin_exec(ਅੱਖਰ **argv) FAST_FUNC;
ਸਥਿਰ ਪੂਰਨ builtin_exit(ਅੱਖਰ **argv) FAST_FUNC;
#ਜੇ ENABLE_HUSH_EXPORT
ਸਥਿਰ ਪੂਰਨ builtin_export(ਅੱਖਰ **argv) FAST_FUNC;
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_HUSH_READONLY
ਸਥਿਰ ਪੂਰਨ builtin_readonly(ਅੱਖਰ **argv) FAST_FUNC;
#ਜੇ_ਅੰਤ
ਸਥਿਰ ਪੂਰਨ builtin_false(ਅੱਖਰ **argv) FAST_FUNC;
#ਜੇ ENABLE_HUSH_ਕੰਮ
ਸਥਿਰ ਪੂਰਨ builtin_fg_bg(ਅੱਖਰ **argv) FAST_FUNC;
ਸਥਿਰ ਪੂਰਨ builtin_jobs(ਅੱਖਰ **argv) FAST_FUNC;
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_HUSH_GETOPTS
ਸਥਿਰ ਪੂਰਨ builtin_getopts(ਅੱਖਰ **argv) FAST_FUNC;
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_HUSH_HELP
ਸਥਿਰ ਪੂਰਨ builtin_help(ਅੱਖਰ **argv) FAST_FUNC;
#ਜੇ_ਅੰਤ
#ਜੇ MAX_HISTORY && ENABLE_FEATURE_EDITING
ਸਥਿਰ ਪੂਰਨ builtin_history(ਅੱਖਰ **argv) FAST_FUNC;
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_HUSH_LOCAL
ਸਥਿਰ ਪੂਰਨ builtin_local(ਅੱਖਰ **argv) FAST_FUNC;
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_HUSH_MEMLEAK
ਸਥਿਰ ਪੂਰਨ builtin_memleak(ਅੱਖਰ **argv) FAST_FUNC;
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_HUSH_PRINTF
ਸਥਿਰ ਪੂਰਨ builtin_printf(ਅੱਖਰ **argv) FAST_FUNC;
#ਜੇ_ਅੰਤ
ਸਥਿਰ ਪੂਰਨ builtin_pwd(ਅੱਖਰ **argv) FAST_FUNC;
#ਜੇ ENABLE_HUSH_READ
ਸਥਿਰ ਪੂਰਨ builtin_read(ਅੱਖਰ **argv) FAST_FUNC;
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_HUSH_ਸੈੱਟ
ਸਥਿਰ ਪੂਰਨ builtin_ਸੈੱਟ(ਅੱਖਰ **argv) FAST_FUNC;
#ਜੇ_ਅੰਤ
ਸਥਿਰ ਪੂਰਨ builtin_ਖਿਸਕਾਓ(ਅੱਖਰ **argv) FAST_FUNC;
ਸਥਿਰ ਪੂਰਨ builtin_ਸਰੋਤ(ਅੱਖਰ **argv) FAST_FUNC;
#ਜੇ ENABLE_HUSH_TEST || BASH_TEST2
ਸਥਿਰ ਪੂਰਨ builtin_test(ਅੱਖਰ **argv) FAST_FUNC;
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_HUSH_TRAP
ਸਥਿਰ ਪੂਰਨ builtin_trap(ਅੱਖਰ **argv) FAST_FUNC;
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_HUSH_TYPE
ਸਥਿਰ ਪੂਰਨ builtin_type(ਅੱਖਰ **argv) FAST_FUNC;
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_HUSH_TIMES
ਸਥਿਰ ਪੂਰਨ builtin_times(ਅੱਖਰ **argv) FAST_FUNC;
#ਜੇ_ਅੰਤ
ਸਥਿਰ ਪੂਰਨ builtin_true(ਅੱਖਰ **argv) FAST_FUNC;
#ਜੇ ENABLE_HUSH_UMASK
ਸਥਿਰ ਪੂਰਨ builtin_umask(ਅੱਖਰ **argv) FAST_FUNC;
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_HUSH_UNSET
ਸਥਿਰ ਪੂਰਨ builtin_unset(ਅੱਖਰ **argv) FAST_FUNC;
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_HUSH_KILL
ਸਥਿਰ ਪੂਰਨ builtin_kill(ਅੱਖਰ **argv) FAST_FUNC;
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_HUSH_WAIT
ਸਥਿਰ ਪੂਰਨ builtin_wait(ਅੱਖਰ **argv) FAST_FUNC;
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_HUSH_LOOPS
ਸਥਿਰ ਪੂਰਨ builtin_break(ਅੱਖਰ **argv) FAST_FUNC;
ਸਥਿਰ ਪੂਰਨ builtin_continue(ਅੱਖਰ **argv) FAST_FUNC;
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_HUSH_FUNCTIONS
ਸਥਿਰ ਪੂਰਨ builtin_return(ਅੱਖਰ **argv) FAST_FUNC;
#ਜੇ_ਅੰਤ

/* Table of built-in functions.  They can be forked or not, depending on
 * context: within pipes, they fork.  As simple commands, they do not.
 * When used in non-forking context, they can change global variables
 * in the parent shell process.  If forked, of course they cannot.
 * For example, 'unset foo | whatever' will parse and run, but foo will
 * still be set at the end. */
ਬਣਤਰ built_ਵਿੱਚ_command {
	ਅਚਲ ਅੱਖਰ *b_cmd;
	ਪੂਰਨ (*b_function)(ਅੱਖਰ **argv) FAST_FUNC;
#ਜੇ ENABLE_HUSH_HELP
	ਅਚਲ ਅੱਖਰ *b_descr;
# define BLTIN(cmd, func, help) { cmd, func, help }
#ਨਹੀਂਤਾਂ
# define BLTIN(cmd, func, help) { cmd, func }
#ਜੇ_ਅੰਤ
};

ਸਥਿਰ ਅਚਲ ਬਣਤਰ built_ਵਿੱਚ_command bltins1[] ALIGN_PTR = {
	BLTIN("."        , builtin_ਸਰੋਤ  , "Run commands in file"),
	BLTIN(":"        , builtin_true    , ਨਿਰਾ),
#ਜੇ ENABLE_HUSH_ਕੰਮ
	BLTIN("bg"       , builtin_fg_bg   , "Resume job in background"),
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_HUSH_LOOPS
	BLTIN("break"    , builtin_break   , "Exit loop"),
#ਜੇ_ਅੰਤ
	BLTIN("cd"       , builtin_cd      , "Change directory"),
#ਜੇ ENABLE_HUSH_LOOPS
	BLTIN("continue" , builtin_continue, "Start new loop iteration"),
#ਜੇ_ਅੰਤ
	BLTIN("eval"     , builtin_eval    , "Construct and run shell command"),
	BLTIN("exec"     , builtin_exec    , "Execute command, don't return to shell"),
	BLTIN("exit"     , builtin_exit    , ਨਿਰਾ),
#ਜੇ ENABLE_HUSH_EXPORT
	BLTIN("export"   , builtin_export  , "Set environment variables"),
#ਜੇ_ਅੰਤ
	BLTIN("false"    , builtin_false   , ਨਿਰਾ),
#ਜੇ ENABLE_HUSH_ਕੰਮ
	BLTIN("fg"       , builtin_fg_bg   , "Bring job to foreground"),
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_HUSH_GETOPTS
	BLTIN("getopts"  , builtin_getopts , ਨਿਰਾ),
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_HUSH_HELP
	BLTIN("help"     , builtin_help    , ਨਿਰਾ),
#ਜੇ_ਅੰਤ
#ਜੇ MAX_HISTORY && ENABLE_FEATURE_EDITING
	BLTIN("history"  , builtin_history , "Show history"),
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_HUSH_ਕੰਮ
	BLTIN("jobs"     , builtin_jobs    , "List jobs"),
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_HUSH_KILL
	BLTIN("kill"     , builtin_kill    , "Send signals to processes"),
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_HUSH_LOCAL
	BLTIN("local"    , builtin_local   , "Set local variables"),
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_HUSH_MEMLEAK
	BLTIN("memleak"  , builtin_memleak , ਨਿਰਾ),
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_HUSH_READ
	BLTIN("read"     , builtin_read    , "Input into variable"),
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_HUSH_READONLY
	BLTIN("readonly" , builtin_readonly, "Make variables read-only"),
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_HUSH_FUNCTIONS
	BLTIN("return"   , builtin_return  , "Return from function"),
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_HUSH_ਸੈੱਟ
	BLTIN("set"      , builtin_ਸੈੱਟ     , "Set positional parameters"),
#ਜੇ_ਅੰਤ
	BLTIN("shift"    , builtin_ਖਿਸਕਾਓ   , "Shift positional parameters"),
#ਜੇ BASH_ਸਰੋਤ
	BLTIN("source"   , builtin_ਸਰੋਤ  , ਨਿਰਾ),
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_HUSH_TIMES
	BLTIN("times"    , builtin_times   , ਨਿਰਾ),
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_HUSH_TRAP
	BLTIN("trap"     , builtin_trap    , "Trap signals"),
#ਜੇ_ਅੰਤ
	BLTIN("true"     , builtin_true    , ਨਿਰਾ),
#ਜੇ ENABLE_HUSH_TYPE
	BLTIN("type"     , builtin_type    , "Show command type"),
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_HUSH_ULIMIT
	BLTIN("ulimit"   , shell_builtin_ulimit, "Control resource limits"),
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_HUSH_UMASK
	BLTIN("umask"    , builtin_umask   , "Set file creation mask"),
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_HUSH_UNSET
	BLTIN("unset"    , builtin_unset   , "Unset variables"),
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_HUSH_WAIT
	BLTIN("wait"     , builtin_wait    , "Wait for process to finish"),
#ਜੇ_ਅੰਤ
};
/* These builtins won't be used if we are on NOMMU and need to re-exec
 * (it's cheaper to run an external program in this case):
 */
ਸਥਿਰ ਅਚਲ ਬਣਤਰ built_ਵਿੱਚ_command bltins2[] ALIGN_PTR = {
#ਜੇ ENABLE_HUSH_TEST
	BLTIN("["        , builtin_test    , ਨਿਰਾ),
#ਜੇ_ਅੰਤ
#ਜੇ BASH_TEST2
	BLTIN("[["       , builtin_test    , ਨਿਰਾ),
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_HUSH_ECHO
	BLTIN("echo"     , builtin_echo    , ਨਿਰਾ),
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_HUSH_PRINTF
	BLTIN("printf"   , builtin_printf  , ਨਿਰਾ),
#ਜੇ_ਅੰਤ
	BLTIN("pwd"      , builtin_pwd     , ਨਿਰਾ),
#ਜੇ ENABLE_HUSH_TEST
	BLTIN("test"     , builtin_test    , ਨਿਰਾ),
#ਜੇ_ਅੰਤ
};

#ਜੇ_ਅੰਤ /* !__U_BOOT__ */

/* Debug printouts.
 */
#ਜੇ HUSH_DEBUG >= 2
/* prevent disasters with G.debug_indent < 0 */
# define indent() fdprintf(2, "%*s", (G.debug_indent * 2) & 0xff, "")
# define debug_enter() (G.debug_indent++)
# define debug_leave() (G.debug_indent--)
#ਨਹੀਂਤਾਂ
# define indent()      ((ਖਾਲੀ)0)
# define debug_enter() ((ਖਾਲੀ)0)
# define debug_leave() ((ਖਾਲੀ)0)
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ debug_printf
# define debug_printf(...) (indent(), fdprintf(2, __VA_ARGS__))
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ debug_printf_parse
# define debug_printf_parse(...) (indent(), fdprintf(2, __VA_ARGS__))
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ debug_printf_heredoc
# define debug_printf_heredoc(...) (indent(), fdprintf(2, __VA_ARGS__))
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ debug_printf_exec
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ debug_printf_exec(...) (indent(), fdprintf(2, __VA_ARGS__))
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ debug_printf_env
# define debug_printf_env(...) (indent(), fdprintf(2, __VA_ARGS__))
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ debug_printf_jobs
# define debug_printf_jobs(...) (indent(), fdprintf(2, __VA_ARGS__))
# define DEBUG_JOBS 1
#ਨਹੀਂਤਾਂ
# define DEBUG_JOBS 0
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ debug_printf_expand
# define debug_printf_expand(...) (indent(), fdprintf(2, __VA_ARGS__))
# define DEBUG_EXPAND 1
#ਨਹੀਂਤਾਂ
# define DEBUG_EXPAND 0
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ debug_printf_varexp
# define debug_printf_varexp(...) (indent(), fdprintf(2, __VA_ARGS__))
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ debug_printf_glob
# define debug_printf_glob(...) (indent(), fdprintf(2, __VA_ARGS__))
# define DEBUG_GLOB 1
#ਨਹੀਂਤਾਂ
# define DEBUG_GLOB 0
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ debug_printf_redir
# define debug_printf_redir(...) (indent(), fdprintf(2, __VA_ARGS__))
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ debug_printf_list
# define debug_printf_list(...) (indent(), fdprintf(2, __VA_ARGS__))
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ debug_printf_subst
# define debug_printf_subst(...) (indent(), fdprintf(2, __VA_ARGS__))
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ debug_printf_prompt
# define debug_printf_prompt(...) (indent(), fdprintf(2, __VA_ARGS__))
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ debug_printf_clean
# define debug_printf_clean(...) (indent(), fdprintf(2, __VA_ARGS__))
# define DEBUG_CLEAN 1
#ਨਹੀਂਤਾਂ
# define DEBUG_CLEAN 0
#ਜੇ_ਅੰਤ

#ਜੇ DEBUG_EXPAND
ਸਥਿਰ ਖਾਲੀ debug_print_ਲੜੀਆਂ(ਅਚਲ ਅੱਖਰ *ਅਗੇਤਰ, ਅੱਖਰ **vv)
{
	indent();
	fdprintf(2, "%s:\n", ਅਗੇਤਰ);
	ਜਦੋਂ (*vv)
		fdprintf(2, " '%s'\n", *vv++);
}
#ਨਹੀਂਤਾਂ
# define debug_print_ਲੜੀਆਂ(ਅਗੇਤਰ, vv) ((ਖਾਲੀ)0)
#ਜੇ_ਅੰਤ

/* Leak hunting. Use hush_leaktool.sh for post-processing.
 */
#ਜੇ LEAK_HUNTING
ਸਥਿਰ ਖਾਲੀ *xxmalloc(ਪੂਰਨ lineno, size_t ਮਾਪ)
{
	ਖਾਲੀ *ptr = xmalloc((ਮਾਪ + 0xff) & ~0xff);
	fdprintf(2, "line %d: malloc %p\n", lineno, ptr);
	ਵਾਪਸ ptr;
}
ਸਥਿਰ ਖਾਲੀ *xxrealloc(ਪੂਰਨ lineno, ਖਾਲੀ *ptr, size_t ਮਾਪ)
{
	ptr = xrealloc(ptr, (ਮਾਪ + 0xff) & ~0xff);
	fdprintf(2, "line %d: realloc %p\n", lineno, ptr);
	ਵਾਪਸ ptr;
}
ਸਥਿਰ ਅੱਖਰ *xxstrdup(ਪੂਰਨ lineno, ਅਚਲ ਅੱਖਰ *str)
{
	ਅੱਖਰ *ptr = xstrdup(str);
	fdprintf(2, "line %d: strdup %p\n", lineno, ptr);
	ਵਾਪਸ ptr;
}
ਸਥਿਰ ਖਾਲੀ xxfree(ਖਾਲੀ *ptr)
{
	fdprintf(2, "free %p\n", ptr);
	ਮੁਕਤ(ptr);
}
# define xmalloc(s)     xxmalloc(__LINE__, s)
# define xrealloc(p, s) xxrealloc(__LINE__, p, s)
# define xstrdup(s)     xxstrdup(__LINE__, s)
# define ਮੁਕਤ(p)        xxfree(p)
#ਜੇ_ਅੰਤ

/* Syntax and runtime errors. They always abort scripts.
 * In interactive use they usually discard unparsed and/or unexecuted commands
 * and return to the prompt.
 * HUSH_DEBUG >= 2 prints line number in this file where it was detected.
 */
#ਜੇ HUSH_DEBUG < 2
# define ਸੁਨੇਹਾ_and_die_if_script(lineno, ...)     ਸੁਨੇਹਾ_and_die_if_script(__VA_ARGS__)
# define syntax_error(lineno, ਸੁਨੇਹਾ)              syntax_error(ਸੁਨੇਹਾ)
# define syntax_error_at(lineno, ਸੁਨੇਹਾ)           syntax_error_at(ਸੁਨੇਹਾ)
# define syntax_error_unterm_ch(lineno, ch)     syntax_error_unterm_ch(ch)
# define syntax_error_unterm_str(lineno, s)     syntax_error_unterm_str(s)
# define syntax_error_unexpected_ch(lineno, ch) syntax_error_unexpected_ch(ch)
#ਜੇ_ਅੰਤ

ਸਥਿਰ ਖਾਲੀ die_if_script(ਖਾਲੀ)
{
	ਜੇ (!G_interactive_fd) {
		ਜੇ (G.ਆਖਰੀ_exitcode) /* sometines it's 2, not 1 (bash compat) */
			xfunc_error_retval = G.ਆਖਰੀ_exitcode;
		xfunc_die();
	}
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
ਸਥਿਰ ਖਾਲੀ __maybe_unused ਸੁਨੇਹਾ_and_die_if_script(ਬਿਨਾਨਿਸ਼ਾਨੀ lineno, ਅਚਲ ਅੱਖਰ *fmt, ...)
#ਨਹੀਂਤਾਂ /* !__U_BOOT__ */
ਸਥਿਰ ਖਾਲੀ ਸੁਨੇਹਾ_and_die_if_script(ਬਿਨਾਨਿਸ਼ਾਨੀ lineno, ਅਚਲ ਅੱਖਰ *fmt, ...)
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */
{
	va_list p;

#ਜੇ HUSH_DEBUG >= 2
	bb_error_ਸੁਨੇਹਾ("hush.c:%u", lineno);
#ਜੇ_ਅੰਤ
	va_ਸ਼ੁਰੂ(p, fmt);
	bb_verror_ਸੁਨੇਹਾ(fmt, p, ਨਿਰਾ);
	va_ਸਿਰਾ(p);
	die_if_script();
}

ਸਥਿਰ ਖਾਲੀ syntax_error(ਬਿਨਾਨਿਸ਼ਾਨੀ lineno ਅਣਵਰਤਿਆ_PARAM, ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ)
{
	ਜੇ (ਸੁਨੇਹਾ)
		bb_error_ਸੁਨੇਹਾ("syntax error: %s", ਸੁਨੇਹਾ);
	ਨਹੀਂਤਾਂ
		bb_simple_error_ਸੁਨੇਹਾ("syntax error");
	die_if_script();
}

ਸਥਿਰ ਖਾਲੀ syntax_error_at(ਬਿਨਾਨਿਸ਼ਾਨੀ lineno ਅਣਵਰਤਿਆ_PARAM, ਅਚਲ ਅੱਖਰ *ਸੁਨੇਹਾ)
{
	bb_error_ਸੁਨੇਹਾ("syntax error at '%s'", ਸੁਨੇਹਾ);
	die_if_script();
}

ਸਥਿਰ ਖਾਲੀ syntax_error_unterm_str(ਬਿਨਾਨਿਸ਼ਾਨੀ lineno ਅਣਵਰਤਿਆ_PARAM, ਅਚਲ ਅੱਖਰ *s)
{
	bb_error_ਸੁਨੇਹਾ("syntax error: unterminated %s", s);
//? source4.tests fails: in bash, echo ${^} in script does not terminate the script
// (but bash --posix, or if bash is run as "sh", does terminate in script, so maybe uncomment this?)
//	die_if_script();
}

ਸਥਿਰ ਖਾਲੀ syntax_error_unterm_ch(ਬਿਨਾਨਿਸ਼ਾਨੀ lineno, ਅੱਖਰ ch)
{
	ਅੱਖਰ ਸੁਨੇਹਾ[2] = { ch, '\0' };
	syntax_error_unterm_str(lineno, ਸੁਨੇਹਾ);
}

ਸਥਿਰ ਖਾਲੀ syntax_error_unexpected_ch(ਬਿਨਾਨਿਸ਼ਾਨੀ lineno ਅਣਵਰਤਿਆ_PARAM, ਪੂਰਨ ch)
{
	ਅੱਖਰ ਸੁਨੇਹਾ[2];
	ਸੁਨੇਹਾ[0] = ch;
	ਸੁਨੇਹਾ[1] = '\0';
#ਜੇ HUSH_DEBUG >= 2
	bb_error_ਸੁਨੇਹਾ("hush.c:%u", lineno);
#ਜੇ_ਅੰਤ
	bb_error_ਸੁਨੇਹਾ("syntax error: unexpected %s", ch == EOF ? "EOF" : ਸੁਨੇਹਾ);
	die_if_script();
}

#ਜੇ HUSH_DEBUG < 2
# undef ਸੁਨੇਹਾ_and_die_if_script
# undef syntax_error
# undef syntax_error_at
# undef syntax_error_unterm_ch
# undef syntax_error_unterm_str
# undef syntax_error_unexpected_ch
#ਨਹੀਂਤਾਂ
# define ਸੁਨੇਹਾ_and_die_if_script(...)     ਸੁਨੇਹਾ_and_die_if_script(__LINE__, __VA_ARGS__)
# define syntax_error(ਸੁਨੇਹਾ)              syntax_error(__LINE__, ਸੁਨੇਹਾ)
# define syntax_error_at(ਸੁਨੇਹਾ)           syntax_error_at(__LINE__, ਸੁਨੇਹਾ)
# define syntax_error_unterm_ch(ch)     syntax_error_unterm_ch(__LINE__, ch)
# define syntax_error_unterm_str(s)     syntax_error_unterm_str(__LINE__, s)
# define syntax_error_unexpected_ch(ch) syntax_error_unexpected_ch(__LINE__, ch)
#ਜੇ_ਅੰਤ

/* Utility functions
 */
/* Replace each \x with x in place, return ptr past NUL. */
ਸਥਿਰ ਅੱਖਰ *unbackslash(ਅੱਖਰ *src)
{
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	ਅੱਖਰ *dst = src = (ਅੱਖਰ *)strchrnul(src, '\\');
#ਨਹੀਂਤਾਂ /* !__U_BOOT__ */
	ਅੱਖਰ *dst = src = strchrnul(src, '\\');
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */
	ਜਦੋਂ (1) {
		ਜੇ (*src == '\\') {
			src++;
			ਜੇ (*src != '\0') {
				/* \x -> x */
				*dst++ = *src++;
				ਜਾਰੀ;
			}
			/* else: "\<nul>". Do not delete this backslash.
			 * Testcase: eval 'echo ok\'
			 */
			*dst++ = '\\';
			/* fallthrough */
		}
		ਜੇ ((*dst++ = *src++) == '\0')
			ਰੋਕੋ;
	}
	ਵਾਪਸ dst;
}

ਸਥਿਰ ਅੱਖਰ **ਜੋੜੋ_ਲੜੀਆਂ_ਮੰਜ਼ਿਲ_ਲੜੀਆਂ(ਅੱਖਰ **ਲੜੀਆਂ, ਅੱਖਰ **ਜੋੜੋ, ਪੂਰਨ need_ਮੰਜ਼ਿਲ_dup)
{
	ਪੂਰਨ i;
	ਬਿਨਾਨਿਸ਼ਾਨੀ count1;
	ਬਿਨਾਨਿਸ਼ਾਨੀ count2;
	ਅੱਖਰ **v;

	v = ਲੜੀਆਂ;
	count1 = 0;
	ਜੇ (v) {
		ਜਦੋਂ (*v) {
			count1++;
			v++;
		}
	}
	count2 = 0;
	v = ਜੋੜੋ;
	ਜਦੋਂ (*v) {
		count2++;
		v++;
	}
	v = xrealloc(ਲੜੀਆਂ, (count1 + count2 + 1) * ਆਕਾਰ(ਅੱਖਰ*));
	v[count1 + count2] = ਨਿਰਾ;
	i = count2;
	ਜਦੋਂ (--i >= 0)
		v[count1 + i] = (need_ਮੰਜ਼ਿਲ_dup ? xstrdup(ਜੋੜੋ[i]) : ਜੋੜੋ[i]);
	ਵਾਪਸ v;
}
#ਜੇ LEAK_HUNTING
ਸਥਿਰ ਅੱਖਰ **xx_ਜੋੜੋ_ਲੜੀਆਂ_ਮੰਜ਼ਿਲ_ਲੜੀਆਂ(ਪੂਰਨ lineno, ਅੱਖਰ **ਲੜੀਆਂ, ਅੱਖਰ **ਜੋੜੋ, ਪੂਰਨ need_ਮੰਜ਼ਿਲ_dup)
{
	ਅੱਖਰ **ptr = ਜੋੜੋ_ਲੜੀਆਂ_ਮੰਜ਼ਿਲ_ਲੜੀਆਂ(ਲੜੀਆਂ, ਜੋੜੋ, need_ਮੰਜ਼ਿਲ_dup);
	fdprintf(2, "line %d: add_strings_to_strings %p\n", lineno, ptr);
	ਵਾਪਸ ptr;
}
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਜੋੜੋ_ਲੜੀਆਂ_ਮੰਜ਼ਿਲ_ਲੜੀਆਂ(ਲੜੀਆਂ, ਜੋੜੋ, need_ਮੰਜ਼ਿਲ_dup) \
	xx_ਜੋੜੋ_ਲੜੀਆਂ_ਮੰਜ਼ਿਲ_ਲੜੀਆਂ(__LINE__, ਲੜੀਆਂ, ਜੋੜੋ, need_ਮੰਜ਼ਿਲ_dup)
#ਜੇ_ਅੰਤ

/* Note: takes ownership of "add" ptr (it is not strdup'ed) */
ਸਥਿਰ ਅੱਖਰ **ਜੋੜੋ_string_ਮੰਜ਼ਿਲ_ਲੜੀਆਂ(ਅੱਖਰ **ਲੜੀਆਂ, ਅੱਖਰ *ਜੋੜੋ)
{
	ਅੱਖਰ *v[2];
	v[0] = ਜੋੜੋ;
	v[1] = ਨਿਰਾ;
	ਵਾਪਸ ਜੋੜੋ_ਲੜੀਆਂ_ਮੰਜ਼ਿਲ_ਲੜੀਆਂ(ਲੜੀਆਂ, v, /*dup:*/ 0);
}

#ਜੇ LEAK_HUNTING
ਸਥਿਰ ਅੱਖਰ **xx_ਜੋੜੋ_string_ਮੰਜ਼ਿਲ_ਲੜੀਆਂ(ਪੂਰਨ lineno, ਅੱਖਰ **ਲੜੀਆਂ, ਅੱਖਰ *ਜੋੜੋ)
{
	ਅੱਖਰ **ptr = ਜੋੜੋ_string_ਮੰਜ਼ਿਲ_ਲੜੀਆਂ(ਲੜੀਆਂ, ਜੋੜੋ);
	fdprintf(2, "line %d: add_string_to_strings %p\n", lineno, ptr);
	ਵਾਪਸ ptr;
}
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਜੋੜੋ_string_ਮੰਜ਼ਿਲ_ਲੜੀਆਂ(ਲੜੀਆਂ, ਜੋੜੋ) \
	xx_ਜੋੜੋ_string_ਮੰਜ਼ਿਲ_ਲੜੀਆਂ(__LINE__, ਲੜੀਆਂ, ਜੋੜੋ)
#ਜੇ_ਅੰਤ

ਸਥਿਰ ਖਾਲੀ free_ਲੜੀਆਂ(ਅੱਖਰ **ਲੜੀਆਂ)
{
	ਅੱਖਰ **v;

	ਜੇ (!ਲੜੀਆਂ)
		ਵਾਪਸ;
	v = ਲੜੀਆਂ;
	ਜਦੋਂ (*v) {
		ਮੁਕਤ(*v);
		v++;
	}
	ਮੁਕਤ(ਲੜੀਆਂ);
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
ਸਥਿਰ ਪੂਰਨ dup_CLOEXEC(ਪੂਰਨ fd, ਪੂਰਨ avoid_fd)
{
	ਪੂਰਨ newfd;
 repeat:
	newfd = fcntl(fd, F_DUPFD_CLOEXEC, avoid_fd + 1);
	ਜੇ (newfd >= 0) {
		ਜੇ (F_DUPFD_CLOEXEC == F_DUPFD) /* if old libc (w/o F_DUPFD_CLOEXEC) */
			close_on_exec_on(newfd);
	} ਨਹੀਂਤਾਂ { /* newfd < 0 */
		ਜੇ (errno == EBUSY)
			ਜਾਓ repeat;
		ਜੇ (errno == EINTR)
			ਜਾਓ repeat;
		ਜੇ (errno != EBADF) {
			/* "echo >&9999" gets EINVAL trying to save fd 1 to above 9999.
			 * We could try saving it _below_ 9999 instead (how?), but
			 * this probably means that dup2(9999,1) to effectuate >&9999
			 * would also not work: fd 9999 can't exist.
			 * (This differs from "echo >&99" where saving works, but
			 * subsequent dup2(99,1) fails if fd 99 is not open).
			 */
			bb_perror_ਸੁਨੇਹਾ("fcntl(%d,F_DUPFD,%d)", fd, avoid_fd + 1);
		}
	}
	ਵਾਪਸ newfd;
}

ਸਥਿਰ ਪੂਰਨ xdup_CLOEXEC_and_close(ਪੂਰਨ fd, ਪੂਰਨ avoid_fd)
{
	ਪੂਰਨ newfd;
 repeat:
	newfd = fcntl(fd, F_DUPFD_CLOEXEC, avoid_fd + 1);
	ਜੇ (newfd < 0) {
		ਜੇ (errno == EBUSY)
			ਜਾਓ repeat;
		ਜੇ (errno == EINTR)
			ਜਾਓ repeat;
		/* fd was not open? */
		ਜੇ (errno == EBADF)
			ਵਾਪਸ fd;
		xfunc_die();
	}
	ਜੇ (F_DUPFD_CLOEXEC == F_DUPFD) /* if old libc (w/o F_DUPFD_CLOEXEC) */
		close_on_exec_on(newfd);
	close(fd);
	ਵਾਪਸ newfd;
}

/* Manipulating HFILEs */
ਸਥਿਰ HFILE *hfopen(ਅਚਲ ਅੱਖਰ *name)
{
	HFILE *fp;
	ਪੂਰਨ fd;

	fd = STDIN_FILENO;
	ਜੇ (name) {
		fd = open(name, O_RDONLY | O_CLOEXEC);
		ਜੇ (fd < 0)
			ਵਾਪਸ ਨਿਰਾ;
		ਜੇ (O_CLOEXEC == 0) /* ancient libc */
			close_on_exec_on(fd);
	}

	fp = xmalloc(ਆਕਾਰ(*fp));
	ਜੇ (name == ਨਿਰਾ)
		G.HFILE_stdin = fp;
	fp->fd = fd;
	fp->cur = fp->ਸਿਰਾ_2 = fp->buf;
	fp->ਅਗਲਾ_hfile = G.HFILE_list;
	G.HFILE_list = fp;
	ਵਾਪਸ fp;
}
ਸਥਿਰ ਖਾਲੀ hfclose(HFILE *fp)
{
	HFILE **pp = &G.HFILE_list;
	ਜਦੋਂ (*pp) {
		HFILE *cur = *pp;
		ਜੇ (cur == fp) {
			*pp = cur->ਅਗਲਾ_hfile;
			ਰੋਕੋ;
		}
		pp = &cur->ਅਗਲਾ_hfile;
	}
	ਜੇ (fp->fd >= 0)
		close(fp->fd);
	ਮੁਕਤ(fp);
}
ਸਥਿਰ ਪੂਰਨ refill_HFILE_and_getc(HFILE *fp)
{
	ਪੂਰਨ n;

	ਜੇ (fp->fd < 0) {
		/* Already saw EOF */
		ਵਾਪਸ EOF;
	}
#ਜੇ ENABLE_HUSH_INTERACTIVE && !ENABLE_FEATURE_EDITING
	/* If user presses ^C, read() restarts after SIGINT (we use SA_RESTART).
	 * IOW: ^C will not immediately stop line input.
	 * But poll() is different: it does NOT restart after signals.
	 */
	ਜੇ (fp == G.HFILE_stdin) {
		ਬਣਤਰ pollfd pfd[1];
		pfd[0].fd = fp->fd;
		pfd[0].events = POLLIN;
		n = poll(pfd, 1, -1);
		ਜੇ (n < 0
		 /*&& errno == EINTR - assumed true */
		 && sigismember(&G.pending_ਸੈੱਟ, SIGINT)
		) {
			ਵਾਪਸ '\0';
		}
	}
#ਨਹੀਂਤਾਂ
/* if FEATURE_EDITING=y, we do not use this routine for interactive input */
#ਜੇ_ਅੰਤ
	/* Try to buffer more input */
	n = ਸੁਰੱਖਿਅਤ_read(fp->fd, fp->buf, ਆਕਾਰ(fp->buf));
	ਜੇ (n < 0) {
		bb_simple_perror_ਸੁਨੇਹਾ("read error");
		n = 0;
	}
	fp->cur = fp->buf;
	fp->ਸਿਰਾ_2 = fp->buf + n;
	ਜੇ (n == 0) {
		/* EOF/error */
		close(fp->fd);
		fp->fd = -1;
		ਵਾਪਸ EOF;
	}
	ਵਾਪਸ (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)(*fp->cur++);
}
/* Inlined for common case of non-empty buffer.
 */
ਸਥਿਰ ALWAYS_INLINE ਪੂਰਨ hfgetc(HFILE *fp)
{
	ਜੇ (fp->cur < fp->ਸਿਰਾ_2)
		ਵਾਪਸ (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)(*fp->cur++);
	/* Buffer empty */
	ਵਾਪਸ refill_HFILE_and_getc(fp);
}
ਸਥਿਰ ਪੂਰਨ move_HFILEs_on_redirect(ਪੂਰਨ fd, ਪੂਰਨ avoid_fd)
{
	HFILE *fl = G.HFILE_list;
	ਜਦੋਂ (fl) {
		ਜੇ (fd == fl->fd) {
			/* We use it only on script files, they are all CLOEXEC */
			fl->fd = xdup_CLOEXEC_and_close(fd, avoid_fd);
			debug_printf_redir("redirect_fd %d: matches a script fd, moving it to %d\n", fd, fl->fd);
			ਵਾਪਸ 1; /* "found and moved" */
		}
		fl = fl->ਅਗਲਾ_hfile;
	}
#ਜੇ ENABLE_HUSH_ਬਹੁਲਕ_X
	ਜੇ (G.x_ਬਹੁਲਕ_fd > 0 && fd == G.x_ਬਹੁਲਕ_fd) {
		G.x_ਬਹੁਲਕ_fd = xdup_CLOEXEC_and_close(fd, avoid_fd);
		ਵਾਪਸ 1; /* "found and moved" */
	}
#ਜੇ_ਅੰਤ
	ਵਾਪਸ 0; /* "not in the list" */
}
#ਜੇ ENABLE_FEATURE_SH_STANDALONE && BB_MMU
ਸਥਿਰ ਖਾਲੀ close_all_HFILE_list(ਖਾਲੀ)
{
	HFILE *fl = G.HFILE_list;
	ਜਦੋਂ (fl) {
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
		ਜੇ (fl->fd > 0)
			/*hfclose(fl); - unsafe */
			close(fl->fd);
		fl = fl->ਅਗਲਾ_hfile;
	}
}
#ਜੇ_ਅੰਤ
ਸਥਿਰ ਪੂਰਨ fd_ਵਿੱਚ_HFILEs(ਪੂਰਨ fd)
{
	HFILE *fl = G.HFILE_list;
	ਜਦੋਂ (fl) {
		ਜੇ (fl->fd == fd)
			ਵਾਪਸ 1;
		fl = fl->ਅਗਲਾ_hfile;
	}
	ਵਾਪਸ 0;
}

#ਜੇ_ਅੰਤ /* !__U_BOOT__ */

/* Helpers for setting new $n and restoring them back
 */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ save_arg_t {
	ਅੱਖਰ *sv_argv0;
	ਅੱਖਰ **sv_g_argv;
	ਪੂਰਨ sv_g_argc;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	IF_HUSH_ਸੈੱਟ(smallint sv_g_malloced;)
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */
} save_arg_t;

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
ਸਥਿਰ ਖਾਲੀ save_and_replace_G_ਦਲੀਲਾਂ(save_arg_t *sv, ਅੱਖਰ **argv)
{
	sv->sv_argv0 = argv[0];
	sv->sv_g_argv = G.global_argv;
	sv->sv_g_argc = G.global_argc;
	IF_HUSH_ਸੈੱਟ(sv->sv_g_malloced = G.global_ਦਲੀਲਾਂ_malloced;)

	argv[0] = G.global_argv[0]; /* retain $0 */
	G.global_argv = argv;
	IF_HUSH_ਸੈੱਟ(G.global_ਦਲੀਲਾਂ_malloced = 0;)

	G.global_argc = 1 + string_array_len(argv + 1);
}

ਸਥਿਰ ਖਾਲੀ restore_G_ਦਲੀਲਾਂ(save_arg_t *sv, ਅੱਖਰ **argv)
{
#ਜੇ ENABLE_HUSH_ਸੈੱਟ
	ਜੇ (G.global_ਦਲੀਲਾਂ_malloced) {
		/* someone ran "set -- arg1 arg2 ...", undo */
		ਅੱਖਰ **pp = G.global_argv;
		ਜਦੋਂ (*++pp) /* note: does not free $0 */
			ਮੁਕਤ(*pp);
		ਮੁਕਤ(G.global_argv);
	}
#ਜੇ_ਅੰਤ
	argv[0] = sv->sv_argv0;
	G.global_argv = sv->sv_g_argv;
	G.global_argc = sv->sv_g_argc;
	IF_HUSH_ਸੈੱਟ(G.global_ਦਲੀਲਾਂ_malloced = sv->sv_g_malloced;)
}
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
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
ਗਿਣਤੀ {
	SPECIAL_INTERACTIVE_SIGS = 0
		| (1 << SIGTERM)
		| (1 << SIGINT)
		| (1 << SIGHUP)
		,
	SPECIAL_JOBSTOP_SIGS = 0
#ਜੇ ENABLE_HUSH_ਕੰਮ
		| (1 << SIGTTIN)
		| (1 << SIGTTOU)
		| (1 << SIGTSTP)
#ਜੇ_ਅੰਤ
		,
};

ਸਥਿਰ ਖਾਲੀ ਰਿਕਾਰਡ_pending_signo(ਪੂਰਨ sig)
{
	sigaddset(&G.pending_ਸੈੱਟ, sig);
#ਜੇ ENABLE_FEATURE_EDITING
	ਜੇ (sig != SIGCHLD
	 || (G_traps && G_traps[SIGCHLD] && G_traps[SIGCHLD][0])
	 /* ^^^ if SIGCHLD, interrupt line reading only if it has a trap */
	) {
		bb_got_signal = sig; /* for read_line_input: "we got a signal" */
	}
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_HUSH_FAST
	ਜੇ (sig == SIGCHLD) {
		G.ਗਿਣਤੀ_SIGCHLD++;
//bb_error_msg("[%d] SIGCHLD_handler: G.count_SIGCHLD:%d G.handled_SIGCHLD:%d", getpid(), G.count_SIGCHLD, G.handled_SIGCHLD);
	}
#ਜੇ_ਅੰਤ
}

ਸਥਿਰ sighandler_t install_sighandler(ਪੂਰਨ sig, sighandler_t handler)
{
	ਬਣਤਰ sigaction old_sa;

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
	ਵਾਪਸ old_sa.sa_handler;
}
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
ਸਥਿਰ ਖਾਲੀ hush_exit(ਪੂਰਨ exitcode) NORETURN;

ਸਥਿਰ ਖਾਲੀ restore_ttypgrp_and__exit(ਖਾਲੀ) NORETURN;
ਸਥਿਰ ਖਾਲੀ restore_ttypgrp_and__exit(ਖਾਲੀ)
{
	/* xfunc has failed! die die die */
	/* no EXIT traps, this is an escape hatch! */
	G.exiting = 1;
	hush_exit(xfunc_error_retval);
}

#ਜੇ ENABLE_HUSH_ਕੰਮ

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
ਸਥਿਰ ਖਾਲੀ fflush_and__exit(ਖਾਲੀ) NORETURN;
ਸਥਿਰ ਖਾਲੀ fflush_and__exit(ਖਾਲੀ)
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
ਸਥਿਰ ਖਾਲੀ sigexit(ਪੂਰਨ sig) NORETURN;
ਸਥਿਰ ਖਾਲੀ sigexit(ਪੂਰਨ sig)
{
	/* Careful: we can end up here after [v]fork. Do not restore
	 * tty pgrp then, only top-level shell process does that */
	ਜੇ (G_saved_tty_pgrp && getpid() == G.ਜੜ੍ਹ_pid) {
		/* Disable all signals: job control, SIGPIPE, etc.
		 * Mostly paranoid measure, to prevent infinite SIGTTOU.
		 */
		sigprocmask_allsigs(SIG_ਖੰਡ);
		tcsetpgrp(G_interactive_fd, G_saved_tty_pgrp);
	}

	/* Not a signal, just exit */
	ਜੇ (sig <= 0)
		_exit(- sig);

	kill_myself_with_sig(sig); /* does not return */
}
#ਨਹੀਂਤਾਂ

# define disable_restore_tty_pgrp_on_exit() ((ਖਾਲੀ)0)
# define enable_restore_tty_pgrp_on_exit()  ((ਖਾਲੀ)0)

#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
ਸਥਿਰ sighandler_t pick_sighandler(ਬਿਨਾਨਿਸ਼ਾਨੀ sig)
{
	sighandler_t handler = SIG_DFL;
	ਜੇ (sig < ਆਕਾਰ(ਬਿਨਾਨਿਸ਼ਾਨੀ)*8) {
		ਬਿਨਾਨਿਸ਼ਾਨੀ sigmask = (1 << sig);

#ਜੇ ENABLE_HUSH_ਕੰਮ
		/* is sig fatal? */
		ਜੇ (G_fatal_sig_ਨਕਾਬ & sigmask)
			handler = sigexit;
		ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
		/* sig has special handling? */
		ਜੇ (G.special_sig_ਨਕਾਬ & sigmask) {
			handler = ਰਿਕਾਰਡ_pending_signo;
			/* TTIN/TTOU/TSTP can't be set to record_pending_signo
			 * in order to ignore them: they will be raised
			 * in an endless loop when we try to do some
			 * terminal ioctls! We do have to _ignore_ these.
			 */
			ਜੇ (SPECIAL_JOBSTOP_SIGS & sigmask)
				handler = SIG_IGN;
		}
	}
	ਵਾਪਸ handler;
}
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
/* Restores tty foreground process group, and exits. */
ਸਥਿਰ ਖਾਲੀ hush_exit(ਪੂਰਨ exitcode)
{
#ਜੇ ENABLE_FEATURE_EDITING_SAVE_ON_EXIT
	save_history(G.ਸਤਰ_ਨਿਵੇਸ਼_state); /* may be NULL */
#ਜੇ_ਅੰਤ

	fflush_all();
	ਜੇ (G.exiting <= 0 && G_traps && G_traps[0] && G_traps[0][0]) {
		ਅੱਖਰ *argv[3];
		/* argv[0] is unused */
		argv[1] = xstrdup(G_traps[0]); /* copy, since EXIT trap handler may modify G_traps[0] */
		argv[2] = ਨਿਰਾ;
		G.exiting = 1; /* prevent EXIT trap recursion */
		/* Note: G_traps[0] is not cleared!
		 * "trap" will still show it, if executed
		 * in the handler */
		builtin_eval(argv);
	}

#ਜੇ ENABLE_FEATURE_CLEAN_ਉੱਪਰ
	{
		ਬਣਤਰ variable *cur_var;
		ਜੇ (G.cwd != bb_ਸੁਨੇਹਾ_unknown)
			ਮੁਕਤ((ਅੱਖਰ*)G.cwd);
		cur_var = G.ਸਿਖਰ_var;
		ਜਦੋਂ (cur_var) {
			ਬਣਤਰ variable *tmp = cur_var;
			ਜੇ (!cur_var->max_len)
				ਮੁਕਤ(cur_var->varstr);
			cur_var = cur_var->ਅਗਲਾ;
			ਮੁਕਤ(tmp);
		}
	}
#ਜੇ_ਅੰਤ

	fflush_all();
#ਜੇ ENABLE_HUSH_ਕੰਮ
	sigexit(- (exitcode & 0xff));
#ਨਹੀਂਤਾਂ
	_exit(exitcode);
#ਜੇ_ਅੰਤ
}

//TODO: return a mask of ALL handled sigs?
ਸਥਿਰ ਪੂਰਨ check_and_ਲੜੀਵਾਰ_traps(ਖਾਲੀ)
{
	ਪੂਰਨ ਆਖਰੀ_sig = 0;

	ਜਦੋਂ (1) {
		ਪੂਰਨ sig;

		ਜੇ (sigisemptyset(&G.pending_ਸੈੱਟ))
			ਰੋਕੋ;
		sig = 0;
		ਕਰੋ {
			sig++;
			ਜੇ (sigismember(&G.pending_ਸੈੱਟ, sig)) {
				sigdelset(&G.pending_ਸੈੱਟ, sig);
				ਜਾਓ got_sig;
			}
		} ਜਦੋਂ (sig < NSIG);
		ਰੋਕੋ;
 got_sig:
#ਜੇ ENABLE_HUSH_TRAP
		ਜੇ (G_traps && G_traps[sig]) {
			debug_printf_exec("%s: sig:%d handler:'%s'\n", __func__, sig, G.traps[sig]);
			ਜੇ (G_traps[sig][0]) {
				/* We have user-defined handler */
				smalluint save_rcode;
				ਪੂਰਨ save_pre;
				ਅੱਖਰ *argv[3];
				/* argv[0] is unused */
				argv[1] = xstrdup(G_traps[sig]);
				/* why strdup? trap can modify itself: trap 'trap "echo oops" INT' INT */
				argv[2] = ਨਿਰਾ;
				save_pre = G.pre_trap_exitcode;
				G.pre_trap_exitcode = save_rcode = G.ਆਖਰੀ_exitcode;
				builtin_eval(argv);
				ਮੁਕਤ(argv[1]);
				G.pre_trap_exitcode = save_pre;
				G.ਆਖਰੀ_exitcode = save_rcode;
# ਜੇ ENABLE_HUSH_FUNCTIONS
				ਜੇ (G.return_exitcode >= 0) {
					debug_printf_exec("trap exitcode:%d\n", G.return_exitcode);
					G.ਆਖਰੀ_exitcode = G.return_exitcode;
				}
# endif
				ਆਖਰੀ_sig = sig;
			} /* else: "" trap, ignoring signal */
			ਜਾਰੀ;
		}
#ਜੇ_ਅੰਤ
		/* not a trap: special action */
		ਚੋਣ (sig) {
		ਹਾਲਤ SIGINT:
			debug_printf_exec("%s: sig:%d default SIGINT handler\n", __func__, sig);
			G.flag_SIGINT = 1;
			ਆਖਰੀ_sig = sig;
			ਰੋਕੋ;
#ਜੇ ENABLE_HUSH_ਕੰਮ
		ਹਾਲਤ SIGHUP: {
			/* if (G_interactive_fd) - no need to check, the handler
			 * is only installed if we *are* interactive */
			{
				/* bash compat: "Before exiting, an interactive
				 * shell resends the SIGHUP to all jobs, running
				 * or stopped.  Stopped jobs are sent SIGCONT
				 * to ensure that they receive the SIGHUP."
				 */
				ਬਣਤਰ pipe *ਕੰਮ;
				debug_printf_exec("%s: sig:%d default SIGHUP handler\n", __func__, sig);
				/* bash is observed to signal whole process groups,
				 * not individual processes */
				ਲਈ (ਕੰਮ = G.ਕੰਮ_list; ਕੰਮ; ਕੰਮ = ਕੰਮ->ਅਗਲਾ) {
					ਜੇ (ਕੰਮ->pgrp <= 0)
						ਜਾਰੀ;
					debug_printf_exec("HUPing pgrp %d\n", ਕੰਮ->pgrp);
					ਜੇ (kill(- ਕੰਮ->pgrp, SIGHUP) == 0)
						kill(- ਕੰਮ->pgrp, SIGCONT);
				}
			}
			/* this restores tty pgrp, then kills us with SIGHUP */
			sigexit(SIGHUP);
		}
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_HUSH_FAST
		ਹਾਲਤ SIGCHLD:
			debug_printf_exec("%s: sig:%d default SIGCHLD handler\n", __func__, sig);
			G.ਗਿਣਤੀ_SIGCHLD++;
//bb_error_msg("[%d] check_and_run_traps: G.count_SIGCHLD:%d G.handled_SIGCHLD:%d", getpid(), G.count_SIGCHLD, G.handled_SIGCHLD);
			/* Note:
			 * We don't do 'last_sig = sig' here -> NOT returning this sig.
			 * This simplifies wait builtin a bit.
			 */
			ਰੋਕੋ;
#ਜੇ_ਅੰਤ
		ਮੂਲ: /* ignored: */
			debug_printf_exec("%s: sig:%d default handling is to ignore\n", __func__, sig);
			/* SIGTERM, SIGQUIT, SIGTTIN, SIGTTOU, SIGTSTP */
			/* Note:
			 * We don't do 'last_sig = sig' here -> NOT returning this sig.
			 * Example: wait is not interrupted by TERM
			 * in interactive shell, because TERM is ignored.
			 */
			ਰੋਕੋ;
		}
	}
	ਵਾਪਸ ਆਖਰੀ_sig;
}

ਸਥਿਰ ਅਚਲ ਅੱਖਰ *get_cwd(ਪੂਰਨ force)
{
	ਜੇ (force || G.cwd == ਨਿਰਾ) {
		/* xrealloc_getcwd_or_warn(arg) calls free(arg),
		 * we must not try to free(bb_msg_unknown) */
		ਜੇ (G.cwd == bb_ਸੁਨੇਹਾ_unknown)
			G.cwd = ਨਿਰਾ;
		G.cwd = xrealloc_getcwd_or_warn((ਅੱਖਰ *)G.cwd);
		ਜੇ (!G.cwd)
			G.cwd = bb_ਸੁਨੇਹਾ_unknown;
	}
	ਵਾਪਸ G.cwd;
}

#ਜੇ_ਅੰਤ /* !__U_BOOT__ */

/*
 * Shell and environment variable support
 */
ਸਥਿਰ ਬਣਤਰ variable **get_ptr_ਮੰਜ਼ਿਲ_local_var(ਅਚਲ ਅੱਖਰ *name)
{
	ਬਣਤਰ variable **pp;
	ਬਣਤਰ variable *cur;

	pp = &G.ਸਿਖਰ_var;
	ਜਦੋਂ ((cur = *pp) != ਨਿਰਾ) {
		ਜੇ (varcmp(cur->varstr, name) == 0)
			ਵਾਪਸ pp;
		pp = &cur->ਅਗਲਾ;
	}
	ਵਾਪਸ ਨਿਰਾ;
}

ਸਥਿਰ ਅਚਲ ਅੱਖਰ* FAST_FUNC get_local_var_ਮੁੱਲ(ਅਚਲ ਅੱਖਰ *name)
{
	ਬਣਤਰ variable **vpp;

	ਜੇ (G.expanded_assignments) {
		ਅੱਖਰ **cpp = G.expanded_assignments;
		ਜਦੋਂ (*cpp) {
			ਅੱਖਰ *cp = *cpp;
			ਜੇ (varcmp(cp, name) == 0)
				ਵਾਪਸ strchr(cp, '=') + 1;
			cpp++;
		}
	}

	vpp = get_ptr_ਮੰਜ਼ਿਲ_local_var(name);
	ਜੇ (vpp)
		ਵਾਪਸ strchr((*vpp)->varstr, '=') + 1;

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	ਜੇ (strcmp(name, "PPID") == 0)
		ਵਾਪਸ utoa(G.ਜੜ੍ਹ_ppid);
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */
	// bash compat: UID? EUID?
#ਜੇ ENABLE_HUSH_RANDOM_SUPPORT
	ਜੇ (strcmp(name, "RANDOM") == 0)
		ਵਾਪਸ utoa(ਅਗਲਾ_random(&G.random_gen));
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_HUSH_LINENO_VAR
	ਜੇ (strcmp(name, "LINENO") == 0)
		ਵਾਪਸ utoa(G.execute_lineno);
#ਜੇ_ਅੰਤ
#ਜੇ BASH_EPOCH_VARS
	{
		ਅਚਲ ਅੱਖਰ *fmt = ਨਿਰਾ;
		ਜੇ (strcmp(name, "EPOCHSECONDS") == 0)
			fmt = "%llu";
		ਨਹੀਂਤਾਂ ਜੇ (strcmp(name, "EPOCHREALTIME") == 0)
			fmt = "%llu.%06u";
		ਜੇ (fmt) {
			ਬਣਤਰ timeval tv;
			xgettimeofday(&tv);
			sprintf(G.epoch_buf, fmt, (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ)tv.tv_sec,
					(ਬਿਨਾਨਿਸ਼ਾਨੀ)tv.tv_usec);
			ਵਾਪਸ G.epoch_buf;
		}
	}
#ਜੇ_ਅੰਤ
	ਵਾਪਸ ਨਿਰਾ;
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
#ਜੇ ENABLE_HUSH_GETOPTS
ਸਥਿਰ ਖਾਲੀ handle_ਬਦਲਿਆ_special_ਨਾਂ(ਅਚਲ ਅੱਖਰ *name)
{
	ਜੇ (varcmp(name, "OPTIND") == 0) {
		G.getopt_ਗਿਣਤੀ = 0;
		ਵਾਪਸ;
	}
}
#ਨਹੀਂਤਾਂ
/* Do not even bother evaluating arguments */
# define handle_ਬਦਲਿਆ_special_ਨਾਂ(...) ((ਖਾਲੀ)0)
#ਜੇ_ਅੰਤ
#ਨਹੀਂਤਾਂ /* __U_BOOT__ */
/* Do not even bother evaluating arguments */
# define handle_ਬਦਲਿਆ_special_ਨਾਂ(...) ((ਖਾਲੀ)0)
#ਜੇ_ਅੰਤ /* __U_BOOT__ */

/* str holds "NAME=VAL" and is expected to be malloced.
 * We take ownership of it.
 */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SETFLAG_EXPORT   (1 << 0)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SETFLAG_UNEXPORT (1 << 1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SETFLAG_MAKE_RO  (1 << 2)
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SETFLAG_VARLVL_ਖਿਸਕਾਓ   3
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
ਸਥਿਰ ਪੂਰਨ ਸੈੱਟ_local_var(ਅੱਖਰ *str, ਬਿਨਾਨਿਸ਼ਾਨੀ flags)
#ਨਹੀਂਤਾਂ /* __U_BOOT__ */
ਪੂਰਨ ਸੈੱਟ_local_var_modern(ਅੱਖਰ *str, ਪੂਰਨ flags)
#ਜੇ_ਅੰਤ /* __U_BOOT__ */
{
	ਬਣਤਰ variable **cur_pp;
	ਬਣਤਰ variable *cur;
	ਅੱਖਰ *free_me = ਨਿਰਾ;
	ਅੱਖਰ *eq_sign;
	ਪੂਰਨ name_len;
	ਪੂਰਨ retval;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	ਬਿਨਾਨਿਸ਼ਾਨੀ local_lvl = (flags >> SETFLAG_VARLVL_ਖਿਸਕਾਓ);
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */

	eq_sign = strchr(str, '=');
	ਜੇ (HUSH_DEBUG && !eq_sign)
		bb_simple_error_ਸੁਨੇਹਾ_and_die("BUG in setvar");

	name_len = eq_sign - str + 1; /* including '=' */
	cur_pp = &G.ਸਿਖਰ_var;
	ਜਦੋਂ ((cur = *cur_pp) != ਨਿਰਾ) {
		ਜੇ (ਲੜੀ_ਤੁਲਨਾ(cur->varstr, str, name_len) != 0) {
			cur_pp = &cur->ਅਗਲਾ;
			ਜਾਰੀ;
		}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
		/* We found an existing var with this name */
		ਜੇ (cur->flg_read_only) {
			bb_error_ਸੁਨੇਹਾ("%s: readonly variable", str);
			ਮੁਕਤ(str);
//NOTE: in bash, assignment in "export READONLY_VAR=Z" fails, and sets $?=1,
//but export per se succeeds (does put the var in env). We don't mimic that.
			ਵਾਪਸ -1;
		}
		ਜੇ (flags & SETFLAG_UNEXPORT) { // && cur->flg_export ?
			debug_printf_env("%s: unsetenv '%s'\n", __func__, str);
			*eq_sign = '\0';
			unsetenv(str);
			*eq_sign = '=';
		}
		ਜੇ (cur->var_nest_level < local_lvl) {
			/* bash 3.2.33(1) and exported vars:
			 * # export z=z
			 * # f() { local z=a; env | grep ^z; }
			 * # f
			 * z=a
			 * # env | grep ^z
			 * z=z
			 */
			ਜੇ (cur->flg_export)
				flags |= SETFLAG_EXPORT;
			/* New variable is local ("local VAR=VAL" or
			 * "VAR=VAL cmd")
			 * and existing one is global, or local
			 * on a lower level that new one.
			 * Remove it from global variable list:
			 */
			*cur_pp = cur->ਅਗਲਾ;
			ਜੇ (G.shadowed_vars_pp) {
				/* Save in "shadowed" list */
				debug_printf_env("shadowing %s'%s'/%u by '%s'/%u\n",
					cur->flg_export ? "exported " : "",
					cur->varstr, cur->var_nest_level, str, local_lvl
				);
				cur->ਅਗਲਾ = *G.shadowed_vars_pp;
				*G.shadowed_vars_pp = cur;
			} ਨਹੀਂਤਾਂ {
				/* Came from pseudo_exec_argv(), no need to save: delete it */
				debug_printf_env("shadow-deleting %s'%s'/%u by '%s'/%u\n",
					cur->flg_export ? "exported " : "",
					cur->varstr, cur->var_nest_level, str, local_lvl
				);
				ਜੇ (cur->max_len == 0) /* allocated "VAR=VAL"? */
					free_me = cur->varstr; /* then free it later */
				ਮੁਕਤ(cur);
			}
			ਰੋਕੋ;
		}
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */

		ਜੇ (strcmp(cur->varstr + name_len, eq_sign + 1) == 0) {
			debug_printf_env("assignement '%s' does not change anything\n", str);
 free_and_exp:
			ਮੁਕਤ(str);
			ਜਾਓ ਘਾਤਾਂਕ;
		}

		/* Replace the value in the found "struct variable" */
		ਜੇ (cur->max_len != 0) {
			ਜੇ (cur->max_len >= strnlen(str, cur->max_len + 1)) {
				/* This one is from startup env, reuse space */
				debug_printf_env("reusing startup env for '%s'\n", str);
				strcpy(cur->varstr, str);
				ਜਾਓ free_and_exp;
			}
			/* Can't reuse */
			cur->max_len = 0;
			ਜਾਓ ਸੈੱਟ_str_and_exp;
		}
		/* max_len == 0 signifies "malloced" var, which we can
		 * (and have to) free. But we can't free(cur->varstr) here:
		 * if cur->flg_export is 1, it is in the environment.
		 * We should either unsetenv+free, or wait until putenv,
		 * then putenv(new)+free(old).
		 */
		free_me = cur->varstr;
		ਜਾਓ ਸੈੱਟ_str_and_exp;
	}

	/* Not found or shadowed - create new variable struct */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	debug_printf_env("%s: alloc new var '%s'/%u\n", __func__, str, local_lvl);
#ਨਹੀਂਤਾਂ /* __U_BOOT__ */
	debug_printf_env("%s: alloc new var '%s'\n", __func__, str);
#ਜੇ_ਅੰਤ /* __U_BOOT__ */
	cur = xzalloc(ਆਕਾਰ(*cur));
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	cur->var_nest_level = local_lvl;
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */
	cur->ਅਗਲਾ = *cur_pp;
	*cur_pp = cur;

 ਸੈੱਟ_str_and_exp:
	cur->varstr = str;
 ਘਾਤਾਂਕ:
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
#ਜੇ !BB_MMU || ENABLE_HUSH_READONLY
	ਜੇ (flags & SETFLAG_MAKE_RO) {
		cur->flg_read_only = 1;
	}
#ਜੇ_ਅੰਤ
	ਜੇ (flags & SETFLAG_EXPORT)
		cur->flg_export = 1;
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */
	retval = 0;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	ਜੇ (cur->flg_export) {
		ਜੇ (flags & SETFLAG_UNEXPORT) {
			cur->flg_export = 0;
			/* unsetenv was already done */
		} ਨਹੀਂਤਾਂ {
			debug_printf_env("%s: putenv '%s'/%u\n", __func__, cur->varstr, cur->var_nest_level);
			retval = putenv(cur->varstr);
			/* fall through to "free(free_me)" -
			 * only now we can free old exported malloced string
			 */
		}
	}
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */
	ਮੁਕਤ(free_me);

	handle_ਬਦਲਿਆ_special_ਨਾਂ(cur->varstr);

	ਵਾਪਸ retval;
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
ਸਥਿਰ ਪੂਰਨ ਸੈੱਟ_local_var0(ਅੱਖਰ *str)
{
	ਵਾਪਸ ਸੈੱਟ_local_var(str, 0);
}

ਸਥਿਰ ਖਾਲੀ FAST_FUNC ਸੈੱਟ_local_var_from_halves(ਅਚਲ ਅੱਖਰ *name, ਅਚਲ ਅੱਖਰ *val)
{
	ਅੱਖਰ *var = xasprintf("%s=%s", name, val);
	ਸੈੱਟ_local_var0(var);
}

/* Used at startup and after each cd */
ਸਥਿਰ ਖਾਲੀ ਸੈੱਟ_pwd_var(ਬਿਨਾਨਿਸ਼ਾਨੀ flag)
{
	ਸੈੱਟ_local_var(xasprintf("PWD=%s", get_cwd(/*force:*/ 1)), flag);
}
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */

#ਜੇ ENABLE_HUSH_UNSET || ENABLE_HUSH_GETOPTS
ਸਥਿਰ ਪੂਰਨ unset_local_var(ਅਚਲ ਅੱਖਰ *name)
{
	ਬਣਤਰ variable *cur;
	ਬਣਤਰ variable **cur_pp;

	cur_pp = &G.ਸਿਖਰ_var;
	ਜਦੋਂ ((cur = *cur_pp) != ਨਿਰਾ) {
		ਜੇ (varcmp(cur->varstr, name) == 0) {
			ਜੇ (cur->flg_read_only) {
				bb_error_ਸੁਨੇਹਾ("%s: readonly variable", name);
				ਵਾਪਸ EXIT_FAILURE;
			}

			*cur_pp = cur->ਅਗਲਾ;
			debug_printf_env("%s: unsetenv '%s'\n", __func__, cur->varstr);
			bb_unsetenv(cur->varstr);
			ਜੇ (!cur->max_len)
				ਮੁਕਤ(cur->varstr);
			ਮੁਕਤ(cur);

			ਰੋਕੋ;
		}
		cur_pp = &cur->ਅਗਲਾ;
	}

	/* Handle "unset LINENO" et al even if did not find the variable to unset */
	handle_ਬਦਲਿਆ_special_ਨਾਂ(name);

	ਵਾਪਸ EXIT_SUCCESS;
}
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
/*
 * Helpers for "var1=val1 var2=val2 cmd" feature
 */
ਸਥਿਰ ਖਾਲੀ ਜੋੜੋ_vars(ਬਣਤਰ variable *var)
{
	ਬਣਤਰ variable *ਅਗਲਾ;

	ਜਦੋਂ (var) {
		ਅਗਲਾ = var->ਅਗਲਾ;
		var->ਅਗਲਾ = G.ਸਿਖਰ_var;
		G.ਸਿਖਰ_var = var;
		ਜੇ (var->flg_export) {
			debug_printf_env("%s: restoring exported '%s'/%u\n", __func__, var->varstr, var->var_nest_level);
			putenv(var->varstr);
		} ਨਹੀਂਤਾਂ {
			debug_printf_env("%s: restoring variable '%s'/%u\n", __func__, var->varstr, var->var_nest_level);
		}
		var = ਅਗਲਾ;
	}
}

/* We put strings[i] into variable table and possibly putenv them.
 * If variable is read only, we can free the strings[i]
 * which attempts to overwrite it.
 * The strings[] vector itself is freed.
 */
ਸਥਿਰ ਖਾਲੀ ਸੈੱਟ_vars_and_save_old(ਅੱਖਰ **ਲੜੀਆਂ)
{
	ਅੱਖਰ **s;

	ਜੇ (!ਲੜੀਆਂ)
		ਵਾਪਸ;

	s = ਲੜੀਆਂ;
	ਜਦੋਂ (*s) {
		ਬਣਤਰ variable *var_p;
		ਬਣਤਰ variable **var_pp;
		ਅੱਖਰ *eq;

		eq = strchr(*s, '=');
		ਜੇ (HUSH_DEBUG && !eq)
			bb_simple_error_ਸੁਨੇਹਾ_and_die("BUG in varexp4");
		var_pp = get_ptr_ਮੰਜ਼ਿਲ_local_var(*s);
		ਜੇ (var_pp) {
			var_p = *var_pp;
			ਜੇ (var_p->flg_read_only) {
				ਅੱਖਰ **p;
				bb_error_ਸੁਨੇਹਾ("%s: readonly variable", *s);
				/*
				 * "VAR=V BLTIN" unsets VARs after BLTIN completes.
				 * If VAR is readonly, leaving it in the list
				 * after asssignment error (msg above)
				 * causes doubled error message later, on unset.
				 */
				debug_printf_env("removing/freeing '%s' element\n", *s);
				ਮੁਕਤ(*s);
				p = s;
				ਕਰੋ { *p = p[1]; p++; } ਜਦੋਂ (*p);
				ਜਾਓ ਅਗਲਾ;
			}
			/* below, set_local_var() with nest level will
			 * "shadow" (remove) this variable from
			 * global linked list.
			 */
		}
		debug_printf_env("%s: env override '%s'/%u\n", __func__, *s, G.var_nest_level);
		ਸੈੱਟ_local_var(*s, (G.var_nest_level << SETFLAG_VARLVL_ਖਿਸਕਾਓ) | SETFLAG_EXPORT);
		s++;
 ਅਗਲਾ: ;
	}
	ਮੁਕਤ(ਲੜੀਆਂ);
}

/*
 * Unicode helper
 */
ਸਥਿਰ ਖਾਲੀ reinit_unicode_for_hush(ਖਾਲੀ)
{
	/* Unicode support should be activated even if LANG is set
	 * _during_ shell execution, not only if it was set when
	 * shell was started. Therefore, re-check LANG every time:
	 */
	ਜੇ (ENABLE_FEATURE_CHECK_UNICODE_ਵਿੱਚ_ENV
	 || ENABLE_UNICODE_USING_LOCALE
	) {
		ਅਚਲ ਅੱਖਰ *s = get_local_var_ਮੁੱਲ("LC_ALL");
		ਜੇ (!s) s = get_local_var_ਮੁੱਲ("LC_CTYPE");
		ਜੇ (!s) s = get_local_var_ਮੁੱਲ("LANG");
		reinit_unicode(s);
	}
}

#ਜੇ_ਅੰਤ /* !__U_BOOT__ */
/*
 * in_str support (strings, and "strings" read from files).
 */

#ਜੇ ENABLE_HUSH_INTERACTIVE
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
/* To test correct lineedit/interactive behavior, type from command line:
 *	echo $P\
 *	\
 *	AT\
 *	H\
 *	\
 * It exercises a lot of corner cases.
 */
ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਤਿਆਰੀ_prompt_string(ਖਾਲੀ)
{
	ਅਚਲ ਅੱਖਰ *prompt_str;

	debug_printf_prompt("%s promptmode:%d\n", __func__, G.promptmode);

# ਜੇ ENABLE_FEATURE_EDITING_FANCY_PROMPT
	prompt_str = get_local_var_ਮੁੱਲ(G.promptmode == 0 ? "PS1" : "PS2");
	ਜੇ (!prompt_str)
		prompt_str = "";
# ਨਹੀਂਤਾਂ
	prompt_str = "> "; /* if PS2, else... */
	ਜੇ (G.promptmode == 0) { /* PS1 */
		/* No fancy prompts supported, (re)generate "CURDIR $ " by hand */
		ਮੁਕਤ(G.PS1);
		/* bash uses $PWD value, even if it is set by user.
		 * It uses current dir only if PWD is unset.
		 * We always use current dir. */
		prompt_str = G.PS1 = xasprintf("%s %c ", get_cwd(0), (geteuid() != 0) ? '$' : '#');
	}
# endif
	debug_printf("prompt_str '%s'\n", prompt_str);
	ਵਾਪਸ prompt_str;
}
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
ਸਥਿਰ ਪੂਰਨ get_user_ਨਿਵੇਸ਼(ਬਣਤਰ ਵਿੱਚ_str *i)
#ਨਹੀਂਤਾਂ /* __U_BOOT__ */
ਸਥਿਰ ਖਾਲੀ get_user_ਨਿਵੇਸ਼(ਬਣਤਰ ਵਿੱਚ_str *i)
#ਜੇ_ਅੰਤ /* __U_BOOT__ */
{
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
# ਜੇ ENABLE_FEATURE_EDITING
	/* In EDITING case, this function reads next input line,
	 * saves it in i->p, then returns 1st char of it.
	 */
	ਪੂਰਨ r;
	ਅਚਲ ਅੱਖਰ *prompt_str;

	prompt_str = ਤਿਆਰੀ_prompt_string();
	ਲਈ (;;) {
		reinit_unicode_for_hush();
		G.flag_SIGINT = 0;

		bb_got_signal = 0;
		ਜੇ (!sigisemptyset(&G.pending_ਸੈੱਟ)) {
			/* Whoops, already got a signal, do not call read_line_input */
			bb_got_signal = r = -1;
		} ਨਹੀਂਤਾਂ {
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
			r = read_ਸਤਰ_ਨਿਵੇਸ਼(G.ਸਤਰ_ਨਿਵੇਸ਼_state, prompt_str,
				G.user_ਨਿਵੇਸ਼_buf, CONFIG_FEATURE_EDITING_MAX_LEN-1
			);
			/* read_line_input intercepts ^C, "convert" it to SIGINT */
			ਜੇ (r == 0)
				ਉਠਾਓ(SIGINT);
		}
		/* bash prints ^C (before running a trap, if any)
		 * both on keyboard ^C and on real SIGINT (non-kbd generated).
		 */
		ਜੇ (sigismember(&G.pending_ਸੈੱਟ, SIGINT)) {
			ਲਿਖ(STDOUT_FILENO, "^C\n", 3);
			G.ਆਖਰੀ_exitcode = 128 | SIGINT;
		}
		check_and_ਲੜੀਵਾਰ_traps();
		ਜੇ (r == 0) /* keyboard ^C? */
			ਜਾਰੀ; /* go back, read another input line */
		ਜੇ (r > 0) /* normal input? (no ^C, no ^D, no signals) */
			ਰੋਕੋ;
		ਜੇ (!bb_got_signal) {
			/* r < 0: ^D/EOF/error detected (but not signal) */
			/* ^D on interactive input goes to next line before exiting: */
			ਲਿਖ(STDOUT_FILENO, "\n", 1);
			i->p = ਨਿਰਾ;
			i->peek_buf[0] = r = EOF;
			ਵਾਪਸ r;
		}
		/* it was a signal: go back, read another input line */
	}
	i->p = G.user_ਨਿਵੇਸ਼_buf;
	ਵਾਪਸ (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)*i->p++;
# ਨਹੀਂਤਾਂ
	/* In !EDITING case, this function gets called for every char.
	 * Buffering happens deeper in the call chain, in hfgetc(i->file).
	 */
	ਪੂਰਨ r;

	ਲਈ (;;) {
		G.flag_SIGINT = 0;
		ਜੇ (i->ਆਖਰੀ_char == '\0' || i->ਆਖਰੀ_char == '\n') {
			ਅਚਲ ਅੱਖਰ *prompt_str = ਤਿਆਰੀ_prompt_string();
			/* Why check_and_run_traps here? Try this interactively:
			 * $ trap 'echo INT' INT; (sleep 2; kill -INT $$) &
			 * $ <[enter], repeatedly...>
			 * Without check_and_run_traps, handler never runs.
			 */
			check_and_ਲੜੀਵਾਰ_traps();
			fputs_stdout(prompt_str);
			fflush_all();
		}
		r = hfgetc(i->file);
		/* In !ENABLE_FEATURE_EDITING we don't use read_line_input,
		 * no ^C masking happens during fgetc, no special code for ^C:
		 * it generates SIGINT as usual.
		 */
		check_and_ਲੜੀਵਾਰ_traps();
		ਜੇ (r != '\0' && !G.flag_SIGINT)
			ਰੋਕੋ;
		ਜੇ (G.flag_SIGINT) {
			/* ^C or SIGINT: repeat */
			/* bash prints ^C even on real SIGINT (non-kbd generated) */
			/* kernel prints "^C" itself, just print newline: */
			ਲਿਖ(STDOUT_FILENO, "\n", 1);
			G.ਆਖਰੀ_exitcode = 128 | SIGINT;
		}
	}
	ਵਾਪਸ r;
# endif
#ਨਹੀਂਤਾਂ /* __U_BOOT__ */
	ਪੂਰਨ n;
	ਪੂਰਨ promptme;
	ਸਥਿਰ ਅੱਖਰ the_command[CONFIG_SYS_CBSIZE + 1];

	bootretry_reset_cmd_timeout();
	promptme = 1;
	n = u_boot_cli_readline(i);

# ifdef CONFIG_BOOT_RETRY_ਸਮਾਂ
	ਜੇ (n == -2) {
		puts("\nTimeout waiting for command\n");
#  ifdef CONFIG_RESET_ਮੰਜ਼ਿਲ_RETRY
		do_reset(ਨਿਰਾ, 0, 0, ਨਿਰਾ);
#  elif IS_ENABLED(CONFIG_RETRY_BOOTCMD)
	strcpy(console_ਬਫਰ, "run bootcmd\n");
# ਨਹੀਂਤਾਂ
#	error "This only works with CONFIG_RESET_TO_RETRY or CONFIG_BOOT_RETRY_COMMAND enabled"
#  endif
	}
# endif
	ਜੇ (n == -1 ) {
		G.flag_repeat = 0;
		promptme = 0;
	}
	n = ਲੜੀ_ਲੰਬਾਈ(console_ਬਫਰ);
	console_ਬਫਰ[n] = '\n';
	console_ਬਫਰ[n+1]= '\0';
	ਜੇ (had_ctrlc())
		G.flag_repeat = 0;
	clear_ctrlc();
	G.do_repeat = 0;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	ਜੇ (G.promptmode == 1) {
#ਨਹੀਂਤਾਂ /* __U_BOOT__ */
	ਜੇ (!G.promptmode) {
#ਜੇ_ਅੰਤ /* __U_BOOT__ */
		ਜੇ (console_ਬਫਰ[0] == '\n'&& G.flag_repeat == 0) {
			strcpy(the_command, console_ਬਫਰ);
		}
		ਨਹੀਂਤਾਂ {
			ਜੇ (console_ਬਫਰ[0] != '\n') {
				strcpy(the_command, console_ਬਫਰ);
				G.flag_repeat = 1;
			}
			ਨਹੀਂਤਾਂ {
				G.do_repeat = 1;
			}
		}
		i->p = the_command;
	}
	ਨਹੀਂਤਾਂ {
		ਜੇ (console_ਬਫਰ[0] != '\n') {
			ਜੇ (ਲੜੀ_ਲੰਬਾਈ(the_command) + ਲੜੀ_ਲੰਬਾਈ(console_ਬਫਰ)
				< CONFIG_SYS_CBSIZE) {
				n = ਲੜੀ_ਲੰਬਾਈ(the_command);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
				/*
				 * To avoid writing to bad places, we check if
				 * n is greater than 0.
				 * This bug was found by Harald Seiler.
				 */
				ਜੇ (n > 0)
					the_command[n-1] = ' ';
				strcpy(&the_command[n], console_ਬਫਰ);
#ਨਹੀਂਤਾਂ /* !__U_BOOT__ */
			the_command[n-1] = ' ';
			strcpy(&the_command[n], console_ਬਫਰ);
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */
				}
				ਨਹੀਂਤਾਂ {
					the_command[0] = '\n';
					the_command[1] = '\0';
					G.flag_repeat = 0;
				}
		}
		ਜੇ (promptme == 0) {
			the_command[0] = '\n';
			the_command[1] = '\0';
		}
		i->p = console_ਬਫਰ;
	}
#ਜੇ_ਅੰਤ /* __U_BOOT__ */
}
/* This is the magic location that prints prompts
 * and gets data back from the user */
ਸਥਿਰ ਪੂਰਨ fgetc_interactive(ਬਣਤਰ ਵਿੱਚ_str *i)
{
	ਪੂਰਨ ch;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	/* If it's interactive stdin, get new line. */
	ਜੇ (G_interactive_fd && i->file == G.HFILE_stdin) {
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
		/* Returns first char (or EOF), the rest is in i->p[] */
		ch = get_user_ਨਿਵੇਸ਼(i);
#ਨਹੀਂਤਾਂ /* __U_BOOT__ */
		/* Avoid garbage value and make clang happy. */
		ch = 0;
		/*
		 * get_user_input() does not return anything when used in
		 * U-Boot.
		 * So, we need to take the read character from i->p[].
		 */
		get_user_ਨਿਵੇਸ਼(i);
		ਜੇ (i->p && *i->p) {
			ch = *i->p++;
		}
#ਜੇ_ਅੰਤ /* __U_BOOT__ */
		G.promptmode = 1; /* PS2 */
		debug_printf_prompt("%s promptmode=%d\n", __func__, G.promptmode);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	} ਨਹੀਂਤਾਂ {
		/* Not stdin: script file, sourced file, etc */
		ਕਰੋ ch = hfgetc(i->file); ਜਦੋਂ (ch == '\0');
	}
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */
	ਵਾਪਸ ch;
}
#ਨਹੀਂਤਾਂ  /* !INTERACTIVE */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
ਸਥਿਰ ALWAYS_INLINE ਪੂਰਨ fgetc_interactive(ਬਣਤਰ ਵਿੱਚ_str *i)
{
	ਪੂਰਨ ch;
	ਕਰੋ ch = hfgetc(i->file); ਜਦੋਂ (ch == '\0');
	ਵਾਪਸ ch;
}
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */
#ਜੇ_ਅੰਤ  /* !INTERACTIVE */

ਸਥਿਰ ਪੂਰਨ i_getch(ਬਣਤਰ ਵਿੱਚ_str *i)
{
	ਪੂਰਨ ch;

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	ਜੇ (!i->file) {
		/* string-based in_str */
		ch = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)*i->p;
		ਜੇ (ch != '\0') {
			i->p++;
			i->ਆਖਰੀ_char = ch;
#ਜੇ ENABLE_HUSH_LINENO_VAR
			ਜੇ (ch == '\n') {
				G.parse_lineno++;
				debug_printf_parse("G.parse_lineno++ = %u\n", G.parse_lineno);
			}
#ਜੇ_ਅੰਤ
			ਵਾਪਸ ch;
		}
		ਵਾਪਸ EOF;
	}

#ਜੇ_ਅੰਤ /* !__U_BOOT__ */
	/* FILE-based in_str */

#ਜੇ ENABLE_FEATURE_EDITING
	/* This can be stdin, check line editing char[] buffer */
	ਜੇ (i->p && *i->p != '\0') {
		ch = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)*i->p++;
		ਜਾਓ ਬਾਹਰਲਾ;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	}
#ਨਹੀਂਤਾਂ /* __U_BOOT__ */
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
	} ਨਹੀਂਤਾਂ ਜੇ (i->p && *i->p == '\0' && G.ਲੜੀਵਾਰ_command_flags){
		ਵਾਪਸ EOF;
	}
#ਜੇ_ਅੰਤ /* __U_BOOT__ */
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	/* peek_buf[] is an int array, not char. Can contain EOF. */
	ch = i->peek_buf[0];
	ਜੇ (ch != 0) {
		ਪੂਰਨ ch2 = i->peek_buf[1];
		i->peek_buf[0] = ch2;
		ਜੇ (ch2 == 0) /* very likely, avoid redundant write */
			ਜਾਓ ਬਾਹਰਲਾ;
		i->peek_buf[1] = 0;
		ਜਾਓ ਬਾਹਰਲਾ;
	}

#ਜੇ_ਅੰਤ /* !__U_BOOT__ */
	ch = fgetc_interactive(i);
 ਬਾਹਰਲਾ:
	debug_printf("file_get: got '%c' %d\n", ch, ch);
	i->ਆਖਰੀ_char = ch;
#ਜੇ ENABLE_HUSH_LINENO_VAR
	ਜੇ (ch == '\n') {
		G.parse_lineno++;
		debug_printf_parse("G.parse_lineno++ = %u\n", G.parse_lineno);
	}
#ਜੇ_ਅੰਤ
	ਵਾਪਸ ch;
}

ਸਥਿਰ ਪੂਰਨ i_peek(ਬਣਤਰ ਵਿੱਚ_str *i)
{
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	ਪੂਰਨ ch;

	ਜੇ (!i->file) {
		/* string-based in_str */
		/* Doesn't report EOF on NUL. None of the callers care. */
		ਵਾਪਸ (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)*i->p;
	}

	/* FILE-based in_str */

#ਜੇ ENABLE_FEATURE_EDITING && ENABLE_HUSH_INTERACTIVE
	/* This can be stdin, check line editing char[] buffer */
	ਜੇ (i->p && *i->p != '\0')
		ਵਾਪਸ (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)*i->p;
#ਜੇ_ਅੰਤ
	/* peek_buf[] is an int array, not char. Can contain EOF. */
	ch = i->peek_buf[0];
	ਜੇ (ch != 0)
		ਵਾਪਸ ch;

	/* Need to get a new char */
	ch = fgetc_interactive(i);
	debug_printf("file_peek: got '%c' %d\n", ch, ch);

	/* Save it by either rolling back line editing buffer, or in i->peek_buf[0] */
#ਜੇ ENABLE_FEATURE_EDITING && ENABLE_HUSH_INTERACTIVE
	ਜੇ (i->p) {
		i->p -= 1;
		ਵਾਪਸ ch;
	}
#ਜੇ_ਅੰਤ
	i->peek_buf[0] = ch;
	/*i->peek_buf[1] = 0; - already is */
	ਵਾਪਸ ch;
#ਨਹੀਂਤਾਂ /* __U_BOOT__ */
	/* string-based in_str */
	/* Doesn't report EOF on NUL. None of the callers care. */
	ਵਾਪਸ (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)*i->p;
#ਜੇ_ਅੰਤ /* __U_BOOT__ */
}

/* Only ever called if i_peek() was called, and did not return EOF.
 * IOW: we know the previous peek saw an ordinary char, not EOF, not NUL,
 * not end-of-line. Therefore we never need to read a new editing line here.
 */
ਸਥਿਰ ਪੂਰਨ i_peek2(ਬਣਤਰ ਵਿੱਚ_str *i)
{
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	ਪੂਰਨ ch;
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */

	/* There are two cases when i->p[] buffer exists.
	 * (1) it's a string in_str.
	 * (2) It's a file, and we have a saved line editing buffer.
	 * In both cases, we know that i->p[0] exists and not NUL, and
	 * the peek2 result is in i->p[1].
	 */
	ਜੇ (i->p)
		ਵਾਪਸ (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)i->p[1];

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	/* Now we know it is a file-based in_str. */

	/* peek_buf[] is an int array, not char. Can contain EOF. */
	/* Is there 2nd char? */
	ch = i->peek_buf[1];
	ਜੇ (ch == 0) {
		/* We did not read it yet, get it now */
		ਕਰੋ ch = hfgetc(i->file); ਜਦੋਂ (ch == '\0');
		i->peek_buf[1] = ch;
	}

	debug_printf("file_peek2: got '%c' %d\n", ch, ch);
	ਵਾਪਸ ch;
#ਨਹੀਂਤਾਂ
	ਵਾਪਸ 0;
#ਜੇ_ਅੰਤ /* __U_BOOT__ */
}

ਸਥਿਰ ਪੂਰਨ i_getch_and_eat_bkslash_nl(ਬਣਤਰ ਵਿੱਚ_str *ਨਿਵੇਸ਼)
{
	ਲਈ (;;) {
		ਪੂਰਨ ch, ch2;

		ch = i_getch(ਨਿਵੇਸ਼);
		ਜੇ (ch != '\\')
			ਵਾਪਸ ch;
		ch2 = i_peek(ਨਿਵੇਸ਼);
		ਜੇ (ch2 != '\n')
			ਵਾਪਸ ch;
		/* backslash+newline, skip it */
		i_getch(ਨਿਵੇਸ਼);
	}
}

/* Note: this function _eats_ \<newline> pairs, safe to use plain
 * i_getch() after it instead of i_getch_and_eat_bkslash_nl().
 */
ਸਥਿਰ ਪੂਰਨ i_peek_and_eat_bkslash_nl(ਬਣਤਰ ਵਿੱਚ_str *ਨਿਵੇਸ਼)
{
	ਲਈ (;;) {
		ਪੂਰਨ ch, ch2;

		ch = i_peek(ਨਿਵੇਸ਼);
		ਜੇ (ch != '\\')
			ਵਾਪਸ ch;
		ch2 = i_peek2(ਨਿਵੇਸ਼);
		ਜੇ (ch2 != '\n')
			ਵਾਪਸ ch;
		/* backslash+newline, skip it */
		i_getch(ਨਿਵੇਸ਼);
		i_getch(ਨਿਵੇਸ਼);
	}
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
ਸਥਿਰ ਖਾਲੀ ਤਿਆਰੀ_file_ਵਿੱਚ_str(ਬਣਤਰ ਵਿੱਚ_str *i, HFILE *fp)
#ਨਹੀਂਤਾਂ /* __U_BOOT__ */
ਸਥਿਰ ਖਾਲੀ ਤਿਆਰੀ_file_ਵਿੱਚ_str(ਬਣਤਰ ਵਿੱਚ_str *i)
#ਜੇ_ਅੰਤ /* __U_BOOT__ */
{
	memset(i, 0, ਆਕਾਰ(*i));
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	i->file = fp;
	/* i->p = NULL; */
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */
}

ਸਥਿਰ ਖਾਲੀ ਤਿਆਰੀ_string_ਵਿੱਚ_str(ਬਣਤਰ ਵਿੱਚ_str *i, ਅਚਲ ਅੱਖਰ *s)
{
	memset(i, 0, ਆਕਾਰ(*i));
	/*i->file = NULL */;
	i->p = s;
}

/*
 * o_string support
 */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ B_CHUNK  (32 * ਆਕਾਰ(ਅੱਖਰ*))

ਸਥਿਰ ਖਾਲੀ o_reset_ਮੰਜ਼ਿਲ_ਖਾਲੀ_ਥਾਵਾਂ_unquoted(o_string *o)
{
	o->ਲੰਮਾਈ = 0;
	o->has_quoted_part = 0;
	ਜੇ (o->ਅੰਕੜੇ)
		o->ਅੰਕੜੇ[0] = '\0';
}

ਸਥਿਰ ਖਾਲੀ o_free_and_ਸੈੱਟ_NULL(o_string *o)
{
	ਮੁਕਤ(o->ਅੰਕੜੇ);
	memset(o, 0, ਆਕਾਰ(*o));
}

ਸਥਿਰ ALWAYS_INLINE ਖਾਲੀ o_free(o_string *o)
{
	ਮੁਕਤ(o->ਅੰਕੜੇ);
}

ਸਥਿਰ ਖਾਲੀ o_grow_by(o_string *o, ਪੂਰਨ len)
{
	ਜੇ (o->ਲੰਮਾਈ + len > o->maxlen) {
		o->maxlen += (2 * len) | (B_CHUNK-1);
		o->ਅੰਕੜੇ = xrealloc(o->ਅੰਕੜੇ, 1 + o->maxlen);
	}
}

ਸਥਿਰ ਖਾਲੀ o_addchr(o_string *o, ਪੂਰਨ ch)
{
	debug_printf("o_addchr: '%c' o->length=%d o=%p\n", ch, o->ਲੰਮਾਈ, o);
	ਜੇ (o->ਲੰਮਾਈ < o->maxlen) {
		/* likely. avoid o_grow_by() call */
 ਜੋੜੋ:
		o->ਅੰਕੜੇ[o->ਲੰਮਾਈ] = ch;
		o->ਲੰਮਾਈ++;
		o->ਅੰਕੜੇ[o->ਲੰਮਾਈ] = '\0';
		ਵਾਪਸ;
	}
	o_grow_by(o, 1);
	ਜਾਓ ਜੋੜੋ;
}

#ਜੇ 0
/* Valid only if we know o_string is not empty */
ਸਥਿਰ ਖਾਲੀ o_delchr(o_string *o)
{
	o->ਲੰਮਾਈ--;
	o->ਅੰਕੜੇ[o->ਲੰਮਾਈ] = '\0';
}
#ਜੇ_ਅੰਤ

ਸਥਿਰ ਖਾਲੀ o_addblock(o_string *o, ਅਚਲ ਅੱਖਰ *str, ਪੂਰਨ len)
{
	o_grow_by(o, len);
	((ਅੱਖਰ*)mempcpy(&o->ਅੰਕੜੇ[o->ਲੰਮਾਈ], str, len))[0] = '\0';
	o->ਲੰਮਾਈ += len;
}

ਸਥਿਰ ਖਾਲੀ o_addstr(o_string *o, ਅਚਲ ਅੱਖਰ *str)
{
	o_addblock(o, str, ਲੜੀ_ਲੰਬਾਈ(str));
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
ਸਥਿਰ ਖਾਲੀ o_addstr_with_NUL(o_string *o, ਅਚਲ ਅੱਖਰ *str)
{
	o_addblock(o, str, ਲੜੀ_ਲੰਬਾਈ(str) + 1);
}
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */

#ਜੇ !BB_MMU
ਸਥਿਰ ਖਾਲੀ nommu_addchr(o_string *o, ਪੂਰਨ ch)
{
	ਜੇ (o)
		o_addchr(o, ch);
}
#ਨਹੀਂਤਾਂ
# define nommu_addchr(o, str) ((ਖਾਲੀ)0)
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
#ਜੇ ENABLE_HUSH_ਬਹੁਲਕ_X
ਸਥਿਰ ਖਾਲੀ x_ਬਹੁਲਕ_addchr(ਪੂਰਨ ch)
{
	o_addchr(&G.x_ਬਹੁਲਕ_buf, ch);
}
ਸਥਿਰ ਖਾਲੀ x_ਬਹੁਲਕ_addstr(ਅਚਲ ਅੱਖਰ *str)
{
	o_addstr(&G.x_ਬਹੁਲਕ_buf, str);
}
ਸਥਿਰ ਖਾਲੀ x_ਬਹੁਲਕ_addblock(ਅਚਲ ਅੱਖਰ *str, ਪੂਰਨ len)
{
	o_addblock(&G.x_ਬਹੁਲਕ_buf, str, len);
}
ਸਥਿਰ ਖਾਲੀ x_ਬਹੁਲਕ_ਅਗੇਤਰ(ਖਾਲੀ)
{
	ਪੂਰਨ n = G.x_ਬਹੁਲਕ_ਡੂੰਘਾਈ;
	ਕਰੋ x_ਬਹੁਲਕ_addchr('+'); ਜਦੋਂ (--n >= 0);
}
ਸਥਿਰ ਖਾਲੀ x_ਬਹੁਲਕ_flush(ਖਾਲੀ)
{
	ਪੂਰਨ len = G.x_ਬਹੁਲਕ_buf.ਲੰਮਾਈ;
	ਜੇ (len <= 0)
		ਵਾਪਸ;
	ਜੇ (G.x_ਬਹੁਲਕ_fd > 0) {
		G.x_ਬਹੁਲਕ_buf.ਅੰਕੜੇ[len] = '\n';
		ਭਰਿਆ_write(G.x_ਬਹੁਲਕ_fd, G.x_ਬਹੁਲਕ_buf.ਅੰਕੜੇ, len + 1);
	}
	G.x_ਬਹੁਲਕ_buf.ਲੰਮਾਈ = 0;
}
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */

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

#ਜੇ ENABLE_HUSH_BRACE_EXPANSION
# define MAYBE_BRACES "{}"
#ਨਹੀਂਤਾਂ
# define MAYBE_BRACES ""
#ਜੇ_ਅੰਤ

/* My analysis of quoting semantics tells me that state information
 * is associated with a destination, not a source.
 */
ਸਥਿਰ ਖਾਲੀ o_addqchr(o_string *o, ਪੂਰਨ ch)
{
	ਪੂਰਨ sz = 1;
	/* '-' is included because of this case:
	 * >filename0 >filename1 >filename9; v='-'; echo filename[0"$v"9]
	 */
	ਅੱਖਰ *ਮਿਲਿਆ = strchr("*?[-\\" MAYBE_BRACES, ch);
	ਜੇ (ਮਿਲਿਆ)
		sz++;
	o_grow_by(o, sz);
	ਜੇ (ਮਿਲਿਆ) {
		o->ਅੰਕੜੇ[o->ਲੰਮਾਈ] = '\\';
		o->ਲੰਮਾਈ++;
	}
	o->ਅੰਕੜੇ[o->ਲੰਮਾਈ] = ch;
	o->ਲੰਮਾਈ++;
	o->ਅੰਕੜੇ[o->ਲੰਮਾਈ] = '\0';
}

ਸਥਿਰ ਖਾਲੀ o_ਜੋੜੋ_Qchr(o_string *o, ਪੂਰਨ ch)
{
	ਪੂਰਨ sz = 1;
	ਜੇ ((o->o_expflags & EXP_FLAG_ESC_GLOB_CHARS)
	 && strchr("*?[-\\" MAYBE_BRACES, ch)
	) {
		sz++;
		o->ਅੰਕੜੇ[o->ਲੰਮਾਈ] = '\\';
		o->ਲੰਮਾਈ++;
	}
	o_grow_by(o, sz);
	o->ਅੰਕੜੇ[o->ਲੰਮਾਈ] = ch;
	o->ਲੰਮਾਈ++;
	o->ਅੰਕੜੇ[o->ਲੰਮਾਈ] = '\0';
}

ਸਥਿਰ ਖਾਲੀ o_addqblock(o_string *o, ਅਚਲ ਅੱਖਰ *str, ਪੂਰਨ len)
{
	ਜਦੋਂ (len) {
		ਅੱਖਰ ch;
		ਪੂਰਨ sz;
		ਪੂਰਨ ordinary_cnt = strcspn(str, "*?[-\\" MAYBE_BRACES);
		ਜੇ (ordinary_cnt > len) /* paranoia */
			ordinary_cnt = len;
		o_addblock(o, str, ordinary_cnt);
		ਜੇ (ordinary_cnt == len)
			ਵਾਪਸ; /* NUL is already added by o_addblock */
		str += ordinary_cnt;
		len -= ordinary_cnt + 1; /* we are processing + 1 char below */

		ch = *str++;
		sz = 1;
		ਜੇ (ch) { /* it is necessarily one of "*?[-\\" MAYBE_BRACES */
			sz++;
			o->ਅੰਕੜੇ[o->ਲੰਮਾਈ] = '\\';
			o->ਲੰਮਾਈ++;
		}
		o_grow_by(o, sz);
		o->ਅੰਕੜੇ[o->ਲੰਮਾਈ] = ch;
		o->ਲੰਮਾਈ++;
	}
	o->ਅੰਕੜੇ[o->ਲੰਮਾਈ] = '\0';
}

ਸਥਿਰ ਖਾਲੀ o_ਜੋੜੋ_Qblock(o_string *o, ਅਚਲ ਅੱਖਰ *str, ਪੂਰਨ len)
{
	ਜੇ (!(o->o_expflags & EXP_FLAG_ESC_GLOB_CHARS)) {
		o_addblock(o, str, len);
		ਵਾਪਸ;
	}
	o_addqblock(o, str, len);
}

ਸਥਿਰ ਖਾਲੀ o_ਜੋੜੋ_Qstr(o_string *o, ਅਚਲ ਅੱਖਰ *str)
{
	o_ਜੋੜੋ_Qblock(o, str, ਲੜੀ_ਲੰਬਾਈ(str));
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
#ਜੇ DEBUG_EXPAND || DEBUG_GLOB
ਸਥਿਰ ਖਾਲੀ debug_print_list(ਅਚਲ ਅੱਖਰ *ਅਗੇਤਰ, o_string *o, ਪੂਰਨ n)
{
	ਅੱਖਰ **list = (ਅੱਖਰ**)o->ਅੰਕੜੇ;
	ਪੂਰਨ string_ਸ਼ੁਰੂ = ((n + 0xf) & ~0xf) * ਆਕਾਰ(list[0]);
	ਪੂਰਨ i = 0;

	indent();
	fdprintf(2, "%s: list:%p n:%d string_start:%d length:%d maxlen:%d glob:%d quoted:%d escape:%d\n",
			ਅਗੇਤਰ, list, n, string_ਸ਼ੁਰੂ, o->ਲੰਮਾਈ, o->maxlen,
			!!(o->o_expflags & EXP_FLAG_GLOB),
			o->has_quoted_part,
			!!(o->o_expflags & EXP_FLAG_ESC_GLOB_CHARS));
	ਜਦੋਂ (i < n) {
		indent();
		fdprintf(2, " list[%d]=%d '%s' %p\n", i, (ਪੂਰਨ)(uintptr_t)list[i],
				o->ਅੰਕੜੇ + (ਪੂਰਨ)(uintptr_t)list[i] + string_ਸ਼ੁਰੂ,
				o->ਅੰਕੜੇ + (ਪੂਰਨ)(uintptr_t)list[i] + string_ਸ਼ੁਰੂ);
		i++;
	}
	ਜੇ (n) {
		ਅਚਲ ਅੱਖਰ *p = o->ਅੰਕੜੇ + (ਪੂਰਨ)(uintptr_t)list[n - 1] + string_ਸ਼ੁਰੂ;
		indent();
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
		fdprintf(2, " total_sz:%ld\n", (ਲੰਮਾ)((p + ਲੜੀ_ਲੰਬਾਈ(p) + 1) - o->ਅੰਕੜੇ));
#ਨਹੀਂਤਾਂ /* __U_BOOT__ */
		ਲਿਖੋ(" total_sz:%ld\n", (ਲੰਮਾ)((p + ਲੜੀ_ਲੰਬਾਈ(p) + 1) - o->ਅੰਕੜੇ));
#ਜੇ_ਅੰਤ /* __U_BOOT__ */
	}
}
#ਨਹੀਂਤਾਂ
# define debug_print_list(ਅਗੇਤਰ, o, n) ((ਖਾਲੀ)0)
#ਜੇ_ਅੰਤ

/* n = o_save_ptr_helper(str, n) "starts new string" by storing an index value
 * in list[n] so that it points past last stored byte so far.
 * It returns n+1. */
ਸਥਿਰ ਪੂਰਨ o_save_ptr_helper(o_string *o, ਪੂਰਨ n)
{
	ਅੱਖਰ **list = (ਅੱਖਰ**)o->ਅੰਕੜੇ;
	ਪੂਰਨ string_ਸ਼ੁਰੂ;
	ਪੂਰਨ string_len;

	ਜੇ (!o->has_ਖਾਲੀ_ਥਾਵਾਂ_ਥਾਂ) {
		string_ਸ਼ੁਰੂ = ((n + 0xf) & ~0xf) * ਆਕਾਰ(list[0]);
		string_len = o->ਲੰਮਾਈ - string_ਸ਼ੁਰੂ;
		ਜੇ (!(n & 0xf)) { /* 0, 0x10, 0x20...? */
			debug_printf_list("list[%d]=%d string_start=%d (growing)\n", n, string_len, string_ਸ਼ੁਰੂ);
			/* list[n] points to string_start, make space for 16 more pointers */
			o->maxlen += 0x10 * ਆਕਾਰ(list[0]);
			o->ਅੰਕੜੇ = xrealloc(o->ਅੰਕੜੇ, o->maxlen + 1);
			list = (ਅੱਖਰ**)o->ਅੰਕੜੇ;
			memmove(list + n + 0x10, list + n, string_len);
			/*
			 * expand_on_ifs() has a "previous argv[] ends in IFS?"
			 * check. (grep for -prev-ifs-check-).
			 * Ensure that argv[-1][last] is not garbage
			 * but zero bytes, to save index check there.
			 */
			list[n + 0x10 - 1] = 0;
			o->ਲੰਮਾਈ += 0x10 * ਆਕਾਰ(list[0]);
		} ਨਹੀਂਤਾਂ {
			debug_printf_list("list[%d]=%d string_start=%d\n",
					n, string_len, string_ਸ਼ੁਰੂ);
		}
	} ਨਹੀਂਤਾਂ {
		/* We have empty slot at list[n], reuse without growth */
		string_ਸ਼ੁਰੂ = ((n+1 + 0xf) & ~0xf) * ਆਕਾਰ(list[0]); /* NB: n+1! */
		string_len = o->ਲੰਮਾਈ - string_ਸ਼ੁਰੂ;
		debug_printf_list("list[%d]=%d string_start=%d (empty slot)\n",
				n, string_len, string_ਸ਼ੁਰੂ);
		o->has_ਖਾਲੀ_ਥਾਵਾਂ_ਥਾਂ = 0;
	}
	o->has_quoted_part = 0;
	list[n] = (ਅੱਖਰ*)(uintptr_t)string_len;
	ਵਾਪਸ n + 1;
}

/* "What was our last o_save_ptr'ed position (byte offset relative o->data)?" */
ਸਥਿਰ ਪੂਰਨ o_get_ਆਖਰੀ_ptr(o_string *o, ਪੂਰਨ n)
{
	ਅੱਖਰ **list = (ਅੱਖਰ**)o->ਅੰਕੜੇ;
	ਪੂਰਨ string_ਸ਼ੁਰੂ = ((n + 0xf) & ~0xf) * ਆਕਾਰ(list[0]);

	ਵਾਪਸ ((ਪੂਰਨ)(uintptr_t)list[n-1]) + string_ਸ਼ੁਰੂ;
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

#ਜੇ ENABLE_HUSH_BRACE_EXPANSION
/* There in a GNU extension, GLOB_BRACE, but it is not usable:
 * first, it processes even {a} (no commas), second,
 * I didn't manage to make it return strings when they don't match
 * existing files. Need to re-implement it.
 */

/* Helper */
ਸਥਿਰ ਪੂਰਨ glob_needed(ਅਚਲ ਅੱਖਰ *s)
{
	ਜਦੋਂ (*s) {
		ਜੇ (*s == '\\') {
			ਜੇ (!s[1])
				ਵਾਪਸ 0;
			s += 2;
			ਜਾਰੀ;
		}
		ਜੇ (*s == '*' || *s == '[' || *s == '?' || *s == '{')
			ਵਾਪਸ 1;
		s++;
	}
	ਵਾਪਸ 0;
}
/* Return pointer to next closing brace or to comma */
ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਗਲਾ_brace_sub(ਅਚਲ ਅੱਖਰ *cp)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਡੂੰਘਾਈ = 0;
	cp++;
	ਜਦੋਂ (*cp != '\0') {
		ਜੇ (*cp == '\\') {
			ਜੇ (*++cp == '\0')
				ਰੋਕੋ;
			cp++;
			ਜਾਰੀ;
		}
		ਜੇ ((*cp == '}' && ਡੂੰਘਾਈ-- == 0) || (*cp == ',' && ਡੂੰਘਾਈ == 0))
			ਰੋਕੋ;
		ਜੇ (*cp++ == '{')
			ਡੂੰਘਾਈ++;
	}

	ਵਾਪਸ *cp != '\0' ? cp : ਨਿਰਾ;
}
/* Recursive brace globber. Note: may garble pattern[]. */
ਸਥਿਰ ਪੂਰਨ glob_brace(ਅੱਖਰ *ਨਮੂਨਾ, o_string *o, ਪੂਰਨ n)
{
	ਅੱਖਰ *new_ਨਮੂਨਾ_buf;
	ਅਚਲ ਅੱਖਰ *begin;
	ਅਚਲ ਅੱਖਰ *ਅਗਲਾ;
	ਅਚਲ ਅੱਖਰ *rest;
	ਅਚਲ ਅੱਖਰ *p;
	size_t rest_len;

	debug_printf_glob("glob_brace('%s')\n", ਨਮੂਨਾ);

	begin = ਨਮੂਨਾ;
	ਜਦੋਂ (1) {
		ਜੇ (*begin == '\0')
			ਜਾਓ simple_glob;
		ਜੇ (*begin == '{') {
			/* Find the first sub-pattern and at the same time
			 * find the rest after the closing brace */
			ਅਗਲਾ = ਅਗਲਾ_brace_sub(begin);
			ਜੇ (ਅਗਲਾ == ਨਿਰਾ) {
				/* An illegal expression */
				ਜਾਓ simple_glob;
			}
			ਜੇ (*ਅਗਲਾ == '}') {
				/* "{abc}" with no commas - illegal
				 * brace expr, disregard and skip it */
				begin = ਅਗਲਾ + 1;
				ਜਾਰੀ;
			}
			ਰੋਕੋ;
		}
		ਜੇ (*begin == '\\' && begin[1] != '\0')
			begin++;
		begin++;
	}
	debug_printf_glob("begin:%s\n", begin);
	debug_printf_glob("next:%s\n", ਅਗਲਾ);

	/* Now find the end of the whole brace expression */
	rest = ਅਗਲਾ;
	ਜਦੋਂ (*rest != '}') {
		rest = ਅਗਲਾ_brace_sub(rest);
		ਜੇ (rest == ਨਿਰਾ) {
			/* An illegal expression */
			ਜਾਓ simple_glob;
		}
		debug_printf_glob("rest:%s\n", rest);
	}
	rest_len = ਲੜੀ_ਲੰਬਾਈ(++rest) + 1;

	/* We are sure the brace expression is well-formed */

	/* Allocate working buffer large enough for our work */
	new_ਨਮੂਨਾ_buf = xmalloc(ਲੜੀ_ਲੰਬਾਈ(ਨਮੂਨਾ));

	/* We have a brace expression.  BEGIN points to the opening {,
	 * NEXT points past the terminator of the first element, and REST
	 * points past the final }.  We will accumulate result names from
	 * recursive runs for each brace alternative in the buffer using
	 * GLOB_APPEND. */

	p = begin + 1;
	ਜਦੋਂ (1) {
		/* Construct the new glob expression */
		ਸਮ੍ਰਿਤੀ_ਨਕਲ(
			mempcpy(
				mempcpy(new_ਨਮੂਨਾ_buf,
					/* We know the prefix for all sub-patterns */
					ਨਮੂਨਾ, begin - ਨਮੂਨਾ),
				p, ਅਗਲਾ - p),
			rest, rest_len);

		/* Note: glob_brace() may garble new_pattern_buf[].
		 * That's why we re-copy prefix every time (1st memcpy above).
		 */
		n = glob_brace(new_ਨਮੂਨਾ_buf, o, n);
		ਜੇ (*ਅਗਲਾ == '}') {
			/* We saw the last entry */
			ਰੋਕੋ;
		}
		p = ਅਗਲਾ + 1;
		ਅਗਲਾ = ਅਗਲਾ_brace_sub(ਅਗਲਾ);
	}
	ਮੁਕਤ(new_ਨਮੂਨਾ_buf);
	ਵਾਪਸ n;

 simple_glob:
	{
		ਪੂਰਨ gr;
		glob_t globdata;

		memset(&globdata, 0, ਆਕਾਰ(globdata));
		gr = glob(ਨਮੂਨਾ, 0, ਨਿਰਾ, &globdata);
		debug_printf_glob("glob('%s'):%d\n", ਨਮੂਨਾ, gr);
		ਜੇ (gr != 0) {
			ਜੇ (gr == GLOB_NOMATCH) {
				globfree(&globdata);
				/* NB: garbles parameter */
				unbackslash(ਨਮੂਨਾ);
				o_addstr_with_NUL(o, ਨਮੂਨਾ);
				debug_printf_glob("glob pattern '%s' is literal\n", ਨਮੂਨਾ);
				ਵਾਪਸ o_save_ptr_helper(o, n);
			}
			ਜੇ (gr == GLOB_NOSPACE)
				bb_die_ਸਮ੍ਰਿਤੀ_exhausted();
			/* GLOB_ABORTED? Only happens with GLOB_ERR flag,
			 * but we didn't specify it. Paranoia again. */
			bb_error_ਸੁਨੇਹਾ_and_die("glob error %d on '%s'", gr, ਨਮੂਨਾ);
		}
		ਜੇ (globdata.gl_pathv && globdata.gl_pathv[0]) {
			ਅੱਖਰ **argv = globdata.gl_pathv;
			ਜਦੋਂ (1) {
				o_addstr_with_NUL(o, *argv);
				n = o_save_ptr_helper(o, n);
				argv++;
				ਜੇ (!*argv)
					ਰੋਕੋ;
			}
		}
		globfree(&globdata);
	}
	ਵਾਪਸ n;
}
/* Performs globbing on last list[],
 * saving each result as a new list[].
 */
ਸਥਿਰ ਪੂਰਨ perform_glob(o_string *o, ਪੂਰਨ n)
{
	ਅੱਖਰ *ਨਮੂਨਾ, *copy;

	debug_printf_glob("start perform_glob: n:%d o->data:%p\n", n, o->ਅੰਕੜੇ);
	ਜੇ (!o->ਅੰਕੜੇ)
		ਵਾਪਸ o_save_ptr_helper(o, n);
	ਨਮੂਨਾ = o->ਅੰਕੜੇ + o_get_ਆਖਰੀ_ptr(o, n);
	debug_printf_glob("glob pattern '%s'\n", ਨਮੂਨਾ);
	ਜੇ (!glob_needed(ਨਮੂਨਾ)) {
		/* unbackslash last string in o in place, fix length */
		o->ਲੰਮਾਈ = unbackslash(ਨਮੂਨਾ) - o->ਅੰਕੜੇ;
		debug_printf_glob("glob pattern '%s' is literal\n", ਨਮੂਨਾ);
		ਵਾਪਸ o_save_ptr_helper(o, n);
	}

	copy = xstrdup(ਨਮੂਨਾ);
	/* "forget" pattern in o */
	o->ਲੰਮਾਈ = ਨਮੂਨਾ - o->ਅੰਕੜੇ;
	n = glob_brace(copy, o, n);
	ਮੁਕਤ(copy);
	ਜੇ (DEBUG_GLOB)
		debug_print_list("perform_glob returning", o, n);
	ਵਾਪਸ n;
}

#ਨਹੀਂਤਾਂ /* !HUSH_BRACE_EXPANSION */

/* Helper */
ਸਥਿਰ ਪੂਰਨ glob_needed(ਅਚਲ ਅੱਖਰ *s)
{
	ਜਦੋਂ (*s) {
		ਜੇ (*s == '\\') {
			ਜੇ (!s[1])
				ਵਾਪਸ 0;
			s += 2;
			ਜਾਰੀ;
		}
		ਜੇ (*s == '*' || *s == '[' || *s == '?')
			ਵਾਪਸ 1;
		s++;
	}
	ਵਾਪਸ 0;
}
/* Performs globbing on last list[],
 * saving each result as a new list[].
 */
ਸਥਿਰ ਪੂਰਨ perform_glob(o_string *o, ਪੂਰਨ n)
{
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	glob_t globdata;
	ਪੂਰਨ gr;
#ਜੇ_ਅੰਤ /* __U_BOOT__ */
	ਅੱਖਰ *ਨਮੂਨਾ;

	debug_printf_glob("start perform_glob: n:%d o->data:%p\n", n, o->ਅੰਕੜੇ);
	ਜੇ (!o->ਅੰਕੜੇ)
		ਵਾਪਸ o_save_ptr_helper(o, n);
	ਨਮੂਨਾ = o->ਅੰਕੜੇ + o_get_ਆਖਰੀ_ptr(o, n);
	debug_printf_glob("glob pattern '%s'\n", ਨਮੂਨਾ);
	ਜੇ (!glob_needed(ਨਮੂਨਾ)) {
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
 literal:
#ਜੇ_ਅੰਤ /* __U_BOOT__ */
		/* unbackslash last string in o in place, fix length */
		o->ਲੰਮਾਈ = unbackslash(ਨਮੂਨਾ) - o->ਅੰਕੜੇ;
		debug_printf_glob("glob pattern '%s' is literal\n", ਨਮੂਨਾ);
		ਵਾਪਸ o_save_ptr_helper(o, n);
	}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	memset(&globdata, 0, ਆਕਾਰ(globdata));
	/* Can't use GLOB_NOCHECK: it does not unescape the string.
	 * If we glob "*.\*" and don't find anything, we need
	 * to fall back to using literal "*.*", but GLOB_NOCHECK
	 * will return "*.\*"!
	 */
	gr = glob(ਨਮੂਨਾ, 0, ਨਿਰਾ, &globdata);
	debug_printf_glob("glob('%s'):%d\n", ਨਮੂਨਾ, gr);
	ਜੇ (gr != 0) {
		ਜੇ (gr == GLOB_NOMATCH) {
			globfree(&globdata);
			ਜਾਓ literal;
		}
		ਜੇ (gr == GLOB_NOSPACE)
			bb_die_ਸਮ੍ਰਿਤੀ_exhausted();
		/* GLOB_ABORTED? Only happens with GLOB_ERR flag,
		 * but we didn't specify it. Paranoia again. */
		bb_error_ਸੁਨੇਹਾ_and_die("glob error %d on '%s'", gr, ਨਮੂਨਾ);
	}
	ਜੇ (globdata.gl_pathv && globdata.gl_pathv[0]) {
		ਅੱਖਰ **argv = globdata.gl_pathv;
		/* "forget" pattern in o */
		o->ਲੰਮਾਈ = ਨਮੂਨਾ - o->ਅੰਕੜੇ;
		ਜਦੋਂ (1) {
			o_addstr_with_NUL(o, *argv);
			n = o_save_ptr_helper(o, n);
			argv++;
			ਜੇ (!*argv)
				ਰੋਕੋ;
		}
	}
	globfree(&globdata);
	ਜੇ (DEBUG_GLOB)
		debug_print_list("perform_glob returning", o, n);
	ਵਾਪਸ n;
#ਨਹੀਂਤਾਂ /* __U_BOOT__ */
	/*
	 * NOTE We only use perform glob to call unbackslash to remove backslash
	 * from string once expanded.
	 * So, it seems OK to return this if no previous return was done.
	 */
	ਵਾਪਸ o_save_ptr_helper(o, n);
#ਜੇ_ਅੰਤ /* __U_BOOT__ */
}

#ਜੇ_ਅੰਤ /* !HUSH_BRACE_EXPANSION */

/* If o->o_expflags & EXP_FLAG_GLOB, glob the string so far remembered.
 * Otherwise, just finish current list[] and start new */
ਸਥਿਰ ਪੂਰਨ o_save_ptr(o_string *o, ਪੂਰਨ n)
{
	ਜੇ (o->o_expflags & EXP_FLAG_GLOB) {
		/* If o->has_empty_slot, list[n] was already globbed
		 * (if it was requested back then when it was filled)
		 * so don't do that again! */
		ਜੇ (!o->has_ਖਾਲੀ_ਥਾਵਾਂ_ਥਾਂ)
			ਵਾਪਸ perform_glob(o, n); /* o_save_ptr_helper is inside */
	}
	ਵਾਪਸ o_save_ptr_helper(o, n);
}

/* "Please convert list[n] to real char* ptrs, and NULL terminate it." */
ਸਥਿਰ ਅੱਖਰ **o_finalize_list(o_string *o, ਪੂਰਨ n)
{
	ਅੱਖਰ **list;
	ਪੂਰਨ string_ਸ਼ੁਰੂ;

	ਜੇ (DEBUG_EXPAND)
		debug_print_list("finalized", o, n);
	debug_printf_expand("finalized n:%d\n", n);
	list = (ਅੱਖਰ**)o->ਅੰਕੜੇ;
	string_ਸ਼ੁਰੂ = ((n + 0xf) & ~0xf) * ਆਕਾਰ(list[0]);
	list[--n] = ਨਿਰਾ;
	ਜਦੋਂ (n) {
		n--;
		list[n] = o->ਅੰਕੜੇ + (ਪੂਰਨ)(uintptr_t)list[n] + string_ਸ਼ੁਰੂ;
	}
	ਵਾਪਸ list;
}

ਸਥਿਰ ਖਾਲੀ free_pipe_list(ਬਣਤਰ pipe *pi);

/* Returns pi->next - next pipe in the list */
ਸਥਿਰ ਬਣਤਰ pipe *free_pipe(ਬਣਤਰ pipe *pi)
{
	ਬਣਤਰ pipe *ਅਗਲਾ;
	ਪੂਰਨ i;

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	debug_printf_clean("free_pipe (pid %d)\n", getpid());
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */
	ਲਈ (i = 0; i < pi->ਅੰਕ_cmds; i++) {
		ਬਣਤਰ command *command;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
		ਬਣਤਰ redir_struct *r, *rnext;
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */

		command = &pi->cmds[i];
		debug_printf_clean("  command %d:\n", i);
		ਜੇ (command->argv) {
			ਜੇ (DEBUG_CLEAN) {
				ਪੂਰਨ a;
				ਅੱਖਰ **p;
				ਲਈ (a = 0, p = command->argv; *p; a++, p++) {
					debug_printf_clean("   argv[%d] = %s\n", a, *p);
				}
			}
			free_ਲੜੀਆਂ(command->argv);
			//command->argv = NULL;
		}
		/* not "else if": on syntax error, we may have both! */
		ਜੇ (command->ਸਮੂਹ) {
			debug_printf_clean("   begin group (cmd_type:%d)\n",
					command->cmd_type);
			free_pipe_list(command->ਸਮੂਹ);
			debug_printf_clean("   end group\n");
			//command->group = NULL;
		}
		/* else is crucial here.
		 * If group != NULL, child_func is meaningless */
#ਜੇ ENABLE_HUSH_FUNCTIONS
		ਨਹੀਂਤਾਂ ਜੇ (command->ਬੱਚਾ_func) {
			debug_printf_exec("cmd %p releases child func at %p\n", command, command->ਬੱਚਾ_func);
			command->ਬੱਚਾ_func->ਮਾਪਾ_cmd = ਨਿਰਾ;
		}
#ਜੇ_ਅੰਤ
#ਜੇ !BB_MMU
		ਮੁਕਤ(command->ਸਮੂਹ_as_string);
		//command->group_as_string = NULL;
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
		ਲਈ (r = command->redirects; r; r = rnext) {
			debug_printf_clean("   redirect %d%s",
					r->rd_fd, redir_ਸਾਰਣੀ[r->rd_type].descrip);
			/* guard against the case >$FOO, where foo is unset or blank */
			ਜੇ (r->rd_filename) {
				debug_printf_clean(" fname:'%s'\n", r->rd_filename);
				ਮੁਕਤ(r->rd_filename);
				//r->rd_filename = NULL;
			}
			debug_printf_clean(" rd_dup:%d\n", r->rd_dup);
			rnext = r->ਅਗਲਾ;
			ਮੁਕਤ(r);
		}
		//command->redirects = NULL;
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */
	}
	ਮੁਕਤ(pi->cmds);   /* children are an array, they get freed all at once */
	//pi->cmds = NULL;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
#ਜੇ ENABLE_HUSH_ਕੰਮ
	ਮੁਕਤ(pi->cmdtext);
	//pi->cmdtext = NULL;
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */

	ਅਗਲਾ = pi->ਅਗਲਾ;
	ਮੁਕਤ(pi);
	ਵਾਪਸ ਅਗਲਾ;
}

ਸਥਿਰ ਖਾਲੀ free_pipe_list(ਬਣਤਰ pipe *pi)
{
	ਜਦੋਂ (pi) {
#ਜੇ HAS_KEYWORDS
		debug_printf_clean("pipe reserved word %d\n", pi->res_ਸ਼ਬਦ);
#ਜੇ_ਅੰਤ
		debug_printf_clean("pipe followup code %d\n", pi->followup);
		pi = free_pipe(pi);
	}
}

/*** Parsing routines ***/

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ debug_print_ਰੁੱਖ
ਸਥਿਰ ਖਾਲੀ debug_print_ਰੁੱਖ(ਬਣਤਰ pipe *pi, ਪੂਰਨ lvl)
{
	ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ PIPE[] ALIGN_PTR = {
		[PIPE_SEQ] = "SEQ",
		[PIPE_AND] = "AND",
		[PIPE_OR ] = "OR" ,
		[PIPE_BG ] = "BG" ,
	};
	ਸਥਿਰ ਅਚਲ ਅੱਖਰ *RES[] = {
		[RES_NONE ] = "NONE" ,
# ਜੇ ENABLE_HUSH_IF
		[RES_IF   ] = "IF"   ,
		[RES_THEN ] = "THEN" ,
		[RES_ELIF ] = "ELIF" ,
		[RES_ELSE ] = "ELSE" ,
		[RES_FI   ] = "FI"   ,
# endif
# ਜੇ ENABLE_HUSH_LOOPS
		[RES_FOR  ] = "FOR"  ,
		[RES_WHILE] = "WHILE",
		[RES_UNTIL] = "UNTIL",
		[RES_DO   ] = "DO"   ,
		[RES_ਪੂਰਾ ] = "DONE" ,
# endif
# ਜੇ ENABLE_HUSH_LOOPS || ENABLE_HUSH_CASE
		[RES_ਵਿੱਚ   ] = "IN"   ,
# endif
# ਜੇ ENABLE_HUSH_CASE
		[RES_CASE ] = "CASE" ,
		[RES_CASE_ਵਿੱਚ ] = "CASE_IN" ,
		[RES_ਮੇਲ] = "MATCH",
		[RES_CASE_BODY] = "CASE_BODY",
		[RES_ESAC ] = "ESAC" ,
# endif
		[RES_XXXX ] = "XXXX" ,
		[RES_SNTX ] = "SNTX" ,
	};
	ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ CMDTYPE[] ALIGN_PTR = {
		"{}",
		"()",
		"[noglob]",
# ਜੇ ENABLE_HUSH_FUNCTIONS
		"func()",
# endif
	};

	ਪੂਰਨ pin, prn;

	pin = 0;
	ਜਦੋਂ (pi) {
		fdprintf(2, "%*spipe %d #cmds:%d %sres_word=%s followup=%d %s\n",
				lvl*2, "",
				pin,
				pi->ਅੰਕ_cmds,
				(IF_HAS_KEYWORDS(pi->pi_inverted ? "! " :) ""),
				RES[pi->res_ਸ਼ਬਦ],
				pi->followup, PIPE[pi->followup]
		);
		prn = 0;
		ਜਦੋਂ (prn < pi->ਅੰਕ_cmds) {
			ਬਣਤਰ command *command = &pi->cmds[prn];
			ਅੱਖਰ **argv = command->argv;

			fdprintf(2, "%*s cmd %d assignment_cnt:%d",
					lvl*2, "", prn,
					command->assignment_cnt);
# ਜੇ ENABLE_HUSH_LINENO_VAR
			fdprintf(2, " LINENO:%u", command->lineno);
# endif
			ਜੇ (command->ਸਮੂਹ) {
				fdprintf(2, " group %s: (argv=%p)%s%s\n",
						CMDTYPE[command->cmd_type],
						argv
# ਜੇ !BB_MMU
						, " group_as_string:", command->ਸਮੂਹ_as_string
# ਨਹੀਂਤਾਂ
						, "", ""
# endif
				);
				debug_print_ਰੁੱਖ(command->ਸਮੂਹ, lvl+1);
				prn++;
				ਜਾਰੀ;
			}
			ਜੇ (argv) ਜਦੋਂ (*argv) {
				fdprintf(2, " '%s'", *argv);
				argv++;
			}
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
			ਜੇ (command->redirects)
				fdprintf(2, " {redir}");
#ਜੇ_ਅੰਤ /* __U_BOOT__ */
			fdprintf(2, "\n");
			prn++;
		}
		pi = pi->ਅਗਲਾ;
		pin++;
	}
}
#ਜੇ_ਅੰਤ /* debug_print_tree */

ਸਥਿਰ ਬਣਤਰ pipe *new_pipe(ਖਾਲੀ)
{
	ਬਣਤਰ pipe *pi;
	pi = xzalloc(ਆਕਾਰ(ਬਣਤਰ pipe));
	/*pi->res_word = RES_NONE; - RES_NONE is 0 anyway */
	ਵਾਪਸ pi;
}

/* Command (member of a pipe) is complete, or we start a new pipe
 * if ctx->command is NULL.
 * No errors possible here.
 */
ਸਥਿਰ ਪੂਰਨ ਪੂਰਾ_command(ਬਣਤਰ parse_context *ctx)
{
	/* The command is really already in the pipe structure, so
	 * advance the pipe counter and make a new, null command. */
	ਬਣਤਰ pipe *pi = ctx->pipe;
	ਬਣਤਰ command *command = ctx->command;

#ਜੇ 0	/* Instead we emit error message at run time */
	ਜੇ (ctx->pending_redirect) {
		/* For example, "cmd >" (no filename to redirect to) */
		syntax_error("invalid redirect");
		ctx->pending_redirect = ਨਿਰਾ;
	}
#ਜੇ_ਅੰਤ

	ਜੇ (command) {
		ਜੇ (IS_NULL_CMD(command)) {
			debug_printf_parse("done_command: skipping null cmd, num_cmds=%d\n", pi->ਅੰਕ_cmds);
			ਜਾਓ clear_and_ret;
		}
		pi->ਅੰਕ_cmds++;
		debug_printf_parse("done_command: ++num_cmds=%d\n", pi->ਅੰਕ_cmds);
		//debug_print_tree(ctx->list_head, 20);
	} ਨਹੀਂਤਾਂ {
		debug_printf_parse("done_command: initializing, num_cmds=%d\n", pi->ਅੰਕ_cmds);
	}

	/* Only real trickiness here is that the uncommitted
	 * command structure is not counted in pi->num_cmds. */
	pi->cmds = xrealloc(pi->cmds, ਆਕਾਰ(*pi->cmds) * (pi->ਅੰਕ_cmds+1));
	ctx->command = command = &pi->cmds[pi->ਅੰਕ_cmds];
 clear_and_ret:
	memset(command, 0, ਆਕਾਰ(*command));
#ਜੇ ENABLE_HUSH_LINENO_VAR
	command->lineno = G.parse_lineno;
	debug_printf_parse("command->lineno = G.parse_lineno (%u)\n", G.parse_lineno);
#ਜੇ_ਅੰਤ
	ਵਾਪਸ pi->ਅੰਕ_cmds; /* used only for 0/nonzero check */
}

ਸਥਿਰ ਖਾਲੀ ਪੂਰਾ_pipe(ਬਣਤਰ parse_context *ctx, pipe_style type)
{
	ਪੂਰਨ not_null;

	debug_printf_parse("done_pipe entered, followup %d\n", type);
	/* Close previous command */
	not_null = ਪੂਰਾ_command(ctx);
#ਜੇ HAS_KEYWORDS
	ctx->pipe->pi_inverted = ctx->ctx_inverted;
	ctx->ctx_inverted = 0;
	ctx->pipe->res_ਸ਼ਬਦ = ctx->ctx_res_w;
#ਜੇ_ਅੰਤ
	ਜੇ (type == PIPE_BG && ctx->list_ਸਿਰ != ctx->pipe) {
		/* Necessary since && and || have precedence over &:
		 * "cmd1 && cmd2 &" must spawn both cmds, not only cmd2,
		 * in a backgrounded subshell.
		 */
		ਬਣਤਰ pipe *pi;
		ਬਣਤਰ command *command;

		/* Is this actually this construct, all pipes end with && or ||? */
		pi = ctx->list_ਸਿਰ;
		ਜਦੋਂ (pi != ctx->pipe) {
			ਜੇ (pi->followup != PIPE_AND && pi->followup != PIPE_OR)
				ਜਾਓ no_conv;
			pi = pi->ਅਗਲਾ;
		}

		debug_printf_parse("BG with more than one pipe, converting to { p1 &&...pN; } &\n");
		pi->followup = PIPE_SEQ; /* close pN _not_ with "&"! */
		pi = xzalloc(ਆਕਾਰ(*pi));
		pi->followup = PIPE_BG;
		pi->ਅੰਕ_cmds = 1;
		pi->cmds = xzalloc(ਆਕਾਰ(pi->cmds[0]));
		command = &pi->cmds[0];
		ਜੇ (CMD_ਸਾਧਾਰਨ != 0) /* "if xzalloc didn't do that already" */
			command->cmd_type = CMD_ਸਾਧਾਰਨ;
		command->ਸਮੂਹ = ctx->list_ਸਿਰ;
#ਜੇ !BB_MMU
		command->ਸਮੂਹ_as_string = xstrndup(
			    ctx->as_string.ਅੰਕੜੇ,
			    ctx->as_string.ਲੰਮਾਈ - 1 /* do not copy last char, "&" */
		);
#ਜੇ_ਅੰਤ
		/* Replace all pipes in ctx with one newly created */
		ctx->list_ਸਿਰ = ctx->pipe = pi;
		/* for cases like "cmd && &", do not be tricked by last command
		 * being null - the entire {...} & is NOT null! */
		not_null = 1;
	} ਨਹੀਂਤਾਂ {
 no_conv:
		ctx->pipe->followup = type;
	}

	/* Without this check, even just <enter> on command line generates
	 * tree of three NOPs (!). Which is harmless but annoying.
	 * IOW: it is safe to do it unconditionally. */
	ਜੇ (not_null
#ਜੇ ENABLE_HUSH_IF
	 || ctx->ctx_res_w == RES_FI
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_HUSH_LOOPS
	 || ctx->ctx_res_w == RES_ਪੂਰਾ
	 || ctx->ctx_res_w == RES_FOR
	 || ctx->ctx_res_w == RES_ਵਿੱਚ
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_HUSH_CASE
	 || ctx->ctx_res_w == RES_ESAC
#ਜੇ_ਅੰਤ
	) {
		ਬਣਤਰ pipe *new_p;
		debug_printf_parse("done_pipe: adding new pipe: "
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
				"not_null:%d ctx->ctx_res_w:%d\n",
				not_null, ctx->ctx_res_w);
#ਨਹੀਂਤਾਂ /* __U_BOOT__ */
				"not_null:%d\n",
				not_null);
#ਜੇ_ਅੰਤ /* __U_BOOT__ */
		new_p = new_pipe();
		ctx->pipe->ਅਗਲਾ = new_p;
		ctx->pipe = new_p;
		/* RES_THEN, RES_DO etc are "sticky" -
		 * they remain set for pipes inside if/while.
		 * This is used to control execution.
		 * RES_FOR and RES_IN are NOT sticky (needed to support
		 * cases where variable or value happens to match a keyword):
		 */
#ਜੇ ENABLE_HUSH_LOOPS
		ਜੇ (ctx->ctx_res_w == RES_FOR
		 || ctx->ctx_res_w == RES_ਵਿੱਚ)
			ctx->ctx_res_w = RES_NONE;
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_HUSH_CASE
		ਜੇ (ctx->ctx_res_w == RES_ਮੇਲ)
			ctx->ctx_res_w = RES_CASE_BODY;
		ਜੇ (ctx->ctx_res_w == RES_CASE)
			ctx->ctx_res_w = RES_CASE_ਵਿੱਚ;
#ਜੇ_ਅੰਤ
		ctx->command = ਨਿਰਾ; /* trick done_command below */
		/* Create the memory for command, roughly:
		 * ctx->pipe->cmds = new struct command;
		 * ctx->command = &ctx->pipe->cmds[0];
		 */
		ਪੂਰਾ_command(ctx);
		//debug_print_tree(ctx->list_head, 10);
	}
	debug_printf_parse("done_pipe return\n");
}

ਸਥਿਰ ਖਾਲੀ initialize_context(ਬਣਤਰ parse_context *ctx)
{
	memset(ctx, 0, ਆਕਾਰ(*ctx));
	ਜੇ (MAYBE_ASSIGNMENT != 0)
		ctx->is_assignment = MAYBE_ASSIGNMENT;
	ctx->pipe = ctx->list_ਸਿਰ = new_pipe();
	/* Create the memory for command, roughly:
	 * ctx->pipe->cmds = new struct command;
	 * ctx->command = &ctx->pipe->cmds[0];
	 */
	ਪੂਰਾ_command(ctx);
}

/* If a reserved word is found and processed, parse context is modified
 * and 1 is returned.
 */
#ਜੇ HAS_KEYWORDS
ਬਣਤਰ reserved_combo {
	ਅੱਖਰ literal[6];
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ res;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ assignment_flag;
	uint32_t flag;
};
ਗਿਣਤੀ {
	FLAG_ਸਿਰਾ   = (1 << RES_NONE ),
# ਜੇ ENABLE_HUSH_IF
	FLAG_IF    = (1 << RES_IF   ),
	FLAG_THEN  = (1 << RES_THEN ),
	FLAG_ELIF  = (1 << RES_ELIF ),
	FLAG_ELSE  = (1 << RES_ELSE ),
	FLAG_FI    = (1 << RES_FI   ),
# endif
# ਜੇ ENABLE_HUSH_LOOPS
	FLAG_FOR   = (1 << RES_FOR  ),
	FLAG_WHILE = (1 << RES_WHILE),
	FLAG_UNTIL = (1 << RES_UNTIL),
	FLAG_DO    = (1 << RES_DO   ),
	FLAG_ਪੂਰਾ  = (1 << RES_ਪੂਰਾ ),
	FLAG_ਵਿੱਚ    = (1 << RES_ਵਿੱਚ   ),
# endif
# ਜੇ ENABLE_HUSH_CASE
	FLAG_ਮੇਲ = (1 << RES_ਮੇਲ),
	FLAG_ESAC  = (1 << RES_ESAC ),
# endif
	FLAG_ਸ਼ੁਰੂ = (1 << RES_XXXX ),
};

ਸਥਿਰ ਅਚਲ ਬਣਤਰ reserved_combo* ਮੇਲ_reserved_ਸ਼ਬਦ(o_string *ਸ਼ਬਦ)
{
	/* Mostly a list of accepted follow-up reserved words.
	 * FLAG_END means we are done with the sequence, and are ready
	 * to turn the compound list into a command.
	 * FLAG_START means the word must start a new compound list.
	 */
	ਸਥਿਰ ਅਚਲ ਬਣਤਰ reserved_combo reserved_list[] ALIGN4 = {
# ਜੇ ENABLE_HUSH_IF
		{ "!",     RES_NONE,  NOT_ASSIGNMENT  , 0 },
		{ "if",    RES_IF,    MAYBE_ASSIGNMENT, FLAG_THEN | FLAG_ਸ਼ੁਰੂ },
		{ "then",  RES_THEN,  MAYBE_ASSIGNMENT, FLAG_ELIF | FLAG_ELSE | FLAG_FI },
		{ "elif",  RES_ELIF,  MAYBE_ASSIGNMENT, FLAG_THEN },
		{ "else",  RES_ELSE,  MAYBE_ASSIGNMENT, FLAG_FI   },
		{ "fi",    RES_FI,    NOT_ASSIGNMENT  , FLAG_ਸਿਰਾ  },
# endif
# ਜੇ ENABLE_HUSH_LOOPS
		{ "for",   RES_FOR,   NOT_ASSIGNMENT  , FLAG_ਵਿੱਚ | FLAG_DO | FLAG_ਸ਼ੁਰੂ },
		{ "while", RES_WHILE, MAYBE_ASSIGNMENT, FLAG_DO | FLAG_ਸ਼ੁਰੂ },
		{ "until", RES_UNTIL, MAYBE_ASSIGNMENT, FLAG_DO | FLAG_ਸ਼ੁਰੂ },
		{ "in",    RES_ਵਿੱਚ,    NOT_ASSIGNMENT  , FLAG_DO   },
		{ "do",    RES_DO,    MAYBE_ASSIGNMENT, FLAG_ਪੂਰਾ },
		{ "done",  RES_ਪੂਰਾ,  NOT_ASSIGNMENT  , FLAG_ਸਿਰਾ  },
# endif
# ਜੇ ENABLE_HUSH_CASE
		{ "case",  RES_CASE,  NOT_ASSIGNMENT  , FLAG_ਮੇਲ | FLAG_ਸ਼ੁਰੂ },
		{ "esac",  RES_ESAC,  NOT_ASSIGNMENT  , FLAG_ਸਿਰਾ  },
# endif
	};
	ਅਚਲ ਬਣਤਰ reserved_combo *r;

	ਲਈ (r = reserved_list; r < reserved_list + ARRAY_ਮਾਪ(reserved_list); r++) {
		ਜੇ (strcmp(ਸ਼ਬਦ->ਅੰਕੜੇ, r->literal) == 0)
			ਵਾਪਸ r;
	}
	ਵਾਪਸ ਨਿਰਾ;
}
/* Return NULL: not a keyword, else: keyword
 */
ਸਥਿਰ ਅਚਲ ਬਣਤਰ reserved_combo* reserved_ਸ਼ਬਦ(ਬਣਤਰ parse_context *ctx)
{
# ਜੇ ENABLE_HUSH_CASE
	ਸਥਿਰ ਅਚਲ ਬਣਤਰ reserved_combo reserved_ਮੇਲ = {
		"",        RES_ਮੇਲ, NOT_ASSIGNMENT , FLAG_ਮੇਲ | FLAG_ESAC
	};
# endif
	ਅਚਲ ਬਣਤਰ reserved_combo *r;

	ਜੇ (ctx->ਸ਼ਬਦ.has_quoted_part)
		ਵਾਪਸ 0;
	r = ਮੇਲ_reserved_ਸ਼ਬਦ(&ctx->ਸ਼ਬਦ);
	ਜੇ (!r)
		ਵਾਪਸ r; /* NULL */

	debug_printf("found reserved word %s, res %d\n", r->literal, r->res);
# ਜੇ ENABLE_HUSH_CASE
	ਜੇ (r->res == RES_ਵਿੱਚ && ctx->ctx_res_w == RES_CASE_ਵਿੱਚ) {
		/* "case word IN ..." - IN part starts first MATCH part */
		r = &reserved_ਮੇਲ;
	} ਨਹੀਂਤਾਂ
# endif
	ਜੇ (r->flag == 0) { /* '!' */
		ਜੇ (ctx->ctx_inverted) { /* bash doesn't accept '! ! true' */
			syntax_error("! ! command");
			ctx->ctx_res_w = RES_SNTX;
		}
		ctx->ctx_inverted = 1;
		ਵਾਪਸ r;
	}
	ਜੇ (r->flag & FLAG_ਸ਼ੁਰੂ) {
		ਬਣਤਰ parse_context *old;

		old = xmemdup(ctx, ਆਕਾਰ(*ctx));
		debug_printf_parse("push stack %p\n", old);
		initialize_context(ctx);
		ctx->ਢੇਰ = old;
	} ਨਹੀਂਤਾਂ ਜੇ (/*ctx->ctx_res_w == RES_NONE ||*/ !(ctx->old_flag & (1 << r->res))) {
		syntax_error_at(ctx->ਸ਼ਬਦ.ਅੰਕੜੇ);
		ctx->ctx_res_w = RES_SNTX;
		ਵਾਪਸ r;
	} ਨਹੀਂਤਾਂ {
		/* "{...} fi" is ok. "{...} if" is not
		 * Example:
		 * if { echo foo; } then { echo bar; } fi */
		ਜੇ (ctx->command->ਸਮੂਹ)
			ਪੂਰਾ_pipe(ctx, PIPE_SEQ);
	}

	ctx->ctx_res_w = r->res;
	ctx->old_flag = r->flag;
	ctx->is_assignment = r->assignment_flag;
	debug_printf_parse("ctx->is_assignment='%s'\n", assignment_flag[ctx->is_assignment]);

	ਜੇ (ctx->old_flag & FLAG_ਸਿਰਾ) {
		ਬਣਤਰ parse_context *old;

		ਪੂਰਾ_pipe(ctx, PIPE_SEQ);
		debug_printf_parse("pop stack %p\n", ctx->ਢੇਰ);
		old = ctx->ਢੇਰ;
		old->command->ਸਮੂਹ = ctx->list_ਸਿਰ;
		old->command->cmd_type = CMD_ਸਾਧਾਰਨ;
# ਜੇ !BB_MMU
		/* At this point, the compound command's string is in
		 * ctx->as_string... except for the leading keyword!
		 * Consider this example: "echo a | if true; then echo a; fi"
		 * ctx->as_string will contain "true; then echo a; fi",
		 * with "if " remaining in old->as_string!
		 */
		{
			ਅੱਖਰ *str;
			ਪੂਰਨ len = old->as_string.ਲੰਮਾਈ;
			/* Concatenate halves */
			o_addstr(&old->as_string, ctx->as_string.ਅੰਕੜੇ);
			o_free(&ctx->as_string);
			/* Find where leading keyword starts in first half */
			str = old->as_string.ਅੰਕੜੇ + len;
			ਜੇ (str > old->as_string.ਅੰਕੜੇ)
				str--; /* skip whitespace after keyword */
			ਜਦੋਂ (str > old->as_string.ਅੰਕੜੇ && ਅੱਖਰ_ਹੈ(str[-1]))
				str--;
			/* Ugh, we're done with this horrid hack */
			old->command->ਸਮੂਹ_as_string = xstrdup(str);
			debug_printf_parse("pop, remembering as:'%s'\n",
					old->command->ਸਮੂਹ_as_string);
		}
# endif
		*ctx = *old;   /* physical copy */
		ਮੁਕਤ(old);
	}
	ਵਾਪਸ r;
}
#ਜੇ_ਅੰਤ /* HAS_KEYWORDS */

/* Word is complete, look at it and update parsing context.
 * Normal return is 0. Syntax errors return 1.
 * Note: on return, word is reset, but not o_free'd!
 */
ਸਥਿਰ ਪੂਰਨ ਪੂਰਾ_ਸ਼ਬਦ(ਬਣਤਰ parse_context *ctx)
{
	ਬਣਤਰ command *command = ctx->command;

	debug_printf_parse("done_word entered: '%s' %p\n", ctx->ਸ਼ਬਦ.ਅੰਕੜੇ, command);
	ਜੇ (ctx->ਸ਼ਬਦ.ਲੰਮਾਈ == 0 && !ctx->ਸ਼ਬਦ.has_quoted_part) {
		debug_printf_parse("done_word return 0: true null, ignored\n");
		ਵਾਪਸ 0;
	}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	ਜੇ (ctx->pending_redirect) {
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

		ctx->pending_redirect->rd_filename = xstrdup(ctx->ਸ਼ਬਦ.ਅੰਕੜੇ);
		/* Cater for >\file case:
		 * >\a creates file a; >\\a, >"\a", >"\\a" create file \a
		 * Same with heredocs:
		 * for <<\H delim is H; <<\\H, <<"\H", <<"\\H" - \H
		 */
		ਜੇ (ctx->pending_redirect->rd_type == REDIRECT_HEREDOC) {
			unbackslash(ctx->pending_redirect->rd_filename);
			/* Is it <<"HEREDOC"? */
			ਜੇ (ctx->ਸ਼ਬਦ.has_quoted_part) {
				ctx->pending_redirect->rd_dup |= HEREDOC_QUOTED;
			}
		}
		debug_printf_parse("word stored in rd_filename: '%s'\n", ctx->ਸ਼ਬਦ.ਅੰਕੜੇ);
		ctx->pending_redirect = ਨਿਰਾ;
	} ਨਹੀਂਤਾਂ {
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */
#ਜੇ HAS_KEYWORDS
# ਜੇ ENABLE_HUSH_CASE
		ਜੇ (ctx->ctx_dsemicolon
		 && strcmp(ctx->ਸ਼ਬਦ.ਅੰਕੜੇ, "esac") != 0 /* not "... pattern) cmd;; esac" */
		) {
			/* already done when ctx_dsemicolon was set to 1: */
			/* ctx->ctx_res_w = RES_MATCH; */
			ctx->ctx_dsemicolon = 0;
		} ਨਹੀਂਤਾਂ
# endif
# ਜੇ defined(CMD_TEST2_SINGLEWORD_NOGLOB)
		ਜੇ (command->cmd_type == CMD_TEST2_SINGLEWORD_NOGLOB
		 && strcmp(ctx->ਸ਼ਬਦ.ਅੰਕੜੇ, "]]") == 0
		) {
			/* allow "[[ ]] >file" etc */
			command->cmd_type = CMD_SINGLEWORD_NOGLOB;
		} ਨਹੀਂਤਾਂ
# endif
		ਜੇ (!command->argv /* if it's the first word... */
# ਜੇ ENABLE_HUSH_LOOPS
		 && ctx->ctx_res_w != RES_FOR /* ...not after FOR or IN */
		 && ctx->ctx_res_w != RES_ਵਿੱਚ
# endif
# ਜੇ ENABLE_HUSH_CASE
		 && ctx->ctx_res_w != RES_CASE
# endif
		) {
			ਅਚਲ ਬਣਤਰ reserved_combo *reserved;
			reserved = reserved_ਸ਼ਬਦ(ctx);
			debug_printf_parse("checking for reserved-ness: %d\n", !!reserved);
			ਜੇ (reserved) {
# ਜੇ ENABLE_HUSH_LINENO_VAR
/* Case:
 * "while ...; do
 *	cmd ..."
 * If we don't close the pipe _now_, immediately after "do", lineno logic
 * sees "cmd" as starting at "do" - i.e., at the previous line.
 */
				ਜੇ (0
				 IF_HUSH_IF(|| reserved->res == RES_THEN)
				 IF_HUSH_IF(|| reserved->res == RES_ELIF)
				 IF_HUSH_IF(|| reserved->res == RES_ELSE)
				 IF_HUSH_LOOPS(|| reserved->res == RES_DO)
				) {
					ਪੂਰਾ_pipe(ctx, PIPE_SEQ);
				}
# endif
				o_reset_ਮੰਜ਼ਿਲ_ਖਾਲੀ_ਥਾਵਾਂ_unquoted(&ctx->ਸ਼ਬਦ);
				debug_printf_parse("done_word return %d\n",
						(ctx->ctx_res_w == RES_SNTX));
				ਵਾਪਸ (ctx->ctx_res_w == RES_SNTX);
			}
# ਜੇ defined(CMD_TEST2_SINGLEWORD_NOGLOB)
			ਜੇ (strcmp(ctx->ਸ਼ਬਦ.ਅੰਕੜੇ, "[[") == 0) {
				command->cmd_type = CMD_TEST2_SINGLEWORD_NOGLOB;
			} ਨਹੀਂਤਾਂ
# endif
# ਜੇ defined(CMD_SINGLEWORD_NOGLOB)
			ਜੇ (0
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
			 IF_HUSH_LOCAL(   || strcmp(ctx->ਸ਼ਬਦ.ਅੰਕੜੇ, "local") == 0)
			 IF_HUSH_EXPORT(  || strcmp(ctx->ਸ਼ਬਦ.ਅੰਕੜੇ, "export") == 0)
			 IF_HUSH_READONLY(|| strcmp(ctx->ਸ਼ਬਦ.ਅੰਕੜੇ, "readonly") == 0)
			) {
				command->cmd_type = CMD_SINGLEWORD_NOGLOB;
			}
# ਨਹੀਂਤਾਂ
			{ /* empty block to pair "if ... else" */ }
# endif
		}
#ਜੇ_ਅੰਤ /* HAS_KEYWORDS */

		ਜੇ (command->ਸਮੂਹ) {
			/* "{ echo foo; } echo bar" - bad */
			syntax_error_at(ctx->ਸ਼ਬਦ.ਅੰਕੜੇ);
			debug_printf_parse("done_word return 1: syntax error, "
					"groups and arglists don't mix\n");
			ਵਾਪਸ 1;
		}

		/* If this word wasn't an assignment, next ones definitely
		 * can't be assignments. Even if they look like ones. */
		ਜੇ (ctx->is_assignment != DEFINITELY_ASSIGNMENT
		 && ctx->is_assignment != ਸ਼ਬਦ_IS_KEYWORD
		) {
			ctx->is_assignment = NOT_ASSIGNMENT;
		} ਨਹੀਂਤਾਂ {
			ਜੇ (ctx->is_assignment == DEFINITELY_ASSIGNMENT) {
				command->assignment_cnt++;
				debug_printf_parse("++assignment_cnt=%d\n", command->assignment_cnt);
			}
			debug_printf_parse("ctx->is_assignment was:'%s'\n", assignment_flag[ctx->is_assignment]);
			ctx->is_assignment = MAYBE_ASSIGNMENT;
		}
		debug_printf_parse("ctx->is_assignment='%s'\n", assignment_flag[ctx->is_assignment]);
		command->argv = ਜੋੜੋ_string_ਮੰਜ਼ਿਲ_ਲੜੀਆਂ(command->argv, xstrdup(ctx->ਸ਼ਬਦ.ਅੰਕੜੇ));
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
		command->argc++;
#ਜੇ_ਅੰਤ /* __U_BOOT__ */
		debug_print_ਲੜੀਆਂ("word appended to argv", command->argv);

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	}
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */

#ਜੇ ENABLE_HUSH_LOOPS
	ਜੇ (ctx->ctx_res_w == RES_FOR) {
		ਜੇ (ctx->ਸ਼ਬਦ.has_quoted_part
		 || endofname(command->argv[0])[0] != '\0'
		) {
			/* bash says just "not a valid identifier" */
			syntax_error("bad for loop variable");
			ਵਾਪਸ 1;
		}
		/* Force FOR to have just one word (variable name) */
		/* NB: basically, this makes hush see "for v in ..."
		 * syntax as if it is "for v; in ...". FOR and IN become
		 * two pipe structs in parse tree. */
		ਪੂਰਾ_pipe(ctx, PIPE_SEQ);
	}
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_HUSH_CASE
	/* Force CASE to have just one word */
	ਜੇ (ctx->ctx_res_w == RES_CASE) {
		ਪੂਰਾ_pipe(ctx, PIPE_SEQ);
	}
#ਜੇ_ਅੰਤ

	o_reset_ਮੰਜ਼ਿਲ_ਖਾਲੀ_ਥਾਵਾਂ_unquoted(&ctx->ਸ਼ਬਦ);

	debug_printf_parse("done_word return 0\n");
	ਵਾਪਸ 0;
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
/* Peek ahead in the input to find out if we have a "&n" construct,
 * as in "2>&1", that represents duplicating a file descriptor.
 * Return:
 * REDIRFD_CLOSE if >&- "close fd" construct is seen,
 * REDIRFD_SYNTAX_ERR if syntax error,
 * REDIRFD_TO_FILE if no & was seen,
 * or the number found.
 */
#ਜੇ BB_MMU
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ parse_redir_ਸੱਜਾ_fd(as_string, ਨਿਵੇਸ਼) \
	parse_redir_ਸੱਜਾ_fd(ਨਿਵੇਸ਼)
#ਜੇ_ਅੰਤ
ਸਥਿਰ ਪੂਰਨ parse_redir_ਸੱਜਾ_fd(o_string *as_string, ਬਣਤਰ ਵਿੱਚ_str *ਨਿਵੇਸ਼)
{
	ਪੂਰਨ ch, d, ok;

	ch = i_peek(ਨਿਵੇਸ਼);
	ਜੇ (ch != '&')
		ਵਾਪਸ REDIRFD_ਮੰਜ਼ਿਲ_FILE;

	ch = i_getch(ਨਿਵੇਸ਼);  /* get the & */
	nommu_addchr(as_string, ch);
	ch = i_peek(ਨਿਵੇਸ਼);
	ਜੇ (ch == '-') {
		ch = i_getch(ਨਿਵੇਸ਼);
		nommu_addchr(as_string, ch);
		ਵਾਪਸ REDIRFD_CLOSE;
	}
	d = 0;
	ok = 0;
	ਜਦੋਂ (ch != EOF && ਅੰਕ_ਹੈ(ch)) {
		d = d*10 + (ch-'0');
		ok = 1;
		ch = i_getch(ਨਿਵੇਸ਼);
		nommu_addchr(as_string, ch);
		ch = i_peek(ਨਿਵੇਸ਼);
	}
	ਜੇ (ok) ਵਾਪਸ d;

//TODO: this is the place to catch ">&file" bashism (redirect both fd 1 and 2)

	bb_simple_error_ਸੁਨੇਹਾ("ambiguous redirect");
	ਵਾਪਸ REDIRFD_SYNTAX_ERR;
}

/* Return code is 0 normal, 1 if a syntax error is detected
 */
ਸਥਿਰ ਪੂਰਨ parse_redirect(ਬਣਤਰ parse_context *ctx,
		ਪੂਰਨ fd,
		redir_type style,
		ਬਣਤਰ ਵਿੱਚ_str *ਨਿਵੇਸ਼)
{
	ਬਣਤਰ command *command = ctx->command;
	ਬਣਤਰ redir_struct *redir;
	ਬਣਤਰ redir_struct **redirp;
	ਪੂਰਨ dup_ਅੰਕ;

	dup_ਅੰਕ = REDIRFD_ਮੰਜ਼ਿਲ_FILE;
	ਜੇ (style != REDIRECT_HEREDOC) {
		/* Check for a '>&1' type redirect */
		dup_ਅੰਕ = parse_redir_ਸੱਜਾ_fd(&ctx->as_string, ਨਿਵੇਸ਼);
		ਜੇ (dup_ਅੰਕ == REDIRFD_SYNTAX_ERR)
			ਵਾਪਸ 1;
	} ਨਹੀਂਤਾਂ {
		ਪੂਰਨ ch = i_peek_and_eat_bkslash_nl(ਨਿਵੇਸ਼);
		dup_ਅੰਕ = (ch == '-'); /* HEREDOC_SKIPTABS bit is 1 */
		ਜੇ (dup_ਅੰਕ) { /* <<-... */
			ch = i_getch(ਨਿਵੇਸ਼);
			nommu_addchr(&ctx->as_string, ch);
			ch = i_peek(ਨਿਵੇਸ਼);
		}
	}

	ਜੇ (style == REDIRECT_OVERWRITE && dup_ਅੰਕ == REDIRFD_ਮੰਜ਼ਿਲ_FILE) {
		ਪੂਰਨ ch = i_peek_and_eat_bkslash_nl(ਨਿਵੇਸ਼);
		ਜੇ (ch == '|') {
			/* >|FILE redirect ("clobbering" >).
			 * Since we do not support "set -o noclobber" yet,
			 * >| and > are the same for now. Just eat |.
			 */
			ch = i_getch(ਨਿਵੇਸ਼);
			nommu_addchr(&ctx->as_string, ch);
		}
	}

	/* Create a new redir_struct and append it to the linked list */
	redirp = &command->redirects;
	ਜਦੋਂ ((redir = *redirp) != ਨਿਰਾ) {
		redirp = &(redir->ਅਗਲਾ);
	}
	*redirp = redir = xzalloc(ਆਕਾਰ(*redir));
	/* redir->next = NULL; */
	/* redir->rd_filename = NULL; */
	redir->rd_type = style;
	redir->rd_fd = (fd == -1) ? redir_ਸਾਰਣੀ[style].default_fd : fd;

	debug_printf_parse("redirect type %d %s\n", redir->rd_fd,
				redir_ਸਾਰਣੀ[style].descrip);

	redir->rd_dup = dup_ਅੰਕ;
	ਜੇ (style != REDIRECT_HEREDOC && dup_ਅੰਕ != REDIRFD_ਮੰਜ਼ਿਲ_FILE) {
		/* Erik had a check here that the file descriptor in question
		 * is legit; I postpone that to "run time"
		 * A "-" representation of "close me" shows up as a -3 here */
		debug_printf_parse("duplicating redirect '%d>&%d'\n",
				redir->rd_fd, redir->rd_dup);
	} ਨਹੀਂਤਾਂ {
#ਜੇ 0		/* Instead we emit error message at run time */
		ਜੇ (ctx->pending_redirect) {
			/* For example, "cmd > <file" */
			syntax_error("invalid redirect");
		}
#ਜੇ_ਅੰਤ
		/* Set ctx->pending_redirect, so we know what to do at the
		 * end of the next parsed word. */
		ctx->pending_redirect = redir;
	}
	ਵਾਪਸ 0;
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
ਸਥਿਰ ਪੂਰਨ redirect_opt_ਅੰਕ(o_string *o)
{
	ਪੂਰਨ ਅੰਕ;

	ਜੇ (o->ਅੰਕੜੇ == ਨਿਰਾ)
		ਵਾਪਸ -1;
	ਅੰਕ = bb_strtou(o->ਅੰਕੜੇ, ਨਿਰਾ, 10);
	ਜੇ (errno || ਅੰਕ < 0)
		ਵਾਪਸ -1;
	o_reset_ਮੰਜ਼ਿਲ_ਖਾਲੀ_ਥਾਵਾਂ_unquoted(o);
	ਵਾਪਸ ਅੰਕ;
}

#ਜੇ BB_MMU
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ fetch_till_str(as_string, ਨਿਵੇਸ਼, ਸ਼ਬਦ, skip_tabs) \
	fetch_till_str(ਨਿਵੇਸ਼, ਸ਼ਬਦ, skip_tabs)
#ਜੇ_ਅੰਤ
ਸਥਿਰ ਅੱਖਰ *fetch_till_str(o_string *as_string,
		ਬਣਤਰ ਵਿੱਚ_str *ਨਿਵੇਸ਼,
		ਅਚਲ ਅੱਖਰ *ਸ਼ਬਦ,
		ਪੂਰਨ heredoc_flags)
{
	o_string heredoc = NULL_O_STRING;
	ਬਿਨਾਨਿਸ਼ਾਨੀ past_EOL;
	ਪੂਰਨ prev = 0; /* not \ */
	ਪੂਰਨ ch;

	/* Starting with "" is necessary for this case:
	 * cat <<EOF
	 *
	 * xxx
	 * EOF
	 */
	heredoc.ਅੰਕੜੇ = xzalloc(1); /* start as "", not as NULL */

	ਜਾਓ jump_ਵਿੱਚ;

	ਜਦੋਂ (1) {
		ch = i_getch(ਨਿਵੇਸ਼);
		ਜੇ (ch != EOF)
			nommu_addchr(as_string, ch);
		ਜੇ (ch == '\n' || ch == EOF) {
 check_heredoc_ਸਿਰਾ:
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
			ਜੇ ((heredoc_flags & HEREDOC_QUOTED) || prev != '\\') {
#ਨਹੀਂਤਾਂ /* __U_BOOT__ */
			ਜੇ (prev != '\\') {
#ਜੇ_ਅੰਤ
				/* End-of-line, and not a line continuation */
				ਜੇ (strcmp(heredoc.ਅੰਕੜੇ + past_EOL, ਸ਼ਬਦ) == 0) {
					heredoc.ਅੰਕੜੇ[past_EOL] = '\0';
					debug_printf_heredoc("parsed '%s' heredoc '%s'\n", ਸ਼ਬਦ, heredoc.ਅੰਕੜੇ);
					ਵਾਪਸ heredoc.ਅੰਕੜੇ;
				}
				ਜੇ (ch == '\n') {
					/* This is a new line.
					 * Remember position and backslash-escaping status.
					 */
					o_addchr(&heredoc, ch);
					prev = ch;
 jump_ਵਿੱਚ:
					past_EOL = heredoc.ਲੰਮਾਈ;
					/* Get 1st char of next line, possibly skipping leading tabs */
					ਕਰੋ {
						ch = i_getch(ਨਿਵੇਸ਼);
						ਜੇ (ch != EOF)
							nommu_addchr(as_string, ch);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
					} ਜਦੋਂ ((heredoc_flags & HEREDOC_SKIPTABS) && ch == '\t');
#ਨਹੀਂਤਾਂ /* __U_BOOT__ */
				} ਜਦੋਂ (ch == '\t');
#ਜੇ_ਅੰਤ
					/* If this immediately ended the line,
					 * go back to end-of-line checks.
					 */
					ਜੇ (ch == '\n')
						ਜਾਓ check_heredoc_ਸਿਰਾ;
				}
			} ਨਹੀਂਤਾਂ {
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
				heredoc.ਅੰਕੜੇ[--heredoc.ਲੰਮਾਈ] = '\0';
				prev = 0; /* not '\' */
				ਜਾਰੀ;
			}
		}
		ਜੇ (ch == EOF) {
			o_free(&heredoc);
			ਵਾਪਸ ਨਿਰਾ; /* error */
		}
		o_addchr(&heredoc, ch);
		nommu_addchr(as_string, ch);
		ਜੇ (prev == '\\' && ch == '\\')
			/* Correctly handle foo\\<eol> (not a line cont.) */
			prev = 0; /* not '\' */
		ਨਹੀਂਤਾਂ
			prev = ch;
	}
}
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */

/* Look at entire parse tree for not-yet-loaded REDIRECT_HEREDOCs
 * and load them all. There should be exactly heredoc_cnt of them.
 */
#ਜੇ BB_MMU
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ fetch_heredocs(as_string, pi, heredoc_cnt, ਨਿਵੇਸ਼) \
	fetch_heredocs(pi, heredoc_cnt, ਨਿਵੇਸ਼)
#ਜੇ_ਅੰਤ
ਸਥਿਰ ਪੂਰਨ fetch_heredocs(o_string *as_string, ਬਣਤਰ pipe *pi, ਪੂਰਨ heredoc_cnt, ਬਣਤਰ ਵਿੱਚ_str *ਨਿਵੇਸ਼)
{
	ਜਦੋਂ (pi && heredoc_cnt) {
		ਪੂਰਨ i;
		ਬਣਤਰ command *cmd = pi->cmds;

		debug_printf_heredoc("fetch_heredocs: num_cmds:%d cmd argv0:'%s'\n",
				pi->ਅੰਕ_cmds,
				cmd->argv ? cmd->argv[0] : "NONE"
		);
		ਲਈ (i = 0; i < pi->ਅੰਕ_cmds; i++) {
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
			ਬਣਤਰ redir_struct *redir = cmd->redirects;

#ਜੇ_ਅੰਤ /* !__U_BOOT__ */
			debug_printf_heredoc("fetch_heredocs: %d cmd argv0:'%s'\n",
					i, cmd->argv ? cmd->argv[0] : "NONE");
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
			ਜਦੋਂ (redir) {
				ਜੇ (redir->rd_type == REDIRECT_HEREDOC) {
					ਅੱਖਰ *p;

					redir->rd_type = REDIRECT_HEREDOC2;
					/* redir->rd_dup is (ab)used to indicate <<- */
					p = fetch_till_str(as_string, ਨਿਵੇਸ਼,
							redir->rd_filename, redir->rd_dup);
					ਜੇ (!p) {
						syntax_error("unexpected EOF in here document");
						ਵਾਪਸ -1;
					}
					ਮੁਕਤ(redir->rd_filename);
					redir->rd_filename = p;
					heredoc_cnt--;
				}
				redir = redir->ਅਗਲਾ;
			}
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */
			ਜੇ (cmd->ਸਮੂਹ) {
				//bb_error_msg("%s:%u heredoc_cnt:%d", __func__, __LINE__, heredoc_cnt);
				heredoc_cnt = fetch_heredocs(as_string, cmd->ਸਮੂਹ, heredoc_cnt, ਨਿਵੇਸ਼);
				//bb_error_msg("%s:%u heredoc_cnt:%d", __func__, __LINE__, heredoc_cnt);
				ਜੇ (heredoc_cnt < 0)
					ਵਾਪਸ heredoc_cnt; /* error */
			}
			cmd++;
		}
		pi = pi->ਅਗਲਾ;
	}
	ਵਾਪਸ heredoc_cnt;
}

ਸਥਿਰ ਪੂਰਨ ਲੜੀਵਾਰ_list(ਬਣਤਰ pipe *pi);
#ਜੇ BB_MMU
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ parse_ਧਾਰਾ(pstring, heredoc_cnt_ptr, ਨਿਵੇਸ਼, ਸਿਰਾ_trigger) \
	parse_ਧਾਰਾ(heredoc_cnt_ptr, ਨਿਵੇਸ਼, ਸਿਰਾ_trigger)
#ਜੇ_ਅੰਤ
ਸਥਿਰ ਬਣਤਰ pipe *parse_ਧਾਰਾ(ਅੱਖਰ **pstring,
		ਪੂਰਨ *heredoc_cnt_ptr,
		ਬਣਤਰ ਵਿੱਚ_str *ਨਿਵੇਸ਼,
		ਪੂਰਨ ਸਿਰਾ_trigger);

/* Returns number of heredocs not yet consumed,
 * or -1 on error.
 */
ਸਥਿਰ ਪੂਰਨ parse_ਸਮੂਹ(ਬਣਤਰ parse_context *ctx,
		ਬਣਤਰ ਵਿੱਚ_str *ਨਿਵੇਸ਼, ਪੂਰਨ ch)
{
	/* ctx->word contains characters seen prior to ( or {.
	 * Typically it's empty, but for function defs,
	 * it contains function name (without '()'). */
#ਜੇ BB_MMU
# define as_string ਨਿਰਾ
#ਨਹੀਂਤਾਂ
	ਅੱਖਰ *as_string = ਨਿਰਾ;
#ਜੇ_ਅੰਤ
	ਬਣਤਰ pipe *pipe_list;
	ਪੂਰਨ heredoc_cnt = 0;
	ਪੂਰਨ endch;
	ਬਣਤਰ command *command = ctx->command;

	debug_printf_parse("parse_group entered\n");
#ਜੇ ENABLE_HUSH_FUNCTIONS
	ਜੇ (ch == '(' && !ctx->ਸ਼ਬਦ.has_quoted_part) {
		ਜੇ (ctx->ਸ਼ਬਦ.ਲੰਮਾਈ)
			ਜੇ (ਪੂਰਾ_ਸ਼ਬਦ(ctx))
				ਵਾਪਸ -1;
		ਜੇ (!command->argv)
			ਜਾਓ skip; /* (... */
		ਜੇ (command->argv[1]) { /* word word ... (... */
			syntax_error_unexpected_ch('(');
			ਵਾਪਸ -1;
		}
		/* it is "word(..." or "word (..." */
		ਕਰੋ
			ch = i_getch(ਨਿਵੇਸ਼);
		ਜਦੋਂ (ch == ' ' || ch == '\t');
		ਜੇ (ch != ')') {
			syntax_error_unexpected_ch(ch);
			ਵਾਪਸ -1;
		}
		nommu_addchr(&ctx->as_string, ch);
		ਕਰੋ
			ch = i_getch(ਨਿਵੇਸ਼);
		ਜਦੋਂ (ch == ' ' || ch == '\t' || ch == '\n');
		ਜੇ (ch != '{' && ch != '(') {
			syntax_error_unexpected_ch(ch);
			ਵਾਪਸ -1;
		}
//bash allows functions named "123", "..", "return"!
//		if (endofname(command->argv[0])[0] != '\0') {
//			syntax_error("bad function name");
//			return -1;
//		}
		nommu_addchr(&ctx->as_string, ch);
		command->cmd_type = CMD_FUNCDEF;
		ਜਾਓ skip;
	}
#ਜੇ_ਅੰਤ

#ਜੇ 0 /* Prevented by caller */
	ਜੇ (command->argv /* word [word]{... */
	 || ctx->ਸ਼ਬਦ.ਲੰਮਾਈ /* word{... */
	 || ctx->ਸ਼ਬਦ.has_quoted_part /* ""{... */
	) {
		syntax_error(ਨਿਰਾ);
		debug_printf_parse("parse_group return -1: "
			"syntax error, groups and arglists don't mix\n");
		ਵਾਪਸ -1;
	}
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
 IF_HUSH_FUNCTIONS(skip:)
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */

	endch = '}';
	ਜੇ (ch == '(') {
		endch = ')';
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
		IF_HUSH_FUNCTIONS(ਜੇ (command->cmd_type != CMD_FUNCDEF))
			command->cmd_type = CMD_SUBSHELL;
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */
	} ਨਹੀਂਤਾਂ {
		/* bash does not allow "{echo...", requires whitespace */
		ch = i_peek(ਨਿਵੇਸ਼);
		ਜੇ (ch != ' ' && ch != '\t' && ch != '\n'
		 && ch != '('	/* but "{(..." is allowed (without whitespace) */
		) {
			syntax_error_unexpected_ch(ch);
			ਵਾਪਸ -1;
		}
		ਜੇ (ch != '(') {
			ch = i_getch(ਨਿਵੇਸ਼);
			nommu_addchr(&ctx->as_string, ch);
		}
	}

	debug_printf_heredoc("calling parse_stream, heredoc_cnt:%d\n", heredoc_cnt);
	pipe_list = parse_ਧਾਰਾ(&as_string, &heredoc_cnt, ਨਿਵੇਸ਼, endch);
	debug_printf_heredoc("parse_stream returned: heredoc_cnt:%d\n", heredoc_cnt);
#ਜੇ !BB_MMU
	ਜੇ (as_string)
		o_addstr(&ctx->as_string, as_string);
#ਜੇ_ਅੰਤ

	/* empty ()/{} or parse error? */
	ਜੇ (!pipe_list || pipe_list == ERR_PTR) {
		/* parse_stream already emitted error msg */
		ਜੇ (!BB_MMU)
			ਮੁਕਤ(as_string);
		debug_printf_parse("parse_group return -1: "
			"parse_stream returned %p\n", pipe_list);
		ਵਾਪਸ -1;
	}
#ਜੇ !BB_MMU
	as_string[ਲੜੀ_ਲੰਬਾਈ(as_string) - 1] = '\0'; /* plink ')' or '}' */
	command->ਸਮੂਹ_as_string = as_string;
	debug_printf_parse("end of group, remembering as:'%s'\n",
			command->ਸਮੂਹ_as_string);
#ਜੇ_ਅੰਤ

#ਜੇ ENABLE_HUSH_FUNCTIONS
	/* Convert "f() (cmds)" to "f() {(cmds)}" */
	ਜੇ (command->cmd_type == CMD_FUNCDEF && endch == ')') {
		ਬਣਤਰ command *cmd2;

		cmd2 = xzalloc(ਆਕਾਰ(*cmd2));
		cmd2->cmd_type = CMD_SUBSHELL;
		cmd2->ਸਮੂਹ = pipe_list;
# ਜੇ !BB_MMU
//UNTESTED!
		cmd2->ਸਮੂਹ_as_string = command->ਸਮੂਹ_as_string;
		command->ਸਮੂਹ_as_string = xasprintf("(%s)", command->ਸਮੂਹ_as_string);
# endif

		pipe_list = new_pipe();
		pipe_list->cmds = cmd2;
		pipe_list->ਅੰਕ_cmds = 1;
	}
#ਜੇ_ਅੰਤ

	command->ਸਮੂਹ = pipe_list;

	debug_printf_parse("parse_group return %d\n", heredoc_cnt);
	ਵਾਪਸ heredoc_cnt;
	/* command remains "open", available for possible redirects */
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ as_string
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
#ਜੇ ENABLE_HUSH_TICK || ENABLE_FEATURE_SH_MATH || ENABLE_HUSH_DOLLAR_OPS
/* Subroutines for copying $(...) and `...` things */
/* '...' */
ਸਥਿਰ ਪੂਰਨ ਜੋੜੋ_till_single_quote(o_string *dest, ਬਣਤਰ ਵਿੱਚ_str *ਨਿਵੇਸ਼)
{
	ਜਦੋਂ (1) {
		ਪੂਰਨ ch = i_getch(ਨਿਵੇਸ਼);
		ਜੇ (ch == EOF) {
			syntax_error_unterm_ch('\'');
			ਵਾਪਸ 0;
		}
		ਜੇ (ch == '\'')
			ਵਾਪਸ 1;
		o_addchr(dest, ch);
	}
}
ਸਥਿਰ ਪੂਰਨ ਜੋੜੋ_till_single_quote_dquoted(o_string *dest, ਬਣਤਰ ਵਿੱਚ_str *ਨਿਵੇਸ਼)
{
	ਜਦੋਂ (1) {
		ਪੂਰਨ ch = i_getch(ਨਿਵੇਸ਼);
		ਜੇ (ch == EOF) {
			syntax_error_unterm_ch('\'');
			ਵਾਪਸ 0;
		}
		ਜੇ (ch == '\'')
			ਵਾਪਸ 1;
		o_addqchr(dest, ch);
	}
}

/* "...\"...`..`...." - do we need to handle "...$(..)..." too? */
ਸਥਿਰ ਪੂਰਨ ਜੋੜੋ_till_backquote(o_string *dest, ਬਣਤਰ ਵਿੱਚ_str *ਨਿਵੇਸ਼, ਪੂਰਨ ਵਿੱਚ_dquote);
ਸਥਿਰ ਪੂਰਨ ਜੋੜੋ_till_double_quote(o_string *dest, ਬਣਤਰ ਵਿੱਚ_str *ਨਿਵੇਸ਼)
{
	ਜਦੋਂ (1) {
		ਪੂਰਨ ch = i_getch(ਨਿਵੇਸ਼);
		ਜੇ (ch == EOF) {
			syntax_error_unterm_ch('"');
			ਵਾਪਸ 0;
		}
		ਜੇ (ch == '"')
			ਵਾਪਸ 1;
		ਜੇ (ch == '\\') {  /* \x. Copy both chars. */
			o_addchr(dest, ch);
			ch = i_getch(ਨਿਵੇਸ਼);
		}
		o_addchr(dest, ch);
		ਜੇ (ch == '`') {
			ਜੇ (!ਜੋੜੋ_till_backquote(dest, ਨਿਵੇਸ਼, /*in_dquote:*/ 1))
				ਵਾਪਸ 0;
			o_addchr(dest, ch);
			ਜਾਰੀ;
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
ਸਥਿਰ ਪੂਰਨ ਜੋੜੋ_till_backquote(o_string *dest, ਬਣਤਰ ਵਿੱਚ_str *ਨਿਵੇਸ਼, ਪੂਰਨ ਵਿੱਚ_dquote)
{
	ਜਦੋਂ (1) {
		ਪੂਰਨ ch = i_getch(ਨਿਵੇਸ਼);
		ਜੇ (ch == '`')
			ਵਾਪਸ 1;
		ਜੇ (ch == '\\') {
			/* \x. Copy both unless it is \`, \$, \\ and maybe \" */
			ch = i_getch(ਨਿਵੇਸ਼);
			ਜੇ (ch != '`'
			 && ch != '$'
			 && ch != '\\'
			 && (!ਵਿੱਚ_dquote || ch != '"')
			) {
				o_addchr(dest, '\\');
			}
		}
		ਜੇ (ch == EOF) {
			syntax_error_unterm_ch('`');
			ਵਾਪਸ 0;
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
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DOUBLE_CLOSE_CHAR_FLAG 0x80
ਸਥਿਰ ਪੂਰਨ ਜੋੜੋ_till_closing_bracket(o_string *dest, ਬਣਤਰ ਵਿੱਚ_str *ਨਿਵੇਸ਼, ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਿਰਾ_ch)
{
	ਪੂਰਨ ch;
	ਅੱਖਰ dbl = ਸਿਰਾ_ch & DOUBLE_CLOSE_CHAR_FLAG;
# ਜੇ BASH_SUBSTR || BASH_ਨਮੂਨਾ_SUBST
	ਅੱਖਰ ਸਿਰਾ_char2 = ਸਿਰਾ_ch >> 8;
# endif
	ਸਿਰਾ_ch &= (DOUBLE_CLOSE_CHAR_FLAG - 1);

# ਜੇ ENABLE_HUSH_INTERACTIVE
	G.promptmode = 1; /* PS2 */
# endif
	debug_printf_prompt("%s promptmode=%d\n", __func__, G.promptmode);

	ਜਦੋਂ (1) {
		ch = i_getch(ਨਿਵੇਸ਼);
		ਜੇ (ch == EOF) {
			syntax_error_unterm_ch(ਸਿਰਾ_ch);
			ਵਾਪਸ 0;
		}
		ਜੇ (ch == ਸਿਰਾ_ch
# ਜੇ BASH_SUBSTR || BASH_ਨਮੂਨਾ_SUBST
		 || ch == ਸਿਰਾ_char2
# endif
		) {
			ਜੇ (!dbl)
				ਰੋਕੋ;
			/* we look for closing )) of $((EXPR)) */
			ਜੇ (i_peek_and_eat_bkslash_nl(ਨਿਵੇਸ਼) == ਸਿਰਾ_ch) {
				i_getch(ਨਿਵੇਸ਼); /* eat second ')' */
				ਰੋਕੋ;
			}
		}
		o_addchr(dest, ch);
		//bb_error_msg("%s:o_addchr('%c')", __func__, ch);
		ਜੇ (ch == '(' || ch == '{') {
			ch = (ch == '(' ? ')' : '}');
			ਜੇ (!ਜੋੜੋ_till_closing_bracket(dest, ਨਿਵੇਸ਼, ch))
				ਵਾਪਸ 0;
			o_addchr(dest, ch);
			ਜਾਰੀ;
		}
		ਜੇ (ch == '\'') {
			ਜੇ (!ਜੋੜੋ_till_single_quote(dest, ਨਿਵੇਸ਼))
				ਵਾਪਸ 0;
			o_addchr(dest, ch);
			ਜਾਰੀ;
		}
		ਜੇ (ch == '"') {
			ਜੇ (!ਜੋੜੋ_till_double_quote(dest, ਨਿਵੇਸ਼))
				ਵਾਪਸ 0;
			o_addchr(dest, ch);
			ਜਾਰੀ;
		}
		ਜੇ (ch == '`') {
			ਜੇ (!ਜੋੜੋ_till_backquote(dest, ਨਿਵੇਸ਼, /*in_dquote:*/ 0))
				ਵਾਪਸ 0;
			o_addchr(dest, ch);
			ਜਾਰੀ;
		}
		ਜੇ (ch == '\\') {
			/* \x. Copy verbatim. Important for  \(, \) */
			ch = i_getch(ਨਿਵੇਸ਼);
			ਜੇ (ch == EOF) {
				syntax_error_unterm_ch(ਸਿਰਾ_ch);
				ਵਾਪਸ 0;
			}
# ਜੇ 0
			ਜੇ (ch == '\n') {
				/* "backslash+newline", ignore both */
				o_delchr(dest); /* undo insertion of '\' */
				ਜਾਰੀ;
			}
# endif
			o_addchr(dest, ch);
			//bb_error_msg("%s:o_addchr('%c') after '\\'", __func__, ch);
			ਜਾਰੀ;
		}
	}
	debug_printf_parse("%s return '%s' ch:'%c'\n", __func__, dest->ਅੰਕੜੇ, ch);
	ਵਾਪਸ ch;
}
#ਜੇ_ਅੰਤ /* ENABLE_HUSH_TICK || ENABLE_FEATURE_SH_MATH || ENABLE_HUSH_DOLLAR_OPS */

#ਜੇ BASH_DOLLAR_SQUOTE
/* Return code: 1 for "found and parsed", 0 for "seen something else" */
# ਜੇ BB_MMU
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ parse_dollar_squote(as_string, dest, ਨਿਵੇਸ਼) \
	parse_dollar_squote(dest, ਨਿਵੇਸ਼)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ as_string ਨਿਰਾ
# endif
ਸਥਿਰ ਪੂਰਨ parse_dollar_squote(o_string *as_string, o_string *dest, ਬਣਤਰ ਵਿੱਚ_str *ਨਿਵੇਸ਼)
{
	ਪੂਰਨ ਸ਼ੁਰੂ;
	ਪੂਰਨ ch = i_peek_and_eat_bkslash_nl(ਨਿਵੇਸ਼);  /* first character after the $ */
	debug_printf_parse("parse_dollar_squote entered: ch='%c'\n", ch);
	ਜੇ (ch != '\'')
		ਵਾਪਸ 0;

	dest->has_quoted_part = 1;
	ਸ਼ੁਰੂ = dest->ਲੰਮਾਈ;

	ch = i_getch(ਨਿਵੇਸ਼); /* eat ' */
	nommu_addchr(as_string, ch);
	ਜਦੋਂ (1) {
		ch = i_getch(ਨਿਵੇਸ਼);
		nommu_addchr(as_string, ch);
		ਜੇ (ch == EOF) {
			syntax_error_unterm_ch('\'');
			ਵਾਪਸ 0;
		}
		ਜੇ (ch == '\'')
			ਰੋਕੋ;
		ਜੇ (ch == SPECIAL_VAR_ਚਿੰਨ੍ਹ) {
			/* Convert raw ^C to corresponding special variable reference */
			o_addchr(dest, SPECIAL_VAR_ਚਿੰਨ੍ਹ);
			o_addchr(dest, SPECIAL_VAR_QUOTED_SVS);
			/* will addchr() another SPECIAL_VAR_SYMBOL (see after the if() block) */
		} ਨਹੀਂਤਾਂ ਜੇ (ch == '\\') {
			ਸਥਿਰ ਅਚਲ ਅੱਖਰ C_escapes[] ALIGN1 = "nrbtfav""x\\01234567";

			ch = i_getch(ਨਿਵੇਸ਼);
			nommu_addchr(as_string, ch);
			ਜੇ (strchr(C_escapes, ch)) {
				ਅੱਖਰ buf[4];
				ਅੱਖਰ *p = buf;
				ਪੂਰਨ cnt = 2;

				buf[0] = ch;
				ਜੇ ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)(ch - '0') <= 7) { /* \ooo */
					ਕਰੋ {
						ch = i_peek(ਨਿਵੇਸ਼);
						ਜੇ ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)(ch - '0') > 7)
							ਰੋਕੋ;
						*++p = ch = i_getch(ਨਿਵੇਸ਼);
						nommu_addchr(as_string, ch);
					} ਜਦੋਂ (--cnt != 0);
				} ਨਹੀਂਤਾਂ ਜੇ (ch == 'x') { /* \xHH */
					ਕਰੋ {
						ch = i_peek(ਨਿਵੇਸ਼);
						ਜੇ (!isxdigit(ch))
							ਰੋਕੋ;
						*++p = ch = i_getch(ਨਿਵੇਸ਼);
						nommu_addchr(as_string, ch);
					} ਜਦੋਂ (--cnt != 0);
					ਜੇ (cnt == 2) { /* \x but next char is "bad" */
						ch = 'x';
						ਜਾਓ unrecognized;
					}
				} /* else simple seq like \\ or \t */
				*++p = '\0';
				p = buf;
				ch = bb_process_escape_sequence((ਖਾਲੀ*)&p);
				//bb_error_msg("buf:'%s' ch:%x", buf, ch);
				ਜੇ (ch == '\0')
					ਜਾਰੀ; /* bash compat: $'...\0...' emits nothing */
			} ਨਹੀਂਤਾਂ { /* unrecognized "\z": encode both chars unless ' or " */
				ਜੇ (ch != '\'' && ch != '"') {
 unrecognized:
					o_addqchr(dest, '\\');
				}
			}
		} /* if (\...) */
		o_addqchr(dest, ch);
	}

	ਜੇ (dest->ਲੰਮਾਈ == ਸ਼ੁਰੂ) {
		/* $'', $'\0', $'\000\x00' and the like */
		o_addchr(dest, SPECIAL_VAR_ਚਿੰਨ੍ਹ);
		o_addchr(dest, SPECIAL_VAR_ਚਿੰਨ੍ਹ);
	}

	ਵਾਪਸ 1;
# undef as_string
}
#ਨਹੀਂਤਾਂ
# define parse_dollar_squote(as_string, dest, ਨਿਵੇਸ਼) 0
#ਜੇ_ਅੰਤ /* BASH_DOLLAR_SQUOTE */
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */

/* Return code: 0 for OK, 1 for syntax error */
#ਜੇ BB_MMU
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ parse_dollar(as_string, dest, ਨਿਵੇਸ਼, quote_ਨਕਾਬ) \
	parse_dollar(dest, ਨਿਵੇਸ਼, quote_ਨਕਾਬ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ as_string ਨਿਰਾ
#ਜੇ_ਅੰਤ
ਸਥਿਰ ਪੂਰਨ parse_dollar(o_string *as_string,
		o_string *dest,
		ਬਣਤਰ ਵਿੱਚ_str *ਨਿਵੇਸ਼, ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ quote_ਨਕਾਬ)
{
	ਪੂਰਨ ch = i_peek_and_eat_bkslash_nl(ਨਿਵੇਸ਼);  /* first character after the $ */

	debug_printf_parse("parse_dollar entered: ch='%c' quote_mask:0x%x\n", ch, quote_ਨਕਾਬ);
	ਜੇ (ਅੱਖਰ_ਹੈ(ch)) {
 make_var:
		ch = i_getch(ਨਿਵੇਸ਼);
		nommu_addchr(as_string, ch);
 /*make_var1:*/
		o_addchr(dest, SPECIAL_VAR_ਚਿੰਨ੍ਹ);
		ਜਦੋਂ (1) {
			debug_printf_parse(": '%c'\n", ch);
			o_addchr(dest, ch | quote_ਨਕਾਬ);
			quote_ਨਕਾਬ = 0;
			ch = i_peek_and_eat_bkslash_nl(ਨਿਵੇਸ਼);
			ਜੇ (!ਅੱਖਰ_ਜਾਂ_ਅੰਕ(ch) && ch != '_') {
				/* End of variable name reached */
				ਰੋਕੋ;
			}
			ch = i_getch(ਨਿਵੇਸ਼);
			nommu_addchr(as_string, ch);
		}
		o_addchr(dest, SPECIAL_VAR_ਚਿੰਨ੍ਹ);
	} ਨਹੀਂਤਾਂ ਜੇ (ਅੰਕ_ਹੈ(ch)) {
 make_one_char_var:
		ch = i_getch(ਨਿਵੇਸ਼);
		nommu_addchr(as_string, ch);
		o_addchr(dest, SPECIAL_VAR_ਚਿੰਨ੍ਹ);
		debug_printf_parse(": '%c'\n", ch);
		o_addchr(dest, ch | quote_ਨਕਾਬ);
		o_addchr(dest, SPECIAL_VAR_ਚਿੰਨ੍ਹ);
	} ਨਹੀਂਤਾਂ ਚੋਣ (ch) {
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	ਹਾਲਤ '$': /* pid */
	ਹਾਲਤ '!': /* last bg pid */
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */
	ਹਾਲਤ '?': /* last exit code */
	ਹਾਲਤ '#': /* number of args */
	ਹਾਲਤ '*': /*ਦਲੀਲਾਂ*/
	ਹਾਲਤ '@': /*ਦਲੀਲਾਂ*/
	ਹਾਲਤ '-': /* $- option flags set by set builtin or shell options (-i etc) */
		ਜਾਓ make_one_char_var;
	ਹਾਲਤ '{': {
		ਅੱਖਰ len_single_ch;

		o_addchr(dest, SPECIAL_VAR_ਚਿੰਨ੍ਹ);

		ch = i_getch(ਨਿਵੇਸ਼); /* eat '{' */
		nommu_addchr(as_string, ch);

		ch = i_getch_and_eat_bkslash_nl(ਨਿਵੇਸ਼); /* first char after '{' */
		/* It should be ${?}, or ${#var},
		 * or even ${?+subst} - operator acting on a special variable,
		 * or the beginning of variable name.
		 */
		ਜੇ (ch == EOF
		 || (!strchr(_SPECIAL_VARS_STR, ch) && !ਅੱਖਰ_ਜਾਂ_ਅੰਕ(ch)) /* not one of those */
		) {
 bad_dollar_syntax:
			syntax_error_unterm_str("${name}");
			debug_printf_parse("parse_dollar return 0: unterminated ${name}\n");
			ਵਾਪਸ 0;
		}
		nommu_addchr(as_string, ch);
		len_single_ch = ch;
		ch |= quote_ਨਕਾਬ;

		/* It's possible to just call add_till_closing_bracket() at this point.
		 * However, this regresses some of our testsuite cases
		 * which check invalid constructs like ${%}.
		 * Oh well... let's check that the var name part is fine... */

		ਜੇ (ਅੰਕ_ਹੈ(len_single_ch)
		 || (len_single_ch == '#' && ਅੰਕ_ਹੈ(i_peek_and_eat_bkslash_nl(ਨਿਵੇਸ਼)))
		) {
			/* Execution engine uses plain xatoi_positive()
			 * to interpret ${NNN} and {#NNN},
			 * check syntax here in the parser.
			 * (bash does not support expressions in ${#NN},
			 * e.g. ${#$var} and {#1:+WORD} are not supported).
			 */
			ਬਿਨਾਨਿਸ਼ਾਨੀ cnt = 9; /* max 9 digits for ${NN} and 8 for {#NN} */
			ਜਦੋਂ (1) {
				o_addchr(dest, ch);
				debug_printf_parse(": '%c'\n", ch);
				ch = i_getch_and_eat_bkslash_nl(ਨਿਵੇਸ਼);
				nommu_addchr(as_string, ch);
				ਜੇ (ch == '}')
					ਰੋਕੋ;
				ਜੇ (--cnt == 0)
					ਜਾਓ bad_dollar_syntax;
				ਜੇ (len_single_ch != '#' && strchr(VAR_SUBST_OPS, ch))
					/* ${NN<op>...} is valid */
					ਜਾਓ eat_until_closing;
				ਜੇ (!ਅੰਕ_ਹੈ(ch))
					ਜਾਓ bad_dollar_syntax;
			}
		} ਨਹੀਂਤਾਂ
		ਜਦੋਂ (1) {
			ਬਿਨਾਨਿਸ਼ਾਨੀ pos;

			o_addchr(dest, ch);
			debug_printf_parse(": '%c'\n", ch);

			ch = i_getch(ਨਿਵੇਸ਼);
			nommu_addchr(as_string, ch);
			ਜੇ (ch == '}')
				ਰੋਕੋ;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
			ਜੇ (!ਅੱਖਰ_ਜਾਂ_ਅੰਕ(ch) && ch != '_') {
#ਨਹੀਂਤਾਂ /* __U_BOOT__ */
			/*
			 * In several places in U-Boot, we use variable like
			 * foo# (e.g. serial#), particularly in env.
			 * So, we need to authorize # to appear inside
			 * variable name and then expand this variable.
			 * NOTE Having # in variable name is not permitted in
			 * upstream hush but expansion will be done (even though
			 * the result will be empty).
			 */
			ਜੇ (!ਅੱਖਰ_ਜਾਂ_ਅੰਕ(ch) && ch != '_' && ch != '#') {
#ਜੇ_ਅੰਤ /* __U_BOOT__ */
				ਬਿਨਾਨਿਸ਼ਾਨੀ ਸਿਰਾ_ch;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
				ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ ਆਖਰੀ_ch;
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */
				/* handle parameter expansions
				 * http://www.opengroup.org/onlinepubs/009695399/utilities/xcu_chap02.html#tag_02_06_02
				 */
				ਜੇ (!strchr(VAR_SUBST_OPS, ch)) { /* ${var<bad_char>... */
					ਜੇ (len_single_ch != '#'
					/*|| !strchr(SPECIAL_VARS_STR, ch) - disallow errors like ${#+} ? */
					 || i_peek(ਨਿਵੇਸ਼) != '}'
					) {
						ਜਾਓ bad_dollar_syntax;
					}
					/* else: it's "length of C" ${#C} op,
					 * where C is a single char
					 * special var name, e.g. ${#!}.
					 */
				}
 eat_until_closing:
				/* Eat everything until closing '}' (or ':') */
				ਸਿਰਾ_ch = '}';
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
				ਜੇ (BASH_SUBSTR
				 && ch == ':'
				 && !strchr(MINUS_PLUS_EQUAL_QUESTION, i_peek(ਨਿਵੇਸ਼))
				) {
					/* It's ${var:N[:M]} thing */
					ਸਿਰਾ_ch = '}' * 0x100 + ':';
				}
				ਜੇ (BASH_ਨਮੂਨਾ_SUBST
				 && ch == '/'
				) {
					/* It's ${var/[/]pattern[/repl]} thing */
					ਜੇ (i_peek(ਨਿਵੇਸ਼) == '/') { /* ${var//pattern[/repl]}? */
						i_getch(ਨਿਵੇਸ਼);
						nommu_addchr(as_string, '/');
						ch = '\\';
					}
					ਸਿਰਾ_ch = '}' * 0x100 + '/';
				}
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */
				o_addchr(dest, ch);
				/* The pattern can't be empty.
				 * IOW: if the first char after "${v//" is a slash,
				 * it does not terminate the pattern - it's the first char of the pattern:
				 *  v=/dev/ram; echo ${v////-}  prints -dev-ram (pattern is "/")
				 *  v=/dev/ram; echo ${v///r/-} prints /dev-am  (pattern is "/r")
				 */
				ਜੇ (i_peek(ਨਿਵੇਸ਼) == '/') {
					o_addchr(dest, i_getch(ਨਿਵੇਸ਼));
				}
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
 again:
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */
				ਜੇ (!BB_MMU)
					pos = dest->ਲੰਮਾਈ;
#ਜੇ ENABLE_HUSH_DOLLAR_OPS
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
				ਆਖਰੀ_ch = ਜੋੜੋ_till_closing_bracket(dest, ਨਿਵੇਸ਼, ਸਿਰਾ_ch);
				ਜੇ (ਆਖਰੀ_ch == 0) /* error? */
					ਵਾਪਸ 0;
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */
#ਨਹੀਂਤਾਂ
# error Simple ਕੋਡ ਮੰਜ਼ਿਲ only allow ${var} is not implemented
#ਜੇ_ਅੰਤ
				ਜੇ (as_string) {
					o_addstr(as_string, dest->ਅੰਕੜੇ + pos);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
					o_addchr(as_string, ਆਖਰੀ_ch);
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */
				}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
				ਜੇ ((BASH_SUBSTR || BASH_ਨਮੂਨਾ_SUBST)
					 && (ਸਿਰਾ_ch & 0xff00)
				) {
					/* close the first block: */
					o_addchr(dest, SPECIAL_VAR_ਚਿੰਨ੍ਹ);
					/* while parsing N from ${var:N[:M]}
					 * or pattern from ${var/[/]pattern[/repl]} */
					ਜੇ ((ਸਿਰਾ_ch & 0xff) == ਆਖਰੀ_ch) {
						/* got ':' or '/'- parse the rest */
						ਸਿਰਾ_ch = '}';
						ਜਾਓ again;
					}
					/* got '}' */
					ਜੇ (BASH_SUBSTR && ਸਿਰਾ_ch == '}' * 0x100 + ':') {
						/* it's ${var:N} - emulate :999999999 */
						o_addstr(dest, "999999999");
					} /* else: it's ${var/[/]pattern} */
				}
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */
				ਰੋਕੋ;
			}
			len_single_ch = 0; /* it can't be ${#C} op */
		}
		o_addchr(dest, SPECIAL_VAR_ਚਿੰਨ੍ਹ);
		ਰੋਕੋ;
	}
#ਜੇ ENABLE_FEATURE_SH_MATH || ENABLE_HUSH_TICK
	ਹਾਲਤ '(': {
		ਬਿਨਾਨਿਸ਼ਾਨੀ pos;

		ch = i_getch(ਨਿਵੇਸ਼);
		nommu_addchr(as_string, ch);
# ਜੇ ENABLE_FEATURE_SH_MATH
		ਜੇ (i_peek_and_eat_bkslash_nl(ਨਿਵੇਸ਼) == '(') {
			ch = i_getch(ਨਿਵੇਸ਼);
			nommu_addchr(as_string, ch);
			o_addchr(dest, SPECIAL_VAR_ਚਿੰਨ੍ਹ);
			o_addchr(dest, quote_ਨਕਾਬ | '+');
			ਜੇ (!BB_MMU)
				pos = dest->ਲੰਮਾਈ;
			ਜੇ (!ਜੋੜੋ_till_closing_bracket(dest, ਨਿਵੇਸ਼, ')' | DOUBLE_CLOSE_CHAR_FLAG))
				ਵਾਪਸ 0; /* error */
			ਜੇ (as_string) {
				o_addstr(as_string, dest->ਅੰਕੜੇ + pos);
				o_addchr(as_string, ')');
				o_addchr(as_string, ')');
			}
			o_addchr(dest, SPECIAL_VAR_ਚਿੰਨ੍ਹ);
			ਰੋਕੋ;
		}
# endif
# ਜੇ ENABLE_HUSH_TICK
		o_addchr(dest, SPECIAL_VAR_ਚਿੰਨ੍ਹ);
		o_addchr(dest, quote_ਨਕਾਬ | '`');
		ਜੇ (!BB_MMU)
			pos = dest->ਲੰਮਾਈ;
		ਜੇ (!ਜੋੜੋ_till_closing_bracket(dest, ਨਿਵੇਸ਼, ')'))
			ਵਾਪਸ 0; /* error */
		ਜੇ (as_string) {
			o_addstr(as_string, dest->ਅੰਕੜੇ + pos);
			o_addchr(as_string, ')');
		}
		o_addchr(dest, SPECIAL_VAR_ਚਿੰਨ੍ਹ);
# endif
		ਰੋਕੋ;
	}
#ਜੇ_ਅੰਤ
	ਹਾਲਤ '_':
		ਜਾਓ make_var;
#ਜੇ 0
	/* TODO: $_: */
	/* $_ Shell or shell script name; or last argument of last command
	 * (if last command wasn't a pipe; if it was, bash sets $_ to "");
	 * but in command's env, set to full pathname used to invoke it */
		ch = i_getch(ਨਿਵੇਸ਼);
		nommu_addchr(as_string, ch);
		ch = i_peek_and_eat_bkslash_nl(ਨਿਵੇਸ਼);
		ਜੇ (ਅੱਖਰ_ਜਾਂ_ਅੰਕ(ch)) { /* it's $_name or $_123 */
			ch = '_';
			ਜਾਓ make_var1;
		}
		/* else: it's $_ */
#ਜੇ_ਅੰਤ
	ਮੂਲ:
		o_ਜੋੜੋ_Qchr(dest, '$');
	}
	debug_printf_parse("parse_dollar return 1 (ok)\n");
	ਵਾਪਸ 1;
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ as_string
}

#ਜੇ BB_MMU
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ encode_string(as_string, dest, ਨਿਵੇਸ਼, dquote_ਸਿਰਾ) \
	encode_string(dest, ਨਿਵੇਸ਼, dquote_ਸਿਰਾ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ as_string ਨਿਰਾ
#ਜੇ_ਅੰਤ
ਸਥਿਰ ਪੂਰਨ encode_string(o_string *as_string,
		o_string *dest,
		ਬਣਤਰ ਵਿੱਚ_str *ਨਿਵੇਸ਼,
		ਪੂਰਨ dquote_ਸਿਰਾ)
{
	ਪੂਰਨ ch;
	ਪੂਰਨ ਅਗਲਾ;

 again:
	ch = i_getch(ਨਿਵੇਸ਼);
	ਜੇ (ch != EOF)
		nommu_addchr(as_string, ch);
	ਜੇ (ch == dquote_ਸਿਰਾ) { /* may be only '"' or EOF */
		debug_printf_parse("encode_string return 1 (ok)\n");
		ਵਾਪਸ 1;
	}
	/* note: can't move it above ch == dquote_end check! */
	ਜੇ (ch == EOF) {
		syntax_error_unterm_ch('"');
		ਵਾਪਸ 0; /* error */
	}
	ਅਗਲਾ = '\0';
	ਜੇ (ch != '\n') {
		ਅਗਲਾ = i_peek(ਨਿਵੇਸ਼);
	}
	debug_printf_parse("\" ch=%c (%d) escape=%d\n",
			ch, ch, !!(dest->o_expflags & EXP_FLAG_ESC_GLOB_CHARS));
	ਜੇ (ch == '\\') {
		ਜੇ (ਅਗਲਾ == EOF) {
			/* Testcase: in interactive shell a file with
			 *  echo "unterminated string\<eof>
			 * is sourced.
			 */
			syntax_error_unterm_ch('"');
			ਵਾਪਸ 0; /* error */
		}
		/* bash:
		 * "The backslash retains its special meaning [in "..."]
		 * only when followed by one of the following characters:
		 * $, `, ", \, or <newline>.  A double quote may be quoted
		 * within double quotes by preceding it with a backslash."
		 * NB: in (unquoted) heredoc, above does not apply to ",
		 * therefore we check for it by "next == dquote_end" cond.
		 */
		ਜੇ (ਅਗਲਾ == dquote_ਸਿਰਾ || strchr("$`\\\n", ਅਗਲਾ)) {
			ch = i_getch(ਨਿਵੇਸ਼); /* eat next */
			ਜੇ (ch == '\n')
				ਜਾਓ again; /* skip \<newline> */
		} /* else: ch remains == '\\', and we double it below: */
		o_addqchr(dest, ch); /* \c if c is a glob char, else just c */
		nommu_addchr(as_string, ch);
		ਜਾਓ again;
	}
	ਜੇ (ch == '$') {
		//if (parse_dollar_squote(as_string, dest, input))
		//	goto again;
		ਜੇ (!parse_dollar(as_string, dest, ਨਿਵੇਸ਼, /*quote_mask:*/ 0x80)) {
			debug_printf_parse("encode_string return 0: "
					"parse_dollar returned 0 (error)\n");
			ਵਾਪਸ 0;
		}
		ਜਾਓ again;
	}
#ਜੇ ENABLE_HUSH_TICK
	ਜੇ (ch == '`') {
		//unsigned pos = dest->length;
		o_addchr(dest, SPECIAL_VAR_ਚਿੰਨ੍ਹ);
		o_addchr(dest, 0x80 | '`');
		ਜੇ (!ਜੋੜੋ_till_backquote(dest, ਨਿਵੇਸ਼, /*in_dquote:*/ dquote_ਸਿਰਾ == '"'))
			ਵਾਪਸ 0; /* error */
		o_addchr(dest, SPECIAL_VAR_ਚਿੰਨ੍ਹ);
		//debug_printf_subst("SUBST RES3 '%s'\n", dest->data + pos);
		ਜਾਓ again;
	}
#ਜੇ_ਅੰਤ
	o_ਜੋੜੋ_Qchr(dest, ch);
	ਜੇ (ch == SPECIAL_VAR_ਚਿੰਨ੍ਹ) {
		/* Convert "^C" to corresponding special variable reference */
		o_addchr(dest, SPECIAL_VAR_QUOTED_SVS);
		o_addchr(dest, SPECIAL_VAR_ਚਿੰਨ੍ਹ);
	}
	ਜਾਓ again;
#ਪਰਿਭਾਸ਼ਾ_ਰੱਦ as_string
}

/*
 * Scan input until EOF or end_trigger char.
 * Return a list of pipes to execute, or NULL on EOF
 * or if end_trigger character is met.
 * On syntax error, exit if shell is not interactive,
 * reset parsing machinery and start parsing anew,
 * or return ERR_PTR.
 */
ਸਥਿਰ ਬਣਤਰ pipe *parse_ਧਾਰਾ(ਅੱਖਰ **pstring,
		ਪੂਰਨ *heredoc_cnt_ptr,
		ਬਣਤਰ ਵਿੱਚ_str *ਨਿਵੇਸ਼,
		ਪੂਰਨ ਸਿਰਾ_trigger)
{
	ਬਣਤਰ parse_context ctx;
	ਪੂਰਨ heredoc_cnt;

	/* Single-quote triggers a bypass of the main loop until its mate is
	 * found.  When recursing, quote state is passed in via ctx.word.o_expflags.
	 */
	debug_printf_parse("parse_stream entered, end_trigger='%c'\n",
			ਸਿਰਾ_trigger ? ਸਿਰਾ_trigger : 'X');
	debug_enter();

	initialize_context(&ctx);

	/* If very first arg is "" or '', ctx.word.data may end up NULL.
	 * Preventing this:
	 */
	ctx.ਸ਼ਬਦ.ਅੰਕੜੇ = xzalloc(1); /* start as "", not as NULL */

	/* We used to separate words on $IFS here. This was wrong.
	 * $IFS is used only for word splitting when $var is expanded,
	 * here we should use blank chars as separators, not $IFS
	 */

	heredoc_cnt = 0;
	ਜਦੋਂ (1) {
		ਅਚਲ ਅੱਖਰ *is_blank;
		ਅਚਲ ਅੱਖਰ *is_special;
		ਪੂਰਨ ch;
		ਪੂਰਨ ਅਗਲਾ;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
		ਪੂਰਨ redir_fd;
		redir_type redir_style;
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */

		ch = i_getch(ਨਿਵੇਸ਼);
		debug_printf_parse(": ch=%c (%d) escape=%d\n",
				ch, ch, !!(ctx.ਸ਼ਬਦ.o_expflags & EXP_FLAG_ESC_GLOB_CHARS));
		ਜੇ (ch == EOF) {
			ਬਣਤਰ pipe *pi;

			ਜੇ (heredoc_cnt) {
				syntax_error_unterm_str("here document");
				ਜਾਓ parse_error_exitcode1;
			}
			ਜੇ (ਸਿਰਾ_trigger == ')') {
				syntax_error_unterm_ch('(');
				ਜਾਓ parse_error_exitcode1;
			}
			ਜੇ (ਸਿਰਾ_trigger == '}') {
				syntax_error_unterm_ch('{');
				ਜਾਓ parse_error_exitcode1;
			}

			ਜੇ (ਪੂਰਾ_ਸ਼ਬਦ(&ctx)) {
				ਜਾਓ parse_error_exitcode1;
			}
			o_free_and_ਸੈੱਟ_NULL(&ctx.ਸ਼ਬਦ);
			ਪੂਰਾ_pipe(&ctx, PIPE_SEQ);

			/* Do we sit inside of any if's, loops or case's? */
			ਜੇ (HAS_KEYWORDS
			IF_HAS_KEYWORDS(&& (ctx.ctx_res_w != RES_NONE || ctx.old_flag != 0))
			) {
				syntax_error_unterm_str("compound statement");
				ਜਾਓ parse_error_exitcode1;
			}

			pi = ctx.list_ਸਿਰ;
			/* If we got nothing... */
			/* (this makes bare "&" cmd a no-op.
			 * bash says: "syntax error near unexpected token '&'") */
			ਜੇ (pi->ਅੰਕ_cmds == 0
			IF_HAS_KEYWORDS(&& pi->res_ਸ਼ਬਦ == RES_NONE)
			) {
				free_pipe_list(pi);
				pi = ਨਿਰਾ;
			}
#ਜੇ !BB_MMU
			debug_printf_parse("as_string1 '%s'\n", ctx.as_string.ਅੰਕੜੇ);
			ਜੇ (pstring)
				*pstring = ctx.as_string.ਅੰਕੜੇ;
			ਨਹੀਂਤਾਂ
				o_free(&ctx.as_string);
#ਜੇ_ਅੰਤ
			// heredoc_cnt must be 0 here anyway
			//if (heredoc_cnt_ptr)
			//	*heredoc_cnt_ptr = heredoc_cnt;
			debug_leave();
			debug_printf_heredoc("parse_stream return heredoc_cnt:%d\n", heredoc_cnt);
			debug_printf_parse("parse_stream return %p: EOF\n", pi);
			ਵਾਪਸ pi;
		}

		/* Handle "'" and "\" first, as they won't play nice with
		 * i_peek_and_eat_bkslash_nl() anyway:
		 *   echo z\\
		 * and
		 *   echo '\
		 *   '
		 * would break.
		 */
		ਜੇ (ch == '\\') {
			ch = i_getch(ਨਿਵੇਸ਼);
			ਜੇ (ch == '\n')
				ਜਾਰੀ; /* drop \<newline>, get next char */
			nommu_addchr(&ctx.as_string, '\\');
			ਜੇ (ch == SPECIAL_VAR_ਚਿੰਨ੍ਹ) {
				nommu_addchr(&ctx.as_string, ch);
				/* Convert \^C to corresponding special variable reference */
				ਜਾਓ case_SPECIAL_VAR_ਚਿੰਨ੍ਹ;
			}
			o_addchr(&ctx.ਸ਼ਬਦ, '\\');
			ਜੇ (ch == EOF) {
				/* Testcase: eval 'echo Ok\' */
				/* bash-4.3.43 was removing backslash,
				 * but 4.4.19 retains it, most other shells too
				 */
				ਜਾਰੀ; /* get next char */
			}
			/* Example: echo Hello \2>file
			 * we need to know that word 2 is quoted
			 */
			ctx.ਸ਼ਬਦ.has_quoted_part = 1;
			nommu_addchr(&ctx.as_string, ch);
			o_addchr(&ctx.ਸ਼ਬਦ, ch);
			ਜਾਰੀ; /* get next char */
		}
		nommu_addchr(&ctx.as_string, ch);
		ਜੇ (ch == '\'') {
			ctx.ਸ਼ਬਦ.has_quoted_part = 1;
			ਅਗਲਾ = i_getch(ਨਿਵੇਸ਼);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
			ਜੇ (ਅਗਲਾ == '\'' && !ctx.pending_redirect)
				ਜਾਓ ਪਾਓ_ਖਾਲੀ_ਥਾਵਾਂ_quoted_str_marker;
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */

			ch = ਅਗਲਾ;
			ਜਦੋਂ (1) {
				ਜੇ (ch == EOF) {
					syntax_error_unterm_ch('\'');
					ਜਾਓ parse_error_exitcode1;
				}
				nommu_addchr(&ctx.as_string, ch);
				ਜੇ (ch == '\'')
					ਰੋਕੋ;
				ਜੇ (ch == SPECIAL_VAR_ਚਿੰਨ੍ਹ) {
					/* Convert raw ^C to corresponding special variable reference */
					o_addchr(&ctx.ਸ਼ਬਦ, SPECIAL_VAR_ਚਿੰਨ੍ਹ);
					o_addchr(&ctx.ਸ਼ਬਦ, SPECIAL_VAR_QUOTED_SVS);
				}
				o_addqchr(&ctx.ਸ਼ਬਦ, ch);
				ch = i_getch(ਨਿਵੇਸ਼);
			}
			ਜਾਰੀ; /* get next char */
		}

		ਅਗਲਾ = '\0';
		ਜੇ (ch != '\n')
			ਅਗਲਾ = i_peek_and_eat_bkslash_nl(ਨਿਵੇਸ਼);

		is_special = "{}<>&|();#" /* special outside of "str" */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
				"$\"" IF_HUSH_TICK("`") /* always special */
#ਨਹੀਂਤਾਂ /* __U_BOOT__ */
				"$\""
#ਜੇ_ਅੰਤ /* __U_BOOT__ */
				SPECIAL_VAR_ਚਿੰਨ੍ਹ_STR;
#ਜੇ defined(CMD_TEST2_SINGLEWORD_NOGLOB)
		ਜੇ (ctx.command->cmd_type == CMD_TEST2_SINGLEWORD_NOGLOB) {
			/* In [[ ]], {}<>&|() are not special */
			is_special += 8;
		} ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
		/* Are { and } special here? */
		ਜੇ (ctx.command->argv /* word [word]{... - non-special */
		 || ctx.ਸ਼ਬਦ.ਲੰਮਾਈ       /* word{... - non-special */
		 || ctx.ਸ਼ਬਦ.has_quoted_part     /* ""{... - non-special */
		 || (ਅਗਲਾ != ';'             /* }; - special */
		    && ਅਗਲਾ != ')'           /* }) - special */
		    && ਅਗਲਾ != '('           /* {( - special */
		    && ਅਗਲਾ != '&'           /* }& and }&& ... - special */
		    && ਅਗਲਾ != '|'           /* }|| ... - special */
		    && !strchr(defifs, ਅਗਲਾ) /* {word - non-special */
		    )
		) {
			/* They are not special, skip "{}" */
			is_special += 2;
		}
		is_special = strchr(is_special, ch);
		is_blank = strchr(defifs, ch);

		ਜੇ (!is_special && !is_blank) { /* ordinary char */
 ordinary_char:
			o_ਜੋੜੋ_Qchr(&ctx.ਸ਼ਬਦ, ch);
			ਜੇ ((ctx.is_assignment == MAYBE_ASSIGNMENT
			    || ctx.is_assignment == ਸ਼ਬਦ_IS_KEYWORD)
			 && ch == '='
			 && endofname(ctx.ਸ਼ਬਦ.ਅੰਕੜੇ)[0] == '='
			) {
				ctx.is_assignment = DEFINITELY_ASSIGNMENT;
				debug_printf_parse("ctx.is_assignment='%s'\n", assignment_flag[ctx.is_assignment]);
			}
			ਜਾਰੀ;
		}

		ਜੇ (is_blank) {
#ਜੇ ENABLE_HUSH_LINENO_VAR
/* Case:
 * "while ...; do<whitespace><newline>
 *	cmd ..."
 * would think that "cmd" starts in <whitespace> -
 * i.e., at the previous line.
 * We need to skip all whitespace before newlines.
 */
			ਜਦੋਂ (ch != '\n') {
				ਅਗਲਾ = i_peek(ਨਿਵੇਸ਼);
				ਜੇ (ਅਗਲਾ != ' ' && ਅਗਲਾ != '\t' && ਅਗਲਾ != '\n')
					ਰੋਕੋ; /* next char is not ws */
				ch = i_getch(ਨਿਵੇਸ਼);
			}
			/* ch == last eaten whitespace char */
#ਜੇ_ਅੰਤ
			ਜੇ (ਪੂਰਾ_ਸ਼ਬਦ(&ctx)) {
				ਜਾਓ parse_error_exitcode1;
			}
			ਜੇ (ch == '\n') {
				/* Is this a case when newline is simply ignored?
				 * Some examples:
				 * "cmd | <newline> cmd ..."
				 * "case ... in <newline> word) ..."
				 */
				ਜੇ (IS_NULL_CMD(ctx.command)
				 && ctx.ਸ਼ਬਦ.ਲੰਮਾਈ == 0
				 && !ctx.ਸ਼ਬਦ.has_quoted_part
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
					ਬਣਤਰ pipe *pi = ctx.list_ਸਿਰ;
					ਜੇ (pi->ਅੰਕ_cmds != 0       /* check #1 */
					 && pi->followup != PIPE_BG /* check #2 */
					) {
						ਜਾਰੀ;
					}
				}
				/* Treat newline as a command separator. */
				ਪੂਰਾ_pipe(&ctx, PIPE_SEQ);
				debug_printf_heredoc("heredoc_cnt:%d\n", heredoc_cnt);
				ਜੇ (heredoc_cnt) {
					heredoc_cnt = fetch_heredocs(&ctx.as_string, ctx.list_ਸਿਰ, heredoc_cnt, ਨਿਵੇਸ਼);
					ਜੇ (heredoc_cnt != 0)
						ਜਾਓ parse_error_exitcode1;
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
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
		ਜੇ (ch == '}') {
#ਨਹੀਂਤਾਂ /* __U_BOOT__ */
		ਜੇ (ch == '}' || ch == ')') {
#ਜੇ_ਅੰਤ /* __U_BOOT__ */
			ਜੇ (ctx.ਸ਼ਬਦ.ਲੰਮਾਈ != 0 /* word} */
			 || ctx.ਸ਼ਬਦ.has_quoted_part    /* ""} */
			) {
				ਜਾਓ ordinary_char;
			}
			ਜੇ (!IS_NULL_CMD(ctx.command)) { /* cmd } */
				/* Generally, there should be semicolon: "cmd; }"
				 * However, bash allows to omit it if "cmd" is
				 * a group. Examples:
				 * { { echo 1; } }
				 * {(echo 1)}
				 * { echo 0 >&2 | { echo 1; } }
				 * { while false; do :; done }
				 * { case a in b) ;; esac }
				 */
				ਜੇ (ctx.command->ਸਮੂਹ)
					ਜਾਓ ਪਦ_ਸਮੂਹ;
				ਜਾਓ ordinary_char;
			}
			ਜੇ (!IS_NULL_PIPE(ctx.pipe)) /* cmd | } */
				/* Can't be an end of {cmd}, skip the check */
				ਜਾਓ skip_ਸਿਰਾ_trigger;
			/* else: } does terminate a group */
		}
 ਪਦ_ਸਮੂਹ:
		ਜੇ (ਸਿਰਾ_trigger && ਸਿਰਾ_trigger == ch
		 && (ch != ';' || heredoc_cnt == 0)
#ਜੇ ENABLE_HUSH_CASE
		 && (ch != ')'
		    || ctx.ctx_res_w != RES_ਮੇਲ
		    || (!ctx.ਸ਼ਬਦ.has_quoted_part && strcmp(ctx.ਸ਼ਬਦ.ਅੰਕੜੇ, "esac") == 0)
		    )
#ਜੇ_ਅੰਤ
		) {
			ਜੇ (ਪੂਰਾ_ਸ਼ਬਦ(&ctx)) {
				ਜਾਓ parse_error_exitcode1;
			}
			ਪੂਰਾ_pipe(&ctx, PIPE_SEQ);
			ctx.is_assignment = MAYBE_ASSIGNMENT;
			debug_printf_parse("ctx.is_assignment='%s'\n", assignment_flag[ctx.is_assignment]);
			/* Do we sit outside of any if's, loops or case's? */
			ਜੇ (!HAS_KEYWORDS
			IF_HAS_KEYWORDS(|| (ctx.ctx_res_w == RES_NONE && ctx.old_flag == 0))
			) {
				o_free_and_ਸੈੱਟ_NULL(&ctx.ਸ਼ਬਦ);
#ਜੇ !BB_MMU
				debug_printf_parse("as_string2 '%s'\n", ctx.as_string.ਅੰਕੜੇ);
				ਜੇ (pstring)
					*pstring = ctx.as_string.ਅੰਕੜੇ;
				ਨਹੀਂਤਾਂ
					o_free(&ctx.as_string);
#ਜੇ_ਅੰਤ
				ਜੇ (ch != ';' && IS_NULL_PIPE(ctx.list_ਸਿਰ)) {
					/* Example: bare "{ }", "()" */
					G.ਆਖਰੀ_exitcode = 2; /* bash compat */
					syntax_error_unexpected_ch(ch);
					ਜਾਓ parse_error;
				}
				ਜੇ (heredoc_cnt_ptr)
					*heredoc_cnt_ptr = heredoc_cnt;
				debug_printf_heredoc("parse_stream return heredoc_cnt:%d\n", heredoc_cnt);
				debug_printf_parse("parse_stream return %p: "
						"end_trigger char found\n",
						ctx.list_ਸਿਰ);
				debug_leave();
				ਵਾਪਸ ctx.list_ਸਿਰ;
			}
		}

		ਜੇ (is_blank)
			ਜਾਰੀ;

		/* Catch <, > before deciding whether this word is
		 * an assignment. a=1 2>z b=2: b=2 is still assignment */
		ਚੋਣ (ch) {
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
		ਹਾਲਤ '>':
			redir_fd = redirect_opt_ਅੰਕ(&ctx.ਸ਼ਬਦ);
			ਜੇ (ਪੂਰਾ_ਸ਼ਬਦ(&ctx)) {
				ਜਾਓ parse_error_exitcode1;
			}
			redir_style = REDIRECT_OVERWRITE;
			ਜੇ (ਅਗਲਾ == '>') {
				redir_style = REDIRECT_APPEND;
				ch = i_getch(ਨਿਵੇਸ਼);
				nommu_addchr(&ctx.as_string, ch);
			}
#ਜੇ 0
			ਨਹੀਂਤਾਂ ਜੇ (ਅਗਲਾ == '(') {
				syntax_error(">(process) not supported");
				ਜਾਓ parse_error_exitcode1;
			}
#ਜੇ_ਅੰਤ
			ਜੇ (parse_redirect(&ctx, redir_fd, redir_style, ਨਿਵੇਸ਼))
				ਜਾਓ parse_error_exitcode1;
			ਜਾਰੀ; /* get next char */
		ਹਾਲਤ '<':
			redir_fd = redirect_opt_ਅੰਕ(&ctx.ਸ਼ਬਦ);
			ਜੇ (ਪੂਰਾ_ਸ਼ਬਦ(&ctx)) {
				ਜਾਓ parse_error_exitcode1;
			}
			redir_style = REDIRECT_ਨਿਵੇਸ਼;
			ਜੇ (ਅਗਲਾ == '<') {
				redir_style = REDIRECT_HEREDOC;
				heredoc_cnt++;
				debug_printf_heredoc("++heredoc_cnt=%d\n", heredoc_cnt);
				ch = i_getch(ਨਿਵੇਸ਼);
				nommu_addchr(&ctx.as_string, ch);
			} ਨਹੀਂਤਾਂ ਜੇ (ਅਗਲਾ == '>') {
				redir_style = REDIRECT_IO;
				ch = i_getch(ਨਿਵੇਸ਼);
				nommu_addchr(&ctx.as_string, ch);
			}
#ਜੇ 0
			ਨਹੀਂਤਾਂ ਜੇ (ਅਗਲਾ == '(') {
				syntax_error("<(process) not supported");
				ਜਾਓ parse_error_exitcode1;
			}
#ਜੇ_ਅੰਤ
			ਜੇ (parse_redirect(&ctx, redir_fd, redir_style, ਨਿਵੇਸ਼))
				ਜਾਓ parse_error_exitcode1;
			ਜਾਰੀ; /* get next char */
#ਨਹੀਂਤਾਂ /* __U_BOOT__ */
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
			ਹਾਲਤ '>':
				fallthrough;
			ਹਾਲਤ '<':
				o_ਜੋੜੋ_Qchr(&ctx.ਸ਼ਬਦ, ch);
				ਜਾਰੀ;
#ਜੇ_ਅੰਤ /* __U_BOOT__ */
		ਹਾਲਤ '#':
			ਜੇ (ctx.ਸ਼ਬਦ.ਲੰਮਾਈ == 0 && !ctx.ਸ਼ਬਦ.has_quoted_part) {
				/* skip "#comment" */
				/* note: we do not add it to &ctx.as_string */
/* TODO: in bash:
 * comment inside $() goes to the next \n, even inside quoted string (!):
 * cmd "$(cmd2 #comment)" - syntax error
 * cmd "`cmd2 #comment`" - ok
 * We accept both (comment ends where command subst ends, in both cases).
 */
				ਜਦੋਂ (1) {
					ch = i_peek(ਨਿਵੇਸ਼);
					ਜੇ (ch == '\n') {
						nommu_addchr(&ctx.as_string, '\n');
						ਰੋਕੋ;
					}
					ch = i_getch(ਨਿਵੇਸ਼);
					ਜੇ (ch == EOF)
						ਰੋਕੋ;
				}
				ਜਾਰੀ; /* get next char */
			}
			ਰੋਕੋ;
		}
 skip_ਸਿਰਾ_trigger:

		ਜੇ (ctx.is_assignment == MAYBE_ASSIGNMENT
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
		 /* check that we are not in word in "a=1 2>word b=1": */
		 && !ctx.pending_redirect
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */
		) {
			/* ch is a special char and thus this word
			 * cannot be an assignment */
			ctx.is_assignment = NOT_ASSIGNMENT;
			debug_printf_parse("ctx.is_assignment='%s'\n", assignment_flag[ctx.is_assignment]);
		}

		/* Note: nommu_addchr(&ctx.as_string, ch) is already done */

		ਚੋਣ (ch) {
		case_SPECIAL_VAR_ਚਿੰਨ੍ਹ:
		ਹਾਲਤ SPECIAL_VAR_ਚਿੰਨ੍ਹ:
			/* Convert raw ^C to corresponding special variable reference */
			o_addchr(&ctx.ਸ਼ਬਦ, SPECIAL_VAR_ਚਿੰਨ੍ਹ);
			o_addchr(&ctx.ਸ਼ਬਦ, SPECIAL_VAR_QUOTED_SVS);
			/* fall through */
		ਹਾਲਤ '#':
			/* non-comment #: "echo a#b" etc */
			o_addchr(&ctx.ਸ਼ਬਦ, ch);
			ਜਾਰੀ; /* get next char */
		ਹਾਲਤ '$':
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
			ਜੇ (parse_dollar_squote(&ctx.as_string, &ctx.ਸ਼ਬਦ, ਨਿਵੇਸ਼))
				ਜਾਰੀ; /* get next char */
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */
			ਜੇ (!parse_dollar(&ctx.as_string, &ctx.ਸ਼ਬਦ, ਨਿਵੇਸ਼, /*quote_mask:*/ 0)) {
				debug_printf_parse("parse_stream parse error: "
					"parse_dollar returned 0 (error)\n");
				ਜਾਓ parse_error_exitcode1;
			}
			ਜਾਰੀ; /* get next char */
		ਹਾਲਤ '"':
			ctx.ਸ਼ਬਦ.has_quoted_part = 1;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
			ਜੇ (ਅਗਲਾ == '"' && !ctx.pending_redirect) {
#ਨਹੀਂਤਾਂ /* __U_BOOT__ */
			ਜੇ (ਅਗਲਾ == '"') {
#ਜੇ_ਅੰਤ /* __U_BOOT__ */
				i_getch(ਨਿਵੇਸ਼); /* eat second " */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
 ਪਾਓ_ਖਾਲੀ_ਥਾਵਾਂ_quoted_str_marker:
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */
				nommu_addchr(&ctx.as_string, ਅਗਲਾ);
				o_addchr(&ctx.ਸ਼ਬਦ, SPECIAL_VAR_ਚਿੰਨ੍ਹ);
				o_addchr(&ctx.ਸ਼ਬਦ, SPECIAL_VAR_ਚਿੰਨ੍ਹ);
				ਜਾਰੀ; /* get next char */
			}
			ਜੇ (ctx.is_assignment == NOT_ASSIGNMENT)
				ctx.ਸ਼ਬਦ.o_expflags |= EXP_FLAG_ESC_GLOB_CHARS;
			ਜੇ (!encode_string(&ctx.as_string, &ctx.ਸ਼ਬਦ, ਨਿਵੇਸ਼, '"'))
				ਜਾਓ parse_error_exitcode1;
			ctx.ਸ਼ਬਦ.o_expflags &= ~EXP_FLAG_ESC_GLOB_CHARS;
			ਜਾਰੀ; /* get next char */
#ਜੇ ENABLE_HUSH_TICK
		ਹਾਲਤ '`': {
			USE_FOR_NOMMU(ਬਿਨਾਨਿਸ਼ਾਨੀ pos;)

			o_addchr(&ctx.ਸ਼ਬਦ, SPECIAL_VAR_ਚਿੰਨ੍ਹ);
			o_addchr(&ctx.ਸ਼ਬਦ, '`');
			USE_FOR_NOMMU(pos = ctx.ਸ਼ਬਦ.ਲੰਮਾਈ;)
			ਜੇ (!ਜੋੜੋ_till_backquote(&ctx.ਸ਼ਬਦ, ਨਿਵੇਸ਼, /*in_dquote:*/ 0))
				ਜਾਓ parse_error_exitcode1;
# ਜੇ !BB_MMU
			o_addstr(&ctx.as_string, ctx.ਸ਼ਬਦ.ਅੰਕੜੇ + pos);
			o_addchr(&ctx.as_string, '`');
# endif
			o_addchr(&ctx.ਸ਼ਬਦ, SPECIAL_VAR_ਚਿੰਨ੍ਹ);
			//debug_printf_subst("SUBST RES3 '%s'\n", ctx.word.data + pos);
			ਜਾਰੀ; /* get next char */
		}
#ਜੇ_ਅੰਤ
		ਹਾਲਤ ';':
#ਜੇ ENABLE_HUSH_CASE
 case_semi:
#ਜੇ_ਅੰਤ
			ਜੇ (ਪੂਰਾ_ਸ਼ਬਦ(&ctx)) {
				ਜਾਓ parse_error_exitcode1;
			}
			ਪੂਰਾ_pipe(&ctx, PIPE_SEQ);
#ਜੇ ENABLE_HUSH_CASE
			/* Eat multiple semicolons, detect
			 * whether it means something special */
			ਜਦੋਂ (1) {
				ch = i_peek_and_eat_bkslash_nl(ਨਿਵੇਸ਼);
				ਜੇ (ch != ';')
					ਰੋਕੋ;
				ch = i_getch(ਨਿਵੇਸ਼);
				nommu_addchr(&ctx.as_string, ch);
				ਜੇ (ctx.ctx_res_w == RES_CASE_BODY) {
					ctx.ctx_dsemicolon = 1;
					ctx.ctx_res_w = RES_ਮੇਲ;
					ਰੋਕੋ;
				}
			}
#ਜੇ_ਅੰਤ
 new_cmd:
			/* We just finished a cmd. New one may start
			 * with an assignment */
			ctx.is_assignment = MAYBE_ASSIGNMENT;
			debug_printf_parse("ctx.is_assignment='%s'\n", assignment_flag[ctx.is_assignment]);
			ਜਾਰੀ; /* get next char */
		ਹਾਲਤ '&':
			ਜੇ (ਪੂਰਾ_ਸ਼ਬਦ(&ctx)) {
				ਜਾਓ parse_error_exitcode1;
			}
			ਜੇ (ਅਗਲਾ == '&') {
				ch = i_getch(ਨਿਵੇਸ਼);
				nommu_addchr(&ctx.as_string, ch);
				ਪੂਰਾ_pipe(&ctx, PIPE_AND);
			} ਨਹੀਂਤਾਂ {
				ਪੂਰਾ_pipe(&ctx, PIPE_BG);
			}
			ਜਾਓ new_cmd;
		ਹਾਲਤ '|':
			ਜੇ (ਪੂਰਾ_ਸ਼ਬਦ(&ctx)) {
				ਜਾਓ parse_error_exitcode1;
			}
#ਜੇ ENABLE_HUSH_CASE
			ਜੇ (ctx.ctx_res_w == RES_ਮੇਲ)
				ਰੋਕੋ; /* we are in case's "word | word)" */
#ਜੇ_ਅੰਤ
			ਜੇ (ਅਗਲਾ == '|') { /* || */
				ch = i_getch(ਨਿਵੇਸ਼);
				nommu_addchr(&ctx.as_string, ch);
				ਪੂਰਾ_pipe(&ctx, PIPE_OR);
			} ਨਹੀਂਤਾਂ {
				/* we could pick up a file descriptor choice here
				 * with redirect_opt_num(), but bash doesn't do it.
				 * "echo foo 2| cat" yields "foo 2". */
				ਪੂਰਾ_command(&ctx);
			}
			ਜਾਓ new_cmd;
		ਹਾਲਤ '(':
#ਜੇ ENABLE_HUSH_CASE
			/* "case... in [(]word)..." - skip '(' */
			ਜੇ (ctx.ctx_res_w == RES_ਮੇਲ
			 && ctx.command->argv == ਨਿਰਾ /* not (word|(... */
			 && ctx.ਸ਼ਬਦ.ਲੰਮਾਈ == 0 /* not word(... */
			 && ctx.ਸ਼ਬਦ.has_quoted_part == 0 /* not ""(... */
			) {
				ਜਾਰੀ; /* get next char */
			}
#ਜੇ_ਅੰਤ
			/* fall through */
		ਹਾਲਤ '{': {
			ਪੂਰਨ n = parse_ਸਮੂਹ(&ctx, ਨਿਵੇਸ਼, ch);
			ਜੇ (n < 0) {
				ਜਾਓ parse_error_exitcode1;
			}
			debug_printf_heredoc("parse_group done, needs heredocs:%d\n", n);
			heredoc_cnt += n;
			ਜਾਓ new_cmd;
		}
		ਹਾਲਤ ')':
#ਜੇ ENABLE_HUSH_CASE
			ਜੇ (ctx.ctx_res_w == RES_ਮੇਲ)
				ਜਾਓ case_semi;
#ਜੇ_ਅੰਤ
		ਹਾਲਤ '}':
			/* proper use of this character is caught by end_trigger:
			 * if we see {, we call parse_group(..., end_trigger='}')
			 * and it will match } earlier (not here). */
			G.ਆਖਰੀ_exitcode = 2;
			syntax_error_unexpected_ch(ch);
			ਜਾਓ parse_error;
		ਮੂਲ:
			ਜੇ (HUSH_DEBUG)
				bb_error_ਸੁਨੇਹਾ_and_die("BUG: unexpected %c", ch);
		}
	} /* while (1) */

 parse_error_exitcode1:
	G.ਆਖਰੀ_exitcode = 1;
 parse_error:
	{
		ਬਣਤਰ parse_context *pctx;
		IF_HAS_KEYWORDS(ਬਣਤਰ parse_context *p2;)

		/* Clean up allocated tree.
		 * Sample for finding leaks on syntax error recovery path.
		 * Run it from interactive shell, watch pmap `pidof hush`.
		 * while if false; then false; fi; do break; fi
		 * Samples to catch leaks at execution:
		 * while if (true | { true;}); then echo ok; fi; do break; done
		 * while if (true | { true;}); then echo ok; fi; do (if echo ok; break; then :; fi) | cat; break; done
		 */
		pctx = &ctx;
		ਕਰੋ {
			/* Update pipe/command counts,
			 * otherwise freeing may miss some */
			ਪੂਰਾ_pipe(pctx, PIPE_SEQ);
			debug_printf_clean("freeing list %p from ctx %p\n",
					pctx->list_ਸਿਰ, pctx);
			debug_print_ਰੁੱਖ(pctx->list_ਸਿਰ, 0);
			free_pipe_list(pctx->list_ਸਿਰ);
			debug_printf_clean("freed list %p\n", pctx->list_ਸਿਰ);
#ਜੇ !BB_MMU
			o_free(&pctx->as_string);
#ਜੇ_ਅੰਤ
			IF_HAS_KEYWORDS(p2 = pctx->ਢੇਰ;)
			ਜੇ (pctx != &ctx) {
				ਮੁਕਤ(pctx);
			}
			IF_HAS_KEYWORDS(pctx = p2;)
		} ਜਦੋਂ (HAS_KEYWORDS && pctx);

		o_free(&ctx.ਸ਼ਬਦ);
#ਜੇ !BB_MMU
		ਜੇ (pstring)
			*pstring = ਨਿਰਾ;
#ਜੇ_ਅੰਤ
		debug_leave();
		ਵਾਪਸ ERR_PTR;
	}
}

/*** Execution routines ***/

/* Expansion can recurse, need forward decls: */
#ਜੇ !BASH_ਨਮੂਨਾ_SUBST && !ENABLE_HUSH_CASE
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ expand_string_ਮੰਜ਼ਿਲ_string(str, EXP_flags, do_unbackslash) \
	expand_string_ਮੰਜ਼ਿਲ_string(str)
#ਜੇ_ਅੰਤ
ਸਥਿਰ ਅੱਖਰ *expand_string_ਮੰਜ਼ਿਲ_string(ਅਚਲ ਅੱਖਰ *str, ਪੂਰਨ EXP_flags, ਪੂਰਨ do_unbackslash);
#ਜੇ ENABLE_HUSH_TICK
ਸਥਿਰ ਪੂਰਨ process_command_subs(o_string *dest, ਅਚਲ ਅੱਖਰ *s);
#ਜੇ_ਅੰਤ
ਸਥਿਰ ਪੂਰਨ expand_vars_ਮੰਜ਼ਿਲ_list(o_string *output, ਪੂਰਨ n, ਅੱਖਰ *arg);

/* expand_strvec_to_strvec() takes a list of strings, expands
 * all variable references within and returns a pointer to
 * a list of expanded strings, possibly with larger number
 * of strings. (Think VAR="a b"; echo $VAR).
 * This new list is allocated as a single malloc block.
 * NULL-terminated list of char* pointers is at the beginning of it,
 * followed by strings themselves.
 * Caller can deallocate entire list by single free(list). */

/* A horde of its helpers come first: */

ਸਥਿਰ ਖਾਲੀ o_addblock_duplicate_backslash(o_string *o, ਅਚਲ ਅੱਖਰ *str, ਪੂਰਨ len)
{
	ਜਦੋਂ (--len >= 0) {
		ਅੱਖਰ c = *str++;

#ਜੇ ENABLE_HUSH_BRACE_EXPANSION
		ਜੇ (c == '{' || c == '}') {
			/* { -> \{, } -> \} */
			o_addchr(o, '\\');
			/* And now we want to add { or } and continue:
			 *  o_addchr(o, c);
			 *  continue;
			 * luckily, just falling through achieves this.
			 */
		}
#ਜੇ_ਅੰਤ
		o_addchr(o, c);
		ਜੇ (c == '\\') {
			/* \z -> \\\z; \<eol> -> \\<eol> */
			o_addchr(o, '\\');
			ਜੇ (len) {
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
ਸਥਿਰ ਪੂਰਨ expand_on_ifs(o_string *output, ਪੂਰਨ n, ਅਚਲ ਅੱਖਰ *str)
{
	ਪੂਰਨ ਆਖਰੀ_is_ifs = 0;

	ਜਦੋਂ (1) {
		ਪੂਰਨ ਸ਼ਬਦ_len;

		ਜੇ (!*str)  /* EOL - do not finalize word */
			ਰੋਕੋ;
		ਸ਼ਬਦ_len = strcspn(str, G.ifs);
		ਜੇ (ਸ਼ਬਦ_len) {
			/* We have WORD_LEN leading non-IFS chars */
			ਜੇ (!(output->o_expflags & EXP_FLAG_GLOB)) {
				o_addblock(output, str, ਸ਼ਬਦ_len);
			} ਨਹੀਂਤਾਂ {
				/* Protect backslashes against globbing up :)
				 * Example: "v='\*'; echo b$v" prints "b\*"
				 * (and does not try to glob on "*")
				 */
				o_addblock_duplicate_backslash(output, str, ਸ਼ਬਦ_len);
				/*/ Why can't we do it easier? */
				/*o_addblock(output, str, word_len); - WRONG: "v='\*'; echo Z$v" prints "Z*" instead of "Z\*" */
				/*o_addqblock(output, str, word_len); - WRONG: "v='*'; echo Z$v" prints "Z*" instead of Z* files */
			}
			ਆਖਰੀ_is_ifs = 0;
			str += ਸ਼ਬਦ_len;
			ਜੇ (!*str)  /* EOL - do not finalize word */
				ਰੋਕੋ;
		}

		/* We know str here points to at least one IFS char */
		ਆਖਰੀ_is_ifs = 1;
		str += strspn(str, G.ifs_whitespace); /* skip IFS whitespace chars */
		ਜੇ (!*str)  /* EOL - do not finalize word */
			ਰੋਕੋ;

		ਜੇ (G.ifs_whitespace != G.ifs /* usually false ($IFS is usually all whitespace), */
		 && strchr(G.ifs, *str)       /* the second check would fail */
		) {
			/* This is a non-whitespace $IFS char */
			/* Skip it and IFS whitespace chars, start new word */
			str++;
			str += strspn(str, G.ifs_whitespace);
			ਜਾਓ new_ਸ਼ਬਦ;
		}

		/* Start new word... but not always! */
		/* Case "v=' a'; echo ''$v": we do need to finalize empty word: */
		ਜੇ (output->has_quoted_part
		/*
		 * Case "v=' a'; echo $v":
		 * here nothing precedes the space in $v expansion,
		 * therefore we should not finish the word
		 * (IOW: if there *is* word to finalize, only then do it):
		 * It's okay if this accesses the byte before first argv[]:
		 * past call to o_save_ptr() cleared it to zero byte
		 * (grep for -prev-ifs-check-).
		 */
		 || output->ਅੰਕੜੇ[output->ਲੰਮਾਈ - 1]
		) {
 new_ਸ਼ਬਦ:
			o_addchr(output, '\0');
			debug_print_list("expand_on_ifs", output, n);
			n = o_save_ptr(output, n);
		}
	}

	output->ended_ਵਿੱਚ_ifs = ਆਖਰੀ_is_ifs;
	debug_print_list("expand_on_ifs[1]", output, n);
	ਵਾਪਸ n;
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
/* Helper to expand $((...)) and heredoc body. These act as if
 * they are in double quotes, with the exception that they are not :).
 * Just the rules are similar: "expand only $var and `cmd`"
 *
 * Returns malloced string.
 * As an optimization, we return NULL if expansion is not needed.
 */
ਸਥਿਰ ਅੱਖਰ *encode_then_expand_string(ਅਚਲ ਅੱਖਰ *str)
{
	ਅੱਖਰ *exp_str;
	ਬਣਤਰ ਵਿੱਚ_str ਨਿਵੇਸ਼;
	o_string dest = NULL_O_STRING;
	ਅਚਲ ਅੱਖਰ *cp;

	cp = str;
	ਲਈ (;;) {
		ਜੇ (!*cp) ਵਾਪਸ ਨਿਰਾ; /* string has no special chars */
		ਜੇ (*cp == '$') ਰੋਕੋ;
		ਜੇ (*cp == '\\') ਰੋਕੋ;
#ਜੇ ENABLE_HUSH_TICK
		ਜੇ (*cp == '`') ਰੋਕੋ;
#ਜੇ_ਅੰਤ
		cp++;
	}

	/* We need to expand. Example:
	 * echo $(($a + `echo 1`)) $((1 + $((2)) ))
	 */
	ਤਿਆਰੀ_string_ਵਿੱਚ_str(&ਨਿਵੇਸ਼, str);
	encode_string(ਨਿਰਾ, &dest, &ਨਿਵੇਸ਼, EOF);
//TODO: error check (encode_string returns 0 on error)?
	//bb_error_msg("'%s' -> '%s'", str, dest.data);
	exp_str = expand_string_ਮੰਜ਼ਿਲ_string(dest.ਅੰਕੜੇ,
			EXP_FLAG_ESC_GLOB_CHARS,
			/*unbackslash:*/ 1
	);
	//bb_error_msg("'%s' -> '%s'", dest.data, exp_str);
	o_free(&dest);
	ਵਾਪਸ exp_str;
}

ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਪਹਿਲਾ_special_char_ਵਿੱਚ_vararg(ਅਚਲ ਅੱਖਰ *cp)
{
	ਲਈ (;;) {
		ਜੇ (!*cp) ਵਾਪਸ ਨਿਰਾ; /* string has no special chars */
		ਜੇ (*cp == '$') ਵਾਪਸ cp;
		ਜੇ (*cp == '\\') ਵਾਪਸ cp;
		ਜੇ (*cp == '\'') ਵਾਪਸ cp;
		ਜੇ (*cp == '"') ਵਾਪਸ cp;
#ਜੇ ENABLE_HUSH_TICK
		ਜੇ (*cp == '`') ਵਾਪਸ cp;
#ਜੇ_ਅੰਤ
		/* dquoted "${x:+ARG}" should not glob, therefore
		 * '*' et al require some non-literal processing: */
		ਜੇ (*cp == '*') ਵਾਪਸ cp;
		ਜੇ (*cp == '?') ਵਾਪਸ cp;
		ਜੇ (*cp == '[') ਵਾਪਸ cp;
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
#ਜੇ !BASH_ਨਮੂਨਾ_SUBST
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ encode_then_expand_vararg(str, handle_squotes, do_unbackslash) \
	encode_then_expand_vararg(str, handle_squotes)
#ਜੇ_ਅੰਤ
ਸਥਿਰ ਅੱਖਰ *encode_then_expand_vararg(ਅਚਲ ਅੱਖਰ *str, ਪੂਰਨ handle_squotes, ਪੂਰਨ do_unbackslash)
{
#ਜੇ !BASH_ਨਮੂਨਾ_SUBST && ENABLE_HUSH_CASE
	ਅਚਲ ਪੂਰਨ do_unbackslash = 0;
#ਜੇ_ਅੰਤ
	ਅੱਖਰ *exp_str;
	ਬਣਤਰ ਵਿੱਚ_str ਨਿਵੇਸ਼;
	o_string dest = NULL_O_STRING;

	ਜੇ (!ਪਹਿਲਾ_special_char_ਵਿੱਚ_vararg(str)) {
		/* string has no special chars */
		ਵਾਪਸ ਨਿਰਾ;
	}

	ਤਿਆਰੀ_string_ਵਿੱਚ_str(&ਨਿਵੇਸ਼, str);
	dest.ਅੰਕੜੇ = xzalloc(1); /* start as "", not as NULL */
	exp_str = ਨਿਰਾ;

	ਲਈ (;;) {
		ਪੂਰਨ ch;

		ch = i_getch(&ਨਿਵੇਸ਼);
		debug_printf_parse("%s: ch=%c (%d) escape=%d\n",
				__func__, ch, ch, !!dest.o_expflags);

		ਜੇ (!dest.o_expflags) {
			ਜੇ (ch == EOF)
				ਰੋਕੋ;
			ਜੇ (handle_squotes && ch == '\'') {
				ਜੇ (!ਜੋੜੋ_till_single_quote_dquoted(&dest, &ਨਿਵੇਸ਼))
					ਜਾਓ ret; /* error */
				ਜਾਰੀ;
			}
		}
		ਜੇ (ch == EOF) {
			syntax_error_unterm_ch('"');
			ਜਾਓ ret; /* error */
		}
		ਜੇ (ch == '"') {
			dest.o_expflags ^= EXP_FLAG_ESC_GLOB_CHARS;
			ਜਾਰੀ;
		}
		ਜੇ (ch == '\\') {
			ch = i_getch(&ਨਿਵੇਸ਼);
			ਜੇ (ch == EOF) {
//example? error message?	syntax_error_unterm_ch('"');
				debug_printf_parse("%s: error: \\<eof>\n", __func__);
				ਜਾਓ ret;
			}
			o_addqchr(&dest, ch);
			ਜਾਰੀ;
		}
		ਜੇ (ch == '$') {
			ਜੇ (parse_dollar_squote(ਨਿਰਾ, &dest, &ਨਿਵੇਸ਼))
				ਜਾਰੀ;
			ਜੇ (!parse_dollar(ਨਿਰਾ, &dest, &ਨਿਵੇਸ਼, /*quote_mask:*/ 0x80)) {
				debug_printf_parse("%s: error: parse_dollar returned 0 (error)\n", __func__);
				ਜਾਓ ret;
			}
			ਜਾਰੀ;
		}
#ਜੇ ENABLE_HUSH_TICK
		ਜੇ (ch == '`') {
			//unsigned pos = dest->length;
			o_addchr(&dest, SPECIAL_VAR_ਚਿੰਨ੍ਹ);
			o_addchr(&dest, 0x80 | '`');
			ਜੇ (!ਜੋੜੋ_till_backquote(&dest, &ਨਿਵੇਸ਼,
					/*in_dquote:*/ dest.o_expflags /* nonzero if EXP_FLAG_ESC_GLOB_CHARS set */
				)
			) {
				ਜਾਓ ret; /* error */
			}
			o_addchr(&dest, SPECIAL_VAR_ਚਿੰਨ੍ਹ);
			//debug_printf_subst("SUBST RES3 '%s'\n", dest->data + pos);
			ਜਾਰੀ;
		}
#ਜੇ_ਅੰਤ
		o_ਜੋੜੋ_Qchr(&dest, ch);
	} /* for (;;) */

	debug_printf_parse("encode: '%s' -> '%s'\n", str, dest.ਅੰਕੜੇ);
	exp_str = expand_string_ਮੰਜ਼ਿਲ_string(dest.ਅੰਕੜੇ,
			do_unbackslash ? EXP_FLAG_ESC_GLOB_CHARS : 0,
			do_unbackslash
	);
 ret:
	debug_printf_parse("expand: '%s' -> '%s'\n", dest.ਅੰਕੜੇ, exp_str);
	o_free(&dest);
	ਵਾਪਸ exp_str;
}

/* Expanding ARG in ${var+ARG}, ${var-ARG}
 */
ਸਥਿਰ NOINLINE ਪੂਰਨ encode_then_append_var_plusminus(o_string *output, ਪੂਰਨ n,
		ਅੱਖਰ *str, ਪੂਰਨ dquoted)
{
	ਬਣਤਰ ਵਿੱਚ_str ਨਿਵੇਸ਼;
	o_string dest = NULL_O_STRING;

	ਜੇ (!ਪਹਿਲਾ_special_char_ਵਿੱਚ_vararg(str)
	 && '\0' == str[strcspn(str, G.ifs)]
	) {
		/* string has no special chars
		 * && string has no $IFS chars
		 */
		ਜੇ (dquoted) {
			/* Prints 1 (quoted expansion is a "" word, not nothing):
			 * set -- "${notexist-}"; echo $#
			 */
			output->has_quoted_part = 1;
		}
		ਵਾਪਸ expand_vars_ਮੰਜ਼ਿਲ_list(output, n, str);
	}

	ਤਿਆਰੀ_string_ਵਿੱਚ_str(&ਨਿਵੇਸ਼, str);

	ਲਈ (;;) {
		ਪੂਰਨ ch;

		ch = i_getch(&ਨਿਵੇਸ਼);
		debug_printf_parse("%s: ch=%c (%d) escape=%x\n",
				__func__, ch, ch, dest.o_expflags);

		ਜੇ (!dest.o_expflags) {
			ਜੇ (ch == EOF)
				ਰੋਕੋ;
			ਜੇ (!dquoted && !(output->o_expflags & EXP_FLAG_SINGLEWORD) && strchr(G.ifs, ch)) {
				/* PREFIX${x:d${e}f ...} and we met space: expand "d${e}f" and start new word.
				 * do not assume we are at the start of the word (PREFIX above).
				 */
				ਜੇ (dest.ਅੰਕੜੇ) {
					n = expand_vars_ਮੰਜ਼ਿਲ_list(output, n, dest.ਅੰਕੜੇ);
					o_free_and_ਸੈੱਟ_NULL(&dest);
					o_addchr(output, '\0');
					n = o_save_ptr(output, n); /* create next word */
				} ਨਹੀਂਤਾਂ
				ਜੇ (output->ਲੰਮਾਈ != o_get_ਆਖਰੀ_ptr(output, n)
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
				ਜਾਰੀ;
			}
			ਜੇ (!dquoted && ch == '\'') {
				ਜੇ (!ਜੋੜੋ_till_single_quote_dquoted(&dest, &ਨਿਵੇਸ਼))
					ਜਾਓ ret; /* error */
				o_addchr(&dest, SPECIAL_VAR_ਚਿੰਨ੍ਹ);
				o_addchr(&dest, SPECIAL_VAR_ਚਿੰਨ੍ਹ);
				ਜਾਰੀ;
			}
		}
		ਜੇ (ch == EOF) {
			syntax_error_unterm_ch('"');
			ਜਾਓ ret; /* error */
		}
		ਜੇ (ch == '"') {
			dest.o_expflags ^= EXP_FLAG_ESC_GLOB_CHARS;
			ਜੇ (dest.o_expflags) {
				o_addchr(&dest, SPECIAL_VAR_ਚਿੰਨ੍ਹ);
				o_addchr(&dest, SPECIAL_VAR_ਚਿੰਨ੍ਹ);
			}
			ਜਾਰੀ;
		}
		ਜੇ (ch == '\\') {
			ch = i_getch(&ਨਿਵੇਸ਼);
			ਜੇ (ch == EOF) {
//example? error message?	syntax_error_unterm_ch('"');
				debug_printf_parse("%s: error: \\<eof>\n", __func__);
				ਜਾਓ ret;
			}
			o_addqchr(&dest, ch);
			ਜਾਰੀ;
		}
		ਜੇ (ch == '$') {
			ਜੇ (!parse_dollar(ਨਿਰਾ, &dest, &ਨਿਵੇਸ਼, /*quote_mask:*/ (dest.o_expflags || dquoted) ? 0x80 : 0)) {
				debug_printf_parse("%s: error: parse_dollar returned 0 (error)\n", __func__);
				ਜਾਓ ret;
			}
			ਜਾਰੀ;
		}
#ਜੇ ENABLE_HUSH_TICK
		ਜੇ (ch == '`') {
			//unsigned pos = dest->length;
			o_addchr(&dest, SPECIAL_VAR_ਚਿੰਨ੍ਹ);
			o_addchr(&dest, (dest.o_expflags || dquoted) ? 0x80 | '`' : '`');
			ਜੇ (!ਜੋੜੋ_till_backquote(&dest, &ਨਿਵੇਸ਼,
					/*in_dquote:*/ dest.o_expflags /* nonzero if EXP_FLAG_ESC_GLOB_CHARS set */
				)
			) {
				ਜਾਓ ret; /* error */
			}
			o_addchr(&dest, SPECIAL_VAR_ਚਿੰਨ੍ਹ);
			//debug_printf_subst("SUBST RES3 '%s'\n", dest->data + pos);
			ਜਾਰੀ;
		}
#ਜੇ_ਅੰਤ
		ਜੇ (dquoted) {
			/* Always glob-protect if in dquotes:
			 * x=x; echo "${x:+/bin/c*}" - prints: /bin/c*
			 * x=x; echo "${x:+"/bin/c*"}" - prints: /bin/c*
			 */
			o_addqchr(&dest, ch);
		} ਨਹੀਂਤਾਂ {
			/* Glob-protect only if char is quoted:
			 * x=x; echo ${x:+/bin/c*} - prints many filenames
			 * x=x; echo ${x:+"/bin/c*"} - prints: /bin/c*
			 */
			o_ਜੋੜੋ_Qchr(&dest, ch);
		}
	} /* for (;;) */

	ਜੇ (dest.ਅੰਕੜੇ) {
		n = expand_vars_ਮੰਜ਼ਿਲ_list(output, n, dest.ਅੰਕੜੇ);
	}
 ret:
	o_free(&dest);
	ਵਾਪਸ n;
}
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
#ਜੇ ENABLE_FEATURE_SH_MATH
ਸਥਿਰ arith_t expand_and_evaluate_arith(ਅਚਲ ਅੱਖਰ *arg, ਅਚਲ ਅੱਖਰ **errmsg_p)
{
	arith_state_t math_state;
	arith_t res;
	ਅੱਖਰ *exp_str;

	math_state.lookupvar = get_local_var_ਮੁੱਲ;
	math_state.setvar = ਸੈੱਟ_local_var_from_halves;
	//math_state.endofname = endofname;
	exp_str = encode_then_expand_string(arg);
	res = arith(&math_state, exp_str ? exp_str : arg);
	ਮੁਕਤ(exp_str);
	ਜੇ (errmsg_p)
		*errmsg_p = math_state.errmsg;
	ਜੇ (math_state.errmsg)
		ਸੁਨੇਹਾ_and_die_if_script(math_state.errmsg);
	ਵਾਪਸ res;
}
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
#ਜੇ BASH_ਨਮੂਨਾ_SUBST
/* ${var/[/]pattern[/repl]} helpers */
ਸਥਿਰ ਅੱਖਰ *strstr_ਨਮੂਨਾ(ਅੱਖਰ *val, ਅਚਲ ਅੱਖਰ *ਨਮੂਨਾ, ਪੂਰਨ *ਮਾਪ)
{
	ਪੂਰਨ ਪਹਿਲਾ_escaped = (ਨਮੂਨਾ[0] == '\\' && ਨਮੂਨਾ[1]);
	/* "first_escaped" trick allows to treat e.g. "\*no_glob_chars"
	 * as literal too (as it is semi-common, and easy to accomodate
	 * by just using str + 1).
	 */
	ਪੂਰਨ sz = strcspn(ਨਮੂਨਾ + ਪਹਿਲਾ_escaped * 2, "*?[\\");
	ਜੇ ((ਨਮੂਨਾ + ਪਹਿਲਾ_escaped * 2)[sz] == '\0') {
		/* Optimization for trivial patterns.
		 * Testcase for very slow replace (performs about 22k replaces):
		 * x=::::::::::::::::::::::
		 * x=$x$x;x=$x$x;x=$x$x;x=$x$x;x=$x$x;x=$x$x;x=$x$x;x=$x$x;x=$x$x;x=$x$x;echo ${#x}
		 * echo "${x//:/|}"
		 */
		*ਮਾਪ = sz + ਪਹਿਲਾ_escaped;
		ਵਾਪਸ ਲੜੀ_ਖੋਜ(val, ਨਮੂਨਾ + ਪਹਿਲਾ_escaped);
	}

	ਜਦੋਂ (1) {
		ਅੱਖਰ *ਸਿਰਾ_2 = ਜਾਂਚ_and_ਮੇਲ(val, ਨਮੂਨਾ, ਜਾਂਚ_MOVE_FROM_ਸੱਜਾ + ਜਾਂਚ_ਮੇਲ_ਖੱਬਾ_HALF);
		debug_printf_varexp("val:'%s' pattern:'%s' end:'%s'\n", val, ਨਮੂਨਾ, ਸਿਰਾ_2);
		ਜੇ (ਸਿਰਾ_2) {
			*ਮਾਪ = ਸਿਰਾ_2 - val;
			ਵਾਪਸ val;
		}
		ਜੇ (*val == '\0')
			ਵਾਪਸ ਨਿਰਾ;
		/* Optimization: if "*pat" did not match the start of "string",
		 * we know that "tring", "ring" etc will not match too:
		 */
		ਜੇ (ਨਮੂਨਾ[0] == '*')
			ਵਾਪਸ ਨਿਰਾ;
		val++;
	}
}
ਸਥਿਰ ਅੱਖਰ *replace_ਨਮੂਨਾ(ਅੱਖਰ *val, ਅਚਲ ਅੱਖਰ *ਨਮੂਨਾ, ਅਚਲ ਅੱਖਰ *repl, ਅੱਖਰ exp_op)
{
	ਅੱਖਰ *ਨਤੀਜਾ = ਨਿਰਾ;
	ਬਿਨਾਨਿਸ਼ਾਨੀ res_len = 0;
	ਬਿਨਾਨਿਸ਼ਾਨੀ repl_len = ਲੜੀ_ਲੰਬਾਈ(repl);

	/* Null pattern never matches, including if "var" is empty */
	ਜੇ (!ਨਮੂਨਾ[0])
		ਵਾਪਸ ਨਤੀਜਾ; /* NULL, no replaces happened */

	ਜਦੋਂ (1) {
		ਪੂਰਨ ਮਾਪ;
		ਅੱਖਰ *s = strstr_ਨਮੂਨਾ(val, ਨਮੂਨਾ, &ਮਾਪ);
		ਜੇ (!s)
			ਰੋਕੋ;

		ਨਤੀਜਾ = xrealloc(ਨਤੀਜਾ, res_len + (s - val) + repl_len + 1);
		strcpy(mempcpy(ਨਤੀਜਾ + res_len, val, s - val), repl);
		res_len += (s - val) + repl_len;
		debug_printf_varexp("val:'%s' s:'%s' result:'%s'\n", val, s, ਨਤੀਜਾ);

		val = s + ਮਾਪ;
		ਜੇ (exp_op == '/')
			ਰੋਕੋ;
	}
	ਜੇ (*val && ਨਤੀਜਾ) {
		ਨਤੀਜਾ = xrealloc(ਨਤੀਜਾ, res_len + ਲੜੀ_ਲੰਬਾਈ(val) + 1);
		strcpy(ਨਤੀਜਾ + res_len, val);
		debug_printf_varexp("val:'%s' result:'%s'\n", val, ਨਤੀਜਾ);
	}
	debug_printf_varexp("result:'%s'\n", ਨਤੀਜਾ);
	ਵਾਪਸ ਨਤੀਜਾ;
}
#ਜੇ_ਅੰਤ /* BASH_PATTERN_SUBST */
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */

ਸਥਿਰ ਪੂਰਨ append_str_maybe_ifs_ਵੰਡੋ(o_string *output, ਪੂਰਨ n,
		ਪੂਰਨ ਪਹਿਲਾ_ch, ਅਚਲ ਅੱਖਰ *val)
{
	ਜੇ (!(ਪਹਿਲਾ_ch & 0x80)) { /* unquoted $VAR */
		debug_printf_expand("unquoted '%s', output->o_escape:%d\n", val,
				!!(output->o_expflags & EXP_FLAG_ESC_GLOB_CHARS));
		ਜੇ (val && val[0])
			n = expand_on_ifs(output, n, val);
	} ਨਹੀਂਤਾਂ { /* quoted "$VAR" */
		output->has_quoted_part = 1;
		debug_printf_expand("quoted '%s', output->o_escape:%d\n", val,
				!!(output->o_expflags & EXP_FLAG_ESC_GLOB_CHARS));
		ਜੇ (val && val[0])
			o_ਜੋੜੋ_Qstr(output, val);
	}
	ਵਾਪਸ n;
}

/* Handle <SPECIAL_VAR_SYMBOL>varname...<SPECIAL_VAR_SYMBOL> construct.
 */
ਸਥਿਰ NOINLINE ਪੂਰਨ expand_one_var(o_string *output, ਪੂਰਨ n,
		ਪੂਰਨ ਪਹਿਲਾ_ch, ਅੱਖਰ *arg, ਅੱਖਰ **pp)
{
	ਅਚਲ ਅੱਖਰ *val;
	ਅੱਖਰ *ਮੰਜ਼ਿਲ_be_freed;
	ਅੱਖਰ *p;
	ਅੱਖਰ *var;
	ਅੱਖਰ exp_op;
	ਅੱਖਰ exp_save = exp_save; /* for compiler */
	ਅੱਖਰ *exp_saveptr; /* points to expansion operator */
	ਅੱਖਰ *exp_ਸ਼ਬਦ = exp_ਸ਼ਬਦ; /* for compiler */
	ਅੱਖਰ arg0;

	val = ਨਿਰਾ;
	ਮੰਜ਼ਿਲ_be_freed = ਨਿਰਾ;
	p = *pp;
	*p = '\0'; /* replace trailing SPECIAL_VAR_SYMBOL */
	var = arg;
	exp_saveptr = arg[1] ? strchr(VAR_ENCODED_SUBST_OPS, arg[1]) : ਨਿਰਾ;
	arg0 = arg[0];
	arg[0] = (arg0 & 0x7f);
	exp_op = 0;

	ਜੇ (arg[0] == '#' && arg[1] /* ${#...} but not ${#} */
	 && (!exp_saveptr               /* and ( not(${#<op_char>...}) */
	    || (arg[2] == '\0' && strchr(SPECIAL_VARS_STR, arg[1])) /* or ${#C} "len of $C" ) */
	    )		/* NB: skipping ^^^specvar check mishandles ${#::2} */
	) {
		/* It must be length operator: ${#var} */
		var++;
		exp_op = 'L';
	} ਨਹੀਂਤਾਂ {
		/* Maybe handle parameter expansion */
		ਜੇ (exp_saveptr /* if 2nd char is one of expansion operators */
		 && strchr(NUMERIC_SPECVARS_STR, arg[0]) /* 1st char is special variable */
		) {
			/* ${?:0}, ${#[:]%0} etc */
			exp_saveptr = var + 1;
		} ਨਹੀਂਤਾਂ {
			/* ${?}, ${var}, ${var:0}, ${var[:]%0} etc */
			exp_saveptr = var+1 + strcspn(var+1, VAR_ENCODED_SUBST_OPS);
		}
		exp_op = exp_save = *exp_saveptr;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
		ਜੇ (exp_op) {
			exp_ਸ਼ਬਦ = exp_saveptr + 1;
			ਜੇ (exp_op == ':') {
				exp_op = *exp_ਸ਼ਬਦ++;
//TODO: try ${var:} and ${var:bogus} in non-bash config
				ਜੇ (BASH_SUBSTR
				 && (!exp_op || !strchr(MINUS_PLUS_EQUAL_QUESTION, exp_op))
				) {
					/* oops... it's ${var:N[:M]}, not ${var:?xxx} or some such */
					exp_op = ':';
					exp_ਸ਼ਬਦ--;
				}
			}
			*exp_saveptr = '\0';
		} /* else: it's not an expansion op, but bare ${var} */
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */
	}

	/* Look up the variable in question */
	ਜੇ (ਅੰਕ_ਹੈ(var[0])) {
		/* parse_dollar should have vetted var for us */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
		ਪੂਰਨ nn = xatoi_ਸਕਾਰਾਤਮਕ(var);
#ਨਹੀਂਤਾਂ /* __U_BOOT__ */
		ਪੂਰਨ nn = simple_strtoul(var, ਨਿਰਾ, 10);
#ਜੇ_ਅੰਤ /* __U_BOOT__ */
		ਜੇ (nn < G.global_argc)
			val = G.global_argv[nn];
		/* else val remains NULL: $N with too big N */
	} ਨਹੀਂਤਾਂ {
		ਚੋਣ (var[0]) {
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
		ਹਾਲਤ '$': /* pid */
			val = utoa(G.ਜੜ੍ਹ_pid);
			ਰੋਕੋ;
		ਹਾਲਤ '!': /* bg pid */
			val = G.ਆਖਰੀ_bg_pid ? utoa(G.ਆਖਰੀ_bg_pid) : "";
			ਰੋਕੋ;
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */
		ਹਾਲਤ '?': /* exitcode */
			val = utoa(G.ਆਖਰੀ_exitcode);
			ਰੋਕੋ;
		ਹਾਲਤ '#': /* argc */
			val = utoa(G.global_argc ? G.global_argc-1 : 0);
			ਰੋਕੋ;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
		ਹਾਲਤ '-': { /* active options */
			/* Check set_mode() to see what option chars we support */
			ਅੱਖਰ *cp;
			val = cp = G.optstring_buf;
			ਜੇ (G.o_opt[OPT_O_ERREXIT])
				*cp++ = 'e';
			ਜੇ (G_interactive_fd)
				*cp++ = 'i';
			ਜੇ (G_x_ਬਹੁਲਕ)
				*cp++ = 'x';
			/* If G.o_opt[OPT_O_NOEXEC] is true,
			 * commands read but are not executed,
			 * so $- can not execute too, 'n' is never seen in $-.
			 */
			ਜੇ (G.opt_c)
				*cp++ = 'c';
			ਜੇ (G.opt_s)
				*cp++ = 's';
			*cp = '\0';
			ਰੋਕੋ;
		}
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */
		ਮੂਲ:
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
			val = get_local_var_ਮੁੱਲ(var);
#ਨਹੀਂਤਾਂ /* __U_BOOT__ */
			/*
			 * Environment variable set with setenv* have to be
			 * expanded.
			 * So, we first search if the variable exists in
			 * environment, if this is not the case, we default to
			 * local value.
			 */
			val = env_get(var);
			ਜੇ (!val)
				val = get_local_var_ਮੁੱਲ(var);
#ਜੇ_ਅੰਤ /* __U_BOOT__ */
		}
	}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	/* Handle any expansions */
	ਜੇ (exp_op == 'L') {
		reinit_unicode_for_hush();
		debug_printf_expand("expand: length(%s)=", val);
		val = utoa(val ? unicode_strlen(val) : 0);
		debug_printf_expand("%s\n", val);
	} ਨਹੀਂਤਾਂ ਜੇ (exp_op) {
		ਜੇ (exp_op == '%' || exp_op == '#') {
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
			ਜੇ (val && val[0]) {
				ਅੱਖਰ *t;
				ਅੱਖਰ *exp_exp_ਸ਼ਬਦ;
				ਅੱਖਰ *loc;
				ਬਿਨਾਨਿਸ਼ਾਨੀ ਜਾਂਚ_flags = pick_ਜਾਂਚ(exp_op, *exp_ਸ਼ਬਦ);
				ਜੇ (exp_op == *exp_ਸ਼ਬਦ)  /* ## or %% */
					exp_ਸ਼ਬਦ++;
				debug_printf_expand("expand: exp_word:'%s'\n", exp_ਸ਼ਬਦ);
				exp_exp_ਸ਼ਬਦ = encode_then_expand_vararg(exp_ਸ਼ਬਦ, /*handle_squotes:*/ 1, /*unbackslash:*/ 0);
				ਜੇ (exp_exp_ਸ਼ਬਦ)
					exp_ਸ਼ਬਦ = exp_exp_ਸ਼ਬਦ;
				debug_printf_expand("expand: exp_word:'%s'\n", exp_ਸ਼ਬਦ);
				/*
				 * HACK ALERT. We depend here on the fact that
				 * G.global_argv and results of utoa and get_local_var_value
				 * are actually in writable memory:
				 * scan_and_match momentarily stores NULs there.
				 */
				t = (ਅੱਖਰ*)val;
				loc = ਜਾਂਚ_and_ਮੇਲ(t, exp_ਸ਼ਬਦ, ਜਾਂਚ_flags);
				debug_printf_expand("op:%c str:'%s' pat:'%s' res:'%s'\n", exp_op, t, exp_ਸ਼ਬਦ, loc);
				ਮੁਕਤ(exp_exp_ਸ਼ਬਦ);
				ਜੇ (loc) { /* match was found */
					ਜੇ (ਜਾਂਚ_flags & ਜਾਂਚ_ਮੇਲ_ਖੱਬਾ_HALF) /* #[#] */
						val = loc; /* take right part */
					ਨਹੀਂਤਾਂ /* %[%] */
						val = ਮੰਜ਼ਿਲ_be_freed = xstrndup(val, loc - val); /*ਖੱਬਾ*/
				}
			}
		}
#ਜੇ BASH_ਨਮੂਨਾ_SUBST
		ਨਹੀਂਤਾਂ ਜੇ (exp_op == '/' || exp_op == '\\') {
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
			ਜੇ (val /*&& val[0]*/) {
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
				ਅੱਖਰ *ਨਮੂਨਾ, *repl, *t;
				ਨਮੂਨਾ = encode_then_expand_vararg(exp_ਸ਼ਬਦ, /*handle_squotes:*/ 1, /*unbackslash:*/ 0);
				ਜੇ (!ਨਮੂਨਾ)
					ਨਮੂਨਾ = xstrdup(exp_ਸ਼ਬਦ);
				debug_printf_varexp("pattern:'%s'->'%s'\n", exp_ਸ਼ਬਦ, ਨਮੂਨਾ);
				*p++ = SPECIAL_VAR_ਚਿੰਨ੍ਹ;
				exp_ਸ਼ਬਦ = p;
				p = strchr(p, SPECIAL_VAR_ਚਿੰਨ੍ਹ);
				*p = '\0';
				repl = encode_then_expand_vararg(exp_ਸ਼ਬਦ, /*handle_squotes:*/ 1, /*unbackslash:*/ 1);
				debug_printf_varexp("repl:'%s'->'%s'\n", exp_ਸ਼ਬਦ, repl);
				/* HACK ALERT. We depend here on the fact that
				 * G.global_argv and results of utoa and get_local_var_value
				 * are actually in writable memory:
				 * replace_pattern momentarily stores NULs there. */
				t = (ਅੱਖਰ*)val;
				ਮੰਜ਼ਿਲ_be_freed = replace_ਨਮੂਨਾ(t,
						ਨਮੂਨਾ,
						(repl ? repl : exp_ਸ਼ਬਦ),
						exp_op);
				ਜੇ (ਮੰਜ਼ਿਲ_be_freed) /* at least one replace happened */
					val = ਮੰਜ਼ਿਲ_be_freed;
				ਮੁਕਤ(ਨਮੂਨਾ);
				ਮੁਕਤ(repl);
			} ਨਹੀਂਤਾਂ {
				/* Unset variable always gives nothing */
				//  a=; echo ${a/*/w}      # "w"
				//  unset a; echo ${a/*/w} # ""
				/* Just skip "replace" part */
				*p++ = SPECIAL_VAR_ਚਿੰਨ੍ਹ;
				p = strchr(p, SPECIAL_VAR_ਚਿੰਨ੍ਹ);
				*p = '\0';
			}
		}
#ਜੇ_ਅੰਤ /* BASH_PATTERN_SUBST */
		ਨਹੀਂਤਾਂ ਜੇ (exp_op == ':') {
#ਜੇ BASH_SUBSTR && ENABLE_FEATURE_SH_MATH
			/* It's ${var:N[:M]} bashism.
			 * Note that in encoded form it has TWO parts:
			 * var:N<SPECIAL_VAR_SYMBOL>M<SPECIAL_VAR_SYMBOL>
			 */
			arith_t beg, len;
			ਬਿਨਾਨਿਸ਼ਾਨੀ vallen;
			ਅਚਲ ਅੱਖਰ *errmsg;

			beg = expand_and_evaluate_arith(exp_ਸ਼ਬਦ, &errmsg);
			ਜੇ (errmsg)
				ਜਾਓ ਖਾਲੀ_ਥਾਵਾਂ_ਨਤੀਜਾ;
			debug_printf_varexp("beg:'%s'=%lld\n", exp_ਸ਼ਬਦ, (ਲੰਮਾ ਲੰਮਾ)beg);
			*p++ = SPECIAL_VAR_ਚਿੰਨ੍ਹ;
			exp_ਸ਼ਬਦ = p;
			p = strchr(p, SPECIAL_VAR_ਚਿੰਨ੍ਹ);
			*p = '\0';
			vallen = val ? ਲੜੀ_ਲੰਬਾਈ(val) : 0;
			ਜੇ (beg < 0) {
				/* negative beg counts from the end */
				beg = (arith_t)vallen + beg;
			}
			/* If expansion will be empty, do not even evaluate len */
			ਜੇ (!val || beg < 0 || beg > vallen) {
				/* Why > vallen, not >=? bash:
				 * unset b; a=ab; : ${a:2:${b=3}}; echo $b  # "", b=3 (!!!)
				 * unset b; a=a; : ${a:2:${b=3}}; echo $b   # "", b not set
				 */
				ਜਾਓ ਖਾਲੀ_ਥਾਵਾਂ_ਨਤੀਜਾ;
			}
			len = expand_and_evaluate_arith(exp_ਸ਼ਬਦ, &errmsg);
			ਜੇ (errmsg)
				ਜਾਓ ਖਾਲੀ_ਥਾਵਾਂ_ਨਤੀਜਾ;
			debug_printf_varexp("len:'%s'=%lld\n", exp_ਸ਼ਬਦ, (ਲੰਮਾ ਲੰਮਾ)len);
			debug_printf_varexp("from val:'%s'\n", val);
			ਜੇ (len < 0) {
				/* in bash, len=-n means strlen()-n */
				len = (arith_t)vallen - beg + len;
				ਜੇ (len < 0) /* bash compat */
					ਸੁਨੇਹਾ_and_die_if_script("%s: substring expression < 0", var);
			}
			ਜੇ (len <= 0 || !val /*|| beg >= vallen*/) {
 ਖਾਲੀ_ਥਾਵਾਂ_ਨਤੀਜਾ:
				val = ਨਿਰਾ;
			} ਨਹੀਂਤਾਂ {
				/* Paranoia. What if user entered 9999999999999
				 * which fits in arith_t but not int? */
				ਜੇ (len > ਵੱਧ_ਤੋਂ_ਵੱਧ_ਅੰਕ)
					len = ਵੱਧ_ਤੋਂ_ਵੱਧ_ਅੰਕ;
				val = ਮੰਜ਼ਿਲ_be_freed = xstrndup(val + beg, len);
			}
			debug_printf_varexp("val:'%s'\n", val);
#ਨਹੀਂਤਾਂ /* not (HUSH_SUBSTR_EXPANSION && FEATURE_SH_MATH) */
			ਸੁਨੇਹਾ_and_die_if_script("malformed ${%s:...}", var);
			val = ਨਿਰਾ;
#ਜੇ_ਅੰਤ
		} ਨਹੀਂਤਾਂ { /* one of "-=+?" */
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
			ਪੂਰਨ use_ਸ਼ਬਦ = (!val || ((exp_save == ':') && !val[0]));
			ਜੇ (exp_op == '+')
				use_ਸ਼ਬਦ = !use_ਸ਼ਬਦ;
			debug_printf_expand("expand: op:%c (null:%s) test:%i\n", exp_op,
					(exp_save == ':') ? "true" : "false", use_ਸ਼ਬਦ);
			ਜੇ (use_ਸ਼ਬਦ) {
				ਜੇ (exp_op == '+' || exp_op == '-') {
					/* ${var+word} - use alternative value */
					/* ${var-word} - use default value */
					n = encode_then_append_var_plusminus(output, n, exp_ਸ਼ਬਦ,
							/*dquoted:*/ (arg0 & 0x80)
					);
					val = ਨਿਰਾ;
				} ਨਹੀਂਤਾਂ {
					/* ${var?word} - indicate error if unset */
					/* ${var=word} - assign and use default value */
					ਮੰਜ਼ਿਲ_be_freed = encode_then_expand_vararg(exp_ਸ਼ਬਦ,
							/*handle_squotes:*/ !(arg0 & 0x80),
							/*unbackslash:*/ 0
					);
					ਜੇ (ਮੰਜ਼ਿਲ_be_freed)
						exp_ਸ਼ਬਦ = ਮੰਜ਼ਿਲ_be_freed;
					ਜੇ (exp_op == '?') {
						/* mimic bash message */
						ਸੁਨੇਹਾ_and_die_if_script("%s: %s",
							var,
							exp_ਸ਼ਬਦ[0]
							? exp_ਸ਼ਬਦ
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
					} ਨਹੀਂਤਾਂ {
						val = exp_ਸ਼ਬਦ;
					}
					ਜੇ (exp_op == '=') {
						/* ${var=[word]} or ${var:=[word]} */
						ਜੇ (ਅੰਕ_ਹੈ(var[0]) || var[0] == '#') {
							/* mimic bash message */
							ਸੁਨੇਹਾ_and_die_if_script("$%s: cannot assign in this way", var);
							val = ਨਿਰਾ;
						} ਨਹੀਂਤਾਂ {
							ਅੱਖਰ *new_var = xasprintf("%s=%s", var, val);
							ਸੈੱਟ_local_var0(new_var);
						}
					}
				}
			}
		} /* one of "-=+?" */

		*exp_saveptr = exp_save;
	} /* if (exp_op) */

#ਜੇ_ਅੰਤ /* !__U_BOOT__ */
	arg[0] = arg0;
	*pp = p;

	n = append_str_maybe_ifs_ਵੰਡੋ(output, n, ਪਹਿਲਾ_ch, val);

	ਮੁਕਤ(ਮੰਜ਼ਿਲ_be_freed);
	ਵਾਪਸ n;
}

/* Expand all variable references in given string, adding words to list[]
 * at n, n+1,... positions. Return updated n (so that list[n] is next one
 * to be filled). This routine is extremely tricky: has to deal with
 * variables/parameters with whitespace, $* and $@, and constructs like
 * 'echo -$*-'. If you play here, you must run testsuite afterwards! */
ਸਥਿਰ NOINLINE ਪੂਰਨ expand_vars_ਮੰਜ਼ਿਲ_list(o_string *output, ਪੂਰਨ n, ਅੱਖਰ *arg)
{
	/* output->o_expflags & EXP_FLAG_SINGLEWORD (0x80) if we are in
	 * expansion of right-hand side of assignment == 1-element expand.
	 */
	ਅੱਖਰ cant_be_null = 0; /* only bit 0x80 matters */
	ਅੱਖਰ *p;

	debug_printf_expand("expand_vars_to_list: arg:'%s' singleword:%x\n", arg,
			!!(output->o_expflags & EXP_FLAG_SINGLEWORD));
	debug_print_list("expand_vars_to_list[0]", output, n);

	ਜਦੋਂ ((p = strchr(arg, SPECIAL_VAR_ਚਿੰਨ੍ਹ)) != ਨਿਰਾ) {
		ਅੱਖਰ ਪਹਿਲਾ_ch;
#ਜੇ ENABLE_FEATURE_SH_MATH
		ਅੱਖਰ arith_buf[ਆਕਾਰ(arith_t)*3 + 2];
#ਜੇ_ਅੰਤ

		ਜੇ (output->ended_ਵਿੱਚ_ifs) {
			o_addchr(output, '\0');
			n = o_save_ptr(output, n);
			output->ended_ਵਿੱਚ_ifs = 0;
		}

		o_addblock(output, arg, p - arg);
		debug_print_list("expand_vars_to_list[1]", output, n);
		arg = ++p;
		p = strchr(p, SPECIAL_VAR_ਚਿੰਨ੍ਹ);

		/* Fetch special var name (if it is indeed one of them)
		 * and quote bit, force the bit on if singleword expansion -
		 * important for not getting v=$@ expand to many words. */
		ਪਹਿਲਾ_ch = arg[0] | (output->o_expflags & EXP_FLAG_SINGLEWORD);

		/* Is this variable quoted and thus expansion can't be null?
		 * "$@" is special. Even if quoted, it can still
		 * expand to nothing (not even an empty string),
		 * thus it is excluded. */
		ਜੇ ((ਪਹਿਲਾ_ch & 0x7f) != '@')
			cant_be_null |= ਪਹਿਲਾ_ch;

		ਚੋਣ (ਪਹਿਲਾ_ch & 0x7f) {
		/* Highest bit in first_ch indicates that var is double-quoted */
		ਹਾਲਤ '*':
		ਹਾਲਤ '@': {
			ਪੂਰਨ i;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
			ਜੇ (!G.global_argv[1])
#ਨਹੀਂਤਾਂ /* __U_BOOT__ */
			ਜੇ (!G.global_argv || !G.global_argv[1])
#ਜੇ_ਅੰਤ /* __U_BOOT__ */
				ਰੋਕੋ;
			i = 1;
			cant_be_null |= ਪਹਿਲਾ_ch; /* do it for "$@" _now_, when we know it's not empty */
			ਜੇ (!(ਪਹਿਲਾ_ch & 0x80)) { /* unquoted $* or $@ */
				ਜਦੋਂ (G.global_argv[i]) {
					n = expand_on_ifs(output, n, G.global_argv[i]);
					debug_printf_expand("expand_vars_to_list: argv %d (last %d)\n", i, G.global_argc - 1);
					ਜੇ (G.global_argv[i++][0] && G.global_argv[i]) {
						/* this argv[] is not empty and not last:
						 * put terminating NUL, start new word */
						o_addchr(output, '\0');
						debug_print_list("expand_vars_to_list[2]", output, n);
						n = o_save_ptr(output, n);
						debug_print_list("expand_vars_to_list[3]", output, n);
					}
				}
			} ਨਹੀਂਤਾਂ
			/* If EXP_FLAG_SINGLEWORD, we handle assignment 'a=....$@.....'
			 * and in this case should treat it like '$*' - see 'else...' below */
			ਜੇ (ਪਹਿਲਾ_ch == (ਅੱਖਰ)('@'|0x80)  /* quoted $@ */
			 && !(output->o_expflags & EXP_FLAG_SINGLEWORD) /* not v="$@" case */
			) {
				ਜਦੋਂ (1) {
					o_ਜੋੜੋ_Qstr(output, G.global_argv[i]);
					ਜੇ (++i >= G.global_argc)
						ਰੋਕੋ;
					o_addchr(output, '\0');
					debug_print_list("expand_vars_to_list[4]", output, n);
					n = o_save_ptr(output, n);
				}
			} ਨਹੀਂਤਾਂ { /* quoted $* (or v="$@" case): add as one word */
				ਜਦੋਂ (1) {
					o_ਜੋੜੋ_Qstr(output, G.global_argv[i]);
					ਜੇ (!G.global_argv[++i])
						ਰੋਕੋ;
					ਜੇ (G.ifs[0])
						o_addchr(output, G.ifs[0]);
				}
				output->has_quoted_part = 1;
			}
			ਰੋਕੋ;
		}
		ਹਾਲਤ SPECIAL_VAR_ਚਿੰਨ੍ਹ: {
			/* <SPECIAL_VAR_SYMBOL><SPECIAL_VAR_SYMBOL> */
			/* "Empty variable", used to make "" etc to not disappear */
			output->has_quoted_part = 1;
			cant_be_null = 0x80;
			arg++;
			ਰੋਕੋ;
		}
		ਹਾਲਤ SPECIAL_VAR_QUOTED_SVS:
			/* <SPECIAL_VAR_SYMBOL><SPECIAL_VAR_QUOTED_SVS><SPECIAL_VAR_SYMBOL> */
			/* "^C variable", represents literal ^C char (possible in scripts) */
			o_addchr(output, SPECIAL_VAR_ਚਿੰਨ੍ਹ);
			arg++;
			ਰੋਕੋ;
#ਜੇ ENABLE_HUSH_TICK
		ਹਾਲਤ '`': {
			/* <SPECIAL_VAR_SYMBOL>`cmd<SPECIAL_VAR_SYMBOL> */
			o_string subst_ਨਤੀਜਾ = NULL_O_STRING;

			*p = '\0'; /* replace trailing <SPECIAL_VAR_SYMBOL> */
			arg++;
			/* Can't just stuff it into output o_string,
			 * expanded result may need to be globbed
			 * and $IFS-split */
			debug_printf_subst("SUBST '%s' first_ch %x\n", arg, ਪਹਿਲਾ_ch);
			G.ਆਖਰੀ_exitcode = process_command_subs(&subst_ਨਤੀਜਾ, arg);
			G.expand_exitcode = G.ਆਖਰੀ_exitcode;
			debug_printf_subst("SUBST RES:%d '%s'\n", G.ਆਖਰੀ_exitcode, subst_ਨਤੀਜਾ.ਅੰਕੜੇ);
			n = append_str_maybe_ifs_ਵੰਡੋ(output, n, ਪਹਿਲਾ_ch, subst_ਨਤੀਜਾ.ਅੰਕੜੇ);
			o_free(&subst_ਨਤੀਜਾ);
			ਰੋਕੋ;
		}
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_FEATURE_SH_MATH
		ਹਾਲਤ '+': {
			/* <SPECIAL_VAR_SYMBOL>+arith<SPECIAL_VAR_SYMBOL> */
			arith_t res;

			arg++; /* skip '+' */
			*p = '\0'; /* replace trailing <SPECIAL_VAR_SYMBOL> */
			debug_printf_subst("ARITH '%s' first_ch %x\n", arg, ਪਹਿਲਾ_ch);
			res = expand_and_evaluate_arith(arg, ਨਿਰਾ);
			debug_printf_subst("ARITH RES '"ARITH_FMT"'\n", res);
			sprintf(arith_buf, ARITH_FMT, res);
			ਜੇ (res < 0
			 && ਪਹਿਲਾ_ch == (ਅੱਖਰ)('+'|0x80)
			/* && (output->o_expflags & EXP_FLAG_ESC_GLOB_CHARS) */
			) {
				/* Quoted negative ariths, like filename[0"$((-9))"],
				 * should not be interpreted as glob ranges.
				 * Convert leading '-' to '\-':
				 */
				o_grow_by(output, 1);
				output->ਅੰਕੜੇ[output->ਲੰਮਾਈ++] = '\\';
			}
			o_addstr(output, arith_buf);
			ਰੋਕੋ;
		}
#ਜੇ_ਅੰਤ
		ਮੂਲ:
			/* <SPECIAL_VAR_SYMBOL>varname[ops]<SPECIAL_VAR_SYMBOL> */
			n = expand_one_var(output, n, ਪਹਿਲਾ_ch, arg, &p);
			ਰੋਕੋ;
		} /* switch (char after <SPECIAL_VAR_SYMBOL>) */

		/* Restore NULL'ed SPECIAL_VAR_SYMBOL.
		 * Do the check to avoid writing to a const string. */
		ਜੇ (*p != SPECIAL_VAR_ਚਿੰਨ੍ਹ)
			*p = SPECIAL_VAR_ਚਿੰਨ੍ਹ;
		arg = ++p;
	} /* end of "while (SPECIAL_VAR_SYMBOL is found) ..." */

	ਜੇ (*arg) {
		/* handle trailing string */
		ਜੇ (output->ended_ਵਿੱਚ_ifs) {
			o_addchr(output, '\0');
			n = o_save_ptr(output, n);
		}
		debug_print_list("expand_vars_to_list[a]", output, n);
		/* this part is literal, and it was already pre-quoted
		 * if needed (much earlier), do not use o_addQstr here!
		 */
		o_addstr(output, arg);
		debug_print_list("expand_vars_to_list[b]", output, n);
	} ਨਹੀਂਤਾਂ
	ਜੇ (output->ਲੰਮਾਈ == o_get_ਆਖਰੀ_ptr(output, n) /* expansion is empty */
	 && !(cant_be_null & 0x80)   /* and all vars were not quoted */
	 && !output->has_quoted_part
	) {
		n--;
		/* allow to reuse list[n] later without re-growth */
		output->has_ਖਾਲੀ_ਥਾਵਾਂ_ਥਾਂ = 1;
	}

	ਵਾਪਸ n;
}

ਸਥਿਰ ਅੱਖਰ **expand_variables(ਅੱਖਰ **argv, ਬਿਨਾਨਿਸ਼ਾਨੀ expflags)
{
	ਪੂਰਨ n;
	ਅੱਖਰ **list;
	o_string output = NULL_O_STRING;

	output.o_expflags = expflags;

	n = 0;
	ਲਈ (;;) {
		/* go to next list[n] */
		output.ended_ਵਿੱਚ_ifs = 0;
		n = o_save_ptr(&output, n);

		ਜੇ (!*argv)
			ਰੋਕੋ;

		/* expand argv[i] */
		n = expand_vars_ਮੰਜ਼ਿਲ_list(&output, n, *argv++);
		/* if (!output->has_empty_slot) -- need this?? */
			o_addchr(&output, '\0');
	}
	debug_print_list("expand_variables", &output, n);

	/* output.data (malloced in one block) gets returned in "list" */
	list = o_finalize_list(&output, n);
	debug_print_ਲੜੀਆਂ("expand_variables[1]", list);
	ਵਾਪਸ list;
}

ਸਥਿਰ ਅੱਖਰ **expand_strvec_ਮੰਜ਼ਿਲ_strvec(ਅੱਖਰ **argv)
{
	ਵਾਪਸ expand_variables(argv, EXP_FLAG_GLOB | EXP_FLAG_ESC_GLOB_CHARS);
}

#ਜੇ defined(CMD_SINGLEWORD_NOGLOB) || defined(CMD_TEST2_SINGLEWORD_NOGLOB)
ਸਥਿਰ ਅੱਖਰ **expand_strvec_ਮੰਜ਼ਿਲ_strvec_singleword_noglob(ਅੱਖਰ **argv)
{
	ਵਾਪਸ expand_variables(argv, EXP_FLAG_SINGLEWORD);
}
#ਜੇ_ਅੰਤ

/* Used for expansion of right hand of assignments,
 * $((...)), heredocs, variable expansion parts.
 *
 * NB: should NOT do globbing!
 * "export v=/bin/c*; env | grep ^v=" outputs "v=/bin/c*"
 */
ਸਥਿਰ ਅੱਖਰ *expand_string_ਮੰਜ਼ਿਲ_string(ਅਚਲ ਅੱਖਰ *str, ਪੂਰਨ EXP_flags, ਪੂਰਨ do_unbackslash)
{
#ਜੇ !BASH_ਨਮੂਨਾ_SUBST && !ENABLE_HUSH_CASE
	ਅਚਲ ਪੂਰਨ do_unbackslash = 1;
	ਅਚਲ ਪੂਰਨ EXP_flags = EXP_FLAG_ESC_GLOB_CHARS;
#ਜੇ_ਅੰਤ
	ਅੱਖਰ *argv[2], **list;

	debug_printf_expand("string_to_string<='%s'\n", str);
	/* This is generally an optimization, but it also
	 * handles "", which otherwise trips over !list[0] check below.
	 * (is this ever happens that we actually get str="" here?)
	 */
	ਜੇ (!strchr(str, SPECIAL_VAR_ਚਿੰਨ੍ਹ) && !strchr(str, '\\')) {
		//TODO: Can use on strings with \ too, just unbackslash() them?
		debug_printf_expand("string_to_string(fast)=>'%s'\n", str);
		ਵਾਪਸ xstrdup(str);
	}

	argv[0] = (ਅੱਖਰ*)str;
	argv[1] = ਨਿਰਾ;
	list = expand_variables(argv, EXP_flags | EXP_FLAG_SINGLEWORD);
	ਜੇ (!list[0]) {
		/* Example where it happens:
		 * x=; echo ${x:-"$@"}
		 */
		((ਅੱਖਰ*)list)[0] = '\0';
	} ਨਹੀਂਤਾਂ {
		ਜੇ (HUSH_DEBUG)
			ਜੇ (list[1])
				bb_simple_error_ਸੁਨੇਹਾ_and_die("BUG in varexp2");
		/* actually, just move string 2*sizeof(char*) bytes back */
		overlapping_strcpy((ਅੱਖਰ*)list, list[0]);
		ਜੇ (do_unbackslash)
			unbackslash((ਅੱਖਰ*)list);
	}
	debug_printf_expand("string_to_string=>'%s'\n", (ਅੱਖਰ*)list);
	ਵਾਪਸ (ਅੱਖਰ*)list;
}

#ਜੇ 0
ਸਥਿਰ ਅੱਖਰ* expand_strvec_ਮੰਜ਼ਿਲ_string(ਅੱਖਰ **argv)
{
	ਅੱਖਰ **list;

	list = expand_variables(argv, EXP_FLAG_SINGLEWORD);
	/* Convert all NULs to spaces */
	ਜੇ (list[0]) {
		ਪੂਰਨ n = 1;
		ਜਦੋਂ (list[n]) {
			ਜੇ (HUSH_DEBUG)
				ਜੇ (list[n-1] + ਲੜੀ_ਲੰਬਾਈ(list[n-1]) + 1 != list[n])
					bb_error_ਸੁਨੇਹਾ_and_die("BUG in varexp3");
			/* bash uses ' ' regardless of $IFS contents */
			list[n][-1] = ' ';
			n++;
		}
	}
	overlapping_strcpy((ਅੱਖਰ*)list, list[0] ? list[0] : "");
	debug_printf_expand("strvec_to_string='%s'\n", (ਅੱਖਰ*)list);
	ਵਾਪਸ (ਅੱਖਰ*)list;
}
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
ਸਥਿਰ ਅੱਖਰ **expand_assignments(ਅੱਖਰ **argv, ਪੂਰਨ ਗਿਣਤੀ_2)
{
	ਪੂਰਨ i;
	ਅੱਖਰ **p;

	G.expanded_assignments = p = ਨਿਰਾ;
	/* Expand assignments into one string each */
	ਲਈ (i = 0; i < ਗਿਣਤੀ_2; i++) {
		p = ਜੋੜੋ_string_ਮੰਜ਼ਿਲ_ਲੜੀਆਂ(p,
			expand_string_ਮੰਜ਼ਿਲ_string(argv[i],
				EXP_FLAG_ESC_GLOB_CHARS,
				/*unbackslash:*/ 1
			)
		);
		G.expanded_assignments = p;
	}
	G.expanded_assignments = ਨਿਰਾ;
	ਵਾਪਸ p;
}

ਸਥਿਰ ਖਾਲੀ switch_off_special_sigs(ਬਿਨਾਨਿਸ਼ਾਨੀ ਨਕਾਬ)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ sig = 0;
	ਜਦੋਂ ((ਨਕਾਬ >>= 1) != 0) {
		sig++;
		ਜੇ (!(ਨਕਾਬ & 1))
			ਜਾਰੀ;
#ਜੇ ENABLE_HUSH_TRAP
		ਜੇ (G_traps) {
			ਜੇ (G_traps[sig] && !G_traps[sig][0])
				/* trap is '', has to remain SIG_IGN */
				ਜਾਰੀ;
			ਮੁਕਤ(G_traps[sig]);
			G_traps[sig] = ਨਿਰਾ;
		}
#ਜੇ_ਅੰਤ
		/* We are here only if no trap or trap was not '' */
		install_sighandler(sig, SIG_DFL);
	}
}
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
#ਜੇ BB_MMU
/* never called */
ਖਾਲੀ re_execute_shell(ਅੱਖਰ ***ਮੰਜ਼ਿਲ_free, ਅਚਲ ਅੱਖਰ *s,
		ਅੱਖਰ *g_argv0, ਅੱਖਰ **g_argv,
		ਅੱਖਰ **builtin_argv) NORETURN;

ਸਥਿਰ ਖਾਲੀ reset_traps_ਮੰਜ਼ਿਲ_defaults(ਖਾਲੀ)
{
	/* This function is always called in a child shell
	 * after fork (not vfork, NOMMU doesn't use this function).
	 */
	IF_HUSH_TRAP(ਬਿਨਾਨਿਸ਼ਾਨੀ sig;)
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਨਕਾਬ;

	/* Child shells are not interactive.
	 * SIGTTIN/SIGTTOU/SIGTSTP should not have special handling.
	 * Testcase: (while :; do :; done) + ^Z should background.
	 * Same goes for SIGTERM, SIGHUP, SIGINT.
	 */
	ਨਕਾਬ = (G.special_sig_ਨਕਾਬ & SPECIAL_INTERACTIVE_SIGS) | G_fatal_sig_ਨਕਾਬ;
	ਜੇ (!G_traps && !ਨਕਾਬ)
		ਵਾਪਸ; /* already no traps and no special sigs */

	/* Switch off special sigs */
	switch_off_special_sigs(ਨਕਾਬ);
# ਜੇ ENABLE_HUSH_ਕੰਮ
	G_fatal_sig_ਨਕਾਬ = 0;
# endif
	G.special_sig_ਨਕਾਬ &= ~SPECIAL_INTERACTIVE_SIGS;
	/* SIGQUIT,SIGCHLD and maybe SPECIAL_JOBSTOP_SIGS
	 * remain set in G.special_sig_mask */

# ਜੇ ENABLE_HUSH_TRAP
	ਜੇ (!G_traps)
		ਵਾਪਸ;

	/* Reset all sigs to default except ones with empty traps */
	ਲਈ (sig = 0; sig < NSIG; sig++) {
		ਜੇ (!G_traps[sig])
			ਜਾਰੀ; /* no trap: nothing to do */
		ਜੇ (!G_traps[sig][0])
			ਜਾਰੀ; /* empty trap: has to remain SIG_IGN */
		/* sig has non-empty trap, reset it: */
		ਮੁਕਤ(G_traps[sig]);
		G_traps[sig] = ਨਿਰਾ;
		/* There is no signal for trap 0 (EXIT) */
		ਜੇ (sig == 0)
			ਜਾਰੀ;
		install_sighandler(sig, pick_sighandler(sig));
	}
# endif
}

#ਨਹੀਂਤਾਂ /* !BB_MMU */

ਸਥਿਰ ਖਾਲੀ re_execute_shell(ਅੱਖਰ ***ਮੰਜ਼ਿਲ_free, ਅਚਲ ਅੱਖਰ *s,
		ਅੱਖਰ *g_argv0, ਅੱਖਰ **g_argv,
		ਅੱਖਰ **builtin_argv) NORETURN;
ਸਥਿਰ ਖਾਲੀ re_execute_shell(ਅੱਖਰ ***ਮੰਜ਼ਿਲ_free, ਅਚਲ ਅੱਖਰ *s,
		ਅੱਖਰ *g_argv0, ਅੱਖਰ **g_argv,
		ਅੱਖਰ **builtin_argv)
{
# define NOMMU_HACK_FMT ("-$%x:%x:%x:%x:%x:%llx" IF_HUSH_LOOPS(":%x"))
	/* delims + 2 * (number of bytes in printed hex numbers) */
	ਅੱਖਰ param_buf[ਆਕਾਰ(NOMMU_HACK_FMT) + 2 * (ਆਕਾਰ(ਪੂਰਨ)*6 + ਆਕਾਰ(ਲੰਮਾ ਲੰਮਾ)*1)];
	ਅੱਖਰ *heredoc_argv[4];
	ਬਣਤਰ variable *cur;
# ਜੇ ENABLE_HUSH_FUNCTIONS
	ਬਣਤਰ function *funcp;
# endif
	ਅੱਖਰ **argv, **pp;
	ਬਿਨਾਨਿਸ਼ਾਨੀ cnt;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ ਖਾਲੀ_ਥਾਵਾਂ_trap_ਨਕਾਬ;

	ਜੇ (!g_argv0) { /* heredoc */
		argv = heredoc_argv;
		argv[0] = (ਅੱਖਰ *) G.argv0_for_re_execing;
		argv[1] = (ਅੱਖਰ *) "-<";
		argv[2] = (ਅੱਖਰ *) s;
		argv[3] = ਨਿਰਾ;
		pp = &argv[3]; /* used as pointer to empty environment */
		ਜਾਓ do_exec;
	}

	cnt = 0;
	pp = builtin_argv;
	ਜੇ (pp) ਜਦੋਂ (*pp++)
		cnt++;

	ਖਾਲੀ_ਥਾਵਾਂ_trap_ਨਕਾਬ = 0;
	ਜੇ (G_traps) {
		ਪੂਰਨ sig;
		ਲਈ (sig = 1; sig < NSIG; sig++) {
			ਜੇ (G_traps[sig] && !G_traps[sig][0])
				ਖਾਲੀ_ਥਾਵਾਂ_trap_ਨਕਾਬ |= 1LL << sig;
		}
	}

	sprintf(param_buf, NOMMU_HACK_FMT
			, (ਬਿਨਾਨਿਸ਼ਾਨੀ) G.ਜੜ੍ਹ_pid
			, (ਬਿਨਾਨਿਸ਼ਾਨੀ) G.ਜੜ੍ਹ_ppid
			, (ਬਿਨਾਨਿਸ਼ਾਨੀ) G.ਆਖਰੀ_bg_pid
			, (ਬਿਨਾਨਿਸ਼ਾਨੀ) G.ਆਖਰੀ_exitcode
			, cnt
			, ਖਾਲੀ_ਥਾਵਾਂ_trap_ਨਕਾਬ
			IF_HUSH_LOOPS(, G.ਡੂੰਘਾਈ_ਦਾ_loop)
			);
# undef NOMMU_HACK_FMT
	/* 1:hush 2:-$<pid>:<pid>:<exitcode>:<etc...> <vars...> <funcs...>
	 * 3:-c 4:<cmd> 5:<arg0> <argN...> 6:NULL
	 */
	cnt += 6;
	ਲਈ (cur = G.ਸਿਖਰ_var; cur; cur = cur->ਅਗਲਾ) {
		ਜੇ (!cur->flg_export || cur->flg_read_only)
			cnt += 2;
	}
# ਜੇ ENABLE_HUSH_LINENO_VAR
	cnt += 2;
# endif
# ਜੇ ENABLE_HUSH_FUNCTIONS
	ਲਈ (funcp = G.ਸਿਖਰ_func; funcp; funcp = funcp->ਅਗਲਾ)
		cnt += 3;
# endif
	pp = g_argv;
	ਜਦੋਂ (*pp++)
		cnt++;
	*ਮੰਜ਼ਿਲ_free = argv = pp = xzalloc(ਆਕਾਰ(argv[0]) * cnt);
	*pp++ = (ਅੱਖਰ *) G.argv0_for_re_execing;
	*pp++ = param_buf;
	ਲਈ (cur = G.ਸਿਖਰ_var; cur; cur = cur->ਅਗਲਾ) {
		ਜੇ (strcmp(cur->varstr, hush_ਸੰਸਕਰਣ_str) == 0)
			ਜਾਰੀ;
		ਜੇ (cur->flg_read_only) {
			*pp++ = (ਅੱਖਰ *) "-R";
			*pp++ = cur->varstr;
		} ਨਹੀਂਤਾਂ ਜੇ (!cur->flg_export) {
			*pp++ = (ਅੱਖਰ *) "-V";
			*pp++ = cur->varstr;
		}
	}
# ਜੇ ENABLE_HUSH_LINENO_VAR
	*pp++ = (ਅੱਖਰ *) "-L";
	*pp++ = utoa(G.execute_lineno);
# endif
# ਜੇ ENABLE_HUSH_FUNCTIONS
	ਲਈ (funcp = G.ਸਿਖਰ_func; funcp; funcp = funcp->ਅਗਲਾ) {
		*pp++ = (ਅੱਖਰ *) "-F";
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
	*pp++ = (ਅੱਖਰ *) "-c";
	*pp++ = (ਅੱਖਰ *) s;
	ਜੇ (builtin_argv) {
		ਜਦੋਂ (*++builtin_argv)
			*pp++ = *builtin_argv;
		*pp++ = (ਅੱਖਰ *) "";
	}
	*pp++ = g_argv0;
	ਜਦੋਂ (*g_argv)
		*pp++ = *g_argv++;
	/* *pp = NULL; - is already there */
	pp = environ;

 do_exec:
	debug_printf_exec("re_execute_shell pid:%d cmd:'%s'\n", getpid(), s);
	/* Don't propagate SIG_IGN to the child */
	ਜੇ (SPECIAL_JOBSTOP_SIGS != 0)
		switch_off_special_sigs(G.special_sig_ਨਕਾਬ & SPECIAL_JOBSTOP_SIGS);
	execve(bb_busybox_exec_ਰਸਤਾ, argv, pp);
	/* Fallback. Useful for init=/bin/hush usage etc */
	ਜੇ (argv[0][0] == '/')
		execve(argv[0], argv, pp);
	xfunc_error_retval = 127;
	bb_simple_error_ਸੁਨੇਹਾ_and_die("can't re-execute the shell");
}
#ਜੇ_ਅੰਤ  /* !BB_MMU */

#ਜੇ_ਅੰਤ /* !__U_BOOT__ */

ਸਥਿਰ ਪੂਰਨ ਲੜੀਵਾਰ_and_free_list(ਬਣਤਰ pipe *pi);

/* Executing from string: eval, sh -c '...'
 *          or from file: /etc/profile, . file, sh <script>, sh (intereactive)
 * end_trigger controls how often we stop parsing
 * NUL: parse all, execute, return
 * ';': parse till ';' or newline, execute, repeat till EOF
 */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
ਸਥਿਰ ਖਾਲੀ parse_and_ਲੜੀਵਾਰ_ਧਾਰਾ(ਬਣਤਰ ਵਿੱਚ_str *inp, ਪੂਰਨ ਸਿਰਾ_trigger)
#ਨਹੀਂਤਾਂ /* __U_BOOT__ */
ਸਥਿਰ ਪੂਰਨ parse_and_ਲੜੀਵਾਰ_ਧਾਰਾ(ਬਣਤਰ ਵਿੱਚ_str *inp, ਪੂਰਨ ਸਿਰਾ_trigger)
#ਜੇ_ਅੰਤ /* __U_BOOT__ */
{
	/* Why we need empty flag?
	 * An obscure corner case "false; ``; echo $?":
	 * empty command in `` should still set $? to 0.
	 * But we can't just set $? to 0 at the start,
	 * this breaks "false; echo `echo $?`" case.
	 */
	bool ਖਾਲੀ_ਥਾਵਾਂ = 1;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	ਜਦੋਂ (1) {
#ਨਹੀਂਤਾਂ /* __U_BOOT__ */
	ਕਰੋ {
#ਜੇ_ਅੰਤ /* __U_BOOT__ */
		ਬਣਤਰ pipe *pipe_list;

#ਜੇ ENABLE_HUSH_INTERACTIVE
		ਜੇ (ਸਿਰਾ_trigger == ';') {
			G.promptmode = 0; /* PS1 */
			debug_printf_prompt("%s promptmode=%d\n", __func__, G.promptmode);
		}
#ਜੇ_ਅੰਤ
		pipe_list = parse_ਧਾਰਾ(ਨਿਰਾ, ਨਿਰਾ, inp, ਸਿਰਾ_trigger);
		ਜੇ (!pipe_list || pipe_list == ERR_PTR) { /* EOF/error */
			/* If we are in "big" script
			 * (not in `cmd` or something similar)...
			 */
			ਜੇ (pipe_list == ERR_PTR && ਸਿਰਾ_trigger == ';') {
				/* Discard cached input (rest of line) */
				ਪੂਰਨ ch = inp->ਆਖਰੀ_char;
				ਜਦੋਂ (ch != EOF && ch != '\n') {
					//bb_error_msg("Discarded:'%c'", ch);
					ch = i_getch(inp);
				}
				/* Force prompt */
				inp->p = ਨਿਰਾ;
				/* This stream isn't empty */
				ਖਾਲੀ_ਥਾਵਾਂ = 0;
				ਜਾਰੀ;
			}
			ਜੇ (!pipe_list && ਖਾਲੀ_ਥਾਵਾਂ)
				G.ਆਖਰੀ_exitcode = 0;
			ਰੋਕੋ;
		}
		debug_print_ਰੁੱਖ(pipe_list, 0);
		debug_printf_exec("parse_and_run_stream: run_and_free_list\n");
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
		ਲੜੀਵਾਰ_and_free_list(pipe_list);
#ਨਹੀਂਤਾਂ /* __U_BOOT__ */
		ਪੂਰਨ rcode = ਲੜੀਵਾਰ_and_free_list(pipe_list);
		/*
		 * We reset input string to not run the following command, so running
		 * 'exit; echo foo' does not print foo.
		 */
		ਜੇ (rcode <= EXIT_RET_ਕੋਡ)
			ਤਿਆਰੀ_file_ਵਿੱਚ_str(inp);
#ਜੇ_ਅੰਤ /* __U_BOOT__ */
		ਖਾਲੀ_ਥਾਵਾਂ = 0;
		ਜੇ (G_flag_return_ਵਿੱਚ_ਤਰੱਕੀ == 1)
			ਰੋਕੋ;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	}
#ਨਹੀਂਤਾਂ /* __U_BOOT__ */
	/*
	 * This do/while is needed by run_command to avoid looping on a command
	 * with syntax error.
	 */
	} ਜਦੋਂ (!(G.ਲੜੀਵਾਰ_command_flags & FLAG_EXIT_FROM_LOOP));

	ਵਾਪਸ G.ਆਖਰੀ_exitcode;
#ਜੇ_ਅੰਤ /* __U_BOOT__ */
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
ਸਥਿਰ ਖਾਲੀ parse_and_ਲੜੀਵਾਰ_string(ਅਚਲ ਅੱਖਰ *s)
#ਨਹੀਂਤਾਂ /* __U_BOOT__ */
ਸਥਿਰ ਪੂਰਨ parse_and_ਲੜੀਵਾਰ_string(ਅਚਲ ਅੱਖਰ *s)
#ਜੇ_ਅੰਤ /* __U_BOOT__ */
{
	ਬਣਤਰ ਵਿੱਚ_str ਨਿਵੇਸ਼;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	IF_HUSH_LINENO_VAR(ਬਿਨਾਨਿਸ਼ਾਨੀ sv = G.parse_lineno;)
#ਨਹੀਂਤਾਂ /* __U_BOOT__ */
	//IF_HUSH_LINENO_VAR(unsigned sv = G.parse_lineno;)
#ਜੇ_ਅੰਤ /* __U_BOOT__ */

	ਤਿਆਰੀ_string_ਵਿੱਚ_str(&ਨਿਵੇਸ਼, s);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	parse_and_ਲੜੀਵਾਰ_ਧਾਰਾ(&ਨਿਵੇਸ਼, '\0');
#ਨਹੀਂਤਾਂ /* __U_BOOT__ */
	ਵਾਪਸ parse_and_ਲੜੀਵਾਰ_ਧਾਰਾ(&ਨਿਵੇਸ਼, '\0');
#ਜੇ_ਅੰਤ /* __U_BOOT__ */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	IF_HUSH_LINENO_VAR(ਬਿਨਾਨਿਸ਼ਾਨੀ sv = G.parse_lineno;)
#ਨਹੀਂਤਾਂ /* __U_BOOT__ */
	//IF_HUSH_LINENO_VAR(unsigned sv = G.parse_lineno;)
#ਜੇ_ਅੰਤ /* __U_BOOT__ */
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
ਪੂਰਨ parse_string_outer_modern(ਅਚਲ ਅੱਖਰ *cmd, ਪੂਰਨ flags)
{
	ਪੂਰਨ ret;
	ਪੂਰਨ old_flags;

	/*
	 * Keep old values of run_command to be able to restore them once
	 * command was executed.
	 */
	old_flags = G.ਲੜੀਵਾਰ_command_flags;
	G.ਲੜੀਵਾਰ_command_flags = flags;

	ret = parse_and_ਲੜੀਵਾਰ_string(cmd);

	G.ਲੜੀਵਾਰ_command_flags = old_flags;

	ਵਾਪਸ ret;
}
#ਜੇ_ਅੰਤ /* __U_BOOT__ */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
ਸਥਿਰ ਖਾਲੀ parse_and_ਲੜੀਵਾਰ_file(HFILE *fp)
#ਨਹੀਂਤਾਂ /* __U_BOOT__ */
ਖਾਲੀ parse_and_ਲੜੀਵਾਰ_file(ਖਾਲੀ)
#ਜੇ_ਅੰਤ /* __U_BOOT__ */
{
	ਬਣਤਰ ਵਿੱਚ_str ਨਿਵੇਸ਼;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	IF_HUSH_LINENO_VAR(ਬਿਨਾਨਿਸ਼ਾਨੀ sv = G.parse_lineno;)

	IF_HUSH_LINENO_VAR(G.parse_lineno = 1;)
	ਤਿਆਰੀ_file_ਵਿੱਚ_str(&ਨਿਵੇਸ਼, fp);
#ਨਹੀਂਤਾਂ /* __U_BOOT__ */
	ਤਿਆਰੀ_file_ਵਿੱਚ_str(&ਨਿਵੇਸ਼);
#ਜੇ_ਅੰਤ /* __U_BOOT__ */
	parse_and_ਲੜੀਵਾਰ_ਧਾਰਾ(&ਨਿਵੇਸ਼, ';');
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	IF_HUSH_LINENO_VAR(G.parse_lineno = sv;)
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
#ਜੇ ENABLE_HUSH_TICK
ਸਥਿਰ ਪੂਰਨ generate_ਧਾਰਾ_from_string(ਅਚਲ ਅੱਖਰ *s, pid_t *pid_p)
{
	pid_t pid;
	ਪੂਰਨ channel[2];
# ਜੇ !BB_MMU
	ਅੱਖਰ **ਮੰਜ਼ਿਲ_free = ਨਿਰਾ;
# endif

	xpipe(channel);
	pid = BB_MMU ? xfork() : xvfork();
	ਜੇ (pid == 0) { /*ਬੱਚਾ*/
		disable_restore_tty_pgrp_on_exit();
		/* Process substitution is not considered to be usual
		 * 'command execution'.
		 * SUSv3 says ctrl-Z should be ignored, ctrl-C should not.
		 */
		bb_ਸੰਕੇਤ(0
			+ (1 << SIGTSTP)
			+ (1 << SIGTTIN)
			+ (1 << SIGTTOU)
			, SIG_IGN);
		close(channel[0]); /* NB: close _first_, then move fd! */
		xmove_fd(channel[1], 1);
# ਜੇ ENABLE_HUSH_TRAP
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
		ਜੇ (is_prefixed_with(s, "trap")
		 && skip_whitespace(s + 4)[0] == '\0'
		) {
			ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ argv[] ALIGN_PTR = { ਨਿਰਾ, ਨਿਰਾ };
			builtin_trap((ਅੱਖਰ**)argv);
			fflush_all(); /* important */
			_exit(0);
		}
# endif
# ਜੇ BB_MMU
		/* Prevent it from trying to handle ctrl-z etc */
		IF_HUSH_ਕੰਮ(G.ਲੜੀਵਾਰ_list_level = 1;)
		CLEAR_RANDOM_T(&G.random_gen); /* or else $RANDOM repeats in child */
		reset_traps_ਮੰਜ਼ਿਲ_defaults();
		IF_HUSH_ਬਹੁਲਕ_X(G.x_ਬਹੁਲਕ_ਡੂੰਘਾਈ++;)
		//bb_error_msg("%s: ++x_mode_depth=%d", __func__, G.x_mode_depth);
		parse_and_ਲੜੀਵਾਰ_string(s);
		_exit(G.ਆਖਰੀ_exitcode);
# ਨਹੀਂਤਾਂ
	/* We re-execute after vfork on NOMMU. This makes this script safe:
	 * yes "0123456789012345678901234567890" | dd bs=32 count=64k >BIG
	 * huge=`cat BIG` # was blocking here forever
	 * echo OK
	 */
		re_execute_shell(&ਮੰਜ਼ਿਲ_free,
				s,
				G.global_argv[0],
				G.global_argv + 1,
				ਨਿਰਾ);
# endif
	}

	/*ਮਾਪਾ*/
	*pid_p = pid;
# ਜੇ ENABLE_HUSH_FAST
	G.ਗਿਣਤੀ_SIGCHLD++;
//bb_error_msg("[%d] fork in generate_stream_from_string:"
//		" G.count_SIGCHLD:%d G.handled_SIGCHLD:%d",
//		getpid(), G.count_SIGCHLD, G.handled_SIGCHLD);
# endif
	enable_restore_tty_pgrp_on_exit();
# ਜੇ !BB_MMU
	ਮੁਕਤ(ਮੰਜ਼ਿਲ_free);
# endif
	close(channel[1]);
	ਵਾਪਸ channel[0];
}

/* Return code is exit status of the process that is run. */
ਸਥਿਰ ਪੂਰਨ process_command_subs(o_string *dest, ਅਚਲ ਅੱਖਰ *s)
{
	FILE *fp;
	pid_t pid;
	ਪੂਰਨ ਸਥਿਤੀ, ch, eol_cnt;

	fp = xfdopen_for_read(generate_ਧਾਰਾ_from_string(s, &pid));

	/* Now send results of command back into original context */
	eol_cnt = 0;
	ਜਦੋਂ ((ch = getc(fp)) != EOF) {
		ਜੇ (ch == '\0')
			ਜਾਰੀ;
		ਜੇ (ch == '\n') {
			eol_cnt++;
			ਜਾਰੀ;
		}
		ਜਦੋਂ (eol_cnt) {
			o_addchr(dest, '\n');
			eol_cnt--;
		}
		o_ਜੋੜੋ_Qchr(dest, ch);
	}

	debug_printf("done reading from `cmd` pipe, closing it\n");
	fclose(fp);
	/* We need to extract exitcode. Test case
	 * "true; echo `sleep 1; false` $?"
	 * should print 1 */
	ਸੁਰੱਖਿਅਤ_waitpid(pid, &ਸਥਿਤੀ, 0);
	debug_printf("child exited. returning its exitcode:%d\n", WEXITSTATUS(ਸਥਿਤੀ));
	ਵਾਪਸ WEXITSTATUS(ਸਥਿਤੀ);
}
#ਜੇ_ਅੰਤ /* ENABLE_HUSH_TICK */

ਸਥਿਰ ਖਾਲੀ ਤਿਆਰੀ_heredoc(ਬਣਤਰ redir_struct *redir)
{
	ਬਣਤਰ fd_pair pair;
	pid_t pid;
	ਪੂਰਨ len, written;
	/* the _body_ of heredoc (misleading field name) */
	ਅਚਲ ਅੱਖਰ *heredoc = redir->rd_filename;
	ਅੱਖਰ *expanded;
#ਜੇ !BB_MMU
	ਅੱਖਰ **ਮੰਜ਼ਿਲ_free;
#ਜੇ_ਅੰਤ

	expanded = ਨਿਰਾ;
	ਜੇ (!(redir->rd_dup & HEREDOC_QUOTED)) {
		expanded = encode_then_expand_string(heredoc);
		ਜੇ (expanded)
			heredoc = expanded;
	}
	len = ਲੜੀ_ਲੰਬਾਈ(heredoc);

	close(redir->rd_fd); /* often saves dup2+close in xmove_fd */
	xpiped_pair(pair);
	xmove_fd(pair.rd, redir->rd_fd);

	/* Try writing without forking. Newer kernels have
	 * dynamically growing pipes. Must use non-blocking write! */
	ndelay_on(pair.wr);
	ਜਦੋਂ (1) {
		written = ਲਿਖ(pair.wr, heredoc, len);
		ਜੇ (written <= 0)
			ਰੋਕੋ;
		len -= written;
		ਜੇ (len == 0) {
			close(pair.wr);
			ਮੁਕਤ(expanded);
			ਵਾਪਸ;
		}
		heredoc += written;
	}
	ndelay_off(pair.wr);

	/* Okay, pipe buffer was not big enough */
	/* Note: we must not create a stray child (bastard? :)
	 * for the unsuspecting parent process. Child creates a grandchild
	 * and exits before parent execs the process which consumes heredoc
	 * (that exec happens after we return from this function) */
#ਜੇ !BB_MMU
	ਮੰਜ਼ਿਲ_free = ਨਿਰਾ;
#ਜੇ_ਅੰਤ
	pid = xvfork();
	ਜੇ (pid == 0) {
		/*ਬੱਚਾ*/
		disable_restore_tty_pgrp_on_exit();
		pid = BB_MMU ? xfork() : xvfork();
		ਜੇ (pid != 0)
			_exit(0);
		/* grandchild */
		close(redir->rd_fd); /* read side of the pipe */
#ਜੇ BB_MMU
		ਭਰਿਆ_write(pair.wr, heredoc, len); /* may loop or block */
		_exit(0);
#ਨਹੀਂਤਾਂ
		/* Delegate blocking writes to another process */
		xmove_fd(pair.wr, STDOUT_FILENO);
		re_execute_shell(&ਮੰਜ਼ਿਲ_free, heredoc, ਨਿਰਾ, ਨਿਰਾ, ਨਿਰਾ);
#ਜੇ_ਅੰਤ
	}
	/*ਮਾਪਾ*/
#ਜੇ ENABLE_HUSH_FAST
	G.ਗਿਣਤੀ_SIGCHLD++;
//bb_error_msg("[%d] fork in setup_heredoc: G.count_SIGCHLD:%d G.handled_SIGCHLD:%d", getpid(), G.count_SIGCHLD, G.handled_SIGCHLD);
#ਜੇ_ਅੰਤ
	enable_restore_tty_pgrp_on_exit();
#ਜੇ !BB_MMU
	ਮੁਕਤ(ਮੰਜ਼ਿਲ_free);
#ਜੇ_ਅੰਤ
	close(pair.wr);
	ਮੁਕਤ(expanded);
	wait(ਨਿਰਾ); /* wait till child has died */
}

ਬਣਤਰ squirrel {
	ਪੂਰਨ orig_fd;
	ਪੂਰਨ moved_ਮੰਜ਼ਿਲ;
	/* moved_to = n: fd was moved to n; restore back to orig_fd after redir */
	/* moved_to = -1: fd was opened by redirect; close orig_fd after redir */
};

ਸਥਿਰ ਬਣਤਰ squirrel *append_squirrel(ਬਣਤਰ squirrel *sq, ਪੂਰਨ i, ਪੂਰਨ orig, ਪੂਰਨ moved)
{
	sq = xrealloc(sq, (i + 2) * ਆਕਾਰ(sq[0]));
	sq[i].orig_fd = orig;
	sq[i].moved_ਮੰਜ਼ਿਲ = moved;
	sq[i+1].orig_fd = -1; /* end marker */
	ਵਾਪਸ sq;
}

ਸਥਿਰ ਬਣਤਰ squirrel *ਜੋੜੋ_squirrel(ਬਣਤਰ squirrel *sq, ਪੂਰਨ fd, ਪੂਰਨ avoid_fd)
{
	ਪੂਰਨ moved_ਮੰਜ਼ਿਲ;
	ਪੂਰਨ i;

	i = 0;
	ਜੇ (sq) ਲਈ (; sq[i].orig_fd >= 0; i++) {
		/* If we collide with an already moved fd... */
		ਜੇ (fd == sq[i].moved_ਮੰਜ਼ਿਲ) {
			moved_ਮੰਜ਼ਿਲ = dup_CLOEXEC(sq[i].moved_ਮੰਜ਼ਿਲ, avoid_fd);
			debug_printf_redir("redirect_fd %d: already busy, moving to %d\n", fd, moved_ਮੰਜ਼ਿਲ);
			ਜੇ (moved_ਮੰਜ਼ਿਲ < 0) {
				/* "echo 2>/dev/tty 10>&9999" testcase:
				 * We move fd 2 to 10, then discover we need to move fd 10
				 * (and not hit 9999) and the latter fails.
				 */
				ਵਾਪਸ ਨਿਰਾ; /* fcntl failed */
			}
			sq[i].moved_ਮੰਜ਼ਿਲ = moved_ਮੰਜ਼ਿਲ;
			ਵਾਪਸ sq;
		}
		ਜੇ (fd == sq[i].orig_fd) {
			/* Example: echo Hello >/dev/null 1>&2 */
			debug_printf_redir("redirect_fd %d: already moved\n", fd);
			ਵਾਪਸ sq;
		}
	}

	/* If this fd is open, we move and remember it; if it's closed, moved_to = -1 */
	moved_ਮੰਜ਼ਿਲ = dup_CLOEXEC(fd, avoid_fd);
	debug_printf_redir("redirect_fd %d: previous fd is moved to %d (-1 if it was closed)\n", fd, moved_ਮੰਜ਼ਿਲ);
	ਜੇ (moved_ਮੰਜ਼ਿਲ < 0 && errno != EBADF)
		ਵਾਪਸ ਨਿਰਾ; /* fcntl failed (not because fd is closed) */
	ਵਾਪਸ append_squirrel(sq, i, fd, moved_ਮੰਜ਼ਿਲ);
}

ਸਥਿਰ ਬਣਤਰ squirrel *ਜੋੜੋ_squirrel_closed(ਬਣਤਰ squirrel *sq, ਪੂਰਨ fd)
{
	ਪੂਰਨ i;

	i = 0;
	ਜੇ (sq) ਲਈ (; sq[i].orig_fd >= 0; i++) {
		/* If we collide with an already moved fd... */
		ਜੇ (fd == sq[i].orig_fd) {
			/* Examples:
			 * "echo 3>FILE 3>&- 3>FILE"
			 * "echo 3>&- 3>FILE"
			 * No need for last redirect to insert
			 * another "need to close 3" indicator.
			 */
			debug_printf_redir("redirect_fd %d: already moved or closed\n", fd);
			ਵਾਪਸ sq;
		}
	}

	debug_printf_redir("redirect_fd %d: previous fd was closed\n", fd);
	ਵਾਪਸ append_squirrel(sq, i, fd, -1);
}

/* fd: redirect wants this fd to be used (e.g. 3>file).
 * Move all conflicting internally used fds,
 * and remember them so that we can restore them later.
 */
ਸਥਿਰ ਪੂਰਨ save_fd_on_redirect(ਪੂਰਨ fd, ਪੂਰਨ avoid_fd, ਬਣਤਰ squirrel **sqp)
{
	ਬਣਤਰ squirrel *new_squirrel;

	ਜੇ (avoid_fd < 9) /* the important case here is that it can be -1 */
		avoid_fd = 9;

#ਜੇ ENABLE_HUSH_INTERACTIVE
	ਜੇ (fd != 0 /* don't trigger for G_interactive_fd == 0 (that's "not interactive" flag) */
	 && fd == G_interactive_fd
	) {
		/* Testcase: "ls -l /proc/$$/fd 255>&-" should work */
		G_interactive_fd = xdup_CLOEXEC_and_close(G_interactive_fd, avoid_fd);
		debug_printf_redir("redirect_fd %d: matches interactive_fd, moving it to %d\n", fd, G_interactive_fd);
		ਵਾਪਸ 1; /* "we closed fd" */
	}
#ਜੇ_ਅੰਤ
	/* Are we called from setup_redirects(squirrel==NULL)
	 * in redirect in a [v]forked child?
	 */
	ਜੇ (sqp == ਨਿਰਾ) {
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
		ਵਾਪਸ 0;
	}

	/* If this one of script's fds? */
	ਜੇ (move_HFILEs_on_redirect(fd, avoid_fd))
		ਵਾਪਸ 1; /* yes. "we closed fd" (actually moved it) */

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
	ਜੇ (sqp == ERR_PTR) {
		/* Don't preserve redirected fds: exec is _meant_ to change these */
		//bb_error_msg("sqp == ERR_PTR: exec >FILE");
		ਵਾਪਸ 0;
	}

	/* Check whether it collides with any open fds (e.g. stdio), save fds as needed */
	new_squirrel = ਜੋੜੋ_squirrel(*sqp, fd, avoid_fd);
	ਜੇ (!new_squirrel)
		ਵਾਪਸ -1; /* redirect error */
	*sqp = new_squirrel;
	ਵਾਪਸ 0; /* "we did not close fd" */
}

ਸਥਿਰ ਖਾਲੀ restore_redirects(ਬਣਤਰ squirrel *sq)
{
	ਜੇ (sq) {
		ਪੂਰਨ i;
		ਲਈ (i = 0; sq[i].orig_fd >= 0; i++) {
			ਜੇ (sq[i].moved_ਮੰਜ਼ਿਲ >= 0) {
				/* We simply die on error */
				debug_printf_redir("restoring redirected fd from %d to %d\n", sq[i].moved_ਮੰਜ਼ਿਲ, sq[i].orig_fd);
				xmove_fd(sq[i].moved_ਮੰਜ਼ਿਲ, sq[i].orig_fd);
			} ਨਹੀਂਤਾਂ {
				/* cmd1 9>FILE; cmd2_should_see_fd9_closed */
				debug_printf_redir("restoring redirected fd %d: closing it\n", sq[i].orig_fd);
				close(sq[i].orig_fd);
			}
		}
		ਮੁਕਤ(sq);
	}
	ਜੇ (G.HFILE_stdin
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

#ਜੇ ENABLE_FEATURE_SH_STANDALONE && BB_MMU
ਸਥਿਰ ਖਾਲੀ close_saved_fds_and_FILE_fds(ਖਾਲੀ)
{
	ਜੇ (G_interactive_fd)
		close(G_interactive_fd);
	close_all_HFILE_list();
}
#ਜੇ_ਅੰਤ

ਸਥਿਰ ਪੂਰਨ internally_opened_fd(ਪੂਰਨ fd, ਬਣਤਰ squirrel *sq)
{
	ਪੂਰਨ i;

#ਜੇ ENABLE_HUSH_INTERACTIVE
	ਜੇ (fd == G_interactive_fd)
		ਵਾਪਸ 1;
#ਜੇ_ਅੰਤ
	/* If this one of script's fds? */
	ਜੇ (fd_ਵਿੱਚ_HFILEs(fd))
		ਵਾਪਸ 1;

	ਜੇ (sq) ਲਈ (i = 0; sq[i].orig_fd >= 0; i++) {
		ਜੇ (fd == sq[i].moved_ਮੰਜ਼ਿਲ)
			ਵਾਪਸ 1;
	}
	ਵਾਪਸ 0;
}

/* sqp != NULL means we squirrel away copies of stdin, stdout,
 * and stderr if they are redirected.
 * If redirection fails, return 1. This will make caller
 * skip command execution and restore already created redirect fds.
 */
ਸਥਿਰ ਪੂਰਨ ਤਿਆਰੀ_redirects(ਬਣਤਰ command *prog, ਬਣਤਰ squirrel **sqp)
{
	ਬਣਤਰ redir_struct *redir;

	ਲਈ (redir = prog->redirects; redir; redir = redir->ਅਗਲਾ) {
		ਪੂਰਨ newfd;
		ਪੂਰਨ closed;

		ਜੇ (redir->rd_type == REDIRECT_HEREDOC2) {
			/* "rd_fd<<HERE" case */
			ਜੇ (save_fd_on_redirect(redir->rd_fd, /*avoid:*/ 0, sqp) < 0)
				ਵਾਪਸ 1;
			/* for REDIRECT_HEREDOC2, rd_filename holds _contents_
			 * of the heredoc */
			debug_printf_redir("set heredoc '%s'\n",
					redir->rd_filename);
			ਤਿਆਰੀ_heredoc(redir);
			ਜਾਰੀ;
		}

		ਜੇ (redir->rd_dup == REDIRFD_ਮੰਜ਼ਿਲ_FILE) {
			/* "rd_fd<*>file" case (<*> is <,>,>>,<>) */
			ਅੱਖਰ *p;
			ਪੂਰਨ ਬਹੁਲਕ;

			ਜੇ (redir->rd_filename == ਨਿਰਾ) {
				/* Examples:
				 * "cmd >" (no filename)
				 * "cmd > <file" (2nd redirect starts too early)
				 */
				syntax_error("invalid redirect");
				ਵਾਪਸ 1;
			}
			ਬਹੁਲਕ = redir_ਸਾਰਣੀ[redir->rd_type].ਬਹੁਲਕ;
			p = expand_string_ਮੰਜ਼ਿਲ_string(redir->rd_filename,
				EXP_FLAG_ESC_GLOB_CHARS, /*unbackslash:*/ 1);
			newfd = open_or_warn(p, ਬਹੁਲਕ);
			ਮੁਕਤ(p);
			ਜੇ (newfd < 0) {
				/* Error message from open_or_warn can be lost
				 * if stderr has been redirected, but bash
				 * and ash both lose it as well
				 * (though zsh doesn't!)
				 */
				ਵਾਪਸ 1;
			}
			ਜੇ (newfd == redir->rd_fd && sqp
			 && sqp != ERR_PTR /* not a redirect in "exec" */
			) {
				/* open() gave us precisely the fd we wanted.
				 * This means that this fd was not busy
				 * (not opened to anywhere).
				 * Remember to close it on restore:
				 */
				*sqp = ਜੋੜੋ_squirrel_closed(*sqp, newfd);
				debug_printf_redir("redir to previously closed fd %d\n", newfd);
			}
		} ਨਹੀਂਤਾਂ {
			/* "rd_fd>&rd_dup" or "rd_fd>&-" case */
			newfd = redir->rd_dup;
		}

		ਜੇ (newfd == redir->rd_fd)
			ਜਾਰੀ;

		/* if "N>FILE": move newfd to redir->rd_fd */
		/* if "N>&M": dup newfd to redir->rd_fd */
		/* if "N>&-": close redir->rd_fd (newfd is REDIRFD_CLOSE) */

		closed = save_fd_on_redirect(redir->rd_fd, /*avoid:*/ newfd, sqp);
		ਜੇ (closed < 0)
			ਵਾਪਸ 1; /* error */
		ਜੇ (newfd == REDIRFD_CLOSE) {
			/* "N>&-" means "close me" */
			ਜੇ (!closed) {
				/* ^^^ optimization: saving may already
				 * have closed it. If not... */
				close(redir->rd_fd);
			}
			/* Sometimes we do another close on restore, getting EBADF.
			 * Consider "echo 3>FILE 3>&-"
			 * first redirect remembers "need to close 3",
			 * and second redirect closes 3! Restore code then closes 3 again.
			 */
		} ਨਹੀਂਤਾਂ {
			/* if newfd is a script fd or saved fd, do not allow to use it */
			ਜੇ (internally_opened_fd(newfd, sqp && sqp != ERR_PTR ? *sqp : ਨਿਰਾ)) {
				bb_error_ਸੁਨੇਹਾ("fd#%d is not open", newfd);
				ਵਾਪਸ 1;
			}
			ਜੇ (dup2(newfd, redir->rd_fd) < 0) {
				/* "echo >&99" testcase */
				bb_perror_ਸੁਨੇਹਾ("dup2(%d,%d)", newfd, redir->rd_fd);
				ਵਾਪਸ 1;
			}
			ਜੇ (redir->rd_dup == REDIRFD_ਮੰਜ਼ਿਲ_FILE)
				/* "rd_fd > FILE" */
				close(newfd);
			/* else: "rd_fd > rd_dup" */
		}
	}
	ਵਾਪਸ 0;
}

ਸਥਿਰ ਅੱਖਰ *ਲੱਭੋ_ਵਿੱਚ_ਰਸਤਾ(ਅਚਲ ਅੱਖਰ *arg)
{
	ਅੱਖਰ *ret = ਨਿਰਾ;
	ਅਚਲ ਅੱਖਰ *ਰਸਤਾ = get_local_var_ਮੁੱਲ("PATH");

	ਜੇ (!ਰਸਤਾ)
		ਵਾਪਸ ਨਿਰਾ;

	ਜਦੋਂ (1) {
		ਅਚਲ ਅੱਖਰ *ਸਿਰਾ_2 = strchrnul(ਰਸਤਾ, ':');
		ਪੂਰਨ sz = ਸਿਰਾ_2 - ਰਸਤਾ; /* must be int! */

		ਮੁਕਤ(ret);
		ਜੇ (sz != 0) {
			ret = xasprintf("%.*s/%s", sz, ਰਸਤਾ, arg);
		} ਨਹੀਂਤਾਂ {
			/* We have xxx::yyyy in $PATH,
			 * it means "use current dir" */
			ret = xstrdup(arg);
		}
		ਜੇ (access(ret, F_OK) == 0)
			ਰੋਕੋ;

		ਜੇ (*ਸਿਰਾ_2 == '\0') {
			ਮੁਕਤ(ret);
			ਵਾਪਸ ਨਿਰਾ;
		}
		ਰਸਤਾ = ਸਿਰਾ_2 + 1;
	}

	ਵਾਪਸ ret;
}

ਸਥਿਰ ਅਚਲ ਬਣਤਰ built_ਵਿੱਚ_command *ਲੱਭੋ_builtin_helper(ਅਚਲ ਅੱਖਰ *name,
		ਅਚਲ ਬਣਤਰ built_ਵਿੱਚ_command *x,
		ਅਚਲ ਬਣਤਰ built_ਵਿੱਚ_command *ਸਿਰਾ_2)
{
	ਜਦੋਂ (x != ਸਿਰਾ_2) {
		ਜੇ (strcmp(name, x->b_cmd) != 0) {
			x++;
			ਜਾਰੀ;
		}
		debug_printf_exec("found builtin '%s'\n", name);
		ਵਾਪਸ x;
	}
	ਵਾਪਸ ਨਿਰਾ;
}
ਸਥਿਰ ਅਚਲ ਬਣਤਰ built_ਵਿੱਚ_command *ਲੱਭੋ_builtin1(ਅਚਲ ਅੱਖਰ *name)
{
	ਵਾਪਸ ਲੱਭੋ_builtin_helper(name, bltins1, &bltins1[ARRAY_ਮਾਪ(bltins1)]);
}
ਸਥਿਰ ਅਚਲ ਬਣਤਰ built_ਵਿੱਚ_command *ਲੱਭੋ_builtin(ਅਚਲ ਅੱਖਰ *name)
{
	ਅਚਲ ਬਣਤਰ built_ਵਿੱਚ_command *x = ਲੱਭੋ_builtin1(name);
	ਜੇ (x)
		ਵਾਪਸ x;
	ਵਾਪਸ ਲੱਭੋ_builtin_helper(name, bltins2, &bltins2[ARRAY_ਮਾਪ(bltins2)]);
}

#ਜੇ ENABLE_HUSH_ਕੰਮ && ENABLE_FEATURE_TAB_COMPLETION
ਸਥਿਰ ਅਚਲ ਅੱਖਰ * FAST_FUNC hush_command_name(ਪੂਰਨ i)
{
	ਜੇ (/*i >= 0 && */ i < ARRAY_ਮਾਪ(bltins1)) {
		ਵਾਪਸ bltins1[i].b_cmd;
	}
	i -= ARRAY_ਮਾਪ(bltins1);
	ਜੇ (i < ARRAY_ਮਾਪ(bltins2)) {
		ਵਾਪਸ bltins2[i].b_cmd;
	}
# ਜੇ ENABLE_HUSH_FUNCTIONS
	{
		ਬਣਤਰ function *funcp;
		i -= ARRAY_ਮਾਪ(bltins2);
		ਲਈ (funcp = G.ਸਿਖਰ_func; funcp; funcp = funcp->ਅਗਲਾ) {
			ਜੇ (--i < 0)
				ਵਾਪਸ funcp->name;
		}
	}
# endif
	ਵਾਪਸ ਨਿਰਾ;
}
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
ਸਥਿਰ ਖਾਲੀ remove_nested_vars(ਖਾਲੀ)
{
	ਬਣਤਰ variable *cur;
	ਬਣਤਰ variable **cur_pp;

	cur_pp = &G.ਸਿਖਰ_var;
	ਜਦੋਂ ((cur = *cur_pp) != ਨਿਰਾ) {
		ਜੇ (cur->var_nest_level <= G.var_nest_level) {
			cur_pp = &cur->ਅਗਲਾ;
			ਜਾਰੀ;
		}
		/* Unexport */
		ਜੇ (cur->flg_export) {
			debug_printf_env("unexporting nested '%s'/%u\n", cur->varstr, cur->var_nest_level);
			bb_unsetenv(cur->varstr);
		}
		/* Remove from global list */
		*cur_pp = cur->ਅਗਲਾ;
		/* Free */
		ਜੇ (!cur->max_len) {
			debug_printf_env("freeing nested '%s'/%u\n", cur->varstr, cur->var_nest_level);
			ਮੁਕਤ(cur->varstr);
		}
		ਮੁਕਤ(cur);
	}
}

ਸਥਿਰ ਖਾਲੀ enter_var_nest_level(ਖਾਲੀ)
{
	G.var_nest_level++;
	debug_printf_env("var_nest_level++ %u\n", G.var_nest_level);

	/* Try:	f() { echo -n .; f; }; f
	 * struct variable::var_nest_level is uint16_t,
	 * thus limiting recursion to < 2^16.
	 * In any case, with 8 Mbyte stack SEGV happens
	 * not too long after 2^16 recursions anyway.
	 */
	ਜੇ (G.var_nest_level > 0xff00)
		bb_error_ਸੁਨੇਹਾ_and_die("fatal recursion (depth %u)", G.var_nest_level);
}

ਸਥਿਰ ਖਾਲੀ leave_var_nest_level(ਖਾਲੀ)
{
	G.var_nest_level--;
	debug_printf_env("var_nest_level-- %u\n", G.var_nest_level);
	ਜੇ (HUSH_DEBUG && (ਪੂਰਨ)G.var_nest_level < 0)
		bb_simple_error_ਸੁਨੇਹਾ_and_die("BUG: nesting underflow");

	remove_nested_vars();
}
#ਜੇ_ਅੰਤ /* __U_BOOT__ */

#ਜੇ ENABLE_HUSH_FUNCTIONS
ਸਥਿਰ ਬਣਤਰ function **ਲੱਭੋ_function_ਥਾਂ(ਅਚਲ ਅੱਖਰ *name)
{
	ਬਣਤਰ function *funcp;
	ਬਣਤਰ function **funcpp = &G.ਸਿਖਰ_func;

	ਜਦੋਂ ((funcp = *funcpp) != ਨਿਰਾ) {
		ਜੇ (strcmp(name, funcp->name) == 0) {
			debug_printf_exec("found function '%s'\n", name);
			ਰੋਕੋ;
		}
		funcpp = &funcp->ਅਗਲਾ;
	}
	ਵਾਪਸ funcpp;
}

ਸਥਿਰ ALWAYS_INLINE ਅਚਲ ਬਣਤਰ function *ਲੱਭੋ_function(ਅਚਲ ਅੱਖਰ *name)
{
	ਅਚਲ ਬਣਤਰ function *funcp = *ਲੱਭੋ_function_ਥਾਂ(name);
	ਵਾਪਸ funcp;
}

/* Note: takes ownership on name ptr */
ਸਥਿਰ ਬਣਤਰ function *new_function(ਅੱਖਰ *name)
{
	ਬਣਤਰ function **funcpp = ਲੱਭੋ_function_ਥਾਂ(name);
	ਬਣਤਰ function *funcp = *funcpp;

	ਜੇ (funcp != ਨਿਰਾ) {
		ਬਣਤਰ command *cmd = funcp->ਮਾਪਾ_cmd;
		debug_printf_exec("func %p parent_cmd %p\n", funcp, cmd);
		ਜੇ (!cmd) {
			debug_printf_exec("freeing & replacing function '%s'\n", funcp->name);
			ਮੁਕਤ(funcp->name);
			/* Note: if !funcp->body, do not free body_as_string!
			 * This is a special case of "-F name body" function:
			 * body_as_string was not malloced! */
			ਜੇ (funcp->body) {
				free_pipe_list(funcp->body);
# ਜੇ !BB_MMU
				ਮੁਕਤ(funcp->body_as_string);
# endif
			}
		} ਨਹੀਂਤਾਂ {
			debug_printf_exec("reinserting in tree & replacing function '%s'\n", funcp->name);
			cmd->argv[0] = funcp->name;
			cmd->ਸਮੂਹ = funcp->body;
# ਜੇ !BB_MMU
			cmd->ਸਮੂਹ_as_string = funcp->body_as_string;
# endif
		}
	} ਨਹੀਂਤਾਂ {
		debug_printf_exec("remembering new function '%s'\n", name);
		funcp = *funcpp = xzalloc(ਆਕਾਰ(*funcp));
		/*funcp->next = NULL;*/
	}

	funcp->name = name;
	ਵਾਪਸ funcp;
}

# ਜੇ ENABLE_HUSH_UNSET
ਸਥਿਰ ਖਾਲੀ unset_func(ਅਚਲ ਅੱਖਰ *name)
{
	ਬਣਤਰ function **funcpp = ਲੱਭੋ_function_ਥਾਂ(name);
	ਬਣਤਰ function *funcp = *funcpp;

	ਜੇ (funcp != ਨਿਰਾ) {
		debug_printf_exec("freeing function '%s'\n", funcp->name);
		*funcpp = funcp->ਅਗਲਾ;
		/* funcp is unlinked now, deleting it.
		 * Note: if !funcp->body, the function was created by
		 * "-F name body", do not free ->body_as_string
		 * and ->name as they were not malloced. */
		ਜੇ (funcp->body) {
			free_pipe_list(funcp->body);
			ਮੁਕਤ(funcp->name);
#  ਜੇ !BB_MMU
			ਮੁਕਤ(funcp->body_as_string);
#  endif
		}
		ਮੁਕਤ(funcp);
	}
}
# endif

# ਜੇ BB_MMU
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ exec_function(ਮੰਜ਼ਿਲ_free, funcp, argv) \
	exec_function(funcp, argv)
# endif
ਸਥਿਰ ਖਾਲੀ exec_function(ਅੱਖਰ ***ਮੰਜ਼ਿਲ_free,
		ਅਚਲ ਬਣਤਰ function *funcp,
		ਅੱਖਰ **argv) NORETURN;
ਸਥਿਰ ਖਾਲੀ exec_function(ਅੱਖਰ ***ਮੰਜ਼ਿਲ_free,
		ਅਚਲ ਬਣਤਰ function *funcp,
		ਅੱਖਰ **argv)
{
# ਜੇ BB_MMU
	ਪੂਰਨ n;

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
	G_flag_return_ਵਿੱਚ_ਤਰੱਕੀ = -1;
	enter_var_nest_level();
	IF_HUSH_LOCAL(G.func_nest_level++;)

	/* On MMU, funcp->body is always non-NULL */
	n = ਲੜੀਵਾਰ_list(funcp->body);
	_exit(n);
# ਨਹੀਂਤਾਂ
//?	close_saved_fds_and_FILE_fds();

//TODO: check whether "true | func_with_return" works

	re_execute_shell(ਮੰਜ਼ਿਲ_free,
			funcp->body_as_string,
			G.global_argv[0],
			argv + 1,
			ਨਿਰਾ);
# endif
}

ਸਥਿਰ ਪੂਰਨ ਲੜੀਵਾਰ_function(ਅਚਲ ਬਣਤਰ function *funcp, ਅੱਖਰ **argv)
{
	ਪੂਰਨ rc;
	save_arg_t sv;
	smallint sv_flg;

	save_and_replace_G_ਦਲੀਲਾਂ(&sv, argv);

	/* "We are in function, ok to use return" */
	sv_flg = G_flag_return_ਵਿੱਚ_ਤਰੱਕੀ;
	G_flag_return_ਵਿੱਚ_ਤਰੱਕੀ = -1;

	/* Make "local" variables properly shadow previous ones */
	IF_HUSH_LOCAL(enter_var_nest_level();)
	IF_HUSH_LOCAL(G.func_nest_level++;)

	/* On MMU, funcp->body is always non-NULL */
# ਜੇ !BB_MMU
	ਜੇ (!funcp->body) {
		/* Function defined by -F */
		parse_and_ਲੜੀਵਾਰ_string(funcp->body_as_string);
		rc = G.ਆਖਰੀ_exitcode;
	} ਨਹੀਂਤਾਂ
# endif
	{
		rc = ਲੜੀਵਾਰ_list(funcp->body);
	}

	IF_HUSH_LOCAL(G.func_nest_level--;)
	IF_HUSH_LOCAL(leave_var_nest_level();)

	G_flag_return_ਵਿੱਚ_ਤਰੱਕੀ = sv_flg;
# ਜੇ ENABLE_HUSH_TRAP
	debug_printf_exec("G.return_exitcode=-1\n");
	G.return_exitcode = -1; /* invalidate stashed return value */
# endif

	restore_G_ਦਲੀਲਾਂ(&sv, argv);

	ਵਾਪਸ rc;
}
#ਜੇ_ਅੰਤ /* ENABLE_HUSH_FUNCTIONS */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
#ਜੇ BB_MMU
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ exec_builtin(ਮੰਜ਼ਿਲ_free, x, argv) \
	exec_builtin(x, argv)
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ exec_builtin(ਮੰਜ਼ਿਲ_free, x, argv) \
	exec_builtin(ਮੰਜ਼ਿਲ_free, argv)
#ਜੇ_ਅੰਤ
ਸਥਿਰ ਖਾਲੀ exec_builtin(ਅੱਖਰ ***ਮੰਜ਼ਿਲ_free,
		ਅਚਲ ਬਣਤਰ built_ਵਿੱਚ_command *x,
		ਅੱਖਰ **argv) NORETURN;
ਸਥਿਰ ਖਾਲੀ exec_builtin(ਅੱਖਰ ***ਮੰਜ਼ਿਲ_free,
		ਅਚਲ ਬਣਤਰ built_ਵਿੱਚ_command *x,
		ਅੱਖਰ **argv)
{
#ਜੇ BB_MMU
	ਪੂਰਨ rcode;
//?	close_saved_fds_and_FILE_fds();
	rcode = x->b_function(argv);
	fflush_all();
	_exit(rcode);
#ਨਹੀਂਤਾਂ
	fflush_all();
	/* On NOMMU, we must never block!
	 * Example: { sleep 99 | read line; } & echo Ok
	 */
	re_execute_shell(ਮੰਜ਼ਿਲ_free,
			argv[0],
			G.global_argv[0],
			G.global_argv + 1,
			argv);
#ਜੇ_ਅੰਤ
}
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
ਸਥਿਰ ਖਾਲੀ execvp_or_die(ਅੱਖਰ **argv) NORETURN;
ਸਥਿਰ ਖਾਲੀ execvp_or_die(ਅੱਖਰ **argv)
{
	ਪੂਰਨ e;
	debug_printf_exec("execing '%s'\n", argv[0]);
	/* Don't propagate SIG_IGN to the child */
	ਜੇ (SPECIAL_JOBSTOP_SIGS != 0)
		switch_off_special_sigs(G.special_sig_ਨਕਾਬ & SPECIAL_JOBSTOP_SIGS);
	execvp(argv[0], argv);
	e = 2;
	ਜੇ (errno == EACCES) e = 126;
	ਜੇ (errno == ENOENT) e = 127;
	bb_perror_ਸੁਨੇਹਾ("can't execute '%s'", argv[0]);
	_exit(e);
}

#ਜੇ ENABLE_HUSH_ਬਹੁਲਕ_X
ਸਥਿਰ ਖਾਲੀ x_ਬਹੁਲਕ_print_optionally_squoted(ਅਚਲ ਅੱਖਰ *str)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ len;
	ਅਚਲ ਅੱਖਰ *cp;

	cp = str;

	/* the set of chars which-cause-string-to-be-squoted mimics bash */
	/* test a char with: bash -c 'set -x; echo "CH"' */
	ਜੇ (str[strcspn(str, "\\\"'`$(){}[]<>;#&|~*?!^"
			" " "\001\002\003\004\005\006\007"
			"\010\011\012\013\014\015\016\017"
			"\020\021\022\023\024\025\026\027"
			"\030\031\032\033\034\035\036\037"
			)
		] == '\0'
	) {
		/* string has no special chars */
		x_ਬਹੁਲਕ_addstr(str);
		ਵਾਪਸ;
	}

	cp = str;
	ਲਈ (;;) {
		/* print '....' up to EOL or first squote */
		len = (ਪੂਰਨ)(strchrnul(cp, '\'') - cp);
		ਜੇ (len != 0) {
			x_ਬਹੁਲਕ_addchr('\'');
			x_ਬਹੁਲਕ_addblock(cp, len);
			x_ਬਹੁਲਕ_addchr('\'');
			cp += len;
		}
		ਜੇ (*cp == '\0')
			ਰੋਕੋ;
		/* string contains squote(s), print them as \' */
		x_ਬਹੁਲਕ_addchr('\\');
		x_ਬਹੁਲਕ_addchr('\'');
		cp++;
	}
}
ਸਥਿਰ ਖਾਲੀ dump_cmd_ਵਿੱਚ_x_ਬਹੁਲਕ(ਅੱਖਰ **argv)
{
	ਜੇ (G_x_ਬਹੁਲਕ && argv) {
		ਬਿਨਾਨਿਸ਼ਾਨੀ n;

		/* "+[+++...][ cmd...]\n\0" */
		x_ਬਹੁਲਕ_ਅਗੇਤਰ();
		n = 0;
		ਜਦੋਂ (argv[n]) {
			x_ਬਹੁਲਕ_addchr(' ');
			ਜੇ (argv[n][0] == '\0') {
				x_ਬਹੁਲਕ_addchr('\'');
				x_ਬਹੁਲਕ_addchr('\'');
			} ਨਹੀਂਤਾਂ {
				x_ਬਹੁਲਕ_print_optionally_squoted(argv[n]);
			}
			n++;
		}
		x_ਬਹੁਲਕ_flush();
	}
}
#ਨਹੀਂਤਾਂ
# define dump_cmd_ਵਿੱਚ_x_ਬਹੁਲਕ(argv) ((ਖਾਲੀ)0)
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
#ਜੇ ENABLE_HUSH_COMMAND
ਸਥਿਰ ਖਾਲੀ if_command_vV_print_and_exit(ਅੱਖਰ opt_vV, ਅੱਖਰ *cmd, ਅਚਲ ਅੱਖਰ *explanation)
{
	ਅੱਖਰ *ਮੰਜ਼ਿਲ_free;

	ਜੇ (!opt_vV)
		ਵਾਪਸ;

	ਮੰਜ਼ਿਲ_free = ਨਿਰਾ;
	ਜੇ (!explanation) {
		ਅੱਖਰ *ਰਸਤਾ_2 = getenv("PATH");
		explanation = ਮੰਜ਼ਿਲ_free = ਲੱਭੋ_executable(cmd, &ਰਸਤਾ_2); /* path == NULL is ok */
		ਜੇ (!explanation)
			_exit(1); /* PROG was not found */
		ਜੇ (opt_vV != 'V')
			cmd = ਮੰਜ਼ਿਲ_free; /* -v PROG prints "/path/to/PROG" */
	}
	ਲਿਖੋ((opt_vV == 'V') ? "%s is %s\n" : "%s\n", cmd, explanation);
	ਮੁਕਤ(ਮੰਜ਼ਿਲ_free);
	fflush_all();
	_exit(0);
}
#ਨਹੀਂਤਾਂ
# define if_command_vV_print_and_exit(a,b,c) ((ਖਾਲੀ)0)
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */

#ਜੇ BB_MMU
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ pseudo_exec_argv(nommu_save, argv, assignment_cnt, argv_expanded) \
	pseudo_exec_argv(argv, assignment_cnt, argv_expanded)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ pseudo_exec(nommu_save, command, argv_expanded) \
	pseudo_exec(command, argv_expanded)
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
/* Called after [v]fork() in run_pipe, or from builtin_exec.
 * Never returns.
 * Don't exit() here.  If you don't exec, use _exit instead.
 * The at_exit handlers apparently confuse the calling process,
 * in particular stdin handling. Not sure why? -- because of vfork! (vda)
 */
ਸਥਿਰ ਖਾਲੀ pseudo_exec_argv(nommu_save_t *nommu_save,
		ਅੱਖਰ **argv, ਪੂਰਨ assignment_cnt,
		ਅੱਖਰ **argv_expanded) NORETURN;
ਸਥਿਰ NOINLINE ਖਾਲੀ pseudo_exec_argv(nommu_save_t *nommu_save,
		ਅੱਖਰ **argv, ਪੂਰਨ assignment_cnt,
		ਅੱਖਰ **argv_expanded)
{
	ਅਚਲ ਬਣਤਰ built_ਵਿੱਚ_command *x;
	ਬਣਤਰ variable **sv_shadowed;
	ਅੱਖਰ **new_env;
	IF_HUSH_COMMAND(ਅੱਖਰ opt_vV = 0;)
	IF_HUSH_FUNCTIONS(ਅਚਲ ਬਣਤਰ function *funcp;)

	new_env = expand_assignments(argv, assignment_cnt);
	dump_cmd_ਵਿੱਚ_x_ਬਹੁਲਕ(new_env);

	ਜੇ (!argv[assignment_cnt]) {
		/* Case when we are here: ... | var=val | ...
		 * (note that we do not exit early, i.e., do not optimize out
		 * expand_assignments(): think about ... | var=`sleep 1` | ...
		 */
		free_ਲੜੀਆਂ(new_env);
		_exit_SUCCESS();
	}

	sv_shadowed = G.shadowed_vars_pp;
#ਜੇ BB_MMU
	G.shadowed_vars_pp = ਨਿਰਾ; /* "don't save, free them instead" */
#ਨਹੀਂਤਾਂ
	G.shadowed_vars_pp = &nommu_save->old_vars;
	G.var_nest_level++;
#ਜੇ_ਅੰਤ
	ਸੈੱਟ_vars_and_save_old(new_env);
	G.shadowed_vars_pp = sv_shadowed;

	ਜੇ (argv_expanded) {
		argv = argv_expanded;
	} ਨਹੀਂਤਾਂ {
		argv = expand_strvec_ਮੰਜ਼ਿਲ_strvec(argv + assignment_cnt);
#ਜੇ !BB_MMU
		nommu_save->argv = argv;
#ਜੇ_ਅੰਤ
	}
	dump_cmd_ਵਿੱਚ_x_ਬਹੁਲਕ(argv);

#ਜੇ ENABLE_FEATURE_SH_STANDALONE || BB_MMU
	ਜੇ (strchr(argv[0], '/') != ਨਿਰਾ)
		ਜਾਓ skip;
#ਜੇ_ਅੰਤ

#ਜੇ ENABLE_HUSH_FUNCTIONS
	/* Check if the command matches any functions (this goes before bltins) */
	funcp = ਲੱਭੋ_function(argv[0]);
	ਜੇ (funcp)
		exec_function(&nommu_save->argv_from_re_execing, funcp, argv);
#ਜੇ_ਅੰਤ

#ਜੇ ENABLE_HUSH_COMMAND
	/* "command BAR": run BAR without looking it up among functions
	 * "command -v BAR": print "BAR" or "/path/to/BAR"; or exit 1
	 * "command -V BAR": print "BAR is {a function,a shell builtin,/path/to/BAR}"
	 */
	ਜਦੋਂ (strcmp(argv[0], "command") == 0 && argv[1]) {
		ਅੱਖਰ *p;

		argv++;
		p = *argv;
		ਜੇ (p[0] != '-' || !p[1])
			ਜਾਰੀ; /* bash allows "command command command [-OPT] BAR" */

		ਲਈ (;;) {
			p++;
			ਚੋਣ (*p) {
			ਹਾਲਤ '\0':
				argv++;
				p = *argv;
				ਜੇ (p[0] != '-' || !p[1])
					ਜਾਓ after_opts;
				ਜਾਰੀ; /* next arg is also -opts, process it too */
			ਹਾਲਤ 'v':
			ਹਾਲਤ 'V':
				opt_vV = *p;
				ਜਾਰੀ;
			ਮੂਲ:
				bb_error_ਸੁਨੇਹਾ_and_die("%s: %s: invalid option", "command", argv[0]);
			}
		}
	}
 after_opts:
# ਜੇ ENABLE_HUSH_FUNCTIONS
	ਜੇ (opt_vV && ਲੱਭੋ_function(argv[0]))
		if_command_vV_print_and_exit(opt_vV, argv[0], "a function");
# endif
#ਜੇ_ਅੰਤ

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
	x = BB_MMU ? ਲੱਭੋ_builtin(argv[0]) : ਲੱਭੋ_builtin1(argv[0]);
	ਜੇ (x) {
		if_command_vV_print_and_exit(opt_vV, argv[0], "a shell builtin");
		exec_builtin(&nommu_save->argv_from_re_execing, x, argv);
	}

#ਜੇ ENABLE_FEATURE_SH_STANDALONE
	/* Check if the command matches any busybox applets */
	{
		ਪੂਰਨ a = ਲੱਭੋ_applet_by_name(argv[0]);
		ਜੇ (a >= 0) {
			if_command_vV_print_and_exit(opt_vV, argv[0], "an applet");
# ਜੇ BB_MMU /* see above why on NOMMU it is not allowed */
			ਜੇ (APPLET_IS_NOEXEC(a)) {
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
				switch_off_special_sigs(G.special_sig_ਨਕਾਬ);
				debug_printf_exec("running applet '%s'\n", argv[0]);
				ਲੜੀਵਾਰ_noexec_applet_and_exit(a, argv[0], argv);
			}
# endif
			/* Re-exec ourselves */
			debug_printf_exec("re-execing applet '%s'\n", argv[0]);
			/* Don't propagate SIG_IGN to the child */
			ਜੇ (SPECIAL_JOBSTOP_SIGS != 0)
				switch_off_special_sigs(G.special_sig_ਨਕਾਬ & SPECIAL_JOBSTOP_SIGS);
			execv(bb_busybox_exec_ਰਸਤਾ, argv);
			/* If they called chroot or otherwise made the binary no longer
			 * executable, fall through */
		}
	}
#ਜੇ_ਅੰਤ

#ਜੇ ENABLE_FEATURE_SH_STANDALONE || BB_MMU
 skip:
#ਜੇ_ਅੰਤ
	if_command_vV_print_and_exit(opt_vV, argv[0], ਨਿਰਾ);
	execvp_or_die(argv);
}

/* Called after [v]fork() in run_pipe
 */
ਸਥਿਰ ਖਾਲੀ pseudo_exec(nommu_save_t *nommu_save,
		ਬਣਤਰ command *command,
		ਅੱਖਰ **argv_expanded) NORETURN;
ਸਥਿਰ ਖਾਲੀ pseudo_exec(nommu_save_t *nommu_save,
		ਬਣਤਰ command *command,
		ਅੱਖਰ **argv_expanded)
{
#ਜੇ ENABLE_HUSH_FUNCTIONS
	ਜੇ (command->cmd_type == CMD_FUNCDEF) {
		/* Ignore funcdefs in pipes:
		 * true | f() { cmd }
		 */
		_exit(0);
	}
#ਜੇ_ਅੰਤ

	ਜੇ (command->argv) {
		pseudo_exec_argv(nommu_save, command->argv,
				command->assignment_cnt, argv_expanded);
	}

	ਜੇ (command->ਸਮੂਹ) {
		/* Cases when we are here:
		 * ( list )
		 * { list } &
		 * ... | ( list ) | ...
		 * ... | { list } | ...
		 */
#ਜੇ BB_MMU
		ਪੂਰਨ rcode;
		debug_printf_exec("pseudo_exec: run_list\n");
		reset_traps_ਮੰਜ਼ਿਲ_defaults();
		rcode = ਲੜੀਵਾਰ_list(command->ਸਮੂਹ);
		/* OK to leak memory by not calling free_pipe_list,
		 * since this process is about to exit */
		_exit(rcode);
#ਨਹੀਂਤਾਂ
		re_execute_shell(&nommu_save->argv_from_re_execing,
				command->ਸਮੂਹ_as_string,
				G.global_argv[0],
				G.global_argv + 1,
				ਨਿਰਾ);
#ਜੇ_ਅੰਤ
	}

	/* Case when we are here: ... | >file */
	debug_printf_exec("pseudo_exec'ed null command\n");
	_exit_SUCCESS();
}

#ਜੇ ENABLE_HUSH_ਕੰਮ
ਸਥਿਰ ਅਚਲ ਅੱਖਰ *get_cmdtext(ਬਣਤਰ pipe *pi)
{
	ਅੱਖਰ **argv;
	ਅੱਖਰ *p;
	ਪੂਰਨ len;

	/* This is subtle. ->cmdtext is created only on first backgrounding.
	 * (Think "cat, <ctrl-z>, fg, <ctrl-z>, fg, <ctrl-z>...." here...)
	 * On subsequent bg argv is trashed, but we won't use it */
	ਜੇ (pi->cmdtext)
		ਵਾਪਸ pi->cmdtext;

	argv = pi->cmds[0].argv;
	ਜੇ (!argv) {
		pi->cmdtext = xzalloc(1);
		ਵਾਪਸ pi->cmdtext;
	}
	len = 0;
	ਕਰੋ {
		len += ਲੜੀ_ਲੰਬਾਈ(*argv) + 1;
	} ਜਦੋਂ (*++argv);
	p = xmalloc(len);
	pi->cmdtext = p;
	argv = pi->cmds[0].argv;
	ਕਰੋ {
		p = stpcpy(p, *argv);
		*p++ = ' ';
	} ਜਦੋਂ (*++argv);
	p[-1] = '\0';
	ਵਾਪਸ pi->cmdtext;
}

ਸਥਿਰ ਖਾਲੀ remove_ਕੰਮ_from_ਸਾਰਣੀ(ਬਣਤਰ pipe *pi)
{
	ਬਣਤਰ pipe *prev_pipe;

	ਜੇ (pi == G.ਕੰਮ_list) {
		G.ਕੰਮ_list = pi->ਅਗਲਾ;
	} ਨਹੀਂਤਾਂ {
		prev_pipe = G.ਕੰਮ_list;
		ਜਦੋਂ (prev_pipe->ਅਗਲਾ != pi)
			prev_pipe = prev_pipe->ਅਗਲਾ;
		prev_pipe->ਅਗਲਾ = pi->ਅਗਲਾ;
	}
	G.ਆਖਰੀ_jobid = 0;
	ਜੇ (G.ਕੰਮ_list)
		G.ਆਖਰੀ_jobid = G.ਕੰਮ_list->jobid;
}

ਸਥਿਰ ਖਾਲੀ delete_ਮੁਕੰਮਲ_ਕੰਮ(ਬਣਤਰ pipe *pi)
{
	remove_ਕੰਮ_from_ਸਾਰਣੀ(pi);
	free_pipe(pi);
}

ਸਥਿਰ ਖਾਲੀ clean_ਉੱਪਰ_ਆਖਰੀ_dead_ਕੰਮ(ਖਾਲੀ)
{
	ਜੇ (G.ਕੰਮ_list && !G.ਕੰਮ_list->alive_cmds)
		delete_ਮੁਕੰਮਲ_ਕੰਮ(G.ਕੰਮ_list);
}

ਸਥਿਰ ਖਾਲੀ ਪਾਓ_ਕੰਮ_into_ਸਾਰਣੀ(ਬਣਤਰ pipe *pi)
{
	ਬਣਤਰ pipe *ਕੰਮ, **jobp;
	ਪੂਰਨ i;

	clean_ਉੱਪਰ_ਆਖਰੀ_dead_ਕੰਮ();

	/* Find the end of the list, and find next job ID to use */
	i = 0;
	jobp = &G.ਕੰਮ_list;
	ਜਦੋਂ ((ਕੰਮ = *jobp) != ਨਿਰਾ) {
		ਜੇ (ਕੰਮ->jobid > i)
			i = ਕੰਮ->jobid;
		jobp = &ਕੰਮ->ਅਗਲਾ;
	}
	pi->jobid = i + 1;

	/* Create a new job struct at the end */
	ਕੰਮ = *jobp = xmemdup(pi, ਆਕਾਰ(*pi));
	ਕੰਮ->ਅਗਲਾ = ਨਿਰਾ;
	ਕੰਮ->cmds = xzalloc(ਆਕਾਰ(pi->cmds[0]) * pi->ਅੰਕ_cmds);
	/* Cannot copy entire pi->cmds[] vector! This causes double frees */
	ਲਈ (i = 0; i < pi->ਅੰਕ_cmds; i++) {
		ਕੰਮ->cmds[i].pid = pi->cmds[i].pid;
		/* all other fields are not used and stay zero */
	}
	ਕੰਮ->cmdtext = xstrdup(get_cmdtext(pi));

	ਜੇ (G_interactive_fd)
		ਲਿਖੋ("[%u] %u %s\n", ਕੰਮ->jobid, (ਬਿਨਾਨਿਸ਼ਾਨੀ)ਕੰਮ->cmds[0].pid, ਕੰਮ->cmdtext);
	G.ਆਖਰੀ_jobid = ਕੰਮ->jobid;
}
#ਜੇ_ਅੰਤ /*ਕੰਮ*/

ਸਥਿਰ ਪੂਰਨ ਕੰਮ_exited_or_stopped(ਬਣਤਰ pipe *pi)
{
	ਪੂਰਨ rcode, i;

	ਜੇ (pi->alive_cmds != pi->stopped_cmds)
		ਵਾਪਸ -1;

	/* All processes in fg pipe have exited or stopped */
	rcode = 0;
	i = pi->ਅੰਕ_cmds;
	ਜਦੋਂ (--i >= 0) {
		rcode = pi->cmds[i].cmd_exitcode;
		/* usually last process gives overall exitstatus,
		 * but with "set -o pipefail", last *failed* process does */
		ਜੇ (G.o_opt[OPT_O_PIPEFAIL] == 0 || rcode != 0)
			ਰੋਕੋ;
	}
	IF_HAS_KEYWORDS(ਜੇ (pi->pi_inverted) rcode = !rcode;)
	ਵਾਪਸ rcode;
}

ਸਥਿਰ ਪੂਰਨ process_wait_ਨਤੀਜਾ(ਬਣਤਰ pipe *fg_pipe, pid_t childpid, ਪੂਰਨ ਸਥਿਤੀ)
{
#ਜੇ ENABLE_HUSH_ਕੰਮ
	ਬਣਤਰ pipe *pi;
#ਜੇ_ਅੰਤ
	ਪੂਰਨ i, dead;

	dead = WIFEXITED(ਸਥਿਤੀ) || WIFSIGNALED(ਸਥਿਤੀ);

#ਜੇ DEBUG_JOBS
	ਜੇ (WIFSTOPPED(ਸਥਿਤੀ))
		debug_printf_jobs("pid %d stopped by sig %d (exitcode %d)\n",
				childpid, WSTOPSIG(ਸਥਿਤੀ), WEXITSTATUS(ਸਥਿਤੀ));
	ਜੇ (WIFSIGNALED(ਸਥਿਤੀ))
		debug_printf_jobs("pid %d killed by sig %d (exitcode %d)\n",
				childpid, WTERMSIG(ਸਥਿਤੀ), WEXITSTATUS(ਸਥਿਤੀ));
	ਜੇ (WIFEXITED(ਸਥਿਤੀ))
		debug_printf_jobs("pid %d exited, exitcode %d\n",
				childpid, WEXITSTATUS(ਸਥਿਤੀ));
#ਜੇ_ਅੰਤ
	/* Were we asked to wait for a fg pipe? */
	ਜੇ (fg_pipe) {
		i = fg_pipe->ਅੰਕ_cmds;

		ਜਦੋਂ (--i >= 0) {
			ਪੂਰਨ rcode;

			debug_printf_jobs("check pid %d\n", fg_pipe->cmds[i].pid);
			ਜੇ (fg_pipe->cmds[i].pid != childpid)
				ਜਾਰੀ;
			ਜੇ (dead) {
				ਪੂਰਨ ex;
				fg_pipe->cmds[i].pid = 0;
				fg_pipe->alive_cmds--;
				ex = WEXITSTATUS(ਸਥਿਤੀ);
				/* bash prints killer signal's name for *last*
				 * process in pipe (prints just newline for SIGINT/SIGPIPE).
				 * Mimic this. Example: "sleep 5" + (^\ or kill -QUIT)
				 */
				ਜੇ (WIFSIGNALED(ਸਥਿਤੀ)) {
					ਪੂਰਨ sig = WTERMSIG(ਸਥਿਤੀ);
#ਜੇ ENABLE_HUSH_ਕੰਮ
					ਜੇ (G.ਲੜੀਵਾਰ_list_level == 1
					/* ^^^^^ Do not print in nested contexts, example:
					 * echo `sleep 1; sh -c 'kill -9 $$'` - prints "137", NOT "Killed 137"
					 */
					 && i == fg_pipe->ਅੰਕ_cmds-1
					) {
						/* strsignal() is for bash compat. ~600 bloat versus bbox's get_signame() */
						puts(sig == SIGINT || sig == SIGPIPE ? "" : strsignal(sig));
					}
#ਜੇ_ਅੰਤ
					/* TODO: if (WCOREDUMP(status)) + " (core dumped)"; */
					/* MIPS has 128 sigs (1..128), if sig==128,
					 * 128 + sig would result in exitcode 256 -> 0!
					 */
					ex = 128 | sig;
				}
				fg_pipe->cmds[i].cmd_exitcode = ex;
			} ਨਹੀਂਤਾਂ {
				fg_pipe->stopped_cmds++;
			}
			debug_printf_jobs("fg_pipe: alive_cmds %d stopped_cmds %d\n",
					fg_pipe->alive_cmds, fg_pipe->stopped_cmds);
			rcode = ਕੰਮ_exited_or_stopped(fg_pipe);
			ਜੇ (rcode >= 0) {
/* Note: *non-interactive* bash does not continue if all processes in fg pipe
 * are stopped. Testcase: "cat | cat" in a script (not on command line!)
 * and "killall -STOP cat" */
				ਜੇ (G_interactive_fd) {
#ਜੇ ENABLE_HUSH_ਕੰਮ
					ਜੇ (fg_pipe->alive_cmds != 0)
						ਪਾਓ_ਕੰਮ_into_ਸਾਰਣੀ(fg_pipe);
#ਜੇ_ਅੰਤ
					ਵਾਪਸ rcode;
				}
				ਜੇ (fg_pipe->alive_cmds == 0)
					ਵਾਪਸ rcode;
			}
			/* There are still running processes in the fg_pipe */
			ਵਾਪਸ -1;
		}
		/* It wasn't in fg_pipe, look for process in bg pipes */
	}

#ਜੇ ENABLE_HUSH_ਕੰਮ
	/* We were asked to wait for bg or orphaned children */
	/* No need to remember exitcode in this case */
	ਲਈ (pi = G.ਕੰਮ_list; pi; pi = pi->ਅਗਲਾ) {
		ਲਈ (i = 0; i < pi->ਅੰਕ_cmds; i++) {
			ਜੇ (pi->cmds[i].pid == childpid)
				ਜਾਓ ਮਿਲਿਆ_pi_and_prognum;
		}
	}
	/* Happens when shell is used as init process (init=/bin/sh) */
	debug_printf("checkjobs: pid %d was not in our list!\n", childpid);
	ਵਾਪਸ -1; /* this wasn't a process from fg_pipe */

 ਮਿਲਿਆ_pi_and_prognum:
	ਜੇ (dead) {
		/* child exited */
		ਪੂਰਨ rcode = WEXITSTATUS(ਸਥਿਤੀ);
		ਜੇ (WIFSIGNALED(ਸਥਿਤੀ))
			/* NB: not 128 + sig, MIPS has sig 128 */
			rcode = 128 | WTERMSIG(ਸਥਿਤੀ);
		pi->cmds[i].cmd_exitcode = rcode;
		ਜੇ (G.ਆਖਰੀ_bg_pid == pi->cmds[i].pid)
			G.ਆਖਰੀ_bg_pid_exitcode = rcode;
		pi->cmds[i].pid = 0;
		pi->alive_cmds--;
		ਜੇ (!pi->alive_cmds) {
# ਜੇ ENABLE_HUSH_BASH_COMPAT
			G.dead_ਕੰਮ_exitcode = ਕੰਮ_exited_or_stopped(pi);
# endif
			ਜੇ (G_interactive_fd) {
				ਲਿਖੋ(ਕੰਮ_ਸਥਿਤੀ_FORMAT, pi->jobid,
						"Done", pi->cmdtext);
				delete_ਮੁਕੰਮਲ_ਕੰਮ(pi);
			} ਨਹੀਂਤਾਂ {
/*
 * bash deletes finished jobs from job table only in interactive mode,
 * after "jobs" cmd, or if pid of a new process matches one of the old ones
 * (see cleanup_dead_jobs(), delete_old_job(), J_NOTIFIED in bash source).
 * Testcase script: "(exit 3) & sleep 1; wait %1; echo $?" prints 3 in bash.
 * We only retain one "dead" job, if it's the single job on the list.
 * This covers most of real-world scenarios where this is useful.
 */
				ਜੇ (pi != G.ਕੰਮ_list)
					delete_ਮੁਕੰਮਲ_ਕੰਮ(pi);
			}
		}
	} ਨਹੀਂਤਾਂ {
		/* child stopped */
		pi->stopped_cmds++;
	}
#ਜੇ_ਅੰਤ
	ਵਾਪਸ -1; /* this wasn't a process from fg_pipe */
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
ਸਥਿਰ ਪੂਰਨ checkjobs(ਬਣਤਰ pipe *fg_pipe, pid_t waitfor_pid)
{
	ਪੂਰਨ ਗੁਣ;
	ਪੂਰਨ ਸਥਿਤੀ;
	ਪੂਰਨ rcode = 0;

	debug_printf_jobs("checkjobs %p\n", fg_pipe);

	ਗੁਣ = WUNTRACED;
	ਜੇ (fg_pipe == ਨਿਰਾ)
		ਗੁਣ |= WNOHANG;

	errno = 0;
#ਜੇ ENABLE_HUSH_FAST
	ਜੇ (G.handled_SIGCHLD == G.ਗਿਣਤੀ_SIGCHLD) {
//bb_error_msg("[%d] checkjobs: G.count_SIGCHLD:%d G.handled_SIGCHLD:%d children?:%d fg_pipe:%p",
//getpid(), G.count_SIGCHLD, G.handled_SIGCHLD, G.we_have_children, fg_pipe);
		/* There was neither fork nor SIGCHLD since last waitpid */
		/* Avoid doing waitpid syscall if possible */
		ਜੇ (!G.we_have_children) {
			errno = ECHILD;
			ਵਾਪਸ -1;
		}
		ਜੇ (fg_pipe == ਨਿਰਾ) { /* is WNOHANG set? */
			/* We have children, but they did not exit
			 * or stop yet (we saw no SIGCHLD) */
			ਵਾਪਸ 0;
		}
		/* else: !WNOHANG, waitpid will block, can't short-circuit */
	}
#ਜੇ_ਅੰਤ

/* Do we do this right?
 * bash-3.00# sleep 20 | false
 * <ctrl-Z pressed>
 * [3]+  Stopped          sleep 20 | false
 * bash-3.00# echo $?
 * 1   <========== bg pipe is not fully done, but exitcode is already known!
 * [hush 1.14.0: yes we do it right]
 */
	ਜਦੋਂ (1) {
		pid_t childpid;
#ਜੇ ENABLE_HUSH_FAST
		ਪੂਰਨ i;
		i = G.ਗਿਣਤੀ_SIGCHLD;
#ਜੇ_ਅੰਤ
		childpid = waitpid(-1, &ਸਥਿਤੀ, ਗੁਣ);
		ਜੇ (childpid <= 0) {
			ਜੇ (childpid && errno != ECHILD)
				bb_simple_perror_ਸੁਨੇਹਾ("waitpid");
#ਜੇ ENABLE_HUSH_FAST
			ਨਹੀਂਤਾਂ { /* Until next SIGCHLD, waitpid's are useless */
				G.we_have_children = (childpid == 0);
				G.handled_SIGCHLD = i;
//bb_error_msg("[%d] checkjobs: waitpid returned <= 0, G.count_SIGCHLD:%d G.handled_SIGCHLD:%d", getpid(), G.count_SIGCHLD, G.handled_SIGCHLD);
			}
#ਜੇ_ਅੰਤ
			/* ECHILD (no children), or 0 (no change in children status) */
			rcode = childpid;
			ਰੋਕੋ;
		}
		rcode = process_wait_ਨਤੀਜਾ(fg_pipe, childpid, ਸਥਿਤੀ);
		ਜੇ (rcode >= 0) {
			/* fg_pipe exited or stopped */
			ਰੋਕੋ;
		}
		ਜੇ (childpid == waitfor_pid) { /* "wait PID" */
			debug_printf_exec("childpid==waitfor_pid:%d status:0x%08x\n", childpid, ਸਥਿਤੀ);
			rcode = WEXITSTATUS(ਸਥਿਤੀ);
			ਜੇ (WIFSIGNALED(ਸਥਿਤੀ))
				rcode = 128 | WTERMSIG(ਸਥਿਤੀ);
			ਜੇ (WIFSTOPPED(ਸਥਿਤੀ))
				/* bash: "cmd & wait $!" and cmd stops: $? = 128 | stopsig */
				rcode = 128 | WSTOPSIG(ਸਥਿਤੀ);
			rcode++;
			ਰੋਕੋ; /* "wait PID" called us, give it exitcode+1 */
		}
#ਜੇ ENABLE_HUSH_BASH_COMPAT
		ਜੇ (-1 == waitfor_pid /* "wait -n" (wait for any one job) */
		 && G.dead_ਕੰਮ_exitcode >= 0 /* some job did finish */
		) {
			debug_printf_exec("waitfor_pid:-1\n");
			rcode = G.dead_ਕੰਮ_exitcode + 1;
			ਰੋਕੋ;
		}
#ਜੇ_ਅੰਤ
		/* This wasn't one of our processes, or */
		/* fg_pipe still has running processes, do waitpid again */
	} /* while (waitpid succeeds)... */

	ਵਾਪਸ rcode;
}

#ਜੇ ENABLE_HUSH_ਕੰਮ
ਸਥਿਰ ਪੂਰਨ checkjobs_and_fg_shell(ਬਣਤਰ pipe *fg_pipe)
{
	pid_t p;
	ਪੂਰਨ rcode = checkjobs(fg_pipe, 0 /*(no pid to wait for)*/);
	ਜੇ (G_saved_tty_pgrp) {
		/* Job finished, move the shell to the foreground */
		p = getpgrp(); /* our process group id */
		debug_printf_jobs("fg'ing ourself: getpgrp()=%d\n", (ਪੂਰਨ)p);
		tcsetpgrp(G_interactive_fd, p);
	}
	ਵਾਪਸ rcode;
}
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */

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
ਸਥਿਰ ਖਾਲੀ ਸੈੱਟ_G_ifs(ਖਾਲੀ)
{
	/* Testcase: set -- q w e; (IFS='' echo "$*"; IFS=''; echo "$*"); echo "$*"
	 * Result should be 3 lines: q w e, qwe, q w e
	 */
	ਜੇ (G.ifs_whitespace != G.ifs)
		ਮੁਕਤ(G.ifs_whitespace);
	G.ifs = get_local_var_ਮੁੱਲ("IFS");
	ਜੇ (G.ifs) {
		ਅੱਖਰ *p;
		G.ifs_whitespace = (ਅੱਖਰ*)G.ifs;
		p = skip_whitespace(G.ifs);
		ਜੇ (*p) {
			/* Not all $IFS is whitespace */
			ਅੱਖਰ *d;
			ਪੂਰਨ len = p - G.ifs;
			p = skip_non_whitespace(p);
			G.ifs_whitespace = xmalloc(len + ਲੜੀ_ਲੰਬਾਈ(p) + 1); /* can overestimate */
			d = mempcpy(G.ifs_whitespace, G.ifs, len);
			ਜਦੋਂ (*p) {
				ਜੇ (ਖਾਲੀ_ਥਾਂ_ਹੈ(*p))
					*d++ = *p;
				p++;
			}
			*d = '\0';
		}
	} ਨਹੀਂਤਾਂ {
		G.ifs = defifs;
		G.ifs_whitespace = (ਅੱਖਰ*)G.ifs;
	}
}
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
#ਜੇ !ENABLE_HUSH_ਬਹੁਲਕ_X
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ redirect_and_varexp_helper(command, sqp, argv_expanded) \
	redirect_and_varexp_helper(command, sqp)
#ਜੇ_ਅੰਤ
ਸਥਿਰ ਪੂਰਨ redirect_and_varexp_helper(
		ਬਣਤਰ command *command,
		ਬਣਤਰ squirrel **sqp,
		ਅੱਖਰ **argv_expanded)
{
	/* Assignments occur before redirects. Try:
	 * a=`sleep 1` sleep 2 3>/qwe/rty
	 */

	ਅੱਖਰ **new_env = expand_assignments(command->argv, command->assignment_cnt);
	dump_cmd_ਵਿੱਚ_x_ਬਹੁਲਕ(new_env);
	dump_cmd_ਵਿੱਚ_x_ਬਹੁਲਕ(argv_expanded);
	/* this takes ownership of new_env[i] elements, and frees new_env: */
	ਸੈੱਟ_vars_and_save_old(new_env);

	ਵਾਪਸ ਤਿਆਰੀ_redirects(command, sqp);
}
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */

ਸਥਿਰ NOINLINE ਪੂਰਨ ਲੜੀਵਾਰ_pipe(ਬਣਤਰ pipe *pi)
{
	ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ null_ptr = ਨਿਰਾ;

	ਪੂਰਨ cmd_no;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	ਪੂਰਨ ਅਗਲਾ_infd;
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */
	ਬਣਤਰ command *command;
	ਅੱਖਰ **argv_expanded;
	ਅੱਖਰ **argv;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	ਬਣਤਰ squirrel *squirrel = ਨਿਰਾ;
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */
	ਪੂਰਨ rcode;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	/*
	 * Set rcode here to avoid returning a garbage value in the middle of
	 * the function.
	 * Also, if an error occurs, rcode value would be changed and last
	 * return will signal the error.
	 */
	rcode = 0;
#ਜੇ_ਅੰਤ /* __U_BOOT__ */

	debug_printf_exec("run_pipe start: members:%d\n", pi->ਅੰਕ_cmds);
	debug_enter();

	ਸੈੱਟ_G_ifs();

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	IF_HUSH_ਕੰਮ(pi->pgrp = -1;)
	pi->stopped_cmds = 0;
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */
	command = &pi->cmds[0];
	argv_expanded = ਨਿਰਾ;

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	ਜੇ (pi->ਅੰਕ_cmds != 1
	 || pi->followup == PIPE_BG
	 || command->cmd_type == CMD_SUBSHELL
	) {
		ਜਾਓ must_fork;
	}

	pi->alive_cmds = 1;
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */

	debug_printf_exec(": group:%p argv:'%s'\n",
		command->ਸਮੂਹ, command->argv ? command->argv[0] : "NONE");

	ਜੇ (command->ਸਮੂਹ) {
#ਜੇ ENABLE_HUSH_FUNCTIONS
		ਜੇ (command->cmd_type == CMD_FUNCDEF) {
			/* "executing" func () { list } */
			ਬਣਤਰ function *funcp;

			funcp = new_function(command->argv[0]);
			/* funcp->name is already set to argv[0] */
			funcp->body = command->ਸਮੂਹ;
# ਜੇ !BB_MMU
			funcp->body_as_string = command->ਸਮੂਹ_as_string;
			command->ਸਮੂਹ_as_string = ਨਿਰਾ;
# endif
			command->ਸਮੂਹ = ਨਿਰਾ;
			command->argv[0] = ਨਿਰਾ;
			debug_printf_exec("cmd %p has child func at %p\n", command, funcp);
			funcp->ਮਾਪਾ_cmd = command;
			command->ਬੱਚਾ_func = funcp;

			debug_printf_exec("run_pipe: return EXIT_SUCCESS\n");
			debug_leave();
			ਵਾਪਸ EXIT_SUCCESS;
		}
#ਜੇ_ਅੰਤ
		/* { list } */
		debug_printf_exec("non-subshell group\n");
		rcode = 1; /* exitcode if redir failed */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
		ਜੇ (ਤਿਆਰੀ_redirects(command, &squirrel) == 0) {
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */
			debug_printf_exec(": run_list\n");
//FIXME: we need to pass squirrel down into run_list()
//for SH_STANDALONE case, or else this construct:
// { find /proc/self/fd; true; } >FILE; cmd2
//has no way of closing saved fd#1 for "find",
//and in SH_STANDALONE mode, "find" is not execed,
//therefore CLOEXEC on saved fd does not help.
			rcode = ਲੜੀਵਾਰ_list(command->ਸਮੂਹ) & 0xff;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
		}
		restore_redirects(squirrel);
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */
		IF_HAS_KEYWORDS(ਜੇ (pi->pi_inverted) rcode = !rcode;)
		debug_leave();
		debug_printf_exec("run_pipe: return %d\n", rcode);
		ਵਾਪਸ rcode;
	}

	argv = command->argv ? command->argv : (ਅੱਖਰ **) &null_ptr;
	{
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
		ਅਚਲ ਬਣਤਰ built_ਵਿੱਚ_command *x;
		IF_HUSH_FUNCTIONS(ਅਚਲ ਬਣਤਰ function *funcp;)
		IF_NOT_HUSH_FUNCTIONS(ਗਿਣਤੀ { funcp = 0 };)
		ਬਣਤਰ variable **sv_shadowed;
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */
		ਬਣਤਰ variable *old_vars;

#ਜੇ ENABLE_HUSH_LINENO_VAR
		G.execute_lineno = command->lineno;
#ਜੇ_ਅੰਤ

		ਜੇ (argv[command->assignment_cnt] == ਨਿਰਾ) {
			/* Assignments, but no command.
			 * Ensure redirects take effect (that is, create files).
			 * Try "a=t >file"
			 */
			ਬਿਨਾਨਿਸ਼ਾਨੀ i;
			G.expand_exitcode = 0;
 only_assignments:
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
			rcode = ਤਿਆਰੀ_redirects(command, &squirrel);
			restore_redirects(squirrel);
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */

			/* Set shell variables */
			i = 0;
			ਜਦੋਂ (i < command->assignment_cnt) {
				ਅੱਖਰ *p = expand_string_ਮੰਜ਼ਿਲ_string(argv[i],
						EXP_FLAG_ESC_GLOB_CHARS,
						/*unbackslash:*/ 1
				);
#ਜੇ ENABLE_HUSH_ਬਹੁਲਕ_X
				ਜੇ (G_x_ਬਹੁਲਕ) {
					ਅੱਖਰ *eq;
					ਜੇ (i == 0)
						x_ਬਹੁਲਕ_ਅਗੇਤਰ();
					x_ਬਹੁਲਕ_addchr(' ');
					eq = strchrnul(p, '=');
					ਜੇ (*eq) eq++;
					x_ਬਹੁਲਕ_addblock(p, (eq - p));
					x_ਬਹੁਲਕ_print_optionally_squoted(eq);
					x_ਬਹੁਲਕ_flush();
				}
#ਜੇ_ਅੰਤ
				debug_printf_env("set shell var:'%s'->'%s'\n", *argv, p);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
				ਜੇ (ਸੈੱਟ_local_var0(p)) {
#ਨਹੀਂਤਾਂ /* __U_BOOT__ */
				ਜੇ (ਸੈੱਟ_local_var_modern(p, /*flag:*/ 0)) {
#ਜੇ_ਅੰਤ
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
			ਜੇ (rcode == 0)
				rcode = G.expand_exitcode;
			IF_HAS_KEYWORDS(ਜੇ (pi->pi_inverted) rcode = !rcode;)
			debug_leave();
			debug_printf_exec("run_pipe: return %d\n", rcode);
			ਵਾਪਸ rcode;
		}

		/* Expand the rest into (possibly) many strings each */
#ਜੇ defined(CMD_TEST2_SINGLEWORD_NOGLOB)
		ਜੇ (command->cmd_type == CMD_TEST2_SINGLEWORD_NOGLOB)
			argv_expanded = expand_strvec_ਮੰਜ਼ਿਲ_strvec_singleword_noglob(argv + command->assignment_cnt);
		ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
#ਜੇ defined(CMD_SINGLEWORD_NOGLOB)
		ਜੇ (command->cmd_type == CMD_SINGLEWORD_NOGLOB)
			argv_expanded = expand_strvec_ਮੰਜ਼ਿਲ_strvec_singleword_noglob(argv + command->assignment_cnt);
		ਨਹੀਂਤਾਂ
#ਜੇ_ਅੰਤ
			argv_expanded = expand_strvec_ਮੰਜ਼ਿਲ_strvec(argv + command->assignment_cnt);

		/* If someone gives us an empty string: `cmd with empty output` */
		ਜੇ (!argv_expanded[0]) {
			ਮੁਕਤ(argv_expanded);
			/* `false` still has to set exitcode 1 */
			G.expand_exitcode = G.ਆਖਰੀ_exitcode;
			ਜਾਓ only_assignments;
		}

		old_vars = ਨਿਰਾ;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
		sv_shadowed = G.shadowed_vars_pp;

		/* Check if argv[0] matches any functions (this goes before bltins) */
		IF_HUSH_FUNCTIONS(funcp = ਲੱਭੋ_function(argv_expanded[0]);)
		IF_HUSH_FUNCTIONS(x = ਨਿਰਾ;)
		IF_HUSH_FUNCTIONS(ਜੇ (!funcp))
			x = ਲੱਭੋ_builtin(argv_expanded[0]);
		ਜੇ (x || funcp) {
			ਜੇ (x && x->b_function == builtin_exec && argv_expanded[1] == ਨਿਰਾ) {
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

				ਜਾਓ clean_ਉੱਪਰ_and_ret1;
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
			ਜੇ (rcode == 0) {
				ਜੇ (!funcp) {
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
#ਜੇ ENABLE_HUSH_FUNCTIONS
				ਨਹੀਂਤਾਂ {
					debug_printf_exec(": function '%s' '%s'...\n",
						funcp->name, argv_expanded[1]);
					rcode = ਲੜੀਵਾਰ_function(funcp, argv_expanded) & 0xff;
					/*
					 * But do collect *to old_vars list* vars shadowed
					 * within function execution. To that end, restore
					 * this pointer _after_ function run:
					 */
					G.shadowed_vars_pp = sv_shadowed;
				}
#ਜੇ_ਅੰਤ
			}
		} ਨਹੀਂਤਾਂ
		ਜੇ (ENABLE_FEATURE_SH_NOFORK && ਅੰਕ_APPLETS > 1) {
			ਪੂਰਨ n = ਲੱਭੋ_applet_by_name(argv_expanded[0]);
			ਜੇ (n < 0 || !APPLET_IS_NOFORK(n))
				ਜਾਓ must_fork;

			enter_var_nest_level();
			/* Collect all variables "shadowed" by helper into old_vars list */
			G.shadowed_vars_pp = &old_vars;
			rcode = redirect_and_varexp_helper(command, &squirrel, argv_expanded);
			G.shadowed_vars_pp = sv_shadowed;

			ਜੇ (rcode == 0) {
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
				rcode = ਲੜੀਵਾਰ_nofork_applet(n, argv_expanded);
			}
		} ਨਹੀਂਤਾਂ
			ਜਾਓ must_fork;

		restore_redirects(squirrel);
 clean_ਉੱਪਰ_and_ret1:
		leave_var_nest_level();
		ਜੋੜੋ_vars(old_vars);

		/*
		 * Try "usleep 99999999" + ^C + "echo $?"
		 * with FEATURE_SH_NOFORK=y.
		 */
		ਜੇ (!funcp) {
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
			ਜੇ (sigismember(&G.pending_ਸੈੱਟ, SIGINT))
				rcode = 128 | SIGINT;
		}
		ਮੁਕਤ(argv_expanded);
		IF_HAS_KEYWORDS(ਜੇ (pi->pi_inverted) rcode = !rcode;)
		debug_leave();
		debug_printf_exec("run_pipe return %d\n", rcode);
		ਵਾਪਸ rcode;
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */
	}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
 must_fork:
	/* NB: argv_expanded may already be created, and that
	 * might include `cmd` runs! Do not rerun it! We *must*
	 * use argv_expanded if it's non-NULL */

	/* Going to fork a child per each pipe member */
	pi->alive_cmds = 0;
	ਅਗਲਾ_infd = 0;
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */

	cmd_no = 0;
	ਜਦੋਂ (cmd_no < pi->ਅੰਕ_cmds) {
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
		ਬਣਤਰ fd_pair pipefds;
#ਜੇ !BB_MMU
		ਪੂਰਨ sv_var_nest_level = G.var_nest_level;
		ਅਸਥਿਰ nommu_save_t nommu_save;
		nommu_save.old_vars = ਨਿਰਾ;
		nommu_save.argv = ਨਿਰਾ;
		nommu_save.argv_from_re_execing = ਨਿਰਾ;
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */
		command = &pi->cmds[cmd_no];
		cmd_no++;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
		/* Replace argv and argc by expanded if it exists. */
		ਜੇ (argv_expanded) {
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
			command->argc = list_ਮਾਪ(argv_expanded);
			command->argv = argv_expanded;
		}
#ਜੇ_ਅੰਤ /* __U_BOOT__ */
			ਜੇ (command->argv) {
			debug_printf_exec(": pipe member '%s' '%s'...\n",
					command->argv[0], command->argv[1]);
		} ਨਹੀਂਤਾਂ {
			debug_printf_exec(": pipe member with no argv\n");
		}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
		/* pipes are inserted between pairs of commands */
		pipefds.rd = 0;
		pipefds.wr = 1;
		ਜੇ (cmd_no < pi->ਅੰਕ_cmds)
			xpiped_pair(pipefds);

#ਜੇ ENABLE_HUSH_LINENO_VAR
		G.execute_lineno = command->lineno;
#ਜੇ_ਅੰਤ

		command->pid = BB_MMU ? fork() : vfork();
		ਜੇ (!command->pid) { /*ਬੱਚਾ*/
#ਜੇ ENABLE_HUSH_ਕੰਮ
			disable_restore_tty_pgrp_on_exit();
			CLEAR_RANDOM_T(&G.random_gen); /* or else $RANDOM repeats in child */

			/* Every child adds itself to new process group
			 * with pgid == pid_of_first_child_in_pipe */
			ਜੇ (G.ਲੜੀਵਾਰ_list_level == 1 && G_interactive_fd) {
				pid_t pgrp;
				pgrp = pi->pgrp;
				ਜੇ (pgrp < 0) /* true for 1st process only */
					pgrp = getpid();
				ਜੇ (setpgid(0, pgrp) == 0
				 && pi->followup != PIPE_BG
				 && G_saved_tty_pgrp /* we have ctty */
				) {
					/* We do it in *every* child, not just first,
					 * to avoid races */
					tcsetpgrp(G_interactive_fd, pgrp);
				}
			}
#ਜੇ_ਅੰਤ
			ਜੇ (pi->alive_cmds == 0 && pi->followup == PIPE_BG) {
				/* 1st cmd in backgrounded pipe
				 * should have its stdin /dev/null'ed */
				close(0);
				ਜੇ (open(bb_dev_null, O_RDONLY))
					xopen("/", O_RDONLY);
			} ਨਹੀਂਤਾਂ {
				xmove_fd(ਅਗਲਾ_infd, 0);
			}
			xmove_fd(pipefds.wr, 1);
			ਜੇ (pipefds.rd > 1)
				close(pipefds.rd);
			/* Like bash, explicit redirects override pipes,
			 * and the pipe fd (fd#1) is available for dup'ing:
			 * "cmd1 2>&1 | cmd2": fd#1 is duped to fd#2, thus stderr
			 * of cmd1 goes into pipe.
			 */
			ਜੇ (ਤਿਆਰੀ_redirects(command, ਨਿਰਾ)) {
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
#ਜੇ ENABLE_HUSH_FAST
		G.ਗਿਣਤੀ_SIGCHLD++;
//bb_error_msg("[%d] fork in run_pipe: G.count_SIGCHLD:%d G.handled_SIGCHLD:%d", getpid(), G.count_SIGCHLD, G.handled_SIGCHLD);
#ਜੇ_ਅੰਤ
		enable_restore_tty_pgrp_on_exit();
#ਜੇ !BB_MMU
		/* Clean up after vforked child */
		ਮੁਕਤ(nommu_save.argv);
		ਮੁਕਤ(nommu_save.argv_from_re_execing);
		G.var_nest_level = sv_var_nest_level;
		remove_nested_vars();
		ਜੋੜੋ_vars(nommu_save.old_vars);
#ਜੇ_ਅੰਤ
		ਮੁਕਤ(argv_expanded);
		argv_expanded = ਨਿਰਾ;
		ਜੇ (command->pid < 0) { /* [v]fork failed */
			/* Clearly indicate, was it fork or vfork */
			bb_simple_perror_ਸੁਨੇਹਾ(BB_MMU ? "vfork"+1 : "vfork");
		} ਨਹੀਂਤਾਂ {
			pi->alive_cmds++;
#ਜੇ ENABLE_HUSH_ਕੰਮ
			/* Second and next children need to know pid of first one */
			ਜੇ (pi->pgrp < 0)
				pi->pgrp = command->pid;
#ਜੇ_ਅੰਤ
		}

		ਜੇ (cmd_no > 1)
			close(ਅਗਲਾ_infd);
		ਜੇ (cmd_no < pi->ਅੰਕ_cmds)
			close(pipefds.wr);
		/* Pass read (output) pipe end to next iteration */
		ਅਗਲਾ_infd = pipefds.rd;
#ਨਹੀਂਤਾਂ /* __U_BOOT__ */
		/* Process the command */
		rcode = cmd_process(G.do_repeat ? CMD_FLAG_REPEAT : 0,
				    command->argc, command->argv,
				    &(G.flag_repeat), ਨਿਰਾ);

		ਜੇ (argv_expanded) {
			/*
			 * expand_strvec_to_strvec() allocates memory to expand
			 * argv, we need to free it.
			 */
			ਮੁਕਤ(argv_expanded);

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
#ਜੇ_ਅੰਤ /* __U_BOOT__ */
	}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	ਜੇ (!pi->alive_cmds) {
		debug_leave();
		debug_printf_exec("run_pipe return 1 (all forks failed, no children)\n");
		ਵਾਪਸ 1;
	}
#ਜੇ_ਅੰਤ /* __U_BOOT__ */

	debug_leave();
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	debug_printf_exec("run_pipe return -1 (%u children started)\n", pi->alive_cmds);
	ਵਾਪਸ -1;
#ਨਹੀਂਤਾਂ /* __U_BOOT__ */
	IF_HAS_KEYWORDS(ਜੇ (pi->pi_inverted) rcode = !rcode;)
	debug_printf_exec("run_pipe return %d\n", rcode);
	ਵਾਪਸ rcode;
#ਜੇ_ਅੰਤ /* __U_BOOT__ */
}

/* NB: called by pseudo_exec, and therefore must not modify any
 * global data until exec/_exit (we can be a child after vfork!) */
ਸਥਿਰ ਪੂਰਨ ਲੜੀਵਾਰ_list(ਬਣਤਰ pipe *pi)
{
#ਜੇ ENABLE_HUSH_CASE
	ਅੱਖਰ *case_ਸ਼ਬਦ = ਨਿਰਾ;
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_HUSH_LOOPS
	ਬਣਤਰ pipe *loop_ਸਿਖਰ = ਨਿਰਾ;
	ਅੱਖਰ **for_lcur = ਨਿਰਾ;
	ਅੱਖਰ **for_list = ਨਿਰਾ;
#ਜੇ_ਅੰਤ
	smallint ਆਖਰੀ_followup;
	smalluint rcode;
#ਜੇ ENABLE_HUSH_IF || ENABLE_HUSH_CASE
	smalluint cond_ਕੋਡ = 0;
#ਨਹੀਂਤਾਂ
	ਗਿਣਤੀ { cond_ਕੋਡ = 0 };
#ਜੇ_ਅੰਤ
#ਜੇ HAS_KEYWORDS
	smallint rword;      /* RES_foo */
	smallint ਆਖਰੀ_rword; /* ditto */
#ਜੇ_ਅੰਤ

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	debug_printf_exec("run_list lvl %d start\n", G.ਲੜੀਵਾਰ_list_level);
	debug_enter();
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */

	ਸੈੱਟ_G_ifs();

#ਜੇ ENABLE_HUSH_LOOPS
	/* Check syntax for "for" */
	{
		ਬਣਤਰ pipe *cpipe;
		ਲਈ (cpipe = pi; cpipe; cpipe = cpipe->ਅਗਲਾ) {
			ਜੇ (cpipe->res_ਸ਼ਬਦ != RES_FOR && cpipe->res_ਸ਼ਬਦ != RES_ਵਿੱਚ)
				ਜਾਰੀ;
			/* current word is FOR or IN (BOLD in comments below) */
			ਜੇ (cpipe->ਅਗਲਾ == ਨਿਰਾ) {
				syntax_error("malformed for");
				debug_leave();
				debug_printf_exec("run_list lvl %d return 1\n", G.ਲੜੀਵਾਰ_list_level);
				ਵਾਪਸ 1;
			}
			/* "FOR v; do ..." and "for v IN a b; do..." are ok */
			ਜੇ (cpipe->ਅਗਲਾ->res_ਸ਼ਬਦ == RES_DO)
				ਜਾਰੀ;
			/* next word is not "do". It must be "in" then ("FOR v in ...") */
			ਜੇ (cpipe->res_ਸ਼ਬਦ == RES_ਵਿੱਚ /* "for v IN a b; not_do..."? */
			 || cpipe->ਅਗਲਾ->res_ਸ਼ਬਦ != RES_ਵਿੱਚ /* FOR v not_do_and_not_in..."? */
			) {
				syntax_error("malformed for");
				debug_leave();
				debug_printf_exec("run_list lvl %d return 1\n", G.ਲੜੀਵਾਰ_list_level);
				ਵਾਪਸ 1;
			}
		}
	}
#ਜੇ_ਅੰਤ

	/* Past this point, all code paths should jump to ret: label
	 * in order to return, no direct "return" statements please.
	 * This helps to ensure that no memory is leaked. */

#ਜੇ ENABLE_HUSH_ਕੰਮ
	G.ਲੜੀਵਾਰ_list_level++;
#ਜੇ_ਅੰਤ

#ਜੇ HAS_KEYWORDS
	rword = RES_NONE;
	ਆਖਰੀ_rword = RES_XXXX;
#ਜੇ_ਅੰਤ
	ਆਖਰੀ_followup = PIPE_SEQ;
	rcode = G.ਆਖਰੀ_exitcode;

	/* Go through list of pipes, (maybe) executing them. */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	ਲਈ (; pi; pi = IF_HUSH_LOOPS(rword == RES_ਪੂਰਾ ? loop_ਸਿਖਰ : ) pi->ਅਗਲਾ) {
#ਨਹੀਂਤਾਂ /* __U_BOOT__ */
	ਲਈ (; pi; pi = rword == RES_ਪੂਰਾ ? loop_ਸਿਖਰ : pi->ਅਗਲਾ) {
#ਜੇ_ਅੰਤ /* __U_BOOT__ */
		ਪੂਰਨ r;
		ਪੂਰਨ sv_errexit_ਡੂੰਘਾਈ;

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
		ਜੇ (G.flag_SIGINT)
			ਰੋਕੋ;
		ਜੇ (G_flag_return_ਵਿੱਚ_ਤਰੱਕੀ == 1)
			ਰੋਕੋ;
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */

		IF_HAS_KEYWORDS(rword = pi->res_ਸ਼ਬਦ;)
		debug_printf_exec(": rword:%d cond_code:%d last_rword:%d\n",
				rword, cond_ਕੋਡ, ਆਖਰੀ_rword);

		sv_errexit_ਡੂੰਘਾਈ = G.errexit_ਡੂੰਘਾਈ;
		ਜੇ (
#ਜੇ ENABLE_HUSH_IF
		    rword == RES_IF || rword == RES_ELIF ||
#ਜੇ_ਅੰਤ
		    pi->followup != PIPE_SEQ
		) {
			G.errexit_ਡੂੰਘਾਈ++;
		}
#ਜੇ ENABLE_HUSH_LOOPS
		ਜੇ ((rword == RES_WHILE || rword == RES_UNTIL || rword == RES_FOR)
		 && loop_ਸਿਖਰ == ਨਿਰਾ /* avoid bumping G.depth_of_loop twice */
		) {
			/* start of a loop: remember where loop starts */
			loop_ਸਿਖਰ = pi;
			G.ਡੂੰਘਾਈ_ਦਾ_loop++;
		}
#ਜੇ_ਅੰਤ
		/* Still in the same "if...", "then..." or "do..." branch? */
		ਜੇ (IF_HAS_KEYWORDS(rword == ਆਖਰੀ_rword &&) 1) {
			ਜੇ ((rcode == 0 && ਆਖਰੀ_followup == PIPE_OR)
			 || (rcode != 0 && ਆਖਰੀ_followup == PIPE_AND)
			) {
				/* It is "<true> || CMD" or "<false> && CMD"
				 * and we should not execute CMD */
				debug_printf_exec("skipped cmd because of || or &&\n");
				ਆਖਰੀ_followup = pi->followup;
				ਜਾਓ dont_check_jobs_but_continue;
			}
		}
		ਆਖਰੀ_followup = pi->followup;
#ਜੇ ENABLE_HUSH_IF
		ਜੇ (cond_ਕੋਡ != 0) {
			ਜੇ (rword == RES_THEN) {
				/* if false; then ... fi has exitcode 0! */
				G.ਆਖਰੀ_exitcode = rcode = EXIT_SUCCESS;
				/* "if <false> THEN cmd": skip cmd */
				debug_printf_exec("skipped THEN cmd because IF condition was false\n");
				ਆਖਰੀ_rword = rword;
				ਜਾਰੀ;
			}
		} ਨਹੀਂਤਾਂ {
			ਜੇ (rword == RES_ELSE
			 || (rword == RES_ELIF && ਆਖਰੀ_rword != RES_ELIF)
			) {
				/* "if <true> then ... ELSE/ELIF cmd":
				 * skip cmd and all following ones */
				debug_printf_exec("skipped ELSE/ELIF branch because IF condition was true\n");
				ਰੋਕੋ;
			}
			//if (rword == RES_THEN): "if <true> THEN cmd", run cmd (fall through)
		}
#ਜੇ_ਅੰਤ
		IF_HAS_KEYWORDS(ਆਖਰੀ_rword = rword;)
#ਜੇ ENABLE_HUSH_LOOPS
		ਜੇ (rword == RES_FOR) { /* && pi->num_cmds - always == 1 */
			ਜੇ (!for_lcur) {
				/* first loop through for */

				ਸਥਿਰ ਅਚਲ ਅੱਖਰ encoded_dollar_at[] ALIGN1 = {
					SPECIAL_VAR_ਚਿੰਨ੍ਹ, '@' | 0x80, SPECIAL_VAR_ਚਿੰਨ੍ਹ, '\0'
				}; /* encoded representation of "$@" */
				ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਅਚਲ encoded_dollar_at_argv[] ALIGN_PTR = {
					encoded_dollar_at, ਨਿਰਾ
				}; /* argv list with one element: "$@" */
				ਅੱਖਰ **vals;

				G.ਆਖਰੀ_exitcode = rcode = EXIT_SUCCESS;
				vals = (ਅੱਖਰ**)encoded_dollar_at_argv;
				ਜੇ (pi->ਅਗਲਾ->res_ਸ਼ਬਦ == RES_ਵਿੱਚ) {
					/* if no variable values after "in" we skip "for" */
					ਜੇ (!pi->ਅਗਲਾ->cmds[0].argv) {
						debug_printf_exec(": null FOR: exitcode EXIT_SUCCESS\n");
						ਰੋਕੋ;
					}
					vals = pi->ਅਗਲਾ->cmds[0].argv;
				} /* else: "for var; do..." -> assume "$@" list */
				/* create list of variable values */
				debug_print_ਲੜੀਆਂ("for_list made from", vals);
				for_list = expand_strvec_ਮੰਜ਼ਿਲ_strvec(vals);
				for_lcur = for_list;
				debug_print_ਲੜੀਆਂ("for_list", for_list);
			}
			ਜੇ (!*for_lcur) {
				/* "for" loop is over, clean up */
				ਮੁਕਤ(for_list);
				for_list = ਨਿਰਾ;
				for_lcur = ਨਿਰਾ;
				ਰੋਕੋ;
			}
			/* Insert next value from for_lcur */
			/* note: *for_lcur already has quotes removed, $var expanded, etc */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
			ਸੈੱਟ_local_var_from_halves(pi->cmds[0].argv[0], *for_lcur++);
#ਨਹੀਂਤਾਂ /* __U_BOOT__ */
			/* We cannot use xasprintf, so we emulate it. */
			ਅੱਖਰ *ਭਰਿਆ_var;
			ਅੱਖਰ *var = pi->cmds[0].argv[0];
			ਅੱਖਰ *val = *for_lcur++;

			/* + 1 to take into account =. */
			ਭਰਿਆ_var = xmalloc(ਲੜੀ_ਲੰਬਾਈ(var) + ਲੜੀ_ਲੰਬਾਈ(val) + 1);
			sprintf(ਭਰਿਆ_var, "%s=%s", var, val);

			ਸੈੱਟ_local_var_modern(ਭਰਿਆ_var, /*flag:*/ 0);
#ਜੇ_ਅੰਤ /* __U_BOOT__ */
			ਜਾਰੀ;
		}
		ਜੇ (rword == RES_ਵਿੱਚ) {
			ਜਾਰੀ; /* "for v IN list;..." - "in" has no cmds anyway */
		}
		ਜੇ (rword == RES_ਪੂਰਾ) {
			ਜਾਰੀ; /* "done" has no cmds too */
		}
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_HUSH_CASE
		ਜੇ (rword == RES_CASE) {
			debug_printf_exec("CASE cond_code:%d\n", cond_ਕੋਡ);
			case_ਸ਼ਬਦ = expand_string_ਮੰਜ਼ਿਲ_string(pi->cmds->argv[0],
				EXP_FLAG_ESC_GLOB_CHARS, /*unbackslash:*/ 1);
			debug_printf_exec("CASE word1:'%s'\n", case_ਸ਼ਬਦ);
			//unbackslash(case_word);
			//debug_printf_exec("CASE word2:'%s'\n", case_word);
			ਜਾਰੀ;
		}
		ਜੇ (rword == RES_ਮੇਲ) {
			ਅੱਖਰ **argv;

			debug_printf_exec("MATCH cond_code:%d\n", cond_ਕੋਡ);
			ਜੇ (!case_ਸ਼ਬਦ) /* "case ... matched_word) ... WORD)": we executed selected branch, stop */
				ਰੋਕੋ;
			/* all prev words didn't match, does this one match? */
			argv = pi->cmds->argv;
			ਜਦੋਂ (*argv) {
				ਅੱਖਰ *ਨਮੂਨਾ;
				debug_printf_exec("expand_string_to_string('%s')\n", *argv);
				ਨਮੂਨਾ = expand_string_ਮੰਜ਼ਿਲ_string(*argv,
						EXP_FLAG_ESC_GLOB_CHARS,
						/*unbackslash:*/ 0
				);
				/* TODO: which FNM_xxx flags to use? */
				cond_ਕੋਡ = (fnmatch(ਨਮੂਨਾ, case_ਸ਼ਬਦ, /*flags:*/ 0) != 0);
				debug_printf_exec("cond_code=fnmatch(pattern:'%s',str:'%s'):%d\n",
						ਨਮੂਨਾ, case_ਸ਼ਬਦ, cond_ਕੋਡ);
				ਮੁਕਤ(ਨਮੂਨਾ);
				ਜੇ (cond_ਕੋਡ == 0) {
					/* match! we will execute this branch */
					ਮੁਕਤ(case_ਸ਼ਬਦ);
					case_ਸ਼ਬਦ = ਨਿਰਾ; /* make future "word)" stop */
					ਰੋਕੋ;
				}
				argv++;
			}
			ਜਾਰੀ;
		}
		ਜੇ (rword == RES_CASE_BODY) { /* inside of a case branch */
			debug_printf_exec("CASE_BODY cond_code:%d\n", cond_ਕੋਡ);
			ਜੇ (cond_ਕੋਡ != 0)
				ਜਾਰੀ; /* not matched yet, skip this pipe */
		}
		ਜੇ (rword == RES_ESAC) {
			debug_printf_exec("ESAC cond_code:%d\n", cond_ਕੋਡ);
			ਜੇ (case_ਸ਼ਬਦ) {
				/* "case" did not match anything: still set $? (to 0) */
				G.ਆਖਰੀ_exitcode = rcode = EXIT_SUCCESS;
			}
		}
#ਜੇ_ਅੰਤ
		/* Just pressing <enter> in shell should check for jobs.
		 * OTOH, in non-interactive shell this is useless
		 * and only leads to extra job checks */
		ਜੇ (pi->ਅੰਕ_cmds == 0) {
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
			ਜੇ (G_interactive_fd)
				ਜਾਓ check_jobs_and_continue;
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */
			ਜਾਰੀ;
		}

		/* After analyzing all keywords and conditions, we decided
		 * to execute this pipe. NB: have to do checkjobs(NULL)
		 * after run_pipe to collect any background children,
		 * even if list execution is to be stopped. */
		debug_printf_exec(": run_pipe with %d members\n", pi->ਅੰਕ_cmds);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
#ਜੇ ENABLE_HUSH_LOOPS
		G.flag_break_continue = 0;
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
		rcode = r = G.o_opt[OPT_O_NOEXEC] ? 0 : ਲੜੀਵਾਰ_pipe(pi);
		/* NB: rcode is a smalluint, r is int */
#ਨਹੀਂਤਾਂ /* __U_BOOT__ */
		rcode = r = ਲੜੀਵਾਰ_pipe(pi); /* NB: rcode is a smalluint, r is int */
		ਜੇ (r <= EXIT_RET_ਕੋਡ) {
			ਪੂਰਨ ਪਿਛਲਾ_rcode = G.ਆਖਰੀ_exitcode;
			/*
			 * This magic is to get the exit code given by the user.
			 * Contrary to old shell code, we use + EXIT_RET_CODE as EXIT_RET_CODE
			 * equals -2.
			 */
			G.ਆਖਰੀ_exitcode = -r + EXIT_RET_ਕੋਡ;

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
			ਜੇ (ਪਿਛਲਾ_rcode != r) {
				/*
				 * If run from run_command, run_command_flags will be set, so we check
				 * this to know if we are in main input shell.
				 */
				ਜੇ (!G.ਲੜੀਵਾਰ_command_flags)
					ਲਿਖੋ("exit not allowed from main input shell.\n");

				ਰੋਕੋ;
			}
			ਜਾਰੀ;
		}
#ਜੇ_ਅੰਤ /* __U_BOOT__ */
		ਜੇ (r != -1) {
			/* We ran a builtin, function, or group.
			 * rcode is already known
			 * and we don't need to wait for anything. */
			debug_printf_exec(": builtin/func exitcode %d\n", rcode);
			G.ਆਖਰੀ_exitcode = rcode;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
			check_and_ਲੜੀਵਾਰ_traps();
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */
#ਜੇ ENABLE_HUSH_TRAP && ENABLE_HUSH_FUNCTIONS
			rcode = G.ਆਖਰੀ_exitcode; /* "return" in trap can change it, read back */
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
#ਜੇ ENABLE_HUSH_LOOPS
			/* Was it "break" or "continue"? */
			ਜੇ (G.flag_break_continue) {
				smallint fbc = G.flag_break_continue;
				/* We might fall into outer *loop*,
				 * don't want to break it too */
				ਜੇ (loop_ਸਿਖਰ) {
					G.ਡੂੰਘਾਈ_break_continue--;
					ਜੇ (G.ਡੂੰਘਾਈ_break_continue == 0)
						G.flag_break_continue = 0;
					/* else: e.g. "continue 2" should *break* once, *then* continue */
				} /* else: "while... do... { we are here (innermost list is not a loop!) };...done" */
				ਜੇ (G.ਡੂੰਘਾਈ_break_continue != 0 || fbc == BC_BREAK) {
					checkjobs(ਨਿਰਾ, 0 /*(no pid to wait for)*/);
					ਰੋਕੋ;
				}
				/* "continue": simulate end of loop */
				rword = RES_ਪੂਰਾ;
				ਜਾਰੀ;
			}
#ਜੇ_ਅੰਤ
			ਜੇ (G_flag_return_ਵਿੱਚ_ਤਰੱਕੀ == 1) {
				checkjobs(ਨਿਰਾ, 0 /*(no pid to wait for)*/);
				ਰੋਕੋ;
			}

		} ਨਹੀਂਤਾਂ ਜੇ (pi->followup == PIPE_BG) {
			/* What does bash do with attempts to background builtins? */
			/* even bash 3.2 doesn't do that well with nested bg:
			 * try "{ { sleep 10; echo DEEP; } & echo HERE; } &".
			 * I'm NOT treating inner &'s as jobs */
#ਜੇ ENABLE_HUSH_ਕੰਮ
			ਜੇ (G.ਲੜੀਵਾਰ_list_level == 1)
				ਪਾਓ_ਕੰਮ_into_ਸਾਰਣੀ(pi);
#ਜੇ_ਅੰਤ
			/* Last command's pid goes to $! */
			G.ਆਖਰੀ_bg_pid = pi->cmds[pi->ਅੰਕ_cmds - 1].pid;
			G.ਆਖਰੀ_bg_pid_exitcode = 0;
			debug_printf_exec(": cmd&: exitcode EXIT_SUCCESS\n");
/* Check pi->pi_inverted? "! sleep 1 & echo $?": bash says 1. dash and ash say 0 */
			rcode = EXIT_SUCCESS;
			ਜਾਓ check_traps;
		} ਨਹੀਂਤਾਂ {
#ਜੇ ENABLE_HUSH_ਕੰਮ
			ਜੇ (G.ਲੜੀਵਾਰ_list_level == 1 && G_interactive_fd) {
				/* Waits for completion, then fg's main shell */
				rcode = checkjobs_and_fg_shell(pi);
				debug_printf_exec(": checkjobs_and_fg_shell exitcode %d\n", rcode);
				ਜਾਓ check_traps;
			}
#ਜੇ_ਅੰਤ
			/* This one just waits for completion */
			rcode = checkjobs(pi, 0 /*(no pid to wait for)*/);
			debug_printf_exec(": checkjobs exitcode %d\n", rcode);
 check_traps:
			G.ਆਖਰੀ_exitcode = rcode;
			check_and_ਲੜੀਵਾਰ_traps();
#ਜੇ ENABLE_HUSH_TRAP && ENABLE_HUSH_FUNCTIONS
			rcode = G.ਆਖਰੀ_exitcode; /* "return" in trap can change it, read back */
#ਜੇ_ਅੰਤ
		}
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
		/* Handle "set -e" */
		ਜੇ (rcode != 0 && G.o_opt[OPT_O_ERREXIT]) {
			debug_printf_exec("ERREXIT:1 errexit_depth:%d\n", G.errexit_ਡੂੰਘਾਈ);
			ਜੇ (G.errexit_ਡੂੰਘਾਈ == 0)
				hush_exit(rcode);
		}
#ਨਹੀਂਤਾਂ /* __U_BOOT__ */
		} /* if (r != -1) */
#ਜੇ_ਅੰਤ /* __U_BOOT__ */
		G.errexit_ਡੂੰਘਾਈ = sv_errexit_ਡੂੰਘਾਈ;

		/* Analyze how result affects subsequent commands */
#ਜੇ ENABLE_HUSH_IF
		ਜੇ (rword == RES_IF || rword == RES_ELIF) {
			debug_printf_exec("cond_code=rcode:%d\n", rcode);
			cond_ਕੋਡ = rcode;
		}
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
 check_jobs_and_continue:
		checkjobs(ਨਿਰਾ, 0 /*(no pid to wait for)*/);
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */
 dont_check_jobs_but_continue: ;
#ਜੇ ENABLE_HUSH_LOOPS
		/* Beware of "while false; true; do ..."! */
		ਜੇ (pi->ਅਗਲਾ
		 && (pi->ਅਗਲਾ->res_ਸ਼ਬਦ == RES_DO || pi->ਅਗਲਾ->res_ਸ਼ਬਦ == RES_ਪੂਰਾ)
		 /* check for RES_DONE is needed for "while ...; do \n done" case */
		) {
			ਜੇ (rword == RES_WHILE) {
				ਜੇ (rcode) {
					/* "while false; do...done" - exitcode 0 */
					G.ਆਖਰੀ_exitcode = rcode = EXIT_SUCCESS;
					debug_printf_exec(": while expr is false: breaking (exitcode:EXIT_SUCCESS)\n");
					ਰੋਕੋ;
				}
			}
			ਜੇ (rword == RES_UNTIL) {
				ਜੇ (!rcode) {
					debug_printf_exec(": until expr is true: breaking\n");
					ਰੋਕੋ;
				}
			}
		}
#ਜੇ_ਅੰਤ
	} /* for (pi) */

#ਜੇ ENABLE_HUSH_ਕੰਮ
	G.ਲੜੀਵਾਰ_list_level--;
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_HUSH_LOOPS
	ਜੇ (loop_ਸਿਖਰ)
		G.ਡੂੰਘਾਈ_ਦਾ_loop--;
	ਮੁਕਤ(for_list);
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_HUSH_CASE
	ਮੁਕਤ(case_ਸ਼ਬਦ);
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	debug_leave();
	debug_printf_exec("run_list lvl %d return %d\n", G.ਲੜੀਵਾਰ_list_level, rcode);
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */
	ਵਾਪਸ rcode;
}

/* Select which version we will use */
ਸਥਿਰ ਪੂਰਨ ਲੜੀਵਾਰ_and_free_list(ਬਣਤਰ pipe *pi)
{
	ਪੂਰਨ rcode = 0;
	debug_printf_exec("run_and_free_list entered\n");
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	ਜੇ (!G.o_opt[OPT_O_NOEXEC]) {
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */
		debug_printf_exec(": run_list: 1st pipe with %d cmds\n", pi->ਅੰਕ_cmds);
		rcode = ਲੜੀਵਾਰ_list(pi);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	}
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */
	/* free_pipe_list has the side effect of clearing memory.
	 * In the long run that function can be merged with run_list,
	 * but doing that now would hobble the debugging effort. */
	free_pipe_list(pi);
	debug_printf_exec("run_and_free_list return %d\n", rcode);
	ਵਾਪਸ rcode;
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
ਸਥਿਰ ਖਾਲੀ install_sighandlers(ਬਿਨਾਨਿਸ਼ਾਨੀ ਨਕਾਬ)
{
	sighandler_t old_handler;
	ਬਿਨਾਨਿਸ਼ਾਨੀ sig = 0;
	ਜਦੋਂ ((ਨਕਾਬ >>= 1) != 0) {
		sig++;
		ਜੇ (!(ਨਕਾਬ & 1))
			ਜਾਰੀ;
		old_handler = install_sighandler(sig, pick_sighandler(sig));
		/* POSIX allows shell to re-enable SIGCHLD
		 * even if it was SIG_IGN on entry.
		 * Therefore we skip IGN check for it:
		 */
		ਜੇ (sig == SIGCHLD)
			ਜਾਰੀ;
		/* Interactive bash re-enables SIGHUP which is SIG_IGNed on entry.
		 * Try:
		 * trap '' hup; bash; echo RET  # type "kill -hup $$", see SIGHUP having effect
		 * trap '' hup; bash -c 'kill -hup $$; echo ALIVE'  # here SIGHUP is SIG_IGNed
		 */
		ਜੇ (sig == SIGHUP && G_interactive_fd)
			ਜਾਰੀ;
		/* Unless one of the above signals, is it SIG_IGN? */
		ਜੇ (old_handler == SIG_IGN) {
			/* oops... restore back to IGN, and record this fact */
			install_sighandler(sig, old_handler);
#ਜੇ ENABLE_HUSH_TRAP
			ਜੇ (!G_traps)
				G_traps = xzalloc(ਆਕਾਰ(G_traps[0]) * NSIG);
			ਮੁਕਤ(G_traps[sig]);
			G_traps[sig] = xzalloc(1); /* == xstrdup(""); */
#ਜੇ_ਅੰਤ
		}
	}
}

/* Called a few times only (or even once if "sh -c") */
ਸਥਿਰ ਖਾਲੀ install_special_sighandlers(ਖਾਲੀ)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਨਕਾਬ;

	/* Which signals are shell-special? */
	ਨਕਾਬ = (1 << SIGQUIT) | (1 << SIGCHLD);
	ਜੇ (G_interactive_fd) {
		ਨਕਾਬ |= SPECIAL_INTERACTIVE_SIGS;
		ਜੇ (G_saved_tty_pgrp) /* we have ctty, job control sigs work */
			ਨਕਾਬ |= SPECIAL_JOBSTOP_SIGS;
	}
	/* Careful, do not re-install handlers we already installed */
	ਜੇ (G.special_sig_ਨਕਾਬ != ਨਕਾਬ) {
		ਬਿਨਾਨਿਸ਼ਾਨੀ diff = ਨਕਾਬ & ~G.special_sig_ਨਕਾਬ;
		G.special_sig_ਨਕਾਬ = ਨਕਾਬ;
		install_sighandlers(diff);
	}
}

#ਜੇ ENABLE_HUSH_ਕੰਮ
/* helper */
/* Set handlers to restore tty pgrp and exit */
ਸਥਿਰ ਖਾਲੀ install_fatal_sighandlers(ਖਾਲੀ)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਨਕਾਬ;

	/* We will restore tty pgrp on these signals */
	ਨਕਾਬ = 0
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
	G_fatal_sig_ਨਕਾਬ = ਨਕਾਬ;

	install_sighandlers(ਨਕਾਬ);
}
#ਜੇ_ਅੰਤ

ਸਥਿਰ ਪੂਰਨ ਸੈੱਟ_ਬਹੁਲਕ(ਪੂਰਨ state, ਅੱਖਰ ਬਹੁਲਕ, ਅਚਲ ਅੱਖਰ *o_opt)
{
	ਪੂਰਨ idx;
	ਚੋਣ (ਬਹੁਲਕ) {
	ਹਾਲਤ 'n':
		/* set -n has no effect in interactive shell */
		/* Try: while set -n; do echo $-; done */
		ਜੇ (!G_interactive_fd)
			G.o_opt[OPT_O_NOEXEC] = state;
		ਰੋਕੋ;
	ਹਾਲਤ 'x':
		IF_HUSH_ਬਹੁਲਕ_X(G_x_ਬਹੁਲਕ = state;)
		IF_HUSH_ਬਹੁਲਕ_X(ਜੇ (G.x_ਬਹੁਲਕ_fd <= 0) G.x_ਬਹੁਲਕ_fd = dup_CLOEXEC(2, 10);)
		ਰੋਕੋ;
	ਹਾਲਤ 'e':
		G.o_opt[OPT_O_ERREXIT] = state;
		ਰੋਕੋ;
	ਹਾਲਤ 'o':
		ਜੇ (!o_opt) {
			/* "set -o" or "set +o" without parameter.
			 * in bash, set -o produces this output:
			 *  pipefail        off
			 * and set +o:
			 *  set +o pipefail
			 * We always use the second form.
			 */
			ਅਚਲ ਅੱਖਰ *p = o_opt_ਲੜੀਆਂ;
			idx = 0;
			ਜਦੋਂ (*p) {
				ਲਿਖੋ("set %co %s\n", (G.o_opt[idx] ? '-' : '+'), p);
				idx++;
				p += ਲੜੀ_ਲੰਬਾਈ(p) + 1;
			}
			ਰੋਕੋ;
		}
		idx = ਸੂਚਕਾਂਕ_ਵਿੱਚ_ਲੜੀਆਂ(o_opt_ਲੜੀਆਂ, o_opt);
		ਜੇ (idx >= 0) {
			G.o_opt[idx] = state;
			ਰੋਕੋ;
		}
		/* fall through to error */
	ਮੂਲ:
		ਵਾਪਸ EXIT_FAILURE;
	}
	ਵਾਪਸ EXIT_SUCCESS;
}

ਪੂਰਨ hush_main(ਪੂਰਨ argc, ਅੱਖਰ **argv) MAIN_EXTERNALLY_VISIBLE;
ਪੂਰਨ hush_main(ਪੂਰਨ argc, ਅੱਖਰ **argv)
{
	pid_t cached_getpid;
	ਗਿਣਤੀ {
		OPT_login = (1 << 0),
	};
	ਬਿਨਾਨਿਸ਼ਾਨੀ flags;
#ਜੇ !BB_MMU
	ਬਿਨਾਨਿਸ਼ਾਨੀ builtin_argc = 0;
#ਜੇ_ਅੰਤ
	ਅੱਖਰ **e;
	ਬਣਤਰ variable *cur_var;
	ਬਣਤਰ variable *shell_ver;

	INIT_G();
	ਜੇ (EXIT_SUCCESS != 0) /* if EXIT_SUCCESS == 0, it is already done */
		G.ਆਖਰੀ_exitcode = EXIT_SUCCESS;
#ਜੇ !BB_MMU
	/* "Big heredoc" support via "sh -< STRING" invocation.
	 * Check it first (do not bother to run the usual init code,
	 * it is not needed for this case).
	 */
	ਜੇ (argv[1]
	 && argv[1][0] == '-' && argv[1][1] == '<' /*&& !argv[1][2]*/
	 /*&& argv[2] && !argv[3] - we don't check some conditions */
	) {
		ਭਰਿਆ_write1_str(argv[2]);
		_exit(0);
	}
	G.argv0_for_re_execing = argv[0];
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_HUSH_TRAP
# ਜੇ ENABLE_HUSH_FUNCTIONS
	G.return_exitcode = -1;
# endif
	G.pre_trap_exitcode = -1;
#ਜੇ_ਅੰਤ

#ਜੇ ENABLE_HUSH_FAST
	G.ਗਿਣਤੀ_SIGCHLD++; /* ensure it is != G.handled_SIGCHLD */
#ਜੇ_ਅੰਤ

	cached_getpid = getpid();   /* for tcsetpgrp() during init */
	G.ਜੜ੍ਹ_pid = cached_getpid; /* for $PID  (NOMMU can override via -$HEXPID:HEXPPID:...) */
	G.ਜੜ੍ਹ_ppid = getppid();    /* for $PPID (NOMMU can override) */

	/* Deal with HUSH_VERSION */
	debug_printf_env("unsetenv '%s'\n", "HUSH_VERSION");
	unsetenv("HUSH_VERSION"); /* in case it exists in initial env */
	shell_ver = xzalloc(ਆਕਾਰ(*shell_ver));
	shell_ver->flg_export = 1;
	shell_ver->flg_read_only = 1;
	/* Code which handles ${var<op>...} needs writable values for all variables,
	 * therefore we xstrdup: */
	shell_ver->varstr = xstrdup(hush_ਸੰਸਕਰਣ_str);

	/* Create shell local variables from the values
	 * currently living in the environment */
	G.ਸਿਖਰ_var = shell_ver;
	cur_var = G.ਸਿਖਰ_var;
	e = environ;
	ਜੇ (e) ਜਦੋਂ (*e) {
		ਅੱਖਰ *ਮੁੱਲ = strchr(*e, '=');
		ਜੇ (ਮੁੱਲ) { /* paranoia */
			cur_var->ਅਗਲਾ = xzalloc(ਆਕਾਰ(*cur_var));
			cur_var = cur_var->ਅਗਲਾ;
			cur_var->varstr = *e;
			cur_var->max_len = ਲੜੀ_ਲੰਬਾਈ(*e);
			cur_var->flg_export = 1;
		}
		e++;
	}
	/* (Re)insert HUSH_VERSION into env (AFTER we scanned the env!) */
	debug_printf_env("putenv '%s'\n", shell_ver->varstr);
	putenv(shell_ver->varstr);

	/* Export PWD */
	ਸੈੱਟ_pwd_var(SETFLAG_EXPORT);

#ਜੇ BASH_HOSTNAME_VAR
	/* Set (but not export) HOSTNAME unless already set */
	ਜੇ (!get_local_var_ਮੁੱਲ("HOSTNAME")) {
		ਬਣਤਰ utsname uts;
		uname(&uts);
		ਸੈੱਟ_local_var_from_halves("HOSTNAME", uts.nodename);
	}
#ਜੇ_ਅੰਤ
	/* IFS is not inherited from the parent environment */
	ਸੈੱਟ_local_var_from_halves("IFS", defifs);

	ਜੇ (!get_local_var_ਮੁੱਲ("PATH"))
		ਸੈੱਟ_local_var_from_halves("PATH", bb_default_ਜੜ੍ਹ_ਰਸਤਾ);

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

#ਜੇ ਅੰਕ_SCRIPTS > 0
	ਜੇ (argc < 0) {
		ਅੱਖਰ *script = get_script_content(-argc - 1);
		G.global_argv = argv;
		G.global_argc = string_array_len(argv);
		//install_special_sighandlers(); - needed?
		parse_and_ਲੜੀਵਾਰ_string(script);
		ਜਾਓ final_return;
	}
#ਜੇ_ਅੰਤ

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
	ਜਦੋਂ (1) {
		ਪੂਰਨ opt = getopt(argc, argv, "+" /* stop at 1st non-option */
				"cexinsl"
#ਜੇ !BB_MMU
				"$:R:V:"
# ਜੇ ENABLE_HUSH_LINENO_VAR
				"L:"
# endif
# ਜੇ ENABLE_HUSH_FUNCTIONS
				"F:"
# endif
#ਜੇ_ਅੰਤ
		);
		ਜੇ (opt <= 0)
			ਰੋਕੋ;
		ਚੋਣ (opt) {
		ਹਾਲਤ 'c':
			/* Note: -c is not an option with param!
			 * "hush -c -l SCRIPT" is valid. "hush -cSCRIPT" is not.
			 */
			G.opt_c = 1;
			ਰੋਕੋ;
		ਹਾਲਤ 'i':
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
//ਇੱਥੇ
//(under "bash", it's the opposite: it runs $BASH_ENV file only *without* -i).
//
//ash -i -c 'sleep 3; sleep 3', on ^C, drops into a prompt instead of exiting
//(this may be a bug, bash does not do this).
//(ash -i -c 'sleep 3' won't show this, the last command gets auto-"exec"ed)
//
//None of the above feel like useful features people would rely on.
			ਰੋਕੋ;
		ਹਾਲਤ 's':
			G.opt_s = 1;
			ਰੋਕੋ;
		ਹਾਲਤ 'l':
			flags |= OPT_login;
			ਰੋਕੋ;
#ਜੇ !BB_MMU
		ਹਾਲਤ '$': {
			ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਲੰਮਾ ਖਾਲੀ_ਥਾਵਾਂ_trap_ਨਕਾਬ;

			G.ਜੜ੍ਹ_pid = bb_strtou(optarg, &optarg, 16);
			optarg++;
			G.ਜੜ੍ਹ_ppid = bb_strtou(optarg, &optarg, 16);
			optarg++;
			G.ਆਖਰੀ_bg_pid = bb_strtou(optarg, &optarg, 16);
			optarg++;
			G.ਆਖਰੀ_exitcode = bb_strtou(optarg, &optarg, 16);
			optarg++;
			builtin_argc = bb_strtou(optarg, &optarg, 16);
			optarg++;
			ਖਾਲੀ_ਥਾਵਾਂ_trap_ਨਕਾਬ = bb_strtoull(optarg, &optarg, 16);
			ਜੇ (ਖਾਲੀ_ਥਾਵਾਂ_trap_ਨਕਾਬ != 0) {
				IF_HUSH_TRAP(ਪੂਰਨ sig;)
				install_special_sighandlers();
# ਜੇ ENABLE_HUSH_TRAP
				G_traps = xzalloc(ਆਕਾਰ(G_traps[0]) * NSIG);
				ਲਈ (sig = 1; sig < NSIG; sig++) {
					ਜੇ (ਖਾਲੀ_ਥਾਵਾਂ_trap_ਨਕਾਬ & (1LL << sig)) {
						G_traps[sig] = xzalloc(1); /* == xstrdup(""); */
						install_sighandler(sig, SIG_IGN);
					}
				}
# endif
			}
# ਜੇ ENABLE_HUSH_LOOPS
			optarg++;
			G.ਡੂੰਘਾਈ_ਦਾ_loop = bb_strtou(optarg, &optarg, 16);
# endif
			/* Suppress "killed by signal" message, -$ hack is used
			 * for subshells: echo `sh -c 'kill -9 $$'`
			 * should be silent.
			 */
			IF_HUSH_ਕੰਮ(G.ਲੜੀਵਾਰ_list_level = 1;)
# ਜੇ ENABLE_HUSH_FUNCTIONS
			/* nommu uses re-exec trick for "... | func | ...",
			 * should allow "return".
			 * This accidentally allows returns in subshells.
			 */
			G_flag_return_ਵਿੱਚ_ਤਰੱਕੀ = -1;
# endif
			ਰੋਕੋ;
		}
		ਹਾਲਤ 'R':
		ਹਾਲਤ 'V':
			ਸੈੱਟ_local_var(xstrdup(optarg), opt == 'R' ? SETFLAG_MAKE_RO : 0);
			ਰੋਕੋ;
# ਜੇ ENABLE_HUSH_LINENO_VAR
		ਹਾਲਤ 'L':
			G.parse_lineno = xatou(optarg);
			ਰੋਕੋ;
# endif
# ਜੇ ENABLE_HUSH_FUNCTIONS
		ਹਾਲਤ 'F': {
			ਬਣਤਰ function *funcp = new_function(optarg);
			/* funcp->name is already set to optarg */
			/* funcp->body is set to NULL. It's a special case. */
			funcp->body_as_string = argv[optind];
			optind++;
			ਰੋਕੋ;
		}
# endif
#ਜੇ_ਅੰਤ
		/*case '?': invalid option encountered (set_mode('?') will fail) */
		/*case 'n':*/
		/*case 'x':*/
		/*case 'e':*/
		ਮੂਲ:
			ਜੇ (ਸੈੱਟ_ਬਹੁਲਕ(1, opt, ਨਿਰਾ) == 0) /* no error */
				ਰੋਕੋ;
			bb_show_usage();
		}
	} /* option parsing loop */

	/* Skip options. Try "hush -l": $1 should not be "-l"! */
	G.global_argc = argc - (optind - 1);
	G.global_argv = argv + (optind - 1);
	G.global_argv[0] = argv[0];

	/* If we are login shell... */
	ਜੇ (flags & OPT_login) {
		ਅਚਲ ਅੱਖਰ *hp = ਨਿਰਾ;
		HFILE *ਨਿਵੇਸ਼;

		debug_printf("sourcing /etc/profile\n");
		ਨਿਵੇਸ਼ = hfopen("/etc/profile");
 ਲੜੀਵਾਰ_profile:
		ਜੇ (ਨਿਵੇਸ਼ != ਨਿਰਾ) {
			install_special_sighandlers();
			parse_and_ਲੜੀਵਾਰ_file(ਨਿਵੇਸ਼);
			hfclose(ਨਿਵੇਸ਼);
		}
		/* bash: after sourcing /etc/profile,
		 * tries to source (in the given order):
		 * ~/.bash_profile, ~/.bash_login, ~/.profile,
		 * stopping on first found. --noprofile turns this off.
		 * bash also sources ~/.bash_logout on exit.
		 * If called as sh, skips .bash_XXX files.
		 */
		ਜੇ (!hp) { /* unless we looped on the "goto" already */
			hp = get_local_var_ਮੁੱਲ("HOME");
			ਜੇ (hp && hp[0]) {
				debug_printf("sourcing ~/.profile\n");
				hp = concat_ਰਸਤਾ_file(hp, ".profile");
				ਨਿਵੇਸ਼ = hfopen(hp);
				ਮੁਕਤ((ਅੱਖਰ*)hp);
				ਜਾਓ ਲੜੀਵਾਰ_profile;
			}
		}
	}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	/* -c takes effect *after* -l */
	ਜੇ (G.opt_c) {
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
		ਅੱਖਰ *script;

		install_special_sighandlers();

		G.global_argc--;
		G.global_argv++;
#ਜੇ !BB_MMU
		ਜੇ (builtin_argc) {
			/* -c 'builtin' [BARGV...] "" ARG0 [ARG1...] */
			ਅਚਲ ਬਣਤਰ built_ਵਿੱਚ_command *x;
			x = ਲੱਭੋ_builtin(G.global_argv[0]);
			ਜੇ (x) { /* paranoia */
				argv = G.global_argv;
				G.global_argc -= builtin_argc + 1; /* skip [BARGV...] "" */
				G.global_argv += builtin_argc + 1;
				G.global_argv[-1] = ਨਿਰਾ; /* replace "" */
				G.ਆਖਰੀ_exitcode = x->b_function(argv);
			}
			ਜਾਓ final_return;
		}
#ਜੇ_ਅੰਤ

		script = G.global_argv[0];
		ਜੇ (!script)
			bb_error_ਸੁਨੇਹਾ_and_die(bb_ਸੁਨੇਹਾ_requires_arg, "-c");
		ਜੇ (!G.global_argv[1]) {
			/* -c 'script' (no params): prevent empty $0 */
			G.global_argv[0] = argv[0];
		} ਨਹੀਂਤਾਂ { /* else -c 'script' ARG0 [ARG1...]: $0 is ARG0 */
			G.global_argc--;
			G.global_argv++;
		}
		parse_and_ਲੜੀਵਾਰ_string(script);
		ਜਾਓ final_return;
	}

	/* -s is: hush -s ARGV1 ARGV2 (no SCRIPT) */
	ਜੇ (!G.opt_s && G.global_argv[1]) {
		HFILE *ਨਿਵੇਸ਼;
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
		ਨਿਵੇਸ਼ = hfopen(G.global_argv[0]);
		ਜੇ (!ਨਿਵੇਸ਼) {
			bb_simple_perror_ਸੁਨੇਹਾ_and_die(G.global_argv[0]);
		}
		xfunc_error_retval = 1;
		install_special_sighandlers();
		parse_and_ਲੜੀਵਾਰ_file(ਨਿਵੇਸ਼);
#ਜੇ ENABLE_FEATURE_CLEAN_ਉੱਪਰ
		hfclose(ਨਿਵੇਸ਼);
#ਜੇ_ਅੰਤ
		ਜਾਓ final_return;
	}
	/* "implicit" -s: bare interactive hush shows 's' in $- */
	G.opt_s = 1;

#ਜੇ_ਅੰਤ /* __U_BOOT__ */
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
#ਜੇ ENABLE_HUSH_ਕੰਮ
	ਜੇ (isatty(STDIN_FILENO) && isatty(STDOUT_FILENO)) {
		G_saved_tty_pgrp = tcgetpgrp(STDIN_FILENO);
		debug_printf("saved_tty_pgrp:%d\n", G_saved_tty_pgrp);
		ਜੇ (G_saved_tty_pgrp < 0)
			G_saved_tty_pgrp = 0;

		/* try to dup stdin to high fd#, >= 255 */
		G_interactive_fd = dup_CLOEXEC(STDIN_FILENO, 254);
		ਜੇ (G_interactive_fd < 0) {
			/* try to dup to any fd */
			G_interactive_fd = dup_CLOEXEC(STDIN_FILENO, -1);
			ਜੇ (G_interactive_fd < 0) {
				/* give up */
				G_interactive_fd = 0;
				G_saved_tty_pgrp = 0;
			}
		}
	}
	debug_printf("interactive_fd:%d\n", G_interactive_fd);
	ਜੇ (G_interactive_fd) {
		ਜੇ (G_saved_tty_pgrp) {
			/* If we were run as 'hush &', sleep until we are
			 * in the foreground (tty pgrp == our pgrp).
			 * If we get started under a job aware app (like bash),
			 * make sure we are now in charge so we don't fight over
			 * who gets the foreground */
			ਜਦੋਂ (1) {
				pid_t shell_pgrp = getpgrp();
				G_saved_tty_pgrp = tcgetpgrp(G_interactive_fd);
				ਜੇ (G_saved_tty_pgrp == shell_pgrp)
					ਰੋਕੋ;
				/* send TTIN to ourself (should stop us) */
				kill(- shell_pgrp, SIGTTIN);
			}
		}

		/* Install more signal handlers */
		install_special_sighandlers();

		ਜੇ (G_saved_tty_pgrp) {
			/* Set other signals to restore saved_tty_pgrp */
			install_fatal_sighandlers();
			/* Put ourselves in our own process group
			 * (bash, too, does this only if ctty is available) */
			bb_setpgrp(); /* is the same as setpgid(our_pid, our_pid); */
			/* Grab control of the terminal */
			tcsetpgrp(G_interactive_fd, cached_getpid);
		}
		enable_restore_tty_pgrp_on_exit();

# ਜੇ ENABLE_FEATURE_EDITING
		G.ਸਤਰ_ਨਿਵੇਸ਼_state = new_ਸਤਰ_ਨਿਵੇਸ਼_t(FOR_SHELL);
#  ਜੇ ENABLE_FEATURE_TAB_COMPLETION
		G.ਸਤਰ_ਨਿਵੇਸ਼_state->get_exe_name = hush_command_name;
#  endif
#  ਜੇ EDITING_HAS_sh_get_var
		G.ਸਤਰ_ਨਿਵੇਸ਼_state->sh_get_var = get_local_var_ਮੁੱਲ;
#  endif
# endif
# ਜੇ ENABLE_HUSH_SAVEHISTORY && MAX_HISTORY > 0
		{
			ਅਚਲ ਅੱਖਰ *hp = get_local_var_ਮੁੱਲ("HISTFILE");
			ਜੇ (!hp) {
				hp = get_local_var_ਮੁੱਲ("HOME");
				ਜੇ (hp)
					hp = concat_ਰਸਤਾ_file(hp, ".hush_history");
			} ਨਹੀਂਤਾਂ {
				hp = xstrdup(hp);
			}
			ਜੇ (hp) {
				G.ਸਤਰ_ਨਿਵੇਸ਼_state->hist_file = hp;
				//set_local_var(xasprintf("HISTFILE=%s", ...));
			}
#  ਜੇ ENABLE_FEATURE_SH_HISTFILESIZE
			hp = get_local_var_ਮੁੱਲ("HISTFILESIZE");
			G.ਸਤਰ_ਨਿਵੇਸ਼_state->max_history = ਮਾਪ_from_HISTFILESIZE(hp);
#  endif
		}
# endif
	} ਨਹੀਂਤਾਂ {
		install_special_sighandlers();
	}
#ਜਾਂਜੇ ENABLE_HUSH_INTERACTIVE
	/* No job control compiled in, only prompt/line editing */
	ਜੇ (isatty(STDIN_FILENO) && isatty(STDOUT_FILENO)) {
		G_interactive_fd = dup_CLOEXEC(STDIN_FILENO, 254);
		ਜੇ (G_interactive_fd < 0) {
			/* try to dup to any fd */
			G_interactive_fd = dup_CLOEXEC(STDIN_FILENO, -1);
			ਜੇ (G_interactive_fd < 0)
				/* give up */
				G_interactive_fd = 0;
		}
	}
	install_special_sighandlers();
#ਨਹੀਂਤਾਂ
	/* We have interactiveness code disabled */
	install_special_sighandlers();
#ਜੇ_ਅੰਤ
	/* bash:
	 * if interactive but not a login shell, sources ~/.bashrc
	 * (--norc turns this off, --rcfile <file> overrides)
	 */

	ਜੇ (G_interactive_fd) {
#ਜੇ ENABLE_HUSH_INTERACTIVE && ENABLE_FEATURE_EDITING_FANCY_PROMPT
		/* Set (but not export) PS1/2 unless already set */
		ਜੇ (!get_local_var_ਮੁੱਲ("PS1"))
			ਸੈੱਟ_local_var_from_halves("PS1", "\\w \\$ ");
		ਜੇ (!get_local_var_ਮੁੱਲ("PS2"))
			ਸੈੱਟ_local_var_from_halves("PS2", "> ");
#ਜੇ_ਅੰਤ
		ਜੇ (!ENABLE_FEATURE_SH_EXTRA_QUIET) {
			/* note: ash and hush share this string */
			ਲਿਖੋ("\n\n%s %s\n"
				IF_HUSH_HELP("Enter 'help' for a list of built-in commands.\n")
				"\n",
				bb_banner,
				"hush - the humble shell"
			);
		}
	}

	parse_and_ਲੜੀਵਾਰ_file(hfopen(ਨਿਰਾ)); /* stdin */

 final_return:
	hush_exit(G.ਆਖਰੀ_exitcode);
}

/*
 * Built-ins
 */
ਸਥਿਰ ਪੂਰਨ FAST_FUNC builtin_true(ਅੱਖਰ **argv ਅਣਵਰਤਿਆ_PARAM)
{
	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ FAST_FUNC builtin_false(ਅੱਖਰ **argv ਅਣਵਰਤਿਆ_PARAM)
{
	ਵਾਪਸ 1;
}

#ਜੇ ENABLE_HUSH_TEST || ENABLE_HUSH_ECHO || ENABLE_HUSH_PRINTF || ENABLE_HUSH_KILL
ਸਥਿਰ NOINLINE ਪੂਰਨ ਲੜੀਵਾਰ_applet_main(ਅੱਖਰ **argv, ਪੂਰਨ (*applet_main_func)(ਪੂਰਨ argc, ਅੱਖਰ **argv))
{
	ਪੂਰਨ argc = string_array_len(argv);
	ਵਾਪਸ applet_main_func(argc, argv);
}
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_HUSH_TEST || BASH_TEST2
ਸਥਿਰ ਪੂਰਨ FAST_FUNC builtin_test(ਅੱਖਰ **argv)
{
	ਵਾਪਸ ਲੜੀਵਾਰ_applet_main(argv, test_main);
}
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_HUSH_ECHO
ਸਥਿਰ ਪੂਰਨ FAST_FUNC builtin_echo(ਅੱਖਰ **argv)
{
	ਵਾਪਸ ਲੜੀਵਾਰ_applet_main(argv, echo_main);
}
#ਜੇ_ਅੰਤ
#ਜੇ ENABLE_HUSH_PRINTF
ਸਥਿਰ ਪੂਰਨ FAST_FUNC builtin_printf(ਅੱਖਰ **argv)
{
	ਵਾਪਸ ਲੜੀਵਾਰ_applet_main(argv, printf_main);
}
#ਜੇ_ਅੰਤ

#ਜੇ ENABLE_HUSH_HELP
ਸਥਿਰ ਪੂਰਨ FAST_FUNC builtin_help(ਅੱਖਰ **argv ਅਣਵਰਤਿਆ_PARAM)
{
	ਅਚਲ ਬਣਤਰ built_ਵਿੱਚ_command *x;

	ਲਿਖੋ(
		"Built-in commands:\n"
		"------------------\n");
	ਲਈ (x = bltins1; x != &bltins1[ARRAY_ਮਾਪ(bltins1)]; x++) {
		ਜੇ (x->b_descr)
			ਲਿਖੋ("%-10s%s\n", x->b_cmd, x->b_descr);
	}
	ਵਾਪਸ EXIT_SUCCESS;
}
#ਜੇ_ਅੰਤ

#ਜੇ MAX_HISTORY && ENABLE_FEATURE_EDITING
ਸਥਿਰ ਪੂਰਨ FAST_FUNC builtin_history(ਅੱਖਰ **argv ਅਣਵਰਤਿਆ_PARAM)
{
	show_history(G.ਸਤਰ_ਨਿਵੇਸ਼_state);
	ਵਾਪਸ EXIT_SUCCESS;
}
#ਜੇ_ਅੰਤ

ਸਥਿਰ ਪੂਰਨ FAST_FUNC builtin_cd(ਅੱਖਰ **argv)
{
	ਅਚਲ ਅੱਖਰ *newdir;

	argv = skip_dash_dash(argv);
	newdir = argv[0];
	ਜੇ (newdir == ਨਿਰਾ) {
		/* bash does nothing (exitcode 0) if HOME is ""; if it's unset,
		 * bash says "bash: cd: HOME not set" and does nothing
		 * (exitcode 1)
		 */
		ਅਚਲ ਅੱਖਰ *home = get_local_var_ਮੁੱਲ("HOME");
		newdir = home ? home : "/";
	}
	ਜੇ (chdir(newdir)) {
		/* Mimic bash message exactly */
		bb_perror_ਸੁਨੇਹਾ("cd: %s", newdir);
		ਵਾਪਸ EXIT_FAILURE;
	}
	/* Read current dir (get_cwd(1) is inside) and set PWD.
	 * Note: do not enforce exporting. If PWD was unset or unexported,
	 * set it again, but do not export. bash does the same.
	 */
	ਸੈੱਟ_pwd_var(/*flag:*/ 0);
	ਵਾਪਸ EXIT_SUCCESS;
}

ਸਥਿਰ ਪੂਰਨ FAST_FUNC builtin_pwd(ਅੱਖਰ **argv ਅਣਵਰਤਿਆ_PARAM)
{
	puts(get_cwd(0));
	ਵਾਪਸ EXIT_SUCCESS;
}

ਸਥਿਰ ਪੂਰਨ FAST_FUNC builtin_eval(ਅੱਖਰ **argv)
{
	argv = skip_dash_dash(argv);

	ਜੇ (!argv[0])
		ਵਾਪਸ EXIT_SUCCESS;

	IF_HUSH_ਬਹੁਲਕ_X(G.x_ਬਹੁਲਕ_ਡੂੰਘਾਈ++;)
	//bb_error_msg("%s: ++x_mode_depth=%d", __func__, G.x_mode_depth);
	ਜੇ (!argv[1]) {
		/* bash:
		 * eval "echo Hi; done" ("done" is syntax error):
		 * "echo Hi" will not execute too.
		 */
		parse_and_ਲੜੀਵਾਰ_string(argv[0]);
	} ਨਹੀਂਤਾਂ {
		/* "The eval utility shall construct a command by
		 * concatenating arguments together, separating
		 * each with a <space> character."
		 */
		ਅੱਖਰ *str, *p;
		ਬਿਨਾਨਿਸ਼ਾਨੀ len = 0;
		ਅੱਖਰ **pp = argv;
		ਕਰੋ
			len += ਲੜੀ_ਲੰਬਾਈ(*pp) + 1;
		ਜਦੋਂ (*++pp);
		str = p = xmalloc(len);
		pp = argv;
		ਲਈ (;;) {
			p = stpcpy(p, *pp);
			pp++;
			ਜੇ (!*pp)
				ਰੋਕੋ;
			*p++ = ' ';
		}
		parse_and_ਲੜੀਵਾਰ_string(str);
		ਮੁਕਤ(str);
	}
	IF_HUSH_ਬਹੁਲਕ_X(G.x_ਬਹੁਲਕ_ਡੂੰਘਾਈ--;)
	//bb_error_msg("%s: --x_mode_depth=%d", __func__, G.x_mode_depth);
	ਵਾਪਸ G.ਆਖਰੀ_exitcode;
}

ਸਥਿਰ ਪੂਰਨ FAST_FUNC builtin_exec(ਅੱਖਰ **argv)
{
	argv = skip_dash_dash(argv);
	ਜੇ (argv[0] == ਨਿਰਾ)
		ਵਾਪਸ EXIT_SUCCESS; /* bash does this */

	/* Careful: we can end up here after [v]fork. Do not restore
	 * tty pgrp then, only top-level shell process does that */
	ਜੇ (G_saved_tty_pgrp && getpid() == G.ਜੜ੍ਹ_pid)
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

ਸਥਿਰ ਪੂਰਨ FAST_FUNC builtin_exit(ਅੱਖਰ **argv)
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
	ਜੇ (argv[0] == ਨਿਰਾ) {
#ਜੇ ENABLE_HUSH_TRAP
		ਜੇ (G.pre_trap_exitcode >= 0) /* "exit" in trap uses $? from before the trap */
			hush_exit(G.pre_trap_exitcode);
#ਜੇ_ਅੰਤ
		hush_exit(G.ਆਖਰੀ_exitcode);
	}
	/* mimic bash: exit 123abc == exit 255 + error msg */
	xfunc_error_retval = 255;
	/* bash: exit -2 == exit 254, no error msg */
	hush_exit(xatoi(argv[0]) & 0xff);
}

#ਜੇ ENABLE_HUSH_TYPE
/* http://www.opengroup.org/onlinepubs/9699919799/utilities/type.html */
ਸਥਿਰ ਪੂਰਨ FAST_FUNC builtin_type(ਅੱਖਰ **argv)
{
	ਪੂਰਨ ret = EXIT_SUCCESS;

	ਜਦੋਂ (*++argv) {
		ਅਚਲ ਅੱਖਰ *type;
		ਅੱਖਰ *ਰਸਤਾ_2 = ਨਿਰਾ;

		ਜੇ (0) {} /* make conditional compile easier below */
		/*else if (find_alias(*argv))
			type = "an alias";*/
# ਜੇ ENABLE_HUSH_FUNCTIONS
		ਨਹੀਂਤਾਂ ਜੇ (ਲੱਭੋ_function(*argv))
			type = "a function";
# endif
		ਨਹੀਂਤਾਂ ਜੇ (ਲੱਭੋ_builtin(*argv))
			type = "a shell builtin";
		ਨਹੀਂਤਾਂ ਜੇ ((ਰਸਤਾ_2 = ਲੱਭੋ_ਵਿੱਚ_ਰਸਤਾ(*argv)) != ਨਿਰਾ)
			type = ਰਸਤਾ_2;
		ਨਹੀਂਤਾਂ {
			bb_error_ਸੁਨੇਹਾ("type: %s: not found", *argv);
			ret = EXIT_FAILURE;
			ਜਾਰੀ;
		}

		ਲਿਖੋ("%s is %s\n", *argv, type);
		ਮੁਕਤ(ਰਸਤਾ_2);
	}

	ਵਾਪਸ ret;
}
#ਜੇ_ਅੰਤ

#ਜੇ ENABLE_HUSH_READ
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
ਸਥਿਰ ਪੂਰਨ FAST_FUNC builtin_read(ਅੱਖਰ **argv)
{
	ਅਚਲ ਅੱਖਰ *r;
	ਬਣਤਰ builtin_read_params params;

	memset(&params, 0, ਆਕਾਰ(params));

	/* "!": do not abort on errors.
	 * Option string must start with "sr" to match BUILTIN_READ_xxx
	 */
	params.read_flags = getopt32(argv,
# ਜੇ BASH_READ_D
		IF_NOT_HUSH_BASH_COMPAT("^")
		"!srn:p:t:u:d:" IF_NOT_HUSH_BASH_COMPAT("\0" "-1"/*min 1 arg*/),
		&params.opt_n, &params.opt_p, &params.opt_t, &params.opt_u, &params.opt_d
# ਨਹੀਂਤਾਂ
		IF_NOT_HUSH_BASH_COMPAT("^")
		"!srn:p:t:u:" IF_NOT_HUSH_BASH_COMPAT("\0" "-1"/*min 1 arg*/),
		&params.opt_n, &params.opt_p, &params.opt_t, &params.opt_u
# endif
//TODO: print "read: need variable name"
//for the case of !BASH "read" with no args (now it fails silently)
//(or maybe extend getopt32() to emit a message if "-1" fails)
	);
	ਜੇ ((uint32_t)params.read_flags == (uint32_t)-1)
		ਵਾਪਸ EXIT_FAILURE;
	argv += optind;
	params.argv = argv;
	params.setvar = ਸੈੱਟ_local_var_from_halves;
	params.ifs = get_local_var_ਮੁੱਲ("IFS"); /* can be NULL */

 again:
	r = shell_builtin_read(&params);

	ਜੇ ((uintptr_t)r == 1 && errno == EINTR) {
		ਬਿਨਾਨਿਸ਼ਾਨੀ sig = check_and_ਲੜੀਵਾਰ_traps();
		ਜੇ (sig != SIGINT)
			ਜਾਓ again;
	}

	ਜੇ ((uintptr_t)r > 1) {
		bb_simple_error_ਸੁਨੇਹਾ(r);
		r = (ਅੱਖਰ*)(uintptr_t)1;
	}

	ਵਾਪਸ (uintptr_t)r;
}
#ਜੇ_ਅੰਤ

#ਜੇ ENABLE_HUSH_UMASK
ਸਥਿਰ ਪੂਰਨ FAST_FUNC builtin_umask(ਅੱਖਰ **argv)
{
	ਪੂਰਨ rc;
	mode_t ਨਕਾਬ;

	rc = 1;
	ਨਕਾਬ = umask(0);
	argv = skip_dash_dash(argv);
	ਜੇ (argv[0]) {
		mode_t old_ਨਕਾਬ = ਨਕਾਬ;

		/* numeric umasks are taken as-is */
		/* symbolic umasks are inverted: "umask a=rx" calls umask(222) */
		ਜੇ (!ਅੰਕ_ਹੈ(argv[0][0]))
			ਨਕਾਬ ^= 0777;
		ਨਕਾਬ = bb_parse_ਬਹੁਲਕ(argv[0], ਨਕਾਬ);
		ਜੇ (!ਅੰਕ_ਹੈ(argv[0][0]))
			ਨਕਾਬ ^= 0777;
		ਜੇ ((ਬਿਨਾਨਿਸ਼ਾਨੀ)ਨਕਾਬ > 0777) {
			ਨਕਾਬ = old_ਨਕਾਬ;
			/* bash messages:
			 * bash: umask: 'q': invalid symbolic mode operator
			 * bash: umask: 999: octal number out of range
			 */
			bb_error_ਸੁਨੇਹਾ("%s: invalid mode '%s'", "umask", argv[0]);
			rc = 0;
		}
	} ਨਹੀਂਤਾਂ {
		/* Mimic bash */
		ਲਿਖੋ("%04o\n", (ਬਿਨਾਨਿਸ਼ਾਨੀ) ਨਕਾਬ);
		/* fall through and restore mask which we set to 0 */
	}
	umask(ਨਕਾਬ);

	ਵਾਪਸ !rc; /* rc != 0 - success */
}
#ਜੇ_ਅੰਤ

#ਜੇ ENABLE_HUSH_EXPORT || ENABLE_HUSH_READONLY || ENABLE_HUSH_ਸੈੱਟ || ENABLE_HUSH_TRAP
ਸਥਿਰ ਖਾਲੀ print_escaped(ਅਚਲ ਅੱਖਰ *s)
{
//TODO? bash "set" does not quote variables which contain only alnums and "%+,-./:=@_~",
// (but "export" quotes all variables, even with only these chars).
// I think quoting strings with %+,=~ looks better
// (example: "set" printing var== instead of var='=' looks strange)
// IOW: do not quote "-./:@_": / is used in pathnames, : in PATH, -._ often in file names, @ in emails

	ਜੇ (*s == '\'')
		ਜਾਓ squote;
	ਕਰੋ {
		ਅਚਲ ਅੱਖਰ *p = strchrnul(s, '\'');
		/* print 'xxxx', possibly just '' */
		ਲਿਖੋ("'%.*s'", (ਪੂਰਨ)(p - s), s);
		ਜੇ (*p == '\0')
			ਰੋਕੋ;
		s = p;
 squote:
		/* s points to '; print "'''...'''" */
		ਅੱਖਰ_ਲਿਖੋ('"');
		ਕਰੋ ਅੱਖਰ_ਲਿਖੋ('\''); ਜਦੋਂ (*++s == '\'');
		ਅੱਖਰ_ਲਿਖੋ('"');
	} ਜਦੋਂ (*s);
}
#ਜੇ_ਅੰਤ

#ਜੇ ENABLE_HUSH_EXPORT || ENABLE_HUSH_LOCAL || ENABLE_HUSH_READONLY
ਸਥਿਰ ਪੂਰਨ helper_export_local(ਅੱਖਰ **argv, ਬਿਨਾਨਿਸ਼ਾਨੀ flags)
{
	ਕਰੋ {
		ਅੱਖਰ *name = *argv;
		ਅਚਲ ਅੱਖਰ *name_ਸਿਰਾ = endofname(name);

		ਜੇ (*name_ਸਿਰਾ == '\0') {
			ਬਣਤਰ variable *var, **vpp;

			vpp = get_ptr_ਮੰਜ਼ਿਲ_local_var(name);
			var = vpp ? *vpp : ਨਿਰਾ;

			ਜੇ (flags & SETFLAG_UNEXPORT) {
				/* export -n NAME (without =VALUE) */
				ਜੇ (var) {
					var->flg_export = 0;
					debug_printf_env("%s: unsetenv '%s'\n", __func__, name);
					unsetenv(name);
				} /* else: export -n NOT_EXISTING_VAR: no-op */
				ਜਾਰੀ;
			}
			ਜੇ (flags & SETFLAG_EXPORT) {
				/* export NAME (without =VALUE) */
				ਜੇ (var) {
					var->flg_export = 1;
					debug_printf_env("%s: putenv '%s'\n", __func__, var->varstr);
					putenv(var->varstr);
					ਜਾਰੀ;
				}
			}
			ਜੇ (flags & SETFLAG_MAKE_RO) {
				/* readonly NAME (without =VALUE) */
				ਜੇ (var) {
					var->flg_read_only = 1;
					ਜਾਰੀ;
				}
			}
# ਜੇ ENABLE_HUSH_LOCAL
			/* Is this "local" bltin? */
			ਜੇ (!(flags & (SETFLAG_EXPORT|SETFLAG_UNEXPORT|SETFLAG_MAKE_RO))) {
				ਬਿਨਾਨਿਸ਼ਾਨੀ lvl = flags >> SETFLAG_VARLVL_ਖਿਸਕਾਓ;
				ਜੇ (var && var->var_nest_level == lvl) {
					/* "local x=abc; ...; local x" - ignore second local decl */
					ਜਾਰੀ;
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
		} ਨਹੀਂਤਾਂ {
			ਜੇ (*name_ਸਿਰਾ != '=') {
				bb_error_ਸੁਨੇਹਾ("'%s': bad variable name", name);
				/* do not parse following argv[]s: */
				ਵਾਪਸ 1;
			}
			/* (Un)exporting/making local NAME=VALUE */
			name = xstrdup(name);
			/* Testcase: export PS1='\w \$ ' */
			unbackslash(name);
		}
		debug_printf_env("%s: set_local_var('%s')\n", __func__, name);
		ਜੇ (ਸੈੱਟ_local_var(name, flags))
			ਵਾਪਸ EXIT_FAILURE;
	} ਜਦੋਂ (*++argv);
	ਵਾਪਸ EXIT_SUCCESS;
}
#ਜੇ_ਅੰਤ

#ਜੇ ENABLE_HUSH_EXPORT
ਸਥਿਰ ਪੂਰਨ FAST_FUNC builtin_export(ਅੱਖਰ **argv)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ opt_unexport;

# ਜੇ ENABLE_HUSH_EXPORT_N
	/* "!": do not abort on errors */
	opt_unexport = getopt32(argv, "!n");
	ਜੇ (opt_unexport == (uint32_t)-1)
		ਵਾਪਸ EXIT_FAILURE;
	argv += optind;
# ਨਹੀਂਤਾਂ
	opt_unexport = 0;
	argv++;
# endif

	ਜੇ (argv[0] == ਨਿਰਾ) {
		ਅੱਖਰ **e = environ;
		ਜੇ (e) {
			ਜਦੋਂ (*e) {
# ਜੇ 0
				puts(*e++);
# ਨਹੀਂਤਾਂ
				/* ash emits: export VAR='VAL'
				 * bash: declare -x VAR="VAL"
				 * we follow ash example */
				ਅਚਲ ਅੱਖਰ *s = *e++;
				ਅਚਲ ਅੱਖਰ *p = strchr(s, '=');

				ਜੇ (!p) /* wtf? take next variable */
					ਜਾਰੀ;
				/* "export VAR=" */
				ਲਿਖੋ("%s %.*s", "export", (ਪੂਰਨ)(p - s) + 1, s);
				print_escaped(p + 1);
				ਅੱਖਰ_ਲਿਖੋ('\n');
# endif
			}
			/*fflush_all(); - done after each builtin anyway */
		}
		ਵਾਪਸ EXIT_SUCCESS;
	}

	ਵਾਪਸ helper_export_local(argv, opt_unexport ? SETFLAG_UNEXPORT : SETFLAG_EXPORT);
}
#ਜੇ_ਅੰਤ

#ਜੇ ENABLE_HUSH_LOCAL
ਸਥਿਰ ਪੂਰਨ FAST_FUNC builtin_local(ਅੱਖਰ **argv)
{
	ਜੇ (G.func_nest_level == 0) {
		bb_error_ਸੁਨੇਹਾ("%s: not in a function", argv[0]);
		ਵਾਪਸ EXIT_FAILURE; /* bash compat */
	}
//TODO? ash and bash support "local -" special form,
//which saves/restores $- around function call (including async returns, such as ^C)
//(IOW: it makes "set +/-..." effects local)
	argv++;
	/* Since all builtins run in a nested variable level,
	 * need to use level - 1 here. Or else the variable will be removed at once
	 * after builtin returns.
	 */
	ਵਾਪਸ helper_export_local(argv, (G.var_nest_level - 1) << SETFLAG_VARLVL_ਖਿਸਕਾਓ);
}
#ਜੇ_ਅੰਤ

#ਜੇ ENABLE_HUSH_READONLY
ਸਥਿਰ ਪੂਰਨ FAST_FUNC builtin_readonly(ਅੱਖਰ **argv)
{
	argv++;
	ਜੇ (*argv == ਨਿਰਾ) {
		/* bash: readonly [-p]: list all readonly VARs
		 * (-p has no effect in bash)
		 */
		ਬਣਤਰ variable *e;
		ਲਈ (e = G.ਸਿਖਰ_var; e; e = e->ਅਗਲਾ) {
			ਜੇ (e->flg_read_only) {
				ਅਚਲ ਅੱਖਰ *s = e->varstr;
				ਅਚਲ ਅੱਖਰ *p = strchr(s, '=');

				ਜੇ (!p) /* wtf? take next variable */
					ਜਾਰੀ;
				/* "readonly VAR=" */
				ਲਿਖੋ("%s %.*s", "readonly", (ਪੂਰਨ)(p - s) + 1, s);
				print_escaped(p + 1);
				ਅੱਖਰ_ਲਿਖੋ('\n');
			}
		}
		ਵਾਪਸ EXIT_SUCCESS;
	}
	ਵਾਪਸ helper_export_local(argv, SETFLAG_MAKE_RO);
}
#ਜੇ_ਅੰਤ

#ਜੇ ENABLE_HUSH_UNSET
/* http://www.opengroup.org/onlinepubs/9699919799/utilities/V3_chap02.html#unset */
ਸਥਿਰ ਪੂਰਨ FAST_FUNC builtin_unset(ਅੱਖਰ **argv)
{
	ਪੂਰਨ ret;
	ਬਿਨਾਨਿਸ਼ਾਨੀ opts;

	/* "!": do not abort on errors */
	/* "+": stop at 1st non-option */
	opts = getopt32(argv, "!+vf");
	ਜੇ (opts == (ਬਿਨਾਨਿਸ਼ਾਨੀ)-1)
		ਵਾਪਸ EXIT_FAILURE;
	ਜੇ (opts == 3) {
		bb_simple_error_ਸੁਨੇਹਾ("unset: -v and -f are exclusive");
		ਵਾਪਸ EXIT_FAILURE;
	}
	argv += optind;

	ret = EXIT_SUCCESS;
	ਜਦੋਂ (*argv) {
		ਜੇ (!(opts & 2)) { /* not -f */
			ਜੇ (unset_local_var(*argv)) {
				/* unset <nonexistent_var> doesn't fail.
				 * Error is when one tries to unset RO var.
				 * Message was printed by unset_local_var. */
				ret = EXIT_FAILURE;
			}
		}
# ਜੇ ENABLE_HUSH_FUNCTIONS
		ਨਹੀਂਤਾਂ {
			unset_func(*argv);
		}
# endif
		argv++;
	}
	ਵਾਪਸ ret;
}
#ਜੇ_ਅੰਤ

#ਜੇ ENABLE_HUSH_ਸੈੱਟ
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
ਸਥਿਰ ਪੂਰਨ FAST_FUNC builtin_ਸੈੱਟ(ਅੱਖਰ **argv)
{
	ਪੂਰਨ n;
	ਅੱਖਰ **pp, **g_argv;
	ਅੱਖਰ *arg = *++argv;

	ਜੇ (arg == ਨਿਰਾ) {
		ਬਣਤਰ variable *e;
		ਲਈ (e = G.ਸਿਖਰ_var; e; e = e->ਅਗਲਾ) {
			ਅਚਲ ਅੱਖਰ *s = e->varstr;
			ਅਚਲ ਅੱਖਰ *p = strchr(s, '=');

			ਜੇ (!p) /* wtf? take next variable */
				ਜਾਰੀ;
			/* var= */
			ਲਿਖੋ("%.*s", (ਪੂਰਨ)(p - s) + 1, s);
			print_escaped(p + 1);
			ਅੱਖਰ_ਲਿਖੋ('\n');
		}
		ਵਾਪਸ EXIT_SUCCESS;
	}

	ਕਰੋ {
		ਜੇ (strcmp(arg, "--") == 0) {
			++argv;
			ਜਾਓ ਸੈੱਟ_argv;
		}
		ਜੇ (arg[0] != '+' && arg[0] != '-')
			ਰੋਕੋ;
		ਲਈ (n = 1; arg[n]; ++n) {
			ਜੇ (ਸੈੱਟ_ਬਹੁਲਕ((arg[0] == '-'), arg[n], argv[1])) {
				bb_error_ਸੁਨੇਹਾ("%s: %s: invalid option", "set", arg);
				ਵਾਪਸ EXIT_FAILURE;
			}
			ਜੇ (arg[n] == 'o' && argv[1])
				argv++;
		}
	} ਜਦੋਂ ((arg = *++argv) != ਨਿਰਾ);
	/* Now argv[0] is 1st argument */

	ਜੇ (arg == ਨਿਰਾ)
		ਵਾਪਸ EXIT_SUCCESS;
 ਸੈੱਟ_argv:

	/* NB: G.global_argv[0] ($0) is never freed/changed */
	g_argv = G.global_argv;
	ਜੇ (G.global_ਦਲੀਲਾਂ_malloced) {
		pp = g_argv;
		ਜਦੋਂ (*++pp)
			ਮੁਕਤ(*pp);
		g_argv[1] = ਨਿਰਾ;
	} ਨਹੀਂਤਾਂ {
		G.global_ਦਲੀਲਾਂ_malloced = 1;
		pp = xzalloc(ਆਕਾਰ(pp[0]) * 2);
		pp[0] = g_argv[0]; /* retain $0 */
		g_argv = pp;
	}
	/* This realloc's G.global_argv */
	G.global_argv = pp = ਜੋੜੋ_ਲੜੀਆਂ_ਮੰਜ਼ਿਲ_ਲੜੀਆਂ(g_argv, argv, /*dup:*/ 1);

	G.global_argc = 1 + string_array_len(pp + 1);

	ਵਾਪਸ EXIT_SUCCESS;
}
#ਜੇ_ਅੰਤ

ਸਥਿਰ ਪੂਰਨ FAST_FUNC builtin_ਖਿਸਕਾਓ(ਅੱਖਰ **argv)
{
	ਪੂਰਨ n = 1;
	argv = skip_dash_dash(argv);
	ਜੇ (argv[0]) {
		n = bb_strtou(argv[0], ਨਿਰਾ, 10);
		ਜੇ (errno || n < 0) {
			/* shared string with ash.c */
			bb_error_ਸੁਨੇਹਾ("Illegal number: %s", argv[0]);
			/*
			 * ash aborts in this case.
			 * bash prints error message and set $? to 1.
			 * Interestingly, for "shift 99999" bash does not
			 * print error message, but does set $? to 1
			 * (and does no shifting at all).
			 */
		}
	}
	ਜੇ (n >= 0 && n < G.global_argc) {
		ਜੇ (G_global_ਦਲੀਲਾਂ_malloced) {
			ਪੂਰਨ m = 1;
			ਜਦੋਂ (m <= n)
				ਮੁਕਤ(G.global_argv[m++]);
		}
		G.global_argc -= n;
		memmove(&G.global_argv[1], &G.global_argv[n+1],
				G.global_argc * ਆਕਾਰ(G.global_argv[0]));
		ਵਾਪਸ EXIT_SUCCESS;
	}
	ਵਾਪਸ EXIT_FAILURE;
}

#ਜੇ ENABLE_HUSH_GETOPTS
ਸਥਿਰ ਪੂਰਨ FAST_FUNC builtin_getopts(ਅੱਖਰ **argv)
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
	ਅੱਖਰ cbuf[2];
	ਅਚਲ ਅੱਖਰ *cp, *optstring, *var;
	ਪੂਰਨ c, n, exitcode, my_opterr;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਗਿਣਤੀ_2;

	optstring = *++argv;
	ਜੇ (!optstring || !(var = *++argv)) {
		bb_simple_error_ਸੁਨੇਹਾ("usage: getopts OPTSTRING VAR [ARGS]");
		ਵਾਪਸ EXIT_FAILURE;
	}

	ਜੇ (argv[1])
		argv[0] = G.global_argv[0]; /* for error messages in getopt() */
	ਨਹੀਂਤਾਂ
		argv = G.global_argv;
	cbuf[1] = '\0';

	my_opterr = 0;
	ਜੇ (optstring[0] != ':') {
		cp = get_local_var_ਮੁੱਲ("OPTERR");
		/* 0 if "OPTERR=0", 1 otherwise */
		my_opterr = (!cp || NOT_LONE_CHAR(cp, '0'));
	}

	/* getopts stops on first non-option. Add "+" to force that */
	/*if (optstring[0] != '+')*/ {
		ਅੱਖਰ *s = alloca(ਲੜੀ_ਲੰਬਾਈ(optstring) + 2);
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
	ਗਿਣਤੀ_2 = 0;
	n = string_array_len(argv);
	ਕਰੋ {
		optarg = ਨਿਰਾ;
		opterr = (ਗਿਣਤੀ_2 < G.getopt_ਗਿਣਤੀ) ? 0 : my_opterr;
		c = getopt(n, argv, optstring);
		ਜੇ (c < 0)
			ਰੋਕੋ;
		ਗਿਣਤੀ_2++;
	} ਜਦੋਂ (ਗਿਣਤੀ_2 <= G.getopt_ਗਿਣਤੀ);

	/* Set OPTIND. Prevent resetting of the magic counter! */
	ਸੈੱਟ_local_var_from_halves("OPTIND", utoa(optind));
	G.getopt_ਗਿਣਤੀ = ਗਿਣਤੀ_2; /* "next time, give me N+1'th result" */
	GETOPT_RESET(); /* just in case */

	/* Set OPTARG */
	/* Always set or unset, never left as-is, even on exit/error:
	 * "If no option was found, or if the option that was found
	 * does not have an option-argument, OPTARG shall be unset."
	 */
	cp = optarg;
	ਜੇ (c == '?') {
		/* If ":optstring" and unknown option is seen,
		 * it is stored to OPTARG.
		 */
		ਜੇ (optstring[1] == ':') {
			cbuf[0] = optopt;
			cp = cbuf;
		}
	}
	ਜੇ (cp)
		ਸੈੱਟ_local_var_from_halves("OPTARG", cp);
	ਨਹੀਂਤਾਂ
		unset_local_var("OPTARG");

	/* Convert -1 to "?" */
	exitcode = EXIT_SUCCESS;
	ਜੇ (c < 0) { /* -1: end of options */
		exitcode = EXIT_FAILURE;
		c = '?';
	}

	/* Set VAR */
	cbuf[0] = c;
	ਸੈੱਟ_local_var_from_halves(var, cbuf);

	ਵਾਪਸ exitcode;
}
#ਜੇ_ਅੰਤ

ਸਥਿਰ ਪੂਰਨ FAST_FUNC builtin_ਸਰੋਤ(ਅੱਖਰ **argv)
{
	ਅੱਖਰ *arg_ਰਸਤਾ, *filename;
	HFILE *ਨਿਵੇਸ਼;
	save_arg_t sv;
	ਅੱਖਰ *ਦਲੀਲਾਂ_need_save;
#ਜੇ ENABLE_HUSH_FUNCTIONS
	smallint sv_flg;
#ਜੇ_ਅੰਤ

	argv = skip_dash_dash(argv);
	filename = argv[0];
	ਜੇ (!filename) {
		/* bash says: "bash: .: filename argument required" */
		ਵਾਪਸ 2; /* bash compat */
	}
	arg_ਰਸਤਾ = ਨਿਰਾ;
	ਜੇ (!strchr(filename, '/')) {
		arg_ਰਸਤਾ = ਲੱਭੋ_ਵਿੱਚ_ਰਸਤਾ(filename);
		ਜੇ (arg_ਰਸਤਾ)
			filename = arg_ਰਸਤਾ;
		ਨਹੀਂਤਾਂ ਜੇ (!ENABLE_HUSH_BASH_ਸਰੋਤ_CURDIR) {
			errno = ENOENT;
			bb_simple_perror_ਸੁਨੇਹਾ(filename);
			ਵਾਪਸ EXIT_FAILURE;
		}
	}
	ਨਿਵੇਸ਼ = hfopen(filename);
	ਮੁਕਤ(arg_ਰਸਤਾ);
	ਜੇ (!ਨਿਵੇਸ਼) {
		bb_perror_ਸੁਨੇਹਾ("%s", filename);
		/* POSIX: non-interactive shell should abort here,
		 * not merely fail. So far no one complained :)
		 */
		ਵਾਪਸ EXIT_FAILURE;
	}

#ਜੇ ENABLE_HUSH_FUNCTIONS
	sv_flg = G_flag_return_ਵਿੱਚ_ਤਰੱਕੀ;
	/* "we are inside sourced file, ok to use return" */
	G_flag_return_ਵਿੱਚ_ਤਰੱਕੀ = -1;
#ਜੇ_ਅੰਤ
	ਦਲੀਲਾਂ_need_save = argv[1]; /* used as a boolean variable */
	ਜੇ (ਦਲੀਲਾਂ_need_save)
		save_and_replace_G_ਦਲੀਲਾਂ(&sv, argv);

	/* "false; . ./empty_line; echo Zero:$?" should print 0 */
	G.ਆਖਰੀ_exitcode = 0;
	parse_and_ਲੜੀਵਾਰ_file(ਨਿਵੇਸ਼);
	hfclose(ਨਿਵੇਸ਼);

	ਜੇ (ਦਲੀਲਾਂ_need_save) /* can't use argv[1] instead: "shift" can mangle it */
		restore_G_ਦਲੀਲਾਂ(&sv, argv);
#ਜੇ ENABLE_HUSH_FUNCTIONS
	G_flag_return_ਵਿੱਚ_ਤਰੱਕੀ = sv_flg;
#ਜੇ_ਅੰਤ

	ਵਾਪਸ G.ਆਖਰੀ_exitcode;
}

#ਜੇ ENABLE_HUSH_TRAP
ਸਥਿਰ ਪੂਰਨ FAST_FUNC builtin_trap(ਅੱਖਰ **argv)
{
	ਪੂਰਨ sig;
	ਅੱਖਰ *new_cmd;

	ਜੇ (!G_traps)
		G_traps = xzalloc(ਆਕਾਰ(G_traps[0]) * NSIG);

	argv++;
	ਜੇ (!*argv) {
		ਪੂਰਨ i;
		/* No args: print all trapped */
		ਲਈ (i = 0; i < NSIG; ++i) {
			ਜੇ (G_traps[i]) {
				ਲਿਖੋ("trap -- ");
				print_escaped(G_traps[i]);
				/* note: bash adds "SIG", but only if invoked
				 * as "bash". If called as "sh", or if set -o posix,
				 * then it prints short signal names.
				 * We are printing short names: */
				ਲਿਖੋ(" %s\n", get_signame(i));
			}
		}
		/*fflush_all(); - done after each builtin anyway */
		ਵਾਪਸ EXIT_SUCCESS;
	}

	new_cmd = ਨਿਰਾ;
	/* If first arg is a number: reset all specified signals */
	sig = bb_strtou(*argv, ਨਿਰਾ, 10);
	ਜੇ (errno == 0) {
		ਪੂਰਨ ret;
 process_sig_list:
		ret = EXIT_SUCCESS;
		ਜਦੋਂ (*argv) {
			sighandler_t handler;

			sig = get_signum(*argv++);
			ਜੇ (sig < 0) {
				ret = EXIT_FAILURE;
				/* Mimic bash message exactly */
				bb_error_ਸੁਨੇਹਾ("trap: %s: invalid signal specification", argv[-1]);
				ਜਾਰੀ;
			}

			ਮੁਕਤ(G_traps[sig]);
			G_traps[sig] = xstrdup(new_cmd);

			debug_printf("trap: setting SIG%s (%i) to '%s'\n",
				get_signame(sig), sig, G_traps[sig]);

			/* There is no signal for 0 (EXIT) */
			ਜੇ (sig == 0)
				ਜਾਰੀ;

			ਜੇ (new_cmd)
				handler = (new_cmd[0] ? ਰਿਕਾਰਡ_pending_signo : SIG_IGN);
			ਨਹੀਂਤਾਂ
				/* We are removing trap handler */
				handler = pick_sighandler(sig);
			install_sighandler(sig, handler);
		}
		ਵਾਪਸ ret;
	}

	ਜੇ (!argv[1]) { /* no second arg */
		bb_simple_error_ਸੁਨੇਹਾ("trap: invalid arguments");
		ਵਾਪਸ EXIT_FAILURE;
	}

	/* First arg is "-": reset all specified to default */
	/* First arg is "--": skip it, the rest is "handler SIGs..." */
	/* Everything else: set arg as signal handler
	 * (includes "" case, which ignores signal) */
	ਜੇ (argv[0][0] == '-') {
		ਜੇ (argv[0][1] == '\0') { /* "-" */
			/* new_cmd remains NULL: "reset these sigs" */
			ਜਾਓ reset_traps;
		}
		ਜੇ (argv[0][1] == '-' && argv[0][2] == '\0') { /* "--" */
			argv++;
		}
		/* else: "-something", no special meaning */
	}
	new_cmd = *argv;
 reset_traps:
	argv++;
	ਜਾਓ process_sig_list;
}
#ਜੇ_ਅੰਤ

#ਜੇ ENABLE_HUSH_ਕੰਮ
ਸਥਿਰ ਬਣਤਰ pipe *parse_jobspec(ਅਚਲ ਅੱਖਰ *str)
{
	ਬਣਤਰ pipe *pi;
	ਬਿਨਾਨਿਸ਼ਾਨੀ jobnum;

	ਜੇ (ਲੜੀ_ਤੋਂ_ਪੜ੍ਹੋ(str, "%%%u", &jobnum) != 1) {
		ਜੇ (str[0] != '%'
		 || (str[1] != '%' && str[1] != '+' && str[1] != '\0')
		) {
			bb_error_ਸੁਨੇਹਾ("bad argument '%s'", str);
			ਵਾਪਸ ਨਿਰਾ;
		}
		/* It is "%%", "%+" or "%" - current job */
		jobnum = G.ਆਖਰੀ_jobid;
		ਜੇ (jobnum == 0) {
			bb_simple_error_ਸੁਨੇਹਾ("no current job");
			ਵਾਪਸ ਨਿਰਾ;
		}
	}
	ਲਈ (pi = G.ਕੰਮ_list; pi; pi = pi->ਅਗਲਾ) {
		ਜੇ (pi->jobid == jobnum) {
			ਵਾਪਸ pi;
		}
	}
	bb_error_ਸੁਨੇਹਾ("%u: no such job", jobnum);
	ਵਾਪਸ ਨਿਰਾ;
}

ਸਥਿਰ ਪੂਰਨ FAST_FUNC builtin_jobs(ਅੱਖਰ **argv ਅਣਵਰਤਿਆ_PARAM)
{
	ਬਣਤਰ pipe *ਕੰਮ;
	ਅਚਲ ਅੱਖਰ *ਸਥਿਤੀ_string;

	checkjobs(ਨਿਰਾ, 0 /*(no pid to wait for)*/);
	ਲਈ (ਕੰਮ = G.ਕੰਮ_list; ਕੰਮ; ਕੰਮ = ਕੰਮ->ਅਗਲਾ) {
		ਜੇ (ਕੰਮ->alive_cmds == ਕੰਮ->stopped_cmds)
			ਸਥਿਤੀ_string = "Stopped";
		ਨਹੀਂਤਾਂ
			ਸਥਿਤੀ_string = "Running";

		ਲਿਖੋ(ਕੰਮ_ਸਥਿਤੀ_FORMAT, ਕੰਮ->jobid, ਸਥਿਤੀ_string, ਕੰਮ->cmdtext);
	}

	clean_ਉੱਪਰ_ਆਖਰੀ_dead_ਕੰਮ();

	ਵਾਪਸ EXIT_SUCCESS;
}

/* built-in 'fg' and 'bg' handler */
ਸਥਿਰ ਪੂਰਨ FAST_FUNC builtin_fg_bg(ਅੱਖਰ **argv)
{
	ਪੂਰਨ i;
	ਬਣਤਰ pipe *pi;

	ਜੇ (!G_interactive_fd)
		ਵਾਪਸ EXIT_FAILURE;

	/* If they gave us no args, assume they want the last backgrounded task */
	ਜੇ (!argv[1]) {
		ਲਈ (pi = G.ਕੰਮ_list; pi; pi = pi->ਅਗਲਾ) {
			ਜੇ (pi->jobid == G.ਆਖਰੀ_jobid) {
				ਜਾਓ ਮਿਲਿਆ;
			}
		}
		bb_error_ਸੁਨੇਹਾ("%s: no current job", argv[0]);
		ਵਾਪਸ EXIT_FAILURE;
	}

	pi = parse_jobspec(argv[1]);
	ਜੇ (!pi)
		ਵਾਪਸ EXIT_FAILURE;
 ਮਿਲਿਆ:
	/* TODO: bash prints a string representation
	 * of job being foregrounded (like "sleep 1 | cat") */
	ਜੇ (argv[0][0] == 'f' && G_saved_tty_pgrp) {
		/* Put the job into the foreground. */
		tcsetpgrp(G_interactive_fd, pi->pgrp);
	}

	/* Restart the processes in the job */
	debug_printf_jobs("reviving %d procs, pgrp %d\n", pi->ਅੰਕ_cmds, pi->pgrp);
	ਲਈ (i = 0; i < pi->ਅੰਕ_cmds; i++) {
		debug_printf_jobs("reviving pid %d\n", pi->cmds[i].pid);
	}
	pi->stopped_cmds = 0;

	i = kill(- pi->pgrp, SIGCONT);
	ਜੇ (i < 0) {
		ਜੇ (errno == ESRCH) {
			delete_ਮੁਕੰਮਲ_ਕੰਮ(pi);
			ਵਾਪਸ EXIT_SUCCESS;
		}
		bb_simple_perror_ਸੁਨੇਹਾ("kill (SIGCONT)");
	}

	ਜੇ (argv[0][0] == 'f') {
		remove_ਕੰਮ_from_ਸਾਰਣੀ(pi); /* FG job shouldn't be in job table */
		ਵਾਪਸ checkjobs_and_fg_shell(pi);
	}
	ਵਾਪਸ EXIT_SUCCESS;
}
#ਜੇ_ਅੰਤ

#ਜੇ ENABLE_HUSH_KILL
ਸਥਿਰ ਪੂਰਨ FAST_FUNC builtin_kill(ਅੱਖਰ **argv)
{
	ਪੂਰਨ ret = 0;

# ਜੇ ENABLE_HUSH_ਕੰਮ
	ਜੇ (argv[1] && strcmp(argv[1], "-l") != 0) {
		ਪੂਰਨ i = 1;

		ਕਰੋ {
			ਬਣਤਰ pipe *pi;
			ਅੱਖਰ *dst;
			ਪੂਰਨ j, n;

			ਜੇ (argv[i][0] != '%')
				ਜਾਰੀ;
			/*
			 * "kill %N" - job kill
			 * Converting to pgrp / pid kill
			 */
			pi = parse_jobspec(argv[i]);
			ਜੇ (!pi) {
				/* Eat bad jobspec */
				j = i;
				ਕਰੋ {
					j++;
					argv[j - 1] = argv[j];
				} ਜਦੋਂ (argv[j]);
				ret = 1;
				i--;
				ਜਾਰੀ;
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
			n = G_interactive_fd ? 1 : pi->ਅੰਕ_cmds;
			dst = alloca(n * ਆਕਾਰ(ਪੂਰਨ)*4);
			argv[i] = dst;
			ਜੇ (G_interactive_fd)
				dst += sprintf(dst, " -%u", (ਪੂਰਨ)pi->pgrp);
			ਨਹੀਂਤਾਂ ਲਈ (j = 0; j < n; j++) {
				ਬਣਤਰ command *cmd = &pi->cmds[j];
				/* Skip exited members of the job */
				ਜੇ (cmd->pid == 0)
					ਜਾਰੀ;
				/*
				 * kill_main has matching code to expect
				 * leading space. Needed to not confuse
				 * negative pids with "kill -SIGNAL_NO" syntax
				 */
				dst += sprintf(dst, " %u", (ਪੂਰਨ)cmd->pid);
			}
			*dst = '\0';
		} ਜਦੋਂ (argv[++i]);
	}
# endif

	ਜੇ (argv[1] || ret == 0) {
		ret = ਲੜੀਵਾਰ_applet_main(argv, kill_main);
	}
	/* else: ret = 1, "kill %bad_jobspec" case */
	ਵਾਪਸ ret;
}
#ਜੇ_ਅੰਤ

#ਜੇ ENABLE_HUSH_WAIT
/* http://www.opengroup.org/onlinepubs/9699919799/utilities/wait.html */
# ਜੇ !ENABLE_HUSH_ਕੰਮ
#  define wait_for_ਬੱਚਾ_or_signal(pipe,pid) wait_for_ਬੱਚਾ_or_signal(pid)
# endif
ਸਥਿਰ ਪੂਰਨ wait_for_ਬੱਚਾ_or_signal(ਬਣਤਰ pipe *waitfor_pipe, pid_t waitfor_pid)
{
	ਪੂਰਨ ret = 0;
	ਲਈ (;;) {
		ਪੂਰਨ sig;
		sigset_t oldset;

		ਜੇ (!sigisemptyset(&G.pending_ਸੈੱਟ))
			ਜਾਓ check_sig;

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

		ਜੇ (!sigisemptyset(&G.pending_ਸੈੱਟ)) {
			/* Crap! we raced with some signal! */
			ਜਾਓ restore;
		}

		/*errno = 0; - checkjobs does this */
/* Can't pass waitfor_pipe into checkjobs(): it won't be interruptible */
		ret = checkjobs(ਨਿਰਾ, waitfor_pid); /* waitpid(WNOHANG) inside */
		debug_printf_exec("checkjobs:%d\n", ret);
# ਜੇ ENABLE_HUSH_ਕੰਮ
		ਜੇ (waitfor_pipe) {
			ਪੂਰਨ rcode = ਕੰਮ_exited_or_stopped(waitfor_pipe);
			debug_printf_exec("job_exited_or_stopped:%d\n", rcode);
			ਜੇ (rcode >= 0) {
				ret = rcode;
				sigprocmask(SIG_SETMASK, &oldset, ਨਿਰਾ);
				ਰੋਕੋ;
			}
		}
# endif
		/* if ECHILD, there are no children (ret is -1 or 0) */
		/* if ret == 0, no children changed state */
		/* if ret != 0, it's exitcode+1 of exited waitfor_pid child */
		ਜੇ (errno == ECHILD || ret) {
			ret--;
			ਜੇ (ret < 0) /* if ECHILD, may need to fix "ret" */
				ret = 0;
# ਜੇ ENABLE_HUSH_BASH_COMPAT
			ਜੇ (waitfor_pid == -1 && errno == ECHILD) {
				/* exitcode of "wait -n" with no children is 127, not 0 */
				ret = 127;
			}
# endif
			sigprocmask(SIG_SETMASK, &oldset, ਨਿਰਾ);
			ਰੋਕੋ;
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
		sigprocmask(SIG_SETMASK, &oldset, ਨਿਰਾ);
 check_sig:
		/* So, did we get a signal? */
		sig = check_and_ਲੜੀਵਾਰ_traps();
		ਜੇ (sig /*&& sig != SIGCHLD - always true */) {
			/* Do this for any (non-ignored) signal, not only for ^C */
			ret = 128 | sig;
			ਰੋਕੋ;
		}
		/* SIGCHLD, or no signal, or ignored one, such as SIGQUIT. Repeat */
	}
	ਵਾਪਸ ret;
}

ਸਥਿਰ ਪੂਰਨ FAST_FUNC builtin_wait(ਅੱਖਰ **argv)
{
	ਪੂਰਨ ret;
	ਪੂਰਨ ਸਥਿਤੀ;

	argv = skip_dash_dash(argv);
# ਜੇ ENABLE_HUSH_BASH_COMPAT
	ਜੇ (argv[0] && strcmp(argv[0], "-n") == 0) {
		/* wait -n */
		/* (bash accepts "wait -n PID" too and ignores PID) */
		G.dead_ਕੰਮ_exitcode = -1;
		ਵਾਪਸ wait_for_ਬੱਚਾ_or_signal(ਨਿਰਾ, -1 /*no job, wait for one job*/);
	}
# endif
	ਜੇ (argv[0] == ਨਿਰਾ) {
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
		ਵਾਪਸ wait_for_ਬੱਚਾ_or_signal(ਨਿਰਾ, 0 /*no job and no pid to wait for*/);
	}

	ਕਰੋ {
		pid_t pid = bb_strtou(*argv, ਨਿਰਾ, 10);
		ਜੇ (errno || pid <= 0) {
# ਜੇ ENABLE_HUSH_ਕੰਮ
			ਜੇ (argv[0][0] == '%') {
				ਬਣਤਰ pipe *wait_pipe;
				ret = 127; /* bash compat for bad jobspecs */
				wait_pipe = parse_jobspec(*argv);
				ਜੇ (wait_pipe) {
					ret = ਕੰਮ_exited_or_stopped(wait_pipe);
					ਜੇ (ret < 0) {
						ret = wait_for_ਬੱਚਾ_or_signal(wait_pipe, 0);
					} ਨਹੀਂਤਾਂ {
						/* waiting on "last dead job" removes it */
						clean_ਉੱਪਰ_ਆਖਰੀ_dead_ਕੰਮ();
					}
				}
				/* else: parse_jobspec() already emitted error msg */
				ਜਾਰੀ;
			}
# endif
			/* mimic bash message */
			bb_error_ਸੁਨੇਹਾ("wait: '%s': not a pid or valid job spec", *argv);
			ret = EXIT_FAILURE;
			ਜਾਰੀ; /* bash checks all argv[] */
		}

		/* Do we have such child? */
		ret = waitpid(pid, &ਸਥਿਤੀ, WNOHANG);
		ਜੇ (ret < 0) {
			/* No */
			ret = 127;
			ਜੇ (errno == ECHILD) {
				ਜੇ (pid == G.ਆਖਰੀ_bg_pid) {
					/* "wait $!" but last bg task has already exited. Try:
					 * (sleep 1; exit 3) & sleep 2; echo $?; wait $!; echo $?
					 * In bash it prints exitcode 0, then 3.
					 * In dash, it is 127.
					 */
					ret = G.ਆਖਰੀ_bg_pid_exitcode;
				} ਨਹੀਂਤਾਂ {
					/* Example: "wait 1". mimic bash message */
					bb_error_ਸੁਨੇਹਾ("wait: pid %u is not a child of this shell", (ਬਿਨਾਨਿਸ਼ਾਨੀ)pid);
				}
			} ਨਹੀਂਤਾਂ {
				/* ??? */
				bb_perror_ਸੁਨੇਹਾ("wait %s", *argv);
			}
			ਜਾਰੀ; /* bash checks all argv[] */
		}
		ਜੇ (ret == 0) {
			/* Yes, and it still runs */
			ret = wait_for_ਬੱਚਾ_or_signal(ਨਿਰਾ, pid);
		} ਨਹੀਂਤਾਂ {
			/* Yes, and it just exited */
			process_wait_ਨਤੀਜਾ(ਨਿਰਾ, pid, ਸਥਿਤੀ);
			ret = WEXITSTATUS(ਸਥਿਤੀ);
			ਜੇ (WIFSIGNALED(ਸਥਿਤੀ))
				ret = 128 | WTERMSIG(ਸਥਿਤੀ);
		}
	} ਜਦੋਂ (*++argv);

	ਵਾਪਸ ret;
}
#ਜੇ_ਅੰਤ

#ਜੇ ENABLE_HUSH_LOOPS || ENABLE_HUSH_FUNCTIONS
ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ parse_numeric_argv1(ਅੱਖਰ **argv, ਬਿਨਾਨਿਸ਼ਾਨੀ def, ਬਿਨਾਨਿਸ਼ਾਨੀ def_min)
{
	ਜੇ (argv[1]) {
		def = bb_strtou(argv[1], ਨਿਰਾ, 10);
		ਜੇ (errno || def < def_min || argv[2]) {
			bb_error_ਸੁਨੇਹਾ("%s: bad arguments", argv[0]);
			def = UINT_MAX;
		}
	}
	ਵਾਪਸ def;
}
#ਜੇ_ਅੰਤ

#ਜੇ ENABLE_HUSH_LOOPS
ਸਥਿਰ ਪੂਰਨ FAST_FUNC builtin_break(ਅੱਖਰ **argv)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਡੂੰਘਾਈ;
	ਜੇ (G.ਡੂੰਘਾਈ_ਦਾ_loop == 0) {
		bb_error_ਸੁਨੇਹਾ("%s: only meaningful in a loop", argv[0]);
		/* if we came from builtin_continue(), need to undo "= 1" */
		G.flag_break_continue = 0;
		ਵਾਪਸ EXIT_SUCCESS; /* bash compat */
	}
	G.flag_break_continue++; /* BC_BREAK = 1, or BC_CONTINUE = 2 */

	G.ਡੂੰਘਾਈ_break_continue = ਡੂੰਘਾਈ = parse_numeric_argv1(argv, 1, 1);
	ਜੇ (ਡੂੰਘਾਈ == UINT_MAX)
		G.flag_break_continue = BC_BREAK;
	ਜੇ (G.ਡੂੰਘਾਈ_ਦਾ_loop < ਡੂੰਘਾਈ)
		G.ਡੂੰਘਾਈ_break_continue = G.ਡੂੰਘਾਈ_ਦਾ_loop;

	ਵਾਪਸ EXIT_SUCCESS;
}

ਸਥਿਰ ਪੂਰਨ FAST_FUNC builtin_continue(ਅੱਖਰ **argv)
{
	G.flag_break_continue = 1; /* BC_CONTINUE = 2 = 1+1 */
	ਵਾਪਸ builtin_break(argv);
}
#ਜੇ_ਅੰਤ

#ਜੇ ENABLE_HUSH_FUNCTIONS
ਸਥਿਰ ਪੂਰਨ FAST_FUNC builtin_return(ਅੱਖਰ **argv)
{
	ਪੂਰਨ rc;

	ਜੇ (G_flag_return_ਵਿੱਚ_ਤਰੱਕੀ != -1) {
		bb_error_ਸੁਨੇਹਾ("%s: not in a function or sourced script", argv[0]);
		ਵਾਪਸ EXIT_FAILURE; /* bash compat */
	}

	G_flag_return_ਵਿੱਚ_ਤਰੱਕੀ = 1;

	/* bash:
	 * out of range: wraps around at 256, does not error out
	 * non-numeric param:
	 * f() { false; return qwe; }; f; echo $?
	 * bash: return: qwe: numeric argument required  <== we do this
	 * 255  <== we also do this
	 */
	rc = parse_numeric_argv1(argv, G.ਆਖਰੀ_exitcode, 0);
# ਜੇ ENABLE_HUSH_TRAP
	ਜੇ (argv[1]) { /* "return ARG" inside a running trap sets $? */
		debug_printf_exec("G.return_exitcode=%d\n", rc);
		G.return_exitcode = rc;
	}
# endif
	ਵਾਪਸ rc;
}
#ਜੇ_ਅੰਤ

#ਜੇ ENABLE_HUSH_TIMES
ਸਥਿਰ ਪੂਰਨ FAST_FUNC builtin_times(ਅੱਖਰ **argv ਅਣਵਰਤਿਆ_PARAM)
{
	ਸਥਿਰ ਅਚਲ uint8_t times_tbl[] ALIGN1 = {
		' ',  offsetof(ਬਣਤਰ tms, tms_utime),
		'\n', offsetof(ਬਣਤਰ tms, tms_stime),
		' ',  offsetof(ਬਣਤਰ tms, tms_cutime),
		'\n', offsetof(ਬਣਤਰ tms, tms_cstime),
		0
	};
	ਅਚਲ uint8_t *p;
	ਬਿਨਾਨਿਸ਼ਾਨੀ clk_tck;
	ਬਣਤਰ tms buf;

	clk_tck = bb_clk_tck();

	times(&buf);
	p = times_tbl;
	ਕਰੋ {
		ਬਿਨਾਨਿਸ਼ਾਨੀ sec, frac;
		ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ t;
		t = *(clock_t *)(((ਅੱਖਰ *) &buf) + p[1]);
		sec = t / clk_tck;
		frac = t % clk_tck;
		ਲਿਖੋ("%um%u.%03us%c",
			sec / 60, sec % 60,
			(frac * 1000) / clk_tck,
			p[0]);
		p += 2;
	} ਜਦੋਂ (*p);

	ਵਾਪਸ EXIT_SUCCESS;
}
#ਜੇ_ਅੰਤ

#ਜੇ ENABLE_HUSH_MEMLEAK
ਸਥਿਰ ਪੂਰਨ FAST_FUNC builtin_memleak(ਅੱਖਰ **argv ਅਣਵਰਤਿਆ_PARAM)
{
	ਖਾਲੀ *p;
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ l;

# ifdef M_TRIM_THRESHOLD
	/* Optional. Reduces probability of false positives */
	malloc_trim(0);
# endif
	/* Crude attempt to find where "free memory" starts,
	 * sans fragmentation. */
	p = ਸਮ੍ਰਿਤੀ_ਦਿਓ(240);
	l = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)p;
	ਮੁਕਤ(p);
	p = ਸਮ੍ਰਿਤੀ_ਦਿਓ(3400);
	ਜੇ (l < (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)p) l = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)p;
	ਮੁਕਤ(p);

# ਜੇ 0  /* debug */
	{
		ਬਣਤਰ mallinfo mi = mallinfo();
		ਲਿਖੋ("top alloc:0x%lx malloced:%d+%d=%d\n", l,
			mi.arena, mi.hblkhd, mi.arena + mi.hblkhd);
	}
# endif

	ਜੇ (!G.memleak_ਮੁੱਲ)
		G.memleak_ਮੁੱਲ = l;

	l -= G.memleak_ਮੁੱਲ;
	ਜੇ ((ਲੰਮਾ)l < 0)
		l = 0;
	l /= 1024;
	ਜੇ (l > 127)
		l = 127;

	/* Exitcode is "how many kilobytes we leaked since 1st call" */
	ਵਾਪਸ l;
}
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ /* !__U_BOOT__ */
