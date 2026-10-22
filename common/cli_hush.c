// SPDX-License-Identifier: GPL-2.0+
/*
 * sh.c -- a prototype Bourne shell grammar parser
 *      Intended to follow the original Thompson and Ritchie
 *      "small and simple is beautiful" philosophy, which
 *      incidentally is a good match to today's BusyBox.
 *
 * Copyright (C) 2000,2001  Larry Doolittle  <larry@doolittle.boa.org>
 *
 * Credits:
 *      The parser routines proper are all original material, first
 *      written Dec 2000 and Jan 2001 by Larry Doolittle.
 *      The execution engine, the builtins, and much of the underlying
 *      support has been adapted from busybox-0.49pre's lash,
 *      which is Copyright (C) 2000 by Lineo, Inc., and
 *      written by Erik Andersen <andersen@lineo.com>, <andersee@debian.org>.
 *      That, in turn, is based in part on ladsh.c, by Michael K. Johnson and
 *      Erik W. Troan, which they placed in the public domain.  I don't know
 *      how much of the Johnson/Troan code has survived the repeated rewrites.
 * Other credits:
 *      b_addchr() derived from similar w_addchar function in glibc-2.2
 *      setup_redirect(), redirect_opt_num(), and big chunks of main()
 *        and many builtins derived from contributions by Erik Andersen
 *      miscellaneous bugfixes from Matt Kraai
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
 * Bash grammar not implemented: (how many of these were in original sh?)
 *      $@ (those sure look like weird quoting rules)
 *      $_
 *      ! negation operator for pipes
 *      &> and >& redirection of stdout+stderr
 *      Brace Expansion
 *      Tilde Expansion
 *      fancy forms of Parameter Expansion
 *      aliases
 *      Arithmetic Expansion
 *      <(list) and >(list) Process Substitution
 *      reserved words: case, esac, select, function
 *      Here Documents ( << word )
 *      Functions
 * Major bugs:
 *      job handling woefully incomplete and buggy
 *      reserved word execution woefully incomplete and buggy
 * to-do:
 *      port selected bugfixes from post-0.49 busybox lash - done?
 *      finish implementing reserved words: for, while, until, do, done
 *      change { and } from special chars to reserved words
 *      builtins: break, continue, eval, return, set, trap, ulimit
 *      test magic exec
 *      handle children going into background
 *      clean up recognition of null pipes
 *      check setting of global_argc and global_argv
 *      control-C handling, probably with longjmp
 *      follow IFS rules more precisely, including update semantics
 *      figure out what to do with backslash-newline
 *      explain why we use signal instead of sigaction
 *      propagate syntax errors, die on resource errors?
 *      continuation lines, both explicit and implicit - done?
 *      memory leak finding and plugging - done?
 *      more testing, especially quoting rules and redirection
 *      document how quoting rules not precisely followed for variable assignments
 *      maybe change map[] to use 2-bit entries
 *      (eventually) remove all the printf's
 */

#define __U_BOOT__
#ifdef __U_BOOT__
#include <env.h>
#include <malloc.h>         /* malloc, free, realloc*/
#include <linux/ctype.h>    /* isalpha, isdigit */
#include <console.h>
#include <bootretry.h>
#include <cli.h>
#include <cli_hush.h>
#include <command.h>        /* find_cmd */
#include <vsprintf.h>
#endif
#ifndef __U_BOOT__
#include <ctype.h>     /* isalpha, isdigit */
#include <unistd.h>    /* getpid */
#include <stdlib.h>    /* getenv, atoi */
#include <string.h>    /* strchr */
#include <stdio.h>     /* popen etc. */
#include <glob.h>      /* glob, of course */
#include <stdarg.h>    /* va_list */
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>    /* should be pretty obvious */

#include <sys/stat.h>  /* ulimit */
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

/* #include <dmalloc.h> */

#ប្រសិនបើ 1
#include "busybox.h"
#include "cmdedit.h"
#បើមិនដូច្នេះ
#define applet_name "hush"
#include "standalone.h"
#define hush_main main
#define BB_BANNER
#endif
#endif
#define SPECIAL_VAR_SYMBOL 03
#define SUBSTED_VAR_SYMBOL 04
#ifndef __U_BOOT__
#define FLAG_EXIT_FROM_LOOP 1
#define FLAG_PARSE_SEMICOLON (1 << 1)		/* symbol ';' is special for parser */
#define FLAG_REPARSING       (1 << 2)		/* >= 2nd pass */

#endif

#ifdef __U_BOOT__
#define EXIT_SUCCESS 0
#define EOF -1
#define syntax() syntax_err()
#define xstrdup strdup
#define error_msg printf
#បើមិនដូច្នេះ
typedef ga_nanaa {
	REDIRECT_INPUT     = 1,
	REDIRECT_OVERWRITE = 2,
	REDIRECT_APPEND    = 3,
	REDIRECT_HEREIS    = 4,
	REDIRECT_IO        = 5
} redir_type;

/* The descrip member of this structure is only used to make debugging
 * output pretty */
sa_nracanaa {ចំនួនគត់ mode; ចំនួនគត់ default_fd; var_na *descrip;} redir_table[] = {
	{ 0,                         0, "()" },
	{ O_RDONLY,                  0, "<"  },
	{ O_CREAT|O_TRUNC|O_WRONLY,  1, ">"  },
	{ O_CREAT|O_APPEND|O_WRONLY, 1, ">>" },
	{ O_RDONLY,                 -1, "<<" },
	{ O_RDWR,                    1, "<>" }
};
#endif

typedef ga_nanaa {
	PIPE_SEQ = 1,
	PIPE_AND = 2,
	PIPE_OR  = 3,
	PIPE_BG  = 4,
} pipe_style;

/* might eventually control execution */
typedef ga_nanaa {
	RES_NONE  = 0,
	RES_IF    = 1,
	RES_THEN  = 2,
	RES_ELIF  = 3,
	RES_ELSE  = 4,
	RES_FI    = 5,
	RES_FOR   = 6,
	RES_WHILE = 7,
	RES_UNTIL = 8,
	RES_DO    = 9,
	RES_DONE  = 10,
	RES_XXXX  = 11,
	RES_IN    = 12,
	RES_SNTX  = 13
} reserved_style;
#define FLAG_END   (1<<RES_NONE)
#define FLAG_IF    (1<<RES_IF)
#define FLAG_THEN  (1<<RES_THEN)
#define FLAG_ELIF  (1<<RES_ELIF)
#define FLAG_ELSE  (1<<RES_ELSE)
#define FLAG_FI    (1<<RES_FI)
#define FLAG_FOR   (1<<RES_FOR)
#define FLAG_WHILE (1<<RES_WHILE)
#define FLAG_UNTIL (1<<RES_UNTIL)
#define FLAG_DO    (1<<RES_DO)
#define FLAG_DONE  (1<<RES_DONE)
#define FLAG_IN    (1<<RES_IN)
#define FLAG_START (1<<RES_XXXX)

/* This holds pointers to the various results of parsing */
sa_nracanaa p_context {
	sa_nracanaa child_prog *child;
	sa_nracanaa pipe *list_head;
	sa_nracanaa pipe *pipe;
#ifndef __U_BOOT__
	sa_nracanaa redir_struct *pending_redirect;
#endif
	reserved_style w;
	ចំនួនគត់ old_flag;				/* for figuring out valid reserved words */
	sa_nracanaa p_context *stack;
	ចំនួនគត់ type;			/* define type of parser : ";$" common or special symbol */
	/* How about quoting status? */
};

#ifndef __U_BOOT__
sa_nracanaa redir_struct {
	redir_type type;			/* type of redirection */
	ចំនួនគត់ fd;						/* file descriptor being redirected */
	ចំនួនគត់ dup;					/* -1, or file descriptor being duplicated */
	sa_nracanaa redir_struct *next;	/* pointer to the next redirect in the list */
	glob_t word;				/* *word.gl_pathv is the filename */
};
#endif

sa_nracanaa child_prog {
#ifndef __U_BOOT__
	pid_t pid;					/* 0 if exited */
#endif
	var_na **argv;				/* program name and arguments */
	/* was quoted when parsed; copy of struct o_string.nonnull field */
	ចំនួនគត់ *argv_nonnull;
#ifdef __U_BOOT__
	ចំនួនគត់    argc;                            /* number of program arguments */
#endif
	sa_nracanaa pipe *group;			/* if non-NULL, first in group or subshell */
#ifndef __U_BOOT__
	ចំនួនគត់ subshell;				/* flag, non-zero if group must be forked */
	sa_nracanaa redir_struct *redirects;	/* I/O redirections */
	glob_t glob_result;			/* result of parameter globbing */
	ចំនួនគត់ is_stopped;				/* is the program currently running? */
	sa_nracanaa pipe *family;		/* pointer back to the child's parent pipe */
#endif
	ចំនួនគត់ sp;				/* number of SPECIAL_VAR_SYMBOL */
	ចំនួនគត់ type;
};

sa_nracanaa pipe {
#ifndef __U_BOOT__
	ចំនួនគត់ jobid;					/* job number */
#endif
	ចំនួនគត់ num_progs;				/* total number of programs in job */
#ifndef __U_BOOT__
	ចំនួនគត់ running_progs;			/* number of programs running */
	var_na *text;					/* name of job */
	var_na *cmdbuf;				/* buffer various argv's point into */
	pid_t pgrp;					/* process group ID for the job */
#endif
	sa_nracanaa child_prog *progs;	/* array of commands in pipe */
	sa_nracanaa pipe *next;			/* to track background commands */
#ifndef __U_BOOT__
	ចំនួនគត់ stopped_progs;			/* number of programs alive, but stopped */
	ចំនួនគត់ job_context;			/* bitmask defining current context */
#endif
	pipe_style followup;		/* PIPE_BG, PIPE_SEQ, PIPE_OR, PIPE_AND */
	reserved_style r_mode;		/* supports if, for, while, until */
};

#ifndef __U_BOOT__
sa_nracanaa close_me {
	ចំនួនគត់ fd;
	sa_nracanaa close_me *next;
};
#endif

sa_nracanaa variables {
	var_na *name;
	var_na *value;
	ចំនួនគត់ flg_export;
	ចំនួនគត់ flg_read_only;
	sa_nracanaa variables *next;
};

/* globals, connect us to the outside world
 * the first three support $?, $#, and $1 */
#ifndef __U_BOOT__
var_na **global_argv;
unsigned ចំនួនគត់ global_argc;
#endif
static unsigned ចំនួនគត់ last_return_code;
#ifndef __U_BOOT__
extern var_na **environ; /* This is in <unistd.h>, but protected with __USE_GNU */
#endif

/* "globals" within this file */
static uchar *ifs;
static var_na map[256];
#ifndef __U_BOOT__
static ចំនួនគត់ fake_mode;
static ចំនួនគត់ interactive;
static sa_nracanaa close_me *close_me_head;
static const var_na *cwd;
static sa_nracanaa pipe *job_list;
static unsigned ចំនួនគត់ last_bg_pid;
static unsigned ចំនួនគត់ last_jobid;
static unsigned ចំនួនគត់ shell_terminal;
static var_na *PS1;
static var_na *PS2;
sa_nracanaa variables shell_ver = { "HUSH_VERSION", "0.01", 1, 1, 0 };
sa_nracanaa variables *top_vars = &shell_ver;
#បើមិនដូច្នេះ
static ចំនួនគត់ flag_repeat = 0;
static ចំនួនគត់ do_repeat = 0;
static sa_nracanaa variables *top_vars = NULL ;
#endif /*__U_BOOT__ */

#define B_CHUNK (100)
#define B_NOSPAC 1

typedef sa_nracanaa {
	var_na *data;
	ចំនួនគត់ length;
	ចំនួនគត់ maxlen;
	ចំនួនគត់ quote;
	ចំនួនគត់ nonnull;
} o_string;
#define NULL_O_STRING {NULL,0,0,0,0}
/* used for initialization:
	o_string foo = NULL_O_STRING; */

/* I can almost use ordinary FILE *.  Is open_memstream() universally
 * available?  Where is it documented? */
sa_nracanaa in_str {
	const unsigned var_na *p;
#ifndef __U_BOOT__
	var_na peek_buf[2];
#endif
	ចំនួនគត់ __promptme;
	ចំនួនគត់ promptmode;
#ifndef __U_BOOT__
	FILE *file;
#endif
	ចំនួនគត់ (*get) (sa_nracanaa in_str *);
	ចំនួនគត់ (*peek) (sa_nracanaa in_str *);
};
#define b_getch(input) ((input)->get(input))
#define b_peek(input) ((input)->peek(input))

#ifndef __U_BOOT__
#define JOB_STATUS_FORMAT "[%d] %-22s %.40s\n"

sa_nracanaa built_in_command {
	var_na *cmd;					/* name */
	var_na *descr;				/* description */
	ចំនួនគត់ (*function) (sa_nracanaa child_prog *);	/* function ptr */
};
#endif

/* define DEBUG_SHELL for debugging output (obviously ;-)) */
#ប្រសិនបើ 0
#define DEBUG_SHELL
#endif

/* This should be in utility.c */
#ifdef DEBUG_SHELL
#ifndef __U_BOOT__
static void debug_printf(const var_na *format, ...)
{
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
}
#បើមិនដូច្នេះ
#define debug_printf(fmt,args...)	printf (fmt ,##args)
#endif
#បើមិនដូច្នេះ
static anta_hstha void debug_printf(const var_na *format, ...) { }
#endif
#define final_printf debug_printf

#ifdef __U_BOOT__
static void syntax_err(void) {
	 printf("syntax error\n");
}
#បើមិនដូច្នេះ
static void __syntax(var_na *file, ចំនួនគត់ line) {
	error_msg("syntax error %s:%d", file, line);
}
#define syntax() __syntax(__FILE__, __LINE__)
#endif

#ifdef __U_BOOT__
static void *xmalloc(size_t size);
static void *xrealloc(void *ptr, size_t size);
#បើមិនដូច្នេះ
/* Index of subroutines: */
/*   function prototypes for builtins */
static ចំនួនគត់ builtin_cd(sa_nracanaa child_prog *child);
static ចំនួនគត់ builtin_env(sa_nracanaa child_prog *child);
static ចំនួនគត់ builtin_eval(sa_nracanaa child_prog *child);
static ចំនួនគត់ builtin_exec(sa_nracanaa child_prog *child);
static ចំនួនគត់ builtin_exit(sa_nracanaa child_prog *child);
static ចំនួនគត់ builtin_export(sa_nracanaa child_prog *child);
static ចំនួនគត់ builtin_fg_bg(sa_nracanaa child_prog *child);
static ចំនួនគត់ builtin_help(sa_nracanaa child_prog *child);
static ចំនួនគត់ builtin_jobs(sa_nracanaa child_prog *child);
static ចំនួនគត់ builtin_pwd(sa_nracanaa child_prog *child);
static ចំនួនគត់ builtin_read(sa_nracanaa child_prog *child);
static ចំនួនគត់ builtin_set(sa_nracanaa child_prog *child);
static ចំនួនគត់ builtin_shift(sa_nracanaa child_prog *child);
static ចំនួនគត់ builtin_source(sa_nracanaa child_prog *child);
static ចំនួនគត់ builtin_umask(sa_nracanaa child_prog *child);
static ចំនួនគត់ builtin_unset(sa_nracanaa child_prog *child);
static ចំនួនគត់ builtin_not_written(sa_nracanaa child_prog *child);
#endif
/*   o_string manipulation: */
static ចំនួនគត់ b_check_space(o_string *o, ចំនួនគត់ len);
static ចំនួនគត់ b_addchr(o_string *o, ចំនួនគត់ ch);
static void b_reset(o_string *o);
static ចំនួនគត់ b_addqchr(o_string *o, ចំនួនគត់ ch, ចំនួនគត់ quote);
#ifndef __U_BOOT__
static ចំនួនគត់ b_adduint(o_string *o, unsigned ចំនួនគត់ i);
#endif
/*  in_str manipulations: */
static ចំនួនគត់ static_get(sa_nracanaa in_str *i);
static ចំនួនគត់ static_peek(sa_nracanaa in_str *i);
static ចំនួនគត់ file_get(sa_nracanaa in_str *i);
static ចំនួនគត់ file_peek(sa_nracanaa in_str *i);
#ifndef __U_BOOT__
static void setup_file_in_str(sa_nracanaa in_str *i, FILE *f);
#បើមិនដូច្នេះ
static void setup_file_in_str(sa_nracanaa in_str *i);
#endif
static void setup_string_in_str(sa_nracanaa in_str *i, const var_na *s);
#ifndef __U_BOOT__
/*  close_me manipulations: */
static void mark_open(ចំនួនគត់ fd);
static void mark_closed(ចំនួនគត់ fd);
static void close_all(void);
#endif
/*  "run" the final data structures: */
static var_na *indenter(ចំនួនគត់ i);
static ចំនួនគត់ free_pipe_list(sa_nracanaa pipe *head, ចំនួនគត់ indent);
static ចំនួនគត់ free_pipe(sa_nracanaa pipe *pi, ចំនួនគត់ indent);
/*  really run the final data structures: */
#ifndef __U_BOOT__
static ចំនួនគត់ setup_redirects(sa_nracanaa child_prog *prog, ចំនួនគត់ squirrel[]);
#endif
static ចំនួនគត់ run_list_real(sa_nracanaa pipe *pi);
#ifndef __U_BOOT__
static void pseudo_exec(sa_nracanaa child_prog *child) __attribute__ ((noreturn));
#endif
static ចំនួនគត់ run_pipe_real(sa_nracanaa pipe *pi);
/*   extended glob support: */
#ifndef __U_BOOT__
static ចំនួនគត់ globhack(const var_na *src, ចំនួនគត់ flags, glob_t *pglob);
static ចំនួនគត់ glob_needed(const var_na *s);
static ចំនួនគត់ xglob(o_string *dest, ចំនួនគត់ flags, glob_t *pglob);
#endif
/*   variable assignment: */
static ចំនួនគត់ is_assignment(const var_na *s);
/*   data structure manipulation: */
#ifndef __U_BOOT__
static ចំនួនគត់ setup_redirect(sa_nracanaa p_context *ctx, ចំនួនគត់ fd, redir_type style, sa_nracanaa in_str *input);
#endif
static void initialize_context(sa_nracanaa p_context *ctx);
static ចំនួនគត់ done_word(o_string *dest, sa_nracanaa p_context *ctx);
static ចំនួនគត់ done_command(sa_nracanaa p_context *ctx);
static ចំនួនគត់ done_pipe(sa_nracanaa p_context *ctx, pipe_style type);
/*   primary string parsing: */
#ifndef __U_BOOT__
static ចំនួនគត់ redirect_dup_num(sa_nracanaa in_str *input);
static ចំនួនគត់ redirect_opt_num(o_string *o);
static ចំនួនគត់ process_command_subs(o_string *dest, sa_nracanaa p_context *ctx, sa_nracanaa in_str *input, ចំនួនគត់ subst_end);
static ចំនួនគត់ parse_group(o_string *dest, sa_nracanaa p_context *ctx, sa_nracanaa in_str *input, ចំនួនគត់ ch);
#endif
static var_na *lookup_param(var_na *src);
static var_na *make_string(var_na **inp, ចំនួនគត់ *nonnull);
static ចំនួនគត់ handle_dollar(o_string *dest, sa_nracanaa p_context *ctx, sa_nracanaa in_str *input);
#ifndef __U_BOOT__
static ចំនួនគត់ parse_string(o_string *dest, sa_nracanaa p_context *ctx, const var_na *src);
#endif
static ចំនួនគត់ parse_stream(o_string *dest, sa_nracanaa p_context *ctx, sa_nracanaa in_str *input0, ចំនួនគត់ end_trigger);
/*   setup: */
static ចំនួនគត់ parse_stream_outer(sa_nracanaa in_str *inp, ចំនួនគត់ flag);
#ifndef __U_BOOT__
static ចំនួនគត់ parse_string_outer(const var_na *s, ចំនួនគត់ flag);
static ចំនួនគត់ parse_file_outer(FILE *f);
#endif
#ifndef __U_BOOT__
/*   job management: */
static ចំនួនគត់ checkjobs(sa_nracanaa pipe* fg_pipe);
static void insert_bg_job(sa_nracanaa pipe *pi);
static void remove_bg_job(sa_nracanaa pipe *pi);
#endif
/*     local variable support */
static var_na **make_list_in(var_na **inp, var_na *name);
static var_na *insert_var_value(var_na *inp);
static var_na *insert_var_value_sub(var_na *inp, ចំនួនគត់ tag_subst);

#ifndef __U_BOOT__
/* Table of built-in functions.  They can be forked or not, depending on
 * context: within pipes, they fork.  As simple commands, they do not.
 * When used in non-forking context, they can change global variables
 * in the parent shell process.  If forked, of course they can not.
 * For example, 'unset foo | whatever' will parse and run, but foo will
 * still be set at the end. */
static sa_nracanaa built_in_command bltins[] = {
	{"bg", "Resume a job in the background", builtin_fg_bg},
	{"break", "Exit for, while or until loop", builtin_not_written},
	{"cd", "Change working directory", builtin_cd},
	{"continue", "Continue for, while or until loop", builtin_not_written},
	{"env", "Print all environment variables", builtin_env},
	{"eval", "Construct and run shell command", builtin_eval},
	{"exec", "Exec command, replacing this shell with the exec'd process",
		builtin_exec},
	{"exit", "Exit from shell()", builtin_exit},
	{"export", "Set environment variable", builtin_export},
	{"fg", "Bring job into the foreground", builtin_fg_bg},
	{"jobs", "Lists the active jobs", builtin_jobs},
	{"pwd", "Print current directory", builtin_pwd},
	{"read", "Input environment variable", builtin_read},
	{"return", "Return from a function", builtin_not_written},
	{"set", "Set/unset shell local variables", builtin_set},
	{"shift", "Shift positional parameters", builtin_shift},
	{"trap", "Trap signals", builtin_not_written},
	{"ulimit","Controls resource limits", builtin_not_written},
	{"umask","Sets file creation mask", builtin_umask},
	{"unset", "Unset environment variable", builtin_unset},
	{".", "Source-in and run commands in a file", builtin_source},
	{"help", "List shell built-in commands", builtin_help},
	{NULL, NULL, NULL}
};

static const var_na *set_cwd(void)
{
	ប្រសិនបើ(cwd==unknown)
		cwd = NULL;     /* xgetcwd(arg) called free(arg) */
	cwd = xgetcwd((var_na *)cwd);
	ប្រសិនបើ (!cwd)
		cwd = unknown;
	ត្រឡប់ cwd;
}

/* built-in 'eval' handler */
static ចំនួនគត់ builtin_eval(sa_nracanaa child_prog *child)
{
	var_na *str = NULL;
	ចំនួនគត់ rcode = EXIT_SUCCESS;

	ប្រសិនបើ (child->argv[1]) {
		str = make_string(child->argv + 1);
		parse_string_outer(str, FLAG_EXIT_FROM_LOOP |
					FLAG_PARSE_SEMICOLON);
		free(str);
		rcode = last_return_code;
	}
	ត្រឡប់ rcode;
}

/* built-in 'cd <path>' handler */
static ចំនួនគត់ builtin_cd(sa_nracanaa child_prog *child)
{
	var_na *newdir;
	ប្រសិនបើ (child->argv[1] == NULL)
		newdir = env_get("HOME");
	បើមិនដូច្នេះ
		newdir = child->argv[1];
	ប្រសិនបើ (chdir(newdir)) {
		printf("cd: %s: %s\n", newdir, strerror(errno));
		ត្រឡប់ EXIT_FAILURE;
	}
	set_cwd();
	ត្រឡប់ EXIT_SUCCESS;
}

/* built-in 'env' handler */
static ចំនួនគត់ builtin_env(sa_nracanaa child_prog *dummy)
{
	var_na **e = environ;
	ប្រសិនបើ (e == NULL) ត្រឡប់ EXIT_FAILURE;
	សម្រាប់ (; *e; e++) {
		puts(*e);
	}
	ត្រឡប់ EXIT_SUCCESS;
}

/* built-in 'exec' handler */
static ចំនួនគត់ builtin_exec(sa_nracanaa child_prog *child)
{
	ប្រសិនបើ (child->argv[1] == NULL)
		ត្រឡប់ EXIT_SUCCESS;   /* Really? */
	child->argv++;
	pseudo_exec(child);
	/* never returns */
}

/* built-in 'exit' handler */
static ចំនួនគត់ builtin_exit(sa_nracanaa child_prog *child)
{
	ប្រសិនបើ (child->argv[1] == NULL)
		exit(last_return_code);
	exit (atoi(child->argv[1]));
}

/* built-in 'export VAR=value' handler */
static ចំនួនគត់ builtin_export(sa_nracanaa child_prog *child)
{
	ចំនួនគត់ res = 0;
	var_na *name = child->argv[1];

	ប្រសិនបើ (name == NULL) {
		ត្រឡប់ (builtin_env(child));
	}

	name = strdup(name);

	ប្រសិនបើ(name) {
		var_na *value = strchr(name, '=');

		ប្រសិនបើ (!value) {
			var_na *tmp;
			/* They are exporting something without an =VALUE */

			value = get_local_var(name);
			ប្រសិនបើ (value) {
				size_t ln = strlen(name);

				tmp = realloc(name, ln+strlen(value)+2);
				ប្រសិនបើ(tmp==NULL)
					res = -1;
				បើមិនដូច្នេះ {
					sprintf(tmp+ln, "=%s", value);
					name = tmp;
				}
			} បើមិនដូច្នេះ {
				/* bash does not return an error when trying to export
				 * an undefined variable.  Do likewise. */
				res = 1;
			}
		}
	}
	ប្រសិនបើ (res<0)
		perror_msg("export");
	បើមិនដូច្នេះ ប្រសិនបើ(res==0)
		res = set_local_var(name, 1);
	បើមិនដូច្នេះ
		res = 0;
	free(name);
	ត្រឡប់ res;
}

/* built-in 'fg' and 'bg' handler */
static ចំនួនគត់ builtin_fg_bg(sa_nracanaa child_prog *child)
{
	ចំនួនគត់ i, jobnum;
	sa_nracanaa pipe *pi=NULL;

	ប្រសិនបើ (!interactive)
		ត្រឡប់ EXIT_FAILURE;
	/* If they gave us no args, assume they want the last backgrounded task */
	ប្រសិនបើ (!child->argv[1]) {
		សម្រាប់ (pi = job_list; pi; pi = pi->next) {
			ប្រសិនបើ (pi->jobid == last_jobid) {
				bha_nga;
			}
		}
		ប្រសិនបើ (!pi) {
			error_msg("%s: no current job", child->argv[0]);
			ត្រឡប់ EXIT_FAILURE;
		}
	} បើមិនដូច្នេះ {
		ប្រសិនបើ (sscanf(child->argv[1], "%%%d", &jobnum) != 1) {
			error_msg("%s: bad argument '%s'", child->argv[0], child->argv[1]);
			ត្រឡប់ EXIT_FAILURE;
		}
		សម្រាប់ (pi = job_list; pi; pi = pi->next) {
			ប្រសិនបើ (pi->jobid == jobnum) {
				bha_nga;
			}
		}
		ប្រសិនបើ (!pi) {
			error_msg("%s: %d: no such job", child->argv[0], jobnum);
			ត្រឡប់ EXIT_FAILURE;
		}
	}

	ប្រសិនបើ (*child->argv[0] == 'f') {
		/* Put the job into the foreground.  */
		tcsetpgrp(shell_terminal, pi->pgrp);
	}

	/* Restart the processes in the job */
	សម្រាប់ (i = 0; i < pi->num_progs; i++)
		pi->progs[i].is_stopped = 0;

	ប្រសិនបើ ( (i=kill(- pi->pgrp, SIGCONT)) < 0) {
		ប្រសិនបើ (i == ESRCH) {
			remove_bg_job(pi);
		} បើមិនដូច្នេះ {
			perror_msg("kill (SIGCONT)");
		}
	}

	pi->stopped_progs = 0;
	ត្រឡប់ EXIT_SUCCESS;
}

/* built-in 'help' handler */
static ចំនួនគត់ builtin_help(sa_nracanaa child_prog *dummy)
{
	sa_nracanaa built_in_command *x;

	printf("\nBuilt-in commands:\n");
	printf("-------------------\n");
	សម្រាប់ (x = bltins; x->cmd; x++) {
		ប្រសិនបើ (x->descr==NULL)
			continue;
		printf("%s\t%s\n", x->cmd, x->descr);
	}
	printf("\n\n");
	ត្រឡប់ EXIT_SUCCESS;
}

/* built-in 'jobs' handler */
static ចំនួនគត់ builtin_jobs(sa_nracanaa child_prog *child)
{
	sa_nracanaa pipe *job;
	var_na *status_string;

	សម្រាប់ (job = job_list; job; job = job->next) {
		ប្រសិនបើ (job->running_progs == job->stopped_progs)
			status_string = "Stopped";
		បើមិនដូច្នេះ
			status_string = "Running";

		printf(JOB_STATUS_FORMAT, job->jobid, status_string, job->text);
	}
	ត្រឡប់ EXIT_SUCCESS;
}

/* built-in 'pwd' handler */
static ចំនួនគត់ builtin_pwd(sa_nracanaa child_prog *dummy)
{
	puts(set_cwd());
	ត្រឡប់ EXIT_SUCCESS;
}

/* built-in 'read VAR' handler */
static ចំនួនគត់ builtin_read(sa_nracanaa child_prog *child)
{
	ចំនួនគត់ res;

	ប្រសិនបើ (child->argv[1]) {
		var_na string[BUFSIZ];
		var_na *var = 0;

		string[0] = 0;  /* In case stdin has only EOF */
		/* read string */
		fgets(string, parimaa_na(string), stdin);
		chomp(string);
		var = malloc(strlen(child->argv[1])+strlen(string)+2);
		ប្រសិនបើ(var) {
			sprintf(var, "%s=%s", child->argv[1], string);
			res = set_local_var(var, 0);
		} បើមិនដូច្នេះ
			res = -1;
		ប្រសិនបើ (res)
			fprintf(stderr, "read: %m\n");
		free(var);      /* So not move up to avoid breaking errno */
		ត្រឡប់ res;
	} បើមិនដូច្នេះ {
		do res=getchar(); ខណៈពេល(res!='\n' && res!=EOF);
		ត្រឡប់ 0;
	}
}

/* built-in 'set VAR=value' handler */
static ចំនួនគត់ builtin_set(sa_nracanaa child_prog *child)
{
	var_na *temp = child->argv[1];
	sa_nracanaa variables *e;

	ប្រសិនបើ (temp == NULL)
		សម្រាប់(e = top_vars; e; e=e->next)
			printf("%s=%s\n", e->name, e->value);
	បើមិនដូច្នេះ
		set_local_var(temp, 0);

		ត្រឡប់ EXIT_SUCCESS;
}

/* Built-in 'shift' handler */
static ចំនួនគត់ builtin_shift(sa_nracanaa child_prog *child)
{
	ចំនួនគត់ n=1;
	ប្រសិនបើ (child->argv[1]) {
		n=atoi(child->argv[1]);
	}
	ប្រសិនបើ (n>=0 && n<global_argc) {
		/* XXX This probably breaks $0 */
		global_argc -= n;
		global_argv += n;
		ត្រឡប់ EXIT_SUCCESS;
	} បើមិនដូច្នេះ {
		ត្រឡប់ EXIT_FAILURE;
	}
}

/* Built-in '.' handler (read-in and execute commands from file) */
static ចំនួនគត់ builtin_source(sa_nracanaa child_prog *child)
{
	FILE *input;
	ចំនួនគត់ status;

	ប្រសិនបើ (child->argv[1] == NULL)
		ត្រឡប់ EXIT_FAILURE;

	/* XXX search through $PATH is missing */
	input = fopen(child->argv[1], "r");
	ប្រសិនបើ (!input) {
		error_msg("Couldn't open file '%s'", child->argv[1]);
		ត្រឡប់ EXIT_FAILURE;
	}

	/* Now run the file */
	/* XXX argv and argc are broken; need to save old global_argv
	 * (pointer only is OK!) on this stack frame,
	 * set global_argv=child->argv+1, recurse, and restore. */
	mark_open(fileno(input));
	status = parse_file_outer(input);
	mark_closed(fileno(input));
	fclose(input);
	ត្រឡប់ (status);
}

static ចំនួនគត់ builtin_umask(sa_nracanaa child_prog *child)
{
	mode_t new_umask;
	const var_na *arg = child->argv[1];
	var_na *end;
	ប្រសិនបើ (arg) {
		new_umask=strtoul(arg, &end, 8);
		ប្រសិនបើ (*end!='\0' || end == arg) {
			ត្រឡប់ EXIT_FAILURE;
		}
	} បើមិនដូច្នេះ {
		printf("%.3o\n", (unsigned ចំនួនគត់) (new_umask=umask(0)));
	}
	umask(new_umask);
	ត្រឡប់ EXIT_SUCCESS;
}

/* built-in 'unset VAR' handler */
static ចំនួនគត់ builtin_unset(sa_nracanaa child_prog *child)
{
	/* bash returned already true */
	unset_local_var(child->argv[1]);
	ត្រឡប់ EXIT_SUCCESS;
}

static ចំនួនគត់ builtin_not_written(sa_nracanaa child_prog *child)
{
	printf("builtin_%s not written\n",child->argv[0]);
	ត្រឡប់ EXIT_FAILURE;
}
#endif

static ចំនួនគត់ b_check_space(o_string *o, ចំនួនគត់ len)
{
	/* It would be easy to drop a more restrictive policy
	 * in here, such as setting a maximum string length */
	ប្រសិនបើ (o->length + len > o->maxlen) {
		var_na *old_data = o->data;
		/* assert (data == NULL || o->maxlen != 0); */
		o->maxlen += max(2*len, B_CHUNK);
		o->data = realloc(o->data, 1 + o->maxlen);
		ប្រសិនបើ (o->data == NULL) {
			free(old_data);
		}
	}
	ត្រឡប់ o->data == NULL;
}

static ចំនួនគត់ b_addchr(o_string *o, ចំនួនគត់ ch)
{
	debug_printf("b_addchr: %c %d %p\n", ch, o->length, o);
	ប្រសិនបើ (b_check_space(o, 1)) ត្រឡប់ B_NOSPAC;
	o->data[o->length] = ch;
	o->length++;
	o->data[o->length] = '\0';
	ត្រឡប់ 0;
}

static void b_reset(o_string *o)
{
	o->length = 0;
	o->nonnull = 0;
	ប្រសិនបើ (o->data != NULL) *o->data = '\0';
}

static void b_free(o_string *o)
{
	b_reset(o);
	free(o->data);
	o->data = NULL;
	o->maxlen = 0;
}

/* My analysis of quoting semantics tells me that state information
 * is associated with a destination, not a source.
 */
static ចំនួនគត់ b_addqchr(o_string *o, ចំនួនគត់ ch, ចំនួនគត់ quote)
{
	ប្រសិនបើ (quote && strchr("*?[\\",ch)) {
		ចំនួនគត់ rc;
		rc = b_addchr(o, '\\');
		ប្រសិនបើ (rc) ត្រឡប់ rc;
	}
	ត្រឡប់ b_addchr(o, ch);
}

#ifndef __U_BOOT__
static ចំនួនគត់ b_adduint(o_string *o, unsigned ចំនួនគត់ i)
{
	ចំនួនគត់ r;
	var_na *p = simple_itoa(i);
	/* no escape checking necessary */
	do r=b_addchr(o, *p++); ខណៈពេល (r==0 && *p);
	ត្រឡប់ r;
}
#endif

static ចំនួនគត់ static_get(sa_nracanaa in_str *i)
{
	ចំនួនគត់ ch = *i->p++;
	ប្រសិនបើ (ch=='\0') ត្រឡប់ EOF;
	ត្រឡប់ ch;
}

static ចំនួនគត់ static_peek(sa_nracanaa in_str *i)
{
	ត្រឡប់ *i->p;
}

#ifndef __U_BOOT__
static anta_hstha void cmdedit_set_initial_prompt(void)
{
#ifndef CONFIG_FEATURE_SH_FANCY_PROMPT
	PS1 = NULL;
#បើមិនដូច្នេះ
	PS1 = env_get("PS1");
	ប្រសិនបើ(PS1==0)
		PS1 = "\\w \\$ ";
#endif
}

static anta_hstha void setup_prompt_string(ចំនួនគត់ promptmode, var_na **prompt_str)
{
	debug_printf("setup_prompt_string %d ",promptmode);
#ifndef CONFIG_FEATURE_SH_FANCY_PROMPT
	/* Set up the prompt */
	ប្រសិនបើ (promptmode == 1) {
		free(PS1);
		PS1=xmalloc(strlen(cwd)+4);
		sprintf(PS1, "%s %s", cwd, ( geteuid() != 0 ) ?  "$ ":"# ");
		*prompt_str = PS1;
	} បើមិនដូច្នេះ {
		*prompt_str = PS2;
	}
#បើមិនដូច្នេះ
	*prompt_str = (promptmode==1)? PS1 : PS2;
#endif
	debug_printf("result %s\n",*prompt_str);
}
#endif

#ifdef __U_BOOT__
static ចំនួនគត់ uboot_cli_readline(sa_nracanaa in_str *i)
{
	var_na *prompt;
	var_na __maybe_unused *ps_prompt = NULL;

	ប្រសិនបើ (i->promptmode == 1)
		prompt = CONFIG_SYS_PROMPT;
	បើមិនដូច្នេះ
		prompt = CONFIG_SYS_PROMPT_HUSH_PS2;

#ifdef CONFIG_CMDLINE_PS_SUPPORT
	ប្រសិនបើ (i->promptmode == 1)
		ps_prompt = env_get("PS1");
	បើមិនដូច្នេះ
		ps_prompt = env_get("PS2");
	ប្រសិនបើ (ps_prompt)
		prompt = ps_prompt;
#endif

	ត្រឡប់ cli_readline(prompt);
}
#endif

static void get_user_input(sa_nracanaa in_str *i)
{
#ifndef __U_BOOT__
	var_na *prompt_str;
	static var_na the_command[BUFSIZ];

	setup_prompt_string(i->promptmode, &prompt_str);
#ifdef CONFIG_FEATURE_COMMAND_EDITING
	/*
	 ** enable command line editing only while a command line
	 ** is actually being read; otherwise, we'll end up bequeathing
	 ** atexit() handlers and other unwanted stuff to our
	 ** child processes (rob@sysgo.de)
	 */
	cmdedit_read_input(prompt_str, the_command);
#បើមិនដូច្នេះ
	fputs(prompt_str, stdout);
	fflush(stdout);
	the_command[0]=fgetc(i->file);
	the_command[1]='\0';
#endif
	fflush(stdout);
	i->p = the_command;
#បើមិនដូច្នេះ
	ចំនួនគត់ n;
	static var_na the_command[CONFIG_SYS_CBSIZE + 1];

	bootretry_reset_cmd_timeout();
	i->__promptme = 1;
	n = uboot_cli_readline(i);

#ifdef CONFIG_BOOT_RETRY_TIME
	ប្រសិនបើ (n == -2) {
	  puts("\nTimeout waiting for command\n");
#  ifdef CONFIG_RESET_TO_RETRY
	  do_reset(NULL, 0, 0, NULL);
#  elif IS_ENABLED(CONFIG_RETRY_BOOTCMD)
	strcpy(console_buffer, "run bootcmd\n");
# បើមិនដូច្នេះ
#	error "This only works with CONFIG_RESET_TO_RETRY or CONFIG_BOOT_RETRY_COMMAND enabled"
#  endif
	}
#endif
	ប្រសិនបើ (n == -1 ) {
		flag_repeat = 0;
		i->__promptme = 0;
	}
	n = strlen(console_buffer);
	console_buffer[n] = '\n';
	console_buffer[n+1]= '\0';
	ប្រសិនបើ (had_ctrlc()) flag_repeat = 0;
	clear_ctrlc();
	do_repeat = 0;
	ប្រសិនបើ (i->promptmode == 1) {
		ប្រសិនបើ (console_buffer[0] == '\n'&& flag_repeat == 0) {
			strcpy(the_command,console_buffer);
		}
		បើមិនដូច្នេះ {
			ប្រសិនបើ (console_buffer[0] != '\n') {
				strcpy(the_command,console_buffer);
				flag_repeat = 1;
			}
			បើមិនដូច្នេះ {
				do_repeat = 1;
			}
		}
		i->p = the_command;
	}
	បើមិនដូច្នេះ {
		ប្រសិនបើ (console_buffer[0] != '\n') {
			ប្រសិនបើ (strlen(the_command) + strlen(console_buffer)
			    < CONFIG_SYS_CBSIZE) {
				n = strlen(the_command);
				the_command[n-1] = ' ';
				strcpy(&the_command[n],console_buffer);
			}
			បើមិនដូច្នេះ {
				the_command[0] = '\n';
				the_command[1] = '\0';
				flag_repeat = 0;
			}
		}
		ប្រសិនបើ (i->__promptme == 0) {
			the_command[0] = '\n';
			the_command[1] = '\0';
		}
		i->p = console_buffer;
	}
#endif
}

/* This is the magic location that prints prompts
 * and gets data back from the user */
static ចំនួនគត់ file_get(sa_nracanaa in_str *i)
{
	ចំនួនគត់ ch;

	ch = 0;
	/* If there is data waiting, eat it up */
	ប្រសិនបើ (i->p && *i->p) {
		ch = *i->p++;
	} បើមិនដូច្នេះ {
		/* need to double check i->file because we might be doing something
		 * more complicated by now, like sourcing or substituting. */
#ifndef __U_BOOT__
		ប្រសិនបើ (i->__promptme && interactive && i->file == stdin) {
			ខណៈពេល(! i->p || (interactive && strlen(i->p)==0) ) {
#បើមិនដូច្នេះ
			ខណៈពេល(! i->p  || strlen(i->p)==0 ) {
#endif
				get_user_input(i);
			}
			i->promptmode=2;
#ifndef __U_BOOT__
			i->__promptme = 0;
#endif
			ប្រសិនបើ (i->p && *i->p) {
				ch = *i->p++;
			}
#ifndef __U_BOOT__
		} បើមិនដូច្នេះ {
			ch = fgetc(i->file);
		}

#endif
		debug_printf("b_getch: got a %d\n", ch);
	}
#ifndef __U_BOOT__
	ប្រសិនបើ (ch == '\n') i->__promptme=1;
#endif
	ត្រឡប់ ch;
}

/* All the callers guarantee this routine will never be
 * used right after a newline, so prompting is not needed.
 */
static ចំនួនគត់ file_peek(sa_nracanaa in_str *i)
{
#ifndef __U_BOOT__
	ប្រសិនបើ (i->p && *i->p) {
#endif
		ត្រឡប់ *i->p;
#ifndef __U_BOOT__
	} បើមិនដូច្នេះ {
		i->peek_buf[0] = fgetc(i->file);
		i->peek_buf[1] = '\0';
		i->p = i->peek_buf;
		debug_printf("b_peek: got a %d\n", *i->p);
		ត្រឡប់ *i->p;
	}
#endif
}

#ifndef __U_BOOT__
static void setup_file_in_str(sa_nracanaa in_str *i, FILE *f)
#បើមិនដូច្នេះ
static void setup_file_in_str(sa_nracanaa in_str *i)
#endif
{
	i->peek = file_peek;
	i->get = file_get;
	i->__promptme=1;
	i->promptmode=1;
#ifndef __U_BOOT__
	i->file = f;
#endif
	i->p = NULL;
}

static void setup_string_in_str(sa_nracanaa in_str *i, const var_na *s)
{
	i->peek = static_peek;
	i->get = static_get;
	i->__promptme=1;
	i->promptmode=1;
	i->p = s;
}

#ifndef __U_BOOT__
static void mark_open(ចំនួនគត់ fd)
{
	sa_nracanaa close_me *new = xmalloc(parimaa_na(sa_nracanaa close_me));
	new->fd = fd;
	new->next = close_me_head;
	close_me_head = new;
}

static void mark_closed(ចំនួនគត់ fd)
{
	sa_nracanaa close_me *tmp;
	ប្រសិនបើ (close_me_head == NULL || close_me_head->fd != fd)
		error_msg_and_die("corrupt close_me");
	tmp = close_me_head;
	close_me_head = close_me_head->next;
	free(tmp);
}

static void close_all(void)
{
	sa_nracanaa close_me *c;
	សម្រាប់ (c=close_me_head; c; c=c->next) {
		close(c->fd);
	}
	close_me_head = NULL;
}

/* squirrel != NULL means we squirrel away copies of stdin, stdout,
 * and stderr if they are redirected. */
static ចំនួនគត់ setup_redirects(sa_nracanaa child_prog *prog, ចំនួនគត់ squirrel[])
{
	ចំនួនគត់ openfd, mode;
	sa_nracanaa redir_struct *redir;

	សម្រាប់ (redir=prog->redirects; redir; redir=redir->next) {
		ប្រសិនបើ (redir->dup == -1 && redir->word.gl_pathv == NULL) {
			/* something went wrong in the parse.  Pretend it didn't happen */
			continue;
		}
		ប្រសិនបើ (redir->dup == -1) {
			mode=redir_table[redir->type].mode;
			openfd = open(redir->word.gl_pathv[0], mode, 0666);
			ប្រសិនបើ (openfd < 0) {
			/* this could get lost if stderr has been redirected, but
			   bash and ash both lose it as well (though zsh doesn't!) */
				perror_msg("error opening %s", redir->word.gl_pathv[0]);
				ត្រឡប់ 1;
			}
		} បើមិនដូច្នេះ {
			openfd = redir->dup;
		}

		ប្រសិនបើ (openfd != redir->fd) {
			ប្រសិនបើ (squirrel && redir->fd < 3) {
				squirrel[redir->fd] = dup(redir->fd);
			}
			ប្រសិនបើ (openfd == -3) {
				close(openfd);
			} បើមិនដូច្នេះ {
				dup2(openfd, redir->fd);
				ប្រសិនបើ (redir->dup == -1)
					close (openfd);
			}
		}
	}
	ត្រឡប់ 0;
}

static void restore_redirects(ចំនួនគត់ squirrel[])
{
	ចំនួនគត់ i, fd;
	សម្រាប់ (i=0; i<3; i++) {
		fd = squirrel[i];
		ប្រសិនបើ (fd != -1) {
			/* No error checking.  I sure wouldn't know what
			 * to do with an error if I found one! */
			dup2(fd, i);
			close(fd);
		}
	}
}

/* never returns */
/* XXX no exit() here.  If you don't exec, use _exit instead.
 * The at_exit handlers apparently confuse the calling process,
 * in particular stdin handling.  Not sure why? */
static void pseudo_exec(sa_nracanaa child_prog *child)
{
	ចំនួនគត់ i, rcode;
	var_na *p;
	sa_nracanaa built_in_command *x;
	ប្រសិនបើ (child->argv) {
		សម្រាប់ (i=0; is_assignment(child->argv[i]); i++) {
			debug_printf("pid %d environment modification: %s\n",getpid(),child->argv[i]);
			p = insert_var_value(child->argv[i]);
			putenv(strdup(p));
			ប្រសិនបើ (p != child->argv[i]) free(p);
		}
		child->argv+=i;  /* XXX this hack isn't so horrible, since we are about
					to exit, and therefore don't need to keep data
					structures consistent for free() use. */
		/* If a variable is assigned in a forest, and nobody listens,
		 * was it ever really set?
		 */
		ប្រសិនបើ (child->argv[0] == NULL) {
			_exit(EXIT_SUCCESS);
		}

		/*
		 * Check if the command matches any of the builtins.
		 * Depending on context, this might be redundant.  But it's
		 * easier to waste a few CPU cycles than it is to figure out
		 * if this is one of those cases.
		 */
		សម្រាប់ (x = bltins; x->cmd; x++) {
			ប្រសិនបើ (strcmp(child->argv[0], x->cmd) == 0 ) {
				debug_printf("builtin exec %s\n", child->argv[0]);
				rcode = x->function(child);
				fflush(stdout);
				_exit(rcode);
			}
		}

		/* Check if the command matches any busybox internal commands
		 * ("applets") here.
		 * FIXME: This feature is not 100% safe, since
		 * BusyBox is not fully reentrant, so we have no guarantee the things
		 * from the .bss are still zeroed, or that things from .data are still
		 * at their defaults.  We could exec ourself from /proc/self/exe, but I
		 * really dislike relying on /proc for things.  We could exec ourself
		 * from global_argv[0], but if we are in a chroot, we may not be able
		 * to find ourself... */
#ifdef CONFIG_FEATURE_SH_STANDALONE_SHELL
		{
			ចំនួនគត់ argc_l;
			var_na** argv_l=child->argv;
			var_na *name = child->argv[0];

#ifdef CONFIG_FEATURE_SH_APPLETS_ALWAYS_WIN
			/* Following discussions from November 2000 on the busybox mailing
			 * list, the default configuration, (without
			 * get_last_path_component()) lets the user force use of an
			 * external command by specifying the full (with slashes) filename.
			 * If you enable CONFIG_FEATURE_SH_APPLETS_ALWAYS_WIN then applets
			 * _aways_ override external commands, so if you want to run
			 * /bin/cat, it will use BusyBox cat even if /bin/cat exists on the
			 * filesystem and is _not_ busybox.  Some systems may want this,
			 * most do not.  */
			name = get_last_path_component(name);
#endif
			/* Count argc for use in a second... */
			សម្រាប់(argc_l=0;*argv_l!=NULL; argv_l++, argc_l++);
			optind = 1;
			debug_printf("running applet %s\n", name);
			run_applet_by_name(name, argc_l, child->argv);
		}
#endif
		debug_printf("exec of %s\n",child->argv[0]);
		execvp(child->argv[0],child->argv);
		perror_msg("couldn't exec: %s",child->argv[0]);
		_exit(1);
	} បើមិនដូច្នេះ ប្រសិនបើ (child->group) {
		debug_printf("runtime nesting to group\n");
		interactive=0;    /* crucial!!!! */
		rcode = run_list_real(child->group);
		/* OK to leak memory by not calling free_pipe_list,
		 * since this process is about to exit */
		_exit(rcode);
	} បើមិនដូច្នេះ {
		/* Can happen.  See what bash does with ">foo" by itself. */
		debug_printf("trying to pseudo_exec null command\n");
		_exit(EXIT_SUCCESS);
	}
}

static void insert_bg_job(sa_nracanaa pipe *pi)
{
	sa_nracanaa pipe *thejob;

	/* Linear search for the ID of the job to use */
	pi->jobid = 1;
	សម្រាប់ (thejob = job_list; thejob; thejob = thejob->next)
		ប្រសិនបើ (thejob->jobid >= pi->jobid)
			pi->jobid = thejob->jobid + 1;

	/* add thejob to the list of running jobs */
	ប្រសិនបើ (!job_list) {
		thejob = job_list = xmalloc(parimaa_na(*thejob));
	} បើមិនដូច្នេះ {
		សម្រាប់ (thejob = job_list; thejob->next; thejob = thejob->next) /* nothing */;
		thejob->next = xmalloc(parimaa_na(*thejob));
		thejob = thejob->next;
	}

	/* physically copy the struct job */
	memcpy(thejob, pi, parimaa_na(sa_nracanaa pipe));
	thejob->next = NULL;
	thejob->running_progs = thejob->num_progs;
	thejob->stopped_progs = 0;
	thejob->text = xmalloc(BUFSIZ); /* cmdedit buffer size */

	/*if (pi->progs[0] && pi->progs[0].argv && pi->progs[0].argv[0]) */
	{
		var_na *bar=thejob->text;
		var_na **foo=pi->progs[0].argv;
		ខណៈពេល(foo && *foo) {
			bar += sprintf(bar, "%s ", *foo++);
		}
	}

	/* we don't wait for background thejobs to return -- append it
	   to the list of backgrounded thejobs and leave it alone */
	printf("[%d] %d\n", thejob->jobid, thejob->progs[0].pid);
	last_bg_pid = thejob->progs[0].pid;
	last_jobid = thejob->jobid;
}

/* remove a backgrounded job */
static void remove_bg_job(sa_nracanaa pipe *pi)
{
	sa_nracanaa pipe *prev_pipe;

	ប្រសិនបើ (pi == job_list) {
		job_list = pi->next;
	} បើមិនដូច្នេះ {
		prev_pipe = job_list;
		ខណៈពេល (prev_pipe->next != pi)
			prev_pipe = prev_pipe->next;
		prev_pipe->next = pi->next;
	}
	ប្រសិនបើ (job_list)
		last_jobid = job_list->jobid;
	បើមិនដូច្នេះ
		last_jobid = 0;

	pi->stopped_progs = 0;
	free_pipe(pi, 0);
	free(pi);
}

/* Checks to see if any processes have exited -- if they
   have, figure out why and see if a job has completed */
static ចំនួនគត់ checkjobs(sa_nracanaa pipe* fg_pipe)
{
	ចំនួនគត់ attributes;
	ចំនួនគត់ status;
	ចំនួនគត់ prognum = 0;
	sa_nracanaa pipe *pi;
	pid_t childpid;

	attributes = WUNTRACED;
	ប្រសិនបើ (fg_pipe==NULL) {
		attributes |= WNOHANG;
	}

	ខណៈពេល ((childpid = waitpid(-1, &status, attributes)) > 0) {
		ប្រសិនបើ (fg_pipe) {
			ចំនួនគត់ i, rcode = 0;
			សម្រាប់ (i=0; i < fg_pipe->num_progs; i++) {
				ប្រសិនបើ (fg_pipe->progs[i].pid == childpid) {
					ប្រសិនបើ (i==fg_pipe->num_progs-1)
						rcode=WEXITSTATUS(status);
					(fg_pipe->num_progs)--;
					ត្រឡប់(rcode);
				}
			}
		}

		សម្រាប់ (pi = job_list; pi; pi = pi->next) {
			prognum = 0;
			ខណៈពេល (prognum < pi->num_progs && pi->progs[prognum].pid != childpid) {
				prognum++;
			}
			ប្រសិនបើ (prognum < pi->num_progs)
				bha_nga;
		}

		ប្រសិនបើ(pi==NULL) {
			debug_printf("checkjobs: pid %d was not in our list!\n", childpid);
			continue;
		}

		ប្រសិនបើ (WIFEXITED(status) || WIFSIGNALED(status)) {
			/* child exited */
			pi->running_progs--;
			pi->progs[prognum].pid = 0;

			ប្រសិនបើ (!pi->running_progs) {
				printf(JOB_STATUS_FORMAT, pi->jobid, "Done", pi->text);
				remove_bg_job(pi);
			}
		} បើមិនដូច្នេះ {
			/* child stopped */
			pi->stopped_progs++;
			pi->progs[prognum].is_stopped = 1;

#ប្រសិនបើ 0
			/* Printing this stuff is a pain, since it tends to
			 * overwrite the prompt an inconveinient moments.  So
			 * don't do that.  */
			ប្រសិនបើ (pi->stopped_progs == pi->num_progs) {
				printf("\n"JOB_STATUS_FORMAT, pi->jobid, "Stopped", pi->text);
			}
#endif
		}
	}

	ប្រសិនបើ (childpid == -1 && errno != ECHILD)
		perror_msg("waitpid");

	/* move the shell to the foreground */
	/*if (interactive && tcsetpgrp(shell_terminal, getpgid(0))) */
	/*	perror_msg("tcsetpgrp-2"); */
	ត្រឡប់ -1;
}

/* Figure out our controlling tty, checking in order stderr,
 * stdin, and stdout.  If check_pgrp is set, also check that
 * we belong to the foreground process group associated with
 * that tty.  The value of shell_terminal is needed in order to call
 * tcsetpgrp(shell_terminal, ...); */
void controlling_tty(ចំនួនគត់ check_pgrp)
{
	pid_t curpgrp;

	ប្រសិនបើ ((curpgrp = tcgetpgrp(shell_terminal = 2)) < 0
			&& (curpgrp = tcgetpgrp(shell_terminal = 0)) < 0
			&& (curpgrp = tcgetpgrp(shell_terminal = 1)) < 0)
		goto shell_terminal_error;

	ប្រសិនបើ (check_pgrp && curpgrp != getpgid(0))
		goto shell_terminal_error;

	ត្រឡប់;

shell_terminal_error:
		shell_terminal = -1;
		ត្រឡប់;
}
#endif

/* run_pipe_real() starts all the jobs, but doesn't wait for anything
 * to finish.  See checkjobs().
 *
 * return code is normally -1, when the caller has to wait for children
 * to finish to determine the exit status of the pipe.  If the pipe
 * is a simple builtin command, however, the action is done by the
 * time run_pipe_real returns, and the exit code is provided as the
 * return value.
 *
 * The input of the pipe is always stdin, the output is always
 * stdout.  The outpipe[] mechanism in BusyBox-0.48 lash is bogus,
 * because it tries to avoid running the command substitution in
 * subshell, when that is in fact necessary.  The subshell process
 * now has its stdout directed to the input of the appropriate pipe,
 * so this routine is noticeably simpler.
 */
static ចំនួនគត់ run_pipe_real(sa_nracanaa pipe *pi)
{
	ចំនួនគត់ i;
#ifndef __U_BOOT__
	ចំនួនគត់ nextin, nextout;
	ចំនួនគត់ pipefds[2];				/* pipefds[0] is for reading */
	sa_nracanaa child_prog *child;
	sa_nracanaa built_in_command *x;
	var_na *p;
# ប្រសិនបើ __GNUC__
	/* Avoid longjmp clobbering */
	(void) &i;
	(void) &nextin;
	(void) &nextout;
	(void) &child;
# endif
#បើមិនដូច្នេះ
	ចំនួនគត់ nextin;
	ចំនួនគត់ flag = do_repeat ? CMD_FLAG_REPEAT : 0;
	sa_nracanaa child_prog *child;
	var_na *p;
# ប្រសិនបើ __GNUC__
	/* Avoid longjmp clobbering */
	(void) &i;
	(void) &nextin;
	(void) &child;
# endif
#endif	/* __U_BOOT__ */

	nextin = 0;
#ifndef __U_BOOT__
	pi->pgrp = -1;
#endif

	/* Check if this is a simple builtin (not part of a pipe).
	 * Builtins within pipes have to fork anyway, and are handled in
	 * pseudo_exec.  "echo foo | read bar" doesn't work on bash, either.
	 */
	ប្រសិនបើ (pi->num_progs == 1) child = & (pi->progs[0]);
#ifndef __U_BOOT__
	ប្រសិនបើ (pi->num_progs == 1 && child->group && child->subshell == 0) {
		ចំនួនគត់ squirrel[] = {-1, -1, -1};
		ចំនួនគត់ rcode;
		debug_printf("non-subshell grouping\n");
		setup_redirects(child, squirrel);
		/* XXX could we merge code with following builtin case,
		 * by creating a pseudo builtin that calls run_list_real? */
		rcode = run_list_real(child->group);
		restore_redirects(squirrel);
#បើមិនដូច្នេះ
		ប្រសិនបើ (pi->num_progs == 1 && child->group) {
		ចំនួនគត់ rcode;
		debug_printf("non-subshell grouping\n");
		rcode = run_list_real(child->group);
#endif
		ត្រឡប់ rcode;
	} បើមិនដូច្នេះ ប្រសិនបើ (pi->num_progs == 1 && pi->progs[0].argv != NULL) {
		សម្រាប់ (i=0; is_assignment(child->argv[i]); i++) { /* nothing */ }
		ប្រសិនបើ (i!=0 && child->argv[i]==NULL) {
			/* assignments, but no command: set the local environment */
			សម្រាប់ (i=0; child->argv[i]!=NULL; i++) {

				/* Ok, this case is tricky.  We have to decide if this is a
				 * local variable, or an already exported variable.  If it is
				 * already exported, we have to export the new value.  If it is
				 * not exported, we need only set this as a local variable.
				 * This junk is all to decide whether or not to export this
				 * variable. */
				ចំនួនគត់ export_me=0;
				var_na *name, *value;
				name = xstrdup(child->argv[i]);
				debug_printf("Local environment set: %s\n", name);
				value = strchr(name, '=');
				ប្រសិនបើ (value)
					*value=0;
#ifndef __U_BOOT__
				ប្រសិនបើ ( get_local_var(name)) {
					export_me=1;
				}
#endif
				free(name);
				p = insert_var_value(child->argv[i]);
				set_local_var(p, export_me);
				ប្រសិនបើ (p != child->argv[i]) free(p);
			}
			ត្រឡប់ EXIT_SUCCESS;   /* don't worry about errors in set_local_var() yet */
		}
		សម្រាប់ (i = 0; is_assignment(child->argv[i]); i++) {
			p = insert_var_value(child->argv[i]);
#ifndef __U_BOOT__
			putenv(strdup(p));
#បើមិនដូច្នេះ
			set_local_var(p, 0);
#endif
			ប្រសិនបើ (p != child->argv[i]) {
				child->sp--;
				free(p);
			}
		}
		ប្រសិនបើ (child->sp) {
			var_na * str = NULL;

			str = make_string(child->argv + i,
					  child->argv_nonnull + i);
			parse_string_outer(str, FLAG_EXIT_FROM_LOOP | FLAG_REPARSING);
			free(str);
			ត្រឡប់ last_return_code;
		}
#ifndef __U_BOOT__
		សម្រាប់ (x = bltins; x->cmd; x++) {
			ប្រសិនបើ (strcmp(child->argv[i], x->cmd) == 0 ) {
				ចំនួនគត់ squirrel[] = {-1, -1, -1};
				ចំនួនគត់ rcode;
				ប្រសិនបើ (x->function == builtin_exec && child->argv[i+1]==NULL) {
					debug_printf("magic exec\n");
					setup_redirects(child,NULL);
					ត្រឡប់ EXIT_SUCCESS;
				}
				debug_printf("builtin inline %s\n", child->argv[0]);
				/* XXX setup_redirects acts on file descriptors, not FILEs.
				 * This is perfect for work that comes after exec().
				 * Is it really safe for inline use?  Experimentally,
				 * things seem to work with glibc. */
				setup_redirects(child, squirrel);

				child->argv += i;  /* XXX horrible hack */
				rcode = x->function(child);
				/* XXX restore hack so free() can work right */
				child->argv -= i;
				restore_redirects(squirrel);
			}
			ត្រឡប់ rcode;
		}
#បើមិនដូច្នេះ
		/* check ";", because ,example , argv consist from
		 * "help;flinfo" must not execute
		 */
		ប្រសិនបើ (strchr(child->argv[i], ';')) {
			printf("Unknown command '%s' - try 'help' or use "
					"'run' command\n", child->argv[i]);
			ត្រឡប់ -1;
		}
		/* Process the command */
		ត្រឡប់ cmd_process(flag, child->argc - i, child->argv + i,
				   &flag_repeat, NULL);
#endif
	}
#ifndef __U_BOOT__

	សម្រាប់ (i = 0; i < pi->num_progs; i++) {
		child = & (pi->progs[i]);

		/* pipes are inserted between pairs of commands */
		ប្រសិនបើ ((i + 1) < pi->num_progs) {
			ប្រសិនបើ (pipe(pipefds)<0) perror_msg_and_die("pipe");
			nextout = pipefds[1];
		} បើមិនដូច្នេះ {
			nextout=1;
			pipefds[0] = -1;
		}

		/* XXX test for failed fork()? */
		ប្រសិនបើ (!(child->pid = fork())) {
			/* Set the handling for job control signals back to the default.  */
			sa_nkeeta(SIGINT, SIG_DFL);
			sa_nkeeta(SIGQUIT, SIG_DFL);
			sa_nkeeta(SIGTERM, SIG_DFL);
			sa_nkeeta(SIGTSTP, SIG_DFL);
			sa_nkeeta(SIGTTIN, SIG_DFL);
			sa_nkeeta(SIGTTOU, SIG_DFL);
			sa_nkeeta(SIGCHLD, SIG_DFL);

			close_all();

			ប្រសិនបើ (nextin != 0) {
				dup2(nextin, 0);
				close(nextin);
			}
			ប្រសិនបើ (nextout != 1) {
				dup2(nextout, 1);
				close(nextout);
			}
			ប្រសិនបើ (pipefds[0]!=-1) {
				close(pipefds[0]);  /* opposite end of our output pipe */
			}

			/* Like bash, explicit redirects override pipes,
			 * and the pipe fd is available for dup'ing. */
			setup_redirects(child,NULL);

			ប្រសិនបើ (interactive && pi->followup!=PIPE_BG) {
				/* If we (the child) win the race, put ourselves in the process
				 * group whose leader is the first process in this pipe. */
				ប្រសិនបើ (pi->pgrp < 0) {
					pi->pgrp = getpid();
				}
				ប្រសិនបើ (setpgid(0, pi->pgrp) == 0) {
					tcsetpgrp(2, pi->pgrp);
				}
			}

			pseudo_exec(child);
		}

		/* put our child in the process group whose leader is the
		   first process in this pipe */
		ប្រសិនបើ (pi->pgrp < 0) {
			pi->pgrp = child->pid;
		}
		/* Don't check for errors.  The child may be dead already,
		 * in which case setpgid returns error code EACCES. */
		setpgid(child->pid, pi->pgrp);

		ប្រសិនបើ (nextin != 0)
			close(nextin);
		ប្រសិនបើ (nextout != 1)
			close(nextout);

		/* If there isn't another process, nextin is garbage
		   but it doesn't matter */
		nextin = pipefds[0];
	}
#endif
	ត្រឡប់ -1;
}

static ចំនួនគត់ run_list_real(sa_nracanaa pipe *pi)
{
	var_na *save_name = NULL;
	var_na **list = NULL;
	var_na **save_list = NULL;
	sa_nracanaa pipe *rpipe;
	ចំនួនគត់ flag_rep = 0;
#ifndef __U_BOOT__
	ចំនួនគត់ save_num_progs;
#endif
	ចំនួនគត់ rcode=0, flag_skip=1;
	ចំនួនគត់ flag_restore = 0;
	ចំនួនគត់ if_code=0, next_if_code=0;  /* need double-buffer to handle elif */
	reserved_style rmode, skip_more_in_this_rmode=RES_XXXX;
	/* check syntax for "for" */
	សម្រាប់ (rpipe = pi; rpipe; rpipe = rpipe->next) {
		ប្រសិនបើ ((rpipe->r_mode == RES_IN ||
		    rpipe->r_mode == RES_FOR) &&
		    (rpipe->next == NULL)) {
				syntax();
#ifdef __U_BOOT__
				flag_repeat = 0;
#endif
				ត្រឡប់ 1;
		}
		ប្រសិនបើ ((rpipe->r_mode == RES_IN &&
			(rpipe->next->r_mode == RES_IN &&
			rpipe->next->progs->argv != NULL))||
			(rpipe->r_mode == RES_FOR &&
			rpipe->next->r_mode != RES_IN)) {
				syntax();
#ifdef __U_BOOT__
				flag_repeat = 0;
#endif
				ត្រឡប់ 1;
		}
	}
	សម្រាប់ (; pi; pi = (flag_restore != 0) ? rpipe : pi->next) {
		ប្រសិនបើ (pi->r_mode == RES_WHILE || pi->r_mode == RES_UNTIL ||
			pi->r_mode == RES_FOR) {
#ifdef __U_BOOT__
				/* check Ctrl-C */
				ctrlc();
				ប្រសិនបើ ((had_ctrlc())) {
					ត្រឡប់ 1;
				}
#endif
				flag_restore = 0;
				ប្រសិនបើ (!rpipe) {
					flag_rep = 0;
					rpipe = pi;
				}
		}
		rmode = pi->r_mode;
		debug_printf("rmode=%d  if_code=%d  next_if_code=%d skip_more=%d\n", rmode, if_code, next_if_code, skip_more_in_this_rmode);
		ប្រសិនបើ (rmode == skip_more_in_this_rmode && flag_skip) {
			ប្រសិនបើ (pi->followup == PIPE_SEQ) flag_skip=0;
			continue;
		}
		flag_skip = 1;
		skip_more_in_this_rmode = RES_XXXX;
		ប្រសិនបើ (rmode == RES_THEN || rmode == RES_ELSE) if_code = next_if_code;
		ប្រសិនបើ (rmode == RES_THEN &&  if_code) continue;
		ប្រសិនបើ (rmode == RES_ELSE && !if_code) continue;
		ប្រសិនបើ (rmode == RES_ELIF && !if_code) bha_nga;
		ប្រសិនបើ (rmode == RES_FOR && pi->num_progs) {
			ប្រសិនបើ (!list) {
				/* if no variable values after "in" we skip "for" */
				ប្រសិនបើ (!pi->next->progs->argv) continue;
				/* create list of variable values */
				list = make_list_in(pi->next->progs->argv,
					pi->progs->argv[0]);
				save_list = list;
				save_name = pi->progs->argv[0];
				pi->progs->argv[0] = NULL;
				flag_rep = 1;
			}
			ប្រសិនបើ (!(*list)) {
				free(pi->progs->argv[0]);
				free(save_list);
				list = NULL;
				flag_rep = 0;
				pi->progs->argv[0] = save_name;
#ifndef __U_BOOT__
				pi->progs->glob_result.gl_pathv[0] =
					pi->progs->argv[0];
#endif
				continue;
			} បើមិនដូច្នេះ {
				/* insert new value from list for variable */
				free(pi->progs->argv[0]);
				pi->progs->argv[0] = *list++;
#ifndef __U_BOOT__
				pi->progs->glob_result.gl_pathv[0] =
					pi->progs->argv[0];
#endif
			}
		}
		ប្រសិនបើ (rmode == RES_IN) continue;
		ប្រសិនបើ (rmode == RES_DO) {
			ប្រសិនបើ (!flag_rep) continue;
		}
		ប្រសិនបើ (rmode == RES_DONE) {
			ប្រសិនបើ (flag_rep) {
				flag_restore = 1;
			} បើមិនដូច្នេះ {
				rpipe = NULL;
			}
		}
		ប្រសិនបើ (pi->num_progs == 0) continue;
#ifndef __U_BOOT__
		save_num_progs = pi->num_progs; /* save number of programs */
#endif
		rcode = run_pipe_real(pi);
		debug_printf("run_pipe_real returned %d\n",rcode);
#ifndef __U_BOOT__
		ប្រសិនបើ (rcode!=-1) {
			/* We only ran a builtin: rcode was set by the return value
			 * of run_pipe_real(), and we don't need to wait for anything. */
		} បើមិនដូច្នេះ ប្រសិនបើ (pi->followup==PIPE_BG) {
			/* XXX check bash's behavior with nontrivial pipes */
			/* XXX compute jobid */
			/* XXX what does bash do with attempts to background builtins? */
			insert_bg_job(pi);
			rcode = EXIT_SUCCESS;
		} បើមិនដូច្នេះ {
			ប្រសិនបើ (interactive) {
				/* move the new process group into the foreground */
				ប្រសិនបើ (tcsetpgrp(shell_terminal, pi->pgrp) && errno != ENOTTY)
					perror_msg("tcsetpgrp-3");
				rcode = checkjobs(pi);
				/* move the shell to the foreground */
				ប្រសិនបើ (tcsetpgrp(shell_terminal, getpgid(0)) && errno != ENOTTY)
					perror_msg("tcsetpgrp-4");
			} បើមិនដូច្នេះ {
				rcode = checkjobs(pi);
			}
			debug_printf("checkjobs returned %d\n",rcode);
		}
		last_return_code=rcode;
#បើមិនដូច្នេះ
		ប្រសិនបើ (rcode < -1) {
			last_return_code = -rcode - 2;
			ត្រឡប់ -2;	/* exit */
		}
		last_return_code = rcode;
#endif
#ifndef __U_BOOT__
		pi->num_progs = save_num_progs; /* restore number of programs */
#endif
		ប្រសិនបើ ( rmode == RES_IF || rmode == RES_ELIF )
			next_if_code=rcode;  /* can be overwritten a number of times */
		ប្រសិនបើ (rmode == RES_WHILE)
			flag_rep = !last_return_code;
		ប្រសិនបើ (rmode == RES_UNTIL)
			flag_rep = last_return_code;
		ប្រសិនបើ ( (rcode==EXIT_SUCCESS && pi->followup==PIPE_OR) ||
		     (rcode!=EXIT_SUCCESS && pi->followup==PIPE_AND) )
			skip_more_in_this_rmode=rmode;
#ifndef __U_BOOT__
		checkjobs(NULL);
#endif
	}
	ត្រឡប់ rcode;
}

/* broken, of course, but OK for testing */
static var_na *indenter(ចំនួនគត់ i)
{
	static var_na blanks[]="                                    ";
	ត្រឡប់ &blanks[parimaa_na(blanks)-i-1];
}

/* return code is the exit status of the pipe */
static ចំនួនគត់ free_pipe(sa_nracanaa pipe *pi, ចំនួនគត់ indent)
{
	var_na **p;
	sa_nracanaa child_prog *child;
#ifndef __U_BOOT__
	sa_nracanaa redir_struct *r, *rnext;
#endif
	ចំនួនគត់ a, i, ret_code=0;
	var_na *ind = indenter(indent);

#ifndef __U_BOOT__
	ប្រសិនបើ (pi->stopped_progs > 0)
		ត្រឡប់ ret_code;
	final_printf("%s run pipe: (pid %d)\n",ind,getpid());
#endif
	សម្រាប់ (i=0; i<pi->num_progs; i++) {
		child = &pi->progs[i];
		final_printf("%s  command %d:\n",ind,i);
		ប្រសិនបើ (child->argv) {
			សម្រាប់ (a=0,p=child->argv; *p; a++,p++) {
				final_printf("%s   argv[%d] = %s\n",ind,a,*p);
			}
#ifndef __U_BOOT__
			globfree(&child->glob_result);
#បើមិនដូច្នេះ
			សម្រាប់ (a = 0; a < child->argc; a++) {
				free(child->argv[a]);
			}
			free(child->argv);
			free(child->argv_nonnull);
			child->argc = 0;
#endif
			child->argv=NULL;
		} បើមិនដូច្នេះ ប្រសិនបើ (child->group) {
#ifndef __U_BOOT__
			final_printf("%s   begin group (subshell:%d)\n",ind, child->subshell);
#endif
			ret_code = free_pipe_list(child->group,indent+3);
			final_printf("%s   end group\n",ind);
		} បើមិនដូច្នេះ {
			final_printf("%s   (nil)\n",ind);
		}
#ifndef __U_BOOT__
		សម្រាប់ (r=child->redirects; r; r=rnext) {
			final_printf("%s   redirect %d%s", ind, r->fd, redir_table[r->type].descrip);
			ប្រសិនបើ (r->dup == -1) {
				/* guard against the case >$FOO, where foo is unset or blank */
				ប្រសិនបើ (r->word.gl_pathv) {
					final_printf(" %s\n", *r->word.gl_pathv);
					globfree(&r->word);
				}
			} បើមិនដូច្នេះ {
				final_printf("&%d\n", r->dup);
			}
			rnext=r->next;
			free(r);
		}
		child->redirects=NULL;
#endif
	}
	free(pi->progs);   /* children are an array, they get freed all at once */
	pi->progs=NULL;
	ត្រឡប់ ret_code;
}

static ចំនួនគត់ free_pipe_list(sa_nracanaa pipe *head, ចំនួនគត់ indent)
{
	ចំនួនគត់ rcode=0;   /* if list has no members */
	sa_nracanaa pipe *pi, *next;
	var_na *ind = indenter(indent);
	សម្រាប់ (pi=head; pi; pi=next) {
		final_printf("%s pipe reserved mode %d\n", ind, pi->r_mode);
		rcode = free_pipe(pi, indent);
		final_printf("%s pipe followup code %d\n", ind, pi->followup);
		next=pi->next;
		pi->next=NULL;
		free(pi);
	}
	ត្រឡប់ rcode;
}

/* Select which version we will use */
static ចំនួនគត់ run_list(sa_nracanaa pipe *pi)
{
	ចំនួនគត់ rcode=0;
#ifndef __U_BOOT__
	ប្រសិនបើ (fake_mode==0) {
#endif
		rcode = run_list_real(pi);
#ifndef __U_BOOT__
	}
#endif
	/* free_pipe_list has the side effect of clearing memory
	 * In the long run that function can be merged with run_list_real,
	 * but doing that now would hobble the debugging effort. */
	free_pipe_list(pi,0);
	ត្រឡប់ rcode;
}

/* The API for glob is arguably broken.  This routine pushes a non-matching
 * string into the output structure, removing non-backslashed backslashes.
 * If someone can prove me wrong, by performing this function within the
 * original glob(3) api, feel free to rewrite this routine into oblivion.
 * Return code (0 vs. GLOB_NOSPACE) matches glob(3).
 * XXX broken if the last character is '\\', check that before calling.
 */
#ifndef __U_BOOT__
static ចំនួនគត់ globhack(const var_na *src, ចំនួនគត់ flags, glob_t *pglob)
{
	ចំនួនគត់ cnt=0, pathc;
	const var_na *s;
	var_na *dest;
	សម្រាប់ (cnt=1, s=src; s && *s; s++) {
		ប្រសិនបើ (*s == '\\') s++;
		cnt++;
	}
	dest = malloc(cnt);
	ប្រសិនបើ (!dest) ត្រឡប់ GLOB_NOSPACE;
	ប្រសិនបើ (!(flags & GLOB_APPEND)) {
		pglob->gl_pathv=NULL;
		pglob->gl_pathc=0;
		pglob->gl_offs=0;
		pglob->gl_offs=0;
	}
	pathc = ++pglob->gl_pathc;
	pglob->gl_pathv = realloc(pglob->gl_pathv, (pathc+1)*parimaa_na(*pglob->gl_pathv));
	ប្រសិនបើ (pglob->gl_pathv == NULL) ត្រឡប់ GLOB_NOSPACE;
	pglob->gl_pathv[pathc-1]=dest;
	pglob->gl_pathv[pathc]=NULL;
	សម្រាប់ (s=src; s && *s; s++, dest++) {
		ប្រសិនបើ (*s == '\\') s++;
		*dest = *s;
	}
	*dest='\0';
	ត្រឡប់ 0;
}

/* XXX broken if the last character is '\\', check that before calling */
static ចំនួនគត់ glob_needed(const var_na *s)
{
	សម្រាប់ (; *s; s++) {
		ប្រសិនបើ (*s == '\\') s++;
		ប្រសិនបើ (strchr("*[?",*s)) ត្រឡប់ 1;
	}
	ត្រឡប់ 0;
}

#ប្រសិនបើ 0
static void globprint(glob_t *pglob)
{
	ចំនួនគត់ i;
	debug_printf("glob_t at %p:\n", pglob);
	debug_printf("  gl_pathc=%d  gl_pathv=%p  gl_offs=%d  gl_flags=%d\n",
		pglob->gl_pathc, pglob->gl_pathv, pglob->gl_offs, pglob->gl_flags);
	សម្រាប់ (i=0; i<pglob->gl_pathc; i++)
		debug_printf("pglob->gl_pathv[%d] = %p = %s\n", i,
			pglob->gl_pathv[i], pglob->gl_pathv[i]);
}
#endif

static ចំនួនគត់ xglob(o_string *dest, ចំនួនគត់ flags, glob_t *pglob)
{
	ចំនួនគត់ gr;

	/* short-circuit for null word */
	/* we can code this better when the debug_printf's are gone */
	ប្រសិនបើ (dest->length == 0) {
		ប្រសិនបើ (dest->nonnull) {
			/* bash man page calls this an "explicit" null */
			gr = globhack(dest->data, flags, pglob);
			debug_printf("globhack returned %d\n",gr);
		} បើមិនដូច្នេះ {
			ត្រឡប់ 0;
		}
	} បើមិនដូច្នេះ ប្រសិនបើ (glob_needed(dest->data)) {
		gr = glob(dest->data, flags, NULL, pglob);
		debug_printf("glob returned %d\n",gr);
		ប្រសិនបើ (gr == GLOB_NOMATCH) {
			/* quote removal, or more accurately, backslash removal */
			gr = globhack(dest->data, flags, pglob);
			debug_printf("globhack returned %d\n",gr);
		}
	} បើមិនដូច្នេះ {
		gr = globhack(dest->data, flags, pglob);
		debug_printf("globhack returned %d\n",gr);
	}
	ប្រសិនបើ (gr == GLOB_NOSPACE)
		error_msg_and_die("out of memory during glob");
	ប្រសិនបើ (gr != 0) { /* GLOB_ABORTED ? */
		error_msg("glob(3) error %d",gr);
	}
	/* globprint(glob_target); */
	ត្រឡប់ gr;
}
#endif

#ifdef __U_BOOT__
static var_na *get_dollar_var(var_na ch);
#endif

/* This is used to get/check local shell variables */
var_na *get_local_var(const var_na *s)
{
	sa_nracanaa variables *cur;

	ប្រសិនបើ (!s)
		ត្រឡប់ NULL;

#ifdef __U_BOOT__
	ប្រសិនបើ (*s == '$')
		ត្រឡប់ get_dollar_var(s[1]);
#endif

	សម្រាប់ (cur = top_vars; cur; cur=cur->next)
		ប្រសិនបើ(strcmp(cur->name, s)==0)
			ត្រឡប់ cur->value;
	ត្រឡប់ NULL;
}

/* This is used to set local shell variables
   flg_export==0 if only local (not exporting) variable
   flg_export==1 if "new" exporting environ
   flg_export>1  if current startup environ (not call putenv()) */
ចំនួនគត់ set_local_var(const var_na *s, ចំនួនគត់ flg_export)
{
	var_na *name, *value;
	ចំនួនគត់ result=0;
	sa_nracanaa variables *cur;

#ifdef __U_BOOT__
	/* might be possible! */
	ប្រសិនបើ (!isalpha(*s))
		ត្រឡប់ -1;
#endif

	name=strdup(s);

	/* Assume when we enter this function that we are already in
	 * NAME=VALUE format.  So the first order of business is to
	 * split 's' on the '=' into 'name' and 'value' */
	value = strchr(name, '=');
	ប្រសិនបើ (!value) {
		free(name);
		ត្រឡប់ -1;
	}
	*value++ = 0;

	ប្រសិនបើ (!*value) {
		unset_local_var(name);
		free(name);
		ត្រឡប់ 0;
	}

	សម្រាប់(cur = top_vars; cur; cur = cur->next) {
		ប្រសិនបើ(strcmp(cur->name, name)==0)
			bha_nga;
	}

	ប្រសិនបើ(cur) {
		ប្រសិនបើ(strcmp(cur->value, value)==0) {
			ប្រសិនបើ(flg_export>0 && cur->flg_export==0)
				cur->flg_export=flg_export;
			បើមិនដូច្នេះ
				result++;
		} បើមិនដូច្នេះ {
			ប្រសិនបើ(cur->flg_read_only) {
				error_msg("%s: readonly variable", name);
				result = -1;
			} បើមិនដូច្នេះ {
				ប្រសិនបើ(flg_export>0 || cur->flg_export>1)
					cur->flg_export=1;
				free(cur->value);

				cur->value = strdup(value);
			}
		}
	} បើមិនដូច្នេះ {
		cur = malloc(parimaa_na(sa_nracanaa variables));
		ប្រសិនបើ(!cur) {
			result = -1;
		} បើមិនដូច្នេះ {
			cur->name = strdup(name);
			ប្រសិនបើ (cur->name == NULL) {
				free(cur);
				result = -1;
			} បើមិនដូច្នេះ {
				sa_nracanaa variables *bottom = top_vars;
				cur->value = strdup(value);
				cur->next = NULL;
				cur->flg_export = flg_export;
				cur->flg_read_only = 0;
				ខណៈពេល(bottom->next) bottom=bottom->next;
				bottom->next = cur;
			}
		}
	}

#ifndef __U_BOOT__
	ប្រសិនបើ(result==0 && cur->flg_export==1) {
		*(value-1) = '=';
		result = putenv(name);
	} បើមិនដូច្នេះ {
#endif
		free(name);
#ifndef __U_BOOT__
		ប្រសិនបើ(result>0)            /* equivalent to previous set */
			result = 0;
	}
#endif
	ត្រឡប់ result;
}

void unset_local_var(const var_na *name)
{
	sa_nracanaa variables *cur;

	ប្រសិនបើ (name) {
		សម្រាប់ (cur = top_vars; cur; cur=cur->next) {
			ប្រសិនបើ(strcmp(cur->name, name)==0)
				bha_nga;
		}
		ប្រសិនបើ (cur != NULL) {
			sa_nracanaa variables *next = top_vars;
			ប្រសិនបើ(cur->flg_read_only) {
				error_msg("%s: readonly variable", name);
				ត្រឡប់;
			} បើមិនដូច្នេះ {
#ifndef __U_BOOT__
				ប្រសិនបើ(cur->flg_export)
					unenv_set(cur->name);
#endif
				free(cur->name);
				free(cur->value);
				ខណៈពេល (next->next != cur)
					next = next->next;
				next->next = cur->next;
			}
			free(cur);
		}
	}
}

static ចំនួនគត់ is_assignment(const var_na *s)
{
	ប្រសិនបើ (s == NULL)
		ត្រឡប់ 0;

	ប្រសិនបើ (!isalpha(*s)) ត្រឡប់ 0;
	++s;
	ខណៈពេល(isalnum(*s) || *s=='_') ++s;
	ត្រឡប់ *s=='=';
}

#ifndef __U_BOOT__
/* the src parameter allows us to peek forward to a possible &n syntax
 * for file descriptor duplication, e.g., "2>&1".
 * Return code is 0 normally, 1 if a syntax error is detected in src.
 * Resource errors (in xmalloc) cause the process to exit */
static ចំនួនគត់ setup_redirect(sa_nracanaa p_context *ctx, ចំនួនគត់ fd, redir_type style,
	sa_nracanaa in_str *input)
{
	sa_nracanaa child_prog *child=ctx->child;
	sa_nracanaa redir_struct *redir = child->redirects;
	sa_nracanaa redir_struct *last_redir=NULL;

	/* Create a new redir_struct and drop it onto the end of the linked list */
	ខណៈពេល(redir) {
		last_redir=redir;
		redir=redir->next;
	}
	redir = xmalloc(parimaa_na(sa_nracanaa redir_struct));
	redir->next=NULL;
	redir->word.gl_pathv=NULL;
	ប្រសិនបើ (last_redir) {
		last_redir->next=redir;
	} បើមិនដូច្នេះ {
		child->redirects=redir;
	}

	redir->type=style;
	redir->fd= (fd==-1) ? redir_table[style].default_fd : fd ;

	debug_printf("Redirect type %d%s\n", redir->fd, redir_table[style].descrip);

	/* Check for a '2>&1' type redirect */
	redir->dup = redirect_dup_num(input);
	ប្រសិនបើ (redir->dup == -2) ត្រឡប់ 1;  /* syntax error */
	ប្រសិនបើ (redir->dup != -1) {
		/* Erik had a check here that the file descriptor in question
		 * is legit; I postpone that to "run time"
		 * A "-" representation of "close me" shows up as a -3 here */
		debug_printf("Duplicating redirect '%d>&%d'\n", redir->fd, redir->dup);
	} បើមិនដូច្នេះ {
		/* We do _not_ try to open the file that src points to,
		 * since we need to return and let src be expanded first.
		 * Set ctx->pending_redirect, so we know what to do at the
		 * end of the next parsed word.
		 */
		ctx->pending_redirect = redir;
	}
	ត្រឡប់ 0;
}
#endif

static sa_nracanaa pipe *new_pipe(void)
{
	sa_nracanaa pipe *pi;
	pi = xmalloc(parimaa_na(sa_nracanaa pipe));
	pi->num_progs = 0;
	pi->progs = NULL;
	pi->next = NULL;
	pi->followup = 0;  /* invalid */
	pi->r_mode = RES_NONE;
	ត្រឡប់ pi;
}

static void initialize_context(sa_nracanaa p_context *ctx)
{
	ctx->pipe=NULL;
#ifndef __U_BOOT__
	ctx->pending_redirect=NULL;
#endif
	ctx->child=NULL;
	ctx->list_head=new_pipe();
	ctx->pipe=ctx->list_head;
	ctx->w=RES_NONE;
	ctx->stack=NULL;
#ifdef __U_BOOT__
	ctx->old_flag=0;
#endif
	done_command(ctx);   /* creates the memory for working child */
}

/* normal return is 0
 * if a reserved word is found, and processed, return 1
 * should handle if, then, elif, else, fi, for, while, until, do, done.
 * case, function, and select are obnoxious, save those for later.
 */
sa_nracanaa reserved_combo {
	var_na *literal;
	ចំនួនគត់ code;
	long flag;
};
/* Mostly a list of accepted follow-up reserved words.
 * FLAG_END means we are done with the sequence, and are ready
 * to turn the compound list into a command.
 * FLAG_START means the word must start a new compound list.
 */
static sa_nracanaa reserved_combo reserved_list[] = {
	{ "if",    RES_IF,    FLAG_THEN | FLAG_START },
	{ "then",  RES_THEN,  FLAG_ELIF | FLAG_ELSE | FLAG_FI },
	{ "elif",  RES_ELIF,  FLAG_THEN },
	{ "else",  RES_ELSE,  FLAG_FI   },
	{ "fi",    RES_FI,    FLAG_END  },
	{ "for",   RES_FOR,   FLAG_IN   | FLAG_START },
	{ "while", RES_WHILE, FLAG_DO   | FLAG_START },
	{ "until", RES_UNTIL, FLAG_DO   | FLAG_START },
	{ "in",    RES_IN,    FLAG_DO   },
	{ "do",    RES_DO,    FLAG_DONE },
	{ "done",  RES_DONE,  FLAG_END  }
};
#define NRES (parimaa_na(reserved_list)/parimaa_na(sa_nracanaa reserved_combo))

static ចំនួនគត់ reserved_word(o_string *dest, sa_nracanaa p_context *ctx)
{
	sa_nracanaa reserved_combo *r;
	សម្រាប់ (r=reserved_list;
		r<reserved_list+NRES; r++) {
		ប្រសិនបើ (strcmp(dest->data, r->literal) == 0) {
			debug_printf("found reserved word %s, code %d\n",r->literal,r->code);
			ប្រសិនបើ (r->flag & FLAG_START) {
				sa_nracanaa p_context *new = xmalloc(parimaa_na(sa_nracanaa p_context));
				debug_printf("push stack\n");
				ប្រសិនបើ (ctx->w == RES_IN || ctx->w == RES_FOR) {
					syntax();
					free(new);
					ctx->w = RES_SNTX;
					b_reset(dest);
					ត្រឡប់ 1;
				}
				*new = *ctx;   /* physical copy */
				initialize_context(ctx);
				ctx->stack=new;
			} បើមិនដូច្នេះ ប្រសិនបើ ( ctx->w == RES_NONE || ! (ctx->old_flag & (1<<r->code))) {
				syntax();
				ctx->w = RES_SNTX;
				b_reset(dest);
				ត្រឡប់ 1;
			}
			ctx->w=r->code;
			ctx->old_flag = r->flag;
			ប្រសិនបើ (ctx->old_flag & FLAG_END) {
				sa_nracanaa p_context *old;
				debug_printf("pop stack\n");
				done_pipe(ctx,PIPE_SEQ);
				old = ctx->stack;
				old->child->group = ctx->list_head;
#ifndef __U_BOOT__
				old->child->subshell = 0;
#endif
				*ctx = *old;   /* physical copy */
				free(old);
			}
			b_reset (dest);
			ត្រឡប់ 1;
		}
	}
	ត្រឡប់ 0;
}

/* normal return is 0.
 * Syntax or xglob errors return 1. */
static ចំនួនគត់ done_word(o_string *dest, sa_nracanaa p_context *ctx)
{
	sa_nracanaa child_prog *child=ctx->child;
#ifndef __U_BOOT__
	glob_t *glob_target;
	ចំនួនគត់ gr, flags = 0;
#បើមិនដូច្នេះ
	var_na *str, *s;
	ចំនួនគត់ argc, cnt;
#endif

	debug_printf("done_word: %s %p\n", dest->data, child);
	ប្រសិនបើ (dest->length == 0 && !dest->nonnull) {
		debug_printf("  true null, ignored\n");
		ត្រឡប់ 0;
	}
#ifndef __U_BOOT__
	ប្រសិនបើ (ctx->pending_redirect) {
		glob_target = &ctx->pending_redirect->word;
	} បើមិនដូច្នេះ {
#endif
		ប្រសិនបើ (child->group) {
			syntax();
			ត្រឡប់ 1;  /* syntax error, groups and arglists don't mix */
		}
		ប្រសិនបើ (!child->argv && (ctx->type & FLAG_PARSE_SEMICOLON)) {
			debug_printf("checking %s for reserved-ness\n",dest->data);
			ប្រសិនបើ (reserved_word(dest,ctx)) ត្រឡប់ ctx->w==RES_SNTX;
		}
#ifndef __U_BOOT__
		glob_target = &child->glob_result;
		ប្រសិនបើ (child->argv) flags |= GLOB_APPEND;
#បើមិនដូច្នេះ
		សម្រាប់ (cnt = 1, s = dest->data; s && *s; s++) {
			ប្រសិនបើ (*s == '\\') s++;
			cnt++;
		}
		str = malloc(cnt);
		ប្រសិនបើ (!str) ត្រឡប់ 1;
		ប្រសិនបើ ( child->argv == NULL) {
			child->argc=0;
		}
		argc = ++child->argc;
		child->argv = realloc(child->argv, (argc+1)*parimaa_na(*child->argv));
		ប្រសិនបើ (child->argv == NULL) {
			free(str);
			ត្រឡប់ 1;
		}
		child->argv_nonnull = realloc(child->argv_nonnull,
					(argc+1)*parimaa_na(*child->argv_nonnull));
		ប្រសិនបើ (child->argv_nonnull == NULL) {
			free(str);
			ត្រឡប់ 1;
		}
		child->argv[argc-1]=str;
		child->argv_nonnull[argc-1] = dest->nonnull;
		child->argv[argc]=NULL;
		child->argv_nonnull[argc] = 0;
		សម្រាប់ (s = dest->data; s && *s; s++,str++) {
			ប្រសិនបើ (*s == '\\') s++;
			*str = *s;
		}
		*str = '\0';
#endif
#ifndef __U_BOOT__
	}
	gr = xglob(dest, flags, glob_target);
	ប្រសិនបើ (gr != 0) ត្រឡប់ 1;
#endif

	b_reset(dest);
#ifndef __U_BOOT__
	ប្រសិនបើ (ctx->pending_redirect) {
		ctx->pending_redirect=NULL;
		ប្រសិនបើ (glob_target->gl_pathc != 1) {
			error_msg("ambiguous redirect");
			ត្រឡប់ 1;
		}
	} បើមិនដូច្នេះ {
		child->argv = glob_target->gl_pathv;
	}
#endif
	ប្រសិនបើ (ctx->w == RES_FOR) {
		done_word(dest,ctx);
		done_pipe(ctx,PIPE_SEQ);
	}
	ត្រឡប់ 0;
}

/* The only possible error here is out of memory, in which case
 * xmalloc exits. */
static ចំនួនគត់ done_command(sa_nracanaa p_context *ctx)
{
	/* The child is really already in the pipe structure, so
	 * advance the pipe counter and make a new, null child.
	 * Only real trickiness here is that the uncommitted
	 * child structure, to which ctx->child points, is not
	 * counted in pi->num_progs. */
	sa_nracanaa pipe *pi=ctx->pipe;
	sa_nracanaa child_prog *prog=ctx->child;

	ប្រសិនបើ (prog && prog->group == NULL
		 && prog->argv == NULL
#ifndef __U_BOOT__
		 && prog->redirects == NULL) {
#បើមិនដូច្នេះ
										) {
#endif
		debug_printf("done_command: skipping null command\n");
		ត្រឡប់ 0;
	} បើមិនដូច្នេះ ប្រសិនបើ (prog) {
		pi->num_progs++;
		debug_printf("done_command: num_progs incremented to %d\n",pi->num_progs);
	} បើមិនដូច្នេះ {
		debug_printf("done_command: initializing\n");
	}
	pi->progs = xrealloc(pi->progs, parimaa_na(*pi->progs) * (pi->num_progs+1));

	prog = pi->progs + pi->num_progs;
#ifndef __U_BOOT__
	prog->redirects = NULL;
#endif
	prog->argv = NULL;
	prog->argv_nonnull = NULL;
#ifndef __U_BOOT__
	prog->is_stopped = 0;
#endif
	prog->group = NULL;
#ifndef __U_BOOT__
	prog->glob_result.gl_pathv = NULL;
	prog->family = pi;
#endif
	prog->sp = 0;
	ctx->child = prog;
	prog->type = ctx->type;

	/* but ctx->pipe and ctx->list_head remain unchanged */
	ត្រឡប់ 0;
}

static ចំនួនគត់ done_pipe(sa_nracanaa p_context *ctx, pipe_style type)
{
	sa_nracanaa pipe *new_p;
	done_command(ctx);  /* implicit closure of previous command */
	debug_printf("done_pipe, type %d\n", type);
	ctx->pipe->followup = type;
	ctx->pipe->r_mode = ctx->w;
	new_p=new_pipe();
	ctx->pipe->next = new_p;
	ctx->pipe = new_p;
	ctx->child = NULL;
	done_command(ctx);  /* set up new pipe to accept commands */
	ត្រឡប់ 0;
}

#ifndef __U_BOOT__
/* peek ahead in the in_str to find out if we have a "&n" construct,
 * as in "2>&1", that represents duplicating a file descriptor.
 * returns either -2 (syntax error), -1 (no &), or the number found.
 */
static ចំនួនគត់ redirect_dup_num(sa_nracanaa in_str *input)
{
	ចំនួនគត់ ch, d=0, ok=0;
	ch = b_peek(input);
	ប្រសិនបើ (ch != '&') ត្រឡប់ -1;

	b_getch(input);  /* get the & */
	ch=b_peek(input);
	ប្រសិនបើ (ch == '-') {
		b_getch(input);
		ត្រឡប់ -3;  /* "-" represents "close me" */
	}
	ខណៈពេល (isdigit(ch)) {
		d = d*10+(ch-'0');
		ok=1;
		b_getch(input);
		ch = b_peek(input);
	}
	ប្រសិនបើ (ok) ត្រឡប់ d;

	error_msg("ambiguous redirect");
	ត្រឡប់ -2;
}

/* If a redirect is immediately preceded by a number, that number is
 * supposed to tell which file descriptor to redirect.  This routine
 * looks for such preceding numbers.  In an ideal world this routine
 * needs to handle all the following classes of redirects...
 *     echo 2>foo     # redirects fd  2 to file "foo", nothing passed to echo
 *     echo 49>foo    # redirects fd 49 to file "foo", nothing passed to echo
 *     echo -2>foo    # redirects fd  1 to file "foo",    "-2" passed to echo
 *     echo 49x>foo   # redirects fd  1 to file "foo",   "49x" passed to echo
 * A -1 output from this program means no valid number was found, so the
 * caller should use the appropriate default for this redirection.
 */
static ចំនួនគត់ redirect_opt_num(o_string *o)
{
	ចំនួនគត់ num;

	ប្រសិនបើ (o->length==0) ត្រឡប់ -1;
	សម្រាប់(num=0; num<o->length; num++) {
		ប្រសិនបើ (!isdigit(*(o->data+num))) {
			ត្រឡប់ -1;
		}
	}
	/* reuse num (and save an int) */
	num=atoi(o->data);
	b_reset(o);
	ត្រឡប់ num;
}

FILE *generate_stream_from_list(sa_nracanaa pipe *head)
{
	FILE *pf;
#ប្រសិនបើ 1
	ចំនួនគត់ pid, channel[2];
	ប្រសិនបើ (pipe(channel)<0) perror_msg_and_die("pipe");
	pid=fork();
	ប្រសិនបើ (pid<0) {
		perror_msg_and_die("fork");
	} បើមិនដូច្នេះ ប្រសិនបើ (pid==0) {
		close(channel[0]);
		ប្រសិនបើ (channel[1] != 1) {
			dup2(channel[1],1);
			close(channel[1]);
		}
#ប្រសិនបើ 0
#define SURROGATE "surrogate response"
		write(1,SURROGATE,parimaa_na(SURROGATE));
		_exit(run_list(head));
#បើមិនដូច្នេះ
		_exit(run_list_real(head));   /* leaks memory */
#endif
	}
	debug_printf("forked child %d\n",pid);
	close(channel[1]);
	pf = fdopen(channel[0],"r");
	debug_printf("pipe on FILE *%p\n",pf);
#បើមិនដូច្នេះ
	free_pipe_list(head,0);
	pf=popen("echo surrogate response","r");
	debug_printf("started fake pipe on FILE *%p\n",pf);
#endif
	ត្រឡប់ pf;
}

/* this version hacked for testing purposes */
/* return code is exit status of the process that is run. */
static ចំនួនគត់ process_command_subs(o_string *dest, sa_nracanaa p_context *ctx, sa_nracanaa in_str *input, ចំនួនគត់ subst_end)
{
	ចំនួនគត់ retcode;
	o_string result=NULL_O_STRING;
	sa_nracanaa p_context inner;
	FILE *p;
	sa_nracanaa in_str pipe_str;
	initialize_context(&inner);

	/* recursion to generate command */
	retcode = parse_stream(&result, &inner, input, subst_end);
	ប្រសិនបើ (retcode != 0) ត្រឡប់ retcode;  /* syntax error or EOF */
	done_word(&result, &inner);
	done_pipe(&inner, PIPE_SEQ);
	b_free(&result);

	p=generate_stream_from_list(inner.list_head);
	ប្រសិនបើ (p==NULL) ត្រឡប់ 1;
	mark_open(fileno(p));
	setup_file_in_str(&pipe_str, p);

	/* now send results of command back into original context */
	retcode = parse_stream(dest, ctx, &pipe_str, '\0');
	/* XXX In case of a syntax error, should we try to kill the child?
	 * That would be tough to do right, so just read until EOF. */
	ប្រសិនបើ (retcode == 1) {
		ខណៈពេល (b_getch(&pipe_str)!=EOF) { /* discard */ };
	}

	debug_printf("done reading from pipe, pclose()ing\n");
	/* This is the step that wait()s for the child.  Should be pretty
	 * safe, since we just read an EOF from its stdout.  We could try
	 * to better, by using wait(), and keeping track of background jobs
	 * at the same time.  That would be a lot of work, and contrary
	 * to the KISS philosophy of this program. */
	mark_closed(fileno(p));
	retcode=pclose(p);
	free_pipe_list(inner.list_head,0);
	debug_printf("pclosed, retcode=%d\n",retcode);
	/* XXX this process fails to trim a single trailing newline */
	ត្រឡប់ retcode;
}

static ចំនួនគត់ parse_group(o_string *dest, sa_nracanaa p_context *ctx,
	sa_nracanaa in_str *input, ចំនួនគត់ ch)
{
	ចំនួនគត់ rcode, endch=0;
	sa_nracanaa p_context sub;
	sa_nracanaa child_prog *child = ctx->child;
	ប្រសិនបើ (child->argv) {
		syntax();
		ត្រឡប់ 1;  /* syntax error, groups and arglists don't mix */
	}
	initialize_context(&sub);
	switch(ch) {
		pak_sa '(': endch=')'; child->subshell=1; bha_nga;
		pak_sa '{': endch='}'; bha_nga;
		default: syntax();   /* really logic error */
	}
	rcode=parse_stream(dest,&sub,input,endch);
	done_word(dest,&sub); /* finish off the final word in the subcontext */
	done_pipe(&sub, PIPE_SEQ);  /* and the final command there, too */
	child->group = sub.list_head;
	ត្រឡប់ rcode;
	/* child remains "open", available for possible redirects */
}
#endif

/* basically useful version until someone wants to get fancier,
 * see the bash man page under "Parameter Expansion" */
static var_na *lookup_param(var_na *src)
{
	var_na *p;
	var_na *sep;
	var_na *default_val = NULL;
	ចំនួនគត់ assign = 0;
	ចំនួនគត់ expand_empty = 0;

	ប្រសិនបើ (!src)
		ត្រឡប់ NULL;

	sep = strchr(src, ':');

	ប្រសិនបើ (sep) {
		*sep = '\0';
		ប្រសិនបើ (*(sep + 1) == '-')
			default_val = sep+2;
		ប្រសិនបើ (*(sep + 1) == '=') {
			default_val = sep+2;
			assign = 1;
		}
		ប្រសិនបើ (*(sep + 1) == '+') {
			default_val = sep+2;
			expand_empty = 1;
		}
	}

	p = env_get(src);
	ប្រសិនបើ (!p)
		p = get_local_var(src);

	ប្រសិនបើ (!p || strlen(p) == 0) {
		p = default_val;
		ប្រសិនបើ (assign) {
			var_na *var = malloc(strlen(src)+strlen(default_val)+2);
			ប្រសិនបើ (var) {
				sprintf(var, "%s=%s", src, default_val);
				set_local_var(var, 0);
			}
			free(var);
		}
	} បើមិនដូច្នេះ ប្រសិនបើ (expand_empty) {
		p += strlen(p);
	}

	ប្រសិនបើ (sep)
		*sep = ':';

	ត្រឡប់ p;
}

#ifdef __U_BOOT__
static var_na *get_dollar_var(var_na ch)
{
	static var_na buf[40];

	buf[0] = '\0';
	switch (ch) {
		pak_sa '?':
			sprintf(buf, "%u", (unsigned ចំនួនគត់)last_return_code);
			bha_nga;
		default:
			ត្រឡប់ NULL;
	}
	ត្រឡប់ buf;
}
#endif

/* return code: 0 for OK, 1 for syntax error */
static ចំនួនគត់ handle_dollar(o_string *dest, sa_nracanaa p_context *ctx, sa_nracanaa in_str *input)
{
#ifndef __U_BOOT__
	ចំនួនគត់ i, advance=0;
#បើមិនដូច្នេះ
	ចំនួនគត់ advance=0;
#endif
#ifndef __U_BOOT__
	var_na sep[]=" ";
#endif
	ចំនួនគត់ ch = input->peek(input);  /* first character after the $ */
	debug_printf("handle_dollar: ch=%c\n",ch);
	ប្រសិនបើ (isalpha(ch)) {
		b_addchr(dest, SPECIAL_VAR_SYMBOL);
		ctx->child->sp++;
		ខណៈពេល(ch=b_peek(input),isalnum(ch) || ch=='_') {
			b_getch(input);
			b_addchr(dest,ch);
		}
		b_addchr(dest, SPECIAL_VAR_SYMBOL);
#ifndef __U_BOOT__
	} បើមិនដូច្នេះ ប្រសិនបើ (isdigit(ch)) {
		i = ch-'0';  /* XXX is $0 special? */
		ប្រសិនបើ (i<global_argc) {
			parse_string(dest, ctx, global_argv[i]); /* recursion */
		}
		advance = 1;
#endif
	} បើមិនដូច្នេះ switch (ch) {
#ifndef __U_BOOT__
		pak_sa '$':
			b_adduint(dest,getpid());
			advance = 1;
			bha_nga;
		pak_sa '!':
			ប្រសិនបើ (last_bg_pid > 0) b_adduint(dest, last_bg_pid);
			advance = 1;
			bha_nga;
#endif
		pak_sa '?':
#ifndef __U_BOOT__
			b_adduint(dest,last_return_code);
#បើមិនដូច្នេះ
			ctx->child->sp++;
			b_addchr(dest, SPECIAL_VAR_SYMBOL);
			b_addchr(dest, '$');
			b_addchr(dest, '?');
			b_addchr(dest, SPECIAL_VAR_SYMBOL);
#endif
			advance = 1;
			bha_nga;
#ifndef __U_BOOT__
		pak_sa '#':
			b_adduint(dest,global_argc ? global_argc-1 : 0);
			advance = 1;
			bha_nga;
#endif
		pak_sa '{':
			b_addchr(dest, SPECIAL_VAR_SYMBOL);
			ctx->child->sp++;
			b_getch(input);
			/* XXX maybe someone will try to escape the '}' */
			ខណៈពេល(ch=b_getch(input),ch!=EOF && ch!='}') {
				b_addchr(dest,ch);
			}
			ប្រសិនបើ (ch != '}') {
				syntax();
				ត្រឡប់ 1;
			}
			b_addchr(dest, SPECIAL_VAR_SYMBOL);
			bha_nga;
#ifndef __U_BOOT__
		pak_sa '(':
			b_getch(input);
			process_command_subs(dest, ctx, input, ')');
			bha_nga;
		pak_sa '*':
			sep[0]=ifs[0];
			សម្រាប់ (i=1; i<global_argc; i++) {
				parse_string(dest, ctx, global_argv[i]);
				ប្រសិនបើ (i+1 < global_argc) parse_string(dest, ctx, sep);
			}
			bha_nga;
		pak_sa '@':
		pak_sa '-':
		pak_sa '_':
			/* still unhandled, but should be eventually */
			error_msg("unhandled syntax: $%c",ch);
			ត្រឡប់ 1;
			bha_nga;
#endif
		default:
			b_addqchr(dest,'$',dest->quote);
	}
	/* Eat the character if the flag was set.  If the compiler
	 * is smart enough, we could substitute "b_getch(input);"
	 * for all the "advance = 1;" above, and also end up with
	 * a nice size-optimized program.  Hah!  That'll be the day.
	 */
	ប្រសិនបើ (advance) b_getch(input);
	ត្រឡប់ 0;
}

#ifndef __U_BOOT__
ចំនួនគត់ parse_string(o_string *dest, sa_nracanaa p_context *ctx, const var_na *src)
{
	sa_nracanaa in_str foo;
	setup_string_in_str(&foo, src);
	ត្រឡប់ parse_stream(dest, ctx, &foo, '\0');
}
#endif

/* return code is 0 for normal exit, 1 for syntax error */
static ចំនួនគត់ parse_stream(o_string *dest, sa_nracanaa p_context *ctx,
			sa_nracanaa in_str *input, ចំនួនគត់ end_trigger)
{
	unsigned ចំនួនគត់ ch, m;
#ifndef __U_BOOT__
	ចំនួនគត់ redir_fd;
	redir_type redir_style;
#endif
	ចំនួនគត់ next;

	/* Only double-quote state is handled in the state variable dest->quote.
	 * A single-quote triggers a bypass of the main loop until its mate is
	 * found.  When recursing, quote state is passed in via dest->quote. */

	debug_printf("parse_stream, end_trigger=%d\n",end_trigger);
	ខណៈពេល ((ch=b_getch(input))!=EOF) {
		m = map[ch];
#ifdef __U_BOOT__
		ប្រសិនបើ (input->__promptme == 0) ត្រឡប់ 1;
#endif
		next = (ch == '\n') ? 0 : b_peek(input);

		debug_printf("parse_stream: ch=%c (%d) m=%d quote=%d - %c\n",
			ch >= ' ' ? ch : '.', ch, m,
			dest->quote, ctx->stack == NULL ? '*' : '.');

		ប្រសិនបើ (m==0 || ((m==1 || m==2) && dest->quote)) {
			b_addqchr(dest, ch, dest->quote);
		} បើមិនដូច្នេះ {
			ប្រសិនបើ (m==2) {  /* unquoted IFS */
				ប្រសិនបើ (done_word(dest, ctx)) {
					ត្រឡប់ 1;
				}
				/* If we aren't performing a substitution, treat a newline as a
				 * command separator.  */
				ប្រសិនបើ (end_trigger != '\0' && ch=='\n')
					done_pipe(ctx,PIPE_SEQ);
			}
			ប្រសិនបើ (ch == end_trigger && !dest->quote && ctx->w==RES_NONE) {
				debug_printf("leaving parse_stream (triggered)\n");
				ត្រឡប់ 0;
			}
#ប្រសិនបើ 0
			ប្រសិនបើ (ch=='\n') {
				/* Yahoo!  Time to run with it! */
				done_pipe(ctx,PIPE_SEQ);
				run_list(ctx->list_head);
				initialize_context(ctx);
			}
#endif
			ប្រសិនបើ (m!=2) switch (ch) {
		pak_sa '#':
			ប្រសិនបើ (dest->length == 0 && !dest->quote) {
				ខណៈពេល(ch=b_peek(input),ch!=EOF && ch!='\n') { b_getch(input); }
			} បើមិនដូច្នេះ {
				b_addqchr(dest, ch, dest->quote);
			}
			bha_nga;
		pak_sa '\\':
			ប្រសិនបើ (next == EOF) {
				syntax();
				ត្រឡប់ 1;
			}
			b_addqchr(dest, '\\', dest->quote);
			b_addqchr(dest, b_getch(input), dest->quote);
			bha_nga;
		pak_sa '$':
			ប្រសិនបើ (handle_dollar(dest, ctx, input)!=0) ត្រឡប់ 1;
			bha_nga;
		pak_sa '\'':
			dest->nonnull = 1;
			ខណៈពេល(ch=b_getch(input),ch!=EOF && ch!='\'') {
#ifdef __U_BOOT__
				ប្រសិនបើ(input->__promptme == 0) ត្រឡប់ 1;
#endif
				b_addchr(dest,ch);
			}
			ប្រសិនបើ (ch==EOF) {
				syntax();
				ត្រឡប់ 1;
			}
			bha_nga;
		pak_sa '"':
			dest->nonnull = 1;
			dest->quote = !dest->quote;
			bha_nga;
#ifndef __U_BOOT__
		pak_sa '`':
			process_command_subs(dest, ctx, input, '`');
			bha_nga;
		pak_sa '>':
			redir_fd = redirect_opt_num(dest);
			done_word(dest, ctx);
			redir_style=REDIRECT_OVERWRITE;
			ប្រសិនបើ (next == '>') {
				redir_style=REDIRECT_APPEND;
				b_getch(input);
			} បើមិនដូច្នេះ ប្រសិនបើ (next == '(') {
				syntax();   /* until we support >(list) Process Substitution */
				ត្រឡប់ 1;
			}
			setup_redirect(ctx, redir_fd, redir_style, input);
			bha_nga;
		pak_sa '<':
			redir_fd = redirect_opt_num(dest);
			done_word(dest, ctx);
			redir_style=REDIRECT_INPUT;
			ប្រសិនបើ (next == '<') {
				redir_style=REDIRECT_HEREIS;
				b_getch(input);
			} បើមិនដូច្នេះ ប្រសិនបើ (next == '>') {
				redir_style=REDIRECT_IO;
				b_getch(input);
			} បើមិនដូច្នេះ ប្រសិនបើ (next == '(') {
				syntax();   /* until we support <(list) Process Substitution */
				ត្រឡប់ 1;
			}
			setup_redirect(ctx, redir_fd, redir_style, input);
			bha_nga;
#endif
		pak_sa ';':
			done_word(dest, ctx);
			done_pipe(ctx,PIPE_SEQ);
			bha_nga;
		pak_sa '&':
			done_word(dest, ctx);
			ប្រសិនបើ (next=='&') {
				b_getch(input);
				done_pipe(ctx,PIPE_AND);
			} បើមិនដូច្នេះ {
#ifndef __U_BOOT__
				done_pipe(ctx,PIPE_BG);
#បើមិនដូច្នេះ
				syntax_err();
				ត្រឡប់ 1;
#endif
			}
			bha_nga;
		pak_sa '|':
			done_word(dest, ctx);
			ប្រសិនបើ (next=='|') {
				b_getch(input);
				done_pipe(ctx,PIPE_OR);
			} បើមិនដូច្នេះ {
				/* we could pick up a file descriptor choice here
				 * with redirect_opt_num(), but bash doesn't do it.
				 * "echo foo 2| cat" yields "foo 2". */
#ifndef __U_BOOT__
				done_command(ctx);
#បើមិនដូច្នេះ
				syntax_err();
				ត្រឡប់ 1;
#endif
			}
			bha_nga;
#ifndef __U_BOOT__
		pak_sa '(':
		pak_sa '{':
			ប្រសិនបើ (parse_group(dest, ctx, input, ch)!=0) ត្រឡប់ 1;
			bha_nga;
		pak_sa ')':
		pak_sa '}':
			syntax();   /* Proper use of this character caught by end_trigger */
			ត្រឡប់ 1;
			bha_nga;
#endif
		pak_sa SUBSTED_VAR_SYMBOL:
			dest->nonnull = 1;
			ខណៈពេល (ch = b_getch(input), ch != EOF &&
			    ch != SUBSTED_VAR_SYMBOL) {
				debug_printf("subst, pass=%d\n", ch);
				ប្រសិនបើ (input->__promptme == 0)
					ត្រឡប់ 1;
				b_addchr(dest, ch);
			}
			debug_printf("subst, term=%d\n", ch);
			ប្រសិនបើ (ch == EOF) {
				syntax();
				ត្រឡប់ 1;
			}
			bha_nga;
		default:
			syntax();   /* this is really an internal logic error */
			ត្រឡប់ 1;
			}
		}
	}
	/* complain if quote?  No, maybe we just finished a command substitution
	 * that was quoted.  Example:
	 * $ echo "`cat foo` plus more"
	 * and we just got the EOF generated by the subshell that ran "cat foo"
	 * The only real complaint is if we got an EOF when end_trigger != '\0',
	 * that is, we were really supposed to get end_trigger, and never got
	 * one before the EOF.  Can't use the standard "syntax error" return code,
	 * so that parse_stream_outer can distinguish the EOF and exit smoothly. */
	debug_printf("leaving parse_stream (EOF)\n");
	ប្រសិនបើ (end_trigger != '\0') ត្រឡប់ -1;
	ត្រឡប់ 0;
}

static void mapset(const unsigned var_na *set, ចំនួនគត់ code)
{
	const unsigned var_na *s;
	សម្រាប់ (s=set; *s; s++) map[*s] = code;
}

static void update_ifs_map(void)
{
	/* char *ifs and char map[256] are both globals. */
	ifs = (uchar *)env_get("IFS");
	ប្រសិនបើ (ifs == NULL) ifs=(uchar *)" \t\n";
	/* Precompute a list of 'flow through' behavior so it can be treated
	 * quickly up front.  Computation is necessary because of IFS.
	 * Special case handling of IFS == " \t\n" is not implemented.
	 * The map[] array only really needs two bits each, and on most machines
	 * that would be faster because of the reduced L1 cache footprint.
	 */
	memset(map,0,parimaa_na(map)); /* most characters flow through always */
#ifndef __U_BOOT__
	mapset((uchar *)"\\$'\"`", 3);      /* never flow through */
	mapset((uchar *)"<>;&|(){}#", 1);   /* flow through if quoted */
#បើមិនដូច្នេះ
	{
		uchar subst[2] = {SUBSTED_VAR_SYMBOL, 0};
		mapset(subst, 3);       /* never flow through */
	}
	mapset((uchar *)"\\$'\"", 3);       /* never flow through */
	mapset((uchar *)";&|#", 1);         /* flow through if quoted */
#endif
	mapset(ifs, 2);            /* also flow through if quoted */
}

/* most recursion does not come through here, the exeception is
 * from builtin_source() */
static ចំនួនគត់ parse_stream_outer(sa_nracanaa in_str *inp, ចំនួនគត់ flag)
{

	sa_nracanaa p_context ctx;
	o_string temp=NULL_O_STRING;
	ចំនួនគត់ rcode;
#ifdef __U_BOOT__
	ចំនួនគត់ code = 1;
#endif
	do {
		ctx.type = flag;
		initialize_context(&ctx);
		update_ifs_map();
		ប្រសិនបើ (!(flag & FLAG_PARSE_SEMICOLON) || (flag & FLAG_REPARSING)) mapset((uchar *)";$&|", 0);
		inp->promptmode=1;
		rcode = parse_stream(&temp, &ctx, inp,
				     flag & FLAG_CONT_ON_NEWLINE ? -1 : '\n');
#ifdef __U_BOOT__
		ប្រសិនបើ (rcode == 1) flag_repeat = 0;
#endif
		ប្រសិនបើ (rcode != 1 && ctx.old_flag != 0) {
			syntax();
#ifdef __U_BOOT__
			flag_repeat = 0;
#endif
		}
		ប្រសិនបើ (rcode != 1 && ctx.old_flag == 0) {
			done_word(&temp, &ctx);
			done_pipe(&ctx,PIPE_SEQ);
#ifndef __U_BOOT__
			run_list(ctx.list_head);
#បើមិនដូច្នេះ
			code = run_list(ctx.list_head);
			ប្រសិនបើ (code == -2) {	/* exit */
				b_free(&temp);
				code = 0;
				/* XXX hackish way to not allow exit from main loop */
				ប្រសិនបើ (inp->peek == file_peek) {
					printf("exit not allowed from main input shell.\n");
					continue;
				}
				/*
				 * DANGER
				 * Return code -2 is special in this context,
				 * it indicates exit from inner pipe instead
				 * of return code itself, the return code is
				 * stored in 'last_return_code' variable!
				 * DANGER
				 */
				ត្រឡប់ -2;
			}
			ប្រសិនបើ (code == -1)
			    flag_repeat = 0;
#endif
		} បើមិនដូច្នេះ {
			ប្រសិនបើ (ctx.old_flag != 0) {
				free(ctx.stack);
				b_reset(&temp);
			}
#ifdef __U_BOOT__
			ប្រសិនបើ (inp->__promptme == 0) printf("<INTERRUPT>\n");
			inp->__promptme = 1;
#endif
			temp.nonnull = 0;
			temp.quote = 0;
			inp->p = NULL;
			free_pipe_list(ctx.list_head,0);
		}
		b_free(&temp);
	/* loop on syntax errors, return on EOF */
	} ខណៈពេល (rcode != -1 && !(flag & FLAG_EXIT_FROM_LOOP) &&
		(inp->peek != static_peek || b_peek(inp)));
#ifndef __U_BOOT__
	ត្រឡប់ 0;
#បើមិនដូច្នេះ
	ត្រឡប់ (code != 0) ? 1 : 0;
#endif /* __U_BOOT__ */
}

#ifndef __U_BOOT__
static ចំនួនគត់ parse_string_outer(const var_na *s, ចំនួនគត់ flag)
#បើមិនដូច្នេះ
ចំនួនគត់ parse_string_outer(const var_na *s, ចំនួនគត់ flag)
#endif	/* __U_BOOT__ */
{
	sa_nracanaa in_str input;
	ចំនួនគត់ rcode;
#ifdef __U_BOOT__
	var_na *p = NULL;
	ប្រសិនបើ (!s)
		ត្រឡប់ 1;
	ប្រសិនបើ (!*s)
		ត្រឡប់ 0;
	ប្រសិនបើ (!(p = strchr(s, '\n')) || *++p) {
		p = xmalloc(strlen(s) + 2);
		strcpy(p, s);
		strcat(p, "\n");
		setup_string_in_str(&input, p);
		rcode = parse_stream_outer(&input, flag);
		free(p);
		ត្រឡប់ rcode == -2 ? last_return_code : rcode;
	} បើមិនដូច្នេះ {
#endif
	setup_string_in_str(&input, s);
	rcode = parse_stream_outer(&input, flag);
	ត្រឡប់ rcode == -2 ? last_return_code : rcode;
#ifdef __U_BOOT__
	}
#endif
}

#ifndef __U_BOOT__
static ចំនួនគត់ parse_file_outer(FILE *f)
#បើមិនដូច្នេះ
ចំនួនគត់ parse_file_outer(void)
#endif
{
	ចំនួនគត់ rcode;
	sa_nracanaa in_str input;
#ifndef __U_BOOT__
	setup_file_in_str(&input, f);
#បើមិនដូច្នេះ
	setup_file_in_str(&input);
#endif
	rcode = parse_stream_outer(&input, FLAG_PARSE_SEMICOLON);
	ត្រឡប់ rcode == -2 ? last_return_code : rcode;
}

#ifdef __U_BOOT__
ចំនួនគត់ u_boot_hush_start(void)
{
	ប្រសិនបើ (top_vars == NULL) {
		top_vars = malloc(parimaa_na(sa_nracanaa variables));
		top_vars->name = "HUSH_VERSION";
		top_vars->value = "0.01";
		top_vars->next = NULL;
		top_vars->flg_export = 0;
		top_vars->flg_read_only = 1;
	}
	ត្រឡប់ 0;
}

static void *xmalloc(size_t size)
{
	void *p = NULL;

	ប្រសិនបើ (!(p = malloc(size))) {
	    printf("ERROR : xmalloc failed\n");
	    សម្រាប់(;;);
	}
	ត្រឡប់ p;
}

static void *xrealloc(void *ptr, size_t size)
{
	void *p = NULL;

	ប្រសិនបើ (!(p = realloc(ptr, size))) {
	    printf("ERROR : xrealloc failed\n");
	    សម្រាប់(;;);
	}
	ត្រឡប់ p;
}
#endif /* __U_BOOT__ */

#ifndef __U_BOOT__
/* Make sure we have a controlling tty.  If we get started under a job
 * aware app (like bash for example), make sure we are now in charge so
 * we don't fight over who gets the foreground */
static void setup_job_control(void)
{
	static pid_t shell_pgrp;
	/* Loop until we are in the foreground.  */
	ខណៈពេល (tcgetpgrp (shell_terminal) != (shell_pgrp = getpgrp ()))
		kill (- shell_pgrp, SIGTTIN);

	/* Ignore interactive and job-control signals.  */
	sa_nkeeta(SIGINT, SIG_IGN);
	sa_nkeeta(SIGQUIT, SIG_IGN);
	sa_nkeeta(SIGTERM, SIG_IGN);
	sa_nkeeta(SIGTSTP, SIG_IGN);
	sa_nkeeta(SIGTTIN, SIG_IGN);
	sa_nkeeta(SIGTTOU, SIG_IGN);
	sa_nkeeta(SIGCHLD, SIG_IGN);

	/* Put ourselves in our own process group.  */
	setsid();
	shell_pgrp = getpid ();
	setpgid (shell_pgrp, shell_pgrp);

	/* Grab control of the terminal.  */
	tcsetpgrp(shell_terminal, shell_pgrp);
}

ចំនួនគត់ hush_main(ចំនួនគត់ argc, var_na * const *argv)
{
	ចំនួនគត់ opt;
	FILE *input;
	var_na **e = environ;

	/* XXX what should these be while sourcing /etc/profile? */
	global_argc = argc;
	global_argv = argv;

	/* (re?) initialize globals.  Sometimes hush_main() ends up calling
	 * hush_main(), therefore we cannot rely on the BSS to zero out this
	 * stuff.  Reset these to 0 every time. */
	ifs = NULL;
	/* map[] is taken care of with call to update_ifs_map() */
	fake_mode = 0;
	interactive = 0;
	close_me_head = NULL;
	last_bg_pid = 0;
	job_list = NULL;
	last_jobid = 0;

	/* Initialize some more globals to non-zero values */
	set_cwd();
#ifdef CONFIG_FEATURE_COMMAND_EDITING
	cmdedit_set_initial_prompt();
#បើមិនដូច្នេះ
	PS1 = NULL;
#endif
	PS2 = "> ";

	/* initialize our shell local variables with the values
	 * currently living in the environment */
	ប្រសិនបើ (e) {
		សម្រាប់ (; *e; e++)
			set_local_var(*e, 2);   /* without call putenv() */
	}

	last_return_code=EXIT_SUCCESS;

	ប្រសិនបើ (argv[0] && argv[0][0] == '-') {
		debug_printf("\nsourcing /etc/profile\n");
		ប្រសិនបើ ((input = fopen("/etc/profile", "r")) != NULL) {
			mark_open(fileno(input));
			parse_file_outer(input);
			mark_closed(fileno(input));
			fclose(input);
		}
	}
	input=stdin;

	ខណៈពេល ((opt = getopt(argc, argv, "c:xif")) > 0) {
		switch (opt) {
			pak_sa 'c':
				{
					global_argv = argv+optind;
					global_argc = argc-optind;
					opt = parse_string_outer(optarg, FLAG_PARSE_SEMICOLON);
					goto final_return;
				}
				bha_nga;
			pak_sa 'i':
				interactive++;
				bha_nga;
			pak_sa 'f':
				fake_mode++;
				bha_nga;
			default:
#ifndef BB_VER
				fprintf(stderr, "Usage: sh [FILE]...\n"
						"   or: sh -c command [args]...\n\n");
				exit(EXIT_FAILURE);
#បើមិនដូច្នេះ
				show_usage();
#endif
		}
	}
	/* A shell is interactive if the `-i' flag was given, or if all of
	 * the following conditions are met:
	 *	  no -c command
	 *    no arguments remaining or the -s flag given
	 *    standard input is a terminal
	 *    standard output is a terminal
	 *    Refer to Posix.2, the description of the `sh' utility. */
	ប្រសិនបើ (argv[optind]==NULL && input==stdin &&
			isatty(fileno(stdin)) && isatty(fileno(stdout))) {
		interactive++;
	}

	debug_printf("\ninteractive=%d\n", interactive);
	ប្រសិនបើ (interactive) {
		/* Looks like they want an interactive shell */
#ifndef CONFIG_FEATURE_SH_EXTRA_QUIET
		printf( "\n\n" BB_BANNER " hush - the humble shell v0.01 (testing)\n");
		printf( "Enter 'help' for a list of built-in commands.\n\n");
#endif
		setup_job_control();
	}

	ប្រសិនបើ (argv[optind]==NULL) {
		opt=parse_file_outer(stdin);
		goto final_return;
	}

	debug_printf("\nrunning script '%s'\n", argv[optind]);
	global_argv = argv+optind;
	global_argc = argc-optind;
	input = xfopen(argv[optind], "r");
	opt = parse_file_outer(input);

#ifdef CONFIG_FEATURE_CLEAN_UP
	fclose(input);
	ប្រសិនបើ (cwd && cwd != unknown)
		free((var_na*)cwd);
	{
		sa_nracanaa variables *cur, *tmp;
		សម្រាប់(cur = top_vars; cur; cur = tmp) {
			tmp = cur->next;
			ប្រសិនបើ (!cur->flg_read_only) {
				free(cur->name);
				free(cur->value);
				free(cur);
			}
		}
	}
#endif

final_return:
	ត្រឡប់(opt?opt:last_return_code);
}
#endif

static var_na *insert_var_value(var_na *inp)
{
	ត្រឡប់ insert_var_value_sub(inp, 0);
}

static var_na *insert_var_value_sub(var_na *inp, ចំនួនគត់ tag_subst)
{
	ចំនួនគត់ res_str_len = 0;
	ចំនួនគត់ len;
	ចំនួនគត់ done = 0;
	var_na *p, *p1, *res_str = NULL;

	ខណៈពេល ((p = strchr(inp, SPECIAL_VAR_SYMBOL))) {
		/* check the beginning of the string for normal characters */
		ប្រសិនបើ (p != inp) {
			/* copy any characters to the result string */
			len = p - inp;
			res_str = xrealloc(res_str, (res_str_len + len));
			strncpy((res_str + res_str_len), inp, len);
			res_str_len += len;
		}
		inp = ++p;
		/* find the ending marker */
		p = strchr(inp, SPECIAL_VAR_SYMBOL);
		*p = '\0';
		/* look up the value to substitute */
		ប្រសិនបើ ((p1 = lookup_param(inp))) {
			ប្រសិនបើ (tag_subst)
				len = res_str_len + strlen(p1) + 2;
			បើមិនដូច្នេះ
				len = res_str_len + strlen(p1);
			res_str = xrealloc(res_str, (1 + len));
			ប្រសិនបើ (tag_subst) {
				/*
				 * copy the variable value to the result
				 * string
				 */
				strcpy((res_str + res_str_len + 1), p1);

				/*
				 * mark the replaced text to be accepted as
				 * is
				 */
				res_str[res_str_len] = SUBSTED_VAR_SYMBOL;
				res_str[res_str_len + 1 + strlen(p1)] =
					SUBSTED_VAR_SYMBOL;
			} បើមិនដូច្នេះ
				/*
				 * copy the variable value to the result
				 * string
				 */
				strcpy((res_str + res_str_len), p1);

			res_str_len = len;
		}
		*p = SPECIAL_VAR_SYMBOL;
		inp = ++p;
		done = 1;
	}
	ប្រសិនបើ (done) {
		res_str = xrealloc(res_str, (1 + res_str_len + strlen(inp)));
		strcpy((res_str + res_str_len), inp);
		ខណៈពេល ((p = strchr(res_str, '\n'))) {
			*p = ' ';
		}
	}
	ត្រឡប់ (res_str == NULL) ? inp : res_str;
}

static var_na **make_list_in(var_na **inp, var_na *name)
{
	ចំនួនគត់ len, i;
	ចំនួនគត់ name_len = strlen(name);
	ចំនួនគត់ n = 0;
	var_na **list;
	var_na *p1, *p2, *p3;

	/* create list of variable values */
	list = xmalloc(parimaa_na(*list));
	សម្រាប់ (i = 0; inp[i]; i++) {
		p3 = insert_var_value(inp[i]);
		p1 = p3;
		ខណៈពេល (*p1) {
			ប្រសិនបើ (*p1 == ' ') {
				p1++;
				continue;
			}
			ប្រសិនបើ ((p2 = strchr(p1, ' '))) {
				len = p2 - p1;
			} បើមិនដូច្នេះ {
				len = strlen(p1);
				p2 = p1 + len;
			}
			/* we use n + 2 in realloc for list,because we add
			 * new element and then we will add NULL element */
			list = xrealloc(list, parimaa_na(*list) * (n + 2));
			list[n] = xmalloc(2 + name_len + len);
			strcpy(list[n], name);
			strcat(list[n], "=");
			strncat(list[n], p1, len);
			list[n++][name_len + len + 1] = '\0';
			p1 = p2;
		}
		ប្រសិនបើ (p3 != inp[i]) free(p3);
	}
	list[n] = NULL;
	ត្រឡប់ list;
}

/*
 * Make new string for parser
 * inp     - array of argument strings to flatten
 * nonnull - indicates argument was quoted when originally parsed
 */
static var_na *make_string(var_na **inp, ចំនួនគត់ *nonnull)
{
	var_na *p;
	var_na *str = NULL;
	ចំនួនគត់ n;
	ចំនួនគត់ len = 2;
	var_na *noeval_str;
	ចំនួនគត់ noeval = 0;

	noeval_str = get_local_var("HUSH_NO_EVAL");
	ប្រសិនបើ (noeval_str != NULL && *noeval_str != '0' && *noeval_str != '\0')
		noeval = 1;
	សម្រាប់ (n = 0; inp[n]; n++) {
		p = insert_var_value_sub(inp[n], noeval);
		var_na *new_str = xrealloc(str, (len + strlen(p) + (2 * nonnull[n])));
		ប្រសិនបើ (!new_str) {
			free(str);
			ប្រសិនបើ (p != inp[n]) free(p);
			ត្រឡប់ NULL;
		}
		str = new_str;
		ប្រសិនបើ (n) {
			strcat(str, " ");
		} បើមិនដូច្នេះ {
			*str = '\0';
		}
		ប្រសិនបើ (nonnull[n])
			strcat(str, "'");
		strcat(str, p);
		ប្រសិនបើ (nonnull[n])
			strcat(str, "'");
		len = strlen(str) + 3;
		ប្រសិនបើ (p != inp[n]) free(p);
	}
	len = strlen(str);
	*(str + len) = '\n';
	*(str + len + 1) = '\0';
	ត្រឡប់ str;
}

#ifdef __U_BOOT__
static ចំនួនគត់ do_showvar(sa_nracanaa cmd_tbl *cmdtp, ចំនួនគត់ flag, ចំនួនគត់ argc,
		      var_na *const argv[])
{
	ចំនួនគត់ i, k;
	ចំនួនគត់ rcode = 0;
	sa_nracanaa variables *cur;

	ប្រសិនបើ (argc == 1) {		/* Print all env variables	*/
		សម្រាប់ (cur = top_vars; cur; cur = cur->next) {
			printf ("%s=%s\n", cur->name, cur->value);
			ប្រសិនបើ (ctrlc ()) {
				puts ("\n ** Abort\n");
				ត្រឡប់ 1;
			}
		}
		ត្រឡប់ 0;
	}
	សម្រាប់ (i = 1; i < argc; ++i) {	/* print single env variables	*/
		var_na *name = argv[i];

		k = -1;
		សម្រាប់ (cur = top_vars; cur; cur = cur->next) {
			ប្រសិនបើ(strcmp (cur->name, name) == 0) {
				k = 0;
				printf ("%s=%s\n", cur->name, cur->value);
			}
			ប្រសិនបើ (ctrlc ()) {
				puts ("\n ** Abort\n");
				ត្រឡប់ 1;
			}
		}
		ប្រសិនបើ (k < 0) {
			printf ("## Error: \"%s\" not defined\n", name);
			rcode ++;
		}
	}
	ត្រឡប់ rcode;
}

U_BOOT_CMD(
	showvar, CONFIG_SYS_MAXARGS, 1,	do_showvar,
	"print local hushshell variables",
	"\n    - print values of all hushshell variables\n"
	"showvar name ...\n"
	"    - print value of hushshell variable 'name'"
);

#endif
/****************************************************************************/
