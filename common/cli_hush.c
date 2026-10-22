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
#अन्तर्भाव <env.h>
#अन्तर्भाव <malloc.h>         /* malloc, free, realloc*/
#अन्तर्भाव <linux/ctype.h>    /* isalpha, isdigit */
#अन्तर्भाव <console.h>
#अन्तर्भाव <bootretry.h>
#अन्तर्भाव <cli.h>
#अन्तर्भाव <cli_hush.h>
#अन्तर्भाव <command.h>        /* find_cmd */
#अन्तर्भाव <vsprintf.h>
#endif
#ifndef __U_BOOT__
#अन्तर्भाव <ctype.h>     /* isalpha, isdigit */
#अन्तर्भाव <unistd.h>    /* getpid */
#अन्तर्भाव <stdlib.h>    /* getenv, atoi */
#अन्तर्भाव <string.h>    /* strchr */
#अन्तर्भाव <stdio.h>     /* popen etc. */
#अन्तर्भाव <glob.h>      /* glob, of course */
#अन्तर्भाव <stdarg.h>    /* va_list */
#अन्तर्भाव <errno.h>
#अन्तर्भाव <fcntl.h>
#अन्तर्भाव <getopt.h>    /* should be pretty obvious */

#अन्तर्भाव <sys/stat.h>  /* ulimit */
#अन्तर्भाव <sys/types.h>
#अन्तर्भाव <sys/wait.h>
#अन्तर्भाव <signal.h>

/* #include <dmalloc.h> */

#जर 1
#अन्तर्भाव "busybox.h"
#अन्तर्भाव "cmdedit.h"
#अन्यथा
#define applet_name "hush"
#अन्तर्भाव "standalone.h"
#define hush_main main
#define BB_BANNER
#endif
#endif
#define SPECIAL_VAR_चिन्ह 03
#define SUBSTED_VAR_चिन्ह 04
#ifndef __U_BOOT__
#define FLAG_EXIT_FROM_LOOP 1
#define FLAG_PARSE_SEMICOLON (1 << 1)		/* symbol ';' is special for parser */
#define FLAG_REPARSING       (1 << 2)		/* >= 2nd pass */

#endif

#ifdef __U_BOOT__
#define निकास_सफल 0
#define खातापूर्ण -1
#define syntax() syntax_err()
#define xstrdup strdup
#define error_संदेश म_लिखो
#अन्यथा
प्रकारव्याख्या गणना {
	REDIRECT_आदान     = 1,
	REDIRECT_OVERWRITE = 2,
	REDIRECT_APPEND    = 3,
	REDIRECT_HEREIS    = 4,
	REDIRECT_IO        = 5
} redir_type;

/* The descrip member of this structure is only used to make debugging
 * output pretty */
संरचना {पूर्णांक mode; पूर्णांक default_fd; अक्षर *descrip;} redir_table[] = {
	{ 0,                         0, "()" },
	{ O_RDONLY,                  0, "<"  },
	{ O_CREAT|O_TRUNC|O_WRONLY,  1, ">"  },
	{ O_CREAT|O_APPEND|O_WRONLY, 1, ">>" },
	{ O_RDONLY,                 -1, "<<" },
	{ O_RDWR,                    1, "<>" }
};
#endif

प्रकारव्याख्या गणना {
	PIPE_SEQ = 1,
	PIPE_AND = 2,
	PIPE_OR  = 3,
	PIPE_BG  = 4,
} pipe_style;

/* might eventually control execution */
प्रकारव्याख्या गणना {
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
संरचना p_context {
	संरचना child_prog *child;
	संरचना pipe *list_head;
	संरचना pipe *pipe;
#ifndef __U_BOOT__
	संरचना redir_struct *pending_redirect;
#endif
	reserved_style w;
	पूर्णांक old_flag;				/* for figuring out valid reserved words */
	संरचना p_context *चळत;
	पूर्णांक type;			/* define type of parser : ";$" common or special symbol */
	/* How about quoting status? */
};

#ifndef __U_BOOT__
संरचना redir_struct {
	redir_type type;			/* type of redirection */
	पूर्णांक fd;						/* file descriptor being redirected */
	पूर्णांक dup;					/* -1, or file descriptor being duplicated */
	संरचना redir_struct *next;	/* pointer to the next redirect in the list */
	glob_t word;				/* *word.gl_pathv is the filename */
};
#endif

संरचना child_prog {
#ifndef __U_BOOT__
	pid_t pid;					/* 0 if exited */
#endif
	अक्षर **argv;				/* program name and arguments */
	/* was quoted when parsed; copy of struct o_string.nonnull field */
	पूर्णांक *argv_nonnull;
#ifdef __U_BOOT__
	पूर्णांक    argc;                            /* number of program arguments */
#endif
	संरचना pipe *group;			/* if non-NULL, first in group or subshell */
#ifndef __U_BOOT__
	पूर्णांक subshell;				/* flag, non-zero if group must be forked */
	संरचना redir_struct *redirects;	/* I/O redirections */
	glob_t glob_result;			/* result of parameter globbing */
	पूर्णांक is_stopped;				/* is the program currently running? */
	संरचना pipe *family;		/* pointer back to the child's parent pipe */
#endif
	पूर्णांक sp;				/* number of SPECIAL_VAR_SYMBOL */
	पूर्णांक type;
};

संरचना pipe {
#ifndef __U_BOOT__
	पूर्णांक jobid;					/* job number */
#endif
	पूर्णांक अंक_progs;				/* total number of programs in job */
#ifndef __U_BOOT__
	पूर्णांक running_progs;			/* number of programs running */
	अक्षर *text;					/* name of job */
	अक्षर *cmdbuf;				/* buffer various argv's point into */
	pid_t pgrp;					/* process group ID for the job */
#endif
	संरचना child_prog *progs;	/* array of commands in pipe */
	संरचना pipe *next;			/* to track background commands */
#ifndef __U_BOOT__
	पूर्णांक stopped_progs;			/* number of programs alive, but stopped */
	पूर्णांक job_context;			/* bitmask defining current context */
#endif
	pipe_style followup;		/* PIPE_BG, PIPE_SEQ, PIPE_OR, PIPE_AND */
	reserved_style r_mode;		/* supports if, for, while, until */
};

#ifndef __U_BOOT__
संरचना close_me {
	पूर्णांक fd;
	संरचना close_me *next;
};
#endif

संरचना variables {
	अक्षर *name;
	अक्षर *मूल्य;
	पूर्णांक flg_export;
	पूर्णांक flg_read_only;
	संरचना variables *next;
};

/* globals, connect us to the outside world
 * the first three support $?, $#, and $1 */
#ifndef __U_BOOT__
अक्षर **global_argv;
अचिन्हित पूर्णांक global_argc;
#endif
स्थिर अचिन्हित पूर्णांक last_return_code;
#ifndef __U_BOOT__
बाह्य अक्षर **environ; /* This is in <unistd.h>, but protected with __USE_GNU */
#endif

/* "globals" within this file */
स्थिर uchar *ifs;
स्थिर अक्षर map[256];
#ifndef __U_BOOT__
स्थिर पूर्णांक fake_mode;
स्थिर पूर्णांक interactive;
स्थिर संरचना close_me *close_me_head;
स्थिर स्थिरांक अक्षर *cwd;
स्थिर संरचना pipe *job_list;
स्थिर अचिन्हित पूर्णांक last_bg_pid;
स्थिर अचिन्हित पूर्णांक last_jobid;
स्थिर अचिन्हित पूर्णांक shell_terminal;
स्थिर अक्षर *PS1;
स्थिर अक्षर *PS2;
संरचना variables shell_ver = { "HUSH_VERSION", "0.01", 1, 1, 0 };
संरचना variables *शीर्ष_vars = &shell_ver;
#अन्यथा
स्थिर पूर्णांक flag_repeat = 0;
स्थिर पूर्णांक do_repeat = 0;
स्थिर संरचना variables *शीर्ष_vars = रिक्त ;
#endif /*__U_BOOT__ */

#define B_CHUNK (100)
#define B_NOSPAC 1

प्रकारव्याख्या संरचना {
	अक्षर *data;
	पूर्णांक length;
	पूर्णांक maxlen;
	पूर्णांक quote;
	पूर्णांक nonnull;
} o_string;
#define NULL_O_STRING {रिक्त,0,0,0,0}
/* used for initialization:
	o_string foo = NULL_O_STRING; */

/* I can almost use ordinary FILE *.  Is open_memstream() universally
 * available?  Where is it documented? */
संरचना in_str {
	स्थिरांक अचिन्हित अक्षर *p;
#ifndef __U_BOOT__
	अक्षर peek_buf[2];
#endif
	पूर्णांक __promptme;
	पूर्णांक promptmode;
#ifndef __U_BOOT__
	खाता *file;
#endif
	पूर्णांक (*get) (संरचना in_str *);
	पूर्णांक (*peek) (संरचना in_str *);
};
#define b_getch(आदान) ((आदान)->get(आदान))
#define b_peek(आदान) ((आदान)->peek(आदान))

#ifndef __U_BOOT__
#define JOB_STATUS_FORMAT "[%d] %-22s %.40s\n"

संरचना built_in_command {
	अक्षर *cmd;					/* name */
	अक्षर *descr;				/* description */
	पूर्णांक (*function) (संरचना child_prog *);	/* function ptr */
};
#endif

/* define DEBUG_SHELL for debugging output (obviously ;-)) */
#जर 0
#define DEBUG_SHELL
#endif

/* This should be in utility.c */
#ifdef DEBUG_SHELL
#ifndef __U_BOOT__
स्थिर शून्य debug_printf(स्थिरांक अक्षर *format, ...)
{
	बहु_सूची args;
	बहु_शुरू(args, format);
	भख_लिखो(मानक_त्रुटि, format, args);
	बहु_पूर्ण(args);
}
#अन्यथा
#define debug_printf(fmt,args...)	म_लिखो (fmt ,##args)
#endif
#अन्यथा
स्थिर ओळीत शून्य debug_printf(स्थिरांक अक्षर *format, ...) { }
#endif
#define final_printf debug_printf

#ifdef __U_BOOT__
स्थिर शून्य syntax_err(शून्य) {
	 म_लिखो("syntax error\n");
}
#अन्यथा
स्थिर शून्य __syntax(अक्षर *file, पूर्णांक ओळ) {
	error_संदेश("syntax error %s:%d", file, ओळ);
}
#define syntax() __syntax(__FILE__, __LINE__)
#endif

#ifdef __U_BOOT__
स्थिर शून्य *xmalloc(माप_प्रकार size);
स्थिर शून्य *xrealloc(शून्य *ptr, माप_प्रकार size);
#अन्यथा
/* Index of subroutines: */
/*   function prototypes for builtins */
स्थिर पूर्णांक builtin_cd(संरचना child_prog *child);
स्थिर पूर्णांक builtin_env(संरचना child_prog *child);
स्थिर पूर्णांक builtin_eval(संरचना child_prog *child);
स्थिर पूर्णांक builtin_exec(संरचना child_prog *child);
स्थिर पूर्णांक builtin_exit(संरचना child_prog *child);
स्थिर पूर्णांक builtin_export(संरचना child_prog *child);
स्थिर पूर्णांक builtin_fg_bg(संरचना child_prog *child);
स्थिर पूर्णांक builtin_help(संरचना child_prog *child);
स्थिर पूर्णांक builtin_jobs(संरचना child_prog *child);
स्थिर पूर्णांक builtin_pwd(संरचना child_prog *child);
स्थिर पूर्णांक builtin_read(संरचना child_prog *child);
स्थिर पूर्णांक builtin_set(संरचना child_prog *child);
स्थिर पूर्णांक builtin_shift(संरचना child_prog *child);
स्थिर पूर्णांक builtin_source(संरचना child_prog *child);
स्थिर पूर्णांक builtin_umask(संरचना child_prog *child);
स्थिर पूर्णांक builtin_unset(संरचना child_prog *child);
स्थिर पूर्णांक builtin_not_written(संरचना child_prog *child);
#endif
/*   o_string manipulation: */
स्थिर पूर्णांक b_check_space(o_string *o, पूर्णांक len);
स्थिर पूर्णांक b_addchr(o_string *o, पूर्णांक ch);
स्थिर शून्य b_reset(o_string *o);
स्थिर पूर्णांक b_addqchr(o_string *o, पूर्णांक ch, पूर्णांक quote);
#ifndef __U_BOOT__
स्थिर पूर्णांक b_adduint(o_string *o, अचिन्हित पूर्णांक i);
#endif
/*  in_str manipulations: */
स्थिर पूर्णांक static_get(संरचना in_str *i);
स्थिर पूर्णांक static_peek(संरचना in_str *i);
स्थिर पूर्णांक file_get(संरचना in_str *i);
स्थिर पूर्णांक file_peek(संरचना in_str *i);
#ifndef __U_BOOT__
स्थिर शून्य setup_file_in_str(संरचना in_str *i, खाता *f);
#अन्यथा
स्थिर शून्य setup_file_in_str(संरचना in_str *i);
#endif
स्थिर शून्य setup_string_in_str(संरचना in_str *i, स्थिरांक अक्षर *s);
#ifndef __U_BOOT__
/*  close_me manipulations: */
स्थिर शून्य mark_open(पूर्णांक fd);
स्थिर शून्य mark_closed(पूर्णांक fd);
स्थिर शून्य close_all(शून्य);
#endif
/*  "run" the final data structures: */
स्थिर अक्षर *indenter(पूर्णांक i);
स्थिर पूर्णांक free_pipe_list(संरचना pipe *head, पूर्णांक indent);
स्थिर पूर्णांक free_pipe(संरचना pipe *pi, पूर्णांक indent);
/*  really run the final data structures: */
#ifndef __U_BOOT__
स्थिर पूर्णांक setup_redirects(संरचना child_prog *prog, पूर्णांक squirrel[]);
#endif
स्थिर पूर्णांक run_list_real(संरचना pipe *pi);
#ifndef __U_BOOT__
स्थिर शून्य pseudo_exec(संरचना child_prog *child) __attribute__ ((noreturn));
#endif
स्थिर पूर्णांक run_pipe_real(संरचना pipe *pi);
/*   extended glob support: */
#ifndef __U_BOOT__
स्थिर पूर्णांक globhack(स्थिरांक अक्षर *src, पूर्णांक flags, glob_t *pglob);
स्थिर पूर्णांक glob_needed(स्थिरांक अक्षर *s);
स्थिर पूर्णांक xglob(o_string *dest, पूर्णांक flags, glob_t *pglob);
#endif
/*   variable assignment: */
स्थिर पूर्णांक is_assignment(स्थिरांक अक्षर *s);
/*   data structure manipulation: */
#ifndef __U_BOOT__
स्थिर पूर्णांक setup_redirect(संरचना p_context *ctx, पूर्णांक fd, redir_type style, संरचना in_str *आदान);
#endif
स्थिर शून्य initialize_context(संरचना p_context *ctx);
स्थिर पूर्णांक done_word(o_string *dest, संरचना p_context *ctx);
स्थिर पूर्णांक done_command(संरचना p_context *ctx);
स्थिर पूर्णांक done_pipe(संरचना p_context *ctx, pipe_style type);
/*   primary string parsing: */
#ifndef __U_BOOT__
स्थिर पूर्णांक redirect_dup_अंक(संरचना in_str *आदान);
स्थिर पूर्णांक redirect_opt_अंक(o_string *o);
स्थिर पूर्णांक process_command_subs(o_string *dest, संरचना p_context *ctx, संरचना in_str *आदान, पूर्णांक subst_end);
स्थिर पूर्णांक parse_group(o_string *dest, संरचना p_context *ctx, संरचना in_str *आदान, पूर्णांक ch);
#endif
स्थिर अक्षर *lookup_param(अक्षर *src);
स्थिर अक्षर *make_string(अक्षर **inp, पूर्णांक *nonnull);
स्थिर पूर्णांक handle_dollar(o_string *dest, संरचना p_context *ctx, संरचना in_str *आदान);
#ifndef __U_BOOT__
स्थिर पूर्णांक parse_string(o_string *dest, संरचना p_context *ctx, स्थिरांक अक्षर *src);
#endif
स्थिर पूर्णांक parse_stream(o_string *dest, संरचना p_context *ctx, संरचना in_str *input0, पूर्णांक end_trigger);
/*   setup: */
स्थिर पूर्णांक parse_stream_outer(संरचना in_str *inp, पूर्णांक flag);
#ifndef __U_BOOT__
स्थिर पूर्णांक parse_string_outer(स्थिरांक अक्षर *s, पूर्णांक flag);
स्थिर पूर्णांक parse_file_outer(खाता *f);
#endif
#ifndef __U_BOOT__
/*   job management: */
स्थिर पूर्णांक checkjobs(संरचना pipe* fg_pipe);
स्थिर शून्य घाला_bg_job(संरचना pipe *pi);
स्थिर शून्य remove_bg_job(संरचना pipe *pi);
#endif
/*     local variable support */
स्थिर अक्षर **make_list_in(अक्षर **inp, अक्षर *name);
स्थिर अक्षर *घाला_var_मूल्य(अक्षर *inp);
स्थिर अक्षर *घाला_var_मूल्य_sub(अक्षर *inp, पूर्णांक tag_subst);

#ifndef __U_BOOT__
/* Table of built-in functions.  They can be forked or not, depending on
 * context: within pipes, they fork.  As simple commands, they do not.
 * When used in non-forking context, they can change global variables
 * in the parent shell process.  If forked, of course they can not.
 * For example, 'unset foo | whatever' will parse and run, but foo will
 * still be set at the end. */
स्थिर संरचना built_in_command bltins[] = {
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
	{रिक्त, रिक्त, रिक्त}
};

स्थिर स्थिरांक अक्षर *set_cwd(शून्य)
{
	जर(cwd==unknown)
		cwd = रिक्त;     /* xgetcwd(arg) called free(arg) */
	cwd = xgetcwd((अक्षर *)cwd);
	जर (!cwd)
		cwd = unknown;
	परत cwd;
}

/* built-in 'eval' handler */
स्थिर पूर्णांक builtin_eval(संरचना child_prog *child)
{
	अक्षर *str = रिक्त;
	पूर्णांक rcode = निकास_सफल;

	जर (child->argv[1]) {
		str = make_string(child->argv + 1);
		parse_string_outer(str, FLAG_EXIT_FROM_LOOP |
					FLAG_PARSE_SEMICOLON);
		मुक्त(str);
		rcode = last_return_code;
	}
	परत rcode;
}

/* built-in 'cd <path>' handler */
स्थिर पूर्णांक builtin_cd(संरचना child_prog *child)
{
	अक्षर *newdir;
	जर (child->argv[1] == रिक्त)
		newdir = env_get("HOME");
	अन्यथा
		newdir = child->argv[1];
	जर (स_बदलो(newdir)) {
		म_लिखो("cd: %s: %s\n", newdir, म_त्रुटि(त्रुटि_सं));
		परत निकास_त्रुटि;
	}
	set_cwd();
	परत निकास_सफल;
}

/* built-in 'env' handler */
स्थिर पूर्णांक builtin_env(संरचना child_prog *dummy)
{
	अक्षर **e = environ;
	जर (e == रिक्त) परत निकास_त्रुटि;
	साठी (; *e; e++) {
		माला_दो(*e);
	}
	परत निकास_सफल;
}

/* built-in 'exec' handler */
स्थिर पूर्णांक builtin_exec(संरचना child_prog *child)
{
	जर (child->argv[1] == रिक्त)
		परत निकास_सफल;   /* Really? */
	child->argv++;
	pseudo_exec(child);
	/* never returns */
}

/* built-in 'exit' handler */
स्थिर पूर्णांक builtin_exit(संरचना child_prog *child)
{
	जर (child->argv[1] == रिक्त)
		निकास(last_return_code);
	निकास (म_से_प(child->argv[1]));
}

/* built-in 'export VAR=value' handler */
स्थिर पूर्णांक builtin_export(संरचना child_prog *child)
{
	पूर्णांक res = 0;
	अक्षर *name = child->argv[1];

	जर (name == रिक्त) {
		परत (builtin_env(child));
	}

	name = strdup(name);

	जर(name) {
		अक्षर *मूल्य = म_अक्षर(name, '=');

		जर (!मूल्य) {
			अक्षर *tmp;
			/* They are exporting something without an =VALUE */

			मूल्य = get_local_var(name);
			जर (मूल्य) {
				माप_प्रकार ln = म_माप(name);

				tmp = पुनः_स्मृति(name, ln+म_माप(मूल्य)+2);
				जर(tmp==रिक्त)
					res = -1;
				अन्यथा {
					प्र_लिखो(tmp+ln, "=%s", मूल्य);
					name = tmp;
				}
			} अन्यथा {
				/* bash does not return an error when trying to export
				 * an undefined variable.  Do likewise. */
				res = 1;
			}
		}
	}
	जर (res<0)
		perror_संदेश("export");
	अन्यथा जर(res==0)
		res = set_local_var(name, 1);
	अन्यथा
		res = 0;
	मुक्त(name);
	परत res;
}

/* built-in 'fg' and 'bg' handler */
स्थिर पूर्णांक builtin_fg_bg(संरचना child_prog *child)
{
	पूर्णांक i, jobnum;
	संरचना pipe *pi=रिक्त;

	जर (!interactive)
		परत निकास_त्रुटि;
	/* If they gave us no args, assume they want the last backgrounded task */
	जर (!child->argv[1]) {
		साठी (pi = job_list; pi; pi = pi->next) {
			जर (pi->jobid == last_jobid) {
				थांबा;
			}
		}
		जर (!pi) {
			error_संदेश("%s: no current job", child->argv[0]);
			परत निकास_त्रुटि;
		}
	} अन्यथा {
		जर (माला_पूछो(child->argv[1], "%%%d", &jobnum) != 1) {
			error_संदेश("%s: bad argument '%s'", child->argv[0], child->argv[1]);
			परत निकास_त्रुटि;
		}
		साठी (pi = job_list; pi; pi = pi->next) {
			जर (pi->jobid == jobnum) {
				थांबा;
			}
		}
		जर (!pi) {
			error_संदेश("%s: %d: no such job", child->argv[0], jobnum);
			परत निकास_त्रुटि;
		}
	}

	जर (*child->argv[0] == 'f') {
		/* Put the job into the foreground.  */
		tcsetpgrp(shell_terminal, pi->pgrp);
	}

	/* Restart the processes in the job */
	साठी (i = 0; i < pi->अंक_progs; i++)
		pi->progs[i].is_stopped = 0;

	जर ( (i=समाप्त(- pi->pgrp, SIGCONT)) < 0) {
		जर (i == ESRCH) {
			remove_bg_job(pi);
		} अन्यथा {
			perror_संदेश("kill (SIGCONT)");
		}
	}

	pi->stopped_progs = 0;
	परत निकास_सफल;
}

/* built-in 'help' handler */
स्थिर पूर्णांक builtin_help(संरचना child_prog *dummy)
{
	संरचना built_in_command *x;

	म_लिखो("\nBuilt-in commands:\n");
	म_लिखो("-------------------\n");
	साठी (x = bltins; x->cmd; x++) {
		जर (x->descr==रिक्त)
			सुरूठेवा;
		म_लिखो("%s\t%s\n", x->cmd, x->descr);
	}
	म_लिखो("\n\n");
	परत निकास_सफल;
}

/* built-in 'jobs' handler */
स्थिर पूर्णांक builtin_jobs(संरचना child_prog *child)
{
	संरचना pipe *job;
	अक्षर *status_string;

	साठी (job = job_list; job; job = job->next) {
		जर (job->running_progs == job->stopped_progs)
			status_string = "Stopped";
		अन्यथा
			status_string = "Running";

		म_लिखो(JOB_STATUS_FORMAT, job->jobid, status_string, job->text);
	}
	परत निकास_सफल;
}

/* built-in 'pwd' handler */
स्थिर पूर्णांक builtin_pwd(संरचना child_prog *dummy)
{
	माला_दो(set_cwd());
	परत निकास_सफल;
}

/* built-in 'read VAR' handler */
स्थिर पूर्णांक builtin_read(संरचना child_prog *child)
{
	पूर्णांक res;

	जर (child->argv[1]) {
		अक्षर string[बफ_मान];
		अक्षर *var = 0;

		string[0] = 0;  /* In case stdin has only EOF */
		/* read string */
		ख_माला_लो(string, आकार(string), मानक_निवेश);
		chomp(string);
		var = दो_स्मृति(म_माप(child->argv[1])+म_माप(string)+2);
		जर(var) {
			प्र_लिखो(var, "%s=%s", child->argv[1], string);
			res = set_local_var(var, 0);
		} अन्यथा
			res = -1;
		जर (res)
			ख_लिखो(मानक_त्रुटि, "read: %m\n");
		मुक्त(var);      /* So not move up to avoid breaking errno */
		परत res;
	} अन्यथा {
		करा res=अक्षर_लो(); पर्यंत(res!='\n' && res!=खातापूर्ण);
		परत 0;
	}
}

/* built-in 'set VAR=value' handler */
स्थिर पूर्णांक builtin_set(संरचना child_prog *child)
{
	अक्षर *temp = child->argv[1];
	संरचना variables *e;

	जर (temp == रिक्त)
		साठी(e = शीर्ष_vars; e; e=e->next)
			म_लिखो("%s=%s\n", e->name, e->मूल्य);
	अन्यथा
		set_local_var(temp, 0);

		परत निकास_सफल;
}

/* Built-in 'shift' handler */
स्थिर पूर्णांक builtin_shift(संरचना child_prog *child)
{
	पूर्णांक n=1;
	जर (child->argv[1]) {
		n=म_से_प(child->argv[1]);
	}
	जर (n>=0 && n<global_argc) {
		/* XXX This probably breaks $0 */
		global_argc -= n;
		global_argv += n;
		परत निकास_सफल;
	} अन्यथा {
		परत निकास_त्रुटि;
	}
}

/* Built-in '.' handler (read-in and execute commands from file) */
स्थिर पूर्णांक builtin_source(संरचना child_prog *child)
{
	खाता *आदान;
	पूर्णांक status;

	जर (child->argv[1] == रिक्त)
		परत निकास_त्रुटि;

	/* XXX search through $PATH is missing */
	आदान = ख_खोलो(child->argv[1], "r");
	जर (!आदान) {
		error_संदेश("Couldn't open file '%s'", child->argv[1]);
		परत निकास_त्रुटि;
	}

	/* Now run the file */
	/* XXX argv and argc are broken; need to save old global_argv
	 * (pointer only is OK!) on this stack frame,
	 * set global_argv=child->argv+1, recurse, and restore. */
	mark_open(fileno(आदान));
	status = parse_file_outer(आदान);
	mark_closed(fileno(आदान));
	ख_बंद(आदान);
	परत (status);
}

स्थिर पूर्णांक builtin_umask(संरचना child_prog *child)
{
	mode_t new_umask;
	स्थिरांक अक्षर *arg = child->argv[1];
	अक्षर *end;
	जर (arg) {
		new_umask=म_से_अदीर्घ(arg, &end, 8);
		जर (*end!='\0' || end == arg) {
			परत निकास_त्रुटि;
		}
	} अन्यथा {
		म_लिखो("%.3o\n", (अचिन्हित पूर्णांक) (new_umask=umask(0)));
	}
	umask(new_umask);
	परत निकास_सफल;
}

/* built-in 'unset VAR' handler */
स्थिर पूर्णांक builtin_unset(संरचना child_prog *child)
{
	/* bash returned already true */
	unset_local_var(child->argv[1]);
	परत निकास_सफल;
}

स्थिर पूर्णांक builtin_not_written(संरचना child_prog *child)
{
	म_लिखो("builtin_%s not written\n",child->argv[0]);
	परत निकास_त्रुटि;
}
#endif

स्थिर पूर्णांक b_check_space(o_string *o, पूर्णांक len)
{
	/* It would be easy to drop a more restrictive policy
	 * in here, such as setting a maximum string length */
	जर (o->length + len > o->maxlen) {
		अक्षर *old_data = o->data;
		/* assert (data == NULL || o->maxlen != 0); */
		o->maxlen += max(2*len, B_CHUNK);
		o->data = पुनः_स्मृति(o->data, 1 + o->maxlen);
		जर (o->data == रिक्त) {
			मुक्त(old_data);
		}
	}
	परत o->data == रिक्त;
}

स्थिर पूर्णांक b_addchr(o_string *o, पूर्णांक ch)
{
	debug_printf("b_addchr: %c %d %p\n", ch, o->length, o);
	जर (b_check_space(o, 1)) परत B_NOSPAC;
	o->data[o->length] = ch;
	o->length++;
	o->data[o->length] = '\0';
	परत 0;
}

स्थिर शून्य b_reset(o_string *o)
{
	o->length = 0;
	o->nonnull = 0;
	जर (o->data != रिक्त) *o->data = '\0';
}

स्थिर शून्य b_free(o_string *o)
{
	b_reset(o);
	मुक्त(o->data);
	o->data = रिक्त;
	o->maxlen = 0;
}

/* My analysis of quoting semantics tells me that state information
 * is associated with a destination, not a source.
 */
स्थिर पूर्णांक b_addqchr(o_string *o, पूर्णांक ch, पूर्णांक quote)
{
	जर (quote && म_अक्षर("*?[\\",ch)) {
		पूर्णांक rc;
		rc = b_addchr(o, '\\');
		जर (rc) परत rc;
	}
	परत b_addchr(o, ch);
}

#ifndef __U_BOOT__
स्थिर पूर्णांक b_adduint(o_string *o, अचिन्हित पूर्णांक i)
{
	पूर्णांक r;
	अक्षर *p = simple_itoa(i);
	/* no escape checking necessary */
	करा r=b_addchr(o, *p++); पर्यंत (r==0 && *p);
	परत r;
}
#endif

स्थिर पूर्णांक static_get(संरचना in_str *i)
{
	पूर्णांक ch = *i->p++;
	जर (ch=='\0') परत खातापूर्ण;
	परत ch;
}

स्थिर पूर्णांक static_peek(संरचना in_str *i)
{
	परत *i->p;
}

#ifndef __U_BOOT__
स्थिर ओळीत शून्य cmdedit_set_initial_prompt(शून्य)
{
#ifndef CONFIG_FEATURE_SH_FANCY_PROMPT
	PS1 = रिक्त;
#अन्यथा
	PS1 = env_get("PS1");
	जर(PS1==0)
		PS1 = "\\w \\$ ";
#endif
}

स्थिर ओळीत शून्य setup_prompt_string(पूर्णांक promptmode, अक्षर **prompt_str)
{
	debug_printf("setup_prompt_string %d ",promptmode);
#ifndef CONFIG_FEATURE_SH_FANCY_PROMPT
	/* Set up the prompt */
	जर (promptmode == 1) {
		मुक्त(PS1);
		PS1=xmalloc(म_माप(cwd)+4);
		प्र_लिखो(PS1, "%s %s", cwd, ( geteuid() != 0 ) ?  "$ ":"# ");
		*prompt_str = PS1;
	} अन्यथा {
		*prompt_str = PS2;
	}
#अन्यथा
	*prompt_str = (promptmode==1)? PS1 : PS2;
#endif
	debug_printf("result %s\n",*prompt_str);
}
#endif

#ifdef __U_BOOT__
स्थिर पूर्णांक uboot_cli_readline(संरचना in_str *i)
{
	अक्षर *prompt;
	अक्षर __maybe_unused *ps_prompt = रिक्त;

	जर (i->promptmode == 1)
		prompt = CONFIG_SYS_PROMPT;
	अन्यथा
		prompt = CONFIG_SYS_PROMPT_HUSH_PS2;

#ifdef CONFIG_CMDLINE_PS_SUPPORT
	जर (i->promptmode == 1)
		ps_prompt = env_get("PS1");
	अन्यथा
		ps_prompt = env_get("PS2");
	जर (ps_prompt)
		prompt = ps_prompt;
#endif

	परत cli_readline(prompt);
}
#endif

स्थिर शून्य get_user_आदान(संरचना in_str *i)
{
#ifndef __U_BOOT__
	अक्षर *prompt_str;
	स्थिर अक्षर the_command[बफ_मान];

	setup_prompt_string(i->promptmode, &prompt_str);
#ifdef CONFIG_FEATURE_COMMAND_EDITING
	/*
	 ** enable command line editing only while a command line
	 ** is actually being read; otherwise, we'll end up bequeathing
	 ** atexit() handlers and other unwanted stuff to our
	 ** child processes (rob@sysgo.de)
	 */
	cmdedit_read_आदान(prompt_str, the_command);
#अन्यथा
	ख_माला_दो(prompt_str, मानक_निकास);
	ख_साफ(मानक_निकास);
	the_command[0]=ख_अक्षर_लो(i->file);
	the_command[1]='\0';
#endif
	ख_साफ(मानक_निकास);
	i->p = the_command;
#अन्यथा
	पूर्णांक n;
	स्थिर अक्षर the_command[CONFIG_SYS_CBSIZE + 1];

	bootretry_reset_cmd_timeout();
	i->__promptme = 1;
	n = uboot_cli_readline(i);

#ifdef CONFIG_BOOT_RETRY_TIME
	जर (n == -2) {
	  माला_दो("\nTimeout waiting for command\n");
#  ifdef CONFIG_RESET_TO_RETRY
	  do_reset(रिक्त, 0, 0, रिक्त);
#  elif IS_ENABLED(CONFIG_RETRY_BOOTCMD)
	म_नकल(console_buffer, "run bootcmd\n");
# अन्यथा
#	error "This only works with CONFIG_RESET_TO_RETRY or CONFIG_BOOT_RETRY_COMMAND enabled"
#  endif
	}
#endif
	जर (n == -1 ) {
		flag_repeat = 0;
		i->__promptme = 0;
	}
	n = म_माप(console_buffer);
	console_buffer[n] = '\n';
	console_buffer[n+1]= '\0';
	जर (had_ctrlc()) flag_repeat = 0;
	clear_ctrlc();
	do_repeat = 0;
	जर (i->promptmode == 1) {
		जर (console_buffer[0] == '\n'&& flag_repeat == 0) {
			म_नकल(the_command,console_buffer);
		}
		अन्यथा {
			जर (console_buffer[0] != '\n') {
				म_नकल(the_command,console_buffer);
				flag_repeat = 1;
			}
			अन्यथा {
				do_repeat = 1;
			}
		}
		i->p = the_command;
	}
	अन्यथा {
		जर (console_buffer[0] != '\n') {
			जर (म_माप(the_command) + म_माप(console_buffer)
			    < CONFIG_SYS_CBSIZE) {
				n = म_माप(the_command);
				the_command[n-1] = ' ';
				म_नकल(&the_command[n],console_buffer);
			}
			अन्यथा {
				the_command[0] = '\n';
				the_command[1] = '\0';
				flag_repeat = 0;
			}
		}
		जर (i->__promptme == 0) {
			the_command[0] = '\n';
			the_command[1] = '\0';
		}
		i->p = console_buffer;
	}
#endif
}

/* This is the magic location that prints prompts
 * and gets data back from the user */
स्थिर पूर्णांक file_get(संरचना in_str *i)
{
	पूर्णांक ch;

	ch = 0;
	/* If there is data waiting, eat it up */
	जर (i->p && *i->p) {
		ch = *i->p++;
	} अन्यथा {
		/* need to double check i->file because we might be doing something
		 * more complicated by now, like sourcing or substituting. */
#ifndef __U_BOOT__
		जर (i->__promptme && interactive && i->file == मानक_निवेश) {
			पर्यंत(! i->p || (interactive && म_माप(i->p)==0) ) {
#अन्यथा
			पर्यंत(! i->p  || म_माप(i->p)==0 ) {
#endif
				get_user_आदान(i);
			}
			i->promptmode=2;
#ifndef __U_BOOT__
			i->__promptme = 0;
#endif
			जर (i->p && *i->p) {
				ch = *i->p++;
			}
#ifndef __U_BOOT__
		} अन्यथा {
			ch = ख_अक्षर_लो(i->file);
		}

#endif
		debug_printf("b_getch: got a %d\n", ch);
	}
#ifndef __U_BOOT__
	जर (ch == '\n') i->__promptme=1;
#endif
	परत ch;
}

/* All the callers guarantee this routine will never be
 * used right after a newline, so prompting is not needed.
 */
स्थिर पूर्णांक file_peek(संरचना in_str *i)
{
#ifndef __U_BOOT__
	जर (i->p && *i->p) {
#endif
		परत *i->p;
#ifndef __U_BOOT__
	} अन्यथा {
		i->peek_buf[0] = ख_अक्षर_लो(i->file);
		i->peek_buf[1] = '\0';
		i->p = i->peek_buf;
		debug_printf("b_peek: got a %d\n", *i->p);
		परत *i->p;
	}
#endif
}

#ifndef __U_BOOT__
स्थिर शून्य setup_file_in_str(संरचना in_str *i, खाता *f)
#अन्यथा
स्थिर शून्य setup_file_in_str(संरचना in_str *i)
#endif
{
	i->peek = file_peek;
	i->get = file_get;
	i->__promptme=1;
	i->promptmode=1;
#ifndef __U_BOOT__
	i->file = f;
#endif
	i->p = रिक्त;
}

स्थिर शून्य setup_string_in_str(संरचना in_str *i, स्थिरांक अक्षर *s)
{
	i->peek = static_peek;
	i->get = static_get;
	i->__promptme=1;
	i->promptmode=1;
	i->p = s;
}

#ifndef __U_BOOT__
स्थिर शून्य mark_open(पूर्णांक fd)
{
	संरचना close_me *new = xmalloc(आकार(संरचना close_me));
	new->fd = fd;
	new->next = close_me_head;
	close_me_head = new;
}

स्थिर शून्य mark_closed(पूर्णांक fd)
{
	संरचना close_me *tmp;
	जर (close_me_head == रिक्त || close_me_head->fd != fd)
		error_संदेश_and_die("corrupt close_me");
	tmp = close_me_head;
	close_me_head = close_me_head->next;
	मुक्त(tmp);
}

स्थिर शून्य close_all(शून्य)
{
	संरचना close_me *c;
	साठी (c=close_me_head; c; c=c->next) {
		बंद(c->fd);
	}
	close_me_head = रिक्त;
}

/* squirrel != NULL means we squirrel away copies of stdin, stdout,
 * and stderr if they are redirected. */
स्थिर पूर्णांक setup_redirects(संरचना child_prog *prog, पूर्णांक squirrel[])
{
	पूर्णांक openfd, mode;
	संरचना redir_struct *redir;

	साठी (redir=prog->redirects; redir; redir=redir->next) {
		जर (redir->dup == -1 && redir->word.gl_pathv == रिक्त) {
			/* something went wrong in the parse.  Pretend it didn't happen */
			सुरूठेवा;
		}
		जर (redir->dup == -1) {
			mode=redir_table[redir->type].mode;
			openfd = खोलो(redir->word.gl_pathv[0], mode, 0666);
			जर (openfd < 0) {
			/* this could get lost if stderr has been redirected, but
			   bash and ash both lose it as well (though zsh doesn't!) */
				perror_संदेश("error opening %s", redir->word.gl_pathv[0]);
				परत 1;
			}
		} अन्यथा {
			openfd = redir->dup;
		}

		जर (openfd != redir->fd) {
			जर (squirrel && redir->fd < 3) {
				squirrel[redir->fd] = dup(redir->fd);
			}
			जर (openfd == -3) {
				बंद(openfd);
			} अन्यथा {
				dup2(openfd, redir->fd);
				जर (redir->dup == -1)
					बंद (openfd);
			}
		}
	}
	परत 0;
}

स्थिर शून्य restore_redirects(पूर्णांक squirrel[])
{
	पूर्णांक i, fd;
	साठी (i=0; i<3; i++) {
		fd = squirrel[i];
		जर (fd != -1) {
			/* No error checking.  I sure wouldn't know what
			 * to do with an error if I found one! */
			dup2(fd, i);
			बंद(fd);
		}
	}
}

/* never returns */
/* XXX no exit() here.  If you don't exec, use _exit instead.
 * The at_exit handlers apparently confuse the calling process,
 * in particular stdin handling.  Not sure why? */
स्थिर शून्य pseudo_exec(संरचना child_prog *child)
{
	पूर्णांक i, rcode;
	अक्षर *p;
	संरचना built_in_command *x;
	जर (child->argv) {
		साठी (i=0; is_assignment(child->argv[i]); i++) {
			debug_printf("pid %d environment modification: %s\n",getpid(),child->argv[i]);
			p = घाला_var_मूल्य(child->argv[i]);
			putenv(strdup(p));
			जर (p != child->argv[i]) मुक्त(p);
		}
		child->argv+=i;  /* XXX this hack isn't so horrible, since we are about
					to exit, and therefore don't need to keep data
					structures consistent for free() use. */
		/* If a variable is assigned in a forest, and nobody listens,
		 * was it ever really set?
		 */
		जर (child->argv[0] == रिक्त) {
			_exit(निकास_सफल);
		}

		/*
		 * Check if the command matches any of the builtins.
		 * Depending on context, this might be redundant.  But it's
		 * easier to waste a few CPU cycles than it is to figure out
		 * if this is one of those cases.
		 */
		साठी (x = bltins; x->cmd; x++) {
			जर (म_भेद(child->argv[0], x->cmd) == 0 ) {
				debug_printf("builtin exec %s\n", child->argv[0]);
				rcode = x->function(child);
				ख_साफ(मानक_निकास);
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
			पूर्णांक argc_l;
			अक्षर** argv_l=child->argv;
			अक्षर *name = child->argv[0];

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
			साठी(argc_l=0;*argv_l!=रिक्त; argv_l++, argc_l++);
			optind = 1;
			debug_printf("running applet %s\n", name);
			run_applet_by_name(name, argc_l, child->argv);
		}
#endif
		debug_printf("exec of %s\n",child->argv[0]);
		execvp(child->argv[0],child->argv);
		perror_संदेश("couldn't exec: %s",child->argv[0]);
		_exit(1);
	} अन्यथा जर (child->group) {
		debug_printf("runtime nesting to group\n");
		interactive=0;    /* crucial!!!! */
		rcode = run_list_real(child->group);
		/* OK to leak memory by not calling free_pipe_list,
		 * since this process is about to exit */
		_exit(rcode);
	} अन्यथा {
		/* Can happen.  See what bash does with ">foo" by itself. */
		debug_printf("trying to pseudo_exec null command\n");
		_exit(निकास_सफल);
	}
}

स्थिर शून्य घाला_bg_job(संरचना pipe *pi)
{
	संरचना pipe *thejob;

	/* Linear search for the ID of the job to use */
	pi->jobid = 1;
	साठी (thejob = job_list; thejob; thejob = thejob->next)
		जर (thejob->jobid >= pi->jobid)
			pi->jobid = thejob->jobid + 1;

	/* add thejob to the list of running jobs */
	जर (!job_list) {
		thejob = job_list = xmalloc(आकार(*thejob));
	} अन्यथा {
		साठी (thejob = job_list; thejob->next; thejob = thejob->next) /* nothing */;
		thejob->next = xmalloc(आकार(*thejob));
		thejob = thejob->next;
	}

	/* physically copy the struct job */
	स_नकल(thejob, pi, आकार(संरचना pipe));
	thejob->next = रिक्त;
	thejob->running_progs = thejob->अंक_progs;
	thejob->stopped_progs = 0;
	thejob->text = xmalloc(बफ_मान); /* cmdedit buffer size */

	/*if (pi->progs[0] && pi->progs[0].argv && pi->progs[0].argv[0]) */
	{
		अक्षर *bar=thejob->text;
		अक्षर **foo=pi->progs[0].argv;
		पर्यंत(foo && *foo) {
			bar += प्र_लिखो(bar, "%s ", *foo++);
		}
	}

	/* we don't wait for background thejobs to return -- append it
	   to the list of backgrounded thejobs and leave it alone */
	म_लिखो("[%d] %d\n", thejob->jobid, thejob->progs[0].pid);
	last_bg_pid = thejob->progs[0].pid;
	last_jobid = thejob->jobid;
}

/* remove a backgrounded job */
स्थिर शून्य remove_bg_job(संरचना pipe *pi)
{
	संरचना pipe *prev_pipe;

	जर (pi == job_list) {
		job_list = pi->next;
	} अन्यथा {
		prev_pipe = job_list;
		पर्यंत (prev_pipe->next != pi)
			prev_pipe = prev_pipe->next;
		prev_pipe->next = pi->next;
	}
	जर (job_list)
		last_jobid = job_list->jobid;
	अन्यथा
		last_jobid = 0;

	pi->stopped_progs = 0;
	free_pipe(pi, 0);
	मुक्त(pi);
}

/* Checks to see if any processes have exited -- if they
   have, figure out why and see if a job has completed */
स्थिर पूर्णांक checkjobs(संरचना pipe* fg_pipe)
{
	पूर्णांक attributes;
	पूर्णांक status;
	पूर्णांक prognum = 0;
	संरचना pipe *pi;
	pid_t childpid;

	attributes = WUNTRACED;
	जर (fg_pipe==रिक्त) {
		attributes |= WNOHANG;
	}

	पर्यंत ((childpid = waitpid(-1, &status, attributes)) > 0) {
		जर (fg_pipe) {
			पूर्णांक i, rcode = 0;
			साठी (i=0; i < fg_pipe->अंक_progs; i++) {
				जर (fg_pipe->progs[i].pid == childpid) {
					जर (i==fg_pipe->अंक_progs-1)
						rcode=WEXITSTATUS(status);
					(fg_pipe->अंक_progs)--;
					परत(rcode);
				}
			}
		}

		साठी (pi = job_list; pi; pi = pi->next) {
			prognum = 0;
			पर्यंत (prognum < pi->अंक_progs && pi->progs[prognum].pid != childpid) {
				prognum++;
			}
			जर (prognum < pi->अंक_progs)
				थांबा;
		}

		जर(pi==रिक्त) {
			debug_printf("checkjobs: pid %d was not in our list!\n", childpid);
			सुरूठेवा;
		}

		जर (WIFEXITED(status) || WIFSIGNALED(status)) {
			/* child exited */
			pi->running_progs--;
			pi->progs[prognum].pid = 0;

			जर (!pi->running_progs) {
				म_लिखो(JOB_STATUS_FORMAT, pi->jobid, "Done", pi->text);
				remove_bg_job(pi);
			}
		} अन्यथा {
			/* child stopped */
			pi->stopped_progs++;
			pi->progs[prognum].is_stopped = 1;

#जर 0
			/* Printing this stuff is a pain, since it tends to
			 * overwrite the prompt an inconveinient moments.  So
			 * don't do that.  */
			जर (pi->stopped_progs == pi->अंक_progs) {
				म_लिखो("\n"JOB_STATUS_FORMAT, pi->jobid, "Stopped", pi->text);
			}
#endif
		}
	}

	जर (childpid == -1 && त्रुटि_सं != ECHILD)
		perror_संदेश("waitpid");

	/* move the shell to the foreground */
	/*if (interactive && tcsetpgrp(shell_terminal, getpgid(0))) */
	/*	perror_msg("tcsetpgrp-2"); */
	परत -1;
}

/* Figure out our controlling tty, checking in order stderr,
 * stdin, and stdout.  If check_pgrp is set, also check that
 * we belong to the foreground process group associated with
 * that tty.  The value of shell_terminal is needed in order to call
 * tcsetpgrp(shell_terminal, ...); */
शून्य controlling_tty(पूर्णांक check_pgrp)
{
	pid_t curpgrp;

	जर ((curpgrp = tcgetpgrp(shell_terminal = 2)) < 0
			&& (curpgrp = tcgetpgrp(shell_terminal = 0)) < 0
			&& (curpgrp = tcgetpgrp(shell_terminal = 1)) < 0)
		जा shell_terminal_error;

	जर (check_pgrp && curpgrp != getpgid(0))
		जा shell_terminal_error;

	परत;

shell_terminal_error:
		shell_terminal = -1;
		परत;
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
स्थिर पूर्णांक run_pipe_real(संरचना pipe *pi)
{
	पूर्णांक i;
#ifndef __U_BOOT__
	पूर्णांक nextin, nextout;
	पूर्णांक pipefds[2];				/* pipefds[0] is for reading */
	संरचना child_prog *child;
	संरचना built_in_command *x;
	अक्षर *p;
# जर __GNUC__
	/* Avoid longjmp clobbering */
	(शून्य) &i;
	(शून्य) &nextin;
	(शून्य) &nextout;
	(शून्य) &child;
# endif
#अन्यथा
	पूर्णांक nextin;
	पूर्णांक flag = do_repeat ? CMD_FLAG_REPEAT : 0;
	संरचना child_prog *child;
	अक्षर *p;
# जर __GNUC__
	/* Avoid longjmp clobbering */
	(शून्य) &i;
	(शून्य) &nextin;
	(शून्य) &child;
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
	जर (pi->अंक_progs == 1) child = & (pi->progs[0]);
#ifndef __U_BOOT__
	जर (pi->अंक_progs == 1 && child->group && child->subshell == 0) {
		पूर्णांक squirrel[] = {-1, -1, -1};
		पूर्णांक rcode;
		debug_printf("non-subshell grouping\n");
		setup_redirects(child, squirrel);
		/* XXX could we merge code with following builtin case,
		 * by creating a pseudo builtin that calls run_list_real? */
		rcode = run_list_real(child->group);
		restore_redirects(squirrel);
#अन्यथा
		जर (pi->अंक_progs == 1 && child->group) {
		पूर्णांक rcode;
		debug_printf("non-subshell grouping\n");
		rcode = run_list_real(child->group);
#endif
		परत rcode;
	} अन्यथा जर (pi->अंक_progs == 1 && pi->progs[0].argv != रिक्त) {
		साठी (i=0; is_assignment(child->argv[i]); i++) { /* nothing */ }
		जर (i!=0 && child->argv[i]==रिक्त) {
			/* assignments, but no command: set the local environment */
			साठी (i=0; child->argv[i]!=रिक्त; i++) {

				/* Ok, this case is tricky.  We have to decide if this is a
				 * local variable, or an already exported variable.  If it is
				 * already exported, we have to export the new value.  If it is
				 * not exported, we need only set this as a local variable.
				 * This junk is all to decide whether or not to export this
				 * variable. */
				पूर्णांक export_me=0;
				अक्षर *name, *मूल्य;
				name = xstrdup(child->argv[i]);
				debug_printf("Local environment set: %s\n", name);
				मूल्य = म_अक्षर(name, '=');
				जर (मूल्य)
					*मूल्य=0;
#ifndef __U_BOOT__
				जर ( get_local_var(name)) {
					export_me=1;
				}
#endif
				मुक्त(name);
				p = घाला_var_मूल्य(child->argv[i]);
				set_local_var(p, export_me);
				जर (p != child->argv[i]) मुक्त(p);
			}
			परत निकास_सफल;   /* don't worry about errors in set_local_var() yet */
		}
		साठी (i = 0; is_assignment(child->argv[i]); i++) {
			p = घाला_var_मूल्य(child->argv[i]);
#ifndef __U_BOOT__
			putenv(strdup(p));
#अन्यथा
			set_local_var(p, 0);
#endif
			जर (p != child->argv[i]) {
				child->sp--;
				मुक्त(p);
			}
		}
		जर (child->sp) {
			अक्षर * str = रिक्त;

			str = make_string(child->argv + i,
					  child->argv_nonnull + i);
			parse_string_outer(str, FLAG_EXIT_FROM_LOOP | FLAG_REPARSING);
			मुक्त(str);
			परत last_return_code;
		}
#ifndef __U_BOOT__
		साठी (x = bltins; x->cmd; x++) {
			जर (म_भेद(child->argv[i], x->cmd) == 0 ) {
				पूर्णांक squirrel[] = {-1, -1, -1};
				पूर्णांक rcode;
				जर (x->function == builtin_exec && child->argv[i+1]==रिक्त) {
					debug_printf("magic exec\n");
					setup_redirects(child,रिक्त);
					परत निकास_सफल;
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
			परत rcode;
		}
#अन्यथा
		/* check ";", because ,example , argv consist from
		 * "help;flinfo" must not execute
		 */
		जर (म_अक्षर(child->argv[i], ';')) {
			म_लिखो("Unknown command '%s' - try 'help' or use "
					"'run' command\n", child->argv[i]);
			परत -1;
		}
		/* Process the command */
		परत cmd_process(flag, child->argc - i, child->argv + i,
				   &flag_repeat, रिक्त);
#endif
	}
#ifndef __U_BOOT__

	साठी (i = 0; i < pi->अंक_progs; i++) {
		child = & (pi->progs[i]);

		/* pipes are inserted between pairs of commands */
		जर ((i + 1) < pi->अंक_progs) {
			जर (pipe(pipefds)<0) perror_संदेश_and_die("pipe");
			nextout = pipefds[1];
		} अन्यथा {
			nextout=1;
			pipefds[0] = -1;
		}

		/* XXX test for failed fork()? */
		जर (!(child->pid = विभाजन())) {
			/* Set the handling for job control signals back to the default.  */
			संकेत(संक_विघ्न, संक_पूर्व);
			संकेत(SIGQUIT, संक_पूर्व);
			संकेत(संक_इति, संक_पूर्व);
			संकेत(SIGTSTP, संक_पूर्व);
			संकेत(SIGTTIN, संक_पूर्व);
			संकेत(SIGTTOU, संक_पूर्व);
			संकेत(SIGCHLD, संक_पूर्व);

			close_all();

			जर (nextin != 0) {
				dup2(nextin, 0);
				बंद(nextin);
			}
			जर (nextout != 1) {
				dup2(nextout, 1);
				बंद(nextout);
			}
			जर (pipefds[0]!=-1) {
				बंद(pipefds[0]);  /* opposite end of our output pipe */
			}

			/* Like bash, explicit redirects override pipes,
			 * and the pipe fd is available for dup'ing. */
			setup_redirects(child,रिक्त);

			जर (interactive && pi->followup!=PIPE_BG) {
				/* If we (the child) win the race, put ourselves in the process
				 * group whose leader is the first process in this pipe. */
				जर (pi->pgrp < 0) {
					pi->pgrp = getpid();
				}
				जर (setpgid(0, pi->pgrp) == 0) {
					tcsetpgrp(2, pi->pgrp);
				}
			}

			pseudo_exec(child);
		}

		/* put our child in the process group whose leader is the
		   first process in this pipe */
		जर (pi->pgrp < 0) {
			pi->pgrp = child->pid;
		}
		/* Don't check for errors.  The child may be dead already,
		 * in which case setpgid returns error code EACCES. */
		setpgid(child->pid, pi->pgrp);

		जर (nextin != 0)
			बंद(nextin);
		जर (nextout != 1)
			बंद(nextout);

		/* If there isn't another process, nextin is garbage
		   but it doesn't matter */
		nextin = pipefds[0];
	}
#endif
	परत -1;
}

स्थिर पूर्णांक run_list_real(संरचना pipe *pi)
{
	अक्षर *save_name = रिक्त;
	अक्षर **list = रिक्त;
	अक्षर **save_list = रिक्त;
	संरचना pipe *rpipe;
	पूर्णांक flag_rep = 0;
#ifndef __U_BOOT__
	पूर्णांक save_अंक_progs;
#endif
	पूर्णांक rcode=0, flag_skip=1;
	पूर्णांक flag_restore = 0;
	पूर्णांक if_code=0, next_if_code=0;  /* need double-buffer to handle elif */
	reserved_style rmode, skip_more_in_this_rmode=RES_XXXX;
	/* check syntax for "for" */
	साठी (rpipe = pi; rpipe; rpipe = rpipe->next) {
		जर ((rpipe->r_mode == RES_IN ||
		    rpipe->r_mode == RES_FOR) &&
		    (rpipe->next == रिक्त)) {
				syntax();
#ifdef __U_BOOT__
				flag_repeat = 0;
#endif
				परत 1;
		}
		जर ((rpipe->r_mode == RES_IN &&
			(rpipe->next->r_mode == RES_IN &&
			rpipe->next->progs->argv != रिक्त))||
			(rpipe->r_mode == RES_FOR &&
			rpipe->next->r_mode != RES_IN)) {
				syntax();
#ifdef __U_BOOT__
				flag_repeat = 0;
#endif
				परत 1;
		}
	}
	साठी (; pi; pi = (flag_restore != 0) ? rpipe : pi->next) {
		जर (pi->r_mode == RES_WHILE || pi->r_mode == RES_UNTIL ||
			pi->r_mode == RES_FOR) {
#ifdef __U_BOOT__
				/* check Ctrl-C */
				ctrlc();
				जर ((had_ctrlc())) {
					परत 1;
				}
#endif
				flag_restore = 0;
				जर (!rpipe) {
					flag_rep = 0;
					rpipe = pi;
				}
		}
		rmode = pi->r_mode;
		debug_printf("rmode=%d  if_code=%d  next_if_code=%d skip_more=%d\n", rmode, if_code, next_if_code, skip_more_in_this_rmode);
		जर (rmode == skip_more_in_this_rmode && flag_skip) {
			जर (pi->followup == PIPE_SEQ) flag_skip=0;
			सुरूठेवा;
		}
		flag_skip = 1;
		skip_more_in_this_rmode = RES_XXXX;
		जर (rmode == RES_THEN || rmode == RES_ELSE) if_code = next_if_code;
		जर (rmode == RES_THEN &&  if_code) सुरूठेवा;
		जर (rmode == RES_ELSE && !if_code) सुरूठेवा;
		जर (rmode == RES_ELIF && !if_code) थांबा;
		जर (rmode == RES_FOR && pi->अंक_progs) {
			जर (!list) {
				/* if no variable values after "in" we skip "for" */
				जर (!pi->next->progs->argv) सुरूठेवा;
				/* create list of variable values */
				list = make_list_in(pi->next->progs->argv,
					pi->progs->argv[0]);
				save_list = list;
				save_name = pi->progs->argv[0];
				pi->progs->argv[0] = रिक्त;
				flag_rep = 1;
			}
			जर (!(*list)) {
				मुक्त(pi->progs->argv[0]);
				मुक्त(save_list);
				list = रिक्त;
				flag_rep = 0;
				pi->progs->argv[0] = save_name;
#ifndef __U_BOOT__
				pi->progs->glob_result.gl_pathv[0] =
					pi->progs->argv[0];
#endif
				सुरूठेवा;
			} अन्यथा {
				/* insert new value from list for variable */
				मुक्त(pi->progs->argv[0]);
				pi->progs->argv[0] = *list++;
#ifndef __U_BOOT__
				pi->progs->glob_result.gl_pathv[0] =
					pi->progs->argv[0];
#endif
			}
		}
		जर (rmode == RES_IN) सुरूठेवा;
		जर (rmode == RES_DO) {
			जर (!flag_rep) सुरूठेवा;
		}
		जर (rmode == RES_DONE) {
			जर (flag_rep) {
				flag_restore = 1;
			} अन्यथा {
				rpipe = रिक्त;
			}
		}
		जर (pi->अंक_progs == 0) सुरूठेवा;
#ifndef __U_BOOT__
		save_अंक_progs = pi->अंक_progs; /* save number of programs */
#endif
		rcode = run_pipe_real(pi);
		debug_printf("run_pipe_real returned %d\n",rcode);
#ifndef __U_BOOT__
		जर (rcode!=-1) {
			/* We only ran a builtin: rcode was set by the return value
			 * of run_pipe_real(), and we don't need to wait for anything. */
		} अन्यथा जर (pi->followup==PIPE_BG) {
			/* XXX check bash's behavior with nontrivial pipes */
			/* XXX compute jobid */
			/* XXX what does bash do with attempts to background builtins? */
			घाला_bg_job(pi);
			rcode = निकास_सफल;
		} अन्यथा {
			जर (interactive) {
				/* move the new process group into the foreground */
				जर (tcsetpgrp(shell_terminal, pi->pgrp) && त्रुटि_सं != ENOTTY)
					perror_संदेश("tcsetpgrp-3");
				rcode = checkjobs(pi);
				/* move the shell to the foreground */
				जर (tcsetpgrp(shell_terminal, getpgid(0)) && त्रुटि_सं != ENOTTY)
					perror_संदेश("tcsetpgrp-4");
			} अन्यथा {
				rcode = checkjobs(pi);
			}
			debug_printf("checkjobs returned %d\n",rcode);
		}
		last_return_code=rcode;
#अन्यथा
		जर (rcode < -1) {
			last_return_code = -rcode - 2;
			परत -2;	/* exit */
		}
		last_return_code = rcode;
#endif
#ifndef __U_BOOT__
		pi->अंक_progs = save_अंक_progs; /* restore number of programs */
#endif
		जर ( rmode == RES_IF || rmode == RES_ELIF )
			next_if_code=rcode;  /* can be overwritten a number of times */
		जर (rmode == RES_WHILE)
			flag_rep = !last_return_code;
		जर (rmode == RES_UNTIL)
			flag_rep = last_return_code;
		जर ( (rcode==निकास_सफल && pi->followup==PIPE_OR) ||
		     (rcode!=निकास_सफल && pi->followup==PIPE_AND) )
			skip_more_in_this_rmode=rmode;
#ifndef __U_BOOT__
		checkjobs(रिक्त);
#endif
	}
	परत rcode;
}

/* broken, of course, but OK for testing */
स्थिर अक्षर *indenter(पूर्णांक i)
{
	स्थिर अक्षर blanks[]="                                    ";
	परत &blanks[आकार(blanks)-i-1];
}

/* return code is the exit status of the pipe */
स्थिर पूर्णांक free_pipe(संरचना pipe *pi, पूर्णांक indent)
{
	अक्षर **p;
	संरचना child_prog *child;
#ifndef __U_BOOT__
	संरचना redir_struct *r, *rnext;
#endif
	पूर्णांक a, i, ret_code=0;
	अक्षर *ind = indenter(indent);

#ifndef __U_BOOT__
	जर (pi->stopped_progs > 0)
		परत ret_code;
	final_printf("%s run pipe: (pid %d)\n",ind,getpid());
#endif
	साठी (i=0; i<pi->अंक_progs; i++) {
		child = &pi->progs[i];
		final_printf("%s  command %d:\n",ind,i);
		जर (child->argv) {
			साठी (a=0,p=child->argv; *p; a++,p++) {
				final_printf("%s   argv[%d] = %s\n",ind,a,*p);
			}
#ifndef __U_BOOT__
			globfree(&child->glob_result);
#अन्यथा
			साठी (a = 0; a < child->argc; a++) {
				मुक्त(child->argv[a]);
			}
			मुक्त(child->argv);
			मुक्त(child->argv_nonnull);
			child->argc = 0;
#endif
			child->argv=रिक्त;
		} अन्यथा जर (child->group) {
#ifndef __U_BOOT__
			final_printf("%s   begin group (subshell:%d)\n",ind, child->subshell);
#endif
			ret_code = free_pipe_list(child->group,indent+3);
			final_printf("%s   end group\n",ind);
		} अन्यथा {
			final_printf("%s   (nil)\n",ind);
		}
#ifndef __U_BOOT__
		साठी (r=child->redirects; r; r=rnext) {
			final_printf("%s   redirect %d%s", ind, r->fd, redir_table[r->type].descrip);
			जर (r->dup == -1) {
				/* guard against the case >$FOO, where foo is unset or blank */
				जर (r->word.gl_pathv) {
					final_printf(" %s\n", *r->word.gl_pathv);
					globfree(&r->word);
				}
			} अन्यथा {
				final_printf("&%d\n", r->dup);
			}
			rnext=r->next;
			मुक्त(r);
		}
		child->redirects=रिक्त;
#endif
	}
	मुक्त(pi->progs);   /* children are an array, they get freed all at once */
	pi->progs=रिक्त;
	परत ret_code;
}

स्थिर पूर्णांक free_pipe_list(संरचना pipe *head, पूर्णांक indent)
{
	पूर्णांक rcode=0;   /* if list has no members */
	संरचना pipe *pi, *next;
	अक्षर *ind = indenter(indent);
	साठी (pi=head; pi; pi=next) {
		final_printf("%s pipe reserved mode %d\n", ind, pi->r_mode);
		rcode = free_pipe(pi, indent);
		final_printf("%s pipe followup code %d\n", ind, pi->followup);
		next=pi->next;
		pi->next=रिक्त;
		मुक्त(pi);
	}
	परत rcode;
}

/* Select which version we will use */
स्थिर पूर्णांक run_list(संरचना pipe *pi)
{
	पूर्णांक rcode=0;
#ifndef __U_BOOT__
	जर (fake_mode==0) {
#endif
		rcode = run_list_real(pi);
#ifndef __U_BOOT__
	}
#endif
	/* free_pipe_list has the side effect of clearing memory
	 * In the long run that function can be merged with run_list_real,
	 * but doing that now would hobble the debugging effort. */
	free_pipe_list(pi,0);
	परत rcode;
}

/* The API for glob is arguably broken.  This routine pushes a non-matching
 * string into the output structure, removing non-backslashed backslashes.
 * If someone can prove me wrong, by performing this function within the
 * original glob(3) api, feel free to rewrite this routine into oblivion.
 * Return code (0 vs. GLOB_NOSPACE) matches glob(3).
 * XXX broken if the last character is '\\', check that before calling.
 */
#ifndef __U_BOOT__
स्थिर पूर्णांक globhack(स्थिरांक अक्षर *src, पूर्णांक flags, glob_t *pglob)
{
	पूर्णांक cnt=0, pathc;
	स्थिरांक अक्षर *s;
	अक्षर *dest;
	साठी (cnt=1, s=src; s && *s; s++) {
		जर (*s == '\\') s++;
		cnt++;
	}
	dest = दो_स्मृति(cnt);
	जर (!dest) परत GLOB_NOSPACE;
	जर (!(flags & GLOB_APPEND)) {
		pglob->gl_pathv=रिक्त;
		pglob->gl_pathc=0;
		pglob->gl_offs=0;
		pglob->gl_offs=0;
	}
	pathc = ++pglob->gl_pathc;
	pglob->gl_pathv = पुनः_स्मृति(pglob->gl_pathv, (pathc+1)*आकार(*pglob->gl_pathv));
	जर (pglob->gl_pathv == रिक्त) परत GLOB_NOSPACE;
	pglob->gl_pathv[pathc-1]=dest;
	pglob->gl_pathv[pathc]=रिक्त;
	साठी (s=src; s && *s; s++, dest++) {
		जर (*s == '\\') s++;
		*dest = *s;
	}
	*dest='\0';
	परत 0;
}

/* XXX broken if the last character is '\\', check that before calling */
स्थिर पूर्णांक glob_needed(स्थिरांक अक्षर *s)
{
	साठी (; *s; s++) {
		जर (*s == '\\') s++;
		जर (म_अक्षर("*[?",*s)) परत 1;
	}
	परत 0;
}

#जर 0
स्थिर शून्य globprint(glob_t *pglob)
{
	पूर्णांक i;
	debug_printf("glob_t at %p:\n", pglob);
	debug_printf("  gl_pathc=%d  gl_pathv=%p  gl_offs=%d  gl_flags=%d\n",
		pglob->gl_pathc, pglob->gl_pathv, pglob->gl_offs, pglob->gl_flags);
	साठी (i=0; i<pglob->gl_pathc; i++)
		debug_printf("pglob->gl_pathv[%d] = %p = %s\n", i,
			pglob->gl_pathv[i], pglob->gl_pathv[i]);
}
#endif

स्थिर पूर्णांक xglob(o_string *dest, पूर्णांक flags, glob_t *pglob)
{
	पूर्णांक gr;

	/* short-circuit for null word */
	/* we can code this better when the debug_printf's are gone */
	जर (dest->length == 0) {
		जर (dest->nonnull) {
			/* bash man page calls this an "explicit" null */
			gr = globhack(dest->data, flags, pglob);
			debug_printf("globhack returned %d\n",gr);
		} अन्यथा {
			परत 0;
		}
	} अन्यथा जर (glob_needed(dest->data)) {
		gr = glob(dest->data, flags, रिक्त, pglob);
		debug_printf("glob returned %d\n",gr);
		जर (gr == GLOB_NOMATCH) {
			/* quote removal, or more accurately, backslash removal */
			gr = globhack(dest->data, flags, pglob);
			debug_printf("globhack returned %d\n",gr);
		}
	} अन्यथा {
		gr = globhack(dest->data, flags, pglob);
		debug_printf("globhack returned %d\n",gr);
	}
	जर (gr == GLOB_NOSPACE)
		error_संदेश_and_die("out of memory during glob");
	जर (gr != 0) { /* GLOB_ABORTED ? */
		error_संदेश("glob(3) error %d",gr);
	}
	/* globprint(glob_target); */
	परत gr;
}
#endif

#ifdef __U_BOOT__
स्थिर अक्षर *get_dollar_var(अक्षर ch);
#endif

/* This is used to get/check local shell variables */
अक्षर *get_local_var(स्थिरांक अक्षर *s)
{
	संरचना variables *cur;

	जर (!s)
		परत रिक्त;

#ifdef __U_BOOT__
	जर (*s == '$')
		परत get_dollar_var(s[1]);
#endif

	साठी (cur = शीर्ष_vars; cur; cur=cur->next)
		जर(म_भेद(cur->name, s)==0)
			परत cur->मूल्य;
	परत रिक्त;
}

/* This is used to set local shell variables
   flg_export==0 if only local (not exporting) variable
   flg_export==1 if "new" exporting environ
   flg_export>1  if current startup environ (not call putenv()) */
पूर्णांक set_local_var(स्थिरांक अक्षर *s, पूर्णांक flg_export)
{
	अक्षर *name, *मूल्य;
	पूर्णांक result=0;
	संरचना variables *cur;

#ifdef __U_BOOT__
	/* might be possible! */
	जर (!है_अक्षर(*s))
		परत -1;
#endif

	name=strdup(s);

	/* Assume when we enter this function that we are already in
	 * NAME=VALUE format.  So the first order of business is to
	 * split 's' on the '=' into 'name' and 'value' */
	मूल्य = म_अक्षर(name, '=');
	जर (!मूल्य) {
		मुक्त(name);
		परत -1;
	}
	*मूल्य++ = 0;

	जर (!*मूल्य) {
		unset_local_var(name);
		मुक्त(name);
		परत 0;
	}

	साठी(cur = शीर्ष_vars; cur; cur = cur->next) {
		जर(म_भेद(cur->name, name)==0)
			थांबा;
	}

	जर(cur) {
		जर(म_भेद(cur->मूल्य, मूल्य)==0) {
			जर(flg_export>0 && cur->flg_export==0)
				cur->flg_export=flg_export;
			अन्यथा
				result++;
		} अन्यथा {
			जर(cur->flg_read_only) {
				error_संदेश("%s: readonly variable", name);
				result = -1;
			} अन्यथा {
				जर(flg_export>0 || cur->flg_export>1)
					cur->flg_export=1;
				मुक्त(cur->मूल्य);

				cur->मूल्य = strdup(मूल्य);
			}
		}
	} अन्यथा {
		cur = दो_स्मृति(आकार(संरचना variables));
		जर(!cur) {
			result = -1;
		} अन्यथा {
			cur->name = strdup(name);
			जर (cur->name == रिक्त) {
				मुक्त(cur);
				result = -1;
			} अन्यथा {
				संरचना variables *bottom = शीर्ष_vars;
				cur->मूल्य = strdup(मूल्य);
				cur->next = रिक्त;
				cur->flg_export = flg_export;
				cur->flg_read_only = 0;
				पर्यंत(bottom->next) bottom=bottom->next;
				bottom->next = cur;
			}
		}
	}

#ifndef __U_BOOT__
	जर(result==0 && cur->flg_export==1) {
		*(मूल्य-1) = '=';
		result = putenv(name);
	} अन्यथा {
#endif
		मुक्त(name);
#ifndef __U_BOOT__
		जर(result>0)            /* equivalent to previous set */
			result = 0;
	}
#endif
	परत result;
}

शून्य unset_local_var(स्थिरांक अक्षर *name)
{
	संरचना variables *cur;

	जर (name) {
		साठी (cur = शीर्ष_vars; cur; cur=cur->next) {
			जर(म_भेद(cur->name, name)==0)
				थांबा;
		}
		जर (cur != रिक्त) {
			संरचना variables *next = शीर्ष_vars;
			जर(cur->flg_read_only) {
				error_संदेश("%s: readonly variable", name);
				परत;
			} अन्यथा {
#ifndef __U_BOOT__
				जर(cur->flg_export)
					unenv_set(cur->name);
#endif
				मुक्त(cur->name);
				मुक्त(cur->मूल्य);
				पर्यंत (next->next != cur)
					next = next->next;
				next->next = cur->next;
			}
			मुक्त(cur);
		}
	}
}

स्थिर पूर्णांक is_assignment(स्थिरांक अक्षर *s)
{
	जर (s == रिक्त)
		परत 0;

	जर (!है_अक्षर(*s)) परत 0;
	++s;
	पर्यंत(है_अक्षर_अंक(*s) || *s=='_') ++s;
	परत *s=='=';
}

#ifndef __U_BOOT__
/* the src parameter allows us to peek forward to a possible &n syntax
 * for file descriptor duplication, e.g., "2>&1".
 * Return code is 0 normally, 1 if a syntax error is detected in src.
 * Resource errors (in xmalloc) cause the process to exit */
स्थिर पूर्णांक setup_redirect(संरचना p_context *ctx, पूर्णांक fd, redir_type style,
	संरचना in_str *आदान)
{
	संरचना child_prog *child=ctx->child;
	संरचना redir_struct *redir = child->redirects;
	संरचना redir_struct *last_redir=रिक्त;

	/* Create a new redir_struct and drop it onto the end of the linked list */
	पर्यंत(redir) {
		last_redir=redir;
		redir=redir->next;
	}
	redir = xmalloc(आकार(संरचना redir_struct));
	redir->next=रिक्त;
	redir->word.gl_pathv=रिक्त;
	जर (last_redir) {
		last_redir->next=redir;
	} अन्यथा {
		child->redirects=redir;
	}

	redir->type=style;
	redir->fd= (fd==-1) ? redir_table[style].default_fd : fd ;

	debug_printf("Redirect type %d%s\n", redir->fd, redir_table[style].descrip);

	/* Check for a '2>&1' type redirect */
	redir->dup = redirect_dup_अंक(आदान);
	जर (redir->dup == -2) परत 1;  /* syntax error */
	जर (redir->dup != -1) {
		/* Erik had a check here that the file descriptor in question
		 * is legit; I postpone that to "run time"
		 * A "-" representation of "close me" shows up as a -3 here */
		debug_printf("Duplicating redirect '%d>&%d'\n", redir->fd, redir->dup);
	} अन्यथा {
		/* We do _not_ try to open the file that src points to,
		 * since we need to return and let src be expanded first.
		 * Set ctx->pending_redirect, so we know what to do at the
		 * end of the next parsed word.
		 */
		ctx->pending_redirect = redir;
	}
	परत 0;
}
#endif

स्थिर संरचना pipe *new_pipe(शून्य)
{
	संरचना pipe *pi;
	pi = xmalloc(आकार(संरचना pipe));
	pi->अंक_progs = 0;
	pi->progs = रिक्त;
	pi->next = रिक्त;
	pi->followup = 0;  /* invalid */
	pi->r_mode = RES_NONE;
	परत pi;
}

स्थिर शून्य initialize_context(संरचना p_context *ctx)
{
	ctx->pipe=रिक्त;
#ifndef __U_BOOT__
	ctx->pending_redirect=रिक्त;
#endif
	ctx->child=रिक्त;
	ctx->list_head=new_pipe();
	ctx->pipe=ctx->list_head;
	ctx->w=RES_NONE;
	ctx->चळत=रिक्त;
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
संरचना reserved_combo {
	अक्षर *literal;
	पूर्णांक code;
	दीर्घ flag;
};
/* Mostly a list of accepted follow-up reserved words.
 * FLAG_END means we are done with the sequence, and are ready
 * to turn the compound list into a command.
 * FLAG_START means the word must start a new compound list.
 */
स्थिर संरचना reserved_combo reserved_list[] = {
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
#define NRES (आकार(reserved_list)/आकार(संरचना reserved_combo))

स्थिर पूर्णांक reserved_word(o_string *dest, संरचना p_context *ctx)
{
	संरचना reserved_combo *r;
	साठी (r=reserved_list;
		r<reserved_list+NRES; r++) {
		जर (म_भेद(dest->data, r->literal) == 0) {
			debug_printf("found reserved word %s, code %d\n",r->literal,r->code);
			जर (r->flag & FLAG_START) {
				संरचना p_context *new = xmalloc(आकार(संरचना p_context));
				debug_printf("push stack\n");
				जर (ctx->w == RES_IN || ctx->w == RES_FOR) {
					syntax();
					मुक्त(new);
					ctx->w = RES_SNTX;
					b_reset(dest);
					परत 1;
				}
				*new = *ctx;   /* physical copy */
				initialize_context(ctx);
				ctx->चळत=new;
			} अन्यथा जर ( ctx->w == RES_NONE || ! (ctx->old_flag & (1<<r->code))) {
				syntax();
				ctx->w = RES_SNTX;
				b_reset(dest);
				परत 1;
			}
			ctx->w=r->code;
			ctx->old_flag = r->flag;
			जर (ctx->old_flag & FLAG_END) {
				संरचना p_context *old;
				debug_printf("pop stack\n");
				done_pipe(ctx,PIPE_SEQ);
				old = ctx->चळत;
				old->child->group = ctx->list_head;
#ifndef __U_BOOT__
				old->child->subshell = 0;
#endif
				*ctx = *old;   /* physical copy */
				मुक्त(old);
			}
			b_reset (dest);
			परत 1;
		}
	}
	परत 0;
}

/* normal return is 0.
 * Syntax or xglob errors return 1. */
स्थिर पूर्णांक done_word(o_string *dest, संरचना p_context *ctx)
{
	संरचना child_prog *child=ctx->child;
#ifndef __U_BOOT__
	glob_t *glob_target;
	पूर्णांक gr, flags = 0;
#अन्यथा
	अक्षर *str, *s;
	पूर्णांक argc, cnt;
#endif

	debug_printf("done_word: %s %p\n", dest->data, child);
	जर (dest->length == 0 && !dest->nonnull) {
		debug_printf("  true null, ignored\n");
		परत 0;
	}
#ifndef __U_BOOT__
	जर (ctx->pending_redirect) {
		glob_target = &ctx->pending_redirect->word;
	} अन्यथा {
#endif
		जर (child->group) {
			syntax();
			परत 1;  /* syntax error, groups and arglists don't mix */
		}
		जर (!child->argv && (ctx->type & FLAG_PARSE_SEMICOLON)) {
			debug_printf("checking %s for reserved-ness\n",dest->data);
			जर (reserved_word(dest,ctx)) परत ctx->w==RES_SNTX;
		}
#ifndef __U_BOOT__
		glob_target = &child->glob_result;
		जर (child->argv) flags |= GLOB_APPEND;
#अन्यथा
		साठी (cnt = 1, s = dest->data; s && *s; s++) {
			जर (*s == '\\') s++;
			cnt++;
		}
		str = दो_स्मृति(cnt);
		जर (!str) परत 1;
		जर ( child->argv == रिक्त) {
			child->argc=0;
		}
		argc = ++child->argc;
		child->argv = पुनः_स्मृति(child->argv, (argc+1)*आकार(*child->argv));
		जर (child->argv == रिक्त) {
			मुक्त(str);
			परत 1;
		}
		child->argv_nonnull = पुनः_स्मृति(child->argv_nonnull,
					(argc+1)*आकार(*child->argv_nonnull));
		जर (child->argv_nonnull == रिक्त) {
			मुक्त(str);
			परत 1;
		}
		child->argv[argc-1]=str;
		child->argv_nonnull[argc-1] = dest->nonnull;
		child->argv[argc]=रिक्त;
		child->argv_nonnull[argc] = 0;
		साठी (s = dest->data; s && *s; s++,str++) {
			जर (*s == '\\') s++;
			*str = *s;
		}
		*str = '\0';
#endif
#ifndef __U_BOOT__
	}
	gr = xglob(dest, flags, glob_target);
	जर (gr != 0) परत 1;
#endif

	b_reset(dest);
#ifndef __U_BOOT__
	जर (ctx->pending_redirect) {
		ctx->pending_redirect=रिक्त;
		जर (glob_target->gl_pathc != 1) {
			error_संदेश("ambiguous redirect");
			परत 1;
		}
	} अन्यथा {
		child->argv = glob_target->gl_pathv;
	}
#endif
	जर (ctx->w == RES_FOR) {
		done_word(dest,ctx);
		done_pipe(ctx,PIPE_SEQ);
	}
	परत 0;
}

/* The only possible error here is out of memory, in which case
 * xmalloc exits. */
स्थिर पूर्णांक done_command(संरचना p_context *ctx)
{
	/* The child is really already in the pipe structure, so
	 * advance the pipe counter and make a new, null child.
	 * Only real trickiness here is that the uncommitted
	 * child structure, to which ctx->child points, is not
	 * counted in pi->num_progs. */
	संरचना pipe *pi=ctx->pipe;
	संरचना child_prog *prog=ctx->child;

	जर (prog && prog->group == रिक्त
		 && prog->argv == रिक्त
#ifndef __U_BOOT__
		 && prog->redirects == रिक्त) {
#अन्यथा
										) {
#endif
		debug_printf("done_command: skipping null command\n");
		परत 0;
	} अन्यथा जर (prog) {
		pi->अंक_progs++;
		debug_printf("done_command: num_progs incremented to %d\n",pi->अंक_progs);
	} अन्यथा {
		debug_printf("done_command: initializing\n");
	}
	pi->progs = xrealloc(pi->progs, आकार(*pi->progs) * (pi->अंक_progs+1));

	prog = pi->progs + pi->अंक_progs;
#ifndef __U_BOOT__
	prog->redirects = रिक्त;
#endif
	prog->argv = रिक्त;
	prog->argv_nonnull = रिक्त;
#ifndef __U_BOOT__
	prog->is_stopped = 0;
#endif
	prog->group = रिक्त;
#ifndef __U_BOOT__
	prog->glob_result.gl_pathv = रिक्त;
	prog->family = pi;
#endif
	prog->sp = 0;
	ctx->child = prog;
	prog->type = ctx->type;

	/* but ctx->pipe and ctx->list_head remain unchanged */
	परत 0;
}

स्थिर पूर्णांक done_pipe(संरचना p_context *ctx, pipe_style type)
{
	संरचना pipe *new_p;
	done_command(ctx);  /* implicit closure of previous command */
	debug_printf("done_pipe, type %d\n", type);
	ctx->pipe->followup = type;
	ctx->pipe->r_mode = ctx->w;
	new_p=new_pipe();
	ctx->pipe->next = new_p;
	ctx->pipe = new_p;
	ctx->child = रिक्त;
	done_command(ctx);  /* set up new pipe to accept commands */
	परत 0;
}

#ifndef __U_BOOT__
/* peek ahead in the in_str to find out if we have a "&n" construct,
 * as in "2>&1", that represents duplicating a file descriptor.
 * returns either -2 (syntax error), -1 (no &), or the number found.
 */
स्थिर पूर्णांक redirect_dup_अंक(संरचना in_str *आदान)
{
	पूर्णांक ch, d=0, ok=0;
	ch = b_peek(आदान);
	जर (ch != '&') परत -1;

	b_getch(आदान);  /* get the & */
	ch=b_peek(आदान);
	जर (ch == '-') {
		b_getch(आदान);
		परत -3;  /* "-" represents "close me" */
	}
	पर्यंत (है_अंक(ch)) {
		d = d*10+(ch-'0');
		ok=1;
		b_getch(आदान);
		ch = b_peek(आदान);
	}
	जर (ok) परत d;

	error_संदेश("ambiguous redirect");
	परत -2;
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
स्थिर पूर्णांक redirect_opt_अंक(o_string *o)
{
	पूर्णांक अंक;

	जर (o->length==0) परत -1;
	साठी(अंक=0; अंक<o->length; अंक++) {
		जर (!है_अंक(*(o->data+अंक))) {
			परत -1;
		}
	}
	/* reuse num (and save an int) */
	अंक=म_से_प(o->data);
	b_reset(o);
	परत अंक;
}

खाता *generate_stream_from_list(संरचना pipe *head)
{
	खाता *pf;
#जर 1
	पूर्णांक pid, channel[2];
	जर (pipe(channel)<0) perror_संदेश_and_die("pipe");
	pid=विभाजन();
	जर (pid<0) {
		perror_संदेश_and_die("fork");
	} अन्यथा जर (pid==0) {
		बंद(channel[0]);
		जर (channel[1] != 1) {
			dup2(channel[1],1);
			बंद(channel[1]);
		}
#जर 0
#define SURROGATE "surrogate response"
		लिख(1,SURROGATE,आकार(SURROGATE));
		_exit(run_list(head));
#अन्यथा
		_exit(run_list_real(head));   /* leaks memory */
#endif
	}
	debug_printf("forked child %d\n",pid);
	बंद(channel[1]);
	pf = fdopen(channel[0],"r");
	debug_printf("pipe on FILE *%p\n",pf);
#अन्यथा
	free_pipe_list(head,0);
	pf=popen("echo surrogate response","r");
	debug_printf("started fake pipe on FILE *%p\n",pf);
#endif
	परत pf;
}

/* this version hacked for testing purposes */
/* return code is exit status of the process that is run. */
स्थिर पूर्णांक process_command_subs(o_string *dest, संरचना p_context *ctx, संरचना in_str *आदान, पूर्णांक subst_end)
{
	पूर्णांक retcode;
	o_string result=NULL_O_STRING;
	संरचना p_context inner;
	खाता *p;
	संरचना in_str pipe_str;
	initialize_context(&inner);

	/* recursion to generate command */
	retcode = parse_stream(&result, &inner, आदान, subst_end);
	जर (retcode != 0) परत retcode;  /* syntax error or EOF */
	done_word(&result, &inner);
	done_pipe(&inner, PIPE_SEQ);
	b_free(&result);

	p=generate_stream_from_list(inner.list_head);
	जर (p==रिक्त) परत 1;
	mark_open(fileno(p));
	setup_file_in_str(&pipe_str, p);

	/* now send results of command back into original context */
	retcode = parse_stream(dest, ctx, &pipe_str, '\0');
	/* XXX In case of a syntax error, should we try to kill the child?
	 * That would be tough to do right, so just read until EOF. */
	जर (retcode == 1) {
		पर्यंत (b_getch(&pipe_str)!=खातापूर्ण) { /* discard */ };
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
	परत retcode;
}

स्थिर पूर्णांक parse_group(o_string *dest, संरचना p_context *ctx,
	संरचना in_str *आदान, पूर्णांक ch)
{
	पूर्णांक rcode, endch=0;
	संरचना p_context sub;
	संरचना child_prog *child = ctx->child;
	जर (child->argv) {
		syntax();
		परत 1;  /* syntax error, groups and arglists don't mix */
	}
	initialize_context(&sub);
	निवड(ch) {
		स्थिती '(': endch=')'; child->subshell=1; थांबा;
		स्थिती '{': endch='}'; थांबा;
		पूर्वनिर्धारित: syntax();   /* really logic error */
	}
	rcode=parse_stream(dest,&sub,आदान,endch);
	done_word(dest,&sub); /* finish off the final word in the subcontext */
	done_pipe(&sub, PIPE_SEQ);  /* and the final command there, too */
	child->group = sub.list_head;
	परत rcode;
	/* child remains "open", available for possible redirects */
}
#endif

/* basically useful version until someone wants to get fancier,
 * see the bash man page under "Parameter Expansion" */
स्थिर अक्षर *lookup_param(अक्षर *src)
{
	अक्षर *p;
	अक्षर *sep;
	अक्षर *default_val = रिक्त;
	पूर्णांक assign = 0;
	पूर्णांक expand_empty = 0;

	जर (!src)
		परत रिक्त;

	sep = म_अक्षर(src, ':');

	जर (sep) {
		*sep = '\0';
		जर (*(sep + 1) == '-')
			default_val = sep+2;
		जर (*(sep + 1) == '=') {
			default_val = sep+2;
			assign = 1;
		}
		जर (*(sep + 1) == '+') {
			default_val = sep+2;
			expand_empty = 1;
		}
	}

	p = env_get(src);
	जर (!p)
		p = get_local_var(src);

	जर (!p || म_माप(p) == 0) {
		p = default_val;
		जर (assign) {
			अक्षर *var = दो_स्मृति(म_माप(src)+म_माप(default_val)+2);
			जर (var) {
				प्र_लिखो(var, "%s=%s", src, default_val);
				set_local_var(var, 0);
			}
			मुक्त(var);
		}
	} अन्यथा जर (expand_empty) {
		p += म_माप(p);
	}

	जर (sep)
		*sep = ':';

	परत p;
}

#ifdef __U_BOOT__
स्थिर अक्षर *get_dollar_var(अक्षर ch)
{
	स्थिर अक्षर buf[40];

	buf[0] = '\0';
	निवड (ch) {
		स्थिती '?':
			प्र_लिखो(buf, "%u", (अचिन्हित पूर्णांक)last_return_code);
			थांबा;
		पूर्वनिर्धारित:
			परत रिक्त;
	}
	परत buf;
}
#endif

/* return code: 0 for OK, 1 for syntax error */
स्थिर पूर्णांक handle_dollar(o_string *dest, संरचना p_context *ctx, संरचना in_str *आदान)
{
#ifndef __U_BOOT__
	पूर्णांक i, advance=0;
#अन्यथा
	पूर्णांक advance=0;
#endif
#ifndef __U_BOOT__
	अक्षर sep[]=" ";
#endif
	पूर्णांक ch = आदान->peek(आदान);  /* first character after the $ */
	debug_printf("handle_dollar: ch=%c\n",ch);
	जर (है_अक्षर(ch)) {
		b_addchr(dest, SPECIAL_VAR_चिन्ह);
		ctx->child->sp++;
		पर्यंत(ch=b_peek(आदान),है_अक्षर_अंक(ch) || ch=='_') {
			b_getch(आदान);
			b_addchr(dest,ch);
		}
		b_addchr(dest, SPECIAL_VAR_चिन्ह);
#ifndef __U_BOOT__
	} अन्यथा जर (है_अंक(ch)) {
		i = ch-'0';  /* XXX is $0 special? */
		जर (i<global_argc) {
			parse_string(dest, ctx, global_argv[i]); /* recursion */
		}
		advance = 1;
#endif
	} अन्यथा निवड (ch) {
#ifndef __U_BOOT__
		स्थिती '$':
			b_adduint(dest,getpid());
			advance = 1;
			थांबा;
		स्थिती '!':
			जर (last_bg_pid > 0) b_adduint(dest, last_bg_pid);
			advance = 1;
			थांबा;
#endif
		स्थिती '?':
#ifndef __U_BOOT__
			b_adduint(dest,last_return_code);
#अन्यथा
			ctx->child->sp++;
			b_addchr(dest, SPECIAL_VAR_चिन्ह);
			b_addchr(dest, '$');
			b_addchr(dest, '?');
			b_addchr(dest, SPECIAL_VAR_चिन्ह);
#endif
			advance = 1;
			थांबा;
#ifndef __U_BOOT__
		स्थिती '#':
			b_adduint(dest,global_argc ? global_argc-1 : 0);
			advance = 1;
			थांबा;
#endif
		स्थिती '{':
			b_addchr(dest, SPECIAL_VAR_चिन्ह);
			ctx->child->sp++;
			b_getch(आदान);
			/* XXX maybe someone will try to escape the '}' */
			पर्यंत(ch=b_getch(आदान),ch!=खातापूर्ण && ch!='}') {
				b_addchr(dest,ch);
			}
			जर (ch != '}') {
				syntax();
				परत 1;
			}
			b_addchr(dest, SPECIAL_VAR_चिन्ह);
			थांबा;
#ifndef __U_BOOT__
		स्थिती '(':
			b_getch(आदान);
			process_command_subs(dest, ctx, आदान, ')');
			थांबा;
		स्थिती '*':
			sep[0]=ifs[0];
			साठी (i=1; i<global_argc; i++) {
				parse_string(dest, ctx, global_argv[i]);
				जर (i+1 < global_argc) parse_string(dest, ctx, sep);
			}
			थांबा;
		स्थिती '@':
		स्थिती '-':
		स्थिती '_':
			/* still unhandled, but should be eventually */
			error_संदेश("unhandled syntax: $%c",ch);
			परत 1;
			थांबा;
#endif
		पूर्वनिर्धारित:
			b_addqchr(dest,'$',dest->quote);
	}
	/* Eat the character if the flag was set.  If the compiler
	 * is smart enough, we could substitute "b_getch(input);"
	 * for all the "advance = 1;" above, and also end up with
	 * a nice size-optimized program.  Hah!  That'll be the day.
	 */
	जर (advance) b_getch(आदान);
	परत 0;
}

#ifndef __U_BOOT__
पूर्णांक parse_string(o_string *dest, संरचना p_context *ctx, स्थिरांक अक्षर *src)
{
	संरचना in_str foo;
	setup_string_in_str(&foo, src);
	परत parse_stream(dest, ctx, &foo, '\0');
}
#endif

/* return code is 0 for normal exit, 1 for syntax error */
स्थिर पूर्णांक parse_stream(o_string *dest, संरचना p_context *ctx,
			संरचना in_str *आदान, पूर्णांक end_trigger)
{
	अचिन्हित पूर्णांक ch, m;
#ifndef __U_BOOT__
	पूर्णांक redir_fd;
	redir_type redir_style;
#endif
	पूर्णांक next;

	/* Only double-quote state is handled in the state variable dest->quote.
	 * A single-quote triggers a bypass of the main loop until its mate is
	 * found.  When recursing, quote state is passed in via dest->quote. */

	debug_printf("parse_stream, end_trigger=%d\n",end_trigger);
	पर्यंत ((ch=b_getch(आदान))!=खातापूर्ण) {
		m = map[ch];
#ifdef __U_BOOT__
		जर (आदान->__promptme == 0) परत 1;
#endif
		next = (ch == '\n') ? 0 : b_peek(आदान);

		debug_printf("parse_stream: ch=%c (%d) m=%d quote=%d - %c\n",
			ch >= ' ' ? ch : '.', ch, m,
			dest->quote, ctx->चळत == रिक्त ? '*' : '.');

		जर (m==0 || ((m==1 || m==2) && dest->quote)) {
			b_addqchr(dest, ch, dest->quote);
		} अन्यथा {
			जर (m==2) {  /* unquoted IFS */
				जर (done_word(dest, ctx)) {
					परत 1;
				}
				/* If we aren't performing a substitution, treat a newline as a
				 * command separator.  */
				जर (end_trigger != '\0' && ch=='\n')
					done_pipe(ctx,PIPE_SEQ);
			}
			जर (ch == end_trigger && !dest->quote && ctx->w==RES_NONE) {
				debug_printf("leaving parse_stream (triggered)\n");
				परत 0;
			}
#जर 0
			जर (ch=='\n') {
				/* Yahoo!  Time to run with it! */
				done_pipe(ctx,PIPE_SEQ);
				run_list(ctx->list_head);
				initialize_context(ctx);
			}
#endif
			जर (m!=2) निवड (ch) {
		स्थिती '#':
			जर (dest->length == 0 && !dest->quote) {
				पर्यंत(ch=b_peek(आदान),ch!=खातापूर्ण && ch!='\n') { b_getch(आदान); }
			} अन्यथा {
				b_addqchr(dest, ch, dest->quote);
			}
			थांबा;
		स्थिती '\\':
			जर (next == खातापूर्ण) {
				syntax();
				परत 1;
			}
			b_addqchr(dest, '\\', dest->quote);
			b_addqchr(dest, b_getch(आदान), dest->quote);
			थांबा;
		स्थिती '$':
			जर (handle_dollar(dest, ctx, आदान)!=0) परत 1;
			थांबा;
		स्थिती '\'':
			dest->nonnull = 1;
			पर्यंत(ch=b_getch(आदान),ch!=खातापूर्ण && ch!='\'') {
#ifdef __U_BOOT__
				जर(आदान->__promptme == 0) परत 1;
#endif
				b_addchr(dest,ch);
			}
			जर (ch==खातापूर्ण) {
				syntax();
				परत 1;
			}
			थांबा;
		स्थिती '"':
			dest->nonnull = 1;
			dest->quote = !dest->quote;
			थांबा;
#ifndef __U_BOOT__
		स्थिती '`':
			process_command_subs(dest, ctx, आदान, '`');
			थांबा;
		स्थिती '>':
			redir_fd = redirect_opt_अंक(dest);
			done_word(dest, ctx);
			redir_style=REDIRECT_OVERWRITE;
			जर (next == '>') {
				redir_style=REDIRECT_APPEND;
				b_getch(आदान);
			} अन्यथा जर (next == '(') {
				syntax();   /* until we support >(list) Process Substitution */
				परत 1;
			}
			setup_redirect(ctx, redir_fd, redir_style, आदान);
			थांबा;
		स्थिती '<':
			redir_fd = redirect_opt_अंक(dest);
			done_word(dest, ctx);
			redir_style=REDIRECT_आदान;
			जर (next == '<') {
				redir_style=REDIRECT_HEREIS;
				b_getch(आदान);
			} अन्यथा जर (next == '>') {
				redir_style=REDIRECT_IO;
				b_getch(आदान);
			} अन्यथा जर (next == '(') {
				syntax();   /* until we support <(list) Process Substitution */
				परत 1;
			}
			setup_redirect(ctx, redir_fd, redir_style, आदान);
			थांबा;
#endif
		स्थिती ';':
			done_word(dest, ctx);
			done_pipe(ctx,PIPE_SEQ);
			थांबा;
		स्थिती '&':
			done_word(dest, ctx);
			जर (next=='&') {
				b_getch(आदान);
				done_pipe(ctx,PIPE_AND);
			} अन्यथा {
#ifndef __U_BOOT__
				done_pipe(ctx,PIPE_BG);
#अन्यथा
				syntax_err();
				परत 1;
#endif
			}
			थांबा;
		स्थिती '|':
			done_word(dest, ctx);
			जर (next=='|') {
				b_getch(आदान);
				done_pipe(ctx,PIPE_OR);
			} अन्यथा {
				/* we could pick up a file descriptor choice here
				 * with redirect_opt_num(), but bash doesn't do it.
				 * "echo foo 2| cat" yields "foo 2". */
#ifndef __U_BOOT__
				done_command(ctx);
#अन्यथा
				syntax_err();
				परत 1;
#endif
			}
			थांबा;
#ifndef __U_BOOT__
		स्थिती '(':
		स्थिती '{':
			जर (parse_group(dest, ctx, आदान, ch)!=0) परत 1;
			थांबा;
		स्थिती ')':
		स्थिती '}':
			syntax();   /* Proper use of this character caught by end_trigger */
			परत 1;
			थांबा;
#endif
		स्थिती SUBSTED_VAR_चिन्ह:
			dest->nonnull = 1;
			पर्यंत (ch = b_getch(आदान), ch != खातापूर्ण &&
			    ch != SUBSTED_VAR_चिन्ह) {
				debug_printf("subst, pass=%d\n", ch);
				जर (आदान->__promptme == 0)
					परत 1;
				b_addchr(dest, ch);
			}
			debug_printf("subst, term=%d\n", ch);
			जर (ch == खातापूर्ण) {
				syntax();
				परत 1;
			}
			थांबा;
		पूर्वनिर्धारित:
			syntax();   /* this is really an internal logic error */
			परत 1;
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
	जर (end_trigger != '\0') परत -1;
	परत 0;
}

स्थिर शून्य mapset(स्थिरांक अचिन्हित अक्षर *set, पूर्णांक code)
{
	स्थिरांक अचिन्हित अक्षर *s;
	साठी (s=set; *s; s++) map[*s] = code;
}

स्थिर शून्य update_ifs_map(शून्य)
{
	/* char *ifs and char map[256] are both globals. */
	ifs = (uchar *)env_get("IFS");
	जर (ifs == रिक्त) ifs=(uchar *)" \t\n";
	/* Precompute a list of 'flow through' behavior so it can be treated
	 * quickly up front.  Computation is necessary because of IFS.
	 * Special case handling of IFS == " \t\n" is not implemented.
	 * The map[] array only really needs two bits each, and on most machines
	 * that would be faster because of the reduced L1 cache footprint.
	 */
	स_रखो(map,0,आकार(map)); /* most characters flow through always */
#ifndef __U_BOOT__
	mapset((uchar *)"\\$'\"`", 3);      /* never flow through */
	mapset((uchar *)"<>;&|(){}#", 1);   /* flow through if quoted */
#अन्यथा
	{
		uchar subst[2] = {SUBSTED_VAR_चिन्ह, 0};
		mapset(subst, 3);       /* never flow through */
	}
	mapset((uchar *)"\\$'\"", 3);       /* never flow through */
	mapset((uchar *)";&|#", 1);         /* flow through if quoted */
#endif
	mapset(ifs, 2);            /* also flow through if quoted */
}

/* most recursion does not come through here, the exeception is
 * from builtin_source() */
स्थिर पूर्णांक parse_stream_outer(संरचना in_str *inp, पूर्णांक flag)
{

	संरचना p_context ctx;
	o_string temp=NULL_O_STRING;
	पूर्णांक rcode;
#ifdef __U_BOOT__
	पूर्णांक code = 1;
#endif
	करा {
		ctx.type = flag;
		initialize_context(&ctx);
		update_ifs_map();
		जर (!(flag & FLAG_PARSE_SEMICOLON) || (flag & FLAG_REPARSING)) mapset((uchar *)";$&|", 0);
		inp->promptmode=1;
		rcode = parse_stream(&temp, &ctx, inp,
				     flag & FLAG_CONT_ON_NEWLINE ? -1 : '\n');
#ifdef __U_BOOT__
		जर (rcode == 1) flag_repeat = 0;
#endif
		जर (rcode != 1 && ctx.old_flag != 0) {
			syntax();
#ifdef __U_BOOT__
			flag_repeat = 0;
#endif
		}
		जर (rcode != 1 && ctx.old_flag == 0) {
			done_word(&temp, &ctx);
			done_pipe(&ctx,PIPE_SEQ);
#ifndef __U_BOOT__
			run_list(ctx.list_head);
#अन्यथा
			code = run_list(ctx.list_head);
			जर (code == -2) {	/* exit */
				b_free(&temp);
				code = 0;
				/* XXX hackish way to not allow exit from main loop */
				जर (inp->peek == file_peek) {
					म_लिखो("exit not allowed from main input shell.\n");
					सुरूठेवा;
				}
				/*
				 * DANGER
				 * Return code -2 is special in this context,
				 * it indicates exit from inner pipe instead
				 * of return code itself, the return code is
				 * stored in 'last_return_code' variable!
				 * DANGER
				 */
				परत -2;
			}
			जर (code == -1)
			    flag_repeat = 0;
#endif
		} अन्यथा {
			जर (ctx.old_flag != 0) {
				मुक्त(ctx.चळत);
				b_reset(&temp);
			}
#ifdef __U_BOOT__
			जर (inp->__promptme == 0) म_लिखो("<INTERRUPT>\n");
			inp->__promptme = 1;
#endif
			temp.nonnull = 0;
			temp.quote = 0;
			inp->p = रिक्त;
			free_pipe_list(ctx.list_head,0);
		}
		b_free(&temp);
	/* loop on syntax errors, return on EOF */
	} पर्यंत (rcode != -1 && !(flag & FLAG_EXIT_FROM_LOOP) &&
		(inp->peek != static_peek || b_peek(inp)));
#ifndef __U_BOOT__
	परत 0;
#अन्यथा
	परत (code != 0) ? 1 : 0;
#endif /* __U_BOOT__ */
}

#ifndef __U_BOOT__
स्थिर पूर्णांक parse_string_outer(स्थिरांक अक्षर *s, पूर्णांक flag)
#अन्यथा
पूर्णांक parse_string_outer(स्थिरांक अक्षर *s, पूर्णांक flag)
#endif	/* __U_BOOT__ */
{
	संरचना in_str आदान;
	पूर्णांक rcode;
#ifdef __U_BOOT__
	अक्षर *p = रिक्त;
	जर (!s)
		परत 1;
	जर (!*s)
		परत 0;
	जर (!(p = म_अक्षर(s, '\n')) || *++p) {
		p = xmalloc(म_माप(s) + 2);
		म_नकल(p, s);
		म_जोड़ो(p, "\n");
		setup_string_in_str(&आदान, p);
		rcode = parse_stream_outer(&आदान, flag);
		मुक्त(p);
		परत rcode == -2 ? last_return_code : rcode;
	} अन्यथा {
#endif
	setup_string_in_str(&आदान, s);
	rcode = parse_stream_outer(&आदान, flag);
	परत rcode == -2 ? last_return_code : rcode;
#ifdef __U_BOOT__
	}
#endif
}

#ifndef __U_BOOT__
स्थिर पूर्णांक parse_file_outer(खाता *f)
#अन्यथा
पूर्णांक parse_file_outer(शून्य)
#endif
{
	पूर्णांक rcode;
	संरचना in_str आदान;
#ifndef __U_BOOT__
	setup_file_in_str(&आदान, f);
#अन्यथा
	setup_file_in_str(&आदान);
#endif
	rcode = parse_stream_outer(&आदान, FLAG_PARSE_SEMICOLON);
	परत rcode == -2 ? last_return_code : rcode;
}

#ifdef __U_BOOT__
पूर्णांक u_boot_hush_start(शून्य)
{
	जर (शीर्ष_vars == रिक्त) {
		शीर्ष_vars = दो_स्मृति(आकार(संरचना variables));
		शीर्ष_vars->name = "HUSH_VERSION";
		शीर्ष_vars->मूल्य = "0.01";
		शीर्ष_vars->next = रिक्त;
		शीर्ष_vars->flg_export = 0;
		शीर्ष_vars->flg_read_only = 1;
	}
	परत 0;
}

स्थिर शून्य *xmalloc(माप_प्रकार size)
{
	शून्य *p = रिक्त;

	जर (!(p = दो_स्मृति(size))) {
	    म_लिखो("ERROR : xmalloc failed\n");
	    साठी(;;);
	}
	परत p;
}

स्थिर शून्य *xrealloc(शून्य *ptr, माप_प्रकार size)
{
	शून्य *p = रिक्त;

	जर (!(p = पुनः_स्मृति(ptr, size))) {
	    म_लिखो("ERROR : xrealloc failed\n");
	    साठी(;;);
	}
	परत p;
}
#endif /* __U_BOOT__ */

#ifndef __U_BOOT__
/* Make sure we have a controlling tty.  If we get started under a job
 * aware app (like bash for example), make sure we are now in charge so
 * we don't fight over who gets the foreground */
स्थिर शून्य setup_job_control(शून्य)
{
	स्थिर pid_t shell_pgrp;
	/* Loop until we are in the foreground.  */
	पर्यंत (tcgetpgrp (shell_terminal) != (shell_pgrp = getpgrp ()))
		समाप्त (- shell_pgrp, SIGTTIN);

	/* Ignore interactive and job-control signals.  */
	संकेत(संक_विघ्न, संक_छोड़ो);
	संकेत(SIGQUIT, संक_छोड़ो);
	संकेत(संक_इति, संक_छोड़ो);
	संकेत(SIGTSTP, संक_छोड़ो);
	संकेत(SIGTTIN, संक_छोड़ो);
	संकेत(SIGTTOU, संक_छोड़ो);
	संकेत(SIGCHLD, संक_छोड़ो);

	/* Put ourselves in our own process group.  */
	setsid();
	shell_pgrp = getpid ();
	setpgid (shell_pgrp, shell_pgrp);

	/* Grab control of the terminal.  */
	tcsetpgrp(shell_terminal, shell_pgrp);
}

पूर्णांक hush_main(पूर्णांक argc, अक्षर * स्थिरांक *argv)
{
	पूर्णांक opt;
	खाता *आदान;
	अक्षर **e = environ;

	/* XXX what should these be while sourcing /etc/profile? */
	global_argc = argc;
	global_argv = argv;

	/* (re?) initialize globals.  Sometimes hush_main() ends up calling
	 * hush_main(), therefore we cannot rely on the BSS to zero out this
	 * stuff.  Reset these to 0 every time. */
	ifs = रिक्त;
	/* map[] is taken care of with call to update_ifs_map() */
	fake_mode = 0;
	interactive = 0;
	close_me_head = रिक्त;
	last_bg_pid = 0;
	job_list = रिक्त;
	last_jobid = 0;

	/* Initialize some more globals to non-zero values */
	set_cwd();
#ifdef CONFIG_FEATURE_COMMAND_EDITING
	cmdedit_set_initial_prompt();
#अन्यथा
	PS1 = रिक्त;
#endif
	PS2 = "> ";

	/* initialize our shell local variables with the values
	 * currently living in the environment */
	जर (e) {
		साठी (; *e; e++)
			set_local_var(*e, 2);   /* without call putenv() */
	}

	last_return_code=निकास_सफल;

	जर (argv[0] && argv[0][0] == '-') {
		debug_printf("\nsourcing /etc/profile\n");
		जर ((आदान = ख_खोलो("/etc/profile", "r")) != रिक्त) {
			mark_open(fileno(आदान));
			parse_file_outer(आदान);
			mark_closed(fileno(आदान));
			ख_बंद(आदान);
		}
	}
	आदान=मानक_निवेश;

	पर्यंत ((opt = getopt(argc, argv, "c:xif")) > 0) {
		निवड (opt) {
			स्थिती 'c':
				{
					global_argv = argv+optind;
					global_argc = argc-optind;
					opt = parse_string_outer(optarg, FLAG_PARSE_SEMICOLON);
					जा final_return;
				}
				थांबा;
			स्थिती 'i':
				interactive++;
				थांबा;
			स्थिती 'f':
				fake_mode++;
				थांबा;
			पूर्वनिर्धारित:
#ifndef BB_VER
				ख_लिखो(मानक_त्रुटि, "Usage: sh [FILE]...\n"
						"   or: sh -c command [args]...\n\n");
				निकास(निकास_त्रुटि);
#अन्यथा
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
	जर (argv[optind]==रिक्त && आदान==मानक_निवेश &&
			isatty(fileno(मानक_निवेश)) && isatty(fileno(मानक_निकास))) {
		interactive++;
	}

	debug_printf("\ninteractive=%d\n", interactive);
	जर (interactive) {
		/* Looks like they want an interactive shell */
#ifndef CONFIG_FEATURE_SH_EXTRA_QUIET
		म_लिखो( "\n\n" BB_BANNER " hush - the humble shell v0.01 (testing)\n");
		म_लिखो( "Enter 'help' for a list of built-in commands.\n\n");
#endif
		setup_job_control();
	}

	जर (argv[optind]==रिक्त) {
		opt=parse_file_outer(मानक_निवेश);
		जा final_return;
	}

	debug_printf("\nrunning script '%s'\n", argv[optind]);
	global_argv = argv+optind;
	global_argc = argc-optind;
	आदान = xfopen(argv[optind], "r");
	opt = parse_file_outer(आदान);

#ifdef CONFIG_FEATURE_CLEAN_UP
	ख_बंद(आदान);
	जर (cwd && cwd != unknown)
		मुक्त((अक्षर*)cwd);
	{
		संरचना variables *cur, *tmp;
		साठी(cur = शीर्ष_vars; cur; cur = tmp) {
			tmp = cur->next;
			जर (!cur->flg_read_only) {
				मुक्त(cur->name);
				मुक्त(cur->मूल्य);
				मुक्त(cur);
			}
		}
	}
#endif

final_return:
	परत(opt?opt:last_return_code);
}
#endif

स्थिर अक्षर *घाला_var_मूल्य(अक्षर *inp)
{
	परत घाला_var_मूल्य_sub(inp, 0);
}

स्थिर अक्षर *घाला_var_मूल्य_sub(अक्षर *inp, पूर्णांक tag_subst)
{
	पूर्णांक res_str_len = 0;
	पूर्णांक len;
	पूर्णांक done = 0;
	अक्षर *p, *p1, *res_str = रिक्त;

	पर्यंत ((p = म_अक्षर(inp, SPECIAL_VAR_चिन्ह))) {
		/* check the beginning of the string for normal characters */
		जर (p != inp) {
			/* copy any characters to the result string */
			len = p - inp;
			res_str = xrealloc(res_str, (res_str_len + len));
			म_नकलन((res_str + res_str_len), inp, len);
			res_str_len += len;
		}
		inp = ++p;
		/* find the ending marker */
		p = म_अक्षर(inp, SPECIAL_VAR_चिन्ह);
		*p = '\0';
		/* look up the value to substitute */
		जर ((p1 = lookup_param(inp))) {
			जर (tag_subst)
				len = res_str_len + म_माप(p1) + 2;
			अन्यथा
				len = res_str_len + म_माप(p1);
			res_str = xrealloc(res_str, (1 + len));
			जर (tag_subst) {
				/*
				 * copy the variable value to the result
				 * string
				 */
				म_नकल((res_str + res_str_len + 1), p1);

				/*
				 * mark the replaced text to be accepted as
				 * is
				 */
				res_str[res_str_len] = SUBSTED_VAR_चिन्ह;
				res_str[res_str_len + 1 + म_माप(p1)] =
					SUBSTED_VAR_चिन्ह;
			} अन्यथा
				/*
				 * copy the variable value to the result
				 * string
				 */
				म_नकल((res_str + res_str_len), p1);

			res_str_len = len;
		}
		*p = SPECIAL_VAR_चिन्ह;
		inp = ++p;
		done = 1;
	}
	जर (done) {
		res_str = xrealloc(res_str, (1 + res_str_len + म_माप(inp)));
		म_नकल((res_str + res_str_len), inp);
		पर्यंत ((p = म_अक्षर(res_str, '\n'))) {
			*p = ' ';
		}
	}
	परत (res_str == रिक्त) ? inp : res_str;
}

स्थिर अक्षर **make_list_in(अक्षर **inp, अक्षर *name)
{
	पूर्णांक len, i;
	पूर्णांक name_len = म_माप(name);
	पूर्णांक n = 0;
	अक्षर **list;
	अक्षर *p1, *p2, *p3;

	/* create list of variable values */
	list = xmalloc(आकार(*list));
	साठी (i = 0; inp[i]; i++) {
		p3 = घाला_var_मूल्य(inp[i]);
		p1 = p3;
		पर्यंत (*p1) {
			जर (*p1 == ' ') {
				p1++;
				सुरूठेवा;
			}
			जर ((p2 = म_अक्षर(p1, ' '))) {
				len = p2 - p1;
			} अन्यथा {
				len = म_माप(p1);
				p2 = p1 + len;
			}
			/* we use n + 2 in realloc for list,because we add
			 * new element and then we will add NULL element */
			list = xrealloc(list, आकार(*list) * (n + 2));
			list[n] = xmalloc(2 + name_len + len);
			म_नकल(list[n], name);
			म_जोड़ो(list[n], "=");
			म_जोड़न(list[n], p1, len);
			list[n++][name_len + len + 1] = '\0';
			p1 = p2;
		}
		जर (p3 != inp[i]) मुक्त(p3);
	}
	list[n] = रिक्त;
	परत list;
}

/*
 * Make new string for parser
 * inp     - array of argument strings to flatten
 * nonnull - indicates argument was quoted when originally parsed
 */
स्थिर अक्षर *make_string(अक्षर **inp, पूर्णांक *nonnull)
{
	अक्षर *p;
	अक्षर *str = रिक्त;
	पूर्णांक n;
	पूर्णांक len = 2;
	अक्षर *noeval_str;
	पूर्णांक noeval = 0;

	noeval_str = get_local_var("HUSH_NO_EVAL");
	जर (noeval_str != रिक्त && *noeval_str != '0' && *noeval_str != '\0')
		noeval = 1;
	साठी (n = 0; inp[n]; n++) {
		p = घाला_var_मूल्य_sub(inp[n], noeval);
		अक्षर *new_str = xrealloc(str, (len + म_माप(p) + (2 * nonnull[n])));
		जर (!new_str) {
			मुक्त(str);
			जर (p != inp[n]) मुक्त(p);
			परत रिक्त;
		}
		str = new_str;
		जर (n) {
			म_जोड़ो(str, " ");
		} अन्यथा {
			*str = '\0';
		}
		जर (nonnull[n])
			म_जोड़ो(str, "'");
		म_जोड़ो(str, p);
		जर (nonnull[n])
			म_जोड़ो(str, "'");
		len = म_माप(str) + 3;
		जर (p != inp[n]) मुक्त(p);
	}
	len = म_माप(str);
	*(str + len) = '\n';
	*(str + len + 1) = '\0';
	परत str;
}

#ifdef __U_BOOT__
स्थिर पूर्णांक do_showvar(संरचना cmd_tbl *cmdtp, पूर्णांक flag, पूर्णांक argc,
		      अक्षर *स्थिरांक argv[])
{
	पूर्णांक i, k;
	पूर्णांक rcode = 0;
	संरचना variables *cur;

	जर (argc == 1) {		/* Print all env variables	*/
		साठी (cur = शीर्ष_vars; cur; cur = cur->next) {
			म_लिखो ("%s=%s\n", cur->name, cur->मूल्य);
			जर (ctrlc ()) {
				माला_दो ("\n ** Abort\n");
				परत 1;
			}
		}
		परत 0;
	}
	साठी (i = 1; i < argc; ++i) {	/* print single env variables	*/
		अक्षर *name = argv[i];

		k = -1;
		साठी (cur = शीर्ष_vars; cur; cur = cur->next) {
			जर(म_भेद (cur->name, name) == 0) {
				k = 0;
				म_लिखो ("%s=%s\n", cur->name, cur->मूल्य);
			}
			जर (ctrlc ()) {
				माला_दो ("\n ** Abort\n");
				परत 1;
			}
		}
		जर (k < 0) {
			म_लिखो ("## Error: \"%s\" not defined\n", name);
			rcode ++;
		}
	}
	परत rcode;
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
