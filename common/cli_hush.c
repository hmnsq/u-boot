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

#यदि 1
#अन्तर्भाव "busybox.h"
#अन्तर्भाव "cmdedit.h"
#अन्यथा
#define applet_name "hush"
#अन्तर्भाव "standalone.h"
#define hush_main main
#define BB_BANNER
#endif
#endif
#define SPECIAL_VAR_प्रतीक 03
#define SUBSTED_VAR_प्रतीक 04
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
#define error_संदेश printf
#अन्यथा
प्रकारनिर्देश गणना {
	REDIRECT_निवेश     = 1,
	REDIRECT_OVERWRITE = 2,
	REDIRECT_APPEND    = 3,
	REDIRECT_HEREIS    = 4,
	REDIRECT_IO        = 5
} redir_type;

/* The descrip member of this structure is only used to make debugging
 * output pretty */
संरचना {पूर्णाङ्क mode; पूर्णाङ्क default_fd; वर्ण *descrip;} redir_table[] = {
	{ 0,                         0, "()" },
	{ O_RDONLY,                  0, "<"  },
	{ O_CREAT|O_TRUNC|O_WRONLY,  1, ">"  },
	{ O_CREAT|O_APPEND|O_WRONLY, 1, ">>" },
	{ O_RDONLY,                 -1, "<<" },
	{ O_RDWR,                    1, "<>" }
};
#endif

प्रकारनिर्देश गणना {
	PIPE_SEQ = 1,
	PIPE_AND = 2,
	PIPE_OR  = 3,
	PIPE_BG  = 4,
} pipe_style;

/* might eventually control execution */
प्रकारनिर्देश गणना {
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
	पूर्णाङ्क old_flag;				/* for figuring out valid reserved words */
	संरचना p_context *ढेर;
	पूर्णाङ्क type;			/* define type of parser : ";$" common or special symbol */
	/* How about quoting status? */
};

#ifndef __U_BOOT__
संरचना redir_struct {
	redir_type type;			/* type of redirection */
	पूर्णाङ्क fd;						/* file descriptor being redirected */
	पूर्णाङ्क dup;					/* -1, or file descriptor being duplicated */
	संरचना redir_struct *next;	/* pointer to the next redirect in the list */
	glob_t word;				/* *word.gl_pathv is the filename */
};
#endif

संरचना child_prog {
#ifndef __U_BOOT__
	pid_t pid;					/* 0 if exited */
#endif
	वर्ण **argv;				/* program name and arguments */
	/* was quoted when parsed; copy of struct o_string.nonnull field */
	पूर्णाङ्क *argv_nonnull;
#ifdef __U_BOOT__
	पूर्णाङ्क    argc;                            /* number of program arguments */
#endif
	संरचना pipe *group;			/* if non-NULL, first in group or subshell */
#ifndef __U_BOOT__
	पूर्णाङ्क subshell;				/* flag, non-zero if group must be forked */
	संरचना redir_struct *redirects;	/* I/O redirections */
	glob_t glob_result;			/* result of parameter globbing */
	पूर्णाङ्क is_stopped;				/* is the program currently running? */
	संरचना pipe *family;		/* pointer back to the child's parent pipe */
#endif
	पूर्णाङ्क sp;				/* number of SPECIAL_VAR_SYMBOL */
	पूर्णाङ्क type;
};

संरचना pipe {
#ifndef __U_BOOT__
	पूर्णाङ्क jobid;					/* job number */
#endif
	पूर्णाङ्क अंक_progs;				/* total number of programs in job */
#ifndef __U_BOOT__
	पूर्णाङ्क running_progs;			/* number of programs running */
	वर्ण *text;					/* name of job */
	वर्ण *cmdbuf;				/* buffer various argv's point into */
	pid_t pgrp;					/* process group ID for the job */
#endif
	संरचना child_prog *progs;	/* array of commands in pipe */
	संरचना pipe *next;			/* to track background commands */
#ifndef __U_BOOT__
	पूर्णाङ्क stopped_progs;			/* number of programs alive, but stopped */
	पूर्णाङ्क job_context;			/* bitmask defining current context */
#endif
	pipe_style followup;		/* PIPE_BG, PIPE_SEQ, PIPE_OR, PIPE_AND */
	reserved_style r_mode;		/* supports if, for, while, until */
};

#ifndef __U_BOOT__
संरचना close_me {
	पूर्णाङ्क fd;
	संरचना close_me *next;
};
#endif

संरचना variables {
	वर्ण *name;
	वर्ण *मूल्य;
	पूर्णाङ्क flg_export;
	पूर्णाङ्क flg_read_only;
	संरचना variables *next;
};

/* globals, connect us to the outside world
 * the first three support $?, $#, and $1 */
#ifndef __U_BOOT__
वर्ण **global_argv;
unsigned पूर्णाङ्क global_argc;
#endif
स्थिर unsigned पूर्णाङ्क last_return_code;
#ifndef __U_BOOT__
बाह्य वर्ण **environ; /* This is in <unistd.h>, but protected with __USE_GNU */
#endif

/* "globals" within this file */
स्थिर uchar *ifs;
स्थिर वर्ण map[256];
#ifndef __U_BOOT__
स्थिर पूर्णाङ्क fake_mode;
स्थिर पूर्णाङ्क interactive;
स्थिर संरचना close_me *close_me_head;
स्थिर ध्रुव वर्ण *cwd;
स्थिर संरचना pipe *job_list;
स्थिर unsigned पूर्णाङ्क last_bg_pid;
स्थिर unsigned पूर्णाङ्क last_jobid;
स्थिर unsigned पूर्णाङ्क shell_terminal;
स्थिर वर्ण *PS1;
स्थिर वर्ण *PS2;
संरचना variables shell_ver = { "HUSH_VERSION", "0.01", 1, 1, 0 };
संरचना variables *शीर्ष_vars = &shell_ver;
#अन्यथा
स्थिर पूर्णाङ्क flag_repeat = 0;
स्थिर पूर्णाङ्क do_repeat = 0;
स्थिर संरचना variables *शीर्ष_vars = रिक्त ;
#endif /*__U_BOOT__ */

#define B_CHUNK (100)
#define B_NOSPAC 1

प्रकारनिर्देश संरचना {
	वर्ण *data;
	पूर्णाङ्क length;
	पूर्णाङ्क maxlen;
	पूर्णाङ्क quote;
	पूर्णाङ्क nonnull;
} o_string;
#define NULL_O_STRING {रिक्त,0,0,0,0}
/* used for initialization:
	o_string foo = NULL_O_STRING; */

/* I can almost use ordinary FILE *.  Is open_memstream() universally
 * available?  Where is it documented? */
संरचना in_str {
	ध्रुव unsigned वर्ण *p;
#ifndef __U_BOOT__
	वर्ण peek_buf[2];
#endif
	पूर्णाङ्क __promptme;
	पूर्णाङ्क promptmode;
#ifndef __U_BOOT__
	FILE *file;
#endif
	पूर्णाङ्क (*get) (संरचना in_str *);
	पूर्णाङ्क (*peek) (संरचना in_str *);
};
#define b_getch(निवेश) ((निवेश)->get(निवेश))
#define b_peek(निवेश) ((निवेश)->peek(निवेश))

#ifndef __U_BOOT__
#define JOB_STATUS_FORMAT "[%d] %-22s %.40s\n"

संरचना built_in_command {
	वर्ण *cmd;					/* name */
	वर्ण *descr;				/* description */
	पूर्णाङ्क (*function) (संरचना child_prog *);	/* function ptr */
};
#endif

/* define DEBUG_SHELL for debugging output (obviously ;-)) */
#यदि 0
#define DEBUG_SHELL
#endif

/* This should be in utility.c */
#ifdef DEBUG_SHELL
#ifndef __U_BOOT__
स्थिर शून्य debug_printf(ध्रुव वर्ण *format, ...)
{
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
}
#अन्यथा
#define debug_printf(fmt,args...)	printf (fmt ,##args)
#endif
#अन्यथा
स्थिर अन्तःस्थ शून्य debug_printf(ध्रुव वर्ण *format, ...) { }
#endif
#define final_printf debug_printf

#ifdef __U_BOOT__
स्थिर शून्य syntax_err(शून्य) {
	 printf("syntax error\n");
}
#अन्यथा
स्थिर शून्य __syntax(वर्ण *file, पूर्णाङ्क पंक्ति) {
	error_संदेश("syntax error %s:%d", file, पंक्ति);
}
#define syntax() __syntax(__FILE__, __LINE__)
#endif

#ifdef __U_BOOT__
स्थिर शून्य *xmalloc(size_t size);
स्थिर शून्य *xrealloc(शून्य *ptr, size_t size);
#अन्यथा
/* Index of subroutines: */
/*   function prototypes for builtins */
स्थिर पूर्णाङ्क builtin_cd(संरचना child_prog *child);
स्थिर पूर्णाङ्क builtin_env(संरचना child_prog *child);
स्थिर पूर्णाङ्क builtin_eval(संरचना child_prog *child);
स्थिर पूर्णाङ्क builtin_exec(संरचना child_prog *child);
स्थिर पूर्णाङ्क builtin_exit(संरचना child_prog *child);
स्थिर पूर्णाङ्क builtin_export(संरचना child_prog *child);
स्थिर पूर्णाङ्क builtin_fg_bg(संरचना child_prog *child);
स्थिर पूर्णाङ्क builtin_help(संरचना child_prog *child);
स्थिर पूर्णाङ्क builtin_jobs(संरचना child_prog *child);
स्थिर पूर्णाङ्क builtin_pwd(संरचना child_prog *child);
स्थिर पूर्णाङ्क builtin_read(संरचना child_prog *child);
स्थिर पूर्णाङ्क builtin_set(संरचना child_prog *child);
स्थिर पूर्णाङ्क builtin_shift(संरचना child_prog *child);
स्थिर पूर्णाङ्क builtin_source(संरचना child_prog *child);
स्थिर पूर्णाङ्क builtin_umask(संरचना child_prog *child);
स्थिर पूर्णाङ्क builtin_unset(संरचना child_prog *child);
स्थिर पूर्णाङ्क builtin_not_written(संरचना child_prog *child);
#endif
/*   o_string manipulation: */
स्थिर पूर्णाङ्क b_check_space(o_string *o, पूर्णाङ्क len);
स्थिर पूर्णाङ्क b_addchr(o_string *o, पूर्णाङ्क ch);
स्थिर शून्य b_reset(o_string *o);
स्थिर पूर्णाङ्क b_addqchr(o_string *o, पूर्णाङ्क ch, पूर्णाङ्क quote);
#ifndef __U_BOOT__
स्थिर पूर्णाङ्क b_adduint(o_string *o, unsigned पूर्णाङ्क i);
#endif
/*  in_str manipulations: */
स्थिर पूर्णाङ्क static_get(संरचना in_str *i);
स्थिर पूर्णाङ्क static_peek(संरचना in_str *i);
स्थिर पूर्णाङ्क file_get(संरचना in_str *i);
स्थिर पूर्णाङ्क file_peek(संरचना in_str *i);
#ifndef __U_BOOT__
स्थिर शून्य setup_file_in_str(संरचना in_str *i, FILE *f);
#अन्यथा
स्थिर शून्य setup_file_in_str(संरचना in_str *i);
#endif
स्थिर शून्य setup_string_in_str(संरचना in_str *i, ध्रुव वर्ण *s);
#ifndef __U_BOOT__
/*  close_me manipulations: */
स्थिर शून्य mark_open(पूर्णाङ्क fd);
स्थिर शून्य mark_closed(पूर्णाङ्क fd);
स्थिर शून्य close_all(शून्य);
#endif
/*  "run" the final data structures: */
स्थिर वर्ण *indenter(पूर्णाङ्क i);
स्थिर पूर्णाङ्क free_pipe_list(संरचना pipe *head, पूर्णाङ्क indent);
स्थिर पूर्णाङ्क free_pipe(संरचना pipe *pi, पूर्णाङ्क indent);
/*  really run the final data structures: */
#ifndef __U_BOOT__
स्थिर पूर्णाङ्क setup_redirects(संरचना child_prog *prog, पूर्णाङ्क squirrel[]);
#endif
स्थिर पूर्णाङ्क run_list_real(संरचना pipe *pi);
#ifndef __U_BOOT__
स्थिर शून्य pseudo_exec(संरचना child_prog *child) __attribute__ ((noreturn));
#endif
स्थिर पूर्णाङ्क run_pipe_real(संरचना pipe *pi);
/*   extended glob support: */
#ifndef __U_BOOT__
स्थिर पूर्णाङ्क globhack(ध्रुव वर्ण *src, पूर्णाङ्क flags, glob_t *pglob);
स्थिर पूर्णाङ्क glob_needed(ध्रुव वर्ण *s);
स्थिर पूर्णाङ्क xglob(o_string *dest, पूर्णाङ्क flags, glob_t *pglob);
#endif
/*   variable assignment: */
स्थिर पूर्णाङ्क is_assignment(ध्रुव वर्ण *s);
/*   data structure manipulation: */
#ifndef __U_BOOT__
स्थिर पूर्णाङ्क setup_redirect(संरचना p_context *ctx, पूर्णाङ्क fd, redir_type style, संरचना in_str *निवेश);
#endif
स्थिर शून्य initialize_context(संरचना p_context *ctx);
स्थिर पूर्णाङ्क done_word(o_string *dest, संरचना p_context *ctx);
स्थिर पूर्णाङ्क done_command(संरचना p_context *ctx);
स्थिर पूर्णाङ्क done_pipe(संरचना p_context *ctx, pipe_style type);
/*   primary string parsing: */
#ifndef __U_BOOT__
स्थिर पूर्णाङ्क redirect_dup_अंक(संरचना in_str *निवेश);
स्थिर पूर्णाङ्क redirect_opt_अंक(o_string *o);
स्थिर पूर्णाङ्क process_command_subs(o_string *dest, संरचना p_context *ctx, संरचना in_str *निवेश, पूर्णाङ्क subst_end);
स्थिर पूर्णाङ्क parse_group(o_string *dest, संरचना p_context *ctx, संरचना in_str *निवेश, पूर्णाङ्क ch);
#endif
स्थिर वर्ण *lookup_param(वर्ण *src);
स्थिर वर्ण *make_string(वर्ण **inp, पूर्णाङ्क *nonnull);
स्थिर पूर्णाङ्क handle_dollar(o_string *dest, संरचना p_context *ctx, संरचना in_str *निवेश);
#ifndef __U_BOOT__
स्थिर पूर्णाङ्क parse_string(o_string *dest, संरचना p_context *ctx, ध्रुव वर्ण *src);
#endif
स्थिर पूर्णाङ्क parse_stream(o_string *dest, संरचना p_context *ctx, संरचना in_str *input0, पूर्णाङ्क end_trigger);
/*   setup: */
स्थिर पूर्णाङ्क parse_stream_outer(संरचना in_str *inp, पूर्णाङ्क flag);
#ifndef __U_BOOT__
स्थिर पूर्णाङ्क parse_string_outer(ध्रुव वर्ण *s, पूर्णाङ्क flag);
स्थिर पूर्णाङ्क parse_file_outer(FILE *f);
#endif
#ifndef __U_BOOT__
/*   job management: */
स्थिर पूर्णाङ्क checkjobs(संरचना pipe* fg_pipe);
स्थिर शून्य जोड़ो_bg_job(संरचना pipe *pi);
स्थिर शून्य remove_bg_job(संरचना pipe *pi);
#endif
/*     local variable support */
स्थिर वर्ण **make_list_in(वर्ण **inp, वर्ण *name);
स्थिर वर्ण *जोड़ो_var_मूल्य(वर्ण *inp);
स्थिर वर्ण *जोड़ो_var_मूल्य_sub(वर्ण *inp, पूर्णाङ्क tag_subst);

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

स्थिर ध्रुव वर्ण *set_cwd(शून्य)
{
	यदि(cwd==unknown)
		cwd = रिक्त;     /* xgetcwd(arg) called free(arg) */
	cwd = xgetcwd((वर्ण *)cwd);
	यदि (!cwd)
		cwd = unknown;
	प्रत्यागम cwd;
}

/* built-in 'eval' handler */
स्थिर पूर्णाङ्क builtin_eval(संरचना child_prog *child)
{
	वर्ण *str = रिक्त;
	पूर्णाङ्क rcode = EXIT_SUCCESS;

	यदि (child->argv[1]) {
		str = make_string(child->argv + 1);
		parse_string_outer(str, FLAG_EXIT_FROM_LOOP |
					FLAG_PARSE_SEMICOLON);
		मुक्त(str);
		rcode = last_return_code;
	}
	प्रत्यागम rcode;
}

/* built-in 'cd <path>' handler */
स्थिर पूर्णाङ्क builtin_cd(संरचना child_prog *child)
{
	वर्ण *newdir;
	यदि (child->argv[1] == रिक्त)
		newdir = env_get("HOME");
	अन्यथा
		newdir = child->argv[1];
	यदि (chdir(newdir)) {
		printf("cd: %s: %s\n", newdir, strerror(errno));
		प्रत्यागम EXIT_FAILURE;
	}
	set_cwd();
	प्रत्यागम EXIT_SUCCESS;
}

/* built-in 'env' handler */
स्थिर पूर्णाङ्क builtin_env(संरचना child_prog *dummy)
{
	वर्ण **e = environ;
	यदि (e == रिक्त) प्रत्यागम EXIT_FAILURE;
	प्रति (; *e; e++) {
		puts(*e);
	}
	प्रत्यागम EXIT_SUCCESS;
}

/* built-in 'exec' handler */
स्थिर पूर्णाङ्क builtin_exec(संरचना child_prog *child)
{
	यदि (child->argv[1] == रिक्त)
		प्रत्यागम EXIT_SUCCESS;   /* Really? */
	child->argv++;
	pseudo_exec(child);
	/* never returns */
}

/* built-in 'exit' handler */
स्थिर पूर्णाङ्क builtin_exit(संरचना child_prog *child)
{
	यदि (child->argv[1] == रिक्त)
		exit(last_return_code);
	exit (atoi(child->argv[1]));
}

/* built-in 'export VAR=value' handler */
स्थिर पूर्णाङ्क builtin_export(संरचना child_prog *child)
{
	पूर्णाङ्क res = 0;
	वर्ण *name = child->argv[1];

	यदि (name == रिक्त) {
		प्रत्यागम (builtin_env(child));
	}

	name = strdup(name);

	यदि(name) {
		वर्ण *मूल्य = strchr(name, '=');

		यदि (!मूल्य) {
			वर्ण *tmp;
			/* They are exporting something without an =VALUE */

			मूल्य = get_local_var(name);
			यदि (मूल्य) {
				size_t ln = strlen(name);

				tmp = realloc(name, ln+strlen(मूल्य)+2);
				यदि(tmp==रिक्त)
					res = -1;
				अन्यथा {
					sprintf(tmp+ln, "=%s", मूल्य);
					name = tmp;
				}
			} अन्यथा {
				/* bash does not return an error when trying to export
				 * an undefined variable.  Do likewise. */
				res = 1;
			}
		}
	}
	यदि (res<0)
		perror_संदेश("export");
	अन्यथा यदि(res==0)
		res = set_local_var(name, 1);
	अन्यथा
		res = 0;
	मुक्त(name);
	प्रत्यागम res;
}

/* built-in 'fg' and 'bg' handler */
स्थिर पूर्णाङ्क builtin_fg_bg(संरचना child_prog *child)
{
	पूर्णाङ्क i, jobnum;
	संरचना pipe *pi=रिक्त;

	यदि (!interactive)
		प्रत्यागम EXIT_FAILURE;
	/* If they gave us no args, assume they want the last backgrounded task */
	यदि (!child->argv[1]) {
		प्रति (pi = job_list; pi; pi = pi->next) {
			यदि (pi->jobid == last_jobid) {
				भङ्ग;
			}
		}
		यदि (!pi) {
			error_संदेश("%s: no current job", child->argv[0]);
			प्रत्यागम EXIT_FAILURE;
		}
	} अन्यथा {
		यदि (sscanf(child->argv[1], "%%%d", &jobnum) != 1) {
			error_संदेश("%s: bad argument '%s'", child->argv[0], child->argv[1]);
			प्रत्यागम EXIT_FAILURE;
		}
		प्रति (pi = job_list; pi; pi = pi->next) {
			यदि (pi->jobid == jobnum) {
				भङ्ग;
			}
		}
		यदि (!pi) {
			error_संदेश("%s: %d: no such job", child->argv[0], jobnum);
			प्रत्यागम EXIT_FAILURE;
		}
	}

	यदि (*child->argv[0] == 'f') {
		/* Put the job into the foreground.  */
		tcsetpgrp(shell_terminal, pi->pgrp);
	}

	/* Restart the processes in the job */
	प्रति (i = 0; i < pi->अंक_progs; i++)
		pi->progs[i].is_stopped = 0;

	यदि ( (i=kill(- pi->pgrp, SIGCONT)) < 0) {
		यदि (i == ESRCH) {
			remove_bg_job(pi);
		} अन्यथा {
			perror_संदेश("kill (SIGCONT)");
		}
	}

	pi->stopped_progs = 0;
	प्रत्यागम EXIT_SUCCESS;
}

/* built-in 'help' handler */
स्थिर पूर्णाङ्क builtin_help(संरचना child_prog *dummy)
{
	संरचना built_in_command *x;

	printf("\nBuilt-in commands:\n");
	printf("-------------------\n");
	प्रति (x = bltins; x->cmd; x++) {
		यदि (x->descr==रिक्त)
			अनुवर्त;
		printf("%s\t%s\n", x->cmd, x->descr);
	}
	printf("\n\n");
	प्रत्यागम EXIT_SUCCESS;
}

/* built-in 'jobs' handler */
स्थिर पूर्णाङ्क builtin_jobs(संरचना child_prog *child)
{
	संरचना pipe *job;
	वर्ण *status_string;

	प्रति (job = job_list; job; job = job->next) {
		यदि (job->running_progs == job->stopped_progs)
			status_string = "Stopped";
		अन्यथा
			status_string = "Running";

		printf(JOB_STATUS_FORMAT, job->jobid, status_string, job->text);
	}
	प्रत्यागम EXIT_SUCCESS;
}

/* built-in 'pwd' handler */
स्थिर पूर्णाङ्क builtin_pwd(संरचना child_prog *dummy)
{
	puts(set_cwd());
	प्रत्यागम EXIT_SUCCESS;
}

/* built-in 'read VAR' handler */
स्थिर पूर्णाङ्क builtin_read(संरचना child_prog *child)
{
	पूर्णाङ्क res;

	यदि (child->argv[1]) {
		वर्ण string[BUFSIZ];
		वर्ण *var = 0;

		string[0] = 0;  /* In case stdin has only EOF */
		/* read string */
		fgets(string, परिमाण(string), stdin);
		chomp(string);
		var = malloc(strlen(child->argv[1])+strlen(string)+2);
		यदि(var) {
			sprintf(var, "%s=%s", child->argv[1], string);
			res = set_local_var(var, 0);
		} अन्यथा
			res = -1;
		यदि (res)
			fprintf(stderr, "read: %m\n");
		मुक्त(var);      /* So not move up to avoid breaking errno */
		प्रत्यागम res;
	} अन्यथा {
		कुरु res=getchar(); यावत्(res!='\n' && res!=EOF);
		प्रत्यागम 0;
	}
}

/* built-in 'set VAR=value' handler */
स्थिर पूर्णाङ्क builtin_set(संरचना child_prog *child)
{
	वर्ण *temp = child->argv[1];
	संरचना variables *e;

	यदि (temp == रिक्त)
		प्रति(e = शीर्ष_vars; e; e=e->next)
			printf("%s=%s\n", e->name, e->मूल्य);
	अन्यथा
		set_local_var(temp, 0);

		प्रत्यागम EXIT_SUCCESS;
}

/* Built-in 'shift' handler */
स्थिर पूर्णाङ्क builtin_shift(संरचना child_prog *child)
{
	पूर्णाङ्क n=1;
	यदि (child->argv[1]) {
		n=atoi(child->argv[1]);
	}
	यदि (n>=0 && n<global_argc) {
		/* XXX This probably breaks $0 */
		global_argc -= n;
		global_argv += n;
		प्रत्यागम EXIT_SUCCESS;
	} अन्यथा {
		प्रत्यागम EXIT_FAILURE;
	}
}

/* Built-in '.' handler (read-in and execute commands from file) */
स्थिर पूर्णाङ्क builtin_source(संरचना child_prog *child)
{
	FILE *निवेश;
	पूर्णाङ्क status;

	यदि (child->argv[1] == रिक्त)
		प्रत्यागम EXIT_FAILURE;

	/* XXX search through $PATH is missing */
	निवेश = fopen(child->argv[1], "r");
	यदि (!निवेश) {
		error_संदेश("Couldn't open file '%s'", child->argv[1]);
		प्रत्यागम EXIT_FAILURE;
	}

	/* Now run the file */
	/* XXX argv and argc are broken; need to save old global_argv
	 * (pointer only is OK!) on this stack frame,
	 * set global_argv=child->argv+1, recurse, and restore. */
	mark_open(fileno(निवेश));
	status = parse_file_outer(निवेश);
	mark_closed(fileno(निवेश));
	fclose(निवेश);
	प्रत्यागम (status);
}

स्थिर पूर्णाङ्क builtin_umask(संरचना child_prog *child)
{
	mode_t new_umask;
	ध्रुव वर्ण *arg = child->argv[1];
	वर्ण *end;
	यदि (arg) {
		new_umask=strtoul(arg, &end, 8);
		यदि (*end!='\0' || end == arg) {
			प्रत्यागम EXIT_FAILURE;
		}
	} अन्यथा {
		printf("%.3o\n", (unsigned पूर्णाङ्क) (new_umask=umask(0)));
	}
	umask(new_umask);
	प्रत्यागम EXIT_SUCCESS;
}

/* built-in 'unset VAR' handler */
स्थिर पूर्णाङ्क builtin_unset(संरचना child_prog *child)
{
	/* bash returned already true */
	unset_local_var(child->argv[1]);
	प्रत्यागम EXIT_SUCCESS;
}

स्थिर पूर्णाङ्क builtin_not_written(संरचना child_prog *child)
{
	printf("builtin_%s not written\n",child->argv[0]);
	प्रत्यागम EXIT_FAILURE;
}
#endif

स्थिर पूर्णाङ्क b_check_space(o_string *o, पूर्णाङ्क len)
{
	/* It would be easy to drop a more restrictive policy
	 * in here, such as setting a maximum string length */
	यदि (o->length + len > o->maxlen) {
		वर्ण *old_data = o->data;
		/* assert (data == NULL || o->maxlen != 0); */
		o->maxlen += max(2*len, B_CHUNK);
		o->data = realloc(o->data, 1 + o->maxlen);
		यदि (o->data == रिक्त) {
			मुक्त(old_data);
		}
	}
	प्रत्यागम o->data == रिक्त;
}

स्थिर पूर्णाङ्क b_addchr(o_string *o, पूर्णाङ्क ch)
{
	debug_printf("b_addchr: %c %d %p\n", ch, o->length, o);
	यदि (b_check_space(o, 1)) प्रत्यागम B_NOSPAC;
	o->data[o->length] = ch;
	o->length++;
	o->data[o->length] = '\0';
	प्रत्यागम 0;
}

स्थिर शून्य b_reset(o_string *o)
{
	o->length = 0;
	o->nonnull = 0;
	यदि (o->data != रिक्त) *o->data = '\0';
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
स्थिर पूर्णाङ्क b_addqchr(o_string *o, पूर्णाङ्क ch, पूर्णाङ्क quote)
{
	यदि (quote && strchr("*?[\\",ch)) {
		पूर्णाङ्क rc;
		rc = b_addchr(o, '\\');
		यदि (rc) प्रत्यागम rc;
	}
	प्रत्यागम b_addchr(o, ch);
}

#ifndef __U_BOOT__
स्थिर पूर्णाङ्क b_adduint(o_string *o, unsigned पूर्णाङ्क i)
{
	पूर्णाङ्क r;
	वर्ण *p = simple_itoa(i);
	/* no escape checking necessary */
	कुरु r=b_addchr(o, *p++); यावत् (r==0 && *p);
	प्रत्यागम r;
}
#endif

स्थिर पूर्णाङ्क static_get(संरचना in_str *i)
{
	पूर्णाङ्क ch = *i->p++;
	यदि (ch=='\0') प्रत्यागम EOF;
	प्रत्यागम ch;
}

स्थिर पूर्णाङ्क static_peek(संरचना in_str *i)
{
	प्रत्यागम *i->p;
}

#ifndef __U_BOOT__
स्थिर अन्तःस्थ शून्य cmdedit_set_initial_prompt(शून्य)
{
#ifndef CONFIG_FEATURE_SH_FANCY_PROMPT
	PS1 = रिक्त;
#अन्यथा
	PS1 = env_get("PS1");
	यदि(PS1==0)
		PS1 = "\\w \\$ ";
#endif
}

स्थिर अन्तःस्थ शून्य setup_prompt_string(पूर्णाङ्क promptmode, वर्ण **prompt_str)
{
	debug_printf("setup_prompt_string %d ",promptmode);
#ifndef CONFIG_FEATURE_SH_FANCY_PROMPT
	/* Set up the prompt */
	यदि (promptmode == 1) {
		मुक्त(PS1);
		PS1=xmalloc(strlen(cwd)+4);
		sprintf(PS1, "%s %s", cwd, ( geteuid() != 0 ) ?  "$ ":"# ");
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
स्थिर पूर्णाङ्क uboot_cli_readline(संरचना in_str *i)
{
	वर्ण *prompt;
	वर्ण __maybe_unused *ps_prompt = रिक्त;

	यदि (i->promptmode == 1)
		prompt = CONFIG_SYS_PROMPT;
	अन्यथा
		prompt = CONFIG_SYS_PROMPT_HUSH_PS2;

#ifdef CONFIG_CMDLINE_PS_SUPPORT
	यदि (i->promptmode == 1)
		ps_prompt = env_get("PS1");
	अन्यथा
		ps_prompt = env_get("PS2");
	यदि (ps_prompt)
		prompt = ps_prompt;
#endif

	प्रत्यागम cli_readline(prompt);
}
#endif

स्थिर शून्य get_user_निवेश(संरचना in_str *i)
{
#ifndef __U_BOOT__
	वर्ण *prompt_str;
	स्थिर वर्ण the_command[BUFSIZ];

	setup_prompt_string(i->promptmode, &prompt_str);
#ifdef CONFIG_FEATURE_COMMAND_EDITING
	/*
	 ** enable command line editing only while a command line
	 ** is actually being read; otherwise, we'll end up bequeathing
	 ** atexit() handlers and other unwanted stuff to our
	 ** child processes (rob@sysgo.de)
	 */
	cmdedit_read_निवेश(prompt_str, the_command);
#अन्यथा
	fputs(prompt_str, stdout);
	fflush(stdout);
	the_command[0]=fgetc(i->file);
	the_command[1]='\0';
#endif
	fflush(stdout);
	i->p = the_command;
#अन्यथा
	पूर्णाङ्क n;
	स्थिर वर्ण the_command[CONFIG_SYS_CBSIZE + 1];

	bootretry_reset_cmd_timeout();
	i->__promptme = 1;
	n = uboot_cli_readline(i);

#ifdef CONFIG_BOOT_RETRY_TIME
	यदि (n == -2) {
	  puts("\nTimeout waiting for command\n");
#  ifdef CONFIG_RESET_TO_RETRY
	  do_reset(रिक्त, 0, 0, रिक्त);
#  elif IS_ENABLED(CONFIG_RETRY_BOOTCMD)
	strcpy(console_buffer, "run bootcmd\n");
# अन्यथा
#	error "This only works with CONFIG_RESET_TO_RETRY or CONFIG_BOOT_RETRY_COMMAND enabled"
#  endif
	}
#endif
	यदि (n == -1 ) {
		flag_repeat = 0;
		i->__promptme = 0;
	}
	n = strlen(console_buffer);
	console_buffer[n] = '\n';
	console_buffer[n+1]= '\0';
	यदि (had_ctrlc()) flag_repeat = 0;
	clear_ctrlc();
	do_repeat = 0;
	यदि (i->promptmode == 1) {
		यदि (console_buffer[0] == '\n'&& flag_repeat == 0) {
			strcpy(the_command,console_buffer);
		}
		अन्यथा {
			यदि (console_buffer[0] != '\n') {
				strcpy(the_command,console_buffer);
				flag_repeat = 1;
			}
			अन्यथा {
				do_repeat = 1;
			}
		}
		i->p = the_command;
	}
	अन्यथा {
		यदि (console_buffer[0] != '\n') {
			यदि (strlen(the_command) + strlen(console_buffer)
			    < CONFIG_SYS_CBSIZE) {
				n = strlen(the_command);
				the_command[n-1] = ' ';
				strcpy(&the_command[n],console_buffer);
			}
			अन्यथा {
				the_command[0] = '\n';
				the_command[1] = '\0';
				flag_repeat = 0;
			}
		}
		यदि (i->__promptme == 0) {
			the_command[0] = '\n';
			the_command[1] = '\0';
		}
		i->p = console_buffer;
	}
#endif
}

/* This is the magic location that prints prompts
 * and gets data back from the user */
स्थिर पूर्णाङ्क file_get(संरचना in_str *i)
{
	पूर्णाङ्क ch;

	ch = 0;
	/* If there is data waiting, eat it up */
	यदि (i->p && *i->p) {
		ch = *i->p++;
	} अन्यथा {
		/* need to double check i->file because we might be doing something
		 * more complicated by now, like sourcing or substituting. */
#ifndef __U_BOOT__
		यदि (i->__promptme && interactive && i->file == stdin) {
			यावत्(! i->p || (interactive && strlen(i->p)==0) ) {
#अन्यथा
			यावत्(! i->p  || strlen(i->p)==0 ) {
#endif
				get_user_निवेश(i);
			}
			i->promptmode=2;
#ifndef __U_BOOT__
			i->__promptme = 0;
#endif
			यदि (i->p && *i->p) {
				ch = *i->p++;
			}
#ifndef __U_BOOT__
		} अन्यथा {
			ch = fgetc(i->file);
		}

#endif
		debug_printf("b_getch: got a %d\n", ch);
	}
#ifndef __U_BOOT__
	यदि (ch == '\n') i->__promptme=1;
#endif
	प्रत्यागम ch;
}

/* All the callers guarantee this routine will never be
 * used right after a newline, so prompting is not needed.
 */
स्थिर पूर्णाङ्क file_peek(संरचना in_str *i)
{
#ifndef __U_BOOT__
	यदि (i->p && *i->p) {
#endif
		प्रत्यागम *i->p;
#ifndef __U_BOOT__
	} अन्यथा {
		i->peek_buf[0] = fgetc(i->file);
		i->peek_buf[1] = '\0';
		i->p = i->peek_buf;
		debug_printf("b_peek: got a %d\n", *i->p);
		प्रत्यागम *i->p;
	}
#endif
}

#ifndef __U_BOOT__
स्थिर शून्य setup_file_in_str(संरचना in_str *i, FILE *f)
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

स्थिर शून्य setup_string_in_str(संरचना in_str *i, ध्रुव वर्ण *s)
{
	i->peek = static_peek;
	i->get = static_get;
	i->__promptme=1;
	i->promptmode=1;
	i->p = s;
}

#ifndef __U_BOOT__
स्थिर शून्य mark_open(पूर्णाङ्क fd)
{
	संरचना close_me *new = xmalloc(परिमाण(संरचना close_me));
	new->fd = fd;
	new->next = close_me_head;
	close_me_head = new;
}

स्थिर शून्य mark_closed(पूर्णाङ्क fd)
{
	संरचना close_me *tmp;
	यदि (close_me_head == रिक्त || close_me_head->fd != fd)
		error_संदेश_and_die("corrupt close_me");
	tmp = close_me_head;
	close_me_head = close_me_head->next;
	मुक्त(tmp);
}

स्थिर शून्य close_all(शून्य)
{
	संरचना close_me *c;
	प्रति (c=close_me_head; c; c=c->next) {
		close(c->fd);
	}
	close_me_head = रिक्त;
}

/* squirrel != NULL means we squirrel away copies of stdin, stdout,
 * and stderr if they are redirected. */
स्थिर पूर्णाङ्क setup_redirects(संरचना child_prog *prog, पूर्णाङ्क squirrel[])
{
	पूर्णाङ्क openfd, mode;
	संरचना redir_struct *redir;

	प्रति (redir=prog->redirects; redir; redir=redir->next) {
		यदि (redir->dup == -1 && redir->word.gl_pathv == रिक्त) {
			/* something went wrong in the parse.  Pretend it didn't happen */
			अनुवर्त;
		}
		यदि (redir->dup == -1) {
			mode=redir_table[redir->type].mode;
			openfd = open(redir->word.gl_pathv[0], mode, 0666);
			यदि (openfd < 0) {
			/* this could get lost if stderr has been redirected, but
			   bash and ash both lose it as well (though zsh doesn't!) */
				perror_संदेश("error opening %s", redir->word.gl_pathv[0]);
				प्रत्यागम 1;
			}
		} अन्यथा {
			openfd = redir->dup;
		}

		यदि (openfd != redir->fd) {
			यदि (squirrel && redir->fd < 3) {
				squirrel[redir->fd] = dup(redir->fd);
			}
			यदि (openfd == -3) {
				close(openfd);
			} अन्यथा {
				dup2(openfd, redir->fd);
				यदि (redir->dup == -1)
					close (openfd);
			}
		}
	}
	प्रत्यागम 0;
}

स्थिर शून्य restore_redirects(पूर्णाङ्क squirrel[])
{
	पूर्णाङ्क i, fd;
	प्रति (i=0; i<3; i++) {
		fd = squirrel[i];
		यदि (fd != -1) {
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
स्थिर शून्य pseudo_exec(संरचना child_prog *child)
{
	पूर्णाङ्क i, rcode;
	वर्ण *p;
	संरचना built_in_command *x;
	यदि (child->argv) {
		प्रति (i=0; is_assignment(child->argv[i]); i++) {
			debug_printf("pid %d environment modification: %s\n",getpid(),child->argv[i]);
			p = जोड़ो_var_मूल्य(child->argv[i]);
			putenv(strdup(p));
			यदि (p != child->argv[i]) मुक्त(p);
		}
		child->argv+=i;  /* XXX this hack isn't so horrible, since we are about
					to exit, and therefore don't need to keep data
					structures consistent for free() use. */
		/* If a variable is assigned in a forest, and nobody listens,
		 * was it ever really set?
		 */
		यदि (child->argv[0] == रिक्त) {
			_exit(EXIT_SUCCESS);
		}

		/*
		 * Check if the command matches any of the builtins.
		 * Depending on context, this might be redundant.  But it's
		 * easier to waste a few CPU cycles than it is to figure out
		 * if this is one of those cases.
		 */
		प्रति (x = bltins; x->cmd; x++) {
			यदि (strcmp(child->argv[0], x->cmd) == 0 ) {
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
			पूर्णाङ्क argc_l;
			वर्ण** argv_l=child->argv;
			वर्ण *name = child->argv[0];

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
			प्रति(argc_l=0;*argv_l!=रिक्त; argv_l++, argc_l++);
			optind = 1;
			debug_printf("running applet %s\n", name);
			run_applet_by_name(name, argc_l, child->argv);
		}
#endif
		debug_printf("exec of %s\n",child->argv[0]);
		execvp(child->argv[0],child->argv);
		perror_संदेश("couldn't exec: %s",child->argv[0]);
		_exit(1);
	} अन्यथा यदि (child->group) {
		debug_printf("runtime nesting to group\n");
		interactive=0;    /* crucial!!!! */
		rcode = run_list_real(child->group);
		/* OK to leak memory by not calling free_pipe_list,
		 * since this process is about to exit */
		_exit(rcode);
	} अन्यथा {
		/* Can happen.  See what bash does with ">foo" by itself. */
		debug_printf("trying to pseudo_exec null command\n");
		_exit(EXIT_SUCCESS);
	}
}

स्थिर शून्य जोड़ो_bg_job(संरचना pipe *pi)
{
	संरचना pipe *thejob;

	/* Linear search for the ID of the job to use */
	pi->jobid = 1;
	प्रति (thejob = job_list; thejob; thejob = thejob->next)
		यदि (thejob->jobid >= pi->jobid)
			pi->jobid = thejob->jobid + 1;

	/* add thejob to the list of running jobs */
	यदि (!job_list) {
		thejob = job_list = xmalloc(परिमाण(*thejob));
	} अन्यथा {
		प्रति (thejob = job_list; thejob->next; thejob = thejob->next) /* nothing */;
		thejob->next = xmalloc(परिमाण(*thejob));
		thejob = thejob->next;
	}

	/* physically copy the struct job */
	memcpy(thejob, pi, परिमाण(संरचना pipe));
	thejob->next = रिक्त;
	thejob->running_progs = thejob->अंक_progs;
	thejob->stopped_progs = 0;
	thejob->text = xmalloc(BUFSIZ); /* cmdedit buffer size */

	/*if (pi->progs[0] && pi->progs[0].argv && pi->progs[0].argv[0]) */
	{
		वर्ण *bar=thejob->text;
		वर्ण **foo=pi->progs[0].argv;
		यावत्(foo && *foo) {
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
स्थिर शून्य remove_bg_job(संरचना pipe *pi)
{
	संरचना pipe *prev_pipe;

	यदि (pi == job_list) {
		job_list = pi->next;
	} अन्यथा {
		prev_pipe = job_list;
		यावत् (prev_pipe->next != pi)
			prev_pipe = prev_pipe->next;
		prev_pipe->next = pi->next;
	}
	यदि (job_list)
		last_jobid = job_list->jobid;
	अन्यथा
		last_jobid = 0;

	pi->stopped_progs = 0;
	free_pipe(pi, 0);
	मुक्त(pi);
}

/* Checks to see if any processes have exited -- if they
   have, figure out why and see if a job has completed */
स्थिर पूर्णाङ्क checkjobs(संरचना pipe* fg_pipe)
{
	पूर्णाङ्क attributes;
	पूर्णाङ्क status;
	पूर्णाङ्क prognum = 0;
	संरचना pipe *pi;
	pid_t childpid;

	attributes = WUNTRACED;
	यदि (fg_pipe==रिक्त) {
		attributes |= WNOHANG;
	}

	यावत् ((childpid = waitpid(-1, &status, attributes)) > 0) {
		यदि (fg_pipe) {
			पूर्णाङ्क i, rcode = 0;
			प्रति (i=0; i < fg_pipe->अंक_progs; i++) {
				यदि (fg_pipe->progs[i].pid == childpid) {
					यदि (i==fg_pipe->अंक_progs-1)
						rcode=WEXITSTATUS(status);
					(fg_pipe->अंक_progs)--;
					प्रत्यागम(rcode);
				}
			}
		}

		प्रति (pi = job_list; pi; pi = pi->next) {
			prognum = 0;
			यावत् (prognum < pi->अंक_progs && pi->progs[prognum].pid != childpid) {
				prognum++;
			}
			यदि (prognum < pi->अंक_progs)
				भङ्ग;
		}

		यदि(pi==रिक्त) {
			debug_printf("checkjobs: pid %d was not in our list!\n", childpid);
			अनुवर्त;
		}

		यदि (WIFEXITED(status) || WIFSIGNALED(status)) {
			/* child exited */
			pi->running_progs--;
			pi->progs[prognum].pid = 0;

			यदि (!pi->running_progs) {
				printf(JOB_STATUS_FORMAT, pi->jobid, "Done", pi->text);
				remove_bg_job(pi);
			}
		} अन्यथा {
			/* child stopped */
			pi->stopped_progs++;
			pi->progs[prognum].is_stopped = 1;

#यदि 0
			/* Printing this stuff is a pain, since it tends to
			 * overwrite the prompt an inconveinient moments.  So
			 * don't do that.  */
			यदि (pi->stopped_progs == pi->अंक_progs) {
				printf("\n"JOB_STATUS_FORMAT, pi->jobid, "Stopped", pi->text);
			}
#endif
		}
	}

	यदि (childpid == -1 && errno != ECHILD)
		perror_संदेश("waitpid");

	/* move the shell to the foreground */
	/*if (interactive && tcsetpgrp(shell_terminal, getpgid(0))) */
	/*	perror_msg("tcsetpgrp-2"); */
	प्रत्यागम -1;
}

/* Figure out our controlling tty, checking in order stderr,
 * stdin, and stdout.  If check_pgrp is set, also check that
 * we belong to the foreground process group associated with
 * that tty.  The value of shell_terminal is needed in order to call
 * tcsetpgrp(shell_terminal, ...); */
शून्य controlling_tty(पूर्णाङ्क check_pgrp)
{
	pid_t curpgrp;

	यदि ((curpgrp = tcgetpgrp(shell_terminal = 2)) < 0
			&& (curpgrp = tcgetpgrp(shell_terminal = 0)) < 0
			&& (curpgrp = tcgetpgrp(shell_terminal = 1)) < 0)
		गच्छ shell_terminal_error;

	यदि (check_pgrp && curpgrp != getpgid(0))
		गच्छ shell_terminal_error;

	प्रत्यागम;

shell_terminal_error:
		shell_terminal = -1;
		प्रत्यागम;
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
स्थिर पूर्णाङ्क run_pipe_real(संरचना pipe *pi)
{
	पूर्णाङ्क i;
#ifndef __U_BOOT__
	पूर्णाङ्क nextin, nextout;
	पूर्णाङ्क pipefds[2];				/* pipefds[0] is for reading */
	संरचना child_prog *child;
	संरचना built_in_command *x;
	वर्ण *p;
# यदि __GNUC__
	/* Avoid longjmp clobbering */
	(शून्य) &i;
	(शून्य) &nextin;
	(शून्य) &nextout;
	(शून्य) &child;
# endif
#अन्यथा
	पूर्णाङ्क nextin;
	पूर्णाङ्क flag = do_repeat ? CMD_FLAG_REPEAT : 0;
	संरचना child_prog *child;
	वर्ण *p;
# यदि __GNUC__
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
	यदि (pi->अंक_progs == 1) child = & (pi->progs[0]);
#ifndef __U_BOOT__
	यदि (pi->अंक_progs == 1 && child->group && child->subshell == 0) {
		पूर्णाङ्क squirrel[] = {-1, -1, -1};
		पूर्णाङ्क rcode;
		debug_printf("non-subshell grouping\n");
		setup_redirects(child, squirrel);
		/* XXX could we merge code with following builtin case,
		 * by creating a pseudo builtin that calls run_list_real? */
		rcode = run_list_real(child->group);
		restore_redirects(squirrel);
#अन्यथा
		यदि (pi->अंक_progs == 1 && child->group) {
		पूर्णाङ्क rcode;
		debug_printf("non-subshell grouping\n");
		rcode = run_list_real(child->group);
#endif
		प्रत्यागम rcode;
	} अन्यथा यदि (pi->अंक_progs == 1 && pi->progs[0].argv != रिक्त) {
		प्रति (i=0; is_assignment(child->argv[i]); i++) { /* nothing */ }
		यदि (i!=0 && child->argv[i]==रिक्त) {
			/* assignments, but no command: set the local environment */
			प्रति (i=0; child->argv[i]!=रिक्त; i++) {

				/* Ok, this case is tricky.  We have to decide if this is a
				 * local variable, or an already exported variable.  If it is
				 * already exported, we have to export the new value.  If it is
				 * not exported, we need only set this as a local variable.
				 * This junk is all to decide whether or not to export this
				 * variable. */
				पूर्णाङ्क export_me=0;
				वर्ण *name, *मूल्य;
				name = xstrdup(child->argv[i]);
				debug_printf("Local environment set: %s\n", name);
				मूल्य = strchr(name, '=');
				यदि (मूल्य)
					*मूल्य=0;
#ifndef __U_BOOT__
				यदि ( get_local_var(name)) {
					export_me=1;
				}
#endif
				मुक्त(name);
				p = जोड़ो_var_मूल्य(child->argv[i]);
				set_local_var(p, export_me);
				यदि (p != child->argv[i]) मुक्त(p);
			}
			प्रत्यागम EXIT_SUCCESS;   /* don't worry about errors in set_local_var() yet */
		}
		प्रति (i = 0; is_assignment(child->argv[i]); i++) {
			p = जोड़ो_var_मूल्य(child->argv[i]);
#ifndef __U_BOOT__
			putenv(strdup(p));
#अन्यथा
			set_local_var(p, 0);
#endif
			यदि (p != child->argv[i]) {
				child->sp--;
				मुक्त(p);
			}
		}
		यदि (child->sp) {
			वर्ण * str = रिक्त;

			str = make_string(child->argv + i,
					  child->argv_nonnull + i);
			parse_string_outer(str, FLAG_EXIT_FROM_LOOP | FLAG_REPARSING);
			मुक्त(str);
			प्रत्यागम last_return_code;
		}
#ifndef __U_BOOT__
		प्रति (x = bltins; x->cmd; x++) {
			यदि (strcmp(child->argv[i], x->cmd) == 0 ) {
				पूर्णाङ्क squirrel[] = {-1, -1, -1};
				पूर्णाङ्क rcode;
				यदि (x->function == builtin_exec && child->argv[i+1]==रिक्त) {
					debug_printf("magic exec\n");
					setup_redirects(child,रिक्त);
					प्रत्यागम EXIT_SUCCESS;
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
			प्रत्यागम rcode;
		}
#अन्यथा
		/* check ";", because ,example , argv consist from
		 * "help;flinfo" must not execute
		 */
		यदि (strchr(child->argv[i], ';')) {
			printf("Unknown command '%s' - try 'help' or use "
					"'run' command\n", child->argv[i]);
			प्रत्यागम -1;
		}
		/* Process the command */
		प्रत्यागम cmd_process(flag, child->argc - i, child->argv + i,
				   &flag_repeat, रिक्त);
#endif
	}
#ifndef __U_BOOT__

	प्रति (i = 0; i < pi->अंक_progs; i++) {
		child = & (pi->progs[i]);

		/* pipes are inserted between pairs of commands */
		यदि ((i + 1) < pi->अंक_progs) {
			यदि (pipe(pipefds)<0) perror_संदेश_and_die("pipe");
			nextout = pipefds[1];
		} अन्यथा {
			nextout=1;
			pipefds[0] = -1;
		}

		/* XXX test for failed fork()? */
		यदि (!(child->pid = fork())) {
			/* Set the handling for job control signals back to the default.  */
			सङ्केत(SIGINT, SIG_DFL);
			सङ्केत(SIGQUIT, SIG_DFL);
			सङ्केत(SIGTERM, SIG_DFL);
			सङ्केत(SIGTSTP, SIG_DFL);
			सङ्केत(SIGTTIN, SIG_DFL);
			सङ्केत(SIGTTOU, SIG_DFL);
			सङ्केत(SIGCHLD, SIG_DFL);

			close_all();

			यदि (nextin != 0) {
				dup2(nextin, 0);
				close(nextin);
			}
			यदि (nextout != 1) {
				dup2(nextout, 1);
				close(nextout);
			}
			यदि (pipefds[0]!=-1) {
				close(pipefds[0]);  /* opposite end of our output pipe */
			}

			/* Like bash, explicit redirects override pipes,
			 * and the pipe fd is available for dup'ing. */
			setup_redirects(child,रिक्त);

			यदि (interactive && pi->followup!=PIPE_BG) {
				/* If we (the child) win the race, put ourselves in the process
				 * group whose leader is the first process in this pipe. */
				यदि (pi->pgrp < 0) {
					pi->pgrp = getpid();
				}
				यदि (setpgid(0, pi->pgrp) == 0) {
					tcsetpgrp(2, pi->pgrp);
				}
			}

			pseudo_exec(child);
		}

		/* put our child in the process group whose leader is the
		   first process in this pipe */
		यदि (pi->pgrp < 0) {
			pi->pgrp = child->pid;
		}
		/* Don't check for errors.  The child may be dead already,
		 * in which case setpgid returns error code EACCES. */
		setpgid(child->pid, pi->pgrp);

		यदि (nextin != 0)
			close(nextin);
		यदि (nextout != 1)
			close(nextout);

		/* If there isn't another process, nextin is garbage
		   but it doesn't matter */
		nextin = pipefds[0];
	}
#endif
	प्रत्यागम -1;
}

स्थिर पूर्णाङ्क run_list_real(संरचना pipe *pi)
{
	वर्ण *save_name = रिक्त;
	वर्ण **list = रिक्त;
	वर्ण **save_list = रिक्त;
	संरचना pipe *rpipe;
	पूर्णाङ्क flag_rep = 0;
#ifndef __U_BOOT__
	पूर्णाङ्क save_अंक_progs;
#endif
	पूर्णाङ्क rcode=0, flag_skip=1;
	पूर्णाङ्क flag_restore = 0;
	पूर्णाङ्क if_code=0, next_if_code=0;  /* need double-buffer to handle elif */
	reserved_style rmode, skip_more_in_this_rmode=RES_XXXX;
	/* check syntax for "for" */
	प्रति (rpipe = pi; rpipe; rpipe = rpipe->next) {
		यदि ((rpipe->r_mode == RES_IN ||
		    rpipe->r_mode == RES_FOR) &&
		    (rpipe->next == रिक्त)) {
				syntax();
#ifdef __U_BOOT__
				flag_repeat = 0;
#endif
				प्रत्यागम 1;
		}
		यदि ((rpipe->r_mode == RES_IN &&
			(rpipe->next->r_mode == RES_IN &&
			rpipe->next->progs->argv != रिक्त))||
			(rpipe->r_mode == RES_FOR &&
			rpipe->next->r_mode != RES_IN)) {
				syntax();
#ifdef __U_BOOT__
				flag_repeat = 0;
#endif
				प्रत्यागम 1;
		}
	}
	प्रति (; pi; pi = (flag_restore != 0) ? rpipe : pi->next) {
		यदि (pi->r_mode == RES_WHILE || pi->r_mode == RES_UNTIL ||
			pi->r_mode == RES_FOR) {
#ifdef __U_BOOT__
				/* check Ctrl-C */
				ctrlc();
				यदि ((had_ctrlc())) {
					प्रत्यागम 1;
				}
#endif
				flag_restore = 0;
				यदि (!rpipe) {
					flag_rep = 0;
					rpipe = pi;
				}
		}
		rmode = pi->r_mode;
		debug_printf("rmode=%d  if_code=%d  next_if_code=%d skip_more=%d\n", rmode, if_code, next_if_code, skip_more_in_this_rmode);
		यदि (rmode == skip_more_in_this_rmode && flag_skip) {
			यदि (pi->followup == PIPE_SEQ) flag_skip=0;
			अनुवर्त;
		}
		flag_skip = 1;
		skip_more_in_this_rmode = RES_XXXX;
		यदि (rmode == RES_THEN || rmode == RES_ELSE) if_code = next_if_code;
		यदि (rmode == RES_THEN &&  if_code) अनुवर्त;
		यदि (rmode == RES_ELSE && !if_code) अनुवर्त;
		यदि (rmode == RES_ELIF && !if_code) भङ्ग;
		यदि (rmode == RES_FOR && pi->अंक_progs) {
			यदि (!list) {
				/* if no variable values after "in" we skip "for" */
				यदि (!pi->next->progs->argv) अनुवर्त;
				/* create list of variable values */
				list = make_list_in(pi->next->progs->argv,
					pi->progs->argv[0]);
				save_list = list;
				save_name = pi->progs->argv[0];
				pi->progs->argv[0] = रिक्त;
				flag_rep = 1;
			}
			यदि (!(*list)) {
				मुक्त(pi->progs->argv[0]);
				मुक्त(save_list);
				list = रिक्त;
				flag_rep = 0;
				pi->progs->argv[0] = save_name;
#ifndef __U_BOOT__
				pi->progs->glob_result.gl_pathv[0] =
					pi->progs->argv[0];
#endif
				अनुवर्त;
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
		यदि (rmode == RES_IN) अनुवर्त;
		यदि (rmode == RES_DO) {
			यदि (!flag_rep) अनुवर्त;
		}
		यदि (rmode == RES_DONE) {
			यदि (flag_rep) {
				flag_restore = 1;
			} अन्यथा {
				rpipe = रिक्त;
			}
		}
		यदि (pi->अंक_progs == 0) अनुवर्त;
#ifndef __U_BOOT__
		save_अंक_progs = pi->अंक_progs; /* save number of programs */
#endif
		rcode = run_pipe_real(pi);
		debug_printf("run_pipe_real returned %d\n",rcode);
#ifndef __U_BOOT__
		यदि (rcode!=-1) {
			/* We only ran a builtin: rcode was set by the return value
			 * of run_pipe_real(), and we don't need to wait for anything. */
		} अन्यथा यदि (pi->followup==PIPE_BG) {
			/* XXX check bash's behavior with nontrivial pipes */
			/* XXX compute jobid */
			/* XXX what does bash do with attempts to background builtins? */
			जोड़ो_bg_job(pi);
			rcode = EXIT_SUCCESS;
		} अन्यथा {
			यदि (interactive) {
				/* move the new process group into the foreground */
				यदि (tcsetpgrp(shell_terminal, pi->pgrp) && errno != ENOTTY)
					perror_संदेश("tcsetpgrp-3");
				rcode = checkjobs(pi);
				/* move the shell to the foreground */
				यदि (tcsetpgrp(shell_terminal, getpgid(0)) && errno != ENOTTY)
					perror_संदेश("tcsetpgrp-4");
			} अन्यथा {
				rcode = checkjobs(pi);
			}
			debug_printf("checkjobs returned %d\n",rcode);
		}
		last_return_code=rcode;
#अन्यथा
		यदि (rcode < -1) {
			last_return_code = -rcode - 2;
			प्रत्यागम -2;	/* exit */
		}
		last_return_code = rcode;
#endif
#ifndef __U_BOOT__
		pi->अंक_progs = save_अंक_progs; /* restore number of programs */
#endif
		यदि ( rmode == RES_IF || rmode == RES_ELIF )
			next_if_code=rcode;  /* can be overwritten a number of times */
		यदि (rmode == RES_WHILE)
			flag_rep = !last_return_code;
		यदि (rmode == RES_UNTIL)
			flag_rep = last_return_code;
		यदि ( (rcode==EXIT_SUCCESS && pi->followup==PIPE_OR) ||
		     (rcode!=EXIT_SUCCESS && pi->followup==PIPE_AND) )
			skip_more_in_this_rmode=rmode;
#ifndef __U_BOOT__
		checkjobs(रिक्त);
#endif
	}
	प्रत्यागम rcode;
}

/* broken, of course, but OK for testing */
स्थिर वर्ण *indenter(पूर्णाङ्क i)
{
	स्थिर वर्ण blanks[]="                                    ";
	प्रत्यागम &blanks[परिमाण(blanks)-i-1];
}

/* return code is the exit status of the pipe */
स्थिर पूर्णाङ्क free_pipe(संरचना pipe *pi, पूर्णाङ्क indent)
{
	वर्ण **p;
	संरचना child_prog *child;
#ifndef __U_BOOT__
	संरचना redir_struct *r, *rnext;
#endif
	पूर्णाङ्क a, i, ret_code=0;
	वर्ण *ind = indenter(indent);

#ifndef __U_BOOT__
	यदि (pi->stopped_progs > 0)
		प्रत्यागम ret_code;
	final_printf("%s run pipe: (pid %d)\n",ind,getpid());
#endif
	प्रति (i=0; i<pi->अंक_progs; i++) {
		child = &pi->progs[i];
		final_printf("%s  command %d:\n",ind,i);
		यदि (child->argv) {
			प्रति (a=0,p=child->argv; *p; a++,p++) {
				final_printf("%s   argv[%d] = %s\n",ind,a,*p);
			}
#ifndef __U_BOOT__
			globfree(&child->glob_result);
#अन्यथा
			प्रति (a = 0; a < child->argc; a++) {
				मुक्त(child->argv[a]);
			}
			मुक्त(child->argv);
			मुक्त(child->argv_nonnull);
			child->argc = 0;
#endif
			child->argv=रिक्त;
		} अन्यथा यदि (child->group) {
#ifndef __U_BOOT__
			final_printf("%s   begin group (subshell:%d)\n",ind, child->subshell);
#endif
			ret_code = free_pipe_list(child->group,indent+3);
			final_printf("%s   end group\n",ind);
		} अन्यथा {
			final_printf("%s   (nil)\n",ind);
		}
#ifndef __U_BOOT__
		प्रति (r=child->redirects; r; r=rnext) {
			final_printf("%s   redirect %d%s", ind, r->fd, redir_table[r->type].descrip);
			यदि (r->dup == -1) {
				/* guard against the case >$FOO, where foo is unset or blank */
				यदि (r->word.gl_pathv) {
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
	प्रत्यागम ret_code;
}

स्थिर पूर्णाङ्क free_pipe_list(संरचना pipe *head, पूर्णाङ्क indent)
{
	पूर्णाङ्क rcode=0;   /* if list has no members */
	संरचना pipe *pi, *next;
	वर्ण *ind = indenter(indent);
	प्रति (pi=head; pi; pi=next) {
		final_printf("%s pipe reserved mode %d\n", ind, pi->r_mode);
		rcode = free_pipe(pi, indent);
		final_printf("%s pipe followup code %d\n", ind, pi->followup);
		next=pi->next;
		pi->next=रिक्त;
		मुक्त(pi);
	}
	प्रत्यागम rcode;
}

/* Select which version we will use */
स्थिर पूर्णाङ्क run_list(संरचना pipe *pi)
{
	पूर्णाङ्क rcode=0;
#ifndef __U_BOOT__
	यदि (fake_mode==0) {
#endif
		rcode = run_list_real(pi);
#ifndef __U_BOOT__
	}
#endif
	/* free_pipe_list has the side effect of clearing memory
	 * In the long run that function can be merged with run_list_real,
	 * but doing that now would hobble the debugging effort. */
	free_pipe_list(pi,0);
	प्रत्यागम rcode;
}

/* The API for glob is arguably broken.  This routine pushes a non-matching
 * string into the output structure, removing non-backslashed backslashes.
 * If someone can prove me wrong, by performing this function within the
 * original glob(3) api, feel free to rewrite this routine into oblivion.
 * Return code (0 vs. GLOB_NOSPACE) matches glob(3).
 * XXX broken if the last character is '\\', check that before calling.
 */
#ifndef __U_BOOT__
स्थिर पूर्णाङ्क globhack(ध्रुव वर्ण *src, पूर्णाङ्क flags, glob_t *pglob)
{
	पूर्णाङ्क cnt=0, pathc;
	ध्रुव वर्ण *s;
	वर्ण *dest;
	प्रति (cnt=1, s=src; s && *s; s++) {
		यदि (*s == '\\') s++;
		cnt++;
	}
	dest = malloc(cnt);
	यदि (!dest) प्रत्यागम GLOB_NOSPACE;
	यदि (!(flags & GLOB_APPEND)) {
		pglob->gl_pathv=रिक्त;
		pglob->gl_pathc=0;
		pglob->gl_offs=0;
		pglob->gl_offs=0;
	}
	pathc = ++pglob->gl_pathc;
	pglob->gl_pathv = realloc(pglob->gl_pathv, (pathc+1)*परिमाण(*pglob->gl_pathv));
	यदि (pglob->gl_pathv == रिक्त) प्रत्यागम GLOB_NOSPACE;
	pglob->gl_pathv[pathc-1]=dest;
	pglob->gl_pathv[pathc]=रिक्त;
	प्रति (s=src; s && *s; s++, dest++) {
		यदि (*s == '\\') s++;
		*dest = *s;
	}
	*dest='\0';
	प्रत्यागम 0;
}

/* XXX broken if the last character is '\\', check that before calling */
स्थिर पूर्णाङ्क glob_needed(ध्रुव वर्ण *s)
{
	प्रति (; *s; s++) {
		यदि (*s == '\\') s++;
		यदि (strchr("*[?",*s)) प्रत्यागम 1;
	}
	प्रत्यागम 0;
}

#यदि 0
स्थिर शून्य globprint(glob_t *pglob)
{
	पूर्णाङ्क i;
	debug_printf("glob_t at %p:\n", pglob);
	debug_printf("  gl_pathc=%d  gl_pathv=%p  gl_offs=%d  gl_flags=%d\n",
		pglob->gl_pathc, pglob->gl_pathv, pglob->gl_offs, pglob->gl_flags);
	प्रति (i=0; i<pglob->gl_pathc; i++)
		debug_printf("pglob->gl_pathv[%d] = %p = %s\n", i,
			pglob->gl_pathv[i], pglob->gl_pathv[i]);
}
#endif

स्थिर पूर्णाङ्क xglob(o_string *dest, पूर्णाङ्क flags, glob_t *pglob)
{
	पूर्णाङ्क gr;

	/* short-circuit for null word */
	/* we can code this better when the debug_printf's are gone */
	यदि (dest->length == 0) {
		यदि (dest->nonnull) {
			/* bash man page calls this an "explicit" null */
			gr = globhack(dest->data, flags, pglob);
			debug_printf("globhack returned %d\n",gr);
		} अन्यथा {
			प्रत्यागम 0;
		}
	} अन्यथा यदि (glob_needed(dest->data)) {
		gr = glob(dest->data, flags, रिक्त, pglob);
		debug_printf("glob returned %d\n",gr);
		यदि (gr == GLOB_NOMATCH) {
			/* quote removal, or more accurately, backslash removal */
			gr = globhack(dest->data, flags, pglob);
			debug_printf("globhack returned %d\n",gr);
		}
	} अन्यथा {
		gr = globhack(dest->data, flags, pglob);
		debug_printf("globhack returned %d\n",gr);
	}
	यदि (gr == GLOB_NOSPACE)
		error_संदेश_and_die("out of memory during glob");
	यदि (gr != 0) { /* GLOB_ABORTED ? */
		error_संदेश("glob(3) error %d",gr);
	}
	/* globprint(glob_target); */
	प्रत्यागम gr;
}
#endif

#ifdef __U_BOOT__
स्थिर वर्ण *get_dollar_var(वर्ण ch);
#endif

/* This is used to get/check local shell variables */
वर्ण *get_local_var(ध्रुव वर्ण *s)
{
	संरचना variables *cur;

	यदि (!s)
		प्रत्यागम रिक्त;

#ifdef __U_BOOT__
	यदि (*s == '$')
		प्रत्यागम get_dollar_var(s[1]);
#endif

	प्रति (cur = शीर्ष_vars; cur; cur=cur->next)
		यदि(strcmp(cur->name, s)==0)
			प्रत्यागम cur->मूल्य;
	प्रत्यागम रिक्त;
}

/* This is used to set local shell variables
   flg_export==0 if only local (not exporting) variable
   flg_export==1 if "new" exporting environ
   flg_export>1  if current startup environ (not call putenv()) */
पूर्णाङ्क set_local_var(ध्रुव वर्ण *s, पूर्णाङ्क flg_export)
{
	वर्ण *name, *मूल्य;
	पूर्णाङ्क result=0;
	संरचना variables *cur;

#ifdef __U_BOOT__
	/* might be possible! */
	यदि (!isalpha(*s))
		प्रत्यागम -1;
#endif

	name=strdup(s);

	/* Assume when we enter this function that we are already in
	 * NAME=VALUE format.  So the first order of business is to
	 * split 's' on the '=' into 'name' and 'value' */
	मूल्य = strchr(name, '=');
	यदि (!मूल्य) {
		मुक्त(name);
		प्रत्यागम -1;
	}
	*मूल्य++ = 0;

	यदि (!*मूल्य) {
		unset_local_var(name);
		मुक्त(name);
		प्रत्यागम 0;
	}

	प्रति(cur = शीर्ष_vars; cur; cur = cur->next) {
		यदि(strcmp(cur->name, name)==0)
			भङ्ग;
	}

	यदि(cur) {
		यदि(strcmp(cur->मूल्य, मूल्य)==0) {
			यदि(flg_export>0 && cur->flg_export==0)
				cur->flg_export=flg_export;
			अन्यथा
				result++;
		} अन्यथा {
			यदि(cur->flg_read_only) {
				error_संदेश("%s: readonly variable", name);
				result = -1;
			} अन्यथा {
				यदि(flg_export>0 || cur->flg_export>1)
					cur->flg_export=1;
				मुक्त(cur->मूल्य);

				cur->मूल्य = strdup(मूल्य);
			}
		}
	} अन्यथा {
		cur = malloc(परिमाण(संरचना variables));
		यदि(!cur) {
			result = -1;
		} अन्यथा {
			cur->name = strdup(name);
			यदि (cur->name == रिक्त) {
				मुक्त(cur);
				result = -1;
			} अन्यथा {
				संरचना variables *bottom = शीर्ष_vars;
				cur->मूल्य = strdup(मूल्य);
				cur->next = रिक्त;
				cur->flg_export = flg_export;
				cur->flg_read_only = 0;
				यावत्(bottom->next) bottom=bottom->next;
				bottom->next = cur;
			}
		}
	}

#ifndef __U_BOOT__
	यदि(result==0 && cur->flg_export==1) {
		*(मूल्य-1) = '=';
		result = putenv(name);
	} अन्यथा {
#endif
		मुक्त(name);
#ifndef __U_BOOT__
		यदि(result>0)            /* equivalent to previous set */
			result = 0;
	}
#endif
	प्रत्यागम result;
}

शून्य unset_local_var(ध्रुव वर्ण *name)
{
	संरचना variables *cur;

	यदि (name) {
		प्रति (cur = शीर्ष_vars; cur; cur=cur->next) {
			यदि(strcmp(cur->name, name)==0)
				भङ्ग;
		}
		यदि (cur != रिक्त) {
			संरचना variables *next = शीर्ष_vars;
			यदि(cur->flg_read_only) {
				error_संदेश("%s: readonly variable", name);
				प्रत्यागम;
			} अन्यथा {
#ifndef __U_BOOT__
				यदि(cur->flg_export)
					unenv_set(cur->name);
#endif
				मुक्त(cur->name);
				मुक्त(cur->मूल्य);
				यावत् (next->next != cur)
					next = next->next;
				next->next = cur->next;
			}
			मुक्त(cur);
		}
	}
}

स्थिर पूर्णाङ्क is_assignment(ध्रुव वर्ण *s)
{
	यदि (s == रिक्त)
		प्रत्यागम 0;

	यदि (!isalpha(*s)) प्रत्यागम 0;
	++s;
	यावत्(isalnum(*s) || *s=='_') ++s;
	प्रत्यागम *s=='=';
}

#ifndef __U_BOOT__
/* the src parameter allows us to peek forward to a possible &n syntax
 * for file descriptor duplication, e.g., "2>&1".
 * Return code is 0 normally, 1 if a syntax error is detected in src.
 * Resource errors (in xmalloc) cause the process to exit */
स्थिर पूर्णाङ्क setup_redirect(संरचना p_context *ctx, पूर्णाङ्क fd, redir_type style,
	संरचना in_str *निवेश)
{
	संरचना child_prog *child=ctx->child;
	संरचना redir_struct *redir = child->redirects;
	संरचना redir_struct *last_redir=रिक्त;

	/* Create a new redir_struct and drop it onto the end of the linked list */
	यावत्(redir) {
		last_redir=redir;
		redir=redir->next;
	}
	redir = xmalloc(परिमाण(संरचना redir_struct));
	redir->next=रिक्त;
	redir->word.gl_pathv=रिक्त;
	यदि (last_redir) {
		last_redir->next=redir;
	} अन्यथा {
		child->redirects=redir;
	}

	redir->type=style;
	redir->fd= (fd==-1) ? redir_table[style].default_fd : fd ;

	debug_printf("Redirect type %d%s\n", redir->fd, redir_table[style].descrip);

	/* Check for a '2>&1' type redirect */
	redir->dup = redirect_dup_अंक(निवेश);
	यदि (redir->dup == -2) प्रत्यागम 1;  /* syntax error */
	यदि (redir->dup != -1) {
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
	प्रत्यागम 0;
}
#endif

स्थिर संरचना pipe *new_pipe(शून्य)
{
	संरचना pipe *pi;
	pi = xmalloc(परिमाण(संरचना pipe));
	pi->अंक_progs = 0;
	pi->progs = रिक्त;
	pi->next = रिक्त;
	pi->followup = 0;  /* invalid */
	pi->r_mode = RES_NONE;
	प्रत्यागम pi;
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
	ctx->ढेर=रिक्त;
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
	वर्ण *literal;
	पूर्णाङ्क code;
	long flag;
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
#define NRES (परिमाण(reserved_list)/परिमाण(संरचना reserved_combo))

स्थिर पूर्णाङ्क reserved_word(o_string *dest, संरचना p_context *ctx)
{
	संरचना reserved_combo *r;
	प्रति (r=reserved_list;
		r<reserved_list+NRES; r++) {
		यदि (strcmp(dest->data, r->literal) == 0) {
			debug_printf("found reserved word %s, code %d\n",r->literal,r->code);
			यदि (r->flag & FLAG_START) {
				संरचना p_context *new = xmalloc(परिमाण(संरचना p_context));
				debug_printf("push stack\n");
				यदि (ctx->w == RES_IN || ctx->w == RES_FOR) {
					syntax();
					मुक्त(new);
					ctx->w = RES_SNTX;
					b_reset(dest);
					प्रत्यागम 1;
				}
				*new = *ctx;   /* physical copy */
				initialize_context(ctx);
				ctx->ढेर=new;
			} अन्यथा यदि ( ctx->w == RES_NONE || ! (ctx->old_flag & (1<<r->code))) {
				syntax();
				ctx->w = RES_SNTX;
				b_reset(dest);
				प्रत्यागम 1;
			}
			ctx->w=r->code;
			ctx->old_flag = r->flag;
			यदि (ctx->old_flag & FLAG_END) {
				संरचना p_context *old;
				debug_printf("pop stack\n");
				done_pipe(ctx,PIPE_SEQ);
				old = ctx->ढेर;
				old->child->group = ctx->list_head;
#ifndef __U_BOOT__
				old->child->subshell = 0;
#endif
				*ctx = *old;   /* physical copy */
				मुक्त(old);
			}
			b_reset (dest);
			प्रत्यागम 1;
		}
	}
	प्रत्यागम 0;
}

/* normal return is 0.
 * Syntax or xglob errors return 1. */
स्थिर पूर्णाङ्क done_word(o_string *dest, संरचना p_context *ctx)
{
	संरचना child_prog *child=ctx->child;
#ifndef __U_BOOT__
	glob_t *glob_target;
	पूर्णाङ्क gr, flags = 0;
#अन्यथा
	वर्ण *str, *s;
	पूर्णाङ्क argc, cnt;
#endif

	debug_printf("done_word: %s %p\n", dest->data, child);
	यदि (dest->length == 0 && !dest->nonnull) {
		debug_printf("  true null, ignored\n");
		प्रत्यागम 0;
	}
#ifndef __U_BOOT__
	यदि (ctx->pending_redirect) {
		glob_target = &ctx->pending_redirect->word;
	} अन्यथा {
#endif
		यदि (child->group) {
			syntax();
			प्रत्यागम 1;  /* syntax error, groups and arglists don't mix */
		}
		यदि (!child->argv && (ctx->type & FLAG_PARSE_SEMICOLON)) {
			debug_printf("checking %s for reserved-ness\n",dest->data);
			यदि (reserved_word(dest,ctx)) प्रत्यागम ctx->w==RES_SNTX;
		}
#ifndef __U_BOOT__
		glob_target = &child->glob_result;
		यदि (child->argv) flags |= GLOB_APPEND;
#अन्यथा
		प्रति (cnt = 1, s = dest->data; s && *s; s++) {
			यदि (*s == '\\') s++;
			cnt++;
		}
		str = malloc(cnt);
		यदि (!str) प्रत्यागम 1;
		यदि ( child->argv == रिक्त) {
			child->argc=0;
		}
		argc = ++child->argc;
		child->argv = realloc(child->argv, (argc+1)*परिमाण(*child->argv));
		यदि (child->argv == रिक्त) {
			मुक्त(str);
			प्रत्यागम 1;
		}
		child->argv_nonnull = realloc(child->argv_nonnull,
					(argc+1)*परिमाण(*child->argv_nonnull));
		यदि (child->argv_nonnull == रिक्त) {
			मुक्त(str);
			प्रत्यागम 1;
		}
		child->argv[argc-1]=str;
		child->argv_nonnull[argc-1] = dest->nonnull;
		child->argv[argc]=रिक्त;
		child->argv_nonnull[argc] = 0;
		प्रति (s = dest->data; s && *s; s++,str++) {
			यदि (*s == '\\') s++;
			*str = *s;
		}
		*str = '\0';
#endif
#ifndef __U_BOOT__
	}
	gr = xglob(dest, flags, glob_target);
	यदि (gr != 0) प्रत्यागम 1;
#endif

	b_reset(dest);
#ifndef __U_BOOT__
	यदि (ctx->pending_redirect) {
		ctx->pending_redirect=रिक्त;
		यदि (glob_target->gl_pathc != 1) {
			error_संदेश("ambiguous redirect");
			प्रत्यागम 1;
		}
	} अन्यथा {
		child->argv = glob_target->gl_pathv;
	}
#endif
	यदि (ctx->w == RES_FOR) {
		done_word(dest,ctx);
		done_pipe(ctx,PIPE_SEQ);
	}
	प्रत्यागम 0;
}

/* The only possible error here is out of memory, in which case
 * xmalloc exits. */
स्थिर पूर्णाङ्क done_command(संरचना p_context *ctx)
{
	/* The child is really already in the pipe structure, so
	 * advance the pipe counter and make a new, null child.
	 * Only real trickiness here is that the uncommitted
	 * child structure, to which ctx->child points, is not
	 * counted in pi->num_progs. */
	संरचना pipe *pi=ctx->pipe;
	संरचना child_prog *prog=ctx->child;

	यदि (prog && prog->group == रिक्त
		 && prog->argv == रिक्त
#ifndef __U_BOOT__
		 && prog->redirects == रिक्त) {
#अन्यथा
										) {
#endif
		debug_printf("done_command: skipping null command\n");
		प्रत्यागम 0;
	} अन्यथा यदि (prog) {
		pi->अंक_progs++;
		debug_printf("done_command: num_progs incremented to %d\n",pi->अंक_progs);
	} अन्यथा {
		debug_printf("done_command: initializing\n");
	}
	pi->progs = xrealloc(pi->progs, परिमाण(*pi->progs) * (pi->अंक_progs+1));

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
	प्रत्यागम 0;
}

स्थिर पूर्णाङ्क done_pipe(संरचना p_context *ctx, pipe_style type)
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
	प्रत्यागम 0;
}

#ifndef __U_BOOT__
/* peek ahead in the in_str to find out if we have a "&n" construct,
 * as in "2>&1", that represents duplicating a file descriptor.
 * returns either -2 (syntax error), -1 (no &), or the number found.
 */
स्थिर पूर्णाङ्क redirect_dup_अंक(संरचना in_str *निवेश)
{
	पूर्णाङ्क ch, d=0, ok=0;
	ch = b_peek(निवेश);
	यदि (ch != '&') प्रत्यागम -1;

	b_getch(निवेश);  /* get the & */
	ch=b_peek(निवेश);
	यदि (ch == '-') {
		b_getch(निवेश);
		प्रत्यागम -3;  /* "-" represents "close me" */
	}
	यावत् (isdigit(ch)) {
		d = d*10+(ch-'0');
		ok=1;
		b_getch(निवेश);
		ch = b_peek(निवेश);
	}
	यदि (ok) प्रत्यागम d;

	error_संदेश("ambiguous redirect");
	प्रत्यागम -2;
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
स्थिर पूर्णाङ्क redirect_opt_अंक(o_string *o)
{
	पूर्णाङ्क अंक;

	यदि (o->length==0) प्रत्यागम -1;
	प्रति(अंक=0; अंक<o->length; अंक++) {
		यदि (!isdigit(*(o->data+अंक))) {
			प्रत्यागम -1;
		}
	}
	/* reuse num (and save an int) */
	अंक=atoi(o->data);
	b_reset(o);
	प्रत्यागम अंक;
}

FILE *generate_stream_from_list(संरचना pipe *head)
{
	FILE *pf;
#यदि 1
	पूर्णाङ्क pid, channel[2];
	यदि (pipe(channel)<0) perror_संदेश_and_die("pipe");
	pid=fork();
	यदि (pid<0) {
		perror_संदेश_and_die("fork");
	} अन्यथा यदि (pid==0) {
		close(channel[0]);
		यदि (channel[1] != 1) {
			dup2(channel[1],1);
			close(channel[1]);
		}
#यदि 0
#define SURROGATE "surrogate response"
		लिख(1,SURROGATE,परिमाण(SURROGATE));
		_exit(run_list(head));
#अन्यथा
		_exit(run_list_real(head));   /* leaks memory */
#endif
	}
	debug_printf("forked child %d\n",pid);
	close(channel[1]);
	pf = fdopen(channel[0],"r");
	debug_printf("pipe on FILE *%p\n",pf);
#अन्यथा
	free_pipe_list(head,0);
	pf=popen("echo surrogate response","r");
	debug_printf("started fake pipe on FILE *%p\n",pf);
#endif
	प्रत्यागम pf;
}

/* this version hacked for testing purposes */
/* return code is exit status of the process that is run. */
स्थिर पूर्णाङ्क process_command_subs(o_string *dest, संरचना p_context *ctx, संरचना in_str *निवेश, पूर्णाङ्क subst_end)
{
	पूर्णाङ्क retcode;
	o_string result=NULL_O_STRING;
	संरचना p_context inner;
	FILE *p;
	संरचना in_str pipe_str;
	initialize_context(&inner);

	/* recursion to generate command */
	retcode = parse_stream(&result, &inner, निवेश, subst_end);
	यदि (retcode != 0) प्रत्यागम retcode;  /* syntax error or EOF */
	done_word(&result, &inner);
	done_pipe(&inner, PIPE_SEQ);
	b_free(&result);

	p=generate_stream_from_list(inner.list_head);
	यदि (p==रिक्त) प्रत्यागम 1;
	mark_open(fileno(p));
	setup_file_in_str(&pipe_str, p);

	/* now send results of command back into original context */
	retcode = parse_stream(dest, ctx, &pipe_str, '\0');
	/* XXX In case of a syntax error, should we try to kill the child?
	 * That would be tough to do right, so just read until EOF. */
	यदि (retcode == 1) {
		यावत् (b_getch(&pipe_str)!=EOF) { /* discard */ };
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
	प्रत्यागम retcode;
}

स्थिर पूर्णाङ्क parse_group(o_string *dest, संरचना p_context *ctx,
	संरचना in_str *निवेश, पूर्णाङ्क ch)
{
	पूर्णाङ्क rcode, endch=0;
	संरचना p_context sub;
	संरचना child_prog *child = ctx->child;
	यदि (child->argv) {
		syntax();
		प्रत्यागम 1;  /* syntax error, groups and arglists don't mix */
	}
	initialize_context(&sub);
	विकल्प(ch) {
		पक्ष '(': endch=')'; child->subshell=1; भङ्ग;
		पक्ष '{': endch='}'; भङ्ग;
		स्वभाव: syntax();   /* really logic error */
	}
	rcode=parse_stream(dest,&sub,निवेश,endch);
	done_word(dest,&sub); /* finish off the final word in the subcontext */
	done_pipe(&sub, PIPE_SEQ);  /* and the final command there, too */
	child->group = sub.list_head;
	प्रत्यागम rcode;
	/* child remains "open", available for possible redirects */
}
#endif

/* basically useful version until someone wants to get fancier,
 * see the bash man page under "Parameter Expansion" */
स्थिर वर्ण *lookup_param(वर्ण *src)
{
	वर्ण *p;
	वर्ण *sep;
	वर्ण *default_val = रिक्त;
	पूर्णाङ्क assign = 0;
	पूर्णाङ्क expand_empty = 0;

	यदि (!src)
		प्रत्यागम रिक्त;

	sep = strchr(src, ':');

	यदि (sep) {
		*sep = '\0';
		यदि (*(sep + 1) == '-')
			default_val = sep+2;
		यदि (*(sep + 1) == '=') {
			default_val = sep+2;
			assign = 1;
		}
		यदि (*(sep + 1) == '+') {
			default_val = sep+2;
			expand_empty = 1;
		}
	}

	p = env_get(src);
	यदि (!p)
		p = get_local_var(src);

	यदि (!p || strlen(p) == 0) {
		p = default_val;
		यदि (assign) {
			वर्ण *var = malloc(strlen(src)+strlen(default_val)+2);
			यदि (var) {
				sprintf(var, "%s=%s", src, default_val);
				set_local_var(var, 0);
			}
			मुक्त(var);
		}
	} अन्यथा यदि (expand_empty) {
		p += strlen(p);
	}

	यदि (sep)
		*sep = ':';

	प्रत्यागम p;
}

#ifdef __U_BOOT__
स्थिर वर्ण *get_dollar_var(वर्ण ch)
{
	स्थिर वर्ण buf[40];

	buf[0] = '\0';
	विकल्प (ch) {
		पक्ष '?':
			sprintf(buf, "%u", (unsigned पूर्णाङ्क)last_return_code);
			भङ्ग;
		स्वभाव:
			प्रत्यागम रिक्त;
	}
	प्रत्यागम buf;
}
#endif

/* return code: 0 for OK, 1 for syntax error */
स्थिर पूर्णाङ्क handle_dollar(o_string *dest, संरचना p_context *ctx, संरचना in_str *निवेश)
{
#ifndef __U_BOOT__
	पूर्णाङ्क i, advance=0;
#अन्यथा
	पूर्णाङ्क advance=0;
#endif
#ifndef __U_BOOT__
	वर्ण sep[]=" ";
#endif
	पूर्णाङ्क ch = निवेश->peek(निवेश);  /* first character after the $ */
	debug_printf("handle_dollar: ch=%c\n",ch);
	यदि (isalpha(ch)) {
		b_addchr(dest, SPECIAL_VAR_प्रतीक);
		ctx->child->sp++;
		यावत्(ch=b_peek(निवेश),isalnum(ch) || ch=='_') {
			b_getch(निवेश);
			b_addchr(dest,ch);
		}
		b_addchr(dest, SPECIAL_VAR_प्रतीक);
#ifndef __U_BOOT__
	} अन्यथा यदि (isdigit(ch)) {
		i = ch-'0';  /* XXX is $0 special? */
		यदि (i<global_argc) {
			parse_string(dest, ctx, global_argv[i]); /* recursion */
		}
		advance = 1;
#endif
	} अन्यथा विकल्प (ch) {
#ifndef __U_BOOT__
		पक्ष '$':
			b_adduint(dest,getpid());
			advance = 1;
			भङ्ग;
		पक्ष '!':
			यदि (last_bg_pid > 0) b_adduint(dest, last_bg_pid);
			advance = 1;
			भङ्ग;
#endif
		पक्ष '?':
#ifndef __U_BOOT__
			b_adduint(dest,last_return_code);
#अन्यथा
			ctx->child->sp++;
			b_addchr(dest, SPECIAL_VAR_प्रतीक);
			b_addchr(dest, '$');
			b_addchr(dest, '?');
			b_addchr(dest, SPECIAL_VAR_प्रतीक);
#endif
			advance = 1;
			भङ्ग;
#ifndef __U_BOOT__
		पक्ष '#':
			b_adduint(dest,global_argc ? global_argc-1 : 0);
			advance = 1;
			भङ्ग;
#endif
		पक्ष '{':
			b_addchr(dest, SPECIAL_VAR_प्रतीक);
			ctx->child->sp++;
			b_getch(निवेश);
			/* XXX maybe someone will try to escape the '}' */
			यावत्(ch=b_getch(निवेश),ch!=EOF && ch!='}') {
				b_addchr(dest,ch);
			}
			यदि (ch != '}') {
				syntax();
				प्रत्यागम 1;
			}
			b_addchr(dest, SPECIAL_VAR_प्रतीक);
			भङ्ग;
#ifndef __U_BOOT__
		पक्ष '(':
			b_getch(निवेश);
			process_command_subs(dest, ctx, निवेश, ')');
			भङ्ग;
		पक्ष '*':
			sep[0]=ifs[0];
			प्रति (i=1; i<global_argc; i++) {
				parse_string(dest, ctx, global_argv[i]);
				यदि (i+1 < global_argc) parse_string(dest, ctx, sep);
			}
			भङ्ग;
		पक्ष '@':
		पक्ष '-':
		पक्ष '_':
			/* still unhandled, but should be eventually */
			error_संदेश("unhandled syntax: $%c",ch);
			प्रत्यागम 1;
			भङ्ग;
#endif
		स्वभाव:
			b_addqchr(dest,'$',dest->quote);
	}
	/* Eat the character if the flag was set.  If the compiler
	 * is smart enough, we could substitute "b_getch(input);"
	 * for all the "advance = 1;" above, and also end up with
	 * a nice size-optimized program.  Hah!  That'll be the day.
	 */
	यदि (advance) b_getch(निवेश);
	प्रत्यागम 0;
}

#ifndef __U_BOOT__
पूर्णाङ्क parse_string(o_string *dest, संरचना p_context *ctx, ध्रुव वर्ण *src)
{
	संरचना in_str foo;
	setup_string_in_str(&foo, src);
	प्रत्यागम parse_stream(dest, ctx, &foo, '\0');
}
#endif

/* return code is 0 for normal exit, 1 for syntax error */
स्थिर पूर्णाङ्क parse_stream(o_string *dest, संरचना p_context *ctx,
			संरचना in_str *निवेश, पूर्णाङ्क end_trigger)
{
	unsigned पूर्णाङ्क ch, m;
#ifndef __U_BOOT__
	पूर्णाङ्क redir_fd;
	redir_type redir_style;
#endif
	पूर्णाङ्क next;

	/* Only double-quote state is handled in the state variable dest->quote.
	 * A single-quote triggers a bypass of the main loop until its mate is
	 * found.  When recursing, quote state is passed in via dest->quote. */

	debug_printf("parse_stream, end_trigger=%d\n",end_trigger);
	यावत् ((ch=b_getch(निवेश))!=EOF) {
		m = map[ch];
#ifdef __U_BOOT__
		यदि (निवेश->__promptme == 0) प्रत्यागम 1;
#endif
		next = (ch == '\n') ? 0 : b_peek(निवेश);

		debug_printf("parse_stream: ch=%c (%d) m=%d quote=%d - %c\n",
			ch >= ' ' ? ch : '.', ch, m,
			dest->quote, ctx->ढेर == रिक्त ? '*' : '.');

		यदि (m==0 || ((m==1 || m==2) && dest->quote)) {
			b_addqchr(dest, ch, dest->quote);
		} अन्यथा {
			यदि (m==2) {  /* unquoted IFS */
				यदि (done_word(dest, ctx)) {
					प्रत्यागम 1;
				}
				/* If we aren't performing a substitution, treat a newline as a
				 * command separator.  */
				यदि (end_trigger != '\0' && ch=='\n')
					done_pipe(ctx,PIPE_SEQ);
			}
			यदि (ch == end_trigger && !dest->quote && ctx->w==RES_NONE) {
				debug_printf("leaving parse_stream (triggered)\n");
				प्रत्यागम 0;
			}
#यदि 0
			यदि (ch=='\n') {
				/* Yahoo!  Time to run with it! */
				done_pipe(ctx,PIPE_SEQ);
				run_list(ctx->list_head);
				initialize_context(ctx);
			}
#endif
			यदि (m!=2) विकल्प (ch) {
		पक्ष '#':
			यदि (dest->length == 0 && !dest->quote) {
				यावत्(ch=b_peek(निवेश),ch!=EOF && ch!='\n') { b_getch(निवेश); }
			} अन्यथा {
				b_addqchr(dest, ch, dest->quote);
			}
			भङ्ग;
		पक्ष '\\':
			यदि (next == EOF) {
				syntax();
				प्रत्यागम 1;
			}
			b_addqchr(dest, '\\', dest->quote);
			b_addqchr(dest, b_getch(निवेश), dest->quote);
			भङ्ग;
		पक्ष '$':
			यदि (handle_dollar(dest, ctx, निवेश)!=0) प्रत्यागम 1;
			भङ्ग;
		पक्ष '\'':
			dest->nonnull = 1;
			यावत्(ch=b_getch(निवेश),ch!=EOF && ch!='\'') {
#ifdef __U_BOOT__
				यदि(निवेश->__promptme == 0) प्रत्यागम 1;
#endif
				b_addchr(dest,ch);
			}
			यदि (ch==EOF) {
				syntax();
				प्रत्यागम 1;
			}
			भङ्ग;
		पक्ष '"':
			dest->nonnull = 1;
			dest->quote = !dest->quote;
			भङ्ग;
#ifndef __U_BOOT__
		पक्ष '`':
			process_command_subs(dest, ctx, निवेश, '`');
			भङ्ग;
		पक्ष '>':
			redir_fd = redirect_opt_अंक(dest);
			done_word(dest, ctx);
			redir_style=REDIRECT_OVERWRITE;
			यदि (next == '>') {
				redir_style=REDIRECT_APPEND;
				b_getch(निवेश);
			} अन्यथा यदि (next == '(') {
				syntax();   /* until we support >(list) Process Substitution */
				प्रत्यागम 1;
			}
			setup_redirect(ctx, redir_fd, redir_style, निवेश);
			भङ्ग;
		पक्ष '<':
			redir_fd = redirect_opt_अंक(dest);
			done_word(dest, ctx);
			redir_style=REDIRECT_निवेश;
			यदि (next == '<') {
				redir_style=REDIRECT_HEREIS;
				b_getch(निवेश);
			} अन्यथा यदि (next == '>') {
				redir_style=REDIRECT_IO;
				b_getch(निवेश);
			} अन्यथा यदि (next == '(') {
				syntax();   /* until we support <(list) Process Substitution */
				प्रत्यागम 1;
			}
			setup_redirect(ctx, redir_fd, redir_style, निवेश);
			भङ्ग;
#endif
		पक्ष ';':
			done_word(dest, ctx);
			done_pipe(ctx,PIPE_SEQ);
			भङ्ग;
		पक्ष '&':
			done_word(dest, ctx);
			यदि (next=='&') {
				b_getch(निवेश);
				done_pipe(ctx,PIPE_AND);
			} अन्यथा {
#ifndef __U_BOOT__
				done_pipe(ctx,PIPE_BG);
#अन्यथा
				syntax_err();
				प्रत्यागम 1;
#endif
			}
			भङ्ग;
		पक्ष '|':
			done_word(dest, ctx);
			यदि (next=='|') {
				b_getch(निवेश);
				done_pipe(ctx,PIPE_OR);
			} अन्यथा {
				/* we could pick up a file descriptor choice here
				 * with redirect_opt_num(), but bash doesn't do it.
				 * "echo foo 2| cat" yields "foo 2". */
#ifndef __U_BOOT__
				done_command(ctx);
#अन्यथा
				syntax_err();
				प्रत्यागम 1;
#endif
			}
			भङ्ग;
#ifndef __U_BOOT__
		पक्ष '(':
		पक्ष '{':
			यदि (parse_group(dest, ctx, निवेश, ch)!=0) प्रत्यागम 1;
			भङ्ग;
		पक्ष ')':
		पक्ष '}':
			syntax();   /* Proper use of this character caught by end_trigger */
			प्रत्यागम 1;
			भङ्ग;
#endif
		पक्ष SUBSTED_VAR_प्रतीक:
			dest->nonnull = 1;
			यावत् (ch = b_getch(निवेश), ch != EOF &&
			    ch != SUBSTED_VAR_प्रतीक) {
				debug_printf("subst, pass=%d\n", ch);
				यदि (निवेश->__promptme == 0)
					प्रत्यागम 1;
				b_addchr(dest, ch);
			}
			debug_printf("subst, term=%d\n", ch);
			यदि (ch == EOF) {
				syntax();
				प्रत्यागम 1;
			}
			भङ्ग;
		स्वभाव:
			syntax();   /* this is really an internal logic error */
			प्रत्यागम 1;
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
	यदि (end_trigger != '\0') प्रत्यागम -1;
	प्रत्यागम 0;
}

स्थिर शून्य mapset(ध्रुव unsigned वर्ण *set, पूर्णाङ्क code)
{
	ध्रुव unsigned वर्ण *s;
	प्रति (s=set; *s; s++) map[*s] = code;
}

स्थिर शून्य update_ifs_map(शून्य)
{
	/* char *ifs and char map[256] are both globals. */
	ifs = (uchar *)env_get("IFS");
	यदि (ifs == रिक्त) ifs=(uchar *)" \t\n";
	/* Precompute a list of 'flow through' behavior so it can be treated
	 * quickly up front.  Computation is necessary because of IFS.
	 * Special case handling of IFS == " \t\n" is not implemented.
	 * The map[] array only really needs two bits each, and on most machines
	 * that would be faster because of the reduced L1 cache footprint.
	 */
	memset(map,0,परिमाण(map)); /* most characters flow through always */
#ifndef __U_BOOT__
	mapset((uchar *)"\\$'\"`", 3);      /* never flow through */
	mapset((uchar *)"<>;&|(){}#", 1);   /* flow through if quoted */
#अन्यथा
	{
		uchar subst[2] = {SUBSTED_VAR_प्रतीक, 0};
		mapset(subst, 3);       /* never flow through */
	}
	mapset((uchar *)"\\$'\"", 3);       /* never flow through */
	mapset((uchar *)";&|#", 1);         /* flow through if quoted */
#endif
	mapset(ifs, 2);            /* also flow through if quoted */
}

/* most recursion does not come through here, the exeception is
 * from builtin_source() */
स्थिर पूर्णाङ्क parse_stream_outer(संरचना in_str *inp, पूर्णाङ्क flag)
{

	संरचना p_context ctx;
	o_string temp=NULL_O_STRING;
	पूर्णाङ्क rcode;
#ifdef __U_BOOT__
	पूर्णाङ्क code = 1;
#endif
	कुरु {
		ctx.type = flag;
		initialize_context(&ctx);
		update_ifs_map();
		यदि (!(flag & FLAG_PARSE_SEMICOLON) || (flag & FLAG_REPARSING)) mapset((uchar *)";$&|", 0);
		inp->promptmode=1;
		rcode = parse_stream(&temp, &ctx, inp,
				     flag & FLAG_CONT_ON_NEWLINE ? -1 : '\n');
#ifdef __U_BOOT__
		यदि (rcode == 1) flag_repeat = 0;
#endif
		यदि (rcode != 1 && ctx.old_flag != 0) {
			syntax();
#ifdef __U_BOOT__
			flag_repeat = 0;
#endif
		}
		यदि (rcode != 1 && ctx.old_flag == 0) {
			done_word(&temp, &ctx);
			done_pipe(&ctx,PIPE_SEQ);
#ifndef __U_BOOT__
			run_list(ctx.list_head);
#अन्यथा
			code = run_list(ctx.list_head);
			यदि (code == -2) {	/* exit */
				b_free(&temp);
				code = 0;
				/* XXX hackish way to not allow exit from main loop */
				यदि (inp->peek == file_peek) {
					printf("exit not allowed from main input shell.\n");
					अनुवर्त;
				}
				/*
				 * DANGER
				 * Return code -2 is special in this context,
				 * it indicates exit from inner pipe instead
				 * of return code itself, the return code is
				 * stored in 'last_return_code' variable!
				 * DANGER
				 */
				प्रत्यागम -2;
			}
			यदि (code == -1)
			    flag_repeat = 0;
#endif
		} अन्यथा {
			यदि (ctx.old_flag != 0) {
				मुक्त(ctx.ढेर);
				b_reset(&temp);
			}
#ifdef __U_BOOT__
			यदि (inp->__promptme == 0) printf("<INTERRUPT>\n");
			inp->__promptme = 1;
#endif
			temp.nonnull = 0;
			temp.quote = 0;
			inp->p = रिक्त;
			free_pipe_list(ctx.list_head,0);
		}
		b_free(&temp);
	/* loop on syntax errors, return on EOF */
	} यावत् (rcode != -1 && !(flag & FLAG_EXIT_FROM_LOOP) &&
		(inp->peek != static_peek || b_peek(inp)));
#ifndef __U_BOOT__
	प्रत्यागम 0;
#अन्यथा
	प्रत्यागम (code != 0) ? 1 : 0;
#endif /* __U_BOOT__ */
}

#ifndef __U_BOOT__
स्थिर पूर्णाङ्क parse_string_outer(ध्रुव वर्ण *s, पूर्णाङ्क flag)
#अन्यथा
पूर्णाङ्क parse_string_outer(ध्रुव वर्ण *s, पूर्णाङ्क flag)
#endif	/* __U_BOOT__ */
{
	संरचना in_str निवेश;
	पूर्णाङ्क rcode;
#ifdef __U_BOOT__
	वर्ण *p = रिक्त;
	यदि (!s)
		प्रत्यागम 1;
	यदि (!*s)
		प्रत्यागम 0;
	यदि (!(p = strchr(s, '\n')) || *++p) {
		p = xmalloc(strlen(s) + 2);
		strcpy(p, s);
		strcat(p, "\n");
		setup_string_in_str(&निवेश, p);
		rcode = parse_stream_outer(&निवेश, flag);
		मुक्त(p);
		प्रत्यागम rcode == -2 ? last_return_code : rcode;
	} अन्यथा {
#endif
	setup_string_in_str(&निवेश, s);
	rcode = parse_stream_outer(&निवेश, flag);
	प्रत्यागम rcode == -2 ? last_return_code : rcode;
#ifdef __U_BOOT__
	}
#endif
}

#ifndef __U_BOOT__
स्थिर पूर्णाङ्क parse_file_outer(FILE *f)
#अन्यथा
पूर्णाङ्क parse_file_outer(शून्य)
#endif
{
	पूर्णाङ्क rcode;
	संरचना in_str निवेश;
#ifndef __U_BOOT__
	setup_file_in_str(&निवेश, f);
#अन्यथा
	setup_file_in_str(&निवेश);
#endif
	rcode = parse_stream_outer(&निवेश, FLAG_PARSE_SEMICOLON);
	प्रत्यागम rcode == -2 ? last_return_code : rcode;
}

#ifdef __U_BOOT__
पूर्णाङ्क u_boot_hush_start(शून्य)
{
	यदि (शीर्ष_vars == रिक्त) {
		शीर्ष_vars = malloc(परिमाण(संरचना variables));
		शीर्ष_vars->name = "HUSH_VERSION";
		शीर्ष_vars->मूल्य = "0.01";
		शीर्ष_vars->next = रिक्त;
		शीर्ष_vars->flg_export = 0;
		शीर्ष_vars->flg_read_only = 1;
	}
	प्रत्यागम 0;
}

स्थिर शून्य *xmalloc(size_t size)
{
	शून्य *p = रिक्त;

	यदि (!(p = malloc(size))) {
	    printf("ERROR : xmalloc failed\n");
	    प्रति(;;);
	}
	प्रत्यागम p;
}

स्थिर शून्य *xrealloc(शून्य *ptr, size_t size)
{
	शून्य *p = रिक्त;

	यदि (!(p = realloc(ptr, size))) {
	    printf("ERROR : xrealloc failed\n");
	    प्रति(;;);
	}
	प्रत्यागम p;
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
	यावत् (tcgetpgrp (shell_terminal) != (shell_pgrp = getpgrp ()))
		kill (- shell_pgrp, SIGTTIN);

	/* Ignore interactive and job-control signals.  */
	सङ्केत(SIGINT, SIG_IGN);
	सङ्केत(SIGQUIT, SIG_IGN);
	सङ्केत(SIGTERM, SIG_IGN);
	सङ्केत(SIGTSTP, SIG_IGN);
	सङ्केत(SIGTTIN, SIG_IGN);
	सङ्केत(SIGTTOU, SIG_IGN);
	सङ्केत(SIGCHLD, SIG_IGN);

	/* Put ourselves in our own process group.  */
	setsid();
	shell_pgrp = getpid ();
	setpgid (shell_pgrp, shell_pgrp);

	/* Grab control of the terminal.  */
	tcsetpgrp(shell_terminal, shell_pgrp);
}

पूर्णाङ्क hush_main(पूर्णाङ्क argc, वर्ण * ध्रुव *argv)
{
	पूर्णाङ्क opt;
	FILE *निवेश;
	वर्ण **e = environ;

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
	यदि (e) {
		प्रति (; *e; e++)
			set_local_var(*e, 2);   /* without call putenv() */
	}

	last_return_code=EXIT_SUCCESS;

	यदि (argv[0] && argv[0][0] == '-') {
		debug_printf("\nsourcing /etc/profile\n");
		यदि ((निवेश = fopen("/etc/profile", "r")) != रिक्त) {
			mark_open(fileno(निवेश));
			parse_file_outer(निवेश);
			mark_closed(fileno(निवेश));
			fclose(निवेश);
		}
	}
	निवेश=stdin;

	यावत् ((opt = getopt(argc, argv, "c:xif")) > 0) {
		विकल्प (opt) {
			पक्ष 'c':
				{
					global_argv = argv+optind;
					global_argc = argc-optind;
					opt = parse_string_outer(optarg, FLAG_PARSE_SEMICOLON);
					गच्छ final_return;
				}
				भङ्ग;
			पक्ष 'i':
				interactive++;
				भङ्ग;
			पक्ष 'f':
				fake_mode++;
				भङ्ग;
			स्वभाव:
#ifndef BB_VER
				fprintf(stderr, "Usage: sh [FILE]...\n"
						"   or: sh -c command [args]...\n\n");
				exit(EXIT_FAILURE);
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
	यदि (argv[optind]==रिक्त && निवेश==stdin &&
			isatty(fileno(stdin)) && isatty(fileno(stdout))) {
		interactive++;
	}

	debug_printf("\ninteractive=%d\n", interactive);
	यदि (interactive) {
		/* Looks like they want an interactive shell */
#ifndef CONFIG_FEATURE_SH_EXTRA_QUIET
		printf( "\n\n" BB_BANNER " hush - the humble shell v0.01 (testing)\n");
		printf( "Enter 'help' for a list of built-in commands.\n\n");
#endif
		setup_job_control();
	}

	यदि (argv[optind]==रिक्त) {
		opt=parse_file_outer(stdin);
		गच्छ final_return;
	}

	debug_printf("\nrunning script '%s'\n", argv[optind]);
	global_argv = argv+optind;
	global_argc = argc-optind;
	निवेश = xfopen(argv[optind], "r");
	opt = parse_file_outer(निवेश);

#ifdef CONFIG_FEATURE_CLEAN_UP
	fclose(निवेश);
	यदि (cwd && cwd != unknown)
		मुक्त((वर्ण*)cwd);
	{
		संरचना variables *cur, *tmp;
		प्रति(cur = शीर्ष_vars; cur; cur = tmp) {
			tmp = cur->next;
			यदि (!cur->flg_read_only) {
				मुक्त(cur->name);
				मुक्त(cur->मूल्य);
				मुक्त(cur);
			}
		}
	}
#endif

final_return:
	प्रत्यागम(opt?opt:last_return_code);
}
#endif

स्थिर वर्ण *जोड़ो_var_मूल्य(वर्ण *inp)
{
	प्रत्यागम जोड़ो_var_मूल्य_sub(inp, 0);
}

स्थिर वर्ण *जोड़ो_var_मूल्य_sub(वर्ण *inp, पूर्णाङ्क tag_subst)
{
	पूर्णाङ्क res_str_len = 0;
	पूर्णाङ्क len;
	पूर्णाङ्क done = 0;
	वर्ण *p, *p1, *res_str = रिक्त;

	यावत् ((p = strchr(inp, SPECIAL_VAR_प्रतीक))) {
		/* check the beginning of the string for normal characters */
		यदि (p != inp) {
			/* copy any characters to the result string */
			len = p - inp;
			res_str = xrealloc(res_str, (res_str_len + len));
			strncpy((res_str + res_str_len), inp, len);
			res_str_len += len;
		}
		inp = ++p;
		/* find the ending marker */
		p = strchr(inp, SPECIAL_VAR_प्रतीक);
		*p = '\0';
		/* look up the value to substitute */
		यदि ((p1 = lookup_param(inp))) {
			यदि (tag_subst)
				len = res_str_len + strlen(p1) + 2;
			अन्यथा
				len = res_str_len + strlen(p1);
			res_str = xrealloc(res_str, (1 + len));
			यदि (tag_subst) {
				/*
				 * copy the variable value to the result
				 * string
				 */
				strcpy((res_str + res_str_len + 1), p1);

				/*
				 * mark the replaced text to be accepted as
				 * is
				 */
				res_str[res_str_len] = SUBSTED_VAR_प्रतीक;
				res_str[res_str_len + 1 + strlen(p1)] =
					SUBSTED_VAR_प्रतीक;
			} अन्यथा
				/*
				 * copy the variable value to the result
				 * string
				 */
				strcpy((res_str + res_str_len), p1);

			res_str_len = len;
		}
		*p = SPECIAL_VAR_प्रतीक;
		inp = ++p;
		done = 1;
	}
	यदि (done) {
		res_str = xrealloc(res_str, (1 + res_str_len + strlen(inp)));
		strcpy((res_str + res_str_len), inp);
		यावत् ((p = strchr(res_str, '\n'))) {
			*p = ' ';
		}
	}
	प्रत्यागम (res_str == रिक्त) ? inp : res_str;
}

स्थिर वर्ण **make_list_in(वर्ण **inp, वर्ण *name)
{
	पूर्णाङ्क len, i;
	पूर्णाङ्क name_len = strlen(name);
	पूर्णाङ्क n = 0;
	वर्ण **list;
	वर्ण *p1, *p2, *p3;

	/* create list of variable values */
	list = xmalloc(परिमाण(*list));
	प्रति (i = 0; inp[i]; i++) {
		p3 = जोड़ो_var_मूल्य(inp[i]);
		p1 = p3;
		यावत् (*p1) {
			यदि (*p1 == ' ') {
				p1++;
				अनुवर्त;
			}
			यदि ((p2 = strchr(p1, ' '))) {
				len = p2 - p1;
			} अन्यथा {
				len = strlen(p1);
				p2 = p1 + len;
			}
			/* we use n + 2 in realloc for list,because we add
			 * new element and then we will add NULL element */
			list = xrealloc(list, परिमाण(*list) * (n + 2));
			list[n] = xmalloc(2 + name_len + len);
			strcpy(list[n], name);
			strcat(list[n], "=");
			strncat(list[n], p1, len);
			list[n++][name_len + len + 1] = '\0';
			p1 = p2;
		}
		यदि (p3 != inp[i]) मुक्त(p3);
	}
	list[n] = रिक्त;
	प्रत्यागम list;
}

/*
 * Make new string for parser
 * inp     - array of argument strings to flatten
 * nonnull - indicates argument was quoted when originally parsed
 */
स्थिर वर्ण *make_string(वर्ण **inp, पूर्णाङ्क *nonnull)
{
	वर्ण *p;
	वर्ण *str = रिक्त;
	पूर्णाङ्क n;
	पूर्णाङ्क len = 2;
	वर्ण *noeval_str;
	पूर्णाङ्क noeval = 0;

	noeval_str = get_local_var("HUSH_NO_EVAL");
	यदि (noeval_str != रिक्त && *noeval_str != '0' && *noeval_str != '\0')
		noeval = 1;
	प्रति (n = 0; inp[n]; n++) {
		p = जोड़ो_var_मूल्य_sub(inp[n], noeval);
		वर्ण *new_str = xrealloc(str, (len + strlen(p) + (2 * nonnull[n])));
		यदि (!new_str) {
			मुक्त(str);
			यदि (p != inp[n]) मुक्त(p);
			प्रत्यागम रिक्त;
		}
		str = new_str;
		यदि (n) {
			strcat(str, " ");
		} अन्यथा {
			*str = '\0';
		}
		यदि (nonnull[n])
			strcat(str, "'");
		strcat(str, p);
		यदि (nonnull[n])
			strcat(str, "'");
		len = strlen(str) + 3;
		यदि (p != inp[n]) मुक्त(p);
	}
	len = strlen(str);
	*(str + len) = '\n';
	*(str + len + 1) = '\0';
	प्रत्यागम str;
}

#ifdef __U_BOOT__
स्थिर पूर्णाङ्क do_showvar(संरचना cmd_tbl *cmdtp, पूर्णाङ्क flag, पूर्णाङ्क argc,
		      वर्ण *ध्रुव argv[])
{
	पूर्णाङ्क i, k;
	पूर्णाङ्क rcode = 0;
	संरचना variables *cur;

	यदि (argc == 1) {		/* Print all env variables	*/
		प्रति (cur = शीर्ष_vars; cur; cur = cur->next) {
			printf ("%s=%s\n", cur->name, cur->मूल्य);
			यदि (ctrlc ()) {
				puts ("\n ** Abort\n");
				प्रत्यागम 1;
			}
		}
		प्रत्यागम 0;
	}
	प्रति (i = 1; i < argc; ++i) {	/* print single env variables	*/
		वर्ण *name = argv[i];

		k = -1;
		प्रति (cur = शीर्ष_vars; cur; cur = cur->next) {
			यदि(strcmp (cur->name, name) == 0) {
				k = 0;
				printf ("%s=%s\n", cur->name, cur->मूल्य);
			}
			यदि (ctrlc ()) {
				puts ("\n ** Abort\n");
				प्रत्यागम 1;
			}
		}
		यदि (k < 0) {
			printf ("## Error: \"%s\" not defined\n", name);
			rcode ++;
		}
	}
	प्रत्यागम rcode;
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
