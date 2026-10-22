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

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ __U_BOOT__
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
#ਅਨ੍ਤਰ੍ਭਾਵ <env.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <malloc.h>         /* malloc, free, realloc*/
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/ctype.h>    /* isalpha, isdigit */
#ਅਨ੍ਤਰ੍ਭਾਵ <console.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <bootretry.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <cli.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <cli_hush.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <command.h>        /* find_cmd */
#ਅਨ੍ਤਰ੍ਭਾਵ <vsprintf.h>
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
#ਅਨ੍ਤਰ੍ਭਾਵ <ctype.h>     /* isalpha, isdigit */
#ਅਨ੍ਤਰ੍ਭਾਵ <unistd.h>    /* getpid */
#ਅਨ੍ਤਰ੍ਭਾਵ <stdlib.h>    /* getenv, atoi */
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>    /* strchr */
#ਅਨ੍ਤਰ੍ਭਾਵ <stdio.h>     /* popen etc. */
#ਅਨ੍ਤਰ੍ਭਾਵ <glob.h>      /* glob, of course */
#ਅਨ੍ਤਰ੍ਭਾਵ <stdarg.h>    /* va_list */
#ਅਨ੍ਤਰ੍ਭਾਵ <errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <fcntl.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <getopt.h>    /* should be pretty obvious */

#ਅਨ੍ਤਰ੍ਭਾਵ <sys/stat.h>  /* ulimit */
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/types.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <sys/wait.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <signal.h>

/* #include <dmalloc.h> */

#ਜੇ 1
#ਅਨ੍ਤਰ੍ਭਾਵ "busybox.h"
#ਅਨ੍ਤਰ੍ਭਾਵ "cmdedit.h"
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ applet_name "hush"
#ਅਨ੍ਤਰ੍ਭਾਵ "standalone.h"
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ hush_main ਮੁੱਖ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BB_BANNER
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SPECIAL_VAR_ਚਿੰਨ੍ਹ 03
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SUBSTED_VAR_ਚਿੰਨ੍ਹ 04
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FLAG_EXIT_FROM_LOOP 1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FLAG_PARSE_SEMICOLON (1 << 1)		/* symbol ';' is special for parser */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FLAG_REPARSING       (1 << 2)		/* >= 2nd pass */

#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EXIT_SUCCESS 0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EOF -1
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ syntax() syntax_err()
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ xstrdup strdup
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ error_ਸੁਨੇਹਾ ਲਿਖੋ
#ਨਹੀਂਤਾਂ
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਗਿਣਤੀ {
	REDIRECT_ਨਿਵੇਸ਼     = 1,
	REDIRECT_OVERWRITE = 2,
	REDIRECT_APPEND    = 3,
	REDIRECT_HEREIS    = 4,
	REDIRECT_IO        = 5
} redir_type;

/* The descrip member of this structure is only used to make debugging
 * output pretty */
ਬਣਤਰ {ਪੂਰਨ ਬਹੁਲਕ; ਪੂਰਨ default_fd; ਅੱਖਰ *descrip;} redir_ਸਾਰਣੀ[] = {
	{ 0,                         0, "()" },
	{ O_RDONLY,                  0, "<"  },
	{ O_CREAT|O_TRUNC|O_WRONLY,  1, ">"  },
	{ O_CREAT|O_APPEND|O_WRONLY, 1, ">>" },
	{ O_RDONLY,                 -1, "<<" },
	{ O_RDWR,                    1, "<>" }
};
#ਜੇ_ਅੰਤ

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਗਿਣਤੀ {
	PIPE_SEQ = 1,
	PIPE_AND = 2,
	PIPE_OR  = 3,
	PIPE_BG  = 4,
} pipe_style;

/* might eventually control execution */
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਗਿਣਤੀ {
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
	RES_ਪੂਰਾ  = 10,
	RES_XXXX  = 11,
	RES_ਵਿੱਚ    = 12,
	RES_SNTX  = 13
} reserved_style;
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FLAG_ਸਿਰਾ   (1<<RES_NONE)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FLAG_IF    (1<<RES_IF)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FLAG_THEN  (1<<RES_THEN)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FLAG_ELIF  (1<<RES_ELIF)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FLAG_ELSE  (1<<RES_ELSE)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FLAG_FI    (1<<RES_FI)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FLAG_FOR   (1<<RES_FOR)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FLAG_WHILE (1<<RES_WHILE)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FLAG_UNTIL (1<<RES_UNTIL)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FLAG_DO    (1<<RES_DO)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FLAG_ਪੂਰਾ  (1<<RES_ਪੂਰਾ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FLAG_ਵਿੱਚ    (1<<RES_ਵਿੱਚ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ FLAG_ਸ਼ੁਰੂ (1<<RES_XXXX)

/* This holds pointers to the various results of parsing */
ਬਣਤਰ p_context {
	ਬਣਤਰ ਬੱਚਾ_prog *ਬੱਚਾ;
	ਬਣਤਰ pipe *list_ਸਿਰ;
	ਬਣਤਰ pipe *pipe;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	ਬਣਤਰ redir_struct *pending_redirect;
#ਜੇ_ਅੰਤ
	reserved_style w;
	ਪੂਰਨ old_flag;				/* for figuring out valid reserved words */
	ਬਣਤਰ p_context *ਢੇਰ;
	ਪੂਰਨ type;			/* define type of parser : ";$" common or special symbol */
	/* How about quoting status? */
};

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
ਬਣਤਰ redir_struct {
	redir_type type;			/* type of redirection */
	ਪੂਰਨ fd;						/* file descriptor being redirected */
	ਪੂਰਨ dup;					/* -1, or file descriptor being duplicated */
	ਬਣਤਰ redir_struct *ਅਗਲਾ;	/* pointer to the next redirect in the list */
	glob_t ਸ਼ਬਦ;				/* *word.gl_pathv is the filename */
};
#ਜੇ_ਅੰਤ

ਬਣਤਰ ਬੱਚਾ_prog {
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	pid_t pid;					/* 0 if exited */
#ਜੇ_ਅੰਤ
	ਅੱਖਰ **argv;				/* program name and arguments */
	/* was quoted when parsed; copy of struct o_string.nonnull field */
	ਪੂਰਨ *argv_nonnull;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	ਪੂਰਨ    argc;                            /* number of program arguments */
#ਜੇ_ਅੰਤ
	ਬਣਤਰ pipe *ਸਮੂਹ;			/* if non-NULL, first in group or subshell */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	ਪੂਰਨ subshell;				/* flag, non-zero if group must be forked */
	ਬਣਤਰ redir_struct *redirects;	/* I/O redirections */
	glob_t glob_ਨਤੀਜਾ;			/* result of parameter globbing */
	ਪੂਰਨ is_stopped;				/* is the program currently running? */
	ਬਣਤਰ pipe *family;		/* pointer back to the child's parent pipe */
#ਜੇ_ਅੰਤ
	ਪੂਰਨ sp;				/* number of SPECIAL_VAR_SYMBOL */
	ਪੂਰਨ type;
};

ਬਣਤਰ pipe {
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	ਪੂਰਨ jobid;					/* job number */
#ਜੇ_ਅੰਤ
	ਪੂਰਨ ਅੰਕ_progs;				/* total number of programs in job */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	ਪੂਰਨ running_progs;			/* number of programs running */
	ਅੱਖਰ *ਪਾਠ;					/* name of job */
	ਅੱਖਰ *cmdbuf;				/* buffer various argv's point into */
	pid_t pgrp;					/* process group ID for the job */
#ਜੇ_ਅੰਤ
	ਬਣਤਰ ਬੱਚਾ_prog *progs;	/* array of commands in pipe */
	ਬਣਤਰ pipe *ਅਗਲਾ;			/* to track background commands */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	ਪੂਰਨ stopped_progs;			/* number of programs alive, but stopped */
	ਪੂਰਨ ਕੰਮ_context;			/* bitmask defining current context */
#ਜੇ_ਅੰਤ
	pipe_style followup;		/* PIPE_BG, PIPE_SEQ, PIPE_OR, PIPE_AND */
	reserved_style r_ਬਹੁਲਕ;		/* supports if, for, while, until */
};

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
ਬਣਤਰ close_me {
	ਪੂਰਨ fd;
	ਬਣਤਰ close_me *ਅਗਲਾ;
};
#ਜੇ_ਅੰਤ

ਬਣਤਰ variables {
	ਅੱਖਰ *name;
	ਅੱਖਰ *ਮੁੱਲ;
	ਪੂਰਨ flg_export;
	ਪੂਰਨ flg_read_only;
	ਬਣਤਰ variables *ਅਗਲਾ;
};

/* globals, connect us to the outside world
 * the first three support $?, $#, and $1 */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
ਅੱਖਰ **global_argv;
ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ global_argc;
#ਜੇ_ਅੰਤ
ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਆਖਰੀ_return_ਕੋਡ;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
ਬਾਹਰੀ ਅੱਖਰ **environ; /* This is in <unistd.h>, but protected with __USE_GNU */
#ਜੇ_ਅੰਤ

/* "globals" within this file */
ਸਥਿਰ uchar *ifs;
ਸਥਿਰ ਅੱਖਰ map[256];
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
ਸਥਿਰ ਪੂਰਨ fake_ਬਹੁਲਕ;
ਸਥਿਰ ਪੂਰਨ interactive;
ਸਥਿਰ ਬਣਤਰ close_me *close_me_ਸਿਰ;
ਸਥਿਰ ਅਚਲ ਅੱਖਰ *cwd;
ਸਥਿਰ ਬਣਤਰ pipe *ਕੰਮ_list;
ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਆਖਰੀ_bg_pid;
ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ਆਖਰੀ_jobid;
ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ shell_terminal;
ਸਥਿਰ ਅੱਖਰ *PS1;
ਸਥਿਰ ਅੱਖਰ *PS2;
ਬਣਤਰ variables shell_ver = { "HUSH_VERSION", "0.01", 1, 1, 0 };
ਬਣਤਰ variables *ਸਿਖਰ_vars = &shell_ver;
#ਨਹੀਂਤਾਂ
ਸਥਿਰ ਪੂਰਨ flag_repeat = 0;
ਸਥਿਰ ਪੂਰਨ do_repeat = 0;
ਸਥਿਰ ਬਣਤਰ variables *ਸਿਖਰ_vars = ਨਿਰਾ ;
#ਜੇ_ਅੰਤ /*__U_BOOT__ */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ B_CHUNK (100)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ B_NOSPAC 1

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ {
	ਅੱਖਰ *ਅੰਕੜੇ;
	ਪੂਰਨ ਲੰਮਾਈ;
	ਪੂਰਨ maxlen;
	ਪੂਰਨ quote;
	ਪੂਰਨ nonnull;
} o_string;
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NULL_O_STRING {ਨਿਰਾ,0,0,0,0}
/* used for initialization:
	o_string foo = NULL_O_STRING; */

/* I can almost use ordinary FILE *.  Is open_memstream() universally
 * available?  Where is it documented? */
ਬਣਤਰ ਵਿੱਚ_str {
	ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *p;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	ਅੱਖਰ peek_buf[2];
#ਜੇ_ਅੰਤ
	ਪੂਰਨ __promptme;
	ਪੂਰਨ promptmode;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	FILE *file;
#ਜੇ_ਅੰਤ
	ਪੂਰਨ (*get) (ਬਣਤਰ ਵਿੱਚ_str *);
	ਪੂਰਨ (*peek) (ਬਣਤਰ ਵਿੱਚ_str *);
};
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ b_getch(ਨਿਵੇਸ਼) ((ਨਿਵੇਸ਼)->get(ਨਿਵੇਸ਼))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ b_peek(ਨਿਵੇਸ਼) ((ਨਿਵੇਸ਼)->peek(ਨਿਵੇਸ਼))

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਕੰਮ_ਸਥਿਤੀ_FORMAT "[%d] %-22s %.40s\n"

ਬਣਤਰ built_ਵਿੱਚ_command {
	ਅੱਖਰ *cmd;					/* name */
	ਅੱਖਰ *descr;				/* description */
	ਪੂਰਨ (*function) (ਬਣਤਰ ਬੱਚਾ_prog *);	/* function ptr */
};
#ਜੇ_ਅੰਤ

/* define DEBUG_SHELL for debugging output (obviously ;-)) */
#ਜੇ 0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEBUG_SHELL
#ਜੇ_ਅੰਤ

/* This should be in utility.c */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ DEBUG_SHELL
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
ਸਥਿਰ ਖਾਲੀ debug_printf(ਅਚਲ ਅੱਖਰ *format, ...)
{
	va_list ਦਲੀਲਾਂ;
	va_ਸ਼ੁਰੂ(ਦਲੀਲਾਂ, format);
	vfprintf(stderr, format, ਦਲੀਲਾਂ);
	va_ਸਿਰਾ(ਦਲੀਲਾਂ);
}
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ debug_printf(fmt,ਦਲੀਲਾਂ...)	ਲਿਖੋ (fmt ,##args)
#ਜੇ_ਅੰਤ
#ਨਹੀਂਤਾਂ
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ debug_printf(ਅਚਲ ਅੱਖਰ *format, ...) { }
#ਜੇ_ਅੰਤ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ final_printf debug_printf

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
ਸਥਿਰ ਖਾਲੀ syntax_err(ਖਾਲੀ) {
	 ਲਿਖੋ("syntax error\n");
}
#ਨਹੀਂਤਾਂ
ਸਥਿਰ ਖਾਲੀ __syntax(ਅੱਖਰ *file, ਪੂਰਨ ਸਤਰ) {
	error_ਸੁਨੇਹਾ("syntax error %s:%d", file, ਸਤਰ);
}
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ syntax() __syntax(__FILE__, __LINE__)
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
ਸਥਿਰ ਖਾਲੀ *xmalloc(size_t ਮਾਪ);
ਸਥਿਰ ਖਾਲੀ *xrealloc(ਖਾਲੀ *ptr, size_t ਮਾਪ);
#ਨਹੀਂਤਾਂ
/* Index of subroutines: */
/*   function prototypes for builtins */
ਸਥਿਰ ਪੂਰਨ builtin_cd(ਬਣਤਰ ਬੱਚਾ_prog *ਬੱਚਾ);
ਸਥਿਰ ਪੂਰਨ builtin_env(ਬਣਤਰ ਬੱਚਾ_prog *ਬੱਚਾ);
ਸਥਿਰ ਪੂਰਨ builtin_eval(ਬਣਤਰ ਬੱਚਾ_prog *ਬੱਚਾ);
ਸਥਿਰ ਪੂਰਨ builtin_exec(ਬਣਤਰ ਬੱਚਾ_prog *ਬੱਚਾ);
ਸਥਿਰ ਪੂਰਨ builtin_exit(ਬਣਤਰ ਬੱਚਾ_prog *ਬੱਚਾ);
ਸਥਿਰ ਪੂਰਨ builtin_export(ਬਣਤਰ ਬੱਚਾ_prog *ਬੱਚਾ);
ਸਥਿਰ ਪੂਰਨ builtin_fg_bg(ਬਣਤਰ ਬੱਚਾ_prog *ਬੱਚਾ);
ਸਥਿਰ ਪੂਰਨ builtin_help(ਬਣਤਰ ਬੱਚਾ_prog *ਬੱਚਾ);
ਸਥਿਰ ਪੂਰਨ builtin_jobs(ਬਣਤਰ ਬੱਚਾ_prog *ਬੱਚਾ);
ਸਥਿਰ ਪੂਰਨ builtin_pwd(ਬਣਤਰ ਬੱਚਾ_prog *ਬੱਚਾ);
ਸਥਿਰ ਪੂਰਨ builtin_read(ਬਣਤਰ ਬੱਚਾ_prog *ਬੱਚਾ);
ਸਥਿਰ ਪੂਰਨ builtin_ਸੈੱਟ(ਬਣਤਰ ਬੱਚਾ_prog *ਬੱਚਾ);
ਸਥਿਰ ਪੂਰਨ builtin_ਖਿਸਕਾਓ(ਬਣਤਰ ਬੱਚਾ_prog *ਬੱਚਾ);
ਸਥਿਰ ਪੂਰਨ builtin_ਸਰੋਤ(ਬਣਤਰ ਬੱਚਾ_prog *ਬੱਚਾ);
ਸਥਿਰ ਪੂਰਨ builtin_umask(ਬਣਤਰ ਬੱਚਾ_prog *ਬੱਚਾ);
ਸਥਿਰ ਪੂਰਨ builtin_unset(ਬਣਤਰ ਬੱਚਾ_prog *ਬੱਚਾ);
ਸਥਿਰ ਪੂਰਨ builtin_not_written(ਬਣਤਰ ਬੱਚਾ_prog *ਬੱਚਾ);
#ਜੇ_ਅੰਤ
/*   o_string manipulation: */
ਸਥਿਰ ਪੂਰਨ b_check_space(o_string *o, ਪੂਰਨ len);
ਸਥਿਰ ਪੂਰਨ b_addchr(o_string *o, ਪੂਰਨ ch);
ਸਥਿਰ ਖਾਲੀ b_reset(o_string *o);
ਸਥਿਰ ਪੂਰਨ b_addqchr(o_string *o, ਪੂਰਨ ch, ਪੂਰਨ quote);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
ਸਥਿਰ ਪੂਰਨ b_adduint(o_string *o, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i);
#ਜੇ_ਅੰਤ
/*  in_str manipulations: */
ਸਥਿਰ ਪੂਰਨ static_get(ਬਣਤਰ ਵਿੱਚ_str *i);
ਸਥਿਰ ਪੂਰਨ static_peek(ਬਣਤਰ ਵਿੱਚ_str *i);
ਸਥਿਰ ਪੂਰਨ file_get(ਬਣਤਰ ਵਿੱਚ_str *i);
ਸਥਿਰ ਪੂਰਨ file_peek(ਬਣਤਰ ਵਿੱਚ_str *i);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
ਸਥਿਰ ਖਾਲੀ ਤਿਆਰੀ_file_ਵਿੱਚ_str(ਬਣਤਰ ਵਿੱਚ_str *i, FILE *f);
#ਨਹੀਂਤਾਂ
ਸਥਿਰ ਖਾਲੀ ਤਿਆਰੀ_file_ਵਿੱਚ_str(ਬਣਤਰ ਵਿੱਚ_str *i);
#ਜੇ_ਅੰਤ
ਸਥਿਰ ਖਾਲੀ ਤਿਆਰੀ_string_ਵਿੱਚ_str(ਬਣਤਰ ਵਿੱਚ_str *i, ਅਚਲ ਅੱਖਰ *s);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
/*  close_me manipulations: */
ਸਥਿਰ ਖਾਲੀ mark_open(ਪੂਰਨ fd);
ਸਥਿਰ ਖਾਲੀ mark_closed(ਪੂਰਨ fd);
ਸਥਿਰ ਖਾਲੀ close_all(ਖਾਲੀ);
#ਜੇ_ਅੰਤ
/*  "run" the final data structures: */
ਸਥਿਰ ਅੱਖਰ *indenter(ਪੂਰਨ i);
ਸਥਿਰ ਪੂਰਨ free_pipe_list(ਬਣਤਰ pipe *ਸਿਰ, ਪੂਰਨ indent);
ਸਥਿਰ ਪੂਰਨ free_pipe(ਬਣਤਰ pipe *pi, ਪੂਰਨ indent);
/*  really run the final data structures: */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
ਸਥਿਰ ਪੂਰਨ ਤਿਆਰੀ_redirects(ਬਣਤਰ ਬੱਚਾ_prog *prog, ਪੂਰਨ squirrel[]);
#ਜੇ_ਅੰਤ
ਸਥਿਰ ਪੂਰਨ ਲੜੀਵਾਰ_list_real(ਬਣਤਰ pipe *pi);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
ਸਥਿਰ ਖਾਲੀ pseudo_exec(ਬਣਤਰ ਬੱਚਾ_prog *ਬੱਚਾ) __attribute__ ((noreturn));
#ਜੇ_ਅੰਤ
ਸਥਿਰ ਪੂਰਨ ਲੜੀਵਾਰ_pipe_real(ਬਣਤਰ pipe *pi);
/*   extended glob support: */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
ਸਥਿਰ ਪੂਰਨ globhack(ਅਚਲ ਅੱਖਰ *src, ਪੂਰਨ flags, glob_t *pglob);
ਸਥਿਰ ਪੂਰਨ glob_needed(ਅਚਲ ਅੱਖਰ *s);
ਸਥਿਰ ਪੂਰਨ xglob(o_string *dest, ਪੂਰਨ flags, glob_t *pglob);
#ਜੇ_ਅੰਤ
/*   variable assignment: */
ਸਥਿਰ ਪੂਰਨ is_assignment(ਅਚਲ ਅੱਖਰ *s);
/*   data structure manipulation: */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
ਸਥਿਰ ਪੂਰਨ ਤਿਆਰੀ_redirect(ਬਣਤਰ p_context *ctx, ਪੂਰਨ fd, redir_type style, ਬਣਤਰ ਵਿੱਚ_str *ਨਿਵੇਸ਼);
#ਜੇ_ਅੰਤ
ਸਥਿਰ ਖਾਲੀ initialize_context(ਬਣਤਰ p_context *ctx);
ਸਥਿਰ ਪੂਰਨ ਪੂਰਾ_ਸ਼ਬਦ(o_string *dest, ਬਣਤਰ p_context *ctx);
ਸਥਿਰ ਪੂਰਨ ਪੂਰਾ_command(ਬਣਤਰ p_context *ctx);
ਸਥਿਰ ਪੂਰਨ ਪੂਰਾ_pipe(ਬਣਤਰ p_context *ctx, pipe_style type);
/*   primary string parsing: */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
ਸਥਿਰ ਪੂਰਨ redirect_dup_ਅੰਕ(ਬਣਤਰ ਵਿੱਚ_str *ਨਿਵੇਸ਼);
ਸਥਿਰ ਪੂਰਨ redirect_opt_ਅੰਕ(o_string *o);
ਸਥਿਰ ਪੂਰਨ process_command_subs(o_string *dest, ਬਣਤਰ p_context *ctx, ਬਣਤਰ ਵਿੱਚ_str *ਨਿਵੇਸ਼, ਪੂਰਨ subst_ਸਿਰਾ);
ਸਥਿਰ ਪੂਰਨ parse_ਸਮੂਹ(o_string *dest, ਬਣਤਰ p_context *ctx, ਬਣਤਰ ਵਿੱਚ_str *ਨਿਵੇਸ਼, ਪੂਰਨ ch);
#ਜੇ_ਅੰਤ
ਸਥਿਰ ਅੱਖਰ *lookup_param(ਅੱਖਰ *src);
ਸਥਿਰ ਅੱਖਰ *make_string(ਅੱਖਰ **inp, ਪੂਰਨ *nonnull);
ਸਥਿਰ ਪੂਰਨ handle_dollar(o_string *dest, ਬਣਤਰ p_context *ctx, ਬਣਤਰ ਵਿੱਚ_str *ਨਿਵੇਸ਼);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
ਸਥਿਰ ਪੂਰਨ parse_string(o_string *dest, ਬਣਤਰ p_context *ctx, ਅਚਲ ਅੱਖਰ *src);
#ਜੇ_ਅੰਤ
ਸਥਿਰ ਪੂਰਨ parse_ਧਾਰਾ(o_string *dest, ਬਣਤਰ p_context *ctx, ਬਣਤਰ ਵਿੱਚ_str *input0, ਪੂਰਨ ਸਿਰਾ_trigger);
/*   setup: */
ਸਥਿਰ ਪੂਰਨ parse_ਧਾਰਾ_outer(ਬਣਤਰ ਵਿੱਚ_str *inp, ਪੂਰਨ flag);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
ਸਥਿਰ ਪੂਰਨ parse_string_outer(ਅਚਲ ਅੱਖਰ *s, ਪੂਰਨ flag);
ਸਥਿਰ ਪੂਰਨ parse_file_outer(FILE *f);
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
/*   job management: */
ਸਥਿਰ ਪੂਰਨ checkjobs(ਬਣਤਰ pipe* fg_pipe);
ਸਥਿਰ ਖਾਲੀ ਪਾਓ_bg_ਕੰਮ(ਬਣਤਰ pipe *pi);
ਸਥਿਰ ਖਾਲੀ remove_bg_ਕੰਮ(ਬਣਤਰ pipe *pi);
#ਜੇ_ਅੰਤ
/*     local variable support */
ਸਥਿਰ ਅੱਖਰ **make_list_ਵਿੱਚ(ਅੱਖਰ **inp, ਅੱਖਰ *name);
ਸਥਿਰ ਅੱਖਰ *ਪਾਓ_var_ਮੁੱਲ(ਅੱਖਰ *inp);
ਸਥਿਰ ਅੱਖਰ *ਪਾਓ_var_ਮੁੱਲ_sub(ਅੱਖਰ *inp, ਪੂਰਨ ਟੈਗ_subst);

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
/* Table of built-in functions.  They can be forked or not, depending on
 * context: within pipes, they fork.  As simple commands, they do not.
 * When used in non-forking context, they can change global variables
 * in the parent shell process.  If forked, of course they can not.
 * For example, 'unset foo | whatever' will parse and run, but foo will
 * still be set at the end. */
ਸਥਿਰ ਬਣਤਰ built_ਵਿੱਚ_command bltins[] = {
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
	{"set", "Set/unset shell local variables", builtin_ਸੈੱਟ},
	{"shift", "Shift positional parameters", builtin_ਖਿਸਕਾਓ},
	{"trap", "Trap signals", builtin_not_written},
	{"ulimit","Controls resource limits", builtin_not_written},
	{"umask","Sets file creation mask", builtin_umask},
	{"unset", "Unset environment variable", builtin_unset},
	{".", "Source-in and run commands in a file", builtin_ਸਰੋਤ},
	{"help", "List shell built-in commands", builtin_help},
	{ਨਿਰਾ, ਨਿਰਾ, ਨਿਰਾ}
};

ਸਥਿਰ ਅਚਲ ਅੱਖਰ *ਸੈੱਟ_cwd(ਖਾਲੀ)
{
	ਜੇ(cwd==unknown)
		cwd = ਨਿਰਾ;     /* xgetcwd(arg) called free(arg) */
	cwd = xgetcwd((ਅੱਖਰ *)cwd);
	ਜੇ (!cwd)
		cwd = unknown;
	ਵਾਪਸ cwd;
}

/* built-in 'eval' handler */
ਸਥਿਰ ਪੂਰਨ builtin_eval(ਬਣਤਰ ਬੱਚਾ_prog *ਬੱਚਾ)
{
	ਅੱਖਰ *str = ਨਿਰਾ;
	ਪੂਰਨ rcode = EXIT_SUCCESS;

	ਜੇ (ਬੱਚਾ->argv[1]) {
		str = make_string(ਬੱਚਾ->argv + 1);
		parse_string_outer(str, FLAG_EXIT_FROM_LOOP |
					FLAG_PARSE_SEMICOLON);
		ਮੁਕਤ(str);
		rcode = ਆਖਰੀ_return_ਕੋਡ;
	}
	ਵਾਪਸ rcode;
}

/* built-in 'cd <path>' handler */
ਸਥਿਰ ਪੂਰਨ builtin_cd(ਬਣਤਰ ਬੱਚਾ_prog *ਬੱਚਾ)
{
	ਅੱਖਰ *newdir;
	ਜੇ (ਬੱਚਾ->argv[1] == ਨਿਰਾ)
		newdir = env_get("HOME");
	ਨਹੀਂਤਾਂ
		newdir = ਬੱਚਾ->argv[1];
	ਜੇ (chdir(newdir)) {
		ਲਿਖੋ("cd: %s: %s\n", newdir, strerror(errno));
		ਵਾਪਸ EXIT_FAILURE;
	}
	ਸੈੱਟ_cwd();
	ਵਾਪਸ EXIT_SUCCESS;
}

/* built-in 'env' handler */
ਸਥਿਰ ਪੂਰਨ builtin_env(ਬਣਤਰ ਬੱਚਾ_prog *dummy)
{
	ਅੱਖਰ **e = environ;
	ਜੇ (e == ਨਿਰਾ) ਵਾਪਸ EXIT_FAILURE;
	ਲਈ (; *e; e++) {
		puts(*e);
	}
	ਵਾਪਸ EXIT_SUCCESS;
}

/* built-in 'exec' handler */
ਸਥਿਰ ਪੂਰਨ builtin_exec(ਬਣਤਰ ਬੱਚਾ_prog *ਬੱਚਾ)
{
	ਜੇ (ਬੱਚਾ->argv[1] == ਨਿਰਾ)
		ਵਾਪਸ EXIT_SUCCESS;   /* Really? */
	ਬੱਚਾ->argv++;
	pseudo_exec(ਬੱਚਾ);
	/* never returns */
}

/* built-in 'exit' handler */
ਸਥਿਰ ਪੂਰਨ builtin_exit(ਬਣਤਰ ਬੱਚਾ_prog *ਬੱਚਾ)
{
	ਜੇ (ਬੱਚਾ->argv[1] == ਨਿਰਾ)
		ਬਾਹਰ_ਨਿਕਲੋ(ਆਖਰੀ_return_ਕੋਡ);
	ਬਾਹਰ_ਨਿਕਲੋ (atoi(ਬੱਚਾ->argv[1]));
}

/* built-in 'export VAR=value' handler */
ਸਥਿਰ ਪੂਰਨ builtin_export(ਬਣਤਰ ਬੱਚਾ_prog *ਬੱਚਾ)
{
	ਪੂਰਨ res = 0;
	ਅੱਖਰ *name = ਬੱਚਾ->argv[1];

	ਜੇ (name == ਨਿਰਾ) {
		ਵਾਪਸ (builtin_env(ਬੱਚਾ));
	}

	name = strdup(name);

	ਜੇ(name) {
		ਅੱਖਰ *ਮੁੱਲ = strchr(name, '=');

		ਜੇ (!ਮੁੱਲ) {
			ਅੱਖਰ *tmp;
			/* They are exporting something without an =VALUE */

			ਮੁੱਲ = get_local_var(name);
			ਜੇ (ਮੁੱਲ) {
				size_t ln = ਲੜੀ_ਲੰਬਾਈ(name);

				tmp = realloc(name, ln+ਲੜੀ_ਲੰਬਾਈ(ਮੁੱਲ)+2);
				ਜੇ(tmp==ਨਿਰਾ)
					res = -1;
				ਨਹੀਂਤਾਂ {
					sprintf(tmp+ln, "=%s", ਮੁੱਲ);
					name = tmp;
				}
			} ਨਹੀਂਤਾਂ {
				/* bash does not return an error when trying to export
				 * an undefined variable.  Do likewise. */
				res = 1;
			}
		}
	}
	ਜੇ (res<0)
		perror_ਸੁਨੇਹਾ("export");
	ਨਹੀਂਤਾਂ ਜੇ(res==0)
		res = ਸੈੱਟ_local_var(name, 1);
	ਨਹੀਂਤਾਂ
		res = 0;
	ਮੁਕਤ(name);
	ਵਾਪਸ res;
}

/* built-in 'fg' and 'bg' handler */
ਸਥਿਰ ਪੂਰਨ builtin_fg_bg(ਬਣਤਰ ਬੱਚਾ_prog *ਬੱਚਾ)
{
	ਪੂਰਨ i, jobnum;
	ਬਣਤਰ pipe *pi=ਨਿਰਾ;

	ਜੇ (!interactive)
		ਵਾਪਸ EXIT_FAILURE;
	/* If they gave us no args, assume they want the last backgrounded task */
	ਜੇ (!ਬੱਚਾ->argv[1]) {
		ਲਈ (pi = ਕੰਮ_list; pi; pi = pi->ਅਗਲਾ) {
			ਜੇ (pi->jobid == ਆਖਰੀ_jobid) {
				ਰੋਕੋ;
			}
		}
		ਜੇ (!pi) {
			error_ਸੁਨੇਹਾ("%s: no current job", ਬੱਚਾ->argv[0]);
			ਵਾਪਸ EXIT_FAILURE;
		}
	} ਨਹੀਂਤਾਂ {
		ਜੇ (ਲੜੀ_ਤੋਂ_ਪੜ੍ਹੋ(ਬੱਚਾ->argv[1], "%%%d", &jobnum) != 1) {
			error_ਸੁਨੇਹਾ("%s: bad argument '%s'", ਬੱਚਾ->argv[0], ਬੱਚਾ->argv[1]);
			ਵਾਪਸ EXIT_FAILURE;
		}
		ਲਈ (pi = ਕੰਮ_list; pi; pi = pi->ਅਗਲਾ) {
			ਜੇ (pi->jobid == jobnum) {
				ਰੋਕੋ;
			}
		}
		ਜੇ (!pi) {
			error_ਸੁਨੇਹਾ("%s: %d: no such job", ਬੱਚਾ->argv[0], jobnum);
			ਵਾਪਸ EXIT_FAILURE;
		}
	}

	ਜੇ (*ਬੱਚਾ->argv[0] == 'f') {
		/* Put the job into the foreground.  */
		tcsetpgrp(shell_terminal, pi->pgrp);
	}

	/* Restart the processes in the job */
	ਲਈ (i = 0; i < pi->ਅੰਕ_progs; i++)
		pi->progs[i].is_stopped = 0;

	ਜੇ ( (i=kill(- pi->pgrp, SIGCONT)) < 0) {
		ਜੇ (i == ESRCH) {
			remove_bg_ਕੰਮ(pi);
		} ਨਹੀਂਤਾਂ {
			perror_ਸੁਨੇਹਾ("kill (SIGCONT)");
		}
	}

	pi->stopped_progs = 0;
	ਵਾਪਸ EXIT_SUCCESS;
}

/* built-in 'help' handler */
ਸਥਿਰ ਪੂਰਨ builtin_help(ਬਣਤਰ ਬੱਚਾ_prog *dummy)
{
	ਬਣਤਰ built_ਵਿੱਚ_command *x;

	ਲਿਖੋ("\nBuilt-in commands:\n");
	ਲਿਖੋ("-------------------\n");
	ਲਈ (x = bltins; x->cmd; x++) {
		ਜੇ (x->descr==ਨਿਰਾ)
			ਜਾਰੀ;
		ਲਿਖੋ("%s\t%s\n", x->cmd, x->descr);
	}
	ਲਿਖੋ("\n\n");
	ਵਾਪਸ EXIT_SUCCESS;
}

/* built-in 'jobs' handler */
ਸਥਿਰ ਪੂਰਨ builtin_jobs(ਬਣਤਰ ਬੱਚਾ_prog *ਬੱਚਾ)
{
	ਬਣਤਰ pipe *ਕੰਮ;
	ਅੱਖਰ *ਸਥਿਤੀ_string;

	ਲਈ (ਕੰਮ = ਕੰਮ_list; ਕੰਮ; ਕੰਮ = ਕੰਮ->ਅਗਲਾ) {
		ਜੇ (ਕੰਮ->running_progs == ਕੰਮ->stopped_progs)
			ਸਥਿਤੀ_string = "Stopped";
		ਨਹੀਂਤਾਂ
			ਸਥਿਤੀ_string = "Running";

		ਲਿਖੋ(ਕੰਮ_ਸਥਿਤੀ_FORMAT, ਕੰਮ->jobid, ਸਥਿਤੀ_string, ਕੰਮ->ਪਾਠ);
	}
	ਵਾਪਸ EXIT_SUCCESS;
}

/* built-in 'pwd' handler */
ਸਥਿਰ ਪੂਰਨ builtin_pwd(ਬਣਤਰ ਬੱਚਾ_prog *dummy)
{
	puts(ਸੈੱਟ_cwd());
	ਵਾਪਸ EXIT_SUCCESS;
}

/* built-in 'read VAR' handler */
ਸਥਿਰ ਪੂਰਨ builtin_read(ਬਣਤਰ ਬੱਚਾ_prog *ਬੱਚਾ)
{
	ਪੂਰਨ res;

	ਜੇ (ਬੱਚਾ->argv[1]) {
		ਅੱਖਰ string[BUFSIZ];
		ਅੱਖਰ *var = 0;

		string[0] = 0;  /* In case stdin has only EOF */
		/* read string */
		fgets(string, ਆਕਾਰ(string), stdin);
		chomp(string);
		var = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਲੜੀ_ਲੰਬਾਈ(ਬੱਚਾ->argv[1])+ਲੜੀ_ਲੰਬਾਈ(string)+2);
		ਜੇ(var) {
			sprintf(var, "%s=%s", ਬੱਚਾ->argv[1], string);
			res = ਸੈੱਟ_local_var(var, 0);
		} ਨਹੀਂਤਾਂ
			res = -1;
		ਜੇ (res)
			fprintf(stderr, "read: %m\n");
		ਮੁਕਤ(var);      /* So not move up to avoid breaking errno */
		ਵਾਪਸ res;
	} ਨਹੀਂਤਾਂ {
		ਕਰੋ res=getchar(); ਜਦੋਂ(res!='\n' && res!=EOF);
		ਵਾਪਸ 0;
	}
}

/* built-in 'set VAR=value' handler */
ਸਥਿਰ ਪੂਰਨ builtin_ਸੈੱਟ(ਬਣਤਰ ਬੱਚਾ_prog *ਬੱਚਾ)
{
	ਅੱਖਰ *ਆਰਜ਼ੀ = ਬੱਚਾ->argv[1];
	ਬਣਤਰ variables *e;

	ਜੇ (ਆਰਜ਼ੀ == ਨਿਰਾ)
		ਲਈ(e = ਸਿਖਰ_vars; e; e=e->ਅਗਲਾ)
			ਲਿਖੋ("%s=%s\n", e->name, e->ਮੁੱਲ);
	ਨਹੀਂਤਾਂ
		ਸੈੱਟ_local_var(ਆਰਜ਼ੀ, 0);

		ਵਾਪਸ EXIT_SUCCESS;
}

/* Built-in 'shift' handler */
ਸਥਿਰ ਪੂਰਨ builtin_ਖਿਸਕਾਓ(ਬਣਤਰ ਬੱਚਾ_prog *ਬੱਚਾ)
{
	ਪੂਰਨ n=1;
	ਜੇ (ਬੱਚਾ->argv[1]) {
		n=atoi(ਬੱਚਾ->argv[1]);
	}
	ਜੇ (n>=0 && n<global_argc) {
		/* XXX This probably breaks $0 */
		global_argc -= n;
		global_argv += n;
		ਵਾਪਸ EXIT_SUCCESS;
	} ਨਹੀਂਤਾਂ {
		ਵਾਪਸ EXIT_FAILURE;
	}
}

/* Built-in '.' handler (read-in and execute commands from file) */
ਸਥਿਰ ਪੂਰਨ builtin_ਸਰੋਤ(ਬਣਤਰ ਬੱਚਾ_prog *ਬੱਚਾ)
{
	FILE *ਨਿਵੇਸ਼;
	ਪੂਰਨ ਸਥਿਤੀ;

	ਜੇ (ਬੱਚਾ->argv[1] == ਨਿਰਾ)
		ਵਾਪਸ EXIT_FAILURE;

	/* XXX search through $PATH is missing */
	ਨਿਵੇਸ਼ = fopen(ਬੱਚਾ->argv[1], "r");
	ਜੇ (!ਨਿਵੇਸ਼) {
		error_ਸੁਨੇਹਾ("Couldn't open file '%s'", ਬੱਚਾ->argv[1]);
		ਵਾਪਸ EXIT_FAILURE;
	}

	/* Now run the file */
	/* XXX argv and argc are broken; need to save old global_argv
	 * (pointer only is OK!) on this stack frame,
	 * set global_argv=child->argv+1, recurse, and restore. */
	mark_open(fileno(ਨਿਵੇਸ਼));
	ਸਥਿਤੀ = parse_file_outer(ਨਿਵੇਸ਼);
	mark_closed(fileno(ਨਿਵੇਸ਼));
	fclose(ਨਿਵੇਸ਼);
	ਵਾਪਸ (ਸਥਿਤੀ);
}

ਸਥਿਰ ਪੂਰਨ builtin_umask(ਬਣਤਰ ਬੱਚਾ_prog *ਬੱਚਾ)
{
	mode_t new_umask;
	ਅਚਲ ਅੱਖਰ *arg = ਬੱਚਾ->argv[1];
	ਅੱਖਰ *ਸਿਰਾ_2;
	ਜੇ (arg) {
		new_umask=strtoul(arg, &ਸਿਰਾ_2, 8);
		ਜੇ (*ਸਿਰਾ_2!='\0' || ਸਿਰਾ_2 == arg) {
			ਵਾਪਸ EXIT_FAILURE;
		}
	} ਨਹੀਂਤਾਂ {
		ਲਿਖੋ("%.3o\n", (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ) (new_umask=umask(0)));
	}
	umask(new_umask);
	ਵਾਪਸ EXIT_SUCCESS;
}

/* built-in 'unset VAR' handler */
ਸਥਿਰ ਪੂਰਨ builtin_unset(ਬਣਤਰ ਬੱਚਾ_prog *ਬੱਚਾ)
{
	/* bash returned already true */
	unset_local_var(ਬੱਚਾ->argv[1]);
	ਵਾਪਸ EXIT_SUCCESS;
}

ਸਥਿਰ ਪੂਰਨ builtin_not_written(ਬਣਤਰ ਬੱਚਾ_prog *ਬੱਚਾ)
{
	ਲਿਖੋ("builtin_%s not written\n",ਬੱਚਾ->argv[0]);
	ਵਾਪਸ EXIT_FAILURE;
}
#ਜੇ_ਅੰਤ

ਸਥਿਰ ਪੂਰਨ b_check_space(o_string *o, ਪੂਰਨ len)
{
	/* It would be easy to drop a more restrictive policy
	 * in here, such as setting a maximum string length */
	ਜੇ (o->ਲੰਮਾਈ + len > o->maxlen) {
		ਅੱਖਰ *old_ਅੰਕੜੇ = o->ਅੰਕੜੇ;
		/* assert (data == NULL || o->maxlen != 0); */
		o->maxlen += max(2*len, B_CHUNK);
		o->ਅੰਕੜੇ = realloc(o->ਅੰਕੜੇ, 1 + o->maxlen);
		ਜੇ (o->ਅੰਕੜੇ == ਨਿਰਾ) {
			ਮੁਕਤ(old_ਅੰਕੜੇ);
		}
	}
	ਵਾਪਸ o->ਅੰਕੜੇ == ਨਿਰਾ;
}

ਸਥਿਰ ਪੂਰਨ b_addchr(o_string *o, ਪੂਰਨ ch)
{
	debug_printf("b_addchr: %c %d %p\n", ch, o->ਲੰਮਾਈ, o);
	ਜੇ (b_check_space(o, 1)) ਵਾਪਸ B_NOSPAC;
	o->ਅੰਕੜੇ[o->ਲੰਮਾਈ] = ch;
	o->ਲੰਮਾਈ++;
	o->ਅੰਕੜੇ[o->ਲੰਮਾਈ] = '\0';
	ਵਾਪਸ 0;
}

ਸਥਿਰ ਖਾਲੀ b_reset(o_string *o)
{
	o->ਲੰਮਾਈ = 0;
	o->nonnull = 0;
	ਜੇ (o->ਅੰਕੜੇ != ਨਿਰਾ) *o->ਅੰਕੜੇ = '\0';
}

ਸਥਿਰ ਖਾਲੀ b_free(o_string *o)
{
	b_reset(o);
	ਮੁਕਤ(o->ਅੰਕੜੇ);
	o->ਅੰਕੜੇ = ਨਿਰਾ;
	o->maxlen = 0;
}

/* My analysis of quoting semantics tells me that state information
 * is associated with a destination, not a source.
 */
ਸਥਿਰ ਪੂਰਨ b_addqchr(o_string *o, ਪੂਰਨ ch, ਪੂਰਨ quote)
{
	ਜੇ (quote && strchr("*?[\\",ch)) {
		ਪੂਰਨ rc;
		rc = b_addchr(o, '\\');
		ਜੇ (rc) ਵਾਪਸ rc;
	}
	ਵਾਪਸ b_addchr(o, ch);
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
ਸਥਿਰ ਪੂਰਨ b_adduint(o_string *o, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ i)
{
	ਪੂਰਨ r;
	ਅੱਖਰ *p = simple_itoa(i);
	/* no escape checking necessary */
	ਕਰੋ r=b_addchr(o, *p++); ਜਦੋਂ (r==0 && *p);
	ਵਾਪਸ r;
}
#ਜੇ_ਅੰਤ

ਸਥਿਰ ਪੂਰਨ static_get(ਬਣਤਰ ਵਿੱਚ_str *i)
{
	ਪੂਰਨ ch = *i->p++;
	ਜੇ (ch=='\0') ਵਾਪਸ EOF;
	ਵਾਪਸ ch;
}

ਸਥਿਰ ਪੂਰਨ static_peek(ਬਣਤਰ ਵਿੱਚ_str *i)
{
	ਵਾਪਸ *i->p;
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ cmdedit_ਸੈੱਟ_initial_prompt(ਖਾਲੀ)
{
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ CONFIG_FEATURE_SH_FANCY_PROMPT
	PS1 = ਨਿਰਾ;
#ਨਹੀਂਤਾਂ
	PS1 = env_get("PS1");
	ਜੇ(PS1==0)
		PS1 = "\\w \\$ ";
#ਜੇ_ਅੰਤ
}

ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ ਤਿਆਰੀ_prompt_string(ਪੂਰਨ promptmode, ਅੱਖਰ **prompt_str)
{
	debug_printf("setup_prompt_string %d ",promptmode);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ CONFIG_FEATURE_SH_FANCY_PROMPT
	/* Set up the prompt */
	ਜੇ (promptmode == 1) {
		ਮੁਕਤ(PS1);
		PS1=xmalloc(ਲੜੀ_ਲੰਬਾਈ(cwd)+4);
		sprintf(PS1, "%s %s", cwd, ( geteuid() != 0 ) ?  "$ ":"# ");
		*prompt_str = PS1;
	} ਨਹੀਂਤਾਂ {
		*prompt_str = PS2;
	}
#ਨਹੀਂਤਾਂ
	*prompt_str = (promptmode==1)? PS1 : PS2;
#ਜੇ_ਅੰਤ
	debug_printf("result %s\n",*prompt_str);
}
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
ਸਥਿਰ ਪੂਰਨ uboot_cli_readline(ਬਣਤਰ ਵਿੱਚ_str *i)
{
	ਅੱਖਰ *prompt;
	ਅੱਖਰ __maybe_unused *ps_prompt = ਨਿਰਾ;

	ਜੇ (i->promptmode == 1)
		prompt = CONFIG_SYS_PROMPT;
	ਨਹੀਂਤਾਂ
		prompt = CONFIG_SYS_PROMPT_HUSH_PS2;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_CMDLINE_PS_SUPPORT
	ਜੇ (i->promptmode == 1)
		ps_prompt = env_get("PS1");
	ਨਹੀਂਤਾਂ
		ps_prompt = env_get("PS2");
	ਜੇ (ps_prompt)
		prompt = ps_prompt;
#ਜੇ_ਅੰਤ

	ਵਾਪਸ cli_readline(prompt);
}
#ਜੇ_ਅੰਤ

ਸਥਿਰ ਖਾਲੀ get_user_ਨਿਵੇਸ਼(ਬਣਤਰ ਵਿੱਚ_str *i)
{
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	ਅੱਖਰ *prompt_str;
	ਸਥਿਰ ਅੱਖਰ the_command[BUFSIZ];

	ਤਿਆਰੀ_prompt_string(i->promptmode, &prompt_str);
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_FEATURE_COMMAND_EDITING
	/*
	 ** enable command line editing only while a command line
	 ** is actually being read; otherwise, we'll end up bequeathing
	 ** atexit() handlers and other unwanted stuff to our
	 ** child processes (rob@sysgo.de)
	 */
	cmdedit_read_ਨਿਵੇਸ਼(prompt_str, the_command);
#ਨਹੀਂਤਾਂ
	fputs(prompt_str, stdout);
	fflush(stdout);
	the_command[0]=fgetc(i->file);
	the_command[1]='\0';
#ਜੇ_ਅੰਤ
	fflush(stdout);
	i->p = the_command;
#ਨਹੀਂਤਾਂ
	ਪੂਰਨ n;
	ਸਥਿਰ ਅੱਖਰ the_command[CONFIG_SYS_CBSIZE + 1];

	bootretry_reset_cmd_timeout();
	i->__promptme = 1;
	n = uboot_cli_readline(i);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_BOOT_RETRY_ਸਮਾਂ
	ਜੇ (n == -2) {
	  puts("\nTimeout waiting for command\n");
#  ifdef CONFIG_RESET_ਮੰਜ਼ਿਲ_RETRY
	  do_reset(ਨਿਰਾ, 0, 0, ਨਿਰਾ);
#  elif IS_ENABLED(CONFIG_RETRY_BOOTCMD)
	strcpy(console_ਬਫਰ, "run bootcmd\n");
# ਨਹੀਂਤਾਂ
#	error "This only works with CONFIG_RESET_TO_RETRY or CONFIG_BOOT_RETRY_COMMAND enabled"
#  endif
	}
#ਜੇ_ਅੰਤ
	ਜੇ (n == -1 ) {
		flag_repeat = 0;
		i->__promptme = 0;
	}
	n = ਲੜੀ_ਲੰਬਾਈ(console_ਬਫਰ);
	console_ਬਫਰ[n] = '\n';
	console_ਬਫਰ[n+1]= '\0';
	ਜੇ (had_ctrlc()) flag_repeat = 0;
	clear_ctrlc();
	do_repeat = 0;
	ਜੇ (i->promptmode == 1) {
		ਜੇ (console_ਬਫਰ[0] == '\n'&& flag_repeat == 0) {
			strcpy(the_command,console_ਬਫਰ);
		}
		ਨਹੀਂਤਾਂ {
			ਜੇ (console_ਬਫਰ[0] != '\n') {
				strcpy(the_command,console_ਬਫਰ);
				flag_repeat = 1;
			}
			ਨਹੀਂਤਾਂ {
				do_repeat = 1;
			}
		}
		i->p = the_command;
	}
	ਨਹੀਂਤਾਂ {
		ਜੇ (console_ਬਫਰ[0] != '\n') {
			ਜੇ (ਲੜੀ_ਲੰਬਾਈ(the_command) + ਲੜੀ_ਲੰਬਾਈ(console_ਬਫਰ)
			    < CONFIG_SYS_CBSIZE) {
				n = ਲੜੀ_ਲੰਬਾਈ(the_command);
				the_command[n-1] = ' ';
				strcpy(&the_command[n],console_ਬਫਰ);
			}
			ਨਹੀਂਤਾਂ {
				the_command[0] = '\n';
				the_command[1] = '\0';
				flag_repeat = 0;
			}
		}
		ਜੇ (i->__promptme == 0) {
			the_command[0] = '\n';
			the_command[1] = '\0';
		}
		i->p = console_ਬਫਰ;
	}
#ਜੇ_ਅੰਤ
}

/* This is the magic location that prints prompts
 * and gets data back from the user */
ਸਥਿਰ ਪੂਰਨ file_get(ਬਣਤਰ ਵਿੱਚ_str *i)
{
	ਪੂਰਨ ch;

	ch = 0;
	/* If there is data waiting, eat it up */
	ਜੇ (i->p && *i->p) {
		ch = *i->p++;
	} ਨਹੀਂਤਾਂ {
		/* need to double check i->file because we might be doing something
		 * more complicated by now, like sourcing or substituting. */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
		ਜੇ (i->__promptme && interactive && i->file == stdin) {
			ਜਦੋਂ(! i->p || (interactive && ਲੜੀ_ਲੰਬਾਈ(i->p)==0) ) {
#ਨਹੀਂਤਾਂ
			ਜਦੋਂ(! i->p  || ਲੜੀ_ਲੰਬਾਈ(i->p)==0 ) {
#ਜੇ_ਅੰਤ
				get_user_ਨਿਵੇਸ਼(i);
			}
			i->promptmode=2;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
			i->__promptme = 0;
#ਜੇ_ਅੰਤ
			ਜੇ (i->p && *i->p) {
				ch = *i->p++;
			}
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
		} ਨਹੀਂਤਾਂ {
			ch = fgetc(i->file);
		}

#ਜੇ_ਅੰਤ
		debug_printf("b_getch: got a %d\n", ch);
	}
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	ਜੇ (ch == '\n') i->__promptme=1;
#ਜੇ_ਅੰਤ
	ਵਾਪਸ ch;
}

/* All the callers guarantee this routine will never be
 * used right after a newline, so prompting is not needed.
 */
ਸਥਿਰ ਪੂਰਨ file_peek(ਬਣਤਰ ਵਿੱਚ_str *i)
{
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	ਜੇ (i->p && *i->p) {
#ਜੇ_ਅੰਤ
		ਵਾਪਸ *i->p;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	} ਨਹੀਂਤਾਂ {
		i->peek_buf[0] = fgetc(i->file);
		i->peek_buf[1] = '\0';
		i->p = i->peek_buf;
		debug_printf("b_peek: got a %d\n", *i->p);
		ਵਾਪਸ *i->p;
	}
#ਜੇ_ਅੰਤ
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
ਸਥਿਰ ਖਾਲੀ ਤਿਆਰੀ_file_ਵਿੱਚ_str(ਬਣਤਰ ਵਿੱਚ_str *i, FILE *f)
#ਨਹੀਂਤਾਂ
ਸਥਿਰ ਖਾਲੀ ਤਿਆਰੀ_file_ਵਿੱਚ_str(ਬਣਤਰ ਵਿੱਚ_str *i)
#ਜੇ_ਅੰਤ
{
	i->peek = file_peek;
	i->get = file_get;
	i->__promptme=1;
	i->promptmode=1;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	i->file = f;
#ਜੇ_ਅੰਤ
	i->p = ਨਿਰਾ;
}

ਸਥਿਰ ਖਾਲੀ ਤਿਆਰੀ_string_ਵਿੱਚ_str(ਬਣਤਰ ਵਿੱਚ_str *i, ਅਚਲ ਅੱਖਰ *s)
{
	i->peek = static_peek;
	i->get = static_get;
	i->__promptme=1;
	i->promptmode=1;
	i->p = s;
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
ਸਥਿਰ ਖਾਲੀ mark_open(ਪੂਰਨ fd)
{
	ਬਣਤਰ close_me *new = xmalloc(ਆਕਾਰ(ਬਣਤਰ close_me));
	new->fd = fd;
	new->ਅਗਲਾ = close_me_ਸਿਰ;
	close_me_ਸਿਰ = new;
}

ਸਥਿਰ ਖਾਲੀ mark_closed(ਪੂਰਨ fd)
{
	ਬਣਤਰ close_me *tmp;
	ਜੇ (close_me_ਸਿਰ == ਨਿਰਾ || close_me_ਸਿਰ->fd != fd)
		error_ਸੁਨੇਹਾ_and_die("corrupt close_me");
	tmp = close_me_ਸਿਰ;
	close_me_ਸਿਰ = close_me_ਸਿਰ->ਅਗਲਾ;
	ਮੁਕਤ(tmp);
}

ਸਥਿਰ ਖਾਲੀ close_all(ਖਾਲੀ)
{
	ਬਣਤਰ close_me *c;
	ਲਈ (c=close_me_ਸਿਰ; c; c=c->ਅਗਲਾ) {
		close(c->fd);
	}
	close_me_ਸਿਰ = ਨਿਰਾ;
}

/* squirrel != NULL means we squirrel away copies of stdin, stdout,
 * and stderr if they are redirected. */
ਸਥਿਰ ਪੂਰਨ ਤਿਆਰੀ_redirects(ਬਣਤਰ ਬੱਚਾ_prog *prog, ਪੂਰਨ squirrel[])
{
	ਪੂਰਨ openfd, ਬਹੁਲਕ;
	ਬਣਤਰ redir_struct *redir;

	ਲਈ (redir=prog->redirects; redir; redir=redir->ਅਗਲਾ) {
		ਜੇ (redir->dup == -1 && redir->ਸ਼ਬਦ.gl_pathv == ਨਿਰਾ) {
			/* something went wrong in the parse.  Pretend it didn't happen */
			ਜਾਰੀ;
		}
		ਜੇ (redir->dup == -1) {
			ਬਹੁਲਕ=redir_ਸਾਰਣੀ[redir->type].ਬਹੁਲਕ;
			openfd = open(redir->ਸ਼ਬਦ.gl_pathv[0], ਬਹੁਲਕ, 0666);
			ਜੇ (openfd < 0) {
			/* this could get lost if stderr has been redirected, but
			   bash and ash both lose it as well (though zsh doesn't!) */
				perror_ਸੁਨੇਹਾ("error opening %s", redir->ਸ਼ਬਦ.gl_pathv[0]);
				ਵਾਪਸ 1;
			}
		} ਨਹੀਂਤਾਂ {
			openfd = redir->dup;
		}

		ਜੇ (openfd != redir->fd) {
			ਜੇ (squirrel && redir->fd < 3) {
				squirrel[redir->fd] = dup(redir->fd);
			}
			ਜੇ (openfd == -3) {
				close(openfd);
			} ਨਹੀਂਤਾਂ {
				dup2(openfd, redir->fd);
				ਜੇ (redir->dup == -1)
					close (openfd);
			}
		}
	}
	ਵਾਪਸ 0;
}

ਸਥਿਰ ਖਾਲੀ restore_redirects(ਪੂਰਨ squirrel[])
{
	ਪੂਰਨ i, fd;
	ਲਈ (i=0; i<3; i++) {
		fd = squirrel[i];
		ਜੇ (fd != -1) {
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
ਸਥਿਰ ਖਾਲੀ pseudo_exec(ਬਣਤਰ ਬੱਚਾ_prog *ਬੱਚਾ)
{
	ਪੂਰਨ i, rcode;
	ਅੱਖਰ *p;
	ਬਣਤਰ built_ਵਿੱਚ_command *x;
	ਜੇ (ਬੱਚਾ->argv) {
		ਲਈ (i=0; is_assignment(ਬੱਚਾ->argv[i]); i++) {
			debug_printf("pid %d environment modification: %s\n",getpid(),ਬੱਚਾ->argv[i]);
			p = ਪਾਓ_var_ਮੁੱਲ(ਬੱਚਾ->argv[i]);
			putenv(strdup(p));
			ਜੇ (p != ਬੱਚਾ->argv[i]) ਮੁਕਤ(p);
		}
		ਬੱਚਾ->argv+=i;  /* XXX this hack isn't so horrible, since we are about
					to exit, and therefore don't need to keep data
					structures consistent for free() use. */
		/* If a variable is assigned in a forest, and nobody listens,
		 * was it ever really set?
		 */
		ਜੇ (ਬੱਚਾ->argv[0] == ਨਿਰਾ) {
			_exit(EXIT_SUCCESS);
		}

		/*
		 * Check if the command matches any of the builtins.
		 * Depending on context, this might be redundant.  But it's
		 * easier to waste a few CPU cycles than it is to figure out
		 * if this is one of those cases.
		 */
		ਲਈ (x = bltins; x->cmd; x++) {
			ਜੇ (strcmp(ਬੱਚਾ->argv[0], x->cmd) == 0 ) {
				debug_printf("builtin exec %s\n", ਬੱਚਾ->argv[0]);
				rcode = x->function(ਬੱਚਾ);
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
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_FEATURE_SH_STANDALONE_SHELL
		{
			ਪੂਰਨ argc_l;
			ਅੱਖਰ** argv_l=ਬੱਚਾ->argv;
			ਅੱਖਰ *name = ਬੱਚਾ->argv[0];

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_FEATURE_SH_APPLETS_ALWAYS_WIN
			/* Following discussions from November 2000 on the busybox mailing
			 * list, the default configuration, (without
			 * get_last_path_component()) lets the user force use of an
			 * external command by specifying the full (with slashes) filename.
			 * If you enable CONFIG_FEATURE_SH_APPLETS_ALWAYS_WIN then applets
			 * _aways_ override external commands, so if you want to run
			 * /bin/cat, it will use BusyBox cat even if /bin/cat exists on the
			 * filesystem and is _not_ busybox.  Some systems may want this,
			 * most do not.  */
			name = get_ਆਖਰੀ_ਰਸਤਾ_component(name);
#ਜੇ_ਅੰਤ
			/* Count argc for use in a second... */
			ਲਈ(argc_l=0;*argv_l!=ਨਿਰਾ; argv_l++, argc_l++);
			optind = 1;
			debug_printf("running applet %s\n", name);
			ਲੜੀਵਾਰ_applet_by_name(name, argc_l, ਬੱਚਾ->argv);
		}
#ਜੇ_ਅੰਤ
		debug_printf("exec of %s\n",ਬੱਚਾ->argv[0]);
		execvp(ਬੱਚਾ->argv[0],ਬੱਚਾ->argv);
		perror_ਸੁਨੇਹਾ("couldn't exec: %s",ਬੱਚਾ->argv[0]);
		_exit(1);
	} ਨਹੀਂਤਾਂ ਜੇ (ਬੱਚਾ->ਸਮੂਹ) {
		debug_printf("runtime nesting to group\n");
		interactive=0;    /* crucial!!!! */
		rcode = ਲੜੀਵਾਰ_list_real(ਬੱਚਾ->ਸਮੂਹ);
		/* OK to leak memory by not calling free_pipe_list,
		 * since this process is about to exit */
		_exit(rcode);
	} ਨਹੀਂਤਾਂ {
		/* Can happen.  See what bash does with ">foo" by itself. */
		debug_printf("trying to pseudo_exec null command\n");
		_exit(EXIT_SUCCESS);
	}
}

ਸਥਿਰ ਖਾਲੀ ਪਾਓ_bg_ਕੰਮ(ਬਣਤਰ pipe *pi)
{
	ਬਣਤਰ pipe *thejob;

	/* Linear search for the ID of the job to use */
	pi->jobid = 1;
	ਲਈ (thejob = ਕੰਮ_list; thejob; thejob = thejob->ਅਗਲਾ)
		ਜੇ (thejob->jobid >= pi->jobid)
			pi->jobid = thejob->jobid + 1;

	/* add thejob to the list of running jobs */
	ਜੇ (!ਕੰਮ_list) {
		thejob = ਕੰਮ_list = xmalloc(ਆਕਾਰ(*thejob));
	} ਨਹੀਂਤਾਂ {
		ਲਈ (thejob = ਕੰਮ_list; thejob->ਅਗਲਾ; thejob = thejob->ਅਗਲਾ) /* nothing */;
		thejob->ਅਗਲਾ = xmalloc(ਆਕਾਰ(*thejob));
		thejob = thejob->ਅਗਲਾ;
	}

	/* physically copy the struct job */
	ਸਮ੍ਰਿਤੀ_ਨਕਲ(thejob, pi, ਆਕਾਰ(ਬਣਤਰ pipe));
	thejob->ਅਗਲਾ = ਨਿਰਾ;
	thejob->running_progs = thejob->ਅੰਕ_progs;
	thejob->stopped_progs = 0;
	thejob->ਪਾਠ = xmalloc(BUFSIZ); /* cmdedit buffer size */

	/*if (pi->progs[0] && pi->progs[0].argv && pi->progs[0].argv[0]) */
	{
		ਅੱਖਰ *bar=thejob->ਪਾਠ;
		ਅੱਖਰ **foo=pi->progs[0].argv;
		ਜਦੋਂ(foo && *foo) {
			bar += sprintf(bar, "%s ", *foo++);
		}
	}

	/* we don't wait for background thejobs to return -- append it
	   to the list of backgrounded thejobs and leave it alone */
	ਲਿਖੋ("[%d] %d\n", thejob->jobid, thejob->progs[0].pid);
	ਆਖਰੀ_bg_pid = thejob->progs[0].pid;
	ਆਖਰੀ_jobid = thejob->jobid;
}

/* remove a backgrounded job */
ਸਥਿਰ ਖਾਲੀ remove_bg_ਕੰਮ(ਬਣਤਰ pipe *pi)
{
	ਬਣਤਰ pipe *prev_pipe;

	ਜੇ (pi == ਕੰਮ_list) {
		ਕੰਮ_list = pi->ਅਗਲਾ;
	} ਨਹੀਂਤਾਂ {
		prev_pipe = ਕੰਮ_list;
		ਜਦੋਂ (prev_pipe->ਅਗਲਾ != pi)
			prev_pipe = prev_pipe->ਅਗਲਾ;
		prev_pipe->ਅਗਲਾ = pi->ਅਗਲਾ;
	}
	ਜੇ (ਕੰਮ_list)
		ਆਖਰੀ_jobid = ਕੰਮ_list->jobid;
	ਨਹੀਂਤਾਂ
		ਆਖਰੀ_jobid = 0;

	pi->stopped_progs = 0;
	free_pipe(pi, 0);
	ਮੁਕਤ(pi);
}

/* Checks to see if any processes have exited -- if they
   have, figure out why and see if a job has completed */
ਸਥਿਰ ਪੂਰਨ checkjobs(ਬਣਤਰ pipe* fg_pipe)
{
	ਪੂਰਨ ਗੁਣ;
	ਪੂਰਨ ਸਥਿਤੀ;
	ਪੂਰਨ prognum = 0;
	ਬਣਤਰ pipe *pi;
	pid_t childpid;

	ਗੁਣ = WUNTRACED;
	ਜੇ (fg_pipe==ਨਿਰਾ) {
		ਗੁਣ |= WNOHANG;
	}

	ਜਦੋਂ ((childpid = waitpid(-1, &ਸਥਿਤੀ, ਗੁਣ)) > 0) {
		ਜੇ (fg_pipe) {
			ਪੂਰਨ i, rcode = 0;
			ਲਈ (i=0; i < fg_pipe->ਅੰਕ_progs; i++) {
				ਜੇ (fg_pipe->progs[i].pid == childpid) {
					ਜੇ (i==fg_pipe->ਅੰਕ_progs-1)
						rcode=WEXITSTATUS(ਸਥਿਤੀ);
					(fg_pipe->ਅੰਕ_progs)--;
					ਵਾਪਸ(rcode);
				}
			}
		}

		ਲਈ (pi = ਕੰਮ_list; pi; pi = pi->ਅਗਲਾ) {
			prognum = 0;
			ਜਦੋਂ (prognum < pi->ਅੰਕ_progs && pi->progs[prognum].pid != childpid) {
				prognum++;
			}
			ਜੇ (prognum < pi->ਅੰਕ_progs)
				ਰੋਕੋ;
		}

		ਜੇ(pi==ਨਿਰਾ) {
			debug_printf("checkjobs: pid %d was not in our list!\n", childpid);
			ਜਾਰੀ;
		}

		ਜੇ (WIFEXITED(ਸਥਿਤੀ) || WIFSIGNALED(ਸਥਿਤੀ)) {
			/* child exited */
			pi->running_progs--;
			pi->progs[prognum].pid = 0;

			ਜੇ (!pi->running_progs) {
				ਲਿਖੋ(ਕੰਮ_ਸਥਿਤੀ_FORMAT, pi->jobid, "Done", pi->ਪਾਠ);
				remove_bg_ਕੰਮ(pi);
			}
		} ਨਹੀਂਤਾਂ {
			/* child stopped */
			pi->stopped_progs++;
			pi->progs[prognum].is_stopped = 1;

#ਜੇ 0
			/* Printing this stuff is a pain, since it tends to
			 * overwrite the prompt an inconveinient moments.  So
			 * don't do that.  */
			ਜੇ (pi->stopped_progs == pi->ਅੰਕ_progs) {
				ਲਿਖੋ("\n"ਕੰਮ_ਸਥਿਤੀ_FORMAT, pi->jobid, "Stopped", pi->ਪਾਠ);
			}
#ਜੇ_ਅੰਤ
		}
	}

	ਜੇ (childpid == -1 && errno != ECHILD)
		perror_ਸੁਨੇਹਾ("waitpid");

	/* move the shell to the foreground */
	/*if (interactive && tcsetpgrp(shell_terminal, getpgid(0))) */
	/*	perror_msg("tcsetpgrp-2"); */
	ਵਾਪਸ -1;
}

/* Figure out our controlling tty, checking in order stderr,
 * stdin, and stdout.  If check_pgrp is set, also check that
 * we belong to the foreground process group associated with
 * that tty.  The value of shell_terminal is needed in order to call
 * tcsetpgrp(shell_terminal, ...); */
ਖਾਲੀ controlling_tty(ਪੂਰਨ check_pgrp)
{
	pid_t curpgrp;

	ਜੇ ((curpgrp = tcgetpgrp(shell_terminal = 2)) < 0
			&& (curpgrp = tcgetpgrp(shell_terminal = 0)) < 0
			&& (curpgrp = tcgetpgrp(shell_terminal = 1)) < 0)
		ਜਾਓ shell_terminal_error;

	ਜੇ (check_pgrp && curpgrp != getpgid(0))
		ਜਾਓ shell_terminal_error;

	ਵਾਪਸ;

shell_terminal_error:
		shell_terminal = -1;
		ਵਾਪਸ;
}
#ਜੇ_ਅੰਤ

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
ਸਥਿਰ ਪੂਰਨ ਲੜੀਵਾਰ_pipe_real(ਬਣਤਰ pipe *pi)
{
	ਪੂਰਨ i;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	ਪੂਰਨ nextin, nextout;
	ਪੂਰਨ pipefds[2];				/* pipefds[0] is for reading */
	ਬਣਤਰ ਬੱਚਾ_prog *ਬੱਚਾ;
	ਬਣਤਰ built_ਵਿੱਚ_command *x;
	ਅੱਖਰ *p;
# ਜੇ __GNUC__
	/* Avoid longjmp clobbering */
	(ਖਾਲੀ) &i;
	(ਖਾਲੀ) &nextin;
	(ਖਾਲੀ) &nextout;
	(ਖਾਲੀ) &ਬੱਚਾ;
# endif
#ਨਹੀਂਤਾਂ
	ਪੂਰਨ nextin;
	ਪੂਰਨ flag = do_repeat ? CMD_FLAG_REPEAT : 0;
	ਬਣਤਰ ਬੱਚਾ_prog *ਬੱਚਾ;
	ਅੱਖਰ *p;
# ਜੇ __GNUC__
	/* Avoid longjmp clobbering */
	(ਖਾਲੀ) &i;
	(ਖਾਲੀ) &nextin;
	(ਖਾਲੀ) &ਬੱਚਾ;
# endif
#ਜੇ_ਅੰਤ	/* __U_BOOT__ */

	nextin = 0;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	pi->pgrp = -1;
#ਜੇ_ਅੰਤ

	/* Check if this is a simple builtin (not part of a pipe).
	 * Builtins within pipes have to fork anyway, and are handled in
	 * pseudo_exec.  "echo foo | read bar" doesn't work on bash, either.
	 */
	ਜੇ (pi->ਅੰਕ_progs == 1) ਬੱਚਾ = & (pi->progs[0]);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	ਜੇ (pi->ਅੰਕ_progs == 1 && ਬੱਚਾ->ਸਮੂਹ && ਬੱਚਾ->subshell == 0) {
		ਪੂਰਨ squirrel[] = {-1, -1, -1};
		ਪੂਰਨ rcode;
		debug_printf("non-subshell grouping\n");
		ਤਿਆਰੀ_redirects(ਬੱਚਾ, squirrel);
		/* XXX could we merge code with following builtin case,
		 * by creating a pseudo builtin that calls run_list_real? */
		rcode = ਲੜੀਵਾਰ_list_real(ਬੱਚਾ->ਸਮੂਹ);
		restore_redirects(squirrel);
#ਨਹੀਂਤਾਂ
		ਜੇ (pi->ਅੰਕ_progs == 1 && ਬੱਚਾ->ਸਮੂਹ) {
		ਪੂਰਨ rcode;
		debug_printf("non-subshell grouping\n");
		rcode = ਲੜੀਵਾਰ_list_real(ਬੱਚਾ->ਸਮੂਹ);
#ਜੇ_ਅੰਤ
		ਵਾਪਸ rcode;
	} ਨਹੀਂਤਾਂ ਜੇ (pi->ਅੰਕ_progs == 1 && pi->progs[0].argv != ਨਿਰਾ) {
		ਲਈ (i=0; is_assignment(ਬੱਚਾ->argv[i]); i++) { /* nothing */ }
		ਜੇ (i!=0 && ਬੱਚਾ->argv[i]==ਨਿਰਾ) {
			/* assignments, but no command: set the local environment */
			ਲਈ (i=0; ਬੱਚਾ->argv[i]!=ਨਿਰਾ; i++) {

				/* Ok, this case is tricky.  We have to decide if this is a
				 * local variable, or an already exported variable.  If it is
				 * already exported, we have to export the new value.  If it is
				 * not exported, we need only set this as a local variable.
				 * This junk is all to decide whether or not to export this
				 * variable. */
				ਪੂਰਨ export_me=0;
				ਅੱਖਰ *name, *ਮੁੱਲ;
				name = xstrdup(ਬੱਚਾ->argv[i]);
				debug_printf("Local environment set: %s\n", name);
				ਮੁੱਲ = strchr(name, '=');
				ਜੇ (ਮੁੱਲ)
					*ਮੁੱਲ=0;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
				ਜੇ ( get_local_var(name)) {
					export_me=1;
				}
#ਜੇ_ਅੰਤ
				ਮੁਕਤ(name);
				p = ਪਾਓ_var_ਮੁੱਲ(ਬੱਚਾ->argv[i]);
				ਸੈੱਟ_local_var(p, export_me);
				ਜੇ (p != ਬੱਚਾ->argv[i]) ਮੁਕਤ(p);
			}
			ਵਾਪਸ EXIT_SUCCESS;   /* don't worry about errors in set_local_var() yet */
		}
		ਲਈ (i = 0; is_assignment(ਬੱਚਾ->argv[i]); i++) {
			p = ਪਾਓ_var_ਮੁੱਲ(ਬੱਚਾ->argv[i]);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
			putenv(strdup(p));
#ਨਹੀਂਤਾਂ
			ਸੈੱਟ_local_var(p, 0);
#ਜੇ_ਅੰਤ
			ਜੇ (p != ਬੱਚਾ->argv[i]) {
				ਬੱਚਾ->sp--;
				ਮੁਕਤ(p);
			}
		}
		ਜੇ (ਬੱਚਾ->sp) {
			ਅੱਖਰ * str = ਨਿਰਾ;

			str = make_string(ਬੱਚਾ->argv + i,
					  ਬੱਚਾ->argv_nonnull + i);
			parse_string_outer(str, FLAG_EXIT_FROM_LOOP | FLAG_REPARSING);
			ਮੁਕਤ(str);
			ਵਾਪਸ ਆਖਰੀ_return_ਕੋਡ;
		}
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
		ਲਈ (x = bltins; x->cmd; x++) {
			ਜੇ (strcmp(ਬੱਚਾ->argv[i], x->cmd) == 0 ) {
				ਪੂਰਨ squirrel[] = {-1, -1, -1};
				ਪੂਰਨ rcode;
				ਜੇ (x->function == builtin_exec && ਬੱਚਾ->argv[i+1]==ਨਿਰਾ) {
					debug_printf("magic exec\n");
					ਤਿਆਰੀ_redirects(ਬੱਚਾ,ਨਿਰਾ);
					ਵਾਪਸ EXIT_SUCCESS;
				}
				debug_printf("builtin inline %s\n", ਬੱਚਾ->argv[0]);
				/* XXX setup_redirects acts on file descriptors, not FILEs.
				 * This is perfect for work that comes after exec().
				 * Is it really safe for inline use?  Experimentally,
				 * things seem to work with glibc. */
				ਤਿਆਰੀ_redirects(ਬੱਚਾ, squirrel);

				ਬੱਚਾ->argv += i;  /* XXX horrible hack */
				rcode = x->function(ਬੱਚਾ);
				/* XXX restore hack so free() can work right */
				ਬੱਚਾ->argv -= i;
				restore_redirects(squirrel);
			}
			ਵਾਪਸ rcode;
		}
#ਨਹੀਂਤਾਂ
		/* check ";", because ,example , argv consist from
		 * "help;flinfo" must not execute
		 */
		ਜੇ (strchr(ਬੱਚਾ->argv[i], ';')) {
			ਲਿਖੋ("Unknown command '%s' - try 'help' or use "
					"'run' command\n", ਬੱਚਾ->argv[i]);
			ਵਾਪਸ -1;
		}
		/* Process the command */
		ਵਾਪਸ cmd_process(flag, ਬੱਚਾ->argc - i, ਬੱਚਾ->argv + i,
				   &flag_repeat, ਨਿਰਾ);
#ਜੇ_ਅੰਤ
	}
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__

	ਲਈ (i = 0; i < pi->ਅੰਕ_progs; i++) {
		ਬੱਚਾ = & (pi->progs[i]);

		/* pipes are inserted between pairs of commands */
		ਜੇ ((i + 1) < pi->ਅੰਕ_progs) {
			ਜੇ (pipe(pipefds)<0) perror_ਸੁਨੇਹਾ_and_die("pipe");
			nextout = pipefds[1];
		} ਨਹੀਂਤਾਂ {
			nextout=1;
			pipefds[0] = -1;
		}

		/* XXX test for failed fork()? */
		ਜੇ (!(ਬੱਚਾ->pid = fork())) {
			/* Set the handling for job control signals back to the default.  */
			ਸੰਕੇਤ(SIGINT, SIG_DFL);
			ਸੰਕੇਤ(SIGQUIT, SIG_DFL);
			ਸੰਕੇਤ(SIGTERM, SIG_DFL);
			ਸੰਕੇਤ(SIGTSTP, SIG_DFL);
			ਸੰਕੇਤ(SIGTTIN, SIG_DFL);
			ਸੰਕੇਤ(SIGTTOU, SIG_DFL);
			ਸੰਕੇਤ(SIGCHLD, SIG_DFL);

			close_all();

			ਜੇ (nextin != 0) {
				dup2(nextin, 0);
				close(nextin);
			}
			ਜੇ (nextout != 1) {
				dup2(nextout, 1);
				close(nextout);
			}
			ਜੇ (pipefds[0]!=-1) {
				close(pipefds[0]);  /* opposite end of our output pipe */
			}

			/* Like bash, explicit redirects override pipes,
			 * and the pipe fd is available for dup'ing. */
			ਤਿਆਰੀ_redirects(ਬੱਚਾ,ਨਿਰਾ);

			ਜੇ (interactive && pi->followup!=PIPE_BG) {
				/* If we (the child) win the race, put ourselves in the process
				 * group whose leader is the first process in this pipe. */
				ਜੇ (pi->pgrp < 0) {
					pi->pgrp = getpid();
				}
				ਜੇ (setpgid(0, pi->pgrp) == 0) {
					tcsetpgrp(2, pi->pgrp);
				}
			}

			pseudo_exec(ਬੱਚਾ);
		}

		/* put our child in the process group whose leader is the
		   first process in this pipe */
		ਜੇ (pi->pgrp < 0) {
			pi->pgrp = ਬੱਚਾ->pid;
		}
		/* Don't check for errors.  The child may be dead already,
		 * in which case setpgid returns error code EACCES. */
		setpgid(ਬੱਚਾ->pid, pi->pgrp);

		ਜੇ (nextin != 0)
			close(nextin);
		ਜੇ (nextout != 1)
			close(nextout);

		/* If there isn't another process, nextin is garbage
		   but it doesn't matter */
		nextin = pipefds[0];
	}
#ਜੇ_ਅੰਤ
	ਵਾਪਸ -1;
}

ਸਥਿਰ ਪੂਰਨ ਲੜੀਵਾਰ_list_real(ਬਣਤਰ pipe *pi)
{
	ਅੱਖਰ *save_name = ਨਿਰਾ;
	ਅੱਖਰ **list = ਨਿਰਾ;
	ਅੱਖਰ **save_list = ਨਿਰਾ;
	ਬਣਤਰ pipe *rpipe;
	ਪੂਰਨ flag_rep = 0;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	ਪੂਰਨ save_ਅੰਕ_progs;
#ਜੇ_ਅੰਤ
	ਪੂਰਨ rcode=0, flag_skip=1;
	ਪੂਰਨ flag_restore = 0;
	ਪੂਰਨ if_ਕੋਡ=0, ਅਗਲਾ_if_ਕੋਡ=0;  /* need double-buffer to handle elif */
	reserved_style rmode, skip_ਹੋਰ_ਵਿੱਚ_this_rmode=RES_XXXX;
	/* check syntax for "for" */
	ਲਈ (rpipe = pi; rpipe; rpipe = rpipe->ਅਗਲਾ) {
		ਜੇ ((rpipe->r_ਬਹੁਲਕ == RES_ਵਿੱਚ ||
		    rpipe->r_ਬਹੁਲਕ == RES_FOR) &&
		    (rpipe->ਅਗਲਾ == ਨਿਰਾ)) {
				syntax();
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
				flag_repeat = 0;
#ਜੇ_ਅੰਤ
				ਵਾਪਸ 1;
		}
		ਜੇ ((rpipe->r_ਬਹੁਲਕ == RES_ਵਿੱਚ &&
			(rpipe->ਅਗਲਾ->r_ਬਹੁਲਕ == RES_ਵਿੱਚ &&
			rpipe->ਅਗਲਾ->progs->argv != ਨਿਰਾ))||
			(rpipe->r_ਬਹੁਲਕ == RES_FOR &&
			rpipe->ਅਗਲਾ->r_ਬਹੁਲਕ != RES_ਵਿੱਚ)) {
				syntax();
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
				flag_repeat = 0;
#ਜੇ_ਅੰਤ
				ਵਾਪਸ 1;
		}
	}
	ਲਈ (; pi; pi = (flag_restore != 0) ? rpipe : pi->ਅਗਲਾ) {
		ਜੇ (pi->r_ਬਹੁਲਕ == RES_WHILE || pi->r_ਬਹੁਲਕ == RES_UNTIL ||
			pi->r_ਬਹੁਲਕ == RES_FOR) {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
				/* check Ctrl-C */
				ctrlc();
				ਜੇ ((had_ctrlc())) {
					ਵਾਪਸ 1;
				}
#ਜੇ_ਅੰਤ
				flag_restore = 0;
				ਜੇ (!rpipe) {
					flag_rep = 0;
					rpipe = pi;
				}
		}
		rmode = pi->r_ਬਹੁਲਕ;
		debug_printf("rmode=%d  if_code=%d  next_if_code=%d skip_more=%d\n", rmode, if_ਕੋਡ, ਅਗਲਾ_if_ਕੋਡ, skip_ਹੋਰ_ਵਿੱਚ_this_rmode);
		ਜੇ (rmode == skip_ਹੋਰ_ਵਿੱਚ_this_rmode && flag_skip) {
			ਜੇ (pi->followup == PIPE_SEQ) flag_skip=0;
			ਜਾਰੀ;
		}
		flag_skip = 1;
		skip_ਹੋਰ_ਵਿੱਚ_this_rmode = RES_XXXX;
		ਜੇ (rmode == RES_THEN || rmode == RES_ELSE) if_ਕੋਡ = ਅਗਲਾ_if_ਕੋਡ;
		ਜੇ (rmode == RES_THEN &&  if_ਕੋਡ) ਜਾਰੀ;
		ਜੇ (rmode == RES_ELSE && !if_ਕੋਡ) ਜਾਰੀ;
		ਜੇ (rmode == RES_ELIF && !if_ਕੋਡ) ਰੋਕੋ;
		ਜੇ (rmode == RES_FOR && pi->ਅੰਕ_progs) {
			ਜੇ (!list) {
				/* if no variable values after "in" we skip "for" */
				ਜੇ (!pi->ਅਗਲਾ->progs->argv) ਜਾਰੀ;
				/* create list of variable values */
				list = make_list_ਵਿੱਚ(pi->ਅਗਲਾ->progs->argv,
					pi->progs->argv[0]);
				save_list = list;
				save_name = pi->progs->argv[0];
				pi->progs->argv[0] = ਨਿਰਾ;
				flag_rep = 1;
			}
			ਜੇ (!(*list)) {
				ਮੁਕਤ(pi->progs->argv[0]);
				ਮੁਕਤ(save_list);
				list = ਨਿਰਾ;
				flag_rep = 0;
				pi->progs->argv[0] = save_name;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
				pi->progs->glob_ਨਤੀਜਾ.gl_pathv[0] =
					pi->progs->argv[0];
#ਜੇ_ਅੰਤ
				ਜਾਰੀ;
			} ਨਹੀਂਤਾਂ {
				/* insert new value from list for variable */
				ਮੁਕਤ(pi->progs->argv[0]);
				pi->progs->argv[0] = *list++;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
				pi->progs->glob_ਨਤੀਜਾ.gl_pathv[0] =
					pi->progs->argv[0];
#ਜੇ_ਅੰਤ
			}
		}
		ਜੇ (rmode == RES_ਵਿੱਚ) ਜਾਰੀ;
		ਜੇ (rmode == RES_DO) {
			ਜੇ (!flag_rep) ਜਾਰੀ;
		}
		ਜੇ (rmode == RES_ਪੂਰਾ) {
			ਜੇ (flag_rep) {
				flag_restore = 1;
			} ਨਹੀਂਤਾਂ {
				rpipe = ਨਿਰਾ;
			}
		}
		ਜੇ (pi->ਅੰਕ_progs == 0) ਜਾਰੀ;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
		save_ਅੰਕ_progs = pi->ਅੰਕ_progs; /* save number of programs */
#ਜੇ_ਅੰਤ
		rcode = ਲੜੀਵਾਰ_pipe_real(pi);
		debug_printf("run_pipe_real returned %d\n",rcode);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
		ਜੇ (rcode!=-1) {
			/* We only ran a builtin: rcode was set by the return value
			 * of run_pipe_real(), and we don't need to wait for anything. */
		} ਨਹੀਂਤਾਂ ਜੇ (pi->followup==PIPE_BG) {
			/* XXX check bash's behavior with nontrivial pipes */
			/* XXX compute jobid */
			/* XXX what does bash do with attempts to background builtins? */
			ਪਾਓ_bg_ਕੰਮ(pi);
			rcode = EXIT_SUCCESS;
		} ਨਹੀਂਤਾਂ {
			ਜੇ (interactive) {
				/* move the new process group into the foreground */
				ਜੇ (tcsetpgrp(shell_terminal, pi->pgrp) && errno != ENOTTY)
					perror_ਸੁਨੇਹਾ("tcsetpgrp-3");
				rcode = checkjobs(pi);
				/* move the shell to the foreground */
				ਜੇ (tcsetpgrp(shell_terminal, getpgid(0)) && errno != ENOTTY)
					perror_ਸੁਨੇਹਾ("tcsetpgrp-4");
			} ਨਹੀਂਤਾਂ {
				rcode = checkjobs(pi);
			}
			debug_printf("checkjobs returned %d\n",rcode);
		}
		ਆਖਰੀ_return_ਕੋਡ=rcode;
#ਨਹੀਂਤਾਂ
		ਜੇ (rcode < -1) {
			ਆਖਰੀ_return_ਕੋਡ = -rcode - 2;
			ਵਾਪਸ -2;	/* exit */
		}
		ਆਖਰੀ_return_ਕੋਡ = rcode;
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
		pi->ਅੰਕ_progs = save_ਅੰਕ_progs; /* restore number of programs */
#ਜੇ_ਅੰਤ
		ਜੇ ( rmode == RES_IF || rmode == RES_ELIF )
			ਅਗਲਾ_if_ਕੋਡ=rcode;  /* can be overwritten a number of times */
		ਜੇ (rmode == RES_WHILE)
			flag_rep = !ਆਖਰੀ_return_ਕੋਡ;
		ਜੇ (rmode == RES_UNTIL)
			flag_rep = ਆਖਰੀ_return_ਕੋਡ;
		ਜੇ ( (rcode==EXIT_SUCCESS && pi->followup==PIPE_OR) ||
		     (rcode!=EXIT_SUCCESS && pi->followup==PIPE_AND) )
			skip_ਹੋਰ_ਵਿੱਚ_this_rmode=rmode;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
		checkjobs(ਨਿਰਾ);
#ਜੇ_ਅੰਤ
	}
	ਵਾਪਸ rcode;
}

/* broken, of course, but OK for testing */
ਸਥਿਰ ਅੱਖਰ *indenter(ਪੂਰਨ i)
{
	ਸਥਿਰ ਅੱਖਰ blanks[]="                                    ";
	ਵਾਪਸ &blanks[ਆਕਾਰ(blanks)-i-1];
}

/* return code is the exit status of the pipe */
ਸਥਿਰ ਪੂਰਨ free_pipe(ਬਣਤਰ pipe *pi, ਪੂਰਨ indent)
{
	ਅੱਖਰ **p;
	ਬਣਤਰ ਬੱਚਾ_prog *ਬੱਚਾ;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	ਬਣਤਰ redir_struct *r, *rnext;
#ਜੇ_ਅੰਤ
	ਪੂਰਨ a, i, ret_ਕੋਡ=0;
	ਅੱਖਰ *ind = indenter(indent);

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	ਜੇ (pi->stopped_progs > 0)
		ਵਾਪਸ ret_ਕੋਡ;
	final_printf("%s run pipe: (pid %d)\n",ind,getpid());
#ਜੇ_ਅੰਤ
	ਲਈ (i=0; i<pi->ਅੰਕ_progs; i++) {
		ਬੱਚਾ = &pi->progs[i];
		final_printf("%s  command %d:\n",ind,i);
		ਜੇ (ਬੱਚਾ->argv) {
			ਲਈ (a=0,p=ਬੱਚਾ->argv; *p; a++,p++) {
				final_printf("%s   argv[%d] = %s\n",ind,a,*p);
			}
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
			globfree(&ਬੱਚਾ->glob_ਨਤੀਜਾ);
#ਨਹੀਂਤਾਂ
			ਲਈ (a = 0; a < ਬੱਚਾ->argc; a++) {
				ਮੁਕਤ(ਬੱਚਾ->argv[a]);
			}
			ਮੁਕਤ(ਬੱਚਾ->argv);
			ਮੁਕਤ(ਬੱਚਾ->argv_nonnull);
			ਬੱਚਾ->argc = 0;
#ਜੇ_ਅੰਤ
			ਬੱਚਾ->argv=ਨਿਰਾ;
		} ਨਹੀਂਤਾਂ ਜੇ (ਬੱਚਾ->ਸਮੂਹ) {
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
			final_printf("%s   begin group (subshell:%d)\n",ind, ਬੱਚਾ->subshell);
#ਜੇ_ਅੰਤ
			ret_ਕੋਡ = free_pipe_list(ਬੱਚਾ->ਸਮੂਹ,indent+3);
			final_printf("%s   end group\n",ind);
		} ਨਹੀਂਤਾਂ {
			final_printf("%s   (nil)\n",ind);
		}
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
		ਲਈ (r=ਬੱਚਾ->redirects; r; r=rnext) {
			final_printf("%s   redirect %d%s", ind, r->fd, redir_ਸਾਰਣੀ[r->type].descrip);
			ਜੇ (r->dup == -1) {
				/* guard against the case >$FOO, where foo is unset or blank */
				ਜੇ (r->ਸ਼ਬਦ.gl_pathv) {
					final_printf(" %s\n", *r->ਸ਼ਬਦ.gl_pathv);
					globfree(&r->ਸ਼ਬਦ);
				}
			} ਨਹੀਂਤਾਂ {
				final_printf("&%d\n", r->dup);
			}
			rnext=r->ਅਗਲਾ;
			ਮੁਕਤ(r);
		}
		ਬੱਚਾ->redirects=ਨਿਰਾ;
#ਜੇ_ਅੰਤ
	}
	ਮੁਕਤ(pi->progs);   /* children are an array, they get freed all at once */
	pi->progs=ਨਿਰਾ;
	ਵਾਪਸ ret_ਕੋਡ;
}

ਸਥਿਰ ਪੂਰਨ free_pipe_list(ਬਣਤਰ pipe *ਸਿਰ, ਪੂਰਨ indent)
{
	ਪੂਰਨ rcode=0;   /* if list has no members */
	ਬਣਤਰ pipe *pi, *ਅਗਲਾ;
	ਅੱਖਰ *ind = indenter(indent);
	ਲਈ (pi=ਸਿਰ; pi; pi=ਅਗਲਾ) {
		final_printf("%s pipe reserved mode %d\n", ind, pi->r_ਬਹੁਲਕ);
		rcode = free_pipe(pi, indent);
		final_printf("%s pipe followup code %d\n", ind, pi->followup);
		ਅਗਲਾ=pi->ਅਗਲਾ;
		pi->ਅਗਲਾ=ਨਿਰਾ;
		ਮੁਕਤ(pi);
	}
	ਵਾਪਸ rcode;
}

/* Select which version we will use */
ਸਥਿਰ ਪੂਰਨ ਲੜੀਵਾਰ_list(ਬਣਤਰ pipe *pi)
{
	ਪੂਰਨ rcode=0;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	ਜੇ (fake_ਬਹੁਲਕ==0) {
#ਜੇ_ਅੰਤ
		rcode = ਲੜੀਵਾਰ_list_real(pi);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	}
#ਜੇ_ਅੰਤ
	/* free_pipe_list has the side effect of clearing memory
	 * In the long run that function can be merged with run_list_real,
	 * but doing that now would hobble the debugging effort. */
	free_pipe_list(pi,0);
	ਵਾਪਸ rcode;
}

/* The API for glob is arguably broken.  This routine pushes a non-matching
 * string into the output structure, removing non-backslashed backslashes.
 * If someone can prove me wrong, by performing this function within the
 * original glob(3) api, feel free to rewrite this routine into oblivion.
 * Return code (0 vs. GLOB_NOSPACE) matches glob(3).
 * XXX broken if the last character is '\\', check that before calling.
 */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
ਸਥਿਰ ਪੂਰਨ globhack(ਅਚਲ ਅੱਖਰ *src, ਪੂਰਨ flags, glob_t *pglob)
{
	ਪੂਰਨ cnt=0, pathc;
	ਅਚਲ ਅੱਖਰ *s;
	ਅੱਖਰ *dest;
	ਲਈ (cnt=1, s=src; s && *s; s++) {
		ਜੇ (*s == '\\') s++;
		cnt++;
	}
	dest = ਸਮ੍ਰਿਤੀ_ਦਿਓ(cnt);
	ਜੇ (!dest) ਵਾਪਸ GLOB_NOSPACE;
	ਜੇ (!(flags & GLOB_APPEND)) {
		pglob->gl_pathv=ਨਿਰਾ;
		pglob->gl_pathc=0;
		pglob->gl_offs=0;
		pglob->gl_offs=0;
	}
	pathc = ++pglob->gl_pathc;
	pglob->gl_pathv = realloc(pglob->gl_pathv, (pathc+1)*ਆਕਾਰ(*pglob->gl_pathv));
	ਜੇ (pglob->gl_pathv == ਨਿਰਾ) ਵਾਪਸ GLOB_NOSPACE;
	pglob->gl_pathv[pathc-1]=dest;
	pglob->gl_pathv[pathc]=ਨਿਰਾ;
	ਲਈ (s=src; s && *s; s++, dest++) {
		ਜੇ (*s == '\\') s++;
		*dest = *s;
	}
	*dest='\0';
	ਵਾਪਸ 0;
}

/* XXX broken if the last character is '\\', check that before calling */
ਸਥਿਰ ਪੂਰਨ glob_needed(ਅਚਲ ਅੱਖਰ *s)
{
	ਲਈ (; *s; s++) {
		ਜੇ (*s == '\\') s++;
		ਜੇ (strchr("*[?",*s)) ਵਾਪਸ 1;
	}
	ਵਾਪਸ 0;
}

#ਜੇ 0
ਸਥਿਰ ਖਾਲੀ globprint(glob_t *pglob)
{
	ਪੂਰਨ i;
	debug_printf("glob_t at %p:\n", pglob);
	debug_printf("  gl_pathc=%d  gl_pathv=%p  gl_offs=%d  gl_flags=%d\n",
		pglob->gl_pathc, pglob->gl_pathv, pglob->gl_offs, pglob->gl_flags);
	ਲਈ (i=0; i<pglob->gl_pathc; i++)
		debug_printf("pglob->gl_pathv[%d] = %p = %s\n", i,
			pglob->gl_pathv[i], pglob->gl_pathv[i]);
}
#ਜੇ_ਅੰਤ

ਸਥਿਰ ਪੂਰਨ xglob(o_string *dest, ਪੂਰਨ flags, glob_t *pglob)
{
	ਪੂਰਨ gr;

	/* short-circuit for null word */
	/* we can code this better when the debug_printf's are gone */
	ਜੇ (dest->ਲੰਮਾਈ == 0) {
		ਜੇ (dest->nonnull) {
			/* bash man page calls this an "explicit" null */
			gr = globhack(dest->ਅੰਕੜੇ, flags, pglob);
			debug_printf("globhack returned %d\n",gr);
		} ਨਹੀਂਤਾਂ {
			ਵਾਪਸ 0;
		}
	} ਨਹੀਂਤਾਂ ਜੇ (glob_needed(dest->ਅੰਕੜੇ)) {
		gr = glob(dest->ਅੰਕੜੇ, flags, ਨਿਰਾ, pglob);
		debug_printf("glob returned %d\n",gr);
		ਜੇ (gr == GLOB_NOMATCH) {
			/* quote removal, or more accurately, backslash removal */
			gr = globhack(dest->ਅੰਕੜੇ, flags, pglob);
			debug_printf("globhack returned %d\n",gr);
		}
	} ਨਹੀਂਤਾਂ {
		gr = globhack(dest->ਅੰਕੜੇ, flags, pglob);
		debug_printf("globhack returned %d\n",gr);
	}
	ਜੇ (gr == GLOB_NOSPACE)
		error_ਸੁਨੇਹਾ_and_die("out of memory during glob");
	ਜੇ (gr != 0) { /* GLOB_ABORTED ? */
		error_ਸੁਨੇਹਾ("glob(3) error %d",gr);
	}
	/* globprint(glob_target); */
	ਵਾਪਸ gr;
}
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
ਸਥਿਰ ਅੱਖਰ *get_dollar_var(ਅੱਖਰ ch);
#ਜੇ_ਅੰਤ

/* This is used to get/check local shell variables */
ਅੱਖਰ *get_local_var(ਅਚਲ ਅੱਖਰ *s)
{
	ਬਣਤਰ variables *cur;

	ਜੇ (!s)
		ਵਾਪਸ ਨਿਰਾ;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	ਜੇ (*s == '$')
		ਵਾਪਸ get_dollar_var(s[1]);
#ਜੇ_ਅੰਤ

	ਲਈ (cur = ਸਿਖਰ_vars; cur; cur=cur->ਅਗਲਾ)
		ਜੇ(strcmp(cur->name, s)==0)
			ਵਾਪਸ cur->ਮੁੱਲ;
	ਵਾਪਸ ਨਿਰਾ;
}

/* This is used to set local shell variables
   flg_export==0 if only local (not exporting) variable
   flg_export==1 if "new" exporting environ
   flg_export>1  if current startup environ (not call putenv()) */
ਪੂਰਨ ਸੈੱਟ_local_var(ਅਚਲ ਅੱਖਰ *s, ਪੂਰਨ flg_export)
{
	ਅੱਖਰ *name, *ਮੁੱਲ;
	ਪੂਰਨ ਨਤੀਜਾ=0;
	ਬਣਤਰ variables *cur;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	/* might be possible! */
	ਜੇ (!ਅੱਖਰ_ਹੈ(*s))
		ਵਾਪਸ -1;
#ਜੇ_ਅੰਤ

	name=strdup(s);

	/* Assume when we enter this function that we are already in
	 * NAME=VALUE format.  So the first order of business is to
	 * split 's' on the '=' into 'name' and 'value' */
	ਮੁੱਲ = strchr(name, '=');
	ਜੇ (!ਮੁੱਲ) {
		ਮੁਕਤ(name);
		ਵਾਪਸ -1;
	}
	*ਮੁੱਲ++ = 0;

	ਜੇ (!*ਮੁੱਲ) {
		unset_local_var(name);
		ਮੁਕਤ(name);
		ਵਾਪਸ 0;
	}

	ਲਈ(cur = ਸਿਖਰ_vars; cur; cur = cur->ਅਗਲਾ) {
		ਜੇ(strcmp(cur->name, name)==0)
			ਰੋਕੋ;
	}

	ਜੇ(cur) {
		ਜੇ(strcmp(cur->ਮੁੱਲ, ਮੁੱਲ)==0) {
			ਜੇ(flg_export>0 && cur->flg_export==0)
				cur->flg_export=flg_export;
			ਨਹੀਂਤਾਂ
				ਨਤੀਜਾ++;
		} ਨਹੀਂਤਾਂ {
			ਜੇ(cur->flg_read_only) {
				error_ਸੁਨੇਹਾ("%s: readonly variable", name);
				ਨਤੀਜਾ = -1;
			} ਨਹੀਂਤਾਂ {
				ਜੇ(flg_export>0 || cur->flg_export>1)
					cur->flg_export=1;
				ਮੁਕਤ(cur->ਮੁੱਲ);

				cur->ਮੁੱਲ = strdup(ਮੁੱਲ);
			}
		}
	} ਨਹੀਂਤਾਂ {
		cur = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਆਕਾਰ(ਬਣਤਰ variables));
		ਜੇ(!cur) {
			ਨਤੀਜਾ = -1;
		} ਨਹੀਂਤਾਂ {
			cur->name = strdup(name);
			ਜੇ (cur->name == ਨਿਰਾ) {
				ਮੁਕਤ(cur);
				ਨਤੀਜਾ = -1;
			} ਨਹੀਂਤਾਂ {
				ਬਣਤਰ variables *bottom = ਸਿਖਰ_vars;
				cur->ਮੁੱਲ = strdup(ਮੁੱਲ);
				cur->ਅਗਲਾ = ਨਿਰਾ;
				cur->flg_export = flg_export;
				cur->flg_read_only = 0;
				ਜਦੋਂ(bottom->ਅਗਲਾ) bottom=bottom->ਅਗਲਾ;
				bottom->ਅਗਲਾ = cur;
			}
		}
	}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	ਜੇ(ਨਤੀਜਾ==0 && cur->flg_export==1) {
		*(ਮੁੱਲ-1) = '=';
		ਨਤੀਜਾ = putenv(name);
	} ਨਹੀਂਤਾਂ {
#ਜੇ_ਅੰਤ
		ਮੁਕਤ(name);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
		ਜੇ(ਨਤੀਜਾ>0)            /* equivalent to previous set */
			ਨਤੀਜਾ = 0;
	}
#ਜੇ_ਅੰਤ
	ਵਾਪਸ ਨਤੀਜਾ;
}

ਖਾਲੀ unset_local_var(ਅਚਲ ਅੱਖਰ *name)
{
	ਬਣਤਰ variables *cur;

	ਜੇ (name) {
		ਲਈ (cur = ਸਿਖਰ_vars; cur; cur=cur->ਅਗਲਾ) {
			ਜੇ(strcmp(cur->name, name)==0)
				ਰੋਕੋ;
		}
		ਜੇ (cur != ਨਿਰਾ) {
			ਬਣਤਰ variables *ਅਗਲਾ = ਸਿਖਰ_vars;
			ਜੇ(cur->flg_read_only) {
				error_ਸੁਨੇਹਾ("%s: readonly variable", name);
				ਵਾਪਸ;
			} ਨਹੀਂਤਾਂ {
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
				ਜੇ(cur->flg_export)
					unenv_ਸੈੱਟ(cur->name);
#ਜੇ_ਅੰਤ
				ਮੁਕਤ(cur->name);
				ਮੁਕਤ(cur->ਮੁੱਲ);
				ਜਦੋਂ (ਅਗਲਾ->ਅਗਲਾ != cur)
					ਅਗਲਾ = ਅਗਲਾ->ਅਗਲਾ;
				ਅਗਲਾ->ਅਗਲਾ = cur->ਅਗਲਾ;
			}
			ਮੁਕਤ(cur);
		}
	}
}

ਸਥਿਰ ਪੂਰਨ is_assignment(ਅਚਲ ਅੱਖਰ *s)
{
	ਜੇ (s == ਨਿਰਾ)
		ਵਾਪਸ 0;

	ਜੇ (!ਅੱਖਰ_ਹੈ(*s)) ਵਾਪਸ 0;
	++s;
	ਜਦੋਂ(ਅੱਖਰ_ਜਾਂ_ਅੰਕ(*s) || *s=='_') ++s;
	ਵਾਪਸ *s=='=';
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
/* the src parameter allows us to peek forward to a possible &n syntax
 * for file descriptor duplication, e.g., "2>&1".
 * Return code is 0 normally, 1 if a syntax error is detected in src.
 * Resource errors (in xmalloc) cause the process to exit */
ਸਥਿਰ ਪੂਰਨ ਤਿਆਰੀ_redirect(ਬਣਤਰ p_context *ctx, ਪੂਰਨ fd, redir_type style,
	ਬਣਤਰ ਵਿੱਚ_str *ਨਿਵੇਸ਼)
{
	ਬਣਤਰ ਬੱਚਾ_prog *ਬੱਚਾ=ctx->ਬੱਚਾ;
	ਬਣਤਰ redir_struct *redir = ਬੱਚਾ->redirects;
	ਬਣਤਰ redir_struct *ਆਖਰੀ_redir=ਨਿਰਾ;

	/* Create a new redir_struct and drop it onto the end of the linked list */
	ਜਦੋਂ(redir) {
		ਆਖਰੀ_redir=redir;
		redir=redir->ਅਗਲਾ;
	}
	redir = xmalloc(ਆਕਾਰ(ਬਣਤਰ redir_struct));
	redir->ਅਗਲਾ=ਨਿਰਾ;
	redir->ਸ਼ਬਦ.gl_pathv=ਨਿਰਾ;
	ਜੇ (ਆਖਰੀ_redir) {
		ਆਖਰੀ_redir->ਅਗਲਾ=redir;
	} ਨਹੀਂਤਾਂ {
		ਬੱਚਾ->redirects=redir;
	}

	redir->type=style;
	redir->fd= (fd==-1) ? redir_ਸਾਰਣੀ[style].default_fd : fd ;

	debug_printf("Redirect type %d%s\n", redir->fd, redir_ਸਾਰਣੀ[style].descrip);

	/* Check for a '2>&1' type redirect */
	redir->dup = redirect_dup_ਅੰਕ(ਨਿਵੇਸ਼);
	ਜੇ (redir->dup == -2) ਵਾਪਸ 1;  /* syntax error */
	ਜੇ (redir->dup != -1) {
		/* Erik had a check here that the file descriptor in question
		 * is legit; I postpone that to "run time"
		 * A "-" representation of "close me" shows up as a -3 here */
		debug_printf("Duplicating redirect '%d>&%d'\n", redir->fd, redir->dup);
	} ਨਹੀਂਤਾਂ {
		/* We do _not_ try to open the file that src points to,
		 * since we need to return and let src be expanded first.
		 * Set ctx->pending_redirect, so we know what to do at the
		 * end of the next parsed word.
		 */
		ctx->pending_redirect = redir;
	}
	ਵਾਪਸ 0;
}
#ਜੇ_ਅੰਤ

ਸਥਿਰ ਬਣਤਰ pipe *new_pipe(ਖਾਲੀ)
{
	ਬਣਤਰ pipe *pi;
	pi = xmalloc(ਆਕਾਰ(ਬਣਤਰ pipe));
	pi->ਅੰਕ_progs = 0;
	pi->progs = ਨਿਰਾ;
	pi->ਅਗਲਾ = ਨਿਰਾ;
	pi->followup = 0;  /* invalid */
	pi->r_ਬਹੁਲਕ = RES_NONE;
	ਵਾਪਸ pi;
}

ਸਥਿਰ ਖਾਲੀ initialize_context(ਬਣਤਰ p_context *ctx)
{
	ctx->pipe=ਨਿਰਾ;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	ctx->pending_redirect=ਨਿਰਾ;
#ਜੇ_ਅੰਤ
	ctx->ਬੱਚਾ=ਨਿਰਾ;
	ctx->list_ਸਿਰ=new_pipe();
	ctx->pipe=ctx->list_ਸਿਰ;
	ctx->w=RES_NONE;
	ctx->ਢੇਰ=ਨਿਰਾ;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	ctx->old_flag=0;
#ਜੇ_ਅੰਤ
	ਪੂਰਾ_command(ctx);   /* creates the memory for working child */
}

/* normal return is 0
 * if a reserved word is found, and processed, return 1
 * should handle if, then, elif, else, fi, for, while, until, do, done.
 * case, function, and select are obnoxious, save those for later.
 */
ਬਣਤਰ reserved_combo {
	ਅੱਖਰ *literal;
	ਪੂਰਨ ਕੋਡ;
	ਲੰਮਾ flag;
};
/* Mostly a list of accepted follow-up reserved words.
 * FLAG_END means we are done with the sequence, and are ready
 * to turn the compound list into a command.
 * FLAG_START means the word must start a new compound list.
 */
ਸਥਿਰ ਬਣਤਰ reserved_combo reserved_list[] = {
	{ "if",    RES_IF,    FLAG_THEN | FLAG_ਸ਼ੁਰੂ },
	{ "then",  RES_THEN,  FLAG_ELIF | FLAG_ELSE | FLAG_FI },
	{ "elif",  RES_ELIF,  FLAG_THEN },
	{ "else",  RES_ELSE,  FLAG_FI   },
	{ "fi",    RES_FI,    FLAG_ਸਿਰਾ  },
	{ "for",   RES_FOR,   FLAG_ਵਿੱਚ   | FLAG_ਸ਼ੁਰੂ },
	{ "while", RES_WHILE, FLAG_DO   | FLAG_ਸ਼ੁਰੂ },
	{ "until", RES_UNTIL, FLAG_DO   | FLAG_ਸ਼ੁਰੂ },
	{ "in",    RES_ਵਿੱਚ,    FLAG_DO   },
	{ "do",    RES_DO,    FLAG_ਪੂਰਾ },
	{ "done",  RES_ਪੂਰਾ,  FLAG_ਸਿਰਾ  }
};
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NRES (ਆਕਾਰ(reserved_list)/ਆਕਾਰ(ਬਣਤਰ reserved_combo))

ਸਥਿਰ ਪੂਰਨ reserved_ਸ਼ਬਦ(o_string *dest, ਬਣਤਰ p_context *ctx)
{
	ਬਣਤਰ reserved_combo *r;
	ਲਈ (r=reserved_list;
		r<reserved_list+NRES; r++) {
		ਜੇ (strcmp(dest->ਅੰਕੜੇ, r->literal) == 0) {
			debug_printf("found reserved word %s, code %d\n",r->literal,r->ਕੋਡ);
			ਜੇ (r->flag & FLAG_ਸ਼ੁਰੂ) {
				ਬਣਤਰ p_context *new = xmalloc(ਆਕਾਰ(ਬਣਤਰ p_context));
				debug_printf("push stack\n");
				ਜੇ (ctx->w == RES_ਵਿੱਚ || ctx->w == RES_FOR) {
					syntax();
					ਮੁਕਤ(new);
					ctx->w = RES_SNTX;
					b_reset(dest);
					ਵਾਪਸ 1;
				}
				*new = *ctx;   /* physical copy */
				initialize_context(ctx);
				ctx->ਢੇਰ=new;
			} ਨਹੀਂਤਾਂ ਜੇ ( ctx->w == RES_NONE || ! (ctx->old_flag & (1<<r->ਕੋਡ))) {
				syntax();
				ctx->w = RES_SNTX;
				b_reset(dest);
				ਵਾਪਸ 1;
			}
			ctx->w=r->ਕੋਡ;
			ctx->old_flag = r->flag;
			ਜੇ (ctx->old_flag & FLAG_ਸਿਰਾ) {
				ਬਣਤਰ p_context *old;
				debug_printf("pop stack\n");
				ਪੂਰਾ_pipe(ctx,PIPE_SEQ);
				old = ctx->ਢੇਰ;
				old->ਬੱਚਾ->ਸਮੂਹ = ctx->list_ਸਿਰ;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
				old->ਬੱਚਾ->subshell = 0;
#ਜੇ_ਅੰਤ
				*ctx = *old;   /* physical copy */
				ਮੁਕਤ(old);
			}
			b_reset (dest);
			ਵਾਪਸ 1;
		}
	}
	ਵਾਪਸ 0;
}

/* normal return is 0.
 * Syntax or xglob errors return 1. */
ਸਥਿਰ ਪੂਰਨ ਪੂਰਾ_ਸ਼ਬਦ(o_string *dest, ਬਣਤਰ p_context *ctx)
{
	ਬਣਤਰ ਬੱਚਾ_prog *ਬੱਚਾ=ctx->ਬੱਚਾ;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	glob_t *glob_ਨਿਸ਼ਾਨਾ;
	ਪੂਰਨ gr, flags = 0;
#ਨਹੀਂਤਾਂ
	ਅੱਖਰ *str, *s;
	ਪੂਰਨ argc, cnt;
#ਜੇ_ਅੰਤ

	debug_printf("done_word: %s %p\n", dest->ਅੰਕੜੇ, ਬੱਚਾ);
	ਜੇ (dest->ਲੰਮਾਈ == 0 && !dest->nonnull) {
		debug_printf("  true null, ignored\n");
		ਵਾਪਸ 0;
	}
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	ਜੇ (ctx->pending_redirect) {
		glob_ਨਿਸ਼ਾਨਾ = &ctx->pending_redirect->ਸ਼ਬਦ;
	} ਨਹੀਂਤਾਂ {
#ਜੇ_ਅੰਤ
		ਜੇ (ਬੱਚਾ->ਸਮੂਹ) {
			syntax();
			ਵਾਪਸ 1;  /* syntax error, groups and arglists don't mix */
		}
		ਜੇ (!ਬੱਚਾ->argv && (ctx->type & FLAG_PARSE_SEMICOLON)) {
			debug_printf("checking %s for reserved-ness\n",dest->ਅੰਕੜੇ);
			ਜੇ (reserved_ਸ਼ਬਦ(dest,ctx)) ਵਾਪਸ ctx->w==RES_SNTX;
		}
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
		glob_ਨਿਸ਼ਾਨਾ = &ਬੱਚਾ->glob_ਨਤੀਜਾ;
		ਜੇ (ਬੱਚਾ->argv) flags |= GLOB_APPEND;
#ਨਹੀਂਤਾਂ
		ਲਈ (cnt = 1, s = dest->ਅੰਕੜੇ; s && *s; s++) {
			ਜੇ (*s == '\\') s++;
			cnt++;
		}
		str = ਸਮ੍ਰਿਤੀ_ਦਿਓ(cnt);
		ਜੇ (!str) ਵਾਪਸ 1;
		ਜੇ ( ਬੱਚਾ->argv == ਨਿਰਾ) {
			ਬੱਚਾ->argc=0;
		}
		argc = ++ਬੱਚਾ->argc;
		ਬੱਚਾ->argv = realloc(ਬੱਚਾ->argv, (argc+1)*ਆਕਾਰ(*ਬੱਚਾ->argv));
		ਜੇ (ਬੱਚਾ->argv == ਨਿਰਾ) {
			ਮੁਕਤ(str);
			ਵਾਪਸ 1;
		}
		ਬੱਚਾ->argv_nonnull = realloc(ਬੱਚਾ->argv_nonnull,
					(argc+1)*ਆਕਾਰ(*ਬੱਚਾ->argv_nonnull));
		ਜੇ (ਬੱਚਾ->argv_nonnull == ਨਿਰਾ) {
			ਮੁਕਤ(str);
			ਵਾਪਸ 1;
		}
		ਬੱਚਾ->argv[argc-1]=str;
		ਬੱਚਾ->argv_nonnull[argc-1] = dest->nonnull;
		ਬੱਚਾ->argv[argc]=ਨਿਰਾ;
		ਬੱਚਾ->argv_nonnull[argc] = 0;
		ਲਈ (s = dest->ਅੰਕੜੇ; s && *s; s++,str++) {
			ਜੇ (*s == '\\') s++;
			*str = *s;
		}
		*str = '\0';
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	}
	gr = xglob(dest, flags, glob_ਨਿਸ਼ਾਨਾ);
	ਜੇ (gr != 0) ਵਾਪਸ 1;
#ਜੇ_ਅੰਤ

	b_reset(dest);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	ਜੇ (ctx->pending_redirect) {
		ctx->pending_redirect=ਨਿਰਾ;
		ਜੇ (glob_ਨਿਸ਼ਾਨਾ->gl_pathc != 1) {
			error_ਸੁਨੇਹਾ("ambiguous redirect");
			ਵਾਪਸ 1;
		}
	} ਨਹੀਂਤਾਂ {
		ਬੱਚਾ->argv = glob_ਨਿਸ਼ਾਨਾ->gl_pathv;
	}
#ਜੇ_ਅੰਤ
	ਜੇ (ctx->w == RES_FOR) {
		ਪੂਰਾ_ਸ਼ਬਦ(dest,ctx);
		ਪੂਰਾ_pipe(ctx,PIPE_SEQ);
	}
	ਵਾਪਸ 0;
}

/* The only possible error here is out of memory, in which case
 * xmalloc exits. */
ਸਥਿਰ ਪੂਰਨ ਪੂਰਾ_command(ਬਣਤਰ p_context *ctx)
{
	/* The child is really already in the pipe structure, so
	 * advance the pipe counter and make a new, null child.
	 * Only real trickiness here is that the uncommitted
	 * child structure, to which ctx->child points, is not
	 * counted in pi->num_progs. */
	ਬਣਤਰ pipe *pi=ctx->pipe;
	ਬਣਤਰ ਬੱਚਾ_prog *prog=ctx->ਬੱਚਾ;

	ਜੇ (prog && prog->ਸਮੂਹ == ਨਿਰਾ
		 && prog->argv == ਨਿਰਾ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
		 && prog->redirects == ਨਿਰਾ) {
#ਨਹੀਂਤਾਂ
										) {
#ਜੇ_ਅੰਤ
		debug_printf("done_command: skipping null command\n");
		ਵਾਪਸ 0;
	} ਨਹੀਂਤਾਂ ਜੇ (prog) {
		pi->ਅੰਕ_progs++;
		debug_printf("done_command: num_progs incremented to %d\n",pi->ਅੰਕ_progs);
	} ਨਹੀਂਤਾਂ {
		debug_printf("done_command: initializing\n");
	}
	pi->progs = xrealloc(pi->progs, ਆਕਾਰ(*pi->progs) * (pi->ਅੰਕ_progs+1));

	prog = pi->progs + pi->ਅੰਕ_progs;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	prog->redirects = ਨਿਰਾ;
#ਜੇ_ਅੰਤ
	prog->argv = ਨਿਰਾ;
	prog->argv_nonnull = ਨਿਰਾ;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	prog->is_stopped = 0;
#ਜੇ_ਅੰਤ
	prog->ਸਮੂਹ = ਨਿਰਾ;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	prog->glob_ਨਤੀਜਾ.gl_pathv = ਨਿਰਾ;
	prog->family = pi;
#ਜੇ_ਅੰਤ
	prog->sp = 0;
	ctx->ਬੱਚਾ = prog;
	prog->type = ctx->type;

	/* but ctx->pipe and ctx->list_head remain unchanged */
	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ ਪੂਰਾ_pipe(ਬਣਤਰ p_context *ctx, pipe_style type)
{
	ਬਣਤਰ pipe *new_p;
	ਪੂਰਾ_command(ctx);  /* implicit closure of previous command */
	debug_printf("done_pipe, type %d\n", type);
	ctx->pipe->followup = type;
	ctx->pipe->r_ਬਹੁਲਕ = ctx->w;
	new_p=new_pipe();
	ctx->pipe->ਅਗਲਾ = new_p;
	ctx->pipe = new_p;
	ctx->ਬੱਚਾ = ਨਿਰਾ;
	ਪੂਰਾ_command(ctx);  /* set up new pipe to accept commands */
	ਵਾਪਸ 0;
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
/* peek ahead in the in_str to find out if we have a "&n" construct,
 * as in "2>&1", that represents duplicating a file descriptor.
 * returns either -2 (syntax error), -1 (no &), or the number found.
 */
ਸਥਿਰ ਪੂਰਨ redirect_dup_ਅੰਕ(ਬਣਤਰ ਵਿੱਚ_str *ਨਿਵੇਸ਼)
{
	ਪੂਰਨ ch, d=0, ok=0;
	ch = b_peek(ਨਿਵੇਸ਼);
	ਜੇ (ch != '&') ਵਾਪਸ -1;

	b_getch(ਨਿਵੇਸ਼);  /* get the & */
	ch=b_peek(ਨਿਵੇਸ਼);
	ਜੇ (ch == '-') {
		b_getch(ਨਿਵੇਸ਼);
		ਵਾਪਸ -3;  /* "-" represents "close me" */
	}
	ਜਦੋਂ (ਅੰਕ_ਹੈ(ch)) {
		d = d*10+(ch-'0');
		ok=1;
		b_getch(ਨਿਵੇਸ਼);
		ch = b_peek(ਨਿਵੇਸ਼);
	}
	ਜੇ (ok) ਵਾਪਸ d;

	error_ਸੁਨੇਹਾ("ambiguous redirect");
	ਵਾਪਸ -2;
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
ਸਥਿਰ ਪੂਰਨ redirect_opt_ਅੰਕ(o_string *o)
{
	ਪੂਰਨ ਅੰਕ;

	ਜੇ (o->ਲੰਮਾਈ==0) ਵਾਪਸ -1;
	ਲਈ(ਅੰਕ=0; ਅੰਕ<o->ਲੰਮਾਈ; ਅੰਕ++) {
		ਜੇ (!ਅੰਕ_ਹੈ(*(o->ਅੰਕੜੇ+ਅੰਕ))) {
			ਵਾਪਸ -1;
		}
	}
	/* reuse num (and save an int) */
	ਅੰਕ=atoi(o->ਅੰਕੜੇ);
	b_reset(o);
	ਵਾਪਸ ਅੰਕ;
}

FILE *generate_ਧਾਰਾ_from_list(ਬਣਤਰ pipe *ਸਿਰ)
{
	FILE *pf;
#ਜੇ 1
	ਪੂਰਨ pid, channel[2];
	ਜੇ (pipe(channel)<0) perror_ਸੁਨੇਹਾ_and_die("pipe");
	pid=fork();
	ਜੇ (pid<0) {
		perror_ਸੁਨੇਹਾ_and_die("fork");
	} ਨਹੀਂਤਾਂ ਜੇ (pid==0) {
		close(channel[0]);
		ਜੇ (channel[1] != 1) {
			dup2(channel[1],1);
			close(channel[1]);
		}
#ਜੇ 0
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SURROGATE "surrogate response"
		ਲਿਖ(1,SURROGATE,ਆਕਾਰ(SURROGATE));
		_exit(ਲੜੀਵਾਰ_list(ਸਿਰ));
#ਨਹੀਂਤਾਂ
		_exit(ਲੜੀਵਾਰ_list_real(ਸਿਰ));   /* leaks memory */
#ਜੇ_ਅੰਤ
	}
	debug_printf("forked child %d\n",pid);
	close(channel[1]);
	pf = fdopen(channel[0],"r");
	debug_printf("pipe on FILE *%p\n",pf);
#ਨਹੀਂਤਾਂ
	free_pipe_list(ਸਿਰ,0);
	pf=popen("echo surrogate response","r");
	debug_printf("started fake pipe on FILE *%p\n",pf);
#ਜੇ_ਅੰਤ
	ਵਾਪਸ pf;
}

/* this version hacked for testing purposes */
/* return code is exit status of the process that is run. */
ਸਥਿਰ ਪੂਰਨ process_command_subs(o_string *dest, ਬਣਤਰ p_context *ctx, ਬਣਤਰ ਵਿੱਚ_str *ਨਿਵੇਸ਼, ਪੂਰਨ subst_ਸਿਰਾ)
{
	ਪੂਰਨ retcode;
	o_string ਨਤੀਜਾ=NULL_O_STRING;
	ਬਣਤਰ p_context inner;
	FILE *p;
	ਬਣਤਰ ਵਿੱਚ_str pipe_str;
	initialize_context(&inner);

	/* recursion to generate command */
	retcode = parse_ਧਾਰਾ(&ਨਤੀਜਾ, &inner, ਨਿਵੇਸ਼, subst_ਸਿਰਾ);
	ਜੇ (retcode != 0) ਵਾਪਸ retcode;  /* syntax error or EOF */
	ਪੂਰਾ_ਸ਼ਬਦ(&ਨਤੀਜਾ, &inner);
	ਪੂਰਾ_pipe(&inner, PIPE_SEQ);
	b_free(&ਨਤੀਜਾ);

	p=generate_ਧਾਰਾ_from_list(inner.list_ਸਿਰ);
	ਜੇ (p==ਨਿਰਾ) ਵਾਪਸ 1;
	mark_open(fileno(p));
	ਤਿਆਰੀ_file_ਵਿੱਚ_str(&pipe_str, p);

	/* now send results of command back into original context */
	retcode = parse_ਧਾਰਾ(dest, ctx, &pipe_str, '\0');
	/* XXX In case of a syntax error, should we try to kill the child?
	 * That would be tough to do right, so just read until EOF. */
	ਜੇ (retcode == 1) {
		ਜਦੋਂ (b_getch(&pipe_str)!=EOF) { /* discard */ };
	}

	debug_printf("done reading from pipe, pclose()ing\n");
	/* This is the step that wait()s for the child.  Should be pretty
	 * safe, since we just read an EOF from its stdout.  We could try
	 * to better, by using wait(), and keeping track of background jobs
	 * at the same time.  That would be a lot of work, and contrary
	 * to the KISS philosophy of this program. */
	mark_closed(fileno(p));
	retcode=pclose(p);
	free_pipe_list(inner.list_ਸਿਰ,0);
	debug_printf("pclosed, retcode=%d\n",retcode);
	/* XXX this process fails to trim a single trailing newline */
	ਵਾਪਸ retcode;
}

ਸਥਿਰ ਪੂਰਨ parse_ਸਮੂਹ(o_string *dest, ਬਣਤਰ p_context *ctx,
	ਬਣਤਰ ਵਿੱਚ_str *ਨਿਵੇਸ਼, ਪੂਰਨ ch)
{
	ਪੂਰਨ rcode, endch=0;
	ਬਣਤਰ p_context sub;
	ਬਣਤਰ ਬੱਚਾ_prog *ਬੱਚਾ = ctx->ਬੱਚਾ;
	ਜੇ (ਬੱਚਾ->argv) {
		syntax();
		ਵਾਪਸ 1;  /* syntax error, groups and arglists don't mix */
	}
	initialize_context(&sub);
	ਚੋਣ(ch) {
		ਹਾਲਤ '(': endch=')'; ਬੱਚਾ->subshell=1; ਰੋਕੋ;
		ਹਾਲਤ '{': endch='}'; ਰੋਕੋ;
		ਮੂਲ: syntax();   /* really logic error */
	}
	rcode=parse_ਧਾਰਾ(dest,&sub,ਨਿਵੇਸ਼,endch);
	ਪੂਰਾ_ਸ਼ਬਦ(dest,&sub); /* finish off the final word in the subcontext */
	ਪੂਰਾ_pipe(&sub, PIPE_SEQ);  /* and the final command there, too */
	ਬੱਚਾ->ਸਮੂਹ = sub.list_ਸਿਰ;
	ਵਾਪਸ rcode;
	/* child remains "open", available for possible redirects */
}
#ਜੇ_ਅੰਤ

/* basically useful version until someone wants to get fancier,
 * see the bash man page under "Parameter Expansion" */
ਸਥਿਰ ਅੱਖਰ *lookup_param(ਅੱਖਰ *src)
{
	ਅੱਖਰ *p;
	ਅੱਖਰ *sep;
	ਅੱਖਰ *default_val = ਨਿਰਾ;
	ਪੂਰਨ ਸੌਂਪੋ = 0;
	ਪੂਰਨ expand_ਖਾਲੀ_ਥਾਵਾਂ = 0;

	ਜੇ (!src)
		ਵਾਪਸ ਨਿਰਾ;

	sep = strchr(src, ':');

	ਜੇ (sep) {
		*sep = '\0';
		ਜੇ (*(sep + 1) == '-')
			default_val = sep+2;
		ਜੇ (*(sep + 1) == '=') {
			default_val = sep+2;
			ਸੌਂਪੋ = 1;
		}
		ਜੇ (*(sep + 1) == '+') {
			default_val = sep+2;
			expand_ਖਾਲੀ_ਥਾਵਾਂ = 1;
		}
	}

	p = env_get(src);
	ਜੇ (!p)
		p = get_local_var(src);

	ਜੇ (!p || ਲੜੀ_ਲੰਬਾਈ(p) == 0) {
		p = default_val;
		ਜੇ (ਸੌਂਪੋ) {
			ਅੱਖਰ *var = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਲੜੀ_ਲੰਬਾਈ(src)+ਲੜੀ_ਲੰਬਾਈ(default_val)+2);
			ਜੇ (var) {
				sprintf(var, "%s=%s", src, default_val);
				ਸੈੱਟ_local_var(var, 0);
			}
			ਮੁਕਤ(var);
		}
	} ਨਹੀਂਤਾਂ ਜੇ (expand_ਖਾਲੀ_ਥਾਵਾਂ) {
		p += ਲੜੀ_ਲੰਬਾਈ(p);
	}

	ਜੇ (sep)
		*sep = ':';

	ਵਾਪਸ p;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
ਸਥਿਰ ਅੱਖਰ *get_dollar_var(ਅੱਖਰ ch)
{
	ਸਥਿਰ ਅੱਖਰ buf[40];

	buf[0] = '\0';
	ਚੋਣ (ch) {
		ਹਾਲਤ '?':
			sprintf(buf, "%u", (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)ਆਖਰੀ_return_ਕੋਡ);
			ਰੋਕੋ;
		ਮੂਲ:
			ਵਾਪਸ ਨਿਰਾ;
	}
	ਵਾਪਸ buf;
}
#ਜੇ_ਅੰਤ

/* return code: 0 for OK, 1 for syntax error */
ਸਥਿਰ ਪੂਰਨ handle_dollar(o_string *dest, ਬਣਤਰ p_context *ctx, ਬਣਤਰ ਵਿੱਚ_str *ਨਿਵੇਸ਼)
{
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	ਪੂਰਨ i, advance=0;
#ਨਹੀਂਤਾਂ
	ਪੂਰਨ advance=0;
#ਜੇ_ਅੰਤ
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	ਅੱਖਰ sep[]=" ";
#ਜੇ_ਅੰਤ
	ਪੂਰਨ ch = ਨਿਵੇਸ਼->peek(ਨਿਵੇਸ਼);  /* first character after the $ */
	debug_printf("handle_dollar: ch=%c\n",ch);
	ਜੇ (ਅੱਖਰ_ਹੈ(ch)) {
		b_addchr(dest, SPECIAL_VAR_ਚਿੰਨ੍ਹ);
		ctx->ਬੱਚਾ->sp++;
		ਜਦੋਂ(ch=b_peek(ਨਿਵੇਸ਼),ਅੱਖਰ_ਜਾਂ_ਅੰਕ(ch) || ch=='_') {
			b_getch(ਨਿਵੇਸ਼);
			b_addchr(dest,ch);
		}
		b_addchr(dest, SPECIAL_VAR_ਚਿੰਨ੍ਹ);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	} ਨਹੀਂਤਾਂ ਜੇ (ਅੰਕ_ਹੈ(ch)) {
		i = ch-'0';  /* XXX is $0 special? */
		ਜੇ (i<global_argc) {
			parse_string(dest, ctx, global_argv[i]); /* recursion */
		}
		advance = 1;
#ਜੇ_ਅੰਤ
	} ਨਹੀਂਤਾਂ ਚੋਣ (ch) {
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
		ਹਾਲਤ '$':
			b_adduint(dest,getpid());
			advance = 1;
			ਰੋਕੋ;
		ਹਾਲਤ '!':
			ਜੇ (ਆਖਰੀ_bg_pid > 0) b_adduint(dest, ਆਖਰੀ_bg_pid);
			advance = 1;
			ਰੋਕੋ;
#ਜੇ_ਅੰਤ
		ਹਾਲਤ '?':
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
			b_adduint(dest,ਆਖਰੀ_return_ਕੋਡ);
#ਨਹੀਂਤਾਂ
			ctx->ਬੱਚਾ->sp++;
			b_addchr(dest, SPECIAL_VAR_ਚਿੰਨ੍ਹ);
			b_addchr(dest, '$');
			b_addchr(dest, '?');
			b_addchr(dest, SPECIAL_VAR_ਚਿੰਨ੍ਹ);
#ਜੇ_ਅੰਤ
			advance = 1;
			ਰੋਕੋ;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
		ਹਾਲਤ '#':
			b_adduint(dest,global_argc ? global_argc-1 : 0);
			advance = 1;
			ਰੋਕੋ;
#ਜੇ_ਅੰਤ
		ਹਾਲਤ '{':
			b_addchr(dest, SPECIAL_VAR_ਚਿੰਨ੍ਹ);
			ctx->ਬੱਚਾ->sp++;
			b_getch(ਨਿਵੇਸ਼);
			/* XXX maybe someone will try to escape the '}' */
			ਜਦੋਂ(ch=b_getch(ਨਿਵੇਸ਼),ch!=EOF && ch!='}') {
				b_addchr(dest,ch);
			}
			ਜੇ (ch != '}') {
				syntax();
				ਵਾਪਸ 1;
			}
			b_addchr(dest, SPECIAL_VAR_ਚਿੰਨ੍ਹ);
			ਰੋਕੋ;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
		ਹਾਲਤ '(':
			b_getch(ਨਿਵੇਸ਼);
			process_command_subs(dest, ctx, ਨਿਵੇਸ਼, ')');
			ਰੋਕੋ;
		ਹਾਲਤ '*':
			sep[0]=ifs[0];
			ਲਈ (i=1; i<global_argc; i++) {
				parse_string(dest, ctx, global_argv[i]);
				ਜੇ (i+1 < global_argc) parse_string(dest, ctx, sep);
			}
			ਰੋਕੋ;
		ਹਾਲਤ '@':
		ਹਾਲਤ '-':
		ਹਾਲਤ '_':
			/* still unhandled, but should be eventually */
			error_ਸੁਨੇਹਾ("unhandled syntax: $%c",ch);
			ਵਾਪਸ 1;
			ਰੋਕੋ;
#ਜੇ_ਅੰਤ
		ਮੂਲ:
			b_addqchr(dest,'$',dest->quote);
	}
	/* Eat the character if the flag was set.  If the compiler
	 * is smart enough, we could substitute "b_getch(input);"
	 * for all the "advance = 1;" above, and also end up with
	 * a nice size-optimized program.  Hah!  That'll be the day.
	 */
	ਜੇ (advance) b_getch(ਨਿਵੇਸ਼);
	ਵਾਪਸ 0;
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
ਪੂਰਨ parse_string(o_string *dest, ਬਣਤਰ p_context *ctx, ਅਚਲ ਅੱਖਰ *src)
{
	ਬਣਤਰ ਵਿੱਚ_str foo;
	ਤਿਆਰੀ_string_ਵਿੱਚ_str(&foo, src);
	ਵਾਪਸ parse_ਧਾਰਾ(dest, ctx, &foo, '\0');
}
#ਜੇ_ਅੰਤ

/* return code is 0 for normal exit, 1 for syntax error */
ਸਥਿਰ ਪੂਰਨ parse_ਧਾਰਾ(o_string *dest, ਬਣਤਰ p_context *ctx,
			ਬਣਤਰ ਵਿੱਚ_str *ਨਿਵੇਸ਼, ਪੂਰਨ ਸਿਰਾ_trigger)
{
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ ch, m;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	ਪੂਰਨ redir_fd;
	redir_type redir_style;
#ਜੇ_ਅੰਤ
	ਪੂਰਨ ਅਗਲਾ;

	/* Only double-quote state is handled in the state variable dest->quote.
	 * A single-quote triggers a bypass of the main loop until its mate is
	 * found.  When recursing, quote state is passed in via dest->quote. */

	debug_printf("parse_stream, end_trigger=%d\n",ਸਿਰਾ_trigger);
	ਜਦੋਂ ((ch=b_getch(ਨਿਵੇਸ਼))!=EOF) {
		m = map[ch];
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
		ਜੇ (ਨਿਵੇਸ਼->__promptme == 0) ਵਾਪਸ 1;
#ਜੇ_ਅੰਤ
		ਅਗਲਾ = (ch == '\n') ? 0 : b_peek(ਨਿਵੇਸ਼);

		debug_printf("parse_stream: ch=%c (%d) m=%d quote=%d - %c\n",
			ch >= ' ' ? ch : '.', ch, m,
			dest->quote, ctx->ਢੇਰ == ਨਿਰਾ ? '*' : '.');

		ਜੇ (m==0 || ((m==1 || m==2) && dest->quote)) {
			b_addqchr(dest, ch, dest->quote);
		} ਨਹੀਂਤਾਂ {
			ਜੇ (m==2) {  /* unquoted IFS */
				ਜੇ (ਪੂਰਾ_ਸ਼ਬਦ(dest, ctx)) {
					ਵਾਪਸ 1;
				}
				/* If we aren't performing a substitution, treat a newline as a
				 * command separator.  */
				ਜੇ (ਸਿਰਾ_trigger != '\0' && ch=='\n')
					ਪੂਰਾ_pipe(ctx,PIPE_SEQ);
			}
			ਜੇ (ch == ਸਿਰਾ_trigger && !dest->quote && ctx->w==RES_NONE) {
				debug_printf("leaving parse_stream (triggered)\n");
				ਵਾਪਸ 0;
			}
#ਜੇ 0
			ਜੇ (ch=='\n') {
				/* Yahoo!  Time to run with it! */
				ਪੂਰਾ_pipe(ctx,PIPE_SEQ);
				ਲੜੀਵਾਰ_list(ctx->list_ਸਿਰ);
				initialize_context(ctx);
			}
#ਜੇ_ਅੰਤ
			ਜੇ (m!=2) ਚੋਣ (ch) {
		ਹਾਲਤ '#':
			ਜੇ (dest->ਲੰਮਾਈ == 0 && !dest->quote) {
				ਜਦੋਂ(ch=b_peek(ਨਿਵੇਸ਼),ch!=EOF && ch!='\n') { b_getch(ਨਿਵੇਸ਼); }
			} ਨਹੀਂਤਾਂ {
				b_addqchr(dest, ch, dest->quote);
			}
			ਰੋਕੋ;
		ਹਾਲਤ '\\':
			ਜੇ (ਅਗਲਾ == EOF) {
				syntax();
				ਵਾਪਸ 1;
			}
			b_addqchr(dest, '\\', dest->quote);
			b_addqchr(dest, b_getch(ਨਿਵੇਸ਼), dest->quote);
			ਰੋਕੋ;
		ਹਾਲਤ '$':
			ਜੇ (handle_dollar(dest, ctx, ਨਿਵੇਸ਼)!=0) ਵਾਪਸ 1;
			ਰੋਕੋ;
		ਹਾਲਤ '\'':
			dest->nonnull = 1;
			ਜਦੋਂ(ch=b_getch(ਨਿਵੇਸ਼),ch!=EOF && ch!='\'') {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
				ਜੇ(ਨਿਵੇਸ਼->__promptme == 0) ਵਾਪਸ 1;
#ਜੇ_ਅੰਤ
				b_addchr(dest,ch);
			}
			ਜੇ (ch==EOF) {
				syntax();
				ਵਾਪਸ 1;
			}
			ਰੋਕੋ;
		ਹਾਲਤ '"':
			dest->nonnull = 1;
			dest->quote = !dest->quote;
			ਰੋਕੋ;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
		ਹਾਲਤ '`':
			process_command_subs(dest, ctx, ਨਿਵੇਸ਼, '`');
			ਰੋਕੋ;
		ਹਾਲਤ '>':
			redir_fd = redirect_opt_ਅੰਕ(dest);
			ਪੂਰਾ_ਸ਼ਬਦ(dest, ctx);
			redir_style=REDIRECT_OVERWRITE;
			ਜੇ (ਅਗਲਾ == '>') {
				redir_style=REDIRECT_APPEND;
				b_getch(ਨਿਵੇਸ਼);
			} ਨਹੀਂਤਾਂ ਜੇ (ਅਗਲਾ == '(') {
				syntax();   /* until we support >(list) Process Substitution */
				ਵਾਪਸ 1;
			}
			ਤਿਆਰੀ_redirect(ctx, redir_fd, redir_style, ਨਿਵੇਸ਼);
			ਰੋਕੋ;
		ਹਾਲਤ '<':
			redir_fd = redirect_opt_ਅੰਕ(dest);
			ਪੂਰਾ_ਸ਼ਬਦ(dest, ctx);
			redir_style=REDIRECT_ਨਿਵੇਸ਼;
			ਜੇ (ਅਗਲਾ == '<') {
				redir_style=REDIRECT_HEREIS;
				b_getch(ਨਿਵੇਸ਼);
			} ਨਹੀਂਤਾਂ ਜੇ (ਅਗਲਾ == '>') {
				redir_style=REDIRECT_IO;
				b_getch(ਨਿਵੇਸ਼);
			} ਨਹੀਂਤਾਂ ਜੇ (ਅਗਲਾ == '(') {
				syntax();   /* until we support <(list) Process Substitution */
				ਵਾਪਸ 1;
			}
			ਤਿਆਰੀ_redirect(ctx, redir_fd, redir_style, ਨਿਵੇਸ਼);
			ਰੋਕੋ;
#ਜੇ_ਅੰਤ
		ਹਾਲਤ ';':
			ਪੂਰਾ_ਸ਼ਬਦ(dest, ctx);
			ਪੂਰਾ_pipe(ctx,PIPE_SEQ);
			ਰੋਕੋ;
		ਹਾਲਤ '&':
			ਪੂਰਾ_ਸ਼ਬਦ(dest, ctx);
			ਜੇ (ਅਗਲਾ=='&') {
				b_getch(ਨਿਵੇਸ਼);
				ਪੂਰਾ_pipe(ctx,PIPE_AND);
			} ਨਹੀਂਤਾਂ {
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
				ਪੂਰਾ_pipe(ctx,PIPE_BG);
#ਨਹੀਂਤਾਂ
				syntax_err();
				ਵਾਪਸ 1;
#ਜੇ_ਅੰਤ
			}
			ਰੋਕੋ;
		ਹਾਲਤ '|':
			ਪੂਰਾ_ਸ਼ਬਦ(dest, ctx);
			ਜੇ (ਅਗਲਾ=='|') {
				b_getch(ਨਿਵੇਸ਼);
				ਪੂਰਾ_pipe(ctx,PIPE_OR);
			} ਨਹੀਂਤਾਂ {
				/* we could pick up a file descriptor choice here
				 * with redirect_opt_num(), but bash doesn't do it.
				 * "echo foo 2| cat" yields "foo 2". */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
				ਪੂਰਾ_command(ctx);
#ਨਹੀਂਤਾਂ
				syntax_err();
				ਵਾਪਸ 1;
#ਜੇ_ਅੰਤ
			}
			ਰੋਕੋ;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
		ਹਾਲਤ '(':
		ਹਾਲਤ '{':
			ਜੇ (parse_ਸਮੂਹ(dest, ctx, ਨਿਵੇਸ਼, ch)!=0) ਵਾਪਸ 1;
			ਰੋਕੋ;
		ਹਾਲਤ ')':
		ਹਾਲਤ '}':
			syntax();   /* Proper use of this character caught by end_trigger */
			ਵਾਪਸ 1;
			ਰੋਕੋ;
#ਜੇ_ਅੰਤ
		ਹਾਲਤ SUBSTED_VAR_ਚਿੰਨ੍ਹ:
			dest->nonnull = 1;
			ਜਦੋਂ (ch = b_getch(ਨਿਵੇਸ਼), ch != EOF &&
			    ch != SUBSTED_VAR_ਚਿੰਨ੍ਹ) {
				debug_printf("subst, pass=%d\n", ch);
				ਜੇ (ਨਿਵੇਸ਼->__promptme == 0)
					ਵਾਪਸ 1;
				b_addchr(dest, ch);
			}
			debug_printf("subst, term=%d\n", ch);
			ਜੇ (ch == EOF) {
				syntax();
				ਵਾਪਸ 1;
			}
			ਰੋਕੋ;
		ਮੂਲ:
			syntax();   /* this is really an internal logic error */
			ਵਾਪਸ 1;
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
	ਜੇ (ਸਿਰਾ_trigger != '\0') ਵਾਪਸ -1;
	ਵਾਪਸ 0;
}

ਸਥਿਰ ਖਾਲੀ mapset(ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *ਸੈੱਟ, ਪੂਰਨ ਕੋਡ)
{
	ਅਚਲ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *s;
	ਲਈ (s=ਸੈੱਟ; *s; s++) map[*s] = ਕੋਡ;
}

ਸਥਿਰ ਖਾਲੀ update_ifs_map(ਖਾਲੀ)
{
	/* char *ifs and char map[256] are both globals. */
	ifs = (uchar *)env_get("IFS");
	ਜੇ (ifs == ਨਿਰਾ) ifs=(uchar *)" \t\n";
	/* Precompute a list of 'flow through' behavior so it can be treated
	 * quickly up front.  Computation is necessary because of IFS.
	 * Special case handling of IFS == " \t\n" is not implemented.
	 * The map[] array only really needs two bits each, and on most machines
	 * that would be faster because of the reduced L1 cache footprint.
	 */
	memset(map,0,ਆਕਾਰ(map)); /* most characters flow through always */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	mapset((uchar *)"\\$'\"`", 3);      /* never flow through */
	mapset((uchar *)"<>;&|(){}#", 1);   /* flow through if quoted */
#ਨਹੀਂਤਾਂ
	{
		uchar subst[2] = {SUBSTED_VAR_ਚਿੰਨ੍ਹ, 0};
		mapset(subst, 3);       /* never flow through */
	}
	mapset((uchar *)"\\$'\"", 3);       /* never flow through */
	mapset((uchar *)";&|#", 1);         /* flow through if quoted */
#ਜੇ_ਅੰਤ
	mapset(ifs, 2);            /* also flow through if quoted */
}

/* most recursion does not come through here, the exeception is
 * from builtin_source() */
ਸਥਿਰ ਪੂਰਨ parse_ਧਾਰਾ_outer(ਬਣਤਰ ਵਿੱਚ_str *inp, ਪੂਰਨ flag)
{

	ਬਣਤਰ p_context ctx;
	o_string ਆਰਜ਼ੀ=NULL_O_STRING;
	ਪੂਰਨ rcode;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	ਪੂਰਨ ਕੋਡ = 1;
#ਜੇ_ਅੰਤ
	ਕਰੋ {
		ctx.type = flag;
		initialize_context(&ctx);
		update_ifs_map();
		ਜੇ (!(flag & FLAG_PARSE_SEMICOLON) || (flag & FLAG_REPARSING)) mapset((uchar *)";$&|", 0);
		inp->promptmode=1;
		rcode = parse_ਧਾਰਾ(&ਆਰਜ਼ੀ, &ctx, inp,
				     flag & FLAG_CONT_ON_NEWLINE ? -1 : '\n');
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
		ਜੇ (rcode == 1) flag_repeat = 0;
#ਜੇ_ਅੰਤ
		ਜੇ (rcode != 1 && ctx.old_flag != 0) {
			syntax();
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
			flag_repeat = 0;
#ਜੇ_ਅੰਤ
		}
		ਜੇ (rcode != 1 && ctx.old_flag == 0) {
			ਪੂਰਾ_ਸ਼ਬਦ(&ਆਰਜ਼ੀ, &ctx);
			ਪੂਰਾ_pipe(&ctx,PIPE_SEQ);
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
			ਲੜੀਵਾਰ_list(ctx.list_ਸਿਰ);
#ਨਹੀਂਤਾਂ
			ਕੋਡ = ਲੜੀਵਾਰ_list(ctx.list_ਸਿਰ);
			ਜੇ (ਕੋਡ == -2) {	/* exit */
				b_free(&ਆਰਜ਼ੀ);
				ਕੋਡ = 0;
				/* XXX hackish way to not allow exit from main loop */
				ਜੇ (inp->peek == file_peek) {
					ਲਿਖੋ("exit not allowed from main input shell.\n");
					ਜਾਰੀ;
				}
				/*
				 * DANGER
				 * Return code -2 is special in this context,
				 * it indicates exit from inner pipe instead
				 * of return code itself, the return code is
				 * stored in 'last_return_code' variable!
				 * DANGER
				 */
				ਵਾਪਸ -2;
			}
			ਜੇ (ਕੋਡ == -1)
			    flag_repeat = 0;
#ਜੇ_ਅੰਤ
		} ਨਹੀਂਤਾਂ {
			ਜੇ (ctx.old_flag != 0) {
				ਮੁਕਤ(ctx.ਢੇਰ);
				b_reset(&ਆਰਜ਼ੀ);
			}
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
			ਜੇ (inp->__promptme == 0) ਲਿਖੋ("<INTERRUPT>\n");
			inp->__promptme = 1;
#ਜੇ_ਅੰਤ
			ਆਰਜ਼ੀ.nonnull = 0;
			ਆਰਜ਼ੀ.quote = 0;
			inp->p = ਨਿਰਾ;
			free_pipe_list(ctx.list_ਸਿਰ,0);
		}
		b_free(&ਆਰਜ਼ੀ);
	/* loop on syntax errors, return on EOF */
	} ਜਦੋਂ (rcode != -1 && !(flag & FLAG_EXIT_FROM_LOOP) &&
		(inp->peek != static_peek || b_peek(inp)));
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	ਵਾਪਸ 0;
#ਨਹੀਂਤਾਂ
	ਵਾਪਸ (ਕੋਡ != 0) ? 1 : 0;
#ਜੇ_ਅੰਤ /* __U_BOOT__ */
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
ਸਥਿਰ ਪੂਰਨ parse_string_outer(ਅਚਲ ਅੱਖਰ *s, ਪੂਰਨ flag)
#ਨਹੀਂਤਾਂ
ਪੂਰਨ parse_string_outer(ਅਚਲ ਅੱਖਰ *s, ਪੂਰਨ flag)
#ਜੇ_ਅੰਤ	/* __U_BOOT__ */
{
	ਬਣਤਰ ਵਿੱਚ_str ਨਿਵੇਸ਼;
	ਪੂਰਨ rcode;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	ਅੱਖਰ *p = ਨਿਰਾ;
	ਜੇ (!s)
		ਵਾਪਸ 1;
	ਜੇ (!*s)
		ਵਾਪਸ 0;
	ਜੇ (!(p = strchr(s, '\n')) || *++p) {
		p = xmalloc(ਲੜੀ_ਲੰਬਾਈ(s) + 2);
		strcpy(p, s);
		strcat(p, "\n");
		ਤਿਆਰੀ_string_ਵਿੱਚ_str(&ਨਿਵੇਸ਼, p);
		rcode = parse_ਧਾਰਾ_outer(&ਨਿਵੇਸ਼, flag);
		ਮੁਕਤ(p);
		ਵਾਪਸ rcode == -2 ? ਆਖਰੀ_return_ਕੋਡ : rcode;
	} ਨਹੀਂਤਾਂ {
#ਜੇ_ਅੰਤ
	ਤਿਆਰੀ_string_ਵਿੱਚ_str(&ਨਿਵੇਸ਼, s);
	rcode = parse_ਧਾਰਾ_outer(&ਨਿਵੇਸ਼, flag);
	ਵਾਪਸ rcode == -2 ? ਆਖਰੀ_return_ਕੋਡ : rcode;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	}
#ਜੇ_ਅੰਤ
}

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
ਸਥਿਰ ਪੂਰਨ parse_file_outer(FILE *f)
#ਨਹੀਂਤਾਂ
ਪੂਰਨ parse_file_outer(ਖਾਲੀ)
#ਜੇ_ਅੰਤ
{
	ਪੂਰਨ rcode;
	ਬਣਤਰ ਵਿੱਚ_str ਨਿਵੇਸ਼;
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
	ਤਿਆਰੀ_file_ਵਿੱਚ_str(&ਨਿਵੇਸ਼, f);
#ਨਹੀਂਤਾਂ
	ਤਿਆਰੀ_file_ਵਿੱਚ_str(&ਨਿਵੇਸ਼);
#ਜੇ_ਅੰਤ
	rcode = parse_ਧਾਰਾ_outer(&ਨਿਵੇਸ਼, FLAG_PARSE_SEMICOLON);
	ਵਾਪਸ rcode == -2 ? ਆਖਰੀ_return_ਕੋਡ : rcode;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
ਪੂਰਨ u_boot_hush_ਸ਼ੁਰੂ(ਖਾਲੀ)
{
	ਜੇ (ਸਿਖਰ_vars == ਨਿਰਾ) {
		ਸਿਖਰ_vars = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਆਕਾਰ(ਬਣਤਰ variables));
		ਸਿਖਰ_vars->name = "HUSH_VERSION";
		ਸਿਖਰ_vars->ਮੁੱਲ = "0.01";
		ਸਿਖਰ_vars->ਅਗਲਾ = ਨਿਰਾ;
		ਸਿਖਰ_vars->flg_export = 0;
		ਸਿਖਰ_vars->flg_read_only = 1;
	}
	ਵਾਪਸ 0;
}

ਸਥਿਰ ਖਾਲੀ *xmalloc(size_t ਮਾਪ)
{
	ਖਾਲੀ *p = ਨਿਰਾ;

	ਜੇ (!(p = ਸਮ੍ਰਿਤੀ_ਦਿਓ(ਮਾਪ))) {
	    ਲਿਖੋ("ERROR : xmalloc failed\n");
	    ਲਈ(;;);
	}
	ਵਾਪਸ p;
}

ਸਥਿਰ ਖਾਲੀ *xrealloc(ਖਾਲੀ *ptr, size_t ਮਾਪ)
{
	ਖਾਲੀ *p = ਨਿਰਾ;

	ਜੇ (!(p = realloc(ptr, ਮਾਪ))) {
	    ਲਿਖੋ("ERROR : xrealloc failed\n");
	    ਲਈ(;;);
	}
	ਵਾਪਸ p;
}
#ਜੇ_ਅੰਤ /* __U_BOOT__ */

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
/* Make sure we have a controlling tty.  If we get started under a job
 * aware app (like bash for example), make sure we are now in charge so
 * we don't fight over who gets the foreground */
ਸਥਿਰ ਖਾਲੀ ਤਿਆਰੀ_ਕੰਮ_control(ਖਾਲੀ)
{
	ਸਥਿਰ pid_t shell_pgrp;
	/* Loop until we are in the foreground.  */
	ਜਦੋਂ (tcgetpgrp (shell_terminal) != (shell_pgrp = getpgrp ()))
		kill (- shell_pgrp, SIGTTIN);

	/* Ignore interactive and job-control signals.  */
	ਸੰਕੇਤ(SIGINT, SIG_IGN);
	ਸੰਕੇਤ(SIGQUIT, SIG_IGN);
	ਸੰਕੇਤ(SIGTERM, SIG_IGN);
	ਸੰਕੇਤ(SIGTSTP, SIG_IGN);
	ਸੰਕੇਤ(SIGTTIN, SIG_IGN);
	ਸੰਕੇਤ(SIGTTOU, SIG_IGN);
	ਸੰਕੇਤ(SIGCHLD, SIG_IGN);

	/* Put ourselves in our own process group.  */
	setsid();
	shell_pgrp = getpid ();
	setpgid (shell_pgrp, shell_pgrp);

	/* Grab control of the terminal.  */
	tcsetpgrp(shell_terminal, shell_pgrp);
}

ਪੂਰਨ hush_main(ਪੂਰਨ argc, ਅੱਖਰ * ਅਚਲ *argv)
{
	ਪੂਰਨ opt;
	FILE *ਨਿਵੇਸ਼;
	ਅੱਖਰ **e = environ;

	/* XXX what should these be while sourcing /etc/profile? */
	global_argc = argc;
	global_argv = argv;

	/* (re?) initialize globals.  Sometimes hush_main() ends up calling
	 * hush_main(), therefore we cannot rely on the BSS to zero out this
	 * stuff.  Reset these to 0 every time. */
	ifs = ਨਿਰਾ;
	/* map[] is taken care of with call to update_ifs_map() */
	fake_ਬਹੁਲਕ = 0;
	interactive = 0;
	close_me_ਸਿਰ = ਨਿਰਾ;
	ਆਖਰੀ_bg_pid = 0;
	ਕੰਮ_list = ਨਿਰਾ;
	ਆਖਰੀ_jobid = 0;

	/* Initialize some more globals to non-zero values */
	ਸੈੱਟ_cwd();
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_FEATURE_COMMAND_EDITING
	cmdedit_ਸੈੱਟ_initial_prompt();
#ਨਹੀਂਤਾਂ
	PS1 = ਨਿਰਾ;
#ਜੇ_ਅੰਤ
	PS2 = "> ";

	/* initialize our shell local variables with the values
	 * currently living in the environment */
	ਜੇ (e) {
		ਲਈ (; *e; e++)
			ਸੈੱਟ_local_var(*e, 2);   /* without call putenv() */
	}

	ਆਖਰੀ_return_ਕੋਡ=EXIT_SUCCESS;

	ਜੇ (argv[0] && argv[0][0] == '-') {
		debug_printf("\nsourcing /etc/profile\n");
		ਜੇ ((ਨਿਵੇਸ਼ = fopen("/etc/profile", "r")) != ਨਿਰਾ) {
			mark_open(fileno(ਨਿਵੇਸ਼));
			parse_file_outer(ਨਿਵੇਸ਼);
			mark_closed(fileno(ਨਿਵੇਸ਼));
			fclose(ਨਿਵੇਸ਼);
		}
	}
	ਨਿਵੇਸ਼=stdin;

	ਜਦੋਂ ((opt = getopt(argc, argv, "c:xif")) > 0) {
		ਚੋਣ (opt) {
			ਹਾਲਤ 'c':
				{
					global_argv = argv+optind;
					global_argc = argc-optind;
					opt = parse_string_outer(optarg, FLAG_PARSE_SEMICOLON);
					ਜਾਓ final_return;
				}
				ਰੋਕੋ;
			ਹਾਲਤ 'i':
				interactive++;
				ਰੋਕੋ;
			ਹਾਲਤ 'f':
				fake_ਬਹੁਲਕ++;
				ਰੋਕੋ;
			ਮੂਲ:
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ BB_VER
				fprintf(stderr, "Usage: sh [FILE]...\n"
						"   or: sh -c command [args]...\n\n");
				ਬਾਹਰ_ਨਿਕਲੋ(EXIT_FAILURE);
#ਨਹੀਂਤਾਂ
				show_usage();
#ਜੇ_ਅੰਤ
		}
	}
	/* A shell is interactive if the `-i' flag was given, or if all of
	 * the following conditions are met:
	 *	  no -c command
	 *    no arguments remaining or the -s flag given
	 *    standard input is a terminal
	 *    standard output is a terminal
	 *    Refer to Posix.2, the description of the `sh' utility. */
	ਜੇ (argv[optind]==ਨਿਰਾ && ਨਿਵੇਸ਼==stdin &&
			isatty(fileno(stdin)) && isatty(fileno(stdout))) {
		interactive++;
	}

	debug_printf("\ninteractive=%d\n", interactive);
	ਜੇ (interactive) {
		/* Looks like they want an interactive shell */
#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ CONFIG_FEATURE_SH_EXTRA_QUIET
		ਲਿਖੋ( "\n\n" BB_BANNER " hush - the humble shell v0.01 (testing)\n");
		ਲਿਖੋ( "Enter 'help' for a list of built-in commands.\n\n");
#ਜੇ_ਅੰਤ
		ਤਿਆਰੀ_ਕੰਮ_control();
	}

	ਜੇ (argv[optind]==ਨਿਰਾ) {
		opt=parse_file_outer(stdin);
		ਜਾਓ final_return;
	}

	debug_printf("\nrunning script '%s'\n", argv[optind]);
	global_argv = argv+optind;
	global_argc = argc-optind;
	ਨਿਵੇਸ਼ = xfopen(argv[optind], "r");
	opt = parse_file_outer(ਨਿਵੇਸ਼);

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_FEATURE_CLEAN_ਉੱਪਰ
	fclose(ਨਿਵੇਸ਼);
	ਜੇ (cwd && cwd != unknown)
		ਮੁਕਤ((ਅੱਖਰ*)cwd);
	{
		ਬਣਤਰ variables *cur, *tmp;
		ਲਈ(cur = ਸਿਖਰ_vars; cur; cur = tmp) {
			tmp = cur->ਅਗਲਾ;
			ਜੇ (!cur->flg_read_only) {
				ਮੁਕਤ(cur->name);
				ਮੁਕਤ(cur->ਮੁੱਲ);
				ਮੁਕਤ(cur);
			}
		}
	}
#ਜੇ_ਅੰਤ

final_return:
	ਵਾਪਸ(opt?opt:ਆਖਰੀ_return_ਕੋਡ);
}
#ਜੇ_ਅੰਤ

ਸਥਿਰ ਅੱਖਰ *ਪਾਓ_var_ਮੁੱਲ(ਅੱਖਰ *inp)
{
	ਵਾਪਸ ਪਾਓ_var_ਮੁੱਲ_sub(inp, 0);
}

ਸਥਿਰ ਅੱਖਰ *ਪਾਓ_var_ਮੁੱਲ_sub(ਅੱਖਰ *inp, ਪੂਰਨ ਟੈਗ_subst)
{
	ਪੂਰਨ res_str_len = 0;
	ਪੂਰਨ len;
	ਪੂਰਨ ਪੂਰਾ = 0;
	ਅੱਖਰ *p, *p1, *res_str = ਨਿਰਾ;

	ਜਦੋਂ ((p = strchr(inp, SPECIAL_VAR_ਚਿੰਨ੍ਹ))) {
		/* check the beginning of the string for normal characters */
		ਜੇ (p != inp) {
			/* copy any characters to the result string */
			len = p - inp;
			res_str = xrealloc(res_str, (res_str_len + len));
			strncpy((res_str + res_str_len), inp, len);
			res_str_len += len;
		}
		inp = ++p;
		/* find the ending marker */
		p = strchr(inp, SPECIAL_VAR_ਚਿੰਨ੍ਹ);
		*p = '\0';
		/* look up the value to substitute */
		ਜੇ ((p1 = lookup_param(inp))) {
			ਜੇ (ਟੈਗ_subst)
				len = res_str_len + ਲੜੀ_ਲੰਬਾਈ(p1) + 2;
			ਨਹੀਂਤਾਂ
				len = res_str_len + ਲੜੀ_ਲੰਬਾਈ(p1);
			res_str = xrealloc(res_str, (1 + len));
			ਜੇ (ਟੈਗ_subst) {
				/*
				 * copy the variable value to the result
				 * string
				 */
				strcpy((res_str + res_str_len + 1), p1);

				/*
				 * mark the replaced text to be accepted as
				 * is
				 */
				res_str[res_str_len] = SUBSTED_VAR_ਚਿੰਨ੍ਹ;
				res_str[res_str_len + 1 + ਲੜੀ_ਲੰਬਾਈ(p1)] =
					SUBSTED_VAR_ਚਿੰਨ੍ਹ;
			} ਨਹੀਂਤਾਂ
				/*
				 * copy the variable value to the result
				 * string
				 */
				strcpy((res_str + res_str_len), p1);

			res_str_len = len;
		}
		*p = SPECIAL_VAR_ਚਿੰਨ੍ਹ;
		inp = ++p;
		ਪੂਰਾ = 1;
	}
	ਜੇ (ਪੂਰਾ) {
		res_str = xrealloc(res_str, (1 + res_str_len + ਲੜੀ_ਲੰਬਾਈ(inp)));
		strcpy((res_str + res_str_len), inp);
		ਜਦੋਂ ((p = strchr(res_str, '\n'))) {
			*p = ' ';
		}
	}
	ਵਾਪਸ (res_str == ਨਿਰਾ) ? inp : res_str;
}

ਸਥਿਰ ਅੱਖਰ **make_list_ਵਿੱਚ(ਅੱਖਰ **inp, ਅੱਖਰ *name)
{
	ਪੂਰਨ len, i;
	ਪੂਰਨ name_len = ਲੜੀ_ਲੰਬਾਈ(name);
	ਪੂਰਨ n = 0;
	ਅੱਖਰ **list;
	ਅੱਖਰ *p1, *p2, *p3;

	/* create list of variable values */
	list = xmalloc(ਆਕਾਰ(*list));
	ਲਈ (i = 0; inp[i]; i++) {
		p3 = ਪਾਓ_var_ਮੁੱਲ(inp[i]);
		p1 = p3;
		ਜਦੋਂ (*p1) {
			ਜੇ (*p1 == ' ') {
				p1++;
				ਜਾਰੀ;
			}
			ਜੇ ((p2 = strchr(p1, ' '))) {
				len = p2 - p1;
			} ਨਹੀਂਤਾਂ {
				len = ਲੜੀ_ਲੰਬਾਈ(p1);
				p2 = p1 + len;
			}
			/* we use n + 2 in realloc for list,because we add
			 * new element and then we will add NULL element */
			list = xrealloc(list, ਆਕਾਰ(*list) * (n + 2));
			list[n] = xmalloc(2 + name_len + len);
			strcpy(list[n], name);
			strcat(list[n], "=");
			strncat(list[n], p1, len);
			list[n++][name_len + len + 1] = '\0';
			p1 = p2;
		}
		ਜੇ (p3 != inp[i]) ਮੁਕਤ(p3);
	}
	list[n] = ਨਿਰਾ;
	ਵਾਪਸ list;
}

/*
 * Make new string for parser
 * inp     - array of argument strings to flatten
 * nonnull - indicates argument was quoted when originally parsed
 */
ਸਥਿਰ ਅੱਖਰ *make_string(ਅੱਖਰ **inp, ਪੂਰਨ *nonnull)
{
	ਅੱਖਰ *p;
	ਅੱਖਰ *str = ਨਿਰਾ;
	ਪੂਰਨ n;
	ਪੂਰਨ len = 2;
	ਅੱਖਰ *noeval_str;
	ਪੂਰਨ noeval = 0;

	noeval_str = get_local_var("HUSH_NO_EVAL");
	ਜੇ (noeval_str != ਨਿਰਾ && *noeval_str != '0' && *noeval_str != '\0')
		noeval = 1;
	ਲਈ (n = 0; inp[n]; n++) {
		p = ਪਾਓ_var_ਮੁੱਲ_sub(inp[n], noeval);
		ਅੱਖਰ *new_str = xrealloc(str, (len + ਲੜੀ_ਲੰਬਾਈ(p) + (2 * nonnull[n])));
		ਜੇ (!new_str) {
			ਮੁਕਤ(str);
			ਜੇ (p != inp[n]) ਮੁਕਤ(p);
			ਵਾਪਸ ਨਿਰਾ;
		}
		str = new_str;
		ਜੇ (n) {
			strcat(str, " ");
		} ਨਹੀਂਤਾਂ {
			*str = '\0';
		}
		ਜੇ (nonnull[n])
			strcat(str, "'");
		strcat(str, p);
		ਜੇ (nonnull[n])
			strcat(str, "'");
		len = ਲੜੀ_ਲੰਬਾਈ(str) + 3;
		ਜੇ (p != inp[n]) ਮੁਕਤ(p);
	}
	len = ਲੜੀ_ਲੰਬਾਈ(str);
	*(str + len) = '\n';
	*(str + len + 1) = '\0';
	ਵਾਪਸ str;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ __U_BOOT__
ਸਥਿਰ ਪੂਰਨ do_showvar(ਬਣਤਰ cmd_tbl *cmdtp, ਪੂਰਨ flag, ਪੂਰਨ argc,
		      ਅੱਖਰ *ਅਚਲ argv[])
{
	ਪੂਰਨ i, k;
	ਪੂਰਨ rcode = 0;
	ਬਣਤਰ variables *cur;

	ਜੇ (argc == 1) {		/* Print all env variables	*/
		ਲਈ (cur = ਸਿਖਰ_vars; cur; cur = cur->ਅਗਲਾ) {
			ਲਿਖੋ ("%s=%s\n", cur->name, cur->ਮੁੱਲ);
			ਜੇ (ctrlc ()) {
				puts ("\n ** Abort\n");
				ਵਾਪਸ 1;
			}
		}
		ਵਾਪਸ 0;
	}
	ਲਈ (i = 1; i < argc; ++i) {	/* print single env variables	*/
		ਅੱਖਰ *name = argv[i];

		k = -1;
		ਲਈ (cur = ਸਿਖਰ_vars; cur; cur = cur->ਅਗਲਾ) {
			ਜੇ(strcmp (cur->name, name) == 0) {
				k = 0;
				ਲਿਖੋ ("%s=%s\n", cur->name, cur->ਮੁੱਲ);
			}
			ਜੇ (ctrlc ()) {
				puts ("\n ** Abort\n");
				ਵਾਪਸ 1;
			}
		}
		ਜੇ (k < 0) {
			ਲਿਖੋ ("## Error: \"%s\" not defined\n", name);
			rcode ++;
		}
	}
	ਵਾਪਸ rcode;
}

U_BOOT_CMD(
	showvar, CONFIG_SYS_MAXARGS, 1,	do_showvar,
	"print local hushshell variables",
	"\n    - print values of all hushshell variables\n"
	"showvar name ...\n"
	"    - print value of hushshell variable 'name'"
);

#ਜੇ_ਅੰਤ
/****************************************************************************/
