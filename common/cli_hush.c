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

#ವ್ಯಾಖ್ಯಾನಿಸು __U_BOOT__
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __U_BOOT__
#ಅನ್ತರ್ಭಾವ <env.h>
#ಅನ್ತರ್ಭಾವ <malloc.h>         /* malloc, free, realloc*/
#ಅನ್ತರ್ಭಾವ <linux/ctype.h>    /* isalpha, isdigit */
#ಅನ್ತರ್ಭಾವ <console.h>
#ಅನ್ತರ್ಭಾವ <bootretry.h>
#ಅನ್ತರ್ಭಾವ <cli.h>
#ಅನ್ತರ್ಭಾವ <cli_hush.h>
#ಅನ್ತರ್ಭಾವ <command.h>        /* find_cmd */
#ಅನ್ತರ್ಭಾವ <vsprintf.h>
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
#ಅನ್ತರ್ಭಾವ <ctype.h>     /* isalpha, isdigit */
#ಅನ್ತರ್ಭಾವ <unistd.h>    /* getpid */
#ಅನ್ತರ್ಭಾವ <stdlib.h>    /* getenv, atoi */
#ಅನ್ತರ್ಭಾವ <string.h>    /* strchr */
#ಅನ್ತರ್ಭಾವ <stdio.h>     /* popen etc. */
#ಅನ್ತರ್ಭಾವ <glob.h>      /* glob, of course */
#ಅನ್ತರ್ಭಾವ <stdarg.h>    /* va_list */
#ಅನ್ತರ್ಭಾವ <errno.h>
#ಅನ್ತರ್ಭಾವ <fcntl.h>
#ಅನ್ತರ್ಭಾವ <getopt.h>    /* should be pretty obvious */

#ಅನ್ತರ್ಭಾವ <sys/stat.h>  /* ulimit */
#ಅನ್ತರ್ಭಾವ <sys/types.h>
#ಅನ್ತರ್ಭಾವ <sys/wait.h>
#ಅನ್ತರ್ಭಾವ <signal.h>

/* #include <dmalloc.h> */

#ಆದರೆ 1
#ಅನ್ತರ್ಭಾವ "busybox.h"
#ಅನ್ತರ್ಭಾವ "cmdedit.h"
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು applet_name "hush"
#ಅನ್ತರ್ಭಾವ "standalone.h"
#ವ್ಯಾಖ್ಯಾನಿಸು hush_main ಮುಖ್ಯ
#ವ್ಯಾಖ್ಯಾನಿಸು BB_BANNER
#ಆದರೆ_ಅಂತ್ಯ
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿಸು SPECIAL_VAR_ಚಿಹ್ನೆ 03
#ವ್ಯಾಖ್ಯಾನಿಸು SUBSTED_VAR_ಚಿಹ್ನೆ 04
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
#ವ್ಯಾಖ್ಯಾನಿಸು FLAG_EXIT_FROM_LOOP 1
#ವ್ಯಾಖ್ಯಾನಿಸು FLAG_PARSE_SEMICOLON (1 << 1)		/* symbol ';' is special for parser */
#ವ್ಯಾಖ್ಯಾನಿಸು FLAG_REPARSING       (1 << 2)		/* >= 2nd pass */

#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __U_BOOT__
#ವ್ಯಾಖ್ಯಾನಿಸು EXIT_SUCCESS 0
#ವ್ಯಾಖ್ಯಾನಿಸು EOF -1
#ವ್ಯಾಖ್ಯಾನಿಸು syntax() syntax_err()
#ವ್ಯಾಖ್ಯಾನಿಸು xstrdup strdup
#ವ್ಯಾಖ್ಯಾನಿಸು error_ಸಂದೇಶ ಬರೆ
#ಇಲ್ಲದಿದ್ದರೆ
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಎಣಿಕೆ {
	REDIRECT_ಒಳಹರಿವು     = 1,
	REDIRECT_OVERWRITE = 2,
	REDIRECT_APPEND    = 3,
	REDIRECT_HEREIS    = 4,
	REDIRECT_IO        = 5
} redir_type;

/* The descrip member of this structure is only used to make debugging
 * output pretty */
ರಚನೆ {ಪೂರ್ಣಾಂಕ ಬಹುಲಕ; ಪೂರ್ಣಾಂಕ default_fd; ಅಕ್ಷರ *descrip;} redir_ಕೋಷ್ಟಕ[] = {
	{ 0,                         0, "()" },
	{ O_RDONLY,                  0, "<"  },
	{ O_CREAT|O_TRUNC|O_WRONLY,  1, ">"  },
	{ O_CREAT|O_APPEND|O_WRONLY, 1, ">>" },
	{ O_RDONLY,                 -1, "<<" },
	{ O_RDWR,                    1, "<>" }
};
#ಆದರೆ_ಅಂತ್ಯ

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಎಣಿಕೆ {
	PIPE_SEQ = 1,
	PIPE_AND = 2,
	PIPE_OR  = 3,
	PIPE_BG  = 4,
} pipe_style;

/* might eventually control execution */
ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಎಣಿಕೆ {
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
	RES_ಮುಗಿದ  = 10,
	RES_XXXX  = 11,
	RES_ಒಳ    = 12,
	RES_SNTX  = 13
} reserved_style;
#ವ್ಯಾಖ್ಯಾನಿಸು FLAG_ಕೊನೆ   (1<<RES_NONE)
#ವ್ಯಾಖ್ಯಾನಿಸು FLAG_IF    (1<<RES_IF)
#ವ್ಯಾಖ್ಯಾನಿಸು FLAG_THEN  (1<<RES_THEN)
#ವ್ಯಾಖ್ಯಾನಿಸು FLAG_ELIF  (1<<RES_ELIF)
#ವ್ಯಾಖ್ಯಾನಿಸು FLAG_ELSE  (1<<RES_ELSE)
#ವ್ಯಾಖ್ಯಾನಿಸು FLAG_FI    (1<<RES_FI)
#ವ್ಯಾಖ್ಯಾನಿಸು FLAG_FOR   (1<<RES_FOR)
#ವ್ಯಾಖ್ಯಾನಿಸು FLAG_WHILE (1<<RES_WHILE)
#ವ್ಯಾಖ್ಯಾನಿಸು FLAG_UNTIL (1<<RES_UNTIL)
#ವ್ಯಾಖ್ಯಾನಿಸು FLAG_DO    (1<<RES_DO)
#ವ್ಯಾಖ್ಯಾನಿಸು FLAG_ಮುಗಿದ  (1<<RES_ಮುಗಿದ)
#ವ್ಯಾಖ್ಯಾನಿಸು FLAG_ಒಳ    (1<<RES_ಒಳ)
#ವ್ಯಾಖ್ಯಾನಿಸು FLAG_ಆರಂಭ (1<<RES_XXXX)

/* This holds pointers to the various results of parsing */
ರಚನೆ p_context {
	ರಚನೆ ಮಗು_prog *ಮಗು;
	ರಚನೆ pipe *list_ತಲೆ;
	ರಚನೆ pipe *pipe;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	ರಚನೆ redir_struct *pending_redirect;
#ಆದರೆ_ಅಂತ್ಯ
	reserved_style w;
	ಪೂರ್ಣಾಂಕ old_flag;				/* for figuring out valid reserved words */
	ರಚನೆ p_context *ರಾಶಿ;
	ಪೂರ್ಣಾಂಕ type;			/* define type of parser : ";$" common or special symbol */
	/* How about quoting status? */
};

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
ರಚನೆ redir_struct {
	redir_type type;			/* type of redirection */
	ಪೂರ್ಣಾಂಕ fd;						/* file descriptor being redirected */
	ಪೂರ್ಣಾಂಕ dup;					/* -1, or file descriptor being duplicated */
	ರಚನೆ redir_struct *ಮುಂದಿನ;	/* pointer to the next redirect in the list */
	glob_t ಶಬ್ದ;				/* *word.gl_pathv is the filename */
};
#ಆದರೆ_ಅಂತ್ಯ

ರಚನೆ ಮಗು_prog {
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	pid_t pid;					/* 0 if exited */
#ಆದರೆ_ಅಂತ್ಯ
	ಅಕ್ಷರ **argv;				/* program name and arguments */
	/* was quoted when parsed; copy of struct o_string.nonnull field */
	ಪೂರ್ಣಾಂಕ *argv_nonnull;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __U_BOOT__
	ಪೂರ್ಣಾಂಕ    argc;                            /* number of program arguments */
#ಆದರೆ_ಅಂತ್ಯ
	ರಚನೆ pipe *ಗುಂಪು;			/* if non-NULL, first in group or subshell */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	ಪೂರ್ಣಾಂಕ subshell;				/* flag, non-zero if group must be forked */
	ರಚನೆ redir_struct *redirects;	/* I/O redirections */
	glob_t glob_ಫಲಿತಾಂಶ;			/* result of parameter globbing */
	ಪೂರ್ಣಾಂಕ is_stopped;				/* is the program currently running? */
	ರಚನೆ pipe *family;		/* pointer back to the child's parent pipe */
#ಆದರೆ_ಅಂತ್ಯ
	ಪೂರ್ಣಾಂಕ sp;				/* number of SPECIAL_VAR_SYMBOL */
	ಪೂರ್ಣಾಂಕ type;
};

ರಚನೆ pipe {
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	ಪೂರ್ಣಾಂಕ jobid;					/* job number */
#ಆದರೆ_ಅಂತ್ಯ
	ಪೂರ್ಣಾಂಕ ಅಂಕೆ_progs;				/* total number of programs in job */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	ಪೂರ್ಣಾಂಕ running_progs;			/* number of programs running */
	ಅಕ್ಷರ *ಪಠ್ಯ;					/* name of job */
	ಅಕ್ಷರ *cmdbuf;				/* buffer various argv's point into */
	pid_t pgrp;					/* process group ID for the job */
#ಆದರೆ_ಅಂತ್ಯ
	ರಚನೆ ಮಗು_prog *progs;	/* array of commands in pipe */
	ರಚನೆ pipe *ಮುಂದಿನ;			/* to track background commands */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	ಪೂರ್ಣಾಂಕ stopped_progs;			/* number of programs alive, but stopped */
	ಪೂರ್ಣಾಂಕ ಕೆಲಸ_context;			/* bitmask defining current context */
#ಆದರೆ_ಅಂತ್ಯ
	pipe_style followup;		/* PIPE_BG, PIPE_SEQ, PIPE_OR, PIPE_AND */
	reserved_style r_ಬಹುಲಕ;		/* supports if, for, while, until */
};

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
ರಚನೆ close_me {
	ಪೂರ್ಣಾಂಕ fd;
	ರಚನೆ close_me *ಮುಂದಿನ;
};
#ಆದರೆ_ಅಂತ್ಯ

ರಚನೆ variables {
	ಅಕ್ಷರ *name;
	ಅಕ್ಷರ *ಮೌಲ್ಯ;
	ಪೂರ್ಣಾಂಕ flg_export;
	ಪೂರ್ಣಾಂಕ flg_read_only;
	ರಚನೆ variables *ಮುಂದಿನ;
};

/* globals, connect us to the outside world
 * the first three support $?, $#, and $1 */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
ಅಕ್ಷರ **global_argv;
ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ global_argc;
#ಆದರೆ_ಅಂತ್ಯ
ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಕೊನೆಯ_return_ಸಂಕೇತ;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
ಬಾಹ್ಯ ಅಕ್ಷರ **environ; /* This is in <unistd.h>, but protected with __USE_GNU */
#ಆದರೆ_ಅಂತ್ಯ

/* "globals" within this file */
ಸ್ಥಿರ uchar *ifs;
ಸ್ಥಿರ ಅಕ್ಷರ map[256];
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ fake_ಬಹುಲಕ;
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ interactive;
ಸ್ಥಿರ ರಚನೆ close_me *close_me_ತಲೆ;
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *cwd;
ಸ್ಥಿರ ರಚನೆ pipe *ಕೆಲಸ_list;
ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಕೊನೆಯ_bg_pid;
ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ಕೊನೆಯ_jobid;
ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ shell_terminal;
ಸ್ಥಿರ ಅಕ್ಷರ *PS1;
ಸ್ಥಿರ ಅಕ್ಷರ *PS2;
ರಚನೆ variables shell_ver = { "HUSH_VERSION", "0.01", 1, 1, 0 };
ರಚನೆ variables *ತುದಿ_vars = &shell_ver;
#ಇಲ್ಲದಿದ್ದರೆ
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ flag_repeat = 0;
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ do_repeat = 0;
ಸ್ಥಿರ ರಚನೆ variables *ತುದಿ_vars = ಖಾಲಿ ;
#ಆದರೆ_ಅಂತ್ಯ /*__U_BOOT__ */

#ವ್ಯಾಖ್ಯಾನಿಸು B_CHUNK (100)
#ವ್ಯಾಖ್ಯಾನಿಸು B_NOSPAC 1

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ರಚನೆ {
	ಅಕ್ಷರ *ದತ್ತಾಂಶ;
	ಪೂರ್ಣಾಂಕ ಉದ್ದಳತೆ;
	ಪೂರ್ಣಾಂಕ maxlen;
	ಪೂರ್ಣಾಂಕ quote;
	ಪೂರ್ಣಾಂಕ nonnull;
} o_string;
#ವ್ಯಾಖ್ಯಾನಿಸು NULL_O_STRING {ಖಾಲಿ,0,0,0,0}
/* used for initialization:
	o_string foo = NULL_O_STRING; */

/* I can almost use ordinary FILE *.  Is open_memstream() universally
 * available?  Where is it documented? */
ರಚನೆ ಒಳ_str {
	ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *p;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	ಅಕ್ಷರ peek_buf[2];
#ಆದರೆ_ಅಂತ್ಯ
	ಪೂರ್ಣಾಂಕ __promptme;
	ಪೂರ್ಣಾಂಕ promptmode;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	FILE *file;
#ಆದರೆ_ಅಂತ್ಯ
	ಪೂರ್ಣಾಂಕ (*get) (ರಚನೆ ಒಳ_str *);
	ಪೂರ್ಣಾಂಕ (*peek) (ರಚನೆ ಒಳ_str *);
};
#ವ್ಯಾಖ್ಯಾನಿಸು b_getch(ಒಳಹರಿವು) ((ಒಳಹರಿವು)->get(ಒಳಹರಿವು))
#ವ್ಯಾಖ್ಯಾನಿಸು b_peek(ಒಳಹರಿವು) ((ಒಳಹರಿವು)->peek(ಒಳಹರಿವು))

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
#ವ್ಯಾಖ್ಯಾನಿಸು ಕೆಲಸ_ಸ್ಥಿತಿ_FORMAT "[%d] %-22s %.40s\n"

ರಚನೆ built_ಒಳ_command {
	ಅಕ್ಷರ *cmd;					/* name */
	ಅಕ್ಷರ *descr;				/* description */
	ಪೂರ್ಣಾಂಕ (*function) (ರಚನೆ ಮಗು_prog *);	/* function ptr */
};
#ಆದರೆ_ಅಂತ್ಯ

/* define DEBUG_SHELL for debugging output (obviously ;-)) */
#ಆದರೆ 0
#ವ್ಯಾಖ್ಯಾನಿಸು DEBUG_SHELL
#ಆದರೆ_ಅಂತ್ಯ

/* This should be in utility.c */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ DEBUG_SHELL
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
ಸ್ಥಿರ ಶೂನ್ಯ debug_printf(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *format, ...)
{
	va_list ವಾದಗಳು;
	va_ಆರಂಭ(ವಾದಗಳು, format);
	vfprintf(stderr, format, ವಾದಗಳು);
	va_ಕೊನೆ(ವಾದಗಳು);
}
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು debug_printf(fmt,ವಾದಗಳು...)	ಬರೆ (fmt ,##args)
#ಆದರೆ_ಅಂತ್ಯ
#ಇಲ್ಲದಿದ್ದರೆ
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ debug_printf(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *format, ...) { }
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿಸು final_printf debug_printf

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __U_BOOT__
ಸ್ಥಿರ ಶೂನ್ಯ syntax_err(ಶೂನ್ಯ) {
	 ಬರೆ("syntax error\n");
}
#ಇಲ್ಲದಿದ್ದರೆ
ಸ್ಥಿರ ಶೂನ್ಯ __syntax(ಅಕ್ಷರ *file, ಪೂರ್ಣಾಂಕ ಸಾಲು) {
	error_ಸಂದೇಶ("syntax error %s:%d", file, ಸಾಲು);
}
#ವ್ಯಾಖ್ಯಾನಿಸು syntax() __syntax(__FILE__, __LINE__)
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __U_BOOT__
ಸ್ಥಿರ ಶೂನ್ಯ *xmalloc(size_t ಗಾತ್ರ_2);
ಸ್ಥಿರ ಶೂನ್ಯ *xrealloc(ಶೂನ್ಯ *ptr, size_t ಗಾತ್ರ_2);
#ಇಲ್ಲದಿದ್ದರೆ
/* Index of subroutines: */
/*   function prototypes for builtins */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ builtin_cd(ರಚನೆ ಮಗು_prog *ಮಗು);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ builtin_env(ರಚನೆ ಮಗು_prog *ಮಗು);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ builtin_eval(ರಚನೆ ಮಗು_prog *ಮಗು);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ builtin_exec(ರಚನೆ ಮಗು_prog *ಮಗು);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ builtin_exit(ರಚನೆ ಮಗು_prog *ಮಗು);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ builtin_export(ರಚನೆ ಮಗು_prog *ಮಗು);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ builtin_fg_bg(ರಚನೆ ಮಗು_prog *ಮಗು);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ builtin_help(ರಚನೆ ಮಗು_prog *ಮಗು);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ builtin_jobs(ರಚನೆ ಮಗು_prog *ಮಗು);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ builtin_pwd(ರಚನೆ ಮಗು_prog *ಮಗು);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ builtin_read(ರಚನೆ ಮಗು_prog *ಮಗು);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ builtin_ಗಣ(ರಚನೆ ಮಗು_prog *ಮಗು);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ builtin_ಸರಿಸು(ರಚನೆ ಮಗು_prog *ಮಗು);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ builtin_ಮೂಲ(ರಚನೆ ಮಗು_prog *ಮಗು);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ builtin_umask(ರಚನೆ ಮಗು_prog *ಮಗು);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ builtin_unset(ರಚನೆ ಮಗು_prog *ಮಗು);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ builtin_not_written(ರಚನೆ ಮಗು_prog *ಮಗು);
#ಆದರೆ_ಅಂತ್ಯ
/*   o_string manipulation: */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ b_check_space(o_string *o, ಪೂರ್ಣಾಂಕ len);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ b_addchr(o_string *o, ಪೂರ್ಣಾಂಕ ch);
ಸ್ಥಿರ ಶೂನ್ಯ b_reset(o_string *o);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ b_addqchr(o_string *o, ಪೂರ್ಣಾಂಕ ch, ಪೂರ್ಣಾಂಕ quote);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ b_adduint(o_string *o, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i);
#ಆದರೆ_ಅಂತ್ಯ
/*  in_str manipulations: */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ static_get(ರಚನೆ ಒಳ_str *i);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ static_peek(ರಚನೆ ಒಳ_str *i);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ file_get(ರಚನೆ ಒಳ_str *i);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ file_peek(ರಚನೆ ಒಳ_str *i);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
ಸ್ಥಿರ ಶೂನ್ಯ ಸಿದ್ಧತೆ_file_ಒಳ_str(ರಚನೆ ಒಳ_str *i, FILE *f);
#ಇಲ್ಲದಿದ್ದರೆ
ಸ್ಥಿರ ಶೂನ್ಯ ಸಿದ್ಧತೆ_file_ಒಳ_str(ರಚನೆ ಒಳ_str *i);
#ಆದರೆ_ಅಂತ್ಯ
ಸ್ಥಿರ ಶೂನ್ಯ ಸಿದ್ಧತೆ_string_ಒಳ_str(ರಚನೆ ಒಳ_str *i, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
/*  close_me manipulations: */
ಸ್ಥಿರ ಶೂನ್ಯ mark_open(ಪೂರ್ಣಾಂಕ fd);
ಸ್ಥಿರ ಶೂನ್ಯ mark_closed(ಪೂರ್ಣಾಂಕ fd);
ಸ್ಥಿರ ಶೂನ್ಯ close_all(ಶೂನ್ಯ);
#ಆದರೆ_ಅಂತ್ಯ
/*  "run" the final data structures: */
ಸ್ಥಿರ ಅಕ್ಷರ *indenter(ಪೂರ್ಣಾಂಕ i);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ free_pipe_list(ರಚನೆ pipe *ತಲೆ, ಪೂರ್ಣಾಂಕ indent);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ free_pipe(ರಚನೆ pipe *pi, ಪೂರ್ಣಾಂಕ indent);
/*  really run the final data structures: */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಸಿದ್ಧತೆ_redirects(ರಚನೆ ಮಗು_prog *prog, ಪೂರ್ಣಾಂಕ squirrel[]);
#ಆದರೆ_ಅಂತ್ಯ
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಸತತ_list_real(ರಚನೆ pipe *pi);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
ಸ್ಥಿರ ಶೂನ್ಯ pseudo_exec(ರಚನೆ ಮಗು_prog *ಮಗು) __attribute__ ((noreturn));
#ಆದರೆ_ಅಂತ್ಯ
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಸತತ_pipe_real(ರಚನೆ pipe *pi);
/*   extended glob support: */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ globhack(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *src, ಪೂರ್ಣಾಂಕ flags, glob_t *pglob);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ glob_needed(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ xglob(o_string *dest, ಪೂರ್ಣಾಂಕ flags, glob_t *pglob);
#ಆದರೆ_ಅಂತ್ಯ
/*   variable assignment: */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ is_assignment(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s);
/*   data structure manipulation: */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಸಿದ್ಧತೆ_redirect(ರಚನೆ p_context *ctx, ಪೂರ್ಣಾಂಕ fd, redir_type style, ರಚನೆ ಒಳ_str *ಒಳಹರಿವು);
#ಆದರೆ_ಅಂತ್ಯ
ಸ್ಥಿರ ಶೂನ್ಯ initialize_context(ರಚನೆ p_context *ctx);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಮುಗಿದ_ಶಬ್ದ(o_string *dest, ರಚನೆ p_context *ctx);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಮುಗಿದ_command(ರಚನೆ p_context *ctx);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಮುಗಿದ_pipe(ರಚನೆ p_context *ctx, pipe_style type);
/*   primary string parsing: */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ redirect_dup_ಅಂಕೆ(ರಚನೆ ಒಳ_str *ಒಳಹರಿವು);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ redirect_opt_ಅಂಕೆ(o_string *o);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ process_command_subs(o_string *dest, ರಚನೆ p_context *ctx, ರಚನೆ ಒಳ_str *ಒಳಹರಿವು, ಪೂರ್ಣಾಂಕ subst_ಕೊನೆ);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ parse_ಗುಂಪು(o_string *dest, ರಚನೆ p_context *ctx, ರಚನೆ ಒಳ_str *ಒಳಹರಿವು, ಪೂರ್ಣಾಂಕ ch);
#ಆದರೆ_ಅಂತ್ಯ
ಸ್ಥಿರ ಅಕ್ಷರ *lookup_param(ಅಕ್ಷರ *src);
ಸ್ಥಿರ ಅಕ್ಷರ *make_string(ಅಕ್ಷರ **inp, ಪೂರ್ಣಾಂಕ *nonnull);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ handle_dollar(o_string *dest, ರಚನೆ p_context *ctx, ರಚನೆ ಒಳ_str *ಒಳಹರಿವು);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ parse_string(o_string *dest, ರಚನೆ p_context *ctx, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *src);
#ಆದರೆ_ಅಂತ್ಯ
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ parse_ಹರಿವು(o_string *dest, ರಚನೆ p_context *ctx, ರಚನೆ ಒಳ_str *input0, ಪೂರ್ಣಾಂಕ ಕೊನೆ_trigger);
/*   setup: */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ parse_ಹರಿವು_outer(ರಚನೆ ಒಳ_str *inp, ಪೂರ್ಣಾಂಕ flag);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ parse_string_outer(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s, ಪೂರ್ಣಾಂಕ flag);
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ parse_file_outer(FILE *f);
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
/*   job management: */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ checkjobs(ರಚನೆ pipe* fg_pipe);
ಸ್ಥಿರ ಶೂನ್ಯ ಸೇರಿಸು_bg_ಕೆಲಸ(ರಚನೆ pipe *pi);
ಸ್ಥಿರ ಶೂನ್ಯ remove_bg_ಕೆಲಸ(ರಚನೆ pipe *pi);
#ಆದರೆ_ಅಂತ್ಯ
/*     local variable support */
ಸ್ಥಿರ ಅಕ್ಷರ **make_list_ಒಳ(ಅಕ್ಷರ **inp, ಅಕ್ಷರ *name);
ಸ್ಥಿರ ಅಕ್ಷರ *ಸೇರಿಸು_var_ಮೌಲ್ಯ(ಅಕ್ಷರ *inp);
ಸ್ಥಿರ ಅಕ್ಷರ *ಸೇರಿಸು_var_ಮೌಲ್ಯ_sub(ಅಕ್ಷರ *inp, ಪೂರ್ಣಾಂಕ ಟ್ಯಾಗ್_subst);

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
/* Table of built-in functions.  They can be forked or not, depending on
 * context: within pipes, they fork.  As simple commands, they do not.
 * When used in non-forking context, they can change global variables
 * in the parent shell process.  If forked, of course they can not.
 * For example, 'unset foo | whatever' will parse and run, but foo will
 * still be set at the end. */
ಸ್ಥಿರ ರಚನೆ built_ಒಳ_command bltins[] = {
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
	{"set", "Set/unset shell local variables", builtin_ಗಣ},
	{"shift", "Shift positional parameters", builtin_ಸರಿಸು},
	{"trap", "Trap signals", builtin_not_written},
	{"ulimit","Controls resource limits", builtin_not_written},
	{"umask","Sets file creation mask", builtin_umask},
	{"unset", "Unset environment variable", builtin_unset},
	{".", "Source-in and run commands in a file", builtin_ಮೂಲ},
	{"help", "List shell built-in commands", builtin_help},
	{ಖಾಲಿ, ಖಾಲಿ, ಖಾಲಿ}
};

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಗಣ_cwd(ಶೂನ್ಯ)
{
	ಆದರೆ(cwd==unknown)
		cwd = ಖಾಲಿ;     /* xgetcwd(arg) called free(arg) */
	cwd = xgetcwd((ಅಕ್ಷರ *)cwd);
	ಆದರೆ (!cwd)
		cwd = unknown;
	ಹಿಂತಿರುಗಿ cwd;
}

/* built-in 'eval' handler */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ builtin_eval(ರಚನೆ ಮಗು_prog *ಮಗು)
{
	ಅಕ್ಷರ *str = ಖಾಲಿ;
	ಪೂರ್ಣಾಂಕ rcode = EXIT_SUCCESS;

	ಆದರೆ (ಮಗು->argv[1]) {
		str = make_string(ಮಗು->argv + 1);
		parse_string_outer(str, FLAG_EXIT_FROM_LOOP |
					FLAG_PARSE_SEMICOLON);
		ಬಿಡುಗಡೆ(str);
		rcode = ಕೊನೆಯ_return_ಸಂಕೇತ;
	}
	ಹಿಂತಿರುಗಿ rcode;
}

/* built-in 'cd <path>' handler */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ builtin_cd(ರಚನೆ ಮಗು_prog *ಮಗು)
{
	ಅಕ್ಷರ *newdir;
	ಆದರೆ (ಮಗು->argv[1] == ಖಾಲಿ)
		newdir = env_get("HOME");
	ಇಲ್ಲದಿದ್ದರೆ
		newdir = ಮಗು->argv[1];
	ಆದರೆ (chdir(newdir)) {
		ಬರೆ("cd: %s: %s\n", newdir, strerror(errno));
		ಹಿಂತಿರುಗಿ EXIT_FAILURE;
	}
	ಗಣ_cwd();
	ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
}

/* built-in 'env' handler */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ builtin_env(ರಚನೆ ಮಗು_prog *dummy)
{
	ಅಕ್ಷರ **e = environ;
	ಆದರೆ (e == ಖಾಲಿ) ಹಿಂತಿರುಗಿ EXIT_FAILURE;
	ಗಾಗಿ (; *e; e++) {
		puts(*e);
	}
	ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
}

/* built-in 'exec' handler */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ builtin_exec(ರಚನೆ ಮಗು_prog *ಮಗು)
{
	ಆದರೆ (ಮಗು->argv[1] == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ EXIT_SUCCESS;   /* Really? */
	ಮಗು->argv++;
	pseudo_exec(ಮಗು);
	/* never returns */
}

/* built-in 'exit' handler */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ builtin_exit(ರಚನೆ ಮಗು_prog *ಮಗು)
{
	ಆದರೆ (ಮಗು->argv[1] == ಖಾಲಿ)
		ನಿರ್ಗಮಿಸು(ಕೊನೆಯ_return_ಸಂಕೇತ);
	ನಿರ್ಗಮಿಸು (atoi(ಮಗು->argv[1]));
}

/* built-in 'export VAR=value' handler */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ builtin_export(ರಚನೆ ಮಗು_prog *ಮಗು)
{
	ಪೂರ್ಣಾಂಕ res = 0;
	ಅಕ್ಷರ *name = ಮಗು->argv[1];

	ಆದರೆ (name == ಖಾಲಿ) {
		ಹಿಂತಿರುಗಿ (builtin_env(ಮಗು));
	}

	name = strdup(name);

	ಆದರೆ(name) {
		ಅಕ್ಷರ *ಮೌಲ್ಯ = strchr(name, '=');

		ಆದರೆ (!ಮೌಲ್ಯ) {
			ಅಕ್ಷರ *tmp;
			/* They are exporting something without an =VALUE */

			ಮೌಲ್ಯ = get_local_var(name);
			ಆದರೆ (ಮೌಲ್ಯ) {
				size_t ln = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(name);

				tmp = realloc(name, ln+ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ಮೌಲ್ಯ)+2);
				ಆದರೆ(tmp==ಖಾಲಿ)
					res = -1;
				ಇಲ್ಲದಿದ್ದರೆ {
					sprintf(tmp+ln, "=%s", ಮೌಲ್ಯ);
					name = tmp;
				}
			} ಇಲ್ಲದಿದ್ದರೆ {
				/* bash does not return an error when trying to export
				 * an undefined variable.  Do likewise. */
				res = 1;
			}
		}
	}
	ಆದರೆ (res<0)
		perror_ಸಂದೇಶ("export");
	ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ(res==0)
		res = ಗಣ_local_var(name, 1);
	ಇಲ್ಲದಿದ್ದರೆ
		res = 0;
	ಬಿಡುಗಡೆ(name);
	ಹಿಂತಿರುಗಿ res;
}

/* built-in 'fg' and 'bg' handler */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ builtin_fg_bg(ರಚನೆ ಮಗು_prog *ಮಗು)
{
	ಪೂರ್ಣಾಂಕ i, jobnum;
	ರಚನೆ pipe *pi=ಖಾಲಿ;

	ಆದರೆ (!interactive)
		ಹಿಂತಿರುಗಿ EXIT_FAILURE;
	/* If they gave us no args, assume they want the last backgrounded task */
	ಆದರೆ (!ಮಗು->argv[1]) {
		ಗಾಗಿ (pi = ಕೆಲಸ_list; pi; pi = pi->ಮುಂದಿನ) {
			ಆದರೆ (pi->jobid == ಕೊನೆಯ_jobid) {
				ನಿಲ್ಲಿಸು;
			}
		}
		ಆದರೆ (!pi) {
			error_ಸಂದೇಶ("%s: no current job", ಮಗು->argv[0]);
			ಹಿಂತಿರುಗಿ EXIT_FAILURE;
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಓದು(ಮಗು->argv[1], "%%%d", &jobnum) != 1) {
			error_ಸಂದೇಶ("%s: bad argument '%s'", ಮಗು->argv[0], ಮಗು->argv[1]);
			ಹಿಂತಿರುಗಿ EXIT_FAILURE;
		}
		ಗಾಗಿ (pi = ಕೆಲಸ_list; pi; pi = pi->ಮುಂದಿನ) {
			ಆದರೆ (pi->jobid == jobnum) {
				ನಿಲ್ಲಿಸು;
			}
		}
		ಆದರೆ (!pi) {
			error_ಸಂದೇಶ("%s: %d: no such job", ಮಗು->argv[0], jobnum);
			ಹಿಂತಿರುಗಿ EXIT_FAILURE;
		}
	}

	ಆದರೆ (*ಮಗು->argv[0] == 'f') {
		/* Put the job into the foreground.  */
		tcsetpgrp(shell_terminal, pi->pgrp);
	}

	/* Restart the processes in the job */
	ಗಾಗಿ (i = 0; i < pi->ಅಂಕೆ_progs; i++)
		pi->progs[i].is_stopped = 0;

	ಆದರೆ ( (i=kill(- pi->pgrp, SIGCONT)) < 0) {
		ಆದರೆ (i == ESRCH) {
			remove_bg_ಕೆಲಸ(pi);
		} ಇಲ್ಲದಿದ್ದರೆ {
			perror_ಸಂದೇಶ("kill (SIGCONT)");
		}
	}

	pi->stopped_progs = 0;
	ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
}

/* built-in 'help' handler */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ builtin_help(ರಚನೆ ಮಗು_prog *dummy)
{
	ರಚನೆ built_ಒಳ_command *x;

	ಬರೆ("\nBuilt-in commands:\n");
	ಬರೆ("-------------------\n");
	ಗಾಗಿ (x = bltins; x->cmd; x++) {
		ಆದರೆ (x->descr==ಖಾಲಿ)
			ಮುಂದುವರಿ;
		ಬರೆ("%s\t%s\n", x->cmd, x->descr);
	}
	ಬರೆ("\n\n");
	ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
}

/* built-in 'jobs' handler */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ builtin_jobs(ರಚನೆ ಮಗು_prog *ಮಗು)
{
	ರಚನೆ pipe *ಕೆಲಸ;
	ಅಕ್ಷರ *ಸ್ಥಿತಿ_string;

	ಗಾಗಿ (ಕೆಲಸ = ಕೆಲಸ_list; ಕೆಲಸ; ಕೆಲಸ = ಕೆಲಸ->ಮುಂದಿನ) {
		ಆದರೆ (ಕೆಲಸ->running_progs == ಕೆಲಸ->stopped_progs)
			ಸ್ಥಿತಿ_string = "Stopped";
		ಇಲ್ಲದಿದ್ದರೆ
			ಸ್ಥಿತಿ_string = "Running";

		ಬರೆ(ಕೆಲಸ_ಸ್ಥಿತಿ_FORMAT, ಕೆಲಸ->jobid, ಸ್ಥಿತಿ_string, ಕೆಲಸ->ಪಠ್ಯ);
	}
	ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
}

/* built-in 'pwd' handler */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ builtin_pwd(ರಚನೆ ಮಗು_prog *dummy)
{
	puts(ಗಣ_cwd());
	ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
}

/* built-in 'read VAR' handler */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ builtin_read(ರಚನೆ ಮಗು_prog *ಮಗು)
{
	ಪೂರ್ಣಾಂಕ res;

	ಆದರೆ (ಮಗು->argv[1]) {
		ಅಕ್ಷರ string[BUFSIZ];
		ಅಕ್ಷರ *var = 0;

		string[0] = 0;  /* In case stdin has only EOF */
		/* read string */
		fgets(string, ಗಾತ್ರ(string), stdin);
		chomp(string);
		var = ಸ್ಮರಣೆ_ಕೊಡು(ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(ಮಗು->argv[1])+ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(string)+2);
		ಆದರೆ(var) {
			sprintf(var, "%s=%s", ಮಗು->argv[1], string);
			res = ಗಣ_local_var(var, 0);
		} ಇಲ್ಲದಿದ್ದರೆ
			res = -1;
		ಆದರೆ (res)
			fprintf(stderr, "read: %m\n");
		ಬಿಡುಗಡೆ(var);      /* So not move up to avoid breaking errno */
		ಹಿಂತಿರುಗಿ res;
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಮಾಡು res=getchar(); ಆಗ(res!='\n' && res!=EOF);
		ಹಿಂತಿರುಗಿ 0;
	}
}

/* built-in 'set VAR=value' handler */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ builtin_ಗಣ(ರಚನೆ ಮಗು_prog *ಮಗು)
{
	ಅಕ್ಷರ *ತಾತ್ಕಾಲಿಕ = ಮಗು->argv[1];
	ರಚನೆ variables *e;

	ಆದರೆ (ತಾತ್ಕಾಲಿಕ == ಖಾಲಿ)
		ಗಾಗಿ(e = ತುದಿ_vars; e; e=e->ಮುಂದಿನ)
			ಬರೆ("%s=%s\n", e->name, e->ಮೌಲ್ಯ);
	ಇಲ್ಲದಿದ್ದರೆ
		ಗಣ_local_var(ತಾತ್ಕಾಲಿಕ, 0);

		ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
}

/* Built-in 'shift' handler */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ builtin_ಸರಿಸು(ರಚನೆ ಮಗು_prog *ಮಗು)
{
	ಪೂರ್ಣಾಂಕ n=1;
	ಆದರೆ (ಮಗು->argv[1]) {
		n=atoi(ಮಗು->argv[1]);
	}
	ಆದರೆ (n>=0 && n<global_argc) {
		/* XXX This probably breaks $0 */
		global_argc -= n;
		global_argv += n;
		ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಹಿಂತಿರುಗಿ EXIT_FAILURE;
	}
}

/* Built-in '.' handler (read-in and execute commands from file) */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ builtin_ಮೂಲ(ರಚನೆ ಮಗು_prog *ಮಗು)
{
	FILE *ಒಳಹರಿವು;
	ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2;

	ಆದರೆ (ಮಗು->argv[1] == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ EXIT_FAILURE;

	/* XXX search through $PATH is missing */
	ಒಳಹರಿವು = fopen(ಮಗು->argv[1], "r");
	ಆದರೆ (!ಒಳಹರಿವು) {
		error_ಸಂದೇಶ("Couldn't open file '%s'", ಮಗು->argv[1]);
		ಹಿಂತಿರುಗಿ EXIT_FAILURE;
	}

	/* Now run the file */
	/* XXX argv and argc are broken; need to save old global_argv
	 * (pointer only is OK!) on this stack frame,
	 * set global_argv=child->argv+1, recurse, and restore. */
	mark_open(fileno(ಒಳಹರಿವು));
	ಸ್ಥಿತಿ_2 = parse_file_outer(ಒಳಹರಿವು);
	mark_closed(fileno(ಒಳಹರಿವು));
	fclose(ಒಳಹರಿವು);
	ಹಿಂತಿರುಗಿ (ಸ್ಥಿತಿ_2);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ builtin_umask(ರಚನೆ ಮಗು_prog *ಮಗು)
{
	mode_t new_umask;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *arg = ಮಗು->argv[1];
	ಅಕ್ಷರ *ಕೊನೆ_2;
	ಆದರೆ (arg) {
		new_umask=strtoul(arg, &ಕೊನೆ_2, 8);
		ಆದರೆ (*ಕೊನೆ_2!='\0' || ಕೊನೆ_2 == arg) {
			ಹಿಂತಿರುಗಿ EXIT_FAILURE;
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಬರೆ("%.3o\n", (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ) (new_umask=umask(0)));
	}
	umask(new_umask);
	ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
}

/* built-in 'unset VAR' handler */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ builtin_unset(ರಚನೆ ಮಗು_prog *ಮಗು)
{
	/* bash returned already true */
	unset_local_var(ಮಗು->argv[1]);
	ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ builtin_not_written(ರಚನೆ ಮಗು_prog *ಮಗು)
{
	ಬರೆ("builtin_%s not written\n",ಮಗು->argv[0]);
	ಹಿಂತಿರುಗಿ EXIT_FAILURE;
}
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ b_check_space(o_string *o, ಪೂರ್ಣಾಂಕ len)
{
	/* It would be easy to drop a more restrictive policy
	 * in here, such as setting a maximum string length */
	ಆದರೆ (o->ಉದ್ದಳತೆ + len > o->maxlen) {
		ಅಕ್ಷರ *old_ದತ್ತಾಂಶ = o->ದತ್ತಾಂಶ;
		/* assert (data == NULL || o->maxlen != 0); */
		o->maxlen += max(2*len, B_CHUNK);
		o->ದತ್ತಾಂಶ = realloc(o->ದತ್ತಾಂಶ, 1 + o->maxlen);
		ಆದರೆ (o->ದತ್ತಾಂಶ == ಖಾಲಿ) {
			ಬಿಡುಗಡೆ(old_ದತ್ತಾಂಶ);
		}
	}
	ಹಿಂತಿರುಗಿ o->ದತ್ತಾಂಶ == ಖಾಲಿ;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ b_addchr(o_string *o, ಪೂರ್ಣಾಂಕ ch)
{
	debug_printf("b_addchr: %c %d %p\n", ch, o->ಉದ್ದಳತೆ, o);
	ಆದರೆ (b_check_space(o, 1)) ಹಿಂತಿರುಗಿ B_NOSPAC;
	o->ದತ್ತಾಂಶ[o->ಉದ್ದಳತೆ] = ch;
	o->ಉದ್ದಳತೆ++;
	o->ದತ್ತಾಂಶ[o->ಉದ್ದಳತೆ] = '\0';
	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಶೂನ್ಯ b_reset(o_string *o)
{
	o->ಉದ್ದಳತೆ = 0;
	o->nonnull = 0;
	ಆದರೆ (o->ದತ್ತಾಂಶ != ಖಾಲಿ) *o->ದತ್ತಾಂಶ = '\0';
}

ಸ್ಥಿರ ಶೂನ್ಯ b_free(o_string *o)
{
	b_reset(o);
	ಬಿಡುಗಡೆ(o->ದತ್ತಾಂಶ);
	o->ದತ್ತಾಂಶ = ಖಾಲಿ;
	o->maxlen = 0;
}

/* My analysis of quoting semantics tells me that state information
 * is associated with a destination, not a source.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ b_addqchr(o_string *o, ಪೂರ್ಣಾಂಕ ch, ಪೂರ್ಣಾಂಕ quote)
{
	ಆದರೆ (quote && strchr("*?[\\",ch)) {
		ಪೂರ್ಣಾಂಕ rc;
		rc = b_addchr(o, '\\');
		ಆದರೆ (rc) ಹಿಂತಿರುಗಿ rc;
	}
	ಹಿಂತಿರುಗಿ b_addchr(o, ch);
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ b_adduint(o_string *o, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ i)
{
	ಪೂರ್ಣಾಂಕ r;
	ಅಕ್ಷರ *p = simple_itoa(i);
	/* no escape checking necessary */
	ಮಾಡು r=b_addchr(o, *p++); ಆಗ (r==0 && *p);
	ಹಿಂತಿರುಗಿ r;
}
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ static_get(ರಚನೆ ಒಳ_str *i)
{
	ಪೂರ್ಣಾಂಕ ch = *i->p++;
	ಆದರೆ (ch=='\0') ಹಿಂತಿರುಗಿ EOF;
	ಹಿಂತಿರುಗಿ ch;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ static_peek(ರಚನೆ ಒಳ_str *i)
{
	ಹಿಂತಿರುಗಿ *i->p;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ cmdedit_ಗಣ_initial_prompt(ಶೂನ್ಯ)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ CONFIG_FEATURE_SH_FANCY_PROMPT
	PS1 = ಖಾಲಿ;
#ಇಲ್ಲದಿದ್ದರೆ
	PS1 = env_get("PS1");
	ಆದರೆ(PS1==0)
		PS1 = "\\w \\$ ";
#ಆದರೆ_ಅಂತ್ಯ
}

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಶೂನ್ಯ ಸಿದ್ಧತೆ_prompt_string(ಪೂರ್ಣಾಂಕ promptmode, ಅಕ್ಷರ **prompt_str)
{
	debug_printf("setup_prompt_string %d ",promptmode);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ CONFIG_FEATURE_SH_FANCY_PROMPT
	/* Set up the prompt */
	ಆದರೆ (promptmode == 1) {
		ಬಿಡುಗಡೆ(PS1);
		PS1=xmalloc(ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(cwd)+4);
		sprintf(PS1, "%s %s", cwd, ( geteuid() != 0 ) ?  "$ ":"# ");
		*prompt_str = PS1;
	} ಇಲ್ಲದಿದ್ದರೆ {
		*prompt_str = PS2;
	}
#ಇಲ್ಲದಿದ್ದರೆ
	*prompt_str = (promptmode==1)? PS1 : PS2;
#ಆದರೆ_ಅಂತ್ಯ
	debug_printf("result %s\n",*prompt_str);
}
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __U_BOOT__
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ uboot_cli_readline(ರಚನೆ ಒಳ_str *i)
{
	ಅಕ್ಷರ *prompt;
	ಅಕ್ಷರ __maybe_unused *ps_prompt = ಖಾಲಿ;

	ಆದರೆ (i->promptmode == 1)
		prompt = CONFIG_SYS_PROMPT;
	ಇಲ್ಲದಿದ್ದರೆ
		prompt = CONFIG_SYS_PROMPT_HUSH_PS2;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_CMDLINE_PS_SUPPORT
	ಆದರೆ (i->promptmode == 1)
		ps_prompt = env_get("PS1");
	ಇಲ್ಲದಿದ್ದರೆ
		ps_prompt = env_get("PS2");
	ಆದರೆ (ps_prompt)
		prompt = ps_prompt;
#ಆದರೆ_ಅಂತ್ಯ

	ಹಿಂತಿರುಗಿ cli_readline(prompt);
}
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ಶೂನ್ಯ get_user_ಒಳಹರಿವು(ರಚನೆ ಒಳ_str *i)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	ಅಕ್ಷರ *prompt_str;
	ಸ್ಥಿರ ಅಕ್ಷರ the_command[BUFSIZ];

	ಸಿದ್ಧತೆ_prompt_string(i->promptmode, &prompt_str);
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_FEATURE_COMMAND_EDITING
	/*
	 ** enable command line editing only while a command line
	 ** is actually being read; otherwise, we'll end up bequeathing
	 ** atexit() handlers and other unwanted stuff to our
	 ** child processes (rob@sysgo.de)
	 */
	cmdedit_read_ಒಳಹರಿವು(prompt_str, the_command);
#ಇಲ್ಲದಿದ್ದರೆ
	fputs(prompt_str, stdout);
	fflush(stdout);
	the_command[0]=fgetc(i->file);
	the_command[1]='\0';
#ಆದರೆ_ಅಂತ್ಯ
	fflush(stdout);
	i->p = the_command;
#ಇಲ್ಲದಿದ್ದರೆ
	ಪೂರ್ಣಾಂಕ n;
	ಸ್ಥಿರ ಅಕ್ಷರ the_command[CONFIG_SYS_CBSIZE + 1];

	bootretry_reset_cmd_timeout();
	i->__promptme = 1;
	n = uboot_cli_readline(i);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_BOOT_RETRY_ಸಮಯ
	ಆದರೆ (n == -2) {
	  puts("\nTimeout waiting for command\n");
#  ifdef CONFIG_RESET_ಗಮ್ಯ_RETRY
	  do_reset(ಖಾಲಿ, 0, 0, ಖಾಲಿ);
#  elif IS_ENABLED(CONFIG_RETRY_BOOTCMD)
	strcpy(console_ಬಫರ್, "run bootcmd\n");
# ಇಲ್ಲದಿದ್ದರೆ
#	error "This only works with CONFIG_RESET_TO_RETRY or CONFIG_BOOT_RETRY_COMMAND enabled"
#  endif
	}
#ಆದರೆ_ಅಂತ್ಯ
	ಆದರೆ (n == -1 ) {
		flag_repeat = 0;
		i->__promptme = 0;
	}
	n = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(console_ಬಫರ್);
	console_ಬಫರ್[n] = '\n';
	console_ಬಫರ್[n+1]= '\0';
	ಆದರೆ (had_ctrlc()) flag_repeat = 0;
	clear_ctrlc();
	do_repeat = 0;
	ಆದರೆ (i->promptmode == 1) {
		ಆದರೆ (console_ಬಫರ್[0] == '\n'&& flag_repeat == 0) {
			strcpy(the_command,console_ಬಫರ್);
		}
		ಇಲ್ಲದಿದ್ದರೆ {
			ಆದರೆ (console_ಬಫರ್[0] != '\n') {
				strcpy(the_command,console_ಬಫರ್);
				flag_repeat = 1;
			}
			ಇಲ್ಲದಿದ್ದರೆ {
				do_repeat = 1;
			}
		}
		i->p = the_command;
	}
	ಇಲ್ಲದಿದ್ದರೆ {
		ಆದರೆ (console_ಬಫರ್[0] != '\n') {
			ಆದರೆ (ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(the_command) + ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(console_ಬಫರ್)
			    < CONFIG_SYS_CBSIZE) {
				n = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(the_command);
				the_command[n-1] = ' ';
				strcpy(&the_command[n],console_ಬಫರ್);
			}
			ಇಲ್ಲದಿದ್ದರೆ {
				the_command[0] = '\n';
				the_command[1] = '\0';
				flag_repeat = 0;
			}
		}
		ಆದರೆ (i->__promptme == 0) {
			the_command[0] = '\n';
			the_command[1] = '\0';
		}
		i->p = console_ಬಫರ್;
	}
#ಆದರೆ_ಅಂತ್ಯ
}

/* This is the magic location that prints prompts
 * and gets data back from the user */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ file_get(ರಚನೆ ಒಳ_str *i)
{
	ಪೂರ್ಣಾಂಕ ch;

	ch = 0;
	/* If there is data waiting, eat it up */
	ಆದರೆ (i->p && *i->p) {
		ch = *i->p++;
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* need to double check i->file because we might be doing something
		 * more complicated by now, like sourcing or substituting. */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
		ಆದರೆ (i->__promptme && interactive && i->file == stdin) {
			ಆಗ(! i->p || (interactive && ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(i->p)==0) ) {
#ಇಲ್ಲದಿದ್ದರೆ
			ಆಗ(! i->p  || ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(i->p)==0 ) {
#ಆದರೆ_ಅಂತ್ಯ
				get_user_ಒಳಹರಿವು(i);
			}
			i->promptmode=2;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
			i->__promptme = 0;
#ಆದರೆ_ಅಂತ್ಯ
			ಆದರೆ (i->p && *i->p) {
				ch = *i->p++;
			}
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
		} ಇಲ್ಲದಿದ್ದರೆ {
			ch = fgetc(i->file);
		}

#ಆದರೆ_ಅಂತ್ಯ
		debug_printf("b_getch: got a %d\n", ch);
	}
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	ಆದರೆ (ch == '\n') i->__promptme=1;
#ಆದರೆ_ಅಂತ್ಯ
	ಹಿಂತಿರುಗಿ ch;
}

/* All the callers guarantee this routine will never be
 * used right after a newline, so prompting is not needed.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ file_peek(ರಚನೆ ಒಳ_str *i)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	ಆದರೆ (i->p && *i->p) {
#ಆದರೆ_ಅಂತ್ಯ
		ಹಿಂತಿರುಗಿ *i->p;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	} ಇಲ್ಲದಿದ್ದರೆ {
		i->peek_buf[0] = fgetc(i->file);
		i->peek_buf[1] = '\0';
		i->p = i->peek_buf;
		debug_printf("b_peek: got a %d\n", *i->p);
		ಹಿಂತಿರುಗಿ *i->p;
	}
#ಆದರೆ_ಅಂತ್ಯ
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
ಸ್ಥಿರ ಶೂನ್ಯ ಸಿದ್ಧತೆ_file_ಒಳ_str(ರಚನೆ ಒಳ_str *i, FILE *f)
#ಇಲ್ಲದಿದ್ದರೆ
ಸ್ಥಿರ ಶೂನ್ಯ ಸಿದ್ಧತೆ_file_ಒಳ_str(ರಚನೆ ಒಳ_str *i)
#ಆದರೆ_ಅಂತ್ಯ
{
	i->peek = file_peek;
	i->get = file_get;
	i->__promptme=1;
	i->promptmode=1;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	i->file = f;
#ಆದರೆ_ಅಂತ್ಯ
	i->p = ಖಾಲಿ;
}

ಸ್ಥಿರ ಶೂನ್ಯ ಸಿದ್ಧತೆ_string_ಒಳ_str(ರಚನೆ ಒಳ_str *i, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s)
{
	i->peek = static_peek;
	i->get = static_get;
	i->__promptme=1;
	i->promptmode=1;
	i->p = s;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
ಸ್ಥಿರ ಶೂನ್ಯ mark_open(ಪೂರ್ಣಾಂಕ fd)
{
	ರಚನೆ close_me *new = xmalloc(ಗಾತ್ರ(ರಚನೆ close_me));
	new->fd = fd;
	new->ಮುಂದಿನ = close_me_ತಲೆ;
	close_me_ತಲೆ = new;
}

ಸ್ಥಿರ ಶೂನ್ಯ mark_closed(ಪೂರ್ಣಾಂಕ fd)
{
	ರಚನೆ close_me *tmp;
	ಆದರೆ (close_me_ತಲೆ == ಖಾಲಿ || close_me_ತಲೆ->fd != fd)
		error_ಸಂದೇಶ_and_die("corrupt close_me");
	tmp = close_me_ತಲೆ;
	close_me_ತಲೆ = close_me_ತಲೆ->ಮುಂದಿನ;
	ಬಿಡುಗಡೆ(tmp);
}

ಸ್ಥಿರ ಶೂನ್ಯ close_all(ಶೂನ್ಯ)
{
	ರಚನೆ close_me *c;
	ಗಾಗಿ (c=close_me_ತಲೆ; c; c=c->ಮುಂದಿನ) {
		close(c->fd);
	}
	close_me_ತಲೆ = ಖಾಲಿ;
}

/* squirrel != NULL means we squirrel away copies of stdin, stdout,
 * and stderr if they are redirected. */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಸಿದ್ಧತೆ_redirects(ರಚನೆ ಮಗು_prog *prog, ಪೂರ್ಣಾಂಕ squirrel[])
{
	ಪೂರ್ಣಾಂಕ openfd, ಬಹುಲಕ;
	ರಚನೆ redir_struct *redir;

	ಗಾಗಿ (redir=prog->redirects; redir; redir=redir->ಮುಂದಿನ) {
		ಆದರೆ (redir->dup == -1 && redir->ಶಬ್ದ.gl_pathv == ಖಾಲಿ) {
			/* something went wrong in the parse.  Pretend it didn't happen */
			ಮುಂದುವರಿ;
		}
		ಆದರೆ (redir->dup == -1) {
			ಬಹುಲಕ=redir_ಕೋಷ್ಟಕ[redir->type].ಬಹುಲಕ;
			openfd = open(redir->ಶಬ್ದ.gl_pathv[0], ಬಹುಲಕ, 0666);
			ಆದರೆ (openfd < 0) {
			/* this could get lost if stderr has been redirected, but
			   bash and ash both lose it as well (though zsh doesn't!) */
				perror_ಸಂದೇಶ("error opening %s", redir->ಶಬ್ದ.gl_pathv[0]);
				ಹಿಂತಿರುಗಿ 1;
			}
		} ಇಲ್ಲದಿದ್ದರೆ {
			openfd = redir->dup;
		}

		ಆದರೆ (openfd != redir->fd) {
			ಆದರೆ (squirrel && redir->fd < 3) {
				squirrel[redir->fd] = dup(redir->fd);
			}
			ಆದರೆ (openfd == -3) {
				close(openfd);
			} ಇಲ್ಲದಿದ್ದರೆ {
				dup2(openfd, redir->fd);
				ಆದರೆ (redir->dup == -1)
					close (openfd);
			}
		}
	}
	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಶೂನ್ಯ restore_redirects(ಪೂರ್ಣಾಂಕ squirrel[])
{
	ಪೂರ್ಣಾಂಕ i, fd;
	ಗಾಗಿ (i=0; i<3; i++) {
		fd = squirrel[i];
		ಆದರೆ (fd != -1) {
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
ಸ್ಥಿರ ಶೂನ್ಯ pseudo_exec(ರಚನೆ ಮಗು_prog *ಮಗು)
{
	ಪೂರ್ಣಾಂಕ i, rcode;
	ಅಕ್ಷರ *p;
	ರಚನೆ built_ಒಳ_command *x;
	ಆದರೆ (ಮಗು->argv) {
		ಗಾಗಿ (i=0; is_assignment(ಮಗು->argv[i]); i++) {
			debug_printf("pid %d environment modification: %s\n",getpid(),ಮಗು->argv[i]);
			p = ಸೇರಿಸು_var_ಮೌಲ್ಯ(ಮಗು->argv[i]);
			putenv(strdup(p));
			ಆದರೆ (p != ಮಗು->argv[i]) ಬಿಡುಗಡೆ(p);
		}
		ಮಗು->argv+=i;  /* XXX this hack isn't so horrible, since we are about
					to exit, and therefore don't need to keep data
					structures consistent for free() use. */
		/* If a variable is assigned in a forest, and nobody listens,
		 * was it ever really set?
		 */
		ಆದರೆ (ಮಗು->argv[0] == ಖಾಲಿ) {
			_exit(EXIT_SUCCESS);
		}

		/*
		 * Check if the command matches any of the builtins.
		 * Depending on context, this might be redundant.  But it's
		 * easier to waste a few CPU cycles than it is to figure out
		 * if this is one of those cases.
		 */
		ಗಾಗಿ (x = bltins; x->cmd; x++) {
			ಆದರೆ (strcmp(ಮಗು->argv[0], x->cmd) == 0 ) {
				debug_printf("builtin exec %s\n", ಮಗು->argv[0]);
				rcode = x->function(ಮಗು);
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
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_FEATURE_SH_STANDALONE_SHELL
		{
			ಪೂರ್ಣಾಂಕ argc_l;
			ಅಕ್ಷರ** argv_l=ಮಗು->argv;
			ಅಕ್ಷರ *name = ಮಗು->argv[0];

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_FEATURE_SH_APPLETS_ALWAYS_WIN
			/* Following discussions from November 2000 on the busybox mailing
			 * list, the default configuration, (without
			 * get_last_path_component()) lets the user force use of an
			 * external command by specifying the full (with slashes) filename.
			 * If you enable CONFIG_FEATURE_SH_APPLETS_ALWAYS_WIN then applets
			 * _aways_ override external commands, so if you want to run
			 * /bin/cat, it will use BusyBox cat even if /bin/cat exists on the
			 * filesystem and is _not_ busybox.  Some systems may want this,
			 * most do not.  */
			name = get_ಕೊನೆಯ_ಮಾರ್ಗ_component(name);
#ಆದರೆ_ಅಂತ್ಯ
			/* Count argc for use in a second... */
			ಗಾಗಿ(argc_l=0;*argv_l!=ಖಾಲಿ; argv_l++, argc_l++);
			optind = 1;
			debug_printf("running applet %s\n", name);
			ಸತತ_applet_by_name(name, argc_l, ಮಗು->argv);
		}
#ಆದರೆ_ಅಂತ್ಯ
		debug_printf("exec of %s\n",ಮಗು->argv[0]);
		execvp(ಮಗು->argv[0],ಮಗು->argv);
		perror_ಸಂದೇಶ("couldn't exec: %s",ಮಗು->argv[0]);
		_exit(1);
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಮಗು->ಗುಂಪು) {
		debug_printf("runtime nesting to group\n");
		interactive=0;    /* crucial!!!! */
		rcode = ಸತತ_list_real(ಮಗು->ಗುಂಪು);
		/* OK to leak memory by not calling free_pipe_list,
		 * since this process is about to exit */
		_exit(rcode);
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* Can happen.  See what bash does with ">foo" by itself. */
		debug_printf("trying to pseudo_exec null command\n");
		_exit(EXIT_SUCCESS);
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ ಸೇರಿಸು_bg_ಕೆಲಸ(ರಚನೆ pipe *pi)
{
	ರಚನೆ pipe *thejob;

	/* Linear search for the ID of the job to use */
	pi->jobid = 1;
	ಗಾಗಿ (thejob = ಕೆಲಸ_list; thejob; thejob = thejob->ಮುಂದಿನ)
		ಆದರೆ (thejob->jobid >= pi->jobid)
			pi->jobid = thejob->jobid + 1;

	/* add thejob to the list of running jobs */
	ಆದರೆ (!ಕೆಲಸ_list) {
		thejob = ಕೆಲಸ_list = xmalloc(ಗಾತ್ರ(*thejob));
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಗಾಗಿ (thejob = ಕೆಲಸ_list; thejob->ಮುಂದಿನ; thejob = thejob->ಮುಂದಿನ) /* nothing */;
		thejob->ಮುಂದಿನ = xmalloc(ಗಾತ್ರ(*thejob));
		thejob = thejob->ಮುಂದಿನ;
	}

	/* physically copy the struct job */
	ಸ್ಮರಣೆ_ನಕಲು(thejob, pi, ಗಾತ್ರ(ರಚನೆ pipe));
	thejob->ಮುಂದಿನ = ಖಾಲಿ;
	thejob->running_progs = thejob->ಅಂಕೆ_progs;
	thejob->stopped_progs = 0;
	thejob->ಪಠ್ಯ = xmalloc(BUFSIZ); /* cmdedit buffer size */

	/*if (pi->progs[0] && pi->progs[0].argv && pi->progs[0].argv[0]) */
	{
		ಅಕ್ಷರ *bar=thejob->ಪಠ್ಯ;
		ಅಕ್ಷರ **foo=pi->progs[0].argv;
		ಆಗ(foo && *foo) {
			bar += sprintf(bar, "%s ", *foo++);
		}
	}

	/* we don't wait for background thejobs to return -- append it
	   to the list of backgrounded thejobs and leave it alone */
	ಬರೆ("[%d] %d\n", thejob->jobid, thejob->progs[0].pid);
	ಕೊನೆಯ_bg_pid = thejob->progs[0].pid;
	ಕೊನೆಯ_jobid = thejob->jobid;
}

/* remove a backgrounded job */
ಸ್ಥಿರ ಶೂನ್ಯ remove_bg_ಕೆಲಸ(ರಚನೆ pipe *pi)
{
	ರಚನೆ pipe *prev_pipe;

	ಆದರೆ (pi == ಕೆಲಸ_list) {
		ಕೆಲಸ_list = pi->ಮುಂದಿನ;
	} ಇಲ್ಲದಿದ್ದರೆ {
		prev_pipe = ಕೆಲಸ_list;
		ಆಗ (prev_pipe->ಮುಂದಿನ != pi)
			prev_pipe = prev_pipe->ಮುಂದಿನ;
		prev_pipe->ಮುಂದಿನ = pi->ಮುಂದಿನ;
	}
	ಆದರೆ (ಕೆಲಸ_list)
		ಕೊನೆಯ_jobid = ಕೆಲಸ_list->jobid;
	ಇಲ್ಲದಿದ್ದರೆ
		ಕೊನೆಯ_jobid = 0;

	pi->stopped_progs = 0;
	free_pipe(pi, 0);
	ಬಿಡುಗಡೆ(pi);
}

/* Checks to see if any processes have exited -- if they
   have, figure out why and see if a job has completed */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ checkjobs(ರಚನೆ pipe* fg_pipe)
{
	ಪೂರ್ಣಾಂಕ ಗುಣಲಕ್ಷಣಗಳು;
	ಪೂರ್ಣಾಂಕ ಸ್ಥಿತಿ_2;
	ಪೂರ್ಣಾಂಕ prognum = 0;
	ರಚನೆ pipe *pi;
	pid_t childpid;

	ಗುಣಲಕ್ಷಣಗಳು = WUNTRACED;
	ಆದರೆ (fg_pipe==ಖಾಲಿ) {
		ಗುಣಲಕ್ಷಣಗಳು |= WNOHANG;
	}

	ಆಗ ((childpid = waitpid(-1, &ಸ್ಥಿತಿ_2, ಗುಣಲಕ್ಷಣಗಳು)) > 0) {
		ಆದರೆ (fg_pipe) {
			ಪೂರ್ಣಾಂಕ i, rcode = 0;
			ಗಾಗಿ (i=0; i < fg_pipe->ಅಂಕೆ_progs; i++) {
				ಆದರೆ (fg_pipe->progs[i].pid == childpid) {
					ಆದರೆ (i==fg_pipe->ಅಂಕೆ_progs-1)
						rcode=WEXITSTATUS(ಸ್ಥಿತಿ_2);
					(fg_pipe->ಅಂಕೆ_progs)--;
					ಹಿಂತಿರುಗಿ(rcode);
				}
			}
		}

		ಗಾಗಿ (pi = ಕೆಲಸ_list; pi; pi = pi->ಮುಂದಿನ) {
			prognum = 0;
			ಆಗ (prognum < pi->ಅಂಕೆ_progs && pi->progs[prognum].pid != childpid) {
				prognum++;
			}
			ಆದರೆ (prognum < pi->ಅಂಕೆ_progs)
				ನಿಲ್ಲಿಸು;
		}

		ಆದರೆ(pi==ಖಾಲಿ) {
			debug_printf("checkjobs: pid %d was not in our list!\n", childpid);
			ಮುಂದುವರಿ;
		}

		ಆದರೆ (WIFEXITED(ಸ್ಥಿತಿ_2) || WIFSIGNALED(ಸ್ಥಿತಿ_2)) {
			/* child exited */
			pi->running_progs--;
			pi->progs[prognum].pid = 0;

			ಆದರೆ (!pi->running_progs) {
				ಬರೆ(ಕೆಲಸ_ಸ್ಥಿತಿ_FORMAT, pi->jobid, "Done", pi->ಪಠ್ಯ);
				remove_bg_ಕೆಲಸ(pi);
			}
		} ಇಲ್ಲದಿದ್ದರೆ {
			/* child stopped */
			pi->stopped_progs++;
			pi->progs[prognum].is_stopped = 1;

#ಆದರೆ 0
			/* Printing this stuff is a pain, since it tends to
			 * overwrite the prompt an inconveinient moments.  So
			 * don't do that.  */
			ಆದರೆ (pi->stopped_progs == pi->ಅಂಕೆ_progs) {
				ಬರೆ("\n"ಕೆಲಸ_ಸ್ಥಿತಿ_FORMAT, pi->jobid, "Stopped", pi->ಪಠ್ಯ);
			}
#ಆದರೆ_ಅಂತ್ಯ
		}
	}

	ಆದರೆ (childpid == -1 && errno != ECHILD)
		perror_ಸಂದೇಶ("waitpid");

	/* move the shell to the foreground */
	/*if (interactive && tcsetpgrp(shell_terminal, getpgid(0))) */
	/*	perror_msg("tcsetpgrp-2"); */
	ಹಿಂತಿರುಗಿ -1;
}

/* Figure out our controlling tty, checking in order stderr,
 * stdin, and stdout.  If check_pgrp is set, also check that
 * we belong to the foreground process group associated with
 * that tty.  The value of shell_terminal is needed in order to call
 * tcsetpgrp(shell_terminal, ...); */
ಶೂನ್ಯ controlling_tty(ಪೂರ್ಣಾಂಕ check_pgrp)
{
	pid_t curpgrp;

	ಆದರೆ ((curpgrp = tcgetpgrp(shell_terminal = 2)) < 0
			&& (curpgrp = tcgetpgrp(shell_terminal = 0)) < 0
			&& (curpgrp = tcgetpgrp(shell_terminal = 1)) < 0)
		ಹೋಗು shell_terminal_error;

	ಆದರೆ (check_pgrp && curpgrp != getpgid(0))
		ಹೋಗು shell_terminal_error;

	ಹಿಂತಿರುಗಿ;

shell_terminal_error:
		shell_terminal = -1;
		ಹಿಂತಿರುಗಿ;
}
#ಆದರೆ_ಅಂತ್ಯ

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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಸತತ_pipe_real(ರಚನೆ pipe *pi)
{
	ಪೂರ್ಣಾಂಕ i;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	ಪೂರ್ಣಾಂಕ nextin, nextout;
	ಪೂರ್ಣಾಂಕ pipefds[2];				/* pipefds[0] is for reading */
	ರಚನೆ ಮಗು_prog *ಮಗು;
	ರಚನೆ built_ಒಳ_command *x;
	ಅಕ್ಷರ *p;
# ಆದರೆ __GNUC__
	/* Avoid longjmp clobbering */
	(ಶೂನ್ಯ) &i;
	(ಶೂನ್ಯ) &nextin;
	(ಶೂನ್ಯ) &nextout;
	(ಶೂನ್ಯ) &ಮಗು;
# endif
#ಇಲ್ಲದಿದ್ದರೆ
	ಪೂರ್ಣಾಂಕ nextin;
	ಪೂರ್ಣಾಂಕ flag = do_repeat ? CMD_FLAG_REPEAT : 0;
	ರಚನೆ ಮಗು_prog *ಮಗು;
	ಅಕ್ಷರ *p;
# ಆದರೆ __GNUC__
	/* Avoid longjmp clobbering */
	(ಶೂನ್ಯ) &i;
	(ಶೂನ್ಯ) &nextin;
	(ಶೂನ್ಯ) &ಮಗು;
# endif
#ಆದರೆ_ಅಂತ್ಯ	/* __U_BOOT__ */

	nextin = 0;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	pi->pgrp = -1;
#ಆದರೆ_ಅಂತ್ಯ

	/* Check if this is a simple builtin (not part of a pipe).
	 * Builtins within pipes have to fork anyway, and are handled in
	 * pseudo_exec.  "echo foo | read bar" doesn't work on bash, either.
	 */
	ಆದರೆ (pi->ಅಂಕೆ_progs == 1) ಮಗು = & (pi->progs[0]);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	ಆದರೆ (pi->ಅಂಕೆ_progs == 1 && ಮಗು->ಗುಂಪು && ಮಗು->subshell == 0) {
		ಪೂರ್ಣಾಂಕ squirrel[] = {-1, -1, -1};
		ಪೂರ್ಣಾಂಕ rcode;
		debug_printf("non-subshell grouping\n");
		ಸಿದ್ಧತೆ_redirects(ಮಗು, squirrel);
		/* XXX could we merge code with following builtin case,
		 * by creating a pseudo builtin that calls run_list_real? */
		rcode = ಸತತ_list_real(ಮಗು->ಗುಂಪು);
		restore_redirects(squirrel);
#ಇಲ್ಲದಿದ್ದರೆ
		ಆದರೆ (pi->ಅಂಕೆ_progs == 1 && ಮಗು->ಗುಂಪು) {
		ಪೂರ್ಣಾಂಕ rcode;
		debug_printf("non-subshell grouping\n");
		rcode = ಸತತ_list_real(ಮಗು->ಗುಂಪು);
#ಆದರೆ_ಅಂತ್ಯ
		ಹಿಂತಿರುಗಿ rcode;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (pi->ಅಂಕೆ_progs == 1 && pi->progs[0].argv != ಖಾಲಿ) {
		ಗಾಗಿ (i=0; is_assignment(ಮಗು->argv[i]); i++) { /* nothing */ }
		ಆದರೆ (i!=0 && ಮಗು->argv[i]==ಖಾಲಿ) {
			/* assignments, but no command: set the local environment */
			ಗಾಗಿ (i=0; ಮಗು->argv[i]!=ಖಾಲಿ; i++) {

				/* Ok, this case is tricky.  We have to decide if this is a
				 * local variable, or an already exported variable.  If it is
				 * already exported, we have to export the new value.  If it is
				 * not exported, we need only set this as a local variable.
				 * This junk is all to decide whether or not to export this
				 * variable. */
				ಪೂರ್ಣಾಂಕ export_me=0;
				ಅಕ್ಷರ *name, *ಮೌಲ್ಯ;
				name = xstrdup(ಮಗು->argv[i]);
				debug_printf("Local environment set: %s\n", name);
				ಮೌಲ್ಯ = strchr(name, '=');
				ಆದರೆ (ಮೌಲ್ಯ)
					*ಮೌಲ್ಯ=0;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
				ಆದರೆ ( get_local_var(name)) {
					export_me=1;
				}
#ಆದರೆ_ಅಂತ್ಯ
				ಬಿಡುಗಡೆ(name);
				p = ಸೇರಿಸು_var_ಮೌಲ್ಯ(ಮಗು->argv[i]);
				ಗಣ_local_var(p, export_me);
				ಆದರೆ (p != ಮಗು->argv[i]) ಬಿಡುಗಡೆ(p);
			}
			ಹಿಂತಿರುಗಿ EXIT_SUCCESS;   /* don't worry about errors in set_local_var() yet */
		}
		ಗಾಗಿ (i = 0; is_assignment(ಮಗು->argv[i]); i++) {
			p = ಸೇರಿಸು_var_ಮೌಲ್ಯ(ಮಗು->argv[i]);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
			putenv(strdup(p));
#ಇಲ್ಲದಿದ್ದರೆ
			ಗಣ_local_var(p, 0);
#ಆದರೆ_ಅಂತ್ಯ
			ಆದರೆ (p != ಮಗು->argv[i]) {
				ಮಗು->sp--;
				ಬಿಡುಗಡೆ(p);
			}
		}
		ಆದರೆ (ಮಗು->sp) {
			ಅಕ್ಷರ * str = ಖಾಲಿ;

			str = make_string(ಮಗು->argv + i,
					  ಮಗು->argv_nonnull + i);
			parse_string_outer(str, FLAG_EXIT_FROM_LOOP | FLAG_REPARSING);
			ಬಿಡುಗಡೆ(str);
			ಹಿಂತಿರುಗಿ ಕೊನೆಯ_return_ಸಂಕೇತ;
		}
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
		ಗಾಗಿ (x = bltins; x->cmd; x++) {
			ಆದರೆ (strcmp(ಮಗು->argv[i], x->cmd) == 0 ) {
				ಪೂರ್ಣಾಂಕ squirrel[] = {-1, -1, -1};
				ಪೂರ್ಣಾಂಕ rcode;
				ಆದರೆ (x->function == builtin_exec && ಮಗು->argv[i+1]==ಖಾಲಿ) {
					debug_printf("magic exec\n");
					ಸಿದ್ಧತೆ_redirects(ಮಗು,ಖಾಲಿ);
					ಹಿಂತಿರುಗಿ EXIT_SUCCESS;
				}
				debug_printf("builtin inline %s\n", ಮಗು->argv[0]);
				/* XXX setup_redirects acts on file descriptors, not FILEs.
				 * This is perfect for work that comes after exec().
				 * Is it really safe for inline use?  Experimentally,
				 * things seem to work with glibc. */
				ಸಿದ್ಧತೆ_redirects(ಮಗು, squirrel);

				ಮಗು->argv += i;  /* XXX horrible hack */
				rcode = x->function(ಮಗು);
				/* XXX restore hack so free() can work right */
				ಮಗು->argv -= i;
				restore_redirects(squirrel);
			}
			ಹಿಂತಿರುಗಿ rcode;
		}
#ಇಲ್ಲದಿದ್ದರೆ
		/* check ";", because ,example , argv consist from
		 * "help;flinfo" must not execute
		 */
		ಆದರೆ (strchr(ಮಗು->argv[i], ';')) {
			ಬರೆ("Unknown command '%s' - try 'help' or use "
					"'run' command\n", ಮಗು->argv[i]);
			ಹಿಂತಿರುಗಿ -1;
		}
		/* Process the command */
		ಹಿಂತಿರುಗಿ cmd_process(flag, ಮಗು->argc - i, ಮಗು->argv + i,
				   &flag_repeat, ಖಾಲಿ);
#ಆದರೆ_ಅಂತ್ಯ
	}
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__

	ಗಾಗಿ (i = 0; i < pi->ಅಂಕೆ_progs; i++) {
		ಮಗು = & (pi->progs[i]);

		/* pipes are inserted between pairs of commands */
		ಆದರೆ ((i + 1) < pi->ಅಂಕೆ_progs) {
			ಆದರೆ (pipe(pipefds)<0) perror_ಸಂದೇಶ_and_die("pipe");
			nextout = pipefds[1];
		} ಇಲ್ಲದಿದ್ದರೆ {
			nextout=1;
			pipefds[0] = -1;
		}

		/* XXX test for failed fork()? */
		ಆದರೆ (!(ಮಗು->pid = fork())) {
			/* Set the handling for job control signals back to the default.  */
			ಸಂಕೇತ(SIGINT, SIG_DFL);
			ಸಂಕೇತ(SIGQUIT, SIG_DFL);
			ಸಂಕೇತ(SIGTERM, SIG_DFL);
			ಸಂಕೇತ(SIGTSTP, SIG_DFL);
			ಸಂಕೇತ(SIGTTIN, SIG_DFL);
			ಸಂಕೇತ(SIGTTOU, SIG_DFL);
			ಸಂಕೇತ(SIGCHLD, SIG_DFL);

			close_all();

			ಆದರೆ (nextin != 0) {
				dup2(nextin, 0);
				close(nextin);
			}
			ಆದರೆ (nextout != 1) {
				dup2(nextout, 1);
				close(nextout);
			}
			ಆದರೆ (pipefds[0]!=-1) {
				close(pipefds[0]);  /* opposite end of our output pipe */
			}

			/* Like bash, explicit redirects override pipes,
			 * and the pipe fd is available for dup'ing. */
			ಸಿದ್ಧತೆ_redirects(ಮಗು,ಖಾಲಿ);

			ಆದರೆ (interactive && pi->followup!=PIPE_BG) {
				/* If we (the child) win the race, put ourselves in the process
				 * group whose leader is the first process in this pipe. */
				ಆದರೆ (pi->pgrp < 0) {
					pi->pgrp = getpid();
				}
				ಆದರೆ (setpgid(0, pi->pgrp) == 0) {
					tcsetpgrp(2, pi->pgrp);
				}
			}

			pseudo_exec(ಮಗು);
		}

		/* put our child in the process group whose leader is the
		   first process in this pipe */
		ಆದರೆ (pi->pgrp < 0) {
			pi->pgrp = ಮಗು->pid;
		}
		/* Don't check for errors.  The child may be dead already,
		 * in which case setpgid returns error code EACCES. */
		setpgid(ಮಗು->pid, pi->pgrp);

		ಆದರೆ (nextin != 0)
			close(nextin);
		ಆದರೆ (nextout != 1)
			close(nextout);

		/* If there isn't another process, nextin is garbage
		   but it doesn't matter */
		nextin = pipefds[0];
	}
#ಆದರೆ_ಅಂತ್ಯ
	ಹಿಂತಿರುಗಿ -1;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಸತತ_list_real(ರಚನೆ pipe *pi)
{
	ಅಕ್ಷರ *save_name = ಖಾಲಿ;
	ಅಕ್ಷರ **list = ಖಾಲಿ;
	ಅಕ್ಷರ **save_list = ಖಾಲಿ;
	ರಚನೆ pipe *rpipe;
	ಪೂರ್ಣಾಂಕ flag_rep = 0;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	ಪೂರ್ಣಾಂಕ save_ಅಂಕೆ_progs;
#ಆದರೆ_ಅಂತ್ಯ
	ಪೂರ್ಣಾಂಕ rcode=0, flag_skip=1;
	ಪೂರ್ಣಾಂಕ flag_restore = 0;
	ಪೂರ್ಣಾಂಕ if_ಸಂಕೇತ=0, ಮುಂದಿನ_if_ಸಂಕೇತ=0;  /* need double-buffer to handle elif */
	reserved_style rmode, skip_ಇನ್ನಷ್ಟು_ಒಳ_this_rmode=RES_XXXX;
	/* check syntax for "for" */
	ಗಾಗಿ (rpipe = pi; rpipe; rpipe = rpipe->ಮುಂದಿನ) {
		ಆದರೆ ((rpipe->r_ಬಹುಲಕ == RES_ಒಳ ||
		    rpipe->r_ಬಹುಲಕ == RES_FOR) &&
		    (rpipe->ಮುಂದಿನ == ಖಾಲಿ)) {
				syntax();
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __U_BOOT__
				flag_repeat = 0;
#ಆದರೆ_ಅಂತ್ಯ
				ಹಿಂತಿರುಗಿ 1;
		}
		ಆದರೆ ((rpipe->r_ಬಹುಲಕ == RES_ಒಳ &&
			(rpipe->ಮುಂದಿನ->r_ಬಹುಲಕ == RES_ಒಳ &&
			rpipe->ಮುಂದಿನ->progs->argv != ಖಾಲಿ))||
			(rpipe->r_ಬಹುಲಕ == RES_FOR &&
			rpipe->ಮುಂದಿನ->r_ಬಹುಲಕ != RES_ಒಳ)) {
				syntax();
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __U_BOOT__
				flag_repeat = 0;
#ಆದರೆ_ಅಂತ್ಯ
				ಹಿಂತಿರುಗಿ 1;
		}
	}
	ಗಾಗಿ (; pi; pi = (flag_restore != 0) ? rpipe : pi->ಮುಂದಿನ) {
		ಆದರೆ (pi->r_ಬಹುಲಕ == RES_WHILE || pi->r_ಬಹುಲಕ == RES_UNTIL ||
			pi->r_ಬಹುಲಕ == RES_FOR) {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __U_BOOT__
				/* check Ctrl-C */
				ctrlc();
				ಆದರೆ ((had_ctrlc())) {
					ಹಿಂತಿರುಗಿ 1;
				}
#ಆದರೆ_ಅಂತ್ಯ
				flag_restore = 0;
				ಆದರೆ (!rpipe) {
					flag_rep = 0;
					rpipe = pi;
				}
		}
		rmode = pi->r_ಬಹುಲಕ;
		debug_printf("rmode=%d  if_code=%d  next_if_code=%d skip_more=%d\n", rmode, if_ಸಂಕೇತ, ಮುಂದಿನ_if_ಸಂಕೇತ, skip_ಇನ್ನಷ್ಟು_ಒಳ_this_rmode);
		ಆದರೆ (rmode == skip_ಇನ್ನಷ್ಟು_ಒಳ_this_rmode && flag_skip) {
			ಆದರೆ (pi->followup == PIPE_SEQ) flag_skip=0;
			ಮುಂದುವರಿ;
		}
		flag_skip = 1;
		skip_ಇನ್ನಷ್ಟು_ಒಳ_this_rmode = RES_XXXX;
		ಆದರೆ (rmode == RES_THEN || rmode == RES_ELSE) if_ಸಂಕೇತ = ಮುಂದಿನ_if_ಸಂಕೇತ;
		ಆದರೆ (rmode == RES_THEN &&  if_ಸಂಕೇತ) ಮುಂದುವರಿ;
		ಆದರೆ (rmode == RES_ELSE && !if_ಸಂಕೇತ) ಮುಂದುವರಿ;
		ಆದರೆ (rmode == RES_ELIF && !if_ಸಂಕೇತ) ನಿಲ್ಲಿಸು;
		ಆದರೆ (rmode == RES_FOR && pi->ಅಂಕೆ_progs) {
			ಆದರೆ (!list) {
				/* if no variable values after "in" we skip "for" */
				ಆದರೆ (!pi->ಮುಂದಿನ->progs->argv) ಮುಂದುವರಿ;
				/* create list of variable values */
				list = make_list_ಒಳ(pi->ಮುಂದಿನ->progs->argv,
					pi->progs->argv[0]);
				save_list = list;
				save_name = pi->progs->argv[0];
				pi->progs->argv[0] = ಖಾಲಿ;
				flag_rep = 1;
			}
			ಆದರೆ (!(*list)) {
				ಬಿಡುಗಡೆ(pi->progs->argv[0]);
				ಬಿಡುಗಡೆ(save_list);
				list = ಖಾಲಿ;
				flag_rep = 0;
				pi->progs->argv[0] = save_name;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
				pi->progs->glob_ಫಲಿತಾಂಶ.gl_pathv[0] =
					pi->progs->argv[0];
#ಆದರೆ_ಅಂತ್ಯ
				ಮುಂದುವರಿ;
			} ಇಲ್ಲದಿದ್ದರೆ {
				/* insert new value from list for variable */
				ಬಿಡುಗಡೆ(pi->progs->argv[0]);
				pi->progs->argv[0] = *list++;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
				pi->progs->glob_ಫಲಿತಾಂಶ.gl_pathv[0] =
					pi->progs->argv[0];
#ಆದರೆ_ಅಂತ್ಯ
			}
		}
		ಆದರೆ (rmode == RES_ಒಳ) ಮುಂದುವರಿ;
		ಆದರೆ (rmode == RES_DO) {
			ಆದರೆ (!flag_rep) ಮುಂದುವರಿ;
		}
		ಆದರೆ (rmode == RES_ಮುಗಿದ) {
			ಆದರೆ (flag_rep) {
				flag_restore = 1;
			} ಇಲ್ಲದಿದ್ದರೆ {
				rpipe = ಖಾಲಿ;
			}
		}
		ಆದರೆ (pi->ಅಂಕೆ_progs == 0) ಮುಂದುವರಿ;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
		save_ಅಂಕೆ_progs = pi->ಅಂಕೆ_progs; /* save number of programs */
#ಆದರೆ_ಅಂತ್ಯ
		rcode = ಸತತ_pipe_real(pi);
		debug_printf("run_pipe_real returned %d\n",rcode);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
		ಆದರೆ (rcode!=-1) {
			/* We only ran a builtin: rcode was set by the return value
			 * of run_pipe_real(), and we don't need to wait for anything. */
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (pi->followup==PIPE_BG) {
			/* XXX check bash's behavior with nontrivial pipes */
			/* XXX compute jobid */
			/* XXX what does bash do with attempts to background builtins? */
			ಸೇರಿಸು_bg_ಕೆಲಸ(pi);
			rcode = EXIT_SUCCESS;
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಆದರೆ (interactive) {
				/* move the new process group into the foreground */
				ಆದರೆ (tcsetpgrp(shell_terminal, pi->pgrp) && errno != ENOTTY)
					perror_ಸಂದೇಶ("tcsetpgrp-3");
				rcode = checkjobs(pi);
				/* move the shell to the foreground */
				ಆದರೆ (tcsetpgrp(shell_terminal, getpgid(0)) && errno != ENOTTY)
					perror_ಸಂದೇಶ("tcsetpgrp-4");
			} ಇಲ್ಲದಿದ್ದರೆ {
				rcode = checkjobs(pi);
			}
			debug_printf("checkjobs returned %d\n",rcode);
		}
		ಕೊನೆಯ_return_ಸಂಕೇತ=rcode;
#ಇಲ್ಲದಿದ್ದರೆ
		ಆದರೆ (rcode < -1) {
			ಕೊನೆಯ_return_ಸಂಕೇತ = -rcode - 2;
			ಹಿಂತಿರುಗಿ -2;	/* exit */
		}
		ಕೊನೆಯ_return_ಸಂಕೇತ = rcode;
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
		pi->ಅಂಕೆ_progs = save_ಅಂಕೆ_progs; /* restore number of programs */
#ಆದರೆ_ಅಂತ್ಯ
		ಆದರೆ ( rmode == RES_IF || rmode == RES_ELIF )
			ಮುಂದಿನ_if_ಸಂಕೇತ=rcode;  /* can be overwritten a number of times */
		ಆದರೆ (rmode == RES_WHILE)
			flag_rep = !ಕೊನೆಯ_return_ಸಂಕೇತ;
		ಆದರೆ (rmode == RES_UNTIL)
			flag_rep = ಕೊನೆಯ_return_ಸಂಕೇತ;
		ಆದರೆ ( (rcode==EXIT_SUCCESS && pi->followup==PIPE_OR) ||
		     (rcode!=EXIT_SUCCESS && pi->followup==PIPE_AND) )
			skip_ಇನ್ನಷ್ಟು_ಒಳ_this_rmode=rmode;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
		checkjobs(ಖಾಲಿ);
#ಆದರೆ_ಅಂತ್ಯ
	}
	ಹಿಂತಿರುಗಿ rcode;
}

/* broken, of course, but OK for testing */
ಸ್ಥಿರ ಅಕ್ಷರ *indenter(ಪೂರ್ಣಾಂಕ i)
{
	ಸ್ಥಿರ ಅಕ್ಷರ blanks[]="                                    ";
	ಹಿಂತಿರುಗಿ &blanks[ಗಾತ್ರ(blanks)-i-1];
}

/* return code is the exit status of the pipe */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ free_pipe(ರಚನೆ pipe *pi, ಪೂರ್ಣಾಂಕ indent)
{
	ಅಕ್ಷರ **p;
	ರಚನೆ ಮಗು_prog *ಮಗು;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	ರಚನೆ redir_struct *r, *rnext;
#ಆದರೆ_ಅಂತ್ಯ
	ಪೂರ್ಣಾಂಕ a, i, ret_ಸಂಕೇತ=0;
	ಅಕ್ಷರ *ind = indenter(indent);

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	ಆದರೆ (pi->stopped_progs > 0)
		ಹಿಂತಿರುಗಿ ret_ಸಂಕೇತ;
	final_printf("%s run pipe: (pid %d)\n",ind,getpid());
#ಆದರೆ_ಅಂತ್ಯ
	ಗಾಗಿ (i=0; i<pi->ಅಂಕೆ_progs; i++) {
		ಮಗು = &pi->progs[i];
		final_printf("%s  command %d:\n",ind,i);
		ಆದರೆ (ಮಗು->argv) {
			ಗಾಗಿ (a=0,p=ಮಗು->argv; *p; a++,p++) {
				final_printf("%s   argv[%d] = %s\n",ind,a,*p);
			}
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
			globfree(&ಮಗು->glob_ಫಲಿತಾಂಶ);
#ಇಲ್ಲದಿದ್ದರೆ
			ಗಾಗಿ (a = 0; a < ಮಗು->argc; a++) {
				ಬಿಡುಗಡೆ(ಮಗು->argv[a]);
			}
			ಬಿಡುಗಡೆ(ಮಗು->argv);
			ಬಿಡುಗಡೆ(ಮಗು->argv_nonnull);
			ಮಗು->argc = 0;
#ಆದರೆ_ಅಂತ್ಯ
			ಮಗು->argv=ಖಾಲಿ;
		} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಮಗು->ಗುಂಪು) {
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
			final_printf("%s   begin group (subshell:%d)\n",ind, ಮಗು->subshell);
#ಆದರೆ_ಅಂತ್ಯ
			ret_ಸಂಕೇತ = free_pipe_list(ಮಗು->ಗುಂಪು,indent+3);
			final_printf("%s   end group\n",ind);
		} ಇಲ್ಲದಿದ್ದರೆ {
			final_printf("%s   (nil)\n",ind);
		}
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
		ಗಾಗಿ (r=ಮಗು->redirects; r; r=rnext) {
			final_printf("%s   redirect %d%s", ind, r->fd, redir_ಕೋಷ್ಟಕ[r->type].descrip);
			ಆದರೆ (r->dup == -1) {
				/* guard against the case >$FOO, where foo is unset or blank */
				ಆದರೆ (r->ಶಬ್ದ.gl_pathv) {
					final_printf(" %s\n", *r->ಶಬ್ದ.gl_pathv);
					globfree(&r->ಶಬ್ದ);
				}
			} ಇಲ್ಲದಿದ್ದರೆ {
				final_printf("&%d\n", r->dup);
			}
			rnext=r->ಮುಂದಿನ;
			ಬಿಡುಗಡೆ(r);
		}
		ಮಗು->redirects=ಖಾಲಿ;
#ಆದರೆ_ಅಂತ್ಯ
	}
	ಬಿಡುಗಡೆ(pi->progs);   /* children are an array, they get freed all at once */
	pi->progs=ಖಾಲಿ;
	ಹಿಂತಿರುಗಿ ret_ಸಂಕೇತ;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ free_pipe_list(ರಚನೆ pipe *ತಲೆ, ಪೂರ್ಣಾಂಕ indent)
{
	ಪೂರ್ಣಾಂಕ rcode=0;   /* if list has no members */
	ರಚನೆ pipe *pi, *ಮುಂದಿನ;
	ಅಕ್ಷರ *ind = indenter(indent);
	ಗಾಗಿ (pi=ತಲೆ; pi; pi=ಮುಂದಿನ) {
		final_printf("%s pipe reserved mode %d\n", ind, pi->r_ಬಹುಲಕ);
		rcode = free_pipe(pi, indent);
		final_printf("%s pipe followup code %d\n", ind, pi->followup);
		ಮುಂದಿನ=pi->ಮುಂದಿನ;
		pi->ಮುಂದಿನ=ಖಾಲಿ;
		ಬಿಡುಗಡೆ(pi);
	}
	ಹಿಂತಿರುಗಿ rcode;
}

/* Select which version we will use */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಸತತ_list(ರಚನೆ pipe *pi)
{
	ಪೂರ್ಣಾಂಕ rcode=0;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	ಆದರೆ (fake_ಬಹುಲಕ==0) {
#ಆದರೆ_ಅಂತ್ಯ
		rcode = ಸತತ_list_real(pi);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	}
#ಆದರೆ_ಅಂತ್ಯ
	/* free_pipe_list has the side effect of clearing memory
	 * In the long run that function can be merged with run_list_real,
	 * but doing that now would hobble the debugging effort. */
	free_pipe_list(pi,0);
	ಹಿಂತಿರುಗಿ rcode;
}

/* The API for glob is arguably broken.  This routine pushes a non-matching
 * string into the output structure, removing non-backslashed backslashes.
 * If someone can prove me wrong, by performing this function within the
 * original glob(3) api, feel free to rewrite this routine into oblivion.
 * Return code (0 vs. GLOB_NOSPACE) matches glob(3).
 * XXX broken if the last character is '\\', check that before calling.
 */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ globhack(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *src, ಪೂರ್ಣಾಂಕ flags, glob_t *pglob)
{
	ಪೂರ್ಣಾಂಕ cnt=0, pathc;
	ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s;
	ಅಕ್ಷರ *dest;
	ಗಾಗಿ (cnt=1, s=src; s && *s; s++) {
		ಆದರೆ (*s == '\\') s++;
		cnt++;
	}
	dest = ಸ್ಮರಣೆ_ಕೊಡು(cnt);
	ಆದರೆ (!dest) ಹಿಂತಿರುಗಿ GLOB_NOSPACE;
	ಆದರೆ (!(flags & GLOB_APPEND)) {
		pglob->gl_pathv=ಖಾಲಿ;
		pglob->gl_pathc=0;
		pglob->gl_offs=0;
		pglob->gl_offs=0;
	}
	pathc = ++pglob->gl_pathc;
	pglob->gl_pathv = realloc(pglob->gl_pathv, (pathc+1)*ಗಾತ್ರ(*pglob->gl_pathv));
	ಆದರೆ (pglob->gl_pathv == ಖಾಲಿ) ಹಿಂತಿರುಗಿ GLOB_NOSPACE;
	pglob->gl_pathv[pathc-1]=dest;
	pglob->gl_pathv[pathc]=ಖಾಲಿ;
	ಗಾಗಿ (s=src; s && *s; s++, dest++) {
		ಆದರೆ (*s == '\\') s++;
		*dest = *s;
	}
	*dest='\0';
	ಹಿಂತಿರುಗಿ 0;
}

/* XXX broken if the last character is '\\', check that before calling */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ glob_needed(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s)
{
	ಗಾಗಿ (; *s; s++) {
		ಆದರೆ (*s == '\\') s++;
		ಆದರೆ (strchr("*[?",*s)) ಹಿಂತಿರುಗಿ 1;
	}
	ಹಿಂತಿರುಗಿ 0;
}

#ಆದರೆ 0
ಸ್ಥಿರ ಶೂನ್ಯ globprint(glob_t *pglob)
{
	ಪೂರ್ಣಾಂಕ i;
	debug_printf("glob_t at %p:\n", pglob);
	debug_printf("  gl_pathc=%d  gl_pathv=%p  gl_offs=%d  gl_flags=%d\n",
		pglob->gl_pathc, pglob->gl_pathv, pglob->gl_offs, pglob->gl_flags);
	ಗಾಗಿ (i=0; i<pglob->gl_pathc; i++)
		debug_printf("pglob->gl_pathv[%d] = %p = %s\n", i,
			pglob->gl_pathv[i], pglob->gl_pathv[i]);
}
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ xglob(o_string *dest, ಪೂರ್ಣಾಂಕ flags, glob_t *pglob)
{
	ಪೂರ್ಣಾಂಕ gr;

	/* short-circuit for null word */
	/* we can code this better when the debug_printf's are gone */
	ಆದರೆ (dest->ಉದ್ದಳತೆ == 0) {
		ಆದರೆ (dest->nonnull) {
			/* bash man page calls this an "explicit" null */
			gr = globhack(dest->ದತ್ತಾಂಶ, flags, pglob);
			debug_printf("globhack returned %d\n",gr);
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಹಿಂತಿರುಗಿ 0;
		}
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (glob_needed(dest->ದತ್ತಾಂಶ)) {
		gr = glob(dest->ದತ್ತಾಂಶ, flags, ಖಾಲಿ, pglob);
		debug_printf("glob returned %d\n",gr);
		ಆದರೆ (gr == GLOB_NOMATCH) {
			/* quote removal, or more accurately, backslash removal */
			gr = globhack(dest->ದತ್ತಾಂಶ, flags, pglob);
			debug_printf("globhack returned %d\n",gr);
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		gr = globhack(dest->ದತ್ತಾಂಶ, flags, pglob);
		debug_printf("globhack returned %d\n",gr);
	}
	ಆದರೆ (gr == GLOB_NOSPACE)
		error_ಸಂದೇಶ_and_die("out of memory during glob");
	ಆದರೆ (gr != 0) { /* GLOB_ABORTED ? */
		error_ಸಂದೇಶ("glob(3) error %d",gr);
	}
	/* globprint(glob_target); */
	ಹಿಂತಿರುಗಿ gr;
}
#ಆದರೆ_ಅಂತ್ಯ

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __U_BOOT__
ಸ್ಥಿರ ಅಕ್ಷರ *get_dollar_var(ಅಕ್ಷರ ch);
#ಆದರೆ_ಅಂತ್ಯ

/* This is used to get/check local shell variables */
ಅಕ್ಷರ *get_local_var(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s)
{
	ರಚನೆ variables *cur;

	ಆದರೆ (!s)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __U_BOOT__
	ಆದರೆ (*s == '$')
		ಹಿಂತಿರುಗಿ get_dollar_var(s[1]);
#ಆದರೆ_ಅಂತ್ಯ

	ಗಾಗಿ (cur = ತುದಿ_vars; cur; cur=cur->ಮುಂದಿನ)
		ಆದರೆ(strcmp(cur->name, s)==0)
			ಹಿಂತಿರುಗಿ cur->ಮೌಲ್ಯ;
	ಹಿಂತಿರುಗಿ ಖಾಲಿ;
}

/* This is used to set local shell variables
   flg_export==0 if only local (not exporting) variable
   flg_export==1 if "new" exporting environ
   flg_export>1  if current startup environ (not call putenv()) */
ಪೂರ್ಣಾಂಕ ಗಣ_local_var(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s, ಪೂರ್ಣಾಂಕ flg_export)
{
	ಅಕ್ಷರ *name, *ಮೌಲ್ಯ;
	ಪೂರ್ಣಾಂಕ ಫಲಿತಾಂಶ=0;
	ರಚನೆ variables *cur;

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __U_BOOT__
	/* might be possible! */
	ಆದರೆ (!ಅಕ್ಷರವೇ(*s))
		ಹಿಂತಿರುಗಿ -1;
#ಆದರೆ_ಅಂತ್ಯ

	name=strdup(s);

	/* Assume when we enter this function that we are already in
	 * NAME=VALUE format.  So the first order of business is to
	 * split 's' on the '=' into 'name' and 'value' */
	ಮೌಲ್ಯ = strchr(name, '=');
	ಆದರೆ (!ಮೌಲ್ಯ) {
		ಬಿಡುಗಡೆ(name);
		ಹಿಂತಿರುಗಿ -1;
	}
	*ಮೌಲ್ಯ++ = 0;

	ಆದರೆ (!*ಮೌಲ್ಯ) {
		unset_local_var(name);
		ಬಿಡುಗಡೆ(name);
		ಹಿಂತಿರುಗಿ 0;
	}

	ಗಾಗಿ(cur = ತುದಿ_vars; cur; cur = cur->ಮುಂದಿನ) {
		ಆದರೆ(strcmp(cur->name, name)==0)
			ನಿಲ್ಲಿಸು;
	}

	ಆದರೆ(cur) {
		ಆದರೆ(strcmp(cur->ಮೌಲ್ಯ, ಮೌಲ್ಯ)==0) {
			ಆದರೆ(flg_export>0 && cur->flg_export==0)
				cur->flg_export=flg_export;
			ಇಲ್ಲದಿದ್ದರೆ
				ಫಲಿತಾಂಶ++;
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಆದರೆ(cur->flg_read_only) {
				error_ಸಂದೇಶ("%s: readonly variable", name);
				ಫಲಿತಾಂಶ = -1;
			} ಇಲ್ಲದಿದ್ದರೆ {
				ಆದರೆ(flg_export>0 || cur->flg_export>1)
					cur->flg_export=1;
				ಬಿಡುಗಡೆ(cur->ಮೌಲ್ಯ);

				cur->ಮೌಲ್ಯ = strdup(ಮೌಲ್ಯ);
			}
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		cur = ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ(ರಚನೆ variables));
		ಆದರೆ(!cur) {
			ಫಲಿತಾಂಶ = -1;
		} ಇಲ್ಲದಿದ್ದರೆ {
			cur->name = strdup(name);
			ಆದರೆ (cur->name == ಖಾಲಿ) {
				ಬಿಡುಗಡೆ(cur);
				ಫಲಿತಾಂಶ = -1;
			} ಇಲ್ಲದಿದ್ದರೆ {
				ರಚನೆ variables *bottom = ತುದಿ_vars;
				cur->ಮೌಲ್ಯ = strdup(ಮೌಲ್ಯ);
				cur->ಮುಂದಿನ = ಖಾಲಿ;
				cur->flg_export = flg_export;
				cur->flg_read_only = 0;
				ಆಗ(bottom->ಮುಂದಿನ) bottom=bottom->ಮುಂದಿನ;
				bottom->ಮುಂದಿನ = cur;
			}
		}
	}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	ಆದರೆ(ಫಲಿತಾಂಶ==0 && cur->flg_export==1) {
		*(ಮೌಲ್ಯ-1) = '=';
		ಫಲಿತಾಂಶ = putenv(name);
	} ಇಲ್ಲದಿದ್ದರೆ {
#ಆದರೆ_ಅಂತ್ಯ
		ಬಿಡುಗಡೆ(name);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
		ಆದರೆ(ಫಲಿತಾಂಶ>0)            /* equivalent to previous set */
			ಫಲಿತಾಂಶ = 0;
	}
#ಆದರೆ_ಅಂತ್ಯ
	ಹಿಂತಿರುಗಿ ಫಲಿತಾಂಶ;
}

ಶೂನ್ಯ unset_local_var(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *name)
{
	ರಚನೆ variables *cur;

	ಆದರೆ (name) {
		ಗಾಗಿ (cur = ತುದಿ_vars; cur; cur=cur->ಮುಂದಿನ) {
			ಆದರೆ(strcmp(cur->name, name)==0)
				ನಿಲ್ಲಿಸು;
		}
		ಆದರೆ (cur != ಖಾಲಿ) {
			ರಚನೆ variables *ಮುಂದಿನ = ತುದಿ_vars;
			ಆದರೆ(cur->flg_read_only) {
				error_ಸಂದೇಶ("%s: readonly variable", name);
				ಹಿಂತಿರುಗಿ;
			} ಇಲ್ಲದಿದ್ದರೆ {
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
				ಆದರೆ(cur->flg_export)
					unenv_ಗಣ(cur->name);
#ಆದರೆ_ಅಂತ್ಯ
				ಬಿಡುಗಡೆ(cur->name);
				ಬಿಡುಗಡೆ(cur->ಮೌಲ್ಯ);
				ಆಗ (ಮುಂದಿನ->ಮುಂದಿನ != cur)
					ಮುಂದಿನ = ಮುಂದಿನ->ಮುಂದಿನ;
				ಮುಂದಿನ->ಮುಂದಿನ = cur->ಮುಂದಿನ;
			}
			ಬಿಡುಗಡೆ(cur);
		}
	}
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ is_assignment(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s)
{
	ಆದರೆ (s == ಖಾಲಿ)
		ಹಿಂತಿರುಗಿ 0;

	ಆದರೆ (!ಅಕ್ಷರವೇ(*s)) ಹಿಂತಿರುಗಿ 0;
	++s;
	ಆಗ(ಅಕ್ಷರವೇ_ಅಂಕಿಯೇ(*s) || *s=='_') ++s;
	ಹಿಂತಿರುಗಿ *s=='=';
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
/* the src parameter allows us to peek forward to a possible &n syntax
 * for file descriptor duplication, e.g., "2>&1".
 * Return code is 0 normally, 1 if a syntax error is detected in src.
 * Resource errors (in xmalloc) cause the process to exit */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಸಿದ್ಧತೆ_redirect(ರಚನೆ p_context *ctx, ಪೂರ್ಣಾಂಕ fd, redir_type style,
	ರಚನೆ ಒಳ_str *ಒಳಹರಿವು)
{
	ರಚನೆ ಮಗು_prog *ಮಗು=ctx->ಮಗು;
	ರಚನೆ redir_struct *redir = ಮಗು->redirects;
	ರಚನೆ redir_struct *ಕೊನೆಯ_redir=ಖಾಲಿ;

	/* Create a new redir_struct and drop it onto the end of the linked list */
	ಆಗ(redir) {
		ಕೊನೆಯ_redir=redir;
		redir=redir->ಮುಂದಿನ;
	}
	redir = xmalloc(ಗಾತ್ರ(ರಚನೆ redir_struct));
	redir->ಮುಂದಿನ=ಖಾಲಿ;
	redir->ಶಬ್ದ.gl_pathv=ಖಾಲಿ;
	ಆದರೆ (ಕೊನೆಯ_redir) {
		ಕೊನೆಯ_redir->ಮುಂದಿನ=redir;
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಮಗು->redirects=redir;
	}

	redir->type=style;
	redir->fd= (fd==-1) ? redir_ಕೋಷ್ಟಕ[style].default_fd : fd ;

	debug_printf("Redirect type %d%s\n", redir->fd, redir_ಕೋಷ್ಟಕ[style].descrip);

	/* Check for a '2>&1' type redirect */
	redir->dup = redirect_dup_ಅಂಕೆ(ಒಳಹರಿವು);
	ಆದರೆ (redir->dup == -2) ಹಿಂತಿರುಗಿ 1;  /* syntax error */
	ಆದರೆ (redir->dup != -1) {
		/* Erik had a check here that the file descriptor in question
		 * is legit; I postpone that to "run time"
		 * A "-" representation of "close me" shows up as a -3 here */
		debug_printf("Duplicating redirect '%d>&%d'\n", redir->fd, redir->dup);
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* We do _not_ try to open the file that src points to,
		 * since we need to return and let src be expanded first.
		 * Set ctx->pending_redirect, so we know what to do at the
		 * end of the next parsed word.
		 */
		ctx->pending_redirect = redir;
	}
	ಹಿಂತಿರುಗಿ 0;
}
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ರಚನೆ pipe *new_pipe(ಶೂನ್ಯ)
{
	ರಚನೆ pipe *pi;
	pi = xmalloc(ಗಾತ್ರ(ರಚನೆ pipe));
	pi->ಅಂಕೆ_progs = 0;
	pi->progs = ಖಾಲಿ;
	pi->ಮುಂದಿನ = ಖಾಲಿ;
	pi->followup = 0;  /* invalid */
	pi->r_ಬಹುಲಕ = RES_NONE;
	ಹಿಂತಿರುಗಿ pi;
}

ಸ್ಥಿರ ಶೂನ್ಯ initialize_context(ರಚನೆ p_context *ctx)
{
	ctx->pipe=ಖಾಲಿ;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	ctx->pending_redirect=ಖಾಲಿ;
#ಆದರೆ_ಅಂತ್ಯ
	ctx->ಮಗು=ಖಾಲಿ;
	ctx->list_ತಲೆ=new_pipe();
	ctx->pipe=ctx->list_ತಲೆ;
	ctx->w=RES_NONE;
	ctx->ರಾಶಿ=ಖಾಲಿ;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __U_BOOT__
	ctx->old_flag=0;
#ಆದರೆ_ಅಂತ್ಯ
	ಮುಗಿದ_command(ctx);   /* creates the memory for working child */
}

/* normal return is 0
 * if a reserved word is found, and processed, return 1
 * should handle if, then, elif, else, fi, for, while, until, do, done.
 * case, function, and select are obnoxious, save those for later.
 */
ರಚನೆ reserved_combo {
	ಅಕ್ಷರ *literal;
	ಪೂರ್ಣಾಂಕ ಸಂಕೇತ_2;
	ದೀರ್ಘ flag;
};
/* Mostly a list of accepted follow-up reserved words.
 * FLAG_END means we are done with the sequence, and are ready
 * to turn the compound list into a command.
 * FLAG_START means the word must start a new compound list.
 */
ಸ್ಥಿರ ರಚನೆ reserved_combo reserved_list[] = {
	{ "if",    RES_IF,    FLAG_THEN | FLAG_ಆರಂಭ },
	{ "then",  RES_THEN,  FLAG_ELIF | FLAG_ELSE | FLAG_FI },
	{ "elif",  RES_ELIF,  FLAG_THEN },
	{ "else",  RES_ELSE,  FLAG_FI   },
	{ "fi",    RES_FI,    FLAG_ಕೊನೆ  },
	{ "for",   RES_FOR,   FLAG_ಒಳ   | FLAG_ಆರಂಭ },
	{ "while", RES_WHILE, FLAG_DO   | FLAG_ಆರಂಭ },
	{ "until", RES_UNTIL, FLAG_DO   | FLAG_ಆರಂಭ },
	{ "in",    RES_ಒಳ,    FLAG_DO   },
	{ "do",    RES_DO,    FLAG_ಮುಗಿದ },
	{ "done",  RES_ಮುಗಿದ,  FLAG_ಕೊನೆ  }
};
#ವ್ಯಾಖ್ಯಾನಿಸು NRES (ಗಾತ್ರ(reserved_list)/ಗಾತ್ರ(ರಚನೆ reserved_combo))

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ reserved_ಶಬ್ದ(o_string *dest, ರಚನೆ p_context *ctx)
{
	ರಚನೆ reserved_combo *r;
	ಗಾಗಿ (r=reserved_list;
		r<reserved_list+NRES; r++) {
		ಆದರೆ (strcmp(dest->ದತ್ತಾಂಶ, r->literal) == 0) {
			debug_printf("found reserved word %s, code %d\n",r->literal,r->ಸಂಕೇತ_2);
			ಆದರೆ (r->flag & FLAG_ಆರಂಭ) {
				ರಚನೆ p_context *new = xmalloc(ಗಾತ್ರ(ರಚನೆ p_context));
				debug_printf("push stack\n");
				ಆದರೆ (ctx->w == RES_ಒಳ || ctx->w == RES_FOR) {
					syntax();
					ಬಿಡುಗಡೆ(new);
					ctx->w = RES_SNTX;
					b_reset(dest);
					ಹಿಂತಿರುಗಿ 1;
				}
				*new = *ctx;   /* physical copy */
				initialize_context(ctx);
				ctx->ರಾಶಿ=new;
			} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ( ctx->w == RES_NONE || ! (ctx->old_flag & (1<<r->ಸಂಕೇತ_2))) {
				syntax();
				ctx->w = RES_SNTX;
				b_reset(dest);
				ಹಿಂತಿರುಗಿ 1;
			}
			ctx->w=r->ಸಂಕೇತ_2;
			ctx->old_flag = r->flag;
			ಆದರೆ (ctx->old_flag & FLAG_ಕೊನೆ) {
				ರಚನೆ p_context *old;
				debug_printf("pop stack\n");
				ಮುಗಿದ_pipe(ctx,PIPE_SEQ);
				old = ctx->ರಾಶಿ;
				old->ಮಗು->ಗುಂಪು = ctx->list_ತಲೆ;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
				old->ಮಗು->subshell = 0;
#ಆದರೆ_ಅಂತ್ಯ
				*ctx = *old;   /* physical copy */
				ಬಿಡುಗಡೆ(old);
			}
			b_reset (dest);
			ಹಿಂತಿರುಗಿ 1;
		}
	}
	ಹಿಂತಿರುಗಿ 0;
}

/* normal return is 0.
 * Syntax or xglob errors return 1. */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಮುಗಿದ_ಶಬ್ದ(o_string *dest, ರಚನೆ p_context *ctx)
{
	ರಚನೆ ಮಗು_prog *ಮಗು=ctx->ಮಗು;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	glob_t *glob_ಗುರಿ;
	ಪೂರ್ಣಾಂಕ gr, flags = 0;
#ಇಲ್ಲದಿದ್ದರೆ
	ಅಕ್ಷರ *str, *s;
	ಪೂರ್ಣಾಂಕ argc, cnt;
#ಆದರೆ_ಅಂತ್ಯ

	debug_printf("done_word: %s %p\n", dest->ದತ್ತಾಂಶ, ಮಗು);
	ಆದರೆ (dest->ಉದ್ದಳತೆ == 0 && !dest->nonnull) {
		debug_printf("  true null, ignored\n");
		ಹಿಂತಿರುಗಿ 0;
	}
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	ಆದರೆ (ctx->pending_redirect) {
		glob_ಗುರಿ = &ctx->pending_redirect->ಶಬ್ದ;
	} ಇಲ್ಲದಿದ್ದರೆ {
#ಆದರೆ_ಅಂತ್ಯ
		ಆದರೆ (ಮಗು->ಗುಂಪು) {
			syntax();
			ಹಿಂತಿರುಗಿ 1;  /* syntax error, groups and arglists don't mix */
		}
		ಆದರೆ (!ಮಗು->argv && (ctx->type & FLAG_PARSE_SEMICOLON)) {
			debug_printf("checking %s for reserved-ness\n",dest->ದತ್ತಾಂಶ);
			ಆದರೆ (reserved_ಶಬ್ದ(dest,ctx)) ಹಿಂತಿರುಗಿ ctx->w==RES_SNTX;
		}
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
		glob_ಗುರಿ = &ಮಗು->glob_ಫಲಿತಾಂಶ;
		ಆದರೆ (ಮಗು->argv) flags |= GLOB_APPEND;
#ಇಲ್ಲದಿದ್ದರೆ
		ಗಾಗಿ (cnt = 1, s = dest->ದತ್ತಾಂಶ; s && *s; s++) {
			ಆದರೆ (*s == '\\') s++;
			cnt++;
		}
		str = ಸ್ಮರಣೆ_ಕೊಡು(cnt);
		ಆದರೆ (!str) ಹಿಂತಿರುಗಿ 1;
		ಆದರೆ ( ಮಗು->argv == ಖಾಲಿ) {
			ಮಗು->argc=0;
		}
		argc = ++ಮಗು->argc;
		ಮಗು->argv = realloc(ಮಗು->argv, (argc+1)*ಗಾತ್ರ(*ಮಗು->argv));
		ಆದರೆ (ಮಗು->argv == ಖಾಲಿ) {
			ಬಿಡುಗಡೆ(str);
			ಹಿಂತಿರುಗಿ 1;
		}
		ಮಗು->argv_nonnull = realloc(ಮಗು->argv_nonnull,
					(argc+1)*ಗಾತ್ರ(*ಮಗು->argv_nonnull));
		ಆದರೆ (ಮಗು->argv_nonnull == ಖಾಲಿ) {
			ಬಿಡುಗಡೆ(str);
			ಹಿಂತಿರುಗಿ 1;
		}
		ಮಗು->argv[argc-1]=str;
		ಮಗು->argv_nonnull[argc-1] = dest->nonnull;
		ಮಗು->argv[argc]=ಖಾಲಿ;
		ಮಗು->argv_nonnull[argc] = 0;
		ಗಾಗಿ (s = dest->ದತ್ತಾಂಶ; s && *s; s++,str++) {
			ಆದರೆ (*s == '\\') s++;
			*str = *s;
		}
		*str = '\0';
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	}
	gr = xglob(dest, flags, glob_ಗುರಿ);
	ಆದರೆ (gr != 0) ಹಿಂತಿರುಗಿ 1;
#ಆದರೆ_ಅಂತ್ಯ

	b_reset(dest);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	ಆದರೆ (ctx->pending_redirect) {
		ctx->pending_redirect=ಖಾಲಿ;
		ಆದರೆ (glob_ಗುರಿ->gl_pathc != 1) {
			error_ಸಂದೇಶ("ambiguous redirect");
			ಹಿಂತಿರುಗಿ 1;
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		ಮಗು->argv = glob_ಗುರಿ->gl_pathv;
	}
#ಆದರೆ_ಅಂತ್ಯ
	ಆದರೆ (ctx->w == RES_FOR) {
		ಮುಗಿದ_ಶಬ್ದ(dest,ctx);
		ಮುಗಿದ_pipe(ctx,PIPE_SEQ);
	}
	ಹಿಂತಿರುಗಿ 0;
}

/* The only possible error here is out of memory, in which case
 * xmalloc exits. */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಮುಗಿದ_command(ರಚನೆ p_context *ctx)
{
	/* The child is really already in the pipe structure, so
	 * advance the pipe counter and make a new, null child.
	 * Only real trickiness here is that the uncommitted
	 * child structure, to which ctx->child points, is not
	 * counted in pi->num_progs. */
	ರಚನೆ pipe *pi=ctx->pipe;
	ರಚನೆ ಮಗು_prog *prog=ctx->ಮಗು;

	ಆದರೆ (prog && prog->ಗುಂಪು == ಖಾಲಿ
		 && prog->argv == ಖಾಲಿ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
		 && prog->redirects == ಖಾಲಿ) {
#ಇಲ್ಲದಿದ್ದರೆ
										) {
#ಆದರೆ_ಅಂತ್ಯ
		debug_printf("done_command: skipping null command\n");
		ಹಿಂತಿರುಗಿ 0;
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (prog) {
		pi->ಅಂಕೆ_progs++;
		debug_printf("done_command: num_progs incremented to %d\n",pi->ಅಂಕೆ_progs);
	} ಇಲ್ಲದಿದ್ದರೆ {
		debug_printf("done_command: initializing\n");
	}
	pi->progs = xrealloc(pi->progs, ಗಾತ್ರ(*pi->progs) * (pi->ಅಂಕೆ_progs+1));

	prog = pi->progs + pi->ಅಂಕೆ_progs;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	prog->redirects = ಖಾಲಿ;
#ಆದರೆ_ಅಂತ್ಯ
	prog->argv = ಖಾಲಿ;
	prog->argv_nonnull = ಖಾಲಿ;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	prog->is_stopped = 0;
#ಆದರೆ_ಅಂತ್ಯ
	prog->ಗುಂಪು = ಖಾಲಿ;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	prog->glob_ಫಲಿತಾಂಶ.gl_pathv = ಖಾಲಿ;
	prog->family = pi;
#ಆದರೆ_ಅಂತ್ಯ
	prog->sp = 0;
	ctx->ಮಗು = prog;
	prog->type = ctx->type;

	/* but ctx->pipe and ctx->list_head remain unchanged */
	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ ಮುಗಿದ_pipe(ರಚನೆ p_context *ctx, pipe_style type)
{
	ರಚನೆ pipe *new_p;
	ಮುಗಿದ_command(ctx);  /* implicit closure of previous command */
	debug_printf("done_pipe, type %d\n", type);
	ctx->pipe->followup = type;
	ctx->pipe->r_ಬಹುಲಕ = ctx->w;
	new_p=new_pipe();
	ctx->pipe->ಮುಂದಿನ = new_p;
	ctx->pipe = new_p;
	ctx->ಮಗು = ಖಾಲಿ;
	ಮುಗಿದ_command(ctx);  /* set up new pipe to accept commands */
	ಹಿಂತಿರುಗಿ 0;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
/* peek ahead in the in_str to find out if we have a "&n" construct,
 * as in "2>&1", that represents duplicating a file descriptor.
 * returns either -2 (syntax error), -1 (no &), or the number found.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ redirect_dup_ಅಂಕೆ(ರಚನೆ ಒಳ_str *ಒಳಹರಿವು)
{
	ಪೂರ್ಣಾಂಕ ch, d=0, ok=0;
	ch = b_peek(ಒಳಹರಿವು);
	ಆದರೆ (ch != '&') ಹಿಂತಿರುಗಿ -1;

	b_getch(ಒಳಹರಿವು);  /* get the & */
	ch=b_peek(ಒಳಹರಿವು);
	ಆದರೆ (ch == '-') {
		b_getch(ಒಳಹರಿವು);
		ಹಿಂತಿರುಗಿ -3;  /* "-" represents "close me" */
	}
	ಆಗ (ಅಂಕಿಯೇ(ch)) {
		d = d*10+(ch-'0');
		ok=1;
		b_getch(ಒಳಹರಿವು);
		ch = b_peek(ಒಳಹರಿವು);
	}
	ಆದರೆ (ok) ಹಿಂತಿರುಗಿ d;

	error_ಸಂದೇಶ("ambiguous redirect");
	ಹಿಂತಿರುಗಿ -2;
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ redirect_opt_ಅಂಕೆ(o_string *o)
{
	ಪೂರ್ಣಾಂಕ ಅಂಕೆ;

	ಆದರೆ (o->ಉದ್ದಳತೆ==0) ಹಿಂತಿರುಗಿ -1;
	ಗಾಗಿ(ಅಂಕೆ=0; ಅಂಕೆ<o->ಉದ್ದಳತೆ; ಅಂಕೆ++) {
		ಆದರೆ (!ಅಂಕಿಯೇ(*(o->ದತ್ತಾಂಶ+ಅಂಕೆ))) {
			ಹಿಂತಿರುಗಿ -1;
		}
	}
	/* reuse num (and save an int) */
	ಅಂಕೆ=atoi(o->ದತ್ತಾಂಶ);
	b_reset(o);
	ಹಿಂತಿರುಗಿ ಅಂಕೆ;
}

FILE *generate_ಹರಿವು_from_list(ರಚನೆ pipe *ತಲೆ)
{
	FILE *pf;
#ಆದರೆ 1
	ಪೂರ್ಣಾಂಕ pid, channel[2];
	ಆದರೆ (pipe(channel)<0) perror_ಸಂದೇಶ_and_die("pipe");
	pid=fork();
	ಆದರೆ (pid<0) {
		perror_ಸಂದೇಶ_and_die("fork");
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (pid==0) {
		close(channel[0]);
		ಆದರೆ (channel[1] != 1) {
			dup2(channel[1],1);
			close(channel[1]);
		}
#ಆದರೆ 0
#ವ್ಯಾಖ್ಯಾನಿಸು SURROGATE "surrogate response"
		ಲಿಖ(1,SURROGATE,ಗಾತ್ರ(SURROGATE));
		_exit(ಸತತ_list(ತಲೆ));
#ಇಲ್ಲದಿದ್ದರೆ
		_exit(ಸತತ_list_real(ತಲೆ));   /* leaks memory */
#ಆದರೆ_ಅಂತ್ಯ
	}
	debug_printf("forked child %d\n",pid);
	close(channel[1]);
	pf = fdopen(channel[0],"r");
	debug_printf("pipe on FILE *%p\n",pf);
#ಇಲ್ಲದಿದ್ದರೆ
	free_pipe_list(ತಲೆ,0);
	pf=popen("echo surrogate response","r");
	debug_printf("started fake pipe on FILE *%p\n",pf);
#ಆದರೆ_ಅಂತ್ಯ
	ಹಿಂತಿರುಗಿ pf;
}

/* this version hacked for testing purposes */
/* return code is exit status of the process that is run. */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ process_command_subs(o_string *dest, ರಚನೆ p_context *ctx, ರಚನೆ ಒಳ_str *ಒಳಹರಿವು, ಪೂರ್ಣಾಂಕ subst_ಕೊನೆ)
{
	ಪೂರ್ಣಾಂಕ retcode;
	o_string ಫಲಿತಾಂಶ=NULL_O_STRING;
	ರಚನೆ p_context inner;
	FILE *p;
	ರಚನೆ ಒಳ_str pipe_str;
	initialize_context(&inner);

	/* recursion to generate command */
	retcode = parse_ಹರಿವು(&ಫಲಿತಾಂಶ, &inner, ಒಳಹರಿವು, subst_ಕೊನೆ);
	ಆದರೆ (retcode != 0) ಹಿಂತಿರುಗಿ retcode;  /* syntax error or EOF */
	ಮುಗಿದ_ಶಬ್ದ(&ಫಲಿತಾಂಶ, &inner);
	ಮುಗಿದ_pipe(&inner, PIPE_SEQ);
	b_free(&ಫಲಿತಾಂಶ);

	p=generate_ಹರಿವು_from_list(inner.list_ತಲೆ);
	ಆದರೆ (p==ಖಾಲಿ) ಹಿಂತಿರುಗಿ 1;
	mark_open(fileno(p));
	ಸಿದ್ಧತೆ_file_ಒಳ_str(&pipe_str, p);

	/* now send results of command back into original context */
	retcode = parse_ಹರಿವು(dest, ctx, &pipe_str, '\0');
	/* XXX In case of a syntax error, should we try to kill the child?
	 * That would be tough to do right, so just read until EOF. */
	ಆದರೆ (retcode == 1) {
		ಆಗ (b_getch(&pipe_str)!=EOF) { /* discard */ };
	}

	debug_printf("done reading from pipe, pclose()ing\n");
	/* This is the step that wait()s for the child.  Should be pretty
	 * safe, since we just read an EOF from its stdout.  We could try
	 * to better, by using wait(), and keeping track of background jobs
	 * at the same time.  That would be a lot of work, and contrary
	 * to the KISS philosophy of this program. */
	mark_closed(fileno(p));
	retcode=pclose(p);
	free_pipe_list(inner.list_ತಲೆ,0);
	debug_printf("pclosed, retcode=%d\n",retcode);
	/* XXX this process fails to trim a single trailing newline */
	ಹಿಂತಿರುಗಿ retcode;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ parse_ಗುಂಪು(o_string *dest, ರಚನೆ p_context *ctx,
	ರಚನೆ ಒಳ_str *ಒಳಹರಿವು, ಪೂರ್ಣಾಂಕ ch)
{
	ಪೂರ್ಣಾಂಕ rcode, endch=0;
	ರಚನೆ p_context sub;
	ರಚನೆ ಮಗು_prog *ಮಗು = ctx->ಮಗು;
	ಆದರೆ (ಮಗು->argv) {
		syntax();
		ಹಿಂತಿರುಗಿ 1;  /* syntax error, groups and arglists don't mix */
	}
	initialize_context(&sub);
	ಆಯ್ಕೆ(ch) {
		ಸ್ಥಿತಿ '(': endch=')'; ಮಗು->subshell=1; ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ '{': endch='}'; ನಿಲ್ಲಿಸು;
		ಪೂರ್ವನಿಯೋಜಿತ: syntax();   /* really logic error */
	}
	rcode=parse_ಹರಿವು(dest,&sub,ಒಳಹರಿವು,endch);
	ಮುಗಿದ_ಶಬ್ದ(dest,&sub); /* finish off the final word in the subcontext */
	ಮುಗಿದ_pipe(&sub, PIPE_SEQ);  /* and the final command there, too */
	ಮಗು->ಗುಂಪು = sub.list_ತಲೆ;
	ಹಿಂತಿರುಗಿ rcode;
	/* child remains "open", available for possible redirects */
}
#ಆದರೆ_ಅಂತ್ಯ

/* basically useful version until someone wants to get fancier,
 * see the bash man page under "Parameter Expansion" */
ಸ್ಥಿರ ಅಕ್ಷರ *lookup_param(ಅಕ್ಷರ *src)
{
	ಅಕ್ಷರ *p;
	ಅಕ್ಷರ *sep;
	ಅಕ್ಷರ *default_val = ಖಾಲಿ;
	ಪೂರ್ಣಾಂಕ ನಿಯೋಜಿಸು = 0;
	ಪೂರ್ಣಾಂಕ expand_ಖಾಲಿಗಳು = 0;

	ಆದರೆ (!src)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	sep = strchr(src, ':');

	ಆದರೆ (sep) {
		*sep = '\0';
		ಆದರೆ (*(sep + 1) == '-')
			default_val = sep+2;
		ಆದರೆ (*(sep + 1) == '=') {
			default_val = sep+2;
			ನಿಯೋಜಿಸು = 1;
		}
		ಆದರೆ (*(sep + 1) == '+') {
			default_val = sep+2;
			expand_ಖಾಲಿಗಳು = 1;
		}
	}

	p = env_get(src);
	ಆದರೆ (!p)
		p = get_local_var(src);

	ಆದರೆ (!p || ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(p) == 0) {
		p = default_val;
		ಆದರೆ (ನಿಯೋಜಿಸು) {
			ಅಕ್ಷರ *var = ಸ್ಮರಣೆ_ಕೊಡು(ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(src)+ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(default_val)+2);
			ಆದರೆ (var) {
				sprintf(var, "%s=%s", src, default_val);
				ಗಣ_local_var(var, 0);
			}
			ಬಿಡುಗಡೆ(var);
		}
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (expand_ಖಾಲಿಗಳು) {
		p += ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(p);
	}

	ಆದರೆ (sep)
		*sep = ':';

	ಹಿಂತಿರುಗಿ p;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __U_BOOT__
ಸ್ಥಿರ ಅಕ್ಷರ *get_dollar_var(ಅಕ್ಷರ ch)
{
	ಸ್ಥಿರ ಅಕ್ಷರ buf[40];

	buf[0] = '\0';
	ಆಯ್ಕೆ (ch) {
		ಸ್ಥಿತಿ '?':
			sprintf(buf, "%u", (ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ)ಕೊನೆಯ_return_ಸಂಕೇತ);
			ನಿಲ್ಲಿಸು;
		ಪೂರ್ವನಿಯೋಜಿತ:
			ಹಿಂತಿರುಗಿ ಖಾಲಿ;
	}
	ಹಿಂತಿರುಗಿ buf;
}
#ಆದರೆ_ಅಂತ್ಯ

/* return code: 0 for OK, 1 for syntax error */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ handle_dollar(o_string *dest, ರಚನೆ p_context *ctx, ರಚನೆ ಒಳ_str *ಒಳಹರಿವು)
{
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	ಪೂರ್ಣಾಂಕ i, advance=0;
#ಇಲ್ಲದಿದ್ದರೆ
	ಪೂರ್ಣಾಂಕ advance=0;
#ಆದರೆ_ಅಂತ್ಯ
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	ಅಕ್ಷರ sep[]=" ";
#ಆದರೆ_ಅಂತ್ಯ
	ಪೂರ್ಣಾಂಕ ch = ಒಳಹರಿವು->peek(ಒಳಹರಿವು);  /* first character after the $ */
	debug_printf("handle_dollar: ch=%c\n",ch);
	ಆದರೆ (ಅಕ್ಷರವೇ(ch)) {
		b_addchr(dest, SPECIAL_VAR_ಚಿಹ್ನೆ);
		ctx->ಮಗು->sp++;
		ಆಗ(ch=b_peek(ಒಳಹರಿವು),ಅಕ್ಷರವೇ_ಅಂಕಿಯೇ(ch) || ch=='_') {
			b_getch(ಒಳಹರಿವು);
			b_addchr(dest,ch);
		}
		b_addchr(dest, SPECIAL_VAR_ಚಿಹ್ನೆ);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಅಂಕಿಯೇ(ch)) {
		i = ch-'0';  /* XXX is $0 special? */
		ಆದರೆ (i<global_argc) {
			parse_string(dest, ctx, global_argv[i]); /* recursion */
		}
		advance = 1;
#ಆದರೆ_ಅಂತ್ಯ
	} ಇಲ್ಲದಿದ್ದರೆ ಆಯ್ಕೆ (ch) {
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
		ಸ್ಥಿತಿ '$':
			b_adduint(dest,getpid());
			advance = 1;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ '!':
			ಆದರೆ (ಕೊನೆಯ_bg_pid > 0) b_adduint(dest, ಕೊನೆಯ_bg_pid);
			advance = 1;
			ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ
		ಸ್ಥಿತಿ '?':
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
			b_adduint(dest,ಕೊನೆಯ_return_ಸಂಕೇತ);
#ಇಲ್ಲದಿದ್ದರೆ
			ctx->ಮಗು->sp++;
			b_addchr(dest, SPECIAL_VAR_ಚಿಹ್ನೆ);
			b_addchr(dest, '$');
			b_addchr(dest, '?');
			b_addchr(dest, SPECIAL_VAR_ಚಿಹ್ನೆ);
#ಆದರೆ_ಅಂತ್ಯ
			advance = 1;
			ನಿಲ್ಲಿಸು;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
		ಸ್ಥಿತಿ '#':
			b_adduint(dest,global_argc ? global_argc-1 : 0);
			advance = 1;
			ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ
		ಸ್ಥಿತಿ '{':
			b_addchr(dest, SPECIAL_VAR_ಚಿಹ್ನೆ);
			ctx->ಮಗು->sp++;
			b_getch(ಒಳಹರಿವು);
			/* XXX maybe someone will try to escape the '}' */
			ಆಗ(ch=b_getch(ಒಳಹರಿವು),ch!=EOF && ch!='}') {
				b_addchr(dest,ch);
			}
			ಆದರೆ (ch != '}') {
				syntax();
				ಹಿಂತಿರುಗಿ 1;
			}
			b_addchr(dest, SPECIAL_VAR_ಚಿಹ್ನೆ);
			ನಿಲ್ಲಿಸು;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
		ಸ್ಥಿತಿ '(':
			b_getch(ಒಳಹರಿವು);
			process_command_subs(dest, ctx, ಒಳಹರಿವು, ')');
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ '*':
			sep[0]=ifs[0];
			ಗಾಗಿ (i=1; i<global_argc; i++) {
				parse_string(dest, ctx, global_argv[i]);
				ಆದರೆ (i+1 < global_argc) parse_string(dest, ctx, sep);
			}
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ '@':
		ಸ್ಥಿತಿ '-':
		ಸ್ಥಿತಿ '_':
			/* still unhandled, but should be eventually */
			error_ಸಂದೇಶ("unhandled syntax: $%c",ch);
			ಹಿಂತಿರುಗಿ 1;
			ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ
		ಪೂರ್ವನಿಯೋಜಿತ:
			b_addqchr(dest,'$',dest->quote);
	}
	/* Eat the character if the flag was set.  If the compiler
	 * is smart enough, we could substitute "b_getch(input);"
	 * for all the "advance = 1;" above, and also end up with
	 * a nice size-optimized program.  Hah!  That'll be the day.
	 */
	ಆದರೆ (advance) b_getch(ಒಳಹರಿವು);
	ಹಿಂತಿರುಗಿ 0;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
ಪೂರ್ಣಾಂಕ parse_string(o_string *dest, ರಚನೆ p_context *ctx, ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *src)
{
	ರಚನೆ ಒಳ_str foo;
	ಸಿದ್ಧತೆ_string_ಒಳ_str(&foo, src);
	ಹಿಂತಿರುಗಿ parse_ಹರಿವು(dest, ctx, &foo, '\0');
}
#ಆದರೆ_ಅಂತ್ಯ

/* return code is 0 for normal exit, 1 for syntax error */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ parse_ಹರಿವು(o_string *dest, ರಚನೆ p_context *ctx,
			ರಚನೆ ಒಳ_str *ಒಳಹರಿವು, ಪೂರ್ಣಾಂಕ ಕೊನೆ_trigger)
{
	ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ ch, m;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	ಪೂರ್ಣಾಂಕ redir_fd;
	redir_type redir_style;
#ಆದರೆ_ಅಂತ್ಯ
	ಪೂರ್ಣಾಂಕ ಮುಂದಿನ;

	/* Only double-quote state is handled in the state variable dest->quote.
	 * A single-quote triggers a bypass of the main loop until its mate is
	 * found.  When recursing, quote state is passed in via dest->quote. */

	debug_printf("parse_stream, end_trigger=%d\n",ಕೊನೆ_trigger);
	ಆಗ ((ch=b_getch(ಒಳಹರಿವು))!=EOF) {
		m = map[ch];
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __U_BOOT__
		ಆದರೆ (ಒಳಹರಿವು->__promptme == 0) ಹಿಂತಿರುಗಿ 1;
#ಆದರೆ_ಅಂತ್ಯ
		ಮುಂದಿನ = (ch == '\n') ? 0 : b_peek(ಒಳಹರಿವು);

		debug_printf("parse_stream: ch=%c (%d) m=%d quote=%d - %c\n",
			ch >= ' ' ? ch : '.', ch, m,
			dest->quote, ctx->ರಾಶಿ == ಖಾಲಿ ? '*' : '.');

		ಆದರೆ (m==0 || ((m==1 || m==2) && dest->quote)) {
			b_addqchr(dest, ch, dest->quote);
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಆದರೆ (m==2) {  /* unquoted IFS */
				ಆದರೆ (ಮುಗಿದ_ಶಬ್ದ(dest, ctx)) {
					ಹಿಂತಿರುಗಿ 1;
				}
				/* If we aren't performing a substitution, treat a newline as a
				 * command separator.  */
				ಆದರೆ (ಕೊನೆ_trigger != '\0' && ch=='\n')
					ಮುಗಿದ_pipe(ctx,PIPE_SEQ);
			}
			ಆದರೆ (ch == ಕೊನೆ_trigger && !dest->quote && ctx->w==RES_NONE) {
				debug_printf("leaving parse_stream (triggered)\n");
				ಹಿಂತಿರುಗಿ 0;
			}
#ಆದರೆ 0
			ಆದರೆ (ch=='\n') {
				/* Yahoo!  Time to run with it! */
				ಮುಗಿದ_pipe(ctx,PIPE_SEQ);
				ಸತತ_list(ctx->list_ತಲೆ);
				initialize_context(ctx);
			}
#ಆದರೆ_ಅಂತ್ಯ
			ಆದರೆ (m!=2) ಆಯ್ಕೆ (ch) {
		ಸ್ಥಿತಿ '#':
			ಆದರೆ (dest->ಉದ್ದಳತೆ == 0 && !dest->quote) {
				ಆಗ(ch=b_peek(ಒಳಹರಿವು),ch!=EOF && ch!='\n') { b_getch(ಒಳಹರಿವು); }
			} ಇಲ್ಲದಿದ್ದರೆ {
				b_addqchr(dest, ch, dest->quote);
			}
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ '\\':
			ಆದರೆ (ಮುಂದಿನ == EOF) {
				syntax();
				ಹಿಂತಿರುಗಿ 1;
			}
			b_addqchr(dest, '\\', dest->quote);
			b_addqchr(dest, b_getch(ಒಳಹರಿವು), dest->quote);
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ '$':
			ಆದರೆ (handle_dollar(dest, ctx, ಒಳಹರಿವು)!=0) ಹಿಂತಿರುಗಿ 1;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ '\'':
			dest->nonnull = 1;
			ಆಗ(ch=b_getch(ಒಳಹರಿವು),ch!=EOF && ch!='\'') {
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __U_BOOT__
				ಆದರೆ(ಒಳಹರಿವು->__promptme == 0) ಹಿಂತಿರುಗಿ 1;
#ಆದರೆ_ಅಂತ್ಯ
				b_addchr(dest,ch);
			}
			ಆದರೆ (ch==EOF) {
				syntax();
				ಹಿಂತಿರುಗಿ 1;
			}
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ '"':
			dest->nonnull = 1;
			dest->quote = !dest->quote;
			ನಿಲ್ಲಿಸು;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
		ಸ್ಥಿತಿ '`':
			process_command_subs(dest, ctx, ಒಳಹರಿವು, '`');
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ '>':
			redir_fd = redirect_opt_ಅಂಕೆ(dest);
			ಮುಗಿದ_ಶಬ್ದ(dest, ctx);
			redir_style=REDIRECT_OVERWRITE;
			ಆದರೆ (ಮುಂದಿನ == '>') {
				redir_style=REDIRECT_APPEND;
				b_getch(ಒಳಹರಿವು);
			} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಮುಂದಿನ == '(') {
				syntax();   /* until we support >(list) Process Substitution */
				ಹಿಂತಿರುಗಿ 1;
			}
			ಸಿದ್ಧತೆ_redirect(ctx, redir_fd, redir_style, ಒಳಹರಿವು);
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ '<':
			redir_fd = redirect_opt_ಅಂಕೆ(dest);
			ಮುಗಿದ_ಶಬ್ದ(dest, ctx);
			redir_style=REDIRECT_ಒಳಹರಿವು;
			ಆದರೆ (ಮುಂದಿನ == '<') {
				redir_style=REDIRECT_HEREIS;
				b_getch(ಒಳಹರಿವು);
			} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಮುಂದಿನ == '>') {
				redir_style=REDIRECT_IO;
				b_getch(ಒಳಹರಿವು);
			} ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (ಮುಂದಿನ == '(') {
				syntax();   /* until we support <(list) Process Substitution */
				ಹಿಂತಿರುಗಿ 1;
			}
			ಸಿದ್ಧತೆ_redirect(ctx, redir_fd, redir_style, ಒಳಹರಿವು);
			ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ
		ಸ್ಥಿತಿ ';':
			ಮುಗಿದ_ಶಬ್ದ(dest, ctx);
			ಮುಗಿದ_pipe(ctx,PIPE_SEQ);
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ '&':
			ಮುಗಿದ_ಶಬ್ದ(dest, ctx);
			ಆದರೆ (ಮುಂದಿನ=='&') {
				b_getch(ಒಳಹರಿವು);
				ಮುಗಿದ_pipe(ctx,PIPE_AND);
			} ಇಲ್ಲದಿದ್ದರೆ {
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
				ಮುಗಿದ_pipe(ctx,PIPE_BG);
#ಇಲ್ಲದಿದ್ದರೆ
				syntax_err();
				ಹಿಂತಿರುಗಿ 1;
#ಆದರೆ_ಅಂತ್ಯ
			}
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ '|':
			ಮುಗಿದ_ಶಬ್ದ(dest, ctx);
			ಆದರೆ (ಮುಂದಿನ=='|') {
				b_getch(ಒಳಹರಿವು);
				ಮುಗಿದ_pipe(ctx,PIPE_OR);
			} ಇಲ್ಲದಿದ್ದರೆ {
				/* we could pick up a file descriptor choice here
				 * with redirect_opt_num(), but bash doesn't do it.
				 * "echo foo 2| cat" yields "foo 2". */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
				ಮುಗಿದ_command(ctx);
#ಇಲ್ಲದಿದ್ದರೆ
				syntax_err();
				ಹಿಂತಿರುಗಿ 1;
#ಆದರೆ_ಅಂತ್ಯ
			}
			ನಿಲ್ಲಿಸು;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
		ಸ್ಥಿತಿ '(':
		ಸ್ಥಿತಿ '{':
			ಆದರೆ (parse_ಗುಂಪು(dest, ctx, ಒಳಹರಿವು, ch)!=0) ಹಿಂತಿರುಗಿ 1;
			ನಿಲ್ಲಿಸು;
		ಸ್ಥಿತಿ ')':
		ಸ್ಥಿತಿ '}':
			syntax();   /* Proper use of this character caught by end_trigger */
			ಹಿಂತಿರುಗಿ 1;
			ನಿಲ್ಲಿಸು;
#ಆದರೆ_ಅಂತ್ಯ
		ಸ್ಥಿತಿ SUBSTED_VAR_ಚಿಹ್ನೆ:
			dest->nonnull = 1;
			ಆಗ (ch = b_getch(ಒಳಹರಿವು), ch != EOF &&
			    ch != SUBSTED_VAR_ಚಿಹ್ನೆ) {
				debug_printf("subst, pass=%d\n", ch);
				ಆದರೆ (ಒಳಹರಿವು->__promptme == 0)
					ಹಿಂತಿರುಗಿ 1;
				b_addchr(dest, ch);
			}
			debug_printf("subst, term=%d\n", ch);
			ಆದರೆ (ch == EOF) {
				syntax();
				ಹಿಂತಿರುಗಿ 1;
			}
			ನಿಲ್ಲಿಸು;
		ಪೂರ್ವನಿಯೋಜಿತ:
			syntax();   /* this is really an internal logic error */
			ಹಿಂತಿರುಗಿ 1;
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
	ಆದರೆ (ಕೊನೆ_trigger != '\0') ಹಿಂತಿರುಗಿ -1;
	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಶೂನ್ಯ mapset(ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *ಗಣ, ಪೂರ್ಣಾಂಕ ಸಂಕೇತ_2)
{
	ಸ್ಥಿರಾಂಕ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *s;
	ಗಾಗಿ (s=ಗಣ; *s; s++) map[*s] = ಸಂಕೇತ_2;
}

ಸ್ಥಿರ ಶೂನ್ಯ update_ifs_map(ಶೂನ್ಯ)
{
	/* char *ifs and char map[256] are both globals. */
	ifs = (uchar *)env_get("IFS");
	ಆದರೆ (ifs == ಖಾಲಿ) ifs=(uchar *)" \t\n";
	/* Precompute a list of 'flow through' behavior so it can be treated
	 * quickly up front.  Computation is necessary because of IFS.
	 * Special case handling of IFS == " \t\n" is not implemented.
	 * The map[] array only really needs two bits each, and on most machines
	 * that would be faster because of the reduced L1 cache footprint.
	 */
	memset(map,0,ಗಾತ್ರ(map)); /* most characters flow through always */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	mapset((uchar *)"\\$'\"`", 3);      /* never flow through */
	mapset((uchar *)"<>;&|(){}#", 1);   /* flow through if quoted */
#ಇಲ್ಲದಿದ್ದರೆ
	{
		uchar subst[2] = {SUBSTED_VAR_ಚಿಹ್ನೆ, 0};
		mapset(subst, 3);       /* never flow through */
	}
	mapset((uchar *)"\\$'\"", 3);       /* never flow through */
	mapset((uchar *)";&|#", 1);         /* flow through if quoted */
#ಆದರೆ_ಅಂತ್ಯ
	mapset(ifs, 2);            /* also flow through if quoted */
}

/* most recursion does not come through here, the exeception is
 * from builtin_source() */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ parse_ಹರಿವು_outer(ರಚನೆ ಒಳ_str *inp, ಪೂರ್ಣಾಂಕ flag)
{

	ರಚನೆ p_context ctx;
	o_string ತಾತ್ಕಾಲಿಕ=NULL_O_STRING;
	ಪೂರ್ಣಾಂಕ rcode;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __U_BOOT__
	ಪೂರ್ಣಾಂಕ ಸಂಕೇತ_2 = 1;
#ಆದರೆ_ಅಂತ್ಯ
	ಮಾಡು {
		ctx.type = flag;
		initialize_context(&ctx);
		update_ifs_map();
		ಆದರೆ (!(flag & FLAG_PARSE_SEMICOLON) || (flag & FLAG_REPARSING)) mapset((uchar *)";$&|", 0);
		inp->promptmode=1;
		rcode = parse_ಹರಿವು(&ತಾತ್ಕಾಲಿಕ, &ctx, inp,
				     flag & FLAG_CONT_ON_NEWLINE ? -1 : '\n');
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __U_BOOT__
		ಆದರೆ (rcode == 1) flag_repeat = 0;
#ಆದರೆ_ಅಂತ್ಯ
		ಆದರೆ (rcode != 1 && ctx.old_flag != 0) {
			syntax();
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __U_BOOT__
			flag_repeat = 0;
#ಆದರೆ_ಅಂತ್ಯ
		}
		ಆದರೆ (rcode != 1 && ctx.old_flag == 0) {
			ಮುಗಿದ_ಶಬ್ದ(&ತಾತ್ಕಾಲಿಕ, &ctx);
			ಮುಗಿದ_pipe(&ctx,PIPE_SEQ);
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
			ಸತತ_list(ctx.list_ತಲೆ);
#ಇಲ್ಲದಿದ್ದರೆ
			ಸಂಕೇತ_2 = ಸತತ_list(ctx.list_ತಲೆ);
			ಆದರೆ (ಸಂಕೇತ_2 == -2) {	/* exit */
				b_free(&ತಾತ್ಕಾಲಿಕ);
				ಸಂಕೇತ_2 = 0;
				/* XXX hackish way to not allow exit from main loop */
				ಆದರೆ (inp->peek == file_peek) {
					ಬರೆ("exit not allowed from main input shell.\n");
					ಮುಂದುವರಿ;
				}
				/*
				 * DANGER
				 * Return code -2 is special in this context,
				 * it indicates exit from inner pipe instead
				 * of return code itself, the return code is
				 * stored in 'last_return_code' variable!
				 * DANGER
				 */
				ಹಿಂತಿರುಗಿ -2;
			}
			ಆದರೆ (ಸಂಕೇತ_2 == -1)
			    flag_repeat = 0;
#ಆದರೆ_ಅಂತ್ಯ
		} ಇಲ್ಲದಿದ್ದರೆ {
			ಆದರೆ (ctx.old_flag != 0) {
				ಬಿಡುಗಡೆ(ctx.ರಾಶಿ);
				b_reset(&ತಾತ್ಕಾಲಿಕ);
			}
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __U_BOOT__
			ಆದರೆ (inp->__promptme == 0) ಬರೆ("<INTERRUPT>\n");
			inp->__promptme = 1;
#ಆದರೆ_ಅಂತ್ಯ
			ತಾತ್ಕಾಲಿಕ.nonnull = 0;
			ತಾತ್ಕಾಲಿಕ.quote = 0;
			inp->p = ಖಾಲಿ;
			free_pipe_list(ctx.list_ತಲೆ,0);
		}
		b_free(&ತಾತ್ಕಾಲಿಕ);
	/* loop on syntax errors, return on EOF */
	} ಆಗ (rcode != -1 && !(flag & FLAG_EXIT_FROM_LOOP) &&
		(inp->peek != static_peek || b_peek(inp)));
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	ಹಿಂತಿರುಗಿ 0;
#ಇಲ್ಲದಿದ್ದರೆ
	ಹಿಂತಿರುಗಿ (ಸಂಕೇತ_2 != 0) ? 1 : 0;
#ಆದರೆ_ಅಂತ್ಯ /* __U_BOOT__ */
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ parse_string_outer(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s, ಪೂರ್ಣಾಂಕ flag)
#ಇಲ್ಲದಿದ್ದರೆ
ಪೂರ್ಣಾಂಕ parse_string_outer(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *s, ಪೂರ್ಣಾಂಕ flag)
#ಆದರೆ_ಅಂತ್ಯ	/* __U_BOOT__ */
{
	ರಚನೆ ಒಳ_str ಒಳಹರಿವು;
	ಪೂರ್ಣಾಂಕ rcode;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __U_BOOT__
	ಅಕ್ಷರ *p = ಖಾಲಿ;
	ಆದರೆ (!s)
		ಹಿಂತಿರುಗಿ 1;
	ಆದರೆ (!*s)
		ಹಿಂತಿರುಗಿ 0;
	ಆದರೆ (!(p = strchr(s, '\n')) || *++p) {
		p = xmalloc(ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(s) + 2);
		strcpy(p, s);
		strcat(p, "\n");
		ಸಿದ್ಧತೆ_string_ಒಳ_str(&ಒಳಹರಿವು, p);
		rcode = parse_ಹರಿವು_outer(&ಒಳಹರಿವು, flag);
		ಬಿಡುಗಡೆ(p);
		ಹಿಂತಿರುಗಿ rcode == -2 ? ಕೊನೆಯ_return_ಸಂಕೇತ : rcode;
	} ಇಲ್ಲದಿದ್ದರೆ {
#ಆದರೆ_ಅಂತ್ಯ
	ಸಿದ್ಧತೆ_string_ಒಳ_str(&ಒಳಹರಿವು, s);
	rcode = parse_ಹರಿವು_outer(&ಒಳಹರಿವು, flag);
	ಹಿಂತಿರುಗಿ rcode == -2 ? ಕೊನೆಯ_return_ಸಂಕೇತ : rcode;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __U_BOOT__
	}
#ಆದರೆ_ಅಂತ್ಯ
}

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ parse_file_outer(FILE *f)
#ಇಲ್ಲದಿದ್ದರೆ
ಪೂರ್ಣಾಂಕ parse_file_outer(ಶೂನ್ಯ)
#ಆದರೆ_ಅಂತ್ಯ
{
	ಪೂರ್ಣಾಂಕ rcode;
	ರಚನೆ ಒಳ_str ಒಳಹರಿವು;
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
	ಸಿದ್ಧತೆ_file_ಒಳ_str(&ಒಳಹರಿವು, f);
#ಇಲ್ಲದಿದ್ದರೆ
	ಸಿದ್ಧತೆ_file_ಒಳ_str(&ಒಳಹರಿವು);
#ಆದರೆ_ಅಂತ್ಯ
	rcode = parse_ಹರಿವು_outer(&ಒಳಹರಿವು, FLAG_PARSE_SEMICOLON);
	ಹಿಂತಿರುಗಿ rcode == -2 ? ಕೊನೆಯ_return_ಸಂಕೇತ : rcode;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __U_BOOT__
ಪೂರ್ಣಾಂಕ u_boot_hush_ಆರಂಭ(ಶೂನ್ಯ)
{
	ಆದರೆ (ತುದಿ_vars == ಖಾಲಿ) {
		ತುದಿ_vars = ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ(ರಚನೆ variables));
		ತುದಿ_vars->name = "HUSH_VERSION";
		ತುದಿ_vars->ಮೌಲ್ಯ = "0.01";
		ತುದಿ_vars->ಮುಂದಿನ = ಖಾಲಿ;
		ತುದಿ_vars->flg_export = 0;
		ತುದಿ_vars->flg_read_only = 1;
	}
	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಶೂನ್ಯ *xmalloc(size_t ಗಾತ್ರ_2)
{
	ಶೂನ್ಯ *p = ಖಾಲಿ;

	ಆದರೆ (!(p = ಸ್ಮರಣೆ_ಕೊಡು(ಗಾತ್ರ_2))) {
	    ಬರೆ("ERROR : xmalloc failed\n");
	    ಗಾಗಿ(;;);
	}
	ಹಿಂತಿರುಗಿ p;
}

ಸ್ಥಿರ ಶೂನ್ಯ *xrealloc(ಶೂನ್ಯ *ptr, size_t ಗಾತ್ರ_2)
{
	ಶೂನ್ಯ *p = ಖಾಲಿ;

	ಆದರೆ (!(p = realloc(ptr, ಗಾತ್ರ_2))) {
	    ಬರೆ("ERROR : xrealloc failed\n");
	    ಗಾಗಿ(;;);
	}
	ಹಿಂತಿರುಗಿ p;
}
#ಆದರೆ_ಅಂತ್ಯ /* __U_BOOT__ */

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ __U_BOOT__
/* Make sure we have a controlling tty.  If we get started under a job
 * aware app (like bash for example), make sure we are now in charge so
 * we don't fight over who gets the foreground */
ಸ್ಥಿರ ಶೂನ್ಯ ಸಿದ್ಧತೆ_ಕೆಲಸ_control(ಶೂನ್ಯ)
{
	ಸ್ಥಿರ pid_t shell_pgrp;
	/* Loop until we are in the foreground.  */
	ಆಗ (tcgetpgrp (shell_terminal) != (shell_pgrp = getpgrp ()))
		kill (- shell_pgrp, SIGTTIN);

	/* Ignore interactive and job-control signals.  */
	ಸಂಕೇತ(SIGINT, SIG_IGN);
	ಸಂಕೇತ(SIGQUIT, SIG_IGN);
	ಸಂಕೇತ(SIGTERM, SIG_IGN);
	ಸಂಕೇತ(SIGTSTP, SIG_IGN);
	ಸಂಕೇತ(SIGTTIN, SIG_IGN);
	ಸಂಕೇತ(SIGTTOU, SIG_IGN);
	ಸಂಕೇತ(SIGCHLD, SIG_IGN);

	/* Put ourselves in our own process group.  */
	setsid();
	shell_pgrp = getpid ();
	setpgid (shell_pgrp, shell_pgrp);

	/* Grab control of the terminal.  */
	tcsetpgrp(shell_terminal, shell_pgrp);
}

ಪೂರ್ಣಾಂಕ hush_main(ಪೂರ್ಣಾಂಕ argc, ಅಕ್ಷರ * ಸ್ಥಿರಾಂಕ *argv)
{
	ಪೂರ್ಣಾಂಕ opt;
	FILE *ಒಳಹರಿವು;
	ಅಕ್ಷರ **e = environ;

	/* XXX what should these be while sourcing /etc/profile? */
	global_argc = argc;
	global_argv = argv;

	/* (re?) initialize globals.  Sometimes hush_main() ends up calling
	 * hush_main(), therefore we cannot rely on the BSS to zero out this
	 * stuff.  Reset these to 0 every time. */
	ifs = ಖಾಲಿ;
	/* map[] is taken care of with call to update_ifs_map() */
	fake_ಬಹುಲಕ = 0;
	interactive = 0;
	close_me_ತಲೆ = ಖಾಲಿ;
	ಕೊನೆಯ_bg_pid = 0;
	ಕೆಲಸ_list = ಖಾಲಿ;
	ಕೊನೆಯ_jobid = 0;

	/* Initialize some more globals to non-zero values */
	ಗಣ_cwd();
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_FEATURE_COMMAND_EDITING
	cmdedit_ಗಣ_initial_prompt();
#ಇಲ್ಲದಿದ್ದರೆ
	PS1 = ಖಾಲಿ;
#ಆದರೆ_ಅಂತ್ಯ
	PS2 = "> ";

	/* initialize our shell local variables with the values
	 * currently living in the environment */
	ಆದರೆ (e) {
		ಗಾಗಿ (; *e; e++)
			ಗಣ_local_var(*e, 2);   /* without call putenv() */
	}

	ಕೊನೆಯ_return_ಸಂಕೇತ=EXIT_SUCCESS;

	ಆದರೆ (argv[0] && argv[0][0] == '-') {
		debug_printf("\nsourcing /etc/profile\n");
		ಆದರೆ ((ಒಳಹರಿವು = fopen("/etc/profile", "r")) != ಖಾಲಿ) {
			mark_open(fileno(ಒಳಹರಿವು));
			parse_file_outer(ಒಳಹರಿವು);
			mark_closed(fileno(ಒಳಹರಿವು));
			fclose(ಒಳಹರಿವು);
		}
	}
	ಒಳಹರಿವು=stdin;

	ಆಗ ((opt = getopt(argc, argv, "c:xif")) > 0) {
		ಆಯ್ಕೆ (opt) {
			ಸ್ಥಿತಿ 'c':
				{
					global_argv = argv+optind;
					global_argc = argc-optind;
					opt = parse_string_outer(optarg, FLAG_PARSE_SEMICOLON);
					ಹೋಗು final_return;
				}
				ನಿಲ್ಲಿಸು;
			ಸ್ಥಿತಿ 'i':
				interactive++;
				ನಿಲ್ಲಿಸು;
			ಸ್ಥಿತಿ 'f':
				fake_ಬಹುಲಕ++;
				ನಿಲ್ಲಿಸು;
			ಪೂರ್ವನಿಯೋಜಿತ:
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ BB_VER
				fprintf(stderr, "Usage: sh [FILE]...\n"
						"   or: sh -c command [args]...\n\n");
				ನಿರ್ಗಮಿಸು(EXIT_FAILURE);
#ಇಲ್ಲದಿದ್ದರೆ
				show_usage();
#ಆದರೆ_ಅಂತ್ಯ
		}
	}
	/* A shell is interactive if the `-i' flag was given, or if all of
	 * the following conditions are met:
	 *	  no -c command
	 *    no arguments remaining or the -s flag given
	 *    standard input is a terminal
	 *    standard output is a terminal
	 *    Refer to Posix.2, the description of the `sh' utility. */
	ಆದರೆ (argv[optind]==ಖಾಲಿ && ಒಳಹರಿವು==stdin &&
			isatty(fileno(stdin)) && isatty(fileno(stdout))) {
		interactive++;
	}

	debug_printf("\ninteractive=%d\n", interactive);
	ಆದರೆ (interactive) {
		/* Looks like they want an interactive shell */
#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ CONFIG_FEATURE_SH_EXTRA_QUIET
		ಬರೆ( "\n\n" BB_BANNER " hush - the humble shell v0.01 (testing)\n");
		ಬರೆ( "Enter 'help' for a list of built-in commands.\n\n");
#ಆದರೆ_ಅಂತ್ಯ
		ಸಿದ್ಧತೆ_ಕೆಲಸ_control();
	}

	ಆದರೆ (argv[optind]==ಖಾಲಿ) {
		opt=parse_file_outer(stdin);
		ಹೋಗು final_return;
	}

	debug_printf("\nrunning script '%s'\n", argv[optind]);
	global_argv = argv+optind;
	global_argc = argc-optind;
	ಒಳಹರಿವು = xfopen(argv[optind], "r");
	opt = parse_file_outer(ಒಳಹರಿವು);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_FEATURE_CLEAN_ಮೇಲೆ
	fclose(ಒಳಹರಿವು);
	ಆದರೆ (cwd && cwd != unknown)
		ಬಿಡುಗಡೆ((ಅಕ್ಷರ*)cwd);
	{
		ರಚನೆ variables *cur, *tmp;
		ಗಾಗಿ(cur = ತುದಿ_vars; cur; cur = tmp) {
			tmp = cur->ಮುಂದಿನ;
			ಆದರೆ (!cur->flg_read_only) {
				ಬಿಡುಗಡೆ(cur->name);
				ಬಿಡುಗಡೆ(cur->ಮೌಲ್ಯ);
				ಬಿಡುಗಡೆ(cur);
			}
		}
	}
#ಆದರೆ_ಅಂತ್ಯ

final_return:
	ಹಿಂತಿರುಗಿ(opt?opt:ಕೊನೆಯ_return_ಸಂಕೇತ);
}
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ಅಕ್ಷರ *ಸೇರಿಸು_var_ಮೌಲ್ಯ(ಅಕ್ಷರ *inp)
{
	ಹಿಂತಿರುಗಿ ಸೇರಿಸು_var_ಮೌಲ್ಯ_sub(inp, 0);
}

ಸ್ಥಿರ ಅಕ್ಷರ *ಸೇರಿಸು_var_ಮೌಲ್ಯ_sub(ಅಕ್ಷರ *inp, ಪೂರ್ಣಾಂಕ ಟ್ಯಾಗ್_subst)
{
	ಪೂರ್ಣಾಂಕ res_str_len = 0;
	ಪೂರ್ಣಾಂಕ len;
	ಪೂರ್ಣಾಂಕ ಮುಗಿದ = 0;
	ಅಕ್ಷರ *p, *p1, *res_str = ಖಾಲಿ;

	ಆಗ ((p = strchr(inp, SPECIAL_VAR_ಚಿಹ್ನೆ))) {
		/* check the beginning of the string for normal characters */
		ಆದರೆ (p != inp) {
			/* copy any characters to the result string */
			len = p - inp;
			res_str = xrealloc(res_str, (res_str_len + len));
			strncpy((res_str + res_str_len), inp, len);
			res_str_len += len;
		}
		inp = ++p;
		/* find the ending marker */
		p = strchr(inp, SPECIAL_VAR_ಚಿಹ್ನೆ);
		*p = '\0';
		/* look up the value to substitute */
		ಆದರೆ ((p1 = lookup_param(inp))) {
			ಆದರೆ (ಟ್ಯಾಗ್_subst)
				len = res_str_len + ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(p1) + 2;
			ಇಲ್ಲದಿದ್ದರೆ
				len = res_str_len + ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(p1);
			res_str = xrealloc(res_str, (1 + len));
			ಆದರೆ (ಟ್ಯಾಗ್_subst) {
				/*
				 * copy the variable value to the result
				 * string
				 */
				strcpy((res_str + res_str_len + 1), p1);

				/*
				 * mark the replaced text to be accepted as
				 * is
				 */
				res_str[res_str_len] = SUBSTED_VAR_ಚಿಹ್ನೆ;
				res_str[res_str_len + 1 + ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(p1)] =
					SUBSTED_VAR_ಚಿಹ್ನೆ;
			} ಇಲ್ಲದಿದ್ದರೆ
				/*
				 * copy the variable value to the result
				 * string
				 */
				strcpy((res_str + res_str_len), p1);

			res_str_len = len;
		}
		*p = SPECIAL_VAR_ಚಿಹ್ನೆ;
		inp = ++p;
		ಮುಗಿದ = 1;
	}
	ಆದರೆ (ಮುಗಿದ) {
		res_str = xrealloc(res_str, (1 + res_str_len + ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(inp)));
		strcpy((res_str + res_str_len), inp);
		ಆಗ ((p = strchr(res_str, '\n'))) {
			*p = ' ';
		}
	}
	ಹಿಂತಿರುಗಿ (res_str == ಖಾಲಿ) ? inp : res_str;
}

ಸ್ಥಿರ ಅಕ್ಷರ **make_list_ಒಳ(ಅಕ್ಷರ **inp, ಅಕ್ಷರ *name)
{
	ಪೂರ್ಣಾಂಕ len, i;
	ಪೂರ್ಣಾಂಕ name_len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(name);
	ಪೂರ್ಣಾಂಕ n = 0;
	ಅಕ್ಷರ **list;
	ಅಕ್ಷರ *p1, *p2, *p3;

	/* create list of variable values */
	list = xmalloc(ಗಾತ್ರ(*list));
	ಗಾಗಿ (i = 0; inp[i]; i++) {
		p3 = ಸೇರಿಸು_var_ಮೌಲ್ಯ(inp[i]);
		p1 = p3;
		ಆಗ (*p1) {
			ಆದರೆ (*p1 == ' ') {
				p1++;
				ಮುಂದುವರಿ;
			}
			ಆದರೆ ((p2 = strchr(p1, ' '))) {
				len = p2 - p1;
			} ಇಲ್ಲದಿದ್ದರೆ {
				len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(p1);
				p2 = p1 + len;
			}
			/* we use n + 2 in realloc for list,because we add
			 * new element and then we will add NULL element */
			list = xrealloc(list, ಗಾತ್ರ(*list) * (n + 2));
			list[n] = xmalloc(2 + name_len + len);
			strcpy(list[n], name);
			strcat(list[n], "=");
			strncat(list[n], p1, len);
			list[n++][name_len + len + 1] = '\0';
			p1 = p2;
		}
		ಆದರೆ (p3 != inp[i]) ಬಿಡುಗಡೆ(p3);
	}
	list[n] = ಖಾಲಿ;
	ಹಿಂತಿರುಗಿ list;
}

/*
 * Make new string for parser
 * inp     - array of argument strings to flatten
 * nonnull - indicates argument was quoted when originally parsed
 */
ಸ್ಥಿರ ಅಕ್ಷರ *make_string(ಅಕ್ಷರ **inp, ಪೂರ್ಣಾಂಕ *nonnull)
{
	ಅಕ್ಷರ *p;
	ಅಕ್ಷರ *str = ಖಾಲಿ;
	ಪೂರ್ಣಾಂಕ n;
	ಪೂರ್ಣಾಂಕ len = 2;
	ಅಕ್ಷರ *noeval_str;
	ಪೂರ್ಣಾಂಕ noeval = 0;

	noeval_str = get_local_var("HUSH_NO_EVAL");
	ಆದರೆ (noeval_str != ಖಾಲಿ && *noeval_str != '0' && *noeval_str != '\0')
		noeval = 1;
	ಗಾಗಿ (n = 0; inp[n]; n++) {
		p = ಸೇರಿಸು_var_ಮೌಲ್ಯ_sub(inp[n], noeval);
		ಅಕ್ಷರ *new_str = xrealloc(str, (len + ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(p) + (2 * nonnull[n])));
		ಆದರೆ (!new_str) {
			ಬಿಡುಗಡೆ(str);
			ಆದರೆ (p != inp[n]) ಬಿಡುಗಡೆ(p);
			ಹಿಂತಿರುಗಿ ಖಾಲಿ;
		}
		str = new_str;
		ಆದರೆ (n) {
			strcat(str, " ");
		} ಇಲ್ಲದಿದ್ದರೆ {
			*str = '\0';
		}
		ಆದರೆ (nonnull[n])
			strcat(str, "'");
		strcat(str, p);
		ಆದರೆ (nonnull[n])
			strcat(str, "'");
		len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(str) + 3;
		ಆದರೆ (p != inp[n]) ಬಿಡುಗಡೆ(p);
	}
	len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(str);
	*(str + len) = '\n';
	*(str + len + 1) = '\0';
	ಹಿಂತಿರುಗಿ str;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __U_BOOT__
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ do_showvar(ರಚನೆ cmd_tbl *cmdtp, ಪೂರ್ಣಾಂಕ flag, ಪೂರ್ಣಾಂಕ argc,
		      ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ argv[])
{
	ಪೂರ್ಣಾಂಕ i, k;
	ಪೂರ್ಣಾಂಕ rcode = 0;
	ರಚನೆ variables *cur;

	ಆದರೆ (argc == 1) {		/* Print all env variables	*/
		ಗಾಗಿ (cur = ತುದಿ_vars; cur; cur = cur->ಮುಂದಿನ) {
			ಬರೆ ("%s=%s\n", cur->name, cur->ಮೌಲ್ಯ);
			ಆದರೆ (ctrlc ()) {
				puts ("\n ** Abort\n");
				ಹಿಂತಿರುಗಿ 1;
			}
		}
		ಹಿಂತಿರುಗಿ 0;
	}
	ಗಾಗಿ (i = 1; i < argc; ++i) {	/* print single env variables	*/
		ಅಕ್ಷರ *name = argv[i];

		k = -1;
		ಗಾಗಿ (cur = ತುದಿ_vars; cur; cur = cur->ಮುಂದಿನ) {
			ಆದರೆ(strcmp (cur->name, name) == 0) {
				k = 0;
				ಬರೆ ("%s=%s\n", cur->name, cur->ಮೌಲ್ಯ);
			}
			ಆದರೆ (ctrlc ()) {
				puts ("\n ** Abort\n");
				ಹಿಂತಿರುಗಿ 1;
			}
		}
		ಆದರೆ (k < 0) {
			ಬರೆ ("## Error: \"%s\" not defined\n", name);
			rcode ++;
		}
	}
	ಹಿಂತಿರುಗಿ rcode;
}

U_BOOT_CMD(
	showvar, CONFIG_SYS_MAXARGS, 1,	do_showvar,
	"print local hushshell variables",
	"\n    - print values of all hushshell variables\n"
	"showvar name ...\n"
	"    - print value of hushshell variable 'name'"
);

#ಆದರೆ_ಅಂತ್ಯ
/****************************************************************************/
