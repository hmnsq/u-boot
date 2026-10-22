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

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
#ಆದರೆ !(defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__) \
	|| defined(__APPLE__) \
    )
# include <malloc.h>   /* for malloc_trim */
#ಆದರೆ_ಅಂತ್ಯ
#ಅನ್ತರ್ಭಾವ <glob.h>
/* #include <dmalloc.h> */
#ಆದರೆ ENABLE_HUSH_CASE
# include <fnmatch.h>
#ಆದರೆ_ಅಂತ್ಯ
#ಅನ್ತರ್ಭಾವ <sys/times.h>
#ಅನ್ತರ್ಭಾವ <sys/utsname.h> /* for setting $HOSTNAME */

#ಅನ್ತರ್ಭಾವ "busybox.h"  /* for APPLET_IS_NOFORK/NOEXEC */
#ಅನ್ತರ್ಭಾವ "unicode.h"
#ಅನ್ತರ್ಭಾವ "shell_common.h"
#ಅನ್ತರ್ಭಾವ "math.h"
#ಅನ್ತರ್ಭಾವ "match.h"
#ಆದರೆ ENABLE_HUSH_RANDOM_SUPPORT
# include "random.h"
#ಇಲ್ಲದಿದ್ದರೆ
# define CLEAR_RANDOM_T(rnd) ((ಶೂನ್ಯ)0)
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ O_CLOEXEC
# define O_CLOEXEC 0
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ F_DUPFD_CLOEXEC
# define F_DUPFD_CLOEXEC F_DUPFD
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ ENABLE_FEATURE_SH_EMBEDDED_SCRIPTS && !ENABLE_SHELL_ASH
# include "embedded_scripts.h"
#ಇಲ್ಲದಿದ್ದರೆ
# define ಅಂಕೆ_SCRIPTS 0
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */

/* So far, all bash compat is controlled by one config option */
/* Separate defines document which part of code implements what */
#ವ್ಯಾಖ್ಯಾನಿಸು BASH_ನಮೂನೆ_SUBST ENABLE_HUSH_BASH_COMPAT
#ವ್ಯಾಖ್ಯಾನಿಸು BASH_SUBSTR        ENABLE_HUSH_BASH_COMPAT
#ವ್ಯಾಖ್ಯಾನಿಸು BASH_ಮೂಲ        ENABLE_HUSH_BASH_COMPAT
#ವ್ಯಾಖ್ಯಾನಿಸು BASH_DOLLAR_SQUOTE ENABLE_HUSH_BASH_COMPAT
#ವ್ಯಾಖ್ಯಾನಿಸು BASH_HOSTNAME_VAR  ENABLE_HUSH_BASH_COMPAT
#ವ್ಯಾಖ್ಯಾನಿಸು BASH_EPOCH_VARS    ENABLE_HUSH_BASH_COMPAT
#ವ್ಯಾಖ್ಯಾನಿಸು BASH_TEST2         (ENABLE_HUSH_BASH_COMPAT && ENABLE_HUSH_TEST)
#ವ್ಯಾಖ್ಯಾನಿಸು BASH_READ_D        ENABLE_HUSH_BASH_COMPAT

/* Build knobs */
#ವ್ಯಾಖ್ಯಾನಿಸು LEAK_HUNTING 0
#ವ್ಯಾಖ್ಯಾನಿಸು BUILD_AS_NOMMU 0
/* Enable/disable sanity checks. Ok to enable in production,
 * only adds a bit of bloat. Set to >1 to get non-production level verbosity.
 * Keeping 1 for now even in released versions.
 */
#ವ್ಯಾಖ್ಯಾನಿಸು HUSH_DEBUG 1
/* Slightly bigger (+200 bytes), but faster hush.
 * So far it only enables a trick with counting SIGCHLDs and forks,
 * which allows us to do fewer waitpid's.
 * (we can detect a case where neither forks were done nor SIGCHLDs happened
 * and therefore waitpid will return the same result as last time)
 */
#ವ್ಯಾಖ್ಯಾನಿಸು ENABLE_HUSH_FAST 0
/* TODO: implement simplified code for users which do not need ${var%...} ops
 * So far ${var%...} ops are always enabled:
 */
#ವ್ಯಾಖ್ಯಾನಿಸು ENABLE_HUSH_DOLLAR_OPS 1

#ಆದರೆ BUILD_AS_NOMMU
# undef BB_MMU
# undef USE_FOR_NOMMU
# undef USE_FOR_MMU
# define BB_MMU 0
# define USE_FOR_NOMMU(...) __VA_ARGS__
# define USE_FOR_MMU(...)
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
#ಅನ್ತರ್ಭಾವ "NUM_APPLETS.h"
#ಆದರೆ ಅಂಕೆ_APPLETS == 1
/* STANDALONE does not make sense, and won't compile */
# undef ENABLE_FEATURE_SH_STANDALONE
# undef IF_FEATURE_SH_STANDALONE
# undef IF_NOT_FEATURE_SH_STANDALONE
# define ENABLE_FEATURE_SH_STANDALONE 0
# define IF_FEATURE_SH_STANDALONE(...)
# define IF_NOT_FEATURE_SH_STANDALONE(...) __VA_ARGS__
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ /* __U_BOOT__ */

#ಆದರೆ !ENABLE_HUSH_INTERACTIVE
# undef ENABLE_FEATURE_EDITING
# define ENABLE_FEATURE_EDITING 0
# undef ENABLE_FEATURE_EDITING_FANCY_PROMPT
# define ENABLE_FEATURE_EDITING_FANCY_PROMPT 0
# undef ENABLE_FEATURE_EDITING_SAVE_ON_EXIT
# define ENABLE_FEATURE_EDITING_SAVE_ON_EXIT 0
#ಆದರೆ_ಅಂತ್ಯ

/* Do we support ANY keywords? */
#ಆದರೆ ENABLE_HUSH_IF || ENABLE_HUSH_LOOPS || ENABLE_HUSH_CASE
# define HAS_KEYWORDS 1
# define IF_HAS_KEYWORDS(...) __VA_ARGS__
# define IF_HAS_NO_KEYWORDS(...)
#ಇಲ್ಲದಿದ್ದರೆ
# define HAS_KEYWORDS 0
# define IF_HAS_KEYWORDS(...)
# define IF_HAS_NO_KEYWORDS(...) __VA_ARGS__
#ಆದರೆ_ಅಂತ್ಯ

/* If you comment out one of these below, it will be #defined later
 * to perform debug printfs to stderr: */
#ವ್ಯಾಖ್ಯಾನಿಸು debug_printf(...)         ಮಾಡು {} ಆಗ (0)
/* Finer-grained debug switches */
#ವ್ಯಾಖ್ಯಾನಿಸು debug_printf_parse(...)   ಮಾಡು {} ಆಗ (0)
#ವ್ಯಾಖ್ಯಾನಿಸು debug_printf_heredoc(...) ಮಾಡು {} ಆಗ (0)
#ವ್ಯಾಖ್ಯಾನಿಸು debug_print_ವೃಕ್ಷ(a, b)    ಮಾಡು {} ಆಗ (0)
#ವ್ಯಾಖ್ಯಾನಿಸು debug_printf_exec(...)    ಮಾಡು {} ಆಗ (0)
#ವ್ಯಾಖ್ಯಾನಿಸು debug_printf_env(...)     ಮಾಡು {} ಆಗ (0)
#ವ್ಯಾಖ್ಯಾನಿಸು debug_printf_jobs(...)    ಮಾಡು {} ಆಗ (0)
#ವ್ಯಾಖ್ಯಾನಿಸು debug_printf_expand(...)  ಮಾಡು {} ಆಗ (0)
#ವ್ಯಾಖ್ಯಾನಿಸು debug_printf_varexp(...)  ಮಾಡು {} ಆಗ (0)
#ವ್ಯಾಖ್ಯಾನಿಸು debug_printf_glob(...)    ಮಾಡು {} ಆಗ (0)
#ವ್ಯಾಖ್ಯಾನಿಸು debug_printf_redir(...)   ಮಾಡು {} ಆಗ (0)
#ವ್ಯಾಖ್ಯಾನಿಸು debug_printf_list(...)    ಮಾಡು {} ಆಗ (0)
#ವ್ಯಾಖ್ಯಾನಿಸು debug_printf_subst(...)   ಮಾಡು {} ಆಗ (0)
#ವ್ಯಾಖ್ಯಾನಿಸು debug_printf_prompt(...)  ಮಾಡು {} ಆಗ (0)
#ವ್ಯಾಖ್ಯಾನಿಸು debug_printf_clean(...)   ಮಾಡು {} ಆಗ (0)

#ವ್ಯಾಖ್ಯಾನಿಸು ERR_PTR ((ಶೂನ್ಯ*)(ದೀರ್ಘ)1)

#ವ್ಯಾಖ್ಯಾನಿಸು ಕೆಲಸ_ಸ್ಥಿತಿ_FORMAT    "[%u] %-22s %.40s\n"

#ವ್ಯಾಖ್ಯಾನಿಸು _SPECIAL_VARS_STR     "_*@$!?#-"
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
#ವ್ಯಾಖ್ಯಾನಿಸು SPECIAL_VARS_STR     ("_*@$!?#-" + 1)
#ವ್ಯಾಖ್ಯಾನಿಸು NUMERIC_SPECVARS_STR ("_*@$!?#-" + 3)
#ಇಲ್ಲದಿದ್ದರೆ /* __U_BOOT__ */
#ವ್ಯಾಖ್ಯಾನಿಸು SPECIAL_VARS_STR     "*@$!?#-"
#ವ್ಯಾಖ್ಯಾನಿಸು NUMERIC_SPECVARS_STR "$!?#-"
#ಆದರೆ_ಅಂತ್ಯ /* __U_BOOT__ */
#ಆದರೆ BASH_ನಮೂನೆ_SUBST
/* Support / and // replace ops */
/* Note that // is stored as \ in "encoded" string representation */
# define VAR_ENCODED_SUBST_OPS      "\\/%#:-=+?"
# define VAR_SUBST_OPS             ("\\/%#:-=+?" + 1)
# define MINUS_PLUS_EQUAL_QUESTION ("\\/%#:-=+?" + 5)
#ಇಲ್ಲದಿದ್ದರೆ
# define VAR_ENCODED_SUBST_OPS      "%#:-=+?"
# define VAR_SUBST_OPS              "%#:-=+?"
# define MINUS_PLUS_EQUAL_QUESTION ("%#:-=+?" + 3)
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿಸು SPECIAL_VAR_ಚಿಹ್ನೆ_STR "\3"
#ವ್ಯಾಖ್ಯಾನಿಸು SPECIAL_VAR_ಚಿಹ್ನೆ       3
/* The "variable" with name "\1" emits string "\3". Testcase: "echo ^C" */
#ವ್ಯಾಖ್ಯಾನಿಸು SPECIAL_VAR_QUOTED_SVS   1

ರಚನೆ variable;

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ hush_ಆವೃತ್ತಿ_str[] ALIGN1 = "HUSH_VERSION="BB_VER;

/* This supports saving pointers malloced in vfork child,
 * to be freed in the parent.
 */
#ಆದರೆ !BB_MMU
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ nommu_save_t {
	ರಚನೆ variable *old_vars;
	ಅಕ್ಷರ **argv;
	ಅಕ್ಷರ **argv_from_re_execing;
} nommu_save_t;
#ಆದರೆ_ಅಂತ್ಯ

ಎಣಿಕೆ {
	RES_NONE  = 0,
#ಆದರೆ ENABLE_HUSH_IF
	RES_IF    ,
	RES_THEN  ,
	RES_ELIF  ,
	RES_ELSE  ,
	RES_FI    ,
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_HUSH_LOOPS
	RES_FOR   ,
	RES_WHILE ,
	RES_UNTIL ,
	RES_DO    ,
	RES_ಮುಗಿದ  ,
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_HUSH_LOOPS || ENABLE_HUSH_CASE
	RES_ಒಳ    ,
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_HUSH_CASE
	RES_CASE  ,
	/* three pseudo-keywords support contrived "case" syntax: */
	RES_CASE_ಒಳ,   /* "case ... IN", turns into RES_MATCH when IN is observed */
	RES_ಹೊಂದಿಕೆ ,    /* "word)" */
	RES_CASE_BODY, /* "this command is inside CASE" */
	RES_ESAC  ,
#ಆದರೆ_ಅಂತ್ಯ
	RES_XXXX  ,
	RES_SNTX
};

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ o_string {
	ಅಕ್ಷರ *ದತ್ತಾಂಶ;
	ಪೂರ್ಣಾಂಕ ಉದ್ದಳತೆ; /* position where data is appended */
	ಪೂರ್ಣಾಂಕ maxlen;
	ಪೂರ್ಣಾಂಕ o_expflags;
	/* At least some part of the string was inside '' or "",
	 * possibly empty one: word"", wo''rd etc. */
	smallint has_quoted_part;
	smallint has_ಖಾಲಿಗಳು_ಸ್ಥಾನ;
	smallint ended_ಒಳ_ifs;
} o_string;
ಎಣಿಕೆ {
	EXP_FLAG_SINGLEWORD     = 0x80, /* must be 0x80 */
	EXP_FLAG_GLOB           = 0x2,
	/* Protect newly added chars against globbing
	 * by prepending \ to *, ?, [, \ */
	EXP_FLAG_ESC_GLOB_CHARS = 0x1,
};
/* Used for initialization: o_string foo = NULL_O_STRING; */
#ವ್ಯಾಖ್ಯಾನಿಸು NULL_O_STRING { ಖಾಲಿ }

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ debug_printf_parse
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ assignment_flag[] ALIGN_PTR = {
	"MAYBE_ASSIGNMENT",
	"DEFINITELY_ASSIGNMENT",
	"NOT_ASSIGNMENT",
	"WORD_IS_KEYWORD",
};
#ಆದರೆ_ಅಂತ್ಯ

/* We almost can use standard FILE api, but we need an ability to move
 * its fd when redirects coincide with it. No api exists for that
 * (RFE for it at https://sourceware.org/bugzilla/show_bug.cgi?id=21902).
 * HFILE is our internal alternative. Only supports reading.
 * Since we now can, we incorporate linked list of all opened HFILEs
 * into the struct (used to be a separate mini-list).
 */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ HFILE {
	ಅಕ್ಷರ *cur;
	ಅಕ್ಷರ *ಕೊನೆ_2;
	ರಚನೆ HFILE *ಮುಂದಿನ_hfile;
	ಪೂರ್ಣಾಂಕ fd;
	ಅಕ್ಷರ buf[1024];
} HFILE;

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ ಒಳ_str {
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p;
	ಪೂರ್ಣಾಂಕ peek_buf[2];
	ಪೂರ್ಣಾಂಕ ಕೊನೆಯ_char;
	HFILE *file;
} ಒಳ_str;

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
/* The descrip member of this structure is only used to make
 * debugging output pretty */
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ {
	int32_t ಬಹುಲಕ;
	ಚಿಹ್ನೆಯುಳ್ಳ ಅಕ್ಷರ default_fd;
	ಅಕ್ಷರ descrip[3];
} redir_ಕೋಷ್ಟಕ[] ALIGN4 = {
	{ O_RDONLY,                  0, "<"  },
	{ O_CREAT|O_TRUNC|O_WRONLY,  1, ">"  },
	{ O_CREAT|O_APPEND|O_WRONLY, 1, ">>" },
	{ O_CREAT|O_RDWR,            1, "<>" },
	{ O_RDONLY,                  0, "<<" },
/* Should not be needed. Bogus default_fd helps in debugging */
/*	{ O_RDONLY,                 77, "<<" }, */
};

ರಚನೆ redir_struct {
	ರಚನೆ redir_struct *ಮುಂದಿನ;
	ಅಕ್ಷರ *rd_filename;          /* filename */
	ಪೂರ್ಣಾಂಕ rd_fd;                  /* fd to redirect */
	/* fd to redirect to, or -3 if rd_fd is to be closed (n>&-) */
	ಪೂರ್ಣಾಂಕ rd_dup;
	smallint rd_type;           /* (enum redir_type) */
	/* note: for heredocs, rd_filename contains heredoc delimiter,
	 * and subsequently heredoc itself; and rd_dup is a bitmask:
	 * bit 0: do we need to trim leading tabs?
	 * bit 1: is heredoc quoted (<<'delim' syntax) ?
	 */
};
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಎಣಿಕೆ redir_type {
	REDIRECT_ಒಳಹರಿವು     = 0,
	REDIRECT_OVERWRITE = 1,
	REDIRECT_APPEND    = 2,
	REDIRECT_IO        = 3,
	REDIRECT_HEREDOC   = 4,
	REDIRECT_HEREDOC2  = 5, /* REDIRECT_HEREDOC after heredoc is loaded */

	REDIRFD_CLOSE      = -3,
	REDIRFD_SYNTAX_ERR = -2,
	REDIRFD_ಗಮ್ಯ_FILE    = -1,
	/* otherwise, rd_fd is redirected to rd_dup */

	HEREDOC_SKIPTABS = 1,
	HEREDOC_QUOTED   = 2,
} redir_type;

#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */

ರಚನೆ command {
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	pid_t pid;                  /* 0 if exited */
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */
	ಚಿಹ್ನೆರಹಿತ assignment_cnt;    /* how many argv[i] are assignments? */
#ಆದರೆ ENABLE_HUSH_LINENO_VAR
	ಚಿಹ್ನೆರಹಿತ lineno;
#ಆದರೆ_ಅಂತ್ಯ
	smallint cmd_type;          /* CMD_xxx */
#ವ್ಯಾಖ್ಯಾನಿಸು CMD_ಸಾಮಾನ್ಯ   0
#ವ್ಯಾಖ್ಯಾನಿಸು CMD_SUBSHELL 1
#ಆದರೆ BASH_TEST2
/* used for "[[ EXPR ]]" */
# define CMD_TEST2_SINGLEWORD_NOGLOB 2
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ BASH_TEST2 || ENABLE_HUSH_LOCAL || ENABLE_HUSH_EXPORT || ENABLE_HUSH_READONLY
/* used to prevent word splitting and globbing in "export v=t*" */
# define CMD_SINGLEWORD_NOGLOB 3
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_HUSH_FUNCTIONS
# define CMD_FUNCDEF 4
#ಆದರೆ_ಅಂತ್ಯ

	smalluint cmd_exitcode;
	/* if non-NULL, this "command" is { list }, ( list ), or a compound statement */
	ರಚನೆ pipe *ಗುಂಪು;
#ಆದರೆ !BB_MMU
	ಅಕ್ಷರ *ಗುಂಪು_as_string;
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_HUSH_FUNCTIONS
	ರಚನೆ function *ಮಗು_func;
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
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __U_BOOT__
	ಪೂರ್ಣಾಂಕ argc; /* number of program arguments */
#ಆದರೆ_ಅಂತ್ಯ
	ಅಕ್ಷರ **argv;                /* command name and arguments */
/* argv vector may contain variable references (^Cvar^C, ^C0^C etc)
 * and on execution these are substituted with their values.
 * Substitution can make _several_ words out of one argv[n]!
 * Example: argv[0]=='.^C*^C.' here: echo .$*.
 * References of the form ^C`cmd arg^C are `cmd arg` substitutions.
 */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	ರಚನೆ redir_struct *redirects; /* I/O redirections */
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */
};
/* Is there anything in this command at all? */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
#ವ್ಯಾಖ್ಯಾನಿಸು IS_NULL_CMD(cmd) \
	(!(cmd)->ಗುಂಪು && !(cmd)->argv && !(cmd)->redirects)

#ಇಲ್ಲದಿದ್ದರೆ /* __U_BOOT__ */
#ವ್ಯಾಖ್ಯಾನಿಸು IS_NULL_CMD(cmd) \
	(!(cmd)->ಗುಂಪು && !(cmd)->argv)
#ಆದರೆ_ಅಂತ್ಯ /* __U_BOOT__ */
ರಚನೆ pipe {
	ರಚನೆ pipe *ಮುಂದಿನ;
	ಪೂರ್ಣಾಂಕ ಅಂಕೆ_cmds;               /* total number of commands in pipe */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	ಪೂರ್ಣಾಂಕ alive_cmds;             /* number of commands running (not exited) */
	ಪೂರ್ಣಾಂಕ stopped_cmds;           /* number of commands alive, but stopped */
#ಆದರೆ ENABLE_HUSH_ಕೆಲಸ
	ಚಿಹ್ನೆರಹಿತ jobid;             /* job number */
	pid_t pgrp;                 /* process group ID for the job */
	ಅಕ್ಷರ *cmdtext;              /* name of job */
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */
	ರಚನೆ command *cmds;       /* array of commands in pipe */
	smallint followup;          /* PIPE_BG, PIPE_SEQ, PIPE_OR, PIPE_AND */
	IF_HAS_KEYWORDS(smallint pi_inverted;) /* "! cmd | cmd" */
	IF_HAS_KEYWORDS(smallint res_ಶಬ್ದ;) /* needed for if, for, while, until... */
};
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಎಣಿಕೆ pipe_style {
	PIPE_SEQ = 0,
	PIPE_AND = 1,
	PIPE_OR  = 2,
	PIPE_BG  = 3,
} pipe_style;
/* Is there anything in this pipe at all? */
#ವ್ಯಾಖ್ಯಾನಿಸು IS_NULL_PIPE(pi) \
	((pi)->ಅಂಕೆ_cmds == 0 IF_HAS_KEYWORDS( && (pi)->res_ಶಬ್ದ == RES_NONE))

/* This holds pointers to the various results of parsing */
ರಚನೆ parse_context {
	/* linked list of pipes */
	ರಚನೆ pipe *list_ತಲೆ;
	/* last pipe (being constructed right now) */
	ರಚನೆ pipe *pipe;
	/* last command in pipe (being constructed right now) */
	ರಚನೆ command *command;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	/* last redirect in command->redirects list */
	ರಚನೆ redir_struct *pending_redirect;
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */
	o_string ಶಬ್ದ;
#ಆದರೆ !BB_MMU
	o_string as_string;
#ಆದರೆ_ಅಂತ್ಯ
	smallint is_assignment; /* 0:maybe, 1:yes, 2:no, 3:keyword */
#ಆದರೆ HAS_KEYWORDS
	smallint ctx_res_w;
	smallint ctx_inverted; /* "! cmd | cmd" */
#ಆದರೆ ENABLE_HUSH_CASE
	smallint ctx_dsemicolon; /* ";;" seen */
#ಆದರೆ_ಅಂತ್ಯ
	/* bitmask of FLAG_xxx, for figuring out valid reserved words */
	ಪೂರ್ಣಾಂಕ old_flag;
	/* group we are enclosed in:
	 * example: "if pipe1; pipe2; then pipe3; fi"
	 * when we see "if" or "then", we malloc and copy current context,
	 * and make ->stack point to it. then we parse pipeN.
	 * when closing "then" / fi" / whatever is found,
	 * we move list_head into ->stack->command->group,
	 * copy ->stack into current context, and delete ->stack.
	 * (parsing of { list } and ( list ) doesn't use this method)
	 */
	ರಚನೆ parse_context *ರಾಶಿ;
#ಆದರೆ_ಅಂತ್ಯ
};
ಎಣಿಕೆ {
	MAYBE_ASSIGNMENT      = 0,
	DEFINITELY_ASSIGNMENT = 1,
	NOT_ASSIGNMENT        = 2,
	/* Not an assignment, but next word may be: "if v=xyz cmd;" */
	ಶಬ್ದ_IS_KEYWORD       = 3,
};

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
/* On program start, environ points to initial environment.
 * putenv adds new pointers into it, unsetenv removes them.
 * Neither of these (de)allocates the strings.
 * setenv allocates new strings in malloc space and does putenv,
 * and thus setenv is unusable (leaky) for shell's purposes */
#ವ್ಯಾಖ್ಯಾನಿಸು setenv(...) setenv_is_leaky_dont_use()
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */
ರಚನೆ variable {
	ರಚನೆ variable *ಮುಂದಿನ;
	ಅಕ್ಷರ *varstr;        /* points to "name=" portion */
	ಪೂರ್ಣಾಂಕ max_len;         /* if > 0, name is part of initial env; else name is malloced */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	uint16_t var_nest_level;
	smallint flg_export; /* putenv should be done on this var */
	smallint flg_read_only;
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */
};

ಎಣಿಕೆ {
	BC_BREAK = 1,
	BC_CONTINUE = 2,
};

#ಆದರೆ ENABLE_HUSH_FUNCTIONS
ರಚನೆ function {
	ರಚನೆ function *ಮುಂದಿನ;
	ಅಕ್ಷರ *name;
	ರಚನೆ command *ಪೋಷಕ_cmd;
	ರಚನೆ pipe *body;
# ಆದರೆ !BB_MMU
	ಅಕ್ಷರ *body_as_string;
# endif
};
#ಆದರೆ_ಅಂತ್ಯ

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
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ o_opt_ಅಕ್ಷರಸರಣಿಗಳು[] ALIGN1 =
	"pipefail\0"
	"noexec\0"
	"errexit\0"
#ಆದರೆ ENABLE_HUSH_ಬಹುಲಕ_X
	"xtrace\0"
#ಆದರೆ_ಅಂತ್ಯ
	;
ಎಣಿಕೆ {
	OPT_O_PIPEFAIL,
	OPT_O_NOEXEC,
	OPT_O_ERREXIT,
#ಆದರೆ ENABLE_HUSH_ಬಹುಲಕ_X
	OPT_O_XTRACE,
#ಆದರೆ_ಅಂತ್ಯ
	ಅಂಕೆ_OPT_O
};

/* "Globals" within this file */
/* Sorted roughly by size (smaller offsets == smaller code) */
ರಚನೆ globals {
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
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
#ಆದರೆ ENABLE_HUSH_INTERACTIVE
	/* 'interactive_fd' is a fd# open to ctty, if we have one
	 * _AND_ if we decided to act interactively */
	ಪೂರ್ಣಾಂಕ interactive_fd;
	IF_NOT_FEATURE_EDITING_FANCY_PROMPT(ಅಕ್ಷರ *PS1;)
# define G_interactive_fd (G.interactive_fd)
#ಇಲ್ಲದಿದ್ದರೆ
# define G_interactive_fd 0
#ಆದರೆ_ಅಂತ್ಯ
#ಇಲ್ಲದಿದ್ದರೆ /* __U_BOOT__ */
# define G_interactive_fd 0
#ಆದರೆ_ಅಂತ್ಯ /* __U_BOOT__ */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
#ಆದರೆ ENABLE_FEATURE_EDITING
	ಸಾಲು_ಒಳಹರಿವು_t *ಸಾಲು_ಒಳಹರಿವು_state;
#ಆದರೆ_ಅಂತ್ಯ
	pid_t ಬೇರು_pid;
	pid_t ಬೇರು_ppid;
	pid_t ಕೊನೆಯ_bg_pid;
#ಆದರೆ ENABLE_HUSH_RANDOM_SUPPORT
	random_t random_gen;
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_HUSH_ಕೆಲಸ
	ಪೂರ್ಣಾಂಕ ಸತತ_list_level;
	ಚಿಹ್ನೆರಹಿತ ಕೊನೆಯ_jobid;
	pid_t saved_tty_pgrp;
	ರಚನೆ pipe *ಕೆಲಸ_list;
# define G_saved_tty_pgrp (G.saved_tty_pgrp)
#ಇಲ್ಲದಿದ್ದರೆ
# define G_saved_tty_pgrp 0
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */
	/* How deeply are we in context where "set -e" is ignored */
	ಪೂರ್ಣಾಂಕ errexit_ಆಳ;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
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

	ಅಕ್ಷರ o_opt[ಅಂಕೆ_OPT_O];
#ಆದರೆ ENABLE_HUSH_ಬಹುಲಕ_X
# define G_x_ಬಹುಲಕ (G.o_opt[OPT_O_XTRACE])
#ಇಲ್ಲದಿದ್ದರೆ
# define G_x_ಬಹುಲಕ 0
#ಆದರೆ_ಅಂತ್ಯ
	ಅಕ್ಷರ opt_s;
	ಅಕ್ಷರ opt_c;
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */
#ಆದರೆ ENABLE_HUSH_INTERACTIVE
	smallint promptmode; /* 0: PS1, 1: PS2 */
#ಆದರೆ_ಅಂತ್ಯ
	/* set by signal handler if SIGINT is received _and_ its trap is not set */
	smallint flag_SIGINT;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
#ಆದರೆ ENABLE_HUSH_LOOPS
	smallint flag_break_continue;
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */
#ಆದರೆ ENABLE_HUSH_FUNCTIONS
	/* 0: outside of a function (or sourced file)
	 * -1: inside of a function, ok to use return builtin
	 * 1: return is invoked, skip all till end of func
	 */
	smallint flag_return_ಒಳ_ಪ್ರಗತಿ;
# define G_flag_return_ಒಳ_ಪ್ರಗತಿ (G.flag_return_ಒಳ_ಪ್ರಗತಿ)
#ಇಲ್ಲದಿದ್ದರೆ
# define G_flag_return_ಒಳ_ಪ್ರಗತಿ 0
#ಆದರೆ_ಅಂತ್ಯ
	smallint exiting; /* used to prevent EXIT trap recursion */
	/* These support $? */
	smalluint ಕೊನೆಯ_exitcode;
	smalluint expand_exitcode;
	smalluint ಕೊನೆಯ_bg_pid_exitcode;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
#ಆದರೆ ENABLE_HUSH_ಗಣ
	/* are global_argv and global_argv[1..n] malloced? (note: not [0]) */
	smalluint global_ವಾದಗಳು_malloced;
# define G_global_ವಾದಗಳು_malloced (G.global_ವಾದಗಳು_malloced)
#ಇಲ್ಲದಿದ್ದರೆ
# define G_global_ವಾದಗಳು_malloced 0
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_HUSH_BASH_COMPAT
	ಪೂರ್ಣಾಂಕ dead_ಕೆಲಸ_exitcode; /* for "wait -n" */
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */
	/* how many non-NULL argv's we have. NB: $# + 1 */
	ಪೂರ್ಣಾಂಕ global_argc;
	ಅಕ್ಷರ **global_argv;
#ಆದರೆ !BB_MMU
	ಅಕ್ಷರ *argv0_for_re_execing;
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_HUSH_LOOPS
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	ಚಿಹ್ನೆರಹಿತ ಆಳ_break_continue;
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */
	ಚಿಹ್ನೆರಹಿತ ಆಳ_ದ_loop;
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
#ಆದರೆ ENABLE_HUSH_GETOPTS
	ಚಿಹ್ನೆರಹಿತ getopt_ಎಣಿಕೆ;
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ifs;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __U_BOOT__
	ಪೂರ್ಣಾಂಕ flag_repeat;
	ಪೂರ್ಣಾಂಕ do_repeat;
	ಪೂರ್ಣಾಂಕ ಸತತ_command_flags;
#ಆದರೆ_ಅಂತ್ಯ /* __U_BOOT__ */
	ಅಕ್ಷರ *ifs_whitespace; /* = G.ifs or malloced */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *cwd;
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */
	ರಚನೆ variable *ತುದಿ_var;
	ಅಕ್ಷರ **expanded_assignments;
	ರಚನೆ variable **shadowed_vars_pp;
	ಚಿಹ್ನೆರಹಿತ var_nest_level;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
#ಆದರೆ ENABLE_HUSH_FUNCTIONS
# ಆದರೆ ENABLE_HUSH_LOCAL
	ಚಿಹ್ನೆರಹಿತ func_nest_level; /* solely to prevent "local v" in non-functions */
# endif
	ರಚನೆ function *ತುದಿ_func;
#ಆದರೆ_ಅಂತ್ಯ
	/* Signal and trap handling */
#ಆದರೆ ENABLE_HUSH_FAST
	ಚಿಹ್ನೆರಹಿತ ಎಣಿಕೆ_SIGCHLD;
	ಚಿಹ್ನೆರಹಿತ handled_SIGCHLD;
	smallint we_have_children;
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_HUSH_LINENO_VAR
	ಚಿಹ್ನೆರಹಿತ parse_lineno;
	ಚಿಹ್ನೆರಹಿತ execute_lineno;
#ಆದರೆ_ಅಂತ್ಯ
	HFILE *HFILE_list;
	HFILE *HFILE_stdin;
	/* Which signals have non-DFL handler (even with no traps set)?
	 * Set at the start to:
	 * (SIGQUIT + maybe SPECIAL_INTERACTIVE_SIGS + maybe SPECIAL_JOBSTOP_SIGS)
	 * SPECIAL_INTERACTIVE_SIGS are cleared after fork.
	 * The rest is cleared right before execv syscalls.
	 * Other than these two times, never modified.
	 */
	ಚಿಹ್ನೆರಹಿತ special_sig_ಮುಖವಾಡ;
#ಆದರೆ ENABLE_HUSH_ಕೆಲಸ
	ಚಿಹ್ನೆರಹಿತ fatal_sig_ಮುಖವಾಡ;
# define G_fatal_sig_ಮುಖವಾಡ (G.fatal_sig_ಮುಖವಾಡ)
#ಇಲ್ಲದಿದ್ದರೆ
# define G_fatal_sig_ಮುಖವಾಡ 0
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_HUSH_TRAP
	ಪೂರ್ಣಾಂಕ pre_trap_exitcode;
# ಆದರೆ ENABLE_HUSH_FUNCTIONS
	ಪೂರ್ಣಾಂಕ return_exitcode;
# endif
	ಅಕ್ಷರ **traps; /* char *traps[NSIG] */
# define G_traps G.traps
#ಇಲ್ಲದಿದ್ದರೆ
# define G_traps ((ಅಕ್ಷರ**)ಖಾಲಿ)
#ಆದರೆ_ಅಂತ್ಯ
	sigset_t pending_ಗಣ;
#ಆದರೆ ENABLE_HUSH_MEMLEAK
	ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ memleak_ಮೌಲ್ಯ;
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_HUSH_ಬಹುಲಕ_X
	ಚಿಹ್ನೆರಹಿತ x_ಬಹುಲಕ_ಆಳ;
	/* "set -x" output should not be redirectable with subsequent 2>FILE.
	 * We dup fd#2 to x_mode_fd when "set -x" is executed, and use it
	 * for all subsequent output.
	 */
	ಪೂರ್ಣಾಂಕ x_ಬಹುಲಕ_fd;
	o_string x_ಬಹುಲಕ_buf;
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */
#ಆದರೆ HUSH_DEBUG >= 2
	ಪೂರ್ಣಾಂಕ debug_indent;
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	ರಚನೆ sigaction sa;
	ಅಕ್ಷರ optstring_buf[ಗಾತ್ರ("eixcs")];
#ಆದರೆ BASH_EPOCH_VARS
	ಅಕ್ಷರ epoch_buf[ಗಾತ್ರ("%llu.nnnnnn") + ಗಾತ್ರ(ದೀರ್ಘ ದೀರ್ಘ)*3];
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_FEATURE_EDITING
	ಅಕ್ಷರ user_ಒಳಹರಿವು_buf[CONFIG_FEATURE_EDITING_MAX_LEN];
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */
};
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __U_BOOT__
ರಚನೆ globals *ptr_ಗಮ್ಯ_globals;
#ಆದರೆ_ಅಂತ್ಯ /* __U_BOOT__ */
#ವ್ಯಾಖ್ಯಾನಿಸು G (*ptr_ಗಮ್ಯ_globals)
/* Not #defining name to G.name - this quickly gets unwieldy
 * (too many defines). Also, I actually prefer to see when a variable
 * is global, thus "G." prefix is a useful hint */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __U_BOOT__
#ವ್ಯಾಖ್ಯಾನಿಸು ಗಣ_PTR_ಗಮ್ಯ_GLOBALS(x) ಮಾಡು { \
	(*(ರಚನೆ globals**)&ptr_ಗಮ್ಯ_globals) = (ಶೂನ್ಯ*)(x); \
	barrier(); \
} ಆಗ (0)
#ವ್ಯಾಖ್ಯಾನಿಸು INIT_G() ಮಾಡು { \
	ಗಣ_PTR_ಗಮ್ಯ_GLOBALS(xzalloc(ಗಾತ್ರ(G))); \
	G.promptmode = 1; \
} ಆಗ (0)
#ಇಲ್ಲದಿದ್ದರೆ /* !__U_BOOT__ */
#ವ್ಯಾಖ್ಯಾನಿಸು INIT_G() ಮಾಡು { \
	ಗಣ_PTR_ಗಮ್ಯ_GLOBALS(xzalloc(ಗಾತ್ರ(G))); \
	/* memset(&G.sa, 0, sizeof(G.sa)); */  \
	sigfillset(&G.sa.sa_ಮುಖವಾಡ); \
	G.sa.sa_flags = SA_RESTART; \
} ಆಗ (0)
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
/* Function prototypes for builtins */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ builtin_cd(ಅಕ್ಷರ **argv) FAST_FUNC;
#ಆದರೆ ENABLE_HUSH_ECHO
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ builtin_echo(ಅಕ್ಷರ **argv) FAST_FUNC;
#ಆದರೆ_ಅಂತ್ಯ
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ builtin_eval(ಅಕ್ಷರ **argv) FAST_FUNC;
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ builtin_exec(ಅಕ್ಷರ **argv) FAST_FUNC;
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ builtin_exit(ಅಕ್ಷರ **argv) FAST_FUNC;
#ಆದರೆ ENABLE_HUSH_EXPORT
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ builtin_export(ಅಕ್ಷರ **argv) FAST_FUNC;
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_HUSH_READONLY
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ builtin_readonly(ಅಕ್ಷರ **argv) FAST_FUNC;
#ಆದರೆ_ಅಂತ್ಯ
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ builtin_false(ಅಕ್ಷರ **argv) FAST_FUNC;
#ಆದರೆ ENABLE_HUSH_ಕೆಲಸ
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ builtin_fg_bg(ಅಕ್ಷರ **argv) FAST_FUNC;
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ builtin_jobs(ಅಕ್ಷರ **argv) FAST_FUNC;
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_HUSH_GETOPTS
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ builtin_getopts(ಅಕ್ಷರ **argv) FAST_FUNC;
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_HUSH_HELP
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ builtin_help(ಅಕ್ಷರ **argv) FAST_FUNC;
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ MAX_HISTORY && ENABLE_FEATURE_EDITING
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ builtin_history(ಅಕ್ಷರ **argv) FAST_FUNC;
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_HUSH_LOCAL
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ builtin_local(ಅಕ್ಷರ **argv) FAST_FUNC;
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_HUSH_MEMLEAK
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ builtin_memleak(ಅಕ್ಷರ **argv) FAST_FUNC;
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_HUSH_PRINTF
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ builtin_printf(ಅಕ್ಷರ **argv) FAST_FUNC;
#ಆದರೆ_ಅಂತ್ಯ
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ builtin_pwd(ಅಕ್ಷರ **argv) FAST_FUNC;
#ಆದರೆ ENABLE_HUSH_READ
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ builtin_read(ಅಕ್ಷರ **argv) FAST_FUNC;
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_HUSH_ಗಣ
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ builtin_ಗಣ(ಅಕ್ಷರ **argv) FAST_FUNC;
#ಆದರೆ_ಅಂತ್ಯ
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ builtin_ಸರಿಸು(ಅಕ್ಷರ **argv) FAST_FUNC;
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ builtin_ಮೂಲ(ಅಕ್ಷರ **argv) FAST_FUNC;
#ಆದರೆ ENABLE_HUSH_TEST || BASH_TEST2
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ builtin_test(ಅಕ್ಷರ **argv) FAST_FUNC;
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_HUSH_TRAP
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ builtin_trap(ಅಕ್ಷರ **argv) FAST_FUNC;
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_HUSH_TYPE
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ builtin_type(ಅಕ್ಷರ **argv) FAST_FUNC;
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_HUSH_TIMES
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ builtin_times(ಅಕ್ಷರ **argv) FAST_FUNC;
#ಆದರೆ_ಅಂತ್ಯ
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ builtin_true(ಅಕ್ಷರ **argv) FAST_FUNC;
#ಆದರೆ ENABLE_HUSH_UMASK
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ builtin_umask(ಅಕ್ಷರ **argv) FAST_FUNC;
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_HUSH_UNSET
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ builtin_unset(ಅಕ್ಷರ **argv) FAST_FUNC;
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_HUSH_KILL
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ builtin_kill(ಅಕ್ಷರ **argv) FAST_FUNC;
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_HUSH_WAIT
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ builtin_wait(ಅಕ್ಷರ **argv) FAST_FUNC;
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_HUSH_LOOPS
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ builtin_break(ಅಕ್ಷರ **argv) FAST_FUNC;
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ builtin_continue(ಅಕ್ಷರ **argv) FAST_FUNC;
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_HUSH_FUNCTIONS
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ builtin_return(ಅಕ್ಷರ **argv) FAST_FUNC;
#ಆದರೆ_ಅಂತ್ಯ

/* Table of built-in functions.  They can be forked or not, depending on
 * context: within pipes, they fork.  As simple commands, they do not.
 * When used in non-forking context, they can change global variables
 * in the parent shell process.  If forked, of course they cannot.
 * For example, 'unset foo | whatever' will parse and run, but foo will
 * still be set at the end. */
ರಚನೆ built_ಒಳ_command {
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *b_cmd;
	ಪೂರ್ಣಾಂಕ (*b_function)(ಅಕ್ಷರ **argv) FAST_FUNC;
#ಆದರೆ ENABLE_HUSH_HELP
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *b_descr;
# define BLTIN(cmd, func, help) { cmd, func, help }
#ಇಲ್ಲದಿದ್ದರೆ
# define BLTIN(cmd, func, help) { cmd, func }
#ಆದರೆ_ಅಂತ್ಯ
};

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ built_ಒಳ_command bltins1[] ALIGN_PTR = {
	BLTIN("."        , builtin_ಮೂಲ  , "Run commands in file"),
	BLTIN(":"        , builtin_true    , ಖಾಲಿ),
#ಆದರೆ ENABLE_HUSH_ಕೆಲಸ
	BLTIN("bg"       , builtin_fg_bg   , "Resume job in background"),
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_HUSH_LOOPS
	BLTIN("break"    , builtin_break   , "Exit loop"),
#ಆದರೆ_ಅಂತ್ಯ
	BLTIN("cd"       , builtin_cd      , "Change directory"),
#ಆದರೆ ENABLE_HUSH_LOOPS
	BLTIN("continue" , builtin_continue, "Start new loop iteration"),
#ಆದರೆ_ಅಂತ್ಯ
	BLTIN("eval"     , builtin_eval    , "Construct and run shell command"),
	BLTIN("exec"     , builtin_exec    , "Execute command, don't return to shell"),
	BLTIN("exit"     , builtin_exit    , ಖಾಲಿ),
#ಆದರೆ ENABLE_HUSH_EXPORT
	BLTIN("export"   , builtin_export  , "Set environment variables"),
#ಆದರೆ_ಅಂತ್ಯ
	BLTIN("false"    , builtin_false   , ಖಾಲಿ),
#ಆದರೆ ENABLE_HUSH_ಕೆಲಸ
	BLTIN("fg"       , builtin_fg_bg   , "Bring job to foreground"),
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_HUSH_GETOPTS
	BLTIN("getopts"  , builtin_getopts , ಖಾಲಿ),
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_HUSH_HELP
	BLTIN("help"     , builtin_help    , ಖಾಲಿ),
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ MAX_HISTORY && ENABLE_FEATURE_EDITING
	BLTIN("history"  , builtin_history , "Show history"),
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_HUSH_ಕೆಲಸ
	BLTIN("jobs"     , builtin_jobs    , "List jobs"),
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_HUSH_KILL
	BLTIN("kill"     , builtin_kill    , "Send signals to processes"),
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_HUSH_LOCAL
	BLTIN("local"    , builtin_local   , "Set local variables"),
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_HUSH_MEMLEAK
	BLTIN("memleak"  , builtin_memleak , ಖಾಲಿ),
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_HUSH_READ
	BLTIN("read"     , builtin_read    , "Input into variable"),
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_HUSH_READONLY
	BLTIN("readonly" , builtin_readonly, "Make variables read-only"),
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_HUSH_FUNCTIONS
	BLTIN("return"   , builtin_return  , "Return from function"),
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_HUSH_ಗಣ
	BLTIN("set"      , builtin_ಗಣ     , "Set positional parameters"),
#ಆದರೆ_ಅಂತ್ಯ
	BLTIN("shift"    , builtin_ಸರಿಸು   , "Shift positional parameters"),
#ಆದರೆ BASH_ಮೂಲ
	BLTIN("source"   , builtin_ಮೂಲ  , ಖಾಲಿ),
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_HUSH_TIMES
	BLTIN("times"    , builtin_times   , ಖಾಲಿ),
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_HUSH_TRAP
	BLTIN("trap"     , builtin_trap    , "Trap signals"),
#ಆದರೆ_ಅಂತ್ಯ
	BLTIN("true"     , builtin_true    , ಖಾಲಿ),
#ಆದರೆ ENABLE_HUSH_TYPE
	BLTIN("type"     , builtin_type    , "Show command type"),
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_HUSH_ULIMIT
	BLTIN("ulimit"   , shell_builtin_ulimit, "Control resource limits"),
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_HUSH_UMASK
	BLTIN("umask"    , builtin_umask   , "Set file creation mask"),
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_HUSH_UNSET
	BLTIN("unset"    , builtin_unset   , "Unset variables"),
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_HUSH_WAIT
	BLTIN("wait"     , builtin_wait    , "Wait for process to finish"),
#ಆದರೆ_ಅಂತ್ಯ
};
/* These builtins won't be used if we are on NOMMU and need to re-exec
 * (it's cheaper to run an external program in this case):
 */
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ built_ಒಳ_command bltins2[] ALIGN_PTR = {
#ಆದರೆ ENABLE_HUSH_TEST
	BLTIN("["        , builtin_test    , ಖಾಲಿ),
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ BASH_TEST2
	BLTIN("[["       , builtin_test    , ಖಾಲಿ),
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_HUSH_ECHO
	BLTIN("echo"     , builtin_echo    , ಖಾಲಿ),
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_HUSH_PRINTF
	BLTIN("printf"   , builtin_printf  , ಖಾಲಿ),
#ಆದರೆ_ಅಂತ್ಯ
	BLTIN("pwd"      , builtin_pwd     , ಖಾಲಿ),
#ಆದರೆ ENABLE_HUSH_TEST
	BLTIN("test"     , builtin_test    , ಖಾಲಿ),
#ಆದರೆ_ಅಂತ್ಯ
};

#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */

/* Debug printouts.
 */
#ಆದರೆ HUSH_DEBUG >= 2
/* prevent disasters with G.debug_indent < 0 */
# define indent() fdprintf(2, "%*s", (G.debug_indent * 2) & 0xff, "")
# define debug_enter() (G.debug_indent++)
# define debug_leave() (G.debug_indent--)
#ಇಲ್ಲದಿದ್ದರೆ
# define indent()      ((ಶೂನ್ಯ)0)
# define debug_enter() ((ಶೂನ್ಯ)0)
# define debug_leave() ((ಶೂನ್ಯ)0)
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ debug_printf
# define debug_printf(...) (indent(), fdprintf(2, __VA_ARGS__))
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ debug_printf_parse
# define debug_printf_parse(...) (indent(), fdprintf(2, __VA_ARGS__))
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ debug_printf_heredoc
# define debug_printf_heredoc(...) (indent(), fdprintf(2, __VA_ARGS__))
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ debug_printf_exec
#ವ್ಯಾಖ್ಯಾನಿಸು debug_printf_exec(...) (indent(), fdprintf(2, __VA_ARGS__))
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ debug_printf_env
# define debug_printf_env(...) (indent(), fdprintf(2, __VA_ARGS__))
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ debug_printf_jobs
# define debug_printf_jobs(...) (indent(), fdprintf(2, __VA_ARGS__))
# define DEBUG_JOBS 1
#ಇಲ್ಲದಿದ್ದರೆ
# define DEBUG_JOBS 0
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ debug_printf_expand
# define debug_printf_expand(...) (indent(), fdprintf(2, __VA_ARGS__))
# define DEBUG_EXPAND 1
#ಇಲ್ಲದಿದ್ದರೆ
# define DEBUG_EXPAND 0
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ debug_printf_varexp
# define debug_printf_varexp(...) (indent(), fdprintf(2, __VA_ARGS__))
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ debug_printf_glob
# define debug_printf_glob(...) (indent(), fdprintf(2, __VA_ARGS__))
# define DEBUG_GLOB 1
#ಇಲ್ಲದಿದ್ದರೆ
# define DEBUG_GLOB 0
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ debug_printf_redir
# define debug_printf_redir(...) (indent(), fdprintf(2, __VA_ARGS__))
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ debug_printf_list
# define debug_printf_list(...) (indent(), fdprintf(2, __VA_ARGS__))
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ debug_printf_subst
# define debug_printf_subst(...) (indent(), fdprintf(2, __VA_ARGS__))
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ debug_printf_prompt
# define debug_printf_prompt(...) (indent(), fdprintf(2, __VA_ARGS__))
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ debug_printf_clean
# define debug_printf_clean(...) (indent(), fdprintf(2, __VA_ARGS__))
# define DEBUG_CLEAN 1
#ಇಲ್ಲದಿದ್ದರೆ
# define DEBUG_CLEAN 0
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ DEBUG_EXPAND
ಸ್ಥಿರ ಶೂನ್ಯ debug_print_ಅಕ್ಷರಸರಣಿಗಳು(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಪೂರ್ವಪ್ರತ್ಯಯ, ಅಕ್ಷರ **vv)
{
	indent();
	fdprintf(2, "%s:\n", ಪೂರ್ವಪ್ರತ್ಯಯ);
	ಆಗ (*vv)
		fdprintf(2, " '%s'\n", *vv++);
}
#ಇಲ್ಲದಿದ್ದರೆ
# define debug_print_ಅಕ್ಷರಸರಣಿಗಳು(ಪೂರ್ವಪ್ರತ್ಯಯ, vv) ((ಶೂನ್ಯ)0)
#ಆದರೆ_ಅಂತ್ಯ

/* Leak hunting. Use hush_leaktool.sh for post-processing.
 */
#ಆದರೆ LEAK_HUNTING
ಸ್ಥಿರ ಶೂನ್ಯ *xxmalloc(ಪೂರ್ಣಾಂಕ lineno, size_t ಗಾತ್ರ_2)
{
	ಶೂನ್ಯ *ptr = xmalloc((ಗಾತ್ರ_2 + 0xff) & ~0xff);
	fdprintf(2, "line %d: malloc %p\n", lineno, ptr);
	ಹಿಂತಿರುಗಿ ptr;
}
ಸ್ಥಿರ ಶೂನ್ಯ *xxrealloc(ಪೂರ್ಣಾಂಕ lineno, ಶೂನ್ಯ *ptr, size_t ಗಾತ್ರ_2)
{
	ptr = xrealloc(ptr, (ಗಾತ್ರ_2 + 0xff) & ~0xff);
	fdprintf(2, "line %d: realloc %p\n", lineno, ptr);
	ಹಿಂತಿರುಗಿ ptr;
}
ಸ್ಥಿರ ಅಕ್ಷರ *xxstrdup(ಪೂರ್ಣಾಂಕ lineno, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str)
{
	ಅಕ್ಷರ *ptr = xstrdup(str);
	fdprintf(2, "line %d: strdup %p\n", lineno, ptr);
	ಹಿಂತಿರುಗಿ ptr;
}
ಸ್ಥಿರ ಶೂನ್ಯ xxfree(ಶೂನ್ಯ *ptr)
{
	fdprintf(2, "free %p\n", ptr);
	ಬಿಡುಗಡೆ(ptr);
}
# define xmalloc(s)     xxmalloc(__LINE__, s)
# define xrealloc(p, s) xxrealloc(__LINE__, p, s)
# define xstrdup(s)     xxstrdup(__LINE__, s)
# define ಬಿಡುಗಡೆ(p)        xxfree(p)
#ಆದರೆ_ಅಂತ್ಯ

/* Syntax and runtime errors. They always abort scripts.
 * In interactive use they usually discard unparsed and/or unexecuted commands
 * and return to the prompt.
 * HUSH_DEBUG >= 2 prints line number in this file where it was detected.
 */
#ಆದರೆ HUSH_DEBUG < 2
# define ಸಂದೇಶ_and_die_if_script(lineno, ...)     ಸಂದೇಶ_and_die_if_script(__VA_ARGS__)
# define syntax_error(lineno, ಸಂದೇಶ)              syntax_error(ಸಂದೇಶ)
# define syntax_error_at(lineno, ಸಂದೇಶ)           syntax_error_at(ಸಂದೇಶ)
# define syntax_error_unterm_ch(lineno, ch)     syntax_error_unterm_ch(ch)
# define syntax_error_unterm_str(lineno, s)     syntax_error_unterm_str(s)
# define syntax_error_unexpected_ch(lineno, ch) syntax_error_unexpected_ch(ch)
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ಶೂನ್ಯ die_if_script(ಶೂನ್ಯ)
{
	ಆದರೆ (!G_interactive_fd) {
		ಆದರೆ (G.ಕೊನೆಯ_exitcode) /* sometines it's 2, not 1 (bash compat) */
			xfunc_error_retval = G.ಕೊನೆಯ_exitcode;
		xfunc_die();
	}
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __U_BOOT__
ಸ್ಥಿರ ಶೂನ್ಯ __maybe_unused ಸಂದೇಶ_and_die_if_script(ಚಿಹ್ನೆರಹಿತ lineno, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt, ...)
#ಇಲ್ಲದಿದ್ದರೆ /* !__U_BOOT__ */
ಸ್ಥಿರ ಶೂನ್ಯ ಸಂದೇಶ_and_die_if_script(ಚಿಹ್ನೆರಹಿತ lineno, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt, ...)
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */
{
	va_list p;

#ಆದರೆ HUSH_DEBUG >= 2
	bb_error_ಸಂದೇಶ("hush.c:%u", lineno);
#ಆದರೆ_ಅಂತ್ಯ
	va_ಆರಂಭ(p, fmt);
	bb_verror_ಸಂದೇಶ(fmt, p, ಖಾಲಿ);
	va_ಕೊನೆ(p);
	die_if_script();
}

ಸ್ಥಿರ ಶೂನ್ಯ syntax_error(ಚಿಹ್ನೆರಹಿತ lineno ಬಳಸದ_PARAM, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ)
{
	ಆದರೆ (ಸಂದೇಶ)
		bb_error_ಸಂದೇಶ("syntax error: %s", ಸಂದೇಶ);
	ಇಲ್ಲದಿದ್ದರೆ
		bb_simple_error_ಸಂದೇಶ("syntax error");
	die_if_script();
}

ಸ್ಥಿರ ಶೂನ್ಯ syntax_error_at(ಚಿಹ್ನೆರಹಿತ lineno ಬಳಸದ_PARAM, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಂದೇಶ)
{
	bb_error_ಸಂದೇಶ("syntax error at '%s'", ಸಂದೇಶ);
	die_if_script();
}

ಸ್ಥಿರ ಶೂನ್ಯ syntax_error_unterm_str(ಚಿಹ್ನೆರಹಿತ lineno ಬಳಸದ_PARAM, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s)
{
	bb_error_ಸಂದೇಶ("syntax error: unterminated %s", s);
//? source4.tests fails: in bash, echo ${^} in script does not terminate the script
// (but bash --posix, or if bash is run as "sh", does terminate in script, so maybe uncomment this?)
//	die_if_script();
}

ಸ್ಥಿರ ಶೂನ್ಯ syntax_error_unterm_ch(ಚಿಹ್ನೆರಹಿತ lineno, ಅಕ್ಷರ ch)
{
	ಅಕ್ಷರ ಸಂದೇಶ[2] = { ch, '\0' };
	syntax_error_unterm_str(lineno, ಸಂದೇಶ);
}

ಸ್ಥಿರ ಶೂನ್ಯ syntax_error_unexpected_ch(ಚಿಹ್ನೆರಹಿತ lineno ಬಳಸದ_PARAM, ಪೂರ್ಣಾಂಕ ch)
{
	ಅಕ್ಷರ ಸಂದೇಶ[2];
	ಸಂದೇಶ[0] = ch;
	ಸಂದೇಶ[1] = '\0';
#ಆದರೆ HUSH_DEBUG >= 2
	bb_error_ಸಂದೇಶ("hush.c:%u", lineno);
#ಆದರೆ_ಅಂತ್ಯ
	bb_error_ಸಂದೇಶ("syntax error: unexpected %s", ch == EOF ? "EOF" : ಸಂದೇಶ);
	die_if_script();
}

#ಆದರೆ HUSH_DEBUG < 2
# undef ಸಂದೇಶ_and_die_if_script
# undef syntax_error
# undef syntax_error_at
# undef syntax_error_unterm_ch
# undef syntax_error_unterm_str
# undef syntax_error_unexpected_ch
#ಇಲ್ಲದಿದ್ದರೆ
# define ಸಂದೇಶ_and_die_if_script(...)     ಸಂದೇಶ_and_die_if_script(__LINE__, __VA_ARGS__)
# define syntax_error(ಸಂದೇಶ)              syntax_error(__LINE__, ಸಂದೇಶ)
# define syntax_error_at(ಸಂದೇಶ)           syntax_error_at(__LINE__, ಸಂದೇಶ)
# define syntax_error_unterm_ch(ch)     syntax_error_unterm_ch(__LINE__, ch)
# define syntax_error_unterm_str(s)     syntax_error_unterm_str(__LINE__, s)
# define syntax_error_unexpected_ch(ch) syntax_error_unexpected_ch(__LINE__, ch)
#ಆದರೆ_ಅಂತ್ಯ

/* Utility functions
 */
/* Replace each \x with x in place, return ptr past NUL. */
ಸ್ಥಿರ ಅಕ್ಷರ *unbackslash(ಅಕ್ಷರ *src)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __U_BOOT__
	ಅಕ್ಷರ *dst = src = (ಅಕ್ಷರ *)strchrnul(src, '\\');
#ಇಲ್ಲದಿದ್ದರೆ /* !__U_BOOT__ */
	ಅಕ್ಷರ *dst = src = strchrnul(src, '\\');
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */
	ಆಗ (1) {
		ಆದರೆ (*src == '\\') {
			src++;
			ಆದರೆ (*src != '\0') {
				/* \x -> x */
				*dst++ = *src++;
				ಮುಂದುವರಿ;
			}
			/* else: "\<nul>". Do not delete this backslash.
			 * Testcase: eval 'echo ok\'
			 */
			*dst++ = '\\';
			/* fallthrough */
		}
		ಆದರೆ ((*dst++ = *src++) == '\0')
			ನಿಲ್ಲಿಸು;
	}
	ಹಿಂತಿರುಗಿ dst;
}

ಸ್ಥಿರ ಅಕ್ಷರ **ಕೂಡಿಸು_ಅಕ್ಷರಸರಣಿಗಳು_ಗಮ್ಯ_ಅಕ್ಷರಸರಣಿಗಳು(ಅಕ್ಷರ **ಅಕ್ಷರಸರಣಿಗಳು, ಅಕ್ಷರ **ಕೂಡಿಸು, ಪೂರ್ಣಾಂಕ need_ಗಮ್ಯ_dup)
{
	ಪೂರ್ಣಾಂಕ i;
	ಚಿಹ್ನೆರಹಿತ count1;
	ಚಿಹ್ನೆರಹಿತ count2;
	ಅಕ್ಷರ **v;

	v = ಅಕ್ಷರಸರಣಿಗಳು;
	count1 = 0;
	ಆದರೆ (v) {
		ಆಗ (*v) {
			count1++;
			v++;
		}
	}
	count2 = 0;
	v = ಕೂಡಿಸು;
	ಆಗ (*v) {
		count2++;
		v++;
	}
	v = xrealloc(ಅಕ್ಷರಸರಣಿಗಳು, (count1 + count2 + 1) * ಗಾತ್ರ(ಅಕ್ಷರ*));
	v[count1 + count2] = ಖಾಲಿ;
	i = count2;
	ಆಗ (--i >= 0)
		v[count1 + i] = (need_ಗಮ್ಯ_dup ? xstrdup(ಕೂಡಿಸು[i]) : ಕೂಡಿಸು[i]);
	ಹಿಂತಿರುಗಿ v;
}
#ಆದರೆ LEAK_HUNTING
ಸ್ಥಿರ ಅಕ್ಷರ **xx_ಕೂಡಿಸು_ಅಕ್ಷರಸರಣಿಗಳು_ಗಮ್ಯ_ಅಕ್ಷರಸರಣಿಗಳು(ಪೂರ್ಣಾಂಕ lineno, ಅಕ್ಷರ **ಅಕ್ಷರಸರಣಿಗಳು, ಅಕ್ಷರ **ಕೂಡಿಸು, ಪೂರ್ಣಾಂಕ need_ಗಮ್ಯ_dup)
{
	ಅಕ್ಷರ **ptr = ಕೂಡಿಸು_ಅಕ್ಷರಸರಣಿಗಳು_ಗಮ್ಯ_ಅಕ್ಷರಸರಣಿಗಳು(ಅಕ್ಷರಸರಣಿಗಳು, ಕೂಡಿಸು, need_ಗಮ್ಯ_dup);
	fdprintf(2, "line %d: add_strings_to_strings %p\n", lineno, ptr);
	ಹಿಂತಿರುಗಿ ptr;
}
#ವ್ಯಾಖ್ಯಾನಿಸು ಕೂಡಿಸು_ಅಕ್ಷರಸರಣಿಗಳು_ಗಮ್ಯ_ಅಕ್ಷರಸರಣಿಗಳು(ಅಕ್ಷರಸರಣಿಗಳು, ಕೂಡಿಸು, need_ಗಮ್ಯ_dup) \
	xx_ಕೂಡಿಸು_ಅಕ್ಷರಸರಣಿಗಳು_ಗಮ್ಯ_ಅಕ್ಷರಸರಣಿಗಳು(__LINE__, ಅಕ್ಷರಸರಣಿಗಳು, ಕೂಡಿಸು, need_ಗಮ್ಯ_dup)
#ಆದರೆ_ಅಂತ್ಯ

/* Note: takes ownership of "add" ptr (it is not strdup'ed) */
ಸ್ಥಿರ ಅಕ್ಷರ **ಕೂಡಿಸು_string_ಗಮ್ಯ_ಅಕ್ಷರಸರಣಿಗಳು(ಅಕ್ಷರ **ಅಕ್ಷರಸರಣಿಗಳು, ಅಕ್ಷರ *ಕೂಡಿಸು)
{
	ಅಕ್ಷರ *v[2];
	v[0] = ಕೂಡಿಸು;
	v[1] = ಖಾಲಿ;
	ಹಿಂತಿರುಗಿ ಕೂಡಿಸು_ಅಕ್ಷರಸರಣಿಗಳು_ಗಮ್ಯ_ಅಕ್ಷರಸರಣಿಗಳು(ಅಕ್ಷರಸರಣಿಗಳು, v, /*dup:*/ 0);
}

#ಆದರೆ LEAK_HUNTING
ಸ್ಥಿರ ಅಕ್ಷರ **xx_ಕೂಡಿಸು_string_ಗಮ್ಯ_ಅಕ್ಷರಸರಣಿಗಳು(ಪೂರ್ಣಾಂಕ lineno, ಅಕ್ಷರ **ಅಕ್ಷರಸರಣಿಗಳು, ಅಕ್ಷರ *ಕೂಡಿಸು)
{
	ಅಕ್ಷರ **ptr = ಕೂಡಿಸು_string_ಗಮ್ಯ_ಅಕ್ಷರಸರಣಿಗಳು(ಅಕ್ಷರಸರಣಿಗಳು, ಕೂಡಿಸು);
	fdprintf(2, "line %d: add_string_to_strings %p\n", lineno, ptr);
	ಹಿಂತಿರುಗಿ ptr;
}
#ವ್ಯಾಖ್ಯಾನಿಸು ಕೂಡಿಸು_string_ಗಮ್ಯ_ಅಕ್ಷರಸರಣಿಗಳು(ಅಕ್ಷರಸರಣಿಗಳು, ಕೂಡಿಸು) \
	xx_ಕೂಡಿಸು_string_ಗಮ್ಯ_ಅಕ್ಷರಸರಣಿಗಳು(__LINE__, ಅಕ್ಷರಸರಣಿಗಳು, ಕೂಡಿಸು)
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ಶೂನ್ಯ free_ಅಕ್ಷರಸರಣಿಗಳು(ಅಕ್ಷರ **ಅಕ್ಷರಸರಣಿಗಳು)
{
	ಅಕ್ಷರ **v;

	ಆದರೆ (!ಅಕ್ಷರಸರಣಿಗಳು)
		ಹಿಂತಿರುಗಿ;
	v = ಅಕ್ಷರಸರಣಿಗಳು;
	ಆಗ (*v) {
		ಬಿಡುಗಡೆ(*v);
		v++;
	}
	ಬಿಡುಗಡೆ(ಅಕ್ಷರಸರಣಿಗಳು);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ dup_CLOEXEC(ಪೂರ್ಣಾಂಕ fd, ಪೂರ್ಣಾಂಕ avoid_fd)
{
	ಪೂರ್ಣಾಂಕ newfd;
 repeat:
	newfd = fcntl(fd, F_DUPFD_CLOEXEC, avoid_fd + 1);
	ಆದರೆ (newfd >= 0) {
		ಆದರೆ (F_DUPFD_CLOEXEC == F_DUPFD) /* if old libc (w/o F_DUPFD_CLOEXEC) */
			close_on_exec_on(newfd);
	} ಇಲ್ಲದಿದ್ದರೆ { /* newfd < 0 */
		ಆದರೆ (errno == EBUSY)
			ಹೋಗು repeat;
		ಆದರೆ (errno == EINTR)
			ಹೋಗು repeat;
		ಆದರೆ (errno != EBADF) {
			/* "echo >&9999" gets EINVAL trying to save fd 1 to above 9999.
			 * We could try saving it _below_ 9999 instead (how?), but
			 * this probably means that dup2(9999,1) to effectuate >&9999
			 * would also not work: fd 9999 can't exist.
			 * (This differs from "echo >&99" where saving works, but
			 * subsequent dup2(99,1) fails if fd 99 is not open).
			 */
			bb_perror_ಸಂದೇಶ("fcntl(%d,F_DUPFD,%d)", fd, avoid_fd + 1);
		}
	}
	ಹಿಂತಿರುಗಿ newfd;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ xdup_CLOEXEC_and_close(ಪೂರ್ಣಾಂಕ fd, ಪೂರ್ಣಾಂಕ avoid_fd)
{
	ಪೂರ್ಣಾಂಕ newfd;
 repeat:
	newfd = fcntl(fd, F_DUPFD_CLOEXEC, avoid_fd + 1);
	ಆದರೆ (newfd < 0) {
		ಆದರೆ (errno == EBUSY)
			ಹೋಗು repeat;
		ಆದರೆ (errno == EINTR)
			ಹೋಗು repeat;
		/* fd was not open? */
		ಆದರೆ (errno == EBADF)
			ಹಿಂತಿರುಗಿ fd;
		xfunc_die();
	}
	ಆದರೆ (F_DUPFD_CLOEXEC == F_DUPFD) /* if old libc (w/o F_DUPFD_CLOEXEC) */
		close_on_exec_on(newfd);
	close(fd);
	ಹಿಂತಿರುಗಿ newfd;
}

/* Manipulating HFILEs */
ಸ್ಥಿರ HFILE *hfopen(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name)
{
	HFILE *fp;
	ಪೂರ್ಣಾಂಕ fd;

	fd = STDIN_FILENO;
	ಆದರೆ (name) {
		fd = open(name, O_RDONLY | O_CLOEXEC);
		ಆದರೆ (fd < 0)
			ಹಿಂತಿರುಗಿ ಖಾಲಿ;
		ಆದರೆ (O_CLOEXEC == 0) /* ancient libc */
			close_on_exec_on(fd);
	}

	fp = xmalloc(ಗಾತ್ರ(*fp));
	ಆದರೆ (name == ಖಾಲಿ)
		G.HFILE_stdin = fp;
	fp->fd = fd;
	fp->cur = fp->ಕೊನೆ_2 = fp->buf;
	fp->ಮುಂದಿನ_hfile = G.HFILE_list;
	G.HFILE_list = fp;
	ಹಿಂತಿರುಗಿ fp;
}
ಸ್ಥಿರ ಶೂನ್ಯ hfclose(HFILE *fp)
{
	HFILE **pp = &G.HFILE_list;
	ಆಗ (*pp) {
		HFILE *cur = *pp;
		ಆದರೆ (cur == fp) {
			*pp = cur->ಮುಂದಿನ_hfile;
			ನಿಲ್ಲಿಸು;
		}
		pp = &cur->ಮುಂದಿನ_hfile;
	}
	ಆದರೆ (fp->fd >= 0)
		close(fp->fd);
	ಬಿಡುಗಡೆ(fp);
}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ refill_HFILE_and_getc(HFILE *fp)
{
	ಪೂರ್ಣಾಂಕ n;

	ಆದರೆ (fp->fd < 0) {
		/* Already saw EOF */
		ಹಿಂತಿರುಗಿ EOF;
	}
#ಆದರೆ ENABLE_HUSH_INTERACTIVE && !ENABLE_FEATURE_EDITING
	/* If user presses ^C, read() restarts after SIGINT (we use SA_RESTART).
	 * IOW: ^C will not immediately stop line input.
	 * But poll() is different: it does NOT restart after signals.
	 */
	ಆದರೆ (fp == G.HFILE_stdin) {
		ರಚನೆ pollfd pfd[1];
		pfd[0].fd = fp->fd;
		pfd[0].events = POLLIN;
		n = poll(pfd, 1, -1);
		ಆದರೆ (n < 0
		 /*&& errno == EINTR - assumed true */
		 && sigismember(&G.pending_ಗಣ, SIGINT)
		) {
			ಹಿಂತಿರುಗಿ '\0';
		}
	}
#ಇಲ್ಲದಿದ್ದರೆ
/* if FEATURE_EDITING=y, we do not use this routine for interactive input */
#ಆದರೆ_ಅಂತ್ಯ
	/* Try to buffer more input */
	n = ಸುರಕ್ಷಿತ_read(fp->fd, fp->buf, ಗಾತ್ರ(fp->buf));
	ಆದರೆ (n < 0) {
		bb_simple_perror_ಸಂದೇಶ("read error");
		n = 0;
	}
	fp->cur = fp->buf;
	fp->ಕೊನೆ_2 = fp->buf + n;
	ಆದರೆ (n == 0) {
		/* EOF/error */
		close(fp->fd);
		fp->fd = -1;
		ಹಿಂತಿರುಗಿ EOF;
	}
	ಹಿಂತಿರುಗಿ (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)(*fp->cur++);
}
/* Inlined for common case of non-empty buffer.
 */
ಸ್ಥಿರ ALWAYS_INLINE ಪೂರ್ಣಾಂಕ hfgetc(HFILE *fp)
{
	ಆದರೆ (fp->cur < fp->ಕೊನೆ_2)
		ಹಿಂತಿರುಗಿ (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)(*fp->cur++);
	/* Buffer empty */
	ಹಿಂತಿರುಗಿ refill_HFILE_and_getc(fp);
}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ move_HFILEs_on_redirect(ಪೂರ್ಣಾಂಕ fd, ಪೂರ್ಣಾಂಕ avoid_fd)
{
	HFILE *fl = G.HFILE_list;
	ಆಗ (fl) {
		ಆದರೆ (fd == fl->fd) {
			/* We use it only on script files, they are all CLOEXEC */
			fl->fd = xdup_CLOEXEC_and_close(fd, avoid_fd);
			debug_printf_redir("redirect_fd %d: matches a script fd, moving it to %d\n", fd, fl->fd);
			ಹಿಂತಿರುಗಿ 1; /* "found and moved" */
		}
		fl = fl->ಮುಂದಿನ_hfile;
	}
#ಆದರೆ ENABLE_HUSH_ಬಹುಲಕ_X
	ಆದರೆ (G.x_ಬಹುಲಕ_fd > 0 && fd == G.x_ಬಹುಲಕ_fd) {
		G.x_ಬಹುಲಕ_fd = xdup_CLOEXEC_and_close(fd, avoid_fd);
		ಹಿಂತಿರುಗಿ 1; /* "found and moved" */
	}
#ಆದರೆ_ಅಂತ್ಯ
	ಹಿಂತಿರುಗಿ 0; /* "not in the list" */
}
#ಆದರೆ ENABLE_FEATURE_SH_STANDALONE && BB_MMU
ಸ್ಥಿರ ಶೂನ್ಯ close_all_HFILE_list(ಶೂನ್ಯ)
{
	HFILE *fl = G.HFILE_list;
	ಆಗ (fl) {
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
		ಆದರೆ (fl->fd > 0)
			/*hfclose(fl); - unsafe */
			close(fl->fd);
		fl = fl->ಮುಂದಿನ_hfile;
	}
}
#ಆದರೆ_ಅಂತ್ಯ
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fd_ಒಳ_HFILEs(ಪೂರ್ಣಾಂಕ fd)
{
	HFILE *fl = G.HFILE_list;
	ಆಗ (fl) {
		ಆದರೆ (fl->fd == fd)
			ಹಿಂತಿರುಗಿ 1;
		fl = fl->ಮುಂದಿನ_hfile;
	}
	ಹಿಂತಿರುಗಿ 0;
}

#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */

/* Helpers for setting new $n and restoring them back
 */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ save_arg_t {
	ಅಕ್ಷರ *sv_argv0;
	ಅಕ್ಷರ **sv_g_argv;
	ಪೂರ್ಣಾಂಕ sv_g_argc;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	IF_HUSH_ಗಣ(smallint sv_g_malloced;)
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */
} save_arg_t;

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
ಸ್ಥಿರ ಶೂನ್ಯ save_and_replace_G_ವಾದಗಳು(save_arg_t *sv, ಅಕ್ಷರ **argv)
{
	sv->sv_argv0 = argv[0];
	sv->sv_g_argv = G.global_argv;
	sv->sv_g_argc = G.global_argc;
	IF_HUSH_ಗಣ(sv->sv_g_malloced = G.global_ವಾದಗಳು_malloced;)

	argv[0] = G.global_argv[0]; /* retain $0 */
	G.global_argv = argv;
	IF_HUSH_ಗಣ(G.global_ವಾದಗಳು_malloced = 0;)

	G.global_argc = 1 + string_array_len(argv + 1);
}

ಸ್ಥಿರ ಶೂನ್ಯ restore_G_ವಾದಗಳು(save_arg_t *sv, ಅಕ್ಷರ **argv)
{
#ಆದರೆ ENABLE_HUSH_ಗಣ
	ಆದರೆ (G.global_ವಾದಗಳು_malloced) {
		/* someone ran "set -- arg1 arg2 ...", undo */
		ಅಕ್ಷರ **pp = G.global_argv;
		ಆಗ (*++pp) /* note: does not free $0 */
			ಬಿಡುಗಡೆ(*pp);
		ಬಿಡುಗಡೆ(G.global_argv);
	}
#ಆದರೆ_ಅಂತ್ಯ
	argv[0] = sv->sv_argv0;
	G.global_argv = sv->sv_g_argv;
	G.global_argc = sv->sv_g_argc;
	IF_HUSH_ಗಣ(G.global_ವಾದಗಳು_malloced = sv->sv_g_malloced;)
}
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
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
ಎಣಿಕೆ {
	SPECIAL_INTERACTIVE_SIGS = 0
		| (1 << SIGTERM)
		| (1 << SIGINT)
		| (1 << SIGHUP)
		,
	SPECIAL_JOBSTOP_SIGS = 0
#ಆದರೆ ENABLE_HUSH_ಕೆಲಸ
		| (1 << SIGTTIN)
		| (1 << SIGTTOU)
		| (1 << SIGTSTP)
#ಆದರೆ_ಅಂತ್ಯ
		,
};

ಸ್ಥಿರ ಶೂನ್ಯ ದಾಖಲೆ_pending_signo(ಪೂರ್ಣಾಂಕ sig)
{
	sigaddset(&G.pending_ಗಣ, sig);
#ಆದರೆ ENABLE_FEATURE_EDITING
	ಆದರೆ (sig != SIGCHLD
	 || (G_traps && G_traps[SIGCHLD] && G_traps[SIGCHLD][0])
	 /* ^^^ if SIGCHLD, interrupt line reading only if it has a trap */
	) {
		bb_got_signal = sig; /* for read_line_input: "we got a signal" */
	}
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_HUSH_FAST
	ಆದರೆ (sig == SIGCHLD) {
		G.ಎಣಿಕೆ_SIGCHLD++;
//bb_error_msg("[%d] SIGCHLD_handler: G.count_SIGCHLD:%d G.handled_SIGCHLD:%d", getpid(), G.count_SIGCHLD, G.handled_SIGCHLD);
	}
#ಆದರೆ_ಅಂತ್ಯ
}

ಸ್ಥಿರ sighandler_t install_sighandler(ಪೂರ್ಣಾಂಕ sig, sighandler_t handler)
{
	ರಚನೆ sigaction old_sa;

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
	ಹಿಂತಿರುಗಿ old_sa.sa_handler;
}
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
ಸ್ಥಿರ ಶೂನ್ಯ hush_exit(ಪೂರ್ಣಾಂಕ exitcode) NORETURN;

ಸ್ಥಿರ ಶೂನ್ಯ restore_ttypgrp_and__exit(ಶೂನ್ಯ) NORETURN;
ಸ್ಥಿರ ಶೂನ್ಯ restore_ttypgrp_and__exit(ಶೂನ್ಯ)
{
	/* xfunc has failed! die die die */
	/* no EXIT traps, this is an escape hatch! */
	G.exiting = 1;
	hush_exit(xfunc_error_retval);
}

#ಆದರೆ ENABLE_HUSH_ಕೆಲಸ

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
ಸ್ಥಿರ ಶೂನ್ಯ fflush_and__exit(ಶೂನ್ಯ) NORETURN;
ಸ್ಥಿರ ಶೂನ್ಯ fflush_and__exit(ಶೂನ್ಯ)
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
ಸ್ಥಿರ ಶೂನ್ಯ sigexit(ಪೂರ್ಣಾಂಕ sig) NORETURN;
ಸ್ಥಿರ ಶೂನ್ಯ sigexit(ಪೂರ್ಣಾಂಕ sig)
{
	/* Careful: we can end up here after [v]fork. Do not restore
	 * tty pgrp then, only top-level shell process does that */
	ಆದರೆ (G_saved_tty_pgrp && getpid() == G.ಬೇರು_pid) {
		/* Disable all signals: job control, SIGPIPE, etc.
		 * Mostly paranoid measure, to prevent infinite SIGTTOU.
		 */
		sigprocmask_allsigs(SIG_ಖಂಡ);
		tcsetpgrp(G_interactive_fd, G_saved_tty_pgrp);
	}

	/* Not a signal, just exit */
	ಆದರೆ (sig <= 0)
		_exit(- sig);

	kill_myself_with_sig(sig); /* does not return */
}
#ಇಲ್ಲದಿದ್ದರೆ

# define disable_restore_tty_pgrp_on_exit() ((ಶೂನ್ಯ)0)
# define enable_restore_tty_pgrp_on_exit()  ((ಶೂನ್ಯ)0)

#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
ಸ್ಥಿರ sighandler_t pick_sighandler(ಚಿಹ್ನೆರಹಿತ sig)
{
	sighandler_t handler = SIG_DFL;
	ಆದರೆ (sig < ಗಾತ್ರ(ಚಿಹ್ನೆರಹಿತ)*8) {
		ಚಿಹ್ನೆರಹಿತ sigmask = (1 << sig);

#ಆದರೆ ENABLE_HUSH_ಕೆಲಸ
		/* is sig fatal? */
		ಆದರೆ (G_fatal_sig_ಮುಖವಾಡ & sigmask)
			handler = sigexit;
		ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
		/* sig has special handling? */
		ಆದರೆ (G.special_sig_ಮುಖವಾಡ & sigmask) {
			handler = ದಾಖಲೆ_pending_signo;
			/* TTIN/TTOU/TSTP can't be set to record_pending_signo
			 * in order to ignore them: they will be raised
			 * in an endless loop when we try to do some
			 * terminal ioctls! We do have to _ignore_ these.
			 */
			ಆದರೆ (SPECIAL_JOBSTOP_SIGS & sigmask)
				handler = SIG_IGN;
		}
	}
	ಹಿಂತಿರುಗಿ handler;
}
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
/* Restores tty foreground process group, and exits. */
ಸ್ಥಿರ ಶೂನ್ಯ hush_exit(ಪೂರ್ಣಾಂಕ exitcode)
{
#ಆದರೆ ENABLE_FEATURE_EDITING_SAVE_ON_EXIT
	save_history(G.ಸಾಲು_ಒಳಹರಿವು_state); /* may be NULL */
#ಆದರೆ_ಅಂತ್ಯ

	fflush_all();
	ಆದರೆ (G.exiting <= 0 && G_traps && G_traps[0] && G_traps[0][0]) {
		ಅಕ್ಷರ *argv[3];
		/* argv[0] is unused */
		argv[1] = xstrdup(G_traps[0]); /* copy, since EXIT trap handler may modify G_traps[0] */
		argv[2] = ಖಾಲಿ;
		G.exiting = 1; /* prevent EXIT trap recursion */
		/* Note: G_traps[0] is not cleared!
		 * "trap" will still show it, if executed
		 * in the handler */
		builtin_eval(argv);
	}

#ಆದರೆ ENABLE_FEATURE_CLEAN_ಮೇಲೆ
	{
		ರಚನೆ variable *cur_var;
		ಆದರೆ (G.cwd != bb_ಸಂದೇಶ_unknown)
			ಬಿಡುಗಡೆ((ಅಕ್ಷರ*)G.cwd);
		cur_var = G.ತುದಿ_var;
		ಆಗ (cur_var) {
			ರಚನೆ variable *tmp = cur_var;
			ಆದರೆ (!cur_var->max_len)
				ಬಿಡುಗಡೆ(cur_var->varstr);
			cur_var = cur_var->ಮುಂದಿನ;
			ಬಿಡುಗಡೆ(tmp);
		}
	}
#ಆದರೆ_ಅಂತ್ಯ

	fflush_all();
#ಆದರೆ ENABLE_HUSH_ಕೆಲಸ
	sigexit(- (exitcode & 0xff));
#ಇಲ್ಲದಿದ್ದರೆ
	_exit(exitcode);
#ಆದರೆ_ಅಂತ್ಯ
}

//TODO: return a mask of ALL handled sigs?
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ check_and_ಸತತ_traps(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಂಕ ಕೊನೆಯ_sig = 0;

	ಆಗ (1) {
		ಪೂರ್ಣಾಂಕ sig;

		ಆದರೆ (sigisemptyset(&G.pending_ಗಣ))
			ನಿಲ್ಲಿಸು;
		sig = 0;
		ಮಾಡು {
			sig++;
			ಆದರೆ (sigismember(&G.pending_ಗಣ, sig)) {
				sigdelset(&G.pending_ಗಣ, sig);
				ಹೋಗು got_sig;
			}
		} ಆಗ (sig < NSIG);
		ನಿಲ್ಲಿಸು;
 got_sig:
#ಆದರೆ ENABLE_HUSH_TRAP
		ಆದರೆ (G_traps && G_traps[sig]) {
			debug_printf_exec("%s: sig:%d handler:'%s'\n", __func__, sig, G.traps[sig]);
			ಆದರೆ (G_traps[sig][0]) {
				/* We have user-defined handler */
				smalluint save_rcode;
				ಪೂರ್ಣಾಂಕ save_pre;
				ಅಕ್ಷರ *argv[3];
				/* argv[0] is unused */
				argv[1] = xstrdup(G_traps[sig]);
				/* why strdup? trap can modify itself: trap 'trap "echo oops" INT' INT */
				argv[2] = ಖಾಲಿ;
				save_pre = G.pre_trap_exitcode;
				G.pre_trap_exitcode = save_rcode = G.ಕೊನೆಯ_exitcode;
				builtin_eval(argv);
				ಬಿಡುಗಡೆ(argv[1]);
				G.pre_trap_exitcode = save_pre;
				G.ಕೊನೆಯ_exitcode = save_rcode;
# ಆದರೆ ENABLE_HUSH_FUNCTIONS
				ಆದರೆ (G.return_exitcode >= 0) {
					debug_printf_exec("trap exitcode:%d\n", G.return_exitcode);
					G.ಕೊನೆಯ_exitcode = G.return_exitcode;
				}
# endif
				ಕೊನೆಯ_sig = sig;
			} /* else: "" trap, ignoring signal */
			ಮುಂದುವರಿ;
		}
#ಆದರೆ_ಅಂತ್ಯ
		/* not a trap: special action */
		ಆಯ್ಕೆ (sig) {
		ಸ್ಥಿತಿ SIGINT:
			debug_printf_exec("%s: sig:%d default SIGINT handler\n", __func__, sig);
			G.flag_SIGINT = 1;
			ಕೊನೆಯ_sig = sig;
			ನಿಲ್ಲಿಸು;
#ಆದರೆ ENABLE_HUSH_ಕೆಲಸ
		ಸ್ಥಿತಿ SIGHUP: {
			/* if (G_interactive_fd) - no need to check, the handler
			 * is only installed if we *are* interactive */
			{
				/* bash compat: "Before exiting, an interactive
				 * shell resends the SIGHUP to all jobs, running
				 * or stopped.  Stopped jobs are sent SIGCONT
				 * to ensure that they receive the SIGHUP."
				 */
				ರಚನೆ pipe *ಕೆಲಸ;
				debug_printf_exec("%s: sig:%d default SIGHUP handler\n", __func__, sig);
				/* bash is observed to signal whole process groups,
				 * not individual processes */
				ಗಾಗಿ (ಕೆಲಸ = G.ಕೆಲಸ_list; ಕೆಲಸ; ಕೆಲಸ = ಕೆಲಸ->ಮುಂದಿನ) {
					ಆದರೆ (ಕೆಲಸ->pgrp <= 0)
						ಮುಂದುವರಿ;
					debug_printf_exec("HUPing pgrp %d\n", ಕೆಲಸ->pgrp);
					ಆದರೆ (kill(- ಕೆಲಸ->pgrp, SIGHUP) == 0)
						kill(- ಕೆಲಸ->pgrp, SIGCONT);
				}
			}
			/* this restores tty pgrp, then kills us with SIGHUP */
			sigexit(SIGHUP);
		}
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_HUSH_FAST
		ಸ್ಥಿತಿ SIGCHLD:
			debug_printf_exec("%s: sig:%d default SIGCHLD handler\n", __func__, sig);
			G.ಎಣಿಕೆ_SIGCHLD++;
//bb_error_msg("[%d] check_and_run_traps: G.count_SIGCHLD:%d G.handled_SIGCHLD:%d", getpid(), G.count_SIGCHLD, G.handled_SIGCHLD);
			/* Note:
			 * We don't do 'last_sig = sig' here -> NOT returning this sig.
			 * This simplifies wait builtin a bit.
			 */
			ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ
		ಪೂರ್ವನಿಯೋಜಿತ: /* ignored: */
			debug_printf_exec("%s: sig:%d default handling is to ignore\n", __func__, sig);
			/* SIGTERM, SIGQUIT, SIGTTIN, SIGTTOU, SIGTSTP */
			/* Note:
			 * We don't do 'last_sig = sig' here -> NOT returning this sig.
			 * Example: wait is not interrupted by TERM
			 * in interactive shell, because TERM is ignored.
			 */
			ನಿಲ್ಲಿಸು;
		}
	}
	ಹಿಂತಿರುಗಿ ಕೊನೆಯ_sig;
}

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *get_cwd(ಪೂರ್ಣಾಂಕ force)
{
	ಆದರೆ (force || G.cwd == ಖಾಲಿ) {
		/* xrealloc_getcwd_or_warn(arg) calls free(arg),
		 * we must not try to free(bb_msg_unknown) */
		ಆದರೆ (G.cwd == bb_ಸಂದೇಶ_unknown)
			G.cwd = ಖಾಲಿ;
		G.cwd = xrealloc_getcwd_or_warn((ಅಕ್ಷರ *)G.cwd);
		ಆದರೆ (!G.cwd)
			G.cwd = bb_ಸಂದೇಶ_unknown;
	}
	ಹಿಂತಿರುಗಿ G.cwd;
}

#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */

/*
 * Shell and environment variable support
 */
ಸ್ಥಿರ ರಚನೆ variable **get_ptr_ಗಮ್ಯ_local_var(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name)
{
	ರಚನೆ variable **pp;
	ರಚನೆ variable *cur;

	pp = &G.ತುದಿ_var;
	ಆಗ ((cur = *pp) != ಖಾಲಿ) {
		ಆದರೆ (varcmp(cur->varstr, name) == 0)
			ಹಿಂತಿರುಗಿ pp;
		pp = &cur->ಮುಂದಿನ;
	}
	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* FAST_FUNC get_local_var_ಮೌಲ್ಯ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name)
{
	ರಚನೆ variable **vpp;

	ಆದರೆ (G.expanded_assignments) {
		ಅಕ್ಷರ **cpp = G.expanded_assignments;
		ಆಗ (*cpp) {
			ಅಕ್ಷರ *cp = *cpp;
			ಆದರೆ (varcmp(cp, name) == 0)
				ಹಿಂತಿರುಗಿ strchr(cp, '=') + 1;
			cpp++;
		}
	}

	vpp = get_ptr_ಗಮ್ಯ_local_var(name);
	ಆದರೆ (vpp)
		ಹಿಂತಿರುಗಿ strchr((*vpp)->varstr, '=') + 1;

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	ಆದರೆ (strcmp(name, "PPID") == 0)
		ಹಿಂತಿರುಗಿ utoa(G.ಬೇರು_ppid);
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */
	// bash compat: UID? EUID?
#ಆದರೆ ENABLE_HUSH_RANDOM_SUPPORT
	ಆದರೆ (strcmp(name, "RANDOM") == 0)
		ಹಿಂತಿರುಗಿ utoa(ಮುಂದಿನ_random(&G.random_gen));
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_HUSH_LINENO_VAR
	ಆದರೆ (strcmp(name, "LINENO") == 0)
		ಹಿಂತಿರುಗಿ utoa(G.execute_lineno);
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ BASH_EPOCH_VARS
	{
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *fmt = ಖಾಲಿ;
		ಆದರೆ (strcmp(name, "EPOCHSECONDS") == 0)
			fmt = "%llu";
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (strcmp(name, "EPOCHREALTIME") == 0)
			fmt = "%llu.%06u";
		ಆದರೆ (fmt) {
			ರಚನೆ timeval tv;
			xgettimeofday(&tv);
			sprintf(G.epoch_buf, fmt, (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ)tv.tv_sec,
					(ಚಿಹ್ನೆರಹಿತ)tv.tv_usec);
			ಹಿಂತಿರುಗಿ G.epoch_buf;
		}
	}
#ಆದರೆ_ಅಂತ್ಯ
	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
#ಆದರೆ ENABLE_HUSH_GETOPTS
ಸ್ಥಿರ ಶೂನ್ಯ handle_ಬದಲಾಯಿತು_special_ಹೆಸರುಗಳು(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name)
{
	ಆದರೆ (varcmp(name, "OPTIND") == 0) {
		G.getopt_ಎಣಿಕೆ = 0;
		ಹಿಂತಿರುಗಿ;
	}
}
#ಇಲ್ಲದಿದ್ದರೆ
/* Do not even bother evaluating arguments */
# define handle_ಬದಲಾಯಿತು_special_ಹೆಸರುಗಳು(...) ((ಶೂನ್ಯ)0)
#ಆದರೆ_ಅಂತ್ಯ
#ಇಲ್ಲದಿದ್ದರೆ /* __U_BOOT__ */
/* Do not even bother evaluating arguments */
# define handle_ಬದಲಾಯಿತು_special_ಹೆಸರುಗಳು(...) ((ಶೂನ್ಯ)0)
#ಆದರೆ_ಅಂತ್ಯ /* __U_BOOT__ */

/* str holds "NAME=VAL" and is expected to be malloced.
 * We take ownership of it.
 */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
#ವ್ಯಾಖ್ಯಾನಿಸು SETFLAG_EXPORT   (1 << 0)
#ವ್ಯಾಖ್ಯಾನಿಸು SETFLAG_UNEXPORT (1 << 1)
#ವ್ಯಾಖ್ಯಾನಿಸು SETFLAG_MAKE_RO  (1 << 2)
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */
#ವ್ಯಾಖ್ಯಾನಿಸು SETFLAG_VARLVL_ಸರಿಸು   3
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಗಣ_local_var(ಅಕ್ಷರ *str, ಚಿಹ್ನೆರಹಿತ flags)
#ಇಲ್ಲದಿದ್ದರೆ /* __U_BOOT__ */
ಪೂರ್ಣಾಂಕ ಗಣ_local_var_modern(ಅಕ್ಷರ *str, ಪೂರ್ಣಾಂಕ flags)
#ಆದರೆ_ಅಂತ್ಯ /* __U_BOOT__ */
{
	ರಚನೆ variable **cur_pp;
	ರಚನೆ variable *cur;
	ಅಕ್ಷರ *free_me = ಖಾಲಿ;
	ಅಕ್ಷರ *eq_sign;
	ಪೂರ್ಣಾಂಕ name_len;
	ಪೂರ್ಣಾಂಕ retval;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	ಚಿಹ್ನೆರಹಿತ local_lvl = (flags >> SETFLAG_VARLVL_ಸರಿಸು);
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */

	eq_sign = strchr(str, '=');
	ಆದರೆ (HUSH_DEBUG && !eq_sign)
		bb_simple_error_ಸಂದೇಶ_and_die("BUG in setvar");

	name_len = eq_sign - str + 1; /* including '=' */
	cur_pp = &G.ತುದಿ_var;
	ಆಗ ((cur = *cur_pp) != ಖಾಲಿ) {
		ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಹೋಲಿಕೆ(cur->varstr, str, name_len) != 0) {
			cur_pp = &cur->ಮುಂದಿನ;
			ಮುಂದುವರಿ;
		}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
		/* We found an existing var with this name */
		ಆದರೆ (cur->flg_read_only) {
			bb_error_ಸಂದೇಶ("%s: readonly variable", str);
			ಬಿಡುಗಡೆ(str);
//NOTE: in bash, assignment in "export READONLY_VAR=Z" fails, and sets $?=1,
//but export per se succeeds (does put the var in env). We don't mimic that.
			ಹಿಂತಿರುಗಿ -1;
		}
		ಆದರೆ (flags & SETFLAG_UNEXPORT) { // && cur->flg_export ?
			debug_printf_env("%s: unsetenv '%s'\n", __func__, str);
			*eq_sign = '\0';
			unsetenv(str);
			*eq_sign = '=';
		}
		ಆದರೆ (cur->var_nest_level < local_lvl) {
			/* bash 3.2.33(1) and exported vars:
			 * # export z=z
			 * # f() { local z=a; env | grep ^z; }
			 * # f
			 * z=a
			 * # env | grep ^z
			 * z=z
			 */
			ಆದರೆ (cur->flg_export)
				flags |= SETFLAG_EXPORT;
			/* New variable is local ("local VAR=VAL" or
			 * "VAR=VAL cmd")
			 * and existing one is global, or local
			 * on a lower level that new one.
			 * Remove it from global variable list:
			 */
			*cur_pp = cur->ಮುಂದಿನ;
			ಆದರೆ (G.shadowed_vars_pp) {
				/* Save in "shadowed" list */
				debug_printf_env("shadowing %s'%s'/%u by '%s'/%u\n",
					cur->flg_export ? "exported " : "",
					cur->varstr, cur->var_nest_level, str, local_lvl
				);
				cur->ಮುಂದಿನ = *G.shadowed_vars_pp;
				*G.shadowed_vars_pp = cur;
			} ಇಲ್ಲದಿದ್ದರೆ {
				/* Came from pseudo_exec_argv(), no need to save: delete it */
				debug_printf_env("shadow-deleting %s'%s'/%u by '%s'/%u\n",
					cur->flg_export ? "exported " : "",
					cur->varstr, cur->var_nest_level, str, local_lvl
				);
				ಆದರೆ (cur->max_len == 0) /* allocated "VAR=VAL"? */
					free_me = cur->varstr; /* then free it later */
				ಬಿಡುಗಡೆ(cur);
			}
			ನಿಲ್ಲಿಸು;
		}
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */

		ಆದರೆ (strcmp(cur->varstr + name_len, eq_sign + 1) == 0) {
			debug_printf_env("assignement '%s' does not change anything\n", str);
 free_and_exp:
			ಬಿಡುಗಡೆ(str);
			ಹೋಗು ಘಾತಾಂಕ;
		}

		/* Replace the value in the found "struct variable" */
		ಆದರೆ (cur->max_len != 0) {
			ಆದರೆ (cur->max_len >= strnlen(str, cur->max_len + 1)) {
				/* This one is from startup env, reuse space */
				debug_printf_env("reusing startup env for '%s'\n", str);
				strcpy(cur->varstr, str);
				ಹೋಗು free_and_exp;
			}
			/* Can't reuse */
			cur->max_len = 0;
			ಹೋಗು ಗಣ_str_and_exp;
		}
		/* max_len == 0 signifies "malloced" var, which we can
		 * (and have to) free. But we can't free(cur->varstr) here:
		 * if cur->flg_export is 1, it is in the environment.
		 * We should either unsetenv+free, or wait until putenv,
		 * then putenv(new)+free(old).
		 */
		free_me = cur->varstr;
		ಹೋಗು ಗಣ_str_and_exp;
	}

	/* Not found or shadowed - create new variable struct */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	debug_printf_env("%s: alloc new var '%s'/%u\n", __func__, str, local_lvl);
#ಇಲ್ಲದಿದ್ದರೆ /* __U_BOOT__ */
	debug_printf_env("%s: alloc new var '%s'\n", __func__, str);
#ಆದರೆ_ಅಂತ್ಯ /* __U_BOOT__ */
	cur = xzalloc(ಗಾತ್ರ(*cur));
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	cur->var_nest_level = local_lvl;
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */
	cur->ಮುಂದಿನ = *cur_pp;
	*cur_pp = cur;

 ಗಣ_str_and_exp:
	cur->varstr = str;
 ಘಾತಾಂಕ:
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
#ಆದರೆ !BB_MMU || ENABLE_HUSH_READONLY
	ಆದರೆ (flags & SETFLAG_MAKE_RO) {
		cur->flg_read_only = 1;
	}
#ಆದರೆ_ಅಂತ್ಯ
	ಆದರೆ (flags & SETFLAG_EXPORT)
		cur->flg_export = 1;
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */
	retval = 0;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	ಆದರೆ (cur->flg_export) {
		ಆದರೆ (flags & SETFLAG_UNEXPORT) {
			cur->flg_export = 0;
			/* unsetenv was already done */
		} ಇಲ್ಲದಿದ್ದರೆ {
			debug_printf_env("%s: putenv '%s'/%u\n", __func__, cur->varstr, cur->var_nest_level);
			retval = putenv(cur->varstr);
			/* fall through to "free(free_me)" -
			 * only now we can free old exported malloced string
			 */
		}
	}
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */
	ಬಿಡುಗಡೆ(free_me);

	handle_ಬದಲಾಯಿತು_special_ಹೆಸರುಗಳು(cur->varstr);

	ಹಿಂತಿರುಗಿ retval;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಗಣ_local_var0(ಅಕ್ಷರ *str)
{
	ಹಿಂತಿರುಗಿ ಗಣ_local_var(str, 0);
}

ಸ್ಥಿರ ಶೂನ್ಯ FAST_FUNC ಗಣ_local_var_from_halves(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *val)
{
	ಅಕ್ಷರ *var = xasprintf("%s=%s", name, val);
	ಗಣ_local_var0(var);
}

/* Used at startup and after each cd */
ಸ್ಥಿರ ಶೂನ್ಯ ಗಣ_pwd_var(ಚಿಹ್ನೆರಹಿತ flag)
{
	ಗಣ_local_var(xasprintf("PWD=%s", get_cwd(/*force:*/ 1)), flag);
}
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */

#ಆದರೆ ENABLE_HUSH_UNSET || ENABLE_HUSH_GETOPTS
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ unset_local_var(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name)
{
	ರಚನೆ variable *cur;
	ರಚನೆ variable **cur_pp;

	cur_pp = &G.ತುದಿ_var;
	ಆಗ ((cur = *cur_pp) != ಖಾಲಿ) {
		ಆದರೆ (varcmp(cur->varstr, name) == 0) {
			ಆದರೆ (cur->flg_read_only) {
				bb_error_ಸಂದೇಶ("%s: readonly variable", name);
				ಹಿಂತಿರುಗಿ EXIT_FAILURE;
			}

			*cur_pp = cur->ಮುಂದಿನ;
			debug_printf_env("%s: unsetenv '%s'\n", __func__, cur->varstr);
			bb_unsetenv(cur->varstr);
			ಆದರೆ (!cur->max_len)
				ಬಿಡುಗಡೆ(cur->varstr);
			ಬಿಡುಗಡೆ(cur);

			ನಿಲ್ಲಿಸು;
		}
		cur_pp = &cur->ಮುಂದಿನ;
	}

	/* Handle "unset LINENO" et al even if did not find the variable to unset */
	handle_ಬದಲಾಯಿತು_special_ಹೆಸರುಗಳು(name);

	ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
}
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
/*
 * Helpers for "var1=val1 var2=val2 cmd" feature
 */
ಸ್ಥಿರ ಶೂನ್ಯ ಕೂಡಿಸು_vars(ರಚನೆ variable *var)
{
	ರಚನೆ variable *ಮುಂದಿನ;

	ಆಗ (var) {
		ಮುಂದಿನ = var->ಮುಂದಿನ;
		var->ಮುಂದಿನ = G.ತುದಿ_var;
		G.ತುದಿ_var = var;
		ಆದರೆ (var->flg_export) {
			debug_printf_env("%s: restoring exported '%s'/%u\n", __func__, var->varstr, var->var_nest_level);
			putenv(var->varstr);
		} ಇಲ್ಲದಿದ್ದರೆ {
			debug_printf_env("%s: restoring variable '%s'/%u\n", __func__, var->varstr, var->var_nest_level);
		}
		var = ಮುಂದಿನ;
	}
}

/* We put strings[i] into variable table and possibly putenv them.
 * If variable is read only, we can free the strings[i]
 * which attempts to overwrite it.
 * The strings[] vector itself is freed.
 */
ಸ್ಥಿರ ಶೂನ್ಯ ಗಣ_vars_and_save_old(ಅಕ್ಷರ **ಅಕ್ಷರಸರಣಿಗಳು)
{
	ಅಕ್ಷರ **s;

	ಆದರೆ (!ಅಕ್ಷರಸರಣಿಗಳು)
		ಹಿಂತಿರುಗಿ;

	s = ಅಕ್ಷರಸರಣಿಗಳು;
	ಆಗ (*s) {
		ರಚನೆ variable *var_p;
		ರಚನೆ variable **var_pp;
		ಅಕ್ಷರ *eq;

		eq = strchr(*s, '=');
		ಆದರೆ (HUSH_DEBUG && !eq)
			bb_simple_error_ಸಂದೇಶ_and_die("BUG in varexp4");
		var_pp = get_ptr_ಗಮ್ಯ_local_var(*s);
		ಆದರೆ (var_pp) {
			var_p = *var_pp;
			ಆದರೆ (var_p->flg_read_only) {
				ಅಕ್ಷರ **p;
				bb_error_ಸಂದೇಶ("%s: readonly variable", *s);
				/*
				 * "VAR=V BLTIN" unsets VARs after BLTIN completes.
				 * If VAR is readonly, leaving it in the list
				 * after asssignment error (msg above)
				 * causes doubled error message later, on unset.
				 */
				debug_printf_env("removing/freeing '%s' element\n", *s);
				ಬಿಡುಗಡೆ(*s);
				p = s;
				ಮಾಡು { *p = p[1]; p++; } ಆಗ (*p);
				ಹೋಗು ಮುಂದಿನ;
			}
			/* below, set_local_var() with nest level will
			 * "shadow" (remove) this variable from
			 * global linked list.
			 */
		}
		debug_printf_env("%s: env override '%s'/%u\n", __func__, *s, G.var_nest_level);
		ಗಣ_local_var(*s, (G.var_nest_level << SETFLAG_VARLVL_ಸರಿಸು) | SETFLAG_EXPORT);
		s++;
 ಮುಂದಿನ: ;
	}
	ಬಿಡುಗಡೆ(ಅಕ್ಷರಸರಣಿಗಳು);
}

/*
 * Unicode helper
 */
ಸ್ಥಿರ ಶೂನ್ಯ reinit_unicode_for_hush(ಶೂನ್ಯ)
{
	/* Unicode support should be activated even if LANG is set
	 * _during_ shell execution, not only if it was set when
	 * shell was started. Therefore, re-check LANG every time:
	 */
	ಆದರೆ (ENABLE_FEATURE_CHECK_UNICODE_ಒಳ_ENV
	 || ENABLE_UNICODE_USING_LOCALE
	) {
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s = get_local_var_ಮೌಲ್ಯ("LC_ALL");
		ಆದರೆ (!s) s = get_local_var_ಮೌಲ್ಯ("LC_CTYPE");
		ಆದರೆ (!s) s = get_local_var_ಮೌಲ್ಯ("LANG");
		reinit_unicode(s);
	}
}

#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */
/*
 * in_str support (strings, and "strings" read from files).
 */

#ಆದರೆ ENABLE_HUSH_INTERACTIVE
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
/* To test correct lineedit/interactive behavior, type from command line:
 *	echo $P\
 *	\
 *	AT\
 *	H\
 *	\
 * It exercises a lot of corner cases.
 */
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸಿದ್ಧತೆ_prompt_string(ಶೂನ್ಯ)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *prompt_str;

	debug_printf_prompt("%s promptmode:%d\n", __func__, G.promptmode);

# ಆದರೆ ENABLE_FEATURE_EDITING_FANCY_PROMPT
	prompt_str = get_local_var_ಮೌಲ್ಯ(G.promptmode == 0 ? "PS1" : "PS2");
	ಆದರೆ (!prompt_str)
		prompt_str = "";
# ಇಲ್ಲದಿದ್ದರೆ
	prompt_str = "> "; /* if PS2, else... */
	ಆದರೆ (G.promptmode == 0) { /* PS1 */
		/* No fancy prompts supported, (re)generate "CURDIR $ " by hand */
		ಬಿಡುಗಡೆ(G.PS1);
		/* bash uses $PWD value, even if it is set by user.
		 * It uses current dir only if PWD is unset.
		 * We always use current dir. */
		prompt_str = G.PS1 = xasprintf("%s %c ", get_cwd(0), (geteuid() != 0) ? '$' : '#');
	}
# endif
	debug_printf("prompt_str '%s'\n", prompt_str);
	ಹಿಂತಿರುಗಿ prompt_str;
}
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ get_user_ಒಳಹರಿವು(ರಚನೆ ಒಳ_str *i)
#ಇಲ್ಲದಿದ್ದರೆ /* __U_BOOT__ */
ಸ್ಥಿರ ಶೂನ್ಯ get_user_ಒಳಹರಿವು(ರಚನೆ ಒಳ_str *i)
#ಆದರೆ_ಅಂತ್ಯ /* __U_BOOT__ */
{
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
# ಆದರೆ ENABLE_FEATURE_EDITING
	/* In EDITING case, this function reads next input line,
	 * saves it in i->p, then returns 1st char of it.
	 */
	ಪೂರ್ಣಾಂಕ r;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *prompt_str;

	prompt_str = ಸಿದ್ಧತೆ_prompt_string();
	ಗಾಗಿ (;;) {
		reinit_unicode_for_hush();
		G.flag_SIGINT = 0;

		bb_got_signal = 0;
		ಆದರೆ (!sigisemptyset(&G.pending_ಗಣ)) {
			/* Whoops, already got a signal, do not call read_line_input */
			bb_got_signal = r = -1;
		} ಇಲ್ಲದಿದ್ದರೆ {
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
			r = read_ಸಾಲು_ಒಳಹರಿವು(G.ಸಾಲು_ಒಳಹರಿವು_state, prompt_str,
				G.user_ಒಳಹರಿವು_buf, CONFIG_FEATURE_EDITING_MAX_LEN-1
			);
			/* read_line_input intercepts ^C, "convert" it to SIGINT */
			ಆದರೆ (r == 0)
				ಎತ್ತು(SIGINT);
		}
		/* bash prints ^C (before running a trap, if any)
		 * both on keyboard ^C and on real SIGINT (non-kbd generated).
		 */
		ಆದರೆ (sigismember(&G.pending_ಗಣ, SIGINT)) {
			ಲಿಖ(STDOUT_FILENO, "^C\n", 3);
			G.ಕೊನೆಯ_exitcode = 128 | SIGINT;
		}
		check_and_ಸತತ_traps();
		ಆದರೆ (r == 0) /* keyboard ^C? */
			ಮುಂದುವರಿ; /* go back, read another input line */
		ಆದರೆ (r > 0) /* normal input? (no ^C, no ^D, no signals) */
			ನಿಲ್ಲಿಸು;
		ಆದರೆ (!bb_got_signal) {
			/* r < 0: ^D/EOF/error detected (but not signal) */
			/* ^D on interactive input goes to next line before exiting: */
			ಲಿಖ(STDOUT_FILENO, "\n", 1);
			i->p = ಖಾಲಿ;
			i->peek_buf[0] = r = EOF;
			ಹಿಂತಿರುಗಿ r;
		}
		/* it was a signal: go back, read another input line */
	}
	i->p = G.user_ಒಳಹರಿವು_buf;
	ಹಿಂತಿರುಗಿ (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)*i->p++;
# ಇಲ್ಲದಿದ್ದರೆ
	/* In !EDITING case, this function gets called for every char.
	 * Buffering happens deeper in the call chain, in hfgetc(i->file).
	 */
	ಪೂರ್ಣಾಂಕ r;

	ಗಾಗಿ (;;) {
		G.flag_SIGINT = 0;
		ಆದರೆ (i->ಕೊನೆಯ_char == '\0' || i->ಕೊನೆಯ_char == '\n') {
			ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *prompt_str = ಸಿದ್ಧತೆ_prompt_string();
			/* Why check_and_run_traps here? Try this interactively:
			 * $ trap 'echo INT' INT; (sleep 2; kill -INT $$) &
			 * $ <[enter], repeatedly...>
			 * Without check_and_run_traps, handler never runs.
			 */
			check_and_ಸತತ_traps();
			fputs_stdout(prompt_str);
			fflush_all();
		}
		r = hfgetc(i->file);
		/* In !ENABLE_FEATURE_EDITING we don't use read_line_input,
		 * no ^C masking happens during fgetc, no special code for ^C:
		 * it generates SIGINT as usual.
		 */
		check_and_ಸತತ_traps();
		ಆದರೆ (r != '\0' && !G.flag_SIGINT)
			ನಿಲ್ಲಿಸು;
		ಆದರೆ (G.flag_SIGINT) {
			/* ^C or SIGINT: repeat */
			/* bash prints ^C even on real SIGINT (non-kbd generated) */
			/* kernel prints "^C" itself, just print newline: */
			ಲಿಖ(STDOUT_FILENO, "\n", 1);
			G.ಕೊನೆಯ_exitcode = 128 | SIGINT;
		}
	}
	ಹಿಂತಿರುಗಿ r;
# endif
#ಇಲ್ಲದಿದ್ದರೆ /* __U_BOOT__ */
	ಪೂರ್ಣಾಂಕ n;
	ಪೂರ್ಣಾಂಕ promptme;
	ಸ್ಥಿರ ಅಕ್ಷರ the_command[CONFIG_SYS_CBSIZE + 1];

	bootretry_reset_cmd_timeout();
	promptme = 1;
	n = u_boot_cli_readline(i);

# ifdef CONFIG_BOOT_RETRY_ಸಮಯ
	ಆದರೆ (n == -2) {
		puts("\nTimeout waiting for command\n");
#  ifdef CONFIG_RESET_ಗಮ್ಯ_RETRY
		do_reset(ಖಾಲಿ, 0, 0, ಖಾಲಿ);
#  elif IS_ENABLED(CONFIG_RETRY_BOOTCMD)
	strcpy(console_ಬಫರ್, "run bootcmd\n");
# ಇಲ್ಲದಿದ್ದರೆ
#	error "This only works with CONFIG_RESET_TO_RETRY or CONFIG_BOOT_RETRY_COMMAND enabled"
#  endif
	}
# endif
	ಆದರೆ (n == -1 ) {
		G.flag_repeat = 0;
		promptme = 0;
	}
	n = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(console_ಬಫರ್);
	console_ಬಫರ್[n] = '\n';
	console_ಬಫರ್[n+1]= '\0';
	ಆದರೆ (had_ctrlc())
		G.flag_repeat = 0;
	clear_ctrlc();
	G.do_repeat = 0;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	ಆದರೆ (G.promptmode == 1) {
#ಇಲ್ಲದಿದ್ದರೆ /* __U_BOOT__ */
	ಆದರೆ (!G.promptmode) {
#ಆದರೆ_ಅಂತ್ಯ /* __U_BOOT__ */
		ಆದರೆ (console_ಬಫರ್[0] == '\n'&& G.flag_repeat == 0) {
			strcpy(the_command, console_ಬಫರ್);
		}
		ಇಲ್ಲದಿದ್ದರೆ {
			ಆದರೆ (console_ಬಫರ್[0] != '\n') {
				strcpy(the_command, console_ಬಫರ್);
				G.flag_repeat = 1;
			}
			ಇಲ್ಲದಿದ್ದರೆ {
				G.do_repeat = 1;
			}
		}
		i->p = the_command;
	}
	ಇಲ್ಲದಿದ್ದರೆ {
		ಆದರೆ (console_ಬಫರ್[0] != '\n') {
			ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(the_command) + ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(console_ಬಫರ್)
				< CONFIG_SYS_CBSIZE) {
				n = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(the_command);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __U_BOOT__
				/*
				 * To avoid writing to bad places, we check if
				 * n is greater than 0.
				 * This bug was found by Harald Seiler.
				 */
				ಆದರೆ (n > 0)
					the_command[n-1] = ' ';
				strcpy(&the_command[n], console_ಬಫರ್);
#ಇಲ್ಲದಿದ್ದರೆ /* !__U_BOOT__ */
			the_command[n-1] = ' ';
			strcpy(&the_command[n], console_ಬಫರ್);
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */
				}
				ಇಲ್ಲದಿದ್ದರೆ {
					the_command[0] = '\n';
					the_command[1] = '\0';
					G.flag_repeat = 0;
				}
		}
		ಆದರೆ (promptme == 0) {
			the_command[0] = '\n';
			the_command[1] = '\0';
		}
		i->p = console_ಬಫರ್;
	}
#ಆದರೆ_ಅಂತ್ಯ /* __U_BOOT__ */
}
/* This is the magic location that prints prompts
 * and gets data back from the user */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fgetc_interactive(ರಚನೆ ಒಳ_str *i)
{
	ಪೂರ್ಣಾಂಕ ch;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	/* If it's interactive stdin, get new line. */
	ಆದರೆ (G_interactive_fd && i->file == G.HFILE_stdin) {
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
		/* Returns first char (or EOF), the rest is in i->p[] */
		ch = get_user_ಒಳಹರಿವು(i);
#ಇಲ್ಲದಿದ್ದರೆ /* __U_BOOT__ */
		/* Avoid garbage value and make clang happy. */
		ch = 0;
		/*
		 * get_user_input() does not return anything when used in
		 * U-Boot.
		 * So, we need to take the read character from i->p[].
		 */
		get_user_ಒಳಹರಿವು(i);
		ಆದರೆ (i->p && *i->p) {
			ch = *i->p++;
		}
#ಆದರೆ_ಅಂತ್ಯ /* __U_BOOT__ */
		G.promptmode = 1; /* PS2 */
		debug_printf_prompt("%s promptmode=%d\n", __func__, G.promptmode);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* Not stdin: script file, sourced file, etc */
		ಮಾಡು ch = hfgetc(i->file); ಆಗ (ch == '\0');
	}
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */
	ಹಿಂತಿರುಗಿ ch;
}
#ಇಲ್ಲದಿದ್ದರೆ  /* !INTERACTIVE */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
ಸ್ಥಿರ ALWAYS_INLINE ಪೂರ್ಣಾಂಕ fgetc_interactive(ರಚನೆ ಒಳ_str *i)
{
	ಪೂರ್ಣಾಂಕ ch;
	ಮಾಡು ch = hfgetc(i->file); ಆಗ (ch == '\0');
	ಹಿಂತಿರುಗಿ ch;
}
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */
#ಆದರೆ_ಅಂತ್ಯ  /* !INTERACTIVE */

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ i_getch(ರಚನೆ ಒಳ_str *i)
{
	ಪೂರ್ಣಾಂಕ ch;

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	ಆದರೆ (!i->file) {
		/* string-based in_str */
		ch = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)*i->p;
		ಆದರೆ (ch != '\0') {
			i->p++;
			i->ಕೊನೆಯ_char = ch;
#ಆದರೆ ENABLE_HUSH_LINENO_VAR
			ಆದರೆ (ch == '\n') {
				G.parse_lineno++;
				debug_printf_parse("G.parse_lineno++ = %u\n", G.parse_lineno);
			}
#ಆದರೆ_ಅಂತ್ಯ
			ಹಿಂತಿರುಗಿ ch;
		}
		ಹಿಂತಿರುಗಿ EOF;
	}

#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */
	/* FILE-based in_str */

#ಆದರೆ ENABLE_FEATURE_EDITING
	/* This can be stdin, check line editing char[] buffer */
	ಆದರೆ (i->p && *i->p != '\0') {
		ch = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)*i->p++;
		ಹೋಗು ಹೊರಹೋಗು;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	}
#ಇಲ್ಲದಿದ್ದರೆ /* __U_BOOT__ */
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
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (i->p && *i->p == '\0' && G.ಸತತ_command_flags){
		ಹಿಂತಿರುಗಿ EOF;
	}
#ಆದರೆ_ಅಂತ್ಯ /* __U_BOOT__ */
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	/* peek_buf[] is an int array, not char. Can contain EOF. */
	ch = i->peek_buf[0];
	ಆದರೆ (ch != 0) {
		ಪೂರ್ಣಾಂಕ ch2 = i->peek_buf[1];
		i->peek_buf[0] = ch2;
		ಆದರೆ (ch2 == 0) /* very likely, avoid redundant write */
			ಹೋಗು ಹೊರಹೋಗು;
		i->peek_buf[1] = 0;
		ಹೋಗು ಹೊರಹೋಗು;
	}

#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */
	ch = fgetc_interactive(i);
 ಹೊರಹೋಗು:
	debug_printf("file_get: got '%c' %d\n", ch, ch);
	i->ಕೊನೆಯ_char = ch;
#ಆದರೆ ENABLE_HUSH_LINENO_VAR
	ಆದರೆ (ch == '\n') {
		G.parse_lineno++;
		debug_printf_parse("G.parse_lineno++ = %u\n", G.parse_lineno);
	}
#ಆದರೆ_ಅಂತ್ಯ
	ಹಿಂತಿರುಗಿ ch;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ i_peek(ರಚನೆ ಒಳ_str *i)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	ಪೂರ್ಣಾಂಕ ch;

	ಆದರೆ (!i->file) {
		/* string-based in_str */
		/* Doesn't report EOF on NUL. None of the callers care. */
		ಹಿಂತಿರುಗಿ (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)*i->p;
	}

	/* FILE-based in_str */

#ಆದರೆ ENABLE_FEATURE_EDITING && ENABLE_HUSH_INTERACTIVE
	/* This can be stdin, check line editing char[] buffer */
	ಆದರೆ (i->p && *i->p != '\0')
		ಹಿಂತಿರುಗಿ (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)*i->p;
#ಆದರೆ_ಅಂತ್ಯ
	/* peek_buf[] is an int array, not char. Can contain EOF. */
	ch = i->peek_buf[0];
	ಆದರೆ (ch != 0)
		ಹಿಂತಿರುಗಿ ch;

	/* Need to get a new char */
	ch = fgetc_interactive(i);
	debug_printf("file_peek: got '%c' %d\n", ch, ch);

	/* Save it by either rolling back line editing buffer, or in i->peek_buf[0] */
#ಆದರೆ ENABLE_FEATURE_EDITING && ENABLE_HUSH_INTERACTIVE
	ಆದರೆ (i->p) {
		i->p -= 1;
		ಹಿಂತಿರುಗಿ ch;
	}
#ಆದರೆ_ಅಂತ್ಯ
	i->peek_buf[0] = ch;
	/*i->peek_buf[1] = 0; - already is */
	ಹಿಂತಿರುಗಿ ch;
#ಇಲ್ಲದಿದ್ದರೆ /* __U_BOOT__ */
	/* string-based in_str */
	/* Doesn't report EOF on NUL. None of the callers care. */
	ಹಿಂತಿರುಗಿ (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)*i->p;
#ಆದರೆ_ಅಂತ್ಯ /* __U_BOOT__ */
}

/* Only ever called if i_peek() was called, and did not return EOF.
 * IOW: we know the previous peek saw an ordinary char, not EOF, not NUL,
 * not end-of-line. Therefore we never need to read a new editing line here.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ i_peek2(ರಚನೆ ಒಳ_str *i)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	ಪೂರ್ಣಾಂಕ ch;
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */

	/* There are two cases when i->p[] buffer exists.
	 * (1) it's a string in_str.
	 * (2) It's a file, and we have a saved line editing buffer.
	 * In both cases, we know that i->p[0] exists and not NUL, and
	 * the peek2 result is in i->p[1].
	 */
	ಆದರೆ (i->p)
		ಹಿಂತಿರುಗಿ (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)i->p[1];

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	/* Now we know it is a file-based in_str. */

	/* peek_buf[] is an int array, not char. Can contain EOF. */
	/* Is there 2nd char? */
	ch = i->peek_buf[1];
	ಆದರೆ (ch == 0) {
		/* We did not read it yet, get it now */
		ಮಾಡು ch = hfgetc(i->file); ಆಗ (ch == '\0');
		i->peek_buf[1] = ch;
	}

	debug_printf("file_peek2: got '%c' %d\n", ch, ch);
	ಹಿಂತಿರುಗಿ ch;
#ಇಲ್ಲದಿದ್ದರೆ
	ಹಿಂತಿರುಗಿ 0;
#ಆದರೆ_ಅಂತ್ಯ /* __U_BOOT__ */
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ i_getch_and_eat_bkslash_nl(ರಚನೆ ಒಳ_str *ಒಳಹರಿವು)
{
	ಗಾಗಿ (;;) {
		ಪೂರ್ಣಾಂಕ ch, ch2;

		ch = i_getch(ಒಳಹರಿವು);
		ಆದರೆ (ch != '\\')
			ಹಿಂತಿರುಗಿ ch;
		ch2 = i_peek(ಒಳಹರಿವು);
		ಆದರೆ (ch2 != '\n')
			ಹಿಂತಿರುಗಿ ch;
		/* backslash+newline, skip it */
		i_getch(ಒಳಹರಿವು);
	}
}

/* Note: this function _eats_ \<newline> pairs, safe to use plain
 * i_getch() after it instead of i_getch_and_eat_bkslash_nl().
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ i_peek_and_eat_bkslash_nl(ರಚನೆ ಒಳ_str *ಒಳಹರಿವು)
{
	ಗಾಗಿ (;;) {
		ಪೂರ್ಣಾಂಕ ch, ch2;

		ch = i_peek(ಒಳಹರಿವು);
		ಆದರೆ (ch != '\\')
			ಹಿಂತಿರುಗಿ ch;
		ch2 = i_peek2(ಒಳಹರಿವು);
		ಆದರೆ (ch2 != '\n')
			ಹಿಂತಿರುಗಿ ch;
		/* backslash+newline, skip it */
		i_getch(ಒಳಹರಿವು);
		i_getch(ಒಳಹರಿವು);
	}
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
ಸ್ಥಿರ ಶೂನ್ಯ ಸಿದ್ಧತೆ_file_ಒಳ_str(ರಚನೆ ಒಳ_str *i, HFILE *fp)
#ಇಲ್ಲದಿದ್ದರೆ /* __U_BOOT__ */
ಸ್ಥಿರ ಶೂನ್ಯ ಸಿದ್ಧತೆ_file_ಒಳ_str(ರಚನೆ ಒಳ_str *i)
#ಆದರೆ_ಅಂತ್ಯ /* __U_BOOT__ */
{
	memset(i, 0, ಗಾತ್ರ(*i));
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	i->file = fp;
	/* i->p = NULL; */
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */
}

ಸ್ಥಿರ ಶೂನ್ಯ ಸಿದ್ಧತೆ_string_ಒಳ_str(ರಚನೆ ಒಳ_str *i, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s)
{
	memset(i, 0, ಗಾತ್ರ(*i));
	/*i->file = NULL */;
	i->p = s;
}

/*
 * o_string support
 */
#ವ್ಯಾಖ್ಯಾನಿಸು B_CHUNK  (32 * ಗಾತ್ರ(ಅಕ್ಷರ*))

ಸ್ಥಿರ ಶೂನ್ಯ o_reset_ಗಮ್ಯ_ಖಾಲಿಗಳು_unquoted(o_string *o)
{
	o->ಉದ್ದಳತೆ = 0;
	o->has_quoted_part = 0;
	ಆದರೆ (o->ದತ್ತಾಂಶ)
		o->ದತ್ತಾಂಶ[0] = '\0';
}

ಸ್ಥಿರ ಶೂನ್ಯ o_free_and_ಗಣ_NULL(o_string *o)
{
	ಬಿಡುಗಡೆ(o->ದತ್ತಾಂಶ);
	memset(o, 0, ಗಾತ್ರ(*o));
}

ಸ್ಥಿರ ALWAYS_INLINE ಶೂನ್ಯ o_free(o_string *o)
{
	ಬಿಡುಗಡೆ(o->ದತ್ತಾಂಶ);
}

ಸ್ಥಿರ ಶೂನ್ಯ o_grow_by(o_string *o, ಪೂರ್ಣಾಂಕ len)
{
	ಆದರೆ (o->ಉದ್ದಳತೆ + len > o->maxlen) {
		o->maxlen += (2 * len) | (B_CHUNK-1);
		o->ದತ್ತಾಂಶ = xrealloc(o->ದತ್ತಾಂಶ, 1 + o->maxlen);
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ o_addchr(o_string *o, ಪೂರ್ಣಾಂಕ ch)
{
	debug_printf("o_addchr: '%c' o->length=%d o=%p\n", ch, o->ಉದ್ದಳತೆ, o);
	ಆದರೆ (o->ಉದ್ದಳತೆ < o->maxlen) {
		/* likely. avoid o_grow_by() call */
 ಕೂಡಿಸು:
		o->ದತ್ತಾಂಶ[o->ಉದ್ದಳತೆ] = ch;
		o->ಉದ್ದಳತೆ++;
		o->ದತ್ತಾಂಶ[o->ಉದ್ದಳತೆ] = '\0';
		ಹಿಂತಿರುಗಿ;
	}
	o_grow_by(o, 1);
	ಹೋಗು ಕೂಡಿಸು;
}

#ಆದರೆ 0
/* Valid only if we know o_string is not empty */
ಸ್ಥಿರ ಶೂನ್ಯ o_delchr(o_string *o)
{
	o->ಉದ್ದಳತೆ--;
	o->ದತ್ತಾಂಶ[o->ಉದ್ದಳತೆ] = '\0';
}
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ಶೂನ್ಯ o_addblock(o_string *o, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str, ಪೂರ್ಣಾಂಕ len)
{
	o_grow_by(o, len);
	((ಅಕ್ಷರ*)mempcpy(&o->ದತ್ತಾಂಶ[o->ಉದ್ದಳತೆ], str, len))[0] = '\0';
	o->ಉದ್ದಳತೆ += len;
}

ಸ್ಥಿರ ಶೂನ್ಯ o_addstr(o_string *o, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str)
{
	o_addblock(o, str, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(str));
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
ಸ್ಥಿರ ಶೂನ್ಯ o_addstr_with_NUL(o_string *o, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str)
{
	o_addblock(o, str, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(str) + 1);
}
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */

#ಆದರೆ !BB_MMU
ಸ್ಥಿರ ಶೂನ್ಯ nommu_addchr(o_string *o, ಪೂರ್ಣಾಂಕ ch)
{
	ಆದರೆ (o)
		o_addchr(o, ch);
}
#ಇಲ್ಲದಿದ್ದರೆ
# define nommu_addchr(o, str) ((ಶೂನ್ಯ)0)
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
#ಆದರೆ ENABLE_HUSH_ಬಹುಲಕ_X
ಸ್ಥಿರ ಶೂನ್ಯ x_ಬಹುಲಕ_addchr(ಪೂರ್ಣಾಂಕ ch)
{
	o_addchr(&G.x_ಬಹುಲಕ_buf, ch);
}
ಸ್ಥಿರ ಶೂನ್ಯ x_ಬಹುಲಕ_addstr(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str)
{
	o_addstr(&G.x_ಬಹುಲಕ_buf, str);
}
ಸ್ಥಿರ ಶೂನ್ಯ x_ಬಹುಲಕ_addblock(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str, ಪೂರ್ಣಾಂಕ len)
{
	o_addblock(&G.x_ಬಹುಲಕ_buf, str, len);
}
ಸ್ಥಿರ ಶೂನ್ಯ x_ಬಹುಲಕ_ಪೂರ್ವಪ್ರತ್ಯಯ(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಂಕ n = G.x_ಬಹುಲಕ_ಆಳ;
	ಮಾಡು x_ಬಹುಲಕ_addchr('+'); ಆಗ (--n >= 0);
}
ಸ್ಥಿರ ಶೂನ್ಯ x_ಬಹುಲಕ_flush(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಂಕ len = G.x_ಬಹುಲಕ_buf.ಉದ್ದಳತೆ;
	ಆದರೆ (len <= 0)
		ಹಿಂತಿರುಗಿ;
	ಆದರೆ (G.x_ಬಹುಲಕ_fd > 0) {
		G.x_ಬಹುಲಕ_buf.ದತ್ತಾಂಶ[len] = '\n';
		ತುಂಬಿದ_write(G.x_ಬಹುಲಕ_fd, G.x_ಬಹುಲಕ_buf.ದತ್ತಾಂಶ, len + 1);
	}
	G.x_ಬಹುಲಕ_buf.ಉದ್ದಳತೆ = 0;
}
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */

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

#ಆದರೆ ENABLE_HUSH_BRACE_EXPANSION
# define MAYBE_BRACES "{}"
#ಇಲ್ಲದಿದ್ದರೆ
# define MAYBE_BRACES ""
#ಆದರೆ_ಅಂತ್ಯ

/* My analysis of quoting semantics tells me that state information
 * is associated with a destination, not a source.
 */
ಸ್ಥಿರ ಶೂನ್ಯ o_addqchr(o_string *o, ಪೂರ್ಣಾಂಕ ch)
{
	ಪೂರ್ಣಾಂಕ sz = 1;
	/* '-' is included because of this case:
	 * >filename0 >filename1 >filename9; v='-'; echo filename[0"$v"9]
	 */
	ಅಕ್ಷರ *ಸಿಕ್ಕಿತು = strchr("*?[-\\" MAYBE_BRACES, ch);
	ಆದರೆ (ಸಿಕ್ಕಿತು)
		sz++;
	o_grow_by(o, sz);
	ಆದರೆ (ಸಿಕ್ಕಿತು) {
		o->ದತ್ತಾಂಶ[o->ಉದ್ದಳತೆ] = '\\';
		o->ಉದ್ದಳತೆ++;
	}
	o->ದತ್ತಾಂಶ[o->ಉದ್ದಳತೆ] = ch;
	o->ಉದ್ದಳತೆ++;
	o->ದತ್ತಾಂಶ[o->ಉದ್ದಳತೆ] = '\0';
}

ಸ್ಥಿರ ಶೂನ್ಯ o_ಕೂಡಿಸು_Qchr(o_string *o, ಪೂರ್ಣಾಂಕ ch)
{
	ಪೂರ್ಣಾಂಕ sz = 1;
	ಆದರೆ ((o->o_expflags & EXP_FLAG_ESC_GLOB_CHARS)
	 && strchr("*?[-\\" MAYBE_BRACES, ch)
	) {
		sz++;
		o->ದತ್ತಾಂಶ[o->ಉದ್ದಳತೆ] = '\\';
		o->ಉದ್ದಳತೆ++;
	}
	o_grow_by(o, sz);
	o->ದತ್ತಾಂಶ[o->ಉದ್ದಳತೆ] = ch;
	o->ಉದ್ದಳತೆ++;
	o->ದತ್ತಾಂಶ[o->ಉದ್ದಳತೆ] = '\0';
}

ಸ್ಥಿರ ಶೂನ್ಯ o_addqblock(o_string *o, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str, ಪೂರ್ಣಾಂಕ len)
{
	ಆಗ (len) {
		ಅಕ್ಷರ ch;
		ಪೂರ್ಣಾಂಕ sz;
		ಪೂರ್ಣಾಂಕ ordinary_cnt = strcspn(str, "*?[-\\" MAYBE_BRACES);
		ಆದರೆ (ordinary_cnt > len) /* paranoia */
			ordinary_cnt = len;
		o_addblock(o, str, ordinary_cnt);
		ಆದರೆ (ordinary_cnt == len)
			ಹಿಂತಿರುಗಿ; /* NUL is already added by o_addblock */
		str += ordinary_cnt;
		len -= ordinary_cnt + 1; /* we are processing + 1 char below */

		ch = *str++;
		sz = 1;
		ಆದರೆ (ch) { /* it is necessarily one of "*?[-\\" MAYBE_BRACES */
			sz++;
			o->ದತ್ತಾಂಶ[o->ಉದ್ದಳತೆ] = '\\';
			o->ಉದ್ದಳತೆ++;
		}
		o_grow_by(o, sz);
		o->ದತ್ತಾಂಶ[o->ಉದ್ದಳತೆ] = ch;
		o->ಉದ್ದಳತೆ++;
	}
	o->ದತ್ತಾಂಶ[o->ಉದ್ದಳತೆ] = '\0';
}

ಸ್ಥಿರ ಶೂನ್ಯ o_ಕೂಡಿಸು_Qblock(o_string *o, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str, ಪೂರ್ಣಾಂಕ len)
{
	ಆದರೆ (!(o->o_expflags & EXP_FLAG_ESC_GLOB_CHARS)) {
		o_addblock(o, str, len);
		ಹಿಂತಿರುಗಿ;
	}
	o_addqblock(o, str, len);
}

ಸ್ಥಿರ ಶೂನ್ಯ o_ಕೂಡಿಸು_Qstr(o_string *o, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str)
{
	o_ಕೂಡಿಸು_Qblock(o, str, ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(str));
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
#ಆದರೆ DEBUG_EXPAND || DEBUG_GLOB
ಸ್ಥಿರ ಶೂನ್ಯ debug_print_list(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಪೂರ್ವಪ್ರತ್ಯಯ, o_string *o, ಪೂರ್ಣಾಂಕ n)
{
	ಅಕ್ಷರ **list = (ಅಕ್ಷರ**)o->ದತ್ತಾಂಶ;
	ಪೂರ್ಣಾಂಕ string_ಆರಂಭ = ((n + 0xf) & ~0xf) * ಗಾತ್ರ(list[0]);
	ಪೂರ್ಣಾಂಕ i = 0;

	indent();
	fdprintf(2, "%s: list:%p n:%d string_start:%d length:%d maxlen:%d glob:%d quoted:%d escape:%d\n",
			ಪೂರ್ವಪ್ರತ್ಯಯ, list, n, string_ಆರಂಭ, o->ಉದ್ದಳತೆ, o->maxlen,
			!!(o->o_expflags & EXP_FLAG_GLOB),
			o->has_quoted_part,
			!!(o->o_expflags & EXP_FLAG_ESC_GLOB_CHARS));
	ಆಗ (i < n) {
		indent();
		fdprintf(2, " list[%d]=%d '%s' %p\n", i, (ಪೂರ್ಣಾಂಕ)(uintptr_t)list[i],
				o->ದತ್ತಾಂಶ + (ಪೂರ್ಣಾಂಕ)(uintptr_t)list[i] + string_ಆರಂಭ,
				o->ದತ್ತಾಂಶ + (ಪೂರ್ಣಾಂಕ)(uintptr_t)list[i] + string_ಆರಂಭ);
		i++;
	}
	ಆದರೆ (n) {
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p = o->ದತ್ತಾಂಶ + (ಪೂರ್ಣಾಂಕ)(uintptr_t)list[n - 1] + string_ಆರಂಭ;
		indent();
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
		fdprintf(2, " total_sz:%ld\n", (ದೀರ್ಘ)((p + ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(p) + 1) - o->ದತ್ತಾಂಶ));
#ಇಲ್ಲದಿದ್ದರೆ /* __U_BOOT__ */
		ಬರೆ(" total_sz:%ld\n", (ದೀರ್ಘ)((p + ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(p) + 1) - o->ದತ್ತಾಂಶ));
#ಆದರೆ_ಅಂತ್ಯ /* __U_BOOT__ */
	}
}
#ಇಲ್ಲದಿದ್ದರೆ
# define debug_print_list(ಪೂರ್ವಪ್ರತ್ಯಯ, o, n) ((ಶೂನ್ಯ)0)
#ಆದರೆ_ಅಂತ್ಯ

/* n = o_save_ptr_helper(str, n) "starts new string" by storing an index value
 * in list[n] so that it points past last stored byte so far.
 * It returns n+1. */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ o_save_ptr_helper(o_string *o, ಪೂರ್ಣಾಂಕ n)
{
	ಅಕ್ಷರ **list = (ಅಕ್ಷರ**)o->ದತ್ತಾಂಶ;
	ಪೂರ್ಣಾಂಕ string_ಆರಂಭ;
	ಪೂರ್ಣಾಂಕ string_len;

	ಆದರೆ (!o->has_ಖಾಲಿಗಳು_ಸ್ಥಾನ) {
		string_ಆರಂಭ = ((n + 0xf) & ~0xf) * ಗಾತ್ರ(list[0]);
		string_len = o->ಉದ್ದಳತೆ - string_ಆರಂಭ;
		ಆದರೆ (!(n & 0xf)) { /* 0, 0x10, 0x20...? */
			debug_printf_list("list[%d]=%d string_start=%d (growing)\n", n, string_len, string_ಆರಂಭ);
			/* list[n] points to string_start, make space for 16 more pointers */
			o->maxlen += 0x10 * ಗಾತ್ರ(list[0]);
			o->ದತ್ತಾಂಶ = xrealloc(o->ದತ್ತಾಂಶ, o->maxlen + 1);
			list = (ಅಕ್ಷರ**)o->ದತ್ತಾಂಶ;
			memmove(list + n + 0x10, list + n, string_len);
			/*
			 * expand_on_ifs() has a "previous argv[] ends in IFS?"
			 * check. (grep for -prev-ifs-check-).
			 * Ensure that argv[-1][last] is not garbage
			 * but zero bytes, to save index check there.
			 */
			list[n + 0x10 - 1] = 0;
			o->ಉದ್ದಳತೆ += 0x10 * ಗಾತ್ರ(list[0]);
		} ಇಲ್ಲದಿದ್ದರೆ {
			debug_printf_list("list[%d]=%d string_start=%d\n",
					n, string_len, string_ಆರಂಭ);
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* We have empty slot at list[n], reuse without growth */
		string_ಆರಂಭ = ((n+1 + 0xf) & ~0xf) * ಗಾತ್ರ(list[0]); /* NB: n+1! */
		string_len = o->ಉದ್ದಳತೆ - string_ಆರಂಭ;
		debug_printf_list("list[%d]=%d string_start=%d (empty slot)\n",
				n, string_len, string_ಆರಂಭ);
		o->has_ಖಾಲಿಗಳು_ಸ್ಥಾನ = 0;
	}
	o->has_quoted_part = 0;
	list[n] = (ಅಕ್ಷರ*)(uintptr_t)string_len;
	ಹಿಂತಿರುಗಿ n + 1;
}

/* "What was our last o_save_ptr'ed position (byte offset relative o->data)?" */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ o_get_ಕೊನೆಯ_ptr(o_string *o, ಪೂರ್ಣಾಂಕ n)
{
	ಅಕ್ಷರ **list = (ಅಕ್ಷರ**)o->ದತ್ತಾಂಶ;
	ಪೂರ್ಣಾಂಕ string_ಆರಂಭ = ((n + 0xf) & ~0xf) * ಗಾತ್ರ(list[0]);

	ಹಿಂತಿರುಗಿ ((ಪೂರ್ಣಾಂಕ)(uintptr_t)list[n-1]) + string_ಆರಂಭ;
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

#ಆದರೆ ENABLE_HUSH_BRACE_EXPANSION
/* There in a GNU extension, GLOB_BRACE, but it is not usable:
 * first, it processes even {a} (no commas), second,
 * I didn't manage to make it return strings when they don't match
 * existing files. Need to re-implement it.
 */

/* Helper */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ glob_needed(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s)
{
	ಆಗ (*s) {
		ಆದರೆ (*s == '\\') {
			ಆದರೆ (!s[1])
				ಹಿಂತಿರುಗಿ 0;
			s += 2;
			ಮುಂದುವರಿ;
		}
		ಆದರೆ (*s == '*' || *s == '[' || *s == '?' || *s == '{')
			ಹಿಂತಿರುಗಿ 1;
		s++;
	}
	ಹಿಂತಿರುಗಿ 0;
}
/* Return pointer to next closing brace or to comma */
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮುಂದಿನ_brace_sub(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *cp)
{
	ಚಿಹ್ನೆರಹಿತ ಆಳ = 0;
	cp++;
	ಆಗ (*cp != '\0') {
		ಆದರೆ (*cp == '\\') {
			ಆದರೆ (*++cp == '\0')
				ನಿಲ್ಲಿಸು;
			cp++;
			ಮುಂದುವರಿ;
		}
		ಆದರೆ ((*cp == '}' && ಆಳ-- == 0) || (*cp == ',' && ಆಳ == 0))
			ನಿಲ್ಲಿಸು;
		ಆದರೆ (*cp++ == '{')
			ಆಳ++;
	}

	ಹಿಂತಿರುಗಿ *cp != '\0' ? cp : ಖಾಲಿ;
}
/* Recursive brace globber. Note: may garble pattern[]. */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ glob_brace(ಅಕ್ಷರ *ನಮೂನೆ, o_string *o, ಪೂರ್ಣಾಂಕ n)
{
	ಅಕ್ಷರ *new_ನಮೂನೆ_buf;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *begin;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮುಂದಿನ;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *rest;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p;
	size_t rest_len;

	debug_printf_glob("glob_brace('%s')\n", ನಮೂನೆ);

	begin = ನಮೂನೆ;
	ಆಗ (1) {
		ಆದರೆ (*begin == '\0')
			ಹೋಗು simple_glob;
		ಆದರೆ (*begin == '{') {
			/* Find the first sub-pattern and at the same time
			 * find the rest after the closing brace */
			ಮುಂದಿನ = ಮುಂದಿನ_brace_sub(begin);
			ಆದರೆ (ಮುಂದಿನ == ಖಾಲಿ) {
				/* An illegal expression */
				ಹೋಗು simple_glob;
			}
			ಆದರೆ (*ಮುಂದಿನ == '}') {
				/* "{abc}" with no commas - illegal
				 * brace expr, disregard and skip it */
				begin = ಮುಂದಿನ + 1;
				ಮುಂದುವರಿ;
			}
			ನಿಲ್ಲಿಸು;
		}
		ಆದರೆ (*begin == '\\' && begin[1] != '\0')
			begin++;
		begin++;
	}
	debug_printf_glob("begin:%s\n", begin);
	debug_printf_glob("next:%s\n", ಮುಂದಿನ);

	/* Now find the end of the whole brace expression */
	rest = ಮುಂದಿನ;
	ಆಗ (*rest != '}') {
		rest = ಮುಂದಿನ_brace_sub(rest);
		ಆದರೆ (rest == ಖಾಲಿ) {
			/* An illegal expression */
			ಹೋಗು simple_glob;
		}
		debug_printf_glob("rest:%s\n", rest);
	}
	rest_len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(++rest) + 1;

	/* We are sure the brace expression is well-formed */

	/* Allocate working buffer large enough for our work */
	new_ನಮೂನೆ_buf = xmalloc(ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ನಮೂನೆ));

	/* We have a brace expression.  BEGIN points to the opening {,
	 * NEXT points past the terminator of the first element, and REST
	 * points past the final }.  We will accumulate result names from
	 * recursive runs for each brace alternative in the buffer using
	 * GLOB_APPEND. */

	p = begin + 1;
	ಆಗ (1) {
		/* Construct the new glob expression */
		ಸ್ಮರಣೆ_ನಕಲು(
			mempcpy(
				mempcpy(new_ನಮೂನೆ_buf,
					/* We know the prefix for all sub-patterns */
					ನಮೂನೆ, begin - ನಮೂನೆ),
				p, ಮುಂದಿನ - p),
			rest, rest_len);

		/* Note: glob_brace() may garble new_pattern_buf[].
		 * That's why we re-copy prefix every time (1st memcpy above).
		 */
		n = glob_brace(new_ನಮೂನೆ_buf, o, n);
		ಆದರೆ (*ಮುಂದಿನ == '}') {
			/* We saw the last entry */
			ನಿಲ್ಲಿಸು;
		}
		p = ಮುಂದಿನ + 1;
		ಮುಂದಿನ = ಮುಂದಿನ_brace_sub(ಮುಂದಿನ);
	}
	ಬಿಡುಗಡೆ(new_ನಮೂನೆ_buf);
	ಹಿಂತಿರುಗಿ n;

 simple_glob:
	{
		ಪೂರ್ಣಾಂಕ gr;
		glob_t globdata;

		memset(&globdata, 0, ಗಾತ್ರ(globdata));
		gr = glob(ನಮೂನೆ, 0, ಖಾಲಿ, &globdata);
		debug_printf_glob("glob('%s'):%d\n", ನಮೂನೆ, gr);
		ಆದರೆ (gr != 0) {
			ಆದರೆ (gr == GLOB_NOMATCH) {
				globfree(&globdata);
				/* NB: garbles parameter */
				unbackslash(ನಮೂನೆ);
				o_addstr_with_NUL(o, ನಮೂನೆ);
				debug_printf_glob("glob pattern '%s' is literal\n", ನಮೂನೆ);
				ಹಿಂತಿರುಗಿ o_save_ptr_helper(o, n);
			}
			ಆದರೆ (gr == GLOB_NOSPACE)
				bb_die_ಸ್ಮರಣೆ_exhausted();
			/* GLOB_ABORTED? Only happens with GLOB_ERR flag,
			 * but we didn't specify it. Paranoia again. */
			bb_error_ಸಂದೇಶ_and_die("glob error %d on '%s'", gr, ನಮೂನೆ);
		}
		ಆದರೆ (globdata.gl_pathv && globdata.gl_pathv[0]) {
			ಅಕ್ಷರ **argv = globdata.gl_pathv;
			ಆಗ (1) {
				o_addstr_with_NUL(o, *argv);
				n = o_save_ptr_helper(o, n);
				argv++;
				ಆದರೆ (!*argv)
					ನಿಲ್ಲಿಸು;
			}
		}
		globfree(&globdata);
	}
	ಹಿಂತಿರುಗಿ n;
}
/* Performs globbing on last list[],
 * saving each result as a new list[].
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ perform_glob(o_string *o, ಪೂರ್ಣಾಂಕ n)
{
	ಅಕ್ಷರ *ನಮೂನೆ, *copy;

	debug_printf_glob("start perform_glob: n:%d o->data:%p\n", n, o->ದತ್ತಾಂಶ);
	ಆದರೆ (!o->ದತ್ತಾಂಶ)
		ಹಿಂತಿರುಗಿ o_save_ptr_helper(o, n);
	ನಮೂನೆ = o->ದತ್ತಾಂಶ + o_get_ಕೊನೆಯ_ptr(o, n);
	debug_printf_glob("glob pattern '%s'\n", ನಮೂನೆ);
	ಆದರೆ (!glob_needed(ನಮೂನೆ)) {
		/* unbackslash last string in o in place, fix length */
		o->ಉದ್ದಳತೆ = unbackslash(ನಮೂನೆ) - o->ದತ್ತಾಂಶ;
		debug_printf_glob("glob pattern '%s' is literal\n", ನಮೂನೆ);
		ಹಿಂತಿರುಗಿ o_save_ptr_helper(o, n);
	}

	copy = xstrdup(ನಮೂನೆ);
	/* "forget" pattern in o */
	o->ಉದ್ದಳತೆ = ನಮೂನೆ - o->ದತ್ತಾಂಶ;
	n = glob_brace(copy, o, n);
	ಬಿಡುಗಡೆ(copy);
	ಆದರೆ (DEBUG_GLOB)
		debug_print_list("perform_glob returning", o, n);
	ಹಿಂತಿರುಗಿ n;
}

#ಇಲ್ಲದಿದ್ದರೆ /* !HUSH_BRACE_EXPANSION */

/* Helper */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ glob_needed(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s)
{
	ಆಗ (*s) {
		ಆದರೆ (*s == '\\') {
			ಆದರೆ (!s[1])
				ಹಿಂತಿರುಗಿ 0;
			s += 2;
			ಮುಂದುವರಿ;
		}
		ಆದರೆ (*s == '*' || *s == '[' || *s == '?')
			ಹಿಂತಿರುಗಿ 1;
		s++;
	}
	ಹಿಂತಿರುಗಿ 0;
}
/* Performs globbing on last list[],
 * saving each result as a new list[].
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ perform_glob(o_string *o, ಪೂರ್ಣಾಂಕ n)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	glob_t globdata;
	ಪೂರ್ಣಾಂಕ gr;
#ಆದರೆ_ಅಂತ್ಯ /* __U_BOOT__ */
	ಅಕ್ಷರ *ನಮೂನೆ;

	debug_printf_glob("start perform_glob: n:%d o->data:%p\n", n, o->ದತ್ತಾಂಶ);
	ಆದರೆ (!o->ದತ್ತಾಂಶ)
		ಹಿಂತಿರುಗಿ o_save_ptr_helper(o, n);
	ನಮೂನೆ = o->ದತ್ತಾಂಶ + o_get_ಕೊನೆಯ_ptr(o, n);
	debug_printf_glob("glob pattern '%s'\n", ನಮೂನೆ);
	ಆದರೆ (!glob_needed(ನಮೂನೆ)) {
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
 literal:
#ಆದರೆ_ಅಂತ್ಯ /* __U_BOOT__ */
		/* unbackslash last string in o in place, fix length */
		o->ಉದ್ದಳತೆ = unbackslash(ನಮೂನೆ) - o->ದತ್ತಾಂಶ;
		debug_printf_glob("glob pattern '%s' is literal\n", ನಮೂನೆ);
		ಹಿಂತಿರುಗಿ o_save_ptr_helper(o, n);
	}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	memset(&globdata, 0, ಗಾತ್ರ(globdata));
	/* Can't use GLOB_NOCHECK: it does not unescape the string.
	 * If we glob "*.\*" and don't find anything, we need
	 * to fall back to using literal "*.*", but GLOB_NOCHECK
	 * will return "*.\*"!
	 */
	gr = glob(ನಮೂನೆ, 0, ಖಾಲಿ, &globdata);
	debug_printf_glob("glob('%s'):%d\n", ನಮೂನೆ, gr);
	ಆದರೆ (gr != 0) {
		ಆದರೆ (gr == GLOB_NOMATCH) {
			globfree(&globdata);
			ಹೋಗು literal;
		}
		ಆದರೆ (gr == GLOB_NOSPACE)
			bb_die_ಸ್ಮರಣೆ_exhausted();
		/* GLOB_ABORTED? Only happens with GLOB_ERR flag,
		 * but we didn't specify it. Paranoia again. */
		bb_error_ಸಂದೇಶ_and_die("glob error %d on '%s'", gr, ನಮೂನೆ);
	}
	ಆದರೆ (globdata.gl_pathv && globdata.gl_pathv[0]) {
		ಅಕ್ಷರ **argv = globdata.gl_pathv;
		/* "forget" pattern in o */
		o->ಉದ್ದಳತೆ = ನಮೂನೆ - o->ದತ್ತಾಂಶ;
		ಆಗ (1) {
			o_addstr_with_NUL(o, *argv);
			n = o_save_ptr_helper(o, n);
			argv++;
			ಆದರೆ (!*argv)
				ನಿಲ್ಲಿಸು;
		}
	}
	globfree(&globdata);
	ಆದರೆ (DEBUG_GLOB)
		debug_print_list("perform_glob returning", o, n);
	ಹಿಂತಿರುಗಿ n;
#ಇಲ್ಲದಿದ್ದರೆ /* __U_BOOT__ */
	/*
	 * NOTE We only use perform glob to call unbackslash to remove backslash
	 * from string once expanded.
	 * So, it seems OK to return this if no previous return was done.
	 */
	ಹಿಂತಿರುಗಿ o_save_ptr_helper(o, n);
#ಆದರೆ_ಅಂತ್ಯ /* __U_BOOT__ */
}

#ಆದರೆ_ಅಂತ್ಯ /* !HUSH_BRACE_EXPANSION */

/* If o->o_expflags & EXP_FLAG_GLOB, glob the string so far remembered.
 * Otherwise, just finish current list[] and start new */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ o_save_ptr(o_string *o, ಪೂರ್ಣಾಂಕ n)
{
	ಆದರೆ (o->o_expflags & EXP_FLAG_GLOB) {
		/* If o->has_empty_slot, list[n] was already globbed
		 * (if it was requested back then when it was filled)
		 * so don't do that again! */
		ಆದರೆ (!o->has_ಖಾಲಿಗಳು_ಸ್ಥಾನ)
			ಹಿಂತಿರುಗಿ perform_glob(o, n); /* o_save_ptr_helper is inside */
	}
	ಹಿಂತಿರುಗಿ o_save_ptr_helper(o, n);
}

/* "Please convert list[n] to real char* ptrs, and NULL terminate it." */
ಸ್ಥಿರ ಅಕ್ಷರ **o_finalize_list(o_string *o, ಪೂರ್ಣಾಂಕ n)
{
	ಅಕ್ಷರ **list;
	ಪೂರ್ಣಾಂಕ string_ಆರಂಭ;

	ಆದರೆ (DEBUG_EXPAND)
		debug_print_list("finalized", o, n);
	debug_printf_expand("finalized n:%d\n", n);
	list = (ಅಕ್ಷರ**)o->ದತ್ತಾಂಶ;
	string_ಆರಂಭ = ((n + 0xf) & ~0xf) * ಗಾತ್ರ(list[0]);
	list[--n] = ಖಾಲಿ;
	ಆಗ (n) {
		n--;
		list[n] = o->ದತ್ತಾಂಶ + (ಪೂರ್ಣಾಂಕ)(uintptr_t)list[n] + string_ಆರಂಭ;
	}
	ಹಿಂತಿರುಗಿ list;
}

ಸ್ಥಿರ ಶೂನ್ಯ free_pipe_list(ರಚನೆ pipe *pi);

/* Returns pi->next - next pipe in the list */
ಸ್ಥಿರ ರಚನೆ pipe *free_pipe(ರಚನೆ pipe *pi)
{
	ರಚನೆ pipe *ಮುಂದಿನ;
	ಪೂರ್ಣಾಂಕ i;

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	debug_printf_clean("free_pipe (pid %d)\n", getpid());
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */
	ಗಾಗಿ (i = 0; i < pi->ಅಂಕೆ_cmds; i++) {
		ರಚನೆ command *command;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
		ರಚನೆ redir_struct *r, *rnext;
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */

		command = &pi->cmds[i];
		debug_printf_clean("  command %d:\n", i);
		ಆದರೆ (command->argv) {
			ಆದರೆ (DEBUG_CLEAN) {
				ಪೂರ್ಣಾಂಕ a;
				ಅಕ್ಷರ **p;
				ಗಾಗಿ (a = 0, p = command->argv; *p; a++, p++) {
					debug_printf_clean("   argv[%d] = %s\n", a, *p);
				}
			}
			free_ಅಕ್ಷರಸರಣಿಗಳು(command->argv);
			//command->argv = NULL;
		}
		/* not "else if": on syntax error, we may have both! */
		ಆದರೆ (command->ಗುಂಪು) {
			debug_printf_clean("   begin group (cmd_type:%d)\n",
					command->cmd_type);
			free_pipe_list(command->ಗುಂಪು);
			debug_printf_clean("   end group\n");
			//command->group = NULL;
		}
		/* else is crucial here.
		 * If group != NULL, child_func is meaningless */
#ಆದರೆ ENABLE_HUSH_FUNCTIONS
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (command->ಮಗು_func) {
			debug_printf_exec("cmd %p releases child func at %p\n", command, command->ಮಗು_func);
			command->ಮಗು_func->ಪೋಷಕ_cmd = ಖಾಲಿ;
		}
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ !BB_MMU
		ಬಿಡುಗಡೆ(command->ಗುಂಪು_as_string);
		//command->group_as_string = NULL;
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
		ಗಾಗಿ (r = command->redirects; r; r = rnext) {
			debug_printf_clean("   redirect %d%s",
					r->rd_fd, redir_ಕೋಷ್ಟಕ[r->rd_type].descrip);
			/* guard against the case >$FOO, where foo is unset or blank */
			ಆದರೆ (r->rd_filename) {
				debug_printf_clean(" fname:'%s'\n", r->rd_filename);
				ಬಿಡುಗಡೆ(r->rd_filename);
				//r->rd_filename = NULL;
			}
			debug_printf_clean(" rd_dup:%d\n", r->rd_dup);
			rnext = r->ಮುಂದಿನ;
			ಬಿಡುಗಡೆ(r);
		}
		//command->redirects = NULL;
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */
	}
	ಬಿಡುಗಡೆ(pi->cmds);   /* children are an array, they get freed all at once */
	//pi->cmds = NULL;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
#ಆದರೆ ENABLE_HUSH_ಕೆಲಸ
	ಬಿಡುಗಡೆ(pi->cmdtext);
	//pi->cmdtext = NULL;
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */

	ಮುಂದಿನ = pi->ಮುಂದಿನ;
	ಬಿಡುಗಡೆ(pi);
	ಹಿಂತಿರುಗಿ ಮುಂದಿನ;
}

ಸ್ಥಿರ ಶೂನ್ಯ free_pipe_list(ರಚನೆ pipe *pi)
{
	ಆಗ (pi) {
#ಆದರೆ HAS_KEYWORDS
		debug_printf_clean("pipe reserved word %d\n", pi->res_ಶಬ್ದ);
#ಆದರೆ_ಅಂತ್ಯ
		debug_printf_clean("pipe followup code %d\n", pi->followup);
		pi = free_pipe(pi);
	}
}

/*** Parsing routines ***/

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ debug_print_ವೃಕ್ಷ
ಸ್ಥಿರ ಶೂನ್ಯ debug_print_ವೃಕ್ಷ(ರಚನೆ pipe *pi, ಪೂರ್ಣಾಂಕ lvl)
{
	ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ PIPE[] ALIGN_PTR = {
		[PIPE_SEQ] = "SEQ",
		[PIPE_AND] = "AND",
		[PIPE_OR ] = "OR" ,
		[PIPE_BG ] = "BG" ,
	};
	ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *RES[] = {
		[RES_NONE ] = "NONE" ,
# ಆದರೆ ENABLE_HUSH_IF
		[RES_IF   ] = "IF"   ,
		[RES_THEN ] = "THEN" ,
		[RES_ELIF ] = "ELIF" ,
		[RES_ELSE ] = "ELSE" ,
		[RES_FI   ] = "FI"   ,
# endif
# ಆದರೆ ENABLE_HUSH_LOOPS
		[RES_FOR  ] = "FOR"  ,
		[RES_WHILE] = "WHILE",
		[RES_UNTIL] = "UNTIL",
		[RES_DO   ] = "DO"   ,
		[RES_ಮುಗಿದ ] = "DONE" ,
# endif
# ಆದರೆ ENABLE_HUSH_LOOPS || ENABLE_HUSH_CASE
		[RES_ಒಳ   ] = "IN"   ,
# endif
# ಆದರೆ ENABLE_HUSH_CASE
		[RES_CASE ] = "CASE" ,
		[RES_CASE_ಒಳ ] = "CASE_IN" ,
		[RES_ಹೊಂದಿಕೆ] = "MATCH",
		[RES_CASE_BODY] = "CASE_BODY",
		[RES_ESAC ] = "ESAC" ,
# endif
		[RES_XXXX ] = "XXXX" ,
		[RES_SNTX ] = "SNTX" ,
	};
	ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ CMDTYPE[] ALIGN_PTR = {
		"{}",
		"()",
		"[noglob]",
# ಆದರೆ ENABLE_HUSH_FUNCTIONS
		"func()",
# endif
	};

	ಪೂರ್ಣಾಂಕ pin, prn;

	pin = 0;
	ಆಗ (pi) {
		fdprintf(2, "%*spipe %d #cmds:%d %sres_word=%s followup=%d %s\n",
				lvl*2, "",
				pin,
				pi->ಅಂಕೆ_cmds,
				(IF_HAS_KEYWORDS(pi->pi_inverted ? "! " :) ""),
				RES[pi->res_ಶಬ್ದ],
				pi->followup, PIPE[pi->followup]
		);
		prn = 0;
		ಆಗ (prn < pi->ಅಂಕೆ_cmds) {
			ರಚನೆ command *command = &pi->cmds[prn];
			ಅಕ್ಷರ **argv = command->argv;

			fdprintf(2, "%*s cmd %d assignment_cnt:%d",
					lvl*2, "", prn,
					command->assignment_cnt);
# ಆದರೆ ENABLE_HUSH_LINENO_VAR
			fdprintf(2, " LINENO:%u", command->lineno);
# endif
			ಆದರೆ (command->ಗುಂಪು) {
				fdprintf(2, " group %s: (argv=%p)%s%s\n",
						CMDTYPE[command->cmd_type],
						argv
# ಆದರೆ !BB_MMU
						, " group_as_string:", command->ಗುಂಪು_as_string
# ಇಲ್ಲದಿದ್ದರೆ
						, "", ""
# endif
				);
				debug_print_ವೃಕ್ಷ(command->ಗುಂಪು, lvl+1);
				prn++;
				ಮುಂದುವರಿ;
			}
			ಆದರೆ (argv) ಆಗ (*argv) {
				fdprintf(2, " '%s'", *argv);
				argv++;
			}
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
			ಆದರೆ (command->redirects)
				fdprintf(2, " {redir}");
#ಆದರೆ_ಅಂತ್ಯ /* __U_BOOT__ */
			fdprintf(2, "\n");
			prn++;
		}
		pi = pi->ಮುಂದಿನ;
		pin++;
	}
}
#ಆದರೆ_ಅಂತ್ಯ /* debug_print_tree */

ಸ್ಥಿರ ರಚನೆ pipe *new_pipe(ಶೂನ್ಯ)
{
	ರಚನೆ pipe *pi;
	pi = xzalloc(ಗಾತ್ರ(ರಚನೆ pipe));
	/*pi->res_word = RES_NONE; - RES_NONE is 0 anyway */
	ಹಿಂತಿರುಗಿ pi;
}

/* Command (member of a pipe) is complete, or we start a new pipe
 * if ctx->command is NULL.
 * No errors possible here.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಮುಗಿದ_command(ರಚನೆ parse_context *ctx)
{
	/* The command is really already in the pipe structure, so
	 * advance the pipe counter and make a new, null command. */
	ರಚನೆ pipe *pi = ctx->pipe;
	ರಚನೆ command *command = ctx->command;

#ಆದರೆ 0	/* Instead we emit error message at run time */
	ಆದರೆ (ctx->pending_redirect) {
		/* For example, "cmd >" (no filename to redirect to) */
		syntax_error("invalid redirect");
		ctx->pending_redirect = ಖಾಲಿ;
	}
#ಆದರೆ_ಅಂತ್ಯ

	ಆದರೆ (command) {
		ಆದರೆ (IS_NULL_CMD(command)) {
			debug_printf_parse("done_command: skipping null cmd, num_cmds=%d\n", pi->ಅಂಕೆ_cmds);
			ಹೋಗು clear_and_ret;
		}
		pi->ಅಂಕೆ_cmds++;
		debug_printf_parse("done_command: ++num_cmds=%d\n", pi->ಅಂಕೆ_cmds);
		//debug_print_tree(ctx->list_head, 20);
	} ಇಲ್ಲದಿದ್ದರೆ {
		debug_printf_parse("done_command: initializing, num_cmds=%d\n", pi->ಅಂಕೆ_cmds);
	}

	/* Only real trickiness here is that the uncommitted
	 * command structure is not counted in pi->num_cmds. */
	pi->cmds = xrealloc(pi->cmds, ಗಾತ್ರ(*pi->cmds) * (pi->ಅಂಕೆ_cmds+1));
	ctx->command = command = &pi->cmds[pi->ಅಂಕೆ_cmds];
 clear_and_ret:
	memset(command, 0, ಗಾತ್ರ(*command));
#ಆದರೆ ENABLE_HUSH_LINENO_VAR
	command->lineno = G.parse_lineno;
	debug_printf_parse("command->lineno = G.parse_lineno (%u)\n", G.parse_lineno);
#ಆದರೆ_ಅಂತ್ಯ
	ಹಿಂತಿರುಗಿ pi->ಅಂಕೆ_cmds; /* used only for 0/nonzero check */
}

ಸ್ಥಿರ ಶೂನ್ಯ ಮುಗಿದ_pipe(ರಚನೆ parse_context *ctx, pipe_style type)
{
	ಪೂರ್ಣಾಂಕ not_null;

	debug_printf_parse("done_pipe entered, followup %d\n", type);
	/* Close previous command */
	not_null = ಮುಗಿದ_command(ctx);
#ಆದರೆ HAS_KEYWORDS
	ctx->pipe->pi_inverted = ctx->ctx_inverted;
	ctx->ctx_inverted = 0;
	ctx->pipe->res_ಶಬ್ದ = ctx->ctx_res_w;
#ಆದರೆ_ಅಂತ್ಯ
	ಆದರೆ (type == PIPE_BG && ctx->list_ತಲೆ != ctx->pipe) {
		/* Necessary since && and || have precedence over &:
		 * "cmd1 && cmd2 &" must spawn both cmds, not only cmd2,
		 * in a backgrounded subshell.
		 */
		ರಚನೆ pipe *pi;
		ರಚನೆ command *command;

		/* Is this actually this construct, all pipes end with && or ||? */
		pi = ctx->list_ತಲೆ;
		ಆಗ (pi != ctx->pipe) {
			ಆದರೆ (pi->followup != PIPE_AND && pi->followup != PIPE_OR)
				ಹೋಗು no_conv;
			pi = pi->ಮುಂದಿನ;
		}

		debug_printf_parse("BG with more than one pipe, converting to { p1 &&...pN; } &\n");
		pi->followup = PIPE_SEQ; /* close pN _not_ with "&"! */
		pi = xzalloc(ಗಾತ್ರ(*pi));
		pi->followup = PIPE_BG;
		pi->ಅಂಕೆ_cmds = 1;
		pi->cmds = xzalloc(ಗಾತ್ರ(pi->cmds[0]));
		command = &pi->cmds[0];
		ಆದರೆ (CMD_ಸಾಮಾನ್ಯ != 0) /* "if xzalloc didn't do that already" */
			command->cmd_type = CMD_ಸಾಮಾನ್ಯ;
		command->ಗುಂಪು = ctx->list_ತಲೆ;
#ಆದರೆ !BB_MMU
		command->ಗುಂಪು_as_string = xstrndup(
			    ctx->as_string.ದತ್ತಾಂಶ,
			    ctx->as_string.ಉದ್ದಳತೆ - 1 /* do not copy last char, "&" */
		);
#ಆದರೆ_ಅಂತ್ಯ
		/* Replace all pipes in ctx with one newly created */
		ctx->list_ತಲೆ = ctx->pipe = pi;
		/* for cases like "cmd && &", do not be tricked by last command
		 * being null - the entire {...} & is NOT null! */
		not_null = 1;
	} ಇಲ್ಲದಿದ್ದರೆ {
 no_conv:
		ctx->pipe->followup = type;
	}

	/* Without this check, even just <enter> on command line generates
	 * tree of three NOPs (!). Which is harmless but annoying.
	 * IOW: it is safe to do it unconditionally. */
	ಆದರೆ (not_null
#ಆದರೆ ENABLE_HUSH_IF
	 || ctx->ctx_res_w == RES_FI
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_HUSH_LOOPS
	 || ctx->ctx_res_w == RES_ಮುಗಿದ
	 || ctx->ctx_res_w == RES_FOR
	 || ctx->ctx_res_w == RES_ಒಳ
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_HUSH_CASE
	 || ctx->ctx_res_w == RES_ESAC
#ಆದರೆ_ಅಂತ್ಯ
	) {
		ರಚನೆ pipe *new_p;
		debug_printf_parse("done_pipe: adding new pipe: "
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
				"not_null:%d ctx->ctx_res_w:%d\n",
				not_null, ctx->ctx_res_w);
#ಇಲ್ಲದಿದ್ದರೆ /* __U_BOOT__ */
				"not_null:%d\n",
				not_null);
#ಆದರೆ_ಅಂತ್ಯ /* __U_BOOT__ */
		new_p = new_pipe();
		ctx->pipe->ಮುಂದಿನ = new_p;
		ctx->pipe = new_p;
		/* RES_THEN, RES_DO etc are "sticky" -
		 * they remain set for pipes inside if/while.
		 * This is used to control execution.
		 * RES_FOR and RES_IN are NOT sticky (needed to support
		 * cases where variable or value happens to match a keyword):
		 */
#ಆದರೆ ENABLE_HUSH_LOOPS
		ಆದರೆ (ctx->ctx_res_w == RES_FOR
		 || ctx->ctx_res_w == RES_ಒಳ)
			ctx->ctx_res_w = RES_NONE;
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_HUSH_CASE
		ಆದರೆ (ctx->ctx_res_w == RES_ಹೊಂದಿಕೆ)
			ctx->ctx_res_w = RES_CASE_BODY;
		ಆದರೆ (ctx->ctx_res_w == RES_CASE)
			ctx->ctx_res_w = RES_CASE_ಒಳ;
#ಆದರೆ_ಅಂತ್ಯ
		ctx->command = ಖಾಲಿ; /* trick done_command below */
		/* Create the memory for command, roughly:
		 * ctx->pipe->cmds = new struct command;
		 * ctx->command = &ctx->pipe->cmds[0];
		 */
		ಮುಗಿದ_command(ctx);
		//debug_print_tree(ctx->list_head, 10);
	}
	debug_printf_parse("done_pipe return\n");
}

ಸ್ಥಿರ ಶೂನ್ಯ initialize_context(ರಚನೆ parse_context *ctx)
{
	memset(ctx, 0, ಗಾತ್ರ(*ctx));
	ಆದರೆ (MAYBE_ASSIGNMENT != 0)
		ctx->is_assignment = MAYBE_ASSIGNMENT;
	ctx->pipe = ctx->list_ತಲೆ = new_pipe();
	/* Create the memory for command, roughly:
	 * ctx->pipe->cmds = new struct command;
	 * ctx->command = &ctx->pipe->cmds[0];
	 */
	ಮುಗಿದ_command(ctx);
}

/* If a reserved word is found and processed, parse context is modified
 * and 1 is returned.
 */
#ಆದರೆ HAS_KEYWORDS
ರಚನೆ reserved_combo {
	ಅಕ್ಷರ literal[6];
	ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ res;
	ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ assignment_flag;
	uint32_t flag;
};
ಎಣಿಕೆ {
	FLAG_ಕೊನೆ   = (1 << RES_NONE ),
# ಆದರೆ ENABLE_HUSH_IF
	FLAG_IF    = (1 << RES_IF   ),
	FLAG_THEN  = (1 << RES_THEN ),
	FLAG_ELIF  = (1 << RES_ELIF ),
	FLAG_ELSE  = (1 << RES_ELSE ),
	FLAG_FI    = (1 << RES_FI   ),
# endif
# ಆದರೆ ENABLE_HUSH_LOOPS
	FLAG_FOR   = (1 << RES_FOR  ),
	FLAG_WHILE = (1 << RES_WHILE),
	FLAG_UNTIL = (1 << RES_UNTIL),
	FLAG_DO    = (1 << RES_DO   ),
	FLAG_ಮುಗಿದ  = (1 << RES_ಮುಗಿದ ),
	FLAG_ಒಳ    = (1 << RES_ಒಳ   ),
# endif
# ಆದರೆ ENABLE_HUSH_CASE
	FLAG_ಹೊಂದಿಕೆ = (1 << RES_ಹೊಂದಿಕೆ),
	FLAG_ESAC  = (1 << RES_ESAC ),
# endif
	FLAG_ಆರಂಭ = (1 << RES_XXXX ),
};

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ reserved_combo* ಹೊಂದಿಕೆ_reserved_ಶಬ್ದ(o_string *ಶಬ್ದ)
{
	/* Mostly a list of accepted follow-up reserved words.
	 * FLAG_END means we are done with the sequence, and are ready
	 * to turn the compound list into a command.
	 * FLAG_START means the word must start a new compound list.
	 */
	ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ reserved_combo reserved_list[] ALIGN4 = {
# ಆದರೆ ENABLE_HUSH_IF
		{ "!",     RES_NONE,  NOT_ASSIGNMENT  , 0 },
		{ "if",    RES_IF,    MAYBE_ASSIGNMENT, FLAG_THEN | FLAG_ಆರಂಭ },
		{ "then",  RES_THEN,  MAYBE_ASSIGNMENT, FLAG_ELIF | FLAG_ELSE | FLAG_FI },
		{ "elif",  RES_ELIF,  MAYBE_ASSIGNMENT, FLAG_THEN },
		{ "else",  RES_ELSE,  MAYBE_ASSIGNMENT, FLAG_FI   },
		{ "fi",    RES_FI,    NOT_ASSIGNMENT  , FLAG_ಕೊನೆ  },
# endif
# ಆದರೆ ENABLE_HUSH_LOOPS
		{ "for",   RES_FOR,   NOT_ASSIGNMENT  , FLAG_ಒಳ | FLAG_DO | FLAG_ಆರಂಭ },
		{ "while", RES_WHILE, MAYBE_ASSIGNMENT, FLAG_DO | FLAG_ಆರಂಭ },
		{ "until", RES_UNTIL, MAYBE_ASSIGNMENT, FLAG_DO | FLAG_ಆರಂಭ },
		{ "in",    RES_ಒಳ,    NOT_ASSIGNMENT  , FLAG_DO   },
		{ "do",    RES_DO,    MAYBE_ASSIGNMENT, FLAG_ಮುಗಿದ },
		{ "done",  RES_ಮುಗಿದ,  NOT_ASSIGNMENT  , FLAG_ಕೊನೆ  },
# endif
# ಆದರೆ ENABLE_HUSH_CASE
		{ "case",  RES_CASE,  NOT_ASSIGNMENT  , FLAG_ಹೊಂದಿಕೆ | FLAG_ಆರಂಭ },
		{ "esac",  RES_ESAC,  NOT_ASSIGNMENT  , FLAG_ಕೊನೆ  },
# endif
	};
	ಸ್ಥಿರಾಂಕ ರಚನೆ reserved_combo *r;

	ಗಾಗಿ (r = reserved_list; r < reserved_list + ARRAY_ಗಾತ್ರ(reserved_list); r++) {
		ಆದರೆ (strcmp(ಶಬ್ದ->ದತ್ತಾಂಶ, r->literal) == 0)
			ಹಿಂತಿರುಗಿ r;
	}
	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}
/* Return NULL: not a keyword, else: keyword
 */
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ reserved_combo* reserved_ಶಬ್ದ(ರಚನೆ parse_context *ctx)
{
# ಆದರೆ ENABLE_HUSH_CASE
	ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ reserved_combo reserved_ಹೊಂದಿಕೆ = {
		"",        RES_ಹೊಂದಿಕೆ, NOT_ASSIGNMENT , FLAG_ಹೊಂದಿಕೆ | FLAG_ESAC
	};
# endif
	ಸ್ಥಿರಾಂಕ ರಚನೆ reserved_combo *r;

	ಆದರೆ (ctx->ಶಬ್ದ.has_quoted_part)
		ಹಿಂತಿರುಗಿ 0;
	r = ಹೊಂದಿಕೆ_reserved_ಶಬ್ದ(&ctx->ಶಬ್ದ);
	ಆದರೆ (!r)
		ಹಿಂತಿರುಗಿ r; /* NULL */

	debug_printf("found reserved word %s, res %d\n", r->literal, r->res);
# ಆದರೆ ENABLE_HUSH_CASE
	ಆದರೆ (r->res == RES_ಒಳ && ctx->ctx_res_w == RES_CASE_ಒಳ) {
		/* "case word IN ..." - IN part starts first MATCH part */
		r = &reserved_ಹೊಂದಿಕೆ;
	} ಇಲ್ಲದಿದ್ದರೆ
# endif
	ಆದರೆ (r->flag == 0) { /* '!' */
		ಆದರೆ (ctx->ctx_inverted) { /* bash doesn't accept '! ! true' */
			syntax_error("! ! command");
			ctx->ctx_res_w = RES_SNTX;
		}
		ctx->ctx_inverted = 1;
		ಹಿಂತಿರುಗಿ r;
	}
	ಆದರೆ (r->flag & FLAG_ಆರಂಭ) {
		ರಚನೆ parse_context *old;

		old = xmemdup(ctx, ಗಾತ್ರ(*ctx));
		debug_printf_parse("push stack %p\n", old);
		initialize_context(ctx);
		ctx->ರಾಶಿ = old;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (/*ctx->ctx_res_w == RES_NONE ||*/ !(ctx->old_flag & (1 << r->res))) {
		syntax_error_at(ctx->ಶಬ್ದ.ದತ್ತಾಂಶ);
		ctx->ctx_res_w = RES_SNTX;
		ಹಿಂತಿರುಗಿ r;
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* "{...} fi" is ok. "{...} if" is not
		 * Example:
		 * if { echo foo; } then { echo bar; } fi */
		ಆದರೆ (ctx->command->ಗುಂಪು)
			ಮುಗಿದ_pipe(ctx, PIPE_SEQ);
	}

	ctx->ctx_res_w = r->res;
	ctx->old_flag = r->flag;
	ctx->is_assignment = r->assignment_flag;
	debug_printf_parse("ctx->is_assignment='%s'\n", assignment_flag[ctx->is_assignment]);

	ಆದರೆ (ctx->old_flag & FLAG_ಕೊನೆ) {
		ರಚನೆ parse_context *old;

		ಮುಗಿದ_pipe(ctx, PIPE_SEQ);
		debug_printf_parse("pop stack %p\n", ctx->ರಾಶಿ);
		old = ctx->ರಾಶಿ;
		old->command->ಗುಂಪು = ctx->list_ತಲೆ;
		old->command->cmd_type = CMD_ಸಾಮಾನ್ಯ;
# ಆದರೆ !BB_MMU
		/* At this point, the compound command's string is in
		 * ctx->as_string... except for the leading keyword!
		 * Consider this example: "echo a | if true; then echo a; fi"
		 * ctx->as_string will contain "true; then echo a; fi",
		 * with "if " remaining in old->as_string!
		 */
		{
			ಅಕ್ಷರ *str;
			ಪೂರ್ಣಾಂಕ len = old->as_string.ಉದ್ದಳತೆ;
			/* Concatenate halves */
			o_addstr(&old->as_string, ctx->as_string.ದತ್ತಾಂಶ);
			o_free(&ctx->as_string);
			/* Find where leading keyword starts in first half */
			str = old->as_string.ದತ್ತಾಂಶ + len;
			ಆದರೆ (str > old->as_string.ದತ್ತಾಂಶ)
				str--; /* skip whitespace after keyword */
			ಆಗ (str > old->as_string.ದತ್ತಾಂಶ && ಅಕ್ಷರವೇ(str[-1]))
				str--;
			/* Ugh, we're done with this horrid hack */
			old->command->ಗುಂಪು_as_string = xstrdup(str);
			debug_printf_parse("pop, remembering as:'%s'\n",
					old->command->ಗುಂಪು_as_string);
		}
# endif
		*ctx = *old;   /* physical copy */
		ಬಿಡುಗಡೆ(old);
	}
	ಹಿಂತಿರುಗಿ r;
}
#ಆದರೆ_ಅಂತ್ಯ /* HAS_KEYWORDS */

/* Word is complete, look at it and update parsing context.
 * Normal return is 0. Syntax errors return 1.
 * Note: on return, word is reset, but not o_free'd!
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಮುಗಿದ_ಶಬ್ದ(ರಚನೆ parse_context *ctx)
{
	ರಚನೆ command *command = ctx->command;

	debug_printf_parse("done_word entered: '%s' %p\n", ctx->ಶಬ್ದ.ದತ್ತಾಂಶ, command);
	ಆದರೆ (ctx->ಶಬ್ದ.ಉದ್ದಳತೆ == 0 && !ctx->ಶಬ್ದ.has_quoted_part) {
		debug_printf_parse("done_word return 0: true null, ignored\n");
		ಹಿಂತಿರುಗಿ 0;
	}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	ಆದರೆ (ctx->pending_redirect) {
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

		ctx->pending_redirect->rd_filename = xstrdup(ctx->ಶಬ್ದ.ದತ್ತಾಂಶ);
		/* Cater for >\file case:
		 * >\a creates file a; >\\a, >"\a", >"\\a" create file \a
		 * Same with heredocs:
		 * for <<\H delim is H; <<\\H, <<"\H", <<"\\H" - \H
		 */
		ಆದರೆ (ctx->pending_redirect->rd_type == REDIRECT_HEREDOC) {
			unbackslash(ctx->pending_redirect->rd_filename);
			/* Is it <<"HEREDOC"? */
			ಆದರೆ (ctx->ಶಬ್ದ.has_quoted_part) {
				ctx->pending_redirect->rd_dup |= HEREDOC_QUOTED;
			}
		}
		debug_printf_parse("word stored in rd_filename: '%s'\n", ctx->ಶಬ್ದ.ದತ್ತಾಂಶ);
		ctx->pending_redirect = ಖಾಲಿ;
	} ಇಲ್ಲದಿದ್ದರೆ {
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */
#ಆದರೆ HAS_KEYWORDS
# ಆದರೆ ENABLE_HUSH_CASE
		ಆದರೆ (ctx->ctx_dsemicolon
		 && strcmp(ctx->ಶಬ್ದ.ದತ್ತಾಂಶ, "esac") != 0 /* not "... pattern) cmd;; esac" */
		) {
			/* already done when ctx_dsemicolon was set to 1: */
			/* ctx->ctx_res_w = RES_MATCH; */
			ctx->ctx_dsemicolon = 0;
		} ಇಲ್ಲದಿದ್ದರೆ
# endif
# ಆದರೆ defined(CMD_TEST2_SINGLEWORD_NOGLOB)
		ಆದರೆ (command->cmd_type == CMD_TEST2_SINGLEWORD_NOGLOB
		 && strcmp(ctx->ಶಬ್ದ.ದತ್ತಾಂಶ, "]]") == 0
		) {
			/* allow "[[ ]] >file" etc */
			command->cmd_type = CMD_SINGLEWORD_NOGLOB;
		} ಇಲ್ಲದಿದ್ದರೆ
# endif
		ಆದರೆ (!command->argv /* if it's the first word... */
# ಆದರೆ ENABLE_HUSH_LOOPS
		 && ctx->ctx_res_w != RES_FOR /* ...not after FOR or IN */
		 && ctx->ctx_res_w != RES_ಒಳ
# endif
# ಆದರೆ ENABLE_HUSH_CASE
		 && ctx->ctx_res_w != RES_CASE
# endif
		) {
			ಸ್ಥಿರಾಂಕ ರಚನೆ reserved_combo *reserved;
			reserved = reserved_ಶಬ್ದ(ctx);
			debug_printf_parse("checking for reserved-ness: %d\n", !!reserved);
			ಆದರೆ (reserved) {
# ಆದರೆ ENABLE_HUSH_LINENO_VAR
/* Case:
 * "while ...; do
 *	cmd ..."
 * If we don't close the pipe _now_, immediately after "do", lineno logic
 * sees "cmd" as starting at "do" - i.e., at the previous line.
 */
				ಆದರೆ (0
				 IF_HUSH_IF(|| reserved->res == RES_THEN)
				 IF_HUSH_IF(|| reserved->res == RES_ELIF)
				 IF_HUSH_IF(|| reserved->res == RES_ELSE)
				 IF_HUSH_LOOPS(|| reserved->res == RES_DO)
				) {
					ಮುಗಿದ_pipe(ctx, PIPE_SEQ);
				}
# endif
				o_reset_ಗಮ್ಯ_ಖಾಲಿಗಳು_unquoted(&ctx->ಶಬ್ದ);
				debug_printf_parse("done_word return %d\n",
						(ctx->ctx_res_w == RES_SNTX));
				ಹಿಂತಿರುಗಿ (ctx->ctx_res_w == RES_SNTX);
			}
# ಆದರೆ defined(CMD_TEST2_SINGLEWORD_NOGLOB)
			ಆದರೆ (strcmp(ctx->ಶಬ್ದ.ದತ್ತಾಂಶ, "[[") == 0) {
				command->cmd_type = CMD_TEST2_SINGLEWORD_NOGLOB;
			} ಇಲ್ಲದಿದ್ದರೆ
# endif
# ಆದರೆ defined(CMD_SINGLEWORD_NOGLOB)
			ಆದರೆ (0
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
			 IF_HUSH_LOCAL(   || strcmp(ctx->ಶಬ್ದ.ದತ್ತಾಂಶ, "local") == 0)
			 IF_HUSH_EXPORT(  || strcmp(ctx->ಶಬ್ದ.ದತ್ತಾಂಶ, "export") == 0)
			 IF_HUSH_READONLY(|| strcmp(ctx->ಶಬ್ದ.ದತ್ತಾಂಶ, "readonly") == 0)
			) {
				command->cmd_type = CMD_SINGLEWORD_NOGLOB;
			}
# ಇಲ್ಲದಿದ್ದರೆ
			{ /* empty block to pair "if ... else" */ }
# endif
		}
#ಆದರೆ_ಅಂತ್ಯ /* HAS_KEYWORDS */

		ಆದರೆ (command->ಗುಂಪು) {
			/* "{ echo foo; } echo bar" - bad */
			syntax_error_at(ctx->ಶಬ್ದ.ದತ್ತಾಂಶ);
			debug_printf_parse("done_word return 1: syntax error, "
					"groups and arglists don't mix\n");
			ಹಿಂತಿರುಗಿ 1;
		}

		/* If this word wasn't an assignment, next ones definitely
		 * can't be assignments. Even if they look like ones. */
		ಆದರೆ (ctx->is_assignment != DEFINITELY_ASSIGNMENT
		 && ctx->is_assignment != ಶಬ್ದ_IS_KEYWORD
		) {
			ctx->is_assignment = NOT_ASSIGNMENT;
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಆದರೆ (ctx->is_assignment == DEFINITELY_ASSIGNMENT) {
				command->assignment_cnt++;
				debug_printf_parse("++assignment_cnt=%d\n", command->assignment_cnt);
			}
			debug_printf_parse("ctx->is_assignment was:'%s'\n", assignment_flag[ctx->is_assignment]);
			ctx->is_assignment = MAYBE_ASSIGNMENT;
		}
		debug_printf_parse("ctx->is_assignment='%s'\n", assignment_flag[ctx->is_assignment]);
		command->argv = ಕೂಡಿಸು_string_ಗಮ್ಯ_ಅಕ್ಷರಸರಣಿಗಳು(command->argv, xstrdup(ctx->ಶಬ್ದ.ದತ್ತಾಂಶ));
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __U_BOOT__
		command->argc++;
#ಆದರೆ_ಅಂತ್ಯ /* __U_BOOT__ */
		debug_print_ಅಕ್ಷರಸರಣಿಗಳು("word appended to argv", command->argv);

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	}
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */

#ಆದರೆ ENABLE_HUSH_LOOPS
	ಆದರೆ (ctx->ctx_res_w == RES_FOR) {
		ಆದರೆ (ctx->ಶಬ್ದ.has_quoted_part
		 || endofname(command->argv[0])[0] != '\0'
		) {
			/* bash says just "not a valid identifier" */
			syntax_error("bad for loop variable");
			ಹಿಂತಿರುಗಿ 1;
		}
		/* Force FOR to have just one word (variable name) */
		/* NB: basically, this makes hush see "for v in ..."
		 * syntax as if it is "for v; in ...". FOR and IN become
		 * two pipe structs in parse tree. */
		ಮುಗಿದ_pipe(ctx, PIPE_SEQ);
	}
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_HUSH_CASE
	/* Force CASE to have just one word */
	ಆದರೆ (ctx->ctx_res_w == RES_CASE) {
		ಮುಗಿದ_pipe(ctx, PIPE_SEQ);
	}
#ಆದರೆ_ಅಂತ್ಯ

	o_reset_ಗಮ್ಯ_ಖಾಲಿಗಳು_unquoted(&ctx->ಶಬ್ದ);

	debug_printf_parse("done_word return 0\n");
	ಹಿಂತಿರುಗಿ 0;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
/* Peek ahead in the input to find out if we have a "&n" construct,
 * as in "2>&1", that represents duplicating a file descriptor.
 * Return:
 * REDIRFD_CLOSE if >&- "close fd" construct is seen,
 * REDIRFD_SYNTAX_ERR if syntax error,
 * REDIRFD_TO_FILE if no & was seen,
 * or the number found.
 */
#ಆದರೆ BB_MMU
#ವ್ಯಾಖ್ಯಾನಿಸು parse_redir_ಬಲ_fd(as_string, ಒಳಹರಿವು) \
	parse_redir_ಬಲ_fd(ಒಳಹರಿವು)
#ಆದರೆ_ಅಂತ್ಯ
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ parse_redir_ಬಲ_fd(o_string *as_string, ರಚನೆ ಒಳ_str *ಒಳಹರಿವು)
{
	ಪೂರ್ಣಾಂಕ ch, d, ok;

	ch = i_peek(ಒಳಹರಿವು);
	ಆದರೆ (ch != '&')
		ಹಿಂತಿರುಗಿ REDIRFD_ಗಮ್ಯ_FILE;

	ch = i_getch(ಒಳಹರಿವು);  /* get the & */
	nommu_addchr(as_string, ch);
	ch = i_peek(ಒಳಹರಿವು);
	ಆದರೆ (ch == '-') {
		ch = i_getch(ಒಳಹರಿವು);
		nommu_addchr(as_string, ch);
		ಹಿಂತಿರುಗಿ REDIRFD_CLOSE;
	}
	d = 0;
	ok = 0;
	ಆಗ (ch != EOF && ಅಂಕಿಯೇ(ch)) {
		d = d*10 + (ch-'0');
		ok = 1;
		ch = i_getch(ಒಳಹರಿವು);
		nommu_addchr(as_string, ch);
		ch = i_peek(ಒಳಹರಿವು);
	}
	ಆದರೆ (ok) ಹಿಂತಿರುಗಿ d;

//TODO: this is the place to catch ">&file" bashism (redirect both fd 1 and 2)

	bb_simple_error_ಸಂದೇಶ("ambiguous redirect");
	ಹಿಂತಿರುಗಿ REDIRFD_SYNTAX_ERR;
}

/* Return code is 0 normal, 1 if a syntax error is detected
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ parse_redirect(ರಚನೆ parse_context *ctx,
		ಪೂರ್ಣಾಂಕ fd,
		redir_type style,
		ರಚನೆ ಒಳ_str *ಒಳಹರಿವು)
{
	ರಚನೆ command *command = ctx->command;
	ರಚನೆ redir_struct *redir;
	ರಚನೆ redir_struct **redirp;
	ಪೂರ್ಣಾಂಕ dup_ಅಂಕೆ;

	dup_ಅಂಕೆ = REDIRFD_ಗಮ್ಯ_FILE;
	ಆದರೆ (style != REDIRECT_HEREDOC) {
		/* Check for a '>&1' type redirect */
		dup_ಅಂಕೆ = parse_redir_ಬಲ_fd(&ctx->as_string, ಒಳಹರಿವು);
		ಆದರೆ (dup_ಅಂಕೆ == REDIRFD_SYNTAX_ERR)
			ಹಿಂತಿರುಗಿ 1;
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಪೂರ್ಣಾಂಕ ch = i_peek_and_eat_bkslash_nl(ಒಳಹರಿವು);
		dup_ಅಂಕೆ = (ch == '-'); /* HEREDOC_SKIPTABS bit is 1 */
		ಆದರೆ (dup_ಅಂಕೆ) { /* <<-... */
			ch = i_getch(ಒಳಹರಿವು);
			nommu_addchr(&ctx->as_string, ch);
			ch = i_peek(ಒಳಹರಿವು);
		}
	}

	ಆದರೆ (style == REDIRECT_OVERWRITE && dup_ಅಂಕೆ == REDIRFD_ಗಮ್ಯ_FILE) {
		ಪೂರ್ಣಾಂಕ ch = i_peek_and_eat_bkslash_nl(ಒಳಹರಿವು);
		ಆದರೆ (ch == '|') {
			/* >|FILE redirect ("clobbering" >).
			 * Since we do not support "set -o noclobber" yet,
			 * >| and > are the same for now. Just eat |.
			 */
			ch = i_getch(ಒಳಹರಿವು);
			nommu_addchr(&ctx->as_string, ch);
		}
	}

	/* Create a new redir_struct and append it to the linked list */
	redirp = &command->redirects;
	ಆಗ ((redir = *redirp) != ಖಾಲಿ) {
		redirp = &(redir->ಮುಂದಿನ);
	}
	*redirp = redir = xzalloc(ಗಾತ್ರ(*redir));
	/* redir->next = NULL; */
	/* redir->rd_filename = NULL; */
	redir->rd_type = style;
	redir->rd_fd = (fd == -1) ? redir_ಕೋಷ್ಟಕ[style].default_fd : fd;

	debug_printf_parse("redirect type %d %s\n", redir->rd_fd,
				redir_ಕೋಷ್ಟಕ[style].descrip);

	redir->rd_dup = dup_ಅಂಕೆ;
	ಆದರೆ (style != REDIRECT_HEREDOC && dup_ಅಂಕೆ != REDIRFD_ಗಮ್ಯ_FILE) {
		/* Erik had a check here that the file descriptor in question
		 * is legit; I postpone that to "run time"
		 * A "-" representation of "close me" shows up as a -3 here */
		debug_printf_parse("duplicating redirect '%d>&%d'\n",
				redir->rd_fd, redir->rd_dup);
	} ಇಲ್ಲದಿದ್ದರೆ {
#ಆದರೆ 0		/* Instead we emit error message at run time */
		ಆದರೆ (ctx->pending_redirect) {
			/* For example, "cmd > <file" */
			syntax_error("invalid redirect");
		}
#ಆದರೆ_ಅಂತ್ಯ
		/* Set ctx->pending_redirect, so we know what to do at the
		 * end of the next parsed word. */
		ctx->pending_redirect = redir;
	}
	ಹಿಂತಿರುಗಿ 0;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ redirect_opt_ಅಂಕೆ(o_string *o)
{
	ಪೂರ್ಣಾಂಕ ಅಂಕೆ;

	ಆದರೆ (o->ದತ್ತಾಂಶ == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ -1;
	ಅಂಕೆ = bb_strtou(o->ದತ್ತಾಂಶ, ಖಾಲಿ, 10);
	ಆದರೆ (errno || ಅಂಕೆ < 0)
		ಹಿಂತಿರುಗಿ -1;
	o_reset_ಗಮ್ಯ_ಖಾಲಿಗಳು_unquoted(o);
	ಹಿಂತಿರುಗಿ ಅಂಕೆ;
}

#ಆದರೆ BB_MMU
#ವ್ಯಾಖ್ಯಾನಿಸು fetch_till_str(as_string, ಒಳಹರಿವು, ಶಬ್ದ, skip_tabs) \
	fetch_till_str(ಒಳಹರಿವು, ಶಬ್ದ, skip_tabs)
#ಆದರೆ_ಅಂತ್ಯ
ಸ್ಥಿರ ಅಕ್ಷರ *fetch_till_str(o_string *as_string,
		ರಚನೆ ಒಳ_str *ಒಳಹರಿವು,
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಶಬ್ದ,
		ಪೂರ್ಣಾಂಕ heredoc_flags)
{
	o_string heredoc = NULL_O_STRING;
	ಚಿಹ್ನೆರಹಿತ past_EOL;
	ಪೂರ್ಣಾಂಕ prev = 0; /* not \ */
	ಪೂರ್ಣಾಂಕ ch;

	/* Starting with "" is necessary for this case:
	 * cat <<EOF
	 *
	 * xxx
	 * EOF
	 */
	heredoc.ದತ್ತಾಂಶ = xzalloc(1); /* start as "", not as NULL */

	ಹೋಗು jump_ಒಳ;

	ಆಗ (1) {
		ch = i_getch(ಒಳಹರಿವು);
		ಆದರೆ (ch != EOF)
			nommu_addchr(as_string, ch);
		ಆದರೆ (ch == '\n' || ch == EOF) {
 check_heredoc_ಕೊನೆ:
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
			ಆದರೆ ((heredoc_flags & HEREDOC_QUOTED) || prev != '\\') {
#ಇಲ್ಲದಿದ್ದರೆ /* __U_BOOT__ */
			ಆದರೆ (prev != '\\') {
#ಆದರೆ_ಅಂತ್ಯ
				/* End-of-line, and not a line continuation */
				ಆದರೆ (strcmp(heredoc.ದತ್ತಾಂಶ + past_EOL, ಶಬ್ದ) == 0) {
					heredoc.ದತ್ತಾಂಶ[past_EOL] = '\0';
					debug_printf_heredoc("parsed '%s' heredoc '%s'\n", ಶಬ್ದ, heredoc.ದತ್ತಾಂಶ);
					ಹಿಂತಿರುಗಿ heredoc.ದತ್ತಾಂಶ;
				}
				ಆದರೆ (ch == '\n') {
					/* This is a new line.
					 * Remember position and backslash-escaping status.
					 */
					o_addchr(&heredoc, ch);
					prev = ch;
 jump_ಒಳ:
					past_EOL = heredoc.ಉದ್ದಳತೆ;
					/* Get 1st char of next line, possibly skipping leading tabs */
					ಮಾಡು {
						ch = i_getch(ಒಳಹರಿವು);
						ಆದರೆ (ch != EOF)
							nommu_addchr(as_string, ch);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
					} ಆಗ ((heredoc_flags & HEREDOC_SKIPTABS) && ch == '\t');
#ಇಲ್ಲದಿದ್ದರೆ /* __U_BOOT__ */
				} ಆಗ (ch == '\t');
#ಆದರೆ_ಅಂತ್ಯ
					/* If this immediately ended the line,
					 * go back to end-of-line checks.
					 */
					ಆದರೆ (ch == '\n')
						ಹೋಗು check_heredoc_ಕೊನೆ;
				}
			} ಇಲ್ಲದಿದ್ದರೆ {
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
				heredoc.ದತ್ತಾಂಶ[--heredoc.ಉದ್ದಳತೆ] = '\0';
				prev = 0; /* not '\' */
				ಮುಂದುವರಿ;
			}
		}
		ಆದರೆ (ch == EOF) {
			o_free(&heredoc);
			ಹಿಂತಿರುಗಿ ಖಾಲಿ; /* error */
		}
		o_addchr(&heredoc, ch);
		nommu_addchr(as_string, ch);
		ಆದರೆ (prev == '\\' && ch == '\\')
			/* Correctly handle foo\\<eol> (not a line cont.) */
			prev = 0; /* not '\' */
		ಇಲ್ಲದಿದ್ದರೆ
			prev = ch;
	}
}
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */

/* Look at entire parse tree for not-yet-loaded REDIRECT_HEREDOCs
 * and load them all. There should be exactly heredoc_cnt of them.
 */
#ಆದರೆ BB_MMU
#ವ್ಯಾಖ್ಯಾನಿಸು fetch_heredocs(as_string, pi, heredoc_cnt, ಒಳಹರಿವು) \
	fetch_heredocs(pi, heredoc_cnt, ಒಳಹರಿವು)
#ಆದರೆ_ಅಂತ್ಯ
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fetch_heredocs(o_string *as_string, ರಚನೆ pipe *pi, ಪೂರ್ಣಾಂಕ heredoc_cnt, ರಚನೆ ಒಳ_str *ಒಳಹರಿವು)
{
	ಆಗ (pi && heredoc_cnt) {
		ಪೂರ್ಣಾಂಕ i;
		ರಚನೆ command *cmd = pi->cmds;

		debug_printf_heredoc("fetch_heredocs: num_cmds:%d cmd argv0:'%s'\n",
				pi->ಅಂಕೆ_cmds,
				cmd->argv ? cmd->argv[0] : "NONE"
		);
		ಗಾಗಿ (i = 0; i < pi->ಅಂಕೆ_cmds; i++) {
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
			ರಚನೆ redir_struct *redir = cmd->redirects;

#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */
			debug_printf_heredoc("fetch_heredocs: %d cmd argv0:'%s'\n",
					i, cmd->argv ? cmd->argv[0] : "NONE");
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
			ಆಗ (redir) {
				ಆದರೆ (redir->rd_type == REDIRECT_HEREDOC) {
					ಅಕ್ಷರ *p;

					redir->rd_type = REDIRECT_HEREDOC2;
					/* redir->rd_dup is (ab)used to indicate <<- */
					p = fetch_till_str(as_string, ಒಳಹರಿವು,
							redir->rd_filename, redir->rd_dup);
					ಆದರೆ (!p) {
						syntax_error("unexpected EOF in here document");
						ಹಿಂತಿರುಗಿ -1;
					}
					ಬಿಡುಗಡೆ(redir->rd_filename);
					redir->rd_filename = p;
					heredoc_cnt--;
				}
				redir = redir->ಮುಂದಿನ;
			}
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */
			ಆದರೆ (cmd->ಗುಂಪು) {
				//bb_error_msg("%s:%u heredoc_cnt:%d", __func__, __LINE__, heredoc_cnt);
				heredoc_cnt = fetch_heredocs(as_string, cmd->ಗುಂಪು, heredoc_cnt, ಒಳಹರಿವು);
				//bb_error_msg("%s:%u heredoc_cnt:%d", __func__, __LINE__, heredoc_cnt);
				ಆದರೆ (heredoc_cnt < 0)
					ಹಿಂತಿರುಗಿ heredoc_cnt; /* error */
			}
			cmd++;
		}
		pi = pi->ಮುಂದಿನ;
	}
	ಹಿಂತಿರುಗಿ heredoc_cnt;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಸತತ_list(ರಚನೆ pipe *pi);
#ಆದರೆ BB_MMU
#ವ್ಯಾಖ್ಯಾನಿಸು parse_ಹರಿವು(pstring, heredoc_cnt_ptr, ಒಳಹರಿವು, ಕೊನೆ_trigger) \
	parse_ಹರಿವು(heredoc_cnt_ptr, ಒಳಹರಿವು, ಕೊನೆ_trigger)
#ಆದರೆ_ಅಂತ್ಯ
ಸ್ಥಿರ ರಚನೆ pipe *parse_ಹರಿವು(ಅಕ್ಷರ **pstring,
		ಪೂರ್ಣಾಂಕ *heredoc_cnt_ptr,
		ರಚನೆ ಒಳ_str *ಒಳಹರಿವು,
		ಪೂರ್ಣಾಂಕ ಕೊನೆ_trigger);

/* Returns number of heredocs not yet consumed,
 * or -1 on error.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ parse_ಗುಂಪು(ರಚನೆ parse_context *ctx,
		ರಚನೆ ಒಳ_str *ಒಳಹರಿವು, ಪೂರ್ಣಾಂಕ ch)
{
	/* ctx->word contains characters seen prior to ( or {.
	 * Typically it's empty, but for function defs,
	 * it contains function name (without '()'). */
#ಆದರೆ BB_MMU
# define as_string ಖಾಲಿ
#ಇಲ್ಲದಿದ್ದರೆ
	ಅಕ್ಷರ *as_string = ಖಾಲಿ;
#ಆದರೆ_ಅಂತ್ಯ
	ರಚನೆ pipe *pipe_list;
	ಪೂರ್ಣಾಂಕ heredoc_cnt = 0;
	ಪೂರ್ಣಾಂಕ endch;
	ರಚನೆ command *command = ctx->command;

	debug_printf_parse("parse_group entered\n");
#ಆದರೆ ENABLE_HUSH_FUNCTIONS
	ಆದರೆ (ch == '(' && !ctx->ಶಬ್ದ.has_quoted_part) {
		ಆದರೆ (ctx->ಶಬ್ದ.ಉದ್ದಳತೆ)
			ಆದರೆ (ಮುಗಿದ_ಶಬ್ದ(ctx))
				ಹಿಂತಿರುಗಿ -1;
		ಆದರೆ (!command->argv)
			ಹೋಗು skip; /* (... */
		ಆದರೆ (command->argv[1]) { /* word word ... (... */
			syntax_error_unexpected_ch('(');
			ಹಿಂತಿರುಗಿ -1;
		}
		/* it is "word(..." or "word (..." */
		ಮಾಡು
			ch = i_getch(ಒಳಹರಿವು);
		ಆಗ (ch == ' ' || ch == '\t');
		ಆದರೆ (ch != ')') {
			syntax_error_unexpected_ch(ch);
			ಹಿಂತಿರುಗಿ -1;
		}
		nommu_addchr(&ctx->as_string, ch);
		ಮಾಡು
			ch = i_getch(ಒಳಹರಿವು);
		ಆಗ (ch == ' ' || ch == '\t' || ch == '\n');
		ಆದರೆ (ch != '{' && ch != '(') {
			syntax_error_unexpected_ch(ch);
			ಹಿಂತಿರುಗಿ -1;
		}
//bash allows functions named "123", "..", "return"!
//		if (endofname(command->argv[0])[0] != '\0') {
//			syntax_error("bad function name");
//			return -1;
//		}
		nommu_addchr(&ctx->as_string, ch);
		command->cmd_type = CMD_FUNCDEF;
		ಹೋಗು skip;
	}
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ 0 /* Prevented by caller */
	ಆದರೆ (command->argv /* word [word]{... */
	 || ctx->ಶಬ್ದ.ಉದ್ದಳತೆ /* word{... */
	 || ctx->ಶಬ್ದ.has_quoted_part /* ""{... */
	) {
		syntax_error(ಖಾಲಿ);
		debug_printf_parse("parse_group return -1: "
			"syntax error, groups and arglists don't mix\n");
		ಹಿಂತಿರುಗಿ -1;
	}
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
 IF_HUSH_FUNCTIONS(skip:)
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */

	endch = '}';
	ಆದರೆ (ch == '(') {
		endch = ')';
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
		IF_HUSH_FUNCTIONS(ಆದರೆ (command->cmd_type != CMD_FUNCDEF))
			command->cmd_type = CMD_SUBSHELL;
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* bash does not allow "{echo...", requires whitespace */
		ch = i_peek(ಒಳಹರಿವು);
		ಆದರೆ (ch != ' ' && ch != '\t' && ch != '\n'
		 && ch != '('	/* but "{(..." is allowed (without whitespace) */
		) {
			syntax_error_unexpected_ch(ch);
			ಹಿಂತಿರುಗಿ -1;
		}
		ಆದರೆ (ch != '(') {
			ch = i_getch(ಒಳಹರಿವು);
			nommu_addchr(&ctx->as_string, ch);
		}
	}

	debug_printf_heredoc("calling parse_stream, heredoc_cnt:%d\n", heredoc_cnt);
	pipe_list = parse_ಹರಿವು(&as_string, &heredoc_cnt, ಒಳಹರಿವು, endch);
	debug_printf_heredoc("parse_stream returned: heredoc_cnt:%d\n", heredoc_cnt);
#ಆದರೆ !BB_MMU
	ಆದರೆ (as_string)
		o_addstr(&ctx->as_string, as_string);
#ಆದರೆ_ಅಂತ್ಯ

	/* empty ()/{} or parse error? */
	ಆದರೆ (!pipe_list || pipe_list == ERR_PTR) {
		/* parse_stream already emitted error msg */
		ಆದರೆ (!BB_MMU)
			ಬಿಡುಗಡೆ(as_string);
		debug_printf_parse("parse_group return -1: "
			"parse_stream returned %p\n", pipe_list);
		ಹಿಂತಿರುಗಿ -1;
	}
#ಆದರೆ !BB_MMU
	as_string[ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(as_string) - 1] = '\0'; /* plink ')' or '}' */
	command->ಗುಂಪು_as_string = as_string;
	debug_printf_parse("end of group, remembering as:'%s'\n",
			command->ಗುಂಪು_as_string);
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ ENABLE_HUSH_FUNCTIONS
	/* Convert "f() (cmds)" to "f() {(cmds)}" */
	ಆದರೆ (command->cmd_type == CMD_FUNCDEF && endch == ')') {
		ರಚನೆ command *cmd2;

		cmd2 = xzalloc(ಗಾತ್ರ(*cmd2));
		cmd2->cmd_type = CMD_SUBSHELL;
		cmd2->ಗುಂಪು = pipe_list;
# ಆದರೆ !BB_MMU
//UNTESTED!
		cmd2->ಗುಂಪು_as_string = command->ಗುಂಪು_as_string;
		command->ಗುಂಪು_as_string = xasprintf("(%s)", command->ಗುಂಪು_as_string);
# endif

		pipe_list = new_pipe();
		pipe_list->cmds = cmd2;
		pipe_list->ಅಂಕೆ_cmds = 1;
	}
#ಆದರೆ_ಅಂತ್ಯ

	command->ಗುಂಪು = pipe_list;

	debug_printf_parse("parse_group return %d\n", heredoc_cnt);
	ಹಿಂತಿರುಗಿ heredoc_cnt;
	/* command remains "open", available for possible redirects */
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು as_string
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
#ಆದರೆ ENABLE_HUSH_TICK || ENABLE_FEATURE_SH_MATH || ENABLE_HUSH_DOLLAR_OPS
/* Subroutines for copying $(...) and `...` things */
/* '...' */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಕೂಡಿಸು_till_single_quote(o_string *dest, ರಚನೆ ಒಳ_str *ಒಳಹರಿವು)
{
	ಆಗ (1) {
		ಪೂರ್ಣಾಂಕ ch = i_getch(ಒಳಹರಿವು);
		ಆದರೆ (ch == EOF) {
			syntax_error_unterm_ch('\'');
			ಹಿಂತಿರುಗಿ 0;
		}
		ಆದರೆ (ch == '\'')
			ಹಿಂತಿರುಗಿ 1;
		o_addchr(dest, ch);
	}
}
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಕೂಡಿಸು_till_single_quote_dquoted(o_string *dest, ರಚನೆ ಒಳ_str *ಒಳಹರಿವು)
{
	ಆಗ (1) {
		ಪೂರ್ಣಾಂಕ ch = i_getch(ಒಳಹರಿವು);
		ಆದರೆ (ch == EOF) {
			syntax_error_unterm_ch('\'');
			ಹಿಂತಿರುಗಿ 0;
		}
		ಆದರೆ (ch == '\'')
			ಹಿಂತಿರುಗಿ 1;
		o_addqchr(dest, ch);
	}
}

/* "...\"...`..`...." - do we need to handle "...$(..)..." too? */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಕೂಡಿಸು_till_backquote(o_string *dest, ರಚನೆ ಒಳ_str *ಒಳಹರಿವು, ಪೂರ್ಣಾಂಕ ಒಳ_dquote);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಕೂಡಿಸು_till_double_quote(o_string *dest, ರಚನೆ ಒಳ_str *ಒಳಹರಿವು)
{
	ಆಗ (1) {
		ಪೂರ್ಣಾಂಕ ch = i_getch(ಒಳಹರಿವು);
		ಆದರೆ (ch == EOF) {
			syntax_error_unterm_ch('"');
			ಹಿಂತಿರುಗಿ 0;
		}
		ಆದರೆ (ch == '"')
			ಹಿಂತಿರುಗಿ 1;
		ಆದರೆ (ch == '\\') {  /* \x. Copy both chars. */
			o_addchr(dest, ch);
			ch = i_getch(ಒಳಹರಿವು);
		}
		o_addchr(dest, ch);
		ಆದರೆ (ch == '`') {
			ಆದರೆ (!ಕೂಡಿಸು_till_backquote(dest, ಒಳಹರಿವು, /*in_dquote:*/ 1))
				ಹಿಂತಿರುಗಿ 0;
			o_addchr(dest, ch);
			ಮುಂದುವರಿ;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಕೂಡಿಸು_till_backquote(o_string *dest, ರಚನೆ ಒಳ_str *ಒಳಹರಿವು, ಪೂರ್ಣಾಂಕ ಒಳ_dquote)
{
	ಆಗ (1) {
		ಪೂರ್ಣಾಂಕ ch = i_getch(ಒಳಹರಿವು);
		ಆದರೆ (ch == '`')
			ಹಿಂತಿರುಗಿ 1;
		ಆದರೆ (ch == '\\') {
			/* \x. Copy both unless it is \`, \$, \\ and maybe \" */
			ch = i_getch(ಒಳಹರಿವು);
			ಆದರೆ (ch != '`'
			 && ch != '$'
			 && ch != '\\'
			 && (!ಒಳ_dquote || ch != '"')
			) {
				o_addchr(dest, '\\');
			}
		}
		ಆದರೆ (ch == EOF) {
			syntax_error_unterm_ch('`');
			ಹಿಂತಿರುಗಿ 0;
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
#ವ್ಯಾಖ್ಯಾನಿಸು DOUBLE_CLOSE_CHAR_FLAG 0x80
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಕೂಡಿಸು_till_closing_bracket(o_string *dest, ರಚನೆ ಒಳ_str *ಒಳಹರಿವು, ಚಿಹ್ನೆರಹಿತ ಕೊನೆ_ch)
{
	ಪೂರ್ಣಾಂಕ ch;
	ಅಕ್ಷರ dbl = ಕೊನೆ_ch & DOUBLE_CLOSE_CHAR_FLAG;
# ಆದರೆ BASH_SUBSTR || BASH_ನಮೂನೆ_SUBST
	ಅಕ್ಷರ ಕೊನೆ_char2 = ಕೊನೆ_ch >> 8;
# endif
	ಕೊನೆ_ch &= (DOUBLE_CLOSE_CHAR_FLAG - 1);

# ಆದರೆ ENABLE_HUSH_INTERACTIVE
	G.promptmode = 1; /* PS2 */
# endif
	debug_printf_prompt("%s promptmode=%d\n", __func__, G.promptmode);

	ಆಗ (1) {
		ch = i_getch(ಒಳಹರಿವು);
		ಆದರೆ (ch == EOF) {
			syntax_error_unterm_ch(ಕೊನೆ_ch);
			ಹಿಂತಿರುಗಿ 0;
		}
		ಆದರೆ (ch == ಕೊನೆ_ch
# ಆದರೆ BASH_SUBSTR || BASH_ನಮೂನೆ_SUBST
		 || ch == ಕೊನೆ_char2
# endif
		) {
			ಆದರೆ (!dbl)
				ನಿಲ್ಲಿಸು;
			/* we look for closing )) of $((EXPR)) */
			ಆದರೆ (i_peek_and_eat_bkslash_nl(ಒಳಹರಿವು) == ಕೊನೆ_ch) {
				i_getch(ಒಳಹರಿವು); /* eat second ')' */
				ನಿಲ್ಲಿಸು;
			}
		}
		o_addchr(dest, ch);
		//bb_error_msg("%s:o_addchr('%c')", __func__, ch);
		ಆದರೆ (ch == '(' || ch == '{') {
			ch = (ch == '(' ? ')' : '}');
			ಆದರೆ (!ಕೂಡಿಸು_till_closing_bracket(dest, ಒಳಹರಿವು, ch))
				ಹಿಂತಿರುಗಿ 0;
			o_addchr(dest, ch);
			ಮುಂದುವರಿ;
		}
		ಆದರೆ (ch == '\'') {
			ಆದರೆ (!ಕೂಡಿಸು_till_single_quote(dest, ಒಳಹರಿವು))
				ಹಿಂತಿರುಗಿ 0;
			o_addchr(dest, ch);
			ಮುಂದುವರಿ;
		}
		ಆದರೆ (ch == '"') {
			ಆದರೆ (!ಕೂಡಿಸು_till_double_quote(dest, ಒಳಹರಿವು))
				ಹಿಂತಿರುಗಿ 0;
			o_addchr(dest, ch);
			ಮುಂದುವರಿ;
		}
		ಆದರೆ (ch == '`') {
			ಆದರೆ (!ಕೂಡಿಸು_till_backquote(dest, ಒಳಹರಿವು, /*in_dquote:*/ 0))
				ಹಿಂತಿರುಗಿ 0;
			o_addchr(dest, ch);
			ಮುಂದುವರಿ;
		}
		ಆದರೆ (ch == '\\') {
			/* \x. Copy verbatim. Important for  \(, \) */
			ch = i_getch(ಒಳಹರಿವು);
			ಆದರೆ (ch == EOF) {
				syntax_error_unterm_ch(ಕೊನೆ_ch);
				ಹಿಂತಿರುಗಿ 0;
			}
# ಆದರೆ 0
			ಆದರೆ (ch == '\n') {
				/* "backslash+newline", ignore both */
				o_delchr(dest); /* undo insertion of '\' */
				ಮುಂದುವರಿ;
			}
# endif
			o_addchr(dest, ch);
			//bb_error_msg("%s:o_addchr('%c') after '\\'", __func__, ch);
			ಮುಂದುವರಿ;
		}
	}
	debug_printf_parse("%s return '%s' ch:'%c'\n", __func__, dest->ದತ್ತಾಂಶ, ch);
	ಹಿಂತಿರುಗಿ ch;
}
#ಆದರೆ_ಅಂತ್ಯ /* ENABLE_HUSH_TICK || ENABLE_FEATURE_SH_MATH || ENABLE_HUSH_DOLLAR_OPS */

#ಆದರೆ BASH_DOLLAR_SQUOTE
/* Return code: 1 for "found and parsed", 0 for "seen something else" */
# ಆದರೆ BB_MMU
#ವ್ಯಾಖ್ಯಾನಿಸು parse_dollar_squote(as_string, dest, ಒಳಹರಿವು) \
	parse_dollar_squote(dest, ಒಳಹರಿವು)
#ವ್ಯಾಖ್ಯಾನಿಸು as_string ಖಾಲಿ
# endif
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ parse_dollar_squote(o_string *as_string, o_string *dest, ರಚನೆ ಒಳ_str *ಒಳಹರಿವು)
{
	ಪೂರ್ಣಾಂಕ ಆರಂಭ;
	ಪೂರ್ಣಾಂಕ ch = i_peek_and_eat_bkslash_nl(ಒಳಹರಿವು);  /* first character after the $ */
	debug_printf_parse("parse_dollar_squote entered: ch='%c'\n", ch);
	ಆದರೆ (ch != '\'')
		ಹಿಂತಿರುಗಿ 0;

	dest->has_quoted_part = 1;
	ಆರಂಭ = dest->ಉದ್ದಳತೆ;

	ch = i_getch(ಒಳಹರಿವು); /* eat ' */
	nommu_addchr(as_string, ch);
	ಆಗ (1) {
		ch = i_getch(ಒಳಹರಿವು);
		nommu_addchr(as_string, ch);
		ಆದರೆ (ch == EOF) {
			syntax_error_unterm_ch('\'');
			ಹಿಂತಿರುಗಿ 0;
		}
		ಆದರೆ (ch == '\'')
			ನಿಲ್ಲಿಸು;
		ಆದರೆ (ch == SPECIAL_VAR_ಚಿಹ್ನೆ) {
			/* Convert raw ^C to corresponding special variable reference */
			o_addchr(dest, SPECIAL_VAR_ಚಿಹ್ನೆ);
			o_addchr(dest, SPECIAL_VAR_QUOTED_SVS);
			/* will addchr() another SPECIAL_VAR_SYMBOL (see after the if() block) */
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ch == '\\') {
			ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ C_escapes[] ALIGN1 = "nrbtfav""x\\01234567";

			ch = i_getch(ಒಳಹರಿವು);
			nommu_addchr(as_string, ch);
			ಆದರೆ (strchr(C_escapes, ch)) {
				ಅಕ್ಷರ buf[4];
				ಅಕ್ಷರ *p = buf;
				ಪೂರ್ಣಾಂಕ cnt = 2;

				buf[0] = ch;
				ಆದರೆ ((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)(ch - '0') <= 7) { /* \ooo */
					ಮಾಡು {
						ch = i_peek(ಒಳಹರಿವು);
						ಆದರೆ ((ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)(ch - '0') > 7)
							ನಿಲ್ಲಿಸು;
						*++p = ch = i_getch(ಒಳಹರಿವು);
						nommu_addchr(as_string, ch);
					} ಆಗ (--cnt != 0);
				} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ch == 'x') { /* \xHH */
					ಮಾಡು {
						ch = i_peek(ಒಳಹರಿವು);
						ಆದರೆ (!isxdigit(ch))
							ನಿಲ್ಲಿಸು;
						*++p = ch = i_getch(ಒಳಹರಿವು);
						nommu_addchr(as_string, ch);
					} ಆಗ (--cnt != 0);
					ಆದರೆ (cnt == 2) { /* \x but next char is "bad" */
						ch = 'x';
						ಹೋಗು unrecognized;
					}
				} /* else simple seq like \\ or \t */
				*++p = '\0';
				p = buf;
				ch = bb_process_escape_sequence((ಶೂನ್ಯ*)&p);
				//bb_error_msg("buf:'%s' ch:%x", buf, ch);
				ಆದರೆ (ch == '\0')
					ಮುಂದುವರಿ; /* bash compat: $'...\0...' emits nothing */
			} ಇಲ್ಲದಿದ್ದರೆ { /* unrecognized "\z": encode both chars unless ' or " */
				ಆದರೆ (ch != '\'' && ch != '"') {
 unrecognized:
					o_addqchr(dest, '\\');
				}
			}
		} /* if (\...) */
		o_addqchr(dest, ch);
	}

	ಆದರೆ (dest->ಉದ್ದಳತೆ == ಆರಂಭ) {
		/* $'', $'\0', $'\000\x00' and the like */
		o_addchr(dest, SPECIAL_VAR_ಚಿಹ್ನೆ);
		o_addchr(dest, SPECIAL_VAR_ಚಿಹ್ನೆ);
	}

	ಹಿಂತಿರುಗಿ 1;
# undef as_string
}
#ಇಲ್ಲದಿದ್ದರೆ
# define parse_dollar_squote(as_string, dest, ಒಳಹರಿವು) 0
#ಆದರೆ_ಅಂತ್ಯ /* BASH_DOLLAR_SQUOTE */
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */

/* Return code: 0 for OK, 1 for syntax error */
#ಆದರೆ BB_MMU
#ವ್ಯಾಖ್ಯಾನಿಸು parse_dollar(as_string, dest, ಒಳಹರಿವು, quote_ಮುಖವಾಡ) \
	parse_dollar(dest, ಒಳಹರಿವು, quote_ಮುಖವಾಡ)
#ವ್ಯಾಖ್ಯಾನಿಸು as_string ಖಾಲಿ
#ಆದರೆ_ಅಂತ್ಯ
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ parse_dollar(o_string *as_string,
		o_string *dest,
		ರಚನೆ ಒಳ_str *ಒಳಹರಿವು, ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ quote_ಮುಖವಾಡ)
{
	ಪೂರ್ಣಾಂಕ ch = i_peek_and_eat_bkslash_nl(ಒಳಹರಿವು);  /* first character after the $ */

	debug_printf_parse("parse_dollar entered: ch='%c' quote_mask:0x%x\n", ch, quote_ಮುಖವಾಡ);
	ಆದರೆ (ಅಕ್ಷರವೇ(ch)) {
 make_var:
		ch = i_getch(ಒಳಹರಿವು);
		nommu_addchr(as_string, ch);
 /*make_var1:*/
		o_addchr(dest, SPECIAL_VAR_ಚಿಹ್ನೆ);
		ಆಗ (1) {
			debug_printf_parse(": '%c'\n", ch);
			o_addchr(dest, ch | quote_ಮುಖವಾಡ);
			quote_ಮುಖವಾಡ = 0;
			ch = i_peek_and_eat_bkslash_nl(ಒಳಹರಿವು);
			ಆದರೆ (!ಅಕ್ಷರವೇ_ಅಂಕಿಯೇ(ch) && ch != '_') {
				/* End of variable name reached */
				ನಿಲ್ಲಿಸು;
			}
			ch = i_getch(ಒಳಹರಿವು);
			nommu_addchr(as_string, ch);
		}
		o_addchr(dest, SPECIAL_VAR_ಚಿಹ್ನೆ);
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಅಂಕಿಯೇ(ch)) {
 make_one_char_var:
		ch = i_getch(ಒಳಹರಿವು);
		nommu_addchr(as_string, ch);
		o_addchr(dest, SPECIAL_VAR_ಚಿಹ್ನೆ);
		debug_printf_parse(": '%c'\n", ch);
		o_addchr(dest, ch | quote_ಮುಖವಾಡ);
		o_addchr(dest, SPECIAL_VAR_ಚಿಹ್ನೆ);
	} ಇಲ್ಲದಿದ್ದರೆ ಆಯ್ಕೆ (ch) {
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	ಸ್ಥಿತಿ '$': /* pid */
	ಸ್ಥಿತಿ '!': /* last bg pid */
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */
	ಸ್ಥಿತಿ '?': /* last exit code */
	ಸ್ಥಿತಿ '#': /* number of args */
	ಸ್ಥಿತಿ '*': /*ವಾದಗಳು*/
	ಸ್ಥಿತಿ '@': /*ವಾದಗಳು*/
	ಸ್ಥಿತಿ '-': /* $- option flags set by set builtin or shell options (-i etc) */
		ಹೋಗು make_one_char_var;
	ಸ್ಥಿತಿ '{': {
		ಅಕ್ಷರ len_single_ch;

		o_addchr(dest, SPECIAL_VAR_ಚಿಹ್ನೆ);

		ch = i_getch(ಒಳಹರಿವು); /* eat '{' */
		nommu_addchr(as_string, ch);

		ch = i_getch_and_eat_bkslash_nl(ಒಳಹರಿವು); /* first char after '{' */
		/* It should be ${?}, or ${#var},
		 * or even ${?+subst} - operator acting on a special variable,
		 * or the beginning of variable name.
		 */
		ಆದರೆ (ch == EOF
		 || (!strchr(_SPECIAL_VARS_STR, ch) && !ಅಕ್ಷರವೇ_ಅಂಕಿಯೇ(ch)) /* not one of those */
		) {
 bad_dollar_syntax:
			syntax_error_unterm_str("${name}");
			debug_printf_parse("parse_dollar return 0: unterminated ${name}\n");
			ಹಿಂತಿರುಗಿ 0;
		}
		nommu_addchr(as_string, ch);
		len_single_ch = ch;
		ch |= quote_ಮುಖವಾಡ;

		/* It's possible to just call add_till_closing_bracket() at this point.
		 * However, this regresses some of our testsuite cases
		 * which check invalid constructs like ${%}.
		 * Oh well... let's check that the var name part is fine... */

		ಆದರೆ (ಅಂಕಿಯೇ(len_single_ch)
		 || (len_single_ch == '#' && ಅಂಕಿಯೇ(i_peek_and_eat_bkslash_nl(ಒಳಹರಿವು)))
		) {
			/* Execution engine uses plain xatoi_positive()
			 * to interpret ${NNN} and {#NNN},
			 * check syntax here in the parser.
			 * (bash does not support expressions in ${#NN},
			 * e.g. ${#$var} and {#1:+WORD} are not supported).
			 */
			ಚಿಹ್ನೆರಹಿತ cnt = 9; /* max 9 digits for ${NN} and 8 for {#NN} */
			ಆಗ (1) {
				o_addchr(dest, ch);
				debug_printf_parse(": '%c'\n", ch);
				ch = i_getch_and_eat_bkslash_nl(ಒಳಹರಿವು);
				nommu_addchr(as_string, ch);
				ಆದರೆ (ch == '}')
					ನಿಲ್ಲಿಸು;
				ಆದರೆ (--cnt == 0)
					ಹೋಗು bad_dollar_syntax;
				ಆದರೆ (len_single_ch != '#' && strchr(VAR_SUBST_OPS, ch))
					/* ${NN<op>...} is valid */
					ಹೋಗು eat_until_closing;
				ಆದರೆ (!ಅಂಕಿಯೇ(ch))
					ಹೋಗು bad_dollar_syntax;
			}
		} ಇಲ್ಲದಿದ್ದರೆ
		ಆಗ (1) {
			ಚಿಹ್ನೆರಹಿತ pos;

			o_addchr(dest, ch);
			debug_printf_parse(": '%c'\n", ch);

			ch = i_getch(ಒಳಹರಿವು);
			nommu_addchr(as_string, ch);
			ಆದರೆ (ch == '}')
				ನಿಲ್ಲಿಸು;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
			ಆದರೆ (!ಅಕ್ಷರವೇ_ಅಂಕಿಯೇ(ch) && ch != '_') {
#ಇಲ್ಲದಿದ್ದರೆ /* __U_BOOT__ */
			/*
			 * In several places in U-Boot, we use variable like
			 * foo# (e.g. serial#), particularly in env.
			 * So, we need to authorize # to appear inside
			 * variable name and then expand this variable.
			 * NOTE Having # in variable name is not permitted in
			 * upstream hush but expansion will be done (even though
			 * the result will be empty).
			 */
			ಆದರೆ (!ಅಕ್ಷರವೇ_ಅಂಕಿಯೇ(ch) && ch != '_' && ch != '#') {
#ಆದರೆ_ಅಂತ್ಯ /* __U_BOOT__ */
				ಚಿಹ್ನೆರಹಿತ ಕೊನೆ_ch;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
				ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ ಕೊನೆಯ_ch;
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */
				/* handle parameter expansions
				 * http://www.opengroup.org/onlinepubs/009695399/utilities/xcu_chap02.html#tag_02_06_02
				 */
				ಆದರೆ (!strchr(VAR_SUBST_OPS, ch)) { /* ${var<bad_char>... */
					ಆದರೆ (len_single_ch != '#'
					/*|| !strchr(SPECIAL_VARS_STR, ch) - disallow errors like ${#+} ? */
					 || i_peek(ಒಳಹರಿವು) != '}'
					) {
						ಹೋಗು bad_dollar_syntax;
					}
					/* else: it's "length of C" ${#C} op,
					 * where C is a single char
					 * special var name, e.g. ${#!}.
					 */
				}
 eat_until_closing:
				/* Eat everything until closing '}' (or ':') */
				ಕೊನೆ_ch = '}';
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
				ಆದರೆ (BASH_SUBSTR
				 && ch == ':'
				 && !strchr(MINUS_PLUS_EQUAL_QUESTION, i_peek(ಒಳಹರಿವು))
				) {
					/* It's ${var:N[:M]} thing */
					ಕೊನೆ_ch = '}' * 0x100 + ':';
				}
				ಆದರೆ (BASH_ನಮೂನೆ_SUBST
				 && ch == '/'
				) {
					/* It's ${var/[/]pattern[/repl]} thing */
					ಆದರೆ (i_peek(ಒಳಹರಿವು) == '/') { /* ${var//pattern[/repl]}? */
						i_getch(ಒಳಹರಿವು);
						nommu_addchr(as_string, '/');
						ch = '\\';
					}
					ಕೊನೆ_ch = '}' * 0x100 + '/';
				}
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */
				o_addchr(dest, ch);
				/* The pattern can't be empty.
				 * IOW: if the first char after "${v//" is a slash,
				 * it does not terminate the pattern - it's the first char of the pattern:
				 *  v=/dev/ram; echo ${v////-}  prints -dev-ram (pattern is "/")
				 *  v=/dev/ram; echo ${v///r/-} prints /dev-am  (pattern is "/r")
				 */
				ಆದರೆ (i_peek(ಒಳಹರಿವು) == '/') {
					o_addchr(dest, i_getch(ಒಳಹರಿವು));
				}
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
 again:
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */
				ಆದರೆ (!BB_MMU)
					pos = dest->ಉದ್ದಳತೆ;
#ಆದರೆ ENABLE_HUSH_DOLLAR_OPS
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
				ಕೊನೆಯ_ch = ಕೂಡಿಸು_till_closing_bracket(dest, ಒಳಹರಿವು, ಕೊನೆ_ch);
				ಆದರೆ (ಕೊನೆಯ_ch == 0) /* error? */
					ಹಿಂತಿರುಗಿ 0;
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */
#ಇಲ್ಲದಿದ್ದರೆ
# error Simple ಸಂಕೇತ_2 ಗಮ್ಯ only allow ${var} is not implemented
#ಆದರೆ_ಅಂತ್ಯ
				ಆದರೆ (as_string) {
					o_addstr(as_string, dest->ದತ್ತಾಂಶ + pos);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
					o_addchr(as_string, ಕೊನೆಯ_ch);
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */
				}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
				ಆದರೆ ((BASH_SUBSTR || BASH_ನಮೂನೆ_SUBST)
					 && (ಕೊನೆ_ch & 0xff00)
				) {
					/* close the first block: */
					o_addchr(dest, SPECIAL_VAR_ಚಿಹ್ನೆ);
					/* while parsing N from ${var:N[:M]}
					 * or pattern from ${var/[/]pattern[/repl]} */
					ಆದರೆ ((ಕೊನೆ_ch & 0xff) == ಕೊನೆಯ_ch) {
						/* got ':' or '/'- parse the rest */
						ಕೊನೆ_ch = '}';
						ಹೋಗು again;
					}
					/* got '}' */
					ಆದರೆ (BASH_SUBSTR && ಕೊನೆ_ch == '}' * 0x100 + ':') {
						/* it's ${var:N} - emulate :999999999 */
						o_addstr(dest, "999999999");
					} /* else: it's ${var/[/]pattern} */
				}
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */
				ನಿಲ್ಲಿಸು;
			}
			len_single_ch = 0; /* it can't be ${#C} op */
		}
		o_addchr(dest, SPECIAL_VAR_ಚಿಹ್ನೆ);
		ನಿಲ್ಲಿಸು;
	}
#ಆದರೆ ENABLE_FEATURE_SH_MATH || ENABLE_HUSH_TICK
	ಸ್ಥಿತಿ '(': {
		ಚಿಹ್ನೆರಹಿತ pos;

		ch = i_getch(ಒಳಹರಿವು);
		nommu_addchr(as_string, ch);
# ಆದರೆ ENABLE_FEATURE_SH_MATH
		ಆದರೆ (i_peek_and_eat_bkslash_nl(ಒಳಹರಿವು) == '(') {
			ch = i_getch(ಒಳಹರಿವು);
			nommu_addchr(as_string, ch);
			o_addchr(dest, SPECIAL_VAR_ಚಿಹ್ನೆ);
			o_addchr(dest, quote_ಮುಖವಾಡ | '+');
			ಆದರೆ (!BB_MMU)
				pos = dest->ಉದ್ದಳತೆ;
			ಆದರೆ (!ಕೂಡಿಸು_till_closing_bracket(dest, ಒಳಹರಿವು, ')' | DOUBLE_CLOSE_CHAR_FLAG))
				ಹಿಂತಿರುಗಿ 0; /* error */
			ಆದರೆ (as_string) {
				o_addstr(as_string, dest->ದತ್ತಾಂಶ + pos);
				o_addchr(as_string, ')');
				o_addchr(as_string, ')');
			}
			o_addchr(dest, SPECIAL_VAR_ಚಿಹ್ನೆ);
			ನಿಲ್ಲಿಸು;
		}
# endif
# ಆದರೆ ENABLE_HUSH_TICK
		o_addchr(dest, SPECIAL_VAR_ಚಿಹ್ನೆ);
		o_addchr(dest, quote_ಮುಖವಾಡ | '`');
		ಆದರೆ (!BB_MMU)
			pos = dest->ಉದ್ದಳತೆ;
		ಆದರೆ (!ಕೂಡಿಸು_till_closing_bracket(dest, ಒಳಹರಿವು, ')'))
			ಹಿಂತಿರುಗಿ 0; /* error */
		ಆದರೆ (as_string) {
			o_addstr(as_string, dest->ದತ್ತಾಂಶ + pos);
			o_addchr(as_string, ')');
		}
		o_addchr(dest, SPECIAL_VAR_ಚಿಹ್ನೆ);
# endif
		ನಿಲ್ಲಿಸು;
	}
#ಆದರೆ_ಅಂತ್ಯ
	ಸ್ಥಿತಿ '_':
		ಹೋಗು make_var;
#ಆದರೆ 0
	/* TODO: $_: */
	/* $_ Shell or shell script name; or last argument of last command
	 * (if last command wasn't a pipe; if it was, bash sets $_ to "");
	 * but in command's env, set to full pathname used to invoke it */
		ch = i_getch(ಒಳಹರಿವು);
		nommu_addchr(as_string, ch);
		ch = i_peek_and_eat_bkslash_nl(ಒಳಹರಿವು);
		ಆದರೆ (ಅಕ್ಷರವೇ_ಅಂಕಿಯೇ(ch)) { /* it's $_name or $_123 */
			ch = '_';
			ಹೋಗು make_var1;
		}
		/* else: it's $_ */
#ಆದರೆ_ಅಂತ್ಯ
	ಪೂರ್ವನಿಯೋಜಿತ:
		o_ಕೂಡಿಸು_Qchr(dest, '$');
	}
	debug_printf_parse("parse_dollar return 1 (ok)\n");
	ಹಿಂತಿರುಗಿ 1;
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು as_string
}

#ಆದರೆ BB_MMU
#ವ್ಯಾಖ್ಯಾನಿಸು encode_string(as_string, dest, ಒಳಹರಿವು, dquote_ಕೊನೆ) \
	encode_string(dest, ಒಳಹರಿವು, dquote_ಕೊನೆ)
#ವ್ಯಾಖ್ಯಾನಿಸು as_string ಖಾಲಿ
#ಆದರೆ_ಅಂತ್ಯ
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ encode_string(o_string *as_string,
		o_string *dest,
		ರಚನೆ ಒಳ_str *ಒಳಹರಿವು,
		ಪೂರ್ಣಾಂಕ dquote_ಕೊನೆ)
{
	ಪೂರ್ಣಾಂಕ ch;
	ಪೂರ್ಣಾಂಕ ಮುಂದಿನ;

 again:
	ch = i_getch(ಒಳಹರಿವು);
	ಆದರೆ (ch != EOF)
		nommu_addchr(as_string, ch);
	ಆದರೆ (ch == dquote_ಕೊನೆ) { /* may be only '"' or EOF */
		debug_printf_parse("encode_string return 1 (ok)\n");
		ಹಿಂತಿರುಗಿ 1;
	}
	/* note: can't move it above ch == dquote_end check! */
	ಆದರೆ (ch == EOF) {
		syntax_error_unterm_ch('"');
		ಹಿಂತಿರುಗಿ 0; /* error */
	}
	ಮುಂದಿನ = '\0';
	ಆದರೆ (ch != '\n') {
		ಮುಂದಿನ = i_peek(ಒಳಹರಿವು);
	}
	debug_printf_parse("\" ch=%c (%d) escape=%d\n",
			ch, ch, !!(dest->o_expflags & EXP_FLAG_ESC_GLOB_CHARS));
	ಆದರೆ (ch == '\\') {
		ಆದರೆ (ಮುಂದಿನ == EOF) {
			/* Testcase: in interactive shell a file with
			 *  echo "unterminated string\<eof>
			 * is sourced.
			 */
			syntax_error_unterm_ch('"');
			ಹಿಂತಿರುಗಿ 0; /* error */
		}
		/* bash:
		 * "The backslash retains its special meaning [in "..."]
		 * only when followed by one of the following characters:
		 * $, `, ", \, or <newline>.  A double quote may be quoted
		 * within double quotes by preceding it with a backslash."
		 * NB: in (unquoted) heredoc, above does not apply to ",
		 * therefore we check for it by "next == dquote_end" cond.
		 */
		ಆದರೆ (ಮುಂದಿನ == dquote_ಕೊನೆ || strchr("$`\\\n", ಮುಂದಿನ)) {
			ch = i_getch(ಒಳಹರಿವು); /* eat next */
			ಆದರೆ (ch == '\n')
				ಹೋಗು again; /* skip \<newline> */
		} /* else: ch remains == '\\', and we double it below: */
		o_addqchr(dest, ch); /* \c if c is a glob char, else just c */
		nommu_addchr(as_string, ch);
		ಹೋಗು again;
	}
	ಆದರೆ (ch == '$') {
		//if (parse_dollar_squote(as_string, dest, input))
		//	goto again;
		ಆದರೆ (!parse_dollar(as_string, dest, ಒಳಹರಿವು, /*quote_mask:*/ 0x80)) {
			debug_printf_parse("encode_string return 0: "
					"parse_dollar returned 0 (error)\n");
			ಹಿಂತಿರುಗಿ 0;
		}
		ಹೋಗು again;
	}
#ಆದರೆ ENABLE_HUSH_TICK
	ಆದರೆ (ch == '`') {
		//unsigned pos = dest->length;
		o_addchr(dest, SPECIAL_VAR_ಚಿಹ್ನೆ);
		o_addchr(dest, 0x80 | '`');
		ಆದರೆ (!ಕೂಡಿಸು_till_backquote(dest, ಒಳಹರಿವು, /*in_dquote:*/ dquote_ಕೊನೆ == '"'))
			ಹಿಂತಿರುಗಿ 0; /* error */
		o_addchr(dest, SPECIAL_VAR_ಚಿಹ್ನೆ);
		//debug_printf_subst("SUBST RES3 '%s'\n", dest->data + pos);
		ಹೋಗು again;
	}
#ಆದರೆ_ಅಂತ್ಯ
	o_ಕೂಡಿಸು_Qchr(dest, ch);
	ಆದರೆ (ch == SPECIAL_VAR_ಚಿಹ್ನೆ) {
		/* Convert "^C" to corresponding special variable reference */
		o_addchr(dest, SPECIAL_VAR_QUOTED_SVS);
		o_addchr(dest, SPECIAL_VAR_ಚಿಹ್ನೆ);
	}
	ಹೋಗು again;
#ವ್ಯಾಖ್ಯಾನ_ರದ್ದು as_string
}

/*
 * Scan input until EOF or end_trigger char.
 * Return a list of pipes to execute, or NULL on EOF
 * or if end_trigger character is met.
 * On syntax error, exit if shell is not interactive,
 * reset parsing machinery and start parsing anew,
 * or return ERR_PTR.
 */
ಸ್ಥಿರ ರಚನೆ pipe *parse_ಹರಿವು(ಅಕ್ಷರ **pstring,
		ಪೂರ್ಣಾಂಕ *heredoc_cnt_ptr,
		ರಚನೆ ಒಳ_str *ಒಳಹರಿವು,
		ಪೂರ್ಣಾಂಕ ಕೊನೆ_trigger)
{
	ರಚನೆ parse_context ctx;
	ಪೂರ್ಣಾಂಕ heredoc_cnt;

	/* Single-quote triggers a bypass of the main loop until its mate is
	 * found.  When recursing, quote state is passed in via ctx.word.o_expflags.
	 */
	debug_printf_parse("parse_stream entered, end_trigger='%c'\n",
			ಕೊನೆ_trigger ? ಕೊನೆ_trigger : 'X');
	debug_enter();

	initialize_context(&ctx);

	/* If very first arg is "" or '', ctx.word.data may end up NULL.
	 * Preventing this:
	 */
	ctx.ಶಬ್ದ.ದತ್ತಾಂಶ = xzalloc(1); /* start as "", not as NULL */

	/* We used to separate words on $IFS here. This was wrong.
	 * $IFS is used only for word splitting when $var is expanded,
	 * here we should use blank chars as separators, not $IFS
	 */

	heredoc_cnt = 0;
	ಆಗ (1) {
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *is_blank;
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *is_special;
		ಪೂರ್ಣಾಂಕ ch;
		ಪೂರ್ಣಾಂಕ ಮುಂದಿನ;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
		ಪೂರ್ಣಾಂಕ redir_fd;
		redir_type redir_style;
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */

		ch = i_getch(ಒಳಹರಿವು);
		debug_printf_parse(": ch=%c (%d) escape=%d\n",
				ch, ch, !!(ctx.ಶಬ್ದ.o_expflags & EXP_FLAG_ESC_GLOB_CHARS));
		ಆದರೆ (ch == EOF) {
			ರಚನೆ pipe *pi;

			ಆದರೆ (heredoc_cnt) {
				syntax_error_unterm_str("here document");
				ಹೋಗು parse_error_exitcode1;
			}
			ಆದರೆ (ಕೊನೆ_trigger == ')') {
				syntax_error_unterm_ch('(');
				ಹೋಗು parse_error_exitcode1;
			}
			ಆದರೆ (ಕೊನೆ_trigger == '}') {
				syntax_error_unterm_ch('{');
				ಹೋಗು parse_error_exitcode1;
			}

			ಆದರೆ (ಮುಗಿದ_ಶಬ್ದ(&ctx)) {
				ಹೋಗು parse_error_exitcode1;
			}
			o_free_and_ಗಣ_NULL(&ctx.ಶಬ್ದ);
			ಮುಗಿದ_pipe(&ctx, PIPE_SEQ);

			/* Do we sit inside of any if's, loops or case's? */
			ಆದರೆ (HAS_KEYWORDS
			IF_HAS_KEYWORDS(&& (ctx.ctx_res_w != RES_NONE || ctx.old_flag != 0))
			) {
				syntax_error_unterm_str("compound statement");
				ಹೋಗು parse_error_exitcode1;
			}

			pi = ctx.list_ತಲೆ;
			/* If we got nothing... */
			/* (this makes bare "&" cmd a no-op.
			 * bash says: "syntax error near unexpected token '&'") */
			ಆದರೆ (pi->ಅಂಕೆ_cmds == 0
			IF_HAS_KEYWORDS(&& pi->res_ಶಬ್ದ == RES_NONE)
			) {
				free_pipe_list(pi);
				pi = ಖಾಲಿ;
			}
#ಆದರೆ !BB_MMU
			debug_printf_parse("as_string1 '%s'\n", ctx.as_string.ದತ್ತಾಂಶ);
			ಆದರೆ (pstring)
				*pstring = ctx.as_string.ದತ್ತಾಂಶ;
			ಇಲ್ಲದಿದ್ದರೆ
				o_free(&ctx.as_string);
#ಆದರೆ_ಅಂತ್ಯ
			// heredoc_cnt must be 0 here anyway
			//if (heredoc_cnt_ptr)
			//	*heredoc_cnt_ptr = heredoc_cnt;
			debug_leave();
			debug_printf_heredoc("parse_stream return heredoc_cnt:%d\n", heredoc_cnt);
			debug_printf_parse("parse_stream return %p: EOF\n", pi);
			ಹಿಂತಿರುಗಿ pi;
		}

		/* Handle "'" and "\" first, as they won't play nice with
		 * i_peek_and_eat_bkslash_nl() anyway:
		 *   echo z\\
		 * and
		 *   echo '\
		 *   '
		 * would break.
		 */
		ಆದರೆ (ch == '\\') {
			ch = i_getch(ಒಳಹರಿವು);
			ಆದರೆ (ch == '\n')
				ಮುಂದುವರಿ; /* drop \<newline>, get next char */
			nommu_addchr(&ctx.as_string, '\\');
			ಆದರೆ (ch == SPECIAL_VAR_ಚಿಹ್ನೆ) {
				nommu_addchr(&ctx.as_string, ch);
				/* Convert \^C to corresponding special variable reference */
				ಹೋಗು case_SPECIAL_VAR_ಚಿಹ್ನೆ;
			}
			o_addchr(&ctx.ಶಬ್ದ, '\\');
			ಆದರೆ (ch == EOF) {
				/* Testcase: eval 'echo Ok\' */
				/* bash-4.3.43 was removing backslash,
				 * but 4.4.19 retains it, most other shells too
				 */
				ಮುಂದುವರಿ; /* get next char */
			}
			/* Example: echo Hello \2>file
			 * we need to know that word 2 is quoted
			 */
			ctx.ಶಬ್ದ.has_quoted_part = 1;
			nommu_addchr(&ctx.as_string, ch);
			o_addchr(&ctx.ಶಬ್ದ, ch);
			ಮುಂದುವರಿ; /* get next char */
		}
		nommu_addchr(&ctx.as_string, ch);
		ಆದರೆ (ch == '\'') {
			ctx.ಶಬ್ದ.has_quoted_part = 1;
			ಮುಂದಿನ = i_getch(ಒಳಹರಿವು);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
			ಆದರೆ (ಮುಂದಿನ == '\'' && !ctx.pending_redirect)
				ಹೋಗು ಸೇರಿಸು_ಖಾಲಿಗಳು_quoted_str_marker;
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */

			ch = ಮುಂದಿನ;
			ಆಗ (1) {
				ಆದರೆ (ch == EOF) {
					syntax_error_unterm_ch('\'');
					ಹೋಗು parse_error_exitcode1;
				}
				nommu_addchr(&ctx.as_string, ch);
				ಆದರೆ (ch == '\'')
					ನಿಲ್ಲಿಸು;
				ಆದರೆ (ch == SPECIAL_VAR_ಚಿಹ್ನೆ) {
					/* Convert raw ^C to corresponding special variable reference */
					o_addchr(&ctx.ಶಬ್ದ, SPECIAL_VAR_ಚಿಹ್ನೆ);
					o_addchr(&ctx.ಶಬ್ದ, SPECIAL_VAR_QUOTED_SVS);
				}
				o_addqchr(&ctx.ಶಬ್ದ, ch);
				ch = i_getch(ಒಳಹರಿವು);
			}
			ಮುಂದುವರಿ; /* get next char */
		}

		ಮುಂದಿನ = '\0';
		ಆದರೆ (ch != '\n')
			ಮುಂದಿನ = i_peek_and_eat_bkslash_nl(ಒಳಹರಿವು);

		is_special = "{}<>&|();#" /* special outside of "str" */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
				"$\"" IF_HUSH_TICK("`") /* always special */
#ಇಲ್ಲದಿದ್ದರೆ /* __U_BOOT__ */
				"$\""
#ಆದರೆ_ಅಂತ್ಯ /* __U_BOOT__ */
				SPECIAL_VAR_ಚಿಹ್ನೆ_STR;
#ಆದರೆ defined(CMD_TEST2_SINGLEWORD_NOGLOB)
		ಆದರೆ (ctx.command->cmd_type == CMD_TEST2_SINGLEWORD_NOGLOB) {
			/* In [[ ]], {}<>&|() are not special */
			is_special += 8;
		} ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
		/* Are { and } special here? */
		ಆದರೆ (ctx.command->argv /* word [word]{... - non-special */
		 || ctx.ಶಬ್ದ.ಉದ್ದಳತೆ       /* word{... - non-special */
		 || ctx.ಶಬ್ದ.has_quoted_part     /* ""{... - non-special */
		 || (ಮುಂದಿನ != ';'             /* }; - special */
		    && ಮುಂದಿನ != ')'           /* }) - special */
		    && ಮುಂದಿನ != '('           /* {( - special */
		    && ಮುಂದಿನ != '&'           /* }& and }&& ... - special */
		    && ಮುಂದಿನ != '|'           /* }|| ... - special */
		    && !strchr(defifs, ಮುಂದಿನ) /* {word - non-special */
		    )
		) {
			/* They are not special, skip "{}" */
			is_special += 2;
		}
		is_special = strchr(is_special, ch);
		is_blank = strchr(defifs, ch);

		ಆದರೆ (!is_special && !is_blank) { /* ordinary char */
 ordinary_char:
			o_ಕೂಡಿಸು_Qchr(&ctx.ಶಬ್ದ, ch);
			ಆದರೆ ((ctx.is_assignment == MAYBE_ASSIGNMENT
			    || ctx.is_assignment == ಶಬ್ದ_IS_KEYWORD)
			 && ch == '='
			 && endofname(ctx.ಶಬ್ದ.ದತ್ತಾಂಶ)[0] == '='
			) {
				ctx.is_assignment = DEFINITELY_ASSIGNMENT;
				debug_printf_parse("ctx.is_assignment='%s'\n", assignment_flag[ctx.is_assignment]);
			}
			ಮುಂದುವರಿ;
		}

		ಆದರೆ (is_blank) {
#ಆದರೆ ENABLE_HUSH_LINENO_VAR
/* Case:
 * "while ...; do<whitespace><newline>
 *	cmd ..."
 * would think that "cmd" starts in <whitespace> -
 * i.e., at the previous line.
 * We need to skip all whitespace before newlines.
 */
			ಆಗ (ch != '\n') {
				ಮುಂದಿನ = i_peek(ಒಳಹರಿವು);
				ಆದರೆ (ಮುಂದಿನ != ' ' && ಮುಂದಿನ != '\t' && ಮುಂದಿನ != '\n')
					ನಿಲ್ಲಿಸು; /* next char is not ws */
				ch = i_getch(ಒಳಹರಿವು);
			}
			/* ch == last eaten whitespace char */
#ಆದರೆ_ಅಂತ್ಯ
			ಆದರೆ (ಮುಗಿದ_ಶಬ್ದ(&ctx)) {
				ಹೋಗು parse_error_exitcode1;
			}
			ಆದರೆ (ch == '\n') {
				/* Is this a case when newline is simply ignored?
				 * Some examples:
				 * "cmd | <newline> cmd ..."
				 * "case ... in <newline> word) ..."
				 */
				ಆದರೆ (IS_NULL_CMD(ctx.command)
				 && ctx.ಶಬ್ದ.ಉದ್ದಳತೆ == 0
				 && !ctx.ಶಬ್ದ.has_quoted_part
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
					ರಚನೆ pipe *pi = ctx.list_ತಲೆ;
					ಆದರೆ (pi->ಅಂಕೆ_cmds != 0       /* check #1 */
					 && pi->followup != PIPE_BG /* check #2 */
					) {
						ಮುಂದುವರಿ;
					}
				}
				/* Treat newline as a command separator. */
				ಮುಗಿದ_pipe(&ctx, PIPE_SEQ);
				debug_printf_heredoc("heredoc_cnt:%d\n", heredoc_cnt);
				ಆದರೆ (heredoc_cnt) {
					heredoc_cnt = fetch_heredocs(&ctx.as_string, ctx.list_ತಲೆ, heredoc_cnt, ಒಳಹರಿವು);
					ಆದರೆ (heredoc_cnt != 0)
						ಹೋಗು parse_error_exitcode1;
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
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
		ಆದರೆ (ch == '}') {
#ಇಲ್ಲದಿದ್ದರೆ /* __U_BOOT__ */
		ಆದರೆ (ch == '}' || ch == ')') {
#ಆದರೆ_ಅಂತ್ಯ /* __U_BOOT__ */
			ಆದರೆ (ctx.ಶಬ್ದ.ಉದ್ದಳತೆ != 0 /* word} */
			 || ctx.ಶಬ್ದ.has_quoted_part    /* ""} */
			) {
				ಹೋಗು ordinary_char;
			}
			ಆದರೆ (!IS_NULL_CMD(ctx.command)) { /* cmd } */
				/* Generally, there should be semicolon: "cmd; }"
				 * However, bash allows to omit it if "cmd" is
				 * a group. Examples:
				 * { { echo 1; } }
				 * {(echo 1)}
				 * { echo 0 >&2 | { echo 1; } }
				 * { while false; do :; done }
				 * { case a in b) ;; esac }
				 */
				ಆದರೆ (ctx.command->ಗುಂಪು)
					ಹೋಗು ಪದ_ಗುಂಪು;
				ಹೋಗು ordinary_char;
			}
			ಆದರೆ (!IS_NULL_PIPE(ctx.pipe)) /* cmd | } */
				/* Can't be an end of {cmd}, skip the check */
				ಹೋಗು skip_ಕೊನೆ_trigger;
			/* else: } does terminate a group */
		}
 ಪದ_ಗುಂಪು:
		ಆದರೆ (ಕೊನೆ_trigger && ಕೊನೆ_trigger == ch
		 && (ch != ';' || heredoc_cnt == 0)
#ಆದರೆ ENABLE_HUSH_CASE
		 && (ch != ')'
		    || ctx.ctx_res_w != RES_ಹೊಂದಿಕೆ
		    || (!ctx.ಶಬ್ದ.has_quoted_part && strcmp(ctx.ಶಬ್ದ.ದತ್ತಾಂಶ, "esac") == 0)
		    )
#ಆದರೆ_ಅಂತ್ಯ
		) {
			ಆದರೆ (ಮುಗಿದ_ಶಬ್ದ(&ctx)) {
				ಹೋಗು parse_error_exitcode1;
			}
			ಮುಗಿದ_pipe(&ctx, PIPE_SEQ);
			ctx.is_assignment = MAYBE_ASSIGNMENT;
			debug_printf_parse("ctx.is_assignment='%s'\n", assignment_flag[ctx.is_assignment]);
			/* Do we sit outside of any if's, loops or case's? */
			ಆದರೆ (!HAS_KEYWORDS
			IF_HAS_KEYWORDS(|| (ctx.ctx_res_w == RES_NONE && ctx.old_flag == 0))
			) {
				o_free_and_ಗಣ_NULL(&ctx.ಶಬ್ದ);
#ಆದರೆ !BB_MMU
				debug_printf_parse("as_string2 '%s'\n", ctx.as_string.ದತ್ತಾಂಶ);
				ಆದರೆ (pstring)
					*pstring = ctx.as_string.ದತ್ತಾಂಶ;
				ಇಲ್ಲದಿದ್ದರೆ
					o_free(&ctx.as_string);
#ಆದರೆ_ಅಂತ್ಯ
				ಆದರೆ (ch != ';' && IS_NULL_PIPE(ctx.list_ತಲೆ)) {
					/* Example: bare "{ }", "()" */
					G.ಕೊನೆಯ_exitcode = 2; /* bash compat */
					syntax_error_unexpected_ch(ch);
					ಹೋಗು parse_error;
				}
				ಆದರೆ (heredoc_cnt_ptr)
					*heredoc_cnt_ptr = heredoc_cnt;
				debug_printf_heredoc("parse_stream return heredoc_cnt:%d\n", heredoc_cnt);
				debug_printf_parse("parse_stream return %p: "
						"end_trigger char found\n",
						ctx.list_ತಲೆ);
				debug_leave();
				ಹಿಂತಿರುಗಿ ctx.list_ತಲೆ;
			}
		}

		ಆದರೆ (is_blank)
			ಮುಂದುವರಿ;

		/* Catch <, > before deciding whether this word is
		 * an assignment. a=1 2>z b=2: b=2 is still assignment */
		ಆಯ್ಕೆ (ch) {
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
		ಸ್ಥಿತಿ '>':
			redir_fd = redirect_opt_ಅಂಕೆ(&ctx.ಶಬ್ದ);
			ಆದರೆ (ಮುಗಿದ_ಶಬ್ದ(&ctx)) {
				ಹೋಗು parse_error_exitcode1;
			}
			redir_style = REDIRECT_OVERWRITE;
			ಆದರೆ (ಮುಂದಿನ == '>') {
				redir_style = REDIRECT_APPEND;
				ch = i_getch(ಒಳಹರಿವು);
				nommu_addchr(&ctx.as_string, ch);
			}
#ಆದರೆ 0
			ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಮುಂದಿನ == '(') {
				syntax_error(">(process) not supported");
				ಹೋಗು parse_error_exitcode1;
			}
#ಆದರೆ_ಅಂತ್ಯ
			ಆದರೆ (parse_redirect(&ctx, redir_fd, redir_style, ಒಳಹರಿವು))
				ಹೋಗು parse_error_exitcode1;
			ಮುಂದುವರಿ; /* get next char */
		ಸ್ಥಿತಿ '<':
			redir_fd = redirect_opt_ಅಂಕೆ(&ctx.ಶಬ್ದ);
			ಆದರೆ (ಮುಗಿದ_ಶಬ್ದ(&ctx)) {
				ಹೋಗು parse_error_exitcode1;
			}
			redir_style = REDIRECT_ಒಳಹರಿವು;
			ಆದರೆ (ಮುಂದಿನ == '<') {
				redir_style = REDIRECT_HEREDOC;
				heredoc_cnt++;
				debug_printf_heredoc("++heredoc_cnt=%d\n", heredoc_cnt);
				ch = i_getch(ಒಳಹರಿವು);
				nommu_addchr(&ctx.as_string, ch);
			} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಮುಂದಿನ == '>') {
				redir_style = REDIRECT_IO;
				ch = i_getch(ಒಳಹರಿವು);
				nommu_addchr(&ctx.as_string, ch);
			}
#ಆದರೆ 0
			ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಮುಂದಿನ == '(') {
				syntax_error("<(process) not supported");
				ಹೋಗು parse_error_exitcode1;
			}
#ಆದರೆ_ಅಂತ್ಯ
			ಆದರೆ (parse_redirect(&ctx, redir_fd, redir_style, ಒಳಹರಿವು))
				ಹೋಗು parse_error_exitcode1;
			ಮುಂದುವರಿ; /* get next char */
#ಇಲ್ಲದಿದ್ದರೆ /* __U_BOOT__ */
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
			ಸ್ಥಿತಿ '>':
				fallthrough;
			ಸ್ಥಿತಿ '<':
				o_ಕೂಡಿಸು_Qchr(&ctx.ಶಬ್ದ, ch);
				ಮುಂದುವರಿ;
#ಆದರೆ_ಅಂತ್ಯ /* __U_BOOT__ */
		ಸ್ಥಿತಿ '#':
			ಆದರೆ (ctx.ಶಬ್ದ.ಉದ್ದಳತೆ == 0 && !ctx.ಶಬ್ದ.has_quoted_part) {
				/* skip "#comment" */
				/* note: we do not add it to &ctx.as_string */
/* TODO: in bash:
 * comment inside $() goes to the next \n, even inside quoted string (!):
 * cmd "$(cmd2 #comment)" - syntax error
 * cmd "`cmd2 #comment`" - ok
 * We accept both (comment ends where command subst ends, in both cases).
 */
				ಆಗ (1) {
					ch = i_peek(ಒಳಹರಿವು);
					ಆದರೆ (ch == '\n') {
						nommu_addchr(&ctx.as_string, '\n');
						ನಿಲ್ಲಿಸು;
					}
					ch = i_getch(ಒಳಹರಿವು);
					ಆದರೆ (ch == EOF)
						ನಿಲ್ಲಿಸು;
				}
				ಮುಂದುವರಿ; /* get next char */
			}
			ನಿಲ್ಲಿಸು;
		}
 skip_ಕೊನೆ_trigger:

		ಆದರೆ (ctx.is_assignment == MAYBE_ASSIGNMENT
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
		 /* check that we are not in word in "a=1 2>word b=1": */
		 && !ctx.pending_redirect
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */
		) {
			/* ch is a special char and thus this word
			 * cannot be an assignment */
			ctx.is_assignment = NOT_ASSIGNMENT;
			debug_printf_parse("ctx.is_assignment='%s'\n", assignment_flag[ctx.is_assignment]);
		}

		/* Note: nommu_addchr(&ctx.as_string, ch) is already done */

		ಆಯ್ಕೆ (ch) {
		case_SPECIAL_VAR_ಚಿಹ್ನೆ:
		ಸ್ಥಿತಿ SPECIAL_VAR_ಚಿಹ್ನೆ:
			/* Convert raw ^C to corresponding special variable reference */
			o_addchr(&ctx.ಶಬ್ದ, SPECIAL_VAR_ಚಿಹ್ನೆ);
			o_addchr(&ctx.ಶಬ್ದ, SPECIAL_VAR_QUOTED_SVS);
			/* fall through */
		ಸ್ಥಿತಿ '#':
			/* non-comment #: "echo a#b" etc */
			o_addchr(&ctx.ಶಬ್ದ, ch);
			ಮುಂದುವರಿ; /* get next char */
		ಸ್ಥಿತಿ '$':
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
			ಆದರೆ (parse_dollar_squote(&ctx.as_string, &ctx.ಶಬ್ದ, ಒಳಹರಿವು))
				ಮುಂದುವರಿ; /* get next char */
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */
			ಆದರೆ (!parse_dollar(&ctx.as_string, &ctx.ಶಬ್ದ, ಒಳಹರಿವು, /*quote_mask:*/ 0)) {
				debug_printf_parse("parse_stream parse error: "
					"parse_dollar returned 0 (error)\n");
				ಹೋಗು parse_error_exitcode1;
			}
			ಮುಂದುವರಿ; /* get next char */
		ಸ್ಥಿತಿ '"':
			ctx.ಶಬ್ದ.has_quoted_part = 1;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
			ಆದರೆ (ಮುಂದಿನ == '"' && !ctx.pending_redirect) {
#ಇಲ್ಲದಿದ್ದರೆ /* __U_BOOT__ */
			ಆದರೆ (ಮುಂದಿನ == '"') {
#ಆದರೆ_ಅಂತ್ಯ /* __U_BOOT__ */
				i_getch(ಒಳಹರಿವು); /* eat second " */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
 ಸೇರಿಸು_ಖಾಲಿಗಳು_quoted_str_marker:
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */
				nommu_addchr(&ctx.as_string, ಮುಂದಿನ);
				o_addchr(&ctx.ಶಬ್ದ, SPECIAL_VAR_ಚಿಹ್ನೆ);
				o_addchr(&ctx.ಶಬ್ದ, SPECIAL_VAR_ಚಿಹ್ನೆ);
				ಮುಂದುವರಿ; /* get next char */
			}
			ಆದರೆ (ctx.is_assignment == NOT_ASSIGNMENT)
				ctx.ಶಬ್ದ.o_expflags |= EXP_FLAG_ESC_GLOB_CHARS;
			ಆದರೆ (!encode_string(&ctx.as_string, &ctx.ಶಬ್ದ, ಒಳಹರಿವು, '"'))
				ಹೋಗು parse_error_exitcode1;
			ctx.ಶಬ್ದ.o_expflags &= ~EXP_FLAG_ESC_GLOB_CHARS;
			ಮುಂದುವರಿ; /* get next char */
#ಆದರೆ ENABLE_HUSH_TICK
		ಸ್ಥಿತಿ '`': {
			USE_FOR_NOMMU(ಚಿಹ್ನೆರಹಿತ pos;)

			o_addchr(&ctx.ಶಬ್ದ, SPECIAL_VAR_ಚಿಹ್ನೆ);
			o_addchr(&ctx.ಶಬ್ದ, '`');
			USE_FOR_NOMMU(pos = ctx.ಶಬ್ದ.ಉದ್ದಳತೆ;)
			ಆದರೆ (!ಕೂಡಿಸು_till_backquote(&ctx.ಶಬ್ದ, ಒಳಹರಿವು, /*in_dquote:*/ 0))
				ಹೋಗು parse_error_exitcode1;
# ಆದರೆ !BB_MMU
			o_addstr(&ctx.as_string, ctx.ಶಬ್ದ.ದತ್ತಾಂಶ + pos);
			o_addchr(&ctx.as_string, '`');
# endif
			o_addchr(&ctx.ಶಬ್ದ, SPECIAL_VAR_ಚಿಹ್ನೆ);
			//debug_printf_subst("SUBST RES3 '%s'\n", ctx.word.data + pos);
			ಮುಂದುವರಿ; /* get next char */
		}
#ಆದರೆ_ಅಂತ್ಯ
		ಸ್ಥಿತಿ ';':
#ಆದರೆ ENABLE_HUSH_CASE
 case_semi:
#ಆದರೆ_ಅಂತ್ಯ
			ಆದರೆ (ಮುಗಿದ_ಶಬ್ದ(&ctx)) {
				ಹೋಗು parse_error_exitcode1;
			}
			ಮುಗಿದ_pipe(&ctx, PIPE_SEQ);
#ಆದರೆ ENABLE_HUSH_CASE
			/* Eat multiple semicolons, detect
			 * whether it means something special */
			ಆಗ (1) {
				ch = i_peek_and_eat_bkslash_nl(ಒಳಹರಿವು);
				ಆದರೆ (ch != ';')
					ನಿಲ್ಲಿಸು;
				ch = i_getch(ಒಳಹರಿವು);
				nommu_addchr(&ctx.as_string, ch);
				ಆದರೆ (ctx.ctx_res_w == RES_CASE_BODY) {
					ctx.ctx_dsemicolon = 1;
					ctx.ctx_res_w = RES_ಹೊಂದಿಕೆ;
					ನಿಲ್ಲಿಸು;
				}
			}
#ಆದರೆ_ಅಂತ್ಯ
 new_cmd:
			/* We just finished a cmd. New one may start
			 * with an assignment */
			ctx.is_assignment = MAYBE_ASSIGNMENT;
			debug_printf_parse("ctx.is_assignment='%s'\n", assignment_flag[ctx.is_assignment]);
			ಮುಂದುವರಿ; /* get next char */
		ಸ್ಥಿತಿ '&':
			ಆದರೆ (ಮುಗಿದ_ಶಬ್ದ(&ctx)) {
				ಹೋಗು parse_error_exitcode1;
			}
			ಆದರೆ (ಮುಂದಿನ == '&') {
				ch = i_getch(ಒಳಹರಿವು);
				nommu_addchr(&ctx.as_string, ch);
				ಮುಗಿದ_pipe(&ctx, PIPE_AND);
			} ಇಲ್ಲದಿದ್ದರೆ {
				ಮುಗಿದ_pipe(&ctx, PIPE_BG);
			}
			ಹೋಗು new_cmd;
		ಸ್ಥಿತಿ '|':
			ಆದರೆ (ಮುಗಿದ_ಶಬ್ದ(&ctx)) {
				ಹೋಗು parse_error_exitcode1;
			}
#ಆದರೆ ENABLE_HUSH_CASE
			ಆದರೆ (ctx.ctx_res_w == RES_ಹೊಂದಿಕೆ)
				ನಿಲ್ಲಿಸು; /* we are in case's "word | word)" */
#ಆದರೆ_ಅಂತ್ಯ
			ಆದರೆ (ಮುಂದಿನ == '|') { /* || */
				ch = i_getch(ಒಳಹರಿವು);
				nommu_addchr(&ctx.as_string, ch);
				ಮುಗಿದ_pipe(&ctx, PIPE_OR);
			} ಇಲ್ಲದಿದ್ದರೆ {
				/* we could pick up a file descriptor choice here
				 * with redirect_opt_num(), but bash doesn't do it.
				 * "echo foo 2| cat" yields "foo 2". */
				ಮುಗಿದ_command(&ctx);
			}
			ಹೋಗು new_cmd;
		ಸ್ಥಿತಿ '(':
#ಆದರೆ ENABLE_HUSH_CASE
			/* "case... in [(]word)..." - skip '(' */
			ಆದರೆ (ctx.ctx_res_w == RES_ಹೊಂದಿಕೆ
			 && ctx.command->argv == ಖಾಲಿ /* not (word|(... */
			 && ctx.ಶಬ್ದ.ಉದ್ದಳತೆ == 0 /* not word(... */
			 && ctx.ಶಬ್ದ.has_quoted_part == 0 /* not ""(... */
			) {
				ಮುಂದುವರಿ; /* get next char */
			}
#ಆದರೆ_ಅಂತ್ಯ
			/* fall through */
		ಸ್ಥಿತಿ '{': {
			ಪೂರ್ಣಾಂಕ n = parse_ಗುಂಪು(&ctx, ಒಳಹರಿವು, ch);
			ಆದರೆ (n < 0) {
				ಹೋಗು parse_error_exitcode1;
			}
			debug_printf_heredoc("parse_group done, needs heredocs:%d\n", n);
			heredoc_cnt += n;
			ಹೋಗು new_cmd;
		}
		ಸ್ಥಿತಿ ')':
#ಆದರೆ ENABLE_HUSH_CASE
			ಆದರೆ (ctx.ctx_res_w == RES_ಹೊಂದಿಕೆ)
				ಹೋಗು case_semi;
#ಆದರೆ_ಅಂತ್ಯ
		ಸ್ಥಿತಿ '}':
			/* proper use of this character is caught by end_trigger:
			 * if we see {, we call parse_group(..., end_trigger='}')
			 * and it will match } earlier (not here). */
			G.ಕೊನೆಯ_exitcode = 2;
			syntax_error_unexpected_ch(ch);
			ಹೋಗು parse_error;
		ಪೂರ್ವನಿಯೋಜಿತ:
			ಆದರೆ (HUSH_DEBUG)
				bb_error_ಸಂದೇಶ_and_die("BUG: unexpected %c", ch);
		}
	} /* while (1) */

 parse_error_exitcode1:
	G.ಕೊನೆಯ_exitcode = 1;
 parse_error:
	{
		ರಚನೆ parse_context *pctx;
		IF_HAS_KEYWORDS(ರಚನೆ parse_context *p2;)

		/* Clean up allocated tree.
		 * Sample for finding leaks on syntax error recovery path.
		 * Run it from interactive shell, watch pmap `pidof hush`.
		 * while if false; then false; fi; do break; fi
		 * Samples to catch leaks at execution:
		 * while if (true | { true;}); then echo ok; fi; do break; done
		 * while if (true | { true;}); then echo ok; fi; do (if echo ok; break; then :; fi) | cat; break; done
		 */
		pctx = &ctx;
		ಮಾಡು {
			/* Update pipe/command counts,
			 * otherwise freeing may miss some */
			ಮುಗಿದ_pipe(pctx, PIPE_SEQ);
			debug_printf_clean("freeing list %p from ctx %p\n",
					pctx->list_ತಲೆ, pctx);
			debug_print_ವೃಕ್ಷ(pctx->list_ತಲೆ, 0);
			free_pipe_list(pctx->list_ತಲೆ);
			debug_printf_clean("freed list %p\n", pctx->list_ತಲೆ);
#ಆದರೆ !BB_MMU
			o_free(&pctx->as_string);
#ಆದರೆ_ಅಂತ್ಯ
			IF_HAS_KEYWORDS(p2 = pctx->ರಾಶಿ;)
			ಆದರೆ (pctx != &ctx) {
				ಬಿಡುಗಡೆ(pctx);
			}
			IF_HAS_KEYWORDS(pctx = p2;)
		} ಆಗ (HAS_KEYWORDS && pctx);

		o_free(&ctx.ಶಬ್ದ);
#ಆದರೆ !BB_MMU
		ಆದರೆ (pstring)
			*pstring = ಖಾಲಿ;
#ಆದರೆ_ಅಂತ್ಯ
		debug_leave();
		ಹಿಂತಿರುಗಿ ERR_PTR;
	}
}

/*** Execution routines ***/

/* Expansion can recurse, need forward decls: */
#ಆದರೆ !BASH_ನಮೂನೆ_SUBST && !ENABLE_HUSH_CASE
#ವ್ಯಾಖ್ಯಾನಿಸು expand_string_ಗಮ್ಯ_string(str, EXP_flags, do_unbackslash) \
	expand_string_ಗಮ್ಯ_string(str)
#ಆದರೆ_ಅಂತ್ಯ
ಸ್ಥಿರ ಅಕ್ಷರ *expand_string_ಗಮ್ಯ_string(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str, ಪೂರ್ಣಾಂಕ EXP_flags, ಪೂರ್ಣಾಂಕ do_unbackslash);
#ಆದರೆ ENABLE_HUSH_TICK
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ process_command_subs(o_string *dest, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s);
#ಆದರೆ_ಅಂತ್ಯ
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ expand_vars_ಗಮ್ಯ_list(o_string *output, ಪೂರ್ಣಾಂಕ n, ಅಕ್ಷರ *arg);

/* expand_strvec_to_strvec() takes a list of strings, expands
 * all variable references within and returns a pointer to
 * a list of expanded strings, possibly with larger number
 * of strings. (Think VAR="a b"; echo $VAR).
 * This new list is allocated as a single malloc block.
 * NULL-terminated list of char* pointers is at the beginning of it,
 * followed by strings themselves.
 * Caller can deallocate entire list by single free(list). */

/* A horde of its helpers come first: */

ಸ್ಥಿರ ಶೂನ್ಯ o_addblock_duplicate_backslash(o_string *o, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str, ಪೂರ್ಣಾಂಕ len)
{
	ಆಗ (--len >= 0) {
		ಅಕ್ಷರ c = *str++;

#ಆದರೆ ENABLE_HUSH_BRACE_EXPANSION
		ಆದರೆ (c == '{' || c == '}') {
			/* { -> \{, } -> \} */
			o_addchr(o, '\\');
			/* And now we want to add { or } and continue:
			 *  o_addchr(o, c);
			 *  continue;
			 * luckily, just falling through achieves this.
			 */
		}
#ಆದರೆ_ಅಂತ್ಯ
		o_addchr(o, c);
		ಆದರೆ (c == '\\') {
			/* \z -> \\\z; \<eol> -> \\<eol> */
			o_addchr(o, '\\');
			ಆದರೆ (len) {
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ expand_on_ifs(o_string *output, ಪೂರ್ಣಾಂಕ n, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str)
{
	ಪೂರ್ಣಾಂಕ ಕೊನೆಯ_is_ifs = 0;

	ಆಗ (1) {
		ಪೂರ್ಣಾಂಕ ಶಬ್ದ_len;

		ಆದರೆ (!*str)  /* EOL - do not finalize word */
			ನಿಲ್ಲಿಸು;
		ಶಬ್ದ_len = strcspn(str, G.ifs);
		ಆದರೆ (ಶಬ್ದ_len) {
			/* We have WORD_LEN leading non-IFS chars */
			ಆದರೆ (!(output->o_expflags & EXP_FLAG_GLOB)) {
				o_addblock(output, str, ಶಬ್ದ_len);
			} ಇಲ್ಲದಿದ್ದರೆ {
				/* Protect backslashes against globbing up :)
				 * Example: "v='\*'; echo b$v" prints "b\*"
				 * (and does not try to glob on "*")
				 */
				o_addblock_duplicate_backslash(output, str, ಶಬ್ದ_len);
				/*/ Why can't we do it easier? */
				/*o_addblock(output, str, word_len); - WRONG: "v='\*'; echo Z$v" prints "Z*" instead of "Z\*" */
				/*o_addqblock(output, str, word_len); - WRONG: "v='*'; echo Z$v" prints "Z*" instead of Z* files */
			}
			ಕೊನೆಯ_is_ifs = 0;
			str += ಶಬ್ದ_len;
			ಆದರೆ (!*str)  /* EOL - do not finalize word */
				ನಿಲ್ಲಿಸು;
		}

		/* We know str here points to at least one IFS char */
		ಕೊನೆಯ_is_ifs = 1;
		str += strspn(str, G.ifs_whitespace); /* skip IFS whitespace chars */
		ಆದರೆ (!*str)  /* EOL - do not finalize word */
			ನಿಲ್ಲಿಸು;

		ಆದರೆ (G.ifs_whitespace != G.ifs /* usually false ($IFS is usually all whitespace), */
		 && strchr(G.ifs, *str)       /* the second check would fail */
		) {
			/* This is a non-whitespace $IFS char */
			/* Skip it and IFS whitespace chars, start new word */
			str++;
			str += strspn(str, G.ifs_whitespace);
			ಹೋಗು new_ಶಬ್ದ;
		}

		/* Start new word... but not always! */
		/* Case "v=' a'; echo ''$v": we do need to finalize empty word: */
		ಆದರೆ (output->has_quoted_part
		/*
		 * Case "v=' a'; echo $v":
		 * here nothing precedes the space in $v expansion,
		 * therefore we should not finish the word
		 * (IOW: if there *is* word to finalize, only then do it):
		 * It's okay if this accesses the byte before first argv[]:
		 * past call to o_save_ptr() cleared it to zero byte
		 * (grep for -prev-ifs-check-).
		 */
		 || output->ದತ್ತಾಂಶ[output->ಉದ್ದಳತೆ - 1]
		) {
 new_ಶಬ್ದ:
			o_addchr(output, '\0');
			debug_print_list("expand_on_ifs", output, n);
			n = o_save_ptr(output, n);
		}
	}

	output->ended_ಒಳ_ifs = ಕೊನೆಯ_is_ifs;
	debug_print_list("expand_on_ifs[1]", output, n);
	ಹಿಂತಿರುಗಿ n;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
/* Helper to expand $((...)) and heredoc body. These act as if
 * they are in double quotes, with the exception that they are not :).
 * Just the rules are similar: "expand only $var and `cmd`"
 *
 * Returns malloced string.
 * As an optimization, we return NULL if expansion is not needed.
 */
ಸ್ಥಿರ ಅಕ್ಷರ *encode_then_expand_string(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str)
{
	ಅಕ್ಷರ *exp_str;
	ರಚನೆ ಒಳ_str ಒಳಹರಿವು;
	o_string dest = NULL_O_STRING;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *cp;

	cp = str;
	ಗಾಗಿ (;;) {
		ಆದರೆ (!*cp) ಹಿಂತಿರುಗಿ ಖಾಲಿ; /* string has no special chars */
		ಆದರೆ (*cp == '$') ನಿಲ್ಲಿಸು;
		ಆದರೆ (*cp == '\\') ನಿಲ್ಲಿಸು;
#ಆದರೆ ENABLE_HUSH_TICK
		ಆದರೆ (*cp == '`') ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ
		cp++;
	}

	/* We need to expand. Example:
	 * echo $(($a + `echo 1`)) $((1 + $((2)) ))
	 */
	ಸಿದ್ಧತೆ_string_ಒಳ_str(&ಒಳಹರಿವು, str);
	encode_string(ಖಾಲಿ, &dest, &ಒಳಹರಿವು, EOF);
//TODO: error check (encode_string returns 0 on error)?
	//bb_error_msg("'%s' -> '%s'", str, dest.data);
	exp_str = expand_string_ಗಮ್ಯ_string(dest.ದತ್ತಾಂಶ,
			EXP_FLAG_ESC_GLOB_CHARS,
			/*unbackslash:*/ 1
	);
	//bb_error_msg("'%s' -> '%s'", dest.data, exp_str);
	o_free(&dest);
	ಹಿಂತಿರುಗಿ exp_str;
}

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮೊದಲ_special_char_ಒಳ_vararg(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *cp)
{
	ಗಾಗಿ (;;) {
		ಆದರೆ (!*cp) ಹಿಂತಿರುಗಿ ಖಾಲಿ; /* string has no special chars */
		ಆದರೆ (*cp == '$') ಹಿಂತಿರುಗಿ cp;
		ಆದರೆ (*cp == '\\') ಹಿಂತಿರುಗಿ cp;
		ಆದರೆ (*cp == '\'') ಹಿಂತಿರುಗಿ cp;
		ಆದರೆ (*cp == '"') ಹಿಂತಿರುಗಿ cp;
#ಆದರೆ ENABLE_HUSH_TICK
		ಆದರೆ (*cp == '`') ಹಿಂತಿರುಗಿ cp;
#ಆದರೆ_ಅಂತ್ಯ
		/* dquoted "${x:+ARG}" should not glob, therefore
		 * '*' et al require some non-literal processing: */
		ಆದರೆ (*cp == '*') ಹಿಂತಿರುಗಿ cp;
		ಆದರೆ (*cp == '?') ಹಿಂತಿರುಗಿ cp;
		ಆದರೆ (*cp == '[') ಹಿಂತಿರುಗಿ cp;
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
#ಆದರೆ !BASH_ನಮೂನೆ_SUBST
#ವ್ಯಾಖ್ಯಾನಿಸು encode_then_expand_vararg(str, handle_squotes, do_unbackslash) \
	encode_then_expand_vararg(str, handle_squotes)
#ಆದರೆ_ಅಂತ್ಯ
ಸ್ಥಿರ ಅಕ್ಷರ *encode_then_expand_vararg(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str, ಪೂರ್ಣಾಂಕ handle_squotes, ಪೂರ್ಣಾಂಕ do_unbackslash)
{
#ಆದರೆ !BASH_ನಮೂನೆ_SUBST && ENABLE_HUSH_CASE
	ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ do_unbackslash = 0;
#ಆದರೆ_ಅಂತ್ಯ
	ಅಕ್ಷರ *exp_str;
	ರಚನೆ ಒಳ_str ಒಳಹರಿವು;
	o_string dest = NULL_O_STRING;

	ಆದರೆ (!ಮೊದಲ_special_char_ಒಳ_vararg(str)) {
		/* string has no special chars */
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}

	ಸಿದ್ಧತೆ_string_ಒಳ_str(&ಒಳಹರಿವು, str);
	dest.ದತ್ತಾಂಶ = xzalloc(1); /* start as "", not as NULL */
	exp_str = ಖಾಲಿ;

	ಗಾಗಿ (;;) {
		ಪೂರ್ಣಾಂಕ ch;

		ch = i_getch(&ಒಳಹರಿವು);
		debug_printf_parse("%s: ch=%c (%d) escape=%d\n",
				__func__, ch, ch, !!dest.o_expflags);

		ಆದರೆ (!dest.o_expflags) {
			ಆದರೆ (ch == EOF)
				ನಿಲ್ಲಿಸು;
			ಆದರೆ (handle_squotes && ch == '\'') {
				ಆದರೆ (!ಕೂಡಿಸು_till_single_quote_dquoted(&dest, &ಒಳಹರಿವು))
					ಹೋಗು ret; /* error */
				ಮುಂದುವರಿ;
			}
		}
		ಆದರೆ (ch == EOF) {
			syntax_error_unterm_ch('"');
			ಹೋಗು ret; /* error */
		}
		ಆದರೆ (ch == '"') {
			dest.o_expflags ^= EXP_FLAG_ESC_GLOB_CHARS;
			ಮುಂದುವರಿ;
		}
		ಆದರೆ (ch == '\\') {
			ch = i_getch(&ಒಳಹರಿವು);
			ಆದರೆ (ch == EOF) {
//example? error message?	syntax_error_unterm_ch('"');
				debug_printf_parse("%s: error: \\<eof>\n", __func__);
				ಹೋಗು ret;
			}
			o_addqchr(&dest, ch);
			ಮುಂದುವರಿ;
		}
		ಆದರೆ (ch == '$') {
			ಆದರೆ (parse_dollar_squote(ಖಾಲಿ, &dest, &ಒಳಹರಿವು))
				ಮುಂದುವರಿ;
			ಆದರೆ (!parse_dollar(ಖಾಲಿ, &dest, &ಒಳಹರಿವು, /*quote_mask:*/ 0x80)) {
				debug_printf_parse("%s: error: parse_dollar returned 0 (error)\n", __func__);
				ಹೋಗು ret;
			}
			ಮುಂದುವರಿ;
		}
#ಆದರೆ ENABLE_HUSH_TICK
		ಆದರೆ (ch == '`') {
			//unsigned pos = dest->length;
			o_addchr(&dest, SPECIAL_VAR_ಚಿಹ್ನೆ);
			o_addchr(&dest, 0x80 | '`');
			ಆದರೆ (!ಕೂಡಿಸು_till_backquote(&dest, &ಒಳಹರಿವು,
					/*in_dquote:*/ dest.o_expflags /* nonzero if EXP_FLAG_ESC_GLOB_CHARS set */
				)
			) {
				ಹೋಗು ret; /* error */
			}
			o_addchr(&dest, SPECIAL_VAR_ಚಿಹ್ನೆ);
			//debug_printf_subst("SUBST RES3 '%s'\n", dest->data + pos);
			ಮುಂದುವರಿ;
		}
#ಆದರೆ_ಅಂತ್ಯ
		o_ಕೂಡಿಸು_Qchr(&dest, ch);
	} /* for (;;) */

	debug_printf_parse("encode: '%s' -> '%s'\n", str, dest.ದತ್ತಾಂಶ);
	exp_str = expand_string_ಗಮ್ಯ_string(dest.ದತ್ತಾಂಶ,
			do_unbackslash ? EXP_FLAG_ESC_GLOB_CHARS : 0,
			do_unbackslash
	);
 ret:
	debug_printf_parse("expand: '%s' -> '%s'\n", dest.ದತ್ತಾಂಶ, exp_str);
	o_free(&dest);
	ಹಿಂತಿರುಗಿ exp_str;
}

/* Expanding ARG in ${var+ARG}, ${var-ARG}
 */
ಸ್ಥಿರ NOINLINE ಪೂರ್ಣಾಂಕ encode_then_append_var_plusminus(o_string *output, ಪೂರ್ಣಾಂಕ n,
		ಅಕ್ಷರ *str, ಪೂರ್ಣಾಂಕ dquoted)
{
	ರಚನೆ ಒಳ_str ಒಳಹರಿವು;
	o_string dest = NULL_O_STRING;

	ಆದರೆ (!ಮೊದಲ_special_char_ಒಳ_vararg(str)
	 && '\0' == str[strcspn(str, G.ifs)]
	) {
		/* string has no special chars
		 * && string has no $IFS chars
		 */
		ಆದರೆ (dquoted) {
			/* Prints 1 (quoted expansion is a "" word, not nothing):
			 * set -- "${notexist-}"; echo $#
			 */
			output->has_quoted_part = 1;
		}
		ಹಿಂತಿರುಗಿ expand_vars_ಗಮ್ಯ_list(output, n, str);
	}

	ಸಿದ್ಧತೆ_string_ಒಳ_str(&ಒಳಹರಿವು, str);

	ಗಾಗಿ (;;) {
		ಪೂರ್ಣಾಂಕ ch;

		ch = i_getch(&ಒಳಹರಿವು);
		debug_printf_parse("%s: ch=%c (%d) escape=%x\n",
				__func__, ch, ch, dest.o_expflags);

		ಆದರೆ (!dest.o_expflags) {
			ಆದರೆ (ch == EOF)
				ನಿಲ್ಲಿಸು;
			ಆದರೆ (!dquoted && !(output->o_expflags & EXP_FLAG_SINGLEWORD) && strchr(G.ifs, ch)) {
				/* PREFIX${x:d${e}f ...} and we met space: expand "d${e}f" and start new word.
				 * do not assume we are at the start of the word (PREFIX above).
				 */
				ಆದರೆ (dest.ದತ್ತಾಂಶ) {
					n = expand_vars_ಗಮ್ಯ_list(output, n, dest.ದತ್ತಾಂಶ);
					o_free_and_ಗಣ_NULL(&dest);
					o_addchr(output, '\0');
					n = o_save_ptr(output, n); /* create next word */
				} ಇಲ್ಲದಿದ್ದರೆ
				ಆದರೆ (output->ಉದ್ದಳತೆ != o_get_ಕೊನೆಯ_ptr(output, n)
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
				ಮುಂದುವರಿ;
			}
			ಆದರೆ (!dquoted && ch == '\'') {
				ಆದರೆ (!ಕೂಡಿಸು_till_single_quote_dquoted(&dest, &ಒಳಹರಿವು))
					ಹೋಗು ret; /* error */
				o_addchr(&dest, SPECIAL_VAR_ಚಿಹ್ನೆ);
				o_addchr(&dest, SPECIAL_VAR_ಚಿಹ್ನೆ);
				ಮುಂದುವರಿ;
			}
		}
		ಆದರೆ (ch == EOF) {
			syntax_error_unterm_ch('"');
			ಹೋಗು ret; /* error */
		}
		ಆದರೆ (ch == '"') {
			dest.o_expflags ^= EXP_FLAG_ESC_GLOB_CHARS;
			ಆದರೆ (dest.o_expflags) {
				o_addchr(&dest, SPECIAL_VAR_ಚಿಹ್ನೆ);
				o_addchr(&dest, SPECIAL_VAR_ಚಿಹ್ನೆ);
			}
			ಮುಂದುವರಿ;
		}
		ಆದರೆ (ch == '\\') {
			ch = i_getch(&ಒಳಹರಿವು);
			ಆದರೆ (ch == EOF) {
//example? error message?	syntax_error_unterm_ch('"');
				debug_printf_parse("%s: error: \\<eof>\n", __func__);
				ಹೋಗು ret;
			}
			o_addqchr(&dest, ch);
			ಮುಂದುವರಿ;
		}
		ಆದರೆ (ch == '$') {
			ಆದರೆ (!parse_dollar(ಖಾಲಿ, &dest, &ಒಳಹರಿವು, /*quote_mask:*/ (dest.o_expflags || dquoted) ? 0x80 : 0)) {
				debug_printf_parse("%s: error: parse_dollar returned 0 (error)\n", __func__);
				ಹೋಗು ret;
			}
			ಮುಂದುವರಿ;
		}
#ಆದರೆ ENABLE_HUSH_TICK
		ಆದರೆ (ch == '`') {
			//unsigned pos = dest->length;
			o_addchr(&dest, SPECIAL_VAR_ಚಿಹ್ನೆ);
			o_addchr(&dest, (dest.o_expflags || dquoted) ? 0x80 | '`' : '`');
			ಆದರೆ (!ಕೂಡಿಸು_till_backquote(&dest, &ಒಳಹರಿವು,
					/*in_dquote:*/ dest.o_expflags /* nonzero if EXP_FLAG_ESC_GLOB_CHARS set */
				)
			) {
				ಹೋಗು ret; /* error */
			}
			o_addchr(&dest, SPECIAL_VAR_ಚಿಹ್ನೆ);
			//debug_printf_subst("SUBST RES3 '%s'\n", dest->data + pos);
			ಮುಂದುವರಿ;
		}
#ಆದರೆ_ಅಂತ್ಯ
		ಆದರೆ (dquoted) {
			/* Always glob-protect if in dquotes:
			 * x=x; echo "${x:+/bin/c*}" - prints: /bin/c*
			 * x=x; echo "${x:+"/bin/c*"}" - prints: /bin/c*
			 */
			o_addqchr(&dest, ch);
		} ಇಲ್ಲದಿದ್ದರೆ {
			/* Glob-protect only if char is quoted:
			 * x=x; echo ${x:+/bin/c*} - prints many filenames
			 * x=x; echo ${x:+"/bin/c*"} - prints: /bin/c*
			 */
			o_ಕೂಡಿಸು_Qchr(&dest, ch);
		}
	} /* for (;;) */

	ಆದರೆ (dest.ದತ್ತಾಂಶ) {
		n = expand_vars_ಗಮ್ಯ_list(output, n, dest.ದತ್ತಾಂಶ);
	}
 ret:
	o_free(&dest);
	ಹಿಂತಿರುಗಿ n;
}
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
#ಆದರೆ ENABLE_FEATURE_SH_MATH
ಸ್ಥಿರ arith_t expand_and_evaluate_arith(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *arg, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ **errmsg_p)
{
	arith_state_t math_state;
	arith_t res;
	ಅಕ್ಷರ *exp_str;

	math_state.lookupvar = get_local_var_ಮೌಲ್ಯ;
	math_state.setvar = ಗಣ_local_var_from_halves;
	//math_state.endofname = endofname;
	exp_str = encode_then_expand_string(arg);
	res = arith(&math_state, exp_str ? exp_str : arg);
	ಬಿಡುಗಡೆ(exp_str);
	ಆದರೆ (errmsg_p)
		*errmsg_p = math_state.errmsg;
	ಆದರೆ (math_state.errmsg)
		ಸಂದೇಶ_and_die_if_script(math_state.errmsg);
	ಹಿಂತಿರುಗಿ res;
}
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
#ಆದರೆ BASH_ನಮೂನೆ_SUBST
/* ${var/[/]pattern[/repl]} helpers */
ಸ್ಥಿರ ಅಕ್ಷರ *strstr_ನಮೂನೆ(ಅಕ್ಷರ *val, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ನಮೂನೆ, ಪೂರ್ಣಾಂಕ *ಗಾತ್ರ_2)
{
	ಪೂರ್ಣಾಂಕ ಮೊದಲ_escaped = (ನಮೂನೆ[0] == '\\' && ನಮೂನೆ[1]);
	/* "first_escaped" trick allows to treat e.g. "\*no_glob_chars"
	 * as literal too (as it is semi-common, and easy to accomodate
	 * by just using str + 1).
	 */
	ಪೂರ್ಣಾಂಕ sz = strcspn(ನಮೂನೆ + ಮೊದಲ_escaped * 2, "*?[\\");
	ಆದರೆ ((ನಮೂನೆ + ಮೊದಲ_escaped * 2)[sz] == '\0') {
		/* Optimization for trivial patterns.
		 * Testcase for very slow replace (performs about 22k replaces):
		 * x=::::::::::::::::::::::
		 * x=$x$x;x=$x$x;x=$x$x;x=$x$x;x=$x$x;x=$x$x;x=$x$x;x=$x$x;x=$x$x;x=$x$x;echo ${#x}
		 * echo "${x//:/|}"
		 */
		*ಗಾತ್ರ_2 = sz + ಮೊದಲ_escaped;
		ಹಿಂತಿರುಗಿ ಅಕ್ಷರಸರಣಿ_ಹುಡುಕು(val, ನಮೂನೆ + ಮೊದಲ_escaped);
	}

	ಆಗ (1) {
		ಅಕ್ಷರ *ಕೊನೆ_2 = ಪರಿಶೀಲನೆ_and_ಹೊಂದಿಕೆ(val, ನಮೂನೆ, ಪರಿಶೀಲನೆ_MOVE_FROM_ಬಲ + ಪರಿಶೀಲನೆ_ಹೊಂದಿಕೆ_ಎಡ_HALF);
		debug_printf_varexp("val:'%s' pattern:'%s' end:'%s'\n", val, ನಮೂನೆ, ಕೊನೆ_2);
		ಆದರೆ (ಕೊನೆ_2) {
			*ಗಾತ್ರ_2 = ಕೊನೆ_2 - val;
			ಹಿಂತಿರುಗಿ val;
		}
		ಆದರೆ (*val == '\0')
			ಹಿಂತಿರುಗಿ ಖಾಲಿ;
		/* Optimization: if "*pat" did not match the start of "string",
		 * we know that "tring", "ring" etc will not match too:
		 */
		ಆದರೆ (ನಮೂನೆ[0] == '*')
			ಹಿಂತಿರುಗಿ ಖಾಲಿ;
		val++;
	}
}
ಸ್ಥಿರ ಅಕ್ಷರ *replace_ನಮೂನೆ(ಅಕ್ಷರ *val, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ನಮೂನೆ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *repl, ಅಕ್ಷರ exp_op)
{
	ಅಕ್ಷರ *ಫಲಿತಾಂಶ = ಖಾಲಿ;
	ಚಿಹ್ನೆರಹಿತ res_len = 0;
	ಚಿಹ್ನೆರಹಿತ repl_len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(repl);

	/* Null pattern never matches, including if "var" is empty */
	ಆದರೆ (!ನಮೂನೆ[0])
		ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ; /* NULL, no replaces happened */

	ಆಗ (1) {
		ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2;
		ಅಕ್ಷರ *s = strstr_ನಮೂನೆ(val, ನಮೂನೆ, &ಗಾತ್ರ_2);
		ಆದರೆ (!s)
			ನಿಲ್ಲಿಸು;

		ಫಲಿತಾಂಶ = xrealloc(ಫಲಿತಾಂಶ, res_len + (s - val) + repl_len + 1);
		strcpy(mempcpy(ಫಲಿತಾಂಶ + res_len, val, s - val), repl);
		res_len += (s - val) + repl_len;
		debug_printf_varexp("val:'%s' s:'%s' result:'%s'\n", val, s, ಫಲಿತಾಂಶ);

		val = s + ಗಾತ್ರ_2;
		ಆದರೆ (exp_op == '/')
			ನಿಲ್ಲಿಸು;
	}
	ಆದರೆ (*val && ಫಲಿತಾಂಶ) {
		ಫಲಿತಾಂಶ = xrealloc(ಫಲಿತಾಂಶ, res_len + ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(val) + 1);
		strcpy(ಫಲಿತಾಂಶ + res_len, val);
		debug_printf_varexp("val:'%s' result:'%s'\n", val, ಫಲಿತಾಂಶ);
	}
	debug_printf_varexp("result:'%s'\n", ಫಲಿತಾಂಶ);
	ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
}
#ಆದರೆ_ಅಂತ್ಯ /* BASH_PATTERN_SUBST */
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ append_str_maybe_ifs_ವಿಭಜಿಸು(o_string *output, ಪೂರ್ಣಾಂಕ n,
		ಪೂರ್ಣಾಂಕ ಮೊದಲ_ch, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *val)
{
	ಆದರೆ (!(ಮೊದಲ_ch & 0x80)) { /* unquoted $VAR */
		debug_printf_expand("unquoted '%s', output->o_escape:%d\n", val,
				!!(output->o_expflags & EXP_FLAG_ESC_GLOB_CHARS));
		ಆದರೆ (val && val[0])
			n = expand_on_ifs(output, n, val);
	} ಇಲ್ಲದಿದ್ದರೆ { /* quoted "$VAR" */
		output->has_quoted_part = 1;
		debug_printf_expand("quoted '%s', output->o_escape:%d\n", val,
				!!(output->o_expflags & EXP_FLAG_ESC_GLOB_CHARS));
		ಆದರೆ (val && val[0])
			o_ಕೂಡಿಸು_Qstr(output, val);
	}
	ಹಿಂತಿರುಗಿ n;
}

/* Handle <SPECIAL_VAR_SYMBOL>varname...<SPECIAL_VAR_SYMBOL> construct.
 */
ಸ್ಥಿರ NOINLINE ಪೂರ್ಣಾಂಕ expand_one_var(o_string *output, ಪೂರ್ಣಾಂಕ n,
		ಪೂರ್ಣಾಂಕ ಮೊದಲ_ch, ಅಕ್ಷರ *arg, ಅಕ್ಷರ **pp)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *val;
	ಅಕ್ಷರ *ಗಮ್ಯ_be_freed;
	ಅಕ್ಷರ *p;
	ಅಕ್ಷರ *var;
	ಅಕ್ಷರ exp_op;
	ಅಕ್ಷರ exp_save = exp_save; /* for compiler */
	ಅಕ್ಷರ *exp_saveptr; /* points to expansion operator */
	ಅಕ್ಷರ *exp_ಶಬ್ದ = exp_ಶಬ್ದ; /* for compiler */
	ಅಕ್ಷರ arg0;

	val = ಖಾಲಿ;
	ಗಮ್ಯ_be_freed = ಖಾಲಿ;
	p = *pp;
	*p = '\0'; /* replace trailing SPECIAL_VAR_SYMBOL */
	var = arg;
	exp_saveptr = arg[1] ? strchr(VAR_ENCODED_SUBST_OPS, arg[1]) : ಖಾಲಿ;
	arg0 = arg[0];
	arg[0] = (arg0 & 0x7f);
	exp_op = 0;

	ಆದರೆ (arg[0] == '#' && arg[1] /* ${#...} but not ${#} */
	 && (!exp_saveptr               /* and ( not(${#<op_char>...}) */
	    || (arg[2] == '\0' && strchr(SPECIAL_VARS_STR, arg[1])) /* or ${#C} "len of $C" ) */
	    )		/* NB: skipping ^^^specvar check mishandles ${#::2} */
	) {
		/* It must be length operator: ${#var} */
		var++;
		exp_op = 'L';
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* Maybe handle parameter expansion */
		ಆದರೆ (exp_saveptr /* if 2nd char is one of expansion operators */
		 && strchr(NUMERIC_SPECVARS_STR, arg[0]) /* 1st char is special variable */
		) {
			/* ${?:0}, ${#[:]%0} etc */
			exp_saveptr = var + 1;
		} ಇಲ್ಲದಿದ್ದರೆ {
			/* ${?}, ${var}, ${var:0}, ${var[:]%0} etc */
			exp_saveptr = var+1 + strcspn(var+1, VAR_ENCODED_SUBST_OPS);
		}
		exp_op = exp_save = *exp_saveptr;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
		ಆದರೆ (exp_op) {
			exp_ಶಬ್ದ = exp_saveptr + 1;
			ಆದರೆ (exp_op == ':') {
				exp_op = *exp_ಶಬ್ದ++;
//TODO: try ${var:} and ${var:bogus} in non-bash config
				ಆದರೆ (BASH_SUBSTR
				 && (!exp_op || !strchr(MINUS_PLUS_EQUAL_QUESTION, exp_op))
				) {
					/* oops... it's ${var:N[:M]}, not ${var:?xxx} or some such */
					exp_op = ':';
					exp_ಶಬ್ದ--;
				}
			}
			*exp_saveptr = '\0';
		} /* else: it's not an expansion op, but bare ${var} */
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */
	}

	/* Look up the variable in question */
	ಆದರೆ (ಅಂಕಿಯೇ(var[0])) {
		/* parse_dollar should have vetted var for us */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
		ಪೂರ್ಣಾಂಕ nn = xatoi_ಧನಾತ್ಮಕ(var);
#ಇಲ್ಲದಿದ್ದರೆ /* __U_BOOT__ */
		ಪೂರ್ಣಾಂಕ nn = simple_strtoul(var, ಖಾಲಿ, 10);
#ಆದರೆ_ಅಂತ್ಯ /* __U_BOOT__ */
		ಆದರೆ (nn < G.global_argc)
			val = G.global_argv[nn];
		/* else val remains NULL: $N with too big N */
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಆಯ್ಕೆ (var[0]) {
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
		ಸ್ಥಿತಿ '$': /* pid */
			val = utoa(G.ಬೇರು_pid);
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ '!': /* bg pid */
			val = G.ಕೊನೆಯ_bg_pid ? utoa(G.ಕೊನೆಯ_bg_pid) : "";
			ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */
		ಸ್ಥಿತಿ '?': /* exitcode */
			val = utoa(G.ಕೊನೆಯ_exitcode);
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ '#': /* argc */
			val = utoa(G.global_argc ? G.global_argc-1 : 0);
			ನಿಲ್ಲಿಸು;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
		ಸ್ಥಿತಿ '-': { /* active options */
			/* Check set_mode() to see what option chars we support */
			ಅಕ್ಷರ *cp;
			val = cp = G.optstring_buf;
			ಆದರೆ (G.o_opt[OPT_O_ERREXIT])
				*cp++ = 'e';
			ಆದರೆ (G_interactive_fd)
				*cp++ = 'i';
			ಆದರೆ (G_x_ಬಹುಲಕ)
				*cp++ = 'x';
			/* If G.o_opt[OPT_O_NOEXEC] is true,
			 * commands read but are not executed,
			 * so $- can not execute too, 'n' is never seen in $-.
			 */
			ಆದರೆ (G.opt_c)
				*cp++ = 'c';
			ಆದರೆ (G.opt_s)
				*cp++ = 's';
			*cp = '\0';
			ನಿಲ್ಲಿಸು;
		}
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */
		ಪೂರ್ವನಿಯೋಜಿತ:
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
			val = get_local_var_ಮೌಲ್ಯ(var);
#ಇಲ್ಲದಿದ್ದರೆ /* __U_BOOT__ */
			/*
			 * Environment variable set with setenv* have to be
			 * expanded.
			 * So, we first search if the variable exists in
			 * environment, if this is not the case, we default to
			 * local value.
			 */
			val = env_get(var);
			ಆದರೆ (!val)
				val = get_local_var_ಮೌಲ್ಯ(var);
#ಆದರೆ_ಅಂತ್ಯ /* __U_BOOT__ */
		}
	}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	/* Handle any expansions */
	ಆದರೆ (exp_op == 'L') {
		reinit_unicode_for_hush();
		debug_printf_expand("expand: length(%s)=", val);
		val = utoa(val ? unicode_strlen(val) : 0);
		debug_printf_expand("%s\n", val);
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (exp_op) {
		ಆದರೆ (exp_op == '%' || exp_op == '#') {
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
			ಆದರೆ (val && val[0]) {
				ಅಕ್ಷರ *t;
				ಅಕ್ಷರ *exp_exp_ಶಬ್ದ;
				ಅಕ್ಷರ *loc;
				ಚಿಹ್ನೆರಹಿತ ಪರಿಶೀಲನೆ_flags = pick_ಪರಿಶೀಲನೆ(exp_op, *exp_ಶಬ್ದ);
				ಆದರೆ (exp_op == *exp_ಶಬ್ದ)  /* ## or %% */
					exp_ಶಬ್ದ++;
				debug_printf_expand("expand: exp_word:'%s'\n", exp_ಶಬ್ದ);
				exp_exp_ಶಬ್ದ = encode_then_expand_vararg(exp_ಶಬ್ದ, /*handle_squotes:*/ 1, /*unbackslash:*/ 0);
				ಆದರೆ (exp_exp_ಶಬ್ದ)
					exp_ಶಬ್ದ = exp_exp_ಶಬ್ದ;
				debug_printf_expand("expand: exp_word:'%s'\n", exp_ಶಬ್ದ);
				/*
				 * HACK ALERT. We depend here on the fact that
				 * G.global_argv and results of utoa and get_local_var_value
				 * are actually in writable memory:
				 * scan_and_match momentarily stores NULs there.
				 */
				t = (ಅಕ್ಷರ*)val;
				loc = ಪರಿಶೀಲನೆ_and_ಹೊಂದಿಕೆ(t, exp_ಶಬ್ದ, ಪರಿಶೀಲನೆ_flags);
				debug_printf_expand("op:%c str:'%s' pat:'%s' res:'%s'\n", exp_op, t, exp_ಶಬ್ದ, loc);
				ಬಿಡುಗಡೆ(exp_exp_ಶಬ್ದ);
				ಆದರೆ (loc) { /* match was found */
					ಆದರೆ (ಪರಿಶೀಲನೆ_flags & ಪರಿಶೀಲನೆ_ಹೊಂದಿಕೆ_ಎಡ_HALF) /* #[#] */
						val = loc; /* take right part */
					ಇಲ್ಲದಿದ್ದರೆ /* %[%] */
						val = ಗಮ್ಯ_be_freed = xstrndup(val, loc - val); /*ಎಡ*/
				}
			}
		}
#ಆದರೆ BASH_ನಮೂನೆ_SUBST
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (exp_op == '/' || exp_op == '\\') {
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
			ಆದರೆ (val /*&& val[0]*/) {
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
				ಅಕ್ಷರ *ನಮೂನೆ, *repl, *t;
				ನಮೂನೆ = encode_then_expand_vararg(exp_ಶಬ್ದ, /*handle_squotes:*/ 1, /*unbackslash:*/ 0);
				ಆದರೆ (!ನಮೂನೆ)
					ನಮೂನೆ = xstrdup(exp_ಶಬ್ದ);
				debug_printf_varexp("pattern:'%s'->'%s'\n", exp_ಶಬ್ದ, ನಮೂನೆ);
				*p++ = SPECIAL_VAR_ಚಿಹ್ನೆ;
				exp_ಶಬ್ದ = p;
				p = strchr(p, SPECIAL_VAR_ಚಿಹ್ನೆ);
				*p = '\0';
				repl = encode_then_expand_vararg(exp_ಶಬ್ದ, /*handle_squotes:*/ 1, /*unbackslash:*/ 1);
				debug_printf_varexp("repl:'%s'->'%s'\n", exp_ಶಬ್ದ, repl);
				/* HACK ALERT. We depend here on the fact that
				 * G.global_argv and results of utoa and get_local_var_value
				 * are actually in writable memory:
				 * replace_pattern momentarily stores NULs there. */
				t = (ಅಕ್ಷರ*)val;
				ಗಮ್ಯ_be_freed = replace_ನಮೂನೆ(t,
						ನಮೂನೆ,
						(repl ? repl : exp_ಶಬ್ದ),
						exp_op);
				ಆದರೆ (ಗಮ್ಯ_be_freed) /* at least one replace happened */
					val = ಗಮ್ಯ_be_freed;
				ಬಿಡುಗಡೆ(ನಮೂನೆ);
				ಬಿಡುಗಡೆ(repl);
			} ಇಲ್ಲದಿದ್ದರೆ {
				/* Unset variable always gives nothing */
				//  a=; echo ${a/*/w}      # "w"
				//  unset a; echo ${a/*/w} # ""
				/* Just skip "replace" part */
				*p++ = SPECIAL_VAR_ಚಿಹ್ನೆ;
				p = strchr(p, SPECIAL_VAR_ಚಿಹ್ನೆ);
				*p = '\0';
			}
		}
#ಆದರೆ_ಅಂತ್ಯ /* BASH_PATTERN_SUBST */
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (exp_op == ':') {
#ಆದರೆ BASH_SUBSTR && ENABLE_FEATURE_SH_MATH
			/* It's ${var:N[:M]} bashism.
			 * Note that in encoded form it has TWO parts:
			 * var:N<SPECIAL_VAR_SYMBOL>M<SPECIAL_VAR_SYMBOL>
			 */
			arith_t beg, len;
			ಚಿಹ್ನೆರಹಿತ vallen;
			ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *errmsg;

			beg = expand_and_evaluate_arith(exp_ಶಬ್ದ, &errmsg);
			ಆದರೆ (errmsg)
				ಹೋಗು ಖಾಲಿಗಳು_ಫಲಿತಾಂಶ;
			debug_printf_varexp("beg:'%s'=%lld\n", exp_ಶಬ್ದ, (ದೀರ್ಘ ದೀರ್ಘ)beg);
			*p++ = SPECIAL_VAR_ಚಿಹ್ನೆ;
			exp_ಶಬ್ದ = p;
			p = strchr(p, SPECIAL_VAR_ಚಿಹ್ನೆ);
			*p = '\0';
			vallen = val ? ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(val) : 0;
			ಆದರೆ (beg < 0) {
				/* negative beg counts from the end */
				beg = (arith_t)vallen + beg;
			}
			/* If expansion will be empty, do not even evaluate len */
			ಆದರೆ (!val || beg < 0 || beg > vallen) {
				/* Why > vallen, not >=? bash:
				 * unset b; a=ab; : ${a:2:${b=3}}; echo $b  # "", b=3 (!!!)
				 * unset b; a=a; : ${a:2:${b=3}}; echo $b   # "", b not set
				 */
				ಹೋಗು ಖಾಲಿಗಳು_ಫಲಿತಾಂಶ;
			}
			len = expand_and_evaluate_arith(exp_ಶಬ್ದ, &errmsg);
			ಆದರೆ (errmsg)
				ಹೋಗು ಖಾಲಿಗಳು_ಫಲಿತಾಂಶ;
			debug_printf_varexp("len:'%s'=%lld\n", exp_ಶಬ್ದ, (ದೀರ್ಘ ದೀರ್ಘ)len);
			debug_printf_varexp("from val:'%s'\n", val);
			ಆದರೆ (len < 0) {
				/* in bash, len=-n means strlen()-n */
				len = (arith_t)vallen - beg + len;
				ಆದರೆ (len < 0) /* bash compat */
					ಸಂದೇಶ_and_die_if_script("%s: substring expression < 0", var);
			}
			ಆದರೆ (len <= 0 || !val /*|| beg >= vallen*/) {
 ಖಾಲಿಗಳು_ಫಲಿತಾಂಶ:
				val = ಖಾಲಿ;
			} ಇಲ್ಲದಿದ್ದರೆ {
				/* Paranoia. What if user entered 9999999999999
				 * which fits in arith_t but not int? */
				ಆದರೆ (len > ಗರಿಷ್ಠ_ಪೂರ್ಣಾಂಕ)
					len = ಗರಿಷ್ಠ_ಪೂರ್ಣಾಂಕ;
				val = ಗಮ್ಯ_be_freed = xstrndup(val + beg, len);
			}
			debug_printf_varexp("val:'%s'\n", val);
#ಇಲ್ಲದಿದ್ದರೆ /* not (HUSH_SUBSTR_EXPANSION && FEATURE_SH_MATH) */
			ಸಂದೇಶ_and_die_if_script("malformed ${%s:...}", var);
			val = ಖಾಲಿ;
#ಆದರೆ_ಅಂತ್ಯ
		} ಇಲ್ಲದಿದ್ದರೆ { /* one of "-=+?" */
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
			ಪೂರ್ಣಾಂಕ use_ಶಬ್ದ = (!val || ((exp_save == ':') && !val[0]));
			ಆದರೆ (exp_op == '+')
				use_ಶಬ್ದ = !use_ಶಬ್ದ;
			debug_printf_expand("expand: op:%c (null:%s) test:%i\n", exp_op,
					(exp_save == ':') ? "true" : "false", use_ಶಬ್ದ);
			ಆದರೆ (use_ಶಬ್ದ) {
				ಆದರೆ (exp_op == '+' || exp_op == '-') {
					/* ${var+word} - use alternative value */
					/* ${var-word} - use default value */
					n = encode_then_append_var_plusminus(output, n, exp_ಶಬ್ದ,
							/*dquoted:*/ (arg0 & 0x80)
					);
					val = ಖಾಲಿ;
				} ಇಲ್ಲದಿದ್ದರೆ {
					/* ${var?word} - indicate error if unset */
					/* ${var=word} - assign and use default value */
					ಗಮ್ಯ_be_freed = encode_then_expand_vararg(exp_ಶಬ್ದ,
							/*handle_squotes:*/ !(arg0 & 0x80),
							/*unbackslash:*/ 0
					);
					ಆದರೆ (ಗಮ್ಯ_be_freed)
						exp_ಶಬ್ದ = ಗಮ್ಯ_be_freed;
					ಆದರೆ (exp_op == '?') {
						/* mimic bash message */
						ಸಂದೇಶ_and_die_if_script("%s: %s",
							var,
							exp_ಶಬ್ದ[0]
							? exp_ಶಬ್ದ
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
					} ಇಲ್ಲದಿದ್ದರೆ {
						val = exp_ಶಬ್ದ;
					}
					ಆದರೆ (exp_op == '=') {
						/* ${var=[word]} or ${var:=[word]} */
						ಆದರೆ (ಅಂಕಿಯೇ(var[0]) || var[0] == '#') {
							/* mimic bash message */
							ಸಂದೇಶ_and_die_if_script("$%s: cannot assign in this way", var);
							val = ಖಾಲಿ;
						} ಇಲ್ಲದಿದ್ದರೆ {
							ಅಕ್ಷರ *new_var = xasprintf("%s=%s", var, val);
							ಗಣ_local_var0(new_var);
						}
					}
				}
			}
		} /* one of "-=+?" */

		*exp_saveptr = exp_save;
	} /* if (exp_op) */

#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */
	arg[0] = arg0;
	*pp = p;

	n = append_str_maybe_ifs_ವಿಭಜಿಸು(output, n, ಮೊದಲ_ch, val);

	ಬಿಡುಗಡೆ(ಗಮ್ಯ_be_freed);
	ಹಿಂತಿರುಗಿ n;
}

/* Expand all variable references in given string, adding words to list[]
 * at n, n+1,... positions. Return updated n (so that list[n] is next one
 * to be filled). This routine is extremely tricky: has to deal with
 * variables/parameters with whitespace, $* and $@, and constructs like
 * 'echo -$*-'. If you play here, you must run testsuite afterwards! */
ಸ್ಥಿರ NOINLINE ಪೂರ್ಣಾಂಕ expand_vars_ಗಮ್ಯ_list(o_string *output, ಪೂರ್ಣಾಂಕ n, ಅಕ್ಷರ *arg)
{
	/* output->o_expflags & EXP_FLAG_SINGLEWORD (0x80) if we are in
	 * expansion of right-hand side of assignment == 1-element expand.
	 */
	ಅಕ್ಷರ cant_be_null = 0; /* only bit 0x80 matters */
	ಅಕ್ಷರ *p;

	debug_printf_expand("expand_vars_to_list: arg:'%s' singleword:%x\n", arg,
			!!(output->o_expflags & EXP_FLAG_SINGLEWORD));
	debug_print_list("expand_vars_to_list[0]", output, n);

	ಆಗ ((p = strchr(arg, SPECIAL_VAR_ಚಿಹ್ನೆ)) != ಖಾಲಿ) {
		ಅಕ್ಷರ ಮೊದಲ_ch;
#ಆದರೆ ENABLE_FEATURE_SH_MATH
		ಅಕ್ಷರ arith_buf[ಗಾತ್ರ(arith_t)*3 + 2];
#ಆದರೆ_ಅಂತ್ಯ

		ಆದರೆ (output->ended_ಒಳ_ifs) {
			o_addchr(output, '\0');
			n = o_save_ptr(output, n);
			output->ended_ಒಳ_ifs = 0;
		}

		o_addblock(output, arg, p - arg);
		debug_print_list("expand_vars_to_list[1]", output, n);
		arg = ++p;
		p = strchr(p, SPECIAL_VAR_ಚಿಹ್ನೆ);

		/* Fetch special var name (if it is indeed one of them)
		 * and quote bit, force the bit on if singleword expansion -
		 * important for not getting v=$@ expand to many words. */
		ಮೊದಲ_ch = arg[0] | (output->o_expflags & EXP_FLAG_SINGLEWORD);

		/* Is this variable quoted and thus expansion can't be null?
		 * "$@" is special. Even if quoted, it can still
		 * expand to nothing (not even an empty string),
		 * thus it is excluded. */
		ಆದರೆ ((ಮೊದಲ_ch & 0x7f) != '@')
			cant_be_null |= ಮೊದಲ_ch;

		ಆಯ್ಕೆ (ಮೊದಲ_ch & 0x7f) {
		/* Highest bit in first_ch indicates that var is double-quoted */
		ಸ್ಥಿತಿ '*':
		ಸ್ಥಿತಿ '@': {
			ಪೂರ್ಣಾಂಕ i;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
			ಆದರೆ (!G.global_argv[1])
#ಇಲ್ಲದಿದ್ದರೆ /* __U_BOOT__ */
			ಆದರೆ (!G.global_argv || !G.global_argv[1])
#ಆದರೆ_ಅಂತ್ಯ /* __U_BOOT__ */
				ನಿಲ್ಲಿಸು;
			i = 1;
			cant_be_null |= ಮೊದಲ_ch; /* do it for "$@" _now_, when we know it's not empty */
			ಆದರೆ (!(ಮೊದಲ_ch & 0x80)) { /* unquoted $* or $@ */
				ಆಗ (G.global_argv[i]) {
					n = expand_on_ifs(output, n, G.global_argv[i]);
					debug_printf_expand("expand_vars_to_list: argv %d (last %d)\n", i, G.global_argc - 1);
					ಆದರೆ (G.global_argv[i++][0] && G.global_argv[i]) {
						/* this argv[] is not empty and not last:
						 * put terminating NUL, start new word */
						o_addchr(output, '\0');
						debug_print_list("expand_vars_to_list[2]", output, n);
						n = o_save_ptr(output, n);
						debug_print_list("expand_vars_to_list[3]", output, n);
					}
				}
			} ಇಲ್ಲದಿದ್ದರೆ
			/* If EXP_FLAG_SINGLEWORD, we handle assignment 'a=....$@.....'
			 * and in this case should treat it like '$*' - see 'else...' below */
			ಆದರೆ (ಮೊದಲ_ch == (ಅಕ್ಷರ)('@'|0x80)  /* quoted $@ */
			 && !(output->o_expflags & EXP_FLAG_SINGLEWORD) /* not v="$@" case */
			) {
				ಆಗ (1) {
					o_ಕೂಡಿಸು_Qstr(output, G.global_argv[i]);
					ಆದರೆ (++i >= G.global_argc)
						ನಿಲ್ಲಿಸು;
					o_addchr(output, '\0');
					debug_print_list("expand_vars_to_list[4]", output, n);
					n = o_save_ptr(output, n);
				}
			} ಇಲ್ಲದಿದ್ದರೆ { /* quoted $* (or v="$@" case): add as one word */
				ಆಗ (1) {
					o_ಕೂಡಿಸು_Qstr(output, G.global_argv[i]);
					ಆದರೆ (!G.global_argv[++i])
						ನಿಲ್ಲಿಸು;
					ಆದರೆ (G.ifs[0])
						o_addchr(output, G.ifs[0]);
				}
				output->has_quoted_part = 1;
			}
			ನಿಲ್ಲಿಸು;
		}
		ಸ್ಥಿತಿ SPECIAL_VAR_ಚಿಹ್ನೆ: {
			/* <SPECIAL_VAR_SYMBOL><SPECIAL_VAR_SYMBOL> */
			/* "Empty variable", used to make "" etc to not disappear */
			output->has_quoted_part = 1;
			cant_be_null = 0x80;
			arg++;
			ನಿಲ್ಲಿಸು;
		}
		ಸ್ಥಿತಿ SPECIAL_VAR_QUOTED_SVS:
			/* <SPECIAL_VAR_SYMBOL><SPECIAL_VAR_QUOTED_SVS><SPECIAL_VAR_SYMBOL> */
			/* "^C variable", represents literal ^C char (possible in scripts) */
			o_addchr(output, SPECIAL_VAR_ಚಿಹ್ನೆ);
			arg++;
			ನಿಲ್ಲಿಸು;
#ಆದರೆ ENABLE_HUSH_TICK
		ಸ್ಥಿತಿ '`': {
			/* <SPECIAL_VAR_SYMBOL>`cmd<SPECIAL_VAR_SYMBOL> */
			o_string subst_ಫಲಿತಾಂಶ = NULL_O_STRING;

			*p = '\0'; /* replace trailing <SPECIAL_VAR_SYMBOL> */
			arg++;
			/* Can't just stuff it into output o_string,
			 * expanded result may need to be globbed
			 * and $IFS-split */
			debug_printf_subst("SUBST '%s' first_ch %x\n", arg, ಮೊದಲ_ch);
			G.ಕೊನೆಯ_exitcode = process_command_subs(&subst_ಫಲಿತಾಂಶ, arg);
			G.expand_exitcode = G.ಕೊನೆಯ_exitcode;
			debug_printf_subst("SUBST RES:%d '%s'\n", G.ಕೊನೆಯ_exitcode, subst_ಫಲಿತಾಂಶ.ದತ್ತಾಂಶ);
			n = append_str_maybe_ifs_ವಿಭಜಿಸು(output, n, ಮೊದಲ_ch, subst_ಫಲಿತಾಂಶ.ದತ್ತಾಂಶ);
			o_free(&subst_ಫಲಿತಾಂಶ);
			ನಿಲ್ಲಿಸು;
		}
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_FEATURE_SH_MATH
		ಸ್ಥಿತಿ '+': {
			/* <SPECIAL_VAR_SYMBOL>+arith<SPECIAL_VAR_SYMBOL> */
			arith_t res;

			arg++; /* skip '+' */
			*p = '\0'; /* replace trailing <SPECIAL_VAR_SYMBOL> */
			debug_printf_subst("ARITH '%s' first_ch %x\n", arg, ಮೊದಲ_ch);
			res = expand_and_evaluate_arith(arg, ಖಾಲಿ);
			debug_printf_subst("ARITH RES '"ARITH_FMT"'\n", res);
			sprintf(arith_buf, ARITH_FMT, res);
			ಆದರೆ (res < 0
			 && ಮೊದಲ_ch == (ಅಕ್ಷರ)('+'|0x80)
			/* && (output->o_expflags & EXP_FLAG_ESC_GLOB_CHARS) */
			) {
				/* Quoted negative ariths, like filename[0"$((-9))"],
				 * should not be interpreted as glob ranges.
				 * Convert leading '-' to '\-':
				 */
				o_grow_by(output, 1);
				output->ದತ್ತಾಂಶ[output->ಉದ್ದಳತೆ++] = '\\';
			}
			o_addstr(output, arith_buf);
			ನಿಲ್ಲಿಸು;
		}
#ಆದರೆ_ಅಂತ್ಯ
		ಪೂರ್ವನಿಯೋಜಿತ:
			/* <SPECIAL_VAR_SYMBOL>varname[ops]<SPECIAL_VAR_SYMBOL> */
			n = expand_one_var(output, n, ಮೊದಲ_ch, arg, &p);
			ನಿಲ್ಲಿಸು;
		} /* switch (char after <SPECIAL_VAR_SYMBOL>) */

		/* Restore NULL'ed SPECIAL_VAR_SYMBOL.
		 * Do the check to avoid writing to a const string. */
		ಆದರೆ (*p != SPECIAL_VAR_ಚಿಹ್ನೆ)
			*p = SPECIAL_VAR_ಚಿಹ್ನೆ;
		arg = ++p;
	} /* end of "while (SPECIAL_VAR_SYMBOL is found) ..." */

	ಆದರೆ (*arg) {
		/* handle trailing string */
		ಆದರೆ (output->ended_ಒಳ_ifs) {
			o_addchr(output, '\0');
			n = o_save_ptr(output, n);
		}
		debug_print_list("expand_vars_to_list[a]", output, n);
		/* this part is literal, and it was already pre-quoted
		 * if needed (much earlier), do not use o_addQstr here!
		 */
		o_addstr(output, arg);
		debug_print_list("expand_vars_to_list[b]", output, n);
	} ಇಲ್ಲದಿದ್ದರೆ
	ಆದರೆ (output->ಉದ್ದಳತೆ == o_get_ಕೊನೆಯ_ptr(output, n) /* expansion is empty */
	 && !(cant_be_null & 0x80)   /* and all vars were not quoted */
	 && !output->has_quoted_part
	) {
		n--;
		/* allow to reuse list[n] later without re-growth */
		output->has_ಖಾಲಿಗಳು_ಸ್ಥಾನ = 1;
	}

	ಹಿಂತಿರುಗಿ n;
}

ಸ್ಥಿರ ಅಕ್ಷರ **expand_variables(ಅಕ್ಷರ **argv, ಚಿಹ್ನೆರಹಿತ expflags)
{
	ಪೂರ್ಣಾಂಕ n;
	ಅಕ್ಷರ **list;
	o_string output = NULL_O_STRING;

	output.o_expflags = expflags;

	n = 0;
	ಗಾಗಿ (;;) {
		/* go to next list[n] */
		output.ended_ಒಳ_ifs = 0;
		n = o_save_ptr(&output, n);

		ಆದರೆ (!*argv)
			ನಿಲ್ಲಿಸು;

		/* expand argv[i] */
		n = expand_vars_ಗಮ್ಯ_list(&output, n, *argv++);
		/* if (!output->has_empty_slot) -- need this?? */
			o_addchr(&output, '\0');
	}
	debug_print_list("expand_variables", &output, n);

	/* output.data (malloced in one block) gets returned in "list" */
	list = o_finalize_list(&output, n);
	debug_print_ಅಕ್ಷರಸರಣಿಗಳು("expand_variables[1]", list);
	ಹಿಂತಿರುಗಿ list;
}

ಸ್ಥಿರ ಅಕ್ಷರ **expand_strvec_ಗಮ್ಯ_strvec(ಅಕ್ಷರ **argv)
{
	ಹಿಂತಿರುಗಿ expand_variables(argv, EXP_FLAG_GLOB | EXP_FLAG_ESC_GLOB_CHARS);
}

#ಆದರೆ defined(CMD_SINGLEWORD_NOGLOB) || defined(CMD_TEST2_SINGLEWORD_NOGLOB)
ಸ್ಥಿರ ಅಕ್ಷರ **expand_strvec_ಗಮ್ಯ_strvec_singleword_noglob(ಅಕ್ಷರ **argv)
{
	ಹಿಂತಿರುಗಿ expand_variables(argv, EXP_FLAG_SINGLEWORD);
}
#ಆದರೆ_ಅಂತ್ಯ

/* Used for expansion of right hand of assignments,
 * $((...)), heredocs, variable expansion parts.
 *
 * NB: should NOT do globbing!
 * "export v=/bin/c*; env | grep ^v=" outputs "v=/bin/c*"
 */
ಸ್ಥಿರ ಅಕ್ಷರ *expand_string_ಗಮ್ಯ_string(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str, ಪೂರ್ಣಾಂಕ EXP_flags, ಪೂರ್ಣಾಂಕ do_unbackslash)
{
#ಆದರೆ !BASH_ನಮೂನೆ_SUBST && !ENABLE_HUSH_CASE
	ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ do_unbackslash = 1;
	ಸ್ಥಿರಾಂಕ ಪೂರ್ಣಾಂಕ EXP_flags = EXP_FLAG_ESC_GLOB_CHARS;
#ಆದರೆ_ಅಂತ್ಯ
	ಅಕ್ಷರ *argv[2], **list;

	debug_printf_expand("string_to_string<='%s'\n", str);
	/* This is generally an optimization, but it also
	 * handles "", which otherwise trips over !list[0] check below.
	 * (is this ever happens that we actually get str="" here?)
	 */
	ಆದರೆ (!strchr(str, SPECIAL_VAR_ಚಿಹ್ನೆ) && !strchr(str, '\\')) {
		//TODO: Can use on strings with \ too, just unbackslash() them?
		debug_printf_expand("string_to_string(fast)=>'%s'\n", str);
		ಹಿಂತಿರುಗಿ xstrdup(str);
	}

	argv[0] = (ಅಕ್ಷರ*)str;
	argv[1] = ಖಾಲಿ;
	list = expand_variables(argv, EXP_flags | EXP_FLAG_SINGLEWORD);
	ಆದರೆ (!list[0]) {
		/* Example where it happens:
		 * x=; echo ${x:-"$@"}
		 */
		((ಅಕ್ಷರ*)list)[0] = '\0';
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಆದರೆ (HUSH_DEBUG)
			ಆದರೆ (list[1])
				bb_simple_error_ಸಂದೇಶ_and_die("BUG in varexp2");
		/* actually, just move string 2*sizeof(char*) bytes back */
		overlapping_strcpy((ಅಕ್ಷರ*)list, list[0]);
		ಆದರೆ (do_unbackslash)
			unbackslash((ಅಕ್ಷರ*)list);
	}
	debug_printf_expand("string_to_string=>'%s'\n", (ಅಕ್ಷರ*)list);
	ಹಿಂತಿರುಗಿ (ಅಕ್ಷರ*)list;
}

#ಆದರೆ 0
ಸ್ಥಿರ ಅಕ್ಷರ* expand_strvec_ಗಮ್ಯ_string(ಅಕ್ಷರ **argv)
{
	ಅಕ್ಷರ **list;

	list = expand_variables(argv, EXP_FLAG_SINGLEWORD);
	/* Convert all NULs to spaces */
	ಆದರೆ (list[0]) {
		ಪೂರ್ಣಾಂಕ n = 1;
		ಆಗ (list[n]) {
			ಆದರೆ (HUSH_DEBUG)
				ಆದರೆ (list[n-1] + ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(list[n-1]) + 1 != list[n])
					bb_error_ಸಂದೇಶ_and_die("BUG in varexp3");
			/* bash uses ' ' regardless of $IFS contents */
			list[n][-1] = ' ';
			n++;
		}
	}
	overlapping_strcpy((ಅಕ್ಷರ*)list, list[0] ? list[0] : "");
	debug_printf_expand("strvec_to_string='%s'\n", (ಅಕ್ಷರ*)list);
	ಹಿಂತಿರುಗಿ (ಅಕ್ಷರ*)list;
}
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
ಸ್ಥಿರ ಅಕ್ಷರ **expand_assignments(ಅಕ್ಷರ **argv, ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2)
{
	ಪೂರ್ಣಾಂಕ i;
	ಅಕ್ಷರ **p;

	G.expanded_assignments = p = ಖಾಲಿ;
	/* Expand assignments into one string each */
	ಗಾಗಿ (i = 0; i < ಎಣಿಕೆ_2; i++) {
		p = ಕೂಡಿಸು_string_ಗಮ್ಯ_ಅಕ್ಷರಸರಣಿಗಳು(p,
			expand_string_ಗಮ್ಯ_string(argv[i],
				EXP_FLAG_ESC_GLOB_CHARS,
				/*unbackslash:*/ 1
			)
		);
		G.expanded_assignments = p;
	}
	G.expanded_assignments = ಖಾಲಿ;
	ಹಿಂತಿರುಗಿ p;
}

ಸ್ಥಿರ ಶೂನ್ಯ switch_off_special_sigs(ಚಿಹ್ನೆರಹಿತ ಮುಖವಾಡ)
{
	ಚಿಹ್ನೆರಹಿತ sig = 0;
	ಆಗ ((ಮುಖವಾಡ >>= 1) != 0) {
		sig++;
		ಆದರೆ (!(ಮುಖವಾಡ & 1))
			ಮುಂದುವರಿ;
#ಆದರೆ ENABLE_HUSH_TRAP
		ಆದರೆ (G_traps) {
			ಆದರೆ (G_traps[sig] && !G_traps[sig][0])
				/* trap is '', has to remain SIG_IGN */
				ಮುಂದುವರಿ;
			ಬಿಡುಗಡೆ(G_traps[sig]);
			G_traps[sig] = ಖಾಲಿ;
		}
#ಆದರೆ_ಅಂತ್ಯ
		/* We are here only if no trap or trap was not '' */
		install_sighandler(sig, SIG_DFL);
	}
}
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
#ಆದರೆ BB_MMU
/* never called */
ಶೂನ್ಯ re_execute_shell(ಅಕ್ಷರ ***ಗಮ್ಯ_free, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s,
		ಅಕ್ಷರ *g_argv0, ಅಕ್ಷರ **g_argv,
		ಅಕ್ಷರ **builtin_argv) NORETURN;

ಸ್ಥಿರ ಶೂನ್ಯ reset_traps_ಗಮ್ಯ_defaults(ಶೂನ್ಯ)
{
	/* This function is always called in a child shell
	 * after fork (not vfork, NOMMU doesn't use this function).
	 */
	IF_HUSH_TRAP(ಚಿಹ್ನೆರಹಿತ sig;)
	ಚಿಹ್ನೆರಹಿತ ಮುಖವಾಡ;

	/* Child shells are not interactive.
	 * SIGTTIN/SIGTTOU/SIGTSTP should not have special handling.
	 * Testcase: (while :; do :; done) + ^Z should background.
	 * Same goes for SIGTERM, SIGHUP, SIGINT.
	 */
	ಮುಖವಾಡ = (G.special_sig_ಮುಖವಾಡ & SPECIAL_INTERACTIVE_SIGS) | G_fatal_sig_ಮುಖವಾಡ;
	ಆದರೆ (!G_traps && !ಮುಖವಾಡ)
		ಹಿಂತಿರುಗಿ; /* already no traps and no special sigs */

	/* Switch off special sigs */
	switch_off_special_sigs(ಮುಖವಾಡ);
# ಆದರೆ ENABLE_HUSH_ಕೆಲಸ
	G_fatal_sig_ಮುಖವಾಡ = 0;
# endif
	G.special_sig_ಮುಖವಾಡ &= ~SPECIAL_INTERACTIVE_SIGS;
	/* SIGQUIT,SIGCHLD and maybe SPECIAL_JOBSTOP_SIGS
	 * remain set in G.special_sig_mask */

# ಆದರೆ ENABLE_HUSH_TRAP
	ಆದರೆ (!G_traps)
		ಹಿಂತಿರುಗಿ;

	/* Reset all sigs to default except ones with empty traps */
	ಗಾಗಿ (sig = 0; sig < NSIG; sig++) {
		ಆದರೆ (!G_traps[sig])
			ಮುಂದುವರಿ; /* no trap: nothing to do */
		ಆದರೆ (!G_traps[sig][0])
			ಮುಂದುವರಿ; /* empty trap: has to remain SIG_IGN */
		/* sig has non-empty trap, reset it: */
		ಬಿಡುಗಡೆ(G_traps[sig]);
		G_traps[sig] = ಖಾಲಿ;
		/* There is no signal for trap 0 (EXIT) */
		ಆದರೆ (sig == 0)
			ಮುಂದುವರಿ;
		install_sighandler(sig, pick_sighandler(sig));
	}
# endif
}

#ಇಲ್ಲದಿದ್ದರೆ /* !BB_MMU */

ಸ್ಥಿರ ಶೂನ್ಯ re_execute_shell(ಅಕ್ಷರ ***ಗಮ್ಯ_free, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s,
		ಅಕ್ಷರ *g_argv0, ಅಕ್ಷರ **g_argv,
		ಅಕ್ಷರ **builtin_argv) NORETURN;
ಸ್ಥಿರ ಶೂನ್ಯ re_execute_shell(ಅಕ್ಷರ ***ಗಮ್ಯ_free, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s,
		ಅಕ್ಷರ *g_argv0, ಅಕ್ಷರ **g_argv,
		ಅಕ್ಷರ **builtin_argv)
{
# define NOMMU_HACK_FMT ("-$%x:%x:%x:%x:%x:%llx" IF_HUSH_LOOPS(":%x"))
	/* delims + 2 * (number of bytes in printed hex numbers) */
	ಅಕ್ಷರ param_buf[ಗಾತ್ರ(NOMMU_HACK_FMT) + 2 * (ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ)*6 + ಗಾತ್ರ(ದೀರ್ಘ ದೀರ್ಘ)*1)];
	ಅಕ್ಷರ *heredoc_argv[4];
	ರಚನೆ variable *cur;
# ಆದರೆ ENABLE_HUSH_FUNCTIONS
	ರಚನೆ function *funcp;
# endif
	ಅಕ್ಷರ **argv, **pp;
	ಚಿಹ್ನೆರಹಿತ cnt;
	ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ ಖಾಲಿಗಳು_trap_ಮುಖವಾಡ;

	ಆದರೆ (!g_argv0) { /* heredoc */
		argv = heredoc_argv;
		argv[0] = (ಅಕ್ಷರ *) G.argv0_for_re_execing;
		argv[1] = (ಅಕ್ಷರ *) "-<";
		argv[2] = (ಅಕ್ಷರ *) s;
		argv[3] = ಖಾಲಿ;
		pp = &argv[3]; /* used as pointer to empty environment */
		ಹೋಗು do_exec;
	}

	cnt = 0;
	pp = builtin_argv;
	ಆದರೆ (pp) ಆಗ (*pp++)
		cnt++;

	ಖಾಲಿಗಳು_trap_ಮುಖವಾಡ = 0;
	ಆದರೆ (G_traps) {
		ಪೂರ್ಣಾಂಕ sig;
		ಗಾಗಿ (sig = 1; sig < NSIG; sig++) {
			ಆದರೆ (G_traps[sig] && !G_traps[sig][0])
				ಖಾಲಿಗಳು_trap_ಮುಖವಾಡ |= 1LL << sig;
		}
	}

	sprintf(param_buf, NOMMU_HACK_FMT
			, (ಚಿಹ್ನೆರಹಿತ) G.ಬೇರು_pid
			, (ಚಿಹ್ನೆರಹಿತ) G.ಬೇರು_ppid
			, (ಚಿಹ್ನೆರಹಿತ) G.ಕೊನೆಯ_bg_pid
			, (ಚಿಹ್ನೆರಹಿತ) G.ಕೊನೆಯ_exitcode
			, cnt
			, ಖಾಲಿಗಳು_trap_ಮುಖವಾಡ
			IF_HUSH_LOOPS(, G.ಆಳ_ದ_loop)
			);
# undef NOMMU_HACK_FMT
	/* 1:hush 2:-$<pid>:<pid>:<exitcode>:<etc...> <vars...> <funcs...>
	 * 3:-c 4:<cmd> 5:<arg0> <argN...> 6:NULL
	 */
	cnt += 6;
	ಗಾಗಿ (cur = G.ತುದಿ_var; cur; cur = cur->ಮುಂದಿನ) {
		ಆದರೆ (!cur->flg_export || cur->flg_read_only)
			cnt += 2;
	}
# ಆದರೆ ENABLE_HUSH_LINENO_VAR
	cnt += 2;
# endif
# ಆದರೆ ENABLE_HUSH_FUNCTIONS
	ಗಾಗಿ (funcp = G.ತುದಿ_func; funcp; funcp = funcp->ಮುಂದಿನ)
		cnt += 3;
# endif
	pp = g_argv;
	ಆಗ (*pp++)
		cnt++;
	*ಗಮ್ಯ_free = argv = pp = xzalloc(ಗಾತ್ರ(argv[0]) * cnt);
	*pp++ = (ಅಕ್ಷರ *) G.argv0_for_re_execing;
	*pp++ = param_buf;
	ಗಾಗಿ (cur = G.ತುದಿ_var; cur; cur = cur->ಮುಂದಿನ) {
		ಆದರೆ (strcmp(cur->varstr, hush_ಆವೃತ್ತಿ_str) == 0)
			ಮುಂದುವರಿ;
		ಆದರೆ (cur->flg_read_only) {
			*pp++ = (ಅಕ್ಷರ *) "-R";
			*pp++ = cur->varstr;
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (!cur->flg_export) {
			*pp++ = (ಅಕ್ಷರ *) "-V";
			*pp++ = cur->varstr;
		}
	}
# ಆದರೆ ENABLE_HUSH_LINENO_VAR
	*pp++ = (ಅಕ್ಷರ *) "-L";
	*pp++ = utoa(G.execute_lineno);
# endif
# ಆದರೆ ENABLE_HUSH_FUNCTIONS
	ಗಾಗಿ (funcp = G.ತುದಿ_func; funcp; funcp = funcp->ಮುಂದಿನ) {
		*pp++ = (ಅಕ್ಷರ *) "-F";
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
	*pp++ = (ಅಕ್ಷರ *) "-c";
	*pp++ = (ಅಕ್ಷರ *) s;
	ಆದರೆ (builtin_argv) {
		ಆಗ (*++builtin_argv)
			*pp++ = *builtin_argv;
		*pp++ = (ಅಕ್ಷರ *) "";
	}
	*pp++ = g_argv0;
	ಆಗ (*g_argv)
		*pp++ = *g_argv++;
	/* *pp = NULL; - is already there */
	pp = environ;

 do_exec:
	debug_printf_exec("re_execute_shell pid:%d cmd:'%s'\n", getpid(), s);
	/* Don't propagate SIG_IGN to the child */
	ಆದರೆ (SPECIAL_JOBSTOP_SIGS != 0)
		switch_off_special_sigs(G.special_sig_ಮುಖವಾಡ & SPECIAL_JOBSTOP_SIGS);
	execve(bb_busybox_exec_ಮಾರ್ಗ, argv, pp);
	/* Fallback. Useful for init=/bin/hush usage etc */
	ಆದರೆ (argv[0][0] == '/')
		execve(argv[0], argv, pp);
	xfunc_error_retval = 127;
	bb_simple_error_ಸಂದೇಶ_and_die("can't re-execute the shell");
}
#ಆದರೆ_ಅಂತ್ಯ  /* !BB_MMU */

#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಸತತ_and_free_list(ರಚನೆ pipe *pi);

/* Executing from string: eval, sh -c '...'
 *          or from file: /etc/profile, . file, sh <script>, sh (intereactive)
 * end_trigger controls how often we stop parsing
 * NUL: parse all, execute, return
 * ';': parse till ';' or newline, execute, repeat till EOF
 */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
ಸ್ಥಿರ ಶೂನ್ಯ parse_and_ಸತತ_ಹರಿವು(ರಚನೆ ಒಳ_str *inp, ಪೂರ್ಣಾಂಕ ಕೊನೆ_trigger)
#ಇಲ್ಲದಿದ್ದರೆ /* __U_BOOT__ */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ parse_and_ಸತತ_ಹರಿವು(ರಚನೆ ಒಳ_str *inp, ಪೂರ್ಣಾಂಕ ಕೊನೆ_trigger)
#ಆದರೆ_ಅಂತ್ಯ /* __U_BOOT__ */
{
	/* Why we need empty flag?
	 * An obscure corner case "false; ``; echo $?":
	 * empty command in `` should still set $? to 0.
	 * But we can't just set $? to 0 at the start,
	 * this breaks "false; echo `echo $?`" case.
	 */
	bool ಖಾಲಿಗಳು = 1;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	ಆಗ (1) {
#ಇಲ್ಲದಿದ್ದರೆ /* __U_BOOT__ */
	ಮಾಡು {
#ಆದರೆ_ಅಂತ್ಯ /* __U_BOOT__ */
		ರಚನೆ pipe *pipe_list;

#ಆದರೆ ENABLE_HUSH_INTERACTIVE
		ಆದರೆ (ಕೊನೆ_trigger == ';') {
			G.promptmode = 0; /* PS1 */
			debug_printf_prompt("%s promptmode=%d\n", __func__, G.promptmode);
		}
#ಆದರೆ_ಅಂತ್ಯ
		pipe_list = parse_ಹರಿವು(ಖಾಲಿ, ಖಾಲಿ, inp, ಕೊನೆ_trigger);
		ಆದರೆ (!pipe_list || pipe_list == ERR_PTR) { /* EOF/error */
			/* If we are in "big" script
			 * (not in `cmd` or something similar)...
			 */
			ಆದರೆ (pipe_list == ERR_PTR && ಕೊನೆ_trigger == ';') {
				/* Discard cached input (rest of line) */
				ಪೂರ್ಣಾಂಕ ch = inp->ಕೊನೆಯ_char;
				ಆಗ (ch != EOF && ch != '\n') {
					//bb_error_msg("Discarded:'%c'", ch);
					ch = i_getch(inp);
				}
				/* Force prompt */
				inp->p = ಖಾಲಿ;
				/* This stream isn't empty */
				ಖಾಲಿಗಳು = 0;
				ಮುಂದುವರಿ;
			}
			ಆದರೆ (!pipe_list && ಖಾಲಿಗಳು)
				G.ಕೊನೆಯ_exitcode = 0;
			ನಿಲ್ಲಿಸು;
		}
		debug_print_ವೃಕ್ಷ(pipe_list, 0);
		debug_printf_exec("parse_and_run_stream: run_and_free_list\n");
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
		ಸತತ_and_free_list(pipe_list);
#ಇಲ್ಲದಿದ್ದರೆ /* __U_BOOT__ */
		ಪೂರ್ಣಾಂಕ rcode = ಸತತ_and_free_list(pipe_list);
		/*
		 * We reset input string to not run the following command, so running
		 * 'exit; echo foo' does not print foo.
		 */
		ಆದರೆ (rcode <= EXIT_RET_ಸಂಕೇತ)
			ಸಿದ್ಧತೆ_file_ಒಳ_str(inp);
#ಆದರೆ_ಅಂತ್ಯ /* __U_BOOT__ */
		ಖಾಲಿಗಳು = 0;
		ಆದರೆ (G_flag_return_ಒಳ_ಪ್ರಗತಿ == 1)
			ನಿಲ್ಲಿಸು;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	}
#ಇಲ್ಲದಿದ್ದರೆ /* __U_BOOT__ */
	/*
	 * This do/while is needed by run_command to avoid looping on a command
	 * with syntax error.
	 */
	} ಆಗ (!(G.ಸತತ_command_flags & FLAG_EXIT_FROM_LOOP));

	ಹಿಂತಿರುಗಿ G.ಕೊನೆಯ_exitcode;
#ಆದರೆ_ಅಂತ್ಯ /* __U_BOOT__ */
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
ಸ್ಥಿರ ಶೂನ್ಯ parse_and_ಸತತ_string(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s)
#ಇಲ್ಲದಿದ್ದರೆ /* __U_BOOT__ */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ parse_and_ಸತತ_string(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s)
#ಆದರೆ_ಅಂತ್ಯ /* __U_BOOT__ */
{
	ರಚನೆ ಒಳ_str ಒಳಹರಿವು;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	IF_HUSH_LINENO_VAR(ಚಿಹ್ನೆರಹಿತ sv = G.parse_lineno;)
#ಇಲ್ಲದಿದ್ದರೆ /* __U_BOOT__ */
	//IF_HUSH_LINENO_VAR(unsigned sv = G.parse_lineno;)
#ಆದರೆ_ಅಂತ್ಯ /* __U_BOOT__ */

	ಸಿದ್ಧತೆ_string_ಒಳ_str(&ಒಳಹರಿವು, s);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	parse_and_ಸತತ_ಹರಿವು(&ಒಳಹರಿವು, '\0');
#ಇಲ್ಲದಿದ್ದರೆ /* __U_BOOT__ */
	ಹಿಂತಿರುಗಿ parse_and_ಸತತ_ಹರಿವು(&ಒಳಹರಿವು, '\0');
#ಆದರೆ_ಅಂತ್ಯ /* __U_BOOT__ */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	IF_HUSH_LINENO_VAR(ಚಿಹ್ನೆರಹಿತ sv = G.parse_lineno;)
#ಇಲ್ಲದಿದ್ದರೆ /* __U_BOOT__ */
	//IF_HUSH_LINENO_VAR(unsigned sv = G.parse_lineno;)
#ಆದರೆ_ಅಂತ್ಯ /* __U_BOOT__ */
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __U_BOOT__
ಪೂರ್ಣಾಂಕ parse_string_outer_modern(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *cmd, ಪೂರ್ಣಾಂಕ flags)
{
	ಪೂರ್ಣಾಂಕ ret;
	ಪೂರ್ಣಾಂಕ old_flags;

	/*
	 * Keep old values of run_command to be able to restore them once
	 * command was executed.
	 */
	old_flags = G.ಸತತ_command_flags;
	G.ಸತತ_command_flags = flags;

	ret = parse_and_ಸತತ_string(cmd);

	G.ಸತತ_command_flags = old_flags;

	ಹಿಂತಿರುಗಿ ret;
}
#ಆದರೆ_ಅಂತ್ಯ /* __U_BOOT__ */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
ಸ್ಥಿರ ಶೂನ್ಯ parse_and_ಸತತ_file(HFILE *fp)
#ಇಲ್ಲದಿದ್ದರೆ /* __U_BOOT__ */
ಶೂನ್ಯ parse_and_ಸತತ_file(ಶೂನ್ಯ)
#ಆದರೆ_ಅಂತ್ಯ /* __U_BOOT__ */
{
	ರಚನೆ ಒಳ_str ಒಳಹರಿವು;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	IF_HUSH_LINENO_VAR(ಚಿಹ್ನೆರಹಿತ sv = G.parse_lineno;)

	IF_HUSH_LINENO_VAR(G.parse_lineno = 1;)
	ಸಿದ್ಧತೆ_file_ಒಳ_str(&ಒಳಹರಿವು, fp);
#ಇಲ್ಲದಿದ್ದರೆ /* __U_BOOT__ */
	ಸಿದ್ಧತೆ_file_ಒಳ_str(&ಒಳಹರಿವು);
#ಆದರೆ_ಅಂತ್ಯ /* __U_BOOT__ */
	parse_and_ಸತತ_ಹರಿವು(&ಒಳಹರಿವು, ';');
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	IF_HUSH_LINENO_VAR(G.parse_lineno = sv;)
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
#ಆದರೆ ENABLE_HUSH_TICK
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ generate_ಹರಿವು_from_string(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s, pid_t *pid_p)
{
	pid_t pid;
	ಪೂರ್ಣಾಂಕ channel[2];
# ಆದರೆ !BB_MMU
	ಅಕ್ಷರ **ಗಮ್ಯ_free = ಖಾಲಿ;
# endif

	xpipe(channel);
	pid = BB_MMU ? xfork() : xvfork();
	ಆದರೆ (pid == 0) { /*ಮಗು*/
		disable_restore_tty_pgrp_on_exit();
		/* Process substitution is not considered to be usual
		 * 'command execution'.
		 * SUSv3 says ctrl-Z should be ignored, ctrl-C should not.
		 */
		bb_ಸಂಕೇತಗಳು(0
			+ (1 << SIGTSTP)
			+ (1 << SIGTTIN)
			+ (1 << SIGTTOU)
			, SIG_IGN);
		close(channel[0]); /* NB: close _first_, then move fd! */
		xmove_fd(channel[1], 1);
# ಆದರೆ ENABLE_HUSH_TRAP
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
		ಆದರೆ (is_prefixed_with(s, "trap")
		 && skip_whitespace(s + 4)[0] == '\0'
		) {
			ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ argv[] ALIGN_PTR = { ಖಾಲಿ, ಖಾಲಿ };
			builtin_trap((ಅಕ್ಷರ**)argv);
			fflush_all(); /* important */
			_exit(0);
		}
# endif
# ಆದರೆ BB_MMU
		/* Prevent it from trying to handle ctrl-z etc */
		IF_HUSH_ಕೆಲಸ(G.ಸತತ_list_level = 1;)
		CLEAR_RANDOM_T(&G.random_gen); /* or else $RANDOM repeats in child */
		reset_traps_ಗಮ್ಯ_defaults();
		IF_HUSH_ಬಹುಲಕ_X(G.x_ಬಹುಲಕ_ಆಳ++;)
		//bb_error_msg("%s: ++x_mode_depth=%d", __func__, G.x_mode_depth);
		parse_and_ಸತತ_string(s);
		_exit(G.ಕೊನೆಯ_exitcode);
# ಇಲ್ಲದಿದ್ದರೆ
	/* We re-execute after vfork on NOMMU. This makes this script safe:
	 * yes "0123456789012345678901234567890" | dd bs=32 count=64k >BIG
	 * huge=`cat BIG` # was blocking here forever
	 * echo OK
	 */
		re_execute_shell(&ಗಮ್ಯ_free,
				s,
				G.global_argv[0],
				G.global_argv + 1,
				ಖಾಲಿ);
# endif
	}

	/*ಪೋಷಕ*/
	*pid_p = pid;
# ಆದರೆ ENABLE_HUSH_FAST
	G.ಎಣಿಕೆ_SIGCHLD++;
//bb_error_msg("[%d] fork in generate_stream_from_string:"
//		" G.count_SIGCHLD:%d G.handled_SIGCHLD:%d",
//		getpid(), G.count_SIGCHLD, G.handled_SIGCHLD);
# endif
	enable_restore_tty_pgrp_on_exit();
# ಆದರೆ !BB_MMU
	ಬಿಡುಗಡೆ(ಗಮ್ಯ_free);
# endif
	close(channel[1]);
	ಹಿಂತಿರುಗಿ channel[0];
}

/* Return code is exit status of the process that is run. */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ process_command_subs(o_string *dest, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s)
{
	FILE *fp;
	pid_t pid;
	ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2, ch, eol_cnt;

	fp = xfdopen_for_read(generate_ಹರಿವು_from_string(s, &pid));

	/* Now send results of command back into original context */
	eol_cnt = 0;
	ಆಗ ((ch = getc(fp)) != EOF) {
		ಆದರೆ (ch == '\0')
			ಮುಂದುವರಿ;
		ಆದರೆ (ch == '\n') {
			eol_cnt++;
			ಮುಂದುವರಿ;
		}
		ಆಗ (eol_cnt) {
			o_addchr(dest, '\n');
			eol_cnt--;
		}
		o_ಕೂಡಿಸು_Qchr(dest, ch);
	}

	debug_printf("done reading from `cmd` pipe, closing it\n");
	fclose(fp);
	/* We need to extract exitcode. Test case
	 * "true; echo `sleep 1; false` $?"
	 * should print 1 */
	ಸುರಕ್ಷಿತ_waitpid(pid, &ಸ್ಥಿತಿ_2, 0);
	debug_printf("child exited. returning its exitcode:%d\n", WEXITSTATUS(ಸ್ಥಿತಿ_2));
	ಹಿಂತಿರುಗಿ WEXITSTATUS(ಸ್ಥಿತಿ_2);
}
#ಆದರೆ_ಅಂತ್ಯ /* ENABLE_HUSH_TICK */

ಸ್ಥಿರ ಶೂನ್ಯ ಸಿದ್ಧತೆ_heredoc(ರಚನೆ redir_struct *redir)
{
	ರಚನೆ fd_pair pair;
	pid_t pid;
	ಪೂರ್ಣಾಂಕ len, written;
	/* the _body_ of heredoc (misleading field name) */
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *heredoc = redir->rd_filename;
	ಅಕ್ಷರ *expanded;
#ಆದರೆ !BB_MMU
	ಅಕ್ಷರ **ಗಮ್ಯ_free;
#ಆದರೆ_ಅಂತ್ಯ

	expanded = ಖಾಲಿ;
	ಆದರೆ (!(redir->rd_dup & HEREDOC_QUOTED)) {
		expanded = encode_then_expand_string(heredoc);
		ಆದರೆ (expanded)
			heredoc = expanded;
	}
	len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(heredoc);

	close(redir->rd_fd); /* often saves dup2+close in xmove_fd */
	xpiped_pair(pair);
	xmove_fd(pair.rd, redir->rd_fd);

	/* Try writing without forking. Newer kernels have
	 * dynamically growing pipes. Must use non-blocking write! */
	ndelay_on(pair.wr);
	ಆಗ (1) {
		written = ಲಿಖ(pair.wr, heredoc, len);
		ಆದರೆ (written <= 0)
			ನಿಲ್ಲಿಸು;
		len -= written;
		ಆದರೆ (len == 0) {
			close(pair.wr);
			ಬಿಡುಗಡೆ(expanded);
			ಹಿಂತಿರುಗಿ;
		}
		heredoc += written;
	}
	ndelay_off(pair.wr);

	/* Okay, pipe buffer was not big enough */
	/* Note: we must not create a stray child (bastard? :)
	 * for the unsuspecting parent process. Child creates a grandchild
	 * and exits before parent execs the process which consumes heredoc
	 * (that exec happens after we return from this function) */
#ಆದರೆ !BB_MMU
	ಗಮ್ಯ_free = ಖಾಲಿ;
#ಆದರೆ_ಅಂತ್ಯ
	pid = xvfork();
	ಆದರೆ (pid == 0) {
		/*ಮಗು*/
		disable_restore_tty_pgrp_on_exit();
		pid = BB_MMU ? xfork() : xvfork();
		ಆದರೆ (pid != 0)
			_exit(0);
		/* grandchild */
		close(redir->rd_fd); /* read side of the pipe */
#ಆದರೆ BB_MMU
		ತುಂಬಿದ_write(pair.wr, heredoc, len); /* may loop or block */
		_exit(0);
#ಇಲ್ಲದಿದ್ದರೆ
		/* Delegate blocking writes to another process */
		xmove_fd(pair.wr, STDOUT_FILENO);
		re_execute_shell(&ಗಮ್ಯ_free, heredoc, ಖಾಲಿ, ಖಾಲಿ, ಖಾಲಿ);
#ಆದರೆ_ಅಂತ್ಯ
	}
	/*ಪೋಷಕ*/
#ಆದರೆ ENABLE_HUSH_FAST
	G.ಎಣಿಕೆ_SIGCHLD++;
//bb_error_msg("[%d] fork in setup_heredoc: G.count_SIGCHLD:%d G.handled_SIGCHLD:%d", getpid(), G.count_SIGCHLD, G.handled_SIGCHLD);
#ಆದರೆ_ಅಂತ್ಯ
	enable_restore_tty_pgrp_on_exit();
#ಆದರೆ !BB_MMU
	ಬಿಡುಗಡೆ(ಗಮ್ಯ_free);
#ಆದರೆ_ಅಂತ್ಯ
	close(pair.wr);
	ಬಿಡುಗಡೆ(expanded);
	wait(ಖಾಲಿ); /* wait till child has died */
}

ರಚನೆ squirrel {
	ಪೂರ್ಣಾಂಕ orig_fd;
	ಪೂರ್ಣಾಂಕ moved_ಗಮ್ಯ;
	/* moved_to = n: fd was moved to n; restore back to orig_fd after redir */
	/* moved_to = -1: fd was opened by redirect; close orig_fd after redir */
};

ಸ್ಥಿರ ರಚನೆ squirrel *append_squirrel(ರಚನೆ squirrel *sq, ಪೂರ್ಣಾಂಕ i, ಪೂರ್ಣಾಂಕ orig, ಪೂರ್ಣಾಂಕ moved)
{
	sq = xrealloc(sq, (i + 2) * ಗಾತ್ರ(sq[0]));
	sq[i].orig_fd = orig;
	sq[i].moved_ಗಮ್ಯ = moved;
	sq[i+1].orig_fd = -1; /* end marker */
	ಹಿಂತಿರುಗಿ sq;
}

ಸ್ಥಿರ ರಚನೆ squirrel *ಕೂಡಿಸು_squirrel(ರಚನೆ squirrel *sq, ಪೂರ್ಣಾಂಕ fd, ಪೂರ್ಣಾಂಕ avoid_fd)
{
	ಪೂರ್ಣಾಂಕ moved_ಗಮ್ಯ;
	ಪೂರ್ಣಾಂಕ i;

	i = 0;
	ಆದರೆ (sq) ಗಾಗಿ (; sq[i].orig_fd >= 0; i++) {
		/* If we collide with an already moved fd... */
		ಆದರೆ (fd == sq[i].moved_ಗಮ್ಯ) {
			moved_ಗಮ್ಯ = dup_CLOEXEC(sq[i].moved_ಗಮ್ಯ, avoid_fd);
			debug_printf_redir("redirect_fd %d: already busy, moving to %d\n", fd, moved_ಗಮ್ಯ);
			ಆದರೆ (moved_ಗಮ್ಯ < 0) {
				/* "echo 2>/dev/tty 10>&9999" testcase:
				 * We move fd 2 to 10, then discover we need to move fd 10
				 * (and not hit 9999) and the latter fails.
				 */
				ಹಿಂತಿರುಗಿ ಖಾಲಿ; /* fcntl failed */
			}
			sq[i].moved_ಗಮ್ಯ = moved_ಗಮ್ಯ;
			ಹಿಂತಿರುಗಿ sq;
		}
		ಆದರೆ (fd == sq[i].orig_fd) {
			/* Example: echo Hello >/dev/null 1>&2 */
			debug_printf_redir("redirect_fd %d: already moved\n", fd);
			ಹಿಂತಿರುಗಿ sq;
		}
	}

	/* If this fd is open, we move and remember it; if it's closed, moved_to = -1 */
	moved_ಗಮ್ಯ = dup_CLOEXEC(fd, avoid_fd);
	debug_printf_redir("redirect_fd %d: previous fd is moved to %d (-1 if it was closed)\n", fd, moved_ಗಮ್ಯ);
	ಆದರೆ (moved_ಗಮ್ಯ < 0 && errno != EBADF)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ; /* fcntl failed (not because fd is closed) */
	ಹಿಂತಿರುಗಿ append_squirrel(sq, i, fd, moved_ಗಮ್ಯ);
}

ಸ್ಥಿರ ರಚನೆ squirrel *ಕೂಡಿಸು_squirrel_closed(ರಚನೆ squirrel *sq, ಪೂರ್ಣಾಂಕ fd)
{
	ಪೂರ್ಣಾಂಕ i;

	i = 0;
	ಆದರೆ (sq) ಗಾಗಿ (; sq[i].orig_fd >= 0; i++) {
		/* If we collide with an already moved fd... */
		ಆದರೆ (fd == sq[i].orig_fd) {
			/* Examples:
			 * "echo 3>FILE 3>&- 3>FILE"
			 * "echo 3>&- 3>FILE"
			 * No need for last redirect to insert
			 * another "need to close 3" indicator.
			 */
			debug_printf_redir("redirect_fd %d: already moved or closed\n", fd);
			ಹಿಂತಿರುಗಿ sq;
		}
	}

	debug_printf_redir("redirect_fd %d: previous fd was closed\n", fd);
	ಹಿಂತಿರುಗಿ append_squirrel(sq, i, fd, -1);
}

/* fd: redirect wants this fd to be used (e.g. 3>file).
 * Move all conflicting internally used fds,
 * and remember them so that we can restore them later.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ save_fd_on_redirect(ಪೂರ್ಣಾಂಕ fd, ಪೂರ್ಣಾಂಕ avoid_fd, ರಚನೆ squirrel **sqp)
{
	ರಚನೆ squirrel *new_squirrel;

	ಆದರೆ (avoid_fd < 9) /* the important case here is that it can be -1 */
		avoid_fd = 9;

#ಆದರೆ ENABLE_HUSH_INTERACTIVE
	ಆದರೆ (fd != 0 /* don't trigger for G_interactive_fd == 0 (that's "not interactive" flag) */
	 && fd == G_interactive_fd
	) {
		/* Testcase: "ls -l /proc/$$/fd 255>&-" should work */
		G_interactive_fd = xdup_CLOEXEC_and_close(G_interactive_fd, avoid_fd);
		debug_printf_redir("redirect_fd %d: matches interactive_fd, moving it to %d\n", fd, G_interactive_fd);
		ಹಿಂತಿರುಗಿ 1; /* "we closed fd" */
	}
#ಆದರೆ_ಅಂತ್ಯ
	/* Are we called from setup_redirects(squirrel==NULL)
	 * in redirect in a [v]forked child?
	 */
	ಆದರೆ (sqp == ಖಾಲಿ) {
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
		ಹಿಂತಿರುಗಿ 0;
	}

	/* If this one of script's fds? */
	ಆದರೆ (move_HFILEs_on_redirect(fd, avoid_fd))
		ಹಿಂತಿರುಗಿ 1; /* yes. "we closed fd" (actually moved it) */

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
	ಆದರೆ (sqp == ERR_PTR) {
		/* Don't preserve redirected fds: exec is _meant_ to change these */
		//bb_error_msg("sqp == ERR_PTR: exec >FILE");
		ಹಿಂತಿರುಗಿ 0;
	}

	/* Check whether it collides with any open fds (e.g. stdio), save fds as needed */
	new_squirrel = ಕೂಡಿಸು_squirrel(*sqp, fd, avoid_fd);
	ಆದರೆ (!new_squirrel)
		ಹಿಂತಿರುಗಿ -1; /* redirect error */
	*sqp = new_squirrel;
	ಹಿಂತಿರುಗಿ 0; /* "we did not close fd" */
}

ಸ್ಥಿರ ಶೂನ್ಯ restore_redirects(ರಚನೆ squirrel *sq)
{
	ಆದರೆ (sq) {
		ಪೂರ್ಣಾಂಕ i;
		ಗಾಗಿ (i = 0; sq[i].orig_fd >= 0; i++) {
			ಆದರೆ (sq[i].moved_ಗಮ್ಯ >= 0) {
				/* We simply die on error */
				debug_printf_redir("restoring redirected fd from %d to %d\n", sq[i].moved_ಗಮ್ಯ, sq[i].orig_fd);
				xmove_fd(sq[i].moved_ಗಮ್ಯ, sq[i].orig_fd);
			} ಇಲ್ಲದಿದ್ದರೆ {
				/* cmd1 9>FILE; cmd2_should_see_fd9_closed */
				debug_printf_redir("restoring redirected fd %d: closing it\n", sq[i].orig_fd);
				close(sq[i].orig_fd);
			}
		}
		ಬಿಡುಗಡೆ(sq);
	}
	ಆದರೆ (G.HFILE_stdin
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

#ಆದರೆ ENABLE_FEATURE_SH_STANDALONE && BB_MMU
ಸ್ಥಿರ ಶೂನ್ಯ close_saved_fds_and_FILE_fds(ಶೂನ್ಯ)
{
	ಆದರೆ (G_interactive_fd)
		close(G_interactive_fd);
	close_all_HFILE_list();
}
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ internally_opened_fd(ಪೂರ್ಣಾಂಕ fd, ರಚನೆ squirrel *sq)
{
	ಪೂರ್ಣಾಂಕ i;

#ಆದರೆ ENABLE_HUSH_INTERACTIVE
	ಆದರೆ (fd == G_interactive_fd)
		ಹಿಂತಿರುಗಿ 1;
#ಆದರೆ_ಅಂತ್ಯ
	/* If this one of script's fds? */
	ಆದರೆ (fd_ಒಳ_HFILEs(fd))
		ಹಿಂತಿರುಗಿ 1;

	ಆದರೆ (sq) ಗಾಗಿ (i = 0; sq[i].orig_fd >= 0; i++) {
		ಆದರೆ (fd == sq[i].moved_ಗಮ್ಯ)
			ಹಿಂತಿರುಗಿ 1;
	}
	ಹಿಂತಿರುಗಿ 0;
}

/* sqp != NULL means we squirrel away copies of stdin, stdout,
 * and stderr if they are redirected.
 * If redirection fails, return 1. This will make caller
 * skip command execution and restore already created redirect fds.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಸಿದ್ಧತೆ_redirects(ರಚನೆ command *prog, ರಚನೆ squirrel **sqp)
{
	ರಚನೆ redir_struct *redir;

	ಗಾಗಿ (redir = prog->redirects; redir; redir = redir->ಮುಂದಿನ) {
		ಪೂರ್ಣಾಂಕ newfd;
		ಪೂರ್ಣಾಂಕ closed;

		ಆದರೆ (redir->rd_type == REDIRECT_HEREDOC2) {
			/* "rd_fd<<HERE" case */
			ಆದರೆ (save_fd_on_redirect(redir->rd_fd, /*avoid:*/ 0, sqp) < 0)
				ಹಿಂತಿರುಗಿ 1;
			/* for REDIRECT_HEREDOC2, rd_filename holds _contents_
			 * of the heredoc */
			debug_printf_redir("set heredoc '%s'\n",
					redir->rd_filename);
			ಸಿದ್ಧತೆ_heredoc(redir);
			ಮುಂದುವರಿ;
		}

		ಆದರೆ (redir->rd_dup == REDIRFD_ಗಮ್ಯ_FILE) {
			/* "rd_fd<*>file" case (<*> is <,>,>>,<>) */
			ಅಕ್ಷರ *p;
			ಪೂರ್ಣಾಂಕ ಬಹುಲಕ;

			ಆದರೆ (redir->rd_filename == ಖಾಲಿ) {
				/* Examples:
				 * "cmd >" (no filename)
				 * "cmd > <file" (2nd redirect starts too early)
				 */
				syntax_error("invalid redirect");
				ಹಿಂತಿರುಗಿ 1;
			}
			ಬಹುಲಕ = redir_ಕೋಷ್ಟಕ[redir->rd_type].ಬಹುಲಕ;
			p = expand_string_ಗಮ್ಯ_string(redir->rd_filename,
				EXP_FLAG_ESC_GLOB_CHARS, /*unbackslash:*/ 1);
			newfd = open_or_warn(p, ಬಹುಲಕ);
			ಬಿಡುಗಡೆ(p);
			ಆದರೆ (newfd < 0) {
				/* Error message from open_or_warn can be lost
				 * if stderr has been redirected, but bash
				 * and ash both lose it as well
				 * (though zsh doesn't!)
				 */
				ಹಿಂತಿರುಗಿ 1;
			}
			ಆದರೆ (newfd == redir->rd_fd && sqp
			 && sqp != ERR_PTR /* not a redirect in "exec" */
			) {
				/* open() gave us precisely the fd we wanted.
				 * This means that this fd was not busy
				 * (not opened to anywhere).
				 * Remember to close it on restore:
				 */
				*sqp = ಕೂಡಿಸು_squirrel_closed(*sqp, newfd);
				debug_printf_redir("redir to previously closed fd %d\n", newfd);
			}
		} ಇಲ್ಲದಿದ್ದರೆ {
			/* "rd_fd>&rd_dup" or "rd_fd>&-" case */
			newfd = redir->rd_dup;
		}

		ಆದರೆ (newfd == redir->rd_fd)
			ಮುಂದುವರಿ;

		/* if "N>FILE": move newfd to redir->rd_fd */
		/* if "N>&M": dup newfd to redir->rd_fd */
		/* if "N>&-": close redir->rd_fd (newfd is REDIRFD_CLOSE) */

		closed = save_fd_on_redirect(redir->rd_fd, /*avoid:*/ newfd, sqp);
		ಆದರೆ (closed < 0)
			ಹಿಂತಿರುಗಿ 1; /* error */
		ಆದರೆ (newfd == REDIRFD_CLOSE) {
			/* "N>&-" means "close me" */
			ಆದರೆ (!closed) {
				/* ^^^ optimization: saving may already
				 * have closed it. If not... */
				close(redir->rd_fd);
			}
			/* Sometimes we do another close on restore, getting EBADF.
			 * Consider "echo 3>FILE 3>&-"
			 * first redirect remembers "need to close 3",
			 * and second redirect closes 3! Restore code then closes 3 again.
			 */
		} ಇಲ್ಲದಿದ್ದರೆ {
			/* if newfd is a script fd or saved fd, do not allow to use it */
			ಆದರೆ (internally_opened_fd(newfd, sqp && sqp != ERR_PTR ? *sqp : ಖಾಲಿ)) {
				bb_error_ಸಂದೇಶ("fd#%d is not open", newfd);
				ಹಿಂತಿರುಗಿ 1;
			}
			ಆದರೆ (dup2(newfd, redir->rd_fd) < 0) {
				/* "echo >&99" testcase */
				bb_perror_ಸಂದೇಶ("dup2(%d,%d)", newfd, redir->rd_fd);
				ಹಿಂತಿರುಗಿ 1;
			}
			ಆದರೆ (redir->rd_dup == REDIRFD_ಗಮ್ಯ_FILE)
				/* "rd_fd > FILE" */
				close(newfd);
			/* else: "rd_fd > rd_dup" */
		}
	}
	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಅಕ್ಷರ *ಕಂಡುಹಿಡಿ_ಒಳ_ಮಾರ್ಗ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *arg)
{
	ಅಕ್ಷರ *ret = ಖಾಲಿ;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಮಾರ್ಗ = get_local_var_ಮೌಲ್ಯ("PATH");

	ಆದರೆ (!ಮಾರ್ಗ)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	ಆಗ (1) {
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಕೊನೆ_2 = strchrnul(ಮಾರ್ಗ, ':');
		ಪೂರ್ಣಾಂಕ sz = ಕೊನೆ_2 - ಮಾರ್ಗ; /* must be int! */

		ಬಿಡುಗಡೆ(ret);
		ಆದರೆ (sz != 0) {
			ret = xasprintf("%.*s/%s", sz, ಮಾರ್ಗ, arg);
		} ಇಲ್ಲದಿದ್ದರೆ {
			/* We have xxx::yyyy in $PATH,
			 * it means "use current dir" */
			ret = xstrdup(arg);
		}
		ಆದರೆ (access(ret, F_OK) == 0)
			ನಿಲ್ಲಿಸು;

		ಆದರೆ (*ಕೊನೆ_2 == '\0') {
			ಬಿಡುಗಡೆ(ret);
			ಹಿಂತಿರುಗಿ ಖಾಲಿ;
		}
		ಮಾರ್ಗ = ಕೊನೆ_2 + 1;
	}

	ಹಿಂತಿರುಗಿ ret;
}

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ built_ಒಳ_command *ಕಂಡುಹಿಡಿ_builtin_helper(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name,
		ಸ್ಥಿರಾಂಕ ರಚನೆ built_ಒಳ_command *x,
		ಸ್ಥಿರಾಂಕ ರಚನೆ built_ಒಳ_command *ಕೊನೆ_2)
{
	ಆಗ (x != ಕೊನೆ_2) {
		ಆದರೆ (strcmp(name, x->b_cmd) != 0) {
			x++;
			ಮುಂದುವರಿ;
		}
		debug_printf_exec("found builtin '%s'\n", name);
		ಹಿಂತಿರುಗಿ x;
	}
	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ built_ಒಳ_command *ಕಂಡುಹಿಡಿ_builtin1(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name)
{
	ಹಿಂತಿರುಗಿ ಕಂಡುಹಿಡಿ_builtin_helper(name, bltins1, &bltins1[ARRAY_ಗಾತ್ರ(bltins1)]);
}
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ರಚನೆ built_ಒಳ_command *ಕಂಡುಹಿಡಿ_builtin(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name)
{
	ಸ್ಥಿರಾಂಕ ರಚನೆ built_ಒಳ_command *x = ಕಂಡುಹಿಡಿ_builtin1(name);
	ಆದರೆ (x)
		ಹಿಂತಿರುಗಿ x;
	ಹಿಂತಿರುಗಿ ಕಂಡುಹಿಡಿ_builtin_helper(name, bltins2, &bltins2[ARRAY_ಗಾತ್ರ(bltins2)]);
}

#ಆದರೆ ENABLE_HUSH_ಕೆಲಸ && ENABLE_FEATURE_TAB_COMPLETION
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ * FAST_FUNC hush_command_name(ಪೂರ್ಣಾಂಕ i)
{
	ಆದರೆ (/*i >= 0 && */ i < ARRAY_ಗಾತ್ರ(bltins1)) {
		ಹಿಂತಿರುಗಿ bltins1[i].b_cmd;
	}
	i -= ARRAY_ಗಾತ್ರ(bltins1);
	ಆದರೆ (i < ARRAY_ಗಾತ್ರ(bltins2)) {
		ಹಿಂತಿರುಗಿ bltins2[i].b_cmd;
	}
# ಆದರೆ ENABLE_HUSH_FUNCTIONS
	{
		ರಚನೆ function *funcp;
		i -= ARRAY_ಗಾತ್ರ(bltins2);
		ಗಾಗಿ (funcp = G.ತುದಿ_func; funcp; funcp = funcp->ಮುಂದಿನ) {
			ಆದರೆ (--i < 0)
				ಹಿಂತಿರುಗಿ funcp->name;
		}
	}
# endif
	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
ಸ್ಥಿರ ಶೂನ್ಯ remove_nested_vars(ಶೂನ್ಯ)
{
	ರಚನೆ variable *cur;
	ರಚನೆ variable **cur_pp;

	cur_pp = &G.ತುದಿ_var;
	ಆಗ ((cur = *cur_pp) != ಖಾಲಿ) {
		ಆದರೆ (cur->var_nest_level <= G.var_nest_level) {
			cur_pp = &cur->ಮುಂದಿನ;
			ಮುಂದುವರಿ;
		}
		/* Unexport */
		ಆದರೆ (cur->flg_export) {
			debug_printf_env("unexporting nested '%s'/%u\n", cur->varstr, cur->var_nest_level);
			bb_unsetenv(cur->varstr);
		}
		/* Remove from global list */
		*cur_pp = cur->ಮುಂದಿನ;
		/* Free */
		ಆದರೆ (!cur->max_len) {
			debug_printf_env("freeing nested '%s'/%u\n", cur->varstr, cur->var_nest_level);
			ಬಿಡುಗಡೆ(cur->varstr);
		}
		ಬಿಡುಗಡೆ(cur);
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ enter_var_nest_level(ಶೂನ್ಯ)
{
	G.var_nest_level++;
	debug_printf_env("var_nest_level++ %u\n", G.var_nest_level);

	/* Try:	f() { echo -n .; f; }; f
	 * struct variable::var_nest_level is uint16_t,
	 * thus limiting recursion to < 2^16.
	 * In any case, with 8 Mbyte stack SEGV happens
	 * not too long after 2^16 recursions anyway.
	 */
	ಆದರೆ (G.var_nest_level > 0xff00)
		bb_error_ಸಂದೇಶ_and_die("fatal recursion (depth %u)", G.var_nest_level);
}

ಸ್ಥಿರ ಶೂನ್ಯ leave_var_nest_level(ಶೂನ್ಯ)
{
	G.var_nest_level--;
	debug_printf_env("var_nest_level-- %u\n", G.var_nest_level);
	ಆದರೆ (HUSH_DEBUG && (ಪೂರ್ಣಾಂಕ)G.var_nest_level < 0)
		bb_simple_error_ಸಂದೇಶ_and_die("BUG: nesting underflow");

	remove_nested_vars();
}
#ಆದರೆ_ಅಂತ್ಯ /* __U_BOOT__ */

#ಆದರೆ ENABLE_HUSH_FUNCTIONS
ಸ್ಥಿರ ರಚನೆ function **ಕಂಡುಹಿಡಿ_function_ಸ್ಥಾನ(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name)
{
	ರಚನೆ function *funcp;
	ರಚನೆ function **funcpp = &G.ತುದಿ_func;

	ಆಗ ((funcp = *funcpp) != ಖಾಲಿ) {
		ಆದರೆ (strcmp(name, funcp->name) == 0) {
			debug_printf_exec("found function '%s'\n", name);
			ನಿಲ್ಲಿಸು;
		}
		funcpp = &funcp->ಮುಂದಿನ;
	}
	ಹಿಂತಿರುಗಿ funcpp;
}

ಸ್ಥಿರ ALWAYS_INLINE ಸ್ಥಿರಾಂಕ ರಚನೆ function *ಕಂಡುಹಿಡಿ_function(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name)
{
	ಸ್ಥಿರಾಂಕ ರಚನೆ function *funcp = *ಕಂಡುಹಿಡಿ_function_ಸ್ಥಾನ(name);
	ಹಿಂತಿರುಗಿ funcp;
}

/* Note: takes ownership on name ptr */
ಸ್ಥಿರ ರಚನೆ function *new_function(ಅಕ್ಷರ *name)
{
	ರಚನೆ function **funcpp = ಕಂಡುಹಿಡಿ_function_ಸ್ಥಾನ(name);
	ರಚನೆ function *funcp = *funcpp;

	ಆದರೆ (funcp != ಖಾಲಿ) {
		ರಚನೆ command *cmd = funcp->ಪೋಷಕ_cmd;
		debug_printf_exec("func %p parent_cmd %p\n", funcp, cmd);
		ಆದರೆ (!cmd) {
			debug_printf_exec("freeing & replacing function '%s'\n", funcp->name);
			ಬಿಡುಗಡೆ(funcp->name);
			/* Note: if !funcp->body, do not free body_as_string!
			 * This is a special case of "-F name body" function:
			 * body_as_string was not malloced! */
			ಆದರೆ (funcp->body) {
				free_pipe_list(funcp->body);
# ಆದರೆ !BB_MMU
				ಬಿಡುಗಡೆ(funcp->body_as_string);
# endif
			}
		} ಇಲ್ಲದಿದ್ದರೆ {
			debug_printf_exec("reinserting in tree & replacing function '%s'\n", funcp->name);
			cmd->argv[0] = funcp->name;
			cmd->ಗುಂಪು = funcp->body;
# ಆದರೆ !BB_MMU
			cmd->ಗುಂಪು_as_string = funcp->body_as_string;
# endif
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		debug_printf_exec("remembering new function '%s'\n", name);
		funcp = *funcpp = xzalloc(ಗಾತ್ರ(*funcp));
		/*funcp->next = NULL;*/
	}

	funcp->name = name;
	ಹಿಂತಿರುಗಿ funcp;
}

# ಆದರೆ ENABLE_HUSH_UNSET
ಸ್ಥಿರ ಶೂನ್ಯ unset_func(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name)
{
	ರಚನೆ function **funcpp = ಕಂಡುಹಿಡಿ_function_ಸ್ಥಾನ(name);
	ರಚನೆ function *funcp = *funcpp;

	ಆದರೆ (funcp != ಖಾಲಿ) {
		debug_printf_exec("freeing function '%s'\n", funcp->name);
		*funcpp = funcp->ಮುಂದಿನ;
		/* funcp is unlinked now, deleting it.
		 * Note: if !funcp->body, the function was created by
		 * "-F name body", do not free ->body_as_string
		 * and ->name as they were not malloced. */
		ಆದರೆ (funcp->body) {
			free_pipe_list(funcp->body);
			ಬಿಡುಗಡೆ(funcp->name);
#  ಆದರೆ !BB_MMU
			ಬಿಡುಗಡೆ(funcp->body_as_string);
#  endif
		}
		ಬಿಡುಗಡೆ(funcp);
	}
}
# endif

# ಆದರೆ BB_MMU
#ವ್ಯಾಖ್ಯಾನಿಸು exec_function(ಗಮ್ಯ_free, funcp, argv) \
	exec_function(funcp, argv)
# endif
ಸ್ಥಿರ ಶೂನ್ಯ exec_function(ಅಕ್ಷರ ***ಗಮ್ಯ_free,
		ಸ್ಥಿರಾಂಕ ರಚನೆ function *funcp,
		ಅಕ್ಷರ **argv) NORETURN;
ಸ್ಥಿರ ಶೂನ್ಯ exec_function(ಅಕ್ಷರ ***ಗಮ್ಯ_free,
		ಸ್ಥಿರಾಂಕ ರಚನೆ function *funcp,
		ಅಕ್ಷರ **argv)
{
# ಆದರೆ BB_MMU
	ಪೂರ್ಣಾಂಕ n;

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
	G_flag_return_ಒಳ_ಪ್ರಗತಿ = -1;
	enter_var_nest_level();
	IF_HUSH_LOCAL(G.func_nest_level++;)

	/* On MMU, funcp->body is always non-NULL */
	n = ಸತತ_list(funcp->body);
	_exit(n);
# ಇಲ್ಲದಿದ್ದರೆ
//?	close_saved_fds_and_FILE_fds();

//TODO: check whether "true | func_with_return" works

	re_execute_shell(ಗಮ್ಯ_free,
			funcp->body_as_string,
			G.global_argv[0],
			argv + 1,
			ಖಾಲಿ);
# endif
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಸತತ_function(ಸ್ಥಿರಾಂಕ ರಚನೆ function *funcp, ಅಕ್ಷರ **argv)
{
	ಪೂರ್ಣಾಂಕ rc;
	save_arg_t sv;
	smallint sv_flg;

	save_and_replace_G_ವಾದಗಳು(&sv, argv);

	/* "We are in function, ok to use return" */
	sv_flg = G_flag_return_ಒಳ_ಪ್ರಗತಿ;
	G_flag_return_ಒಳ_ಪ್ರಗತಿ = -1;

	/* Make "local" variables properly shadow previous ones */
	IF_HUSH_LOCAL(enter_var_nest_level();)
	IF_HUSH_LOCAL(G.func_nest_level++;)

	/* On MMU, funcp->body is always non-NULL */
# ಆದರೆ !BB_MMU
	ಆದರೆ (!funcp->body) {
		/* Function defined by -F */
		parse_and_ಸತತ_string(funcp->body_as_string);
		rc = G.ಕೊನೆಯ_exitcode;
	} ಇಲ್ಲದಿದ್ದರೆ
# endif
	{
		rc = ಸತತ_list(funcp->body);
	}

	IF_HUSH_LOCAL(G.func_nest_level--;)
	IF_HUSH_LOCAL(leave_var_nest_level();)

	G_flag_return_ಒಳ_ಪ್ರಗತಿ = sv_flg;
# ಆದರೆ ENABLE_HUSH_TRAP
	debug_printf_exec("G.return_exitcode=-1\n");
	G.return_exitcode = -1; /* invalidate stashed return value */
# endif

	restore_G_ವಾದಗಳು(&sv, argv);

	ಹಿಂತಿರುಗಿ rc;
}
#ಆದರೆ_ಅಂತ್ಯ /* ENABLE_HUSH_FUNCTIONS */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
#ಆದರೆ BB_MMU
#ವ್ಯಾಖ್ಯಾನಿಸು exec_builtin(ಗಮ್ಯ_free, x, argv) \
	exec_builtin(x, argv)
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು exec_builtin(ಗಮ್ಯ_free, x, argv) \
	exec_builtin(ಗಮ್ಯ_free, argv)
#ಆದರೆ_ಅಂತ್ಯ
ಸ್ಥಿರ ಶೂನ್ಯ exec_builtin(ಅಕ್ಷರ ***ಗಮ್ಯ_free,
		ಸ್ಥಿರಾಂಕ ರಚನೆ built_ಒಳ_command *x,
		ಅಕ್ಷರ **argv) NORETURN;
ಸ್ಥಿರ ಶೂನ್ಯ exec_builtin(ಅಕ್ಷರ ***ಗಮ್ಯ_free,
		ಸ್ಥಿರಾಂಕ ರಚನೆ built_ಒಳ_command *x,
		ಅಕ್ಷರ **argv)
{
#ಆದರೆ BB_MMU
	ಪೂರ್ಣಾಂಕ rcode;
//?	close_saved_fds_and_FILE_fds();
	rcode = x->b_function(argv);
	fflush_all();
	_exit(rcode);
#ಇಲ್ಲದಿದ್ದರೆ
	fflush_all();
	/* On NOMMU, we must never block!
	 * Example: { sleep 99 | read line; } & echo Ok
	 */
	re_execute_shell(ಗಮ್ಯ_free,
			argv[0],
			G.global_argv[0],
			G.global_argv + 1,
			argv);
#ಆದರೆ_ಅಂತ್ಯ
}
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
ಸ್ಥಿರ ಶೂನ್ಯ execvp_or_die(ಅಕ್ಷರ **argv) NORETURN;
ಸ್ಥಿರ ಶೂನ್ಯ execvp_or_die(ಅಕ್ಷರ **argv)
{
	ಪೂರ್ಣಾಂಕ e;
	debug_printf_exec("execing '%s'\n", argv[0]);
	/* Don't propagate SIG_IGN to the child */
	ಆದರೆ (SPECIAL_JOBSTOP_SIGS != 0)
		switch_off_special_sigs(G.special_sig_ಮುಖವಾಡ & SPECIAL_JOBSTOP_SIGS);
	execvp(argv[0], argv);
	e = 2;
	ಆದರೆ (errno == EACCES) e = 126;
	ಆದರೆ (errno == ENOENT) e = 127;
	bb_perror_ಸಂದೇಶ("can't execute '%s'", argv[0]);
	_exit(e);
}

#ಆದರೆ ENABLE_HUSH_ಬಹುಲಕ_X
ಸ್ಥಿರ ಶೂನ್ಯ x_ಬಹುಲಕ_print_optionally_squoted(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str)
{
	ಚಿಹ್ನೆರಹಿತ len;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *cp;

	cp = str;

	/* the set of chars which-cause-string-to-be-squoted mimics bash */
	/* test a char with: bash -c 'set -x; echo "CH"' */
	ಆದರೆ (str[strcspn(str, "\\\"'`$(){}[]<>;#&|~*?!^"
			" " "\001\002\003\004\005\006\007"
			"\010\011\012\013\014\015\016\017"
			"\020\021\022\023\024\025\026\027"
			"\030\031\032\033\034\035\036\037"
			)
		] == '\0'
	) {
		/* string has no special chars */
		x_ಬಹುಲಕ_addstr(str);
		ಹಿಂತಿರುಗಿ;
	}

	cp = str;
	ಗಾಗಿ (;;) {
		/* print '....' up to EOL or first squote */
		len = (ಪೂರ್ಣಾಂಕ)(strchrnul(cp, '\'') - cp);
		ಆದರೆ (len != 0) {
			x_ಬಹುಲಕ_addchr('\'');
			x_ಬಹುಲಕ_addblock(cp, len);
			x_ಬಹುಲಕ_addchr('\'');
			cp += len;
		}
		ಆದರೆ (*cp == '\0')
			ನಿಲ್ಲಿಸು;
		/* string contains squote(s), print them as \' */
		x_ಬಹುಲಕ_addchr('\\');
		x_ಬಹುಲಕ_addchr('\'');
		cp++;
	}
}
ಸ್ಥಿರ ಶೂನ್ಯ dump_cmd_ಒಳ_x_ಬಹುಲಕ(ಅಕ್ಷರ **argv)
{
	ಆದರೆ (G_x_ಬಹುಲಕ && argv) {
		ಚಿಹ್ನೆರಹಿತ n;

		/* "+[+++...][ cmd...]\n\0" */
		x_ಬಹುಲಕ_ಪೂರ್ವಪ್ರತ್ಯಯ();
		n = 0;
		ಆಗ (argv[n]) {
			x_ಬಹುಲಕ_addchr(' ');
			ಆದರೆ (argv[n][0] == '\0') {
				x_ಬಹುಲಕ_addchr('\'');
				x_ಬಹುಲಕ_addchr('\'');
			} ಇಲ್ಲದಿದ್ದರೆ {
				x_ಬಹುಲಕ_print_optionally_squoted(argv[n]);
			}
			n++;
		}
		x_ಬಹುಲಕ_flush();
	}
}
#ಇಲ್ಲದಿದ್ದರೆ
# define dump_cmd_ಒಳ_x_ಬಹುಲಕ(argv) ((ಶೂನ್ಯ)0)
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
#ಆದರೆ ENABLE_HUSH_COMMAND
ಸ್ಥಿರ ಶೂನ್ಯ if_command_vV_print_and_exit(ಅಕ್ಷರ opt_vV, ಅಕ್ಷರ *cmd, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *explanation)
{
	ಅಕ್ಷರ *ಗಮ್ಯ_free;

	ಆದರೆ (!opt_vV)
		ಹಿಂತಿರುಗಿ;

	ಗಮ್ಯ_free = ಖಾಲಿ;
	ಆದರೆ (!explanation) {
		ಅಕ್ಷರ *ಮಾರ್ಗ_2 = getenv("PATH");
		explanation = ಗಮ್ಯ_free = ಕಂಡುಹಿಡಿ_executable(cmd, &ಮಾರ್ಗ_2); /* path == NULL is ok */
		ಆದರೆ (!explanation)
			_exit(1); /* PROG was not found */
		ಆದರೆ (opt_vV != 'V')
			cmd = ಗಮ್ಯ_free; /* -v PROG prints "/path/to/PROG" */
	}
	ಬರೆ((opt_vV == 'V') ? "%s is %s\n" : "%s\n", cmd, explanation);
	ಬಿಡುಗಡೆ(ಗಮ್ಯ_free);
	fflush_all();
	_exit(0);
}
#ಇಲ್ಲದಿದ್ದರೆ
# define if_command_vV_print_and_exit(a,b,c) ((ಶೂನ್ಯ)0)
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */

#ಆದರೆ BB_MMU
#ವ್ಯಾಖ್ಯಾನಿಸು pseudo_exec_argv(nommu_save, argv, assignment_cnt, argv_expanded) \
	pseudo_exec_argv(argv, assignment_cnt, argv_expanded)
#ವ್ಯಾಖ್ಯಾನಿಸು pseudo_exec(nommu_save, command, argv_expanded) \
	pseudo_exec(command, argv_expanded)
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
/* Called after [v]fork() in run_pipe, or from builtin_exec.
 * Never returns.
 * Don't exit() here.  If you don't exec, use _exit instead.
 * The at_exit handlers apparently confuse the calling process,
 * in particular stdin handling. Not sure why? -- because of vfork! (vda)
 */
ಸ್ಥಿರ ಶೂನ್ಯ pseudo_exec_argv(nommu_save_t *nommu_save,
		ಅಕ್ಷರ **argv, ಪೂರ್ಣಾಂಕ assignment_cnt,
		ಅಕ್ಷರ **argv_expanded) NORETURN;
ಸ್ಥಿರ NOINLINE ಶೂನ್ಯ pseudo_exec_argv(nommu_save_t *nommu_save,
		ಅಕ್ಷರ **argv, ಪೂರ್ಣಾಂಕ assignment_cnt,
		ಅಕ್ಷರ **argv_expanded)
{
	ಸ್ಥಿರಾಂಕ ರಚನೆ built_ಒಳ_command *x;
	ರಚನೆ variable **sv_shadowed;
	ಅಕ್ಷರ **new_env;
	IF_HUSH_COMMAND(ಅಕ್ಷರ opt_vV = 0;)
	IF_HUSH_FUNCTIONS(ಸ್ಥಿರಾಂಕ ರಚನೆ function *funcp;)

	new_env = expand_assignments(argv, assignment_cnt);
	dump_cmd_ಒಳ_x_ಬಹುಲಕ(new_env);

	ಆದರೆ (!argv[assignment_cnt]) {
		/* Case when we are here: ... | var=val | ...
		 * (note that we do not exit early, i.e., do not optimize out
		 * expand_assignments(): think about ... | var=`sleep 1` | ...
		 */
		free_ಅಕ್ಷರಸರಣಿಗಳು(new_env);
		_exit_SUCCESS();
	}

	sv_shadowed = G.shadowed_vars_pp;
#ಆದರೆ BB_MMU
	G.shadowed_vars_pp = ಖಾಲಿ; /* "don't save, free them instead" */
#ಇಲ್ಲದಿದ್ದರೆ
	G.shadowed_vars_pp = &nommu_save->old_vars;
	G.var_nest_level++;
#ಆದರೆ_ಅಂತ್ಯ
	ಗಣ_vars_and_save_old(new_env);
	G.shadowed_vars_pp = sv_shadowed;

	ಆದರೆ (argv_expanded) {
		argv = argv_expanded;
	} ಇಲ್ಲದಿದ್ದರೆ {
		argv = expand_strvec_ಗಮ್ಯ_strvec(argv + assignment_cnt);
#ಆದರೆ !BB_MMU
		nommu_save->argv = argv;
#ಆದರೆ_ಅಂತ್ಯ
	}
	dump_cmd_ಒಳ_x_ಬಹುಲಕ(argv);

#ಆದರೆ ENABLE_FEATURE_SH_STANDALONE || BB_MMU
	ಆದರೆ (strchr(argv[0], '/') != ಖಾಲಿ)
		ಹೋಗು skip;
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ ENABLE_HUSH_FUNCTIONS
	/* Check if the command matches any functions (this goes before bltins) */
	funcp = ಕಂಡುಹಿಡಿ_function(argv[0]);
	ಆದರೆ (funcp)
		exec_function(&nommu_save->argv_from_re_execing, funcp, argv);
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ ENABLE_HUSH_COMMAND
	/* "command BAR": run BAR without looking it up among functions
	 * "command -v BAR": print "BAR" or "/path/to/BAR"; or exit 1
	 * "command -V BAR": print "BAR is {a function,a shell builtin,/path/to/BAR}"
	 */
	ಆಗ (strcmp(argv[0], "command") == 0 && argv[1]) {
		ಅಕ್ಷರ *p;

		argv++;
		p = *argv;
		ಆದರೆ (p[0] != '-' || !p[1])
			ಮುಂದುವರಿ; /* bash allows "command command command [-OPT] BAR" */

		ಗಾಗಿ (;;) {
			p++;
			ಆಯ್ಕೆ (*p) {
			ಸ್ಥಿತಿ '\0':
				argv++;
				p = *argv;
				ಆದರೆ (p[0] != '-' || !p[1])
					ಹೋಗು after_opts;
				ಮುಂದುವರಿ; /* next arg is also -opts, process it too */
			ಸ್ಥಿತಿ 'v':
			ಸ್ಥಿತಿ 'V':
				opt_vV = *p;
				ಮುಂದುವರಿ;
			ಪೂರ್ವನಿಯೋಜಿತ:
				bb_error_ಸಂದೇಶ_and_die("%s: %s: invalid option", "command", argv[0]);
			}
		}
	}
 after_opts:
# ಆದರೆ ENABLE_HUSH_FUNCTIONS
	ಆದರೆ (opt_vV && ಕಂಡುಹಿಡಿ_function(argv[0]))
		if_command_vV_print_and_exit(opt_vV, argv[0], "a function");
# endif
#ಆದರೆ_ಅಂತ್ಯ

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
	x = BB_MMU ? ಕಂಡುಹಿಡಿ_builtin(argv[0]) : ಕಂಡುಹಿಡಿ_builtin1(argv[0]);
	ಆದರೆ (x) {
		if_command_vV_print_and_exit(opt_vV, argv[0], "a shell builtin");
		exec_builtin(&nommu_save->argv_from_re_execing, x, argv);
	}

#ಆದರೆ ENABLE_FEATURE_SH_STANDALONE
	/* Check if the command matches any busybox applets */
	{
		ಪೂರ್ಣಾಂಕ a = ಕಂಡುಹಿಡಿ_applet_by_name(argv[0]);
		ಆದರೆ (a >= 0) {
			if_command_vV_print_and_exit(opt_vV, argv[0], "an applet");
# ಆದರೆ BB_MMU /* see above why on NOMMU it is not allowed */
			ಆದರೆ (APPLET_IS_NOEXEC(a)) {
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
				switch_off_special_sigs(G.special_sig_ಮುಖವಾಡ);
				debug_printf_exec("running applet '%s'\n", argv[0]);
				ಸತತ_noexec_applet_and_exit(a, argv[0], argv);
			}
# endif
			/* Re-exec ourselves */
			debug_printf_exec("re-execing applet '%s'\n", argv[0]);
			/* Don't propagate SIG_IGN to the child */
			ಆದರೆ (SPECIAL_JOBSTOP_SIGS != 0)
				switch_off_special_sigs(G.special_sig_ಮುಖವಾಡ & SPECIAL_JOBSTOP_SIGS);
			execv(bb_busybox_exec_ಮಾರ್ಗ, argv);
			/* If they called chroot or otherwise made the binary no longer
			 * executable, fall through */
		}
	}
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ ENABLE_FEATURE_SH_STANDALONE || BB_MMU
 skip:
#ಆದರೆ_ಅಂತ್ಯ
	if_command_vV_print_and_exit(opt_vV, argv[0], ಖಾಲಿ);
	execvp_or_die(argv);
}

/* Called after [v]fork() in run_pipe
 */
ಸ್ಥಿರ ಶೂನ್ಯ pseudo_exec(nommu_save_t *nommu_save,
		ರಚನೆ command *command,
		ಅಕ್ಷರ **argv_expanded) NORETURN;
ಸ್ಥಿರ ಶೂನ್ಯ pseudo_exec(nommu_save_t *nommu_save,
		ರಚನೆ command *command,
		ಅಕ್ಷರ **argv_expanded)
{
#ಆದರೆ ENABLE_HUSH_FUNCTIONS
	ಆದರೆ (command->cmd_type == CMD_FUNCDEF) {
		/* Ignore funcdefs in pipes:
		 * true | f() { cmd }
		 */
		_exit(0);
	}
#ಆದರೆ_ಅಂತ್ಯ

	ಆದರೆ (command->argv) {
		pseudo_exec_argv(nommu_save, command->argv,
				command->assignment_cnt, argv_expanded);
	}

	ಆದರೆ (command->ಗುಂಪು) {
		/* Cases when we are here:
		 * ( list )
		 * { list } &
		 * ... | ( list ) | ...
		 * ... | { list } | ...
		 */
#ಆದರೆ BB_MMU
		ಪೂರ್ಣಾಂಕ rcode;
		debug_printf_exec("pseudo_exec: run_list\n");
		reset_traps_ಗಮ್ಯ_defaults();
		rcode = ಸತತ_list(command->ಗುಂಪು);
		/* OK to leak memory by not calling free_pipe_list,
		 * since this process is about to exit */
		_exit(rcode);
#ಇಲ್ಲದಿದ್ದರೆ
		re_execute_shell(&nommu_save->argv_from_re_execing,
				command->ಗುಂಪು_as_string,
				G.global_argv[0],
				G.global_argv + 1,
				ಖಾಲಿ);
#ಆದರೆ_ಅಂತ್ಯ
	}

	/* Case when we are here: ... | >file */
	debug_printf_exec("pseudo_exec'ed null command\n");
	_exit_SUCCESS();
}

#ಆದರೆ ENABLE_HUSH_ಕೆಲಸ
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *get_cmdtext(ರಚನೆ pipe *pi)
{
	ಅಕ್ಷರ **argv;
	ಅಕ್ಷರ *p;
	ಪೂರ್ಣಾಂಕ len;

	/* This is subtle. ->cmdtext is created only on first backgrounding.
	 * (Think "cat, <ctrl-z>, fg, <ctrl-z>, fg, <ctrl-z>...." here...)
	 * On subsequent bg argv is trashed, but we won't use it */
	ಆದರೆ (pi->cmdtext)
		ಹಿಂತಿರುಗಿ pi->cmdtext;

	argv = pi->cmds[0].argv;
	ಆದರೆ (!argv) {
		pi->cmdtext = xzalloc(1);
		ಹಿಂತಿರುಗಿ pi->cmdtext;
	}
	len = 0;
	ಮಾಡು {
		len += ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(*argv) + 1;
	} ಆಗ (*++argv);
	p = xmalloc(len);
	pi->cmdtext = p;
	argv = pi->cmds[0].argv;
	ಮಾಡು {
		p = stpcpy(p, *argv);
		*p++ = ' ';
	} ಆಗ (*++argv);
	p[-1] = '\0';
	ಹಿಂತಿರುಗಿ pi->cmdtext;
}

ಸ್ಥಿರ ಶೂನ್ಯ remove_ಕೆಲಸ_from_ಕೋಷ್ಟಕ(ರಚನೆ pipe *pi)
{
	ರಚನೆ pipe *prev_pipe;

	ಆದರೆ (pi == G.ಕೆಲಸ_list) {
		G.ಕೆಲಸ_list = pi->ಮುಂದಿನ;
	} ಇಲ್ಲದಿದ್ದರೆ {
		prev_pipe = G.ಕೆಲಸ_list;
		ಆಗ (prev_pipe->ಮುಂದಿನ != pi)
			prev_pipe = prev_pipe->ಮುಂದಿನ;
		prev_pipe->ಮುಂದಿನ = pi->ಮುಂದಿನ;
	}
	G.ಕೊನೆಯ_jobid = 0;
	ಆದರೆ (G.ಕೆಲಸ_list)
		G.ಕೊನೆಯ_jobid = G.ಕೆಲಸ_list->jobid;
}

ಸ್ಥಿರ ಶೂನ್ಯ delete_ಪೂರ್ಣಗೊಂಡ_ಕೆಲಸ(ರಚನೆ pipe *pi)
{
	remove_ಕೆಲಸ_from_ಕೋಷ್ಟಕ(pi);
	free_pipe(pi);
}

ಸ್ಥಿರ ಶೂನ್ಯ clean_ಮೇಲೆ_ಕೊನೆಯ_dead_ಕೆಲಸ(ಶೂನ್ಯ)
{
	ಆದರೆ (G.ಕೆಲಸ_list && !G.ಕೆಲಸ_list->alive_cmds)
		delete_ಪೂರ್ಣಗೊಂಡ_ಕೆಲಸ(G.ಕೆಲಸ_list);
}

ಸ್ಥಿರ ಶೂನ್ಯ ಸೇರಿಸು_ಕೆಲಸ_into_ಕೋಷ್ಟಕ(ರಚನೆ pipe *pi)
{
	ರಚನೆ pipe *ಕೆಲಸ, **jobp;
	ಪೂರ್ಣಾಂಕ i;

	clean_ಮೇಲೆ_ಕೊನೆಯ_dead_ಕೆಲಸ();

	/* Find the end of the list, and find next job ID to use */
	i = 0;
	jobp = &G.ಕೆಲಸ_list;
	ಆಗ ((ಕೆಲಸ = *jobp) != ಖಾಲಿ) {
		ಆದರೆ (ಕೆಲಸ->jobid > i)
			i = ಕೆಲಸ->jobid;
		jobp = &ಕೆಲಸ->ಮುಂದಿನ;
	}
	pi->jobid = i + 1;

	/* Create a new job struct at the end */
	ಕೆಲಸ = *jobp = xmemdup(pi, ಗಾತ್ರ(*pi));
	ಕೆಲಸ->ಮುಂದಿನ = ಖಾಲಿ;
	ಕೆಲಸ->cmds = xzalloc(ಗಾತ್ರ(pi->cmds[0]) * pi->ಅಂಕೆ_cmds);
	/* Cannot copy entire pi->cmds[] vector! This causes double frees */
	ಗಾಗಿ (i = 0; i < pi->ಅಂಕೆ_cmds; i++) {
		ಕೆಲಸ->cmds[i].pid = pi->cmds[i].pid;
		/* all other fields are not used and stay zero */
	}
	ಕೆಲಸ->cmdtext = xstrdup(get_cmdtext(pi));

	ಆದರೆ (G_interactive_fd)
		ಬರೆ("[%u] %u %s\n", ಕೆಲಸ->jobid, (ಚಿಹ್ನೆರಹಿತ)ಕೆಲಸ->cmds[0].pid, ಕೆಲಸ->cmdtext);
	G.ಕೊನೆಯ_jobid = ಕೆಲಸ->jobid;
}
#ಆದರೆ_ಅಂತ್ಯ /*ಕೆಲಸ*/

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಕೆಲಸ_exited_or_stopped(ರಚನೆ pipe *pi)
{
	ಪೂರ್ಣಾಂಕ rcode, i;

	ಆದರೆ (pi->alive_cmds != pi->stopped_cmds)
		ಹಿಂತಿರುಗಿ -1;

	/* All processes in fg pipe have exited or stopped */
	rcode = 0;
	i = pi->ಅಂಕೆ_cmds;
	ಆಗ (--i >= 0) {
		rcode = pi->cmds[i].cmd_exitcode;
		/* usually last process gives overall exitstatus,
		 * but with "set -o pipefail", last *failed* process does */
		ಆದರೆ (G.o_opt[OPT_O_PIPEFAIL] == 0 || rcode != 0)
			ನಿಲ್ಲಿಸು;
	}
	IF_HAS_KEYWORDS(ಆದರೆ (pi->pi_inverted) rcode = !rcode;)
	ಹಿಂತಿರುಗಿ rcode;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ process_wait_ಫಲಿತಾಂಶ(ರಚನೆ pipe *fg_pipe, pid_t childpid, ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2)
{
#ಆದರೆ ENABLE_HUSH_ಕೆಲಸ
	ರಚನೆ pipe *pi;
#ಆದರೆ_ಅಂತ್ಯ
	ಪೂರ್ಣಾಂಕ i, dead;

	dead = WIFEXITED(ಸ್ಥಿತಿ_2) || WIFSIGNALED(ಸ್ಥಿತಿ_2);

#ಆದರೆ DEBUG_JOBS
	ಆದರೆ (WIFSTOPPED(ಸ್ಥಿತಿ_2))
		debug_printf_jobs("pid %d stopped by sig %d (exitcode %d)\n",
				childpid, WSTOPSIG(ಸ್ಥಿತಿ_2), WEXITSTATUS(ಸ್ಥಿತಿ_2));
	ಆದರೆ (WIFSIGNALED(ಸ್ಥಿತಿ_2))
		debug_printf_jobs("pid %d killed by sig %d (exitcode %d)\n",
				childpid, WTERMSIG(ಸ್ಥಿತಿ_2), WEXITSTATUS(ಸ್ಥಿತಿ_2));
	ಆದರೆ (WIFEXITED(ಸ್ಥಿತಿ_2))
		debug_printf_jobs("pid %d exited, exitcode %d\n",
				childpid, WEXITSTATUS(ಸ್ಥಿತಿ_2));
#ಆದರೆ_ಅಂತ್ಯ
	/* Were we asked to wait for a fg pipe? */
	ಆದರೆ (fg_pipe) {
		i = fg_pipe->ಅಂಕೆ_cmds;

		ಆಗ (--i >= 0) {
			ಪೂರ್ಣಾಂಕ rcode;

			debug_printf_jobs("check pid %d\n", fg_pipe->cmds[i].pid);
			ಆದರೆ (fg_pipe->cmds[i].pid != childpid)
				ಮುಂದುವರಿ;
			ಆದರೆ (dead) {
				ಪೂರ್ಣಾಂಕ ex;
				fg_pipe->cmds[i].pid = 0;
				fg_pipe->alive_cmds--;
				ex = WEXITSTATUS(ಸ್ಥಿತಿ_2);
				/* bash prints killer signal's name for *last*
				 * process in pipe (prints just newline for SIGINT/SIGPIPE).
				 * Mimic this. Example: "sleep 5" + (^\ or kill -QUIT)
				 */
				ಆದರೆ (WIFSIGNALED(ಸ್ಥಿತಿ_2)) {
					ಪೂರ್ಣಾಂಕ sig = WTERMSIG(ಸ್ಥಿತಿ_2);
#ಆದರೆ ENABLE_HUSH_ಕೆಲಸ
					ಆದರೆ (G.ಸತತ_list_level == 1
					/* ^^^^^ Do not print in nested contexts, example:
					 * echo `sleep 1; sh -c 'kill -9 $$'` - prints "137", NOT "Killed 137"
					 */
					 && i == fg_pipe->ಅಂಕೆ_cmds-1
					) {
						/* strsignal() is for bash compat. ~600 bloat versus bbox's get_signame() */
						puts(sig == SIGINT || sig == SIGPIPE ? "" : strsignal(sig));
					}
#ಆದರೆ_ಅಂತ್ಯ
					/* TODO: if (WCOREDUMP(status)) + " (core dumped)"; */
					/* MIPS has 128 sigs (1..128), if sig==128,
					 * 128 + sig would result in exitcode 256 -> 0!
					 */
					ex = 128 | sig;
				}
				fg_pipe->cmds[i].cmd_exitcode = ex;
			} ಇಲ್ಲದಿದ್ದರೆ {
				fg_pipe->stopped_cmds++;
			}
			debug_printf_jobs("fg_pipe: alive_cmds %d stopped_cmds %d\n",
					fg_pipe->alive_cmds, fg_pipe->stopped_cmds);
			rcode = ಕೆಲಸ_exited_or_stopped(fg_pipe);
			ಆದರೆ (rcode >= 0) {
/* Note: *non-interactive* bash does not continue if all processes in fg pipe
 * are stopped. Testcase: "cat | cat" in a script (not on command line!)
 * and "killall -STOP cat" */
				ಆದರೆ (G_interactive_fd) {
#ಆದರೆ ENABLE_HUSH_ಕೆಲಸ
					ಆದರೆ (fg_pipe->alive_cmds != 0)
						ಸೇರಿಸು_ಕೆಲಸ_into_ಕೋಷ್ಟಕ(fg_pipe);
#ಆದರೆ_ಅಂತ್ಯ
					ಹಿಂತಿರುಗಿ rcode;
				}
				ಆದರೆ (fg_pipe->alive_cmds == 0)
					ಹಿಂತಿರುಗಿ rcode;
			}
			/* There are still running processes in the fg_pipe */
			ಹಿಂತಿರುಗಿ -1;
		}
		/* It wasn't in fg_pipe, look for process in bg pipes */
	}

#ಆದರೆ ENABLE_HUSH_ಕೆಲಸ
	/* We were asked to wait for bg or orphaned children */
	/* No need to remember exitcode in this case */
	ಗಾಗಿ (pi = G.ಕೆಲಸ_list; pi; pi = pi->ಮುಂದಿನ) {
		ಗಾಗಿ (i = 0; i < pi->ಅಂಕೆ_cmds; i++) {
			ಆದರೆ (pi->cmds[i].pid == childpid)
				ಹೋಗು ಸಿಕ್ಕಿತು_pi_and_prognum;
		}
	}
	/* Happens when shell is used as init process (init=/bin/sh) */
	debug_printf("checkjobs: pid %d was not in our list!\n", childpid);
	ಹಿಂತಿರುಗಿ -1; /* this wasn't a process from fg_pipe */

 ಸಿಕ್ಕಿತು_pi_and_prognum:
	ಆದರೆ (dead) {
		/* child exited */
		ಪೂರ್ಣಾಂಕ rcode = WEXITSTATUS(ಸ್ಥಿತಿ_2);
		ಆದರೆ (WIFSIGNALED(ಸ್ಥಿತಿ_2))
			/* NB: not 128 + sig, MIPS has sig 128 */
			rcode = 128 | WTERMSIG(ಸ್ಥಿತಿ_2);
		pi->cmds[i].cmd_exitcode = rcode;
		ಆದರೆ (G.ಕೊನೆಯ_bg_pid == pi->cmds[i].pid)
			G.ಕೊನೆಯ_bg_pid_exitcode = rcode;
		pi->cmds[i].pid = 0;
		pi->alive_cmds--;
		ಆದರೆ (!pi->alive_cmds) {
# ಆದರೆ ENABLE_HUSH_BASH_COMPAT
			G.dead_ಕೆಲಸ_exitcode = ಕೆಲಸ_exited_or_stopped(pi);
# endif
			ಆದರೆ (G_interactive_fd) {
				ಬರೆ(ಕೆಲಸ_ಸ್ಥಿತಿ_FORMAT, pi->jobid,
						"Done", pi->cmdtext);
				delete_ಪೂರ್ಣಗೊಂಡ_ಕೆಲಸ(pi);
			} ಇಲ್ಲದಿದ್ದರೆ {
/*
 * bash deletes finished jobs from job table only in interactive mode,
 * after "jobs" cmd, or if pid of a new process matches one of the old ones
 * (see cleanup_dead_jobs(), delete_old_job(), J_NOTIFIED in bash source).
 * Testcase script: "(exit 3) & sleep 1; wait %1; echo $?" prints 3 in bash.
 * We only retain one "dead" job, if it's the single job on the list.
 * This covers most of real-world scenarios where this is useful.
 */
				ಆದರೆ (pi != G.ಕೆಲಸ_list)
					delete_ಪೂರ್ಣಗೊಂಡ_ಕೆಲಸ(pi);
			}
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* child stopped */
		pi->stopped_cmds++;
	}
#ಆದರೆ_ಅಂತ್ಯ
	ಹಿಂತಿರುಗಿ -1; /* this wasn't a process from fg_pipe */
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ checkjobs(ರಚನೆ pipe *fg_pipe, pid_t waitfor_pid)
{
	ಪೂರ್ಣಾಂಕ ಗುಣಲಕ್ಷಣಗಳು;
	ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2;
	ಪೂರ್ಣಾಂಕ rcode = 0;

	debug_printf_jobs("checkjobs %p\n", fg_pipe);

	ಗುಣಲಕ್ಷಣಗಳು = WUNTRACED;
	ಆದರೆ (fg_pipe == ಖಾಲಿ)
		ಗುಣಲಕ್ಷಣಗಳು |= WNOHANG;

	errno = 0;
#ಆದರೆ ENABLE_HUSH_FAST
	ಆದರೆ (G.handled_SIGCHLD == G.ಎಣಿಕೆ_SIGCHLD) {
//bb_error_msg("[%d] checkjobs: G.count_SIGCHLD:%d G.handled_SIGCHLD:%d children?:%d fg_pipe:%p",
//getpid(), G.count_SIGCHLD, G.handled_SIGCHLD, G.we_have_children, fg_pipe);
		/* There was neither fork nor SIGCHLD since last waitpid */
		/* Avoid doing waitpid syscall if possible */
		ಆದರೆ (!G.we_have_children) {
			errno = ECHILD;
			ಹಿಂತಿರುಗಿ -1;
		}
		ಆದರೆ (fg_pipe == ಖಾಲಿ) { /* is WNOHANG set? */
			/* We have children, but they did not exit
			 * or stop yet (we saw no SIGCHLD) */
			ಹಿಂತಿರುಗಿ 0;
		}
		/* else: !WNOHANG, waitpid will block, can't short-circuit */
	}
#ಆದರೆ_ಅಂತ್ಯ

/* Do we do this right?
 * bash-3.00# sleep 20 | false
 * <ctrl-Z pressed>
 * [3]+  Stopped          sleep 20 | false
 * bash-3.00# echo $?
 * 1   <========== bg pipe is not fully done, but exitcode is already known!
 * [hush 1.14.0: yes we do it right]
 */
	ಆಗ (1) {
		pid_t childpid;
#ಆದರೆ ENABLE_HUSH_FAST
		ಪೂರ್ಣಾಂಕ i;
		i = G.ಎಣಿಕೆ_SIGCHLD;
#ಆದರೆ_ಅಂತ್ಯ
		childpid = waitpid(-1, &ಸ್ಥಿತಿ_2, ಗುಣಲಕ್ಷಣಗಳು);
		ಆದರೆ (childpid <= 0) {
			ಆದರೆ (childpid && errno != ECHILD)
				bb_simple_perror_ಸಂದೇಶ("waitpid");
#ಆದರೆ ENABLE_HUSH_FAST
			ಇಲ್ಲದಿದ್ದರೆ { /* Until next SIGCHLD, waitpid's are useless */
				G.we_have_children = (childpid == 0);
				G.handled_SIGCHLD = i;
//bb_error_msg("[%d] checkjobs: waitpid returned <= 0, G.count_SIGCHLD:%d G.handled_SIGCHLD:%d", getpid(), G.count_SIGCHLD, G.handled_SIGCHLD);
			}
#ಆದರೆ_ಅಂತ್ಯ
			/* ECHILD (no children), or 0 (no change in children status) */
			rcode = childpid;
			ನಿಲ್ಲಿಸು;
		}
		rcode = process_wait_ಫಲಿತಾಂಶ(fg_pipe, childpid, ಸ್ಥಿತಿ_2);
		ಆದರೆ (rcode >= 0) {
			/* fg_pipe exited or stopped */
			ನಿಲ್ಲಿಸು;
		}
		ಆದರೆ (childpid == waitfor_pid) { /* "wait PID" */
			debug_printf_exec("childpid==waitfor_pid:%d status:0x%08x\n", childpid, ಸ್ಥಿತಿ_2);
			rcode = WEXITSTATUS(ಸ್ಥಿತಿ_2);
			ಆದರೆ (WIFSIGNALED(ಸ್ಥಿತಿ_2))
				rcode = 128 | WTERMSIG(ಸ್ಥಿತಿ_2);
			ಆದರೆ (WIFSTOPPED(ಸ್ಥಿತಿ_2))
				/* bash: "cmd & wait $!" and cmd stops: $? = 128 | stopsig */
				rcode = 128 | WSTOPSIG(ಸ್ಥಿತಿ_2);
			rcode++;
			ನಿಲ್ಲಿಸು; /* "wait PID" called us, give it exitcode+1 */
		}
#ಆದರೆ ENABLE_HUSH_BASH_COMPAT
		ಆದರೆ (-1 == waitfor_pid /* "wait -n" (wait for any one job) */
		 && G.dead_ಕೆಲಸ_exitcode >= 0 /* some job did finish */
		) {
			debug_printf_exec("waitfor_pid:-1\n");
			rcode = G.dead_ಕೆಲಸ_exitcode + 1;
			ನಿಲ್ಲಿಸು;
		}
#ಆದರೆ_ಅಂತ್ಯ
		/* This wasn't one of our processes, or */
		/* fg_pipe still has running processes, do waitpid again */
	} /* while (waitpid succeeds)... */

	ಹಿಂತಿರುಗಿ rcode;
}

#ಆದರೆ ENABLE_HUSH_ಕೆಲಸ
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ checkjobs_and_fg_shell(ರಚನೆ pipe *fg_pipe)
{
	pid_t p;
	ಪೂರ್ಣಾಂಕ rcode = checkjobs(fg_pipe, 0 /*(no pid to wait for)*/);
	ಆದರೆ (G_saved_tty_pgrp) {
		/* Job finished, move the shell to the foreground */
		p = getpgrp(); /* our process group id */
		debug_printf_jobs("fg'ing ourself: getpgrp()=%d\n", (ಪೂರ್ಣಾಂಕ)p);
		tcsetpgrp(G_interactive_fd, p);
	}
	ಹಿಂತಿರುಗಿ rcode;
}
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */

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
ಸ್ಥಿರ ಶೂನ್ಯ ಗಣ_G_ifs(ಶೂನ್ಯ)
{
	/* Testcase: set -- q w e; (IFS='' echo "$*"; IFS=''; echo "$*"); echo "$*"
	 * Result should be 3 lines: q w e, qwe, q w e
	 */
	ಆದರೆ (G.ifs_whitespace != G.ifs)
		ಬಿಡುಗಡೆ(G.ifs_whitespace);
	G.ifs = get_local_var_ಮೌಲ್ಯ("IFS");
	ಆದರೆ (G.ifs) {
		ಅಕ್ಷರ *p;
		G.ifs_whitespace = (ಅಕ್ಷರ*)G.ifs;
		p = skip_whitespace(G.ifs);
		ಆದರೆ (*p) {
			/* Not all $IFS is whitespace */
			ಅಕ್ಷರ *d;
			ಪೂರ್ಣಾಂಕ len = p - G.ifs;
			p = skip_non_whitespace(p);
			G.ifs_whitespace = xmalloc(len + ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(p) + 1); /* can overestimate */
			d = mempcpy(G.ifs_whitespace, G.ifs, len);
			ಆಗ (*p) {
				ಆದರೆ (ಖಾಲಿಯೇ(*p))
					*d++ = *p;
				p++;
			}
			*d = '\0';
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		G.ifs = defifs;
		G.ifs_whitespace = (ಅಕ್ಷರ*)G.ifs;
	}
}
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
#ಆದರೆ !ENABLE_HUSH_ಬಹುಲಕ_X
#ವ್ಯಾಖ್ಯಾನಿಸು redirect_and_varexp_helper(command, sqp, argv_expanded) \
	redirect_and_varexp_helper(command, sqp)
#ಆದರೆ_ಅಂತ್ಯ
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ redirect_and_varexp_helper(
		ರಚನೆ command *command,
		ರಚನೆ squirrel **sqp,
		ಅಕ್ಷರ **argv_expanded)
{
	/* Assignments occur before redirects. Try:
	 * a=`sleep 1` sleep 2 3>/qwe/rty
	 */

	ಅಕ್ಷರ **new_env = expand_assignments(command->argv, command->assignment_cnt);
	dump_cmd_ಒಳ_x_ಬಹುಲಕ(new_env);
	dump_cmd_ಒಳ_x_ಬಹುಲಕ(argv_expanded);
	/* this takes ownership of new_env[i] elements, and frees new_env: */
	ಗಣ_vars_and_save_old(new_env);

	ಹಿಂತಿರುಗಿ ಸಿದ್ಧತೆ_redirects(command, sqp);
}
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */

ಸ್ಥಿರ NOINLINE ಪೂರ್ಣಾಂಕ ಸತತ_pipe(ರಚನೆ pipe *pi)
{
	ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ null_ptr = ಖಾಲಿ;

	ಪೂರ್ಣಾಂಕ cmd_no;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	ಪೂರ್ಣಾಂಕ ಮುಂದಿನ_infd;
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */
	ರಚನೆ command *command;
	ಅಕ್ಷರ **argv_expanded;
	ಅಕ್ಷರ **argv;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	ರಚನೆ squirrel *squirrel = ಖಾಲಿ;
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */
	ಪೂರ್ಣಾಂಕ rcode;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __U_BOOT__
	/*
	 * Set rcode here to avoid returning a garbage value in the middle of
	 * the function.
	 * Also, if an error occurs, rcode value would be changed and last
	 * return will signal the error.
	 */
	rcode = 0;
#ಆದರೆ_ಅಂತ್ಯ /* __U_BOOT__ */

	debug_printf_exec("run_pipe start: members:%d\n", pi->ಅಂಕೆ_cmds);
	debug_enter();

	ಗಣ_G_ifs();

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	IF_HUSH_ಕೆಲಸ(pi->pgrp = -1;)
	pi->stopped_cmds = 0;
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */
	command = &pi->cmds[0];
	argv_expanded = ಖಾಲಿ;

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	ಆದರೆ (pi->ಅಂಕೆ_cmds != 1
	 || pi->followup == PIPE_BG
	 || command->cmd_type == CMD_SUBSHELL
	) {
		ಹೋಗು must_fork;
	}

	pi->alive_cmds = 1;
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */

	debug_printf_exec(": group:%p argv:'%s'\n",
		command->ಗುಂಪು, command->argv ? command->argv[0] : "NONE");

	ಆದರೆ (command->ಗುಂಪು) {
#ಆದರೆ ENABLE_HUSH_FUNCTIONS
		ಆದರೆ (command->cmd_type == CMD_FUNCDEF) {
			/* "executing" func () { list } */
			ರಚನೆ function *funcp;

			funcp = new_function(command->argv[0]);
			/* funcp->name is already set to argv[0] */
			funcp->body = command->ಗುಂಪು;
# ಆದರೆ !BB_MMU
			funcp->body_as_string = command->ಗುಂಪು_as_string;
			command->ಗುಂಪು_as_string = ಖಾಲಿ;
# endif
			command->ಗುಂಪು = ಖಾಲಿ;
			command->argv[0] = ಖಾಲಿ;
			debug_printf_exec("cmd %p has child func at %p\n", command, funcp);
			funcp->ಪೋಷಕ_cmd = command;
			command->ಮಗು_func = funcp;

			debug_printf_exec("run_pipe: return EXIT_SUCCESS\n");
			debug_leave();
			ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
		}
#ಆದರೆ_ಅಂತ್ಯ
		/* { list } */
		debug_printf_exec("non-subshell group\n");
		rcode = 1; /* exitcode if redir failed */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
		ಆದರೆ (ಸಿದ್ಧತೆ_redirects(command, &squirrel) == 0) {
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */
			debug_printf_exec(": run_list\n");
//FIXME: we need to pass squirrel down into run_list()
//for SH_STANDALONE case, or else this construct:
// { find /proc/self/fd; true; } >FILE; cmd2
//has no way of closing saved fd#1 for "find",
//and in SH_STANDALONE mode, "find" is not execed,
//therefore CLOEXEC on saved fd does not help.
			rcode = ಸತತ_list(command->ಗುಂಪು) & 0xff;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
		}
		restore_redirects(squirrel);
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */
		IF_HAS_KEYWORDS(ಆದರೆ (pi->pi_inverted) rcode = !rcode;)
		debug_leave();
		debug_printf_exec("run_pipe: return %d\n", rcode);
		ಹಿಂತಿರುಗಿ rcode;
	}

	argv = command->argv ? command->argv : (ಅಕ್ಷರ **) &null_ptr;
	{
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
		ಸ್ಥಿರಾಂಕ ರಚನೆ built_ಒಳ_command *x;
		IF_HUSH_FUNCTIONS(ಸ್ಥಿರಾಂಕ ರಚನೆ function *funcp;)
		IF_NOT_HUSH_FUNCTIONS(ಎಣಿಕೆ { funcp = 0 };)
		ರಚನೆ variable **sv_shadowed;
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */
		ರಚನೆ variable *old_vars;

#ಆದರೆ ENABLE_HUSH_LINENO_VAR
		G.execute_lineno = command->lineno;
#ಆದರೆ_ಅಂತ್ಯ

		ಆದರೆ (argv[command->assignment_cnt] == ಖಾಲಿ) {
			/* Assignments, but no command.
			 * Ensure redirects take effect (that is, create files).
			 * Try "a=t >file"
			 */
			ಚಿಹ್ನೆರಹಿತ i;
			G.expand_exitcode = 0;
 only_assignments:
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
			rcode = ಸಿದ್ಧತೆ_redirects(command, &squirrel);
			restore_redirects(squirrel);
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */

			/* Set shell variables */
			i = 0;
			ಆಗ (i < command->assignment_cnt) {
				ಅಕ್ಷರ *p = expand_string_ಗಮ್ಯ_string(argv[i],
						EXP_FLAG_ESC_GLOB_CHARS,
						/*unbackslash:*/ 1
				);
#ಆದರೆ ENABLE_HUSH_ಬಹುಲಕ_X
				ಆದರೆ (G_x_ಬಹುಲಕ) {
					ಅಕ್ಷರ *eq;
					ಆದರೆ (i == 0)
						x_ಬಹುಲಕ_ಪೂರ್ವಪ್ರತ್ಯಯ();
					x_ಬಹುಲಕ_addchr(' ');
					eq = strchrnul(p, '=');
					ಆದರೆ (*eq) eq++;
					x_ಬಹುಲಕ_addblock(p, (eq - p));
					x_ಬಹುಲಕ_print_optionally_squoted(eq);
					x_ಬಹುಲಕ_flush();
				}
#ಆದರೆ_ಅಂತ್ಯ
				debug_printf_env("set shell var:'%s'->'%s'\n", *argv, p);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
				ಆದರೆ (ಗಣ_local_var0(p)) {
#ಇಲ್ಲದಿದ್ದರೆ /* __U_BOOT__ */
				ಆದರೆ (ಗಣ_local_var_modern(p, /*flag:*/ 0)) {
#ಆದರೆ_ಅಂತ್ಯ
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
			ಆದರೆ (rcode == 0)
				rcode = G.expand_exitcode;
			IF_HAS_KEYWORDS(ಆದರೆ (pi->pi_inverted) rcode = !rcode;)
			debug_leave();
			debug_printf_exec("run_pipe: return %d\n", rcode);
			ಹಿಂತಿರುಗಿ rcode;
		}

		/* Expand the rest into (possibly) many strings each */
#ಆದರೆ defined(CMD_TEST2_SINGLEWORD_NOGLOB)
		ಆದರೆ (command->cmd_type == CMD_TEST2_SINGLEWORD_NOGLOB)
			argv_expanded = expand_strvec_ಗಮ್ಯ_strvec_singleword_noglob(argv + command->assignment_cnt);
		ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ defined(CMD_SINGLEWORD_NOGLOB)
		ಆದರೆ (command->cmd_type == CMD_SINGLEWORD_NOGLOB)
			argv_expanded = expand_strvec_ಗಮ್ಯ_strvec_singleword_noglob(argv + command->assignment_cnt);
		ಇಲ್ಲದಿದ್ದರೆ
#ಆದರೆ_ಅಂತ್ಯ
			argv_expanded = expand_strvec_ಗಮ್ಯ_strvec(argv + command->assignment_cnt);

		/* If someone gives us an empty string: `cmd with empty output` */
		ಆದರೆ (!argv_expanded[0]) {
			ಬಿಡುಗಡೆ(argv_expanded);
			/* `false` still has to set exitcode 1 */
			G.expand_exitcode = G.ಕೊನೆಯ_exitcode;
			ಹೋಗು only_assignments;
		}

		old_vars = ಖಾಲಿ;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
		sv_shadowed = G.shadowed_vars_pp;

		/* Check if argv[0] matches any functions (this goes before bltins) */
		IF_HUSH_FUNCTIONS(funcp = ಕಂಡುಹಿಡಿ_function(argv_expanded[0]);)
		IF_HUSH_FUNCTIONS(x = ಖಾಲಿ;)
		IF_HUSH_FUNCTIONS(ಆದರೆ (!funcp))
			x = ಕಂಡುಹಿಡಿ_builtin(argv_expanded[0]);
		ಆದರೆ (x || funcp) {
			ಆದರೆ (x && x->b_function == builtin_exec && argv_expanded[1] == ಖಾಲಿ) {
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

				ಹೋಗು clean_ಮೇಲೆ_and_ret1;
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
			ಆದರೆ (rcode == 0) {
				ಆದರೆ (!funcp) {
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
#ಆದರೆ ENABLE_HUSH_FUNCTIONS
				ಇಲ್ಲದಿದ್ದರೆ {
					debug_printf_exec(": function '%s' '%s'...\n",
						funcp->name, argv_expanded[1]);
					rcode = ಸತತ_function(funcp, argv_expanded) & 0xff;
					/*
					 * But do collect *to old_vars list* vars shadowed
					 * within function execution. To that end, restore
					 * this pointer _after_ function run:
					 */
					G.shadowed_vars_pp = sv_shadowed;
				}
#ಆದರೆ_ಅಂತ್ಯ
			}
		} ಇಲ್ಲದಿದ್ದರೆ
		ಆದರೆ (ENABLE_FEATURE_SH_NOFORK && ಅಂಕೆ_APPLETS > 1) {
			ಪೂರ್ಣಾಂಕ n = ಕಂಡುಹಿಡಿ_applet_by_name(argv_expanded[0]);
			ಆದರೆ (n < 0 || !APPLET_IS_NOFORK(n))
				ಹೋಗು must_fork;

			enter_var_nest_level();
			/* Collect all variables "shadowed" by helper into old_vars list */
			G.shadowed_vars_pp = &old_vars;
			rcode = redirect_and_varexp_helper(command, &squirrel, argv_expanded);
			G.shadowed_vars_pp = sv_shadowed;

			ಆದರೆ (rcode == 0) {
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
				rcode = ಸತತ_nofork_applet(n, argv_expanded);
			}
		} ಇಲ್ಲದಿದ್ದರೆ
			ಹೋಗು must_fork;

		restore_redirects(squirrel);
 clean_ಮೇಲೆ_and_ret1:
		leave_var_nest_level();
		ಕೂಡಿಸು_vars(old_vars);

		/*
		 * Try "usleep 99999999" + ^C + "echo $?"
		 * with FEATURE_SH_NOFORK=y.
		 */
		ಆದರೆ (!funcp) {
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
			ಆದರೆ (sigismember(&G.pending_ಗಣ, SIGINT))
				rcode = 128 | SIGINT;
		}
		ಬಿಡುಗಡೆ(argv_expanded);
		IF_HAS_KEYWORDS(ಆದರೆ (pi->pi_inverted) rcode = !rcode;)
		debug_leave();
		debug_printf_exec("run_pipe return %d\n", rcode);
		ಹಿಂತಿರುಗಿ rcode;
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */
	}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
 must_fork:
	/* NB: argv_expanded may already be created, and that
	 * might include `cmd` runs! Do not rerun it! We *must*
	 * use argv_expanded if it's non-NULL */

	/* Going to fork a child per each pipe member */
	pi->alive_cmds = 0;
	ಮುಂದಿನ_infd = 0;
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */

	cmd_no = 0;
	ಆಗ (cmd_no < pi->ಅಂಕೆ_cmds) {
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
		ರಚನೆ fd_pair pipefds;
#ಆದರೆ !BB_MMU
		ಪೂರ್ಣಾಂಕ sv_var_nest_level = G.var_nest_level;
		ಅಸ್ಥಿರ nommu_save_t nommu_save;
		nommu_save.old_vars = ಖಾಲಿ;
		nommu_save.argv = ಖಾಲಿ;
		nommu_save.argv_from_re_execing = ಖಾಲಿ;
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */
		command = &pi->cmds[cmd_no];
		cmd_no++;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __U_BOOT__
		/* Replace argv and argc by expanded if it exists. */
		ಆದರೆ (argv_expanded) {
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
			command->argc = list_ಗಾತ್ರ(argv_expanded);
			command->argv = argv_expanded;
		}
#ಆದರೆ_ಅಂತ್ಯ /* __U_BOOT__ */
			ಆದರೆ (command->argv) {
			debug_printf_exec(": pipe member '%s' '%s'...\n",
					command->argv[0], command->argv[1]);
		} ಇಲ್ಲದಿದ್ದರೆ {
			debug_printf_exec(": pipe member with no argv\n");
		}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
		/* pipes are inserted between pairs of commands */
		pipefds.rd = 0;
		pipefds.wr = 1;
		ಆದರೆ (cmd_no < pi->ಅಂಕೆ_cmds)
			xpiped_pair(pipefds);

#ಆದರೆ ENABLE_HUSH_LINENO_VAR
		G.execute_lineno = command->lineno;
#ಆದರೆ_ಅಂತ್ಯ

		command->pid = BB_MMU ? fork() : vfork();
		ಆದರೆ (!command->pid) { /*ಮಗು*/
#ಆದರೆ ENABLE_HUSH_ಕೆಲಸ
			disable_restore_tty_pgrp_on_exit();
			CLEAR_RANDOM_T(&G.random_gen); /* or else $RANDOM repeats in child */

			/* Every child adds itself to new process group
			 * with pgid == pid_of_first_child_in_pipe */
			ಆದರೆ (G.ಸತತ_list_level == 1 && G_interactive_fd) {
				pid_t pgrp;
				pgrp = pi->pgrp;
				ಆದರೆ (pgrp < 0) /* true for 1st process only */
					pgrp = getpid();
				ಆದರೆ (setpgid(0, pgrp) == 0
				 && pi->followup != PIPE_BG
				 && G_saved_tty_pgrp /* we have ctty */
				) {
					/* We do it in *every* child, not just first,
					 * to avoid races */
					tcsetpgrp(G_interactive_fd, pgrp);
				}
			}
#ಆದರೆ_ಅಂತ್ಯ
			ಆದರೆ (pi->alive_cmds == 0 && pi->followup == PIPE_BG) {
				/* 1st cmd in backgrounded pipe
				 * should have its stdin /dev/null'ed */
				close(0);
				ಆದರೆ (open(bb_dev_null, O_RDONLY))
					xopen("/", O_RDONLY);
			} ಇಲ್ಲದಿದ್ದರೆ {
				xmove_fd(ಮುಂದಿನ_infd, 0);
			}
			xmove_fd(pipefds.wr, 1);
			ಆದರೆ (pipefds.rd > 1)
				close(pipefds.rd);
			/* Like bash, explicit redirects override pipes,
			 * and the pipe fd (fd#1) is available for dup'ing:
			 * "cmd1 2>&1 | cmd2": fd#1 is duped to fd#2, thus stderr
			 * of cmd1 goes into pipe.
			 */
			ಆದರೆ (ಸಿದ್ಧತೆ_redirects(command, ಖಾಲಿ)) {
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
#ಆದರೆ ENABLE_HUSH_FAST
		G.ಎಣಿಕೆ_SIGCHLD++;
//bb_error_msg("[%d] fork in run_pipe: G.count_SIGCHLD:%d G.handled_SIGCHLD:%d", getpid(), G.count_SIGCHLD, G.handled_SIGCHLD);
#ಆದರೆ_ಅಂತ್ಯ
		enable_restore_tty_pgrp_on_exit();
#ಆದರೆ !BB_MMU
		/* Clean up after vforked child */
		ಬಿಡುಗಡೆ(nommu_save.argv);
		ಬಿಡುಗಡೆ(nommu_save.argv_from_re_execing);
		G.var_nest_level = sv_var_nest_level;
		remove_nested_vars();
		ಕೂಡಿಸು_vars(nommu_save.old_vars);
#ಆದರೆ_ಅಂತ್ಯ
		ಬಿಡುಗಡೆ(argv_expanded);
		argv_expanded = ಖಾಲಿ;
		ಆದರೆ (command->pid < 0) { /* [v]fork failed */
			/* Clearly indicate, was it fork or vfork */
			bb_simple_perror_ಸಂದೇಶ(BB_MMU ? "vfork"+1 : "vfork");
		} ಇಲ್ಲದಿದ್ದರೆ {
			pi->alive_cmds++;
#ಆದರೆ ENABLE_HUSH_ಕೆಲಸ
			/* Second and next children need to know pid of first one */
			ಆದರೆ (pi->pgrp < 0)
				pi->pgrp = command->pid;
#ಆದರೆ_ಅಂತ್ಯ
		}

		ಆದರೆ (cmd_no > 1)
			close(ಮುಂದಿನ_infd);
		ಆದರೆ (cmd_no < pi->ಅಂಕೆ_cmds)
			close(pipefds.wr);
		/* Pass read (output) pipe end to next iteration */
		ಮುಂದಿನ_infd = pipefds.rd;
#ಇಲ್ಲದಿದ್ದರೆ /* __U_BOOT__ */
		/* Process the command */
		rcode = cmd_process(G.do_repeat ? CMD_FLAG_REPEAT : 0,
				    command->argc, command->argv,
				    &(G.flag_repeat), ಖಾಲಿ);

		ಆದರೆ (argv_expanded) {
			/*
			 * expand_strvec_to_strvec() allocates memory to expand
			 * argv, we need to free it.
			 */
			ಬಿಡುಗಡೆ(argv_expanded);

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
#ಆದರೆ_ಅಂತ್ಯ /* __U_BOOT__ */
	}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	ಆದರೆ (!pi->alive_cmds) {
		debug_leave();
		debug_printf_exec("run_pipe return 1 (all forks failed, no children)\n");
		ಹಿಂತಿರುಗಿ 1;
	}
#ಆದರೆ_ಅಂತ್ಯ /* __U_BOOT__ */

	debug_leave();
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	debug_printf_exec("run_pipe return -1 (%u children started)\n", pi->alive_cmds);
	ಹಿಂತಿರುಗಿ -1;
#ಇಲ್ಲದಿದ್ದರೆ /* __U_BOOT__ */
	IF_HAS_KEYWORDS(ಆದರೆ (pi->pi_inverted) rcode = !rcode;)
	debug_printf_exec("run_pipe return %d\n", rcode);
	ಹಿಂತಿರುಗಿ rcode;
#ಆದರೆ_ಅಂತ್ಯ /* __U_BOOT__ */
}

/* NB: called by pseudo_exec, and therefore must not modify any
 * global data until exec/_exit (we can be a child after vfork!) */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಸತತ_list(ರಚನೆ pipe *pi)
{
#ಆದರೆ ENABLE_HUSH_CASE
	ಅಕ್ಷರ *case_ಶಬ್ದ = ಖಾಲಿ;
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_HUSH_LOOPS
	ರಚನೆ pipe *loop_ತುದಿ = ಖಾಲಿ;
	ಅಕ್ಷರ **for_lcur = ಖಾಲಿ;
	ಅಕ್ಷರ **for_list = ಖಾಲಿ;
#ಆದರೆ_ಅಂತ್ಯ
	smallint ಕೊನೆಯ_followup;
	smalluint rcode;
#ಆದರೆ ENABLE_HUSH_IF || ENABLE_HUSH_CASE
	smalluint cond_ಸಂಕೇತ = 0;
#ಇಲ್ಲದಿದ್ದರೆ
	ಎಣಿಕೆ { cond_ಸಂಕೇತ = 0 };
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ HAS_KEYWORDS
	smallint rword;      /* RES_foo */
	smallint ಕೊನೆಯ_rword; /* ditto */
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	debug_printf_exec("run_list lvl %d start\n", G.ಸತತ_list_level);
	debug_enter();
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */

	ಗಣ_G_ifs();

#ಆದರೆ ENABLE_HUSH_LOOPS
	/* Check syntax for "for" */
	{
		ರಚನೆ pipe *cpipe;
		ಗಾಗಿ (cpipe = pi; cpipe; cpipe = cpipe->ಮುಂದಿನ) {
			ಆದರೆ (cpipe->res_ಶಬ್ದ != RES_FOR && cpipe->res_ಶಬ್ದ != RES_ಒಳ)
				ಮುಂದುವರಿ;
			/* current word is FOR or IN (BOLD in comments below) */
			ಆದರೆ (cpipe->ಮುಂದಿನ == ಖಾಲಿ) {
				syntax_error("malformed for");
				debug_leave();
				debug_printf_exec("run_list lvl %d return 1\n", G.ಸತತ_list_level);
				ಹಿಂತಿರುಗಿ 1;
			}
			/* "FOR v; do ..." and "for v IN a b; do..." are ok */
			ಆದರೆ (cpipe->ಮುಂದಿನ->res_ಶಬ್ದ == RES_DO)
				ಮುಂದುವರಿ;
			/* next word is not "do". It must be "in" then ("FOR v in ...") */
			ಆದರೆ (cpipe->res_ಶಬ್ದ == RES_ಒಳ /* "for v IN a b; not_do..."? */
			 || cpipe->ಮುಂದಿನ->res_ಶಬ್ದ != RES_ಒಳ /* FOR v not_do_and_not_in..."? */
			) {
				syntax_error("malformed for");
				debug_leave();
				debug_printf_exec("run_list lvl %d return 1\n", G.ಸತತ_list_level);
				ಹಿಂತಿರುಗಿ 1;
			}
		}
	}
#ಆದರೆ_ಅಂತ್ಯ

	/* Past this point, all code paths should jump to ret: label
	 * in order to return, no direct "return" statements please.
	 * This helps to ensure that no memory is leaked. */

#ಆದರೆ ENABLE_HUSH_ಕೆಲಸ
	G.ಸತತ_list_level++;
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ HAS_KEYWORDS
	rword = RES_NONE;
	ಕೊನೆಯ_rword = RES_XXXX;
#ಆದರೆ_ಅಂತ್ಯ
	ಕೊನೆಯ_followup = PIPE_SEQ;
	rcode = G.ಕೊನೆಯ_exitcode;

	/* Go through list of pipes, (maybe) executing them. */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	ಗಾಗಿ (; pi; pi = IF_HUSH_LOOPS(rword == RES_ಮುಗಿದ ? loop_ತುದಿ : ) pi->ಮುಂದಿನ) {
#ಇಲ್ಲದಿದ್ದರೆ /* __U_BOOT__ */
	ಗಾಗಿ (; pi; pi = rword == RES_ಮುಗಿದ ? loop_ತುದಿ : pi->ಮುಂದಿನ) {
#ಆದರೆ_ಅಂತ್ಯ /* __U_BOOT__ */
		ಪೂರ್ಣಾಂಕ r;
		ಪೂರ್ಣಾಂಕ sv_errexit_ಆಳ;

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
		ಆದರೆ (G.flag_SIGINT)
			ನಿಲ್ಲಿಸು;
		ಆದರೆ (G_flag_return_ಒಳ_ಪ್ರಗತಿ == 1)
			ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */

		IF_HAS_KEYWORDS(rword = pi->res_ಶಬ್ದ;)
		debug_printf_exec(": rword:%d cond_code:%d last_rword:%d\n",
				rword, cond_ಸಂಕೇತ, ಕೊನೆಯ_rword);

		sv_errexit_ಆಳ = G.errexit_ಆಳ;
		ಆದರೆ (
#ಆದರೆ ENABLE_HUSH_IF
		    rword == RES_IF || rword == RES_ELIF ||
#ಆದರೆ_ಅಂತ್ಯ
		    pi->followup != PIPE_SEQ
		) {
			G.errexit_ಆಳ++;
		}
#ಆದರೆ ENABLE_HUSH_LOOPS
		ಆದರೆ ((rword == RES_WHILE || rword == RES_UNTIL || rword == RES_FOR)
		 && loop_ತುದಿ == ಖಾಲಿ /* avoid bumping G.depth_of_loop twice */
		) {
			/* start of a loop: remember where loop starts */
			loop_ತುದಿ = pi;
			G.ಆಳ_ದ_loop++;
		}
#ಆದರೆ_ಅಂತ್ಯ
		/* Still in the same "if...", "then..." or "do..." branch? */
		ಆದರೆ (IF_HAS_KEYWORDS(rword == ಕೊನೆಯ_rword &&) 1) {
			ಆದರೆ ((rcode == 0 && ಕೊನೆಯ_followup == PIPE_OR)
			 || (rcode != 0 && ಕೊನೆಯ_followup == PIPE_AND)
			) {
				/* It is "<true> || CMD" or "<false> && CMD"
				 * and we should not execute CMD */
				debug_printf_exec("skipped cmd because of || or &&\n");
				ಕೊನೆಯ_followup = pi->followup;
				ಹೋಗು dont_check_jobs_but_continue;
			}
		}
		ಕೊನೆಯ_followup = pi->followup;
#ಆದರೆ ENABLE_HUSH_IF
		ಆದರೆ (cond_ಸಂಕೇತ != 0) {
			ಆದರೆ (rword == RES_THEN) {
				/* if false; then ... fi has exitcode 0! */
				G.ಕೊನೆಯ_exitcode = rcode = EXIT_SUCCESS;
				/* "if <false> THEN cmd": skip cmd */
				debug_printf_exec("skipped THEN cmd because IF condition was false\n");
				ಕೊನೆಯ_rword = rword;
				ಮುಂದುವರಿ;
			}
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಆದರೆ (rword == RES_ELSE
			 || (rword == RES_ELIF && ಕೊನೆಯ_rword != RES_ELIF)
			) {
				/* "if <true> then ... ELSE/ELIF cmd":
				 * skip cmd and all following ones */
				debug_printf_exec("skipped ELSE/ELIF branch because IF condition was true\n");
				ನಿಲ್ಲಿಸು;
			}
			//if (rword == RES_THEN): "if <true> THEN cmd", run cmd (fall through)
		}
#ಆದರೆ_ಅಂತ್ಯ
		IF_HAS_KEYWORDS(ಕೊನೆಯ_rword = rword;)
#ಆದರೆ ENABLE_HUSH_LOOPS
		ಆದರೆ (rword == RES_FOR) { /* && pi->num_cmds - always == 1 */
			ಆದರೆ (!for_lcur) {
				/* first loop through for */

				ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ encoded_dollar_at[] ALIGN1 = {
					SPECIAL_VAR_ಚಿಹ್ನೆ, '@' | 0x80, SPECIAL_VAR_ಚಿಹ್ನೆ, '\0'
				}; /* encoded representation of "$@" */
				ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ encoded_dollar_at_argv[] ALIGN_PTR = {
					encoded_dollar_at, ಖಾಲಿ
				}; /* argv list with one element: "$@" */
				ಅಕ್ಷರ **vals;

				G.ಕೊನೆಯ_exitcode = rcode = EXIT_SUCCESS;
				vals = (ಅಕ್ಷರ**)encoded_dollar_at_argv;
				ಆದರೆ (pi->ಮುಂದಿನ->res_ಶಬ್ದ == RES_ಒಳ) {
					/* if no variable values after "in" we skip "for" */
					ಆದರೆ (!pi->ಮುಂದಿನ->cmds[0].argv) {
						debug_printf_exec(": null FOR: exitcode EXIT_SUCCESS\n");
						ನಿಲ್ಲಿಸು;
					}
					vals = pi->ಮುಂದಿನ->cmds[0].argv;
				} /* else: "for var; do..." -> assume "$@" list */
				/* create list of variable values */
				debug_print_ಅಕ್ಷರಸರಣಿಗಳು("for_list made from", vals);
				for_list = expand_strvec_ಗಮ್ಯ_strvec(vals);
				for_lcur = for_list;
				debug_print_ಅಕ್ಷರಸರಣಿಗಳು("for_list", for_list);
			}
			ಆದರೆ (!*for_lcur) {
				/* "for" loop is over, clean up */
				ಬಿಡುಗಡೆ(for_list);
				for_list = ಖಾಲಿ;
				for_lcur = ಖಾಲಿ;
				ನಿಲ್ಲಿಸು;
			}
			/* Insert next value from for_lcur */
			/* note: *for_lcur already has quotes removed, $var expanded, etc */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
			ಗಣ_local_var_from_halves(pi->cmds[0].argv[0], *for_lcur++);
#ಇಲ್ಲದಿದ್ದರೆ /* __U_BOOT__ */
			/* We cannot use xasprintf, so we emulate it. */
			ಅಕ್ಷರ *ತುಂಬಿದ_var;
			ಅಕ್ಷರ *var = pi->cmds[0].argv[0];
			ಅಕ್ಷರ *val = *for_lcur++;

			/* + 1 to take into account =. */
			ತುಂಬಿದ_var = xmalloc(ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(var) + ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(val) + 1);
			sprintf(ತುಂಬಿದ_var, "%s=%s", var, val);

			ಗಣ_local_var_modern(ತುಂಬಿದ_var, /*flag:*/ 0);
#ಆದರೆ_ಅಂತ್ಯ /* __U_BOOT__ */
			ಮುಂದುವರಿ;
		}
		ಆದರೆ (rword == RES_ಒಳ) {
			ಮುಂದುವರಿ; /* "for v IN list;..." - "in" has no cmds anyway */
		}
		ಆದರೆ (rword == RES_ಮುಗಿದ) {
			ಮುಂದುವರಿ; /* "done" has no cmds too */
		}
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_HUSH_CASE
		ಆದರೆ (rword == RES_CASE) {
			debug_printf_exec("CASE cond_code:%d\n", cond_ಸಂಕೇತ);
			case_ಶಬ್ದ = expand_string_ಗಮ್ಯ_string(pi->cmds->argv[0],
				EXP_FLAG_ESC_GLOB_CHARS, /*unbackslash:*/ 1);
			debug_printf_exec("CASE word1:'%s'\n", case_ಶಬ್ದ);
			//unbackslash(case_word);
			//debug_printf_exec("CASE word2:'%s'\n", case_word);
			ಮುಂದುವರಿ;
		}
		ಆದರೆ (rword == RES_ಹೊಂದಿಕೆ) {
			ಅಕ್ಷರ **argv;

			debug_printf_exec("MATCH cond_code:%d\n", cond_ಸಂಕೇತ);
			ಆದರೆ (!case_ಶಬ್ದ) /* "case ... matched_word) ... WORD)": we executed selected branch, stop */
				ನಿಲ್ಲಿಸು;
			/* all prev words didn't match, does this one match? */
			argv = pi->cmds->argv;
			ಆಗ (*argv) {
				ಅಕ್ಷರ *ನಮೂನೆ;
				debug_printf_exec("expand_string_to_string('%s')\n", *argv);
				ನಮೂನೆ = expand_string_ಗಮ್ಯ_string(*argv,
						EXP_FLAG_ESC_GLOB_CHARS,
						/*unbackslash:*/ 0
				);
				/* TODO: which FNM_xxx flags to use? */
				cond_ಸಂಕೇತ = (fnmatch(ನಮೂನೆ, case_ಶಬ್ದ, /*flags:*/ 0) != 0);
				debug_printf_exec("cond_code=fnmatch(pattern:'%s',str:'%s'):%d\n",
						ನಮೂನೆ, case_ಶಬ್ದ, cond_ಸಂಕೇತ);
				ಬಿಡುಗಡೆ(ನಮೂನೆ);
				ಆದರೆ (cond_ಸಂಕೇತ == 0) {
					/* match! we will execute this branch */
					ಬಿಡುಗಡೆ(case_ಶಬ್ದ);
					case_ಶಬ್ದ = ಖಾಲಿ; /* make future "word)" stop */
					ನಿಲ್ಲಿಸು;
				}
				argv++;
			}
			ಮುಂದುವರಿ;
		}
		ಆದರೆ (rword == RES_CASE_BODY) { /* inside of a case branch */
			debug_printf_exec("CASE_BODY cond_code:%d\n", cond_ಸಂಕೇತ);
			ಆದರೆ (cond_ಸಂಕೇತ != 0)
				ಮುಂದುವರಿ; /* not matched yet, skip this pipe */
		}
		ಆದರೆ (rword == RES_ESAC) {
			debug_printf_exec("ESAC cond_code:%d\n", cond_ಸಂಕೇತ);
			ಆದರೆ (case_ಶಬ್ದ) {
				/* "case" did not match anything: still set $? (to 0) */
				G.ಕೊನೆಯ_exitcode = rcode = EXIT_SUCCESS;
			}
		}
#ಆದರೆ_ಅಂತ್ಯ
		/* Just pressing <enter> in shell should check for jobs.
		 * OTOH, in non-interactive shell this is useless
		 * and only leads to extra job checks */
		ಆದರೆ (pi->ಅಂಕೆ_cmds == 0) {
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
			ಆದರೆ (G_interactive_fd)
				ಹೋಗು check_jobs_and_continue;
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */
			ಮುಂದುವರಿ;
		}

		/* After analyzing all keywords and conditions, we decided
		 * to execute this pipe. NB: have to do checkjobs(NULL)
		 * after run_pipe to collect any background children,
		 * even if list execution is to be stopped. */
		debug_printf_exec(": run_pipe with %d members\n", pi->ಅಂಕೆ_cmds);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
#ಆದರೆ ENABLE_HUSH_LOOPS
		G.flag_break_continue = 0;
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
		rcode = r = G.o_opt[OPT_O_NOEXEC] ? 0 : ಸತತ_pipe(pi);
		/* NB: rcode is a smalluint, r is int */
#ಇಲ್ಲದಿದ್ದರೆ /* __U_BOOT__ */
		rcode = r = ಸತತ_pipe(pi); /* NB: rcode is a smalluint, r is int */
		ಆದರೆ (r <= EXIT_RET_ಸಂಕೇತ) {
			ಪೂರ್ಣಾಂಕ ಹಿಂದಿನ_rcode = G.ಕೊನೆಯ_exitcode;
			/*
			 * This magic is to get the exit code given by the user.
			 * Contrary to old shell code, we use + EXIT_RET_CODE as EXIT_RET_CODE
			 * equals -2.
			 */
			G.ಕೊನೆಯ_exitcode = -r + EXIT_RET_ಸಂಕೇತ;

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
			ಆದರೆ (ಹಿಂದಿನ_rcode != r) {
				/*
				 * If run from run_command, run_command_flags will be set, so we check
				 * this to know if we are in main input shell.
				 */
				ಆದರೆ (!G.ಸತತ_command_flags)
					ಬರೆ("exit not allowed from main input shell.\n");

				ನಿಲ್ಲಿಸು;
			}
			ಮುಂದುವರಿ;
		}
#ಆದರೆ_ಅಂತ್ಯ /* __U_BOOT__ */
		ಆದರೆ (r != -1) {
			/* We ran a builtin, function, or group.
			 * rcode is already known
			 * and we don't need to wait for anything. */
			debug_printf_exec(": builtin/func exitcode %d\n", rcode);
			G.ಕೊನೆಯ_exitcode = rcode;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
			check_and_ಸತತ_traps();
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */
#ಆದರೆ ENABLE_HUSH_TRAP && ENABLE_HUSH_FUNCTIONS
			rcode = G.ಕೊನೆಯ_exitcode; /* "return" in trap can change it, read back */
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
#ಆದರೆ ENABLE_HUSH_LOOPS
			/* Was it "break" or "continue"? */
			ಆದರೆ (G.flag_break_continue) {
				smallint fbc = G.flag_break_continue;
				/* We might fall into outer *loop*,
				 * don't want to break it too */
				ಆದರೆ (loop_ತುದಿ) {
					G.ಆಳ_break_continue--;
					ಆದರೆ (G.ಆಳ_break_continue == 0)
						G.flag_break_continue = 0;
					/* else: e.g. "continue 2" should *break* once, *then* continue */
				} /* else: "while... do... { we are here (innermost list is not a loop!) };...done" */
				ಆದರೆ (G.ಆಳ_break_continue != 0 || fbc == BC_BREAK) {
					checkjobs(ಖಾಲಿ, 0 /*(no pid to wait for)*/);
					ನಿಲ್ಲಿಸು;
				}
				/* "continue": simulate end of loop */
				rword = RES_ಮುಗಿದ;
				ಮುಂದುವರಿ;
			}
#ಆದರೆ_ಅಂತ್ಯ
			ಆದರೆ (G_flag_return_ಒಳ_ಪ್ರಗತಿ == 1) {
				checkjobs(ಖಾಲಿ, 0 /*(no pid to wait for)*/);
				ನಿಲ್ಲಿಸು;
			}

		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (pi->followup == PIPE_BG) {
			/* What does bash do with attempts to background builtins? */
			/* even bash 3.2 doesn't do that well with nested bg:
			 * try "{ { sleep 10; echo DEEP; } & echo HERE; } &".
			 * I'm NOT treating inner &'s as jobs */
#ಆದರೆ ENABLE_HUSH_ಕೆಲಸ
			ಆದರೆ (G.ಸತತ_list_level == 1)
				ಸೇರಿಸು_ಕೆಲಸ_into_ಕೋಷ್ಟಕ(pi);
#ಆದರೆ_ಅಂತ್ಯ
			/* Last command's pid goes to $! */
			G.ಕೊನೆಯ_bg_pid = pi->cmds[pi->ಅಂಕೆ_cmds - 1].pid;
			G.ಕೊನೆಯ_bg_pid_exitcode = 0;
			debug_printf_exec(": cmd&: exitcode EXIT_SUCCESS\n");
/* Check pi->pi_inverted? "! sleep 1 & echo $?": bash says 1. dash and ash say 0 */
			rcode = EXIT_SUCCESS;
			ಹೋಗು check_traps;
		} ಇಲ್ಲದಿದ್ದರೆ {
#ಆದರೆ ENABLE_HUSH_ಕೆಲಸ
			ಆದರೆ (G.ಸತತ_list_level == 1 && G_interactive_fd) {
				/* Waits for completion, then fg's main shell */
				rcode = checkjobs_and_fg_shell(pi);
				debug_printf_exec(": checkjobs_and_fg_shell exitcode %d\n", rcode);
				ಹೋಗು check_traps;
			}
#ಆದರೆ_ಅಂತ್ಯ
			/* This one just waits for completion */
			rcode = checkjobs(pi, 0 /*(no pid to wait for)*/);
			debug_printf_exec(": checkjobs exitcode %d\n", rcode);
 check_traps:
			G.ಕೊನೆಯ_exitcode = rcode;
			check_and_ಸತತ_traps();
#ಆದರೆ ENABLE_HUSH_TRAP && ENABLE_HUSH_FUNCTIONS
			rcode = G.ಕೊನೆಯ_exitcode; /* "return" in trap can change it, read back */
#ಆದರೆ_ಅಂತ್ಯ
		}
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
		/* Handle "set -e" */
		ಆದರೆ (rcode != 0 && G.o_opt[OPT_O_ERREXIT]) {
			debug_printf_exec("ERREXIT:1 errexit_depth:%d\n", G.errexit_ಆಳ);
			ಆದರೆ (G.errexit_ಆಳ == 0)
				hush_exit(rcode);
		}
#ಇಲ್ಲದಿದ್ದರೆ /* __U_BOOT__ */
		} /* if (r != -1) */
#ಆದರೆ_ಅಂತ್ಯ /* __U_BOOT__ */
		G.errexit_ಆಳ = sv_errexit_ಆಳ;

		/* Analyze how result affects subsequent commands */
#ಆದರೆ ENABLE_HUSH_IF
		ಆದರೆ (rword == RES_IF || rword == RES_ELIF) {
			debug_printf_exec("cond_code=rcode:%d\n", rcode);
			cond_ಸಂಕೇತ = rcode;
		}
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
 check_jobs_and_continue:
		checkjobs(ಖಾಲಿ, 0 /*(no pid to wait for)*/);
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */
 dont_check_jobs_but_continue: ;
#ಆದರೆ ENABLE_HUSH_LOOPS
		/* Beware of "while false; true; do ..."! */
		ಆದರೆ (pi->ಮುಂದಿನ
		 && (pi->ಮುಂದಿನ->res_ಶಬ್ದ == RES_DO || pi->ಮುಂದಿನ->res_ಶಬ್ದ == RES_ಮುಗಿದ)
		 /* check for RES_DONE is needed for "while ...; do \n done" case */
		) {
			ಆದರೆ (rword == RES_WHILE) {
				ಆದರೆ (rcode) {
					/* "while false; do...done" - exitcode 0 */
					G.ಕೊನೆಯ_exitcode = rcode = EXIT_SUCCESS;
					debug_printf_exec(": while expr is false: breaking (exitcode:EXIT_SUCCESS)\n");
					ನಿಲ್ಲಿಸು;
				}
			}
			ಆದರೆ (rword == RES_UNTIL) {
				ಆದರೆ (!rcode) {
					debug_printf_exec(": until expr is true: breaking\n");
					ನಿಲ್ಲಿಸು;
				}
			}
		}
#ಆದರೆ_ಅಂತ್ಯ
	} /* for (pi) */

#ಆದರೆ ENABLE_HUSH_ಕೆಲಸ
	G.ಸತತ_list_level--;
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_HUSH_LOOPS
	ಆದರೆ (loop_ತುದಿ)
		G.ಆಳ_ದ_loop--;
	ಬಿಡುಗಡೆ(for_list);
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_HUSH_CASE
	ಬಿಡುಗಡೆ(case_ಶಬ್ದ);
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	debug_leave();
	debug_printf_exec("run_list lvl %d return %d\n", G.ಸತತ_list_level, rcode);
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */
	ಹಿಂತಿರುಗಿ rcode;
}

/* Select which version we will use */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಸತತ_and_free_list(ರಚನೆ pipe *pi)
{
	ಪೂರ್ಣಾಂಕ rcode = 0;
	debug_printf_exec("run_and_free_list entered\n");
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	ಆದರೆ (!G.o_opt[OPT_O_NOEXEC]) {
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */
		debug_printf_exec(": run_list: 1st pipe with %d cmds\n", pi->ಅಂಕೆ_cmds);
		rcode = ಸತತ_list(pi);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	}
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */
	/* free_pipe_list has the side effect of clearing memory.
	 * In the long run that function can be merged with run_list,
	 * but doing that now would hobble the debugging effort. */
	free_pipe_list(pi);
	debug_printf_exec("run_and_free_list return %d\n", rcode);
	ಹಿಂತಿರುಗಿ rcode;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
ಸ್ಥಿರ ಶೂನ್ಯ install_sighandlers(ಚಿಹ್ನೆರಹಿತ ಮುಖವಾಡ)
{
	sighandler_t old_handler;
	ಚಿಹ್ನೆರಹಿತ sig = 0;
	ಆಗ ((ಮುಖವಾಡ >>= 1) != 0) {
		sig++;
		ಆದರೆ (!(ಮುಖವಾಡ & 1))
			ಮುಂದುವರಿ;
		old_handler = install_sighandler(sig, pick_sighandler(sig));
		/* POSIX allows shell to re-enable SIGCHLD
		 * even if it was SIG_IGN on entry.
		 * Therefore we skip IGN check for it:
		 */
		ಆದರೆ (sig == SIGCHLD)
			ಮುಂದುವರಿ;
		/* Interactive bash re-enables SIGHUP which is SIG_IGNed on entry.
		 * Try:
		 * trap '' hup; bash; echo RET  # type "kill -hup $$", see SIGHUP having effect
		 * trap '' hup; bash -c 'kill -hup $$; echo ALIVE'  # here SIGHUP is SIG_IGNed
		 */
		ಆದರೆ (sig == SIGHUP && G_interactive_fd)
			ಮುಂದುವರಿ;
		/* Unless one of the above signals, is it SIG_IGN? */
		ಆದರೆ (old_handler == SIG_IGN) {
			/* oops... restore back to IGN, and record this fact */
			install_sighandler(sig, old_handler);
#ಆದರೆ ENABLE_HUSH_TRAP
			ಆದರೆ (!G_traps)
				G_traps = xzalloc(ಗಾತ್ರ(G_traps[0]) * NSIG);
			ಬಿಡುಗಡೆ(G_traps[sig]);
			G_traps[sig] = xzalloc(1); /* == xstrdup(""); */
#ಆದರೆ_ಅಂತ್ಯ
		}
	}
}

/* Called a few times only (or even once if "sh -c") */
ಸ್ಥಿರ ಶೂನ್ಯ install_special_sighandlers(ಶೂನ್ಯ)
{
	ಚಿಹ್ನೆರಹಿತ ಮುಖವಾಡ;

	/* Which signals are shell-special? */
	ಮುಖವಾಡ = (1 << SIGQUIT) | (1 << SIGCHLD);
	ಆದರೆ (G_interactive_fd) {
		ಮುಖವಾಡ |= SPECIAL_INTERACTIVE_SIGS;
		ಆದರೆ (G_saved_tty_pgrp) /* we have ctty, job control sigs work */
			ಮುಖವಾಡ |= SPECIAL_JOBSTOP_SIGS;
	}
	/* Careful, do not re-install handlers we already installed */
	ಆದರೆ (G.special_sig_ಮುಖವಾಡ != ಮುಖವಾಡ) {
		ಚಿಹ್ನೆರಹಿತ diff = ಮುಖವಾಡ & ~G.special_sig_ಮುಖವಾಡ;
		G.special_sig_ಮುಖವಾಡ = ಮುಖವಾಡ;
		install_sighandlers(diff);
	}
}

#ಆದರೆ ENABLE_HUSH_ಕೆಲಸ
/* helper */
/* Set handlers to restore tty pgrp and exit */
ಸ್ಥಿರ ಶೂನ್ಯ install_fatal_sighandlers(ಶೂನ್ಯ)
{
	ಚಿಹ್ನೆರಹಿತ ಮುಖವಾಡ;

	/* We will restore tty pgrp on these signals */
	ಮುಖವಾಡ = 0
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
	G_fatal_sig_ಮುಖವಾಡ = ಮುಖವಾಡ;

	install_sighandlers(ಮುಖವಾಡ);
}
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಗಣ_ಬಹುಲಕ(ಪೂರ್ಣಾಂಕ state, ಅಕ್ಷರ ಬಹುಲಕ, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *o_opt)
{
	ಪೂರ್ಣಾಂಕ idx;
	ಆಯ್ಕೆ (ಬಹುಲಕ) {
	ಸ್ಥಿತಿ 'n':
		/* set -n has no effect in interactive shell */
		/* Try: while set -n; do echo $-; done */
		ಆದರೆ (!G_interactive_fd)
			G.o_opt[OPT_O_NOEXEC] = state;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ 'x':
		IF_HUSH_ಬಹುಲಕ_X(G_x_ಬಹುಲಕ = state;)
		IF_HUSH_ಬಹುಲಕ_X(ಆದರೆ (G.x_ಬಹುಲಕ_fd <= 0) G.x_ಬಹುಲಕ_fd = dup_CLOEXEC(2, 10);)
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ 'e':
		G.o_opt[OPT_O_ERREXIT] = state;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ 'o':
		ಆದರೆ (!o_opt) {
			/* "set -o" or "set +o" without parameter.
			 * in bash, set -o produces this output:
			 *  pipefail        off
			 * and set +o:
			 *  set +o pipefail
			 * We always use the second form.
			 */
			ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p = o_opt_ಅಕ್ಷರಸರಣಿಗಳು;
			idx = 0;
			ಆಗ (*p) {
				ಬರೆ("set %co %s\n", (G.o_opt[idx] ? '-' : '+'), p);
				idx++;
				p += ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(p) + 1;
			}
			ನಿಲ್ಲಿಸು;
		}
		idx = ಸೂಚ್ಯಂಕ_ಒಳ_ಅಕ್ಷರಸರಣಿಗಳು(o_opt_ಅಕ್ಷರಸರಣಿಗಳು, o_opt);
		ಆದರೆ (idx >= 0) {
			G.o_opt[idx] = state;
			ನಿಲ್ಲಿಸು;
		}
		/* fall through to error */
	ಪೂರ್ವನಿಯೋಜಿತ:
		ಹಿಂತಿರುಗಿ EXIT_FAILURE;
	}
	ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
}

ಪೂರ್ಣಾಂಕ hush_main(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv) MAIN_EXTERNALLY_VISIBLE;
ಪೂರ್ಣಾಂಕ hush_main(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv)
{
	pid_t cached_getpid;
	ಎಣಿಕೆ {
		OPT_login = (1 << 0),
	};
	ಚಿಹ್ನೆರಹಿತ flags;
#ಆದರೆ !BB_MMU
	ಚಿಹ್ನೆರಹಿತ builtin_argc = 0;
#ಆದರೆ_ಅಂತ್ಯ
	ಅಕ್ಷರ **e;
	ರಚನೆ variable *cur_var;
	ರಚನೆ variable *shell_ver;

	INIT_G();
	ಆದರೆ (EXIT_SUCCESS != 0) /* if EXIT_SUCCESS == 0, it is already done */
		G.ಕೊನೆಯ_exitcode = EXIT_SUCCESS;
#ಆದರೆ !BB_MMU
	/* "Big heredoc" support via "sh -< STRING" invocation.
	 * Check it first (do not bother to run the usual init code,
	 * it is not needed for this case).
	 */
	ಆದರೆ (argv[1]
	 && argv[1][0] == '-' && argv[1][1] == '<' /*&& !argv[1][2]*/
	 /*&& argv[2] && !argv[3] - we don't check some conditions */
	) {
		ತುಂಬಿದ_write1_str(argv[2]);
		_exit(0);
	}
	G.argv0_for_re_execing = argv[0];
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_HUSH_TRAP
# ಆದರೆ ENABLE_HUSH_FUNCTIONS
	G.return_exitcode = -1;
# endif
	G.pre_trap_exitcode = -1;
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ ENABLE_HUSH_FAST
	G.ಎಣಿಕೆ_SIGCHLD++; /* ensure it is != G.handled_SIGCHLD */
#ಆದರೆ_ಅಂತ್ಯ

	cached_getpid = getpid();   /* for tcsetpgrp() during init */
	G.ಬೇರು_pid = cached_getpid; /* for $PID  (NOMMU can override via -$HEXPID:HEXPPID:...) */
	G.ಬೇರು_ppid = getppid();    /* for $PPID (NOMMU can override) */

	/* Deal with HUSH_VERSION */
	debug_printf_env("unsetenv '%s'\n", "HUSH_VERSION");
	unsetenv("HUSH_VERSION"); /* in case it exists in initial env */
	shell_ver = xzalloc(ಗಾತ್ರ(*shell_ver));
	shell_ver->flg_export = 1;
	shell_ver->flg_read_only = 1;
	/* Code which handles ${var<op>...} needs writable values for all variables,
	 * therefore we xstrdup: */
	shell_ver->varstr = xstrdup(hush_ಆವೃತ್ತಿ_str);

	/* Create shell local variables from the values
	 * currently living in the environment */
	G.ತುದಿ_var = shell_ver;
	cur_var = G.ತುದಿ_var;
	e = environ;
	ಆದರೆ (e) ಆಗ (*e) {
		ಅಕ್ಷರ *ಮೌಲ್ಯ = strchr(*e, '=');
		ಆದರೆ (ಮೌಲ್ಯ) { /* paranoia */
			cur_var->ಮುಂದಿನ = xzalloc(ಗಾತ್ರ(*cur_var));
			cur_var = cur_var->ಮುಂದಿನ;
			cur_var->varstr = *e;
			cur_var->max_len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(*e);
			cur_var->flg_export = 1;
		}
		e++;
	}
	/* (Re)insert HUSH_VERSION into env (AFTER we scanned the env!) */
	debug_printf_env("putenv '%s'\n", shell_ver->varstr);
	putenv(shell_ver->varstr);

	/* Export PWD */
	ಗಣ_pwd_var(SETFLAG_EXPORT);

#ಆದರೆ BASH_HOSTNAME_VAR
	/* Set (but not export) HOSTNAME unless already set */
	ಆದರೆ (!get_local_var_ಮೌಲ್ಯ("HOSTNAME")) {
		ರಚನೆ utsname uts;
		uname(&uts);
		ಗಣ_local_var_from_halves("HOSTNAME", uts.nodename);
	}
#ಆದರೆ_ಅಂತ್ಯ
	/* IFS is not inherited from the parent environment */
	ಗಣ_local_var_from_halves("IFS", defifs);

	ಆದರೆ (!get_local_var_ಮೌಲ್ಯ("PATH"))
		ಗಣ_local_var_from_halves("PATH", bb_default_ಬೇರು_ಮಾರ್ಗ);

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

#ಆದರೆ ಅಂಕೆ_SCRIPTS > 0
	ಆದರೆ (argc < 0) {
		ಅಕ್ಷರ *script = get_script_content(-argc - 1);
		G.global_argv = argv;
		G.global_argc = string_array_len(argv);
		//install_special_sighandlers(); - needed?
		parse_and_ಸತತ_string(script);
		ಹೋಗು final_return;
	}
#ಆದರೆ_ಅಂತ್ಯ

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
	ಆಗ (1) {
		ಪೂರ್ಣಾಂಕ opt = getopt(argc, argv, "+" /* stop at 1st non-option */
				"cexinsl"
#ಆದರೆ !BB_MMU
				"$:R:V:"
# ಆದರೆ ENABLE_HUSH_LINENO_VAR
				"L:"
# endif
# ಆದರೆ ENABLE_HUSH_FUNCTIONS
				"F:"
# endif
#ಆದರೆ_ಅಂತ್ಯ
		);
		ಆದರೆ (opt <= 0)
			ನಿಲ್ಲಿಸು;
		ಆಯ್ಕೆ (opt) {
		ಸ್ಥಿತಿ 'c':
			/* Note: -c is not an option with param!
			 * "hush -c -l SCRIPT" is valid. "hush -cSCRIPT" is not.
			 */
			G.opt_c = 1;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'i':
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
//ಇಲ್ಲಿ
//(under "bash", it's the opposite: it runs $BASH_ENV file only *without* -i).
//
//ash -i -c 'sleep 3; sleep 3', on ^C, drops into a prompt instead of exiting
//(this may be a bug, bash does not do this).
//(ash -i -c 'sleep 3' won't show this, the last command gets auto-"exec"ed)
//
//None of the above feel like useful features people would rely on.
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 's':
			G.opt_s = 1;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ 'l':
			flags |= OPT_login;
			ನಿಲ್ಲಿಸು;
#ಆದರೆ !BB_MMU
		ಸ್ಥಿತಿ '$': {
			ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ದೀರ್ಘ ಖಾಲಿಗಳು_trap_ಮುಖವಾಡ;

			G.ಬೇರು_pid = bb_strtou(optarg, &optarg, 16);
			optarg++;
			G.ಬೇರು_ppid = bb_strtou(optarg, &optarg, 16);
			optarg++;
			G.ಕೊನೆಯ_bg_pid = bb_strtou(optarg, &optarg, 16);
			optarg++;
			G.ಕೊನೆಯ_exitcode = bb_strtou(optarg, &optarg, 16);
			optarg++;
			builtin_argc = bb_strtou(optarg, &optarg, 16);
			optarg++;
			ಖಾಲಿಗಳು_trap_ಮುಖವಾಡ = bb_strtoull(optarg, &optarg, 16);
			ಆದರೆ (ಖಾಲಿಗಳು_trap_ಮುಖವಾಡ != 0) {
				IF_HUSH_TRAP(ಪೂರ್ಣಾಂಕ sig;)
				install_special_sighandlers();
# ಆದರೆ ENABLE_HUSH_TRAP
				G_traps = xzalloc(ಗಾತ್ರ(G_traps[0]) * NSIG);
				ಗಾಗಿ (sig = 1; sig < NSIG; sig++) {
					ಆದರೆ (ಖಾಲಿಗಳು_trap_ಮುಖವಾಡ & (1LL << sig)) {
						G_traps[sig] = xzalloc(1); /* == xstrdup(""); */
						install_sighandler(sig, SIG_IGN);
					}
				}
# endif
			}
# ಆದರೆ ENABLE_HUSH_LOOPS
			optarg++;
			G.ಆಳ_ದ_loop = bb_strtou(optarg, &optarg, 16);
# endif
			/* Suppress "killed by signal" message, -$ hack is used
			 * for subshells: echo `sh -c 'kill -9 $$'`
			 * should be silent.
			 */
			IF_HUSH_ಕೆಲಸ(G.ಸತತ_list_level = 1;)
# ಆದರೆ ENABLE_HUSH_FUNCTIONS
			/* nommu uses re-exec trick for "... | func | ...",
			 * should allow "return".
			 * This accidentally allows returns in subshells.
			 */
			G_flag_return_ಒಳ_ಪ್ರಗತಿ = -1;
# endif
			ನಿಲ್ಲಿಸು;
		}
		ಸ್ಥಿತಿ 'R':
		ಸ್ಥಿತಿ 'V':
			ಗಣ_local_var(xstrdup(optarg), opt == 'R' ? SETFLAG_MAKE_RO : 0);
			ನಿಲ್ಲಿಸು;
# ಆದರೆ ENABLE_HUSH_LINENO_VAR
		ಸ್ಥಿತಿ 'L':
			G.parse_lineno = xatou(optarg);
			ನಿಲ್ಲಿಸು;
# endif
# ಆದರೆ ENABLE_HUSH_FUNCTIONS
		ಸ್ಥಿತಿ 'F': {
			ರಚನೆ function *funcp = new_function(optarg);
			/* funcp->name is already set to optarg */
			/* funcp->body is set to NULL. It's a special case. */
			funcp->body_as_string = argv[optind];
			optind++;
			ನಿಲ್ಲಿಸು;
		}
# endif
#ಆದರೆ_ಅಂತ್ಯ
		/*case '?': invalid option encountered (set_mode('?') will fail) */
		/*case 'n':*/
		/*case 'x':*/
		/*case 'e':*/
		ಪೂರ್ವನಿಯೋಜಿತ:
			ಆದರೆ (ಗಣ_ಬಹುಲಕ(1, opt, ಖಾಲಿ) == 0) /* no error */
				ನಿಲ್ಲಿಸು;
			bb_show_usage();
		}
	} /* option parsing loop */

	/* Skip options. Try "hush -l": $1 should not be "-l"! */
	G.global_argc = argc - (optind - 1);
	G.global_argv = argv + (optind - 1);
	G.global_argv[0] = argv[0];

	/* If we are login shell... */
	ಆದರೆ (flags & OPT_login) {
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *hp = ಖಾಲಿ;
		HFILE *ಒಳಹರಿವು;

		debug_printf("sourcing /etc/profile\n");
		ಒಳಹರಿವು = hfopen("/etc/profile");
 ಸತತ_profile:
		ಆದರೆ (ಒಳಹರಿವು != ಖಾಲಿ) {
			install_special_sighandlers();
			parse_and_ಸತತ_file(ಒಳಹರಿವು);
			hfclose(ಒಳಹರಿವು);
		}
		/* bash: after sourcing /etc/profile,
		 * tries to source (in the given order):
		 * ~/.bash_profile, ~/.bash_login, ~/.profile,
		 * stopping on first found. --noprofile turns this off.
		 * bash also sources ~/.bash_logout on exit.
		 * If called as sh, skips .bash_XXX files.
		 */
		ಆದರೆ (!hp) { /* unless we looped on the "goto" already */
			hp = get_local_var_ಮೌಲ್ಯ("HOME");
			ಆದರೆ (hp && hp[0]) {
				debug_printf("sourcing ~/.profile\n");
				hp = concat_ಮಾರ್ಗ_file(hp, ".profile");
				ಒಳಹರಿವು = hfopen(hp);
				ಬಿಡುಗಡೆ((ಅಕ್ಷರ*)hp);
				ಹೋಗು ಸತತ_profile;
			}
		}
	}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	/* -c takes effect *after* -l */
	ಆದರೆ (G.opt_c) {
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
		ಅಕ್ಷರ *script;

		install_special_sighandlers();

		G.global_argc--;
		G.global_argv++;
#ಆದರೆ !BB_MMU
		ಆದರೆ (builtin_argc) {
			/* -c 'builtin' [BARGV...] "" ARG0 [ARG1...] */
			ಸ್ಥಿರಾಂಕ ರಚನೆ built_ಒಳ_command *x;
			x = ಕಂಡುಹಿಡಿ_builtin(G.global_argv[0]);
			ಆದರೆ (x) { /* paranoia */
				argv = G.global_argv;
				G.global_argc -= builtin_argc + 1; /* skip [BARGV...] "" */
				G.global_argv += builtin_argc + 1;
				G.global_argv[-1] = ಖಾಲಿ; /* replace "" */
				G.ಕೊನೆಯ_exitcode = x->b_function(argv);
			}
			ಹೋಗು final_return;
		}
#ಆದರೆ_ಅಂತ್ಯ

		script = G.global_argv[0];
		ಆದರೆ (!script)
			bb_error_ಸಂದೇಶ_and_die(bb_ಸಂದೇಶ_requires_arg, "-c");
		ಆದರೆ (!G.global_argv[1]) {
			/* -c 'script' (no params): prevent empty $0 */
			G.global_argv[0] = argv[0];
		} ಇಲ್ಲದಿದ್ದರೆ { /* else -c 'script' ARG0 [ARG1...]: $0 is ARG0 */
			G.global_argc--;
			G.global_argv++;
		}
		parse_and_ಸತತ_string(script);
		ಹೋಗು final_return;
	}

	/* -s is: hush -s ARGV1 ARGV2 (no SCRIPT) */
	ಆದರೆ (!G.opt_s && G.global_argv[1]) {
		HFILE *ಒಳಹರಿವು;
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
		ಒಳಹರಿವು = hfopen(G.global_argv[0]);
		ಆದರೆ (!ಒಳಹರಿವು) {
			bb_simple_perror_ಸಂದೇಶ_and_die(G.global_argv[0]);
		}
		xfunc_error_retval = 1;
		install_special_sighandlers();
		parse_and_ಸತತ_file(ಒಳಹರಿವು);
#ಆದರೆ ENABLE_FEATURE_CLEAN_ಮೇಲೆ
		hfclose(ಒಳಹರಿವು);
#ಆದರೆ_ಅಂತ್ಯ
		ಹೋಗು final_return;
	}
	/* "implicit" -s: bare interactive hush shows 's' in $- */
	G.opt_s = 1;

#ಆದರೆ_ಅಂತ್ಯ /* __U_BOOT__ */
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
#ಆದರೆ ENABLE_HUSH_ಕೆಲಸ
	ಆದರೆ (isatty(STDIN_FILENO) && isatty(STDOUT_FILENO)) {
		G_saved_tty_pgrp = tcgetpgrp(STDIN_FILENO);
		debug_printf("saved_tty_pgrp:%d\n", G_saved_tty_pgrp);
		ಆದರೆ (G_saved_tty_pgrp < 0)
			G_saved_tty_pgrp = 0;

		/* try to dup stdin to high fd#, >= 255 */
		G_interactive_fd = dup_CLOEXEC(STDIN_FILENO, 254);
		ಆದರೆ (G_interactive_fd < 0) {
			/* try to dup to any fd */
			G_interactive_fd = dup_CLOEXEC(STDIN_FILENO, -1);
			ಆದರೆ (G_interactive_fd < 0) {
				/* give up */
				G_interactive_fd = 0;
				G_saved_tty_pgrp = 0;
			}
		}
	}
	debug_printf("interactive_fd:%d\n", G_interactive_fd);
	ಆದರೆ (G_interactive_fd) {
		ಆದರೆ (G_saved_tty_pgrp) {
			/* If we were run as 'hush &', sleep until we are
			 * in the foreground (tty pgrp == our pgrp).
			 * If we get started under a job aware app (like bash),
			 * make sure we are now in charge so we don't fight over
			 * who gets the foreground */
			ಆಗ (1) {
				pid_t shell_pgrp = getpgrp();
				G_saved_tty_pgrp = tcgetpgrp(G_interactive_fd);
				ಆದರೆ (G_saved_tty_pgrp == shell_pgrp)
					ನಿಲ್ಲಿಸು;
				/* send TTIN to ourself (should stop us) */
				kill(- shell_pgrp, SIGTTIN);
			}
		}

		/* Install more signal handlers */
		install_special_sighandlers();

		ಆದರೆ (G_saved_tty_pgrp) {
			/* Set other signals to restore saved_tty_pgrp */
			install_fatal_sighandlers();
			/* Put ourselves in our own process group
			 * (bash, too, does this only if ctty is available) */
			bb_setpgrp(); /* is the same as setpgid(our_pid, our_pid); */
			/* Grab control of the terminal */
			tcsetpgrp(G_interactive_fd, cached_getpid);
		}
		enable_restore_tty_pgrp_on_exit();

# ಆದರೆ ENABLE_FEATURE_EDITING
		G.ಸಾಲು_ಒಳಹರಿವು_state = new_ಸಾಲು_ಒಳಹರಿವು_t(FOR_SHELL);
#  ಆದರೆ ENABLE_FEATURE_TAB_COMPLETION
		G.ಸಾಲು_ಒಳಹರಿವು_state->get_exe_name = hush_command_name;
#  endif
#  ಆದರೆ EDITING_HAS_sh_get_var
		G.ಸಾಲು_ಒಳಹರಿವು_state->sh_get_var = get_local_var_ಮೌಲ್ಯ;
#  endif
# endif
# ಆದರೆ ENABLE_HUSH_SAVEHISTORY && MAX_HISTORY > 0
		{
			ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *hp = get_local_var_ಮೌಲ್ಯ("HISTFILE");
			ಆದರೆ (!hp) {
				hp = get_local_var_ಮೌಲ್ಯ("HOME");
				ಆದರೆ (hp)
					hp = concat_ಮಾರ್ಗ_file(hp, ".hush_history");
			} ಇಲ್ಲದಿದ್ದರೆ {
				hp = xstrdup(hp);
			}
			ಆದರೆ (hp) {
				G.ಸಾಲು_ಒಳಹರಿವು_state->hist_file = hp;
				//set_local_var(xasprintf("HISTFILE=%s", ...));
			}
#  ಆದರೆ ENABLE_FEATURE_SH_HISTFILESIZE
			hp = get_local_var_ಮೌಲ್ಯ("HISTFILESIZE");
			G.ಸಾಲು_ಒಳಹರಿವು_state->max_history = ಗಾತ್ರ_from_HISTFILESIZE(hp);
#  endif
		}
# endif
	} ಇಲ್ಲದಿದ್ದರೆ {
		install_special_sighandlers();
	}
#ಅಥವಾಆದರೆ ENABLE_HUSH_INTERACTIVE
	/* No job control compiled in, only prompt/line editing */
	ಆದರೆ (isatty(STDIN_FILENO) && isatty(STDOUT_FILENO)) {
		G_interactive_fd = dup_CLOEXEC(STDIN_FILENO, 254);
		ಆದರೆ (G_interactive_fd < 0) {
			/* try to dup to any fd */
			G_interactive_fd = dup_CLOEXEC(STDIN_FILENO, -1);
			ಆದರೆ (G_interactive_fd < 0)
				/* give up */
				G_interactive_fd = 0;
		}
	}
	install_special_sighandlers();
#ಇಲ್ಲದಿದ್ದರೆ
	/* We have interactiveness code disabled */
	install_special_sighandlers();
#ಆದರೆ_ಅಂತ್ಯ
	/* bash:
	 * if interactive but not a login shell, sources ~/.bashrc
	 * (--norc turns this off, --rcfile <file> overrides)
	 */

	ಆದರೆ (G_interactive_fd) {
#ಆದರೆ ENABLE_HUSH_INTERACTIVE && ENABLE_FEATURE_EDITING_FANCY_PROMPT
		/* Set (but not export) PS1/2 unless already set */
		ಆದರೆ (!get_local_var_ಮೌಲ್ಯ("PS1"))
			ಗಣ_local_var_from_halves("PS1", "\\w \\$ ");
		ಆದರೆ (!get_local_var_ಮೌಲ್ಯ("PS2"))
			ಗಣ_local_var_from_halves("PS2", "> ");
#ಆದರೆ_ಅಂತ್ಯ
		ಆದರೆ (!ENABLE_FEATURE_SH_EXTRA_QUIET) {
			/* note: ash and hush share this string */
			ಬರೆ("\n\n%s %s\n"
				IF_HUSH_HELP("Enter 'help' for a list of built-in commands.\n")
				"\n",
				bb_banner,
				"hush - the humble shell"
			);
		}
	}

	parse_and_ಸತತ_file(hfopen(ಖಾಲಿ)); /* stdin */

 final_return:
	hush_exit(G.ಕೊನೆಯ_exitcode);
}

/*
 * Built-ins
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ FAST_FUNC builtin_true(ಅಕ್ಷರ **argv ಬಳಸದ_PARAM)
{
	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ FAST_FUNC builtin_false(ಅಕ್ಷರ **argv ಬಳಸದ_PARAM)
{
	ಹಿಂತಿರುಗಿ 1;
}

#ಆದರೆ ENABLE_HUSH_TEST || ENABLE_HUSH_ECHO || ENABLE_HUSH_PRINTF || ENABLE_HUSH_KILL
ಸ್ಥಿರ NOINLINE ಪೂರ್ಣಾಂಕ ಸತತ_applet_main(ಅಕ್ಷರ **argv, ಪೂರ್ಣಾಂಕ (*applet_main_func)(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ **argv))
{
	ಪೂರ್ಣಾಂಕ argc = string_array_len(argv);
	ಹಿಂತಿರುಗಿ applet_main_func(argc, argv);
}
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_HUSH_TEST || BASH_TEST2
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ FAST_FUNC builtin_test(ಅಕ್ಷರ **argv)
{
	ಹಿಂತಿರುಗಿ ಸತತ_applet_main(argv, test_main);
}
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_HUSH_ECHO
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ FAST_FUNC builtin_echo(ಅಕ್ಷರ **argv)
{
	ಹಿಂತಿರುಗಿ ಸತತ_applet_main(argv, echo_main);
}
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ ENABLE_HUSH_PRINTF
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ FAST_FUNC builtin_printf(ಅಕ್ಷರ **argv)
{
	ಹಿಂತಿರುಗಿ ಸತತ_applet_main(argv, printf_main);
}
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ ENABLE_HUSH_HELP
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ FAST_FUNC builtin_help(ಅಕ್ಷರ **argv ಬಳಸದ_PARAM)
{
	ಸ್ಥಿರಾಂಕ ರಚನೆ built_ಒಳ_command *x;

	ಬರೆ(
		"Built-in commands:\n"
		"------------------\n");
	ಗಾಗಿ (x = bltins1; x != &bltins1[ARRAY_ಗಾತ್ರ(bltins1)]; x++) {
		ಆದರೆ (x->b_descr)
			ಬರೆ("%-10s%s\n", x->b_cmd, x->b_descr);
	}
	ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
}
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ MAX_HISTORY && ENABLE_FEATURE_EDITING
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ FAST_FUNC builtin_history(ಅಕ್ಷರ **argv ಬಳಸದ_PARAM)
{
	show_history(G.ಸಾಲು_ಒಳಹರಿವು_state);
	ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
}
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ FAST_FUNC builtin_cd(ಅಕ್ಷರ **argv)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *newdir;

	argv = skip_dash_dash(argv);
	newdir = argv[0];
	ಆದರೆ (newdir == ಖಾಲಿ) {
		/* bash does nothing (exitcode 0) if HOME is ""; if it's unset,
		 * bash says "bash: cd: HOME not set" and does nothing
		 * (exitcode 1)
		 */
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *home = get_local_var_ಮೌಲ್ಯ("HOME");
		newdir = home ? home : "/";
	}
	ಆದರೆ (chdir(newdir)) {
		/* Mimic bash message exactly */
		bb_perror_ಸಂದೇಶ("cd: %s", newdir);
		ಹಿಂತಿರುಗಿ EXIT_FAILURE;
	}
	/* Read current dir (get_cwd(1) is inside) and set PWD.
	 * Note: do not enforce exporting. If PWD was unset or unexported,
	 * set it again, but do not export. bash does the same.
	 */
	ಗಣ_pwd_var(/*flag:*/ 0);
	ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ FAST_FUNC builtin_pwd(ಅಕ್ಷರ **argv ಬಳಸದ_PARAM)
{
	puts(get_cwd(0));
	ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ FAST_FUNC builtin_eval(ಅಕ್ಷರ **argv)
{
	argv = skip_dash_dash(argv);

	ಆದರೆ (!argv[0])
		ಹಿಂತಿರುಗಿ EXIT_SUCCESS;

	IF_HUSH_ಬಹುಲಕ_X(G.x_ಬಹುಲಕ_ಆಳ++;)
	//bb_error_msg("%s: ++x_mode_depth=%d", __func__, G.x_mode_depth);
	ಆದರೆ (!argv[1]) {
		/* bash:
		 * eval "echo Hi; done" ("done" is syntax error):
		 * "echo Hi" will not execute too.
		 */
		parse_and_ಸತತ_string(argv[0]);
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* "The eval utility shall construct a command by
		 * concatenating arguments together, separating
		 * each with a <space> character."
		 */
		ಅಕ್ಷರ *str, *p;
		ಚಿಹ್ನೆರಹಿತ len = 0;
		ಅಕ್ಷರ **pp = argv;
		ಮಾಡು
			len += ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(*pp) + 1;
		ಆಗ (*++pp);
		str = p = xmalloc(len);
		pp = argv;
		ಗಾಗಿ (;;) {
			p = stpcpy(p, *pp);
			pp++;
			ಆದರೆ (!*pp)
				ನಿಲ್ಲಿಸು;
			*p++ = ' ';
		}
		parse_and_ಸತತ_string(str);
		ಬಿಡುಗಡೆ(str);
	}
	IF_HUSH_ಬಹುಲಕ_X(G.x_ಬಹುಲಕ_ಆಳ--;)
	//bb_error_msg("%s: --x_mode_depth=%d", __func__, G.x_mode_depth);
	ಹಿಂತಿರುಗಿ G.ಕೊನೆಯ_exitcode;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ FAST_FUNC builtin_exec(ಅಕ್ಷರ **argv)
{
	argv = skip_dash_dash(argv);
	ಆದರೆ (argv[0] == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ EXIT_SUCCESS; /* bash does this */

	/* Careful: we can end up here after [v]fork. Do not restore
	 * tty pgrp then, only top-level shell process does that */
	ಆದರೆ (G_saved_tty_pgrp && getpid() == G.ಬೇರು_pid)
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

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ FAST_FUNC builtin_exit(ಅಕ್ಷರ **argv)
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
	ಆದರೆ (argv[0] == ಖಾಲಿ) {
#ಆದರೆ ENABLE_HUSH_TRAP
		ಆದರೆ (G.pre_trap_exitcode >= 0) /* "exit" in trap uses $? from before the trap */
			hush_exit(G.pre_trap_exitcode);
#ಆದರೆ_ಅಂತ್ಯ
		hush_exit(G.ಕೊನೆಯ_exitcode);
	}
	/* mimic bash: exit 123abc == exit 255 + error msg */
	xfunc_error_retval = 255;
	/* bash: exit -2 == exit 254, no error msg */
	hush_exit(xatoi(argv[0]) & 0xff);
}

#ಆದರೆ ENABLE_HUSH_TYPE
/* http://www.opengroup.org/onlinepubs/9699919799/utilities/type.html */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ FAST_FUNC builtin_type(ಅಕ್ಷರ **argv)
{
	ಪೂರ್ಣಾಂಕ ret = EXIT_SUCCESS;

	ಆಗ (*++argv) {
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *type;
		ಅಕ್ಷರ *ಮಾರ್ಗ_2 = ಖಾಲಿ;

		ಆದರೆ (0) {} /* make conditional compile easier below */
		/*else if (find_alias(*argv))
			type = "an alias";*/
# ಆದರೆ ENABLE_HUSH_FUNCTIONS
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಕಂಡುಹಿಡಿ_function(*argv))
			type = "a function";
# endif
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಕಂಡುಹಿಡಿ_builtin(*argv))
			type = "a shell builtin";
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((ಮಾರ್ಗ_2 = ಕಂಡುಹಿಡಿ_ಒಳ_ಮಾರ್ಗ(*argv)) != ಖಾಲಿ)
			type = ಮಾರ್ಗ_2;
		ಇಲ್ಲದಿದ್ದರೆ {
			bb_error_ಸಂದೇಶ("type: %s: not found", *argv);
			ret = EXIT_FAILURE;
			ಮುಂದುವರಿ;
		}

		ಬರೆ("%s is %s\n", *argv, type);
		ಬಿಡುಗಡೆ(ಮಾರ್ಗ_2);
	}

	ಹಿಂತಿರುಗಿ ret;
}
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ ENABLE_HUSH_READ
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ FAST_FUNC builtin_read(ಅಕ್ಷರ **argv)
{
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *r;
	ರಚನೆ builtin_read_params params;

	memset(&params, 0, ಗಾತ್ರ(params));

	/* "!": do not abort on errors.
	 * Option string must start with "sr" to match BUILTIN_READ_xxx
	 */
	params.read_flags = getopt32(argv,
# ಆದರೆ BASH_READ_D
		IF_NOT_HUSH_BASH_COMPAT("^")
		"!srn:p:t:u:d:" IF_NOT_HUSH_BASH_COMPAT("\0" "-1"/*min 1 arg*/),
		&params.opt_n, &params.opt_p, &params.opt_t, &params.opt_u, &params.opt_d
# ಇಲ್ಲದಿದ್ದರೆ
		IF_NOT_HUSH_BASH_COMPAT("^")
		"!srn:p:t:u:" IF_NOT_HUSH_BASH_COMPAT("\0" "-1"/*min 1 arg*/),
		&params.opt_n, &params.opt_p, &params.opt_t, &params.opt_u
# endif
//TODO: print "read: need variable name"
//for the case of !BASH "read" with no args (now it fails silently)
//(or maybe extend getopt32() to emit a message if "-1" fails)
	);
	ಆದರೆ ((uint32_t)params.read_flags == (uint32_t)-1)
		ಹಿಂತಿರುಗಿ EXIT_FAILURE;
	argv += optind;
	params.argv = argv;
	params.setvar = ಗಣ_local_var_from_halves;
	params.ifs = get_local_var_ಮೌಲ್ಯ("IFS"); /* can be NULL */

 again:
	r = shell_builtin_read(&params);

	ಆದರೆ ((uintptr_t)r == 1 && errno == EINTR) {
		ಚಿಹ್ನೆರಹಿತ sig = check_and_ಸತತ_traps();
		ಆದರೆ (sig != SIGINT)
			ಹೋಗು again;
	}

	ಆದರೆ ((uintptr_t)r > 1) {
		bb_simple_error_ಸಂದೇಶ(r);
		r = (ಅಕ್ಷರ*)(uintptr_t)1;
	}

	ಹಿಂತಿರುಗಿ (uintptr_t)r;
}
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ ENABLE_HUSH_UMASK
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ FAST_FUNC builtin_umask(ಅಕ್ಷರ **argv)
{
	ಪೂರ್ಣಾಂಕ rc;
	mode_t ಮುಖವಾಡ;

	rc = 1;
	ಮುಖವಾಡ = umask(0);
	argv = skip_dash_dash(argv);
	ಆದರೆ (argv[0]) {
		mode_t old_ಮುಖವಾಡ = ಮುಖವಾಡ;

		/* numeric umasks are taken as-is */
		/* symbolic umasks are inverted: "umask a=rx" calls umask(222) */
		ಆದರೆ (!ಅಂಕಿಯೇ(argv[0][0]))
			ಮುಖವಾಡ ^= 0777;
		ಮುಖವಾಡ = bb_parse_ಬಹುಲಕ(argv[0], ಮುಖವಾಡ);
		ಆದರೆ (!ಅಂಕಿಯೇ(argv[0][0]))
			ಮುಖವಾಡ ^= 0777;
		ಆದರೆ ((ಚಿಹ್ನೆರಹಿತ)ಮುಖವಾಡ > 0777) {
			ಮುಖವಾಡ = old_ಮುಖವಾಡ;
			/* bash messages:
			 * bash: umask: 'q': invalid symbolic mode operator
			 * bash: umask: 999: octal number out of range
			 */
			bb_error_ಸಂದೇಶ("%s: invalid mode '%s'", "umask", argv[0]);
			rc = 0;
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* Mimic bash */
		ಬರೆ("%04o\n", (ಚಿಹ್ನೆರಹಿತ) ಮುಖವಾಡ);
		/* fall through and restore mask which we set to 0 */
	}
	umask(ಮುಖವಾಡ);

	ಹಿಂತಿರುಗಿ !rc; /* rc != 0 - success */
}
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ ENABLE_HUSH_EXPORT || ENABLE_HUSH_READONLY || ENABLE_HUSH_ಗಣ || ENABLE_HUSH_TRAP
ಸ್ಥಿರ ಶೂನ್ಯ print_escaped(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s)
{
//TODO? bash "set" does not quote variables which contain only alnums and "%+,-./:=@_~",
// (but "export" quotes all variables, even with only these chars).
// I think quoting strings with %+,=~ looks better
// (example: "set" printing var== instead of var='=' looks strange)
// IOW: do not quote "-./:@_": / is used in pathnames, : in PATH, -._ often in file names, @ in emails

	ಆದರೆ (*s == '\'')
		ಹೋಗು squote;
	ಮಾಡು {
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p = strchrnul(s, '\'');
		/* print 'xxxx', possibly just '' */
		ಬರೆ("'%.*s'", (ಪೂರ್ಣಾಂಕ)(p - s), s);
		ಆದರೆ (*p == '\0')
			ನಿಲ್ಲಿಸು;
		s = p;
 squote:
		/* s points to '; print "'''...'''" */
		ಅಕ್ಷರ_ಬರೆ('"');
		ಮಾಡು ಅಕ್ಷರ_ಬರೆ('\''); ಆಗ (*++s == '\'');
		ಅಕ್ಷರ_ಬರೆ('"');
	} ಆಗ (*s);
}
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ ENABLE_HUSH_EXPORT || ENABLE_HUSH_LOCAL || ENABLE_HUSH_READONLY
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ helper_export_local(ಅಕ್ಷರ **argv, ಚಿಹ್ನೆರಹಿತ flags)
{
	ಮಾಡು {
		ಅಕ್ಷರ *name = *argv;
		ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name_ಕೊನೆ = endofname(name);

		ಆದರೆ (*name_ಕೊನೆ == '\0') {
			ರಚನೆ variable *var, **vpp;

			vpp = get_ptr_ಗಮ್ಯ_local_var(name);
			var = vpp ? *vpp : ಖಾಲಿ;

			ಆದರೆ (flags & SETFLAG_UNEXPORT) {
				/* export -n NAME (without =VALUE) */
				ಆದರೆ (var) {
					var->flg_export = 0;
					debug_printf_env("%s: unsetenv '%s'\n", __func__, name);
					unsetenv(name);
				} /* else: export -n NOT_EXISTING_VAR: no-op */
				ಮುಂದುವರಿ;
			}
			ಆದರೆ (flags & SETFLAG_EXPORT) {
				/* export NAME (without =VALUE) */
				ಆದರೆ (var) {
					var->flg_export = 1;
					debug_printf_env("%s: putenv '%s'\n", __func__, var->varstr);
					putenv(var->varstr);
					ಮುಂದುವರಿ;
				}
			}
			ಆದರೆ (flags & SETFLAG_MAKE_RO) {
				/* readonly NAME (without =VALUE) */
				ಆದರೆ (var) {
					var->flg_read_only = 1;
					ಮುಂದುವರಿ;
				}
			}
# ಆದರೆ ENABLE_HUSH_LOCAL
			/* Is this "local" bltin? */
			ಆದರೆ (!(flags & (SETFLAG_EXPORT|SETFLAG_UNEXPORT|SETFLAG_MAKE_RO))) {
				ಚಿಹ್ನೆರಹಿತ lvl = flags >> SETFLAG_VARLVL_ಸರಿಸು;
				ಆದರೆ (var && var->var_nest_level == lvl) {
					/* "local x=abc; ...; local x" - ignore second local decl */
					ಮುಂದುವರಿ;
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
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಆದರೆ (*name_ಕೊನೆ != '=') {
				bb_error_ಸಂದೇಶ("'%s': bad variable name", name);
				/* do not parse following argv[]s: */
				ಹಿಂತಿರುಗಿ 1;
			}
			/* (Un)exporting/making local NAME=VALUE */
			name = xstrdup(name);
			/* Testcase: export PS1='\w \$ ' */
			unbackslash(name);
		}
		debug_printf_env("%s: set_local_var('%s')\n", __func__, name);
		ಆದರೆ (ಗಣ_local_var(name, flags))
			ಹಿಂತಿರುಗಿ EXIT_FAILURE;
	} ಆಗ (*++argv);
	ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
}
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ ENABLE_HUSH_EXPORT
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ FAST_FUNC builtin_export(ಅಕ್ಷರ **argv)
{
	ಚಿಹ್ನೆರಹಿತ opt_unexport;

# ಆದರೆ ENABLE_HUSH_EXPORT_N
	/* "!": do not abort on errors */
	opt_unexport = getopt32(argv, "!n");
	ಆದರೆ (opt_unexport == (uint32_t)-1)
		ಹಿಂತಿರುಗಿ EXIT_FAILURE;
	argv += optind;
# ಇಲ್ಲದಿದ್ದರೆ
	opt_unexport = 0;
	argv++;
# endif

	ಆದರೆ (argv[0] == ಖಾಲಿ) {
		ಅಕ್ಷರ **e = environ;
		ಆದರೆ (e) {
			ಆಗ (*e) {
# ಆದರೆ 0
				puts(*e++);
# ಇಲ್ಲದಿದ್ದರೆ
				/* ash emits: export VAR='VAL'
				 * bash: declare -x VAR="VAL"
				 * we follow ash example */
				ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s = *e++;
				ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p = strchr(s, '=');

				ಆದರೆ (!p) /* wtf? take next variable */
					ಮುಂದುವರಿ;
				/* "export VAR=" */
				ಬರೆ("%s %.*s", "export", (ಪೂರ್ಣಾಂಕ)(p - s) + 1, s);
				print_escaped(p + 1);
				ಅಕ್ಷರ_ಬರೆ('\n');
# endif
			}
			/*fflush_all(); - done after each builtin anyway */
		}
		ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
	}

	ಹಿಂತಿರುಗಿ helper_export_local(argv, opt_unexport ? SETFLAG_UNEXPORT : SETFLAG_EXPORT);
}
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ ENABLE_HUSH_LOCAL
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ FAST_FUNC builtin_local(ಅಕ್ಷರ **argv)
{
	ಆದರೆ (G.func_nest_level == 0) {
		bb_error_ಸಂದೇಶ("%s: not in a function", argv[0]);
		ಹಿಂತಿರುಗಿ EXIT_FAILURE; /* bash compat */
	}
//TODO? ash and bash support "local -" special form,
//which saves/restores $- around function call (including async returns, such as ^C)
//(IOW: it makes "set +/-..." effects local)
	argv++;
	/* Since all builtins run in a nested variable level,
	 * need to use level - 1 here. Or else the variable will be removed at once
	 * after builtin returns.
	 */
	ಹಿಂತಿರುಗಿ helper_export_local(argv, (G.var_nest_level - 1) << SETFLAG_VARLVL_ಸರಿಸು);
}
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ ENABLE_HUSH_READONLY
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ FAST_FUNC builtin_readonly(ಅಕ್ಷರ **argv)
{
	argv++;
	ಆದರೆ (*argv == ಖಾಲಿ) {
		/* bash: readonly [-p]: list all readonly VARs
		 * (-p has no effect in bash)
		 */
		ರಚನೆ variable *e;
		ಗಾಗಿ (e = G.ತುದಿ_var; e; e = e->ಮುಂದಿನ) {
			ಆದರೆ (e->flg_read_only) {
				ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s = e->varstr;
				ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p = strchr(s, '=');

				ಆದರೆ (!p) /* wtf? take next variable */
					ಮುಂದುವರಿ;
				/* "readonly VAR=" */
				ಬರೆ("%s %.*s", "readonly", (ಪೂರ್ಣಾಂಕ)(p - s) + 1, s);
				print_escaped(p + 1);
				ಅಕ್ಷರ_ಬರೆ('\n');
			}
		}
		ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
	}
	ಹಿಂತಿರುಗಿ helper_export_local(argv, SETFLAG_MAKE_RO);
}
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ ENABLE_HUSH_UNSET
/* http://www.opengroup.org/onlinepubs/9699919799/utilities/V3_chap02.html#unset */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ FAST_FUNC builtin_unset(ಅಕ್ಷರ **argv)
{
	ಪೂರ್ಣಾಂಕ ret;
	ಚಿಹ್ನೆರಹಿತ opts;

	/* "!": do not abort on errors */
	/* "+": stop at 1st non-option */
	opts = getopt32(argv, "!+vf");
	ಆದರೆ (opts == (ಚಿಹ್ನೆರಹಿತ)-1)
		ಹಿಂತಿರುಗಿ EXIT_FAILURE;
	ಆದರೆ (opts == 3) {
		bb_simple_error_ಸಂದೇಶ("unset: -v and -f are exclusive");
		ಹಿಂತಿರುಗಿ EXIT_FAILURE;
	}
	argv += optind;

	ret = EXIT_SUCCESS;
	ಆಗ (*argv) {
		ಆದರೆ (!(opts & 2)) { /* not -f */
			ಆದರೆ (unset_local_var(*argv)) {
				/* unset <nonexistent_var> doesn't fail.
				 * Error is when one tries to unset RO var.
				 * Message was printed by unset_local_var. */
				ret = EXIT_FAILURE;
			}
		}
# ಆದರೆ ENABLE_HUSH_FUNCTIONS
		ಇಲ್ಲದಿದ್ದರೆ {
			unset_func(*argv);
		}
# endif
		argv++;
	}
	ಹಿಂತಿರುಗಿ ret;
}
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ ENABLE_HUSH_ಗಣ
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ FAST_FUNC builtin_ಗಣ(ಅಕ್ಷರ **argv)
{
	ಪೂರ್ಣಾಂಕ n;
	ಅಕ್ಷರ **pp, **g_argv;
	ಅಕ್ಷರ *arg = *++argv;

	ಆದರೆ (arg == ಖಾಲಿ) {
		ರಚನೆ variable *e;
		ಗಾಗಿ (e = G.ತುದಿ_var; e; e = e->ಮುಂದಿನ) {
			ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s = e->varstr;
			ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *p = strchr(s, '=');

			ಆದರೆ (!p) /* wtf? take next variable */
				ಮುಂದುವರಿ;
			/* var= */
			ಬರೆ("%.*s", (ಪೂರ್ಣಾಂಕ)(p - s) + 1, s);
			print_escaped(p + 1);
			ಅಕ್ಷರ_ಬರೆ('\n');
		}
		ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
	}

	ಮಾಡು {
		ಆದರೆ (strcmp(arg, "--") == 0) {
			++argv;
			ಹೋಗು ಗಣ_argv;
		}
		ಆದರೆ (arg[0] != '+' && arg[0] != '-')
			ನಿಲ್ಲಿಸು;
		ಗಾಗಿ (n = 1; arg[n]; ++n) {
			ಆದರೆ (ಗಣ_ಬಹುಲಕ((arg[0] == '-'), arg[n], argv[1])) {
				bb_error_ಸಂದೇಶ("%s: %s: invalid option", "set", arg);
				ಹಿಂತಿರುಗಿ EXIT_FAILURE;
			}
			ಆದರೆ (arg[n] == 'o' && argv[1])
				argv++;
		}
	} ಆಗ ((arg = *++argv) != ಖಾಲಿ);
	/* Now argv[0] is 1st argument */

	ಆದರೆ (arg == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
 ಗಣ_argv:

	/* NB: G.global_argv[0] ($0) is never freed/changed */
	g_argv = G.global_argv;
	ಆದರೆ (G.global_ವಾದಗಳು_malloced) {
		pp = g_argv;
		ಆಗ (*++pp)
			ಬಿಡುಗಡೆ(*pp);
		g_argv[1] = ಖಾಲಿ;
	} ಇಲ್ಲದಿದ್ದರೆ {
		G.global_ವಾದಗಳು_malloced = 1;
		pp = xzalloc(ಗಾತ್ರ(pp[0]) * 2);
		pp[0] = g_argv[0]; /* retain $0 */
		g_argv = pp;
	}
	/* This realloc's G.global_argv */
	G.global_argv = pp = ಕೂಡಿಸು_ಅಕ್ಷರಸರಣಿಗಳು_ಗಮ್ಯ_ಅಕ್ಷರಸರಣಿಗಳು(g_argv, argv, /*dup:*/ 1);

	G.global_argc = 1 + string_array_len(pp + 1);

	ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
}
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ FAST_FUNC builtin_ಸರಿಸು(ಅಕ್ಷರ **argv)
{
	ಪೂರ್ಣಾಂಕ n = 1;
	argv = skip_dash_dash(argv);
	ಆದರೆ (argv[0]) {
		n = bb_strtou(argv[0], ಖಾಲಿ, 10);
		ಆದರೆ (errno || n < 0) {
			/* shared string with ash.c */
			bb_error_ಸಂದೇಶ("Illegal number: %s", argv[0]);
			/*
			 * ash aborts in this case.
			 * bash prints error message and set $? to 1.
			 * Interestingly, for "shift 99999" bash does not
			 * print error message, but does set $? to 1
			 * (and does no shifting at all).
			 */
		}
	}
	ಆದರೆ (n >= 0 && n < G.global_argc) {
		ಆದರೆ (G_global_ವಾದಗಳು_malloced) {
			ಪೂರ್ಣಾಂಕ m = 1;
			ಆಗ (m <= n)
				ಬಿಡುಗಡೆ(G.global_argv[m++]);
		}
		G.global_argc -= n;
		memmove(&G.global_argv[1], &G.global_argv[n+1],
				G.global_argc * ಗಾತ್ರ(G.global_argv[0]));
		ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
	}
	ಹಿಂತಿರುಗಿ EXIT_FAILURE;
}

#ಆದರೆ ENABLE_HUSH_GETOPTS
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ FAST_FUNC builtin_getopts(ಅಕ್ಷರ **argv)
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
	ಅಕ್ಷರ cbuf[2];
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *cp, *optstring, *var;
	ಪೂರ್ಣಾಂಕ c, n, exitcode, my_opterr;
	ಚಿಹ್ನೆರಹಿತ ಎಣಿಕೆ_2;

	optstring = *++argv;
	ಆದರೆ (!optstring || !(var = *++argv)) {
		bb_simple_error_ಸಂದೇಶ("usage: getopts OPTSTRING VAR [ARGS]");
		ಹಿಂತಿರುಗಿ EXIT_FAILURE;
	}

	ಆದರೆ (argv[1])
		argv[0] = G.global_argv[0]; /* for error messages in getopt() */
	ಇಲ್ಲದಿದ್ದರೆ
		argv = G.global_argv;
	cbuf[1] = '\0';

	my_opterr = 0;
	ಆದರೆ (optstring[0] != ':') {
		cp = get_local_var_ಮೌಲ್ಯ("OPTERR");
		/* 0 if "OPTERR=0", 1 otherwise */
		my_opterr = (!cp || NOT_LONE_CHAR(cp, '0'));
	}

	/* getopts stops on first non-option. Add "+" to force that */
	/*if (optstring[0] != '+')*/ {
		ಅಕ್ಷರ *s = alloca(ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(optstring) + 2);
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
	ಎಣಿಕೆ_2 = 0;
	n = string_array_len(argv);
	ಮಾಡು {
		optarg = ಖಾಲಿ;
		opterr = (ಎಣಿಕೆ_2 < G.getopt_ಎಣಿಕೆ) ? 0 : my_opterr;
		c = getopt(n, argv, optstring);
		ಆದರೆ (c < 0)
			ನಿಲ್ಲಿಸು;
		ಎಣಿಕೆ_2++;
	} ಆಗ (ಎಣಿಕೆ_2 <= G.getopt_ಎಣಿಕೆ);

	/* Set OPTIND. Prevent resetting of the magic counter! */
	ಗಣ_local_var_from_halves("OPTIND", utoa(optind));
	G.getopt_ಎಣಿಕೆ = ಎಣಿಕೆ_2; /* "next time, give me N+1'th result" */
	GETOPT_RESET(); /* just in case */

	/* Set OPTARG */
	/* Always set or unset, never left as-is, even on exit/error:
	 * "If no option was found, or if the option that was found
	 * does not have an option-argument, OPTARG shall be unset."
	 */
	cp = optarg;
	ಆದರೆ (c == '?') {
		/* If ":optstring" and unknown option is seen,
		 * it is stored to OPTARG.
		 */
		ಆದರೆ (optstring[1] == ':') {
			cbuf[0] = optopt;
			cp = cbuf;
		}
	}
	ಆದರೆ (cp)
		ಗಣ_local_var_from_halves("OPTARG", cp);
	ಇಲ್ಲದಿದ್ದರೆ
		unset_local_var("OPTARG");

	/* Convert -1 to "?" */
	exitcode = EXIT_SUCCESS;
	ಆದರೆ (c < 0) { /* -1: end of options */
		exitcode = EXIT_FAILURE;
		c = '?';
	}

	/* Set VAR */
	cbuf[0] = c;
	ಗಣ_local_var_from_halves(var, cbuf);

	ಹಿಂತಿರುಗಿ exitcode;
}
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ FAST_FUNC builtin_ಮೂಲ(ಅಕ್ಷರ **argv)
{
	ಅಕ್ಷರ *arg_ಮಾರ್ಗ, *filename;
	HFILE *ಒಳಹರಿವು;
	save_arg_t sv;
	ಅಕ್ಷರ *ವಾದಗಳು_need_save;
#ಆದರೆ ENABLE_HUSH_FUNCTIONS
	smallint sv_flg;
#ಆದರೆ_ಅಂತ್ಯ

	argv = skip_dash_dash(argv);
	filename = argv[0];
	ಆದರೆ (!filename) {
		/* bash says: "bash: .: filename argument required" */
		ಹಿಂತಿರುಗಿ 2; /* bash compat */
	}
	arg_ಮಾರ್ಗ = ಖಾಲಿ;
	ಆದರೆ (!strchr(filename, '/')) {
		arg_ಮಾರ್ಗ = ಕಂಡುಹಿಡಿ_ಒಳ_ಮಾರ್ಗ(filename);
		ಆದರೆ (arg_ಮಾರ್ಗ)
			filename = arg_ಮಾರ್ಗ;
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (!ENABLE_HUSH_BASH_ಮೂಲ_CURDIR) {
			errno = ENOENT;
			bb_simple_perror_ಸಂದೇಶ(filename);
			ಹಿಂತಿರುಗಿ EXIT_FAILURE;
		}
	}
	ಒಳಹರಿವು = hfopen(filename);
	ಬಿಡುಗಡೆ(arg_ಮಾರ್ಗ);
	ಆದರೆ (!ಒಳಹರಿವು) {
		bb_perror_ಸಂದೇಶ("%s", filename);
		/* POSIX: non-interactive shell should abort here,
		 * not merely fail. So far no one complained :)
		 */
		ಹಿಂತಿರುಗಿ EXIT_FAILURE;
	}

#ಆದರೆ ENABLE_HUSH_FUNCTIONS
	sv_flg = G_flag_return_ಒಳ_ಪ್ರಗತಿ;
	/* "we are inside sourced file, ok to use return" */
	G_flag_return_ಒಳ_ಪ್ರಗತಿ = -1;
#ಆದರೆ_ಅಂತ್ಯ
	ವಾದಗಳು_need_save = argv[1]; /* used as a boolean variable */
	ಆದರೆ (ವಾದಗಳು_need_save)
		save_and_replace_G_ವಾದಗಳು(&sv, argv);

	/* "false; . ./empty_line; echo Zero:$?" should print 0 */
	G.ಕೊನೆಯ_exitcode = 0;
	parse_and_ಸತತ_file(ಒಳಹರಿವು);
	hfclose(ಒಳಹರಿವು);

	ಆದರೆ (ವಾದಗಳು_need_save) /* can't use argv[1] instead: "shift" can mangle it */
		restore_G_ವಾದಗಳು(&sv, argv);
#ಆದರೆ ENABLE_HUSH_FUNCTIONS
	G_flag_return_ಒಳ_ಪ್ರಗತಿ = sv_flg;
#ಆದರೆ_ಅಂತ್ಯ

	ಹಿಂತಿರುಗಿ G.ಕೊನೆಯ_exitcode;
}

#ಆದರೆ ENABLE_HUSH_TRAP
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ FAST_FUNC builtin_trap(ಅಕ್ಷರ **argv)
{
	ಪೂರ್ಣಾಂಕ sig;
	ಅಕ್ಷರ *new_cmd;

	ಆದರೆ (!G_traps)
		G_traps = xzalloc(ಗಾತ್ರ(G_traps[0]) * NSIG);

	argv++;
	ಆದರೆ (!*argv) {
		ಪೂರ್ಣಾಂಕ i;
		/* No args: print all trapped */
		ಗಾಗಿ (i = 0; i < NSIG; ++i) {
			ಆದರೆ (G_traps[i]) {
				ಬರೆ("trap -- ");
				print_escaped(G_traps[i]);
				/* note: bash adds "SIG", but only if invoked
				 * as "bash". If called as "sh", or if set -o posix,
				 * then it prints short signal names.
				 * We are printing short names: */
				ಬರೆ(" %s\n", get_signame(i));
			}
		}
		/*fflush_all(); - done after each builtin anyway */
		ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
	}

	new_cmd = ಖಾಲಿ;
	/* If first arg is a number: reset all specified signals */
	sig = bb_strtou(*argv, ಖಾಲಿ, 10);
	ಆದರೆ (errno == 0) {
		ಪೂರ್ಣಾಂಕ ret;
 process_sig_list:
		ret = EXIT_SUCCESS;
		ಆಗ (*argv) {
			sighandler_t handler;

			sig = get_signum(*argv++);
			ಆದರೆ (sig < 0) {
				ret = EXIT_FAILURE;
				/* Mimic bash message exactly */
				bb_error_ಸಂದೇಶ("trap: %s: invalid signal specification", argv[-1]);
				ಮುಂದುವರಿ;
			}

			ಬಿಡುಗಡೆ(G_traps[sig]);
			G_traps[sig] = xstrdup(new_cmd);

			debug_printf("trap: setting SIG%s (%i) to '%s'\n",
				get_signame(sig), sig, G_traps[sig]);

			/* There is no signal for 0 (EXIT) */
			ಆದರೆ (sig == 0)
				ಮುಂದುವರಿ;

			ಆದರೆ (new_cmd)
				handler = (new_cmd[0] ? ದಾಖಲೆ_pending_signo : SIG_IGN);
			ಇಲ್ಲದಿದ್ದರೆ
				/* We are removing trap handler */
				handler = pick_sighandler(sig);
			install_sighandler(sig, handler);
		}
		ಹಿಂತಿರುಗಿ ret;
	}

	ಆದರೆ (!argv[1]) { /* no second arg */
		bb_simple_error_ಸಂದೇಶ("trap: invalid arguments");
		ಹಿಂತಿರುಗಿ EXIT_FAILURE;
	}

	/* First arg is "-": reset all specified to default */
	/* First arg is "--": skip it, the rest is "handler SIGs..." */
	/* Everything else: set arg as signal handler
	 * (includes "" case, which ignores signal) */
	ಆದರೆ (argv[0][0] == '-') {
		ಆದರೆ (argv[0][1] == '\0') { /* "-" */
			/* new_cmd remains NULL: "reset these sigs" */
			ಹೋಗು reset_traps;
		}
		ಆದರೆ (argv[0][1] == '-' && argv[0][2] == '\0') { /* "--" */
			argv++;
		}
		/* else: "-something", no special meaning */
	}
	new_cmd = *argv;
 reset_traps:
	argv++;
	ಹೋಗು process_sig_list;
}
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ ENABLE_HUSH_ಕೆಲಸ
ಸ್ಥಿರ ರಚನೆ pipe *parse_jobspec(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *str)
{
	ರಚನೆ pipe *pi;
	ಚಿಹ್ನೆರಹಿತ jobnum;

	ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಓದು(str, "%%%u", &jobnum) != 1) {
		ಆದರೆ (str[0] != '%'
		 || (str[1] != '%' && str[1] != '+' && str[1] != '\0')
		) {
			bb_error_ಸಂದೇಶ("bad argument '%s'", str);
			ಹಿಂತಿರುಗಿ ಖಾಲಿ;
		}
		/* It is "%%", "%+" or "%" - current job */
		jobnum = G.ಕೊನೆಯ_jobid;
		ಆದರೆ (jobnum == 0) {
			bb_simple_error_ಸಂದೇಶ("no current job");
			ಹಿಂತಿರುಗಿ ಖಾಲಿ;
		}
	}
	ಗಾಗಿ (pi = G.ಕೆಲಸ_list; pi; pi = pi->ಮುಂದಿನ) {
		ಆದರೆ (pi->jobid == jobnum) {
			ಹಿಂತಿರುಗಿ pi;
		}
	}
	bb_error_ಸಂದೇಶ("%u: no such job", jobnum);
	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ FAST_FUNC builtin_jobs(ಅಕ್ಷರ **argv ಬಳಸದ_PARAM)
{
	ರಚನೆ pipe *ಕೆಲಸ;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿತಿ_string;

	checkjobs(ಖಾಲಿ, 0 /*(no pid to wait for)*/);
	ಗಾಗಿ (ಕೆಲಸ = G.ಕೆಲಸ_list; ಕೆಲಸ; ಕೆಲಸ = ಕೆಲಸ->ಮುಂದಿನ) {
		ಆದರೆ (ಕೆಲಸ->alive_cmds == ಕೆಲಸ->stopped_cmds)
			ಸ್ಥಿತಿ_string = "Stopped";
		ಇಲ್ಲದಿದ್ದರೆ
			ಸ್ಥಿತಿ_string = "Running";

		ಬರೆ(ಕೆಲಸ_ಸ್ಥಿತಿ_FORMAT, ಕೆಲಸ->jobid, ಸ್ಥಿತಿ_string, ಕೆಲಸ->cmdtext);
	}

	clean_ಮೇಲೆ_ಕೊನೆಯ_dead_ಕೆಲಸ();

	ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
}

/* built-in 'fg' and 'bg' handler */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ FAST_FUNC builtin_fg_bg(ಅಕ್ಷರ **argv)
{
	ಪೂರ್ಣಾಂಕ i;
	ರಚನೆ pipe *pi;

	ಆದರೆ (!G_interactive_fd)
		ಹಿಂತಿರುಗಿ EXIT_FAILURE;

	/* If they gave us no args, assume they want the last backgrounded task */
	ಆದರೆ (!argv[1]) {
		ಗಾಗಿ (pi = G.ಕೆಲಸ_list; pi; pi = pi->ಮುಂದಿನ) {
			ಆದರೆ (pi->jobid == G.ಕೊನೆಯ_jobid) {
				ಹೋಗು ಸಿಕ್ಕಿತು;
			}
		}
		bb_error_ಸಂದೇಶ("%s: no current job", argv[0]);
		ಹಿಂತಿರುಗಿ EXIT_FAILURE;
	}

	pi = parse_jobspec(argv[1]);
	ಆದರೆ (!pi)
		ಹಿಂತಿರುಗಿ EXIT_FAILURE;
 ಸಿಕ್ಕಿತು:
	/* TODO: bash prints a string representation
	 * of job being foregrounded (like "sleep 1 | cat") */
	ಆದರೆ (argv[0][0] == 'f' && G_saved_tty_pgrp) {
		/* Put the job into the foreground. */
		tcsetpgrp(G_interactive_fd, pi->pgrp);
	}

	/* Restart the processes in the job */
	debug_printf_jobs("reviving %d procs, pgrp %d\n", pi->ಅಂಕೆ_cmds, pi->pgrp);
	ಗಾಗಿ (i = 0; i < pi->ಅಂಕೆ_cmds; i++) {
		debug_printf_jobs("reviving pid %d\n", pi->cmds[i].pid);
	}
	pi->stopped_cmds = 0;

	i = kill(- pi->pgrp, SIGCONT);
	ಆದರೆ (i < 0) {
		ಆದರೆ (errno == ESRCH) {
			delete_ಪೂರ್ಣಗೊಂಡ_ಕೆಲಸ(pi);
			ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
		}
		bb_simple_perror_ಸಂದೇಶ("kill (SIGCONT)");
	}

	ಆದರೆ (argv[0][0] == 'f') {
		remove_ಕೆಲಸ_from_ಕೋಷ್ಟಕ(pi); /* FG job shouldn't be in job table */
		ಹಿಂತಿರುಗಿ checkjobs_and_fg_shell(pi);
	}
	ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
}
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ ENABLE_HUSH_KILL
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ FAST_FUNC builtin_kill(ಅಕ್ಷರ **argv)
{
	ಪೂರ್ಣಾಂಕ ret = 0;

# ಆದರೆ ENABLE_HUSH_ಕೆಲಸ
	ಆದರೆ (argv[1] && strcmp(argv[1], "-l") != 0) {
		ಪೂರ್ಣಾಂಕ i = 1;

		ಮಾಡು {
			ರಚನೆ pipe *pi;
			ಅಕ್ಷರ *dst;
			ಪೂರ್ಣಾಂಕ j, n;

			ಆದರೆ (argv[i][0] != '%')
				ಮುಂದುವರಿ;
			/*
			 * "kill %N" - job kill
			 * Converting to pgrp / pid kill
			 */
			pi = parse_jobspec(argv[i]);
			ಆದರೆ (!pi) {
				/* Eat bad jobspec */
				j = i;
				ಮಾಡು {
					j++;
					argv[j - 1] = argv[j];
				} ಆಗ (argv[j]);
				ret = 1;
				i--;
				ಮುಂದುವರಿ;
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
			n = G_interactive_fd ? 1 : pi->ಅಂಕೆ_cmds;
			dst = alloca(n * ಗಾತ್ರ(ಪೂರ್ಣಾಂಕ)*4);
			argv[i] = dst;
			ಆದರೆ (G_interactive_fd)
				dst += sprintf(dst, " -%u", (ಪೂರ್ಣಾಂಕ)pi->pgrp);
			ಇಲ್ಲದಿದ್ದರೆ ಗಾಗಿ (j = 0; j < n; j++) {
				ರಚನೆ command *cmd = &pi->cmds[j];
				/* Skip exited members of the job */
				ಆದರೆ (cmd->pid == 0)
					ಮುಂದುವರಿ;
				/*
				 * kill_main has matching code to expect
				 * leading space. Needed to not confuse
				 * negative pids with "kill -SIGNAL_NO" syntax
				 */
				dst += sprintf(dst, " %u", (ಪೂರ್ಣಾಂಕ)cmd->pid);
			}
			*dst = '\0';
		} ಆಗ (argv[++i]);
	}
# endif

	ಆದರೆ (argv[1] || ret == 0) {
		ret = ಸತತ_applet_main(argv, kill_main);
	}
	/* else: ret = 1, "kill %bad_jobspec" case */
	ಹಿಂತಿರುಗಿ ret;
}
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ ENABLE_HUSH_WAIT
/* http://www.opengroup.org/onlinepubs/9699919799/utilities/wait.html */
# ಆದರೆ !ENABLE_HUSH_ಕೆಲಸ
#  define wait_for_ಮಗು_or_signal(pipe,pid) wait_for_ಮಗು_or_signal(pid)
# endif
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ wait_for_ಮಗು_or_signal(ರಚನೆ pipe *waitfor_pipe, pid_t waitfor_pid)
{
	ಪೂರ್ಣಾಂಕ ret = 0;
	ಗಾಗಿ (;;) {
		ಪೂರ್ಣಾಂಕ sig;
		sigset_t oldset;

		ಆದರೆ (!sigisemptyset(&G.pending_ಗಣ))
			ಹೋಗು check_sig;

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

		ಆದರೆ (!sigisemptyset(&G.pending_ಗಣ)) {
			/* Crap! we raced with some signal! */
			ಹೋಗು restore;
		}

		/*errno = 0; - checkjobs does this */
/* Can't pass waitfor_pipe into checkjobs(): it won't be interruptible */
		ret = checkjobs(ಖಾಲಿ, waitfor_pid); /* waitpid(WNOHANG) inside */
		debug_printf_exec("checkjobs:%d\n", ret);
# ಆದರೆ ENABLE_HUSH_ಕೆಲಸ
		ಆದರೆ (waitfor_pipe) {
			ಪೂರ್ಣಾಂಕ rcode = ಕೆಲಸ_exited_or_stopped(waitfor_pipe);
			debug_printf_exec("job_exited_or_stopped:%d\n", rcode);
			ಆದರೆ (rcode >= 0) {
				ret = rcode;
				sigprocmask(SIG_SETMASK, &oldset, ಖಾಲಿ);
				ನಿಲ್ಲಿಸು;
			}
		}
# endif
		/* if ECHILD, there are no children (ret is -1 or 0) */
		/* if ret == 0, no children changed state */
		/* if ret != 0, it's exitcode+1 of exited waitfor_pid child */
		ಆದರೆ (errno == ECHILD || ret) {
			ret--;
			ಆದರೆ (ret < 0) /* if ECHILD, may need to fix "ret" */
				ret = 0;
# ಆದರೆ ENABLE_HUSH_BASH_COMPAT
			ಆದರೆ (waitfor_pid == -1 && errno == ECHILD) {
				/* exitcode of "wait -n" with no children is 127, not 0 */
				ret = 127;
			}
# endif
			sigprocmask(SIG_SETMASK, &oldset, ಖಾಲಿ);
			ನಿಲ್ಲಿಸು;
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
		sigprocmask(SIG_SETMASK, &oldset, ಖಾಲಿ);
 check_sig:
		/* So, did we get a signal? */
		sig = check_and_ಸತತ_traps();
		ಆದರೆ (sig /*&& sig != SIGCHLD - always true */) {
			/* Do this for any (non-ignored) signal, not only for ^C */
			ret = 128 | sig;
			ನಿಲ್ಲಿಸು;
		}
		/* SIGCHLD, or no signal, or ignored one, such as SIGQUIT. Repeat */
	}
	ಹಿಂತಿರುಗಿ ret;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ FAST_FUNC builtin_wait(ಅಕ್ಷರ **argv)
{
	ಪೂರ್ಣಾಂಕ ret;
	ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2;

	argv = skip_dash_dash(argv);
# ಆದರೆ ENABLE_HUSH_BASH_COMPAT
	ಆದರೆ (argv[0] && strcmp(argv[0], "-n") == 0) {
		/* wait -n */
		/* (bash accepts "wait -n PID" too and ignores PID) */
		G.dead_ಕೆಲಸ_exitcode = -1;
		ಹಿಂತಿರುಗಿ wait_for_ಮಗು_or_signal(ಖಾಲಿ, -1 /*no job, wait for one job*/);
	}
# endif
	ಆದರೆ (argv[0] == ಖಾಲಿ) {
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
		ಹಿಂತಿರುಗಿ wait_for_ಮಗು_or_signal(ಖಾಲಿ, 0 /*no job and no pid to wait for*/);
	}

	ಮಾಡು {
		pid_t pid = bb_strtou(*argv, ಖಾಲಿ, 10);
		ಆದರೆ (errno || pid <= 0) {
# ಆದರೆ ENABLE_HUSH_ಕೆಲಸ
			ಆದರೆ (argv[0][0] == '%') {
				ರಚನೆ pipe *wait_pipe;
				ret = 127; /* bash compat for bad jobspecs */
				wait_pipe = parse_jobspec(*argv);
				ಆದರೆ (wait_pipe) {
					ret = ಕೆಲಸ_exited_or_stopped(wait_pipe);
					ಆದರೆ (ret < 0) {
						ret = wait_for_ಮಗು_or_signal(wait_pipe, 0);
					} ಇಲ್ಲದಿದ್ದರೆ {
						/* waiting on "last dead job" removes it */
						clean_ಮೇಲೆ_ಕೊನೆಯ_dead_ಕೆಲಸ();
					}
				}
				/* else: parse_jobspec() already emitted error msg */
				ಮುಂದುವರಿ;
			}
# endif
			/* mimic bash message */
			bb_error_ಸಂದೇಶ("wait: '%s': not a pid or valid job spec", *argv);
			ret = EXIT_FAILURE;
			ಮುಂದುವರಿ; /* bash checks all argv[] */
		}

		/* Do we have such child? */
		ret = waitpid(pid, &ಸ್ಥಿತಿ_2, WNOHANG);
		ಆದರೆ (ret < 0) {
			/* No */
			ret = 127;
			ಆದರೆ (errno == ECHILD) {
				ಆದರೆ (pid == G.ಕೊನೆಯ_bg_pid) {
					/* "wait $!" but last bg task has already exited. Try:
					 * (sleep 1; exit 3) & sleep 2; echo $?; wait $!; echo $?
					 * In bash it prints exitcode 0, then 3.
					 * In dash, it is 127.
					 */
					ret = G.ಕೊನೆಯ_bg_pid_exitcode;
				} ಇಲ್ಲದಿದ್ದರೆ {
					/* Example: "wait 1". mimic bash message */
					bb_error_ಸಂದೇಶ("wait: pid %u is not a child of this shell", (ಚಿಹ್ನೆರಹಿತ)pid);
				}
			} ಇಲ್ಲದಿದ್ದರೆ {
				/* ??? */
				bb_perror_ಸಂದೇಶ("wait %s", *argv);
			}
			ಮುಂದುವರಿ; /* bash checks all argv[] */
		}
		ಆದರೆ (ret == 0) {
			/* Yes, and it still runs */
			ret = wait_for_ಮಗು_or_signal(ಖಾಲಿ, pid);
		} ಇಲ್ಲದಿದ್ದರೆ {
			/* Yes, and it just exited */
			process_wait_ಫಲಿತಾಂಶ(ಖಾಲಿ, pid, ಸ್ಥಿತಿ_2);
			ret = WEXITSTATUS(ಸ್ಥಿತಿ_2);
			ಆದರೆ (WIFSIGNALED(ಸ್ಥಿತಿ_2))
				ret = 128 | WTERMSIG(ಸ್ಥಿತಿ_2);
		}
	} ಆಗ (*++argv);

	ಹಿಂತಿರುಗಿ ret;
}
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ ENABLE_HUSH_LOOPS || ENABLE_HUSH_FUNCTIONS
ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ parse_numeric_argv1(ಅಕ್ಷರ **argv, ಚಿಹ್ನೆರಹಿತ def, ಚಿಹ್ನೆರಹಿತ def_min)
{
	ಆದರೆ (argv[1]) {
		def = bb_strtou(argv[1], ಖಾಲಿ, 10);
		ಆದರೆ (errno || def < def_min || argv[2]) {
			bb_error_ಸಂದೇಶ("%s: bad arguments", argv[0]);
			def = UINT_MAX;
		}
	}
	ಹಿಂತಿರುಗಿ def;
}
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ ENABLE_HUSH_LOOPS
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ FAST_FUNC builtin_break(ಅಕ್ಷರ **argv)
{
	ಚಿಹ್ನೆರಹಿತ ಆಳ;
	ಆದರೆ (G.ಆಳ_ದ_loop == 0) {
		bb_error_ಸಂದೇಶ("%s: only meaningful in a loop", argv[0]);
		/* if we came from builtin_continue(), need to undo "= 1" */
		G.flag_break_continue = 0;
		ಹಿಂತಿರುಗಿ EXIT_SUCCESS; /* bash compat */
	}
	G.flag_break_continue++; /* BC_BREAK = 1, or BC_CONTINUE = 2 */

	G.ಆಳ_break_continue = ಆಳ = parse_numeric_argv1(argv, 1, 1);
	ಆದರೆ (ಆಳ == UINT_MAX)
		G.flag_break_continue = BC_BREAK;
	ಆದರೆ (G.ಆಳ_ದ_loop < ಆಳ)
		G.ಆಳ_break_continue = G.ಆಳ_ದ_loop;

	ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ FAST_FUNC builtin_continue(ಅಕ್ಷರ **argv)
{
	G.flag_break_continue = 1; /* BC_CONTINUE = 2 = 1+1 */
	ಹಿಂತಿರುಗಿ builtin_break(argv);
}
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ ENABLE_HUSH_FUNCTIONS
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ FAST_FUNC builtin_return(ಅಕ್ಷರ **argv)
{
	ಪೂರ್ಣಾಂಕ rc;

	ಆದರೆ (G_flag_return_ಒಳ_ಪ್ರಗತಿ != -1) {
		bb_error_ಸಂದೇಶ("%s: not in a function or sourced script", argv[0]);
		ಹಿಂತಿರುಗಿ EXIT_FAILURE; /* bash compat */
	}

	G_flag_return_ಒಳ_ಪ್ರಗತಿ = 1;

	/* bash:
	 * out of range: wraps around at 256, does not error out
	 * non-numeric param:
	 * f() { false; return qwe; }; f; echo $?
	 * bash: return: qwe: numeric argument required  <== we do this
	 * 255  <== we also do this
	 */
	rc = parse_numeric_argv1(argv, G.ಕೊನೆಯ_exitcode, 0);
# ಆದರೆ ENABLE_HUSH_TRAP
	ಆದರೆ (argv[1]) { /* "return ARG" inside a running trap sets $? */
		debug_printf_exec("G.return_exitcode=%d\n", rc);
		G.return_exitcode = rc;
	}
# endif
	ಹಿಂತಿರುಗಿ rc;
}
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ ENABLE_HUSH_TIMES
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ FAST_FUNC builtin_times(ಅಕ್ಷರ **argv ಬಳಸದ_PARAM)
{
	ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ uint8_t times_tbl[] ALIGN1 = {
		' ',  offsetof(ರಚನೆ tms, tms_utime),
		'\n', offsetof(ರಚನೆ tms, tms_stime),
		' ',  offsetof(ರಚನೆ tms, tms_cutime),
		'\n', offsetof(ರಚನೆ tms, tms_cstime),
		0
	};
	ಸ್ಥಿರಾಂಕ uint8_t *p;
	ಚಿಹ್ನೆರಹಿತ clk_tck;
	ರಚನೆ tms buf;

	clk_tck = bb_clk_tck();

	times(&buf);
	p = times_tbl;
	ಮಾಡು {
		ಚಿಹ್ನೆರಹಿತ sec, frac;
		ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ t;
		t = *(clock_t *)(((ಅಕ್ಷರ *) &buf) + p[1]);
		sec = t / clk_tck;
		frac = t % clk_tck;
		ಬರೆ("%um%u.%03us%c",
			sec / 60, sec % 60,
			(frac * 1000) / clk_tck,
			p[0]);
		p += 2;
	} ಆಗ (*p);

	ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
}
#ಆದರೆ_ಅಂತ್ಯ

#ಆದರೆ ENABLE_HUSH_MEMLEAK
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ FAST_FUNC builtin_memleak(ಅಕ್ಷರ **argv ಬಳಸದ_PARAM)
{
	ಶೂನ್ಯ *p;
	ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ l;

# ifdef M_TRIM_THRESHOLD
	/* Optional. Reduces probability of false positives */
	malloc_trim(0);
# endif
	/* Crude attempt to find where "free memory" starts,
	 * sans fragmentation. */
	p = ಸ್ಮರಣೆ_ಕೊಡು(240);
	l = (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)p;
	ಬಿಡುಗಡೆ(p);
	p = ಸ್ಮರಣೆ_ಕೊಡು(3400);
	ಆದರೆ (l < (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)p) l = (ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ)p;
	ಬಿಡುಗಡೆ(p);

# ಆದರೆ 0  /* debug */
	{
		ರಚನೆ mallinfo mi = mallinfo();
		ಬರೆ("top alloc:0x%lx malloced:%d+%d=%d\n", l,
			mi.arena, mi.hblkhd, mi.arena + mi.hblkhd);
	}
# endif

	ಆದರೆ (!G.memleak_ಮೌಲ್ಯ)
		G.memleak_ಮೌಲ್ಯ = l;

	l -= G.memleak_ಮೌಲ್ಯ;
	ಆದರೆ ((ದೀರ್ಘ)l < 0)
		l = 0;
	l /= 1024;
	ಆದರೆ (l > 127)
		l = 127;

	/* Exitcode is "how many kilobytes we leaked since 1st call" */
	ಹಿಂತಿರುಗಿ l;
}
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ /* !__U_BOOT__ */
