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
#اگر_تعریف_شدہ __U_BOOT__
#شامل <env.h>
#شامل <malloc.h>         /* malloc, free, realloc*/
#شامل <linux/ctype.h>    /* isalpha, isdigit */
#شامل <console.h>
#شامل <bootretry.h>
#شامل <cli.h>
#شامل <cli_hush.h>
#شامل <command.h>        /* find_cmd */
#شامل <vsprintf.h>
#اگر_ختم
#اگر_غیر_تعریف_شدہ __U_BOOT__
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
#ورنہ
#تعریف_کرو applet_name "hush"
#شامل "standalone.h"
#تعریف_کرو hush_main مین
#تعریف_کرو BB_BANNER
#اگر_ختم
#اگر_ختم
#تعریف_کرو SPECIAL_VAR_علامت 03
#تعریف_کرو SUBSTED_VAR_علامت 04
#اگر_غیر_تعریف_شدہ __U_BOOT__
#تعریف_کرو FLAG_EXIT_FROM_LOOP 1
#تعریف_کرو FLAG_PARSE_SEMICOLON (1 << 1)		/* symbol ';' is special for parser */
#تعریف_کرو FLAG_REPARSING       (1 << 2)		/* >= 2nd pass */

#اگر_ختم

#اگر_تعریف_شدہ __U_BOOT__
#تعریف_کرو EXIT_SUCCESS 0
#تعریف_کرو EOF -1
#تعریف_کرو syntax() syntax_err()
#تعریف_کرو xstrdup strdup
#تعریف_کرو error_پیغام لکھو
#ورنہ
قسمتعریف شمار {
	REDIRECT_داخلہ     = 1,
	REDIRECT_OVERWRITE = 2,
	REDIRECT_APPEND    = 3,
	REDIRECT_HEREIS    = 4,
	REDIRECT_IO        = 5
} redir_type;

/* The descrip member of this structure is only used to make debugging
 * output pretty */
ساخت {صحیح منوال; صحیح default_fd; حرف *descrip;} redir_جدول[] = {
	{ 0,                         0, "()" },
	{ O_RDONLY,                  0, "<"  },
	{ O_CREAT|O_TRUNC|O_WRONLY,  1, ">"  },
	{ O_CREAT|O_APPEND|O_WRONLY, 1, ">>" },
	{ O_RDONLY,                 -1, "<<" },
	{ O_RDWR,                    1, "<>" }
};
#اگر_ختم

قسمتعریف شمار {
	PIPE_SEQ = 1,
	PIPE_AND = 2,
	PIPE_OR  = 3,
	PIPE_BG  = 4,
} pipe_style;

/* might eventually control execution */
قسمتعریف شمار {
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
	RES_مکمل  = 10,
	RES_XXXX  = 11,
	RES_اندر    = 12,
	RES_SNTX  = 13
} reserved_style;
#تعریف_کرو FLAG_انتہا   (1<<RES_NONE)
#تعریف_کرو FLAG_IF    (1<<RES_IF)
#تعریف_کرو FLAG_THEN  (1<<RES_THEN)
#تعریف_کرو FLAG_ELIF  (1<<RES_ELIF)
#تعریف_کرو FLAG_ELSE  (1<<RES_ELSE)
#تعریف_کرو FLAG_FI    (1<<RES_FI)
#تعریف_کرو FLAG_FOR   (1<<RES_FOR)
#تعریف_کرو FLAG_WHILE (1<<RES_WHILE)
#تعریف_کرو FLAG_UNTIL (1<<RES_UNTIL)
#تعریف_کرو FLAG_DO    (1<<RES_DO)
#تعریف_کرو FLAG_مکمل  (1<<RES_مکمل)
#تعریف_کرو FLAG_اندر    (1<<RES_اندر)
#تعریف_کرو FLAG_آغاز (1<<RES_XXXX)

/* This holds pointers to the various results of parsing */
ساخت p_context {
	ساخت بچہ_prog *بچہ;
	ساخت pipe *list_سر;
	ساخت pipe *pipe;
#اگر_غیر_تعریف_شدہ __U_BOOT__
	ساخت redir_struct *pending_redirect;
#اگر_ختم
	reserved_style w;
	صحیح old_flag;				/* for figuring out valid reserved words */
	ساخت p_context *ڈھیر;
	صحیح type;			/* define type of parser : ";$" common or special symbol */
	/* How about quoting status? */
};

#اگر_غیر_تعریف_شدہ __U_BOOT__
ساخت redir_struct {
	redir_type type;			/* type of redirection */
	صحیح fd;						/* file descriptor being redirected */
	صحیح dup;					/* -1, or file descriptor being duplicated */
	ساخت redir_struct *اگلا;	/* pointer to the next redirect in the list */
	glob_t لفظ;				/* *word.gl_pathv is the filename */
};
#اگر_ختم

ساخت بچہ_prog {
#اگر_غیر_تعریف_شدہ __U_BOOT__
	pid_t pid;					/* 0 if exited */
#اگر_ختم
	حرف **argv;				/* program name and arguments */
	/* was quoted when parsed; copy of struct o_string.nonnull field */
	صحیح *argv_nonnull;
#اگر_تعریف_شدہ __U_BOOT__
	صحیح    argc;                            /* number of program arguments */
#اگر_ختم
	ساخت pipe *گروہ;			/* if non-NULL, first in group or subshell */
#اگر_غیر_تعریف_شدہ __U_BOOT__
	صحیح subshell;				/* flag, non-zero if group must be forked */
	ساخت redir_struct *redirects;	/* I/O redirections */
	glob_t glob_نتیجہ;			/* result of parameter globbing */
	صحیح is_stopped;				/* is the program currently running? */
	ساخت pipe *family;		/* pointer back to the child's parent pipe */
#اگر_ختم
	صحیح sp;				/* number of SPECIAL_VAR_SYMBOL */
	صحیح type;
};

ساخت pipe {
#اگر_غیر_تعریف_شدہ __U_BOOT__
	صحیح jobid;					/* job number */
#اگر_ختم
	صحیح عدد_progs;				/* total number of programs in job */
#اگر_غیر_تعریف_شدہ __U_BOOT__
	صحیح running_progs;			/* number of programs running */
	حرف *متن;					/* name of job */
	حرف *cmdbuf;				/* buffer various argv's point into */
	pid_t pgrp;					/* process group ID for the job */
#اگر_ختم
	ساخت بچہ_prog *progs;	/* array of commands in pipe */
	ساخت pipe *اگلا;			/* to track background commands */
#اگر_غیر_تعریف_شدہ __U_BOOT__
	صحیح stopped_progs;			/* number of programs alive, but stopped */
	صحیح کام_context;			/* bitmask defining current context */
#اگر_ختم
	pipe_style followup;		/* PIPE_BG, PIPE_SEQ, PIPE_OR, PIPE_AND */
	reserved_style r_منوال;		/* supports if, for, while, until */
};

#اگر_غیر_تعریف_شدہ __U_BOOT__
ساخت close_me {
	صحیح fd;
	ساخت close_me *اگلا;
};
#اگر_ختم

ساخت variables {
	حرف *name;
	حرف *قدر;
	صحیح flg_export;
	صحیح flg_read_only;
	ساخت variables *اگلا;
};

/* globals, connect us to the outside world
 * the first three support $?, $#, and $1 */
#اگر_غیر_تعریف_شدہ __U_BOOT__
حرف **global_argv;
غیرعلامتی صحیح global_argc;
#اگر_ختم
ساکن غیرعلامتی صحیح آخری_return_رمز;
#اگر_غیر_تعریف_شدہ __U_BOOT__
خارجی حرف **environ; /* This is in <unistd.h>, but protected with __USE_GNU */
#اگر_ختم

/* "globals" within this file */
ساکن uchar *ifs;
ساکن حرف map[256];
#اگر_غیر_تعریف_شدہ __U_BOOT__
ساکن صحیح fake_منوال;
ساکن صحیح interactive;
ساکن ساخت close_me *close_me_سر;
ساکن ثابت حرف *cwd;
ساکن ساخت pipe *کام_list;
ساکن غیرعلامتی صحیح آخری_bg_pid;
ساکن غیرعلامتی صحیح آخری_jobid;
ساکن غیرعلامتی صحیح shell_terminal;
ساکن حرف *PS1;
ساکن حرف *PS2;
ساخت variables shell_ver = { "HUSH_VERSION", "0.01", 1, 1, 0 };
ساخت variables *چوٹی_vars = &shell_ver;
#ورنہ
ساکن صحیح flag_repeat = 0;
ساکن صحیح do_repeat = 0;
ساکن ساخت variables *چوٹی_vars = تہی ;
#اگر_ختم /*__U_BOOT__ */

#تعریف_کرو B_CHUNK (100)
#تعریف_کرو B_NOSPAC 1

قسمتعریف ساخت {
	حرف *ڈیٹا;
	صحیح لمبائی;
	صحیح maxlen;
	صحیح quote;
	صحیح nonnull;
} o_string;
#تعریف_کرو NULL_O_STRING {تہی,0,0,0,0}
/* used for initialization:
	o_string foo = NULL_O_STRING; */

/* I can almost use ordinary FILE *.  Is open_memstream() universally
 * available?  Where is it documented? */
ساخت اندر_str {
	ثابت غیرعلامتی حرف *p;
#اگر_غیر_تعریف_شدہ __U_BOOT__
	حرف peek_buf[2];
#اگر_ختم
	صحیح __promptme;
	صحیح promptmode;
#اگر_غیر_تعریف_شدہ __U_BOOT__
	FILE *file;
#اگر_ختم
	صحیح (*get) (ساخت اندر_str *);
	صحیح (*peek) (ساخت اندر_str *);
};
#تعریف_کرو b_getch(داخلہ) ((داخلہ)->get(داخلہ))
#تعریف_کرو b_peek(داخلہ) ((داخلہ)->peek(داخلہ))

#اگر_غیر_تعریف_شدہ __U_BOOT__
#تعریف_کرو کام_حالت_FORMAT "[%d] %-22s %.40s\n"

ساخت built_اندر_command {
	حرف *cmd;					/* name */
	حرف *descr;				/* description */
	صحیح (*function) (ساخت بچہ_prog *);	/* function ptr */
};
#اگر_ختم

/* define DEBUG_SHELL for debugging output (obviously ;-)) */
#اگر 0
#تعریف_کرو DEBUG_SHELL
#اگر_ختم

/* This should be in utility.c */
#اگر_تعریف_شدہ DEBUG_SHELL
#اگر_غیر_تعریف_شدہ __U_BOOT__
ساکن خالی debug_printf(ثابت حرف *format, ...)
{
	va_list دلائل;
	va_آغاز(دلائل, format);
	vfprintf(stderr, format, دلائل);
	va_انتہا(دلائل);
}
#ورنہ
#تعریف_کرو debug_printf(fmt,دلائل...)	لکھو (fmt ,##args)
#اگر_ختم
#ورنہ
ساکن اندرونی خالی debug_printf(ثابت حرف *format, ...) { }
#اگر_ختم
#تعریف_کرو final_printf debug_printf

#اگر_تعریف_شدہ __U_BOOT__
ساکن خالی syntax_err(خالی) {
	 لکھو("syntax error\n");
}
#ورنہ
ساکن خالی __syntax(حرف *file, صحیح سطر) {
	error_پیغام("syntax error %s:%d", file, سطر);
}
#تعریف_کرو syntax() __syntax(__FILE__, __LINE__)
#اگر_ختم

#اگر_تعریف_شدہ __U_BOOT__
ساکن خالی *xmalloc(size_t سائز);
ساکن خالی *xrealloc(خالی *ptr, size_t سائز);
#ورنہ
/* Index of subroutines: */
/*   function prototypes for builtins */
ساکن صحیح builtin_cd(ساخت بچہ_prog *بچہ);
ساکن صحیح builtin_env(ساخت بچہ_prog *بچہ);
ساکن صحیح builtin_eval(ساخت بچہ_prog *بچہ);
ساکن صحیح builtin_exec(ساخت بچہ_prog *بچہ);
ساکن صحیح builtin_exit(ساخت بچہ_prog *بچہ);
ساکن صحیح builtin_export(ساخت بچہ_prog *بچہ);
ساکن صحیح builtin_fg_bg(ساخت بچہ_prog *بچہ);
ساکن صحیح builtin_help(ساخت بچہ_prog *بچہ);
ساکن صحیح builtin_jobs(ساخت بچہ_prog *بچہ);
ساکن صحیح builtin_pwd(ساخت بچہ_prog *بچہ);
ساکن صحیح builtin_read(ساخت بچہ_prog *بچہ);
ساکن صحیح builtin_سیٹ(ساخت بچہ_prog *بچہ);
ساکن صحیح builtin_سرکاؤ(ساخت بچہ_prog *بچہ);
ساکن صحیح builtin_ماخذ(ساخت بچہ_prog *بچہ);
ساکن صحیح builtin_umask(ساخت بچہ_prog *بچہ);
ساکن صحیح builtin_unset(ساخت بچہ_prog *بچہ);
ساکن صحیح builtin_not_written(ساخت بچہ_prog *بچہ);
#اگر_ختم
/*   o_string manipulation: */
ساکن صحیح b_check_space(o_string *o, صحیح len);
ساکن صحیح b_addchr(o_string *o, صحیح ch);
ساکن خالی b_reset(o_string *o);
ساکن صحیح b_addqchr(o_string *o, صحیح ch, صحیح quote);
#اگر_غیر_تعریف_شدہ __U_BOOT__
ساکن صحیح b_adduint(o_string *o, غیرعلامتی صحیح i);
#اگر_ختم
/*  in_str manipulations: */
ساکن صحیح static_get(ساخت اندر_str *i);
ساکن صحیح static_peek(ساخت اندر_str *i);
ساکن صحیح file_get(ساخت اندر_str *i);
ساکن صحیح file_peek(ساخت اندر_str *i);
#اگر_غیر_تعریف_شدہ __U_BOOT__
ساکن خالی تیاری_file_اندر_str(ساخت اندر_str *i, FILE *f);
#ورنہ
ساکن خالی تیاری_file_اندر_str(ساخت اندر_str *i);
#اگر_ختم
ساکن خالی تیاری_string_اندر_str(ساخت اندر_str *i, ثابت حرف *s);
#اگر_غیر_تعریف_شدہ __U_BOOT__
/*  close_me manipulations: */
ساکن خالی mark_open(صحیح fd);
ساکن خالی mark_closed(صحیح fd);
ساکن خالی close_all(خالی);
#اگر_ختم
/*  "run" the final data structures: */
ساکن حرف *indenter(صحیح i);
ساکن صحیح free_pipe_list(ساخت pipe *سر, صحیح indent);
ساکن صحیح free_pipe(ساخت pipe *pi, صحیح indent);
/*  really run the final data structures: */
#اگر_غیر_تعریف_شدہ __U_BOOT__
ساکن صحیح تیاری_redirects(ساخت بچہ_prog *prog, صحیح squirrel[]);
#اگر_ختم
ساکن صحیح تسلسل_list_real(ساخت pipe *pi);
#اگر_غیر_تعریف_شدہ __U_BOOT__
ساکن خالی pseudo_exec(ساخت بچہ_prog *بچہ) __attribute__ ((noreturn));
#اگر_ختم
ساکن صحیح تسلسل_pipe_real(ساخت pipe *pi);
/*   extended glob support: */
#اگر_غیر_تعریف_شدہ __U_BOOT__
ساکن صحیح globhack(ثابت حرف *src, صحیح flags, glob_t *pglob);
ساکن صحیح glob_needed(ثابت حرف *s);
ساکن صحیح xglob(o_string *dest, صحیح flags, glob_t *pglob);
#اگر_ختم
/*   variable assignment: */
ساکن صحیح is_assignment(ثابت حرف *s);
/*   data structure manipulation: */
#اگر_غیر_تعریف_شدہ __U_BOOT__
ساکن صحیح تیاری_redirect(ساخت p_context *ctx, صحیح fd, redir_type style, ساخت اندر_str *داخلہ);
#اگر_ختم
ساکن خالی initialize_context(ساخت p_context *ctx);
ساکن صحیح مکمل_لفظ(o_string *dest, ساخت p_context *ctx);
ساکن صحیح مکمل_command(ساخت p_context *ctx);
ساکن صحیح مکمل_pipe(ساخت p_context *ctx, pipe_style type);
/*   primary string parsing: */
#اگر_غیر_تعریف_شدہ __U_BOOT__
ساکن صحیح redirect_dup_عدد(ساخت اندر_str *داخلہ);
ساکن صحیح redirect_opt_عدد(o_string *o);
ساکن صحیح process_command_subs(o_string *dest, ساخت p_context *ctx, ساخت اندر_str *داخلہ, صحیح subst_انتہا);
ساکن صحیح parse_گروہ(o_string *dest, ساخت p_context *ctx, ساخت اندر_str *داخلہ, صحیح ch);
#اگر_ختم
ساکن حرف *lookup_param(حرف *src);
ساکن حرف *make_string(حرف **inp, صحیح *nonnull);
ساکن صحیح handle_dollar(o_string *dest, ساخت p_context *ctx, ساخت اندر_str *داخلہ);
#اگر_غیر_تعریف_شدہ __U_BOOT__
ساکن صحیح parse_string(o_string *dest, ساخت p_context *ctx, ثابت حرف *src);
#اگر_ختم
ساکن صحیح parse_دھارا(o_string *dest, ساخت p_context *ctx, ساخت اندر_str *input0, صحیح انتہا_trigger);
/*   setup: */
ساکن صحیح parse_دھارا_outer(ساخت اندر_str *inp, صحیح flag);
#اگر_غیر_تعریف_شدہ __U_BOOT__
ساکن صحیح parse_string_outer(ثابت حرف *s, صحیح flag);
ساکن صحیح parse_file_outer(FILE *f);
#اگر_ختم
#اگر_غیر_تعریف_شدہ __U_BOOT__
/*   job management: */
ساکن صحیح checkjobs(ساخت pipe* fg_pipe);
ساکن خالی داخل_کرو_bg_کام(ساخت pipe *pi);
ساکن خالی remove_bg_کام(ساخت pipe *pi);
#اگر_ختم
/*     local variable support */
ساکن حرف **make_list_اندر(حرف **inp, حرف *name);
ساکن حرف *داخل_کرو_var_قدر(حرف *inp);
ساکن حرف *داخل_کرو_var_قدر_sub(حرف *inp, صحیح لیبل_subst);

#اگر_غیر_تعریف_شدہ __U_BOOT__
/* Table of built-in functions.  They can be forked or not, depending on
 * context: within pipes, they fork.  As simple commands, they do not.
 * When used in non-forking context, they can change global variables
 * in the parent shell process.  If forked, of course they can not.
 * For example, 'unset foo | whatever' will parse and run, but foo will
 * still be set at the end. */
ساکن ساخت built_اندر_command bltins[] = {
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
	{"shift", "Shift positional parameters", builtin_سرکاؤ},
	{"trap", "Trap signals", builtin_not_written},
	{"ulimit","Controls resource limits", builtin_not_written},
	{"umask","Sets file creation mask", builtin_umask},
	{"unset", "Unset environment variable", builtin_unset},
	{".", "Source-in and run commands in a file", builtin_ماخذ},
	{"help", "List shell built-in commands", builtin_help},
	{تہی, تہی, تہی}
};

ساکن ثابت حرف *سیٹ_cwd(خالی)
{
	اگر(cwd==unknown)
		cwd = تہی;     /* xgetcwd(arg) called free(arg) */
	cwd = xgetcwd((حرف *)cwd);
	اگر (!cwd)
		cwd = unknown;
	واپس cwd;
}

/* built-in 'eval' handler */
ساکن صحیح builtin_eval(ساخت بچہ_prog *بچہ)
{
	حرف *str = تہی;
	صحیح rcode = EXIT_SUCCESS;

	اگر (بچہ->argv[1]) {
		str = make_string(بچہ->argv + 1);
		parse_string_outer(str, FLAG_EXIT_FROM_LOOP |
					FLAG_PARSE_SEMICOLON);
		آزاد(str);
		rcode = آخری_return_رمز;
	}
	واپس rcode;
}

/* built-in 'cd <path>' handler */
ساکن صحیح builtin_cd(ساخت بچہ_prog *بچہ)
{
	حرف *newdir;
	اگر (بچہ->argv[1] == تہی)
		newdir = env_get("HOME");
	ورنہ
		newdir = بچہ->argv[1];
	اگر (chdir(newdir)) {
		لکھو("cd: %s: %s\n", newdir, strerror(errno));
		واپس EXIT_FAILURE;
	}
	سیٹ_cwd();
	واپس EXIT_SUCCESS;
}

/* built-in 'env' handler */
ساکن صحیح builtin_env(ساخت بچہ_prog *dummy)
{
	حرف **e = environ;
	اگر (e == تہی) واپس EXIT_FAILURE;
	برائے (; *e; e++) {
		puts(*e);
	}
	واپس EXIT_SUCCESS;
}

/* built-in 'exec' handler */
ساکن صحیح builtin_exec(ساخت بچہ_prog *بچہ)
{
	اگر (بچہ->argv[1] == تہی)
		واپس EXIT_SUCCESS;   /* Really? */
	بچہ->argv++;
	pseudo_exec(بچہ);
	/* never returns */
}

/* built-in 'exit' handler */
ساکن صحیح builtin_exit(ساخت بچہ_prog *بچہ)
{
	اگر (بچہ->argv[1] == تہی)
		باہر_نکلو(آخری_return_رمز);
	باہر_نکلو (atoi(بچہ->argv[1]));
}

/* built-in 'export VAR=value' handler */
ساکن صحیح builtin_export(ساخت بچہ_prog *بچہ)
{
	صحیح res = 0;
	حرف *name = بچہ->argv[1];

	اگر (name == تہی) {
		واپس (builtin_env(بچہ));
	}

	name = strdup(name);

	اگر(name) {
		حرف *قدر = strchr(name, '=');

		اگر (!قدر) {
			حرف *tmp;
			/* They are exporting something without an =VALUE */

			قدر = get_local_var(name);
			اگر (قدر) {
				size_t ln = لڑی_لمبائی(name);

				tmp = realloc(name, ln+لڑی_لمبائی(قدر)+2);
				اگر(tmp==تہی)
					res = -1;
				ورنہ {
					sprintf(tmp+ln, "=%s", قدر);
					name = tmp;
				}
			} ورنہ {
				/* bash does not return an error when trying to export
				 * an undefined variable.  Do likewise. */
				res = 1;
			}
		}
	}
	اگر (res<0)
		perror_پیغام("export");
	ورنہ اگر(res==0)
		res = سیٹ_local_var(name, 1);
	ورنہ
		res = 0;
	آزاد(name);
	واپس res;
}

/* built-in 'fg' and 'bg' handler */
ساکن صحیح builtin_fg_bg(ساخت بچہ_prog *بچہ)
{
	صحیح i, jobnum;
	ساخت pipe *pi=تہی;

	اگر (!interactive)
		واپس EXIT_FAILURE;
	/* If they gave us no args, assume they want the last backgrounded task */
	اگر (!بچہ->argv[1]) {
		برائے (pi = کام_list; pi; pi = pi->اگلا) {
			اگر (pi->jobid == آخری_jobid) {
				توڑو;
			}
		}
		اگر (!pi) {
			error_پیغام("%s: no current job", بچہ->argv[0]);
			واپس EXIT_FAILURE;
		}
	} ورنہ {
		اگر (لڑی_سے_پڑھو(بچہ->argv[1], "%%%d", &jobnum) != 1) {
			error_پیغام("%s: bad argument '%s'", بچہ->argv[0], بچہ->argv[1]);
			واپس EXIT_FAILURE;
		}
		برائے (pi = کام_list; pi; pi = pi->اگلا) {
			اگر (pi->jobid == jobnum) {
				توڑو;
			}
		}
		اگر (!pi) {
			error_پیغام("%s: %d: no such job", بچہ->argv[0], jobnum);
			واپس EXIT_FAILURE;
		}
	}

	اگر (*بچہ->argv[0] == 'f') {
		/* Put the job into the foreground.  */
		tcsetpgrp(shell_terminal, pi->pgrp);
	}

	/* Restart the processes in the job */
	برائے (i = 0; i < pi->عدد_progs; i++)
		pi->progs[i].is_stopped = 0;

	اگر ( (i=kill(- pi->pgrp, SIGCONT)) < 0) {
		اگر (i == ESRCH) {
			remove_bg_کام(pi);
		} ورنہ {
			perror_پیغام("kill (SIGCONT)");
		}
	}

	pi->stopped_progs = 0;
	واپس EXIT_SUCCESS;
}

/* built-in 'help' handler */
ساکن صحیح builtin_help(ساخت بچہ_prog *dummy)
{
	ساخت built_اندر_command *x;

	لکھو("\nBuilt-in commands:\n");
	لکھو("-------------------\n");
	برائے (x = bltins; x->cmd; x++) {
		اگر (x->descr==تہی)
			جاری;
		لکھو("%s\t%s\n", x->cmd, x->descr);
	}
	لکھو("\n\n");
	واپس EXIT_SUCCESS;
}

/* built-in 'jobs' handler */
ساکن صحیح builtin_jobs(ساخت بچہ_prog *بچہ)
{
	ساخت pipe *کام;
	حرف *حالت_string;

	برائے (کام = کام_list; کام; کام = کام->اگلا) {
		اگر (کام->running_progs == کام->stopped_progs)
			حالت_string = "Stopped";
		ورنہ
			حالت_string = "Running";

		لکھو(کام_حالت_FORMAT, کام->jobid, حالت_string, کام->متن);
	}
	واپس EXIT_SUCCESS;
}

/* built-in 'pwd' handler */
ساکن صحیح builtin_pwd(ساخت بچہ_prog *dummy)
{
	puts(سیٹ_cwd());
	واپس EXIT_SUCCESS;
}

/* built-in 'read VAR' handler */
ساکن صحیح builtin_read(ساخت بچہ_prog *بچہ)
{
	صحیح res;

	اگر (بچہ->argv[1]) {
		حرف string[BUFSIZ];
		حرف *var = 0;

		string[0] = 0;  /* In case stdin has only EOF */
		/* read string */
		fgets(string, حجم(string), stdin);
		chomp(string);
		var = حافظہ_دو(لڑی_لمبائی(بچہ->argv[1])+لڑی_لمبائی(string)+2);
		اگر(var) {
			sprintf(var, "%s=%s", بچہ->argv[1], string);
			res = سیٹ_local_var(var, 0);
		} ورنہ
			res = -1;
		اگر (res)
			fprintf(stderr, "read: %m\n");
		آزاد(var);      /* So not move up to avoid breaking errno */
		واپس res;
	} ورنہ {
		کرو res=getchar(); جب(res!='\n' && res!=EOF);
		واپس 0;
	}
}

/* built-in 'set VAR=value' handler */
ساکن صحیح builtin_سیٹ(ساخت بچہ_prog *بچہ)
{
	حرف *عارضی = بچہ->argv[1];
	ساخت variables *e;

	اگر (عارضی == تہی)
		برائے(e = چوٹی_vars; e; e=e->اگلا)
			لکھو("%s=%s\n", e->name, e->قدر);
	ورنہ
		سیٹ_local_var(عارضی, 0);

		واپس EXIT_SUCCESS;
}

/* Built-in 'shift' handler */
ساکن صحیح builtin_سرکاؤ(ساخت بچہ_prog *بچہ)
{
	صحیح n=1;
	اگر (بچہ->argv[1]) {
		n=atoi(بچہ->argv[1]);
	}
	اگر (n>=0 && n<global_argc) {
		/* XXX This probably breaks $0 */
		global_argc -= n;
		global_argv += n;
		واپس EXIT_SUCCESS;
	} ورنہ {
		واپس EXIT_FAILURE;
	}
}

/* Built-in '.' handler (read-in and execute commands from file) */
ساکن صحیح builtin_ماخذ(ساخت بچہ_prog *بچہ)
{
	FILE *داخلہ;
	صحیح حالت_2;

	اگر (بچہ->argv[1] == تہی)
		واپس EXIT_FAILURE;

	/* XXX search through $PATH is missing */
	داخلہ = fopen(بچہ->argv[1], "r");
	اگر (!داخلہ) {
		error_پیغام("Couldn't open file '%s'", بچہ->argv[1]);
		واپس EXIT_FAILURE;
	}

	/* Now run the file */
	/* XXX argv and argc are broken; need to save old global_argv
	 * (pointer only is OK!) on this stack frame,
	 * set global_argv=child->argv+1, recurse, and restore. */
	mark_open(fileno(داخلہ));
	حالت_2 = parse_file_outer(داخلہ);
	mark_closed(fileno(داخلہ));
	fclose(داخلہ);
	واپس (حالت_2);
}

ساکن صحیح builtin_umask(ساخت بچہ_prog *بچہ)
{
	mode_t new_umask;
	ثابت حرف *arg = بچہ->argv[1];
	حرف *انتہا_2;
	اگر (arg) {
		new_umask=strtoul(arg, &انتہا_2, 8);
		اگر (*انتہا_2!='\0' || انتہا_2 == arg) {
			واپس EXIT_FAILURE;
		}
	} ورنہ {
		لکھو("%.3o\n", (غیرعلامتی صحیح) (new_umask=umask(0)));
	}
	umask(new_umask);
	واپس EXIT_SUCCESS;
}

/* built-in 'unset VAR' handler */
ساکن صحیح builtin_unset(ساخت بچہ_prog *بچہ)
{
	/* bash returned already true */
	unset_local_var(بچہ->argv[1]);
	واپس EXIT_SUCCESS;
}

ساکن صحیح builtin_not_written(ساخت بچہ_prog *بچہ)
{
	لکھو("builtin_%s not written\n",بچہ->argv[0]);
	واپس EXIT_FAILURE;
}
#اگر_ختم

ساکن صحیح b_check_space(o_string *o, صحیح len)
{
	/* It would be easy to drop a more restrictive policy
	 * in here, such as setting a maximum string length */
	اگر (o->لمبائی + len > o->maxlen) {
		حرف *old_ڈیٹا = o->ڈیٹا;
		/* assert (data == NULL || o->maxlen != 0); */
		o->maxlen += max(2*len, B_CHUNK);
		o->ڈیٹا = realloc(o->ڈیٹا, 1 + o->maxlen);
		اگر (o->ڈیٹا == تہی) {
			آزاد(old_ڈیٹا);
		}
	}
	واپس o->ڈیٹا == تہی;
}

ساکن صحیح b_addchr(o_string *o, صحیح ch)
{
	debug_printf("b_addchr: %c %d %p\n", ch, o->لمبائی, o);
	اگر (b_check_space(o, 1)) واپس B_NOSPAC;
	o->ڈیٹا[o->لمبائی] = ch;
	o->لمبائی++;
	o->ڈیٹا[o->لمبائی] = '\0';
	واپس 0;
}

ساکن خالی b_reset(o_string *o)
{
	o->لمبائی = 0;
	o->nonnull = 0;
	اگر (o->ڈیٹا != تہی) *o->ڈیٹا = '\0';
}

ساکن خالی b_free(o_string *o)
{
	b_reset(o);
	آزاد(o->ڈیٹا);
	o->ڈیٹا = تہی;
	o->maxlen = 0;
}

/* My analysis of quoting semantics tells me that state information
 * is associated with a destination, not a source.
 */
ساکن صحیح b_addqchr(o_string *o, صحیح ch, صحیح quote)
{
	اگر (quote && strchr("*?[\\",ch)) {
		صحیح rc;
		rc = b_addchr(o, '\\');
		اگر (rc) واپس rc;
	}
	واپس b_addchr(o, ch);
}

#اگر_غیر_تعریف_شدہ __U_BOOT__
ساکن صحیح b_adduint(o_string *o, غیرعلامتی صحیح i)
{
	صحیح r;
	حرف *p = simple_itoa(i);
	/* no escape checking necessary */
	کرو r=b_addchr(o, *p++); جب (r==0 && *p);
	واپس r;
}
#اگر_ختم

ساکن صحیح static_get(ساخت اندر_str *i)
{
	صحیح ch = *i->p++;
	اگر (ch=='\0') واپس EOF;
	واپس ch;
}

ساکن صحیح static_peek(ساخت اندر_str *i)
{
	واپس *i->p;
}

#اگر_غیر_تعریف_شدہ __U_BOOT__
ساکن اندرونی خالی cmdedit_سیٹ_initial_prompt(خالی)
{
#اگر_غیر_تعریف_شدہ CONFIG_FEATURE_SH_FANCY_PROMPT
	PS1 = تہی;
#ورنہ
	PS1 = env_get("PS1");
	اگر(PS1==0)
		PS1 = "\\w \\$ ";
#اگر_ختم
}

ساکن اندرونی خالی تیاری_prompt_string(صحیح promptmode, حرف **prompt_str)
{
	debug_printf("setup_prompt_string %d ",promptmode);
#اگر_غیر_تعریف_شدہ CONFIG_FEATURE_SH_FANCY_PROMPT
	/* Set up the prompt */
	اگر (promptmode == 1) {
		آزاد(PS1);
		PS1=xmalloc(لڑی_لمبائی(cwd)+4);
		sprintf(PS1, "%s %s", cwd, ( geteuid() != 0 ) ?  "$ ":"# ");
		*prompt_str = PS1;
	} ورنہ {
		*prompt_str = PS2;
	}
#ورنہ
	*prompt_str = (promptmode==1)? PS1 : PS2;
#اگر_ختم
	debug_printf("result %s\n",*prompt_str);
}
#اگر_ختم

#اگر_تعریف_شدہ __U_BOOT__
ساکن صحیح uboot_cli_readline(ساخت اندر_str *i)
{
	حرف *prompt;
	حرف __maybe_unused *ps_prompt = تہی;

	اگر (i->promptmode == 1)
		prompt = CONFIG_SYS_PROMPT;
	ورنہ
		prompt = CONFIG_SYS_PROMPT_HUSH_PS2;

#اگر_تعریف_شدہ CONFIG_CMDLINE_PS_SUPPORT
	اگر (i->promptmode == 1)
		ps_prompt = env_get("PS1");
	ورنہ
		ps_prompt = env_get("PS2");
	اگر (ps_prompt)
		prompt = ps_prompt;
#اگر_ختم

	واپس cli_readline(prompt);
}
#اگر_ختم

ساکن خالی get_user_داخلہ(ساخت اندر_str *i)
{
#اگر_غیر_تعریف_شدہ __U_BOOT__
	حرف *prompt_str;
	ساکن حرف the_command[BUFSIZ];

	تیاری_prompt_string(i->promptmode, &prompt_str);
#اگر_تعریف_شدہ CONFIG_FEATURE_COMMAND_EDITING
	/*
	 ** enable command line editing only while a command line
	 ** is actually being read; otherwise, we'll end up bequeathing
	 ** atexit() handlers and other unwanted stuff to our
	 ** child processes (rob@sysgo.de)
	 */
	cmdedit_read_داخلہ(prompt_str, the_command);
#ورنہ
	fputs(prompt_str, stdout);
	fflush(stdout);
	the_command[0]=fgetc(i->file);
	the_command[1]='\0';
#اگر_ختم
	fflush(stdout);
	i->p = the_command;
#ورنہ
	صحیح n;
	ساکن حرف the_command[CONFIG_SYS_CBSIZE + 1];

	bootretry_reset_cmd_timeout();
	i->__promptme = 1;
	n = uboot_cli_readline(i);

#اگر_تعریف_شدہ CONFIG_BOOT_RETRY_وقت
	اگر (n == -2) {
	  puts("\nTimeout waiting for command\n");
#  ifdef CONFIG_RESET_مقصد_RETRY
	  do_reset(تہی, 0, 0, تہی);
#  elif IS_ENABLED(CONFIG_RETRY_BOOTCMD)
	strcpy(console_بفر, "run bootcmd\n");
# ورنہ
#	error "This only works with CONFIG_RESET_TO_RETRY or CONFIG_BOOT_RETRY_COMMAND enabled"
#  endif
	}
#اگر_ختم
	اگر (n == -1 ) {
		flag_repeat = 0;
		i->__promptme = 0;
	}
	n = لڑی_لمبائی(console_بفر);
	console_بفر[n] = '\n';
	console_بفر[n+1]= '\0';
	اگر (had_ctrlc()) flag_repeat = 0;
	clear_ctrlc();
	do_repeat = 0;
	اگر (i->promptmode == 1) {
		اگر (console_بفر[0] == '\n'&& flag_repeat == 0) {
			strcpy(the_command,console_بفر);
		}
		ورنہ {
			اگر (console_بفر[0] != '\n') {
				strcpy(the_command,console_بفر);
				flag_repeat = 1;
			}
			ورنہ {
				do_repeat = 1;
			}
		}
		i->p = the_command;
	}
	ورنہ {
		اگر (console_بفر[0] != '\n') {
			اگر (لڑی_لمبائی(the_command) + لڑی_لمبائی(console_بفر)
			    < CONFIG_SYS_CBSIZE) {
				n = لڑی_لمبائی(the_command);
				the_command[n-1] = ' ';
				strcpy(&the_command[n],console_بفر);
			}
			ورنہ {
				the_command[0] = '\n';
				the_command[1] = '\0';
				flag_repeat = 0;
			}
		}
		اگر (i->__promptme == 0) {
			the_command[0] = '\n';
			the_command[1] = '\0';
		}
		i->p = console_بفر;
	}
#اگر_ختم
}

/* This is the magic location that prints prompts
 * and gets data back from the user */
ساکن صحیح file_get(ساخت اندر_str *i)
{
	صحیح ch;

	ch = 0;
	/* If there is data waiting, eat it up */
	اگر (i->p && *i->p) {
		ch = *i->p++;
	} ورنہ {
		/* need to double check i->file because we might be doing something
		 * more complicated by now, like sourcing or substituting. */
#اگر_غیر_تعریف_شدہ __U_BOOT__
		اگر (i->__promptme && interactive && i->file == stdin) {
			جب(! i->p || (interactive && لڑی_لمبائی(i->p)==0) ) {
#ورنہ
			جب(! i->p  || لڑی_لمبائی(i->p)==0 ) {
#اگر_ختم
				get_user_داخلہ(i);
			}
			i->promptmode=2;
#اگر_غیر_تعریف_شدہ __U_BOOT__
			i->__promptme = 0;
#اگر_ختم
			اگر (i->p && *i->p) {
				ch = *i->p++;
			}
#اگر_غیر_تعریف_شدہ __U_BOOT__
		} ورنہ {
			ch = fgetc(i->file);
		}

#اگر_ختم
		debug_printf("b_getch: got a %d\n", ch);
	}
#اگر_غیر_تعریف_شدہ __U_BOOT__
	اگر (ch == '\n') i->__promptme=1;
#اگر_ختم
	واپس ch;
}

/* All the callers guarantee this routine will never be
 * used right after a newline, so prompting is not needed.
 */
ساکن صحیح file_peek(ساخت اندر_str *i)
{
#اگر_غیر_تعریف_شدہ __U_BOOT__
	اگر (i->p && *i->p) {
#اگر_ختم
		واپس *i->p;
#اگر_غیر_تعریف_شدہ __U_BOOT__
	} ورنہ {
		i->peek_buf[0] = fgetc(i->file);
		i->peek_buf[1] = '\0';
		i->p = i->peek_buf;
		debug_printf("b_peek: got a %d\n", *i->p);
		واپس *i->p;
	}
#اگر_ختم
}

#اگر_غیر_تعریف_شدہ __U_BOOT__
ساکن خالی تیاری_file_اندر_str(ساخت اندر_str *i, FILE *f)
#ورنہ
ساکن خالی تیاری_file_اندر_str(ساخت اندر_str *i)
#اگر_ختم
{
	i->peek = file_peek;
	i->get = file_get;
	i->__promptme=1;
	i->promptmode=1;
#اگر_غیر_تعریف_شدہ __U_BOOT__
	i->file = f;
#اگر_ختم
	i->p = تہی;
}

ساکن خالی تیاری_string_اندر_str(ساخت اندر_str *i, ثابت حرف *s)
{
	i->peek = static_peek;
	i->get = static_get;
	i->__promptme=1;
	i->promptmode=1;
	i->p = s;
}

#اگر_غیر_تعریف_شدہ __U_BOOT__
ساکن خالی mark_open(صحیح fd)
{
	ساخت close_me *new = xmalloc(حجم(ساخت close_me));
	new->fd = fd;
	new->اگلا = close_me_سر;
	close_me_سر = new;
}

ساکن خالی mark_closed(صحیح fd)
{
	ساخت close_me *tmp;
	اگر (close_me_سر == تہی || close_me_سر->fd != fd)
		error_پیغام_and_die("corrupt close_me");
	tmp = close_me_سر;
	close_me_سر = close_me_سر->اگلا;
	آزاد(tmp);
}

ساکن خالی close_all(خالی)
{
	ساخت close_me *c;
	برائے (c=close_me_سر; c; c=c->اگلا) {
		close(c->fd);
	}
	close_me_سر = تہی;
}

/* squirrel != NULL means we squirrel away copies of stdin, stdout,
 * and stderr if they are redirected. */
ساکن صحیح تیاری_redirects(ساخت بچہ_prog *prog, صحیح squirrel[])
{
	صحیح openfd, منوال;
	ساخت redir_struct *redir;

	برائے (redir=prog->redirects; redir; redir=redir->اگلا) {
		اگر (redir->dup == -1 && redir->لفظ.gl_pathv == تہی) {
			/* something went wrong in the parse.  Pretend it didn't happen */
			جاری;
		}
		اگر (redir->dup == -1) {
			منوال=redir_جدول[redir->type].منوال;
			openfd = open(redir->لفظ.gl_pathv[0], منوال, 0666);
			اگر (openfd < 0) {
			/* this could get lost if stderr has been redirected, but
			   bash and ash both lose it as well (though zsh doesn't!) */
				perror_پیغام("error opening %s", redir->لفظ.gl_pathv[0]);
				واپس 1;
			}
		} ورنہ {
			openfd = redir->dup;
		}

		اگر (openfd != redir->fd) {
			اگر (squirrel && redir->fd < 3) {
				squirrel[redir->fd] = dup(redir->fd);
			}
			اگر (openfd == -3) {
				close(openfd);
			} ورنہ {
				dup2(openfd, redir->fd);
				اگر (redir->dup == -1)
					close (openfd);
			}
		}
	}
	واپس 0;
}

ساکن خالی restore_redirects(صحیح squirrel[])
{
	صحیح i, fd;
	برائے (i=0; i<3; i++) {
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
ساکن خالی pseudo_exec(ساخت بچہ_prog *بچہ)
{
	صحیح i, rcode;
	حرف *p;
	ساخت built_اندر_command *x;
	اگر (بچہ->argv) {
		برائے (i=0; is_assignment(بچہ->argv[i]); i++) {
			debug_printf("pid %d environment modification: %s\n",getpid(),بچہ->argv[i]);
			p = داخل_کرو_var_قدر(بچہ->argv[i]);
			putenv(strdup(p));
			اگر (p != بچہ->argv[i]) آزاد(p);
		}
		بچہ->argv+=i;  /* XXX this hack isn't so horrible, since we are about
					to exit, and therefore don't need to keep data
					structures consistent for free() use. */
		/* If a variable is assigned in a forest, and nobody listens,
		 * was it ever really set?
		 */
		اگر (بچہ->argv[0] == تہی) {
			_exit(EXIT_SUCCESS);
		}

		/*
		 * Check if the command matches any of the builtins.
		 * Depending on context, this might be redundant.  But it's
		 * easier to waste a few CPU cycles than it is to figure out
		 * if this is one of those cases.
		 */
		برائے (x = bltins; x->cmd; x++) {
			اگر (strcmp(بچہ->argv[0], x->cmd) == 0 ) {
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
#اگر_تعریف_شدہ CONFIG_FEATURE_SH_STANDALONE_SHELL
		{
			صحیح argc_l;
			حرف** argv_l=بچہ->argv;
			حرف *name = بچہ->argv[0];

#اگر_تعریف_شدہ CONFIG_FEATURE_SH_APPLETS_ALWAYS_WIN
			/* Following discussions from November 2000 on the busybox mailing
			 * list, the default configuration, (without
			 * get_last_path_component()) lets the user force use of an
			 * external command by specifying the full (with slashes) filename.
			 * If you enable CONFIG_FEATURE_SH_APPLETS_ALWAYS_WIN then applets
			 * _aways_ override external commands, so if you want to run
			 * /bin/cat, it will use BusyBox cat even if /bin/cat exists on the
			 * filesystem and is _not_ busybox.  Some systems may want this,
			 * most do not.  */
			name = get_آخری_راستہ_component(name);
#اگر_ختم
			/* Count argc for use in a second... */
			برائے(argc_l=0;*argv_l!=تہی; argv_l++, argc_l++);
			optind = 1;
			debug_printf("running applet %s\n", name);
			تسلسل_applet_by_name(name, argc_l, بچہ->argv);
		}
#اگر_ختم
		debug_printf("exec of %s\n",بچہ->argv[0]);
		execvp(بچہ->argv[0],بچہ->argv);
		perror_پیغام("couldn't exec: %s",بچہ->argv[0]);
		_exit(1);
	} ورنہ اگر (بچہ->گروہ) {
		debug_printf("runtime nesting to group\n");
		interactive=0;    /* crucial!!!! */
		rcode = تسلسل_list_real(بچہ->گروہ);
		/* OK to leak memory by not calling free_pipe_list,
		 * since this process is about to exit */
		_exit(rcode);
	} ورنہ {
		/* Can happen.  See what bash does with ">foo" by itself. */
		debug_printf("trying to pseudo_exec null command\n");
		_exit(EXIT_SUCCESS);
	}
}

ساکن خالی داخل_کرو_bg_کام(ساخت pipe *pi)
{
	ساخت pipe *thejob;

	/* Linear search for the ID of the job to use */
	pi->jobid = 1;
	برائے (thejob = کام_list; thejob; thejob = thejob->اگلا)
		اگر (thejob->jobid >= pi->jobid)
			pi->jobid = thejob->jobid + 1;

	/* add thejob to the list of running jobs */
	اگر (!کام_list) {
		thejob = کام_list = xmalloc(حجم(*thejob));
	} ورنہ {
		برائے (thejob = کام_list; thejob->اگلا; thejob = thejob->اگلا) /* nothing */;
		thejob->اگلا = xmalloc(حجم(*thejob));
		thejob = thejob->اگلا;
	}

	/* physically copy the struct job */
	حافظہ_نقل(thejob, pi, حجم(ساخت pipe));
	thejob->اگلا = تہی;
	thejob->running_progs = thejob->عدد_progs;
	thejob->stopped_progs = 0;
	thejob->متن = xmalloc(BUFSIZ); /* cmdedit buffer size */

	/*if (pi->progs[0] && pi->progs[0].argv && pi->progs[0].argv[0]) */
	{
		حرف *bar=thejob->متن;
		حرف **foo=pi->progs[0].argv;
		جب(foo && *foo) {
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
ساکن خالی remove_bg_کام(ساخت pipe *pi)
{
	ساخت pipe *پچھلا_pipe;

	اگر (pi == کام_list) {
		کام_list = pi->اگلا;
	} ورنہ {
		پچھلا_pipe = کام_list;
		جب (پچھلا_pipe->اگلا != pi)
			پچھلا_pipe = پچھلا_pipe->اگلا;
		پچھلا_pipe->اگلا = pi->اگلا;
	}
	اگر (کام_list)
		آخری_jobid = کام_list->jobid;
	ورنہ
		آخری_jobid = 0;

	pi->stopped_progs = 0;
	free_pipe(pi, 0);
	آزاد(pi);
}

/* Checks to see if any processes have exited -- if they
   have, figure out why and see if a job has completed */
ساکن صحیح checkjobs(ساخت pipe* fg_pipe)
{
	صحیح اوصاف;
	صحیح حالت_2;
	صحیح prognum = 0;
	ساخت pipe *pi;
	pid_t childpid;

	اوصاف = WUNTRACED;
	اگر (fg_pipe==تہی) {
		اوصاف |= WNOHANG;
	}

	جب ((childpid = waitpid(-1, &حالت_2, اوصاف)) > 0) {
		اگر (fg_pipe) {
			صحیح i, rcode = 0;
			برائے (i=0; i < fg_pipe->عدد_progs; i++) {
				اگر (fg_pipe->progs[i].pid == childpid) {
					اگر (i==fg_pipe->عدد_progs-1)
						rcode=WEXITSTATUS(حالت_2);
					(fg_pipe->عدد_progs)--;
					واپس(rcode);
				}
			}
		}

		برائے (pi = کام_list; pi; pi = pi->اگلا) {
			prognum = 0;
			جب (prognum < pi->عدد_progs && pi->progs[prognum].pid != childpid) {
				prognum++;
			}
			اگر (prognum < pi->عدد_progs)
				توڑو;
		}

		اگر(pi==تہی) {
			debug_printf("checkjobs: pid %d was not in our list!\n", childpid);
			جاری;
		}

		اگر (WIFEXITED(حالت_2) || WIFSIGNALED(حالت_2)) {
			/* child exited */
			pi->running_progs--;
			pi->progs[prognum].pid = 0;

			اگر (!pi->running_progs) {
				لکھو(کام_حالت_FORMAT, pi->jobid, "Done", pi->متن);
				remove_bg_کام(pi);
			}
		} ورنہ {
			/* child stopped */
			pi->stopped_progs++;
			pi->progs[prognum].is_stopped = 1;

#اگر 0
			/* Printing this stuff is a pain, since it tends to
			 * overwrite the prompt an inconveinient moments.  So
			 * don't do that.  */
			اگر (pi->stopped_progs == pi->عدد_progs) {
				لکھو("\n"کام_حالت_FORMAT, pi->jobid, "Stopped", pi->متن);
			}
#اگر_ختم
		}
	}

	اگر (childpid == -1 && errno != ECHILD)
		perror_پیغام("waitpid");

	/* move the shell to the foreground */
	/*if (interactive && tcsetpgrp(shell_terminal, getpgid(0))) */
	/*	perror_msg("tcsetpgrp-2"); */
	واپس -1;
}

/* Figure out our controlling tty, checking in order stderr,
 * stdin, and stdout.  If check_pgrp is set, also check that
 * we belong to the foreground process group associated with
 * that tty.  The value of shell_terminal is needed in order to call
 * tcsetpgrp(shell_terminal, ...); */
خالی controlling_tty(صحیح check_pgrp)
{
	pid_t curpgrp;

	اگر ((curpgrp = tcgetpgrp(shell_terminal = 2)) < 0
			&& (curpgrp = tcgetpgrp(shell_terminal = 0)) < 0
			&& (curpgrp = tcgetpgrp(shell_terminal = 1)) < 0)
		جاؤ shell_terminal_error;

	اگر (check_pgrp && curpgrp != getpgid(0))
		جاؤ shell_terminal_error;

	واپس;

shell_terminal_error:
		shell_terminal = -1;
		واپس;
}
#اگر_ختم

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
ساکن صحیح تسلسل_pipe_real(ساخت pipe *pi)
{
	صحیح i;
#اگر_غیر_تعریف_شدہ __U_BOOT__
	صحیح nextin, nextout;
	صحیح pipefds[2];				/* pipefds[0] is for reading */
	ساخت بچہ_prog *بچہ;
	ساخت built_اندر_command *x;
	حرف *p;
# اگر __GNUC__
	/* Avoid longjmp clobbering */
	(خالی) &i;
	(خالی) &nextin;
	(خالی) &nextout;
	(خالی) &بچہ;
# endif
#ورنہ
	صحیح nextin;
	صحیح flag = do_repeat ? CMD_FLAG_REPEAT : 0;
	ساخت بچہ_prog *بچہ;
	حرف *p;
# اگر __GNUC__
	/* Avoid longjmp clobbering */
	(خالی) &i;
	(خالی) &nextin;
	(خالی) &بچہ;
# endif
#اگر_ختم	/* __U_BOOT__ */

	nextin = 0;
#اگر_غیر_تعریف_شدہ __U_BOOT__
	pi->pgrp = -1;
#اگر_ختم

	/* Check if this is a simple builtin (not part of a pipe).
	 * Builtins within pipes have to fork anyway, and are handled in
	 * pseudo_exec.  "echo foo | read bar" doesn't work on bash, either.
	 */
	اگر (pi->عدد_progs == 1) بچہ = & (pi->progs[0]);
#اگر_غیر_تعریف_شدہ __U_BOOT__
	اگر (pi->عدد_progs == 1 && بچہ->گروہ && بچہ->subshell == 0) {
		صحیح squirrel[] = {-1, -1, -1};
		صحیح rcode;
		debug_printf("non-subshell grouping\n");
		تیاری_redirects(بچہ, squirrel);
		/* XXX could we merge code with following builtin case,
		 * by creating a pseudo builtin that calls run_list_real? */
		rcode = تسلسل_list_real(بچہ->گروہ);
		restore_redirects(squirrel);
#ورنہ
		اگر (pi->عدد_progs == 1 && بچہ->گروہ) {
		صحیح rcode;
		debug_printf("non-subshell grouping\n");
		rcode = تسلسل_list_real(بچہ->گروہ);
#اگر_ختم
		واپس rcode;
	} ورنہ اگر (pi->عدد_progs == 1 && pi->progs[0].argv != تہی) {
		برائے (i=0; is_assignment(بچہ->argv[i]); i++) { /* nothing */ }
		اگر (i!=0 && بچہ->argv[i]==تہی) {
			/* assignments, but no command: set the local environment */
			برائے (i=0; بچہ->argv[i]!=تہی; i++) {

				/* Ok, this case is tricky.  We have to decide if this is a
				 * local variable, or an already exported variable.  If it is
				 * already exported, we have to export the new value.  If it is
				 * not exported, we need only set this as a local variable.
				 * This junk is all to decide whether or not to export this
				 * variable. */
				صحیح export_me=0;
				حرف *name, *قدر;
				name = xstrdup(بچہ->argv[i]);
				debug_printf("Local environment set: %s\n", name);
				قدر = strchr(name, '=');
				اگر (قدر)
					*قدر=0;
#اگر_غیر_تعریف_شدہ __U_BOOT__
				اگر ( get_local_var(name)) {
					export_me=1;
				}
#اگر_ختم
				آزاد(name);
				p = داخل_کرو_var_قدر(بچہ->argv[i]);
				سیٹ_local_var(p, export_me);
				اگر (p != بچہ->argv[i]) آزاد(p);
			}
			واپس EXIT_SUCCESS;   /* don't worry about errors in set_local_var() yet */
		}
		برائے (i = 0; is_assignment(بچہ->argv[i]); i++) {
			p = داخل_کرو_var_قدر(بچہ->argv[i]);
#اگر_غیر_تعریف_شدہ __U_BOOT__
			putenv(strdup(p));
#ورنہ
			سیٹ_local_var(p, 0);
#اگر_ختم
			اگر (p != بچہ->argv[i]) {
				بچہ->sp--;
				آزاد(p);
			}
		}
		اگر (بچہ->sp) {
			حرف * str = تہی;

			str = make_string(بچہ->argv + i,
					  بچہ->argv_nonnull + i);
			parse_string_outer(str, FLAG_EXIT_FROM_LOOP | FLAG_REPARSING);
			آزاد(str);
			واپس آخری_return_رمز;
		}
#اگر_غیر_تعریف_شدہ __U_BOOT__
		برائے (x = bltins; x->cmd; x++) {
			اگر (strcmp(بچہ->argv[i], x->cmd) == 0 ) {
				صحیح squirrel[] = {-1, -1, -1};
				صحیح rcode;
				اگر (x->function == builtin_exec && بچہ->argv[i+1]==تہی) {
					debug_printf("magic exec\n");
					تیاری_redirects(بچہ,تہی);
					واپس EXIT_SUCCESS;
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
			واپس rcode;
		}
#ورنہ
		/* check ";", because ,example , argv consist from
		 * "help;flinfo" must not execute
		 */
		اگر (strchr(بچہ->argv[i], ';')) {
			لکھو("Unknown command '%s' - try 'help' or use "
					"'run' command\n", بچہ->argv[i]);
			واپس -1;
		}
		/* Process the command */
		واپس cmd_process(flag, بچہ->argc - i, بچہ->argv + i,
				   &flag_repeat, تہی);
#اگر_ختم
	}
#اگر_غیر_تعریف_شدہ __U_BOOT__

	برائے (i = 0; i < pi->عدد_progs; i++) {
		بچہ = & (pi->progs[i]);

		/* pipes are inserted between pairs of commands */
		اگر ((i + 1) < pi->عدد_progs) {
			اگر (pipe(pipefds)<0) perror_پیغام_and_die("pipe");
			nextout = pipefds[1];
		} ورنہ {
			nextout=1;
			pipefds[0] = -1;
		}

		/* XXX test for failed fork()? */
		اگر (!(بچہ->pid = fork())) {
			/* Set the handling for job control signals back to the default.  */
			اشارہ(SIGINT, SIG_DFL);
			اشارہ(SIGQUIT, SIG_DFL);
			اشارہ(SIGTERM, SIG_DFL);
			اشارہ(SIGTSTP, SIG_DFL);
			اشارہ(SIGTTIN, SIG_DFL);
			اشارہ(SIGTTOU, SIG_DFL);
			اشارہ(SIGCHLD, SIG_DFL);

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
			تیاری_redirects(بچہ,تہی);

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

			pseudo_exec(بچہ);
		}

		/* put our child in the process group whose leader is the
		   first process in this pipe */
		اگر (pi->pgrp < 0) {
			pi->pgrp = بچہ->pid;
		}
		/* Don't check for errors.  The child may be dead already,
		 * in which case setpgid returns error code EACCES. */
		setpgid(بچہ->pid, pi->pgrp);

		اگر (nextin != 0)
			close(nextin);
		اگر (nextout != 1)
			close(nextout);

		/* If there isn't another process, nextin is garbage
		   but it doesn't matter */
		nextin = pipefds[0];
	}
#اگر_ختم
	واپس -1;
}

ساکن صحیح تسلسل_list_real(ساخت pipe *pi)
{
	حرف *save_name = تہی;
	حرف **list = تہی;
	حرف **save_list = تہی;
	ساخت pipe *rpipe;
	صحیح flag_rep = 0;
#اگر_غیر_تعریف_شدہ __U_BOOT__
	صحیح save_عدد_progs;
#اگر_ختم
	صحیح rcode=0, flag_skip=1;
	صحیح flag_restore = 0;
	صحیح if_رمز=0, اگلا_if_رمز=0;  /* need double-buffer to handle elif */
	reserved_style rmode, skip_مزید_اندر_this_rmode=RES_XXXX;
	/* check syntax for "for" */
	برائے (rpipe = pi; rpipe; rpipe = rpipe->اگلا) {
		اگر ((rpipe->r_منوال == RES_اندر ||
		    rpipe->r_منوال == RES_FOR) &&
		    (rpipe->اگلا == تہی)) {
				syntax();
#اگر_تعریف_شدہ __U_BOOT__
				flag_repeat = 0;
#اگر_ختم
				واپس 1;
		}
		اگر ((rpipe->r_منوال == RES_اندر &&
			(rpipe->اگلا->r_منوال == RES_اندر &&
			rpipe->اگلا->progs->argv != تہی))||
			(rpipe->r_منوال == RES_FOR &&
			rpipe->اگلا->r_منوال != RES_اندر)) {
				syntax();
#اگر_تعریف_شدہ __U_BOOT__
				flag_repeat = 0;
#اگر_ختم
				واپس 1;
		}
	}
	برائے (; pi; pi = (flag_restore != 0) ? rpipe : pi->اگلا) {
		اگر (pi->r_منوال == RES_WHILE || pi->r_منوال == RES_UNTIL ||
			pi->r_منوال == RES_FOR) {
#اگر_تعریف_شدہ __U_BOOT__
				/* check Ctrl-C */
				ctrlc();
				اگر ((had_ctrlc())) {
					واپس 1;
				}
#اگر_ختم
				flag_restore = 0;
				اگر (!rpipe) {
					flag_rep = 0;
					rpipe = pi;
				}
		}
		rmode = pi->r_منوال;
		debug_printf("rmode=%d  if_code=%d  next_if_code=%d skip_more=%d\n", rmode, if_رمز, اگلا_if_رمز, skip_مزید_اندر_this_rmode);
		اگر (rmode == skip_مزید_اندر_this_rmode && flag_skip) {
			اگر (pi->followup == PIPE_SEQ) flag_skip=0;
			جاری;
		}
		flag_skip = 1;
		skip_مزید_اندر_this_rmode = RES_XXXX;
		اگر (rmode == RES_THEN || rmode == RES_ELSE) if_رمز = اگلا_if_رمز;
		اگر (rmode == RES_THEN &&  if_رمز) جاری;
		اگر (rmode == RES_ELSE && !if_رمز) جاری;
		اگر (rmode == RES_ELIF && !if_رمز) توڑو;
		اگر (rmode == RES_FOR && pi->عدد_progs) {
			اگر (!list) {
				/* if no variable values after "in" we skip "for" */
				اگر (!pi->اگلا->progs->argv) جاری;
				/* create list of variable values */
				list = make_list_اندر(pi->اگلا->progs->argv,
					pi->progs->argv[0]);
				save_list = list;
				save_name = pi->progs->argv[0];
				pi->progs->argv[0] = تہی;
				flag_rep = 1;
			}
			اگر (!(*list)) {
				آزاد(pi->progs->argv[0]);
				آزاد(save_list);
				list = تہی;
				flag_rep = 0;
				pi->progs->argv[0] = save_name;
#اگر_غیر_تعریف_شدہ __U_BOOT__
				pi->progs->glob_نتیجہ.gl_pathv[0] =
					pi->progs->argv[0];
#اگر_ختم
				جاری;
			} ورنہ {
				/* insert new value from list for variable */
				آزاد(pi->progs->argv[0]);
				pi->progs->argv[0] = *list++;
#اگر_غیر_تعریف_شدہ __U_BOOT__
				pi->progs->glob_نتیجہ.gl_pathv[0] =
					pi->progs->argv[0];
#اگر_ختم
			}
		}
		اگر (rmode == RES_اندر) جاری;
		اگر (rmode == RES_DO) {
			اگر (!flag_rep) جاری;
		}
		اگر (rmode == RES_مکمل) {
			اگر (flag_rep) {
				flag_restore = 1;
			} ورنہ {
				rpipe = تہی;
			}
		}
		اگر (pi->عدد_progs == 0) جاری;
#اگر_غیر_تعریف_شدہ __U_BOOT__
		save_عدد_progs = pi->عدد_progs; /* save number of programs */
#اگر_ختم
		rcode = تسلسل_pipe_real(pi);
		debug_printf("run_pipe_real returned %d\n",rcode);
#اگر_غیر_تعریف_شدہ __U_BOOT__
		اگر (rcode!=-1) {
			/* We only ran a builtin: rcode was set by the return value
			 * of run_pipe_real(), and we don't need to wait for anything. */
		} ورنہ اگر (pi->followup==PIPE_BG) {
			/* XXX check bash's behavior with nontrivial pipes */
			/* XXX compute jobid */
			/* XXX what does bash do with attempts to background builtins? */
			داخل_کرو_bg_کام(pi);
			rcode = EXIT_SUCCESS;
		} ورنہ {
			اگر (interactive) {
				/* move the new process group into the foreground */
				اگر (tcsetpgrp(shell_terminal, pi->pgrp) && errno != ENOTTY)
					perror_پیغام("tcsetpgrp-3");
				rcode = checkjobs(pi);
				/* move the shell to the foreground */
				اگر (tcsetpgrp(shell_terminal, getpgid(0)) && errno != ENOTTY)
					perror_پیغام("tcsetpgrp-4");
			} ورنہ {
				rcode = checkjobs(pi);
			}
			debug_printf("checkjobs returned %d\n",rcode);
		}
		آخری_return_رمز=rcode;
#ورنہ
		اگر (rcode < -1) {
			آخری_return_رمز = -rcode - 2;
			واپس -2;	/* exit */
		}
		آخری_return_رمز = rcode;
#اگر_ختم
#اگر_غیر_تعریف_شدہ __U_BOOT__
		pi->عدد_progs = save_عدد_progs; /* restore number of programs */
#اگر_ختم
		اگر ( rmode == RES_IF || rmode == RES_ELIF )
			اگلا_if_رمز=rcode;  /* can be overwritten a number of times */
		اگر (rmode == RES_WHILE)
			flag_rep = !آخری_return_رمز;
		اگر (rmode == RES_UNTIL)
			flag_rep = آخری_return_رمز;
		اگر ( (rcode==EXIT_SUCCESS && pi->followup==PIPE_OR) ||
		     (rcode!=EXIT_SUCCESS && pi->followup==PIPE_AND) )
			skip_مزید_اندر_this_rmode=rmode;
#اگر_غیر_تعریف_شدہ __U_BOOT__
		checkjobs(تہی);
#اگر_ختم
	}
	واپس rcode;
}

/* broken, of course, but OK for testing */
ساکن حرف *indenter(صحیح i)
{
	ساکن حرف blanks[]="                                    ";
	واپس &blanks[حجم(blanks)-i-1];
}

/* return code is the exit status of the pipe */
ساکن صحیح free_pipe(ساخت pipe *pi, صحیح indent)
{
	حرف **p;
	ساخت بچہ_prog *بچہ;
#اگر_غیر_تعریف_شدہ __U_BOOT__
	ساخت redir_struct *r, *rnext;
#اگر_ختم
	صحیح a, i, ret_رمز=0;
	حرف *ind = indenter(indent);

#اگر_غیر_تعریف_شدہ __U_BOOT__
	اگر (pi->stopped_progs > 0)
		واپس ret_رمز;
	final_printf("%s run pipe: (pid %d)\n",ind,getpid());
#اگر_ختم
	برائے (i=0; i<pi->عدد_progs; i++) {
		بچہ = &pi->progs[i];
		final_printf("%s  command %d:\n",ind,i);
		اگر (بچہ->argv) {
			برائے (a=0,p=بچہ->argv; *p; a++,p++) {
				final_printf("%s   argv[%d] = %s\n",ind,a,*p);
			}
#اگر_غیر_تعریف_شدہ __U_BOOT__
			globfree(&بچہ->glob_نتیجہ);
#ورنہ
			برائے (a = 0; a < بچہ->argc; a++) {
				آزاد(بچہ->argv[a]);
			}
			آزاد(بچہ->argv);
			آزاد(بچہ->argv_nonnull);
			بچہ->argc = 0;
#اگر_ختم
			بچہ->argv=تہی;
		} ورنہ اگر (بچہ->گروہ) {
#اگر_غیر_تعریف_شدہ __U_BOOT__
			final_printf("%s   begin group (subshell:%d)\n",ind, بچہ->subshell);
#اگر_ختم
			ret_رمز = free_pipe_list(بچہ->گروہ,indent+3);
			final_printf("%s   end group\n",ind);
		} ورنہ {
			final_printf("%s   (nil)\n",ind);
		}
#اگر_غیر_تعریف_شدہ __U_BOOT__
		برائے (r=بچہ->redirects; r; r=rnext) {
			final_printf("%s   redirect %d%s", ind, r->fd, redir_جدول[r->type].descrip);
			اگر (r->dup == -1) {
				/* guard against the case >$FOO, where foo is unset or blank */
				اگر (r->لفظ.gl_pathv) {
					final_printf(" %s\n", *r->لفظ.gl_pathv);
					globfree(&r->لفظ);
				}
			} ورنہ {
				final_printf("&%d\n", r->dup);
			}
			rnext=r->اگلا;
			آزاد(r);
		}
		بچہ->redirects=تہی;
#اگر_ختم
	}
	آزاد(pi->progs);   /* children are an array, they get freed all at once */
	pi->progs=تہی;
	واپس ret_رمز;
}

ساکن صحیح free_pipe_list(ساخت pipe *سر, صحیح indent)
{
	صحیح rcode=0;   /* if list has no members */
	ساخت pipe *pi, *اگلا;
	حرف *ind = indenter(indent);
	برائے (pi=سر; pi; pi=اگلا) {
		final_printf("%s pipe reserved mode %d\n", ind, pi->r_منوال);
		rcode = free_pipe(pi, indent);
		final_printf("%s pipe followup code %d\n", ind, pi->followup);
		اگلا=pi->اگلا;
		pi->اگلا=تہی;
		آزاد(pi);
	}
	واپس rcode;
}

/* Select which version we will use */
ساکن صحیح تسلسل_list(ساخت pipe *pi)
{
	صحیح rcode=0;
#اگر_غیر_تعریف_شدہ __U_BOOT__
	اگر (fake_منوال==0) {
#اگر_ختم
		rcode = تسلسل_list_real(pi);
#اگر_غیر_تعریف_شدہ __U_BOOT__
	}
#اگر_ختم
	/* free_pipe_list has the side effect of clearing memory
	 * In the long run that function can be merged with run_list_real,
	 * but doing that now would hobble the debugging effort. */
	free_pipe_list(pi,0);
	واپس rcode;
}

/* The API for glob is arguably broken.  This routine pushes a non-matching
 * string into the output structure, removing non-backslashed backslashes.
 * If someone can prove me wrong, by performing this function within the
 * original glob(3) api, feel free to rewrite this routine into oblivion.
 * Return code (0 vs. GLOB_NOSPACE) matches glob(3).
 * XXX broken if the last character is '\\', check that before calling.
 */
#اگر_غیر_تعریف_شدہ __U_BOOT__
ساکن صحیح globhack(ثابت حرف *src, صحیح flags, glob_t *pglob)
{
	صحیح cnt=0, pathc;
	ثابت حرف *s;
	حرف *dest;
	برائے (cnt=1, s=src; s && *s; s++) {
		اگر (*s == '\\') s++;
		cnt++;
	}
	dest = حافظہ_دو(cnt);
	اگر (!dest) واپس GLOB_NOSPACE;
	اگر (!(flags & GLOB_APPEND)) {
		pglob->gl_pathv=تہی;
		pglob->gl_pathc=0;
		pglob->gl_offs=0;
		pglob->gl_offs=0;
	}
	pathc = ++pglob->gl_pathc;
	pglob->gl_pathv = realloc(pglob->gl_pathv, (pathc+1)*حجم(*pglob->gl_pathv));
	اگر (pglob->gl_pathv == تہی) واپس GLOB_NOSPACE;
	pglob->gl_pathv[pathc-1]=dest;
	pglob->gl_pathv[pathc]=تہی;
	برائے (s=src; s && *s; s++, dest++) {
		اگر (*s == '\\') s++;
		*dest = *s;
	}
	*dest='\0';
	واپس 0;
}

/* XXX broken if the last character is '\\', check that before calling */
ساکن صحیح glob_needed(ثابت حرف *s)
{
	برائے (; *s; s++) {
		اگر (*s == '\\') s++;
		اگر (strchr("*[?",*s)) واپس 1;
	}
	واپس 0;
}

#اگر 0
ساکن خالی globprint(glob_t *pglob)
{
	صحیح i;
	debug_printf("glob_t at %p:\n", pglob);
	debug_printf("  gl_pathc=%d  gl_pathv=%p  gl_offs=%d  gl_flags=%d\n",
		pglob->gl_pathc, pglob->gl_pathv, pglob->gl_offs, pglob->gl_flags);
	برائے (i=0; i<pglob->gl_pathc; i++)
		debug_printf("pglob->gl_pathv[%d] = %p = %s\n", i,
			pglob->gl_pathv[i], pglob->gl_pathv[i]);
}
#اگر_ختم

ساکن صحیح xglob(o_string *dest, صحیح flags, glob_t *pglob)
{
	صحیح gr;

	/* short-circuit for null word */
	/* we can code this better when the debug_printf's are gone */
	اگر (dest->لمبائی == 0) {
		اگر (dest->nonnull) {
			/* bash man page calls this an "explicit" null */
			gr = globhack(dest->ڈیٹا, flags, pglob);
			debug_printf("globhack returned %d\n",gr);
		} ورنہ {
			واپس 0;
		}
	} ورنہ اگر (glob_needed(dest->ڈیٹا)) {
		gr = glob(dest->ڈیٹا, flags, تہی, pglob);
		debug_printf("glob returned %d\n",gr);
		اگر (gr == GLOB_NOMATCH) {
			/* quote removal, or more accurately, backslash removal */
			gr = globhack(dest->ڈیٹا, flags, pglob);
			debug_printf("globhack returned %d\n",gr);
		}
	} ورنہ {
		gr = globhack(dest->ڈیٹا, flags, pglob);
		debug_printf("globhack returned %d\n",gr);
	}
	اگر (gr == GLOB_NOSPACE)
		error_پیغام_and_die("out of memory during glob");
	اگر (gr != 0) { /* GLOB_ABORTED ? */
		error_پیغام("glob(3) error %d",gr);
	}
	/* globprint(glob_target); */
	واپس gr;
}
#اگر_ختم

#اگر_تعریف_شدہ __U_BOOT__
ساکن حرف *get_dollar_var(حرف ch);
#اگر_ختم

/* This is used to get/check local shell variables */
حرف *get_local_var(ثابت حرف *s)
{
	ساخت variables *cur;

	اگر (!s)
		واپس تہی;

#اگر_تعریف_شدہ __U_BOOT__
	اگر (*s == '$')
		واپس get_dollar_var(s[1]);
#اگر_ختم

	برائے (cur = چوٹی_vars; cur; cur=cur->اگلا)
		اگر(strcmp(cur->name, s)==0)
			واپس cur->قدر;
	واپس تہی;
}

/* This is used to set local shell variables
   flg_export==0 if only local (not exporting) variable
   flg_export==1 if "new" exporting environ
   flg_export>1  if current startup environ (not call putenv()) */
صحیح سیٹ_local_var(ثابت حرف *s, صحیح flg_export)
{
	حرف *name, *قدر;
	صحیح نتیجہ=0;
	ساخت variables *cur;

#اگر_تعریف_شدہ __U_BOOT__
	/* might be possible! */
	اگر (!حرف_ہے(*s))
		واپس -1;
#اگر_ختم

	name=strdup(s);

	/* Assume when we enter this function that we are already in
	 * NAME=VALUE format.  So the first order of business is to
	 * split 's' on the '=' into 'name' and 'value' */
	قدر = strchr(name, '=');
	اگر (!قدر) {
		آزاد(name);
		واپس -1;
	}
	*قدر++ = 0;

	اگر (!*قدر) {
		unset_local_var(name);
		آزاد(name);
		واپس 0;
	}

	برائے(cur = چوٹی_vars; cur; cur = cur->اگلا) {
		اگر(strcmp(cur->name, name)==0)
			توڑو;
	}

	اگر(cur) {
		اگر(strcmp(cur->قدر, قدر)==0) {
			اگر(flg_export>0 && cur->flg_export==0)
				cur->flg_export=flg_export;
			ورنہ
				نتیجہ++;
		} ورنہ {
			اگر(cur->flg_read_only) {
				error_پیغام("%s: readonly variable", name);
				نتیجہ = -1;
			} ورنہ {
				اگر(flg_export>0 || cur->flg_export>1)
					cur->flg_export=1;
				آزاد(cur->قدر);

				cur->قدر = strdup(قدر);
			}
		}
	} ورنہ {
		cur = حافظہ_دو(حجم(ساخت variables));
		اگر(!cur) {
			نتیجہ = -1;
		} ورنہ {
			cur->name = strdup(name);
			اگر (cur->name == تہی) {
				آزاد(cur);
				نتیجہ = -1;
			} ورنہ {
				ساخت variables *bottom = چوٹی_vars;
				cur->قدر = strdup(قدر);
				cur->اگلا = تہی;
				cur->flg_export = flg_export;
				cur->flg_read_only = 0;
				جب(bottom->اگلا) bottom=bottom->اگلا;
				bottom->اگلا = cur;
			}
		}
	}

#اگر_غیر_تعریف_شدہ __U_BOOT__
	اگر(نتیجہ==0 && cur->flg_export==1) {
		*(قدر-1) = '=';
		نتیجہ = putenv(name);
	} ورنہ {
#اگر_ختم
		آزاد(name);
#اگر_غیر_تعریف_شدہ __U_BOOT__
		اگر(نتیجہ>0)            /* equivalent to previous set */
			نتیجہ = 0;
	}
#اگر_ختم
	واپس نتیجہ;
}

خالی unset_local_var(ثابت حرف *name)
{
	ساخت variables *cur;

	اگر (name) {
		برائے (cur = چوٹی_vars; cur; cur=cur->اگلا) {
			اگر(strcmp(cur->name, name)==0)
				توڑو;
		}
		اگر (cur != تہی) {
			ساخت variables *اگلا = چوٹی_vars;
			اگر(cur->flg_read_only) {
				error_پیغام("%s: readonly variable", name);
				واپس;
			} ورنہ {
#اگر_غیر_تعریف_شدہ __U_BOOT__
				اگر(cur->flg_export)
					unenv_سیٹ(cur->name);
#اگر_ختم
				آزاد(cur->name);
				آزاد(cur->قدر);
				جب (اگلا->اگلا != cur)
					اگلا = اگلا->اگلا;
				اگلا->اگلا = cur->اگلا;
			}
			آزاد(cur);
		}
	}
}

ساکن صحیح is_assignment(ثابت حرف *s)
{
	اگر (s == تہی)
		واپس 0;

	اگر (!حرف_ہے(*s)) واپس 0;
	++s;
	جب(حرف_یا_ہندسہ(*s) || *s=='_') ++s;
	واپس *s=='=';
}

#اگر_غیر_تعریف_شدہ __U_BOOT__
/* the src parameter allows us to peek forward to a possible &n syntax
 * for file descriptor duplication, e.g., "2>&1".
 * Return code is 0 normally, 1 if a syntax error is detected in src.
 * Resource errors (in xmalloc) cause the process to exit */
ساکن صحیح تیاری_redirect(ساخت p_context *ctx, صحیح fd, redir_type style,
	ساخت اندر_str *داخلہ)
{
	ساخت بچہ_prog *بچہ=ctx->بچہ;
	ساخت redir_struct *redir = بچہ->redirects;
	ساخت redir_struct *آخری_redir=تہی;

	/* Create a new redir_struct and drop it onto the end of the linked list */
	جب(redir) {
		آخری_redir=redir;
		redir=redir->اگلا;
	}
	redir = xmalloc(حجم(ساخت redir_struct));
	redir->اگلا=تہی;
	redir->لفظ.gl_pathv=تہی;
	اگر (آخری_redir) {
		آخری_redir->اگلا=redir;
	} ورنہ {
		بچہ->redirects=redir;
	}

	redir->type=style;
	redir->fd= (fd==-1) ? redir_جدول[style].default_fd : fd ;

	debug_printf("Redirect type %d%s\n", redir->fd, redir_جدول[style].descrip);

	/* Check for a '2>&1' type redirect */
	redir->dup = redirect_dup_عدد(داخلہ);
	اگر (redir->dup == -2) واپس 1;  /* syntax error */
	اگر (redir->dup != -1) {
		/* Erik had a check here that the file descriptor in question
		 * is legit; I postpone that to "run time"
		 * A "-" representation of "close me" shows up as a -3 here */
		debug_printf("Duplicating redirect '%d>&%d'\n", redir->fd, redir->dup);
	} ورنہ {
		/* We do _not_ try to open the file that src points to,
		 * since we need to return and let src be expanded first.
		 * Set ctx->pending_redirect, so we know what to do at the
		 * end of the next parsed word.
		 */
		ctx->pending_redirect = redir;
	}
	واپس 0;
}
#اگر_ختم

ساکن ساخت pipe *new_pipe(خالی)
{
	ساخت pipe *pi;
	pi = xmalloc(حجم(ساخت pipe));
	pi->عدد_progs = 0;
	pi->progs = تہی;
	pi->اگلا = تہی;
	pi->followup = 0;  /* invalid */
	pi->r_منوال = RES_NONE;
	واپس pi;
}

ساکن خالی initialize_context(ساخت p_context *ctx)
{
	ctx->pipe=تہی;
#اگر_غیر_تعریف_شدہ __U_BOOT__
	ctx->pending_redirect=تہی;
#اگر_ختم
	ctx->بچہ=تہی;
	ctx->list_سر=new_pipe();
	ctx->pipe=ctx->list_سر;
	ctx->w=RES_NONE;
	ctx->ڈھیر=تہی;
#اگر_تعریف_شدہ __U_BOOT__
	ctx->old_flag=0;
#اگر_ختم
	مکمل_command(ctx);   /* creates the memory for working child */
}

/* normal return is 0
 * if a reserved word is found, and processed, return 1
 * should handle if, then, elif, else, fi, for, while, until, do, done.
 * case, function, and select are obnoxious, save those for later.
 */
ساخت reserved_combo {
	حرف *literal;
	صحیح رمز;
	طویل flag;
};
/* Mostly a list of accepted follow-up reserved words.
 * FLAG_END means we are done with the sequence, and are ready
 * to turn the compound list into a command.
 * FLAG_START means the word must start a new compound list.
 */
ساکن ساخت reserved_combo reserved_list[] = {
	{ "if",    RES_IF,    FLAG_THEN | FLAG_آغاز },
	{ "then",  RES_THEN,  FLAG_ELIF | FLAG_ELSE | FLAG_FI },
	{ "elif",  RES_ELIF,  FLAG_THEN },
	{ "else",  RES_ELSE,  FLAG_FI   },
	{ "fi",    RES_FI,    FLAG_انتہا  },
	{ "for",   RES_FOR,   FLAG_اندر   | FLAG_آغاز },
	{ "while", RES_WHILE, FLAG_DO   | FLAG_آغاز },
	{ "until", RES_UNTIL, FLAG_DO   | FLAG_آغاز },
	{ "in",    RES_اندر,    FLAG_DO   },
	{ "do",    RES_DO,    FLAG_مکمل },
	{ "done",  RES_مکمل,  FLAG_انتہا  }
};
#تعریف_کرو NRES (حجم(reserved_list)/حجم(ساخت reserved_combo))

ساکن صحیح reserved_لفظ(o_string *dest, ساخت p_context *ctx)
{
	ساخت reserved_combo *r;
	برائے (r=reserved_list;
		r<reserved_list+NRES; r++) {
		اگر (strcmp(dest->ڈیٹا, r->literal) == 0) {
			debug_printf("found reserved word %s, code %d\n",r->literal,r->رمز);
			اگر (r->flag & FLAG_آغاز) {
				ساخت p_context *new = xmalloc(حجم(ساخت p_context));
				debug_printf("push stack\n");
				اگر (ctx->w == RES_اندر || ctx->w == RES_FOR) {
					syntax();
					آزاد(new);
					ctx->w = RES_SNTX;
					b_reset(dest);
					واپس 1;
				}
				*new = *ctx;   /* physical copy */
				initialize_context(ctx);
				ctx->ڈھیر=new;
			} ورنہ اگر ( ctx->w == RES_NONE || ! (ctx->old_flag & (1<<r->رمز))) {
				syntax();
				ctx->w = RES_SNTX;
				b_reset(dest);
				واپس 1;
			}
			ctx->w=r->رمز;
			ctx->old_flag = r->flag;
			اگر (ctx->old_flag & FLAG_انتہا) {
				ساخت p_context *old;
				debug_printf("pop stack\n");
				مکمل_pipe(ctx,PIPE_SEQ);
				old = ctx->ڈھیر;
				old->بچہ->گروہ = ctx->list_سر;
#اگر_غیر_تعریف_شدہ __U_BOOT__
				old->بچہ->subshell = 0;
#اگر_ختم
				*ctx = *old;   /* physical copy */
				آزاد(old);
			}
			b_reset (dest);
			واپس 1;
		}
	}
	واپس 0;
}

/* normal return is 0.
 * Syntax or xglob errors return 1. */
ساکن صحیح مکمل_لفظ(o_string *dest, ساخت p_context *ctx)
{
	ساخت بچہ_prog *بچہ=ctx->بچہ;
#اگر_غیر_تعریف_شدہ __U_BOOT__
	glob_t *glob_ہدف;
	صحیح gr, flags = 0;
#ورنہ
	حرف *str, *s;
	صحیح argc, cnt;
#اگر_ختم

	debug_printf("done_word: %s %p\n", dest->ڈیٹا, بچہ);
	اگر (dest->لمبائی == 0 && !dest->nonnull) {
		debug_printf("  true null, ignored\n");
		واپس 0;
	}
#اگر_غیر_تعریف_شدہ __U_BOOT__
	اگر (ctx->pending_redirect) {
		glob_ہدف = &ctx->pending_redirect->لفظ;
	} ورنہ {
#اگر_ختم
		اگر (بچہ->گروہ) {
			syntax();
			واپس 1;  /* syntax error, groups and arglists don't mix */
		}
		اگر (!بچہ->argv && (ctx->type & FLAG_PARSE_SEMICOLON)) {
			debug_printf("checking %s for reserved-ness\n",dest->ڈیٹا);
			اگر (reserved_لفظ(dest,ctx)) واپس ctx->w==RES_SNTX;
		}
#اگر_غیر_تعریف_شدہ __U_BOOT__
		glob_ہدف = &بچہ->glob_نتیجہ;
		اگر (بچہ->argv) flags |= GLOB_APPEND;
#ورنہ
		برائے (cnt = 1, s = dest->ڈیٹا; s && *s; s++) {
			اگر (*s == '\\') s++;
			cnt++;
		}
		str = حافظہ_دو(cnt);
		اگر (!str) واپس 1;
		اگر ( بچہ->argv == تہی) {
			بچہ->argc=0;
		}
		argc = ++بچہ->argc;
		بچہ->argv = realloc(بچہ->argv, (argc+1)*حجم(*بچہ->argv));
		اگر (بچہ->argv == تہی) {
			آزاد(str);
			واپس 1;
		}
		بچہ->argv_nonnull = realloc(بچہ->argv_nonnull,
					(argc+1)*حجم(*بچہ->argv_nonnull));
		اگر (بچہ->argv_nonnull == تہی) {
			آزاد(str);
			واپس 1;
		}
		بچہ->argv[argc-1]=str;
		بچہ->argv_nonnull[argc-1] = dest->nonnull;
		بچہ->argv[argc]=تہی;
		بچہ->argv_nonnull[argc] = 0;
		برائے (s = dest->ڈیٹا; s && *s; s++,str++) {
			اگر (*s == '\\') s++;
			*str = *s;
		}
		*str = '\0';
#اگر_ختم
#اگر_غیر_تعریف_شدہ __U_BOOT__
	}
	gr = xglob(dest, flags, glob_ہدف);
	اگر (gr != 0) واپس 1;
#اگر_ختم

	b_reset(dest);
#اگر_غیر_تعریف_شدہ __U_BOOT__
	اگر (ctx->pending_redirect) {
		ctx->pending_redirect=تہی;
		اگر (glob_ہدف->gl_pathc != 1) {
			error_پیغام("ambiguous redirect");
			واپس 1;
		}
	} ورنہ {
		بچہ->argv = glob_ہدف->gl_pathv;
	}
#اگر_ختم
	اگر (ctx->w == RES_FOR) {
		مکمل_لفظ(dest,ctx);
		مکمل_pipe(ctx,PIPE_SEQ);
	}
	واپس 0;
}

/* The only possible error here is out of memory, in which case
 * xmalloc exits. */
ساکن صحیح مکمل_command(ساخت p_context *ctx)
{
	/* The child is really already in the pipe structure, so
	 * advance the pipe counter and make a new, null child.
	 * Only real trickiness here is that the uncommitted
	 * child structure, to which ctx->child points, is not
	 * counted in pi->num_progs. */
	ساخت pipe *pi=ctx->pipe;
	ساخت بچہ_prog *prog=ctx->بچہ;

	اگر (prog && prog->گروہ == تہی
		 && prog->argv == تہی
#اگر_غیر_تعریف_شدہ __U_BOOT__
		 && prog->redirects == تہی) {
#ورنہ
										) {
#اگر_ختم
		debug_printf("done_command: skipping null command\n");
		واپس 0;
	} ورنہ اگر (prog) {
		pi->عدد_progs++;
		debug_printf("done_command: num_progs incremented to %d\n",pi->عدد_progs);
	} ورنہ {
		debug_printf("done_command: initializing\n");
	}
	pi->progs = xrealloc(pi->progs, حجم(*pi->progs) * (pi->عدد_progs+1));

	prog = pi->progs + pi->عدد_progs;
#اگر_غیر_تعریف_شدہ __U_BOOT__
	prog->redirects = تہی;
#اگر_ختم
	prog->argv = تہی;
	prog->argv_nonnull = تہی;
#اگر_غیر_تعریف_شدہ __U_BOOT__
	prog->is_stopped = 0;
#اگر_ختم
	prog->گروہ = تہی;
#اگر_غیر_تعریف_شدہ __U_BOOT__
	prog->glob_نتیجہ.gl_pathv = تہی;
	prog->family = pi;
#اگر_ختم
	prog->sp = 0;
	ctx->بچہ = prog;
	prog->type = ctx->type;

	/* but ctx->pipe and ctx->list_head remain unchanged */
	واپس 0;
}

ساکن صحیح مکمل_pipe(ساخت p_context *ctx, pipe_style type)
{
	ساخت pipe *new_p;
	مکمل_command(ctx);  /* implicit closure of previous command */
	debug_printf("done_pipe, type %d\n", type);
	ctx->pipe->followup = type;
	ctx->pipe->r_منوال = ctx->w;
	new_p=new_pipe();
	ctx->pipe->اگلا = new_p;
	ctx->pipe = new_p;
	ctx->بچہ = تہی;
	مکمل_command(ctx);  /* set up new pipe to accept commands */
	واپس 0;
}

#اگر_غیر_تعریف_شدہ __U_BOOT__
/* peek ahead in the in_str to find out if we have a "&n" construct,
 * as in "2>&1", that represents duplicating a file descriptor.
 * returns either -2 (syntax error), -1 (no &), or the number found.
 */
ساکن صحیح redirect_dup_عدد(ساخت اندر_str *داخلہ)
{
	صحیح ch, d=0, ok=0;
	ch = b_peek(داخلہ);
	اگر (ch != '&') واپس -1;

	b_getch(داخلہ);  /* get the & */
	ch=b_peek(داخلہ);
	اگر (ch == '-') {
		b_getch(داخلہ);
		واپس -3;  /* "-" represents "close me" */
	}
	جب (ہندسہ_ہے(ch)) {
		d = d*10+(ch-'0');
		ok=1;
		b_getch(داخلہ);
		ch = b_peek(داخلہ);
	}
	اگر (ok) واپس d;

	error_پیغام("ambiguous redirect");
	واپس -2;
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
ساکن صحیح redirect_opt_عدد(o_string *o)
{
	صحیح عدد;

	اگر (o->لمبائی==0) واپس -1;
	برائے(عدد=0; عدد<o->لمبائی; عدد++) {
		اگر (!ہندسہ_ہے(*(o->ڈیٹا+عدد))) {
			واپس -1;
		}
	}
	/* reuse num (and save an int) */
	عدد=atoi(o->ڈیٹا);
	b_reset(o);
	واپس عدد;
}

FILE *generate_دھارا_from_list(ساخت pipe *سر)
{
	FILE *pf;
#اگر 1
	صحیح pid, channel[2];
	اگر (pipe(channel)<0) perror_پیغام_and_die("pipe");
	pid=fork();
	اگر (pid<0) {
		perror_پیغام_and_die("fork");
	} ورنہ اگر (pid==0) {
		close(channel[0]);
		اگر (channel[1] != 1) {
			dup2(channel[1],1);
			close(channel[1]);
		}
#اگر 0
#تعریف_کرو SURROGATE "surrogate response"
		اكتب(1,SURROGATE,حجم(SURROGATE));
		_exit(تسلسل_list(سر));
#ورنہ
		_exit(تسلسل_list_real(سر));   /* leaks memory */
#اگر_ختم
	}
	debug_printf("forked child %d\n",pid);
	close(channel[1]);
	pf = fdopen(channel[0],"r");
	debug_printf("pipe on FILE *%p\n",pf);
#ورنہ
	free_pipe_list(سر,0);
	pf=popen("echo surrogate response","r");
	debug_printf("started fake pipe on FILE *%p\n",pf);
#اگر_ختم
	واپس pf;
}

/* this version hacked for testing purposes */
/* return code is exit status of the process that is run. */
ساکن صحیح process_command_subs(o_string *dest, ساخت p_context *ctx, ساخت اندر_str *داخلہ, صحیح subst_انتہا)
{
	صحیح retcode;
	o_string نتیجہ=NULL_O_STRING;
	ساخت p_context inner;
	FILE *p;
	ساخت اندر_str pipe_str;
	initialize_context(&inner);

	/* recursion to generate command */
	retcode = parse_دھارا(&نتیجہ, &inner, داخلہ, subst_انتہا);
	اگر (retcode != 0) واپس retcode;  /* syntax error or EOF */
	مکمل_لفظ(&نتیجہ, &inner);
	مکمل_pipe(&inner, PIPE_SEQ);
	b_free(&نتیجہ);

	p=generate_دھارا_from_list(inner.list_سر);
	اگر (p==تہی) واپس 1;
	mark_open(fileno(p));
	تیاری_file_اندر_str(&pipe_str, p);

	/* now send results of command back into original context */
	retcode = parse_دھارا(dest, ctx, &pipe_str, '\0');
	/* XXX In case of a syntax error, should we try to kill the child?
	 * That would be tough to do right, so just read until EOF. */
	اگر (retcode == 1) {
		جب (b_getch(&pipe_str)!=EOF) { /* discard */ };
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
	واپس retcode;
}

ساکن صحیح parse_گروہ(o_string *dest, ساخت p_context *ctx,
	ساخت اندر_str *داخلہ, صحیح ch)
{
	صحیح rcode, endch=0;
	ساخت p_context sub;
	ساخت بچہ_prog *بچہ = ctx->بچہ;
	اگر (بچہ->argv) {
		syntax();
		واپس 1;  /* syntax error, groups and arglists don't mix */
	}
	initialize_context(&sub);
	انتخاب(ch) {
		حالت '(': endch=')'; بچہ->subshell=1; توڑو;
		حالت '{': endch='}'; توڑو;
		طےشدہ: syntax();   /* really logic error */
	}
	rcode=parse_دھارا(dest,&sub,داخلہ,endch);
	مکمل_لفظ(dest,&sub); /* finish off the final word in the subcontext */
	مکمل_pipe(&sub, PIPE_SEQ);  /* and the final command there, too */
	بچہ->گروہ = sub.list_سر;
	واپس rcode;
	/* child remains "open", available for possible redirects */
}
#اگر_ختم

/* basically useful version until someone wants to get fancier,
 * see the bash man page under "Parameter Expansion" */
ساکن حرف *lookup_param(حرف *src)
{
	حرف *p;
	حرف *sep;
	حرف *default_val = تہی;
	صحیح تفویض = 0;
	صحیح expand_خالی_خانے = 0;

	اگر (!src)
		واپس تہی;

	sep = strchr(src, ':');

	اگر (sep) {
		*sep = '\0';
		اگر (*(sep + 1) == '-')
			default_val = sep+2;
		اگر (*(sep + 1) == '=') {
			default_val = sep+2;
			تفویض = 1;
		}
		اگر (*(sep + 1) == '+') {
			default_val = sep+2;
			expand_خالی_خانے = 1;
		}
	}

	p = env_get(src);
	اگر (!p)
		p = get_local_var(src);

	اگر (!p || لڑی_لمبائی(p) == 0) {
		p = default_val;
		اگر (تفویض) {
			حرف *var = حافظہ_دو(لڑی_لمبائی(src)+لڑی_لمبائی(default_val)+2);
			اگر (var) {
				sprintf(var, "%s=%s", src, default_val);
				سیٹ_local_var(var, 0);
			}
			آزاد(var);
		}
	} ورنہ اگر (expand_خالی_خانے) {
		p += لڑی_لمبائی(p);
	}

	اگر (sep)
		*sep = ':';

	واپس p;
}

#اگر_تعریف_شدہ __U_BOOT__
ساکن حرف *get_dollar_var(حرف ch)
{
	ساکن حرف buf[40];

	buf[0] = '\0';
	انتخاب (ch) {
		حالت '?':
			sprintf(buf, "%u", (غیرعلامتی صحیح)آخری_return_رمز);
			توڑو;
		طےشدہ:
			واپس تہی;
	}
	واپس buf;
}
#اگر_ختم

/* return code: 0 for OK, 1 for syntax error */
ساکن صحیح handle_dollar(o_string *dest, ساخت p_context *ctx, ساخت اندر_str *داخلہ)
{
#اگر_غیر_تعریف_شدہ __U_BOOT__
	صحیح i, advance=0;
#ورنہ
	صحیح advance=0;
#اگر_ختم
#اگر_غیر_تعریف_شدہ __U_BOOT__
	حرف sep[]=" ";
#اگر_ختم
	صحیح ch = داخلہ->peek(داخلہ);  /* first character after the $ */
	debug_printf("handle_dollar: ch=%c\n",ch);
	اگر (حرف_ہے(ch)) {
		b_addchr(dest, SPECIAL_VAR_علامت);
		ctx->بچہ->sp++;
		جب(ch=b_peek(داخلہ),حرف_یا_ہندسہ(ch) || ch=='_') {
			b_getch(داخلہ);
			b_addchr(dest,ch);
		}
		b_addchr(dest, SPECIAL_VAR_علامت);
#اگر_غیر_تعریف_شدہ __U_BOOT__
	} ورنہ اگر (ہندسہ_ہے(ch)) {
		i = ch-'0';  /* XXX is $0 special? */
		اگر (i<global_argc) {
			parse_string(dest, ctx, global_argv[i]); /* recursion */
		}
		advance = 1;
#اگر_ختم
	} ورنہ انتخاب (ch) {
#اگر_غیر_تعریف_شدہ __U_BOOT__
		حالت '$':
			b_adduint(dest,getpid());
			advance = 1;
			توڑو;
		حالت '!':
			اگر (آخری_bg_pid > 0) b_adduint(dest, آخری_bg_pid);
			advance = 1;
			توڑو;
#اگر_ختم
		حالت '?':
#اگر_غیر_تعریف_شدہ __U_BOOT__
			b_adduint(dest,آخری_return_رمز);
#ورنہ
			ctx->بچہ->sp++;
			b_addchr(dest, SPECIAL_VAR_علامت);
			b_addchr(dest, '$');
			b_addchr(dest, '?');
			b_addchr(dest, SPECIAL_VAR_علامت);
#اگر_ختم
			advance = 1;
			توڑو;
#اگر_غیر_تعریف_شدہ __U_BOOT__
		حالت '#':
			b_adduint(dest,global_argc ? global_argc-1 : 0);
			advance = 1;
			توڑو;
#اگر_ختم
		حالت '{':
			b_addchr(dest, SPECIAL_VAR_علامت);
			ctx->بچہ->sp++;
			b_getch(داخلہ);
			/* XXX maybe someone will try to escape the '}' */
			جب(ch=b_getch(داخلہ),ch!=EOF && ch!='}') {
				b_addchr(dest,ch);
			}
			اگر (ch != '}') {
				syntax();
				واپس 1;
			}
			b_addchr(dest, SPECIAL_VAR_علامت);
			توڑو;
#اگر_غیر_تعریف_شدہ __U_BOOT__
		حالت '(':
			b_getch(داخلہ);
			process_command_subs(dest, ctx, داخلہ, ')');
			توڑو;
		حالت '*':
			sep[0]=ifs[0];
			برائے (i=1; i<global_argc; i++) {
				parse_string(dest, ctx, global_argv[i]);
				اگر (i+1 < global_argc) parse_string(dest, ctx, sep);
			}
			توڑو;
		حالت '@':
		حالت '-':
		حالت '_':
			/* still unhandled, but should be eventually */
			error_پیغام("unhandled syntax: $%c",ch);
			واپس 1;
			توڑو;
#اگر_ختم
		طےشدہ:
			b_addqchr(dest,'$',dest->quote);
	}
	/* Eat the character if the flag was set.  If the compiler
	 * is smart enough, we could substitute "b_getch(input);"
	 * for all the "advance = 1;" above, and also end up with
	 * a nice size-optimized program.  Hah!  That'll be the day.
	 */
	اگر (advance) b_getch(داخلہ);
	واپس 0;
}

#اگر_غیر_تعریف_شدہ __U_BOOT__
صحیح parse_string(o_string *dest, ساخت p_context *ctx, ثابت حرف *src)
{
	ساخت اندر_str foo;
	تیاری_string_اندر_str(&foo, src);
	واپس parse_دھارا(dest, ctx, &foo, '\0');
}
#اگر_ختم

/* return code is 0 for normal exit, 1 for syntax error */
ساکن صحیح parse_دھارا(o_string *dest, ساخت p_context *ctx,
			ساخت اندر_str *داخلہ, صحیح انتہا_trigger)
{
	غیرعلامتی صحیح ch, m;
#اگر_غیر_تعریف_شدہ __U_BOOT__
	صحیح redir_fd;
	redir_type redir_style;
#اگر_ختم
	صحیح اگلا;

	/* Only double-quote state is handled in the state variable dest->quote.
	 * A single-quote triggers a bypass of the main loop until its mate is
	 * found.  When recursing, quote state is passed in via dest->quote. */

	debug_printf("parse_stream, end_trigger=%d\n",انتہا_trigger);
	جب ((ch=b_getch(داخلہ))!=EOF) {
		m = map[ch];
#اگر_تعریف_شدہ __U_BOOT__
		اگر (داخلہ->__promptme == 0) واپس 1;
#اگر_ختم
		اگلا = (ch == '\n') ? 0 : b_peek(داخلہ);

		debug_printf("parse_stream: ch=%c (%d) m=%d quote=%d - %c\n",
			ch >= ' ' ? ch : '.', ch, m,
			dest->quote, ctx->ڈھیر == تہی ? '*' : '.');

		اگر (m==0 || ((m==1 || m==2) && dest->quote)) {
			b_addqchr(dest, ch, dest->quote);
		} ورنہ {
			اگر (m==2) {  /* unquoted IFS */
				اگر (مکمل_لفظ(dest, ctx)) {
					واپس 1;
				}
				/* If we aren't performing a substitution, treat a newline as a
				 * command separator.  */
				اگر (انتہا_trigger != '\0' && ch=='\n')
					مکمل_pipe(ctx,PIPE_SEQ);
			}
			اگر (ch == انتہا_trigger && !dest->quote && ctx->w==RES_NONE) {
				debug_printf("leaving parse_stream (triggered)\n");
				واپس 0;
			}
#اگر 0
			اگر (ch=='\n') {
				/* Yahoo!  Time to run with it! */
				مکمل_pipe(ctx,PIPE_SEQ);
				تسلسل_list(ctx->list_سر);
				initialize_context(ctx);
			}
#اگر_ختم
			اگر (m!=2) انتخاب (ch) {
		حالت '#':
			اگر (dest->لمبائی == 0 && !dest->quote) {
				جب(ch=b_peek(داخلہ),ch!=EOF && ch!='\n') { b_getch(داخلہ); }
			} ورنہ {
				b_addqchr(dest, ch, dest->quote);
			}
			توڑو;
		حالت '\\':
			اگر (اگلا == EOF) {
				syntax();
				واپس 1;
			}
			b_addqchr(dest, '\\', dest->quote);
			b_addqchr(dest, b_getch(داخلہ), dest->quote);
			توڑو;
		حالت '$':
			اگر (handle_dollar(dest, ctx, داخلہ)!=0) واپس 1;
			توڑو;
		حالت '\'':
			dest->nonnull = 1;
			جب(ch=b_getch(داخلہ),ch!=EOF && ch!='\'') {
#اگر_تعریف_شدہ __U_BOOT__
				اگر(داخلہ->__promptme == 0) واپس 1;
#اگر_ختم
				b_addchr(dest,ch);
			}
			اگر (ch==EOF) {
				syntax();
				واپس 1;
			}
			توڑو;
		حالت '"':
			dest->nonnull = 1;
			dest->quote = !dest->quote;
			توڑو;
#اگر_غیر_تعریف_شدہ __U_BOOT__
		حالت '`':
			process_command_subs(dest, ctx, داخلہ, '`');
			توڑو;
		حالت '>':
			redir_fd = redirect_opt_عدد(dest);
			مکمل_لفظ(dest, ctx);
			redir_style=REDIRECT_OVERWRITE;
			اگر (اگلا == '>') {
				redir_style=REDIRECT_APPEND;
				b_getch(داخلہ);
			} ورنہ اگر (اگلا == '(') {
				syntax();   /* until we support >(list) Process Substitution */
				واپس 1;
			}
			تیاری_redirect(ctx, redir_fd, redir_style, داخلہ);
			توڑو;
		حالت '<':
			redir_fd = redirect_opt_عدد(dest);
			مکمل_لفظ(dest, ctx);
			redir_style=REDIRECT_داخلہ;
			اگر (اگلا == '<') {
				redir_style=REDIRECT_HEREIS;
				b_getch(داخلہ);
			} ورنہ اگر (اگلا == '>') {
				redir_style=REDIRECT_IO;
				b_getch(داخلہ);
			} ورنہ اگر (اگلا == '(') {
				syntax();   /* until we support <(list) Process Substitution */
				واپس 1;
			}
			تیاری_redirect(ctx, redir_fd, redir_style, داخلہ);
			توڑو;
#اگر_ختم
		حالت ';':
			مکمل_لفظ(dest, ctx);
			مکمل_pipe(ctx,PIPE_SEQ);
			توڑو;
		حالت '&':
			مکمل_لفظ(dest, ctx);
			اگر (اگلا=='&') {
				b_getch(داخلہ);
				مکمل_pipe(ctx,PIPE_AND);
			} ورنہ {
#اگر_غیر_تعریف_شدہ __U_BOOT__
				مکمل_pipe(ctx,PIPE_BG);
#ورنہ
				syntax_err();
				واپس 1;
#اگر_ختم
			}
			توڑو;
		حالت '|':
			مکمل_لفظ(dest, ctx);
			اگر (اگلا=='|') {
				b_getch(داخلہ);
				مکمل_pipe(ctx,PIPE_OR);
			} ورنہ {
				/* we could pick up a file descriptor choice here
				 * with redirect_opt_num(), but bash doesn't do it.
				 * "echo foo 2| cat" yields "foo 2". */
#اگر_غیر_تعریف_شدہ __U_BOOT__
				مکمل_command(ctx);
#ورنہ
				syntax_err();
				واپس 1;
#اگر_ختم
			}
			توڑو;
#اگر_غیر_تعریف_شدہ __U_BOOT__
		حالت '(':
		حالت '{':
			اگر (parse_گروہ(dest, ctx, داخلہ, ch)!=0) واپس 1;
			توڑو;
		حالت ')':
		حالت '}':
			syntax();   /* Proper use of this character caught by end_trigger */
			واپس 1;
			توڑو;
#اگر_ختم
		حالت SUBSTED_VAR_علامت:
			dest->nonnull = 1;
			جب (ch = b_getch(داخلہ), ch != EOF &&
			    ch != SUBSTED_VAR_علامت) {
				debug_printf("subst, pass=%d\n", ch);
				اگر (داخلہ->__promptme == 0)
					واپس 1;
				b_addchr(dest, ch);
			}
			debug_printf("subst, term=%d\n", ch);
			اگر (ch == EOF) {
				syntax();
				واپس 1;
			}
			توڑو;
		طےشدہ:
			syntax();   /* this is really an internal logic error */
			واپس 1;
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
	اگر (انتہا_trigger != '\0') واپس -1;
	واپس 0;
}

ساکن خالی mapset(ثابت غیرعلامتی حرف *سیٹ, صحیح رمز)
{
	ثابت غیرعلامتی حرف *s;
	برائے (s=سیٹ; *s; s++) map[*s] = رمز;
}

ساکن خالی update_ifs_map(خالی)
{
	/* char *ifs and char map[256] are both globals. */
	ifs = (uchar *)env_get("IFS");
	اگر (ifs == تہی) ifs=(uchar *)" \t\n";
	/* Precompute a list of 'flow through' behavior so it can be treated
	 * quickly up front.  Computation is necessary because of IFS.
	 * Special case handling of IFS == " \t\n" is not implemented.
	 * The map[] array only really needs two bits each, and on most machines
	 * that would be faster because of the reduced L1 cache footprint.
	 */
	memset(map,0,حجم(map)); /* most characters flow through always */
#اگر_غیر_تعریف_شدہ __U_BOOT__
	mapset((uchar *)"\\$'\"`", 3);      /* never flow through */
	mapset((uchar *)"<>;&|(){}#", 1);   /* flow through if quoted */
#ورنہ
	{
		uchar subst[2] = {SUBSTED_VAR_علامت, 0};
		mapset(subst, 3);       /* never flow through */
	}
	mapset((uchar *)"\\$'\"", 3);       /* never flow through */
	mapset((uchar *)";&|#", 1);         /* flow through if quoted */
#اگر_ختم
	mapset(ifs, 2);            /* also flow through if quoted */
}

/* most recursion does not come through here, the exeception is
 * from builtin_source() */
ساکن صحیح parse_دھارا_outer(ساخت اندر_str *inp, صحیح flag)
{

	ساخت p_context ctx;
	o_string عارضی=NULL_O_STRING;
	صحیح rcode;
#اگر_تعریف_شدہ __U_BOOT__
	صحیح رمز = 1;
#اگر_ختم
	کرو {
		ctx.type = flag;
		initialize_context(&ctx);
		update_ifs_map();
		اگر (!(flag & FLAG_PARSE_SEMICOLON) || (flag & FLAG_REPARSING)) mapset((uchar *)";$&|", 0);
		inp->promptmode=1;
		rcode = parse_دھارا(&عارضی, &ctx, inp,
				     flag & FLAG_CONT_ON_NEWLINE ? -1 : '\n');
#اگر_تعریف_شدہ __U_BOOT__
		اگر (rcode == 1) flag_repeat = 0;
#اگر_ختم
		اگر (rcode != 1 && ctx.old_flag != 0) {
			syntax();
#اگر_تعریف_شدہ __U_BOOT__
			flag_repeat = 0;
#اگر_ختم
		}
		اگر (rcode != 1 && ctx.old_flag == 0) {
			مکمل_لفظ(&عارضی, &ctx);
			مکمل_pipe(&ctx,PIPE_SEQ);
#اگر_غیر_تعریف_شدہ __U_BOOT__
			تسلسل_list(ctx.list_سر);
#ورنہ
			رمز = تسلسل_list(ctx.list_سر);
			اگر (رمز == -2) {	/* exit */
				b_free(&عارضی);
				رمز = 0;
				/* XXX hackish way to not allow exit from main loop */
				اگر (inp->peek == file_peek) {
					لکھو("exit not allowed from main input shell.\n");
					جاری;
				}
				/*
				 * DANGER
				 * Return code -2 is special in this context,
				 * it indicates exit from inner pipe instead
				 * of return code itself, the return code is
				 * stored in 'last_return_code' variable!
				 * DANGER
				 */
				واپس -2;
			}
			اگر (رمز == -1)
			    flag_repeat = 0;
#اگر_ختم
		} ورنہ {
			اگر (ctx.old_flag != 0) {
				آزاد(ctx.ڈھیر);
				b_reset(&عارضی);
			}
#اگر_تعریف_شدہ __U_BOOT__
			اگر (inp->__promptme == 0) لکھو("<INTERRUPT>\n");
			inp->__promptme = 1;
#اگر_ختم
			عارضی.nonnull = 0;
			عارضی.quote = 0;
			inp->p = تہی;
			free_pipe_list(ctx.list_سر,0);
		}
		b_free(&عارضی);
	/* loop on syntax errors, return on EOF */
	} جب (rcode != -1 && !(flag & FLAG_EXIT_FROM_LOOP) &&
		(inp->peek != static_peek || b_peek(inp)));
#اگر_غیر_تعریف_شدہ __U_BOOT__
	واپس 0;
#ورنہ
	واپس (رمز != 0) ? 1 : 0;
#اگر_ختم /* __U_BOOT__ */
}

#اگر_غیر_تعریف_شدہ __U_BOOT__
ساکن صحیح parse_string_outer(ثابت حرف *s, صحیح flag)
#ورنہ
صحیح parse_string_outer(ثابت حرف *s, صحیح flag)
#اگر_ختم	/* __U_BOOT__ */
{
	ساخت اندر_str داخلہ;
	صحیح rcode;
#اگر_تعریف_شدہ __U_BOOT__
	حرف *p = تہی;
	اگر (!s)
		واپس 1;
	اگر (!*s)
		واپس 0;
	اگر (!(p = strchr(s, '\n')) || *++p) {
		p = xmalloc(لڑی_لمبائی(s) + 2);
		strcpy(p, s);
		strcat(p, "\n");
		تیاری_string_اندر_str(&داخلہ, p);
		rcode = parse_دھارا_outer(&داخلہ, flag);
		آزاد(p);
		واپس rcode == -2 ? آخری_return_رمز : rcode;
	} ورنہ {
#اگر_ختم
	تیاری_string_اندر_str(&داخلہ, s);
	rcode = parse_دھارا_outer(&داخلہ, flag);
	واپس rcode == -2 ? آخری_return_رمز : rcode;
#اگر_تعریف_شدہ __U_BOOT__
	}
#اگر_ختم
}

#اگر_غیر_تعریف_شدہ __U_BOOT__
ساکن صحیح parse_file_outer(FILE *f)
#ورنہ
صحیح parse_file_outer(خالی)
#اگر_ختم
{
	صحیح rcode;
	ساخت اندر_str داخلہ;
#اگر_غیر_تعریف_شدہ __U_BOOT__
	تیاری_file_اندر_str(&داخلہ, f);
#ورنہ
	تیاری_file_اندر_str(&داخلہ);
#اگر_ختم
	rcode = parse_دھارا_outer(&داخلہ, FLAG_PARSE_SEMICOLON);
	واپس rcode == -2 ? آخری_return_رمز : rcode;
}

#اگر_تعریف_شدہ __U_BOOT__
صحیح u_boot_hush_آغاز(خالی)
{
	اگر (چوٹی_vars == تہی) {
		چوٹی_vars = حافظہ_دو(حجم(ساخت variables));
		چوٹی_vars->name = "HUSH_VERSION";
		چوٹی_vars->قدر = "0.01";
		چوٹی_vars->اگلا = تہی;
		چوٹی_vars->flg_export = 0;
		چوٹی_vars->flg_read_only = 1;
	}
	واپس 0;
}

ساکن خالی *xmalloc(size_t سائز)
{
	خالی *p = تہی;

	اگر (!(p = حافظہ_دو(سائز))) {
	    لکھو("ERROR : xmalloc failed\n");
	    برائے(;;);
	}
	واپس p;
}

ساکن خالی *xrealloc(خالی *ptr, size_t سائز)
{
	خالی *p = تہی;

	اگر (!(p = realloc(ptr, سائز))) {
	    لکھو("ERROR : xrealloc failed\n");
	    برائے(;;);
	}
	واپس p;
}
#اگر_ختم /* __U_BOOT__ */

#اگر_غیر_تعریف_شدہ __U_BOOT__
/* Make sure we have a controlling tty.  If we get started under a job
 * aware app (like bash for example), make sure we are now in charge so
 * we don't fight over who gets the foreground */
ساکن خالی تیاری_کام_control(خالی)
{
	ساکن pid_t shell_pgrp;
	/* Loop until we are in the foreground.  */
	جب (tcgetpgrp (shell_terminal) != (shell_pgrp = getpgrp ()))
		kill (- shell_pgrp, SIGTTIN);

	/* Ignore interactive and job-control signals.  */
	اشارہ(SIGINT, SIG_IGN);
	اشارہ(SIGQUIT, SIG_IGN);
	اشارہ(SIGTERM, SIG_IGN);
	اشارہ(SIGTSTP, SIG_IGN);
	اشارہ(SIGTTIN, SIG_IGN);
	اشارہ(SIGTTOU, SIG_IGN);
	اشارہ(SIGCHLD, SIG_IGN);

	/* Put ourselves in our own process group.  */
	setsid();
	shell_pgrp = getpid ();
	setpgid (shell_pgrp, shell_pgrp);

	/* Grab control of the terminal.  */
	tcsetpgrp(shell_terminal, shell_pgrp);
}

صحیح hush_main(صحیح argc, حرف * ثابت *argv)
{
	صحیح opt;
	FILE *داخلہ;
	حرف **e = environ;

	/* XXX what should these be while sourcing /etc/profile? */
	global_argc = argc;
	global_argv = argv;

	/* (re?) initialize globals.  Sometimes hush_main() ends up calling
	 * hush_main(), therefore we cannot rely on the BSS to zero out this
	 * stuff.  Reset these to 0 every time. */
	ifs = تہی;
	/* map[] is taken care of with call to update_ifs_map() */
	fake_منوال = 0;
	interactive = 0;
	close_me_سر = تہی;
	آخری_bg_pid = 0;
	کام_list = تہی;
	آخری_jobid = 0;

	/* Initialize some more globals to non-zero values */
	سیٹ_cwd();
#اگر_تعریف_شدہ CONFIG_FEATURE_COMMAND_EDITING
	cmdedit_سیٹ_initial_prompt();
#ورنہ
	PS1 = تہی;
#اگر_ختم
	PS2 = "> ";

	/* initialize our shell local variables with the values
	 * currently living in the environment */
	اگر (e) {
		برائے (; *e; e++)
			سیٹ_local_var(*e, 2);   /* without call putenv() */
	}

	آخری_return_رمز=EXIT_SUCCESS;

	اگر (argv[0] && argv[0][0] == '-') {
		debug_printf("\nsourcing /etc/profile\n");
		اگر ((داخلہ = fopen("/etc/profile", "r")) != تہی) {
			mark_open(fileno(داخلہ));
			parse_file_outer(داخلہ);
			mark_closed(fileno(داخلہ));
			fclose(داخلہ);
		}
	}
	داخلہ=stdin;

	جب ((opt = getopt(argc, argv, "c:xif")) > 0) {
		انتخاب (opt) {
			حالت 'c':
				{
					global_argv = argv+optind;
					global_argc = argc-optind;
					opt = parse_string_outer(optarg, FLAG_PARSE_SEMICOLON);
					جاؤ final_return;
				}
				توڑو;
			حالت 'i':
				interactive++;
				توڑو;
			حالت 'f':
				fake_منوال++;
				توڑو;
			طےشدہ:
#اگر_غیر_تعریف_شدہ BB_VER
				fprintf(stderr, "Usage: sh [FILE]...\n"
						"   or: sh -c command [args]...\n\n");
				باہر_نکلو(EXIT_FAILURE);
#ورنہ
				show_usage();
#اگر_ختم
		}
	}
	/* A shell is interactive if the `-i' flag was given, or if all of
	 * the following conditions are met:
	 *	  no -c command
	 *    no arguments remaining or the -s flag given
	 *    standard input is a terminal
	 *    standard output is a terminal
	 *    Refer to Posix.2, the description of the `sh' utility. */
	اگر (argv[optind]==تہی && داخلہ==stdin &&
			isatty(fileno(stdin)) && isatty(fileno(stdout))) {
		interactive++;
	}

	debug_printf("\ninteractive=%d\n", interactive);
	اگر (interactive) {
		/* Looks like they want an interactive shell */
#اگر_غیر_تعریف_شدہ CONFIG_FEATURE_SH_EXTRA_QUIET
		لکھو( "\n\n" BB_BANNER " hush - the humble shell v0.01 (testing)\n");
		لکھو( "Enter 'help' for a list of built-in commands.\n\n");
#اگر_ختم
		تیاری_کام_control();
	}

	اگر (argv[optind]==تہی) {
		opt=parse_file_outer(stdin);
		جاؤ final_return;
	}

	debug_printf("\nrunning script '%s'\n", argv[optind]);
	global_argv = argv+optind;
	global_argc = argc-optind;
	داخلہ = xfopen(argv[optind], "r");
	opt = parse_file_outer(داخلہ);

#اگر_تعریف_شدہ CONFIG_FEATURE_CLEAN_اوپر
	fclose(داخلہ);
	اگر (cwd && cwd != unknown)
		آزاد((حرف*)cwd);
	{
		ساخت variables *cur, *tmp;
		برائے(cur = چوٹی_vars; cur; cur = tmp) {
			tmp = cur->اگلا;
			اگر (!cur->flg_read_only) {
				آزاد(cur->name);
				آزاد(cur->قدر);
				آزاد(cur);
			}
		}
	}
#اگر_ختم

final_return:
	واپس(opt?opt:آخری_return_رمز);
}
#اگر_ختم

ساکن حرف *داخل_کرو_var_قدر(حرف *inp)
{
	واپس داخل_کرو_var_قدر_sub(inp, 0);
}

ساکن حرف *داخل_کرو_var_قدر_sub(حرف *inp, صحیح لیبل_subst)
{
	صحیح res_str_len = 0;
	صحیح len;
	صحیح مکمل = 0;
	حرف *p, *p1, *res_str = تہی;

	جب ((p = strchr(inp, SPECIAL_VAR_علامت))) {
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
		p = strchr(inp, SPECIAL_VAR_علامت);
		*p = '\0';
		/* look up the value to substitute */
		اگر ((p1 = lookup_param(inp))) {
			اگر (لیبل_subst)
				len = res_str_len + لڑی_لمبائی(p1) + 2;
			ورنہ
				len = res_str_len + لڑی_لمبائی(p1);
			res_str = xrealloc(res_str, (1 + len));
			اگر (لیبل_subst) {
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
			} ورنہ
				/*
				 * copy the variable value to the result
				 * string
				 */
				strcpy((res_str + res_str_len), p1);

			res_str_len = len;
		}
		*p = SPECIAL_VAR_علامت;
		inp = ++p;
		مکمل = 1;
	}
	اگر (مکمل) {
		res_str = xrealloc(res_str, (1 + res_str_len + لڑی_لمبائی(inp)));
		strcpy((res_str + res_str_len), inp);
		جب ((p = strchr(res_str, '\n'))) {
			*p = ' ';
		}
	}
	واپس (res_str == تہی) ? inp : res_str;
}

ساکن حرف **make_list_اندر(حرف **inp, حرف *name)
{
	صحیح len, i;
	صحیح name_len = لڑی_لمبائی(name);
	صحیح n = 0;
	حرف **list;
	حرف *p1, *p2, *p3;

	/* create list of variable values */
	list = xmalloc(حجم(*list));
	برائے (i = 0; inp[i]; i++) {
		p3 = داخل_کرو_var_قدر(inp[i]);
		p1 = p3;
		جب (*p1) {
			اگر (*p1 == ' ') {
				p1++;
				جاری;
			}
			اگر ((p2 = strchr(p1, ' '))) {
				len = p2 - p1;
			} ورنہ {
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
		اگر (p3 != inp[i]) آزاد(p3);
	}
	list[n] = تہی;
	واپس list;
}

/*
 * Make new string for parser
 * inp     - array of argument strings to flatten
 * nonnull - indicates argument was quoted when originally parsed
 */
ساکن حرف *make_string(حرف **inp, صحیح *nonnull)
{
	حرف *p;
	حرف *str = تہی;
	صحیح n;
	صحیح len = 2;
	حرف *noeval_str;
	صحیح noeval = 0;

	noeval_str = get_local_var("HUSH_NO_EVAL");
	اگر (noeval_str != تہی && *noeval_str != '0' && *noeval_str != '\0')
		noeval = 1;
	برائے (n = 0; inp[n]; n++) {
		p = داخل_کرو_var_قدر_sub(inp[n], noeval);
		حرف *new_str = xrealloc(str, (len + لڑی_لمبائی(p) + (2 * nonnull[n])));
		اگر (!new_str) {
			آزاد(str);
			اگر (p != inp[n]) آزاد(p);
			واپس تہی;
		}
		str = new_str;
		اگر (n) {
			strcat(str, " ");
		} ورنہ {
			*str = '\0';
		}
		اگر (nonnull[n])
			strcat(str, "'");
		strcat(str, p);
		اگر (nonnull[n])
			strcat(str, "'");
		len = لڑی_لمبائی(str) + 3;
		اگر (p != inp[n]) آزاد(p);
	}
	len = لڑی_لمبائی(str);
	*(str + len) = '\n';
	*(str + len + 1) = '\0';
	واپس str;
}

#اگر_تعریف_شدہ __U_BOOT__
ساکن صحیح do_showvar(ساخت cmd_tbl *cmdtp, صحیح flag, صحیح argc,
		      حرف *ثابت argv[])
{
	صحیح i, k;
	صحیح rcode = 0;
	ساخت variables *cur;

	اگر (argc == 1) {		/* Print all env variables	*/
		برائے (cur = چوٹی_vars; cur; cur = cur->اگلا) {
			لکھو ("%s=%s\n", cur->name, cur->قدر);
			اگر (ctrlc ()) {
				puts ("\n ** Abort\n");
				واپس 1;
			}
		}
		واپس 0;
	}
	برائے (i = 1; i < argc; ++i) {	/* print single env variables	*/
		حرف *name = argv[i];

		k = -1;
		برائے (cur = چوٹی_vars; cur; cur = cur->اگلا) {
			اگر(strcmp (cur->name, name) == 0) {
				k = 0;
				لکھو ("%s=%s\n", cur->name, cur->قدر);
			}
			اگر (ctrlc ()) {
				puts ("\n ** Abort\n");
				واپس 1;
			}
		}
		اگر (k < 0) {
			لکھو ("## Error: \"%s\" not defined\n", name);
			rcode ++;
		}
	}
	واپس rcode;
}

U_BOOT_CMD(
	showvar, CONFIG_SYS_MAXARGS, 1,	do_showvar,
	"print local hushshell variables",
	"\n    - print values of all hushshell variables\n"
	"showvar name ...\n"
	"    - print value of hushshell variable 'name'"
);

#اگر_ختم
/****************************************************************************/
