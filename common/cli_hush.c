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

#ᮚᮦᮔ᮪ 1
#include "busybox.h"
#include "cmdedit.h"
#ᮜᮤᮚ
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
#ᮜᮤᮚ
typedef ga_nanaa {
	REDIRECT_INPUT     = 1,
	REDIRECT_OVERWRITE = 2,
	REDIRECT_APPEND    = 3,
	REDIRECT_HEREIS    = 4,
	REDIRECT_IO        = 5
} redir_type;

/* The descrip member of this structure is only used to make debugging
 * output pretty */
sa_nracanaa {puur_naa_nka mode; puur_naa_nka default_fd; var_na *descrip;} redir_table[] = {
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
	puur_naa_nka old_flag;				/* for figuring out valid reserved words */
	sa_nracanaa p_context *stack;
	puur_naa_nka type;			/* define type of parser : ";$" common or special symbol */
	/* How about quoting status? */
};

#ifndef __U_BOOT__
sa_nracanaa redir_struct {
	redir_type type;			/* type of redirection */
	puur_naa_nka fd;						/* file descriptor being redirected */
	puur_naa_nka dup;					/* -1, or file descriptor being duplicated */
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
	puur_naa_nka *argv_nonnull;
#ifdef __U_BOOT__
	puur_naa_nka    argc;                            /* number of program arguments */
#endif
	sa_nracanaa pipe *group;			/* if non-NULL, first in group or subshell */
#ifndef __U_BOOT__
	puur_naa_nka subshell;				/* flag, non-zero if group must be forked */
	sa_nracanaa redir_struct *redirects;	/* I/O redirections */
	glob_t glob_result;			/* result of parameter globbing */
	puur_naa_nka is_stopped;				/* is the program currently running? */
	sa_nracanaa pipe *family;		/* pointer back to the child's parent pipe */
#endif
	puur_naa_nka sp;				/* number of SPECIAL_VAR_SYMBOL */
	puur_naa_nka type;
};

sa_nracanaa pipe {
#ifndef __U_BOOT__
	puur_naa_nka jobid;					/* job number */
#endif
	puur_naa_nka num_progs;				/* total number of programs in job */
#ifndef __U_BOOT__
	puur_naa_nka running_progs;			/* number of programs running */
	var_na *text;					/* name of job */
	var_na *cmdbuf;				/* buffer various argv's point into */
	pid_t pgrp;					/* process group ID for the job */
#endif
	sa_nracanaa child_prog *progs;	/* array of commands in pipe */
	sa_nracanaa pipe *next;			/* to track background commands */
#ifndef __U_BOOT__
	puur_naa_nka stopped_progs;			/* number of programs alive, but stopped */
	puur_naa_nka job_context;			/* bitmask defining current context */
#endif
	pipe_style followup;		/* PIPE_BG, PIPE_SEQ, PIPE_OR, PIPE_AND */
	reserved_style r_mode;		/* supports if, for, while, until */
};

#ifndef __U_BOOT__
sa_nracanaa close_me {
	puur_naa_nka fd;
	sa_nracanaa close_me *next;
};
#endif

sa_nracanaa variables {
	var_na *name;
	var_na *value;
	puur_naa_nka flg_export;
	puur_naa_nka flg_read_only;
	sa_nracanaa variables *next;
};

/* globals, connect us to the outside world
 * the first three support $?, $#, and $1 */
#ifndef __U_BOOT__
var_na **global_argv;
unsigned puur_naa_nka global_argc;
#endif
static unsigned puur_naa_nka last_return_code;
#ifndef __U_BOOT__
extern var_na **environ; /* This is in <unistd.h>, but protected with __USE_GNU */
#endif

/* "globals" within this file */
static uchar *ifs;
static var_na map[256];
#ifndef __U_BOOT__
static puur_naa_nka fake_mode;
static puur_naa_nka interactive;
static sa_nracanaa close_me *close_me_head;
static const var_na *cwd;
static sa_nracanaa pipe *job_list;
static unsigned puur_naa_nka last_bg_pid;
static unsigned puur_naa_nka last_jobid;
static unsigned puur_naa_nka shell_terminal;
static var_na *PS1;
static var_na *PS2;
sa_nracanaa variables shell_ver = { "HUSH_VERSION", "0.01", 1, 1, 0 };
sa_nracanaa variables *top_vars = &shell_ver;
#ᮜᮤᮚ
static puur_naa_nka flag_repeat = 0;
static puur_naa_nka do_repeat = 0;
static sa_nracanaa variables *top_vars = NULL ;
#endif /*__U_BOOT__ */

#define B_CHUNK (100)
#define B_NOSPAC 1

typedef sa_nracanaa {
	var_na *data;
	puur_naa_nka length;
	puur_naa_nka maxlen;
	puur_naa_nka quote;
	puur_naa_nka nonnull;
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
	puur_naa_nka __promptme;
	puur_naa_nka promptmode;
#ifndef __U_BOOT__
	FILE *file;
#endif
	puur_naa_nka (*get) (sa_nracanaa in_str *);
	puur_naa_nka (*peek) (sa_nracanaa in_str *);
};
#define b_getch(input) ((input)->get(input))
#define b_peek(input) ((input)->peek(input))

#ifndef __U_BOOT__
#define JOB_STATUS_FORMAT "[%d] %-22s %.40s\n"

sa_nracanaa built_in_command {
	var_na *cmd;					/* name */
	var_na *descr;				/* description */
	puur_naa_nka (*function) (sa_nracanaa child_prog *);	/* function ptr */
};
#endif

/* define DEBUG_SHELL for debugging output (obviously ;-)) */
#ᮚᮦᮔ᮪ 0
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
#ᮜᮤᮚ
#define debug_printf(fmt,args...)	printf (fmt ,##args)
#endif
#ᮜᮤᮚ
static anta_hstha void debug_printf(const var_na *format, ...) { }
#endif
#define final_printf debug_printf

#ifdef __U_BOOT__
static void syntax_err(void) {
	 printf("syntax error\n");
}
#ᮜᮤᮚ
static void __syntax(var_na *file, puur_naa_nka line) {
	error_msg("syntax error %s:%d", file, line);
}
#define syntax() __syntax(__FILE__, __LINE__)
#endif

#ifdef __U_BOOT__
static void *xmalloc(size_t size);
static void *xrealloc(void *ptr, size_t size);
#ᮜᮤᮚ
/* Index of subroutines: */
/*   function prototypes for builtins */
static puur_naa_nka builtin_cd(sa_nracanaa child_prog *child);
static puur_naa_nka builtin_env(sa_nracanaa child_prog *child);
static puur_naa_nka builtin_eval(sa_nracanaa child_prog *child);
static puur_naa_nka builtin_exec(sa_nracanaa child_prog *child);
static puur_naa_nka builtin_exit(sa_nracanaa child_prog *child);
static puur_naa_nka builtin_export(sa_nracanaa child_prog *child);
static puur_naa_nka builtin_fg_bg(sa_nracanaa child_prog *child);
static puur_naa_nka builtin_help(sa_nracanaa child_prog *child);
static puur_naa_nka builtin_jobs(sa_nracanaa child_prog *child);
static puur_naa_nka builtin_pwd(sa_nracanaa child_prog *child);
static puur_naa_nka builtin_read(sa_nracanaa child_prog *child);
static puur_naa_nka builtin_set(sa_nracanaa child_prog *child);
static puur_naa_nka builtin_shift(sa_nracanaa child_prog *child);
static puur_naa_nka builtin_source(sa_nracanaa child_prog *child);
static puur_naa_nka builtin_umask(sa_nracanaa child_prog *child);
static puur_naa_nka builtin_unset(sa_nracanaa child_prog *child);
static puur_naa_nka builtin_not_written(sa_nracanaa child_prog *child);
#endif
/*   o_string manipulation: */
static puur_naa_nka b_check_space(o_string *o, puur_naa_nka len);
static puur_naa_nka b_addchr(o_string *o, puur_naa_nka ch);
static void b_reset(o_string *o);
static puur_naa_nka b_addqchr(o_string *o, puur_naa_nka ch, puur_naa_nka quote);
#ifndef __U_BOOT__
static puur_naa_nka b_adduint(o_string *o, unsigned puur_naa_nka i);
#endif
/*  in_str manipulations: */
static puur_naa_nka static_get(sa_nracanaa in_str *i);
static puur_naa_nka static_peek(sa_nracanaa in_str *i);
static puur_naa_nka file_get(sa_nracanaa in_str *i);
static puur_naa_nka file_peek(sa_nracanaa in_str *i);
#ifndef __U_BOOT__
static void setup_file_in_str(sa_nracanaa in_str *i, FILE *f);
#ᮜᮤᮚ
static void setup_file_in_str(sa_nracanaa in_str *i);
#endif
static void setup_string_in_str(sa_nracanaa in_str *i, const var_na *s);
#ifndef __U_BOOT__
/*  close_me manipulations: */
static void mark_open(puur_naa_nka fd);
static void mark_closed(puur_naa_nka fd);
static void close_all(void);
#endif
/*  "run" the final data structures: */
static var_na *indenter(puur_naa_nka i);
static puur_naa_nka free_pipe_list(sa_nracanaa pipe *head, puur_naa_nka indent);
static puur_naa_nka free_pipe(sa_nracanaa pipe *pi, puur_naa_nka indent);
/*  really run the final data structures: */
#ifndef __U_BOOT__
static puur_naa_nka setup_redirects(sa_nracanaa child_prog *prog, puur_naa_nka squirrel[]);
#endif
static puur_naa_nka run_list_real(sa_nracanaa pipe *pi);
#ifndef __U_BOOT__
static void pseudo_exec(sa_nracanaa child_prog *child) __attribute__ ((noreturn));
#endif
static puur_naa_nka run_pipe_real(sa_nracanaa pipe *pi);
/*   extended glob support: */
#ifndef __U_BOOT__
static puur_naa_nka globhack(const var_na *src, puur_naa_nka flags, glob_t *pglob);
static puur_naa_nka glob_needed(const var_na *s);
static puur_naa_nka xglob(o_string *dest, puur_naa_nka flags, glob_t *pglob);
#endif
/*   variable assignment: */
static puur_naa_nka is_assignment(const var_na *s);
/*   data structure manipulation: */
#ifndef __U_BOOT__
static puur_naa_nka setup_redirect(sa_nracanaa p_context *ctx, puur_naa_nka fd, redir_type style, sa_nracanaa in_str *input);
#endif
static void initialize_context(sa_nracanaa p_context *ctx);
static puur_naa_nka done_word(o_string *dest, sa_nracanaa p_context *ctx);
static puur_naa_nka done_command(sa_nracanaa p_context *ctx);
static puur_naa_nka done_pipe(sa_nracanaa p_context *ctx, pipe_style type);
/*   primary string parsing: */
#ifndef __U_BOOT__
static puur_naa_nka redirect_dup_num(sa_nracanaa in_str *input);
static puur_naa_nka redirect_opt_num(o_string *o);
static puur_naa_nka process_command_subs(o_string *dest, sa_nracanaa p_context *ctx, sa_nracanaa in_str *input, puur_naa_nka subst_end);
static puur_naa_nka parse_group(o_string *dest, sa_nracanaa p_context *ctx, sa_nracanaa in_str *input, puur_naa_nka ch);
#endif
static var_na *lookup_param(var_na *src);
static var_na *make_string(var_na **inp, puur_naa_nka *nonnull);
static puur_naa_nka handle_dollar(o_string *dest, sa_nracanaa p_context *ctx, sa_nracanaa in_str *input);
#ifndef __U_BOOT__
static puur_naa_nka parse_string(o_string *dest, sa_nracanaa p_context *ctx, const var_na *src);
#endif
static puur_naa_nka parse_stream(o_string *dest, sa_nracanaa p_context *ctx, sa_nracanaa in_str *input0, puur_naa_nka end_trigger);
/*   setup: */
static puur_naa_nka parse_stream_outer(sa_nracanaa in_str *inp, puur_naa_nka flag);
#ifndef __U_BOOT__
static puur_naa_nka parse_string_outer(const var_na *s, puur_naa_nka flag);
static puur_naa_nka parse_file_outer(FILE *f);
#endif
#ifndef __U_BOOT__
/*   job management: */
static puur_naa_nka checkjobs(sa_nracanaa pipe* fg_pipe);
static void insert_bg_job(sa_nracanaa pipe *pi);
static void remove_bg_job(sa_nracanaa pipe *pi);
#endif
/*     local variable support */
static var_na **make_list_in(var_na **inp, var_na *name);
static var_na *insert_var_value(var_na *inp);
static var_na *insert_var_value_sub(var_na *inp, puur_naa_nka tag_subst);

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
	ᮚᮦᮔ᮪(cwd==unknown)
		cwd = NULL;     /* xgetcwd(arg) called free(arg) */
	cwd = xgetcwd((var_na *)cwd);
	ᮚᮦᮔ᮪ (!cwd)
		cwd = unknown;
	ᮝᮜᮤ cwd;
}

/* built-in 'eval' handler */
static puur_naa_nka builtin_eval(sa_nracanaa child_prog *child)
{
	var_na *str = NULL;
	puur_naa_nka rcode = EXIT_SUCCESS;

	ᮚᮦᮔ᮪ (child->argv[1]) {
		str = make_string(child->argv + 1);
		parse_string_outer(str, FLAG_EXIT_FROM_LOOP |
					FLAG_PARSE_SEMICOLON);
		free(str);
		rcode = last_return_code;
	}
	ᮝᮜᮤ rcode;
}

/* built-in 'cd <path>' handler */
static puur_naa_nka builtin_cd(sa_nracanaa child_prog *child)
{
	var_na *newdir;
	ᮚᮦᮔ᮪ (child->argv[1] == NULL)
		newdir = env_get("HOME");
	ᮜᮤᮚ
		newdir = child->argv[1];
	ᮚᮦᮔ᮪ (chdir(newdir)) {
		printf("cd: %s: %s\n", newdir, strerror(errno));
		ᮝᮜᮤ EXIT_FAILURE;
	}
	set_cwd();
	ᮝᮜᮤ EXIT_SUCCESS;
}

/* built-in 'env' handler */
static puur_naa_nka builtin_env(sa_nracanaa child_prog *dummy)
{
	var_na **e = environ;
	ᮚᮦᮔ᮪ (e == NULL) ᮝᮜᮤ EXIT_FAILURE;
	for (; *e; e++) {
		puts(*e);
	}
	ᮝᮜᮤ EXIT_SUCCESS;
}

/* built-in 'exec' handler */
static puur_naa_nka builtin_exec(sa_nracanaa child_prog *child)
{
	ᮚᮦᮔ᮪ (child->argv[1] == NULL)
		ᮝᮜᮤ EXIT_SUCCESS;   /* Really? */
	child->argv++;
	pseudo_exec(child);
	/* never returns */
}

/* built-in 'exit' handler */
static puur_naa_nka builtin_exit(sa_nracanaa child_prog *child)
{
	ᮚᮦᮔ᮪ (child->argv[1] == NULL)
		exit(last_return_code);
	exit (atoi(child->argv[1]));
}

/* built-in 'export VAR=value' handler */
static puur_naa_nka builtin_export(sa_nracanaa child_prog *child)
{
	puur_naa_nka res = 0;
	var_na *name = child->argv[1];

	ᮚᮦᮔ᮪ (name == NULL) {
		ᮝᮜᮤ (builtin_env(child));
	}

	name = strdup(name);

	ᮚᮦᮔ᮪(name) {
		var_na *value = strchr(name, '=');

		ᮚᮦᮔ᮪ (!value) {
			var_na *tmp;
			/* They are exporting something without an =VALUE */

			value = get_local_var(name);
			ᮚᮦᮔ᮪ (value) {
				size_t ln = strlen(name);

				tmp = realloc(name, ln+strlen(value)+2);
				ᮚᮦᮔ᮪(tmp==NULL)
					res = -1;
				ᮜᮤᮚ {
					sprintf(tmp+ln, "=%s", value);
					name = tmp;
				}
			} ᮜᮤᮚ {
				/* bash does not return an error when trying to export
				 * an undefined variable.  Do likewise. */
				res = 1;
			}
		}
	}
	ᮚᮦᮔ᮪ (res<0)
		perror_msg("export");
	ᮜᮤᮚ ᮚᮦᮔ᮪(res==0)
		res = set_local_var(name, 1);
	ᮜᮤᮚ
		res = 0;
	free(name);
	ᮝᮜᮤ res;
}

/* built-in 'fg' and 'bg' handler */
static puur_naa_nka builtin_fg_bg(sa_nracanaa child_prog *child)
{
	puur_naa_nka i, jobnum;
	sa_nracanaa pipe *pi=NULL;

	ᮚᮦᮔ᮪ (!interactive)
		ᮝᮜᮤ EXIT_FAILURE;
	/* If they gave us no args, assume they want the last backgrounded task */
	ᮚᮦᮔ᮪ (!child->argv[1]) {
		for (pi = job_list; pi; pi = pi->next) {
			ᮚᮦᮔ᮪ (pi->jobid == last_jobid) {
				bha_nga;
			}
		}
		ᮚᮦᮔ᮪ (!pi) {
			error_msg("%s: no current job", child->argv[0]);
			ᮝᮜᮤ EXIT_FAILURE;
		}
	} ᮜᮤᮚ {
		ᮚᮦᮔ᮪ (sscanf(child->argv[1], "%%%d", &jobnum) != 1) {
			error_msg("%s: bad argument '%s'", child->argv[0], child->argv[1]);
			ᮝᮜᮤ EXIT_FAILURE;
		}
		for (pi = job_list; pi; pi = pi->next) {
			ᮚᮦᮔ᮪ (pi->jobid == jobnum) {
				bha_nga;
			}
		}
		ᮚᮦᮔ᮪ (!pi) {
			error_msg("%s: %d: no such job", child->argv[0], jobnum);
			ᮝᮜᮤ EXIT_FAILURE;
		}
	}

	ᮚᮦᮔ᮪ (*child->argv[0] == 'f') {
		/* Put the job into the foreground.  */
		tcsetpgrp(shell_terminal, pi->pgrp);
	}

	/* Restart the processes in the job */
	for (i = 0; i < pi->num_progs; i++)
		pi->progs[i].is_stopped = 0;

	ᮚᮦᮔ᮪ ( (i=kill(- pi->pgrp, SIGCONT)) < 0) {
		ᮚᮦᮔ᮪ (i == ESRCH) {
			remove_bg_job(pi);
		} ᮜᮤᮚ {
			perror_msg("kill (SIGCONT)");
		}
	}

	pi->stopped_progs = 0;
	ᮝᮜᮤ EXIT_SUCCESS;
}

/* built-in 'help' handler */
static puur_naa_nka builtin_help(sa_nracanaa child_prog *dummy)
{
	sa_nracanaa built_in_command *x;

	printf("\nBuilt-in commands:\n");
	printf("-------------------\n");
	for (x = bltins; x->cmd; x++) {
		ᮚᮦᮔ᮪ (x->descr==NULL)
			continue;
		printf("%s\t%s\n", x->cmd, x->descr);
	}
	printf("\n\n");
	ᮝᮜᮤ EXIT_SUCCESS;
}

/* built-in 'jobs' handler */
static puur_naa_nka builtin_jobs(sa_nracanaa child_prog *child)
{
	sa_nracanaa pipe *job;
	var_na *status_string;

	for (job = job_list; job; job = job->next) {
		ᮚᮦᮔ᮪ (job->running_progs == job->stopped_progs)
			status_string = "Stopped";
		ᮜᮤᮚ
			status_string = "Running";

		printf(JOB_STATUS_FORMAT, job->jobid, status_string, job->text);
	}
	ᮝᮜᮤ EXIT_SUCCESS;
}

/* built-in 'pwd' handler */
static puur_naa_nka builtin_pwd(sa_nracanaa child_prog *dummy)
{
	puts(set_cwd());
	ᮝᮜᮤ EXIT_SUCCESS;
}

/* built-in 'read VAR' handler */
static puur_naa_nka builtin_read(sa_nracanaa child_prog *child)
{
	puur_naa_nka res;

	ᮚᮦᮔ᮪ (child->argv[1]) {
		var_na string[BUFSIZ];
		var_na *var = 0;

		string[0] = 0;  /* In case stdin has only EOF */
		/* read string */
		fgets(string, parimaa_na(string), stdin);
		chomp(string);
		var = malloc(strlen(child->argv[1])+strlen(string)+2);
		ᮚᮦᮔ᮪(var) {
			sprintf(var, "%s=%s", child->argv[1], string);
			res = set_local_var(var, 0);
		} ᮜᮤᮚ
			res = -1;
		ᮚᮦᮔ᮪ (res)
			fprintf(stderr, "read: %m\n");
		free(var);      /* So not move up to avoid breaking errno */
		ᮝᮜᮤ res;
	} ᮜᮤᮚ {
		do res=getchar(); ᮞᮨᮓᮨᮜ(res!='\n' && res!=EOF);
		ᮝᮜᮤ 0;
	}
}

/* built-in 'set VAR=value' handler */
static puur_naa_nka builtin_set(sa_nracanaa child_prog *child)
{
	var_na *temp = child->argv[1];
	sa_nracanaa variables *e;

	ᮚᮦᮔ᮪ (temp == NULL)
		for(e = top_vars; e; e=e->next)
			printf("%s=%s\n", e->name, e->value);
	ᮜᮤᮚ
		set_local_var(temp, 0);

		ᮝᮜᮤ EXIT_SUCCESS;
}

/* Built-in 'shift' handler */
static puur_naa_nka builtin_shift(sa_nracanaa child_prog *child)
{
	puur_naa_nka n=1;
	ᮚᮦᮔ᮪ (child->argv[1]) {
		n=atoi(child->argv[1]);
	}
	ᮚᮦᮔ᮪ (n>=0 && n<global_argc) {
		/* XXX This probably breaks $0 */
		global_argc -= n;
		global_argv += n;
		ᮝᮜᮤ EXIT_SUCCESS;
	} ᮜᮤᮚ {
		ᮝᮜᮤ EXIT_FAILURE;
	}
}

/* Built-in '.' handler (read-in and execute commands from file) */
static puur_naa_nka builtin_source(sa_nracanaa child_prog *child)
{
	FILE *input;
	puur_naa_nka status;

	ᮚᮦᮔ᮪ (child->argv[1] == NULL)
		ᮝᮜᮤ EXIT_FAILURE;

	/* XXX search through $PATH is missing */
	input = fopen(child->argv[1], "r");
	ᮚᮦᮔ᮪ (!input) {
		error_msg("Couldn't open file '%s'", child->argv[1]);
		ᮝᮜᮤ EXIT_FAILURE;
	}

	/* Now run the file */
	/* XXX argv and argc are broken; need to save old global_argv
	 * (pointer only is OK!) on this stack frame,
	 * set global_argv=child->argv+1, recurse, and restore. */
	mark_open(fileno(input));
	status = parse_file_outer(input);
	mark_closed(fileno(input));
	fclose(input);
	ᮝᮜᮤ (status);
}

static puur_naa_nka builtin_umask(sa_nracanaa child_prog *child)
{
	mode_t new_umask;
	const var_na *arg = child->argv[1];
	var_na *end;
	ᮚᮦᮔ᮪ (arg) {
		new_umask=strtoul(arg, &end, 8);
		ᮚᮦᮔ᮪ (*end!='\0' || end == arg) {
			ᮝᮜᮤ EXIT_FAILURE;
		}
	} ᮜᮤᮚ {
		printf("%.3o\n", (unsigned puur_naa_nka) (new_umask=umask(0)));
	}
	umask(new_umask);
	ᮝᮜᮤ EXIT_SUCCESS;
}

/* built-in 'unset VAR' handler */
static puur_naa_nka builtin_unset(sa_nracanaa child_prog *child)
{
	/* bash returned already true */
	unset_local_var(child->argv[1]);
	ᮝᮜᮤ EXIT_SUCCESS;
}

static puur_naa_nka builtin_not_written(sa_nracanaa child_prog *child)
{
	printf("builtin_%s not written\n",child->argv[0]);
	ᮝᮜᮤ EXIT_FAILURE;
}
#endif

static puur_naa_nka b_check_space(o_string *o, puur_naa_nka len)
{
	/* It would be easy to drop a more restrictive policy
	 * in here, such as setting a maximum string length */
	ᮚᮦᮔ᮪ (o->length + len > o->maxlen) {
		var_na *old_data = o->data;
		/* assert (data == NULL || o->maxlen != 0); */
		o->maxlen += max(2*len, B_CHUNK);
		o->data = realloc(o->data, 1 + o->maxlen);
		ᮚᮦᮔ᮪ (o->data == NULL) {
			free(old_data);
		}
	}
	ᮝᮜᮤ o->data == NULL;
}

static puur_naa_nka b_addchr(o_string *o, puur_naa_nka ch)
{
	debug_printf("b_addchr: %c %d %p\n", ch, o->length, o);
	ᮚᮦᮔ᮪ (b_check_space(o, 1)) ᮝᮜᮤ B_NOSPAC;
	o->data[o->length] = ch;
	o->length++;
	o->data[o->length] = '\0';
	ᮝᮜᮤ 0;
}

static void b_reset(o_string *o)
{
	o->length = 0;
	o->nonnull = 0;
	ᮚᮦᮔ᮪ (o->data != NULL) *o->data = '\0';
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
static puur_naa_nka b_addqchr(o_string *o, puur_naa_nka ch, puur_naa_nka quote)
{
	ᮚᮦᮔ᮪ (quote && strchr("*?[\\",ch)) {
		puur_naa_nka rc;
		rc = b_addchr(o, '\\');
		ᮚᮦᮔ᮪ (rc) ᮝᮜᮤ rc;
	}
	ᮝᮜᮤ b_addchr(o, ch);
}

#ifndef __U_BOOT__
static puur_naa_nka b_adduint(o_string *o, unsigned puur_naa_nka i)
{
	puur_naa_nka r;
	var_na *p = simple_itoa(i);
	/* no escape checking necessary */
	do r=b_addchr(o, *p++); ᮞᮨᮓᮨᮜ (r==0 && *p);
	ᮝᮜᮤ r;
}
#endif

static puur_naa_nka static_get(sa_nracanaa in_str *i)
{
	puur_naa_nka ch = *i->p++;
	ᮚᮦᮔ᮪ (ch=='\0') ᮝᮜᮤ EOF;
	ᮝᮜᮤ ch;
}

static puur_naa_nka static_peek(sa_nracanaa in_str *i)
{
	ᮝᮜᮤ *i->p;
}

#ifndef __U_BOOT__
static anta_hstha void cmdedit_set_initial_prompt(void)
{
#ifndef CONFIG_FEATURE_SH_FANCY_PROMPT
	PS1 = NULL;
#ᮜᮤᮚ
	PS1 = env_get("PS1");
	ᮚᮦᮔ᮪(PS1==0)
		PS1 = "\\w \\$ ";
#endif
}

static anta_hstha void setup_prompt_string(puur_naa_nka promptmode, var_na **prompt_str)
{
	debug_printf("setup_prompt_string %d ",promptmode);
#ifndef CONFIG_FEATURE_SH_FANCY_PROMPT
	/* Set up the prompt */
	ᮚᮦᮔ᮪ (promptmode == 1) {
		free(PS1);
		PS1=xmalloc(strlen(cwd)+4);
		sprintf(PS1, "%s %s", cwd, ( geteuid() != 0 ) ?  "$ ":"# ");
		*prompt_str = PS1;
	} ᮜᮤᮚ {
		*prompt_str = PS2;
	}
#ᮜᮤᮚ
	*prompt_str = (promptmode==1)? PS1 : PS2;
#endif
	debug_printf("result %s\n",*prompt_str);
}
#endif

#ifdef __U_BOOT__
static puur_naa_nka uboot_cli_readline(sa_nracanaa in_str *i)
{
	var_na *prompt;
	var_na __maybe_unused *ps_prompt = NULL;

	ᮚᮦᮔ᮪ (i->promptmode == 1)
		prompt = CONFIG_SYS_PROMPT;
	ᮜᮤᮚ
		prompt = CONFIG_SYS_PROMPT_HUSH_PS2;

#ifdef CONFIG_CMDLINE_PS_SUPPORT
	ᮚᮦᮔ᮪ (i->promptmode == 1)
		ps_prompt = env_get("PS1");
	ᮜᮤᮚ
		ps_prompt = env_get("PS2");
	ᮚᮦᮔ᮪ (ps_prompt)
		prompt = ps_prompt;
#endif

	ᮝᮜᮤ cli_readline(prompt);
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
#ᮜᮤᮚ
	fputs(prompt_str, stdout);
	fflush(stdout);
	the_command[0]=fgetc(i->file);
	the_command[1]='\0';
#endif
	fflush(stdout);
	i->p = the_command;
#ᮜᮤᮚ
	puur_naa_nka n;
	static var_na the_command[CONFIG_SYS_CBSIZE + 1];

	bootretry_reset_cmd_timeout();
	i->__promptme = 1;
	n = uboot_cli_readline(i);

#ifdef CONFIG_BOOT_RETRY_TIME
	ᮚᮦᮔ᮪ (n == -2) {
	  puts("\nTimeout waiting for command\n");
#  ifdef CONFIG_RESET_TO_RETRY
	  do_reset(NULL, 0, 0, NULL);
#  elif IS_ENABLED(CONFIG_RETRY_BOOTCMD)
	strcpy(console_buffer, "run bootcmd\n");
# ᮜᮤᮚ
#	error "This only works with CONFIG_RESET_TO_RETRY or CONFIG_BOOT_RETRY_COMMAND enabled"
#  endif
	}
#endif
	ᮚᮦᮔ᮪ (n == -1 ) {
		flag_repeat = 0;
		i->__promptme = 0;
	}
	n = strlen(console_buffer);
	console_buffer[n] = '\n';
	console_buffer[n+1]= '\0';
	ᮚᮦᮔ᮪ (had_ctrlc()) flag_repeat = 0;
	clear_ctrlc();
	do_repeat = 0;
	ᮚᮦᮔ᮪ (i->promptmode == 1) {
		ᮚᮦᮔ᮪ (console_buffer[0] == '\n'&& flag_repeat == 0) {
			strcpy(the_command,console_buffer);
		}
		ᮜᮤᮚ {
			ᮚᮦᮔ᮪ (console_buffer[0] != '\n') {
				strcpy(the_command,console_buffer);
				flag_repeat = 1;
			}
			ᮜᮤᮚ {
				do_repeat = 1;
			}
		}
		i->p = the_command;
	}
	ᮜᮤᮚ {
		ᮚᮦᮔ᮪ (console_buffer[0] != '\n') {
			ᮚᮦᮔ᮪ (strlen(the_command) + strlen(console_buffer)
			    < CONFIG_SYS_CBSIZE) {
				n = strlen(the_command);
				the_command[n-1] = ' ';
				strcpy(&the_command[n],console_buffer);
			}
			ᮜᮤᮚ {
				the_command[0] = '\n';
				the_command[1] = '\0';
				flag_repeat = 0;
			}
		}
		ᮚᮦᮔ᮪ (i->__promptme == 0) {
			the_command[0] = '\n';
			the_command[1] = '\0';
		}
		i->p = console_buffer;
	}
#endif
}

/* This is the magic location that prints prompts
 * and gets data back from the user */
static puur_naa_nka file_get(sa_nracanaa in_str *i)
{
	puur_naa_nka ch;

	ch = 0;
	/* If there is data waiting, eat it up */
	ᮚᮦᮔ᮪ (i->p && *i->p) {
		ch = *i->p++;
	} ᮜᮤᮚ {
		/* need to double check i->file because we might be doing something
		 * more complicated by now, like sourcing or substituting. */
#ifndef __U_BOOT__
		ᮚᮦᮔ᮪ (i->__promptme && interactive && i->file == stdin) {
			ᮞᮨᮓᮨᮜ(! i->p || (interactive && strlen(i->p)==0) ) {
#ᮜᮤᮚ
			ᮞᮨᮓᮨᮜ(! i->p  || strlen(i->p)==0 ) {
#endif
				get_user_input(i);
			}
			i->promptmode=2;
#ifndef __U_BOOT__
			i->__promptme = 0;
#endif
			ᮚᮦᮔ᮪ (i->p && *i->p) {
				ch = *i->p++;
			}
#ifndef __U_BOOT__
		} ᮜᮤᮚ {
			ch = fgetc(i->file);
		}

#endif
		debug_printf("b_getch: got a %d\n", ch);
	}
#ifndef __U_BOOT__
	ᮚᮦᮔ᮪ (ch == '\n') i->__promptme=1;
#endif
	ᮝᮜᮤ ch;
}

/* All the callers guarantee this routine will never be
 * used right after a newline, so prompting is not needed.
 */
static puur_naa_nka file_peek(sa_nracanaa in_str *i)
{
#ifndef __U_BOOT__
	ᮚᮦᮔ᮪ (i->p && *i->p) {
#endif
		ᮝᮜᮤ *i->p;
#ifndef __U_BOOT__
	} ᮜᮤᮚ {
		i->peek_buf[0] = fgetc(i->file);
		i->peek_buf[1] = '\0';
		i->p = i->peek_buf;
		debug_printf("b_peek: got a %d\n", *i->p);
		ᮝᮜᮤ *i->p;
	}
#endif
}

#ifndef __U_BOOT__
static void setup_file_in_str(sa_nracanaa in_str *i, FILE *f)
#ᮜᮤᮚ
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
static void mark_open(puur_naa_nka fd)
{
	sa_nracanaa close_me *new = xmalloc(parimaa_na(sa_nracanaa close_me));
	new->fd = fd;
	new->next = close_me_head;
	close_me_head = new;
}

static void mark_closed(puur_naa_nka fd)
{
	sa_nracanaa close_me *tmp;
	ᮚᮦᮔ᮪ (close_me_head == NULL || close_me_head->fd != fd)
		error_msg_and_die("corrupt close_me");
	tmp = close_me_head;
	close_me_head = close_me_head->next;
	free(tmp);
}

static void close_all(void)
{
	sa_nracanaa close_me *c;
	for (c=close_me_head; c; c=c->next) {
		close(c->fd);
	}
	close_me_head = NULL;
}

/* squirrel != NULL means we squirrel away copies of stdin, stdout,
 * and stderr if they are redirected. */
static puur_naa_nka setup_redirects(sa_nracanaa child_prog *prog, puur_naa_nka squirrel[])
{
	puur_naa_nka openfd, mode;
	sa_nracanaa redir_struct *redir;

	for (redir=prog->redirects; redir; redir=redir->next) {
		ᮚᮦᮔ᮪ (redir->dup == -1 && redir->word.gl_pathv == NULL) {
			/* something went wrong in the parse.  Pretend it didn't happen */
			continue;
		}
		ᮚᮦᮔ᮪ (redir->dup == -1) {
			mode=redir_table[redir->type].mode;
			openfd = open(redir->word.gl_pathv[0], mode, 0666);
			ᮚᮦᮔ᮪ (openfd < 0) {
			/* this could get lost if stderr has been redirected, but
			   bash and ash both lose it as well (though zsh doesn't!) */
				perror_msg("error opening %s", redir->word.gl_pathv[0]);
				ᮝᮜᮤ 1;
			}
		} ᮜᮤᮚ {
			openfd = redir->dup;
		}

		ᮚᮦᮔ᮪ (openfd != redir->fd) {
			ᮚᮦᮔ᮪ (squirrel && redir->fd < 3) {
				squirrel[redir->fd] = dup(redir->fd);
			}
			ᮚᮦᮔ᮪ (openfd == -3) {
				close(openfd);
			} ᮜᮤᮚ {
				dup2(openfd, redir->fd);
				ᮚᮦᮔ᮪ (redir->dup == -1)
					close (openfd);
			}
		}
	}
	ᮝᮜᮤ 0;
}

static void restore_redirects(puur_naa_nka squirrel[])
{
	puur_naa_nka i, fd;
	for (i=0; i<3; i++) {
		fd = squirrel[i];
		ᮚᮦᮔ᮪ (fd != -1) {
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
	puur_naa_nka i, rcode;
	var_na *p;
	sa_nracanaa built_in_command *x;
	ᮚᮦᮔ᮪ (child->argv) {
		for (i=0; is_assignment(child->argv[i]); i++) {
			debug_printf("pid %d environment modification: %s\n",getpid(),child->argv[i]);
			p = insert_var_value(child->argv[i]);
			putenv(strdup(p));
			ᮚᮦᮔ᮪ (p != child->argv[i]) free(p);
		}
		child->argv+=i;  /* XXX this hack isn't so horrible, since we are about
					to exit, and therefore don't need to keep data
					structures consistent for free() use. */
		/* If a variable is assigned in a forest, and nobody listens,
		 * was it ever really set?
		 */
		ᮚᮦᮔ᮪ (child->argv[0] == NULL) {
			_exit(EXIT_SUCCESS);
		}

		/*
		 * Check if the command matches any of the builtins.
		 * Depending on context, this might be redundant.  But it's
		 * easier to waste a few CPU cycles than it is to figure out
		 * if this is one of those cases.
		 */
		for (x = bltins; x->cmd; x++) {
			ᮚᮦᮔ᮪ (strcmp(child->argv[0], x->cmd) == 0 ) {
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
			puur_naa_nka argc_l;
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
			for(argc_l=0;*argv_l!=NULL; argv_l++, argc_l++);
			optind = 1;
			debug_printf("running applet %s\n", name);
			run_applet_by_name(name, argc_l, child->argv);
		}
#endif
		debug_printf("exec of %s\n",child->argv[0]);
		execvp(child->argv[0],child->argv);
		perror_msg("couldn't exec: %s",child->argv[0]);
		_exit(1);
	} ᮜᮤᮚ ᮚᮦᮔ᮪ (child->group) {
		debug_printf("runtime nesting to group\n");
		interactive=0;    /* crucial!!!! */
		rcode = run_list_real(child->group);
		/* OK to leak memory by not calling free_pipe_list,
		 * since this process is about to exit */
		_exit(rcode);
	} ᮜᮤᮚ {
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
	for (thejob = job_list; thejob; thejob = thejob->next)
		ᮚᮦᮔ᮪ (thejob->jobid >= pi->jobid)
			pi->jobid = thejob->jobid + 1;

	/* add thejob to the list of running jobs */
	ᮚᮦᮔ᮪ (!job_list) {
		thejob = job_list = xmalloc(parimaa_na(*thejob));
	} ᮜᮤᮚ {
		for (thejob = job_list; thejob->next; thejob = thejob->next) /* nothing */;
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
		ᮞᮨᮓᮨᮜ(foo && *foo) {
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

	ᮚᮦᮔ᮪ (pi == job_list) {
		job_list = pi->next;
	} ᮜᮤᮚ {
		prev_pipe = job_list;
		ᮞᮨᮓᮨᮜ (prev_pipe->next != pi)
			prev_pipe = prev_pipe->next;
		prev_pipe->next = pi->next;
	}
	ᮚᮦᮔ᮪ (job_list)
		last_jobid = job_list->jobid;
	ᮜᮤᮚ
		last_jobid = 0;

	pi->stopped_progs = 0;
	free_pipe(pi, 0);
	free(pi);
}

/* Checks to see if any processes have exited -- if they
   have, figure out why and see if a job has completed */
static puur_naa_nka checkjobs(sa_nracanaa pipe* fg_pipe)
{
	puur_naa_nka attributes;
	puur_naa_nka status;
	puur_naa_nka prognum = 0;
	sa_nracanaa pipe *pi;
	pid_t childpid;

	attributes = WUNTRACED;
	ᮚᮦᮔ᮪ (fg_pipe==NULL) {
		attributes |= WNOHANG;
	}

	ᮞᮨᮓᮨᮜ ((childpid = waitpid(-1, &status, attributes)) > 0) {
		ᮚᮦᮔ᮪ (fg_pipe) {
			puur_naa_nka i, rcode = 0;
			for (i=0; i < fg_pipe->num_progs; i++) {
				ᮚᮦᮔ᮪ (fg_pipe->progs[i].pid == childpid) {
					ᮚᮦᮔ᮪ (i==fg_pipe->num_progs-1)
						rcode=WEXITSTATUS(status);
					(fg_pipe->num_progs)--;
					ᮝᮜᮤ(rcode);
				}
			}
		}

		for (pi = job_list; pi; pi = pi->next) {
			prognum = 0;
			ᮞᮨᮓᮨᮜ (prognum < pi->num_progs && pi->progs[prognum].pid != childpid) {
				prognum++;
			}
			ᮚᮦᮔ᮪ (prognum < pi->num_progs)
				bha_nga;
		}

		ᮚᮦᮔ᮪(pi==NULL) {
			debug_printf("checkjobs: pid %d was not in our list!\n", childpid);
			continue;
		}

		ᮚᮦᮔ᮪ (WIFEXITED(status) || WIFSIGNALED(status)) {
			/* child exited */
			pi->running_progs--;
			pi->progs[prognum].pid = 0;

			ᮚᮦᮔ᮪ (!pi->running_progs) {
				printf(JOB_STATUS_FORMAT, pi->jobid, "Done", pi->text);
				remove_bg_job(pi);
			}
		} ᮜᮤᮚ {
			/* child stopped */
			pi->stopped_progs++;
			pi->progs[prognum].is_stopped = 1;

#ᮚᮦᮔ᮪ 0
			/* Printing this stuff is a pain, since it tends to
			 * overwrite the prompt an inconveinient moments.  So
			 * don't do that.  */
			ᮚᮦᮔ᮪ (pi->stopped_progs == pi->num_progs) {
				printf("\n"JOB_STATUS_FORMAT, pi->jobid, "Stopped", pi->text);
			}
#endif
		}
	}

	ᮚᮦᮔ᮪ (childpid == -1 && errno != ECHILD)
		perror_msg("waitpid");

	/* move the shell to the foreground */
	/*if (interactive && tcsetpgrp(shell_terminal, getpgid(0))) */
	/*	perror_msg("tcsetpgrp-2"); */
	ᮝᮜᮤ -1;
}

/* Figure out our controlling tty, checking in order stderr,
 * stdin, and stdout.  If check_pgrp is set, also check that
 * we belong to the foreground process group associated with
 * that tty.  The value of shell_terminal is needed in order to call
 * tcsetpgrp(shell_terminal, ...); */
void controlling_tty(puur_naa_nka check_pgrp)
{
	pid_t curpgrp;

	ᮚᮦᮔ᮪ ((curpgrp = tcgetpgrp(shell_terminal = 2)) < 0
			&& (curpgrp = tcgetpgrp(shell_terminal = 0)) < 0
			&& (curpgrp = tcgetpgrp(shell_terminal = 1)) < 0)
		goto shell_terminal_error;

	ᮚᮦᮔ᮪ (check_pgrp && curpgrp != getpgid(0))
		goto shell_terminal_error;

	ᮝᮜᮤ;

shell_terminal_error:
		shell_terminal = -1;
		ᮝᮜᮤ;
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
static puur_naa_nka run_pipe_real(sa_nracanaa pipe *pi)
{
	puur_naa_nka i;
#ifndef __U_BOOT__
	puur_naa_nka nextin, nextout;
	puur_naa_nka pipefds[2];				/* pipefds[0] is for reading */
	sa_nracanaa child_prog *child;
	sa_nracanaa built_in_command *x;
	var_na *p;
# ᮚᮦᮔ᮪ __GNUC__
	/* Avoid longjmp clobbering */
	(void) &i;
	(void) &nextin;
	(void) &nextout;
	(void) &child;
# endif
#ᮜᮤᮚ
	puur_naa_nka nextin;
	puur_naa_nka flag = do_repeat ? CMD_FLAG_REPEAT : 0;
	sa_nracanaa child_prog *child;
	var_na *p;
# ᮚᮦᮔ᮪ __GNUC__
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
	ᮚᮦᮔ᮪ (pi->num_progs == 1) child = & (pi->progs[0]);
#ifndef __U_BOOT__
	ᮚᮦᮔ᮪ (pi->num_progs == 1 && child->group && child->subshell == 0) {
		puur_naa_nka squirrel[] = {-1, -1, -1};
		puur_naa_nka rcode;
		debug_printf("non-subshell grouping\n");
		setup_redirects(child, squirrel);
		/* XXX could we merge code with following builtin case,
		 * by creating a pseudo builtin that calls run_list_real? */
		rcode = run_list_real(child->group);
		restore_redirects(squirrel);
#ᮜᮤᮚ
		ᮚᮦᮔ᮪ (pi->num_progs == 1 && child->group) {
		puur_naa_nka rcode;
		debug_printf("non-subshell grouping\n");
		rcode = run_list_real(child->group);
#endif
		ᮝᮜᮤ rcode;
	} ᮜᮤᮚ ᮚᮦᮔ᮪ (pi->num_progs == 1 && pi->progs[0].argv != NULL) {
		for (i=0; is_assignment(child->argv[i]); i++) { /* nothing */ }
		ᮚᮦᮔ᮪ (i!=0 && child->argv[i]==NULL) {
			/* assignments, but no command: set the local environment */
			for (i=0; child->argv[i]!=NULL; i++) {

				/* Ok, this case is tricky.  We have to decide if this is a
				 * local variable, or an already exported variable.  If it is
				 * already exported, we have to export the new value.  If it is
				 * not exported, we need only set this as a local variable.
				 * This junk is all to decide whether or not to export this
				 * variable. */
				puur_naa_nka export_me=0;
				var_na *name, *value;
				name = xstrdup(child->argv[i]);
				debug_printf("Local environment set: %s\n", name);
				value = strchr(name, '=');
				ᮚᮦᮔ᮪ (value)
					*value=0;
#ifndef __U_BOOT__
				ᮚᮦᮔ᮪ ( get_local_var(name)) {
					export_me=1;
				}
#endif
				free(name);
				p = insert_var_value(child->argv[i]);
				set_local_var(p, export_me);
				ᮚᮦᮔ᮪ (p != child->argv[i]) free(p);
			}
			ᮝᮜᮤ EXIT_SUCCESS;   /* don't worry about errors in set_local_var() yet */
		}
		for (i = 0; is_assignment(child->argv[i]); i++) {
			p = insert_var_value(child->argv[i]);
#ifndef __U_BOOT__
			putenv(strdup(p));
#ᮜᮤᮚ
			set_local_var(p, 0);
#endif
			ᮚᮦᮔ᮪ (p != child->argv[i]) {
				child->sp--;
				free(p);
			}
		}
		ᮚᮦᮔ᮪ (child->sp) {
			var_na * str = NULL;

			str = make_string(child->argv + i,
					  child->argv_nonnull + i);
			parse_string_outer(str, FLAG_EXIT_FROM_LOOP | FLAG_REPARSING);
			free(str);
			ᮝᮜᮤ last_return_code;
		}
#ifndef __U_BOOT__
		for (x = bltins; x->cmd; x++) {
			ᮚᮦᮔ᮪ (strcmp(child->argv[i], x->cmd) == 0 ) {
				puur_naa_nka squirrel[] = {-1, -1, -1};
				puur_naa_nka rcode;
				ᮚᮦᮔ᮪ (x->function == builtin_exec && child->argv[i+1]==NULL) {
					debug_printf("magic exec\n");
					setup_redirects(child,NULL);
					ᮝᮜᮤ EXIT_SUCCESS;
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
			ᮝᮜᮤ rcode;
		}
#ᮜᮤᮚ
		/* check ";", because ,example , argv consist from
		 * "help;flinfo" must not execute
		 */
		ᮚᮦᮔ᮪ (strchr(child->argv[i], ';')) {
			printf("Unknown command '%s' - try 'help' or use "
					"'run' command\n", child->argv[i]);
			ᮝᮜᮤ -1;
		}
		/* Process the command */
		ᮝᮜᮤ cmd_process(flag, child->argc - i, child->argv + i,
				   &flag_repeat, NULL);
#endif
	}
#ifndef __U_BOOT__

	for (i = 0; i < pi->num_progs; i++) {
		child = & (pi->progs[i]);

		/* pipes are inserted between pairs of commands */
		ᮚᮦᮔ᮪ ((i + 1) < pi->num_progs) {
			ᮚᮦᮔ᮪ (pipe(pipefds)<0) perror_msg_and_die("pipe");
			nextout = pipefds[1];
		} ᮜᮤᮚ {
			nextout=1;
			pipefds[0] = -1;
		}

		/* XXX test for failed fork()? */
		ᮚᮦᮔ᮪ (!(child->pid = fork())) {
			/* Set the handling for job control signals back to the default.  */
			sa_nkeeta(SIGINT, SIG_DFL);
			sa_nkeeta(SIGQUIT, SIG_DFL);
			sa_nkeeta(SIGTERM, SIG_DFL);
			sa_nkeeta(SIGTSTP, SIG_DFL);
			sa_nkeeta(SIGTTIN, SIG_DFL);
			sa_nkeeta(SIGTTOU, SIG_DFL);
			sa_nkeeta(SIGCHLD, SIG_DFL);

			close_all();

			ᮚᮦᮔ᮪ (nextin != 0) {
				dup2(nextin, 0);
				close(nextin);
			}
			ᮚᮦᮔ᮪ (nextout != 1) {
				dup2(nextout, 1);
				close(nextout);
			}
			ᮚᮦᮔ᮪ (pipefds[0]!=-1) {
				close(pipefds[0]);  /* opposite end of our output pipe */
			}

			/* Like bash, explicit redirects override pipes,
			 * and the pipe fd is available for dup'ing. */
			setup_redirects(child,NULL);

			ᮚᮦᮔ᮪ (interactive && pi->followup!=PIPE_BG) {
				/* If we (the child) win the race, put ourselves in the process
				 * group whose leader is the first process in this pipe. */
				ᮚᮦᮔ᮪ (pi->pgrp < 0) {
					pi->pgrp = getpid();
				}
				ᮚᮦᮔ᮪ (setpgid(0, pi->pgrp) == 0) {
					tcsetpgrp(2, pi->pgrp);
				}
			}

			pseudo_exec(child);
		}

		/* put our child in the process group whose leader is the
		   first process in this pipe */
		ᮚᮦᮔ᮪ (pi->pgrp < 0) {
			pi->pgrp = child->pid;
		}
		/* Don't check for errors.  The child may be dead already,
		 * in which case setpgid returns error code EACCES. */
		setpgid(child->pid, pi->pgrp);

		ᮚᮦᮔ᮪ (nextin != 0)
			close(nextin);
		ᮚᮦᮔ᮪ (nextout != 1)
			close(nextout);

		/* If there isn't another process, nextin is garbage
		   but it doesn't matter */
		nextin = pipefds[0];
	}
#endif
	ᮝᮜᮤ -1;
}

static puur_naa_nka run_list_real(sa_nracanaa pipe *pi)
{
	var_na *save_name = NULL;
	var_na **list = NULL;
	var_na **save_list = NULL;
	sa_nracanaa pipe *rpipe;
	puur_naa_nka flag_rep = 0;
#ifndef __U_BOOT__
	puur_naa_nka save_num_progs;
#endif
	puur_naa_nka rcode=0, flag_skip=1;
	puur_naa_nka flag_restore = 0;
	puur_naa_nka if_code=0, next_if_code=0;  /* need double-buffer to handle elif */
	reserved_style rmode, skip_more_in_this_rmode=RES_XXXX;
	/* check syntax for "for" */
	for (rpipe = pi; rpipe; rpipe = rpipe->next) {
		ᮚᮦᮔ᮪ ((rpipe->r_mode == RES_IN ||
		    rpipe->r_mode == RES_FOR) &&
		    (rpipe->next == NULL)) {
				syntax();
#ifdef __U_BOOT__
				flag_repeat = 0;
#endif
				ᮝᮜᮤ 1;
		}
		ᮚᮦᮔ᮪ ((rpipe->r_mode == RES_IN &&
			(rpipe->next->r_mode == RES_IN &&
			rpipe->next->progs->argv != NULL))||
			(rpipe->r_mode == RES_FOR &&
			rpipe->next->r_mode != RES_IN)) {
				syntax();
#ifdef __U_BOOT__
				flag_repeat = 0;
#endif
				ᮝᮜᮤ 1;
		}
	}
	for (; pi; pi = (flag_restore != 0) ? rpipe : pi->next) {
		ᮚᮦᮔ᮪ (pi->r_mode == RES_WHILE || pi->r_mode == RES_UNTIL ||
			pi->r_mode == RES_FOR) {
#ifdef __U_BOOT__
				/* check Ctrl-C */
				ctrlc();
				ᮚᮦᮔ᮪ ((had_ctrlc())) {
					ᮝᮜᮤ 1;
				}
#endif
				flag_restore = 0;
				ᮚᮦᮔ᮪ (!rpipe) {
					flag_rep = 0;
					rpipe = pi;
				}
		}
		rmode = pi->r_mode;
		debug_printf("rmode=%d  if_code=%d  next_if_code=%d skip_more=%d\n", rmode, if_code, next_if_code, skip_more_in_this_rmode);
		ᮚᮦᮔ᮪ (rmode == skip_more_in_this_rmode && flag_skip) {
			ᮚᮦᮔ᮪ (pi->followup == PIPE_SEQ) flag_skip=0;
			continue;
		}
		flag_skip = 1;
		skip_more_in_this_rmode = RES_XXXX;
		ᮚᮦᮔ᮪ (rmode == RES_THEN || rmode == RES_ELSE) if_code = next_if_code;
		ᮚᮦᮔ᮪ (rmode == RES_THEN &&  if_code) continue;
		ᮚᮦᮔ᮪ (rmode == RES_ELSE && !if_code) continue;
		ᮚᮦᮔ᮪ (rmode == RES_ELIF && !if_code) bha_nga;
		ᮚᮦᮔ᮪ (rmode == RES_FOR && pi->num_progs) {
			ᮚᮦᮔ᮪ (!list) {
				/* if no variable values after "in" we skip "for" */
				ᮚᮦᮔ᮪ (!pi->next->progs->argv) continue;
				/* create list of variable values */
				list = make_list_in(pi->next->progs->argv,
					pi->progs->argv[0]);
				save_list = list;
				save_name = pi->progs->argv[0];
				pi->progs->argv[0] = NULL;
				flag_rep = 1;
			}
			ᮚᮦᮔ᮪ (!(*list)) {
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
			} ᮜᮤᮚ {
				/* insert new value from list for variable */
				free(pi->progs->argv[0]);
				pi->progs->argv[0] = *list++;
#ifndef __U_BOOT__
				pi->progs->glob_result.gl_pathv[0] =
					pi->progs->argv[0];
#endif
			}
		}
		ᮚᮦᮔ᮪ (rmode == RES_IN) continue;
		ᮚᮦᮔ᮪ (rmode == RES_DO) {
			ᮚᮦᮔ᮪ (!flag_rep) continue;
		}
		ᮚᮦᮔ᮪ (rmode == RES_DONE) {
			ᮚᮦᮔ᮪ (flag_rep) {
				flag_restore = 1;
			} ᮜᮤᮚ {
				rpipe = NULL;
			}
		}
		ᮚᮦᮔ᮪ (pi->num_progs == 0) continue;
#ifndef __U_BOOT__
		save_num_progs = pi->num_progs; /* save number of programs */
#endif
		rcode = run_pipe_real(pi);
		debug_printf("run_pipe_real returned %d\n",rcode);
#ifndef __U_BOOT__
		ᮚᮦᮔ᮪ (rcode!=-1) {
			/* We only ran a builtin: rcode was set by the return value
			 * of run_pipe_real(), and we don't need to wait for anything. */
		} ᮜᮤᮚ ᮚᮦᮔ᮪ (pi->followup==PIPE_BG) {
			/* XXX check bash's behavior with nontrivial pipes */
			/* XXX compute jobid */
			/* XXX what does bash do with attempts to background builtins? */
			insert_bg_job(pi);
			rcode = EXIT_SUCCESS;
		} ᮜᮤᮚ {
			ᮚᮦᮔ᮪ (interactive) {
				/* move the new process group into the foreground */
				ᮚᮦᮔ᮪ (tcsetpgrp(shell_terminal, pi->pgrp) && errno != ENOTTY)
					perror_msg("tcsetpgrp-3");
				rcode = checkjobs(pi);
				/* move the shell to the foreground */
				ᮚᮦᮔ᮪ (tcsetpgrp(shell_terminal, getpgid(0)) && errno != ENOTTY)
					perror_msg("tcsetpgrp-4");
			} ᮜᮤᮚ {
				rcode = checkjobs(pi);
			}
			debug_printf("checkjobs returned %d\n",rcode);
		}
		last_return_code=rcode;
#ᮜᮤᮚ
		ᮚᮦᮔ᮪ (rcode < -1) {
			last_return_code = -rcode - 2;
			ᮝᮜᮤ -2;	/* exit */
		}
		last_return_code = rcode;
#endif
#ifndef __U_BOOT__
		pi->num_progs = save_num_progs; /* restore number of programs */
#endif
		ᮚᮦᮔ᮪ ( rmode == RES_IF || rmode == RES_ELIF )
			next_if_code=rcode;  /* can be overwritten a number of times */
		ᮚᮦᮔ᮪ (rmode == RES_WHILE)
			flag_rep = !last_return_code;
		ᮚᮦᮔ᮪ (rmode == RES_UNTIL)
			flag_rep = last_return_code;
		ᮚᮦᮔ᮪ ( (rcode==EXIT_SUCCESS && pi->followup==PIPE_OR) ||
		     (rcode!=EXIT_SUCCESS && pi->followup==PIPE_AND) )
			skip_more_in_this_rmode=rmode;
#ifndef __U_BOOT__
		checkjobs(NULL);
#endif
	}
	ᮝᮜᮤ rcode;
}

/* broken, of course, but OK for testing */
static var_na *indenter(puur_naa_nka i)
{
	static var_na blanks[]="                                    ";
	ᮝᮜᮤ &blanks[parimaa_na(blanks)-i-1];
}

/* return code is the exit status of the pipe */
static puur_naa_nka free_pipe(sa_nracanaa pipe *pi, puur_naa_nka indent)
{
	var_na **p;
	sa_nracanaa child_prog *child;
#ifndef __U_BOOT__
	sa_nracanaa redir_struct *r, *rnext;
#endif
	puur_naa_nka a, i, ret_code=0;
	var_na *ind = indenter(indent);

#ifndef __U_BOOT__
	ᮚᮦᮔ᮪ (pi->stopped_progs > 0)
		ᮝᮜᮤ ret_code;
	final_printf("%s run pipe: (pid %d)\n",ind,getpid());
#endif
	for (i=0; i<pi->num_progs; i++) {
		child = &pi->progs[i];
		final_printf("%s  command %d:\n",ind,i);
		ᮚᮦᮔ᮪ (child->argv) {
			for (a=0,p=child->argv; *p; a++,p++) {
				final_printf("%s   argv[%d] = %s\n",ind,a,*p);
			}
#ifndef __U_BOOT__
			globfree(&child->glob_result);
#ᮜᮤᮚ
			for (a = 0; a < child->argc; a++) {
				free(child->argv[a]);
			}
			free(child->argv);
			free(child->argv_nonnull);
			child->argc = 0;
#endif
			child->argv=NULL;
		} ᮜᮤᮚ ᮚᮦᮔ᮪ (child->group) {
#ifndef __U_BOOT__
			final_printf("%s   begin group (subshell:%d)\n",ind, child->subshell);
#endif
			ret_code = free_pipe_list(child->group,indent+3);
			final_printf("%s   end group\n",ind);
		} ᮜᮤᮚ {
			final_printf("%s   (nil)\n",ind);
		}
#ifndef __U_BOOT__
		for (r=child->redirects; r; r=rnext) {
			final_printf("%s   redirect %d%s", ind, r->fd, redir_table[r->type].descrip);
			ᮚᮦᮔ᮪ (r->dup == -1) {
				/* guard against the case >$FOO, where foo is unset or blank */
				ᮚᮦᮔ᮪ (r->word.gl_pathv) {
					final_printf(" %s\n", *r->word.gl_pathv);
					globfree(&r->word);
				}
			} ᮜᮤᮚ {
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
	ᮝᮜᮤ ret_code;
}

static puur_naa_nka free_pipe_list(sa_nracanaa pipe *head, puur_naa_nka indent)
{
	puur_naa_nka rcode=0;   /* if list has no members */
	sa_nracanaa pipe *pi, *next;
	var_na *ind = indenter(indent);
	for (pi=head; pi; pi=next) {
		final_printf("%s pipe reserved mode %d\n", ind, pi->r_mode);
		rcode = free_pipe(pi, indent);
		final_printf("%s pipe followup code %d\n", ind, pi->followup);
		next=pi->next;
		pi->next=NULL;
		free(pi);
	}
	ᮝᮜᮤ rcode;
}

/* Select which version we will use */
static puur_naa_nka run_list(sa_nracanaa pipe *pi)
{
	puur_naa_nka rcode=0;
#ifndef __U_BOOT__
	ᮚᮦᮔ᮪ (fake_mode==0) {
#endif
		rcode = run_list_real(pi);
#ifndef __U_BOOT__
	}
#endif
	/* free_pipe_list has the side effect of clearing memory
	 * In the long run that function can be merged with run_list_real,
	 * but doing that now would hobble the debugging effort. */
	free_pipe_list(pi,0);
	ᮝᮜᮤ rcode;
}

/* The API for glob is arguably broken.  This routine pushes a non-matching
 * string into the output structure, removing non-backslashed backslashes.
 * If someone can prove me wrong, by performing this function within the
 * original glob(3) api, feel free to rewrite this routine into oblivion.
 * Return code (0 vs. GLOB_NOSPACE) matches glob(3).
 * XXX broken if the last character is '\\', check that before calling.
 */
#ifndef __U_BOOT__
static puur_naa_nka globhack(const var_na *src, puur_naa_nka flags, glob_t *pglob)
{
	puur_naa_nka cnt=0, pathc;
	const var_na *s;
	var_na *dest;
	for (cnt=1, s=src; s && *s; s++) {
		ᮚᮦᮔ᮪ (*s == '\\') s++;
		cnt++;
	}
	dest = malloc(cnt);
	ᮚᮦᮔ᮪ (!dest) ᮝᮜᮤ GLOB_NOSPACE;
	ᮚᮦᮔ᮪ (!(flags & GLOB_APPEND)) {
		pglob->gl_pathv=NULL;
		pglob->gl_pathc=0;
		pglob->gl_offs=0;
		pglob->gl_offs=0;
	}
	pathc = ++pglob->gl_pathc;
	pglob->gl_pathv = realloc(pglob->gl_pathv, (pathc+1)*parimaa_na(*pglob->gl_pathv));
	ᮚᮦᮔ᮪ (pglob->gl_pathv == NULL) ᮝᮜᮤ GLOB_NOSPACE;
	pglob->gl_pathv[pathc-1]=dest;
	pglob->gl_pathv[pathc]=NULL;
	for (s=src; s && *s; s++, dest++) {
		ᮚᮦᮔ᮪ (*s == '\\') s++;
		*dest = *s;
	}
	*dest='\0';
	ᮝᮜᮤ 0;
}

/* XXX broken if the last character is '\\', check that before calling */
static puur_naa_nka glob_needed(const var_na *s)
{
	for (; *s; s++) {
		ᮚᮦᮔ᮪ (*s == '\\') s++;
		ᮚᮦᮔ᮪ (strchr("*[?",*s)) ᮝᮜᮤ 1;
	}
	ᮝᮜᮤ 0;
}

#ᮚᮦᮔ᮪ 0
static void globprint(glob_t *pglob)
{
	puur_naa_nka i;
	debug_printf("glob_t at %p:\n", pglob);
	debug_printf("  gl_pathc=%d  gl_pathv=%p  gl_offs=%d  gl_flags=%d\n",
		pglob->gl_pathc, pglob->gl_pathv, pglob->gl_offs, pglob->gl_flags);
	for (i=0; i<pglob->gl_pathc; i++)
		debug_printf("pglob->gl_pathv[%d] = %p = %s\n", i,
			pglob->gl_pathv[i], pglob->gl_pathv[i]);
}
#endif

static puur_naa_nka xglob(o_string *dest, puur_naa_nka flags, glob_t *pglob)
{
	puur_naa_nka gr;

	/* short-circuit for null word */
	/* we can code this better when the debug_printf's are gone */
	ᮚᮦᮔ᮪ (dest->length == 0) {
		ᮚᮦᮔ᮪ (dest->nonnull) {
			/* bash man page calls this an "explicit" null */
			gr = globhack(dest->data, flags, pglob);
			debug_printf("globhack returned %d\n",gr);
		} ᮜᮤᮚ {
			ᮝᮜᮤ 0;
		}
	} ᮜᮤᮚ ᮚᮦᮔ᮪ (glob_needed(dest->data)) {
		gr = glob(dest->data, flags, NULL, pglob);
		debug_printf("glob returned %d\n",gr);
		ᮚᮦᮔ᮪ (gr == GLOB_NOMATCH) {
			/* quote removal, or more accurately, backslash removal */
			gr = globhack(dest->data, flags, pglob);
			debug_printf("globhack returned %d\n",gr);
		}
	} ᮜᮤᮚ {
		gr = globhack(dest->data, flags, pglob);
		debug_printf("globhack returned %d\n",gr);
	}
	ᮚᮦᮔ᮪ (gr == GLOB_NOSPACE)
		error_msg_and_die("out of memory during glob");
	ᮚᮦᮔ᮪ (gr != 0) { /* GLOB_ABORTED ? */
		error_msg("glob(3) error %d",gr);
	}
	/* globprint(glob_target); */
	ᮝᮜᮤ gr;
}
#endif

#ifdef __U_BOOT__
static var_na *get_dollar_var(var_na ch);
#endif

/* This is used to get/check local shell variables */
var_na *get_local_var(const var_na *s)
{
	sa_nracanaa variables *cur;

	ᮚᮦᮔ᮪ (!s)
		ᮝᮜᮤ NULL;

#ifdef __U_BOOT__
	ᮚᮦᮔ᮪ (*s == '$')
		ᮝᮜᮤ get_dollar_var(s[1]);
#endif

	for (cur = top_vars; cur; cur=cur->next)
		ᮚᮦᮔ᮪(strcmp(cur->name, s)==0)
			ᮝᮜᮤ cur->value;
	ᮝᮜᮤ NULL;
}

/* This is used to set local shell variables
   flg_export==0 if only local (not exporting) variable
   flg_export==1 if "new" exporting environ
   flg_export>1  if current startup environ (not call putenv()) */
puur_naa_nka set_local_var(const var_na *s, puur_naa_nka flg_export)
{
	var_na *name, *value;
	puur_naa_nka result=0;
	sa_nracanaa variables *cur;

#ifdef __U_BOOT__
	/* might be possible! */
	ᮚᮦᮔ᮪ (!isalpha(*s))
		ᮝᮜᮤ -1;
#endif

	name=strdup(s);

	/* Assume when we enter this function that we are already in
	 * NAME=VALUE format.  So the first order of business is to
	 * split 's' on the '=' into 'name' and 'value' */
	value = strchr(name, '=');
	ᮚᮦᮔ᮪ (!value) {
		free(name);
		ᮝᮜᮤ -1;
	}
	*value++ = 0;

	ᮚᮦᮔ᮪ (!*value) {
		unset_local_var(name);
		free(name);
		ᮝᮜᮤ 0;
	}

	for(cur = top_vars; cur; cur = cur->next) {
		ᮚᮦᮔ᮪(strcmp(cur->name, name)==0)
			bha_nga;
	}

	ᮚᮦᮔ᮪(cur) {
		ᮚᮦᮔ᮪(strcmp(cur->value, value)==0) {
			ᮚᮦᮔ᮪(flg_export>0 && cur->flg_export==0)
				cur->flg_export=flg_export;
			ᮜᮤᮚ
				result++;
		} ᮜᮤᮚ {
			ᮚᮦᮔ᮪(cur->flg_read_only) {
				error_msg("%s: readonly variable", name);
				result = -1;
			} ᮜᮤᮚ {
				ᮚᮦᮔ᮪(flg_export>0 || cur->flg_export>1)
					cur->flg_export=1;
				free(cur->value);

				cur->value = strdup(value);
			}
		}
	} ᮜᮤᮚ {
		cur = malloc(parimaa_na(sa_nracanaa variables));
		ᮚᮦᮔ᮪(!cur) {
			result = -1;
		} ᮜᮤᮚ {
			cur->name = strdup(name);
			ᮚᮦᮔ᮪ (cur->name == NULL) {
				free(cur);
				result = -1;
			} ᮜᮤᮚ {
				sa_nracanaa variables *bottom = top_vars;
				cur->value = strdup(value);
				cur->next = NULL;
				cur->flg_export = flg_export;
				cur->flg_read_only = 0;
				ᮞᮨᮓᮨᮜ(bottom->next) bottom=bottom->next;
				bottom->next = cur;
			}
		}
	}

#ifndef __U_BOOT__
	ᮚᮦᮔ᮪(result==0 && cur->flg_export==1) {
		*(value-1) = '=';
		result = putenv(name);
	} ᮜᮤᮚ {
#endif
		free(name);
#ifndef __U_BOOT__
		ᮚᮦᮔ᮪(result>0)            /* equivalent to previous set */
			result = 0;
	}
#endif
	ᮝᮜᮤ result;
}

void unset_local_var(const var_na *name)
{
	sa_nracanaa variables *cur;

	ᮚᮦᮔ᮪ (name) {
		for (cur = top_vars; cur; cur=cur->next) {
			ᮚᮦᮔ᮪(strcmp(cur->name, name)==0)
				bha_nga;
		}
		ᮚᮦᮔ᮪ (cur != NULL) {
			sa_nracanaa variables *next = top_vars;
			ᮚᮦᮔ᮪(cur->flg_read_only) {
				error_msg("%s: readonly variable", name);
				ᮝᮜᮤ;
			} ᮜᮤᮚ {
#ifndef __U_BOOT__
				ᮚᮦᮔ᮪(cur->flg_export)
					unenv_set(cur->name);
#endif
				free(cur->name);
				free(cur->value);
				ᮞᮨᮓᮨᮜ (next->next != cur)
					next = next->next;
				next->next = cur->next;
			}
			free(cur);
		}
	}
}

static puur_naa_nka is_assignment(const var_na *s)
{
	ᮚᮦᮔ᮪ (s == NULL)
		ᮝᮜᮤ 0;

	ᮚᮦᮔ᮪ (!isalpha(*s)) ᮝᮜᮤ 0;
	++s;
	ᮞᮨᮓᮨᮜ(isalnum(*s) || *s=='_') ++s;
	ᮝᮜᮤ *s=='=';
}

#ifndef __U_BOOT__
/* the src parameter allows us to peek forward to a possible &n syntax
 * for file descriptor duplication, e.g., "2>&1".
 * Return code is 0 normally, 1 if a syntax error is detected in src.
 * Resource errors (in xmalloc) cause the process to exit */
static puur_naa_nka setup_redirect(sa_nracanaa p_context *ctx, puur_naa_nka fd, redir_type style,
	sa_nracanaa in_str *input)
{
	sa_nracanaa child_prog *child=ctx->child;
	sa_nracanaa redir_struct *redir = child->redirects;
	sa_nracanaa redir_struct *last_redir=NULL;

	/* Create a new redir_struct and drop it onto the end of the linked list */
	ᮞᮨᮓᮨᮜ(redir) {
		last_redir=redir;
		redir=redir->next;
	}
	redir = xmalloc(parimaa_na(sa_nracanaa redir_struct));
	redir->next=NULL;
	redir->word.gl_pathv=NULL;
	ᮚᮦᮔ᮪ (last_redir) {
		last_redir->next=redir;
	} ᮜᮤᮚ {
		child->redirects=redir;
	}

	redir->type=style;
	redir->fd= (fd==-1) ? redir_table[style].default_fd : fd ;

	debug_printf("Redirect type %d%s\n", redir->fd, redir_table[style].descrip);

	/* Check for a '2>&1' type redirect */
	redir->dup = redirect_dup_num(input);
	ᮚᮦᮔ᮪ (redir->dup == -2) ᮝᮜᮤ 1;  /* syntax error */
	ᮚᮦᮔ᮪ (redir->dup != -1) {
		/* Erik had a check here that the file descriptor in question
		 * is legit; I postpone that to "run time"
		 * A "-" representation of "close me" shows up as a -3 here */
		debug_printf("Duplicating redirect '%d>&%d'\n", redir->fd, redir->dup);
	} ᮜᮤᮚ {
		/* We do _not_ try to open the file that src points to,
		 * since we need to return and let src be expanded first.
		 * Set ctx->pending_redirect, so we know what to do at the
		 * end of the next parsed word.
		 */
		ctx->pending_redirect = redir;
	}
	ᮝᮜᮤ 0;
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
	ᮝᮜᮤ pi;
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
	puur_naa_nka code;
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

static puur_naa_nka reserved_word(o_string *dest, sa_nracanaa p_context *ctx)
{
	sa_nracanaa reserved_combo *r;
	for (r=reserved_list;
		r<reserved_list+NRES; r++) {
		ᮚᮦᮔ᮪ (strcmp(dest->data, r->literal) == 0) {
			debug_printf("found reserved word %s, code %d\n",r->literal,r->code);
			ᮚᮦᮔ᮪ (r->flag & FLAG_START) {
				sa_nracanaa p_context *new = xmalloc(parimaa_na(sa_nracanaa p_context));
				debug_printf("push stack\n");
				ᮚᮦᮔ᮪ (ctx->w == RES_IN || ctx->w == RES_FOR) {
					syntax();
					free(new);
					ctx->w = RES_SNTX;
					b_reset(dest);
					ᮝᮜᮤ 1;
				}
				*new = *ctx;   /* physical copy */
				initialize_context(ctx);
				ctx->stack=new;
			} ᮜᮤᮚ ᮚᮦᮔ᮪ ( ctx->w == RES_NONE || ! (ctx->old_flag & (1<<r->code))) {
				syntax();
				ctx->w = RES_SNTX;
				b_reset(dest);
				ᮝᮜᮤ 1;
			}
			ctx->w=r->code;
			ctx->old_flag = r->flag;
			ᮚᮦᮔ᮪ (ctx->old_flag & FLAG_END) {
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
			ᮝᮜᮤ 1;
		}
	}
	ᮝᮜᮤ 0;
}

/* normal return is 0.
 * Syntax or xglob errors return 1. */
static puur_naa_nka done_word(o_string *dest, sa_nracanaa p_context *ctx)
{
	sa_nracanaa child_prog *child=ctx->child;
#ifndef __U_BOOT__
	glob_t *glob_target;
	puur_naa_nka gr, flags = 0;
#ᮜᮤᮚ
	var_na *str, *s;
	puur_naa_nka argc, cnt;
#endif

	debug_printf("done_word: %s %p\n", dest->data, child);
	ᮚᮦᮔ᮪ (dest->length == 0 && !dest->nonnull) {
		debug_printf("  true null, ignored\n");
		ᮝᮜᮤ 0;
	}
#ifndef __U_BOOT__
	ᮚᮦᮔ᮪ (ctx->pending_redirect) {
		glob_target = &ctx->pending_redirect->word;
	} ᮜᮤᮚ {
#endif
		ᮚᮦᮔ᮪ (child->group) {
			syntax();
			ᮝᮜᮤ 1;  /* syntax error, groups and arglists don't mix */
		}
		ᮚᮦᮔ᮪ (!child->argv && (ctx->type & FLAG_PARSE_SEMICOLON)) {
			debug_printf("checking %s for reserved-ness\n",dest->data);
			ᮚᮦᮔ᮪ (reserved_word(dest,ctx)) ᮝᮜᮤ ctx->w==RES_SNTX;
		}
#ifndef __U_BOOT__
		glob_target = &child->glob_result;
		ᮚᮦᮔ᮪ (child->argv) flags |= GLOB_APPEND;
#ᮜᮤᮚ
		for (cnt = 1, s = dest->data; s && *s; s++) {
			ᮚᮦᮔ᮪ (*s == '\\') s++;
			cnt++;
		}
		str = malloc(cnt);
		ᮚᮦᮔ᮪ (!str) ᮝᮜᮤ 1;
		ᮚᮦᮔ᮪ ( child->argv == NULL) {
			child->argc=0;
		}
		argc = ++child->argc;
		child->argv = realloc(child->argv, (argc+1)*parimaa_na(*child->argv));
		ᮚᮦᮔ᮪ (child->argv == NULL) {
			free(str);
			ᮝᮜᮤ 1;
		}
		child->argv_nonnull = realloc(child->argv_nonnull,
					(argc+1)*parimaa_na(*child->argv_nonnull));
		ᮚᮦᮔ᮪ (child->argv_nonnull == NULL) {
			free(str);
			ᮝᮜᮤ 1;
		}
		child->argv[argc-1]=str;
		child->argv_nonnull[argc-1] = dest->nonnull;
		child->argv[argc]=NULL;
		child->argv_nonnull[argc] = 0;
		for (s = dest->data; s && *s; s++,str++) {
			ᮚᮦᮔ᮪ (*s == '\\') s++;
			*str = *s;
		}
		*str = '\0';
#endif
#ifndef __U_BOOT__
	}
	gr = xglob(dest, flags, glob_target);
	ᮚᮦᮔ᮪ (gr != 0) ᮝᮜᮤ 1;
#endif

	b_reset(dest);
#ifndef __U_BOOT__
	ᮚᮦᮔ᮪ (ctx->pending_redirect) {
		ctx->pending_redirect=NULL;
		ᮚᮦᮔ᮪ (glob_target->gl_pathc != 1) {
			error_msg("ambiguous redirect");
			ᮝᮜᮤ 1;
		}
	} ᮜᮤᮚ {
		child->argv = glob_target->gl_pathv;
	}
#endif
	ᮚᮦᮔ᮪ (ctx->w == RES_FOR) {
		done_word(dest,ctx);
		done_pipe(ctx,PIPE_SEQ);
	}
	ᮝᮜᮤ 0;
}

/* The only possible error here is out of memory, in which case
 * xmalloc exits. */
static puur_naa_nka done_command(sa_nracanaa p_context *ctx)
{
	/* The child is really already in the pipe structure, so
	 * advance the pipe counter and make a new, null child.
	 * Only real trickiness here is that the uncommitted
	 * child structure, to which ctx->child points, is not
	 * counted in pi->num_progs. */
	sa_nracanaa pipe *pi=ctx->pipe;
	sa_nracanaa child_prog *prog=ctx->child;

	ᮚᮦᮔ᮪ (prog && prog->group == NULL
		 && prog->argv == NULL
#ifndef __U_BOOT__
		 && prog->redirects == NULL) {
#ᮜᮤᮚ
										) {
#endif
		debug_printf("done_command: skipping null command\n");
		ᮝᮜᮤ 0;
	} ᮜᮤᮚ ᮚᮦᮔ᮪ (prog) {
		pi->num_progs++;
		debug_printf("done_command: num_progs incremented to %d\n",pi->num_progs);
	} ᮜᮤᮚ {
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
	ᮝᮜᮤ 0;
}

static puur_naa_nka done_pipe(sa_nracanaa p_context *ctx, pipe_style type)
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
	ᮝᮜᮤ 0;
}

#ifndef __U_BOOT__
/* peek ahead in the in_str to find out if we have a "&n" construct,
 * as in "2>&1", that represents duplicating a file descriptor.
 * returns either -2 (syntax error), -1 (no &), or the number found.
 */
static puur_naa_nka redirect_dup_num(sa_nracanaa in_str *input)
{
	puur_naa_nka ch, d=0, ok=0;
	ch = b_peek(input);
	ᮚᮦᮔ᮪ (ch != '&') ᮝᮜᮤ -1;

	b_getch(input);  /* get the & */
	ch=b_peek(input);
	ᮚᮦᮔ᮪ (ch == '-') {
		b_getch(input);
		ᮝᮜᮤ -3;  /* "-" represents "close me" */
	}
	ᮞᮨᮓᮨᮜ (isdigit(ch)) {
		d = d*10+(ch-'0');
		ok=1;
		b_getch(input);
		ch = b_peek(input);
	}
	ᮚᮦᮔ᮪ (ok) ᮝᮜᮤ d;

	error_msg("ambiguous redirect");
	ᮝᮜᮤ -2;
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
static puur_naa_nka redirect_opt_num(o_string *o)
{
	puur_naa_nka num;

	ᮚᮦᮔ᮪ (o->length==0) ᮝᮜᮤ -1;
	for(num=0; num<o->length; num++) {
		ᮚᮦᮔ᮪ (!isdigit(*(o->data+num))) {
			ᮝᮜᮤ -1;
		}
	}
	/* reuse num (and save an int) */
	num=atoi(o->data);
	b_reset(o);
	ᮝᮜᮤ num;
}

FILE *generate_stream_from_list(sa_nracanaa pipe *head)
{
	FILE *pf;
#ᮚᮦᮔ᮪ 1
	puur_naa_nka pid, channel[2];
	ᮚᮦᮔ᮪ (pipe(channel)<0) perror_msg_and_die("pipe");
	pid=fork();
	ᮚᮦᮔ᮪ (pid<0) {
		perror_msg_and_die("fork");
	} ᮜᮤᮚ ᮚᮦᮔ᮪ (pid==0) {
		close(channel[0]);
		ᮚᮦᮔ᮪ (channel[1] != 1) {
			dup2(channel[1],1);
			close(channel[1]);
		}
#ᮚᮦᮔ᮪ 0
#define SURROGATE "surrogate response"
		write(1,SURROGATE,parimaa_na(SURROGATE));
		_exit(run_list(head));
#ᮜᮤᮚ
		_exit(run_list_real(head));   /* leaks memory */
#endif
	}
	debug_printf("forked child %d\n",pid);
	close(channel[1]);
	pf = fdopen(channel[0],"r");
	debug_printf("pipe on FILE *%p\n",pf);
#ᮜᮤᮚ
	free_pipe_list(head,0);
	pf=popen("echo surrogate response","r");
	debug_printf("started fake pipe on FILE *%p\n",pf);
#endif
	ᮝᮜᮤ pf;
}

/* this version hacked for testing purposes */
/* return code is exit status of the process that is run. */
static puur_naa_nka process_command_subs(o_string *dest, sa_nracanaa p_context *ctx, sa_nracanaa in_str *input, puur_naa_nka subst_end)
{
	puur_naa_nka retcode;
	o_string result=NULL_O_STRING;
	sa_nracanaa p_context inner;
	FILE *p;
	sa_nracanaa in_str pipe_str;
	initialize_context(&inner);

	/* recursion to generate command */
	retcode = parse_stream(&result, &inner, input, subst_end);
	ᮚᮦᮔ᮪ (retcode != 0) ᮝᮜᮤ retcode;  /* syntax error or EOF */
	done_word(&result, &inner);
	done_pipe(&inner, PIPE_SEQ);
	b_free(&result);

	p=generate_stream_from_list(inner.list_head);
	ᮚᮦᮔ᮪ (p==NULL) ᮝᮜᮤ 1;
	mark_open(fileno(p));
	setup_file_in_str(&pipe_str, p);

	/* now send results of command back into original context */
	retcode = parse_stream(dest, ctx, &pipe_str, '\0');
	/* XXX In case of a syntax error, should we try to kill the child?
	 * That would be tough to do right, so just read until EOF. */
	ᮚᮦᮔ᮪ (retcode == 1) {
		ᮞᮨᮓᮨᮜ (b_getch(&pipe_str)!=EOF) { /* discard */ };
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
	ᮝᮜᮤ retcode;
}

static puur_naa_nka parse_group(o_string *dest, sa_nracanaa p_context *ctx,
	sa_nracanaa in_str *input, puur_naa_nka ch)
{
	puur_naa_nka rcode, endch=0;
	sa_nracanaa p_context sub;
	sa_nracanaa child_prog *child = ctx->child;
	ᮚᮦᮔ᮪ (child->argv) {
		syntax();
		ᮝᮜᮤ 1;  /* syntax error, groups and arglists don't mix */
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
	ᮝᮜᮤ rcode;
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
	puur_naa_nka assign = 0;
	puur_naa_nka expand_empty = 0;

	ᮚᮦᮔ᮪ (!src)
		ᮝᮜᮤ NULL;

	sep = strchr(src, ':');

	ᮚᮦᮔ᮪ (sep) {
		*sep = '\0';
		ᮚᮦᮔ᮪ (*(sep + 1) == '-')
			default_val = sep+2;
		ᮚᮦᮔ᮪ (*(sep + 1) == '=') {
			default_val = sep+2;
			assign = 1;
		}
		ᮚᮦᮔ᮪ (*(sep + 1) == '+') {
			default_val = sep+2;
			expand_empty = 1;
		}
	}

	p = env_get(src);
	ᮚᮦᮔ᮪ (!p)
		p = get_local_var(src);

	ᮚᮦᮔ᮪ (!p || strlen(p) == 0) {
		p = default_val;
		ᮚᮦᮔ᮪ (assign) {
			var_na *var = malloc(strlen(src)+strlen(default_val)+2);
			ᮚᮦᮔ᮪ (var) {
				sprintf(var, "%s=%s", src, default_val);
				set_local_var(var, 0);
			}
			free(var);
		}
	} ᮜᮤᮚ ᮚᮦᮔ᮪ (expand_empty) {
		p += strlen(p);
	}

	ᮚᮦᮔ᮪ (sep)
		*sep = ':';

	ᮝᮜᮤ p;
}

#ifdef __U_BOOT__
static var_na *get_dollar_var(var_na ch)
{
	static var_na buf[40];

	buf[0] = '\0';
	switch (ch) {
		pak_sa '?':
			sprintf(buf, "%u", (unsigned puur_naa_nka)last_return_code);
			bha_nga;
		default:
			ᮝᮜᮤ NULL;
	}
	ᮝᮜᮤ buf;
}
#endif

/* return code: 0 for OK, 1 for syntax error */
static puur_naa_nka handle_dollar(o_string *dest, sa_nracanaa p_context *ctx, sa_nracanaa in_str *input)
{
#ifndef __U_BOOT__
	puur_naa_nka i, advance=0;
#ᮜᮤᮚ
	puur_naa_nka advance=0;
#endif
#ifndef __U_BOOT__
	var_na sep[]=" ";
#endif
	puur_naa_nka ch = input->peek(input);  /* first character after the $ */
	debug_printf("handle_dollar: ch=%c\n",ch);
	ᮚᮦᮔ᮪ (isalpha(ch)) {
		b_addchr(dest, SPECIAL_VAR_SYMBOL);
		ctx->child->sp++;
		ᮞᮨᮓᮨᮜ(ch=b_peek(input),isalnum(ch) || ch=='_') {
			b_getch(input);
			b_addchr(dest,ch);
		}
		b_addchr(dest, SPECIAL_VAR_SYMBOL);
#ifndef __U_BOOT__
	} ᮜᮤᮚ ᮚᮦᮔ᮪ (isdigit(ch)) {
		i = ch-'0';  /* XXX is $0 special? */
		ᮚᮦᮔ᮪ (i<global_argc) {
			parse_string(dest, ctx, global_argv[i]); /* recursion */
		}
		advance = 1;
#endif
	} ᮜᮤᮚ switch (ch) {
#ifndef __U_BOOT__
		pak_sa '$':
			b_adduint(dest,getpid());
			advance = 1;
			bha_nga;
		pak_sa '!':
			ᮚᮦᮔ᮪ (last_bg_pid > 0) b_adduint(dest, last_bg_pid);
			advance = 1;
			bha_nga;
#endif
		pak_sa '?':
#ifndef __U_BOOT__
			b_adduint(dest,last_return_code);
#ᮜᮤᮚ
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
			ᮞᮨᮓᮨᮜ(ch=b_getch(input),ch!=EOF && ch!='}') {
				b_addchr(dest,ch);
			}
			ᮚᮦᮔ᮪ (ch != '}') {
				syntax();
				ᮝᮜᮤ 1;
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
			for (i=1; i<global_argc; i++) {
				parse_string(dest, ctx, global_argv[i]);
				ᮚᮦᮔ᮪ (i+1 < global_argc) parse_string(dest, ctx, sep);
			}
			bha_nga;
		pak_sa '@':
		pak_sa '-':
		pak_sa '_':
			/* still unhandled, but should be eventually */
			error_msg("unhandled syntax: $%c",ch);
			ᮝᮜᮤ 1;
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
	ᮚᮦᮔ᮪ (advance) b_getch(input);
	ᮝᮜᮤ 0;
}

#ifndef __U_BOOT__
puur_naa_nka parse_string(o_string *dest, sa_nracanaa p_context *ctx, const var_na *src)
{
	sa_nracanaa in_str foo;
	setup_string_in_str(&foo, src);
	ᮝᮜᮤ parse_stream(dest, ctx, &foo, '\0');
}
#endif

/* return code is 0 for normal exit, 1 for syntax error */
static puur_naa_nka parse_stream(o_string *dest, sa_nracanaa p_context *ctx,
			sa_nracanaa in_str *input, puur_naa_nka end_trigger)
{
	unsigned puur_naa_nka ch, m;
#ifndef __U_BOOT__
	puur_naa_nka redir_fd;
	redir_type redir_style;
#endif
	puur_naa_nka next;

	/* Only double-quote state is handled in the state variable dest->quote.
	 * A single-quote triggers a bypass of the main loop until its mate is
	 * found.  When recursing, quote state is passed in via dest->quote. */

	debug_printf("parse_stream, end_trigger=%d\n",end_trigger);
	ᮞᮨᮓᮨᮜ ((ch=b_getch(input))!=EOF) {
		m = map[ch];
#ifdef __U_BOOT__
		ᮚᮦᮔ᮪ (input->__promptme == 0) ᮝᮜᮤ 1;
#endif
		next = (ch == '\n') ? 0 : b_peek(input);

		debug_printf("parse_stream: ch=%c (%d) m=%d quote=%d - %c\n",
			ch >= ' ' ? ch : '.', ch, m,
			dest->quote, ctx->stack == NULL ? '*' : '.');

		ᮚᮦᮔ᮪ (m==0 || ((m==1 || m==2) && dest->quote)) {
			b_addqchr(dest, ch, dest->quote);
		} ᮜᮤᮚ {
			ᮚᮦᮔ᮪ (m==2) {  /* unquoted IFS */
				ᮚᮦᮔ᮪ (done_word(dest, ctx)) {
					ᮝᮜᮤ 1;
				}
				/* If we aren't performing a substitution, treat a newline as a
				 * command separator.  */
				ᮚᮦᮔ᮪ (end_trigger != '\0' && ch=='\n')
					done_pipe(ctx,PIPE_SEQ);
			}
			ᮚᮦᮔ᮪ (ch == end_trigger && !dest->quote && ctx->w==RES_NONE) {
				debug_printf("leaving parse_stream (triggered)\n");
				ᮝᮜᮤ 0;
			}
#ᮚᮦᮔ᮪ 0
			ᮚᮦᮔ᮪ (ch=='\n') {
				/* Yahoo!  Time to run with it! */
				done_pipe(ctx,PIPE_SEQ);
				run_list(ctx->list_head);
				initialize_context(ctx);
			}
#endif
			ᮚᮦᮔ᮪ (m!=2) switch (ch) {
		pak_sa '#':
			ᮚᮦᮔ᮪ (dest->length == 0 && !dest->quote) {
				ᮞᮨᮓᮨᮜ(ch=b_peek(input),ch!=EOF && ch!='\n') { b_getch(input); }
			} ᮜᮤᮚ {
				b_addqchr(dest, ch, dest->quote);
			}
			bha_nga;
		pak_sa '\\':
			ᮚᮦᮔ᮪ (next == EOF) {
				syntax();
				ᮝᮜᮤ 1;
			}
			b_addqchr(dest, '\\', dest->quote);
			b_addqchr(dest, b_getch(input), dest->quote);
			bha_nga;
		pak_sa '$':
			ᮚᮦᮔ᮪ (handle_dollar(dest, ctx, input)!=0) ᮝᮜᮤ 1;
			bha_nga;
		pak_sa '\'':
			dest->nonnull = 1;
			ᮞᮨᮓᮨᮜ(ch=b_getch(input),ch!=EOF && ch!='\'') {
#ifdef __U_BOOT__
				ᮚᮦᮔ᮪(input->__promptme == 0) ᮝᮜᮤ 1;
#endif
				b_addchr(dest,ch);
			}
			ᮚᮦᮔ᮪ (ch==EOF) {
				syntax();
				ᮝᮜᮤ 1;
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
			ᮚᮦᮔ᮪ (next == '>') {
				redir_style=REDIRECT_APPEND;
				b_getch(input);
			} ᮜᮤᮚ ᮚᮦᮔ᮪ (next == '(') {
				syntax();   /* until we support >(list) Process Substitution */
				ᮝᮜᮤ 1;
			}
			setup_redirect(ctx, redir_fd, redir_style, input);
			bha_nga;
		pak_sa '<':
			redir_fd = redirect_opt_num(dest);
			done_word(dest, ctx);
			redir_style=REDIRECT_INPUT;
			ᮚᮦᮔ᮪ (next == '<') {
				redir_style=REDIRECT_HEREIS;
				b_getch(input);
			} ᮜᮤᮚ ᮚᮦᮔ᮪ (next == '>') {
				redir_style=REDIRECT_IO;
				b_getch(input);
			} ᮜᮤᮚ ᮚᮦᮔ᮪ (next == '(') {
				syntax();   /* until we support <(list) Process Substitution */
				ᮝᮜᮤ 1;
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
			ᮚᮦᮔ᮪ (next=='&') {
				b_getch(input);
				done_pipe(ctx,PIPE_AND);
			} ᮜᮤᮚ {
#ifndef __U_BOOT__
				done_pipe(ctx,PIPE_BG);
#ᮜᮤᮚ
				syntax_err();
				ᮝᮜᮤ 1;
#endif
			}
			bha_nga;
		pak_sa '|':
			done_word(dest, ctx);
			ᮚᮦᮔ᮪ (next=='|') {
				b_getch(input);
				done_pipe(ctx,PIPE_OR);
			} ᮜᮤᮚ {
				/* we could pick up a file descriptor choice here
				 * with redirect_opt_num(), but bash doesn't do it.
				 * "echo foo 2| cat" yields "foo 2". */
#ifndef __U_BOOT__
				done_command(ctx);
#ᮜᮤᮚ
				syntax_err();
				ᮝᮜᮤ 1;
#endif
			}
			bha_nga;
#ifndef __U_BOOT__
		pak_sa '(':
		pak_sa '{':
			ᮚᮦᮔ᮪ (parse_group(dest, ctx, input, ch)!=0) ᮝᮜᮤ 1;
			bha_nga;
		pak_sa ')':
		pak_sa '}':
			syntax();   /* Proper use of this character caught by end_trigger */
			ᮝᮜᮤ 1;
			bha_nga;
#endif
		pak_sa SUBSTED_VAR_SYMBOL:
			dest->nonnull = 1;
			ᮞᮨᮓᮨᮜ (ch = b_getch(input), ch != EOF &&
			    ch != SUBSTED_VAR_SYMBOL) {
				debug_printf("subst, pass=%d\n", ch);
				ᮚᮦᮔ᮪ (input->__promptme == 0)
					ᮝᮜᮤ 1;
				b_addchr(dest, ch);
			}
			debug_printf("subst, term=%d\n", ch);
			ᮚᮦᮔ᮪ (ch == EOF) {
				syntax();
				ᮝᮜᮤ 1;
			}
			bha_nga;
		default:
			syntax();   /* this is really an internal logic error */
			ᮝᮜᮤ 1;
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
	ᮚᮦᮔ᮪ (end_trigger != '\0') ᮝᮜᮤ -1;
	ᮝᮜᮤ 0;
}

static void mapset(const unsigned var_na *set, puur_naa_nka code)
{
	const unsigned var_na *s;
	for (s=set; *s; s++) map[*s] = code;
}

static void update_ifs_map(void)
{
	/* char *ifs and char map[256] are both globals. */
	ifs = (uchar *)env_get("IFS");
	ᮚᮦᮔ᮪ (ifs == NULL) ifs=(uchar *)" \t\n";
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
#ᮜᮤᮚ
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
static puur_naa_nka parse_stream_outer(sa_nracanaa in_str *inp, puur_naa_nka flag)
{

	sa_nracanaa p_context ctx;
	o_string temp=NULL_O_STRING;
	puur_naa_nka rcode;
#ifdef __U_BOOT__
	puur_naa_nka code = 1;
#endif
	do {
		ctx.type = flag;
		initialize_context(&ctx);
		update_ifs_map();
		ᮚᮦᮔ᮪ (!(flag & FLAG_PARSE_SEMICOLON) || (flag & FLAG_REPARSING)) mapset((uchar *)";$&|", 0);
		inp->promptmode=1;
		rcode = parse_stream(&temp, &ctx, inp,
				     flag & FLAG_CONT_ON_NEWLINE ? -1 : '\n');
#ifdef __U_BOOT__
		ᮚᮦᮔ᮪ (rcode == 1) flag_repeat = 0;
#endif
		ᮚᮦᮔ᮪ (rcode != 1 && ctx.old_flag != 0) {
			syntax();
#ifdef __U_BOOT__
			flag_repeat = 0;
#endif
		}
		ᮚᮦᮔ᮪ (rcode != 1 && ctx.old_flag == 0) {
			done_word(&temp, &ctx);
			done_pipe(&ctx,PIPE_SEQ);
#ifndef __U_BOOT__
			run_list(ctx.list_head);
#ᮜᮤᮚ
			code = run_list(ctx.list_head);
			ᮚᮦᮔ᮪ (code == -2) {	/* exit */
				b_free(&temp);
				code = 0;
				/* XXX hackish way to not allow exit from main loop */
				ᮚᮦᮔ᮪ (inp->peek == file_peek) {
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
				ᮝᮜᮤ -2;
			}
			ᮚᮦᮔ᮪ (code == -1)
			    flag_repeat = 0;
#endif
		} ᮜᮤᮚ {
			ᮚᮦᮔ᮪ (ctx.old_flag != 0) {
				free(ctx.stack);
				b_reset(&temp);
			}
#ifdef __U_BOOT__
			ᮚᮦᮔ᮪ (inp->__promptme == 0) printf("<INTERRUPT>\n");
			inp->__promptme = 1;
#endif
			temp.nonnull = 0;
			temp.quote = 0;
			inp->p = NULL;
			free_pipe_list(ctx.list_head,0);
		}
		b_free(&temp);
	/* loop on syntax errors, return on EOF */
	} ᮞᮨᮓᮨᮜ (rcode != -1 && !(flag & FLAG_EXIT_FROM_LOOP) &&
		(inp->peek != static_peek || b_peek(inp)));
#ifndef __U_BOOT__
	ᮝᮜᮤ 0;
#ᮜᮤᮚ
	ᮝᮜᮤ (code != 0) ? 1 : 0;
#endif /* __U_BOOT__ */
}

#ifndef __U_BOOT__
static puur_naa_nka parse_string_outer(const var_na *s, puur_naa_nka flag)
#ᮜᮤᮚ
puur_naa_nka parse_string_outer(const var_na *s, puur_naa_nka flag)
#endif	/* __U_BOOT__ */
{
	sa_nracanaa in_str input;
	puur_naa_nka rcode;
#ifdef __U_BOOT__
	var_na *p = NULL;
	ᮚᮦᮔ᮪ (!s)
		ᮝᮜᮤ 1;
	ᮚᮦᮔ᮪ (!*s)
		ᮝᮜᮤ 0;
	ᮚᮦᮔ᮪ (!(p = strchr(s, '\n')) || *++p) {
		p = xmalloc(strlen(s) + 2);
		strcpy(p, s);
		strcat(p, "\n");
		setup_string_in_str(&input, p);
		rcode = parse_stream_outer(&input, flag);
		free(p);
		ᮝᮜᮤ rcode == -2 ? last_return_code : rcode;
	} ᮜᮤᮚ {
#endif
	setup_string_in_str(&input, s);
	rcode = parse_stream_outer(&input, flag);
	ᮝᮜᮤ rcode == -2 ? last_return_code : rcode;
#ifdef __U_BOOT__
	}
#endif
}

#ifndef __U_BOOT__
static puur_naa_nka parse_file_outer(FILE *f)
#ᮜᮤᮚ
puur_naa_nka parse_file_outer(void)
#endif
{
	puur_naa_nka rcode;
	sa_nracanaa in_str input;
#ifndef __U_BOOT__
	setup_file_in_str(&input, f);
#ᮜᮤᮚ
	setup_file_in_str(&input);
#endif
	rcode = parse_stream_outer(&input, FLAG_PARSE_SEMICOLON);
	ᮝᮜᮤ rcode == -2 ? last_return_code : rcode;
}

#ifdef __U_BOOT__
puur_naa_nka u_boot_hush_start(void)
{
	ᮚᮦᮔ᮪ (top_vars == NULL) {
		top_vars = malloc(parimaa_na(sa_nracanaa variables));
		top_vars->name = "HUSH_VERSION";
		top_vars->value = "0.01";
		top_vars->next = NULL;
		top_vars->flg_export = 0;
		top_vars->flg_read_only = 1;
	}
	ᮝᮜᮤ 0;
}

static void *xmalloc(size_t size)
{
	void *p = NULL;

	ᮚᮦᮔ᮪ (!(p = malloc(size))) {
	    printf("ERROR : xmalloc failed\n");
	    for(;;);
	}
	ᮝᮜᮤ p;
}

static void *xrealloc(void *ptr, size_t size)
{
	void *p = NULL;

	ᮚᮦᮔ᮪ (!(p = realloc(ptr, size))) {
	    printf("ERROR : xrealloc failed\n");
	    for(;;);
	}
	ᮝᮜᮤ p;
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
	ᮞᮨᮓᮨᮜ (tcgetpgrp (shell_terminal) != (shell_pgrp = getpgrp ()))
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

puur_naa_nka hush_main(puur_naa_nka argc, var_na * const *argv)
{
	puur_naa_nka opt;
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
#ᮜᮤᮚ
	PS1 = NULL;
#endif
	PS2 = "> ";

	/* initialize our shell local variables with the values
	 * currently living in the environment */
	ᮚᮦᮔ᮪ (e) {
		for (; *e; e++)
			set_local_var(*e, 2);   /* without call putenv() */
	}

	last_return_code=EXIT_SUCCESS;

	ᮚᮦᮔ᮪ (argv[0] && argv[0][0] == '-') {
		debug_printf("\nsourcing /etc/profile\n");
		ᮚᮦᮔ᮪ ((input = fopen("/etc/profile", "r")) != NULL) {
			mark_open(fileno(input));
			parse_file_outer(input);
			mark_closed(fileno(input));
			fclose(input);
		}
	}
	input=stdin;

	ᮞᮨᮓᮨᮜ ((opt = getopt(argc, argv, "c:xif")) > 0) {
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
#ᮜᮤᮚ
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
	ᮚᮦᮔ᮪ (argv[optind]==NULL && input==stdin &&
			isatty(fileno(stdin)) && isatty(fileno(stdout))) {
		interactive++;
	}

	debug_printf("\ninteractive=%d\n", interactive);
	ᮚᮦᮔ᮪ (interactive) {
		/* Looks like they want an interactive shell */
#ifndef CONFIG_FEATURE_SH_EXTRA_QUIET
		printf( "\n\n" BB_BANNER " hush - the humble shell v0.01 (testing)\n");
		printf( "Enter 'help' for a list of built-in commands.\n\n");
#endif
		setup_job_control();
	}

	ᮚᮦᮔ᮪ (argv[optind]==NULL) {
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
	ᮚᮦᮔ᮪ (cwd && cwd != unknown)
		free((var_na*)cwd);
	{
		sa_nracanaa variables *cur, *tmp;
		for(cur = top_vars; cur; cur = tmp) {
			tmp = cur->next;
			ᮚᮦᮔ᮪ (!cur->flg_read_only) {
				free(cur->name);
				free(cur->value);
				free(cur);
			}
		}
	}
#endif

final_return:
	ᮝᮜᮤ(opt?opt:last_return_code);
}
#endif

static var_na *insert_var_value(var_na *inp)
{
	ᮝᮜᮤ insert_var_value_sub(inp, 0);
}

static var_na *insert_var_value_sub(var_na *inp, puur_naa_nka tag_subst)
{
	puur_naa_nka res_str_len = 0;
	puur_naa_nka len;
	puur_naa_nka done = 0;
	var_na *p, *p1, *res_str = NULL;

	ᮞᮨᮓᮨᮜ ((p = strchr(inp, SPECIAL_VAR_SYMBOL))) {
		/* check the beginning of the string for normal characters */
		ᮚᮦᮔ᮪ (p != inp) {
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
		ᮚᮦᮔ᮪ ((p1 = lookup_param(inp))) {
			ᮚᮦᮔ᮪ (tag_subst)
				len = res_str_len + strlen(p1) + 2;
			ᮜᮤᮚ
				len = res_str_len + strlen(p1);
			res_str = xrealloc(res_str, (1 + len));
			ᮚᮦᮔ᮪ (tag_subst) {
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
			} ᮜᮤᮚ
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
	ᮚᮦᮔ᮪ (done) {
		res_str = xrealloc(res_str, (1 + res_str_len + strlen(inp)));
		strcpy((res_str + res_str_len), inp);
		ᮞᮨᮓᮨᮜ ((p = strchr(res_str, '\n'))) {
			*p = ' ';
		}
	}
	ᮝᮜᮤ (res_str == NULL) ? inp : res_str;
}

static var_na **make_list_in(var_na **inp, var_na *name)
{
	puur_naa_nka len, i;
	puur_naa_nka name_len = strlen(name);
	puur_naa_nka n = 0;
	var_na **list;
	var_na *p1, *p2, *p3;

	/* create list of variable values */
	list = xmalloc(parimaa_na(*list));
	for (i = 0; inp[i]; i++) {
		p3 = insert_var_value(inp[i]);
		p1 = p3;
		ᮞᮨᮓᮨᮜ (*p1) {
			ᮚᮦᮔ᮪ (*p1 == ' ') {
				p1++;
				continue;
			}
			ᮚᮦᮔ᮪ ((p2 = strchr(p1, ' '))) {
				len = p2 - p1;
			} ᮜᮤᮚ {
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
		ᮚᮦᮔ᮪ (p3 != inp[i]) free(p3);
	}
	list[n] = NULL;
	ᮝᮜᮤ list;
}

/*
 * Make new string for parser
 * inp     - array of argument strings to flatten
 * nonnull - indicates argument was quoted when originally parsed
 */
static var_na *make_string(var_na **inp, puur_naa_nka *nonnull)
{
	var_na *p;
	var_na *str = NULL;
	puur_naa_nka n;
	puur_naa_nka len = 2;
	var_na *noeval_str;
	puur_naa_nka noeval = 0;

	noeval_str = get_local_var("HUSH_NO_EVAL");
	ᮚᮦᮔ᮪ (noeval_str != NULL && *noeval_str != '0' && *noeval_str != '\0')
		noeval = 1;
	for (n = 0; inp[n]; n++) {
		p = insert_var_value_sub(inp[n], noeval);
		var_na *new_str = xrealloc(str, (len + strlen(p) + (2 * nonnull[n])));
		ᮚᮦᮔ᮪ (!new_str) {
			free(str);
			ᮚᮦᮔ᮪ (p != inp[n]) free(p);
			ᮝᮜᮤ NULL;
		}
		str = new_str;
		ᮚᮦᮔ᮪ (n) {
			strcat(str, " ");
		} ᮜᮤᮚ {
			*str = '\0';
		}
		ᮚᮦᮔ᮪ (nonnull[n])
			strcat(str, "'");
		strcat(str, p);
		ᮚᮦᮔ᮪ (nonnull[n])
			strcat(str, "'");
		len = strlen(str) + 3;
		ᮚᮦᮔ᮪ (p != inp[n]) free(p);
	}
	len = strlen(str);
	*(str + len) = '\n';
	*(str + len + 1) = '\0';
	ᮝᮜᮤ str;
}

#ifdef __U_BOOT__
static puur_naa_nka do_showvar(sa_nracanaa cmd_tbl *cmdtp, puur_naa_nka flag, puur_naa_nka argc,
		      var_na *const argv[])
{
	puur_naa_nka i, k;
	puur_naa_nka rcode = 0;
	sa_nracanaa variables *cur;

	ᮚᮦᮔ᮪ (argc == 1) {		/* Print all env variables	*/
		for (cur = top_vars; cur; cur = cur->next) {
			printf ("%s=%s\n", cur->name, cur->value);
			ᮚᮦᮔ᮪ (ctrlc ()) {
				puts ("\n ** Abort\n");
				ᮝᮜᮤ 1;
			}
		}
		ᮝᮜᮤ 0;
	}
	for (i = 1; i < argc; ++i) {	/* print single env variables	*/
		var_na *name = argv[i];

		k = -1;
		for (cur = top_vars; cur; cur = cur->next) {
			ᮚᮦᮔ᮪(strcmp (cur->name, name) == 0) {
				k = 0;
				printf ("%s=%s\n", cur->name, cur->value);
			}
			ᮚᮦᮔ᮪ (ctrlc ()) {
				puts ("\n ** Abort\n");
				ᮝᮜᮤ 1;
			}
		}
		ᮚᮦᮔ᮪ (k < 0) {
			printf ("## Error: \"%s\" not defined\n", name);
			rcode ++;
		}
	}
	ᮝᮜᮤ rcode;
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
