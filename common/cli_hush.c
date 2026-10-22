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
#அந்தர்பாவ <env.h>
#அந்தர்பாவ <malloc.h>         /* malloc, free, realloc*/
#அந்தர்பாவ <linux/ctype.h>    /* isalpha, isdigit */
#அந்தர்பாவ <console.h>
#அந்தர்பாவ <bootretry.h>
#அந்தர்பாவ <cli.h>
#அந்தர்பாவ <cli_hush.h>
#அந்தர்பாவ <command.h>        /* find_cmd */
#அந்தர்பாவ <vsprintf.h>
#endif
#ifndef __U_BOOT__
#அந்தர்பாவ <ctype.h>     /* isalpha, isdigit */
#அந்தர்பாவ <unistd.h>    /* getpid */
#அந்தர்பாவ <stdlib.h>    /* getenv, atoi */
#அந்தர்பாவ <string.h>    /* strchr */
#அந்தர்பாவ <stdio.h>     /* popen etc. */
#அந்தர்பாவ <glob.h>      /* glob, of course */
#அந்தர்பாவ <stdarg.h>    /* va_list */
#அந்தர்பாவ <errno.h>
#அந்தர்பாவ <fcntl.h>
#அந்தர்பாவ <getopt.h>    /* should be pretty obvious */

#அந்தர்பாவ <sys/stat.h>  /* ulimit */
#அந்தர்பாவ <sys/types.h>
#அந்தர்பாவ <sys/wait.h>
#அந்தர்பாவ <signal.h>

/* #include <dmalloc.h> */

#எனில் 1
#அந்தர்பாவ "busybox.h"
#அந்தர்பாவ "cmdedit.h"
#இல்லையெனில்
#define applet_name "hush"
#அந்தர்பாவ "standalone.h"
#define hush_main முதன்மை
#define BB_BANNER
#endif
#endif
#define SPECIAL_VAR_குறியீடு 03
#define SUBSTED_VAR_குறியீடு 04
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
#define error_செய்தி printf
#இல்லையெனில்
வகைவரை எண்ணிக்கை {
	REDIRECT_உள்ளீடு     = 1,
	REDIRECT_OVERWRITE = 2,
	REDIRECT_APPEND    = 3,
	REDIRECT_HEREIS    = 4,
	REDIRECT_IO        = 5
} redir_type;

/* The descrip member of this structure is only used to make debugging
 * output pretty */
கட்டமைப்பு {முழு mode; முழு default_fd; எழுத்து *descrip;} redir_table[] = {
	{ 0,                         0, "()" },
	{ O_RDONLY,                  0, "<"  },
	{ O_CREAT|O_TRUNC|O_WRONLY,  1, ">"  },
	{ O_CREAT|O_APPEND|O_WRONLY, 1, ">>" },
	{ O_RDONLY,                 -1, "<<" },
	{ O_RDWR,                    1, "<>" }
};
#endif

வகைவரை எண்ணிக்கை {
	PIPE_SEQ = 1,
	PIPE_AND = 2,
	PIPE_OR  = 3,
	PIPE_BG  = 4,
} pipe_style;

/* might eventually control execution */
வகைவரை எண்ணிக்கை {
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
கட்டமைப்பு p_context {
	கட்டமைப்பு child_prog *child;
	கட்டமைப்பு pipe *list_head;
	கட்டமைப்பு pipe *pipe;
#ifndef __U_BOOT__
	கட்டமைப்பு redir_struct *pending_redirect;
#endif
	reserved_style w;
	முழு old_flag;				/* for figuring out valid reserved words */
	கட்டமைப்பு p_context *அடுக்கு;
	முழு type;			/* define type of parser : ";$" common or special symbol */
	/* How about quoting status? */
};

#ifndef __U_BOOT__
கட்டமைப்பு redir_struct {
	redir_type type;			/* type of redirection */
	முழு fd;						/* file descriptor being redirected */
	முழு dup;					/* -1, or file descriptor being duplicated */
	கட்டமைப்பு redir_struct *next;	/* pointer to the next redirect in the list */
	glob_t word;				/* *word.gl_pathv is the filename */
};
#endif

கட்டமைப்பு child_prog {
#ifndef __U_BOOT__
	pid_t pid;					/* 0 if exited */
#endif
	எழுத்து **argv;				/* program name and arguments */
	/* was quoted when parsed; copy of struct o_string.nonnull field */
	முழு *argv_nonnull;
#ifdef __U_BOOT__
	முழு    argc;                            /* number of program arguments */
#endif
	கட்டமைப்பு pipe *group;			/* if non-NULL, first in group or subshell */
#ifndef __U_BOOT__
	முழு subshell;				/* flag, non-zero if group must be forked */
	கட்டமைப்பு redir_struct *redirects;	/* I/O redirections */
	glob_t glob_result;			/* result of parameter globbing */
	முழு is_stopped;				/* is the program currently running? */
	கட்டமைப்பு pipe *family;		/* pointer back to the child's parent pipe */
#endif
	முழு sp;				/* number of SPECIAL_VAR_SYMBOL */
	முழு type;
};

கட்டமைப்பு pipe {
#ifndef __U_BOOT__
	முழு jobid;					/* job number */
#endif
	முழு இலக்கம்_progs;				/* total number of programs in job */
#ifndef __U_BOOT__
	முழு running_progs;			/* number of programs running */
	எழுத்து *text;					/* name of job */
	எழுத்து *cmdbuf;				/* buffer various argv's point into */
	pid_t pgrp;					/* process group ID for the job */
#endif
	கட்டமைப்பு child_prog *progs;	/* array of commands in pipe */
	கட்டமைப்பு pipe *next;			/* to track background commands */
#ifndef __U_BOOT__
	முழு stopped_progs;			/* number of programs alive, but stopped */
	முழு job_context;			/* bitmask defining current context */
#endif
	pipe_style followup;		/* PIPE_BG, PIPE_SEQ, PIPE_OR, PIPE_AND */
	reserved_style r_mode;		/* supports if, for, while, until */
};

#ifndef __U_BOOT__
கட்டமைப்பு close_me {
	முழு fd;
	கட்டமைப்பு close_me *next;
};
#endif

கட்டமைப்பு variables {
	எழுத்து *name;
	எழுத்து *மதிப்பு;
	முழு flg_export;
	முழு flg_read_only;
	கட்டமைப்பு variables *next;
};

/* globals, connect us to the outside world
 * the first three support $?, $#, and $1 */
#ifndef __U_BOOT__
எழுத்து **global_argv;
unsigned முழு global_argc;
#endif
நிலையான unsigned முழு last_return_code;
#ifndef __U_BOOT__
வெளி எழுத்து **environ; /* This is in <unistd.h>, but protected with __USE_GNU */
#endif

/* "globals" within this file */
நிலையான uchar *ifs;
நிலையான எழுத்து map[256];
#ifndef __U_BOOT__
நிலையான முழு fake_mode;
நிலையான முழு interactive;
நிலையான கட்டமைப்பு close_me *close_me_head;
நிலையான மாறிலி எழுத்து *cwd;
நிலையான கட்டமைப்பு pipe *job_list;
நிலையான unsigned முழு last_bg_pid;
நிலையான unsigned முழு last_jobid;
நிலையான unsigned முழு shell_terminal;
நிலையான எழுத்து *PS1;
நிலையான எழுத்து *PS2;
கட்டமைப்பு variables shell_ver = { "HUSH_VERSION", "0.01", 1, 1, 0 };
கட்டமைப்பு variables *உச்சி_vars = &shell_ver;
#இல்லையெனில்
நிலையான முழு flag_repeat = 0;
நிலையான முழு do_repeat = 0;
நிலையான கட்டமைப்பு variables *உச்சி_vars = வெற்று ;
#endif /*__U_BOOT__ */

#define B_CHUNK (100)
#define B_NOSPAC 1

வகைவரை கட்டமைப்பு {
	எழுத்து *data;
	முழு length;
	முழு maxlen;
	முழு quote;
	முழு nonnull;
} o_string;
#define NULL_O_STRING {வெற்று,0,0,0,0}
/* used for initialization:
	o_string foo = NULL_O_STRING; */

/* I can almost use ordinary FILE *.  Is open_memstream() universally
 * available?  Where is it documented? */
கட்டமைப்பு in_str {
	மாறிலி unsigned எழுத்து *p;
#ifndef __U_BOOT__
	எழுத்து peek_buf[2];
#endif
	முழு __promptme;
	முழு promptmode;
#ifndef __U_BOOT__
	FILE *file;
#endif
	முழு (*get) (கட்டமைப்பு in_str *);
	முழு (*peek) (கட்டமைப்பு in_str *);
};
#define b_getch(உள்ளீடு) ((உள்ளீடு)->get(உள்ளீடு))
#define b_peek(உள்ளீடு) ((உள்ளீடு)->peek(உள்ளீடு))

#ifndef __U_BOOT__
#define JOB_STATUS_FORMAT "[%d] %-22s %.40s\n"

கட்டமைப்பு built_in_command {
	எழுத்து *cmd;					/* name */
	எழுத்து *descr;				/* description */
	முழு (*function) (கட்டமைப்பு child_prog *);	/* function ptr */
};
#endif

/* define DEBUG_SHELL for debugging output (obviously ;-)) */
#எனில் 0
#define DEBUG_SHELL
#endif

/* This should be in utility.c */
#ifdef DEBUG_SHELL
#ifndef __U_BOOT__
நிலையான வெறுமை debug_printf(மாறிலி எழுத்து *format, ...)
{
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
}
#இல்லையெனில்
#define debug_printf(fmt,args...)	printf (fmt ,##args)
#endif
#இல்லையெனில்
நிலையான உள்வரி வெறுமை debug_printf(மாறிலி எழுத்து *format, ...) { }
#endif
#define final_printf debug_printf

#ifdef __U_BOOT__
நிலையான வெறுமை syntax_err(வெறுமை) {
	 printf("syntax error\n");
}
#இல்லையெனில்
நிலையான வெறுமை __syntax(எழுத்து *file, முழு வரி) {
	error_செய்தி("syntax error %s:%d", file, வரி);
}
#define syntax() __syntax(__FILE__, __LINE__)
#endif

#ifdef __U_BOOT__
நிலையான வெறுமை *xmalloc(size_t size);
நிலையான வெறுமை *xrealloc(வெறுமை *ptr, size_t size);
#இல்லையெனில்
/* Index of subroutines: */
/*   function prototypes for builtins */
நிலையான முழு builtin_cd(கட்டமைப்பு child_prog *child);
நிலையான முழு builtin_env(கட்டமைப்பு child_prog *child);
நிலையான முழு builtin_eval(கட்டமைப்பு child_prog *child);
நிலையான முழு builtin_exec(கட்டமைப்பு child_prog *child);
நிலையான முழு builtin_exit(கட்டமைப்பு child_prog *child);
நிலையான முழு builtin_export(கட்டமைப்பு child_prog *child);
நிலையான முழு builtin_fg_bg(கட்டமைப்பு child_prog *child);
நிலையான முழு builtin_help(கட்டமைப்பு child_prog *child);
நிலையான முழு builtin_jobs(கட்டமைப்பு child_prog *child);
நிலையான முழு builtin_pwd(கட்டமைப்பு child_prog *child);
நிலையான முழு builtin_read(கட்டமைப்பு child_prog *child);
நிலையான முழு builtin_set(கட்டமைப்பு child_prog *child);
நிலையான முழு builtin_shift(கட்டமைப்பு child_prog *child);
நிலையான முழு builtin_source(கட்டமைப்பு child_prog *child);
நிலையான முழு builtin_umask(கட்டமைப்பு child_prog *child);
நிலையான முழு builtin_unset(கட்டமைப்பு child_prog *child);
நிலையான முழு builtin_not_written(கட்டமைப்பு child_prog *child);
#endif
/*   o_string manipulation: */
நிலையான முழு b_check_space(o_string *o, முழு len);
நிலையான முழு b_addchr(o_string *o, முழு ch);
நிலையான வெறுமை b_reset(o_string *o);
நிலையான முழு b_addqchr(o_string *o, முழு ch, முழு quote);
#ifndef __U_BOOT__
நிலையான முழு b_adduint(o_string *o, unsigned முழு i);
#endif
/*  in_str manipulations: */
நிலையான முழு static_get(கட்டமைப்பு in_str *i);
நிலையான முழு static_peek(கட்டமைப்பு in_str *i);
நிலையான முழு file_get(கட்டமைப்பு in_str *i);
நிலையான முழு file_peek(கட்டமைப்பு in_str *i);
#ifndef __U_BOOT__
நிலையான வெறுமை setup_file_in_str(கட்டமைப்பு in_str *i, FILE *f);
#இல்லையெனில்
நிலையான வெறுமை setup_file_in_str(கட்டமைப்பு in_str *i);
#endif
நிலையான வெறுமை setup_string_in_str(கட்டமைப்பு in_str *i, மாறிலி எழுத்து *s);
#ifndef __U_BOOT__
/*  close_me manipulations: */
நிலையான வெறுமை mark_open(முழு fd);
நிலையான வெறுமை mark_closed(முழு fd);
நிலையான வெறுமை close_all(வெறுமை);
#endif
/*  "run" the final data structures: */
நிலையான எழுத்து *indenter(முழு i);
நிலையான முழு free_pipe_list(கட்டமைப்பு pipe *head, முழு indent);
நிலையான முழு free_pipe(கட்டமைப்பு pipe *pi, முழு indent);
/*  really run the final data structures: */
#ifndef __U_BOOT__
நிலையான முழு setup_redirects(கட்டமைப்பு child_prog *prog, முழு squirrel[]);
#endif
நிலையான முழு run_list_real(கட்டமைப்பு pipe *pi);
#ifndef __U_BOOT__
நிலையான வெறுமை pseudo_exec(கட்டமைப்பு child_prog *child) __attribute__ ((noreturn));
#endif
நிலையான முழு run_pipe_real(கட்டமைப்பு pipe *pi);
/*   extended glob support: */
#ifndef __U_BOOT__
நிலையான முழு globhack(மாறிலி எழுத்து *src, முழு flags, glob_t *pglob);
நிலையான முழு glob_needed(மாறிலி எழுத்து *s);
நிலையான முழு xglob(o_string *dest, முழு flags, glob_t *pglob);
#endif
/*   variable assignment: */
நிலையான முழு is_assignment(மாறிலி எழுத்து *s);
/*   data structure manipulation: */
#ifndef __U_BOOT__
நிலையான முழு setup_redirect(கட்டமைப்பு p_context *ctx, முழு fd, redir_type style, கட்டமைப்பு in_str *உள்ளீடு);
#endif
நிலையான வெறுமை initialize_context(கட்டமைப்பு p_context *ctx);
நிலையான முழு done_word(o_string *dest, கட்டமைப்பு p_context *ctx);
நிலையான முழு done_command(கட்டமைப்பு p_context *ctx);
நிலையான முழு done_pipe(கட்டமைப்பு p_context *ctx, pipe_style type);
/*   primary string parsing: */
#ifndef __U_BOOT__
நிலையான முழு redirect_dup_இலக்கம்(கட்டமைப்பு in_str *உள்ளீடு);
நிலையான முழு redirect_opt_இலக்கம்(o_string *o);
நிலையான முழு process_command_subs(o_string *dest, கட்டமைப்பு p_context *ctx, கட்டமைப்பு in_str *உள்ளீடு, முழு subst_end);
நிலையான முழு parse_group(o_string *dest, கட்டமைப்பு p_context *ctx, கட்டமைப்பு in_str *உள்ளீடு, முழு ch);
#endif
நிலையான எழுத்து *lookup_param(எழுத்து *src);
நிலையான எழுத்து *make_string(எழுத்து **inp, முழு *nonnull);
நிலையான முழு handle_dollar(o_string *dest, கட்டமைப்பு p_context *ctx, கட்டமைப்பு in_str *உள்ளீடு);
#ifndef __U_BOOT__
நிலையான முழு parse_string(o_string *dest, கட்டமைப்பு p_context *ctx, மாறிலி எழுத்து *src);
#endif
நிலையான முழு parse_stream(o_string *dest, கட்டமைப்பு p_context *ctx, கட்டமைப்பு in_str *input0, முழு end_trigger);
/*   setup: */
நிலையான முழு parse_stream_outer(கட்டமைப்பு in_str *inp, முழு flag);
#ifndef __U_BOOT__
நிலையான முழு parse_string_outer(மாறிலி எழுத்து *s, முழு flag);
நிலையான முழு parse_file_outer(FILE *f);
#endif
#ifndef __U_BOOT__
/*   job management: */
நிலையான முழு checkjobs(கட்டமைப்பு pipe* fg_pipe);
நிலையான வெறுமை செருகு_bg_job(கட்டமைப்பு pipe *pi);
நிலையான வெறுமை remove_bg_job(கட்டமைப்பு pipe *pi);
#endif
/*     local variable support */
நிலையான எழுத்து **make_list_in(எழுத்து **inp, எழுத்து *name);
நிலையான எழுத்து *செருகு_var_மதிப்பு(எழுத்து *inp);
நிலையான எழுத்து *செருகு_var_மதிப்பு_sub(எழுத்து *inp, முழு tag_subst);

#ifndef __U_BOOT__
/* Table of built-in functions.  They can be forked or not, depending on
 * context: within pipes, they fork.  As simple commands, they do not.
 * When used in non-forking context, they can change global variables
 * in the parent shell process.  If forked, of course they can not.
 * For example, 'unset foo | whatever' will parse and run, but foo will
 * still be set at the end. */
நிலையான கட்டமைப்பு built_in_command bltins[] = {
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
	{வெற்று, வெற்று, வெற்று}
};

நிலையான மாறிலி எழுத்து *set_cwd(வெறுமை)
{
	எனில்(cwd==unknown)
		cwd = வெற்று;     /* xgetcwd(arg) called free(arg) */
	cwd = xgetcwd((எழுத்து *)cwd);
	எனில் (!cwd)
		cwd = unknown;
	திரும்பு cwd;
}

/* built-in 'eval' handler */
நிலையான முழு builtin_eval(கட்டமைப்பு child_prog *child)
{
	எழுத்து *str = வெற்று;
	முழு rcode = EXIT_SUCCESS;

	எனில் (child->argv[1]) {
		str = make_string(child->argv + 1);
		parse_string_outer(str, FLAG_EXIT_FROM_LOOP |
					FLAG_PARSE_SEMICOLON);
		விடுவி(str);
		rcode = last_return_code;
	}
	திரும்பு rcode;
}

/* built-in 'cd <path>' handler */
நிலையான முழு builtin_cd(கட்டமைப்பு child_prog *child)
{
	எழுத்து *newdir;
	எனில் (child->argv[1] == வெற்று)
		newdir = env_get("HOME");
	இல்லையெனில்
		newdir = child->argv[1];
	எனில் (chdir(newdir)) {
		printf("cd: %s: %s\n", newdir, strerror(errno));
		திரும்பு EXIT_FAILURE;
	}
	set_cwd();
	திரும்பு EXIT_SUCCESS;
}

/* built-in 'env' handler */
நிலையான முழு builtin_env(கட்டமைப்பு child_prog *dummy)
{
	எழுத்து **e = environ;
	எனில் (e == வெற்று) திரும்பு EXIT_FAILURE;
	க்கு (; *e; e++) {
		puts(*e);
	}
	திரும்பு EXIT_SUCCESS;
}

/* built-in 'exec' handler */
நிலையான முழு builtin_exec(கட்டமைப்பு child_prog *child)
{
	எனில் (child->argv[1] == வெற்று)
		திரும்பு EXIT_SUCCESS;   /* Really? */
	child->argv++;
	pseudo_exec(child);
	/* never returns */
}

/* built-in 'exit' handler */
நிலையான முழு builtin_exit(கட்டமைப்பு child_prog *child)
{
	எனில் (child->argv[1] == வெற்று)
		exit(last_return_code);
	exit (atoi(child->argv[1]));
}

/* built-in 'export VAR=value' handler */
நிலையான முழு builtin_export(கட்டமைப்பு child_prog *child)
{
	முழு res = 0;
	எழுத்து *name = child->argv[1];

	எனில் (name == வெற்று) {
		திரும்பு (builtin_env(child));
	}

	name = strdup(name);

	எனில்(name) {
		எழுத்து *மதிப்பு = strchr(name, '=');

		எனில் (!மதிப்பு) {
			எழுத்து *tmp;
			/* They are exporting something without an =VALUE */

			மதிப்பு = get_local_var(name);
			எனில் (மதிப்பு) {
				size_t ln = strlen(name);

				tmp = realloc(name, ln+strlen(மதிப்பு)+2);
				எனில்(tmp==வெற்று)
					res = -1;
				இல்லையெனில் {
					sprintf(tmp+ln, "=%s", மதிப்பு);
					name = tmp;
				}
			} இல்லையெனில் {
				/* bash does not return an error when trying to export
				 * an undefined variable.  Do likewise. */
				res = 1;
			}
		}
	}
	எனில் (res<0)
		perror_செய்தி("export");
	இல்லையெனில் எனில்(res==0)
		res = set_local_var(name, 1);
	இல்லையெனில்
		res = 0;
	விடுவி(name);
	திரும்பு res;
}

/* built-in 'fg' and 'bg' handler */
நிலையான முழு builtin_fg_bg(கட்டமைப்பு child_prog *child)
{
	முழு i, jobnum;
	கட்டமைப்பு pipe *pi=வெற்று;

	எனில் (!interactive)
		திரும்பு EXIT_FAILURE;
	/* If they gave us no args, assume they want the last backgrounded task */
	எனில் (!child->argv[1]) {
		க்கு (pi = job_list; pi; pi = pi->next) {
			எனில் (pi->jobid == last_jobid) {
				நிறுத்து;
			}
		}
		எனில் (!pi) {
			error_செய்தி("%s: no current job", child->argv[0]);
			திரும்பு EXIT_FAILURE;
		}
	} இல்லையெனில் {
		எனில் (sscanf(child->argv[1], "%%%d", &jobnum) != 1) {
			error_செய்தி("%s: bad argument '%s'", child->argv[0], child->argv[1]);
			திரும்பு EXIT_FAILURE;
		}
		க்கு (pi = job_list; pi; pi = pi->next) {
			எனில் (pi->jobid == jobnum) {
				நிறுத்து;
			}
		}
		எனில் (!pi) {
			error_செய்தி("%s: %d: no such job", child->argv[0], jobnum);
			திரும்பு EXIT_FAILURE;
		}
	}

	எனில் (*child->argv[0] == 'f') {
		/* Put the job into the foreground.  */
		tcsetpgrp(shell_terminal, pi->pgrp);
	}

	/* Restart the processes in the job */
	க்கு (i = 0; i < pi->இலக்கம்_progs; i++)
		pi->progs[i].is_stopped = 0;

	எனில் ( (i=kill(- pi->pgrp, SIGCONT)) < 0) {
		எனில் (i == ESRCH) {
			remove_bg_job(pi);
		} இல்லையெனில் {
			perror_செய்தி("kill (SIGCONT)");
		}
	}

	pi->stopped_progs = 0;
	திரும்பு EXIT_SUCCESS;
}

/* built-in 'help' handler */
நிலையான முழு builtin_help(கட்டமைப்பு child_prog *dummy)
{
	கட்டமைப்பு built_in_command *x;

	printf("\nBuilt-in commands:\n");
	printf("-------------------\n");
	க்கு (x = bltins; x->cmd; x++) {
		எனில் (x->descr==வெற்று)
			தொடர்;
		printf("%s\t%s\n", x->cmd, x->descr);
	}
	printf("\n\n");
	திரும்பு EXIT_SUCCESS;
}

/* built-in 'jobs' handler */
நிலையான முழு builtin_jobs(கட்டமைப்பு child_prog *child)
{
	கட்டமைப்பு pipe *job;
	எழுத்து *status_string;

	க்கு (job = job_list; job; job = job->next) {
		எனில் (job->running_progs == job->stopped_progs)
			status_string = "Stopped";
		இல்லையெனில்
			status_string = "Running";

		printf(JOB_STATUS_FORMAT, job->jobid, status_string, job->text);
	}
	திரும்பு EXIT_SUCCESS;
}

/* built-in 'pwd' handler */
நிலையான முழு builtin_pwd(கட்டமைப்பு child_prog *dummy)
{
	puts(set_cwd());
	திரும்பு EXIT_SUCCESS;
}

/* built-in 'read VAR' handler */
நிலையான முழு builtin_read(கட்டமைப்பு child_prog *child)
{
	முழு res;

	எனில் (child->argv[1]) {
		எழுத்து string[BUFSIZ];
		எழுத்து *var = 0;

		string[0] = 0;  /* In case stdin has only EOF */
		/* read string */
		fgets(string, அளவு(string), stdin);
		chomp(string);
		var = malloc(strlen(child->argv[1])+strlen(string)+2);
		எனில்(var) {
			sprintf(var, "%s=%s", child->argv[1], string);
			res = set_local_var(var, 0);
		} இல்லையெனில்
			res = -1;
		எனில் (res)
			fprintf(stderr, "read: %m\n");
		விடுவி(var);      /* So not move up to avoid breaking errno */
		திரும்பு res;
	} இல்லையெனில் {
		செய் res=getchar(); போது(res!='\n' && res!=EOF);
		திரும்பு 0;
	}
}

/* built-in 'set VAR=value' handler */
நிலையான முழு builtin_set(கட்டமைப்பு child_prog *child)
{
	எழுத்து *temp = child->argv[1];
	கட்டமைப்பு variables *e;

	எனில் (temp == வெற்று)
		க்கு(e = உச்சி_vars; e; e=e->next)
			printf("%s=%s\n", e->name, e->மதிப்பு);
	இல்லையெனில்
		set_local_var(temp, 0);

		திரும்பு EXIT_SUCCESS;
}

/* Built-in 'shift' handler */
நிலையான முழு builtin_shift(கட்டமைப்பு child_prog *child)
{
	முழு n=1;
	எனில் (child->argv[1]) {
		n=atoi(child->argv[1]);
	}
	எனில் (n>=0 && n<global_argc) {
		/* XXX This probably breaks $0 */
		global_argc -= n;
		global_argv += n;
		திரும்பு EXIT_SUCCESS;
	} இல்லையெனில் {
		திரும்பு EXIT_FAILURE;
	}
}

/* Built-in '.' handler (read-in and execute commands from file) */
நிலையான முழு builtin_source(கட்டமைப்பு child_prog *child)
{
	FILE *உள்ளீடு;
	முழு status;

	எனில் (child->argv[1] == வெற்று)
		திரும்பு EXIT_FAILURE;

	/* XXX search through $PATH is missing */
	உள்ளீடு = fopen(child->argv[1], "r");
	எனில் (!உள்ளீடு) {
		error_செய்தி("Couldn't open file '%s'", child->argv[1]);
		திரும்பு EXIT_FAILURE;
	}

	/* Now run the file */
	/* XXX argv and argc are broken; need to save old global_argv
	 * (pointer only is OK!) on this stack frame,
	 * set global_argv=child->argv+1, recurse, and restore. */
	mark_open(fileno(உள்ளீடு));
	status = parse_file_outer(உள்ளீடு);
	mark_closed(fileno(உள்ளீடு));
	fclose(உள்ளீடு);
	திரும்பு (status);
}

நிலையான முழு builtin_umask(கட்டமைப்பு child_prog *child)
{
	mode_t new_umask;
	மாறிலி எழுத்து *arg = child->argv[1];
	எழுத்து *end;
	எனில் (arg) {
		new_umask=strtoul(arg, &end, 8);
		எனில் (*end!='\0' || end == arg) {
			திரும்பு EXIT_FAILURE;
		}
	} இல்லையெனில் {
		printf("%.3o\n", (unsigned முழு) (new_umask=umask(0)));
	}
	umask(new_umask);
	திரும்பு EXIT_SUCCESS;
}

/* built-in 'unset VAR' handler */
நிலையான முழு builtin_unset(கட்டமைப்பு child_prog *child)
{
	/* bash returned already true */
	unset_local_var(child->argv[1]);
	திரும்பு EXIT_SUCCESS;
}

நிலையான முழு builtin_not_written(கட்டமைப்பு child_prog *child)
{
	printf("builtin_%s not written\n",child->argv[0]);
	திரும்பு EXIT_FAILURE;
}
#endif

நிலையான முழு b_check_space(o_string *o, முழு len)
{
	/* It would be easy to drop a more restrictive policy
	 * in here, such as setting a maximum string length */
	எனில் (o->length + len > o->maxlen) {
		எழுத்து *old_data = o->data;
		/* assert (data == NULL || o->maxlen != 0); */
		o->maxlen += max(2*len, B_CHUNK);
		o->data = realloc(o->data, 1 + o->maxlen);
		எனில் (o->data == வெற்று) {
			விடுவி(old_data);
		}
	}
	திரும்பு o->data == வெற்று;
}

நிலையான முழு b_addchr(o_string *o, முழு ch)
{
	debug_printf("b_addchr: %c %d %p\n", ch, o->length, o);
	எனில் (b_check_space(o, 1)) திரும்பு B_NOSPAC;
	o->data[o->length] = ch;
	o->length++;
	o->data[o->length] = '\0';
	திரும்பு 0;
}

நிலையான வெறுமை b_reset(o_string *o)
{
	o->length = 0;
	o->nonnull = 0;
	எனில் (o->data != வெற்று) *o->data = '\0';
}

நிலையான வெறுமை b_free(o_string *o)
{
	b_reset(o);
	விடுவி(o->data);
	o->data = வெற்று;
	o->maxlen = 0;
}

/* My analysis of quoting semantics tells me that state information
 * is associated with a destination, not a source.
 */
நிலையான முழு b_addqchr(o_string *o, முழு ch, முழு quote)
{
	எனில் (quote && strchr("*?[\\",ch)) {
		முழு rc;
		rc = b_addchr(o, '\\');
		எனில் (rc) திரும்பு rc;
	}
	திரும்பு b_addchr(o, ch);
}

#ifndef __U_BOOT__
நிலையான முழு b_adduint(o_string *o, unsigned முழு i)
{
	முழு r;
	எழுத்து *p = simple_itoa(i);
	/* no escape checking necessary */
	செய் r=b_addchr(o, *p++); போது (r==0 && *p);
	திரும்பு r;
}
#endif

நிலையான முழு static_get(கட்டமைப்பு in_str *i)
{
	முழு ch = *i->p++;
	எனில் (ch=='\0') திரும்பு EOF;
	திரும்பு ch;
}

நிலையான முழு static_peek(கட்டமைப்பு in_str *i)
{
	திரும்பு *i->p;
}

#ifndef __U_BOOT__
நிலையான உள்வரி வெறுமை cmdedit_set_initial_prompt(வெறுமை)
{
#ifndef CONFIG_FEATURE_SH_FANCY_PROMPT
	PS1 = வெற்று;
#இல்லையெனில்
	PS1 = env_get("PS1");
	எனில்(PS1==0)
		PS1 = "\\w \\$ ";
#endif
}

நிலையான உள்வரி வெறுமை setup_prompt_string(முழு promptmode, எழுத்து **prompt_str)
{
	debug_printf("setup_prompt_string %d ",promptmode);
#ifndef CONFIG_FEATURE_SH_FANCY_PROMPT
	/* Set up the prompt */
	எனில் (promptmode == 1) {
		விடுவி(PS1);
		PS1=xmalloc(strlen(cwd)+4);
		sprintf(PS1, "%s %s", cwd, ( geteuid() != 0 ) ?  "$ ":"# ");
		*prompt_str = PS1;
	} இல்லையெனில் {
		*prompt_str = PS2;
	}
#இல்லையெனில்
	*prompt_str = (promptmode==1)? PS1 : PS2;
#endif
	debug_printf("result %s\n",*prompt_str);
}
#endif

#ifdef __U_BOOT__
நிலையான முழு uboot_cli_readline(கட்டமைப்பு in_str *i)
{
	எழுத்து *prompt;
	எழுத்து __maybe_unused *ps_prompt = வெற்று;

	எனில் (i->promptmode == 1)
		prompt = CONFIG_SYS_PROMPT;
	இல்லையெனில்
		prompt = CONFIG_SYS_PROMPT_HUSH_PS2;

#ifdef CONFIG_CMDLINE_PS_SUPPORT
	எனில் (i->promptmode == 1)
		ps_prompt = env_get("PS1");
	இல்லையெனில்
		ps_prompt = env_get("PS2");
	எனில் (ps_prompt)
		prompt = ps_prompt;
#endif

	திரும்பு cli_readline(prompt);
}
#endif

நிலையான வெறுமை get_user_உள்ளீடு(கட்டமைப்பு in_str *i)
{
#ifndef __U_BOOT__
	எழுத்து *prompt_str;
	நிலையான எழுத்து the_command[BUFSIZ];

	setup_prompt_string(i->promptmode, &prompt_str);
#ifdef CONFIG_FEATURE_COMMAND_EDITING
	/*
	 ** enable command line editing only while a command line
	 ** is actually being read; otherwise, we'll end up bequeathing
	 ** atexit() handlers and other unwanted stuff to our
	 ** child processes (rob@sysgo.de)
	 */
	cmdedit_read_உள்ளீடு(prompt_str, the_command);
#இல்லையெனில்
	fputs(prompt_str, stdout);
	fflush(stdout);
	the_command[0]=fgetc(i->file);
	the_command[1]='\0';
#endif
	fflush(stdout);
	i->p = the_command;
#இல்லையெனில்
	முழு n;
	நிலையான எழுத்து the_command[CONFIG_SYS_CBSIZE + 1];

	bootretry_reset_cmd_timeout();
	i->__promptme = 1;
	n = uboot_cli_readline(i);

#ifdef CONFIG_BOOT_RETRY_TIME
	எனில் (n == -2) {
	  puts("\nTimeout waiting for command\n");
#  ifdef CONFIG_RESET_TO_RETRY
	  do_reset(வெற்று, 0, 0, வெற்று);
#  elif IS_ENABLED(CONFIG_RETRY_BOOTCMD)
	strcpy(console_buffer, "run bootcmd\n");
# இல்லையெனில்
#	error "This only works with CONFIG_RESET_TO_RETRY or CONFIG_BOOT_RETRY_COMMAND enabled"
#  endif
	}
#endif
	எனில் (n == -1 ) {
		flag_repeat = 0;
		i->__promptme = 0;
	}
	n = strlen(console_buffer);
	console_buffer[n] = '\n';
	console_buffer[n+1]= '\0';
	எனில் (had_ctrlc()) flag_repeat = 0;
	clear_ctrlc();
	do_repeat = 0;
	எனில் (i->promptmode == 1) {
		எனில் (console_buffer[0] == '\n'&& flag_repeat == 0) {
			strcpy(the_command,console_buffer);
		}
		இல்லையெனில் {
			எனில் (console_buffer[0] != '\n') {
				strcpy(the_command,console_buffer);
				flag_repeat = 1;
			}
			இல்லையெனில் {
				do_repeat = 1;
			}
		}
		i->p = the_command;
	}
	இல்லையெனில் {
		எனில் (console_buffer[0] != '\n') {
			எனில் (strlen(the_command) + strlen(console_buffer)
			    < CONFIG_SYS_CBSIZE) {
				n = strlen(the_command);
				the_command[n-1] = ' ';
				strcpy(&the_command[n],console_buffer);
			}
			இல்லையெனில் {
				the_command[0] = '\n';
				the_command[1] = '\0';
				flag_repeat = 0;
			}
		}
		எனில் (i->__promptme == 0) {
			the_command[0] = '\n';
			the_command[1] = '\0';
		}
		i->p = console_buffer;
	}
#endif
}

/* This is the magic location that prints prompts
 * and gets data back from the user */
நிலையான முழு file_get(கட்டமைப்பு in_str *i)
{
	முழு ch;

	ch = 0;
	/* If there is data waiting, eat it up */
	எனில் (i->p && *i->p) {
		ch = *i->p++;
	} இல்லையெனில் {
		/* need to double check i->file because we might be doing something
		 * more complicated by now, like sourcing or substituting. */
#ifndef __U_BOOT__
		எனில் (i->__promptme && interactive && i->file == stdin) {
			போது(! i->p || (interactive && strlen(i->p)==0) ) {
#இல்லையெனில்
			போது(! i->p  || strlen(i->p)==0 ) {
#endif
				get_user_உள்ளீடு(i);
			}
			i->promptmode=2;
#ifndef __U_BOOT__
			i->__promptme = 0;
#endif
			எனில் (i->p && *i->p) {
				ch = *i->p++;
			}
#ifndef __U_BOOT__
		} இல்லையெனில் {
			ch = fgetc(i->file);
		}

#endif
		debug_printf("b_getch: got a %d\n", ch);
	}
#ifndef __U_BOOT__
	எனில் (ch == '\n') i->__promptme=1;
#endif
	திரும்பு ch;
}

/* All the callers guarantee this routine will never be
 * used right after a newline, so prompting is not needed.
 */
நிலையான முழு file_peek(கட்டமைப்பு in_str *i)
{
#ifndef __U_BOOT__
	எனில் (i->p && *i->p) {
#endif
		திரும்பு *i->p;
#ifndef __U_BOOT__
	} இல்லையெனில் {
		i->peek_buf[0] = fgetc(i->file);
		i->peek_buf[1] = '\0';
		i->p = i->peek_buf;
		debug_printf("b_peek: got a %d\n", *i->p);
		திரும்பு *i->p;
	}
#endif
}

#ifndef __U_BOOT__
நிலையான வெறுமை setup_file_in_str(கட்டமைப்பு in_str *i, FILE *f)
#இல்லையெனில்
நிலையான வெறுமை setup_file_in_str(கட்டமைப்பு in_str *i)
#endif
{
	i->peek = file_peek;
	i->get = file_get;
	i->__promptme=1;
	i->promptmode=1;
#ifndef __U_BOOT__
	i->file = f;
#endif
	i->p = வெற்று;
}

நிலையான வெறுமை setup_string_in_str(கட்டமைப்பு in_str *i, மாறிலி எழுத்து *s)
{
	i->peek = static_peek;
	i->get = static_get;
	i->__promptme=1;
	i->promptmode=1;
	i->p = s;
}

#ifndef __U_BOOT__
நிலையான வெறுமை mark_open(முழு fd)
{
	கட்டமைப்பு close_me *new = xmalloc(அளவு(கட்டமைப்பு close_me));
	new->fd = fd;
	new->next = close_me_head;
	close_me_head = new;
}

நிலையான வெறுமை mark_closed(முழு fd)
{
	கட்டமைப்பு close_me *tmp;
	எனில் (close_me_head == வெற்று || close_me_head->fd != fd)
		error_செய்தி_and_die("corrupt close_me");
	tmp = close_me_head;
	close_me_head = close_me_head->next;
	விடுவி(tmp);
}

நிலையான வெறுமை close_all(வெறுமை)
{
	கட்டமைப்பு close_me *c;
	க்கு (c=close_me_head; c; c=c->next) {
		close(c->fd);
	}
	close_me_head = வெற்று;
}

/* squirrel != NULL means we squirrel away copies of stdin, stdout,
 * and stderr if they are redirected. */
நிலையான முழு setup_redirects(கட்டமைப்பு child_prog *prog, முழு squirrel[])
{
	முழு openfd, mode;
	கட்டமைப்பு redir_struct *redir;

	க்கு (redir=prog->redirects; redir; redir=redir->next) {
		எனில் (redir->dup == -1 && redir->word.gl_pathv == வெற்று) {
			/* something went wrong in the parse.  Pretend it didn't happen */
			தொடர்;
		}
		எனில் (redir->dup == -1) {
			mode=redir_table[redir->type].mode;
			openfd = open(redir->word.gl_pathv[0], mode, 0666);
			எனில் (openfd < 0) {
			/* this could get lost if stderr has been redirected, but
			   bash and ash both lose it as well (though zsh doesn't!) */
				perror_செய்தி("error opening %s", redir->word.gl_pathv[0]);
				திரும்பு 1;
			}
		} இல்லையெனில் {
			openfd = redir->dup;
		}

		எனில் (openfd != redir->fd) {
			எனில் (squirrel && redir->fd < 3) {
				squirrel[redir->fd] = dup(redir->fd);
			}
			எனில் (openfd == -3) {
				close(openfd);
			} இல்லையெனில் {
				dup2(openfd, redir->fd);
				எனில் (redir->dup == -1)
					close (openfd);
			}
		}
	}
	திரும்பு 0;
}

நிலையான வெறுமை restore_redirects(முழு squirrel[])
{
	முழு i, fd;
	க்கு (i=0; i<3; i++) {
		fd = squirrel[i];
		எனில் (fd != -1) {
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
நிலையான வெறுமை pseudo_exec(கட்டமைப்பு child_prog *child)
{
	முழு i, rcode;
	எழுத்து *p;
	கட்டமைப்பு built_in_command *x;
	எனில் (child->argv) {
		க்கு (i=0; is_assignment(child->argv[i]); i++) {
			debug_printf("pid %d environment modification: %s\n",getpid(),child->argv[i]);
			p = செருகு_var_மதிப்பு(child->argv[i]);
			putenv(strdup(p));
			எனில் (p != child->argv[i]) விடுவி(p);
		}
		child->argv+=i;  /* XXX this hack isn't so horrible, since we are about
					to exit, and therefore don't need to keep data
					structures consistent for free() use. */
		/* If a variable is assigned in a forest, and nobody listens,
		 * was it ever really set?
		 */
		எனில் (child->argv[0] == வெற்று) {
			_exit(EXIT_SUCCESS);
		}

		/*
		 * Check if the command matches any of the builtins.
		 * Depending on context, this might be redundant.  But it's
		 * easier to waste a few CPU cycles than it is to figure out
		 * if this is one of those cases.
		 */
		க்கு (x = bltins; x->cmd; x++) {
			எனில் (strcmp(child->argv[0], x->cmd) == 0 ) {
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
			முழு argc_l;
			எழுத்து** argv_l=child->argv;
			எழுத்து *name = child->argv[0];

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
			க்கு(argc_l=0;*argv_l!=வெற்று; argv_l++, argc_l++);
			optind = 1;
			debug_printf("running applet %s\n", name);
			run_applet_by_name(name, argc_l, child->argv);
		}
#endif
		debug_printf("exec of %s\n",child->argv[0]);
		execvp(child->argv[0],child->argv);
		perror_செய்தி("couldn't exec: %s",child->argv[0]);
		_exit(1);
	} இல்லையெனில் எனில் (child->group) {
		debug_printf("runtime nesting to group\n");
		interactive=0;    /* crucial!!!! */
		rcode = run_list_real(child->group);
		/* OK to leak memory by not calling free_pipe_list,
		 * since this process is about to exit */
		_exit(rcode);
	} இல்லையெனில் {
		/* Can happen.  See what bash does with ">foo" by itself. */
		debug_printf("trying to pseudo_exec null command\n");
		_exit(EXIT_SUCCESS);
	}
}

நிலையான வெறுமை செருகு_bg_job(கட்டமைப்பு pipe *pi)
{
	கட்டமைப்பு pipe *thejob;

	/* Linear search for the ID of the job to use */
	pi->jobid = 1;
	க்கு (thejob = job_list; thejob; thejob = thejob->next)
		எனில் (thejob->jobid >= pi->jobid)
			pi->jobid = thejob->jobid + 1;

	/* add thejob to the list of running jobs */
	எனில் (!job_list) {
		thejob = job_list = xmalloc(அளவு(*thejob));
	} இல்லையெனில் {
		க்கு (thejob = job_list; thejob->next; thejob = thejob->next) /* nothing */;
		thejob->next = xmalloc(அளவு(*thejob));
		thejob = thejob->next;
	}

	/* physically copy the struct job */
	memcpy(thejob, pi, அளவு(கட்டமைப்பு pipe));
	thejob->next = வெற்று;
	thejob->running_progs = thejob->இலக்கம்_progs;
	thejob->stopped_progs = 0;
	thejob->text = xmalloc(BUFSIZ); /* cmdedit buffer size */

	/*if (pi->progs[0] && pi->progs[0].argv && pi->progs[0].argv[0]) */
	{
		எழுத்து *bar=thejob->text;
		எழுத்து **foo=pi->progs[0].argv;
		போது(foo && *foo) {
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
நிலையான வெறுமை remove_bg_job(கட்டமைப்பு pipe *pi)
{
	கட்டமைப்பு pipe *prev_pipe;

	எனில் (pi == job_list) {
		job_list = pi->next;
	} இல்லையெனில் {
		prev_pipe = job_list;
		போது (prev_pipe->next != pi)
			prev_pipe = prev_pipe->next;
		prev_pipe->next = pi->next;
	}
	எனில் (job_list)
		last_jobid = job_list->jobid;
	இல்லையெனில்
		last_jobid = 0;

	pi->stopped_progs = 0;
	free_pipe(pi, 0);
	விடுவி(pi);
}

/* Checks to see if any processes have exited -- if they
   have, figure out why and see if a job has completed */
நிலையான முழு checkjobs(கட்டமைப்பு pipe* fg_pipe)
{
	முழு attributes;
	முழு status;
	முழு prognum = 0;
	கட்டமைப்பு pipe *pi;
	pid_t childpid;

	attributes = WUNTRACED;
	எனில் (fg_pipe==வெற்று) {
		attributes |= WNOHANG;
	}

	போது ((childpid = waitpid(-1, &status, attributes)) > 0) {
		எனில் (fg_pipe) {
			முழு i, rcode = 0;
			க்கு (i=0; i < fg_pipe->இலக்கம்_progs; i++) {
				எனில் (fg_pipe->progs[i].pid == childpid) {
					எனில் (i==fg_pipe->இலக்கம்_progs-1)
						rcode=WEXITSTATUS(status);
					(fg_pipe->இலக்கம்_progs)--;
					திரும்பு(rcode);
				}
			}
		}

		க்கு (pi = job_list; pi; pi = pi->next) {
			prognum = 0;
			போது (prognum < pi->இலக்கம்_progs && pi->progs[prognum].pid != childpid) {
				prognum++;
			}
			எனில் (prognum < pi->இலக்கம்_progs)
				நிறுத்து;
		}

		எனில்(pi==வெற்று) {
			debug_printf("checkjobs: pid %d was not in our list!\n", childpid);
			தொடர்;
		}

		எனில் (WIFEXITED(status) || WIFSIGNALED(status)) {
			/* child exited */
			pi->running_progs--;
			pi->progs[prognum].pid = 0;

			எனில் (!pi->running_progs) {
				printf(JOB_STATUS_FORMAT, pi->jobid, "Done", pi->text);
				remove_bg_job(pi);
			}
		} இல்லையெனில் {
			/* child stopped */
			pi->stopped_progs++;
			pi->progs[prognum].is_stopped = 1;

#எனில் 0
			/* Printing this stuff is a pain, since it tends to
			 * overwrite the prompt an inconveinient moments.  So
			 * don't do that.  */
			எனில் (pi->stopped_progs == pi->இலக்கம்_progs) {
				printf("\n"JOB_STATUS_FORMAT, pi->jobid, "Stopped", pi->text);
			}
#endif
		}
	}

	எனில் (childpid == -1 && errno != ECHILD)
		perror_செய்தி("waitpid");

	/* move the shell to the foreground */
	/*if (interactive && tcsetpgrp(shell_terminal, getpgid(0))) */
	/*	perror_msg("tcsetpgrp-2"); */
	திரும்பு -1;
}

/* Figure out our controlling tty, checking in order stderr,
 * stdin, and stdout.  If check_pgrp is set, also check that
 * we belong to the foreground process group associated with
 * that tty.  The value of shell_terminal is needed in order to call
 * tcsetpgrp(shell_terminal, ...); */
வெறுமை controlling_tty(முழு check_pgrp)
{
	pid_t curpgrp;

	எனில் ((curpgrp = tcgetpgrp(shell_terminal = 2)) < 0
			&& (curpgrp = tcgetpgrp(shell_terminal = 0)) < 0
			&& (curpgrp = tcgetpgrp(shell_terminal = 1)) < 0)
		செல் shell_terminal_error;

	எனில் (check_pgrp && curpgrp != getpgid(0))
		செல் shell_terminal_error;

	திரும்பு;

shell_terminal_error:
		shell_terminal = -1;
		திரும்பு;
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
நிலையான முழு run_pipe_real(கட்டமைப்பு pipe *pi)
{
	முழு i;
#ifndef __U_BOOT__
	முழு nextin, nextout;
	முழு pipefds[2];				/* pipefds[0] is for reading */
	கட்டமைப்பு child_prog *child;
	கட்டமைப்பு built_in_command *x;
	எழுத்து *p;
# எனில் __GNUC__
	/* Avoid longjmp clobbering */
	(வெறுமை) &i;
	(வெறுமை) &nextin;
	(வெறுமை) &nextout;
	(வெறுமை) &child;
# endif
#இல்லையெனில்
	முழு nextin;
	முழு flag = do_repeat ? CMD_FLAG_REPEAT : 0;
	கட்டமைப்பு child_prog *child;
	எழுத்து *p;
# எனில் __GNUC__
	/* Avoid longjmp clobbering */
	(வெறுமை) &i;
	(வெறுமை) &nextin;
	(வெறுமை) &child;
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
	எனில் (pi->இலக்கம்_progs == 1) child = & (pi->progs[0]);
#ifndef __U_BOOT__
	எனில் (pi->இலக்கம்_progs == 1 && child->group && child->subshell == 0) {
		முழு squirrel[] = {-1, -1, -1};
		முழு rcode;
		debug_printf("non-subshell grouping\n");
		setup_redirects(child, squirrel);
		/* XXX could we merge code with following builtin case,
		 * by creating a pseudo builtin that calls run_list_real? */
		rcode = run_list_real(child->group);
		restore_redirects(squirrel);
#இல்லையெனில்
		எனில் (pi->இலக்கம்_progs == 1 && child->group) {
		முழு rcode;
		debug_printf("non-subshell grouping\n");
		rcode = run_list_real(child->group);
#endif
		திரும்பு rcode;
	} இல்லையெனில் எனில் (pi->இலக்கம்_progs == 1 && pi->progs[0].argv != வெற்று) {
		க்கு (i=0; is_assignment(child->argv[i]); i++) { /* nothing */ }
		எனில் (i!=0 && child->argv[i]==வெற்று) {
			/* assignments, but no command: set the local environment */
			க்கு (i=0; child->argv[i]!=வெற்று; i++) {

				/* Ok, this case is tricky.  We have to decide if this is a
				 * local variable, or an already exported variable.  If it is
				 * already exported, we have to export the new value.  If it is
				 * not exported, we need only set this as a local variable.
				 * This junk is all to decide whether or not to export this
				 * variable. */
				முழு export_me=0;
				எழுத்து *name, *மதிப்பு;
				name = xstrdup(child->argv[i]);
				debug_printf("Local environment set: %s\n", name);
				மதிப்பு = strchr(name, '=');
				எனில் (மதிப்பு)
					*மதிப்பு=0;
#ifndef __U_BOOT__
				எனில் ( get_local_var(name)) {
					export_me=1;
				}
#endif
				விடுவி(name);
				p = செருகு_var_மதிப்பு(child->argv[i]);
				set_local_var(p, export_me);
				எனில் (p != child->argv[i]) விடுவி(p);
			}
			திரும்பு EXIT_SUCCESS;   /* don't worry about errors in set_local_var() yet */
		}
		க்கு (i = 0; is_assignment(child->argv[i]); i++) {
			p = செருகு_var_மதிப்பு(child->argv[i]);
#ifndef __U_BOOT__
			putenv(strdup(p));
#இல்லையெனில்
			set_local_var(p, 0);
#endif
			எனில் (p != child->argv[i]) {
				child->sp--;
				விடுவி(p);
			}
		}
		எனில் (child->sp) {
			எழுத்து * str = வெற்று;

			str = make_string(child->argv + i,
					  child->argv_nonnull + i);
			parse_string_outer(str, FLAG_EXIT_FROM_LOOP | FLAG_REPARSING);
			விடுவி(str);
			திரும்பு last_return_code;
		}
#ifndef __U_BOOT__
		க்கு (x = bltins; x->cmd; x++) {
			எனில் (strcmp(child->argv[i], x->cmd) == 0 ) {
				முழு squirrel[] = {-1, -1, -1};
				முழு rcode;
				எனில் (x->function == builtin_exec && child->argv[i+1]==வெற்று) {
					debug_printf("magic exec\n");
					setup_redirects(child,வெற்று);
					திரும்பு EXIT_SUCCESS;
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
			திரும்பு rcode;
		}
#இல்லையெனில்
		/* check ";", because ,example , argv consist from
		 * "help;flinfo" must not execute
		 */
		எனில் (strchr(child->argv[i], ';')) {
			printf("Unknown command '%s' - try 'help' or use "
					"'run' command\n", child->argv[i]);
			திரும்பு -1;
		}
		/* Process the command */
		திரும்பு cmd_process(flag, child->argc - i, child->argv + i,
				   &flag_repeat, வெற்று);
#endif
	}
#ifndef __U_BOOT__

	க்கு (i = 0; i < pi->இலக்கம்_progs; i++) {
		child = & (pi->progs[i]);

		/* pipes are inserted between pairs of commands */
		எனில் ((i + 1) < pi->இலக்கம்_progs) {
			எனில் (pipe(pipefds)<0) perror_செய்தி_and_die("pipe");
			nextout = pipefds[1];
		} இல்லையெனில் {
			nextout=1;
			pipefds[0] = -1;
		}

		/* XXX test for failed fork()? */
		எனில் (!(child->pid = fork())) {
			/* Set the handling for job control signals back to the default.  */
			சமிக்ஞை(SIGINT, SIG_DFL);
			சமிக்ஞை(SIGQUIT, SIG_DFL);
			சமிக்ஞை(SIGTERM, SIG_DFL);
			சமிக்ஞை(SIGTSTP, SIG_DFL);
			சமிக்ஞை(SIGTTIN, SIG_DFL);
			சமிக்ஞை(SIGTTOU, SIG_DFL);
			சமிக்ஞை(SIGCHLD, SIG_DFL);

			close_all();

			எனில் (nextin != 0) {
				dup2(nextin, 0);
				close(nextin);
			}
			எனில் (nextout != 1) {
				dup2(nextout, 1);
				close(nextout);
			}
			எனில் (pipefds[0]!=-1) {
				close(pipefds[0]);  /* opposite end of our output pipe */
			}

			/* Like bash, explicit redirects override pipes,
			 * and the pipe fd is available for dup'ing. */
			setup_redirects(child,வெற்று);

			எனில் (interactive && pi->followup!=PIPE_BG) {
				/* If we (the child) win the race, put ourselves in the process
				 * group whose leader is the first process in this pipe. */
				எனில் (pi->pgrp < 0) {
					pi->pgrp = getpid();
				}
				எனில் (setpgid(0, pi->pgrp) == 0) {
					tcsetpgrp(2, pi->pgrp);
				}
			}

			pseudo_exec(child);
		}

		/* put our child in the process group whose leader is the
		   first process in this pipe */
		எனில் (pi->pgrp < 0) {
			pi->pgrp = child->pid;
		}
		/* Don't check for errors.  The child may be dead already,
		 * in which case setpgid returns error code EACCES. */
		setpgid(child->pid, pi->pgrp);

		எனில் (nextin != 0)
			close(nextin);
		எனில் (nextout != 1)
			close(nextout);

		/* If there isn't another process, nextin is garbage
		   but it doesn't matter */
		nextin = pipefds[0];
	}
#endif
	திரும்பு -1;
}

நிலையான முழு run_list_real(கட்டமைப்பு pipe *pi)
{
	எழுத்து *save_name = வெற்று;
	எழுத்து **list = வெற்று;
	எழுத்து **save_list = வெற்று;
	கட்டமைப்பு pipe *rpipe;
	முழு flag_rep = 0;
#ifndef __U_BOOT__
	முழு save_இலக்கம்_progs;
#endif
	முழு rcode=0, flag_skip=1;
	முழு flag_restore = 0;
	முழு if_code=0, next_if_code=0;  /* need double-buffer to handle elif */
	reserved_style rmode, skip_more_in_this_rmode=RES_XXXX;
	/* check syntax for "for" */
	க்கு (rpipe = pi; rpipe; rpipe = rpipe->next) {
		எனில் ((rpipe->r_mode == RES_IN ||
		    rpipe->r_mode == RES_FOR) &&
		    (rpipe->next == வெற்று)) {
				syntax();
#ifdef __U_BOOT__
				flag_repeat = 0;
#endif
				திரும்பு 1;
		}
		எனில் ((rpipe->r_mode == RES_IN &&
			(rpipe->next->r_mode == RES_IN &&
			rpipe->next->progs->argv != வெற்று))||
			(rpipe->r_mode == RES_FOR &&
			rpipe->next->r_mode != RES_IN)) {
				syntax();
#ifdef __U_BOOT__
				flag_repeat = 0;
#endif
				திரும்பு 1;
		}
	}
	க்கு (; pi; pi = (flag_restore != 0) ? rpipe : pi->next) {
		எனில் (pi->r_mode == RES_WHILE || pi->r_mode == RES_UNTIL ||
			pi->r_mode == RES_FOR) {
#ifdef __U_BOOT__
				/* check Ctrl-C */
				ctrlc();
				எனில் ((had_ctrlc())) {
					திரும்பு 1;
				}
#endif
				flag_restore = 0;
				எனில் (!rpipe) {
					flag_rep = 0;
					rpipe = pi;
				}
		}
		rmode = pi->r_mode;
		debug_printf("rmode=%d  if_code=%d  next_if_code=%d skip_more=%d\n", rmode, if_code, next_if_code, skip_more_in_this_rmode);
		எனில் (rmode == skip_more_in_this_rmode && flag_skip) {
			எனில் (pi->followup == PIPE_SEQ) flag_skip=0;
			தொடர்;
		}
		flag_skip = 1;
		skip_more_in_this_rmode = RES_XXXX;
		எனில் (rmode == RES_THEN || rmode == RES_ELSE) if_code = next_if_code;
		எனில் (rmode == RES_THEN &&  if_code) தொடர்;
		எனில் (rmode == RES_ELSE && !if_code) தொடர்;
		எனில் (rmode == RES_ELIF && !if_code) நிறுத்து;
		எனில் (rmode == RES_FOR && pi->இலக்கம்_progs) {
			எனில் (!list) {
				/* if no variable values after "in" we skip "for" */
				எனில் (!pi->next->progs->argv) தொடர்;
				/* create list of variable values */
				list = make_list_in(pi->next->progs->argv,
					pi->progs->argv[0]);
				save_list = list;
				save_name = pi->progs->argv[0];
				pi->progs->argv[0] = வெற்று;
				flag_rep = 1;
			}
			எனில் (!(*list)) {
				விடுவி(pi->progs->argv[0]);
				விடுவி(save_list);
				list = வெற்று;
				flag_rep = 0;
				pi->progs->argv[0] = save_name;
#ifndef __U_BOOT__
				pi->progs->glob_result.gl_pathv[0] =
					pi->progs->argv[0];
#endif
				தொடர்;
			} இல்லையெனில் {
				/* insert new value from list for variable */
				விடுவி(pi->progs->argv[0]);
				pi->progs->argv[0] = *list++;
#ifndef __U_BOOT__
				pi->progs->glob_result.gl_pathv[0] =
					pi->progs->argv[0];
#endif
			}
		}
		எனில் (rmode == RES_IN) தொடர்;
		எனில் (rmode == RES_DO) {
			எனில் (!flag_rep) தொடர்;
		}
		எனில் (rmode == RES_DONE) {
			எனில் (flag_rep) {
				flag_restore = 1;
			} இல்லையெனில் {
				rpipe = வெற்று;
			}
		}
		எனில் (pi->இலக்கம்_progs == 0) தொடர்;
#ifndef __U_BOOT__
		save_இலக்கம்_progs = pi->இலக்கம்_progs; /* save number of programs */
#endif
		rcode = run_pipe_real(pi);
		debug_printf("run_pipe_real returned %d\n",rcode);
#ifndef __U_BOOT__
		எனில் (rcode!=-1) {
			/* We only ran a builtin: rcode was set by the return value
			 * of run_pipe_real(), and we don't need to wait for anything. */
		} இல்லையெனில் எனில் (pi->followup==PIPE_BG) {
			/* XXX check bash's behavior with nontrivial pipes */
			/* XXX compute jobid */
			/* XXX what does bash do with attempts to background builtins? */
			செருகு_bg_job(pi);
			rcode = EXIT_SUCCESS;
		} இல்லையெனில் {
			எனில் (interactive) {
				/* move the new process group into the foreground */
				எனில் (tcsetpgrp(shell_terminal, pi->pgrp) && errno != ENOTTY)
					perror_செய்தி("tcsetpgrp-3");
				rcode = checkjobs(pi);
				/* move the shell to the foreground */
				எனில் (tcsetpgrp(shell_terminal, getpgid(0)) && errno != ENOTTY)
					perror_செய்தி("tcsetpgrp-4");
			} இல்லையெனில் {
				rcode = checkjobs(pi);
			}
			debug_printf("checkjobs returned %d\n",rcode);
		}
		last_return_code=rcode;
#இல்லையெனில்
		எனில் (rcode < -1) {
			last_return_code = -rcode - 2;
			திரும்பு -2;	/* exit */
		}
		last_return_code = rcode;
#endif
#ifndef __U_BOOT__
		pi->இலக்கம்_progs = save_இலக்கம்_progs; /* restore number of programs */
#endif
		எனில் ( rmode == RES_IF || rmode == RES_ELIF )
			next_if_code=rcode;  /* can be overwritten a number of times */
		எனில் (rmode == RES_WHILE)
			flag_rep = !last_return_code;
		எனில் (rmode == RES_UNTIL)
			flag_rep = last_return_code;
		எனில் ( (rcode==EXIT_SUCCESS && pi->followup==PIPE_OR) ||
		     (rcode!=EXIT_SUCCESS && pi->followup==PIPE_AND) )
			skip_more_in_this_rmode=rmode;
#ifndef __U_BOOT__
		checkjobs(வெற்று);
#endif
	}
	திரும்பு rcode;
}

/* broken, of course, but OK for testing */
நிலையான எழுத்து *indenter(முழு i)
{
	நிலையான எழுத்து blanks[]="                                    ";
	திரும்பு &blanks[அளவு(blanks)-i-1];
}

/* return code is the exit status of the pipe */
நிலையான முழு free_pipe(கட்டமைப்பு pipe *pi, முழு indent)
{
	எழுத்து **p;
	கட்டமைப்பு child_prog *child;
#ifndef __U_BOOT__
	கட்டமைப்பு redir_struct *r, *rnext;
#endif
	முழு a, i, ret_code=0;
	எழுத்து *ind = indenter(indent);

#ifndef __U_BOOT__
	எனில் (pi->stopped_progs > 0)
		திரும்பு ret_code;
	final_printf("%s run pipe: (pid %d)\n",ind,getpid());
#endif
	க்கு (i=0; i<pi->இலக்கம்_progs; i++) {
		child = &pi->progs[i];
		final_printf("%s  command %d:\n",ind,i);
		எனில் (child->argv) {
			க்கு (a=0,p=child->argv; *p; a++,p++) {
				final_printf("%s   argv[%d] = %s\n",ind,a,*p);
			}
#ifndef __U_BOOT__
			globfree(&child->glob_result);
#இல்லையெனில்
			க்கு (a = 0; a < child->argc; a++) {
				விடுவி(child->argv[a]);
			}
			விடுவி(child->argv);
			விடுவி(child->argv_nonnull);
			child->argc = 0;
#endif
			child->argv=வெற்று;
		} இல்லையெனில் எனில் (child->group) {
#ifndef __U_BOOT__
			final_printf("%s   begin group (subshell:%d)\n",ind, child->subshell);
#endif
			ret_code = free_pipe_list(child->group,indent+3);
			final_printf("%s   end group\n",ind);
		} இல்லையெனில் {
			final_printf("%s   (nil)\n",ind);
		}
#ifndef __U_BOOT__
		க்கு (r=child->redirects; r; r=rnext) {
			final_printf("%s   redirect %d%s", ind, r->fd, redir_table[r->type].descrip);
			எனில் (r->dup == -1) {
				/* guard against the case >$FOO, where foo is unset or blank */
				எனில் (r->word.gl_pathv) {
					final_printf(" %s\n", *r->word.gl_pathv);
					globfree(&r->word);
				}
			} இல்லையெனில் {
				final_printf("&%d\n", r->dup);
			}
			rnext=r->next;
			விடுவி(r);
		}
		child->redirects=வெற்று;
#endif
	}
	விடுவி(pi->progs);   /* children are an array, they get freed all at once */
	pi->progs=வெற்று;
	திரும்பு ret_code;
}

நிலையான முழு free_pipe_list(கட்டமைப்பு pipe *head, முழு indent)
{
	முழு rcode=0;   /* if list has no members */
	கட்டமைப்பு pipe *pi, *next;
	எழுத்து *ind = indenter(indent);
	க்கு (pi=head; pi; pi=next) {
		final_printf("%s pipe reserved mode %d\n", ind, pi->r_mode);
		rcode = free_pipe(pi, indent);
		final_printf("%s pipe followup code %d\n", ind, pi->followup);
		next=pi->next;
		pi->next=வெற்று;
		விடுவி(pi);
	}
	திரும்பு rcode;
}

/* Select which version we will use */
நிலையான முழு run_list(கட்டமைப்பு pipe *pi)
{
	முழு rcode=0;
#ifndef __U_BOOT__
	எனில் (fake_mode==0) {
#endif
		rcode = run_list_real(pi);
#ifndef __U_BOOT__
	}
#endif
	/* free_pipe_list has the side effect of clearing memory
	 * In the long run that function can be merged with run_list_real,
	 * but doing that now would hobble the debugging effort. */
	free_pipe_list(pi,0);
	திரும்பு rcode;
}

/* The API for glob is arguably broken.  This routine pushes a non-matching
 * string into the output structure, removing non-backslashed backslashes.
 * If someone can prove me wrong, by performing this function within the
 * original glob(3) api, feel free to rewrite this routine into oblivion.
 * Return code (0 vs. GLOB_NOSPACE) matches glob(3).
 * XXX broken if the last character is '\\', check that before calling.
 */
#ifndef __U_BOOT__
நிலையான முழு globhack(மாறிலி எழுத்து *src, முழு flags, glob_t *pglob)
{
	முழு cnt=0, pathc;
	மாறிலி எழுத்து *s;
	எழுத்து *dest;
	க்கு (cnt=1, s=src; s && *s; s++) {
		எனில் (*s == '\\') s++;
		cnt++;
	}
	dest = malloc(cnt);
	எனில் (!dest) திரும்பு GLOB_NOSPACE;
	எனில் (!(flags & GLOB_APPEND)) {
		pglob->gl_pathv=வெற்று;
		pglob->gl_pathc=0;
		pglob->gl_offs=0;
		pglob->gl_offs=0;
	}
	pathc = ++pglob->gl_pathc;
	pglob->gl_pathv = realloc(pglob->gl_pathv, (pathc+1)*அளவு(*pglob->gl_pathv));
	எனில் (pglob->gl_pathv == வெற்று) திரும்பு GLOB_NOSPACE;
	pglob->gl_pathv[pathc-1]=dest;
	pglob->gl_pathv[pathc]=வெற்று;
	க்கு (s=src; s && *s; s++, dest++) {
		எனில் (*s == '\\') s++;
		*dest = *s;
	}
	*dest='\0';
	திரும்பு 0;
}

/* XXX broken if the last character is '\\', check that before calling */
நிலையான முழு glob_needed(மாறிலி எழுத்து *s)
{
	க்கு (; *s; s++) {
		எனில் (*s == '\\') s++;
		எனில் (strchr("*[?",*s)) திரும்பு 1;
	}
	திரும்பு 0;
}

#எனில் 0
நிலையான வெறுமை globprint(glob_t *pglob)
{
	முழு i;
	debug_printf("glob_t at %p:\n", pglob);
	debug_printf("  gl_pathc=%d  gl_pathv=%p  gl_offs=%d  gl_flags=%d\n",
		pglob->gl_pathc, pglob->gl_pathv, pglob->gl_offs, pglob->gl_flags);
	க்கு (i=0; i<pglob->gl_pathc; i++)
		debug_printf("pglob->gl_pathv[%d] = %p = %s\n", i,
			pglob->gl_pathv[i], pglob->gl_pathv[i]);
}
#endif

நிலையான முழு xglob(o_string *dest, முழு flags, glob_t *pglob)
{
	முழு gr;

	/* short-circuit for null word */
	/* we can code this better when the debug_printf's are gone */
	எனில் (dest->length == 0) {
		எனில் (dest->nonnull) {
			/* bash man page calls this an "explicit" null */
			gr = globhack(dest->data, flags, pglob);
			debug_printf("globhack returned %d\n",gr);
		} இல்லையெனில் {
			திரும்பு 0;
		}
	} இல்லையெனில் எனில் (glob_needed(dest->data)) {
		gr = glob(dest->data, flags, வெற்று, pglob);
		debug_printf("glob returned %d\n",gr);
		எனில் (gr == GLOB_NOMATCH) {
			/* quote removal, or more accurately, backslash removal */
			gr = globhack(dest->data, flags, pglob);
			debug_printf("globhack returned %d\n",gr);
		}
	} இல்லையெனில் {
		gr = globhack(dest->data, flags, pglob);
		debug_printf("globhack returned %d\n",gr);
	}
	எனில் (gr == GLOB_NOSPACE)
		error_செய்தி_and_die("out of memory during glob");
	எனில் (gr != 0) { /* GLOB_ABORTED ? */
		error_செய்தி("glob(3) error %d",gr);
	}
	/* globprint(glob_target); */
	திரும்பு gr;
}
#endif

#ifdef __U_BOOT__
நிலையான எழுத்து *get_dollar_var(எழுத்து ch);
#endif

/* This is used to get/check local shell variables */
எழுத்து *get_local_var(மாறிலி எழுத்து *s)
{
	கட்டமைப்பு variables *cur;

	எனில் (!s)
		திரும்பு வெற்று;

#ifdef __U_BOOT__
	எனில் (*s == '$')
		திரும்பு get_dollar_var(s[1]);
#endif

	க்கு (cur = உச்சி_vars; cur; cur=cur->next)
		எனில்(strcmp(cur->name, s)==0)
			திரும்பு cur->மதிப்பு;
	திரும்பு வெற்று;
}

/* This is used to set local shell variables
   flg_export==0 if only local (not exporting) variable
   flg_export==1 if "new" exporting environ
   flg_export>1  if current startup environ (not call putenv()) */
முழு set_local_var(மாறிலி எழுத்து *s, முழு flg_export)
{
	எழுத்து *name, *மதிப்பு;
	முழு result=0;
	கட்டமைப்பு variables *cur;

#ifdef __U_BOOT__
	/* might be possible! */
	எனில் (!isalpha(*s))
		திரும்பு -1;
#endif

	name=strdup(s);

	/* Assume when we enter this function that we are already in
	 * NAME=VALUE format.  So the first order of business is to
	 * split 's' on the '=' into 'name' and 'value' */
	மதிப்பு = strchr(name, '=');
	எனில் (!மதிப்பு) {
		விடுவி(name);
		திரும்பு -1;
	}
	*மதிப்பு++ = 0;

	எனில் (!*மதிப்பு) {
		unset_local_var(name);
		விடுவி(name);
		திரும்பு 0;
	}

	க்கு(cur = உச்சி_vars; cur; cur = cur->next) {
		எனில்(strcmp(cur->name, name)==0)
			நிறுத்து;
	}

	எனில்(cur) {
		எனில்(strcmp(cur->மதிப்பு, மதிப்பு)==0) {
			எனில்(flg_export>0 && cur->flg_export==0)
				cur->flg_export=flg_export;
			இல்லையெனில்
				result++;
		} இல்லையெனில் {
			எனில்(cur->flg_read_only) {
				error_செய்தி("%s: readonly variable", name);
				result = -1;
			} இல்லையெனில் {
				எனில்(flg_export>0 || cur->flg_export>1)
					cur->flg_export=1;
				விடுவி(cur->மதிப்பு);

				cur->மதிப்பு = strdup(மதிப்பு);
			}
		}
	} இல்லையெனில் {
		cur = malloc(அளவு(கட்டமைப்பு variables));
		எனில்(!cur) {
			result = -1;
		} இல்லையெனில் {
			cur->name = strdup(name);
			எனில் (cur->name == வெற்று) {
				விடுவி(cur);
				result = -1;
			} இல்லையெனில் {
				கட்டமைப்பு variables *bottom = உச்சி_vars;
				cur->மதிப்பு = strdup(மதிப்பு);
				cur->next = வெற்று;
				cur->flg_export = flg_export;
				cur->flg_read_only = 0;
				போது(bottom->next) bottom=bottom->next;
				bottom->next = cur;
			}
		}
	}

#ifndef __U_BOOT__
	எனில்(result==0 && cur->flg_export==1) {
		*(மதிப்பு-1) = '=';
		result = putenv(name);
	} இல்லையெனில் {
#endif
		விடுவி(name);
#ifndef __U_BOOT__
		எனில்(result>0)            /* equivalent to previous set */
			result = 0;
	}
#endif
	திரும்பு result;
}

வெறுமை unset_local_var(மாறிலி எழுத்து *name)
{
	கட்டமைப்பு variables *cur;

	எனில் (name) {
		க்கு (cur = உச்சி_vars; cur; cur=cur->next) {
			எனில்(strcmp(cur->name, name)==0)
				நிறுத்து;
		}
		எனில் (cur != வெற்று) {
			கட்டமைப்பு variables *next = உச்சி_vars;
			எனில்(cur->flg_read_only) {
				error_செய்தி("%s: readonly variable", name);
				திரும்பு;
			} இல்லையெனில் {
#ifndef __U_BOOT__
				எனில்(cur->flg_export)
					unenv_set(cur->name);
#endif
				விடுவி(cur->name);
				விடுவி(cur->மதிப்பு);
				போது (next->next != cur)
					next = next->next;
				next->next = cur->next;
			}
			விடுவி(cur);
		}
	}
}

நிலையான முழு is_assignment(மாறிலி எழுத்து *s)
{
	எனில் (s == வெற்று)
		திரும்பு 0;

	எனில் (!isalpha(*s)) திரும்பு 0;
	++s;
	போது(isalnum(*s) || *s=='_') ++s;
	திரும்பு *s=='=';
}

#ifndef __U_BOOT__
/* the src parameter allows us to peek forward to a possible &n syntax
 * for file descriptor duplication, e.g., "2>&1".
 * Return code is 0 normally, 1 if a syntax error is detected in src.
 * Resource errors (in xmalloc) cause the process to exit */
நிலையான முழு setup_redirect(கட்டமைப்பு p_context *ctx, முழு fd, redir_type style,
	கட்டமைப்பு in_str *உள்ளீடு)
{
	கட்டமைப்பு child_prog *child=ctx->child;
	கட்டமைப்பு redir_struct *redir = child->redirects;
	கட்டமைப்பு redir_struct *last_redir=வெற்று;

	/* Create a new redir_struct and drop it onto the end of the linked list */
	போது(redir) {
		last_redir=redir;
		redir=redir->next;
	}
	redir = xmalloc(அளவு(கட்டமைப்பு redir_struct));
	redir->next=வெற்று;
	redir->word.gl_pathv=வெற்று;
	எனில் (last_redir) {
		last_redir->next=redir;
	} இல்லையெனில் {
		child->redirects=redir;
	}

	redir->type=style;
	redir->fd= (fd==-1) ? redir_table[style].default_fd : fd ;

	debug_printf("Redirect type %d%s\n", redir->fd, redir_table[style].descrip);

	/* Check for a '2>&1' type redirect */
	redir->dup = redirect_dup_இலக்கம்(உள்ளீடு);
	எனில் (redir->dup == -2) திரும்பு 1;  /* syntax error */
	எனில் (redir->dup != -1) {
		/* Erik had a check here that the file descriptor in question
		 * is legit; I postpone that to "run time"
		 * A "-" representation of "close me" shows up as a -3 here */
		debug_printf("Duplicating redirect '%d>&%d'\n", redir->fd, redir->dup);
	} இல்லையெனில் {
		/* We do _not_ try to open the file that src points to,
		 * since we need to return and let src be expanded first.
		 * Set ctx->pending_redirect, so we know what to do at the
		 * end of the next parsed word.
		 */
		ctx->pending_redirect = redir;
	}
	திரும்பு 0;
}
#endif

நிலையான கட்டமைப்பு pipe *new_pipe(வெறுமை)
{
	கட்டமைப்பு pipe *pi;
	pi = xmalloc(அளவு(கட்டமைப்பு pipe));
	pi->இலக்கம்_progs = 0;
	pi->progs = வெற்று;
	pi->next = வெற்று;
	pi->followup = 0;  /* invalid */
	pi->r_mode = RES_NONE;
	திரும்பு pi;
}

நிலையான வெறுமை initialize_context(கட்டமைப்பு p_context *ctx)
{
	ctx->pipe=வெற்று;
#ifndef __U_BOOT__
	ctx->pending_redirect=வெற்று;
#endif
	ctx->child=வெற்று;
	ctx->list_head=new_pipe();
	ctx->pipe=ctx->list_head;
	ctx->w=RES_NONE;
	ctx->அடுக்கு=வெற்று;
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
கட்டமைப்பு reserved_combo {
	எழுத்து *literal;
	முழு code;
	long flag;
};
/* Mostly a list of accepted follow-up reserved words.
 * FLAG_END means we are done with the sequence, and are ready
 * to turn the compound list into a command.
 * FLAG_START means the word must start a new compound list.
 */
நிலையான கட்டமைப்பு reserved_combo reserved_list[] = {
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
#define NRES (அளவு(reserved_list)/அளவு(கட்டமைப்பு reserved_combo))

நிலையான முழு reserved_word(o_string *dest, கட்டமைப்பு p_context *ctx)
{
	கட்டமைப்பு reserved_combo *r;
	க்கு (r=reserved_list;
		r<reserved_list+NRES; r++) {
		எனில் (strcmp(dest->data, r->literal) == 0) {
			debug_printf("found reserved word %s, code %d\n",r->literal,r->code);
			எனில் (r->flag & FLAG_START) {
				கட்டமைப்பு p_context *new = xmalloc(அளவு(கட்டமைப்பு p_context));
				debug_printf("push stack\n");
				எனில் (ctx->w == RES_IN || ctx->w == RES_FOR) {
					syntax();
					விடுவி(new);
					ctx->w = RES_SNTX;
					b_reset(dest);
					திரும்பு 1;
				}
				*new = *ctx;   /* physical copy */
				initialize_context(ctx);
				ctx->அடுக்கு=new;
			} இல்லையெனில் எனில் ( ctx->w == RES_NONE || ! (ctx->old_flag & (1<<r->code))) {
				syntax();
				ctx->w = RES_SNTX;
				b_reset(dest);
				திரும்பு 1;
			}
			ctx->w=r->code;
			ctx->old_flag = r->flag;
			எனில் (ctx->old_flag & FLAG_END) {
				கட்டமைப்பு p_context *old;
				debug_printf("pop stack\n");
				done_pipe(ctx,PIPE_SEQ);
				old = ctx->அடுக்கு;
				old->child->group = ctx->list_head;
#ifndef __U_BOOT__
				old->child->subshell = 0;
#endif
				*ctx = *old;   /* physical copy */
				விடுவி(old);
			}
			b_reset (dest);
			திரும்பு 1;
		}
	}
	திரும்பு 0;
}

/* normal return is 0.
 * Syntax or xglob errors return 1. */
நிலையான முழு done_word(o_string *dest, கட்டமைப்பு p_context *ctx)
{
	கட்டமைப்பு child_prog *child=ctx->child;
#ifndef __U_BOOT__
	glob_t *glob_target;
	முழு gr, flags = 0;
#இல்லையெனில்
	எழுத்து *str, *s;
	முழு argc, cnt;
#endif

	debug_printf("done_word: %s %p\n", dest->data, child);
	எனில் (dest->length == 0 && !dest->nonnull) {
		debug_printf("  true null, ignored\n");
		திரும்பு 0;
	}
#ifndef __U_BOOT__
	எனில் (ctx->pending_redirect) {
		glob_target = &ctx->pending_redirect->word;
	} இல்லையெனில் {
#endif
		எனில் (child->group) {
			syntax();
			திரும்பு 1;  /* syntax error, groups and arglists don't mix */
		}
		எனில் (!child->argv && (ctx->type & FLAG_PARSE_SEMICOLON)) {
			debug_printf("checking %s for reserved-ness\n",dest->data);
			எனில் (reserved_word(dest,ctx)) திரும்பு ctx->w==RES_SNTX;
		}
#ifndef __U_BOOT__
		glob_target = &child->glob_result;
		எனில் (child->argv) flags |= GLOB_APPEND;
#இல்லையெனில்
		க்கு (cnt = 1, s = dest->data; s && *s; s++) {
			எனில் (*s == '\\') s++;
			cnt++;
		}
		str = malloc(cnt);
		எனில் (!str) திரும்பு 1;
		எனில் ( child->argv == வெற்று) {
			child->argc=0;
		}
		argc = ++child->argc;
		child->argv = realloc(child->argv, (argc+1)*அளவு(*child->argv));
		எனில் (child->argv == வெற்று) {
			விடுவி(str);
			திரும்பு 1;
		}
		child->argv_nonnull = realloc(child->argv_nonnull,
					(argc+1)*அளவு(*child->argv_nonnull));
		எனில் (child->argv_nonnull == வெற்று) {
			விடுவி(str);
			திரும்பு 1;
		}
		child->argv[argc-1]=str;
		child->argv_nonnull[argc-1] = dest->nonnull;
		child->argv[argc]=வெற்று;
		child->argv_nonnull[argc] = 0;
		க்கு (s = dest->data; s && *s; s++,str++) {
			எனில் (*s == '\\') s++;
			*str = *s;
		}
		*str = '\0';
#endif
#ifndef __U_BOOT__
	}
	gr = xglob(dest, flags, glob_target);
	எனில் (gr != 0) திரும்பு 1;
#endif

	b_reset(dest);
#ifndef __U_BOOT__
	எனில் (ctx->pending_redirect) {
		ctx->pending_redirect=வெற்று;
		எனில் (glob_target->gl_pathc != 1) {
			error_செய்தி("ambiguous redirect");
			திரும்பு 1;
		}
	} இல்லையெனில் {
		child->argv = glob_target->gl_pathv;
	}
#endif
	எனில் (ctx->w == RES_FOR) {
		done_word(dest,ctx);
		done_pipe(ctx,PIPE_SEQ);
	}
	திரும்பு 0;
}

/* The only possible error here is out of memory, in which case
 * xmalloc exits. */
நிலையான முழு done_command(கட்டமைப்பு p_context *ctx)
{
	/* The child is really already in the pipe structure, so
	 * advance the pipe counter and make a new, null child.
	 * Only real trickiness here is that the uncommitted
	 * child structure, to which ctx->child points, is not
	 * counted in pi->num_progs. */
	கட்டமைப்பு pipe *pi=ctx->pipe;
	கட்டமைப்பு child_prog *prog=ctx->child;

	எனில் (prog && prog->group == வெற்று
		 && prog->argv == வெற்று
#ifndef __U_BOOT__
		 && prog->redirects == வெற்று) {
#இல்லையெனில்
										) {
#endif
		debug_printf("done_command: skipping null command\n");
		திரும்பு 0;
	} இல்லையெனில் எனில் (prog) {
		pi->இலக்கம்_progs++;
		debug_printf("done_command: num_progs incremented to %d\n",pi->இலக்கம்_progs);
	} இல்லையெனில் {
		debug_printf("done_command: initializing\n");
	}
	pi->progs = xrealloc(pi->progs, அளவு(*pi->progs) * (pi->இலக்கம்_progs+1));

	prog = pi->progs + pi->இலக்கம்_progs;
#ifndef __U_BOOT__
	prog->redirects = வெற்று;
#endif
	prog->argv = வெற்று;
	prog->argv_nonnull = வெற்று;
#ifndef __U_BOOT__
	prog->is_stopped = 0;
#endif
	prog->group = வெற்று;
#ifndef __U_BOOT__
	prog->glob_result.gl_pathv = வெற்று;
	prog->family = pi;
#endif
	prog->sp = 0;
	ctx->child = prog;
	prog->type = ctx->type;

	/* but ctx->pipe and ctx->list_head remain unchanged */
	திரும்பு 0;
}

நிலையான முழு done_pipe(கட்டமைப்பு p_context *ctx, pipe_style type)
{
	கட்டமைப்பு pipe *new_p;
	done_command(ctx);  /* implicit closure of previous command */
	debug_printf("done_pipe, type %d\n", type);
	ctx->pipe->followup = type;
	ctx->pipe->r_mode = ctx->w;
	new_p=new_pipe();
	ctx->pipe->next = new_p;
	ctx->pipe = new_p;
	ctx->child = வெற்று;
	done_command(ctx);  /* set up new pipe to accept commands */
	திரும்பு 0;
}

#ifndef __U_BOOT__
/* peek ahead in the in_str to find out if we have a "&n" construct,
 * as in "2>&1", that represents duplicating a file descriptor.
 * returns either -2 (syntax error), -1 (no &), or the number found.
 */
நிலையான முழு redirect_dup_இலக்கம்(கட்டமைப்பு in_str *உள்ளீடு)
{
	முழு ch, d=0, ok=0;
	ch = b_peek(உள்ளீடு);
	எனில் (ch != '&') திரும்பு -1;

	b_getch(உள்ளீடு);  /* get the & */
	ch=b_peek(உள்ளீடு);
	எனில் (ch == '-') {
		b_getch(உள்ளீடு);
		திரும்பு -3;  /* "-" represents "close me" */
	}
	போது (isdigit(ch)) {
		d = d*10+(ch-'0');
		ok=1;
		b_getch(உள்ளீடு);
		ch = b_peek(உள்ளீடு);
	}
	எனில் (ok) திரும்பு d;

	error_செய்தி("ambiguous redirect");
	திரும்பு -2;
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
நிலையான முழு redirect_opt_இலக்கம்(o_string *o)
{
	முழு இலக்கம்;

	எனில் (o->length==0) திரும்பு -1;
	க்கு(இலக்கம்=0; இலக்கம்<o->length; இலக்கம்++) {
		எனில் (!isdigit(*(o->data+இலக்கம்))) {
			திரும்பு -1;
		}
	}
	/* reuse num (and save an int) */
	இலக்கம்=atoi(o->data);
	b_reset(o);
	திரும்பு இலக்கம்;
}

FILE *generate_stream_from_list(கட்டமைப்பு pipe *head)
{
	FILE *pf;
#எனில் 1
	முழு pid, channel[2];
	எனில் (pipe(channel)<0) perror_செய்தி_and_die("pipe");
	pid=fork();
	எனில் (pid<0) {
		perror_செய்தி_and_die("fork");
	} இல்லையெனில் எனில் (pid==0) {
		close(channel[0]);
		எனில் (channel[1] != 1) {
			dup2(channel[1],1);
			close(channel[1]);
		}
#எனில் 0
#define SURROGATE "surrogate response"
		லிக(1,SURROGATE,அளவு(SURROGATE));
		_exit(run_list(head));
#இல்லையெனில்
		_exit(run_list_real(head));   /* leaks memory */
#endif
	}
	debug_printf("forked child %d\n",pid);
	close(channel[1]);
	pf = fdopen(channel[0],"r");
	debug_printf("pipe on FILE *%p\n",pf);
#இல்லையெனில்
	free_pipe_list(head,0);
	pf=popen("echo surrogate response","r");
	debug_printf("started fake pipe on FILE *%p\n",pf);
#endif
	திரும்பு pf;
}

/* this version hacked for testing purposes */
/* return code is exit status of the process that is run. */
நிலையான முழு process_command_subs(o_string *dest, கட்டமைப்பு p_context *ctx, கட்டமைப்பு in_str *உள்ளீடு, முழு subst_end)
{
	முழு retcode;
	o_string result=NULL_O_STRING;
	கட்டமைப்பு p_context inner;
	FILE *p;
	கட்டமைப்பு in_str pipe_str;
	initialize_context(&inner);

	/* recursion to generate command */
	retcode = parse_stream(&result, &inner, உள்ளீடு, subst_end);
	எனில் (retcode != 0) திரும்பு retcode;  /* syntax error or EOF */
	done_word(&result, &inner);
	done_pipe(&inner, PIPE_SEQ);
	b_free(&result);

	p=generate_stream_from_list(inner.list_head);
	எனில் (p==வெற்று) திரும்பு 1;
	mark_open(fileno(p));
	setup_file_in_str(&pipe_str, p);

	/* now send results of command back into original context */
	retcode = parse_stream(dest, ctx, &pipe_str, '\0');
	/* XXX In case of a syntax error, should we try to kill the child?
	 * That would be tough to do right, so just read until EOF. */
	எனில் (retcode == 1) {
		போது (b_getch(&pipe_str)!=EOF) { /* discard */ };
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
	திரும்பு retcode;
}

நிலையான முழு parse_group(o_string *dest, கட்டமைப்பு p_context *ctx,
	கட்டமைப்பு in_str *உள்ளீடு, முழு ch)
{
	முழு rcode, endch=0;
	கட்டமைப்பு p_context sub;
	கட்டமைப்பு child_prog *child = ctx->child;
	எனில் (child->argv) {
		syntax();
		திரும்பு 1;  /* syntax error, groups and arglists don't mix */
	}
	initialize_context(&sub);
	தேர்வு(ch) {
		நிலை '(': endch=')'; child->subshell=1; நிறுத்து;
		நிலை '{': endch='}'; நிறுத்து;
		இயல்பு: syntax();   /* really logic error */
	}
	rcode=parse_stream(dest,&sub,உள்ளீடு,endch);
	done_word(dest,&sub); /* finish off the final word in the subcontext */
	done_pipe(&sub, PIPE_SEQ);  /* and the final command there, too */
	child->group = sub.list_head;
	திரும்பு rcode;
	/* child remains "open", available for possible redirects */
}
#endif

/* basically useful version until someone wants to get fancier,
 * see the bash man page under "Parameter Expansion" */
நிலையான எழுத்து *lookup_param(எழுத்து *src)
{
	எழுத்து *p;
	எழுத்து *sep;
	எழுத்து *default_val = வெற்று;
	முழு assign = 0;
	முழு expand_empty = 0;

	எனில் (!src)
		திரும்பு வெற்று;

	sep = strchr(src, ':');

	எனில் (sep) {
		*sep = '\0';
		எனில் (*(sep + 1) == '-')
			default_val = sep+2;
		எனில் (*(sep + 1) == '=') {
			default_val = sep+2;
			assign = 1;
		}
		எனில் (*(sep + 1) == '+') {
			default_val = sep+2;
			expand_empty = 1;
		}
	}

	p = env_get(src);
	எனில் (!p)
		p = get_local_var(src);

	எனில் (!p || strlen(p) == 0) {
		p = default_val;
		எனில் (assign) {
			எழுத்து *var = malloc(strlen(src)+strlen(default_val)+2);
			எனில் (var) {
				sprintf(var, "%s=%s", src, default_val);
				set_local_var(var, 0);
			}
			விடுவி(var);
		}
	} இல்லையெனில் எனில் (expand_empty) {
		p += strlen(p);
	}

	எனில் (sep)
		*sep = ':';

	திரும்பு p;
}

#ifdef __U_BOOT__
நிலையான எழுத்து *get_dollar_var(எழுத்து ch)
{
	நிலையான எழுத்து buf[40];

	buf[0] = '\0';
	தேர்வு (ch) {
		நிலை '?':
			sprintf(buf, "%u", (unsigned முழு)last_return_code);
			நிறுத்து;
		இயல்பு:
			திரும்பு வெற்று;
	}
	திரும்பு buf;
}
#endif

/* return code: 0 for OK, 1 for syntax error */
நிலையான முழு handle_dollar(o_string *dest, கட்டமைப்பு p_context *ctx, கட்டமைப்பு in_str *உள்ளீடு)
{
#ifndef __U_BOOT__
	முழு i, advance=0;
#இல்லையெனில்
	முழு advance=0;
#endif
#ifndef __U_BOOT__
	எழுத்து sep[]=" ";
#endif
	முழு ch = உள்ளீடு->peek(உள்ளீடு);  /* first character after the $ */
	debug_printf("handle_dollar: ch=%c\n",ch);
	எனில் (isalpha(ch)) {
		b_addchr(dest, SPECIAL_VAR_குறியீடு);
		ctx->child->sp++;
		போது(ch=b_peek(உள்ளீடு),isalnum(ch) || ch=='_') {
			b_getch(உள்ளீடு);
			b_addchr(dest,ch);
		}
		b_addchr(dest, SPECIAL_VAR_குறியீடு);
#ifndef __U_BOOT__
	} இல்லையெனில் எனில் (isdigit(ch)) {
		i = ch-'0';  /* XXX is $0 special? */
		எனில் (i<global_argc) {
			parse_string(dest, ctx, global_argv[i]); /* recursion */
		}
		advance = 1;
#endif
	} இல்லையெனில் தேர்வு (ch) {
#ifndef __U_BOOT__
		நிலை '$':
			b_adduint(dest,getpid());
			advance = 1;
			நிறுத்து;
		நிலை '!':
			எனில் (last_bg_pid > 0) b_adduint(dest, last_bg_pid);
			advance = 1;
			நிறுத்து;
#endif
		நிலை '?':
#ifndef __U_BOOT__
			b_adduint(dest,last_return_code);
#இல்லையெனில்
			ctx->child->sp++;
			b_addchr(dest, SPECIAL_VAR_குறியீடு);
			b_addchr(dest, '$');
			b_addchr(dest, '?');
			b_addchr(dest, SPECIAL_VAR_குறியீடு);
#endif
			advance = 1;
			நிறுத்து;
#ifndef __U_BOOT__
		நிலை '#':
			b_adduint(dest,global_argc ? global_argc-1 : 0);
			advance = 1;
			நிறுத்து;
#endif
		நிலை '{':
			b_addchr(dest, SPECIAL_VAR_குறியீடு);
			ctx->child->sp++;
			b_getch(உள்ளீடு);
			/* XXX maybe someone will try to escape the '}' */
			போது(ch=b_getch(உள்ளீடு),ch!=EOF && ch!='}') {
				b_addchr(dest,ch);
			}
			எனில் (ch != '}') {
				syntax();
				திரும்பு 1;
			}
			b_addchr(dest, SPECIAL_VAR_குறியீடு);
			நிறுத்து;
#ifndef __U_BOOT__
		நிலை '(':
			b_getch(உள்ளீடு);
			process_command_subs(dest, ctx, உள்ளீடு, ')');
			நிறுத்து;
		நிலை '*':
			sep[0]=ifs[0];
			க்கு (i=1; i<global_argc; i++) {
				parse_string(dest, ctx, global_argv[i]);
				எனில் (i+1 < global_argc) parse_string(dest, ctx, sep);
			}
			நிறுத்து;
		நிலை '@':
		நிலை '-':
		நிலை '_':
			/* still unhandled, but should be eventually */
			error_செய்தி("unhandled syntax: $%c",ch);
			திரும்பு 1;
			நிறுத்து;
#endif
		இயல்பு:
			b_addqchr(dest,'$',dest->quote);
	}
	/* Eat the character if the flag was set.  If the compiler
	 * is smart enough, we could substitute "b_getch(input);"
	 * for all the "advance = 1;" above, and also end up with
	 * a nice size-optimized program.  Hah!  That'll be the day.
	 */
	எனில் (advance) b_getch(உள்ளீடு);
	திரும்பு 0;
}

#ifndef __U_BOOT__
முழு parse_string(o_string *dest, கட்டமைப்பு p_context *ctx, மாறிலி எழுத்து *src)
{
	கட்டமைப்பு in_str foo;
	setup_string_in_str(&foo, src);
	திரும்பு parse_stream(dest, ctx, &foo, '\0');
}
#endif

/* return code is 0 for normal exit, 1 for syntax error */
நிலையான முழு parse_stream(o_string *dest, கட்டமைப்பு p_context *ctx,
			கட்டமைப்பு in_str *உள்ளீடு, முழு end_trigger)
{
	unsigned முழு ch, m;
#ifndef __U_BOOT__
	முழு redir_fd;
	redir_type redir_style;
#endif
	முழு next;

	/* Only double-quote state is handled in the state variable dest->quote.
	 * A single-quote triggers a bypass of the main loop until its mate is
	 * found.  When recursing, quote state is passed in via dest->quote. */

	debug_printf("parse_stream, end_trigger=%d\n",end_trigger);
	போது ((ch=b_getch(உள்ளீடு))!=EOF) {
		m = map[ch];
#ifdef __U_BOOT__
		எனில் (உள்ளீடு->__promptme == 0) திரும்பு 1;
#endif
		next = (ch == '\n') ? 0 : b_peek(உள்ளீடு);

		debug_printf("parse_stream: ch=%c (%d) m=%d quote=%d - %c\n",
			ch >= ' ' ? ch : '.', ch, m,
			dest->quote, ctx->அடுக்கு == வெற்று ? '*' : '.');

		எனில் (m==0 || ((m==1 || m==2) && dest->quote)) {
			b_addqchr(dest, ch, dest->quote);
		} இல்லையெனில் {
			எனில் (m==2) {  /* unquoted IFS */
				எனில் (done_word(dest, ctx)) {
					திரும்பு 1;
				}
				/* If we aren't performing a substitution, treat a newline as a
				 * command separator.  */
				எனில் (end_trigger != '\0' && ch=='\n')
					done_pipe(ctx,PIPE_SEQ);
			}
			எனில் (ch == end_trigger && !dest->quote && ctx->w==RES_NONE) {
				debug_printf("leaving parse_stream (triggered)\n");
				திரும்பு 0;
			}
#எனில் 0
			எனில் (ch=='\n') {
				/* Yahoo!  Time to run with it! */
				done_pipe(ctx,PIPE_SEQ);
				run_list(ctx->list_head);
				initialize_context(ctx);
			}
#endif
			எனில் (m!=2) தேர்வு (ch) {
		நிலை '#':
			எனில் (dest->length == 0 && !dest->quote) {
				போது(ch=b_peek(உள்ளீடு),ch!=EOF && ch!='\n') { b_getch(உள்ளீடு); }
			} இல்லையெனில் {
				b_addqchr(dest, ch, dest->quote);
			}
			நிறுத்து;
		நிலை '\\':
			எனில் (next == EOF) {
				syntax();
				திரும்பு 1;
			}
			b_addqchr(dest, '\\', dest->quote);
			b_addqchr(dest, b_getch(உள்ளீடு), dest->quote);
			நிறுத்து;
		நிலை '$':
			எனில் (handle_dollar(dest, ctx, உள்ளீடு)!=0) திரும்பு 1;
			நிறுத்து;
		நிலை '\'':
			dest->nonnull = 1;
			போது(ch=b_getch(உள்ளீடு),ch!=EOF && ch!='\'') {
#ifdef __U_BOOT__
				எனில்(உள்ளீடு->__promptme == 0) திரும்பு 1;
#endif
				b_addchr(dest,ch);
			}
			எனில் (ch==EOF) {
				syntax();
				திரும்பு 1;
			}
			நிறுத்து;
		நிலை '"':
			dest->nonnull = 1;
			dest->quote = !dest->quote;
			நிறுத்து;
#ifndef __U_BOOT__
		நிலை '`':
			process_command_subs(dest, ctx, உள்ளீடு, '`');
			நிறுத்து;
		நிலை '>':
			redir_fd = redirect_opt_இலக்கம்(dest);
			done_word(dest, ctx);
			redir_style=REDIRECT_OVERWRITE;
			எனில் (next == '>') {
				redir_style=REDIRECT_APPEND;
				b_getch(உள்ளீடு);
			} இல்லையெனில் எனில் (next == '(') {
				syntax();   /* until we support >(list) Process Substitution */
				திரும்பு 1;
			}
			setup_redirect(ctx, redir_fd, redir_style, உள்ளீடு);
			நிறுத்து;
		நிலை '<':
			redir_fd = redirect_opt_இலக்கம்(dest);
			done_word(dest, ctx);
			redir_style=REDIRECT_உள்ளீடு;
			எனில் (next == '<') {
				redir_style=REDIRECT_HEREIS;
				b_getch(உள்ளீடு);
			} இல்லையெனில் எனில் (next == '>') {
				redir_style=REDIRECT_IO;
				b_getch(உள்ளீடு);
			} இல்லையெனில் எனில் (next == '(') {
				syntax();   /* until we support <(list) Process Substitution */
				திரும்பு 1;
			}
			setup_redirect(ctx, redir_fd, redir_style, உள்ளீடு);
			நிறுத்து;
#endif
		நிலை ';':
			done_word(dest, ctx);
			done_pipe(ctx,PIPE_SEQ);
			நிறுத்து;
		நிலை '&':
			done_word(dest, ctx);
			எனில் (next=='&') {
				b_getch(உள்ளீடு);
				done_pipe(ctx,PIPE_AND);
			} இல்லையெனில் {
#ifndef __U_BOOT__
				done_pipe(ctx,PIPE_BG);
#இல்லையெனில்
				syntax_err();
				திரும்பு 1;
#endif
			}
			நிறுத்து;
		நிலை '|':
			done_word(dest, ctx);
			எனில் (next=='|') {
				b_getch(உள்ளீடு);
				done_pipe(ctx,PIPE_OR);
			} இல்லையெனில் {
				/* we could pick up a file descriptor choice here
				 * with redirect_opt_num(), but bash doesn't do it.
				 * "echo foo 2| cat" yields "foo 2". */
#ifndef __U_BOOT__
				done_command(ctx);
#இல்லையெனில்
				syntax_err();
				திரும்பு 1;
#endif
			}
			நிறுத்து;
#ifndef __U_BOOT__
		நிலை '(':
		நிலை '{':
			எனில் (parse_group(dest, ctx, உள்ளீடு, ch)!=0) திரும்பு 1;
			நிறுத்து;
		நிலை ')':
		நிலை '}':
			syntax();   /* Proper use of this character caught by end_trigger */
			திரும்பு 1;
			நிறுத்து;
#endif
		நிலை SUBSTED_VAR_குறியீடு:
			dest->nonnull = 1;
			போது (ch = b_getch(உள்ளீடு), ch != EOF &&
			    ch != SUBSTED_VAR_குறியீடு) {
				debug_printf("subst, pass=%d\n", ch);
				எனில் (உள்ளீடு->__promptme == 0)
					திரும்பு 1;
				b_addchr(dest, ch);
			}
			debug_printf("subst, term=%d\n", ch);
			எனில் (ch == EOF) {
				syntax();
				திரும்பு 1;
			}
			நிறுத்து;
		இயல்பு:
			syntax();   /* this is really an internal logic error */
			திரும்பு 1;
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
	எனில் (end_trigger != '\0') திரும்பு -1;
	திரும்பு 0;
}

நிலையான வெறுமை mapset(மாறிலி unsigned எழுத்து *set, முழு code)
{
	மாறிலி unsigned எழுத்து *s;
	க்கு (s=set; *s; s++) map[*s] = code;
}

நிலையான வெறுமை update_ifs_map(வெறுமை)
{
	/* char *ifs and char map[256] are both globals. */
	ifs = (uchar *)env_get("IFS");
	எனில் (ifs == வெற்று) ifs=(uchar *)" \t\n";
	/* Precompute a list of 'flow through' behavior so it can be treated
	 * quickly up front.  Computation is necessary because of IFS.
	 * Special case handling of IFS == " \t\n" is not implemented.
	 * The map[] array only really needs two bits each, and on most machines
	 * that would be faster because of the reduced L1 cache footprint.
	 */
	memset(map,0,அளவு(map)); /* most characters flow through always */
#ifndef __U_BOOT__
	mapset((uchar *)"\\$'\"`", 3);      /* never flow through */
	mapset((uchar *)"<>;&|(){}#", 1);   /* flow through if quoted */
#இல்லையெனில்
	{
		uchar subst[2] = {SUBSTED_VAR_குறியீடு, 0};
		mapset(subst, 3);       /* never flow through */
	}
	mapset((uchar *)"\\$'\"", 3);       /* never flow through */
	mapset((uchar *)";&|#", 1);         /* flow through if quoted */
#endif
	mapset(ifs, 2);            /* also flow through if quoted */
}

/* most recursion does not come through here, the exeception is
 * from builtin_source() */
நிலையான முழு parse_stream_outer(கட்டமைப்பு in_str *inp, முழு flag)
{

	கட்டமைப்பு p_context ctx;
	o_string temp=NULL_O_STRING;
	முழு rcode;
#ifdef __U_BOOT__
	முழு code = 1;
#endif
	செய் {
		ctx.type = flag;
		initialize_context(&ctx);
		update_ifs_map();
		எனில் (!(flag & FLAG_PARSE_SEMICOLON) || (flag & FLAG_REPARSING)) mapset((uchar *)";$&|", 0);
		inp->promptmode=1;
		rcode = parse_stream(&temp, &ctx, inp,
				     flag & FLAG_CONT_ON_NEWLINE ? -1 : '\n');
#ifdef __U_BOOT__
		எனில் (rcode == 1) flag_repeat = 0;
#endif
		எனில் (rcode != 1 && ctx.old_flag != 0) {
			syntax();
#ifdef __U_BOOT__
			flag_repeat = 0;
#endif
		}
		எனில் (rcode != 1 && ctx.old_flag == 0) {
			done_word(&temp, &ctx);
			done_pipe(&ctx,PIPE_SEQ);
#ifndef __U_BOOT__
			run_list(ctx.list_head);
#இல்லையெனில்
			code = run_list(ctx.list_head);
			எனில் (code == -2) {	/* exit */
				b_free(&temp);
				code = 0;
				/* XXX hackish way to not allow exit from main loop */
				எனில் (inp->peek == file_peek) {
					printf("exit not allowed from main input shell.\n");
					தொடர்;
				}
				/*
				 * DANGER
				 * Return code -2 is special in this context,
				 * it indicates exit from inner pipe instead
				 * of return code itself, the return code is
				 * stored in 'last_return_code' variable!
				 * DANGER
				 */
				திரும்பு -2;
			}
			எனில் (code == -1)
			    flag_repeat = 0;
#endif
		} இல்லையெனில் {
			எனில் (ctx.old_flag != 0) {
				விடுவி(ctx.அடுக்கு);
				b_reset(&temp);
			}
#ifdef __U_BOOT__
			எனில் (inp->__promptme == 0) printf("<INTERRUPT>\n");
			inp->__promptme = 1;
#endif
			temp.nonnull = 0;
			temp.quote = 0;
			inp->p = வெற்று;
			free_pipe_list(ctx.list_head,0);
		}
		b_free(&temp);
	/* loop on syntax errors, return on EOF */
	} போது (rcode != -1 && !(flag & FLAG_EXIT_FROM_LOOP) &&
		(inp->peek != static_peek || b_peek(inp)));
#ifndef __U_BOOT__
	திரும்பு 0;
#இல்லையெனில்
	திரும்பு (code != 0) ? 1 : 0;
#endif /* __U_BOOT__ */
}

#ifndef __U_BOOT__
நிலையான முழு parse_string_outer(மாறிலி எழுத்து *s, முழு flag)
#இல்லையெனில்
முழு parse_string_outer(மாறிலி எழுத்து *s, முழு flag)
#endif	/* __U_BOOT__ */
{
	கட்டமைப்பு in_str உள்ளீடு;
	முழு rcode;
#ifdef __U_BOOT__
	எழுத்து *p = வெற்று;
	எனில் (!s)
		திரும்பு 1;
	எனில் (!*s)
		திரும்பு 0;
	எனில் (!(p = strchr(s, '\n')) || *++p) {
		p = xmalloc(strlen(s) + 2);
		strcpy(p, s);
		strcat(p, "\n");
		setup_string_in_str(&உள்ளீடு, p);
		rcode = parse_stream_outer(&உள்ளீடு, flag);
		விடுவி(p);
		திரும்பு rcode == -2 ? last_return_code : rcode;
	} இல்லையெனில் {
#endif
	setup_string_in_str(&உள்ளீடு, s);
	rcode = parse_stream_outer(&உள்ளீடு, flag);
	திரும்பு rcode == -2 ? last_return_code : rcode;
#ifdef __U_BOOT__
	}
#endif
}

#ifndef __U_BOOT__
நிலையான முழு parse_file_outer(FILE *f)
#இல்லையெனில்
முழு parse_file_outer(வெறுமை)
#endif
{
	முழு rcode;
	கட்டமைப்பு in_str உள்ளீடு;
#ifndef __U_BOOT__
	setup_file_in_str(&உள்ளீடு, f);
#இல்லையெனில்
	setup_file_in_str(&உள்ளீடு);
#endif
	rcode = parse_stream_outer(&உள்ளீடு, FLAG_PARSE_SEMICOLON);
	திரும்பு rcode == -2 ? last_return_code : rcode;
}

#ifdef __U_BOOT__
முழு u_boot_hush_start(வெறுமை)
{
	எனில் (உச்சி_vars == வெற்று) {
		உச்சி_vars = malloc(அளவு(கட்டமைப்பு variables));
		உச்சி_vars->name = "HUSH_VERSION";
		உச்சி_vars->மதிப்பு = "0.01";
		உச்சி_vars->next = வெற்று;
		உச்சி_vars->flg_export = 0;
		உச்சி_vars->flg_read_only = 1;
	}
	திரும்பு 0;
}

நிலையான வெறுமை *xmalloc(size_t size)
{
	வெறுமை *p = வெற்று;

	எனில் (!(p = malloc(size))) {
	    printf("ERROR : xmalloc failed\n");
	    க்கு(;;);
	}
	திரும்பு p;
}

நிலையான வெறுமை *xrealloc(வெறுமை *ptr, size_t size)
{
	வெறுமை *p = வெற்று;

	எனில் (!(p = realloc(ptr, size))) {
	    printf("ERROR : xrealloc failed\n");
	    க்கு(;;);
	}
	திரும்பு p;
}
#endif /* __U_BOOT__ */

#ifndef __U_BOOT__
/* Make sure we have a controlling tty.  If we get started under a job
 * aware app (like bash for example), make sure we are now in charge so
 * we don't fight over who gets the foreground */
நிலையான வெறுமை setup_job_control(வெறுமை)
{
	நிலையான pid_t shell_pgrp;
	/* Loop until we are in the foreground.  */
	போது (tcgetpgrp (shell_terminal) != (shell_pgrp = getpgrp ()))
		kill (- shell_pgrp, SIGTTIN);

	/* Ignore interactive and job-control signals.  */
	சமிக்ஞை(SIGINT, SIG_IGN);
	சமிக்ஞை(SIGQUIT, SIG_IGN);
	சமிக்ஞை(SIGTERM, SIG_IGN);
	சமிக்ஞை(SIGTSTP, SIG_IGN);
	சமிக்ஞை(SIGTTIN, SIG_IGN);
	சமிக்ஞை(SIGTTOU, SIG_IGN);
	சமிக்ஞை(SIGCHLD, SIG_IGN);

	/* Put ourselves in our own process group.  */
	setsid();
	shell_pgrp = getpid ();
	setpgid (shell_pgrp, shell_pgrp);

	/* Grab control of the terminal.  */
	tcsetpgrp(shell_terminal, shell_pgrp);
}

முழு hush_main(முழு argc, எழுத்து * மாறிலி *argv)
{
	முழு opt;
	FILE *உள்ளீடு;
	எழுத்து **e = environ;

	/* XXX what should these be while sourcing /etc/profile? */
	global_argc = argc;
	global_argv = argv;

	/* (re?) initialize globals.  Sometimes hush_main() ends up calling
	 * hush_main(), therefore we cannot rely on the BSS to zero out this
	 * stuff.  Reset these to 0 every time. */
	ifs = வெற்று;
	/* map[] is taken care of with call to update_ifs_map() */
	fake_mode = 0;
	interactive = 0;
	close_me_head = வெற்று;
	last_bg_pid = 0;
	job_list = வெற்று;
	last_jobid = 0;

	/* Initialize some more globals to non-zero values */
	set_cwd();
#ifdef CONFIG_FEATURE_COMMAND_EDITING
	cmdedit_set_initial_prompt();
#இல்லையெனில்
	PS1 = வெற்று;
#endif
	PS2 = "> ";

	/* initialize our shell local variables with the values
	 * currently living in the environment */
	எனில் (e) {
		க்கு (; *e; e++)
			set_local_var(*e, 2);   /* without call putenv() */
	}

	last_return_code=EXIT_SUCCESS;

	எனில் (argv[0] && argv[0][0] == '-') {
		debug_printf("\nsourcing /etc/profile\n");
		எனில் ((உள்ளீடு = fopen("/etc/profile", "r")) != வெற்று) {
			mark_open(fileno(உள்ளீடு));
			parse_file_outer(உள்ளீடு);
			mark_closed(fileno(உள்ளீடு));
			fclose(உள்ளீடு);
		}
	}
	உள்ளீடு=stdin;

	போது ((opt = getopt(argc, argv, "c:xif")) > 0) {
		தேர்வு (opt) {
			நிலை 'c':
				{
					global_argv = argv+optind;
					global_argc = argc-optind;
					opt = parse_string_outer(optarg, FLAG_PARSE_SEMICOLON);
					செல் final_return;
				}
				நிறுத்து;
			நிலை 'i':
				interactive++;
				நிறுத்து;
			நிலை 'f':
				fake_mode++;
				நிறுத்து;
			இயல்பு:
#ifndef BB_VER
				fprintf(stderr, "Usage: sh [FILE]...\n"
						"   or: sh -c command [args]...\n\n");
				exit(EXIT_FAILURE);
#இல்லையெனில்
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
	எனில் (argv[optind]==வெற்று && உள்ளீடு==stdin &&
			isatty(fileno(stdin)) && isatty(fileno(stdout))) {
		interactive++;
	}

	debug_printf("\ninteractive=%d\n", interactive);
	எனில் (interactive) {
		/* Looks like they want an interactive shell */
#ifndef CONFIG_FEATURE_SH_EXTRA_QUIET
		printf( "\n\n" BB_BANNER " hush - the humble shell v0.01 (testing)\n");
		printf( "Enter 'help' for a list of built-in commands.\n\n");
#endif
		setup_job_control();
	}

	எனில் (argv[optind]==வெற்று) {
		opt=parse_file_outer(stdin);
		செல் final_return;
	}

	debug_printf("\nrunning script '%s'\n", argv[optind]);
	global_argv = argv+optind;
	global_argc = argc-optind;
	உள்ளீடு = xfopen(argv[optind], "r");
	opt = parse_file_outer(உள்ளீடு);

#ifdef CONFIG_FEATURE_CLEAN_UP
	fclose(உள்ளீடு);
	எனில் (cwd && cwd != unknown)
		விடுவி((எழுத்து*)cwd);
	{
		கட்டமைப்பு variables *cur, *tmp;
		க்கு(cur = உச்சி_vars; cur; cur = tmp) {
			tmp = cur->next;
			எனில் (!cur->flg_read_only) {
				விடுவி(cur->name);
				விடுவி(cur->மதிப்பு);
				விடுவி(cur);
			}
		}
	}
#endif

final_return:
	திரும்பு(opt?opt:last_return_code);
}
#endif

நிலையான எழுத்து *செருகு_var_மதிப்பு(எழுத்து *inp)
{
	திரும்பு செருகு_var_மதிப்பு_sub(inp, 0);
}

நிலையான எழுத்து *செருகு_var_மதிப்பு_sub(எழுத்து *inp, முழு tag_subst)
{
	முழு res_str_len = 0;
	முழு len;
	முழு done = 0;
	எழுத்து *p, *p1, *res_str = வெற்று;

	போது ((p = strchr(inp, SPECIAL_VAR_குறியீடு))) {
		/* check the beginning of the string for normal characters */
		எனில் (p != inp) {
			/* copy any characters to the result string */
			len = p - inp;
			res_str = xrealloc(res_str, (res_str_len + len));
			strncpy((res_str + res_str_len), inp, len);
			res_str_len += len;
		}
		inp = ++p;
		/* find the ending marker */
		p = strchr(inp, SPECIAL_VAR_குறியீடு);
		*p = '\0';
		/* look up the value to substitute */
		எனில் ((p1 = lookup_param(inp))) {
			எனில் (tag_subst)
				len = res_str_len + strlen(p1) + 2;
			இல்லையெனில்
				len = res_str_len + strlen(p1);
			res_str = xrealloc(res_str, (1 + len));
			எனில் (tag_subst) {
				/*
				 * copy the variable value to the result
				 * string
				 */
				strcpy((res_str + res_str_len + 1), p1);

				/*
				 * mark the replaced text to be accepted as
				 * is
				 */
				res_str[res_str_len] = SUBSTED_VAR_குறியீடு;
				res_str[res_str_len + 1 + strlen(p1)] =
					SUBSTED_VAR_குறியீடு;
			} இல்லையெனில்
				/*
				 * copy the variable value to the result
				 * string
				 */
				strcpy((res_str + res_str_len), p1);

			res_str_len = len;
		}
		*p = SPECIAL_VAR_குறியீடு;
		inp = ++p;
		done = 1;
	}
	எனில் (done) {
		res_str = xrealloc(res_str, (1 + res_str_len + strlen(inp)));
		strcpy((res_str + res_str_len), inp);
		போது ((p = strchr(res_str, '\n'))) {
			*p = ' ';
		}
	}
	திரும்பு (res_str == வெற்று) ? inp : res_str;
}

நிலையான எழுத்து **make_list_in(எழுத்து **inp, எழுத்து *name)
{
	முழு len, i;
	முழு name_len = strlen(name);
	முழு n = 0;
	எழுத்து **list;
	எழுத்து *p1, *p2, *p3;

	/* create list of variable values */
	list = xmalloc(அளவு(*list));
	க்கு (i = 0; inp[i]; i++) {
		p3 = செருகு_var_மதிப்பு(inp[i]);
		p1 = p3;
		போது (*p1) {
			எனில் (*p1 == ' ') {
				p1++;
				தொடர்;
			}
			எனில் ((p2 = strchr(p1, ' '))) {
				len = p2 - p1;
			} இல்லையெனில் {
				len = strlen(p1);
				p2 = p1 + len;
			}
			/* we use n + 2 in realloc for list,because we add
			 * new element and then we will add NULL element */
			list = xrealloc(list, அளவு(*list) * (n + 2));
			list[n] = xmalloc(2 + name_len + len);
			strcpy(list[n], name);
			strcat(list[n], "=");
			strncat(list[n], p1, len);
			list[n++][name_len + len + 1] = '\0';
			p1 = p2;
		}
		எனில் (p3 != inp[i]) விடுவி(p3);
	}
	list[n] = வெற்று;
	திரும்பு list;
}

/*
 * Make new string for parser
 * inp     - array of argument strings to flatten
 * nonnull - indicates argument was quoted when originally parsed
 */
நிலையான எழுத்து *make_string(எழுத்து **inp, முழு *nonnull)
{
	எழுத்து *p;
	எழுத்து *str = வெற்று;
	முழு n;
	முழு len = 2;
	எழுத்து *noeval_str;
	முழு noeval = 0;

	noeval_str = get_local_var("HUSH_NO_EVAL");
	எனில் (noeval_str != வெற்று && *noeval_str != '0' && *noeval_str != '\0')
		noeval = 1;
	க்கு (n = 0; inp[n]; n++) {
		p = செருகு_var_மதிப்பு_sub(inp[n], noeval);
		எழுத்து *new_str = xrealloc(str, (len + strlen(p) + (2 * nonnull[n])));
		எனில் (!new_str) {
			விடுவி(str);
			எனில் (p != inp[n]) விடுவி(p);
			திரும்பு வெற்று;
		}
		str = new_str;
		எனில் (n) {
			strcat(str, " ");
		} இல்லையெனில் {
			*str = '\0';
		}
		எனில் (nonnull[n])
			strcat(str, "'");
		strcat(str, p);
		எனில் (nonnull[n])
			strcat(str, "'");
		len = strlen(str) + 3;
		எனில் (p != inp[n]) விடுவி(p);
	}
	len = strlen(str);
	*(str + len) = '\n';
	*(str + len + 1) = '\0';
	திரும்பு str;
}

#ifdef __U_BOOT__
நிலையான முழு do_showvar(கட்டமைப்பு cmd_tbl *cmdtp, முழு flag, முழு argc,
		      எழுத்து *மாறிலி argv[])
{
	முழு i, k;
	முழு rcode = 0;
	கட்டமைப்பு variables *cur;

	எனில் (argc == 1) {		/* Print all env variables	*/
		க்கு (cur = உச்சி_vars; cur; cur = cur->next) {
			printf ("%s=%s\n", cur->name, cur->மதிப்பு);
			எனில் (ctrlc ()) {
				puts ("\n ** Abort\n");
				திரும்பு 1;
			}
		}
		திரும்பு 0;
	}
	க்கு (i = 1; i < argc; ++i) {	/* print single env variables	*/
		எழுத்து *name = argv[i];

		k = -1;
		க்கு (cur = உச்சி_vars; cur; cur = cur->next) {
			எனில்(strcmp (cur->name, name) == 0) {
				k = 0;
				printf ("%s=%s\n", cur->name, cur->மதிப்பு);
			}
			எனில் (ctrlc ()) {
				puts ("\n ** Abort\n");
				திரும்பு 1;
			}
		}
		எனில் (k < 0) {
			printf ("## Error: \"%s\" not defined\n", name);
			rcode ++;
		}
	}
	திரும்பு rcode;
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
