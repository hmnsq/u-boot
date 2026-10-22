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
#ଅନ୍ତର୍ଭାଵ <env.h>
#ଅନ୍ତର୍ଭାଵ <malloc.h>         /* malloc, free, realloc*/
#ଅନ୍ତର୍ଭାଵ <linux/ctype.h>    /* isalpha, isdigit */
#ଅନ୍ତର୍ଭାଵ <console.h>
#ଅନ୍ତର୍ଭାଵ <bootretry.h>
#ଅନ୍ତର୍ଭାଵ <cli.h>
#ଅନ୍ତର୍ଭାଵ <cli_hush.h>
#ଅନ୍ତର୍ଭାଵ <command.h>        /* find_cmd */
#ଅନ୍ତର୍ଭାଵ <vsprintf.h>
#endif
#ifndef __U_BOOT__
#ଅନ୍ତର୍ଭାଵ <ctype.h>     /* isalpha, isdigit */
#ଅନ୍ତର୍ଭାଵ <unistd.h>    /* getpid */
#ଅନ୍ତର୍ଭାଵ <stdlib.h>    /* getenv, atoi */
#ଅନ୍ତର୍ଭାଵ <string.h>    /* strchr */
#ଅନ୍ତର୍ଭାଵ <stdio.h>     /* popen etc. */
#ଅନ୍ତର୍ଭାଵ <glob.h>      /* glob, of course */
#ଅନ୍ତର୍ଭାଵ <stdarg.h>    /* va_list */
#ଅନ୍ତର୍ଭାଵ <errno.h>
#ଅନ୍ତର୍ଭାଵ <fcntl.h>
#ଅନ୍ତର୍ଭାଵ <getopt.h>    /* should be pretty obvious */

#ଅନ୍ତର୍ଭାଵ <sys/stat.h>  /* ulimit */
#ଅନ୍ତର୍ଭାଵ <sys/types.h>
#ଅନ୍ତର୍ଭାଵ <sys/wait.h>
#ଅନ୍ତର୍ଭାଵ <signal.h>

/* #include <dmalloc.h> */

#ଯଦି 1
#ଅନ୍ତର୍ଭାଵ "busybox.h"
#ଅନ୍ତର୍ଭାଵ "cmdedit.h"
#ଅନ୍ୟଥା
#define applet_name "hush"
#ଅନ୍ତର୍ଭାଵ "standalone.h"
#define hush_main ମୁଖ୍ୟ
#define BB_BANNER
#endif
#endif
#define SPECIAL_VAR_ପ୍ରତୀକ 03
#define SUBSTED_VAR_ପ୍ରତୀକ 04
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
#define error_ସନ୍ଦେଶ printf
#ଅନ୍ୟଥା
ପ୍ରକାରବ୍ୟାଖ୍ୟା ଗଣନା {
	REDIRECT_ନିବେଶ     = 1,
	REDIRECT_OVERWRITE = 2,
	REDIRECT_APPEND    = 3,
	REDIRECT_HEREIS    = 4,
	REDIRECT_IO        = 5
} redir_type;

/* The descrip member of this structure is only used to make debugging
 * output pretty */
ସଂରଚନା {ପୂର୍ଣ୍ଣସଂଖ୍ୟା mode; ପୂର୍ଣ୍ଣସଂଖ୍ୟା default_fd; ଅକ୍ଷର *descrip;} redir_table[] = {
	{ 0,                         0, "()" },
	{ O_RDONLY,                  0, "<"  },
	{ O_CREAT|O_TRUNC|O_WRONLY,  1, ">"  },
	{ O_CREAT|O_APPEND|O_WRONLY, 1, ">>" },
	{ O_RDONLY,                 -1, "<<" },
	{ O_RDWR,                    1, "<>" }
};
#endif

ପ୍ରକାରବ୍ୟାଖ୍ୟା ଗଣନା {
	PIPE_SEQ = 1,
	PIPE_AND = 2,
	PIPE_OR  = 3,
	PIPE_BG  = 4,
} pipe_style;

/* might eventually control execution */
ପ୍ରକାରବ୍ୟାଖ୍ୟା ଗଣନା {
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
ସଂରଚନା p_context {
	ସଂରଚନା child_prog *child;
	ସଂରଚନା pipe *list_head;
	ସଂରଚନା pipe *pipe;
#ifndef __U_BOOT__
	ସଂରଚନା redir_struct *pending_redirect;
#endif
	reserved_style w;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା old_flag;				/* for figuring out valid reserved words */
	ସଂରଚନା p_context *ଗଦା;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା type;			/* define type of parser : ";$" common or special symbol */
	/* How about quoting status? */
};

#ifndef __U_BOOT__
ସଂରଚନା redir_struct {
	redir_type type;			/* type of redirection */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd;						/* file descriptor being redirected */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା dup;					/* -1, or file descriptor being duplicated */
	ସଂରଚନା redir_struct *next;	/* pointer to the next redirect in the list */
	glob_t word;				/* *word.gl_pathv is the filename */
};
#endif

ସଂରଚନା child_prog {
#ifndef __U_BOOT__
	pid_t pid;					/* 0 if exited */
#endif
	ଅକ୍ଷର **argv;				/* program name and arguments */
	/* was quoted when parsed; copy of struct o_string.nonnull field */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା *argv_nonnull;
#ifdef __U_BOOT__
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା    argc;                            /* number of program arguments */
#endif
	ସଂରଚନା pipe *group;			/* if non-NULL, first in group or subshell */
#ifndef __U_BOOT__
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା subshell;				/* flag, non-zero if group must be forked */
	ସଂରଚନା redir_struct *redirects;	/* I/O redirections */
	glob_t glob_result;			/* result of parameter globbing */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା is_stopped;				/* is the program currently running? */
	ସଂରଚନା pipe *family;		/* pointer back to the child's parent pipe */
#endif
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା sp;				/* number of SPECIAL_VAR_SYMBOL */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା type;
};

ସଂରଚନା pipe {
#ifndef __U_BOOT__
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା jobid;					/* job number */
#endif
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଅଙ୍କ_progs;				/* total number of programs in job */
#ifndef __U_BOOT__
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା running_progs;			/* number of programs running */
	ଅକ୍ଷର *text;					/* name of job */
	ଅକ୍ଷର *cmdbuf;				/* buffer various argv's point into */
	pid_t pgrp;					/* process group ID for the job */
#endif
	ସଂରଚନା child_prog *progs;	/* array of commands in pipe */
	ସଂରଚନା pipe *next;			/* to track background commands */
#ifndef __U_BOOT__
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା stopped_progs;			/* number of programs alive, but stopped */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା job_context;			/* bitmask defining current context */
#endif
	pipe_style followup;		/* PIPE_BG, PIPE_SEQ, PIPE_OR, PIPE_AND */
	reserved_style r_mode;		/* supports if, for, while, until */
};

#ifndef __U_BOOT__
ସଂରଚନା close_me {
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd;
	ସଂରଚନା close_me *next;
};
#endif

ସଂରଚନା variables {
	ଅକ୍ଷର *name;
	ଅକ୍ଷର *ମୂଲ୍ୟ;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା flg_export;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା flg_read_only;
	ସଂରଚନା variables *next;
};

/* globals, connect us to the outside world
 * the first three support $?, $#, and $1 */
#ifndef __U_BOOT__
ଅକ୍ଷର **global_argv;
unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା global_argc;
#endif
ସ୍ଥିର unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା last_return_code;
#ifndef __U_BOOT__
ବାହ୍ୟ ଅକ୍ଷର **environ; /* This is in <unistd.h>, but protected with __USE_GNU */
#endif

/* "globals" within this file */
ସ୍ଥିର uchar *ifs;
ସ୍ଥିର ଅକ୍ଷର map[256];
#ifndef __U_BOOT__
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fake_mode;
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା interactive;
ସ୍ଥିର ସଂରଚନା close_me *close_me_head;
ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *cwd;
ସ୍ଥିର ସଂରଚନା pipe *job_list;
ସ୍ଥିର unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା last_bg_pid;
ସ୍ଥିର unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା last_jobid;
ସ୍ଥିର unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା shell_terminal;
ସ୍ଥିର ଅକ୍ଷର *PS1;
ସ୍ଥିର ଅକ୍ଷର *PS2;
ସଂରଚନା variables shell_ver = { "HUSH_VERSION", "0.01", 1, 1, 0 };
ସଂରଚନା variables *ଶୀର୍ଷ_vars = &shell_ver;
#ଅନ୍ୟଥା
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା flag_repeat = 0;
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା do_repeat = 0;
ସ୍ଥିର ସଂରଚନା variables *ଶୀର୍ଷ_vars = ଖାଲି ;
#endif /*__U_BOOT__ */

#define B_CHUNK (100)
#define B_NOSPAC 1

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା {
	ଅକ୍ଷର *data;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା length;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା maxlen;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା quote;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା nonnull;
} o_string;
#define NULL_O_STRING {ଖାଲି,0,0,0,0}
/* used for initialization:
	o_string foo = NULL_O_STRING; */

/* I can almost use ordinary FILE *.  Is open_memstream() universally
 * available?  Where is it documented? */
ସଂରଚନା in_str {
	ଅଚଳ unsigned ଅକ୍ଷର *p;
#ifndef __U_BOOT__
	ଅକ୍ଷର peek_buf[2];
#endif
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା __promptme;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା promptmode;
#ifndef __U_BOOT__
	FILE *file;
#endif
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*get) (ସଂରଚନା in_str *);
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*peek) (ସଂରଚନା in_str *);
};
#define b_getch(ନିବେଶ) ((ନିବେଶ)->get(ନିବେଶ))
#define b_peek(ନିବେଶ) ((ନିବେଶ)->peek(ନିବେଶ))

#ifndef __U_BOOT__
#define JOB_STATUS_FORMAT "[%d] %-22s %.40s\n"

ସଂରଚନା built_in_command {
	ଅକ୍ଷର *cmd;					/* name */
	ଅକ୍ଷର *descr;				/* description */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*function) (ସଂରଚନା child_prog *);	/* function ptr */
};
#endif

/* define DEBUG_SHELL for debugging output (obviously ;-)) */
#ଯଦି 0
#define DEBUG_SHELL
#endif

/* This should be in utility.c */
#ifdef DEBUG_SHELL
#ifndef __U_BOOT__
ସ୍ଥିର ଶୂନ୍ୟ debug_printf(ଅଚଳ ଅକ୍ଷର *format, ...)
{
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
}
#ଅନ୍ୟଥା
#define debug_printf(fmt,args...)	printf (fmt ,##args)
#endif
#ଅନ୍ୟଥା
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ debug_printf(ଅଚଳ ଅକ୍ଷର *format, ...) { }
#endif
#define final_printf debug_printf

#ifdef __U_BOOT__
ସ୍ଥିର ଶୂନ୍ୟ syntax_err(ଶୂନ୍ୟ) {
	 printf("syntax error\n");
}
#ଅନ୍ୟଥା
ସ୍ଥିର ଶୂନ୍ୟ __syntax(ଅକ୍ଷର *file, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଧାଡ଼ି) {
	error_ସନ୍ଦେଶ("syntax error %s:%d", file, ଧାଡ଼ି);
}
#define syntax() __syntax(__FILE__, __LINE__)
#endif

#ifdef __U_BOOT__
ସ୍ଥିର ଶୂନ୍ୟ *xmalloc(size_t size);
ସ୍ଥିର ଶୂନ୍ୟ *xrealloc(ଶୂନ୍ୟ *ptr, size_t size);
#ଅନ୍ୟଥା
/* Index of subroutines: */
/*   function prototypes for builtins */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା builtin_cd(ସଂରଚନା child_prog *child);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା builtin_env(ସଂରଚନା child_prog *child);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା builtin_eval(ସଂରଚନା child_prog *child);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା builtin_exec(ସଂରଚନା child_prog *child);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା builtin_exit(ସଂରଚନା child_prog *child);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା builtin_export(ସଂରଚନା child_prog *child);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା builtin_fg_bg(ସଂରଚନା child_prog *child);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା builtin_help(ସଂରଚନା child_prog *child);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା builtin_jobs(ସଂରଚନା child_prog *child);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା builtin_pwd(ସଂରଚନା child_prog *child);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା builtin_read(ସଂରଚନା child_prog *child);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା builtin_set(ସଂରଚନା child_prog *child);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା builtin_shift(ସଂରଚନା child_prog *child);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା builtin_source(ସଂରଚନା child_prog *child);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା builtin_umask(ସଂରଚନା child_prog *child);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା builtin_unset(ସଂରଚନା child_prog *child);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା builtin_not_written(ସଂରଚନା child_prog *child);
#endif
/*   o_string manipulation: */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା b_check_space(o_string *o, ପୂର୍ଣ୍ଣସଂଖ୍ୟା len);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା b_addchr(o_string *o, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ch);
ସ୍ଥିର ଶୂନ୍ୟ b_reset(o_string *o);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା b_addqchr(o_string *o, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ch, ପୂର୍ଣ୍ଣସଂଖ୍ୟା quote);
#ifndef __U_BOOT__
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା b_adduint(o_string *o, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i);
#endif
/*  in_str manipulations: */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା static_get(ସଂରଚନା in_str *i);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା static_peek(ସଂରଚନା in_str *i);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା file_get(ସଂରଚନା in_str *i);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା file_peek(ସଂରଚନା in_str *i);
#ifndef __U_BOOT__
ସ୍ଥିର ଶୂନ୍ୟ setup_file_in_str(ସଂରଚନା in_str *i, FILE *f);
#ଅନ୍ୟଥା
ସ୍ଥିର ଶୂନ୍ୟ setup_file_in_str(ସଂରଚନା in_str *i);
#endif
ସ୍ଥିର ଶୂନ୍ୟ setup_string_in_str(ସଂରଚନା in_str *i, ଅଚଳ ଅକ୍ଷର *s);
#ifndef __U_BOOT__
/*  close_me manipulations: */
ସ୍ଥିର ଶୂନ୍ୟ mark_open(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd);
ସ୍ଥିର ଶୂନ୍ୟ mark_closed(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd);
ସ୍ଥିର ଶୂନ୍ୟ close_all(ଶୂନ୍ୟ);
#endif
/*  "run" the final data structures: */
ସ୍ଥିର ଅକ୍ଷର *indenter(ପୂର୍ଣ୍ଣସଂଖ୍ୟା i);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା free_pipe_list(ସଂରଚନା pipe *head, ପୂର୍ଣ୍ଣସଂଖ୍ୟା indent);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା free_pipe(ସଂରଚନା pipe *pi, ପୂର୍ଣ୍ଣସଂଖ୍ୟା indent);
/*  really run the final data structures: */
#ifndef __U_BOOT__
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା setup_redirects(ସଂରଚନା child_prog *prog, ପୂର୍ଣ୍ଣସଂଖ୍ୟା squirrel[]);
#endif
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା run_list_real(ସଂରଚନା pipe *pi);
#ifndef __U_BOOT__
ସ୍ଥିର ଶୂନ୍ୟ pseudo_exec(ସଂରଚନା child_prog *child) __attribute__ ((noreturn));
#endif
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା run_pipe_real(ସଂରଚନା pipe *pi);
/*   extended glob support: */
#ifndef __U_BOOT__
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା globhack(ଅଚଳ ଅକ୍ଷର *src, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags, glob_t *pglob);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା glob_needed(ଅଚଳ ଅକ୍ଷର *s);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା xglob(o_string *dest, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags, glob_t *pglob);
#endif
/*   variable assignment: */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା is_assignment(ଅଚଳ ଅକ୍ଷର *s);
/*   data structure manipulation: */
#ifndef __U_BOOT__
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା setup_redirect(ସଂରଚନା p_context *ctx, ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd, redir_type style, ସଂରଚନା in_str *ନିବେଶ);
#endif
ସ୍ଥିର ଶୂନ୍ୟ initialize_context(ସଂରଚନା p_context *ctx);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା done_word(o_string *dest, ସଂରଚନା p_context *ctx);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା done_command(ସଂରଚନା p_context *ctx);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା done_pipe(ସଂରଚନା p_context *ctx, pipe_style type);
/*   primary string parsing: */
#ifndef __U_BOOT__
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା redirect_dup_ଅଙ୍କ(ସଂରଚନା in_str *ନିବେଶ);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା redirect_opt_ଅଙ୍କ(o_string *o);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା process_command_subs(o_string *dest, ସଂରଚନା p_context *ctx, ସଂରଚନା in_str *ନିବେଶ, ପୂର୍ଣ୍ଣସଂଖ୍ୟା subst_end);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା parse_group(o_string *dest, ସଂରଚନା p_context *ctx, ସଂରଚନା in_str *ନିବେଶ, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ch);
#endif
ସ୍ଥିର ଅକ୍ଷର *lookup_param(ଅକ୍ଷର *src);
ସ୍ଥିର ଅକ୍ଷର *make_string(ଅକ୍ଷର **inp, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *nonnull);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା handle_dollar(o_string *dest, ସଂରଚନା p_context *ctx, ସଂରଚନା in_str *ନିବେଶ);
#ifndef __U_BOOT__
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା parse_string(o_string *dest, ସଂରଚନା p_context *ctx, ଅଚଳ ଅକ୍ଷର *src);
#endif
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା parse_stream(o_string *dest, ସଂରଚନା p_context *ctx, ସଂରଚନା in_str *input0, ପୂର୍ଣ୍ଣସଂଖ୍ୟା end_trigger);
/*   setup: */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା parse_stream_outer(ସଂରଚନା in_str *inp, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flag);
#ifndef __U_BOOT__
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା parse_string_outer(ଅଚଳ ଅକ୍ଷର *s, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flag);
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା parse_file_outer(FILE *f);
#endif
#ifndef __U_BOOT__
/*   job management: */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା checkjobs(ସଂରଚନା pipe* fg_pipe);
ସ୍ଥିର ଶୂନ୍ୟ ଭର୍ତ୍ତି_bg_job(ସଂରଚନା pipe *pi);
ସ୍ଥିର ଶୂନ୍ୟ remove_bg_job(ସଂରଚନା pipe *pi);
#endif
/*     local variable support */
ସ୍ଥିର ଅକ୍ଷର **make_list_in(ଅକ୍ଷର **inp, ଅକ୍ଷର *name);
ସ୍ଥିର ଅକ୍ଷର *ଭର୍ତ୍ତି_var_ମୂଲ୍ୟ(ଅକ୍ଷର *inp);
ସ୍ଥିର ଅକ୍ଷର *ଭର୍ତ୍ତି_var_ମୂଲ୍ୟ_sub(ଅକ୍ଷର *inp, ପୂର୍ଣ୍ଣସଂଖ୍ୟା tag_subst);

#ifndef __U_BOOT__
/* Table of built-in functions.  They can be forked or not, depending on
 * context: within pipes, they fork.  As simple commands, they do not.
 * When used in non-forking context, they can change global variables
 * in the parent shell process.  If forked, of course they can not.
 * For example, 'unset foo | whatever' will parse and run, but foo will
 * still be set at the end. */
ସ୍ଥିର ସଂରଚନା built_in_command bltins[] = {
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
	{ଖାଲି, ଖାଲି, ଖାଲି}
};

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର *set_cwd(ଶୂନ୍ୟ)
{
	ଯଦି(cwd==unknown)
		cwd = ଖାଲି;     /* xgetcwd(arg) called free(arg) */
	cwd = xgetcwd((ଅକ୍ଷର *)cwd);
	ଯଦି (!cwd)
		cwd = unknown;
	ଫେରନ୍ତୁ cwd;
}

/* built-in 'eval' handler */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା builtin_eval(ସଂରଚନା child_prog *child)
{
	ଅକ୍ଷର *str = ଖାଲି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rcode = EXIT_SUCCESS;

	ଯଦି (child->argv[1]) {
		str = make_string(child->argv + 1);
		parse_string_outer(str, FLAG_EXIT_FROM_LOOP |
					FLAG_PARSE_SEMICOLON);
		ମୁକ୍ତ(str);
		rcode = last_return_code;
	}
	ଫେରନ୍ତୁ rcode;
}

/* built-in 'cd <path>' handler */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା builtin_cd(ସଂରଚନା child_prog *child)
{
	ଅକ୍ଷର *newdir;
	ଯଦି (child->argv[1] == ଖାଲି)
		newdir = env_get("HOME");
	ଅନ୍ୟଥା
		newdir = child->argv[1];
	ଯଦି (chdir(newdir)) {
		printf("cd: %s: %s\n", newdir, strerror(errno));
		ଫେରନ୍ତୁ EXIT_FAILURE;
	}
	set_cwd();
	ଫେରନ୍ତୁ EXIT_SUCCESS;
}

/* built-in 'env' handler */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା builtin_env(ସଂରଚନା child_prog *dummy)
{
	ଅକ୍ଷର **e = environ;
	ଯଦି (e == ଖାଲି) ଫେରନ୍ତୁ EXIT_FAILURE;
	ପାଇଁ (; *e; e++) {
		puts(*e);
	}
	ଫେରନ୍ତୁ EXIT_SUCCESS;
}

/* built-in 'exec' handler */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା builtin_exec(ସଂରଚନା child_prog *child)
{
	ଯଦି (child->argv[1] == ଖାଲି)
		ଫେରନ୍ତୁ EXIT_SUCCESS;   /* Really? */
	child->argv++;
	pseudo_exec(child);
	/* never returns */
}

/* built-in 'exit' handler */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା builtin_exit(ସଂରଚନା child_prog *child)
{
	ଯଦି (child->argv[1] == ଖାଲି)
		exit(last_return_code);
	exit (atoi(child->argv[1]));
}

/* built-in 'export VAR=value' handler */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା builtin_export(ସଂରଚନା child_prog *child)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା res = 0;
	ଅକ୍ଷର *name = child->argv[1];

	ଯଦି (name == ଖାଲି) {
		ଫେରନ୍ତୁ (builtin_env(child));
	}

	name = strdup(name);

	ଯଦି(name) {
		ଅକ୍ଷର *ମୂଲ୍ୟ = strchr(name, '=');

		ଯଦି (!ମୂଲ୍ୟ) {
			ଅକ୍ଷର *tmp;
			/* They are exporting something without an =VALUE */

			ମୂଲ୍ୟ = get_local_var(name);
			ଯଦି (ମୂଲ୍ୟ) {
				size_t ln = strlen(name);

				tmp = realloc(name, ln+strlen(ମୂଲ୍ୟ)+2);
				ଯଦି(tmp==ଖାଲି)
					res = -1;
				ଅନ୍ୟଥା {
					sprintf(tmp+ln, "=%s", ମୂଲ୍ୟ);
					name = tmp;
				}
			} ଅନ୍ୟଥା {
				/* bash does not return an error when trying to export
				 * an undefined variable.  Do likewise. */
				res = 1;
			}
		}
	}
	ଯଦି (res<0)
		perror_ସନ୍ଦେଶ("export");
	ଅନ୍ୟଥା ଯଦି(res==0)
		res = set_local_var(name, 1);
	ଅନ୍ୟଥା
		res = 0;
	ମୁକ୍ତ(name);
	ଫେରନ୍ତୁ res;
}

/* built-in 'fg' and 'bg' handler */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା builtin_fg_bg(ସଂରଚନା child_prog *child)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, jobnum;
	ସଂରଚନା pipe *pi=ଖାଲି;

	ଯଦି (!interactive)
		ଫେରନ୍ତୁ EXIT_FAILURE;
	/* If they gave us no args, assume they want the last backgrounded task */
	ଯଦି (!child->argv[1]) {
		ପାଇଁ (pi = job_list; pi; pi = pi->next) {
			ଯଦି (pi->jobid == last_jobid) {
				ଭାଙ୍ଗ;
			}
		}
		ଯଦି (!pi) {
			error_ସନ୍ଦେଶ("%s: no current job", child->argv[0]);
			ଫେରନ୍ତୁ EXIT_FAILURE;
		}
	} ଅନ୍ୟଥା {
		ଯଦି (sscanf(child->argv[1], "%%%d", &jobnum) != 1) {
			error_ସନ୍ଦେଶ("%s: bad argument '%s'", child->argv[0], child->argv[1]);
			ଫେରନ୍ତୁ EXIT_FAILURE;
		}
		ପାଇଁ (pi = job_list; pi; pi = pi->next) {
			ଯଦି (pi->jobid == jobnum) {
				ଭାଙ୍ଗ;
			}
		}
		ଯଦି (!pi) {
			error_ସନ୍ଦେଶ("%s: %d: no such job", child->argv[0], jobnum);
			ଫେରନ୍ତୁ EXIT_FAILURE;
		}
	}

	ଯଦି (*child->argv[0] == 'f') {
		/* Put the job into the foreground.  */
		tcsetpgrp(shell_terminal, pi->pgrp);
	}

	/* Restart the processes in the job */
	ପାଇଁ (i = 0; i < pi->ଅଙ୍କ_progs; i++)
		pi->progs[i].is_stopped = 0;

	ଯଦି ( (i=kill(- pi->pgrp, SIGCONT)) < 0) {
		ଯଦି (i == ESRCH) {
			remove_bg_job(pi);
		} ଅନ୍ୟଥା {
			perror_ସନ୍ଦେଶ("kill (SIGCONT)");
		}
	}

	pi->stopped_progs = 0;
	ଫେରନ୍ତୁ EXIT_SUCCESS;
}

/* built-in 'help' handler */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା builtin_help(ସଂରଚନା child_prog *dummy)
{
	ସଂରଚନା built_in_command *x;

	printf("\nBuilt-in commands:\n");
	printf("-------------------\n");
	ପାଇଁ (x = bltins; x->cmd; x++) {
		ଯଦି (x->descr==ଖାଲି)
			ଜାରି;
		printf("%s\t%s\n", x->cmd, x->descr);
	}
	printf("\n\n");
	ଫେରନ୍ତୁ EXIT_SUCCESS;
}

/* built-in 'jobs' handler */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା builtin_jobs(ସଂରଚନା child_prog *child)
{
	ସଂରଚନା pipe *job;
	ଅକ୍ଷର *status_string;

	ପାଇଁ (job = job_list; job; job = job->next) {
		ଯଦି (job->running_progs == job->stopped_progs)
			status_string = "Stopped";
		ଅନ୍ୟଥା
			status_string = "Running";

		printf(JOB_STATUS_FORMAT, job->jobid, status_string, job->text);
	}
	ଫେରନ୍ତୁ EXIT_SUCCESS;
}

/* built-in 'pwd' handler */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା builtin_pwd(ସଂରଚନା child_prog *dummy)
{
	puts(set_cwd());
	ଫେରନ୍ତୁ EXIT_SUCCESS;
}

/* built-in 'read VAR' handler */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା builtin_read(ସଂରଚନା child_prog *child)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା res;

	ଯଦି (child->argv[1]) {
		ଅକ୍ଷର string[BUFSIZ];
		ଅକ୍ଷର *var = 0;

		string[0] = 0;  /* In case stdin has only EOF */
		/* read string */
		fgets(string, ଆକାର(string), stdin);
		chomp(string);
		var = malloc(strlen(child->argv[1])+strlen(string)+2);
		ଯଦି(var) {
			sprintf(var, "%s=%s", child->argv[1], string);
			res = set_local_var(var, 0);
		} ଅନ୍ୟଥା
			res = -1;
		ଯଦି (res)
			fprintf(stderr, "read: %m\n");
		ମୁକ୍ତ(var);      /* So not move up to avoid breaking errno */
		ଫେରନ୍ତୁ res;
	} ଅନ୍ୟଥା {
		କର res=getchar(); ଯେତେବେଳେ(res!='\n' && res!=EOF);
		ଫେରନ୍ତୁ 0;
	}
}

/* built-in 'set VAR=value' handler */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା builtin_set(ସଂରଚନା child_prog *child)
{
	ଅକ୍ଷର *temp = child->argv[1];
	ସଂରଚନା variables *e;

	ଯଦି (temp == ଖାଲି)
		ପାଇଁ(e = ଶୀର୍ଷ_vars; e; e=e->next)
			printf("%s=%s\n", e->name, e->ମୂଲ୍ୟ);
	ଅନ୍ୟଥା
		set_local_var(temp, 0);

		ଫେରନ୍ତୁ EXIT_SUCCESS;
}

/* Built-in 'shift' handler */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା builtin_shift(ସଂରଚନା child_prog *child)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା n=1;
	ଯଦି (child->argv[1]) {
		n=atoi(child->argv[1]);
	}
	ଯଦି (n>=0 && n<global_argc) {
		/* XXX This probably breaks $0 */
		global_argc -= n;
		global_argv += n;
		ଫେରନ୍ତୁ EXIT_SUCCESS;
	} ଅନ୍ୟଥା {
		ଫେରନ୍ତୁ EXIT_FAILURE;
	}
}

/* Built-in '.' handler (read-in and execute commands from file) */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା builtin_source(ସଂରଚନା child_prog *child)
{
	FILE *ନିବେଶ;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା status;

	ଯଦି (child->argv[1] == ଖାଲି)
		ଫେରନ୍ତୁ EXIT_FAILURE;

	/* XXX search through $PATH is missing */
	ନିବେଶ = fopen(child->argv[1], "r");
	ଯଦି (!ନିବେଶ) {
		error_ସନ୍ଦେଶ("Couldn't open file '%s'", child->argv[1]);
		ଫେରନ୍ତୁ EXIT_FAILURE;
	}

	/* Now run the file */
	/* XXX argv and argc are broken; need to save old global_argv
	 * (pointer only is OK!) on this stack frame,
	 * set global_argv=child->argv+1, recurse, and restore. */
	mark_open(fileno(ନିବେଶ));
	status = parse_file_outer(ନିବେଶ);
	mark_closed(fileno(ନିବେଶ));
	fclose(ନିବେଶ);
	ଫେରନ୍ତୁ (status);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା builtin_umask(ସଂରଚନା child_prog *child)
{
	mode_t new_umask;
	ଅଚଳ ଅକ୍ଷର *arg = child->argv[1];
	ଅକ୍ଷର *end;
	ଯଦି (arg) {
		new_umask=strtoul(arg, &end, 8);
		ଯଦି (*end!='\0' || end == arg) {
			ଫେରନ୍ତୁ EXIT_FAILURE;
		}
	} ଅନ୍ୟଥା {
		printf("%.3o\n", (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା) (new_umask=umask(0)));
	}
	umask(new_umask);
	ଫେରନ୍ତୁ EXIT_SUCCESS;
}

/* built-in 'unset VAR' handler */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା builtin_unset(ସଂରଚନା child_prog *child)
{
	/* bash returned already true */
	unset_local_var(child->argv[1]);
	ଫେରନ୍ତୁ EXIT_SUCCESS;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା builtin_not_written(ସଂରଚନା child_prog *child)
{
	printf("builtin_%s not written\n",child->argv[0]);
	ଫେରନ୍ତୁ EXIT_FAILURE;
}
#endif

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା b_check_space(o_string *o, ପୂର୍ଣ୍ଣସଂଖ୍ୟା len)
{
	/* It would be easy to drop a more restrictive policy
	 * in here, such as setting a maximum string length */
	ଯଦି (o->length + len > o->maxlen) {
		ଅକ୍ଷର *old_data = o->data;
		/* assert (data == NULL || o->maxlen != 0); */
		o->maxlen += max(2*len, B_CHUNK);
		o->data = realloc(o->data, 1 + o->maxlen);
		ଯଦି (o->data == ଖାଲି) {
			ମୁକ୍ତ(old_data);
		}
	}
	ଫେରନ୍ତୁ o->data == ଖାଲି;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା b_addchr(o_string *o, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ch)
{
	debug_printf("b_addchr: %c %d %p\n", ch, o->length, o);
	ଯଦି (b_check_space(o, 1)) ଫେରନ୍ତୁ B_NOSPAC;
	o->data[o->length] = ch;
	o->length++;
	o->data[o->length] = '\0';
	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ଶୂନ୍ୟ b_reset(o_string *o)
{
	o->length = 0;
	o->nonnull = 0;
	ଯଦି (o->data != ଖାଲି) *o->data = '\0';
}

ସ୍ଥିର ଶୂନ୍ୟ b_free(o_string *o)
{
	b_reset(o);
	ମୁକ୍ତ(o->data);
	o->data = ଖାଲି;
	o->maxlen = 0;
}

/* My analysis of quoting semantics tells me that state information
 * is associated with a destination, not a source.
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା b_addqchr(o_string *o, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ch, ପୂର୍ଣ୍ଣସଂଖ୍ୟା quote)
{
	ଯଦି (quote && strchr("*?[\\",ch)) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
		rc = b_addchr(o, '\\');
		ଯଦି (rc) ଫେରନ୍ତୁ rc;
	}
	ଫେରନ୍ତୁ b_addchr(o, ch);
}

#ifndef __U_BOOT__
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା b_adduint(o_string *o, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା i)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା r;
	ଅକ୍ଷର *p = simple_itoa(i);
	/* no escape checking necessary */
	କର r=b_addchr(o, *p++); ଯେତେବେଳେ (r==0 && *p);
	ଫେରନ୍ତୁ r;
}
#endif

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା static_get(ସଂରଚନା in_str *i)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ch = *i->p++;
	ଯଦି (ch=='\0') ଫେରନ୍ତୁ EOF;
	ଫେରନ୍ତୁ ch;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା static_peek(ସଂରଚନା in_str *i)
{
	ଫେରନ୍ତୁ *i->p;
}

#ifndef __U_BOOT__
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ cmdedit_set_initial_prompt(ଶୂନ୍ୟ)
{
#ifndef CONFIG_FEATURE_SH_FANCY_PROMPT
	PS1 = ଖାଲି;
#ଅନ୍ୟଥା
	PS1 = env_get("PS1");
	ଯଦି(PS1==0)
		PS1 = "\\w \\$ ";
#endif
}

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ setup_prompt_string(ପୂର୍ଣ୍ଣସଂଖ୍ୟା promptmode, ଅକ୍ଷର **prompt_str)
{
	debug_printf("setup_prompt_string %d ",promptmode);
#ifndef CONFIG_FEATURE_SH_FANCY_PROMPT
	/* Set up the prompt */
	ଯଦି (promptmode == 1) {
		ମୁକ୍ତ(PS1);
		PS1=xmalloc(strlen(cwd)+4);
		sprintf(PS1, "%s %s", cwd, ( geteuid() != 0 ) ?  "$ ":"# ");
		*prompt_str = PS1;
	} ଅନ୍ୟଥା {
		*prompt_str = PS2;
	}
#ଅନ୍ୟଥା
	*prompt_str = (promptmode==1)? PS1 : PS2;
#endif
	debug_printf("result %s\n",*prompt_str);
}
#endif

#ifdef __U_BOOT__
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା uboot_cli_readline(ସଂରଚନା in_str *i)
{
	ଅକ୍ଷର *prompt;
	ଅକ୍ଷର __maybe_unused *ps_prompt = ଖାଲି;

	ଯଦି (i->promptmode == 1)
		prompt = CONFIG_SYS_PROMPT;
	ଅନ୍ୟଥା
		prompt = CONFIG_SYS_PROMPT_HUSH_PS2;

#ifdef CONFIG_CMDLINE_PS_SUPPORT
	ଯଦି (i->promptmode == 1)
		ps_prompt = env_get("PS1");
	ଅନ୍ୟଥା
		ps_prompt = env_get("PS2");
	ଯଦି (ps_prompt)
		prompt = ps_prompt;
#endif

	ଫେରନ୍ତୁ cli_readline(prompt);
}
#endif

ସ୍ଥିର ଶୂନ୍ୟ get_user_ନିବେଶ(ସଂରଚନା in_str *i)
{
#ifndef __U_BOOT__
	ଅକ୍ଷର *prompt_str;
	ସ୍ଥିର ଅକ୍ଷର the_command[BUFSIZ];

	setup_prompt_string(i->promptmode, &prompt_str);
#ifdef CONFIG_FEATURE_COMMAND_EDITING
	/*
	 ** enable command line editing only while a command line
	 ** is actually being read; otherwise, we'll end up bequeathing
	 ** atexit() handlers and other unwanted stuff to our
	 ** child processes (rob@sysgo.de)
	 */
	cmdedit_read_ନିବେଶ(prompt_str, the_command);
#ଅନ୍ୟଥା
	fputs(prompt_str, stdout);
	fflush(stdout);
	the_command[0]=fgetc(i->file);
	the_command[1]='\0';
#endif
	fflush(stdout);
	i->p = the_command;
#ଅନ୍ୟଥା
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା n;
	ସ୍ଥିର ଅକ୍ଷର the_command[CONFIG_SYS_CBSIZE + 1];

	bootretry_reset_cmd_timeout();
	i->__promptme = 1;
	n = uboot_cli_readline(i);

#ifdef CONFIG_BOOT_RETRY_TIME
	ଯଦି (n == -2) {
	  puts("\nTimeout waiting for command\n");
#  ifdef CONFIG_RESET_TO_RETRY
	  do_reset(ଖାଲି, 0, 0, ଖାଲି);
#  elif IS_ENABLED(CONFIG_RETRY_BOOTCMD)
	strcpy(console_buffer, "run bootcmd\n");
# ଅନ୍ୟଥା
#	error "This only works with CONFIG_RESET_TO_RETRY or CONFIG_BOOT_RETRY_COMMAND enabled"
#  endif
	}
#endif
	ଯଦି (n == -1 ) {
		flag_repeat = 0;
		i->__promptme = 0;
	}
	n = strlen(console_buffer);
	console_buffer[n] = '\n';
	console_buffer[n+1]= '\0';
	ଯଦି (had_ctrlc()) flag_repeat = 0;
	clear_ctrlc();
	do_repeat = 0;
	ଯଦି (i->promptmode == 1) {
		ଯଦି (console_buffer[0] == '\n'&& flag_repeat == 0) {
			strcpy(the_command,console_buffer);
		}
		ଅନ୍ୟଥା {
			ଯଦି (console_buffer[0] != '\n') {
				strcpy(the_command,console_buffer);
				flag_repeat = 1;
			}
			ଅନ୍ୟଥା {
				do_repeat = 1;
			}
		}
		i->p = the_command;
	}
	ଅନ୍ୟଥା {
		ଯଦି (console_buffer[0] != '\n') {
			ଯଦି (strlen(the_command) + strlen(console_buffer)
			    < CONFIG_SYS_CBSIZE) {
				n = strlen(the_command);
				the_command[n-1] = ' ';
				strcpy(&the_command[n],console_buffer);
			}
			ଅନ୍ୟଥା {
				the_command[0] = '\n';
				the_command[1] = '\0';
				flag_repeat = 0;
			}
		}
		ଯଦି (i->__promptme == 0) {
			the_command[0] = '\n';
			the_command[1] = '\0';
		}
		i->p = console_buffer;
	}
#endif
}

/* This is the magic location that prints prompts
 * and gets data back from the user */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା file_get(ସଂରଚନା in_str *i)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ch;

	ch = 0;
	/* If there is data waiting, eat it up */
	ଯଦି (i->p && *i->p) {
		ch = *i->p++;
	} ଅନ୍ୟଥା {
		/* need to double check i->file because we might be doing something
		 * more complicated by now, like sourcing or substituting. */
#ifndef __U_BOOT__
		ଯଦି (i->__promptme && interactive && i->file == stdin) {
			ଯେତେବେଳେ(! i->p || (interactive && strlen(i->p)==0) ) {
#ଅନ୍ୟଥା
			ଯେତେବେଳେ(! i->p  || strlen(i->p)==0 ) {
#endif
				get_user_ନିବେଶ(i);
			}
			i->promptmode=2;
#ifndef __U_BOOT__
			i->__promptme = 0;
#endif
			ଯଦି (i->p && *i->p) {
				ch = *i->p++;
			}
#ifndef __U_BOOT__
		} ଅନ୍ୟଥା {
			ch = fgetc(i->file);
		}

#endif
		debug_printf("b_getch: got a %d\n", ch);
	}
#ifndef __U_BOOT__
	ଯଦି (ch == '\n') i->__promptme=1;
#endif
	ଫେରନ୍ତୁ ch;
}

/* All the callers guarantee this routine will never be
 * used right after a newline, so prompting is not needed.
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା file_peek(ସଂରଚନା in_str *i)
{
#ifndef __U_BOOT__
	ଯଦି (i->p && *i->p) {
#endif
		ଫେରନ୍ତୁ *i->p;
#ifndef __U_BOOT__
	} ଅନ୍ୟଥା {
		i->peek_buf[0] = fgetc(i->file);
		i->peek_buf[1] = '\0';
		i->p = i->peek_buf;
		debug_printf("b_peek: got a %d\n", *i->p);
		ଫେରନ୍ତୁ *i->p;
	}
#endif
}

#ifndef __U_BOOT__
ସ୍ଥିର ଶୂନ୍ୟ setup_file_in_str(ସଂରଚନା in_str *i, FILE *f)
#ଅନ୍ୟଥା
ସ୍ଥିର ଶୂନ୍ୟ setup_file_in_str(ସଂରଚନା in_str *i)
#endif
{
	i->peek = file_peek;
	i->get = file_get;
	i->__promptme=1;
	i->promptmode=1;
#ifndef __U_BOOT__
	i->file = f;
#endif
	i->p = ଖାଲି;
}

ସ୍ଥିର ଶୂନ୍ୟ setup_string_in_str(ସଂରଚନା in_str *i, ଅଚଳ ଅକ୍ଷର *s)
{
	i->peek = static_peek;
	i->get = static_get;
	i->__promptme=1;
	i->promptmode=1;
	i->p = s;
}

#ifndef __U_BOOT__
ସ୍ଥିର ଶୂନ୍ୟ mark_open(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd)
{
	ସଂରଚନା close_me *new = xmalloc(ଆକାର(ସଂରଚନା close_me));
	new->fd = fd;
	new->next = close_me_head;
	close_me_head = new;
}

ସ୍ଥିର ଶୂନ୍ୟ mark_closed(ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd)
{
	ସଂରଚନା close_me *tmp;
	ଯଦି (close_me_head == ଖାଲି || close_me_head->fd != fd)
		error_ସନ୍ଦେଶ_and_die("corrupt close_me");
	tmp = close_me_head;
	close_me_head = close_me_head->next;
	ମୁକ୍ତ(tmp);
}

ସ୍ଥିର ଶୂନ୍ୟ close_all(ଶୂନ୍ୟ)
{
	ସଂରଚନା close_me *c;
	ପାଇଁ (c=close_me_head; c; c=c->next) {
		close(c->fd);
	}
	close_me_head = ଖାଲି;
}

/* squirrel != NULL means we squirrel away copies of stdin, stdout,
 * and stderr if they are redirected. */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା setup_redirects(ସଂରଚନା child_prog *prog, ପୂର୍ଣ୍ଣସଂଖ୍ୟା squirrel[])
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା openfd, mode;
	ସଂରଚନା redir_struct *redir;

	ପାଇଁ (redir=prog->redirects; redir; redir=redir->next) {
		ଯଦି (redir->dup == -1 && redir->word.gl_pathv == ଖାଲି) {
			/* something went wrong in the parse.  Pretend it didn't happen */
			ଜାରି;
		}
		ଯଦି (redir->dup == -1) {
			mode=redir_table[redir->type].mode;
			openfd = open(redir->word.gl_pathv[0], mode, 0666);
			ଯଦି (openfd < 0) {
			/* this could get lost if stderr has been redirected, but
			   bash and ash both lose it as well (though zsh doesn't!) */
				perror_ସନ୍ଦେଶ("error opening %s", redir->word.gl_pathv[0]);
				ଫେରନ୍ତୁ 1;
			}
		} ଅନ୍ୟଥା {
			openfd = redir->dup;
		}

		ଯଦି (openfd != redir->fd) {
			ଯଦି (squirrel && redir->fd < 3) {
				squirrel[redir->fd] = dup(redir->fd);
			}
			ଯଦି (openfd == -3) {
				close(openfd);
			} ଅନ୍ୟଥା {
				dup2(openfd, redir->fd);
				ଯଦି (redir->dup == -1)
					close (openfd);
			}
		}
	}
	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ଶୂନ୍ୟ restore_redirects(ପୂର୍ଣ୍ଣସଂଖ୍ୟା squirrel[])
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, fd;
	ପାଇଁ (i=0; i<3; i++) {
		fd = squirrel[i];
		ଯଦି (fd != -1) {
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
ସ୍ଥିର ଶୂନ୍ୟ pseudo_exec(ସଂରଚନା child_prog *child)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, rcode;
	ଅକ୍ଷର *p;
	ସଂରଚନା built_in_command *x;
	ଯଦି (child->argv) {
		ପାଇଁ (i=0; is_assignment(child->argv[i]); i++) {
			debug_printf("pid %d environment modification: %s\n",getpid(),child->argv[i]);
			p = ଭର୍ତ୍ତି_var_ମୂଲ୍ୟ(child->argv[i]);
			putenv(strdup(p));
			ଯଦି (p != child->argv[i]) ମୁକ୍ତ(p);
		}
		child->argv+=i;  /* XXX this hack isn't so horrible, since we are about
					to exit, and therefore don't need to keep data
					structures consistent for free() use. */
		/* If a variable is assigned in a forest, and nobody listens,
		 * was it ever really set?
		 */
		ଯଦି (child->argv[0] == ଖାଲି) {
			_exit(EXIT_SUCCESS);
		}

		/*
		 * Check if the command matches any of the builtins.
		 * Depending on context, this might be redundant.  But it's
		 * easier to waste a few CPU cycles than it is to figure out
		 * if this is one of those cases.
		 */
		ପାଇଁ (x = bltins; x->cmd; x++) {
			ଯଦି (strcmp(child->argv[0], x->cmd) == 0 ) {
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
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc_l;
			ଅକ୍ଷର** argv_l=child->argv;
			ଅକ୍ଷର *name = child->argv[0];

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
			ପାଇଁ(argc_l=0;*argv_l!=ଖାଲି; argv_l++, argc_l++);
			optind = 1;
			debug_printf("running applet %s\n", name);
			run_applet_by_name(name, argc_l, child->argv);
		}
#endif
		debug_printf("exec of %s\n",child->argv[0]);
		execvp(child->argv[0],child->argv);
		perror_ସନ୍ଦେଶ("couldn't exec: %s",child->argv[0]);
		_exit(1);
	} ଅନ୍ୟଥା ଯଦି (child->group) {
		debug_printf("runtime nesting to group\n");
		interactive=0;    /* crucial!!!! */
		rcode = run_list_real(child->group);
		/* OK to leak memory by not calling free_pipe_list,
		 * since this process is about to exit */
		_exit(rcode);
	} ଅନ୍ୟଥା {
		/* Can happen.  See what bash does with ">foo" by itself. */
		debug_printf("trying to pseudo_exec null command\n");
		_exit(EXIT_SUCCESS);
	}
}

ସ୍ଥିର ଶୂନ୍ୟ ଭର୍ତ୍ତି_bg_job(ସଂରଚନା pipe *pi)
{
	ସଂରଚନା pipe *thejob;

	/* Linear search for the ID of the job to use */
	pi->jobid = 1;
	ପାଇଁ (thejob = job_list; thejob; thejob = thejob->next)
		ଯଦି (thejob->jobid >= pi->jobid)
			pi->jobid = thejob->jobid + 1;

	/* add thejob to the list of running jobs */
	ଯଦି (!job_list) {
		thejob = job_list = xmalloc(ଆକାର(*thejob));
	} ଅନ୍ୟଥା {
		ପାଇଁ (thejob = job_list; thejob->next; thejob = thejob->next) /* nothing */;
		thejob->next = xmalloc(ଆକାର(*thejob));
		thejob = thejob->next;
	}

	/* physically copy the struct job */
	memcpy(thejob, pi, ଆକାର(ସଂରଚନା pipe));
	thejob->next = ଖାଲି;
	thejob->running_progs = thejob->ଅଙ୍କ_progs;
	thejob->stopped_progs = 0;
	thejob->text = xmalloc(BUFSIZ); /* cmdedit buffer size */

	/*if (pi->progs[0] && pi->progs[0].argv && pi->progs[0].argv[0]) */
	{
		ଅକ୍ଷର *bar=thejob->text;
		ଅକ୍ଷର **foo=pi->progs[0].argv;
		ଯେତେବେଳେ(foo && *foo) {
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
ସ୍ଥିର ଶୂନ୍ୟ remove_bg_job(ସଂରଚନା pipe *pi)
{
	ସଂରଚନା pipe *prev_pipe;

	ଯଦି (pi == job_list) {
		job_list = pi->next;
	} ଅନ୍ୟଥା {
		prev_pipe = job_list;
		ଯେତେବେଳେ (prev_pipe->next != pi)
			prev_pipe = prev_pipe->next;
		prev_pipe->next = pi->next;
	}
	ଯଦି (job_list)
		last_jobid = job_list->jobid;
	ଅନ୍ୟଥା
		last_jobid = 0;

	pi->stopped_progs = 0;
	free_pipe(pi, 0);
	ମୁକ୍ତ(pi);
}

/* Checks to see if any processes have exited -- if they
   have, figure out why and see if a job has completed */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା checkjobs(ସଂରଚନା pipe* fg_pipe)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା attributes;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା status;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା prognum = 0;
	ସଂରଚନା pipe *pi;
	pid_t childpid;

	attributes = WUNTRACED;
	ଯଦି (fg_pipe==ଖାଲି) {
		attributes |= WNOHANG;
	}

	ଯେତେବେଳେ ((childpid = waitpid(-1, &status, attributes)) > 0) {
		ଯଦି (fg_pipe) {
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, rcode = 0;
			ପାଇଁ (i=0; i < fg_pipe->ଅଙ୍କ_progs; i++) {
				ଯଦି (fg_pipe->progs[i].pid == childpid) {
					ଯଦି (i==fg_pipe->ଅଙ୍କ_progs-1)
						rcode=WEXITSTATUS(status);
					(fg_pipe->ଅଙ୍କ_progs)--;
					ଫେରନ୍ତୁ(rcode);
				}
			}
		}

		ପାଇଁ (pi = job_list; pi; pi = pi->next) {
			prognum = 0;
			ଯେତେବେଳେ (prognum < pi->ଅଙ୍କ_progs && pi->progs[prognum].pid != childpid) {
				prognum++;
			}
			ଯଦି (prognum < pi->ଅଙ୍କ_progs)
				ଭାଙ୍ଗ;
		}

		ଯଦି(pi==ଖାଲି) {
			debug_printf("checkjobs: pid %d was not in our list!\n", childpid);
			ଜାରି;
		}

		ଯଦି (WIFEXITED(status) || WIFSIGNALED(status)) {
			/* child exited */
			pi->running_progs--;
			pi->progs[prognum].pid = 0;

			ଯଦି (!pi->running_progs) {
				printf(JOB_STATUS_FORMAT, pi->jobid, "Done", pi->text);
				remove_bg_job(pi);
			}
		} ଅନ୍ୟଥା {
			/* child stopped */
			pi->stopped_progs++;
			pi->progs[prognum].is_stopped = 1;

#ଯଦି 0
			/* Printing this stuff is a pain, since it tends to
			 * overwrite the prompt an inconveinient moments.  So
			 * don't do that.  */
			ଯଦି (pi->stopped_progs == pi->ଅଙ୍କ_progs) {
				printf("\n"JOB_STATUS_FORMAT, pi->jobid, "Stopped", pi->text);
			}
#endif
		}
	}

	ଯଦି (childpid == -1 && errno != ECHILD)
		perror_ସନ୍ଦେଶ("waitpid");

	/* move the shell to the foreground */
	/*if (interactive && tcsetpgrp(shell_terminal, getpgid(0))) */
	/*	perror_msg("tcsetpgrp-2"); */
	ଫେରନ୍ତୁ -1;
}

/* Figure out our controlling tty, checking in order stderr,
 * stdin, and stdout.  If check_pgrp is set, also check that
 * we belong to the foreground process group associated with
 * that tty.  The value of shell_terminal is needed in order to call
 * tcsetpgrp(shell_terminal, ...); */
ଶୂନ୍ୟ controlling_tty(ପୂର୍ଣ୍ଣସଂଖ୍ୟା check_pgrp)
{
	pid_t curpgrp;

	ଯଦି ((curpgrp = tcgetpgrp(shell_terminal = 2)) < 0
			&& (curpgrp = tcgetpgrp(shell_terminal = 0)) < 0
			&& (curpgrp = tcgetpgrp(shell_terminal = 1)) < 0)
		ଯାଅ shell_terminal_error;

	ଯଦି (check_pgrp && curpgrp != getpgid(0))
		ଯାଅ shell_terminal_error;

	ଫେରନ୍ତୁ;

shell_terminal_error:
		shell_terminal = -1;
		ଫେରନ୍ତୁ;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା run_pipe_real(ସଂରଚନା pipe *pi)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
#ifndef __U_BOOT__
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା nextin, nextout;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା pipefds[2];				/* pipefds[0] is for reading */
	ସଂରଚନା child_prog *child;
	ସଂରଚନା built_in_command *x;
	ଅକ୍ଷର *p;
# ଯଦି __GNUC__
	/* Avoid longjmp clobbering */
	(ଶୂନ୍ୟ) &i;
	(ଶୂନ୍ୟ) &nextin;
	(ଶୂନ୍ୟ) &nextout;
	(ଶୂନ୍ୟ) &child;
# endif
#ଅନ୍ୟଥା
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା nextin;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା flag = do_repeat ? CMD_FLAG_REPEAT : 0;
	ସଂରଚନା child_prog *child;
	ଅକ୍ଷର *p;
# ଯଦି __GNUC__
	/* Avoid longjmp clobbering */
	(ଶୂନ୍ୟ) &i;
	(ଶୂନ୍ୟ) &nextin;
	(ଶୂନ୍ୟ) &child;
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
	ଯଦି (pi->ଅଙ୍କ_progs == 1) child = & (pi->progs[0]);
#ifndef __U_BOOT__
	ଯଦି (pi->ଅଙ୍କ_progs == 1 && child->group && child->subshell == 0) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା squirrel[] = {-1, -1, -1};
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା rcode;
		debug_printf("non-subshell grouping\n");
		setup_redirects(child, squirrel);
		/* XXX could we merge code with following builtin case,
		 * by creating a pseudo builtin that calls run_list_real? */
		rcode = run_list_real(child->group);
		restore_redirects(squirrel);
#ଅନ୍ୟଥା
		ଯଦି (pi->ଅଙ୍କ_progs == 1 && child->group) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା rcode;
		debug_printf("non-subshell grouping\n");
		rcode = run_list_real(child->group);
#endif
		ଫେରନ୍ତୁ rcode;
	} ଅନ୍ୟଥା ଯଦି (pi->ଅଙ୍କ_progs == 1 && pi->progs[0].argv != ଖାଲି) {
		ପାଇଁ (i=0; is_assignment(child->argv[i]); i++) { /* nothing */ }
		ଯଦି (i!=0 && child->argv[i]==ଖାଲି) {
			/* assignments, but no command: set the local environment */
			ପାଇଁ (i=0; child->argv[i]!=ଖାଲି; i++) {

				/* Ok, this case is tricky.  We have to decide if this is a
				 * local variable, or an already exported variable.  If it is
				 * already exported, we have to export the new value.  If it is
				 * not exported, we need only set this as a local variable.
				 * This junk is all to decide whether or not to export this
				 * variable. */
				ପୂର୍ଣ୍ଣସଂଖ୍ୟା export_me=0;
				ଅକ୍ଷର *name, *ମୂଲ୍ୟ;
				name = xstrdup(child->argv[i]);
				debug_printf("Local environment set: %s\n", name);
				ମୂଲ୍ୟ = strchr(name, '=');
				ଯଦି (ମୂଲ୍ୟ)
					*ମୂଲ୍ୟ=0;
#ifndef __U_BOOT__
				ଯଦି ( get_local_var(name)) {
					export_me=1;
				}
#endif
				ମୁକ୍ତ(name);
				p = ଭର୍ତ୍ତି_var_ମୂଲ୍ୟ(child->argv[i]);
				set_local_var(p, export_me);
				ଯଦି (p != child->argv[i]) ମୁକ୍ତ(p);
			}
			ଫେରନ୍ତୁ EXIT_SUCCESS;   /* don't worry about errors in set_local_var() yet */
		}
		ପାଇଁ (i = 0; is_assignment(child->argv[i]); i++) {
			p = ଭର୍ତ୍ତି_var_ମୂଲ୍ୟ(child->argv[i]);
#ifndef __U_BOOT__
			putenv(strdup(p));
#ଅନ୍ୟଥା
			set_local_var(p, 0);
#endif
			ଯଦି (p != child->argv[i]) {
				child->sp--;
				ମୁକ୍ତ(p);
			}
		}
		ଯଦି (child->sp) {
			ଅକ୍ଷର * str = ଖାଲି;

			str = make_string(child->argv + i,
					  child->argv_nonnull + i);
			parse_string_outer(str, FLAG_EXIT_FROM_LOOP | FLAG_REPARSING);
			ମୁକ୍ତ(str);
			ଫେରନ୍ତୁ last_return_code;
		}
#ifndef __U_BOOT__
		ପାଇଁ (x = bltins; x->cmd; x++) {
			ଯଦି (strcmp(child->argv[i], x->cmd) == 0 ) {
				ପୂର୍ଣ୍ଣସଂଖ୍ୟା squirrel[] = {-1, -1, -1};
				ପୂର୍ଣ୍ଣସଂଖ୍ୟା rcode;
				ଯଦି (x->function == builtin_exec && child->argv[i+1]==ଖାଲି) {
					debug_printf("magic exec\n");
					setup_redirects(child,ଖାଲି);
					ଫେରନ୍ତୁ EXIT_SUCCESS;
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
			ଫେରନ୍ତୁ rcode;
		}
#ଅନ୍ୟଥା
		/* check ";", because ,example , argv consist from
		 * "help;flinfo" must not execute
		 */
		ଯଦି (strchr(child->argv[i], ';')) {
			printf("Unknown command '%s' - try 'help' or use "
					"'run' command\n", child->argv[i]);
			ଫେରନ୍ତୁ -1;
		}
		/* Process the command */
		ଫେରନ୍ତୁ cmd_process(flag, child->argc - i, child->argv + i,
				   &flag_repeat, ଖାଲି);
#endif
	}
#ifndef __U_BOOT__

	ପାଇଁ (i = 0; i < pi->ଅଙ୍କ_progs; i++) {
		child = & (pi->progs[i]);

		/* pipes are inserted between pairs of commands */
		ଯଦି ((i + 1) < pi->ଅଙ୍କ_progs) {
			ଯଦି (pipe(pipefds)<0) perror_ସନ୍ଦେଶ_and_die("pipe");
			nextout = pipefds[1];
		} ଅନ୍ୟଥା {
			nextout=1;
			pipefds[0] = -1;
		}

		/* XXX test for failed fork()? */
		ଯଦି (!(child->pid = fork())) {
			/* Set the handling for job control signals back to the default.  */
			ସଙ୍କେତ(SIGINT, SIG_DFL);
			ସଙ୍କେତ(SIGQUIT, SIG_DFL);
			ସଙ୍କେତ(SIGTERM, SIG_DFL);
			ସଙ୍କେତ(SIGTSTP, SIG_DFL);
			ସଙ୍କେତ(SIGTTIN, SIG_DFL);
			ସଙ୍କେତ(SIGTTOU, SIG_DFL);
			ସଙ୍କେତ(SIGCHLD, SIG_DFL);

			close_all();

			ଯଦି (nextin != 0) {
				dup2(nextin, 0);
				close(nextin);
			}
			ଯଦି (nextout != 1) {
				dup2(nextout, 1);
				close(nextout);
			}
			ଯଦି (pipefds[0]!=-1) {
				close(pipefds[0]);  /* opposite end of our output pipe */
			}

			/* Like bash, explicit redirects override pipes,
			 * and the pipe fd is available for dup'ing. */
			setup_redirects(child,ଖାଲି);

			ଯଦି (interactive && pi->followup!=PIPE_BG) {
				/* If we (the child) win the race, put ourselves in the process
				 * group whose leader is the first process in this pipe. */
				ଯଦି (pi->pgrp < 0) {
					pi->pgrp = getpid();
				}
				ଯଦି (setpgid(0, pi->pgrp) == 0) {
					tcsetpgrp(2, pi->pgrp);
				}
			}

			pseudo_exec(child);
		}

		/* put our child in the process group whose leader is the
		   first process in this pipe */
		ଯଦି (pi->pgrp < 0) {
			pi->pgrp = child->pid;
		}
		/* Don't check for errors.  The child may be dead already,
		 * in which case setpgid returns error code EACCES. */
		setpgid(child->pid, pi->pgrp);

		ଯଦି (nextin != 0)
			close(nextin);
		ଯଦି (nextout != 1)
			close(nextout);

		/* If there isn't another process, nextin is garbage
		   but it doesn't matter */
		nextin = pipefds[0];
	}
#endif
	ଫେରନ୍ତୁ -1;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା run_list_real(ସଂରଚନା pipe *pi)
{
	ଅକ୍ଷର *save_name = ଖାଲି;
	ଅକ୍ଷର **list = ଖାଲି;
	ଅକ୍ଷର **save_list = ଖାଲି;
	ସଂରଚନା pipe *rpipe;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା flag_rep = 0;
#ifndef __U_BOOT__
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା save_ଅଙ୍କ_progs;
#endif
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rcode=0, flag_skip=1;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା flag_restore = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା if_code=0, next_if_code=0;  /* need double-buffer to handle elif */
	reserved_style rmode, skip_more_in_this_rmode=RES_XXXX;
	/* check syntax for "for" */
	ପାଇଁ (rpipe = pi; rpipe; rpipe = rpipe->next) {
		ଯଦି ((rpipe->r_mode == RES_IN ||
		    rpipe->r_mode == RES_FOR) &&
		    (rpipe->next == ଖାଲି)) {
				syntax();
#ifdef __U_BOOT__
				flag_repeat = 0;
#endif
				ଫେରନ୍ତୁ 1;
		}
		ଯଦି ((rpipe->r_mode == RES_IN &&
			(rpipe->next->r_mode == RES_IN &&
			rpipe->next->progs->argv != ଖାଲି))||
			(rpipe->r_mode == RES_FOR &&
			rpipe->next->r_mode != RES_IN)) {
				syntax();
#ifdef __U_BOOT__
				flag_repeat = 0;
#endif
				ଫେରନ୍ତୁ 1;
		}
	}
	ପାଇଁ (; pi; pi = (flag_restore != 0) ? rpipe : pi->next) {
		ଯଦି (pi->r_mode == RES_WHILE || pi->r_mode == RES_UNTIL ||
			pi->r_mode == RES_FOR) {
#ifdef __U_BOOT__
				/* check Ctrl-C */
				ctrlc();
				ଯଦି ((had_ctrlc())) {
					ଫେରନ୍ତୁ 1;
				}
#endif
				flag_restore = 0;
				ଯଦି (!rpipe) {
					flag_rep = 0;
					rpipe = pi;
				}
		}
		rmode = pi->r_mode;
		debug_printf("rmode=%d  if_code=%d  next_if_code=%d skip_more=%d\n", rmode, if_code, next_if_code, skip_more_in_this_rmode);
		ଯଦି (rmode == skip_more_in_this_rmode && flag_skip) {
			ଯଦି (pi->followup == PIPE_SEQ) flag_skip=0;
			ଜାରି;
		}
		flag_skip = 1;
		skip_more_in_this_rmode = RES_XXXX;
		ଯଦି (rmode == RES_THEN || rmode == RES_ELSE) if_code = next_if_code;
		ଯଦି (rmode == RES_THEN &&  if_code) ଜାରି;
		ଯଦି (rmode == RES_ELSE && !if_code) ଜାରି;
		ଯଦି (rmode == RES_ELIF && !if_code) ଭାଙ୍ଗ;
		ଯଦି (rmode == RES_FOR && pi->ଅଙ୍କ_progs) {
			ଯଦି (!list) {
				/* if no variable values after "in" we skip "for" */
				ଯଦି (!pi->next->progs->argv) ଜାରି;
				/* create list of variable values */
				list = make_list_in(pi->next->progs->argv,
					pi->progs->argv[0]);
				save_list = list;
				save_name = pi->progs->argv[0];
				pi->progs->argv[0] = ଖାଲି;
				flag_rep = 1;
			}
			ଯଦି (!(*list)) {
				ମୁକ୍ତ(pi->progs->argv[0]);
				ମୁକ୍ତ(save_list);
				list = ଖାଲି;
				flag_rep = 0;
				pi->progs->argv[0] = save_name;
#ifndef __U_BOOT__
				pi->progs->glob_result.gl_pathv[0] =
					pi->progs->argv[0];
#endif
				ଜାରି;
			} ଅନ୍ୟଥା {
				/* insert new value from list for variable */
				ମୁକ୍ତ(pi->progs->argv[0]);
				pi->progs->argv[0] = *list++;
#ifndef __U_BOOT__
				pi->progs->glob_result.gl_pathv[0] =
					pi->progs->argv[0];
#endif
			}
		}
		ଯଦି (rmode == RES_IN) ଜାରି;
		ଯଦି (rmode == RES_DO) {
			ଯଦି (!flag_rep) ଜାରି;
		}
		ଯଦି (rmode == RES_DONE) {
			ଯଦି (flag_rep) {
				flag_restore = 1;
			} ଅନ୍ୟଥା {
				rpipe = ଖାଲି;
			}
		}
		ଯଦି (pi->ଅଙ୍କ_progs == 0) ଜାରି;
#ifndef __U_BOOT__
		save_ଅଙ୍କ_progs = pi->ଅଙ୍କ_progs; /* save number of programs */
#endif
		rcode = run_pipe_real(pi);
		debug_printf("run_pipe_real returned %d\n",rcode);
#ifndef __U_BOOT__
		ଯଦି (rcode!=-1) {
			/* We only ran a builtin: rcode was set by the return value
			 * of run_pipe_real(), and we don't need to wait for anything. */
		} ଅନ୍ୟଥା ଯଦି (pi->followup==PIPE_BG) {
			/* XXX check bash's behavior with nontrivial pipes */
			/* XXX compute jobid */
			/* XXX what does bash do with attempts to background builtins? */
			ଭର୍ତ୍ତି_bg_job(pi);
			rcode = EXIT_SUCCESS;
		} ଅନ୍ୟଥା {
			ଯଦି (interactive) {
				/* move the new process group into the foreground */
				ଯଦି (tcsetpgrp(shell_terminal, pi->pgrp) && errno != ENOTTY)
					perror_ସନ୍ଦେଶ("tcsetpgrp-3");
				rcode = checkjobs(pi);
				/* move the shell to the foreground */
				ଯଦି (tcsetpgrp(shell_terminal, getpgid(0)) && errno != ENOTTY)
					perror_ସନ୍ଦେଶ("tcsetpgrp-4");
			} ଅନ୍ୟଥା {
				rcode = checkjobs(pi);
			}
			debug_printf("checkjobs returned %d\n",rcode);
		}
		last_return_code=rcode;
#ଅନ୍ୟଥା
		ଯଦି (rcode < -1) {
			last_return_code = -rcode - 2;
			ଫେରନ୍ତୁ -2;	/* exit */
		}
		last_return_code = rcode;
#endif
#ifndef __U_BOOT__
		pi->ଅଙ୍କ_progs = save_ଅଙ୍କ_progs; /* restore number of programs */
#endif
		ଯଦି ( rmode == RES_IF || rmode == RES_ELIF )
			next_if_code=rcode;  /* can be overwritten a number of times */
		ଯଦି (rmode == RES_WHILE)
			flag_rep = !last_return_code;
		ଯଦି (rmode == RES_UNTIL)
			flag_rep = last_return_code;
		ଯଦି ( (rcode==EXIT_SUCCESS && pi->followup==PIPE_OR) ||
		     (rcode!=EXIT_SUCCESS && pi->followup==PIPE_AND) )
			skip_more_in_this_rmode=rmode;
#ifndef __U_BOOT__
		checkjobs(ଖାଲି);
#endif
	}
	ଫେରନ୍ତୁ rcode;
}

/* broken, of course, but OK for testing */
ସ୍ଥିର ଅକ୍ଷର *indenter(ପୂର୍ଣ୍ଣସଂଖ୍ୟା i)
{
	ସ୍ଥିର ଅକ୍ଷର blanks[]="                                    ";
	ଫେରନ୍ତୁ &blanks[ଆକାର(blanks)-i-1];
}

/* return code is the exit status of the pipe */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା free_pipe(ସଂରଚନା pipe *pi, ପୂର୍ଣ୍ଣସଂଖ୍ୟା indent)
{
	ଅକ୍ଷର **p;
	ସଂରଚନା child_prog *child;
#ifndef __U_BOOT__
	ସଂରଚନା redir_struct *r, *rnext;
#endif
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା a, i, ret_code=0;
	ଅକ୍ଷର *ind = indenter(indent);

#ifndef __U_BOOT__
	ଯଦି (pi->stopped_progs > 0)
		ଫେରନ୍ତୁ ret_code;
	final_printf("%s run pipe: (pid %d)\n",ind,getpid());
#endif
	ପାଇଁ (i=0; i<pi->ଅଙ୍କ_progs; i++) {
		child = &pi->progs[i];
		final_printf("%s  command %d:\n",ind,i);
		ଯଦି (child->argv) {
			ପାଇଁ (a=0,p=child->argv; *p; a++,p++) {
				final_printf("%s   argv[%d] = %s\n",ind,a,*p);
			}
#ifndef __U_BOOT__
			globfree(&child->glob_result);
#ଅନ୍ୟଥା
			ପାଇଁ (a = 0; a < child->argc; a++) {
				ମୁକ୍ତ(child->argv[a]);
			}
			ମୁକ୍ତ(child->argv);
			ମୁକ୍ତ(child->argv_nonnull);
			child->argc = 0;
#endif
			child->argv=ଖାଲି;
		} ଅନ୍ୟଥା ଯଦି (child->group) {
#ifndef __U_BOOT__
			final_printf("%s   begin group (subshell:%d)\n",ind, child->subshell);
#endif
			ret_code = free_pipe_list(child->group,indent+3);
			final_printf("%s   end group\n",ind);
		} ଅନ୍ୟଥା {
			final_printf("%s   (nil)\n",ind);
		}
#ifndef __U_BOOT__
		ପାଇଁ (r=child->redirects; r; r=rnext) {
			final_printf("%s   redirect %d%s", ind, r->fd, redir_table[r->type].descrip);
			ଯଦି (r->dup == -1) {
				/* guard against the case >$FOO, where foo is unset or blank */
				ଯଦି (r->word.gl_pathv) {
					final_printf(" %s\n", *r->word.gl_pathv);
					globfree(&r->word);
				}
			} ଅନ୍ୟଥା {
				final_printf("&%d\n", r->dup);
			}
			rnext=r->next;
			ମୁକ୍ତ(r);
		}
		child->redirects=ଖାଲି;
#endif
	}
	ମୁକ୍ତ(pi->progs);   /* children are an array, they get freed all at once */
	pi->progs=ଖାଲି;
	ଫେରନ୍ତୁ ret_code;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା free_pipe_list(ସଂରଚନା pipe *head, ପୂର୍ଣ୍ଣସଂଖ୍ୟା indent)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rcode=0;   /* if list has no members */
	ସଂରଚନା pipe *pi, *next;
	ଅକ୍ଷର *ind = indenter(indent);
	ପାଇଁ (pi=head; pi; pi=next) {
		final_printf("%s pipe reserved mode %d\n", ind, pi->r_mode);
		rcode = free_pipe(pi, indent);
		final_printf("%s pipe followup code %d\n", ind, pi->followup);
		next=pi->next;
		pi->next=ଖାଲି;
		ମୁକ୍ତ(pi);
	}
	ଫେରନ୍ତୁ rcode;
}

/* Select which version we will use */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା run_list(ସଂରଚନା pipe *pi)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rcode=0;
#ifndef __U_BOOT__
	ଯଦି (fake_mode==0) {
#endif
		rcode = run_list_real(pi);
#ifndef __U_BOOT__
	}
#endif
	/* free_pipe_list has the side effect of clearing memory
	 * In the long run that function can be merged with run_list_real,
	 * but doing that now would hobble the debugging effort. */
	free_pipe_list(pi,0);
	ଫେରନ୍ତୁ rcode;
}

/* The API for glob is arguably broken.  This routine pushes a non-matching
 * string into the output structure, removing non-backslashed backslashes.
 * If someone can prove me wrong, by performing this function within the
 * original glob(3) api, feel free to rewrite this routine into oblivion.
 * Return code (0 vs. GLOB_NOSPACE) matches glob(3).
 * XXX broken if the last character is '\\', check that before calling.
 */
#ifndef __U_BOOT__
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା globhack(ଅଚଳ ଅକ୍ଷର *src, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags, glob_t *pglob)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା cnt=0, pathc;
	ଅଚଳ ଅକ୍ଷର *s;
	ଅକ୍ଷର *dest;
	ପାଇଁ (cnt=1, s=src; s && *s; s++) {
		ଯଦି (*s == '\\') s++;
		cnt++;
	}
	dest = malloc(cnt);
	ଯଦି (!dest) ଫେରନ୍ତୁ GLOB_NOSPACE;
	ଯଦି (!(flags & GLOB_APPEND)) {
		pglob->gl_pathv=ଖାଲି;
		pglob->gl_pathc=0;
		pglob->gl_offs=0;
		pglob->gl_offs=0;
	}
	pathc = ++pglob->gl_pathc;
	pglob->gl_pathv = realloc(pglob->gl_pathv, (pathc+1)*ଆକାର(*pglob->gl_pathv));
	ଯଦି (pglob->gl_pathv == ଖାଲି) ଫେରନ୍ତୁ GLOB_NOSPACE;
	pglob->gl_pathv[pathc-1]=dest;
	pglob->gl_pathv[pathc]=ଖାଲି;
	ପାଇଁ (s=src; s && *s; s++, dest++) {
		ଯଦି (*s == '\\') s++;
		*dest = *s;
	}
	*dest='\0';
	ଫେରନ୍ତୁ 0;
}

/* XXX broken if the last character is '\\', check that before calling */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା glob_needed(ଅଚଳ ଅକ୍ଷର *s)
{
	ପାଇଁ (; *s; s++) {
		ଯଦି (*s == '\\') s++;
		ଯଦି (strchr("*[?",*s)) ଫେରନ୍ତୁ 1;
	}
	ଫେରନ୍ତୁ 0;
}

#ଯଦି 0
ସ୍ଥିର ଶୂନ୍ୟ globprint(glob_t *pglob)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	debug_printf("glob_t at %p:\n", pglob);
	debug_printf("  gl_pathc=%d  gl_pathv=%p  gl_offs=%d  gl_flags=%d\n",
		pglob->gl_pathc, pglob->gl_pathv, pglob->gl_offs, pglob->gl_flags);
	ପାଇଁ (i=0; i<pglob->gl_pathc; i++)
		debug_printf("pglob->gl_pathv[%d] = %p = %s\n", i,
			pglob->gl_pathv[i], pglob->gl_pathv[i]);
}
#endif

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା xglob(o_string *dest, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flags, glob_t *pglob)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା gr;

	/* short-circuit for null word */
	/* we can code this better when the debug_printf's are gone */
	ଯଦି (dest->length == 0) {
		ଯଦି (dest->nonnull) {
			/* bash man page calls this an "explicit" null */
			gr = globhack(dest->data, flags, pglob);
			debug_printf("globhack returned %d\n",gr);
		} ଅନ୍ୟଥା {
			ଫେରନ୍ତୁ 0;
		}
	} ଅନ୍ୟଥା ଯଦି (glob_needed(dest->data)) {
		gr = glob(dest->data, flags, ଖାଲି, pglob);
		debug_printf("glob returned %d\n",gr);
		ଯଦି (gr == GLOB_NOMATCH) {
			/* quote removal, or more accurately, backslash removal */
			gr = globhack(dest->data, flags, pglob);
			debug_printf("globhack returned %d\n",gr);
		}
	} ଅନ୍ୟଥା {
		gr = globhack(dest->data, flags, pglob);
		debug_printf("globhack returned %d\n",gr);
	}
	ଯଦି (gr == GLOB_NOSPACE)
		error_ସନ୍ଦେଶ_and_die("out of memory during glob");
	ଯଦି (gr != 0) { /* GLOB_ABORTED ? */
		error_ସନ୍ଦେଶ("glob(3) error %d",gr);
	}
	/* globprint(glob_target); */
	ଫେରନ୍ତୁ gr;
}
#endif

#ifdef __U_BOOT__
ସ୍ଥିର ଅକ୍ଷର *get_dollar_var(ଅକ୍ଷର ch);
#endif

/* This is used to get/check local shell variables */
ଅକ୍ଷର *get_local_var(ଅଚଳ ଅକ୍ଷର *s)
{
	ସଂରଚନା variables *cur;

	ଯଦି (!s)
		ଫେରନ୍ତୁ ଖାଲି;

#ifdef __U_BOOT__
	ଯଦି (*s == '$')
		ଫେରନ୍ତୁ get_dollar_var(s[1]);
#endif

	ପାଇଁ (cur = ଶୀର୍ଷ_vars; cur; cur=cur->next)
		ଯଦି(strcmp(cur->name, s)==0)
			ଫେରନ୍ତୁ cur->ମୂଲ୍ୟ;
	ଫେରନ୍ତୁ ଖାଲି;
}

/* This is used to set local shell variables
   flg_export==0 if only local (not exporting) variable
   flg_export==1 if "new" exporting environ
   flg_export>1  if current startup environ (not call putenv()) */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା set_local_var(ଅଚଳ ଅକ୍ଷର *s, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flg_export)
{
	ଅକ୍ଷର *name, *ମୂଲ୍ୟ;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା result=0;
	ସଂରଚନା variables *cur;

#ifdef __U_BOOT__
	/* might be possible! */
	ଯଦି (!isalpha(*s))
		ଫେରନ୍ତୁ -1;
#endif

	name=strdup(s);

	/* Assume when we enter this function that we are already in
	 * NAME=VALUE format.  So the first order of business is to
	 * split 's' on the '=' into 'name' and 'value' */
	ମୂଲ୍ୟ = strchr(name, '=');
	ଯଦି (!ମୂଲ୍ୟ) {
		ମୁକ୍ତ(name);
		ଫେରନ୍ତୁ -1;
	}
	*ମୂଲ୍ୟ++ = 0;

	ଯଦି (!*ମୂଲ୍ୟ) {
		unset_local_var(name);
		ମୁକ୍ତ(name);
		ଫେରନ୍ତୁ 0;
	}

	ପାଇଁ(cur = ଶୀର୍ଷ_vars; cur; cur = cur->next) {
		ଯଦି(strcmp(cur->name, name)==0)
			ଭାଙ୍ଗ;
	}

	ଯଦି(cur) {
		ଯଦି(strcmp(cur->ମୂଲ୍ୟ, ମୂଲ୍ୟ)==0) {
			ଯଦି(flg_export>0 && cur->flg_export==0)
				cur->flg_export=flg_export;
			ଅନ୍ୟଥା
				result++;
		} ଅନ୍ୟଥା {
			ଯଦି(cur->flg_read_only) {
				error_ସନ୍ଦେଶ("%s: readonly variable", name);
				result = -1;
			} ଅନ୍ୟଥା {
				ଯଦି(flg_export>0 || cur->flg_export>1)
					cur->flg_export=1;
				ମୁକ୍ତ(cur->ମୂଲ୍ୟ);

				cur->ମୂଲ୍ୟ = strdup(ମୂଲ୍ୟ);
			}
		}
	} ଅନ୍ୟଥା {
		cur = malloc(ଆକାର(ସଂରଚନା variables));
		ଯଦି(!cur) {
			result = -1;
		} ଅନ୍ୟଥା {
			cur->name = strdup(name);
			ଯଦି (cur->name == ଖାଲି) {
				ମୁକ୍ତ(cur);
				result = -1;
			} ଅନ୍ୟଥା {
				ସଂରଚନା variables *bottom = ଶୀର୍ଷ_vars;
				cur->ମୂଲ୍ୟ = strdup(ମୂଲ୍ୟ);
				cur->next = ଖାଲି;
				cur->flg_export = flg_export;
				cur->flg_read_only = 0;
				ଯେତେବେଳେ(bottom->next) bottom=bottom->next;
				bottom->next = cur;
			}
		}
	}

#ifndef __U_BOOT__
	ଯଦି(result==0 && cur->flg_export==1) {
		*(ମୂଲ୍ୟ-1) = '=';
		result = putenv(name);
	} ଅନ୍ୟଥା {
#endif
		ମୁକ୍ତ(name);
#ifndef __U_BOOT__
		ଯଦି(result>0)            /* equivalent to previous set */
			result = 0;
	}
#endif
	ଫେରନ୍ତୁ result;
}

ଶୂନ୍ୟ unset_local_var(ଅଚଳ ଅକ୍ଷର *name)
{
	ସଂରଚନା variables *cur;

	ଯଦି (name) {
		ପାଇଁ (cur = ଶୀର୍ଷ_vars; cur; cur=cur->next) {
			ଯଦି(strcmp(cur->name, name)==0)
				ଭାଙ୍ଗ;
		}
		ଯଦି (cur != ଖାଲି) {
			ସଂରଚନା variables *next = ଶୀର୍ଷ_vars;
			ଯଦି(cur->flg_read_only) {
				error_ସନ୍ଦେଶ("%s: readonly variable", name);
				ଫେରନ୍ତୁ;
			} ଅନ୍ୟଥା {
#ifndef __U_BOOT__
				ଯଦି(cur->flg_export)
					unenv_set(cur->name);
#endif
				ମୁକ୍ତ(cur->name);
				ମୁକ୍ତ(cur->ମୂଲ୍ୟ);
				ଯେତେବେଳେ (next->next != cur)
					next = next->next;
				next->next = cur->next;
			}
			ମୁକ୍ତ(cur);
		}
	}
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା is_assignment(ଅଚଳ ଅକ୍ଷର *s)
{
	ଯଦି (s == ଖାଲି)
		ଫେରନ୍ତୁ 0;

	ଯଦି (!isalpha(*s)) ଫେରନ୍ତୁ 0;
	++s;
	ଯେତେବେଳେ(isalnum(*s) || *s=='_') ++s;
	ଫେରନ୍ତୁ *s=='=';
}

#ifndef __U_BOOT__
/* the src parameter allows us to peek forward to a possible &n syntax
 * for file descriptor duplication, e.g., "2>&1".
 * Return code is 0 normally, 1 if a syntax error is detected in src.
 * Resource errors (in xmalloc) cause the process to exit */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା setup_redirect(ସଂରଚନା p_context *ctx, ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd, redir_type style,
	ସଂରଚନା in_str *ନିବେଶ)
{
	ସଂରଚନା child_prog *child=ctx->child;
	ସଂରଚନା redir_struct *redir = child->redirects;
	ସଂରଚନା redir_struct *last_redir=ଖାଲି;

	/* Create a new redir_struct and drop it onto the end of the linked list */
	ଯେତେବେଳେ(redir) {
		last_redir=redir;
		redir=redir->next;
	}
	redir = xmalloc(ଆକାର(ସଂରଚନା redir_struct));
	redir->next=ଖାଲି;
	redir->word.gl_pathv=ଖାଲି;
	ଯଦି (last_redir) {
		last_redir->next=redir;
	} ଅନ୍ୟଥା {
		child->redirects=redir;
	}

	redir->type=style;
	redir->fd= (fd==-1) ? redir_table[style].default_fd : fd ;

	debug_printf("Redirect type %d%s\n", redir->fd, redir_table[style].descrip);

	/* Check for a '2>&1' type redirect */
	redir->dup = redirect_dup_ଅଙ୍କ(ନିବେଶ);
	ଯଦି (redir->dup == -2) ଫେରନ୍ତୁ 1;  /* syntax error */
	ଯଦି (redir->dup != -1) {
		/* Erik had a check here that the file descriptor in question
		 * is legit; I postpone that to "run time"
		 * A "-" representation of "close me" shows up as a -3 here */
		debug_printf("Duplicating redirect '%d>&%d'\n", redir->fd, redir->dup);
	} ଅନ୍ୟଥା {
		/* We do _not_ try to open the file that src points to,
		 * since we need to return and let src be expanded first.
		 * Set ctx->pending_redirect, so we know what to do at the
		 * end of the next parsed word.
		 */
		ctx->pending_redirect = redir;
	}
	ଫେରନ୍ତୁ 0;
}
#endif

ସ୍ଥିର ସଂରଚନା pipe *new_pipe(ଶୂନ୍ୟ)
{
	ସଂରଚନା pipe *pi;
	pi = xmalloc(ଆକାର(ସଂରଚନା pipe));
	pi->ଅଙ୍କ_progs = 0;
	pi->progs = ଖାଲି;
	pi->next = ଖାଲି;
	pi->followup = 0;  /* invalid */
	pi->r_mode = RES_NONE;
	ଫେରନ୍ତୁ pi;
}

ସ୍ଥିର ଶୂନ୍ୟ initialize_context(ସଂରଚନା p_context *ctx)
{
	ctx->pipe=ଖାଲି;
#ifndef __U_BOOT__
	ctx->pending_redirect=ଖାଲି;
#endif
	ctx->child=ଖାଲି;
	ctx->list_head=new_pipe();
	ctx->pipe=ctx->list_head;
	ctx->w=RES_NONE;
	ctx->ଗଦା=ଖାଲି;
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
ସଂରଚନା reserved_combo {
	ଅକ୍ଷର *literal;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା code;
	long flag;
};
/* Mostly a list of accepted follow-up reserved words.
 * FLAG_END means we are done with the sequence, and are ready
 * to turn the compound list into a command.
 * FLAG_START means the word must start a new compound list.
 */
ସ୍ଥିର ସଂରଚନା reserved_combo reserved_list[] = {
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
#define NRES (ଆକାର(reserved_list)/ଆକାର(ସଂରଚନା reserved_combo))

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା reserved_word(o_string *dest, ସଂରଚନା p_context *ctx)
{
	ସଂରଚନା reserved_combo *r;
	ପାଇଁ (r=reserved_list;
		r<reserved_list+NRES; r++) {
		ଯଦି (strcmp(dest->data, r->literal) == 0) {
			debug_printf("found reserved word %s, code %d\n",r->literal,r->code);
			ଯଦି (r->flag & FLAG_START) {
				ସଂରଚନା p_context *new = xmalloc(ଆକାର(ସଂରଚନା p_context));
				debug_printf("push stack\n");
				ଯଦି (ctx->w == RES_IN || ctx->w == RES_FOR) {
					syntax();
					ମୁକ୍ତ(new);
					ctx->w = RES_SNTX;
					b_reset(dest);
					ଫେରନ୍ତୁ 1;
				}
				*new = *ctx;   /* physical copy */
				initialize_context(ctx);
				ctx->ଗଦା=new;
			} ଅନ୍ୟଥା ଯଦି ( ctx->w == RES_NONE || ! (ctx->old_flag & (1<<r->code))) {
				syntax();
				ctx->w = RES_SNTX;
				b_reset(dest);
				ଫେରନ୍ତୁ 1;
			}
			ctx->w=r->code;
			ctx->old_flag = r->flag;
			ଯଦି (ctx->old_flag & FLAG_END) {
				ସଂରଚନା p_context *old;
				debug_printf("pop stack\n");
				done_pipe(ctx,PIPE_SEQ);
				old = ctx->ଗଦା;
				old->child->group = ctx->list_head;
#ifndef __U_BOOT__
				old->child->subshell = 0;
#endif
				*ctx = *old;   /* physical copy */
				ମୁକ୍ତ(old);
			}
			b_reset (dest);
			ଫେରନ୍ତୁ 1;
		}
	}
	ଫେରନ୍ତୁ 0;
}

/* normal return is 0.
 * Syntax or xglob errors return 1. */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା done_word(o_string *dest, ସଂରଚନା p_context *ctx)
{
	ସଂରଚନା child_prog *child=ctx->child;
#ifndef __U_BOOT__
	glob_t *glob_target;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା gr, flags = 0;
#ଅନ୍ୟଥା
	ଅକ୍ଷର *str, *s;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, cnt;
#endif

	debug_printf("done_word: %s %p\n", dest->data, child);
	ଯଦି (dest->length == 0 && !dest->nonnull) {
		debug_printf("  true null, ignored\n");
		ଫେରନ୍ତୁ 0;
	}
#ifndef __U_BOOT__
	ଯଦି (ctx->pending_redirect) {
		glob_target = &ctx->pending_redirect->word;
	} ଅନ୍ୟଥା {
#endif
		ଯଦି (child->group) {
			syntax();
			ଫେରନ୍ତୁ 1;  /* syntax error, groups and arglists don't mix */
		}
		ଯଦି (!child->argv && (ctx->type & FLAG_PARSE_SEMICOLON)) {
			debug_printf("checking %s for reserved-ness\n",dest->data);
			ଯଦି (reserved_word(dest,ctx)) ଫେରନ୍ତୁ ctx->w==RES_SNTX;
		}
#ifndef __U_BOOT__
		glob_target = &child->glob_result;
		ଯଦି (child->argv) flags |= GLOB_APPEND;
#ଅନ୍ୟଥା
		ପାଇଁ (cnt = 1, s = dest->data; s && *s; s++) {
			ଯଦି (*s == '\\') s++;
			cnt++;
		}
		str = malloc(cnt);
		ଯଦି (!str) ଫେରନ୍ତୁ 1;
		ଯଦି ( child->argv == ଖାଲି) {
			child->argc=0;
		}
		argc = ++child->argc;
		child->argv = realloc(child->argv, (argc+1)*ଆକାର(*child->argv));
		ଯଦି (child->argv == ଖାଲି) {
			ମୁକ୍ତ(str);
			ଫେରନ୍ତୁ 1;
		}
		child->argv_nonnull = realloc(child->argv_nonnull,
					(argc+1)*ଆକାର(*child->argv_nonnull));
		ଯଦି (child->argv_nonnull == ଖାଲି) {
			ମୁକ୍ତ(str);
			ଫେରନ୍ତୁ 1;
		}
		child->argv[argc-1]=str;
		child->argv_nonnull[argc-1] = dest->nonnull;
		child->argv[argc]=ଖାଲି;
		child->argv_nonnull[argc] = 0;
		ପାଇଁ (s = dest->data; s && *s; s++,str++) {
			ଯଦି (*s == '\\') s++;
			*str = *s;
		}
		*str = '\0';
#endif
#ifndef __U_BOOT__
	}
	gr = xglob(dest, flags, glob_target);
	ଯଦି (gr != 0) ଫେରନ୍ତୁ 1;
#endif

	b_reset(dest);
#ifndef __U_BOOT__
	ଯଦି (ctx->pending_redirect) {
		ctx->pending_redirect=ଖାଲି;
		ଯଦି (glob_target->gl_pathc != 1) {
			error_ସନ୍ଦେଶ("ambiguous redirect");
			ଫେରନ୍ତୁ 1;
		}
	} ଅନ୍ୟଥା {
		child->argv = glob_target->gl_pathv;
	}
#endif
	ଯଦି (ctx->w == RES_FOR) {
		done_word(dest,ctx);
		done_pipe(ctx,PIPE_SEQ);
	}
	ଫେରନ୍ତୁ 0;
}

/* The only possible error here is out of memory, in which case
 * xmalloc exits. */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା done_command(ସଂରଚନା p_context *ctx)
{
	/* The child is really already in the pipe structure, so
	 * advance the pipe counter and make a new, null child.
	 * Only real trickiness here is that the uncommitted
	 * child structure, to which ctx->child points, is not
	 * counted in pi->num_progs. */
	ସଂରଚନା pipe *pi=ctx->pipe;
	ସଂରଚନା child_prog *prog=ctx->child;

	ଯଦି (prog && prog->group == ଖାଲି
		 && prog->argv == ଖାଲି
#ifndef __U_BOOT__
		 && prog->redirects == ଖାଲି) {
#ଅନ୍ୟଥା
										) {
#endif
		debug_printf("done_command: skipping null command\n");
		ଫେରନ୍ତୁ 0;
	} ଅନ୍ୟଥା ଯଦି (prog) {
		pi->ଅଙ୍କ_progs++;
		debug_printf("done_command: num_progs incremented to %d\n",pi->ଅଙ୍କ_progs);
	} ଅନ୍ୟଥା {
		debug_printf("done_command: initializing\n");
	}
	pi->progs = xrealloc(pi->progs, ଆକାର(*pi->progs) * (pi->ଅଙ୍କ_progs+1));

	prog = pi->progs + pi->ଅଙ୍କ_progs;
#ifndef __U_BOOT__
	prog->redirects = ଖାଲି;
#endif
	prog->argv = ଖାଲି;
	prog->argv_nonnull = ଖାଲି;
#ifndef __U_BOOT__
	prog->is_stopped = 0;
#endif
	prog->group = ଖାଲି;
#ifndef __U_BOOT__
	prog->glob_result.gl_pathv = ଖାଲି;
	prog->family = pi;
#endif
	prog->sp = 0;
	ctx->child = prog;
	prog->type = ctx->type;

	/* but ctx->pipe and ctx->list_head remain unchanged */
	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା done_pipe(ସଂରଚନା p_context *ctx, pipe_style type)
{
	ସଂରଚନା pipe *new_p;
	done_command(ctx);  /* implicit closure of previous command */
	debug_printf("done_pipe, type %d\n", type);
	ctx->pipe->followup = type;
	ctx->pipe->r_mode = ctx->w;
	new_p=new_pipe();
	ctx->pipe->next = new_p;
	ctx->pipe = new_p;
	ctx->child = ଖାଲି;
	done_command(ctx);  /* set up new pipe to accept commands */
	ଫେରନ୍ତୁ 0;
}

#ifndef __U_BOOT__
/* peek ahead in the in_str to find out if we have a "&n" construct,
 * as in "2>&1", that represents duplicating a file descriptor.
 * returns either -2 (syntax error), -1 (no &), or the number found.
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା redirect_dup_ଅଙ୍କ(ସଂରଚନା in_str *ନିବେଶ)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ch, d=0, ok=0;
	ch = b_peek(ନିବେଶ);
	ଯଦି (ch != '&') ଫେରନ୍ତୁ -1;

	b_getch(ନିବେଶ);  /* get the & */
	ch=b_peek(ନିବେଶ);
	ଯଦି (ch == '-') {
		b_getch(ନିବେଶ);
		ଫେରନ୍ତୁ -3;  /* "-" represents "close me" */
	}
	ଯେତେବେଳେ (isdigit(ch)) {
		d = d*10+(ch-'0');
		ok=1;
		b_getch(ନିବେଶ);
		ch = b_peek(ନିବେଶ);
	}
	ଯଦି (ok) ଫେରନ୍ତୁ d;

	error_ସନ୍ଦେଶ("ambiguous redirect");
	ଫେରନ୍ତୁ -2;
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା redirect_opt_ଅଙ୍କ(o_string *o)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଅଙ୍କ;

	ଯଦି (o->length==0) ଫେରନ୍ତୁ -1;
	ପାଇଁ(ଅଙ୍କ=0; ଅଙ୍କ<o->length; ଅଙ୍କ++) {
		ଯଦି (!isdigit(*(o->data+ଅଙ୍କ))) {
			ଫେରନ୍ତୁ -1;
		}
	}
	/* reuse num (and save an int) */
	ଅଙ୍କ=atoi(o->data);
	b_reset(o);
	ଫେରନ୍ତୁ ଅଙ୍କ;
}

FILE *generate_stream_from_list(ସଂରଚନା pipe *head)
{
	FILE *pf;
#ଯଦି 1
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା pid, channel[2];
	ଯଦି (pipe(channel)<0) perror_ସନ୍ଦେଶ_and_die("pipe");
	pid=fork();
	ଯଦି (pid<0) {
		perror_ସନ୍ଦେଶ_and_die("fork");
	} ଅନ୍ୟଥା ଯଦି (pid==0) {
		close(channel[0]);
		ଯଦି (channel[1] != 1) {
			dup2(channel[1],1);
			close(channel[1]);
		}
#ଯଦି 0
#define SURROGATE "surrogate response"
		ଲିଖ(1,SURROGATE,ଆକାର(SURROGATE));
		_exit(run_list(head));
#ଅନ୍ୟଥା
		_exit(run_list_real(head));   /* leaks memory */
#endif
	}
	debug_printf("forked child %d\n",pid);
	close(channel[1]);
	pf = fdopen(channel[0],"r");
	debug_printf("pipe on FILE *%p\n",pf);
#ଅନ୍ୟଥା
	free_pipe_list(head,0);
	pf=popen("echo surrogate response","r");
	debug_printf("started fake pipe on FILE *%p\n",pf);
#endif
	ଫେରନ୍ତୁ pf;
}

/* this version hacked for testing purposes */
/* return code is exit status of the process that is run. */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା process_command_subs(o_string *dest, ସଂରଚନା p_context *ctx, ସଂରଚନା in_str *ନିବେଶ, ପୂର୍ଣ୍ଣସଂଖ୍ୟା subst_end)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା retcode;
	o_string result=NULL_O_STRING;
	ସଂରଚନା p_context inner;
	FILE *p;
	ସଂରଚନା in_str pipe_str;
	initialize_context(&inner);

	/* recursion to generate command */
	retcode = parse_stream(&result, &inner, ନିବେଶ, subst_end);
	ଯଦି (retcode != 0) ଫେରନ୍ତୁ retcode;  /* syntax error or EOF */
	done_word(&result, &inner);
	done_pipe(&inner, PIPE_SEQ);
	b_free(&result);

	p=generate_stream_from_list(inner.list_head);
	ଯଦି (p==ଖାଲି) ଫେରନ୍ତୁ 1;
	mark_open(fileno(p));
	setup_file_in_str(&pipe_str, p);

	/* now send results of command back into original context */
	retcode = parse_stream(dest, ctx, &pipe_str, '\0');
	/* XXX In case of a syntax error, should we try to kill the child?
	 * That would be tough to do right, so just read until EOF. */
	ଯଦି (retcode == 1) {
		ଯେତେବେଳେ (b_getch(&pipe_str)!=EOF) { /* discard */ };
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
	ଫେରନ୍ତୁ retcode;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା parse_group(o_string *dest, ସଂରଚନା p_context *ctx,
	ସଂରଚନା in_str *ନିବେଶ, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ch)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rcode, endch=0;
	ସଂରଚନା p_context sub;
	ସଂରଚନା child_prog *child = ctx->child;
	ଯଦି (child->argv) {
		syntax();
		ଫେରନ୍ତୁ 1;  /* syntax error, groups and arglists don't mix */
	}
	initialize_context(&sub);
	ବାଛ(ch) {
		ସ୍ଥିତି '(': endch=')'; child->subshell=1; ଭାଙ୍ଗ;
		ସ୍ଥିତି '{': endch='}'; ଭାଙ୍ଗ;
		ପୂର୍ବନିର୍ଦ୍ଧାରିତ: syntax();   /* really logic error */
	}
	rcode=parse_stream(dest,&sub,ନିବେଶ,endch);
	done_word(dest,&sub); /* finish off the final word in the subcontext */
	done_pipe(&sub, PIPE_SEQ);  /* and the final command there, too */
	child->group = sub.list_head;
	ଫେରନ୍ତୁ rcode;
	/* child remains "open", available for possible redirects */
}
#endif

/* basically useful version until someone wants to get fancier,
 * see the bash man page under "Parameter Expansion" */
ସ୍ଥିର ଅକ୍ଷର *lookup_param(ଅକ୍ଷର *src)
{
	ଅକ୍ଷର *p;
	ଅକ୍ଷର *sep;
	ଅକ୍ଷର *default_val = ଖାଲି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା assign = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା expand_empty = 0;

	ଯଦି (!src)
		ଫେରନ୍ତୁ ଖାଲି;

	sep = strchr(src, ':');

	ଯଦି (sep) {
		*sep = '\0';
		ଯଦି (*(sep + 1) == '-')
			default_val = sep+2;
		ଯଦି (*(sep + 1) == '=') {
			default_val = sep+2;
			assign = 1;
		}
		ଯଦି (*(sep + 1) == '+') {
			default_val = sep+2;
			expand_empty = 1;
		}
	}

	p = env_get(src);
	ଯଦି (!p)
		p = get_local_var(src);

	ଯଦି (!p || strlen(p) == 0) {
		p = default_val;
		ଯଦି (assign) {
			ଅକ୍ଷର *var = malloc(strlen(src)+strlen(default_val)+2);
			ଯଦି (var) {
				sprintf(var, "%s=%s", src, default_val);
				set_local_var(var, 0);
			}
			ମୁକ୍ତ(var);
		}
	} ଅନ୍ୟଥା ଯଦି (expand_empty) {
		p += strlen(p);
	}

	ଯଦି (sep)
		*sep = ':';

	ଫେରନ୍ତୁ p;
}

#ifdef __U_BOOT__
ସ୍ଥିର ଅକ୍ଷର *get_dollar_var(ଅକ୍ଷର ch)
{
	ସ୍ଥିର ଅକ୍ଷର buf[40];

	buf[0] = '\0';
	ବାଛ (ch) {
		ସ୍ଥିତି '?':
			sprintf(buf, "%u", (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)last_return_code);
			ଭାଙ୍ଗ;
		ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
			ଫେରନ୍ତୁ ଖାଲି;
	}
	ଫେରନ୍ତୁ buf;
}
#endif

/* return code: 0 for OK, 1 for syntax error */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା handle_dollar(o_string *dest, ସଂରଚନା p_context *ctx, ସଂରଚନା in_str *ନିବେଶ)
{
#ifndef __U_BOOT__
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, advance=0;
#ଅନ୍ୟଥା
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା advance=0;
#endif
#ifndef __U_BOOT__
	ଅକ୍ଷର sep[]=" ";
#endif
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ch = ନିବେଶ->peek(ନିବେଶ);  /* first character after the $ */
	debug_printf("handle_dollar: ch=%c\n",ch);
	ଯଦି (isalpha(ch)) {
		b_addchr(dest, SPECIAL_VAR_ପ୍ରତୀକ);
		ctx->child->sp++;
		ଯେତେବେଳେ(ch=b_peek(ନିବେଶ),isalnum(ch) || ch=='_') {
			b_getch(ନିବେଶ);
			b_addchr(dest,ch);
		}
		b_addchr(dest, SPECIAL_VAR_ପ୍ରତୀକ);
#ifndef __U_BOOT__
	} ଅନ୍ୟଥା ଯଦି (isdigit(ch)) {
		i = ch-'0';  /* XXX is $0 special? */
		ଯଦି (i<global_argc) {
			parse_string(dest, ctx, global_argv[i]); /* recursion */
		}
		advance = 1;
#endif
	} ଅନ୍ୟଥା ବାଛ (ch) {
#ifndef __U_BOOT__
		ସ୍ଥିତି '$':
			b_adduint(dest,getpid());
			advance = 1;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି '!':
			ଯଦି (last_bg_pid > 0) b_adduint(dest, last_bg_pid);
			advance = 1;
			ଭାଙ୍ଗ;
#endif
		ସ୍ଥିତି '?':
#ifndef __U_BOOT__
			b_adduint(dest,last_return_code);
#ଅନ୍ୟଥା
			ctx->child->sp++;
			b_addchr(dest, SPECIAL_VAR_ପ୍ରତୀକ);
			b_addchr(dest, '$');
			b_addchr(dest, '?');
			b_addchr(dest, SPECIAL_VAR_ପ୍ରତୀକ);
#endif
			advance = 1;
			ଭାଙ୍ଗ;
#ifndef __U_BOOT__
		ସ୍ଥିତି '#':
			b_adduint(dest,global_argc ? global_argc-1 : 0);
			advance = 1;
			ଭାଙ୍ଗ;
#endif
		ସ୍ଥିତି '{':
			b_addchr(dest, SPECIAL_VAR_ପ୍ରତୀକ);
			ctx->child->sp++;
			b_getch(ନିବେଶ);
			/* XXX maybe someone will try to escape the '}' */
			ଯେତେବେଳେ(ch=b_getch(ନିବେଶ),ch!=EOF && ch!='}') {
				b_addchr(dest,ch);
			}
			ଯଦି (ch != '}') {
				syntax();
				ଫେରନ୍ତୁ 1;
			}
			b_addchr(dest, SPECIAL_VAR_ପ୍ରତୀକ);
			ଭାଙ୍ଗ;
#ifndef __U_BOOT__
		ସ୍ଥିତି '(':
			b_getch(ନିବେଶ);
			process_command_subs(dest, ctx, ନିବେଶ, ')');
			ଭାଙ୍ଗ;
		ସ୍ଥିତି '*':
			sep[0]=ifs[0];
			ପାଇଁ (i=1; i<global_argc; i++) {
				parse_string(dest, ctx, global_argv[i]);
				ଯଦି (i+1 < global_argc) parse_string(dest, ctx, sep);
			}
			ଭାଙ୍ଗ;
		ସ୍ଥିତି '@':
		ସ୍ଥିତି '-':
		ସ୍ଥିତି '_':
			/* still unhandled, but should be eventually */
			error_ସନ୍ଦେଶ("unhandled syntax: $%c",ch);
			ଫେରନ୍ତୁ 1;
			ଭାଙ୍ଗ;
#endif
		ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
			b_addqchr(dest,'$',dest->quote);
	}
	/* Eat the character if the flag was set.  If the compiler
	 * is smart enough, we could substitute "b_getch(input);"
	 * for all the "advance = 1;" above, and also end up with
	 * a nice size-optimized program.  Hah!  That'll be the day.
	 */
	ଯଦି (advance) b_getch(ନିବେଶ);
	ଫେରନ୍ତୁ 0;
}

#ifndef __U_BOOT__
ପୂର୍ଣ୍ଣସଂଖ୍ୟା parse_string(o_string *dest, ସଂରଚନା p_context *ctx, ଅଚଳ ଅକ୍ଷର *src)
{
	ସଂରଚନା in_str foo;
	setup_string_in_str(&foo, src);
	ଫେରନ୍ତୁ parse_stream(dest, ctx, &foo, '\0');
}
#endif

/* return code is 0 for normal exit, 1 for syntax error */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା parse_stream(o_string *dest, ସଂରଚନା p_context *ctx,
			ସଂରଚନା in_str *ନିବେଶ, ପୂର୍ଣ୍ଣସଂଖ୍ୟା end_trigger)
{
	unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା ch, m;
#ifndef __U_BOOT__
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା redir_fd;
	redir_type redir_style;
#endif
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା next;

	/* Only double-quote state is handled in the state variable dest->quote.
	 * A single-quote triggers a bypass of the main loop until its mate is
	 * found.  When recursing, quote state is passed in via dest->quote. */

	debug_printf("parse_stream, end_trigger=%d\n",end_trigger);
	ଯେତେବେଳେ ((ch=b_getch(ନିବେଶ))!=EOF) {
		m = map[ch];
#ifdef __U_BOOT__
		ଯଦି (ନିବେଶ->__promptme == 0) ଫେରନ୍ତୁ 1;
#endif
		next = (ch == '\n') ? 0 : b_peek(ନିବେଶ);

		debug_printf("parse_stream: ch=%c (%d) m=%d quote=%d - %c\n",
			ch >= ' ' ? ch : '.', ch, m,
			dest->quote, ctx->ଗଦା == ଖାଲି ? '*' : '.');

		ଯଦି (m==0 || ((m==1 || m==2) && dest->quote)) {
			b_addqchr(dest, ch, dest->quote);
		} ଅନ୍ୟଥା {
			ଯଦି (m==2) {  /* unquoted IFS */
				ଯଦି (done_word(dest, ctx)) {
					ଫେରନ୍ତୁ 1;
				}
				/* If we aren't performing a substitution, treat a newline as a
				 * command separator.  */
				ଯଦି (end_trigger != '\0' && ch=='\n')
					done_pipe(ctx,PIPE_SEQ);
			}
			ଯଦି (ch == end_trigger && !dest->quote && ctx->w==RES_NONE) {
				debug_printf("leaving parse_stream (triggered)\n");
				ଫେରନ୍ତୁ 0;
			}
#ଯଦି 0
			ଯଦି (ch=='\n') {
				/* Yahoo!  Time to run with it! */
				done_pipe(ctx,PIPE_SEQ);
				run_list(ctx->list_head);
				initialize_context(ctx);
			}
#endif
			ଯଦି (m!=2) ବାଛ (ch) {
		ସ୍ଥିତି '#':
			ଯଦି (dest->length == 0 && !dest->quote) {
				ଯେତେବେଳେ(ch=b_peek(ନିବେଶ),ch!=EOF && ch!='\n') { b_getch(ନିବେଶ); }
			} ଅନ୍ୟଥା {
				b_addqchr(dest, ch, dest->quote);
			}
			ଭାଙ୍ଗ;
		ସ୍ଥିତି '\\':
			ଯଦି (next == EOF) {
				syntax();
				ଫେରନ୍ତୁ 1;
			}
			b_addqchr(dest, '\\', dest->quote);
			b_addqchr(dest, b_getch(ନିବେଶ), dest->quote);
			ଭାଙ୍ଗ;
		ସ୍ଥିତି '$':
			ଯଦି (handle_dollar(dest, ctx, ନିବେଶ)!=0) ଫେରନ୍ତୁ 1;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି '\'':
			dest->nonnull = 1;
			ଯେତେବେଳେ(ch=b_getch(ନିବେଶ),ch!=EOF && ch!='\'') {
#ifdef __U_BOOT__
				ଯଦି(ନିବେଶ->__promptme == 0) ଫେରନ୍ତୁ 1;
#endif
				b_addchr(dest,ch);
			}
			ଯଦି (ch==EOF) {
				syntax();
				ଫେରନ୍ତୁ 1;
			}
			ଭାଙ୍ଗ;
		ସ୍ଥିତି '"':
			dest->nonnull = 1;
			dest->quote = !dest->quote;
			ଭାଙ୍ଗ;
#ifndef __U_BOOT__
		ସ୍ଥିତି '`':
			process_command_subs(dest, ctx, ନିବେଶ, '`');
			ଭାଙ୍ଗ;
		ସ୍ଥିତି '>':
			redir_fd = redirect_opt_ଅଙ୍କ(dest);
			done_word(dest, ctx);
			redir_style=REDIRECT_OVERWRITE;
			ଯଦି (next == '>') {
				redir_style=REDIRECT_APPEND;
				b_getch(ନିବେଶ);
			} ଅନ୍ୟଥା ଯଦି (next == '(') {
				syntax();   /* until we support >(list) Process Substitution */
				ଫେରନ୍ତୁ 1;
			}
			setup_redirect(ctx, redir_fd, redir_style, ନିବେଶ);
			ଭାଙ୍ଗ;
		ସ୍ଥିତି '<':
			redir_fd = redirect_opt_ଅଙ୍କ(dest);
			done_word(dest, ctx);
			redir_style=REDIRECT_ନିବେଶ;
			ଯଦି (next == '<') {
				redir_style=REDIRECT_HEREIS;
				b_getch(ନିବେଶ);
			} ଅନ୍ୟଥା ଯଦି (next == '>') {
				redir_style=REDIRECT_IO;
				b_getch(ନିବେଶ);
			} ଅନ୍ୟଥା ଯଦି (next == '(') {
				syntax();   /* until we support <(list) Process Substitution */
				ଫେରନ୍ତୁ 1;
			}
			setup_redirect(ctx, redir_fd, redir_style, ନିବେଶ);
			ଭାଙ୍ଗ;
#endif
		ସ୍ଥିତି ';':
			done_word(dest, ctx);
			done_pipe(ctx,PIPE_SEQ);
			ଭାଙ୍ଗ;
		ସ୍ଥିତି '&':
			done_word(dest, ctx);
			ଯଦି (next=='&') {
				b_getch(ନିବେଶ);
				done_pipe(ctx,PIPE_AND);
			} ଅନ୍ୟଥା {
#ifndef __U_BOOT__
				done_pipe(ctx,PIPE_BG);
#ଅନ୍ୟଥା
				syntax_err();
				ଫେରନ୍ତୁ 1;
#endif
			}
			ଭାଙ୍ଗ;
		ସ୍ଥିତି '|':
			done_word(dest, ctx);
			ଯଦି (next=='|') {
				b_getch(ନିବେଶ);
				done_pipe(ctx,PIPE_OR);
			} ଅନ୍ୟଥା {
				/* we could pick up a file descriptor choice here
				 * with redirect_opt_num(), but bash doesn't do it.
				 * "echo foo 2| cat" yields "foo 2". */
#ifndef __U_BOOT__
				done_command(ctx);
#ଅନ୍ୟଥା
				syntax_err();
				ଫେରନ୍ତୁ 1;
#endif
			}
			ଭାଙ୍ଗ;
#ifndef __U_BOOT__
		ସ୍ଥିତି '(':
		ସ୍ଥିତି '{':
			ଯଦି (parse_group(dest, ctx, ନିବେଶ, ch)!=0) ଫେରନ୍ତୁ 1;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି ')':
		ସ୍ଥିତି '}':
			syntax();   /* Proper use of this character caught by end_trigger */
			ଫେରନ୍ତୁ 1;
			ଭାଙ୍ଗ;
#endif
		ସ୍ଥିତି SUBSTED_VAR_ପ୍ରତୀକ:
			dest->nonnull = 1;
			ଯେତେବେଳେ (ch = b_getch(ନିବେଶ), ch != EOF &&
			    ch != SUBSTED_VAR_ପ୍ରତୀକ) {
				debug_printf("subst, pass=%d\n", ch);
				ଯଦି (ନିବେଶ->__promptme == 0)
					ଫେରନ୍ତୁ 1;
				b_addchr(dest, ch);
			}
			debug_printf("subst, term=%d\n", ch);
			ଯଦି (ch == EOF) {
				syntax();
				ଫେରନ୍ତୁ 1;
			}
			ଭାଙ୍ଗ;
		ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
			syntax();   /* this is really an internal logic error */
			ଫେରନ୍ତୁ 1;
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
	ଯଦି (end_trigger != '\0') ଫେରନ୍ତୁ -1;
	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ଶୂନ୍ୟ mapset(ଅଚଳ unsigned ଅକ୍ଷର *set, ପୂର୍ଣ୍ଣସଂଖ୍ୟା code)
{
	ଅଚଳ unsigned ଅକ୍ଷର *s;
	ପାଇଁ (s=set; *s; s++) map[*s] = code;
}

ସ୍ଥିର ଶୂନ୍ୟ update_ifs_map(ଶୂନ୍ୟ)
{
	/* char *ifs and char map[256] are both globals. */
	ifs = (uchar *)env_get("IFS");
	ଯଦି (ifs == ଖାଲି) ifs=(uchar *)" \t\n";
	/* Precompute a list of 'flow through' behavior so it can be treated
	 * quickly up front.  Computation is necessary because of IFS.
	 * Special case handling of IFS == " \t\n" is not implemented.
	 * The map[] array only really needs two bits each, and on most machines
	 * that would be faster because of the reduced L1 cache footprint.
	 */
	memset(map,0,ଆକାର(map)); /* most characters flow through always */
#ifndef __U_BOOT__
	mapset((uchar *)"\\$'\"`", 3);      /* never flow through */
	mapset((uchar *)"<>;&|(){}#", 1);   /* flow through if quoted */
#ଅନ୍ୟଥା
	{
		uchar subst[2] = {SUBSTED_VAR_ପ୍ରତୀକ, 0};
		mapset(subst, 3);       /* never flow through */
	}
	mapset((uchar *)"\\$'\"", 3);       /* never flow through */
	mapset((uchar *)";&|#", 1);         /* flow through if quoted */
#endif
	mapset(ifs, 2);            /* also flow through if quoted */
}

/* most recursion does not come through here, the exeception is
 * from builtin_source() */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା parse_stream_outer(ସଂରଚନା in_str *inp, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flag)
{

	ସଂରଚନା p_context ctx;
	o_string temp=NULL_O_STRING;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rcode;
#ifdef __U_BOOT__
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା code = 1;
#endif
	କର {
		ctx.type = flag;
		initialize_context(&ctx);
		update_ifs_map();
		ଯଦି (!(flag & FLAG_PARSE_SEMICOLON) || (flag & FLAG_REPARSING)) mapset((uchar *)";$&|", 0);
		inp->promptmode=1;
		rcode = parse_stream(&temp, &ctx, inp,
				     flag & FLAG_CONT_ON_NEWLINE ? -1 : '\n');
#ifdef __U_BOOT__
		ଯଦି (rcode == 1) flag_repeat = 0;
#endif
		ଯଦି (rcode != 1 && ctx.old_flag != 0) {
			syntax();
#ifdef __U_BOOT__
			flag_repeat = 0;
#endif
		}
		ଯଦି (rcode != 1 && ctx.old_flag == 0) {
			done_word(&temp, &ctx);
			done_pipe(&ctx,PIPE_SEQ);
#ifndef __U_BOOT__
			run_list(ctx.list_head);
#ଅନ୍ୟଥା
			code = run_list(ctx.list_head);
			ଯଦି (code == -2) {	/* exit */
				b_free(&temp);
				code = 0;
				/* XXX hackish way to not allow exit from main loop */
				ଯଦି (inp->peek == file_peek) {
					printf("exit not allowed from main input shell.\n");
					ଜାରି;
				}
				/*
				 * DANGER
				 * Return code -2 is special in this context,
				 * it indicates exit from inner pipe instead
				 * of return code itself, the return code is
				 * stored in 'last_return_code' variable!
				 * DANGER
				 */
				ଫେରନ୍ତୁ -2;
			}
			ଯଦି (code == -1)
			    flag_repeat = 0;
#endif
		} ଅନ୍ୟଥା {
			ଯଦି (ctx.old_flag != 0) {
				ମୁକ୍ତ(ctx.ଗଦା);
				b_reset(&temp);
			}
#ifdef __U_BOOT__
			ଯଦି (inp->__promptme == 0) printf("<INTERRUPT>\n");
			inp->__promptme = 1;
#endif
			temp.nonnull = 0;
			temp.quote = 0;
			inp->p = ଖାଲି;
			free_pipe_list(ctx.list_head,0);
		}
		b_free(&temp);
	/* loop on syntax errors, return on EOF */
	} ଯେତେବେଳେ (rcode != -1 && !(flag & FLAG_EXIT_FROM_LOOP) &&
		(inp->peek != static_peek || b_peek(inp)));
#ifndef __U_BOOT__
	ଫେରନ୍ତୁ 0;
#ଅନ୍ୟଥା
	ଫେରନ୍ତୁ (code != 0) ? 1 : 0;
#endif /* __U_BOOT__ */
}

#ifndef __U_BOOT__
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା parse_string_outer(ଅଚଳ ଅକ୍ଷର *s, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flag)
#ଅନ୍ୟଥା
ପୂର୍ଣ୍ଣସଂଖ୍ୟା parse_string_outer(ଅଚଳ ଅକ୍ଷର *s, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flag)
#endif	/* __U_BOOT__ */
{
	ସଂରଚନା in_str ନିବେଶ;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rcode;
#ifdef __U_BOOT__
	ଅକ୍ଷର *p = ଖାଲି;
	ଯଦି (!s)
		ଫେରନ୍ତୁ 1;
	ଯଦି (!*s)
		ଫେରନ୍ତୁ 0;
	ଯଦି (!(p = strchr(s, '\n')) || *++p) {
		p = xmalloc(strlen(s) + 2);
		strcpy(p, s);
		strcat(p, "\n");
		setup_string_in_str(&ନିବେଶ, p);
		rcode = parse_stream_outer(&ନିବେଶ, flag);
		ମୁକ୍ତ(p);
		ଫେରନ୍ତୁ rcode == -2 ? last_return_code : rcode;
	} ଅନ୍ୟଥା {
#endif
	setup_string_in_str(&ନିବେଶ, s);
	rcode = parse_stream_outer(&ନିବେଶ, flag);
	ଫେରନ୍ତୁ rcode == -2 ? last_return_code : rcode;
#ifdef __U_BOOT__
	}
#endif
}

#ifndef __U_BOOT__
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା parse_file_outer(FILE *f)
#ଅନ୍ୟଥା
ପୂର୍ଣ୍ଣସଂଖ୍ୟା parse_file_outer(ଶୂନ୍ୟ)
#endif
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rcode;
	ସଂରଚନା in_str ନିବେଶ;
#ifndef __U_BOOT__
	setup_file_in_str(&ନିବେଶ, f);
#ଅନ୍ୟଥା
	setup_file_in_str(&ନିବେଶ);
#endif
	rcode = parse_stream_outer(&ନିବେଶ, FLAG_PARSE_SEMICOLON);
	ଫେରନ୍ତୁ rcode == -2 ? last_return_code : rcode;
}

#ifdef __U_BOOT__
ପୂର୍ଣ୍ଣସଂଖ୍ୟା u_boot_hush_start(ଶୂନ୍ୟ)
{
	ଯଦି (ଶୀର୍ଷ_vars == ଖାଲି) {
		ଶୀର୍ଷ_vars = malloc(ଆକାର(ସଂରଚନା variables));
		ଶୀର୍ଷ_vars->name = "HUSH_VERSION";
		ଶୀର୍ଷ_vars->ମୂଲ୍ୟ = "0.01";
		ଶୀର୍ଷ_vars->next = ଖାଲି;
		ଶୀର୍ଷ_vars->flg_export = 0;
		ଶୀର୍ଷ_vars->flg_read_only = 1;
	}
	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ଶୂନ୍ୟ *xmalloc(size_t size)
{
	ଶୂନ୍ୟ *p = ଖାଲି;

	ଯଦି (!(p = malloc(size))) {
	    printf("ERROR : xmalloc failed\n");
	    ପାଇଁ(;;);
	}
	ଫେରନ୍ତୁ p;
}

ସ୍ଥିର ଶୂନ୍ୟ *xrealloc(ଶୂନ୍ୟ *ptr, size_t size)
{
	ଶୂନ୍ୟ *p = ଖାଲି;

	ଯଦି (!(p = realloc(ptr, size))) {
	    printf("ERROR : xrealloc failed\n");
	    ପାଇଁ(;;);
	}
	ଫେରନ୍ତୁ p;
}
#endif /* __U_BOOT__ */

#ifndef __U_BOOT__
/* Make sure we have a controlling tty.  If we get started under a job
 * aware app (like bash for example), make sure we are now in charge so
 * we don't fight over who gets the foreground */
ସ୍ଥିର ଶୂନ୍ୟ setup_job_control(ଶୂନ୍ୟ)
{
	ସ୍ଥିର pid_t shell_pgrp;
	/* Loop until we are in the foreground.  */
	ଯେତେବେଳେ (tcgetpgrp (shell_terminal) != (shell_pgrp = getpgrp ()))
		kill (- shell_pgrp, SIGTTIN);

	/* Ignore interactive and job-control signals.  */
	ସଙ୍କେତ(SIGINT, SIG_IGN);
	ସଙ୍କେତ(SIGQUIT, SIG_IGN);
	ସଙ୍କେତ(SIGTERM, SIG_IGN);
	ସଙ୍କେତ(SIGTSTP, SIG_IGN);
	ସଙ୍କେତ(SIGTTIN, SIG_IGN);
	ସଙ୍କେତ(SIGTTOU, SIG_IGN);
	ସଙ୍କେତ(SIGCHLD, SIG_IGN);

	/* Put ourselves in our own process group.  */
	setsid();
	shell_pgrp = getpid ();
	setpgid (shell_pgrp, shell_pgrp);

	/* Grab control of the terminal.  */
	tcsetpgrp(shell_terminal, shell_pgrp);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା hush_main(ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର * ଅଚଳ *argv)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା opt;
	FILE *ନିବେଶ;
	ଅକ୍ଷର **e = environ;

	/* XXX what should these be while sourcing /etc/profile? */
	global_argc = argc;
	global_argv = argv;

	/* (re?) initialize globals.  Sometimes hush_main() ends up calling
	 * hush_main(), therefore we cannot rely on the BSS to zero out this
	 * stuff.  Reset these to 0 every time. */
	ifs = ଖାଲି;
	/* map[] is taken care of with call to update_ifs_map() */
	fake_mode = 0;
	interactive = 0;
	close_me_head = ଖାଲି;
	last_bg_pid = 0;
	job_list = ଖାଲି;
	last_jobid = 0;

	/* Initialize some more globals to non-zero values */
	set_cwd();
#ifdef CONFIG_FEATURE_COMMAND_EDITING
	cmdedit_set_initial_prompt();
#ଅନ୍ୟଥା
	PS1 = ଖାଲି;
#endif
	PS2 = "> ";

	/* initialize our shell local variables with the values
	 * currently living in the environment */
	ଯଦି (e) {
		ପାଇଁ (; *e; e++)
			set_local_var(*e, 2);   /* without call putenv() */
	}

	last_return_code=EXIT_SUCCESS;

	ଯଦି (argv[0] && argv[0][0] == '-') {
		debug_printf("\nsourcing /etc/profile\n");
		ଯଦି ((ନିବେଶ = fopen("/etc/profile", "r")) != ଖାଲି) {
			mark_open(fileno(ନିବେଶ));
			parse_file_outer(ନିବେଶ);
			mark_closed(fileno(ନିବେଶ));
			fclose(ନିବେଶ);
		}
	}
	ନିବେଶ=stdin;

	ଯେତେବେଳେ ((opt = getopt(argc, argv, "c:xif")) > 0) {
		ବାଛ (opt) {
			ସ୍ଥିତି 'c':
				{
					global_argv = argv+optind;
					global_argc = argc-optind;
					opt = parse_string_outer(optarg, FLAG_PARSE_SEMICOLON);
					ଯାଅ final_return;
				}
				ଭାଙ୍ଗ;
			ସ୍ଥିତି 'i':
				interactive++;
				ଭାଙ୍ଗ;
			ସ୍ଥିତି 'f':
				fake_mode++;
				ଭାଙ୍ଗ;
			ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
#ifndef BB_VER
				fprintf(stderr, "Usage: sh [FILE]...\n"
						"   or: sh -c command [args]...\n\n");
				exit(EXIT_FAILURE);
#ଅନ୍ୟଥା
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
	ଯଦି (argv[optind]==ଖାଲି && ନିବେଶ==stdin &&
			isatty(fileno(stdin)) && isatty(fileno(stdout))) {
		interactive++;
	}

	debug_printf("\ninteractive=%d\n", interactive);
	ଯଦି (interactive) {
		/* Looks like they want an interactive shell */
#ifndef CONFIG_FEATURE_SH_EXTRA_QUIET
		printf( "\n\n" BB_BANNER " hush - the humble shell v0.01 (testing)\n");
		printf( "Enter 'help' for a list of built-in commands.\n\n");
#endif
		setup_job_control();
	}

	ଯଦି (argv[optind]==ଖାଲି) {
		opt=parse_file_outer(stdin);
		ଯାଅ final_return;
	}

	debug_printf("\nrunning script '%s'\n", argv[optind]);
	global_argv = argv+optind;
	global_argc = argc-optind;
	ନିବେଶ = xfopen(argv[optind], "r");
	opt = parse_file_outer(ନିବେଶ);

#ifdef CONFIG_FEATURE_CLEAN_UP
	fclose(ନିବେଶ);
	ଯଦି (cwd && cwd != unknown)
		ମୁକ୍ତ((ଅକ୍ଷର*)cwd);
	{
		ସଂରଚନା variables *cur, *tmp;
		ପାଇଁ(cur = ଶୀର୍ଷ_vars; cur; cur = tmp) {
			tmp = cur->next;
			ଯଦି (!cur->flg_read_only) {
				ମୁକ୍ତ(cur->name);
				ମୁକ୍ତ(cur->ମୂଲ୍ୟ);
				ମୁକ୍ତ(cur);
			}
		}
	}
#endif

final_return:
	ଫେରନ୍ତୁ(opt?opt:last_return_code);
}
#endif

ସ୍ଥିର ଅକ୍ଷର *ଭର୍ତ୍ତି_var_ମୂଲ୍ୟ(ଅକ୍ଷର *inp)
{
	ଫେରନ୍ତୁ ଭର୍ତ୍ତି_var_ମୂଲ୍ୟ_sub(inp, 0);
}

ସ୍ଥିର ଅକ୍ଷର *ଭର୍ତ୍ତି_var_ମୂଲ୍ୟ_sub(ଅକ୍ଷର *inp, ପୂର୍ଣ୍ଣସଂଖ୍ୟା tag_subst)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା res_str_len = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା len;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା done = 0;
	ଅକ୍ଷର *p, *p1, *res_str = ଖାଲି;

	ଯେତେବେଳେ ((p = strchr(inp, SPECIAL_VAR_ପ୍ରତୀକ))) {
		/* check the beginning of the string for normal characters */
		ଯଦି (p != inp) {
			/* copy any characters to the result string */
			len = p - inp;
			res_str = xrealloc(res_str, (res_str_len + len));
			strncpy((res_str + res_str_len), inp, len);
			res_str_len += len;
		}
		inp = ++p;
		/* find the ending marker */
		p = strchr(inp, SPECIAL_VAR_ପ୍ରତୀକ);
		*p = '\0';
		/* look up the value to substitute */
		ଯଦି ((p1 = lookup_param(inp))) {
			ଯଦି (tag_subst)
				len = res_str_len + strlen(p1) + 2;
			ଅନ୍ୟଥା
				len = res_str_len + strlen(p1);
			res_str = xrealloc(res_str, (1 + len));
			ଯଦି (tag_subst) {
				/*
				 * copy the variable value to the result
				 * string
				 */
				strcpy((res_str + res_str_len + 1), p1);

				/*
				 * mark the replaced text to be accepted as
				 * is
				 */
				res_str[res_str_len] = SUBSTED_VAR_ପ୍ରତୀକ;
				res_str[res_str_len + 1 + strlen(p1)] =
					SUBSTED_VAR_ପ୍ରତୀକ;
			} ଅନ୍ୟଥା
				/*
				 * copy the variable value to the result
				 * string
				 */
				strcpy((res_str + res_str_len), p1);

			res_str_len = len;
		}
		*p = SPECIAL_VAR_ପ୍ରତୀକ;
		inp = ++p;
		done = 1;
	}
	ଯଦି (done) {
		res_str = xrealloc(res_str, (1 + res_str_len + strlen(inp)));
		strcpy((res_str + res_str_len), inp);
		ଯେତେବେଳେ ((p = strchr(res_str, '\n'))) {
			*p = ' ';
		}
	}
	ଫେରନ୍ତୁ (res_str == ଖାଲି) ? inp : res_str;
}

ସ୍ଥିର ଅକ୍ଷର **make_list_in(ଅକ୍ଷର **inp, ଅକ୍ଷର *name)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା len, i;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା name_len = strlen(name);
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = 0;
	ଅକ୍ଷର **list;
	ଅକ୍ଷର *p1, *p2, *p3;

	/* create list of variable values */
	list = xmalloc(ଆକାର(*list));
	ପାଇଁ (i = 0; inp[i]; i++) {
		p3 = ଭର୍ତ୍ତି_var_ମୂଲ୍ୟ(inp[i]);
		p1 = p3;
		ଯେତେବେଳେ (*p1) {
			ଯଦି (*p1 == ' ') {
				p1++;
				ଜାରି;
			}
			ଯଦି ((p2 = strchr(p1, ' '))) {
				len = p2 - p1;
			} ଅନ୍ୟଥା {
				len = strlen(p1);
				p2 = p1 + len;
			}
			/* we use n + 2 in realloc for list,because we add
			 * new element and then we will add NULL element */
			list = xrealloc(list, ଆକାର(*list) * (n + 2));
			list[n] = xmalloc(2 + name_len + len);
			strcpy(list[n], name);
			strcat(list[n], "=");
			strncat(list[n], p1, len);
			list[n++][name_len + len + 1] = '\0';
			p1 = p2;
		}
		ଯଦି (p3 != inp[i]) ମୁକ୍ତ(p3);
	}
	list[n] = ଖାଲି;
	ଫେରନ୍ତୁ list;
}

/*
 * Make new string for parser
 * inp     - array of argument strings to flatten
 * nonnull - indicates argument was quoted when originally parsed
 */
ସ୍ଥିର ଅକ୍ଷର *make_string(ଅକ୍ଷର **inp, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *nonnull)
{
	ଅକ୍ଷର *p;
	ଅକ୍ଷର *str = ଖାଲି;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା n;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା len = 2;
	ଅକ୍ଷର *noeval_str;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା noeval = 0;

	noeval_str = get_local_var("HUSH_NO_EVAL");
	ଯଦି (noeval_str != ଖାଲି && *noeval_str != '0' && *noeval_str != '\0')
		noeval = 1;
	ପାଇଁ (n = 0; inp[n]; n++) {
		p = ଭର୍ତ୍ତି_var_ମୂଲ୍ୟ_sub(inp[n], noeval);
		ଅକ୍ଷର *new_str = xrealloc(str, (len + strlen(p) + (2 * nonnull[n])));
		ଯଦି (!new_str) {
			ମୁକ୍ତ(str);
			ଯଦି (p != inp[n]) ମୁକ୍ତ(p);
			ଫେରନ୍ତୁ ଖାଲି;
		}
		str = new_str;
		ଯଦି (n) {
			strcat(str, " ");
		} ଅନ୍ୟଥା {
			*str = '\0';
		}
		ଯଦି (nonnull[n])
			strcat(str, "'");
		strcat(str, p);
		ଯଦି (nonnull[n])
			strcat(str, "'");
		len = strlen(str) + 3;
		ଯଦି (p != inp[n]) ମୁକ୍ତ(p);
	}
	len = strlen(str);
	*(str + len) = '\n';
	*(str + len + 1) = '\0';
	ଫେରନ୍ତୁ str;
}

#ifdef __U_BOOT__
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା do_showvar(ସଂରଚନା cmd_tbl *cmdtp, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flag, ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc,
		      ଅକ୍ଷର *ଅଚଳ argv[])
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, k;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rcode = 0;
	ସଂରଚନା variables *cur;

	ଯଦି (argc == 1) {		/* Print all env variables	*/
		ପାଇଁ (cur = ଶୀର୍ଷ_vars; cur; cur = cur->next) {
			printf ("%s=%s\n", cur->name, cur->ମୂଲ୍ୟ);
			ଯଦି (ctrlc ()) {
				puts ("\n ** Abort\n");
				ଫେରନ୍ତୁ 1;
			}
		}
		ଫେରନ୍ତୁ 0;
	}
	ପାଇଁ (i = 1; i < argc; ++i) {	/* print single env variables	*/
		ଅକ୍ଷର *name = argv[i];

		k = -1;
		ପାଇଁ (cur = ଶୀର୍ଷ_vars; cur; cur = cur->next) {
			ଯଦି(strcmp (cur->name, name) == 0) {
				k = 0;
				printf ("%s=%s\n", cur->name, cur->ମୂଲ୍ୟ);
			}
			ଯଦି (ctrlc ()) {
				puts ("\n ** Abort\n");
				ଫେରନ୍ତୁ 1;
			}
		}
		ଯଦି (k < 0) {
			printf ("## Error: \"%s\" not defined\n", name);
			rcode ++;
		}
	}
	ଫେରନ୍ତୁ rcode;
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
