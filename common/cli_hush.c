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
#ضمن <env.h>
#ضمن <malloc.h>         /* malloc, free, realloc*/
#ضمن <linux/ctype.h>    /* isalpha, isdigit */
#ضمن <console.h>
#ضمن <bootretry.h>
#ضمن <cli.h>
#ضمن <cli_hush.h>
#ضمن <command.h>        /* find_cmd */
#ضمن <vsprintf.h>
#endif
#ifndef __U_BOOT__
#ضمن <ctype.h>     /* isalpha, isdigit */
#ضمن <unistd.h>    /* getpid */
#ضمن <stdlib.h>    /* getenv, atoi */
#ضمن <string.h>    /* strchr */
#ضمن <stdio.h>     /* popen etc. */
#ضمن <glob.h>      /* glob, of course */
#ضمن <stdarg.h>    /* va_list */
#ضمن <errno.h>
#ضمن <fcntl.h>
#ضمن <getopt.h>    /* should be pretty obvious */

#ضمن <sys/stat.h>  /* ulimit */
#ضمن <sys/types.h>
#ضمن <sys/wait.h>
#ضمن <signal.h>

/* #include <dmalloc.h> */

#إذا 1
#ضمن "busybox.h"
#ضمن "cmdedit.h"
#وإلا
#define applet_name "hush"
#ضمن "standalone.h"
#define hush_main main
#define BB_BANNER
#endif
#endif
#define SPECIAL_VAR_سيمبول 03
#define SUBSTED_VAR_سيمبول 04
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
#define error_ڤسنن printf
#وإلا
تعريفنوع تعداد {
	REDIRECT_ماسوقن     = 1,
	REDIRECT_OVERWRITE = 2,
	REDIRECT_APPEND    = 3,
	REDIRECT_HEREIS    = 4,
	REDIRECT_IO        = 5
} redir_type;

/* The descrip member of this structure is only used to make debugging
 * output pretty */
بنية {صحيح mode; صحيح default_fd; حرف *descrip;} redir_table[] = {
	{ 0,                         0, "()" },
	{ O_RDONLY,                  0, "<"  },
	{ O_CREAT|O_TRUNC|O_WRONLY,  1, ">"  },
	{ O_CREAT|O_APPEND|O_WRONLY, 1, ">>" },
	{ O_RDONLY,                 -1, "<<" },
	{ O_RDWR,                    1, "<>" }
};
#endif

تعريفنوع تعداد {
	PIPE_SEQ = 1,
	PIPE_AND = 2,
	PIPE_OR  = 3,
	PIPE_BG  = 4,
} pipe_style;

/* might eventually control execution */
تعريفنوع تعداد {
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
بنية p_context {
	بنية child_prog *child;
	بنية pipe *list_head;
	بنية pipe *pipe;
#ifndef __U_BOOT__
	بنية redir_struct *pending_redirect;
#endif
	reserved_style w;
	صحيح old_flag;				/* for figuring out valid reserved words */
	بنية p_context *تيندنن;
	صحيح type;			/* define type of parser : ";$" common or special symbol */
	/* How about quoting status? */
};

#ifndef __U_BOOT__
بنية redir_struct {
	redir_type type;			/* type of redirection */
	صحيح fd;						/* file descriptor being redirected */
	صحيح dup;					/* -1, or file descriptor being duplicated */
	بنية redir_struct *next;	/* pointer to the next redirect in the list */
	glob_t word;				/* *word.gl_pathv is the filename */
};
#endif

بنية child_prog {
#ifndef __U_BOOT__
	pid_t pid;					/* 0 if exited */
#endif
	حرف **argv;				/* program name and arguments */
	/* was quoted when parsed; copy of struct o_string.nonnull field */
	صحيح *argv_nonnull;
#ifdef __U_BOOT__
	صحيح    argc;                            /* number of program arguments */
#endif
	بنية pipe *group;			/* if non-NULL, first in group or subshell */
#ifndef __U_BOOT__
	صحيح subshell;				/* flag, non-zero if group must be forked */
	بنية redir_struct *redirects;	/* I/O redirections */
	glob_t glob_result;			/* result of parameter globbing */
	صحيح is_stopped;				/* is the program currently running? */
	بنية pipe *family;		/* pointer back to the child's parent pipe */
#endif
	صحيح sp;				/* number of SPECIAL_VAR_SYMBOL */
	صحيح type;
};

بنية pipe {
#ifndef __U_BOOT__
	صحيح jobid;					/* job number */
#endif
	صحيح اڠک_progs;				/* total number of programs in job */
#ifndef __U_BOOT__
	صحيح running_progs;			/* number of programs running */
	حرف *text;					/* name of job */
	حرف *cmdbuf;				/* buffer various argv's point into */
	pid_t pgrp;					/* process group ID for the job */
#endif
	بنية child_prog *progs;	/* array of commands in pipe */
	بنية pipe *next;			/* to track background commands */
#ifndef __U_BOOT__
	صحيح stopped_progs;			/* number of programs alive, but stopped */
	صحيح job_context;			/* bitmask defining current context */
#endif
	pipe_style followup;		/* PIPE_BG, PIPE_SEQ, PIPE_OR, PIPE_AND */
	reserved_style r_mode;		/* supports if, for, while, until */
};

#ifndef __U_BOOT__
بنية close_me {
	صحيح fd;
	بنية close_me *next;
};
#endif

بنية variables {
	حرف *name;
	حرف *نيلاي;
	صحيح flg_export;
	صحيح flg_read_only;
	بنية variables *next;
};

/* globals, connect us to the outside world
 * the first three support $?, $#, and $1 */
#ifndef __U_BOOT__
حرف **global_argv;
unsigned صحيح global_argc;
#endif
ساكن unsigned صحيح last_return_code;
#ifndef __U_BOOT__
خارجي حرف **environ; /* This is in <unistd.h>, but protected with __USE_GNU */
#endif

/* "globals" within this file */
ساكن uchar *ifs;
ساكن حرف map[256];
#ifndef __U_BOOT__
ساكن صحيح fake_mode;
ساكن صحيح interactive;
ساكن بنية close_me *close_me_head;
ساكن ثابت حرف *cwd;
ساكن بنية pipe *job_list;
ساكن unsigned صحيح last_bg_pid;
ساكن unsigned صحيح last_jobid;
ساكن unsigned صحيح shell_terminal;
ساكن حرف *PS1;
ساكن حرف *PS2;
بنية variables shell_ver = { "HUSH_VERSION", "0.01", 1, 1, 0 };
بنية variables *اتس_vars = &shell_ver;
#وإلا
ساكن صحيح flag_repeat = 0;
ساكن صحيح do_repeat = 0;
ساكن بنية variables *اتس_vars = عدم ;
#endif /*__U_BOOT__ */

#define B_CHUNK (100)
#define B_NOSPAC 1

تعريفنوع بنية {
	حرف *data;
	صحيح length;
	صحيح maxlen;
	صحيح quote;
	صحيح nonnull;
} o_string;
#define NULL_O_STRING {عدم,0,0,0,0}
/* used for initialization:
	o_string foo = NULL_O_STRING; */

/* I can almost use ordinary FILE *.  Is open_memstream() universally
 * available?  Where is it documented? */
بنية in_str {
	ثابت unsigned حرف *p;
#ifndef __U_BOOT__
	حرف peek_buf[2];
#endif
	صحيح __promptme;
	صحيح promptmode;
#ifndef __U_BOOT__
	FILE *file;
#endif
	صحيح (*get) (بنية in_str *);
	صحيح (*peek) (بنية in_str *);
};
#define b_getch(ماسوقن) ((ماسوقن)->get(ماسوقن))
#define b_peek(ماسوقن) ((ماسوقن)->peek(ماسوقن))

#ifndef __U_BOOT__
#define JOB_STATUS_FORMAT "[%d] %-22s %.40s\n"

بنية built_in_command {
	حرف *cmd;					/* name */
	حرف *descr;				/* description */
	صحيح (*function) (بنية child_prog *);	/* function ptr */
};
#endif

/* define DEBUG_SHELL for debugging output (obviously ;-)) */
#إذا 0
#define DEBUG_SHELL
#endif

/* This should be in utility.c */
#ifdef DEBUG_SHELL
#ifndef __U_BOOT__
ساكن فراغ debug_printf(ثابت حرف *format, ...)
{
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
}
#وإلا
#define debug_printf(fmt,args...)	printf (fmt ,##args)
#endif
#وإلا
ساكن مضمن فراغ debug_printf(ثابت حرف *format, ...) { }
#endif
#define final_printf debug_printf

#ifdef __U_BOOT__
ساكن فراغ syntax_err(فراغ) {
	 printf("syntax error\n");
}
#وإلا
ساكن فراغ __syntax(حرف *file, صحيح باريس) {
	error_ڤسنن("syntax error %s:%d", file, باريس);
}
#define syntax() __syntax(__FILE__, __LINE__)
#endif

#ifdef __U_BOOT__
ساكن فراغ *xmalloc(size_t size);
ساكن فراغ *xrealloc(فراغ *ptr, size_t size);
#وإلا
/* Index of subroutines: */
/*   function prototypes for builtins */
ساكن صحيح builtin_cd(بنية child_prog *child);
ساكن صحيح builtin_env(بنية child_prog *child);
ساكن صحيح builtin_eval(بنية child_prog *child);
ساكن صحيح builtin_exec(بنية child_prog *child);
ساكن صحيح builtin_exit(بنية child_prog *child);
ساكن صحيح builtin_export(بنية child_prog *child);
ساكن صحيح builtin_fg_bg(بنية child_prog *child);
ساكن صحيح builtin_help(بنية child_prog *child);
ساكن صحيح builtin_jobs(بنية child_prog *child);
ساكن صحيح builtin_pwd(بنية child_prog *child);
ساكن صحيح builtin_read(بنية child_prog *child);
ساكن صحيح builtin_set(بنية child_prog *child);
ساكن صحيح builtin_shift(بنية child_prog *child);
ساكن صحيح builtin_source(بنية child_prog *child);
ساكن صحيح builtin_umask(بنية child_prog *child);
ساكن صحيح builtin_unset(بنية child_prog *child);
ساكن صحيح builtin_not_written(بنية child_prog *child);
#endif
/*   o_string manipulation: */
ساكن صحيح b_check_space(o_string *o, صحيح len);
ساكن صحيح b_addchr(o_string *o, صحيح ch);
ساكن فراغ b_reset(o_string *o);
ساكن صحيح b_addqchr(o_string *o, صحيح ch, صحيح quote);
#ifndef __U_BOOT__
ساكن صحيح b_adduint(o_string *o, unsigned صحيح i);
#endif
/*  in_str manipulations: */
ساكن صحيح static_get(بنية in_str *i);
ساكن صحيح static_peek(بنية in_str *i);
ساكن صحيح file_get(بنية in_str *i);
ساكن صحيح file_peek(بنية in_str *i);
#ifndef __U_BOOT__
ساكن فراغ setup_file_in_str(بنية in_str *i, FILE *f);
#وإلا
ساكن فراغ setup_file_in_str(بنية in_str *i);
#endif
ساكن فراغ setup_string_in_str(بنية in_str *i, ثابت حرف *s);
#ifndef __U_BOOT__
/*  close_me manipulations: */
ساكن فراغ mark_open(صحيح fd);
ساكن فراغ mark_closed(صحيح fd);
ساكن فراغ close_all(فراغ);
#endif
/*  "run" the final data structures: */
ساكن حرف *indenter(صحيح i);
ساكن صحيح free_pipe_list(بنية pipe *head, صحيح indent);
ساكن صحيح free_pipe(بنية pipe *pi, صحيح indent);
/*  really run the final data structures: */
#ifndef __U_BOOT__
ساكن صحيح setup_redirects(بنية child_prog *prog, صحيح squirrel[]);
#endif
ساكن صحيح run_list_real(بنية pipe *pi);
#ifndef __U_BOOT__
ساكن فراغ pseudo_exec(بنية child_prog *child) __attribute__ ((noreturn));
#endif
ساكن صحيح run_pipe_real(بنية pipe *pi);
/*   extended glob support: */
#ifndef __U_BOOT__
ساكن صحيح globhack(ثابت حرف *src, صحيح flags, glob_t *pglob);
ساكن صحيح glob_needed(ثابت حرف *s);
ساكن صحيح xglob(o_string *dest, صحيح flags, glob_t *pglob);
#endif
/*   variable assignment: */
ساكن صحيح is_assignment(ثابت حرف *s);
/*   data structure manipulation: */
#ifndef __U_BOOT__
ساكن صحيح setup_redirect(بنية p_context *ctx, صحيح fd, redir_type style, بنية in_str *ماسوقن);
#endif
ساكن فراغ initialize_context(بنية p_context *ctx);
ساكن صحيح done_word(o_string *dest, بنية p_context *ctx);
ساكن صحيح done_command(بنية p_context *ctx);
ساكن صحيح done_pipe(بنية p_context *ctx, pipe_style type);
/*   primary string parsing: */
#ifndef __U_BOOT__
ساكن صحيح redirect_dup_اڠک(بنية in_str *ماسوقن);
ساكن صحيح redirect_opt_اڠک(o_string *o);
ساكن صحيح process_command_subs(o_string *dest, بنية p_context *ctx, بنية in_str *ماسوقن, صحيح subst_end);
ساكن صحيح parse_group(o_string *dest, بنية p_context *ctx, بنية in_str *ماسوقن, صحيح ch);
#endif
ساكن حرف *lookup_param(حرف *src);
ساكن حرف *make_string(حرف **inp, صحيح *nonnull);
ساكن صحيح handle_dollar(o_string *dest, بنية p_context *ctx, بنية in_str *ماسوقن);
#ifndef __U_BOOT__
ساكن صحيح parse_string(o_string *dest, بنية p_context *ctx, ثابت حرف *src);
#endif
ساكن صحيح parse_stream(o_string *dest, بنية p_context *ctx, بنية in_str *input0, صحيح end_trigger);
/*   setup: */
ساكن صحيح parse_stream_outer(بنية in_str *inp, صحيح flag);
#ifndef __U_BOOT__
ساكن صحيح parse_string_outer(ثابت حرف *s, صحيح flag);
ساكن صحيح parse_file_outer(FILE *f);
#endif
#ifndef __U_BOOT__
/*   job management: */
ساكن صحيح checkjobs(بنية pipe* fg_pipe);
ساكن فراغ سيسيڤ_bg_job(بنية pipe *pi);
ساكن فراغ remove_bg_job(بنية pipe *pi);
#endif
/*     local variable support */
ساكن حرف **make_list_in(حرف **inp, حرف *name);
ساكن حرف *سيسيڤ_var_نيلاي(حرف *inp);
ساكن حرف *سيسيڤ_var_نيلاي_sub(حرف *inp, صحيح tag_subst);

#ifndef __U_BOOT__
/* Table of built-in functions.  They can be forked or not, depending on
 * context: within pipes, they fork.  As simple commands, they do not.
 * When used in non-forking context, they can change global variables
 * in the parent shell process.  If forked, of course they can not.
 * For example, 'unset foo | whatever' will parse and run, but foo will
 * still be set at the end. */
ساكن بنية built_in_command bltins[] = {
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
	{عدم, عدم, عدم}
};

ساكن ثابت حرف *set_cwd(فراغ)
{
	إذا(cwd==unknown)
		cwd = عدم;     /* xgetcwd(arg) called free(arg) */
	cwd = xgetcwd((حرف *)cwd);
	إذا (!cwd)
		cwd = unknown;
	أرجع cwd;
}

/* built-in 'eval' handler */
ساكن صحيح builtin_eval(بنية child_prog *child)
{
	حرف *str = عدم;
	صحيح rcode = EXIT_SUCCESS;

	إذا (child->argv[1]) {
		str = make_string(child->argv + 1);
		parse_string_outer(str, FLAG_EXIT_FROM_LOOP |
					FLAG_PARSE_SEMICOLON);
		حرر(str);
		rcode = last_return_code;
	}
	أرجع rcode;
}

/* built-in 'cd <path>' handler */
ساكن صحيح builtin_cd(بنية child_prog *child)
{
	حرف *newdir;
	إذا (child->argv[1] == عدم)
		newdir = env_get("HOME");
	وإلا
		newdir = child->argv[1];
	إذا (chdir(newdir)) {
		printf("cd: %s: %s\n", newdir, strerror(errno));
		أرجع EXIT_FAILURE;
	}
	set_cwd();
	أرجع EXIT_SUCCESS;
}

/* built-in 'env' handler */
ساكن صحيح builtin_env(بنية child_prog *dummy)
{
	حرف **e = environ;
	إذا (e == عدم) أرجع EXIT_FAILURE;
	لكل (; *e; e++) {
		puts(*e);
	}
	أرجع EXIT_SUCCESS;
}

/* built-in 'exec' handler */
ساكن صحيح builtin_exec(بنية child_prog *child)
{
	إذا (child->argv[1] == عدم)
		أرجع EXIT_SUCCESS;   /* Really? */
	child->argv++;
	pseudo_exec(child);
	/* never returns */
}

/* built-in 'exit' handler */
ساكن صحيح builtin_exit(بنية child_prog *child)
{
	إذا (child->argv[1] == عدم)
		exit(last_return_code);
	exit (atoi(child->argv[1]));
}

/* built-in 'export VAR=value' handler */
ساكن صحيح builtin_export(بنية child_prog *child)
{
	صحيح res = 0;
	حرف *name = child->argv[1];

	إذا (name == عدم) {
		أرجع (builtin_env(child));
	}

	name = strdup(name);

	إذا(name) {
		حرف *نيلاي = strchr(name, '=');

		إذا (!نيلاي) {
			حرف *tmp;
			/* They are exporting something without an =VALUE */

			نيلاي = get_local_var(name);
			إذا (نيلاي) {
				size_t ln = strlen(name);

				tmp = realloc(name, ln+strlen(نيلاي)+2);
				إذا(tmp==عدم)
					res = -1;
				وإلا {
					sprintf(tmp+ln, "=%s", نيلاي);
					name = tmp;
				}
			} وإلا {
				/* bash does not return an error when trying to export
				 * an undefined variable.  Do likewise. */
				res = 1;
			}
		}
	}
	إذا (res<0)
		perror_ڤسنن("export");
	وإلا إذا(res==0)
		res = set_local_var(name, 1);
	وإلا
		res = 0;
	حرر(name);
	أرجع res;
}

/* built-in 'fg' and 'bg' handler */
ساكن صحيح builtin_fg_bg(بنية child_prog *child)
{
	صحيح i, jobnum;
	بنية pipe *pi=عدم;

	إذا (!interactive)
		أرجع EXIT_FAILURE;
	/* If they gave us no args, assume they want the last backgrounded task */
	إذا (!child->argv[1]) {
		لكل (pi = job_list; pi; pi = pi->next) {
			إذا (pi->jobid == last_jobid) {
				اكسر;
			}
		}
		إذا (!pi) {
			error_ڤسنن("%s: no current job", child->argv[0]);
			أرجع EXIT_FAILURE;
		}
	} وإلا {
		إذا (sscanf(child->argv[1], "%%%d", &jobnum) != 1) {
			error_ڤسنن("%s: bad argument '%s'", child->argv[0], child->argv[1]);
			أرجع EXIT_FAILURE;
		}
		لكل (pi = job_list; pi; pi = pi->next) {
			إذا (pi->jobid == jobnum) {
				اكسر;
			}
		}
		إذا (!pi) {
			error_ڤسنن("%s: %d: no such job", child->argv[0], jobnum);
			أرجع EXIT_FAILURE;
		}
	}

	إذا (*child->argv[0] == 'f') {
		/* Put the job into the foreground.  */
		tcsetpgrp(shell_terminal, pi->pgrp);
	}

	/* Restart the processes in the job */
	لكل (i = 0; i < pi->اڠک_progs; i++)
		pi->progs[i].is_stopped = 0;

	إذا ( (i=kill(- pi->pgrp, SIGCONT)) < 0) {
		إذا (i == ESRCH) {
			remove_bg_job(pi);
		} وإلا {
			perror_ڤسنن("kill (SIGCONT)");
		}
	}

	pi->stopped_progs = 0;
	أرجع EXIT_SUCCESS;
}

/* built-in 'help' handler */
ساكن صحيح builtin_help(بنية child_prog *dummy)
{
	بنية built_in_command *x;

	printf("\nBuilt-in commands:\n");
	printf("-------------------\n");
	لكل (x = bltins; x->cmd; x++) {
		إذا (x->descr==عدم)
			استمر;
		printf("%s\t%s\n", x->cmd, x->descr);
	}
	printf("\n\n");
	أرجع EXIT_SUCCESS;
}

/* built-in 'jobs' handler */
ساكن صحيح builtin_jobs(بنية child_prog *child)
{
	بنية pipe *job;
	حرف *status_string;

	لكل (job = job_list; job; job = job->next) {
		إذا (job->running_progs == job->stopped_progs)
			status_string = "Stopped";
		وإلا
			status_string = "Running";

		printf(JOB_STATUS_FORMAT, job->jobid, status_string, job->text);
	}
	أرجع EXIT_SUCCESS;
}

/* built-in 'pwd' handler */
ساكن صحيح builtin_pwd(بنية child_prog *dummy)
{
	puts(set_cwd());
	أرجع EXIT_SUCCESS;
}

/* built-in 'read VAR' handler */
ساكن صحيح builtin_read(بنية child_prog *child)
{
	صحيح res;

	إذا (child->argv[1]) {
		حرف string[BUFSIZ];
		حرف *var = 0;

		string[0] = 0;  /* In case stdin has only EOF */
		/* read string */
		fgets(string, حجم(string), stdin);
		chomp(string);
		var = malloc(strlen(child->argv[1])+strlen(string)+2);
		إذا(var) {
			sprintf(var, "%s=%s", child->argv[1], string);
			res = set_local_var(var, 0);
		} وإلا
			res = -1;
		إذا (res)
			fprintf(stderr, "read: %m\n");
		حرر(var);      /* So not move up to avoid breaking errno */
		أرجع res;
	} وإلا {
		افعل res=getchar(); طالما(res!='\n' && res!=EOF);
		أرجع 0;
	}
}

/* built-in 'set VAR=value' handler */
ساكن صحيح builtin_set(بنية child_prog *child)
{
	حرف *temp = child->argv[1];
	بنية variables *e;

	إذا (temp == عدم)
		لكل(e = اتس_vars; e; e=e->next)
			printf("%s=%s\n", e->name, e->نيلاي);
	وإلا
		set_local_var(temp, 0);

		أرجع EXIT_SUCCESS;
}

/* Built-in 'shift' handler */
ساكن صحيح builtin_shift(بنية child_prog *child)
{
	صحيح n=1;
	إذا (child->argv[1]) {
		n=atoi(child->argv[1]);
	}
	إذا (n>=0 && n<global_argc) {
		/* XXX This probably breaks $0 */
		global_argc -= n;
		global_argv += n;
		أرجع EXIT_SUCCESS;
	} وإلا {
		أرجع EXIT_FAILURE;
	}
}

/* Built-in '.' handler (read-in and execute commands from file) */
ساكن صحيح builtin_source(بنية child_prog *child)
{
	FILE *ماسوقن;
	صحيح status;

	إذا (child->argv[1] == عدم)
		أرجع EXIT_FAILURE;

	/* XXX search through $PATH is missing */
	ماسوقن = fopen(child->argv[1], "r");
	إذا (!ماسوقن) {
		error_ڤسنن("Couldn't open file '%s'", child->argv[1]);
		أرجع EXIT_FAILURE;
	}

	/* Now run the file */
	/* XXX argv and argc are broken; need to save old global_argv
	 * (pointer only is OK!) on this stack frame,
	 * set global_argv=child->argv+1, recurse, and restore. */
	mark_open(fileno(ماسوقن));
	status = parse_file_outer(ماسوقن);
	mark_closed(fileno(ماسوقن));
	fclose(ماسوقن);
	أرجع (status);
}

ساكن صحيح builtin_umask(بنية child_prog *child)
{
	mode_t new_umask;
	ثابت حرف *arg = child->argv[1];
	حرف *end;
	إذا (arg) {
		new_umask=strtoul(arg, &end, 8);
		إذا (*end!='\0' || end == arg) {
			أرجع EXIT_FAILURE;
		}
	} وإلا {
		printf("%.3o\n", (unsigned صحيح) (new_umask=umask(0)));
	}
	umask(new_umask);
	أرجع EXIT_SUCCESS;
}

/* built-in 'unset VAR' handler */
ساكن صحيح builtin_unset(بنية child_prog *child)
{
	/* bash returned already true */
	unset_local_var(child->argv[1]);
	أرجع EXIT_SUCCESS;
}

ساكن صحيح builtin_not_written(بنية child_prog *child)
{
	printf("builtin_%s not written\n",child->argv[0]);
	أرجع EXIT_FAILURE;
}
#endif

ساكن صحيح b_check_space(o_string *o, صحيح len)
{
	/* It would be easy to drop a more restrictive policy
	 * in here, such as setting a maximum string length */
	إذا (o->length + len > o->maxlen) {
		حرف *old_data = o->data;
		/* assert (data == NULL || o->maxlen != 0); */
		o->maxlen += max(2*len, B_CHUNK);
		o->data = realloc(o->data, 1 + o->maxlen);
		إذا (o->data == عدم) {
			حرر(old_data);
		}
	}
	أرجع o->data == عدم;
}

ساكن صحيح b_addchr(o_string *o, صحيح ch)
{
	debug_printf("b_addchr: %c %d %p\n", ch, o->length, o);
	إذا (b_check_space(o, 1)) أرجع B_NOSPAC;
	o->data[o->length] = ch;
	o->length++;
	o->data[o->length] = '\0';
	أرجع 0;
}

ساكن فراغ b_reset(o_string *o)
{
	o->length = 0;
	o->nonnull = 0;
	إذا (o->data != عدم) *o->data = '\0';
}

ساكن فراغ b_free(o_string *o)
{
	b_reset(o);
	حرر(o->data);
	o->data = عدم;
	o->maxlen = 0;
}

/* My analysis of quoting semantics tells me that state information
 * is associated with a destination, not a source.
 */
ساكن صحيح b_addqchr(o_string *o, صحيح ch, صحيح quote)
{
	إذا (quote && strchr("*?[\\",ch)) {
		صحيح rc;
		rc = b_addchr(o, '\\');
		إذا (rc) أرجع rc;
	}
	أرجع b_addchr(o, ch);
}

#ifndef __U_BOOT__
ساكن صحيح b_adduint(o_string *o, unsigned صحيح i)
{
	صحيح r;
	حرف *p = simple_itoa(i);
	/* no escape checking necessary */
	افعل r=b_addchr(o, *p++); طالما (r==0 && *p);
	أرجع r;
}
#endif

ساكن صحيح static_get(بنية in_str *i)
{
	صحيح ch = *i->p++;
	إذا (ch=='\0') أرجع EOF;
	أرجع ch;
}

ساكن صحيح static_peek(بنية in_str *i)
{
	أرجع *i->p;
}

#ifndef __U_BOOT__
ساكن مضمن فراغ cmdedit_set_initial_prompt(فراغ)
{
#ifndef CONFIG_FEATURE_SH_FANCY_PROMPT
	PS1 = عدم;
#وإلا
	PS1 = env_get("PS1");
	إذا(PS1==0)
		PS1 = "\\w \\$ ";
#endif
}

ساكن مضمن فراغ setup_prompt_string(صحيح promptmode, حرف **prompt_str)
{
	debug_printf("setup_prompt_string %d ",promptmode);
#ifndef CONFIG_FEATURE_SH_FANCY_PROMPT
	/* Set up the prompt */
	إذا (promptmode == 1) {
		حرر(PS1);
		PS1=xmalloc(strlen(cwd)+4);
		sprintf(PS1, "%s %s", cwd, ( geteuid() != 0 ) ?  "$ ":"# ");
		*prompt_str = PS1;
	} وإلا {
		*prompt_str = PS2;
	}
#وإلا
	*prompt_str = (promptmode==1)? PS1 : PS2;
#endif
	debug_printf("result %s\n",*prompt_str);
}
#endif

#ifdef __U_BOOT__
ساكن صحيح uboot_cli_readline(بنية in_str *i)
{
	حرف *prompt;
	حرف __maybe_unused *ps_prompt = عدم;

	إذا (i->promptmode == 1)
		prompt = CONFIG_SYS_PROMPT;
	وإلا
		prompt = CONFIG_SYS_PROMPT_HUSH_PS2;

#ifdef CONFIG_CMDLINE_PS_SUPPORT
	إذا (i->promptmode == 1)
		ps_prompt = env_get("PS1");
	وإلا
		ps_prompt = env_get("PS2");
	إذا (ps_prompt)
		prompt = ps_prompt;
#endif

	أرجع cli_readline(prompt);
}
#endif

ساكن فراغ get_user_ماسوقن(بنية in_str *i)
{
#ifndef __U_BOOT__
	حرف *prompt_str;
	ساكن حرف the_command[BUFSIZ];

	setup_prompt_string(i->promptmode, &prompt_str);
#ifdef CONFIG_FEATURE_COMMAND_EDITING
	/*
	 ** enable command line editing only while a command line
	 ** is actually being read; otherwise, we'll end up bequeathing
	 ** atexit() handlers and other unwanted stuff to our
	 ** child processes (rob@sysgo.de)
	 */
	cmdedit_read_ماسوقن(prompt_str, the_command);
#وإلا
	fputs(prompt_str, stdout);
	fflush(stdout);
	the_command[0]=fgetc(i->file);
	the_command[1]='\0';
#endif
	fflush(stdout);
	i->p = the_command;
#وإلا
	صحيح n;
	ساكن حرف the_command[CONFIG_SYS_CBSIZE + 1];

	bootretry_reset_cmd_timeout();
	i->__promptme = 1;
	n = uboot_cli_readline(i);

#ifdef CONFIG_BOOT_RETRY_TIME
	إذا (n == -2) {
	  puts("\nTimeout waiting for command\n");
#  ifdef CONFIG_RESET_TO_RETRY
	  do_reset(عدم, 0, 0, عدم);
#  elif IS_ENABLED(CONFIG_RETRY_BOOTCMD)
	strcpy(console_buffer, "run bootcmd\n");
# وإلا
#	error "This only works with CONFIG_RESET_TO_RETRY or CONFIG_BOOT_RETRY_COMMAND enabled"
#  endif
	}
#endif
	إذا (n == -1 ) {
		flag_repeat = 0;
		i->__promptme = 0;
	}
	n = strlen(console_buffer);
	console_buffer[n] = '\n';
	console_buffer[n+1]= '\0';
	إذا (had_ctrlc()) flag_repeat = 0;
	clear_ctrlc();
	do_repeat = 0;
	إذا (i->promptmode == 1) {
		إذا (console_buffer[0] == '\n'&& flag_repeat == 0) {
			strcpy(the_command,console_buffer);
		}
		وإلا {
			إذا (console_buffer[0] != '\n') {
				strcpy(the_command,console_buffer);
				flag_repeat = 1;
			}
			وإلا {
				do_repeat = 1;
			}
		}
		i->p = the_command;
	}
	وإلا {
		إذا (console_buffer[0] != '\n') {
			إذا (strlen(the_command) + strlen(console_buffer)
			    < CONFIG_SYS_CBSIZE) {
				n = strlen(the_command);
				the_command[n-1] = ' ';
				strcpy(&the_command[n],console_buffer);
			}
			وإلا {
				the_command[0] = '\n';
				the_command[1] = '\0';
				flag_repeat = 0;
			}
		}
		إذا (i->__promptme == 0) {
			the_command[0] = '\n';
			the_command[1] = '\0';
		}
		i->p = console_buffer;
	}
#endif
}

/* This is the magic location that prints prompts
 * and gets data back from the user */
ساكن صحيح file_get(بنية in_str *i)
{
	صحيح ch;

	ch = 0;
	/* If there is data waiting, eat it up */
	إذا (i->p && *i->p) {
		ch = *i->p++;
	} وإلا {
		/* need to double check i->file because we might be doing something
		 * more complicated by now, like sourcing or substituting. */
#ifndef __U_BOOT__
		إذا (i->__promptme && interactive && i->file == stdin) {
			طالما(! i->p || (interactive && strlen(i->p)==0) ) {
#وإلا
			طالما(! i->p  || strlen(i->p)==0 ) {
#endif
				get_user_ماسوقن(i);
			}
			i->promptmode=2;
#ifndef __U_BOOT__
			i->__promptme = 0;
#endif
			إذا (i->p && *i->p) {
				ch = *i->p++;
			}
#ifndef __U_BOOT__
		} وإلا {
			ch = fgetc(i->file);
		}

#endif
		debug_printf("b_getch: got a %d\n", ch);
	}
#ifndef __U_BOOT__
	إذا (ch == '\n') i->__promptme=1;
#endif
	أرجع ch;
}

/* All the callers guarantee this routine will never be
 * used right after a newline, so prompting is not needed.
 */
ساكن صحيح file_peek(بنية in_str *i)
{
#ifndef __U_BOOT__
	إذا (i->p && *i->p) {
#endif
		أرجع *i->p;
#ifndef __U_BOOT__
	} وإلا {
		i->peek_buf[0] = fgetc(i->file);
		i->peek_buf[1] = '\0';
		i->p = i->peek_buf;
		debug_printf("b_peek: got a %d\n", *i->p);
		أرجع *i->p;
	}
#endif
}

#ifndef __U_BOOT__
ساكن فراغ setup_file_in_str(بنية in_str *i, FILE *f)
#وإلا
ساكن فراغ setup_file_in_str(بنية in_str *i)
#endif
{
	i->peek = file_peek;
	i->get = file_get;
	i->__promptme=1;
	i->promptmode=1;
#ifndef __U_BOOT__
	i->file = f;
#endif
	i->p = عدم;
}

ساكن فراغ setup_string_in_str(بنية in_str *i, ثابت حرف *s)
{
	i->peek = static_peek;
	i->get = static_get;
	i->__promptme=1;
	i->promptmode=1;
	i->p = s;
}

#ifndef __U_BOOT__
ساكن فراغ mark_open(صحيح fd)
{
	بنية close_me *new = xmalloc(حجم(بنية close_me));
	new->fd = fd;
	new->next = close_me_head;
	close_me_head = new;
}

ساكن فراغ mark_closed(صحيح fd)
{
	بنية close_me *tmp;
	إذا (close_me_head == عدم || close_me_head->fd != fd)
		error_ڤسنن_and_die("corrupt close_me");
	tmp = close_me_head;
	close_me_head = close_me_head->next;
	حرر(tmp);
}

ساكن فراغ close_all(فراغ)
{
	بنية close_me *c;
	لكل (c=close_me_head; c; c=c->next) {
		close(c->fd);
	}
	close_me_head = عدم;
}

/* squirrel != NULL means we squirrel away copies of stdin, stdout,
 * and stderr if they are redirected. */
ساكن صحيح setup_redirects(بنية child_prog *prog, صحيح squirrel[])
{
	صحيح openfd, mode;
	بنية redir_struct *redir;

	لكل (redir=prog->redirects; redir; redir=redir->next) {
		إذا (redir->dup == -1 && redir->word.gl_pathv == عدم) {
			/* something went wrong in the parse.  Pretend it didn't happen */
			استمر;
		}
		إذا (redir->dup == -1) {
			mode=redir_table[redir->type].mode;
			openfd = open(redir->word.gl_pathv[0], mode, 0666);
			إذا (openfd < 0) {
			/* this could get lost if stderr has been redirected, but
			   bash and ash both lose it as well (though zsh doesn't!) */
				perror_ڤسنن("error opening %s", redir->word.gl_pathv[0]);
				أرجع 1;
			}
		} وإلا {
			openfd = redir->dup;
		}

		إذا (openfd != redir->fd) {
			إذا (squirrel && redir->fd < 3) {
				squirrel[redir->fd] = dup(redir->fd);
			}
			إذا (openfd == -3) {
				close(openfd);
			} وإلا {
				dup2(openfd, redir->fd);
				إذا (redir->dup == -1)
					close (openfd);
			}
		}
	}
	أرجع 0;
}

ساكن فراغ restore_redirects(صحيح squirrel[])
{
	صحيح i, fd;
	لكل (i=0; i<3; i++) {
		fd = squirrel[i];
		إذا (fd != -1) {
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
ساكن فراغ pseudo_exec(بنية child_prog *child)
{
	صحيح i, rcode;
	حرف *p;
	بنية built_in_command *x;
	إذا (child->argv) {
		لكل (i=0; is_assignment(child->argv[i]); i++) {
			debug_printf("pid %d environment modification: %s\n",getpid(),child->argv[i]);
			p = سيسيڤ_var_نيلاي(child->argv[i]);
			putenv(strdup(p));
			إذا (p != child->argv[i]) حرر(p);
		}
		child->argv+=i;  /* XXX this hack isn't so horrible, since we are about
					to exit, and therefore don't need to keep data
					structures consistent for free() use. */
		/* If a variable is assigned in a forest, and nobody listens,
		 * was it ever really set?
		 */
		إذا (child->argv[0] == عدم) {
			_exit(EXIT_SUCCESS);
		}

		/*
		 * Check if the command matches any of the builtins.
		 * Depending on context, this might be redundant.  But it's
		 * easier to waste a few CPU cycles than it is to figure out
		 * if this is one of those cases.
		 */
		لكل (x = bltins; x->cmd; x++) {
			إذا (strcmp(child->argv[0], x->cmd) == 0 ) {
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
			صحيح argc_l;
			حرف** argv_l=child->argv;
			حرف *name = child->argv[0];

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
			لكل(argc_l=0;*argv_l!=عدم; argv_l++, argc_l++);
			optind = 1;
			debug_printf("running applet %s\n", name);
			run_applet_by_name(name, argc_l, child->argv);
		}
#endif
		debug_printf("exec of %s\n",child->argv[0]);
		execvp(child->argv[0],child->argv);
		perror_ڤسنن("couldn't exec: %s",child->argv[0]);
		_exit(1);
	} وإلا إذا (child->group) {
		debug_printf("runtime nesting to group\n");
		interactive=0;    /* crucial!!!! */
		rcode = run_list_real(child->group);
		/* OK to leak memory by not calling free_pipe_list,
		 * since this process is about to exit */
		_exit(rcode);
	} وإلا {
		/* Can happen.  See what bash does with ">foo" by itself. */
		debug_printf("trying to pseudo_exec null command\n");
		_exit(EXIT_SUCCESS);
	}
}

ساكن فراغ سيسيڤ_bg_job(بنية pipe *pi)
{
	بنية pipe *thejob;

	/* Linear search for the ID of the job to use */
	pi->jobid = 1;
	لكل (thejob = job_list; thejob; thejob = thejob->next)
		إذا (thejob->jobid >= pi->jobid)
			pi->jobid = thejob->jobid + 1;

	/* add thejob to the list of running jobs */
	إذا (!job_list) {
		thejob = job_list = xmalloc(حجم(*thejob));
	} وإلا {
		لكل (thejob = job_list; thejob->next; thejob = thejob->next) /* nothing */;
		thejob->next = xmalloc(حجم(*thejob));
		thejob = thejob->next;
	}

	/* physically copy the struct job */
	memcpy(thejob, pi, حجم(بنية pipe));
	thejob->next = عدم;
	thejob->running_progs = thejob->اڠک_progs;
	thejob->stopped_progs = 0;
	thejob->text = xmalloc(BUFSIZ); /* cmdedit buffer size */

	/*if (pi->progs[0] && pi->progs[0].argv && pi->progs[0].argv[0]) */
	{
		حرف *bar=thejob->text;
		حرف **foo=pi->progs[0].argv;
		طالما(foo && *foo) {
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
ساكن فراغ remove_bg_job(بنية pipe *pi)
{
	بنية pipe *prev_pipe;

	إذا (pi == job_list) {
		job_list = pi->next;
	} وإلا {
		prev_pipe = job_list;
		طالما (prev_pipe->next != pi)
			prev_pipe = prev_pipe->next;
		prev_pipe->next = pi->next;
	}
	إذا (job_list)
		last_jobid = job_list->jobid;
	وإلا
		last_jobid = 0;

	pi->stopped_progs = 0;
	free_pipe(pi, 0);
	حرر(pi);
}

/* Checks to see if any processes have exited -- if they
   have, figure out why and see if a job has completed */
ساكن صحيح checkjobs(بنية pipe* fg_pipe)
{
	صحيح attributes;
	صحيح status;
	صحيح prognum = 0;
	بنية pipe *pi;
	pid_t childpid;

	attributes = WUNTRACED;
	إذا (fg_pipe==عدم) {
		attributes |= WNOHANG;
	}

	طالما ((childpid = waitpid(-1, &status, attributes)) > 0) {
		إذا (fg_pipe) {
			صحيح i, rcode = 0;
			لكل (i=0; i < fg_pipe->اڠک_progs; i++) {
				إذا (fg_pipe->progs[i].pid == childpid) {
					إذا (i==fg_pipe->اڠک_progs-1)
						rcode=WEXITSTATUS(status);
					(fg_pipe->اڠک_progs)--;
					أرجع(rcode);
				}
			}
		}

		لكل (pi = job_list; pi; pi = pi->next) {
			prognum = 0;
			طالما (prognum < pi->اڠک_progs && pi->progs[prognum].pid != childpid) {
				prognum++;
			}
			إذا (prognum < pi->اڠک_progs)
				اكسر;
		}

		إذا(pi==عدم) {
			debug_printf("checkjobs: pid %d was not in our list!\n", childpid);
			استمر;
		}

		إذا (WIFEXITED(status) || WIFSIGNALED(status)) {
			/* child exited */
			pi->running_progs--;
			pi->progs[prognum].pid = 0;

			إذا (!pi->running_progs) {
				printf(JOB_STATUS_FORMAT, pi->jobid, "Done", pi->text);
				remove_bg_job(pi);
			}
		} وإلا {
			/* child stopped */
			pi->stopped_progs++;
			pi->progs[prognum].is_stopped = 1;

#إذا 0
			/* Printing this stuff is a pain, since it tends to
			 * overwrite the prompt an inconveinient moments.  So
			 * don't do that.  */
			إذا (pi->stopped_progs == pi->اڠک_progs) {
				printf("\n"JOB_STATUS_FORMAT, pi->jobid, "Stopped", pi->text);
			}
#endif
		}
	}

	إذا (childpid == -1 && errno != ECHILD)
		perror_ڤسنن("waitpid");

	/* move the shell to the foreground */
	/*if (interactive && tcsetpgrp(shell_terminal, getpgid(0))) */
	/*	perror_msg("tcsetpgrp-2"); */
	أرجع -1;
}

/* Figure out our controlling tty, checking in order stderr,
 * stdin, and stdout.  If check_pgrp is set, also check that
 * we belong to the foreground process group associated with
 * that tty.  The value of shell_terminal is needed in order to call
 * tcsetpgrp(shell_terminal, ...); */
فراغ controlling_tty(صحيح check_pgrp)
{
	pid_t curpgrp;

	إذا ((curpgrp = tcgetpgrp(shell_terminal = 2)) < 0
			&& (curpgrp = tcgetpgrp(shell_terminal = 0)) < 0
			&& (curpgrp = tcgetpgrp(shell_terminal = 1)) < 0)
		اذهب shell_terminal_error;

	إذا (check_pgrp && curpgrp != getpgid(0))
		اذهب shell_terminal_error;

	أرجع;

shell_terminal_error:
		shell_terminal = -1;
		أرجع;
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
ساكن صحيح run_pipe_real(بنية pipe *pi)
{
	صحيح i;
#ifndef __U_BOOT__
	صحيح nextin, nextout;
	صحيح pipefds[2];				/* pipefds[0] is for reading */
	بنية child_prog *child;
	بنية built_in_command *x;
	حرف *p;
# إذا __GNUC__
	/* Avoid longjmp clobbering */
	(فراغ) &i;
	(فراغ) &nextin;
	(فراغ) &nextout;
	(فراغ) &child;
# endif
#وإلا
	صحيح nextin;
	صحيح flag = do_repeat ? CMD_FLAG_REPEAT : 0;
	بنية child_prog *child;
	حرف *p;
# إذا __GNUC__
	/* Avoid longjmp clobbering */
	(فراغ) &i;
	(فراغ) &nextin;
	(فراغ) &child;
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
	إذا (pi->اڠک_progs == 1) child = & (pi->progs[0]);
#ifndef __U_BOOT__
	إذا (pi->اڠک_progs == 1 && child->group && child->subshell == 0) {
		صحيح squirrel[] = {-1, -1, -1};
		صحيح rcode;
		debug_printf("non-subshell grouping\n");
		setup_redirects(child, squirrel);
		/* XXX could we merge code with following builtin case,
		 * by creating a pseudo builtin that calls run_list_real? */
		rcode = run_list_real(child->group);
		restore_redirects(squirrel);
#وإلا
		إذا (pi->اڠک_progs == 1 && child->group) {
		صحيح rcode;
		debug_printf("non-subshell grouping\n");
		rcode = run_list_real(child->group);
#endif
		أرجع rcode;
	} وإلا إذا (pi->اڠک_progs == 1 && pi->progs[0].argv != عدم) {
		لكل (i=0; is_assignment(child->argv[i]); i++) { /* nothing */ }
		إذا (i!=0 && child->argv[i]==عدم) {
			/* assignments, but no command: set the local environment */
			لكل (i=0; child->argv[i]!=عدم; i++) {

				/* Ok, this case is tricky.  We have to decide if this is a
				 * local variable, or an already exported variable.  If it is
				 * already exported, we have to export the new value.  If it is
				 * not exported, we need only set this as a local variable.
				 * This junk is all to decide whether or not to export this
				 * variable. */
				صحيح export_me=0;
				حرف *name, *نيلاي;
				name = xstrdup(child->argv[i]);
				debug_printf("Local environment set: %s\n", name);
				نيلاي = strchr(name, '=');
				إذا (نيلاي)
					*نيلاي=0;
#ifndef __U_BOOT__
				إذا ( get_local_var(name)) {
					export_me=1;
				}
#endif
				حرر(name);
				p = سيسيڤ_var_نيلاي(child->argv[i]);
				set_local_var(p, export_me);
				إذا (p != child->argv[i]) حرر(p);
			}
			أرجع EXIT_SUCCESS;   /* don't worry about errors in set_local_var() yet */
		}
		لكل (i = 0; is_assignment(child->argv[i]); i++) {
			p = سيسيڤ_var_نيلاي(child->argv[i]);
#ifndef __U_BOOT__
			putenv(strdup(p));
#وإلا
			set_local_var(p, 0);
#endif
			إذا (p != child->argv[i]) {
				child->sp--;
				حرر(p);
			}
		}
		إذا (child->sp) {
			حرف * str = عدم;

			str = make_string(child->argv + i,
					  child->argv_nonnull + i);
			parse_string_outer(str, FLAG_EXIT_FROM_LOOP | FLAG_REPARSING);
			حرر(str);
			أرجع last_return_code;
		}
#ifndef __U_BOOT__
		لكل (x = bltins; x->cmd; x++) {
			إذا (strcmp(child->argv[i], x->cmd) == 0 ) {
				صحيح squirrel[] = {-1, -1, -1};
				صحيح rcode;
				إذا (x->function == builtin_exec && child->argv[i+1]==عدم) {
					debug_printf("magic exec\n");
					setup_redirects(child,عدم);
					أرجع EXIT_SUCCESS;
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
			أرجع rcode;
		}
#وإلا
		/* check ";", because ,example , argv consist from
		 * "help;flinfo" must not execute
		 */
		إذا (strchr(child->argv[i], ';')) {
			printf("Unknown command '%s' - try 'help' or use "
					"'run' command\n", child->argv[i]);
			أرجع -1;
		}
		/* Process the command */
		أرجع cmd_process(flag, child->argc - i, child->argv + i,
				   &flag_repeat, عدم);
#endif
	}
#ifndef __U_BOOT__

	لكل (i = 0; i < pi->اڠک_progs; i++) {
		child = & (pi->progs[i]);

		/* pipes are inserted between pairs of commands */
		إذا ((i + 1) < pi->اڠک_progs) {
			إذا (pipe(pipefds)<0) perror_ڤسنن_and_die("pipe");
			nextout = pipefds[1];
		} وإلا {
			nextout=1;
			pipefds[0] = -1;
		}

		/* XXX test for failed fork()? */
		إذا (!(child->pid = fork())) {
			/* Set the handling for job control signals back to the default.  */
			إشارة(SIGINT, SIG_DFL);
			إشارة(SIGQUIT, SIG_DFL);
			إشارة(SIGTERM, SIG_DFL);
			إشارة(SIGTSTP, SIG_DFL);
			إشارة(SIGTTIN, SIG_DFL);
			إشارة(SIGTTOU, SIG_DFL);
			إشارة(SIGCHLD, SIG_DFL);

			close_all();

			إذا (nextin != 0) {
				dup2(nextin, 0);
				close(nextin);
			}
			إذا (nextout != 1) {
				dup2(nextout, 1);
				close(nextout);
			}
			إذا (pipefds[0]!=-1) {
				close(pipefds[0]);  /* opposite end of our output pipe */
			}

			/* Like bash, explicit redirects override pipes,
			 * and the pipe fd is available for dup'ing. */
			setup_redirects(child,عدم);

			إذا (interactive && pi->followup!=PIPE_BG) {
				/* If we (the child) win the race, put ourselves in the process
				 * group whose leader is the first process in this pipe. */
				إذا (pi->pgrp < 0) {
					pi->pgrp = getpid();
				}
				إذا (setpgid(0, pi->pgrp) == 0) {
					tcsetpgrp(2, pi->pgrp);
				}
			}

			pseudo_exec(child);
		}

		/* put our child in the process group whose leader is the
		   first process in this pipe */
		إذا (pi->pgrp < 0) {
			pi->pgrp = child->pid;
		}
		/* Don't check for errors.  The child may be dead already,
		 * in which case setpgid returns error code EACCES. */
		setpgid(child->pid, pi->pgrp);

		إذا (nextin != 0)
			close(nextin);
		إذا (nextout != 1)
			close(nextout);

		/* If there isn't another process, nextin is garbage
		   but it doesn't matter */
		nextin = pipefds[0];
	}
#endif
	أرجع -1;
}

ساكن صحيح run_list_real(بنية pipe *pi)
{
	حرف *save_name = عدم;
	حرف **list = عدم;
	حرف **save_list = عدم;
	بنية pipe *rpipe;
	صحيح flag_rep = 0;
#ifndef __U_BOOT__
	صحيح save_اڠک_progs;
#endif
	صحيح rcode=0, flag_skip=1;
	صحيح flag_restore = 0;
	صحيح if_code=0, next_if_code=0;  /* need double-buffer to handle elif */
	reserved_style rmode, skip_more_in_this_rmode=RES_XXXX;
	/* check syntax for "for" */
	لكل (rpipe = pi; rpipe; rpipe = rpipe->next) {
		إذا ((rpipe->r_mode == RES_IN ||
		    rpipe->r_mode == RES_FOR) &&
		    (rpipe->next == عدم)) {
				syntax();
#ifdef __U_BOOT__
				flag_repeat = 0;
#endif
				أرجع 1;
		}
		إذا ((rpipe->r_mode == RES_IN &&
			(rpipe->next->r_mode == RES_IN &&
			rpipe->next->progs->argv != عدم))||
			(rpipe->r_mode == RES_FOR &&
			rpipe->next->r_mode != RES_IN)) {
				syntax();
#ifdef __U_BOOT__
				flag_repeat = 0;
#endif
				أرجع 1;
		}
	}
	لكل (; pi; pi = (flag_restore != 0) ? rpipe : pi->next) {
		إذا (pi->r_mode == RES_WHILE || pi->r_mode == RES_UNTIL ||
			pi->r_mode == RES_FOR) {
#ifdef __U_BOOT__
				/* check Ctrl-C */
				ctrlc();
				إذا ((had_ctrlc())) {
					أرجع 1;
				}
#endif
				flag_restore = 0;
				إذا (!rpipe) {
					flag_rep = 0;
					rpipe = pi;
				}
		}
		rmode = pi->r_mode;
		debug_printf("rmode=%d  if_code=%d  next_if_code=%d skip_more=%d\n", rmode, if_code, next_if_code, skip_more_in_this_rmode);
		إذا (rmode == skip_more_in_this_rmode && flag_skip) {
			إذا (pi->followup == PIPE_SEQ) flag_skip=0;
			استمر;
		}
		flag_skip = 1;
		skip_more_in_this_rmode = RES_XXXX;
		إذا (rmode == RES_THEN || rmode == RES_ELSE) if_code = next_if_code;
		إذا (rmode == RES_THEN &&  if_code) استمر;
		إذا (rmode == RES_ELSE && !if_code) استمر;
		إذا (rmode == RES_ELIF && !if_code) اكسر;
		إذا (rmode == RES_FOR && pi->اڠک_progs) {
			إذا (!list) {
				/* if no variable values after "in" we skip "for" */
				إذا (!pi->next->progs->argv) استمر;
				/* create list of variable values */
				list = make_list_in(pi->next->progs->argv,
					pi->progs->argv[0]);
				save_list = list;
				save_name = pi->progs->argv[0];
				pi->progs->argv[0] = عدم;
				flag_rep = 1;
			}
			إذا (!(*list)) {
				حرر(pi->progs->argv[0]);
				حرر(save_list);
				list = عدم;
				flag_rep = 0;
				pi->progs->argv[0] = save_name;
#ifndef __U_BOOT__
				pi->progs->glob_result.gl_pathv[0] =
					pi->progs->argv[0];
#endif
				استمر;
			} وإلا {
				/* insert new value from list for variable */
				حرر(pi->progs->argv[0]);
				pi->progs->argv[0] = *list++;
#ifndef __U_BOOT__
				pi->progs->glob_result.gl_pathv[0] =
					pi->progs->argv[0];
#endif
			}
		}
		إذا (rmode == RES_IN) استمر;
		إذا (rmode == RES_DO) {
			إذا (!flag_rep) استمر;
		}
		إذا (rmode == RES_DONE) {
			إذا (flag_rep) {
				flag_restore = 1;
			} وإلا {
				rpipe = عدم;
			}
		}
		إذا (pi->اڠک_progs == 0) استمر;
#ifndef __U_BOOT__
		save_اڠک_progs = pi->اڠک_progs; /* save number of programs */
#endif
		rcode = run_pipe_real(pi);
		debug_printf("run_pipe_real returned %d\n",rcode);
#ifndef __U_BOOT__
		إذا (rcode!=-1) {
			/* We only ran a builtin: rcode was set by the return value
			 * of run_pipe_real(), and we don't need to wait for anything. */
		} وإلا إذا (pi->followup==PIPE_BG) {
			/* XXX check bash's behavior with nontrivial pipes */
			/* XXX compute jobid */
			/* XXX what does bash do with attempts to background builtins? */
			سيسيڤ_bg_job(pi);
			rcode = EXIT_SUCCESS;
		} وإلا {
			إذا (interactive) {
				/* move the new process group into the foreground */
				إذا (tcsetpgrp(shell_terminal, pi->pgrp) && errno != ENOTTY)
					perror_ڤسنن("tcsetpgrp-3");
				rcode = checkjobs(pi);
				/* move the shell to the foreground */
				إذا (tcsetpgrp(shell_terminal, getpgid(0)) && errno != ENOTTY)
					perror_ڤسنن("tcsetpgrp-4");
			} وإلا {
				rcode = checkjobs(pi);
			}
			debug_printf("checkjobs returned %d\n",rcode);
		}
		last_return_code=rcode;
#وإلا
		إذا (rcode < -1) {
			last_return_code = -rcode - 2;
			أرجع -2;	/* exit */
		}
		last_return_code = rcode;
#endif
#ifndef __U_BOOT__
		pi->اڠک_progs = save_اڠک_progs; /* restore number of programs */
#endif
		إذا ( rmode == RES_IF || rmode == RES_ELIF )
			next_if_code=rcode;  /* can be overwritten a number of times */
		إذا (rmode == RES_WHILE)
			flag_rep = !last_return_code;
		إذا (rmode == RES_UNTIL)
			flag_rep = last_return_code;
		إذا ( (rcode==EXIT_SUCCESS && pi->followup==PIPE_OR) ||
		     (rcode!=EXIT_SUCCESS && pi->followup==PIPE_AND) )
			skip_more_in_this_rmode=rmode;
#ifndef __U_BOOT__
		checkjobs(عدم);
#endif
	}
	أرجع rcode;
}

/* broken, of course, but OK for testing */
ساكن حرف *indenter(صحيح i)
{
	ساكن حرف blanks[]="                                    ";
	أرجع &blanks[حجم(blanks)-i-1];
}

/* return code is the exit status of the pipe */
ساكن صحيح free_pipe(بنية pipe *pi, صحيح indent)
{
	حرف **p;
	بنية child_prog *child;
#ifndef __U_BOOT__
	بنية redir_struct *r, *rnext;
#endif
	صحيح a, i, ret_code=0;
	حرف *ind = indenter(indent);

#ifndef __U_BOOT__
	إذا (pi->stopped_progs > 0)
		أرجع ret_code;
	final_printf("%s run pipe: (pid %d)\n",ind,getpid());
#endif
	لكل (i=0; i<pi->اڠک_progs; i++) {
		child = &pi->progs[i];
		final_printf("%s  command %d:\n",ind,i);
		إذا (child->argv) {
			لكل (a=0,p=child->argv; *p; a++,p++) {
				final_printf("%s   argv[%d] = %s\n",ind,a,*p);
			}
#ifndef __U_BOOT__
			globfree(&child->glob_result);
#وإلا
			لكل (a = 0; a < child->argc; a++) {
				حرر(child->argv[a]);
			}
			حرر(child->argv);
			حرر(child->argv_nonnull);
			child->argc = 0;
#endif
			child->argv=عدم;
		} وإلا إذا (child->group) {
#ifndef __U_BOOT__
			final_printf("%s   begin group (subshell:%d)\n",ind, child->subshell);
#endif
			ret_code = free_pipe_list(child->group,indent+3);
			final_printf("%s   end group\n",ind);
		} وإلا {
			final_printf("%s   (nil)\n",ind);
		}
#ifndef __U_BOOT__
		لكل (r=child->redirects; r; r=rnext) {
			final_printf("%s   redirect %d%s", ind, r->fd, redir_table[r->type].descrip);
			إذا (r->dup == -1) {
				/* guard against the case >$FOO, where foo is unset or blank */
				إذا (r->word.gl_pathv) {
					final_printf(" %s\n", *r->word.gl_pathv);
					globfree(&r->word);
				}
			} وإلا {
				final_printf("&%d\n", r->dup);
			}
			rnext=r->next;
			حرر(r);
		}
		child->redirects=عدم;
#endif
	}
	حرر(pi->progs);   /* children are an array, they get freed all at once */
	pi->progs=عدم;
	أرجع ret_code;
}

ساكن صحيح free_pipe_list(بنية pipe *head, صحيح indent)
{
	صحيح rcode=0;   /* if list has no members */
	بنية pipe *pi, *next;
	حرف *ind = indenter(indent);
	لكل (pi=head; pi; pi=next) {
		final_printf("%s pipe reserved mode %d\n", ind, pi->r_mode);
		rcode = free_pipe(pi, indent);
		final_printf("%s pipe followup code %d\n", ind, pi->followup);
		next=pi->next;
		pi->next=عدم;
		حرر(pi);
	}
	أرجع rcode;
}

/* Select which version we will use */
ساكن صحيح run_list(بنية pipe *pi)
{
	صحيح rcode=0;
#ifndef __U_BOOT__
	إذا (fake_mode==0) {
#endif
		rcode = run_list_real(pi);
#ifndef __U_BOOT__
	}
#endif
	/* free_pipe_list has the side effect of clearing memory
	 * In the long run that function can be merged with run_list_real,
	 * but doing that now would hobble the debugging effort. */
	free_pipe_list(pi,0);
	أرجع rcode;
}

/* The API for glob is arguably broken.  This routine pushes a non-matching
 * string into the output structure, removing non-backslashed backslashes.
 * If someone can prove me wrong, by performing this function within the
 * original glob(3) api, feel free to rewrite this routine into oblivion.
 * Return code (0 vs. GLOB_NOSPACE) matches glob(3).
 * XXX broken if the last character is '\\', check that before calling.
 */
#ifndef __U_BOOT__
ساكن صحيح globhack(ثابت حرف *src, صحيح flags, glob_t *pglob)
{
	صحيح cnt=0, pathc;
	ثابت حرف *s;
	حرف *dest;
	لكل (cnt=1, s=src; s && *s; s++) {
		إذا (*s == '\\') s++;
		cnt++;
	}
	dest = malloc(cnt);
	إذا (!dest) أرجع GLOB_NOSPACE;
	إذا (!(flags & GLOB_APPEND)) {
		pglob->gl_pathv=عدم;
		pglob->gl_pathc=0;
		pglob->gl_offs=0;
		pglob->gl_offs=0;
	}
	pathc = ++pglob->gl_pathc;
	pglob->gl_pathv = realloc(pglob->gl_pathv, (pathc+1)*حجم(*pglob->gl_pathv));
	إذا (pglob->gl_pathv == عدم) أرجع GLOB_NOSPACE;
	pglob->gl_pathv[pathc-1]=dest;
	pglob->gl_pathv[pathc]=عدم;
	لكل (s=src; s && *s; s++, dest++) {
		إذا (*s == '\\') s++;
		*dest = *s;
	}
	*dest='\0';
	أرجع 0;
}

/* XXX broken if the last character is '\\', check that before calling */
ساكن صحيح glob_needed(ثابت حرف *s)
{
	لكل (; *s; s++) {
		إذا (*s == '\\') s++;
		إذا (strchr("*[?",*s)) أرجع 1;
	}
	أرجع 0;
}

#إذا 0
ساكن فراغ globprint(glob_t *pglob)
{
	صحيح i;
	debug_printf("glob_t at %p:\n", pglob);
	debug_printf("  gl_pathc=%d  gl_pathv=%p  gl_offs=%d  gl_flags=%d\n",
		pglob->gl_pathc, pglob->gl_pathv, pglob->gl_offs, pglob->gl_flags);
	لكل (i=0; i<pglob->gl_pathc; i++)
		debug_printf("pglob->gl_pathv[%d] = %p = %s\n", i,
			pglob->gl_pathv[i], pglob->gl_pathv[i]);
}
#endif

ساكن صحيح xglob(o_string *dest, صحيح flags, glob_t *pglob)
{
	صحيح gr;

	/* short-circuit for null word */
	/* we can code this better when the debug_printf's are gone */
	إذا (dest->length == 0) {
		إذا (dest->nonnull) {
			/* bash man page calls this an "explicit" null */
			gr = globhack(dest->data, flags, pglob);
			debug_printf("globhack returned %d\n",gr);
		} وإلا {
			أرجع 0;
		}
	} وإلا إذا (glob_needed(dest->data)) {
		gr = glob(dest->data, flags, عدم, pglob);
		debug_printf("glob returned %d\n",gr);
		إذا (gr == GLOB_NOMATCH) {
			/* quote removal, or more accurately, backslash removal */
			gr = globhack(dest->data, flags, pglob);
			debug_printf("globhack returned %d\n",gr);
		}
	} وإلا {
		gr = globhack(dest->data, flags, pglob);
		debug_printf("globhack returned %d\n",gr);
	}
	إذا (gr == GLOB_NOSPACE)
		error_ڤسنن_and_die("out of memory during glob");
	إذا (gr != 0) { /* GLOB_ABORTED ? */
		error_ڤسنن("glob(3) error %d",gr);
	}
	/* globprint(glob_target); */
	أرجع gr;
}
#endif

#ifdef __U_BOOT__
ساكن حرف *get_dollar_var(حرف ch);
#endif

/* This is used to get/check local shell variables */
حرف *get_local_var(ثابت حرف *s)
{
	بنية variables *cur;

	إذا (!s)
		أرجع عدم;

#ifdef __U_BOOT__
	إذا (*s == '$')
		أرجع get_dollar_var(s[1]);
#endif

	لكل (cur = اتس_vars; cur; cur=cur->next)
		إذا(strcmp(cur->name, s)==0)
			أرجع cur->نيلاي;
	أرجع عدم;
}

/* This is used to set local shell variables
   flg_export==0 if only local (not exporting) variable
   flg_export==1 if "new" exporting environ
   flg_export>1  if current startup environ (not call putenv()) */
صحيح set_local_var(ثابت حرف *s, صحيح flg_export)
{
	حرف *name, *نيلاي;
	صحيح result=0;
	بنية variables *cur;

#ifdef __U_BOOT__
	/* might be possible! */
	إذا (!isalpha(*s))
		أرجع -1;
#endif

	name=strdup(s);

	/* Assume when we enter this function that we are already in
	 * NAME=VALUE format.  So the first order of business is to
	 * split 's' on the '=' into 'name' and 'value' */
	نيلاي = strchr(name, '=');
	إذا (!نيلاي) {
		حرر(name);
		أرجع -1;
	}
	*نيلاي++ = 0;

	إذا (!*نيلاي) {
		unset_local_var(name);
		حرر(name);
		أرجع 0;
	}

	لكل(cur = اتس_vars; cur; cur = cur->next) {
		إذا(strcmp(cur->name, name)==0)
			اكسر;
	}

	إذا(cur) {
		إذا(strcmp(cur->نيلاي, نيلاي)==0) {
			إذا(flg_export>0 && cur->flg_export==0)
				cur->flg_export=flg_export;
			وإلا
				result++;
		} وإلا {
			إذا(cur->flg_read_only) {
				error_ڤسنن("%s: readonly variable", name);
				result = -1;
			} وإلا {
				إذا(flg_export>0 || cur->flg_export>1)
					cur->flg_export=1;
				حرر(cur->نيلاي);

				cur->نيلاي = strdup(نيلاي);
			}
		}
	} وإلا {
		cur = malloc(حجم(بنية variables));
		إذا(!cur) {
			result = -1;
		} وإلا {
			cur->name = strdup(name);
			إذا (cur->name == عدم) {
				حرر(cur);
				result = -1;
			} وإلا {
				بنية variables *bottom = اتس_vars;
				cur->نيلاي = strdup(نيلاي);
				cur->next = عدم;
				cur->flg_export = flg_export;
				cur->flg_read_only = 0;
				طالما(bottom->next) bottom=bottom->next;
				bottom->next = cur;
			}
		}
	}

#ifndef __U_BOOT__
	إذا(result==0 && cur->flg_export==1) {
		*(نيلاي-1) = '=';
		result = putenv(name);
	} وإلا {
#endif
		حرر(name);
#ifndef __U_BOOT__
		إذا(result>0)            /* equivalent to previous set */
			result = 0;
	}
#endif
	أرجع result;
}

فراغ unset_local_var(ثابت حرف *name)
{
	بنية variables *cur;

	إذا (name) {
		لكل (cur = اتس_vars; cur; cur=cur->next) {
			إذا(strcmp(cur->name, name)==0)
				اكسر;
		}
		إذا (cur != عدم) {
			بنية variables *next = اتس_vars;
			إذا(cur->flg_read_only) {
				error_ڤسنن("%s: readonly variable", name);
				أرجع;
			} وإلا {
#ifndef __U_BOOT__
				إذا(cur->flg_export)
					unenv_set(cur->name);
#endif
				حرر(cur->name);
				حرر(cur->نيلاي);
				طالما (next->next != cur)
					next = next->next;
				next->next = cur->next;
			}
			حرر(cur);
		}
	}
}

ساكن صحيح is_assignment(ثابت حرف *s)
{
	إذا (s == عدم)
		أرجع 0;

	إذا (!isalpha(*s)) أرجع 0;
	++s;
	طالما(isalnum(*s) || *s=='_') ++s;
	أرجع *s=='=';
}

#ifndef __U_BOOT__
/* the src parameter allows us to peek forward to a possible &n syntax
 * for file descriptor duplication, e.g., "2>&1".
 * Return code is 0 normally, 1 if a syntax error is detected in src.
 * Resource errors (in xmalloc) cause the process to exit */
ساكن صحيح setup_redirect(بنية p_context *ctx, صحيح fd, redir_type style,
	بنية in_str *ماسوقن)
{
	بنية child_prog *child=ctx->child;
	بنية redir_struct *redir = child->redirects;
	بنية redir_struct *last_redir=عدم;

	/* Create a new redir_struct and drop it onto the end of the linked list */
	طالما(redir) {
		last_redir=redir;
		redir=redir->next;
	}
	redir = xmalloc(حجم(بنية redir_struct));
	redir->next=عدم;
	redir->word.gl_pathv=عدم;
	إذا (last_redir) {
		last_redir->next=redir;
	} وإلا {
		child->redirects=redir;
	}

	redir->type=style;
	redir->fd= (fd==-1) ? redir_table[style].default_fd : fd ;

	debug_printf("Redirect type %d%s\n", redir->fd, redir_table[style].descrip);

	/* Check for a '2>&1' type redirect */
	redir->dup = redirect_dup_اڠک(ماسوقن);
	إذا (redir->dup == -2) أرجع 1;  /* syntax error */
	إذا (redir->dup != -1) {
		/* Erik had a check here that the file descriptor in question
		 * is legit; I postpone that to "run time"
		 * A "-" representation of "close me" shows up as a -3 here */
		debug_printf("Duplicating redirect '%d>&%d'\n", redir->fd, redir->dup);
	} وإلا {
		/* We do _not_ try to open the file that src points to,
		 * since we need to return and let src be expanded first.
		 * Set ctx->pending_redirect, so we know what to do at the
		 * end of the next parsed word.
		 */
		ctx->pending_redirect = redir;
	}
	أرجع 0;
}
#endif

ساكن بنية pipe *new_pipe(فراغ)
{
	بنية pipe *pi;
	pi = xmalloc(حجم(بنية pipe));
	pi->اڠک_progs = 0;
	pi->progs = عدم;
	pi->next = عدم;
	pi->followup = 0;  /* invalid */
	pi->r_mode = RES_NONE;
	أرجع pi;
}

ساكن فراغ initialize_context(بنية p_context *ctx)
{
	ctx->pipe=عدم;
#ifndef __U_BOOT__
	ctx->pending_redirect=عدم;
#endif
	ctx->child=عدم;
	ctx->list_head=new_pipe();
	ctx->pipe=ctx->list_head;
	ctx->w=RES_NONE;
	ctx->تيندنن=عدم;
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
بنية reserved_combo {
	حرف *literal;
	صحيح code;
	long flag;
};
/* Mostly a list of accepted follow-up reserved words.
 * FLAG_END means we are done with the sequence, and are ready
 * to turn the compound list into a command.
 * FLAG_START means the word must start a new compound list.
 */
ساكن بنية reserved_combo reserved_list[] = {
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
#define NRES (حجم(reserved_list)/حجم(بنية reserved_combo))

ساكن صحيح reserved_word(o_string *dest, بنية p_context *ctx)
{
	بنية reserved_combo *r;
	لكل (r=reserved_list;
		r<reserved_list+NRES; r++) {
		إذا (strcmp(dest->data, r->literal) == 0) {
			debug_printf("found reserved word %s, code %d\n",r->literal,r->code);
			إذا (r->flag & FLAG_START) {
				بنية p_context *new = xmalloc(حجم(بنية p_context));
				debug_printf("push stack\n");
				إذا (ctx->w == RES_IN || ctx->w == RES_FOR) {
					syntax();
					حرر(new);
					ctx->w = RES_SNTX;
					b_reset(dest);
					أرجع 1;
				}
				*new = *ctx;   /* physical copy */
				initialize_context(ctx);
				ctx->تيندنن=new;
			} وإلا إذا ( ctx->w == RES_NONE || ! (ctx->old_flag & (1<<r->code))) {
				syntax();
				ctx->w = RES_SNTX;
				b_reset(dest);
				أرجع 1;
			}
			ctx->w=r->code;
			ctx->old_flag = r->flag;
			إذا (ctx->old_flag & FLAG_END) {
				بنية p_context *old;
				debug_printf("pop stack\n");
				done_pipe(ctx,PIPE_SEQ);
				old = ctx->تيندنن;
				old->child->group = ctx->list_head;
#ifndef __U_BOOT__
				old->child->subshell = 0;
#endif
				*ctx = *old;   /* physical copy */
				حرر(old);
			}
			b_reset (dest);
			أرجع 1;
		}
	}
	أرجع 0;
}

/* normal return is 0.
 * Syntax or xglob errors return 1. */
ساكن صحيح done_word(o_string *dest, بنية p_context *ctx)
{
	بنية child_prog *child=ctx->child;
#ifndef __U_BOOT__
	glob_t *glob_target;
	صحيح gr, flags = 0;
#وإلا
	حرف *str, *s;
	صحيح argc, cnt;
#endif

	debug_printf("done_word: %s %p\n", dest->data, child);
	إذا (dest->length == 0 && !dest->nonnull) {
		debug_printf("  true null, ignored\n");
		أرجع 0;
	}
#ifndef __U_BOOT__
	إذا (ctx->pending_redirect) {
		glob_target = &ctx->pending_redirect->word;
	} وإلا {
#endif
		إذا (child->group) {
			syntax();
			أرجع 1;  /* syntax error, groups and arglists don't mix */
		}
		إذا (!child->argv && (ctx->type & FLAG_PARSE_SEMICOLON)) {
			debug_printf("checking %s for reserved-ness\n",dest->data);
			إذا (reserved_word(dest,ctx)) أرجع ctx->w==RES_SNTX;
		}
#ifndef __U_BOOT__
		glob_target = &child->glob_result;
		إذا (child->argv) flags |= GLOB_APPEND;
#وإلا
		لكل (cnt = 1, s = dest->data; s && *s; s++) {
			إذا (*s == '\\') s++;
			cnt++;
		}
		str = malloc(cnt);
		إذا (!str) أرجع 1;
		إذا ( child->argv == عدم) {
			child->argc=0;
		}
		argc = ++child->argc;
		child->argv = realloc(child->argv, (argc+1)*حجم(*child->argv));
		إذا (child->argv == عدم) {
			حرر(str);
			أرجع 1;
		}
		child->argv_nonnull = realloc(child->argv_nonnull,
					(argc+1)*حجم(*child->argv_nonnull));
		إذا (child->argv_nonnull == عدم) {
			حرر(str);
			أرجع 1;
		}
		child->argv[argc-1]=str;
		child->argv_nonnull[argc-1] = dest->nonnull;
		child->argv[argc]=عدم;
		child->argv_nonnull[argc] = 0;
		لكل (s = dest->data; s && *s; s++,str++) {
			إذا (*s == '\\') s++;
			*str = *s;
		}
		*str = '\0';
#endif
#ifndef __U_BOOT__
	}
	gr = xglob(dest, flags, glob_target);
	إذا (gr != 0) أرجع 1;
#endif

	b_reset(dest);
#ifndef __U_BOOT__
	إذا (ctx->pending_redirect) {
		ctx->pending_redirect=عدم;
		إذا (glob_target->gl_pathc != 1) {
			error_ڤسنن("ambiguous redirect");
			أرجع 1;
		}
	} وإلا {
		child->argv = glob_target->gl_pathv;
	}
#endif
	إذا (ctx->w == RES_FOR) {
		done_word(dest,ctx);
		done_pipe(ctx,PIPE_SEQ);
	}
	أرجع 0;
}

/* The only possible error here is out of memory, in which case
 * xmalloc exits. */
ساكن صحيح done_command(بنية p_context *ctx)
{
	/* The child is really already in the pipe structure, so
	 * advance the pipe counter and make a new, null child.
	 * Only real trickiness here is that the uncommitted
	 * child structure, to which ctx->child points, is not
	 * counted in pi->num_progs. */
	بنية pipe *pi=ctx->pipe;
	بنية child_prog *prog=ctx->child;

	إذا (prog && prog->group == عدم
		 && prog->argv == عدم
#ifndef __U_BOOT__
		 && prog->redirects == عدم) {
#وإلا
										) {
#endif
		debug_printf("done_command: skipping null command\n");
		أرجع 0;
	} وإلا إذا (prog) {
		pi->اڠک_progs++;
		debug_printf("done_command: num_progs incremented to %d\n",pi->اڠک_progs);
	} وإلا {
		debug_printf("done_command: initializing\n");
	}
	pi->progs = xrealloc(pi->progs, حجم(*pi->progs) * (pi->اڠک_progs+1));

	prog = pi->progs + pi->اڠک_progs;
#ifndef __U_BOOT__
	prog->redirects = عدم;
#endif
	prog->argv = عدم;
	prog->argv_nonnull = عدم;
#ifndef __U_BOOT__
	prog->is_stopped = 0;
#endif
	prog->group = عدم;
#ifndef __U_BOOT__
	prog->glob_result.gl_pathv = عدم;
	prog->family = pi;
#endif
	prog->sp = 0;
	ctx->child = prog;
	prog->type = ctx->type;

	/* but ctx->pipe and ctx->list_head remain unchanged */
	أرجع 0;
}

ساكن صحيح done_pipe(بنية p_context *ctx, pipe_style type)
{
	بنية pipe *new_p;
	done_command(ctx);  /* implicit closure of previous command */
	debug_printf("done_pipe, type %d\n", type);
	ctx->pipe->followup = type;
	ctx->pipe->r_mode = ctx->w;
	new_p=new_pipe();
	ctx->pipe->next = new_p;
	ctx->pipe = new_p;
	ctx->child = عدم;
	done_command(ctx);  /* set up new pipe to accept commands */
	أرجع 0;
}

#ifndef __U_BOOT__
/* peek ahead in the in_str to find out if we have a "&n" construct,
 * as in "2>&1", that represents duplicating a file descriptor.
 * returns either -2 (syntax error), -1 (no &), or the number found.
 */
ساكن صحيح redirect_dup_اڠک(بنية in_str *ماسوقن)
{
	صحيح ch, d=0, ok=0;
	ch = b_peek(ماسوقن);
	إذا (ch != '&') أرجع -1;

	b_getch(ماسوقن);  /* get the & */
	ch=b_peek(ماسوقن);
	إذا (ch == '-') {
		b_getch(ماسوقن);
		أرجع -3;  /* "-" represents "close me" */
	}
	طالما (isdigit(ch)) {
		d = d*10+(ch-'0');
		ok=1;
		b_getch(ماسوقن);
		ch = b_peek(ماسوقن);
	}
	إذا (ok) أرجع d;

	error_ڤسنن("ambiguous redirect");
	أرجع -2;
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
ساكن صحيح redirect_opt_اڠک(o_string *o)
{
	صحيح اڠک;

	إذا (o->length==0) أرجع -1;
	لكل(اڠک=0; اڠک<o->length; اڠک++) {
		إذا (!isdigit(*(o->data+اڠک))) {
			أرجع -1;
		}
	}
	/* reuse num (and save an int) */
	اڠک=atoi(o->data);
	b_reset(o);
	أرجع اڠک;
}

FILE *generate_stream_from_list(بنية pipe *head)
{
	FILE *pf;
#إذا 1
	صحيح pid, channel[2];
	إذا (pipe(channel)<0) perror_ڤسنن_and_die("pipe");
	pid=fork();
	إذا (pid<0) {
		perror_ڤسنن_and_die("fork");
	} وإلا إذا (pid==0) {
		close(channel[0]);
		إذا (channel[1] != 1) {
			dup2(channel[1],1);
			close(channel[1]);
		}
#إذا 0
#define SURROGATE "surrogate response"
		اكتب(1,SURROGATE,حجم(SURROGATE));
		_exit(run_list(head));
#وإلا
		_exit(run_list_real(head));   /* leaks memory */
#endif
	}
	debug_printf("forked child %d\n",pid);
	close(channel[1]);
	pf = fdopen(channel[0],"r");
	debug_printf("pipe on FILE *%p\n",pf);
#وإلا
	free_pipe_list(head,0);
	pf=popen("echo surrogate response","r");
	debug_printf("started fake pipe on FILE *%p\n",pf);
#endif
	أرجع pf;
}

/* this version hacked for testing purposes */
/* return code is exit status of the process that is run. */
ساكن صحيح process_command_subs(o_string *dest, بنية p_context *ctx, بنية in_str *ماسوقن, صحيح subst_end)
{
	صحيح retcode;
	o_string result=NULL_O_STRING;
	بنية p_context inner;
	FILE *p;
	بنية in_str pipe_str;
	initialize_context(&inner);

	/* recursion to generate command */
	retcode = parse_stream(&result, &inner, ماسوقن, subst_end);
	إذا (retcode != 0) أرجع retcode;  /* syntax error or EOF */
	done_word(&result, &inner);
	done_pipe(&inner, PIPE_SEQ);
	b_free(&result);

	p=generate_stream_from_list(inner.list_head);
	إذا (p==عدم) أرجع 1;
	mark_open(fileno(p));
	setup_file_in_str(&pipe_str, p);

	/* now send results of command back into original context */
	retcode = parse_stream(dest, ctx, &pipe_str, '\0');
	/* XXX In case of a syntax error, should we try to kill the child?
	 * That would be tough to do right, so just read until EOF. */
	إذا (retcode == 1) {
		طالما (b_getch(&pipe_str)!=EOF) { /* discard */ };
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
	أرجع retcode;
}

ساكن صحيح parse_group(o_string *dest, بنية p_context *ctx,
	بنية in_str *ماسوقن, صحيح ch)
{
	صحيح rcode, endch=0;
	بنية p_context sub;
	بنية child_prog *child = ctx->child;
	إذا (child->argv) {
		syntax();
		أرجع 1;  /* syntax error, groups and arglists don't mix */
	}
	initialize_context(&sub);
	بدل(ch) {
		حالة '(': endch=')'; child->subshell=1; اكسر;
		حالة '{': endch='}'; اكسر;
		افتراضي: syntax();   /* really logic error */
	}
	rcode=parse_stream(dest,&sub,ماسوقن,endch);
	done_word(dest,&sub); /* finish off the final word in the subcontext */
	done_pipe(&sub, PIPE_SEQ);  /* and the final command there, too */
	child->group = sub.list_head;
	أرجع rcode;
	/* child remains "open", available for possible redirects */
}
#endif

/* basically useful version until someone wants to get fancier,
 * see the bash man page under "Parameter Expansion" */
ساكن حرف *lookup_param(حرف *src)
{
	حرف *p;
	حرف *sep;
	حرف *default_val = عدم;
	صحيح assign = 0;
	صحيح expand_empty = 0;

	إذا (!src)
		أرجع عدم;

	sep = strchr(src, ':');

	إذا (sep) {
		*sep = '\0';
		إذا (*(sep + 1) == '-')
			default_val = sep+2;
		إذا (*(sep + 1) == '=') {
			default_val = sep+2;
			assign = 1;
		}
		إذا (*(sep + 1) == '+') {
			default_val = sep+2;
			expand_empty = 1;
		}
	}

	p = env_get(src);
	إذا (!p)
		p = get_local_var(src);

	إذا (!p || strlen(p) == 0) {
		p = default_val;
		إذا (assign) {
			حرف *var = malloc(strlen(src)+strlen(default_val)+2);
			إذا (var) {
				sprintf(var, "%s=%s", src, default_val);
				set_local_var(var, 0);
			}
			حرر(var);
		}
	} وإلا إذا (expand_empty) {
		p += strlen(p);
	}

	إذا (sep)
		*sep = ':';

	أرجع p;
}

#ifdef __U_BOOT__
ساكن حرف *get_dollar_var(حرف ch)
{
	ساكن حرف buf[40];

	buf[0] = '\0';
	بدل (ch) {
		حالة '?':
			sprintf(buf, "%u", (unsigned صحيح)last_return_code);
			اكسر;
		افتراضي:
			أرجع عدم;
	}
	أرجع buf;
}
#endif

/* return code: 0 for OK, 1 for syntax error */
ساكن صحيح handle_dollar(o_string *dest, بنية p_context *ctx, بنية in_str *ماسوقن)
{
#ifndef __U_BOOT__
	صحيح i, advance=0;
#وإلا
	صحيح advance=0;
#endif
#ifndef __U_BOOT__
	حرف sep[]=" ";
#endif
	صحيح ch = ماسوقن->peek(ماسوقن);  /* first character after the $ */
	debug_printf("handle_dollar: ch=%c\n",ch);
	إذا (isalpha(ch)) {
		b_addchr(dest, SPECIAL_VAR_سيمبول);
		ctx->child->sp++;
		طالما(ch=b_peek(ماسوقن),isalnum(ch) || ch=='_') {
			b_getch(ماسوقن);
			b_addchr(dest,ch);
		}
		b_addchr(dest, SPECIAL_VAR_سيمبول);
#ifndef __U_BOOT__
	} وإلا إذا (isdigit(ch)) {
		i = ch-'0';  /* XXX is $0 special? */
		إذا (i<global_argc) {
			parse_string(dest, ctx, global_argv[i]); /* recursion */
		}
		advance = 1;
#endif
	} وإلا بدل (ch) {
#ifndef __U_BOOT__
		حالة '$':
			b_adduint(dest,getpid());
			advance = 1;
			اكسر;
		حالة '!':
			إذا (last_bg_pid > 0) b_adduint(dest, last_bg_pid);
			advance = 1;
			اكسر;
#endif
		حالة '?':
#ifndef __U_BOOT__
			b_adduint(dest,last_return_code);
#وإلا
			ctx->child->sp++;
			b_addchr(dest, SPECIAL_VAR_سيمبول);
			b_addchr(dest, '$');
			b_addchr(dest, '?');
			b_addchr(dest, SPECIAL_VAR_سيمبول);
#endif
			advance = 1;
			اكسر;
#ifndef __U_BOOT__
		حالة '#':
			b_adduint(dest,global_argc ? global_argc-1 : 0);
			advance = 1;
			اكسر;
#endif
		حالة '{':
			b_addchr(dest, SPECIAL_VAR_سيمبول);
			ctx->child->sp++;
			b_getch(ماسوقن);
			/* XXX maybe someone will try to escape the '}' */
			طالما(ch=b_getch(ماسوقن),ch!=EOF && ch!='}') {
				b_addchr(dest,ch);
			}
			إذا (ch != '}') {
				syntax();
				أرجع 1;
			}
			b_addchr(dest, SPECIAL_VAR_سيمبول);
			اكسر;
#ifndef __U_BOOT__
		حالة '(':
			b_getch(ماسوقن);
			process_command_subs(dest, ctx, ماسوقن, ')');
			اكسر;
		حالة '*':
			sep[0]=ifs[0];
			لكل (i=1; i<global_argc; i++) {
				parse_string(dest, ctx, global_argv[i]);
				إذا (i+1 < global_argc) parse_string(dest, ctx, sep);
			}
			اكسر;
		حالة '@':
		حالة '-':
		حالة '_':
			/* still unhandled, but should be eventually */
			error_ڤسنن("unhandled syntax: $%c",ch);
			أرجع 1;
			اكسر;
#endif
		افتراضي:
			b_addqchr(dest,'$',dest->quote);
	}
	/* Eat the character if the flag was set.  If the compiler
	 * is smart enough, we could substitute "b_getch(input);"
	 * for all the "advance = 1;" above, and also end up with
	 * a nice size-optimized program.  Hah!  That'll be the day.
	 */
	إذا (advance) b_getch(ماسوقن);
	أرجع 0;
}

#ifndef __U_BOOT__
صحيح parse_string(o_string *dest, بنية p_context *ctx, ثابت حرف *src)
{
	بنية in_str foo;
	setup_string_in_str(&foo, src);
	أرجع parse_stream(dest, ctx, &foo, '\0');
}
#endif

/* return code is 0 for normal exit, 1 for syntax error */
ساكن صحيح parse_stream(o_string *dest, بنية p_context *ctx,
			بنية in_str *ماسوقن, صحيح end_trigger)
{
	unsigned صحيح ch, m;
#ifndef __U_BOOT__
	صحيح redir_fd;
	redir_type redir_style;
#endif
	صحيح next;

	/* Only double-quote state is handled in the state variable dest->quote.
	 * A single-quote triggers a bypass of the main loop until its mate is
	 * found.  When recursing, quote state is passed in via dest->quote. */

	debug_printf("parse_stream, end_trigger=%d\n",end_trigger);
	طالما ((ch=b_getch(ماسوقن))!=EOF) {
		m = map[ch];
#ifdef __U_BOOT__
		إذا (ماسوقن->__promptme == 0) أرجع 1;
#endif
		next = (ch == '\n') ? 0 : b_peek(ماسوقن);

		debug_printf("parse_stream: ch=%c (%d) m=%d quote=%d - %c\n",
			ch >= ' ' ? ch : '.', ch, m,
			dest->quote, ctx->تيندنن == عدم ? '*' : '.');

		إذا (m==0 || ((m==1 || m==2) && dest->quote)) {
			b_addqchr(dest, ch, dest->quote);
		} وإلا {
			إذا (m==2) {  /* unquoted IFS */
				إذا (done_word(dest, ctx)) {
					أرجع 1;
				}
				/* If we aren't performing a substitution, treat a newline as a
				 * command separator.  */
				إذا (end_trigger != '\0' && ch=='\n')
					done_pipe(ctx,PIPE_SEQ);
			}
			إذا (ch == end_trigger && !dest->quote && ctx->w==RES_NONE) {
				debug_printf("leaving parse_stream (triggered)\n");
				أرجع 0;
			}
#إذا 0
			إذا (ch=='\n') {
				/* Yahoo!  Time to run with it! */
				done_pipe(ctx,PIPE_SEQ);
				run_list(ctx->list_head);
				initialize_context(ctx);
			}
#endif
			إذا (m!=2) بدل (ch) {
		حالة '#':
			إذا (dest->length == 0 && !dest->quote) {
				طالما(ch=b_peek(ماسوقن),ch!=EOF && ch!='\n') { b_getch(ماسوقن); }
			} وإلا {
				b_addqchr(dest, ch, dest->quote);
			}
			اكسر;
		حالة '\\':
			إذا (next == EOF) {
				syntax();
				أرجع 1;
			}
			b_addqchr(dest, '\\', dest->quote);
			b_addqchr(dest, b_getch(ماسوقن), dest->quote);
			اكسر;
		حالة '$':
			إذا (handle_dollar(dest, ctx, ماسوقن)!=0) أرجع 1;
			اكسر;
		حالة '\'':
			dest->nonnull = 1;
			طالما(ch=b_getch(ماسوقن),ch!=EOF && ch!='\'') {
#ifdef __U_BOOT__
				إذا(ماسوقن->__promptme == 0) أرجع 1;
#endif
				b_addchr(dest,ch);
			}
			إذا (ch==EOF) {
				syntax();
				أرجع 1;
			}
			اكسر;
		حالة '"':
			dest->nonnull = 1;
			dest->quote = !dest->quote;
			اكسر;
#ifndef __U_BOOT__
		حالة '`':
			process_command_subs(dest, ctx, ماسوقن, '`');
			اكسر;
		حالة '>':
			redir_fd = redirect_opt_اڠک(dest);
			done_word(dest, ctx);
			redir_style=REDIRECT_OVERWRITE;
			إذا (next == '>') {
				redir_style=REDIRECT_APPEND;
				b_getch(ماسوقن);
			} وإلا إذا (next == '(') {
				syntax();   /* until we support >(list) Process Substitution */
				أرجع 1;
			}
			setup_redirect(ctx, redir_fd, redir_style, ماسوقن);
			اكسر;
		حالة '<':
			redir_fd = redirect_opt_اڠک(dest);
			done_word(dest, ctx);
			redir_style=REDIRECT_ماسوقن;
			إذا (next == '<') {
				redir_style=REDIRECT_HEREIS;
				b_getch(ماسوقن);
			} وإلا إذا (next == '>') {
				redir_style=REDIRECT_IO;
				b_getch(ماسوقن);
			} وإلا إذا (next == '(') {
				syntax();   /* until we support <(list) Process Substitution */
				أرجع 1;
			}
			setup_redirect(ctx, redir_fd, redir_style, ماسوقن);
			اكسر;
#endif
		حالة ';':
			done_word(dest, ctx);
			done_pipe(ctx,PIPE_SEQ);
			اكسر;
		حالة '&':
			done_word(dest, ctx);
			إذا (next=='&') {
				b_getch(ماسوقن);
				done_pipe(ctx,PIPE_AND);
			} وإلا {
#ifndef __U_BOOT__
				done_pipe(ctx,PIPE_BG);
#وإلا
				syntax_err();
				أرجع 1;
#endif
			}
			اكسر;
		حالة '|':
			done_word(dest, ctx);
			إذا (next=='|') {
				b_getch(ماسوقن);
				done_pipe(ctx,PIPE_OR);
			} وإلا {
				/* we could pick up a file descriptor choice here
				 * with redirect_opt_num(), but bash doesn't do it.
				 * "echo foo 2| cat" yields "foo 2". */
#ifndef __U_BOOT__
				done_command(ctx);
#وإلا
				syntax_err();
				أرجع 1;
#endif
			}
			اكسر;
#ifndef __U_BOOT__
		حالة '(':
		حالة '{':
			إذا (parse_group(dest, ctx, ماسوقن, ch)!=0) أرجع 1;
			اكسر;
		حالة ')':
		حالة '}':
			syntax();   /* Proper use of this character caught by end_trigger */
			أرجع 1;
			اكسر;
#endif
		حالة SUBSTED_VAR_سيمبول:
			dest->nonnull = 1;
			طالما (ch = b_getch(ماسوقن), ch != EOF &&
			    ch != SUBSTED_VAR_سيمبول) {
				debug_printf("subst, pass=%d\n", ch);
				إذا (ماسوقن->__promptme == 0)
					أرجع 1;
				b_addchr(dest, ch);
			}
			debug_printf("subst, term=%d\n", ch);
			إذا (ch == EOF) {
				syntax();
				أرجع 1;
			}
			اكسر;
		افتراضي:
			syntax();   /* this is really an internal logic error */
			أرجع 1;
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
	إذا (end_trigger != '\0') أرجع -1;
	أرجع 0;
}

ساكن فراغ mapset(ثابت unsigned حرف *set, صحيح code)
{
	ثابت unsigned حرف *s;
	لكل (s=set; *s; s++) map[*s] = code;
}

ساكن فراغ update_ifs_map(فراغ)
{
	/* char *ifs and char map[256] are both globals. */
	ifs = (uchar *)env_get("IFS");
	إذا (ifs == عدم) ifs=(uchar *)" \t\n";
	/* Precompute a list of 'flow through' behavior so it can be treated
	 * quickly up front.  Computation is necessary because of IFS.
	 * Special case handling of IFS == " \t\n" is not implemented.
	 * The map[] array only really needs two bits each, and on most machines
	 * that would be faster because of the reduced L1 cache footprint.
	 */
	memset(map,0,حجم(map)); /* most characters flow through always */
#ifndef __U_BOOT__
	mapset((uchar *)"\\$'\"`", 3);      /* never flow through */
	mapset((uchar *)"<>;&|(){}#", 1);   /* flow through if quoted */
#وإلا
	{
		uchar subst[2] = {SUBSTED_VAR_سيمبول, 0};
		mapset(subst, 3);       /* never flow through */
	}
	mapset((uchar *)"\\$'\"", 3);       /* never flow through */
	mapset((uchar *)";&|#", 1);         /* flow through if quoted */
#endif
	mapset(ifs, 2);            /* also flow through if quoted */
}

/* most recursion does not come through here, the exeception is
 * from builtin_source() */
ساكن صحيح parse_stream_outer(بنية in_str *inp, صحيح flag)
{

	بنية p_context ctx;
	o_string temp=NULL_O_STRING;
	صحيح rcode;
#ifdef __U_BOOT__
	صحيح code = 1;
#endif
	افعل {
		ctx.type = flag;
		initialize_context(&ctx);
		update_ifs_map();
		إذا (!(flag & FLAG_PARSE_SEMICOLON) || (flag & FLAG_REPARSING)) mapset((uchar *)";$&|", 0);
		inp->promptmode=1;
		rcode = parse_stream(&temp, &ctx, inp,
				     flag & FLAG_CONT_ON_NEWLINE ? -1 : '\n');
#ifdef __U_BOOT__
		إذا (rcode == 1) flag_repeat = 0;
#endif
		إذا (rcode != 1 && ctx.old_flag != 0) {
			syntax();
#ifdef __U_BOOT__
			flag_repeat = 0;
#endif
		}
		إذا (rcode != 1 && ctx.old_flag == 0) {
			done_word(&temp, &ctx);
			done_pipe(&ctx,PIPE_SEQ);
#ifndef __U_BOOT__
			run_list(ctx.list_head);
#وإلا
			code = run_list(ctx.list_head);
			إذا (code == -2) {	/* exit */
				b_free(&temp);
				code = 0;
				/* XXX hackish way to not allow exit from main loop */
				إذا (inp->peek == file_peek) {
					printf("exit not allowed from main input shell.\n");
					استمر;
				}
				/*
				 * DANGER
				 * Return code -2 is special in this context,
				 * it indicates exit from inner pipe instead
				 * of return code itself, the return code is
				 * stored in 'last_return_code' variable!
				 * DANGER
				 */
				أرجع -2;
			}
			إذا (code == -1)
			    flag_repeat = 0;
#endif
		} وإلا {
			إذا (ctx.old_flag != 0) {
				حرر(ctx.تيندنن);
				b_reset(&temp);
			}
#ifdef __U_BOOT__
			إذا (inp->__promptme == 0) printf("<INTERRUPT>\n");
			inp->__promptme = 1;
#endif
			temp.nonnull = 0;
			temp.quote = 0;
			inp->p = عدم;
			free_pipe_list(ctx.list_head,0);
		}
		b_free(&temp);
	/* loop on syntax errors, return on EOF */
	} طالما (rcode != -1 && !(flag & FLAG_EXIT_FROM_LOOP) &&
		(inp->peek != static_peek || b_peek(inp)));
#ifndef __U_BOOT__
	أرجع 0;
#وإلا
	أرجع (code != 0) ? 1 : 0;
#endif /* __U_BOOT__ */
}

#ifndef __U_BOOT__
ساكن صحيح parse_string_outer(ثابت حرف *s, صحيح flag)
#وإلا
صحيح parse_string_outer(ثابت حرف *s, صحيح flag)
#endif	/* __U_BOOT__ */
{
	بنية in_str ماسوقن;
	صحيح rcode;
#ifdef __U_BOOT__
	حرف *p = عدم;
	إذا (!s)
		أرجع 1;
	إذا (!*s)
		أرجع 0;
	إذا (!(p = strchr(s, '\n')) || *++p) {
		p = xmalloc(strlen(s) + 2);
		strcpy(p, s);
		strcat(p, "\n");
		setup_string_in_str(&ماسوقن, p);
		rcode = parse_stream_outer(&ماسوقن, flag);
		حرر(p);
		أرجع rcode == -2 ? last_return_code : rcode;
	} وإلا {
#endif
	setup_string_in_str(&ماسوقن, s);
	rcode = parse_stream_outer(&ماسوقن, flag);
	أرجع rcode == -2 ? last_return_code : rcode;
#ifdef __U_BOOT__
	}
#endif
}

#ifndef __U_BOOT__
ساكن صحيح parse_file_outer(FILE *f)
#وإلا
صحيح parse_file_outer(فراغ)
#endif
{
	صحيح rcode;
	بنية in_str ماسوقن;
#ifndef __U_BOOT__
	setup_file_in_str(&ماسوقن, f);
#وإلا
	setup_file_in_str(&ماسوقن);
#endif
	rcode = parse_stream_outer(&ماسوقن, FLAG_PARSE_SEMICOLON);
	أرجع rcode == -2 ? last_return_code : rcode;
}

#ifdef __U_BOOT__
صحيح u_boot_hush_start(فراغ)
{
	إذا (اتس_vars == عدم) {
		اتس_vars = malloc(حجم(بنية variables));
		اتس_vars->name = "HUSH_VERSION";
		اتس_vars->نيلاي = "0.01";
		اتس_vars->next = عدم;
		اتس_vars->flg_export = 0;
		اتس_vars->flg_read_only = 1;
	}
	أرجع 0;
}

ساكن فراغ *xmalloc(size_t size)
{
	فراغ *p = عدم;

	إذا (!(p = malloc(size))) {
	    printf("ERROR : xmalloc failed\n");
	    لكل(;;);
	}
	أرجع p;
}

ساكن فراغ *xrealloc(فراغ *ptr, size_t size)
{
	فراغ *p = عدم;

	إذا (!(p = realloc(ptr, size))) {
	    printf("ERROR : xrealloc failed\n");
	    لكل(;;);
	}
	أرجع p;
}
#endif /* __U_BOOT__ */

#ifndef __U_BOOT__
/* Make sure we have a controlling tty.  If we get started under a job
 * aware app (like bash for example), make sure we are now in charge so
 * we don't fight over who gets the foreground */
ساكن فراغ setup_job_control(فراغ)
{
	ساكن pid_t shell_pgrp;
	/* Loop until we are in the foreground.  */
	طالما (tcgetpgrp (shell_terminal) != (shell_pgrp = getpgrp ()))
		kill (- shell_pgrp, SIGTTIN);

	/* Ignore interactive and job-control signals.  */
	إشارة(SIGINT, SIG_IGN);
	إشارة(SIGQUIT, SIG_IGN);
	إشارة(SIGTERM, SIG_IGN);
	إشارة(SIGTSTP, SIG_IGN);
	إشارة(SIGTTIN, SIG_IGN);
	إشارة(SIGTTOU, SIG_IGN);
	إشارة(SIGCHLD, SIG_IGN);

	/* Put ourselves in our own process group.  */
	setsid();
	shell_pgrp = getpid ();
	setpgid (shell_pgrp, shell_pgrp);

	/* Grab control of the terminal.  */
	tcsetpgrp(shell_terminal, shell_pgrp);
}

صحيح hush_main(صحيح argc, حرف * ثابت *argv)
{
	صحيح opt;
	FILE *ماسوقن;
	حرف **e = environ;

	/* XXX what should these be while sourcing /etc/profile? */
	global_argc = argc;
	global_argv = argv;

	/* (re?) initialize globals.  Sometimes hush_main() ends up calling
	 * hush_main(), therefore we cannot rely on the BSS to zero out this
	 * stuff.  Reset these to 0 every time. */
	ifs = عدم;
	/* map[] is taken care of with call to update_ifs_map() */
	fake_mode = 0;
	interactive = 0;
	close_me_head = عدم;
	last_bg_pid = 0;
	job_list = عدم;
	last_jobid = 0;

	/* Initialize some more globals to non-zero values */
	set_cwd();
#ifdef CONFIG_FEATURE_COMMAND_EDITING
	cmdedit_set_initial_prompt();
#وإلا
	PS1 = عدم;
#endif
	PS2 = "> ";

	/* initialize our shell local variables with the values
	 * currently living in the environment */
	إذا (e) {
		لكل (; *e; e++)
			set_local_var(*e, 2);   /* without call putenv() */
	}

	last_return_code=EXIT_SUCCESS;

	إذا (argv[0] && argv[0][0] == '-') {
		debug_printf("\nsourcing /etc/profile\n");
		إذا ((ماسوقن = fopen("/etc/profile", "r")) != عدم) {
			mark_open(fileno(ماسوقن));
			parse_file_outer(ماسوقن);
			mark_closed(fileno(ماسوقن));
			fclose(ماسوقن);
		}
	}
	ماسوقن=stdin;

	طالما ((opt = getopt(argc, argv, "c:xif")) > 0) {
		بدل (opt) {
			حالة 'c':
				{
					global_argv = argv+optind;
					global_argc = argc-optind;
					opt = parse_string_outer(optarg, FLAG_PARSE_SEMICOLON);
					اذهب final_return;
				}
				اكسر;
			حالة 'i':
				interactive++;
				اكسر;
			حالة 'f':
				fake_mode++;
				اكسر;
			افتراضي:
#ifndef BB_VER
				fprintf(stderr, "Usage: sh [FILE]...\n"
						"   or: sh -c command [args]...\n\n");
				exit(EXIT_FAILURE);
#وإلا
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
	إذا (argv[optind]==عدم && ماسوقن==stdin &&
			isatty(fileno(stdin)) && isatty(fileno(stdout))) {
		interactive++;
	}

	debug_printf("\ninteractive=%d\n", interactive);
	إذا (interactive) {
		/* Looks like they want an interactive shell */
#ifndef CONFIG_FEATURE_SH_EXTRA_QUIET
		printf( "\n\n" BB_BANNER " hush - the humble shell v0.01 (testing)\n");
		printf( "Enter 'help' for a list of built-in commands.\n\n");
#endif
		setup_job_control();
	}

	إذا (argv[optind]==عدم) {
		opt=parse_file_outer(stdin);
		اذهب final_return;
	}

	debug_printf("\nrunning script '%s'\n", argv[optind]);
	global_argv = argv+optind;
	global_argc = argc-optind;
	ماسوقن = xfopen(argv[optind], "r");
	opt = parse_file_outer(ماسوقن);

#ifdef CONFIG_FEATURE_CLEAN_UP
	fclose(ماسوقن);
	إذا (cwd && cwd != unknown)
		حرر((حرف*)cwd);
	{
		بنية variables *cur, *tmp;
		لكل(cur = اتس_vars; cur; cur = tmp) {
			tmp = cur->next;
			إذا (!cur->flg_read_only) {
				حرر(cur->name);
				حرر(cur->نيلاي);
				حرر(cur);
			}
		}
	}
#endif

final_return:
	أرجع(opt?opt:last_return_code);
}
#endif

ساكن حرف *سيسيڤ_var_نيلاي(حرف *inp)
{
	أرجع سيسيڤ_var_نيلاي_sub(inp, 0);
}

ساكن حرف *سيسيڤ_var_نيلاي_sub(حرف *inp, صحيح tag_subst)
{
	صحيح res_str_len = 0;
	صحيح len;
	صحيح done = 0;
	حرف *p, *p1, *res_str = عدم;

	طالما ((p = strchr(inp, SPECIAL_VAR_سيمبول))) {
		/* check the beginning of the string for normal characters */
		إذا (p != inp) {
			/* copy any characters to the result string */
			len = p - inp;
			res_str = xrealloc(res_str, (res_str_len + len));
			strncpy((res_str + res_str_len), inp, len);
			res_str_len += len;
		}
		inp = ++p;
		/* find the ending marker */
		p = strchr(inp, SPECIAL_VAR_سيمبول);
		*p = '\0';
		/* look up the value to substitute */
		إذا ((p1 = lookup_param(inp))) {
			إذا (tag_subst)
				len = res_str_len + strlen(p1) + 2;
			وإلا
				len = res_str_len + strlen(p1);
			res_str = xrealloc(res_str, (1 + len));
			إذا (tag_subst) {
				/*
				 * copy the variable value to the result
				 * string
				 */
				strcpy((res_str + res_str_len + 1), p1);

				/*
				 * mark the replaced text to be accepted as
				 * is
				 */
				res_str[res_str_len] = SUBSTED_VAR_سيمبول;
				res_str[res_str_len + 1 + strlen(p1)] =
					SUBSTED_VAR_سيمبول;
			} وإلا
				/*
				 * copy the variable value to the result
				 * string
				 */
				strcpy((res_str + res_str_len), p1);

			res_str_len = len;
		}
		*p = SPECIAL_VAR_سيمبول;
		inp = ++p;
		done = 1;
	}
	إذا (done) {
		res_str = xrealloc(res_str, (1 + res_str_len + strlen(inp)));
		strcpy((res_str + res_str_len), inp);
		طالما ((p = strchr(res_str, '\n'))) {
			*p = ' ';
		}
	}
	أرجع (res_str == عدم) ? inp : res_str;
}

ساكن حرف **make_list_in(حرف **inp, حرف *name)
{
	صحيح len, i;
	صحيح name_len = strlen(name);
	صحيح n = 0;
	حرف **list;
	حرف *p1, *p2, *p3;

	/* create list of variable values */
	list = xmalloc(حجم(*list));
	لكل (i = 0; inp[i]; i++) {
		p3 = سيسيڤ_var_نيلاي(inp[i]);
		p1 = p3;
		طالما (*p1) {
			إذا (*p1 == ' ') {
				p1++;
				استمر;
			}
			إذا ((p2 = strchr(p1, ' '))) {
				len = p2 - p1;
			} وإلا {
				len = strlen(p1);
				p2 = p1 + len;
			}
			/* we use n + 2 in realloc for list,because we add
			 * new element and then we will add NULL element */
			list = xrealloc(list, حجم(*list) * (n + 2));
			list[n] = xmalloc(2 + name_len + len);
			strcpy(list[n], name);
			strcat(list[n], "=");
			strncat(list[n], p1, len);
			list[n++][name_len + len + 1] = '\0';
			p1 = p2;
		}
		إذا (p3 != inp[i]) حرر(p3);
	}
	list[n] = عدم;
	أرجع list;
}

/*
 * Make new string for parser
 * inp     - array of argument strings to flatten
 * nonnull - indicates argument was quoted when originally parsed
 */
ساكن حرف *make_string(حرف **inp, صحيح *nonnull)
{
	حرف *p;
	حرف *str = عدم;
	صحيح n;
	صحيح len = 2;
	حرف *noeval_str;
	صحيح noeval = 0;

	noeval_str = get_local_var("HUSH_NO_EVAL");
	إذا (noeval_str != عدم && *noeval_str != '0' && *noeval_str != '\0')
		noeval = 1;
	لكل (n = 0; inp[n]; n++) {
		p = سيسيڤ_var_نيلاي_sub(inp[n], noeval);
		حرف *new_str = xrealloc(str, (len + strlen(p) + (2 * nonnull[n])));
		إذا (!new_str) {
			حرر(str);
			إذا (p != inp[n]) حرر(p);
			أرجع عدم;
		}
		str = new_str;
		إذا (n) {
			strcat(str, " ");
		} وإلا {
			*str = '\0';
		}
		إذا (nonnull[n])
			strcat(str, "'");
		strcat(str, p);
		إذا (nonnull[n])
			strcat(str, "'");
		len = strlen(str) + 3;
		إذا (p != inp[n]) حرر(p);
	}
	len = strlen(str);
	*(str + len) = '\n';
	*(str + len + 1) = '\0';
	أرجع str;
}

#ifdef __U_BOOT__
ساكن صحيح do_showvar(بنية cmd_tbl *cmdtp, صحيح flag, صحيح argc,
		      حرف *ثابت argv[])
{
	صحيح i, k;
	صحيح rcode = 0;
	بنية variables *cur;

	إذا (argc == 1) {		/* Print all env variables	*/
		لكل (cur = اتس_vars; cur; cur = cur->next) {
			printf ("%s=%s\n", cur->name, cur->نيلاي);
			إذا (ctrlc ()) {
				puts ("\n ** Abort\n");
				أرجع 1;
			}
		}
		أرجع 0;
	}
	لكل (i = 1; i < argc; ++i) {	/* print single env variables	*/
		حرف *name = argv[i];

		k = -1;
		لكل (cur = اتس_vars; cur; cur = cur->next) {
			إذا(strcmp (cur->name, name) == 0) {
				k = 0;
				printf ("%s=%s\n", cur->name, cur->نيلاي);
			}
			إذا (ctrlc ()) {
				puts ("\n ** Abort\n");
				أرجع 1;
			}
		}
		إذا (k < 0) {
			printf ("## Error: \"%s\" not defined\n", name);
			rcode ++;
		}
	}
	أرجع rcode;
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
