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
#شامل <env.h>
#شامل <malloc.h>         /* malloc, free, realloc*/
#شامل <linux/ctype.h>    /* isalpha, isdigit */
#شامل <console.h>
#شامل <bootretry.h>
#شامل <cli.h>
#شامل <cli_hush.h>
#شامل <command.h>        /* find_cmd */
#شامل <vsprintf.h>
#endif
#ifndef __U_BOOT__
#شامل <ctype.h>     /* isalpha, isdigit */
#شامل <unistd.h>    /* getpid */
#شامل <stdlib.h>    /* getenv, atoi */
#شامل <string.h>    /* strchr */
#شامل <stdio.h>     /* popen etc. */
#شامل <glob.h>      /* glob, of course */
#شامل <stdarg.h>    /* va_list */
#شامل <errno.h>
#شامل <fcntl.h>
#شامل <getopt.h>    /* should be pretty obvious */

#شامل <sys/stat.h>  /* ulimit */
#شامل <sys/types.h>
#شامل <sys/wait.h>
#شامل <signal.h>

/* #include <dmalloc.h> */

#اگر 1
#شامل "busybox.h"
#شامل "cmdedit.h"
#وگرنه
#define applet_name "hush"
#شامل "standalone.h"
#define hush_main اصلی
#define BB_BANNER
#endif
#endif
#define SPECIAL_VAR_نماد 03
#define SUBSTED_VAR_نماد 04
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
#define error_پیام printf
#وگرنه
تعریفنوع شمارش {
	REDIRECT_درون_داد     = 1,
	REDIRECT_OVERWRITE = 2,
	REDIRECT_APPEND    = 3,
	REDIRECT_HEREIS    = 4,
	REDIRECT_IO        = 5
} redir_type;

/* The descrip member of this structure is only used to make debugging
 * output pretty */
ساختار {صحیح mode; صحیح default_fd; نویسه *descrip;} redir_table[] = {
	{ 0,                         0, "()" },
	{ O_RDONLY,                  0, "<"  },
	{ O_CREAT|O_TRUNC|O_WRONLY,  1, ">"  },
	{ O_CREAT|O_APPEND|O_WRONLY, 1, ">>" },
	{ O_RDONLY,                 -1, "<<" },
	{ O_RDWR,                    1, "<>" }
};
#endif

تعریفنوع شمارش {
	PIPE_SEQ = 1,
	PIPE_AND = 2,
	PIPE_OR  = 3,
	PIPE_BG  = 4,
} pipe_style;

/* might eventually control execution */
تعریفنوع شمارش {
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
ساختار p_context {
	ساختار child_prog *child;
	ساختار pipe *list_head;
	ساختار pipe *pipe;
#ifndef __U_BOOT__
	ساختار redir_struct *pending_redirect;
#endif
	reserved_style w;
	صحیح old_flag;				/* for figuring out valid reserved words */
	ساختار p_context *پشته;
	صحیح type;			/* define type of parser : ";$" common or special symbol */
	/* How about quoting status? */
};

#ifndef __U_BOOT__
ساختار redir_struct {
	redir_type type;			/* type of redirection */
	صحیح fd;						/* file descriptor being redirected */
	صحیح dup;					/* -1, or file descriptor being duplicated */
	ساختار redir_struct *next;	/* pointer to the next redirect in the list */
	glob_t word;				/* *word.gl_pathv is the filename */
};
#endif

ساختار child_prog {
#ifndef __U_BOOT__
	pid_t pid;					/* 0 if exited */
#endif
	نویسه **argv;				/* program name and arguments */
	/* was quoted when parsed; copy of struct o_string.nonnull field */
	صحیح *argv_nonnull;
#ifdef __U_BOOT__
	صحیح    argc;                            /* number of program arguments */
#endif
	ساختار pipe *group;			/* if non-NULL, first in group or subshell */
#ifndef __U_BOOT__
	صحیح subshell;				/* flag, non-zero if group must be forked */
	ساختار redir_struct *redirects;	/* I/O redirections */
	glob_t glob_result;			/* result of parameter globbing */
	صحیح is_stopped;				/* is the program currently running? */
	ساختار pipe *family;		/* pointer back to the child's parent pipe */
#endif
	صحیح sp;				/* number of SPECIAL_VAR_SYMBOL */
	صحیح type;
};

ساختار pipe {
#ifndef __U_BOOT__
	صحیح jobid;					/* job number */
#endif
	صحیح رقم_progs;				/* total number of programs in job */
#ifndef __U_BOOT__
	صحیح running_progs;			/* number of programs running */
	نویسه *text;					/* name of job */
	نویسه *cmdbuf;				/* buffer various argv's point into */
	pid_t pgrp;					/* process group ID for the job */
#endif
	ساختار child_prog *progs;	/* array of commands in pipe */
	ساختار pipe *next;			/* to track background commands */
#ifndef __U_BOOT__
	صحیح stopped_progs;			/* number of programs alive, but stopped */
	صحیح job_context;			/* bitmask defining current context */
#endif
	pipe_style followup;		/* PIPE_BG, PIPE_SEQ, PIPE_OR, PIPE_AND */
	reserved_style r_mode;		/* supports if, for, while, until */
};

#ifndef __U_BOOT__
ساختار close_me {
	صحیح fd;
	ساختار close_me *next;
};
#endif

ساختار variables {
	نویسه *name;
	نویسه *مقدار;
	صحیح flg_export;
	صحیح flg_read_only;
	ساختار variables *next;
};

/* globals, connect us to the outside world
 * the first three support $?, $#, and $1 */
#ifndef __U_BOOT__
نویسه **global_argv;
unsigned صحیح global_argc;
#endif
ایستا unsigned صحیح last_return_code;
#ifndef __U_BOOT__
بیرونی نویسه **environ; /* This is in <unistd.h>, but protected with __USE_GNU */
#endif

/* "globals" within this file */
ایستا uchar *ifs;
ایستا نویسه map[256];
#ifndef __U_BOOT__
ایستا صحیح fake_mode;
ایستا صحیح interactive;
ایستا ساختار close_me *close_me_head;
ایستا ثابت نویسه *cwd;
ایستا ساختار pipe *job_list;
ایستا unsigned صحیح last_bg_pid;
ایستا unsigned صحیح last_jobid;
ایستا unsigned صحیح shell_terminal;
ایستا نویسه *PS1;
ایستا نویسه *PS2;
ساختار variables shell_ver = { "HUSH_VERSION", "0.01", 1, 1, 0 };
ساختار variables *بالاترین_vars = &shell_ver;
#وگرنه
ایستا صحیح flag_repeat = 0;
ایستا صحیح do_repeat = 0;
ایستا ساختار variables *بالاترین_vars = پوچ ;
#endif /*__U_BOOT__ */

#define B_CHUNK (100)
#define B_NOSPAC 1

تعریفنوع ساختار {
	نویسه *data;
	صحیح length;
	صحیح maxlen;
	صحیح quote;
	صحیح nonnull;
} o_string;
#define NULL_O_STRING {پوچ,0,0,0,0}
/* used for initialization:
	o_string foo = NULL_O_STRING; */

/* I can almost use ordinary FILE *.  Is open_memstream() universally
 * available?  Where is it documented? */
ساختار in_str {
	ثابت unsigned نویسه *p;
#ifndef __U_BOOT__
	نویسه peek_buf[2];
#endif
	صحیح __promptme;
	صحیح promptmode;
#ifndef __U_BOOT__
	FILE *file;
#endif
	صحیح (*get) (ساختار in_str *);
	صحیح (*peek) (ساختار in_str *);
};
#define b_getch(درون_داد) ((درون_داد)->get(درون_داد))
#define b_peek(درون_داد) ((درون_داد)->peek(درون_داد))

#ifndef __U_BOOT__
#define JOB_STATUS_FORMAT "[%d] %-22s %.40s\n"

ساختار built_in_command {
	نویسه *cmd;					/* name */
	نویسه *descr;				/* description */
	صحیح (*function) (ساختار child_prog *);	/* function ptr */
};
#endif

/* define DEBUG_SHELL for debugging output (obviously ;-)) */
#اگر 0
#define DEBUG_SHELL
#endif

/* This should be in utility.c */
#ifdef DEBUG_SHELL
#ifndef __U_BOOT__
ایستا تهی debug_printf(ثابت نویسه *format, ...)
{
	va_list args;
	va_start(args, format);
	vfprintf(stderr, format, args);
	va_end(args);
}
#وگرنه
#define debug_printf(fmt,args...)	printf (fmt ,##args)
#endif
#وگرنه
ایستا درونخطی تهی debug_printf(ثابت نویسه *format, ...) { }
#endif
#define final_printf debug_printf

#ifdef __U_BOOT__
ایستا تهی syntax_err(تهی) {
	 printf("syntax error\n");
}
#وگرنه
ایستا تهی __syntax(نویسه *file, صحیح سطر) {
	error_پیام("syntax error %s:%d", file, سطر);
}
#define syntax() __syntax(__FILE__, __LINE__)
#endif

#ifdef __U_BOOT__
ایستا تهی *xmalloc(size_t size);
ایستا تهی *xrealloc(تهی *ptr, size_t size);
#وگرنه
/* Index of subroutines: */
/*   function prototypes for builtins */
ایستا صحیح builtin_cd(ساختار child_prog *child);
ایستا صحیح builtin_env(ساختار child_prog *child);
ایستا صحیح builtin_eval(ساختار child_prog *child);
ایستا صحیح builtin_exec(ساختار child_prog *child);
ایستا صحیح builtin_exit(ساختار child_prog *child);
ایستا صحیح builtin_export(ساختار child_prog *child);
ایستا صحیح builtin_fg_bg(ساختار child_prog *child);
ایستا صحیح builtin_help(ساختار child_prog *child);
ایستا صحیح builtin_jobs(ساختار child_prog *child);
ایستا صحیح builtin_pwd(ساختار child_prog *child);
ایستا صحیح builtin_read(ساختار child_prog *child);
ایستا صحیح builtin_set(ساختار child_prog *child);
ایستا صحیح builtin_shift(ساختار child_prog *child);
ایستا صحیح builtin_source(ساختار child_prog *child);
ایستا صحیح builtin_umask(ساختار child_prog *child);
ایستا صحیح builtin_unset(ساختار child_prog *child);
ایستا صحیح builtin_not_written(ساختار child_prog *child);
#endif
/*   o_string manipulation: */
ایستا صحیح b_check_space(o_string *o, صحیح len);
ایستا صحیح b_addchr(o_string *o, صحیح ch);
ایستا تهی b_reset(o_string *o);
ایستا صحیح b_addqchr(o_string *o, صحیح ch, صحیح quote);
#ifndef __U_BOOT__
ایستا صحیح b_adduint(o_string *o, unsigned صحیح i);
#endif
/*  in_str manipulations: */
ایستا صحیح static_get(ساختار in_str *i);
ایستا صحیح static_peek(ساختار in_str *i);
ایستا صحیح file_get(ساختار in_str *i);
ایستا صحیح file_peek(ساختار in_str *i);
#ifndef __U_BOOT__
ایستا تهی setup_file_in_str(ساختار in_str *i, FILE *f);
#وگرنه
ایستا تهی setup_file_in_str(ساختار in_str *i);
#endif
ایستا تهی setup_string_in_str(ساختار in_str *i, ثابت نویسه *s);
#ifndef __U_BOOT__
/*  close_me manipulations: */
ایستا تهی mark_open(صحیح fd);
ایستا تهی mark_closed(صحیح fd);
ایستا تهی close_all(تهی);
#endif
/*  "run" the final data structures: */
ایستا نویسه *indenter(صحیح i);
ایستا صحیح free_pipe_list(ساختار pipe *head, صحیح indent);
ایستا صحیح free_pipe(ساختار pipe *pi, صحیح indent);
/*  really run the final data structures: */
#ifndef __U_BOOT__
ایستا صحیح setup_redirects(ساختار child_prog *prog, صحیح squirrel[]);
#endif
ایستا صحیح run_list_real(ساختار pipe *pi);
#ifndef __U_BOOT__
ایستا تهی pseudo_exec(ساختار child_prog *child) __attribute__ ((noreturn));
#endif
ایستا صحیح run_pipe_real(ساختار pipe *pi);
/*   extended glob support: */
#ifndef __U_BOOT__
ایستا صحیح globhack(ثابت نویسه *src, صحیح flags, glob_t *pglob);
ایستا صحیح glob_needed(ثابت نویسه *s);
ایستا صحیح xglob(o_string *dest, صحیح flags, glob_t *pglob);
#endif
/*   variable assignment: */
ایستا صحیح is_assignment(ثابت نویسه *s);
/*   data structure manipulation: */
#ifndef __U_BOOT__
ایستا صحیح setup_redirect(ساختار p_context *ctx, صحیح fd, redir_type style, ساختار in_str *درون_داد);
#endif
ایستا تهی initialize_context(ساختار p_context *ctx);
ایستا صحیح done_word(o_string *dest, ساختار p_context *ctx);
ایستا صحیح done_command(ساختار p_context *ctx);
ایستا صحیح done_pipe(ساختار p_context *ctx, pipe_style type);
/*   primary string parsing: */
#ifndef __U_BOOT__
ایستا صحیح redirect_dup_رقم(ساختار in_str *درون_داد);
ایستا صحیح redirect_opt_رقم(o_string *o);
ایستا صحیح process_command_subs(o_string *dest, ساختار p_context *ctx, ساختار in_str *درون_داد, صحیح subst_end);
ایستا صحیح parse_group(o_string *dest, ساختار p_context *ctx, ساختار in_str *درون_داد, صحیح ch);
#endif
ایستا نویسه *lookup_param(نویسه *src);
ایستا نویسه *make_string(نویسه **inp, صحیح *nonnull);
ایستا صحیح handle_dollar(o_string *dest, ساختار p_context *ctx, ساختار in_str *درون_داد);
#ifndef __U_BOOT__
ایستا صحیح parse_string(o_string *dest, ساختار p_context *ctx, ثابت نویسه *src);
#endif
ایستا صحیح parse_stream(o_string *dest, ساختار p_context *ctx, ساختار in_str *input0, صحیح end_trigger);
/*   setup: */
ایستا صحیح parse_stream_outer(ساختار in_str *inp, صحیح flag);
#ifndef __U_BOOT__
ایستا صحیح parse_string_outer(ثابت نویسه *s, صحیح flag);
ایستا صحیح parse_file_outer(FILE *f);
#endif
#ifndef __U_BOOT__
/*   job management: */
ایستا صحیح checkjobs(ساختار pipe* fg_pipe);
ایستا تهی درج_bg_job(ساختار pipe *pi);
ایستا تهی remove_bg_job(ساختار pipe *pi);
#endif
/*     local variable support */
ایستا نویسه **make_list_in(نویسه **inp, نویسه *name);
ایستا نویسه *درج_var_مقدار(نویسه *inp);
ایستا نویسه *درج_var_مقدار_sub(نویسه *inp, صحیح tag_subst);

#ifndef __U_BOOT__
/* Table of built-in functions.  They can be forked or not, depending on
 * context: within pipes, they fork.  As simple commands, they do not.
 * When used in non-forking context, they can change global variables
 * in the parent shell process.  If forked, of course they can not.
 * For example, 'unset foo | whatever' will parse and run, but foo will
 * still be set at the end. */
ایستا ساختار built_in_command bltins[] = {
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
	{پوچ, پوچ, پوچ}
};

ایستا ثابت نویسه *set_cwd(تهی)
{
	اگر(cwd==unknown)
		cwd = پوچ;     /* xgetcwd(arg) called free(arg) */
	cwd = xgetcwd((نویسه *)cwd);
	اگر (!cwd)
		cwd = unknown;
	بازگشت cwd;
}

/* built-in 'eval' handler */
ایستا صحیح builtin_eval(ساختار child_prog *child)
{
	نویسه *str = پوچ;
	صحیح rcode = EXIT_SUCCESS;

	اگر (child->argv[1]) {
		str = make_string(child->argv + 1);
		parse_string_outer(str, FLAG_EXIT_FROM_LOOP |
					FLAG_PARSE_SEMICOLON);
		آزاد(str);
		rcode = last_return_code;
	}
	بازگشت rcode;
}

/* built-in 'cd <path>' handler */
ایستا صحیح builtin_cd(ساختار child_prog *child)
{
	نویسه *newdir;
	اگر (child->argv[1] == پوچ)
		newdir = env_get("HOME");
	وگرنه
		newdir = child->argv[1];
	اگر (chdir(newdir)) {
		printf("cd: %s: %s\n", newdir, strerror(errno));
		بازگشت EXIT_FAILURE;
	}
	set_cwd();
	بازگشت EXIT_SUCCESS;
}

/* built-in 'env' handler */
ایستا صحیح builtin_env(ساختار child_prog *dummy)
{
	نویسه **e = environ;
	اگر (e == پوچ) بازگشت EXIT_FAILURE;
	برای (; *e; e++) {
		puts(*e);
	}
	بازگشت EXIT_SUCCESS;
}

/* built-in 'exec' handler */
ایستا صحیح builtin_exec(ساختار child_prog *child)
{
	اگر (child->argv[1] == پوچ)
		بازگشت EXIT_SUCCESS;   /* Really? */
	child->argv++;
	pseudo_exec(child);
	/* never returns */
}

/* built-in 'exit' handler */
ایستا صحیح builtin_exit(ساختار child_prog *child)
{
	اگر (child->argv[1] == پوچ)
		exit(last_return_code);
	exit (atoi(child->argv[1]));
}

/* built-in 'export VAR=value' handler */
ایستا صحیح builtin_export(ساختار child_prog *child)
{
	صحیح res = 0;
	نویسه *name = child->argv[1];

	اگر (name == پوچ) {
		بازگشت (builtin_env(child));
	}

	name = strdup(name);

	اگر(name) {
		نویسه *مقدار = strchr(name, '=');

		اگر (!مقدار) {
			نویسه *tmp;
			/* They are exporting something without an =VALUE */

			مقدار = get_local_var(name);
			اگر (مقدار) {
				size_t ln = strlen(name);

				tmp = realloc(name, ln+strlen(مقدار)+2);
				اگر(tmp==پوچ)
					res = -1;
				وگرنه {
					sprintf(tmp+ln, "=%s", مقدار);
					name = tmp;
				}
			} وگرنه {
				/* bash does not return an error when trying to export
				 * an undefined variable.  Do likewise. */
				res = 1;
			}
		}
	}
	اگر (res<0)
		perror_پیام("export");
	وگرنه اگر(res==0)
		res = set_local_var(name, 1);
	وگرنه
		res = 0;
	آزاد(name);
	بازگشت res;
}

/* built-in 'fg' and 'bg' handler */
ایستا صحیح builtin_fg_bg(ساختار child_prog *child)
{
	صحیح i, jobnum;
	ساختار pipe *pi=پوچ;

	اگر (!interactive)
		بازگشت EXIT_FAILURE;
	/* If they gave us no args, assume they want the last backgrounded task */
	اگر (!child->argv[1]) {
		برای (pi = job_list; pi; pi = pi->next) {
			اگر (pi->jobid == last_jobid) {
				بشکن;
			}
		}
		اگر (!pi) {
			error_پیام("%s: no current job", child->argv[0]);
			بازگشت EXIT_FAILURE;
		}
	} وگرنه {
		اگر (sscanf(child->argv[1], "%%%d", &jobnum) != 1) {
			error_پیام("%s: bad argument '%s'", child->argv[0], child->argv[1]);
			بازگشت EXIT_FAILURE;
		}
		برای (pi = job_list; pi; pi = pi->next) {
			اگر (pi->jobid == jobnum) {
				بشکن;
			}
		}
		اگر (!pi) {
			error_پیام("%s: %d: no such job", child->argv[0], jobnum);
			بازگشت EXIT_FAILURE;
		}
	}

	اگر (*child->argv[0] == 'f') {
		/* Put the job into the foreground.  */
		tcsetpgrp(shell_terminal, pi->pgrp);
	}

	/* Restart the processes in the job */
	برای (i = 0; i < pi->رقم_progs; i++)
		pi->progs[i].is_stopped = 0;

	اگر ( (i=kill(- pi->pgrp, SIGCONT)) < 0) {
		اگر (i == ESRCH) {
			remove_bg_job(pi);
		} وگرنه {
			perror_پیام("kill (SIGCONT)");
		}
	}

	pi->stopped_progs = 0;
	بازگشت EXIT_SUCCESS;
}

/* built-in 'help' handler */
ایستا صحیح builtin_help(ساختار child_prog *dummy)
{
	ساختار built_in_command *x;

	printf("\nBuilt-in commands:\n");
	printf("-------------------\n");
	برای (x = bltins; x->cmd; x++) {
		اگر (x->descr==پوچ)
			ادامه;
		printf("%s\t%s\n", x->cmd, x->descr);
	}
	printf("\n\n");
	بازگشت EXIT_SUCCESS;
}

/* built-in 'jobs' handler */
ایستا صحیح builtin_jobs(ساختار child_prog *child)
{
	ساختار pipe *job;
	نویسه *status_string;

	برای (job = job_list; job; job = job->next) {
		اگر (job->running_progs == job->stopped_progs)
			status_string = "Stopped";
		وگرنه
			status_string = "Running";

		printf(JOB_STATUS_FORMAT, job->jobid, status_string, job->text);
	}
	بازگشت EXIT_SUCCESS;
}

/* built-in 'pwd' handler */
ایستا صحیح builtin_pwd(ساختار child_prog *dummy)
{
	puts(set_cwd());
	بازگشت EXIT_SUCCESS;
}

/* built-in 'read VAR' handler */
ایستا صحیح builtin_read(ساختار child_prog *child)
{
	صحیح res;

	اگر (child->argv[1]) {
		نویسه string[BUFSIZ];
		نویسه *var = 0;

		string[0] = 0;  /* In case stdin has only EOF */
		/* read string */
		fgets(string, اندازه(string), stdin);
		chomp(string);
		var = malloc(strlen(child->argv[1])+strlen(string)+2);
		اگر(var) {
			sprintf(var, "%s=%s", child->argv[1], string);
			res = set_local_var(var, 0);
		} وگرنه
			res = -1;
		اگر (res)
			fprintf(stderr, "read: %m\n");
		آزاد(var);      /* So not move up to avoid breaking errno */
		بازگشت res;
	} وگرنه {
		انجامبده res=getchar(); وقتی(res!='\n' && res!=EOF);
		بازگشت 0;
	}
}

/* built-in 'set VAR=value' handler */
ایستا صحیح builtin_set(ساختار child_prog *child)
{
	نویسه *temp = child->argv[1];
	ساختار variables *e;

	اگر (temp == پوچ)
		برای(e = بالاترین_vars; e; e=e->next)
			printf("%s=%s\n", e->name, e->مقدار);
	وگرنه
		set_local_var(temp, 0);

		بازگشت EXIT_SUCCESS;
}

/* Built-in 'shift' handler */
ایستا صحیح builtin_shift(ساختار child_prog *child)
{
	صحیح n=1;
	اگر (child->argv[1]) {
		n=atoi(child->argv[1]);
	}
	اگر (n>=0 && n<global_argc) {
		/* XXX This probably breaks $0 */
		global_argc -= n;
		global_argv += n;
		بازگشت EXIT_SUCCESS;
	} وگرنه {
		بازگشت EXIT_FAILURE;
	}
}

/* Built-in '.' handler (read-in and execute commands from file) */
ایستا صحیح builtin_source(ساختار child_prog *child)
{
	FILE *درون_داد;
	صحیح status;

	اگر (child->argv[1] == پوچ)
		بازگشت EXIT_FAILURE;

	/* XXX search through $PATH is missing */
	درون_داد = fopen(child->argv[1], "r");
	اگر (!درون_داد) {
		error_پیام("Couldn't open file '%s'", child->argv[1]);
		بازگشت EXIT_FAILURE;
	}

	/* Now run the file */
	/* XXX argv and argc are broken; need to save old global_argv
	 * (pointer only is OK!) on this stack frame,
	 * set global_argv=child->argv+1, recurse, and restore. */
	mark_open(fileno(درون_داد));
	status = parse_file_outer(درون_داد);
	mark_closed(fileno(درون_داد));
	fclose(درون_داد);
	بازگشت (status);
}

ایستا صحیح builtin_umask(ساختار child_prog *child)
{
	mode_t new_umask;
	ثابت نویسه *arg = child->argv[1];
	نویسه *end;
	اگر (arg) {
		new_umask=strtoul(arg, &end, 8);
		اگر (*end!='\0' || end == arg) {
			بازگشت EXIT_FAILURE;
		}
	} وگرنه {
		printf("%.3o\n", (unsigned صحیح) (new_umask=umask(0)));
	}
	umask(new_umask);
	بازگشت EXIT_SUCCESS;
}

/* built-in 'unset VAR' handler */
ایستا صحیح builtin_unset(ساختار child_prog *child)
{
	/* bash returned already true */
	unset_local_var(child->argv[1]);
	بازگشت EXIT_SUCCESS;
}

ایستا صحیح builtin_not_written(ساختار child_prog *child)
{
	printf("builtin_%s not written\n",child->argv[0]);
	بازگشت EXIT_FAILURE;
}
#endif

ایستا صحیح b_check_space(o_string *o, صحیح len)
{
	/* It would be easy to drop a more restrictive policy
	 * in here, such as setting a maximum string length */
	اگر (o->length + len > o->maxlen) {
		نویسه *old_data = o->data;
		/* assert (data == NULL || o->maxlen != 0); */
		o->maxlen += max(2*len, B_CHUNK);
		o->data = realloc(o->data, 1 + o->maxlen);
		اگر (o->data == پوچ) {
			آزاد(old_data);
		}
	}
	بازگشت o->data == پوچ;
}

ایستا صحیح b_addchr(o_string *o, صحیح ch)
{
	debug_printf("b_addchr: %c %d %p\n", ch, o->length, o);
	اگر (b_check_space(o, 1)) بازگشت B_NOSPAC;
	o->data[o->length] = ch;
	o->length++;
	o->data[o->length] = '\0';
	بازگشت 0;
}

ایستا تهی b_reset(o_string *o)
{
	o->length = 0;
	o->nonnull = 0;
	اگر (o->data != پوچ) *o->data = '\0';
}

ایستا تهی b_free(o_string *o)
{
	b_reset(o);
	آزاد(o->data);
	o->data = پوچ;
	o->maxlen = 0;
}

/* My analysis of quoting semantics tells me that state information
 * is associated with a destination, not a source.
 */
ایستا صحیح b_addqchr(o_string *o, صحیح ch, صحیح quote)
{
	اگر (quote && strchr("*?[\\",ch)) {
		صحیح rc;
		rc = b_addchr(o, '\\');
		اگر (rc) بازگشت rc;
	}
	بازگشت b_addchr(o, ch);
}

#ifndef __U_BOOT__
ایستا صحیح b_adduint(o_string *o, unsigned صحیح i)
{
	صحیح r;
	نویسه *p = simple_itoa(i);
	/* no escape checking necessary */
	انجامبده r=b_addchr(o, *p++); وقتی (r==0 && *p);
	بازگشت r;
}
#endif

ایستا صحیح static_get(ساختار in_str *i)
{
	صحیح ch = *i->p++;
	اگر (ch=='\0') بازگشت EOF;
	بازگشت ch;
}

ایستا صحیح static_peek(ساختار in_str *i)
{
	بازگشت *i->p;
}

#ifndef __U_BOOT__
ایستا درونخطی تهی cmdedit_set_initial_prompt(تهی)
{
#ifndef CONFIG_FEATURE_SH_FANCY_PROMPT
	PS1 = پوچ;
#وگرنه
	PS1 = env_get("PS1");
	اگر(PS1==0)
		PS1 = "\\w \\$ ";
#endif
}

ایستا درونخطی تهی setup_prompt_string(صحیح promptmode, نویسه **prompt_str)
{
	debug_printf("setup_prompt_string %d ",promptmode);
#ifndef CONFIG_FEATURE_SH_FANCY_PROMPT
	/* Set up the prompt */
	اگر (promptmode == 1) {
		آزاد(PS1);
		PS1=xmalloc(strlen(cwd)+4);
		sprintf(PS1, "%s %s", cwd, ( geteuid() != 0 ) ?  "$ ":"# ");
		*prompt_str = PS1;
	} وگرنه {
		*prompt_str = PS2;
	}
#وگرنه
	*prompt_str = (promptmode==1)? PS1 : PS2;
#endif
	debug_printf("result %s\n",*prompt_str);
}
#endif

#ifdef __U_BOOT__
ایستا صحیح uboot_cli_readline(ساختار in_str *i)
{
	نویسه *prompt;
	نویسه __maybe_unused *ps_prompt = پوچ;

	اگر (i->promptmode == 1)
		prompt = CONFIG_SYS_PROMPT;
	وگرنه
		prompt = CONFIG_SYS_PROMPT_HUSH_PS2;

#ifdef CONFIG_CMDLINE_PS_SUPPORT
	اگر (i->promptmode == 1)
		ps_prompt = env_get("PS1");
	وگرنه
		ps_prompt = env_get("PS2");
	اگر (ps_prompt)
		prompt = ps_prompt;
#endif

	بازگشت cli_readline(prompt);
}
#endif

ایستا تهی get_user_درون_داد(ساختار in_str *i)
{
#ifndef __U_BOOT__
	نویسه *prompt_str;
	ایستا نویسه the_command[BUFSIZ];

	setup_prompt_string(i->promptmode, &prompt_str);
#ifdef CONFIG_FEATURE_COMMAND_EDITING
	/*
	 ** enable command line editing only while a command line
	 ** is actually being read; otherwise, we'll end up bequeathing
	 ** atexit() handlers and other unwanted stuff to our
	 ** child processes (rob@sysgo.de)
	 */
	cmdedit_read_درون_داد(prompt_str, the_command);
#وگرنه
	fputs(prompt_str, stdout);
	fflush(stdout);
	the_command[0]=fgetc(i->file);
	the_command[1]='\0';
#endif
	fflush(stdout);
	i->p = the_command;
#وگرنه
	صحیح n;
	ایستا نویسه the_command[CONFIG_SYS_CBSIZE + 1];

	bootretry_reset_cmd_timeout();
	i->__promptme = 1;
	n = uboot_cli_readline(i);

#ifdef CONFIG_BOOT_RETRY_TIME
	اگر (n == -2) {
	  puts("\nTimeout waiting for command\n");
#  ifdef CONFIG_RESET_TO_RETRY
	  do_reset(پوچ, 0, 0, پوچ);
#  elif IS_ENABLED(CONFIG_RETRY_BOOTCMD)
	strcpy(console_buffer, "run bootcmd\n");
# وگرنه
#	error "This only works with CONFIG_RESET_TO_RETRY or CONFIG_BOOT_RETRY_COMMAND enabled"
#  endif
	}
#endif
	اگر (n == -1 ) {
		flag_repeat = 0;
		i->__promptme = 0;
	}
	n = strlen(console_buffer);
	console_buffer[n] = '\n';
	console_buffer[n+1]= '\0';
	اگر (had_ctrlc()) flag_repeat = 0;
	clear_ctrlc();
	do_repeat = 0;
	اگر (i->promptmode == 1) {
		اگر (console_buffer[0] == '\n'&& flag_repeat == 0) {
			strcpy(the_command,console_buffer);
		}
		وگرنه {
			اگر (console_buffer[0] != '\n') {
				strcpy(the_command,console_buffer);
				flag_repeat = 1;
			}
			وگرنه {
				do_repeat = 1;
			}
		}
		i->p = the_command;
	}
	وگرنه {
		اگر (console_buffer[0] != '\n') {
			اگر (strlen(the_command) + strlen(console_buffer)
			    < CONFIG_SYS_CBSIZE) {
				n = strlen(the_command);
				the_command[n-1] = ' ';
				strcpy(&the_command[n],console_buffer);
			}
			وگرنه {
				the_command[0] = '\n';
				the_command[1] = '\0';
				flag_repeat = 0;
			}
		}
		اگر (i->__promptme == 0) {
			the_command[0] = '\n';
			the_command[1] = '\0';
		}
		i->p = console_buffer;
	}
#endif
}

/* This is the magic location that prints prompts
 * and gets data back from the user */
ایستا صحیح file_get(ساختار in_str *i)
{
	صحیح ch;

	ch = 0;
	/* If there is data waiting, eat it up */
	اگر (i->p && *i->p) {
		ch = *i->p++;
	} وگرنه {
		/* need to double check i->file because we might be doing something
		 * more complicated by now, like sourcing or substituting. */
#ifndef __U_BOOT__
		اگر (i->__promptme && interactive && i->file == stdin) {
			وقتی(! i->p || (interactive && strlen(i->p)==0) ) {
#وگرنه
			وقتی(! i->p  || strlen(i->p)==0 ) {
#endif
				get_user_درون_داد(i);
			}
			i->promptmode=2;
#ifndef __U_BOOT__
			i->__promptme = 0;
#endif
			اگر (i->p && *i->p) {
				ch = *i->p++;
			}
#ifndef __U_BOOT__
		} وگرنه {
			ch = fgetc(i->file);
		}

#endif
		debug_printf("b_getch: got a %d\n", ch);
	}
#ifndef __U_BOOT__
	اگر (ch == '\n') i->__promptme=1;
#endif
	بازگشت ch;
}

/* All the callers guarantee this routine will never be
 * used right after a newline, so prompting is not needed.
 */
ایستا صحیح file_peek(ساختار in_str *i)
{
#ifndef __U_BOOT__
	اگر (i->p && *i->p) {
#endif
		بازگشت *i->p;
#ifndef __U_BOOT__
	} وگرنه {
		i->peek_buf[0] = fgetc(i->file);
		i->peek_buf[1] = '\0';
		i->p = i->peek_buf;
		debug_printf("b_peek: got a %d\n", *i->p);
		بازگشت *i->p;
	}
#endif
}

#ifndef __U_BOOT__
ایستا تهی setup_file_in_str(ساختار in_str *i, FILE *f)
#وگرنه
ایستا تهی setup_file_in_str(ساختار in_str *i)
#endif
{
	i->peek = file_peek;
	i->get = file_get;
	i->__promptme=1;
	i->promptmode=1;
#ifndef __U_BOOT__
	i->file = f;
#endif
	i->p = پوچ;
}

ایستا تهی setup_string_in_str(ساختار in_str *i, ثابت نویسه *s)
{
	i->peek = static_peek;
	i->get = static_get;
	i->__promptme=1;
	i->promptmode=1;
	i->p = s;
}

#ifndef __U_BOOT__
ایستا تهی mark_open(صحیح fd)
{
	ساختار close_me *new = xmalloc(اندازه(ساختار close_me));
	new->fd = fd;
	new->next = close_me_head;
	close_me_head = new;
}

ایستا تهی mark_closed(صحیح fd)
{
	ساختار close_me *tmp;
	اگر (close_me_head == پوچ || close_me_head->fd != fd)
		error_پیام_and_die("corrupt close_me");
	tmp = close_me_head;
	close_me_head = close_me_head->next;
	آزاد(tmp);
}

ایستا تهی close_all(تهی)
{
	ساختار close_me *c;
	برای (c=close_me_head; c; c=c->next) {
		close(c->fd);
	}
	close_me_head = پوچ;
}

/* squirrel != NULL means we squirrel away copies of stdin, stdout,
 * and stderr if they are redirected. */
ایستا صحیح setup_redirects(ساختار child_prog *prog, صحیح squirrel[])
{
	صحیح openfd, mode;
	ساختار redir_struct *redir;

	برای (redir=prog->redirects; redir; redir=redir->next) {
		اگر (redir->dup == -1 && redir->word.gl_pathv == پوچ) {
			/* something went wrong in the parse.  Pretend it didn't happen */
			ادامه;
		}
		اگر (redir->dup == -1) {
			mode=redir_table[redir->type].mode;
			openfd = open(redir->word.gl_pathv[0], mode, 0666);
			اگر (openfd < 0) {
			/* this could get lost if stderr has been redirected, but
			   bash and ash both lose it as well (though zsh doesn't!) */
				perror_پیام("error opening %s", redir->word.gl_pathv[0]);
				بازگشت 1;
			}
		} وگرنه {
			openfd = redir->dup;
		}

		اگر (openfd != redir->fd) {
			اگر (squirrel && redir->fd < 3) {
				squirrel[redir->fd] = dup(redir->fd);
			}
			اگر (openfd == -3) {
				close(openfd);
			} وگرنه {
				dup2(openfd, redir->fd);
				اگر (redir->dup == -1)
					close (openfd);
			}
		}
	}
	بازگشت 0;
}

ایستا تهی restore_redirects(صحیح squirrel[])
{
	صحیح i, fd;
	برای (i=0; i<3; i++) {
		fd = squirrel[i];
		اگر (fd != -1) {
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
ایستا تهی pseudo_exec(ساختار child_prog *child)
{
	صحیح i, rcode;
	نویسه *p;
	ساختار built_in_command *x;
	اگر (child->argv) {
		برای (i=0; is_assignment(child->argv[i]); i++) {
			debug_printf("pid %d environment modification: %s\n",getpid(),child->argv[i]);
			p = درج_var_مقدار(child->argv[i]);
			putenv(strdup(p));
			اگر (p != child->argv[i]) آزاد(p);
		}
		child->argv+=i;  /* XXX this hack isn't so horrible, since we are about
					to exit, and therefore don't need to keep data
					structures consistent for free() use. */
		/* If a variable is assigned in a forest, and nobody listens,
		 * was it ever really set?
		 */
		اگر (child->argv[0] == پوچ) {
			_exit(EXIT_SUCCESS);
		}

		/*
		 * Check if the command matches any of the builtins.
		 * Depending on context, this might be redundant.  But it's
		 * easier to waste a few CPU cycles than it is to figure out
		 * if this is one of those cases.
		 */
		برای (x = bltins; x->cmd; x++) {
			اگر (strcmp(child->argv[0], x->cmd) == 0 ) {
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
			صحیح argc_l;
			نویسه** argv_l=child->argv;
			نویسه *name = child->argv[0];

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
			برای(argc_l=0;*argv_l!=پوچ; argv_l++, argc_l++);
			optind = 1;
			debug_printf("running applet %s\n", name);
			run_applet_by_name(name, argc_l, child->argv);
		}
#endif
		debug_printf("exec of %s\n",child->argv[0]);
		execvp(child->argv[0],child->argv);
		perror_پیام("couldn't exec: %s",child->argv[0]);
		_exit(1);
	} وگرنه اگر (child->group) {
		debug_printf("runtime nesting to group\n");
		interactive=0;    /* crucial!!!! */
		rcode = run_list_real(child->group);
		/* OK to leak memory by not calling free_pipe_list,
		 * since this process is about to exit */
		_exit(rcode);
	} وگرنه {
		/* Can happen.  See what bash does with ">foo" by itself. */
		debug_printf("trying to pseudo_exec null command\n");
		_exit(EXIT_SUCCESS);
	}
}

ایستا تهی درج_bg_job(ساختار pipe *pi)
{
	ساختار pipe *thejob;

	/* Linear search for the ID of the job to use */
	pi->jobid = 1;
	برای (thejob = job_list; thejob; thejob = thejob->next)
		اگر (thejob->jobid >= pi->jobid)
			pi->jobid = thejob->jobid + 1;

	/* add thejob to the list of running jobs */
	اگر (!job_list) {
		thejob = job_list = xmalloc(اندازه(*thejob));
	} وگرنه {
		برای (thejob = job_list; thejob->next; thejob = thejob->next) /* nothing */;
		thejob->next = xmalloc(اندازه(*thejob));
		thejob = thejob->next;
	}

	/* physically copy the struct job */
	memcpy(thejob, pi, اندازه(ساختار pipe));
	thejob->next = پوچ;
	thejob->running_progs = thejob->رقم_progs;
	thejob->stopped_progs = 0;
	thejob->text = xmalloc(BUFSIZ); /* cmdedit buffer size */

	/*if (pi->progs[0] && pi->progs[0].argv && pi->progs[0].argv[0]) */
	{
		نویسه *bar=thejob->text;
		نویسه **foo=pi->progs[0].argv;
		وقتی(foo && *foo) {
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
ایستا تهی remove_bg_job(ساختار pipe *pi)
{
	ساختار pipe *prev_pipe;

	اگر (pi == job_list) {
		job_list = pi->next;
	} وگرنه {
		prev_pipe = job_list;
		وقتی (prev_pipe->next != pi)
			prev_pipe = prev_pipe->next;
		prev_pipe->next = pi->next;
	}
	اگر (job_list)
		last_jobid = job_list->jobid;
	وگرنه
		last_jobid = 0;

	pi->stopped_progs = 0;
	free_pipe(pi, 0);
	آزاد(pi);
}

/* Checks to see if any processes have exited -- if they
   have, figure out why and see if a job has completed */
ایستا صحیح checkjobs(ساختار pipe* fg_pipe)
{
	صحیح attributes;
	صحیح status;
	صحیح prognum = 0;
	ساختار pipe *pi;
	pid_t childpid;

	attributes = WUNTRACED;
	اگر (fg_pipe==پوچ) {
		attributes |= WNOHANG;
	}

	وقتی ((childpid = waitpid(-1, &status, attributes)) > 0) {
		اگر (fg_pipe) {
			صحیح i, rcode = 0;
			برای (i=0; i < fg_pipe->رقم_progs; i++) {
				اگر (fg_pipe->progs[i].pid == childpid) {
					اگر (i==fg_pipe->رقم_progs-1)
						rcode=WEXITSTATUS(status);
					(fg_pipe->رقم_progs)--;
					بازگشت(rcode);
				}
			}
		}

		برای (pi = job_list; pi; pi = pi->next) {
			prognum = 0;
			وقتی (prognum < pi->رقم_progs && pi->progs[prognum].pid != childpid) {
				prognum++;
			}
			اگر (prognum < pi->رقم_progs)
				بشکن;
		}

		اگر(pi==پوچ) {
			debug_printf("checkjobs: pid %d was not in our list!\n", childpid);
			ادامه;
		}

		اگر (WIFEXITED(status) || WIFSIGNALED(status)) {
			/* child exited */
			pi->running_progs--;
			pi->progs[prognum].pid = 0;

			اگر (!pi->running_progs) {
				printf(JOB_STATUS_FORMAT, pi->jobid, "Done", pi->text);
				remove_bg_job(pi);
			}
		} وگرنه {
			/* child stopped */
			pi->stopped_progs++;
			pi->progs[prognum].is_stopped = 1;

#اگر 0
			/* Printing this stuff is a pain, since it tends to
			 * overwrite the prompt an inconveinient moments.  So
			 * don't do that.  */
			اگر (pi->stopped_progs == pi->رقم_progs) {
				printf("\n"JOB_STATUS_FORMAT, pi->jobid, "Stopped", pi->text);
			}
#endif
		}
	}

	اگر (childpid == -1 && errno != ECHILD)
		perror_پیام("waitpid");

	/* move the shell to the foreground */
	/*if (interactive && tcsetpgrp(shell_terminal, getpgid(0))) */
	/*	perror_msg("tcsetpgrp-2"); */
	بازگشت -1;
}

/* Figure out our controlling tty, checking in order stderr,
 * stdin, and stdout.  If check_pgrp is set, also check that
 * we belong to the foreground process group associated with
 * that tty.  The value of shell_terminal is needed in order to call
 * tcsetpgrp(shell_terminal, ...); */
تهی controlling_tty(صحیح check_pgrp)
{
	pid_t curpgrp;

	اگر ((curpgrp = tcgetpgrp(shell_terminal = 2)) < 0
			&& (curpgrp = tcgetpgrp(shell_terminal = 0)) < 0
			&& (curpgrp = tcgetpgrp(shell_terminal = 1)) < 0)
		برو shell_terminal_error;

	اگر (check_pgrp && curpgrp != getpgid(0))
		برو shell_terminal_error;

	بازگشت;

shell_terminal_error:
		shell_terminal = -1;
		بازگشت;
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
ایستا صحیح run_pipe_real(ساختار pipe *pi)
{
	صحیح i;
#ifndef __U_BOOT__
	صحیح nextin, nextout;
	صحیح pipefds[2];				/* pipefds[0] is for reading */
	ساختار child_prog *child;
	ساختار built_in_command *x;
	نویسه *p;
# اگر __GNUC__
	/* Avoid longjmp clobbering */
	(تهی) &i;
	(تهی) &nextin;
	(تهی) &nextout;
	(تهی) &child;
# endif
#وگرنه
	صحیح nextin;
	صحیح flag = do_repeat ? CMD_FLAG_REPEAT : 0;
	ساختار child_prog *child;
	نویسه *p;
# اگر __GNUC__
	/* Avoid longjmp clobbering */
	(تهی) &i;
	(تهی) &nextin;
	(تهی) &child;
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
	اگر (pi->رقم_progs == 1) child = & (pi->progs[0]);
#ifndef __U_BOOT__
	اگر (pi->رقم_progs == 1 && child->group && child->subshell == 0) {
		صحیح squirrel[] = {-1, -1, -1};
		صحیح rcode;
		debug_printf("non-subshell grouping\n");
		setup_redirects(child, squirrel);
		/* XXX could we merge code with following builtin case,
		 * by creating a pseudo builtin that calls run_list_real? */
		rcode = run_list_real(child->group);
		restore_redirects(squirrel);
#وگرنه
		اگر (pi->رقم_progs == 1 && child->group) {
		صحیح rcode;
		debug_printf("non-subshell grouping\n");
		rcode = run_list_real(child->group);
#endif
		بازگشت rcode;
	} وگرنه اگر (pi->رقم_progs == 1 && pi->progs[0].argv != پوچ) {
		برای (i=0; is_assignment(child->argv[i]); i++) { /* nothing */ }
		اگر (i!=0 && child->argv[i]==پوچ) {
			/* assignments, but no command: set the local environment */
			برای (i=0; child->argv[i]!=پوچ; i++) {

				/* Ok, this case is tricky.  We have to decide if this is a
				 * local variable, or an already exported variable.  If it is
				 * already exported, we have to export the new value.  If it is
				 * not exported, we need only set this as a local variable.
				 * This junk is all to decide whether or not to export this
				 * variable. */
				صحیح export_me=0;
				نویسه *name, *مقدار;
				name = xstrdup(child->argv[i]);
				debug_printf("Local environment set: %s\n", name);
				مقدار = strchr(name, '=');
				اگر (مقدار)
					*مقدار=0;
#ifndef __U_BOOT__
				اگر ( get_local_var(name)) {
					export_me=1;
				}
#endif
				آزاد(name);
				p = درج_var_مقدار(child->argv[i]);
				set_local_var(p, export_me);
				اگر (p != child->argv[i]) آزاد(p);
			}
			بازگشت EXIT_SUCCESS;   /* don't worry about errors in set_local_var() yet */
		}
		برای (i = 0; is_assignment(child->argv[i]); i++) {
			p = درج_var_مقدار(child->argv[i]);
#ifndef __U_BOOT__
			putenv(strdup(p));
#وگرنه
			set_local_var(p, 0);
#endif
			اگر (p != child->argv[i]) {
				child->sp--;
				آزاد(p);
			}
		}
		اگر (child->sp) {
			نویسه * str = پوچ;

			str = make_string(child->argv + i,
					  child->argv_nonnull + i);
			parse_string_outer(str, FLAG_EXIT_FROM_LOOP | FLAG_REPARSING);
			آزاد(str);
			بازگشت last_return_code;
		}
#ifndef __U_BOOT__
		برای (x = bltins; x->cmd; x++) {
			اگر (strcmp(child->argv[i], x->cmd) == 0 ) {
				صحیح squirrel[] = {-1, -1, -1};
				صحیح rcode;
				اگر (x->function == builtin_exec && child->argv[i+1]==پوچ) {
					debug_printf("magic exec\n");
					setup_redirects(child,پوچ);
					بازگشت EXIT_SUCCESS;
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
			بازگشت rcode;
		}
#وگرنه
		/* check ";", because ,example , argv consist from
		 * "help;flinfo" must not execute
		 */
		اگر (strchr(child->argv[i], ';')) {
			printf("Unknown command '%s' - try 'help' or use "
					"'run' command\n", child->argv[i]);
			بازگشت -1;
		}
		/* Process the command */
		بازگشت cmd_process(flag, child->argc - i, child->argv + i,
				   &flag_repeat, پوچ);
#endif
	}
#ifndef __U_BOOT__

	برای (i = 0; i < pi->رقم_progs; i++) {
		child = & (pi->progs[i]);

		/* pipes are inserted between pairs of commands */
		اگر ((i + 1) < pi->رقم_progs) {
			اگر (pipe(pipefds)<0) perror_پیام_and_die("pipe");
			nextout = pipefds[1];
		} وگرنه {
			nextout=1;
			pipefds[0] = -1;
		}

		/* XXX test for failed fork()? */
		اگر (!(child->pid = fork())) {
			/* Set the handling for job control signals back to the default.  */
			سیگنال(SIGINT, SIG_DFL);
			سیگنال(SIGQUIT, SIG_DFL);
			سیگنال(SIGTERM, SIG_DFL);
			سیگنال(SIGTSTP, SIG_DFL);
			سیگنال(SIGTTIN, SIG_DFL);
			سیگنال(SIGTTOU, SIG_DFL);
			سیگنال(SIGCHLD, SIG_DFL);

			close_all();

			اگر (nextin != 0) {
				dup2(nextin, 0);
				close(nextin);
			}
			اگر (nextout != 1) {
				dup2(nextout, 1);
				close(nextout);
			}
			اگر (pipefds[0]!=-1) {
				close(pipefds[0]);  /* opposite end of our output pipe */
			}

			/* Like bash, explicit redirects override pipes,
			 * and the pipe fd is available for dup'ing. */
			setup_redirects(child,پوچ);

			اگر (interactive && pi->followup!=PIPE_BG) {
				/* If we (the child) win the race, put ourselves in the process
				 * group whose leader is the first process in this pipe. */
				اگر (pi->pgrp < 0) {
					pi->pgrp = getpid();
				}
				اگر (setpgid(0, pi->pgrp) == 0) {
					tcsetpgrp(2, pi->pgrp);
				}
			}

			pseudo_exec(child);
		}

		/* put our child in the process group whose leader is the
		   first process in this pipe */
		اگر (pi->pgrp < 0) {
			pi->pgrp = child->pid;
		}
		/* Don't check for errors.  The child may be dead already,
		 * in which case setpgid returns error code EACCES. */
		setpgid(child->pid, pi->pgrp);

		اگر (nextin != 0)
			close(nextin);
		اگر (nextout != 1)
			close(nextout);

		/* If there isn't another process, nextin is garbage
		   but it doesn't matter */
		nextin = pipefds[0];
	}
#endif
	بازگشت -1;
}

ایستا صحیح run_list_real(ساختار pipe *pi)
{
	نویسه *save_name = پوچ;
	نویسه **list = پوچ;
	نویسه **save_list = پوچ;
	ساختار pipe *rpipe;
	صحیح flag_rep = 0;
#ifndef __U_BOOT__
	صحیح save_رقم_progs;
#endif
	صحیح rcode=0, flag_skip=1;
	صحیح flag_restore = 0;
	صحیح if_code=0, next_if_code=0;  /* need double-buffer to handle elif */
	reserved_style rmode, skip_more_in_this_rmode=RES_XXXX;
	/* check syntax for "for" */
	برای (rpipe = pi; rpipe; rpipe = rpipe->next) {
		اگر ((rpipe->r_mode == RES_IN ||
		    rpipe->r_mode == RES_FOR) &&
		    (rpipe->next == پوچ)) {
				syntax();
#ifdef __U_BOOT__
				flag_repeat = 0;
#endif
				بازگشت 1;
		}
		اگر ((rpipe->r_mode == RES_IN &&
			(rpipe->next->r_mode == RES_IN &&
			rpipe->next->progs->argv != پوچ))||
			(rpipe->r_mode == RES_FOR &&
			rpipe->next->r_mode != RES_IN)) {
				syntax();
#ifdef __U_BOOT__
				flag_repeat = 0;
#endif
				بازگشت 1;
		}
	}
	برای (; pi; pi = (flag_restore != 0) ? rpipe : pi->next) {
		اگر (pi->r_mode == RES_WHILE || pi->r_mode == RES_UNTIL ||
			pi->r_mode == RES_FOR) {
#ifdef __U_BOOT__
				/* check Ctrl-C */
				ctrlc();
				اگر ((had_ctrlc())) {
					بازگشت 1;
				}
#endif
				flag_restore = 0;
				اگر (!rpipe) {
					flag_rep = 0;
					rpipe = pi;
				}
		}
		rmode = pi->r_mode;
		debug_printf("rmode=%d  if_code=%d  next_if_code=%d skip_more=%d\n", rmode, if_code, next_if_code, skip_more_in_this_rmode);
		اگر (rmode == skip_more_in_this_rmode && flag_skip) {
			اگر (pi->followup == PIPE_SEQ) flag_skip=0;
			ادامه;
		}
		flag_skip = 1;
		skip_more_in_this_rmode = RES_XXXX;
		اگر (rmode == RES_THEN || rmode == RES_ELSE) if_code = next_if_code;
		اگر (rmode == RES_THEN &&  if_code) ادامه;
		اگر (rmode == RES_ELSE && !if_code) ادامه;
		اگر (rmode == RES_ELIF && !if_code) بشکن;
		اگر (rmode == RES_FOR && pi->رقم_progs) {
			اگر (!list) {
				/* if no variable values after "in" we skip "for" */
				اگر (!pi->next->progs->argv) ادامه;
				/* create list of variable values */
				list = make_list_in(pi->next->progs->argv,
					pi->progs->argv[0]);
				save_list = list;
				save_name = pi->progs->argv[0];
				pi->progs->argv[0] = پوچ;
				flag_rep = 1;
			}
			اگر (!(*list)) {
				آزاد(pi->progs->argv[0]);
				آزاد(save_list);
				list = پوچ;
				flag_rep = 0;
				pi->progs->argv[0] = save_name;
#ifndef __U_BOOT__
				pi->progs->glob_result.gl_pathv[0] =
					pi->progs->argv[0];
#endif
				ادامه;
			} وگرنه {
				/* insert new value from list for variable */
				آزاد(pi->progs->argv[0]);
				pi->progs->argv[0] = *list++;
#ifndef __U_BOOT__
				pi->progs->glob_result.gl_pathv[0] =
					pi->progs->argv[0];
#endif
			}
		}
		اگر (rmode == RES_IN) ادامه;
		اگر (rmode == RES_DO) {
			اگر (!flag_rep) ادامه;
		}
		اگر (rmode == RES_DONE) {
			اگر (flag_rep) {
				flag_restore = 1;
			} وگرنه {
				rpipe = پوچ;
			}
		}
		اگر (pi->رقم_progs == 0) ادامه;
#ifndef __U_BOOT__
		save_رقم_progs = pi->رقم_progs; /* save number of programs */
#endif
		rcode = run_pipe_real(pi);
		debug_printf("run_pipe_real returned %d\n",rcode);
#ifndef __U_BOOT__
		اگر (rcode!=-1) {
			/* We only ran a builtin: rcode was set by the return value
			 * of run_pipe_real(), and we don't need to wait for anything. */
		} وگرنه اگر (pi->followup==PIPE_BG) {
			/* XXX check bash's behavior with nontrivial pipes */
			/* XXX compute jobid */
			/* XXX what does bash do with attempts to background builtins? */
			درج_bg_job(pi);
			rcode = EXIT_SUCCESS;
		} وگرنه {
			اگر (interactive) {
				/* move the new process group into the foreground */
				اگر (tcsetpgrp(shell_terminal, pi->pgrp) && errno != ENOTTY)
					perror_پیام("tcsetpgrp-3");
				rcode = checkjobs(pi);
				/* move the shell to the foreground */
				اگر (tcsetpgrp(shell_terminal, getpgid(0)) && errno != ENOTTY)
					perror_پیام("tcsetpgrp-4");
			} وگرنه {
				rcode = checkjobs(pi);
			}
			debug_printf("checkjobs returned %d\n",rcode);
		}
		last_return_code=rcode;
#وگرنه
		اگر (rcode < -1) {
			last_return_code = -rcode - 2;
			بازگشت -2;	/* exit */
		}
		last_return_code = rcode;
#endif
#ifndef __U_BOOT__
		pi->رقم_progs = save_رقم_progs; /* restore number of programs */
#endif
		اگر ( rmode == RES_IF || rmode == RES_ELIF )
			next_if_code=rcode;  /* can be overwritten a number of times */
		اگر (rmode == RES_WHILE)
			flag_rep = !last_return_code;
		اگر (rmode == RES_UNTIL)
			flag_rep = last_return_code;
		اگر ( (rcode==EXIT_SUCCESS && pi->followup==PIPE_OR) ||
		     (rcode!=EXIT_SUCCESS && pi->followup==PIPE_AND) )
			skip_more_in_this_rmode=rmode;
#ifndef __U_BOOT__
		checkjobs(پوچ);
#endif
	}
	بازگشت rcode;
}

/* broken, of course, but OK for testing */
ایستا نویسه *indenter(صحیح i)
{
	ایستا نویسه blanks[]="                                    ";
	بازگشت &blanks[اندازه(blanks)-i-1];
}

/* return code is the exit status of the pipe */
ایستا صحیح free_pipe(ساختار pipe *pi, صحیح indent)
{
	نویسه **p;
	ساختار child_prog *child;
#ifndef __U_BOOT__
	ساختار redir_struct *r, *rnext;
#endif
	صحیح a, i, ret_code=0;
	نویسه *ind = indenter(indent);

#ifndef __U_BOOT__
	اگر (pi->stopped_progs > 0)
		بازگشت ret_code;
	final_printf("%s run pipe: (pid %d)\n",ind,getpid());
#endif
	برای (i=0; i<pi->رقم_progs; i++) {
		child = &pi->progs[i];
		final_printf("%s  command %d:\n",ind,i);
		اگر (child->argv) {
			برای (a=0,p=child->argv; *p; a++,p++) {
				final_printf("%s   argv[%d] = %s\n",ind,a,*p);
			}
#ifndef __U_BOOT__
			globfree(&child->glob_result);
#وگرنه
			برای (a = 0; a < child->argc; a++) {
				آزاد(child->argv[a]);
			}
			آزاد(child->argv);
			آزاد(child->argv_nonnull);
			child->argc = 0;
#endif
			child->argv=پوچ;
		} وگرنه اگر (child->group) {
#ifndef __U_BOOT__
			final_printf("%s   begin group (subshell:%d)\n",ind, child->subshell);
#endif
			ret_code = free_pipe_list(child->group,indent+3);
			final_printf("%s   end group\n",ind);
		} وگرنه {
			final_printf("%s   (nil)\n",ind);
		}
#ifndef __U_BOOT__
		برای (r=child->redirects; r; r=rnext) {
			final_printf("%s   redirect %d%s", ind, r->fd, redir_table[r->type].descrip);
			اگر (r->dup == -1) {
				/* guard against the case >$FOO, where foo is unset or blank */
				اگر (r->word.gl_pathv) {
					final_printf(" %s\n", *r->word.gl_pathv);
					globfree(&r->word);
				}
			} وگرنه {
				final_printf("&%d\n", r->dup);
			}
			rnext=r->next;
			آزاد(r);
		}
		child->redirects=پوچ;
#endif
	}
	آزاد(pi->progs);   /* children are an array, they get freed all at once */
	pi->progs=پوچ;
	بازگشت ret_code;
}

ایستا صحیح free_pipe_list(ساختار pipe *head, صحیح indent)
{
	صحیح rcode=0;   /* if list has no members */
	ساختار pipe *pi, *next;
	نویسه *ind = indenter(indent);
	برای (pi=head; pi; pi=next) {
		final_printf("%s pipe reserved mode %d\n", ind, pi->r_mode);
		rcode = free_pipe(pi, indent);
		final_printf("%s pipe followup code %d\n", ind, pi->followup);
		next=pi->next;
		pi->next=پوچ;
		آزاد(pi);
	}
	بازگشت rcode;
}

/* Select which version we will use */
ایستا صحیح run_list(ساختار pipe *pi)
{
	صحیح rcode=0;
#ifndef __U_BOOT__
	اگر (fake_mode==0) {
#endif
		rcode = run_list_real(pi);
#ifndef __U_BOOT__
	}
#endif
	/* free_pipe_list has the side effect of clearing memory
	 * In the long run that function can be merged with run_list_real,
	 * but doing that now would hobble the debugging effort. */
	free_pipe_list(pi,0);
	بازگشت rcode;
}

/* The API for glob is arguably broken.  This routine pushes a non-matching
 * string into the output structure, removing non-backslashed backslashes.
 * If someone can prove me wrong, by performing this function within the
 * original glob(3) api, feel free to rewrite this routine into oblivion.
 * Return code (0 vs. GLOB_NOSPACE) matches glob(3).
 * XXX broken if the last character is '\\', check that before calling.
 */
#ifndef __U_BOOT__
ایستا صحیح globhack(ثابت نویسه *src, صحیح flags, glob_t *pglob)
{
	صحیح cnt=0, pathc;
	ثابت نویسه *s;
	نویسه *dest;
	برای (cnt=1, s=src; s && *s; s++) {
		اگر (*s == '\\') s++;
		cnt++;
	}
	dest = malloc(cnt);
	اگر (!dest) بازگشت GLOB_NOSPACE;
	اگر (!(flags & GLOB_APPEND)) {
		pglob->gl_pathv=پوچ;
		pglob->gl_pathc=0;
		pglob->gl_offs=0;
		pglob->gl_offs=0;
	}
	pathc = ++pglob->gl_pathc;
	pglob->gl_pathv = realloc(pglob->gl_pathv, (pathc+1)*اندازه(*pglob->gl_pathv));
	اگر (pglob->gl_pathv == پوچ) بازگشت GLOB_NOSPACE;
	pglob->gl_pathv[pathc-1]=dest;
	pglob->gl_pathv[pathc]=پوچ;
	برای (s=src; s && *s; s++, dest++) {
		اگر (*s == '\\') s++;
		*dest = *s;
	}
	*dest='\0';
	بازگشت 0;
}

/* XXX broken if the last character is '\\', check that before calling */
ایستا صحیح glob_needed(ثابت نویسه *s)
{
	برای (; *s; s++) {
		اگر (*s == '\\') s++;
		اگر (strchr("*[?",*s)) بازگشت 1;
	}
	بازگشت 0;
}

#اگر 0
ایستا تهی globprint(glob_t *pglob)
{
	صحیح i;
	debug_printf("glob_t at %p:\n", pglob);
	debug_printf("  gl_pathc=%d  gl_pathv=%p  gl_offs=%d  gl_flags=%d\n",
		pglob->gl_pathc, pglob->gl_pathv, pglob->gl_offs, pglob->gl_flags);
	برای (i=0; i<pglob->gl_pathc; i++)
		debug_printf("pglob->gl_pathv[%d] = %p = %s\n", i,
			pglob->gl_pathv[i], pglob->gl_pathv[i]);
}
#endif

ایستا صحیح xglob(o_string *dest, صحیح flags, glob_t *pglob)
{
	صحیح gr;

	/* short-circuit for null word */
	/* we can code this better when the debug_printf's are gone */
	اگر (dest->length == 0) {
		اگر (dest->nonnull) {
			/* bash man page calls this an "explicit" null */
			gr = globhack(dest->data, flags, pglob);
			debug_printf("globhack returned %d\n",gr);
		} وگرنه {
			بازگشت 0;
		}
	} وگرنه اگر (glob_needed(dest->data)) {
		gr = glob(dest->data, flags, پوچ, pglob);
		debug_printf("glob returned %d\n",gr);
		اگر (gr == GLOB_NOMATCH) {
			/* quote removal, or more accurately, backslash removal */
			gr = globhack(dest->data, flags, pglob);
			debug_printf("globhack returned %d\n",gr);
		}
	} وگرنه {
		gr = globhack(dest->data, flags, pglob);
		debug_printf("globhack returned %d\n",gr);
	}
	اگر (gr == GLOB_NOSPACE)
		error_پیام_and_die("out of memory during glob");
	اگر (gr != 0) { /* GLOB_ABORTED ? */
		error_پیام("glob(3) error %d",gr);
	}
	/* globprint(glob_target); */
	بازگشت gr;
}
#endif

#ifdef __U_BOOT__
ایستا نویسه *get_dollar_var(نویسه ch);
#endif

/* This is used to get/check local shell variables */
نویسه *get_local_var(ثابت نویسه *s)
{
	ساختار variables *cur;

	اگر (!s)
		بازگشت پوچ;

#ifdef __U_BOOT__
	اگر (*s == '$')
		بازگشت get_dollar_var(s[1]);
#endif

	برای (cur = بالاترین_vars; cur; cur=cur->next)
		اگر(strcmp(cur->name, s)==0)
			بازگشت cur->مقدار;
	بازگشت پوچ;
}

/* This is used to set local shell variables
   flg_export==0 if only local (not exporting) variable
   flg_export==1 if "new" exporting environ
   flg_export>1  if current startup environ (not call putenv()) */
صحیح set_local_var(ثابت نویسه *s, صحیح flg_export)
{
	نویسه *name, *مقدار;
	صحیح result=0;
	ساختار variables *cur;

#ifdef __U_BOOT__
	/* might be possible! */
	اگر (!isalpha(*s))
		بازگشت -1;
#endif

	name=strdup(s);

	/* Assume when we enter this function that we are already in
	 * NAME=VALUE format.  So the first order of business is to
	 * split 's' on the '=' into 'name' and 'value' */
	مقدار = strchr(name, '=');
	اگر (!مقدار) {
		آزاد(name);
		بازگشت -1;
	}
	*مقدار++ = 0;

	اگر (!*مقدار) {
		unset_local_var(name);
		آزاد(name);
		بازگشت 0;
	}

	برای(cur = بالاترین_vars; cur; cur = cur->next) {
		اگر(strcmp(cur->name, name)==0)
			بشکن;
	}

	اگر(cur) {
		اگر(strcmp(cur->مقدار, مقدار)==0) {
			اگر(flg_export>0 && cur->flg_export==0)
				cur->flg_export=flg_export;
			وگرنه
				result++;
		} وگرنه {
			اگر(cur->flg_read_only) {
				error_پیام("%s: readonly variable", name);
				result = -1;
			} وگرنه {
				اگر(flg_export>0 || cur->flg_export>1)
					cur->flg_export=1;
				آزاد(cur->مقدار);

				cur->مقدار = strdup(مقدار);
			}
		}
	} وگرنه {
		cur = malloc(اندازه(ساختار variables));
		اگر(!cur) {
			result = -1;
		} وگرنه {
			cur->name = strdup(name);
			اگر (cur->name == پوچ) {
				آزاد(cur);
				result = -1;
			} وگرنه {
				ساختار variables *bottom = بالاترین_vars;
				cur->مقدار = strdup(مقدار);
				cur->next = پوچ;
				cur->flg_export = flg_export;
				cur->flg_read_only = 0;
				وقتی(bottom->next) bottom=bottom->next;
				bottom->next = cur;
			}
		}
	}

#ifndef __U_BOOT__
	اگر(result==0 && cur->flg_export==1) {
		*(مقدار-1) = '=';
		result = putenv(name);
	} وگرنه {
#endif
		آزاد(name);
#ifndef __U_BOOT__
		اگر(result>0)            /* equivalent to previous set */
			result = 0;
	}
#endif
	بازگشت result;
}

تهی unset_local_var(ثابت نویسه *name)
{
	ساختار variables *cur;

	اگر (name) {
		برای (cur = بالاترین_vars; cur; cur=cur->next) {
			اگر(strcmp(cur->name, name)==0)
				بشکن;
		}
		اگر (cur != پوچ) {
			ساختار variables *next = بالاترین_vars;
			اگر(cur->flg_read_only) {
				error_پیام("%s: readonly variable", name);
				بازگشت;
			} وگرنه {
#ifndef __U_BOOT__
				اگر(cur->flg_export)
					unenv_set(cur->name);
#endif
				آزاد(cur->name);
				آزاد(cur->مقدار);
				وقتی (next->next != cur)
					next = next->next;
				next->next = cur->next;
			}
			آزاد(cur);
		}
	}
}

ایستا صحیح is_assignment(ثابت نویسه *s)
{
	اگر (s == پوچ)
		بازگشت 0;

	اگر (!isalpha(*s)) بازگشت 0;
	++s;
	وقتی(isalnum(*s) || *s=='_') ++s;
	بازگشت *s=='=';
}

#ifndef __U_BOOT__
/* the src parameter allows us to peek forward to a possible &n syntax
 * for file descriptor duplication, e.g., "2>&1".
 * Return code is 0 normally, 1 if a syntax error is detected in src.
 * Resource errors (in xmalloc) cause the process to exit */
ایستا صحیح setup_redirect(ساختار p_context *ctx, صحیح fd, redir_type style,
	ساختار in_str *درون_داد)
{
	ساختار child_prog *child=ctx->child;
	ساختار redir_struct *redir = child->redirects;
	ساختار redir_struct *last_redir=پوچ;

	/* Create a new redir_struct and drop it onto the end of the linked list */
	وقتی(redir) {
		last_redir=redir;
		redir=redir->next;
	}
	redir = xmalloc(اندازه(ساختار redir_struct));
	redir->next=پوچ;
	redir->word.gl_pathv=پوچ;
	اگر (last_redir) {
		last_redir->next=redir;
	} وگرنه {
		child->redirects=redir;
	}

	redir->type=style;
	redir->fd= (fd==-1) ? redir_table[style].default_fd : fd ;

	debug_printf("Redirect type %d%s\n", redir->fd, redir_table[style].descrip);

	/* Check for a '2>&1' type redirect */
	redir->dup = redirect_dup_رقم(درون_داد);
	اگر (redir->dup == -2) بازگشت 1;  /* syntax error */
	اگر (redir->dup != -1) {
		/* Erik had a check here that the file descriptor in question
		 * is legit; I postpone that to "run time"
		 * A "-" representation of "close me" shows up as a -3 here */
		debug_printf("Duplicating redirect '%d>&%d'\n", redir->fd, redir->dup);
	} وگرنه {
		/* We do _not_ try to open the file that src points to,
		 * since we need to return and let src be expanded first.
		 * Set ctx->pending_redirect, so we know what to do at the
		 * end of the next parsed word.
		 */
		ctx->pending_redirect = redir;
	}
	بازگشت 0;
}
#endif

ایستا ساختار pipe *new_pipe(تهی)
{
	ساختار pipe *pi;
	pi = xmalloc(اندازه(ساختار pipe));
	pi->رقم_progs = 0;
	pi->progs = پوچ;
	pi->next = پوچ;
	pi->followup = 0;  /* invalid */
	pi->r_mode = RES_NONE;
	بازگشت pi;
}

ایستا تهی initialize_context(ساختار p_context *ctx)
{
	ctx->pipe=پوچ;
#ifndef __U_BOOT__
	ctx->pending_redirect=پوچ;
#endif
	ctx->child=پوچ;
	ctx->list_head=new_pipe();
	ctx->pipe=ctx->list_head;
	ctx->w=RES_NONE;
	ctx->پشته=پوچ;
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
ساختار reserved_combo {
	نویسه *literal;
	صحیح code;
	long flag;
};
/* Mostly a list of accepted follow-up reserved words.
 * FLAG_END means we are done with the sequence, and are ready
 * to turn the compound list into a command.
 * FLAG_START means the word must start a new compound list.
 */
ایستا ساختار reserved_combo reserved_list[] = {
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
#define NRES (اندازه(reserved_list)/اندازه(ساختار reserved_combo))

ایستا صحیح reserved_word(o_string *dest, ساختار p_context *ctx)
{
	ساختار reserved_combo *r;
	برای (r=reserved_list;
		r<reserved_list+NRES; r++) {
		اگر (strcmp(dest->data, r->literal) == 0) {
			debug_printf("found reserved word %s, code %d\n",r->literal,r->code);
			اگر (r->flag & FLAG_START) {
				ساختار p_context *new = xmalloc(اندازه(ساختار p_context));
				debug_printf("push stack\n");
				اگر (ctx->w == RES_IN || ctx->w == RES_FOR) {
					syntax();
					آزاد(new);
					ctx->w = RES_SNTX;
					b_reset(dest);
					بازگشت 1;
				}
				*new = *ctx;   /* physical copy */
				initialize_context(ctx);
				ctx->پشته=new;
			} وگرنه اگر ( ctx->w == RES_NONE || ! (ctx->old_flag & (1<<r->code))) {
				syntax();
				ctx->w = RES_SNTX;
				b_reset(dest);
				بازگشت 1;
			}
			ctx->w=r->code;
			ctx->old_flag = r->flag;
			اگر (ctx->old_flag & FLAG_END) {
				ساختار p_context *old;
				debug_printf("pop stack\n");
				done_pipe(ctx,PIPE_SEQ);
				old = ctx->پشته;
				old->child->group = ctx->list_head;
#ifndef __U_BOOT__
				old->child->subshell = 0;
#endif
				*ctx = *old;   /* physical copy */
				آزاد(old);
			}
			b_reset (dest);
			بازگشت 1;
		}
	}
	بازگشت 0;
}

/* normal return is 0.
 * Syntax or xglob errors return 1. */
ایستا صحیح done_word(o_string *dest, ساختار p_context *ctx)
{
	ساختار child_prog *child=ctx->child;
#ifndef __U_BOOT__
	glob_t *glob_target;
	صحیح gr, flags = 0;
#وگرنه
	نویسه *str, *s;
	صحیح argc, cnt;
#endif

	debug_printf("done_word: %s %p\n", dest->data, child);
	اگر (dest->length == 0 && !dest->nonnull) {
		debug_printf("  true null, ignored\n");
		بازگشت 0;
	}
#ifndef __U_BOOT__
	اگر (ctx->pending_redirect) {
		glob_target = &ctx->pending_redirect->word;
	} وگرنه {
#endif
		اگر (child->group) {
			syntax();
			بازگشت 1;  /* syntax error, groups and arglists don't mix */
		}
		اگر (!child->argv && (ctx->type & FLAG_PARSE_SEMICOLON)) {
			debug_printf("checking %s for reserved-ness\n",dest->data);
			اگر (reserved_word(dest,ctx)) بازگشت ctx->w==RES_SNTX;
		}
#ifndef __U_BOOT__
		glob_target = &child->glob_result;
		اگر (child->argv) flags |= GLOB_APPEND;
#وگرنه
		برای (cnt = 1, s = dest->data; s && *s; s++) {
			اگر (*s == '\\') s++;
			cnt++;
		}
		str = malloc(cnt);
		اگر (!str) بازگشت 1;
		اگر ( child->argv == پوچ) {
			child->argc=0;
		}
		argc = ++child->argc;
		child->argv = realloc(child->argv, (argc+1)*اندازه(*child->argv));
		اگر (child->argv == پوچ) {
			آزاد(str);
			بازگشت 1;
		}
		child->argv_nonnull = realloc(child->argv_nonnull,
					(argc+1)*اندازه(*child->argv_nonnull));
		اگر (child->argv_nonnull == پوچ) {
			آزاد(str);
			بازگشت 1;
		}
		child->argv[argc-1]=str;
		child->argv_nonnull[argc-1] = dest->nonnull;
		child->argv[argc]=پوچ;
		child->argv_nonnull[argc] = 0;
		برای (s = dest->data; s && *s; s++,str++) {
			اگر (*s == '\\') s++;
			*str = *s;
		}
		*str = '\0';
#endif
#ifndef __U_BOOT__
	}
	gr = xglob(dest, flags, glob_target);
	اگر (gr != 0) بازگشت 1;
#endif

	b_reset(dest);
#ifndef __U_BOOT__
	اگر (ctx->pending_redirect) {
		ctx->pending_redirect=پوچ;
		اگر (glob_target->gl_pathc != 1) {
			error_پیام("ambiguous redirect");
			بازگشت 1;
		}
	} وگرنه {
		child->argv = glob_target->gl_pathv;
	}
#endif
	اگر (ctx->w == RES_FOR) {
		done_word(dest,ctx);
		done_pipe(ctx,PIPE_SEQ);
	}
	بازگشت 0;
}

/* The only possible error here is out of memory, in which case
 * xmalloc exits. */
ایستا صحیح done_command(ساختار p_context *ctx)
{
	/* The child is really already in the pipe structure, so
	 * advance the pipe counter and make a new, null child.
	 * Only real trickiness here is that the uncommitted
	 * child structure, to which ctx->child points, is not
	 * counted in pi->num_progs. */
	ساختار pipe *pi=ctx->pipe;
	ساختار child_prog *prog=ctx->child;

	اگر (prog && prog->group == پوچ
		 && prog->argv == پوچ
#ifndef __U_BOOT__
		 && prog->redirects == پوچ) {
#وگرنه
										) {
#endif
		debug_printf("done_command: skipping null command\n");
		بازگشت 0;
	} وگرنه اگر (prog) {
		pi->رقم_progs++;
		debug_printf("done_command: num_progs incremented to %d\n",pi->رقم_progs);
	} وگرنه {
		debug_printf("done_command: initializing\n");
	}
	pi->progs = xrealloc(pi->progs, اندازه(*pi->progs) * (pi->رقم_progs+1));

	prog = pi->progs + pi->رقم_progs;
#ifndef __U_BOOT__
	prog->redirects = پوچ;
#endif
	prog->argv = پوچ;
	prog->argv_nonnull = پوچ;
#ifndef __U_BOOT__
	prog->is_stopped = 0;
#endif
	prog->group = پوچ;
#ifndef __U_BOOT__
	prog->glob_result.gl_pathv = پوچ;
	prog->family = pi;
#endif
	prog->sp = 0;
	ctx->child = prog;
	prog->type = ctx->type;

	/* but ctx->pipe and ctx->list_head remain unchanged */
	بازگشت 0;
}

ایستا صحیح done_pipe(ساختار p_context *ctx, pipe_style type)
{
	ساختار pipe *new_p;
	done_command(ctx);  /* implicit closure of previous command */
	debug_printf("done_pipe, type %d\n", type);
	ctx->pipe->followup = type;
	ctx->pipe->r_mode = ctx->w;
	new_p=new_pipe();
	ctx->pipe->next = new_p;
	ctx->pipe = new_p;
	ctx->child = پوچ;
	done_command(ctx);  /* set up new pipe to accept commands */
	بازگشت 0;
}

#ifndef __U_BOOT__
/* peek ahead in the in_str to find out if we have a "&n" construct,
 * as in "2>&1", that represents duplicating a file descriptor.
 * returns either -2 (syntax error), -1 (no &), or the number found.
 */
ایستا صحیح redirect_dup_رقم(ساختار in_str *درون_داد)
{
	صحیح ch, d=0, ok=0;
	ch = b_peek(درون_داد);
	اگر (ch != '&') بازگشت -1;

	b_getch(درون_داد);  /* get the & */
	ch=b_peek(درون_داد);
	اگر (ch == '-') {
		b_getch(درون_داد);
		بازگشت -3;  /* "-" represents "close me" */
	}
	وقتی (isdigit(ch)) {
		d = d*10+(ch-'0');
		ok=1;
		b_getch(درون_داد);
		ch = b_peek(درون_داد);
	}
	اگر (ok) بازگشت d;

	error_پیام("ambiguous redirect");
	بازگشت -2;
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
ایستا صحیح redirect_opt_رقم(o_string *o)
{
	صحیح رقم;

	اگر (o->length==0) بازگشت -1;
	برای(رقم=0; رقم<o->length; رقم++) {
		اگر (!isdigit(*(o->data+رقم))) {
			بازگشت -1;
		}
	}
	/* reuse num (and save an int) */
	رقم=atoi(o->data);
	b_reset(o);
	بازگشت رقم;
}

FILE *generate_stream_from_list(ساختار pipe *head)
{
	FILE *pf;
#اگر 1
	صحیح pid, channel[2];
	اگر (pipe(channel)<0) perror_پیام_and_die("pipe");
	pid=fork();
	اگر (pid<0) {
		perror_پیام_and_die("fork");
	} وگرنه اگر (pid==0) {
		close(channel[0]);
		اگر (channel[1] != 1) {
			dup2(channel[1],1);
			close(channel[1]);
		}
#اگر 0
#define SURROGATE "surrogate response"
		اكتب(1,SURROGATE,اندازه(SURROGATE));
		_exit(run_list(head));
#وگرنه
		_exit(run_list_real(head));   /* leaks memory */
#endif
	}
	debug_printf("forked child %d\n",pid);
	close(channel[1]);
	pf = fdopen(channel[0],"r");
	debug_printf("pipe on FILE *%p\n",pf);
#وگرنه
	free_pipe_list(head,0);
	pf=popen("echo surrogate response","r");
	debug_printf("started fake pipe on FILE *%p\n",pf);
#endif
	بازگشت pf;
}

/* this version hacked for testing purposes */
/* return code is exit status of the process that is run. */
ایستا صحیح process_command_subs(o_string *dest, ساختار p_context *ctx, ساختار in_str *درون_داد, صحیح subst_end)
{
	صحیح retcode;
	o_string result=NULL_O_STRING;
	ساختار p_context inner;
	FILE *p;
	ساختار in_str pipe_str;
	initialize_context(&inner);

	/* recursion to generate command */
	retcode = parse_stream(&result, &inner, درون_داد, subst_end);
	اگر (retcode != 0) بازگشت retcode;  /* syntax error or EOF */
	done_word(&result, &inner);
	done_pipe(&inner, PIPE_SEQ);
	b_free(&result);

	p=generate_stream_from_list(inner.list_head);
	اگر (p==پوچ) بازگشت 1;
	mark_open(fileno(p));
	setup_file_in_str(&pipe_str, p);

	/* now send results of command back into original context */
	retcode = parse_stream(dest, ctx, &pipe_str, '\0');
	/* XXX In case of a syntax error, should we try to kill the child?
	 * That would be tough to do right, so just read until EOF. */
	اگر (retcode == 1) {
		وقتی (b_getch(&pipe_str)!=EOF) { /* discard */ };
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
	بازگشت retcode;
}

ایستا صحیح parse_group(o_string *dest, ساختار p_context *ctx,
	ساختار in_str *درون_داد, صحیح ch)
{
	صحیح rcode, endch=0;
	ساختار p_context sub;
	ساختار child_prog *child = ctx->child;
	اگر (child->argv) {
		syntax();
		بازگشت 1;  /* syntax error, groups and arglists don't mix */
	}
	initialize_context(&sub);
	انتخاب(ch) {
		حالت '(': endch=')'; child->subshell=1; بشکن;
		حالت '{': endch='}'; بشکن;
		پیشفرض: syntax();   /* really logic error */
	}
	rcode=parse_stream(dest,&sub,درون_داد,endch);
	done_word(dest,&sub); /* finish off the final word in the subcontext */
	done_pipe(&sub, PIPE_SEQ);  /* and the final command there, too */
	child->group = sub.list_head;
	بازگشت rcode;
	/* child remains "open", available for possible redirects */
}
#endif

/* basically useful version until someone wants to get fancier,
 * see the bash man page under "Parameter Expansion" */
ایستا نویسه *lookup_param(نویسه *src)
{
	نویسه *p;
	نویسه *sep;
	نویسه *default_val = پوچ;
	صحیح assign = 0;
	صحیح expand_empty = 0;

	اگر (!src)
		بازگشت پوچ;

	sep = strchr(src, ':');

	اگر (sep) {
		*sep = '\0';
		اگر (*(sep + 1) == '-')
			default_val = sep+2;
		اگر (*(sep + 1) == '=') {
			default_val = sep+2;
			assign = 1;
		}
		اگر (*(sep + 1) == '+') {
			default_val = sep+2;
			expand_empty = 1;
		}
	}

	p = env_get(src);
	اگر (!p)
		p = get_local_var(src);

	اگر (!p || strlen(p) == 0) {
		p = default_val;
		اگر (assign) {
			نویسه *var = malloc(strlen(src)+strlen(default_val)+2);
			اگر (var) {
				sprintf(var, "%s=%s", src, default_val);
				set_local_var(var, 0);
			}
			آزاد(var);
		}
	} وگرنه اگر (expand_empty) {
		p += strlen(p);
	}

	اگر (sep)
		*sep = ':';

	بازگشت p;
}

#ifdef __U_BOOT__
ایستا نویسه *get_dollar_var(نویسه ch)
{
	ایستا نویسه buf[40];

	buf[0] = '\0';
	انتخاب (ch) {
		حالت '?':
			sprintf(buf, "%u", (unsigned صحیح)last_return_code);
			بشکن;
		پیشفرض:
			بازگشت پوچ;
	}
	بازگشت buf;
}
#endif

/* return code: 0 for OK, 1 for syntax error */
ایستا صحیح handle_dollar(o_string *dest, ساختار p_context *ctx, ساختار in_str *درون_داد)
{
#ifndef __U_BOOT__
	صحیح i, advance=0;
#وگرنه
	صحیح advance=0;
#endif
#ifndef __U_BOOT__
	نویسه sep[]=" ";
#endif
	صحیح ch = درون_داد->peek(درون_داد);  /* first character after the $ */
	debug_printf("handle_dollar: ch=%c\n",ch);
	اگر (isalpha(ch)) {
		b_addchr(dest, SPECIAL_VAR_نماد);
		ctx->child->sp++;
		وقتی(ch=b_peek(درون_داد),isalnum(ch) || ch=='_') {
			b_getch(درون_داد);
			b_addchr(dest,ch);
		}
		b_addchr(dest, SPECIAL_VAR_نماد);
#ifndef __U_BOOT__
	} وگرنه اگر (isdigit(ch)) {
		i = ch-'0';  /* XXX is $0 special? */
		اگر (i<global_argc) {
			parse_string(dest, ctx, global_argv[i]); /* recursion */
		}
		advance = 1;
#endif
	} وگرنه انتخاب (ch) {
#ifndef __U_BOOT__
		حالت '$':
			b_adduint(dest,getpid());
			advance = 1;
			بشکن;
		حالت '!':
			اگر (last_bg_pid > 0) b_adduint(dest, last_bg_pid);
			advance = 1;
			بشکن;
#endif
		حالت '?':
#ifndef __U_BOOT__
			b_adduint(dest,last_return_code);
#وگرنه
			ctx->child->sp++;
			b_addchr(dest, SPECIAL_VAR_نماد);
			b_addchr(dest, '$');
			b_addchr(dest, '?');
			b_addchr(dest, SPECIAL_VAR_نماد);
#endif
			advance = 1;
			بشکن;
#ifndef __U_BOOT__
		حالت '#':
			b_adduint(dest,global_argc ? global_argc-1 : 0);
			advance = 1;
			بشکن;
#endif
		حالت '{':
			b_addchr(dest, SPECIAL_VAR_نماد);
			ctx->child->sp++;
			b_getch(درون_داد);
			/* XXX maybe someone will try to escape the '}' */
			وقتی(ch=b_getch(درون_داد),ch!=EOF && ch!='}') {
				b_addchr(dest,ch);
			}
			اگر (ch != '}') {
				syntax();
				بازگشت 1;
			}
			b_addchr(dest, SPECIAL_VAR_نماد);
			بشکن;
#ifndef __U_BOOT__
		حالت '(':
			b_getch(درون_داد);
			process_command_subs(dest, ctx, درون_داد, ')');
			بشکن;
		حالت '*':
			sep[0]=ifs[0];
			برای (i=1; i<global_argc; i++) {
				parse_string(dest, ctx, global_argv[i]);
				اگر (i+1 < global_argc) parse_string(dest, ctx, sep);
			}
			بشکن;
		حالت '@':
		حالت '-':
		حالت '_':
			/* still unhandled, but should be eventually */
			error_پیام("unhandled syntax: $%c",ch);
			بازگشت 1;
			بشکن;
#endif
		پیشفرض:
			b_addqchr(dest,'$',dest->quote);
	}
	/* Eat the character if the flag was set.  If the compiler
	 * is smart enough, we could substitute "b_getch(input);"
	 * for all the "advance = 1;" above, and also end up with
	 * a nice size-optimized program.  Hah!  That'll be the day.
	 */
	اگر (advance) b_getch(درون_داد);
	بازگشت 0;
}

#ifndef __U_BOOT__
صحیح parse_string(o_string *dest, ساختار p_context *ctx, ثابت نویسه *src)
{
	ساختار in_str foo;
	setup_string_in_str(&foo, src);
	بازگشت parse_stream(dest, ctx, &foo, '\0');
}
#endif

/* return code is 0 for normal exit, 1 for syntax error */
ایستا صحیح parse_stream(o_string *dest, ساختار p_context *ctx,
			ساختار in_str *درون_داد, صحیح end_trigger)
{
	unsigned صحیح ch, m;
#ifndef __U_BOOT__
	صحیح redir_fd;
	redir_type redir_style;
#endif
	صحیح next;

	/* Only double-quote state is handled in the state variable dest->quote.
	 * A single-quote triggers a bypass of the main loop until its mate is
	 * found.  When recursing, quote state is passed in via dest->quote. */

	debug_printf("parse_stream, end_trigger=%d\n",end_trigger);
	وقتی ((ch=b_getch(درون_داد))!=EOF) {
		m = map[ch];
#ifdef __U_BOOT__
		اگر (درون_داد->__promptme == 0) بازگشت 1;
#endif
		next = (ch == '\n') ? 0 : b_peek(درون_داد);

		debug_printf("parse_stream: ch=%c (%d) m=%d quote=%d - %c\n",
			ch >= ' ' ? ch : '.', ch, m,
			dest->quote, ctx->پشته == پوچ ? '*' : '.');

		اگر (m==0 || ((m==1 || m==2) && dest->quote)) {
			b_addqchr(dest, ch, dest->quote);
		} وگرنه {
			اگر (m==2) {  /* unquoted IFS */
				اگر (done_word(dest, ctx)) {
					بازگشت 1;
				}
				/* If we aren't performing a substitution, treat a newline as a
				 * command separator.  */
				اگر (end_trigger != '\0' && ch=='\n')
					done_pipe(ctx,PIPE_SEQ);
			}
			اگر (ch == end_trigger && !dest->quote && ctx->w==RES_NONE) {
				debug_printf("leaving parse_stream (triggered)\n");
				بازگشت 0;
			}
#اگر 0
			اگر (ch=='\n') {
				/* Yahoo!  Time to run with it! */
				done_pipe(ctx,PIPE_SEQ);
				run_list(ctx->list_head);
				initialize_context(ctx);
			}
#endif
			اگر (m!=2) انتخاب (ch) {
		حالت '#':
			اگر (dest->length == 0 && !dest->quote) {
				وقتی(ch=b_peek(درون_داد),ch!=EOF && ch!='\n') { b_getch(درون_داد); }
			} وگرنه {
				b_addqchr(dest, ch, dest->quote);
			}
			بشکن;
		حالت '\\':
			اگر (next == EOF) {
				syntax();
				بازگشت 1;
			}
			b_addqchr(dest, '\\', dest->quote);
			b_addqchr(dest, b_getch(درون_داد), dest->quote);
			بشکن;
		حالت '$':
			اگر (handle_dollar(dest, ctx, درون_داد)!=0) بازگشت 1;
			بشکن;
		حالت '\'':
			dest->nonnull = 1;
			وقتی(ch=b_getch(درون_داد),ch!=EOF && ch!='\'') {
#ifdef __U_BOOT__
				اگر(درون_داد->__promptme == 0) بازگشت 1;
#endif
				b_addchr(dest,ch);
			}
			اگر (ch==EOF) {
				syntax();
				بازگشت 1;
			}
			بشکن;
		حالت '"':
			dest->nonnull = 1;
			dest->quote = !dest->quote;
			بشکن;
#ifndef __U_BOOT__
		حالت '`':
			process_command_subs(dest, ctx, درون_داد, '`');
			بشکن;
		حالت '>':
			redir_fd = redirect_opt_رقم(dest);
			done_word(dest, ctx);
			redir_style=REDIRECT_OVERWRITE;
			اگر (next == '>') {
				redir_style=REDIRECT_APPEND;
				b_getch(درون_داد);
			} وگرنه اگر (next == '(') {
				syntax();   /* until we support >(list) Process Substitution */
				بازگشت 1;
			}
			setup_redirect(ctx, redir_fd, redir_style, درون_داد);
			بشکن;
		حالت '<':
			redir_fd = redirect_opt_رقم(dest);
			done_word(dest, ctx);
			redir_style=REDIRECT_درون_داد;
			اگر (next == '<') {
				redir_style=REDIRECT_HEREIS;
				b_getch(درون_داد);
			} وگرنه اگر (next == '>') {
				redir_style=REDIRECT_IO;
				b_getch(درون_داد);
			} وگرنه اگر (next == '(') {
				syntax();   /* until we support <(list) Process Substitution */
				بازگشت 1;
			}
			setup_redirect(ctx, redir_fd, redir_style, درون_داد);
			بشکن;
#endif
		حالت ';':
			done_word(dest, ctx);
			done_pipe(ctx,PIPE_SEQ);
			بشکن;
		حالت '&':
			done_word(dest, ctx);
			اگر (next=='&') {
				b_getch(درون_داد);
				done_pipe(ctx,PIPE_AND);
			} وگرنه {
#ifndef __U_BOOT__
				done_pipe(ctx,PIPE_BG);
#وگرنه
				syntax_err();
				بازگشت 1;
#endif
			}
			بشکن;
		حالت '|':
			done_word(dest, ctx);
			اگر (next=='|') {
				b_getch(درون_داد);
				done_pipe(ctx,PIPE_OR);
			} وگرنه {
				/* we could pick up a file descriptor choice here
				 * with redirect_opt_num(), but bash doesn't do it.
				 * "echo foo 2| cat" yields "foo 2". */
#ifndef __U_BOOT__
				done_command(ctx);
#وگرنه
				syntax_err();
				بازگشت 1;
#endif
			}
			بشکن;
#ifndef __U_BOOT__
		حالت '(':
		حالت '{':
			اگر (parse_group(dest, ctx, درون_داد, ch)!=0) بازگشت 1;
			بشکن;
		حالت ')':
		حالت '}':
			syntax();   /* Proper use of this character caught by end_trigger */
			بازگشت 1;
			بشکن;
#endif
		حالت SUBSTED_VAR_نماد:
			dest->nonnull = 1;
			وقتی (ch = b_getch(درون_داد), ch != EOF &&
			    ch != SUBSTED_VAR_نماد) {
				debug_printf("subst, pass=%d\n", ch);
				اگر (درون_داد->__promptme == 0)
					بازگشت 1;
				b_addchr(dest, ch);
			}
			debug_printf("subst, term=%d\n", ch);
			اگر (ch == EOF) {
				syntax();
				بازگشت 1;
			}
			بشکن;
		پیشفرض:
			syntax();   /* this is really an internal logic error */
			بازگشت 1;
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
	اگر (end_trigger != '\0') بازگشت -1;
	بازگشت 0;
}

ایستا تهی mapset(ثابت unsigned نویسه *set, صحیح code)
{
	ثابت unsigned نویسه *s;
	برای (s=set; *s; s++) map[*s] = code;
}

ایستا تهی update_ifs_map(تهی)
{
	/* char *ifs and char map[256] are both globals. */
	ifs = (uchar *)env_get("IFS");
	اگر (ifs == پوچ) ifs=(uchar *)" \t\n";
	/* Precompute a list of 'flow through' behavior so it can be treated
	 * quickly up front.  Computation is necessary because of IFS.
	 * Special case handling of IFS == " \t\n" is not implemented.
	 * The map[] array only really needs two bits each, and on most machines
	 * that would be faster because of the reduced L1 cache footprint.
	 */
	memset(map,0,اندازه(map)); /* most characters flow through always */
#ifndef __U_BOOT__
	mapset((uchar *)"\\$'\"`", 3);      /* never flow through */
	mapset((uchar *)"<>;&|(){}#", 1);   /* flow through if quoted */
#وگرنه
	{
		uchar subst[2] = {SUBSTED_VAR_نماد, 0};
		mapset(subst, 3);       /* never flow through */
	}
	mapset((uchar *)"\\$'\"", 3);       /* never flow through */
	mapset((uchar *)";&|#", 1);         /* flow through if quoted */
#endif
	mapset(ifs, 2);            /* also flow through if quoted */
}

/* most recursion does not come through here, the exeception is
 * from builtin_source() */
ایستا صحیح parse_stream_outer(ساختار in_str *inp, صحیح flag)
{

	ساختار p_context ctx;
	o_string temp=NULL_O_STRING;
	صحیح rcode;
#ifdef __U_BOOT__
	صحیح code = 1;
#endif
	انجامبده {
		ctx.type = flag;
		initialize_context(&ctx);
		update_ifs_map();
		اگر (!(flag & FLAG_PARSE_SEMICOLON) || (flag & FLAG_REPARSING)) mapset((uchar *)";$&|", 0);
		inp->promptmode=1;
		rcode = parse_stream(&temp, &ctx, inp,
				     flag & FLAG_CONT_ON_NEWLINE ? -1 : '\n');
#ifdef __U_BOOT__
		اگر (rcode == 1) flag_repeat = 0;
#endif
		اگر (rcode != 1 && ctx.old_flag != 0) {
			syntax();
#ifdef __U_BOOT__
			flag_repeat = 0;
#endif
		}
		اگر (rcode != 1 && ctx.old_flag == 0) {
			done_word(&temp, &ctx);
			done_pipe(&ctx,PIPE_SEQ);
#ifndef __U_BOOT__
			run_list(ctx.list_head);
#وگرنه
			code = run_list(ctx.list_head);
			اگر (code == -2) {	/* exit */
				b_free(&temp);
				code = 0;
				/* XXX hackish way to not allow exit from main loop */
				اگر (inp->peek == file_peek) {
					printf("exit not allowed from main input shell.\n");
					ادامه;
				}
				/*
				 * DANGER
				 * Return code -2 is special in this context,
				 * it indicates exit from inner pipe instead
				 * of return code itself, the return code is
				 * stored in 'last_return_code' variable!
				 * DANGER
				 */
				بازگشت -2;
			}
			اگر (code == -1)
			    flag_repeat = 0;
#endif
		} وگرنه {
			اگر (ctx.old_flag != 0) {
				آزاد(ctx.پشته);
				b_reset(&temp);
			}
#ifdef __U_BOOT__
			اگر (inp->__promptme == 0) printf("<INTERRUPT>\n");
			inp->__promptme = 1;
#endif
			temp.nonnull = 0;
			temp.quote = 0;
			inp->p = پوچ;
			free_pipe_list(ctx.list_head,0);
		}
		b_free(&temp);
	/* loop on syntax errors, return on EOF */
	} وقتی (rcode != -1 && !(flag & FLAG_EXIT_FROM_LOOP) &&
		(inp->peek != static_peek || b_peek(inp)));
#ifndef __U_BOOT__
	بازگشت 0;
#وگرنه
	بازگشت (code != 0) ? 1 : 0;
#endif /* __U_BOOT__ */
}

#ifndef __U_BOOT__
ایستا صحیح parse_string_outer(ثابت نویسه *s, صحیح flag)
#وگرنه
صحیح parse_string_outer(ثابت نویسه *s, صحیح flag)
#endif	/* __U_BOOT__ */
{
	ساختار in_str درون_داد;
	صحیح rcode;
#ifdef __U_BOOT__
	نویسه *p = پوچ;
	اگر (!s)
		بازگشت 1;
	اگر (!*s)
		بازگشت 0;
	اگر (!(p = strchr(s, '\n')) || *++p) {
		p = xmalloc(strlen(s) + 2);
		strcpy(p, s);
		strcat(p, "\n");
		setup_string_in_str(&درون_داد, p);
		rcode = parse_stream_outer(&درون_داد, flag);
		آزاد(p);
		بازگشت rcode == -2 ? last_return_code : rcode;
	} وگرنه {
#endif
	setup_string_in_str(&درون_داد, s);
	rcode = parse_stream_outer(&درون_داد, flag);
	بازگشت rcode == -2 ? last_return_code : rcode;
#ifdef __U_BOOT__
	}
#endif
}

#ifndef __U_BOOT__
ایستا صحیح parse_file_outer(FILE *f)
#وگرنه
صحیح parse_file_outer(تهی)
#endif
{
	صحیح rcode;
	ساختار in_str درون_داد;
#ifndef __U_BOOT__
	setup_file_in_str(&درون_داد, f);
#وگرنه
	setup_file_in_str(&درون_داد);
#endif
	rcode = parse_stream_outer(&درون_داد, FLAG_PARSE_SEMICOLON);
	بازگشت rcode == -2 ? last_return_code : rcode;
}

#ifdef __U_BOOT__
صحیح u_boot_hush_start(تهی)
{
	اگر (بالاترین_vars == پوچ) {
		بالاترین_vars = malloc(اندازه(ساختار variables));
		بالاترین_vars->name = "HUSH_VERSION";
		بالاترین_vars->مقدار = "0.01";
		بالاترین_vars->next = پوچ;
		بالاترین_vars->flg_export = 0;
		بالاترین_vars->flg_read_only = 1;
	}
	بازگشت 0;
}

ایستا تهی *xmalloc(size_t size)
{
	تهی *p = پوچ;

	اگر (!(p = malloc(size))) {
	    printf("ERROR : xmalloc failed\n");
	    برای(;;);
	}
	بازگشت p;
}

ایستا تهی *xrealloc(تهی *ptr, size_t size)
{
	تهی *p = پوچ;

	اگر (!(p = realloc(ptr, size))) {
	    printf("ERROR : xrealloc failed\n");
	    برای(;;);
	}
	بازگشت p;
}
#endif /* __U_BOOT__ */

#ifndef __U_BOOT__
/* Make sure we have a controlling tty.  If we get started under a job
 * aware app (like bash for example), make sure we are now in charge so
 * we don't fight over who gets the foreground */
ایستا تهی setup_job_control(تهی)
{
	ایستا pid_t shell_pgrp;
	/* Loop until we are in the foreground.  */
	وقتی (tcgetpgrp (shell_terminal) != (shell_pgrp = getpgrp ()))
		kill (- shell_pgrp, SIGTTIN);

	/* Ignore interactive and job-control signals.  */
	سیگنال(SIGINT, SIG_IGN);
	سیگنال(SIGQUIT, SIG_IGN);
	سیگنال(SIGTERM, SIG_IGN);
	سیگنال(SIGTSTP, SIG_IGN);
	سیگنال(SIGTTIN, SIG_IGN);
	سیگنال(SIGTTOU, SIG_IGN);
	سیگنال(SIGCHLD, SIG_IGN);

	/* Put ourselves in our own process group.  */
	setsid();
	shell_pgrp = getpid ();
	setpgid (shell_pgrp, shell_pgrp);

	/* Grab control of the terminal.  */
	tcsetpgrp(shell_terminal, shell_pgrp);
}

صحیح hush_main(صحیح argc, نویسه * ثابت *argv)
{
	صحیح opt;
	FILE *درون_داد;
	نویسه **e = environ;

	/* XXX what should these be while sourcing /etc/profile? */
	global_argc = argc;
	global_argv = argv;

	/* (re?) initialize globals.  Sometimes hush_main() ends up calling
	 * hush_main(), therefore we cannot rely on the BSS to zero out this
	 * stuff.  Reset these to 0 every time. */
	ifs = پوچ;
	/* map[] is taken care of with call to update_ifs_map() */
	fake_mode = 0;
	interactive = 0;
	close_me_head = پوچ;
	last_bg_pid = 0;
	job_list = پوچ;
	last_jobid = 0;

	/* Initialize some more globals to non-zero values */
	set_cwd();
#ifdef CONFIG_FEATURE_COMMAND_EDITING
	cmdedit_set_initial_prompt();
#وگرنه
	PS1 = پوچ;
#endif
	PS2 = "> ";

	/* initialize our shell local variables with the values
	 * currently living in the environment */
	اگر (e) {
		برای (; *e; e++)
			set_local_var(*e, 2);   /* without call putenv() */
	}

	last_return_code=EXIT_SUCCESS;

	اگر (argv[0] && argv[0][0] == '-') {
		debug_printf("\nsourcing /etc/profile\n");
		اگر ((درون_داد = fopen("/etc/profile", "r")) != پوچ) {
			mark_open(fileno(درون_داد));
			parse_file_outer(درون_داد);
			mark_closed(fileno(درون_داد));
			fclose(درون_داد);
		}
	}
	درون_داد=stdin;

	وقتی ((opt = getopt(argc, argv, "c:xif")) > 0) {
		انتخاب (opt) {
			حالت 'c':
				{
					global_argv = argv+optind;
					global_argc = argc-optind;
					opt = parse_string_outer(optarg, FLAG_PARSE_SEMICOLON);
					برو final_return;
				}
				بشکن;
			حالت 'i':
				interactive++;
				بشکن;
			حالت 'f':
				fake_mode++;
				بشکن;
			پیشفرض:
#ifndef BB_VER
				fprintf(stderr, "Usage: sh [FILE]...\n"
						"   or: sh -c command [args]...\n\n");
				exit(EXIT_FAILURE);
#وگرنه
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
	اگر (argv[optind]==پوچ && درون_داد==stdin &&
			isatty(fileno(stdin)) && isatty(fileno(stdout))) {
		interactive++;
	}

	debug_printf("\ninteractive=%d\n", interactive);
	اگر (interactive) {
		/* Looks like they want an interactive shell */
#ifndef CONFIG_FEATURE_SH_EXTRA_QUIET
		printf( "\n\n" BB_BANNER " hush - the humble shell v0.01 (testing)\n");
		printf( "Enter 'help' for a list of built-in commands.\n\n");
#endif
		setup_job_control();
	}

	اگر (argv[optind]==پوچ) {
		opt=parse_file_outer(stdin);
		برو final_return;
	}

	debug_printf("\nrunning script '%s'\n", argv[optind]);
	global_argv = argv+optind;
	global_argc = argc-optind;
	درون_داد = xfopen(argv[optind], "r");
	opt = parse_file_outer(درون_داد);

#ifdef CONFIG_FEATURE_CLEAN_UP
	fclose(درون_داد);
	اگر (cwd && cwd != unknown)
		آزاد((نویسه*)cwd);
	{
		ساختار variables *cur, *tmp;
		برای(cur = بالاترین_vars; cur; cur = tmp) {
			tmp = cur->next;
			اگر (!cur->flg_read_only) {
				آزاد(cur->name);
				آزاد(cur->مقدار);
				آزاد(cur);
			}
		}
	}
#endif

final_return:
	بازگشت(opt?opt:last_return_code);
}
#endif

ایستا نویسه *درج_var_مقدار(نویسه *inp)
{
	بازگشت درج_var_مقدار_sub(inp, 0);
}

ایستا نویسه *درج_var_مقدار_sub(نویسه *inp, صحیح tag_subst)
{
	صحیح res_str_len = 0;
	صحیح len;
	صحیح done = 0;
	نویسه *p, *p1, *res_str = پوچ;

	وقتی ((p = strchr(inp, SPECIAL_VAR_نماد))) {
		/* check the beginning of the string for normal characters */
		اگر (p != inp) {
			/* copy any characters to the result string */
			len = p - inp;
			res_str = xrealloc(res_str, (res_str_len + len));
			strncpy((res_str + res_str_len), inp, len);
			res_str_len += len;
		}
		inp = ++p;
		/* find the ending marker */
		p = strchr(inp, SPECIAL_VAR_نماد);
		*p = '\0';
		/* look up the value to substitute */
		اگر ((p1 = lookup_param(inp))) {
			اگر (tag_subst)
				len = res_str_len + strlen(p1) + 2;
			وگرنه
				len = res_str_len + strlen(p1);
			res_str = xrealloc(res_str, (1 + len));
			اگر (tag_subst) {
				/*
				 * copy the variable value to the result
				 * string
				 */
				strcpy((res_str + res_str_len + 1), p1);

				/*
				 * mark the replaced text to be accepted as
				 * is
				 */
				res_str[res_str_len] = SUBSTED_VAR_نماد;
				res_str[res_str_len + 1 + strlen(p1)] =
					SUBSTED_VAR_نماد;
			} وگرنه
				/*
				 * copy the variable value to the result
				 * string
				 */
				strcpy((res_str + res_str_len), p1);

			res_str_len = len;
		}
		*p = SPECIAL_VAR_نماد;
		inp = ++p;
		done = 1;
	}
	اگر (done) {
		res_str = xrealloc(res_str, (1 + res_str_len + strlen(inp)));
		strcpy((res_str + res_str_len), inp);
		وقتی ((p = strchr(res_str, '\n'))) {
			*p = ' ';
		}
	}
	بازگشت (res_str == پوچ) ? inp : res_str;
}

ایستا نویسه **make_list_in(نویسه **inp, نویسه *name)
{
	صحیح len, i;
	صحیح name_len = strlen(name);
	صحیح n = 0;
	نویسه **list;
	نویسه *p1, *p2, *p3;

	/* create list of variable values */
	list = xmalloc(اندازه(*list));
	برای (i = 0; inp[i]; i++) {
		p3 = درج_var_مقدار(inp[i]);
		p1 = p3;
		وقتی (*p1) {
			اگر (*p1 == ' ') {
				p1++;
				ادامه;
			}
			اگر ((p2 = strchr(p1, ' '))) {
				len = p2 - p1;
			} وگرنه {
				len = strlen(p1);
				p2 = p1 + len;
			}
			/* we use n + 2 in realloc for list,because we add
			 * new element and then we will add NULL element */
			list = xrealloc(list, اندازه(*list) * (n + 2));
			list[n] = xmalloc(2 + name_len + len);
			strcpy(list[n], name);
			strcat(list[n], "=");
			strncat(list[n], p1, len);
			list[n++][name_len + len + 1] = '\0';
			p1 = p2;
		}
		اگر (p3 != inp[i]) آزاد(p3);
	}
	list[n] = پوچ;
	بازگشت list;
}

/*
 * Make new string for parser
 * inp     - array of argument strings to flatten
 * nonnull - indicates argument was quoted when originally parsed
 */
ایستا نویسه *make_string(نویسه **inp, صحیح *nonnull)
{
	نویسه *p;
	نویسه *str = پوچ;
	صحیح n;
	صحیح len = 2;
	نویسه *noeval_str;
	صحیح noeval = 0;

	noeval_str = get_local_var("HUSH_NO_EVAL");
	اگر (noeval_str != پوچ && *noeval_str != '0' && *noeval_str != '\0')
		noeval = 1;
	برای (n = 0; inp[n]; n++) {
		p = درج_var_مقدار_sub(inp[n], noeval);
		نویسه *new_str = xrealloc(str, (len + strlen(p) + (2 * nonnull[n])));
		اگر (!new_str) {
			آزاد(str);
			اگر (p != inp[n]) آزاد(p);
			بازگشت پوچ;
		}
		str = new_str;
		اگر (n) {
			strcat(str, " ");
		} وگرنه {
			*str = '\0';
		}
		اگر (nonnull[n])
			strcat(str, "'");
		strcat(str, p);
		اگر (nonnull[n])
			strcat(str, "'");
		len = strlen(str) + 3;
		اگر (p != inp[n]) آزاد(p);
	}
	len = strlen(str);
	*(str + len) = '\n';
	*(str + len + 1) = '\0';
	بازگشت str;
}

#ifdef __U_BOOT__
ایستا صحیح do_showvar(ساختار cmd_tbl *cmdtp, صحیح flag, صحیح argc,
		      نویسه *ثابت argv[])
{
	صحیح i, k;
	صحیح rcode = 0;
	ساختار variables *cur;

	اگر (argc == 1) {		/* Print all env variables	*/
		برای (cur = بالاترین_vars; cur; cur = cur->next) {
			printf ("%s=%s\n", cur->name, cur->مقدار);
			اگر (ctrlc ()) {
				puts ("\n ** Abort\n");
				بازگشت 1;
			}
		}
		بازگشت 0;
	}
	برای (i = 1; i < argc; ++i) {	/* print single env variables	*/
		نویسه *name = argv[i];

		k = -1;
		برای (cur = بالاترین_vars; cur; cur = cur->next) {
			اگر(strcmp (cur->name, name) == 0) {
				k = 0;
				printf ("%s=%s\n", cur->name, cur->مقدار);
			}
			اگر (ctrlc ()) {
				puts ("\n ** Abort\n");
				بازگشت 1;
			}
		}
		اگر (k < 0) {
			printf ("## Error: \"%s\" not defined\n", name);
			rcode ++;
		}
	}
	بازگشت rcode;
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
