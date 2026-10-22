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
#ಅನ್ತರ್ಭಾವ <env.h>
#ಅನ್ತರ್ಭಾವ <malloc.h>         /* malloc, free, realloc*/
#ಅನ್ತರ್ಭಾವ <linux/ctype.h>    /* isalpha, isdigit */
#ಅನ್ತರ್ಭಾವ <console.h>
#ಅನ್ತರ್ಭಾವ <bootretry.h>
#ಅನ್ತರ್ಭಾವ <cli.h>
#ಅನ್ತರ್ಭಾವ <cli_hush.h>
#ಅನ್ತರ್ಭಾವ <command.h>        /* find_cmd */
#ಅನ್ತರ್ಭಾವ <vsprintf.h>
#endif
#ifndef __U_BOOT__
#ಅನ್ತರ್ಭಾವ <ctype.h>     /* isalpha, isdigit */
#ಅನ್ತರ್ಭಾವ <unistd.h>    /* getpid */
#ಅನ್ತರ್ಭಾವ <stdlib.h>    /* getenv, atoi */
#ಅನ್ತರ್ಭಾವ <string.h>    /* strchr */
#ಅನ್ತರ್ಭಾವ <stdio.h>     /* popen etc. */
#ಅನ್ತರ್ಭಾವ <glob.h>      /* glob, of course */
#ಅನ್ತರ್ಭಾವ <stdarg.h>    /* va_list */
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <fcntl.h>
#ಅನ್ತರ್ಭಾವ <getopt.h>    /* should be pretty obvious */

#ಅನ್ತರ್ಭಾವ <sys/stat.h>  /* ulimit */
#ಅನ್ತರ್ಭಾವ <sys/types.h>
#ಅನ್ತರ್ಭಾವ <sys/wait.h>
#ಅನ್ತರ್ಭಾವ <signal.h>

/* #include <dmalloc.h> */

#ಯದಿ 1
#ಅನ್ತರ್ಭಾವ "busybox.h"
#ಅನ್ತರ್ಭಾವ "cmdedit.h"
#ಅನ್ಯಥಾ
#define applet_name "hush"
#ಅನ್ತರ್ಭಾವ "standalone.h"
#define hush_main main
#define BB_BANNER
#endif
#endif
#define SPECIAL_VAR_ಚಿಹ್ನೆ 03
#define SUBSTED_VAR_ಚಿಹ್ನೆ 04
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
#define error_ಸಂದೇಶ printf
#ಅನ್ಯಥಾ
ಪ್ರಕಾರನಿರ್ದೇಶ ಗಣನಾ {
	REDIRECT_ಒಳಹರಿವು     = 1,
	REDIRECT_OVERWRITE = 2,
	REDIRECT_APPEND    = 3,
	REDIRECT_HEREIS    = 4,
	REDIRECT_IO        = 5
} redir_type;

/* The descrip member of this structure is only used to make debugging
 * output pretty */
ಸಂರಚನಾ {ಪೂರ್ಣಾಙ್ಕ mode; ಪೂರ್ಣಾಙ್ಕ default_fd; ವರ್ಣ *descrip;} redir_table[] = {
	{ 0,                         0, "()" },
	{ O_RDONLY,                  0, "<"  },
	{ O_CREAT|O_TRUNC|O_WRONLY,  1, ">"  },
	{ O_CREAT|O_APPEND|O_WRONLY, 1, ">>" },
	{ O_RDONLY,                 -1, "<<" },
	{ O_RDWR,                    1, "<>" }
};
#endif

ಪ್ರಕಾರನಿರ್ದೇಶ ಗಣನಾ {
	PIPE_SEQ = 1,
	PIPE_AND = 2,
	PIPE_OR  = 3,
	PIPE_BG  = 4,
} pipe_style;

/* might eventually control execution */
ಪ್ರಕಾರನಿರ್ದೇಶ ಗಣನಾ {
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
ಸಂರಚನಾ p_context {
	ಸಂರಚನಾ child_prog *child;
	ಸಂರಚನಾ pipe *list_head;
	ಸಂರಚನಾ pipe *pipe;
#ifndef __U_BOOT__
	ಸಂರಚನಾ redir_struct *pending_redirect;
#endif
	reserved_style w;
	ಪೂರ್ಣಾಙ್ಕ old_flag;				/* for figuring out valid reserved words */
	ಸಂರಚನಾ p_context *ರಾಶಿ;
	ಪೂರ್ಣಾಙ್ಕ type;			/* define type of parser : ";$" common or special symbol */
	/* How about quoting status? */
};

#ifndef __U_BOOT__
ಸಂರಚನಾ redir_struct {
	redir_type type;			/* type of redirection */
	ಪೂರ್ಣಾಙ್ಕ fd;						/* file descriptor being redirected */
	ಪೂರ್ಣಾಙ್ಕ dup;					/* -1, or file descriptor being duplicated */
	ಸಂರಚನಾ redir_struct *next;	/* pointer to the next redirect in the list */
	glob_t word;				/* *word.gl_pathv is the filename */
};
#endif

ಸಂರಚನಾ child_prog {
#ifndef __U_BOOT__
	pid_t pid;					/* 0 if exited */
#endif
	ವರ್ಣ **argv;				/* program name and arguments */
	/* was quoted when parsed; copy of struct o_string.nonnull field */
	ಪೂರ್ಣಾಙ್ಕ *argv_nonnull;
#ifdef __U_BOOT__
	ಪೂರ್ಣಾಙ್ಕ    argc;                            /* number of program arguments */
#endif
	ಸಂರಚನಾ pipe *group;			/* if non-NULL, first in group or subshell */
#ifndef __U_BOOT__
	ಪೂರ್ಣಾಙ್ಕ subshell;				/* flag, non-zero if group must be forked */
	ಸಂರಚನಾ redir_struct *redirects;	/* I/O redirections */
	glob_t glob_result;			/* result of parameter globbing */
	ಪೂರ್ಣಾಙ್ಕ is_stopped;				/* is the program currently running? */
	ಸಂರಚನಾ pipe *family;		/* pointer back to the child's parent pipe */
#endif
	ಪೂರ್ಣಾಙ್ಕ sp;				/* number of SPECIAL_VAR_SYMBOL */
	ಪೂರ್ಣಾಙ್ಕ type;
};

ಸಂರಚನಾ pipe {
#ifndef __U_BOOT__
	ಪೂರ್ಣಾಙ್ಕ jobid;					/* job number */
#endif
	ಪೂರ್ಣಾಙ್ಕ ಅಂಕೆ_progs;				/* total number of programs in job */
#ifndef __U_BOOT__
	ಪೂರ್ಣಾಙ್ಕ running_progs;			/* number of programs running */
	ವರ್ಣ *text;					/* name of job */
	ವರ್ಣ *cmdbuf;				/* buffer various argv's point into */
	pid_t pgrp;					/* process group ID for the job */
#endif
	ಸಂರಚನಾ child_prog *progs;	/* array of commands in pipe */
	ಸಂರಚನಾ pipe *next;			/* to track background commands */
#ifndef __U_BOOT__
	ಪೂರ್ಣಾಙ್ಕ stopped_progs;			/* number of programs alive, but stopped */
	ಪೂರ್ಣಾಙ್ಕ job_context;			/* bitmask defining current context */
#endif
	pipe_style followup;		/* PIPE_BG, PIPE_SEQ, PIPE_OR, PIPE_AND */
	reserved_style r_mode;		/* supports if, for, while, until */
};

#ifndef __U_BOOT__
ಸಂರಚನಾ close_me {
	ಪೂರ್ಣಾಙ್ಕ fd;
	ಸಂರಚನಾ close_me *next;
};
#endif

ಸಂರಚನಾ variables {
	ವರ್ಣ *name;
	ವರ್ಣ *ಮೌಲ್ಯ;
	ಪೂರ್ಣಾಙ್ಕ flg_export;
	ಪೂರ್ಣಾಙ್ಕ flg_read_only;
	ಸಂರಚನಾ variables *next;
};

/* globals, connect us to the outside world
 * the first three support $?, $#, and $1 */
#ifndef __U_BOOT__
ವರ್ಣ **global_argv;
unsigned ಪೂರ್ಣಾಙ್ಕ global_argc;
#endif
ಸ್ಥಿರ unsigned ಪೂರ್ಣಾಙ್ಕ last_return_code;
#ifndef __U_BOOT__
ಬಾಹ್ಯ ವರ್ಣ **environ; /* This is in <unistd.h>, but protected with __USE_GNU */
#endif

/* "globals" within this file */
ಸ್ಥಿರ uchar *ifs;
ಸ್ಥಿರ ವರ್ಣ map[256];
#ifndef __U_BOOT__
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ fake_mode;
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ interactive;
ಸ್ಥಿರ ಸಂರಚನಾ close_me *close_me_head;
ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *cwd;
ಸ್ಥಿರ ಸಂರಚನಾ pipe *job_list;
ಸ್ಥಿರ unsigned ಪೂರ್ಣಾಙ್ಕ last_bg_pid;
ಸ್ಥಿರ unsigned ಪೂರ್ಣಾಙ್ಕ last_jobid;
ಸ್ಥಿರ unsigned ಪೂರ್ಣಾಙ್ಕ shell_terminal;
ಸ್ಥಿರ ವರ್ಣ *PS1;
ಸ್ಥಿರ ವರ್ಣ *PS2;
ಸಂರಚನಾ variables shell_ver = { "HUSH_VERSION", "0.01", 1, 1, 0 };
ಸಂರಚನಾ variables *ತುದಿ_vars = &shell_ver;
#ಅನ್ಯಥಾ
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ flag_repeat = 0;
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ do_repeat = 0;
ಸ್ಥಿರ ಸಂರಚನಾ variables *ತುದಿ_vars = ರಿಕ್ತ ;
#endif /*__U_BOOT__ */

#define B_CHUNK (100)
#define B_NOSPAC 1

ಪ್ರಕಾರನಿರ್ದೇಶ ಸಂರಚನಾ {
	ವರ್ಣ *data;
	ಪೂರ್ಣಾಙ್ಕ length;
	ಪೂರ್ಣಾಙ್ಕ maxlen;
	ಪೂರ್ಣಾಙ್ಕ quote;
	ಪೂರ್ಣಾಙ್ಕ nonnull;
} o_string;
#define NULL_O_STRING {ರಿಕ್ತ,0,0,0,0}
/* used for initialization:
	o_string foo = NULL_O_STRING; */

/* I can almost use ordinary FILE *.  Is open_memstream() universally
 * available?  Where is it documented? */
ಸಂರಚನಾ in_str {
	ಧ್ರುವ unsigned ವರ್ಣ *p;
#ifndef __U_BOOT__
	ವರ್ಣ peek_buf[2];
#endif
	ಪೂರ್ಣಾಙ್ಕ __promptme;
	ಪೂರ್ಣಾಙ್ಕ promptmode;
#ifndef __U_BOOT__
	FILE *file;
#endif
	ಪೂರ್ಣಾಙ್ಕ (*get) (ಸಂರಚನಾ in_str *);
	ಪೂರ್ಣಾಙ್ಕ (*peek) (ಸಂರಚನಾ in_str *);
};
#define b_getch(ಒಳಹರಿವು) ((ಒಳಹರಿವು)->get(ಒಳಹರಿವು))
#define b_peek(ಒಳಹರಿವು) ((ಒಳಹರಿವು)->peek(ಒಳಹರಿವು))

#ifndef __U_BOOT__
#define JOB_STATUS_FORMAT "[%d] %-22s %.40s\n"

ಸಂರಚನಾ built_in_command {
	ವರ್ಣ *cmd;					/* name */
	ವರ್ಣ *descr;				/* description */
	ಪೂರ್ಣಾಙ್ಕ (*function) (ಸಂರಚನಾ child_prog *);	/* function ptr */
};
#endif

/* define DEBUG_SHELL for debugging output (obviously ;-)) */
#ಯದಿ 0
#define DEBUG_SHELL
#endif

/* This should be in utility.c */
#ifdef DEBUG_SHELL
#ifndef __U_BOOT__
ಸ್ಥಿರ ಶೂನ್ಯ debug_printf(ಧ್ರುವ ವರ್ಣ *format, ...)
{
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
}
#ಅನ್ಯಥಾ
#define debug_printf(fmt,args...)	printf (fmt ,##args)
#endif
#ಅನ್ಯಥಾ
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ debug_printf(ಧ್ರುವ ವರ್ಣ *format, ...) { }
#endif
#define final_printf debug_printf

#ifdef __U_BOOT__
ಸ್ಥಿರ ಶೂನ್ಯ syntax_err(ಶೂನ್ಯ) {
	 printf("syntax error\n");
}
#ಅನ್ಯಥಾ
ಸ್ಥಿರ ಶೂನ್ಯ __syntax(ವರ್ಣ *file, ಪೂರ್ಣಾಙ್ಕ ಸಾಲು) {
	error_ಸಂದೇಶ("syntax error %s:%d", file, ಸಾಲು);
}
#define syntax() __syntax(__FILE__, __LINE__)
#endif

#ifdef __U_BOOT__
ಸ್ಥಿರ ಶೂನ್ಯ *xmalloc(size_t size);
ಸ್ಥಿರ ಶೂನ್ಯ *xrealloc(ಶೂನ್ಯ *ptr, size_t size);
#ಅನ್ಯಥಾ
/* Index of subroutines: */
/*   function prototypes for builtins */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ builtin_cd(ಸಂರಚನಾ child_prog *child);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ builtin_env(ಸಂರಚನಾ child_prog *child);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ builtin_eval(ಸಂರಚನಾ child_prog *child);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ builtin_exec(ಸಂರಚನಾ child_prog *child);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ builtin_exit(ಸಂರಚನಾ child_prog *child);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ builtin_export(ಸಂರಚನಾ child_prog *child);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ builtin_fg_bg(ಸಂರಚನಾ child_prog *child);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ builtin_help(ಸಂರಚನಾ child_prog *child);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ builtin_jobs(ಸಂರಚನಾ child_prog *child);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ builtin_pwd(ಸಂರಚನಾ child_prog *child);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ builtin_read(ಸಂರಚನಾ child_prog *child);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ builtin_set(ಸಂರಚನಾ child_prog *child);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ builtin_shift(ಸಂರಚನಾ child_prog *child);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ builtin_source(ಸಂರಚನಾ child_prog *child);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ builtin_umask(ಸಂರಚನಾ child_prog *child);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ builtin_unset(ಸಂರಚನಾ child_prog *child);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ builtin_not_written(ಸಂರಚನಾ child_prog *child);
#endif
/*   o_string manipulation: */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ b_check_space(o_string *o, ಪೂರ್ಣಾಙ್ಕ len);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ b_addchr(o_string *o, ಪೂರ್ಣಾಙ್ಕ ch);
ಸ್ಥಿರ ಶೂನ್ಯ b_reset(o_string *o);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ b_addqchr(o_string *o, ಪೂರ್ಣಾಙ್ಕ ch, ಪೂರ್ಣಾಙ್ಕ quote);
#ifndef __U_BOOT__
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ b_adduint(o_string *o, unsigned ಪೂರ್ಣಾಙ್ಕ i);
#endif
/*  in_str manipulations: */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ static_get(ಸಂರಚನಾ in_str *i);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ static_peek(ಸಂರಚನಾ in_str *i);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ file_get(ಸಂರಚನಾ in_str *i);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ file_peek(ಸಂರಚನಾ in_str *i);
#ifndef __U_BOOT__
ಸ್ಥಿರ ಶೂನ್ಯ setup_file_in_str(ಸಂರಚನಾ in_str *i, FILE *f);
#ಅನ್ಯಥಾ
ಸ್ಥಿರ ಶೂನ್ಯ setup_file_in_str(ಸಂರಚನಾ in_str *i);
#endif
ಸ್ಥಿರ ಶೂನ್ಯ setup_string_in_str(ಸಂರಚನಾ in_str *i, ಧ್ರುವ ವರ್ಣ *s);
#ifndef __U_BOOT__
/*  close_me manipulations: */
ಸ್ಥಿರ ಶೂನ್ಯ mark_open(ಪೂರ್ಣಾಙ್ಕ fd);
ಸ್ಥಿರ ಶೂನ್ಯ mark_closed(ಪೂರ್ಣಾಙ್ಕ fd);
ಸ್ಥಿರ ಶೂನ್ಯ close_all(ಶೂನ್ಯ);
#endif
/*  "run" the final data structures: */
ಸ್ಥಿರ ವರ್ಣ *indenter(ಪೂರ್ಣಾಙ್ಕ i);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ free_pipe_list(ಸಂರಚನಾ pipe *head, ಪೂರ್ಣಾಙ್ಕ indent);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ free_pipe(ಸಂರಚನಾ pipe *pi, ಪೂರ್ಣಾಙ್ಕ indent);
/*  really run the final data structures: */
#ifndef __U_BOOT__
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ setup_redirects(ಸಂರಚನಾ child_prog *prog, ಪೂರ್ಣಾಙ್ಕ squirrel[]);
#endif
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ run_list_real(ಸಂರಚನಾ pipe *pi);
#ifndef __U_BOOT__
ಸ್ಥಿರ ಶೂನ್ಯ pseudo_exec(ಸಂರಚನಾ child_prog *child) __attribute__ ((noreturn));
#endif
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ run_pipe_real(ಸಂರಚನಾ pipe *pi);
/*   extended glob support: */
#ifndef __U_BOOT__
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ globhack(ಧ್ರುವ ವರ್ಣ *src, ಪೂರ್ಣಾಙ್ಕ flags, glob_t *pglob);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ glob_needed(ಧ್ರುವ ವರ್ಣ *s);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ xglob(o_string *dest, ಪೂರ್ಣಾಙ್ಕ flags, glob_t *pglob);
#endif
/*   variable assignment: */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ is_assignment(ಧ್ರುವ ವರ್ಣ *s);
/*   data structure manipulation: */
#ifndef __U_BOOT__
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ setup_redirect(ಸಂರಚನಾ p_context *ctx, ಪೂರ್ಣಾಙ್ಕ fd, redir_type style, ಸಂರಚನಾ in_str *ಒಳಹರಿವು);
#endif
ಸ್ಥಿರ ಶೂನ್ಯ initialize_context(ಸಂರಚನಾ p_context *ctx);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ done_word(o_string *dest, ಸಂರಚನಾ p_context *ctx);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ done_command(ಸಂರಚನಾ p_context *ctx);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ done_pipe(ಸಂರಚನಾ p_context *ctx, pipe_style type);
/*   primary string parsing: */
#ifndef __U_BOOT__
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ redirect_dup_ಅಂಕೆ(ಸಂರಚನಾ in_str *ಒಳಹರಿವು);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ redirect_opt_ಅಂಕೆ(o_string *o);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ process_command_subs(o_string *dest, ಸಂರಚನಾ p_context *ctx, ಸಂರಚನಾ in_str *ಒಳಹರಿವು, ಪೂರ್ಣಾಙ್ಕ subst_end);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ parse_group(o_string *dest, ಸಂರಚನಾ p_context *ctx, ಸಂರಚನಾ in_str *ಒಳಹರಿವು, ಪೂರ್ಣಾಙ್ಕ ch);
#endif
ಸ್ಥಿರ ವರ್ಣ *lookup_param(ವರ್ಣ *src);
ಸ್ಥಿರ ವರ್ಣ *make_string(ವರ್ಣ **inp, ಪೂರ್ಣಾಙ್ಕ *nonnull);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ handle_dollar(o_string *dest, ಸಂರಚನಾ p_context *ctx, ಸಂರಚನಾ in_str *ಒಳಹರಿವು);
#ifndef __U_BOOT__
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ parse_string(o_string *dest, ಸಂರಚನಾ p_context *ctx, ಧ್ರುವ ವರ್ಣ *src);
#endif
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ parse_stream(o_string *dest, ಸಂರಚನಾ p_context *ctx, ಸಂರಚನಾ in_str *input0, ಪೂರ್ಣಾಙ್ಕ end_trigger);
/*   setup: */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ parse_stream_outer(ಸಂರಚನಾ in_str *inp, ಪೂರ್ಣಾಙ್ಕ flag);
#ifndef __U_BOOT__
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ parse_string_outer(ಧ್ರುವ ವರ್ಣ *s, ಪೂರ್ಣಾಙ್ಕ flag);
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ parse_file_outer(FILE *f);
#endif
#ifndef __U_BOOT__
/*   job management: */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ checkjobs(ಸಂರಚನಾ pipe* fg_pipe);
ಸ್ಥಿರ ಶೂನ್ಯ ಸೇರಿಸು_bg_job(ಸಂರಚನಾ pipe *pi);
ಸ್ಥಿರ ಶೂನ್ಯ remove_bg_job(ಸಂರಚನಾ pipe *pi);
#endif
/*     local variable support */
ಸ್ಥಿರ ವರ್ಣ **make_list_in(ವರ್ಣ **inp, ವರ್ಣ *name);
ಸ್ಥಿರ ವರ್ಣ *ಸೇರಿಸು_var_ಮೌಲ್ಯ(ವರ್ಣ *inp);
ಸ್ಥಿರ ವರ್ಣ *ಸೇರಿಸು_var_ಮೌಲ್ಯ_sub(ವರ್ಣ *inp, ಪೂರ್ಣಾಙ್ಕ tag_subst);

#ifndef __U_BOOT__
/* Table of built-in functions.  They can be forked or not, depending on
 * context: within pipes, they fork.  As simple commands, they do not.
 * When used in non-forking context, they can change global variables
 * in the parent shell process.  If forked, of course they can not.
 * For example, 'unset foo | whatever' will parse and run, but foo will
 * still be set at the end. */
ಸ್ಥಿರ ಸಂರಚನಾ built_in_command bltins[] = {
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
	{ರಿಕ್ತ, ರಿಕ್ತ, ರಿಕ್ತ}
};

ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ *set_cwd(ಶೂನ್ಯ)
{
	ಯದಿ(cwd==unknown)
		cwd = ರಿಕ್ತ;     /* xgetcwd(arg) called free(arg) */
	cwd = xgetcwd((ವರ್ಣ *)cwd);
	ಯದಿ (!cwd)
		cwd = unknown;
	ಪ್ರತ್ಯಾಗಮ cwd;
}

/* built-in 'eval' handler */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ builtin_eval(ಸಂರಚನಾ child_prog *child)
{
	ವರ್ಣ *str = ರಿಕ್ತ;
	ಪೂರ್ಣಾಙ್ಕ rcode = EXIT_SUCCESS;

	ಯದಿ (child->argv[1]) {
		str = make_string(child->argv + 1);
		parse_string_outer(str, FLAG_EXIT_FROM_LOOP |
					FLAG_PARSE_SEMICOLON);
		ಮುಕ್ತ(str);
		rcode = last_return_code;
	}
	ಪ್ರತ್ಯಾಗಮ rcode;
}

/* built-in 'cd <path>' handler */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ builtin_cd(ಸಂರಚನಾ child_prog *child)
{
	ವರ್ಣ *newdir;
	ಯದಿ (child->argv[1] == ರಿಕ್ತ)
		newdir = env_get("HOME");
	ಅನ್ಯಥಾ
		newdir = child->argv[1];
	ಯದಿ (chdir(newdir)) {
		printf("cd: %s: %s\n", newdir, strerror(errno));
		ಪ್ರತ್ಯಾಗಮ EXIT_FAILURE;
	}
	set_cwd();
	ಪ್ರತ್ಯಾಗಮ EXIT_SUCCESS;
}

/* built-in 'env' handler */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ builtin_env(ಸಂರಚನಾ child_prog *dummy)
{
	ವರ್ಣ **e = environ;
	ಯದಿ (e == ರಿಕ್ತ) ಪ್ರತ್ಯಾಗಮ EXIT_FAILURE;
	ಪ್ರತಿ (; *e; e++) {
		puts(*e);
	}
	ಪ್ರತ್ಯಾಗಮ EXIT_SUCCESS;
}

/* built-in 'exec' handler */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ builtin_exec(ಸಂರಚನಾ child_prog *child)
{
	ಯದಿ (child->argv[1] == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ EXIT_SUCCESS;   /* Really? */
	child->argv++;
	pseudo_exec(child);
	/* never returns */
}

/* built-in 'exit' handler */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ builtin_exit(ಸಂರಚನಾ child_prog *child)
{
	ಯದಿ (child->argv[1] == ರಿಕ್ತ)
		exit(last_return_code);
	exit (atoi(child->argv[1]));
}

/* built-in 'export VAR=value' handler */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ builtin_export(ಸಂರಚನಾ child_prog *child)
{
	ಪೂರ್ಣಾಙ್ಕ res = 0;
	ವರ್ಣ *name = child->argv[1];

	ಯದಿ (name == ರಿಕ್ತ) {
		ಪ್ರತ್ಯಾಗಮ (builtin_env(child));
	}

	name = strdup(name);

	ಯದಿ(name) {
		ವರ್ಣ *ಮೌಲ್ಯ = strchr(name, '=');

		ಯದಿ (!ಮೌಲ್ಯ) {
			ವರ್ಣ *tmp;
			/* They are exporting something without an =VALUE */

			ಮೌಲ್ಯ = get_local_var(name);
			ಯದಿ (ಮೌಲ್ಯ) {
				size_t ln = strlen(name);

				tmp = realloc(name, ln+strlen(ಮೌಲ್ಯ)+2);
				ಯದಿ(tmp==ರಿಕ್ತ)
					res = -1;
				ಅನ್ಯಥಾ {
					sprintf(tmp+ln, "=%s", ಮೌಲ್ಯ);
					name = tmp;
				}
			} ಅನ್ಯಥಾ {
				/* bash does not return an error when trying to export
				 * an undefined variable.  Do likewise. */
				res = 1;
			}
		}
	}
	ಯದಿ (res<0)
		perror_ಸಂದೇಶ("export");
	ಅನ್ಯಥಾ ಯದಿ(res==0)
		res = set_local_var(name, 1);
	ಅನ್ಯಥಾ
		res = 0;
	ಮುಕ್ತ(name);
	ಪ್ರತ್ಯಾಗಮ res;
}

/* built-in 'fg' and 'bg' handler */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ builtin_fg_bg(ಸಂರಚನಾ child_prog *child)
{
	ಪೂರ್ಣಾಙ್ಕ i, jobnum;
	ಸಂರಚನಾ pipe *pi=ರಿಕ್ತ;

	ಯದಿ (!interactive)
		ಪ್ರತ್ಯಾಗಮ EXIT_FAILURE;
	/* If they gave us no args, assume they want the last backgrounded task */
	ಯದಿ (!child->argv[1]) {
		ಪ್ರತಿ (pi = job_list; pi; pi = pi->next) {
			ಯದಿ (pi->jobid == last_jobid) {
				ಭಙ್ಗ;
			}
		}
		ಯದಿ (!pi) {
			error_ಸಂದೇಶ("%s: no current job", child->argv[0]);
			ಪ್ರತ್ಯಾಗಮ EXIT_FAILURE;
		}
	} ಅನ್ಯಥಾ {
		ಯದಿ (sscanf(child->argv[1], "%%%d", &jobnum) != 1) {
			error_ಸಂದೇಶ("%s: bad argument '%s'", child->argv[0], child->argv[1]);
			ಪ್ರತ್ಯಾಗಮ EXIT_FAILURE;
		}
		ಪ್ರತಿ (pi = job_list; pi; pi = pi->next) {
			ಯದಿ (pi->jobid == jobnum) {
				ಭಙ್ಗ;
			}
		}
		ಯದಿ (!pi) {
			error_ಸಂದೇಶ("%s: %d: no such job", child->argv[0], jobnum);
			ಪ್ರತ್ಯಾಗಮ EXIT_FAILURE;
		}
	}

	ಯದಿ (*child->argv[0] == 'f') {
		/* Put the job into the foreground.  */
		tcsetpgrp(shell_terminal, pi->pgrp);
	}

	/* Restart the processes in the job */
	ಪ್ರತಿ (i = 0; i < pi->ಅಂಕೆ_progs; i++)
		pi->progs[i].is_stopped = 0;

	ಯದಿ ( (i=kill(- pi->pgrp, SIGCONT)) < 0) {
		ಯದಿ (i == ESRCH) {
			remove_bg_job(pi);
		} ಅನ್ಯಥಾ {
			perror_ಸಂದೇಶ("kill (SIGCONT)");
		}
	}

	pi->stopped_progs = 0;
	ಪ್ರತ್ಯಾಗಮ EXIT_SUCCESS;
}

/* built-in 'help' handler */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ builtin_help(ಸಂರಚನಾ child_prog *dummy)
{
	ಸಂರಚನಾ built_in_command *x;

	printf("\nBuilt-in commands:\n");
	printf("-------------------\n");
	ಪ್ರತಿ (x = bltins; x->cmd; x++) {
		ಯದಿ (x->descr==ರಿಕ್ತ)
			ಅನುವರ್ತ;
		printf("%s\t%s\n", x->cmd, x->descr);
	}
	printf("\n\n");
	ಪ್ರತ್ಯಾಗಮ EXIT_SUCCESS;
}

/* built-in 'jobs' handler */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ builtin_jobs(ಸಂರಚನಾ child_prog *child)
{
	ಸಂರಚನಾ pipe *job;
	ವರ್ಣ *status_string;

	ಪ್ರತಿ (job = job_list; job; job = job->next) {
		ಯದಿ (job->running_progs == job->stopped_progs)
			status_string = "Stopped";
		ಅನ್ಯಥಾ
			status_string = "Running";

		printf(JOB_STATUS_FORMAT, job->jobid, status_string, job->text);
	}
	ಪ್ರತ್ಯಾಗಮ EXIT_SUCCESS;
}

/* built-in 'pwd' handler */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ builtin_pwd(ಸಂರಚನಾ child_prog *dummy)
{
	puts(set_cwd());
	ಪ್ರತ್ಯಾಗಮ EXIT_SUCCESS;
}

/* built-in 'read VAR' handler */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ builtin_read(ಸಂರಚನಾ child_prog *child)
{
	ಪೂರ್ಣಾಙ್ಕ res;

	ಯದಿ (child->argv[1]) {
		ವರ್ಣ string[BUFSIZ];
		ವರ್ಣ *var = 0;

		string[0] = 0;  /* In case stdin has only EOF */
		/* read string */
		fgets(string, ಪರಿಮಾಣ(string), stdin);
		chomp(string);
		var = malloc(strlen(child->argv[1])+strlen(string)+2);
		ಯದಿ(var) {
			sprintf(var, "%s=%s", child->argv[1], string);
			res = set_local_var(var, 0);
		} ಅನ್ಯಥಾ
			res = -1;
		ಯದಿ (res)
			fprintf(stderr, "read: %m\n");
		ಮುಕ್ತ(var);      /* So not move up to avoid breaking errno */
		ಪ್ರತ್ಯಾಗಮ res;
	} ಅನ್ಯಥಾ {
		ಕುರು res=getchar(); ಯಾವತ್(res!='\n' && res!=EOF);
		ಪ್ರತ್ಯಾಗಮ 0;
	}
}

/* built-in 'set VAR=value' handler */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ builtin_set(ಸಂರಚನಾ child_prog *child)
{
	ವರ್ಣ *temp = child->argv[1];
	ಸಂರಚನಾ variables *e;

	ಯದಿ (temp == ರಿಕ್ತ)
		ಪ್ರತಿ(e = ತುದಿ_vars; e; e=e->next)
			printf("%s=%s\n", e->name, e->ಮೌಲ್ಯ);
	ಅನ್ಯಥಾ
		set_local_var(temp, 0);

		ಪ್ರತ್ಯಾಗಮ EXIT_SUCCESS;
}

/* Built-in 'shift' handler */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ builtin_shift(ಸಂರಚನಾ child_prog *child)
{
	ಪೂರ್ಣಾಙ್ಕ n=1;
	ಯದಿ (child->argv[1]) {
		n=atoi(child->argv[1]);
	}
	ಯದಿ (n>=0 && n<global_argc) {
		/* XXX This probably breaks $0 */
		global_argc -= n;
		global_argv += n;
		ಪ್ರತ್ಯಾಗಮ EXIT_SUCCESS;
	} ಅನ್ಯಥಾ {
		ಪ್ರತ್ಯಾಗಮ EXIT_FAILURE;
	}
}

/* Built-in '.' handler (read-in and execute commands from file) */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ builtin_source(ಸಂರಚನಾ child_prog *child)
{
	FILE *ಒಳಹರಿವು;
	ಪೂರ್ಣಾಙ್ಕ status;

	ಯದಿ (child->argv[1] == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ EXIT_FAILURE;

	/* XXX search through $PATH is missing */
	ಒಳಹರಿವು = fopen(child->argv[1], "r");
	ಯದಿ (!ಒಳಹರಿವು) {
		error_ಸಂದೇಶ("Couldn't open file '%s'", child->argv[1]);
		ಪ್ರತ್ಯಾಗಮ EXIT_FAILURE;
	}

	/* Now run the file */
	/* XXX argv and argc are broken; need to save old global_argv
	 * (pointer only is OK!) on this stack frame,
	 * set global_argv=child->argv+1, recurse, and restore. */
	mark_open(fileno(ಒಳಹರಿವು));
	status = parse_file_outer(ಒಳಹರಿವು);
	mark_closed(fileno(ಒಳಹರಿವು));
	fclose(ಒಳಹರಿವು);
	ಪ್ರತ್ಯಾಗಮ (status);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ builtin_umask(ಸಂರಚನಾ child_prog *child)
{
	mode_t new_umask;
	ಧ್ರುವ ವರ್ಣ *arg = child->argv[1];
	ವರ್ಣ *end;
	ಯದಿ (arg) {
		new_umask=strtoul(arg, &end, 8);
		ಯದಿ (*end!='\0' || end == arg) {
			ಪ್ರತ್ಯಾಗಮ EXIT_FAILURE;
		}
	} ಅನ್ಯಥಾ {
		printf("%.3o\n", (unsigned ಪೂರ್ಣಾಙ್ಕ) (new_umask=umask(0)));
	}
	umask(new_umask);
	ಪ್ರತ್ಯಾಗಮ EXIT_SUCCESS;
}

/* built-in 'unset VAR' handler */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ builtin_unset(ಸಂರಚನಾ child_prog *child)
{
	/* bash returned already true */
	unset_local_var(child->argv[1]);
	ಪ್ರತ್ಯಾಗಮ EXIT_SUCCESS;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ builtin_not_written(ಸಂರಚನಾ child_prog *child)
{
	printf("builtin_%s not written\n",child->argv[0]);
	ಪ್ರತ್ಯಾಗಮ EXIT_FAILURE;
}
#endif

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ b_check_space(o_string *o, ಪೂರ್ಣಾಙ್ಕ len)
{
	/* It would be easy to drop a more restrictive policy
	 * in here, such as setting a maximum string length */
	ಯದಿ (o->length + len > o->maxlen) {
		ವರ್ಣ *old_data = o->data;
		/* assert (data == NULL || o->maxlen != 0); */
		o->maxlen += max(2*len, B_CHUNK);
		o->data = realloc(o->data, 1 + o->maxlen);
		ಯದಿ (o->data == ರಿಕ್ತ) {
			ಮುಕ್ತ(old_data);
		}
	}
	ಪ್ರತ್ಯಾಗಮ o->data == ರಿಕ್ತ;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ b_addchr(o_string *o, ಪೂರ್ಣಾಙ್ಕ ch)
{
	debug_printf("b_addchr: %c %d %p\n", ch, o->length, o);
	ಯದಿ (b_check_space(o, 1)) ಪ್ರತ್ಯಾಗಮ B_NOSPAC;
	o->data[o->length] = ch;
	o->length++;
	o->data[o->length] = '\0';
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಶೂನ್ಯ b_reset(o_string *o)
{
	o->length = 0;
	o->nonnull = 0;
	ಯದಿ (o->data != ರಿಕ್ತ) *o->data = '\0';
}

ಸ್ಥಿರ ಶೂನ್ಯ b_free(o_string *o)
{
	b_reset(o);
	ಮುಕ್ತ(o->data);
	o->data = ರಿಕ್ತ;
	o->maxlen = 0;
}

/* My analysis of quoting semantics tells me that state information
 * is associated with a destination, not a source.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ b_addqchr(o_string *o, ಪೂರ್ಣಾಙ್ಕ ch, ಪೂರ್ಣಾಙ್ಕ quote)
{
	ಯದಿ (quote && strchr("*?[\\",ch)) {
		ಪೂರ್ಣಾಙ್ಕ rc;
		rc = b_addchr(o, '\\');
		ಯದಿ (rc) ಪ್ರತ್ಯಾಗಮ rc;
	}
	ಪ್ರತ್ಯಾಗಮ b_addchr(o, ch);
}

#ifndef __U_BOOT__
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ b_adduint(o_string *o, unsigned ಪೂರ್ಣಾಙ್ಕ i)
{
	ಪೂರ್ಣಾಙ್ಕ r;
	ವರ್ಣ *p = simple_itoa(i);
	/* no escape checking necessary */
	ಕುರು r=b_addchr(o, *p++); ಯಾವತ್ (r==0 && *p);
	ಪ್ರತ್ಯಾಗಮ r;
}
#endif

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ static_get(ಸಂರಚನಾ in_str *i)
{
	ಪೂರ್ಣಾಙ್ಕ ch = *i->p++;
	ಯದಿ (ch=='\0') ಪ್ರತ್ಯಾಗಮ EOF;
	ಪ್ರತ್ಯಾಗಮ ch;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ static_peek(ಸಂರಚನಾ in_str *i)
{
	ಪ್ರತ್ಯಾಗಮ *i->p;
}

#ifndef __U_BOOT__
ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ cmdedit_set_initial_prompt(ಶೂನ್ಯ)
{
#ifndef CONFIG_FEATURE_SH_FANCY_PROMPT
	PS1 = ರಿಕ್ತ;
#ಅನ್ಯಥಾ
	PS1 = env_get("PS1");
	ಯದಿ(PS1==0)
		PS1 = "\\w \\$ ";
#endif
}

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಶೂನ್ಯ setup_prompt_string(ಪೂರ್ಣಾಙ್ಕ promptmode, ವರ್ಣ **prompt_str)
{
	debug_printf("setup_prompt_string %d ",promptmode);
#ifndef CONFIG_FEATURE_SH_FANCY_PROMPT
	/* Set up the prompt */
	ಯದಿ (promptmode == 1) {
		ಮುಕ್ತ(PS1);
		PS1=xmalloc(strlen(cwd)+4);
		sprintf(PS1, "%s %s", cwd, ( geteuid() != 0 ) ?  "$ ":"# ");
		*prompt_str = PS1;
	} ಅನ್ಯಥಾ {
		*prompt_str = PS2;
	}
#ಅನ್ಯಥಾ
	*prompt_str = (promptmode==1)? PS1 : PS2;
#endif
	debug_printf("result %s\n",*prompt_str);
}
#endif

#ifdef __U_BOOT__
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ uboot_cli_readline(ಸಂರಚನಾ in_str *i)
{
	ವರ್ಣ *prompt;
	ವರ್ಣ __maybe_unused *ps_prompt = ರಿಕ್ತ;

	ಯದಿ (i->promptmode == 1)
		prompt = CONFIG_SYS_PROMPT;
	ಅನ್ಯಥಾ
		prompt = CONFIG_SYS_PROMPT_HUSH_PS2;

#ifdef CONFIG_CMDLINE_PS_SUPPORT
	ಯದಿ (i->promptmode == 1)
		ps_prompt = env_get("PS1");
	ಅನ್ಯಥಾ
		ps_prompt = env_get("PS2");
	ಯದಿ (ps_prompt)
		prompt = ps_prompt;
#endif

	ಪ್ರತ್ಯಾಗಮ cli_readline(prompt);
}
#endif

ಸ್ಥಿರ ಶೂನ್ಯ get_user_ಒಳಹರಿವು(ಸಂರಚನಾ in_str *i)
{
#ifndef __U_BOOT__
	ವರ್ಣ *prompt_str;
	ಸ್ಥಿರ ವರ್ಣ the_command[BUFSIZ];

	setup_prompt_string(i->promptmode, &prompt_str);
#ifdef CONFIG_FEATURE_COMMAND_EDITING
	/*
	 ** enable command line editing only while a command line
	 ** is actually being read; otherwise, we'll end up bequeathing
	 ** atexit() handlers and other unwanted stuff to our
	 ** child processes (rob@sysgo.de)
	 */
	cmdedit_read_ಒಳಹರಿವು(prompt_str, the_command);
#ಅನ್ಯಥಾ
	fputs(prompt_str, stdout);
	fflush(stdout);
	the_command[0]=fgetc(i->file);
	the_command[1]='\0';
#endif
	fflush(stdout);
	i->p = the_command;
#ಅನ್ಯಥಾ
	ಪೂರ್ಣಾಙ್ಕ n;
	ಸ್ಥಿರ ವರ್ಣ the_command[CONFIG_SYS_CBSIZE + 1];

	bootretry_reset_cmd_timeout();
	i->__promptme = 1;
	n = uboot_cli_readline(i);

#ifdef CONFIG_BOOT_RETRY_TIME
	ಯದಿ (n == -2) {
	  puts("\nTimeout waiting for command\n");
#  ifdef CONFIG_RESET_TO_RETRY
	  do_reset(ರಿಕ್ತ, 0, 0, ರಿಕ್ತ);
#  elif IS_ENABLED(CONFIG_RETRY_BOOTCMD)
	strcpy(console_buffer, "run bootcmd\n");
# ಅನ್ಯಥಾ
#	error "This only works with CONFIG_RESET_TO_RETRY or CONFIG_BOOT_RETRY_COMMAND enabled"
#  endif
	}
#endif
	ಯದಿ (n == -1 ) {
		flag_repeat = 0;
		i->__promptme = 0;
	}
	n = strlen(console_buffer);
	console_buffer[n] = '\n';
	console_buffer[n+1]= '\0';
	ಯದಿ (had_ctrlc()) flag_repeat = 0;
	clear_ctrlc();
	do_repeat = 0;
	ಯದಿ (i->promptmode == 1) {
		ಯದಿ (console_buffer[0] == '\n'&& flag_repeat == 0) {
			strcpy(the_command,console_buffer);
		}
		ಅನ್ಯಥಾ {
			ಯದಿ (console_buffer[0] != '\n') {
				strcpy(the_command,console_buffer);
				flag_repeat = 1;
			}
			ಅನ್ಯಥಾ {
				do_repeat = 1;
			}
		}
		i->p = the_command;
	}
	ಅನ್ಯಥಾ {
		ಯದಿ (console_buffer[0] != '\n') {
			ಯದಿ (strlen(the_command) + strlen(console_buffer)
			    < CONFIG_SYS_CBSIZE) {
				n = strlen(the_command);
				the_command[n-1] = ' ';
				strcpy(&the_command[n],console_buffer);
			}
			ಅನ್ಯಥಾ {
				the_command[0] = '\n';
				the_command[1] = '\0';
				flag_repeat = 0;
			}
		}
		ಯದಿ (i->__promptme == 0) {
			the_command[0] = '\n';
			the_command[1] = '\0';
		}
		i->p = console_buffer;
	}
#endif
}

/* This is the magic location that prints prompts
 * and gets data back from the user */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ file_get(ಸಂರಚನಾ in_str *i)
{
	ಪೂರ್ಣಾಙ್ಕ ch;

	ch = 0;
	/* If there is data waiting, eat it up */
	ಯದಿ (i->p && *i->p) {
		ch = *i->p++;
	} ಅನ್ಯಥಾ {
		/* need to double check i->file because we might be doing something
		 * more complicated by now, like sourcing or substituting. */
#ifndef __U_BOOT__
		ಯದಿ (i->__promptme && interactive && i->file == stdin) {
			ಯಾವತ್(! i->p || (interactive && strlen(i->p)==0) ) {
#ಅನ್ಯಥಾ
			ಯಾವತ್(! i->p  || strlen(i->p)==0 ) {
#endif
				get_user_ಒಳಹರಿವು(i);
			}
			i->promptmode=2;
#ifndef __U_BOOT__
			i->__promptme = 0;
#endif
			ಯದಿ (i->p && *i->p) {
				ch = *i->p++;
			}
#ifndef __U_BOOT__
		} ಅನ್ಯಥಾ {
			ch = fgetc(i->file);
		}

#endif
		debug_printf("b_getch: got a %d\n", ch);
	}
#ifndef __U_BOOT__
	ಯದಿ (ch == '\n') i->__promptme=1;
#endif
	ಪ್ರತ್ಯಾಗಮ ch;
}

/* All the callers guarantee this routine will never be
 * used right after a newline, so prompting is not needed.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ file_peek(ಸಂರಚನಾ in_str *i)
{
#ifndef __U_BOOT__
	ಯದಿ (i->p && *i->p) {
#endif
		ಪ್ರತ್ಯಾಗಮ *i->p;
#ifndef __U_BOOT__
	} ಅನ್ಯಥಾ {
		i->peek_buf[0] = fgetc(i->file);
		i->peek_buf[1] = '\0';
		i->p = i->peek_buf;
		debug_printf("b_peek: got a %d\n", *i->p);
		ಪ್ರತ್ಯಾಗಮ *i->p;
	}
#endif
}

#ifndef __U_BOOT__
ಸ್ಥಿರ ಶೂನ್ಯ setup_file_in_str(ಸಂರಚನಾ in_str *i, FILE *f)
#ಅನ್ಯಥಾ
ಸ್ಥಿರ ಶೂನ್ಯ setup_file_in_str(ಸಂರಚನಾ in_str *i)
#endif
{
	i->peek = file_peek;
	i->get = file_get;
	i->__promptme=1;
	i->promptmode=1;
#ifndef __U_BOOT__
	i->file = f;
#endif
	i->p = ರಿಕ್ತ;
}

ಸ್ಥಿರ ಶೂನ್ಯ setup_string_in_str(ಸಂರಚನಾ in_str *i, ಧ್ರುವ ವರ್ಣ *s)
{
	i->peek = static_peek;
	i->get = static_get;
	i->__promptme=1;
	i->promptmode=1;
	i->p = s;
}

#ifndef __U_BOOT__
ಸ್ಥಿರ ಶೂನ್ಯ mark_open(ಪೂರ್ಣಾಙ್ಕ fd)
{
	ಸಂರಚನಾ close_me *new = xmalloc(ಪರಿಮಾಣ(ಸಂರಚನಾ close_me));
	new->fd = fd;
	new->next = close_me_head;
	close_me_head = new;
}

ಸ್ಥಿರ ಶೂನ್ಯ mark_closed(ಪೂರ್ಣಾಙ್ಕ fd)
{
	ಸಂರಚನಾ close_me *tmp;
	ಯದಿ (close_me_head == ರಿಕ್ತ || close_me_head->fd != fd)
		error_ಸಂದೇಶ_and_die("corrupt close_me");
	tmp = close_me_head;
	close_me_head = close_me_head->next;
	ಮುಕ್ತ(tmp);
}

ಸ್ಥಿರ ಶೂನ್ಯ close_all(ಶೂನ್ಯ)
{
	ಸಂರಚನಾ close_me *c;
	ಪ್ರತಿ (c=close_me_head; c; c=c->next) {
		close(c->fd);
	}
	close_me_head = ರಿಕ್ತ;
}

/* squirrel != NULL means we squirrel away copies of stdin, stdout,
 * and stderr if they are redirected. */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ setup_redirects(ಸಂರಚನಾ child_prog *prog, ಪೂರ್ಣಾಙ್ಕ squirrel[])
{
	ಪೂರ್ಣಾಙ್ಕ openfd, mode;
	ಸಂರಚನಾ redir_struct *redir;

	ಪ್ರತಿ (redir=prog->redirects; redir; redir=redir->next) {
		ಯದಿ (redir->dup == -1 && redir->word.gl_pathv == ರಿಕ್ತ) {
			/* something went wrong in the parse.  Pretend it didn't happen */
			ಅನುವರ್ತ;
		}
		ಯದಿ (redir->dup == -1) {
			mode=redir_table[redir->type].mode;
			openfd = open(redir->word.gl_pathv[0], mode, 0666);
			ಯದಿ (openfd < 0) {
			/* this could get lost if stderr has been redirected, but
			   bash and ash both lose it as well (though zsh doesn't!) */
				perror_ಸಂದೇಶ("error opening %s", redir->word.gl_pathv[0]);
				ಪ್ರತ್ಯಾಗಮ 1;
			}
		} ಅನ್ಯಥಾ {
			openfd = redir->dup;
		}

		ಯದಿ (openfd != redir->fd) {
			ಯದಿ (squirrel && redir->fd < 3) {
				squirrel[redir->fd] = dup(redir->fd);
			}
			ಯದಿ (openfd == -3) {
				close(openfd);
			} ಅನ್ಯಥಾ {
				dup2(openfd, redir->fd);
				ಯದಿ (redir->dup == -1)
					close (openfd);
			}
		}
	}
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಶೂನ್ಯ restore_redirects(ಪೂರ್ಣಾಙ್ಕ squirrel[])
{
	ಪೂರ್ಣಾಙ್ಕ i, fd;
	ಪ್ರತಿ (i=0; i<3; i++) {
		fd = squirrel[i];
		ಯದಿ (fd != -1) {
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
ಸ್ಥಿರ ಶೂನ್ಯ pseudo_exec(ಸಂರಚನಾ child_prog *child)
{
	ಪೂರ್ಣಾಙ್ಕ i, rcode;
	ವರ್ಣ *p;
	ಸಂರಚನಾ built_in_command *x;
	ಯದಿ (child->argv) {
		ಪ್ರತಿ (i=0; is_assignment(child->argv[i]); i++) {
			debug_printf("pid %d environment modification: %s\n",getpid(),child->argv[i]);
			p = ಸೇರಿಸು_var_ಮೌಲ್ಯ(child->argv[i]);
			putenv(strdup(p));
			ಯದಿ (p != child->argv[i]) ಮುಕ್ತ(p);
		}
		child->argv+=i;  /* XXX this hack isn't so horrible, since we are about
					to exit, and therefore don't need to keep data
					structures consistent for free() use. */
		/* If a variable is assigned in a forest, and nobody listens,
		 * was it ever really set?
		 */
		ಯದಿ (child->argv[0] == ರಿಕ್ತ) {
			_exit(EXIT_SUCCESS);
		}

		/*
		 * Check if the command matches any of the builtins.
		 * Depending on context, this might be redundant.  But it's
		 * easier to waste a few CPU cycles than it is to figure out
		 * if this is one of those cases.
		 */
		ಪ್ರತಿ (x = bltins; x->cmd; x++) {
			ಯದಿ (strcmp(child->argv[0], x->cmd) == 0 ) {
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
			ಪೂರ್ಣಾಙ್ಕ argc_l;
			ವರ್ಣ** argv_l=child->argv;
			ವರ್ಣ *name = child->argv[0];

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
			ಪ್ರತಿ(argc_l=0;*argv_l!=ರಿಕ್ತ; argv_l++, argc_l++);
			optind = 1;
			debug_printf("running applet %s\n", name);
			run_applet_by_name(name, argc_l, child->argv);
		}
#endif
		debug_printf("exec of %s\n",child->argv[0]);
		execvp(child->argv[0],child->argv);
		perror_ಸಂದೇಶ("couldn't exec: %s",child->argv[0]);
		_exit(1);
	} ಅನ್ಯಥಾ ಯದಿ (child->group) {
		debug_printf("runtime nesting to group\n");
		interactive=0;    /* crucial!!!! */
		rcode = run_list_real(child->group);
		/* OK to leak memory by not calling free_pipe_list,
		 * since this process is about to exit */
		_exit(rcode);
	} ಅನ್ಯಥಾ {
		/* Can happen.  See what bash does with ">foo" by itself. */
		debug_printf("trying to pseudo_exec null command\n");
		_exit(EXIT_SUCCESS);
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ ಸೇರಿಸು_bg_job(ಸಂರಚನಾ pipe *pi)
{
	ಸಂರಚನಾ pipe *thejob;

	/* Linear search for the ID of the job to use */
	pi->jobid = 1;
	ಪ್ರತಿ (thejob = job_list; thejob; thejob = thejob->next)
		ಯದಿ (thejob->jobid >= pi->jobid)
			pi->jobid = thejob->jobid + 1;

	/* add thejob to the list of running jobs */
	ಯದಿ (!job_list) {
		thejob = job_list = xmalloc(ಪರಿಮಾಣ(*thejob));
	} ಅನ್ಯಥಾ {
		ಪ್ರತಿ (thejob = job_list; thejob->next; thejob = thejob->next) /* nothing */;
		thejob->next = xmalloc(ಪರಿಮಾಣ(*thejob));
		thejob = thejob->next;
	}

	/* physically copy the struct job */
	memcpy(thejob, pi, ಪರಿಮಾಣ(ಸಂರಚನಾ pipe));
	thejob->next = ರಿಕ್ತ;
	thejob->running_progs = thejob->ಅಂಕೆ_progs;
	thejob->stopped_progs = 0;
	thejob->text = xmalloc(BUFSIZ); /* cmdedit buffer size */

	/*if (pi->progs[0] && pi->progs[0].argv && pi->progs[0].argv[0]) */
	{
		ವರ್ಣ *bar=thejob->text;
		ವರ್ಣ **foo=pi->progs[0].argv;
		ಯಾವತ್(foo && *foo) {
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
ಸ್ಥಿರ ಶೂನ್ಯ remove_bg_job(ಸಂರಚನಾ pipe *pi)
{
	ಸಂರಚನಾ pipe *prev_pipe;

	ಯದಿ (pi == job_list) {
		job_list = pi->next;
	} ಅನ್ಯಥಾ {
		prev_pipe = job_list;
		ಯಾವತ್ (prev_pipe->next != pi)
			prev_pipe = prev_pipe->next;
		prev_pipe->next = pi->next;
	}
	ಯದಿ (job_list)
		last_jobid = job_list->jobid;
	ಅನ್ಯಥಾ
		last_jobid = 0;

	pi->stopped_progs = 0;
	free_pipe(pi, 0);
	ಮುಕ್ತ(pi);
}

/* Checks to see if any processes have exited -- if they
   have, figure out why and see if a job has completed */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ checkjobs(ಸಂರಚನಾ pipe* fg_pipe)
{
	ಪೂರ್ಣಾಙ್ಕ attributes;
	ಪೂರ್ಣಾಙ್ಕ status;
	ಪೂರ್ಣಾಙ್ಕ prognum = 0;
	ಸಂರಚನಾ pipe *pi;
	pid_t childpid;

	attributes = WUNTRACED;
	ಯದಿ (fg_pipe==ರಿಕ್ತ) {
		attributes |= WNOHANG;
	}

	ಯಾವತ್ ((childpid = waitpid(-1, &status, attributes)) > 0) {
		ಯದಿ (fg_pipe) {
			ಪೂರ್ಣಾಙ್ಕ i, rcode = 0;
			ಪ್ರತಿ (i=0; i < fg_pipe->ಅಂಕೆ_progs; i++) {
				ಯದಿ (fg_pipe->progs[i].pid == childpid) {
					ಯದಿ (i==fg_pipe->ಅಂಕೆ_progs-1)
						rcode=WEXITSTATUS(status);
					(fg_pipe->ಅಂಕೆ_progs)--;
					ಪ್ರತ್ಯಾಗಮ(rcode);
				}
			}
		}

		ಪ್ರತಿ (pi = job_list; pi; pi = pi->next) {
			prognum = 0;
			ಯಾವತ್ (prognum < pi->ಅಂಕೆ_progs && pi->progs[prognum].pid != childpid) {
				prognum++;
			}
			ಯದಿ (prognum < pi->ಅಂಕೆ_progs)
				ಭಙ್ಗ;
		}

		ಯದಿ(pi==ರಿಕ್ತ) {
			debug_printf("checkjobs: pid %d was not in our list!\n", childpid);
			ಅನುವರ್ತ;
		}

		ಯದಿ (WIFEXITED(status) || WIFSIGNALED(status)) {
			/* child exited */
			pi->running_progs--;
			pi->progs[prognum].pid = 0;

			ಯದಿ (!pi->running_progs) {
				printf(JOB_STATUS_FORMAT, pi->jobid, "Done", pi->text);
				remove_bg_job(pi);
			}
		} ಅನ್ಯಥಾ {
			/* child stopped */
			pi->stopped_progs++;
			pi->progs[prognum].is_stopped = 1;

#ಯದಿ 0
			/* Printing this stuff is a pain, since it tends to
			 * overwrite the prompt an inconveinient moments.  So
			 * don't do that.  */
			ಯದಿ (pi->stopped_progs == pi->ಅಂಕೆ_progs) {
				printf("\n"JOB_STATUS_FORMAT, pi->jobid, "Stopped", pi->text);
			}
#endif
		}
	}

	ಯದಿ (childpid == -1 && errno != ECHILD)
		perror_ಸಂದೇಶ("waitpid");

	/* move the shell to the foreground */
	/*if (interactive && tcsetpgrp(shell_terminal, getpgid(0))) */
	/*	perror_msg("tcsetpgrp-2"); */
	ಪ್ರತ್ಯಾಗಮ -1;
}

/* Figure out our controlling tty, checking in order stderr,
 * stdin, and stdout.  If check_pgrp is set, also check that
 * we belong to the foreground process group associated with
 * that tty.  The value of shell_terminal is needed in order to call
 * tcsetpgrp(shell_terminal, ...); */
ಶೂನ್ಯ controlling_tty(ಪೂರ್ಣಾಙ್ಕ check_pgrp)
{
	pid_t curpgrp;

	ಯದಿ ((curpgrp = tcgetpgrp(shell_terminal = 2)) < 0
			&& (curpgrp = tcgetpgrp(shell_terminal = 0)) < 0
			&& (curpgrp = tcgetpgrp(shell_terminal = 1)) < 0)
		ಗಚ್ಛ shell_terminal_error;

	ಯದಿ (check_pgrp && curpgrp != getpgid(0))
		ಗಚ್ಛ shell_terminal_error;

	ಪ್ರತ್ಯಾಗಮ;

shell_terminal_error:
		shell_terminal = -1;
		ಪ್ರತ್ಯಾಗಮ;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ run_pipe_real(ಸಂರಚನಾ pipe *pi)
{
	ಪೂರ್ಣಾಙ್ಕ i;
#ifndef __U_BOOT__
	ಪೂರ್ಣಾಙ್ಕ nextin, nextout;
	ಪೂರ್ಣಾಙ್ಕ pipefds[2];				/* pipefds[0] is for reading */
	ಸಂರಚನಾ child_prog *child;
	ಸಂರಚನಾ built_in_command *x;
	ವರ್ಣ *p;
# ಯದಿ __GNUC__
	/* Avoid longjmp clobbering */
	(ಶೂನ್ಯ) &i;
	(ಶೂನ್ಯ) &nextin;
	(ಶೂನ್ಯ) &nextout;
	(ಶೂನ್ಯ) &child;
# endif
#ಅನ್ಯಥಾ
	ಪೂರ್ಣಾಙ್ಕ nextin;
	ಪೂರ್ಣಾಙ್ಕ flag = do_repeat ? CMD_FLAG_REPEAT : 0;
	ಸಂರಚನಾ child_prog *child;
	ವರ್ಣ *p;
# ಯದಿ __GNUC__
	/* Avoid longjmp clobbering */
	(ಶೂನ್ಯ) &i;
	(ಶೂನ್ಯ) &nextin;
	(ಶೂನ್ಯ) &child;
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
	ಯದಿ (pi->ಅಂಕೆ_progs == 1) child = & (pi->progs[0]);
#ifndef __U_BOOT__
	ಯದಿ (pi->ಅಂಕೆ_progs == 1 && child->group && child->subshell == 0) {
		ಪೂರ್ಣಾಙ್ಕ squirrel[] = {-1, -1, -1};
		ಪೂರ್ಣಾಙ್ಕ rcode;
		debug_printf("non-subshell grouping\n");
		setup_redirects(child, squirrel);
		/* XXX could we merge code with following builtin case,
		 * by creating a pseudo builtin that calls run_list_real? */
		rcode = run_list_real(child->group);
		restore_redirects(squirrel);
#ಅನ್ಯಥಾ
		ಯದಿ (pi->ಅಂಕೆ_progs == 1 && child->group) {
		ಪೂರ್ಣಾಙ್ಕ rcode;
		debug_printf("non-subshell grouping\n");
		rcode = run_list_real(child->group);
#endif
		ಪ್ರತ್ಯಾಗಮ rcode;
	} ಅನ್ಯಥಾ ಯದಿ (pi->ಅಂಕೆ_progs == 1 && pi->progs[0].argv != ರಿಕ್ತ) {
		ಪ್ರತಿ (i=0; is_assignment(child->argv[i]); i++) { /* nothing */ }
		ಯದಿ (i!=0 && child->argv[i]==ರಿಕ್ತ) {
			/* assignments, but no command: set the local environment */
			ಪ್ರತಿ (i=0; child->argv[i]!=ರಿಕ್ತ; i++) {

				/* Ok, this case is tricky.  We have to decide if this is a
				 * local variable, or an already exported variable.  If it is
				 * already exported, we have to export the new value.  If it is
				 * not exported, we need only set this as a local variable.
				 * This junk is all to decide whether or not to export this
				 * variable. */
				ಪೂರ್ಣಾಙ್ಕ export_me=0;
				ವರ್ಣ *name, *ಮೌಲ್ಯ;
				name = xstrdup(child->argv[i]);
				debug_printf("Local environment set: %s\n", name);
				ಮೌಲ್ಯ = strchr(name, '=');
				ಯದಿ (ಮೌಲ್ಯ)
					*ಮೌಲ್ಯ=0;
#ifndef __U_BOOT__
				ಯದಿ ( get_local_var(name)) {
					export_me=1;
				}
#endif
				ಮುಕ್ತ(name);
				p = ಸೇರಿಸು_var_ಮೌಲ್ಯ(child->argv[i]);
				set_local_var(p, export_me);
				ಯದಿ (p != child->argv[i]) ಮುಕ್ತ(p);
			}
			ಪ್ರತ್ಯಾಗಮ EXIT_SUCCESS;   /* don't worry about errors in set_local_var() yet */
		}
		ಪ್ರತಿ (i = 0; is_assignment(child->argv[i]); i++) {
			p = ಸೇರಿಸು_var_ಮೌಲ್ಯ(child->argv[i]);
#ifndef __U_BOOT__
			putenv(strdup(p));
#ಅನ್ಯಥಾ
			set_local_var(p, 0);
#endif
			ಯದಿ (p != child->argv[i]) {
				child->sp--;
				ಮುಕ್ತ(p);
			}
		}
		ಯದಿ (child->sp) {
			ವರ್ಣ * str = ರಿಕ್ತ;

			str = make_string(child->argv + i,
					  child->argv_nonnull + i);
			parse_string_outer(str, FLAG_EXIT_FROM_LOOP | FLAG_REPARSING);
			ಮುಕ್ತ(str);
			ಪ್ರತ್ಯಾಗಮ last_return_code;
		}
#ifndef __U_BOOT__
		ಪ್ರತಿ (x = bltins; x->cmd; x++) {
			ಯದಿ (strcmp(child->argv[i], x->cmd) == 0 ) {
				ಪೂರ್ಣಾಙ್ಕ squirrel[] = {-1, -1, -1};
				ಪೂರ್ಣಾಙ್ಕ rcode;
				ಯದಿ (x->function == builtin_exec && child->argv[i+1]==ರಿಕ್ತ) {
					debug_printf("magic exec\n");
					setup_redirects(child,ರಿಕ್ತ);
					ಪ್ರತ್ಯಾಗಮ EXIT_SUCCESS;
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
			ಪ್ರತ್ಯಾಗಮ rcode;
		}
#ಅನ್ಯಥಾ
		/* check ";", because ,example , argv consist from
		 * "help;flinfo" must not execute
		 */
		ಯದಿ (strchr(child->argv[i], ';')) {
			printf("Unknown command '%s' - try 'help' or use "
					"'run' command\n", child->argv[i]);
			ಪ್ರತ್ಯಾಗಮ -1;
		}
		/* Process the command */
		ಪ್ರತ್ಯಾಗಮ cmd_process(flag, child->argc - i, child->argv + i,
				   &flag_repeat, ರಿಕ್ತ);
#endif
	}
#ifndef __U_BOOT__

	ಪ್ರತಿ (i = 0; i < pi->ಅಂಕೆ_progs; i++) {
		child = & (pi->progs[i]);

		/* pipes are inserted between pairs of commands */
		ಯದಿ ((i + 1) < pi->ಅಂಕೆ_progs) {
			ಯದಿ (pipe(pipefds)<0) perror_ಸಂದೇಶ_and_die("pipe");
			nextout = pipefds[1];
		} ಅನ್ಯಥಾ {
			nextout=1;
			pipefds[0] = -1;
		}

		/* XXX test for failed fork()? */
		ಯದಿ (!(child->pid = fork())) {
			/* Set the handling for job control signals back to the default.  */
			ಸಙ್ಕೇತ(SIGINT, SIG_DFL);
			ಸಙ್ಕೇತ(SIGQUIT, SIG_DFL);
			ಸಙ್ಕೇತ(SIGTERM, SIG_DFL);
			ಸಙ್ಕೇತ(SIGTSTP, SIG_DFL);
			ಸಙ್ಕೇತ(SIGTTIN, SIG_DFL);
			ಸಙ್ಕೇತ(SIGTTOU, SIG_DFL);
			ಸಙ್ಕೇತ(SIGCHLD, SIG_DFL);

			close_all();

			ಯದಿ (nextin != 0) {
				dup2(nextin, 0);
				close(nextin);
			}
			ಯದಿ (nextout != 1) {
				dup2(nextout, 1);
				close(nextout);
			}
			ಯದಿ (pipefds[0]!=-1) {
				close(pipefds[0]);  /* opposite end of our output pipe */
			}

			/* Like bash, explicit redirects override pipes,
			 * and the pipe fd is available for dup'ing. */
			setup_redirects(child,ರಿಕ್ತ);

			ಯದಿ (interactive && pi->followup!=PIPE_BG) {
				/* If we (the child) win the race, put ourselves in the process
				 * group whose leader is the first process in this pipe. */
				ಯದಿ (pi->pgrp < 0) {
					pi->pgrp = getpid();
				}
				ಯದಿ (setpgid(0, pi->pgrp) == 0) {
					tcsetpgrp(2, pi->pgrp);
				}
			}

			pseudo_exec(child);
		}

		/* put our child in the process group whose leader is the
		   first process in this pipe */
		ಯದಿ (pi->pgrp < 0) {
			pi->pgrp = child->pid;
		}
		/* Don't check for errors.  The child may be dead already,
		 * in which case setpgid returns error code EACCES. */
		setpgid(child->pid, pi->pgrp);

		ಯದಿ (nextin != 0)
			close(nextin);
		ಯದಿ (nextout != 1)
			close(nextout);

		/* If there isn't another process, nextin is garbage
		   but it doesn't matter */
		nextin = pipefds[0];
	}
#endif
	ಪ್ರತ್ಯಾಗಮ -1;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ run_list_real(ಸಂರಚನಾ pipe *pi)
{
	ವರ್ಣ *save_name = ರಿಕ್ತ;
	ವರ್ಣ **list = ರಿಕ್ತ;
	ವರ್ಣ **save_list = ರಿಕ್ತ;
	ಸಂರಚನಾ pipe *rpipe;
	ಪೂರ್ಣಾಙ್ಕ flag_rep = 0;
#ifndef __U_BOOT__
	ಪೂರ್ಣಾಙ್ಕ save_ಅಂಕೆ_progs;
#endif
	ಪೂರ್ಣಾಙ್ಕ rcode=0, flag_skip=1;
	ಪೂರ್ಣಾಙ್ಕ flag_restore = 0;
	ಪೂರ್ಣಾಙ್ಕ if_code=0, next_if_code=0;  /* need double-buffer to handle elif */
	reserved_style rmode, skip_more_in_this_rmode=RES_XXXX;
	/* check syntax for "for" */
	ಪ್ರತಿ (rpipe = pi; rpipe; rpipe = rpipe->next) {
		ಯದಿ ((rpipe->r_mode == RES_IN ||
		    rpipe->r_mode == RES_FOR) &&
		    (rpipe->next == ರಿಕ್ತ)) {
				syntax();
#ifdef __U_BOOT__
				flag_repeat = 0;
#endif
				ಪ್ರತ್ಯಾಗಮ 1;
		}
		ಯದಿ ((rpipe->r_mode == RES_IN &&
			(rpipe->next->r_mode == RES_IN &&
			rpipe->next->progs->argv != ರಿಕ್ತ))||
			(rpipe->r_mode == RES_FOR &&
			rpipe->next->r_mode != RES_IN)) {
				syntax();
#ifdef __U_BOOT__
				flag_repeat = 0;
#endif
				ಪ್ರತ್ಯಾಗಮ 1;
		}
	}
	ಪ್ರತಿ (; pi; pi = (flag_restore != 0) ? rpipe : pi->next) {
		ಯದಿ (pi->r_mode == RES_WHILE || pi->r_mode == RES_UNTIL ||
			pi->r_mode == RES_FOR) {
#ifdef __U_BOOT__
				/* check Ctrl-C */
				ctrlc();
				ಯದಿ ((had_ctrlc())) {
					ಪ್ರತ್ಯಾಗಮ 1;
				}
#endif
				flag_restore = 0;
				ಯದಿ (!rpipe) {
					flag_rep = 0;
					rpipe = pi;
				}
		}
		rmode = pi->r_mode;
		debug_printf("rmode=%d  if_code=%d  next_if_code=%d skip_more=%d\n", rmode, if_code, next_if_code, skip_more_in_this_rmode);
		ಯದಿ (rmode == skip_more_in_this_rmode && flag_skip) {
			ಯದಿ (pi->followup == PIPE_SEQ) flag_skip=0;
			ಅನುವರ್ತ;
		}
		flag_skip = 1;
		skip_more_in_this_rmode = RES_XXXX;
		ಯದಿ (rmode == RES_THEN || rmode == RES_ELSE) if_code = next_if_code;
		ಯದಿ (rmode == RES_THEN &&  if_code) ಅನುವರ್ತ;
		ಯದಿ (rmode == RES_ELSE && !if_code) ಅನುವರ್ತ;
		ಯದಿ (rmode == RES_ELIF && !if_code) ಭಙ್ಗ;
		ಯದಿ (rmode == RES_FOR && pi->ಅಂಕೆ_progs) {
			ಯದಿ (!list) {
				/* if no variable values after "in" we skip "for" */
				ಯದಿ (!pi->next->progs->argv) ಅನುವರ್ತ;
				/* create list of variable values */
				list = make_list_in(pi->next->progs->argv,
					pi->progs->argv[0]);
				save_list = list;
				save_name = pi->progs->argv[0];
				pi->progs->argv[0] = ರಿಕ್ತ;
				flag_rep = 1;
			}
			ಯದಿ (!(*list)) {
				ಮುಕ್ತ(pi->progs->argv[0]);
				ಮುಕ್ತ(save_list);
				list = ರಿಕ್ತ;
				flag_rep = 0;
				pi->progs->argv[0] = save_name;
#ifndef __U_BOOT__
				pi->progs->glob_result.gl_pathv[0] =
					pi->progs->argv[0];
#endif
				ಅನುವರ್ತ;
			} ಅನ್ಯಥಾ {
				/* insert new value from list for variable */
				ಮುಕ್ತ(pi->progs->argv[0]);
				pi->progs->argv[0] = *list++;
#ifndef __U_BOOT__
				pi->progs->glob_result.gl_pathv[0] =
					pi->progs->argv[0];
#endif
			}
		}
		ಯದಿ (rmode == RES_IN) ಅನುವರ್ತ;
		ಯದಿ (rmode == RES_DO) {
			ಯದಿ (!flag_rep) ಅನುವರ್ತ;
		}
		ಯದಿ (rmode == RES_DONE) {
			ಯದಿ (flag_rep) {
				flag_restore = 1;
			} ಅನ್ಯಥಾ {
				rpipe = ರಿಕ್ತ;
			}
		}
		ಯದಿ (pi->ಅಂಕೆ_progs == 0) ಅನುವರ್ತ;
#ifndef __U_BOOT__
		save_ಅಂಕೆ_progs = pi->ಅಂಕೆ_progs; /* save number of programs */
#endif
		rcode = run_pipe_real(pi);
		debug_printf("run_pipe_real returned %d\n",rcode);
#ifndef __U_BOOT__
		ಯದಿ (rcode!=-1) {
			/* We only ran a builtin: rcode was set by the return value
			 * of run_pipe_real(), and we don't need to wait for anything. */
		} ಅನ್ಯಥಾ ಯದಿ (pi->followup==PIPE_BG) {
			/* XXX check bash's behavior with nontrivial pipes */
			/* XXX compute jobid */
			/* XXX what does bash do with attempts to background builtins? */
			ಸೇರಿಸು_bg_job(pi);
			rcode = EXIT_SUCCESS;
		} ಅನ್ಯಥಾ {
			ಯದಿ (interactive) {
				/* move the new process group into the foreground */
				ಯದಿ (tcsetpgrp(shell_terminal, pi->pgrp) && errno != ENOTTY)
					perror_ಸಂದೇಶ("tcsetpgrp-3");
				rcode = checkjobs(pi);
				/* move the shell to the foreground */
				ಯದಿ (tcsetpgrp(shell_terminal, getpgid(0)) && errno != ENOTTY)
					perror_ಸಂದೇಶ("tcsetpgrp-4");
			} ಅನ್ಯಥಾ {
				rcode = checkjobs(pi);
			}
			debug_printf("checkjobs returned %d\n",rcode);
		}
		last_return_code=rcode;
#ಅನ್ಯಥಾ
		ಯದಿ (rcode < -1) {
			last_return_code = -rcode - 2;
			ಪ್ರತ್ಯಾಗಮ -2;	/* exit */
		}
		last_return_code = rcode;
#endif
#ifndef __U_BOOT__
		pi->ಅಂಕೆ_progs = save_ಅಂಕೆ_progs; /* restore number of programs */
#endif
		ಯದಿ ( rmode == RES_IF || rmode == RES_ELIF )
			next_if_code=rcode;  /* can be overwritten a number of times */
		ಯದಿ (rmode == RES_WHILE)
			flag_rep = !last_return_code;
		ಯದಿ (rmode == RES_UNTIL)
			flag_rep = last_return_code;
		ಯದಿ ( (rcode==EXIT_SUCCESS && pi->followup==PIPE_OR) ||
		     (rcode!=EXIT_SUCCESS && pi->followup==PIPE_AND) )
			skip_more_in_this_rmode=rmode;
#ifndef __U_BOOT__
		checkjobs(ರಿಕ್ತ);
#endif
	}
	ಪ್ರತ್ಯಾಗಮ rcode;
}

/* broken, of course, but OK for testing */
ಸ್ಥಿರ ವರ್ಣ *indenter(ಪೂರ್ಣಾಙ್ಕ i)
{
	ಸ್ಥಿರ ವರ್ಣ blanks[]="                                    ";
	ಪ್ರತ್ಯಾಗಮ &blanks[ಪರಿಮಾಣ(blanks)-i-1];
}

/* return code is the exit status of the pipe */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ free_pipe(ಸಂರಚನಾ pipe *pi, ಪೂರ್ಣಾಙ್ಕ indent)
{
	ವರ್ಣ **p;
	ಸಂರಚನಾ child_prog *child;
#ifndef __U_BOOT__
	ಸಂರಚನಾ redir_struct *r, *rnext;
#endif
	ಪೂರ್ಣಾಙ್ಕ a, i, ret_code=0;
	ವರ್ಣ *ind = indenter(indent);

#ifndef __U_BOOT__
	ಯದಿ (pi->stopped_progs > 0)
		ಪ್ರತ್ಯಾಗಮ ret_code;
	final_printf("%s run pipe: (pid %d)\n",ind,getpid());
#endif
	ಪ್ರತಿ (i=0; i<pi->ಅಂಕೆ_progs; i++) {
		child = &pi->progs[i];
		final_printf("%s  command %d:\n",ind,i);
		ಯದಿ (child->argv) {
			ಪ್ರತಿ (a=0,p=child->argv; *p; a++,p++) {
				final_printf("%s   argv[%d] = %s\n",ind,a,*p);
			}
#ifndef __U_BOOT__
			globfree(&child->glob_result);
#ಅನ್ಯಥಾ
			ಪ್ರತಿ (a = 0; a < child->argc; a++) {
				ಮುಕ್ತ(child->argv[a]);
			}
			ಮುಕ್ತ(child->argv);
			ಮುಕ್ತ(child->argv_nonnull);
			child->argc = 0;
#endif
			child->argv=ರಿಕ್ತ;
		} ಅನ್ಯಥಾ ಯದಿ (child->group) {
#ifndef __U_BOOT__
			final_printf("%s   begin group (subshell:%d)\n",ind, child->subshell);
#endif
			ret_code = free_pipe_list(child->group,indent+3);
			final_printf("%s   end group\n",ind);
		} ಅನ್ಯಥಾ {
			final_printf("%s   (nil)\n",ind);
		}
#ifndef __U_BOOT__
		ಪ್ರತಿ (r=child->redirects; r; r=rnext) {
			final_printf("%s   redirect %d%s", ind, r->fd, redir_table[r->type].descrip);
			ಯದಿ (r->dup == -1) {
				/* guard against the case >$FOO, where foo is unset or blank */
				ಯದಿ (r->word.gl_pathv) {
					final_printf(" %s\n", *r->word.gl_pathv);
					globfree(&r->word);
				}
			} ಅನ್ಯಥಾ {
				final_printf("&%d\n", r->dup);
			}
			rnext=r->next;
			ಮುಕ್ತ(r);
		}
		child->redirects=ರಿಕ್ತ;
#endif
	}
	ಮುಕ್ತ(pi->progs);   /* children are an array, they get freed all at once */
	pi->progs=ರಿಕ್ತ;
	ಪ್ರತ್ಯಾಗಮ ret_code;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ free_pipe_list(ಸಂರಚನಾ pipe *head, ಪೂರ್ಣಾಙ್ಕ indent)
{
	ಪೂರ್ಣಾಙ್ಕ rcode=0;   /* if list has no members */
	ಸಂರಚನಾ pipe *pi, *next;
	ವರ್ಣ *ind = indenter(indent);
	ಪ್ರತಿ (pi=head; pi; pi=next) {
		final_printf("%s pipe reserved mode %d\n", ind, pi->r_mode);
		rcode = free_pipe(pi, indent);
		final_printf("%s pipe followup code %d\n", ind, pi->followup);
		next=pi->next;
		pi->next=ರಿಕ್ತ;
		ಮುಕ್ತ(pi);
	}
	ಪ್ರತ್ಯಾಗಮ rcode;
}

/* Select which version we will use */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ run_list(ಸಂರಚನಾ pipe *pi)
{
	ಪೂರ್ಣಾಙ್ಕ rcode=0;
#ifndef __U_BOOT__
	ಯದಿ (fake_mode==0) {
#endif
		rcode = run_list_real(pi);
#ifndef __U_BOOT__
	}
#endif
	/* free_pipe_list has the side effect of clearing memory
	 * In the long run that function can be merged with run_list_real,
	 * but doing that now would hobble the debugging effort. */
	free_pipe_list(pi,0);
	ಪ್ರತ್ಯಾಗಮ rcode;
}

/* The API for glob is arguably broken.  This routine pushes a non-matching
 * string into the output structure, removing non-backslashed backslashes.
 * If someone can prove me wrong, by performing this function within the
 * original glob(3) api, feel free to rewrite this routine into oblivion.
 * Return code (0 vs. GLOB_NOSPACE) matches glob(3).
 * XXX broken if the last character is '\\', check that before calling.
 */
#ifndef __U_BOOT__
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ globhack(ಧ್ರುವ ವರ್ಣ *src, ಪೂರ್ಣಾಙ್ಕ flags, glob_t *pglob)
{
	ಪೂರ್ಣಾಙ್ಕ cnt=0, pathc;
	ಧ್ರುವ ವರ್ಣ *s;
	ವರ್ಣ *dest;
	ಪ್ರತಿ (cnt=1, s=src; s && *s; s++) {
		ಯದಿ (*s == '\\') s++;
		cnt++;
	}
	dest = malloc(cnt);
	ಯದಿ (!dest) ಪ್ರತ್ಯಾಗಮ GLOB_NOSPACE;
	ಯದಿ (!(flags & GLOB_APPEND)) {
		pglob->gl_pathv=ರಿಕ್ತ;
		pglob->gl_pathc=0;
		pglob->gl_offs=0;
		pglob->gl_offs=0;
	}
	pathc = ++pglob->gl_pathc;
	pglob->gl_pathv = realloc(pglob->gl_pathv, (pathc+1)*ಪರಿಮಾಣ(*pglob->gl_pathv));
	ಯದಿ (pglob->gl_pathv == ರಿಕ್ತ) ಪ್ರತ್ಯಾಗಮ GLOB_NOSPACE;
	pglob->gl_pathv[pathc-1]=dest;
	pglob->gl_pathv[pathc]=ರಿಕ್ತ;
	ಪ್ರತಿ (s=src; s && *s; s++, dest++) {
		ಯದಿ (*s == '\\') s++;
		*dest = *s;
	}
	*dest='\0';
	ಪ್ರತ್ಯಾಗಮ 0;
}

/* XXX broken if the last character is '\\', check that before calling */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ glob_needed(ಧ್ರುವ ವರ್ಣ *s)
{
	ಪ್ರತಿ (; *s; s++) {
		ಯದಿ (*s == '\\') s++;
		ಯದಿ (strchr("*[?",*s)) ಪ್ರತ್ಯಾಗಮ 1;
	}
	ಪ್ರತ್ಯಾಗಮ 0;
}

#ಯದಿ 0
ಸ್ಥಿರ ಶೂನ್ಯ globprint(glob_t *pglob)
{
	ಪೂರ್ಣಾಙ್ಕ i;
	debug_printf("glob_t at %p:\n", pglob);
	debug_printf("  gl_pathc=%d  gl_pathv=%p  gl_offs=%d  gl_flags=%d\n",
		pglob->gl_pathc, pglob->gl_pathv, pglob->gl_offs, pglob->gl_flags);
	ಪ್ರತಿ (i=0; i<pglob->gl_pathc; i++)
		debug_printf("pglob->gl_pathv[%d] = %p = %s\n", i,
			pglob->gl_pathv[i], pglob->gl_pathv[i]);
}
#endif

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ xglob(o_string *dest, ಪೂರ್ಣಾಙ್ಕ flags, glob_t *pglob)
{
	ಪೂರ್ಣಾಙ್ಕ gr;

	/* short-circuit for null word */
	/* we can code this better when the debug_printf's are gone */
	ಯದಿ (dest->length == 0) {
		ಯದಿ (dest->nonnull) {
			/* bash man page calls this an "explicit" null */
			gr = globhack(dest->data, flags, pglob);
			debug_printf("globhack returned %d\n",gr);
		} ಅನ್ಯಥಾ {
			ಪ್ರತ್ಯಾಗಮ 0;
		}
	} ಅನ್ಯಥಾ ಯದಿ (glob_needed(dest->data)) {
		gr = glob(dest->data, flags, ರಿಕ್ತ, pglob);
		debug_printf("glob returned %d\n",gr);
		ಯದಿ (gr == GLOB_NOMATCH) {
			/* quote removal, or more accurately, backslash removal */
			gr = globhack(dest->data, flags, pglob);
			debug_printf("globhack returned %d\n",gr);
		}
	} ಅನ್ಯಥಾ {
		gr = globhack(dest->data, flags, pglob);
		debug_printf("globhack returned %d\n",gr);
	}
	ಯದಿ (gr == GLOB_NOSPACE)
		error_ಸಂದೇಶ_and_die("out of memory during glob");
	ಯದಿ (gr != 0) { /* GLOB_ABORTED ? */
		error_ಸಂದೇಶ("glob(3) error %d",gr);
	}
	/* globprint(glob_target); */
	ಪ್ರತ್ಯಾಗಮ gr;
}
#endif

#ifdef __U_BOOT__
ಸ್ಥಿರ ವರ್ಣ *get_dollar_var(ವರ್ಣ ch);
#endif

/* This is used to get/check local shell variables */
ವರ್ಣ *get_local_var(ಧ್ರುವ ವರ್ಣ *s)
{
	ಸಂರಚನಾ variables *cur;

	ಯದಿ (!s)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

#ifdef __U_BOOT__
	ಯದಿ (*s == '$')
		ಪ್ರತ್ಯಾಗಮ get_dollar_var(s[1]);
#endif

	ಪ್ರತಿ (cur = ತುದಿ_vars; cur; cur=cur->next)
		ಯದಿ(strcmp(cur->name, s)==0)
			ಪ್ರತ್ಯಾಗಮ cur->ಮೌಲ್ಯ;
	ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
}

/* This is used to set local shell variables
   flg_export==0 if only local (not exporting) variable
   flg_export==1 if "new" exporting environ
   flg_export>1  if current startup environ (not call putenv()) */
ಪೂರ್ಣಾಙ್ಕ set_local_var(ಧ್ರುವ ವರ್ಣ *s, ಪೂರ್ಣಾಙ್ಕ flg_export)
{
	ವರ್ಣ *name, *ಮೌಲ್ಯ;
	ಪೂರ್ಣಾಙ್ಕ result=0;
	ಸಂರಚನಾ variables *cur;

#ifdef __U_BOOT__
	/* might be possible! */
	ಯದಿ (!isalpha(*s))
		ಪ್ರತ್ಯಾಗಮ -1;
#endif

	name=strdup(s);

	/* Assume when we enter this function that we are already in
	 * NAME=VALUE format.  So the first order of business is to
	 * split 's' on the '=' into 'name' and 'value' */
	ಮೌಲ್ಯ = strchr(name, '=');
	ಯದಿ (!ಮೌಲ್ಯ) {
		ಮುಕ್ತ(name);
		ಪ್ರತ್ಯಾಗಮ -1;
	}
	*ಮೌಲ್ಯ++ = 0;

	ಯದಿ (!*ಮೌಲ್ಯ) {
		unset_local_var(name);
		ಮುಕ್ತ(name);
		ಪ್ರತ್ಯಾಗಮ 0;
	}

	ಪ್ರತಿ(cur = ತುದಿ_vars; cur; cur = cur->next) {
		ಯದಿ(strcmp(cur->name, name)==0)
			ಭಙ್ಗ;
	}

	ಯದಿ(cur) {
		ಯದಿ(strcmp(cur->ಮೌಲ್ಯ, ಮೌಲ್ಯ)==0) {
			ಯದಿ(flg_export>0 && cur->flg_export==0)
				cur->flg_export=flg_export;
			ಅನ್ಯಥಾ
				result++;
		} ಅನ್ಯಥಾ {
			ಯದಿ(cur->flg_read_only) {
				error_ಸಂದೇಶ("%s: readonly variable", name);
				result = -1;
			} ಅನ್ಯಥಾ {
				ಯದಿ(flg_export>0 || cur->flg_export>1)
					cur->flg_export=1;
				ಮುಕ್ತ(cur->ಮೌಲ್ಯ);

				cur->ಮೌಲ್ಯ = strdup(ಮೌಲ್ಯ);
			}
		}
	} ಅನ್ಯಥಾ {
		cur = malloc(ಪರಿಮಾಣ(ಸಂರಚನಾ variables));
		ಯದಿ(!cur) {
			result = -1;
		} ಅನ್ಯಥಾ {
			cur->name = strdup(name);
			ಯದಿ (cur->name == ರಿಕ್ತ) {
				ಮುಕ್ತ(cur);
				result = -1;
			} ಅನ್ಯಥಾ {
				ಸಂರಚನಾ variables *bottom = ತುದಿ_vars;
				cur->ಮೌಲ್ಯ = strdup(ಮೌಲ್ಯ);
				cur->next = ರಿಕ್ತ;
				cur->flg_export = flg_export;
				cur->flg_read_only = 0;
				ಯಾವತ್(bottom->next) bottom=bottom->next;
				bottom->next = cur;
			}
		}
	}

#ifndef __U_BOOT__
	ಯದಿ(result==0 && cur->flg_export==1) {
		*(ಮೌಲ್ಯ-1) = '=';
		result = putenv(name);
	} ಅನ್ಯಥಾ {
#endif
		ಮುಕ್ತ(name);
#ifndef __U_BOOT__
		ಯದಿ(result>0)            /* equivalent to previous set */
			result = 0;
	}
#endif
	ಪ್ರತ್ಯಾಗಮ result;
}

ಶೂನ್ಯ unset_local_var(ಧ್ರುವ ವರ್ಣ *name)
{
	ಸಂರಚನಾ variables *cur;

	ಯದಿ (name) {
		ಪ್ರತಿ (cur = ತುದಿ_vars; cur; cur=cur->next) {
			ಯದಿ(strcmp(cur->name, name)==0)
				ಭಙ್ಗ;
		}
		ಯದಿ (cur != ರಿಕ್ತ) {
			ಸಂರಚನಾ variables *next = ತುದಿ_vars;
			ಯದಿ(cur->flg_read_only) {
				error_ಸಂದೇಶ("%s: readonly variable", name);
				ಪ್ರತ್ಯಾಗಮ;
			} ಅನ್ಯಥಾ {
#ifndef __U_BOOT__
				ಯದಿ(cur->flg_export)
					unenv_set(cur->name);
#endif
				ಮುಕ್ತ(cur->name);
				ಮುಕ್ತ(cur->ಮೌಲ್ಯ);
				ಯಾವತ್ (next->next != cur)
					next = next->next;
				next->next = cur->next;
			}
			ಮುಕ್ತ(cur);
		}
	}
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ is_assignment(ಧ್ರುವ ವರ್ಣ *s)
{
	ಯದಿ (s == ರಿಕ್ತ)
		ಪ್ರತ್ಯಾಗಮ 0;

	ಯದಿ (!isalpha(*s)) ಪ್ರತ್ಯಾಗಮ 0;
	++s;
	ಯಾವತ್(isalnum(*s) || *s=='_') ++s;
	ಪ್ರತ್ಯಾಗಮ *s=='=';
}

#ifndef __U_BOOT__
/* the src parameter allows us to peek forward to a possible &n syntax
 * for file descriptor duplication, e.g., "2>&1".
 * Return code is 0 normally, 1 if a syntax error is detected in src.
 * Resource errors (in xmalloc) cause the process to exit */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ setup_redirect(ಸಂರಚನಾ p_context *ctx, ಪೂರ್ಣಾಙ್ಕ fd, redir_type style,
	ಸಂರಚನಾ in_str *ಒಳಹರಿವು)
{
	ಸಂರಚನಾ child_prog *child=ctx->child;
	ಸಂರಚನಾ redir_struct *redir = child->redirects;
	ಸಂರಚನಾ redir_struct *last_redir=ರಿಕ್ತ;

	/* Create a new redir_struct and drop it onto the end of the linked list */
	ಯಾವತ್(redir) {
		last_redir=redir;
		redir=redir->next;
	}
	redir = xmalloc(ಪರಿಮಾಣ(ಸಂರಚನಾ redir_struct));
	redir->next=ರಿಕ್ತ;
	redir->word.gl_pathv=ರಿಕ್ತ;
	ಯದಿ (last_redir) {
		last_redir->next=redir;
	} ಅನ್ಯಥಾ {
		child->redirects=redir;
	}

	redir->type=style;
	redir->fd= (fd==-1) ? redir_table[style].default_fd : fd ;

	debug_printf("Redirect type %d%s\n", redir->fd, redir_table[style].descrip);

	/* Check for a '2>&1' type redirect */
	redir->dup = redirect_dup_ಅಂಕೆ(ಒಳಹರಿವು);
	ಯದಿ (redir->dup == -2) ಪ್ರತ್ಯಾಗಮ 1;  /* syntax error */
	ಯದಿ (redir->dup != -1) {
		/* Erik had a check here that the file descriptor in question
		 * is legit; I postpone that to "run time"
		 * A "-" representation of "close me" shows up as a -3 here */
		debug_printf("Duplicating redirect '%d>&%d'\n", redir->fd, redir->dup);
	} ಅನ್ಯಥಾ {
		/* We do _not_ try to open the file that src points to,
		 * since we need to return and let src be expanded first.
		 * Set ctx->pending_redirect, so we know what to do at the
		 * end of the next parsed word.
		 */
		ctx->pending_redirect = redir;
	}
	ಪ್ರತ್ಯಾಗಮ 0;
}
#endif

ಸ್ಥಿರ ಸಂರಚನಾ pipe *new_pipe(ಶೂನ್ಯ)
{
	ಸಂರಚನಾ pipe *pi;
	pi = xmalloc(ಪರಿಮಾಣ(ಸಂರಚನಾ pipe));
	pi->ಅಂಕೆ_progs = 0;
	pi->progs = ರಿಕ್ತ;
	pi->next = ರಿಕ್ತ;
	pi->followup = 0;  /* invalid */
	pi->r_mode = RES_NONE;
	ಪ್ರತ್ಯಾಗಮ pi;
}

ಸ್ಥಿರ ಶೂನ್ಯ initialize_context(ಸಂರಚನಾ p_context *ctx)
{
	ctx->pipe=ರಿಕ್ತ;
#ifndef __U_BOOT__
	ctx->pending_redirect=ರಿಕ್ತ;
#endif
	ctx->child=ರಿಕ್ತ;
	ctx->list_head=new_pipe();
	ctx->pipe=ctx->list_head;
	ctx->w=RES_NONE;
	ctx->ರಾಶಿ=ರಿಕ್ತ;
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
ಸಂರಚನಾ reserved_combo {
	ವರ್ಣ *literal;
	ಪೂರ್ಣಾಙ್ಕ code;
	long flag;
};
/* Mostly a list of accepted follow-up reserved words.
 * FLAG_END means we are done with the sequence, and are ready
 * to turn the compound list into a command.
 * FLAG_START means the word must start a new compound list.
 */
ಸ್ಥಿರ ಸಂರಚನಾ reserved_combo reserved_list[] = {
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
#define NRES (ಪರಿಮಾಣ(reserved_list)/ಪರಿಮಾಣ(ಸಂರಚನಾ reserved_combo))

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ reserved_word(o_string *dest, ಸಂರಚನಾ p_context *ctx)
{
	ಸಂರಚನಾ reserved_combo *r;
	ಪ್ರತಿ (r=reserved_list;
		r<reserved_list+NRES; r++) {
		ಯದಿ (strcmp(dest->data, r->literal) == 0) {
			debug_printf("found reserved word %s, code %d\n",r->literal,r->code);
			ಯದಿ (r->flag & FLAG_START) {
				ಸಂರಚನಾ p_context *new = xmalloc(ಪರಿಮಾಣ(ಸಂರಚನಾ p_context));
				debug_printf("push stack\n");
				ಯದಿ (ctx->w == RES_IN || ctx->w == RES_FOR) {
					syntax();
					ಮುಕ್ತ(new);
					ctx->w = RES_SNTX;
					b_reset(dest);
					ಪ್ರತ್ಯಾಗಮ 1;
				}
				*new = *ctx;   /* physical copy */
				initialize_context(ctx);
				ctx->ರಾಶಿ=new;
			} ಅನ್ಯಥಾ ಯದಿ ( ctx->w == RES_NONE || ! (ctx->old_flag & (1<<r->code))) {
				syntax();
				ctx->w = RES_SNTX;
				b_reset(dest);
				ಪ್ರತ್ಯಾಗಮ 1;
			}
			ctx->w=r->code;
			ctx->old_flag = r->flag;
			ಯದಿ (ctx->old_flag & FLAG_END) {
				ಸಂರಚನಾ p_context *old;
				debug_printf("pop stack\n");
				done_pipe(ctx,PIPE_SEQ);
				old = ctx->ರಾಶಿ;
				old->child->group = ctx->list_head;
#ifndef __U_BOOT__
				old->child->subshell = 0;
#endif
				*ctx = *old;   /* physical copy */
				ಮುಕ್ತ(old);
			}
			b_reset (dest);
			ಪ್ರತ್ಯಾಗಮ 1;
		}
	}
	ಪ್ರತ್ಯಾಗಮ 0;
}

/* normal return is 0.
 * Syntax or xglob errors return 1. */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ done_word(o_string *dest, ಸಂರಚನಾ p_context *ctx)
{
	ಸಂರಚನಾ child_prog *child=ctx->child;
#ifndef __U_BOOT__
	glob_t *glob_target;
	ಪೂರ್ಣಾಙ್ಕ gr, flags = 0;
#ಅನ್ಯಥಾ
	ವರ್ಣ *str, *s;
	ಪೂರ್ಣಾಙ್ಕ argc, cnt;
#endif

	debug_printf("done_word: %s %p\n", dest->data, child);
	ಯದಿ (dest->length == 0 && !dest->nonnull) {
		debug_printf("  true null, ignored\n");
		ಪ್ರತ್ಯಾಗಮ 0;
	}
#ifndef __U_BOOT__
	ಯದಿ (ctx->pending_redirect) {
		glob_target = &ctx->pending_redirect->word;
	} ಅನ್ಯಥಾ {
#endif
		ಯದಿ (child->group) {
			syntax();
			ಪ್ರತ್ಯಾಗಮ 1;  /* syntax error, groups and arglists don't mix */
		}
		ಯದಿ (!child->argv && (ctx->type & FLAG_PARSE_SEMICOLON)) {
			debug_printf("checking %s for reserved-ness\n",dest->data);
			ಯದಿ (reserved_word(dest,ctx)) ಪ್ರತ್ಯಾಗಮ ctx->w==RES_SNTX;
		}
#ifndef __U_BOOT__
		glob_target = &child->glob_result;
		ಯದಿ (child->argv) flags |= GLOB_APPEND;
#ಅನ್ಯಥಾ
		ಪ್ರತಿ (cnt = 1, s = dest->data; s && *s; s++) {
			ಯದಿ (*s == '\\') s++;
			cnt++;
		}
		str = malloc(cnt);
		ಯದಿ (!str) ಪ್ರತ್ಯಾಗಮ 1;
		ಯದಿ ( child->argv == ರಿಕ್ತ) {
			child->argc=0;
		}
		argc = ++child->argc;
		child->argv = realloc(child->argv, (argc+1)*ಪರಿಮಾಣ(*child->argv));
		ಯದಿ (child->argv == ರಿಕ್ತ) {
			ಮುಕ್ತ(str);
			ಪ್ರತ್ಯಾಗಮ 1;
		}
		child->argv_nonnull = realloc(child->argv_nonnull,
					(argc+1)*ಪರಿಮಾಣ(*child->argv_nonnull));
		ಯದಿ (child->argv_nonnull == ರಿಕ್ತ) {
			ಮುಕ್ತ(str);
			ಪ್ರತ್ಯಾಗಮ 1;
		}
		child->argv[argc-1]=str;
		child->argv_nonnull[argc-1] = dest->nonnull;
		child->argv[argc]=ರಿಕ್ತ;
		child->argv_nonnull[argc] = 0;
		ಪ್ರತಿ (s = dest->data; s && *s; s++,str++) {
			ಯದಿ (*s == '\\') s++;
			*str = *s;
		}
		*str = '\0';
#endif
#ifndef __U_BOOT__
	}
	gr = xglob(dest, flags, glob_target);
	ಯದಿ (gr != 0) ಪ್ರತ್ಯಾಗಮ 1;
#endif

	b_reset(dest);
#ifndef __U_BOOT__
	ಯದಿ (ctx->pending_redirect) {
		ctx->pending_redirect=ರಿಕ್ತ;
		ಯದಿ (glob_target->gl_pathc != 1) {
			error_ಸಂದೇಶ("ambiguous redirect");
			ಪ್ರತ್ಯಾಗಮ 1;
		}
	} ಅನ್ಯಥಾ {
		child->argv = glob_target->gl_pathv;
	}
#endif
	ಯದಿ (ctx->w == RES_FOR) {
		done_word(dest,ctx);
		done_pipe(ctx,PIPE_SEQ);
	}
	ಪ್ರತ್ಯಾಗಮ 0;
}

/* The only possible error here is out of memory, in which case
 * xmalloc exits. */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ done_command(ಸಂರಚನಾ p_context *ctx)
{
	/* The child is really already in the pipe structure, so
	 * advance the pipe counter and make a new, null child.
	 * Only real trickiness here is that the uncommitted
	 * child structure, to which ctx->child points, is not
	 * counted in pi->num_progs. */
	ಸಂರಚನಾ pipe *pi=ctx->pipe;
	ಸಂರಚನಾ child_prog *prog=ctx->child;

	ಯದಿ (prog && prog->group == ರಿಕ್ತ
		 && prog->argv == ರಿಕ್ತ
#ifndef __U_BOOT__
		 && prog->redirects == ರಿಕ್ತ) {
#ಅನ್ಯಥಾ
										) {
#endif
		debug_printf("done_command: skipping null command\n");
		ಪ್ರತ್ಯಾಗಮ 0;
	} ಅನ್ಯಥಾ ಯದಿ (prog) {
		pi->ಅಂಕೆ_progs++;
		debug_printf("done_command: num_progs incremented to %d\n",pi->ಅಂಕೆ_progs);
	} ಅನ್ಯಥಾ {
		debug_printf("done_command: initializing\n");
	}
	pi->progs = xrealloc(pi->progs, ಪರಿಮಾಣ(*pi->progs) * (pi->ಅಂಕೆ_progs+1));

	prog = pi->progs + pi->ಅಂಕೆ_progs;
#ifndef __U_BOOT__
	prog->redirects = ರಿಕ್ತ;
#endif
	prog->argv = ರಿಕ್ತ;
	prog->argv_nonnull = ರಿಕ್ತ;
#ifndef __U_BOOT__
	prog->is_stopped = 0;
#endif
	prog->group = ರಿಕ್ತ;
#ifndef __U_BOOT__
	prog->glob_result.gl_pathv = ರಿಕ್ತ;
	prog->family = pi;
#endif
	prog->sp = 0;
	ctx->child = prog;
	prog->type = ctx->type;

	/* but ctx->pipe and ctx->list_head remain unchanged */
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ done_pipe(ಸಂರಚನಾ p_context *ctx, pipe_style type)
{
	ಸಂರಚನಾ pipe *new_p;
	done_command(ctx);  /* implicit closure of previous command */
	debug_printf("done_pipe, type %d\n", type);
	ctx->pipe->followup = type;
	ctx->pipe->r_mode = ctx->w;
	new_p=new_pipe();
	ctx->pipe->next = new_p;
	ctx->pipe = new_p;
	ctx->child = ರಿಕ್ತ;
	done_command(ctx);  /* set up new pipe to accept commands */
	ಪ್ರತ್ಯಾಗಮ 0;
}

#ifndef __U_BOOT__
/* peek ahead in the in_str to find out if we have a "&n" construct,
 * as in "2>&1", that represents duplicating a file descriptor.
 * returns either -2 (syntax error), -1 (no &), or the number found.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ redirect_dup_ಅಂಕೆ(ಸಂರಚನಾ in_str *ಒಳಹರಿವು)
{
	ಪೂರ್ಣಾಙ್ಕ ch, d=0, ok=0;
	ch = b_peek(ಒಳಹರಿವು);
	ಯದಿ (ch != '&') ಪ್ರತ್ಯಾಗಮ -1;

	b_getch(ಒಳಹರಿವು);  /* get the & */
	ch=b_peek(ಒಳಹರಿವು);
	ಯದಿ (ch == '-') {
		b_getch(ಒಳಹರಿವು);
		ಪ್ರತ್ಯಾಗಮ -3;  /* "-" represents "close me" */
	}
	ಯಾವತ್ (isdigit(ch)) {
		d = d*10+(ch-'0');
		ok=1;
		b_getch(ಒಳಹರಿವು);
		ch = b_peek(ಒಳಹರಿವು);
	}
	ಯದಿ (ok) ಪ್ರತ್ಯಾಗಮ d;

	error_ಸಂದೇಶ("ambiguous redirect");
	ಪ್ರತ್ಯಾಗಮ -2;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ redirect_opt_ಅಂಕೆ(o_string *o)
{
	ಪೂರ್ಣಾಙ್ಕ ಅಂಕೆ;

	ಯದಿ (o->length==0) ಪ್ರತ್ಯಾಗಮ -1;
	ಪ್ರತಿ(ಅಂಕೆ=0; ಅಂಕೆ<o->length; ಅಂಕೆ++) {
		ಯದಿ (!isdigit(*(o->data+ಅಂಕೆ))) {
			ಪ್ರತ್ಯಾಗಮ -1;
		}
	}
	/* reuse num (and save an int) */
	ಅಂಕೆ=atoi(o->data);
	b_reset(o);
	ಪ್ರತ್ಯಾಗಮ ಅಂಕೆ;
}

FILE *generate_stream_from_list(ಸಂರಚನಾ pipe *head)
{
	FILE *pf;
#ಯದಿ 1
	ಪೂರ್ಣಾಙ್ಕ pid, channel[2];
	ಯದಿ (pipe(channel)<0) perror_ಸಂದೇಶ_and_die("pipe");
	pid=fork();
	ಯದಿ (pid<0) {
		perror_ಸಂದೇಶ_and_die("fork");
	} ಅನ್ಯಥಾ ಯದಿ (pid==0) {
		close(channel[0]);
		ಯದಿ (channel[1] != 1) {
			dup2(channel[1],1);
			close(channel[1]);
		}
#ಯದಿ 0
#define SURROGATE "surrogate response"
		ಲಿಖ(1,SURROGATE,ಪರಿಮಾಣ(SURROGATE));
		_exit(run_list(head));
#ಅನ್ಯಥಾ
		_exit(run_list_real(head));   /* leaks memory */
#endif
	}
	debug_printf("forked child %d\n",pid);
	close(channel[1]);
	pf = fdopen(channel[0],"r");
	debug_printf("pipe on FILE *%p\n",pf);
#ಅನ್ಯಥಾ
	free_pipe_list(head,0);
	pf=popen("echo surrogate response","r");
	debug_printf("started fake pipe on FILE *%p\n",pf);
#endif
	ಪ್ರತ್ಯಾಗಮ pf;
}

/* this version hacked for testing purposes */
/* return code is exit status of the process that is run. */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ process_command_subs(o_string *dest, ಸಂರಚನಾ p_context *ctx, ಸಂರಚನಾ in_str *ಒಳಹರಿವು, ಪೂರ್ಣಾಙ್ಕ subst_end)
{
	ಪೂರ್ಣಾಙ್ಕ retcode;
	o_string result=NULL_O_STRING;
	ಸಂರಚನಾ p_context inner;
	FILE *p;
	ಸಂರಚನಾ in_str pipe_str;
	initialize_context(&inner);

	/* recursion to generate command */
	retcode = parse_stream(&result, &inner, ಒಳಹರಿವು, subst_end);
	ಯದಿ (retcode != 0) ಪ್ರತ್ಯಾಗಮ retcode;  /* syntax error or EOF */
	done_word(&result, &inner);
	done_pipe(&inner, PIPE_SEQ);
	b_free(&result);

	p=generate_stream_from_list(inner.list_head);
	ಯದಿ (p==ರಿಕ್ತ) ಪ್ರತ್ಯಾಗಮ 1;
	mark_open(fileno(p));
	setup_file_in_str(&pipe_str, p);

	/* now send results of command back into original context */
	retcode = parse_stream(dest, ctx, &pipe_str, '\0');
	/* XXX In case of a syntax error, should we try to kill the child?
	 * That would be tough to do right, so just read until EOF. */
	ಯದಿ (retcode == 1) {
		ಯಾವತ್ (b_getch(&pipe_str)!=EOF) { /* discard */ };
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
	ಪ್ರತ್ಯಾಗಮ retcode;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ parse_group(o_string *dest, ಸಂರಚನಾ p_context *ctx,
	ಸಂರಚನಾ in_str *ಒಳಹರಿವು, ಪೂರ್ಣಾಙ್ಕ ch)
{
	ಪೂರ್ಣಾಙ್ಕ rcode, endch=0;
	ಸಂರಚನಾ p_context sub;
	ಸಂರಚನಾ child_prog *child = ctx->child;
	ಯದಿ (child->argv) {
		syntax();
		ಪ್ರತ್ಯಾಗಮ 1;  /* syntax error, groups and arglists don't mix */
	}
	initialize_context(&sub);
	ವಿಕಲ್ಪ(ch) {
		ಪಕ್ಷ '(': endch=')'; child->subshell=1; ಭಙ್ಗ;
		ಪಕ್ಷ '{': endch='}'; ಭಙ್ಗ;
		ಸ್ವಭಾವ: syntax();   /* really logic error */
	}
	rcode=parse_stream(dest,&sub,ಒಳಹರಿವು,endch);
	done_word(dest,&sub); /* finish off the final word in the subcontext */
	done_pipe(&sub, PIPE_SEQ);  /* and the final command there, too */
	child->group = sub.list_head;
	ಪ್ರತ್ಯಾಗಮ rcode;
	/* child remains "open", available for possible redirects */
}
#endif

/* basically useful version until someone wants to get fancier,
 * see the bash man page under "Parameter Expansion" */
ಸ್ಥಿರ ವರ್ಣ *lookup_param(ವರ್ಣ *src)
{
	ವರ್ಣ *p;
	ವರ್ಣ *sep;
	ವರ್ಣ *default_val = ರಿಕ್ತ;
	ಪೂರ್ಣಾಙ್ಕ assign = 0;
	ಪೂರ್ಣಾಙ್ಕ expand_empty = 0;

	ಯದಿ (!src)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	sep = strchr(src, ':');

	ಯದಿ (sep) {
		*sep = '\0';
		ಯದಿ (*(sep + 1) == '-')
			default_val = sep+2;
		ಯದಿ (*(sep + 1) == '=') {
			default_val = sep+2;
			assign = 1;
		}
		ಯದಿ (*(sep + 1) == '+') {
			default_val = sep+2;
			expand_empty = 1;
		}
	}

	p = env_get(src);
	ಯದಿ (!p)
		p = get_local_var(src);

	ಯದಿ (!p || strlen(p) == 0) {
		p = default_val;
		ಯದಿ (assign) {
			ವರ್ಣ *var = malloc(strlen(src)+strlen(default_val)+2);
			ಯದಿ (var) {
				sprintf(var, "%s=%s", src, default_val);
				set_local_var(var, 0);
			}
			ಮುಕ್ತ(var);
		}
	} ಅನ್ಯಥಾ ಯದಿ (expand_empty) {
		p += strlen(p);
	}

	ಯದಿ (sep)
		*sep = ':';

	ಪ್ರತ್ಯಾಗಮ p;
}

#ifdef __U_BOOT__
ಸ್ಥಿರ ವರ್ಣ *get_dollar_var(ವರ್ಣ ch)
{
	ಸ್ಥಿರ ವರ್ಣ buf[40];

	buf[0] = '\0';
	ವಿಕಲ್ಪ (ch) {
		ಪಕ್ಷ '?':
			sprintf(buf, "%u", (unsigned ಪೂರ್ಣಾಙ್ಕ)last_return_code);
			ಭಙ್ಗ;
		ಸ್ವಭಾವ:
			ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
	}
	ಪ್ರತ್ಯಾಗಮ buf;
}
#endif

/* return code: 0 for OK, 1 for syntax error */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ handle_dollar(o_string *dest, ಸಂರಚನಾ p_context *ctx, ಸಂರಚನಾ in_str *ಒಳಹರಿವು)
{
#ifndef __U_BOOT__
	ಪೂರ್ಣಾಙ್ಕ i, advance=0;
#ಅನ್ಯಥಾ
	ಪೂರ್ಣಾಙ್ಕ advance=0;
#endif
#ifndef __U_BOOT__
	ವರ್ಣ sep[]=" ";
#endif
	ಪೂರ್ಣಾಙ್ಕ ch = ಒಳಹರಿವು->peek(ಒಳಹರಿವು);  /* first character after the $ */
	debug_printf("handle_dollar: ch=%c\n",ch);
	ಯದಿ (isalpha(ch)) {
		b_addchr(dest, SPECIAL_VAR_ಚಿಹ್ನೆ);
		ctx->child->sp++;
		ಯಾವತ್(ch=b_peek(ಒಳಹರಿವು),isalnum(ch) || ch=='_') {
			b_getch(ಒಳಹರಿವು);
			b_addchr(dest,ch);
		}
		b_addchr(dest, SPECIAL_VAR_ಚಿಹ್ನೆ);
#ifndef __U_BOOT__
	} ಅನ್ಯಥಾ ಯದಿ (isdigit(ch)) {
		i = ch-'0';  /* XXX is $0 special? */
		ಯದಿ (i<global_argc) {
			parse_string(dest, ctx, global_argv[i]); /* recursion */
		}
		advance = 1;
#endif
	} ಅನ್ಯಥಾ ವಿಕಲ್ಪ (ch) {
#ifndef __U_BOOT__
		ಪಕ್ಷ '$':
			b_adduint(dest,getpid());
			advance = 1;
			ಭಙ್ಗ;
		ಪಕ್ಷ '!':
			ಯದಿ (last_bg_pid > 0) b_adduint(dest, last_bg_pid);
			advance = 1;
			ಭಙ್ಗ;
#endif
		ಪಕ್ಷ '?':
#ifndef __U_BOOT__
			b_adduint(dest,last_return_code);
#ಅನ್ಯಥಾ
			ctx->child->sp++;
			b_addchr(dest, SPECIAL_VAR_ಚಿಹ್ನೆ);
			b_addchr(dest, '$');
			b_addchr(dest, '?');
			b_addchr(dest, SPECIAL_VAR_ಚಿಹ್ನೆ);
#endif
			advance = 1;
			ಭಙ್ಗ;
#ifndef __U_BOOT__
		ಪಕ್ಷ '#':
			b_adduint(dest,global_argc ? global_argc-1 : 0);
			advance = 1;
			ಭಙ್ಗ;
#endif
		ಪಕ್ಷ '{':
			b_addchr(dest, SPECIAL_VAR_ಚಿಹ್ನೆ);
			ctx->child->sp++;
			b_getch(ಒಳಹರಿವು);
			/* XXX maybe someone will try to escape the '}' */
			ಯಾವತ್(ch=b_getch(ಒಳಹರಿವು),ch!=EOF && ch!='}') {
				b_addchr(dest,ch);
			}
			ಯದಿ (ch != '}') {
				syntax();
				ಪ್ರತ್ಯಾಗಮ 1;
			}
			b_addchr(dest, SPECIAL_VAR_ಚಿಹ್ನೆ);
			ಭಙ್ಗ;
#ifndef __U_BOOT__
		ಪಕ್ಷ '(':
			b_getch(ಒಳಹರಿವು);
			process_command_subs(dest, ctx, ಒಳಹರಿವು, ')');
			ಭಙ್ಗ;
		ಪಕ್ಷ '*':
			sep[0]=ifs[0];
			ಪ್ರತಿ (i=1; i<global_argc; i++) {
				parse_string(dest, ctx, global_argv[i]);
				ಯದಿ (i+1 < global_argc) parse_string(dest, ctx, sep);
			}
			ಭಙ್ಗ;
		ಪಕ್ಷ '@':
		ಪಕ್ಷ '-':
		ಪಕ್ಷ '_':
			/* still unhandled, but should be eventually */
			error_ಸಂದೇಶ("unhandled syntax: $%c",ch);
			ಪ್ರತ್ಯಾಗಮ 1;
			ಭಙ್ಗ;
#endif
		ಸ್ವಭಾವ:
			b_addqchr(dest,'$',dest->quote);
	}
	/* Eat the character if the flag was set.  If the compiler
	 * is smart enough, we could substitute "b_getch(input);"
	 * for all the "advance = 1;" above, and also end up with
	 * a nice size-optimized program.  Hah!  That'll be the day.
	 */
	ಯದಿ (advance) b_getch(ಒಳಹರಿವು);
	ಪ್ರತ್ಯಾಗಮ 0;
}

#ifndef __U_BOOT__
ಪೂರ್ಣಾಙ್ಕ parse_string(o_string *dest, ಸಂರಚನಾ p_context *ctx, ಧ್ರುವ ವರ್ಣ *src)
{
	ಸಂರಚನಾ in_str foo;
	setup_string_in_str(&foo, src);
	ಪ್ರತ್ಯಾಗಮ parse_stream(dest, ctx, &foo, '\0');
}
#endif

/* return code is 0 for normal exit, 1 for syntax error */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ parse_stream(o_string *dest, ಸಂರಚನಾ p_context *ctx,
			ಸಂರಚನಾ in_str *ಒಳಹರಿವು, ಪೂರ್ಣಾಙ್ಕ end_trigger)
{
	unsigned ಪೂರ್ಣಾಙ್ಕ ch, m;
#ifndef __U_BOOT__
	ಪೂರ್ಣಾಙ್ಕ redir_fd;
	redir_type redir_style;
#endif
	ಪೂರ್ಣಾಙ್ಕ next;

	/* Only double-quote state is handled in the state variable dest->quote.
	 * A single-quote triggers a bypass of the main loop until its mate is
	 * found.  When recursing, quote state is passed in via dest->quote. */

	debug_printf("parse_stream, end_trigger=%d\n",end_trigger);
	ಯಾವತ್ ((ch=b_getch(ಒಳಹರಿವು))!=EOF) {
		m = map[ch];
#ifdef __U_BOOT__
		ಯದಿ (ಒಳಹರಿವು->__promptme == 0) ಪ್ರತ್ಯಾಗಮ 1;
#endif
		next = (ch == '\n') ? 0 : b_peek(ಒಳಹರಿವು);

		debug_printf("parse_stream: ch=%c (%d) m=%d quote=%d - %c\n",
			ch >= ' ' ? ch : '.', ch, m,
			dest->quote, ctx->ರಾಶಿ == ರಿಕ್ತ ? '*' : '.');

		ಯದಿ (m==0 || ((m==1 || m==2) && dest->quote)) {
			b_addqchr(dest, ch, dest->quote);
		} ಅನ್ಯಥಾ {
			ಯದಿ (m==2) {  /* unquoted IFS */
				ಯದಿ (done_word(dest, ctx)) {
					ಪ್ರತ್ಯಾಗಮ 1;
				}
				/* If we aren't performing a substitution, treat a newline as a
				 * command separator.  */
				ಯದಿ (end_trigger != '\0' && ch=='\n')
					done_pipe(ctx,PIPE_SEQ);
			}
			ಯದಿ (ch == end_trigger && !dest->quote && ctx->w==RES_NONE) {
				debug_printf("leaving parse_stream (triggered)\n");
				ಪ್ರತ್ಯಾಗಮ 0;
			}
#ಯದಿ 0
			ಯದಿ (ch=='\n') {
				/* Yahoo!  Time to run with it! */
				done_pipe(ctx,PIPE_SEQ);
				run_list(ctx->list_head);
				initialize_context(ctx);
			}
#endif
			ಯದಿ (m!=2) ವಿಕಲ್ಪ (ch) {
		ಪಕ್ಷ '#':
			ಯದಿ (dest->length == 0 && !dest->quote) {
				ಯಾವತ್(ch=b_peek(ಒಳಹರಿವು),ch!=EOF && ch!='\n') { b_getch(ಒಳಹರಿವು); }
			} ಅನ್ಯಥಾ {
				b_addqchr(dest, ch, dest->quote);
			}
			ಭಙ್ಗ;
		ಪಕ್ಷ '\\':
			ಯದಿ (next == EOF) {
				syntax();
				ಪ್ರತ್ಯಾಗಮ 1;
			}
			b_addqchr(dest, '\\', dest->quote);
			b_addqchr(dest, b_getch(ಒಳಹರಿವು), dest->quote);
			ಭಙ್ಗ;
		ಪಕ್ಷ '$':
			ಯದಿ (handle_dollar(dest, ctx, ಒಳಹರಿವು)!=0) ಪ್ರತ್ಯಾಗಮ 1;
			ಭಙ್ಗ;
		ಪಕ್ಷ '\'':
			dest->nonnull = 1;
			ಯಾವತ್(ch=b_getch(ಒಳಹರಿವು),ch!=EOF && ch!='\'') {
#ifdef __U_BOOT__
				ಯದಿ(ಒಳಹರಿವು->__promptme == 0) ಪ್ರತ್ಯಾಗಮ 1;
#endif
				b_addchr(dest,ch);
			}
			ಯದಿ (ch==EOF) {
				syntax();
				ಪ್ರತ್ಯಾಗಮ 1;
			}
			ಭಙ್ಗ;
		ಪಕ್ಷ '"':
			dest->nonnull = 1;
			dest->quote = !dest->quote;
			ಭಙ್ಗ;
#ifndef __U_BOOT__
		ಪಕ್ಷ '`':
			process_command_subs(dest, ctx, ಒಳಹರಿವು, '`');
			ಭಙ್ಗ;
		ಪಕ್ಷ '>':
			redir_fd = redirect_opt_ಅಂಕೆ(dest);
			done_word(dest, ctx);
			redir_style=REDIRECT_OVERWRITE;
			ಯದಿ (next == '>') {
				redir_style=REDIRECT_APPEND;
				b_getch(ಒಳಹರಿವು);
			} ಅನ್ಯಥಾ ಯದಿ (next == '(') {
				syntax();   /* until we support >(list) Process Substitution */
				ಪ್ರತ್ಯಾಗಮ 1;
			}
			setup_redirect(ctx, redir_fd, redir_style, ಒಳಹರಿವು);
			ಭಙ್ಗ;
		ಪಕ್ಷ '<':
			redir_fd = redirect_opt_ಅಂಕೆ(dest);
			done_word(dest, ctx);
			redir_style=REDIRECT_ಒಳಹರಿವು;
			ಯದಿ (next == '<') {
				redir_style=REDIRECT_HEREIS;
				b_getch(ಒಳಹರಿವು);
			} ಅನ್ಯಥಾ ಯದಿ (next == '>') {
				redir_style=REDIRECT_IO;
				b_getch(ಒಳಹರಿವು);
			} ಅನ್ಯಥಾ ಯದಿ (next == '(') {
				syntax();   /* until we support <(list) Process Substitution */
				ಪ್ರತ್ಯಾಗಮ 1;
			}
			setup_redirect(ctx, redir_fd, redir_style, ಒಳಹರಿವು);
			ಭಙ್ಗ;
#endif
		ಪಕ್ಷ ';':
			done_word(dest, ctx);
			done_pipe(ctx,PIPE_SEQ);
			ಭಙ್ಗ;
		ಪಕ್ಷ '&':
			done_word(dest, ctx);
			ಯದಿ (next=='&') {
				b_getch(ಒಳಹರಿವು);
				done_pipe(ctx,PIPE_AND);
			} ಅನ್ಯಥಾ {
#ifndef __U_BOOT__
				done_pipe(ctx,PIPE_BG);
#ಅನ್ಯಥಾ
				syntax_err();
				ಪ್ರತ್ಯಾಗಮ 1;
#endif
			}
			ಭಙ್ಗ;
		ಪಕ್ಷ '|':
			done_word(dest, ctx);
			ಯದಿ (next=='|') {
				b_getch(ಒಳಹರಿವು);
				done_pipe(ctx,PIPE_OR);
			} ಅನ್ಯಥಾ {
				/* we could pick up a file descriptor choice here
				 * with redirect_opt_num(), but bash doesn't do it.
				 * "echo foo 2| cat" yields "foo 2". */
#ifndef __U_BOOT__
				done_command(ctx);
#ಅನ್ಯಥಾ
				syntax_err();
				ಪ್ರತ್ಯಾಗಮ 1;
#endif
			}
			ಭಙ್ಗ;
#ifndef __U_BOOT__
		ಪಕ್ಷ '(':
		ಪಕ್ಷ '{':
			ಯದಿ (parse_group(dest, ctx, ಒಳಹರಿವು, ch)!=0) ಪ್ರತ್ಯಾಗಮ 1;
			ಭಙ್ಗ;
		ಪಕ್ಷ ')':
		ಪಕ್ಷ '}':
			syntax();   /* Proper use of this character caught by end_trigger */
			ಪ್ರತ್ಯಾಗಮ 1;
			ಭಙ್ಗ;
#endif
		ಪಕ್ಷ SUBSTED_VAR_ಚಿಹ್ನೆ:
			dest->nonnull = 1;
			ಯಾವತ್ (ch = b_getch(ಒಳಹರಿವು), ch != EOF &&
			    ch != SUBSTED_VAR_ಚಿಹ್ನೆ) {
				debug_printf("subst, pass=%d\n", ch);
				ಯದಿ (ಒಳಹರಿವು->__promptme == 0)
					ಪ್ರತ್ಯಾಗಮ 1;
				b_addchr(dest, ch);
			}
			debug_printf("subst, term=%d\n", ch);
			ಯದಿ (ch == EOF) {
				syntax();
				ಪ್ರತ್ಯಾಗಮ 1;
			}
			ಭಙ್ಗ;
		ಸ್ವಭಾವ:
			syntax();   /* this is really an internal logic error */
			ಪ್ರತ್ಯಾಗಮ 1;
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
	ಯದಿ (end_trigger != '\0') ಪ್ರತ್ಯಾಗಮ -1;
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಶೂನ್ಯ mapset(ಧ್ರುವ unsigned ವರ್ಣ *set, ಪೂರ್ಣಾಙ್ಕ code)
{
	ಧ್ರುವ unsigned ವರ್ಣ *s;
	ಪ್ರತಿ (s=set; *s; s++) map[*s] = code;
}

ಸ್ಥಿರ ಶೂನ್ಯ update_ifs_map(ಶೂನ್ಯ)
{
	/* char *ifs and char map[256] are both globals. */
	ifs = (uchar *)env_get("IFS");
	ಯದಿ (ifs == ರಿಕ್ತ) ifs=(uchar *)" \t\n";
	/* Precompute a list of 'flow through' behavior so it can be treated
	 * quickly up front.  Computation is necessary because of IFS.
	 * Special case handling of IFS == " \t\n" is not implemented.
	 * The map[] array only really needs two bits each, and on most machines
	 * that would be faster because of the reduced L1 cache footprint.
	 */
	memset(map,0,ಪರಿಮಾಣ(map)); /* most characters flow through always */
#ifndef __U_BOOT__
	mapset((uchar *)"\\$'\"`", 3);      /* never flow through */
	mapset((uchar *)"<>;&|(){}#", 1);   /* flow through if quoted */
#ಅನ್ಯಥಾ
	{
		uchar subst[2] = {SUBSTED_VAR_ಚಿಹ್ನೆ, 0};
		mapset(subst, 3);       /* never flow through */
	}
	mapset((uchar *)"\\$'\"", 3);       /* never flow through */
	mapset((uchar *)";&|#", 1);         /* flow through if quoted */
#endif
	mapset(ifs, 2);            /* also flow through if quoted */
}

/* most recursion does not come through here, the exeception is
 * from builtin_source() */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ parse_stream_outer(ಸಂರಚನಾ in_str *inp, ಪೂರ್ಣಾಙ್ಕ flag)
{

	ಸಂರಚನಾ p_context ctx;
	o_string temp=NULL_O_STRING;
	ಪೂರ್ಣಾಙ್ಕ rcode;
#ifdef __U_BOOT__
	ಪೂರ್ಣಾಙ್ಕ code = 1;
#endif
	ಕುರು {
		ctx.type = flag;
		initialize_context(&ctx);
		update_ifs_map();
		ಯದಿ (!(flag & FLAG_PARSE_SEMICOLON) || (flag & FLAG_REPARSING)) mapset((uchar *)";$&|", 0);
		inp->promptmode=1;
		rcode = parse_stream(&temp, &ctx, inp,
				     flag & FLAG_CONT_ON_NEWLINE ? -1 : '\n');
#ifdef __U_BOOT__
		ಯದಿ (rcode == 1) flag_repeat = 0;
#endif
		ಯದಿ (rcode != 1 && ctx.old_flag != 0) {
			syntax();
#ifdef __U_BOOT__
			flag_repeat = 0;
#endif
		}
		ಯದಿ (rcode != 1 && ctx.old_flag == 0) {
			done_word(&temp, &ctx);
			done_pipe(&ctx,PIPE_SEQ);
#ifndef __U_BOOT__
			run_list(ctx.list_head);
#ಅನ್ಯಥಾ
			code = run_list(ctx.list_head);
			ಯದಿ (code == -2) {	/* exit */
				b_free(&temp);
				code = 0;
				/* XXX hackish way to not allow exit from main loop */
				ಯದಿ (inp->peek == file_peek) {
					printf("exit not allowed from main input shell.\n");
					ಅನುವರ್ತ;
				}
				/*
				 * DANGER
				 * Return code -2 is special in this context,
				 * it indicates exit from inner pipe instead
				 * of return code itself, the return code is
				 * stored in 'last_return_code' variable!
				 * DANGER
				 */
				ಪ್ರತ್ಯಾಗಮ -2;
			}
			ಯದಿ (code == -1)
			    flag_repeat = 0;
#endif
		} ಅನ್ಯಥಾ {
			ಯದಿ (ctx.old_flag != 0) {
				ಮುಕ್ತ(ctx.ರಾಶಿ);
				b_reset(&temp);
			}
#ifdef __U_BOOT__
			ಯದಿ (inp->__promptme == 0) printf("<INTERRUPT>\n");
			inp->__promptme = 1;
#endif
			temp.nonnull = 0;
			temp.quote = 0;
			inp->p = ರಿಕ್ತ;
			free_pipe_list(ctx.list_head,0);
		}
		b_free(&temp);
	/* loop on syntax errors, return on EOF */
	} ಯಾವತ್ (rcode != -1 && !(flag & FLAG_EXIT_FROM_LOOP) &&
		(inp->peek != static_peek || b_peek(inp)));
#ifndef __U_BOOT__
	ಪ್ರತ್ಯಾಗಮ 0;
#ಅನ್ಯಥಾ
	ಪ್ರತ್ಯಾಗಮ (code != 0) ? 1 : 0;
#endif /* __U_BOOT__ */
}

#ifndef __U_BOOT__
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ parse_string_outer(ಧ್ರುವ ವರ್ಣ *s, ಪೂರ್ಣಾಙ್ಕ flag)
#ಅನ್ಯಥಾ
ಪೂರ್ಣಾಙ್ಕ parse_string_outer(ಧ್ರುವ ವರ್ಣ *s, ಪೂರ್ಣಾಙ್ಕ flag)
#endif	/* __U_BOOT__ */
{
	ಸಂರಚನಾ in_str ಒಳಹರಿವು;
	ಪೂರ್ಣಾಙ್ಕ rcode;
#ifdef __U_BOOT__
	ವರ್ಣ *p = ರಿಕ್ತ;
	ಯದಿ (!s)
		ಪ್ರತ್ಯಾಗಮ 1;
	ಯದಿ (!*s)
		ಪ್ರತ್ಯಾಗಮ 0;
	ಯದಿ (!(p = strchr(s, '\n')) || *++p) {
		p = xmalloc(strlen(s) + 2);
		strcpy(p, s);
		strcat(p, "\n");
		setup_string_in_str(&ಒಳಹರಿವು, p);
		rcode = parse_stream_outer(&ಒಳಹರಿವು, flag);
		ಮುಕ್ತ(p);
		ಪ್ರತ್ಯಾಗಮ rcode == -2 ? last_return_code : rcode;
	} ಅನ್ಯಥಾ {
#endif
	setup_string_in_str(&ಒಳಹರಿವು, s);
	rcode = parse_stream_outer(&ಒಳಹರಿವು, flag);
	ಪ್ರತ್ಯಾಗಮ rcode == -2 ? last_return_code : rcode;
#ifdef __U_BOOT__
	}
#endif
}

#ifndef __U_BOOT__
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ parse_file_outer(FILE *f)
#ಅನ್ಯಥಾ
ಪೂರ್ಣಾಙ್ಕ parse_file_outer(ಶೂನ್ಯ)
#endif
{
	ಪೂರ್ಣಾಙ್ಕ rcode;
	ಸಂರಚನಾ in_str ಒಳಹರಿವು;
#ifndef __U_BOOT__
	setup_file_in_str(&ಒಳಹರಿವು, f);
#ಅನ್ಯಥಾ
	setup_file_in_str(&ಒಳಹರಿವು);
#endif
	rcode = parse_stream_outer(&ಒಳಹರಿವು, FLAG_PARSE_SEMICOLON);
	ಪ್ರತ್ಯಾಗಮ rcode == -2 ? last_return_code : rcode;
}

#ifdef __U_BOOT__
ಪೂರ್ಣಾಙ್ಕ u_boot_hush_start(ಶೂನ್ಯ)
{
	ಯದಿ (ತುದಿ_vars == ರಿಕ್ತ) {
		ತುದಿ_vars = malloc(ಪರಿಮಾಣ(ಸಂರಚನಾ variables));
		ತುದಿ_vars->name = "HUSH_VERSION";
		ತುದಿ_vars->ಮೌಲ್ಯ = "0.01";
		ತುದಿ_vars->next = ರಿಕ್ತ;
		ತುದಿ_vars->flg_export = 0;
		ತುದಿ_vars->flg_read_only = 1;
	}
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಶೂನ್ಯ *xmalloc(size_t size)
{
	ಶೂನ್ಯ *p = ರಿಕ್ತ;

	ಯದಿ (!(p = malloc(size))) {
	    printf("ERROR : xmalloc failed\n");
	    ಪ್ರತಿ(;;);
	}
	ಪ್ರತ್ಯಾಗಮ p;
}

ಸ್ಥಿರ ಶೂನ್ಯ *xrealloc(ಶೂನ್ಯ *ptr, size_t size)
{
	ಶೂನ್ಯ *p = ರಿಕ್ತ;

	ಯದಿ (!(p = realloc(ptr, size))) {
	    printf("ERROR : xrealloc failed\n");
	    ಪ್ರತಿ(;;);
	}
	ಪ್ರತ್ಯಾಗಮ p;
}
#endif /* __U_BOOT__ */

#ifndef __U_BOOT__
/* Make sure we have a controlling tty.  If we get started under a job
 * aware app (like bash for example), make sure we are now in charge so
 * we don't fight over who gets the foreground */
ಸ್ಥಿರ ಶೂನ್ಯ setup_job_control(ಶೂನ್ಯ)
{
	ಸ್ಥಿರ pid_t shell_pgrp;
	/* Loop until we are in the foreground.  */
	ಯಾವತ್ (tcgetpgrp (shell_terminal) != (shell_pgrp = getpgrp ()))
		kill (- shell_pgrp, SIGTTIN);

	/* Ignore interactive and job-control signals.  */
	ಸಙ್ಕೇತ(SIGINT, SIG_IGN);
	ಸಙ್ಕೇತ(SIGQUIT, SIG_IGN);
	ಸಙ್ಕೇತ(SIGTERM, SIG_IGN);
	ಸಙ್ಕೇತ(SIGTSTP, SIG_IGN);
	ಸಙ್ಕೇತ(SIGTTIN, SIG_IGN);
	ಸಙ್ಕೇತ(SIGTTOU, SIG_IGN);
	ಸಙ್ಕೇತ(SIGCHLD, SIG_IGN);

	/* Put ourselves in our own process group.  */
	setsid();
	shell_pgrp = getpid ();
	setpgid (shell_pgrp, shell_pgrp);

	/* Grab control of the terminal.  */
	tcsetpgrp(shell_terminal, shell_pgrp);
}

ಪೂರ್ಣಾಙ್ಕ hush_main(ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ * ಧ್ರುವ *argv)
{
	ಪೂರ್ಣಾಙ್ಕ opt;
	FILE *ಒಳಹರಿವು;
	ವರ್ಣ **e = environ;

	/* XXX what should these be while sourcing /etc/profile? */
	global_argc = argc;
	global_argv = argv;

	/* (re?) initialize globals.  Sometimes hush_main() ends up calling
	 * hush_main(), therefore we cannot rely on the BSS to zero out this
	 * stuff.  Reset these to 0 every time. */
	ifs = ರಿಕ್ತ;
	/* map[] is taken care of with call to update_ifs_map() */
	fake_mode = 0;
	interactive = 0;
	close_me_head = ರಿಕ್ತ;
	last_bg_pid = 0;
	job_list = ರಿಕ್ತ;
	last_jobid = 0;

	/* Initialize some more globals to non-zero values */
	set_cwd();
#ifdef CONFIG_FEATURE_COMMAND_EDITING
	cmdedit_set_initial_prompt();
#ಅನ್ಯಥಾ
	PS1 = ರಿಕ್ತ;
#endif
	PS2 = "> ";

	/* initialize our shell local variables with the values
	 * currently living in the environment */
	ಯದಿ (e) {
		ಪ್ರತಿ (; *e; e++)
			set_local_var(*e, 2);   /* without call putenv() */
	}

	last_return_code=EXIT_SUCCESS;

	ಯದಿ (argv[0] && argv[0][0] == '-') {
		debug_printf("\nsourcing /etc/profile\n");
		ಯದಿ ((ಒಳಹರಿವು = fopen("/etc/profile", "r")) != ರಿಕ್ತ) {
			mark_open(fileno(ಒಳಹರಿವು));
			parse_file_outer(ಒಳಹರಿವು);
			mark_closed(fileno(ಒಳಹರಿವು));
			fclose(ಒಳಹರಿವು);
		}
	}
	ಒಳಹರಿವು=stdin;

	ಯಾವತ್ ((opt = getopt(argc, argv, "c:xif")) > 0) {
		ವಿಕಲ್ಪ (opt) {
			ಪಕ್ಷ 'c':
				{
					global_argv = argv+optind;
					global_argc = argc-optind;
					opt = parse_string_outer(optarg, FLAG_PARSE_SEMICOLON);
					ಗಚ್ಛ final_return;
				}
				ಭಙ್ಗ;
			ಪಕ್ಷ 'i':
				interactive++;
				ಭಙ್ಗ;
			ಪಕ್ಷ 'f':
				fake_mode++;
				ಭಙ್ಗ;
			ಸ್ವಭಾವ:
#ifndef BB_VER
				fprintf(stderr, "Usage: sh [FILE]...\n"
						"   or: sh -c command [args]...\n\n");
				exit(EXIT_FAILURE);
#ಅನ್ಯಥಾ
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
	ಯದಿ (argv[optind]==ರಿಕ್ತ && ಒಳಹರಿವು==stdin &&
			isatty(fileno(stdin)) && isatty(fileno(stdout))) {
		interactive++;
	}

	debug_printf("\ninteractive=%d\n", interactive);
	ಯದಿ (interactive) {
		/* Looks like they want an interactive shell */
#ifndef CONFIG_FEATURE_SH_EXTRA_QUIET
		printf( "\n\n" BB_BANNER " hush - the humble shell v0.01 (testing)\n");
		printf( "Enter 'help' for a list of built-in commands.\n\n");
#endif
		setup_job_control();
	}

	ಯದಿ (argv[optind]==ರಿಕ್ತ) {
		opt=parse_file_outer(stdin);
		ಗಚ್ಛ final_return;
	}

	debug_printf("\nrunning script '%s'\n", argv[optind]);
	global_argv = argv+optind;
	global_argc = argc-optind;
	ಒಳಹರಿವು = xfopen(argv[optind], "r");
	opt = parse_file_outer(ಒಳಹರಿವು);

#ifdef CONFIG_FEATURE_CLEAN_UP
	fclose(ಒಳಹರಿವು);
	ಯದಿ (cwd && cwd != unknown)
		ಮುಕ್ತ((ವರ್ಣ*)cwd);
	{
		ಸಂರಚನಾ variables *cur, *tmp;
		ಪ್ರತಿ(cur = ತುದಿ_vars; cur; cur = tmp) {
			tmp = cur->next;
			ಯದಿ (!cur->flg_read_only) {
				ಮುಕ್ತ(cur->name);
				ಮುಕ್ತ(cur->ಮೌಲ್ಯ);
				ಮುಕ್ತ(cur);
			}
		}
	}
#endif

final_return:
	ಪ್ರತ್ಯಾಗಮ(opt?opt:last_return_code);
}
#endif

ಸ್ಥಿರ ವರ್ಣ *ಸೇರಿಸು_var_ಮೌಲ್ಯ(ವರ್ಣ *inp)
{
	ಪ್ರತ್ಯಾಗಮ ಸೇರಿಸು_var_ಮೌಲ್ಯ_sub(inp, 0);
}

ಸ್ಥಿರ ವರ್ಣ *ಸೇರಿಸು_var_ಮೌಲ್ಯ_sub(ವರ್ಣ *inp, ಪೂರ್ಣಾಙ್ಕ tag_subst)
{
	ಪೂರ್ಣಾಙ್ಕ res_str_len = 0;
	ಪೂರ್ಣಾಙ್ಕ len;
	ಪೂರ್ಣಾಙ್ಕ done = 0;
	ವರ್ಣ *p, *p1, *res_str = ರಿಕ್ತ;

	ಯಾವತ್ ((p = strchr(inp, SPECIAL_VAR_ಚಿಹ್ನೆ))) {
		/* check the beginning of the string for normal characters */
		ಯದಿ (p != inp) {
			/* copy any characters to the result string */
			len = p - inp;
			res_str = xrealloc(res_str, (res_str_len + len));
			strncpy((res_str + res_str_len), inp, len);
			res_str_len += len;
		}
		inp = ++p;
		/* find the ending marker */
		p = strchr(inp, SPECIAL_VAR_ಚಿಹ್ನೆ);
		*p = '\0';
		/* look up the value to substitute */
		ಯದಿ ((p1 = lookup_param(inp))) {
			ಯದಿ (tag_subst)
				len = res_str_len + strlen(p1) + 2;
			ಅನ್ಯಥಾ
				len = res_str_len + strlen(p1);
			res_str = xrealloc(res_str, (1 + len));
			ಯದಿ (tag_subst) {
				/*
				 * copy the variable value to the result
				 * string
				 */
				strcpy((res_str + res_str_len + 1), p1);

				/*
				 * mark the replaced text to be accepted as
				 * is
				 */
				res_str[res_str_len] = SUBSTED_VAR_ಚಿಹ್ನೆ;
				res_str[res_str_len + 1 + strlen(p1)] =
					SUBSTED_VAR_ಚಿಹ್ನೆ;
			} ಅನ್ಯಥಾ
				/*
				 * copy the variable value to the result
				 * string
				 */
				strcpy((res_str + res_str_len), p1);

			res_str_len = len;
		}
		*p = SPECIAL_VAR_ಚಿಹ್ನೆ;
		inp = ++p;
		done = 1;
	}
	ಯದಿ (done) {
		res_str = xrealloc(res_str, (1 + res_str_len + strlen(inp)));
		strcpy((res_str + res_str_len), inp);
		ಯಾವತ್ ((p = strchr(res_str, '\n'))) {
			*p = ' ';
		}
	}
	ಪ್ರತ್ಯಾಗಮ (res_str == ರಿಕ್ತ) ? inp : res_str;
}

ಸ್ಥಿರ ವರ್ಣ **make_list_in(ವರ್ಣ **inp, ವರ್ಣ *name)
{
	ಪೂರ್ಣಾಙ್ಕ len, i;
	ಪೂರ್ಣಾಙ್ಕ name_len = strlen(name);
	ಪೂರ್ಣಾಙ್ಕ n = 0;
	ವರ್ಣ **list;
	ವರ್ಣ *p1, *p2, *p3;

	/* create list of variable values */
	list = xmalloc(ಪರಿಮಾಣ(*list));
	ಪ್ರತಿ (i = 0; inp[i]; i++) {
		p3 = ಸೇರಿಸು_var_ಮೌಲ್ಯ(inp[i]);
		p1 = p3;
		ಯಾವತ್ (*p1) {
			ಯದಿ (*p1 == ' ') {
				p1++;
				ಅನುವರ್ತ;
			}
			ಯದಿ ((p2 = strchr(p1, ' '))) {
				len = p2 - p1;
			} ಅನ್ಯಥಾ {
				len = strlen(p1);
				p2 = p1 + len;
			}
			/* we use n + 2 in realloc for list,because we add
			 * new element and then we will add NULL element */
			list = xrealloc(list, ಪರಿಮಾಣ(*list) * (n + 2));
			list[n] = xmalloc(2 + name_len + len);
			strcpy(list[n], name);
			strcat(list[n], "=");
			strncat(list[n], p1, len);
			list[n++][name_len + len + 1] = '\0';
			p1 = p2;
		}
		ಯದಿ (p3 != inp[i]) ಮುಕ್ತ(p3);
	}
	list[n] = ರಿಕ್ತ;
	ಪ್ರತ್ಯಾಗಮ list;
}

/*
 * Make new string for parser
 * inp     - array of argument strings to flatten
 * nonnull - indicates argument was quoted when originally parsed
 */
ಸ್ಥಿರ ವರ್ಣ *make_string(ವರ್ಣ **inp, ಪೂರ್ಣಾಙ್ಕ *nonnull)
{
	ವರ್ಣ *p;
	ವರ್ಣ *str = ರಿಕ್ತ;
	ಪೂರ್ಣಾಙ್ಕ n;
	ಪೂರ್ಣಾಙ್ಕ len = 2;
	ವರ್ಣ *noeval_str;
	ಪೂರ್ಣಾಙ್ಕ noeval = 0;

	noeval_str = get_local_var("HUSH_NO_EVAL");
	ಯದಿ (noeval_str != ರಿಕ್ತ && *noeval_str != '0' && *noeval_str != '\0')
		noeval = 1;
	ಪ್ರತಿ (n = 0; inp[n]; n++) {
		p = ಸೇರಿಸು_var_ಮೌಲ್ಯ_sub(inp[n], noeval);
		ವರ್ಣ *new_str = xrealloc(str, (len + strlen(p) + (2 * nonnull[n])));
		ಯದಿ (!new_str) {
			ಮುಕ್ತ(str);
			ಯದಿ (p != inp[n]) ಮುಕ್ತ(p);
			ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;
		}
		str = new_str;
		ಯದಿ (n) {
			strcat(str, " ");
		} ಅನ್ಯಥಾ {
			*str = '\0';
		}
		ಯದಿ (nonnull[n])
			strcat(str, "'");
		strcat(str, p);
		ಯದಿ (nonnull[n])
			strcat(str, "'");
		len = strlen(str) + 3;
		ಯದಿ (p != inp[n]) ಮುಕ್ತ(p);
	}
	len = strlen(str);
	*(str + len) = '\n';
	*(str + len + 1) = '\0';
	ಪ್ರತ್ಯಾಗಮ str;
}

#ifdef __U_BOOT__
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ do_showvar(ಸಂರಚನಾ cmd_tbl *cmdtp, ಪೂರ್ಣಾಙ್ಕ flag, ಪೂರ್ಣಾಙ್ಕ argc,
		      ವರ್ಣ *ಧ್ರುವ argv[])
{
	ಪೂರ್ಣಾಙ್ಕ i, k;
	ಪೂರ್ಣಾಙ್ಕ rcode = 0;
	ಸಂರಚನಾ variables *cur;

	ಯದಿ (argc == 1) {		/* Print all env variables	*/
		ಪ್ರತಿ (cur = ತುದಿ_vars; cur; cur = cur->next) {
			printf ("%s=%s\n", cur->name, cur->ಮೌಲ್ಯ);
			ಯದಿ (ctrlc ()) {
				puts ("\n ** Abort\n");
				ಪ್ರತ್ಯಾಗಮ 1;
			}
		}
		ಪ್ರತ್ಯಾಗಮ 0;
	}
	ಪ್ರತಿ (i = 1; i < argc; ++i) {	/* print single env variables	*/
		ವರ್ಣ *name = argv[i];

		k = -1;
		ಪ್ರತಿ (cur = ತುದಿ_vars; cur; cur = cur->next) {
			ಯದಿ(strcmp (cur->name, name) == 0) {
				k = 0;
				printf ("%s=%s\n", cur->name, cur->ಮೌಲ್ಯ);
			}
			ಯದಿ (ctrlc ()) {
				puts ("\n ** Abort\n");
				ಪ್ರತ್ಯಾಗಮ 1;
			}
		}
		ಯದಿ (k < 0) {
			printf ("## Error: \"%s\" not defined\n", name);
			rcode ++;
		}
	}
	ಪ್ರತ್ಯಾಗಮ rcode;
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
