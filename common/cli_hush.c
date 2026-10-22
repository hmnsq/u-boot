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
#অন্তর্ভাৱ <env.h>
#অন্তর্ভাৱ <malloc.h>         /* malloc, free, realloc*/
#অন্তর্ভাৱ <linux/ctype.h>    /* isalpha, isdigit */
#অন্তর্ভাৱ <console.h>
#অন্তর্ভাৱ <bootretry.h>
#অন্তর্ভাৱ <cli.h>
#অন্তর্ভাৱ <cli_hush.h>
#অন্তর্ভাৱ <command.h>        /* find_cmd */
#অন্তর্ভাৱ <vsprintf.h>
#endif
#ifndef __U_BOOT__
#অন্তর্ভাৱ <ctype.h>     /* isalpha, isdigit */
#অন্তর্ভাৱ <unistd.h>    /* getpid */
#অন্তর্ভাৱ <stdlib.h>    /* getenv, atoi */
#অন্তর্ভাৱ <string.h>    /* strchr */
#অন্তর্ভাৱ <stdio.h>     /* popen etc. */
#অন্তর্ভাৱ <glob.h>      /* glob, of course */
#অন্তর্ভাৱ <stdarg.h>    /* va_list */
#অন্তর্ভাৱ <errno.h>
#অন্তর্ভাৱ <fcntl.h>
#অন্তর্ভাৱ <getopt.h>    /* should be pretty obvious */

#অন্তর্ভাৱ <sys/stat.h>  /* ulimit */
#অন্তর্ভাৱ <sys/types.h>
#অন্তর্ভাৱ <sys/wait.h>
#অন্তর্ভাৱ <signal.h>

/* #include <dmalloc.h> */

#যদি 1
#অন্তর্ভাৱ "busybox.h"
#অন্তর্ভাৱ "cmdedit.h"
#অন্যথা
#define applet_name "hush"
#অন্তর্ভাৱ "standalone.h"
#define hush_main main
#define BB_BANNER
#endif
#endif
#define SPECIAL_VAR_প্ৰতীক 03
#define SUBSTED_VAR_প্ৰতীক 04
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
#define error_বাৰ্তা printf
#অন্যথা
প্রকারনির্দেশ গণনা {
	REDIRECT_প্ৰবেশ     = 1,
	REDIRECT_OVERWRITE = 2,
	REDIRECT_APPEND    = 3,
	REDIRECT_HEREIS    = 4,
	REDIRECT_IO        = 5
} redir_type;

/* The descrip member of this structure is only used to make debugging
 * output pretty */
সংরচনা {পূর্ণাঙ্ক mode; পূর্ণাঙ্ক default_fd; ৱর্ণ *descrip;} redir_table[] = {
	{ 0,                         0, "()" },
	{ O_RDONLY,                  0, "<"  },
	{ O_CREAT|O_TRUNC|O_WRONLY,  1, ">"  },
	{ O_CREAT|O_APPEND|O_WRONLY, 1, ">>" },
	{ O_RDONLY,                 -1, "<<" },
	{ O_RDWR,                    1, "<>" }
};
#endif

প্রকারনির্দেশ গণনা {
	PIPE_SEQ = 1,
	PIPE_AND = 2,
	PIPE_OR  = 3,
	PIPE_BG  = 4,
} pipe_style;

/* might eventually control execution */
প্রকারনির্দেশ গণনা {
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
সংরচনা p_context {
	সংরচনা child_prog *child;
	সংরচনা pipe *list_head;
	সংরচনা pipe *pipe;
#ifndef __U_BOOT__
	সংরচনা redir_struct *pending_redirect;
#endif
	reserved_style w;
	পূর্ণাঙ্ক old_flag;				/* for figuring out valid reserved words */
	সংরচনা p_context *স্তূপ;
	পূর্ণাঙ্ক type;			/* define type of parser : ";$" common or special symbol */
	/* How about quoting status? */
};

#ifndef __U_BOOT__
সংরচনা redir_struct {
	redir_type type;			/* type of redirection */
	পূর্ণাঙ্ক fd;						/* file descriptor being redirected */
	পূর্ণাঙ্ক dup;					/* -1, or file descriptor being duplicated */
	সংরচনা redir_struct *next;	/* pointer to the next redirect in the list */
	glob_t word;				/* *word.gl_pathv is the filename */
};
#endif

সংরচনা child_prog {
#ifndef __U_BOOT__
	pid_t pid;					/* 0 if exited */
#endif
	ৱর্ণ **argv;				/* program name and arguments */
	/* was quoted when parsed; copy of struct o_string.nonnull field */
	পূর্ণাঙ্ক *argv_nonnull;
#ifdef __U_BOOT__
	পূর্ণাঙ্ক    argc;                            /* number of program arguments */
#endif
	সংরচনা pipe *group;			/* if non-NULL, first in group or subshell */
#ifndef __U_BOOT__
	পূর্ণাঙ্ক subshell;				/* flag, non-zero if group must be forked */
	সংরচনা redir_struct *redirects;	/* I/O redirections */
	glob_t glob_result;			/* result of parameter globbing */
	পূর্ণাঙ্ক is_stopped;				/* is the program currently running? */
	সংরচনা pipe *family;		/* pointer back to the child's parent pipe */
#endif
	পূর্ণাঙ্ক sp;				/* number of SPECIAL_VAR_SYMBOL */
	পূর্ণাঙ্ক type;
};

সংরচনা pipe {
#ifndef __U_BOOT__
	পূর্ণাঙ্ক jobid;					/* job number */
#endif
	পূর্ণাঙ্ক অঙ্ক_progs;				/* total number of programs in job */
#ifndef __U_BOOT__
	পূর্ণাঙ্ক running_progs;			/* number of programs running */
	ৱর্ণ *text;					/* name of job */
	ৱর্ণ *cmdbuf;				/* buffer various argv's point into */
	pid_t pgrp;					/* process group ID for the job */
#endif
	সংরচনা child_prog *progs;	/* array of commands in pipe */
	সংরচনা pipe *next;			/* to track background commands */
#ifndef __U_BOOT__
	পূর্ণাঙ্ক stopped_progs;			/* number of programs alive, but stopped */
	পূর্ণাঙ্ক job_context;			/* bitmask defining current context */
#endif
	pipe_style followup;		/* PIPE_BG, PIPE_SEQ, PIPE_OR, PIPE_AND */
	reserved_style r_mode;		/* supports if, for, while, until */
};

#ifndef __U_BOOT__
সংরচনা close_me {
	পূর্ণাঙ্ক fd;
	সংরচনা close_me *next;
};
#endif

সংরচনা variables {
	ৱর্ণ *name;
	ৱর্ণ *মান;
	পূর্ণাঙ্ক flg_export;
	পূর্ণাঙ্ক flg_read_only;
	সংরচনা variables *next;
};

/* globals, connect us to the outside world
 * the first three support $?, $#, and $1 */
#ifndef __U_BOOT__
ৱর্ণ **global_argv;
unsigned পূর্ণাঙ্ক global_argc;
#endif
স্থির unsigned পূর্ণাঙ্ক last_return_code;
#ifndef __U_BOOT__
বাহ্য ৱর্ণ **environ; /* This is in <unistd.h>, but protected with __USE_GNU */
#endif

/* "globals" within this file */
স্থির uchar *ifs;
স্থির ৱর্ণ map[256];
#ifndef __U_BOOT__
স্থির পূর্ণাঙ্ক fake_mode;
স্থির পূর্ণাঙ্ক interactive;
স্থির সংরচনা close_me *close_me_head;
স্থির ধ্রুৱ ৱর্ণ *cwd;
স্থির সংরচনা pipe *job_list;
স্থির unsigned পূর্ণাঙ্ক last_bg_pid;
স্থির unsigned পূর্ণাঙ্ক last_jobid;
স্থির unsigned পূর্ণাঙ্ক shell_terminal;
স্থির ৱর্ণ *PS1;
স্থির ৱর্ণ *PS2;
সংরচনা variables shell_ver = { "HUSH_VERSION", "0.01", 1, 1, 0 };
সংরচনা variables *শীৰ্ষ_vars = &shell_ver;
#অন্যথা
স্থির পূর্ণাঙ্ক flag_repeat = 0;
স্থির পূর্ণাঙ্ক do_repeat = 0;
স্থির সংরচনা variables *শীৰ্ষ_vars = রিক্ত ;
#endif /*__U_BOOT__ */

#define B_CHUNK (100)
#define B_NOSPAC 1

প্রকারনির্দেশ সংরচনা {
	ৱর্ণ *data;
	পূর্ণাঙ্ক length;
	পূর্ণাঙ্ক maxlen;
	পূর্ণাঙ্ক quote;
	পূর্ণাঙ্ক nonnull;
} o_string;
#define NULL_O_STRING {রিক্ত,0,0,0,0}
/* used for initialization:
	o_string foo = NULL_O_STRING; */

/* I can almost use ordinary FILE *.  Is open_memstream() universally
 * available?  Where is it documented? */
সংরচনা in_str {
	ধ্রুৱ unsigned ৱর্ণ *p;
#ifndef __U_BOOT__
	ৱর্ণ peek_buf[2];
#endif
	পূর্ণাঙ্ক __promptme;
	পূর্ণাঙ্ক promptmode;
#ifndef __U_BOOT__
	FILE *file;
#endif
	পূর্ণাঙ্ক (*get) (সংরচনা in_str *);
	পূর্ণাঙ্ক (*peek) (সংরচনা in_str *);
};
#define b_getch(প্ৰবেশ) ((প্ৰবেশ)->get(প্ৰবেশ))
#define b_peek(প্ৰবেশ) ((প্ৰবেশ)->peek(প্ৰবেশ))

#ifndef __U_BOOT__
#define JOB_STATUS_FORMAT "[%d] %-22s %.40s\n"

সংরচনা built_in_command {
	ৱর্ণ *cmd;					/* name */
	ৱর্ণ *descr;				/* description */
	পূর্ণাঙ্ক (*function) (সংরচনা child_prog *);	/* function ptr */
};
#endif

/* define DEBUG_SHELL for debugging output (obviously ;-)) */
#যদি 0
#define DEBUG_SHELL
#endif

/* This should be in utility.c */
#ifdef DEBUG_SHELL
#ifndef __U_BOOT__
স্থির শূন্য debug_printf(ধ্রুৱ ৱর্ণ *format, ...)
{
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
}
#অন্যথা
#define debug_printf(fmt,args...)	printf (fmt ,##args)
#endif
#অন্যথা
স্থির অন্তঃস্থ শূন্য debug_printf(ধ্রুৱ ৱর্ণ *format, ...) { }
#endif
#define final_printf debug_printf

#ifdef __U_BOOT__
স্থির শূন্য syntax_err(শূন্য) {
	 printf("syntax error\n");
}
#অন্যথা
স্থির শূন্য __syntax(ৱর্ণ *file, পূর্ণাঙ্ক সাৰি) {
	error_বাৰ্তা("syntax error %s:%d", file, সাৰি);
}
#define syntax() __syntax(__FILE__, __LINE__)
#endif

#ifdef __U_BOOT__
স্থির শূন্য *xmalloc(size_t size);
স্থির শূন্য *xrealloc(শূন্য *ptr, size_t size);
#অন্যথা
/* Index of subroutines: */
/*   function prototypes for builtins */
স্থির পূর্ণাঙ্ক builtin_cd(সংরচনা child_prog *child);
স্থির পূর্ণাঙ্ক builtin_env(সংরচনা child_prog *child);
স্থির পূর্ণাঙ্ক builtin_eval(সংরচনা child_prog *child);
স্থির পূর্ণাঙ্ক builtin_exec(সংরচনা child_prog *child);
স্থির পূর্ণাঙ্ক builtin_exit(সংরচনা child_prog *child);
স্থির পূর্ণাঙ্ক builtin_export(সংরচনা child_prog *child);
স্থির পূর্ণাঙ্ক builtin_fg_bg(সংরচনা child_prog *child);
স্থির পূর্ণাঙ্ক builtin_help(সংরচনা child_prog *child);
স্থির পূর্ণাঙ্ক builtin_jobs(সংরচনা child_prog *child);
স্থির পূর্ণাঙ্ক builtin_pwd(সংরচনা child_prog *child);
স্থির পূর্ণাঙ্ক builtin_read(সংরচনা child_prog *child);
স্থির পূর্ণাঙ্ক builtin_set(সংরচনা child_prog *child);
স্থির পূর্ণাঙ্ক builtin_shift(সংরচনা child_prog *child);
স্থির পূর্ণাঙ্ক builtin_source(সংরচনা child_prog *child);
স্থির পূর্ণাঙ্ক builtin_umask(সংরচনা child_prog *child);
স্থির পূর্ণাঙ্ক builtin_unset(সংরচনা child_prog *child);
স্থির পূর্ণাঙ্ক builtin_not_written(সংরচনা child_prog *child);
#endif
/*   o_string manipulation: */
স্থির পূর্ণাঙ্ক b_check_space(o_string *o, পূর্ণাঙ্ক len);
স্থির পূর্ণাঙ্ক b_addchr(o_string *o, পূর্ণাঙ্ক ch);
স্থির শূন্য b_reset(o_string *o);
স্থির পূর্ণাঙ্ক b_addqchr(o_string *o, পূর্ণাঙ্ক ch, পূর্ণাঙ্ক quote);
#ifndef __U_BOOT__
স্থির পূর্ণাঙ্ক b_adduint(o_string *o, unsigned পূর্ণাঙ্ক i);
#endif
/*  in_str manipulations: */
স্থির পূর্ণাঙ্ক static_get(সংরচনা in_str *i);
স্থির পূর্ণাঙ্ক static_peek(সংরচনা in_str *i);
স্থির পূর্ণাঙ্ক file_get(সংরচনা in_str *i);
স্থির পূর্ণাঙ্ক file_peek(সংরচনা in_str *i);
#ifndef __U_BOOT__
স্থির শূন্য setup_file_in_str(সংরচনা in_str *i, FILE *f);
#অন্যথা
স্থির শূন্য setup_file_in_str(সংরচনা in_str *i);
#endif
স্থির শূন্য setup_string_in_str(সংরচনা in_str *i, ধ্রুৱ ৱর্ণ *s);
#ifndef __U_BOOT__
/*  close_me manipulations: */
স্থির শূন্য mark_open(পূর্ণাঙ্ক fd);
স্থির শূন্য mark_closed(পূর্ণাঙ্ক fd);
স্থির শূন্য close_all(শূন্য);
#endif
/*  "run" the final data structures: */
স্থির ৱর্ণ *indenter(পূর্ণাঙ্ক i);
স্থির পূর্ণাঙ্ক free_pipe_list(সংরচনা pipe *head, পূর্ণাঙ্ক indent);
স্থির পূর্ণাঙ্ক free_pipe(সংরচনা pipe *pi, পূর্ণাঙ্ক indent);
/*  really run the final data structures: */
#ifndef __U_BOOT__
স্থির পূর্ণাঙ্ক setup_redirects(সংরচনা child_prog *prog, পূর্ণাঙ্ক squirrel[]);
#endif
স্থির পূর্ণাঙ্ক run_list_real(সংরচনা pipe *pi);
#ifndef __U_BOOT__
স্থির শূন্য pseudo_exec(সংরচনা child_prog *child) __attribute__ ((noreturn));
#endif
স্থির পূর্ণাঙ্ক run_pipe_real(সংরচনা pipe *pi);
/*   extended glob support: */
#ifndef __U_BOOT__
স্থির পূর্ণাঙ্ক globhack(ধ্রুৱ ৱর্ণ *src, পূর্ণাঙ্ক flags, glob_t *pglob);
স্থির পূর্ণাঙ্ক glob_needed(ধ্রুৱ ৱর্ণ *s);
স্থির পূর্ণাঙ্ক xglob(o_string *dest, পূর্ণাঙ্ক flags, glob_t *pglob);
#endif
/*   variable assignment: */
স্থির পূর্ণাঙ্ক is_assignment(ধ্রুৱ ৱর্ণ *s);
/*   data structure manipulation: */
#ifndef __U_BOOT__
স্থির পূর্ণাঙ্ক setup_redirect(সংরচনা p_context *ctx, পূর্ণাঙ্ক fd, redir_type style, সংরচনা in_str *প্ৰবেশ);
#endif
স্থির শূন্য initialize_context(সংরচনা p_context *ctx);
স্থির পূর্ণাঙ্ক done_word(o_string *dest, সংরচনা p_context *ctx);
স্থির পূর্ণাঙ্ক done_command(সংরচনা p_context *ctx);
স্থির পূর্ণাঙ্ক done_pipe(সংরচনা p_context *ctx, pipe_style type);
/*   primary string parsing: */
#ifndef __U_BOOT__
স্থির পূর্ণাঙ্ক redirect_dup_অঙ্ক(সংরচনা in_str *প্ৰবেশ);
স্থির পূর্ণাঙ্ক redirect_opt_অঙ্ক(o_string *o);
স্থির পূর্ণাঙ্ক process_command_subs(o_string *dest, সংরচনা p_context *ctx, সংরচনা in_str *প্ৰবেশ, পূর্ণাঙ্ক subst_end);
স্থির পূর্ণাঙ্ক parse_group(o_string *dest, সংরচনা p_context *ctx, সংরচনা in_str *প্ৰবেশ, পূর্ণাঙ্ক ch);
#endif
স্থির ৱর্ণ *lookup_param(ৱর্ণ *src);
স্থির ৱর্ণ *make_string(ৱর্ণ **inp, পূর্ণাঙ্ক *nonnull);
স্থির পূর্ণাঙ্ক handle_dollar(o_string *dest, সংরচনা p_context *ctx, সংরচনা in_str *প্ৰবেশ);
#ifndef __U_BOOT__
স্থির পূর্ণাঙ্ক parse_string(o_string *dest, সংরচনা p_context *ctx, ধ্রুৱ ৱর্ণ *src);
#endif
স্থির পূর্ণাঙ্ক parse_stream(o_string *dest, সংরচনা p_context *ctx, সংরচনা in_str *input0, পূর্ণাঙ্ক end_trigger);
/*   setup: */
স্থির পূর্ণাঙ্ক parse_stream_outer(সংরচনা in_str *inp, পূর্ণাঙ্ক flag);
#ifndef __U_BOOT__
স্থির পূর্ণাঙ্ক parse_string_outer(ধ্রুৱ ৱর্ণ *s, পূর্ণাঙ্ক flag);
স্থির পূর্ণাঙ্ক parse_file_outer(FILE *f);
#endif
#ifndef __U_BOOT__
/*   job management: */
স্থির পূর্ণাঙ্ক checkjobs(সংরচনা pipe* fg_pipe);
স্থির শূন্য ঢোকাও_bg_job(সংরচনা pipe *pi);
স্থির শূন্য remove_bg_job(সংরচনা pipe *pi);
#endif
/*     local variable support */
স্থির ৱর্ণ **make_list_in(ৱর্ণ **inp, ৱর্ণ *name);
স্থির ৱর্ণ *ঢোকাও_var_মান(ৱর্ণ *inp);
স্থির ৱর্ণ *ঢোকাও_var_মান_sub(ৱর্ণ *inp, পূর্ণাঙ্ক tag_subst);

#ifndef __U_BOOT__
/* Table of built-in functions.  They can be forked or not, depending on
 * context: within pipes, they fork.  As simple commands, they do not.
 * When used in non-forking context, they can change global variables
 * in the parent shell process.  If forked, of course they can not.
 * For example, 'unset foo | whatever' will parse and run, but foo will
 * still be set at the end. */
স্থির সংরচনা built_in_command bltins[] = {
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
	{রিক্ত, রিক্ত, রিক্ত}
};

স্থির ধ্রুৱ ৱর্ণ *set_cwd(শূন্য)
{
	যদি(cwd==unknown)
		cwd = রিক্ত;     /* xgetcwd(arg) called free(arg) */
	cwd = xgetcwd((ৱর্ণ *)cwd);
	যদি (!cwd)
		cwd = unknown;
	ঘূৰাই cwd;
}

/* built-in 'eval' handler */
স্থির পূর্ণাঙ্ক builtin_eval(সংরচনা child_prog *child)
{
	ৱর্ণ *str = রিক্ত;
	পূর্ণাঙ্ক rcode = EXIT_SUCCESS;

	যদি (child->argv[1]) {
		str = make_string(child->argv + 1);
		parse_string_outer(str, FLAG_EXIT_FROM_LOOP |
					FLAG_PARSE_SEMICOLON);
		মুক্ত(str);
		rcode = last_return_code;
	}
	ঘূৰাই rcode;
}

/* built-in 'cd <path>' handler */
স্থির পূর্ণাঙ্ক builtin_cd(সংরচনা child_prog *child)
{
	ৱর্ণ *newdir;
	যদি (child->argv[1] == রিক্ত)
		newdir = env_get("HOME");
	অন্যথা
		newdir = child->argv[1];
	যদি (chdir(newdir)) {
		printf("cd: %s: %s\n", newdir, strerror(errno));
		ঘূৰাই EXIT_FAILURE;
	}
	set_cwd();
	ঘূৰাই EXIT_SUCCESS;
}

/* built-in 'env' handler */
স্থির পূর্ণাঙ্ক builtin_env(সংরচনা child_prog *dummy)
{
	ৱর্ণ **e = environ;
	যদি (e == রিক্ত) ঘূৰাই EXIT_FAILURE;
	বাবে (; *e; e++) {
		puts(*e);
	}
	ঘূৰাই EXIT_SUCCESS;
}

/* built-in 'exec' handler */
স্থির পূর্ণাঙ্ক builtin_exec(সংরচনা child_prog *child)
{
	যদি (child->argv[1] == রিক্ত)
		ঘূৰাই EXIT_SUCCESS;   /* Really? */
	child->argv++;
	pseudo_exec(child);
	/* never returns */
}

/* built-in 'exit' handler */
স্থির পূর্ণাঙ্ক builtin_exit(সংরচনা child_prog *child)
{
	যদি (child->argv[1] == রিক্ত)
		exit(last_return_code);
	exit (atoi(child->argv[1]));
}

/* built-in 'export VAR=value' handler */
স্থির পূর্ণাঙ্ক builtin_export(সংরচনা child_prog *child)
{
	পূর্ণাঙ্ক res = 0;
	ৱর্ণ *name = child->argv[1];

	যদি (name == রিক্ত) {
		ঘূৰাই (builtin_env(child));
	}

	name = strdup(name);

	যদি(name) {
		ৱর্ণ *মান = strchr(name, '=');

		যদি (!মান) {
			ৱর্ণ *tmp;
			/* They are exporting something without an =VALUE */

			মান = get_local_var(name);
			যদি (মান) {
				size_t ln = strlen(name);

				tmp = realloc(name, ln+strlen(মান)+2);
				যদি(tmp==রিক্ত)
					res = -1;
				অন্যথা {
					sprintf(tmp+ln, "=%s", মান);
					name = tmp;
				}
			} অন্যথা {
				/* bash does not return an error when trying to export
				 * an undefined variable.  Do likewise. */
				res = 1;
			}
		}
	}
	যদি (res<0)
		perror_বাৰ্তা("export");
	অন্যথা যদি(res==0)
		res = set_local_var(name, 1);
	অন্যথা
		res = 0;
	মুক্ত(name);
	ঘূৰাই res;
}

/* built-in 'fg' and 'bg' handler */
স্থির পূর্ণাঙ্ক builtin_fg_bg(সংরচনা child_prog *child)
{
	পূর্ণাঙ্ক i, jobnum;
	সংরচনা pipe *pi=রিক্ত;

	যদি (!interactive)
		ঘূৰাই EXIT_FAILURE;
	/* If they gave us no args, assume they want the last backgrounded task */
	যদি (!child->argv[1]) {
		বাবে (pi = job_list; pi; pi = pi->next) {
			যদি (pi->jobid == last_jobid) {
				ভঙ্গ;
			}
		}
		যদি (!pi) {
			error_বাৰ্তা("%s: no current job", child->argv[0]);
			ঘূৰাই EXIT_FAILURE;
		}
	} অন্যথা {
		যদি (sscanf(child->argv[1], "%%%d", &jobnum) != 1) {
			error_বাৰ্তা("%s: bad argument '%s'", child->argv[0], child->argv[1]);
			ঘূৰাই EXIT_FAILURE;
		}
		বাবে (pi = job_list; pi; pi = pi->next) {
			যদি (pi->jobid == jobnum) {
				ভঙ্গ;
			}
		}
		যদি (!pi) {
			error_বাৰ্তা("%s: %d: no such job", child->argv[0], jobnum);
			ঘূৰাই EXIT_FAILURE;
		}
	}

	যদি (*child->argv[0] == 'f') {
		/* Put the job into the foreground.  */
		tcsetpgrp(shell_terminal, pi->pgrp);
	}

	/* Restart the processes in the job */
	বাবে (i = 0; i < pi->অঙ্ক_progs; i++)
		pi->progs[i].is_stopped = 0;

	যদি ( (i=kill(- pi->pgrp, SIGCONT)) < 0) {
		যদি (i == ESRCH) {
			remove_bg_job(pi);
		} অন্যথা {
			perror_বাৰ্তা("kill (SIGCONT)");
		}
	}

	pi->stopped_progs = 0;
	ঘূৰাই EXIT_SUCCESS;
}

/* built-in 'help' handler */
স্থির পূর্ণাঙ্ক builtin_help(সংরচনা child_prog *dummy)
{
	সংরচনা built_in_command *x;

	printf("\nBuilt-in commands:\n");
	printf("-------------------\n");
	বাবে (x = bltins; x->cmd; x++) {
		যদি (x->descr==রিক্ত)
			অনুৱর্ত;
		printf("%s\t%s\n", x->cmd, x->descr);
	}
	printf("\n\n");
	ঘূৰাই EXIT_SUCCESS;
}

/* built-in 'jobs' handler */
স্থির পূর্ণাঙ্ক builtin_jobs(সংরচনা child_prog *child)
{
	সংরচনা pipe *job;
	ৱর্ণ *status_string;

	বাবে (job = job_list; job; job = job->next) {
		যদি (job->running_progs == job->stopped_progs)
			status_string = "Stopped";
		অন্যথা
			status_string = "Running";

		printf(JOB_STATUS_FORMAT, job->jobid, status_string, job->text);
	}
	ঘূৰাই EXIT_SUCCESS;
}

/* built-in 'pwd' handler */
স্থির পূর্ণাঙ্ক builtin_pwd(সংরচনা child_prog *dummy)
{
	puts(set_cwd());
	ঘূৰাই EXIT_SUCCESS;
}

/* built-in 'read VAR' handler */
স্থির পূর্ণাঙ্ক builtin_read(সংরচনা child_prog *child)
{
	পূর্ণাঙ্ক res;

	যদি (child->argv[1]) {
		ৱর্ণ string[BUFSIZ];
		ৱর্ণ *var = 0;

		string[0] = 0;  /* In case stdin has only EOF */
		/* read string */
		fgets(string, পরিমাণ(string), stdin);
		chomp(string);
		var = malloc(strlen(child->argv[1])+strlen(string)+2);
		যদি(var) {
			sprintf(var, "%s=%s", child->argv[1], string);
			res = set_local_var(var, 0);
		} অন্যথা
			res = -1;
		যদি (res)
			fprintf(stderr, "read: %m\n");
		মুক্ত(var);      /* So not move up to avoid breaking errno */
		ঘূৰাই res;
	} অন্যথা {
		কৰা res=getchar(); যেতিয়ালৈ(res!='\n' && res!=EOF);
		ঘূৰাই 0;
	}
}

/* built-in 'set VAR=value' handler */
স্থির পূর্ণাঙ্ক builtin_set(সংরচনা child_prog *child)
{
	ৱর্ণ *temp = child->argv[1];
	সংরচনা variables *e;

	যদি (temp == রিক্ত)
		বাবে(e = শীৰ্ষ_vars; e; e=e->next)
			printf("%s=%s\n", e->name, e->মান);
	অন্যথা
		set_local_var(temp, 0);

		ঘূৰাই EXIT_SUCCESS;
}

/* Built-in 'shift' handler */
স্থির পূর্ণাঙ্ক builtin_shift(সংরচনা child_prog *child)
{
	পূর্ণাঙ্ক n=1;
	যদি (child->argv[1]) {
		n=atoi(child->argv[1]);
	}
	যদি (n>=0 && n<global_argc) {
		/* XXX This probably breaks $0 */
		global_argc -= n;
		global_argv += n;
		ঘূৰাই EXIT_SUCCESS;
	} অন্যথা {
		ঘূৰাই EXIT_FAILURE;
	}
}

/* Built-in '.' handler (read-in and execute commands from file) */
স্থির পূর্ণাঙ্ক builtin_source(সংরচনা child_prog *child)
{
	FILE *প্ৰবেশ;
	পূর্ণাঙ্ক status;

	যদি (child->argv[1] == রিক্ত)
		ঘূৰাই EXIT_FAILURE;

	/* XXX search through $PATH is missing */
	প্ৰবেশ = fopen(child->argv[1], "r");
	যদি (!প্ৰবেশ) {
		error_বাৰ্তা("Couldn't open file '%s'", child->argv[1]);
		ঘূৰাই EXIT_FAILURE;
	}

	/* Now run the file */
	/* XXX argv and argc are broken; need to save old global_argv
	 * (pointer only is OK!) on this stack frame,
	 * set global_argv=child->argv+1, recurse, and restore. */
	mark_open(fileno(প্ৰবেশ));
	status = parse_file_outer(প্ৰবেশ);
	mark_closed(fileno(প্ৰবেশ));
	fclose(প্ৰবেশ);
	ঘূৰাই (status);
}

স্থির পূর্ণাঙ্ক builtin_umask(সংরচনা child_prog *child)
{
	mode_t new_umask;
	ধ্রুৱ ৱর্ণ *arg = child->argv[1];
	ৱর্ণ *end;
	যদি (arg) {
		new_umask=strtoul(arg, &end, 8);
		যদি (*end!='\0' || end == arg) {
			ঘূৰাই EXIT_FAILURE;
		}
	} অন্যথা {
		printf("%.3o\n", (unsigned পূর্ণাঙ্ক) (new_umask=umask(0)));
	}
	umask(new_umask);
	ঘূৰাই EXIT_SUCCESS;
}

/* built-in 'unset VAR' handler */
স্থির পূর্ণাঙ্ক builtin_unset(সংরচনা child_prog *child)
{
	/* bash returned already true */
	unset_local_var(child->argv[1]);
	ঘূৰাই EXIT_SUCCESS;
}

স্থির পূর্ণাঙ্ক builtin_not_written(সংরচনা child_prog *child)
{
	printf("builtin_%s not written\n",child->argv[0]);
	ঘূৰাই EXIT_FAILURE;
}
#endif

স্থির পূর্ণাঙ্ক b_check_space(o_string *o, পূর্ণাঙ্ক len)
{
	/* It would be easy to drop a more restrictive policy
	 * in here, such as setting a maximum string length */
	যদি (o->length + len > o->maxlen) {
		ৱর্ণ *old_data = o->data;
		/* assert (data == NULL || o->maxlen != 0); */
		o->maxlen += max(2*len, B_CHUNK);
		o->data = realloc(o->data, 1 + o->maxlen);
		যদি (o->data == রিক্ত) {
			মুক্ত(old_data);
		}
	}
	ঘূৰাই o->data == রিক্ত;
}

স্থির পূর্ণাঙ্ক b_addchr(o_string *o, পূর্ণাঙ্ক ch)
{
	debug_printf("b_addchr: %c %d %p\n", ch, o->length, o);
	যদি (b_check_space(o, 1)) ঘূৰাই B_NOSPAC;
	o->data[o->length] = ch;
	o->length++;
	o->data[o->length] = '\0';
	ঘূৰাই 0;
}

স্থির শূন্য b_reset(o_string *o)
{
	o->length = 0;
	o->nonnull = 0;
	যদি (o->data != রিক্ত) *o->data = '\0';
}

স্থির শূন্য b_free(o_string *o)
{
	b_reset(o);
	মুক্ত(o->data);
	o->data = রিক্ত;
	o->maxlen = 0;
}

/* My analysis of quoting semantics tells me that state information
 * is associated with a destination, not a source.
 */
স্থির পূর্ণাঙ্ক b_addqchr(o_string *o, পূর্ণাঙ্ক ch, পূর্ণাঙ্ক quote)
{
	যদি (quote && strchr("*?[\\",ch)) {
		পূর্ণাঙ্ক rc;
		rc = b_addchr(o, '\\');
		যদি (rc) ঘূৰাই rc;
	}
	ঘূৰাই b_addchr(o, ch);
}

#ifndef __U_BOOT__
স্থির পূর্ণাঙ্ক b_adduint(o_string *o, unsigned পূর্ণাঙ্ক i)
{
	পূর্ণাঙ্ক r;
	ৱর্ণ *p = simple_itoa(i);
	/* no escape checking necessary */
	কৰা r=b_addchr(o, *p++); যেতিয়ালৈ (r==0 && *p);
	ঘূৰাই r;
}
#endif

স্থির পূর্ণাঙ্ক static_get(সংরচনা in_str *i)
{
	পূর্ণাঙ্ক ch = *i->p++;
	যদি (ch=='\0') ঘূৰাই EOF;
	ঘূৰাই ch;
}

স্থির পূর্ণাঙ্ক static_peek(সংরচনা in_str *i)
{
	ঘূৰাই *i->p;
}

#ifndef __U_BOOT__
স্থির অন্তঃস্থ শূন্য cmdedit_set_initial_prompt(শূন্য)
{
#ifndef CONFIG_FEATURE_SH_FANCY_PROMPT
	PS1 = রিক্ত;
#অন্যথা
	PS1 = env_get("PS1");
	যদি(PS1==0)
		PS1 = "\\w \\$ ";
#endif
}

স্থির অন্তঃস্থ শূন্য setup_prompt_string(পূর্ণাঙ্ক promptmode, ৱর্ণ **prompt_str)
{
	debug_printf("setup_prompt_string %d ",promptmode);
#ifndef CONFIG_FEATURE_SH_FANCY_PROMPT
	/* Set up the prompt */
	যদি (promptmode == 1) {
		মুক্ত(PS1);
		PS1=xmalloc(strlen(cwd)+4);
		sprintf(PS1, "%s %s", cwd, ( geteuid() != 0 ) ?  "$ ":"# ");
		*prompt_str = PS1;
	} অন্যথা {
		*prompt_str = PS2;
	}
#অন্যথা
	*prompt_str = (promptmode==1)? PS1 : PS2;
#endif
	debug_printf("result %s\n",*prompt_str);
}
#endif

#ifdef __U_BOOT__
স্থির পূর্ণাঙ্ক uboot_cli_readline(সংরচনা in_str *i)
{
	ৱর্ণ *prompt;
	ৱর্ণ __maybe_unused *ps_prompt = রিক্ত;

	যদি (i->promptmode == 1)
		prompt = CONFIG_SYS_PROMPT;
	অন্যথা
		prompt = CONFIG_SYS_PROMPT_HUSH_PS2;

#ifdef CONFIG_CMDLINE_PS_SUPPORT
	যদি (i->promptmode == 1)
		ps_prompt = env_get("PS1");
	অন্যথা
		ps_prompt = env_get("PS2");
	যদি (ps_prompt)
		prompt = ps_prompt;
#endif

	ঘূৰাই cli_readline(prompt);
}
#endif

স্থির শূন্য get_user_প্ৰবেশ(সংরচনা in_str *i)
{
#ifndef __U_BOOT__
	ৱর্ণ *prompt_str;
	স্থির ৱর্ণ the_command[BUFSIZ];

	setup_prompt_string(i->promptmode, &prompt_str);
#ifdef CONFIG_FEATURE_COMMAND_EDITING
	/*
	 ** enable command line editing only while a command line
	 ** is actually being read; otherwise, we'll end up bequeathing
	 ** atexit() handlers and other unwanted stuff to our
	 ** child processes (rob@sysgo.de)
	 */
	cmdedit_read_প্ৰবেশ(prompt_str, the_command);
#অন্যথা
	fputs(prompt_str, stdout);
	fflush(stdout);
	the_command[0]=fgetc(i->file);
	the_command[1]='\0';
#endif
	fflush(stdout);
	i->p = the_command;
#অন্যথা
	পূর্ণাঙ্ক n;
	স্থির ৱর্ণ the_command[CONFIG_SYS_CBSIZE + 1];

	bootretry_reset_cmd_timeout();
	i->__promptme = 1;
	n = uboot_cli_readline(i);

#ifdef CONFIG_BOOT_RETRY_TIME
	যদি (n == -2) {
	  puts("\nTimeout waiting for command\n");
#  ifdef CONFIG_RESET_TO_RETRY
	  do_reset(রিক্ত, 0, 0, রিক্ত);
#  elif IS_ENABLED(CONFIG_RETRY_BOOTCMD)
	strcpy(console_buffer, "run bootcmd\n");
# অন্যথা
#	error "This only works with CONFIG_RESET_TO_RETRY or CONFIG_BOOT_RETRY_COMMAND enabled"
#  endif
	}
#endif
	যদি (n == -1 ) {
		flag_repeat = 0;
		i->__promptme = 0;
	}
	n = strlen(console_buffer);
	console_buffer[n] = '\n';
	console_buffer[n+1]= '\0';
	যদি (had_ctrlc()) flag_repeat = 0;
	clear_ctrlc();
	do_repeat = 0;
	যদি (i->promptmode == 1) {
		যদি (console_buffer[0] == '\n'&& flag_repeat == 0) {
			strcpy(the_command,console_buffer);
		}
		অন্যথা {
			যদি (console_buffer[0] != '\n') {
				strcpy(the_command,console_buffer);
				flag_repeat = 1;
			}
			অন্যথা {
				do_repeat = 1;
			}
		}
		i->p = the_command;
	}
	অন্যথা {
		যদি (console_buffer[0] != '\n') {
			যদি (strlen(the_command) + strlen(console_buffer)
			    < CONFIG_SYS_CBSIZE) {
				n = strlen(the_command);
				the_command[n-1] = ' ';
				strcpy(&the_command[n],console_buffer);
			}
			অন্যথা {
				the_command[0] = '\n';
				the_command[1] = '\0';
				flag_repeat = 0;
			}
		}
		যদি (i->__promptme == 0) {
			the_command[0] = '\n';
			the_command[1] = '\0';
		}
		i->p = console_buffer;
	}
#endif
}

/* This is the magic location that prints prompts
 * and gets data back from the user */
স্থির পূর্ণাঙ্ক file_get(সংরচনা in_str *i)
{
	পূর্ণাঙ্ক ch;

	ch = 0;
	/* If there is data waiting, eat it up */
	যদি (i->p && *i->p) {
		ch = *i->p++;
	} অন্যথা {
		/* need to double check i->file because we might be doing something
		 * more complicated by now, like sourcing or substituting. */
#ifndef __U_BOOT__
		যদি (i->__promptme && interactive && i->file == stdin) {
			যেতিয়ালৈ(! i->p || (interactive && strlen(i->p)==0) ) {
#অন্যথা
			যেতিয়ালৈ(! i->p  || strlen(i->p)==0 ) {
#endif
				get_user_প্ৰবেশ(i);
			}
			i->promptmode=2;
#ifndef __U_BOOT__
			i->__promptme = 0;
#endif
			যদি (i->p && *i->p) {
				ch = *i->p++;
			}
#ifndef __U_BOOT__
		} অন্যথা {
			ch = fgetc(i->file);
		}

#endif
		debug_printf("b_getch: got a %d\n", ch);
	}
#ifndef __U_BOOT__
	যদি (ch == '\n') i->__promptme=1;
#endif
	ঘূৰাই ch;
}

/* All the callers guarantee this routine will never be
 * used right after a newline, so prompting is not needed.
 */
স্থির পূর্ণাঙ্ক file_peek(সংরচনা in_str *i)
{
#ifndef __U_BOOT__
	যদি (i->p && *i->p) {
#endif
		ঘূৰাই *i->p;
#ifndef __U_BOOT__
	} অন্যথা {
		i->peek_buf[0] = fgetc(i->file);
		i->peek_buf[1] = '\0';
		i->p = i->peek_buf;
		debug_printf("b_peek: got a %d\n", *i->p);
		ঘূৰাই *i->p;
	}
#endif
}

#ifndef __U_BOOT__
স্থির শূন্য setup_file_in_str(সংরচনা in_str *i, FILE *f)
#অন্যথা
স্থির শূন্য setup_file_in_str(সংরচনা in_str *i)
#endif
{
	i->peek = file_peek;
	i->get = file_get;
	i->__promptme=1;
	i->promptmode=1;
#ifndef __U_BOOT__
	i->file = f;
#endif
	i->p = রিক্ত;
}

স্থির শূন্য setup_string_in_str(সংরচনা in_str *i, ধ্রুৱ ৱর্ণ *s)
{
	i->peek = static_peek;
	i->get = static_get;
	i->__promptme=1;
	i->promptmode=1;
	i->p = s;
}

#ifndef __U_BOOT__
স্থির শূন্য mark_open(পূর্ণাঙ্ক fd)
{
	সংরচনা close_me *new = xmalloc(পরিমাণ(সংরচনা close_me));
	new->fd = fd;
	new->next = close_me_head;
	close_me_head = new;
}

স্থির শূন্য mark_closed(পূর্ণাঙ্ক fd)
{
	সংরচনা close_me *tmp;
	যদি (close_me_head == রিক্ত || close_me_head->fd != fd)
		error_বাৰ্তা_and_die("corrupt close_me");
	tmp = close_me_head;
	close_me_head = close_me_head->next;
	মুক্ত(tmp);
}

স্থির শূন্য close_all(শূন্য)
{
	সংরচনা close_me *c;
	বাবে (c=close_me_head; c; c=c->next) {
		close(c->fd);
	}
	close_me_head = রিক্ত;
}

/* squirrel != NULL means we squirrel away copies of stdin, stdout,
 * and stderr if they are redirected. */
স্থির পূর্ণাঙ্ক setup_redirects(সংরচনা child_prog *prog, পূর্ণাঙ্ক squirrel[])
{
	পূর্ণাঙ্ক openfd, mode;
	সংরচনা redir_struct *redir;

	বাবে (redir=prog->redirects; redir; redir=redir->next) {
		যদি (redir->dup == -1 && redir->word.gl_pathv == রিক্ত) {
			/* something went wrong in the parse.  Pretend it didn't happen */
			অনুৱর্ত;
		}
		যদি (redir->dup == -1) {
			mode=redir_table[redir->type].mode;
			openfd = open(redir->word.gl_pathv[0], mode, 0666);
			যদি (openfd < 0) {
			/* this could get lost if stderr has been redirected, but
			   bash and ash both lose it as well (though zsh doesn't!) */
				perror_বাৰ্তা("error opening %s", redir->word.gl_pathv[0]);
				ঘূৰাই 1;
			}
		} অন্যথা {
			openfd = redir->dup;
		}

		যদি (openfd != redir->fd) {
			যদি (squirrel && redir->fd < 3) {
				squirrel[redir->fd] = dup(redir->fd);
			}
			যদি (openfd == -3) {
				close(openfd);
			} অন্যথা {
				dup2(openfd, redir->fd);
				যদি (redir->dup == -1)
					close (openfd);
			}
		}
	}
	ঘূৰাই 0;
}

স্থির শূন্য restore_redirects(পূর্ণাঙ্ক squirrel[])
{
	পূর্ণাঙ্ক i, fd;
	বাবে (i=0; i<3; i++) {
		fd = squirrel[i];
		যদি (fd != -1) {
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
স্থির শূন্য pseudo_exec(সংরচনা child_prog *child)
{
	পূর্ণাঙ্ক i, rcode;
	ৱর্ণ *p;
	সংরচনা built_in_command *x;
	যদি (child->argv) {
		বাবে (i=0; is_assignment(child->argv[i]); i++) {
			debug_printf("pid %d environment modification: %s\n",getpid(),child->argv[i]);
			p = ঢোকাও_var_মান(child->argv[i]);
			putenv(strdup(p));
			যদি (p != child->argv[i]) মুক্ত(p);
		}
		child->argv+=i;  /* XXX this hack isn't so horrible, since we are about
					to exit, and therefore don't need to keep data
					structures consistent for free() use. */
		/* If a variable is assigned in a forest, and nobody listens,
		 * was it ever really set?
		 */
		যদি (child->argv[0] == রিক্ত) {
			_exit(EXIT_SUCCESS);
		}

		/*
		 * Check if the command matches any of the builtins.
		 * Depending on context, this might be redundant.  But it's
		 * easier to waste a few CPU cycles than it is to figure out
		 * if this is one of those cases.
		 */
		বাবে (x = bltins; x->cmd; x++) {
			যদি (strcmp(child->argv[0], x->cmd) == 0 ) {
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
			পূর্ণাঙ্ক argc_l;
			ৱর্ণ** argv_l=child->argv;
			ৱর্ণ *name = child->argv[0];

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
			বাবে(argc_l=0;*argv_l!=রিক্ত; argv_l++, argc_l++);
			optind = 1;
			debug_printf("running applet %s\n", name);
			run_applet_by_name(name, argc_l, child->argv);
		}
#endif
		debug_printf("exec of %s\n",child->argv[0]);
		execvp(child->argv[0],child->argv);
		perror_বাৰ্তা("couldn't exec: %s",child->argv[0]);
		_exit(1);
	} অন্যথা যদি (child->group) {
		debug_printf("runtime nesting to group\n");
		interactive=0;    /* crucial!!!! */
		rcode = run_list_real(child->group);
		/* OK to leak memory by not calling free_pipe_list,
		 * since this process is about to exit */
		_exit(rcode);
	} অন্যথা {
		/* Can happen.  See what bash does with ">foo" by itself. */
		debug_printf("trying to pseudo_exec null command\n");
		_exit(EXIT_SUCCESS);
	}
}

স্থির শূন্য ঢোকাও_bg_job(সংরচনা pipe *pi)
{
	সংরচনা pipe *thejob;

	/* Linear search for the ID of the job to use */
	pi->jobid = 1;
	বাবে (thejob = job_list; thejob; thejob = thejob->next)
		যদি (thejob->jobid >= pi->jobid)
			pi->jobid = thejob->jobid + 1;

	/* add thejob to the list of running jobs */
	যদি (!job_list) {
		thejob = job_list = xmalloc(পরিমাণ(*thejob));
	} অন্যথা {
		বাবে (thejob = job_list; thejob->next; thejob = thejob->next) /* nothing */;
		thejob->next = xmalloc(পরিমাণ(*thejob));
		thejob = thejob->next;
	}

	/* physically copy the struct job */
	memcpy(thejob, pi, পরিমাণ(সংরচনা pipe));
	thejob->next = রিক্ত;
	thejob->running_progs = thejob->অঙ্ক_progs;
	thejob->stopped_progs = 0;
	thejob->text = xmalloc(BUFSIZ); /* cmdedit buffer size */

	/*if (pi->progs[0] && pi->progs[0].argv && pi->progs[0].argv[0]) */
	{
		ৱর্ণ *bar=thejob->text;
		ৱর্ণ **foo=pi->progs[0].argv;
		যেতিয়ালৈ(foo && *foo) {
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
স্থির শূন্য remove_bg_job(সংরচনা pipe *pi)
{
	সংরচনা pipe *prev_pipe;

	যদি (pi == job_list) {
		job_list = pi->next;
	} অন্যথা {
		prev_pipe = job_list;
		যেতিয়ালৈ (prev_pipe->next != pi)
			prev_pipe = prev_pipe->next;
		prev_pipe->next = pi->next;
	}
	যদি (job_list)
		last_jobid = job_list->jobid;
	অন্যথা
		last_jobid = 0;

	pi->stopped_progs = 0;
	free_pipe(pi, 0);
	মুক্ত(pi);
}

/* Checks to see if any processes have exited -- if they
   have, figure out why and see if a job has completed */
স্থির পূর্ণাঙ্ক checkjobs(সংরচনা pipe* fg_pipe)
{
	পূর্ণাঙ্ক attributes;
	পূর্ণাঙ্ক status;
	পূর্ণাঙ্ক prognum = 0;
	সংরচনা pipe *pi;
	pid_t childpid;

	attributes = WUNTRACED;
	যদি (fg_pipe==রিক্ত) {
		attributes |= WNOHANG;
	}

	যেতিয়ালৈ ((childpid = waitpid(-1, &status, attributes)) > 0) {
		যদি (fg_pipe) {
			পূর্ণাঙ্ক i, rcode = 0;
			বাবে (i=0; i < fg_pipe->অঙ্ক_progs; i++) {
				যদি (fg_pipe->progs[i].pid == childpid) {
					যদি (i==fg_pipe->অঙ্ক_progs-1)
						rcode=WEXITSTATUS(status);
					(fg_pipe->অঙ্ক_progs)--;
					ঘূৰাই(rcode);
				}
			}
		}

		বাবে (pi = job_list; pi; pi = pi->next) {
			prognum = 0;
			যেতিয়ালৈ (prognum < pi->অঙ্ক_progs && pi->progs[prognum].pid != childpid) {
				prognum++;
			}
			যদি (prognum < pi->অঙ্ক_progs)
				ভঙ্গ;
		}

		যদি(pi==রিক্ত) {
			debug_printf("checkjobs: pid %d was not in our list!\n", childpid);
			অনুৱর্ত;
		}

		যদি (WIFEXITED(status) || WIFSIGNALED(status)) {
			/* child exited */
			pi->running_progs--;
			pi->progs[prognum].pid = 0;

			যদি (!pi->running_progs) {
				printf(JOB_STATUS_FORMAT, pi->jobid, "Done", pi->text);
				remove_bg_job(pi);
			}
		} অন্যথা {
			/* child stopped */
			pi->stopped_progs++;
			pi->progs[prognum].is_stopped = 1;

#যদি 0
			/* Printing this stuff is a pain, since it tends to
			 * overwrite the prompt an inconveinient moments.  So
			 * don't do that.  */
			যদি (pi->stopped_progs == pi->অঙ্ক_progs) {
				printf("\n"JOB_STATUS_FORMAT, pi->jobid, "Stopped", pi->text);
			}
#endif
		}
	}

	যদি (childpid == -1 && errno != ECHILD)
		perror_বাৰ্তা("waitpid");

	/* move the shell to the foreground */
	/*if (interactive && tcsetpgrp(shell_terminal, getpgid(0))) */
	/*	perror_msg("tcsetpgrp-2"); */
	ঘূৰাই -1;
}

/* Figure out our controlling tty, checking in order stderr,
 * stdin, and stdout.  If check_pgrp is set, also check that
 * we belong to the foreground process group associated with
 * that tty.  The value of shell_terminal is needed in order to call
 * tcsetpgrp(shell_terminal, ...); */
শূন্য controlling_tty(পূর্ণাঙ্ক check_pgrp)
{
	pid_t curpgrp;

	যদি ((curpgrp = tcgetpgrp(shell_terminal = 2)) < 0
			&& (curpgrp = tcgetpgrp(shell_terminal = 0)) < 0
			&& (curpgrp = tcgetpgrp(shell_terminal = 1)) < 0)
		গচ্ছ shell_terminal_error;

	যদি (check_pgrp && curpgrp != getpgid(0))
		গচ্ছ shell_terminal_error;

	ঘূৰাই;

shell_terminal_error:
		shell_terminal = -1;
		ঘূৰাই;
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
স্থির পূর্ণাঙ্ক run_pipe_real(সংরচনা pipe *pi)
{
	পূর্ণাঙ্ক i;
#ifndef __U_BOOT__
	পূর্ণাঙ্ক nextin, nextout;
	পূর্ণাঙ্ক pipefds[2];				/* pipefds[0] is for reading */
	সংরচনা child_prog *child;
	সংরচনা built_in_command *x;
	ৱর্ণ *p;
# যদি __GNUC__
	/* Avoid longjmp clobbering */
	(শূন্য) &i;
	(শূন্য) &nextin;
	(শূন্য) &nextout;
	(শূন্য) &child;
# endif
#অন্যথা
	পূর্ণাঙ্ক nextin;
	পূর্ণাঙ্ক flag = do_repeat ? CMD_FLAG_REPEAT : 0;
	সংরচনা child_prog *child;
	ৱর্ণ *p;
# যদি __GNUC__
	/* Avoid longjmp clobbering */
	(শূন্য) &i;
	(শূন্য) &nextin;
	(শূন্য) &child;
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
	যদি (pi->অঙ্ক_progs == 1) child = & (pi->progs[0]);
#ifndef __U_BOOT__
	যদি (pi->অঙ্ক_progs == 1 && child->group && child->subshell == 0) {
		পূর্ণাঙ্ক squirrel[] = {-1, -1, -1};
		পূর্ণাঙ্ক rcode;
		debug_printf("non-subshell grouping\n");
		setup_redirects(child, squirrel);
		/* XXX could we merge code with following builtin case,
		 * by creating a pseudo builtin that calls run_list_real? */
		rcode = run_list_real(child->group);
		restore_redirects(squirrel);
#অন্যথা
		যদি (pi->অঙ্ক_progs == 1 && child->group) {
		পূর্ণাঙ্ক rcode;
		debug_printf("non-subshell grouping\n");
		rcode = run_list_real(child->group);
#endif
		ঘূৰাই rcode;
	} অন্যথা যদি (pi->অঙ্ক_progs == 1 && pi->progs[0].argv != রিক্ত) {
		বাবে (i=0; is_assignment(child->argv[i]); i++) { /* nothing */ }
		যদি (i!=0 && child->argv[i]==রিক্ত) {
			/* assignments, but no command: set the local environment */
			বাবে (i=0; child->argv[i]!=রিক্ত; i++) {

				/* Ok, this case is tricky.  We have to decide if this is a
				 * local variable, or an already exported variable.  If it is
				 * already exported, we have to export the new value.  If it is
				 * not exported, we need only set this as a local variable.
				 * This junk is all to decide whether or not to export this
				 * variable. */
				পূর্ণাঙ্ক export_me=0;
				ৱর্ণ *name, *মান;
				name = xstrdup(child->argv[i]);
				debug_printf("Local environment set: %s\n", name);
				মান = strchr(name, '=');
				যদি (মান)
					*মান=0;
#ifndef __U_BOOT__
				যদি ( get_local_var(name)) {
					export_me=1;
				}
#endif
				মুক্ত(name);
				p = ঢোকাও_var_মান(child->argv[i]);
				set_local_var(p, export_me);
				যদি (p != child->argv[i]) মুক্ত(p);
			}
			ঘূৰাই EXIT_SUCCESS;   /* don't worry about errors in set_local_var() yet */
		}
		বাবে (i = 0; is_assignment(child->argv[i]); i++) {
			p = ঢোকাও_var_মান(child->argv[i]);
#ifndef __U_BOOT__
			putenv(strdup(p));
#অন্যথা
			set_local_var(p, 0);
#endif
			যদি (p != child->argv[i]) {
				child->sp--;
				মুক্ত(p);
			}
		}
		যদি (child->sp) {
			ৱর্ণ * str = রিক্ত;

			str = make_string(child->argv + i,
					  child->argv_nonnull + i);
			parse_string_outer(str, FLAG_EXIT_FROM_LOOP | FLAG_REPARSING);
			মুক্ত(str);
			ঘূৰাই last_return_code;
		}
#ifndef __U_BOOT__
		বাবে (x = bltins; x->cmd; x++) {
			যদি (strcmp(child->argv[i], x->cmd) == 0 ) {
				পূর্ণাঙ্ক squirrel[] = {-1, -1, -1};
				পূর্ণাঙ্ক rcode;
				যদি (x->function == builtin_exec && child->argv[i+1]==রিক্ত) {
					debug_printf("magic exec\n");
					setup_redirects(child,রিক্ত);
					ঘূৰাই EXIT_SUCCESS;
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
			ঘূৰাই rcode;
		}
#অন্যথা
		/* check ";", because ,example , argv consist from
		 * "help;flinfo" must not execute
		 */
		যদি (strchr(child->argv[i], ';')) {
			printf("Unknown command '%s' - try 'help' or use "
					"'run' command\n", child->argv[i]);
			ঘূৰাই -1;
		}
		/* Process the command */
		ঘূৰাই cmd_process(flag, child->argc - i, child->argv + i,
				   &flag_repeat, রিক্ত);
#endif
	}
#ifndef __U_BOOT__

	বাবে (i = 0; i < pi->অঙ্ক_progs; i++) {
		child = & (pi->progs[i]);

		/* pipes are inserted between pairs of commands */
		যদি ((i + 1) < pi->অঙ্ক_progs) {
			যদি (pipe(pipefds)<0) perror_বাৰ্তা_and_die("pipe");
			nextout = pipefds[1];
		} অন্যথা {
			nextout=1;
			pipefds[0] = -1;
		}

		/* XXX test for failed fork()? */
		যদি (!(child->pid = fork())) {
			/* Set the handling for job control signals back to the default.  */
			সঙ্কেত(SIGINT, SIG_DFL);
			সঙ্কেত(SIGQUIT, SIG_DFL);
			সঙ্কেত(SIGTERM, SIG_DFL);
			সঙ্কেত(SIGTSTP, SIG_DFL);
			সঙ্কেত(SIGTTIN, SIG_DFL);
			সঙ্কেত(SIGTTOU, SIG_DFL);
			সঙ্কেত(SIGCHLD, SIG_DFL);

			close_all();

			যদি (nextin != 0) {
				dup2(nextin, 0);
				close(nextin);
			}
			যদি (nextout != 1) {
				dup2(nextout, 1);
				close(nextout);
			}
			যদি (pipefds[0]!=-1) {
				close(pipefds[0]);  /* opposite end of our output pipe */
			}

			/* Like bash, explicit redirects override pipes,
			 * and the pipe fd is available for dup'ing. */
			setup_redirects(child,রিক্ত);

			যদি (interactive && pi->followup!=PIPE_BG) {
				/* If we (the child) win the race, put ourselves in the process
				 * group whose leader is the first process in this pipe. */
				যদি (pi->pgrp < 0) {
					pi->pgrp = getpid();
				}
				যদি (setpgid(0, pi->pgrp) == 0) {
					tcsetpgrp(2, pi->pgrp);
				}
			}

			pseudo_exec(child);
		}

		/* put our child in the process group whose leader is the
		   first process in this pipe */
		যদি (pi->pgrp < 0) {
			pi->pgrp = child->pid;
		}
		/* Don't check for errors.  The child may be dead already,
		 * in which case setpgid returns error code EACCES. */
		setpgid(child->pid, pi->pgrp);

		যদি (nextin != 0)
			close(nextin);
		যদি (nextout != 1)
			close(nextout);

		/* If there isn't another process, nextin is garbage
		   but it doesn't matter */
		nextin = pipefds[0];
	}
#endif
	ঘূৰাই -1;
}

স্থির পূর্ণাঙ্ক run_list_real(সংরচনা pipe *pi)
{
	ৱর্ণ *save_name = রিক্ত;
	ৱর্ণ **list = রিক্ত;
	ৱর্ণ **save_list = রিক্ত;
	সংরচনা pipe *rpipe;
	পূর্ণাঙ্ক flag_rep = 0;
#ifndef __U_BOOT__
	পূর্ণাঙ্ক save_অঙ্ক_progs;
#endif
	পূর্ণাঙ্ক rcode=0, flag_skip=1;
	পূর্ণাঙ্ক flag_restore = 0;
	পূর্ণাঙ্ক if_code=0, next_if_code=0;  /* need double-buffer to handle elif */
	reserved_style rmode, skip_more_in_this_rmode=RES_XXXX;
	/* check syntax for "for" */
	বাবে (rpipe = pi; rpipe; rpipe = rpipe->next) {
		যদি ((rpipe->r_mode == RES_IN ||
		    rpipe->r_mode == RES_FOR) &&
		    (rpipe->next == রিক্ত)) {
				syntax();
#ifdef __U_BOOT__
				flag_repeat = 0;
#endif
				ঘূৰাই 1;
		}
		যদি ((rpipe->r_mode == RES_IN &&
			(rpipe->next->r_mode == RES_IN &&
			rpipe->next->progs->argv != রিক্ত))||
			(rpipe->r_mode == RES_FOR &&
			rpipe->next->r_mode != RES_IN)) {
				syntax();
#ifdef __U_BOOT__
				flag_repeat = 0;
#endif
				ঘূৰাই 1;
		}
	}
	বাবে (; pi; pi = (flag_restore != 0) ? rpipe : pi->next) {
		যদি (pi->r_mode == RES_WHILE || pi->r_mode == RES_UNTIL ||
			pi->r_mode == RES_FOR) {
#ifdef __U_BOOT__
				/* check Ctrl-C */
				ctrlc();
				যদি ((had_ctrlc())) {
					ঘূৰাই 1;
				}
#endif
				flag_restore = 0;
				যদি (!rpipe) {
					flag_rep = 0;
					rpipe = pi;
				}
		}
		rmode = pi->r_mode;
		debug_printf("rmode=%d  if_code=%d  next_if_code=%d skip_more=%d\n", rmode, if_code, next_if_code, skip_more_in_this_rmode);
		যদি (rmode == skip_more_in_this_rmode && flag_skip) {
			যদি (pi->followup == PIPE_SEQ) flag_skip=0;
			অনুৱর্ত;
		}
		flag_skip = 1;
		skip_more_in_this_rmode = RES_XXXX;
		যদি (rmode == RES_THEN || rmode == RES_ELSE) if_code = next_if_code;
		যদি (rmode == RES_THEN &&  if_code) অনুৱর্ত;
		যদি (rmode == RES_ELSE && !if_code) অনুৱর্ত;
		যদি (rmode == RES_ELIF && !if_code) ভঙ্গ;
		যদি (rmode == RES_FOR && pi->অঙ্ক_progs) {
			যদি (!list) {
				/* if no variable values after "in" we skip "for" */
				যদি (!pi->next->progs->argv) অনুৱর্ত;
				/* create list of variable values */
				list = make_list_in(pi->next->progs->argv,
					pi->progs->argv[0]);
				save_list = list;
				save_name = pi->progs->argv[0];
				pi->progs->argv[0] = রিক্ত;
				flag_rep = 1;
			}
			যদি (!(*list)) {
				মুক্ত(pi->progs->argv[0]);
				মুক্ত(save_list);
				list = রিক্ত;
				flag_rep = 0;
				pi->progs->argv[0] = save_name;
#ifndef __U_BOOT__
				pi->progs->glob_result.gl_pathv[0] =
					pi->progs->argv[0];
#endif
				অনুৱর্ত;
			} অন্যথা {
				/* insert new value from list for variable */
				মুক্ত(pi->progs->argv[0]);
				pi->progs->argv[0] = *list++;
#ifndef __U_BOOT__
				pi->progs->glob_result.gl_pathv[0] =
					pi->progs->argv[0];
#endif
			}
		}
		যদি (rmode == RES_IN) অনুৱর্ত;
		যদি (rmode == RES_DO) {
			যদি (!flag_rep) অনুৱর্ত;
		}
		যদি (rmode == RES_DONE) {
			যদি (flag_rep) {
				flag_restore = 1;
			} অন্যথা {
				rpipe = রিক্ত;
			}
		}
		যদি (pi->অঙ্ক_progs == 0) অনুৱর্ত;
#ifndef __U_BOOT__
		save_অঙ্ক_progs = pi->অঙ্ক_progs; /* save number of programs */
#endif
		rcode = run_pipe_real(pi);
		debug_printf("run_pipe_real returned %d\n",rcode);
#ifndef __U_BOOT__
		যদি (rcode!=-1) {
			/* We only ran a builtin: rcode was set by the return value
			 * of run_pipe_real(), and we don't need to wait for anything. */
		} অন্যথা যদি (pi->followup==PIPE_BG) {
			/* XXX check bash's behavior with nontrivial pipes */
			/* XXX compute jobid */
			/* XXX what does bash do with attempts to background builtins? */
			ঢোকাও_bg_job(pi);
			rcode = EXIT_SUCCESS;
		} অন্যথা {
			যদি (interactive) {
				/* move the new process group into the foreground */
				যদি (tcsetpgrp(shell_terminal, pi->pgrp) && errno != ENOTTY)
					perror_বাৰ্তা("tcsetpgrp-3");
				rcode = checkjobs(pi);
				/* move the shell to the foreground */
				যদি (tcsetpgrp(shell_terminal, getpgid(0)) && errno != ENOTTY)
					perror_বাৰ্তা("tcsetpgrp-4");
			} অন্যথা {
				rcode = checkjobs(pi);
			}
			debug_printf("checkjobs returned %d\n",rcode);
		}
		last_return_code=rcode;
#অন্যথা
		যদি (rcode < -1) {
			last_return_code = -rcode - 2;
			ঘূৰাই -2;	/* exit */
		}
		last_return_code = rcode;
#endif
#ifndef __U_BOOT__
		pi->অঙ্ক_progs = save_অঙ্ক_progs; /* restore number of programs */
#endif
		যদি ( rmode == RES_IF || rmode == RES_ELIF )
			next_if_code=rcode;  /* can be overwritten a number of times */
		যদি (rmode == RES_WHILE)
			flag_rep = !last_return_code;
		যদি (rmode == RES_UNTIL)
			flag_rep = last_return_code;
		যদি ( (rcode==EXIT_SUCCESS && pi->followup==PIPE_OR) ||
		     (rcode!=EXIT_SUCCESS && pi->followup==PIPE_AND) )
			skip_more_in_this_rmode=rmode;
#ifndef __U_BOOT__
		checkjobs(রিক্ত);
#endif
	}
	ঘূৰাই rcode;
}

/* broken, of course, but OK for testing */
স্থির ৱর্ণ *indenter(পূর্ণাঙ্ক i)
{
	স্থির ৱর্ণ blanks[]="                                    ";
	ঘূৰাই &blanks[পরিমাণ(blanks)-i-1];
}

/* return code is the exit status of the pipe */
স্থির পূর্ণাঙ্ক free_pipe(সংরচনা pipe *pi, পূর্ণাঙ্ক indent)
{
	ৱর্ণ **p;
	সংরচনা child_prog *child;
#ifndef __U_BOOT__
	সংরচনা redir_struct *r, *rnext;
#endif
	পূর্ণাঙ্ক a, i, ret_code=0;
	ৱর্ণ *ind = indenter(indent);

#ifndef __U_BOOT__
	যদি (pi->stopped_progs > 0)
		ঘূৰাই ret_code;
	final_printf("%s run pipe: (pid %d)\n",ind,getpid());
#endif
	বাবে (i=0; i<pi->অঙ্ক_progs; i++) {
		child = &pi->progs[i];
		final_printf("%s  command %d:\n",ind,i);
		যদি (child->argv) {
			বাবে (a=0,p=child->argv; *p; a++,p++) {
				final_printf("%s   argv[%d] = %s\n",ind,a,*p);
			}
#ifndef __U_BOOT__
			globfree(&child->glob_result);
#অন্যথা
			বাবে (a = 0; a < child->argc; a++) {
				মুক্ত(child->argv[a]);
			}
			মুক্ত(child->argv);
			মুক্ত(child->argv_nonnull);
			child->argc = 0;
#endif
			child->argv=রিক্ত;
		} অন্যথা যদি (child->group) {
#ifndef __U_BOOT__
			final_printf("%s   begin group (subshell:%d)\n",ind, child->subshell);
#endif
			ret_code = free_pipe_list(child->group,indent+3);
			final_printf("%s   end group\n",ind);
		} অন্যথা {
			final_printf("%s   (nil)\n",ind);
		}
#ifndef __U_BOOT__
		বাবে (r=child->redirects; r; r=rnext) {
			final_printf("%s   redirect %d%s", ind, r->fd, redir_table[r->type].descrip);
			যদি (r->dup == -1) {
				/* guard against the case >$FOO, where foo is unset or blank */
				যদি (r->word.gl_pathv) {
					final_printf(" %s\n", *r->word.gl_pathv);
					globfree(&r->word);
				}
			} অন্যথা {
				final_printf("&%d\n", r->dup);
			}
			rnext=r->next;
			মুক্ত(r);
		}
		child->redirects=রিক্ত;
#endif
	}
	মুক্ত(pi->progs);   /* children are an array, they get freed all at once */
	pi->progs=রিক্ত;
	ঘূৰাই ret_code;
}

স্থির পূর্ণাঙ্ক free_pipe_list(সংরচনা pipe *head, পূর্ণাঙ্ক indent)
{
	পূর্ণাঙ্ক rcode=0;   /* if list has no members */
	সংরচনা pipe *pi, *next;
	ৱর্ণ *ind = indenter(indent);
	বাবে (pi=head; pi; pi=next) {
		final_printf("%s pipe reserved mode %d\n", ind, pi->r_mode);
		rcode = free_pipe(pi, indent);
		final_printf("%s pipe followup code %d\n", ind, pi->followup);
		next=pi->next;
		pi->next=রিক্ত;
		মুক্ত(pi);
	}
	ঘূৰাই rcode;
}

/* Select which version we will use */
স্থির পূর্ণাঙ্ক run_list(সংরচনা pipe *pi)
{
	পূর্ণাঙ্ক rcode=0;
#ifndef __U_BOOT__
	যদি (fake_mode==0) {
#endif
		rcode = run_list_real(pi);
#ifndef __U_BOOT__
	}
#endif
	/* free_pipe_list has the side effect of clearing memory
	 * In the long run that function can be merged with run_list_real,
	 * but doing that now would hobble the debugging effort. */
	free_pipe_list(pi,0);
	ঘূৰাই rcode;
}

/* The API for glob is arguably broken.  This routine pushes a non-matching
 * string into the output structure, removing non-backslashed backslashes.
 * If someone can prove me wrong, by performing this function within the
 * original glob(3) api, feel free to rewrite this routine into oblivion.
 * Return code (0 vs. GLOB_NOSPACE) matches glob(3).
 * XXX broken if the last character is '\\', check that before calling.
 */
#ifndef __U_BOOT__
স্থির পূর্ণাঙ্ক globhack(ধ্রুৱ ৱর্ণ *src, পূর্ণাঙ্ক flags, glob_t *pglob)
{
	পূর্ণাঙ্ক cnt=0, pathc;
	ধ্রুৱ ৱর্ণ *s;
	ৱর্ণ *dest;
	বাবে (cnt=1, s=src; s && *s; s++) {
		যদি (*s == '\\') s++;
		cnt++;
	}
	dest = malloc(cnt);
	যদি (!dest) ঘূৰাই GLOB_NOSPACE;
	যদি (!(flags & GLOB_APPEND)) {
		pglob->gl_pathv=রিক্ত;
		pglob->gl_pathc=0;
		pglob->gl_offs=0;
		pglob->gl_offs=0;
	}
	pathc = ++pglob->gl_pathc;
	pglob->gl_pathv = realloc(pglob->gl_pathv, (pathc+1)*পরিমাণ(*pglob->gl_pathv));
	যদি (pglob->gl_pathv == রিক্ত) ঘূৰাই GLOB_NOSPACE;
	pglob->gl_pathv[pathc-1]=dest;
	pglob->gl_pathv[pathc]=রিক্ত;
	বাবে (s=src; s && *s; s++, dest++) {
		যদি (*s == '\\') s++;
		*dest = *s;
	}
	*dest='\0';
	ঘূৰাই 0;
}

/* XXX broken if the last character is '\\', check that before calling */
স্থির পূর্ণাঙ্ক glob_needed(ধ্রুৱ ৱর্ণ *s)
{
	বাবে (; *s; s++) {
		যদি (*s == '\\') s++;
		যদি (strchr("*[?",*s)) ঘূৰাই 1;
	}
	ঘূৰাই 0;
}

#যদি 0
স্থির শূন্য globprint(glob_t *pglob)
{
	পূর্ণাঙ্ক i;
	debug_printf("glob_t at %p:\n", pglob);
	debug_printf("  gl_pathc=%d  gl_pathv=%p  gl_offs=%d  gl_flags=%d\n",
		pglob->gl_pathc, pglob->gl_pathv, pglob->gl_offs, pglob->gl_flags);
	বাবে (i=0; i<pglob->gl_pathc; i++)
		debug_printf("pglob->gl_pathv[%d] = %p = %s\n", i,
			pglob->gl_pathv[i], pglob->gl_pathv[i]);
}
#endif

স্থির পূর্ণাঙ্ক xglob(o_string *dest, পূর্ণাঙ্ক flags, glob_t *pglob)
{
	পূর্ণাঙ্ক gr;

	/* short-circuit for null word */
	/* we can code this better when the debug_printf's are gone */
	যদি (dest->length == 0) {
		যদি (dest->nonnull) {
			/* bash man page calls this an "explicit" null */
			gr = globhack(dest->data, flags, pglob);
			debug_printf("globhack returned %d\n",gr);
		} অন্যথা {
			ঘূৰাই 0;
		}
	} অন্যথা যদি (glob_needed(dest->data)) {
		gr = glob(dest->data, flags, রিক্ত, pglob);
		debug_printf("glob returned %d\n",gr);
		যদি (gr == GLOB_NOMATCH) {
			/* quote removal, or more accurately, backslash removal */
			gr = globhack(dest->data, flags, pglob);
			debug_printf("globhack returned %d\n",gr);
		}
	} অন্যথা {
		gr = globhack(dest->data, flags, pglob);
		debug_printf("globhack returned %d\n",gr);
	}
	যদি (gr == GLOB_NOSPACE)
		error_বাৰ্তা_and_die("out of memory during glob");
	যদি (gr != 0) { /* GLOB_ABORTED ? */
		error_বাৰ্তা("glob(3) error %d",gr);
	}
	/* globprint(glob_target); */
	ঘূৰাই gr;
}
#endif

#ifdef __U_BOOT__
স্থির ৱর্ণ *get_dollar_var(ৱর্ণ ch);
#endif

/* This is used to get/check local shell variables */
ৱর্ণ *get_local_var(ধ্রুৱ ৱর্ণ *s)
{
	সংরচনা variables *cur;

	যদি (!s)
		ঘূৰাই রিক্ত;

#ifdef __U_BOOT__
	যদি (*s == '$')
		ঘূৰাই get_dollar_var(s[1]);
#endif

	বাবে (cur = শীৰ্ষ_vars; cur; cur=cur->next)
		যদি(strcmp(cur->name, s)==0)
			ঘূৰাই cur->মান;
	ঘূৰাই রিক্ত;
}

/* This is used to set local shell variables
   flg_export==0 if only local (not exporting) variable
   flg_export==1 if "new" exporting environ
   flg_export>1  if current startup environ (not call putenv()) */
পূর্ণাঙ্ক set_local_var(ধ্রুৱ ৱর্ণ *s, পূর্ণাঙ্ক flg_export)
{
	ৱর্ণ *name, *মান;
	পূর্ণাঙ্ক result=0;
	সংরচনা variables *cur;

#ifdef __U_BOOT__
	/* might be possible! */
	যদি (!isalpha(*s))
		ঘূৰাই -1;
#endif

	name=strdup(s);

	/* Assume when we enter this function that we are already in
	 * NAME=VALUE format.  So the first order of business is to
	 * split 's' on the '=' into 'name' and 'value' */
	মান = strchr(name, '=');
	যদি (!মান) {
		মুক্ত(name);
		ঘূৰাই -1;
	}
	*মান++ = 0;

	যদি (!*মান) {
		unset_local_var(name);
		মুক্ত(name);
		ঘূৰাই 0;
	}

	বাবে(cur = শীৰ্ষ_vars; cur; cur = cur->next) {
		যদি(strcmp(cur->name, name)==0)
			ভঙ্গ;
	}

	যদি(cur) {
		যদি(strcmp(cur->মান, মান)==0) {
			যদি(flg_export>0 && cur->flg_export==0)
				cur->flg_export=flg_export;
			অন্যথা
				result++;
		} অন্যথা {
			যদি(cur->flg_read_only) {
				error_বাৰ্তা("%s: readonly variable", name);
				result = -1;
			} অন্যথা {
				যদি(flg_export>0 || cur->flg_export>1)
					cur->flg_export=1;
				মুক্ত(cur->মান);

				cur->মান = strdup(মান);
			}
		}
	} অন্যথা {
		cur = malloc(পরিমাণ(সংরচনা variables));
		যদি(!cur) {
			result = -1;
		} অন্যথা {
			cur->name = strdup(name);
			যদি (cur->name == রিক্ত) {
				মুক্ত(cur);
				result = -1;
			} অন্যথা {
				সংরচনা variables *bottom = শীৰ্ষ_vars;
				cur->মান = strdup(মান);
				cur->next = রিক্ত;
				cur->flg_export = flg_export;
				cur->flg_read_only = 0;
				যেতিয়ালৈ(bottom->next) bottom=bottom->next;
				bottom->next = cur;
			}
		}
	}

#ifndef __U_BOOT__
	যদি(result==0 && cur->flg_export==1) {
		*(মান-1) = '=';
		result = putenv(name);
	} অন্যথা {
#endif
		মুক্ত(name);
#ifndef __U_BOOT__
		যদি(result>0)            /* equivalent to previous set */
			result = 0;
	}
#endif
	ঘূৰাই result;
}

শূন্য unset_local_var(ধ্রুৱ ৱর্ণ *name)
{
	সংরচনা variables *cur;

	যদি (name) {
		বাবে (cur = শীৰ্ষ_vars; cur; cur=cur->next) {
			যদি(strcmp(cur->name, name)==0)
				ভঙ্গ;
		}
		যদি (cur != রিক্ত) {
			সংরচনা variables *next = শীৰ্ষ_vars;
			যদি(cur->flg_read_only) {
				error_বাৰ্তা("%s: readonly variable", name);
				ঘূৰাই;
			} অন্যথা {
#ifndef __U_BOOT__
				যদি(cur->flg_export)
					unenv_set(cur->name);
#endif
				মুক্ত(cur->name);
				মুক্ত(cur->মান);
				যেতিয়ালৈ (next->next != cur)
					next = next->next;
				next->next = cur->next;
			}
			মুক্ত(cur);
		}
	}
}

স্থির পূর্ণাঙ্ক is_assignment(ধ্রুৱ ৱর্ণ *s)
{
	যদি (s == রিক্ত)
		ঘূৰাই 0;

	যদি (!isalpha(*s)) ঘূৰাই 0;
	++s;
	যেতিয়ালৈ(isalnum(*s) || *s=='_') ++s;
	ঘূৰাই *s=='=';
}

#ifndef __U_BOOT__
/* the src parameter allows us to peek forward to a possible &n syntax
 * for file descriptor duplication, e.g., "2>&1".
 * Return code is 0 normally, 1 if a syntax error is detected in src.
 * Resource errors (in xmalloc) cause the process to exit */
স্থির পূর্ণাঙ্ক setup_redirect(সংরচনা p_context *ctx, পূর্ণাঙ্ক fd, redir_type style,
	সংরচনা in_str *প্ৰবেশ)
{
	সংরচনা child_prog *child=ctx->child;
	সংরচনা redir_struct *redir = child->redirects;
	সংরচনা redir_struct *last_redir=রিক্ত;

	/* Create a new redir_struct and drop it onto the end of the linked list */
	যেতিয়ালৈ(redir) {
		last_redir=redir;
		redir=redir->next;
	}
	redir = xmalloc(পরিমাণ(সংরচনা redir_struct));
	redir->next=রিক্ত;
	redir->word.gl_pathv=রিক্ত;
	যদি (last_redir) {
		last_redir->next=redir;
	} অন্যথা {
		child->redirects=redir;
	}

	redir->type=style;
	redir->fd= (fd==-1) ? redir_table[style].default_fd : fd ;

	debug_printf("Redirect type %d%s\n", redir->fd, redir_table[style].descrip);

	/* Check for a '2>&1' type redirect */
	redir->dup = redirect_dup_অঙ্ক(প্ৰবেশ);
	যদি (redir->dup == -2) ঘূৰাই 1;  /* syntax error */
	যদি (redir->dup != -1) {
		/* Erik had a check here that the file descriptor in question
		 * is legit; I postpone that to "run time"
		 * A "-" representation of "close me" shows up as a -3 here */
		debug_printf("Duplicating redirect '%d>&%d'\n", redir->fd, redir->dup);
	} অন্যথা {
		/* We do _not_ try to open the file that src points to,
		 * since we need to return and let src be expanded first.
		 * Set ctx->pending_redirect, so we know what to do at the
		 * end of the next parsed word.
		 */
		ctx->pending_redirect = redir;
	}
	ঘূৰাই 0;
}
#endif

স্থির সংরচনা pipe *new_pipe(শূন্য)
{
	সংরচনা pipe *pi;
	pi = xmalloc(পরিমাণ(সংরচনা pipe));
	pi->অঙ্ক_progs = 0;
	pi->progs = রিক্ত;
	pi->next = রিক্ত;
	pi->followup = 0;  /* invalid */
	pi->r_mode = RES_NONE;
	ঘূৰাই pi;
}

স্থির শূন্য initialize_context(সংরচনা p_context *ctx)
{
	ctx->pipe=রিক্ত;
#ifndef __U_BOOT__
	ctx->pending_redirect=রিক্ত;
#endif
	ctx->child=রিক্ত;
	ctx->list_head=new_pipe();
	ctx->pipe=ctx->list_head;
	ctx->w=RES_NONE;
	ctx->স্তূপ=রিক্ত;
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
সংরচনা reserved_combo {
	ৱর্ণ *literal;
	পূর্ণাঙ্ক code;
	long flag;
};
/* Mostly a list of accepted follow-up reserved words.
 * FLAG_END means we are done with the sequence, and are ready
 * to turn the compound list into a command.
 * FLAG_START means the word must start a new compound list.
 */
স্থির সংরচনা reserved_combo reserved_list[] = {
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
#define NRES (পরিমাণ(reserved_list)/পরিমাণ(সংরচনা reserved_combo))

স্থির পূর্ণাঙ্ক reserved_word(o_string *dest, সংরচনা p_context *ctx)
{
	সংরচনা reserved_combo *r;
	বাবে (r=reserved_list;
		r<reserved_list+NRES; r++) {
		যদি (strcmp(dest->data, r->literal) == 0) {
			debug_printf("found reserved word %s, code %d\n",r->literal,r->code);
			যদি (r->flag & FLAG_START) {
				সংরচনা p_context *new = xmalloc(পরিমাণ(সংরচনা p_context));
				debug_printf("push stack\n");
				যদি (ctx->w == RES_IN || ctx->w == RES_FOR) {
					syntax();
					মুক্ত(new);
					ctx->w = RES_SNTX;
					b_reset(dest);
					ঘূৰাই 1;
				}
				*new = *ctx;   /* physical copy */
				initialize_context(ctx);
				ctx->স্তূপ=new;
			} অন্যথা যদি ( ctx->w == RES_NONE || ! (ctx->old_flag & (1<<r->code))) {
				syntax();
				ctx->w = RES_SNTX;
				b_reset(dest);
				ঘূৰাই 1;
			}
			ctx->w=r->code;
			ctx->old_flag = r->flag;
			যদি (ctx->old_flag & FLAG_END) {
				সংরচনা p_context *old;
				debug_printf("pop stack\n");
				done_pipe(ctx,PIPE_SEQ);
				old = ctx->স্তূপ;
				old->child->group = ctx->list_head;
#ifndef __U_BOOT__
				old->child->subshell = 0;
#endif
				*ctx = *old;   /* physical copy */
				মুক্ত(old);
			}
			b_reset (dest);
			ঘূৰাই 1;
		}
	}
	ঘূৰাই 0;
}

/* normal return is 0.
 * Syntax or xglob errors return 1. */
স্থির পূর্ণাঙ্ক done_word(o_string *dest, সংরচনা p_context *ctx)
{
	সংরচনা child_prog *child=ctx->child;
#ifndef __U_BOOT__
	glob_t *glob_target;
	পূর্ণাঙ্ক gr, flags = 0;
#অন্যথা
	ৱর্ণ *str, *s;
	পূর্ণাঙ্ক argc, cnt;
#endif

	debug_printf("done_word: %s %p\n", dest->data, child);
	যদি (dest->length == 0 && !dest->nonnull) {
		debug_printf("  true null, ignored\n");
		ঘূৰাই 0;
	}
#ifndef __U_BOOT__
	যদি (ctx->pending_redirect) {
		glob_target = &ctx->pending_redirect->word;
	} অন্যথা {
#endif
		যদি (child->group) {
			syntax();
			ঘূৰাই 1;  /* syntax error, groups and arglists don't mix */
		}
		যদি (!child->argv && (ctx->type & FLAG_PARSE_SEMICOLON)) {
			debug_printf("checking %s for reserved-ness\n",dest->data);
			যদি (reserved_word(dest,ctx)) ঘূৰাই ctx->w==RES_SNTX;
		}
#ifndef __U_BOOT__
		glob_target = &child->glob_result;
		যদি (child->argv) flags |= GLOB_APPEND;
#অন্যথা
		বাবে (cnt = 1, s = dest->data; s && *s; s++) {
			যদি (*s == '\\') s++;
			cnt++;
		}
		str = malloc(cnt);
		যদি (!str) ঘূৰাই 1;
		যদি ( child->argv == রিক্ত) {
			child->argc=0;
		}
		argc = ++child->argc;
		child->argv = realloc(child->argv, (argc+1)*পরিমাণ(*child->argv));
		যদি (child->argv == রিক্ত) {
			মুক্ত(str);
			ঘূৰাই 1;
		}
		child->argv_nonnull = realloc(child->argv_nonnull,
					(argc+1)*পরিমাণ(*child->argv_nonnull));
		যদি (child->argv_nonnull == রিক্ত) {
			মুক্ত(str);
			ঘূৰাই 1;
		}
		child->argv[argc-1]=str;
		child->argv_nonnull[argc-1] = dest->nonnull;
		child->argv[argc]=রিক্ত;
		child->argv_nonnull[argc] = 0;
		বাবে (s = dest->data; s && *s; s++,str++) {
			যদি (*s == '\\') s++;
			*str = *s;
		}
		*str = '\0';
#endif
#ifndef __U_BOOT__
	}
	gr = xglob(dest, flags, glob_target);
	যদি (gr != 0) ঘূৰাই 1;
#endif

	b_reset(dest);
#ifndef __U_BOOT__
	যদি (ctx->pending_redirect) {
		ctx->pending_redirect=রিক্ত;
		যদি (glob_target->gl_pathc != 1) {
			error_বাৰ্তা("ambiguous redirect");
			ঘূৰাই 1;
		}
	} অন্যথা {
		child->argv = glob_target->gl_pathv;
	}
#endif
	যদি (ctx->w == RES_FOR) {
		done_word(dest,ctx);
		done_pipe(ctx,PIPE_SEQ);
	}
	ঘূৰাই 0;
}

/* The only possible error here is out of memory, in which case
 * xmalloc exits. */
স্থির পূর্ণাঙ্ক done_command(সংরচনা p_context *ctx)
{
	/* The child is really already in the pipe structure, so
	 * advance the pipe counter and make a new, null child.
	 * Only real trickiness here is that the uncommitted
	 * child structure, to which ctx->child points, is not
	 * counted in pi->num_progs. */
	সংরচনা pipe *pi=ctx->pipe;
	সংরচনা child_prog *prog=ctx->child;

	যদি (prog && prog->group == রিক্ত
		 && prog->argv == রিক্ত
#ifndef __U_BOOT__
		 && prog->redirects == রিক্ত) {
#অন্যথা
										) {
#endif
		debug_printf("done_command: skipping null command\n");
		ঘূৰাই 0;
	} অন্যথা যদি (prog) {
		pi->অঙ্ক_progs++;
		debug_printf("done_command: num_progs incremented to %d\n",pi->অঙ্ক_progs);
	} অন্যথা {
		debug_printf("done_command: initializing\n");
	}
	pi->progs = xrealloc(pi->progs, পরিমাণ(*pi->progs) * (pi->অঙ্ক_progs+1));

	prog = pi->progs + pi->অঙ্ক_progs;
#ifndef __U_BOOT__
	prog->redirects = রিক্ত;
#endif
	prog->argv = রিক্ত;
	prog->argv_nonnull = রিক্ত;
#ifndef __U_BOOT__
	prog->is_stopped = 0;
#endif
	prog->group = রিক্ত;
#ifndef __U_BOOT__
	prog->glob_result.gl_pathv = রিক্ত;
	prog->family = pi;
#endif
	prog->sp = 0;
	ctx->child = prog;
	prog->type = ctx->type;

	/* but ctx->pipe and ctx->list_head remain unchanged */
	ঘূৰাই 0;
}

স্থির পূর্ণাঙ্ক done_pipe(সংরচনা p_context *ctx, pipe_style type)
{
	সংরচনা pipe *new_p;
	done_command(ctx);  /* implicit closure of previous command */
	debug_printf("done_pipe, type %d\n", type);
	ctx->pipe->followup = type;
	ctx->pipe->r_mode = ctx->w;
	new_p=new_pipe();
	ctx->pipe->next = new_p;
	ctx->pipe = new_p;
	ctx->child = রিক্ত;
	done_command(ctx);  /* set up new pipe to accept commands */
	ঘূৰাই 0;
}

#ifndef __U_BOOT__
/* peek ahead in the in_str to find out if we have a "&n" construct,
 * as in "2>&1", that represents duplicating a file descriptor.
 * returns either -2 (syntax error), -1 (no &), or the number found.
 */
স্থির পূর্ণাঙ্ক redirect_dup_অঙ্ক(সংরচনা in_str *প্ৰবেশ)
{
	পূর্ণাঙ্ক ch, d=0, ok=0;
	ch = b_peek(প্ৰবেশ);
	যদি (ch != '&') ঘূৰাই -1;

	b_getch(প্ৰবেশ);  /* get the & */
	ch=b_peek(প্ৰবেশ);
	যদি (ch == '-') {
		b_getch(প্ৰবেশ);
		ঘূৰাই -3;  /* "-" represents "close me" */
	}
	যেতিয়ালৈ (isdigit(ch)) {
		d = d*10+(ch-'0');
		ok=1;
		b_getch(প্ৰবেশ);
		ch = b_peek(প্ৰবেশ);
	}
	যদি (ok) ঘূৰাই d;

	error_বাৰ্তা("ambiguous redirect");
	ঘূৰাই -2;
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
স্থির পূর্ণাঙ্ক redirect_opt_অঙ্ক(o_string *o)
{
	পূর্ণাঙ্ক অঙ্ক;

	যদি (o->length==0) ঘূৰাই -1;
	বাবে(অঙ্ক=0; অঙ্ক<o->length; অঙ্ক++) {
		যদি (!isdigit(*(o->data+অঙ্ক))) {
			ঘূৰাই -1;
		}
	}
	/* reuse num (and save an int) */
	অঙ্ক=atoi(o->data);
	b_reset(o);
	ঘূৰাই অঙ্ক;
}

FILE *generate_stream_from_list(সংরচনা pipe *head)
{
	FILE *pf;
#যদি 1
	পূর্ণাঙ্ক pid, channel[2];
	যদি (pipe(channel)<0) perror_বাৰ্তা_and_die("pipe");
	pid=fork();
	যদি (pid<0) {
		perror_বাৰ্তা_and_die("fork");
	} অন্যথা যদি (pid==0) {
		close(channel[0]);
		যদি (channel[1] != 1) {
			dup2(channel[1],1);
			close(channel[1]);
		}
#যদি 0
#define SURROGATE "surrogate response"
		লিখ(1,SURROGATE,পরিমাণ(SURROGATE));
		_exit(run_list(head));
#অন্যথা
		_exit(run_list_real(head));   /* leaks memory */
#endif
	}
	debug_printf("forked child %d\n",pid);
	close(channel[1]);
	pf = fdopen(channel[0],"r");
	debug_printf("pipe on FILE *%p\n",pf);
#অন্যথা
	free_pipe_list(head,0);
	pf=popen("echo surrogate response","r");
	debug_printf("started fake pipe on FILE *%p\n",pf);
#endif
	ঘূৰাই pf;
}

/* this version hacked for testing purposes */
/* return code is exit status of the process that is run. */
স্থির পূর্ণাঙ্ক process_command_subs(o_string *dest, সংরচনা p_context *ctx, সংরচনা in_str *প্ৰবেশ, পূর্ণাঙ্ক subst_end)
{
	পূর্ণাঙ্ক retcode;
	o_string result=NULL_O_STRING;
	সংরচনা p_context inner;
	FILE *p;
	সংরচনা in_str pipe_str;
	initialize_context(&inner);

	/* recursion to generate command */
	retcode = parse_stream(&result, &inner, প্ৰবেশ, subst_end);
	যদি (retcode != 0) ঘূৰাই retcode;  /* syntax error or EOF */
	done_word(&result, &inner);
	done_pipe(&inner, PIPE_SEQ);
	b_free(&result);

	p=generate_stream_from_list(inner.list_head);
	যদি (p==রিক্ত) ঘূৰাই 1;
	mark_open(fileno(p));
	setup_file_in_str(&pipe_str, p);

	/* now send results of command back into original context */
	retcode = parse_stream(dest, ctx, &pipe_str, '\0');
	/* XXX In case of a syntax error, should we try to kill the child?
	 * That would be tough to do right, so just read until EOF. */
	যদি (retcode == 1) {
		যেতিয়ালৈ (b_getch(&pipe_str)!=EOF) { /* discard */ };
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
	ঘূৰাই retcode;
}

স্থির পূর্ণাঙ্ক parse_group(o_string *dest, সংরচনা p_context *ctx,
	সংরচনা in_str *প্ৰবেশ, পূর্ণাঙ্ক ch)
{
	পূর্ণাঙ্ক rcode, endch=0;
	সংরচনা p_context sub;
	সংরচনা child_prog *child = ctx->child;
	যদি (child->argv) {
		syntax();
		ঘূৰাই 1;  /* syntax error, groups and arglists don't mix */
	}
	initialize_context(&sub);
	ৱিকল্প(ch) {
		পক্ষ '(': endch=')'; child->subshell=1; ভঙ্গ;
		পক্ষ '{': endch='}'; ভঙ্গ;
		স্ৱভাৱ: syntax();   /* really logic error */
	}
	rcode=parse_stream(dest,&sub,প্ৰবেশ,endch);
	done_word(dest,&sub); /* finish off the final word in the subcontext */
	done_pipe(&sub, PIPE_SEQ);  /* and the final command there, too */
	child->group = sub.list_head;
	ঘূৰাই rcode;
	/* child remains "open", available for possible redirects */
}
#endif

/* basically useful version until someone wants to get fancier,
 * see the bash man page under "Parameter Expansion" */
স্থির ৱর্ণ *lookup_param(ৱর্ণ *src)
{
	ৱর্ণ *p;
	ৱর্ণ *sep;
	ৱর্ণ *default_val = রিক্ত;
	পূর্ণাঙ্ক assign = 0;
	পূর্ণাঙ্ক expand_empty = 0;

	যদি (!src)
		ঘূৰাই রিক্ত;

	sep = strchr(src, ':');

	যদি (sep) {
		*sep = '\0';
		যদি (*(sep + 1) == '-')
			default_val = sep+2;
		যদি (*(sep + 1) == '=') {
			default_val = sep+2;
			assign = 1;
		}
		যদি (*(sep + 1) == '+') {
			default_val = sep+2;
			expand_empty = 1;
		}
	}

	p = env_get(src);
	যদি (!p)
		p = get_local_var(src);

	যদি (!p || strlen(p) == 0) {
		p = default_val;
		যদি (assign) {
			ৱর্ণ *var = malloc(strlen(src)+strlen(default_val)+2);
			যদি (var) {
				sprintf(var, "%s=%s", src, default_val);
				set_local_var(var, 0);
			}
			মুক্ত(var);
		}
	} অন্যথা যদি (expand_empty) {
		p += strlen(p);
	}

	যদি (sep)
		*sep = ':';

	ঘূৰাই p;
}

#ifdef __U_BOOT__
স্থির ৱর্ণ *get_dollar_var(ৱর্ণ ch)
{
	স্থির ৱর্ণ buf[40];

	buf[0] = '\0';
	ৱিকল্প (ch) {
		পক্ষ '?':
			sprintf(buf, "%u", (unsigned পূর্ণাঙ্ক)last_return_code);
			ভঙ্গ;
		স্ৱভাৱ:
			ঘূৰাই রিক্ত;
	}
	ঘূৰাই buf;
}
#endif

/* return code: 0 for OK, 1 for syntax error */
স্থির পূর্ণাঙ্ক handle_dollar(o_string *dest, সংরচনা p_context *ctx, সংরচনা in_str *প্ৰবেশ)
{
#ifndef __U_BOOT__
	পূর্ণাঙ্ক i, advance=0;
#অন্যথা
	পূর্ণাঙ্ক advance=0;
#endif
#ifndef __U_BOOT__
	ৱর্ণ sep[]=" ";
#endif
	পূর্ণাঙ্ক ch = প্ৰবেশ->peek(প্ৰবেশ);  /* first character after the $ */
	debug_printf("handle_dollar: ch=%c\n",ch);
	যদি (isalpha(ch)) {
		b_addchr(dest, SPECIAL_VAR_প্ৰতীক);
		ctx->child->sp++;
		যেতিয়ালৈ(ch=b_peek(প্ৰবেশ),isalnum(ch) || ch=='_') {
			b_getch(প্ৰবেশ);
			b_addchr(dest,ch);
		}
		b_addchr(dest, SPECIAL_VAR_প্ৰতীক);
#ifndef __U_BOOT__
	} অন্যথা যদি (isdigit(ch)) {
		i = ch-'0';  /* XXX is $0 special? */
		যদি (i<global_argc) {
			parse_string(dest, ctx, global_argv[i]); /* recursion */
		}
		advance = 1;
#endif
	} অন্যথা ৱিকল্প (ch) {
#ifndef __U_BOOT__
		পক্ষ '$':
			b_adduint(dest,getpid());
			advance = 1;
			ভঙ্গ;
		পক্ষ '!':
			যদি (last_bg_pid > 0) b_adduint(dest, last_bg_pid);
			advance = 1;
			ভঙ্গ;
#endif
		পক্ষ '?':
#ifndef __U_BOOT__
			b_adduint(dest,last_return_code);
#অন্যথা
			ctx->child->sp++;
			b_addchr(dest, SPECIAL_VAR_প্ৰতীক);
			b_addchr(dest, '$');
			b_addchr(dest, '?');
			b_addchr(dest, SPECIAL_VAR_প্ৰতীক);
#endif
			advance = 1;
			ভঙ্গ;
#ifndef __U_BOOT__
		পক্ষ '#':
			b_adduint(dest,global_argc ? global_argc-1 : 0);
			advance = 1;
			ভঙ্গ;
#endif
		পক্ষ '{':
			b_addchr(dest, SPECIAL_VAR_প্ৰতীক);
			ctx->child->sp++;
			b_getch(প্ৰবেশ);
			/* XXX maybe someone will try to escape the '}' */
			যেতিয়ালৈ(ch=b_getch(প্ৰবেশ),ch!=EOF && ch!='}') {
				b_addchr(dest,ch);
			}
			যদি (ch != '}') {
				syntax();
				ঘূৰাই 1;
			}
			b_addchr(dest, SPECIAL_VAR_প্ৰতীক);
			ভঙ্গ;
#ifndef __U_BOOT__
		পক্ষ '(':
			b_getch(প্ৰবেশ);
			process_command_subs(dest, ctx, প্ৰবেশ, ')');
			ভঙ্গ;
		পক্ষ '*':
			sep[0]=ifs[0];
			বাবে (i=1; i<global_argc; i++) {
				parse_string(dest, ctx, global_argv[i]);
				যদি (i+1 < global_argc) parse_string(dest, ctx, sep);
			}
			ভঙ্গ;
		পক্ষ '@':
		পক্ষ '-':
		পক্ষ '_':
			/* still unhandled, but should be eventually */
			error_বাৰ্তা("unhandled syntax: $%c",ch);
			ঘূৰাই 1;
			ভঙ্গ;
#endif
		স্ৱভাৱ:
			b_addqchr(dest,'$',dest->quote);
	}
	/* Eat the character if the flag was set.  If the compiler
	 * is smart enough, we could substitute "b_getch(input);"
	 * for all the "advance = 1;" above, and also end up with
	 * a nice size-optimized program.  Hah!  That'll be the day.
	 */
	যদি (advance) b_getch(প্ৰবেশ);
	ঘূৰাই 0;
}

#ifndef __U_BOOT__
পূর্ণাঙ্ক parse_string(o_string *dest, সংরচনা p_context *ctx, ধ্রুৱ ৱর্ণ *src)
{
	সংরচনা in_str foo;
	setup_string_in_str(&foo, src);
	ঘূৰাই parse_stream(dest, ctx, &foo, '\0');
}
#endif

/* return code is 0 for normal exit, 1 for syntax error */
স্থির পূর্ণাঙ্ক parse_stream(o_string *dest, সংরচনা p_context *ctx,
			সংরচনা in_str *প্ৰবেশ, পূর্ণাঙ্ক end_trigger)
{
	unsigned পূর্ণাঙ্ক ch, m;
#ifndef __U_BOOT__
	পূর্ণাঙ্ক redir_fd;
	redir_type redir_style;
#endif
	পূর্ণাঙ্ক next;

	/* Only double-quote state is handled in the state variable dest->quote.
	 * A single-quote triggers a bypass of the main loop until its mate is
	 * found.  When recursing, quote state is passed in via dest->quote. */

	debug_printf("parse_stream, end_trigger=%d\n",end_trigger);
	যেতিয়ালৈ ((ch=b_getch(প্ৰবেশ))!=EOF) {
		m = map[ch];
#ifdef __U_BOOT__
		যদি (প্ৰবেশ->__promptme == 0) ঘূৰাই 1;
#endif
		next = (ch == '\n') ? 0 : b_peek(প্ৰবেশ);

		debug_printf("parse_stream: ch=%c (%d) m=%d quote=%d - %c\n",
			ch >= ' ' ? ch : '.', ch, m,
			dest->quote, ctx->স্তূপ == রিক্ত ? '*' : '.');

		যদি (m==0 || ((m==1 || m==2) && dest->quote)) {
			b_addqchr(dest, ch, dest->quote);
		} অন্যথা {
			যদি (m==2) {  /* unquoted IFS */
				যদি (done_word(dest, ctx)) {
					ঘূৰাই 1;
				}
				/* If we aren't performing a substitution, treat a newline as a
				 * command separator.  */
				যদি (end_trigger != '\0' && ch=='\n')
					done_pipe(ctx,PIPE_SEQ);
			}
			যদি (ch == end_trigger && !dest->quote && ctx->w==RES_NONE) {
				debug_printf("leaving parse_stream (triggered)\n");
				ঘূৰাই 0;
			}
#যদি 0
			যদি (ch=='\n') {
				/* Yahoo!  Time to run with it! */
				done_pipe(ctx,PIPE_SEQ);
				run_list(ctx->list_head);
				initialize_context(ctx);
			}
#endif
			যদি (m!=2) ৱিকল্প (ch) {
		পক্ষ '#':
			যদি (dest->length == 0 && !dest->quote) {
				যেতিয়ালৈ(ch=b_peek(প্ৰবেশ),ch!=EOF && ch!='\n') { b_getch(প্ৰবেশ); }
			} অন্যথা {
				b_addqchr(dest, ch, dest->quote);
			}
			ভঙ্গ;
		পক্ষ '\\':
			যদি (next == EOF) {
				syntax();
				ঘূৰাই 1;
			}
			b_addqchr(dest, '\\', dest->quote);
			b_addqchr(dest, b_getch(প্ৰবেশ), dest->quote);
			ভঙ্গ;
		পক্ষ '$':
			যদি (handle_dollar(dest, ctx, প্ৰবেশ)!=0) ঘূৰাই 1;
			ভঙ্গ;
		পক্ষ '\'':
			dest->nonnull = 1;
			যেতিয়ালৈ(ch=b_getch(প্ৰবেশ),ch!=EOF && ch!='\'') {
#ifdef __U_BOOT__
				যদি(প্ৰবেশ->__promptme == 0) ঘূৰাই 1;
#endif
				b_addchr(dest,ch);
			}
			যদি (ch==EOF) {
				syntax();
				ঘূৰাই 1;
			}
			ভঙ্গ;
		পক্ষ '"':
			dest->nonnull = 1;
			dest->quote = !dest->quote;
			ভঙ্গ;
#ifndef __U_BOOT__
		পক্ষ '`':
			process_command_subs(dest, ctx, প্ৰবেশ, '`');
			ভঙ্গ;
		পক্ষ '>':
			redir_fd = redirect_opt_অঙ্ক(dest);
			done_word(dest, ctx);
			redir_style=REDIRECT_OVERWRITE;
			যদি (next == '>') {
				redir_style=REDIRECT_APPEND;
				b_getch(প্ৰবেশ);
			} অন্যথা যদি (next == '(') {
				syntax();   /* until we support >(list) Process Substitution */
				ঘূৰাই 1;
			}
			setup_redirect(ctx, redir_fd, redir_style, প্ৰবেশ);
			ভঙ্গ;
		পক্ষ '<':
			redir_fd = redirect_opt_অঙ্ক(dest);
			done_word(dest, ctx);
			redir_style=REDIRECT_প্ৰবেশ;
			যদি (next == '<') {
				redir_style=REDIRECT_HEREIS;
				b_getch(প্ৰবেশ);
			} অন্যথা যদি (next == '>') {
				redir_style=REDIRECT_IO;
				b_getch(প্ৰবেশ);
			} অন্যথা যদি (next == '(') {
				syntax();   /* until we support <(list) Process Substitution */
				ঘূৰাই 1;
			}
			setup_redirect(ctx, redir_fd, redir_style, প্ৰবেশ);
			ভঙ্গ;
#endif
		পক্ষ ';':
			done_word(dest, ctx);
			done_pipe(ctx,PIPE_SEQ);
			ভঙ্গ;
		পক্ষ '&':
			done_word(dest, ctx);
			যদি (next=='&') {
				b_getch(প্ৰবেশ);
				done_pipe(ctx,PIPE_AND);
			} অন্যথা {
#ifndef __U_BOOT__
				done_pipe(ctx,PIPE_BG);
#অন্যথা
				syntax_err();
				ঘূৰাই 1;
#endif
			}
			ভঙ্গ;
		পক্ষ '|':
			done_word(dest, ctx);
			যদি (next=='|') {
				b_getch(প্ৰবেশ);
				done_pipe(ctx,PIPE_OR);
			} অন্যথা {
				/* we could pick up a file descriptor choice here
				 * with redirect_opt_num(), but bash doesn't do it.
				 * "echo foo 2| cat" yields "foo 2". */
#ifndef __U_BOOT__
				done_command(ctx);
#অন্যথা
				syntax_err();
				ঘূৰাই 1;
#endif
			}
			ভঙ্গ;
#ifndef __U_BOOT__
		পক্ষ '(':
		পক্ষ '{':
			যদি (parse_group(dest, ctx, প্ৰবেশ, ch)!=0) ঘূৰাই 1;
			ভঙ্গ;
		পক্ষ ')':
		পক্ষ '}':
			syntax();   /* Proper use of this character caught by end_trigger */
			ঘূৰাই 1;
			ভঙ্গ;
#endif
		পক্ষ SUBSTED_VAR_প্ৰতীক:
			dest->nonnull = 1;
			যেতিয়ালৈ (ch = b_getch(প্ৰবেশ), ch != EOF &&
			    ch != SUBSTED_VAR_প্ৰতীক) {
				debug_printf("subst, pass=%d\n", ch);
				যদি (প্ৰবেশ->__promptme == 0)
					ঘূৰাই 1;
				b_addchr(dest, ch);
			}
			debug_printf("subst, term=%d\n", ch);
			যদি (ch == EOF) {
				syntax();
				ঘূৰাই 1;
			}
			ভঙ্গ;
		স্ৱভাৱ:
			syntax();   /* this is really an internal logic error */
			ঘূৰাই 1;
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
	যদি (end_trigger != '\0') ঘূৰাই -1;
	ঘূৰাই 0;
}

স্থির শূন্য mapset(ধ্রুৱ unsigned ৱর্ণ *set, পূর্ণাঙ্ক code)
{
	ধ্রুৱ unsigned ৱর্ণ *s;
	বাবে (s=set; *s; s++) map[*s] = code;
}

স্থির শূন্য update_ifs_map(শূন্য)
{
	/* char *ifs and char map[256] are both globals. */
	ifs = (uchar *)env_get("IFS");
	যদি (ifs == রিক্ত) ifs=(uchar *)" \t\n";
	/* Precompute a list of 'flow through' behavior so it can be treated
	 * quickly up front.  Computation is necessary because of IFS.
	 * Special case handling of IFS == " \t\n" is not implemented.
	 * The map[] array only really needs two bits each, and on most machines
	 * that would be faster because of the reduced L1 cache footprint.
	 */
	memset(map,0,পরিমাণ(map)); /* most characters flow through always */
#ifndef __U_BOOT__
	mapset((uchar *)"\\$'\"`", 3);      /* never flow through */
	mapset((uchar *)"<>;&|(){}#", 1);   /* flow through if quoted */
#অন্যথা
	{
		uchar subst[2] = {SUBSTED_VAR_প্ৰতীক, 0};
		mapset(subst, 3);       /* never flow through */
	}
	mapset((uchar *)"\\$'\"", 3);       /* never flow through */
	mapset((uchar *)";&|#", 1);         /* flow through if quoted */
#endif
	mapset(ifs, 2);            /* also flow through if quoted */
}

/* most recursion does not come through here, the exeception is
 * from builtin_source() */
স্থির পূর্ণাঙ্ক parse_stream_outer(সংরচনা in_str *inp, পূর্ণাঙ্ক flag)
{

	সংরচনা p_context ctx;
	o_string temp=NULL_O_STRING;
	পূর্ণাঙ্ক rcode;
#ifdef __U_BOOT__
	পূর্ণাঙ্ক code = 1;
#endif
	কৰা {
		ctx.type = flag;
		initialize_context(&ctx);
		update_ifs_map();
		যদি (!(flag & FLAG_PARSE_SEMICOLON) || (flag & FLAG_REPARSING)) mapset((uchar *)";$&|", 0);
		inp->promptmode=1;
		rcode = parse_stream(&temp, &ctx, inp,
				     flag & FLAG_CONT_ON_NEWLINE ? -1 : '\n');
#ifdef __U_BOOT__
		যদি (rcode == 1) flag_repeat = 0;
#endif
		যদি (rcode != 1 && ctx.old_flag != 0) {
			syntax();
#ifdef __U_BOOT__
			flag_repeat = 0;
#endif
		}
		যদি (rcode != 1 && ctx.old_flag == 0) {
			done_word(&temp, &ctx);
			done_pipe(&ctx,PIPE_SEQ);
#ifndef __U_BOOT__
			run_list(ctx.list_head);
#অন্যথা
			code = run_list(ctx.list_head);
			যদি (code == -2) {	/* exit */
				b_free(&temp);
				code = 0;
				/* XXX hackish way to not allow exit from main loop */
				যদি (inp->peek == file_peek) {
					printf("exit not allowed from main input shell.\n");
					অনুৱর্ত;
				}
				/*
				 * DANGER
				 * Return code -2 is special in this context,
				 * it indicates exit from inner pipe instead
				 * of return code itself, the return code is
				 * stored in 'last_return_code' variable!
				 * DANGER
				 */
				ঘূৰাই -2;
			}
			যদি (code == -1)
			    flag_repeat = 0;
#endif
		} অন্যথা {
			যদি (ctx.old_flag != 0) {
				মুক্ত(ctx.স্তূপ);
				b_reset(&temp);
			}
#ifdef __U_BOOT__
			যদি (inp->__promptme == 0) printf("<INTERRUPT>\n");
			inp->__promptme = 1;
#endif
			temp.nonnull = 0;
			temp.quote = 0;
			inp->p = রিক্ত;
			free_pipe_list(ctx.list_head,0);
		}
		b_free(&temp);
	/* loop on syntax errors, return on EOF */
	} যেতিয়ালৈ (rcode != -1 && !(flag & FLAG_EXIT_FROM_LOOP) &&
		(inp->peek != static_peek || b_peek(inp)));
#ifndef __U_BOOT__
	ঘূৰাই 0;
#অন্যথা
	ঘূৰাই (code != 0) ? 1 : 0;
#endif /* __U_BOOT__ */
}

#ifndef __U_BOOT__
স্থির পূর্ণাঙ্ক parse_string_outer(ধ্রুৱ ৱর্ণ *s, পূর্ণাঙ্ক flag)
#অন্যথা
পূর্ণাঙ্ক parse_string_outer(ধ্রুৱ ৱর্ণ *s, পূর্ণাঙ্ক flag)
#endif	/* __U_BOOT__ */
{
	সংরচনা in_str প্ৰবেশ;
	পূর্ণাঙ্ক rcode;
#ifdef __U_BOOT__
	ৱর্ণ *p = রিক্ত;
	যদি (!s)
		ঘূৰাই 1;
	যদি (!*s)
		ঘূৰাই 0;
	যদি (!(p = strchr(s, '\n')) || *++p) {
		p = xmalloc(strlen(s) + 2);
		strcpy(p, s);
		strcat(p, "\n");
		setup_string_in_str(&প্ৰবেশ, p);
		rcode = parse_stream_outer(&প্ৰবেশ, flag);
		মুক্ত(p);
		ঘূৰাই rcode == -2 ? last_return_code : rcode;
	} অন্যথা {
#endif
	setup_string_in_str(&প্ৰবেশ, s);
	rcode = parse_stream_outer(&প্ৰবেশ, flag);
	ঘূৰাই rcode == -2 ? last_return_code : rcode;
#ifdef __U_BOOT__
	}
#endif
}

#ifndef __U_BOOT__
স্থির পূর্ণাঙ্ক parse_file_outer(FILE *f)
#অন্যথা
পূর্ণাঙ্ক parse_file_outer(শূন্য)
#endif
{
	পূর্ণাঙ্ক rcode;
	সংরচনা in_str প্ৰবেশ;
#ifndef __U_BOOT__
	setup_file_in_str(&প্ৰবেশ, f);
#অন্যথা
	setup_file_in_str(&প্ৰবেশ);
#endif
	rcode = parse_stream_outer(&প্ৰবেশ, FLAG_PARSE_SEMICOLON);
	ঘূৰাই rcode == -2 ? last_return_code : rcode;
}

#ifdef __U_BOOT__
পূর্ণাঙ্ক u_boot_hush_start(শূন্য)
{
	যদি (শীৰ্ষ_vars == রিক্ত) {
		শীৰ্ষ_vars = malloc(পরিমাণ(সংরচনা variables));
		শীৰ্ষ_vars->name = "HUSH_VERSION";
		শীৰ্ষ_vars->মান = "0.01";
		শীৰ্ষ_vars->next = রিক্ত;
		শীৰ্ষ_vars->flg_export = 0;
		শীৰ্ষ_vars->flg_read_only = 1;
	}
	ঘূৰাই 0;
}

স্থির শূন্য *xmalloc(size_t size)
{
	শূন্য *p = রিক্ত;

	যদি (!(p = malloc(size))) {
	    printf("ERROR : xmalloc failed\n");
	    বাবে(;;);
	}
	ঘূৰাই p;
}

স্থির শূন্য *xrealloc(শূন্য *ptr, size_t size)
{
	শূন্য *p = রিক্ত;

	যদি (!(p = realloc(ptr, size))) {
	    printf("ERROR : xrealloc failed\n");
	    বাবে(;;);
	}
	ঘূৰাই p;
}
#endif /* __U_BOOT__ */

#ifndef __U_BOOT__
/* Make sure we have a controlling tty.  If we get started under a job
 * aware app (like bash for example), make sure we are now in charge so
 * we don't fight over who gets the foreground */
স্থির শূন্য setup_job_control(শূন্য)
{
	স্থির pid_t shell_pgrp;
	/* Loop until we are in the foreground.  */
	যেতিয়ালৈ (tcgetpgrp (shell_terminal) != (shell_pgrp = getpgrp ()))
		kill (- shell_pgrp, SIGTTIN);

	/* Ignore interactive and job-control signals.  */
	সঙ্কেত(SIGINT, SIG_IGN);
	সঙ্কেত(SIGQUIT, SIG_IGN);
	সঙ্কেত(SIGTERM, SIG_IGN);
	সঙ্কেত(SIGTSTP, SIG_IGN);
	সঙ্কেত(SIGTTIN, SIG_IGN);
	সঙ্কেত(SIGTTOU, SIG_IGN);
	সঙ্কেত(SIGCHLD, SIG_IGN);

	/* Put ourselves in our own process group.  */
	setsid();
	shell_pgrp = getpid ();
	setpgid (shell_pgrp, shell_pgrp);

	/* Grab control of the terminal.  */
	tcsetpgrp(shell_terminal, shell_pgrp);
}

পূর্ণাঙ্ক hush_main(পূর্ণাঙ্ক argc, ৱর্ণ * ধ্রুৱ *argv)
{
	পূর্ণাঙ্ক opt;
	FILE *প্ৰবেশ;
	ৱর্ণ **e = environ;

	/* XXX what should these be while sourcing /etc/profile? */
	global_argc = argc;
	global_argv = argv;

	/* (re?) initialize globals.  Sometimes hush_main() ends up calling
	 * hush_main(), therefore we cannot rely on the BSS to zero out this
	 * stuff.  Reset these to 0 every time. */
	ifs = রিক্ত;
	/* map[] is taken care of with call to update_ifs_map() */
	fake_mode = 0;
	interactive = 0;
	close_me_head = রিক্ত;
	last_bg_pid = 0;
	job_list = রিক্ত;
	last_jobid = 0;

	/* Initialize some more globals to non-zero values */
	set_cwd();
#ifdef CONFIG_FEATURE_COMMAND_EDITING
	cmdedit_set_initial_prompt();
#অন্যথা
	PS1 = রিক্ত;
#endif
	PS2 = "> ";

	/* initialize our shell local variables with the values
	 * currently living in the environment */
	যদি (e) {
		বাবে (; *e; e++)
			set_local_var(*e, 2);   /* without call putenv() */
	}

	last_return_code=EXIT_SUCCESS;

	যদি (argv[0] && argv[0][0] == '-') {
		debug_printf("\nsourcing /etc/profile\n");
		যদি ((প্ৰবেশ = fopen("/etc/profile", "r")) != রিক্ত) {
			mark_open(fileno(প্ৰবেশ));
			parse_file_outer(প্ৰবেশ);
			mark_closed(fileno(প্ৰবেশ));
			fclose(প্ৰবেশ);
		}
	}
	প্ৰবেশ=stdin;

	যেতিয়ালৈ ((opt = getopt(argc, argv, "c:xif")) > 0) {
		ৱিকল্প (opt) {
			পক্ষ 'c':
				{
					global_argv = argv+optind;
					global_argc = argc-optind;
					opt = parse_string_outer(optarg, FLAG_PARSE_SEMICOLON);
					গচ্ছ final_return;
				}
				ভঙ্গ;
			পক্ষ 'i':
				interactive++;
				ভঙ্গ;
			পক্ষ 'f':
				fake_mode++;
				ভঙ্গ;
			স্ৱভাৱ:
#ifndef BB_VER
				fprintf(stderr, "Usage: sh [FILE]...\n"
						"   or: sh -c command [args]...\n\n");
				exit(EXIT_FAILURE);
#অন্যথা
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
	যদি (argv[optind]==রিক্ত && প্ৰবেশ==stdin &&
			isatty(fileno(stdin)) && isatty(fileno(stdout))) {
		interactive++;
	}

	debug_printf("\ninteractive=%d\n", interactive);
	যদি (interactive) {
		/* Looks like they want an interactive shell */
#ifndef CONFIG_FEATURE_SH_EXTRA_QUIET
		printf( "\n\n" BB_BANNER " hush - the humble shell v0.01 (testing)\n");
		printf( "Enter 'help' for a list of built-in commands.\n\n");
#endif
		setup_job_control();
	}

	যদি (argv[optind]==রিক্ত) {
		opt=parse_file_outer(stdin);
		গচ্ছ final_return;
	}

	debug_printf("\nrunning script '%s'\n", argv[optind]);
	global_argv = argv+optind;
	global_argc = argc-optind;
	প্ৰবেশ = xfopen(argv[optind], "r");
	opt = parse_file_outer(প্ৰবেশ);

#ifdef CONFIG_FEATURE_CLEAN_UP
	fclose(প্ৰবেশ);
	যদি (cwd && cwd != unknown)
		মুক্ত((ৱর্ণ*)cwd);
	{
		সংরচনা variables *cur, *tmp;
		বাবে(cur = শীৰ্ষ_vars; cur; cur = tmp) {
			tmp = cur->next;
			যদি (!cur->flg_read_only) {
				মুক্ত(cur->name);
				মুক্ত(cur->মান);
				মুক্ত(cur);
			}
		}
	}
#endif

final_return:
	ঘূৰাই(opt?opt:last_return_code);
}
#endif

স্থির ৱর্ণ *ঢোকাও_var_মান(ৱর্ণ *inp)
{
	ঘূৰাই ঢোকাও_var_মান_sub(inp, 0);
}

স্থির ৱর্ণ *ঢোকাও_var_মান_sub(ৱর্ণ *inp, পূর্ণাঙ্ক tag_subst)
{
	পূর্ণাঙ্ক res_str_len = 0;
	পূর্ণাঙ্ক len;
	পূর্ণাঙ্ক done = 0;
	ৱর্ণ *p, *p1, *res_str = রিক্ত;

	যেতিয়ালৈ ((p = strchr(inp, SPECIAL_VAR_প্ৰতীক))) {
		/* check the beginning of the string for normal characters */
		যদি (p != inp) {
			/* copy any characters to the result string */
			len = p - inp;
			res_str = xrealloc(res_str, (res_str_len + len));
			strncpy((res_str + res_str_len), inp, len);
			res_str_len += len;
		}
		inp = ++p;
		/* find the ending marker */
		p = strchr(inp, SPECIAL_VAR_প্ৰতীক);
		*p = '\0';
		/* look up the value to substitute */
		যদি ((p1 = lookup_param(inp))) {
			যদি (tag_subst)
				len = res_str_len + strlen(p1) + 2;
			অন্যথা
				len = res_str_len + strlen(p1);
			res_str = xrealloc(res_str, (1 + len));
			যদি (tag_subst) {
				/*
				 * copy the variable value to the result
				 * string
				 */
				strcpy((res_str + res_str_len + 1), p1);

				/*
				 * mark the replaced text to be accepted as
				 * is
				 */
				res_str[res_str_len] = SUBSTED_VAR_প্ৰতীক;
				res_str[res_str_len + 1 + strlen(p1)] =
					SUBSTED_VAR_প্ৰতীক;
			} অন্যথা
				/*
				 * copy the variable value to the result
				 * string
				 */
				strcpy((res_str + res_str_len), p1);

			res_str_len = len;
		}
		*p = SPECIAL_VAR_প্ৰতীক;
		inp = ++p;
		done = 1;
	}
	যদি (done) {
		res_str = xrealloc(res_str, (1 + res_str_len + strlen(inp)));
		strcpy((res_str + res_str_len), inp);
		যেতিয়ালৈ ((p = strchr(res_str, '\n'))) {
			*p = ' ';
		}
	}
	ঘূৰাই (res_str == রিক্ত) ? inp : res_str;
}

স্থির ৱর্ণ **make_list_in(ৱর্ণ **inp, ৱর্ণ *name)
{
	পূর্ণাঙ্ক len, i;
	পূর্ণাঙ্ক name_len = strlen(name);
	পূর্ণাঙ্ক n = 0;
	ৱর্ণ **list;
	ৱর্ণ *p1, *p2, *p3;

	/* create list of variable values */
	list = xmalloc(পরিমাণ(*list));
	বাবে (i = 0; inp[i]; i++) {
		p3 = ঢোকাও_var_মান(inp[i]);
		p1 = p3;
		যেতিয়ালৈ (*p1) {
			যদি (*p1 == ' ') {
				p1++;
				অনুৱর্ত;
			}
			যদি ((p2 = strchr(p1, ' '))) {
				len = p2 - p1;
			} অন্যথা {
				len = strlen(p1);
				p2 = p1 + len;
			}
			/* we use n + 2 in realloc for list,because we add
			 * new element and then we will add NULL element */
			list = xrealloc(list, পরিমাণ(*list) * (n + 2));
			list[n] = xmalloc(2 + name_len + len);
			strcpy(list[n], name);
			strcat(list[n], "=");
			strncat(list[n], p1, len);
			list[n++][name_len + len + 1] = '\0';
			p1 = p2;
		}
		যদি (p3 != inp[i]) মুক্ত(p3);
	}
	list[n] = রিক্ত;
	ঘূৰাই list;
}

/*
 * Make new string for parser
 * inp     - array of argument strings to flatten
 * nonnull - indicates argument was quoted when originally parsed
 */
স্থির ৱর্ণ *make_string(ৱর্ণ **inp, পূর্ণাঙ্ক *nonnull)
{
	ৱর্ণ *p;
	ৱর্ণ *str = রিক্ত;
	পূর্ণাঙ্ক n;
	পূর্ণাঙ্ক len = 2;
	ৱর্ণ *noeval_str;
	পূর্ণাঙ্ক noeval = 0;

	noeval_str = get_local_var("HUSH_NO_EVAL");
	যদি (noeval_str != রিক্ত && *noeval_str != '0' && *noeval_str != '\0')
		noeval = 1;
	বাবে (n = 0; inp[n]; n++) {
		p = ঢোকাও_var_মান_sub(inp[n], noeval);
		ৱর্ণ *new_str = xrealloc(str, (len + strlen(p) + (2 * nonnull[n])));
		যদি (!new_str) {
			মুক্ত(str);
			যদি (p != inp[n]) মুক্ত(p);
			ঘূৰাই রিক্ত;
		}
		str = new_str;
		যদি (n) {
			strcat(str, " ");
		} অন্যথা {
			*str = '\0';
		}
		যদি (nonnull[n])
			strcat(str, "'");
		strcat(str, p);
		যদি (nonnull[n])
			strcat(str, "'");
		len = strlen(str) + 3;
		যদি (p != inp[n]) মুক্ত(p);
	}
	len = strlen(str);
	*(str + len) = '\n';
	*(str + len + 1) = '\0';
	ঘূৰাই str;
}

#ifdef __U_BOOT__
স্থির পূর্ণাঙ্ক do_showvar(সংরচনা cmd_tbl *cmdtp, পূর্ণাঙ্ক flag, পূর্ণাঙ্ক argc,
		      ৱর্ণ *ধ্রুৱ argv[])
{
	পূর্ণাঙ্ক i, k;
	পূর্ণাঙ্ক rcode = 0;
	সংরচনা variables *cur;

	যদি (argc == 1) {		/* Print all env variables	*/
		বাবে (cur = শীৰ্ষ_vars; cur; cur = cur->next) {
			printf ("%s=%s\n", cur->name, cur->মান);
			যদি (ctrlc ()) {
				puts ("\n ** Abort\n");
				ঘূৰাই 1;
			}
		}
		ঘূৰাই 0;
	}
	বাবে (i = 1; i < argc; ++i) {	/* print single env variables	*/
		ৱর্ণ *name = argv[i];

		k = -1;
		বাবে (cur = শীৰ্ষ_vars; cur; cur = cur->next) {
			যদি(strcmp (cur->name, name) == 0) {
				k = 0;
				printf ("%s=%s\n", cur->name, cur->মান);
			}
			যদি (ctrlc ()) {
				puts ("\n ** Abort\n");
				ঘূৰাই 1;
			}
		}
		যদি (k < 0) {
			printf ("## Error: \"%s\" not defined\n", name);
			rcode ++;
		}
	}
	ঘূৰাই rcode;
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
