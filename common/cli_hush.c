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

#وصف_ڪريو __U_BOOT__
#جيڪڏهن_وصف_ٿيل __U_BOOT__
#ضمن <env.h>
#ضمن <malloc.h>         /* malloc, free, realloc*/
#ضمن <linux/ctype.h>    /* isalpha, isdigit */
#ضمن <console.h>
#ضمن <bootretry.h>
#ضمن <cli.h>
#ضمن <cli_hush.h>
#ضمن <command.h>        /* find_cmd */
#ضمن <vsprintf.h>
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
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
#وصف_ڪريو applet_name "hush"
#ضمن "standalone.h"
#وصف_ڪريو hush_main مکيه
#وصف_ڪريو BB_BANNER
#جيڪڏهن_پورو
#جيڪڏهن_پورو
#وصف_ڪريو SPECIAL_VAR_نشان 03
#وصف_ڪريو SUBSTED_VAR_نشان 04
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
#وصف_ڪريو FLAG_EXIT_FROM_LOOP 1
#وصف_ڪريو FLAG_PARSE_SEMICOLON (1 << 1)		/* symbol ';' is special for parser */
#وصف_ڪريو FLAG_REPARSING       (1 << 2)		/* >= 2nd pass */

#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل __U_BOOT__
#وصف_ڪريو EXIT_SUCCESS 0
#وصف_ڪريو EOF -1
#وصف_ڪريو syntax() syntax_err()
#وصف_ڪريو xstrdup strdup
#وصف_ڪريو error_نياپو لکو
#وإلا
تعريفنوع تعداد {
	REDIRECT_داخلا     = 1,
	REDIRECT_OVERWRITE = 2,
	REDIRECT_APPEND    = 3,
	REDIRECT_HEREIS    = 4,
	REDIRECT_IO        = 5
} redir_type;

/* The descrip member of this structure is only used to make debugging
 * output pretty */
بنية {صحيح گهڻائي; صحيح default_fd; حرف *descrip;} redir_جدول[] = {
	{ 0,                         0, "()" },
	{ O_RDONLY,                  0, "<"  },
	{ O_CREAT|O_TRUNC|O_WRONLY,  1, ">"  },
	{ O_CREAT|O_APPEND|O_WRONLY, 1, ">>" },
	{ O_RDONLY,                 -1, "<<" },
	{ O_RDWR,                    1, "<>" }
};
#جيڪڏهن_پورو

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
	RES_پورو  = 10,
	RES_XXXX  = 11,
	RES_اندريون    = 12,
	RES_SNTX  = 13
} reserved_style;
#وصف_ڪريو FLAG_پڇاڙي   (1<<RES_NONE)
#وصف_ڪريو FLAG_IF    (1<<RES_IF)
#وصف_ڪريو FLAG_THEN  (1<<RES_THEN)
#وصف_ڪريو FLAG_ELIF  (1<<RES_ELIF)
#وصف_ڪريو FLAG_ELSE  (1<<RES_ELSE)
#وصف_ڪريو FLAG_FI    (1<<RES_FI)
#وصف_ڪريو FLAG_FOR   (1<<RES_FOR)
#وصف_ڪريو FLAG_WHILE (1<<RES_WHILE)
#وصف_ڪريو FLAG_UNTIL (1<<RES_UNTIL)
#وصف_ڪريو FLAG_DO    (1<<RES_DO)
#وصف_ڪريو FLAG_پورو  (1<<RES_پورو)
#وصف_ڪريو FLAG_اندريون    (1<<RES_اندريون)
#وصف_ڪريو FLAG_شروعات (1<<RES_XXXX)

/* This holds pointers to the various results of parsing */
بنية p_context {
	بنية ٻار_prog *ٻار;
	بنية pipe *list_سر;
	بنية pipe *pipe;
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
	بنية redir_struct *pending_redirect;
#جيڪڏهن_پورو
	reserved_style w;
	صحيح old_flag;				/* for figuring out valid reserved words */
	بنية p_context *ڍير;
	صحيح type;			/* define type of parser : ";$" common or special symbol */
	/* How about quoting status? */
};

#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
بنية redir_struct {
	redir_type type;			/* type of redirection */
	صحيح fd;						/* file descriptor being redirected */
	صحيح dup;					/* -1, or file descriptor being duplicated */
	بنية redir_struct *ايندڙ;	/* pointer to the next redirect in the list */
	glob_t لفظ;				/* *word.gl_pathv is the filename */
};
#جيڪڏهن_پورو

بنية ٻار_prog {
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
	pid_t pid;					/* 0 if exited */
#جيڪڏهن_پورو
	حرف **argv;				/* program name and arguments */
	/* was quoted when parsed; copy of struct o_string.nonnull field */
	صحيح *argv_nonnull;
#جيڪڏهن_وصف_ٿيل __U_BOOT__
	صحيح    argc;                            /* number of program arguments */
#جيڪڏهن_پورو
	بنية pipe *گروهه;			/* if non-NULL, first in group or subshell */
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
	صحيح subshell;				/* flag, non-zero if group must be forked */
	بنية redir_struct *redirects;	/* I/O redirections */
	glob_t glob_نتيجو;			/* result of parameter globbing */
	صحيح is_stopped;				/* is the program currently running? */
	بنية pipe *family;		/* pointer back to the child's parent pipe */
#جيڪڏهن_پورو
	صحيح sp;				/* number of SPECIAL_VAR_SYMBOL */
	صحيح type;
};

بنية pipe {
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
	صحيح jobid;					/* job number */
#جيڪڏهن_پورو
	صحيح انگ_progs;				/* total number of programs in job */
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
	صحيح running_progs;			/* number of programs running */
	حرف *متن;					/* name of job */
	حرف *cmdbuf;				/* buffer various argv's point into */
	pid_t pgrp;					/* process group ID for the job */
#جيڪڏهن_پورو
	بنية ٻار_prog *progs;	/* array of commands in pipe */
	بنية pipe *ايندڙ;			/* to track background commands */
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
	صحيح stopped_progs;			/* number of programs alive, but stopped */
	صحيح ڪم_context;			/* bitmask defining current context */
#جيڪڏهن_پورو
	pipe_style followup;		/* PIPE_BG, PIPE_SEQ, PIPE_OR, PIPE_AND */
	reserved_style r_گهڻائي;		/* supports if, for, while, until */
};

#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
بنية close_me {
	صحيح fd;
	بنية close_me *ايندڙ;
};
#جيڪڏهن_پورو

بنية variables {
	حرف *name;
	حرف *قدر;
	صحيح flg_export;
	صحيح flg_read_only;
	بنية variables *ايندڙ;
};

/* globals, connect us to the outside world
 * the first three support $?, $#, and $1 */
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
حرف **global_argv;
بنانشان صحيح global_argc;
#جيڪڏهن_پورو
ساكن بنانشان صحيح آخري_return_ڪوڊ;
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
خارجي حرف **environ; /* This is in <unistd.h>, but protected with __USE_GNU */
#جيڪڏهن_پورو

/* "globals" within this file */
ساكن uchar *ifs;
ساكن حرف map[256];
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
ساكن صحيح fake_گهڻائي;
ساكن صحيح interactive;
ساكن بنية close_me *close_me_سر;
ساكن ثابت حرف *cwd;
ساكن بنية pipe *ڪم_list;
ساكن بنانشان صحيح آخري_bg_pid;
ساكن بنانشان صحيح آخري_jobid;
ساكن بنانشان صحيح shell_terminal;
ساكن حرف *PS1;
ساكن حرف *PS2;
بنية variables shell_ver = { "HUSH_VERSION", "0.01", 1, 1, 0 };
بنية variables *چوٽي_vars = &shell_ver;
#وإلا
ساكن صحيح flag_repeat = 0;
ساكن صحيح do_repeat = 0;
ساكن بنية variables *چوٽي_vars = عدم ;
#جيڪڏهن_پورو /*__U_BOOT__ */

#وصف_ڪريو B_CHUNK (100)
#وصف_ڪريو B_NOSPAC 1

تعريفنوع بنية {
	حرف *معلومات;
	صحيح ڊيگهائي;
	صحيح maxlen;
	صحيح quote;
	صحيح nonnull;
} o_string;
#وصف_ڪريو NULL_O_STRING {عدم,0,0,0,0}
/* used for initialization:
	o_string foo = NULL_O_STRING; */

/* I can almost use ordinary FILE *.  Is open_memstream() universally
 * available?  Where is it documented? */
بنية اندريون_str {
	ثابت بنانشان حرف *p;
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
	حرف peek_buf[2];
#جيڪڏهن_پورو
	صحيح __promptme;
	صحيح promptmode;
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
	FILE *file;
#جيڪڏهن_پورو
	صحيح (*get) (بنية اندريون_str *);
	صحيح (*peek) (بنية اندريون_str *);
};
#وصف_ڪريو b_getch(داخلا) ((داخلا)->get(داخلا))
#وصف_ڪريو b_peek(داخلا) ((داخلا)->peek(داخلا))

#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
#وصف_ڪريو ڪم_حالت_FORMAT "[%d] %-22s %.40s\n"

بنية built_اندريون_command {
	حرف *cmd;					/* name */
	حرف *descr;				/* description */
	صحيح (*function) (بنية ٻار_prog *);	/* function ptr */
};
#جيڪڏهن_پورو

/* define DEBUG_SHELL for debugging output (obviously ;-)) */
#إذا 0
#وصف_ڪريو DEBUG_SHELL
#جيڪڏهن_پورو

/* This should be in utility.c */
#جيڪڏهن_وصف_ٿيل DEBUG_SHELL
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
ساكن فراغ debug_printf(ثابت حرف *format, ...)
{
	va_list دليلون;
	va_شروعات(دليلون, format);
	vfprintf(stderr, format, دليلون);
	va_پڇاڙي(دليلون);
}
#وإلا
#وصف_ڪريو debug_printf(fmt,دليلون...)	لکو (fmt ,##args)
#جيڪڏهن_پورو
#وإلا
ساكن مضمن فراغ debug_printf(ثابت حرف *format, ...) { }
#جيڪڏهن_پورو
#وصف_ڪريو final_printf debug_printf

#جيڪڏهن_وصف_ٿيل __U_BOOT__
ساكن فراغ syntax_err(فراغ) {
	 لکو("syntax error\n");
}
#وإلا
ساكن فراغ __syntax(حرف *file, صحيح سٽ) {
	error_نياپو("syntax error %s:%d", file, سٽ);
}
#وصف_ڪريو syntax() __syntax(__FILE__, __LINE__)
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل __U_BOOT__
ساكن فراغ *xmalloc(size_t سائيز);
ساكن فراغ *xrealloc(فراغ *ptr, size_t سائيز);
#وإلا
/* Index of subroutines: */
/*   function prototypes for builtins */
ساكن صحيح builtin_cd(بنية ٻار_prog *ٻار);
ساكن صحيح builtin_env(بنية ٻار_prog *ٻار);
ساكن صحيح builtin_eval(بنية ٻار_prog *ٻار);
ساكن صحيح builtin_exec(بنية ٻار_prog *ٻار);
ساكن صحيح builtin_exit(بنية ٻار_prog *ٻار);
ساكن صحيح builtin_export(بنية ٻار_prog *ٻار);
ساكن صحيح builtin_fg_bg(بنية ٻار_prog *ٻار);
ساكن صحيح builtin_help(بنية ٻار_prog *ٻار);
ساكن صحيح builtin_jobs(بنية ٻار_prog *ٻار);
ساكن صحيح builtin_pwd(بنية ٻار_prog *ٻار);
ساكن صحيح builtin_read(بنية ٻار_prog *ٻار);
ساكن صحيح builtin_سيٽ(بنية ٻار_prog *ٻار);
ساكن صحيح builtin_سرڪايو(بنية ٻار_prog *ٻار);
ساكن صحيح builtin_ذريعو(بنية ٻار_prog *ٻار);
ساكن صحيح builtin_umask(بنية ٻار_prog *ٻار);
ساكن صحيح builtin_unset(بنية ٻار_prog *ٻار);
ساكن صحيح builtin_not_written(بنية ٻار_prog *ٻار);
#جيڪڏهن_پورو
/*   o_string manipulation: */
ساكن صحيح b_check_space(o_string *o, صحيح len);
ساكن صحيح b_addchr(o_string *o, صحيح ch);
ساكن فراغ b_reset(o_string *o);
ساكن صحيح b_addqchr(o_string *o, صحيح ch, صحيح quote);
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
ساكن صحيح b_adduint(o_string *o, بنانشان صحيح i);
#جيڪڏهن_پورو
/*  in_str manipulations: */
ساكن صحيح static_get(بنية اندريون_str *i);
ساكن صحيح static_peek(بنية اندريون_str *i);
ساكن صحيح file_get(بنية اندريون_str *i);
ساكن صحيح file_peek(بنية اندريون_str *i);
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
ساكن فراغ تياري_file_اندريون_str(بنية اندريون_str *i, FILE *f);
#وإلا
ساكن فراغ تياري_file_اندريون_str(بنية اندريون_str *i);
#جيڪڏهن_پورو
ساكن فراغ تياري_string_اندريون_str(بنية اندريون_str *i, ثابت حرف *s);
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
/*  close_me manipulations: */
ساكن فراغ mark_open(صحيح fd);
ساكن فراغ mark_closed(صحيح fd);
ساكن فراغ close_all(فراغ);
#جيڪڏهن_پورو
/*  "run" the final data structures: */
ساكن حرف *indenter(صحيح i);
ساكن صحيح free_pipe_list(بنية pipe *سر, صحيح indent);
ساكن صحيح free_pipe(بنية pipe *pi, صحيح indent);
/*  really run the final data structures: */
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
ساكن صحيح تياري_redirects(بنية ٻار_prog *prog, صحيح squirrel[]);
#جيڪڏهن_پورو
ساكن صحيح لاڳيتو_list_real(بنية pipe *pi);
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
ساكن فراغ pseudo_exec(بنية ٻار_prog *ٻار) __attribute__ ((noreturn));
#جيڪڏهن_پورو
ساكن صحيح لاڳيتو_pipe_real(بنية pipe *pi);
/*   extended glob support: */
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
ساكن صحيح globhack(ثابت حرف *src, صحيح flags, glob_t *pglob);
ساكن صحيح glob_needed(ثابت حرف *s);
ساكن صحيح xglob(o_string *dest, صحيح flags, glob_t *pglob);
#جيڪڏهن_پورو
/*   variable assignment: */
ساكن صحيح is_assignment(ثابت حرف *s);
/*   data structure manipulation: */
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
ساكن صحيح تياري_redirect(بنية p_context *ctx, صحيح fd, redir_type style, بنية اندريون_str *داخلا);
#جيڪڏهن_پورو
ساكن فراغ initialize_context(بنية p_context *ctx);
ساكن صحيح پورو_لفظ(o_string *dest, بنية p_context *ctx);
ساكن صحيح پورو_command(بنية p_context *ctx);
ساكن صحيح پورو_pipe(بنية p_context *ctx, pipe_style type);
/*   primary string parsing: */
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
ساكن صحيح redirect_dup_انگ(بنية اندريون_str *داخلا);
ساكن صحيح redirect_opt_انگ(o_string *o);
ساكن صحيح process_command_subs(o_string *dest, بنية p_context *ctx, بنية اندريون_str *داخلا, صحيح subst_پڇاڙي);
ساكن صحيح parse_گروهه(o_string *dest, بنية p_context *ctx, بنية اندريون_str *داخلا, صحيح ch);
#جيڪڏهن_پورو
ساكن حرف *lookup_param(حرف *src);
ساكن حرف *make_string(حرف **inp, صحيح *nonnull);
ساكن صحيح handle_dollar(o_string *dest, بنية p_context *ctx, بنية اندريون_str *داخلا);
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
ساكن صحيح parse_string(o_string *dest, بنية p_context *ctx, ثابت حرف *src);
#جيڪڏهن_پورو
ساكن صحيح parse_وهڪرو(o_string *dest, بنية p_context *ctx, بنية اندريون_str *input0, صحيح پڇاڙي_trigger);
/*   setup: */
ساكن صحيح parse_وهڪرو_outer(بنية اندريون_str *inp, صحيح flag);
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
ساكن صحيح parse_string_outer(ثابت حرف *s, صحيح flag);
ساكن صحيح parse_file_outer(FILE *f);
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
/*   job management: */
ساكن صحيح checkjobs(بنية pipe* fg_pipe);
ساكن فراغ وجهو_bg_ڪم(بنية pipe *pi);
ساكن فراغ remove_bg_ڪم(بنية pipe *pi);
#جيڪڏهن_پورو
/*     local variable support */
ساكن حرف **make_list_اندريون(حرف **inp, حرف *name);
ساكن حرف *وجهو_var_قدر(حرف *inp);
ساكن حرف *وجهو_var_قدر_sub(حرف *inp, صحيح ٽيگ_subst);

#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
/* Table of built-in functions.  They can be forked or not, depending on
 * context: within pipes, they fork.  As simple commands, they do not.
 * When used in non-forking context, they can change global variables
 * in the parent shell process.  If forked, of course they can not.
 * For example, 'unset foo | whatever' will parse and run, but foo will
 * still be set at the end. */
ساكن بنية built_اندريون_command bltins[] = {
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
	{"set", "Set/unset shell local variables", builtin_سيٽ},
	{"shift", "Shift positional parameters", builtin_سرڪايو},
	{"trap", "Trap signals", builtin_not_written},
	{"ulimit","Controls resource limits", builtin_not_written},
	{"umask","Sets file creation mask", builtin_umask},
	{"unset", "Unset environment variable", builtin_unset},
	{".", "Source-in and run commands in a file", builtin_ذريعو},
	{"help", "List shell built-in commands", builtin_help},
	{عدم, عدم, عدم}
};

ساكن ثابت حرف *سيٽ_cwd(فراغ)
{
	إذا(cwd==unknown)
		cwd = عدم;     /* xgetcwd(arg) called free(arg) */
	cwd = xgetcwd((حرف *)cwd);
	إذا (!cwd)
		cwd = unknown;
	أرجع cwd;
}

/* built-in 'eval' handler */
ساكن صحيح builtin_eval(بنية ٻار_prog *ٻار)
{
	حرف *str = عدم;
	صحيح rcode = EXIT_SUCCESS;

	إذا (ٻار->argv[1]) {
		str = make_string(ٻار->argv + 1);
		parse_string_outer(str, FLAG_EXIT_FROM_LOOP |
					FLAG_PARSE_SEMICOLON);
		حرر(str);
		rcode = آخري_return_ڪوڊ;
	}
	أرجع rcode;
}

/* built-in 'cd <path>' handler */
ساكن صحيح builtin_cd(بنية ٻار_prog *ٻار)
{
	حرف *newdir;
	إذا (ٻار->argv[1] == عدم)
		newdir = env_get("HOME");
	وإلا
		newdir = ٻار->argv[1];
	إذا (chdir(newdir)) {
		لکو("cd: %s: %s\n", newdir, strerror(errno));
		أرجع EXIT_FAILURE;
	}
	سيٽ_cwd();
	أرجع EXIT_SUCCESS;
}

/* built-in 'env' handler */
ساكن صحيح builtin_env(بنية ٻار_prog *dummy)
{
	حرف **e = environ;
	إذا (e == عدم) أرجع EXIT_FAILURE;
	لكل (; *e; e++) {
		puts(*e);
	}
	أرجع EXIT_SUCCESS;
}

/* built-in 'exec' handler */
ساكن صحيح builtin_exec(بنية ٻار_prog *ٻار)
{
	إذا (ٻار->argv[1] == عدم)
		أرجع EXIT_SUCCESS;   /* Really? */
	ٻار->argv++;
	pseudo_exec(ٻار);
	/* never returns */
}

/* built-in 'exit' handler */
ساكن صحيح builtin_exit(بنية ٻار_prog *ٻار)
{
	إذا (ٻار->argv[1] == عدم)
		ٻاهر_نڪرو(آخري_return_ڪوڊ);
	ٻاهر_نڪرو (atoi(ٻار->argv[1]));
}

/* built-in 'export VAR=value' handler */
ساكن صحيح builtin_export(بنية ٻار_prog *ٻار)
{
	صحيح res = 0;
	حرف *name = ٻار->argv[1];

	إذا (name == عدم) {
		أرجع (builtin_env(ٻار));
	}

	name = strdup(name);

	إذا(name) {
		حرف *قدر = strchr(name, '=');

		إذا (!قدر) {
			حرف *tmp;
			/* They are exporting something without an =VALUE */

			قدر = get_local_var(name);
			إذا (قدر) {
				size_t ln = لڙي_ڊيگهه(name);

				tmp = realloc(name, ln+لڙي_ڊيگهه(قدر)+2);
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
		perror_نياپو("export");
	وإلا إذا(res==0)
		res = سيٽ_local_var(name, 1);
	وإلا
		res = 0;
	حرر(name);
	أرجع res;
}

/* built-in 'fg' and 'bg' handler */
ساكن صحيح builtin_fg_bg(بنية ٻار_prog *ٻار)
{
	صحيح i, jobnum;
	بنية pipe *pi=عدم;

	إذا (!interactive)
		أرجع EXIT_FAILURE;
	/* If they gave us no args, assume they want the last backgrounded task */
	إذا (!ٻار->argv[1]) {
		لكل (pi = ڪم_list; pi; pi = pi->ايندڙ) {
			إذا (pi->jobid == آخري_jobid) {
				اكسر;
			}
		}
		إذا (!pi) {
			error_نياپو("%s: no current job", ٻار->argv[0]);
			أرجع EXIT_FAILURE;
		}
	} وإلا {
		إذا (لڙي_مان_پڙهو(ٻار->argv[1], "%%%d", &jobnum) != 1) {
			error_نياپو("%s: bad argument '%s'", ٻار->argv[0], ٻار->argv[1]);
			أرجع EXIT_FAILURE;
		}
		لكل (pi = ڪم_list; pi; pi = pi->ايندڙ) {
			إذا (pi->jobid == jobnum) {
				اكسر;
			}
		}
		إذا (!pi) {
			error_نياپو("%s: %d: no such job", ٻار->argv[0], jobnum);
			أرجع EXIT_FAILURE;
		}
	}

	إذا (*ٻار->argv[0] == 'f') {
		/* Put the job into the foreground.  */
		tcsetpgrp(shell_terminal, pi->pgrp);
	}

	/* Restart the processes in the job */
	لكل (i = 0; i < pi->انگ_progs; i++)
		pi->progs[i].is_stopped = 0;

	إذا ( (i=kill(- pi->pgrp, SIGCONT)) < 0) {
		إذا (i == ESRCH) {
			remove_bg_ڪم(pi);
		} وإلا {
			perror_نياپو("kill (SIGCONT)");
		}
	}

	pi->stopped_progs = 0;
	أرجع EXIT_SUCCESS;
}

/* built-in 'help' handler */
ساكن صحيح builtin_help(بنية ٻار_prog *dummy)
{
	بنية built_اندريون_command *x;

	لکو("\nBuilt-in commands:\n");
	لکو("-------------------\n");
	لكل (x = bltins; x->cmd; x++) {
		إذا (x->descr==عدم)
			استمر;
		لکو("%s\t%s\n", x->cmd, x->descr);
	}
	لکو("\n\n");
	أرجع EXIT_SUCCESS;
}

/* built-in 'jobs' handler */
ساكن صحيح builtin_jobs(بنية ٻار_prog *ٻار)
{
	بنية pipe *ڪم;
	حرف *حالت_string;

	لكل (ڪم = ڪم_list; ڪم; ڪم = ڪم->ايندڙ) {
		إذا (ڪم->running_progs == ڪم->stopped_progs)
			حالت_string = "Stopped";
		وإلا
			حالت_string = "Running";

		لکو(ڪم_حالت_FORMAT, ڪم->jobid, حالت_string, ڪم->متن);
	}
	أرجع EXIT_SUCCESS;
}

/* built-in 'pwd' handler */
ساكن صحيح builtin_pwd(بنية ٻار_prog *dummy)
{
	puts(سيٽ_cwd());
	أرجع EXIT_SUCCESS;
}

/* built-in 'read VAR' handler */
ساكن صحيح builtin_read(بنية ٻار_prog *ٻار)
{
	صحيح res;

	إذا (ٻار->argv[1]) {
		حرف string[BUFSIZ];
		حرف *var = 0;

		string[0] = 0;  /* In case stdin has only EOF */
		/* read string */
		fgets(string, حجم(string), stdin);
		chomp(string);
		var = يادگيري_ڏيو(لڙي_ڊيگهه(ٻار->argv[1])+لڙي_ڊيگهه(string)+2);
		إذا(var) {
			sprintf(var, "%s=%s", ٻار->argv[1], string);
			res = سيٽ_local_var(var, 0);
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
ساكن صحيح builtin_سيٽ(بنية ٻار_prog *ٻار)
{
	حرف *عارضي = ٻار->argv[1];
	بنية variables *e;

	إذا (عارضي == عدم)
		لكل(e = چوٽي_vars; e; e=e->ايندڙ)
			لکو("%s=%s\n", e->name, e->قدر);
	وإلا
		سيٽ_local_var(عارضي, 0);

		أرجع EXIT_SUCCESS;
}

/* Built-in 'shift' handler */
ساكن صحيح builtin_سرڪايو(بنية ٻار_prog *ٻار)
{
	صحيح n=1;
	إذا (ٻار->argv[1]) {
		n=atoi(ٻار->argv[1]);
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
ساكن صحيح builtin_ذريعو(بنية ٻار_prog *ٻار)
{
	FILE *داخلا;
	صحيح حالت;

	إذا (ٻار->argv[1] == عدم)
		أرجع EXIT_FAILURE;

	/* XXX search through $PATH is missing */
	داخلا = fopen(ٻار->argv[1], "r");
	إذا (!داخلا) {
		error_نياپو("Couldn't open file '%s'", ٻار->argv[1]);
		أرجع EXIT_FAILURE;
	}

	/* Now run the file */
	/* XXX argv and argc are broken; need to save old global_argv
	 * (pointer only is OK!) on this stack frame,
	 * set global_argv=child->argv+1, recurse, and restore. */
	mark_open(fileno(داخلا));
	حالت = parse_file_outer(داخلا);
	mark_closed(fileno(داخلا));
	fclose(داخلا);
	أرجع (حالت);
}

ساكن صحيح builtin_umask(بنية ٻار_prog *ٻار)
{
	mode_t new_umask;
	ثابت حرف *arg = ٻار->argv[1];
	حرف *پڇاڙي_2;
	إذا (arg) {
		new_umask=strtoul(arg, &پڇاڙي_2, 8);
		إذا (*پڇاڙي_2!='\0' || پڇاڙي_2 == arg) {
			أرجع EXIT_FAILURE;
		}
	} وإلا {
		لکو("%.3o\n", (بنانشان صحيح) (new_umask=umask(0)));
	}
	umask(new_umask);
	أرجع EXIT_SUCCESS;
}

/* built-in 'unset VAR' handler */
ساكن صحيح builtin_unset(بنية ٻار_prog *ٻار)
{
	/* bash returned already true */
	unset_local_var(ٻار->argv[1]);
	أرجع EXIT_SUCCESS;
}

ساكن صحيح builtin_not_written(بنية ٻار_prog *ٻار)
{
	لکو("builtin_%s not written\n",ٻار->argv[0]);
	أرجع EXIT_FAILURE;
}
#جيڪڏهن_پورو

ساكن صحيح b_check_space(o_string *o, صحيح len)
{
	/* It would be easy to drop a more restrictive policy
	 * in here, such as setting a maximum string length */
	إذا (o->ڊيگهائي + len > o->maxlen) {
		حرف *old_معلومات = o->معلومات;
		/* assert (data == NULL || o->maxlen != 0); */
		o->maxlen += max(2*len, B_CHUNK);
		o->معلومات = realloc(o->معلومات, 1 + o->maxlen);
		إذا (o->معلومات == عدم) {
			حرر(old_معلومات);
		}
	}
	أرجع o->معلومات == عدم;
}

ساكن صحيح b_addchr(o_string *o, صحيح ch)
{
	debug_printf("b_addchr: %c %d %p\n", ch, o->ڊيگهائي, o);
	إذا (b_check_space(o, 1)) أرجع B_NOSPAC;
	o->معلومات[o->ڊيگهائي] = ch;
	o->ڊيگهائي++;
	o->معلومات[o->ڊيگهائي] = '\0';
	أرجع 0;
}

ساكن فراغ b_reset(o_string *o)
{
	o->ڊيگهائي = 0;
	o->nonnull = 0;
	إذا (o->معلومات != عدم) *o->معلومات = '\0';
}

ساكن فراغ b_free(o_string *o)
{
	b_reset(o);
	حرر(o->معلومات);
	o->معلومات = عدم;
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

#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
ساكن صحيح b_adduint(o_string *o, بنانشان صحيح i)
{
	صحيح r;
	حرف *p = simple_itoa(i);
	/* no escape checking necessary */
	افعل r=b_addchr(o, *p++); طالما (r==0 && *p);
	أرجع r;
}
#جيڪڏهن_پورو

ساكن صحيح static_get(بنية اندريون_str *i)
{
	صحيح ch = *i->p++;
	إذا (ch=='\0') أرجع EOF;
	أرجع ch;
}

ساكن صحيح static_peek(بنية اندريون_str *i)
{
	أرجع *i->p;
}

#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
ساكن مضمن فراغ cmdedit_سيٽ_initial_prompt(فراغ)
{
#جيڪڏهن_وصف_نه_ٿيل CONFIG_FEATURE_SH_FANCY_PROMPT
	PS1 = عدم;
#وإلا
	PS1 = env_get("PS1");
	إذا(PS1==0)
		PS1 = "\\w \\$ ";
#جيڪڏهن_پورو
}

ساكن مضمن فراغ تياري_prompt_string(صحيح promptmode, حرف **prompt_str)
{
	debug_printf("setup_prompt_string %d ",promptmode);
#جيڪڏهن_وصف_نه_ٿيل CONFIG_FEATURE_SH_FANCY_PROMPT
	/* Set up the prompt */
	إذا (promptmode == 1) {
		حرر(PS1);
		PS1=xmalloc(لڙي_ڊيگهه(cwd)+4);
		sprintf(PS1, "%s %s", cwd, ( geteuid() != 0 ) ?  "$ ":"# ");
		*prompt_str = PS1;
	} وإلا {
		*prompt_str = PS2;
	}
#وإلا
	*prompt_str = (promptmode==1)? PS1 : PS2;
#جيڪڏهن_پورو
	debug_printf("result %s\n",*prompt_str);
}
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل __U_BOOT__
ساكن صحيح uboot_cli_readline(بنية اندريون_str *i)
{
	حرف *prompt;
	حرف __maybe_unused *ps_prompt = عدم;

	إذا (i->promptmode == 1)
		prompt = CONFIG_SYS_PROMPT;
	وإلا
		prompt = CONFIG_SYS_PROMPT_HUSH_PS2;

#جيڪڏهن_وصف_ٿيل CONFIG_CMDLINE_PS_SUPPORT
	إذا (i->promptmode == 1)
		ps_prompt = env_get("PS1");
	وإلا
		ps_prompt = env_get("PS2");
	إذا (ps_prompt)
		prompt = ps_prompt;
#جيڪڏهن_پورو

	أرجع cli_readline(prompt);
}
#جيڪڏهن_پورو

ساكن فراغ get_user_داخلا(بنية اندريون_str *i)
{
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
	حرف *prompt_str;
	ساكن حرف the_command[BUFSIZ];

	تياري_prompt_string(i->promptmode, &prompt_str);
#جيڪڏهن_وصف_ٿيل CONFIG_FEATURE_COMMAND_EDITING
	/*
	 ** enable command line editing only while a command line
	 ** is actually being read; otherwise, we'll end up bequeathing
	 ** atexit() handlers and other unwanted stuff to our
	 ** child processes (rob@sysgo.de)
	 */
	cmdedit_read_داخلا(prompt_str, the_command);
#وإلا
	fputs(prompt_str, stdout);
	fflush(stdout);
	the_command[0]=fgetc(i->file);
	the_command[1]='\0';
#جيڪڏهن_پورو
	fflush(stdout);
	i->p = the_command;
#وإلا
	صحيح n;
	ساكن حرف the_command[CONFIG_SYS_CBSIZE + 1];

	bootretry_reset_cmd_timeout();
	i->__promptme = 1;
	n = uboot_cli_readline(i);

#جيڪڏهن_وصف_ٿيل CONFIG_BOOT_RETRY_وقت
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
#جيڪڏهن_پورو
	إذا (n == -1 ) {
		flag_repeat = 0;
		i->__promptme = 0;
	}
	n = لڙي_ڊيگهه(console_بفر);
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
			إذا (لڙي_ڊيگهه(the_command) + لڙي_ڊيگهه(console_بفر)
			    < CONFIG_SYS_CBSIZE) {
				n = لڙي_ڊيگهه(the_command);
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
#جيڪڏهن_پورو
}

/* This is the magic location that prints prompts
 * and gets data back from the user */
ساكن صحيح file_get(بنية اندريون_str *i)
{
	صحيح ch;

	ch = 0;
	/* If there is data waiting, eat it up */
	إذا (i->p && *i->p) {
		ch = *i->p++;
	} وإلا {
		/* need to double check i->file because we might be doing something
		 * more complicated by now, like sourcing or substituting. */
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
		إذا (i->__promptme && interactive && i->file == stdin) {
			طالما(! i->p || (interactive && لڙي_ڊيگهه(i->p)==0) ) {
#وإلا
			طالما(! i->p  || لڙي_ڊيگهه(i->p)==0 ) {
#جيڪڏهن_پورو
				get_user_داخلا(i);
			}
			i->promptmode=2;
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
			i->__promptme = 0;
#جيڪڏهن_پورو
			إذا (i->p && *i->p) {
				ch = *i->p++;
			}
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
		} وإلا {
			ch = fgetc(i->file);
		}

#جيڪڏهن_پورو
		debug_printf("b_getch: got a %d\n", ch);
	}
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
	إذا (ch == '\n') i->__promptme=1;
#جيڪڏهن_پورو
	أرجع ch;
}

/* All the callers guarantee this routine will never be
 * used right after a newline, so prompting is not needed.
 */
ساكن صحيح file_peek(بنية اندريون_str *i)
{
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
	إذا (i->p && *i->p) {
#جيڪڏهن_پورو
		أرجع *i->p;
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
	} وإلا {
		i->peek_buf[0] = fgetc(i->file);
		i->peek_buf[1] = '\0';
		i->p = i->peek_buf;
		debug_printf("b_peek: got a %d\n", *i->p);
		أرجع *i->p;
	}
#جيڪڏهن_پورو
}

#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
ساكن فراغ تياري_file_اندريون_str(بنية اندريون_str *i, FILE *f)
#وإلا
ساكن فراغ تياري_file_اندريون_str(بنية اندريون_str *i)
#جيڪڏهن_پورو
{
	i->peek = file_peek;
	i->get = file_get;
	i->__promptme=1;
	i->promptmode=1;
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
	i->file = f;
#جيڪڏهن_پورو
	i->p = عدم;
}

ساكن فراغ تياري_string_اندريون_str(بنية اندريون_str *i, ثابت حرف *s)
{
	i->peek = static_peek;
	i->get = static_get;
	i->__promptme=1;
	i->promptmode=1;
	i->p = s;
}

#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
ساكن فراغ mark_open(صحيح fd)
{
	بنية close_me *new = xmalloc(حجم(بنية close_me));
	new->fd = fd;
	new->ايندڙ = close_me_سر;
	close_me_سر = new;
}

ساكن فراغ mark_closed(صحيح fd)
{
	بنية close_me *tmp;
	إذا (close_me_سر == عدم || close_me_سر->fd != fd)
		error_نياپو_and_die("corrupt close_me");
	tmp = close_me_سر;
	close_me_سر = close_me_سر->ايندڙ;
	حرر(tmp);
}

ساكن فراغ close_all(فراغ)
{
	بنية close_me *c;
	لكل (c=close_me_سر; c; c=c->ايندڙ) {
		close(c->fd);
	}
	close_me_سر = عدم;
}

/* squirrel != NULL means we squirrel away copies of stdin, stdout,
 * and stderr if they are redirected. */
ساكن صحيح تياري_redirects(بنية ٻار_prog *prog, صحيح squirrel[])
{
	صحيح openfd, گهڻائي;
	بنية redir_struct *redir;

	لكل (redir=prog->redirects; redir; redir=redir->ايندڙ) {
		إذا (redir->dup == -1 && redir->لفظ.gl_pathv == عدم) {
			/* something went wrong in the parse.  Pretend it didn't happen */
			استمر;
		}
		إذا (redir->dup == -1) {
			گهڻائي=redir_جدول[redir->type].گهڻائي;
			openfd = open(redir->لفظ.gl_pathv[0], گهڻائي, 0666);
			إذا (openfd < 0) {
			/* this could get lost if stderr has been redirected, but
			   bash and ash both lose it as well (though zsh doesn't!) */
				perror_نياپو("error opening %s", redir->لفظ.gl_pathv[0]);
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
ساكن فراغ pseudo_exec(بنية ٻار_prog *ٻار)
{
	صحيح i, rcode;
	حرف *p;
	بنية built_اندريون_command *x;
	إذا (ٻار->argv) {
		لكل (i=0; is_assignment(ٻار->argv[i]); i++) {
			debug_printf("pid %d environment modification: %s\n",getpid(),ٻار->argv[i]);
			p = وجهو_var_قدر(ٻار->argv[i]);
			putenv(strdup(p));
			إذا (p != ٻار->argv[i]) حرر(p);
		}
		ٻار->argv+=i;  /* XXX this hack isn't so horrible, since we are about
					to exit, and therefore don't need to keep data
					structures consistent for free() use. */
		/* If a variable is assigned in a forest, and nobody listens,
		 * was it ever really set?
		 */
		إذا (ٻار->argv[0] == عدم) {
			_exit(EXIT_SUCCESS);
		}

		/*
		 * Check if the command matches any of the builtins.
		 * Depending on context, this might be redundant.  But it's
		 * easier to waste a few CPU cycles than it is to figure out
		 * if this is one of those cases.
		 */
		لكل (x = bltins; x->cmd; x++) {
			إذا (strcmp(ٻار->argv[0], x->cmd) == 0 ) {
				debug_printf("builtin exec %s\n", ٻار->argv[0]);
				rcode = x->function(ٻار);
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
#جيڪڏهن_وصف_ٿيل CONFIG_FEATURE_SH_STANDALONE_SHELL
		{
			صحيح argc_l;
			حرف** argv_l=ٻار->argv;
			حرف *name = ٻار->argv[0];

#جيڪڏهن_وصف_ٿيل CONFIG_FEATURE_SH_APPLETS_ALWAYS_WIN
			/* Following discussions from November 2000 on the busybox mailing
			 * list, the default configuration, (without
			 * get_last_path_component()) lets the user force use of an
			 * external command by specifying the full (with slashes) filename.
			 * If you enable CONFIG_FEATURE_SH_APPLETS_ALWAYS_WIN then applets
			 * _aways_ override external commands, so if you want to run
			 * /bin/cat, it will use BusyBox cat even if /bin/cat exists on the
			 * filesystem and is _not_ busybox.  Some systems may want this,
			 * most do not.  */
			name = get_آخري_رستو_component(name);
#جيڪڏهن_پورو
			/* Count argc for use in a second... */
			لكل(argc_l=0;*argv_l!=عدم; argv_l++, argc_l++);
			optind = 1;
			debug_printf("running applet %s\n", name);
			لاڳيتو_applet_by_name(name, argc_l, ٻار->argv);
		}
#جيڪڏهن_پورو
		debug_printf("exec of %s\n",ٻار->argv[0]);
		execvp(ٻار->argv[0],ٻار->argv);
		perror_نياپو("couldn't exec: %s",ٻار->argv[0]);
		_exit(1);
	} وإلا إذا (ٻار->گروهه) {
		debug_printf("runtime nesting to group\n");
		interactive=0;    /* crucial!!!! */
		rcode = لاڳيتو_list_real(ٻار->گروهه);
		/* OK to leak memory by not calling free_pipe_list,
		 * since this process is about to exit */
		_exit(rcode);
	} وإلا {
		/* Can happen.  See what bash does with ">foo" by itself. */
		debug_printf("trying to pseudo_exec null command\n");
		_exit(EXIT_SUCCESS);
	}
}

ساكن فراغ وجهو_bg_ڪم(بنية pipe *pi)
{
	بنية pipe *thejob;

	/* Linear search for the ID of the job to use */
	pi->jobid = 1;
	لكل (thejob = ڪم_list; thejob; thejob = thejob->ايندڙ)
		إذا (thejob->jobid >= pi->jobid)
			pi->jobid = thejob->jobid + 1;

	/* add thejob to the list of running jobs */
	إذا (!ڪم_list) {
		thejob = ڪم_list = xmalloc(حجم(*thejob));
	} وإلا {
		لكل (thejob = ڪم_list; thejob->ايندڙ; thejob = thejob->ايندڙ) /* nothing */;
		thejob->ايندڙ = xmalloc(حجم(*thejob));
		thejob = thejob->ايندڙ;
	}

	/* physically copy the struct job */
	يادگيري_نقل(thejob, pi, حجم(بنية pipe));
	thejob->ايندڙ = عدم;
	thejob->running_progs = thejob->انگ_progs;
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
	لکو("[%d] %d\n", thejob->jobid, thejob->progs[0].pid);
	آخري_bg_pid = thejob->progs[0].pid;
	آخري_jobid = thejob->jobid;
}

/* remove a backgrounded job */
ساكن فراغ remove_bg_ڪم(بنية pipe *pi)
{
	بنية pipe *prev_pipe;

	إذا (pi == ڪم_list) {
		ڪم_list = pi->ايندڙ;
	} وإلا {
		prev_pipe = ڪم_list;
		طالما (prev_pipe->ايندڙ != pi)
			prev_pipe = prev_pipe->ايندڙ;
		prev_pipe->ايندڙ = pi->ايندڙ;
	}
	إذا (ڪم_list)
		آخري_jobid = ڪم_list->jobid;
	وإلا
		آخري_jobid = 0;

	pi->stopped_progs = 0;
	free_pipe(pi, 0);
	حرر(pi);
}

/* Checks to see if any processes have exited -- if they
   have, figure out why and see if a job has completed */
ساكن صحيح checkjobs(بنية pipe* fg_pipe)
{
	صحيح خاصيتون;
	صحيح حالت;
	صحيح prognum = 0;
	بنية pipe *pi;
	pid_t childpid;

	خاصيتون = WUNTRACED;
	إذا (fg_pipe==عدم) {
		خاصيتون |= WNOHANG;
	}

	طالما ((childpid = waitpid(-1, &حالت, خاصيتون)) > 0) {
		إذا (fg_pipe) {
			صحيح i, rcode = 0;
			لكل (i=0; i < fg_pipe->انگ_progs; i++) {
				إذا (fg_pipe->progs[i].pid == childpid) {
					إذا (i==fg_pipe->انگ_progs-1)
						rcode=WEXITSTATUS(حالت);
					(fg_pipe->انگ_progs)--;
					أرجع(rcode);
				}
			}
		}

		لكل (pi = ڪم_list; pi; pi = pi->ايندڙ) {
			prognum = 0;
			طالما (prognum < pi->انگ_progs && pi->progs[prognum].pid != childpid) {
				prognum++;
			}
			إذا (prognum < pi->انگ_progs)
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
				لکو(ڪم_حالت_FORMAT, pi->jobid, "Done", pi->متن);
				remove_bg_ڪم(pi);
			}
		} وإلا {
			/* child stopped */
			pi->stopped_progs++;
			pi->progs[prognum].is_stopped = 1;

#إذا 0
			/* Printing this stuff is a pain, since it tends to
			 * overwrite the prompt an inconveinient moments.  So
			 * don't do that.  */
			إذا (pi->stopped_progs == pi->انگ_progs) {
				لکو("\n"ڪم_حالت_FORMAT, pi->jobid, "Stopped", pi->متن);
			}
#جيڪڏهن_پورو
		}
	}

	إذا (childpid == -1 && errno != ECHILD)
		perror_نياپو("waitpid");

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
#جيڪڏهن_پورو

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
ساكن صحيح لاڳيتو_pipe_real(بنية pipe *pi)
{
	صحيح i;
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
	صحيح nextin, nextout;
	صحيح pipefds[2];				/* pipefds[0] is for reading */
	بنية ٻار_prog *ٻار;
	بنية built_اندريون_command *x;
	حرف *p;
# إذا __GNUC__
	/* Avoid longjmp clobbering */
	(فراغ) &i;
	(فراغ) &nextin;
	(فراغ) &nextout;
	(فراغ) &ٻار;
# endif
#وإلا
	صحيح nextin;
	صحيح flag = do_repeat ? CMD_FLAG_REPEAT : 0;
	بنية ٻار_prog *ٻار;
	حرف *p;
# إذا __GNUC__
	/* Avoid longjmp clobbering */
	(فراغ) &i;
	(فراغ) &nextin;
	(فراغ) &ٻار;
# endif
#جيڪڏهن_پورو	/* __U_BOOT__ */

	nextin = 0;
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
	pi->pgrp = -1;
#جيڪڏهن_پورو

	/* Check if this is a simple builtin (not part of a pipe).
	 * Builtins within pipes have to fork anyway, and are handled in
	 * pseudo_exec.  "echo foo | read bar" doesn't work on bash, either.
	 */
	إذا (pi->انگ_progs == 1) ٻار = & (pi->progs[0]);
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
	إذا (pi->انگ_progs == 1 && ٻار->گروهه && ٻار->subshell == 0) {
		صحيح squirrel[] = {-1, -1, -1};
		صحيح rcode;
		debug_printf("non-subshell grouping\n");
		تياري_redirects(ٻار, squirrel);
		/* XXX could we merge code with following builtin case,
		 * by creating a pseudo builtin that calls run_list_real? */
		rcode = لاڳيتو_list_real(ٻار->گروهه);
		restore_redirects(squirrel);
#وإلا
		إذا (pi->انگ_progs == 1 && ٻار->گروهه) {
		صحيح rcode;
		debug_printf("non-subshell grouping\n");
		rcode = لاڳيتو_list_real(ٻار->گروهه);
#جيڪڏهن_پورو
		أرجع rcode;
	} وإلا إذا (pi->انگ_progs == 1 && pi->progs[0].argv != عدم) {
		لكل (i=0; is_assignment(ٻار->argv[i]); i++) { /* nothing */ }
		إذا (i!=0 && ٻار->argv[i]==عدم) {
			/* assignments, but no command: set the local environment */
			لكل (i=0; ٻار->argv[i]!=عدم; i++) {

				/* Ok, this case is tricky.  We have to decide if this is a
				 * local variable, or an already exported variable.  If it is
				 * already exported, we have to export the new value.  If it is
				 * not exported, we need only set this as a local variable.
				 * This junk is all to decide whether or not to export this
				 * variable. */
				صحيح export_me=0;
				حرف *name, *قدر;
				name = xstrdup(ٻار->argv[i]);
				debug_printf("Local environment set: %s\n", name);
				قدر = strchr(name, '=');
				إذا (قدر)
					*قدر=0;
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
				إذا ( get_local_var(name)) {
					export_me=1;
				}
#جيڪڏهن_پورو
				حرر(name);
				p = وجهو_var_قدر(ٻار->argv[i]);
				سيٽ_local_var(p, export_me);
				إذا (p != ٻار->argv[i]) حرر(p);
			}
			أرجع EXIT_SUCCESS;   /* don't worry about errors in set_local_var() yet */
		}
		لكل (i = 0; is_assignment(ٻار->argv[i]); i++) {
			p = وجهو_var_قدر(ٻار->argv[i]);
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
			putenv(strdup(p));
#وإلا
			سيٽ_local_var(p, 0);
#جيڪڏهن_پورو
			إذا (p != ٻار->argv[i]) {
				ٻار->sp--;
				حرر(p);
			}
		}
		إذا (ٻار->sp) {
			حرف * str = عدم;

			str = make_string(ٻار->argv + i,
					  ٻار->argv_nonnull + i);
			parse_string_outer(str, FLAG_EXIT_FROM_LOOP | FLAG_REPARSING);
			حرر(str);
			أرجع آخري_return_ڪوڊ;
		}
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
		لكل (x = bltins; x->cmd; x++) {
			إذا (strcmp(ٻار->argv[i], x->cmd) == 0 ) {
				صحيح squirrel[] = {-1, -1, -1};
				صحيح rcode;
				إذا (x->function == builtin_exec && ٻار->argv[i+1]==عدم) {
					debug_printf("magic exec\n");
					تياري_redirects(ٻار,عدم);
					أرجع EXIT_SUCCESS;
				}
				debug_printf("builtin inline %s\n", ٻار->argv[0]);
				/* XXX setup_redirects acts on file descriptors, not FILEs.
				 * This is perfect for work that comes after exec().
				 * Is it really safe for inline use?  Experimentally,
				 * things seem to work with glibc. */
				تياري_redirects(ٻار, squirrel);

				ٻار->argv += i;  /* XXX horrible hack */
				rcode = x->function(ٻار);
				/* XXX restore hack so free() can work right */
				ٻار->argv -= i;
				restore_redirects(squirrel);
			}
			أرجع rcode;
		}
#وإلا
		/* check ";", because ,example , argv consist from
		 * "help;flinfo" must not execute
		 */
		إذا (strchr(ٻار->argv[i], ';')) {
			لکو("Unknown command '%s' - try 'help' or use "
					"'run' command\n", ٻار->argv[i]);
			أرجع -1;
		}
		/* Process the command */
		أرجع cmd_process(flag, ٻار->argc - i, ٻار->argv + i,
				   &flag_repeat, عدم);
#جيڪڏهن_پورو
	}
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__

	لكل (i = 0; i < pi->انگ_progs; i++) {
		ٻار = & (pi->progs[i]);

		/* pipes are inserted between pairs of commands */
		إذا ((i + 1) < pi->انگ_progs) {
			إذا (pipe(pipefds)<0) perror_نياپو_and_die("pipe");
			nextout = pipefds[1];
		} وإلا {
			nextout=1;
			pipefds[0] = -1;
		}

		/* XXX test for failed fork()? */
		إذا (!(ٻار->pid = fork())) {
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
			تياري_redirects(ٻار,عدم);

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

			pseudo_exec(ٻار);
		}

		/* put our child in the process group whose leader is the
		   first process in this pipe */
		إذا (pi->pgrp < 0) {
			pi->pgrp = ٻار->pid;
		}
		/* Don't check for errors.  The child may be dead already,
		 * in which case setpgid returns error code EACCES. */
		setpgid(ٻار->pid, pi->pgrp);

		إذا (nextin != 0)
			close(nextin);
		إذا (nextout != 1)
			close(nextout);

		/* If there isn't another process, nextin is garbage
		   but it doesn't matter */
		nextin = pipefds[0];
	}
#جيڪڏهن_پورو
	أرجع -1;
}

ساكن صحيح لاڳيتو_list_real(بنية pipe *pi)
{
	حرف *save_name = عدم;
	حرف **list = عدم;
	حرف **save_list = عدم;
	بنية pipe *rpipe;
	صحيح flag_rep = 0;
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
	صحيح save_انگ_progs;
#جيڪڏهن_پورو
	صحيح rcode=0, flag_skip=1;
	صحيح flag_restore = 0;
	صحيح if_ڪوڊ=0, ايندڙ_if_ڪوڊ=0;  /* need double-buffer to handle elif */
	reserved_style rmode, skip_وڌيڪ_اندريون_this_rmode=RES_XXXX;
	/* check syntax for "for" */
	لكل (rpipe = pi; rpipe; rpipe = rpipe->ايندڙ) {
		إذا ((rpipe->r_گهڻائي == RES_اندريون ||
		    rpipe->r_گهڻائي == RES_FOR) &&
		    (rpipe->ايندڙ == عدم)) {
				syntax();
#جيڪڏهن_وصف_ٿيل __U_BOOT__
				flag_repeat = 0;
#جيڪڏهن_پورو
				أرجع 1;
		}
		إذا ((rpipe->r_گهڻائي == RES_اندريون &&
			(rpipe->ايندڙ->r_گهڻائي == RES_اندريون &&
			rpipe->ايندڙ->progs->argv != عدم))||
			(rpipe->r_گهڻائي == RES_FOR &&
			rpipe->ايندڙ->r_گهڻائي != RES_اندريون)) {
				syntax();
#جيڪڏهن_وصف_ٿيل __U_BOOT__
				flag_repeat = 0;
#جيڪڏهن_پورو
				أرجع 1;
		}
	}
	لكل (; pi; pi = (flag_restore != 0) ? rpipe : pi->ايندڙ) {
		إذا (pi->r_گهڻائي == RES_WHILE || pi->r_گهڻائي == RES_UNTIL ||
			pi->r_گهڻائي == RES_FOR) {
#جيڪڏهن_وصف_ٿيل __U_BOOT__
				/* check Ctrl-C */
				ctrlc();
				إذا ((had_ctrlc())) {
					أرجع 1;
				}
#جيڪڏهن_پورو
				flag_restore = 0;
				إذا (!rpipe) {
					flag_rep = 0;
					rpipe = pi;
				}
		}
		rmode = pi->r_گهڻائي;
		debug_printf("rmode=%d  if_code=%d  next_if_code=%d skip_more=%d\n", rmode, if_ڪوڊ, ايندڙ_if_ڪوڊ, skip_وڌيڪ_اندريون_this_rmode);
		إذا (rmode == skip_وڌيڪ_اندريون_this_rmode && flag_skip) {
			إذا (pi->followup == PIPE_SEQ) flag_skip=0;
			استمر;
		}
		flag_skip = 1;
		skip_وڌيڪ_اندريون_this_rmode = RES_XXXX;
		إذا (rmode == RES_THEN || rmode == RES_ELSE) if_ڪوڊ = ايندڙ_if_ڪوڊ;
		إذا (rmode == RES_THEN &&  if_ڪوڊ) استمر;
		إذا (rmode == RES_ELSE && !if_ڪوڊ) استمر;
		إذا (rmode == RES_ELIF && !if_ڪوڊ) اكسر;
		إذا (rmode == RES_FOR && pi->انگ_progs) {
			إذا (!list) {
				/* if no variable values after "in" we skip "for" */
				إذا (!pi->ايندڙ->progs->argv) استمر;
				/* create list of variable values */
				list = make_list_اندريون(pi->ايندڙ->progs->argv,
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
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
				pi->progs->glob_نتيجو.gl_pathv[0] =
					pi->progs->argv[0];
#جيڪڏهن_پورو
				استمر;
			} وإلا {
				/* insert new value from list for variable */
				حرر(pi->progs->argv[0]);
				pi->progs->argv[0] = *list++;
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
				pi->progs->glob_نتيجو.gl_pathv[0] =
					pi->progs->argv[0];
#جيڪڏهن_پورو
			}
		}
		إذا (rmode == RES_اندريون) استمر;
		إذا (rmode == RES_DO) {
			إذا (!flag_rep) استمر;
		}
		إذا (rmode == RES_پورو) {
			إذا (flag_rep) {
				flag_restore = 1;
			} وإلا {
				rpipe = عدم;
			}
		}
		إذا (pi->انگ_progs == 0) استمر;
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
		save_انگ_progs = pi->انگ_progs; /* save number of programs */
#جيڪڏهن_پورو
		rcode = لاڳيتو_pipe_real(pi);
		debug_printf("run_pipe_real returned %d\n",rcode);
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
		إذا (rcode!=-1) {
			/* We only ran a builtin: rcode was set by the return value
			 * of run_pipe_real(), and we don't need to wait for anything. */
		} وإلا إذا (pi->followup==PIPE_BG) {
			/* XXX check bash's behavior with nontrivial pipes */
			/* XXX compute jobid */
			/* XXX what does bash do with attempts to background builtins? */
			وجهو_bg_ڪم(pi);
			rcode = EXIT_SUCCESS;
		} وإلا {
			إذا (interactive) {
				/* move the new process group into the foreground */
				إذا (tcsetpgrp(shell_terminal, pi->pgrp) && errno != ENOTTY)
					perror_نياپو("tcsetpgrp-3");
				rcode = checkjobs(pi);
				/* move the shell to the foreground */
				إذا (tcsetpgrp(shell_terminal, getpgid(0)) && errno != ENOTTY)
					perror_نياپو("tcsetpgrp-4");
			} وإلا {
				rcode = checkjobs(pi);
			}
			debug_printf("checkjobs returned %d\n",rcode);
		}
		آخري_return_ڪوڊ=rcode;
#وإلا
		إذا (rcode < -1) {
			آخري_return_ڪوڊ = -rcode - 2;
			أرجع -2;	/* exit */
		}
		آخري_return_ڪوڊ = rcode;
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
		pi->انگ_progs = save_انگ_progs; /* restore number of programs */
#جيڪڏهن_پورو
		إذا ( rmode == RES_IF || rmode == RES_ELIF )
			ايندڙ_if_ڪوڊ=rcode;  /* can be overwritten a number of times */
		إذا (rmode == RES_WHILE)
			flag_rep = !آخري_return_ڪوڊ;
		إذا (rmode == RES_UNTIL)
			flag_rep = آخري_return_ڪوڊ;
		إذا ( (rcode==EXIT_SUCCESS && pi->followup==PIPE_OR) ||
		     (rcode!=EXIT_SUCCESS && pi->followup==PIPE_AND) )
			skip_وڌيڪ_اندريون_this_rmode=rmode;
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
		checkjobs(عدم);
#جيڪڏهن_پورو
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
	بنية ٻار_prog *ٻار;
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
	بنية redir_struct *r, *rnext;
#جيڪڏهن_پورو
	صحيح a, i, ret_ڪوڊ=0;
	حرف *ind = indenter(indent);

#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
	إذا (pi->stopped_progs > 0)
		أرجع ret_ڪوڊ;
	final_printf("%s run pipe: (pid %d)\n",ind,getpid());
#جيڪڏهن_پورو
	لكل (i=0; i<pi->انگ_progs; i++) {
		ٻار = &pi->progs[i];
		final_printf("%s  command %d:\n",ind,i);
		إذا (ٻار->argv) {
			لكل (a=0,p=ٻار->argv; *p; a++,p++) {
				final_printf("%s   argv[%d] = %s\n",ind,a,*p);
			}
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
			globfree(&ٻار->glob_نتيجو);
#وإلا
			لكل (a = 0; a < ٻار->argc; a++) {
				حرر(ٻار->argv[a]);
			}
			حرر(ٻار->argv);
			حرر(ٻار->argv_nonnull);
			ٻار->argc = 0;
#جيڪڏهن_پورو
			ٻار->argv=عدم;
		} وإلا إذا (ٻار->گروهه) {
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
			final_printf("%s   begin group (subshell:%d)\n",ind, ٻار->subshell);
#جيڪڏهن_پورو
			ret_ڪوڊ = free_pipe_list(ٻار->گروهه,indent+3);
			final_printf("%s   end group\n",ind);
		} وإلا {
			final_printf("%s   (nil)\n",ind);
		}
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
		لكل (r=ٻار->redirects; r; r=rnext) {
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
			rnext=r->ايندڙ;
			حرر(r);
		}
		ٻار->redirects=عدم;
#جيڪڏهن_پورو
	}
	حرر(pi->progs);   /* children are an array, they get freed all at once */
	pi->progs=عدم;
	أرجع ret_ڪوڊ;
}

ساكن صحيح free_pipe_list(بنية pipe *سر, صحيح indent)
{
	صحيح rcode=0;   /* if list has no members */
	بنية pipe *pi, *ايندڙ;
	حرف *ind = indenter(indent);
	لكل (pi=سر; pi; pi=ايندڙ) {
		final_printf("%s pipe reserved mode %d\n", ind, pi->r_گهڻائي);
		rcode = free_pipe(pi, indent);
		final_printf("%s pipe followup code %d\n", ind, pi->followup);
		ايندڙ=pi->ايندڙ;
		pi->ايندڙ=عدم;
		حرر(pi);
	}
	أرجع rcode;
}

/* Select which version we will use */
ساكن صحيح لاڳيتو_list(بنية pipe *pi)
{
	صحيح rcode=0;
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
	إذا (fake_گهڻائي==0) {
#جيڪڏهن_پورو
		rcode = لاڳيتو_list_real(pi);
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
	}
#جيڪڏهن_پورو
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
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
ساكن صحيح globhack(ثابت حرف *src, صحيح flags, glob_t *pglob)
{
	صحيح cnt=0, pathc;
	ثابت حرف *s;
	حرف *dest;
	لكل (cnt=1, s=src; s && *s; s++) {
		إذا (*s == '\\') s++;
		cnt++;
	}
	dest = يادگيري_ڏيو(cnt);
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
#جيڪڏهن_پورو

ساكن صحيح xglob(o_string *dest, صحيح flags, glob_t *pglob)
{
	صحيح gr;

	/* short-circuit for null word */
	/* we can code this better when the debug_printf's are gone */
	إذا (dest->ڊيگهائي == 0) {
		إذا (dest->nonnull) {
			/* bash man page calls this an "explicit" null */
			gr = globhack(dest->معلومات, flags, pglob);
			debug_printf("globhack returned %d\n",gr);
		} وإلا {
			أرجع 0;
		}
	} وإلا إذا (glob_needed(dest->معلومات)) {
		gr = glob(dest->معلومات, flags, عدم, pglob);
		debug_printf("glob returned %d\n",gr);
		إذا (gr == GLOB_NOMATCH) {
			/* quote removal, or more accurately, backslash removal */
			gr = globhack(dest->معلومات, flags, pglob);
			debug_printf("globhack returned %d\n",gr);
		}
	} وإلا {
		gr = globhack(dest->معلومات, flags, pglob);
		debug_printf("globhack returned %d\n",gr);
	}
	إذا (gr == GLOB_NOSPACE)
		error_نياپو_and_die("out of memory during glob");
	إذا (gr != 0) { /* GLOB_ABORTED ? */
		error_نياپو("glob(3) error %d",gr);
	}
	/* globprint(glob_target); */
	أرجع gr;
}
#جيڪڏهن_پورو

#جيڪڏهن_وصف_ٿيل __U_BOOT__
ساكن حرف *get_dollar_var(حرف ch);
#جيڪڏهن_پورو

/* This is used to get/check local shell variables */
حرف *get_local_var(ثابت حرف *s)
{
	بنية variables *cur;

	إذا (!s)
		أرجع عدم;

#جيڪڏهن_وصف_ٿيل __U_BOOT__
	إذا (*s == '$')
		أرجع get_dollar_var(s[1]);
#جيڪڏهن_پورو

	لكل (cur = چوٽي_vars; cur; cur=cur->ايندڙ)
		إذا(strcmp(cur->name, s)==0)
			أرجع cur->قدر;
	أرجع عدم;
}

/* This is used to set local shell variables
   flg_export==0 if only local (not exporting) variable
   flg_export==1 if "new" exporting environ
   flg_export>1  if current startup environ (not call putenv()) */
صحيح سيٽ_local_var(ثابت حرف *s, صحيح flg_export)
{
	حرف *name, *قدر;
	صحيح نتيجو=0;
	بنية variables *cur;

#جيڪڏهن_وصف_ٿيل __U_BOOT__
	/* might be possible! */
	إذا (!اکر_آهي(*s))
		أرجع -1;
#جيڪڏهن_پورو

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

	لكل(cur = چوٽي_vars; cur; cur = cur->ايندڙ) {
		إذا(strcmp(cur->name, name)==0)
			اكسر;
	}

	إذا(cur) {
		إذا(strcmp(cur->قدر, قدر)==0) {
			إذا(flg_export>0 && cur->flg_export==0)
				cur->flg_export=flg_export;
			وإلا
				نتيجو++;
		} وإلا {
			إذا(cur->flg_read_only) {
				error_نياپو("%s: readonly variable", name);
				نتيجو = -1;
			} وإلا {
				إذا(flg_export>0 || cur->flg_export>1)
					cur->flg_export=1;
				حرر(cur->قدر);

				cur->قدر = strdup(قدر);
			}
		}
	} وإلا {
		cur = يادگيري_ڏيو(حجم(بنية variables));
		إذا(!cur) {
			نتيجو = -1;
		} وإلا {
			cur->name = strdup(name);
			إذا (cur->name == عدم) {
				حرر(cur);
				نتيجو = -1;
			} وإلا {
				بنية variables *bottom = چوٽي_vars;
				cur->قدر = strdup(قدر);
				cur->ايندڙ = عدم;
				cur->flg_export = flg_export;
				cur->flg_read_only = 0;
				طالما(bottom->ايندڙ) bottom=bottom->ايندڙ;
				bottom->ايندڙ = cur;
			}
		}
	}

#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
	إذا(نتيجو==0 && cur->flg_export==1) {
		*(قدر-1) = '=';
		نتيجو = putenv(name);
	} وإلا {
#جيڪڏهن_پورو
		حرر(name);
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
		إذا(نتيجو>0)            /* equivalent to previous set */
			نتيجو = 0;
	}
#جيڪڏهن_پورو
	أرجع نتيجو;
}

فراغ unset_local_var(ثابت حرف *name)
{
	بنية variables *cur;

	إذا (name) {
		لكل (cur = چوٽي_vars; cur; cur=cur->ايندڙ) {
			إذا(strcmp(cur->name, name)==0)
				اكسر;
		}
		إذا (cur != عدم) {
			بنية variables *ايندڙ = چوٽي_vars;
			إذا(cur->flg_read_only) {
				error_نياپو("%s: readonly variable", name);
				أرجع;
			} وإلا {
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
				إذا(cur->flg_export)
					unenv_سيٽ(cur->name);
#جيڪڏهن_پورو
				حرر(cur->name);
				حرر(cur->قدر);
				طالما (ايندڙ->ايندڙ != cur)
					ايندڙ = ايندڙ->ايندڙ;
				ايندڙ->ايندڙ = cur->ايندڙ;
			}
			حرر(cur);
		}
	}
}

ساكن صحيح is_assignment(ثابت حرف *s)
{
	إذا (s == عدم)
		أرجع 0;

	إذا (!اکر_آهي(*s)) أرجع 0;
	++s;
	طالما(اکر_يا_انگ(*s) || *s=='_') ++s;
	أرجع *s=='=';
}

#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
/* the src parameter allows us to peek forward to a possible &n syntax
 * for file descriptor duplication, e.g., "2>&1".
 * Return code is 0 normally, 1 if a syntax error is detected in src.
 * Resource errors (in xmalloc) cause the process to exit */
ساكن صحيح تياري_redirect(بنية p_context *ctx, صحيح fd, redir_type style,
	بنية اندريون_str *داخلا)
{
	بنية ٻار_prog *ٻار=ctx->ٻار;
	بنية redir_struct *redir = ٻار->redirects;
	بنية redir_struct *آخري_redir=عدم;

	/* Create a new redir_struct and drop it onto the end of the linked list */
	طالما(redir) {
		آخري_redir=redir;
		redir=redir->ايندڙ;
	}
	redir = xmalloc(حجم(بنية redir_struct));
	redir->ايندڙ=عدم;
	redir->لفظ.gl_pathv=عدم;
	إذا (آخري_redir) {
		آخري_redir->ايندڙ=redir;
	} وإلا {
		ٻار->redirects=redir;
	}

	redir->type=style;
	redir->fd= (fd==-1) ? redir_جدول[style].default_fd : fd ;

	debug_printf("Redirect type %d%s\n", redir->fd, redir_جدول[style].descrip);

	/* Check for a '2>&1' type redirect */
	redir->dup = redirect_dup_انگ(داخلا);
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
#جيڪڏهن_پورو

ساكن بنية pipe *new_pipe(فراغ)
{
	بنية pipe *pi;
	pi = xmalloc(حجم(بنية pipe));
	pi->انگ_progs = 0;
	pi->progs = عدم;
	pi->ايندڙ = عدم;
	pi->followup = 0;  /* invalid */
	pi->r_گهڻائي = RES_NONE;
	أرجع pi;
}

ساكن فراغ initialize_context(بنية p_context *ctx)
{
	ctx->pipe=عدم;
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
	ctx->pending_redirect=عدم;
#جيڪڏهن_پورو
	ctx->ٻار=عدم;
	ctx->list_سر=new_pipe();
	ctx->pipe=ctx->list_سر;
	ctx->w=RES_NONE;
	ctx->ڍير=عدم;
#جيڪڏهن_وصف_ٿيل __U_BOOT__
	ctx->old_flag=0;
#جيڪڏهن_پورو
	پورو_command(ctx);   /* creates the memory for working child */
}

/* normal return is 0
 * if a reserved word is found, and processed, return 1
 * should handle if, then, elif, else, fi, for, while, until, do, done.
 * case, function, and select are obnoxious, save those for later.
 */
بنية reserved_combo {
	حرف *literal;
	صحيح ڪوڊ;
	ڊگهو flag;
};
/* Mostly a list of accepted follow-up reserved words.
 * FLAG_END means we are done with the sequence, and are ready
 * to turn the compound list into a command.
 * FLAG_START means the word must start a new compound list.
 */
ساكن بنية reserved_combo reserved_list[] = {
	{ "if",    RES_IF,    FLAG_THEN | FLAG_شروعات },
	{ "then",  RES_THEN,  FLAG_ELIF | FLAG_ELSE | FLAG_FI },
	{ "elif",  RES_ELIF,  FLAG_THEN },
	{ "else",  RES_ELSE,  FLAG_FI   },
	{ "fi",    RES_FI,    FLAG_پڇاڙي  },
	{ "for",   RES_FOR,   FLAG_اندريون   | FLAG_شروعات },
	{ "while", RES_WHILE, FLAG_DO   | FLAG_شروعات },
	{ "until", RES_UNTIL, FLAG_DO   | FLAG_شروعات },
	{ "in",    RES_اندريون,    FLAG_DO   },
	{ "do",    RES_DO,    FLAG_پورو },
	{ "done",  RES_پورو,  FLAG_پڇاڙي  }
};
#وصف_ڪريو NRES (حجم(reserved_list)/حجم(بنية reserved_combo))

ساكن صحيح reserved_لفظ(o_string *dest, بنية p_context *ctx)
{
	بنية reserved_combo *r;
	لكل (r=reserved_list;
		r<reserved_list+NRES; r++) {
		إذا (strcmp(dest->معلومات, r->literal) == 0) {
			debug_printf("found reserved word %s, code %d\n",r->literal,r->ڪوڊ);
			إذا (r->flag & FLAG_شروعات) {
				بنية p_context *new = xmalloc(حجم(بنية p_context));
				debug_printf("push stack\n");
				إذا (ctx->w == RES_اندريون || ctx->w == RES_FOR) {
					syntax();
					حرر(new);
					ctx->w = RES_SNTX;
					b_reset(dest);
					أرجع 1;
				}
				*new = *ctx;   /* physical copy */
				initialize_context(ctx);
				ctx->ڍير=new;
			} وإلا إذا ( ctx->w == RES_NONE || ! (ctx->old_flag & (1<<r->ڪوڊ))) {
				syntax();
				ctx->w = RES_SNTX;
				b_reset(dest);
				أرجع 1;
			}
			ctx->w=r->ڪوڊ;
			ctx->old_flag = r->flag;
			إذا (ctx->old_flag & FLAG_پڇاڙي) {
				بنية p_context *old;
				debug_printf("pop stack\n");
				پورو_pipe(ctx,PIPE_SEQ);
				old = ctx->ڍير;
				old->ٻار->گروهه = ctx->list_سر;
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
				old->ٻار->subshell = 0;
#جيڪڏهن_پورو
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
ساكن صحيح پورو_لفظ(o_string *dest, بنية p_context *ctx)
{
	بنية ٻار_prog *ٻار=ctx->ٻار;
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
	glob_t *glob_هدف;
	صحيح gr, flags = 0;
#وإلا
	حرف *str, *s;
	صحيح argc, cnt;
#جيڪڏهن_پورو

	debug_printf("done_word: %s %p\n", dest->معلومات, ٻار);
	إذا (dest->ڊيگهائي == 0 && !dest->nonnull) {
		debug_printf("  true null, ignored\n");
		أرجع 0;
	}
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
	إذا (ctx->pending_redirect) {
		glob_هدف = &ctx->pending_redirect->لفظ;
	} وإلا {
#جيڪڏهن_پورو
		إذا (ٻار->گروهه) {
			syntax();
			أرجع 1;  /* syntax error, groups and arglists don't mix */
		}
		إذا (!ٻار->argv && (ctx->type & FLAG_PARSE_SEMICOLON)) {
			debug_printf("checking %s for reserved-ness\n",dest->معلومات);
			إذا (reserved_لفظ(dest,ctx)) أرجع ctx->w==RES_SNTX;
		}
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
		glob_هدف = &ٻار->glob_نتيجو;
		إذا (ٻار->argv) flags |= GLOB_APPEND;
#وإلا
		لكل (cnt = 1, s = dest->معلومات; s && *s; s++) {
			إذا (*s == '\\') s++;
			cnt++;
		}
		str = يادگيري_ڏيو(cnt);
		إذا (!str) أرجع 1;
		إذا ( ٻار->argv == عدم) {
			ٻار->argc=0;
		}
		argc = ++ٻار->argc;
		ٻار->argv = realloc(ٻار->argv, (argc+1)*حجم(*ٻار->argv));
		إذا (ٻار->argv == عدم) {
			حرر(str);
			أرجع 1;
		}
		ٻار->argv_nonnull = realloc(ٻار->argv_nonnull,
					(argc+1)*حجم(*ٻار->argv_nonnull));
		إذا (ٻار->argv_nonnull == عدم) {
			حرر(str);
			أرجع 1;
		}
		ٻار->argv[argc-1]=str;
		ٻار->argv_nonnull[argc-1] = dest->nonnull;
		ٻار->argv[argc]=عدم;
		ٻار->argv_nonnull[argc] = 0;
		لكل (s = dest->معلومات; s && *s; s++,str++) {
			إذا (*s == '\\') s++;
			*str = *s;
		}
		*str = '\0';
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
	}
	gr = xglob(dest, flags, glob_هدف);
	إذا (gr != 0) أرجع 1;
#جيڪڏهن_پورو

	b_reset(dest);
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
	إذا (ctx->pending_redirect) {
		ctx->pending_redirect=عدم;
		إذا (glob_هدف->gl_pathc != 1) {
			error_نياپو("ambiguous redirect");
			أرجع 1;
		}
	} وإلا {
		ٻار->argv = glob_هدف->gl_pathv;
	}
#جيڪڏهن_پورو
	إذا (ctx->w == RES_FOR) {
		پورو_لفظ(dest,ctx);
		پورو_pipe(ctx,PIPE_SEQ);
	}
	أرجع 0;
}

/* The only possible error here is out of memory, in which case
 * xmalloc exits. */
ساكن صحيح پورو_command(بنية p_context *ctx)
{
	/* The child is really already in the pipe structure, so
	 * advance the pipe counter and make a new, null child.
	 * Only real trickiness here is that the uncommitted
	 * child structure, to which ctx->child points, is not
	 * counted in pi->num_progs. */
	بنية pipe *pi=ctx->pipe;
	بنية ٻار_prog *prog=ctx->ٻار;

	إذا (prog && prog->گروهه == عدم
		 && prog->argv == عدم
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
		 && prog->redirects == عدم) {
#وإلا
										) {
#جيڪڏهن_پورو
		debug_printf("done_command: skipping null command\n");
		أرجع 0;
	} وإلا إذا (prog) {
		pi->انگ_progs++;
		debug_printf("done_command: num_progs incremented to %d\n",pi->انگ_progs);
	} وإلا {
		debug_printf("done_command: initializing\n");
	}
	pi->progs = xrealloc(pi->progs, حجم(*pi->progs) * (pi->انگ_progs+1));

	prog = pi->progs + pi->انگ_progs;
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
	prog->redirects = عدم;
#جيڪڏهن_پورو
	prog->argv = عدم;
	prog->argv_nonnull = عدم;
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
	prog->is_stopped = 0;
#جيڪڏهن_پورو
	prog->گروهه = عدم;
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
	prog->glob_نتيجو.gl_pathv = عدم;
	prog->family = pi;
#جيڪڏهن_پورو
	prog->sp = 0;
	ctx->ٻار = prog;
	prog->type = ctx->type;

	/* but ctx->pipe and ctx->list_head remain unchanged */
	أرجع 0;
}

ساكن صحيح پورو_pipe(بنية p_context *ctx, pipe_style type)
{
	بنية pipe *new_p;
	پورو_command(ctx);  /* implicit closure of previous command */
	debug_printf("done_pipe, type %d\n", type);
	ctx->pipe->followup = type;
	ctx->pipe->r_گهڻائي = ctx->w;
	new_p=new_pipe();
	ctx->pipe->ايندڙ = new_p;
	ctx->pipe = new_p;
	ctx->ٻار = عدم;
	پورو_command(ctx);  /* set up new pipe to accept commands */
	أرجع 0;
}

#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
/* peek ahead in the in_str to find out if we have a "&n" construct,
 * as in "2>&1", that represents duplicating a file descriptor.
 * returns either -2 (syntax error), -1 (no &), or the number found.
 */
ساكن صحيح redirect_dup_انگ(بنية اندريون_str *داخلا)
{
	صحيح ch, d=0, ok=0;
	ch = b_peek(داخلا);
	إذا (ch != '&') أرجع -1;

	b_getch(داخلا);  /* get the & */
	ch=b_peek(داخلا);
	إذا (ch == '-') {
		b_getch(داخلا);
		أرجع -3;  /* "-" represents "close me" */
	}
	طالما (انگ_آهي(ch)) {
		d = d*10+(ch-'0');
		ok=1;
		b_getch(داخلا);
		ch = b_peek(داخلا);
	}
	إذا (ok) أرجع d;

	error_نياپو("ambiguous redirect");
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
ساكن صحيح redirect_opt_انگ(o_string *o)
{
	صحيح انگ;

	إذا (o->ڊيگهائي==0) أرجع -1;
	لكل(انگ=0; انگ<o->ڊيگهائي; انگ++) {
		إذا (!انگ_آهي(*(o->معلومات+انگ))) {
			أرجع -1;
		}
	}
	/* reuse num (and save an int) */
	انگ=atoi(o->معلومات);
	b_reset(o);
	أرجع انگ;
}

FILE *generate_وهڪرو_from_list(بنية pipe *سر)
{
	FILE *pf;
#إذا 1
	صحيح pid, channel[2];
	إذا (pipe(channel)<0) perror_نياپو_and_die("pipe");
	pid=fork();
	إذا (pid<0) {
		perror_نياپو_and_die("fork");
	} وإلا إذا (pid==0) {
		close(channel[0]);
		إذا (channel[1] != 1) {
			dup2(channel[1],1);
			close(channel[1]);
		}
#إذا 0
#وصف_ڪريو SURROGATE "surrogate response"
		اكتب(1,SURROGATE,حجم(SURROGATE));
		_exit(لاڳيتو_list(سر));
#وإلا
		_exit(لاڳيتو_list_real(سر));   /* leaks memory */
#جيڪڏهن_پورو
	}
	debug_printf("forked child %d\n",pid);
	close(channel[1]);
	pf = fdopen(channel[0],"r");
	debug_printf("pipe on FILE *%p\n",pf);
#وإلا
	free_pipe_list(سر,0);
	pf=popen("echo surrogate response","r");
	debug_printf("started fake pipe on FILE *%p\n",pf);
#جيڪڏهن_پورو
	أرجع pf;
}

/* this version hacked for testing purposes */
/* return code is exit status of the process that is run. */
ساكن صحيح process_command_subs(o_string *dest, بنية p_context *ctx, بنية اندريون_str *داخلا, صحيح subst_پڇاڙي)
{
	صحيح retcode;
	o_string نتيجو=NULL_O_STRING;
	بنية p_context inner;
	FILE *p;
	بنية اندريون_str pipe_str;
	initialize_context(&inner);

	/* recursion to generate command */
	retcode = parse_وهڪرو(&نتيجو, &inner, داخلا, subst_پڇاڙي);
	إذا (retcode != 0) أرجع retcode;  /* syntax error or EOF */
	پورو_لفظ(&نتيجو, &inner);
	پورو_pipe(&inner, PIPE_SEQ);
	b_free(&نتيجو);

	p=generate_وهڪرو_from_list(inner.list_سر);
	إذا (p==عدم) أرجع 1;
	mark_open(fileno(p));
	تياري_file_اندريون_str(&pipe_str, p);

	/* now send results of command back into original context */
	retcode = parse_وهڪرو(dest, ctx, &pipe_str, '\0');
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

ساكن صحيح parse_گروهه(o_string *dest, بنية p_context *ctx,
	بنية اندريون_str *داخلا, صحيح ch)
{
	صحيح rcode, endch=0;
	بنية p_context sub;
	بنية ٻار_prog *ٻار = ctx->ٻار;
	إذا (ٻار->argv) {
		syntax();
		أرجع 1;  /* syntax error, groups and arglists don't mix */
	}
	initialize_context(&sub);
	بدل(ch) {
		حالة '(': endch=')'; ٻار->subshell=1; اكسر;
		حالة '{': endch='}'; اكسر;
		افتراضي: syntax();   /* really logic error */
	}
	rcode=parse_وهڪرو(dest,&sub,داخلا,endch);
	پورو_لفظ(dest,&sub); /* finish off the final word in the subcontext */
	پورو_pipe(&sub, PIPE_SEQ);  /* and the final command there, too */
	ٻار->گروهه = sub.list_سر;
	أرجع rcode;
	/* child remains "open", available for possible redirects */
}
#جيڪڏهن_پورو

/* basically useful version until someone wants to get fancier,
 * see the bash man page under "Parameter Expansion" */
ساكن حرف *lookup_param(حرف *src)
{
	حرف *p;
	حرف *sep;
	حرف *default_val = عدم;
	صحيح سونپيو = 0;
	صحيح expand_خالي_جايون = 0;

	إذا (!src)
		أرجع عدم;

	sep = strchr(src, ':');

	إذا (sep) {
		*sep = '\0';
		إذا (*(sep + 1) == '-')
			default_val = sep+2;
		إذا (*(sep + 1) == '=') {
			default_val = sep+2;
			سونپيو = 1;
		}
		إذا (*(sep + 1) == '+') {
			default_val = sep+2;
			expand_خالي_جايون = 1;
		}
	}

	p = env_get(src);
	إذا (!p)
		p = get_local_var(src);

	إذا (!p || لڙي_ڊيگهه(p) == 0) {
		p = default_val;
		إذا (سونپيو) {
			حرف *var = يادگيري_ڏيو(لڙي_ڊيگهه(src)+لڙي_ڊيگهه(default_val)+2);
			إذا (var) {
				sprintf(var, "%s=%s", src, default_val);
				سيٽ_local_var(var, 0);
			}
			حرر(var);
		}
	} وإلا إذا (expand_خالي_جايون) {
		p += لڙي_ڊيگهه(p);
	}

	إذا (sep)
		*sep = ':';

	أرجع p;
}

#جيڪڏهن_وصف_ٿيل __U_BOOT__
ساكن حرف *get_dollar_var(حرف ch)
{
	ساكن حرف buf[40];

	buf[0] = '\0';
	بدل (ch) {
		حالة '?':
			sprintf(buf, "%u", (بنانشان صحيح)آخري_return_ڪوڊ);
			اكسر;
		افتراضي:
			أرجع عدم;
	}
	أرجع buf;
}
#جيڪڏهن_پورو

/* return code: 0 for OK, 1 for syntax error */
ساكن صحيح handle_dollar(o_string *dest, بنية p_context *ctx, بنية اندريون_str *داخلا)
{
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
	صحيح i, advance=0;
#وإلا
	صحيح advance=0;
#جيڪڏهن_پورو
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
	حرف sep[]=" ";
#جيڪڏهن_پورو
	صحيح ch = داخلا->peek(داخلا);  /* first character after the $ */
	debug_printf("handle_dollar: ch=%c\n",ch);
	إذا (اکر_آهي(ch)) {
		b_addchr(dest, SPECIAL_VAR_نشان);
		ctx->ٻار->sp++;
		طالما(ch=b_peek(داخلا),اکر_يا_انگ(ch) || ch=='_') {
			b_getch(داخلا);
			b_addchr(dest,ch);
		}
		b_addchr(dest, SPECIAL_VAR_نشان);
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
	} وإلا إذا (انگ_آهي(ch)) {
		i = ch-'0';  /* XXX is $0 special? */
		إذا (i<global_argc) {
			parse_string(dest, ctx, global_argv[i]); /* recursion */
		}
		advance = 1;
#جيڪڏهن_پورو
	} وإلا بدل (ch) {
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
		حالة '$':
			b_adduint(dest,getpid());
			advance = 1;
			اكسر;
		حالة '!':
			إذا (آخري_bg_pid > 0) b_adduint(dest, آخري_bg_pid);
			advance = 1;
			اكسر;
#جيڪڏهن_پورو
		حالة '?':
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
			b_adduint(dest,آخري_return_ڪوڊ);
#وإلا
			ctx->ٻار->sp++;
			b_addchr(dest, SPECIAL_VAR_نشان);
			b_addchr(dest, '$');
			b_addchr(dest, '?');
			b_addchr(dest, SPECIAL_VAR_نشان);
#جيڪڏهن_پورو
			advance = 1;
			اكسر;
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
		حالة '#':
			b_adduint(dest,global_argc ? global_argc-1 : 0);
			advance = 1;
			اكسر;
#جيڪڏهن_پورو
		حالة '{':
			b_addchr(dest, SPECIAL_VAR_نشان);
			ctx->ٻار->sp++;
			b_getch(داخلا);
			/* XXX maybe someone will try to escape the '}' */
			طالما(ch=b_getch(داخلا),ch!=EOF && ch!='}') {
				b_addchr(dest,ch);
			}
			إذا (ch != '}') {
				syntax();
				أرجع 1;
			}
			b_addchr(dest, SPECIAL_VAR_نشان);
			اكسر;
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
		حالة '(':
			b_getch(داخلا);
			process_command_subs(dest, ctx, داخلا, ')');
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
			error_نياپو("unhandled syntax: $%c",ch);
			أرجع 1;
			اكسر;
#جيڪڏهن_پورو
		افتراضي:
			b_addqchr(dest,'$',dest->quote);
	}
	/* Eat the character if the flag was set.  If the compiler
	 * is smart enough, we could substitute "b_getch(input);"
	 * for all the "advance = 1;" above, and also end up with
	 * a nice size-optimized program.  Hah!  That'll be the day.
	 */
	إذا (advance) b_getch(داخلا);
	أرجع 0;
}

#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
صحيح parse_string(o_string *dest, بنية p_context *ctx, ثابت حرف *src)
{
	بنية اندريون_str foo;
	تياري_string_اندريون_str(&foo, src);
	أرجع parse_وهڪرو(dest, ctx, &foo, '\0');
}
#جيڪڏهن_پورو

/* return code is 0 for normal exit, 1 for syntax error */
ساكن صحيح parse_وهڪرو(o_string *dest, بنية p_context *ctx,
			بنية اندريون_str *داخلا, صحيح پڇاڙي_trigger)
{
	بنانشان صحيح ch, m;
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
	صحيح redir_fd;
	redir_type redir_style;
#جيڪڏهن_پورو
	صحيح ايندڙ;

	/* Only double-quote state is handled in the state variable dest->quote.
	 * A single-quote triggers a bypass of the main loop until its mate is
	 * found.  When recursing, quote state is passed in via dest->quote. */

	debug_printf("parse_stream, end_trigger=%d\n",پڇاڙي_trigger);
	طالما ((ch=b_getch(داخلا))!=EOF) {
		m = map[ch];
#جيڪڏهن_وصف_ٿيل __U_BOOT__
		إذا (داخلا->__promptme == 0) أرجع 1;
#جيڪڏهن_پورو
		ايندڙ = (ch == '\n') ? 0 : b_peek(داخلا);

		debug_printf("parse_stream: ch=%c (%d) m=%d quote=%d - %c\n",
			ch >= ' ' ? ch : '.', ch, m,
			dest->quote, ctx->ڍير == عدم ? '*' : '.');

		إذا (m==0 || ((m==1 || m==2) && dest->quote)) {
			b_addqchr(dest, ch, dest->quote);
		} وإلا {
			إذا (m==2) {  /* unquoted IFS */
				إذا (پورو_لفظ(dest, ctx)) {
					أرجع 1;
				}
				/* If we aren't performing a substitution, treat a newline as a
				 * command separator.  */
				إذا (پڇاڙي_trigger != '\0' && ch=='\n')
					پورو_pipe(ctx,PIPE_SEQ);
			}
			إذا (ch == پڇاڙي_trigger && !dest->quote && ctx->w==RES_NONE) {
				debug_printf("leaving parse_stream (triggered)\n");
				أرجع 0;
			}
#إذا 0
			إذا (ch=='\n') {
				/* Yahoo!  Time to run with it! */
				پورو_pipe(ctx,PIPE_SEQ);
				لاڳيتو_list(ctx->list_سر);
				initialize_context(ctx);
			}
#جيڪڏهن_پورو
			إذا (m!=2) بدل (ch) {
		حالة '#':
			إذا (dest->ڊيگهائي == 0 && !dest->quote) {
				طالما(ch=b_peek(داخلا),ch!=EOF && ch!='\n') { b_getch(داخلا); }
			} وإلا {
				b_addqchr(dest, ch, dest->quote);
			}
			اكسر;
		حالة '\\':
			إذا (ايندڙ == EOF) {
				syntax();
				أرجع 1;
			}
			b_addqchr(dest, '\\', dest->quote);
			b_addqchr(dest, b_getch(داخلا), dest->quote);
			اكسر;
		حالة '$':
			إذا (handle_dollar(dest, ctx, داخلا)!=0) أرجع 1;
			اكسر;
		حالة '\'':
			dest->nonnull = 1;
			طالما(ch=b_getch(داخلا),ch!=EOF && ch!='\'') {
#جيڪڏهن_وصف_ٿيل __U_BOOT__
				إذا(داخلا->__promptme == 0) أرجع 1;
#جيڪڏهن_پورو
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
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
		حالة '`':
			process_command_subs(dest, ctx, داخلا, '`');
			اكسر;
		حالة '>':
			redir_fd = redirect_opt_انگ(dest);
			پورو_لفظ(dest, ctx);
			redir_style=REDIRECT_OVERWRITE;
			إذا (ايندڙ == '>') {
				redir_style=REDIRECT_APPEND;
				b_getch(داخلا);
			} وإلا إذا (ايندڙ == '(') {
				syntax();   /* until we support >(list) Process Substitution */
				أرجع 1;
			}
			تياري_redirect(ctx, redir_fd, redir_style, داخلا);
			اكسر;
		حالة '<':
			redir_fd = redirect_opt_انگ(dest);
			پورو_لفظ(dest, ctx);
			redir_style=REDIRECT_داخلا;
			إذا (ايندڙ == '<') {
				redir_style=REDIRECT_HEREIS;
				b_getch(داخلا);
			} وإلا إذا (ايندڙ == '>') {
				redir_style=REDIRECT_IO;
				b_getch(داخلا);
			} وإلا إذا (ايندڙ == '(') {
				syntax();   /* until we support <(list) Process Substitution */
				أرجع 1;
			}
			تياري_redirect(ctx, redir_fd, redir_style, داخلا);
			اكسر;
#جيڪڏهن_پورو
		حالة ';':
			پورو_لفظ(dest, ctx);
			پورو_pipe(ctx,PIPE_SEQ);
			اكسر;
		حالة '&':
			پورو_لفظ(dest, ctx);
			إذا (ايندڙ=='&') {
				b_getch(داخلا);
				پورو_pipe(ctx,PIPE_AND);
			} وإلا {
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
				پورو_pipe(ctx,PIPE_BG);
#وإلا
				syntax_err();
				أرجع 1;
#جيڪڏهن_پورو
			}
			اكسر;
		حالة '|':
			پورو_لفظ(dest, ctx);
			إذا (ايندڙ=='|') {
				b_getch(داخلا);
				پورو_pipe(ctx,PIPE_OR);
			} وإلا {
				/* we could pick up a file descriptor choice here
				 * with redirect_opt_num(), but bash doesn't do it.
				 * "echo foo 2| cat" yields "foo 2". */
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
				پورو_command(ctx);
#وإلا
				syntax_err();
				أرجع 1;
#جيڪڏهن_پورو
			}
			اكسر;
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
		حالة '(':
		حالة '{':
			إذا (parse_گروهه(dest, ctx, داخلا, ch)!=0) أرجع 1;
			اكسر;
		حالة ')':
		حالة '}':
			syntax();   /* Proper use of this character caught by end_trigger */
			أرجع 1;
			اكسر;
#جيڪڏهن_پورو
		حالة SUBSTED_VAR_نشان:
			dest->nonnull = 1;
			طالما (ch = b_getch(داخلا), ch != EOF &&
			    ch != SUBSTED_VAR_نشان) {
				debug_printf("subst, pass=%d\n", ch);
				إذا (داخلا->__promptme == 0)
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
	إذا (پڇاڙي_trigger != '\0') أرجع -1;
	أرجع 0;
}

ساكن فراغ mapset(ثابت بنانشان حرف *سيٽ, صحيح ڪوڊ)
{
	ثابت بنانشان حرف *s;
	لكل (s=سيٽ; *s; s++) map[*s] = ڪوڊ;
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
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
	mapset((uchar *)"\\$'\"`", 3);      /* never flow through */
	mapset((uchar *)"<>;&|(){}#", 1);   /* flow through if quoted */
#وإلا
	{
		uchar subst[2] = {SUBSTED_VAR_نشان, 0};
		mapset(subst, 3);       /* never flow through */
	}
	mapset((uchar *)"\\$'\"", 3);       /* never flow through */
	mapset((uchar *)";&|#", 1);         /* flow through if quoted */
#جيڪڏهن_پورو
	mapset(ifs, 2);            /* also flow through if quoted */
}

/* most recursion does not come through here, the exeception is
 * from builtin_source() */
ساكن صحيح parse_وهڪرو_outer(بنية اندريون_str *inp, صحيح flag)
{

	بنية p_context ctx;
	o_string عارضي=NULL_O_STRING;
	صحيح rcode;
#جيڪڏهن_وصف_ٿيل __U_BOOT__
	صحيح ڪوڊ = 1;
#جيڪڏهن_پورو
	افعل {
		ctx.type = flag;
		initialize_context(&ctx);
		update_ifs_map();
		إذا (!(flag & FLAG_PARSE_SEMICOLON) || (flag & FLAG_REPARSING)) mapset((uchar *)";$&|", 0);
		inp->promptmode=1;
		rcode = parse_وهڪرو(&عارضي, &ctx, inp,
				     flag & FLAG_CONT_ON_NEWLINE ? -1 : '\n');
#جيڪڏهن_وصف_ٿيل __U_BOOT__
		إذا (rcode == 1) flag_repeat = 0;
#جيڪڏهن_پورو
		إذا (rcode != 1 && ctx.old_flag != 0) {
			syntax();
#جيڪڏهن_وصف_ٿيل __U_BOOT__
			flag_repeat = 0;
#جيڪڏهن_پورو
		}
		إذا (rcode != 1 && ctx.old_flag == 0) {
			پورو_لفظ(&عارضي, &ctx);
			پورو_pipe(&ctx,PIPE_SEQ);
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
			لاڳيتو_list(ctx.list_سر);
#وإلا
			ڪوڊ = لاڳيتو_list(ctx.list_سر);
			إذا (ڪوڊ == -2) {	/* exit */
				b_free(&عارضي);
				ڪوڊ = 0;
				/* XXX hackish way to not allow exit from main loop */
				إذا (inp->peek == file_peek) {
					لکو("exit not allowed from main input shell.\n");
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
			إذا (ڪوڊ == -1)
			    flag_repeat = 0;
#جيڪڏهن_پورو
		} وإلا {
			إذا (ctx.old_flag != 0) {
				حرر(ctx.ڍير);
				b_reset(&عارضي);
			}
#جيڪڏهن_وصف_ٿيل __U_BOOT__
			إذا (inp->__promptme == 0) لکو("<INTERRUPT>\n");
			inp->__promptme = 1;
#جيڪڏهن_پورو
			عارضي.nonnull = 0;
			عارضي.quote = 0;
			inp->p = عدم;
			free_pipe_list(ctx.list_سر,0);
		}
		b_free(&عارضي);
	/* loop on syntax errors, return on EOF */
	} طالما (rcode != -1 && !(flag & FLAG_EXIT_FROM_LOOP) &&
		(inp->peek != static_peek || b_peek(inp)));
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
	أرجع 0;
#وإلا
	أرجع (ڪوڊ != 0) ? 1 : 0;
#جيڪڏهن_پورو /* __U_BOOT__ */
}

#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
ساكن صحيح parse_string_outer(ثابت حرف *s, صحيح flag)
#وإلا
صحيح parse_string_outer(ثابت حرف *s, صحيح flag)
#جيڪڏهن_پورو	/* __U_BOOT__ */
{
	بنية اندريون_str داخلا;
	صحيح rcode;
#جيڪڏهن_وصف_ٿيل __U_BOOT__
	حرف *p = عدم;
	إذا (!s)
		أرجع 1;
	إذا (!*s)
		أرجع 0;
	إذا (!(p = strchr(s, '\n')) || *++p) {
		p = xmalloc(لڙي_ڊيگهه(s) + 2);
		strcpy(p, s);
		strcat(p, "\n");
		تياري_string_اندريون_str(&داخلا, p);
		rcode = parse_وهڪرو_outer(&داخلا, flag);
		حرر(p);
		أرجع rcode == -2 ? آخري_return_ڪوڊ : rcode;
	} وإلا {
#جيڪڏهن_پورو
	تياري_string_اندريون_str(&داخلا, s);
	rcode = parse_وهڪرو_outer(&داخلا, flag);
	أرجع rcode == -2 ? آخري_return_ڪوڊ : rcode;
#جيڪڏهن_وصف_ٿيل __U_BOOT__
	}
#جيڪڏهن_پورو
}

#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
ساكن صحيح parse_file_outer(FILE *f)
#وإلا
صحيح parse_file_outer(فراغ)
#جيڪڏهن_پورو
{
	صحيح rcode;
	بنية اندريون_str داخلا;
#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
	تياري_file_اندريون_str(&داخلا, f);
#وإلا
	تياري_file_اندريون_str(&داخلا);
#جيڪڏهن_پورو
	rcode = parse_وهڪرو_outer(&داخلا, FLAG_PARSE_SEMICOLON);
	أرجع rcode == -2 ? آخري_return_ڪوڊ : rcode;
}

#جيڪڏهن_وصف_ٿيل __U_BOOT__
صحيح u_boot_hush_شروعات(فراغ)
{
	إذا (چوٽي_vars == عدم) {
		چوٽي_vars = يادگيري_ڏيو(حجم(بنية variables));
		چوٽي_vars->name = "HUSH_VERSION";
		چوٽي_vars->قدر = "0.01";
		چوٽي_vars->ايندڙ = عدم;
		چوٽي_vars->flg_export = 0;
		چوٽي_vars->flg_read_only = 1;
	}
	أرجع 0;
}

ساكن فراغ *xmalloc(size_t سائيز)
{
	فراغ *p = عدم;

	إذا (!(p = يادگيري_ڏيو(سائيز))) {
	    لکو("ERROR : xmalloc failed\n");
	    لكل(;;);
	}
	أرجع p;
}

ساكن فراغ *xrealloc(فراغ *ptr, size_t سائيز)
{
	فراغ *p = عدم;

	إذا (!(p = realloc(ptr, سائيز))) {
	    لکو("ERROR : xrealloc failed\n");
	    لكل(;;);
	}
	أرجع p;
}
#جيڪڏهن_پورو /* __U_BOOT__ */

#جيڪڏهن_وصف_نه_ٿيل __U_BOOT__
/* Make sure we have a controlling tty.  If we get started under a job
 * aware app (like bash for example), make sure we are now in charge so
 * we don't fight over who gets the foreground */
ساكن فراغ تياري_ڪم_control(فراغ)
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
	FILE *داخلا;
	حرف **e = environ;

	/* XXX what should these be while sourcing /etc/profile? */
	global_argc = argc;
	global_argv = argv;

	/* (re?) initialize globals.  Sometimes hush_main() ends up calling
	 * hush_main(), therefore we cannot rely on the BSS to zero out this
	 * stuff.  Reset these to 0 every time. */
	ifs = عدم;
	/* map[] is taken care of with call to update_ifs_map() */
	fake_گهڻائي = 0;
	interactive = 0;
	close_me_سر = عدم;
	آخري_bg_pid = 0;
	ڪم_list = عدم;
	آخري_jobid = 0;

	/* Initialize some more globals to non-zero values */
	سيٽ_cwd();
#جيڪڏهن_وصف_ٿيل CONFIG_FEATURE_COMMAND_EDITING
	cmdedit_سيٽ_initial_prompt();
#وإلا
	PS1 = عدم;
#جيڪڏهن_پورو
	PS2 = "> ";

	/* initialize our shell local variables with the values
	 * currently living in the environment */
	إذا (e) {
		لكل (; *e; e++)
			سيٽ_local_var(*e, 2);   /* without call putenv() */
	}

	آخري_return_ڪوڊ=EXIT_SUCCESS;

	إذا (argv[0] && argv[0][0] == '-') {
		debug_printf("\nsourcing /etc/profile\n");
		إذا ((داخلا = fopen("/etc/profile", "r")) != عدم) {
			mark_open(fileno(داخلا));
			parse_file_outer(داخلا);
			mark_closed(fileno(داخلا));
			fclose(داخلا);
		}
	}
	داخلا=stdin;

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
				fake_گهڻائي++;
				اكسر;
			افتراضي:
#جيڪڏهن_وصف_نه_ٿيل BB_VER
				fprintf(stderr, "Usage: sh [FILE]...\n"
						"   or: sh -c command [args]...\n\n");
				ٻاهر_نڪرو(EXIT_FAILURE);
#وإلا
				show_usage();
#جيڪڏهن_پورو
		}
	}
	/* A shell is interactive if the `-i' flag was given, or if all of
	 * the following conditions are met:
	 *	  no -c command
	 *    no arguments remaining or the -s flag given
	 *    standard input is a terminal
	 *    standard output is a terminal
	 *    Refer to Posix.2, the description of the `sh' utility. */
	إذا (argv[optind]==عدم && داخلا==stdin &&
			isatty(fileno(stdin)) && isatty(fileno(stdout))) {
		interactive++;
	}

	debug_printf("\ninteractive=%d\n", interactive);
	إذا (interactive) {
		/* Looks like they want an interactive shell */
#جيڪڏهن_وصف_نه_ٿيل CONFIG_FEATURE_SH_EXTRA_QUIET
		لکو( "\n\n" BB_BANNER " hush - the humble shell v0.01 (testing)\n");
		لکو( "Enter 'help' for a list of built-in commands.\n\n");
#جيڪڏهن_پورو
		تياري_ڪم_control();
	}

	إذا (argv[optind]==عدم) {
		opt=parse_file_outer(stdin);
		اذهب final_return;
	}

	debug_printf("\nrunning script '%s'\n", argv[optind]);
	global_argv = argv+optind;
	global_argc = argc-optind;
	داخلا = xfopen(argv[optind], "r");
	opt = parse_file_outer(داخلا);

#جيڪڏهن_وصف_ٿيل CONFIG_FEATURE_CLEAN_مٿي
	fclose(داخلا);
	إذا (cwd && cwd != unknown)
		حرر((حرف*)cwd);
	{
		بنية variables *cur, *tmp;
		لكل(cur = چوٽي_vars; cur; cur = tmp) {
			tmp = cur->ايندڙ;
			إذا (!cur->flg_read_only) {
				حرر(cur->name);
				حرر(cur->قدر);
				حرر(cur);
			}
		}
	}
#جيڪڏهن_پورو

final_return:
	أرجع(opt?opt:آخري_return_ڪوڊ);
}
#جيڪڏهن_پورو

ساكن حرف *وجهو_var_قدر(حرف *inp)
{
	أرجع وجهو_var_قدر_sub(inp, 0);
}

ساكن حرف *وجهو_var_قدر_sub(حرف *inp, صحيح ٽيگ_subst)
{
	صحيح res_str_len = 0;
	صحيح len;
	صحيح پورو = 0;
	حرف *p, *p1, *res_str = عدم;

	طالما ((p = strchr(inp, SPECIAL_VAR_نشان))) {
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
		p = strchr(inp, SPECIAL_VAR_نشان);
		*p = '\0';
		/* look up the value to substitute */
		إذا ((p1 = lookup_param(inp))) {
			إذا (ٽيگ_subst)
				len = res_str_len + لڙي_ڊيگهه(p1) + 2;
			وإلا
				len = res_str_len + لڙي_ڊيگهه(p1);
			res_str = xrealloc(res_str, (1 + len));
			إذا (ٽيگ_subst) {
				/*
				 * copy the variable value to the result
				 * string
				 */
				strcpy((res_str + res_str_len + 1), p1);

				/*
				 * mark the replaced text to be accepted as
				 * is
				 */
				res_str[res_str_len] = SUBSTED_VAR_نشان;
				res_str[res_str_len + 1 + لڙي_ڊيگهه(p1)] =
					SUBSTED_VAR_نشان;
			} وإلا
				/*
				 * copy the variable value to the result
				 * string
				 */
				strcpy((res_str + res_str_len), p1);

			res_str_len = len;
		}
		*p = SPECIAL_VAR_نشان;
		inp = ++p;
		پورو = 1;
	}
	إذا (پورو) {
		res_str = xrealloc(res_str, (1 + res_str_len + لڙي_ڊيگهه(inp)));
		strcpy((res_str + res_str_len), inp);
		طالما ((p = strchr(res_str, '\n'))) {
			*p = ' ';
		}
	}
	أرجع (res_str == عدم) ? inp : res_str;
}

ساكن حرف **make_list_اندريون(حرف **inp, حرف *name)
{
	صحيح len, i;
	صحيح name_len = لڙي_ڊيگهه(name);
	صحيح n = 0;
	حرف **list;
	حرف *p1, *p2, *p3;

	/* create list of variable values */
	list = xmalloc(حجم(*list));
	لكل (i = 0; inp[i]; i++) {
		p3 = وجهو_var_قدر(inp[i]);
		p1 = p3;
		طالما (*p1) {
			إذا (*p1 == ' ') {
				p1++;
				استمر;
			}
			إذا ((p2 = strchr(p1, ' '))) {
				len = p2 - p1;
			} وإلا {
				len = لڙي_ڊيگهه(p1);
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
		p = وجهو_var_قدر_sub(inp[n], noeval);
		حرف *new_str = xrealloc(str, (len + لڙي_ڊيگهه(p) + (2 * nonnull[n])));
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
		len = لڙي_ڊيگهه(str) + 3;
		إذا (p != inp[n]) حرر(p);
	}
	len = لڙي_ڊيگهه(str);
	*(str + len) = '\n';
	*(str + len + 1) = '\0';
	أرجع str;
}

#جيڪڏهن_وصف_ٿيل __U_BOOT__
ساكن صحيح do_showvar(بنية cmd_tbl *cmdtp, صحيح flag, صحيح argc,
		      حرف *ثابت argv[])
{
	صحيح i, k;
	صحيح rcode = 0;
	بنية variables *cur;

	إذا (argc == 1) {		/* Print all env variables	*/
		لكل (cur = چوٽي_vars; cur; cur = cur->ايندڙ) {
			لکو ("%s=%s\n", cur->name, cur->قدر);
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
		لكل (cur = چوٽي_vars; cur; cur = cur->ايندڙ) {
			إذا(strcmp (cur->name, name) == 0) {
				k = 0;
				لکو ("%s=%s\n", cur->name, cur->قدر);
			}
			إذا (ctrlc ()) {
				puts ("\n ** Abort\n");
				أرجع 1;
			}
		}
		إذا (k < 0) {
			لکو ("## Error: \"%s\" not defined\n", name);
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

#جيڪڏهن_پورو
/****************************************************************************/
