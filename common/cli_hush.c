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

#تعریف_کرو __U_BOOT__
#جے_تعریف_شدہ __U_BOOT__
#ضمن <env.h>
#ضمن <malloc.h>         /* malloc, free, realloc*/
#ضمن <linux/ctype.h>    /* isalpha, isdigit */
#ضمن <console.h>
#ضمن <bootretry.h>
#ضمن <cli.h>
#ضمن <cli_hush.h>
#ضمن <command.h>        /* find_cmd */
#ضمن <vsprintf.h>
#جے_انت
#جے_بنا_تعریف __U_BOOT__
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
#تعریف_کرو applet_name "hush"
#ضمن "standalone.h"
#تعریف_کرو hush_main مکھ
#تعریف_کرو BB_BANNER
#جے_انت
#جے_انت
#تعریف_کرو SPECIAL_VAR_علامت 03
#تعریف_کرو SUBSTED_VAR_علامت 04
#جے_بنا_تعریف __U_BOOT__
#تعریف_کرو FLAG_EXIT_FROM_LOOP 1
#تعریف_کرو FLAG_PARSE_SEMICOLON (1 << 1)		/* symbol ';' is special for parser */
#تعریف_کرو FLAG_REPARSING       (1 << 2)		/* >= 2nd pass */

#جے_انت

#جے_تعریف_شدہ __U_BOOT__
#تعریف_کرو EXIT_SUCCESS 0
#تعریف_کرو EOF -1
#تعریف_کرو syntax() syntax_err()
#تعریف_کرو xstrdup strdup
#تعریف_کرو error_پیغام لکھو
#وإلا
تعريفنوع تعداد {
	REDIRECT_داخلہ     = 1,
	REDIRECT_OVERWRITE = 2,
	REDIRECT_APPEND    = 3,
	REDIRECT_HEREIS    = 4,
	REDIRECT_IO        = 5
} redir_type;

/* The descrip member of this structure is only used to make debugging
 * output pretty */
بنية {صحيح منوال; صحيح default_fd; حرف *descrip;} redir_جدول[] = {
	{ 0,                         0, "()" },
	{ O_RDONLY,                  0, "<"  },
	{ O_CREAT|O_TRUNC|O_WRONLY,  1, ">"  },
	{ O_CREAT|O_APPEND|O_WRONLY, 1, ">>" },
	{ O_RDONLY,                 -1, "<<" },
	{ O_RDWR,                    1, "<>" }
};
#جے_انت

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
	RES_پورا  = 10,
	RES_XXXX  = 11,
	RES_وچ    = 12,
	RES_SNTX  = 13
} reserved_style;
#تعریف_کرو FLAG_سرا   (1<<RES_NONE)
#تعریف_کرو FLAG_IF    (1<<RES_IF)
#تعریف_کرو FLAG_THEN  (1<<RES_THEN)
#تعریف_کرو FLAG_ELIF  (1<<RES_ELIF)
#تعریف_کرو FLAG_ELSE  (1<<RES_ELSE)
#تعریف_کرو FLAG_FI    (1<<RES_FI)
#تعریف_کرو FLAG_FOR   (1<<RES_FOR)
#تعریف_کرو FLAG_WHILE (1<<RES_WHILE)
#تعریف_کرو FLAG_UNTIL (1<<RES_UNTIL)
#تعریف_کرو FLAG_DO    (1<<RES_DO)
#تعریف_کرو FLAG_پورا  (1<<RES_پورا)
#تعریف_کرو FLAG_وچ    (1<<RES_وچ)
#تعریف_کرو FLAG_شروع (1<<RES_XXXX)

/* This holds pointers to the various results of parsing */
بنية p_context {
	بنية بچہ_prog *بچہ;
	بنية pipe *list_سر;
	بنية pipe *pipe;
#جے_بنا_تعریف __U_BOOT__
	بنية redir_struct *pending_redirect;
#جے_انت
	reserved_style w;
	صحيح old_flag;				/* for figuring out valid reserved words */
	بنية p_context *ڈھیر;
	صحيح type;			/* define type of parser : ";$" common or special symbol */
	/* How about quoting status? */
};

#جے_بنا_تعریف __U_BOOT__
بنية redir_struct {
	redir_type type;			/* type of redirection */
	صحيح fd;						/* file descriptor being redirected */
	صحيح dup;					/* -1, or file descriptor being duplicated */
	بنية redir_struct *اگلا;	/* pointer to the next redirect in the list */
	glob_t لفظ;				/* *word.gl_pathv is the filename */
};
#جے_انت

بنية بچہ_prog {
#جے_بنا_تعریف __U_BOOT__
	pid_t pid;					/* 0 if exited */
#جے_انت
	حرف **argv;				/* program name and arguments */
	/* was quoted when parsed; copy of struct o_string.nonnull field */
	صحيح *argv_nonnull;
#جے_تعریف_شدہ __U_BOOT__
	صحيح    argc;                            /* number of program arguments */
#جے_انت
	بنية pipe *گروہ;			/* if non-NULL, first in group or subshell */
#جے_بنا_تعریف __U_BOOT__
	صحيح subshell;				/* flag, non-zero if group must be forked */
	بنية redir_struct *redirects;	/* I/O redirections */
	glob_t glob_نتیجہ;			/* result of parameter globbing */
	صحيح is_stopped;				/* is the program currently running? */
	بنية pipe *family;		/* pointer back to the child's parent pipe */
#جے_انت
	صحيح sp;				/* number of SPECIAL_VAR_SYMBOL */
	صحيح type;
};

بنية pipe {
#جے_بنا_تعریف __U_BOOT__
	صحيح jobid;					/* job number */
#جے_انت
	صحيح عدد_progs;				/* total number of programs in job */
#جے_بنا_تعریف __U_BOOT__
	صحيح running_progs;			/* number of programs running */
	حرف *متن;					/* name of job */
	حرف *cmdbuf;				/* buffer various argv's point into */
	pid_t pgrp;					/* process group ID for the job */
#جے_انت
	بنية بچہ_prog *progs;	/* array of commands in pipe */
	بنية pipe *اگلا;			/* to track background commands */
#جے_بنا_تعریف __U_BOOT__
	صحيح stopped_progs;			/* number of programs alive, but stopped */
	صحيح کم_context;			/* bitmask defining current context */
#جے_انت
	pipe_style followup;		/* PIPE_BG, PIPE_SEQ, PIPE_OR, PIPE_AND */
	reserved_style r_منوال;		/* supports if, for, while, until */
};

#جے_بنا_تعریف __U_BOOT__
بنية close_me {
	صحيح fd;
	بنية close_me *اگلا;
};
#جے_انت

بنية variables {
	حرف *name;
	حرف *قدر;
	صحيح flg_export;
	صحيح flg_read_only;
	بنية variables *اگلا;
};

/* globals, connect us to the outside world
 * the first three support $?, $#, and $1 */
#جے_بنا_تعریف __U_BOOT__
حرف **global_argv;
بنانشانی صحيح global_argc;
#جے_انت
ساكن بنانشانی صحيح آخری_return_کوڈ;
#جے_بنا_تعریف __U_BOOT__
خارجي حرف **environ; /* This is in <unistd.h>, but protected with __USE_GNU */
#جے_انت

/* "globals" within this file */
ساكن uchar *ifs;
ساكن حرف map[256];
#جے_بنا_تعریف __U_BOOT__
ساكن صحيح fake_منوال;
ساكن صحيح interactive;
ساكن بنية close_me *close_me_سر;
ساكن ثابت حرف *cwd;
ساكن بنية pipe *کم_list;
ساكن بنانشانی صحيح آخری_bg_pid;
ساكن بنانشانی صحيح آخری_jobid;
ساكن بنانشانی صحيح shell_terminal;
ساكن حرف *PS1;
ساكن حرف *PS2;
بنية variables shell_ver = { "HUSH_VERSION", "0.01", 1, 1, 0 };
بنية variables *چوٹی_vars = &shell_ver;
#وإلا
ساكن صحيح flag_repeat = 0;
ساكن صحيح do_repeat = 0;
ساكن بنية variables *چوٹی_vars = عدم ;
#جے_انت /*__U_BOOT__ */

#تعریف_کرو B_CHUNK (100)
#تعریف_کرو B_NOSPAC 1

تعريفنوع بنية {
	حرف *انکڑے;
	صحيح لمائی;
	صحيح maxlen;
	صحيح quote;
	صحيح nonnull;
} o_string;
#تعریف_کرو NULL_O_STRING {عدم,0,0,0,0}
/* used for initialization:
	o_string foo = NULL_O_STRING; */

/* I can almost use ordinary FILE *.  Is open_memstream() universally
 * available?  Where is it documented? */
بنية وچ_str {
	ثابت بنانشانی حرف *p;
#جے_بنا_تعریف __U_BOOT__
	حرف peek_buf[2];
#جے_انت
	صحيح __promptme;
	صحيح promptmode;
#جے_بنا_تعریف __U_BOOT__
	FILE *file;
#جے_انت
	صحيح (*get) (بنية وچ_str *);
	صحيح (*peek) (بنية وچ_str *);
};
#تعریف_کرو b_getch(داخلہ) ((داخلہ)->get(داخلہ))
#تعریف_کرو b_peek(داخلہ) ((داخلہ)->peek(داخلہ))

#جے_بنا_تعریف __U_BOOT__
#تعریف_کرو کم_حالت_FORMAT "[%d] %-22s %.40s\n"

بنية built_وچ_command {
	حرف *cmd;					/* name */
	حرف *descr;				/* description */
	صحيح (*function) (بنية بچہ_prog *);	/* function ptr */
};
#جے_انت

/* define DEBUG_SHELL for debugging output (obviously ;-)) */
#إذا 0
#تعریف_کرو DEBUG_SHELL
#جے_انت

/* This should be in utility.c */
#جے_تعریف_شدہ DEBUG_SHELL
#جے_بنا_تعریف __U_BOOT__
ساكن فراغ debug_printf(ثابت حرف *format, ...)
{
	va_list دلیلاں;
	va_شروع(دلیلاں, format);
	vfprintf(stderr, format, دلیلاں);
	va_سرا(دلیلاں);
}
#وإلا
#تعریف_کرو debug_printf(fmt,دلیلاں...)	لکھو (fmt ,##args)
#جے_انت
#وإلا
ساكن مضمن فراغ debug_printf(ثابت حرف *format, ...) { }
#جے_انت
#تعریف_کرو final_printf debug_printf

#جے_تعریف_شدہ __U_BOOT__
ساكن فراغ syntax_err(فراغ) {
	 لکھو("syntax error\n");
}
#وإلا
ساكن فراغ __syntax(حرف *file, صحيح سطر) {
	error_پیغام("syntax error %s:%d", file, سطر);
}
#تعریف_کرو syntax() __syntax(__FILE__, __LINE__)
#جے_انت

#جے_تعریف_شدہ __U_BOOT__
ساكن فراغ *xmalloc(size_t ناپ);
ساكن فراغ *xrealloc(فراغ *ptr, size_t ناپ);
#وإلا
/* Index of subroutines: */
/*   function prototypes for builtins */
ساكن صحيح builtin_cd(بنية بچہ_prog *بچہ);
ساكن صحيح builtin_env(بنية بچہ_prog *بچہ);
ساكن صحيح builtin_eval(بنية بچہ_prog *بچہ);
ساكن صحيح builtin_exec(بنية بچہ_prog *بچہ);
ساكن صحيح builtin_exit(بنية بچہ_prog *بچہ);
ساكن صحيح builtin_export(بنية بچہ_prog *بچہ);
ساكن صحيح builtin_fg_bg(بنية بچہ_prog *بچہ);
ساكن صحيح builtin_help(بنية بچہ_prog *بچہ);
ساكن صحيح builtin_jobs(بنية بچہ_prog *بچہ);
ساكن صحيح builtin_pwd(بنية بچہ_prog *بچہ);
ساكن صحيح builtin_read(بنية بچہ_prog *بچہ);
ساكن صحيح builtin_سیٹ(بنية بچہ_prog *بچہ);
ساكن صحيح builtin_کھسکاؤ(بنية بچہ_prog *بچہ);
ساكن صحيح builtin_سروت(بنية بچہ_prog *بچہ);
ساكن صحيح builtin_umask(بنية بچہ_prog *بچہ);
ساكن صحيح builtin_unset(بنية بچہ_prog *بچہ);
ساكن صحيح builtin_not_written(بنية بچہ_prog *بچہ);
#جے_انت
/*   o_string manipulation: */
ساكن صحيح b_check_space(o_string *o, صحيح len);
ساكن صحيح b_addchr(o_string *o, صحيح ch);
ساكن فراغ b_reset(o_string *o);
ساكن صحيح b_addqchr(o_string *o, صحيح ch, صحيح quote);
#جے_بنا_تعریف __U_BOOT__
ساكن صحيح b_adduint(o_string *o, بنانشانی صحيح i);
#جے_انت
/*  in_str manipulations: */
ساكن صحيح static_get(بنية وچ_str *i);
ساكن صحيح static_peek(بنية وچ_str *i);
ساكن صحيح file_get(بنية وچ_str *i);
ساكن صحيح file_peek(بنية وچ_str *i);
#جے_بنا_تعریف __U_BOOT__
ساكن فراغ تیاری_file_وچ_str(بنية وچ_str *i, FILE *f);
#وإلا
ساكن فراغ تیاری_file_وچ_str(بنية وچ_str *i);
#جے_انت
ساكن فراغ تیاری_string_وچ_str(بنية وچ_str *i, ثابت حرف *s);
#جے_بنا_تعریف __U_BOOT__
/*  close_me manipulations: */
ساكن فراغ mark_open(صحيح fd);
ساكن فراغ mark_closed(صحيح fd);
ساكن فراغ close_all(فراغ);
#جے_انت
/*  "run" the final data structures: */
ساكن حرف *indenter(صحيح i);
ساكن صحيح free_pipe_list(بنية pipe *سر, صحيح indent);
ساكن صحيح free_pipe(بنية pipe *pi, صحيح indent);
/*  really run the final data structures: */
#جے_بنا_تعریف __U_BOOT__
ساكن صحيح تیاری_redirects(بنية بچہ_prog *prog, صحيح squirrel[]);
#جے_انت
ساكن صحيح لگاتار_list_real(بنية pipe *pi);
#جے_بنا_تعریف __U_BOOT__
ساكن فراغ pseudo_exec(بنية بچہ_prog *بچہ) __attribute__ ((noreturn));
#جے_انت
ساكن صحيح لگاتار_pipe_real(بنية pipe *pi);
/*   extended glob support: */
#جے_بنا_تعریف __U_BOOT__
ساكن صحيح globhack(ثابت حرف *src, صحيح flags, glob_t *pglob);
ساكن صحيح glob_needed(ثابت حرف *s);
ساكن صحيح xglob(o_string *dest, صحيح flags, glob_t *pglob);
#جے_انت
/*   variable assignment: */
ساكن صحيح is_assignment(ثابت حرف *s);
/*   data structure manipulation: */
#جے_بنا_تعریف __U_BOOT__
ساكن صحيح تیاری_redirect(بنية p_context *ctx, صحيح fd, redir_type style, بنية وچ_str *داخلہ);
#جے_انت
ساكن فراغ initialize_context(بنية p_context *ctx);
ساكن صحيح پورا_لفظ(o_string *dest, بنية p_context *ctx);
ساكن صحيح پورا_command(بنية p_context *ctx);
ساكن صحيح پورا_pipe(بنية p_context *ctx, pipe_style type);
/*   primary string parsing: */
#جے_بنا_تعریف __U_BOOT__
ساكن صحيح redirect_dup_عدد(بنية وچ_str *داخلہ);
ساكن صحيح redirect_opt_عدد(o_string *o);
ساكن صحيح process_command_subs(o_string *dest, بنية p_context *ctx, بنية وچ_str *داخلہ, صحيح subst_سرا);
ساكن صحيح parse_گروہ(o_string *dest, بنية p_context *ctx, بنية وچ_str *داخلہ, صحيح ch);
#جے_انت
ساكن حرف *lookup_param(حرف *src);
ساكن حرف *make_string(حرف **inp, صحيح *nonnull);
ساكن صحيح handle_dollar(o_string *dest, بنية p_context *ctx, بنية وچ_str *داخلہ);
#جے_بنا_تعریف __U_BOOT__
ساكن صحيح parse_string(o_string *dest, بنية p_context *ctx, ثابت حرف *src);
#جے_انت
ساكن صحيح parse_دھارا(o_string *dest, بنية p_context *ctx, بنية وچ_str *input0, صحيح سرا_trigger);
/*   setup: */
ساكن صحيح parse_دھارا_outer(بنية وچ_str *inp, صحيح flag);
#جے_بنا_تعریف __U_BOOT__
ساكن صحيح parse_string_outer(ثابت حرف *s, صحيح flag);
ساكن صحيح parse_file_outer(FILE *f);
#جے_انت
#جے_بنا_تعریف __U_BOOT__
/*   job management: */
ساكن صحيح checkjobs(بنية pipe* fg_pipe);
ساكن فراغ داخل_کرو_bg_کم(بنية pipe *pi);
ساكن فراغ remove_bg_کم(بنية pipe *pi);
#جے_انت
/*     local variable support */
ساكن حرف **make_list_وچ(حرف **inp, حرف *name);
ساكن حرف *داخل_کرو_var_قدر(حرف *inp);
ساكن حرف *داخل_کرو_var_قدر_sub(حرف *inp, صحيح ٹیگ_subst);

#جے_بنا_تعریف __U_BOOT__
/* Table of built-in functions.  They can be forked or not, depending on
 * context: within pipes, they fork.  As simple commands, they do not.
 * When used in non-forking context, they can change global variables
 * in the parent shell process.  If forked, of course they can not.
 * For example, 'unset foo | whatever' will parse and run, but foo will
 * still be set at the end. */
ساكن بنية built_وچ_command bltins[] = {
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
	{"set", "Set/unset shell local variables", builtin_سیٹ},
	{"shift", "Shift positional parameters", builtin_کھسکاؤ},
	{"trap", "Trap signals", builtin_not_written},
	{"ulimit","Controls resource limits", builtin_not_written},
	{"umask","Sets file creation mask", builtin_umask},
	{"unset", "Unset environment variable", builtin_unset},
	{".", "Source-in and run commands in a file", builtin_سروت},
	{"help", "List shell built-in commands", builtin_help},
	{عدم, عدم, عدم}
};

ساكن ثابت حرف *سیٹ_cwd(فراغ)
{
	إذا(cwd==unknown)
		cwd = عدم;     /* xgetcwd(arg) called free(arg) */
	cwd = xgetcwd((حرف *)cwd);
	إذا (!cwd)
		cwd = unknown;
	أرجع cwd;
}

/* built-in 'eval' handler */
ساكن صحيح builtin_eval(بنية بچہ_prog *بچہ)
{
	حرف *str = عدم;
	صحيح rcode = EXIT_SUCCESS;

	إذا (بچہ->argv[1]) {
		str = make_string(بچہ->argv + 1);
		parse_string_outer(str, FLAG_EXIT_FROM_LOOP |
					FLAG_PARSE_SEMICOLON);
		حرر(str);
		rcode = آخری_return_کوڈ;
	}
	أرجع rcode;
}

/* built-in 'cd <path>' handler */
ساكن صحيح builtin_cd(بنية بچہ_prog *بچہ)
{
	حرف *newdir;
	إذا (بچہ->argv[1] == عدم)
		newdir = env_get("HOME");
	وإلا
		newdir = بچہ->argv[1];
	إذا (chdir(newdir)) {
		لکھو("cd: %s: %s\n", newdir, strerror(errno));
		أرجع EXIT_FAILURE;
	}
	سیٹ_cwd();
	أرجع EXIT_SUCCESS;
}

/* built-in 'env' handler */
ساكن صحيح builtin_env(بنية بچہ_prog *dummy)
{
	حرف **e = environ;
	إذا (e == عدم) أرجع EXIT_FAILURE;
	لكل (; *e; e++) {
		puts(*e);
	}
	أرجع EXIT_SUCCESS;
}

/* built-in 'exec' handler */
ساكن صحيح builtin_exec(بنية بچہ_prog *بچہ)
{
	إذا (بچہ->argv[1] == عدم)
		أرجع EXIT_SUCCESS;   /* Really? */
	بچہ->argv++;
	pseudo_exec(بچہ);
	/* never returns */
}

/* built-in 'exit' handler */
ساكن صحيح builtin_exit(بنية بچہ_prog *بچہ)
{
	إذا (بچہ->argv[1] == عدم)
		باہر_نکلو(آخری_return_کوڈ);
	باہر_نکلو (atoi(بچہ->argv[1]));
}

/* built-in 'export VAR=value' handler */
ساكن صحيح builtin_export(بنية بچہ_prog *بچہ)
{
	صحيح res = 0;
	حرف *name = بچہ->argv[1];

	إذا (name == عدم) {
		أرجع (builtin_env(بچہ));
	}

	name = strdup(name);

	إذا(name) {
		حرف *قدر = strchr(name, '=');

		إذا (!قدر) {
			حرف *tmp;
			/* They are exporting something without an =VALUE */

			قدر = get_local_var(name);
			إذا (قدر) {
				size_t ln = لڑی_لمبائی(name);

				tmp = realloc(name, ln+لڑی_لمبائی(قدر)+2);
				إذا(tmp==عدم)
					res = -1;
				وإلا {
					sprintf(tmp+ln, "=%s", قدر);
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
		perror_پیغام("export");
	وإلا إذا(res==0)
		res = سیٹ_local_var(name, 1);
	وإلا
		res = 0;
	حرر(name);
	أرجع res;
}

/* built-in 'fg' and 'bg' handler */
ساكن صحيح builtin_fg_bg(بنية بچہ_prog *بچہ)
{
	صحيح i, jobnum;
	بنية pipe *pi=عدم;

	إذا (!interactive)
		أرجع EXIT_FAILURE;
	/* If they gave us no args, assume they want the last backgrounded task */
	إذا (!بچہ->argv[1]) {
		لكل (pi = کم_list; pi; pi = pi->اگلا) {
			إذا (pi->jobid == آخری_jobid) {
				اكسر;
			}
		}
		إذا (!pi) {
			error_پیغام("%s: no current job", بچہ->argv[0]);
			أرجع EXIT_FAILURE;
		}
	} وإلا {
		إذا (لڑی_توں_پڑھو(بچہ->argv[1], "%%%d", &jobnum) != 1) {
			error_پیغام("%s: bad argument '%s'", بچہ->argv[0], بچہ->argv[1]);
			أرجع EXIT_FAILURE;
		}
		لكل (pi = کم_list; pi; pi = pi->اگلا) {
			إذا (pi->jobid == jobnum) {
				اكسر;
			}
		}
		إذا (!pi) {
			error_پیغام("%s: %d: no such job", بچہ->argv[0], jobnum);
			أرجع EXIT_FAILURE;
		}
	}

	إذا (*بچہ->argv[0] == 'f') {
		/* Put the job into the foreground.  */
		tcsetpgrp(shell_terminal, pi->pgrp);
	}

	/* Restart the processes in the job */
	لكل (i = 0; i < pi->عدد_progs; i++)
		pi->progs[i].is_stopped = 0;

	إذا ( (i=kill(- pi->pgrp, SIGCONT)) < 0) {
		إذا (i == ESRCH) {
			remove_bg_کم(pi);
		} وإلا {
			perror_پیغام("kill (SIGCONT)");
		}
	}

	pi->stopped_progs = 0;
	أرجع EXIT_SUCCESS;
}

/* built-in 'help' handler */
ساكن صحيح builtin_help(بنية بچہ_prog *dummy)
{
	بنية built_وچ_command *x;

	لکھو("\nBuilt-in commands:\n");
	لکھو("-------------------\n");
	لكل (x = bltins; x->cmd; x++) {
		إذا (x->descr==عدم)
			استمر;
		لکھو("%s\t%s\n", x->cmd, x->descr);
	}
	لکھو("\n\n");
	أرجع EXIT_SUCCESS;
}

/* built-in 'jobs' handler */
ساكن صحيح builtin_jobs(بنية بچہ_prog *بچہ)
{
	بنية pipe *کم;
	حرف *حالت_string;

	لكل (کم = کم_list; کم; کم = کم->اگلا) {
		إذا (کم->running_progs == کم->stopped_progs)
			حالت_string = "Stopped";
		وإلا
			حالت_string = "Running";

		لکھو(کم_حالت_FORMAT, کم->jobid, حالت_string, کم->متن);
	}
	أرجع EXIT_SUCCESS;
}

/* built-in 'pwd' handler */
ساكن صحيح builtin_pwd(بنية بچہ_prog *dummy)
{
	puts(سیٹ_cwd());
	أرجع EXIT_SUCCESS;
}

/* built-in 'read VAR' handler */
ساكن صحيح builtin_read(بنية بچہ_prog *بچہ)
{
	صحيح res;

	إذا (بچہ->argv[1]) {
		حرف string[BUFSIZ];
		حرف *var = 0;

		string[0] = 0;  /* In case stdin has only EOF */
		/* read string */
		fgets(string, حجم(string), stdin);
		chomp(string);
		var = حافظہ_دیو(لڑی_لمبائی(بچہ->argv[1])+لڑی_لمبائی(string)+2);
		إذا(var) {
			sprintf(var, "%s=%s", بچہ->argv[1], string);
			res = سیٹ_local_var(var, 0);
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
ساكن صحيح builtin_سیٹ(بنية بچہ_prog *بچہ)
{
	حرف *عارضی = بچہ->argv[1];
	بنية variables *e;

	إذا (عارضی == عدم)
		لكل(e = چوٹی_vars; e; e=e->اگلا)
			لکھو("%s=%s\n", e->name, e->قدر);
	وإلا
		سیٹ_local_var(عارضی, 0);

		أرجع EXIT_SUCCESS;
}

/* Built-in 'shift' handler */
ساكن صحيح builtin_کھسکاؤ(بنية بچہ_prog *بچہ)
{
	صحيح n=1;
	إذا (بچہ->argv[1]) {
		n=atoi(بچہ->argv[1]);
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
ساكن صحيح builtin_سروت(بنية بچہ_prog *بچہ)
{
	FILE *داخلہ;
	صحيح حالت;

	إذا (بچہ->argv[1] == عدم)
		أرجع EXIT_FAILURE;

	/* XXX search through $PATH is missing */
	داخلہ = fopen(بچہ->argv[1], "r");
	إذا (!داخلہ) {
		error_پیغام("Couldn't open file '%s'", بچہ->argv[1]);
		أرجع EXIT_FAILURE;
	}

	/* Now run the file */
	/* XXX argv and argc are broken; need to save old global_argv
	 * (pointer only is OK!) on this stack frame,
	 * set global_argv=child->argv+1, recurse, and restore. */
	mark_open(fileno(داخلہ));
	حالت = parse_file_outer(داخلہ);
	mark_closed(fileno(داخلہ));
	fclose(داخلہ);
	أرجع (حالت);
}

ساكن صحيح builtin_umask(بنية بچہ_prog *بچہ)
{
	mode_t new_umask;
	ثابت حرف *arg = بچہ->argv[1];
	حرف *سرا_2;
	إذا (arg) {
		new_umask=strtoul(arg, &سرا_2, 8);
		إذا (*سرا_2!='\0' || سرا_2 == arg) {
			أرجع EXIT_FAILURE;
		}
	} وإلا {
		لکھو("%.3o\n", (بنانشانی صحيح) (new_umask=umask(0)));
	}
	umask(new_umask);
	أرجع EXIT_SUCCESS;
}

/* built-in 'unset VAR' handler */
ساكن صحيح builtin_unset(بنية بچہ_prog *بچہ)
{
	/* bash returned already true */
	unset_local_var(بچہ->argv[1]);
	أرجع EXIT_SUCCESS;
}

ساكن صحيح builtin_not_written(بنية بچہ_prog *بچہ)
{
	لکھو("builtin_%s not written\n",بچہ->argv[0]);
	أرجع EXIT_FAILURE;
}
#جے_انت

ساكن صحيح b_check_space(o_string *o, صحيح len)
{
	/* It would be easy to drop a more restrictive policy
	 * in here, such as setting a maximum string length */
	إذا (o->لمائی + len > o->maxlen) {
		حرف *old_انکڑے = o->انکڑے;
		/* assert (data == NULL || o->maxlen != 0); */
		o->maxlen += max(2*len, B_CHUNK);
		o->انکڑے = realloc(o->انکڑے, 1 + o->maxlen);
		إذا (o->انکڑے == عدم) {
			حرر(old_انکڑے);
		}
	}
	أرجع o->انکڑے == عدم;
}

ساكن صحيح b_addchr(o_string *o, صحيح ch)
{
	debug_printf("b_addchr: %c %d %p\n", ch, o->لمائی, o);
	إذا (b_check_space(o, 1)) أرجع B_NOSPAC;
	o->انکڑے[o->لمائی] = ch;
	o->لمائی++;
	o->انکڑے[o->لمائی] = '\0';
	أرجع 0;
}

ساكن فراغ b_reset(o_string *o)
{
	o->لمائی = 0;
	o->nonnull = 0;
	إذا (o->انکڑے != عدم) *o->انکڑے = '\0';
}

ساكن فراغ b_free(o_string *o)
{
	b_reset(o);
	حرر(o->انکڑے);
	o->انکڑے = عدم;
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

#جے_بنا_تعریف __U_BOOT__
ساكن صحيح b_adduint(o_string *o, بنانشانی صحيح i)
{
	صحيح r;
	حرف *p = simple_itoa(i);
	/* no escape checking necessary */
	افعل r=b_addchr(o, *p++); طالما (r==0 && *p);
	أرجع r;
}
#جے_انت

ساكن صحيح static_get(بنية وچ_str *i)
{
	صحيح ch = *i->p++;
	إذا (ch=='\0') أرجع EOF;
	أرجع ch;
}

ساكن صحيح static_peek(بنية وچ_str *i)
{
	أرجع *i->p;
}

#جے_بنا_تعریف __U_BOOT__
ساكن مضمن فراغ cmdedit_سیٹ_initial_prompt(فراغ)
{
#جے_بنا_تعریف CONFIG_FEATURE_SH_FANCY_PROMPT
	PS1 = عدم;
#وإلا
	PS1 = env_get("PS1");
	إذا(PS1==0)
		PS1 = "\\w \\$ ";
#جے_انت
}

ساكن مضمن فراغ تیاری_prompt_string(صحيح promptmode, حرف **prompt_str)
{
	debug_printf("setup_prompt_string %d ",promptmode);
#جے_بنا_تعریف CONFIG_FEATURE_SH_FANCY_PROMPT
	/* Set up the prompt */
	إذا (promptmode == 1) {
		حرر(PS1);
		PS1=xmalloc(لڑی_لمبائی(cwd)+4);
		sprintf(PS1, "%s %s", cwd, ( geteuid() != 0 ) ?  "$ ":"# ");
		*prompt_str = PS1;
	} وإلا {
		*prompt_str = PS2;
	}
#وإلا
	*prompt_str = (promptmode==1)? PS1 : PS2;
#جے_انت
	debug_printf("result %s\n",*prompt_str);
}
#جے_انت

#جے_تعریف_شدہ __U_BOOT__
ساكن صحيح uboot_cli_readline(بنية وچ_str *i)
{
	حرف *prompt;
	حرف __maybe_unused *ps_prompt = عدم;

	إذا (i->promptmode == 1)
		prompt = CONFIG_SYS_PROMPT;
	وإلا
		prompt = CONFIG_SYS_PROMPT_HUSH_PS2;

#جے_تعریف_شدہ CONFIG_CMDLINE_PS_SUPPORT
	إذا (i->promptmode == 1)
		ps_prompt = env_get("PS1");
	وإلا
		ps_prompt = env_get("PS2");
	إذا (ps_prompt)
		prompt = ps_prompt;
#جے_انت

	أرجع cli_readline(prompt);
}
#جے_انت

ساكن فراغ get_user_داخلہ(بنية وچ_str *i)
{
#جے_بنا_تعریف __U_BOOT__
	حرف *prompt_str;
	ساكن حرف the_command[BUFSIZ];

	تیاری_prompt_string(i->promptmode, &prompt_str);
#جے_تعریف_شدہ CONFIG_FEATURE_COMMAND_EDITING
	/*
	 ** enable command line editing only while a command line
	 ** is actually being read; otherwise, we'll end up bequeathing
	 ** atexit() handlers and other unwanted stuff to our
	 ** child processes (rob@sysgo.de)
	 */
	cmdedit_read_داخلہ(prompt_str, the_command);
#وإلا
	fputs(prompt_str, stdout);
	fflush(stdout);
	the_command[0]=fgetc(i->file);
	the_command[1]='\0';
#جے_انت
	fflush(stdout);
	i->p = the_command;
#وإلا
	صحيح n;
	ساكن حرف the_command[CONFIG_SYS_CBSIZE + 1];

	bootretry_reset_cmd_timeout();
	i->__promptme = 1;
	n = uboot_cli_readline(i);

#جے_تعریف_شدہ CONFIG_BOOT_RETRY_ویلا
	إذا (n == -2) {
	  puts("\nTimeout waiting for command\n");
#  ifdef CONFIG_RESET_منزل_RETRY
	  do_reset(عدم, 0, 0, عدم);
#  elif IS_ENABLED(CONFIG_RETRY_BOOTCMD)
	strcpy(console_بفر, "run bootcmd\n");
# وإلا
#	error "This only works with CONFIG_RESET_TO_RETRY or CONFIG_BOOT_RETRY_COMMAND enabled"
#  endif
	}
#جے_انت
	إذا (n == -1 ) {
		flag_repeat = 0;
		i->__promptme = 0;
	}
	n = لڑی_لمبائی(console_بفر);
	console_بفر[n] = '\n';
	console_بفر[n+1]= '\0';
	إذا (had_ctrlc()) flag_repeat = 0;
	clear_ctrlc();
	do_repeat = 0;
	إذا (i->promptmode == 1) {
		إذا (console_بفر[0] == '\n'&& flag_repeat == 0) {
			strcpy(the_command,console_بفر);
		}
		وإلا {
			إذا (console_بفر[0] != '\n') {
				strcpy(the_command,console_بفر);
				flag_repeat = 1;
			}
			وإلا {
				do_repeat = 1;
			}
		}
		i->p = the_command;
	}
	وإلا {
		إذا (console_بفر[0] != '\n') {
			إذا (لڑی_لمبائی(the_command) + لڑی_لمبائی(console_بفر)
			    < CONFIG_SYS_CBSIZE) {
				n = لڑی_لمبائی(the_command);
				the_command[n-1] = ' ';
				strcpy(&the_command[n],console_بفر);
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
		i->p = console_بفر;
	}
#جے_انت
}

/* This is the magic location that prints prompts
 * and gets data back from the user */
ساكن صحيح file_get(بنية وچ_str *i)
{
	صحيح ch;

	ch = 0;
	/* If there is data waiting, eat it up */
	إذا (i->p && *i->p) {
		ch = *i->p++;
	} وإلا {
		/* need to double check i->file because we might be doing something
		 * more complicated by now, like sourcing or substituting. */
#جے_بنا_تعریف __U_BOOT__
		إذا (i->__promptme && interactive && i->file == stdin) {
			طالما(! i->p || (interactive && لڑی_لمبائی(i->p)==0) ) {
#وإلا
			طالما(! i->p  || لڑی_لمبائی(i->p)==0 ) {
#جے_انت
				get_user_داخلہ(i);
			}
			i->promptmode=2;
#جے_بنا_تعریف __U_BOOT__
			i->__promptme = 0;
#جے_انت
			إذا (i->p && *i->p) {
				ch = *i->p++;
			}
#جے_بنا_تعریف __U_BOOT__
		} وإلا {
			ch = fgetc(i->file);
		}

#جے_انت
		debug_printf("b_getch: got a %d\n", ch);
	}
#جے_بنا_تعریف __U_BOOT__
	إذا (ch == '\n') i->__promptme=1;
#جے_انت
	أرجع ch;
}

/* All the callers guarantee this routine will never be
 * used right after a newline, so prompting is not needed.
 */
ساكن صحيح file_peek(بنية وچ_str *i)
{
#جے_بنا_تعریف __U_BOOT__
	إذا (i->p && *i->p) {
#جے_انت
		أرجع *i->p;
#جے_بنا_تعریف __U_BOOT__
	} وإلا {
		i->peek_buf[0] = fgetc(i->file);
		i->peek_buf[1] = '\0';
		i->p = i->peek_buf;
		debug_printf("b_peek: got a %d\n", *i->p);
		أرجع *i->p;
	}
#جے_انت
}

#جے_بنا_تعریف __U_BOOT__
ساكن فراغ تیاری_file_وچ_str(بنية وچ_str *i, FILE *f)
#وإلا
ساكن فراغ تیاری_file_وچ_str(بنية وچ_str *i)
#جے_انت
{
	i->peek = file_peek;
	i->get = file_get;
	i->__promptme=1;
	i->promptmode=1;
#جے_بنا_تعریف __U_BOOT__
	i->file = f;
#جے_انت
	i->p = عدم;
}

ساكن فراغ تیاری_string_وچ_str(بنية وچ_str *i, ثابت حرف *s)
{
	i->peek = static_peek;
	i->get = static_get;
	i->__promptme=1;
	i->promptmode=1;
	i->p = s;
}

#جے_بنا_تعریف __U_BOOT__
ساكن فراغ mark_open(صحيح fd)
{
	بنية close_me *new = xmalloc(حجم(بنية close_me));
	new->fd = fd;
	new->اگلا = close_me_سر;
	close_me_سر = new;
}

ساكن فراغ mark_closed(صحيح fd)
{
	بنية close_me *tmp;
	إذا (close_me_سر == عدم || close_me_سر->fd != fd)
		error_پیغام_and_die("corrupt close_me");
	tmp = close_me_سر;
	close_me_سر = close_me_سر->اگلا;
	حرر(tmp);
}

ساكن فراغ close_all(فراغ)
{
	بنية close_me *c;
	لكل (c=close_me_سر; c; c=c->اگلا) {
		close(c->fd);
	}
	close_me_سر = عدم;
}

/* squirrel != NULL means we squirrel away copies of stdin, stdout,
 * and stderr if they are redirected. */
ساكن صحيح تیاری_redirects(بنية بچہ_prog *prog, صحيح squirrel[])
{
	صحيح openfd, منوال;
	بنية redir_struct *redir;

	لكل (redir=prog->redirects; redir; redir=redir->اگلا) {
		إذا (redir->dup == -1 && redir->لفظ.gl_pathv == عدم) {
			/* something went wrong in the parse.  Pretend it didn't happen */
			استمر;
		}
		إذا (redir->dup == -1) {
			منوال=redir_جدول[redir->type].منوال;
			openfd = open(redir->لفظ.gl_pathv[0], منوال, 0666);
			إذا (openfd < 0) {
			/* this could get lost if stderr has been redirected, but
			   bash and ash both lose it as well (though zsh doesn't!) */
				perror_پیغام("error opening %s", redir->لفظ.gl_pathv[0]);
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
ساكن فراغ pseudo_exec(بنية بچہ_prog *بچہ)
{
	صحيح i, rcode;
	حرف *p;
	بنية built_وچ_command *x;
	إذا (بچہ->argv) {
		لكل (i=0; is_assignment(بچہ->argv[i]); i++) {
			debug_printf("pid %d environment modification: %s\n",getpid(),بچہ->argv[i]);
			p = داخل_کرو_var_قدر(بچہ->argv[i]);
			putenv(strdup(p));
			إذا (p != بچہ->argv[i]) حرر(p);
		}
		بچہ->argv+=i;  /* XXX this hack isn't so horrible, since we are about
					to exit, and therefore don't need to keep data
					structures consistent for free() use. */
		/* If a variable is assigned in a forest, and nobody listens,
		 * was it ever really set?
		 */
		إذا (بچہ->argv[0] == عدم) {
			_exit(EXIT_SUCCESS);
		}

		/*
		 * Check if the command matches any of the builtins.
		 * Depending on context, this might be redundant.  But it's
		 * easier to waste a few CPU cycles than it is to figure out
		 * if this is one of those cases.
		 */
		لكل (x = bltins; x->cmd; x++) {
			إذا (strcmp(بچہ->argv[0], x->cmd) == 0 ) {
				debug_printf("builtin exec %s\n", بچہ->argv[0]);
				rcode = x->function(بچہ);
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
#جے_تعریف_شدہ CONFIG_FEATURE_SH_STANDALONE_SHELL
		{
			صحيح argc_l;
			حرف** argv_l=بچہ->argv;
			حرف *name = بچہ->argv[0];

#جے_تعریف_شدہ CONFIG_FEATURE_SH_APPLETS_ALWAYS_WIN
			/* Following discussions from November 2000 on the busybox mailing
			 * list, the default configuration, (without
			 * get_last_path_component()) lets the user force use of an
			 * external command by specifying the full (with slashes) filename.
			 * If you enable CONFIG_FEATURE_SH_APPLETS_ALWAYS_WIN then applets
			 * _aways_ override external commands, so if you want to run
			 * /bin/cat, it will use BusyBox cat even if /bin/cat exists on the
			 * filesystem and is _not_ busybox.  Some systems may want this,
			 * most do not.  */
			name = get_آخری_رستہ_component(name);
#جے_انت
			/* Count argc for use in a second... */
			لكل(argc_l=0;*argv_l!=عدم; argv_l++, argc_l++);
			optind = 1;
			debug_printf("running applet %s\n", name);
			لگاتار_applet_by_name(name, argc_l, بچہ->argv);
		}
#جے_انت
		debug_printf("exec of %s\n",بچہ->argv[0]);
		execvp(بچہ->argv[0],بچہ->argv);
		perror_پیغام("couldn't exec: %s",بچہ->argv[0]);
		_exit(1);
	} وإلا إذا (بچہ->گروہ) {
		debug_printf("runtime nesting to group\n");
		interactive=0;    /* crucial!!!! */
		rcode = لگاتار_list_real(بچہ->گروہ);
		/* OK to leak memory by not calling free_pipe_list,
		 * since this process is about to exit */
		_exit(rcode);
	} وإلا {
		/* Can happen.  See what bash does with ">foo" by itself. */
		debug_printf("trying to pseudo_exec null command\n");
		_exit(EXIT_SUCCESS);
	}
}

ساكن فراغ داخل_کرو_bg_کم(بنية pipe *pi)
{
	بنية pipe *thejob;

	/* Linear search for the ID of the job to use */
	pi->jobid = 1;
	لكل (thejob = کم_list; thejob; thejob = thejob->اگلا)
		إذا (thejob->jobid >= pi->jobid)
			pi->jobid = thejob->jobid + 1;

	/* add thejob to the list of running jobs */
	إذا (!کم_list) {
		thejob = کم_list = xmalloc(حجم(*thejob));
	} وإلا {
		لكل (thejob = کم_list; thejob->اگلا; thejob = thejob->اگلا) /* nothing */;
		thejob->اگلا = xmalloc(حجم(*thejob));
		thejob = thejob->اگلا;
	}

	/* physically copy the struct job */
	حافظہ_نقل(thejob, pi, حجم(بنية pipe));
	thejob->اگلا = عدم;
	thejob->running_progs = thejob->عدد_progs;
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
	لکھو("[%d] %d\n", thejob->jobid, thejob->progs[0].pid);
	آخری_bg_pid = thejob->progs[0].pid;
	آخری_jobid = thejob->jobid;
}

/* remove a backgrounded job */
ساكن فراغ remove_bg_کم(بنية pipe *pi)
{
	بنية pipe *prev_pipe;

	إذا (pi == کم_list) {
		کم_list = pi->اگلا;
	} وإلا {
		prev_pipe = کم_list;
		طالما (prev_pipe->اگلا != pi)
			prev_pipe = prev_pipe->اگلا;
		prev_pipe->اگلا = pi->اگلا;
	}
	إذا (کم_list)
		آخری_jobid = کم_list->jobid;
	وإلا
		آخری_jobid = 0;

	pi->stopped_progs = 0;
	free_pipe(pi, 0);
	حرر(pi);
}

/* Checks to see if any processes have exited -- if they
   have, figure out why and see if a job has completed */
ساكن صحيح checkjobs(بنية pipe* fg_pipe)
{
	صحيح گن;
	صحيح حالت;
	صحيح prognum = 0;
	بنية pipe *pi;
	pid_t childpid;

	گن = WUNTRACED;
	إذا (fg_pipe==عدم) {
		گن |= WNOHANG;
	}

	طالما ((childpid = waitpid(-1, &حالت, گن)) > 0) {
		إذا (fg_pipe) {
			صحيح i, rcode = 0;
			لكل (i=0; i < fg_pipe->عدد_progs; i++) {
				إذا (fg_pipe->progs[i].pid == childpid) {
					إذا (i==fg_pipe->عدد_progs-1)
						rcode=WEXITSTATUS(حالت);
					(fg_pipe->عدد_progs)--;
					أرجع(rcode);
				}
			}
		}

		لكل (pi = کم_list; pi; pi = pi->اگلا) {
			prognum = 0;
			طالما (prognum < pi->عدد_progs && pi->progs[prognum].pid != childpid) {
				prognum++;
			}
			إذا (prognum < pi->عدد_progs)
				اكسر;
		}

		إذا(pi==عدم) {
			debug_printf("checkjobs: pid %d was not in our list!\n", childpid);
			استمر;
		}

		إذا (WIFEXITED(حالت) || WIFSIGNALED(حالت)) {
			/* child exited */
			pi->running_progs--;
			pi->progs[prognum].pid = 0;

			إذا (!pi->running_progs) {
				لکھو(کم_حالت_FORMAT, pi->jobid, "Done", pi->متن);
				remove_bg_کم(pi);
			}
		} وإلا {
			/* child stopped */
			pi->stopped_progs++;
			pi->progs[prognum].is_stopped = 1;

#إذا 0
			/* Printing this stuff is a pain, since it tends to
			 * overwrite the prompt an inconveinient moments.  So
			 * don't do that.  */
			إذا (pi->stopped_progs == pi->عدد_progs) {
				لکھو("\n"کم_حالت_FORMAT, pi->jobid, "Stopped", pi->متن);
			}
#جے_انت
		}
	}

	إذا (childpid == -1 && errno != ECHILD)
		perror_پیغام("waitpid");

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
#جے_انت

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
ساكن صحيح لگاتار_pipe_real(بنية pipe *pi)
{
	صحيح i;
#جے_بنا_تعریف __U_BOOT__
	صحيح nextin, nextout;
	صحيح pipefds[2];				/* pipefds[0] is for reading */
	بنية بچہ_prog *بچہ;
	بنية built_وچ_command *x;
	حرف *p;
# إذا __GNUC__
	/* Avoid longjmp clobbering */
	(فراغ) &i;
	(فراغ) &nextin;
	(فراغ) &nextout;
	(فراغ) &بچہ;
# endif
#وإلا
	صحيح nextin;
	صحيح flag = do_repeat ? CMD_FLAG_REPEAT : 0;
	بنية بچہ_prog *بچہ;
	حرف *p;
# إذا __GNUC__
	/* Avoid longjmp clobbering */
	(فراغ) &i;
	(فراغ) &nextin;
	(فراغ) &بچہ;
# endif
#جے_انت	/* __U_BOOT__ */

	nextin = 0;
#جے_بنا_تعریف __U_BOOT__
	pi->pgrp = -1;
#جے_انت

	/* Check if this is a simple builtin (not part of a pipe).
	 * Builtins within pipes have to fork anyway, and are handled in
	 * pseudo_exec.  "echo foo | read bar" doesn't work on bash, either.
	 */
	إذا (pi->عدد_progs == 1) بچہ = & (pi->progs[0]);
#جے_بنا_تعریف __U_BOOT__
	إذا (pi->عدد_progs == 1 && بچہ->گروہ && بچہ->subshell == 0) {
		صحيح squirrel[] = {-1, -1, -1};
		صحيح rcode;
		debug_printf("non-subshell grouping\n");
		تیاری_redirects(بچہ, squirrel);
		/* XXX could we merge code with following builtin case,
		 * by creating a pseudo builtin that calls run_list_real? */
		rcode = لگاتار_list_real(بچہ->گروہ);
		restore_redirects(squirrel);
#وإلا
		إذا (pi->عدد_progs == 1 && بچہ->گروہ) {
		صحيح rcode;
		debug_printf("non-subshell grouping\n");
		rcode = لگاتار_list_real(بچہ->گروہ);
#جے_انت
		أرجع rcode;
	} وإلا إذا (pi->عدد_progs == 1 && pi->progs[0].argv != عدم) {
		لكل (i=0; is_assignment(بچہ->argv[i]); i++) { /* nothing */ }
		إذا (i!=0 && بچہ->argv[i]==عدم) {
			/* assignments, but no command: set the local environment */
			لكل (i=0; بچہ->argv[i]!=عدم; i++) {

				/* Ok, this case is tricky.  We have to decide if this is a
				 * local variable, or an already exported variable.  If it is
				 * already exported, we have to export the new value.  If it is
				 * not exported, we need only set this as a local variable.
				 * This junk is all to decide whether or not to export this
				 * variable. */
				صحيح export_me=0;
				حرف *name, *قدر;
				name = xstrdup(بچہ->argv[i]);
				debug_printf("Local environment set: %s\n", name);
				قدر = strchr(name, '=');
				إذا (قدر)
					*قدر=0;
#جے_بنا_تعریف __U_BOOT__
				إذا ( get_local_var(name)) {
					export_me=1;
				}
#جے_انت
				حرر(name);
				p = داخل_کرو_var_قدر(بچہ->argv[i]);
				سیٹ_local_var(p, export_me);
				إذا (p != بچہ->argv[i]) حرر(p);
			}
			أرجع EXIT_SUCCESS;   /* don't worry about errors in set_local_var() yet */
		}
		لكل (i = 0; is_assignment(بچہ->argv[i]); i++) {
			p = داخل_کرو_var_قدر(بچہ->argv[i]);
#جے_بنا_تعریف __U_BOOT__
			putenv(strdup(p));
#وإلا
			سیٹ_local_var(p, 0);
#جے_انت
			إذا (p != بچہ->argv[i]) {
				بچہ->sp--;
				حرر(p);
			}
		}
		إذا (بچہ->sp) {
			حرف * str = عدم;

			str = make_string(بچہ->argv + i,
					  بچہ->argv_nonnull + i);
			parse_string_outer(str, FLAG_EXIT_FROM_LOOP | FLAG_REPARSING);
			حرر(str);
			أرجع آخری_return_کوڈ;
		}
#جے_بنا_تعریف __U_BOOT__
		لكل (x = bltins; x->cmd; x++) {
			إذا (strcmp(بچہ->argv[i], x->cmd) == 0 ) {
				صحيح squirrel[] = {-1, -1, -1};
				صحيح rcode;
				إذا (x->function == builtin_exec && بچہ->argv[i+1]==عدم) {
					debug_printf("magic exec\n");
					تیاری_redirects(بچہ,عدم);
					أرجع EXIT_SUCCESS;
				}
				debug_printf("builtin inline %s\n", بچہ->argv[0]);
				/* XXX setup_redirects acts on file descriptors, not FILEs.
				 * This is perfect for work that comes after exec().
				 * Is it really safe for inline use?  Experimentally,
				 * things seem to work with glibc. */
				تیاری_redirects(بچہ, squirrel);

				بچہ->argv += i;  /* XXX horrible hack */
				rcode = x->function(بچہ);
				/* XXX restore hack so free() can work right */
				بچہ->argv -= i;
				restore_redirects(squirrel);
			}
			أرجع rcode;
		}
#وإلا
		/* check ";", because ,example , argv consist from
		 * "help;flinfo" must not execute
		 */
		إذا (strchr(بچہ->argv[i], ';')) {
			لکھو("Unknown command '%s' - try 'help' or use "
					"'run' command\n", بچہ->argv[i]);
			أرجع -1;
		}
		/* Process the command */
		أرجع cmd_process(flag, بچہ->argc - i, بچہ->argv + i,
				   &flag_repeat, عدم);
#جے_انت
	}
#جے_بنا_تعریف __U_BOOT__

	لكل (i = 0; i < pi->عدد_progs; i++) {
		بچہ = & (pi->progs[i]);

		/* pipes are inserted between pairs of commands */
		إذا ((i + 1) < pi->عدد_progs) {
			إذا (pipe(pipefds)<0) perror_پیغام_and_die("pipe");
			nextout = pipefds[1];
		} وإلا {
			nextout=1;
			pipefds[0] = -1;
		}

		/* XXX test for failed fork()? */
		إذا (!(بچہ->pid = fork())) {
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
			تیاری_redirects(بچہ,عدم);

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

			pseudo_exec(بچہ);
		}

		/* put our child in the process group whose leader is the
		   first process in this pipe */
		إذا (pi->pgrp < 0) {
			pi->pgrp = بچہ->pid;
		}
		/* Don't check for errors.  The child may be dead already,
		 * in which case setpgid returns error code EACCES. */
		setpgid(بچہ->pid, pi->pgrp);

		إذا (nextin != 0)
			close(nextin);
		إذا (nextout != 1)
			close(nextout);

		/* If there isn't another process, nextin is garbage
		   but it doesn't matter */
		nextin = pipefds[0];
	}
#جے_انت
	أرجع -1;
}

ساكن صحيح لگاتار_list_real(بنية pipe *pi)
{
	حرف *save_name = عدم;
	حرف **list = عدم;
	حرف **save_list = عدم;
	بنية pipe *rpipe;
	صحيح flag_rep = 0;
#جے_بنا_تعریف __U_BOOT__
	صحيح save_عدد_progs;
#جے_انت
	صحيح rcode=0, flag_skip=1;
	صحيح flag_restore = 0;
	صحيح if_کوڈ=0, اگلا_if_کوڈ=0;  /* need double-buffer to handle elif */
	reserved_style rmode, skip_ہور_وچ_this_rmode=RES_XXXX;
	/* check syntax for "for" */
	لكل (rpipe = pi; rpipe; rpipe = rpipe->اگلا) {
		إذا ((rpipe->r_منوال == RES_وچ ||
		    rpipe->r_منوال == RES_FOR) &&
		    (rpipe->اگلا == عدم)) {
				syntax();
#جے_تعریف_شدہ __U_BOOT__
				flag_repeat = 0;
#جے_انت
				أرجع 1;
		}
		إذا ((rpipe->r_منوال == RES_وچ &&
			(rpipe->اگلا->r_منوال == RES_وچ &&
			rpipe->اگلا->progs->argv != عدم))||
			(rpipe->r_منوال == RES_FOR &&
			rpipe->اگلا->r_منوال != RES_وچ)) {
				syntax();
#جے_تعریف_شدہ __U_BOOT__
				flag_repeat = 0;
#جے_انت
				أرجع 1;
		}
	}
	لكل (; pi; pi = (flag_restore != 0) ? rpipe : pi->اگلا) {
		إذا (pi->r_منوال == RES_WHILE || pi->r_منوال == RES_UNTIL ||
			pi->r_منوال == RES_FOR) {
#جے_تعریف_شدہ __U_BOOT__
				/* check Ctrl-C */
				ctrlc();
				إذا ((had_ctrlc())) {
					أرجع 1;
				}
#جے_انت
				flag_restore = 0;
				إذا (!rpipe) {
					flag_rep = 0;
					rpipe = pi;
				}
		}
		rmode = pi->r_منوال;
		debug_printf("rmode=%d  if_code=%d  next_if_code=%d skip_more=%d\n", rmode, if_کوڈ, اگلا_if_کوڈ, skip_ہور_وچ_this_rmode);
		إذا (rmode == skip_ہور_وچ_this_rmode && flag_skip) {
			إذا (pi->followup == PIPE_SEQ) flag_skip=0;
			استمر;
		}
		flag_skip = 1;
		skip_ہور_وچ_this_rmode = RES_XXXX;
		إذا (rmode == RES_THEN || rmode == RES_ELSE) if_کوڈ = اگلا_if_کوڈ;
		إذا (rmode == RES_THEN &&  if_کوڈ) استمر;
		إذا (rmode == RES_ELSE && !if_کوڈ) استمر;
		إذا (rmode == RES_ELIF && !if_کوڈ) اكسر;
		إذا (rmode == RES_FOR && pi->عدد_progs) {
			إذا (!list) {
				/* if no variable values after "in" we skip "for" */
				إذا (!pi->اگلا->progs->argv) استمر;
				/* create list of variable values */
				list = make_list_وچ(pi->اگلا->progs->argv,
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
#جے_بنا_تعریف __U_BOOT__
				pi->progs->glob_نتیجہ.gl_pathv[0] =
					pi->progs->argv[0];
#جے_انت
				استمر;
			} وإلا {
				/* insert new value from list for variable */
				حرر(pi->progs->argv[0]);
				pi->progs->argv[0] = *list++;
#جے_بنا_تعریف __U_BOOT__
				pi->progs->glob_نتیجہ.gl_pathv[0] =
					pi->progs->argv[0];
#جے_انت
			}
		}
		إذا (rmode == RES_وچ) استمر;
		إذا (rmode == RES_DO) {
			إذا (!flag_rep) استمر;
		}
		إذا (rmode == RES_پورا) {
			إذا (flag_rep) {
				flag_restore = 1;
			} وإلا {
				rpipe = عدم;
			}
		}
		إذا (pi->عدد_progs == 0) استمر;
#جے_بنا_تعریف __U_BOOT__
		save_عدد_progs = pi->عدد_progs; /* save number of programs */
#جے_انت
		rcode = لگاتار_pipe_real(pi);
		debug_printf("run_pipe_real returned %d\n",rcode);
#جے_بنا_تعریف __U_BOOT__
		إذا (rcode!=-1) {
			/* We only ran a builtin: rcode was set by the return value
			 * of run_pipe_real(), and we don't need to wait for anything. */
		} وإلا إذا (pi->followup==PIPE_BG) {
			/* XXX check bash's behavior with nontrivial pipes */
			/* XXX compute jobid */
			/* XXX what does bash do with attempts to background builtins? */
			داخل_کرو_bg_کم(pi);
			rcode = EXIT_SUCCESS;
		} وإلا {
			إذا (interactive) {
				/* move the new process group into the foreground */
				إذا (tcsetpgrp(shell_terminal, pi->pgrp) && errno != ENOTTY)
					perror_پیغام("tcsetpgrp-3");
				rcode = checkjobs(pi);
				/* move the shell to the foreground */
				إذا (tcsetpgrp(shell_terminal, getpgid(0)) && errno != ENOTTY)
					perror_پیغام("tcsetpgrp-4");
			} وإلا {
				rcode = checkjobs(pi);
			}
			debug_printf("checkjobs returned %d\n",rcode);
		}
		آخری_return_کوڈ=rcode;
#وإلا
		إذا (rcode < -1) {
			آخری_return_کوڈ = -rcode - 2;
			أرجع -2;	/* exit */
		}
		آخری_return_کوڈ = rcode;
#جے_انت
#جے_بنا_تعریف __U_BOOT__
		pi->عدد_progs = save_عدد_progs; /* restore number of programs */
#جے_انت
		إذا ( rmode == RES_IF || rmode == RES_ELIF )
			اگلا_if_کوڈ=rcode;  /* can be overwritten a number of times */
		إذا (rmode == RES_WHILE)
			flag_rep = !آخری_return_کوڈ;
		إذا (rmode == RES_UNTIL)
			flag_rep = آخری_return_کوڈ;
		إذا ( (rcode==EXIT_SUCCESS && pi->followup==PIPE_OR) ||
		     (rcode!=EXIT_SUCCESS && pi->followup==PIPE_AND) )
			skip_ہور_وچ_this_rmode=rmode;
#جے_بنا_تعریف __U_BOOT__
		checkjobs(عدم);
#جے_انت
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
	بنية بچہ_prog *بچہ;
#جے_بنا_تعریف __U_BOOT__
	بنية redir_struct *r, *rnext;
#جے_انت
	صحيح a, i, ret_کوڈ=0;
	حرف *ind = indenter(indent);

#جے_بنا_تعریف __U_BOOT__
	إذا (pi->stopped_progs > 0)
		أرجع ret_کوڈ;
	final_printf("%s run pipe: (pid %d)\n",ind,getpid());
#جے_انت
	لكل (i=0; i<pi->عدد_progs; i++) {
		بچہ = &pi->progs[i];
		final_printf("%s  command %d:\n",ind,i);
		إذا (بچہ->argv) {
			لكل (a=0,p=بچہ->argv; *p; a++,p++) {
				final_printf("%s   argv[%d] = %s\n",ind,a,*p);
			}
#جے_بنا_تعریف __U_BOOT__
			globfree(&بچہ->glob_نتیجہ);
#وإلا
			لكل (a = 0; a < بچہ->argc; a++) {
				حرر(بچہ->argv[a]);
			}
			حرر(بچہ->argv);
			حرر(بچہ->argv_nonnull);
			بچہ->argc = 0;
#جے_انت
			بچہ->argv=عدم;
		} وإلا إذا (بچہ->گروہ) {
#جے_بنا_تعریف __U_BOOT__
			final_printf("%s   begin group (subshell:%d)\n",ind, بچہ->subshell);
#جے_انت
			ret_کوڈ = free_pipe_list(بچہ->گروہ,indent+3);
			final_printf("%s   end group\n",ind);
		} وإلا {
			final_printf("%s   (nil)\n",ind);
		}
#جے_بنا_تعریف __U_BOOT__
		لكل (r=بچہ->redirects; r; r=rnext) {
			final_printf("%s   redirect %d%s", ind, r->fd, redir_جدول[r->type].descrip);
			إذا (r->dup == -1) {
				/* guard against the case >$FOO, where foo is unset or blank */
				إذا (r->لفظ.gl_pathv) {
					final_printf(" %s\n", *r->لفظ.gl_pathv);
					globfree(&r->لفظ);
				}
			} وإلا {
				final_printf("&%d\n", r->dup);
			}
			rnext=r->اگلا;
			حرر(r);
		}
		بچہ->redirects=عدم;
#جے_انت
	}
	حرر(pi->progs);   /* children are an array, they get freed all at once */
	pi->progs=عدم;
	أرجع ret_کوڈ;
}

ساكن صحيح free_pipe_list(بنية pipe *سر, صحيح indent)
{
	صحيح rcode=0;   /* if list has no members */
	بنية pipe *pi, *اگلا;
	حرف *ind = indenter(indent);
	لكل (pi=سر; pi; pi=اگلا) {
		final_printf("%s pipe reserved mode %d\n", ind, pi->r_منوال);
		rcode = free_pipe(pi, indent);
		final_printf("%s pipe followup code %d\n", ind, pi->followup);
		اگلا=pi->اگلا;
		pi->اگلا=عدم;
		حرر(pi);
	}
	أرجع rcode;
}

/* Select which version we will use */
ساكن صحيح لگاتار_list(بنية pipe *pi)
{
	صحيح rcode=0;
#جے_بنا_تعریف __U_BOOT__
	إذا (fake_منوال==0) {
#جے_انت
		rcode = لگاتار_list_real(pi);
#جے_بنا_تعریف __U_BOOT__
	}
#جے_انت
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
#جے_بنا_تعریف __U_BOOT__
ساكن صحيح globhack(ثابت حرف *src, صحيح flags, glob_t *pglob)
{
	صحيح cnt=0, pathc;
	ثابت حرف *s;
	حرف *dest;
	لكل (cnt=1, s=src; s && *s; s++) {
		إذا (*s == '\\') s++;
		cnt++;
	}
	dest = حافظہ_دیو(cnt);
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
#جے_انت

ساكن صحيح xglob(o_string *dest, صحيح flags, glob_t *pglob)
{
	صحيح gr;

	/* short-circuit for null word */
	/* we can code this better when the debug_printf's are gone */
	إذا (dest->لمائی == 0) {
		إذا (dest->nonnull) {
			/* bash man page calls this an "explicit" null */
			gr = globhack(dest->انکڑے, flags, pglob);
			debug_printf("globhack returned %d\n",gr);
		} وإلا {
			أرجع 0;
		}
	} وإلا إذا (glob_needed(dest->انکڑے)) {
		gr = glob(dest->انکڑے, flags, عدم, pglob);
		debug_printf("glob returned %d\n",gr);
		إذا (gr == GLOB_NOMATCH) {
			/* quote removal, or more accurately, backslash removal */
			gr = globhack(dest->انکڑے, flags, pglob);
			debug_printf("globhack returned %d\n",gr);
		}
	} وإلا {
		gr = globhack(dest->انکڑے, flags, pglob);
		debug_printf("globhack returned %d\n",gr);
	}
	إذا (gr == GLOB_NOSPACE)
		error_پیغام_and_die("out of memory during glob");
	إذا (gr != 0) { /* GLOB_ABORTED ? */
		error_پیغام("glob(3) error %d",gr);
	}
	/* globprint(glob_target); */
	أرجع gr;
}
#جے_انت

#جے_تعریف_شدہ __U_BOOT__
ساكن حرف *get_dollar_var(حرف ch);
#جے_انت

/* This is used to get/check local shell variables */
حرف *get_local_var(ثابت حرف *s)
{
	بنية variables *cur;

	إذا (!s)
		أرجع عدم;

#جے_تعریف_شدہ __U_BOOT__
	إذا (*s == '$')
		أرجع get_dollar_var(s[1]);
#جے_انت

	لكل (cur = چوٹی_vars; cur; cur=cur->اگلا)
		إذا(strcmp(cur->name, s)==0)
			أرجع cur->قدر;
	أرجع عدم;
}

/* This is used to set local shell variables
   flg_export==0 if only local (not exporting) variable
   flg_export==1 if "new" exporting environ
   flg_export>1  if current startup environ (not call putenv()) */
صحيح سیٹ_local_var(ثابت حرف *s, صحيح flg_export)
{
	حرف *name, *قدر;
	صحيح نتیجہ=0;
	بنية variables *cur;

#جے_تعریف_شدہ __U_BOOT__
	/* might be possible! */
	إذا (!حرف_اے(*s))
		أرجع -1;
#جے_انت

	name=strdup(s);

	/* Assume when we enter this function that we are already in
	 * NAME=VALUE format.  So the first order of business is to
	 * split 's' on the '=' into 'name' and 'value' */
	قدر = strchr(name, '=');
	إذا (!قدر) {
		حرر(name);
		أرجع -1;
	}
	*قدر++ = 0;

	إذا (!*قدر) {
		unset_local_var(name);
		حرر(name);
		أرجع 0;
	}

	لكل(cur = چوٹی_vars; cur; cur = cur->اگلا) {
		إذا(strcmp(cur->name, name)==0)
			اكسر;
	}

	إذا(cur) {
		إذا(strcmp(cur->قدر, قدر)==0) {
			إذا(flg_export>0 && cur->flg_export==0)
				cur->flg_export=flg_export;
			وإلا
				نتیجہ++;
		} وإلا {
			إذا(cur->flg_read_only) {
				error_پیغام("%s: readonly variable", name);
				نتیجہ = -1;
			} وإلا {
				إذا(flg_export>0 || cur->flg_export>1)
					cur->flg_export=1;
				حرر(cur->قدر);

				cur->قدر = strdup(قدر);
			}
		}
	} وإلا {
		cur = حافظہ_دیو(حجم(بنية variables));
		إذا(!cur) {
			نتیجہ = -1;
		} وإلا {
			cur->name = strdup(name);
			إذا (cur->name == عدم) {
				حرر(cur);
				نتیجہ = -1;
			} وإلا {
				بنية variables *bottom = چوٹی_vars;
				cur->قدر = strdup(قدر);
				cur->اگلا = عدم;
				cur->flg_export = flg_export;
				cur->flg_read_only = 0;
				طالما(bottom->اگلا) bottom=bottom->اگلا;
				bottom->اگلا = cur;
			}
		}
	}

#جے_بنا_تعریف __U_BOOT__
	إذا(نتیجہ==0 && cur->flg_export==1) {
		*(قدر-1) = '=';
		نتیجہ = putenv(name);
	} وإلا {
#جے_انت
		حرر(name);
#جے_بنا_تعریف __U_BOOT__
		إذا(نتیجہ>0)            /* equivalent to previous set */
			نتیجہ = 0;
	}
#جے_انت
	أرجع نتیجہ;
}

فراغ unset_local_var(ثابت حرف *name)
{
	بنية variables *cur;

	إذا (name) {
		لكل (cur = چوٹی_vars; cur; cur=cur->اگلا) {
			إذا(strcmp(cur->name, name)==0)
				اكسر;
		}
		إذا (cur != عدم) {
			بنية variables *اگلا = چوٹی_vars;
			إذا(cur->flg_read_only) {
				error_پیغام("%s: readonly variable", name);
				أرجع;
			} وإلا {
#جے_بنا_تعریف __U_BOOT__
				إذا(cur->flg_export)
					unenv_سیٹ(cur->name);
#جے_انت
				حرر(cur->name);
				حرر(cur->قدر);
				طالما (اگلا->اگلا != cur)
					اگلا = اگلا->اگلا;
				اگلا->اگلا = cur->اگلا;
			}
			حرر(cur);
		}
	}
}

ساكن صحيح is_assignment(ثابت حرف *s)
{
	إذا (s == عدم)
		أرجع 0;

	إذا (!حرف_اے(*s)) أرجع 0;
	++s;
	طالما(حرف_یا_ہندسہ(*s) || *s=='_') ++s;
	أرجع *s=='=';
}

#جے_بنا_تعریف __U_BOOT__
/* the src parameter allows us to peek forward to a possible &n syntax
 * for file descriptor duplication, e.g., "2>&1".
 * Return code is 0 normally, 1 if a syntax error is detected in src.
 * Resource errors (in xmalloc) cause the process to exit */
ساكن صحيح تیاری_redirect(بنية p_context *ctx, صحيح fd, redir_type style,
	بنية وچ_str *داخلہ)
{
	بنية بچہ_prog *بچہ=ctx->بچہ;
	بنية redir_struct *redir = بچہ->redirects;
	بنية redir_struct *آخری_redir=عدم;

	/* Create a new redir_struct and drop it onto the end of the linked list */
	طالما(redir) {
		آخری_redir=redir;
		redir=redir->اگلا;
	}
	redir = xmalloc(حجم(بنية redir_struct));
	redir->اگلا=عدم;
	redir->لفظ.gl_pathv=عدم;
	إذا (آخری_redir) {
		آخری_redir->اگلا=redir;
	} وإلا {
		بچہ->redirects=redir;
	}

	redir->type=style;
	redir->fd= (fd==-1) ? redir_جدول[style].default_fd : fd ;

	debug_printf("Redirect type %d%s\n", redir->fd, redir_جدول[style].descrip);

	/* Check for a '2>&1' type redirect */
	redir->dup = redirect_dup_عدد(داخلہ);
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
#جے_انت

ساكن بنية pipe *new_pipe(فراغ)
{
	بنية pipe *pi;
	pi = xmalloc(حجم(بنية pipe));
	pi->عدد_progs = 0;
	pi->progs = عدم;
	pi->اگلا = عدم;
	pi->followup = 0;  /* invalid */
	pi->r_منوال = RES_NONE;
	أرجع pi;
}

ساكن فراغ initialize_context(بنية p_context *ctx)
{
	ctx->pipe=عدم;
#جے_بنا_تعریف __U_BOOT__
	ctx->pending_redirect=عدم;
#جے_انت
	ctx->بچہ=عدم;
	ctx->list_سر=new_pipe();
	ctx->pipe=ctx->list_سر;
	ctx->w=RES_NONE;
	ctx->ڈھیر=عدم;
#جے_تعریف_شدہ __U_BOOT__
	ctx->old_flag=0;
#جے_انت
	پورا_command(ctx);   /* creates the memory for working child */
}

/* normal return is 0
 * if a reserved word is found, and processed, return 1
 * should handle if, then, elif, else, fi, for, while, until, do, done.
 * case, function, and select are obnoxious, save those for later.
 */
بنية reserved_combo {
	حرف *literal;
	صحيح کوڈ;
	لما flag;
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
	{ "fi",    RES_FI,    FLAG_سرا  },
	{ "for",   RES_FOR,   FLAG_وچ   | FLAG_شروع },
	{ "while", RES_WHILE, FLAG_DO   | FLAG_شروع },
	{ "until", RES_UNTIL, FLAG_DO   | FLAG_شروع },
	{ "in",    RES_وچ,    FLAG_DO   },
	{ "do",    RES_DO,    FLAG_پورا },
	{ "done",  RES_پورا,  FLAG_سرا  }
};
#تعریف_کرو NRES (حجم(reserved_list)/حجم(بنية reserved_combo))

ساكن صحيح reserved_لفظ(o_string *dest, بنية p_context *ctx)
{
	بنية reserved_combo *r;
	لكل (r=reserved_list;
		r<reserved_list+NRES; r++) {
		إذا (strcmp(dest->انکڑے, r->literal) == 0) {
			debug_printf("found reserved word %s, code %d\n",r->literal,r->کوڈ);
			إذا (r->flag & FLAG_شروع) {
				بنية p_context *new = xmalloc(حجم(بنية p_context));
				debug_printf("push stack\n");
				إذا (ctx->w == RES_وچ || ctx->w == RES_FOR) {
					syntax();
					حرر(new);
					ctx->w = RES_SNTX;
					b_reset(dest);
					أرجع 1;
				}
				*new = *ctx;   /* physical copy */
				initialize_context(ctx);
				ctx->ڈھیر=new;
			} وإلا إذا ( ctx->w == RES_NONE || ! (ctx->old_flag & (1<<r->کوڈ))) {
				syntax();
				ctx->w = RES_SNTX;
				b_reset(dest);
				أرجع 1;
			}
			ctx->w=r->کوڈ;
			ctx->old_flag = r->flag;
			إذا (ctx->old_flag & FLAG_سرا) {
				بنية p_context *old;
				debug_printf("pop stack\n");
				پورا_pipe(ctx,PIPE_SEQ);
				old = ctx->ڈھیر;
				old->بچہ->گروہ = ctx->list_سر;
#جے_بنا_تعریف __U_BOOT__
				old->بچہ->subshell = 0;
#جے_انت
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
ساكن صحيح پورا_لفظ(o_string *dest, بنية p_context *ctx)
{
	بنية بچہ_prog *بچہ=ctx->بچہ;
#جے_بنا_تعریف __U_BOOT__
	glob_t *glob_نشانہ;
	صحيح gr, flags = 0;
#وإلا
	حرف *str, *s;
	صحيح argc, cnt;
#جے_انت

	debug_printf("done_word: %s %p\n", dest->انکڑے, بچہ);
	إذا (dest->لمائی == 0 && !dest->nonnull) {
		debug_printf("  true null, ignored\n");
		أرجع 0;
	}
#جے_بنا_تعریف __U_BOOT__
	إذا (ctx->pending_redirect) {
		glob_نشانہ = &ctx->pending_redirect->لفظ;
	} وإلا {
#جے_انت
		إذا (بچہ->گروہ) {
			syntax();
			أرجع 1;  /* syntax error, groups and arglists don't mix */
		}
		إذا (!بچہ->argv && (ctx->type & FLAG_PARSE_SEMICOLON)) {
			debug_printf("checking %s for reserved-ness\n",dest->انکڑے);
			إذا (reserved_لفظ(dest,ctx)) أرجع ctx->w==RES_SNTX;
		}
#جے_بنا_تعریف __U_BOOT__
		glob_نشانہ = &بچہ->glob_نتیجہ;
		إذا (بچہ->argv) flags |= GLOB_APPEND;
#وإلا
		لكل (cnt = 1, s = dest->انکڑے; s && *s; s++) {
			إذا (*s == '\\') s++;
			cnt++;
		}
		str = حافظہ_دیو(cnt);
		إذا (!str) أرجع 1;
		إذا ( بچہ->argv == عدم) {
			بچہ->argc=0;
		}
		argc = ++بچہ->argc;
		بچہ->argv = realloc(بچہ->argv, (argc+1)*حجم(*بچہ->argv));
		إذا (بچہ->argv == عدم) {
			حرر(str);
			أرجع 1;
		}
		بچہ->argv_nonnull = realloc(بچہ->argv_nonnull,
					(argc+1)*حجم(*بچہ->argv_nonnull));
		إذا (بچہ->argv_nonnull == عدم) {
			حرر(str);
			أرجع 1;
		}
		بچہ->argv[argc-1]=str;
		بچہ->argv_nonnull[argc-1] = dest->nonnull;
		بچہ->argv[argc]=عدم;
		بچہ->argv_nonnull[argc] = 0;
		لكل (s = dest->انکڑے; s && *s; s++,str++) {
			إذا (*s == '\\') s++;
			*str = *s;
		}
		*str = '\0';
#جے_انت
#جے_بنا_تعریف __U_BOOT__
	}
	gr = xglob(dest, flags, glob_نشانہ);
	إذا (gr != 0) أرجع 1;
#جے_انت

	b_reset(dest);
#جے_بنا_تعریف __U_BOOT__
	إذا (ctx->pending_redirect) {
		ctx->pending_redirect=عدم;
		إذا (glob_نشانہ->gl_pathc != 1) {
			error_پیغام("ambiguous redirect");
			أرجع 1;
		}
	} وإلا {
		بچہ->argv = glob_نشانہ->gl_pathv;
	}
#جے_انت
	إذا (ctx->w == RES_FOR) {
		پورا_لفظ(dest,ctx);
		پورا_pipe(ctx,PIPE_SEQ);
	}
	أرجع 0;
}

/* The only possible error here is out of memory, in which case
 * xmalloc exits. */
ساكن صحيح پورا_command(بنية p_context *ctx)
{
	/* The child is really already in the pipe structure, so
	 * advance the pipe counter and make a new, null child.
	 * Only real trickiness here is that the uncommitted
	 * child structure, to which ctx->child points, is not
	 * counted in pi->num_progs. */
	بنية pipe *pi=ctx->pipe;
	بنية بچہ_prog *prog=ctx->بچہ;

	إذا (prog && prog->گروہ == عدم
		 && prog->argv == عدم
#جے_بنا_تعریف __U_BOOT__
		 && prog->redirects == عدم) {
#وإلا
										) {
#جے_انت
		debug_printf("done_command: skipping null command\n");
		أرجع 0;
	} وإلا إذا (prog) {
		pi->عدد_progs++;
		debug_printf("done_command: num_progs incremented to %d\n",pi->عدد_progs);
	} وإلا {
		debug_printf("done_command: initializing\n");
	}
	pi->progs = xrealloc(pi->progs, حجم(*pi->progs) * (pi->عدد_progs+1));

	prog = pi->progs + pi->عدد_progs;
#جے_بنا_تعریف __U_BOOT__
	prog->redirects = عدم;
#جے_انت
	prog->argv = عدم;
	prog->argv_nonnull = عدم;
#جے_بنا_تعریف __U_BOOT__
	prog->is_stopped = 0;
#جے_انت
	prog->گروہ = عدم;
#جے_بنا_تعریف __U_BOOT__
	prog->glob_نتیجہ.gl_pathv = عدم;
	prog->family = pi;
#جے_انت
	prog->sp = 0;
	ctx->بچہ = prog;
	prog->type = ctx->type;

	/* but ctx->pipe and ctx->list_head remain unchanged */
	أرجع 0;
}

ساكن صحيح پورا_pipe(بنية p_context *ctx, pipe_style type)
{
	بنية pipe *new_p;
	پورا_command(ctx);  /* implicit closure of previous command */
	debug_printf("done_pipe, type %d\n", type);
	ctx->pipe->followup = type;
	ctx->pipe->r_منوال = ctx->w;
	new_p=new_pipe();
	ctx->pipe->اگلا = new_p;
	ctx->pipe = new_p;
	ctx->بچہ = عدم;
	پورا_command(ctx);  /* set up new pipe to accept commands */
	أرجع 0;
}

#جے_بنا_تعریف __U_BOOT__
/* peek ahead in the in_str to find out if we have a "&n" construct,
 * as in "2>&1", that represents duplicating a file descriptor.
 * returns either -2 (syntax error), -1 (no &), or the number found.
 */
ساكن صحيح redirect_dup_عدد(بنية وچ_str *داخلہ)
{
	صحيح ch, d=0, ok=0;
	ch = b_peek(داخلہ);
	إذا (ch != '&') أرجع -1;

	b_getch(داخلہ);  /* get the & */
	ch=b_peek(داخلہ);
	إذا (ch == '-') {
		b_getch(داخلہ);
		أرجع -3;  /* "-" represents "close me" */
	}
	طالما (ہندسہ_اے(ch)) {
		d = d*10+(ch-'0');
		ok=1;
		b_getch(داخلہ);
		ch = b_peek(داخلہ);
	}
	إذا (ok) أرجع d;

	error_پیغام("ambiguous redirect");
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
ساكن صحيح redirect_opt_عدد(o_string *o)
{
	صحيح عدد;

	إذا (o->لمائی==0) أرجع -1;
	لكل(عدد=0; عدد<o->لمائی; عدد++) {
		إذا (!ہندسہ_اے(*(o->انکڑے+عدد))) {
			أرجع -1;
		}
	}
	/* reuse num (and save an int) */
	عدد=atoi(o->انکڑے);
	b_reset(o);
	أرجع عدد;
}

FILE *generate_دھارا_from_list(بنية pipe *سر)
{
	FILE *pf;
#إذا 1
	صحيح pid, channel[2];
	إذا (pipe(channel)<0) perror_پیغام_and_die("pipe");
	pid=fork();
	إذا (pid<0) {
		perror_پیغام_and_die("fork");
	} وإلا إذا (pid==0) {
		close(channel[0]);
		إذا (channel[1] != 1) {
			dup2(channel[1],1);
			close(channel[1]);
		}
#إذا 0
#تعریف_کرو SURROGATE "surrogate response"
		اكتب(1,SURROGATE,حجم(SURROGATE));
		_exit(لگاتار_list(سر));
#وإلا
		_exit(لگاتار_list_real(سر));   /* leaks memory */
#جے_انت
	}
	debug_printf("forked child %d\n",pid);
	close(channel[1]);
	pf = fdopen(channel[0],"r");
	debug_printf("pipe on FILE *%p\n",pf);
#وإلا
	free_pipe_list(سر,0);
	pf=popen("echo surrogate response","r");
	debug_printf("started fake pipe on FILE *%p\n",pf);
#جے_انت
	أرجع pf;
}

/* this version hacked for testing purposes */
/* return code is exit status of the process that is run. */
ساكن صحيح process_command_subs(o_string *dest, بنية p_context *ctx, بنية وچ_str *داخلہ, صحيح subst_سرا)
{
	صحيح retcode;
	o_string نتیجہ=NULL_O_STRING;
	بنية p_context inner;
	FILE *p;
	بنية وچ_str pipe_str;
	initialize_context(&inner);

	/* recursion to generate command */
	retcode = parse_دھارا(&نتیجہ, &inner, داخلہ, subst_سرا);
	إذا (retcode != 0) أرجع retcode;  /* syntax error or EOF */
	پورا_لفظ(&نتیجہ, &inner);
	پورا_pipe(&inner, PIPE_SEQ);
	b_free(&نتیجہ);

	p=generate_دھارا_from_list(inner.list_سر);
	إذا (p==عدم) أرجع 1;
	mark_open(fileno(p));
	تیاری_file_وچ_str(&pipe_str, p);

	/* now send results of command back into original context */
	retcode = parse_دھارا(dest, ctx, &pipe_str, '\0');
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

ساكن صحيح parse_گروہ(o_string *dest, بنية p_context *ctx,
	بنية وچ_str *داخلہ, صحيح ch)
{
	صحيح rcode, endch=0;
	بنية p_context sub;
	بنية بچہ_prog *بچہ = ctx->بچہ;
	إذا (بچہ->argv) {
		syntax();
		أرجع 1;  /* syntax error, groups and arglists don't mix */
	}
	initialize_context(&sub);
	بدل(ch) {
		حالة '(': endch=')'; بچہ->subshell=1; اكسر;
		حالة '{': endch='}'; اكسر;
		افتراضي: syntax();   /* really logic error */
	}
	rcode=parse_دھارا(dest,&sub,داخلہ,endch);
	پورا_لفظ(dest,&sub); /* finish off the final word in the subcontext */
	پورا_pipe(&sub, PIPE_SEQ);  /* and the final command there, too */
	بچہ->گروہ = sub.list_سر;
	أرجع rcode;
	/* child remains "open", available for possible redirects */
}
#جے_انت

/* basically useful version until someone wants to get fancier,
 * see the bash man page under "Parameter Expansion" */
ساكن حرف *lookup_param(حرف *src)
{
	حرف *p;
	حرف *sep;
	حرف *default_val = عدم;
	صحيح سونپو = 0;
	صحيح expand_خالی_تھاواں = 0;

	إذا (!src)
		أرجع عدم;

	sep = strchr(src, ':');

	إذا (sep) {
		*sep = '\0';
		إذا (*(sep + 1) == '-')
			default_val = sep+2;
		إذا (*(sep + 1) == '=') {
			default_val = sep+2;
			سونپو = 1;
		}
		إذا (*(sep + 1) == '+') {
			default_val = sep+2;
			expand_خالی_تھاواں = 1;
		}
	}

	p = env_get(src);
	إذا (!p)
		p = get_local_var(src);

	إذا (!p || لڑی_لمبائی(p) == 0) {
		p = default_val;
		إذا (سونپو) {
			حرف *var = حافظہ_دیو(لڑی_لمبائی(src)+لڑی_لمبائی(default_val)+2);
			إذا (var) {
				sprintf(var, "%s=%s", src, default_val);
				سیٹ_local_var(var, 0);
			}
			حرر(var);
		}
	} وإلا إذا (expand_خالی_تھاواں) {
		p += لڑی_لمبائی(p);
	}

	إذا (sep)
		*sep = ':';

	أرجع p;
}

#جے_تعریف_شدہ __U_BOOT__
ساكن حرف *get_dollar_var(حرف ch)
{
	ساكن حرف buf[40];

	buf[0] = '\0';
	بدل (ch) {
		حالة '?':
			sprintf(buf, "%u", (بنانشانی صحيح)آخری_return_کوڈ);
			اكسر;
		افتراضي:
			أرجع عدم;
	}
	أرجع buf;
}
#جے_انت

/* return code: 0 for OK, 1 for syntax error */
ساكن صحيح handle_dollar(o_string *dest, بنية p_context *ctx, بنية وچ_str *داخلہ)
{
#جے_بنا_تعریف __U_BOOT__
	صحيح i, advance=0;
#وإلا
	صحيح advance=0;
#جے_انت
#جے_بنا_تعریف __U_BOOT__
	حرف sep[]=" ";
#جے_انت
	صحيح ch = داخلہ->peek(داخلہ);  /* first character after the $ */
	debug_printf("handle_dollar: ch=%c\n",ch);
	إذا (حرف_اے(ch)) {
		b_addchr(dest, SPECIAL_VAR_علامت);
		ctx->بچہ->sp++;
		طالما(ch=b_peek(داخلہ),حرف_یا_ہندسہ(ch) || ch=='_') {
			b_getch(داخلہ);
			b_addchr(dest,ch);
		}
		b_addchr(dest, SPECIAL_VAR_علامت);
#جے_بنا_تعریف __U_BOOT__
	} وإلا إذا (ہندسہ_اے(ch)) {
		i = ch-'0';  /* XXX is $0 special? */
		إذا (i<global_argc) {
			parse_string(dest, ctx, global_argv[i]); /* recursion */
		}
		advance = 1;
#جے_انت
	} وإلا بدل (ch) {
#جے_بنا_تعریف __U_BOOT__
		حالة '$':
			b_adduint(dest,getpid());
			advance = 1;
			اكسر;
		حالة '!':
			إذا (آخری_bg_pid > 0) b_adduint(dest, آخری_bg_pid);
			advance = 1;
			اكسر;
#جے_انت
		حالة '?':
#جے_بنا_تعریف __U_BOOT__
			b_adduint(dest,آخری_return_کوڈ);
#وإلا
			ctx->بچہ->sp++;
			b_addchr(dest, SPECIAL_VAR_علامت);
			b_addchr(dest, '$');
			b_addchr(dest, '?');
			b_addchr(dest, SPECIAL_VAR_علامت);
#جے_انت
			advance = 1;
			اكسر;
#جے_بنا_تعریف __U_BOOT__
		حالة '#':
			b_adduint(dest,global_argc ? global_argc-1 : 0);
			advance = 1;
			اكسر;
#جے_انت
		حالة '{':
			b_addchr(dest, SPECIAL_VAR_علامت);
			ctx->بچہ->sp++;
			b_getch(داخلہ);
			/* XXX maybe someone will try to escape the '}' */
			طالما(ch=b_getch(داخلہ),ch!=EOF && ch!='}') {
				b_addchr(dest,ch);
			}
			إذا (ch != '}') {
				syntax();
				أرجع 1;
			}
			b_addchr(dest, SPECIAL_VAR_علامت);
			اكسر;
#جے_بنا_تعریف __U_BOOT__
		حالة '(':
			b_getch(داخلہ);
			process_command_subs(dest, ctx, داخلہ, ')');
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
			error_پیغام("unhandled syntax: $%c",ch);
			أرجع 1;
			اكسر;
#جے_انت
		افتراضي:
			b_addqchr(dest,'$',dest->quote);
	}
	/* Eat the character if the flag was set.  If the compiler
	 * is smart enough, we could substitute "b_getch(input);"
	 * for all the "advance = 1;" above, and also end up with
	 * a nice size-optimized program.  Hah!  That'll be the day.
	 */
	إذا (advance) b_getch(داخلہ);
	أرجع 0;
}

#جے_بنا_تعریف __U_BOOT__
صحيح parse_string(o_string *dest, بنية p_context *ctx, ثابت حرف *src)
{
	بنية وچ_str foo;
	تیاری_string_وچ_str(&foo, src);
	أرجع parse_دھارا(dest, ctx, &foo, '\0');
}
#جے_انت

/* return code is 0 for normal exit, 1 for syntax error */
ساكن صحيح parse_دھارا(o_string *dest, بنية p_context *ctx,
			بنية وچ_str *داخلہ, صحيح سرا_trigger)
{
	بنانشانی صحيح ch, m;
#جے_بنا_تعریف __U_BOOT__
	صحيح redir_fd;
	redir_type redir_style;
#جے_انت
	صحيح اگلا;

	/* Only double-quote state is handled in the state variable dest->quote.
	 * A single-quote triggers a bypass of the main loop until its mate is
	 * found.  When recursing, quote state is passed in via dest->quote. */

	debug_printf("parse_stream, end_trigger=%d\n",سرا_trigger);
	طالما ((ch=b_getch(داخلہ))!=EOF) {
		m = map[ch];
#جے_تعریف_شدہ __U_BOOT__
		إذا (داخلہ->__promptme == 0) أرجع 1;
#جے_انت
		اگلا = (ch == '\n') ? 0 : b_peek(داخلہ);

		debug_printf("parse_stream: ch=%c (%d) m=%d quote=%d - %c\n",
			ch >= ' ' ? ch : '.', ch, m,
			dest->quote, ctx->ڈھیر == عدم ? '*' : '.');

		إذا (m==0 || ((m==1 || m==2) && dest->quote)) {
			b_addqchr(dest, ch, dest->quote);
		} وإلا {
			إذا (m==2) {  /* unquoted IFS */
				إذا (پورا_لفظ(dest, ctx)) {
					أرجع 1;
				}
				/* If we aren't performing a substitution, treat a newline as a
				 * command separator.  */
				إذا (سرا_trigger != '\0' && ch=='\n')
					پورا_pipe(ctx,PIPE_SEQ);
			}
			إذا (ch == سرا_trigger && !dest->quote && ctx->w==RES_NONE) {
				debug_printf("leaving parse_stream (triggered)\n");
				أرجع 0;
			}
#إذا 0
			إذا (ch=='\n') {
				/* Yahoo!  Time to run with it! */
				پورا_pipe(ctx,PIPE_SEQ);
				لگاتار_list(ctx->list_سر);
				initialize_context(ctx);
			}
#جے_انت
			إذا (m!=2) بدل (ch) {
		حالة '#':
			إذا (dest->لمائی == 0 && !dest->quote) {
				طالما(ch=b_peek(داخلہ),ch!=EOF && ch!='\n') { b_getch(داخلہ); }
			} وإلا {
				b_addqchr(dest, ch, dest->quote);
			}
			اكسر;
		حالة '\\':
			إذا (اگلا == EOF) {
				syntax();
				أرجع 1;
			}
			b_addqchr(dest, '\\', dest->quote);
			b_addqchr(dest, b_getch(داخلہ), dest->quote);
			اكسر;
		حالة '$':
			إذا (handle_dollar(dest, ctx, داخلہ)!=0) أرجع 1;
			اكسر;
		حالة '\'':
			dest->nonnull = 1;
			طالما(ch=b_getch(داخلہ),ch!=EOF && ch!='\'') {
#جے_تعریف_شدہ __U_BOOT__
				إذا(داخلہ->__promptme == 0) أرجع 1;
#جے_انت
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
#جے_بنا_تعریف __U_BOOT__
		حالة '`':
			process_command_subs(dest, ctx, داخلہ, '`');
			اكسر;
		حالة '>':
			redir_fd = redirect_opt_عدد(dest);
			پورا_لفظ(dest, ctx);
			redir_style=REDIRECT_OVERWRITE;
			إذا (اگلا == '>') {
				redir_style=REDIRECT_APPEND;
				b_getch(داخلہ);
			} وإلا إذا (اگلا == '(') {
				syntax();   /* until we support >(list) Process Substitution */
				أرجع 1;
			}
			تیاری_redirect(ctx, redir_fd, redir_style, داخلہ);
			اكسر;
		حالة '<':
			redir_fd = redirect_opt_عدد(dest);
			پورا_لفظ(dest, ctx);
			redir_style=REDIRECT_داخلہ;
			إذا (اگلا == '<') {
				redir_style=REDIRECT_HEREIS;
				b_getch(داخلہ);
			} وإلا إذا (اگلا == '>') {
				redir_style=REDIRECT_IO;
				b_getch(داخلہ);
			} وإلا إذا (اگلا == '(') {
				syntax();   /* until we support <(list) Process Substitution */
				أرجع 1;
			}
			تیاری_redirect(ctx, redir_fd, redir_style, داخلہ);
			اكسر;
#جے_انت
		حالة ';':
			پورا_لفظ(dest, ctx);
			پورا_pipe(ctx,PIPE_SEQ);
			اكسر;
		حالة '&':
			پورا_لفظ(dest, ctx);
			إذا (اگلا=='&') {
				b_getch(داخلہ);
				پورا_pipe(ctx,PIPE_AND);
			} وإلا {
#جے_بنا_تعریف __U_BOOT__
				پورا_pipe(ctx,PIPE_BG);
#وإلا
				syntax_err();
				أرجع 1;
#جے_انت
			}
			اكسر;
		حالة '|':
			پورا_لفظ(dest, ctx);
			إذا (اگلا=='|') {
				b_getch(داخلہ);
				پورا_pipe(ctx,PIPE_OR);
			} وإلا {
				/* we could pick up a file descriptor choice here
				 * with redirect_opt_num(), but bash doesn't do it.
				 * "echo foo 2| cat" yields "foo 2". */
#جے_بنا_تعریف __U_BOOT__
				پورا_command(ctx);
#وإلا
				syntax_err();
				أرجع 1;
#جے_انت
			}
			اكسر;
#جے_بنا_تعریف __U_BOOT__
		حالة '(':
		حالة '{':
			إذا (parse_گروہ(dest, ctx, داخلہ, ch)!=0) أرجع 1;
			اكسر;
		حالة ')':
		حالة '}':
			syntax();   /* Proper use of this character caught by end_trigger */
			أرجع 1;
			اكسر;
#جے_انت
		حالة SUBSTED_VAR_علامت:
			dest->nonnull = 1;
			طالما (ch = b_getch(داخلہ), ch != EOF &&
			    ch != SUBSTED_VAR_علامت) {
				debug_printf("subst, pass=%d\n", ch);
				إذا (داخلہ->__promptme == 0)
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
	إذا (سرا_trigger != '\0') أرجع -1;
	أرجع 0;
}

ساكن فراغ mapset(ثابت بنانشانی حرف *سیٹ, صحيح کوڈ)
{
	ثابت بنانشانی حرف *s;
	لكل (s=سیٹ; *s; s++) map[*s] = کوڈ;
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
#جے_بنا_تعریف __U_BOOT__
	mapset((uchar *)"\\$'\"`", 3);      /* never flow through */
	mapset((uchar *)"<>;&|(){}#", 1);   /* flow through if quoted */
#وإلا
	{
		uchar subst[2] = {SUBSTED_VAR_علامت, 0};
		mapset(subst, 3);       /* never flow through */
	}
	mapset((uchar *)"\\$'\"", 3);       /* never flow through */
	mapset((uchar *)";&|#", 1);         /* flow through if quoted */
#جے_انت
	mapset(ifs, 2);            /* also flow through if quoted */
}

/* most recursion does not come through here, the exeception is
 * from builtin_source() */
ساكن صحيح parse_دھارا_outer(بنية وچ_str *inp, صحيح flag)
{

	بنية p_context ctx;
	o_string عارضی=NULL_O_STRING;
	صحيح rcode;
#جے_تعریف_شدہ __U_BOOT__
	صحيح کوڈ = 1;
#جے_انت
	افعل {
		ctx.type = flag;
		initialize_context(&ctx);
		update_ifs_map();
		إذا (!(flag & FLAG_PARSE_SEMICOLON) || (flag & FLAG_REPARSING)) mapset((uchar *)";$&|", 0);
		inp->promptmode=1;
		rcode = parse_دھارا(&عارضی, &ctx, inp,
				     flag & FLAG_CONT_ON_NEWLINE ? -1 : '\n');
#جے_تعریف_شدہ __U_BOOT__
		إذا (rcode == 1) flag_repeat = 0;
#جے_انت
		إذا (rcode != 1 && ctx.old_flag != 0) {
			syntax();
#جے_تعریف_شدہ __U_BOOT__
			flag_repeat = 0;
#جے_انت
		}
		إذا (rcode != 1 && ctx.old_flag == 0) {
			پورا_لفظ(&عارضی, &ctx);
			پورا_pipe(&ctx,PIPE_SEQ);
#جے_بنا_تعریف __U_BOOT__
			لگاتار_list(ctx.list_سر);
#وإلا
			کوڈ = لگاتار_list(ctx.list_سر);
			إذا (کوڈ == -2) {	/* exit */
				b_free(&عارضی);
				کوڈ = 0;
				/* XXX hackish way to not allow exit from main loop */
				إذا (inp->peek == file_peek) {
					لکھو("exit not allowed from main input shell.\n");
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
			إذا (کوڈ == -1)
			    flag_repeat = 0;
#جے_انت
		} وإلا {
			إذا (ctx.old_flag != 0) {
				حرر(ctx.ڈھیر);
				b_reset(&عارضی);
			}
#جے_تعریف_شدہ __U_BOOT__
			إذا (inp->__promptme == 0) لکھو("<INTERRUPT>\n");
			inp->__promptme = 1;
#جے_انت
			عارضی.nonnull = 0;
			عارضی.quote = 0;
			inp->p = عدم;
			free_pipe_list(ctx.list_سر,0);
		}
		b_free(&عارضی);
	/* loop on syntax errors, return on EOF */
	} طالما (rcode != -1 && !(flag & FLAG_EXIT_FROM_LOOP) &&
		(inp->peek != static_peek || b_peek(inp)));
#جے_بنا_تعریف __U_BOOT__
	أرجع 0;
#وإلا
	أرجع (کوڈ != 0) ? 1 : 0;
#جے_انت /* __U_BOOT__ */
}

#جے_بنا_تعریف __U_BOOT__
ساكن صحيح parse_string_outer(ثابت حرف *s, صحيح flag)
#وإلا
صحيح parse_string_outer(ثابت حرف *s, صحيح flag)
#جے_انت	/* __U_BOOT__ */
{
	بنية وچ_str داخلہ;
	صحيح rcode;
#جے_تعریف_شدہ __U_BOOT__
	حرف *p = عدم;
	إذا (!s)
		أرجع 1;
	إذا (!*s)
		أرجع 0;
	إذا (!(p = strchr(s, '\n')) || *++p) {
		p = xmalloc(لڑی_لمبائی(s) + 2);
		strcpy(p, s);
		strcat(p, "\n");
		تیاری_string_وچ_str(&داخلہ, p);
		rcode = parse_دھارا_outer(&داخلہ, flag);
		حرر(p);
		أرجع rcode == -2 ? آخری_return_کوڈ : rcode;
	} وإلا {
#جے_انت
	تیاری_string_وچ_str(&داخلہ, s);
	rcode = parse_دھارا_outer(&داخلہ, flag);
	أرجع rcode == -2 ? آخری_return_کوڈ : rcode;
#جے_تعریف_شدہ __U_BOOT__
	}
#جے_انت
}

#جے_بنا_تعریف __U_BOOT__
ساكن صحيح parse_file_outer(FILE *f)
#وإلا
صحيح parse_file_outer(فراغ)
#جے_انت
{
	صحيح rcode;
	بنية وچ_str داخلہ;
#جے_بنا_تعریف __U_BOOT__
	تیاری_file_وچ_str(&داخلہ, f);
#وإلا
	تیاری_file_وچ_str(&داخلہ);
#جے_انت
	rcode = parse_دھارا_outer(&داخلہ, FLAG_PARSE_SEMICOLON);
	أرجع rcode == -2 ? آخری_return_کوڈ : rcode;
}

#جے_تعریف_شدہ __U_BOOT__
صحيح u_boot_hush_شروع(فراغ)
{
	إذا (چوٹی_vars == عدم) {
		چوٹی_vars = حافظہ_دیو(حجم(بنية variables));
		چوٹی_vars->name = "HUSH_VERSION";
		چوٹی_vars->قدر = "0.01";
		چوٹی_vars->اگلا = عدم;
		چوٹی_vars->flg_export = 0;
		چوٹی_vars->flg_read_only = 1;
	}
	أرجع 0;
}

ساكن فراغ *xmalloc(size_t ناپ)
{
	فراغ *p = عدم;

	إذا (!(p = حافظہ_دیو(ناپ))) {
	    لکھو("ERROR : xmalloc failed\n");
	    لكل(;;);
	}
	أرجع p;
}

ساكن فراغ *xrealloc(فراغ *ptr, size_t ناپ)
{
	فراغ *p = عدم;

	إذا (!(p = realloc(ptr, ناپ))) {
	    لکھو("ERROR : xrealloc failed\n");
	    لكل(;;);
	}
	أرجع p;
}
#جے_انت /* __U_BOOT__ */

#جے_بنا_تعریف __U_BOOT__
/* Make sure we have a controlling tty.  If we get started under a job
 * aware app (like bash for example), make sure we are now in charge so
 * we don't fight over who gets the foreground */
ساكن فراغ تیاری_کم_control(فراغ)
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
	FILE *داخلہ;
	حرف **e = environ;

	/* XXX what should these be while sourcing /etc/profile? */
	global_argc = argc;
	global_argv = argv;

	/* (re?) initialize globals.  Sometimes hush_main() ends up calling
	 * hush_main(), therefore we cannot rely on the BSS to zero out this
	 * stuff.  Reset these to 0 every time. */
	ifs = عدم;
	/* map[] is taken care of with call to update_ifs_map() */
	fake_منوال = 0;
	interactive = 0;
	close_me_سر = عدم;
	آخری_bg_pid = 0;
	کم_list = عدم;
	آخری_jobid = 0;

	/* Initialize some more globals to non-zero values */
	سیٹ_cwd();
#جے_تعریف_شدہ CONFIG_FEATURE_COMMAND_EDITING
	cmdedit_سیٹ_initial_prompt();
#وإلا
	PS1 = عدم;
#جے_انت
	PS2 = "> ";

	/* initialize our shell local variables with the values
	 * currently living in the environment */
	إذا (e) {
		لكل (; *e; e++)
			سیٹ_local_var(*e, 2);   /* without call putenv() */
	}

	آخری_return_کوڈ=EXIT_SUCCESS;

	إذا (argv[0] && argv[0][0] == '-') {
		debug_printf("\nsourcing /etc/profile\n");
		إذا ((داخلہ = fopen("/etc/profile", "r")) != عدم) {
			mark_open(fileno(داخلہ));
			parse_file_outer(داخلہ);
			mark_closed(fileno(داخلہ));
			fclose(داخلہ);
		}
	}
	داخلہ=stdin;

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
				fake_منوال++;
				اكسر;
			افتراضي:
#جے_بنا_تعریف BB_VER
				fprintf(stderr, "Usage: sh [FILE]...\n"
						"   or: sh -c command [args]...\n\n");
				باہر_نکلو(EXIT_FAILURE);
#وإلا
				show_usage();
#جے_انت
		}
	}
	/* A shell is interactive if the `-i' flag was given, or if all of
	 * the following conditions are met:
	 *	  no -c command
	 *    no arguments remaining or the -s flag given
	 *    standard input is a terminal
	 *    standard output is a terminal
	 *    Refer to Posix.2, the description of the `sh' utility. */
	إذا (argv[optind]==عدم && داخلہ==stdin &&
			isatty(fileno(stdin)) && isatty(fileno(stdout))) {
		interactive++;
	}

	debug_printf("\ninteractive=%d\n", interactive);
	إذا (interactive) {
		/* Looks like they want an interactive shell */
#جے_بنا_تعریف CONFIG_FEATURE_SH_EXTRA_QUIET
		لکھو( "\n\n" BB_BANNER " hush - the humble shell v0.01 (testing)\n");
		لکھو( "Enter 'help' for a list of built-in commands.\n\n");
#جے_انت
		تیاری_کم_control();
	}

	إذا (argv[optind]==عدم) {
		opt=parse_file_outer(stdin);
		اذهب final_return;
	}

	debug_printf("\nrunning script '%s'\n", argv[optind]);
	global_argv = argv+optind;
	global_argc = argc-optind;
	داخلہ = xfopen(argv[optind], "r");
	opt = parse_file_outer(داخلہ);

#جے_تعریف_شدہ CONFIG_FEATURE_CLEAN_اتے
	fclose(داخلہ);
	إذا (cwd && cwd != unknown)
		حرر((حرف*)cwd);
	{
		بنية variables *cur, *tmp;
		لكل(cur = چوٹی_vars; cur; cur = tmp) {
			tmp = cur->اگلا;
			إذا (!cur->flg_read_only) {
				حرر(cur->name);
				حرر(cur->قدر);
				حرر(cur);
			}
		}
	}
#جے_انت

final_return:
	أرجع(opt?opt:آخری_return_کوڈ);
}
#جے_انت

ساكن حرف *داخل_کرو_var_قدر(حرف *inp)
{
	أرجع داخل_کرو_var_قدر_sub(inp, 0);
}

ساكن حرف *داخل_کرو_var_قدر_sub(حرف *inp, صحيح ٹیگ_subst)
{
	صحيح res_str_len = 0;
	صحيح len;
	صحيح پورا = 0;
	حرف *p, *p1, *res_str = عدم;

	طالما ((p = strchr(inp, SPECIAL_VAR_علامت))) {
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
		p = strchr(inp, SPECIAL_VAR_علامت);
		*p = '\0';
		/* look up the value to substitute */
		إذا ((p1 = lookup_param(inp))) {
			إذا (ٹیگ_subst)
				len = res_str_len + لڑی_لمبائی(p1) + 2;
			وإلا
				len = res_str_len + لڑی_لمبائی(p1);
			res_str = xrealloc(res_str, (1 + len));
			إذا (ٹیگ_subst) {
				/*
				 * copy the variable value to the result
				 * string
				 */
				strcpy((res_str + res_str_len + 1), p1);

				/*
				 * mark the replaced text to be accepted as
				 * is
				 */
				res_str[res_str_len] = SUBSTED_VAR_علامت;
				res_str[res_str_len + 1 + لڑی_لمبائی(p1)] =
					SUBSTED_VAR_علامت;
			} وإلا
				/*
				 * copy the variable value to the result
				 * string
				 */
				strcpy((res_str + res_str_len), p1);

			res_str_len = len;
		}
		*p = SPECIAL_VAR_علامت;
		inp = ++p;
		پورا = 1;
	}
	إذا (پورا) {
		res_str = xrealloc(res_str, (1 + res_str_len + لڑی_لمبائی(inp)));
		strcpy((res_str + res_str_len), inp);
		طالما ((p = strchr(res_str, '\n'))) {
			*p = ' ';
		}
	}
	أرجع (res_str == عدم) ? inp : res_str;
}

ساكن حرف **make_list_وچ(حرف **inp, حرف *name)
{
	صحيح len, i;
	صحيح name_len = لڑی_لمبائی(name);
	صحيح n = 0;
	حرف **list;
	حرف *p1, *p2, *p3;

	/* create list of variable values */
	list = xmalloc(حجم(*list));
	لكل (i = 0; inp[i]; i++) {
		p3 = داخل_کرو_var_قدر(inp[i]);
		p1 = p3;
		طالما (*p1) {
			إذا (*p1 == ' ') {
				p1++;
				استمر;
			}
			إذا ((p2 = strchr(p1, ' '))) {
				len = p2 - p1;
			} وإلا {
				len = لڑی_لمبائی(p1);
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
		p = داخل_کرو_var_قدر_sub(inp[n], noeval);
		حرف *new_str = xrealloc(str, (len + لڑی_لمبائی(p) + (2 * nonnull[n])));
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
		len = لڑی_لمبائی(str) + 3;
		إذا (p != inp[n]) حرر(p);
	}
	len = لڑی_لمبائی(str);
	*(str + len) = '\n';
	*(str + len + 1) = '\0';
	أرجع str;
}

#جے_تعریف_شدہ __U_BOOT__
ساكن صحيح do_showvar(بنية cmd_tbl *cmdtp, صحيح flag, صحيح argc,
		      حرف *ثابت argv[])
{
	صحيح i, k;
	صحيح rcode = 0;
	بنية variables *cur;

	إذا (argc == 1) {		/* Print all env variables	*/
		لكل (cur = چوٹی_vars; cur; cur = cur->اگلا) {
			لکھو ("%s=%s\n", cur->name, cur->قدر);
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
		لكل (cur = چوٹی_vars; cur; cur = cur->اگلا) {
			إذا(strcmp (cur->name, name) == 0) {
				k = 0;
				لکھو ("%s=%s\n", cur->name, cur->قدر);
			}
			إذا (ctrlc ()) {
				puts ("\n ** Abort\n");
				أرجع 1;
			}
		}
		إذا (k < 0) {
			لکھو ("## Error: \"%s\" not defined\n", name);
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

#جے_انت
/****************************************************************************/
