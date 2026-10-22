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
#અન્તર્ભાવ <env.h>
#અન્તર્ભાવ <malloc.h>         /* malloc, free, realloc*/
#અન્તર્ભાવ <linux/ctype.h>    /* isalpha, isdigit */
#અન્તર્ભાવ <console.h>
#અન્તર્ભાવ <bootretry.h>
#અન્તર્ભાવ <cli.h>
#અન્તર્ભાવ <cli_hush.h>
#અન્તર્ભાવ <command.h>        /* find_cmd */
#અન્તર્ભાવ <vsprintf.h>
#endif
#ifndef __U_BOOT__
#અન્તર્ભાવ <ctype.h>     /* isalpha, isdigit */
#અન્તર્ભાવ <unistd.h>    /* getpid */
#અન્તર્ભાવ <stdlib.h>    /* getenv, atoi */
#અન્તર્ભાવ <string.h>    /* strchr */
#અન્તર્ભાવ <stdio.h>     /* popen etc. */
#અન્તર્ભાવ <glob.h>      /* glob, of course */
#અન્તર્ભાવ <stdarg.h>    /* va_list */
#અન્તર્ભાવ <errno.h>
#અન્તર્ભાવ <fcntl.h>
#અન્તર્ભાવ <getopt.h>    /* should be pretty obvious */

#અન્તર્ભાવ <sys/stat.h>  /* ulimit */
#અન્તર્ભાવ <sys/types.h>
#અન્તર્ભાવ <sys/wait.h>
#અન્તર્ભાવ <signal.h>

/* #include <dmalloc.h> */

#જો 1
#અન્તર્ભાવ "busybox.h"
#અન્તર્ભાવ "cmdedit.h"
#નહીંતર
#define applet_name "hush"
#અન્તર્ભાવ "standalone.h"
#define hush_main મુખ્ય
#define BB_BANNER
#endif
#endif
#define SPECIAL_VAR_પ્રતીક 03
#define SUBSTED_VAR_પ્રતીક 04
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
#define error_સંદેશ printf
#નહીંતર
પ્રકારવ્યાખ્યા ગણના {
	REDIRECT_આગત     = 1,
	REDIRECT_OVERWRITE = 2,
	REDIRECT_APPEND    = 3,
	REDIRECT_HEREIS    = 4,
	REDIRECT_IO        = 5
} redir_type;

/* The descrip member of this structure is only used to make debugging
 * output pretty */
સંરચના {પૂર્ણાંક mode; પૂર્ણાંક default_fd; અક્ષર *descrip;} redir_table[] = {
	{ 0,                         0, "()" },
	{ O_RDONLY,                  0, "<"  },
	{ O_CREAT|O_TRUNC|O_WRONLY,  1, ">"  },
	{ O_CREAT|O_APPEND|O_WRONLY, 1, ">>" },
	{ O_RDONLY,                 -1, "<<" },
	{ O_RDWR,                    1, "<>" }
};
#endif

પ્રકારવ્યાખ્યા ગણના {
	PIPE_SEQ = 1,
	PIPE_AND = 2,
	PIPE_OR  = 3,
	PIPE_BG  = 4,
} pipe_style;

/* might eventually control execution */
પ્રકારવ્યાખ્યા ગણના {
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
સંરચના p_context {
	સંરચના child_prog *child;
	સંરચના pipe *list_head;
	સંરચના pipe *pipe;
#ifndef __U_BOOT__
	સંરચના redir_struct *pending_redirect;
#endif
	reserved_style w;
	પૂર્ણાંક old_flag;				/* for figuring out valid reserved words */
	સંરચના p_context *ઢગલો;
	પૂર્ણાંક type;			/* define type of parser : ";$" common or special symbol */
	/* How about quoting status? */
};

#ifndef __U_BOOT__
સંરચના redir_struct {
	redir_type type;			/* type of redirection */
	પૂર્ણાંક fd;						/* file descriptor being redirected */
	પૂર્ણાંક dup;					/* -1, or file descriptor being duplicated */
	સંરચના redir_struct *next;	/* pointer to the next redirect in the list */
	glob_t word;				/* *word.gl_pathv is the filename */
};
#endif

સંરચના child_prog {
#ifndef __U_BOOT__
	pid_t pid;					/* 0 if exited */
#endif
	અક્ષર **argv;				/* program name and arguments */
	/* was quoted when parsed; copy of struct o_string.nonnull field */
	પૂર્ણાંક *argv_nonnull;
#ifdef __U_BOOT__
	પૂર્ણાંક    argc;                            /* number of program arguments */
#endif
	સંરચના pipe *group;			/* if non-NULL, first in group or subshell */
#ifndef __U_BOOT__
	પૂર્ણાંક subshell;				/* flag, non-zero if group must be forked */
	સંરચના redir_struct *redirects;	/* I/O redirections */
	glob_t glob_result;			/* result of parameter globbing */
	પૂર્ણાંક is_stopped;				/* is the program currently running? */
	સંરચના pipe *family;		/* pointer back to the child's parent pipe */
#endif
	પૂર્ણાંક sp;				/* number of SPECIAL_VAR_SYMBOL */
	પૂર્ણાંક type;
};

સંરચના pipe {
#ifndef __U_BOOT__
	પૂર્ણાંક jobid;					/* job number */
#endif
	પૂર્ણાંક અંક_progs;				/* total number of programs in job */
#ifndef __U_BOOT__
	પૂર્ણાંક running_progs;			/* number of programs running */
	અક્ષર *text;					/* name of job */
	અક્ષર *cmdbuf;				/* buffer various argv's point into */
	pid_t pgrp;					/* process group ID for the job */
#endif
	સંરચના child_prog *progs;	/* array of commands in pipe */
	સંરચના pipe *next;			/* to track background commands */
#ifndef __U_BOOT__
	પૂર્ણાંક stopped_progs;			/* number of programs alive, but stopped */
	પૂર્ણાંક job_context;			/* bitmask defining current context */
#endif
	pipe_style followup;		/* PIPE_BG, PIPE_SEQ, PIPE_OR, PIPE_AND */
	reserved_style r_mode;		/* supports if, for, while, until */
};

#ifndef __U_BOOT__
સંરચના close_me {
	પૂર્ણાંક fd;
	સંરચના close_me *next;
};
#endif

સંરચના variables {
	અક્ષર *name;
	અક્ષર *કિંમત;
	પૂર્ણાંક flg_export;
	પૂર્ણાંક flg_read_only;
	સંરચના variables *next;
};

/* globals, connect us to the outside world
 * the first three support $?, $#, and $1 */
#ifndef __U_BOOT__
અક્ષર **global_argv;
unsigned પૂર્ણાંક global_argc;
#endif
સ્થિર unsigned પૂર્ણાંક last_return_code;
#ifndef __U_BOOT__
બાહ્ય અક્ષર **environ; /* This is in <unistd.h>, but protected with __USE_GNU */
#endif

/* "globals" within this file */
સ્થિર uchar *ifs;
સ્થિર અક્ષર map[256];
#ifndef __U_BOOT__
સ્થિર પૂર્ણાંક fake_mode;
સ્થિર પૂર્ણાંક interactive;
સ્થિર સંરચના close_me *close_me_head;
સ્થિર અચલ અક્ષર *cwd;
સ્થિર સંરચના pipe *job_list;
સ્થિર unsigned પૂર્ણાંક last_bg_pid;
સ્થિર unsigned પૂર્ણાંક last_jobid;
સ્થિર unsigned પૂર્ણાંક shell_terminal;
સ્થિર અક્ષર *PS1;
સ્થિર અક્ષર *PS2;
સંરચના variables shell_ver = { "HUSH_VERSION", "0.01", 1, 1, 0 };
સંરચના variables *ટોચ_vars = &shell_ver;
#નહીંતર
સ્થિર પૂર્ણાંક flag_repeat = 0;
સ્થિર પૂર્ણાંક do_repeat = 0;
સ્થિર સંરચના variables *ટોચ_vars = ખાલી ;
#endif /*__U_BOOT__ */

#define B_CHUNK (100)
#define B_NOSPAC 1

પ્રકારવ્યાખ્યા સંરચના {
	અક્ષર *data;
	પૂર્ણાંક length;
	પૂર્ણાંક maxlen;
	પૂર્ણાંક quote;
	પૂર્ણાંક nonnull;
} o_string;
#define NULL_O_STRING {ખાલી,0,0,0,0}
/* used for initialization:
	o_string foo = NULL_O_STRING; */

/* I can almost use ordinary FILE *.  Is open_memstream() universally
 * available?  Where is it documented? */
સંરચના in_str {
	અચલ unsigned અક્ષર *p;
#ifndef __U_BOOT__
	અક્ષર peek_buf[2];
#endif
	પૂર્ણાંક __promptme;
	પૂર્ણાંક promptmode;
#ifndef __U_BOOT__
	FILE *file;
#endif
	પૂર્ણાંક (*get) (સંરચના in_str *);
	પૂર્ણાંક (*peek) (સંરચના in_str *);
};
#define b_getch(આગત) ((આગત)->get(આગત))
#define b_peek(આગત) ((આગત)->peek(આગત))

#ifndef __U_BOOT__
#define JOB_STATUS_FORMAT "[%d] %-22s %.40s\n"

સંરચના built_in_command {
	અક્ષર *cmd;					/* name */
	અક્ષર *descr;				/* description */
	પૂર્ણાંક (*function) (સંરચના child_prog *);	/* function ptr */
};
#endif

/* define DEBUG_SHELL for debugging output (obviously ;-)) */
#જો 0
#define DEBUG_SHELL
#endif

/* This should be in utility.c */
#ifdef DEBUG_SHELL
#ifndef __U_BOOT__
સ્થિર શૂન્ય debug_printf(અચલ અક્ષર *format, ...)
{
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
}
#નહીંતર
#define debug_printf(fmt,args...)	printf (fmt ,##args)
#endif
#નહીંતર
સ્થિર અંતઃરેખ શૂન્ય debug_printf(અચલ અક્ષર *format, ...) { }
#endif
#define final_printf debug_printf

#ifdef __U_BOOT__
સ્થિર શૂન્ય syntax_err(શૂન્ય) {
	 printf("syntax error\n");
}
#નહીંતર
સ્થિર શૂન્ય __syntax(અક્ષર *file, પૂર્ણાંક લીટી) {
	error_સંદેશ("syntax error %s:%d", file, લીટી);
}
#define syntax() __syntax(__FILE__, __LINE__)
#endif

#ifdef __U_BOOT__
સ્થિર શૂન્ય *xmalloc(size_t size);
સ્થિર શૂન્ય *xrealloc(શૂન્ય *ptr, size_t size);
#નહીંતર
/* Index of subroutines: */
/*   function prototypes for builtins */
સ્થિર પૂર્ણાંક builtin_cd(સંરચના child_prog *child);
સ્થિર પૂર્ણાંક builtin_env(સંરચના child_prog *child);
સ્થિર પૂર્ણાંક builtin_eval(સંરચના child_prog *child);
સ્થિર પૂર્ણાંક builtin_exec(સંરચના child_prog *child);
સ્થિર પૂર્ણાંક builtin_exit(સંરચના child_prog *child);
સ્થિર પૂર્ણાંક builtin_export(સંરચના child_prog *child);
સ્થિર પૂર્ણાંક builtin_fg_bg(સંરચના child_prog *child);
સ્થિર પૂર્ણાંક builtin_help(સંરચના child_prog *child);
સ્થિર પૂર્ણાંક builtin_jobs(સંરચના child_prog *child);
સ્થિર પૂર્ણાંક builtin_pwd(સંરચના child_prog *child);
સ્થિર પૂર્ણાંક builtin_read(સંરચના child_prog *child);
સ્થિર પૂર્ણાંક builtin_set(સંરચના child_prog *child);
સ્થિર પૂર્ણાંક builtin_shift(સંરચના child_prog *child);
સ્થિર પૂર્ણાંક builtin_source(સંરચના child_prog *child);
સ્થિર પૂર્ણાંક builtin_umask(સંરચના child_prog *child);
સ્થિર પૂર્ણાંક builtin_unset(સંરચના child_prog *child);
સ્થિર પૂર્ણાંક builtin_not_written(સંરચના child_prog *child);
#endif
/*   o_string manipulation: */
સ્થિર પૂર્ણાંક b_check_space(o_string *o, પૂર્ણાંક len);
સ્થિર પૂર્ણાંક b_addchr(o_string *o, પૂર્ણાંક ch);
સ્થિર શૂન્ય b_reset(o_string *o);
સ્થિર પૂર્ણાંક b_addqchr(o_string *o, પૂર્ણાંક ch, પૂર્ણાંક quote);
#ifndef __U_BOOT__
સ્થિર પૂર્ણાંક b_adduint(o_string *o, unsigned પૂર્ણાંક i);
#endif
/*  in_str manipulations: */
સ્થિર પૂર્ણાંક static_get(સંરચના in_str *i);
સ્થિર પૂર્ણાંક static_peek(સંરચના in_str *i);
સ્થિર પૂર્ણાંક file_get(સંરચના in_str *i);
સ્થિર પૂર્ણાંક file_peek(સંરચના in_str *i);
#ifndef __U_BOOT__
સ્થિર શૂન્ય setup_file_in_str(સંરચના in_str *i, FILE *f);
#નહીંતર
સ્થિર શૂન્ય setup_file_in_str(સંરચના in_str *i);
#endif
સ્થિર શૂન્ય setup_string_in_str(સંરચના in_str *i, અચલ અક્ષર *s);
#ifndef __U_BOOT__
/*  close_me manipulations: */
સ્થિર શૂન્ય mark_open(પૂર્ણાંક fd);
સ્થિર શૂન્ય mark_closed(પૂર્ણાંક fd);
સ્થિર શૂન્ય close_all(શૂન્ય);
#endif
/*  "run" the final data structures: */
સ્થિર અક્ષર *indenter(પૂર્ણાંક i);
સ્થિર પૂર્ણાંક free_pipe_list(સંરચના pipe *head, પૂર્ણાંક indent);
સ્થિર પૂર્ણાંક free_pipe(સંરચના pipe *pi, પૂર્ણાંક indent);
/*  really run the final data structures: */
#ifndef __U_BOOT__
સ્થિર પૂર્ણાંક setup_redirects(સંરચના child_prog *prog, પૂર્ણાંક squirrel[]);
#endif
સ્થિર પૂર્ણાંક run_list_real(સંરચના pipe *pi);
#ifndef __U_BOOT__
સ્થિર શૂન્ય pseudo_exec(સંરચના child_prog *child) __attribute__ ((noreturn));
#endif
સ્થિર પૂર્ણાંક run_pipe_real(સંરચના pipe *pi);
/*   extended glob support: */
#ifndef __U_BOOT__
સ્થિર પૂર્ણાંક globhack(અચલ અક્ષર *src, પૂર્ણાંક flags, glob_t *pglob);
સ્થિર પૂર્ણાંક glob_needed(અચલ અક્ષર *s);
સ્થિર પૂર્ણાંક xglob(o_string *dest, પૂર્ણાંક flags, glob_t *pglob);
#endif
/*   variable assignment: */
સ્થિર પૂર્ણાંક is_assignment(અચલ અક્ષર *s);
/*   data structure manipulation: */
#ifndef __U_BOOT__
સ્થિર પૂર્ણાંક setup_redirect(સંરચના p_context *ctx, પૂર્ણાંક fd, redir_type style, સંરચના in_str *આગત);
#endif
સ્થિર શૂન્ય initialize_context(સંરચના p_context *ctx);
સ્થિર પૂર્ણાંક done_word(o_string *dest, સંરચના p_context *ctx);
સ્થિર પૂર્ણાંક done_command(સંરચના p_context *ctx);
સ્થિર પૂર્ણાંક done_pipe(સંરચના p_context *ctx, pipe_style type);
/*   primary string parsing: */
#ifndef __U_BOOT__
સ્થિર પૂર્ણાંક redirect_dup_અંક(સંરચના in_str *આગત);
સ્થિર પૂર્ણાંક redirect_opt_અંક(o_string *o);
સ્થિર પૂર્ણાંક process_command_subs(o_string *dest, સંરચના p_context *ctx, સંરચના in_str *આગત, પૂર્ણાંક subst_end);
સ્થિર પૂર્ણાંક parse_group(o_string *dest, સંરચના p_context *ctx, સંરચના in_str *આગત, પૂર્ણાંક ch);
#endif
સ્થિર અક્ષર *lookup_param(અક્ષર *src);
સ્થિર અક્ષર *make_string(અક્ષર **inp, પૂર્ણાંક *nonnull);
સ્થિર પૂર્ણાંક handle_dollar(o_string *dest, સંરચના p_context *ctx, સંરચના in_str *આગત);
#ifndef __U_BOOT__
સ્થિર પૂર્ણાંક parse_string(o_string *dest, સંરચના p_context *ctx, અચલ અક્ષર *src);
#endif
સ્થિર પૂર્ણાંક parse_stream(o_string *dest, સંરચના p_context *ctx, સંરચના in_str *input0, પૂર્ણાંક end_trigger);
/*   setup: */
સ્થિર પૂર્ણાંક parse_stream_outer(સંરચના in_str *inp, પૂર્ણાંક flag);
#ifndef __U_BOOT__
સ્થિર પૂર્ણાંક parse_string_outer(અચલ અક્ષર *s, પૂર્ણાંક flag);
સ્થિર પૂર્ણાંક parse_file_outer(FILE *f);
#endif
#ifndef __U_BOOT__
/*   job management: */
સ્થિર પૂર્ણાંક checkjobs(સંરચના pipe* fg_pipe);
સ્થિર શૂન્ય ઉમેરો_bg_job(સંરચના pipe *pi);
સ્થિર શૂન્ય remove_bg_job(સંરચના pipe *pi);
#endif
/*     local variable support */
સ્થિર અક્ષર **make_list_in(અક્ષર **inp, અક્ષર *name);
સ્થિર અક્ષર *ઉમેરો_var_કિંમત(અક્ષર *inp);
સ્થિર અક્ષર *ઉમેરો_var_કિંમત_sub(અક્ષર *inp, પૂર્ણાંક tag_subst);

#ifndef __U_BOOT__
/* Table of built-in functions.  They can be forked or not, depending on
 * context: within pipes, they fork.  As simple commands, they do not.
 * When used in non-forking context, they can change global variables
 * in the parent shell process.  If forked, of course they can not.
 * For example, 'unset foo | whatever' will parse and run, but foo will
 * still be set at the end. */
સ્થિર સંરચના built_in_command bltins[] = {
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
	{ખાલી, ખાલી, ખાલી}
};

સ્થિર અચલ અક્ષર *set_cwd(શૂન્ય)
{
	જો(cwd==unknown)
		cwd = ખાલી;     /* xgetcwd(arg) called free(arg) */
	cwd = xgetcwd((અક્ષર *)cwd);
	જો (!cwd)
		cwd = unknown;
	પાછા cwd;
}

/* built-in 'eval' handler */
સ્થિર પૂર્ણાંક builtin_eval(સંરચના child_prog *child)
{
	અક્ષર *str = ખાલી;
	પૂર્ણાંક rcode = EXIT_SUCCESS;

	જો (child->argv[1]) {
		str = make_string(child->argv + 1);
		parse_string_outer(str, FLAG_EXIT_FROM_LOOP |
					FLAG_PARSE_SEMICOLON);
		મુક્ત(str);
		rcode = last_return_code;
	}
	પાછા rcode;
}

/* built-in 'cd <path>' handler */
સ્થિર પૂર્ણાંક builtin_cd(સંરચના child_prog *child)
{
	અક્ષર *newdir;
	જો (child->argv[1] == ખાલી)
		newdir = env_get("HOME");
	નહીંતર
		newdir = child->argv[1];
	જો (chdir(newdir)) {
		printf("cd: %s: %s\n", newdir, strerror(errno));
		પાછા EXIT_FAILURE;
	}
	set_cwd();
	પાછા EXIT_SUCCESS;
}

/* built-in 'env' handler */
સ્થિર પૂર્ણાંક builtin_env(સંરચના child_prog *dummy)
{
	અક્ષર **e = environ;
	જો (e == ખાલી) પાછા EXIT_FAILURE;
	માટે (; *e; e++) {
		puts(*e);
	}
	પાછા EXIT_SUCCESS;
}

/* built-in 'exec' handler */
સ્થિર પૂર્ણાંક builtin_exec(સંરચના child_prog *child)
{
	જો (child->argv[1] == ખાલી)
		પાછા EXIT_SUCCESS;   /* Really? */
	child->argv++;
	pseudo_exec(child);
	/* never returns */
}

/* built-in 'exit' handler */
સ્થિર પૂર્ણાંક builtin_exit(સંરચના child_prog *child)
{
	જો (child->argv[1] == ખાલી)
		exit(last_return_code);
	exit (atoi(child->argv[1]));
}

/* built-in 'export VAR=value' handler */
સ્થિર પૂર્ણાંક builtin_export(સંરચના child_prog *child)
{
	પૂર્ણાંક res = 0;
	અક્ષર *name = child->argv[1];

	જો (name == ખાલી) {
		પાછા (builtin_env(child));
	}

	name = strdup(name);

	જો(name) {
		અક્ષર *કિંમત = strchr(name, '=');

		જો (!કિંમત) {
			અક્ષર *tmp;
			/* They are exporting something without an =VALUE */

			કિંમત = get_local_var(name);
			જો (કિંમત) {
				size_t ln = strlen(name);

				tmp = realloc(name, ln+strlen(કિંમત)+2);
				જો(tmp==ખાલી)
					res = -1;
				નહીંતર {
					sprintf(tmp+ln, "=%s", કિંમત);
					name = tmp;
				}
			} નહીંતર {
				/* bash does not return an error when trying to export
				 * an undefined variable.  Do likewise. */
				res = 1;
			}
		}
	}
	જો (res<0)
		perror_સંદેશ("export");
	નહીંતર જો(res==0)
		res = set_local_var(name, 1);
	નહીંતર
		res = 0;
	મુક્ત(name);
	પાછા res;
}

/* built-in 'fg' and 'bg' handler */
સ્થિર પૂર્ણાંક builtin_fg_bg(સંરચના child_prog *child)
{
	પૂર્ણાંક i, jobnum;
	સંરચના pipe *pi=ખાલી;

	જો (!interactive)
		પાછા EXIT_FAILURE;
	/* If they gave us no args, assume they want the last backgrounded task */
	જો (!child->argv[1]) {
		માટે (pi = job_list; pi; pi = pi->next) {
			જો (pi->jobid == last_jobid) {
				તોડો;
			}
		}
		જો (!pi) {
			error_સંદેશ("%s: no current job", child->argv[0]);
			પાછા EXIT_FAILURE;
		}
	} નહીંતર {
		જો (sscanf(child->argv[1], "%%%d", &jobnum) != 1) {
			error_સંદેશ("%s: bad argument '%s'", child->argv[0], child->argv[1]);
			પાછા EXIT_FAILURE;
		}
		માટે (pi = job_list; pi; pi = pi->next) {
			જો (pi->jobid == jobnum) {
				તોડો;
			}
		}
		જો (!pi) {
			error_સંદેશ("%s: %d: no such job", child->argv[0], jobnum);
			પાછા EXIT_FAILURE;
		}
	}

	જો (*child->argv[0] == 'f') {
		/* Put the job into the foreground.  */
		tcsetpgrp(shell_terminal, pi->pgrp);
	}

	/* Restart the processes in the job */
	માટે (i = 0; i < pi->અંક_progs; i++)
		pi->progs[i].is_stopped = 0;

	જો ( (i=kill(- pi->pgrp, SIGCONT)) < 0) {
		જો (i == ESRCH) {
			remove_bg_job(pi);
		} નહીંતર {
			perror_સંદેશ("kill (SIGCONT)");
		}
	}

	pi->stopped_progs = 0;
	પાછા EXIT_SUCCESS;
}

/* built-in 'help' handler */
સ્થિર પૂર્ણાંક builtin_help(સંરચના child_prog *dummy)
{
	સંરચના built_in_command *x;

	printf("\nBuilt-in commands:\n");
	printf("-------------------\n");
	માટે (x = bltins; x->cmd; x++) {
		જો (x->descr==ખાલી)
			ચાલુ;
		printf("%s\t%s\n", x->cmd, x->descr);
	}
	printf("\n\n");
	પાછા EXIT_SUCCESS;
}

/* built-in 'jobs' handler */
સ્થિર પૂર્ણાંક builtin_jobs(સંરચના child_prog *child)
{
	સંરચના pipe *job;
	અક્ષર *status_string;

	માટે (job = job_list; job; job = job->next) {
		જો (job->running_progs == job->stopped_progs)
			status_string = "Stopped";
		નહીંતર
			status_string = "Running";

		printf(JOB_STATUS_FORMAT, job->jobid, status_string, job->text);
	}
	પાછા EXIT_SUCCESS;
}

/* built-in 'pwd' handler */
સ્થિર પૂર્ણાંક builtin_pwd(સંરચના child_prog *dummy)
{
	puts(set_cwd());
	પાછા EXIT_SUCCESS;
}

/* built-in 'read VAR' handler */
સ્થિર પૂર્ણાંક builtin_read(સંરચના child_prog *child)
{
	પૂર્ણાંક res;

	જો (child->argv[1]) {
		અક્ષર string[BUFSIZ];
		અક્ષર *var = 0;

		string[0] = 0;  /* In case stdin has only EOF */
		/* read string */
		fgets(string, કદ(string), stdin);
		chomp(string);
		var = malloc(strlen(child->argv[1])+strlen(string)+2);
		જો(var) {
			sprintf(var, "%s=%s", child->argv[1], string);
			res = set_local_var(var, 0);
		} નહીંતર
			res = -1;
		જો (res)
			fprintf(stderr, "read: %m\n");
		મુક્ત(var);      /* So not move up to avoid breaking errno */
		પાછા res;
	} નહીંતર {
		કરો res=getchar(); જ્યારે(res!='\n' && res!=EOF);
		પાછા 0;
	}
}

/* built-in 'set VAR=value' handler */
સ્થિર પૂર્ણાંક builtin_set(સંરચના child_prog *child)
{
	અક્ષર *temp = child->argv[1];
	સંરચના variables *e;

	જો (temp == ખાલી)
		માટે(e = ટોચ_vars; e; e=e->next)
			printf("%s=%s\n", e->name, e->કિંમત);
	નહીંતર
		set_local_var(temp, 0);

		પાછા EXIT_SUCCESS;
}

/* Built-in 'shift' handler */
સ્થિર પૂર્ણાંક builtin_shift(સંરચના child_prog *child)
{
	પૂર્ણાંક n=1;
	જો (child->argv[1]) {
		n=atoi(child->argv[1]);
	}
	જો (n>=0 && n<global_argc) {
		/* XXX This probably breaks $0 */
		global_argc -= n;
		global_argv += n;
		પાછા EXIT_SUCCESS;
	} નહીંતર {
		પાછા EXIT_FAILURE;
	}
}

/* Built-in '.' handler (read-in and execute commands from file) */
સ્થિર પૂર્ણાંક builtin_source(સંરચના child_prog *child)
{
	FILE *આગત;
	પૂર્ણાંક status;

	જો (child->argv[1] == ખાલી)
		પાછા EXIT_FAILURE;

	/* XXX search through $PATH is missing */
	આગત = fopen(child->argv[1], "r");
	જો (!આગત) {
		error_સંદેશ("Couldn't open file '%s'", child->argv[1]);
		પાછા EXIT_FAILURE;
	}

	/* Now run the file */
	/* XXX argv and argc are broken; need to save old global_argv
	 * (pointer only is OK!) on this stack frame,
	 * set global_argv=child->argv+1, recurse, and restore. */
	mark_open(fileno(આગત));
	status = parse_file_outer(આગત);
	mark_closed(fileno(આગત));
	fclose(આગત);
	પાછા (status);
}

સ્થિર પૂર્ણાંક builtin_umask(સંરચના child_prog *child)
{
	mode_t new_umask;
	અચલ અક્ષર *arg = child->argv[1];
	અક્ષર *end;
	જો (arg) {
		new_umask=strtoul(arg, &end, 8);
		જો (*end!='\0' || end == arg) {
			પાછા EXIT_FAILURE;
		}
	} નહીંતર {
		printf("%.3o\n", (unsigned પૂર્ણાંક) (new_umask=umask(0)));
	}
	umask(new_umask);
	પાછા EXIT_SUCCESS;
}

/* built-in 'unset VAR' handler */
સ્થિર પૂર્ણાંક builtin_unset(સંરચના child_prog *child)
{
	/* bash returned already true */
	unset_local_var(child->argv[1]);
	પાછા EXIT_SUCCESS;
}

સ્થિર પૂર્ણાંક builtin_not_written(સંરચના child_prog *child)
{
	printf("builtin_%s not written\n",child->argv[0]);
	પાછા EXIT_FAILURE;
}
#endif

સ્થિર પૂર્ણાંક b_check_space(o_string *o, પૂર્ણાંક len)
{
	/* It would be easy to drop a more restrictive policy
	 * in here, such as setting a maximum string length */
	જો (o->length + len > o->maxlen) {
		અક્ષર *old_data = o->data;
		/* assert (data == NULL || o->maxlen != 0); */
		o->maxlen += max(2*len, B_CHUNK);
		o->data = realloc(o->data, 1 + o->maxlen);
		જો (o->data == ખાલી) {
			મુક્ત(old_data);
		}
	}
	પાછા o->data == ખાલી;
}

સ્થિર પૂર્ણાંક b_addchr(o_string *o, પૂર્ણાંક ch)
{
	debug_printf("b_addchr: %c %d %p\n", ch, o->length, o);
	જો (b_check_space(o, 1)) પાછા B_NOSPAC;
	o->data[o->length] = ch;
	o->length++;
	o->data[o->length] = '\0';
	પાછા 0;
}

સ્થિર શૂન્ય b_reset(o_string *o)
{
	o->length = 0;
	o->nonnull = 0;
	જો (o->data != ખાલી) *o->data = '\0';
}

સ્થિર શૂન્ય b_free(o_string *o)
{
	b_reset(o);
	મુક્ત(o->data);
	o->data = ખાલી;
	o->maxlen = 0;
}

/* My analysis of quoting semantics tells me that state information
 * is associated with a destination, not a source.
 */
સ્થિર પૂર્ણાંક b_addqchr(o_string *o, પૂર્ણાંક ch, પૂર્ણાંક quote)
{
	જો (quote && strchr("*?[\\",ch)) {
		પૂર્ણાંક rc;
		rc = b_addchr(o, '\\');
		જો (rc) પાછા rc;
	}
	પાછા b_addchr(o, ch);
}

#ifndef __U_BOOT__
સ્થિર પૂર્ણાંક b_adduint(o_string *o, unsigned પૂર્ણાંક i)
{
	પૂર્ણાંક r;
	અક્ષર *p = simple_itoa(i);
	/* no escape checking necessary */
	કરો r=b_addchr(o, *p++); જ્યારે (r==0 && *p);
	પાછા r;
}
#endif

સ્થિર પૂર્ણાંક static_get(સંરચના in_str *i)
{
	પૂર્ણાંક ch = *i->p++;
	જો (ch=='\0') પાછા EOF;
	પાછા ch;
}

સ્થિર પૂર્ણાંક static_peek(સંરચના in_str *i)
{
	પાછા *i->p;
}

#ifndef __U_BOOT__
સ્થિર અંતઃરેખ શૂન્ય cmdedit_set_initial_prompt(શૂન્ય)
{
#ifndef CONFIG_FEATURE_SH_FANCY_PROMPT
	PS1 = ખાલી;
#નહીંતર
	PS1 = env_get("PS1");
	જો(PS1==0)
		PS1 = "\\w \\$ ";
#endif
}

સ્થિર અંતઃરેખ શૂન્ય setup_prompt_string(પૂર્ણાંક promptmode, અક્ષર **prompt_str)
{
	debug_printf("setup_prompt_string %d ",promptmode);
#ifndef CONFIG_FEATURE_SH_FANCY_PROMPT
	/* Set up the prompt */
	જો (promptmode == 1) {
		મુક્ત(PS1);
		PS1=xmalloc(strlen(cwd)+4);
		sprintf(PS1, "%s %s", cwd, ( geteuid() != 0 ) ?  "$ ":"# ");
		*prompt_str = PS1;
	} નહીંતર {
		*prompt_str = PS2;
	}
#નહીંતર
	*prompt_str = (promptmode==1)? PS1 : PS2;
#endif
	debug_printf("result %s\n",*prompt_str);
}
#endif

#ifdef __U_BOOT__
સ્થિર પૂર્ણાંક uboot_cli_readline(સંરચના in_str *i)
{
	અક્ષર *prompt;
	અક્ષર __maybe_unused *ps_prompt = ખાલી;

	જો (i->promptmode == 1)
		prompt = CONFIG_SYS_PROMPT;
	નહીંતર
		prompt = CONFIG_SYS_PROMPT_HUSH_PS2;

#ifdef CONFIG_CMDLINE_PS_SUPPORT
	જો (i->promptmode == 1)
		ps_prompt = env_get("PS1");
	નહીંતર
		ps_prompt = env_get("PS2");
	જો (ps_prompt)
		prompt = ps_prompt;
#endif

	પાછા cli_readline(prompt);
}
#endif

સ્થિર શૂન્ય get_user_આગત(સંરચના in_str *i)
{
#ifndef __U_BOOT__
	અક્ષર *prompt_str;
	સ્થિર અક્ષર the_command[BUFSIZ];

	setup_prompt_string(i->promptmode, &prompt_str);
#ifdef CONFIG_FEATURE_COMMAND_EDITING
	/*
	 ** enable command line editing only while a command line
	 ** is actually being read; otherwise, we'll end up bequeathing
	 ** atexit() handlers and other unwanted stuff to our
	 ** child processes (rob@sysgo.de)
	 */
	cmdedit_read_આગત(prompt_str, the_command);
#નહીંતર
	fputs(prompt_str, stdout);
	fflush(stdout);
	the_command[0]=fgetc(i->file);
	the_command[1]='\0';
#endif
	fflush(stdout);
	i->p = the_command;
#નહીંતર
	પૂર્ણાંક n;
	સ્થિર અક્ષર the_command[CONFIG_SYS_CBSIZE + 1];

	bootretry_reset_cmd_timeout();
	i->__promptme = 1;
	n = uboot_cli_readline(i);

#ifdef CONFIG_BOOT_RETRY_TIME
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
#endif
	જો (n == -1 ) {
		flag_repeat = 0;
		i->__promptme = 0;
	}
	n = strlen(console_buffer);
	console_buffer[n] = '\n';
	console_buffer[n+1]= '\0';
	જો (had_ctrlc()) flag_repeat = 0;
	clear_ctrlc();
	do_repeat = 0;
	જો (i->promptmode == 1) {
		જો (console_buffer[0] == '\n'&& flag_repeat == 0) {
			strcpy(the_command,console_buffer);
		}
		નહીંતર {
			જો (console_buffer[0] != '\n') {
				strcpy(the_command,console_buffer);
				flag_repeat = 1;
			}
			નહીંતર {
				do_repeat = 1;
			}
		}
		i->p = the_command;
	}
	નહીંતર {
		જો (console_buffer[0] != '\n') {
			જો (strlen(the_command) + strlen(console_buffer)
			    < CONFIG_SYS_CBSIZE) {
				n = strlen(the_command);
				the_command[n-1] = ' ';
				strcpy(&the_command[n],console_buffer);
			}
			નહીંતર {
				the_command[0] = '\n';
				the_command[1] = '\0';
				flag_repeat = 0;
			}
		}
		જો (i->__promptme == 0) {
			the_command[0] = '\n';
			the_command[1] = '\0';
		}
		i->p = console_buffer;
	}
#endif
}

/* This is the magic location that prints prompts
 * and gets data back from the user */
સ્થિર પૂર્ણાંક file_get(સંરચના in_str *i)
{
	પૂર્ણાંક ch;

	ch = 0;
	/* If there is data waiting, eat it up */
	જો (i->p && *i->p) {
		ch = *i->p++;
	} નહીંતર {
		/* need to double check i->file because we might be doing something
		 * more complicated by now, like sourcing or substituting. */
#ifndef __U_BOOT__
		જો (i->__promptme && interactive && i->file == stdin) {
			જ્યારે(! i->p || (interactive && strlen(i->p)==0) ) {
#નહીંતર
			જ્યારે(! i->p  || strlen(i->p)==0 ) {
#endif
				get_user_આગત(i);
			}
			i->promptmode=2;
#ifndef __U_BOOT__
			i->__promptme = 0;
#endif
			જો (i->p && *i->p) {
				ch = *i->p++;
			}
#ifndef __U_BOOT__
		} નહીંતર {
			ch = fgetc(i->file);
		}

#endif
		debug_printf("b_getch: got a %d\n", ch);
	}
#ifndef __U_BOOT__
	જો (ch == '\n') i->__promptme=1;
#endif
	પાછા ch;
}

/* All the callers guarantee this routine will never be
 * used right after a newline, so prompting is not needed.
 */
સ્થિર પૂર્ણાંક file_peek(સંરચના in_str *i)
{
#ifndef __U_BOOT__
	જો (i->p && *i->p) {
#endif
		પાછા *i->p;
#ifndef __U_BOOT__
	} નહીંતર {
		i->peek_buf[0] = fgetc(i->file);
		i->peek_buf[1] = '\0';
		i->p = i->peek_buf;
		debug_printf("b_peek: got a %d\n", *i->p);
		પાછા *i->p;
	}
#endif
}

#ifndef __U_BOOT__
સ્થિર શૂન્ય setup_file_in_str(સંરચના in_str *i, FILE *f)
#નહીંતર
સ્થિર શૂન્ય setup_file_in_str(સંરચના in_str *i)
#endif
{
	i->peek = file_peek;
	i->get = file_get;
	i->__promptme=1;
	i->promptmode=1;
#ifndef __U_BOOT__
	i->file = f;
#endif
	i->p = ખાલી;
}

સ્થિર શૂન્ય setup_string_in_str(સંરચના in_str *i, અચલ અક્ષર *s)
{
	i->peek = static_peek;
	i->get = static_get;
	i->__promptme=1;
	i->promptmode=1;
	i->p = s;
}

#ifndef __U_BOOT__
સ્થિર શૂન્ય mark_open(પૂર્ણાંક fd)
{
	સંરચના close_me *new = xmalloc(કદ(સંરચના close_me));
	new->fd = fd;
	new->next = close_me_head;
	close_me_head = new;
}

સ્થિર શૂન્ય mark_closed(પૂર્ણાંક fd)
{
	સંરચના close_me *tmp;
	જો (close_me_head == ખાલી || close_me_head->fd != fd)
		error_સંદેશ_and_die("corrupt close_me");
	tmp = close_me_head;
	close_me_head = close_me_head->next;
	મુક્ત(tmp);
}

સ્થિર શૂન્ય close_all(શૂન્ય)
{
	સંરચના close_me *c;
	માટે (c=close_me_head; c; c=c->next) {
		close(c->fd);
	}
	close_me_head = ખાલી;
}

/* squirrel != NULL means we squirrel away copies of stdin, stdout,
 * and stderr if they are redirected. */
સ્થિર પૂર્ણાંક setup_redirects(સંરચના child_prog *prog, પૂર્ણાંક squirrel[])
{
	પૂર્ણાંક openfd, mode;
	સંરચના redir_struct *redir;

	માટે (redir=prog->redirects; redir; redir=redir->next) {
		જો (redir->dup == -1 && redir->word.gl_pathv == ખાલી) {
			/* something went wrong in the parse.  Pretend it didn't happen */
			ચાલુ;
		}
		જો (redir->dup == -1) {
			mode=redir_table[redir->type].mode;
			openfd = open(redir->word.gl_pathv[0], mode, 0666);
			જો (openfd < 0) {
			/* this could get lost if stderr has been redirected, but
			   bash and ash both lose it as well (though zsh doesn't!) */
				perror_સંદેશ("error opening %s", redir->word.gl_pathv[0]);
				પાછા 1;
			}
		} નહીંતર {
			openfd = redir->dup;
		}

		જો (openfd != redir->fd) {
			જો (squirrel && redir->fd < 3) {
				squirrel[redir->fd] = dup(redir->fd);
			}
			જો (openfd == -3) {
				close(openfd);
			} નહીંતર {
				dup2(openfd, redir->fd);
				જો (redir->dup == -1)
					close (openfd);
			}
		}
	}
	પાછા 0;
}

સ્થિર શૂન્ય restore_redirects(પૂર્ણાંક squirrel[])
{
	પૂર્ણાંક i, fd;
	માટે (i=0; i<3; i++) {
		fd = squirrel[i];
		જો (fd != -1) {
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
સ્થિર શૂન્ય pseudo_exec(સંરચના child_prog *child)
{
	પૂર્ણાંક i, rcode;
	અક્ષર *p;
	સંરચના built_in_command *x;
	જો (child->argv) {
		માટે (i=0; is_assignment(child->argv[i]); i++) {
			debug_printf("pid %d environment modification: %s\n",getpid(),child->argv[i]);
			p = ઉમેરો_var_કિંમત(child->argv[i]);
			putenv(strdup(p));
			જો (p != child->argv[i]) મુક્ત(p);
		}
		child->argv+=i;  /* XXX this hack isn't so horrible, since we are about
					to exit, and therefore don't need to keep data
					structures consistent for free() use. */
		/* If a variable is assigned in a forest, and nobody listens,
		 * was it ever really set?
		 */
		જો (child->argv[0] == ખાલી) {
			_exit(EXIT_SUCCESS);
		}

		/*
		 * Check if the command matches any of the builtins.
		 * Depending on context, this might be redundant.  But it's
		 * easier to waste a few CPU cycles than it is to figure out
		 * if this is one of those cases.
		 */
		માટે (x = bltins; x->cmd; x++) {
			જો (strcmp(child->argv[0], x->cmd) == 0 ) {
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
			પૂર્ણાંક argc_l;
			અક્ષર** argv_l=child->argv;
			અક્ષર *name = child->argv[0];

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
			માટે(argc_l=0;*argv_l!=ખાલી; argv_l++, argc_l++);
			optind = 1;
			debug_printf("running applet %s\n", name);
			run_applet_by_name(name, argc_l, child->argv);
		}
#endif
		debug_printf("exec of %s\n",child->argv[0]);
		execvp(child->argv[0],child->argv);
		perror_સંદેશ("couldn't exec: %s",child->argv[0]);
		_exit(1);
	} નહીંતર જો (child->group) {
		debug_printf("runtime nesting to group\n");
		interactive=0;    /* crucial!!!! */
		rcode = run_list_real(child->group);
		/* OK to leak memory by not calling free_pipe_list,
		 * since this process is about to exit */
		_exit(rcode);
	} નહીંતર {
		/* Can happen.  See what bash does with ">foo" by itself. */
		debug_printf("trying to pseudo_exec null command\n");
		_exit(EXIT_SUCCESS);
	}
}

સ્થિર શૂન્ય ઉમેરો_bg_job(સંરચના pipe *pi)
{
	સંરચના pipe *thejob;

	/* Linear search for the ID of the job to use */
	pi->jobid = 1;
	માટે (thejob = job_list; thejob; thejob = thejob->next)
		જો (thejob->jobid >= pi->jobid)
			pi->jobid = thejob->jobid + 1;

	/* add thejob to the list of running jobs */
	જો (!job_list) {
		thejob = job_list = xmalloc(કદ(*thejob));
	} નહીંતર {
		માટે (thejob = job_list; thejob->next; thejob = thejob->next) /* nothing */;
		thejob->next = xmalloc(કદ(*thejob));
		thejob = thejob->next;
	}

	/* physically copy the struct job */
	memcpy(thejob, pi, કદ(સંરચના pipe));
	thejob->next = ખાલી;
	thejob->running_progs = thejob->અંક_progs;
	thejob->stopped_progs = 0;
	thejob->text = xmalloc(BUFSIZ); /* cmdedit buffer size */

	/*if (pi->progs[0] && pi->progs[0].argv && pi->progs[0].argv[0]) */
	{
		અક્ષર *bar=thejob->text;
		અક્ષર **foo=pi->progs[0].argv;
		જ્યારે(foo && *foo) {
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
સ્થિર શૂન્ય remove_bg_job(સંરચના pipe *pi)
{
	સંરચના pipe *prev_pipe;

	જો (pi == job_list) {
		job_list = pi->next;
	} નહીંતર {
		prev_pipe = job_list;
		જ્યારે (prev_pipe->next != pi)
			prev_pipe = prev_pipe->next;
		prev_pipe->next = pi->next;
	}
	જો (job_list)
		last_jobid = job_list->jobid;
	નહીંતર
		last_jobid = 0;

	pi->stopped_progs = 0;
	free_pipe(pi, 0);
	મુક્ત(pi);
}

/* Checks to see if any processes have exited -- if they
   have, figure out why and see if a job has completed */
સ્થિર પૂર્ણાંક checkjobs(સંરચના pipe* fg_pipe)
{
	પૂર્ણાંક attributes;
	પૂર્ણાંક status;
	પૂર્ણાંક prognum = 0;
	સંરચના pipe *pi;
	pid_t childpid;

	attributes = WUNTRACED;
	જો (fg_pipe==ખાલી) {
		attributes |= WNOHANG;
	}

	જ્યારે ((childpid = waitpid(-1, &status, attributes)) > 0) {
		જો (fg_pipe) {
			પૂર્ણાંક i, rcode = 0;
			માટે (i=0; i < fg_pipe->અંક_progs; i++) {
				જો (fg_pipe->progs[i].pid == childpid) {
					જો (i==fg_pipe->અંક_progs-1)
						rcode=WEXITSTATUS(status);
					(fg_pipe->અંક_progs)--;
					પાછા(rcode);
				}
			}
		}

		માટે (pi = job_list; pi; pi = pi->next) {
			prognum = 0;
			જ્યારે (prognum < pi->અંક_progs && pi->progs[prognum].pid != childpid) {
				prognum++;
			}
			જો (prognum < pi->અંક_progs)
				તોડો;
		}

		જો(pi==ખાલી) {
			debug_printf("checkjobs: pid %d was not in our list!\n", childpid);
			ચાલુ;
		}

		જો (WIFEXITED(status) || WIFSIGNALED(status)) {
			/* child exited */
			pi->running_progs--;
			pi->progs[prognum].pid = 0;

			જો (!pi->running_progs) {
				printf(JOB_STATUS_FORMAT, pi->jobid, "Done", pi->text);
				remove_bg_job(pi);
			}
		} નહીંતર {
			/* child stopped */
			pi->stopped_progs++;
			pi->progs[prognum].is_stopped = 1;

#જો 0
			/* Printing this stuff is a pain, since it tends to
			 * overwrite the prompt an inconveinient moments.  So
			 * don't do that.  */
			જો (pi->stopped_progs == pi->અંક_progs) {
				printf("\n"JOB_STATUS_FORMAT, pi->jobid, "Stopped", pi->text);
			}
#endif
		}
	}

	જો (childpid == -1 && errno != ECHILD)
		perror_સંદેશ("waitpid");

	/* move the shell to the foreground */
	/*if (interactive && tcsetpgrp(shell_terminal, getpgid(0))) */
	/*	perror_msg("tcsetpgrp-2"); */
	પાછા -1;
}

/* Figure out our controlling tty, checking in order stderr,
 * stdin, and stdout.  If check_pgrp is set, also check that
 * we belong to the foreground process group associated with
 * that tty.  The value of shell_terminal is needed in order to call
 * tcsetpgrp(shell_terminal, ...); */
શૂન્ય controlling_tty(પૂર્ણાંક check_pgrp)
{
	pid_t curpgrp;

	જો ((curpgrp = tcgetpgrp(shell_terminal = 2)) < 0
			&& (curpgrp = tcgetpgrp(shell_terminal = 0)) < 0
			&& (curpgrp = tcgetpgrp(shell_terminal = 1)) < 0)
		જાઓ shell_terminal_error;

	જો (check_pgrp && curpgrp != getpgid(0))
		જાઓ shell_terminal_error;

	પાછા;

shell_terminal_error:
		shell_terminal = -1;
		પાછા;
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
સ્થિર પૂર્ણાંક run_pipe_real(સંરચના pipe *pi)
{
	પૂર્ણાંક i;
#ifndef __U_BOOT__
	પૂર્ણાંક nextin, nextout;
	પૂર્ણાંક pipefds[2];				/* pipefds[0] is for reading */
	સંરચના child_prog *child;
	સંરચના built_in_command *x;
	અક્ષર *p;
# જો __GNUC__
	/* Avoid longjmp clobbering */
	(શૂન્ય) &i;
	(શૂન્ય) &nextin;
	(શૂન્ય) &nextout;
	(શૂન્ય) &child;
# endif
#નહીંતર
	પૂર્ણાંક nextin;
	પૂર્ણાંક flag = do_repeat ? CMD_FLAG_REPEAT : 0;
	સંરચના child_prog *child;
	અક્ષર *p;
# જો __GNUC__
	/* Avoid longjmp clobbering */
	(શૂન્ય) &i;
	(શૂન્ય) &nextin;
	(શૂન્ય) &child;
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
	જો (pi->અંક_progs == 1) child = & (pi->progs[0]);
#ifndef __U_BOOT__
	જો (pi->અંક_progs == 1 && child->group && child->subshell == 0) {
		પૂર્ણાંક squirrel[] = {-1, -1, -1};
		પૂર્ણાંક rcode;
		debug_printf("non-subshell grouping\n");
		setup_redirects(child, squirrel);
		/* XXX could we merge code with following builtin case,
		 * by creating a pseudo builtin that calls run_list_real? */
		rcode = run_list_real(child->group);
		restore_redirects(squirrel);
#નહીંતર
		જો (pi->અંક_progs == 1 && child->group) {
		પૂર્ણાંક rcode;
		debug_printf("non-subshell grouping\n");
		rcode = run_list_real(child->group);
#endif
		પાછા rcode;
	} નહીંતર જો (pi->અંક_progs == 1 && pi->progs[0].argv != ખાલી) {
		માટે (i=0; is_assignment(child->argv[i]); i++) { /* nothing */ }
		જો (i!=0 && child->argv[i]==ખાલી) {
			/* assignments, but no command: set the local environment */
			માટે (i=0; child->argv[i]!=ખાલી; i++) {

				/* Ok, this case is tricky.  We have to decide if this is a
				 * local variable, or an already exported variable.  If it is
				 * already exported, we have to export the new value.  If it is
				 * not exported, we need only set this as a local variable.
				 * This junk is all to decide whether or not to export this
				 * variable. */
				પૂર્ણાંક export_me=0;
				અક્ષર *name, *કિંમત;
				name = xstrdup(child->argv[i]);
				debug_printf("Local environment set: %s\n", name);
				કિંમત = strchr(name, '=');
				જો (કિંમત)
					*કિંમત=0;
#ifndef __U_BOOT__
				જો ( get_local_var(name)) {
					export_me=1;
				}
#endif
				મુક્ત(name);
				p = ઉમેરો_var_કિંમત(child->argv[i]);
				set_local_var(p, export_me);
				જો (p != child->argv[i]) મુક્ત(p);
			}
			પાછા EXIT_SUCCESS;   /* don't worry about errors in set_local_var() yet */
		}
		માટે (i = 0; is_assignment(child->argv[i]); i++) {
			p = ઉમેરો_var_કિંમત(child->argv[i]);
#ifndef __U_BOOT__
			putenv(strdup(p));
#નહીંતર
			set_local_var(p, 0);
#endif
			જો (p != child->argv[i]) {
				child->sp--;
				મુક્ત(p);
			}
		}
		જો (child->sp) {
			અક્ષર * str = ખાલી;

			str = make_string(child->argv + i,
					  child->argv_nonnull + i);
			parse_string_outer(str, FLAG_EXIT_FROM_LOOP | FLAG_REPARSING);
			મુક્ત(str);
			પાછા last_return_code;
		}
#ifndef __U_BOOT__
		માટે (x = bltins; x->cmd; x++) {
			જો (strcmp(child->argv[i], x->cmd) == 0 ) {
				પૂર્ણાંક squirrel[] = {-1, -1, -1};
				પૂર્ણાંક rcode;
				જો (x->function == builtin_exec && child->argv[i+1]==ખાલી) {
					debug_printf("magic exec\n");
					setup_redirects(child,ખાલી);
					પાછા EXIT_SUCCESS;
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
			પાછા rcode;
		}
#નહીંતર
		/* check ";", because ,example , argv consist from
		 * "help;flinfo" must not execute
		 */
		જો (strchr(child->argv[i], ';')) {
			printf("Unknown command '%s' - try 'help' or use "
					"'run' command\n", child->argv[i]);
			પાછા -1;
		}
		/* Process the command */
		પાછા cmd_process(flag, child->argc - i, child->argv + i,
				   &flag_repeat, ખાલી);
#endif
	}
#ifndef __U_BOOT__

	માટે (i = 0; i < pi->અંક_progs; i++) {
		child = & (pi->progs[i]);

		/* pipes are inserted between pairs of commands */
		જો ((i + 1) < pi->અંક_progs) {
			જો (pipe(pipefds)<0) perror_સંદેશ_and_die("pipe");
			nextout = pipefds[1];
		} નહીંતર {
			nextout=1;
			pipefds[0] = -1;
		}

		/* XXX test for failed fork()? */
		જો (!(child->pid = fork())) {
			/* Set the handling for job control signals back to the default.  */
			સંકેત(SIGINT, SIG_DFL);
			સંકેત(SIGQUIT, SIG_DFL);
			સંકેત(SIGTERM, SIG_DFL);
			સંકેત(SIGTSTP, SIG_DFL);
			સંકેત(SIGTTIN, SIG_DFL);
			સંકેત(SIGTTOU, SIG_DFL);
			સંકેત(SIGCHLD, SIG_DFL);

			close_all();

			જો (nextin != 0) {
				dup2(nextin, 0);
				close(nextin);
			}
			જો (nextout != 1) {
				dup2(nextout, 1);
				close(nextout);
			}
			જો (pipefds[0]!=-1) {
				close(pipefds[0]);  /* opposite end of our output pipe */
			}

			/* Like bash, explicit redirects override pipes,
			 * and the pipe fd is available for dup'ing. */
			setup_redirects(child,ખાલી);

			જો (interactive && pi->followup!=PIPE_BG) {
				/* If we (the child) win the race, put ourselves in the process
				 * group whose leader is the first process in this pipe. */
				જો (pi->pgrp < 0) {
					pi->pgrp = getpid();
				}
				જો (setpgid(0, pi->pgrp) == 0) {
					tcsetpgrp(2, pi->pgrp);
				}
			}

			pseudo_exec(child);
		}

		/* put our child in the process group whose leader is the
		   first process in this pipe */
		જો (pi->pgrp < 0) {
			pi->pgrp = child->pid;
		}
		/* Don't check for errors.  The child may be dead already,
		 * in which case setpgid returns error code EACCES. */
		setpgid(child->pid, pi->pgrp);

		જો (nextin != 0)
			close(nextin);
		જો (nextout != 1)
			close(nextout);

		/* If there isn't another process, nextin is garbage
		   but it doesn't matter */
		nextin = pipefds[0];
	}
#endif
	પાછા -1;
}

સ્થિર પૂર્ણાંક run_list_real(સંરચના pipe *pi)
{
	અક્ષર *save_name = ખાલી;
	અક્ષર **list = ખાલી;
	અક્ષર **save_list = ખાલી;
	સંરચના pipe *rpipe;
	પૂર્ણાંક flag_rep = 0;
#ifndef __U_BOOT__
	પૂર્ણાંક save_અંક_progs;
#endif
	પૂર્ણાંક rcode=0, flag_skip=1;
	પૂર્ણાંક flag_restore = 0;
	પૂર્ણાંક if_code=0, next_if_code=0;  /* need double-buffer to handle elif */
	reserved_style rmode, skip_more_in_this_rmode=RES_XXXX;
	/* check syntax for "for" */
	માટે (rpipe = pi; rpipe; rpipe = rpipe->next) {
		જો ((rpipe->r_mode == RES_IN ||
		    rpipe->r_mode == RES_FOR) &&
		    (rpipe->next == ખાલી)) {
				syntax();
#ifdef __U_BOOT__
				flag_repeat = 0;
#endif
				પાછા 1;
		}
		જો ((rpipe->r_mode == RES_IN &&
			(rpipe->next->r_mode == RES_IN &&
			rpipe->next->progs->argv != ખાલી))||
			(rpipe->r_mode == RES_FOR &&
			rpipe->next->r_mode != RES_IN)) {
				syntax();
#ifdef __U_BOOT__
				flag_repeat = 0;
#endif
				પાછા 1;
		}
	}
	માટે (; pi; pi = (flag_restore != 0) ? rpipe : pi->next) {
		જો (pi->r_mode == RES_WHILE || pi->r_mode == RES_UNTIL ||
			pi->r_mode == RES_FOR) {
#ifdef __U_BOOT__
				/* check Ctrl-C */
				ctrlc();
				જો ((had_ctrlc())) {
					પાછા 1;
				}
#endif
				flag_restore = 0;
				જો (!rpipe) {
					flag_rep = 0;
					rpipe = pi;
				}
		}
		rmode = pi->r_mode;
		debug_printf("rmode=%d  if_code=%d  next_if_code=%d skip_more=%d\n", rmode, if_code, next_if_code, skip_more_in_this_rmode);
		જો (rmode == skip_more_in_this_rmode && flag_skip) {
			જો (pi->followup == PIPE_SEQ) flag_skip=0;
			ચાલુ;
		}
		flag_skip = 1;
		skip_more_in_this_rmode = RES_XXXX;
		જો (rmode == RES_THEN || rmode == RES_ELSE) if_code = next_if_code;
		જો (rmode == RES_THEN &&  if_code) ચાલુ;
		જો (rmode == RES_ELSE && !if_code) ચાલુ;
		જો (rmode == RES_ELIF && !if_code) તોડો;
		જો (rmode == RES_FOR && pi->અંક_progs) {
			જો (!list) {
				/* if no variable values after "in" we skip "for" */
				જો (!pi->next->progs->argv) ચાલુ;
				/* create list of variable values */
				list = make_list_in(pi->next->progs->argv,
					pi->progs->argv[0]);
				save_list = list;
				save_name = pi->progs->argv[0];
				pi->progs->argv[0] = ખાલી;
				flag_rep = 1;
			}
			જો (!(*list)) {
				મુક્ત(pi->progs->argv[0]);
				મુક્ત(save_list);
				list = ખાલી;
				flag_rep = 0;
				pi->progs->argv[0] = save_name;
#ifndef __U_BOOT__
				pi->progs->glob_result.gl_pathv[0] =
					pi->progs->argv[0];
#endif
				ચાલુ;
			} નહીંતર {
				/* insert new value from list for variable */
				મુક્ત(pi->progs->argv[0]);
				pi->progs->argv[0] = *list++;
#ifndef __U_BOOT__
				pi->progs->glob_result.gl_pathv[0] =
					pi->progs->argv[0];
#endif
			}
		}
		જો (rmode == RES_IN) ચાલુ;
		જો (rmode == RES_DO) {
			જો (!flag_rep) ચાલુ;
		}
		જો (rmode == RES_DONE) {
			જો (flag_rep) {
				flag_restore = 1;
			} નહીંતર {
				rpipe = ખાલી;
			}
		}
		જો (pi->અંક_progs == 0) ચાલુ;
#ifndef __U_BOOT__
		save_અંક_progs = pi->અંક_progs; /* save number of programs */
#endif
		rcode = run_pipe_real(pi);
		debug_printf("run_pipe_real returned %d\n",rcode);
#ifndef __U_BOOT__
		જો (rcode!=-1) {
			/* We only ran a builtin: rcode was set by the return value
			 * of run_pipe_real(), and we don't need to wait for anything. */
		} નહીંતર જો (pi->followup==PIPE_BG) {
			/* XXX check bash's behavior with nontrivial pipes */
			/* XXX compute jobid */
			/* XXX what does bash do with attempts to background builtins? */
			ઉમેરો_bg_job(pi);
			rcode = EXIT_SUCCESS;
		} નહીંતર {
			જો (interactive) {
				/* move the new process group into the foreground */
				જો (tcsetpgrp(shell_terminal, pi->pgrp) && errno != ENOTTY)
					perror_સંદેશ("tcsetpgrp-3");
				rcode = checkjobs(pi);
				/* move the shell to the foreground */
				જો (tcsetpgrp(shell_terminal, getpgid(0)) && errno != ENOTTY)
					perror_સંદેશ("tcsetpgrp-4");
			} નહીંતર {
				rcode = checkjobs(pi);
			}
			debug_printf("checkjobs returned %d\n",rcode);
		}
		last_return_code=rcode;
#નહીંતર
		જો (rcode < -1) {
			last_return_code = -rcode - 2;
			પાછા -2;	/* exit */
		}
		last_return_code = rcode;
#endif
#ifndef __U_BOOT__
		pi->અંક_progs = save_અંક_progs; /* restore number of programs */
#endif
		જો ( rmode == RES_IF || rmode == RES_ELIF )
			next_if_code=rcode;  /* can be overwritten a number of times */
		જો (rmode == RES_WHILE)
			flag_rep = !last_return_code;
		જો (rmode == RES_UNTIL)
			flag_rep = last_return_code;
		જો ( (rcode==EXIT_SUCCESS && pi->followup==PIPE_OR) ||
		     (rcode!=EXIT_SUCCESS && pi->followup==PIPE_AND) )
			skip_more_in_this_rmode=rmode;
#ifndef __U_BOOT__
		checkjobs(ખાલી);
#endif
	}
	પાછા rcode;
}

/* broken, of course, but OK for testing */
સ્થિર અક્ષર *indenter(પૂર્ણાંક i)
{
	સ્થિર અક્ષર blanks[]="                                    ";
	પાછા &blanks[કદ(blanks)-i-1];
}

/* return code is the exit status of the pipe */
સ્થિર પૂર્ણાંક free_pipe(સંરચના pipe *pi, પૂર્ણાંક indent)
{
	અક્ષર **p;
	સંરચના child_prog *child;
#ifndef __U_BOOT__
	સંરચના redir_struct *r, *rnext;
#endif
	પૂર્ણાંક a, i, ret_code=0;
	અક્ષર *ind = indenter(indent);

#ifndef __U_BOOT__
	જો (pi->stopped_progs > 0)
		પાછા ret_code;
	final_printf("%s run pipe: (pid %d)\n",ind,getpid());
#endif
	માટે (i=0; i<pi->અંક_progs; i++) {
		child = &pi->progs[i];
		final_printf("%s  command %d:\n",ind,i);
		જો (child->argv) {
			માટે (a=0,p=child->argv; *p; a++,p++) {
				final_printf("%s   argv[%d] = %s\n",ind,a,*p);
			}
#ifndef __U_BOOT__
			globfree(&child->glob_result);
#નહીંતર
			માટે (a = 0; a < child->argc; a++) {
				મુક્ત(child->argv[a]);
			}
			મુક્ત(child->argv);
			મુક્ત(child->argv_nonnull);
			child->argc = 0;
#endif
			child->argv=ખાલી;
		} નહીંતર જો (child->group) {
#ifndef __U_BOOT__
			final_printf("%s   begin group (subshell:%d)\n",ind, child->subshell);
#endif
			ret_code = free_pipe_list(child->group,indent+3);
			final_printf("%s   end group\n",ind);
		} નહીંતર {
			final_printf("%s   (nil)\n",ind);
		}
#ifndef __U_BOOT__
		માટે (r=child->redirects; r; r=rnext) {
			final_printf("%s   redirect %d%s", ind, r->fd, redir_table[r->type].descrip);
			જો (r->dup == -1) {
				/* guard against the case >$FOO, where foo is unset or blank */
				જો (r->word.gl_pathv) {
					final_printf(" %s\n", *r->word.gl_pathv);
					globfree(&r->word);
				}
			} નહીંતર {
				final_printf("&%d\n", r->dup);
			}
			rnext=r->next;
			મુક્ત(r);
		}
		child->redirects=ખાલી;
#endif
	}
	મુક્ત(pi->progs);   /* children are an array, they get freed all at once */
	pi->progs=ખાલી;
	પાછા ret_code;
}

સ્થિર પૂર્ણાંક free_pipe_list(સંરચના pipe *head, પૂર્ણાંક indent)
{
	પૂર્ણાંક rcode=0;   /* if list has no members */
	સંરચના pipe *pi, *next;
	અક્ષર *ind = indenter(indent);
	માટે (pi=head; pi; pi=next) {
		final_printf("%s pipe reserved mode %d\n", ind, pi->r_mode);
		rcode = free_pipe(pi, indent);
		final_printf("%s pipe followup code %d\n", ind, pi->followup);
		next=pi->next;
		pi->next=ખાલી;
		મુક્ત(pi);
	}
	પાછા rcode;
}

/* Select which version we will use */
સ્થિર પૂર્ણાંક run_list(સંરચના pipe *pi)
{
	પૂર્ણાંક rcode=0;
#ifndef __U_BOOT__
	જો (fake_mode==0) {
#endif
		rcode = run_list_real(pi);
#ifndef __U_BOOT__
	}
#endif
	/* free_pipe_list has the side effect of clearing memory
	 * In the long run that function can be merged with run_list_real,
	 * but doing that now would hobble the debugging effort. */
	free_pipe_list(pi,0);
	પાછા rcode;
}

/* The API for glob is arguably broken.  This routine pushes a non-matching
 * string into the output structure, removing non-backslashed backslashes.
 * If someone can prove me wrong, by performing this function within the
 * original glob(3) api, feel free to rewrite this routine into oblivion.
 * Return code (0 vs. GLOB_NOSPACE) matches glob(3).
 * XXX broken if the last character is '\\', check that before calling.
 */
#ifndef __U_BOOT__
સ્થિર પૂર્ણાંક globhack(અચલ અક્ષર *src, પૂર્ણાંક flags, glob_t *pglob)
{
	પૂર્ણાંક cnt=0, pathc;
	અચલ અક્ષર *s;
	અક્ષર *dest;
	માટે (cnt=1, s=src; s && *s; s++) {
		જો (*s == '\\') s++;
		cnt++;
	}
	dest = malloc(cnt);
	જો (!dest) પાછા GLOB_NOSPACE;
	જો (!(flags & GLOB_APPEND)) {
		pglob->gl_pathv=ખાલી;
		pglob->gl_pathc=0;
		pglob->gl_offs=0;
		pglob->gl_offs=0;
	}
	pathc = ++pglob->gl_pathc;
	pglob->gl_pathv = realloc(pglob->gl_pathv, (pathc+1)*કદ(*pglob->gl_pathv));
	જો (pglob->gl_pathv == ખાલી) પાછા GLOB_NOSPACE;
	pglob->gl_pathv[pathc-1]=dest;
	pglob->gl_pathv[pathc]=ખાલી;
	માટે (s=src; s && *s; s++, dest++) {
		જો (*s == '\\') s++;
		*dest = *s;
	}
	*dest='\0';
	પાછા 0;
}

/* XXX broken if the last character is '\\', check that before calling */
સ્થિર પૂર્ણાંક glob_needed(અચલ અક્ષર *s)
{
	માટે (; *s; s++) {
		જો (*s == '\\') s++;
		જો (strchr("*[?",*s)) પાછા 1;
	}
	પાછા 0;
}

#જો 0
સ્થિર શૂન્ય globprint(glob_t *pglob)
{
	પૂર્ણાંક i;
	debug_printf("glob_t at %p:\n", pglob);
	debug_printf("  gl_pathc=%d  gl_pathv=%p  gl_offs=%d  gl_flags=%d\n",
		pglob->gl_pathc, pglob->gl_pathv, pglob->gl_offs, pglob->gl_flags);
	માટે (i=0; i<pglob->gl_pathc; i++)
		debug_printf("pglob->gl_pathv[%d] = %p = %s\n", i,
			pglob->gl_pathv[i], pglob->gl_pathv[i]);
}
#endif

સ્થિર પૂર્ણાંક xglob(o_string *dest, પૂર્ણાંક flags, glob_t *pglob)
{
	પૂર્ણાંક gr;

	/* short-circuit for null word */
	/* we can code this better when the debug_printf's are gone */
	જો (dest->length == 0) {
		જો (dest->nonnull) {
			/* bash man page calls this an "explicit" null */
			gr = globhack(dest->data, flags, pglob);
			debug_printf("globhack returned %d\n",gr);
		} નહીંતર {
			પાછા 0;
		}
	} નહીંતર જો (glob_needed(dest->data)) {
		gr = glob(dest->data, flags, ખાલી, pglob);
		debug_printf("glob returned %d\n",gr);
		જો (gr == GLOB_NOMATCH) {
			/* quote removal, or more accurately, backslash removal */
			gr = globhack(dest->data, flags, pglob);
			debug_printf("globhack returned %d\n",gr);
		}
	} નહીંતર {
		gr = globhack(dest->data, flags, pglob);
		debug_printf("globhack returned %d\n",gr);
	}
	જો (gr == GLOB_NOSPACE)
		error_સંદેશ_and_die("out of memory during glob");
	જો (gr != 0) { /* GLOB_ABORTED ? */
		error_સંદેશ("glob(3) error %d",gr);
	}
	/* globprint(glob_target); */
	પાછા gr;
}
#endif

#ifdef __U_BOOT__
સ્થિર અક્ષર *get_dollar_var(અક્ષર ch);
#endif

/* This is used to get/check local shell variables */
અક્ષર *get_local_var(અચલ અક્ષર *s)
{
	સંરચના variables *cur;

	જો (!s)
		પાછા ખાલી;

#ifdef __U_BOOT__
	જો (*s == '$')
		પાછા get_dollar_var(s[1]);
#endif

	માટે (cur = ટોચ_vars; cur; cur=cur->next)
		જો(strcmp(cur->name, s)==0)
			પાછા cur->કિંમત;
	પાછા ખાલી;
}

/* This is used to set local shell variables
   flg_export==0 if only local (not exporting) variable
   flg_export==1 if "new" exporting environ
   flg_export>1  if current startup environ (not call putenv()) */
પૂર્ણાંક set_local_var(અચલ અક્ષર *s, પૂર્ણાંક flg_export)
{
	અક્ષર *name, *કિંમત;
	પૂર્ણાંક result=0;
	સંરચના variables *cur;

#ifdef __U_BOOT__
	/* might be possible! */
	જો (!isalpha(*s))
		પાછા -1;
#endif

	name=strdup(s);

	/* Assume when we enter this function that we are already in
	 * NAME=VALUE format.  So the first order of business is to
	 * split 's' on the '=' into 'name' and 'value' */
	કિંમત = strchr(name, '=');
	જો (!કિંમત) {
		મુક્ત(name);
		પાછા -1;
	}
	*કિંમત++ = 0;

	જો (!*કિંમત) {
		unset_local_var(name);
		મુક્ત(name);
		પાછા 0;
	}

	માટે(cur = ટોચ_vars; cur; cur = cur->next) {
		જો(strcmp(cur->name, name)==0)
			તોડો;
	}

	જો(cur) {
		જો(strcmp(cur->કિંમત, કિંમત)==0) {
			જો(flg_export>0 && cur->flg_export==0)
				cur->flg_export=flg_export;
			નહીંતર
				result++;
		} નહીંતર {
			જો(cur->flg_read_only) {
				error_સંદેશ("%s: readonly variable", name);
				result = -1;
			} નહીંતર {
				જો(flg_export>0 || cur->flg_export>1)
					cur->flg_export=1;
				મુક્ત(cur->કિંમત);

				cur->કિંમત = strdup(કિંમત);
			}
		}
	} નહીંતર {
		cur = malloc(કદ(સંરચના variables));
		જો(!cur) {
			result = -1;
		} નહીંતર {
			cur->name = strdup(name);
			જો (cur->name == ખાલી) {
				મુક્ત(cur);
				result = -1;
			} નહીંતર {
				સંરચના variables *bottom = ટોચ_vars;
				cur->કિંમત = strdup(કિંમત);
				cur->next = ખાલી;
				cur->flg_export = flg_export;
				cur->flg_read_only = 0;
				જ્યારે(bottom->next) bottom=bottom->next;
				bottom->next = cur;
			}
		}
	}

#ifndef __U_BOOT__
	જો(result==0 && cur->flg_export==1) {
		*(કિંમત-1) = '=';
		result = putenv(name);
	} નહીંતર {
#endif
		મુક્ત(name);
#ifndef __U_BOOT__
		જો(result>0)            /* equivalent to previous set */
			result = 0;
	}
#endif
	પાછા result;
}

શૂન્ય unset_local_var(અચલ અક્ષર *name)
{
	સંરચના variables *cur;

	જો (name) {
		માટે (cur = ટોચ_vars; cur; cur=cur->next) {
			જો(strcmp(cur->name, name)==0)
				તોડો;
		}
		જો (cur != ખાલી) {
			સંરચના variables *next = ટોચ_vars;
			જો(cur->flg_read_only) {
				error_સંદેશ("%s: readonly variable", name);
				પાછા;
			} નહીંતર {
#ifndef __U_BOOT__
				જો(cur->flg_export)
					unenv_set(cur->name);
#endif
				મુક્ત(cur->name);
				મુક્ત(cur->કિંમત);
				જ્યારે (next->next != cur)
					next = next->next;
				next->next = cur->next;
			}
			મુક્ત(cur);
		}
	}
}

સ્થિર પૂર્ણાંક is_assignment(અચલ અક્ષર *s)
{
	જો (s == ખાલી)
		પાછા 0;

	જો (!isalpha(*s)) પાછા 0;
	++s;
	જ્યારે(isalnum(*s) || *s=='_') ++s;
	પાછા *s=='=';
}

#ifndef __U_BOOT__
/* the src parameter allows us to peek forward to a possible &n syntax
 * for file descriptor duplication, e.g., "2>&1".
 * Return code is 0 normally, 1 if a syntax error is detected in src.
 * Resource errors (in xmalloc) cause the process to exit */
સ્થિર પૂર્ણાંક setup_redirect(સંરચના p_context *ctx, પૂર્ણાંક fd, redir_type style,
	સંરચના in_str *આગત)
{
	સંરચના child_prog *child=ctx->child;
	સંરચના redir_struct *redir = child->redirects;
	સંરચના redir_struct *last_redir=ખાલી;

	/* Create a new redir_struct and drop it onto the end of the linked list */
	જ્યારે(redir) {
		last_redir=redir;
		redir=redir->next;
	}
	redir = xmalloc(કદ(સંરચના redir_struct));
	redir->next=ખાલી;
	redir->word.gl_pathv=ખાલી;
	જો (last_redir) {
		last_redir->next=redir;
	} નહીંતર {
		child->redirects=redir;
	}

	redir->type=style;
	redir->fd= (fd==-1) ? redir_table[style].default_fd : fd ;

	debug_printf("Redirect type %d%s\n", redir->fd, redir_table[style].descrip);

	/* Check for a '2>&1' type redirect */
	redir->dup = redirect_dup_અંક(આગત);
	જો (redir->dup == -2) પાછા 1;  /* syntax error */
	જો (redir->dup != -1) {
		/* Erik had a check here that the file descriptor in question
		 * is legit; I postpone that to "run time"
		 * A "-" representation of "close me" shows up as a -3 here */
		debug_printf("Duplicating redirect '%d>&%d'\n", redir->fd, redir->dup);
	} નહીંતર {
		/* We do _not_ try to open the file that src points to,
		 * since we need to return and let src be expanded first.
		 * Set ctx->pending_redirect, so we know what to do at the
		 * end of the next parsed word.
		 */
		ctx->pending_redirect = redir;
	}
	પાછા 0;
}
#endif

સ્થિર સંરચના pipe *new_pipe(શૂન્ય)
{
	સંરચના pipe *pi;
	pi = xmalloc(કદ(સંરચના pipe));
	pi->અંક_progs = 0;
	pi->progs = ખાલી;
	pi->next = ખાલી;
	pi->followup = 0;  /* invalid */
	pi->r_mode = RES_NONE;
	પાછા pi;
}

સ્થિર શૂન્ય initialize_context(સંરચના p_context *ctx)
{
	ctx->pipe=ખાલી;
#ifndef __U_BOOT__
	ctx->pending_redirect=ખાલી;
#endif
	ctx->child=ખાલી;
	ctx->list_head=new_pipe();
	ctx->pipe=ctx->list_head;
	ctx->w=RES_NONE;
	ctx->ઢગલો=ખાલી;
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
સંરચના reserved_combo {
	અક્ષર *literal;
	પૂર્ણાંક code;
	long flag;
};
/* Mostly a list of accepted follow-up reserved words.
 * FLAG_END means we are done with the sequence, and are ready
 * to turn the compound list into a command.
 * FLAG_START means the word must start a new compound list.
 */
સ્થિર સંરચના reserved_combo reserved_list[] = {
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
#define NRES (કદ(reserved_list)/કદ(સંરચના reserved_combo))

સ્થિર પૂર્ણાંક reserved_word(o_string *dest, સંરચના p_context *ctx)
{
	સંરચના reserved_combo *r;
	માટે (r=reserved_list;
		r<reserved_list+NRES; r++) {
		જો (strcmp(dest->data, r->literal) == 0) {
			debug_printf("found reserved word %s, code %d\n",r->literal,r->code);
			જો (r->flag & FLAG_START) {
				સંરચના p_context *new = xmalloc(કદ(સંરચના p_context));
				debug_printf("push stack\n");
				જો (ctx->w == RES_IN || ctx->w == RES_FOR) {
					syntax();
					મુક્ત(new);
					ctx->w = RES_SNTX;
					b_reset(dest);
					પાછા 1;
				}
				*new = *ctx;   /* physical copy */
				initialize_context(ctx);
				ctx->ઢગલો=new;
			} નહીંતર જો ( ctx->w == RES_NONE || ! (ctx->old_flag & (1<<r->code))) {
				syntax();
				ctx->w = RES_SNTX;
				b_reset(dest);
				પાછા 1;
			}
			ctx->w=r->code;
			ctx->old_flag = r->flag;
			જો (ctx->old_flag & FLAG_END) {
				સંરચના p_context *old;
				debug_printf("pop stack\n");
				done_pipe(ctx,PIPE_SEQ);
				old = ctx->ઢગલો;
				old->child->group = ctx->list_head;
#ifndef __U_BOOT__
				old->child->subshell = 0;
#endif
				*ctx = *old;   /* physical copy */
				મુક્ત(old);
			}
			b_reset (dest);
			પાછા 1;
		}
	}
	પાછા 0;
}

/* normal return is 0.
 * Syntax or xglob errors return 1. */
સ્થિર પૂર્ણાંક done_word(o_string *dest, સંરચના p_context *ctx)
{
	સંરચના child_prog *child=ctx->child;
#ifndef __U_BOOT__
	glob_t *glob_target;
	પૂર્ણાંક gr, flags = 0;
#નહીંતર
	અક્ષર *str, *s;
	પૂર્ણાંક argc, cnt;
#endif

	debug_printf("done_word: %s %p\n", dest->data, child);
	જો (dest->length == 0 && !dest->nonnull) {
		debug_printf("  true null, ignored\n");
		પાછા 0;
	}
#ifndef __U_BOOT__
	જો (ctx->pending_redirect) {
		glob_target = &ctx->pending_redirect->word;
	} નહીંતર {
#endif
		જો (child->group) {
			syntax();
			પાછા 1;  /* syntax error, groups and arglists don't mix */
		}
		જો (!child->argv && (ctx->type & FLAG_PARSE_SEMICOLON)) {
			debug_printf("checking %s for reserved-ness\n",dest->data);
			જો (reserved_word(dest,ctx)) પાછા ctx->w==RES_SNTX;
		}
#ifndef __U_BOOT__
		glob_target = &child->glob_result;
		જો (child->argv) flags |= GLOB_APPEND;
#નહીંતર
		માટે (cnt = 1, s = dest->data; s && *s; s++) {
			જો (*s == '\\') s++;
			cnt++;
		}
		str = malloc(cnt);
		જો (!str) પાછા 1;
		જો ( child->argv == ખાલી) {
			child->argc=0;
		}
		argc = ++child->argc;
		child->argv = realloc(child->argv, (argc+1)*કદ(*child->argv));
		જો (child->argv == ખાલી) {
			મુક્ત(str);
			પાછા 1;
		}
		child->argv_nonnull = realloc(child->argv_nonnull,
					(argc+1)*કદ(*child->argv_nonnull));
		જો (child->argv_nonnull == ખાલી) {
			મુક્ત(str);
			પાછા 1;
		}
		child->argv[argc-1]=str;
		child->argv_nonnull[argc-1] = dest->nonnull;
		child->argv[argc]=ખાલી;
		child->argv_nonnull[argc] = 0;
		માટે (s = dest->data; s && *s; s++,str++) {
			જો (*s == '\\') s++;
			*str = *s;
		}
		*str = '\0';
#endif
#ifndef __U_BOOT__
	}
	gr = xglob(dest, flags, glob_target);
	જો (gr != 0) પાછા 1;
#endif

	b_reset(dest);
#ifndef __U_BOOT__
	જો (ctx->pending_redirect) {
		ctx->pending_redirect=ખાલી;
		જો (glob_target->gl_pathc != 1) {
			error_સંદેશ("ambiguous redirect");
			પાછા 1;
		}
	} નહીંતર {
		child->argv = glob_target->gl_pathv;
	}
#endif
	જો (ctx->w == RES_FOR) {
		done_word(dest,ctx);
		done_pipe(ctx,PIPE_SEQ);
	}
	પાછા 0;
}

/* The only possible error here is out of memory, in which case
 * xmalloc exits. */
સ્થિર પૂર્ણાંક done_command(સંરચના p_context *ctx)
{
	/* The child is really already in the pipe structure, so
	 * advance the pipe counter and make a new, null child.
	 * Only real trickiness here is that the uncommitted
	 * child structure, to which ctx->child points, is not
	 * counted in pi->num_progs. */
	સંરચના pipe *pi=ctx->pipe;
	સંરચના child_prog *prog=ctx->child;

	જો (prog && prog->group == ખાલી
		 && prog->argv == ખાલી
#ifndef __U_BOOT__
		 && prog->redirects == ખાલી) {
#નહીંતર
										) {
#endif
		debug_printf("done_command: skipping null command\n");
		પાછા 0;
	} નહીંતર જો (prog) {
		pi->અંક_progs++;
		debug_printf("done_command: num_progs incremented to %d\n",pi->અંક_progs);
	} નહીંતર {
		debug_printf("done_command: initializing\n");
	}
	pi->progs = xrealloc(pi->progs, કદ(*pi->progs) * (pi->અંક_progs+1));

	prog = pi->progs + pi->અંક_progs;
#ifndef __U_BOOT__
	prog->redirects = ખાલી;
#endif
	prog->argv = ખાલી;
	prog->argv_nonnull = ખાલી;
#ifndef __U_BOOT__
	prog->is_stopped = 0;
#endif
	prog->group = ખાલી;
#ifndef __U_BOOT__
	prog->glob_result.gl_pathv = ખાલી;
	prog->family = pi;
#endif
	prog->sp = 0;
	ctx->child = prog;
	prog->type = ctx->type;

	/* but ctx->pipe and ctx->list_head remain unchanged */
	પાછા 0;
}

સ્થિર પૂર્ણાંક done_pipe(સંરચના p_context *ctx, pipe_style type)
{
	સંરચના pipe *new_p;
	done_command(ctx);  /* implicit closure of previous command */
	debug_printf("done_pipe, type %d\n", type);
	ctx->pipe->followup = type;
	ctx->pipe->r_mode = ctx->w;
	new_p=new_pipe();
	ctx->pipe->next = new_p;
	ctx->pipe = new_p;
	ctx->child = ખાલી;
	done_command(ctx);  /* set up new pipe to accept commands */
	પાછા 0;
}

#ifndef __U_BOOT__
/* peek ahead in the in_str to find out if we have a "&n" construct,
 * as in "2>&1", that represents duplicating a file descriptor.
 * returns either -2 (syntax error), -1 (no &), or the number found.
 */
સ્થિર પૂર્ણાંક redirect_dup_અંક(સંરચના in_str *આગત)
{
	પૂર્ણાંક ch, d=0, ok=0;
	ch = b_peek(આગત);
	જો (ch != '&') પાછા -1;

	b_getch(આગત);  /* get the & */
	ch=b_peek(આગત);
	જો (ch == '-') {
		b_getch(આગત);
		પાછા -3;  /* "-" represents "close me" */
	}
	જ્યારે (isdigit(ch)) {
		d = d*10+(ch-'0');
		ok=1;
		b_getch(આગત);
		ch = b_peek(આગત);
	}
	જો (ok) પાછા d;

	error_સંદેશ("ambiguous redirect");
	પાછા -2;
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
સ્થિર પૂર્ણાંક redirect_opt_અંક(o_string *o)
{
	પૂર્ણાંક અંક;

	જો (o->length==0) પાછા -1;
	માટે(અંક=0; અંક<o->length; અંક++) {
		જો (!isdigit(*(o->data+અંક))) {
			પાછા -1;
		}
	}
	/* reuse num (and save an int) */
	અંક=atoi(o->data);
	b_reset(o);
	પાછા અંક;
}

FILE *generate_stream_from_list(સંરચના pipe *head)
{
	FILE *pf;
#જો 1
	પૂર્ણાંક pid, channel[2];
	જો (pipe(channel)<0) perror_સંદેશ_and_die("pipe");
	pid=fork();
	જો (pid<0) {
		perror_સંદેશ_and_die("fork");
	} નહીંતર જો (pid==0) {
		close(channel[0]);
		જો (channel[1] != 1) {
			dup2(channel[1],1);
			close(channel[1]);
		}
#જો 0
#define SURROGATE "surrogate response"
		લિખ(1,SURROGATE,કદ(SURROGATE));
		_exit(run_list(head));
#નહીંતર
		_exit(run_list_real(head));   /* leaks memory */
#endif
	}
	debug_printf("forked child %d\n",pid);
	close(channel[1]);
	pf = fdopen(channel[0],"r");
	debug_printf("pipe on FILE *%p\n",pf);
#નહીંતર
	free_pipe_list(head,0);
	pf=popen("echo surrogate response","r");
	debug_printf("started fake pipe on FILE *%p\n",pf);
#endif
	પાછા pf;
}

/* this version hacked for testing purposes */
/* return code is exit status of the process that is run. */
સ્થિર પૂર્ણાંક process_command_subs(o_string *dest, સંરચના p_context *ctx, સંરચના in_str *આગત, પૂર્ણાંક subst_end)
{
	પૂર્ણાંક retcode;
	o_string result=NULL_O_STRING;
	સંરચના p_context inner;
	FILE *p;
	સંરચના in_str pipe_str;
	initialize_context(&inner);

	/* recursion to generate command */
	retcode = parse_stream(&result, &inner, આગત, subst_end);
	જો (retcode != 0) પાછા retcode;  /* syntax error or EOF */
	done_word(&result, &inner);
	done_pipe(&inner, PIPE_SEQ);
	b_free(&result);

	p=generate_stream_from_list(inner.list_head);
	જો (p==ખાલી) પાછા 1;
	mark_open(fileno(p));
	setup_file_in_str(&pipe_str, p);

	/* now send results of command back into original context */
	retcode = parse_stream(dest, ctx, &pipe_str, '\0');
	/* XXX In case of a syntax error, should we try to kill the child?
	 * That would be tough to do right, so just read until EOF. */
	જો (retcode == 1) {
		જ્યારે (b_getch(&pipe_str)!=EOF) { /* discard */ };
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
	પાછા retcode;
}

સ્થિર પૂર્ણાંક parse_group(o_string *dest, સંરચના p_context *ctx,
	સંરચના in_str *આગત, પૂર્ણાંક ch)
{
	પૂર્ણાંક rcode, endch=0;
	સંરચના p_context sub;
	સંરચના child_prog *child = ctx->child;
	જો (child->argv) {
		syntax();
		પાછા 1;  /* syntax error, groups and arglists don't mix */
	}
	initialize_context(&sub);
	પસંદગી(ch) {
		સ્થિતિ '(': endch=')'; child->subshell=1; તોડો;
		સ્થિતિ '{': endch='}'; તોડો;
		મૂળભૂત: syntax();   /* really logic error */
	}
	rcode=parse_stream(dest,&sub,આગત,endch);
	done_word(dest,&sub); /* finish off the final word in the subcontext */
	done_pipe(&sub, PIPE_SEQ);  /* and the final command there, too */
	child->group = sub.list_head;
	પાછા rcode;
	/* child remains "open", available for possible redirects */
}
#endif

/* basically useful version until someone wants to get fancier,
 * see the bash man page under "Parameter Expansion" */
સ્થિર અક્ષર *lookup_param(અક્ષર *src)
{
	અક્ષર *p;
	અક્ષર *sep;
	અક્ષર *default_val = ખાલી;
	પૂર્ણાંક assign = 0;
	પૂર્ણાંક expand_empty = 0;

	જો (!src)
		પાછા ખાલી;

	sep = strchr(src, ':');

	જો (sep) {
		*sep = '\0';
		જો (*(sep + 1) == '-')
			default_val = sep+2;
		જો (*(sep + 1) == '=') {
			default_val = sep+2;
			assign = 1;
		}
		જો (*(sep + 1) == '+') {
			default_val = sep+2;
			expand_empty = 1;
		}
	}

	p = env_get(src);
	જો (!p)
		p = get_local_var(src);

	જો (!p || strlen(p) == 0) {
		p = default_val;
		જો (assign) {
			અક્ષર *var = malloc(strlen(src)+strlen(default_val)+2);
			જો (var) {
				sprintf(var, "%s=%s", src, default_val);
				set_local_var(var, 0);
			}
			મુક્ત(var);
		}
	} નહીંતર જો (expand_empty) {
		p += strlen(p);
	}

	જો (sep)
		*sep = ':';

	પાછા p;
}

#ifdef __U_BOOT__
સ્થિર અક્ષર *get_dollar_var(અક્ષર ch)
{
	સ્થિર અક્ષર buf[40];

	buf[0] = '\0';
	પસંદગી (ch) {
		સ્થિતિ '?':
			sprintf(buf, "%u", (unsigned પૂર્ણાંક)last_return_code);
			તોડો;
		મૂળભૂત:
			પાછા ખાલી;
	}
	પાછા buf;
}
#endif

/* return code: 0 for OK, 1 for syntax error */
સ્થિર પૂર્ણાંક handle_dollar(o_string *dest, સંરચના p_context *ctx, સંરચના in_str *આગત)
{
#ifndef __U_BOOT__
	પૂર્ણાંક i, advance=0;
#નહીંતર
	પૂર્ણાંક advance=0;
#endif
#ifndef __U_BOOT__
	અક્ષર sep[]=" ";
#endif
	પૂર્ણાંક ch = આગત->peek(આગત);  /* first character after the $ */
	debug_printf("handle_dollar: ch=%c\n",ch);
	જો (isalpha(ch)) {
		b_addchr(dest, SPECIAL_VAR_પ્રતીક);
		ctx->child->sp++;
		જ્યારે(ch=b_peek(આગત),isalnum(ch) || ch=='_') {
			b_getch(આગત);
			b_addchr(dest,ch);
		}
		b_addchr(dest, SPECIAL_VAR_પ્રતીક);
#ifndef __U_BOOT__
	} નહીંતર જો (isdigit(ch)) {
		i = ch-'0';  /* XXX is $0 special? */
		જો (i<global_argc) {
			parse_string(dest, ctx, global_argv[i]); /* recursion */
		}
		advance = 1;
#endif
	} નહીંતર પસંદગી (ch) {
#ifndef __U_BOOT__
		સ્થિતિ '$':
			b_adduint(dest,getpid());
			advance = 1;
			તોડો;
		સ્થિતિ '!':
			જો (last_bg_pid > 0) b_adduint(dest, last_bg_pid);
			advance = 1;
			તોડો;
#endif
		સ્થિતિ '?':
#ifndef __U_BOOT__
			b_adduint(dest,last_return_code);
#નહીંતર
			ctx->child->sp++;
			b_addchr(dest, SPECIAL_VAR_પ્રતીક);
			b_addchr(dest, '$');
			b_addchr(dest, '?');
			b_addchr(dest, SPECIAL_VAR_પ્રતીક);
#endif
			advance = 1;
			તોડો;
#ifndef __U_BOOT__
		સ્થિતિ '#':
			b_adduint(dest,global_argc ? global_argc-1 : 0);
			advance = 1;
			તોડો;
#endif
		સ્થિતિ '{':
			b_addchr(dest, SPECIAL_VAR_પ્રતીક);
			ctx->child->sp++;
			b_getch(આગત);
			/* XXX maybe someone will try to escape the '}' */
			જ્યારે(ch=b_getch(આગત),ch!=EOF && ch!='}') {
				b_addchr(dest,ch);
			}
			જો (ch != '}') {
				syntax();
				પાછા 1;
			}
			b_addchr(dest, SPECIAL_VAR_પ્રતીક);
			તોડો;
#ifndef __U_BOOT__
		સ્થિતિ '(':
			b_getch(આગત);
			process_command_subs(dest, ctx, આગત, ')');
			તોડો;
		સ્થિતિ '*':
			sep[0]=ifs[0];
			માટે (i=1; i<global_argc; i++) {
				parse_string(dest, ctx, global_argv[i]);
				જો (i+1 < global_argc) parse_string(dest, ctx, sep);
			}
			તોડો;
		સ્થિતિ '@':
		સ્થિતિ '-':
		સ્થિતિ '_':
			/* still unhandled, but should be eventually */
			error_સંદેશ("unhandled syntax: $%c",ch);
			પાછા 1;
			તોડો;
#endif
		મૂળભૂત:
			b_addqchr(dest,'$',dest->quote);
	}
	/* Eat the character if the flag was set.  If the compiler
	 * is smart enough, we could substitute "b_getch(input);"
	 * for all the "advance = 1;" above, and also end up with
	 * a nice size-optimized program.  Hah!  That'll be the day.
	 */
	જો (advance) b_getch(આગત);
	પાછા 0;
}

#ifndef __U_BOOT__
પૂર્ણાંક parse_string(o_string *dest, સંરચના p_context *ctx, અચલ અક્ષર *src)
{
	સંરચના in_str foo;
	setup_string_in_str(&foo, src);
	પાછા parse_stream(dest, ctx, &foo, '\0');
}
#endif

/* return code is 0 for normal exit, 1 for syntax error */
સ્થિર પૂર્ણાંક parse_stream(o_string *dest, સંરચના p_context *ctx,
			સંરચના in_str *આગત, પૂર્ણાંક end_trigger)
{
	unsigned પૂર્ણાંક ch, m;
#ifndef __U_BOOT__
	પૂર્ણાંક redir_fd;
	redir_type redir_style;
#endif
	પૂર્ણાંક next;

	/* Only double-quote state is handled in the state variable dest->quote.
	 * A single-quote triggers a bypass of the main loop until its mate is
	 * found.  When recursing, quote state is passed in via dest->quote. */

	debug_printf("parse_stream, end_trigger=%d\n",end_trigger);
	જ્યારે ((ch=b_getch(આગત))!=EOF) {
		m = map[ch];
#ifdef __U_BOOT__
		જો (આગત->__promptme == 0) પાછા 1;
#endif
		next = (ch == '\n') ? 0 : b_peek(આગત);

		debug_printf("parse_stream: ch=%c (%d) m=%d quote=%d - %c\n",
			ch >= ' ' ? ch : '.', ch, m,
			dest->quote, ctx->ઢગલો == ખાલી ? '*' : '.');

		જો (m==0 || ((m==1 || m==2) && dest->quote)) {
			b_addqchr(dest, ch, dest->quote);
		} નહીંતર {
			જો (m==2) {  /* unquoted IFS */
				જો (done_word(dest, ctx)) {
					પાછા 1;
				}
				/* If we aren't performing a substitution, treat a newline as a
				 * command separator.  */
				જો (end_trigger != '\0' && ch=='\n')
					done_pipe(ctx,PIPE_SEQ);
			}
			જો (ch == end_trigger && !dest->quote && ctx->w==RES_NONE) {
				debug_printf("leaving parse_stream (triggered)\n");
				પાછા 0;
			}
#જો 0
			જો (ch=='\n') {
				/* Yahoo!  Time to run with it! */
				done_pipe(ctx,PIPE_SEQ);
				run_list(ctx->list_head);
				initialize_context(ctx);
			}
#endif
			જો (m!=2) પસંદગી (ch) {
		સ્થિતિ '#':
			જો (dest->length == 0 && !dest->quote) {
				જ્યારે(ch=b_peek(આગત),ch!=EOF && ch!='\n') { b_getch(આગત); }
			} નહીંતર {
				b_addqchr(dest, ch, dest->quote);
			}
			તોડો;
		સ્થિતિ '\\':
			જો (next == EOF) {
				syntax();
				પાછા 1;
			}
			b_addqchr(dest, '\\', dest->quote);
			b_addqchr(dest, b_getch(આગત), dest->quote);
			તોડો;
		સ્થિતિ '$':
			જો (handle_dollar(dest, ctx, આગત)!=0) પાછા 1;
			તોડો;
		સ્થિતિ '\'':
			dest->nonnull = 1;
			જ્યારે(ch=b_getch(આગત),ch!=EOF && ch!='\'') {
#ifdef __U_BOOT__
				જો(આગત->__promptme == 0) પાછા 1;
#endif
				b_addchr(dest,ch);
			}
			જો (ch==EOF) {
				syntax();
				પાછા 1;
			}
			તોડો;
		સ્થિતિ '"':
			dest->nonnull = 1;
			dest->quote = !dest->quote;
			તોડો;
#ifndef __U_BOOT__
		સ્થિતિ '`':
			process_command_subs(dest, ctx, આગત, '`');
			તોડો;
		સ્થિતિ '>':
			redir_fd = redirect_opt_અંક(dest);
			done_word(dest, ctx);
			redir_style=REDIRECT_OVERWRITE;
			જો (next == '>') {
				redir_style=REDIRECT_APPEND;
				b_getch(આગત);
			} નહીંતર જો (next == '(') {
				syntax();   /* until we support >(list) Process Substitution */
				પાછા 1;
			}
			setup_redirect(ctx, redir_fd, redir_style, આગત);
			તોડો;
		સ્થિતિ '<':
			redir_fd = redirect_opt_અંક(dest);
			done_word(dest, ctx);
			redir_style=REDIRECT_આગત;
			જો (next == '<') {
				redir_style=REDIRECT_HEREIS;
				b_getch(આગત);
			} નહીંતર જો (next == '>') {
				redir_style=REDIRECT_IO;
				b_getch(આગત);
			} નહીંતર જો (next == '(') {
				syntax();   /* until we support <(list) Process Substitution */
				પાછા 1;
			}
			setup_redirect(ctx, redir_fd, redir_style, આગત);
			તોડો;
#endif
		સ્થિતિ ';':
			done_word(dest, ctx);
			done_pipe(ctx,PIPE_SEQ);
			તોડો;
		સ્થિતિ '&':
			done_word(dest, ctx);
			જો (next=='&') {
				b_getch(આગત);
				done_pipe(ctx,PIPE_AND);
			} નહીંતર {
#ifndef __U_BOOT__
				done_pipe(ctx,PIPE_BG);
#નહીંતર
				syntax_err();
				પાછા 1;
#endif
			}
			તોડો;
		સ્થિતિ '|':
			done_word(dest, ctx);
			જો (next=='|') {
				b_getch(આગત);
				done_pipe(ctx,PIPE_OR);
			} નહીંતર {
				/* we could pick up a file descriptor choice here
				 * with redirect_opt_num(), but bash doesn't do it.
				 * "echo foo 2| cat" yields "foo 2". */
#ifndef __U_BOOT__
				done_command(ctx);
#નહીંતર
				syntax_err();
				પાછા 1;
#endif
			}
			તોડો;
#ifndef __U_BOOT__
		સ્થિતિ '(':
		સ્થિતિ '{':
			જો (parse_group(dest, ctx, આગત, ch)!=0) પાછા 1;
			તોડો;
		સ્થિતિ ')':
		સ્થિતિ '}':
			syntax();   /* Proper use of this character caught by end_trigger */
			પાછા 1;
			તોડો;
#endif
		સ્થિતિ SUBSTED_VAR_પ્રતીક:
			dest->nonnull = 1;
			જ્યારે (ch = b_getch(આગત), ch != EOF &&
			    ch != SUBSTED_VAR_પ્રતીક) {
				debug_printf("subst, pass=%d\n", ch);
				જો (આગત->__promptme == 0)
					પાછા 1;
				b_addchr(dest, ch);
			}
			debug_printf("subst, term=%d\n", ch);
			જો (ch == EOF) {
				syntax();
				પાછા 1;
			}
			તોડો;
		મૂળભૂત:
			syntax();   /* this is really an internal logic error */
			પાછા 1;
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
	જો (end_trigger != '\0') પાછા -1;
	પાછા 0;
}

સ્થિર શૂન્ય mapset(અચલ unsigned અક્ષર *set, પૂર્ણાંક code)
{
	અચલ unsigned અક્ષર *s;
	માટે (s=set; *s; s++) map[*s] = code;
}

સ્થિર શૂન્ય update_ifs_map(શૂન્ય)
{
	/* char *ifs and char map[256] are both globals. */
	ifs = (uchar *)env_get("IFS");
	જો (ifs == ખાલી) ifs=(uchar *)" \t\n";
	/* Precompute a list of 'flow through' behavior so it can be treated
	 * quickly up front.  Computation is necessary because of IFS.
	 * Special case handling of IFS == " \t\n" is not implemented.
	 * The map[] array only really needs two bits each, and on most machines
	 * that would be faster because of the reduced L1 cache footprint.
	 */
	memset(map,0,કદ(map)); /* most characters flow through always */
#ifndef __U_BOOT__
	mapset((uchar *)"\\$'\"`", 3);      /* never flow through */
	mapset((uchar *)"<>;&|(){}#", 1);   /* flow through if quoted */
#નહીંતર
	{
		uchar subst[2] = {SUBSTED_VAR_પ્રતીક, 0};
		mapset(subst, 3);       /* never flow through */
	}
	mapset((uchar *)"\\$'\"", 3);       /* never flow through */
	mapset((uchar *)";&|#", 1);         /* flow through if quoted */
#endif
	mapset(ifs, 2);            /* also flow through if quoted */
}

/* most recursion does not come through here, the exeception is
 * from builtin_source() */
સ્થિર પૂર્ણાંક parse_stream_outer(સંરચના in_str *inp, પૂર્ણાંક flag)
{

	સંરચના p_context ctx;
	o_string temp=NULL_O_STRING;
	પૂર્ણાંક rcode;
#ifdef __U_BOOT__
	પૂર્ણાંક code = 1;
#endif
	કરો {
		ctx.type = flag;
		initialize_context(&ctx);
		update_ifs_map();
		જો (!(flag & FLAG_PARSE_SEMICOLON) || (flag & FLAG_REPARSING)) mapset((uchar *)";$&|", 0);
		inp->promptmode=1;
		rcode = parse_stream(&temp, &ctx, inp,
				     flag & FLAG_CONT_ON_NEWLINE ? -1 : '\n');
#ifdef __U_BOOT__
		જો (rcode == 1) flag_repeat = 0;
#endif
		જો (rcode != 1 && ctx.old_flag != 0) {
			syntax();
#ifdef __U_BOOT__
			flag_repeat = 0;
#endif
		}
		જો (rcode != 1 && ctx.old_flag == 0) {
			done_word(&temp, &ctx);
			done_pipe(&ctx,PIPE_SEQ);
#ifndef __U_BOOT__
			run_list(ctx.list_head);
#નહીંતર
			code = run_list(ctx.list_head);
			જો (code == -2) {	/* exit */
				b_free(&temp);
				code = 0;
				/* XXX hackish way to not allow exit from main loop */
				જો (inp->peek == file_peek) {
					printf("exit not allowed from main input shell.\n");
					ચાલુ;
				}
				/*
				 * DANGER
				 * Return code -2 is special in this context,
				 * it indicates exit from inner pipe instead
				 * of return code itself, the return code is
				 * stored in 'last_return_code' variable!
				 * DANGER
				 */
				પાછા -2;
			}
			જો (code == -1)
			    flag_repeat = 0;
#endif
		} નહીંતર {
			જો (ctx.old_flag != 0) {
				મુક્ત(ctx.ઢગલો);
				b_reset(&temp);
			}
#ifdef __U_BOOT__
			જો (inp->__promptme == 0) printf("<INTERRUPT>\n");
			inp->__promptme = 1;
#endif
			temp.nonnull = 0;
			temp.quote = 0;
			inp->p = ખાલી;
			free_pipe_list(ctx.list_head,0);
		}
		b_free(&temp);
	/* loop on syntax errors, return on EOF */
	} જ્યારે (rcode != -1 && !(flag & FLAG_EXIT_FROM_LOOP) &&
		(inp->peek != static_peek || b_peek(inp)));
#ifndef __U_BOOT__
	પાછા 0;
#નહીંતર
	પાછા (code != 0) ? 1 : 0;
#endif /* __U_BOOT__ */
}

#ifndef __U_BOOT__
સ્થિર પૂર્ણાંક parse_string_outer(અચલ અક્ષર *s, પૂર્ણાંક flag)
#નહીંતર
પૂર્ણાંક parse_string_outer(અચલ અક્ષર *s, પૂર્ણાંક flag)
#endif	/* __U_BOOT__ */
{
	સંરચના in_str આગત;
	પૂર્ણાંક rcode;
#ifdef __U_BOOT__
	અક્ષર *p = ખાલી;
	જો (!s)
		પાછા 1;
	જો (!*s)
		પાછા 0;
	જો (!(p = strchr(s, '\n')) || *++p) {
		p = xmalloc(strlen(s) + 2);
		strcpy(p, s);
		strcat(p, "\n");
		setup_string_in_str(&આગત, p);
		rcode = parse_stream_outer(&આગત, flag);
		મુક્ત(p);
		પાછા rcode == -2 ? last_return_code : rcode;
	} નહીંતર {
#endif
	setup_string_in_str(&આગત, s);
	rcode = parse_stream_outer(&આગત, flag);
	પાછા rcode == -2 ? last_return_code : rcode;
#ifdef __U_BOOT__
	}
#endif
}

#ifndef __U_BOOT__
સ્થિર પૂર્ણાંક parse_file_outer(FILE *f)
#નહીંતર
પૂર્ણાંક parse_file_outer(શૂન્ય)
#endif
{
	પૂર્ણાંક rcode;
	સંરચના in_str આગત;
#ifndef __U_BOOT__
	setup_file_in_str(&આગત, f);
#નહીંતર
	setup_file_in_str(&આગત);
#endif
	rcode = parse_stream_outer(&આગત, FLAG_PARSE_SEMICOLON);
	પાછા rcode == -2 ? last_return_code : rcode;
}

#ifdef __U_BOOT__
પૂર્ણાંક u_boot_hush_start(શૂન્ય)
{
	જો (ટોચ_vars == ખાલી) {
		ટોચ_vars = malloc(કદ(સંરચના variables));
		ટોચ_vars->name = "HUSH_VERSION";
		ટોચ_vars->કિંમત = "0.01";
		ટોચ_vars->next = ખાલી;
		ટોચ_vars->flg_export = 0;
		ટોચ_vars->flg_read_only = 1;
	}
	પાછા 0;
}

સ્થિર શૂન્ય *xmalloc(size_t size)
{
	શૂન્ય *p = ખાલી;

	જો (!(p = malloc(size))) {
	    printf("ERROR : xmalloc failed\n");
	    માટે(;;);
	}
	પાછા p;
}

સ્થિર શૂન્ય *xrealloc(શૂન્ય *ptr, size_t size)
{
	શૂન્ય *p = ખાલી;

	જો (!(p = realloc(ptr, size))) {
	    printf("ERROR : xrealloc failed\n");
	    માટે(;;);
	}
	પાછા p;
}
#endif /* __U_BOOT__ */

#ifndef __U_BOOT__
/* Make sure we have a controlling tty.  If we get started under a job
 * aware app (like bash for example), make sure we are now in charge so
 * we don't fight over who gets the foreground */
સ્થિર શૂન્ય setup_job_control(શૂન્ય)
{
	સ્થિર pid_t shell_pgrp;
	/* Loop until we are in the foreground.  */
	જ્યારે (tcgetpgrp (shell_terminal) != (shell_pgrp = getpgrp ()))
		kill (- shell_pgrp, SIGTTIN);

	/* Ignore interactive and job-control signals.  */
	સંકેત(SIGINT, SIG_IGN);
	સંકેત(SIGQUIT, SIG_IGN);
	સંકેત(SIGTERM, SIG_IGN);
	સંકેત(SIGTSTP, SIG_IGN);
	સંકેત(SIGTTIN, SIG_IGN);
	સંકેત(SIGTTOU, SIG_IGN);
	સંકેત(SIGCHLD, SIG_IGN);

	/* Put ourselves in our own process group.  */
	setsid();
	shell_pgrp = getpid ();
	setpgid (shell_pgrp, shell_pgrp);

	/* Grab control of the terminal.  */
	tcsetpgrp(shell_terminal, shell_pgrp);
}

પૂર્ણાંક hush_main(પૂર્ણાંક argc, અક્ષર * અચલ *argv)
{
	પૂર્ણાંક opt;
	FILE *આગત;
	અક્ષર **e = environ;

	/* XXX what should these be while sourcing /etc/profile? */
	global_argc = argc;
	global_argv = argv;

	/* (re?) initialize globals.  Sometimes hush_main() ends up calling
	 * hush_main(), therefore we cannot rely on the BSS to zero out this
	 * stuff.  Reset these to 0 every time. */
	ifs = ખાલી;
	/* map[] is taken care of with call to update_ifs_map() */
	fake_mode = 0;
	interactive = 0;
	close_me_head = ખાલી;
	last_bg_pid = 0;
	job_list = ખાલી;
	last_jobid = 0;

	/* Initialize some more globals to non-zero values */
	set_cwd();
#ifdef CONFIG_FEATURE_COMMAND_EDITING
	cmdedit_set_initial_prompt();
#નહીંતર
	PS1 = ખાલી;
#endif
	PS2 = "> ";

	/* initialize our shell local variables with the values
	 * currently living in the environment */
	જો (e) {
		માટે (; *e; e++)
			set_local_var(*e, 2);   /* without call putenv() */
	}

	last_return_code=EXIT_SUCCESS;

	જો (argv[0] && argv[0][0] == '-') {
		debug_printf("\nsourcing /etc/profile\n");
		જો ((આગત = fopen("/etc/profile", "r")) != ખાલી) {
			mark_open(fileno(આગત));
			parse_file_outer(આગત);
			mark_closed(fileno(આગત));
			fclose(આગત);
		}
	}
	આગત=stdin;

	જ્યારે ((opt = getopt(argc, argv, "c:xif")) > 0) {
		પસંદગી (opt) {
			સ્થિતિ 'c':
				{
					global_argv = argv+optind;
					global_argc = argc-optind;
					opt = parse_string_outer(optarg, FLAG_PARSE_SEMICOLON);
					જાઓ final_return;
				}
				તોડો;
			સ્થિતિ 'i':
				interactive++;
				તોડો;
			સ્થિતિ 'f':
				fake_mode++;
				તોડો;
			મૂળભૂત:
#ifndef BB_VER
				fprintf(stderr, "Usage: sh [FILE]...\n"
						"   or: sh -c command [args]...\n\n");
				exit(EXIT_FAILURE);
#નહીંતર
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
	જો (argv[optind]==ખાલી && આગત==stdin &&
			isatty(fileno(stdin)) && isatty(fileno(stdout))) {
		interactive++;
	}

	debug_printf("\ninteractive=%d\n", interactive);
	જો (interactive) {
		/* Looks like they want an interactive shell */
#ifndef CONFIG_FEATURE_SH_EXTRA_QUIET
		printf( "\n\n" BB_BANNER " hush - the humble shell v0.01 (testing)\n");
		printf( "Enter 'help' for a list of built-in commands.\n\n");
#endif
		setup_job_control();
	}

	જો (argv[optind]==ખાલી) {
		opt=parse_file_outer(stdin);
		જાઓ final_return;
	}

	debug_printf("\nrunning script '%s'\n", argv[optind]);
	global_argv = argv+optind;
	global_argc = argc-optind;
	આગત = xfopen(argv[optind], "r");
	opt = parse_file_outer(આગત);

#ifdef CONFIG_FEATURE_CLEAN_UP
	fclose(આગત);
	જો (cwd && cwd != unknown)
		મુક્ત((અક્ષર*)cwd);
	{
		સંરચના variables *cur, *tmp;
		માટે(cur = ટોચ_vars; cur; cur = tmp) {
			tmp = cur->next;
			જો (!cur->flg_read_only) {
				મુક્ત(cur->name);
				મુક્ત(cur->કિંમત);
				મુક્ત(cur);
			}
		}
	}
#endif

final_return:
	પાછા(opt?opt:last_return_code);
}
#endif

સ્થિર અક્ષર *ઉમેરો_var_કિંમત(અક્ષર *inp)
{
	પાછા ઉમેરો_var_કિંમત_sub(inp, 0);
}

સ્થિર અક્ષર *ઉમેરો_var_કિંમત_sub(અક્ષર *inp, પૂર્ણાંક tag_subst)
{
	પૂર્ણાંક res_str_len = 0;
	પૂર્ણાંક len;
	પૂર્ણાંક done = 0;
	અક્ષર *p, *p1, *res_str = ખાલી;

	જ્યારે ((p = strchr(inp, SPECIAL_VAR_પ્રતીક))) {
		/* check the beginning of the string for normal characters */
		જો (p != inp) {
			/* copy any characters to the result string */
			len = p - inp;
			res_str = xrealloc(res_str, (res_str_len + len));
			strncpy((res_str + res_str_len), inp, len);
			res_str_len += len;
		}
		inp = ++p;
		/* find the ending marker */
		p = strchr(inp, SPECIAL_VAR_પ્રતીક);
		*p = '\0';
		/* look up the value to substitute */
		જો ((p1 = lookup_param(inp))) {
			જો (tag_subst)
				len = res_str_len + strlen(p1) + 2;
			નહીંતર
				len = res_str_len + strlen(p1);
			res_str = xrealloc(res_str, (1 + len));
			જો (tag_subst) {
				/*
				 * copy the variable value to the result
				 * string
				 */
				strcpy((res_str + res_str_len + 1), p1);

				/*
				 * mark the replaced text to be accepted as
				 * is
				 */
				res_str[res_str_len] = SUBSTED_VAR_પ્રતીક;
				res_str[res_str_len + 1 + strlen(p1)] =
					SUBSTED_VAR_પ્રતીક;
			} નહીંતર
				/*
				 * copy the variable value to the result
				 * string
				 */
				strcpy((res_str + res_str_len), p1);

			res_str_len = len;
		}
		*p = SPECIAL_VAR_પ્રતીક;
		inp = ++p;
		done = 1;
	}
	જો (done) {
		res_str = xrealloc(res_str, (1 + res_str_len + strlen(inp)));
		strcpy((res_str + res_str_len), inp);
		જ્યારે ((p = strchr(res_str, '\n'))) {
			*p = ' ';
		}
	}
	પાછા (res_str == ખાલી) ? inp : res_str;
}

સ્થિર અક્ષર **make_list_in(અક્ષર **inp, અક્ષર *name)
{
	પૂર્ણાંક len, i;
	પૂર્ણાંક name_len = strlen(name);
	પૂર્ણાંક n = 0;
	અક્ષર **list;
	અક્ષર *p1, *p2, *p3;

	/* create list of variable values */
	list = xmalloc(કદ(*list));
	માટે (i = 0; inp[i]; i++) {
		p3 = ઉમેરો_var_કિંમત(inp[i]);
		p1 = p3;
		જ્યારે (*p1) {
			જો (*p1 == ' ') {
				p1++;
				ચાલુ;
			}
			જો ((p2 = strchr(p1, ' '))) {
				len = p2 - p1;
			} નહીંતર {
				len = strlen(p1);
				p2 = p1 + len;
			}
			/* we use n + 2 in realloc for list,because we add
			 * new element and then we will add NULL element */
			list = xrealloc(list, કદ(*list) * (n + 2));
			list[n] = xmalloc(2 + name_len + len);
			strcpy(list[n], name);
			strcat(list[n], "=");
			strncat(list[n], p1, len);
			list[n++][name_len + len + 1] = '\0';
			p1 = p2;
		}
		જો (p3 != inp[i]) મુક્ત(p3);
	}
	list[n] = ખાલી;
	પાછા list;
}

/*
 * Make new string for parser
 * inp     - array of argument strings to flatten
 * nonnull - indicates argument was quoted when originally parsed
 */
સ્થિર અક્ષર *make_string(અક્ષર **inp, પૂર્ણાંક *nonnull)
{
	અક્ષર *p;
	અક્ષર *str = ખાલી;
	પૂર્ણાંક n;
	પૂર્ણાંક len = 2;
	અક્ષર *noeval_str;
	પૂર્ણાંક noeval = 0;

	noeval_str = get_local_var("HUSH_NO_EVAL");
	જો (noeval_str != ખાલી && *noeval_str != '0' && *noeval_str != '\0')
		noeval = 1;
	માટે (n = 0; inp[n]; n++) {
		p = ઉમેરો_var_કિંમત_sub(inp[n], noeval);
		અક્ષર *new_str = xrealloc(str, (len + strlen(p) + (2 * nonnull[n])));
		જો (!new_str) {
			મુક્ત(str);
			જો (p != inp[n]) મુક્ત(p);
			પાછા ખાલી;
		}
		str = new_str;
		જો (n) {
			strcat(str, " ");
		} નહીંતર {
			*str = '\0';
		}
		જો (nonnull[n])
			strcat(str, "'");
		strcat(str, p);
		જો (nonnull[n])
			strcat(str, "'");
		len = strlen(str) + 3;
		જો (p != inp[n]) મુક્ત(p);
	}
	len = strlen(str);
	*(str + len) = '\n';
	*(str + len + 1) = '\0';
	પાછા str;
}

#ifdef __U_BOOT__
સ્થિર પૂર્ણાંક do_showvar(સંરચના cmd_tbl *cmdtp, પૂર્ણાંક flag, પૂર્ણાંક argc,
		      અક્ષર *અચલ argv[])
{
	પૂર્ણાંક i, k;
	પૂર્ણાંક rcode = 0;
	સંરચના variables *cur;

	જો (argc == 1) {		/* Print all env variables	*/
		માટે (cur = ટોચ_vars; cur; cur = cur->next) {
			printf ("%s=%s\n", cur->name, cur->કિંમત);
			જો (ctrlc ()) {
				puts ("\n ** Abort\n");
				પાછા 1;
			}
		}
		પાછા 0;
	}
	માટે (i = 1; i < argc; ++i) {	/* print single env variables	*/
		અક્ષર *name = argv[i];

		k = -1;
		માટે (cur = ટોચ_vars; cur; cur = cur->next) {
			જો(strcmp (cur->name, name) == 0) {
				k = 0;
				printf ("%s=%s\n", cur->name, cur->કિંમત);
			}
			જો (ctrlc ()) {
				puts ("\n ** Abort\n");
				પાછા 1;
			}
		}
		જો (k < 0) {
			printf ("## Error: \"%s\" not defined\n", name);
			rcode ++;
		}
	}
	પાછા rcode;
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
