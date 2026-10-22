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
#അന്തര്ഭാവ <env.h>
#അന്തര്ഭാവ <malloc.h>         /* malloc, free, realloc*/
#അന്തര്ഭാവ <linux/ctype.h>    /* isalpha, isdigit */
#അന്തര്ഭാവ <console.h>
#അന്തര്ഭാവ <bootretry.h>
#അന്തര്ഭാവ <cli.h>
#അന്തര്ഭാവ <cli_hush.h>
#അന്തര്ഭാവ <command.h>        /* find_cmd */
#അന്തര്ഭാവ <vsprintf.h>
#endif
#ifndef __U_BOOT__
#അന്തര്ഭാവ <ctype.h>     /* isalpha, isdigit */
#അന്തര്ഭാവ <unistd.h>    /* getpid */
#അന്തര്ഭാവ <stdlib.h>    /* getenv, atoi */
#അന്തര്ഭാവ <string.h>    /* strchr */
#അന്തര്ഭാവ <stdio.h>     /* popen etc. */
#അന്തര്ഭാവ <glob.h>      /* glob, of course */
#അന്തര്ഭാവ <stdarg.h>    /* va_list */
#അന്തര്ഭാവ <errno.h>
#അന്തര്ഭാവ <fcntl.h>
#അന്തര്ഭാവ <getopt.h>    /* should be pretty obvious */

#അന്തര്ഭാവ <sys/stat.h>  /* ulimit */
#അന്തര്ഭാവ <sys/types.h>
#അന്തര്ഭാവ <sys/wait.h>
#അന്തര്ഭാവ <signal.h>

/* #include <dmalloc.h> */

#എങ്കിൽ 1
#അന്തര്ഭാവ "busybox.h"
#അന്തര്ഭാവ "cmdedit.h"
#അല്ലെങ്കിൽ
#define applet_name "hush"
#അന്തര്ഭാവ "standalone.h"
#define hush_main പ്രധാന
#define BB_BANNER
#endif
#endif
#define SPECIAL_VAR_ചിഹ്നം 03
#define SUBSTED_VAR_ചിഹ്നം 04
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
#define error_സന്ദേശം printf
#അല്ലെങ്കിൽ
തരനിർവചനം എണ്ണൽ {
	REDIRECT_നിവേശം     = 1,
	REDIRECT_OVERWRITE = 2,
	REDIRECT_APPEND    = 3,
	REDIRECT_HEREIS    = 4,
	REDIRECT_IO        = 5
} redir_type;

/* The descrip member of this structure is only used to make debugging
 * output pretty */
ഘടന {പൂർണ്ണസംഖ്യ mode; പൂർണ്ണസംഖ്യ default_fd; അക്ഷരം *descrip;} redir_table[] = {
	{ 0,                         0, "()" },
	{ O_RDONLY,                  0, "<"  },
	{ O_CREAT|O_TRUNC|O_WRONLY,  1, ">"  },
	{ O_CREAT|O_APPEND|O_WRONLY, 1, ">>" },
	{ O_RDONLY,                 -1, "<<" },
	{ O_RDWR,                    1, "<>" }
};
#endif

തരനിർവചനം എണ്ണൽ {
	PIPE_SEQ = 1,
	PIPE_AND = 2,
	PIPE_OR  = 3,
	PIPE_BG  = 4,
} pipe_style;

/* might eventually control execution */
തരനിർവചനം എണ്ണൽ {
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
ഘടന p_context {
	ഘടന child_prog *child;
	ഘടന pipe *list_head;
	ഘടന pipe *pipe;
#ifndef __U_BOOT__
	ഘടന redir_struct *pending_redirect;
#endif
	reserved_style w;
	പൂർണ്ണസംഖ്യ old_flag;				/* for figuring out valid reserved words */
	ഘടന p_context *അട്ടി;
	പൂർണ്ണസംഖ്യ type;			/* define type of parser : ";$" common or special symbol */
	/* How about quoting status? */
};

#ifndef __U_BOOT__
ഘടന redir_struct {
	redir_type type;			/* type of redirection */
	പൂർണ്ണസംഖ്യ fd;						/* file descriptor being redirected */
	പൂർണ്ണസംഖ്യ dup;					/* -1, or file descriptor being duplicated */
	ഘടന redir_struct *next;	/* pointer to the next redirect in the list */
	glob_t word;				/* *word.gl_pathv is the filename */
};
#endif

ഘടന child_prog {
#ifndef __U_BOOT__
	pid_t pid;					/* 0 if exited */
#endif
	അക്ഷരം **argv;				/* program name and arguments */
	/* was quoted when parsed; copy of struct o_string.nonnull field */
	പൂർണ്ണസംഖ്യ *argv_nonnull;
#ifdef __U_BOOT__
	പൂർണ്ണസംഖ്യ    argc;                            /* number of program arguments */
#endif
	ഘടന pipe *group;			/* if non-NULL, first in group or subshell */
#ifndef __U_BOOT__
	പൂർണ്ണസംഖ്യ subshell;				/* flag, non-zero if group must be forked */
	ഘടന redir_struct *redirects;	/* I/O redirections */
	glob_t glob_result;			/* result of parameter globbing */
	പൂർണ്ണസംഖ്യ is_stopped;				/* is the program currently running? */
	ഘടന pipe *family;		/* pointer back to the child's parent pipe */
#endif
	പൂർണ്ണസംഖ്യ sp;				/* number of SPECIAL_VAR_SYMBOL */
	പൂർണ്ണസംഖ്യ type;
};

ഘടന pipe {
#ifndef __U_BOOT__
	പൂർണ്ണസംഖ്യ jobid;					/* job number */
#endif
	പൂർണ്ണസംഖ്യ അക്കം_progs;				/* total number of programs in job */
#ifndef __U_BOOT__
	പൂർണ്ണസംഖ്യ running_progs;			/* number of programs running */
	അക്ഷരം *text;					/* name of job */
	അക്ഷരം *cmdbuf;				/* buffer various argv's point into */
	pid_t pgrp;					/* process group ID for the job */
#endif
	ഘടന child_prog *progs;	/* array of commands in pipe */
	ഘടന pipe *next;			/* to track background commands */
#ifndef __U_BOOT__
	പൂർണ്ണസംഖ്യ stopped_progs;			/* number of programs alive, but stopped */
	പൂർണ്ണസംഖ്യ job_context;			/* bitmask defining current context */
#endif
	pipe_style followup;		/* PIPE_BG, PIPE_SEQ, PIPE_OR, PIPE_AND */
	reserved_style r_mode;		/* supports if, for, while, until */
};

#ifndef __U_BOOT__
ഘടന close_me {
	പൂർണ്ണസംഖ്യ fd;
	ഘടന close_me *next;
};
#endif

ഘടന variables {
	അക്ഷരം *name;
	അക്ഷരം *മൂല്യം;
	പൂർണ്ണസംഖ്യ flg_export;
	പൂർണ്ണസംഖ്യ flg_read_only;
	ഘടന variables *next;
};

/* globals, connect us to the outside world
 * the first three support $?, $#, and $1 */
#ifndef __U_BOOT__
അക്ഷരം **global_argv;
unsigned പൂർണ്ണസംഖ്യ global_argc;
#endif
സ്ഥിതം unsigned പൂർണ്ണസംഖ്യ last_return_code;
#ifndef __U_BOOT__
ബാഹ്യ അക്ഷരം **environ; /* This is in <unistd.h>, but protected with __USE_GNU */
#endif

/* "globals" within this file */
സ്ഥിതം uchar *ifs;
സ്ഥിതം അക്ഷരം map[256];
#ifndef __U_BOOT__
സ്ഥിതം പൂർണ്ണസംഖ്യ fake_mode;
സ്ഥിതം പൂർണ്ണസംഖ്യ interactive;
സ്ഥിതം ഘടന close_me *close_me_head;
സ്ഥിതം സ്ഥിരം അക്ഷരം *cwd;
സ്ഥിതം ഘടന pipe *job_list;
സ്ഥിതം unsigned പൂർണ്ണസംഖ്യ last_bg_pid;
സ്ഥിതം unsigned പൂർണ്ണസംഖ്യ last_jobid;
സ്ഥിതം unsigned പൂർണ്ണസംഖ്യ shell_terminal;
സ്ഥിതം അക്ഷരം *PS1;
സ്ഥിതം അക്ഷരം *PS2;
ഘടന variables shell_ver = { "HUSH_VERSION", "0.01", 1, 1, 0 };
ഘടന variables *മുകൾ_vars = &shell_ver;
#അല്ലെങ്കിൽ
സ്ഥിതം പൂർണ്ണസംഖ്യ flag_repeat = 0;
സ്ഥിതം പൂർണ്ണസംഖ്യ do_repeat = 0;
സ്ഥിതം ഘടന variables *മുകൾ_vars = ശൂന്യ ;
#endif /*__U_BOOT__ */

#define B_CHUNK (100)
#define B_NOSPAC 1

തരനിർവചനം ഘടന {
	അക്ഷരം *data;
	പൂർണ്ണസംഖ്യ length;
	പൂർണ്ണസംഖ്യ maxlen;
	പൂർണ്ണസംഖ്യ quote;
	പൂർണ്ണസംഖ്യ nonnull;
} o_string;
#define NULL_O_STRING {ശൂന്യ,0,0,0,0}
/* used for initialization:
	o_string foo = NULL_O_STRING; */

/* I can almost use ordinary FILE *.  Is open_memstream() universally
 * available?  Where is it documented? */
ഘടന in_str {
	സ്ഥിരം unsigned അക്ഷരം *p;
#ifndef __U_BOOT__
	അക്ഷരം peek_buf[2];
#endif
	പൂർണ്ണസംഖ്യ __promptme;
	പൂർണ്ണസംഖ്യ promptmode;
#ifndef __U_BOOT__
	FILE *file;
#endif
	പൂർണ്ണസംഖ്യ (*get) (ഘടന in_str *);
	പൂർണ്ണസംഖ്യ (*peek) (ഘടന in_str *);
};
#define b_getch(നിവേശം) ((നിവേശം)->get(നിവേശം))
#define b_peek(നിവേശം) ((നിവേശം)->peek(നിവേശം))

#ifndef __U_BOOT__
#define JOB_STATUS_FORMAT "[%d] %-22s %.40s\n"

ഘടന built_in_command {
	അക്ഷരം *cmd;					/* name */
	അക്ഷരം *descr;				/* description */
	പൂർണ്ണസംഖ്യ (*function) (ഘടന child_prog *);	/* function ptr */
};
#endif

/* define DEBUG_SHELL for debugging output (obviously ;-)) */
#എങ്കിൽ 0
#define DEBUG_SHELL
#endif

/* This should be in utility.c */
#ifdef DEBUG_SHELL
#ifndef __U_BOOT__
സ്ഥിതം ശൂന്യം debug_printf(സ്ഥിരം അക്ഷരം *format, ...)
{
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
}
#അല്ലെങ്കിൽ
#define debug_printf(fmt,args...)	printf (fmt ,##args)
#endif
#അല്ലെങ്കിൽ
സ്ഥിതം വരിയിൽ ശൂന്യം debug_printf(സ്ഥിരം അക്ഷരം *format, ...) { }
#endif
#define final_printf debug_printf

#ifdef __U_BOOT__
സ്ഥിതം ശൂന്യം syntax_err(ശൂന്യം) {
	 printf("syntax error\n");
}
#അല്ലെങ്കിൽ
സ്ഥിതം ശൂന്യം __syntax(അക്ഷരം *file, പൂർണ്ണസംഖ്യ വരി) {
	error_സന്ദേശം("syntax error %s:%d", file, വരി);
}
#define syntax() __syntax(__FILE__, __LINE__)
#endif

#ifdef __U_BOOT__
സ്ഥിതം ശൂന്യം *xmalloc(size_t size);
സ്ഥിതം ശൂന്യം *xrealloc(ശൂന്യം *ptr, size_t size);
#അല്ലെങ്കിൽ
/* Index of subroutines: */
/*   function prototypes for builtins */
സ്ഥിതം പൂർണ്ണസംഖ്യ builtin_cd(ഘടന child_prog *child);
സ്ഥിതം പൂർണ്ണസംഖ്യ builtin_env(ഘടന child_prog *child);
സ്ഥിതം പൂർണ്ണസംഖ്യ builtin_eval(ഘടന child_prog *child);
സ്ഥിതം പൂർണ്ണസംഖ്യ builtin_exec(ഘടന child_prog *child);
സ്ഥിതം പൂർണ്ണസംഖ്യ builtin_exit(ഘടന child_prog *child);
സ്ഥിതം പൂർണ്ണസംഖ്യ builtin_export(ഘടന child_prog *child);
സ്ഥിതം പൂർണ്ണസംഖ്യ builtin_fg_bg(ഘടന child_prog *child);
സ്ഥിതം പൂർണ്ണസംഖ്യ builtin_help(ഘടന child_prog *child);
സ്ഥിതം പൂർണ്ണസംഖ്യ builtin_jobs(ഘടന child_prog *child);
സ്ഥിതം പൂർണ്ണസംഖ്യ builtin_pwd(ഘടന child_prog *child);
സ്ഥിതം പൂർണ്ണസംഖ്യ builtin_read(ഘടന child_prog *child);
സ്ഥിതം പൂർണ്ണസംഖ്യ builtin_set(ഘടന child_prog *child);
സ്ഥിതം പൂർണ്ണസംഖ്യ builtin_shift(ഘടന child_prog *child);
സ്ഥിതം പൂർണ്ണസംഖ്യ builtin_source(ഘടന child_prog *child);
സ്ഥിതം പൂർണ്ണസംഖ്യ builtin_umask(ഘടന child_prog *child);
സ്ഥിതം പൂർണ്ണസംഖ്യ builtin_unset(ഘടന child_prog *child);
സ്ഥിതം പൂർണ്ണസംഖ്യ builtin_not_written(ഘടന child_prog *child);
#endif
/*   o_string manipulation: */
സ്ഥിതം പൂർണ്ണസംഖ്യ b_check_space(o_string *o, പൂർണ്ണസംഖ്യ len);
സ്ഥിതം പൂർണ്ണസംഖ്യ b_addchr(o_string *o, പൂർണ്ണസംഖ്യ ch);
സ്ഥിതം ശൂന്യം b_reset(o_string *o);
സ്ഥിതം പൂർണ്ണസംഖ്യ b_addqchr(o_string *o, പൂർണ്ണസംഖ്യ ch, പൂർണ്ണസംഖ്യ quote);
#ifndef __U_BOOT__
സ്ഥിതം പൂർണ്ണസംഖ്യ b_adduint(o_string *o, unsigned പൂർണ്ണസംഖ്യ i);
#endif
/*  in_str manipulations: */
സ്ഥിതം പൂർണ്ണസംഖ്യ static_get(ഘടന in_str *i);
സ്ഥിതം പൂർണ്ണസംഖ്യ static_peek(ഘടന in_str *i);
സ്ഥിതം പൂർണ്ണസംഖ്യ file_get(ഘടന in_str *i);
സ്ഥിതം പൂർണ്ണസംഖ്യ file_peek(ഘടന in_str *i);
#ifndef __U_BOOT__
സ്ഥിതം ശൂന്യം setup_file_in_str(ഘടന in_str *i, FILE *f);
#അല്ലെങ്കിൽ
സ്ഥിതം ശൂന്യം setup_file_in_str(ഘടന in_str *i);
#endif
സ്ഥിതം ശൂന്യം setup_string_in_str(ഘടന in_str *i, സ്ഥിരം അക്ഷരം *s);
#ifndef __U_BOOT__
/*  close_me manipulations: */
സ്ഥിതം ശൂന്യം mark_open(പൂർണ്ണസംഖ്യ fd);
സ്ഥിതം ശൂന്യം mark_closed(പൂർണ്ണസംഖ്യ fd);
സ്ഥിതം ശൂന്യം close_all(ശൂന്യം);
#endif
/*  "run" the final data structures: */
സ്ഥിതം അക്ഷരം *indenter(പൂർണ്ണസംഖ്യ i);
സ്ഥിതം പൂർണ്ണസംഖ്യ free_pipe_list(ഘടന pipe *head, പൂർണ്ണസംഖ്യ indent);
സ്ഥിതം പൂർണ്ണസംഖ്യ free_pipe(ഘടന pipe *pi, പൂർണ്ണസംഖ്യ indent);
/*  really run the final data structures: */
#ifndef __U_BOOT__
സ്ഥിതം പൂർണ്ണസംഖ്യ setup_redirects(ഘടന child_prog *prog, പൂർണ്ണസംഖ്യ squirrel[]);
#endif
സ്ഥിതം പൂർണ്ണസംഖ്യ run_list_real(ഘടന pipe *pi);
#ifndef __U_BOOT__
സ്ഥിതം ശൂന്യം pseudo_exec(ഘടന child_prog *child) __attribute__ ((noreturn));
#endif
സ്ഥിതം പൂർണ്ണസംഖ്യ run_pipe_real(ഘടന pipe *pi);
/*   extended glob support: */
#ifndef __U_BOOT__
സ്ഥിതം പൂർണ്ണസംഖ്യ globhack(സ്ഥിരം അക്ഷരം *src, പൂർണ്ണസംഖ്യ flags, glob_t *pglob);
സ്ഥിതം പൂർണ്ണസംഖ്യ glob_needed(സ്ഥിരം അക്ഷരം *s);
സ്ഥിതം പൂർണ്ണസംഖ്യ xglob(o_string *dest, പൂർണ്ണസംഖ്യ flags, glob_t *pglob);
#endif
/*   variable assignment: */
സ്ഥിതം പൂർണ്ണസംഖ്യ is_assignment(സ്ഥിരം അക്ഷരം *s);
/*   data structure manipulation: */
#ifndef __U_BOOT__
സ്ഥിതം പൂർണ്ണസംഖ്യ setup_redirect(ഘടന p_context *ctx, പൂർണ്ണസംഖ്യ fd, redir_type style, ഘടന in_str *നിവേശം);
#endif
സ്ഥിതം ശൂന്യം initialize_context(ഘടന p_context *ctx);
സ്ഥിതം പൂർണ്ണസംഖ്യ done_word(o_string *dest, ഘടന p_context *ctx);
സ്ഥിതം പൂർണ്ണസംഖ്യ done_command(ഘടന p_context *ctx);
സ്ഥിതം പൂർണ്ണസംഖ്യ done_pipe(ഘടന p_context *ctx, pipe_style type);
/*   primary string parsing: */
#ifndef __U_BOOT__
സ്ഥിതം പൂർണ്ണസംഖ്യ redirect_dup_അക്കം(ഘടന in_str *നിവേശം);
സ്ഥിതം പൂർണ്ണസംഖ്യ redirect_opt_അക്കം(o_string *o);
സ്ഥിതം പൂർണ്ണസംഖ്യ process_command_subs(o_string *dest, ഘടന p_context *ctx, ഘടന in_str *നിവേശം, പൂർണ്ണസംഖ്യ subst_end);
സ്ഥിതം പൂർണ്ണസംഖ്യ parse_group(o_string *dest, ഘടന p_context *ctx, ഘടന in_str *നിവേശം, പൂർണ്ണസംഖ്യ ch);
#endif
സ്ഥിതം അക്ഷരം *lookup_param(അക്ഷരം *src);
സ്ഥിതം അക്ഷരം *make_string(അക്ഷരം **inp, പൂർണ്ണസംഖ്യ *nonnull);
സ്ഥിതം പൂർണ്ണസംഖ്യ handle_dollar(o_string *dest, ഘടന p_context *ctx, ഘടന in_str *നിവേശം);
#ifndef __U_BOOT__
സ്ഥിതം പൂർണ്ണസംഖ്യ parse_string(o_string *dest, ഘടന p_context *ctx, സ്ഥിരം അക്ഷരം *src);
#endif
സ്ഥിതം പൂർണ്ണസംഖ്യ parse_stream(o_string *dest, ഘടന p_context *ctx, ഘടന in_str *input0, പൂർണ്ണസംഖ്യ end_trigger);
/*   setup: */
സ്ഥിതം പൂർണ്ണസംഖ്യ parse_stream_outer(ഘടന in_str *inp, പൂർണ്ണസംഖ്യ flag);
#ifndef __U_BOOT__
സ്ഥിതം പൂർണ്ണസംഖ്യ parse_string_outer(സ്ഥിരം അക്ഷരം *s, പൂർണ്ണസംഖ്യ flag);
സ്ഥിതം പൂർണ്ണസംഖ്യ parse_file_outer(FILE *f);
#endif
#ifndef __U_BOOT__
/*   job management: */
സ്ഥിതം പൂർണ്ണസംഖ്യ checkjobs(ഘടന pipe* fg_pipe);
സ്ഥിതം ശൂന്യം തിരുകുക_bg_job(ഘടന pipe *pi);
സ്ഥിതം ശൂന്യം remove_bg_job(ഘടന pipe *pi);
#endif
/*     local variable support */
സ്ഥിതം അക്ഷരം **make_list_in(അക്ഷരം **inp, അക്ഷരം *name);
സ്ഥിതം അക്ഷരം *തിരുകുക_var_മൂല്യം(അക്ഷരം *inp);
സ്ഥിതം അക്ഷരം *തിരുകുക_var_മൂല്യം_sub(അക്ഷരം *inp, പൂർണ്ണസംഖ്യ tag_subst);

#ifndef __U_BOOT__
/* Table of built-in functions.  They can be forked or not, depending on
 * context: within pipes, they fork.  As simple commands, they do not.
 * When used in non-forking context, they can change global variables
 * in the parent shell process.  If forked, of course they can not.
 * For example, 'unset foo | whatever' will parse and run, but foo will
 * still be set at the end. */
സ്ഥിതം ഘടന built_in_command bltins[] = {
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
	{ശൂന്യ, ശൂന്യ, ശൂന്യ}
};

സ്ഥിതം സ്ഥിരം അക്ഷരം *set_cwd(ശൂന്യം)
{
	എങ്കിൽ(cwd==unknown)
		cwd = ശൂന്യ;     /* xgetcwd(arg) called free(arg) */
	cwd = xgetcwd((അക്ഷരം *)cwd);
	എങ്കിൽ (!cwd)
		cwd = unknown;
	തിരികെ cwd;
}

/* built-in 'eval' handler */
സ്ഥിതം പൂർണ്ണസംഖ്യ builtin_eval(ഘടന child_prog *child)
{
	അക്ഷരം *str = ശൂന്യ;
	പൂർണ്ണസംഖ്യ rcode = EXIT_SUCCESS;

	എങ്കിൽ (child->argv[1]) {
		str = make_string(child->argv + 1);
		parse_string_outer(str, FLAG_EXIT_FROM_LOOP |
					FLAG_PARSE_SEMICOLON);
		സ്വതന്ത്രം(str);
		rcode = last_return_code;
	}
	തിരികെ rcode;
}

/* built-in 'cd <path>' handler */
സ്ഥിതം പൂർണ്ണസംഖ്യ builtin_cd(ഘടന child_prog *child)
{
	അക്ഷരം *newdir;
	എങ്കിൽ (child->argv[1] == ശൂന്യ)
		newdir = env_get("HOME");
	അല്ലെങ്കിൽ
		newdir = child->argv[1];
	എങ്കിൽ (chdir(newdir)) {
		printf("cd: %s: %s\n", newdir, strerror(errno));
		തിരികെ EXIT_FAILURE;
	}
	set_cwd();
	തിരികെ EXIT_SUCCESS;
}

/* built-in 'env' handler */
സ്ഥിതം പൂർണ്ണസംഖ്യ builtin_env(ഘടന child_prog *dummy)
{
	അക്ഷരം **e = environ;
	എങ്കിൽ (e == ശൂന്യ) തിരികെ EXIT_FAILURE;
	വേണ്ടി (; *e; e++) {
		puts(*e);
	}
	തിരികെ EXIT_SUCCESS;
}

/* built-in 'exec' handler */
സ്ഥിതം പൂർണ്ണസംഖ്യ builtin_exec(ഘടന child_prog *child)
{
	എങ്കിൽ (child->argv[1] == ശൂന്യ)
		തിരികെ EXIT_SUCCESS;   /* Really? */
	child->argv++;
	pseudo_exec(child);
	/* never returns */
}

/* built-in 'exit' handler */
സ്ഥിതം പൂർണ്ണസംഖ്യ builtin_exit(ഘടന child_prog *child)
{
	എങ്കിൽ (child->argv[1] == ശൂന്യ)
		exit(last_return_code);
	exit (atoi(child->argv[1]));
}

/* built-in 'export VAR=value' handler */
സ്ഥിതം പൂർണ്ണസംഖ്യ builtin_export(ഘടന child_prog *child)
{
	പൂർണ്ണസംഖ്യ res = 0;
	അക്ഷരം *name = child->argv[1];

	എങ്കിൽ (name == ശൂന്യ) {
		തിരികെ (builtin_env(child));
	}

	name = strdup(name);

	എങ്കിൽ(name) {
		അക്ഷരം *മൂല്യം = strchr(name, '=');

		എങ്കിൽ (!മൂല്യം) {
			അക്ഷരം *tmp;
			/* They are exporting something without an =VALUE */

			മൂല്യം = get_local_var(name);
			എങ്കിൽ (മൂല്യം) {
				size_t ln = strlen(name);

				tmp = realloc(name, ln+strlen(മൂല്യം)+2);
				എങ്കിൽ(tmp==ശൂന്യ)
					res = -1;
				അല്ലെങ്കിൽ {
					sprintf(tmp+ln, "=%s", മൂല്യം);
					name = tmp;
				}
			} അല്ലെങ്കിൽ {
				/* bash does not return an error when trying to export
				 * an undefined variable.  Do likewise. */
				res = 1;
			}
		}
	}
	എങ്കിൽ (res<0)
		perror_സന്ദേശം("export");
	അല്ലെങ്കിൽ എങ്കിൽ(res==0)
		res = set_local_var(name, 1);
	അല്ലെങ്കിൽ
		res = 0;
	സ്വതന്ത്രം(name);
	തിരികെ res;
}

/* built-in 'fg' and 'bg' handler */
സ്ഥിതം പൂർണ്ണസംഖ്യ builtin_fg_bg(ഘടന child_prog *child)
{
	പൂർണ്ണസംഖ്യ i, jobnum;
	ഘടന pipe *pi=ശൂന്യ;

	എങ്കിൽ (!interactive)
		തിരികെ EXIT_FAILURE;
	/* If they gave us no args, assume they want the last backgrounded task */
	എങ്കിൽ (!child->argv[1]) {
		വേണ്ടി (pi = job_list; pi; pi = pi->next) {
			എങ്കിൽ (pi->jobid == last_jobid) {
				നിർത്തുക;
			}
		}
		എങ്കിൽ (!pi) {
			error_സന്ദേശം("%s: no current job", child->argv[0]);
			തിരികെ EXIT_FAILURE;
		}
	} അല്ലെങ്കിൽ {
		എങ്കിൽ (sscanf(child->argv[1], "%%%d", &jobnum) != 1) {
			error_സന്ദേശം("%s: bad argument '%s'", child->argv[0], child->argv[1]);
			തിരികെ EXIT_FAILURE;
		}
		വേണ്ടി (pi = job_list; pi; pi = pi->next) {
			എങ്കിൽ (pi->jobid == jobnum) {
				നിർത്തുക;
			}
		}
		എങ്കിൽ (!pi) {
			error_സന്ദേശം("%s: %d: no such job", child->argv[0], jobnum);
			തിരികെ EXIT_FAILURE;
		}
	}

	എങ്കിൽ (*child->argv[0] == 'f') {
		/* Put the job into the foreground.  */
		tcsetpgrp(shell_terminal, pi->pgrp);
	}

	/* Restart the processes in the job */
	വേണ്ടി (i = 0; i < pi->അക്കം_progs; i++)
		pi->progs[i].is_stopped = 0;

	എങ്കിൽ ( (i=kill(- pi->pgrp, SIGCONT)) < 0) {
		എങ്കിൽ (i == ESRCH) {
			remove_bg_job(pi);
		} അല്ലെങ്കിൽ {
			perror_സന്ദേശം("kill (SIGCONT)");
		}
	}

	pi->stopped_progs = 0;
	തിരികെ EXIT_SUCCESS;
}

/* built-in 'help' handler */
സ്ഥിതം പൂർണ്ണസംഖ്യ builtin_help(ഘടന child_prog *dummy)
{
	ഘടന built_in_command *x;

	printf("\nBuilt-in commands:\n");
	printf("-------------------\n");
	വേണ്ടി (x = bltins; x->cmd; x++) {
		എങ്കിൽ (x->descr==ശൂന്യ)
			തുടരുക;
		printf("%s\t%s\n", x->cmd, x->descr);
	}
	printf("\n\n");
	തിരികെ EXIT_SUCCESS;
}

/* built-in 'jobs' handler */
സ്ഥിതം പൂർണ്ണസംഖ്യ builtin_jobs(ഘടന child_prog *child)
{
	ഘടന pipe *job;
	അക്ഷരം *status_string;

	വേണ്ടി (job = job_list; job; job = job->next) {
		എങ്കിൽ (job->running_progs == job->stopped_progs)
			status_string = "Stopped";
		അല്ലെങ്കിൽ
			status_string = "Running";

		printf(JOB_STATUS_FORMAT, job->jobid, status_string, job->text);
	}
	തിരികെ EXIT_SUCCESS;
}

/* built-in 'pwd' handler */
സ്ഥിതം പൂർണ്ണസംഖ്യ builtin_pwd(ഘടന child_prog *dummy)
{
	puts(set_cwd());
	തിരികെ EXIT_SUCCESS;
}

/* built-in 'read VAR' handler */
സ്ഥിതം പൂർണ്ണസംഖ്യ builtin_read(ഘടന child_prog *child)
{
	പൂർണ്ണസംഖ്യ res;

	എങ്കിൽ (child->argv[1]) {
		അക്ഷരം string[BUFSIZ];
		അക്ഷരം *var = 0;

		string[0] = 0;  /* In case stdin has only EOF */
		/* read string */
		fgets(string, വലുപ്പം(string), stdin);
		chomp(string);
		var = malloc(strlen(child->argv[1])+strlen(string)+2);
		എങ്കിൽ(var) {
			sprintf(var, "%s=%s", child->argv[1], string);
			res = set_local_var(var, 0);
		} അല്ലെങ്കിൽ
			res = -1;
		എങ്കിൽ (res)
			fprintf(stderr, "read: %m\n");
		സ്വതന്ത്രം(var);      /* So not move up to avoid breaking errno */
		തിരികെ res;
	} അല്ലെങ്കിൽ {
		ചെയ്യുക res=getchar(); ആകുമ്പോൾ(res!='\n' && res!=EOF);
		തിരികെ 0;
	}
}

/* built-in 'set VAR=value' handler */
സ്ഥിതം പൂർണ്ണസംഖ്യ builtin_set(ഘടന child_prog *child)
{
	അക്ഷരം *temp = child->argv[1];
	ഘടന variables *e;

	എങ്കിൽ (temp == ശൂന്യ)
		വേണ്ടി(e = മുകൾ_vars; e; e=e->next)
			printf("%s=%s\n", e->name, e->മൂല്യം);
	അല്ലെങ്കിൽ
		set_local_var(temp, 0);

		തിരികെ EXIT_SUCCESS;
}

/* Built-in 'shift' handler */
സ്ഥിതം പൂർണ്ണസംഖ്യ builtin_shift(ഘടന child_prog *child)
{
	പൂർണ്ണസംഖ്യ n=1;
	എങ്കിൽ (child->argv[1]) {
		n=atoi(child->argv[1]);
	}
	എങ്കിൽ (n>=0 && n<global_argc) {
		/* XXX This probably breaks $0 */
		global_argc -= n;
		global_argv += n;
		തിരികെ EXIT_SUCCESS;
	} അല്ലെങ്കിൽ {
		തിരികെ EXIT_FAILURE;
	}
}

/* Built-in '.' handler (read-in and execute commands from file) */
സ്ഥിതം പൂർണ്ണസംഖ്യ builtin_source(ഘടന child_prog *child)
{
	FILE *നിവേശം;
	പൂർണ്ണസംഖ്യ status;

	എങ്കിൽ (child->argv[1] == ശൂന്യ)
		തിരികെ EXIT_FAILURE;

	/* XXX search through $PATH is missing */
	നിവേശം = fopen(child->argv[1], "r");
	എങ്കിൽ (!നിവേശം) {
		error_സന്ദേശം("Couldn't open file '%s'", child->argv[1]);
		തിരികെ EXIT_FAILURE;
	}

	/* Now run the file */
	/* XXX argv and argc are broken; need to save old global_argv
	 * (pointer only is OK!) on this stack frame,
	 * set global_argv=child->argv+1, recurse, and restore. */
	mark_open(fileno(നിവേശം));
	status = parse_file_outer(നിവേശം);
	mark_closed(fileno(നിവേശം));
	fclose(നിവേശം);
	തിരികെ (status);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ builtin_umask(ഘടന child_prog *child)
{
	mode_t new_umask;
	സ്ഥിരം അക്ഷരം *arg = child->argv[1];
	അക്ഷരം *end;
	എങ്കിൽ (arg) {
		new_umask=strtoul(arg, &end, 8);
		എങ്കിൽ (*end!='\0' || end == arg) {
			തിരികെ EXIT_FAILURE;
		}
	} അല്ലെങ്കിൽ {
		printf("%.3o\n", (unsigned പൂർണ്ണസംഖ്യ) (new_umask=umask(0)));
	}
	umask(new_umask);
	തിരികെ EXIT_SUCCESS;
}

/* built-in 'unset VAR' handler */
സ്ഥിതം പൂർണ്ണസംഖ്യ builtin_unset(ഘടന child_prog *child)
{
	/* bash returned already true */
	unset_local_var(child->argv[1]);
	തിരികെ EXIT_SUCCESS;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ builtin_not_written(ഘടന child_prog *child)
{
	printf("builtin_%s not written\n",child->argv[0]);
	തിരികെ EXIT_FAILURE;
}
#endif

സ്ഥിതം പൂർണ്ണസംഖ്യ b_check_space(o_string *o, പൂർണ്ണസംഖ്യ len)
{
	/* It would be easy to drop a more restrictive policy
	 * in here, such as setting a maximum string length */
	എങ്കിൽ (o->length + len > o->maxlen) {
		അക്ഷരം *old_data = o->data;
		/* assert (data == NULL || o->maxlen != 0); */
		o->maxlen += max(2*len, B_CHUNK);
		o->data = realloc(o->data, 1 + o->maxlen);
		എങ്കിൽ (o->data == ശൂന്യ) {
			സ്വതന്ത്രം(old_data);
		}
	}
	തിരികെ o->data == ശൂന്യ;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ b_addchr(o_string *o, പൂർണ്ണസംഖ്യ ch)
{
	debug_printf("b_addchr: %c %d %p\n", ch, o->length, o);
	എങ്കിൽ (b_check_space(o, 1)) തിരികെ B_NOSPAC;
	o->data[o->length] = ch;
	o->length++;
	o->data[o->length] = '\0';
	തിരികെ 0;
}

സ്ഥിതം ശൂന്യം b_reset(o_string *o)
{
	o->length = 0;
	o->nonnull = 0;
	എങ്കിൽ (o->data != ശൂന്യ) *o->data = '\0';
}

സ്ഥിതം ശൂന്യം b_free(o_string *o)
{
	b_reset(o);
	സ്വതന്ത്രം(o->data);
	o->data = ശൂന്യ;
	o->maxlen = 0;
}

/* My analysis of quoting semantics tells me that state information
 * is associated with a destination, not a source.
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ b_addqchr(o_string *o, പൂർണ്ണസംഖ്യ ch, പൂർണ്ണസംഖ്യ quote)
{
	എങ്കിൽ (quote && strchr("*?[\\",ch)) {
		പൂർണ്ണസംഖ്യ rc;
		rc = b_addchr(o, '\\');
		എങ്കിൽ (rc) തിരികെ rc;
	}
	തിരികെ b_addchr(o, ch);
}

#ifndef __U_BOOT__
സ്ഥിതം പൂർണ്ണസംഖ്യ b_adduint(o_string *o, unsigned പൂർണ്ണസംഖ്യ i)
{
	പൂർണ്ണസംഖ്യ r;
	അക്ഷരം *p = simple_itoa(i);
	/* no escape checking necessary */
	ചെയ്യുക r=b_addchr(o, *p++); ആകുമ്പോൾ (r==0 && *p);
	തിരികെ r;
}
#endif

സ്ഥിതം പൂർണ്ണസംഖ്യ static_get(ഘടന in_str *i)
{
	പൂർണ്ണസംഖ്യ ch = *i->p++;
	എങ്കിൽ (ch=='\0') തിരികെ EOF;
	തിരികെ ch;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ static_peek(ഘടന in_str *i)
{
	തിരികെ *i->p;
}

#ifndef __U_BOOT__
സ്ഥിതം വരിയിൽ ശൂന്യം cmdedit_set_initial_prompt(ശൂന്യം)
{
#ifndef CONFIG_FEATURE_SH_FANCY_PROMPT
	PS1 = ശൂന്യ;
#അല്ലെങ്കിൽ
	PS1 = env_get("PS1");
	എങ്കിൽ(PS1==0)
		PS1 = "\\w \\$ ";
#endif
}

സ്ഥിതം വരിയിൽ ശൂന്യം setup_prompt_string(പൂർണ്ണസംഖ്യ promptmode, അക്ഷരം **prompt_str)
{
	debug_printf("setup_prompt_string %d ",promptmode);
#ifndef CONFIG_FEATURE_SH_FANCY_PROMPT
	/* Set up the prompt */
	എങ്കിൽ (promptmode == 1) {
		സ്വതന്ത്രം(PS1);
		PS1=xmalloc(strlen(cwd)+4);
		sprintf(PS1, "%s %s", cwd, ( geteuid() != 0 ) ?  "$ ":"# ");
		*prompt_str = PS1;
	} അല്ലെങ്കിൽ {
		*prompt_str = PS2;
	}
#അല്ലെങ്കിൽ
	*prompt_str = (promptmode==1)? PS1 : PS2;
#endif
	debug_printf("result %s\n",*prompt_str);
}
#endif

#ifdef __U_BOOT__
സ്ഥിതം പൂർണ്ണസംഖ്യ uboot_cli_readline(ഘടന in_str *i)
{
	അക്ഷരം *prompt;
	അക്ഷരം __maybe_unused *ps_prompt = ശൂന്യ;

	എങ്കിൽ (i->promptmode == 1)
		prompt = CONFIG_SYS_PROMPT;
	അല്ലെങ്കിൽ
		prompt = CONFIG_SYS_PROMPT_HUSH_PS2;

#ifdef CONFIG_CMDLINE_PS_SUPPORT
	എങ്കിൽ (i->promptmode == 1)
		ps_prompt = env_get("PS1");
	അല്ലെങ്കിൽ
		ps_prompt = env_get("PS2");
	എങ്കിൽ (ps_prompt)
		prompt = ps_prompt;
#endif

	തിരികെ cli_readline(prompt);
}
#endif

സ്ഥിതം ശൂന്യം get_user_നിവേശം(ഘടന in_str *i)
{
#ifndef __U_BOOT__
	അക്ഷരം *prompt_str;
	സ്ഥിതം അക്ഷരം the_command[BUFSIZ];

	setup_prompt_string(i->promptmode, &prompt_str);
#ifdef CONFIG_FEATURE_COMMAND_EDITING
	/*
	 ** enable command line editing only while a command line
	 ** is actually being read; otherwise, we'll end up bequeathing
	 ** atexit() handlers and other unwanted stuff to our
	 ** child processes (rob@sysgo.de)
	 */
	cmdedit_read_നിവേശം(prompt_str, the_command);
#അല്ലെങ്കിൽ
	fputs(prompt_str, stdout);
	fflush(stdout);
	the_command[0]=fgetc(i->file);
	the_command[1]='\0';
#endif
	fflush(stdout);
	i->p = the_command;
#അല്ലെങ്കിൽ
	പൂർണ്ണസംഖ്യ n;
	സ്ഥിതം അക്ഷരം the_command[CONFIG_SYS_CBSIZE + 1];

	bootretry_reset_cmd_timeout();
	i->__promptme = 1;
	n = uboot_cli_readline(i);

#ifdef CONFIG_BOOT_RETRY_TIME
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
#endif
	എങ്കിൽ (n == -1 ) {
		flag_repeat = 0;
		i->__promptme = 0;
	}
	n = strlen(console_buffer);
	console_buffer[n] = '\n';
	console_buffer[n+1]= '\0';
	എങ്കിൽ (had_ctrlc()) flag_repeat = 0;
	clear_ctrlc();
	do_repeat = 0;
	എങ്കിൽ (i->promptmode == 1) {
		എങ്കിൽ (console_buffer[0] == '\n'&& flag_repeat == 0) {
			strcpy(the_command,console_buffer);
		}
		അല്ലെങ്കിൽ {
			എങ്കിൽ (console_buffer[0] != '\n') {
				strcpy(the_command,console_buffer);
				flag_repeat = 1;
			}
			അല്ലെങ്കിൽ {
				do_repeat = 1;
			}
		}
		i->p = the_command;
	}
	അല്ലെങ്കിൽ {
		എങ്കിൽ (console_buffer[0] != '\n') {
			എങ്കിൽ (strlen(the_command) + strlen(console_buffer)
			    < CONFIG_SYS_CBSIZE) {
				n = strlen(the_command);
				the_command[n-1] = ' ';
				strcpy(&the_command[n],console_buffer);
			}
			അല്ലെങ്കിൽ {
				the_command[0] = '\n';
				the_command[1] = '\0';
				flag_repeat = 0;
			}
		}
		എങ്കിൽ (i->__promptme == 0) {
			the_command[0] = '\n';
			the_command[1] = '\0';
		}
		i->p = console_buffer;
	}
#endif
}

/* This is the magic location that prints prompts
 * and gets data back from the user */
സ്ഥിതം പൂർണ്ണസംഖ്യ file_get(ഘടന in_str *i)
{
	പൂർണ്ണസംഖ്യ ch;

	ch = 0;
	/* If there is data waiting, eat it up */
	എങ്കിൽ (i->p && *i->p) {
		ch = *i->p++;
	} അല്ലെങ്കിൽ {
		/* need to double check i->file because we might be doing something
		 * more complicated by now, like sourcing or substituting. */
#ifndef __U_BOOT__
		എങ്കിൽ (i->__promptme && interactive && i->file == stdin) {
			ആകുമ്പോൾ(! i->p || (interactive && strlen(i->p)==0) ) {
#അല്ലെങ്കിൽ
			ആകുമ്പോൾ(! i->p  || strlen(i->p)==0 ) {
#endif
				get_user_നിവേശം(i);
			}
			i->promptmode=2;
#ifndef __U_BOOT__
			i->__promptme = 0;
#endif
			എങ്കിൽ (i->p && *i->p) {
				ch = *i->p++;
			}
#ifndef __U_BOOT__
		} അല്ലെങ്കിൽ {
			ch = fgetc(i->file);
		}

#endif
		debug_printf("b_getch: got a %d\n", ch);
	}
#ifndef __U_BOOT__
	എങ്കിൽ (ch == '\n') i->__promptme=1;
#endif
	തിരികെ ch;
}

/* All the callers guarantee this routine will never be
 * used right after a newline, so prompting is not needed.
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ file_peek(ഘടന in_str *i)
{
#ifndef __U_BOOT__
	എങ്കിൽ (i->p && *i->p) {
#endif
		തിരികെ *i->p;
#ifndef __U_BOOT__
	} അല്ലെങ്കിൽ {
		i->peek_buf[0] = fgetc(i->file);
		i->peek_buf[1] = '\0';
		i->p = i->peek_buf;
		debug_printf("b_peek: got a %d\n", *i->p);
		തിരികെ *i->p;
	}
#endif
}

#ifndef __U_BOOT__
സ്ഥിതം ശൂന്യം setup_file_in_str(ഘടന in_str *i, FILE *f)
#അല്ലെങ്കിൽ
സ്ഥിതം ശൂന്യം setup_file_in_str(ഘടന in_str *i)
#endif
{
	i->peek = file_peek;
	i->get = file_get;
	i->__promptme=1;
	i->promptmode=1;
#ifndef __U_BOOT__
	i->file = f;
#endif
	i->p = ശൂന്യ;
}

സ്ഥിതം ശൂന്യം setup_string_in_str(ഘടന in_str *i, സ്ഥിരം അക്ഷരം *s)
{
	i->peek = static_peek;
	i->get = static_get;
	i->__promptme=1;
	i->promptmode=1;
	i->p = s;
}

#ifndef __U_BOOT__
സ്ഥിതം ശൂന്യം mark_open(പൂർണ്ണസംഖ്യ fd)
{
	ഘടന close_me *new = xmalloc(വലുപ്പം(ഘടന close_me));
	new->fd = fd;
	new->next = close_me_head;
	close_me_head = new;
}

സ്ഥിതം ശൂന്യം mark_closed(പൂർണ്ണസംഖ്യ fd)
{
	ഘടന close_me *tmp;
	എങ്കിൽ (close_me_head == ശൂന്യ || close_me_head->fd != fd)
		error_സന്ദേശം_and_die("corrupt close_me");
	tmp = close_me_head;
	close_me_head = close_me_head->next;
	സ്വതന്ത്രം(tmp);
}

സ്ഥിതം ശൂന്യം close_all(ശൂന്യം)
{
	ഘടന close_me *c;
	വേണ്ടി (c=close_me_head; c; c=c->next) {
		close(c->fd);
	}
	close_me_head = ശൂന്യ;
}

/* squirrel != NULL means we squirrel away copies of stdin, stdout,
 * and stderr if they are redirected. */
സ്ഥിതം പൂർണ്ണസംഖ്യ setup_redirects(ഘടന child_prog *prog, പൂർണ്ണസംഖ്യ squirrel[])
{
	പൂർണ്ണസംഖ്യ openfd, mode;
	ഘടന redir_struct *redir;

	വേണ്ടി (redir=prog->redirects; redir; redir=redir->next) {
		എങ്കിൽ (redir->dup == -1 && redir->word.gl_pathv == ശൂന്യ) {
			/* something went wrong in the parse.  Pretend it didn't happen */
			തുടരുക;
		}
		എങ്കിൽ (redir->dup == -1) {
			mode=redir_table[redir->type].mode;
			openfd = open(redir->word.gl_pathv[0], mode, 0666);
			എങ്കിൽ (openfd < 0) {
			/* this could get lost if stderr has been redirected, but
			   bash and ash both lose it as well (though zsh doesn't!) */
				perror_സന്ദേശം("error opening %s", redir->word.gl_pathv[0]);
				തിരികെ 1;
			}
		} അല്ലെങ്കിൽ {
			openfd = redir->dup;
		}

		എങ്കിൽ (openfd != redir->fd) {
			എങ്കിൽ (squirrel && redir->fd < 3) {
				squirrel[redir->fd] = dup(redir->fd);
			}
			എങ്കിൽ (openfd == -3) {
				close(openfd);
			} അല്ലെങ്കിൽ {
				dup2(openfd, redir->fd);
				എങ്കിൽ (redir->dup == -1)
					close (openfd);
			}
		}
	}
	തിരികെ 0;
}

സ്ഥിതം ശൂന്യം restore_redirects(പൂർണ്ണസംഖ്യ squirrel[])
{
	പൂർണ്ണസംഖ്യ i, fd;
	വേണ്ടി (i=0; i<3; i++) {
		fd = squirrel[i];
		എങ്കിൽ (fd != -1) {
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
സ്ഥിതം ശൂന്യം pseudo_exec(ഘടന child_prog *child)
{
	പൂർണ്ണസംഖ്യ i, rcode;
	അക്ഷരം *p;
	ഘടന built_in_command *x;
	എങ്കിൽ (child->argv) {
		വേണ്ടി (i=0; is_assignment(child->argv[i]); i++) {
			debug_printf("pid %d environment modification: %s\n",getpid(),child->argv[i]);
			p = തിരുകുക_var_മൂല്യം(child->argv[i]);
			putenv(strdup(p));
			എങ്കിൽ (p != child->argv[i]) സ്വതന്ത്രം(p);
		}
		child->argv+=i;  /* XXX this hack isn't so horrible, since we are about
					to exit, and therefore don't need to keep data
					structures consistent for free() use. */
		/* If a variable is assigned in a forest, and nobody listens,
		 * was it ever really set?
		 */
		എങ്കിൽ (child->argv[0] == ശൂന്യ) {
			_exit(EXIT_SUCCESS);
		}

		/*
		 * Check if the command matches any of the builtins.
		 * Depending on context, this might be redundant.  But it's
		 * easier to waste a few CPU cycles than it is to figure out
		 * if this is one of those cases.
		 */
		വേണ്ടി (x = bltins; x->cmd; x++) {
			എങ്കിൽ (strcmp(child->argv[0], x->cmd) == 0 ) {
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
			പൂർണ്ണസംഖ്യ argc_l;
			അക്ഷരം** argv_l=child->argv;
			അക്ഷരം *name = child->argv[0];

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
			വേണ്ടി(argc_l=0;*argv_l!=ശൂന്യ; argv_l++, argc_l++);
			optind = 1;
			debug_printf("running applet %s\n", name);
			run_applet_by_name(name, argc_l, child->argv);
		}
#endif
		debug_printf("exec of %s\n",child->argv[0]);
		execvp(child->argv[0],child->argv);
		perror_സന്ദേശം("couldn't exec: %s",child->argv[0]);
		_exit(1);
	} അല്ലെങ്കിൽ എങ്കിൽ (child->group) {
		debug_printf("runtime nesting to group\n");
		interactive=0;    /* crucial!!!! */
		rcode = run_list_real(child->group);
		/* OK to leak memory by not calling free_pipe_list,
		 * since this process is about to exit */
		_exit(rcode);
	} അല്ലെങ്കിൽ {
		/* Can happen.  See what bash does with ">foo" by itself. */
		debug_printf("trying to pseudo_exec null command\n");
		_exit(EXIT_SUCCESS);
	}
}

സ്ഥിതം ശൂന്യം തിരുകുക_bg_job(ഘടന pipe *pi)
{
	ഘടന pipe *thejob;

	/* Linear search for the ID of the job to use */
	pi->jobid = 1;
	വേണ്ടി (thejob = job_list; thejob; thejob = thejob->next)
		എങ്കിൽ (thejob->jobid >= pi->jobid)
			pi->jobid = thejob->jobid + 1;

	/* add thejob to the list of running jobs */
	എങ്കിൽ (!job_list) {
		thejob = job_list = xmalloc(വലുപ്പം(*thejob));
	} അല്ലെങ്കിൽ {
		വേണ്ടി (thejob = job_list; thejob->next; thejob = thejob->next) /* nothing */;
		thejob->next = xmalloc(വലുപ്പം(*thejob));
		thejob = thejob->next;
	}

	/* physically copy the struct job */
	memcpy(thejob, pi, വലുപ്പം(ഘടന pipe));
	thejob->next = ശൂന്യ;
	thejob->running_progs = thejob->അക്കം_progs;
	thejob->stopped_progs = 0;
	thejob->text = xmalloc(BUFSIZ); /* cmdedit buffer size */

	/*if (pi->progs[0] && pi->progs[0].argv && pi->progs[0].argv[0]) */
	{
		അക്ഷരം *bar=thejob->text;
		അക്ഷരം **foo=pi->progs[0].argv;
		ആകുമ്പോൾ(foo && *foo) {
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
സ്ഥിതം ശൂന്യം remove_bg_job(ഘടന pipe *pi)
{
	ഘടന pipe *prev_pipe;

	എങ്കിൽ (pi == job_list) {
		job_list = pi->next;
	} അല്ലെങ്കിൽ {
		prev_pipe = job_list;
		ആകുമ്പോൾ (prev_pipe->next != pi)
			prev_pipe = prev_pipe->next;
		prev_pipe->next = pi->next;
	}
	എങ്കിൽ (job_list)
		last_jobid = job_list->jobid;
	അല്ലെങ്കിൽ
		last_jobid = 0;

	pi->stopped_progs = 0;
	free_pipe(pi, 0);
	സ്വതന്ത്രം(pi);
}

/* Checks to see if any processes have exited -- if they
   have, figure out why and see if a job has completed */
സ്ഥിതം പൂർണ്ണസംഖ്യ checkjobs(ഘടന pipe* fg_pipe)
{
	പൂർണ്ണസംഖ്യ attributes;
	പൂർണ്ണസംഖ്യ status;
	പൂർണ്ണസംഖ്യ prognum = 0;
	ഘടന pipe *pi;
	pid_t childpid;

	attributes = WUNTRACED;
	എങ്കിൽ (fg_pipe==ശൂന്യ) {
		attributes |= WNOHANG;
	}

	ആകുമ്പോൾ ((childpid = waitpid(-1, &status, attributes)) > 0) {
		എങ്കിൽ (fg_pipe) {
			പൂർണ്ണസംഖ്യ i, rcode = 0;
			വേണ്ടി (i=0; i < fg_pipe->അക്കം_progs; i++) {
				എങ്കിൽ (fg_pipe->progs[i].pid == childpid) {
					എങ്കിൽ (i==fg_pipe->അക്കം_progs-1)
						rcode=WEXITSTATUS(status);
					(fg_pipe->അക്കം_progs)--;
					തിരികെ(rcode);
				}
			}
		}

		വേണ്ടി (pi = job_list; pi; pi = pi->next) {
			prognum = 0;
			ആകുമ്പോൾ (prognum < pi->അക്കം_progs && pi->progs[prognum].pid != childpid) {
				prognum++;
			}
			എങ്കിൽ (prognum < pi->അക്കം_progs)
				നിർത്തുക;
		}

		എങ്കിൽ(pi==ശൂന്യ) {
			debug_printf("checkjobs: pid %d was not in our list!\n", childpid);
			തുടരുക;
		}

		എങ്കിൽ (WIFEXITED(status) || WIFSIGNALED(status)) {
			/* child exited */
			pi->running_progs--;
			pi->progs[prognum].pid = 0;

			എങ്കിൽ (!pi->running_progs) {
				printf(JOB_STATUS_FORMAT, pi->jobid, "Done", pi->text);
				remove_bg_job(pi);
			}
		} അല്ലെങ്കിൽ {
			/* child stopped */
			pi->stopped_progs++;
			pi->progs[prognum].is_stopped = 1;

#എങ്കിൽ 0
			/* Printing this stuff is a pain, since it tends to
			 * overwrite the prompt an inconveinient moments.  So
			 * don't do that.  */
			എങ്കിൽ (pi->stopped_progs == pi->അക്കം_progs) {
				printf("\n"JOB_STATUS_FORMAT, pi->jobid, "Stopped", pi->text);
			}
#endif
		}
	}

	എങ്കിൽ (childpid == -1 && errno != ECHILD)
		perror_സന്ദേശം("waitpid");

	/* move the shell to the foreground */
	/*if (interactive && tcsetpgrp(shell_terminal, getpgid(0))) */
	/*	perror_msg("tcsetpgrp-2"); */
	തിരികെ -1;
}

/* Figure out our controlling tty, checking in order stderr,
 * stdin, and stdout.  If check_pgrp is set, also check that
 * we belong to the foreground process group associated with
 * that tty.  The value of shell_terminal is needed in order to call
 * tcsetpgrp(shell_terminal, ...); */
ശൂന്യം controlling_tty(പൂർണ്ണസംഖ്യ check_pgrp)
{
	pid_t curpgrp;

	എങ്കിൽ ((curpgrp = tcgetpgrp(shell_terminal = 2)) < 0
			&& (curpgrp = tcgetpgrp(shell_terminal = 0)) < 0
			&& (curpgrp = tcgetpgrp(shell_terminal = 1)) < 0)
		പോകുക shell_terminal_error;

	എങ്കിൽ (check_pgrp && curpgrp != getpgid(0))
		പോകുക shell_terminal_error;

	തിരികെ;

shell_terminal_error:
		shell_terminal = -1;
		തിരികെ;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ run_pipe_real(ഘടന pipe *pi)
{
	പൂർണ്ണസംഖ്യ i;
#ifndef __U_BOOT__
	പൂർണ്ണസംഖ്യ nextin, nextout;
	പൂർണ്ണസംഖ്യ pipefds[2];				/* pipefds[0] is for reading */
	ഘടന child_prog *child;
	ഘടന built_in_command *x;
	അക്ഷരം *p;
# എങ്കിൽ __GNUC__
	/* Avoid longjmp clobbering */
	(ശൂന്യം) &i;
	(ശൂന്യം) &nextin;
	(ശൂന്യം) &nextout;
	(ശൂന്യം) &child;
# endif
#അല്ലെങ്കിൽ
	പൂർണ്ണസംഖ്യ nextin;
	പൂർണ്ണസംഖ്യ flag = do_repeat ? CMD_FLAG_REPEAT : 0;
	ഘടന child_prog *child;
	അക്ഷരം *p;
# എങ്കിൽ __GNUC__
	/* Avoid longjmp clobbering */
	(ശൂന്യം) &i;
	(ശൂന്യം) &nextin;
	(ശൂന്യം) &child;
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
	എങ്കിൽ (pi->അക്കം_progs == 1) child = & (pi->progs[0]);
#ifndef __U_BOOT__
	എങ്കിൽ (pi->അക്കം_progs == 1 && child->group && child->subshell == 0) {
		പൂർണ്ണസംഖ്യ squirrel[] = {-1, -1, -1};
		പൂർണ്ണസംഖ്യ rcode;
		debug_printf("non-subshell grouping\n");
		setup_redirects(child, squirrel);
		/* XXX could we merge code with following builtin case,
		 * by creating a pseudo builtin that calls run_list_real? */
		rcode = run_list_real(child->group);
		restore_redirects(squirrel);
#അല്ലെങ്കിൽ
		എങ്കിൽ (pi->അക്കം_progs == 1 && child->group) {
		പൂർണ്ണസംഖ്യ rcode;
		debug_printf("non-subshell grouping\n");
		rcode = run_list_real(child->group);
#endif
		തിരികെ rcode;
	} അല്ലെങ്കിൽ എങ്കിൽ (pi->അക്കം_progs == 1 && pi->progs[0].argv != ശൂന്യ) {
		വേണ്ടി (i=0; is_assignment(child->argv[i]); i++) { /* nothing */ }
		എങ്കിൽ (i!=0 && child->argv[i]==ശൂന്യ) {
			/* assignments, but no command: set the local environment */
			വേണ്ടി (i=0; child->argv[i]!=ശൂന്യ; i++) {

				/* Ok, this case is tricky.  We have to decide if this is a
				 * local variable, or an already exported variable.  If it is
				 * already exported, we have to export the new value.  If it is
				 * not exported, we need only set this as a local variable.
				 * This junk is all to decide whether or not to export this
				 * variable. */
				പൂർണ്ണസംഖ്യ export_me=0;
				അക്ഷരം *name, *മൂല്യം;
				name = xstrdup(child->argv[i]);
				debug_printf("Local environment set: %s\n", name);
				മൂല്യം = strchr(name, '=');
				എങ്കിൽ (മൂല്യം)
					*മൂല്യം=0;
#ifndef __U_BOOT__
				എങ്കിൽ ( get_local_var(name)) {
					export_me=1;
				}
#endif
				സ്വതന്ത്രം(name);
				p = തിരുകുക_var_മൂല്യം(child->argv[i]);
				set_local_var(p, export_me);
				എങ്കിൽ (p != child->argv[i]) സ്വതന്ത്രം(p);
			}
			തിരികെ EXIT_SUCCESS;   /* don't worry about errors in set_local_var() yet */
		}
		വേണ്ടി (i = 0; is_assignment(child->argv[i]); i++) {
			p = തിരുകുക_var_മൂല്യം(child->argv[i]);
#ifndef __U_BOOT__
			putenv(strdup(p));
#അല്ലെങ്കിൽ
			set_local_var(p, 0);
#endif
			എങ്കിൽ (p != child->argv[i]) {
				child->sp--;
				സ്വതന്ത്രം(p);
			}
		}
		എങ്കിൽ (child->sp) {
			അക്ഷരം * str = ശൂന്യ;

			str = make_string(child->argv + i,
					  child->argv_nonnull + i);
			parse_string_outer(str, FLAG_EXIT_FROM_LOOP | FLAG_REPARSING);
			സ്വതന്ത്രം(str);
			തിരികെ last_return_code;
		}
#ifndef __U_BOOT__
		വേണ്ടി (x = bltins; x->cmd; x++) {
			എങ്കിൽ (strcmp(child->argv[i], x->cmd) == 0 ) {
				പൂർണ്ണസംഖ്യ squirrel[] = {-1, -1, -1};
				പൂർണ്ണസംഖ്യ rcode;
				എങ്കിൽ (x->function == builtin_exec && child->argv[i+1]==ശൂന്യ) {
					debug_printf("magic exec\n");
					setup_redirects(child,ശൂന്യ);
					തിരികെ EXIT_SUCCESS;
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
			തിരികെ rcode;
		}
#അല്ലെങ്കിൽ
		/* check ";", because ,example , argv consist from
		 * "help;flinfo" must not execute
		 */
		എങ്കിൽ (strchr(child->argv[i], ';')) {
			printf("Unknown command '%s' - try 'help' or use "
					"'run' command\n", child->argv[i]);
			തിരികെ -1;
		}
		/* Process the command */
		തിരികെ cmd_process(flag, child->argc - i, child->argv + i,
				   &flag_repeat, ശൂന്യ);
#endif
	}
#ifndef __U_BOOT__

	വേണ്ടി (i = 0; i < pi->അക്കം_progs; i++) {
		child = & (pi->progs[i]);

		/* pipes are inserted between pairs of commands */
		എങ്കിൽ ((i + 1) < pi->അക്കം_progs) {
			എങ്കിൽ (pipe(pipefds)<0) perror_സന്ദേശം_and_die("pipe");
			nextout = pipefds[1];
		} അല്ലെങ്കിൽ {
			nextout=1;
			pipefds[0] = -1;
		}

		/* XXX test for failed fork()? */
		എങ്കിൽ (!(child->pid = fork())) {
			/* Set the handling for job control signals back to the default.  */
			സൂചന(SIGINT, SIG_DFL);
			സൂചന(SIGQUIT, SIG_DFL);
			സൂചന(SIGTERM, SIG_DFL);
			സൂചന(SIGTSTP, SIG_DFL);
			സൂചന(SIGTTIN, SIG_DFL);
			സൂചന(SIGTTOU, SIG_DFL);
			സൂചന(SIGCHLD, SIG_DFL);

			close_all();

			എങ്കിൽ (nextin != 0) {
				dup2(nextin, 0);
				close(nextin);
			}
			എങ്കിൽ (nextout != 1) {
				dup2(nextout, 1);
				close(nextout);
			}
			എങ്കിൽ (pipefds[0]!=-1) {
				close(pipefds[0]);  /* opposite end of our output pipe */
			}

			/* Like bash, explicit redirects override pipes,
			 * and the pipe fd is available for dup'ing. */
			setup_redirects(child,ശൂന്യ);

			എങ്കിൽ (interactive && pi->followup!=PIPE_BG) {
				/* If we (the child) win the race, put ourselves in the process
				 * group whose leader is the first process in this pipe. */
				എങ്കിൽ (pi->pgrp < 0) {
					pi->pgrp = getpid();
				}
				എങ്കിൽ (setpgid(0, pi->pgrp) == 0) {
					tcsetpgrp(2, pi->pgrp);
				}
			}

			pseudo_exec(child);
		}

		/* put our child in the process group whose leader is the
		   first process in this pipe */
		എങ്കിൽ (pi->pgrp < 0) {
			pi->pgrp = child->pid;
		}
		/* Don't check for errors.  The child may be dead already,
		 * in which case setpgid returns error code EACCES. */
		setpgid(child->pid, pi->pgrp);

		എങ്കിൽ (nextin != 0)
			close(nextin);
		എങ്കിൽ (nextout != 1)
			close(nextout);

		/* If there isn't another process, nextin is garbage
		   but it doesn't matter */
		nextin = pipefds[0];
	}
#endif
	തിരികെ -1;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ run_list_real(ഘടന pipe *pi)
{
	അക്ഷരം *save_name = ശൂന്യ;
	അക്ഷരം **list = ശൂന്യ;
	അക്ഷരം **save_list = ശൂന്യ;
	ഘടന pipe *rpipe;
	പൂർണ്ണസംഖ്യ flag_rep = 0;
#ifndef __U_BOOT__
	പൂർണ്ണസംഖ്യ save_അക്കം_progs;
#endif
	പൂർണ്ണസംഖ്യ rcode=0, flag_skip=1;
	പൂർണ്ണസംഖ്യ flag_restore = 0;
	പൂർണ്ണസംഖ്യ if_code=0, next_if_code=0;  /* need double-buffer to handle elif */
	reserved_style rmode, skip_more_in_this_rmode=RES_XXXX;
	/* check syntax for "for" */
	വേണ്ടി (rpipe = pi; rpipe; rpipe = rpipe->next) {
		എങ്കിൽ ((rpipe->r_mode == RES_IN ||
		    rpipe->r_mode == RES_FOR) &&
		    (rpipe->next == ശൂന്യ)) {
				syntax();
#ifdef __U_BOOT__
				flag_repeat = 0;
#endif
				തിരികെ 1;
		}
		എങ്കിൽ ((rpipe->r_mode == RES_IN &&
			(rpipe->next->r_mode == RES_IN &&
			rpipe->next->progs->argv != ശൂന്യ))||
			(rpipe->r_mode == RES_FOR &&
			rpipe->next->r_mode != RES_IN)) {
				syntax();
#ifdef __U_BOOT__
				flag_repeat = 0;
#endif
				തിരികെ 1;
		}
	}
	വേണ്ടി (; pi; pi = (flag_restore != 0) ? rpipe : pi->next) {
		എങ്കിൽ (pi->r_mode == RES_WHILE || pi->r_mode == RES_UNTIL ||
			pi->r_mode == RES_FOR) {
#ifdef __U_BOOT__
				/* check Ctrl-C */
				ctrlc();
				എങ്കിൽ ((had_ctrlc())) {
					തിരികെ 1;
				}
#endif
				flag_restore = 0;
				എങ്കിൽ (!rpipe) {
					flag_rep = 0;
					rpipe = pi;
				}
		}
		rmode = pi->r_mode;
		debug_printf("rmode=%d  if_code=%d  next_if_code=%d skip_more=%d\n", rmode, if_code, next_if_code, skip_more_in_this_rmode);
		എങ്കിൽ (rmode == skip_more_in_this_rmode && flag_skip) {
			എങ്കിൽ (pi->followup == PIPE_SEQ) flag_skip=0;
			തുടരുക;
		}
		flag_skip = 1;
		skip_more_in_this_rmode = RES_XXXX;
		എങ്കിൽ (rmode == RES_THEN || rmode == RES_ELSE) if_code = next_if_code;
		എങ്കിൽ (rmode == RES_THEN &&  if_code) തുടരുക;
		എങ്കിൽ (rmode == RES_ELSE && !if_code) തുടരുക;
		എങ്കിൽ (rmode == RES_ELIF && !if_code) നിർത്തുക;
		എങ്കിൽ (rmode == RES_FOR && pi->അക്കം_progs) {
			എങ്കിൽ (!list) {
				/* if no variable values after "in" we skip "for" */
				എങ്കിൽ (!pi->next->progs->argv) തുടരുക;
				/* create list of variable values */
				list = make_list_in(pi->next->progs->argv,
					pi->progs->argv[0]);
				save_list = list;
				save_name = pi->progs->argv[0];
				pi->progs->argv[0] = ശൂന്യ;
				flag_rep = 1;
			}
			എങ്കിൽ (!(*list)) {
				സ്വതന്ത്രം(pi->progs->argv[0]);
				സ്വതന്ത്രം(save_list);
				list = ശൂന്യ;
				flag_rep = 0;
				pi->progs->argv[0] = save_name;
#ifndef __U_BOOT__
				pi->progs->glob_result.gl_pathv[0] =
					pi->progs->argv[0];
#endif
				തുടരുക;
			} അല്ലെങ്കിൽ {
				/* insert new value from list for variable */
				സ്വതന്ത്രം(pi->progs->argv[0]);
				pi->progs->argv[0] = *list++;
#ifndef __U_BOOT__
				pi->progs->glob_result.gl_pathv[0] =
					pi->progs->argv[0];
#endif
			}
		}
		എങ്കിൽ (rmode == RES_IN) തുടരുക;
		എങ്കിൽ (rmode == RES_DO) {
			എങ്കിൽ (!flag_rep) തുടരുക;
		}
		എങ്കിൽ (rmode == RES_DONE) {
			എങ്കിൽ (flag_rep) {
				flag_restore = 1;
			} അല്ലെങ്കിൽ {
				rpipe = ശൂന്യ;
			}
		}
		എങ്കിൽ (pi->അക്കം_progs == 0) തുടരുക;
#ifndef __U_BOOT__
		save_അക്കം_progs = pi->അക്കം_progs; /* save number of programs */
#endif
		rcode = run_pipe_real(pi);
		debug_printf("run_pipe_real returned %d\n",rcode);
#ifndef __U_BOOT__
		എങ്കിൽ (rcode!=-1) {
			/* We only ran a builtin: rcode was set by the return value
			 * of run_pipe_real(), and we don't need to wait for anything. */
		} അല്ലെങ്കിൽ എങ്കിൽ (pi->followup==PIPE_BG) {
			/* XXX check bash's behavior with nontrivial pipes */
			/* XXX compute jobid */
			/* XXX what does bash do with attempts to background builtins? */
			തിരുകുക_bg_job(pi);
			rcode = EXIT_SUCCESS;
		} അല്ലെങ്കിൽ {
			എങ്കിൽ (interactive) {
				/* move the new process group into the foreground */
				എങ്കിൽ (tcsetpgrp(shell_terminal, pi->pgrp) && errno != ENOTTY)
					perror_സന്ദേശം("tcsetpgrp-3");
				rcode = checkjobs(pi);
				/* move the shell to the foreground */
				എങ്കിൽ (tcsetpgrp(shell_terminal, getpgid(0)) && errno != ENOTTY)
					perror_സന്ദേശം("tcsetpgrp-4");
			} അല്ലെങ്കിൽ {
				rcode = checkjobs(pi);
			}
			debug_printf("checkjobs returned %d\n",rcode);
		}
		last_return_code=rcode;
#അല്ലെങ്കിൽ
		എങ്കിൽ (rcode < -1) {
			last_return_code = -rcode - 2;
			തിരികെ -2;	/* exit */
		}
		last_return_code = rcode;
#endif
#ifndef __U_BOOT__
		pi->അക്കം_progs = save_അക്കം_progs; /* restore number of programs */
#endif
		എങ്കിൽ ( rmode == RES_IF || rmode == RES_ELIF )
			next_if_code=rcode;  /* can be overwritten a number of times */
		എങ്കിൽ (rmode == RES_WHILE)
			flag_rep = !last_return_code;
		എങ്കിൽ (rmode == RES_UNTIL)
			flag_rep = last_return_code;
		എങ്കിൽ ( (rcode==EXIT_SUCCESS && pi->followup==PIPE_OR) ||
		     (rcode!=EXIT_SUCCESS && pi->followup==PIPE_AND) )
			skip_more_in_this_rmode=rmode;
#ifndef __U_BOOT__
		checkjobs(ശൂന്യ);
#endif
	}
	തിരികെ rcode;
}

/* broken, of course, but OK for testing */
സ്ഥിതം അക്ഷരം *indenter(പൂർണ്ണസംഖ്യ i)
{
	സ്ഥിതം അക്ഷരം blanks[]="                                    ";
	തിരികെ &blanks[വലുപ്പം(blanks)-i-1];
}

/* return code is the exit status of the pipe */
സ്ഥിതം പൂർണ്ണസംഖ്യ free_pipe(ഘടന pipe *pi, പൂർണ്ണസംഖ്യ indent)
{
	അക്ഷരം **p;
	ഘടന child_prog *child;
#ifndef __U_BOOT__
	ഘടന redir_struct *r, *rnext;
#endif
	പൂർണ്ണസംഖ്യ a, i, ret_code=0;
	അക്ഷരം *ind = indenter(indent);

#ifndef __U_BOOT__
	എങ്കിൽ (pi->stopped_progs > 0)
		തിരികെ ret_code;
	final_printf("%s run pipe: (pid %d)\n",ind,getpid());
#endif
	വേണ്ടി (i=0; i<pi->അക്കം_progs; i++) {
		child = &pi->progs[i];
		final_printf("%s  command %d:\n",ind,i);
		എങ്കിൽ (child->argv) {
			വേണ്ടി (a=0,p=child->argv; *p; a++,p++) {
				final_printf("%s   argv[%d] = %s\n",ind,a,*p);
			}
#ifndef __U_BOOT__
			globfree(&child->glob_result);
#അല്ലെങ്കിൽ
			വേണ്ടി (a = 0; a < child->argc; a++) {
				സ്വതന്ത്രം(child->argv[a]);
			}
			സ്വതന്ത്രം(child->argv);
			സ്വതന്ത്രം(child->argv_nonnull);
			child->argc = 0;
#endif
			child->argv=ശൂന്യ;
		} അല്ലെങ്കിൽ എങ്കിൽ (child->group) {
#ifndef __U_BOOT__
			final_printf("%s   begin group (subshell:%d)\n",ind, child->subshell);
#endif
			ret_code = free_pipe_list(child->group,indent+3);
			final_printf("%s   end group\n",ind);
		} അല്ലെങ്കിൽ {
			final_printf("%s   (nil)\n",ind);
		}
#ifndef __U_BOOT__
		വേണ്ടി (r=child->redirects; r; r=rnext) {
			final_printf("%s   redirect %d%s", ind, r->fd, redir_table[r->type].descrip);
			എങ്കിൽ (r->dup == -1) {
				/* guard against the case >$FOO, where foo is unset or blank */
				എങ്കിൽ (r->word.gl_pathv) {
					final_printf(" %s\n", *r->word.gl_pathv);
					globfree(&r->word);
				}
			} അല്ലെങ്കിൽ {
				final_printf("&%d\n", r->dup);
			}
			rnext=r->next;
			സ്വതന്ത്രം(r);
		}
		child->redirects=ശൂന്യ;
#endif
	}
	സ്വതന്ത്രം(pi->progs);   /* children are an array, they get freed all at once */
	pi->progs=ശൂന്യ;
	തിരികെ ret_code;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ free_pipe_list(ഘടന pipe *head, പൂർണ്ണസംഖ്യ indent)
{
	പൂർണ്ണസംഖ്യ rcode=0;   /* if list has no members */
	ഘടന pipe *pi, *next;
	അക്ഷരം *ind = indenter(indent);
	വേണ്ടി (pi=head; pi; pi=next) {
		final_printf("%s pipe reserved mode %d\n", ind, pi->r_mode);
		rcode = free_pipe(pi, indent);
		final_printf("%s pipe followup code %d\n", ind, pi->followup);
		next=pi->next;
		pi->next=ശൂന്യ;
		സ്വതന്ത്രം(pi);
	}
	തിരികെ rcode;
}

/* Select which version we will use */
സ്ഥിതം പൂർണ്ണസംഖ്യ run_list(ഘടന pipe *pi)
{
	പൂർണ്ണസംഖ്യ rcode=0;
#ifndef __U_BOOT__
	എങ്കിൽ (fake_mode==0) {
#endif
		rcode = run_list_real(pi);
#ifndef __U_BOOT__
	}
#endif
	/* free_pipe_list has the side effect of clearing memory
	 * In the long run that function can be merged with run_list_real,
	 * but doing that now would hobble the debugging effort. */
	free_pipe_list(pi,0);
	തിരികെ rcode;
}

/* The API for glob is arguably broken.  This routine pushes a non-matching
 * string into the output structure, removing non-backslashed backslashes.
 * If someone can prove me wrong, by performing this function within the
 * original glob(3) api, feel free to rewrite this routine into oblivion.
 * Return code (0 vs. GLOB_NOSPACE) matches glob(3).
 * XXX broken if the last character is '\\', check that before calling.
 */
#ifndef __U_BOOT__
സ്ഥിതം പൂർണ്ണസംഖ്യ globhack(സ്ഥിരം അക്ഷരം *src, പൂർണ്ണസംഖ്യ flags, glob_t *pglob)
{
	പൂർണ്ണസംഖ്യ cnt=0, pathc;
	സ്ഥിരം അക്ഷരം *s;
	അക്ഷരം *dest;
	വേണ്ടി (cnt=1, s=src; s && *s; s++) {
		എങ്കിൽ (*s == '\\') s++;
		cnt++;
	}
	dest = malloc(cnt);
	എങ്കിൽ (!dest) തിരികെ GLOB_NOSPACE;
	എങ്കിൽ (!(flags & GLOB_APPEND)) {
		pglob->gl_pathv=ശൂന്യ;
		pglob->gl_pathc=0;
		pglob->gl_offs=0;
		pglob->gl_offs=0;
	}
	pathc = ++pglob->gl_pathc;
	pglob->gl_pathv = realloc(pglob->gl_pathv, (pathc+1)*വലുപ്പം(*pglob->gl_pathv));
	എങ്കിൽ (pglob->gl_pathv == ശൂന്യ) തിരികെ GLOB_NOSPACE;
	pglob->gl_pathv[pathc-1]=dest;
	pglob->gl_pathv[pathc]=ശൂന്യ;
	വേണ്ടി (s=src; s && *s; s++, dest++) {
		എങ്കിൽ (*s == '\\') s++;
		*dest = *s;
	}
	*dest='\0';
	തിരികെ 0;
}

/* XXX broken if the last character is '\\', check that before calling */
സ്ഥിതം പൂർണ്ണസംഖ്യ glob_needed(സ്ഥിരം അക്ഷരം *s)
{
	വേണ്ടി (; *s; s++) {
		എങ്കിൽ (*s == '\\') s++;
		എങ്കിൽ (strchr("*[?",*s)) തിരികെ 1;
	}
	തിരികെ 0;
}

#എങ്കിൽ 0
സ്ഥിതം ശൂന്യം globprint(glob_t *pglob)
{
	പൂർണ്ണസംഖ്യ i;
	debug_printf("glob_t at %p:\n", pglob);
	debug_printf("  gl_pathc=%d  gl_pathv=%p  gl_offs=%d  gl_flags=%d\n",
		pglob->gl_pathc, pglob->gl_pathv, pglob->gl_offs, pglob->gl_flags);
	വേണ്ടി (i=0; i<pglob->gl_pathc; i++)
		debug_printf("pglob->gl_pathv[%d] = %p = %s\n", i,
			pglob->gl_pathv[i], pglob->gl_pathv[i]);
}
#endif

സ്ഥിതം പൂർണ്ണസംഖ്യ xglob(o_string *dest, പൂർണ്ണസംഖ്യ flags, glob_t *pglob)
{
	പൂർണ്ണസംഖ്യ gr;

	/* short-circuit for null word */
	/* we can code this better when the debug_printf's are gone */
	എങ്കിൽ (dest->length == 0) {
		എങ്കിൽ (dest->nonnull) {
			/* bash man page calls this an "explicit" null */
			gr = globhack(dest->data, flags, pglob);
			debug_printf("globhack returned %d\n",gr);
		} അല്ലെങ്കിൽ {
			തിരികെ 0;
		}
	} അല്ലെങ്കിൽ എങ്കിൽ (glob_needed(dest->data)) {
		gr = glob(dest->data, flags, ശൂന്യ, pglob);
		debug_printf("glob returned %d\n",gr);
		എങ്കിൽ (gr == GLOB_NOMATCH) {
			/* quote removal, or more accurately, backslash removal */
			gr = globhack(dest->data, flags, pglob);
			debug_printf("globhack returned %d\n",gr);
		}
	} അല്ലെങ്കിൽ {
		gr = globhack(dest->data, flags, pglob);
		debug_printf("globhack returned %d\n",gr);
	}
	എങ്കിൽ (gr == GLOB_NOSPACE)
		error_സന്ദേശം_and_die("out of memory during glob");
	എങ്കിൽ (gr != 0) { /* GLOB_ABORTED ? */
		error_സന്ദേശം("glob(3) error %d",gr);
	}
	/* globprint(glob_target); */
	തിരികെ gr;
}
#endif

#ifdef __U_BOOT__
സ്ഥിതം അക്ഷരം *get_dollar_var(അക്ഷരം ch);
#endif

/* This is used to get/check local shell variables */
അക്ഷരം *get_local_var(സ്ഥിരം അക്ഷരം *s)
{
	ഘടന variables *cur;

	എങ്കിൽ (!s)
		തിരികെ ശൂന്യ;

#ifdef __U_BOOT__
	എങ്കിൽ (*s == '$')
		തിരികെ get_dollar_var(s[1]);
#endif

	വേണ്ടി (cur = മുകൾ_vars; cur; cur=cur->next)
		എങ്കിൽ(strcmp(cur->name, s)==0)
			തിരികെ cur->മൂല്യം;
	തിരികെ ശൂന്യ;
}

/* This is used to set local shell variables
   flg_export==0 if only local (not exporting) variable
   flg_export==1 if "new" exporting environ
   flg_export>1  if current startup environ (not call putenv()) */
പൂർണ്ണസംഖ്യ set_local_var(സ്ഥിരം അക്ഷരം *s, പൂർണ്ണസംഖ്യ flg_export)
{
	അക്ഷരം *name, *മൂല്യം;
	പൂർണ്ണസംഖ്യ result=0;
	ഘടന variables *cur;

#ifdef __U_BOOT__
	/* might be possible! */
	എങ്കിൽ (!isalpha(*s))
		തിരികെ -1;
#endif

	name=strdup(s);

	/* Assume when we enter this function that we are already in
	 * NAME=VALUE format.  So the first order of business is to
	 * split 's' on the '=' into 'name' and 'value' */
	മൂല്യം = strchr(name, '=');
	എങ്കിൽ (!മൂല്യം) {
		സ്വതന്ത്രം(name);
		തിരികെ -1;
	}
	*മൂല്യം++ = 0;

	എങ്കിൽ (!*മൂല്യം) {
		unset_local_var(name);
		സ്വതന്ത്രം(name);
		തിരികെ 0;
	}

	വേണ്ടി(cur = മുകൾ_vars; cur; cur = cur->next) {
		എങ്കിൽ(strcmp(cur->name, name)==0)
			നിർത്തുക;
	}

	എങ്കിൽ(cur) {
		എങ്കിൽ(strcmp(cur->മൂല്യം, മൂല്യം)==0) {
			എങ്കിൽ(flg_export>0 && cur->flg_export==0)
				cur->flg_export=flg_export;
			അല്ലെങ്കിൽ
				result++;
		} അല്ലെങ്കിൽ {
			എങ്കിൽ(cur->flg_read_only) {
				error_സന്ദേശം("%s: readonly variable", name);
				result = -1;
			} അല്ലെങ്കിൽ {
				എങ്കിൽ(flg_export>0 || cur->flg_export>1)
					cur->flg_export=1;
				സ്വതന്ത്രം(cur->മൂല്യം);

				cur->മൂല്യം = strdup(മൂല്യം);
			}
		}
	} അല്ലെങ്കിൽ {
		cur = malloc(വലുപ്പം(ഘടന variables));
		എങ്കിൽ(!cur) {
			result = -1;
		} അല്ലെങ്കിൽ {
			cur->name = strdup(name);
			എങ്കിൽ (cur->name == ശൂന്യ) {
				സ്വതന്ത്രം(cur);
				result = -1;
			} അല്ലെങ്കിൽ {
				ഘടന variables *bottom = മുകൾ_vars;
				cur->മൂല്യം = strdup(മൂല്യം);
				cur->next = ശൂന്യ;
				cur->flg_export = flg_export;
				cur->flg_read_only = 0;
				ആകുമ്പോൾ(bottom->next) bottom=bottom->next;
				bottom->next = cur;
			}
		}
	}

#ifndef __U_BOOT__
	എങ്കിൽ(result==0 && cur->flg_export==1) {
		*(മൂല്യം-1) = '=';
		result = putenv(name);
	} അല്ലെങ്കിൽ {
#endif
		സ്വതന്ത്രം(name);
#ifndef __U_BOOT__
		എങ്കിൽ(result>0)            /* equivalent to previous set */
			result = 0;
	}
#endif
	തിരികെ result;
}

ശൂന്യം unset_local_var(സ്ഥിരം അക്ഷരം *name)
{
	ഘടന variables *cur;

	എങ്കിൽ (name) {
		വേണ്ടി (cur = മുകൾ_vars; cur; cur=cur->next) {
			എങ്കിൽ(strcmp(cur->name, name)==0)
				നിർത്തുക;
		}
		എങ്കിൽ (cur != ശൂന്യ) {
			ഘടന variables *next = മുകൾ_vars;
			എങ്കിൽ(cur->flg_read_only) {
				error_സന്ദേശം("%s: readonly variable", name);
				തിരികെ;
			} അല്ലെങ്കിൽ {
#ifndef __U_BOOT__
				എങ്കിൽ(cur->flg_export)
					unenv_set(cur->name);
#endif
				സ്വതന്ത്രം(cur->name);
				സ്വതന്ത്രം(cur->മൂല്യം);
				ആകുമ്പോൾ (next->next != cur)
					next = next->next;
				next->next = cur->next;
			}
			സ്വതന്ത്രം(cur);
		}
	}
}

സ്ഥിതം പൂർണ്ണസംഖ്യ is_assignment(സ്ഥിരം അക്ഷരം *s)
{
	എങ്കിൽ (s == ശൂന്യ)
		തിരികെ 0;

	എങ്കിൽ (!isalpha(*s)) തിരികെ 0;
	++s;
	ആകുമ്പോൾ(isalnum(*s) || *s=='_') ++s;
	തിരികെ *s=='=';
}

#ifndef __U_BOOT__
/* the src parameter allows us to peek forward to a possible &n syntax
 * for file descriptor duplication, e.g., "2>&1".
 * Return code is 0 normally, 1 if a syntax error is detected in src.
 * Resource errors (in xmalloc) cause the process to exit */
സ്ഥിതം പൂർണ്ണസംഖ്യ setup_redirect(ഘടന p_context *ctx, പൂർണ്ണസംഖ്യ fd, redir_type style,
	ഘടന in_str *നിവേശം)
{
	ഘടന child_prog *child=ctx->child;
	ഘടന redir_struct *redir = child->redirects;
	ഘടന redir_struct *last_redir=ശൂന്യ;

	/* Create a new redir_struct and drop it onto the end of the linked list */
	ആകുമ്പോൾ(redir) {
		last_redir=redir;
		redir=redir->next;
	}
	redir = xmalloc(വലുപ്പം(ഘടന redir_struct));
	redir->next=ശൂന്യ;
	redir->word.gl_pathv=ശൂന്യ;
	എങ്കിൽ (last_redir) {
		last_redir->next=redir;
	} അല്ലെങ്കിൽ {
		child->redirects=redir;
	}

	redir->type=style;
	redir->fd= (fd==-1) ? redir_table[style].default_fd : fd ;

	debug_printf("Redirect type %d%s\n", redir->fd, redir_table[style].descrip);

	/* Check for a '2>&1' type redirect */
	redir->dup = redirect_dup_അക്കം(നിവേശം);
	എങ്കിൽ (redir->dup == -2) തിരികെ 1;  /* syntax error */
	എങ്കിൽ (redir->dup != -1) {
		/* Erik had a check here that the file descriptor in question
		 * is legit; I postpone that to "run time"
		 * A "-" representation of "close me" shows up as a -3 here */
		debug_printf("Duplicating redirect '%d>&%d'\n", redir->fd, redir->dup);
	} അല്ലെങ്കിൽ {
		/* We do _not_ try to open the file that src points to,
		 * since we need to return and let src be expanded first.
		 * Set ctx->pending_redirect, so we know what to do at the
		 * end of the next parsed word.
		 */
		ctx->pending_redirect = redir;
	}
	തിരികെ 0;
}
#endif

സ്ഥിതം ഘടന pipe *new_pipe(ശൂന്യം)
{
	ഘടന pipe *pi;
	pi = xmalloc(വലുപ്പം(ഘടന pipe));
	pi->അക്കം_progs = 0;
	pi->progs = ശൂന്യ;
	pi->next = ശൂന്യ;
	pi->followup = 0;  /* invalid */
	pi->r_mode = RES_NONE;
	തിരികെ pi;
}

സ്ഥിതം ശൂന്യം initialize_context(ഘടന p_context *ctx)
{
	ctx->pipe=ശൂന്യ;
#ifndef __U_BOOT__
	ctx->pending_redirect=ശൂന്യ;
#endif
	ctx->child=ശൂന്യ;
	ctx->list_head=new_pipe();
	ctx->pipe=ctx->list_head;
	ctx->w=RES_NONE;
	ctx->അട്ടി=ശൂന്യ;
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
ഘടന reserved_combo {
	അക്ഷരം *literal;
	പൂർണ്ണസംഖ്യ code;
	long flag;
};
/* Mostly a list of accepted follow-up reserved words.
 * FLAG_END means we are done with the sequence, and are ready
 * to turn the compound list into a command.
 * FLAG_START means the word must start a new compound list.
 */
സ്ഥിതം ഘടന reserved_combo reserved_list[] = {
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
#define NRES (വലുപ്പം(reserved_list)/വലുപ്പം(ഘടന reserved_combo))

സ്ഥിതം പൂർണ്ണസംഖ്യ reserved_word(o_string *dest, ഘടന p_context *ctx)
{
	ഘടന reserved_combo *r;
	വേണ്ടി (r=reserved_list;
		r<reserved_list+NRES; r++) {
		എങ്കിൽ (strcmp(dest->data, r->literal) == 0) {
			debug_printf("found reserved word %s, code %d\n",r->literal,r->code);
			എങ്കിൽ (r->flag & FLAG_START) {
				ഘടന p_context *new = xmalloc(വലുപ്പം(ഘടന p_context));
				debug_printf("push stack\n");
				എങ്കിൽ (ctx->w == RES_IN || ctx->w == RES_FOR) {
					syntax();
					സ്വതന്ത്രം(new);
					ctx->w = RES_SNTX;
					b_reset(dest);
					തിരികെ 1;
				}
				*new = *ctx;   /* physical copy */
				initialize_context(ctx);
				ctx->അട്ടി=new;
			} അല്ലെങ്കിൽ എങ്കിൽ ( ctx->w == RES_NONE || ! (ctx->old_flag & (1<<r->code))) {
				syntax();
				ctx->w = RES_SNTX;
				b_reset(dest);
				തിരികെ 1;
			}
			ctx->w=r->code;
			ctx->old_flag = r->flag;
			എങ്കിൽ (ctx->old_flag & FLAG_END) {
				ഘടന p_context *old;
				debug_printf("pop stack\n");
				done_pipe(ctx,PIPE_SEQ);
				old = ctx->അട്ടി;
				old->child->group = ctx->list_head;
#ifndef __U_BOOT__
				old->child->subshell = 0;
#endif
				*ctx = *old;   /* physical copy */
				സ്വതന്ത്രം(old);
			}
			b_reset (dest);
			തിരികെ 1;
		}
	}
	തിരികെ 0;
}

/* normal return is 0.
 * Syntax or xglob errors return 1. */
സ്ഥിതം പൂർണ്ണസംഖ്യ done_word(o_string *dest, ഘടന p_context *ctx)
{
	ഘടന child_prog *child=ctx->child;
#ifndef __U_BOOT__
	glob_t *glob_target;
	പൂർണ്ണസംഖ്യ gr, flags = 0;
#അല്ലെങ്കിൽ
	അക്ഷരം *str, *s;
	പൂർണ്ണസംഖ്യ argc, cnt;
#endif

	debug_printf("done_word: %s %p\n", dest->data, child);
	എങ്കിൽ (dest->length == 0 && !dest->nonnull) {
		debug_printf("  true null, ignored\n");
		തിരികെ 0;
	}
#ifndef __U_BOOT__
	എങ്കിൽ (ctx->pending_redirect) {
		glob_target = &ctx->pending_redirect->word;
	} അല്ലെങ്കിൽ {
#endif
		എങ്കിൽ (child->group) {
			syntax();
			തിരികെ 1;  /* syntax error, groups and arglists don't mix */
		}
		എങ്കിൽ (!child->argv && (ctx->type & FLAG_PARSE_SEMICOLON)) {
			debug_printf("checking %s for reserved-ness\n",dest->data);
			എങ്കിൽ (reserved_word(dest,ctx)) തിരികെ ctx->w==RES_SNTX;
		}
#ifndef __U_BOOT__
		glob_target = &child->glob_result;
		എങ്കിൽ (child->argv) flags |= GLOB_APPEND;
#അല്ലെങ്കിൽ
		വേണ്ടി (cnt = 1, s = dest->data; s && *s; s++) {
			എങ്കിൽ (*s == '\\') s++;
			cnt++;
		}
		str = malloc(cnt);
		എങ്കിൽ (!str) തിരികെ 1;
		എങ്കിൽ ( child->argv == ശൂന്യ) {
			child->argc=0;
		}
		argc = ++child->argc;
		child->argv = realloc(child->argv, (argc+1)*വലുപ്പം(*child->argv));
		എങ്കിൽ (child->argv == ശൂന്യ) {
			സ്വതന്ത്രം(str);
			തിരികെ 1;
		}
		child->argv_nonnull = realloc(child->argv_nonnull,
					(argc+1)*വലുപ്പം(*child->argv_nonnull));
		എങ്കിൽ (child->argv_nonnull == ശൂന്യ) {
			സ്വതന്ത്രം(str);
			തിരികെ 1;
		}
		child->argv[argc-1]=str;
		child->argv_nonnull[argc-1] = dest->nonnull;
		child->argv[argc]=ശൂന്യ;
		child->argv_nonnull[argc] = 0;
		വേണ്ടി (s = dest->data; s && *s; s++,str++) {
			എങ്കിൽ (*s == '\\') s++;
			*str = *s;
		}
		*str = '\0';
#endif
#ifndef __U_BOOT__
	}
	gr = xglob(dest, flags, glob_target);
	എങ്കിൽ (gr != 0) തിരികെ 1;
#endif

	b_reset(dest);
#ifndef __U_BOOT__
	എങ്കിൽ (ctx->pending_redirect) {
		ctx->pending_redirect=ശൂന്യ;
		എങ്കിൽ (glob_target->gl_pathc != 1) {
			error_സന്ദേശം("ambiguous redirect");
			തിരികെ 1;
		}
	} അല്ലെങ്കിൽ {
		child->argv = glob_target->gl_pathv;
	}
#endif
	എങ്കിൽ (ctx->w == RES_FOR) {
		done_word(dest,ctx);
		done_pipe(ctx,PIPE_SEQ);
	}
	തിരികെ 0;
}

/* The only possible error here is out of memory, in which case
 * xmalloc exits. */
സ്ഥിതം പൂർണ്ണസംഖ്യ done_command(ഘടന p_context *ctx)
{
	/* The child is really already in the pipe structure, so
	 * advance the pipe counter and make a new, null child.
	 * Only real trickiness here is that the uncommitted
	 * child structure, to which ctx->child points, is not
	 * counted in pi->num_progs. */
	ഘടന pipe *pi=ctx->pipe;
	ഘടന child_prog *prog=ctx->child;

	എങ്കിൽ (prog && prog->group == ശൂന്യ
		 && prog->argv == ശൂന്യ
#ifndef __U_BOOT__
		 && prog->redirects == ശൂന്യ) {
#അല്ലെങ്കിൽ
										) {
#endif
		debug_printf("done_command: skipping null command\n");
		തിരികെ 0;
	} അല്ലെങ്കിൽ എങ്കിൽ (prog) {
		pi->അക്കം_progs++;
		debug_printf("done_command: num_progs incremented to %d\n",pi->അക്കം_progs);
	} അല്ലെങ്കിൽ {
		debug_printf("done_command: initializing\n");
	}
	pi->progs = xrealloc(pi->progs, വലുപ്പം(*pi->progs) * (pi->അക്കം_progs+1));

	prog = pi->progs + pi->അക്കം_progs;
#ifndef __U_BOOT__
	prog->redirects = ശൂന്യ;
#endif
	prog->argv = ശൂന്യ;
	prog->argv_nonnull = ശൂന്യ;
#ifndef __U_BOOT__
	prog->is_stopped = 0;
#endif
	prog->group = ശൂന്യ;
#ifndef __U_BOOT__
	prog->glob_result.gl_pathv = ശൂന്യ;
	prog->family = pi;
#endif
	prog->sp = 0;
	ctx->child = prog;
	prog->type = ctx->type;

	/* but ctx->pipe and ctx->list_head remain unchanged */
	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ done_pipe(ഘടന p_context *ctx, pipe_style type)
{
	ഘടന pipe *new_p;
	done_command(ctx);  /* implicit closure of previous command */
	debug_printf("done_pipe, type %d\n", type);
	ctx->pipe->followup = type;
	ctx->pipe->r_mode = ctx->w;
	new_p=new_pipe();
	ctx->pipe->next = new_p;
	ctx->pipe = new_p;
	ctx->child = ശൂന്യ;
	done_command(ctx);  /* set up new pipe to accept commands */
	തിരികെ 0;
}

#ifndef __U_BOOT__
/* peek ahead in the in_str to find out if we have a "&n" construct,
 * as in "2>&1", that represents duplicating a file descriptor.
 * returns either -2 (syntax error), -1 (no &), or the number found.
 */
സ്ഥിതം പൂർണ്ണസംഖ്യ redirect_dup_അക്കം(ഘടന in_str *നിവേശം)
{
	പൂർണ്ണസംഖ്യ ch, d=0, ok=0;
	ch = b_peek(നിവേശം);
	എങ്കിൽ (ch != '&') തിരികെ -1;

	b_getch(നിവേശം);  /* get the & */
	ch=b_peek(നിവേശം);
	എങ്കിൽ (ch == '-') {
		b_getch(നിവേശം);
		തിരികെ -3;  /* "-" represents "close me" */
	}
	ആകുമ്പോൾ (isdigit(ch)) {
		d = d*10+(ch-'0');
		ok=1;
		b_getch(നിവേശം);
		ch = b_peek(നിവേശം);
	}
	എങ്കിൽ (ok) തിരികെ d;

	error_സന്ദേശം("ambiguous redirect");
	തിരികെ -2;
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
സ്ഥിതം പൂർണ്ണസംഖ്യ redirect_opt_അക്കം(o_string *o)
{
	പൂർണ്ണസംഖ്യ അക്കം;

	എങ്കിൽ (o->length==0) തിരികെ -1;
	വേണ്ടി(അക്കം=0; അക്കം<o->length; അക്കം++) {
		എങ്കിൽ (!isdigit(*(o->data+അക്കം))) {
			തിരികെ -1;
		}
	}
	/* reuse num (and save an int) */
	അക്കം=atoi(o->data);
	b_reset(o);
	തിരികെ അക്കം;
}

FILE *generate_stream_from_list(ഘടന pipe *head)
{
	FILE *pf;
#എങ്കിൽ 1
	പൂർണ്ണസംഖ്യ pid, channel[2];
	എങ്കിൽ (pipe(channel)<0) perror_സന്ദേശം_and_die("pipe");
	pid=fork();
	എങ്കിൽ (pid<0) {
		perror_സന്ദേശം_and_die("fork");
	} അല്ലെങ്കിൽ എങ്കിൽ (pid==0) {
		close(channel[0]);
		എങ്കിൽ (channel[1] != 1) {
			dup2(channel[1],1);
			close(channel[1]);
		}
#എങ്കിൽ 0
#define SURROGATE "surrogate response"
		ലിഖ(1,SURROGATE,വലുപ്പം(SURROGATE));
		_exit(run_list(head));
#അല്ലെങ്കിൽ
		_exit(run_list_real(head));   /* leaks memory */
#endif
	}
	debug_printf("forked child %d\n",pid);
	close(channel[1]);
	pf = fdopen(channel[0],"r");
	debug_printf("pipe on FILE *%p\n",pf);
#അല്ലെങ്കിൽ
	free_pipe_list(head,0);
	pf=popen("echo surrogate response","r");
	debug_printf("started fake pipe on FILE *%p\n",pf);
#endif
	തിരികെ pf;
}

/* this version hacked for testing purposes */
/* return code is exit status of the process that is run. */
സ്ഥിതം പൂർണ്ണസംഖ്യ process_command_subs(o_string *dest, ഘടന p_context *ctx, ഘടന in_str *നിവേശം, പൂർണ്ണസംഖ്യ subst_end)
{
	പൂർണ്ണസംഖ്യ retcode;
	o_string result=NULL_O_STRING;
	ഘടന p_context inner;
	FILE *p;
	ഘടന in_str pipe_str;
	initialize_context(&inner);

	/* recursion to generate command */
	retcode = parse_stream(&result, &inner, നിവേശം, subst_end);
	എങ്കിൽ (retcode != 0) തിരികെ retcode;  /* syntax error or EOF */
	done_word(&result, &inner);
	done_pipe(&inner, PIPE_SEQ);
	b_free(&result);

	p=generate_stream_from_list(inner.list_head);
	എങ്കിൽ (p==ശൂന്യ) തിരികെ 1;
	mark_open(fileno(p));
	setup_file_in_str(&pipe_str, p);

	/* now send results of command back into original context */
	retcode = parse_stream(dest, ctx, &pipe_str, '\0');
	/* XXX In case of a syntax error, should we try to kill the child?
	 * That would be tough to do right, so just read until EOF. */
	എങ്കിൽ (retcode == 1) {
		ആകുമ്പോൾ (b_getch(&pipe_str)!=EOF) { /* discard */ };
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
	തിരികെ retcode;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ parse_group(o_string *dest, ഘടന p_context *ctx,
	ഘടന in_str *നിവേശം, പൂർണ്ണസംഖ്യ ch)
{
	പൂർണ്ണസംഖ്യ rcode, endch=0;
	ഘടന p_context sub;
	ഘടന child_prog *child = ctx->child;
	എങ്കിൽ (child->argv) {
		syntax();
		തിരികെ 1;  /* syntax error, groups and arglists don't mix */
	}
	initialize_context(&sub);
	തിരഞ്ഞെടുപ്പ്(ch) {
		അവസ്ഥ '(': endch=')'; child->subshell=1; നിർത്തുക;
		അവസ്ഥ '{': endch='}'; നിർത്തുക;
		സ്ഥിരസ്ഥിതി: syntax();   /* really logic error */
	}
	rcode=parse_stream(dest,&sub,നിവേശം,endch);
	done_word(dest,&sub); /* finish off the final word in the subcontext */
	done_pipe(&sub, PIPE_SEQ);  /* and the final command there, too */
	child->group = sub.list_head;
	തിരികെ rcode;
	/* child remains "open", available for possible redirects */
}
#endif

/* basically useful version until someone wants to get fancier,
 * see the bash man page under "Parameter Expansion" */
സ്ഥിതം അക്ഷരം *lookup_param(അക്ഷരം *src)
{
	അക്ഷരം *p;
	അക്ഷരം *sep;
	അക്ഷരം *default_val = ശൂന്യ;
	പൂർണ്ണസംഖ്യ assign = 0;
	പൂർണ്ണസംഖ്യ expand_empty = 0;

	എങ്കിൽ (!src)
		തിരികെ ശൂന്യ;

	sep = strchr(src, ':');

	എങ്കിൽ (sep) {
		*sep = '\0';
		എങ്കിൽ (*(sep + 1) == '-')
			default_val = sep+2;
		എങ്കിൽ (*(sep + 1) == '=') {
			default_val = sep+2;
			assign = 1;
		}
		എങ്കിൽ (*(sep + 1) == '+') {
			default_val = sep+2;
			expand_empty = 1;
		}
	}

	p = env_get(src);
	എങ്കിൽ (!p)
		p = get_local_var(src);

	എങ്കിൽ (!p || strlen(p) == 0) {
		p = default_val;
		എങ്കിൽ (assign) {
			അക്ഷരം *var = malloc(strlen(src)+strlen(default_val)+2);
			എങ്കിൽ (var) {
				sprintf(var, "%s=%s", src, default_val);
				set_local_var(var, 0);
			}
			സ്വതന്ത്രം(var);
		}
	} അല്ലെങ്കിൽ എങ്കിൽ (expand_empty) {
		p += strlen(p);
	}

	എങ്കിൽ (sep)
		*sep = ':';

	തിരികെ p;
}

#ifdef __U_BOOT__
സ്ഥിതം അക്ഷരം *get_dollar_var(അക്ഷരം ch)
{
	സ്ഥിതം അക്ഷരം buf[40];

	buf[0] = '\0';
	തിരഞ്ഞെടുപ്പ് (ch) {
		അവസ്ഥ '?':
			sprintf(buf, "%u", (unsigned പൂർണ്ണസംഖ്യ)last_return_code);
			നിർത്തുക;
		സ്ഥിരസ്ഥിതി:
			തിരികെ ശൂന്യ;
	}
	തിരികെ buf;
}
#endif

/* return code: 0 for OK, 1 for syntax error */
സ്ഥിതം പൂർണ്ണസംഖ്യ handle_dollar(o_string *dest, ഘടന p_context *ctx, ഘടന in_str *നിവേശം)
{
#ifndef __U_BOOT__
	പൂർണ്ണസംഖ്യ i, advance=0;
#അല്ലെങ്കിൽ
	പൂർണ്ണസംഖ്യ advance=0;
#endif
#ifndef __U_BOOT__
	അക്ഷരം sep[]=" ";
#endif
	പൂർണ്ണസംഖ്യ ch = നിവേശം->peek(നിവേശം);  /* first character after the $ */
	debug_printf("handle_dollar: ch=%c\n",ch);
	എങ്കിൽ (isalpha(ch)) {
		b_addchr(dest, SPECIAL_VAR_ചിഹ്നം);
		ctx->child->sp++;
		ആകുമ്പോൾ(ch=b_peek(നിവേശം),isalnum(ch) || ch=='_') {
			b_getch(നിവേശം);
			b_addchr(dest,ch);
		}
		b_addchr(dest, SPECIAL_VAR_ചിഹ്നം);
#ifndef __U_BOOT__
	} അല്ലെങ്കിൽ എങ്കിൽ (isdigit(ch)) {
		i = ch-'0';  /* XXX is $0 special? */
		എങ്കിൽ (i<global_argc) {
			parse_string(dest, ctx, global_argv[i]); /* recursion */
		}
		advance = 1;
#endif
	} അല്ലെങ്കിൽ തിരഞ്ഞെടുപ്പ് (ch) {
#ifndef __U_BOOT__
		അവസ്ഥ '$':
			b_adduint(dest,getpid());
			advance = 1;
			നിർത്തുക;
		അവസ്ഥ '!':
			എങ്കിൽ (last_bg_pid > 0) b_adduint(dest, last_bg_pid);
			advance = 1;
			നിർത്തുക;
#endif
		അവസ്ഥ '?':
#ifndef __U_BOOT__
			b_adduint(dest,last_return_code);
#അല്ലെങ്കിൽ
			ctx->child->sp++;
			b_addchr(dest, SPECIAL_VAR_ചിഹ്നം);
			b_addchr(dest, '$');
			b_addchr(dest, '?');
			b_addchr(dest, SPECIAL_VAR_ചിഹ്നം);
#endif
			advance = 1;
			നിർത്തുക;
#ifndef __U_BOOT__
		അവസ്ഥ '#':
			b_adduint(dest,global_argc ? global_argc-1 : 0);
			advance = 1;
			നിർത്തുക;
#endif
		അവസ്ഥ '{':
			b_addchr(dest, SPECIAL_VAR_ചിഹ്നം);
			ctx->child->sp++;
			b_getch(നിവേശം);
			/* XXX maybe someone will try to escape the '}' */
			ആകുമ്പോൾ(ch=b_getch(നിവേശം),ch!=EOF && ch!='}') {
				b_addchr(dest,ch);
			}
			എങ്കിൽ (ch != '}') {
				syntax();
				തിരികെ 1;
			}
			b_addchr(dest, SPECIAL_VAR_ചിഹ്നം);
			നിർത്തുക;
#ifndef __U_BOOT__
		അവസ്ഥ '(':
			b_getch(നിവേശം);
			process_command_subs(dest, ctx, നിവേശം, ')');
			നിർത്തുക;
		അവസ്ഥ '*':
			sep[0]=ifs[0];
			വേണ്ടി (i=1; i<global_argc; i++) {
				parse_string(dest, ctx, global_argv[i]);
				എങ്കിൽ (i+1 < global_argc) parse_string(dest, ctx, sep);
			}
			നിർത്തുക;
		അവസ്ഥ '@':
		അവസ്ഥ '-':
		അവസ്ഥ '_':
			/* still unhandled, but should be eventually */
			error_സന്ദേശം("unhandled syntax: $%c",ch);
			തിരികെ 1;
			നിർത്തുക;
#endif
		സ്ഥിരസ്ഥിതി:
			b_addqchr(dest,'$',dest->quote);
	}
	/* Eat the character if the flag was set.  If the compiler
	 * is smart enough, we could substitute "b_getch(input);"
	 * for all the "advance = 1;" above, and also end up with
	 * a nice size-optimized program.  Hah!  That'll be the day.
	 */
	എങ്കിൽ (advance) b_getch(നിവേശം);
	തിരികെ 0;
}

#ifndef __U_BOOT__
പൂർണ്ണസംഖ്യ parse_string(o_string *dest, ഘടന p_context *ctx, സ്ഥിരം അക്ഷരം *src)
{
	ഘടന in_str foo;
	setup_string_in_str(&foo, src);
	തിരികെ parse_stream(dest, ctx, &foo, '\0');
}
#endif

/* return code is 0 for normal exit, 1 for syntax error */
സ്ഥിതം പൂർണ്ണസംഖ്യ parse_stream(o_string *dest, ഘടന p_context *ctx,
			ഘടന in_str *നിവേശം, പൂർണ്ണസംഖ്യ end_trigger)
{
	unsigned പൂർണ്ണസംഖ്യ ch, m;
#ifndef __U_BOOT__
	പൂർണ്ണസംഖ്യ redir_fd;
	redir_type redir_style;
#endif
	പൂർണ്ണസംഖ്യ next;

	/* Only double-quote state is handled in the state variable dest->quote.
	 * A single-quote triggers a bypass of the main loop until its mate is
	 * found.  When recursing, quote state is passed in via dest->quote. */

	debug_printf("parse_stream, end_trigger=%d\n",end_trigger);
	ആകുമ്പോൾ ((ch=b_getch(നിവേശം))!=EOF) {
		m = map[ch];
#ifdef __U_BOOT__
		എങ്കിൽ (നിവേശം->__promptme == 0) തിരികെ 1;
#endif
		next = (ch == '\n') ? 0 : b_peek(നിവേശം);

		debug_printf("parse_stream: ch=%c (%d) m=%d quote=%d - %c\n",
			ch >= ' ' ? ch : '.', ch, m,
			dest->quote, ctx->അട്ടി == ശൂന്യ ? '*' : '.');

		എങ്കിൽ (m==0 || ((m==1 || m==2) && dest->quote)) {
			b_addqchr(dest, ch, dest->quote);
		} അല്ലെങ്കിൽ {
			എങ്കിൽ (m==2) {  /* unquoted IFS */
				എങ്കിൽ (done_word(dest, ctx)) {
					തിരികെ 1;
				}
				/* If we aren't performing a substitution, treat a newline as a
				 * command separator.  */
				എങ്കിൽ (end_trigger != '\0' && ch=='\n')
					done_pipe(ctx,PIPE_SEQ);
			}
			എങ്കിൽ (ch == end_trigger && !dest->quote && ctx->w==RES_NONE) {
				debug_printf("leaving parse_stream (triggered)\n");
				തിരികെ 0;
			}
#എങ്കിൽ 0
			എങ്കിൽ (ch=='\n') {
				/* Yahoo!  Time to run with it! */
				done_pipe(ctx,PIPE_SEQ);
				run_list(ctx->list_head);
				initialize_context(ctx);
			}
#endif
			എങ്കിൽ (m!=2) തിരഞ്ഞെടുപ്പ് (ch) {
		അവസ്ഥ '#':
			എങ്കിൽ (dest->length == 0 && !dest->quote) {
				ആകുമ്പോൾ(ch=b_peek(നിവേശം),ch!=EOF && ch!='\n') { b_getch(നിവേശം); }
			} അല്ലെങ്കിൽ {
				b_addqchr(dest, ch, dest->quote);
			}
			നിർത്തുക;
		അവസ്ഥ '\\':
			എങ്കിൽ (next == EOF) {
				syntax();
				തിരികെ 1;
			}
			b_addqchr(dest, '\\', dest->quote);
			b_addqchr(dest, b_getch(നിവേശം), dest->quote);
			നിർത്തുക;
		അവസ്ഥ '$':
			എങ്കിൽ (handle_dollar(dest, ctx, നിവേശം)!=0) തിരികെ 1;
			നിർത്തുക;
		അവസ്ഥ '\'':
			dest->nonnull = 1;
			ആകുമ്പോൾ(ch=b_getch(നിവേശം),ch!=EOF && ch!='\'') {
#ifdef __U_BOOT__
				എങ്കിൽ(നിവേശം->__promptme == 0) തിരികെ 1;
#endif
				b_addchr(dest,ch);
			}
			എങ്കിൽ (ch==EOF) {
				syntax();
				തിരികെ 1;
			}
			നിർത്തുക;
		അവസ്ഥ '"':
			dest->nonnull = 1;
			dest->quote = !dest->quote;
			നിർത്തുക;
#ifndef __U_BOOT__
		അവസ്ഥ '`':
			process_command_subs(dest, ctx, നിവേശം, '`');
			നിർത്തുക;
		അവസ്ഥ '>':
			redir_fd = redirect_opt_അക്കം(dest);
			done_word(dest, ctx);
			redir_style=REDIRECT_OVERWRITE;
			എങ്കിൽ (next == '>') {
				redir_style=REDIRECT_APPEND;
				b_getch(നിവേശം);
			} അല്ലെങ്കിൽ എങ്കിൽ (next == '(') {
				syntax();   /* until we support >(list) Process Substitution */
				തിരികെ 1;
			}
			setup_redirect(ctx, redir_fd, redir_style, നിവേശം);
			നിർത്തുക;
		അവസ്ഥ '<':
			redir_fd = redirect_opt_അക്കം(dest);
			done_word(dest, ctx);
			redir_style=REDIRECT_നിവേശം;
			എങ്കിൽ (next == '<') {
				redir_style=REDIRECT_HEREIS;
				b_getch(നിവേശം);
			} അല്ലെങ്കിൽ എങ്കിൽ (next == '>') {
				redir_style=REDIRECT_IO;
				b_getch(നിവേശം);
			} അല്ലെങ്കിൽ എങ്കിൽ (next == '(') {
				syntax();   /* until we support <(list) Process Substitution */
				തിരികെ 1;
			}
			setup_redirect(ctx, redir_fd, redir_style, നിവേശം);
			നിർത്തുക;
#endif
		അവസ്ഥ ';':
			done_word(dest, ctx);
			done_pipe(ctx,PIPE_SEQ);
			നിർത്തുക;
		അവസ്ഥ '&':
			done_word(dest, ctx);
			എങ്കിൽ (next=='&') {
				b_getch(നിവേശം);
				done_pipe(ctx,PIPE_AND);
			} അല്ലെങ്കിൽ {
#ifndef __U_BOOT__
				done_pipe(ctx,PIPE_BG);
#അല്ലെങ്കിൽ
				syntax_err();
				തിരികെ 1;
#endif
			}
			നിർത്തുക;
		അവസ്ഥ '|':
			done_word(dest, ctx);
			എങ്കിൽ (next=='|') {
				b_getch(നിവേശം);
				done_pipe(ctx,PIPE_OR);
			} അല്ലെങ്കിൽ {
				/* we could pick up a file descriptor choice here
				 * with redirect_opt_num(), but bash doesn't do it.
				 * "echo foo 2| cat" yields "foo 2". */
#ifndef __U_BOOT__
				done_command(ctx);
#അല്ലെങ്കിൽ
				syntax_err();
				തിരികെ 1;
#endif
			}
			നിർത്തുക;
#ifndef __U_BOOT__
		അവസ്ഥ '(':
		അവസ്ഥ '{':
			എങ്കിൽ (parse_group(dest, ctx, നിവേശം, ch)!=0) തിരികെ 1;
			നിർത്തുക;
		അവസ്ഥ ')':
		അവസ്ഥ '}':
			syntax();   /* Proper use of this character caught by end_trigger */
			തിരികെ 1;
			നിർത്തുക;
#endif
		അവസ്ഥ SUBSTED_VAR_ചിഹ്നം:
			dest->nonnull = 1;
			ആകുമ്പോൾ (ch = b_getch(നിവേശം), ch != EOF &&
			    ch != SUBSTED_VAR_ചിഹ്നം) {
				debug_printf("subst, pass=%d\n", ch);
				എങ്കിൽ (നിവേശം->__promptme == 0)
					തിരികെ 1;
				b_addchr(dest, ch);
			}
			debug_printf("subst, term=%d\n", ch);
			എങ്കിൽ (ch == EOF) {
				syntax();
				തിരികെ 1;
			}
			നിർത്തുക;
		സ്ഥിരസ്ഥിതി:
			syntax();   /* this is really an internal logic error */
			തിരികെ 1;
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
	എങ്കിൽ (end_trigger != '\0') തിരികെ -1;
	തിരികെ 0;
}

സ്ഥിതം ശൂന്യം mapset(സ്ഥിരം unsigned അക്ഷരം *set, പൂർണ്ണസംഖ്യ code)
{
	സ്ഥിരം unsigned അക്ഷരം *s;
	വേണ്ടി (s=set; *s; s++) map[*s] = code;
}

സ്ഥിതം ശൂന്യം update_ifs_map(ശൂന്യം)
{
	/* char *ifs and char map[256] are both globals. */
	ifs = (uchar *)env_get("IFS");
	എങ്കിൽ (ifs == ശൂന്യ) ifs=(uchar *)" \t\n";
	/* Precompute a list of 'flow through' behavior so it can be treated
	 * quickly up front.  Computation is necessary because of IFS.
	 * Special case handling of IFS == " \t\n" is not implemented.
	 * The map[] array only really needs two bits each, and on most machines
	 * that would be faster because of the reduced L1 cache footprint.
	 */
	memset(map,0,വലുപ്പം(map)); /* most characters flow through always */
#ifndef __U_BOOT__
	mapset((uchar *)"\\$'\"`", 3);      /* never flow through */
	mapset((uchar *)"<>;&|(){}#", 1);   /* flow through if quoted */
#അല്ലെങ്കിൽ
	{
		uchar subst[2] = {SUBSTED_VAR_ചിഹ്നം, 0};
		mapset(subst, 3);       /* never flow through */
	}
	mapset((uchar *)"\\$'\"", 3);       /* never flow through */
	mapset((uchar *)";&|#", 1);         /* flow through if quoted */
#endif
	mapset(ifs, 2);            /* also flow through if quoted */
}

/* most recursion does not come through here, the exeception is
 * from builtin_source() */
സ്ഥിതം പൂർണ്ണസംഖ്യ parse_stream_outer(ഘടന in_str *inp, പൂർണ്ണസംഖ്യ flag)
{

	ഘടന p_context ctx;
	o_string temp=NULL_O_STRING;
	പൂർണ്ണസംഖ്യ rcode;
#ifdef __U_BOOT__
	പൂർണ്ണസംഖ്യ code = 1;
#endif
	ചെയ്യുക {
		ctx.type = flag;
		initialize_context(&ctx);
		update_ifs_map();
		എങ്കിൽ (!(flag & FLAG_PARSE_SEMICOLON) || (flag & FLAG_REPARSING)) mapset((uchar *)";$&|", 0);
		inp->promptmode=1;
		rcode = parse_stream(&temp, &ctx, inp,
				     flag & FLAG_CONT_ON_NEWLINE ? -1 : '\n');
#ifdef __U_BOOT__
		എങ്കിൽ (rcode == 1) flag_repeat = 0;
#endif
		എങ്കിൽ (rcode != 1 && ctx.old_flag != 0) {
			syntax();
#ifdef __U_BOOT__
			flag_repeat = 0;
#endif
		}
		എങ്കിൽ (rcode != 1 && ctx.old_flag == 0) {
			done_word(&temp, &ctx);
			done_pipe(&ctx,PIPE_SEQ);
#ifndef __U_BOOT__
			run_list(ctx.list_head);
#അല്ലെങ്കിൽ
			code = run_list(ctx.list_head);
			എങ്കിൽ (code == -2) {	/* exit */
				b_free(&temp);
				code = 0;
				/* XXX hackish way to not allow exit from main loop */
				എങ്കിൽ (inp->peek == file_peek) {
					printf("exit not allowed from main input shell.\n");
					തുടരുക;
				}
				/*
				 * DANGER
				 * Return code -2 is special in this context,
				 * it indicates exit from inner pipe instead
				 * of return code itself, the return code is
				 * stored in 'last_return_code' variable!
				 * DANGER
				 */
				തിരികെ -2;
			}
			എങ്കിൽ (code == -1)
			    flag_repeat = 0;
#endif
		} അല്ലെങ്കിൽ {
			എങ്കിൽ (ctx.old_flag != 0) {
				സ്വതന്ത്രം(ctx.അട്ടി);
				b_reset(&temp);
			}
#ifdef __U_BOOT__
			എങ്കിൽ (inp->__promptme == 0) printf("<INTERRUPT>\n");
			inp->__promptme = 1;
#endif
			temp.nonnull = 0;
			temp.quote = 0;
			inp->p = ശൂന്യ;
			free_pipe_list(ctx.list_head,0);
		}
		b_free(&temp);
	/* loop on syntax errors, return on EOF */
	} ആകുമ്പോൾ (rcode != -1 && !(flag & FLAG_EXIT_FROM_LOOP) &&
		(inp->peek != static_peek || b_peek(inp)));
#ifndef __U_BOOT__
	തിരികെ 0;
#അല്ലെങ്കിൽ
	തിരികെ (code != 0) ? 1 : 0;
#endif /* __U_BOOT__ */
}

#ifndef __U_BOOT__
സ്ഥിതം പൂർണ്ണസംഖ്യ parse_string_outer(സ്ഥിരം അക്ഷരം *s, പൂർണ്ണസംഖ്യ flag)
#അല്ലെങ്കിൽ
പൂർണ്ണസംഖ്യ parse_string_outer(സ്ഥിരം അക്ഷരം *s, പൂർണ്ണസംഖ്യ flag)
#endif	/* __U_BOOT__ */
{
	ഘടന in_str നിവേശം;
	പൂർണ്ണസംഖ്യ rcode;
#ifdef __U_BOOT__
	അക്ഷരം *p = ശൂന്യ;
	എങ്കിൽ (!s)
		തിരികെ 1;
	എങ്കിൽ (!*s)
		തിരികെ 0;
	എങ്കിൽ (!(p = strchr(s, '\n')) || *++p) {
		p = xmalloc(strlen(s) + 2);
		strcpy(p, s);
		strcat(p, "\n");
		setup_string_in_str(&നിവേശം, p);
		rcode = parse_stream_outer(&നിവേശം, flag);
		സ്വതന്ത്രം(p);
		തിരികെ rcode == -2 ? last_return_code : rcode;
	} അല്ലെങ്കിൽ {
#endif
	setup_string_in_str(&നിവേശം, s);
	rcode = parse_stream_outer(&നിവേശം, flag);
	തിരികെ rcode == -2 ? last_return_code : rcode;
#ifdef __U_BOOT__
	}
#endif
}

#ifndef __U_BOOT__
സ്ഥിതം പൂർണ്ണസംഖ്യ parse_file_outer(FILE *f)
#അല്ലെങ്കിൽ
പൂർണ്ണസംഖ്യ parse_file_outer(ശൂന്യം)
#endif
{
	പൂർണ്ണസംഖ്യ rcode;
	ഘടന in_str നിവേശം;
#ifndef __U_BOOT__
	setup_file_in_str(&നിവേശം, f);
#അല്ലെങ്കിൽ
	setup_file_in_str(&നിവേശം);
#endif
	rcode = parse_stream_outer(&നിവേശം, FLAG_PARSE_SEMICOLON);
	തിരികെ rcode == -2 ? last_return_code : rcode;
}

#ifdef __U_BOOT__
പൂർണ്ണസംഖ്യ u_boot_hush_start(ശൂന്യം)
{
	എങ്കിൽ (മുകൾ_vars == ശൂന്യ) {
		മുകൾ_vars = malloc(വലുപ്പം(ഘടന variables));
		മുകൾ_vars->name = "HUSH_VERSION";
		മുകൾ_vars->മൂല്യം = "0.01";
		മുകൾ_vars->next = ശൂന്യ;
		മുകൾ_vars->flg_export = 0;
		മുകൾ_vars->flg_read_only = 1;
	}
	തിരികെ 0;
}

സ്ഥിതം ശൂന്യം *xmalloc(size_t size)
{
	ശൂന്യം *p = ശൂന്യ;

	എങ്കിൽ (!(p = malloc(size))) {
	    printf("ERROR : xmalloc failed\n");
	    വേണ്ടി(;;);
	}
	തിരികെ p;
}

സ്ഥിതം ശൂന്യം *xrealloc(ശൂന്യം *ptr, size_t size)
{
	ശൂന്യം *p = ശൂന്യ;

	എങ്കിൽ (!(p = realloc(ptr, size))) {
	    printf("ERROR : xrealloc failed\n");
	    വേണ്ടി(;;);
	}
	തിരികെ p;
}
#endif /* __U_BOOT__ */

#ifndef __U_BOOT__
/* Make sure we have a controlling tty.  If we get started under a job
 * aware app (like bash for example), make sure we are now in charge so
 * we don't fight over who gets the foreground */
സ്ഥിതം ശൂന്യം setup_job_control(ശൂന്യം)
{
	സ്ഥിതം pid_t shell_pgrp;
	/* Loop until we are in the foreground.  */
	ആകുമ്പോൾ (tcgetpgrp (shell_terminal) != (shell_pgrp = getpgrp ()))
		kill (- shell_pgrp, SIGTTIN);

	/* Ignore interactive and job-control signals.  */
	സൂചന(SIGINT, SIG_IGN);
	സൂചന(SIGQUIT, SIG_IGN);
	സൂചന(SIGTERM, SIG_IGN);
	സൂചന(SIGTSTP, SIG_IGN);
	സൂചന(SIGTTIN, SIG_IGN);
	സൂചന(SIGTTOU, SIG_IGN);
	സൂചന(SIGCHLD, SIG_IGN);

	/* Put ourselves in our own process group.  */
	setsid();
	shell_pgrp = getpid ();
	setpgid (shell_pgrp, shell_pgrp);

	/* Grab control of the terminal.  */
	tcsetpgrp(shell_terminal, shell_pgrp);
}

പൂർണ്ണസംഖ്യ hush_main(പൂർണ്ണസംഖ്യ argc, അക്ഷരം * സ്ഥിരം *argv)
{
	പൂർണ്ണസംഖ്യ opt;
	FILE *നിവേശം;
	അക്ഷരം **e = environ;

	/* XXX what should these be while sourcing /etc/profile? */
	global_argc = argc;
	global_argv = argv;

	/* (re?) initialize globals.  Sometimes hush_main() ends up calling
	 * hush_main(), therefore we cannot rely on the BSS to zero out this
	 * stuff.  Reset these to 0 every time. */
	ifs = ശൂന്യ;
	/* map[] is taken care of with call to update_ifs_map() */
	fake_mode = 0;
	interactive = 0;
	close_me_head = ശൂന്യ;
	last_bg_pid = 0;
	job_list = ശൂന്യ;
	last_jobid = 0;

	/* Initialize some more globals to non-zero values */
	set_cwd();
#ifdef CONFIG_FEATURE_COMMAND_EDITING
	cmdedit_set_initial_prompt();
#അല്ലെങ്കിൽ
	PS1 = ശൂന്യ;
#endif
	PS2 = "> ";

	/* initialize our shell local variables with the values
	 * currently living in the environment */
	എങ്കിൽ (e) {
		വേണ്ടി (; *e; e++)
			set_local_var(*e, 2);   /* without call putenv() */
	}

	last_return_code=EXIT_SUCCESS;

	എങ്കിൽ (argv[0] && argv[0][0] == '-') {
		debug_printf("\nsourcing /etc/profile\n");
		എങ്കിൽ ((നിവേശം = fopen("/etc/profile", "r")) != ശൂന്യ) {
			mark_open(fileno(നിവേശം));
			parse_file_outer(നിവേശം);
			mark_closed(fileno(നിവേശം));
			fclose(നിവേശം);
		}
	}
	നിവേശം=stdin;

	ആകുമ്പോൾ ((opt = getopt(argc, argv, "c:xif")) > 0) {
		തിരഞ്ഞെടുപ്പ് (opt) {
			അവസ്ഥ 'c':
				{
					global_argv = argv+optind;
					global_argc = argc-optind;
					opt = parse_string_outer(optarg, FLAG_PARSE_SEMICOLON);
					പോകുക final_return;
				}
				നിർത്തുക;
			അവസ്ഥ 'i':
				interactive++;
				നിർത്തുക;
			അവസ്ഥ 'f':
				fake_mode++;
				നിർത്തുക;
			സ്ഥിരസ്ഥിതി:
#ifndef BB_VER
				fprintf(stderr, "Usage: sh [FILE]...\n"
						"   or: sh -c command [args]...\n\n");
				exit(EXIT_FAILURE);
#അല്ലെങ്കിൽ
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
	എങ്കിൽ (argv[optind]==ശൂന്യ && നിവേശം==stdin &&
			isatty(fileno(stdin)) && isatty(fileno(stdout))) {
		interactive++;
	}

	debug_printf("\ninteractive=%d\n", interactive);
	എങ്കിൽ (interactive) {
		/* Looks like they want an interactive shell */
#ifndef CONFIG_FEATURE_SH_EXTRA_QUIET
		printf( "\n\n" BB_BANNER " hush - the humble shell v0.01 (testing)\n");
		printf( "Enter 'help' for a list of built-in commands.\n\n");
#endif
		setup_job_control();
	}

	എങ്കിൽ (argv[optind]==ശൂന്യ) {
		opt=parse_file_outer(stdin);
		പോകുക final_return;
	}

	debug_printf("\nrunning script '%s'\n", argv[optind]);
	global_argv = argv+optind;
	global_argc = argc-optind;
	നിവേശം = xfopen(argv[optind], "r");
	opt = parse_file_outer(നിവേശം);

#ifdef CONFIG_FEATURE_CLEAN_UP
	fclose(നിവേശം);
	എങ്കിൽ (cwd && cwd != unknown)
		സ്വതന്ത്രം((അക്ഷരം*)cwd);
	{
		ഘടന variables *cur, *tmp;
		വേണ്ടി(cur = മുകൾ_vars; cur; cur = tmp) {
			tmp = cur->next;
			എങ്കിൽ (!cur->flg_read_only) {
				സ്വതന്ത്രം(cur->name);
				സ്വതന്ത്രം(cur->മൂല്യം);
				സ്വതന്ത്രം(cur);
			}
		}
	}
#endif

final_return:
	തിരികെ(opt?opt:last_return_code);
}
#endif

സ്ഥിതം അക്ഷരം *തിരുകുക_var_മൂല്യം(അക്ഷരം *inp)
{
	തിരികെ തിരുകുക_var_മൂല്യം_sub(inp, 0);
}

സ്ഥിതം അക്ഷരം *തിരുകുക_var_മൂല്യം_sub(അക്ഷരം *inp, പൂർണ്ണസംഖ്യ tag_subst)
{
	പൂർണ്ണസംഖ്യ res_str_len = 0;
	പൂർണ്ണസംഖ്യ len;
	പൂർണ്ണസംഖ്യ done = 0;
	അക്ഷരം *p, *p1, *res_str = ശൂന്യ;

	ആകുമ്പോൾ ((p = strchr(inp, SPECIAL_VAR_ചിഹ്നം))) {
		/* check the beginning of the string for normal characters */
		എങ്കിൽ (p != inp) {
			/* copy any characters to the result string */
			len = p - inp;
			res_str = xrealloc(res_str, (res_str_len + len));
			strncpy((res_str + res_str_len), inp, len);
			res_str_len += len;
		}
		inp = ++p;
		/* find the ending marker */
		p = strchr(inp, SPECIAL_VAR_ചിഹ്നം);
		*p = '\0';
		/* look up the value to substitute */
		എങ്കിൽ ((p1 = lookup_param(inp))) {
			എങ്കിൽ (tag_subst)
				len = res_str_len + strlen(p1) + 2;
			അല്ലെങ്കിൽ
				len = res_str_len + strlen(p1);
			res_str = xrealloc(res_str, (1 + len));
			എങ്കിൽ (tag_subst) {
				/*
				 * copy the variable value to the result
				 * string
				 */
				strcpy((res_str + res_str_len + 1), p1);

				/*
				 * mark the replaced text to be accepted as
				 * is
				 */
				res_str[res_str_len] = SUBSTED_VAR_ചിഹ്നം;
				res_str[res_str_len + 1 + strlen(p1)] =
					SUBSTED_VAR_ചിഹ്നം;
			} അല്ലെങ്കിൽ
				/*
				 * copy the variable value to the result
				 * string
				 */
				strcpy((res_str + res_str_len), p1);

			res_str_len = len;
		}
		*p = SPECIAL_VAR_ചിഹ്നം;
		inp = ++p;
		done = 1;
	}
	എങ്കിൽ (done) {
		res_str = xrealloc(res_str, (1 + res_str_len + strlen(inp)));
		strcpy((res_str + res_str_len), inp);
		ആകുമ്പോൾ ((p = strchr(res_str, '\n'))) {
			*p = ' ';
		}
	}
	തിരികെ (res_str == ശൂന്യ) ? inp : res_str;
}

സ്ഥിതം അക്ഷരം **make_list_in(അക്ഷരം **inp, അക്ഷരം *name)
{
	പൂർണ്ണസംഖ്യ len, i;
	പൂർണ്ണസംഖ്യ name_len = strlen(name);
	പൂർണ്ണസംഖ്യ n = 0;
	അക്ഷരം **list;
	അക്ഷരം *p1, *p2, *p3;

	/* create list of variable values */
	list = xmalloc(വലുപ്പം(*list));
	വേണ്ടി (i = 0; inp[i]; i++) {
		p3 = തിരുകുക_var_മൂല്യം(inp[i]);
		p1 = p3;
		ആകുമ്പോൾ (*p1) {
			എങ്കിൽ (*p1 == ' ') {
				p1++;
				തുടരുക;
			}
			എങ്കിൽ ((p2 = strchr(p1, ' '))) {
				len = p2 - p1;
			} അല്ലെങ്കിൽ {
				len = strlen(p1);
				p2 = p1 + len;
			}
			/* we use n + 2 in realloc for list,because we add
			 * new element and then we will add NULL element */
			list = xrealloc(list, വലുപ്പം(*list) * (n + 2));
			list[n] = xmalloc(2 + name_len + len);
			strcpy(list[n], name);
			strcat(list[n], "=");
			strncat(list[n], p1, len);
			list[n++][name_len + len + 1] = '\0';
			p1 = p2;
		}
		എങ്കിൽ (p3 != inp[i]) സ്വതന്ത്രം(p3);
	}
	list[n] = ശൂന്യ;
	തിരികെ list;
}

/*
 * Make new string for parser
 * inp     - array of argument strings to flatten
 * nonnull - indicates argument was quoted when originally parsed
 */
സ്ഥിതം അക്ഷരം *make_string(അക്ഷരം **inp, പൂർണ്ണസംഖ്യ *nonnull)
{
	അക്ഷരം *p;
	അക്ഷരം *str = ശൂന്യ;
	പൂർണ്ണസംഖ്യ n;
	പൂർണ്ണസംഖ്യ len = 2;
	അക്ഷരം *noeval_str;
	പൂർണ്ണസംഖ്യ noeval = 0;

	noeval_str = get_local_var("HUSH_NO_EVAL");
	എങ്കിൽ (noeval_str != ശൂന്യ && *noeval_str != '0' && *noeval_str != '\0')
		noeval = 1;
	വേണ്ടി (n = 0; inp[n]; n++) {
		p = തിരുകുക_var_മൂല്യം_sub(inp[n], noeval);
		അക്ഷരം *new_str = xrealloc(str, (len + strlen(p) + (2 * nonnull[n])));
		എങ്കിൽ (!new_str) {
			സ്വതന്ത്രം(str);
			എങ്കിൽ (p != inp[n]) സ്വതന്ത്രം(p);
			തിരികെ ശൂന്യ;
		}
		str = new_str;
		എങ്കിൽ (n) {
			strcat(str, " ");
		} അല്ലെങ്കിൽ {
			*str = '\0';
		}
		എങ്കിൽ (nonnull[n])
			strcat(str, "'");
		strcat(str, p);
		എങ്കിൽ (nonnull[n])
			strcat(str, "'");
		len = strlen(str) + 3;
		എങ്കിൽ (p != inp[n]) സ്വതന്ത്രം(p);
	}
	len = strlen(str);
	*(str + len) = '\n';
	*(str + len + 1) = '\0';
	തിരികെ str;
}

#ifdef __U_BOOT__
സ്ഥിതം പൂർണ്ണസംഖ്യ do_showvar(ഘടന cmd_tbl *cmdtp, പൂർണ്ണസംഖ്യ flag, പൂർണ്ണസംഖ്യ argc,
		      അക്ഷരം *സ്ഥിരം argv[])
{
	പൂർണ്ണസംഖ്യ i, k;
	പൂർണ്ണസംഖ്യ rcode = 0;
	ഘടന variables *cur;

	എങ്കിൽ (argc == 1) {		/* Print all env variables	*/
		വേണ്ടി (cur = മുകൾ_vars; cur; cur = cur->next) {
			printf ("%s=%s\n", cur->name, cur->മൂല്യം);
			എങ്കിൽ (ctrlc ()) {
				puts ("\n ** Abort\n");
				തിരികെ 1;
			}
		}
		തിരികെ 0;
	}
	വേണ്ടി (i = 1; i < argc; ++i) {	/* print single env variables	*/
		അക്ഷരം *name = argv[i];

		k = -1;
		വേണ്ടി (cur = മുകൾ_vars; cur; cur = cur->next) {
			എങ്കിൽ(strcmp (cur->name, name) == 0) {
				k = 0;
				printf ("%s=%s\n", cur->name, cur->മൂല്യം);
			}
			എങ്കിൽ (ctrlc ()) {
				puts ("\n ** Abort\n");
				തിരികെ 1;
			}
		}
		എങ്കിൽ (k < 0) {
			printf ("## Error: \"%s\" not defined\n", name);
			rcode ++;
		}
	}
	തിരികെ rcode;
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
