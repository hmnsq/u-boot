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

#घोषणा __U_BOOT__
#अगर_घोषित __U_BOOT__
#समावेश <env.h>
#समावेश <malloc.h>         /* malloc, free, realloc*/
#समावेश <linux/ctype.h>    /* isalpha, isdigit */
#समावेश <console.h>
#समावेश <bootretry.h>
#समावेश <cli.h>
#समावेश <cli_hush.h>
#समावेश <command.h>        /* find_cmd */
#समावेश <vsprintf.h>
#पूर्ण_अगर
#अगर_अघोषित __U_BOOT__
#समावेश <ctype.h>     /* isalpha, isdigit */
#समावेश <unistd.h>    /* getpid */
#समावेश <stdlib.h>    /* getenv, atoi */
#समावेश <string.h>    /* strchr */
#समावेश <stdio.h>     /* popen etc. */
#समावेश <glob.h>      /* glob, of course */
#समावेश <stdarg.h>    /* va_list */
#समावेश <errno.h>
#समावेश <fcntl.h>
#समावेश <getopt.h>    /* should be pretty obvious */

#समावेश <sys/stat.h>  /* ulimit */
#समावेश <sys/types.h>
#समावेश <sys/wait.h>
#समावेश <signal.h>

/* #include <dmalloc.h> */

#अगर 1
#समावेश "busybox.h"
#समावेश "cmdedit.h"
#अन्यथा
#घोषणा applet_name "hush"
#समावेश "standalone.h"
#घोषणा hush_main मुख्य
#घोषणा BB_BANNER
#पूर्ण_अगर
#पूर्ण_अगर
#घोषणा SPECIAL_VAR_प्रतीक 03
#घोषणा SUBSTED_VAR_प्रतीक 04
#अगर_अघोषित __U_BOOT__
#घोषणा FLAG_EXIT_FROM_LOOP 1
#घोषणा FLAG_PARSE_SEMICOLON (1 << 1)		/* symbol ';' is special for parser */
#घोषणा FLAG_REPARSING       (1 << 2)		/* >= 2nd pass */

#पूर्ण_अगर

#अगर_घोषित __U_BOOT__
#घोषणा निकास_सफल 0
#घोषणा खातापूर्ण -1
#घोषणा syntax() syntax_err()
#घोषणा xstrdup strdup
#घोषणा error_संदेश म_लिखो
#अन्यथा
प्रकार क्रमागत {
	REDIRECT_निवेश     = 1,
	REDIRECT_OVERWRITE = 2,
	REDIRECT_APPEND    = 3,
	REDIRECT_HEREIS    = 4,
	REDIRECT_IO        = 5
} redir_type;

/* The descrip member of this structure is only used to make debugging
 * output pretty */
काष्ठा {पूर्णांक mode; पूर्णांक default_fd; अक्षर *descrip;} redir_table[] = {
	{ 0,                         0, "()" },
	{ O_RDONLY,                  0, "<"  },
	{ O_CREAT|O_TRUNC|O_WRONLY,  1, ">"  },
	{ O_CREAT|O_APPEND|O_WRONLY, 1, ">>" },
	{ O_RDONLY,                 -1, "<<" },
	{ O_RDWR,                    1, "<>" }
};
#पूर्ण_अगर

प्रकार क्रमागत {
	PIPE_SEQ = 1,
	PIPE_AND = 2,
	PIPE_OR  = 3,
	PIPE_BG  = 4,
} pipe_style;

/* might eventually control execution */
प्रकार क्रमागत {
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
#घोषणा FLAG_END   (1<<RES_NONE)
#घोषणा FLAG_IF    (1<<RES_IF)
#घोषणा FLAG_THEN  (1<<RES_THEN)
#घोषणा FLAG_ELIF  (1<<RES_ELIF)
#घोषणा FLAG_ELSE  (1<<RES_ELSE)
#घोषणा FLAG_FI    (1<<RES_FI)
#घोषणा FLAG_FOR   (1<<RES_FOR)
#घोषणा FLAG_WHILE (1<<RES_WHILE)
#घोषणा FLAG_UNTIL (1<<RES_UNTIL)
#घोषणा FLAG_DO    (1<<RES_DO)
#घोषणा FLAG_DONE  (1<<RES_DONE)
#घोषणा FLAG_IN    (1<<RES_IN)
#घोषणा FLAG_START (1<<RES_XXXX)

/* This holds pointers to the various results of parsing */
काष्ठा p_context {
	काष्ठा child_prog *child;
	काष्ठा pipe *list_शीर्ष;
	काष्ठा pipe *pipe;
#अगर_अघोषित __U_BOOT__
	काष्ठा redir_struct *pending_redirect;
#पूर्ण_अगर
	reserved_style w;
	पूर्णांक old_flag;				/* for figuring out valid reserved words */
	काष्ठा p_context *ढेर;
	पूर्णांक type;			/* define type of parser : ";$" common or special symbol */
	/* How about quoting status? */
};

#अगर_अघोषित __U_BOOT__
काष्ठा redir_struct {
	redir_type type;			/* type of redirection */
	पूर्णांक fd;						/* file descriptor being redirected */
	पूर्णांक dup;					/* -1, or file descriptor being duplicated */
	काष्ठा redir_struct *अग्र;	/* pointer to the next redirect in the list */
	glob_t word;				/* *word.gl_pathv is the filename */
};
#पूर्ण_अगर

काष्ठा child_prog {
#अगर_अघोषित __U_BOOT__
	pid_t pid;					/* 0 if exited */
#पूर्ण_अगर
	अक्षर **argv;				/* program name and arguments */
	/* was quoted when parsed; copy of struct o_string.nonnull field */
	पूर्णांक *argv_nonnull;
#अगर_घोषित __U_BOOT__
	पूर्णांक    argc;                            /* number of program arguments */
#पूर्ण_अगर
	काष्ठा pipe *group;			/* if non-NULL, first in group or subshell */
#अगर_अघोषित __U_BOOT__
	पूर्णांक subshell;				/* flag, non-zero if group must be forked */
	काष्ठा redir_struct *redirects;	/* I/O redirections */
	glob_t glob_परिणाम;			/* result of parameter globbing */
	पूर्णांक is_stopped;				/* is the program currently running? */
	काष्ठा pipe *family;		/* pointer back to the child's parent pipe */
#पूर्ण_अगर
	पूर्णांक sp;				/* number of SPECIAL_VAR_SYMBOL */
	पूर्णांक type;
};

काष्ठा pipe {
#अगर_अघोषित __U_BOOT__
	पूर्णांक jobid;					/* job number */
#पूर्ण_अगर
	पूर्णांक अंक_progs;				/* total number of programs in job */
#अगर_अघोषित __U_BOOT__
	पूर्णांक running_progs;			/* number of programs running */
	अक्षर *text;					/* name of job */
	अक्षर *cmdbuf;				/* buffer various argv's point into */
	pid_t pgrp;					/* process group ID for the job */
#पूर्ण_अगर
	काष्ठा child_prog *progs;	/* array of commands in pipe */
	काष्ठा pipe *अग्र;			/* to track background commands */
#अगर_अघोषित __U_BOOT__
	पूर्णांक stopped_progs;			/* number of programs alive, but stopped */
	पूर्णांक job_context;			/* bitmask defining current context */
#पूर्ण_अगर
	pipe_style followup;		/* PIPE_BG, PIPE_SEQ, PIPE_OR, PIPE_AND */
	reserved_style r_mode;		/* supports if, for, while, until */
};

#अगर_अघोषित __U_BOOT__
काष्ठा close_me {
	पूर्णांक fd;
	काष्ठा close_me *अग्र;
};
#पूर्ण_अगर

काष्ठा variables {
	अक्षर *name;
	अक्षर *मूल्य;
	पूर्णांक flg_export;
	पूर्णांक flg_read_only;
	काष्ठा variables *अग्र;
};

/* globals, connect us to the outside world
 * the first three support $?, $#, and $1 */
#अगर_अघोषित __U_BOOT__
अक्षर **global_argv;
अचिन्हित पूर्णांक global_argc;
#पूर्ण_अगर
अटल अचिन्हित पूर्णांक last_return_code;
#अगर_अघोषित __U_BOOT__
बाह्य अक्षर **environ; /* This is in <unistd.h>, but protected with __USE_GNU */
#पूर्ण_अगर

/* "globals" within this file */
अटल uchar *ifs;
अटल अक्षर map[256];
#अगर_अघोषित __U_BOOT__
अटल पूर्णांक fake_mode;
अटल पूर्णांक interactive;
अटल काष्ठा close_me *close_me_शीर्ष;
अटल स्थिर अक्षर *cwd;
अटल काष्ठा pipe *job_list;
अटल अचिन्हित पूर्णांक last_bg_pid;
अटल अचिन्हित पूर्णांक last_jobid;
अटल अचिन्हित पूर्णांक shell_terminal;
अटल अक्षर *PS1;
अटल अक्षर *PS2;
काष्ठा variables shell_ver = { "HUSH_VERSION", "0.01", 1, 1, 0 };
काष्ठा variables *शीर्ष_vars = &shell_ver;
#अन्यथा
अटल पूर्णांक flag_repeat = 0;
अटल पूर्णांक do_repeat = 0;
अटल काष्ठा variables *शीर्ष_vars = शून्य ;
#पूर्ण_अगर /*__U_BOOT__ */

#घोषणा B_CHUNK (100)
#घोषणा B_NOSPAC 1

प्रकार काष्ठा {
	अक्षर *तथ्य;
	पूर्णांक लंबाई;
	पूर्णांक maxlen;
	पूर्णांक quote;
	पूर्णांक nonnull;
} o_string;
#घोषणा NULL_O_STRING {शून्य,0,0,0,0}
/* used for initialization:
	o_string foo = NULL_O_STRING; */

/* I can almost use ordinary FILE *.  Is open_memstream() universally
 * available?  Where is it documented? */
काष्ठा in_str {
	स्थिर अचिन्हित अक्षर *p;
#अगर_अघोषित __U_BOOT__
	अक्षर peek_buf[2];
#पूर्ण_अगर
	पूर्णांक __promptme;
	पूर्णांक promptmode;
#अगर_अघोषित __U_BOOT__
	खाता *file;
#पूर्ण_अगर
	पूर्णांक (*get) (काष्ठा in_str *);
	पूर्णांक (*peek) (काष्ठा in_str *);
};
#घोषणा b_getch(निवेश) ((निवेश)->get(निवेश))
#घोषणा b_peek(निवेश) ((निवेश)->peek(निवेश))

#अगर_अघोषित __U_BOOT__
#घोषणा JOB_STATUS_FORMAT "[%d] %-22s %.40s\n"

काष्ठा built_in_command {
	अक्षर *cmd;					/* name */
	अक्षर *descr;				/* description */
	पूर्णांक (*function) (काष्ठा child_prog *);	/* function ptr */
};
#पूर्ण_अगर

/* define DEBUG_SHELL for debugging output (obviously ;-)) */
#अगर 0
#घोषणा DEBUG_SHELL
#पूर्ण_अगर

/* This should be in utility.c */
#अगर_घोषित DEBUG_SHELL
#अगर_अघोषित __U_BOOT__
अटल व्योम debug_printf(स्थिर अक्षर *format, ...)
{
	बहु_सूची args;
	बहु_शुरू(args, format);
	भख_लिखो(मानक_त्रुटि, format, args);
	बहु_पूर्ण(args);
}
#अन्यथा
#घोषणा debug_printf(fmt,args...)	म_लिखो (fmt ,##args)
#पूर्ण_अगर
#अन्यथा
अटल अंतरभूत व्योम debug_printf(स्थिर अक्षर *format, ...) { }
#पूर्ण_अगर
#घोषणा final_printf debug_printf

#अगर_घोषित __U_BOOT__
अटल व्योम syntax_err(व्योम) {
	 म_लिखो("syntax error\n");
}
#अन्यथा
अटल व्योम __syntax(अक्षर *file, पूर्णांक पंक्ति) {
	error_संदेश("syntax error %s:%d", file, पंक्ति);
}
#घोषणा syntax() __syntax(__खाता__, __LINE__)
#पूर्ण_अगर

#अगर_घोषित __U_BOOT__
अटल व्योम *xmalloc(माप_प्रकार आकार);
अटल व्योम *xrealloc(व्योम *ptr, माप_प्रकार आकार);
#अन्यथा
/* Index of subroutines: */
/*   function prototypes for builtins */
अटल पूर्णांक builtin_cd(काष्ठा child_prog *child);
अटल पूर्णांक builtin_env(काष्ठा child_prog *child);
अटल पूर्णांक builtin_eval(काष्ठा child_prog *child);
अटल पूर्णांक builtin_exec(काष्ठा child_prog *child);
अटल पूर्णांक builtin_exit(काष्ठा child_prog *child);
अटल पूर्णांक builtin_export(काष्ठा child_prog *child);
अटल पूर्णांक builtin_fg_bg(काष्ठा child_prog *child);
अटल पूर्णांक builtin_help(काष्ठा child_prog *child);
अटल पूर्णांक builtin_jobs(काष्ठा child_prog *child);
अटल पूर्णांक builtin_pwd(काष्ठा child_prog *child);
अटल पूर्णांक builtin_read(काष्ठा child_prog *child);
अटल पूर्णांक builtin_set(काष्ठा child_prog *child);
अटल पूर्णांक builtin_shift(काष्ठा child_prog *child);
अटल पूर्णांक builtin_source(काष्ठा child_prog *child);
अटल पूर्णांक builtin_umask(काष्ठा child_prog *child);
अटल पूर्णांक builtin_unset(काष्ठा child_prog *child);
अटल पूर्णांक builtin_not_written(काष्ठा child_prog *child);
#पूर्ण_अगर
/*   o_string manipulation: */
अटल पूर्णांक b_check_space(o_string *o, पूर्णांक len);
अटल पूर्णांक b_addchr(o_string *o, पूर्णांक ch);
अटल व्योम b_reset(o_string *o);
अटल पूर्णांक b_addqchr(o_string *o, पूर्णांक ch, पूर्णांक quote);
#अगर_अघोषित __U_BOOT__
अटल पूर्णांक b_adduint(o_string *o, अचिन्हित पूर्णांक i);
#पूर्ण_अगर
/*  in_str manipulations: */
अटल पूर्णांक static_get(काष्ठा in_str *i);
अटल पूर्णांक static_peek(काष्ठा in_str *i);
अटल पूर्णांक file_get(काष्ठा in_str *i);
अटल पूर्णांक file_peek(काष्ठा in_str *i);
#अगर_अघोषित __U_BOOT__
अटल व्योम setup_file_in_str(काष्ठा in_str *i, खाता *f);
#अन्यथा
अटल व्योम setup_file_in_str(काष्ठा in_str *i);
#पूर्ण_अगर
अटल व्योम setup_string_in_str(काष्ठा in_str *i, स्थिर अक्षर *s);
#अगर_अघोषित __U_BOOT__
/*  close_me manipulations: */
अटल व्योम mark_open(पूर्णांक fd);
अटल व्योम mark_closed(पूर्णांक fd);
अटल व्योम close_all(व्योम);
#पूर्ण_अगर
/*  "run" the final data structures: */
अटल अक्षर *indenter(पूर्णांक i);
अटल पूर्णांक free_pipe_list(काष्ठा pipe *शीर्ष_2, पूर्णांक indent);
अटल पूर्णांक free_pipe(काष्ठा pipe *pi, पूर्णांक indent);
/*  really run the final data structures: */
#अगर_अघोषित __U_BOOT__
अटल पूर्णांक setup_redirects(काष्ठा child_prog *prog, पूर्णांक squirrel[]);
#पूर्ण_अगर
अटल पूर्णांक run_list_real(काष्ठा pipe *pi);
#अगर_अघोषित __U_BOOT__
अटल व्योम pseudo_exec(काष्ठा child_prog *child) __attribute__ ((noreturn));
#पूर्ण_अगर
अटल पूर्णांक run_pipe_real(काष्ठा pipe *pi);
/*   extended glob support: */
#अगर_अघोषित __U_BOOT__
अटल पूर्णांक globhack(स्थिर अक्षर *src, पूर्णांक flags, glob_t *pglob);
अटल पूर्णांक glob_needed(स्थिर अक्षर *s);
अटल पूर्णांक xglob(o_string *dest, पूर्णांक flags, glob_t *pglob);
#पूर्ण_अगर
/*   variable assignment: */
अटल पूर्णांक is_assignment(स्थिर अक्षर *s);
/*   data structure manipulation: */
#अगर_अघोषित __U_BOOT__
अटल पूर्णांक setup_redirect(काष्ठा p_context *ctx, पूर्णांक fd, redir_type style, काष्ठा in_str *निवेश);
#पूर्ण_अगर
अटल व्योम initialize_context(काष्ठा p_context *ctx);
अटल पूर्णांक done_word(o_string *dest, काष्ठा p_context *ctx);
अटल पूर्णांक done_command(काष्ठा p_context *ctx);
अटल पूर्णांक done_pipe(काष्ठा p_context *ctx, pipe_style type);
/*   primary string parsing: */
#अगर_अघोषित __U_BOOT__
अटल पूर्णांक redirect_dup_अंक(काष्ठा in_str *निवेश);
अटल पूर्णांक redirect_opt_अंक(o_string *o);
अटल पूर्णांक process_command_subs(o_string *dest, काष्ठा p_context *ctx, काष्ठा in_str *निवेश, पूर्णांक subst_end);
अटल पूर्णांक parse_group(o_string *dest, काष्ठा p_context *ctx, काष्ठा in_str *निवेश, पूर्णांक ch);
#पूर्ण_अगर
अटल अक्षर *lookup_param(अक्षर *src);
अटल अक्षर *make_string(अक्षर **inp, पूर्णांक *nonnull);
अटल पूर्णांक handle_dollar(o_string *dest, काष्ठा p_context *ctx, काष्ठा in_str *निवेश);
#अगर_अघोषित __U_BOOT__
अटल पूर्णांक parse_string(o_string *dest, काष्ठा p_context *ctx, स्थिर अक्षर *src);
#पूर्ण_अगर
अटल पूर्णांक parse_stream(o_string *dest, काष्ठा p_context *ctx, काष्ठा in_str *input0, पूर्णांक end_trigger);
/*   setup: */
अटल पूर्णांक parse_stream_outer(काष्ठा in_str *inp, पूर्णांक flag);
#अगर_अघोषित __U_BOOT__
अटल पूर्णांक parse_string_outer(स्थिर अक्षर *s, पूर्णांक flag);
अटल पूर्णांक parse_file_outer(खाता *f);
#पूर्ण_अगर
#अगर_अघोषित __U_BOOT__
/*   job management: */
अटल पूर्णांक checkjobs(काष्ठा pipe* fg_pipe);
अटल व्योम जोड़ो_bg_job(काष्ठा pipe *pi);
अटल व्योम remove_bg_job(काष्ठा pipe *pi);
#पूर्ण_अगर
/*     local variable support */
अटल अक्षर **make_list_in(अक्षर **inp, अक्षर *name);
अटल अक्षर *जोड़ो_var_मूल्य(अक्षर *inp);
अटल अक्षर *जोड़ो_var_मूल्य_sub(अक्षर *inp, पूर्णांक tag_subst);

#अगर_अघोषित __U_BOOT__
/* Table of built-in functions.  They can be forked or not, depending on
 * context: within pipes, they fork.  As simple commands, they do not.
 * When used in non-forking context, they can change global variables
 * in the parent shell process.  If forked, of course they can not.
 * For example, 'unset foo | whatever' will parse and run, but foo will
 * still be set at the end. */
अटल काष्ठा built_in_command bltins[] = {
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
	{शून्य, शून्य, शून्य}
};

अटल स्थिर अक्षर *set_cwd(व्योम)
{
	अगर(cwd==unknown)
		cwd = शून्य;     /* xgetcwd(arg) called free(arg) */
	cwd = xgetcwd((अक्षर *)cwd);
	अगर (!cwd)
		cwd = unknown;
	वापस cwd;
}

/* built-in 'eval' handler */
अटल पूर्णांक builtin_eval(काष्ठा child_prog *child)
{
	अक्षर *str = शून्य;
	पूर्णांक rcode = निकास_सफल;

	अगर (child->argv[1]) {
		str = make_string(child->argv + 1);
		parse_string_outer(str, FLAG_EXIT_FROM_LOOP |
					FLAG_PARSE_SEMICOLON);
		मुक्त(str);
		rcode = last_return_code;
	}
	वापस rcode;
}

/* built-in 'cd <path>' handler */
अटल पूर्णांक builtin_cd(काष्ठा child_prog *child)
{
	अक्षर *newdir;
	अगर (child->argv[1] == शून्य)
		newdir = env_get("HOME");
	अन्यथा
		newdir = child->argv[1];
	अगर (स_बदलो(newdir)) {
		म_लिखो("cd: %s: %s\n", newdir, म_त्रुटि(त्रुटि_सं));
		वापस निकास_त्रुटि;
	}
	set_cwd();
	वापस निकास_सफल;
}

/* built-in 'env' handler */
अटल पूर्णांक builtin_env(काष्ठा child_prog *dummy)
{
	अक्षर **e = environ;
	अगर (e == शून्य) वापस निकास_त्रुटि;
	क्रम (; *e; e++) {
		माला_दो(*e);
	}
	वापस निकास_सफल;
}

/* built-in 'exec' handler */
अटल पूर्णांक builtin_exec(काष्ठा child_prog *child)
{
	अगर (child->argv[1] == शून्य)
		वापस निकास_सफल;   /* Really? */
	child->argv++;
	pseudo_exec(child);
	/* never returns */
}

/* built-in 'exit' handler */
अटल पूर्णांक builtin_exit(काष्ठा child_prog *child)
{
	अगर (child->argv[1] == शून्य)
		निकास(last_return_code);
	निकास (म_से_प(child->argv[1]));
}

/* built-in 'export VAR=value' handler */
अटल पूर्णांक builtin_export(काष्ठा child_prog *child)
{
	पूर्णांक res = 0;
	अक्षर *name = child->argv[1];

	अगर (name == शून्य) {
		वापस (builtin_env(child));
	}

	name = strdup(name);

	अगर(name) {
		अक्षर *मूल्य = म_अक्षर(name, '=');

		अगर (!मूल्य) {
			अक्षर *tmp;
			/* They are exporting something without an =VALUE */

			मूल्य = get_local_var(name);
			अगर (मूल्य) {
				माप_प्रकार ln = म_माप(name);

				tmp = पुनः_स्मृति(name, ln+म_माप(मूल्य)+2);
				अगर(tmp==शून्य)
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
	अगर (res<0)
		perror_संदेश("export");
	अन्यथा अगर(res==0)
		res = set_local_var(name, 1);
	अन्यथा
		res = 0;
	मुक्त(name);
	वापस res;
}

/* built-in 'fg' and 'bg' handler */
अटल पूर्णांक builtin_fg_bg(काष्ठा child_prog *child)
{
	पूर्णांक i, jobnum;
	काष्ठा pipe *pi=शून्य;

	अगर (!interactive)
		वापस निकास_त्रुटि;
	/* If they gave us no args, assume they want the last backgrounded task */
	अगर (!child->argv[1]) {
		क्रम (pi = job_list; pi; pi = pi->अग्र) {
			अगर (pi->jobid == last_jobid) {
				अवरोध;
			}
		}
		अगर (!pi) {
			error_संदेश("%s: no current job", child->argv[0]);
			वापस निकास_त्रुटि;
		}
	} अन्यथा {
		अगर (माला_पूछो(child->argv[1], "%%%d", &jobnum) != 1) {
			error_संदेश("%s: bad argument '%s'", child->argv[0], child->argv[1]);
			वापस निकास_त्रुटि;
		}
		क्रम (pi = job_list; pi; pi = pi->अग्र) {
			अगर (pi->jobid == jobnum) {
				अवरोध;
			}
		}
		अगर (!pi) {
			error_संदेश("%s: %d: no such job", child->argv[0], jobnum);
			वापस निकास_त्रुटि;
		}
	}

	अगर (*child->argv[0] == 'f') {
		/* Put the job into the foreground.  */
		tcsetpgrp(shell_terminal, pi->pgrp);
	}

	/* Restart the processes in the job */
	क्रम (i = 0; i < pi->अंक_progs; i++)
		pi->progs[i].is_stopped = 0;

	अगर ( (i=समाप्त(- pi->pgrp, SIGCONT)) < 0) {
		अगर (i == ESRCH) {
			remove_bg_job(pi);
		} अन्यथा {
			perror_संदेश("kill (SIGCONT)");
		}
	}

	pi->stopped_progs = 0;
	वापस निकास_सफल;
}

/* built-in 'help' handler */
अटल पूर्णांक builtin_help(काष्ठा child_prog *dummy)
{
	काष्ठा built_in_command *x;

	म_लिखो("\nBuilt-in commands:\n");
	म_लिखो("-------------------\n");
	क्रम (x = bltins; x->cmd; x++) {
		अगर (x->descr==शून्य)
			जारी;
		म_लिखो("%s\t%s\n", x->cmd, x->descr);
	}
	म_लिखो("\n\n");
	वापस निकास_सफल;
}

/* built-in 'jobs' handler */
अटल पूर्णांक builtin_jobs(काष्ठा child_prog *child)
{
	काष्ठा pipe *job;
	अक्षर *status_string;

	क्रम (job = job_list; job; job = job->अग्र) {
		अगर (job->running_progs == job->stopped_progs)
			status_string = "Stopped";
		अन्यथा
			status_string = "Running";

		म_लिखो(JOB_STATUS_FORMAT, job->jobid, status_string, job->text);
	}
	वापस निकास_सफल;
}

/* built-in 'pwd' handler */
अटल पूर्णांक builtin_pwd(काष्ठा child_prog *dummy)
{
	माला_दो(set_cwd());
	वापस निकास_सफल;
}

/* built-in 'read VAR' handler */
अटल पूर्णांक builtin_read(काष्ठा child_prog *child)
{
	पूर्णांक res;

	अगर (child->argv[1]) {
		अक्षर string[बफ_मान];
		अक्षर *var = 0;

		string[0] = 0;  /* In case stdin has only EOF */
		/* read string */
		ख_माला_लो(string, माप(string), मानक_निवेश);
		chomp(string);
		var = दो_स्मृति(म_माप(child->argv[1])+म_माप(string)+2);
		अगर(var) {
			प्र_लिखो(var, "%s=%s", child->argv[1], string);
			res = set_local_var(var, 0);
		} अन्यथा
			res = -1;
		अगर (res)
			ख_लिखो(मानक_त्रुटि, "read: %m\n");
		मुक्त(var);      /* So not move up to avoid breaking errno */
		वापस res;
	} अन्यथा {
		करो res=अक्षर_लो(); जबतक(res!='\n' && res!=खातापूर्ण);
		वापस 0;
	}
}

/* built-in 'set VAR=value' handler */
अटल पूर्णांक builtin_set(काष्ठा child_prog *child)
{
	अक्षर *अस्थायी = child->argv[1];
	काष्ठा variables *e;

	अगर (अस्थायी == शून्य)
		क्रम(e = शीर्ष_vars; e; e=e->अग्र)
			म_लिखो("%s=%s\n", e->name, e->मूल्य);
	अन्यथा
		set_local_var(अस्थायी, 0);

		वापस निकास_सफल;
}

/* Built-in 'shift' handler */
अटल पूर्णांक builtin_shift(काष्ठा child_prog *child)
{
	पूर्णांक n=1;
	अगर (child->argv[1]) {
		n=म_से_प(child->argv[1]);
	}
	अगर (n>=0 && n<global_argc) {
		/* XXX This probably breaks $0 */
		global_argc -= n;
		global_argv += n;
		वापस निकास_सफल;
	} अन्यथा {
		वापस निकास_त्रुटि;
	}
}

/* Built-in '.' handler (read-in and execute commands from file) */
अटल पूर्णांक builtin_source(काष्ठा child_prog *child)
{
	खाता *निवेश;
	पूर्णांक status;

	अगर (child->argv[1] == शून्य)
		वापस निकास_त्रुटि;

	/* XXX search through $PATH is missing */
	निवेश = ख_खोलो(child->argv[1], "r");
	अगर (!निवेश) {
		error_संदेश("Couldn't open file '%s'", child->argv[1]);
		वापस निकास_त्रुटि;
	}

	/* Now run the file */
	/* XXX argv and argc are broken; need to save old global_argv
	 * (pointer only is OK!) on this stack frame,
	 * set global_argv=child->argv+1, recurse, and restore. */
	mark_open(fileno(निवेश));
	status = parse_file_outer(निवेश);
	mark_closed(fileno(निवेश));
	ख_बंद(निवेश);
	वापस (status);
}

अटल पूर्णांक builtin_umask(काष्ठा child_prog *child)
{
	mode_t new_umask;
	स्थिर अक्षर *arg = child->argv[1];
	अक्षर *end;
	अगर (arg) {
		new_umask=म_से_अदीर्घ(arg, &end, 8);
		अगर (*end!='\0' || end == arg) {
			वापस निकास_त्रुटि;
		}
	} अन्यथा {
		म_लिखो("%.3o\n", (अचिन्हित पूर्णांक) (new_umask=umask(0)));
	}
	umask(new_umask);
	वापस निकास_सफल;
}

/* built-in 'unset VAR' handler */
अटल पूर्णांक builtin_unset(काष्ठा child_prog *child)
{
	/* bash returned already true */
	unset_local_var(child->argv[1]);
	वापस निकास_सफल;
}

अटल पूर्णांक builtin_not_written(काष्ठा child_prog *child)
{
	म_लिखो("builtin_%s not written\n",child->argv[0]);
	वापस निकास_त्रुटि;
}
#पूर्ण_अगर

अटल पूर्णांक b_check_space(o_string *o, पूर्णांक len)
{
	/* It would be easy to drop a more restrictive policy
	 * in here, such as setting a maximum string length */
	अगर (o->लंबाई + len > o->maxlen) {
		अक्षर *old_तथ्य = o->तथ्य;
		/* assert (data == NULL || o->maxlen != 0); */
		o->maxlen += max(2*len, B_CHUNK);
		o->तथ्य = पुनः_स्मृति(o->तथ्य, 1 + o->maxlen);
		अगर (o->तथ्य == शून्य) {
			मुक्त(old_तथ्य);
		}
	}
	वापस o->तथ्य == शून्य;
}

अटल पूर्णांक b_addchr(o_string *o, पूर्णांक ch)
{
	debug_printf("b_addchr: %c %d %p\n", ch, o->लंबाई, o);
	अगर (b_check_space(o, 1)) वापस B_NOSPAC;
	o->तथ्य[o->लंबाई] = ch;
	o->लंबाई++;
	o->तथ्य[o->लंबाई] = '\0';
	वापस 0;
}

अटल व्योम b_reset(o_string *o)
{
	o->लंबाई = 0;
	o->nonnull = 0;
	अगर (o->तथ्य != शून्य) *o->तथ्य = '\0';
}

अटल व्योम b_free(o_string *o)
{
	b_reset(o);
	मुक्त(o->तथ्य);
	o->तथ्य = शून्य;
	o->maxlen = 0;
}

/* My analysis of quoting semantics tells me that state information
 * is associated with a destination, not a source.
 */
अटल पूर्णांक b_addqchr(o_string *o, पूर्णांक ch, पूर्णांक quote)
{
	अगर (quote && म_अक्षर("*?[\\",ch)) {
		पूर्णांक rc;
		rc = b_addchr(o, '\\');
		अगर (rc) वापस rc;
	}
	वापस b_addchr(o, ch);
}

#अगर_अघोषित __U_BOOT__
अटल पूर्णांक b_adduint(o_string *o, अचिन्हित पूर्णांक i)
{
	पूर्णांक r;
	अक्षर *p = simple_itoa(i);
	/* no escape checking necessary */
	करो r=b_addchr(o, *p++); जबतक (r==0 && *p);
	वापस r;
}
#पूर्ण_अगर

अटल पूर्णांक static_get(काष्ठा in_str *i)
{
	पूर्णांक ch = *i->p++;
	अगर (ch=='\0') वापस खातापूर्ण;
	वापस ch;
}

अटल पूर्णांक static_peek(काष्ठा in_str *i)
{
	वापस *i->p;
}

#अगर_अघोषित __U_BOOT__
अटल अंतरभूत व्योम cmdedit_set_initial_prompt(व्योम)
{
#अगर_अघोषित CONFIG_FEATURE_SH_FANCY_PROMPT
	PS1 = शून्य;
#अन्यथा
	PS1 = env_get("PS1");
	अगर(PS1==0)
		PS1 = "\\w \\$ ";
#पूर्ण_अगर
}

अटल अंतरभूत व्योम setup_prompt_string(पूर्णांक promptmode, अक्षर **prompt_str)
{
	debug_printf("setup_prompt_string %d ",promptmode);
#अगर_अघोषित CONFIG_FEATURE_SH_FANCY_PROMPT
	/* Set up the prompt */
	अगर (promptmode == 1) {
		मुक्त(PS1);
		PS1=xmalloc(म_माप(cwd)+4);
		प्र_लिखो(PS1, "%s %s", cwd, ( geteuid() != 0 ) ?  "$ ":"# ");
		*prompt_str = PS1;
	} अन्यथा {
		*prompt_str = PS2;
	}
#अन्यथा
	*prompt_str = (promptmode==1)? PS1 : PS2;
#पूर्ण_अगर
	debug_printf("result %s\n",*prompt_str);
}
#पूर्ण_अगर

#अगर_घोषित __U_BOOT__
अटल पूर्णांक uboot_cli_readline(काष्ठा in_str *i)
{
	अक्षर *prompt;
	अक्षर __maybe_unused *ps_prompt = शून्य;

	अगर (i->promptmode == 1)
		prompt = CONFIG_SYS_PROMPT;
	अन्यथा
		prompt = CONFIG_SYS_PROMPT_HUSH_PS2;

#अगर_घोषित CONFIG_CMDLINE_PS_SUPPORT
	अगर (i->promptmode == 1)
		ps_prompt = env_get("PS1");
	अन्यथा
		ps_prompt = env_get("PS2");
	अगर (ps_prompt)
		prompt = ps_prompt;
#पूर्ण_अगर

	वापस cli_readline(prompt);
}
#पूर्ण_अगर

अटल व्योम get_user_निवेश(काष्ठा in_str *i)
{
#अगर_अघोषित __U_BOOT__
	अक्षर *prompt_str;
	अटल अक्षर the_command[बफ_मान];

	setup_prompt_string(i->promptmode, &prompt_str);
#अगर_घोषित CONFIG_FEATURE_COMMAND_EDITING
	/*
	 ** enable command line editing only while a command line
	 ** is actually being read; otherwise, we'll end up bequeathing
	 ** atexit() handlers and other unwanted stuff to our
	 ** child processes (rob@sysgo.de)
	 */
	cmdedit_read_निवेश(prompt_str, the_command);
#अन्यथा
	ख_माला_दो(prompt_str, मानक_निकास);
	ख_साफ(मानक_निकास);
	the_command[0]=ख_अक्षर_लो(i->file);
	the_command[1]='\0';
#पूर्ण_अगर
	ख_साफ(मानक_निकास);
	i->p = the_command;
#अन्यथा
	पूर्णांक n;
	अटल अक्षर the_command[CONFIG_SYS_CBSIZE + 1];

	bootretry_reset_cmd_timeout();
	i->__promptme = 1;
	n = uboot_cli_readline(i);

#अगर_घोषित CONFIG_BOOT_RETRY_TIME
	अगर (n == -2) {
	  माला_दो("\nTimeout waiting for command\n");
#  ifdef CONFIG_RESET_TO_RETRY
	  do_reset(शून्य, 0, 0, शून्य);
#  elif IS_ENABLED(CONFIG_RETRY_BOOTCMD)
	म_नकल(console_बफर, "run bootcmd\n");
# अन्यथा
#	error "This only works with CONFIG_RESET_TO_RETRY or CONFIG_BOOT_RETRY_COMMAND enabled"
#  endif
	}
#पूर्ण_अगर
	अगर (n == -1 ) {
		flag_repeat = 0;
		i->__promptme = 0;
	}
	n = म_माप(console_बफर);
	console_बफर[n] = '\n';
	console_बफर[n+1]= '\0';
	अगर (had_ctrlc()) flag_repeat = 0;
	clear_ctrlc();
	do_repeat = 0;
	अगर (i->promptmode == 1) {
		अगर (console_बफर[0] == '\n'&& flag_repeat == 0) {
			म_नकल(the_command,console_बफर);
		}
		अन्यथा {
			अगर (console_बफर[0] != '\n') {
				म_नकल(the_command,console_बफर);
				flag_repeat = 1;
			}
			अन्यथा {
				do_repeat = 1;
			}
		}
		i->p = the_command;
	}
	अन्यथा {
		अगर (console_बफर[0] != '\n') {
			अगर (म_माप(the_command) + म_माप(console_बफर)
			    < CONFIG_SYS_CBSIZE) {
				n = म_माप(the_command);
				the_command[n-1] = ' ';
				म_नकल(&the_command[n],console_बफर);
			}
			अन्यथा {
				the_command[0] = '\n';
				the_command[1] = '\0';
				flag_repeat = 0;
			}
		}
		अगर (i->__promptme == 0) {
			the_command[0] = '\n';
			the_command[1] = '\0';
		}
		i->p = console_बफर;
	}
#पूर्ण_अगर
}

/* This is the magic location that prints prompts
 * and gets data back from the user */
अटल पूर्णांक file_get(काष्ठा in_str *i)
{
	पूर्णांक ch;

	ch = 0;
	/* If there is data waiting, eat it up */
	अगर (i->p && *i->p) {
		ch = *i->p++;
	} अन्यथा {
		/* need to double check i->file because we might be doing something
		 * more complicated by now, like sourcing or substituting. */
#अगर_अघोषित __U_BOOT__
		अगर (i->__promptme && interactive && i->file == मानक_निवेश) {
			जबतक(! i->p || (interactive && म_माप(i->p)==0) ) {
#अन्यथा
			जबतक(! i->p  || म_माप(i->p)==0 ) {
#पूर्ण_अगर
				get_user_निवेश(i);
			}
			i->promptmode=2;
#अगर_अघोषित __U_BOOT__
			i->__promptme = 0;
#पूर्ण_अगर
			अगर (i->p && *i->p) {
				ch = *i->p++;
			}
#अगर_अघोषित __U_BOOT__
		} अन्यथा {
			ch = ख_अक्षर_लो(i->file);
		}

#पूर्ण_अगर
		debug_printf("b_getch: got a %d\n", ch);
	}
#अगर_अघोषित __U_BOOT__
	अगर (ch == '\n') i->__promptme=1;
#पूर्ण_अगर
	वापस ch;
}

/* All the callers guarantee this routine will never be
 * used right after a newline, so prompting is not needed.
 */
अटल पूर्णांक file_peek(काष्ठा in_str *i)
{
#अगर_अघोषित __U_BOOT__
	अगर (i->p && *i->p) {
#पूर्ण_अगर
		वापस *i->p;
#अगर_अघोषित __U_BOOT__
	} अन्यथा {
		i->peek_buf[0] = ख_अक्षर_लो(i->file);
		i->peek_buf[1] = '\0';
		i->p = i->peek_buf;
		debug_printf("b_peek: got a %d\n", *i->p);
		वापस *i->p;
	}
#पूर्ण_अगर
}

#अगर_अघोषित __U_BOOT__
अटल व्योम setup_file_in_str(काष्ठा in_str *i, खाता *f)
#अन्यथा
अटल व्योम setup_file_in_str(काष्ठा in_str *i)
#पूर्ण_अगर
{
	i->peek = file_peek;
	i->get = file_get;
	i->__promptme=1;
	i->promptmode=1;
#अगर_अघोषित __U_BOOT__
	i->file = f;
#पूर्ण_अगर
	i->p = शून्य;
}

अटल व्योम setup_string_in_str(काष्ठा in_str *i, स्थिर अक्षर *s)
{
	i->peek = static_peek;
	i->get = static_get;
	i->__promptme=1;
	i->promptmode=1;
	i->p = s;
}

#अगर_अघोषित __U_BOOT__
अटल व्योम mark_open(पूर्णांक fd)
{
	काष्ठा close_me *new = xmalloc(माप(काष्ठा close_me));
	new->fd = fd;
	new->अग्र = close_me_शीर्ष;
	close_me_शीर्ष = new;
}

अटल व्योम mark_closed(पूर्णांक fd)
{
	काष्ठा close_me *tmp;
	अगर (close_me_शीर्ष == शून्य || close_me_शीर्ष->fd != fd)
		error_संदेश_and_die("corrupt close_me");
	tmp = close_me_शीर्ष;
	close_me_शीर्ष = close_me_शीर्ष->अग्र;
	मुक्त(tmp);
}

अटल व्योम close_all(व्योम)
{
	काष्ठा close_me *c;
	क्रम (c=close_me_शीर्ष; c; c=c->अग्र) {
		बंद(c->fd);
	}
	close_me_शीर्ष = शून्य;
}

/* squirrel != NULL means we squirrel away copies of stdin, stdout,
 * and stderr if they are redirected. */
अटल पूर्णांक setup_redirects(काष्ठा child_prog *prog, पूर्णांक squirrel[])
{
	पूर्णांक openfd, mode;
	काष्ठा redir_struct *redir;

	क्रम (redir=prog->redirects; redir; redir=redir->अग्र) {
		अगर (redir->dup == -1 && redir->word.gl_pathv == शून्य) {
			/* something went wrong in the parse.  Pretend it didn't happen */
			जारी;
		}
		अगर (redir->dup == -1) {
			mode=redir_table[redir->type].mode;
			openfd = खोलो(redir->word.gl_pathv[0], mode, 0666);
			अगर (openfd < 0) {
			/* this could get lost if stderr has been redirected, but
			   bash and ash both lose it as well (though zsh doesn't!) */
				perror_संदेश("error opening %s", redir->word.gl_pathv[0]);
				वापस 1;
			}
		} अन्यथा {
			openfd = redir->dup;
		}

		अगर (openfd != redir->fd) {
			अगर (squirrel && redir->fd < 3) {
				squirrel[redir->fd] = dup(redir->fd);
			}
			अगर (openfd == -3) {
				बंद(openfd);
			} अन्यथा {
				dup2(openfd, redir->fd);
				अगर (redir->dup == -1)
					बंद (openfd);
			}
		}
	}
	वापस 0;
}

अटल व्योम restore_redirects(पूर्णांक squirrel[])
{
	पूर्णांक i, fd;
	क्रम (i=0; i<3; i++) {
		fd = squirrel[i];
		अगर (fd != -1) {
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
अटल व्योम pseudo_exec(काष्ठा child_prog *child)
{
	पूर्णांक i, rcode;
	अक्षर *p;
	काष्ठा built_in_command *x;
	अगर (child->argv) {
		क्रम (i=0; is_assignment(child->argv[i]); i++) {
			debug_printf("pid %d environment modification: %s\n",getpid(),child->argv[i]);
			p = जोड़ो_var_मूल्य(child->argv[i]);
			putenv(strdup(p));
			अगर (p != child->argv[i]) मुक्त(p);
		}
		child->argv+=i;  /* XXX this hack isn't so horrible, since we are about
					to exit, and therefore don't need to keep data
					structures consistent for free() use. */
		/* If a variable is assigned in a forest, and nobody listens,
		 * was it ever really set?
		 */
		अगर (child->argv[0] == शून्य) {
			_निकास(निकास_सफल);
		}

		/*
		 * Check if the command matches any of the builtins.
		 * Depending on context, this might be redundant.  But it's
		 * easier to waste a few CPU cycles than it is to figure out
		 * if this is one of those cases.
		 */
		क्रम (x = bltins; x->cmd; x++) {
			अगर (म_भेद(child->argv[0], x->cmd) == 0 ) {
				debug_printf("builtin exec %s\n", child->argv[0]);
				rcode = x->function(child);
				ख_साफ(मानक_निकास);
				_निकास(rcode);
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
#अगर_घोषित CONFIG_FEATURE_SH_STANDALONE_SHELL
		{
			पूर्णांक argc_l;
			अक्षर** argv_l=child->argv;
			अक्षर *name = child->argv[0];

#अगर_घोषित CONFIG_FEATURE_SH_APPLETS_ALWAYS_WIN
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
#पूर्ण_अगर
			/* Count argc for use in a second... */
			क्रम(argc_l=0;*argv_l!=शून्य; argv_l++, argc_l++);
			optind = 1;
			debug_printf("running applet %s\n", name);
			run_applet_by_name(name, argc_l, child->argv);
		}
#पूर्ण_अगर
		debug_printf("exec of %s\n",child->argv[0]);
		execvp(child->argv[0],child->argv);
		perror_संदेश("couldn't exec: %s",child->argv[0]);
		_निकास(1);
	} अन्यथा अगर (child->group) {
		debug_printf("runtime nesting to group\n");
		interactive=0;    /* crucial!!!! */
		rcode = run_list_real(child->group);
		/* OK to leak memory by not calling free_pipe_list,
		 * since this process is about to exit */
		_निकास(rcode);
	} अन्यथा {
		/* Can happen.  See what bash does with ">foo" by itself. */
		debug_printf("trying to pseudo_exec null command\n");
		_निकास(निकास_सफल);
	}
}

अटल व्योम जोड़ो_bg_job(काष्ठा pipe *pi)
{
	काष्ठा pipe *thejob;

	/* Linear search for the ID of the job to use */
	pi->jobid = 1;
	क्रम (thejob = job_list; thejob; thejob = thejob->अग्र)
		अगर (thejob->jobid >= pi->jobid)
			pi->jobid = thejob->jobid + 1;

	/* add thejob to the list of running jobs */
	अगर (!job_list) {
		thejob = job_list = xmalloc(माप(*thejob));
	} अन्यथा {
		क्रम (thejob = job_list; thejob->अग्र; thejob = thejob->अग्र) /* nothing */;
		thejob->अग्र = xmalloc(माप(*thejob));
		thejob = thejob->अग्र;
	}

	/* physically copy the struct job */
	स_नकल(thejob, pi, माप(काष्ठा pipe));
	thejob->अग्र = शून्य;
	thejob->running_progs = thejob->अंक_progs;
	thejob->stopped_progs = 0;
	thejob->text = xmalloc(बफ_मान); /* cmdedit buffer size */

	/*if (pi->progs[0] && pi->progs[0].argv && pi->progs[0].argv[0]) */
	{
		अक्षर *bar=thejob->text;
		अक्षर **foo=pi->progs[0].argv;
		जबतक(foo && *foo) {
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
अटल व्योम remove_bg_job(काष्ठा pipe *pi)
{
	काष्ठा pipe *पूर्व_pipe;

	अगर (pi == job_list) {
		job_list = pi->अग्र;
	} अन्यथा {
		पूर्व_pipe = job_list;
		जबतक (पूर्व_pipe->अग्र != pi)
			पूर्व_pipe = पूर्व_pipe->अग्र;
		पूर्व_pipe->अग्र = pi->अग्र;
	}
	अगर (job_list)
		last_jobid = job_list->jobid;
	अन्यथा
		last_jobid = 0;

	pi->stopped_progs = 0;
	free_pipe(pi, 0);
	मुक्त(pi);
}

/* Checks to see if any processes have exited -- if they
   have, figure out why and see if a job has completed */
अटल पूर्णांक checkjobs(काष्ठा pipe* fg_pipe)
{
	पूर्णांक attributes;
	पूर्णांक status;
	पूर्णांक prognum = 0;
	काष्ठा pipe *pi;
	pid_t childpid;

	attributes = WUNTRACED;
	अगर (fg_pipe==शून्य) {
		attributes |= WNOHANG;
	}

	जबतक ((childpid = waitpid(-1, &status, attributes)) > 0) {
		अगर (fg_pipe) {
			पूर्णांक i, rcode = 0;
			क्रम (i=0; i < fg_pipe->अंक_progs; i++) {
				अगर (fg_pipe->progs[i].pid == childpid) {
					अगर (i==fg_pipe->अंक_progs-1)
						rcode=WEXITSTATUS(status);
					(fg_pipe->अंक_progs)--;
					वापस(rcode);
				}
			}
		}

		क्रम (pi = job_list; pi; pi = pi->अग्र) {
			prognum = 0;
			जबतक (prognum < pi->अंक_progs && pi->progs[prognum].pid != childpid) {
				prognum++;
			}
			अगर (prognum < pi->अंक_progs)
				अवरोध;
		}

		अगर(pi==शून्य) {
			debug_printf("checkjobs: pid %d was not in our list!\n", childpid);
			जारी;
		}

		अगर (WIFEXITED(status) || WIFSIGNALED(status)) {
			/* child exited */
			pi->running_progs--;
			pi->progs[prognum].pid = 0;

			अगर (!pi->running_progs) {
				म_लिखो(JOB_STATUS_FORMAT, pi->jobid, "Done", pi->text);
				remove_bg_job(pi);
			}
		} अन्यथा {
			/* child stopped */
			pi->stopped_progs++;
			pi->progs[prognum].is_stopped = 1;

#अगर 0
			/* Printing this stuff is a pain, since it tends to
			 * overwrite the prompt an inconveinient moments.  So
			 * don't do that.  */
			अगर (pi->stopped_progs == pi->अंक_progs) {
				म_लिखो("\n"JOB_STATUS_FORMAT, pi->jobid, "Stopped", pi->text);
			}
#पूर्ण_अगर
		}
	}

	अगर (childpid == -1 && त्रुटि_सं != ECHILD)
		perror_संदेश("waitpid");

	/* move the shell to the foreground */
	/*if (interactive && tcsetpgrp(shell_terminal, getpgid(0))) */
	/*	perror_msg("tcsetpgrp-2"); */
	वापस -1;
}

/* Figure out our controlling tty, checking in order stderr,
 * stdin, and stdout.  If check_pgrp is set, also check that
 * we belong to the foreground process group associated with
 * that tty.  The value of shell_terminal is needed in order to call
 * tcsetpgrp(shell_terminal, ...); */
व्योम controlling_tty(पूर्णांक check_pgrp)
{
	pid_t curpgrp;

	अगर ((curpgrp = tcgetpgrp(shell_terminal = 2)) < 0
			&& (curpgrp = tcgetpgrp(shell_terminal = 0)) < 0
			&& (curpgrp = tcgetpgrp(shell_terminal = 1)) < 0)
		जाओ shell_terminal_error;

	अगर (check_pgrp && curpgrp != getpgid(0))
		जाओ shell_terminal_error;

	वापस;

shell_terminal_error:
		shell_terminal = -1;
		वापस;
}
#पूर्ण_अगर

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
अटल पूर्णांक run_pipe_real(काष्ठा pipe *pi)
{
	पूर्णांक i;
#अगर_अघोषित __U_BOOT__
	पूर्णांक nextin, nextout;
	पूर्णांक pipefds[2];				/* pipefds[0] is for reading */
	काष्ठा child_prog *child;
	काष्ठा built_in_command *x;
	अक्षर *p;
# अगर __GNUC__
	/* Avoid longjmp clobbering */
	(व्योम) &i;
	(व्योम) &nextin;
	(व्योम) &nextout;
	(व्योम) &child;
# endif
#अन्यथा
	पूर्णांक nextin;
	पूर्णांक flag = do_repeat ? CMD_FLAG_REPEAT : 0;
	काष्ठा child_prog *child;
	अक्षर *p;
# अगर __GNUC__
	/* Avoid longjmp clobbering */
	(व्योम) &i;
	(व्योम) &nextin;
	(व्योम) &child;
# endif
#पूर्ण_अगर	/* __U_BOOT__ */

	nextin = 0;
#अगर_अघोषित __U_BOOT__
	pi->pgrp = -1;
#पूर्ण_अगर

	/* Check if this is a simple builtin (not part of a pipe).
	 * Builtins within pipes have to fork anyway, and are handled in
	 * pseudo_exec.  "echo foo | read bar" doesn't work on bash, either.
	 */
	अगर (pi->अंक_progs == 1) child = & (pi->progs[0]);
#अगर_अघोषित __U_BOOT__
	अगर (pi->अंक_progs == 1 && child->group && child->subshell == 0) {
		पूर्णांक squirrel[] = {-1, -1, -1};
		पूर्णांक rcode;
		debug_printf("non-subshell grouping\n");
		setup_redirects(child, squirrel);
		/* XXX could we merge code with following builtin case,
		 * by creating a pseudo builtin that calls run_list_real? */
		rcode = run_list_real(child->group);
		restore_redirects(squirrel);
#अन्यथा
		अगर (pi->अंक_progs == 1 && child->group) {
		पूर्णांक rcode;
		debug_printf("non-subshell grouping\n");
		rcode = run_list_real(child->group);
#पूर्ण_अगर
		वापस rcode;
	} अन्यथा अगर (pi->अंक_progs == 1 && pi->progs[0].argv != शून्य) {
		क्रम (i=0; is_assignment(child->argv[i]); i++) { /* nothing */ }
		अगर (i!=0 && child->argv[i]==शून्य) {
			/* assignments, but no command: set the local environment */
			क्रम (i=0; child->argv[i]!=शून्य; i++) {

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
				अगर (मूल्य)
					*मूल्य=0;
#अगर_अघोषित __U_BOOT__
				अगर ( get_local_var(name)) {
					export_me=1;
				}
#पूर्ण_अगर
				मुक्त(name);
				p = जोड़ो_var_मूल्य(child->argv[i]);
				set_local_var(p, export_me);
				अगर (p != child->argv[i]) मुक्त(p);
			}
			वापस निकास_सफल;   /* don't worry about errors in set_local_var() yet */
		}
		क्रम (i = 0; is_assignment(child->argv[i]); i++) {
			p = जोड़ो_var_मूल्य(child->argv[i]);
#अगर_अघोषित __U_BOOT__
			putenv(strdup(p));
#अन्यथा
			set_local_var(p, 0);
#पूर्ण_अगर
			अगर (p != child->argv[i]) {
				child->sp--;
				मुक्त(p);
			}
		}
		अगर (child->sp) {
			अक्षर * str = शून्य;

			str = make_string(child->argv + i,
					  child->argv_nonnull + i);
			parse_string_outer(str, FLAG_EXIT_FROM_LOOP | FLAG_REPARSING);
			मुक्त(str);
			वापस last_return_code;
		}
#अगर_अघोषित __U_BOOT__
		क्रम (x = bltins; x->cmd; x++) {
			अगर (म_भेद(child->argv[i], x->cmd) == 0 ) {
				पूर्णांक squirrel[] = {-1, -1, -1};
				पूर्णांक rcode;
				अगर (x->function == builtin_exec && child->argv[i+1]==शून्य) {
					debug_printf("magic exec\n");
					setup_redirects(child,शून्य);
					वापस निकास_सफल;
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
			वापस rcode;
		}
#अन्यथा
		/* check ";", because ,example , argv consist from
		 * "help;flinfo" must not execute
		 */
		अगर (म_अक्षर(child->argv[i], ';')) {
			म_लिखो("Unknown command '%s' - try 'help' or use "
					"'run' command\n", child->argv[i]);
			वापस -1;
		}
		/* Process the command */
		वापस cmd_process(flag, child->argc - i, child->argv + i,
				   &flag_repeat, शून्य);
#पूर्ण_अगर
	}
#अगर_अघोषित __U_BOOT__

	क्रम (i = 0; i < pi->अंक_progs; i++) {
		child = & (pi->progs[i]);

		/* pipes are inserted between pairs of commands */
		अगर ((i + 1) < pi->अंक_progs) {
			अगर (pipe(pipefds)<0) perror_संदेश_and_die("pipe");
			nextout = pipefds[1];
		} अन्यथा {
			nextout=1;
			pipefds[0] = -1;
		}

		/* XXX test for failed fork()? */
		अगर (!(child->pid = विभाजन())) {
			/* Set the handling for job control signals back to the default.  */
			संकेत(संक_विघ्न, संक_पूर्व);
			संकेत(SIGQUIT, संक_पूर्व);
			संकेत(संक_इति, संक_पूर्व);
			संकेत(SIGTSTP, संक_पूर्व);
			संकेत(SIGTTIN, संक_पूर्व);
			संकेत(SIGTTOU, संक_पूर्व);
			संकेत(SIGCHLD, संक_पूर्व);

			close_all();

			अगर (nextin != 0) {
				dup2(nextin, 0);
				बंद(nextin);
			}
			अगर (nextout != 1) {
				dup2(nextout, 1);
				बंद(nextout);
			}
			अगर (pipefds[0]!=-1) {
				बंद(pipefds[0]);  /* opposite end of our output pipe */
			}

			/* Like bash, explicit redirects override pipes,
			 * and the pipe fd is available for dup'ing. */
			setup_redirects(child,शून्य);

			अगर (interactive && pi->followup!=PIPE_BG) {
				/* If we (the child) win the race, put ourselves in the process
				 * group whose leader is the first process in this pipe. */
				अगर (pi->pgrp < 0) {
					pi->pgrp = getpid();
				}
				अगर (setpgid(0, pi->pgrp) == 0) {
					tcsetpgrp(2, pi->pgrp);
				}
			}

			pseudo_exec(child);
		}

		/* put our child in the process group whose leader is the
		   first process in this pipe */
		अगर (pi->pgrp < 0) {
			pi->pgrp = child->pid;
		}
		/* Don't check for errors.  The child may be dead already,
		 * in which case setpgid returns error code EACCES. */
		setpgid(child->pid, pi->pgrp);

		अगर (nextin != 0)
			बंद(nextin);
		अगर (nextout != 1)
			बंद(nextout);

		/* If there isn't another process, nextin is garbage
		   but it doesn't matter */
		nextin = pipefds[0];
	}
#पूर्ण_अगर
	वापस -1;
}

अटल पूर्णांक run_list_real(काष्ठा pipe *pi)
{
	अक्षर *save_name = शून्य;
	अक्षर **list = शून्य;
	अक्षर **save_list = शून्य;
	काष्ठा pipe *rpipe;
	पूर्णांक flag_rep = 0;
#अगर_अघोषित __U_BOOT__
	पूर्णांक save_अंक_progs;
#पूर्ण_अगर
	पूर्णांक rcode=0, flag_skip=1;
	पूर्णांक flag_restore = 0;
	पूर्णांक if_code=0, अग्र_if_code=0;  /* need double-buffer to handle elif */
	reserved_style rmode, skip_more_in_this_rmode=RES_XXXX;
	/* check syntax for "for" */
	क्रम (rpipe = pi; rpipe; rpipe = rpipe->अग्र) {
		अगर ((rpipe->r_mode == RES_IN ||
		    rpipe->r_mode == RES_FOR) &&
		    (rpipe->अग्र == शून्य)) {
				syntax();
#अगर_घोषित __U_BOOT__
				flag_repeat = 0;
#पूर्ण_अगर
				वापस 1;
		}
		अगर ((rpipe->r_mode == RES_IN &&
			(rpipe->अग्र->r_mode == RES_IN &&
			rpipe->अग्र->progs->argv != शून्य))||
			(rpipe->r_mode == RES_FOR &&
			rpipe->अग्र->r_mode != RES_IN)) {
				syntax();
#अगर_घोषित __U_BOOT__
				flag_repeat = 0;
#पूर्ण_अगर
				वापस 1;
		}
	}
	क्रम (; pi; pi = (flag_restore != 0) ? rpipe : pi->अग्र) {
		अगर (pi->r_mode == RES_WHILE || pi->r_mode == RES_UNTIL ||
			pi->r_mode == RES_FOR) {
#अगर_घोषित __U_BOOT__
				/* check Ctrl-C */
				ctrlc();
				अगर ((had_ctrlc())) {
					वापस 1;
				}
#पूर्ण_अगर
				flag_restore = 0;
				अगर (!rpipe) {
					flag_rep = 0;
					rpipe = pi;
				}
		}
		rmode = pi->r_mode;
		debug_printf("rmode=%d  if_code=%d  next_if_code=%d skip_more=%d\n", rmode, if_code, अग्र_if_code, skip_more_in_this_rmode);
		अगर (rmode == skip_more_in_this_rmode && flag_skip) {
			अगर (pi->followup == PIPE_SEQ) flag_skip=0;
			जारी;
		}
		flag_skip = 1;
		skip_more_in_this_rmode = RES_XXXX;
		अगर (rmode == RES_THEN || rmode == RES_ELSE) if_code = अग्र_if_code;
		अगर (rmode == RES_THEN &&  if_code) जारी;
		अगर (rmode == RES_ELSE && !if_code) जारी;
		अगर (rmode == RES_ELIF && !if_code) अवरोध;
		अगर (rmode == RES_FOR && pi->अंक_progs) {
			अगर (!list) {
				/* if no variable values after "in" we skip "for" */
				अगर (!pi->अग्र->progs->argv) जारी;
				/* create list of variable values */
				list = make_list_in(pi->अग्र->progs->argv,
					pi->progs->argv[0]);
				save_list = list;
				save_name = pi->progs->argv[0];
				pi->progs->argv[0] = शून्य;
				flag_rep = 1;
			}
			अगर (!(*list)) {
				मुक्त(pi->progs->argv[0]);
				मुक्त(save_list);
				list = शून्य;
				flag_rep = 0;
				pi->progs->argv[0] = save_name;
#अगर_अघोषित __U_BOOT__
				pi->progs->glob_परिणाम.gl_pathv[0] =
					pi->progs->argv[0];
#पूर्ण_अगर
				जारी;
			} अन्यथा {
				/* insert new value from list for variable */
				मुक्त(pi->progs->argv[0]);
				pi->progs->argv[0] = *list++;
#अगर_अघोषित __U_BOOT__
				pi->progs->glob_परिणाम.gl_pathv[0] =
					pi->progs->argv[0];
#पूर्ण_अगर
			}
		}
		अगर (rmode == RES_IN) जारी;
		अगर (rmode == RES_DO) {
			अगर (!flag_rep) जारी;
		}
		अगर (rmode == RES_DONE) {
			अगर (flag_rep) {
				flag_restore = 1;
			} अन्यथा {
				rpipe = शून्य;
			}
		}
		अगर (pi->अंक_progs == 0) जारी;
#अगर_अघोषित __U_BOOT__
		save_अंक_progs = pi->अंक_progs; /* save number of programs */
#पूर्ण_अगर
		rcode = run_pipe_real(pi);
		debug_printf("run_pipe_real returned %d\n",rcode);
#अगर_अघोषित __U_BOOT__
		अगर (rcode!=-1) {
			/* We only ran a builtin: rcode was set by the return value
			 * of run_pipe_real(), and we don't need to wait for anything. */
		} अन्यथा अगर (pi->followup==PIPE_BG) {
			/* XXX check bash's behavior with nontrivial pipes */
			/* XXX compute jobid */
			/* XXX what does bash do with attempts to background builtins? */
			जोड़ो_bg_job(pi);
			rcode = निकास_सफल;
		} अन्यथा {
			अगर (interactive) {
				/* move the new process group into the foreground */
				अगर (tcsetpgrp(shell_terminal, pi->pgrp) && त्रुटि_सं != ENOTTY)
					perror_संदेश("tcsetpgrp-3");
				rcode = checkjobs(pi);
				/* move the shell to the foreground */
				अगर (tcsetpgrp(shell_terminal, getpgid(0)) && त्रुटि_सं != ENOTTY)
					perror_संदेश("tcsetpgrp-4");
			} अन्यथा {
				rcode = checkjobs(pi);
			}
			debug_printf("checkjobs returned %d\n",rcode);
		}
		last_return_code=rcode;
#अन्यथा
		अगर (rcode < -1) {
			last_return_code = -rcode - 2;
			वापस -2;	/* exit */
		}
		last_return_code = rcode;
#पूर्ण_अगर
#अगर_अघोषित __U_BOOT__
		pi->अंक_progs = save_अंक_progs; /* restore number of programs */
#पूर्ण_अगर
		अगर ( rmode == RES_IF || rmode == RES_ELIF )
			अग्र_if_code=rcode;  /* can be overwritten a number of times */
		अगर (rmode == RES_WHILE)
			flag_rep = !last_return_code;
		अगर (rmode == RES_UNTIL)
			flag_rep = last_return_code;
		अगर ( (rcode==निकास_सफल && pi->followup==PIPE_OR) ||
		     (rcode!=निकास_सफल && pi->followup==PIPE_AND) )
			skip_more_in_this_rmode=rmode;
#अगर_अघोषित __U_BOOT__
		checkjobs(शून्य);
#पूर्ण_अगर
	}
	वापस rcode;
}

/* broken, of course, but OK for testing */
अटल अक्षर *indenter(पूर्णांक i)
{
	अटल अक्षर blanks[]="                                    ";
	वापस &blanks[माप(blanks)-i-1];
}

/* return code is the exit status of the pipe */
अटल पूर्णांक free_pipe(काष्ठा pipe *pi, पूर्णांक indent)
{
	अक्षर **p;
	काष्ठा child_prog *child;
#अगर_अघोषित __U_BOOT__
	काष्ठा redir_struct *r, *rnext;
#पूर्ण_अगर
	पूर्णांक a, i, ret_code=0;
	अक्षर *ind = indenter(indent);

#अगर_अघोषित __U_BOOT__
	अगर (pi->stopped_progs > 0)
		वापस ret_code;
	final_printf("%s run pipe: (pid %d)\n",ind,getpid());
#पूर्ण_अगर
	क्रम (i=0; i<pi->अंक_progs; i++) {
		child = &pi->progs[i];
		final_printf("%s  command %d:\n",ind,i);
		अगर (child->argv) {
			क्रम (a=0,p=child->argv; *p; a++,p++) {
				final_printf("%s   argv[%d] = %s\n",ind,a,*p);
			}
#अगर_अघोषित __U_BOOT__
			globfree(&child->glob_परिणाम);
#अन्यथा
			क्रम (a = 0; a < child->argc; a++) {
				मुक्त(child->argv[a]);
			}
			मुक्त(child->argv);
			मुक्त(child->argv_nonnull);
			child->argc = 0;
#पूर्ण_अगर
			child->argv=शून्य;
		} अन्यथा अगर (child->group) {
#अगर_अघोषित __U_BOOT__
			final_printf("%s   begin group (subshell:%d)\n",ind, child->subshell);
#पूर्ण_अगर
			ret_code = free_pipe_list(child->group,indent+3);
			final_printf("%s   end group\n",ind);
		} अन्यथा {
			final_printf("%s   (nil)\n",ind);
		}
#अगर_अघोषित __U_BOOT__
		क्रम (r=child->redirects; r; r=rnext) {
			final_printf("%s   redirect %d%s", ind, r->fd, redir_table[r->type].descrip);
			अगर (r->dup == -1) {
				/* guard against the case >$FOO, where foo is unset or blank */
				अगर (r->word.gl_pathv) {
					final_printf(" %s\n", *r->word.gl_pathv);
					globfree(&r->word);
				}
			} अन्यथा {
				final_printf("&%d\n", r->dup);
			}
			rnext=r->अग्र;
			मुक्त(r);
		}
		child->redirects=शून्य;
#पूर्ण_अगर
	}
	मुक्त(pi->progs);   /* children are an array, they get freed all at once */
	pi->progs=शून्य;
	वापस ret_code;
}

अटल पूर्णांक free_pipe_list(काष्ठा pipe *शीर्ष_2, पूर्णांक indent)
{
	पूर्णांक rcode=0;   /* if list has no members */
	काष्ठा pipe *pi, *अग्र;
	अक्षर *ind = indenter(indent);
	क्रम (pi=शीर्ष_2; pi; pi=अग्र) {
		final_printf("%s pipe reserved mode %d\n", ind, pi->r_mode);
		rcode = free_pipe(pi, indent);
		final_printf("%s pipe followup code %d\n", ind, pi->followup);
		अग्र=pi->अग्र;
		pi->अग्र=शून्य;
		मुक्त(pi);
	}
	वापस rcode;
}

/* Select which version we will use */
अटल पूर्णांक run_list(काष्ठा pipe *pi)
{
	पूर्णांक rcode=0;
#अगर_अघोषित __U_BOOT__
	अगर (fake_mode==0) {
#पूर्ण_अगर
		rcode = run_list_real(pi);
#अगर_अघोषित __U_BOOT__
	}
#पूर्ण_अगर
	/* free_pipe_list has the side effect of clearing memory
	 * In the long run that function can be merged with run_list_real,
	 * but doing that now would hobble the debugging effort. */
	free_pipe_list(pi,0);
	वापस rcode;
}

/* The API for glob is arguably broken.  This routine pushes a non-matching
 * string into the output structure, removing non-backslashed backslashes.
 * If someone can prove me wrong, by performing this function within the
 * original glob(3) api, feel free to rewrite this routine into oblivion.
 * Return code (0 vs. GLOB_NOSPACE) matches glob(3).
 * XXX broken if the last character is '\\', check that before calling.
 */
#अगर_अघोषित __U_BOOT__
अटल पूर्णांक globhack(स्थिर अक्षर *src, पूर्णांक flags, glob_t *pglob)
{
	पूर्णांक cnt=0, pathc;
	स्थिर अक्षर *s;
	अक्षर *dest;
	क्रम (cnt=1, s=src; s && *s; s++) {
		अगर (*s == '\\') s++;
		cnt++;
	}
	dest = दो_स्मृति(cnt);
	अगर (!dest) वापस GLOB_NOSPACE;
	अगर (!(flags & GLOB_APPEND)) {
		pglob->gl_pathv=शून्य;
		pglob->gl_pathc=0;
		pglob->gl_offs=0;
		pglob->gl_offs=0;
	}
	pathc = ++pglob->gl_pathc;
	pglob->gl_pathv = पुनः_स्मृति(pglob->gl_pathv, (pathc+1)*माप(*pglob->gl_pathv));
	अगर (pglob->gl_pathv == शून्य) वापस GLOB_NOSPACE;
	pglob->gl_pathv[pathc-1]=dest;
	pglob->gl_pathv[pathc]=शून्य;
	क्रम (s=src; s && *s; s++, dest++) {
		अगर (*s == '\\') s++;
		*dest = *s;
	}
	*dest='\0';
	वापस 0;
}

/* XXX broken if the last character is '\\', check that before calling */
अटल पूर्णांक glob_needed(स्थिर अक्षर *s)
{
	क्रम (; *s; s++) {
		अगर (*s == '\\') s++;
		अगर (म_अक्षर("*[?",*s)) वापस 1;
	}
	वापस 0;
}

#अगर 0
अटल व्योम globprint(glob_t *pglob)
{
	पूर्णांक i;
	debug_printf("glob_t at %p:\n", pglob);
	debug_printf("  gl_pathc=%d  gl_pathv=%p  gl_offs=%d  gl_flags=%d\n",
		pglob->gl_pathc, pglob->gl_pathv, pglob->gl_offs, pglob->gl_flags);
	क्रम (i=0; i<pglob->gl_pathc; i++)
		debug_printf("pglob->gl_pathv[%d] = %p = %s\n", i,
			pglob->gl_pathv[i], pglob->gl_pathv[i]);
}
#पूर्ण_अगर

अटल पूर्णांक xglob(o_string *dest, पूर्णांक flags, glob_t *pglob)
{
	पूर्णांक gr;

	/* short-circuit for null word */
	/* we can code this better when the debug_printf's are gone */
	अगर (dest->लंबाई == 0) {
		अगर (dest->nonnull) {
			/* bash man page calls this an "explicit" null */
			gr = globhack(dest->तथ्य, flags, pglob);
			debug_printf("globhack returned %d\n",gr);
		} अन्यथा {
			वापस 0;
		}
	} अन्यथा अगर (glob_needed(dest->तथ्य)) {
		gr = glob(dest->तथ्य, flags, शून्य, pglob);
		debug_printf("glob returned %d\n",gr);
		अगर (gr == GLOB_NOMATCH) {
			/* quote removal, or more accurately, backslash removal */
			gr = globhack(dest->तथ्य, flags, pglob);
			debug_printf("globhack returned %d\n",gr);
		}
	} अन्यथा {
		gr = globhack(dest->तथ्य, flags, pglob);
		debug_printf("globhack returned %d\n",gr);
	}
	अगर (gr == GLOB_NOSPACE)
		error_संदेश_and_die("out of memory during glob");
	अगर (gr != 0) { /* GLOB_ABORTED ? */
		error_संदेश("glob(3) error %d",gr);
	}
	/* globprint(glob_target); */
	वापस gr;
}
#पूर्ण_अगर

#अगर_घोषित __U_BOOT__
अटल अक्षर *get_dollar_var(अक्षर ch);
#पूर्ण_अगर

/* This is used to get/check local shell variables */
अक्षर *get_local_var(स्थिर अक्षर *s)
{
	काष्ठा variables *cur;

	अगर (!s)
		वापस शून्य;

#अगर_घोषित __U_BOOT__
	अगर (*s == '$')
		वापस get_dollar_var(s[1]);
#पूर्ण_अगर

	क्रम (cur = शीर्ष_vars; cur; cur=cur->अग्र)
		अगर(म_भेद(cur->name, s)==0)
			वापस cur->मूल्य;
	वापस शून्य;
}

/* This is used to set local shell variables
   flg_export==0 if only local (not exporting) variable
   flg_export==1 if "new" exporting environ
   flg_export>1  if current startup environ (not call putenv()) */
पूर्णांक set_local_var(स्थिर अक्षर *s, पूर्णांक flg_export)
{
	अक्षर *name, *मूल्य;
	पूर्णांक परिणाम=0;
	काष्ठा variables *cur;

#अगर_घोषित __U_BOOT__
	/* might be possible! */
	अगर (!है_अक्षर(*s))
		वापस -1;
#पूर्ण_अगर

	name=strdup(s);

	/* Assume when we enter this function that we are already in
	 * NAME=VALUE format.  So the first order of business is to
	 * split 's' on the '=' into 'name' and 'value' */
	मूल्य = म_अक्षर(name, '=');
	अगर (!मूल्य) {
		मुक्त(name);
		वापस -1;
	}
	*मूल्य++ = 0;

	अगर (!*मूल्य) {
		unset_local_var(name);
		मुक्त(name);
		वापस 0;
	}

	क्रम(cur = शीर्ष_vars; cur; cur = cur->अग्र) {
		अगर(म_भेद(cur->name, name)==0)
			अवरोध;
	}

	अगर(cur) {
		अगर(म_भेद(cur->मूल्य, मूल्य)==0) {
			अगर(flg_export>0 && cur->flg_export==0)
				cur->flg_export=flg_export;
			अन्यथा
				परिणाम++;
		} अन्यथा {
			अगर(cur->flg_read_only) {
				error_संदेश("%s: readonly variable", name);
				परिणाम = -1;
			} अन्यथा {
				अगर(flg_export>0 || cur->flg_export>1)
					cur->flg_export=1;
				मुक्त(cur->मूल्य);

				cur->मूल्य = strdup(मूल्य);
			}
		}
	} अन्यथा {
		cur = दो_स्मृति(माप(काष्ठा variables));
		अगर(!cur) {
			परिणाम = -1;
		} अन्यथा {
			cur->name = strdup(name);
			अगर (cur->name == शून्य) {
				मुक्त(cur);
				परिणाम = -1;
			} अन्यथा {
				काष्ठा variables *bottom = शीर्ष_vars;
				cur->मूल्य = strdup(मूल्य);
				cur->अग्र = शून्य;
				cur->flg_export = flg_export;
				cur->flg_read_only = 0;
				जबतक(bottom->अग्र) bottom=bottom->अग्र;
				bottom->अग्र = cur;
			}
		}
	}

#अगर_अघोषित __U_BOOT__
	अगर(परिणाम==0 && cur->flg_export==1) {
		*(मूल्य-1) = '=';
		परिणाम = putenv(name);
	} अन्यथा {
#पूर्ण_अगर
		मुक्त(name);
#अगर_अघोषित __U_BOOT__
		अगर(परिणाम>0)            /* equivalent to previous set */
			परिणाम = 0;
	}
#पूर्ण_अगर
	वापस परिणाम;
}

व्योम unset_local_var(स्थिर अक्षर *name)
{
	काष्ठा variables *cur;

	अगर (name) {
		क्रम (cur = शीर्ष_vars; cur; cur=cur->अग्र) {
			अगर(म_भेद(cur->name, name)==0)
				अवरोध;
		}
		अगर (cur != शून्य) {
			काष्ठा variables *अग्र = शीर्ष_vars;
			अगर(cur->flg_read_only) {
				error_संदेश("%s: readonly variable", name);
				वापस;
			} अन्यथा {
#अगर_अघोषित __U_BOOT__
				अगर(cur->flg_export)
					unenv_set(cur->name);
#पूर्ण_अगर
				मुक्त(cur->name);
				मुक्त(cur->मूल्य);
				जबतक (अग्र->अग्र != cur)
					अग्र = अग्र->अग्र;
				अग्र->अग्र = cur->अग्र;
			}
			मुक्त(cur);
		}
	}
}

अटल पूर्णांक is_assignment(स्थिर अक्षर *s)
{
	अगर (s == शून्य)
		वापस 0;

	अगर (!है_अक्षर(*s)) वापस 0;
	++s;
	जबतक(है_अक्षर_अंक(*s) || *s=='_') ++s;
	वापस *s=='=';
}

#अगर_अघोषित __U_BOOT__
/* the src parameter allows us to peek forward to a possible &n syntax
 * for file descriptor duplication, e.g., "2>&1".
 * Return code is 0 normally, 1 if a syntax error is detected in src.
 * Resource errors (in xmalloc) cause the process to exit */
अटल पूर्णांक setup_redirect(काष्ठा p_context *ctx, पूर्णांक fd, redir_type style,
	काष्ठा in_str *निवेश)
{
	काष्ठा child_prog *child=ctx->child;
	काष्ठा redir_struct *redir = child->redirects;
	काष्ठा redir_struct *last_redir=शून्य;

	/* Create a new redir_struct and drop it onto the end of the linked list */
	जबतक(redir) {
		last_redir=redir;
		redir=redir->अग्र;
	}
	redir = xmalloc(माप(काष्ठा redir_struct));
	redir->अग्र=शून्य;
	redir->word.gl_pathv=शून्य;
	अगर (last_redir) {
		last_redir->अग्र=redir;
	} अन्यथा {
		child->redirects=redir;
	}

	redir->type=style;
	redir->fd= (fd==-1) ? redir_table[style].default_fd : fd ;

	debug_printf("Redirect type %d%s\n", redir->fd, redir_table[style].descrip);

	/* Check for a '2>&1' type redirect */
	redir->dup = redirect_dup_अंक(निवेश);
	अगर (redir->dup == -2) वापस 1;  /* syntax error */
	अगर (redir->dup != -1) {
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
	वापस 0;
}
#पूर्ण_अगर

अटल काष्ठा pipe *new_pipe(व्योम)
{
	काष्ठा pipe *pi;
	pi = xmalloc(माप(काष्ठा pipe));
	pi->अंक_progs = 0;
	pi->progs = शून्य;
	pi->अग्र = शून्य;
	pi->followup = 0;  /* invalid */
	pi->r_mode = RES_NONE;
	वापस pi;
}

अटल व्योम initialize_context(काष्ठा p_context *ctx)
{
	ctx->pipe=शून्य;
#अगर_अघोषित __U_BOOT__
	ctx->pending_redirect=शून्य;
#पूर्ण_अगर
	ctx->child=शून्य;
	ctx->list_शीर्ष=new_pipe();
	ctx->pipe=ctx->list_शीर्ष;
	ctx->w=RES_NONE;
	ctx->ढेर=शून्य;
#अगर_घोषित __U_BOOT__
	ctx->old_flag=0;
#पूर्ण_अगर
	done_command(ctx);   /* creates the memory for working child */
}

/* normal return is 0
 * if a reserved word is found, and processed, return 1
 * should handle if, then, elif, else, fi, for, while, until, do, done.
 * case, function, and select are obnoxious, save those for later.
 */
काष्ठा reserved_combo {
	अक्षर *literal;
	पूर्णांक code;
	दीर्घ flag;
};
/* Mostly a list of accepted follow-up reserved words.
 * FLAG_END means we are done with the sequence, and are ready
 * to turn the compound list into a command.
 * FLAG_START means the word must start a new compound list.
 */
अटल काष्ठा reserved_combo reserved_list[] = {
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
#घोषणा NRES (माप(reserved_list)/माप(काष्ठा reserved_combo))

अटल पूर्णांक reserved_word(o_string *dest, काष्ठा p_context *ctx)
{
	काष्ठा reserved_combo *r;
	क्रम (r=reserved_list;
		r<reserved_list+NRES; r++) {
		अगर (म_भेद(dest->तथ्य, r->literal) == 0) {
			debug_printf("found reserved word %s, code %d\n",r->literal,r->code);
			अगर (r->flag & FLAG_START) {
				काष्ठा p_context *new = xmalloc(माप(काष्ठा p_context));
				debug_printf("push stack\n");
				अगर (ctx->w == RES_IN || ctx->w == RES_FOR) {
					syntax();
					मुक्त(new);
					ctx->w = RES_SNTX;
					b_reset(dest);
					वापस 1;
				}
				*new = *ctx;   /* physical copy */
				initialize_context(ctx);
				ctx->ढेर=new;
			} अन्यथा अगर ( ctx->w == RES_NONE || ! (ctx->old_flag & (1<<r->code))) {
				syntax();
				ctx->w = RES_SNTX;
				b_reset(dest);
				वापस 1;
			}
			ctx->w=r->code;
			ctx->old_flag = r->flag;
			अगर (ctx->old_flag & FLAG_END) {
				काष्ठा p_context *old;
				debug_printf("pop stack\n");
				done_pipe(ctx,PIPE_SEQ);
				old = ctx->ढेर;
				old->child->group = ctx->list_शीर्ष;
#अगर_अघोषित __U_BOOT__
				old->child->subshell = 0;
#पूर्ण_अगर
				*ctx = *old;   /* physical copy */
				मुक्त(old);
			}
			b_reset (dest);
			वापस 1;
		}
	}
	वापस 0;
}

/* normal return is 0.
 * Syntax or xglob errors return 1. */
अटल पूर्णांक done_word(o_string *dest, काष्ठा p_context *ctx)
{
	काष्ठा child_prog *child=ctx->child;
#अगर_अघोषित __U_BOOT__
	glob_t *glob_target;
	पूर्णांक gr, flags = 0;
#अन्यथा
	अक्षर *str, *s;
	पूर्णांक argc, cnt;
#पूर्ण_अगर

	debug_printf("done_word: %s %p\n", dest->तथ्य, child);
	अगर (dest->लंबाई == 0 && !dest->nonnull) {
		debug_printf("  true null, ignored\n");
		वापस 0;
	}
#अगर_अघोषित __U_BOOT__
	अगर (ctx->pending_redirect) {
		glob_target = &ctx->pending_redirect->word;
	} अन्यथा {
#पूर्ण_अगर
		अगर (child->group) {
			syntax();
			वापस 1;  /* syntax error, groups and arglists don't mix */
		}
		अगर (!child->argv && (ctx->type & FLAG_PARSE_SEMICOLON)) {
			debug_printf("checking %s for reserved-ness\n",dest->तथ्य);
			अगर (reserved_word(dest,ctx)) वापस ctx->w==RES_SNTX;
		}
#अगर_अघोषित __U_BOOT__
		glob_target = &child->glob_परिणाम;
		अगर (child->argv) flags |= GLOB_APPEND;
#अन्यथा
		क्रम (cnt = 1, s = dest->तथ्य; s && *s; s++) {
			अगर (*s == '\\') s++;
			cnt++;
		}
		str = दो_स्मृति(cnt);
		अगर (!str) वापस 1;
		अगर ( child->argv == शून्य) {
			child->argc=0;
		}
		argc = ++child->argc;
		child->argv = पुनः_स्मृति(child->argv, (argc+1)*माप(*child->argv));
		अगर (child->argv == शून्य) {
			मुक्त(str);
			वापस 1;
		}
		child->argv_nonnull = पुनः_स्मृति(child->argv_nonnull,
					(argc+1)*माप(*child->argv_nonnull));
		अगर (child->argv_nonnull == शून्य) {
			मुक्त(str);
			वापस 1;
		}
		child->argv[argc-1]=str;
		child->argv_nonnull[argc-1] = dest->nonnull;
		child->argv[argc]=शून्य;
		child->argv_nonnull[argc] = 0;
		क्रम (s = dest->तथ्य; s && *s; s++,str++) {
			अगर (*s == '\\') s++;
			*str = *s;
		}
		*str = '\0';
#पूर्ण_अगर
#अगर_अघोषित __U_BOOT__
	}
	gr = xglob(dest, flags, glob_target);
	अगर (gr != 0) वापस 1;
#पूर्ण_अगर

	b_reset(dest);
#अगर_अघोषित __U_BOOT__
	अगर (ctx->pending_redirect) {
		ctx->pending_redirect=शून्य;
		अगर (glob_target->gl_pathc != 1) {
			error_संदेश("ambiguous redirect");
			वापस 1;
		}
	} अन्यथा {
		child->argv = glob_target->gl_pathv;
	}
#पूर्ण_अगर
	अगर (ctx->w == RES_FOR) {
		done_word(dest,ctx);
		done_pipe(ctx,PIPE_SEQ);
	}
	वापस 0;
}

/* The only possible error here is out of memory, in which case
 * xmalloc exits. */
अटल पूर्णांक done_command(काष्ठा p_context *ctx)
{
	/* The child is really already in the pipe structure, so
	 * advance the pipe counter and make a new, null child.
	 * Only real trickiness here is that the uncommitted
	 * child structure, to which ctx->child points, is not
	 * counted in pi->num_progs. */
	काष्ठा pipe *pi=ctx->pipe;
	काष्ठा child_prog *prog=ctx->child;

	अगर (prog && prog->group == शून्य
		 && prog->argv == शून्य
#अगर_अघोषित __U_BOOT__
		 && prog->redirects == शून्य) {
#अन्यथा
										) {
#पूर्ण_अगर
		debug_printf("done_command: skipping null command\n");
		वापस 0;
	} अन्यथा अगर (prog) {
		pi->अंक_progs++;
		debug_printf("done_command: num_progs incremented to %d\n",pi->अंक_progs);
	} अन्यथा {
		debug_printf("done_command: initializing\n");
	}
	pi->progs = xrealloc(pi->progs, माप(*pi->progs) * (pi->अंक_progs+1));

	prog = pi->progs + pi->अंक_progs;
#अगर_अघोषित __U_BOOT__
	prog->redirects = शून्य;
#पूर्ण_अगर
	prog->argv = शून्य;
	prog->argv_nonnull = शून्य;
#अगर_अघोषित __U_BOOT__
	prog->is_stopped = 0;
#पूर्ण_अगर
	prog->group = शून्य;
#अगर_अघोषित __U_BOOT__
	prog->glob_परिणाम.gl_pathv = शून्य;
	prog->family = pi;
#पूर्ण_अगर
	prog->sp = 0;
	ctx->child = prog;
	prog->type = ctx->type;

	/* but ctx->pipe and ctx->list_head remain unchanged */
	वापस 0;
}

अटल पूर्णांक done_pipe(काष्ठा p_context *ctx, pipe_style type)
{
	काष्ठा pipe *new_p;
	done_command(ctx);  /* implicit closure of previous command */
	debug_printf("done_pipe, type %d\n", type);
	ctx->pipe->followup = type;
	ctx->pipe->r_mode = ctx->w;
	new_p=new_pipe();
	ctx->pipe->अग्र = new_p;
	ctx->pipe = new_p;
	ctx->child = शून्य;
	done_command(ctx);  /* set up new pipe to accept commands */
	वापस 0;
}

#अगर_अघोषित __U_BOOT__
/* peek ahead in the in_str to find out if we have a "&n" construct,
 * as in "2>&1", that represents duplicating a file descriptor.
 * returns either -2 (syntax error), -1 (no &), or the number found.
 */
अटल पूर्णांक redirect_dup_अंक(काष्ठा in_str *निवेश)
{
	पूर्णांक ch, d=0, ok=0;
	ch = b_peek(निवेश);
	अगर (ch != '&') वापस -1;

	b_getch(निवेश);  /* get the & */
	ch=b_peek(निवेश);
	अगर (ch == '-') {
		b_getch(निवेश);
		वापस -3;  /* "-" represents "close me" */
	}
	जबतक (है_अंक(ch)) {
		d = d*10+(ch-'0');
		ok=1;
		b_getch(निवेश);
		ch = b_peek(निवेश);
	}
	अगर (ok) वापस d;

	error_संदेश("ambiguous redirect");
	वापस -2;
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
अटल पूर्णांक redirect_opt_अंक(o_string *o)
{
	पूर्णांक अंक;

	अगर (o->लंबाई==0) वापस -1;
	क्रम(अंक=0; अंक<o->लंबाई; अंक++) {
		अगर (!है_अंक(*(o->तथ्य+अंक))) {
			वापस -1;
		}
	}
	/* reuse num (and save an int) */
	अंक=म_से_प(o->तथ्य);
	b_reset(o);
	वापस अंक;
}

खाता *generate_stream_from_list(काष्ठा pipe *शीर्ष_2)
{
	खाता *pf;
#अगर 1
	पूर्णांक pid, channel[2];
	अगर (pipe(channel)<0) perror_संदेश_and_die("pipe");
	pid=विभाजन();
	अगर (pid<0) {
		perror_संदेश_and_die("fork");
	} अन्यथा अगर (pid==0) {
		बंद(channel[0]);
		अगर (channel[1] != 1) {
			dup2(channel[1],1);
			बंद(channel[1]);
		}
#अगर 0
#घोषणा SURROGATE "surrogate response"
		ग_लिखो(1,SURROGATE,माप(SURROGATE));
		_निकास(run_list(शीर्ष_2));
#अन्यथा
		_निकास(run_list_real(शीर्ष_2));   /* leaks memory */
#पूर्ण_अगर
	}
	debug_printf("forked child %d\n",pid);
	बंद(channel[1]);
	pf = fdopen(channel[0],"r");
	debug_printf("pipe on FILE *%p\n",pf);
#अन्यथा
	free_pipe_list(शीर्ष_2,0);
	pf=popen("echo surrogate response","r");
	debug_printf("started fake pipe on FILE *%p\n",pf);
#पूर्ण_अगर
	वापस pf;
}

/* this version hacked for testing purposes */
/* return code is exit status of the process that is run. */
अटल पूर्णांक process_command_subs(o_string *dest, काष्ठा p_context *ctx, काष्ठा in_str *निवेश, पूर्णांक subst_end)
{
	पूर्णांक retcode;
	o_string परिणाम=NULL_O_STRING;
	काष्ठा p_context inner;
	खाता *p;
	काष्ठा in_str pipe_str;
	initialize_context(&inner);

	/* recursion to generate command */
	retcode = parse_stream(&परिणाम, &inner, निवेश, subst_end);
	अगर (retcode != 0) वापस retcode;  /* syntax error or EOF */
	done_word(&परिणाम, &inner);
	done_pipe(&inner, PIPE_SEQ);
	b_free(&परिणाम);

	p=generate_stream_from_list(inner.list_शीर्ष);
	अगर (p==शून्य) वापस 1;
	mark_open(fileno(p));
	setup_file_in_str(&pipe_str, p);

	/* now send results of command back into original context */
	retcode = parse_stream(dest, ctx, &pipe_str, '\0');
	/* XXX In case of a syntax error, should we try to kill the child?
	 * That would be tough to do right, so just read until EOF. */
	अगर (retcode == 1) {
		जबतक (b_getch(&pipe_str)!=खातापूर्ण) { /* discard */ };
	}

	debug_printf("done reading from pipe, pclose()ing\n");
	/* This is the step that wait()s for the child.  Should be pretty
	 * safe, since we just read an EOF from its stdout.  We could try
	 * to better, by using wait(), and keeping track of background jobs
	 * at the same time.  That would be a lot of work, and contrary
	 * to the KISS philosophy of this program. */
	mark_closed(fileno(p));
	retcode=pclose(p);
	free_pipe_list(inner.list_शीर्ष,0);
	debug_printf("pclosed, retcode=%d\n",retcode);
	/* XXX this process fails to trim a single trailing newline */
	वापस retcode;
}

अटल पूर्णांक parse_group(o_string *dest, काष्ठा p_context *ctx,
	काष्ठा in_str *निवेश, पूर्णांक ch)
{
	पूर्णांक rcode, endch=0;
	काष्ठा p_context sub;
	काष्ठा child_prog *child = ctx->child;
	अगर (child->argv) {
		syntax();
		वापस 1;  /* syntax error, groups and arglists don't mix */
	}
	initialize_context(&sub);
	चयन(ch) {
		हाल '(': endch=')'; child->subshell=1; अवरोध;
		हाल '{': endch='}'; अवरोध;
		शेष: syntax();   /* really logic error */
	}
	rcode=parse_stream(dest,&sub,निवेश,endch);
	done_word(dest,&sub); /* finish off the final word in the subcontext */
	done_pipe(&sub, PIPE_SEQ);  /* and the final command there, too */
	child->group = sub.list_शीर्ष;
	वापस rcode;
	/* child remains "open", available for possible redirects */
}
#पूर्ण_अगर

/* basically useful version until someone wants to get fancier,
 * see the bash man page under "Parameter Expansion" */
अटल अक्षर *lookup_param(अक्षर *src)
{
	अक्षर *p;
	अक्षर *sep;
	अक्षर *default_val = शून्य;
	पूर्णांक assign = 0;
	पूर्णांक expand_empty = 0;

	अगर (!src)
		वापस शून्य;

	sep = म_अक्षर(src, ':');

	अगर (sep) {
		*sep = '\0';
		अगर (*(sep + 1) == '-')
			default_val = sep+2;
		अगर (*(sep + 1) == '=') {
			default_val = sep+2;
			assign = 1;
		}
		अगर (*(sep + 1) == '+') {
			default_val = sep+2;
			expand_empty = 1;
		}
	}

	p = env_get(src);
	अगर (!p)
		p = get_local_var(src);

	अगर (!p || म_माप(p) == 0) {
		p = default_val;
		अगर (assign) {
			अक्षर *var = दो_स्मृति(म_माप(src)+म_माप(default_val)+2);
			अगर (var) {
				प्र_लिखो(var, "%s=%s", src, default_val);
				set_local_var(var, 0);
			}
			मुक्त(var);
		}
	} अन्यथा अगर (expand_empty) {
		p += म_माप(p);
	}

	अगर (sep)
		*sep = ':';

	वापस p;
}

#अगर_घोषित __U_BOOT__
अटल अक्षर *get_dollar_var(अक्षर ch)
{
	अटल अक्षर buf[40];

	buf[0] = '\0';
	चयन (ch) {
		हाल '?':
			प्र_लिखो(buf, "%u", (अचिन्हित पूर्णांक)last_return_code);
			अवरोध;
		शेष:
			वापस शून्य;
	}
	वापस buf;
}
#पूर्ण_अगर

/* return code: 0 for OK, 1 for syntax error */
अटल पूर्णांक handle_dollar(o_string *dest, काष्ठा p_context *ctx, काष्ठा in_str *निवेश)
{
#अगर_अघोषित __U_BOOT__
	पूर्णांक i, advance=0;
#अन्यथा
	पूर्णांक advance=0;
#पूर्ण_अगर
#अगर_अघोषित __U_BOOT__
	अक्षर sep[]=" ";
#पूर्ण_अगर
	पूर्णांक ch = निवेश->peek(निवेश);  /* first character after the $ */
	debug_printf("handle_dollar: ch=%c\n",ch);
	अगर (है_अक्षर(ch)) {
		b_addchr(dest, SPECIAL_VAR_प्रतीक);
		ctx->child->sp++;
		जबतक(ch=b_peek(निवेश),है_अक्षर_अंक(ch) || ch=='_') {
			b_getch(निवेश);
			b_addchr(dest,ch);
		}
		b_addchr(dest, SPECIAL_VAR_प्रतीक);
#अगर_अघोषित __U_BOOT__
	} अन्यथा अगर (है_अंक(ch)) {
		i = ch-'0';  /* XXX is $0 special? */
		अगर (i<global_argc) {
			parse_string(dest, ctx, global_argv[i]); /* recursion */
		}
		advance = 1;
#पूर्ण_अगर
	} अन्यथा चयन (ch) {
#अगर_अघोषित __U_BOOT__
		हाल '$':
			b_adduint(dest,getpid());
			advance = 1;
			अवरोध;
		हाल '!':
			अगर (last_bg_pid > 0) b_adduint(dest, last_bg_pid);
			advance = 1;
			अवरोध;
#पूर्ण_अगर
		हाल '?':
#अगर_अघोषित __U_BOOT__
			b_adduint(dest,last_return_code);
#अन्यथा
			ctx->child->sp++;
			b_addchr(dest, SPECIAL_VAR_प्रतीक);
			b_addchr(dest, '$');
			b_addchr(dest, '?');
			b_addchr(dest, SPECIAL_VAR_प्रतीक);
#पूर्ण_अगर
			advance = 1;
			अवरोध;
#अगर_अघोषित __U_BOOT__
		हाल '#':
			b_adduint(dest,global_argc ? global_argc-1 : 0);
			advance = 1;
			अवरोध;
#पूर्ण_अगर
		हाल '{':
			b_addchr(dest, SPECIAL_VAR_प्रतीक);
			ctx->child->sp++;
			b_getch(निवेश);
			/* XXX maybe someone will try to escape the '}' */
			जबतक(ch=b_getch(निवेश),ch!=खातापूर्ण && ch!='}') {
				b_addchr(dest,ch);
			}
			अगर (ch != '}') {
				syntax();
				वापस 1;
			}
			b_addchr(dest, SPECIAL_VAR_प्रतीक);
			अवरोध;
#अगर_अघोषित __U_BOOT__
		हाल '(':
			b_getch(निवेश);
			process_command_subs(dest, ctx, निवेश, ')');
			अवरोध;
		हाल '*':
			sep[0]=ifs[0];
			क्रम (i=1; i<global_argc; i++) {
				parse_string(dest, ctx, global_argv[i]);
				अगर (i+1 < global_argc) parse_string(dest, ctx, sep);
			}
			अवरोध;
		हाल '@':
		हाल '-':
		हाल '_':
			/* still unhandled, but should be eventually */
			error_संदेश("unhandled syntax: $%c",ch);
			वापस 1;
			अवरोध;
#पूर्ण_अगर
		शेष:
			b_addqchr(dest,'$',dest->quote);
	}
	/* Eat the character if the flag was set.  If the compiler
	 * is smart enough, we could substitute "b_getch(input);"
	 * for all the "advance = 1;" above, and also end up with
	 * a nice size-optimized program.  Hah!  That'll be the day.
	 */
	अगर (advance) b_getch(निवेश);
	वापस 0;
}

#अगर_अघोषित __U_BOOT__
पूर्णांक parse_string(o_string *dest, काष्ठा p_context *ctx, स्थिर अक्षर *src)
{
	काष्ठा in_str foo;
	setup_string_in_str(&foo, src);
	वापस parse_stream(dest, ctx, &foo, '\0');
}
#पूर्ण_अगर

/* return code is 0 for normal exit, 1 for syntax error */
अटल पूर्णांक parse_stream(o_string *dest, काष्ठा p_context *ctx,
			काष्ठा in_str *निवेश, पूर्णांक end_trigger)
{
	अचिन्हित पूर्णांक ch, m;
#अगर_अघोषित __U_BOOT__
	पूर्णांक redir_fd;
	redir_type redir_style;
#पूर्ण_अगर
	पूर्णांक अग्र;

	/* Only double-quote state is handled in the state variable dest->quote.
	 * A single-quote triggers a bypass of the main loop until its mate is
	 * found.  When recursing, quote state is passed in via dest->quote. */

	debug_printf("parse_stream, end_trigger=%d\n",end_trigger);
	जबतक ((ch=b_getch(निवेश))!=खातापूर्ण) {
		m = map[ch];
#अगर_घोषित __U_BOOT__
		अगर (निवेश->__promptme == 0) वापस 1;
#पूर्ण_अगर
		अग्र = (ch == '\n') ? 0 : b_peek(निवेश);

		debug_printf("parse_stream: ch=%c (%d) m=%d quote=%d - %c\n",
			ch >= ' ' ? ch : '.', ch, m,
			dest->quote, ctx->ढेर == शून्य ? '*' : '.');

		अगर (m==0 || ((m==1 || m==2) && dest->quote)) {
			b_addqchr(dest, ch, dest->quote);
		} अन्यथा {
			अगर (m==2) {  /* unquoted IFS */
				अगर (done_word(dest, ctx)) {
					वापस 1;
				}
				/* If we aren't performing a substitution, treat a newline as a
				 * command separator.  */
				अगर (end_trigger != '\0' && ch=='\n')
					done_pipe(ctx,PIPE_SEQ);
			}
			अगर (ch == end_trigger && !dest->quote && ctx->w==RES_NONE) {
				debug_printf("leaving parse_stream (triggered)\n");
				वापस 0;
			}
#अगर 0
			अगर (ch=='\n') {
				/* Yahoo!  Time to run with it! */
				done_pipe(ctx,PIPE_SEQ);
				run_list(ctx->list_शीर्ष);
				initialize_context(ctx);
			}
#पूर्ण_अगर
			अगर (m!=2) चयन (ch) {
		हाल '#':
			अगर (dest->लंबाई == 0 && !dest->quote) {
				जबतक(ch=b_peek(निवेश),ch!=खातापूर्ण && ch!='\n') { b_getch(निवेश); }
			} अन्यथा {
				b_addqchr(dest, ch, dest->quote);
			}
			अवरोध;
		हाल '\\':
			अगर (अग्र == खातापूर्ण) {
				syntax();
				वापस 1;
			}
			b_addqchr(dest, '\\', dest->quote);
			b_addqchr(dest, b_getch(निवेश), dest->quote);
			अवरोध;
		हाल '$':
			अगर (handle_dollar(dest, ctx, निवेश)!=0) वापस 1;
			अवरोध;
		हाल '\'':
			dest->nonnull = 1;
			जबतक(ch=b_getch(निवेश),ch!=खातापूर्ण && ch!='\'') {
#अगर_घोषित __U_BOOT__
				अगर(निवेश->__promptme == 0) वापस 1;
#पूर्ण_अगर
				b_addchr(dest,ch);
			}
			अगर (ch==खातापूर्ण) {
				syntax();
				वापस 1;
			}
			अवरोध;
		हाल '"':
			dest->nonnull = 1;
			dest->quote = !dest->quote;
			अवरोध;
#अगर_अघोषित __U_BOOT__
		हाल '`':
			process_command_subs(dest, ctx, निवेश, '`');
			अवरोध;
		हाल '>':
			redir_fd = redirect_opt_अंक(dest);
			done_word(dest, ctx);
			redir_style=REDIRECT_OVERWRITE;
			अगर (अग्र == '>') {
				redir_style=REDIRECT_APPEND;
				b_getch(निवेश);
			} अन्यथा अगर (अग्र == '(') {
				syntax();   /* until we support >(list) Process Substitution */
				वापस 1;
			}
			setup_redirect(ctx, redir_fd, redir_style, निवेश);
			अवरोध;
		हाल '<':
			redir_fd = redirect_opt_अंक(dest);
			done_word(dest, ctx);
			redir_style=REDIRECT_निवेश;
			अगर (अग्र == '<') {
				redir_style=REDIRECT_HEREIS;
				b_getch(निवेश);
			} अन्यथा अगर (अग्र == '>') {
				redir_style=REDIRECT_IO;
				b_getch(निवेश);
			} अन्यथा अगर (अग्र == '(') {
				syntax();   /* until we support <(list) Process Substitution */
				वापस 1;
			}
			setup_redirect(ctx, redir_fd, redir_style, निवेश);
			अवरोध;
#पूर्ण_अगर
		हाल ';':
			done_word(dest, ctx);
			done_pipe(ctx,PIPE_SEQ);
			अवरोध;
		हाल '&':
			done_word(dest, ctx);
			अगर (अग्र=='&') {
				b_getch(निवेश);
				done_pipe(ctx,PIPE_AND);
			} अन्यथा {
#अगर_अघोषित __U_BOOT__
				done_pipe(ctx,PIPE_BG);
#अन्यथा
				syntax_err();
				वापस 1;
#पूर्ण_अगर
			}
			अवरोध;
		हाल '|':
			done_word(dest, ctx);
			अगर (अग्र=='|') {
				b_getch(निवेश);
				done_pipe(ctx,PIPE_OR);
			} अन्यथा {
				/* we could pick up a file descriptor choice here
				 * with redirect_opt_num(), but bash doesn't do it.
				 * "echo foo 2| cat" yields "foo 2". */
#अगर_अघोषित __U_BOOT__
				done_command(ctx);
#अन्यथा
				syntax_err();
				वापस 1;
#पूर्ण_अगर
			}
			अवरोध;
#अगर_अघोषित __U_BOOT__
		हाल '(':
		हाल '{':
			अगर (parse_group(dest, ctx, निवेश, ch)!=0) वापस 1;
			अवरोध;
		हाल ')':
		हाल '}':
			syntax();   /* Proper use of this character caught by end_trigger */
			वापस 1;
			अवरोध;
#पूर्ण_अगर
		हाल SUBSTED_VAR_प्रतीक:
			dest->nonnull = 1;
			जबतक (ch = b_getch(निवेश), ch != खातापूर्ण &&
			    ch != SUBSTED_VAR_प्रतीक) {
				debug_printf("subst, pass=%d\n", ch);
				अगर (निवेश->__promptme == 0)
					वापस 1;
				b_addchr(dest, ch);
			}
			debug_printf("subst, term=%d\n", ch);
			अगर (ch == खातापूर्ण) {
				syntax();
				वापस 1;
			}
			अवरोध;
		शेष:
			syntax();   /* this is really an internal logic error */
			वापस 1;
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
	अगर (end_trigger != '\0') वापस -1;
	वापस 0;
}

अटल व्योम mapset(स्थिर अचिन्हित अक्षर *set, पूर्णांक code)
{
	स्थिर अचिन्हित अक्षर *s;
	क्रम (s=set; *s; s++) map[*s] = code;
}

अटल व्योम update_ifs_map(व्योम)
{
	/* char *ifs and char map[256] are both globals. */
	ifs = (uchar *)env_get("IFS");
	अगर (ifs == शून्य) ifs=(uchar *)" \t\n";
	/* Precompute a list of 'flow through' behavior so it can be treated
	 * quickly up front.  Computation is necessary because of IFS.
	 * Special case handling of IFS == " \t\n" is not implemented.
	 * The map[] array only really needs two bits each, and on most machines
	 * that would be faster because of the reduced L1 cache footprint.
	 */
	स_रखो(map,0,माप(map)); /* most characters flow through always */
#अगर_अघोषित __U_BOOT__
	mapset((uchar *)"\\$'\"`", 3);      /* never flow through */
	mapset((uchar *)"<>;&|(){}#", 1);   /* flow through if quoted */
#अन्यथा
	{
		uchar subst[2] = {SUBSTED_VAR_प्रतीक, 0};
		mapset(subst, 3);       /* never flow through */
	}
	mapset((uchar *)"\\$'\"", 3);       /* never flow through */
	mapset((uchar *)";&|#", 1);         /* flow through if quoted */
#पूर्ण_अगर
	mapset(ifs, 2);            /* also flow through if quoted */
}

/* most recursion does not come through here, the exeception is
 * from builtin_source() */
अटल पूर्णांक parse_stream_outer(काष्ठा in_str *inp, पूर्णांक flag)
{

	काष्ठा p_context ctx;
	o_string अस्थायी=NULL_O_STRING;
	पूर्णांक rcode;
#अगर_घोषित __U_BOOT__
	पूर्णांक code = 1;
#पूर्ण_अगर
	करो {
		ctx.type = flag;
		initialize_context(&ctx);
		update_ifs_map();
		अगर (!(flag & FLAG_PARSE_SEMICOLON) || (flag & FLAG_REPARSING)) mapset((uchar *)";$&|", 0);
		inp->promptmode=1;
		rcode = parse_stream(&अस्थायी, &ctx, inp,
				     flag & FLAG_CONT_ON_NEWLINE ? -1 : '\n');
#अगर_घोषित __U_BOOT__
		अगर (rcode == 1) flag_repeat = 0;
#पूर्ण_अगर
		अगर (rcode != 1 && ctx.old_flag != 0) {
			syntax();
#अगर_घोषित __U_BOOT__
			flag_repeat = 0;
#पूर्ण_अगर
		}
		अगर (rcode != 1 && ctx.old_flag == 0) {
			done_word(&अस्थायी, &ctx);
			done_pipe(&ctx,PIPE_SEQ);
#अगर_अघोषित __U_BOOT__
			run_list(ctx.list_शीर्ष);
#अन्यथा
			code = run_list(ctx.list_शीर्ष);
			अगर (code == -2) {	/* exit */
				b_free(&अस्थायी);
				code = 0;
				/* XXX hackish way to not allow exit from main loop */
				अगर (inp->peek == file_peek) {
					म_लिखो("exit not allowed from main input shell.\n");
					जारी;
				}
				/*
				 * DANGER
				 * Return code -2 is special in this context,
				 * it indicates exit from inner pipe instead
				 * of return code itself, the return code is
				 * stored in 'last_return_code' variable!
				 * DANGER
				 */
				वापस -2;
			}
			अगर (code == -1)
			    flag_repeat = 0;
#पूर्ण_अगर
		} अन्यथा {
			अगर (ctx.old_flag != 0) {
				मुक्त(ctx.ढेर);
				b_reset(&अस्थायी);
			}
#अगर_घोषित __U_BOOT__
			अगर (inp->__promptme == 0) म_लिखो("<INTERRUPT>\n");
			inp->__promptme = 1;
#पूर्ण_अगर
			अस्थायी.nonnull = 0;
			अस्थायी.quote = 0;
			inp->p = शून्य;
			free_pipe_list(ctx.list_शीर्ष,0);
		}
		b_free(&अस्थायी);
	/* loop on syntax errors, return on EOF */
	} जबतक (rcode != -1 && !(flag & FLAG_EXIT_FROM_LOOP) &&
		(inp->peek != static_peek || b_peek(inp)));
#अगर_अघोषित __U_BOOT__
	वापस 0;
#अन्यथा
	वापस (code != 0) ? 1 : 0;
#पूर्ण_अगर /* __U_BOOT__ */
}

#अगर_अघोषित __U_BOOT__
अटल पूर्णांक parse_string_outer(स्थिर अक्षर *s, पूर्णांक flag)
#अन्यथा
पूर्णांक parse_string_outer(स्थिर अक्षर *s, पूर्णांक flag)
#पूर्ण_अगर	/* __U_BOOT__ */
{
	काष्ठा in_str निवेश;
	पूर्णांक rcode;
#अगर_घोषित __U_BOOT__
	अक्षर *p = शून्य;
	अगर (!s)
		वापस 1;
	अगर (!*s)
		वापस 0;
	अगर (!(p = म_अक्षर(s, '\n')) || *++p) {
		p = xmalloc(म_माप(s) + 2);
		म_नकल(p, s);
		म_जोड़ो(p, "\n");
		setup_string_in_str(&निवेश, p);
		rcode = parse_stream_outer(&निवेश, flag);
		मुक्त(p);
		वापस rcode == -2 ? last_return_code : rcode;
	} अन्यथा {
#पूर्ण_अगर
	setup_string_in_str(&निवेश, s);
	rcode = parse_stream_outer(&निवेश, flag);
	वापस rcode == -2 ? last_return_code : rcode;
#अगर_घोषित __U_BOOT__
	}
#पूर्ण_अगर
}

#अगर_अघोषित __U_BOOT__
अटल पूर्णांक parse_file_outer(खाता *f)
#अन्यथा
पूर्णांक parse_file_outer(व्योम)
#पूर्ण_अगर
{
	पूर्णांक rcode;
	काष्ठा in_str निवेश;
#अगर_अघोषित __U_BOOT__
	setup_file_in_str(&निवेश, f);
#अन्यथा
	setup_file_in_str(&निवेश);
#पूर्ण_अगर
	rcode = parse_stream_outer(&निवेश, FLAG_PARSE_SEMICOLON);
	वापस rcode == -2 ? last_return_code : rcode;
}

#अगर_घोषित __U_BOOT__
पूर्णांक u_boot_hush_start(व्योम)
{
	अगर (शीर्ष_vars == शून्य) {
		शीर्ष_vars = दो_स्मृति(माप(काष्ठा variables));
		शीर्ष_vars->name = "HUSH_VERSION";
		शीर्ष_vars->मूल्य = "0.01";
		शीर्ष_vars->अग्र = शून्य;
		शीर्ष_vars->flg_export = 0;
		शीर्ष_vars->flg_read_only = 1;
	}
	वापस 0;
}

अटल व्योम *xmalloc(माप_प्रकार आकार)
{
	व्योम *p = शून्य;

	अगर (!(p = दो_स्मृति(आकार))) {
	    म_लिखो("ERROR : xmalloc failed\n");
	    क्रम(;;);
	}
	वापस p;
}

अटल व्योम *xrealloc(व्योम *ptr, माप_प्रकार आकार)
{
	व्योम *p = शून्य;

	अगर (!(p = पुनः_स्मृति(ptr, आकार))) {
	    म_लिखो("ERROR : xrealloc failed\n");
	    क्रम(;;);
	}
	वापस p;
}
#पूर्ण_अगर /* __U_BOOT__ */

#अगर_अघोषित __U_BOOT__
/* Make sure we have a controlling tty.  If we get started under a job
 * aware app (like bash for example), make sure we are now in charge so
 * we don't fight over who gets the foreground */
अटल व्योम setup_job_control(व्योम)
{
	अटल pid_t shell_pgrp;
	/* Loop until we are in the foreground.  */
	जबतक (tcgetpgrp (shell_terminal) != (shell_pgrp = getpgrp ()))
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

पूर्णांक hush_main(पूर्णांक argc, अक्षर * स्थिर *argv)
{
	पूर्णांक opt;
	खाता *निवेश;
	अक्षर **e = environ;

	/* XXX what should these be while sourcing /etc/profile? */
	global_argc = argc;
	global_argv = argv;

	/* (re?) initialize globals.  Sometimes hush_main() ends up calling
	 * hush_main(), therefore we cannot rely on the BSS to zero out this
	 * stuff.  Reset these to 0 every time. */
	ifs = शून्य;
	/* map[] is taken care of with call to update_ifs_map() */
	fake_mode = 0;
	interactive = 0;
	close_me_शीर्ष = शून्य;
	last_bg_pid = 0;
	job_list = शून्य;
	last_jobid = 0;

	/* Initialize some more globals to non-zero values */
	set_cwd();
#अगर_घोषित CONFIG_FEATURE_COMMAND_EDITING
	cmdedit_set_initial_prompt();
#अन्यथा
	PS1 = शून्य;
#पूर्ण_अगर
	PS2 = "> ";

	/* initialize our shell local variables with the values
	 * currently living in the environment */
	अगर (e) {
		क्रम (; *e; e++)
			set_local_var(*e, 2);   /* without call putenv() */
	}

	last_return_code=निकास_सफल;

	अगर (argv[0] && argv[0][0] == '-') {
		debug_printf("\nsourcing /etc/profile\n");
		अगर ((निवेश = ख_खोलो("/etc/profile", "r")) != शून्य) {
			mark_open(fileno(निवेश));
			parse_file_outer(निवेश);
			mark_closed(fileno(निवेश));
			ख_बंद(निवेश);
		}
	}
	निवेश=मानक_निवेश;

	जबतक ((opt = getopt(argc, argv, "c:xif")) > 0) {
		चयन (opt) {
			हाल 'c':
				{
					global_argv = argv+optind;
					global_argc = argc-optind;
					opt = parse_string_outer(optarg, FLAG_PARSE_SEMICOLON);
					जाओ final_return;
				}
				अवरोध;
			हाल 'i':
				interactive++;
				अवरोध;
			हाल 'f':
				fake_mode++;
				अवरोध;
			शेष:
#अगर_अघोषित BB_VER
				ख_लिखो(मानक_त्रुटि, "Usage: sh [FILE]...\n"
						"   or: sh -c command [args]...\n\n");
				निकास(निकास_त्रुटि);
#अन्यथा
				show_usage();
#पूर्ण_अगर
		}
	}
	/* A shell is interactive if the `-i' flag was given, or if all of
	 * the following conditions are met:
	 *	  no -c command
	 *    no arguments remaining or the -s flag given
	 *    standard input is a terminal
	 *    standard output is a terminal
	 *    Refer to Posix.2, the description of the `sh' utility. */
	अगर (argv[optind]==शून्य && निवेश==मानक_निवेश &&
			isatty(fileno(मानक_निवेश)) && isatty(fileno(मानक_निकास))) {
		interactive++;
	}

	debug_printf("\ninteractive=%d\n", interactive);
	अगर (interactive) {
		/* Looks like they want an interactive shell */
#अगर_अघोषित CONFIG_FEATURE_SH_EXTRA_QUIET
		म_लिखो( "\n\n" BB_BANNER " hush - the humble shell v0.01 (testing)\n");
		म_लिखो( "Enter 'help' for a list of built-in commands.\n\n");
#पूर्ण_अगर
		setup_job_control();
	}

	अगर (argv[optind]==शून्य) {
		opt=parse_file_outer(मानक_निवेश);
		जाओ final_return;
	}

	debug_printf("\nrunning script '%s'\n", argv[optind]);
	global_argv = argv+optind;
	global_argc = argc-optind;
	निवेश = xfopen(argv[optind], "r");
	opt = parse_file_outer(निवेश);

#अगर_घोषित CONFIG_FEATURE_CLEAN_UP
	ख_बंद(निवेश);
	अगर (cwd && cwd != unknown)
		मुक्त((अक्षर*)cwd);
	{
		काष्ठा variables *cur, *tmp;
		क्रम(cur = शीर्ष_vars; cur; cur = tmp) {
			tmp = cur->अग्र;
			अगर (!cur->flg_read_only) {
				मुक्त(cur->name);
				मुक्त(cur->मूल्य);
				मुक्त(cur);
			}
		}
	}
#पूर्ण_अगर

final_return:
	वापस(opt?opt:last_return_code);
}
#पूर्ण_अगर

अटल अक्षर *जोड़ो_var_मूल्य(अक्षर *inp)
{
	वापस जोड़ो_var_मूल्य_sub(inp, 0);
}

अटल अक्षर *जोड़ो_var_मूल्य_sub(अक्षर *inp, पूर्णांक tag_subst)
{
	पूर्णांक res_str_len = 0;
	पूर्णांक len;
	पूर्णांक done = 0;
	अक्षर *p, *p1, *res_str = शून्य;

	जबतक ((p = म_अक्षर(inp, SPECIAL_VAR_प्रतीक))) {
		/* check the beginning of the string for normal characters */
		अगर (p != inp) {
			/* copy any characters to the result string */
			len = p - inp;
			res_str = xrealloc(res_str, (res_str_len + len));
			म_नकलन((res_str + res_str_len), inp, len);
			res_str_len += len;
		}
		inp = ++p;
		/* find the ending marker */
		p = म_अक्षर(inp, SPECIAL_VAR_प्रतीक);
		*p = '\0';
		/* look up the value to substitute */
		अगर ((p1 = lookup_param(inp))) {
			अगर (tag_subst)
				len = res_str_len + म_माप(p1) + 2;
			अन्यथा
				len = res_str_len + म_माप(p1);
			res_str = xrealloc(res_str, (1 + len));
			अगर (tag_subst) {
				/*
				 * copy the variable value to the result
				 * string
				 */
				म_नकल((res_str + res_str_len + 1), p1);

				/*
				 * mark the replaced text to be accepted as
				 * is
				 */
				res_str[res_str_len] = SUBSTED_VAR_प्रतीक;
				res_str[res_str_len + 1 + म_माप(p1)] =
					SUBSTED_VAR_प्रतीक;
			} अन्यथा
				/*
				 * copy the variable value to the result
				 * string
				 */
				म_नकल((res_str + res_str_len), p1);

			res_str_len = len;
		}
		*p = SPECIAL_VAR_प्रतीक;
		inp = ++p;
		done = 1;
	}
	अगर (done) {
		res_str = xrealloc(res_str, (1 + res_str_len + म_माप(inp)));
		म_नकल((res_str + res_str_len), inp);
		जबतक ((p = म_अक्षर(res_str, '\n'))) {
			*p = ' ';
		}
	}
	वापस (res_str == शून्य) ? inp : res_str;
}

अटल अक्षर **make_list_in(अक्षर **inp, अक्षर *name)
{
	पूर्णांक len, i;
	पूर्णांक name_len = म_माप(name);
	पूर्णांक n = 0;
	अक्षर **list;
	अक्षर *p1, *p2, *p3;

	/* create list of variable values */
	list = xmalloc(माप(*list));
	क्रम (i = 0; inp[i]; i++) {
		p3 = जोड़ो_var_मूल्य(inp[i]);
		p1 = p3;
		जबतक (*p1) {
			अगर (*p1 == ' ') {
				p1++;
				जारी;
			}
			अगर ((p2 = म_अक्षर(p1, ' '))) {
				len = p2 - p1;
			} अन्यथा {
				len = म_माप(p1);
				p2 = p1 + len;
			}
			/* we use n + 2 in realloc for list,because we add
			 * new element and then we will add NULL element */
			list = xrealloc(list, माप(*list) * (n + 2));
			list[n] = xmalloc(2 + name_len + len);
			म_नकल(list[n], name);
			म_जोड़ो(list[n], "=");
			म_जोड़न(list[n], p1, len);
			list[n++][name_len + len + 1] = '\0';
			p1 = p2;
		}
		अगर (p3 != inp[i]) मुक्त(p3);
	}
	list[n] = शून्य;
	वापस list;
}

/*
 * Make new string for parser
 * inp     - array of argument strings to flatten
 * nonnull - indicates argument was quoted when originally parsed
 */
अटल अक्षर *make_string(अक्षर **inp, पूर्णांक *nonnull)
{
	अक्षर *p;
	अक्षर *str = शून्य;
	पूर्णांक n;
	पूर्णांक len = 2;
	अक्षर *noeval_str;
	पूर्णांक noeval = 0;

	noeval_str = get_local_var("HUSH_NO_EVAL");
	अगर (noeval_str != शून्य && *noeval_str != '0' && *noeval_str != '\0')
		noeval = 1;
	क्रम (n = 0; inp[n]; n++) {
		p = जोड़ो_var_मूल्य_sub(inp[n], noeval);
		अक्षर *new_str = xrealloc(str, (len + म_माप(p) + (2 * nonnull[n])));
		अगर (!new_str) {
			मुक्त(str);
			अगर (p != inp[n]) मुक्त(p);
			वापस शून्य;
		}
		str = new_str;
		अगर (n) {
			म_जोड़ो(str, " ");
		} अन्यथा {
			*str = '\0';
		}
		अगर (nonnull[n])
			म_जोड़ो(str, "'");
		म_जोड़ो(str, p);
		अगर (nonnull[n])
			म_जोड़ो(str, "'");
		len = म_माप(str) + 3;
		अगर (p != inp[n]) मुक्त(p);
	}
	len = म_माप(str);
	*(str + len) = '\n';
	*(str + len + 1) = '\0';
	वापस str;
}

#अगर_घोषित __U_BOOT__
अटल पूर्णांक do_showvar(काष्ठा cmd_tbl *cmdtp, पूर्णांक flag, पूर्णांक argc,
		      अक्षर *स्थिर argv[])
{
	पूर्णांक i, k;
	पूर्णांक rcode = 0;
	काष्ठा variables *cur;

	अगर (argc == 1) {		/* Print all env variables	*/
		क्रम (cur = शीर्ष_vars; cur; cur = cur->अग्र) {
			म_लिखो ("%s=%s\n", cur->name, cur->मूल्य);
			अगर (ctrlc ()) {
				माला_दो ("\n ** Abort\n");
				वापस 1;
			}
		}
		वापस 0;
	}
	क्रम (i = 1; i < argc; ++i) {	/* print single env variables	*/
		अक्षर *name = argv[i];

		k = -1;
		क्रम (cur = शीर्ष_vars; cur; cur = cur->अग्र) {
			अगर(म_भेद (cur->name, name) == 0) {
				k = 0;
				म_लिखो ("%s=%s\n", cur->name, cur->मूल्य);
			}
			अगर (ctrlc ()) {
				माला_दो ("\n ** Abort\n");
				वापस 1;
			}
		}
		अगर (k < 0) {
			म_लिखो ("## Error: \"%s\" not defined\n", name);
			rcode ++;
		}
	}
	वापस rcode;
}

U_BOOT_CMD(
	showvar, CONFIG_SYS_MAXARGS, 1,	do_showvar,
	"print local hushshell variables",
	"\n    - print values of all hushshell variables\n"
	"showvar name ...\n"
	"    - print value of hushshell variable 'name'"
);

#पूर्ण_अगर
/****************************************************************************/
