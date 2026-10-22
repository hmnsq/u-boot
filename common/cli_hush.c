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

#تعریف_کن __U_BOOT__
#اگر_تعریف_شده __U_BOOT__
#ضمن <env.h>
#ضمن <malloc.h>         /* malloc, free, realloc*/
#ضمن <linux/ctype.h>    /* isalpha, isdigit */
#ضمن <console.h>
#ضمن <bootretry.h>
#ضمن <cli.h>
#ضمن <cli_hush.h>
#ضمن <command.h>        /* find_cmd */
#ضمن <vsprintf.h>
#پایان_اگر
#اگر_تعریف_نشده __U_BOOT__
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
#تعریف_کن applet_name "hush"
#ضمن "standalone.h"
#تعریف_کن hush_main اصلی
#تعریف_کن BB_BANNER
#پایان_اگر
#پایان_اگر
#تعریف_کن SPECIAL_VAR_نماد 03
#تعریف_کن SUBSTED_VAR_نماد 04
#اگر_تعریف_نشده __U_BOOT__
#تعریف_کن FLAG_EXIT_FROM_LOOP 1
#تعریف_کن FLAG_PARSE_SEMICOLON (1 << 1)		/* symbol ';' is special for parser */
#تعریف_کن FLAG_REPARSING       (1 << 2)		/* >= 2nd pass */

#پایان_اگر

#اگر_تعریف_شده __U_BOOT__
#تعریف_کن EXIT_SUCCESS 0
#تعریف_کن EOF -1
#تعریف_کن syntax() syntax_err()
#تعریف_کن xstrdup strdup
#تعریف_کن error_پیام چاپ_کن
#وإلا
تعريفنوع تعداد {
	REDIRECT_درون_داد     = 1,
	REDIRECT_OVERWRITE = 2,
	REDIRECT_APPEND    = 3,
	REDIRECT_HEREIS    = 4,
	REDIRECT_IO        = 5
} redir_type;

/* The descrip member of this structure is only used to make debugging
 * output pretty */
بنية {صحيح نما; صحيح default_fd; حرف *descrip;} redir_جدول[] = {
	{ 0,                         0, "()" },
	{ O_RDONLY,                  0, "<"  },
	{ O_CREAT|O_TRUNC|O_WRONLY,  1, ">"  },
	{ O_CREAT|O_APPEND|O_WRONLY, 1, ">>" },
	{ O_RDONLY,                 -1, "<<" },
	{ O_RDWR,                    1, "<>" }
};
#پایان_اگر

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
	RES_انجامشده  = 10,
	RES_XXXX  = 11,
	RES_درون    = 12,
	RES_SNTX  = 13
} reserved_style;
#تعریف_کن FLAG_انتها   (1<<RES_NONE)
#تعریف_کن FLAG_IF    (1<<RES_IF)
#تعریف_کن FLAG_THEN  (1<<RES_THEN)
#تعریف_کن FLAG_ELIF  (1<<RES_ELIF)
#تعریف_کن FLAG_ELSE  (1<<RES_ELSE)
#تعریف_کن FLAG_FI    (1<<RES_FI)
#تعریف_کن FLAG_FOR   (1<<RES_FOR)
#تعریف_کن FLAG_WHILE (1<<RES_WHILE)
#تعریف_کن FLAG_UNTIL (1<<RES_UNTIL)
#تعریف_کن FLAG_DO    (1<<RES_DO)
#تعریف_کن FLAG_انجامشده  (1<<RES_انجامشده)
#تعریف_کن FLAG_درون    (1<<RES_درون)
#تعریف_کن FLAG_شروع (1<<RES_XXXX)

/* This holds pointers to the various results of parsing */
بنية p_context {
	بنية فرزند_prog *فرزند;
	بنية pipe *list_سر;
	بنية pipe *pipe;
#اگر_تعریف_نشده __U_BOOT__
	بنية redir_struct *pending_redirect;
#پایان_اگر
	reserved_style w;
	صحيح old_flag;				/* for figuring out valid reserved words */
	بنية p_context *پشته;
	صحيح type;			/* define type of parser : ";$" common or special symbol */
	/* How about quoting status? */
};

#اگر_تعریف_نشده __U_BOOT__
بنية redir_struct {
	redir_type type;			/* type of redirection */
	صحيح fd;						/* file descriptor being redirected */
	صحيح dup;					/* -1, or file descriptor being duplicated */
	بنية redir_struct *بعدی;	/* pointer to the next redirect in the list */
	glob_t کلمه;				/* *word.gl_pathv is the filename */
};
#پایان_اگر

بنية فرزند_prog {
#اگر_تعریف_نشده __U_BOOT__
	pid_t pid;					/* 0 if exited */
#پایان_اگر
	حرف **argv;				/* program name and arguments */
	/* was quoted when parsed; copy of struct o_string.nonnull field */
	صحيح *argv_nonnull;
#اگر_تعریف_شده __U_BOOT__
	صحيح    argc;                            /* number of program arguments */
#پایان_اگر
	بنية pipe *گروه;			/* if non-NULL, first in group or subshell */
#اگر_تعریف_نشده __U_BOOT__
	صحيح subshell;				/* flag, non-zero if group must be forked */
	بنية redir_struct *redirects;	/* I/O redirections */
	glob_t glob_نتیجه;			/* result of parameter globbing */
	صحيح is_stopped;				/* is the program currently running? */
	بنية pipe *family;		/* pointer back to the child's parent pipe */
#پایان_اگر
	صحيح sp;				/* number of SPECIAL_VAR_SYMBOL */
	صحيح type;
};

بنية pipe {
#اگر_تعریف_نشده __U_BOOT__
	صحيح jobid;					/* job number */
#پایان_اگر
	صحيح رقم_progs;				/* total number of programs in job */
#اگر_تعریف_نشده __U_BOOT__
	صحيح running_progs;			/* number of programs running */
	حرف *متن;					/* name of job */
	حرف *cmdbuf;				/* buffer various argv's point into */
	pid_t pgrp;					/* process group ID for the job */
#پایان_اگر
	بنية فرزند_prog *progs;	/* array of commands in pipe */
	بنية pipe *بعدی;			/* to track background commands */
#اگر_تعریف_نشده __U_BOOT__
	صحيح stopped_progs;			/* number of programs alive, but stopped */
	صحيح کار_context;			/* bitmask defining current context */
#پایان_اگر
	pipe_style followup;		/* PIPE_BG, PIPE_SEQ, PIPE_OR, PIPE_AND */
	reserved_style r_نما;		/* supports if, for, while, until */
};

#اگر_تعریف_نشده __U_BOOT__
بنية close_me {
	صحيح fd;
	بنية close_me *بعدی;
};
#پایان_اگر

بنية variables {
	حرف *name;
	حرف *مقدار;
	صحيح flg_export;
	صحيح flg_read_only;
	بنية variables *بعدی;
};

/* globals, connect us to the outside world
 * the first three support $?, $#, and $1 */
#اگر_تعریف_نشده __U_BOOT__
حرف **global_argv;
بیعلامت صحيح global_argc;
#پایان_اگر
ساكن بیعلامت صحيح آخرین_return_کد;
#اگر_تعریف_نشده __U_BOOT__
خارجي حرف **environ; /* This is in <unistd.h>, but protected with __USE_GNU */
#پایان_اگر

/* "globals" within this file */
ساكن uchar *ifs;
ساكن حرف map[256];
#اگر_تعریف_نشده __U_BOOT__
ساكن صحيح fake_نما;
ساكن صحيح interactive;
ساكن بنية close_me *close_me_سر;
ساكن ثابت حرف *cwd;
ساكن بنية pipe *کار_list;
ساكن بیعلامت صحيح آخرین_bg_pid;
ساكن بیعلامت صحيح آخرین_jobid;
ساكن بیعلامت صحيح shell_terminal;
ساكن حرف *PS1;
ساكن حرف *PS2;
بنية variables shell_ver = { "HUSH_VERSION", "0.01", 1, 1, 0 };
بنية variables *بالاترین_vars = &shell_ver;
#وإلا
ساكن صحيح flag_repeat = 0;
ساكن صحيح do_repeat = 0;
ساكن بنية variables *بالاترین_vars = عدم ;
#پایان_اگر /*__U_BOOT__ */

#تعریف_کن B_CHUNK (100)
#تعریف_کن B_NOSPAC 1

تعريفنوع بنية {
	حرف *داده;
	صحيح درازی;
	صحيح maxlen;
	صحيح quote;
	صحيح nonnull;
} o_string;
#تعریف_کن NULL_O_STRING {عدم,0,0,0,0}
/* used for initialization:
	o_string foo = NULL_O_STRING; */

/* I can almost use ordinary FILE *.  Is open_memstream() universally
 * available?  Where is it documented? */
بنية درون_str {
	ثابت بیعلامت حرف *p;
#اگر_تعریف_نشده __U_BOOT__
	حرف peek_buf[2];
#پایان_اگر
	صحيح __promptme;
	صحيح promptmode;
#اگر_تعریف_نشده __U_BOOT__
	FILE *file;
#پایان_اگر
	صحيح (*get) (بنية درون_str *);
	صحيح (*peek) (بنية درون_str *);
};
#تعریف_کن b_getch(درون_داد) ((درون_داد)->get(درون_داد))
#تعریف_کن b_peek(درون_داد) ((درون_داد)->peek(درون_داد))

#اگر_تعریف_نشده __U_BOOT__
#تعریف_کن کار_وضعیت_FORMAT "[%d] %-22s %.40s\n"

بنية built_درون_command {
	حرف *cmd;					/* name */
	حرف *descr;				/* description */
	صحيح (*function) (بنية فرزند_prog *);	/* function ptr */
};
#پایان_اگر

/* define DEBUG_SHELL for debugging output (obviously ;-)) */
#إذا 0
#تعریف_کن DEBUG_SHELL
#پایان_اگر

/* This should be in utility.c */
#اگر_تعریف_شده DEBUG_SHELL
#اگر_تعریف_نشده __U_BOOT__
ساكن فراغ debug_printf(ثابت حرف *format, ...)
{
	va_list آرگومانها;
	va_شروع(آرگومانها, format);
	vfprintf(stderr, format, آرگومانها);
	va_انتها(آرگومانها);
}
#وإلا
#تعریف_کن debug_printf(fmt,آرگومانها...)	چاپ_کن (fmt ,##args)
#پایان_اگر
#وإلا
ساكن مضمن فراغ debug_printf(ثابت حرف *format, ...) { }
#پایان_اگر
#تعریف_کن final_printf debug_printf

#اگر_تعریف_شده __U_BOOT__
ساكن فراغ syntax_err(فراغ) {
	 چاپ_کن("syntax error\n");
}
#وإلا
ساكن فراغ __syntax(حرف *file, صحيح سطر) {
	error_پیام("syntax error %s:%d", file, سطر);
}
#تعریف_کن syntax() __syntax(__FILE__, __LINE__)
#پایان_اگر

#اگر_تعریف_شده __U_BOOT__
ساكن فراغ *xmalloc(size_t اندازه);
ساكن فراغ *xrealloc(فراغ *ptr, size_t اندازه);
#وإلا
/* Index of subroutines: */
/*   function prototypes for builtins */
ساكن صحيح builtin_cd(بنية فرزند_prog *فرزند);
ساكن صحيح builtin_env(بنية فرزند_prog *فرزند);
ساكن صحيح builtin_eval(بنية فرزند_prog *فرزند);
ساكن صحيح builtin_exec(بنية فرزند_prog *فرزند);
ساكن صحيح builtin_exit(بنية فرزند_prog *فرزند);
ساكن صحيح builtin_export(بنية فرزند_prog *فرزند);
ساكن صحيح builtin_fg_bg(بنية فرزند_prog *فرزند);
ساكن صحيح builtin_help(بنية فرزند_prog *فرزند);
ساكن صحيح builtin_jobs(بنية فرزند_prog *فرزند);
ساكن صحيح builtin_pwd(بنية فرزند_prog *فرزند);
ساكن صحيح builtin_read(بنية فرزند_prog *فرزند);
ساكن صحيح builtin_مجموعه(بنية فرزند_prog *فرزند);
ساكن صحيح builtin_لغزش(بنية فرزند_prog *فرزند);
ساكن صحيح builtin_منبع(بنية فرزند_prog *فرزند);
ساكن صحيح builtin_umask(بنية فرزند_prog *فرزند);
ساكن صحيح builtin_unset(بنية فرزند_prog *فرزند);
ساكن صحيح builtin_not_written(بنية فرزند_prog *فرزند);
#پایان_اگر
/*   o_string manipulation: */
ساكن صحيح b_check_space(o_string *o, صحيح len);
ساكن صحيح b_addchr(o_string *o, صحيح ch);
ساكن فراغ b_reset(o_string *o);
ساكن صحيح b_addqchr(o_string *o, صحيح ch, صحيح quote);
#اگر_تعریف_نشده __U_BOOT__
ساكن صحيح b_adduint(o_string *o, بیعلامت صحيح i);
#پایان_اگر
/*  in_str manipulations: */
ساكن صحيح static_get(بنية درون_str *i);
ساكن صحيح static_peek(بنية درون_str *i);
ساكن صحيح file_get(بنية درون_str *i);
ساكن صحيح file_peek(بنية درون_str *i);
#اگر_تعریف_نشده __U_BOOT__
ساكن فراغ راهاندازی_file_درون_str(بنية درون_str *i, FILE *f);
#وإلا
ساكن فراغ راهاندازی_file_درون_str(بنية درون_str *i);
#پایان_اگر
ساكن فراغ راهاندازی_string_درون_str(بنية درون_str *i, ثابت حرف *s);
#اگر_تعریف_نشده __U_BOOT__
/*  close_me manipulations: */
ساكن فراغ mark_open(صحيح fd);
ساكن فراغ mark_closed(صحيح fd);
ساكن فراغ close_all(فراغ);
#پایان_اگر
/*  "run" the final data structures: */
ساكن حرف *indenter(صحيح i);
ساكن صحيح free_pipe_list(بنية pipe *سر, صحيح indent);
ساكن صحيح free_pipe(بنية pipe *pi, صحيح indent);
/*  really run the final data structures: */
#اگر_تعریف_نشده __U_BOOT__
ساكن صحيح راهاندازی_redirects(بنية فرزند_prog *prog, صحيح squirrel[]);
#پایان_اگر
ساكن صحيح پیاپی_list_real(بنية pipe *pi);
#اگر_تعریف_نشده __U_BOOT__
ساكن فراغ pseudo_exec(بنية فرزند_prog *فرزند) __attribute__ ((noreturn));
#پایان_اگر
ساكن صحيح پیاپی_pipe_real(بنية pipe *pi);
/*   extended glob support: */
#اگر_تعریف_نشده __U_BOOT__
ساكن صحيح globhack(ثابت حرف *src, صحيح flags, glob_t *pglob);
ساكن صحيح glob_needed(ثابت حرف *s);
ساكن صحيح xglob(o_string *dest, صحيح flags, glob_t *pglob);
#پایان_اگر
/*   variable assignment: */
ساكن صحيح is_assignment(ثابت حرف *s);
/*   data structure manipulation: */
#اگر_تعریف_نشده __U_BOOT__
ساكن صحيح راهاندازی_redirect(بنية p_context *ctx, صحيح fd, redir_type style, بنية درون_str *درون_داد);
#پایان_اگر
ساكن فراغ initialize_context(بنية p_context *ctx);
ساكن صحيح انجامشده_کلمه(o_string *dest, بنية p_context *ctx);
ساكن صحيح انجامشده_command(بنية p_context *ctx);
ساكن صحيح انجامشده_pipe(بنية p_context *ctx, pipe_style type);
/*   primary string parsing: */
#اگر_تعریف_نشده __U_BOOT__
ساكن صحيح redirect_dup_رقم(بنية درون_str *درون_داد);
ساكن صحيح redirect_opt_رقم(o_string *o);
ساكن صحيح process_command_subs(o_string *dest, بنية p_context *ctx, بنية درون_str *درون_داد, صحيح subst_انتها);
ساكن صحيح parse_گروه(o_string *dest, بنية p_context *ctx, بنية درون_str *درون_داد, صحيح ch);
#پایان_اگر
ساكن حرف *lookup_param(حرف *src);
ساكن حرف *make_string(حرف **inp, صحيح *nonnull);
ساكن صحيح handle_dollar(o_string *dest, بنية p_context *ctx, بنية درون_str *درون_داد);
#اگر_تعریف_نشده __U_BOOT__
ساكن صحيح parse_string(o_string *dest, بنية p_context *ctx, ثابت حرف *src);
#پایان_اگر
ساكن صحيح parse_جریان(o_string *dest, بنية p_context *ctx, بنية درون_str *input0, صحيح انتها_trigger);
/*   setup: */
ساكن صحيح parse_جریان_outer(بنية درون_str *inp, صحيح flag);
#اگر_تعریف_نشده __U_BOOT__
ساكن صحيح parse_string_outer(ثابت حرف *s, صحيح flag);
ساكن صحيح parse_file_outer(FILE *f);
#پایان_اگر
#اگر_تعریف_نشده __U_BOOT__
/*   job management: */
ساكن صحيح checkjobs(بنية pipe* fg_pipe);
ساكن فراغ درج_bg_کار(بنية pipe *pi);
ساكن فراغ remove_bg_کار(بنية pipe *pi);
#پایان_اگر
/*     local variable support */
ساكن حرف **make_list_درون(حرف **inp, حرف *name);
ساكن حرف *درج_var_مقدار(حرف *inp);
ساكن حرف *درج_var_مقدار_sub(حرف *inp, صحيح برچسب_subst);

#اگر_تعریف_نشده __U_BOOT__
/* Table of built-in functions.  They can be forked or not, depending on
 * context: within pipes, they fork.  As simple commands, they do not.
 * When used in non-forking context, they can change global variables
 * in the parent shell process.  If forked, of course they can not.
 * For example, 'unset foo | whatever' will parse and run, but foo will
 * still be set at the end. */
ساكن بنية built_درون_command bltins[] = {
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
	{"set", "Set/unset shell local variables", builtin_مجموعه},
	{"shift", "Shift positional parameters", builtin_لغزش},
	{"trap", "Trap signals", builtin_not_written},
	{"ulimit","Controls resource limits", builtin_not_written},
	{"umask","Sets file creation mask", builtin_umask},
	{"unset", "Unset environment variable", builtin_unset},
	{".", "Source-in and run commands in a file", builtin_منبع},
	{"help", "List shell built-in commands", builtin_help},
	{عدم, عدم, عدم}
};

ساكن ثابت حرف *مجموعه_cwd(فراغ)
{
	إذا(cwd==unknown)
		cwd = عدم;     /* xgetcwd(arg) called free(arg) */
	cwd = xgetcwd((حرف *)cwd);
	إذا (!cwd)
		cwd = unknown;
	أرجع cwd;
}

/* built-in 'eval' handler */
ساكن صحيح builtin_eval(بنية فرزند_prog *فرزند)
{
	حرف *str = عدم;
	صحيح rcode = EXIT_SUCCESS;

	إذا (فرزند->argv[1]) {
		str = make_string(فرزند->argv + 1);
		parse_string_outer(str, FLAG_EXIT_FROM_LOOP |
					FLAG_PARSE_SEMICOLON);
		حرر(str);
		rcode = آخرین_return_کد;
	}
	أرجع rcode;
}

/* built-in 'cd <path>' handler */
ساكن صحيح builtin_cd(بنية فرزند_prog *فرزند)
{
	حرف *newdir;
	إذا (فرزند->argv[1] == عدم)
		newdir = env_get("HOME");
	وإلا
		newdir = فرزند->argv[1];
	إذا (chdir(newdir)) {
		چاپ_کن("cd: %s: %s\n", newdir, strerror(errno));
		أرجع EXIT_FAILURE;
	}
	مجموعه_cwd();
	أرجع EXIT_SUCCESS;
}

/* built-in 'env' handler */
ساكن صحيح builtin_env(بنية فرزند_prog *dummy)
{
	حرف **e = environ;
	إذا (e == عدم) أرجع EXIT_FAILURE;
	لكل (; *e; e++) {
		puts(*e);
	}
	أرجع EXIT_SUCCESS;
}

/* built-in 'exec' handler */
ساكن صحيح builtin_exec(بنية فرزند_prog *فرزند)
{
	إذا (فرزند->argv[1] == عدم)
		أرجع EXIT_SUCCESS;   /* Really? */
	فرزند->argv++;
	pseudo_exec(فرزند);
	/* never returns */
}

/* built-in 'exit' handler */
ساكن صحيح builtin_exit(بنية فرزند_prog *فرزند)
{
	إذا (فرزند->argv[1] == عدم)
		خروج(آخرین_return_کد);
	خروج (atoi(فرزند->argv[1]));
}

/* built-in 'export VAR=value' handler */
ساكن صحيح builtin_export(بنية فرزند_prog *فرزند)
{
	صحيح res = 0;
	حرف *name = فرزند->argv[1];

	إذا (name == عدم) {
		أرجع (builtin_env(فرزند));
	}

	name = strdup(name);

	إذا(name) {
		حرف *مقدار = strchr(name, '=');

		إذا (!مقدار) {
			حرف *tmp;
			/* They are exporting something without an =VALUE */

			مقدار = get_local_var(name);
			إذا (مقدار) {
				size_t ln = طول_رشته(name);

				tmp = realloc(name, ln+طول_رشته(مقدار)+2);
				إذا(tmp==عدم)
					res = -1;
				وإلا {
					sprintf(tmp+ln, "=%s", مقدار);
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
		perror_پیام("export");
	وإلا إذا(res==0)
		res = مجموعه_local_var(name, 1);
	وإلا
		res = 0;
	حرر(name);
	أرجع res;
}

/* built-in 'fg' and 'bg' handler */
ساكن صحيح builtin_fg_bg(بنية فرزند_prog *فرزند)
{
	صحيح i, jobnum;
	بنية pipe *pi=عدم;

	إذا (!interactive)
		أرجع EXIT_FAILURE;
	/* If they gave us no args, assume they want the last backgrounded task */
	إذا (!فرزند->argv[1]) {
		لكل (pi = کار_list; pi; pi = pi->بعدی) {
			إذا (pi->jobid == آخرین_jobid) {
				اكسر;
			}
		}
		إذا (!pi) {
			error_پیام("%s: no current job", فرزند->argv[0]);
			أرجع EXIT_FAILURE;
		}
	} وإلا {
		إذا (از_رشته_بخوان(فرزند->argv[1], "%%%d", &jobnum) != 1) {
			error_پیام("%s: bad argument '%s'", فرزند->argv[0], فرزند->argv[1]);
			أرجع EXIT_FAILURE;
		}
		لكل (pi = کار_list; pi; pi = pi->بعدی) {
			إذا (pi->jobid == jobnum) {
				اكسر;
			}
		}
		إذا (!pi) {
			error_پیام("%s: %d: no such job", فرزند->argv[0], jobnum);
			أرجع EXIT_FAILURE;
		}
	}

	إذا (*فرزند->argv[0] == 'f') {
		/* Put the job into the foreground.  */
		tcsetpgrp(shell_terminal, pi->pgrp);
	}

	/* Restart the processes in the job */
	لكل (i = 0; i < pi->رقم_progs; i++)
		pi->progs[i].is_stopped = 0;

	إذا ( (i=kill(- pi->pgrp, SIGCONT)) < 0) {
		إذا (i == ESRCH) {
			remove_bg_کار(pi);
		} وإلا {
			perror_پیام("kill (SIGCONT)");
		}
	}

	pi->stopped_progs = 0;
	أرجع EXIT_SUCCESS;
}

/* built-in 'help' handler */
ساكن صحيح builtin_help(بنية فرزند_prog *dummy)
{
	بنية built_درون_command *x;

	چاپ_کن("\nBuilt-in commands:\n");
	چاپ_کن("-------------------\n");
	لكل (x = bltins; x->cmd; x++) {
		إذا (x->descr==عدم)
			استمر;
		چاپ_کن("%s\t%s\n", x->cmd, x->descr);
	}
	چاپ_کن("\n\n");
	أرجع EXIT_SUCCESS;
}

/* built-in 'jobs' handler */
ساكن صحيح builtin_jobs(بنية فرزند_prog *فرزند)
{
	بنية pipe *کار;
	حرف *وضعیت_string;

	لكل (کار = کار_list; کار; کار = کار->بعدی) {
		إذا (کار->running_progs == کار->stopped_progs)
			وضعیت_string = "Stopped";
		وإلا
			وضعیت_string = "Running";

		چاپ_کن(کار_وضعیت_FORMAT, کار->jobid, وضعیت_string, کار->متن);
	}
	أرجع EXIT_SUCCESS;
}

/* built-in 'pwd' handler */
ساكن صحيح builtin_pwd(بنية فرزند_prog *dummy)
{
	puts(مجموعه_cwd());
	أرجع EXIT_SUCCESS;
}

/* built-in 'read VAR' handler */
ساكن صحيح builtin_read(بنية فرزند_prog *فرزند)
{
	صحيح res;

	إذا (فرزند->argv[1]) {
		حرف string[BUFSIZ];
		حرف *var = 0;

		string[0] = 0;  /* In case stdin has only EOF */
		/* read string */
		fgets(string, حجم(string), stdin);
		chomp(string);
		var = حافظه_بده(طول_رشته(فرزند->argv[1])+طول_رشته(string)+2);
		إذا(var) {
			sprintf(var, "%s=%s", فرزند->argv[1], string);
			res = مجموعه_local_var(var, 0);
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
ساكن صحيح builtin_مجموعه(بنية فرزند_prog *فرزند)
{
	حرف *موقت = فرزند->argv[1];
	بنية variables *e;

	إذا (موقت == عدم)
		لكل(e = بالاترین_vars; e; e=e->بعدی)
			چاپ_کن("%s=%s\n", e->name, e->مقدار);
	وإلا
		مجموعه_local_var(موقت, 0);

		أرجع EXIT_SUCCESS;
}

/* Built-in 'shift' handler */
ساكن صحيح builtin_لغزش(بنية فرزند_prog *فرزند)
{
	صحيح n=1;
	إذا (فرزند->argv[1]) {
		n=atoi(فرزند->argv[1]);
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
ساكن صحيح builtin_منبع(بنية فرزند_prog *فرزند)
{
	FILE *درون_داد;
	صحيح وضعیت;

	إذا (فرزند->argv[1] == عدم)
		أرجع EXIT_FAILURE;

	/* XXX search through $PATH is missing */
	درون_داد = fopen(فرزند->argv[1], "r");
	إذا (!درون_داد) {
		error_پیام("Couldn't open file '%s'", فرزند->argv[1]);
		أرجع EXIT_FAILURE;
	}

	/* Now run the file */
	/* XXX argv and argc are broken; need to save old global_argv
	 * (pointer only is OK!) on this stack frame,
	 * set global_argv=child->argv+1, recurse, and restore. */
	mark_open(fileno(درون_داد));
	وضعیت = parse_file_outer(درون_داد);
	mark_closed(fileno(درون_داد));
	fclose(درون_داد);
	أرجع (وضعیت);
}

ساكن صحيح builtin_umask(بنية فرزند_prog *فرزند)
{
	mode_t new_umask;
	ثابت حرف *arg = فرزند->argv[1];
	حرف *انتها_2;
	إذا (arg) {
		new_umask=strtoul(arg, &انتها_2, 8);
		إذا (*انتها_2!='\0' || انتها_2 == arg) {
			أرجع EXIT_FAILURE;
		}
	} وإلا {
		چاپ_کن("%.3o\n", (بیعلامت صحيح) (new_umask=umask(0)));
	}
	umask(new_umask);
	أرجع EXIT_SUCCESS;
}

/* built-in 'unset VAR' handler */
ساكن صحيح builtin_unset(بنية فرزند_prog *فرزند)
{
	/* bash returned already true */
	unset_local_var(فرزند->argv[1]);
	أرجع EXIT_SUCCESS;
}

ساكن صحيح builtin_not_written(بنية فرزند_prog *فرزند)
{
	چاپ_کن("builtin_%s not written\n",فرزند->argv[0]);
	أرجع EXIT_FAILURE;
}
#پایان_اگر

ساكن صحيح b_check_space(o_string *o, صحيح len)
{
	/* It would be easy to drop a more restrictive policy
	 * in here, such as setting a maximum string length */
	إذا (o->درازی + len > o->maxlen) {
		حرف *old_داده = o->داده;
		/* assert (data == NULL || o->maxlen != 0); */
		o->maxlen += max(2*len, B_CHUNK);
		o->داده = realloc(o->داده, 1 + o->maxlen);
		إذا (o->داده == عدم) {
			حرر(old_داده);
		}
	}
	أرجع o->داده == عدم;
}

ساكن صحيح b_addchr(o_string *o, صحيح ch)
{
	debug_printf("b_addchr: %c %d %p\n", ch, o->درازی, o);
	إذا (b_check_space(o, 1)) أرجع B_NOSPAC;
	o->داده[o->درازی] = ch;
	o->درازی++;
	o->داده[o->درازی] = '\0';
	أرجع 0;
}

ساكن فراغ b_reset(o_string *o)
{
	o->درازی = 0;
	o->nonnull = 0;
	إذا (o->داده != عدم) *o->داده = '\0';
}

ساكن فراغ b_free(o_string *o)
{
	b_reset(o);
	حرر(o->داده);
	o->داده = عدم;
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

#اگر_تعریف_نشده __U_BOOT__
ساكن صحيح b_adduint(o_string *o, بیعلامت صحيح i)
{
	صحيح r;
	حرف *p = simple_itoa(i);
	/* no escape checking necessary */
	افعل r=b_addchr(o, *p++); طالما (r==0 && *p);
	أرجع r;
}
#پایان_اگر

ساكن صحيح static_get(بنية درون_str *i)
{
	صحيح ch = *i->p++;
	إذا (ch=='\0') أرجع EOF;
	أرجع ch;
}

ساكن صحيح static_peek(بنية درون_str *i)
{
	أرجع *i->p;
}

#اگر_تعریف_نشده __U_BOOT__
ساكن مضمن فراغ cmdedit_مجموعه_initial_prompt(فراغ)
{
#اگر_تعریف_نشده CONFIG_FEATURE_SH_FANCY_PROMPT
	PS1 = عدم;
#وإلا
	PS1 = env_get("PS1");
	إذا(PS1==0)
		PS1 = "\\w \\$ ";
#پایان_اگر
}

ساكن مضمن فراغ راهاندازی_prompt_string(صحيح promptmode, حرف **prompt_str)
{
	debug_printf("setup_prompt_string %d ",promptmode);
#اگر_تعریف_نشده CONFIG_FEATURE_SH_FANCY_PROMPT
	/* Set up the prompt */
	إذا (promptmode == 1) {
		حرر(PS1);
		PS1=xmalloc(طول_رشته(cwd)+4);
		sprintf(PS1, "%s %s", cwd, ( geteuid() != 0 ) ?  "$ ":"# ");
		*prompt_str = PS1;
	} وإلا {
		*prompt_str = PS2;
	}
#وإلا
	*prompt_str = (promptmode==1)? PS1 : PS2;
#پایان_اگر
	debug_printf("result %s\n",*prompt_str);
}
#پایان_اگر

#اگر_تعریف_شده __U_BOOT__
ساكن صحيح uboot_cli_readline(بنية درون_str *i)
{
	حرف *prompt;
	حرف __maybe_unused *ps_prompt = عدم;

	إذا (i->promptmode == 1)
		prompt = CONFIG_SYS_PROMPT;
	وإلا
		prompt = CONFIG_SYS_PROMPT_HUSH_PS2;

#اگر_تعریف_شده CONFIG_CMDLINE_PS_SUPPORT
	إذا (i->promptmode == 1)
		ps_prompt = env_get("PS1");
	وإلا
		ps_prompt = env_get("PS2");
	إذا (ps_prompt)
		prompt = ps_prompt;
#پایان_اگر

	أرجع cli_readline(prompt);
}
#پایان_اگر

ساكن فراغ get_user_درون_داد(بنية درون_str *i)
{
#اگر_تعریف_نشده __U_BOOT__
	حرف *prompt_str;
	ساكن حرف the_command[BUFSIZ];

	راهاندازی_prompt_string(i->promptmode, &prompt_str);
#اگر_تعریف_شده CONFIG_FEATURE_COMMAND_EDITING
	/*
	 ** enable command line editing only while a command line
	 ** is actually being read; otherwise, we'll end up bequeathing
	 ** atexit() handlers and other unwanted stuff to our
	 ** child processes (rob@sysgo.de)
	 */
	cmdedit_read_درون_داد(prompt_str, the_command);
#وإلا
	fputs(prompt_str, stdout);
	fflush(stdout);
	the_command[0]=fgetc(i->file);
	the_command[1]='\0';
#پایان_اگر
	fflush(stdout);
	i->p = the_command;
#وإلا
	صحيح n;
	ساكن حرف the_command[CONFIG_SYS_CBSIZE + 1];

	bootretry_reset_cmd_timeout();
	i->__promptme = 1;
	n = uboot_cli_readline(i);

#اگر_تعریف_شده CONFIG_BOOT_RETRY_زمان
	إذا (n == -2) {
	  puts("\nTimeout waiting for command\n");
#  ifdef CONFIG_RESET_مقصد_RETRY
	  do_reset(عدم, 0, 0, عدم);
#  elif IS_ENABLED(CONFIG_RETRY_BOOTCMD)
	strcpy(console_میانگیر, "run bootcmd\n");
# وإلا
#	error "This only works with CONFIG_RESET_TO_RETRY or CONFIG_BOOT_RETRY_COMMAND enabled"
#  endif
	}
#پایان_اگر
	إذا (n == -1 ) {
		flag_repeat = 0;
		i->__promptme = 0;
	}
	n = طول_رشته(console_میانگیر);
	console_میانگیر[n] = '\n';
	console_میانگیر[n+1]= '\0';
	إذا (had_ctrlc()) flag_repeat = 0;
	clear_ctrlc();
	do_repeat = 0;
	إذا (i->promptmode == 1) {
		إذا (console_میانگیر[0] == '\n'&& flag_repeat == 0) {
			strcpy(the_command,console_میانگیر);
		}
		وإلا {
			إذا (console_میانگیر[0] != '\n') {
				strcpy(the_command,console_میانگیر);
				flag_repeat = 1;
			}
			وإلا {
				do_repeat = 1;
			}
		}
		i->p = the_command;
	}
	وإلا {
		إذا (console_میانگیر[0] != '\n') {
			إذا (طول_رشته(the_command) + طول_رشته(console_میانگیر)
			    < CONFIG_SYS_CBSIZE) {
				n = طول_رشته(the_command);
				the_command[n-1] = ' ';
				strcpy(&the_command[n],console_میانگیر);
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
		i->p = console_میانگیر;
	}
#پایان_اگر
}

/* This is the magic location that prints prompts
 * and gets data back from the user */
ساكن صحيح file_get(بنية درون_str *i)
{
	صحيح ch;

	ch = 0;
	/* If there is data waiting, eat it up */
	إذا (i->p && *i->p) {
		ch = *i->p++;
	} وإلا {
		/* need to double check i->file because we might be doing something
		 * more complicated by now, like sourcing or substituting. */
#اگر_تعریف_نشده __U_BOOT__
		إذا (i->__promptme && interactive && i->file == stdin) {
			طالما(! i->p || (interactive && طول_رشته(i->p)==0) ) {
#وإلا
			طالما(! i->p  || طول_رشته(i->p)==0 ) {
#پایان_اگر
				get_user_درون_داد(i);
			}
			i->promptmode=2;
#اگر_تعریف_نشده __U_BOOT__
			i->__promptme = 0;
#پایان_اگر
			إذا (i->p && *i->p) {
				ch = *i->p++;
			}
#اگر_تعریف_نشده __U_BOOT__
		} وإلا {
			ch = fgetc(i->file);
		}

#پایان_اگر
		debug_printf("b_getch: got a %d\n", ch);
	}
#اگر_تعریف_نشده __U_BOOT__
	إذا (ch == '\n') i->__promptme=1;
#پایان_اگر
	أرجع ch;
}

/* All the callers guarantee this routine will never be
 * used right after a newline, so prompting is not needed.
 */
ساكن صحيح file_peek(بنية درون_str *i)
{
#اگر_تعریف_نشده __U_BOOT__
	إذا (i->p && *i->p) {
#پایان_اگر
		أرجع *i->p;
#اگر_تعریف_نشده __U_BOOT__
	} وإلا {
		i->peek_buf[0] = fgetc(i->file);
		i->peek_buf[1] = '\0';
		i->p = i->peek_buf;
		debug_printf("b_peek: got a %d\n", *i->p);
		أرجع *i->p;
	}
#پایان_اگر
}

#اگر_تعریف_نشده __U_BOOT__
ساكن فراغ راهاندازی_file_درون_str(بنية درون_str *i, FILE *f)
#وإلا
ساكن فراغ راهاندازی_file_درون_str(بنية درون_str *i)
#پایان_اگر
{
	i->peek = file_peek;
	i->get = file_get;
	i->__promptme=1;
	i->promptmode=1;
#اگر_تعریف_نشده __U_BOOT__
	i->file = f;
#پایان_اگر
	i->p = عدم;
}

ساكن فراغ راهاندازی_string_درون_str(بنية درون_str *i, ثابت حرف *s)
{
	i->peek = static_peek;
	i->get = static_get;
	i->__promptme=1;
	i->promptmode=1;
	i->p = s;
}

#اگر_تعریف_نشده __U_BOOT__
ساكن فراغ mark_open(صحيح fd)
{
	بنية close_me *new = xmalloc(حجم(بنية close_me));
	new->fd = fd;
	new->بعدی = close_me_سر;
	close_me_سر = new;
}

ساكن فراغ mark_closed(صحيح fd)
{
	بنية close_me *tmp;
	إذا (close_me_سر == عدم || close_me_سر->fd != fd)
		error_پیام_and_die("corrupt close_me");
	tmp = close_me_سر;
	close_me_سر = close_me_سر->بعدی;
	حرر(tmp);
}

ساكن فراغ close_all(فراغ)
{
	بنية close_me *c;
	لكل (c=close_me_سر; c; c=c->بعدی) {
		close(c->fd);
	}
	close_me_سر = عدم;
}

/* squirrel != NULL means we squirrel away copies of stdin, stdout,
 * and stderr if they are redirected. */
ساكن صحيح راهاندازی_redirects(بنية فرزند_prog *prog, صحيح squirrel[])
{
	صحيح openfd, نما;
	بنية redir_struct *redir;

	لكل (redir=prog->redirects; redir; redir=redir->بعدی) {
		إذا (redir->dup == -1 && redir->کلمه.gl_pathv == عدم) {
			/* something went wrong in the parse.  Pretend it didn't happen */
			استمر;
		}
		إذا (redir->dup == -1) {
			نما=redir_جدول[redir->type].نما;
			openfd = open(redir->کلمه.gl_pathv[0], نما, 0666);
			إذا (openfd < 0) {
			/* this could get lost if stderr has been redirected, but
			   bash and ash both lose it as well (though zsh doesn't!) */
				perror_پیام("error opening %s", redir->کلمه.gl_pathv[0]);
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
ساكن فراغ pseudo_exec(بنية فرزند_prog *فرزند)
{
	صحيح i, rcode;
	حرف *p;
	بنية built_درون_command *x;
	إذا (فرزند->argv) {
		لكل (i=0; is_assignment(فرزند->argv[i]); i++) {
			debug_printf("pid %d environment modification: %s\n",getpid(),فرزند->argv[i]);
			p = درج_var_مقدار(فرزند->argv[i]);
			putenv(strdup(p));
			إذا (p != فرزند->argv[i]) حرر(p);
		}
		فرزند->argv+=i;  /* XXX this hack isn't so horrible, since we are about
					to exit, and therefore don't need to keep data
					structures consistent for free() use. */
		/* If a variable is assigned in a forest, and nobody listens,
		 * was it ever really set?
		 */
		إذا (فرزند->argv[0] == عدم) {
			_exit(EXIT_SUCCESS);
		}

		/*
		 * Check if the command matches any of the builtins.
		 * Depending on context, this might be redundant.  But it's
		 * easier to waste a few CPU cycles than it is to figure out
		 * if this is one of those cases.
		 */
		لكل (x = bltins; x->cmd; x++) {
			إذا (strcmp(فرزند->argv[0], x->cmd) == 0 ) {
				debug_printf("builtin exec %s\n", فرزند->argv[0]);
				rcode = x->function(فرزند);
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
#اگر_تعریف_شده CONFIG_FEATURE_SH_STANDALONE_SHELL
		{
			صحيح argc_l;
			حرف** argv_l=فرزند->argv;
			حرف *name = فرزند->argv[0];

#اگر_تعریف_شده CONFIG_FEATURE_SH_APPLETS_ALWAYS_WIN
			/* Following discussions from November 2000 on the busybox mailing
			 * list, the default configuration, (without
			 * get_last_path_component()) lets the user force use of an
			 * external command by specifying the full (with slashes) filename.
			 * If you enable CONFIG_FEATURE_SH_APPLETS_ALWAYS_WIN then applets
			 * _aways_ override external commands, so if you want to run
			 * /bin/cat, it will use BusyBox cat even if /bin/cat exists on the
			 * filesystem and is _not_ busybox.  Some systems may want this,
			 * most do not.  */
			name = get_آخرین_مسیر_component(name);
#پایان_اگر
			/* Count argc for use in a second... */
			لكل(argc_l=0;*argv_l!=عدم; argv_l++, argc_l++);
			optind = 1;
			debug_printf("running applet %s\n", name);
			پیاپی_applet_by_name(name, argc_l, فرزند->argv);
		}
#پایان_اگر
		debug_printf("exec of %s\n",فرزند->argv[0]);
		execvp(فرزند->argv[0],فرزند->argv);
		perror_پیام("couldn't exec: %s",فرزند->argv[0]);
		_exit(1);
	} وإلا إذا (فرزند->گروه) {
		debug_printf("runtime nesting to group\n");
		interactive=0;    /* crucial!!!! */
		rcode = پیاپی_list_real(فرزند->گروه);
		/* OK to leak memory by not calling free_pipe_list,
		 * since this process is about to exit */
		_exit(rcode);
	} وإلا {
		/* Can happen.  See what bash does with ">foo" by itself. */
		debug_printf("trying to pseudo_exec null command\n");
		_exit(EXIT_SUCCESS);
	}
}

ساكن فراغ درج_bg_کار(بنية pipe *pi)
{
	بنية pipe *thejob;

	/* Linear search for the ID of the job to use */
	pi->jobid = 1;
	لكل (thejob = کار_list; thejob; thejob = thejob->بعدی)
		إذا (thejob->jobid >= pi->jobid)
			pi->jobid = thejob->jobid + 1;

	/* add thejob to the list of running jobs */
	إذا (!کار_list) {
		thejob = کار_list = xmalloc(حجم(*thejob));
	} وإلا {
		لكل (thejob = کار_list; thejob->بعدی; thejob = thejob->بعدی) /* nothing */;
		thejob->بعدی = xmalloc(حجم(*thejob));
		thejob = thejob->بعدی;
	}

	/* physically copy the struct job */
	کپی_حافظه(thejob, pi, حجم(بنية pipe));
	thejob->بعدی = عدم;
	thejob->running_progs = thejob->رقم_progs;
	thejob->stopped_progs = 0;
	thejob->متن = xmalloc(BUFSIZ); /* cmdedit buffer size */

	/*if (pi->progs[0] && pi->progs[0].argv && pi->progs[0].argv[0]) */
	{
		حرف *bar=thejob->متن;
		حرف **foo=pi->progs[0].argv;
		طالما(foo && *foo) {
			bar += sprintf(bar, "%s ", *foo++);
		}
	}

	/* we don't wait for background thejobs to return -- append it
	   to the list of backgrounded thejobs and leave it alone */
	چاپ_کن("[%d] %d\n", thejob->jobid, thejob->progs[0].pid);
	آخرین_bg_pid = thejob->progs[0].pid;
	آخرین_jobid = thejob->jobid;
}

/* remove a backgrounded job */
ساكن فراغ remove_bg_کار(بنية pipe *pi)
{
	بنية pipe *prev_pipe;

	إذا (pi == کار_list) {
		کار_list = pi->بعدی;
	} وإلا {
		prev_pipe = کار_list;
		طالما (prev_pipe->بعدی != pi)
			prev_pipe = prev_pipe->بعدی;
		prev_pipe->بعدی = pi->بعدی;
	}
	إذا (کار_list)
		آخرین_jobid = کار_list->jobid;
	وإلا
		آخرین_jobid = 0;

	pi->stopped_progs = 0;
	free_pipe(pi, 0);
	حرر(pi);
}

/* Checks to see if any processes have exited -- if they
   have, figure out why and see if a job has completed */
ساكن صحيح checkjobs(بنية pipe* fg_pipe)
{
	صحيح صفات;
	صحيح وضعیت;
	صحيح prognum = 0;
	بنية pipe *pi;
	pid_t childpid;

	صفات = WUNTRACED;
	إذا (fg_pipe==عدم) {
		صفات |= WNOHANG;
	}

	طالما ((childpid = waitpid(-1, &وضعیت, صفات)) > 0) {
		إذا (fg_pipe) {
			صحيح i, rcode = 0;
			لكل (i=0; i < fg_pipe->رقم_progs; i++) {
				إذا (fg_pipe->progs[i].pid == childpid) {
					إذا (i==fg_pipe->رقم_progs-1)
						rcode=WEXITSTATUS(وضعیت);
					(fg_pipe->رقم_progs)--;
					أرجع(rcode);
				}
			}
		}

		لكل (pi = کار_list; pi; pi = pi->بعدی) {
			prognum = 0;
			طالما (prognum < pi->رقم_progs && pi->progs[prognum].pid != childpid) {
				prognum++;
			}
			إذا (prognum < pi->رقم_progs)
				اكسر;
		}

		إذا(pi==عدم) {
			debug_printf("checkjobs: pid %d was not in our list!\n", childpid);
			استمر;
		}

		إذا (WIFEXITED(وضعیت) || WIFSIGNALED(وضعیت)) {
			/* child exited */
			pi->running_progs--;
			pi->progs[prognum].pid = 0;

			إذا (!pi->running_progs) {
				چاپ_کن(کار_وضعیت_FORMAT, pi->jobid, "Done", pi->متن);
				remove_bg_کار(pi);
			}
		} وإلا {
			/* child stopped */
			pi->stopped_progs++;
			pi->progs[prognum].is_stopped = 1;

#إذا 0
			/* Printing this stuff is a pain, since it tends to
			 * overwrite the prompt an inconveinient moments.  So
			 * don't do that.  */
			إذا (pi->stopped_progs == pi->رقم_progs) {
				چاپ_کن("\n"کار_وضعیت_FORMAT, pi->jobid, "Stopped", pi->متن);
			}
#پایان_اگر
		}
	}

	إذا (childpid == -1 && errno != ECHILD)
		perror_پیام("waitpid");

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
#پایان_اگر

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
ساكن صحيح پیاپی_pipe_real(بنية pipe *pi)
{
	صحيح i;
#اگر_تعریف_نشده __U_BOOT__
	صحيح nextin, nextout;
	صحيح pipefds[2];				/* pipefds[0] is for reading */
	بنية فرزند_prog *فرزند;
	بنية built_درون_command *x;
	حرف *p;
# إذا __GNUC__
	/* Avoid longjmp clobbering */
	(فراغ) &i;
	(فراغ) &nextin;
	(فراغ) &nextout;
	(فراغ) &فرزند;
# endif
#وإلا
	صحيح nextin;
	صحيح flag = do_repeat ? CMD_FLAG_REPEAT : 0;
	بنية فرزند_prog *فرزند;
	حرف *p;
# إذا __GNUC__
	/* Avoid longjmp clobbering */
	(فراغ) &i;
	(فراغ) &nextin;
	(فراغ) &فرزند;
# endif
#پایان_اگر	/* __U_BOOT__ */

	nextin = 0;
#اگر_تعریف_نشده __U_BOOT__
	pi->pgrp = -1;
#پایان_اگر

	/* Check if this is a simple builtin (not part of a pipe).
	 * Builtins within pipes have to fork anyway, and are handled in
	 * pseudo_exec.  "echo foo | read bar" doesn't work on bash, either.
	 */
	إذا (pi->رقم_progs == 1) فرزند = & (pi->progs[0]);
#اگر_تعریف_نشده __U_BOOT__
	إذا (pi->رقم_progs == 1 && فرزند->گروه && فرزند->subshell == 0) {
		صحيح squirrel[] = {-1, -1, -1};
		صحيح rcode;
		debug_printf("non-subshell grouping\n");
		راهاندازی_redirects(فرزند, squirrel);
		/* XXX could we merge code with following builtin case,
		 * by creating a pseudo builtin that calls run_list_real? */
		rcode = پیاپی_list_real(فرزند->گروه);
		restore_redirects(squirrel);
#وإلا
		إذا (pi->رقم_progs == 1 && فرزند->گروه) {
		صحيح rcode;
		debug_printf("non-subshell grouping\n");
		rcode = پیاپی_list_real(فرزند->گروه);
#پایان_اگر
		أرجع rcode;
	} وإلا إذا (pi->رقم_progs == 1 && pi->progs[0].argv != عدم) {
		لكل (i=0; is_assignment(فرزند->argv[i]); i++) { /* nothing */ }
		إذا (i!=0 && فرزند->argv[i]==عدم) {
			/* assignments, but no command: set the local environment */
			لكل (i=0; فرزند->argv[i]!=عدم; i++) {

				/* Ok, this case is tricky.  We have to decide if this is a
				 * local variable, or an already exported variable.  If it is
				 * already exported, we have to export the new value.  If it is
				 * not exported, we need only set this as a local variable.
				 * This junk is all to decide whether or not to export this
				 * variable. */
				صحيح export_me=0;
				حرف *name, *مقدار;
				name = xstrdup(فرزند->argv[i]);
				debug_printf("Local environment set: %s\n", name);
				مقدار = strchr(name, '=');
				إذا (مقدار)
					*مقدار=0;
#اگر_تعریف_نشده __U_BOOT__
				إذا ( get_local_var(name)) {
					export_me=1;
				}
#پایان_اگر
				حرر(name);
				p = درج_var_مقدار(فرزند->argv[i]);
				مجموعه_local_var(p, export_me);
				إذا (p != فرزند->argv[i]) حرر(p);
			}
			أرجع EXIT_SUCCESS;   /* don't worry about errors in set_local_var() yet */
		}
		لكل (i = 0; is_assignment(فرزند->argv[i]); i++) {
			p = درج_var_مقدار(فرزند->argv[i]);
#اگر_تعریف_نشده __U_BOOT__
			putenv(strdup(p));
#وإلا
			مجموعه_local_var(p, 0);
#پایان_اگر
			إذا (p != فرزند->argv[i]) {
				فرزند->sp--;
				حرر(p);
			}
		}
		إذا (فرزند->sp) {
			حرف * str = عدم;

			str = make_string(فرزند->argv + i,
					  فرزند->argv_nonnull + i);
			parse_string_outer(str, FLAG_EXIT_FROM_LOOP | FLAG_REPARSING);
			حرر(str);
			أرجع آخرین_return_کد;
		}
#اگر_تعریف_نشده __U_BOOT__
		لكل (x = bltins; x->cmd; x++) {
			إذا (strcmp(فرزند->argv[i], x->cmd) == 0 ) {
				صحيح squirrel[] = {-1, -1, -1};
				صحيح rcode;
				إذا (x->function == builtin_exec && فرزند->argv[i+1]==عدم) {
					debug_printf("magic exec\n");
					راهاندازی_redirects(فرزند,عدم);
					أرجع EXIT_SUCCESS;
				}
				debug_printf("builtin inline %s\n", فرزند->argv[0]);
				/* XXX setup_redirects acts on file descriptors, not FILEs.
				 * This is perfect for work that comes after exec().
				 * Is it really safe for inline use?  Experimentally,
				 * things seem to work with glibc. */
				راهاندازی_redirects(فرزند, squirrel);

				فرزند->argv += i;  /* XXX horrible hack */
				rcode = x->function(فرزند);
				/* XXX restore hack so free() can work right */
				فرزند->argv -= i;
				restore_redirects(squirrel);
			}
			أرجع rcode;
		}
#وإلا
		/* check ";", because ,example , argv consist from
		 * "help;flinfo" must not execute
		 */
		إذا (strchr(فرزند->argv[i], ';')) {
			چاپ_کن("Unknown command '%s' - try 'help' or use "
					"'run' command\n", فرزند->argv[i]);
			أرجع -1;
		}
		/* Process the command */
		أرجع cmd_process(flag, فرزند->argc - i, فرزند->argv + i,
				   &flag_repeat, عدم);
#پایان_اگر
	}
#اگر_تعریف_نشده __U_BOOT__

	لكل (i = 0; i < pi->رقم_progs; i++) {
		فرزند = & (pi->progs[i]);

		/* pipes are inserted between pairs of commands */
		إذا ((i + 1) < pi->رقم_progs) {
			إذا (pipe(pipefds)<0) perror_پیام_and_die("pipe");
			nextout = pipefds[1];
		} وإلا {
			nextout=1;
			pipefds[0] = -1;
		}

		/* XXX test for failed fork()? */
		إذا (!(فرزند->pid = fork())) {
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
			راهاندازی_redirects(فرزند,عدم);

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

			pseudo_exec(فرزند);
		}

		/* put our child in the process group whose leader is the
		   first process in this pipe */
		إذا (pi->pgrp < 0) {
			pi->pgrp = فرزند->pid;
		}
		/* Don't check for errors.  The child may be dead already,
		 * in which case setpgid returns error code EACCES. */
		setpgid(فرزند->pid, pi->pgrp);

		إذا (nextin != 0)
			close(nextin);
		إذا (nextout != 1)
			close(nextout);

		/* If there isn't another process, nextin is garbage
		   but it doesn't matter */
		nextin = pipefds[0];
	}
#پایان_اگر
	أرجع -1;
}

ساكن صحيح پیاپی_list_real(بنية pipe *pi)
{
	حرف *save_name = عدم;
	حرف **list = عدم;
	حرف **save_list = عدم;
	بنية pipe *rpipe;
	صحيح flag_rep = 0;
#اگر_تعریف_نشده __U_BOOT__
	صحيح save_رقم_progs;
#پایان_اگر
	صحيح rcode=0, flag_skip=1;
	صحيح flag_restore = 0;
	صحيح if_کد=0, بعدی_if_کد=0;  /* need double-buffer to handle elif */
	reserved_style rmode, skip_بیشتر_درون_this_rmode=RES_XXXX;
	/* check syntax for "for" */
	لكل (rpipe = pi; rpipe; rpipe = rpipe->بعدی) {
		إذا ((rpipe->r_نما == RES_درون ||
		    rpipe->r_نما == RES_FOR) &&
		    (rpipe->بعدی == عدم)) {
				syntax();
#اگر_تعریف_شده __U_BOOT__
				flag_repeat = 0;
#پایان_اگر
				أرجع 1;
		}
		إذا ((rpipe->r_نما == RES_درون &&
			(rpipe->بعدی->r_نما == RES_درون &&
			rpipe->بعدی->progs->argv != عدم))||
			(rpipe->r_نما == RES_FOR &&
			rpipe->بعدی->r_نما != RES_درون)) {
				syntax();
#اگر_تعریف_شده __U_BOOT__
				flag_repeat = 0;
#پایان_اگر
				أرجع 1;
		}
	}
	لكل (; pi; pi = (flag_restore != 0) ? rpipe : pi->بعدی) {
		إذا (pi->r_نما == RES_WHILE || pi->r_نما == RES_UNTIL ||
			pi->r_نما == RES_FOR) {
#اگر_تعریف_شده __U_BOOT__
				/* check Ctrl-C */
				ctrlc();
				إذا ((had_ctrlc())) {
					أرجع 1;
				}
#پایان_اگر
				flag_restore = 0;
				إذا (!rpipe) {
					flag_rep = 0;
					rpipe = pi;
				}
		}
		rmode = pi->r_نما;
		debug_printf("rmode=%d  if_code=%d  next_if_code=%d skip_more=%d\n", rmode, if_کد, بعدی_if_کد, skip_بیشتر_درون_this_rmode);
		إذا (rmode == skip_بیشتر_درون_this_rmode && flag_skip) {
			إذا (pi->followup == PIPE_SEQ) flag_skip=0;
			استمر;
		}
		flag_skip = 1;
		skip_بیشتر_درون_this_rmode = RES_XXXX;
		إذا (rmode == RES_THEN || rmode == RES_ELSE) if_کد = بعدی_if_کد;
		إذا (rmode == RES_THEN &&  if_کد) استمر;
		إذا (rmode == RES_ELSE && !if_کد) استمر;
		إذا (rmode == RES_ELIF && !if_کد) اكسر;
		إذا (rmode == RES_FOR && pi->رقم_progs) {
			إذا (!list) {
				/* if no variable values after "in" we skip "for" */
				إذا (!pi->بعدی->progs->argv) استمر;
				/* create list of variable values */
				list = make_list_درون(pi->بعدی->progs->argv,
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
#اگر_تعریف_نشده __U_BOOT__
				pi->progs->glob_نتیجه.gl_pathv[0] =
					pi->progs->argv[0];
#پایان_اگر
				استمر;
			} وإلا {
				/* insert new value from list for variable */
				حرر(pi->progs->argv[0]);
				pi->progs->argv[0] = *list++;
#اگر_تعریف_نشده __U_BOOT__
				pi->progs->glob_نتیجه.gl_pathv[0] =
					pi->progs->argv[0];
#پایان_اگر
			}
		}
		إذا (rmode == RES_درون) استمر;
		إذا (rmode == RES_DO) {
			إذا (!flag_rep) استمر;
		}
		إذا (rmode == RES_انجامشده) {
			إذا (flag_rep) {
				flag_restore = 1;
			} وإلا {
				rpipe = عدم;
			}
		}
		إذا (pi->رقم_progs == 0) استمر;
#اگر_تعریف_نشده __U_BOOT__
		save_رقم_progs = pi->رقم_progs; /* save number of programs */
#پایان_اگر
		rcode = پیاپی_pipe_real(pi);
		debug_printf("run_pipe_real returned %d\n",rcode);
#اگر_تعریف_نشده __U_BOOT__
		إذا (rcode!=-1) {
			/* We only ran a builtin: rcode was set by the return value
			 * of run_pipe_real(), and we don't need to wait for anything. */
		} وإلا إذا (pi->followup==PIPE_BG) {
			/* XXX check bash's behavior with nontrivial pipes */
			/* XXX compute jobid */
			/* XXX what does bash do with attempts to background builtins? */
			درج_bg_کار(pi);
			rcode = EXIT_SUCCESS;
		} وإلا {
			إذا (interactive) {
				/* move the new process group into the foreground */
				إذا (tcsetpgrp(shell_terminal, pi->pgrp) && errno != ENOTTY)
					perror_پیام("tcsetpgrp-3");
				rcode = checkjobs(pi);
				/* move the shell to the foreground */
				إذا (tcsetpgrp(shell_terminal, getpgid(0)) && errno != ENOTTY)
					perror_پیام("tcsetpgrp-4");
			} وإلا {
				rcode = checkjobs(pi);
			}
			debug_printf("checkjobs returned %d\n",rcode);
		}
		آخرین_return_کد=rcode;
#وإلا
		إذا (rcode < -1) {
			آخرین_return_کد = -rcode - 2;
			أرجع -2;	/* exit */
		}
		آخرین_return_کد = rcode;
#پایان_اگر
#اگر_تعریف_نشده __U_BOOT__
		pi->رقم_progs = save_رقم_progs; /* restore number of programs */
#پایان_اگر
		إذا ( rmode == RES_IF || rmode == RES_ELIF )
			بعدی_if_کد=rcode;  /* can be overwritten a number of times */
		إذا (rmode == RES_WHILE)
			flag_rep = !آخرین_return_کد;
		إذا (rmode == RES_UNTIL)
			flag_rep = آخرین_return_کد;
		إذا ( (rcode==EXIT_SUCCESS && pi->followup==PIPE_OR) ||
		     (rcode!=EXIT_SUCCESS && pi->followup==PIPE_AND) )
			skip_بیشتر_درون_this_rmode=rmode;
#اگر_تعریف_نشده __U_BOOT__
		checkjobs(عدم);
#پایان_اگر
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
	بنية فرزند_prog *فرزند;
#اگر_تعریف_نشده __U_BOOT__
	بنية redir_struct *r, *rnext;
#پایان_اگر
	صحيح a, i, ret_کد=0;
	حرف *ind = indenter(indent);

#اگر_تعریف_نشده __U_BOOT__
	إذا (pi->stopped_progs > 0)
		أرجع ret_کد;
	final_printf("%s run pipe: (pid %d)\n",ind,getpid());
#پایان_اگر
	لكل (i=0; i<pi->رقم_progs; i++) {
		فرزند = &pi->progs[i];
		final_printf("%s  command %d:\n",ind,i);
		إذا (فرزند->argv) {
			لكل (a=0,p=فرزند->argv; *p; a++,p++) {
				final_printf("%s   argv[%d] = %s\n",ind,a,*p);
			}
#اگر_تعریف_نشده __U_BOOT__
			globfree(&فرزند->glob_نتیجه);
#وإلا
			لكل (a = 0; a < فرزند->argc; a++) {
				حرر(فرزند->argv[a]);
			}
			حرر(فرزند->argv);
			حرر(فرزند->argv_nonnull);
			فرزند->argc = 0;
#پایان_اگر
			فرزند->argv=عدم;
		} وإلا إذا (فرزند->گروه) {
#اگر_تعریف_نشده __U_BOOT__
			final_printf("%s   begin group (subshell:%d)\n",ind, فرزند->subshell);
#پایان_اگر
			ret_کد = free_pipe_list(فرزند->گروه,indent+3);
			final_printf("%s   end group\n",ind);
		} وإلا {
			final_printf("%s   (nil)\n",ind);
		}
#اگر_تعریف_نشده __U_BOOT__
		لكل (r=فرزند->redirects; r; r=rnext) {
			final_printf("%s   redirect %d%s", ind, r->fd, redir_جدول[r->type].descrip);
			إذا (r->dup == -1) {
				/* guard against the case >$FOO, where foo is unset or blank */
				إذا (r->کلمه.gl_pathv) {
					final_printf(" %s\n", *r->کلمه.gl_pathv);
					globfree(&r->کلمه);
				}
			} وإلا {
				final_printf("&%d\n", r->dup);
			}
			rnext=r->بعدی;
			حرر(r);
		}
		فرزند->redirects=عدم;
#پایان_اگر
	}
	حرر(pi->progs);   /* children are an array, they get freed all at once */
	pi->progs=عدم;
	أرجع ret_کد;
}

ساكن صحيح free_pipe_list(بنية pipe *سر, صحيح indent)
{
	صحيح rcode=0;   /* if list has no members */
	بنية pipe *pi, *بعدی;
	حرف *ind = indenter(indent);
	لكل (pi=سر; pi; pi=بعدی) {
		final_printf("%s pipe reserved mode %d\n", ind, pi->r_نما);
		rcode = free_pipe(pi, indent);
		final_printf("%s pipe followup code %d\n", ind, pi->followup);
		بعدی=pi->بعدی;
		pi->بعدی=عدم;
		حرر(pi);
	}
	أرجع rcode;
}

/* Select which version we will use */
ساكن صحيح پیاپی_list(بنية pipe *pi)
{
	صحيح rcode=0;
#اگر_تعریف_نشده __U_BOOT__
	إذا (fake_نما==0) {
#پایان_اگر
		rcode = پیاپی_list_real(pi);
#اگر_تعریف_نشده __U_BOOT__
	}
#پایان_اگر
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
#اگر_تعریف_نشده __U_BOOT__
ساكن صحيح globhack(ثابت حرف *src, صحيح flags, glob_t *pglob)
{
	صحيح cnt=0, pathc;
	ثابت حرف *s;
	حرف *dest;
	لكل (cnt=1, s=src; s && *s; s++) {
		إذا (*s == '\\') s++;
		cnt++;
	}
	dest = حافظه_بده(cnt);
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
#پایان_اگر

ساكن صحيح xglob(o_string *dest, صحيح flags, glob_t *pglob)
{
	صحيح gr;

	/* short-circuit for null word */
	/* we can code this better when the debug_printf's are gone */
	إذا (dest->درازی == 0) {
		إذا (dest->nonnull) {
			/* bash man page calls this an "explicit" null */
			gr = globhack(dest->داده, flags, pglob);
			debug_printf("globhack returned %d\n",gr);
		} وإلا {
			أرجع 0;
		}
	} وإلا إذا (glob_needed(dest->داده)) {
		gr = glob(dest->داده, flags, عدم, pglob);
		debug_printf("glob returned %d\n",gr);
		إذا (gr == GLOB_NOMATCH) {
			/* quote removal, or more accurately, backslash removal */
			gr = globhack(dest->داده, flags, pglob);
			debug_printf("globhack returned %d\n",gr);
		}
	} وإلا {
		gr = globhack(dest->داده, flags, pglob);
		debug_printf("globhack returned %d\n",gr);
	}
	إذا (gr == GLOB_NOSPACE)
		error_پیام_and_die("out of memory during glob");
	إذا (gr != 0) { /* GLOB_ABORTED ? */
		error_پیام("glob(3) error %d",gr);
	}
	/* globprint(glob_target); */
	أرجع gr;
}
#پایان_اگر

#اگر_تعریف_شده __U_BOOT__
ساكن حرف *get_dollar_var(حرف ch);
#پایان_اگر

/* This is used to get/check local shell variables */
حرف *get_local_var(ثابت حرف *s)
{
	بنية variables *cur;

	إذا (!s)
		أرجع عدم;

#اگر_تعریف_شده __U_BOOT__
	إذا (*s == '$')
		أرجع get_dollar_var(s[1]);
#پایان_اگر

	لكل (cur = بالاترین_vars; cur; cur=cur->بعدی)
		إذا(strcmp(cur->name, s)==0)
			أرجع cur->مقدار;
	أرجع عدم;
}

/* This is used to set local shell variables
   flg_export==0 if only local (not exporting) variable
   flg_export==1 if "new" exporting environ
   flg_export>1  if current startup environ (not call putenv()) */
صحيح مجموعه_local_var(ثابت حرف *s, صحيح flg_export)
{
	حرف *name, *مقدار;
	صحيح نتیجه=0;
	بنية variables *cur;

#اگر_تعریف_شده __U_BOOT__
	/* might be possible! */
	إذا (!آیا_حرف(*s))
		أرجع -1;
#پایان_اگر

	name=strdup(s);

	/* Assume when we enter this function that we are already in
	 * NAME=VALUE format.  So the first order of business is to
	 * split 's' on the '=' into 'name' and 'value' */
	مقدار = strchr(name, '=');
	إذا (!مقدار) {
		حرر(name);
		أرجع -1;
	}
	*مقدار++ = 0;

	إذا (!*مقدار) {
		unset_local_var(name);
		حرر(name);
		أرجع 0;
	}

	لكل(cur = بالاترین_vars; cur; cur = cur->بعدی) {
		إذا(strcmp(cur->name, name)==0)
			اكسر;
	}

	إذا(cur) {
		إذا(strcmp(cur->مقدار, مقدار)==0) {
			إذا(flg_export>0 && cur->flg_export==0)
				cur->flg_export=flg_export;
			وإلا
				نتیجه++;
		} وإلا {
			إذا(cur->flg_read_only) {
				error_پیام("%s: readonly variable", name);
				نتیجه = -1;
			} وإلا {
				إذا(flg_export>0 || cur->flg_export>1)
					cur->flg_export=1;
				حرر(cur->مقدار);

				cur->مقدار = strdup(مقدار);
			}
		}
	} وإلا {
		cur = حافظه_بده(حجم(بنية variables));
		إذا(!cur) {
			نتیجه = -1;
		} وإلا {
			cur->name = strdup(name);
			إذا (cur->name == عدم) {
				حرر(cur);
				نتیجه = -1;
			} وإلا {
				بنية variables *bottom = بالاترین_vars;
				cur->مقدار = strdup(مقدار);
				cur->بعدی = عدم;
				cur->flg_export = flg_export;
				cur->flg_read_only = 0;
				طالما(bottom->بعدی) bottom=bottom->بعدی;
				bottom->بعدی = cur;
			}
		}
	}

#اگر_تعریف_نشده __U_BOOT__
	إذا(نتیجه==0 && cur->flg_export==1) {
		*(مقدار-1) = '=';
		نتیجه = putenv(name);
	} وإلا {
#پایان_اگر
		حرر(name);
#اگر_تعریف_نشده __U_BOOT__
		إذا(نتیجه>0)            /* equivalent to previous set */
			نتیجه = 0;
	}
#پایان_اگر
	أرجع نتیجه;
}

فراغ unset_local_var(ثابت حرف *name)
{
	بنية variables *cur;

	إذا (name) {
		لكل (cur = بالاترین_vars; cur; cur=cur->بعدی) {
			إذا(strcmp(cur->name, name)==0)
				اكسر;
		}
		إذا (cur != عدم) {
			بنية variables *بعدی = بالاترین_vars;
			إذا(cur->flg_read_only) {
				error_پیام("%s: readonly variable", name);
				أرجع;
			} وإلا {
#اگر_تعریف_نشده __U_BOOT__
				إذا(cur->flg_export)
					unenv_مجموعه(cur->name);
#پایان_اگر
				حرر(cur->name);
				حرر(cur->مقدار);
				طالما (بعدی->بعدی != cur)
					بعدی = بعدی->بعدی;
				بعدی->بعدی = cur->بعدی;
			}
			حرر(cur);
		}
	}
}

ساكن صحيح is_assignment(ثابت حرف *s)
{
	إذا (s == عدم)
		أرجع 0;

	إذا (!آیا_حرف(*s)) أرجع 0;
	++s;
	طالما(آیا_حرف_یا_رقم(*s) || *s=='_') ++s;
	أرجع *s=='=';
}

#اگر_تعریف_نشده __U_BOOT__
/* the src parameter allows us to peek forward to a possible &n syntax
 * for file descriptor duplication, e.g., "2>&1".
 * Return code is 0 normally, 1 if a syntax error is detected in src.
 * Resource errors (in xmalloc) cause the process to exit */
ساكن صحيح راهاندازی_redirect(بنية p_context *ctx, صحيح fd, redir_type style,
	بنية درون_str *درون_داد)
{
	بنية فرزند_prog *فرزند=ctx->فرزند;
	بنية redir_struct *redir = فرزند->redirects;
	بنية redir_struct *آخرین_redir=عدم;

	/* Create a new redir_struct and drop it onto the end of the linked list */
	طالما(redir) {
		آخرین_redir=redir;
		redir=redir->بعدی;
	}
	redir = xmalloc(حجم(بنية redir_struct));
	redir->بعدی=عدم;
	redir->کلمه.gl_pathv=عدم;
	إذا (آخرین_redir) {
		آخرین_redir->بعدی=redir;
	} وإلا {
		فرزند->redirects=redir;
	}

	redir->type=style;
	redir->fd= (fd==-1) ? redir_جدول[style].default_fd : fd ;

	debug_printf("Redirect type %d%s\n", redir->fd, redir_جدول[style].descrip);

	/* Check for a '2>&1' type redirect */
	redir->dup = redirect_dup_رقم(درون_داد);
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
#پایان_اگر

ساكن بنية pipe *new_pipe(فراغ)
{
	بنية pipe *pi;
	pi = xmalloc(حجم(بنية pipe));
	pi->رقم_progs = 0;
	pi->progs = عدم;
	pi->بعدی = عدم;
	pi->followup = 0;  /* invalid */
	pi->r_نما = RES_NONE;
	أرجع pi;
}

ساكن فراغ initialize_context(بنية p_context *ctx)
{
	ctx->pipe=عدم;
#اگر_تعریف_نشده __U_BOOT__
	ctx->pending_redirect=عدم;
#پایان_اگر
	ctx->فرزند=عدم;
	ctx->list_سر=new_pipe();
	ctx->pipe=ctx->list_سر;
	ctx->w=RES_NONE;
	ctx->پشته=عدم;
#اگر_تعریف_شده __U_BOOT__
	ctx->old_flag=0;
#پایان_اگر
	انجامشده_command(ctx);   /* creates the memory for working child */
}

/* normal return is 0
 * if a reserved word is found, and processed, return 1
 * should handle if, then, elif, else, fi, for, while, until, do, done.
 * case, function, and select are obnoxious, save those for later.
 */
بنية reserved_combo {
	حرف *literal;
	صحيح کد;
	دراز flag;
};
/* Mostly a list of accepted follow-up reserved words.
 * FLAG_END means we are done with the sequence, and are ready
 * to turn the compound list into a command.
 * FLAG_START means the word must start a new compound list.
 */
ساكن بنية reserved_combo reserved_list[] = {
	{ "if",    RES_IF,    FLAG_THEN | FLAG_شروع },
	{ "then",  RES_THEN,  FLAG_ELIF | FLAG_ELSE | FLAG_FI },
	{ "elif",  RES_ELIF,  FLAG_THEN },
	{ "else",  RES_ELSE,  FLAG_FI   },
	{ "fi",    RES_FI,    FLAG_انتها  },
	{ "for",   RES_FOR,   FLAG_درون   | FLAG_شروع },
	{ "while", RES_WHILE, FLAG_DO   | FLAG_شروع },
	{ "until", RES_UNTIL, FLAG_DO   | FLAG_شروع },
	{ "in",    RES_درون,    FLAG_DO   },
	{ "do",    RES_DO,    FLAG_انجامشده },
	{ "done",  RES_انجامشده,  FLAG_انتها  }
};
#تعریف_کن NRES (حجم(reserved_list)/حجم(بنية reserved_combo))

ساكن صحيح reserved_کلمه(o_string *dest, بنية p_context *ctx)
{
	بنية reserved_combo *r;
	لكل (r=reserved_list;
		r<reserved_list+NRES; r++) {
		إذا (strcmp(dest->داده, r->literal) == 0) {
			debug_printf("found reserved word %s, code %d\n",r->literal,r->کد);
			إذا (r->flag & FLAG_شروع) {
				بنية p_context *new = xmalloc(حجم(بنية p_context));
				debug_printf("push stack\n");
				إذا (ctx->w == RES_درون || ctx->w == RES_FOR) {
					syntax();
					حرر(new);
					ctx->w = RES_SNTX;
					b_reset(dest);
					أرجع 1;
				}
				*new = *ctx;   /* physical copy */
				initialize_context(ctx);
				ctx->پشته=new;
			} وإلا إذا ( ctx->w == RES_NONE || ! (ctx->old_flag & (1<<r->کد))) {
				syntax();
				ctx->w = RES_SNTX;
				b_reset(dest);
				أرجع 1;
			}
			ctx->w=r->کد;
			ctx->old_flag = r->flag;
			إذا (ctx->old_flag & FLAG_انتها) {
				بنية p_context *old;
				debug_printf("pop stack\n");
				انجامشده_pipe(ctx,PIPE_SEQ);
				old = ctx->پشته;
				old->فرزند->گروه = ctx->list_سر;
#اگر_تعریف_نشده __U_BOOT__
				old->فرزند->subshell = 0;
#پایان_اگر
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
ساكن صحيح انجامشده_کلمه(o_string *dest, بنية p_context *ctx)
{
	بنية فرزند_prog *فرزند=ctx->فرزند;
#اگر_تعریف_نشده __U_BOOT__
	glob_t *glob_هدف;
	صحيح gr, flags = 0;
#وإلا
	حرف *str, *s;
	صحيح argc, cnt;
#پایان_اگر

	debug_printf("done_word: %s %p\n", dest->داده, فرزند);
	إذا (dest->درازی == 0 && !dest->nonnull) {
		debug_printf("  true null, ignored\n");
		أرجع 0;
	}
#اگر_تعریف_نشده __U_BOOT__
	إذا (ctx->pending_redirect) {
		glob_هدف = &ctx->pending_redirect->کلمه;
	} وإلا {
#پایان_اگر
		إذا (فرزند->گروه) {
			syntax();
			أرجع 1;  /* syntax error, groups and arglists don't mix */
		}
		إذا (!فرزند->argv && (ctx->type & FLAG_PARSE_SEMICOLON)) {
			debug_printf("checking %s for reserved-ness\n",dest->داده);
			إذا (reserved_کلمه(dest,ctx)) أرجع ctx->w==RES_SNTX;
		}
#اگر_تعریف_نشده __U_BOOT__
		glob_هدف = &فرزند->glob_نتیجه;
		إذا (فرزند->argv) flags |= GLOB_APPEND;
#وإلا
		لكل (cnt = 1, s = dest->داده; s && *s; s++) {
			إذا (*s == '\\') s++;
			cnt++;
		}
		str = حافظه_بده(cnt);
		إذا (!str) أرجع 1;
		إذا ( فرزند->argv == عدم) {
			فرزند->argc=0;
		}
		argc = ++فرزند->argc;
		فرزند->argv = realloc(فرزند->argv, (argc+1)*حجم(*فرزند->argv));
		إذا (فرزند->argv == عدم) {
			حرر(str);
			أرجع 1;
		}
		فرزند->argv_nonnull = realloc(فرزند->argv_nonnull,
					(argc+1)*حجم(*فرزند->argv_nonnull));
		إذا (فرزند->argv_nonnull == عدم) {
			حرر(str);
			أرجع 1;
		}
		فرزند->argv[argc-1]=str;
		فرزند->argv_nonnull[argc-1] = dest->nonnull;
		فرزند->argv[argc]=عدم;
		فرزند->argv_nonnull[argc] = 0;
		لكل (s = dest->داده; s && *s; s++,str++) {
			إذا (*s == '\\') s++;
			*str = *s;
		}
		*str = '\0';
#پایان_اگر
#اگر_تعریف_نشده __U_BOOT__
	}
	gr = xglob(dest, flags, glob_هدف);
	إذا (gr != 0) أرجع 1;
#پایان_اگر

	b_reset(dest);
#اگر_تعریف_نشده __U_BOOT__
	إذا (ctx->pending_redirect) {
		ctx->pending_redirect=عدم;
		إذا (glob_هدف->gl_pathc != 1) {
			error_پیام("ambiguous redirect");
			أرجع 1;
		}
	} وإلا {
		فرزند->argv = glob_هدف->gl_pathv;
	}
#پایان_اگر
	إذا (ctx->w == RES_FOR) {
		انجامشده_کلمه(dest,ctx);
		انجامشده_pipe(ctx,PIPE_SEQ);
	}
	أرجع 0;
}

/* The only possible error here is out of memory, in which case
 * xmalloc exits. */
ساكن صحيح انجامشده_command(بنية p_context *ctx)
{
	/* The child is really already in the pipe structure, so
	 * advance the pipe counter and make a new, null child.
	 * Only real trickiness here is that the uncommitted
	 * child structure, to which ctx->child points, is not
	 * counted in pi->num_progs. */
	بنية pipe *pi=ctx->pipe;
	بنية فرزند_prog *prog=ctx->فرزند;

	إذا (prog && prog->گروه == عدم
		 && prog->argv == عدم
#اگر_تعریف_نشده __U_BOOT__
		 && prog->redirects == عدم) {
#وإلا
										) {
#پایان_اگر
		debug_printf("done_command: skipping null command\n");
		أرجع 0;
	} وإلا إذا (prog) {
		pi->رقم_progs++;
		debug_printf("done_command: num_progs incremented to %d\n",pi->رقم_progs);
	} وإلا {
		debug_printf("done_command: initializing\n");
	}
	pi->progs = xrealloc(pi->progs, حجم(*pi->progs) * (pi->رقم_progs+1));

	prog = pi->progs + pi->رقم_progs;
#اگر_تعریف_نشده __U_BOOT__
	prog->redirects = عدم;
#پایان_اگر
	prog->argv = عدم;
	prog->argv_nonnull = عدم;
#اگر_تعریف_نشده __U_BOOT__
	prog->is_stopped = 0;
#پایان_اگر
	prog->گروه = عدم;
#اگر_تعریف_نشده __U_BOOT__
	prog->glob_نتیجه.gl_pathv = عدم;
	prog->family = pi;
#پایان_اگر
	prog->sp = 0;
	ctx->فرزند = prog;
	prog->type = ctx->type;

	/* but ctx->pipe and ctx->list_head remain unchanged */
	أرجع 0;
}

ساكن صحيح انجامشده_pipe(بنية p_context *ctx, pipe_style type)
{
	بنية pipe *new_p;
	انجامشده_command(ctx);  /* implicit closure of previous command */
	debug_printf("done_pipe, type %d\n", type);
	ctx->pipe->followup = type;
	ctx->pipe->r_نما = ctx->w;
	new_p=new_pipe();
	ctx->pipe->بعدی = new_p;
	ctx->pipe = new_p;
	ctx->فرزند = عدم;
	انجامشده_command(ctx);  /* set up new pipe to accept commands */
	أرجع 0;
}

#اگر_تعریف_نشده __U_BOOT__
/* peek ahead in the in_str to find out if we have a "&n" construct,
 * as in "2>&1", that represents duplicating a file descriptor.
 * returns either -2 (syntax error), -1 (no &), or the number found.
 */
ساكن صحيح redirect_dup_رقم(بنية درون_str *درون_داد)
{
	صحيح ch, d=0, ok=0;
	ch = b_peek(درون_داد);
	إذا (ch != '&') أرجع -1;

	b_getch(درون_داد);  /* get the & */
	ch=b_peek(درون_داد);
	إذا (ch == '-') {
		b_getch(درون_داد);
		أرجع -3;  /* "-" represents "close me" */
	}
	طالما (آیا_رقم(ch)) {
		d = d*10+(ch-'0');
		ok=1;
		b_getch(درون_داد);
		ch = b_peek(درون_داد);
	}
	إذا (ok) أرجع d;

	error_پیام("ambiguous redirect");
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
ساكن صحيح redirect_opt_رقم(o_string *o)
{
	صحيح رقم;

	إذا (o->درازی==0) أرجع -1;
	لكل(رقم=0; رقم<o->درازی; رقم++) {
		إذا (!آیا_رقم(*(o->داده+رقم))) {
			أرجع -1;
		}
	}
	/* reuse num (and save an int) */
	رقم=atoi(o->داده);
	b_reset(o);
	أرجع رقم;
}

FILE *generate_جریان_from_list(بنية pipe *سر)
{
	FILE *pf;
#إذا 1
	صحيح pid, channel[2];
	إذا (pipe(channel)<0) perror_پیام_and_die("pipe");
	pid=fork();
	إذا (pid<0) {
		perror_پیام_and_die("fork");
	} وإلا إذا (pid==0) {
		close(channel[0]);
		إذا (channel[1] != 1) {
			dup2(channel[1],1);
			close(channel[1]);
		}
#إذا 0
#تعریف_کن SURROGATE "surrogate response"
		اكتب(1,SURROGATE,حجم(SURROGATE));
		_exit(پیاپی_list(سر));
#وإلا
		_exit(پیاپی_list_real(سر));   /* leaks memory */
#پایان_اگر
	}
	debug_printf("forked child %d\n",pid);
	close(channel[1]);
	pf = fdopen(channel[0],"r");
	debug_printf("pipe on FILE *%p\n",pf);
#وإلا
	free_pipe_list(سر,0);
	pf=popen("echo surrogate response","r");
	debug_printf("started fake pipe on FILE *%p\n",pf);
#پایان_اگر
	أرجع pf;
}

/* this version hacked for testing purposes */
/* return code is exit status of the process that is run. */
ساكن صحيح process_command_subs(o_string *dest, بنية p_context *ctx, بنية درون_str *درون_داد, صحيح subst_انتها)
{
	صحيح retcode;
	o_string نتیجه=NULL_O_STRING;
	بنية p_context inner;
	FILE *p;
	بنية درون_str pipe_str;
	initialize_context(&inner);

	/* recursion to generate command */
	retcode = parse_جریان(&نتیجه, &inner, درون_داد, subst_انتها);
	إذا (retcode != 0) أرجع retcode;  /* syntax error or EOF */
	انجامشده_کلمه(&نتیجه, &inner);
	انجامشده_pipe(&inner, PIPE_SEQ);
	b_free(&نتیجه);

	p=generate_جریان_from_list(inner.list_سر);
	إذا (p==عدم) أرجع 1;
	mark_open(fileno(p));
	راهاندازی_file_درون_str(&pipe_str, p);

	/* now send results of command back into original context */
	retcode = parse_جریان(dest, ctx, &pipe_str, '\0');
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
	free_pipe_list(inner.list_سر,0);
	debug_printf("pclosed, retcode=%d\n",retcode);
	/* XXX this process fails to trim a single trailing newline */
	أرجع retcode;
}

ساكن صحيح parse_گروه(o_string *dest, بنية p_context *ctx,
	بنية درون_str *درون_داد, صحيح ch)
{
	صحيح rcode, endch=0;
	بنية p_context sub;
	بنية فرزند_prog *فرزند = ctx->فرزند;
	إذا (فرزند->argv) {
		syntax();
		أرجع 1;  /* syntax error, groups and arglists don't mix */
	}
	initialize_context(&sub);
	بدل(ch) {
		حالة '(': endch=')'; فرزند->subshell=1; اكسر;
		حالة '{': endch='}'; اكسر;
		افتراضي: syntax();   /* really logic error */
	}
	rcode=parse_جریان(dest,&sub,درون_داد,endch);
	انجامشده_کلمه(dest,&sub); /* finish off the final word in the subcontext */
	انجامشده_pipe(&sub, PIPE_SEQ);  /* and the final command there, too */
	فرزند->گروه = sub.list_سر;
	أرجع rcode;
	/* child remains "open", available for possible redirects */
}
#پایان_اگر

/* basically useful version until someone wants to get fancier,
 * see the bash man page under "Parameter Expansion" */
ساكن حرف *lookup_param(حرف *src)
{
	حرف *p;
	حرف *sep;
	حرف *default_val = عدم;
	صحيح تخصیص_بده = 0;
	صحيح expand_خالی = 0;

	إذا (!src)
		أرجع عدم;

	sep = strchr(src, ':');

	إذا (sep) {
		*sep = '\0';
		إذا (*(sep + 1) == '-')
			default_val = sep+2;
		إذا (*(sep + 1) == '=') {
			default_val = sep+2;
			تخصیص_بده = 1;
		}
		إذا (*(sep + 1) == '+') {
			default_val = sep+2;
			expand_خالی = 1;
		}
	}

	p = env_get(src);
	إذا (!p)
		p = get_local_var(src);

	إذا (!p || طول_رشته(p) == 0) {
		p = default_val;
		إذا (تخصیص_بده) {
			حرف *var = حافظه_بده(طول_رشته(src)+طول_رشته(default_val)+2);
			إذا (var) {
				sprintf(var, "%s=%s", src, default_val);
				مجموعه_local_var(var, 0);
			}
			حرر(var);
		}
	} وإلا إذا (expand_خالی) {
		p += طول_رشته(p);
	}

	إذا (sep)
		*sep = ':';

	أرجع p;
}

#اگر_تعریف_شده __U_BOOT__
ساكن حرف *get_dollar_var(حرف ch)
{
	ساكن حرف buf[40];

	buf[0] = '\0';
	بدل (ch) {
		حالة '?':
			sprintf(buf, "%u", (بیعلامت صحيح)آخرین_return_کد);
			اكسر;
		افتراضي:
			أرجع عدم;
	}
	أرجع buf;
}
#پایان_اگر

/* return code: 0 for OK, 1 for syntax error */
ساكن صحيح handle_dollar(o_string *dest, بنية p_context *ctx, بنية درون_str *درون_داد)
{
#اگر_تعریف_نشده __U_BOOT__
	صحيح i, advance=0;
#وإلا
	صحيح advance=0;
#پایان_اگر
#اگر_تعریف_نشده __U_BOOT__
	حرف sep[]=" ";
#پایان_اگر
	صحيح ch = درون_داد->peek(درون_داد);  /* first character after the $ */
	debug_printf("handle_dollar: ch=%c\n",ch);
	إذا (آیا_حرف(ch)) {
		b_addchr(dest, SPECIAL_VAR_نماد);
		ctx->فرزند->sp++;
		طالما(ch=b_peek(درون_داد),آیا_حرف_یا_رقم(ch) || ch=='_') {
			b_getch(درون_داد);
			b_addchr(dest,ch);
		}
		b_addchr(dest, SPECIAL_VAR_نماد);
#اگر_تعریف_نشده __U_BOOT__
	} وإلا إذا (آیا_رقم(ch)) {
		i = ch-'0';  /* XXX is $0 special? */
		إذا (i<global_argc) {
			parse_string(dest, ctx, global_argv[i]); /* recursion */
		}
		advance = 1;
#پایان_اگر
	} وإلا بدل (ch) {
#اگر_تعریف_نشده __U_BOOT__
		حالة '$':
			b_adduint(dest,getpid());
			advance = 1;
			اكسر;
		حالة '!':
			إذا (آخرین_bg_pid > 0) b_adduint(dest, آخرین_bg_pid);
			advance = 1;
			اكسر;
#پایان_اگر
		حالة '?':
#اگر_تعریف_نشده __U_BOOT__
			b_adduint(dest,آخرین_return_کد);
#وإلا
			ctx->فرزند->sp++;
			b_addchr(dest, SPECIAL_VAR_نماد);
			b_addchr(dest, '$');
			b_addchr(dest, '?');
			b_addchr(dest, SPECIAL_VAR_نماد);
#پایان_اگر
			advance = 1;
			اكسر;
#اگر_تعریف_نشده __U_BOOT__
		حالة '#':
			b_adduint(dest,global_argc ? global_argc-1 : 0);
			advance = 1;
			اكسر;
#پایان_اگر
		حالة '{':
			b_addchr(dest, SPECIAL_VAR_نماد);
			ctx->فرزند->sp++;
			b_getch(درون_داد);
			/* XXX maybe someone will try to escape the '}' */
			طالما(ch=b_getch(درون_داد),ch!=EOF && ch!='}') {
				b_addchr(dest,ch);
			}
			إذا (ch != '}') {
				syntax();
				أرجع 1;
			}
			b_addchr(dest, SPECIAL_VAR_نماد);
			اكسر;
#اگر_تعریف_نشده __U_BOOT__
		حالة '(':
			b_getch(درون_داد);
			process_command_subs(dest, ctx, درون_داد, ')');
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
			error_پیام("unhandled syntax: $%c",ch);
			أرجع 1;
			اكسر;
#پایان_اگر
		افتراضي:
			b_addqchr(dest,'$',dest->quote);
	}
	/* Eat the character if the flag was set.  If the compiler
	 * is smart enough, we could substitute "b_getch(input);"
	 * for all the "advance = 1;" above, and also end up with
	 * a nice size-optimized program.  Hah!  That'll be the day.
	 */
	إذا (advance) b_getch(درون_داد);
	أرجع 0;
}

#اگر_تعریف_نشده __U_BOOT__
صحيح parse_string(o_string *dest, بنية p_context *ctx, ثابت حرف *src)
{
	بنية درون_str foo;
	راهاندازی_string_درون_str(&foo, src);
	أرجع parse_جریان(dest, ctx, &foo, '\0');
}
#پایان_اگر

/* return code is 0 for normal exit, 1 for syntax error */
ساكن صحيح parse_جریان(o_string *dest, بنية p_context *ctx,
			بنية درون_str *درون_داد, صحيح انتها_trigger)
{
	بیعلامت صحيح ch, m;
#اگر_تعریف_نشده __U_BOOT__
	صحيح redir_fd;
	redir_type redir_style;
#پایان_اگر
	صحيح بعدی;

	/* Only double-quote state is handled in the state variable dest->quote.
	 * A single-quote triggers a bypass of the main loop until its mate is
	 * found.  When recursing, quote state is passed in via dest->quote. */

	debug_printf("parse_stream, end_trigger=%d\n",انتها_trigger);
	طالما ((ch=b_getch(درون_داد))!=EOF) {
		m = map[ch];
#اگر_تعریف_شده __U_BOOT__
		إذا (درون_داد->__promptme == 0) أرجع 1;
#پایان_اگر
		بعدی = (ch == '\n') ? 0 : b_peek(درون_داد);

		debug_printf("parse_stream: ch=%c (%d) m=%d quote=%d - %c\n",
			ch >= ' ' ? ch : '.', ch, m,
			dest->quote, ctx->پشته == عدم ? '*' : '.');

		إذا (m==0 || ((m==1 || m==2) && dest->quote)) {
			b_addqchr(dest, ch, dest->quote);
		} وإلا {
			إذا (m==2) {  /* unquoted IFS */
				إذا (انجامشده_کلمه(dest, ctx)) {
					أرجع 1;
				}
				/* If we aren't performing a substitution, treat a newline as a
				 * command separator.  */
				إذا (انتها_trigger != '\0' && ch=='\n')
					انجامشده_pipe(ctx,PIPE_SEQ);
			}
			إذا (ch == انتها_trigger && !dest->quote && ctx->w==RES_NONE) {
				debug_printf("leaving parse_stream (triggered)\n");
				أرجع 0;
			}
#إذا 0
			إذا (ch=='\n') {
				/* Yahoo!  Time to run with it! */
				انجامشده_pipe(ctx,PIPE_SEQ);
				پیاپی_list(ctx->list_سر);
				initialize_context(ctx);
			}
#پایان_اگر
			إذا (m!=2) بدل (ch) {
		حالة '#':
			إذا (dest->درازی == 0 && !dest->quote) {
				طالما(ch=b_peek(درون_داد),ch!=EOF && ch!='\n') { b_getch(درون_داد); }
			} وإلا {
				b_addqchr(dest, ch, dest->quote);
			}
			اكسر;
		حالة '\\':
			إذا (بعدی == EOF) {
				syntax();
				أرجع 1;
			}
			b_addqchr(dest, '\\', dest->quote);
			b_addqchr(dest, b_getch(درون_داد), dest->quote);
			اكسر;
		حالة '$':
			إذا (handle_dollar(dest, ctx, درون_داد)!=0) أرجع 1;
			اكسر;
		حالة '\'':
			dest->nonnull = 1;
			طالما(ch=b_getch(درون_داد),ch!=EOF && ch!='\'') {
#اگر_تعریف_شده __U_BOOT__
				إذا(درون_داد->__promptme == 0) أرجع 1;
#پایان_اگر
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
#اگر_تعریف_نشده __U_BOOT__
		حالة '`':
			process_command_subs(dest, ctx, درون_داد, '`');
			اكسر;
		حالة '>':
			redir_fd = redirect_opt_رقم(dest);
			انجامشده_کلمه(dest, ctx);
			redir_style=REDIRECT_OVERWRITE;
			إذا (بعدی == '>') {
				redir_style=REDIRECT_APPEND;
				b_getch(درون_داد);
			} وإلا إذا (بعدی == '(') {
				syntax();   /* until we support >(list) Process Substitution */
				أرجع 1;
			}
			راهاندازی_redirect(ctx, redir_fd, redir_style, درون_داد);
			اكسر;
		حالة '<':
			redir_fd = redirect_opt_رقم(dest);
			انجامشده_کلمه(dest, ctx);
			redir_style=REDIRECT_درون_داد;
			إذا (بعدی == '<') {
				redir_style=REDIRECT_HEREIS;
				b_getch(درون_داد);
			} وإلا إذا (بعدی == '>') {
				redir_style=REDIRECT_IO;
				b_getch(درون_داد);
			} وإلا إذا (بعدی == '(') {
				syntax();   /* until we support <(list) Process Substitution */
				أرجع 1;
			}
			راهاندازی_redirect(ctx, redir_fd, redir_style, درون_داد);
			اكسر;
#پایان_اگر
		حالة ';':
			انجامشده_کلمه(dest, ctx);
			انجامشده_pipe(ctx,PIPE_SEQ);
			اكسر;
		حالة '&':
			انجامشده_کلمه(dest, ctx);
			إذا (بعدی=='&') {
				b_getch(درون_داد);
				انجامشده_pipe(ctx,PIPE_AND);
			} وإلا {
#اگر_تعریف_نشده __U_BOOT__
				انجامشده_pipe(ctx,PIPE_BG);
#وإلا
				syntax_err();
				أرجع 1;
#پایان_اگر
			}
			اكسر;
		حالة '|':
			انجامشده_کلمه(dest, ctx);
			إذا (بعدی=='|') {
				b_getch(درون_داد);
				انجامشده_pipe(ctx,PIPE_OR);
			} وإلا {
				/* we could pick up a file descriptor choice here
				 * with redirect_opt_num(), but bash doesn't do it.
				 * "echo foo 2| cat" yields "foo 2". */
#اگر_تعریف_نشده __U_BOOT__
				انجامشده_command(ctx);
#وإلا
				syntax_err();
				أرجع 1;
#پایان_اگر
			}
			اكسر;
#اگر_تعریف_نشده __U_BOOT__
		حالة '(':
		حالة '{':
			إذا (parse_گروه(dest, ctx, درون_داد, ch)!=0) أرجع 1;
			اكسر;
		حالة ')':
		حالة '}':
			syntax();   /* Proper use of this character caught by end_trigger */
			أرجع 1;
			اكسر;
#پایان_اگر
		حالة SUBSTED_VAR_نماد:
			dest->nonnull = 1;
			طالما (ch = b_getch(درون_داد), ch != EOF &&
			    ch != SUBSTED_VAR_نماد) {
				debug_printf("subst, pass=%d\n", ch);
				إذا (درون_داد->__promptme == 0)
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
	إذا (انتها_trigger != '\0') أرجع -1;
	أرجع 0;
}

ساكن فراغ mapset(ثابت بیعلامت حرف *مجموعه, صحيح کد)
{
	ثابت بیعلامت حرف *s;
	لكل (s=مجموعه; *s; s++) map[*s] = کد;
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
#اگر_تعریف_نشده __U_BOOT__
	mapset((uchar *)"\\$'\"`", 3);      /* never flow through */
	mapset((uchar *)"<>;&|(){}#", 1);   /* flow through if quoted */
#وإلا
	{
		uchar subst[2] = {SUBSTED_VAR_نماد, 0};
		mapset(subst, 3);       /* never flow through */
	}
	mapset((uchar *)"\\$'\"", 3);       /* never flow through */
	mapset((uchar *)";&|#", 1);         /* flow through if quoted */
#پایان_اگر
	mapset(ifs, 2);            /* also flow through if quoted */
}

/* most recursion does not come through here, the exeception is
 * from builtin_source() */
ساكن صحيح parse_جریان_outer(بنية درون_str *inp, صحيح flag)
{

	بنية p_context ctx;
	o_string موقت=NULL_O_STRING;
	صحيح rcode;
#اگر_تعریف_شده __U_BOOT__
	صحيح کد = 1;
#پایان_اگر
	افعل {
		ctx.type = flag;
		initialize_context(&ctx);
		update_ifs_map();
		إذا (!(flag & FLAG_PARSE_SEMICOLON) || (flag & FLAG_REPARSING)) mapset((uchar *)";$&|", 0);
		inp->promptmode=1;
		rcode = parse_جریان(&موقت, &ctx, inp,
				     flag & FLAG_CONT_ON_NEWLINE ? -1 : '\n');
#اگر_تعریف_شده __U_BOOT__
		إذا (rcode == 1) flag_repeat = 0;
#پایان_اگر
		إذا (rcode != 1 && ctx.old_flag != 0) {
			syntax();
#اگر_تعریف_شده __U_BOOT__
			flag_repeat = 0;
#پایان_اگر
		}
		إذا (rcode != 1 && ctx.old_flag == 0) {
			انجامشده_کلمه(&موقت, &ctx);
			انجامشده_pipe(&ctx,PIPE_SEQ);
#اگر_تعریف_نشده __U_BOOT__
			پیاپی_list(ctx.list_سر);
#وإلا
			کد = پیاپی_list(ctx.list_سر);
			إذا (کد == -2) {	/* exit */
				b_free(&موقت);
				کد = 0;
				/* XXX hackish way to not allow exit from main loop */
				إذا (inp->peek == file_peek) {
					چاپ_کن("exit not allowed from main input shell.\n");
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
			إذا (کد == -1)
			    flag_repeat = 0;
#پایان_اگر
		} وإلا {
			إذا (ctx.old_flag != 0) {
				حرر(ctx.پشته);
				b_reset(&موقت);
			}
#اگر_تعریف_شده __U_BOOT__
			إذا (inp->__promptme == 0) چاپ_کن("<INTERRUPT>\n");
			inp->__promptme = 1;
#پایان_اگر
			موقت.nonnull = 0;
			موقت.quote = 0;
			inp->p = عدم;
			free_pipe_list(ctx.list_سر,0);
		}
		b_free(&موقت);
	/* loop on syntax errors, return on EOF */
	} طالما (rcode != -1 && !(flag & FLAG_EXIT_FROM_LOOP) &&
		(inp->peek != static_peek || b_peek(inp)));
#اگر_تعریف_نشده __U_BOOT__
	أرجع 0;
#وإلا
	أرجع (کد != 0) ? 1 : 0;
#پایان_اگر /* __U_BOOT__ */
}

#اگر_تعریف_نشده __U_BOOT__
ساكن صحيح parse_string_outer(ثابت حرف *s, صحيح flag)
#وإلا
صحيح parse_string_outer(ثابت حرف *s, صحيح flag)
#پایان_اگر	/* __U_BOOT__ */
{
	بنية درون_str درون_داد;
	صحيح rcode;
#اگر_تعریف_شده __U_BOOT__
	حرف *p = عدم;
	إذا (!s)
		أرجع 1;
	إذا (!*s)
		أرجع 0;
	إذا (!(p = strchr(s, '\n')) || *++p) {
		p = xmalloc(طول_رشته(s) + 2);
		strcpy(p, s);
		strcat(p, "\n");
		راهاندازی_string_درون_str(&درون_داد, p);
		rcode = parse_جریان_outer(&درون_داد, flag);
		حرر(p);
		أرجع rcode == -2 ? آخرین_return_کد : rcode;
	} وإلا {
#پایان_اگر
	راهاندازی_string_درون_str(&درون_داد, s);
	rcode = parse_جریان_outer(&درون_داد, flag);
	أرجع rcode == -2 ? آخرین_return_کد : rcode;
#اگر_تعریف_شده __U_BOOT__
	}
#پایان_اگر
}

#اگر_تعریف_نشده __U_BOOT__
ساكن صحيح parse_file_outer(FILE *f)
#وإلا
صحيح parse_file_outer(فراغ)
#پایان_اگر
{
	صحيح rcode;
	بنية درون_str درون_داد;
#اگر_تعریف_نشده __U_BOOT__
	راهاندازی_file_درون_str(&درون_داد, f);
#وإلا
	راهاندازی_file_درون_str(&درون_داد);
#پایان_اگر
	rcode = parse_جریان_outer(&درون_داد, FLAG_PARSE_SEMICOLON);
	أرجع rcode == -2 ? آخرین_return_کد : rcode;
}

#اگر_تعریف_شده __U_BOOT__
صحيح u_boot_hush_شروع(فراغ)
{
	إذا (بالاترین_vars == عدم) {
		بالاترین_vars = حافظه_بده(حجم(بنية variables));
		بالاترین_vars->name = "HUSH_VERSION";
		بالاترین_vars->مقدار = "0.01";
		بالاترین_vars->بعدی = عدم;
		بالاترین_vars->flg_export = 0;
		بالاترین_vars->flg_read_only = 1;
	}
	أرجع 0;
}

ساكن فراغ *xmalloc(size_t اندازه)
{
	فراغ *p = عدم;

	إذا (!(p = حافظه_بده(اندازه))) {
	    چاپ_کن("ERROR : xmalloc failed\n");
	    لكل(;;);
	}
	أرجع p;
}

ساكن فراغ *xrealloc(فراغ *ptr, size_t اندازه)
{
	فراغ *p = عدم;

	إذا (!(p = realloc(ptr, اندازه))) {
	    چاپ_کن("ERROR : xrealloc failed\n");
	    لكل(;;);
	}
	أرجع p;
}
#پایان_اگر /* __U_BOOT__ */

#اگر_تعریف_نشده __U_BOOT__
/* Make sure we have a controlling tty.  If we get started under a job
 * aware app (like bash for example), make sure we are now in charge so
 * we don't fight over who gets the foreground */
ساكن فراغ راهاندازی_کار_control(فراغ)
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
	FILE *درون_داد;
	حرف **e = environ;

	/* XXX what should these be while sourcing /etc/profile? */
	global_argc = argc;
	global_argv = argv;

	/* (re?) initialize globals.  Sometimes hush_main() ends up calling
	 * hush_main(), therefore we cannot rely on the BSS to zero out this
	 * stuff.  Reset these to 0 every time. */
	ifs = عدم;
	/* map[] is taken care of with call to update_ifs_map() */
	fake_نما = 0;
	interactive = 0;
	close_me_سر = عدم;
	آخرین_bg_pid = 0;
	کار_list = عدم;
	آخرین_jobid = 0;

	/* Initialize some more globals to non-zero values */
	مجموعه_cwd();
#اگر_تعریف_شده CONFIG_FEATURE_COMMAND_EDITING
	cmdedit_مجموعه_initial_prompt();
#وإلا
	PS1 = عدم;
#پایان_اگر
	PS2 = "> ";

	/* initialize our shell local variables with the values
	 * currently living in the environment */
	إذا (e) {
		لكل (; *e; e++)
			مجموعه_local_var(*e, 2);   /* without call putenv() */
	}

	آخرین_return_کد=EXIT_SUCCESS;

	إذا (argv[0] && argv[0][0] == '-') {
		debug_printf("\nsourcing /etc/profile\n");
		إذا ((درون_داد = fopen("/etc/profile", "r")) != عدم) {
			mark_open(fileno(درون_داد));
			parse_file_outer(درون_داد);
			mark_closed(fileno(درون_داد));
			fclose(درون_داد);
		}
	}
	درون_داد=stdin;

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
				fake_نما++;
				اكسر;
			افتراضي:
#اگر_تعریف_نشده BB_VER
				fprintf(stderr, "Usage: sh [FILE]...\n"
						"   or: sh -c command [args]...\n\n");
				خروج(EXIT_FAILURE);
#وإلا
				show_usage();
#پایان_اگر
		}
	}
	/* A shell is interactive if the `-i' flag was given, or if all of
	 * the following conditions are met:
	 *	  no -c command
	 *    no arguments remaining or the -s flag given
	 *    standard input is a terminal
	 *    standard output is a terminal
	 *    Refer to Posix.2, the description of the `sh' utility. */
	إذا (argv[optind]==عدم && درون_داد==stdin &&
			isatty(fileno(stdin)) && isatty(fileno(stdout))) {
		interactive++;
	}

	debug_printf("\ninteractive=%d\n", interactive);
	إذا (interactive) {
		/* Looks like they want an interactive shell */
#اگر_تعریف_نشده CONFIG_FEATURE_SH_EXTRA_QUIET
		چاپ_کن( "\n\n" BB_BANNER " hush - the humble shell v0.01 (testing)\n");
		چاپ_کن( "Enter 'help' for a list of built-in commands.\n\n");
#پایان_اگر
		راهاندازی_کار_control();
	}

	إذا (argv[optind]==عدم) {
		opt=parse_file_outer(stdin);
		اذهب final_return;
	}

	debug_printf("\nrunning script '%s'\n", argv[optind]);
	global_argv = argv+optind;
	global_argc = argc-optind;
	درون_داد = xfopen(argv[optind], "r");
	opt = parse_file_outer(درون_داد);

#اگر_تعریف_شده CONFIG_FEATURE_CLEAN_به_بالا
	fclose(درون_داد);
	إذا (cwd && cwd != unknown)
		حرر((حرف*)cwd);
	{
		بنية variables *cur, *tmp;
		لكل(cur = بالاترین_vars; cur; cur = tmp) {
			tmp = cur->بعدی;
			إذا (!cur->flg_read_only) {
				حرر(cur->name);
				حرر(cur->مقدار);
				حرر(cur);
			}
		}
	}
#پایان_اگر

final_return:
	أرجع(opt?opt:آخرین_return_کد);
}
#پایان_اگر

ساكن حرف *درج_var_مقدار(حرف *inp)
{
	أرجع درج_var_مقدار_sub(inp, 0);
}

ساكن حرف *درج_var_مقدار_sub(حرف *inp, صحيح برچسب_subst)
{
	صحيح res_str_len = 0;
	صحيح len;
	صحيح انجامشده = 0;
	حرف *p, *p1, *res_str = عدم;

	طالما ((p = strchr(inp, SPECIAL_VAR_نماد))) {
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
		p = strchr(inp, SPECIAL_VAR_نماد);
		*p = '\0';
		/* look up the value to substitute */
		إذا ((p1 = lookup_param(inp))) {
			إذا (برچسب_subst)
				len = res_str_len + طول_رشته(p1) + 2;
			وإلا
				len = res_str_len + طول_رشته(p1);
			res_str = xrealloc(res_str, (1 + len));
			إذا (برچسب_subst) {
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
				res_str[res_str_len + 1 + طول_رشته(p1)] =
					SUBSTED_VAR_نماد;
			} وإلا
				/*
				 * copy the variable value to the result
				 * string
				 */
				strcpy((res_str + res_str_len), p1);

			res_str_len = len;
		}
		*p = SPECIAL_VAR_نماد;
		inp = ++p;
		انجامشده = 1;
	}
	إذا (انجامشده) {
		res_str = xrealloc(res_str, (1 + res_str_len + طول_رشته(inp)));
		strcpy((res_str + res_str_len), inp);
		طالما ((p = strchr(res_str, '\n'))) {
			*p = ' ';
		}
	}
	أرجع (res_str == عدم) ? inp : res_str;
}

ساكن حرف **make_list_درون(حرف **inp, حرف *name)
{
	صحيح len, i;
	صحيح name_len = طول_رشته(name);
	صحيح n = 0;
	حرف **list;
	حرف *p1, *p2, *p3;

	/* create list of variable values */
	list = xmalloc(حجم(*list));
	لكل (i = 0; inp[i]; i++) {
		p3 = درج_var_مقدار(inp[i]);
		p1 = p3;
		طالما (*p1) {
			إذا (*p1 == ' ') {
				p1++;
				استمر;
			}
			إذا ((p2 = strchr(p1, ' '))) {
				len = p2 - p1;
			} وإلا {
				len = طول_رشته(p1);
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
		p = درج_var_مقدار_sub(inp[n], noeval);
		حرف *new_str = xrealloc(str, (len + طول_رشته(p) + (2 * nonnull[n])));
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
		len = طول_رشته(str) + 3;
		إذا (p != inp[n]) حرر(p);
	}
	len = طول_رشته(str);
	*(str + len) = '\n';
	*(str + len + 1) = '\0';
	أرجع str;
}

#اگر_تعریف_شده __U_BOOT__
ساكن صحيح do_showvar(بنية cmd_tbl *cmdtp, صحيح flag, صحيح argc,
		      حرف *ثابت argv[])
{
	صحيح i, k;
	صحيح rcode = 0;
	بنية variables *cur;

	إذا (argc == 1) {		/* Print all env variables	*/
		لكل (cur = بالاترین_vars; cur; cur = cur->بعدی) {
			چاپ_کن ("%s=%s\n", cur->name, cur->مقدار);
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
		لكل (cur = بالاترین_vars; cur; cur = cur->بعدی) {
			إذا(strcmp (cur->name, name) == 0) {
				k = 0;
				چاپ_کن ("%s=%s\n", cur->name, cur->مقدار);
			}
			إذا (ctrlc ()) {
				puts ("\n ** Abort\n");
				أرجع 1;
			}
		}
		إذا (k < 0) {
			چاپ_کن ("## Error: \"%s\" not defined\n", name);
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

#پایان_اگر
/****************************************************************************/
