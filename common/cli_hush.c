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

#నిర్వచించు __U_BOOT__
#నిర్వచితమైతే __U_BOOT__
#అన్తర్భావ <env.h>
#అన్తర్భావ <malloc.h>         /* malloc, free, realloc*/
#అన్తర్భావ <linux/ctype.h>    /* isalpha, isdigit */
#అన్తర్భావ <console.h>
#అన్తర్భావ <bootretry.h>
#అన్తర్భావ <cli.h>
#అన్తర్భావ <cli_hush.h>
#అన్తర్భావ <command.h>        /* find_cmd */
#అన్తర్భావ <vsprintf.h>
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే __U_BOOT__
#అన్తర్భావ <ctype.h>     /* isalpha, isdigit */
#అన్తర్భావ <unistd.h>    /* getpid */
#అన్తర్భావ <stdlib.h>    /* getenv, atoi */
#అన్తర్భావ <string.h>    /* strchr */
#అన్తర్భావ <stdio.h>     /* popen etc. */
#అన్తర్భావ <glob.h>      /* glob, of course */
#అన్తర్భావ <stdarg.h>    /* va_list */
#అన్తర్భావ <errno.h>
#అన్తర్భావ <fcntl.h>
#అన్తర్భావ <getopt.h>    /* should be pretty obvious */

#అన్తర్భావ <sys/stat.h>  /* ulimit */
#అన్తర్భావ <sys/types.h>
#అన్తర్భావ <sys/wait.h>
#అన్తర్భావ <signal.h>

/* #include <dmalloc.h> */

#ఒకవేళ 1
#అన్తర్భావ "busybox.h"
#అన్తర్భావ "cmdedit.h"
#లేకపోతే
#నిర్వచించు applet_name "hush"
#అన్తర్భావ "standalone.h"
#నిర్వచించు hush_main ప్రధాన
#నిర్వచించు BB_BANNER
#అయితే_ముగింపు
#అయితే_ముగింపు
#నిర్వచించు SPECIAL_VAR_చిహ్నం 03
#నిర్వచించు SUBSTED_VAR_చిహ్నం 04
#నిర్వచితం_కాకపోతే __U_BOOT__
#నిర్వచించు FLAG_EXIT_FROM_LOOP 1
#నిర్వచించు FLAG_PARSE_SEMICOLON (1 << 1)		/* symbol ';' is special for parser */
#నిర్వచించు FLAG_REPARSING       (1 << 2)		/* >= 2nd pass */

#అయితే_ముగింపు

#నిర్వచితమైతే __U_BOOT__
#నిర్వచించు EXIT_SUCCESS 0
#నిర్వచించు EOF -1
#నిర్వచించు syntax() syntax_err()
#నిర్వచించు xstrdup strdup
#నిర్వచించు error_సందేశం రాయి
#లేకపోతే
రకనిర్వచనం గణన {
	REDIRECT_ఆదానం     = 1,
	REDIRECT_OVERWRITE = 2,
	REDIRECT_APPEND    = 3,
	REDIRECT_HEREIS    = 4,
	REDIRECT_IO        = 5
} redir_type;

/* The descrip member of this structure is only used to make debugging
 * output pretty */
నిర్మాణం {పూర్ణాంకం బహుళకం; పూర్ణాంకం default_fd; అక్షరం *descrip;} redir_పట్టిక[] = {
	{ 0,                         0, "()" },
	{ O_RDONLY,                  0, "<"  },
	{ O_CREAT|O_TRUNC|O_WRONLY,  1, ">"  },
	{ O_CREAT|O_APPEND|O_WRONLY, 1, ">>" },
	{ O_RDONLY,                 -1, "<<" },
	{ O_RDWR,                    1, "<>" }
};
#అయితే_ముగింపు

రకనిర్వచనం గణన {
	PIPE_SEQ = 1,
	PIPE_AND = 2,
	PIPE_OR  = 3,
	PIPE_BG  = 4,
} pipe_style;

/* might eventually control execution */
రకనిర్వచనం గణన {
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
	RES_పూర్తి  = 10,
	RES_XXXX  = 11,
	RES_లోపల    = 12,
	RES_SNTX  = 13
} reserved_style;
#నిర్వచించు FLAG_చివర   (1<<RES_NONE)
#నిర్వచించు FLAG_IF    (1<<RES_IF)
#నిర్వచించు FLAG_THEN  (1<<RES_THEN)
#నిర్వచించు FLAG_ELIF  (1<<RES_ELIF)
#నిర్వచించు FLAG_ELSE  (1<<RES_ELSE)
#నిర్వచించు FLAG_FI    (1<<RES_FI)
#నిర్వచించు FLAG_FOR   (1<<RES_FOR)
#నిర్వచించు FLAG_WHILE (1<<RES_WHILE)
#నిర్వచించు FLAG_UNTIL (1<<RES_UNTIL)
#నిర్వచించు FLAG_DO    (1<<RES_DO)
#నిర్వచించు FLAG_పూర్తి  (1<<RES_పూర్తి)
#నిర్వచించు FLAG_లోపల    (1<<RES_లోపల)
#నిర్వచించు FLAG_ప్రారంభం (1<<RES_XXXX)

/* This holds pointers to the various results of parsing */
నిర్మాణం p_context {
	నిర్మాణం బిడ్డ_prog *బిడ్డ;
	నిర్మాణం pipe *list_తల;
	నిర్మాణం pipe *pipe;
#నిర్వచితం_కాకపోతే __U_BOOT__
	నిర్మాణం redir_struct *pending_redirect;
#అయితే_ముగింపు
	reserved_style w;
	పూర్ణాంకం old_flag;				/* for figuring out valid reserved words */
	నిర్మాణం p_context *దొంతర;
	పూర్ణాంకం type;			/* define type of parser : ";$" common or special symbol */
	/* How about quoting status? */
};

#నిర్వచితం_కాకపోతే __U_BOOT__
నిర్మాణం redir_struct {
	redir_type type;			/* type of redirection */
	పూర్ణాంకం fd;						/* file descriptor being redirected */
	పూర్ణాంకం dup;					/* -1, or file descriptor being duplicated */
	నిర్మాణం redir_struct *తదుపరి;	/* pointer to the next redirect in the list */
	glob_t మాట;				/* *word.gl_pathv is the filename */
};
#అయితే_ముగింపు

నిర్మాణం బిడ్డ_prog {
#నిర్వచితం_కాకపోతే __U_BOOT__
	pid_t pid;					/* 0 if exited */
#అయితే_ముగింపు
	అక్షరం **argv;				/* program name and arguments */
	/* was quoted when parsed; copy of struct o_string.nonnull field */
	పూర్ణాంకం *argv_nonnull;
#నిర్వచితమైతే __U_BOOT__
	పూర్ణాంకం    argc;                            /* number of program arguments */
#అయితే_ముగింపు
	నిర్మాణం pipe *సమూహం;			/* if non-NULL, first in group or subshell */
#నిర్వచితం_కాకపోతే __U_BOOT__
	పూర్ణాంకం subshell;				/* flag, non-zero if group must be forked */
	నిర్మాణం redir_struct *redirects;	/* I/O redirections */
	glob_t glob_ఫలితం;			/* result of parameter globbing */
	పూర్ణాంకం is_stopped;				/* is the program currently running? */
	నిర్మాణం pipe *family;		/* pointer back to the child's parent pipe */
#అయితే_ముగింపు
	పూర్ణాంకం sp;				/* number of SPECIAL_VAR_SYMBOL */
	పూర్ణాంకం type;
};

నిర్మాణం pipe {
#నిర్వచితం_కాకపోతే __U_BOOT__
	పూర్ణాంకం jobid;					/* job number */
#అయితే_ముగింపు
	పూర్ణాంకం అంకె_progs;				/* total number of programs in job */
#నిర్వచితం_కాకపోతే __U_BOOT__
	పూర్ణాంకం running_progs;			/* number of programs running */
	అక్షరం *పాఠ్యం;					/* name of job */
	అక్షరం *cmdbuf;				/* buffer various argv's point into */
	pid_t pgrp;					/* process group ID for the job */
#అయితే_ముగింపు
	నిర్మాణం బిడ్డ_prog *progs;	/* array of commands in pipe */
	నిర్మాణం pipe *తదుపరి;			/* to track background commands */
#నిర్వచితం_కాకపోతే __U_BOOT__
	పూర్ణాంకం stopped_progs;			/* number of programs alive, but stopped */
	పూర్ణాంకం పని_context;			/* bitmask defining current context */
#అయితే_ముగింపు
	pipe_style followup;		/* PIPE_BG, PIPE_SEQ, PIPE_OR, PIPE_AND */
	reserved_style r_బహుళకం;		/* supports if, for, while, until */
};

#నిర్వచితం_కాకపోతే __U_BOOT__
నిర్మాణం close_me {
	పూర్ణాంకం fd;
	నిర్మాణం close_me *తదుపరి;
};
#అయితే_ముగింపు

నిర్మాణం variables {
	అక్షరం *name;
	అక్షరం *విలువ;
	పూర్ణాంకం flg_export;
	పూర్ణాంకం flg_read_only;
	నిర్మాణం variables *తదుపరి;
};

/* globals, connect us to the outside world
 * the first three support $?, $#, and $1 */
#నిర్వచితం_కాకపోతే __U_BOOT__
అక్షరం **global_argv;
గుర్తులేని పూర్ణాంకం global_argc;
#అయితే_ముగింపు
స్థిర గుర్తులేని పూర్ణాంకం చివరి_return_సంకేతం;
#నిర్వచితం_కాకపోతే __U_BOOT__
బాహ్య అక్షరం **environ; /* This is in <unistd.h>, but protected with __USE_GNU */
#అయితే_ముగింపు

/* "globals" within this file */
స్థిర uchar *ifs;
స్థిర అక్షరం map[256];
#నిర్వచితం_కాకపోతే __U_BOOT__
స్థిర పూర్ణాంకం fake_బహుళకం;
స్థిర పూర్ణాంకం interactive;
స్థిర నిర్మాణం close_me *close_me_తల;
స్థిర స్థిరరాశి అక్షరం *cwd;
స్థిర నిర్మాణం pipe *పని_list;
స్థిర గుర్తులేని పూర్ణాంకం చివరి_bg_pid;
స్థిర గుర్తులేని పూర్ణాంకం చివరి_jobid;
స్థిర గుర్తులేని పూర్ణాంకం shell_terminal;
స్థిర అక్షరం *PS1;
స్థిర అక్షరం *PS2;
నిర్మాణం variables shell_ver = { "HUSH_VERSION", "0.01", 1, 1, 0 };
నిర్మాణం variables *శిఖరం_vars = &shell_ver;
#లేకపోతే
స్థిర పూర్ణాంకం flag_repeat = 0;
స్థిర పూర్ణాంకం do_repeat = 0;
స్థిర నిర్మాణం variables *శిఖరం_vars = ఖాళీ ;
#అయితే_ముగింపు /*__U_BOOT__ */

#నిర్వచించు B_CHUNK (100)
#నిర్వచించు B_NOSPAC 1

రకనిర్వచనం నిర్మాణం {
	అక్షరం *దత్తాంశం;
	పూర్ణాంకం నిడివి;
	పూర్ణాంకం maxlen;
	పూర్ణాంకం quote;
	పూర్ణాంకం nonnull;
} o_string;
#నిర్వచించు NULL_O_STRING {ఖాళీ,0,0,0,0}
/* used for initialization:
	o_string foo = NULL_O_STRING; */

/* I can almost use ordinary FILE *.  Is open_memstream() universally
 * available?  Where is it documented? */
నిర్మాణం లోపల_str {
	స్థిరరాశి గుర్తులేని అక్షరం *p;
#నిర్వచితం_కాకపోతే __U_BOOT__
	అక్షరం peek_buf[2];
#అయితే_ముగింపు
	పూర్ణాంకం __promptme;
	పూర్ణాంకం promptmode;
#నిర్వచితం_కాకపోతే __U_BOOT__
	FILE *file;
#అయితే_ముగింపు
	పూర్ణాంకం (*get) (నిర్మాణం లోపల_str *);
	పూర్ణాంకం (*peek) (నిర్మాణం లోపల_str *);
};
#నిర్వచించు b_getch(ఆదానం) ((ఆదానం)->get(ఆదానం))
#నిర్వచించు b_peek(ఆదానం) ((ఆదానం)->peek(ఆదానం))

#నిర్వచితం_కాకపోతే __U_BOOT__
#నిర్వచించు పని_స్థితి_FORMAT "[%d] %-22s %.40s\n"

నిర్మాణం built_లోపల_command {
	అక్షరం *cmd;					/* name */
	అక్షరం *descr;				/* description */
	పూర్ణాంకం (*function) (నిర్మాణం బిడ్డ_prog *);	/* function ptr */
};
#అయితే_ముగింపు

/* define DEBUG_SHELL for debugging output (obviously ;-)) */
#ఒకవేళ 0
#నిర్వచించు DEBUG_SHELL
#అయితే_ముగింపు

/* This should be in utility.c */
#నిర్వచితమైతే DEBUG_SHELL
#నిర్వచితం_కాకపోతే __U_BOOT__
స్థిర శూన్య debug_printf(స్థిరరాశి అక్షరం *format, ...)
{
	va_list వాదనలు;
	va_ప్రారంభం(వాదనలు, format);
	vfprintf(stderr, format, వాదనలు);
	va_చివర(వాదనలు);
}
#లేకపోతే
#నిర్వచించు debug_printf(fmt,వాదనలు...)	రాయి (fmt ,##args)
#అయితే_ముగింపు
#లేకపోతే
స్థిర అంతఃవరుస శూన్య debug_printf(స్థిరరాశి అక్షరం *format, ...) { }
#అయితే_ముగింపు
#నిర్వచించు final_printf debug_printf

#నిర్వచితమైతే __U_BOOT__
స్థిర శూన్య syntax_err(శూన్య) {
	 రాయి("syntax error\n");
}
#లేకపోతే
స్థిర శూన్య __syntax(అక్షరం *file, పూర్ణాంకం పంక్తి) {
	error_సందేశం("syntax error %s:%d", file, పంక్తి);
}
#నిర్వచించు syntax() __syntax(__FILE__, __LINE__)
#అయితే_ముగింపు

#నిర్వచితమైతే __U_BOOT__
స్థిర శూన్య *xmalloc(size_t కొలత);
స్థిర శూన్య *xrealloc(శూన్య *ptr, size_t కొలత);
#లేకపోతే
/* Index of subroutines: */
/*   function prototypes for builtins */
స్థిర పూర్ణాంకం builtin_cd(నిర్మాణం బిడ్డ_prog *బిడ్డ);
స్థిర పూర్ణాంకం builtin_env(నిర్మాణం బిడ్డ_prog *బిడ్డ);
స్థిర పూర్ణాంకం builtin_eval(నిర్మాణం బిడ్డ_prog *బిడ్డ);
స్థిర పూర్ణాంకం builtin_exec(నిర్మాణం బిడ్డ_prog *బిడ్డ);
స్థిర పూర్ణాంకం builtin_exit(నిర్మాణం బిడ్డ_prog *బిడ్డ);
స్థిర పూర్ణాంకం builtin_export(నిర్మాణం బిడ్డ_prog *బిడ్డ);
స్థిర పూర్ణాంకం builtin_fg_bg(నిర్మాణం బిడ్డ_prog *బిడ్డ);
స్థిర పూర్ణాంకం builtin_help(నిర్మాణం బిడ్డ_prog *బిడ్డ);
స్థిర పూర్ణాంకం builtin_jobs(నిర్మాణం బిడ్డ_prog *బిడ్డ);
స్థిర పూర్ణాంకం builtin_pwd(నిర్మాణం బిడ్డ_prog *బిడ్డ);
స్థిర పూర్ణాంకం builtin_read(నిర్మాణం బిడ్డ_prog *బిడ్డ);
స్థిర పూర్ణాంకం builtin_సమితి(నిర్మాణం బిడ్డ_prog *బిడ్డ);
స్థిర పూర్ణాంకం builtin_జరుపు(నిర్మాణం బిడ్డ_prog *బిడ్డ);
స్థిర పూర్ణాంకం builtin_ఆధారం(నిర్మాణం బిడ్డ_prog *బిడ్డ);
స్థిర పూర్ణాంకం builtin_umask(నిర్మాణం బిడ్డ_prog *బిడ్డ);
స్థిర పూర్ణాంకం builtin_unset(నిర్మాణం బిడ్డ_prog *బిడ్డ);
స్థిర పూర్ణాంకం builtin_not_written(నిర్మాణం బిడ్డ_prog *బిడ్డ);
#అయితే_ముగింపు
/*   o_string manipulation: */
స్థిర పూర్ణాంకం b_check_space(o_string *o, పూర్ణాంకం len);
స్థిర పూర్ణాంకం b_addchr(o_string *o, పూర్ణాంకం ch);
స్థిర శూన్య b_reset(o_string *o);
స్థిర పూర్ణాంకం b_addqchr(o_string *o, పూర్ణాంకం ch, పూర్ణాంకం quote);
#నిర్వచితం_కాకపోతే __U_BOOT__
స్థిర పూర్ణాంకం b_adduint(o_string *o, గుర్తులేని పూర్ణాంకం i);
#అయితే_ముగింపు
/*  in_str manipulations: */
స్థిర పూర్ణాంకం static_get(నిర్మాణం లోపల_str *i);
స్థిర పూర్ణాంకం static_peek(నిర్మాణం లోపల_str *i);
స్థిర పూర్ణాంకం file_get(నిర్మాణం లోపల_str *i);
స్థిర పూర్ణాంకం file_peek(నిర్మాణం లోపల_str *i);
#నిర్వచితం_కాకపోతే __U_BOOT__
స్థిర శూన్య అమరిక_file_లోపల_str(నిర్మాణం లోపల_str *i, FILE *f);
#లేకపోతే
స్థిర శూన్య అమరిక_file_లోపల_str(నిర్మాణం లోపల_str *i);
#అయితే_ముగింపు
స్థిర శూన్య అమరిక_string_లోపల_str(నిర్మాణం లోపల_str *i, స్థిరరాశి అక్షరం *s);
#నిర్వచితం_కాకపోతే __U_BOOT__
/*  close_me manipulations: */
స్థిర శూన్య mark_open(పూర్ణాంకం fd);
స్థిర శూన్య mark_closed(పూర్ణాంకం fd);
స్థిర శూన్య close_all(శూన్య);
#అయితే_ముగింపు
/*  "run" the final data structures: */
స్థిర అక్షరం *indenter(పూర్ణాంకం i);
స్థిర పూర్ణాంకం free_pipe_list(నిర్మాణం pipe *తల, పూర్ణాంకం indent);
స్థిర పూర్ణాంకం free_pipe(నిర్మాణం pipe *pi, పూర్ణాంకం indent);
/*  really run the final data structures: */
#నిర్వచితం_కాకపోతే __U_BOOT__
స్థిర పూర్ణాంకం అమరిక_redirects(నిర్మాణం బిడ్డ_prog *prog, పూర్ణాంకం squirrel[]);
#అయితే_ముగింపు
స్థిర పూర్ణాంకం నిరంతరం_list_real(నిర్మాణం pipe *pi);
#నిర్వచితం_కాకపోతే __U_BOOT__
స్థిర శూన్య pseudo_exec(నిర్మాణం బిడ్డ_prog *బిడ్డ) __attribute__ ((noreturn));
#అయితే_ముగింపు
స్థిర పూర్ణాంకం నిరంతరం_pipe_real(నిర్మాణం pipe *pi);
/*   extended glob support: */
#నిర్వచితం_కాకపోతే __U_BOOT__
స్థిర పూర్ణాంకం globhack(స్థిరరాశి అక్షరం *src, పూర్ణాంకం flags, glob_t *pglob);
స్థిర పూర్ణాంకం glob_needed(స్థిరరాశి అక్షరం *s);
స్థిర పూర్ణాంకం xglob(o_string *dest, పూర్ణాంకం flags, glob_t *pglob);
#అయితే_ముగింపు
/*   variable assignment: */
స్థిర పూర్ణాంకం is_assignment(స్థిరరాశి అక్షరం *s);
/*   data structure manipulation: */
#నిర్వచితం_కాకపోతే __U_BOOT__
స్థిర పూర్ణాంకం అమరిక_redirect(నిర్మాణం p_context *ctx, పూర్ణాంకం fd, redir_type style, నిర్మాణం లోపల_str *ఆదానం);
#అయితే_ముగింపు
స్థిర శూన్య initialize_context(నిర్మాణం p_context *ctx);
స్థిర పూర్ణాంకం పూర్తి_మాట(o_string *dest, నిర్మాణం p_context *ctx);
స్థిర పూర్ణాంకం పూర్తి_command(నిర్మాణం p_context *ctx);
స్థిర పూర్ణాంకం పూర్తి_pipe(నిర్మాణం p_context *ctx, pipe_style type);
/*   primary string parsing: */
#నిర్వచితం_కాకపోతే __U_BOOT__
స్థిర పూర్ణాంకం redirect_dup_అంకె(నిర్మాణం లోపల_str *ఆదానం);
స్థిర పూర్ణాంకం redirect_opt_అంకె(o_string *o);
స్థిర పూర్ణాంకం process_command_subs(o_string *dest, నిర్మాణం p_context *ctx, నిర్మాణం లోపల_str *ఆదానం, పూర్ణాంకం subst_చివర);
స్థిర పూర్ణాంకం parse_సమూహం(o_string *dest, నిర్మాణం p_context *ctx, నిర్మాణం లోపల_str *ఆదానం, పూర్ణాంకం ch);
#అయితే_ముగింపు
స్థిర అక్షరం *lookup_param(అక్షరం *src);
స్థిర అక్షరం *make_string(అక్షరం **inp, పూర్ణాంకం *nonnull);
స్థిర పూర్ణాంకం handle_dollar(o_string *dest, నిర్మాణం p_context *ctx, నిర్మాణం లోపల_str *ఆదానం);
#నిర్వచితం_కాకపోతే __U_BOOT__
స్థిర పూర్ణాంకం parse_string(o_string *dest, నిర్మాణం p_context *ctx, స్థిరరాశి అక్షరం *src);
#అయితే_ముగింపు
స్థిర పూర్ణాంకం parse_ప్రవాహం(o_string *dest, నిర్మాణం p_context *ctx, నిర్మాణం లోపల_str *input0, పూర్ణాంకం చివర_trigger);
/*   setup: */
స్థిర పూర్ణాంకం parse_ప్రవాహం_outer(నిర్మాణం లోపల_str *inp, పూర్ణాంకం flag);
#నిర్వచితం_కాకపోతే __U_BOOT__
స్థిర పూర్ణాంకం parse_string_outer(స్థిరరాశి అక్షరం *s, పూర్ణాంకం flag);
స్థిర పూర్ణాంకం parse_file_outer(FILE *f);
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే __U_BOOT__
/*   job management: */
స్థిర పూర్ణాంకం checkjobs(నిర్మాణం pipe* fg_pipe);
స్థిర శూన్య చొప్పించు_bg_పని(నిర్మాణం pipe *pi);
స్థిర శూన్య remove_bg_పని(నిర్మాణం pipe *pi);
#అయితే_ముగింపు
/*     local variable support */
స్థిర అక్షరం **make_list_లోపల(అక్షరం **inp, అక్షరం *name);
స్థిర అక్షరం *చొప్పించు_var_విలువ(అక్షరం *inp);
స్థిర అక్షరం *చొప్పించు_var_విలువ_sub(అక్షరం *inp, పూర్ణాంకం ట్యాగ్_subst);

#నిర్వచితం_కాకపోతే __U_BOOT__
/* Table of built-in functions.  They can be forked or not, depending on
 * context: within pipes, they fork.  As simple commands, they do not.
 * When used in non-forking context, they can change global variables
 * in the parent shell process.  If forked, of course they can not.
 * For example, 'unset foo | whatever' will parse and run, but foo will
 * still be set at the end. */
స్థిర నిర్మాణం built_లోపల_command bltins[] = {
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
	{"set", "Set/unset shell local variables", builtin_సమితి},
	{"shift", "Shift positional parameters", builtin_జరుపు},
	{"trap", "Trap signals", builtin_not_written},
	{"ulimit","Controls resource limits", builtin_not_written},
	{"umask","Sets file creation mask", builtin_umask},
	{"unset", "Unset environment variable", builtin_unset},
	{".", "Source-in and run commands in a file", builtin_ఆధారం},
	{"help", "List shell built-in commands", builtin_help},
	{ఖాళీ, ఖాళీ, ఖాళీ}
};

స్థిర స్థిరరాశి అక్షరం *సమితి_cwd(శూన్య)
{
	ఒకవేళ(cwd==unknown)
		cwd = ఖాళీ;     /* xgetcwd(arg) called free(arg) */
	cwd = xgetcwd((అక్షరం *)cwd);
	ఒకవేళ (!cwd)
		cwd = unknown;
	తిరిగి cwd;
}

/* built-in 'eval' handler */
స్థిర పూర్ణాంకం builtin_eval(నిర్మాణం బిడ్డ_prog *బిడ్డ)
{
	అక్షరం *str = ఖాళీ;
	పూర్ణాంకం rcode = EXIT_SUCCESS;

	ఒకవేళ (బిడ్డ->argv[1]) {
		str = make_string(బిడ్డ->argv + 1);
		parse_string_outer(str, FLAG_EXIT_FROM_LOOP |
					FLAG_PARSE_SEMICOLON);
		విడుదల(str);
		rcode = చివరి_return_సంకేతం;
	}
	తిరిగి rcode;
}

/* built-in 'cd <path>' handler */
స్థిర పూర్ణాంకం builtin_cd(నిర్మాణం బిడ్డ_prog *బిడ్డ)
{
	అక్షరం *newdir;
	ఒకవేళ (బిడ్డ->argv[1] == ఖాళీ)
		newdir = env_get("HOME");
	లేకపోతే
		newdir = బిడ్డ->argv[1];
	ఒకవేళ (chdir(newdir)) {
		రాయి("cd: %s: %s\n", newdir, strerror(errno));
		తిరిగి EXIT_FAILURE;
	}
	సమితి_cwd();
	తిరిగి EXIT_SUCCESS;
}

/* built-in 'env' handler */
స్థిర పూర్ణాంకం builtin_env(నిర్మాణం బిడ్డ_prog *dummy)
{
	అక్షరం **e = environ;
	ఒకవేళ (e == ఖాళీ) తిరిగి EXIT_FAILURE;
	కొరకు (; *e; e++) {
		puts(*e);
	}
	తిరిగి EXIT_SUCCESS;
}

/* built-in 'exec' handler */
స్థిర పూర్ణాంకం builtin_exec(నిర్మాణం బిడ్డ_prog *బిడ్డ)
{
	ఒకవేళ (బిడ్డ->argv[1] == ఖాళీ)
		తిరిగి EXIT_SUCCESS;   /* Really? */
	బిడ్డ->argv++;
	pseudo_exec(బిడ్డ);
	/* never returns */
}

/* built-in 'exit' handler */
స్థిర పూర్ణాంకం builtin_exit(నిర్మాణం బిడ్డ_prog *బిడ్డ)
{
	ఒకవేళ (బిడ్డ->argv[1] == ఖాళీ)
		నిష్క్రమించు(చివరి_return_సంకేతం);
	నిష్క్రమించు (atoi(బిడ్డ->argv[1]));
}

/* built-in 'export VAR=value' handler */
స్థిర పూర్ణాంకం builtin_export(నిర్మాణం బిడ్డ_prog *బిడ్డ)
{
	పూర్ణాంకం res = 0;
	అక్షరం *name = బిడ్డ->argv[1];

	ఒకవేళ (name == ఖాళీ) {
		తిరిగి (builtin_env(బిడ్డ));
	}

	name = strdup(name);

	ఒకవేళ(name) {
		అక్షరం *విలువ = strchr(name, '=');

		ఒకవేళ (!విలువ) {
			అక్షరం *tmp;
			/* They are exporting something without an =VALUE */

			విలువ = get_local_var(name);
			ఒకవేళ (విలువ) {
				size_t ln = అక్షరశ్రేణి_పొడవు(name);

				tmp = realloc(name, ln+అక్షరశ్రేణి_పొడవు(విలువ)+2);
				ఒకవేళ(tmp==ఖాళీ)
					res = -1;
				లేకపోతే {
					sprintf(tmp+ln, "=%s", విలువ);
					name = tmp;
				}
			} లేకపోతే {
				/* bash does not return an error when trying to export
				 * an undefined variable.  Do likewise. */
				res = 1;
			}
		}
	}
	ఒకవేళ (res<0)
		perror_సందేశం("export");
	లేకపోతే ఒకవేళ(res==0)
		res = సమితి_local_var(name, 1);
	లేకపోతే
		res = 0;
	విడుదల(name);
	తిరిగి res;
}

/* built-in 'fg' and 'bg' handler */
స్థిర పూర్ణాంకం builtin_fg_bg(నిర్మాణం బిడ్డ_prog *బిడ్డ)
{
	పూర్ణాంకం i, jobnum;
	నిర్మాణం pipe *pi=ఖాళీ;

	ఒకవేళ (!interactive)
		తిరిగి EXIT_FAILURE;
	/* If they gave us no args, assume they want the last backgrounded task */
	ఒకవేళ (!బిడ్డ->argv[1]) {
		కొరకు (pi = పని_list; pi; pi = pi->తదుపరి) {
			ఒకవేళ (pi->jobid == చివరి_jobid) {
				ఆపు;
			}
		}
		ఒకవేళ (!pi) {
			error_సందేశం("%s: no current job", బిడ్డ->argv[0]);
			తిరిగి EXIT_FAILURE;
		}
	} లేకపోతే {
		ఒకవేళ (అక్షరశ్రేణి_చదువు(బిడ్డ->argv[1], "%%%d", &jobnum) != 1) {
			error_సందేశం("%s: bad argument '%s'", బిడ్డ->argv[0], బిడ్డ->argv[1]);
			తిరిగి EXIT_FAILURE;
		}
		కొరకు (pi = పని_list; pi; pi = pi->తదుపరి) {
			ఒకవేళ (pi->jobid == jobnum) {
				ఆపు;
			}
		}
		ఒకవేళ (!pi) {
			error_సందేశం("%s: %d: no such job", బిడ్డ->argv[0], jobnum);
			తిరిగి EXIT_FAILURE;
		}
	}

	ఒకవేళ (*బిడ్డ->argv[0] == 'f') {
		/* Put the job into the foreground.  */
		tcsetpgrp(shell_terminal, pi->pgrp);
	}

	/* Restart the processes in the job */
	కొరకు (i = 0; i < pi->అంకె_progs; i++)
		pi->progs[i].is_stopped = 0;

	ఒకవేళ ( (i=kill(- pi->pgrp, SIGCONT)) < 0) {
		ఒకవేళ (i == ESRCH) {
			remove_bg_పని(pi);
		} లేకపోతే {
			perror_సందేశం("kill (SIGCONT)");
		}
	}

	pi->stopped_progs = 0;
	తిరిగి EXIT_SUCCESS;
}

/* built-in 'help' handler */
స్థిర పూర్ణాంకం builtin_help(నిర్మాణం బిడ్డ_prog *dummy)
{
	నిర్మాణం built_లోపల_command *x;

	రాయి("\nBuilt-in commands:\n");
	రాయి("-------------------\n");
	కొరకు (x = bltins; x->cmd; x++) {
		ఒకవేళ (x->descr==ఖాళీ)
			కొనసాగు;
		రాయి("%s\t%s\n", x->cmd, x->descr);
	}
	రాయి("\n\n");
	తిరిగి EXIT_SUCCESS;
}

/* built-in 'jobs' handler */
స్థిర పూర్ణాంకం builtin_jobs(నిర్మాణం బిడ్డ_prog *బిడ్డ)
{
	నిర్మాణం pipe *పని;
	అక్షరం *స్థితి_string;

	కొరకు (పని = పని_list; పని; పని = పని->తదుపరి) {
		ఒకవేళ (పని->running_progs == పని->stopped_progs)
			స్థితి_string = "Stopped";
		లేకపోతే
			స్థితి_string = "Running";

		రాయి(పని_స్థితి_FORMAT, పని->jobid, స్థితి_string, పని->పాఠ్యం);
	}
	తిరిగి EXIT_SUCCESS;
}

/* built-in 'pwd' handler */
స్థిర పూర్ణాంకం builtin_pwd(నిర్మాణం బిడ్డ_prog *dummy)
{
	puts(సమితి_cwd());
	తిరిగి EXIT_SUCCESS;
}

/* built-in 'read VAR' handler */
స్థిర పూర్ణాంకం builtin_read(నిర్మాణం బిడ్డ_prog *బిడ్డ)
{
	పూర్ణాంకం res;

	ఒకవేళ (బిడ్డ->argv[1]) {
		అక్షరం string[BUFSIZ];
		అక్షరం *var = 0;

		string[0] = 0;  /* In case stdin has only EOF */
		/* read string */
		fgets(string, పరిమాణం(string), stdin);
		chomp(string);
		var = జ్ఞాపకం_ఇవ్వు(అక్షరశ్రేణి_పొడవు(బిడ్డ->argv[1])+అక్షరశ్రేణి_పొడవు(string)+2);
		ఒకవేళ(var) {
			sprintf(var, "%s=%s", బిడ్డ->argv[1], string);
			res = సమితి_local_var(var, 0);
		} లేకపోతే
			res = -1;
		ఒకవేళ (res)
			fprintf(stderr, "read: %m\n");
		విడుదల(var);      /* So not move up to avoid breaking errno */
		తిరిగి res;
	} లేకపోతే {
		చేయి res=getchar(); అయితే(res!='\n' && res!=EOF);
		తిరిగి 0;
	}
}

/* built-in 'set VAR=value' handler */
స్థిర పూర్ణాంకం builtin_సమితి(నిర్మాణం బిడ్డ_prog *బిడ్డ)
{
	అక్షరం *తాత్కాలిక = బిడ్డ->argv[1];
	నిర్మాణం variables *e;

	ఒకవేళ (తాత్కాలిక == ఖాళీ)
		కొరకు(e = శిఖరం_vars; e; e=e->తదుపరి)
			రాయి("%s=%s\n", e->name, e->విలువ);
	లేకపోతే
		సమితి_local_var(తాత్కాలిక, 0);

		తిరిగి EXIT_SUCCESS;
}

/* Built-in 'shift' handler */
స్థిర పూర్ణాంకం builtin_జరుపు(నిర్మాణం బిడ్డ_prog *బిడ్డ)
{
	పూర్ణాంకం n=1;
	ఒకవేళ (బిడ్డ->argv[1]) {
		n=atoi(బిడ్డ->argv[1]);
	}
	ఒకవేళ (n>=0 && n<global_argc) {
		/* XXX This probably breaks $0 */
		global_argc -= n;
		global_argv += n;
		తిరిగి EXIT_SUCCESS;
	} లేకపోతే {
		తిరిగి EXIT_FAILURE;
	}
}

/* Built-in '.' handler (read-in and execute commands from file) */
స్థిర పూర్ణాంకం builtin_ఆధారం(నిర్మాణం బిడ్డ_prog *బిడ్డ)
{
	FILE *ఆదానం;
	పూర్ణాంకం స్థితి_2;

	ఒకవేళ (బిడ్డ->argv[1] == ఖాళీ)
		తిరిగి EXIT_FAILURE;

	/* XXX search through $PATH is missing */
	ఆదానం = fopen(బిడ్డ->argv[1], "r");
	ఒకవేళ (!ఆదానం) {
		error_సందేశం("Couldn't open file '%s'", బిడ్డ->argv[1]);
		తిరిగి EXIT_FAILURE;
	}

	/* Now run the file */
	/* XXX argv and argc are broken; need to save old global_argv
	 * (pointer only is OK!) on this stack frame,
	 * set global_argv=child->argv+1, recurse, and restore. */
	mark_open(fileno(ఆదానం));
	స్థితి_2 = parse_file_outer(ఆదానం);
	mark_closed(fileno(ఆదానం));
	fclose(ఆదానం);
	తిరిగి (స్థితి_2);
}

స్థిర పూర్ణాంకం builtin_umask(నిర్మాణం బిడ్డ_prog *బిడ్డ)
{
	mode_t new_umask;
	స్థిరరాశి అక్షరం *arg = బిడ్డ->argv[1];
	అక్షరం *చివర_2;
	ఒకవేళ (arg) {
		new_umask=strtoul(arg, &చివర_2, 8);
		ఒకవేళ (*చివర_2!='\0' || చివర_2 == arg) {
			తిరిగి EXIT_FAILURE;
		}
	} లేకపోతే {
		రాయి("%.3o\n", (గుర్తులేని పూర్ణాంకం) (new_umask=umask(0)));
	}
	umask(new_umask);
	తిరిగి EXIT_SUCCESS;
}

/* built-in 'unset VAR' handler */
స్థిర పూర్ణాంకం builtin_unset(నిర్మాణం బిడ్డ_prog *బిడ్డ)
{
	/* bash returned already true */
	unset_local_var(బిడ్డ->argv[1]);
	తిరిగి EXIT_SUCCESS;
}

స్థిర పూర్ణాంకం builtin_not_written(నిర్మాణం బిడ్డ_prog *బిడ్డ)
{
	రాయి("builtin_%s not written\n",బిడ్డ->argv[0]);
	తిరిగి EXIT_FAILURE;
}
#అయితే_ముగింపు

స్థిర పూర్ణాంకం b_check_space(o_string *o, పూర్ణాంకం len)
{
	/* It would be easy to drop a more restrictive policy
	 * in here, such as setting a maximum string length */
	ఒకవేళ (o->నిడివి + len > o->maxlen) {
		అక్షరం *old_దత్తాంశం = o->దత్తాంశం;
		/* assert (data == NULL || o->maxlen != 0); */
		o->maxlen += max(2*len, B_CHUNK);
		o->దత్తాంశం = realloc(o->దత్తాంశం, 1 + o->maxlen);
		ఒకవేళ (o->దత్తాంశం == ఖాళీ) {
			విడుదల(old_దత్తాంశం);
		}
	}
	తిరిగి o->దత్తాంశం == ఖాళీ;
}

స్థిర పూర్ణాంకం b_addchr(o_string *o, పూర్ణాంకం ch)
{
	debug_printf("b_addchr: %c %d %p\n", ch, o->నిడివి, o);
	ఒకవేళ (b_check_space(o, 1)) తిరిగి B_NOSPAC;
	o->దత్తాంశం[o->నిడివి] = ch;
	o->నిడివి++;
	o->దత్తాంశం[o->నిడివి] = '\0';
	తిరిగి 0;
}

స్థిర శూన్య b_reset(o_string *o)
{
	o->నిడివి = 0;
	o->nonnull = 0;
	ఒకవేళ (o->దత్తాంశం != ఖాళీ) *o->దత్తాంశం = '\0';
}

స్థిర శూన్య b_free(o_string *o)
{
	b_reset(o);
	విడుదల(o->దత్తాంశం);
	o->దత్తాంశం = ఖాళీ;
	o->maxlen = 0;
}

/* My analysis of quoting semantics tells me that state information
 * is associated with a destination, not a source.
 */
స్థిర పూర్ణాంకం b_addqchr(o_string *o, పూర్ణాంకం ch, పూర్ణాంకం quote)
{
	ఒకవేళ (quote && strchr("*?[\\",ch)) {
		పూర్ణాంకం rc;
		rc = b_addchr(o, '\\');
		ఒకవేళ (rc) తిరిగి rc;
	}
	తిరిగి b_addchr(o, ch);
}

#నిర్వచితం_కాకపోతే __U_BOOT__
స్థిర పూర్ణాంకం b_adduint(o_string *o, గుర్తులేని పూర్ణాంకం i)
{
	పూర్ణాంకం r;
	అక్షరం *p = simple_itoa(i);
	/* no escape checking necessary */
	చేయి r=b_addchr(o, *p++); అయితే (r==0 && *p);
	తిరిగి r;
}
#అయితే_ముగింపు

స్థిర పూర్ణాంకం static_get(నిర్మాణం లోపల_str *i)
{
	పూర్ణాంకం ch = *i->p++;
	ఒకవేళ (ch=='\0') తిరిగి EOF;
	తిరిగి ch;
}

స్థిర పూర్ణాంకం static_peek(నిర్మాణం లోపల_str *i)
{
	తిరిగి *i->p;
}

#నిర్వచితం_కాకపోతే __U_BOOT__
స్థిర అంతఃవరుస శూన్య cmdedit_సమితి_initial_prompt(శూన్య)
{
#నిర్వచితం_కాకపోతే CONFIG_FEATURE_SH_FANCY_PROMPT
	PS1 = ఖాళీ;
#లేకపోతే
	PS1 = env_get("PS1");
	ఒకవేళ(PS1==0)
		PS1 = "\\w \\$ ";
#అయితే_ముగింపు
}

స్థిర అంతఃవరుస శూన్య అమరిక_prompt_string(పూర్ణాంకం promptmode, అక్షరం **prompt_str)
{
	debug_printf("setup_prompt_string %d ",promptmode);
#నిర్వచితం_కాకపోతే CONFIG_FEATURE_SH_FANCY_PROMPT
	/* Set up the prompt */
	ఒకవేళ (promptmode == 1) {
		విడుదల(PS1);
		PS1=xmalloc(అక్షరశ్రేణి_పొడవు(cwd)+4);
		sprintf(PS1, "%s %s", cwd, ( geteuid() != 0 ) ?  "$ ":"# ");
		*prompt_str = PS1;
	} లేకపోతే {
		*prompt_str = PS2;
	}
#లేకపోతే
	*prompt_str = (promptmode==1)? PS1 : PS2;
#అయితే_ముగింపు
	debug_printf("result %s\n",*prompt_str);
}
#అయితే_ముగింపు

#నిర్వచితమైతే __U_BOOT__
స్థిర పూర్ణాంకం uboot_cli_readline(నిర్మాణం లోపల_str *i)
{
	అక్షరం *prompt;
	అక్షరం __maybe_unused *ps_prompt = ఖాళీ;

	ఒకవేళ (i->promptmode == 1)
		prompt = CONFIG_SYS_PROMPT;
	లేకపోతే
		prompt = CONFIG_SYS_PROMPT_HUSH_PS2;

#నిర్వచితమైతే CONFIG_CMDLINE_PS_SUPPORT
	ఒకవేళ (i->promptmode == 1)
		ps_prompt = env_get("PS1");
	లేకపోతే
		ps_prompt = env_get("PS2");
	ఒకవేళ (ps_prompt)
		prompt = ps_prompt;
#అయితే_ముగింపు

	తిరిగి cli_readline(prompt);
}
#అయితే_ముగింపు

స్థిర శూన్య get_user_ఆదానం(నిర్మాణం లోపల_str *i)
{
#నిర్వచితం_కాకపోతే __U_BOOT__
	అక్షరం *prompt_str;
	స్థిర అక్షరం the_command[BUFSIZ];

	అమరిక_prompt_string(i->promptmode, &prompt_str);
#నిర్వచితమైతే CONFIG_FEATURE_COMMAND_EDITING
	/*
	 ** enable command line editing only while a command line
	 ** is actually being read; otherwise, we'll end up bequeathing
	 ** atexit() handlers and other unwanted stuff to our
	 ** child processes (rob@sysgo.de)
	 */
	cmdedit_read_ఆదానం(prompt_str, the_command);
#లేకపోతే
	fputs(prompt_str, stdout);
	fflush(stdout);
	the_command[0]=fgetc(i->file);
	the_command[1]='\0';
#అయితే_ముగింపు
	fflush(stdout);
	i->p = the_command;
#లేకపోతే
	పూర్ణాంకం n;
	స్థిర అక్షరం the_command[CONFIG_SYS_CBSIZE + 1];

	bootretry_reset_cmd_timeout();
	i->__promptme = 1;
	n = uboot_cli_readline(i);

#నిర్వచితమైతే CONFIG_BOOT_RETRY_సమయం
	ఒకవేళ (n == -2) {
	  puts("\nTimeout waiting for command\n");
#  ifdef CONFIG_RESET_గమ్యం_RETRY
	  do_reset(ఖాళీ, 0, 0, ఖాళీ);
#  elif IS_ENABLED(CONFIG_RETRY_BOOTCMD)
	strcpy(console_బఫరు, "run bootcmd\n");
# లేకపోతే
#	error "This only works with CONFIG_RESET_TO_RETRY or CONFIG_BOOT_RETRY_COMMAND enabled"
#  endif
	}
#అయితే_ముగింపు
	ఒకవేళ (n == -1 ) {
		flag_repeat = 0;
		i->__promptme = 0;
	}
	n = అక్షరశ్రేణి_పొడవు(console_బఫరు);
	console_బఫరు[n] = '\n';
	console_బఫరు[n+1]= '\0';
	ఒకవేళ (had_ctrlc()) flag_repeat = 0;
	clear_ctrlc();
	do_repeat = 0;
	ఒకవేళ (i->promptmode == 1) {
		ఒకవేళ (console_బఫరు[0] == '\n'&& flag_repeat == 0) {
			strcpy(the_command,console_బఫరు);
		}
		లేకపోతే {
			ఒకవేళ (console_బఫరు[0] != '\n') {
				strcpy(the_command,console_బఫరు);
				flag_repeat = 1;
			}
			లేకపోతే {
				do_repeat = 1;
			}
		}
		i->p = the_command;
	}
	లేకపోతే {
		ఒకవేళ (console_బఫరు[0] != '\n') {
			ఒకవేళ (అక్షరశ్రేణి_పొడవు(the_command) + అక్షరశ్రేణి_పొడవు(console_బఫరు)
			    < CONFIG_SYS_CBSIZE) {
				n = అక్షరశ్రేణి_పొడవు(the_command);
				the_command[n-1] = ' ';
				strcpy(&the_command[n],console_బఫరు);
			}
			లేకపోతే {
				the_command[0] = '\n';
				the_command[1] = '\0';
				flag_repeat = 0;
			}
		}
		ఒకవేళ (i->__promptme == 0) {
			the_command[0] = '\n';
			the_command[1] = '\0';
		}
		i->p = console_బఫరు;
	}
#అయితే_ముగింపు
}

/* This is the magic location that prints prompts
 * and gets data back from the user */
స్థిర పూర్ణాంకం file_get(నిర్మాణం లోపల_str *i)
{
	పూర్ణాంకం ch;

	ch = 0;
	/* If there is data waiting, eat it up */
	ఒకవేళ (i->p && *i->p) {
		ch = *i->p++;
	} లేకపోతే {
		/* need to double check i->file because we might be doing something
		 * more complicated by now, like sourcing or substituting. */
#నిర్వచితం_కాకపోతే __U_BOOT__
		ఒకవేళ (i->__promptme && interactive && i->file == stdin) {
			అయితే(! i->p || (interactive && అక్షరశ్రేణి_పొడవు(i->p)==0) ) {
#లేకపోతే
			అయితే(! i->p  || అక్షరశ్రేణి_పొడవు(i->p)==0 ) {
#అయితే_ముగింపు
				get_user_ఆదానం(i);
			}
			i->promptmode=2;
#నిర్వచితం_కాకపోతే __U_BOOT__
			i->__promptme = 0;
#అయితే_ముగింపు
			ఒకవేళ (i->p && *i->p) {
				ch = *i->p++;
			}
#నిర్వచితం_కాకపోతే __U_BOOT__
		} లేకపోతే {
			ch = fgetc(i->file);
		}

#అయితే_ముగింపు
		debug_printf("b_getch: got a %d\n", ch);
	}
#నిర్వచితం_కాకపోతే __U_BOOT__
	ఒకవేళ (ch == '\n') i->__promptme=1;
#అయితే_ముగింపు
	తిరిగి ch;
}

/* All the callers guarantee this routine will never be
 * used right after a newline, so prompting is not needed.
 */
స్థిర పూర్ణాంకం file_peek(నిర్మాణం లోపల_str *i)
{
#నిర్వచితం_కాకపోతే __U_BOOT__
	ఒకవేళ (i->p && *i->p) {
#అయితే_ముగింపు
		తిరిగి *i->p;
#నిర్వచితం_కాకపోతే __U_BOOT__
	} లేకపోతే {
		i->peek_buf[0] = fgetc(i->file);
		i->peek_buf[1] = '\0';
		i->p = i->peek_buf;
		debug_printf("b_peek: got a %d\n", *i->p);
		తిరిగి *i->p;
	}
#అయితే_ముగింపు
}

#నిర్వచితం_కాకపోతే __U_BOOT__
స్థిర శూన్య అమరిక_file_లోపల_str(నిర్మాణం లోపల_str *i, FILE *f)
#లేకపోతే
స్థిర శూన్య అమరిక_file_లోపల_str(నిర్మాణం లోపల_str *i)
#అయితే_ముగింపు
{
	i->peek = file_peek;
	i->get = file_get;
	i->__promptme=1;
	i->promptmode=1;
#నిర్వచితం_కాకపోతే __U_BOOT__
	i->file = f;
#అయితే_ముగింపు
	i->p = ఖాళీ;
}

స్థిర శూన్య అమరిక_string_లోపల_str(నిర్మాణం లోపల_str *i, స్థిరరాశి అక్షరం *s)
{
	i->peek = static_peek;
	i->get = static_get;
	i->__promptme=1;
	i->promptmode=1;
	i->p = s;
}

#నిర్వచితం_కాకపోతే __U_BOOT__
స్థిర శూన్య mark_open(పూర్ణాంకం fd)
{
	నిర్మాణం close_me *new = xmalloc(పరిమాణం(నిర్మాణం close_me));
	new->fd = fd;
	new->తదుపరి = close_me_తల;
	close_me_తల = new;
}

స్థిర శూన్య mark_closed(పూర్ణాంకం fd)
{
	నిర్మాణం close_me *tmp;
	ఒకవేళ (close_me_తల == ఖాళీ || close_me_తల->fd != fd)
		error_సందేశం_and_die("corrupt close_me");
	tmp = close_me_తల;
	close_me_తల = close_me_తల->తదుపరి;
	విడుదల(tmp);
}

స్థిర శూన్య close_all(శూన్య)
{
	నిర్మాణం close_me *c;
	కొరకు (c=close_me_తల; c; c=c->తదుపరి) {
		close(c->fd);
	}
	close_me_తల = ఖాళీ;
}

/* squirrel != NULL means we squirrel away copies of stdin, stdout,
 * and stderr if they are redirected. */
స్థిర పూర్ణాంకం అమరిక_redirects(నిర్మాణం బిడ్డ_prog *prog, పూర్ణాంకం squirrel[])
{
	పూర్ణాంకం openfd, బహుళకం;
	నిర్మాణం redir_struct *redir;

	కొరకు (redir=prog->redirects; redir; redir=redir->తదుపరి) {
		ఒకవేళ (redir->dup == -1 && redir->మాట.gl_pathv == ఖాళీ) {
			/* something went wrong in the parse.  Pretend it didn't happen */
			కొనసాగు;
		}
		ఒకవేళ (redir->dup == -1) {
			బహుళకం=redir_పట్టిక[redir->type].బహుళకం;
			openfd = open(redir->మాట.gl_pathv[0], బహుళకం, 0666);
			ఒకవేళ (openfd < 0) {
			/* this could get lost if stderr has been redirected, but
			   bash and ash both lose it as well (though zsh doesn't!) */
				perror_సందేశం("error opening %s", redir->మాట.gl_pathv[0]);
				తిరిగి 1;
			}
		} లేకపోతే {
			openfd = redir->dup;
		}

		ఒకవేళ (openfd != redir->fd) {
			ఒకవేళ (squirrel && redir->fd < 3) {
				squirrel[redir->fd] = dup(redir->fd);
			}
			ఒకవేళ (openfd == -3) {
				close(openfd);
			} లేకపోతే {
				dup2(openfd, redir->fd);
				ఒకవేళ (redir->dup == -1)
					close (openfd);
			}
		}
	}
	తిరిగి 0;
}

స్థిర శూన్య restore_redirects(పూర్ణాంకం squirrel[])
{
	పూర్ణాంకం i, fd;
	కొరకు (i=0; i<3; i++) {
		fd = squirrel[i];
		ఒకవేళ (fd != -1) {
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
స్థిర శూన్య pseudo_exec(నిర్మాణం బిడ్డ_prog *బిడ్డ)
{
	పూర్ణాంకం i, rcode;
	అక్షరం *p;
	నిర్మాణం built_లోపల_command *x;
	ఒకవేళ (బిడ్డ->argv) {
		కొరకు (i=0; is_assignment(బిడ్డ->argv[i]); i++) {
			debug_printf("pid %d environment modification: %s\n",getpid(),బిడ్డ->argv[i]);
			p = చొప్పించు_var_విలువ(బిడ్డ->argv[i]);
			putenv(strdup(p));
			ఒకవేళ (p != బిడ్డ->argv[i]) విడుదల(p);
		}
		బిడ్డ->argv+=i;  /* XXX this hack isn't so horrible, since we are about
					to exit, and therefore don't need to keep data
					structures consistent for free() use. */
		/* If a variable is assigned in a forest, and nobody listens,
		 * was it ever really set?
		 */
		ఒకవేళ (బిడ్డ->argv[0] == ఖాళీ) {
			_exit(EXIT_SUCCESS);
		}

		/*
		 * Check if the command matches any of the builtins.
		 * Depending on context, this might be redundant.  But it's
		 * easier to waste a few CPU cycles than it is to figure out
		 * if this is one of those cases.
		 */
		కొరకు (x = bltins; x->cmd; x++) {
			ఒకవేళ (strcmp(బిడ్డ->argv[0], x->cmd) == 0 ) {
				debug_printf("builtin exec %s\n", బిడ్డ->argv[0]);
				rcode = x->function(బిడ్డ);
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
#నిర్వచితమైతే CONFIG_FEATURE_SH_STANDALONE_SHELL
		{
			పూర్ణాంకం argc_l;
			అక్షరం** argv_l=బిడ్డ->argv;
			అక్షరం *name = బిడ్డ->argv[0];

#నిర్వచితమైతే CONFIG_FEATURE_SH_APPLETS_ALWAYS_WIN
			/* Following discussions from November 2000 on the busybox mailing
			 * list, the default configuration, (without
			 * get_last_path_component()) lets the user force use of an
			 * external command by specifying the full (with slashes) filename.
			 * If you enable CONFIG_FEATURE_SH_APPLETS_ALWAYS_WIN then applets
			 * _aways_ override external commands, so if you want to run
			 * /bin/cat, it will use BusyBox cat even if /bin/cat exists on the
			 * filesystem and is _not_ busybox.  Some systems may want this,
			 * most do not.  */
			name = get_చివరి_మార్గం_component(name);
#అయితే_ముగింపు
			/* Count argc for use in a second... */
			కొరకు(argc_l=0;*argv_l!=ఖాళీ; argv_l++, argc_l++);
			optind = 1;
			debug_printf("running applet %s\n", name);
			నిరంతరం_applet_by_name(name, argc_l, బిడ్డ->argv);
		}
#అయితే_ముగింపు
		debug_printf("exec of %s\n",బిడ్డ->argv[0]);
		execvp(బిడ్డ->argv[0],బిడ్డ->argv);
		perror_సందేశం("couldn't exec: %s",బిడ్డ->argv[0]);
		_exit(1);
	} లేకపోతే ఒకవేళ (బిడ్డ->సమూహం) {
		debug_printf("runtime nesting to group\n");
		interactive=0;    /* crucial!!!! */
		rcode = నిరంతరం_list_real(బిడ్డ->సమూహం);
		/* OK to leak memory by not calling free_pipe_list,
		 * since this process is about to exit */
		_exit(rcode);
	} లేకపోతే {
		/* Can happen.  See what bash does with ">foo" by itself. */
		debug_printf("trying to pseudo_exec null command\n");
		_exit(EXIT_SUCCESS);
	}
}

స్థిర శూన్య చొప్పించు_bg_పని(నిర్మాణం pipe *pi)
{
	నిర్మాణం pipe *thejob;

	/* Linear search for the ID of the job to use */
	pi->jobid = 1;
	కొరకు (thejob = పని_list; thejob; thejob = thejob->తదుపరి)
		ఒకవేళ (thejob->jobid >= pi->jobid)
			pi->jobid = thejob->jobid + 1;

	/* add thejob to the list of running jobs */
	ఒకవేళ (!పని_list) {
		thejob = పని_list = xmalloc(పరిమాణం(*thejob));
	} లేకపోతే {
		కొరకు (thejob = పని_list; thejob->తదుపరి; thejob = thejob->తదుపరి) /* nothing */;
		thejob->తదుపరి = xmalloc(పరిమాణం(*thejob));
		thejob = thejob->తదుపరి;
	}

	/* physically copy the struct job */
	జ్ఞాపకం_నకలు(thejob, pi, పరిమాణం(నిర్మాణం pipe));
	thejob->తదుపరి = ఖాళీ;
	thejob->running_progs = thejob->అంకె_progs;
	thejob->stopped_progs = 0;
	thejob->పాఠ్యం = xmalloc(BUFSIZ); /* cmdedit buffer size */

	/*if (pi->progs[0] && pi->progs[0].argv && pi->progs[0].argv[0]) */
	{
		అక్షరం *bar=thejob->పాఠ్యం;
		అక్షరం **foo=pi->progs[0].argv;
		అయితే(foo && *foo) {
			bar += sprintf(bar, "%s ", *foo++);
		}
	}

	/* we don't wait for background thejobs to return -- append it
	   to the list of backgrounded thejobs and leave it alone */
	రాయి("[%d] %d\n", thejob->jobid, thejob->progs[0].pid);
	చివరి_bg_pid = thejob->progs[0].pid;
	చివరి_jobid = thejob->jobid;
}

/* remove a backgrounded job */
స్థిర శూన్య remove_bg_పని(నిర్మాణం pipe *pi)
{
	నిర్మాణం pipe *prev_pipe;

	ఒకవేళ (pi == పని_list) {
		పని_list = pi->తదుపరి;
	} లేకపోతే {
		prev_pipe = పని_list;
		అయితే (prev_pipe->తదుపరి != pi)
			prev_pipe = prev_pipe->తదుపరి;
		prev_pipe->తదుపరి = pi->తదుపరి;
	}
	ఒకవేళ (పని_list)
		చివరి_jobid = పని_list->jobid;
	లేకపోతే
		చివరి_jobid = 0;

	pi->stopped_progs = 0;
	free_pipe(pi, 0);
	విడుదల(pi);
}

/* Checks to see if any processes have exited -- if they
   have, figure out why and see if a job has completed */
స్థిర పూర్ణాంకం checkjobs(నిర్మాణం pipe* fg_pipe)
{
	పూర్ణాంకం లక్షణాలు;
	పూర్ణాంకం స్థితి_2;
	పూర్ణాంకం prognum = 0;
	నిర్మాణం pipe *pi;
	pid_t childpid;

	లక్షణాలు = WUNTRACED;
	ఒకవేళ (fg_pipe==ఖాళీ) {
		లక్షణాలు |= WNOHANG;
	}

	అయితే ((childpid = waitpid(-1, &స్థితి_2, లక్షణాలు)) > 0) {
		ఒకవేళ (fg_pipe) {
			పూర్ణాంకం i, rcode = 0;
			కొరకు (i=0; i < fg_pipe->అంకె_progs; i++) {
				ఒకవేళ (fg_pipe->progs[i].pid == childpid) {
					ఒకవేళ (i==fg_pipe->అంకె_progs-1)
						rcode=WEXITSTATUS(స్థితి_2);
					(fg_pipe->అంకె_progs)--;
					తిరిగి(rcode);
				}
			}
		}

		కొరకు (pi = పని_list; pi; pi = pi->తదుపరి) {
			prognum = 0;
			అయితే (prognum < pi->అంకె_progs && pi->progs[prognum].pid != childpid) {
				prognum++;
			}
			ఒకవేళ (prognum < pi->అంకె_progs)
				ఆపు;
		}

		ఒకవేళ(pi==ఖాళీ) {
			debug_printf("checkjobs: pid %d was not in our list!\n", childpid);
			కొనసాగు;
		}

		ఒకవేళ (WIFEXITED(స్థితి_2) || WIFSIGNALED(స్థితి_2)) {
			/* child exited */
			pi->running_progs--;
			pi->progs[prognum].pid = 0;

			ఒకవేళ (!pi->running_progs) {
				రాయి(పని_స్థితి_FORMAT, pi->jobid, "Done", pi->పాఠ్యం);
				remove_bg_పని(pi);
			}
		} లేకపోతే {
			/* child stopped */
			pi->stopped_progs++;
			pi->progs[prognum].is_stopped = 1;

#ఒకవేళ 0
			/* Printing this stuff is a pain, since it tends to
			 * overwrite the prompt an inconveinient moments.  So
			 * don't do that.  */
			ఒకవేళ (pi->stopped_progs == pi->అంకె_progs) {
				రాయి("\n"పని_స్థితి_FORMAT, pi->jobid, "Stopped", pi->పాఠ్యం);
			}
#అయితే_ముగింపు
		}
	}

	ఒకవేళ (childpid == -1 && errno != ECHILD)
		perror_సందేశం("waitpid");

	/* move the shell to the foreground */
	/*if (interactive && tcsetpgrp(shell_terminal, getpgid(0))) */
	/*	perror_msg("tcsetpgrp-2"); */
	తిరిగి -1;
}

/* Figure out our controlling tty, checking in order stderr,
 * stdin, and stdout.  If check_pgrp is set, also check that
 * we belong to the foreground process group associated with
 * that tty.  The value of shell_terminal is needed in order to call
 * tcsetpgrp(shell_terminal, ...); */
శూన్య controlling_tty(పూర్ణాంకం check_pgrp)
{
	pid_t curpgrp;

	ఒకవేళ ((curpgrp = tcgetpgrp(shell_terminal = 2)) < 0
			&& (curpgrp = tcgetpgrp(shell_terminal = 0)) < 0
			&& (curpgrp = tcgetpgrp(shell_terminal = 1)) < 0)
		వెళ్ళు shell_terminal_error;

	ఒకవేళ (check_pgrp && curpgrp != getpgid(0))
		వెళ్ళు shell_terminal_error;

	తిరిగి;

shell_terminal_error:
		shell_terminal = -1;
		తిరిగి;
}
#అయితే_ముగింపు

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
స్థిర పూర్ణాంకం నిరంతరం_pipe_real(నిర్మాణం pipe *pi)
{
	పూర్ణాంకం i;
#నిర్వచితం_కాకపోతే __U_BOOT__
	పూర్ణాంకం nextin, nextout;
	పూర్ణాంకం pipefds[2];				/* pipefds[0] is for reading */
	నిర్మాణం బిడ్డ_prog *బిడ్డ;
	నిర్మాణం built_లోపల_command *x;
	అక్షరం *p;
# ఒకవేళ __GNUC__
	/* Avoid longjmp clobbering */
	(శూన్య) &i;
	(శూన్య) &nextin;
	(శూన్య) &nextout;
	(శూన్య) &బిడ్డ;
# endif
#లేకపోతే
	పూర్ణాంకం nextin;
	పూర్ణాంకం flag = do_repeat ? CMD_FLAG_REPEAT : 0;
	నిర్మాణం బిడ్డ_prog *బిడ్డ;
	అక్షరం *p;
# ఒకవేళ __GNUC__
	/* Avoid longjmp clobbering */
	(శూన్య) &i;
	(శూన్య) &nextin;
	(శూన్య) &బిడ్డ;
# endif
#అయితే_ముగింపు	/* __U_BOOT__ */

	nextin = 0;
#నిర్వచితం_కాకపోతే __U_BOOT__
	pi->pgrp = -1;
#అయితే_ముగింపు

	/* Check if this is a simple builtin (not part of a pipe).
	 * Builtins within pipes have to fork anyway, and are handled in
	 * pseudo_exec.  "echo foo | read bar" doesn't work on bash, either.
	 */
	ఒకవేళ (pi->అంకె_progs == 1) బిడ్డ = & (pi->progs[0]);
#నిర్వచితం_కాకపోతే __U_BOOT__
	ఒకవేళ (pi->అంకె_progs == 1 && బిడ్డ->సమూహం && బిడ్డ->subshell == 0) {
		పూర్ణాంకం squirrel[] = {-1, -1, -1};
		పూర్ణాంకం rcode;
		debug_printf("non-subshell grouping\n");
		అమరిక_redirects(బిడ్డ, squirrel);
		/* XXX could we merge code with following builtin case,
		 * by creating a pseudo builtin that calls run_list_real? */
		rcode = నిరంతరం_list_real(బిడ్డ->సమూహం);
		restore_redirects(squirrel);
#లేకపోతే
		ఒకవేళ (pi->అంకె_progs == 1 && బిడ్డ->సమూహం) {
		పూర్ణాంకం rcode;
		debug_printf("non-subshell grouping\n");
		rcode = నిరంతరం_list_real(బిడ్డ->సమూహం);
#అయితే_ముగింపు
		తిరిగి rcode;
	} లేకపోతే ఒకవేళ (pi->అంకె_progs == 1 && pi->progs[0].argv != ఖాళీ) {
		కొరకు (i=0; is_assignment(బిడ్డ->argv[i]); i++) { /* nothing */ }
		ఒకవేళ (i!=0 && బిడ్డ->argv[i]==ఖాళీ) {
			/* assignments, but no command: set the local environment */
			కొరకు (i=0; బిడ్డ->argv[i]!=ఖాళీ; i++) {

				/* Ok, this case is tricky.  We have to decide if this is a
				 * local variable, or an already exported variable.  If it is
				 * already exported, we have to export the new value.  If it is
				 * not exported, we need only set this as a local variable.
				 * This junk is all to decide whether or not to export this
				 * variable. */
				పూర్ణాంకం export_me=0;
				అక్షరం *name, *విలువ;
				name = xstrdup(బిడ్డ->argv[i]);
				debug_printf("Local environment set: %s\n", name);
				విలువ = strchr(name, '=');
				ఒకవేళ (విలువ)
					*విలువ=0;
#నిర్వచితం_కాకపోతే __U_BOOT__
				ఒకవేళ ( get_local_var(name)) {
					export_me=1;
				}
#అయితే_ముగింపు
				విడుదల(name);
				p = చొప్పించు_var_విలువ(బిడ్డ->argv[i]);
				సమితి_local_var(p, export_me);
				ఒకవేళ (p != బిడ్డ->argv[i]) విడుదల(p);
			}
			తిరిగి EXIT_SUCCESS;   /* don't worry about errors in set_local_var() yet */
		}
		కొరకు (i = 0; is_assignment(బిడ్డ->argv[i]); i++) {
			p = చొప్పించు_var_విలువ(బిడ్డ->argv[i]);
#నిర్వచితం_కాకపోతే __U_BOOT__
			putenv(strdup(p));
#లేకపోతే
			సమితి_local_var(p, 0);
#అయితే_ముగింపు
			ఒకవేళ (p != బిడ్డ->argv[i]) {
				బిడ్డ->sp--;
				విడుదల(p);
			}
		}
		ఒకవేళ (బిడ్డ->sp) {
			అక్షరం * str = ఖాళీ;

			str = make_string(బిడ్డ->argv + i,
					  బిడ్డ->argv_nonnull + i);
			parse_string_outer(str, FLAG_EXIT_FROM_LOOP | FLAG_REPARSING);
			విడుదల(str);
			తిరిగి చివరి_return_సంకేతం;
		}
#నిర్వచితం_కాకపోతే __U_BOOT__
		కొరకు (x = bltins; x->cmd; x++) {
			ఒకవేళ (strcmp(బిడ్డ->argv[i], x->cmd) == 0 ) {
				పూర్ణాంకం squirrel[] = {-1, -1, -1};
				పూర్ణాంకం rcode;
				ఒకవేళ (x->function == builtin_exec && బిడ్డ->argv[i+1]==ఖాళీ) {
					debug_printf("magic exec\n");
					అమరిక_redirects(బిడ్డ,ఖాళీ);
					తిరిగి EXIT_SUCCESS;
				}
				debug_printf("builtin inline %s\n", బిడ్డ->argv[0]);
				/* XXX setup_redirects acts on file descriptors, not FILEs.
				 * This is perfect for work that comes after exec().
				 * Is it really safe for inline use?  Experimentally,
				 * things seem to work with glibc. */
				అమరిక_redirects(బిడ్డ, squirrel);

				బిడ్డ->argv += i;  /* XXX horrible hack */
				rcode = x->function(బిడ్డ);
				/* XXX restore hack so free() can work right */
				బిడ్డ->argv -= i;
				restore_redirects(squirrel);
			}
			తిరిగి rcode;
		}
#లేకపోతే
		/* check ";", because ,example , argv consist from
		 * "help;flinfo" must not execute
		 */
		ఒకవేళ (strchr(బిడ్డ->argv[i], ';')) {
			రాయి("Unknown command '%s' - try 'help' or use "
					"'run' command\n", బిడ్డ->argv[i]);
			తిరిగి -1;
		}
		/* Process the command */
		తిరిగి cmd_process(flag, బిడ్డ->argc - i, బిడ్డ->argv + i,
				   &flag_repeat, ఖాళీ);
#అయితే_ముగింపు
	}
#నిర్వచితం_కాకపోతే __U_BOOT__

	కొరకు (i = 0; i < pi->అంకె_progs; i++) {
		బిడ్డ = & (pi->progs[i]);

		/* pipes are inserted between pairs of commands */
		ఒకవేళ ((i + 1) < pi->అంకె_progs) {
			ఒకవేళ (pipe(pipefds)<0) perror_సందేశం_and_die("pipe");
			nextout = pipefds[1];
		} లేకపోతే {
			nextout=1;
			pipefds[0] = -1;
		}

		/* XXX test for failed fork()? */
		ఒకవేళ (!(బిడ్డ->pid = fork())) {
			/* Set the handling for job control signals back to the default.  */
			సంకేతం(SIGINT, SIG_DFL);
			సంకేతం(SIGQUIT, SIG_DFL);
			సంకేతం(SIGTERM, SIG_DFL);
			సంకేతం(SIGTSTP, SIG_DFL);
			సంకేతం(SIGTTIN, SIG_DFL);
			సంకేతం(SIGTTOU, SIG_DFL);
			సంకేతం(SIGCHLD, SIG_DFL);

			close_all();

			ఒకవేళ (nextin != 0) {
				dup2(nextin, 0);
				close(nextin);
			}
			ఒకవేళ (nextout != 1) {
				dup2(nextout, 1);
				close(nextout);
			}
			ఒకవేళ (pipefds[0]!=-1) {
				close(pipefds[0]);  /* opposite end of our output pipe */
			}

			/* Like bash, explicit redirects override pipes,
			 * and the pipe fd is available for dup'ing. */
			అమరిక_redirects(బిడ్డ,ఖాళీ);

			ఒకవేళ (interactive && pi->followup!=PIPE_BG) {
				/* If we (the child) win the race, put ourselves in the process
				 * group whose leader is the first process in this pipe. */
				ఒకవేళ (pi->pgrp < 0) {
					pi->pgrp = getpid();
				}
				ఒకవేళ (setpgid(0, pi->pgrp) == 0) {
					tcsetpgrp(2, pi->pgrp);
				}
			}

			pseudo_exec(బిడ్డ);
		}

		/* put our child in the process group whose leader is the
		   first process in this pipe */
		ఒకవేళ (pi->pgrp < 0) {
			pi->pgrp = బిడ్డ->pid;
		}
		/* Don't check for errors.  The child may be dead already,
		 * in which case setpgid returns error code EACCES. */
		setpgid(బిడ్డ->pid, pi->pgrp);

		ఒకవేళ (nextin != 0)
			close(nextin);
		ఒకవేళ (nextout != 1)
			close(nextout);

		/* If there isn't another process, nextin is garbage
		   but it doesn't matter */
		nextin = pipefds[0];
	}
#అయితే_ముగింపు
	తిరిగి -1;
}

స్థిర పూర్ణాంకం నిరంతరం_list_real(నిర్మాణం pipe *pi)
{
	అక్షరం *save_name = ఖాళీ;
	అక్షరం **list = ఖాళీ;
	అక్షరం **save_list = ఖాళీ;
	నిర్మాణం pipe *rpipe;
	పూర్ణాంకం flag_rep = 0;
#నిర్వచితం_కాకపోతే __U_BOOT__
	పూర్ణాంకం save_అంకె_progs;
#అయితే_ముగింపు
	పూర్ణాంకం rcode=0, flag_skip=1;
	పూర్ణాంకం flag_restore = 0;
	పూర్ణాంకం if_సంకేతం=0, తదుపరి_if_సంకేతం=0;  /* need double-buffer to handle elif */
	reserved_style rmode, skip_మరిన్ని_లోపల_this_rmode=RES_XXXX;
	/* check syntax for "for" */
	కొరకు (rpipe = pi; rpipe; rpipe = rpipe->తదుపరి) {
		ఒకవేళ ((rpipe->r_బహుళకం == RES_లోపల ||
		    rpipe->r_బహుళకం == RES_FOR) &&
		    (rpipe->తదుపరి == ఖాళీ)) {
				syntax();
#నిర్వచితమైతే __U_BOOT__
				flag_repeat = 0;
#అయితే_ముగింపు
				తిరిగి 1;
		}
		ఒకవేళ ((rpipe->r_బహుళకం == RES_లోపల &&
			(rpipe->తదుపరి->r_బహుళకం == RES_లోపల &&
			rpipe->తదుపరి->progs->argv != ఖాళీ))||
			(rpipe->r_బహుళకం == RES_FOR &&
			rpipe->తదుపరి->r_బహుళకం != RES_లోపల)) {
				syntax();
#నిర్వచితమైతే __U_BOOT__
				flag_repeat = 0;
#అయితే_ముగింపు
				తిరిగి 1;
		}
	}
	కొరకు (; pi; pi = (flag_restore != 0) ? rpipe : pi->తదుపరి) {
		ఒకవేళ (pi->r_బహుళకం == RES_WHILE || pi->r_బహుళకం == RES_UNTIL ||
			pi->r_బహుళకం == RES_FOR) {
#నిర్వచితమైతే __U_BOOT__
				/* check Ctrl-C */
				ctrlc();
				ఒకవేళ ((had_ctrlc())) {
					తిరిగి 1;
				}
#అయితే_ముగింపు
				flag_restore = 0;
				ఒకవేళ (!rpipe) {
					flag_rep = 0;
					rpipe = pi;
				}
		}
		rmode = pi->r_బహుళకం;
		debug_printf("rmode=%d  if_code=%d  next_if_code=%d skip_more=%d\n", rmode, if_సంకేతం, తదుపరి_if_సంకేతం, skip_మరిన్ని_లోపల_this_rmode);
		ఒకవేళ (rmode == skip_మరిన్ని_లోపల_this_rmode && flag_skip) {
			ఒకవేళ (pi->followup == PIPE_SEQ) flag_skip=0;
			కొనసాగు;
		}
		flag_skip = 1;
		skip_మరిన్ని_లోపల_this_rmode = RES_XXXX;
		ఒకవేళ (rmode == RES_THEN || rmode == RES_ELSE) if_సంకేతం = తదుపరి_if_సంకేతం;
		ఒకవేళ (rmode == RES_THEN &&  if_సంకేతం) కొనసాగు;
		ఒకవేళ (rmode == RES_ELSE && !if_సంకేతం) కొనసాగు;
		ఒకవేళ (rmode == RES_ELIF && !if_సంకేతం) ఆపు;
		ఒకవేళ (rmode == RES_FOR && pi->అంకె_progs) {
			ఒకవేళ (!list) {
				/* if no variable values after "in" we skip "for" */
				ఒకవేళ (!pi->తదుపరి->progs->argv) కొనసాగు;
				/* create list of variable values */
				list = make_list_లోపల(pi->తదుపరి->progs->argv,
					pi->progs->argv[0]);
				save_list = list;
				save_name = pi->progs->argv[0];
				pi->progs->argv[0] = ఖాళీ;
				flag_rep = 1;
			}
			ఒకవేళ (!(*list)) {
				విడుదల(pi->progs->argv[0]);
				విడుదల(save_list);
				list = ఖాళీ;
				flag_rep = 0;
				pi->progs->argv[0] = save_name;
#నిర్వచితం_కాకపోతే __U_BOOT__
				pi->progs->glob_ఫలితం.gl_pathv[0] =
					pi->progs->argv[0];
#అయితే_ముగింపు
				కొనసాగు;
			} లేకపోతే {
				/* insert new value from list for variable */
				విడుదల(pi->progs->argv[0]);
				pi->progs->argv[0] = *list++;
#నిర్వచితం_కాకపోతే __U_BOOT__
				pi->progs->glob_ఫలితం.gl_pathv[0] =
					pi->progs->argv[0];
#అయితే_ముగింపు
			}
		}
		ఒకవేళ (rmode == RES_లోపల) కొనసాగు;
		ఒకవేళ (rmode == RES_DO) {
			ఒకవేళ (!flag_rep) కొనసాగు;
		}
		ఒకవేళ (rmode == RES_పూర్తి) {
			ఒకవేళ (flag_rep) {
				flag_restore = 1;
			} లేకపోతే {
				rpipe = ఖాళీ;
			}
		}
		ఒకవేళ (pi->అంకె_progs == 0) కొనసాగు;
#నిర్వచితం_కాకపోతే __U_BOOT__
		save_అంకె_progs = pi->అంకె_progs; /* save number of programs */
#అయితే_ముగింపు
		rcode = నిరంతరం_pipe_real(pi);
		debug_printf("run_pipe_real returned %d\n",rcode);
#నిర్వచితం_కాకపోతే __U_BOOT__
		ఒకవేళ (rcode!=-1) {
			/* We only ran a builtin: rcode was set by the return value
			 * of run_pipe_real(), and we don't need to wait for anything. */
		} లేకపోతే ఒకవేళ (pi->followup==PIPE_BG) {
			/* XXX check bash's behavior with nontrivial pipes */
			/* XXX compute jobid */
			/* XXX what does bash do with attempts to background builtins? */
			చొప్పించు_bg_పని(pi);
			rcode = EXIT_SUCCESS;
		} లేకపోతే {
			ఒకవేళ (interactive) {
				/* move the new process group into the foreground */
				ఒకవేళ (tcsetpgrp(shell_terminal, pi->pgrp) && errno != ENOTTY)
					perror_సందేశం("tcsetpgrp-3");
				rcode = checkjobs(pi);
				/* move the shell to the foreground */
				ఒకవేళ (tcsetpgrp(shell_terminal, getpgid(0)) && errno != ENOTTY)
					perror_సందేశం("tcsetpgrp-4");
			} లేకపోతే {
				rcode = checkjobs(pi);
			}
			debug_printf("checkjobs returned %d\n",rcode);
		}
		చివరి_return_సంకేతం=rcode;
#లేకపోతే
		ఒకవేళ (rcode < -1) {
			చివరి_return_సంకేతం = -rcode - 2;
			తిరిగి -2;	/* exit */
		}
		చివరి_return_సంకేతం = rcode;
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే __U_BOOT__
		pi->అంకె_progs = save_అంకె_progs; /* restore number of programs */
#అయితే_ముగింపు
		ఒకవేళ ( rmode == RES_IF || rmode == RES_ELIF )
			తదుపరి_if_సంకేతం=rcode;  /* can be overwritten a number of times */
		ఒకవేళ (rmode == RES_WHILE)
			flag_rep = !చివరి_return_సంకేతం;
		ఒకవేళ (rmode == RES_UNTIL)
			flag_rep = చివరి_return_సంకేతం;
		ఒకవేళ ( (rcode==EXIT_SUCCESS && pi->followup==PIPE_OR) ||
		     (rcode!=EXIT_SUCCESS && pi->followup==PIPE_AND) )
			skip_మరిన్ని_లోపల_this_rmode=rmode;
#నిర్వచితం_కాకపోతే __U_BOOT__
		checkjobs(ఖాళీ);
#అయితే_ముగింపు
	}
	తిరిగి rcode;
}

/* broken, of course, but OK for testing */
స్థిర అక్షరం *indenter(పూర్ణాంకం i)
{
	స్థిర అక్షరం blanks[]="                                    ";
	తిరిగి &blanks[పరిమాణం(blanks)-i-1];
}

/* return code is the exit status of the pipe */
స్థిర పూర్ణాంకం free_pipe(నిర్మాణం pipe *pi, పూర్ణాంకం indent)
{
	అక్షరం **p;
	నిర్మాణం బిడ్డ_prog *బిడ్డ;
#నిర్వచితం_కాకపోతే __U_BOOT__
	నిర్మాణం redir_struct *r, *rnext;
#అయితే_ముగింపు
	పూర్ణాంకం a, i, ret_సంకేతం=0;
	అక్షరం *ind = indenter(indent);

#నిర్వచితం_కాకపోతే __U_BOOT__
	ఒకవేళ (pi->stopped_progs > 0)
		తిరిగి ret_సంకేతం;
	final_printf("%s run pipe: (pid %d)\n",ind,getpid());
#అయితే_ముగింపు
	కొరకు (i=0; i<pi->అంకె_progs; i++) {
		బిడ్డ = &pi->progs[i];
		final_printf("%s  command %d:\n",ind,i);
		ఒకవేళ (బిడ్డ->argv) {
			కొరకు (a=0,p=బిడ్డ->argv; *p; a++,p++) {
				final_printf("%s   argv[%d] = %s\n",ind,a,*p);
			}
#నిర్వచితం_కాకపోతే __U_BOOT__
			globfree(&బిడ్డ->glob_ఫలితం);
#లేకపోతే
			కొరకు (a = 0; a < బిడ్డ->argc; a++) {
				విడుదల(బిడ్డ->argv[a]);
			}
			విడుదల(బిడ్డ->argv);
			విడుదల(బిడ్డ->argv_nonnull);
			బిడ్డ->argc = 0;
#అయితే_ముగింపు
			బిడ్డ->argv=ఖాళీ;
		} లేకపోతే ఒకవేళ (బిడ్డ->సమూహం) {
#నిర్వచితం_కాకపోతే __U_BOOT__
			final_printf("%s   begin group (subshell:%d)\n",ind, బిడ్డ->subshell);
#అయితే_ముగింపు
			ret_సంకేతం = free_pipe_list(బిడ్డ->సమూహం,indent+3);
			final_printf("%s   end group\n",ind);
		} లేకపోతే {
			final_printf("%s   (nil)\n",ind);
		}
#నిర్వచితం_కాకపోతే __U_BOOT__
		కొరకు (r=బిడ్డ->redirects; r; r=rnext) {
			final_printf("%s   redirect %d%s", ind, r->fd, redir_పట్టిక[r->type].descrip);
			ఒకవేళ (r->dup == -1) {
				/* guard against the case >$FOO, where foo is unset or blank */
				ఒకవేళ (r->మాట.gl_pathv) {
					final_printf(" %s\n", *r->మాట.gl_pathv);
					globfree(&r->మాట);
				}
			} లేకపోతే {
				final_printf("&%d\n", r->dup);
			}
			rnext=r->తదుపరి;
			విడుదల(r);
		}
		బిడ్డ->redirects=ఖాళీ;
#అయితే_ముగింపు
	}
	విడుదల(pi->progs);   /* children are an array, they get freed all at once */
	pi->progs=ఖాళీ;
	తిరిగి ret_సంకేతం;
}

స్థిర పూర్ణాంకం free_pipe_list(నిర్మాణం pipe *తల, పూర్ణాంకం indent)
{
	పూర్ణాంకం rcode=0;   /* if list has no members */
	నిర్మాణం pipe *pi, *తదుపరి;
	అక్షరం *ind = indenter(indent);
	కొరకు (pi=తల; pi; pi=తదుపరి) {
		final_printf("%s pipe reserved mode %d\n", ind, pi->r_బహుళకం);
		rcode = free_pipe(pi, indent);
		final_printf("%s pipe followup code %d\n", ind, pi->followup);
		తదుపరి=pi->తదుపరి;
		pi->తదుపరి=ఖాళీ;
		విడుదల(pi);
	}
	తిరిగి rcode;
}

/* Select which version we will use */
స్థిర పూర్ణాంకం నిరంతరం_list(నిర్మాణం pipe *pi)
{
	పూర్ణాంకం rcode=0;
#నిర్వచితం_కాకపోతే __U_BOOT__
	ఒకవేళ (fake_బహుళకం==0) {
#అయితే_ముగింపు
		rcode = నిరంతరం_list_real(pi);
#నిర్వచితం_కాకపోతే __U_BOOT__
	}
#అయితే_ముగింపు
	/* free_pipe_list has the side effect of clearing memory
	 * In the long run that function can be merged with run_list_real,
	 * but doing that now would hobble the debugging effort. */
	free_pipe_list(pi,0);
	తిరిగి rcode;
}

/* The API for glob is arguably broken.  This routine pushes a non-matching
 * string into the output structure, removing non-backslashed backslashes.
 * If someone can prove me wrong, by performing this function within the
 * original glob(3) api, feel free to rewrite this routine into oblivion.
 * Return code (0 vs. GLOB_NOSPACE) matches glob(3).
 * XXX broken if the last character is '\\', check that before calling.
 */
#నిర్వచితం_కాకపోతే __U_BOOT__
స్థిర పూర్ణాంకం globhack(స్థిరరాశి అక్షరం *src, పూర్ణాంకం flags, glob_t *pglob)
{
	పూర్ణాంకం cnt=0, pathc;
	స్థిరరాశి అక్షరం *s;
	అక్షరం *dest;
	కొరకు (cnt=1, s=src; s && *s; s++) {
		ఒకవేళ (*s == '\\') s++;
		cnt++;
	}
	dest = జ్ఞాపకం_ఇవ్వు(cnt);
	ఒకవేళ (!dest) తిరిగి GLOB_NOSPACE;
	ఒకవేళ (!(flags & GLOB_APPEND)) {
		pglob->gl_pathv=ఖాళీ;
		pglob->gl_pathc=0;
		pglob->gl_offs=0;
		pglob->gl_offs=0;
	}
	pathc = ++pglob->gl_pathc;
	pglob->gl_pathv = realloc(pglob->gl_pathv, (pathc+1)*పరిమాణం(*pglob->gl_pathv));
	ఒకవేళ (pglob->gl_pathv == ఖాళీ) తిరిగి GLOB_NOSPACE;
	pglob->gl_pathv[pathc-1]=dest;
	pglob->gl_pathv[pathc]=ఖాళీ;
	కొరకు (s=src; s && *s; s++, dest++) {
		ఒకవేళ (*s == '\\') s++;
		*dest = *s;
	}
	*dest='\0';
	తిరిగి 0;
}

/* XXX broken if the last character is '\\', check that before calling */
స్థిర పూర్ణాంకం glob_needed(స్థిరరాశి అక్షరం *s)
{
	కొరకు (; *s; s++) {
		ఒకవేళ (*s == '\\') s++;
		ఒకవేళ (strchr("*[?",*s)) తిరిగి 1;
	}
	తిరిగి 0;
}

#ఒకవేళ 0
స్థిర శూన్య globprint(glob_t *pglob)
{
	పూర్ణాంకం i;
	debug_printf("glob_t at %p:\n", pglob);
	debug_printf("  gl_pathc=%d  gl_pathv=%p  gl_offs=%d  gl_flags=%d\n",
		pglob->gl_pathc, pglob->gl_pathv, pglob->gl_offs, pglob->gl_flags);
	కొరకు (i=0; i<pglob->gl_pathc; i++)
		debug_printf("pglob->gl_pathv[%d] = %p = %s\n", i,
			pglob->gl_pathv[i], pglob->gl_pathv[i]);
}
#అయితే_ముగింపు

స్థిర పూర్ణాంకం xglob(o_string *dest, పూర్ణాంకం flags, glob_t *pglob)
{
	పూర్ణాంకం gr;

	/* short-circuit for null word */
	/* we can code this better when the debug_printf's are gone */
	ఒకవేళ (dest->నిడివి == 0) {
		ఒకవేళ (dest->nonnull) {
			/* bash man page calls this an "explicit" null */
			gr = globhack(dest->దత్తాంశం, flags, pglob);
			debug_printf("globhack returned %d\n",gr);
		} లేకపోతే {
			తిరిగి 0;
		}
	} లేకపోతే ఒకవేళ (glob_needed(dest->దత్తాంశం)) {
		gr = glob(dest->దత్తాంశం, flags, ఖాళీ, pglob);
		debug_printf("glob returned %d\n",gr);
		ఒకవేళ (gr == GLOB_NOMATCH) {
			/* quote removal, or more accurately, backslash removal */
			gr = globhack(dest->దత్తాంశం, flags, pglob);
			debug_printf("globhack returned %d\n",gr);
		}
	} లేకపోతే {
		gr = globhack(dest->దత్తాంశం, flags, pglob);
		debug_printf("globhack returned %d\n",gr);
	}
	ఒకవేళ (gr == GLOB_NOSPACE)
		error_సందేశం_and_die("out of memory during glob");
	ఒకవేళ (gr != 0) { /* GLOB_ABORTED ? */
		error_సందేశం("glob(3) error %d",gr);
	}
	/* globprint(glob_target); */
	తిరిగి gr;
}
#అయితే_ముగింపు

#నిర్వచితమైతే __U_BOOT__
స్థిర అక్షరం *get_dollar_var(అక్షరం ch);
#అయితే_ముగింపు

/* This is used to get/check local shell variables */
అక్షరం *get_local_var(స్థిరరాశి అక్షరం *s)
{
	నిర్మాణం variables *cur;

	ఒకవేళ (!s)
		తిరిగి ఖాళీ;

#నిర్వచితమైతే __U_BOOT__
	ఒకవేళ (*s == '$')
		తిరిగి get_dollar_var(s[1]);
#అయితే_ముగింపు

	కొరకు (cur = శిఖరం_vars; cur; cur=cur->తదుపరి)
		ఒకవేళ(strcmp(cur->name, s)==0)
			తిరిగి cur->విలువ;
	తిరిగి ఖాళీ;
}

/* This is used to set local shell variables
   flg_export==0 if only local (not exporting) variable
   flg_export==1 if "new" exporting environ
   flg_export>1  if current startup environ (not call putenv()) */
పూర్ణాంకం సమితి_local_var(స్థిరరాశి అక్షరం *s, పూర్ణాంకం flg_export)
{
	అక్షరం *name, *విలువ;
	పూర్ణాంకం ఫలితం=0;
	నిర్మాణం variables *cur;

#నిర్వచితమైతే __U_BOOT__
	/* might be possible! */
	ఒకవేళ (!అక్షరమా(*s))
		తిరిగి -1;
#అయితే_ముగింపు

	name=strdup(s);

	/* Assume when we enter this function that we are already in
	 * NAME=VALUE format.  So the first order of business is to
	 * split 's' on the '=' into 'name' and 'value' */
	విలువ = strchr(name, '=');
	ఒకవేళ (!విలువ) {
		విడుదల(name);
		తిరిగి -1;
	}
	*విలువ++ = 0;

	ఒకవేళ (!*విలువ) {
		unset_local_var(name);
		విడుదల(name);
		తిరిగి 0;
	}

	కొరకు(cur = శిఖరం_vars; cur; cur = cur->తదుపరి) {
		ఒకవేళ(strcmp(cur->name, name)==0)
			ఆపు;
	}

	ఒకవేళ(cur) {
		ఒకవేళ(strcmp(cur->విలువ, విలువ)==0) {
			ఒకవేళ(flg_export>0 && cur->flg_export==0)
				cur->flg_export=flg_export;
			లేకపోతే
				ఫలితం++;
		} లేకపోతే {
			ఒకవేళ(cur->flg_read_only) {
				error_సందేశం("%s: readonly variable", name);
				ఫలితం = -1;
			} లేకపోతే {
				ఒకవేళ(flg_export>0 || cur->flg_export>1)
					cur->flg_export=1;
				విడుదల(cur->విలువ);

				cur->విలువ = strdup(విలువ);
			}
		}
	} లేకపోతే {
		cur = జ్ఞాపకం_ఇవ్వు(పరిమాణం(నిర్మాణం variables));
		ఒకవేళ(!cur) {
			ఫలితం = -1;
		} లేకపోతే {
			cur->name = strdup(name);
			ఒకవేళ (cur->name == ఖాళీ) {
				విడుదల(cur);
				ఫలితం = -1;
			} లేకపోతే {
				నిర్మాణం variables *bottom = శిఖరం_vars;
				cur->విలువ = strdup(విలువ);
				cur->తదుపరి = ఖాళీ;
				cur->flg_export = flg_export;
				cur->flg_read_only = 0;
				అయితే(bottom->తదుపరి) bottom=bottom->తదుపరి;
				bottom->తదుపరి = cur;
			}
		}
	}

#నిర్వచితం_కాకపోతే __U_BOOT__
	ఒకవేళ(ఫలితం==0 && cur->flg_export==1) {
		*(విలువ-1) = '=';
		ఫలితం = putenv(name);
	} లేకపోతే {
#అయితే_ముగింపు
		విడుదల(name);
#నిర్వచితం_కాకపోతే __U_BOOT__
		ఒకవేళ(ఫలితం>0)            /* equivalent to previous set */
			ఫలితం = 0;
	}
#అయితే_ముగింపు
	తిరిగి ఫలితం;
}

శూన్య unset_local_var(స్థిరరాశి అక్షరం *name)
{
	నిర్మాణం variables *cur;

	ఒకవేళ (name) {
		కొరకు (cur = శిఖరం_vars; cur; cur=cur->తదుపరి) {
			ఒకవేళ(strcmp(cur->name, name)==0)
				ఆపు;
		}
		ఒకవేళ (cur != ఖాళీ) {
			నిర్మాణం variables *తదుపరి = శిఖరం_vars;
			ఒకవేళ(cur->flg_read_only) {
				error_సందేశం("%s: readonly variable", name);
				తిరిగి;
			} లేకపోతే {
#నిర్వచితం_కాకపోతే __U_BOOT__
				ఒకవేళ(cur->flg_export)
					unenv_సమితి(cur->name);
#అయితే_ముగింపు
				విడుదల(cur->name);
				విడుదల(cur->విలువ);
				అయితే (తదుపరి->తదుపరి != cur)
					తదుపరి = తదుపరి->తదుపరి;
				తదుపరి->తదుపరి = cur->తదుపరి;
			}
			విడుదల(cur);
		}
	}
}

స్థిర పూర్ణాంకం is_assignment(స్థిరరాశి అక్షరం *s)
{
	ఒకవేళ (s == ఖాళీ)
		తిరిగి 0;

	ఒకవేళ (!అక్షరమా(*s)) తిరిగి 0;
	++s;
	అయితే(అక్షరమా_అంకెనా(*s) || *s=='_') ++s;
	తిరిగి *s=='=';
}

#నిర్వచితం_కాకపోతే __U_BOOT__
/* the src parameter allows us to peek forward to a possible &n syntax
 * for file descriptor duplication, e.g., "2>&1".
 * Return code is 0 normally, 1 if a syntax error is detected in src.
 * Resource errors (in xmalloc) cause the process to exit */
స్థిర పూర్ణాంకం అమరిక_redirect(నిర్మాణం p_context *ctx, పూర్ణాంకం fd, redir_type style,
	నిర్మాణం లోపల_str *ఆదానం)
{
	నిర్మాణం బిడ్డ_prog *బిడ్డ=ctx->బిడ్డ;
	నిర్మాణం redir_struct *redir = బిడ్డ->redirects;
	నిర్మాణం redir_struct *చివరి_redir=ఖాళీ;

	/* Create a new redir_struct and drop it onto the end of the linked list */
	అయితే(redir) {
		చివరి_redir=redir;
		redir=redir->తదుపరి;
	}
	redir = xmalloc(పరిమాణం(నిర్మాణం redir_struct));
	redir->తదుపరి=ఖాళీ;
	redir->మాట.gl_pathv=ఖాళీ;
	ఒకవేళ (చివరి_redir) {
		చివరి_redir->తదుపరి=redir;
	} లేకపోతే {
		బిడ్డ->redirects=redir;
	}

	redir->type=style;
	redir->fd= (fd==-1) ? redir_పట్టిక[style].default_fd : fd ;

	debug_printf("Redirect type %d%s\n", redir->fd, redir_పట్టిక[style].descrip);

	/* Check for a '2>&1' type redirect */
	redir->dup = redirect_dup_అంకె(ఆదానం);
	ఒకవేళ (redir->dup == -2) తిరిగి 1;  /* syntax error */
	ఒకవేళ (redir->dup != -1) {
		/* Erik had a check here that the file descriptor in question
		 * is legit; I postpone that to "run time"
		 * A "-" representation of "close me" shows up as a -3 here */
		debug_printf("Duplicating redirect '%d>&%d'\n", redir->fd, redir->dup);
	} లేకపోతే {
		/* We do _not_ try to open the file that src points to,
		 * since we need to return and let src be expanded first.
		 * Set ctx->pending_redirect, so we know what to do at the
		 * end of the next parsed word.
		 */
		ctx->pending_redirect = redir;
	}
	తిరిగి 0;
}
#అయితే_ముగింపు

స్థిర నిర్మాణం pipe *new_pipe(శూన్య)
{
	నిర్మాణం pipe *pi;
	pi = xmalloc(పరిమాణం(నిర్మాణం pipe));
	pi->అంకె_progs = 0;
	pi->progs = ఖాళీ;
	pi->తదుపరి = ఖాళీ;
	pi->followup = 0;  /* invalid */
	pi->r_బహుళకం = RES_NONE;
	తిరిగి pi;
}

స్థిర శూన్య initialize_context(నిర్మాణం p_context *ctx)
{
	ctx->pipe=ఖాళీ;
#నిర్వచితం_కాకపోతే __U_BOOT__
	ctx->pending_redirect=ఖాళీ;
#అయితే_ముగింపు
	ctx->బిడ్డ=ఖాళీ;
	ctx->list_తల=new_pipe();
	ctx->pipe=ctx->list_తల;
	ctx->w=RES_NONE;
	ctx->దొంతర=ఖాళీ;
#నిర్వచితమైతే __U_BOOT__
	ctx->old_flag=0;
#అయితే_ముగింపు
	పూర్తి_command(ctx);   /* creates the memory for working child */
}

/* normal return is 0
 * if a reserved word is found, and processed, return 1
 * should handle if, then, elif, else, fi, for, while, until, do, done.
 * case, function, and select are obnoxious, save those for later.
 */
నిర్మాణం reserved_combo {
	అక్షరం *literal;
	పూర్ణాంకం సంకేతం_2;
	దీర్ఘ flag;
};
/* Mostly a list of accepted follow-up reserved words.
 * FLAG_END means we are done with the sequence, and are ready
 * to turn the compound list into a command.
 * FLAG_START means the word must start a new compound list.
 */
స్థిర నిర్మాణం reserved_combo reserved_list[] = {
	{ "if",    RES_IF,    FLAG_THEN | FLAG_ప్రారంభం },
	{ "then",  RES_THEN,  FLAG_ELIF | FLAG_ELSE | FLAG_FI },
	{ "elif",  RES_ELIF,  FLAG_THEN },
	{ "else",  RES_ELSE,  FLAG_FI   },
	{ "fi",    RES_FI,    FLAG_చివర  },
	{ "for",   RES_FOR,   FLAG_లోపల   | FLAG_ప్రారంభం },
	{ "while", RES_WHILE, FLAG_DO   | FLAG_ప్రారంభం },
	{ "until", RES_UNTIL, FLAG_DO   | FLAG_ప్రారంభం },
	{ "in",    RES_లోపల,    FLAG_DO   },
	{ "do",    RES_DO,    FLAG_పూర్తి },
	{ "done",  RES_పూర్తి,  FLAG_చివర  }
};
#నిర్వచించు NRES (పరిమాణం(reserved_list)/పరిమాణం(నిర్మాణం reserved_combo))

స్థిర పూర్ణాంకం reserved_మాట(o_string *dest, నిర్మాణం p_context *ctx)
{
	నిర్మాణం reserved_combo *r;
	కొరకు (r=reserved_list;
		r<reserved_list+NRES; r++) {
		ఒకవేళ (strcmp(dest->దత్తాంశం, r->literal) == 0) {
			debug_printf("found reserved word %s, code %d\n",r->literal,r->సంకేతం_2);
			ఒకవేళ (r->flag & FLAG_ప్రారంభం) {
				నిర్మాణం p_context *new = xmalloc(పరిమాణం(నిర్మాణం p_context));
				debug_printf("push stack\n");
				ఒకవేళ (ctx->w == RES_లోపల || ctx->w == RES_FOR) {
					syntax();
					విడుదల(new);
					ctx->w = RES_SNTX;
					b_reset(dest);
					తిరిగి 1;
				}
				*new = *ctx;   /* physical copy */
				initialize_context(ctx);
				ctx->దొంతర=new;
			} లేకపోతే ఒకవేళ ( ctx->w == RES_NONE || ! (ctx->old_flag & (1<<r->సంకేతం_2))) {
				syntax();
				ctx->w = RES_SNTX;
				b_reset(dest);
				తిరిగి 1;
			}
			ctx->w=r->సంకేతం_2;
			ctx->old_flag = r->flag;
			ఒకవేళ (ctx->old_flag & FLAG_చివర) {
				నిర్మాణం p_context *old;
				debug_printf("pop stack\n");
				పూర్తి_pipe(ctx,PIPE_SEQ);
				old = ctx->దొంతర;
				old->బిడ్డ->సమూహం = ctx->list_తల;
#నిర్వచితం_కాకపోతే __U_BOOT__
				old->బిడ్డ->subshell = 0;
#అయితే_ముగింపు
				*ctx = *old;   /* physical copy */
				విడుదల(old);
			}
			b_reset (dest);
			తిరిగి 1;
		}
	}
	తిరిగి 0;
}

/* normal return is 0.
 * Syntax or xglob errors return 1. */
స్థిర పూర్ణాంకం పూర్తి_మాట(o_string *dest, నిర్మాణం p_context *ctx)
{
	నిర్మాణం బిడ్డ_prog *బిడ్డ=ctx->బిడ్డ;
#నిర్వచితం_కాకపోతే __U_BOOT__
	glob_t *glob_లక్ష్యం;
	పూర్ణాంకం gr, flags = 0;
#లేకపోతే
	అక్షరం *str, *s;
	పూర్ణాంకం argc, cnt;
#అయితే_ముగింపు

	debug_printf("done_word: %s %p\n", dest->దత్తాంశం, బిడ్డ);
	ఒకవేళ (dest->నిడివి == 0 && !dest->nonnull) {
		debug_printf("  true null, ignored\n");
		తిరిగి 0;
	}
#నిర్వచితం_కాకపోతే __U_BOOT__
	ఒకవేళ (ctx->pending_redirect) {
		glob_లక్ష్యం = &ctx->pending_redirect->మాట;
	} లేకపోతే {
#అయితే_ముగింపు
		ఒకవేళ (బిడ్డ->సమూహం) {
			syntax();
			తిరిగి 1;  /* syntax error, groups and arglists don't mix */
		}
		ఒకవేళ (!బిడ్డ->argv && (ctx->type & FLAG_PARSE_SEMICOLON)) {
			debug_printf("checking %s for reserved-ness\n",dest->దత్తాంశం);
			ఒకవేళ (reserved_మాట(dest,ctx)) తిరిగి ctx->w==RES_SNTX;
		}
#నిర్వచితం_కాకపోతే __U_BOOT__
		glob_లక్ష్యం = &బిడ్డ->glob_ఫలితం;
		ఒకవేళ (బిడ్డ->argv) flags |= GLOB_APPEND;
#లేకపోతే
		కొరకు (cnt = 1, s = dest->దత్తాంశం; s && *s; s++) {
			ఒకవేళ (*s == '\\') s++;
			cnt++;
		}
		str = జ్ఞాపకం_ఇవ్వు(cnt);
		ఒకవేళ (!str) తిరిగి 1;
		ఒకవేళ ( బిడ్డ->argv == ఖాళీ) {
			బిడ్డ->argc=0;
		}
		argc = ++బిడ్డ->argc;
		బిడ్డ->argv = realloc(బిడ్డ->argv, (argc+1)*పరిమాణం(*బిడ్డ->argv));
		ఒకవేళ (బిడ్డ->argv == ఖాళీ) {
			విడుదల(str);
			తిరిగి 1;
		}
		బిడ్డ->argv_nonnull = realloc(బిడ్డ->argv_nonnull,
					(argc+1)*పరిమాణం(*బిడ్డ->argv_nonnull));
		ఒకవేళ (బిడ్డ->argv_nonnull == ఖాళీ) {
			విడుదల(str);
			తిరిగి 1;
		}
		బిడ్డ->argv[argc-1]=str;
		బిడ్డ->argv_nonnull[argc-1] = dest->nonnull;
		బిడ్డ->argv[argc]=ఖాళీ;
		బిడ్డ->argv_nonnull[argc] = 0;
		కొరకు (s = dest->దత్తాంశం; s && *s; s++,str++) {
			ఒకవేళ (*s == '\\') s++;
			*str = *s;
		}
		*str = '\0';
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే __U_BOOT__
	}
	gr = xglob(dest, flags, glob_లక్ష్యం);
	ఒకవేళ (gr != 0) తిరిగి 1;
#అయితే_ముగింపు

	b_reset(dest);
#నిర్వచితం_కాకపోతే __U_BOOT__
	ఒకవేళ (ctx->pending_redirect) {
		ctx->pending_redirect=ఖాళీ;
		ఒకవేళ (glob_లక్ష్యం->gl_pathc != 1) {
			error_సందేశం("ambiguous redirect");
			తిరిగి 1;
		}
	} లేకపోతే {
		బిడ్డ->argv = glob_లక్ష్యం->gl_pathv;
	}
#అయితే_ముగింపు
	ఒకవేళ (ctx->w == RES_FOR) {
		పూర్తి_మాట(dest,ctx);
		పూర్తి_pipe(ctx,PIPE_SEQ);
	}
	తిరిగి 0;
}

/* The only possible error here is out of memory, in which case
 * xmalloc exits. */
స్థిర పూర్ణాంకం పూర్తి_command(నిర్మాణం p_context *ctx)
{
	/* The child is really already in the pipe structure, so
	 * advance the pipe counter and make a new, null child.
	 * Only real trickiness here is that the uncommitted
	 * child structure, to which ctx->child points, is not
	 * counted in pi->num_progs. */
	నిర్మాణం pipe *pi=ctx->pipe;
	నిర్మాణం బిడ్డ_prog *prog=ctx->బిడ్డ;

	ఒకవేళ (prog && prog->సమూహం == ఖాళీ
		 && prog->argv == ఖాళీ
#నిర్వచితం_కాకపోతే __U_BOOT__
		 && prog->redirects == ఖాళీ) {
#లేకపోతే
										) {
#అయితే_ముగింపు
		debug_printf("done_command: skipping null command\n");
		తిరిగి 0;
	} లేకపోతే ఒకవేళ (prog) {
		pi->అంకె_progs++;
		debug_printf("done_command: num_progs incremented to %d\n",pi->అంకె_progs);
	} లేకపోతే {
		debug_printf("done_command: initializing\n");
	}
	pi->progs = xrealloc(pi->progs, పరిమాణం(*pi->progs) * (pi->అంకె_progs+1));

	prog = pi->progs + pi->అంకె_progs;
#నిర్వచితం_కాకపోతే __U_BOOT__
	prog->redirects = ఖాళీ;
#అయితే_ముగింపు
	prog->argv = ఖాళీ;
	prog->argv_nonnull = ఖాళీ;
#నిర్వచితం_కాకపోతే __U_BOOT__
	prog->is_stopped = 0;
#అయితే_ముగింపు
	prog->సమూహం = ఖాళీ;
#నిర్వచితం_కాకపోతే __U_BOOT__
	prog->glob_ఫలితం.gl_pathv = ఖాళీ;
	prog->family = pi;
#అయితే_ముగింపు
	prog->sp = 0;
	ctx->బిడ్డ = prog;
	prog->type = ctx->type;

	/* but ctx->pipe and ctx->list_head remain unchanged */
	తిరిగి 0;
}

స్థిర పూర్ణాంకం పూర్తి_pipe(నిర్మాణం p_context *ctx, pipe_style type)
{
	నిర్మాణం pipe *new_p;
	పూర్తి_command(ctx);  /* implicit closure of previous command */
	debug_printf("done_pipe, type %d\n", type);
	ctx->pipe->followup = type;
	ctx->pipe->r_బహుళకం = ctx->w;
	new_p=new_pipe();
	ctx->pipe->తదుపరి = new_p;
	ctx->pipe = new_p;
	ctx->బిడ్డ = ఖాళీ;
	పూర్తి_command(ctx);  /* set up new pipe to accept commands */
	తిరిగి 0;
}

#నిర్వచితం_కాకపోతే __U_BOOT__
/* peek ahead in the in_str to find out if we have a "&n" construct,
 * as in "2>&1", that represents duplicating a file descriptor.
 * returns either -2 (syntax error), -1 (no &), or the number found.
 */
స్థిర పూర్ణాంకం redirect_dup_అంకె(నిర్మాణం లోపల_str *ఆదానం)
{
	పూర్ణాంకం ch, d=0, ok=0;
	ch = b_peek(ఆదానం);
	ఒకవేళ (ch != '&') తిరిగి -1;

	b_getch(ఆదానం);  /* get the & */
	ch=b_peek(ఆదానం);
	ఒకవేళ (ch == '-') {
		b_getch(ఆదానం);
		తిరిగి -3;  /* "-" represents "close me" */
	}
	అయితే (అంకెనా(ch)) {
		d = d*10+(ch-'0');
		ok=1;
		b_getch(ఆదానం);
		ch = b_peek(ఆదానం);
	}
	ఒకవేళ (ok) తిరిగి d;

	error_సందేశం("ambiguous redirect");
	తిరిగి -2;
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
స్థిర పూర్ణాంకం redirect_opt_అంకె(o_string *o)
{
	పూర్ణాంకం అంకె;

	ఒకవేళ (o->నిడివి==0) తిరిగి -1;
	కొరకు(అంకె=0; అంకె<o->నిడివి; అంకె++) {
		ఒకవేళ (!అంకెనా(*(o->దత్తాంశం+అంకె))) {
			తిరిగి -1;
		}
	}
	/* reuse num (and save an int) */
	అంకె=atoi(o->దత్తాంశం);
	b_reset(o);
	తిరిగి అంకె;
}

FILE *generate_ప్రవాహం_from_list(నిర్మాణం pipe *తల)
{
	FILE *pf;
#ఒకవేళ 1
	పూర్ణాంకం pid, channel[2];
	ఒకవేళ (pipe(channel)<0) perror_సందేశం_and_die("pipe");
	pid=fork();
	ఒకవేళ (pid<0) {
		perror_సందేశం_and_die("fork");
	} లేకపోతే ఒకవేళ (pid==0) {
		close(channel[0]);
		ఒకవేళ (channel[1] != 1) {
			dup2(channel[1],1);
			close(channel[1]);
		}
#ఒకవేళ 0
#నిర్వచించు SURROGATE "surrogate response"
		లిఖ(1,SURROGATE,పరిమాణం(SURROGATE));
		_exit(నిరంతరం_list(తల));
#లేకపోతే
		_exit(నిరంతరం_list_real(తల));   /* leaks memory */
#అయితే_ముగింపు
	}
	debug_printf("forked child %d\n",pid);
	close(channel[1]);
	pf = fdopen(channel[0],"r");
	debug_printf("pipe on FILE *%p\n",pf);
#లేకపోతే
	free_pipe_list(తల,0);
	pf=popen("echo surrogate response","r");
	debug_printf("started fake pipe on FILE *%p\n",pf);
#అయితే_ముగింపు
	తిరిగి pf;
}

/* this version hacked for testing purposes */
/* return code is exit status of the process that is run. */
స్థిర పూర్ణాంకం process_command_subs(o_string *dest, నిర్మాణం p_context *ctx, నిర్మాణం లోపల_str *ఆదానం, పూర్ణాంకం subst_చివర)
{
	పూర్ణాంకం retcode;
	o_string ఫలితం=NULL_O_STRING;
	నిర్మాణం p_context inner;
	FILE *p;
	నిర్మాణం లోపల_str pipe_str;
	initialize_context(&inner);

	/* recursion to generate command */
	retcode = parse_ప్రవాహం(&ఫలితం, &inner, ఆదానం, subst_చివర);
	ఒకవేళ (retcode != 0) తిరిగి retcode;  /* syntax error or EOF */
	పూర్తి_మాట(&ఫలితం, &inner);
	పూర్తి_pipe(&inner, PIPE_SEQ);
	b_free(&ఫలితం);

	p=generate_ప్రవాహం_from_list(inner.list_తల);
	ఒకవేళ (p==ఖాళీ) తిరిగి 1;
	mark_open(fileno(p));
	అమరిక_file_లోపల_str(&pipe_str, p);

	/* now send results of command back into original context */
	retcode = parse_ప్రవాహం(dest, ctx, &pipe_str, '\0');
	/* XXX In case of a syntax error, should we try to kill the child?
	 * That would be tough to do right, so just read until EOF. */
	ఒకవేళ (retcode == 1) {
		అయితే (b_getch(&pipe_str)!=EOF) { /* discard */ };
	}

	debug_printf("done reading from pipe, pclose()ing\n");
	/* This is the step that wait()s for the child.  Should be pretty
	 * safe, since we just read an EOF from its stdout.  We could try
	 * to better, by using wait(), and keeping track of background jobs
	 * at the same time.  That would be a lot of work, and contrary
	 * to the KISS philosophy of this program. */
	mark_closed(fileno(p));
	retcode=pclose(p);
	free_pipe_list(inner.list_తల,0);
	debug_printf("pclosed, retcode=%d\n",retcode);
	/* XXX this process fails to trim a single trailing newline */
	తిరిగి retcode;
}

స్థిర పూర్ణాంకం parse_సమూహం(o_string *dest, నిర్మాణం p_context *ctx,
	నిర్మాణం లోపల_str *ఆదానం, పూర్ణాంకం ch)
{
	పూర్ణాంకం rcode, endch=0;
	నిర్మాణం p_context sub;
	నిర్మాణం బిడ్డ_prog *బిడ్డ = ctx->బిడ్డ;
	ఒకవేళ (బిడ్డ->argv) {
		syntax();
		తిరిగి 1;  /* syntax error, groups and arglists don't mix */
	}
	initialize_context(&sub);
	ఎంపిక(ch) {
		స్థితి '(': endch=')'; బిడ్డ->subshell=1; ఆపు;
		స్థితి '{': endch='}'; ఆపు;
		అప్రమేయ: syntax();   /* really logic error */
	}
	rcode=parse_ప్రవాహం(dest,&sub,ఆదానం,endch);
	పూర్తి_మాట(dest,&sub); /* finish off the final word in the subcontext */
	పూర్తి_pipe(&sub, PIPE_SEQ);  /* and the final command there, too */
	బిడ్డ->సమూహం = sub.list_తల;
	తిరిగి rcode;
	/* child remains "open", available for possible redirects */
}
#అయితే_ముగింపు

/* basically useful version until someone wants to get fancier,
 * see the bash man page under "Parameter Expansion" */
స్థిర అక్షరం *lookup_param(అక్షరం *src)
{
	అక్షరం *p;
	అక్షరం *sep;
	అక్షరం *default_val = ఖాళీ;
	పూర్ణాంకం కేటాయించు = 0;
	పూర్ణాంకం expand_ఖాళీలు = 0;

	ఒకవేళ (!src)
		తిరిగి ఖాళీ;

	sep = strchr(src, ':');

	ఒకవేళ (sep) {
		*sep = '\0';
		ఒకవేళ (*(sep + 1) == '-')
			default_val = sep+2;
		ఒకవేళ (*(sep + 1) == '=') {
			default_val = sep+2;
			కేటాయించు = 1;
		}
		ఒకవేళ (*(sep + 1) == '+') {
			default_val = sep+2;
			expand_ఖాళీలు = 1;
		}
	}

	p = env_get(src);
	ఒకవేళ (!p)
		p = get_local_var(src);

	ఒకవేళ (!p || అక్షరశ్రేణి_పొడవు(p) == 0) {
		p = default_val;
		ఒకవేళ (కేటాయించు) {
			అక్షరం *var = జ్ఞాపకం_ఇవ్వు(అక్షరశ్రేణి_పొడవు(src)+అక్షరశ్రేణి_పొడవు(default_val)+2);
			ఒకవేళ (var) {
				sprintf(var, "%s=%s", src, default_val);
				సమితి_local_var(var, 0);
			}
			విడుదల(var);
		}
	} లేకపోతే ఒకవేళ (expand_ఖాళీలు) {
		p += అక్షరశ్రేణి_పొడవు(p);
	}

	ఒకవేళ (sep)
		*sep = ':';

	తిరిగి p;
}

#నిర్వచితమైతే __U_BOOT__
స్థిర అక్షరం *get_dollar_var(అక్షరం ch)
{
	స్థిర అక్షరం buf[40];

	buf[0] = '\0';
	ఎంపిక (ch) {
		స్థితి '?':
			sprintf(buf, "%u", (గుర్తులేని పూర్ణాంకం)చివరి_return_సంకేతం);
			ఆపు;
		అప్రమేయ:
			తిరిగి ఖాళీ;
	}
	తిరిగి buf;
}
#అయితే_ముగింపు

/* return code: 0 for OK, 1 for syntax error */
స్థిర పూర్ణాంకం handle_dollar(o_string *dest, నిర్మాణం p_context *ctx, నిర్మాణం లోపల_str *ఆదానం)
{
#నిర్వచితం_కాకపోతే __U_BOOT__
	పూర్ణాంకం i, advance=0;
#లేకపోతే
	పూర్ణాంకం advance=0;
#అయితే_ముగింపు
#నిర్వచితం_కాకపోతే __U_BOOT__
	అక్షరం sep[]=" ";
#అయితే_ముగింపు
	పూర్ణాంకం ch = ఆదానం->peek(ఆదానం);  /* first character after the $ */
	debug_printf("handle_dollar: ch=%c\n",ch);
	ఒకవేళ (అక్షరమా(ch)) {
		b_addchr(dest, SPECIAL_VAR_చిహ్నం);
		ctx->బిడ్డ->sp++;
		అయితే(ch=b_peek(ఆదానం),అక్షరమా_అంకెనా(ch) || ch=='_') {
			b_getch(ఆదానం);
			b_addchr(dest,ch);
		}
		b_addchr(dest, SPECIAL_VAR_చిహ్నం);
#నిర్వచితం_కాకపోతే __U_BOOT__
	} లేకపోతే ఒకవేళ (అంకెనా(ch)) {
		i = ch-'0';  /* XXX is $0 special? */
		ఒకవేళ (i<global_argc) {
			parse_string(dest, ctx, global_argv[i]); /* recursion */
		}
		advance = 1;
#అయితే_ముగింపు
	} లేకపోతే ఎంపిక (ch) {
#నిర్వచితం_కాకపోతే __U_BOOT__
		స్థితి '$':
			b_adduint(dest,getpid());
			advance = 1;
			ఆపు;
		స్థితి '!':
			ఒకవేళ (చివరి_bg_pid > 0) b_adduint(dest, చివరి_bg_pid);
			advance = 1;
			ఆపు;
#అయితే_ముగింపు
		స్థితి '?':
#నిర్వచితం_కాకపోతే __U_BOOT__
			b_adduint(dest,చివరి_return_సంకేతం);
#లేకపోతే
			ctx->బిడ్డ->sp++;
			b_addchr(dest, SPECIAL_VAR_చిహ్నం);
			b_addchr(dest, '$');
			b_addchr(dest, '?');
			b_addchr(dest, SPECIAL_VAR_చిహ్నం);
#అయితే_ముగింపు
			advance = 1;
			ఆపు;
#నిర్వచితం_కాకపోతే __U_BOOT__
		స్థితి '#':
			b_adduint(dest,global_argc ? global_argc-1 : 0);
			advance = 1;
			ఆపు;
#అయితే_ముగింపు
		స్థితి '{':
			b_addchr(dest, SPECIAL_VAR_చిహ్నం);
			ctx->బిడ్డ->sp++;
			b_getch(ఆదానం);
			/* XXX maybe someone will try to escape the '}' */
			అయితే(ch=b_getch(ఆదానం),ch!=EOF && ch!='}') {
				b_addchr(dest,ch);
			}
			ఒకవేళ (ch != '}') {
				syntax();
				తిరిగి 1;
			}
			b_addchr(dest, SPECIAL_VAR_చిహ్నం);
			ఆపు;
#నిర్వచితం_కాకపోతే __U_BOOT__
		స్థితి '(':
			b_getch(ఆదానం);
			process_command_subs(dest, ctx, ఆదానం, ')');
			ఆపు;
		స్థితి '*':
			sep[0]=ifs[0];
			కొరకు (i=1; i<global_argc; i++) {
				parse_string(dest, ctx, global_argv[i]);
				ఒకవేళ (i+1 < global_argc) parse_string(dest, ctx, sep);
			}
			ఆపు;
		స్థితి '@':
		స్థితి '-':
		స్థితి '_':
			/* still unhandled, but should be eventually */
			error_సందేశం("unhandled syntax: $%c",ch);
			తిరిగి 1;
			ఆపు;
#అయితే_ముగింపు
		అప్రమేయ:
			b_addqchr(dest,'$',dest->quote);
	}
	/* Eat the character if the flag was set.  If the compiler
	 * is smart enough, we could substitute "b_getch(input);"
	 * for all the "advance = 1;" above, and also end up with
	 * a nice size-optimized program.  Hah!  That'll be the day.
	 */
	ఒకవేళ (advance) b_getch(ఆదానం);
	తిరిగి 0;
}

#నిర్వచితం_కాకపోతే __U_BOOT__
పూర్ణాంకం parse_string(o_string *dest, నిర్మాణం p_context *ctx, స్థిరరాశి అక్షరం *src)
{
	నిర్మాణం లోపల_str foo;
	అమరిక_string_లోపల_str(&foo, src);
	తిరిగి parse_ప్రవాహం(dest, ctx, &foo, '\0');
}
#అయితే_ముగింపు

/* return code is 0 for normal exit, 1 for syntax error */
స్థిర పూర్ణాంకం parse_ప్రవాహం(o_string *dest, నిర్మాణం p_context *ctx,
			నిర్మాణం లోపల_str *ఆదానం, పూర్ణాంకం చివర_trigger)
{
	గుర్తులేని పూర్ణాంకం ch, m;
#నిర్వచితం_కాకపోతే __U_BOOT__
	పూర్ణాంకం redir_fd;
	redir_type redir_style;
#అయితే_ముగింపు
	పూర్ణాంకం తదుపరి;

	/* Only double-quote state is handled in the state variable dest->quote.
	 * A single-quote triggers a bypass of the main loop until its mate is
	 * found.  When recursing, quote state is passed in via dest->quote. */

	debug_printf("parse_stream, end_trigger=%d\n",చివర_trigger);
	అయితే ((ch=b_getch(ఆదానం))!=EOF) {
		m = map[ch];
#నిర్వచితమైతే __U_BOOT__
		ఒకవేళ (ఆదానం->__promptme == 0) తిరిగి 1;
#అయితే_ముగింపు
		తదుపరి = (ch == '\n') ? 0 : b_peek(ఆదానం);

		debug_printf("parse_stream: ch=%c (%d) m=%d quote=%d - %c\n",
			ch >= ' ' ? ch : '.', ch, m,
			dest->quote, ctx->దొంతర == ఖాళీ ? '*' : '.');

		ఒకవేళ (m==0 || ((m==1 || m==2) && dest->quote)) {
			b_addqchr(dest, ch, dest->quote);
		} లేకపోతే {
			ఒకవేళ (m==2) {  /* unquoted IFS */
				ఒకవేళ (పూర్తి_మాట(dest, ctx)) {
					తిరిగి 1;
				}
				/* If we aren't performing a substitution, treat a newline as a
				 * command separator.  */
				ఒకవేళ (చివర_trigger != '\0' && ch=='\n')
					పూర్తి_pipe(ctx,PIPE_SEQ);
			}
			ఒకవేళ (ch == చివర_trigger && !dest->quote && ctx->w==RES_NONE) {
				debug_printf("leaving parse_stream (triggered)\n");
				తిరిగి 0;
			}
#ఒకవేళ 0
			ఒకవేళ (ch=='\n') {
				/* Yahoo!  Time to run with it! */
				పూర్తి_pipe(ctx,PIPE_SEQ);
				నిరంతరం_list(ctx->list_తల);
				initialize_context(ctx);
			}
#అయితే_ముగింపు
			ఒకవేళ (m!=2) ఎంపిక (ch) {
		స్థితి '#':
			ఒకవేళ (dest->నిడివి == 0 && !dest->quote) {
				అయితే(ch=b_peek(ఆదానం),ch!=EOF && ch!='\n') { b_getch(ఆదానం); }
			} లేకపోతే {
				b_addqchr(dest, ch, dest->quote);
			}
			ఆపు;
		స్థితి '\\':
			ఒకవేళ (తదుపరి == EOF) {
				syntax();
				తిరిగి 1;
			}
			b_addqchr(dest, '\\', dest->quote);
			b_addqchr(dest, b_getch(ఆదానం), dest->quote);
			ఆపు;
		స్థితి '$':
			ఒకవేళ (handle_dollar(dest, ctx, ఆదానం)!=0) తిరిగి 1;
			ఆపు;
		స్థితి '\'':
			dest->nonnull = 1;
			అయితే(ch=b_getch(ఆదానం),ch!=EOF && ch!='\'') {
#నిర్వచితమైతే __U_BOOT__
				ఒకవేళ(ఆదానం->__promptme == 0) తిరిగి 1;
#అయితే_ముగింపు
				b_addchr(dest,ch);
			}
			ఒకవేళ (ch==EOF) {
				syntax();
				తిరిగి 1;
			}
			ఆపు;
		స్థితి '"':
			dest->nonnull = 1;
			dest->quote = !dest->quote;
			ఆపు;
#నిర్వచితం_కాకపోతే __U_BOOT__
		స్థితి '`':
			process_command_subs(dest, ctx, ఆదానం, '`');
			ఆపు;
		స్థితి '>':
			redir_fd = redirect_opt_అంకె(dest);
			పూర్తి_మాట(dest, ctx);
			redir_style=REDIRECT_OVERWRITE;
			ఒకవేళ (తదుపరి == '>') {
				redir_style=REDIRECT_APPEND;
				b_getch(ఆదానం);
			} లేకపోతే ఒకవేళ (తదుపరి == '(') {
				syntax();   /* until we support >(list) Process Substitution */
				తిరిగి 1;
			}
			అమరిక_redirect(ctx, redir_fd, redir_style, ఆదానం);
			ఆపు;
		స్థితి '<':
			redir_fd = redirect_opt_అంకె(dest);
			పూర్తి_మాట(dest, ctx);
			redir_style=REDIRECT_ఆదానం;
			ఒకవేళ (తదుపరి == '<') {
				redir_style=REDIRECT_HEREIS;
				b_getch(ఆదానం);
			} లేకపోతే ఒకవేళ (తదుపరి == '>') {
				redir_style=REDIRECT_IO;
				b_getch(ఆదానం);
			} లేకపోతే ఒకవేళ (తదుపరి == '(') {
				syntax();   /* until we support <(list) Process Substitution */
				తిరిగి 1;
			}
			అమరిక_redirect(ctx, redir_fd, redir_style, ఆదానం);
			ఆపు;
#అయితే_ముగింపు
		స్థితి ';':
			పూర్తి_మాట(dest, ctx);
			పూర్తి_pipe(ctx,PIPE_SEQ);
			ఆపు;
		స్థితి '&':
			పూర్తి_మాట(dest, ctx);
			ఒకవేళ (తదుపరి=='&') {
				b_getch(ఆదానం);
				పూర్తి_pipe(ctx,PIPE_AND);
			} లేకపోతే {
#నిర్వచితం_కాకపోతే __U_BOOT__
				పూర్తి_pipe(ctx,PIPE_BG);
#లేకపోతే
				syntax_err();
				తిరిగి 1;
#అయితే_ముగింపు
			}
			ఆపు;
		స్థితి '|':
			పూర్తి_మాట(dest, ctx);
			ఒకవేళ (తదుపరి=='|') {
				b_getch(ఆదానం);
				పూర్తి_pipe(ctx,PIPE_OR);
			} లేకపోతే {
				/* we could pick up a file descriptor choice here
				 * with redirect_opt_num(), but bash doesn't do it.
				 * "echo foo 2| cat" yields "foo 2". */
#నిర్వచితం_కాకపోతే __U_BOOT__
				పూర్తి_command(ctx);
#లేకపోతే
				syntax_err();
				తిరిగి 1;
#అయితే_ముగింపు
			}
			ఆపు;
#నిర్వచితం_కాకపోతే __U_BOOT__
		స్థితి '(':
		స్థితి '{':
			ఒకవేళ (parse_సమూహం(dest, ctx, ఆదానం, ch)!=0) తిరిగి 1;
			ఆపు;
		స్థితి ')':
		స్థితి '}':
			syntax();   /* Proper use of this character caught by end_trigger */
			తిరిగి 1;
			ఆపు;
#అయితే_ముగింపు
		స్థితి SUBSTED_VAR_చిహ్నం:
			dest->nonnull = 1;
			అయితే (ch = b_getch(ఆదానం), ch != EOF &&
			    ch != SUBSTED_VAR_చిహ్నం) {
				debug_printf("subst, pass=%d\n", ch);
				ఒకవేళ (ఆదానం->__promptme == 0)
					తిరిగి 1;
				b_addchr(dest, ch);
			}
			debug_printf("subst, term=%d\n", ch);
			ఒకవేళ (ch == EOF) {
				syntax();
				తిరిగి 1;
			}
			ఆపు;
		అప్రమేయ:
			syntax();   /* this is really an internal logic error */
			తిరిగి 1;
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
	ఒకవేళ (చివర_trigger != '\0') తిరిగి -1;
	తిరిగి 0;
}

స్థిర శూన్య mapset(స్థిరరాశి గుర్తులేని అక్షరం *సమితి, పూర్ణాంకం సంకేతం_2)
{
	స్థిరరాశి గుర్తులేని అక్షరం *s;
	కొరకు (s=సమితి; *s; s++) map[*s] = సంకేతం_2;
}

స్థిర శూన్య update_ifs_map(శూన్య)
{
	/* char *ifs and char map[256] are both globals. */
	ifs = (uchar *)env_get("IFS");
	ఒకవేళ (ifs == ఖాళీ) ifs=(uchar *)" \t\n";
	/* Precompute a list of 'flow through' behavior so it can be treated
	 * quickly up front.  Computation is necessary because of IFS.
	 * Special case handling of IFS == " \t\n" is not implemented.
	 * The map[] array only really needs two bits each, and on most machines
	 * that would be faster because of the reduced L1 cache footprint.
	 */
	memset(map,0,పరిమాణం(map)); /* most characters flow through always */
#నిర్వచితం_కాకపోతే __U_BOOT__
	mapset((uchar *)"\\$'\"`", 3);      /* never flow through */
	mapset((uchar *)"<>;&|(){}#", 1);   /* flow through if quoted */
#లేకపోతే
	{
		uchar subst[2] = {SUBSTED_VAR_చిహ్నం, 0};
		mapset(subst, 3);       /* never flow through */
	}
	mapset((uchar *)"\\$'\"", 3);       /* never flow through */
	mapset((uchar *)";&|#", 1);         /* flow through if quoted */
#అయితే_ముగింపు
	mapset(ifs, 2);            /* also flow through if quoted */
}

/* most recursion does not come through here, the exeception is
 * from builtin_source() */
స్థిర పూర్ణాంకం parse_ప్రవాహం_outer(నిర్మాణం లోపల_str *inp, పూర్ణాంకం flag)
{

	నిర్మాణం p_context ctx;
	o_string తాత్కాలిక=NULL_O_STRING;
	పూర్ణాంకం rcode;
#నిర్వచితమైతే __U_BOOT__
	పూర్ణాంకం సంకేతం_2 = 1;
#అయితే_ముగింపు
	చేయి {
		ctx.type = flag;
		initialize_context(&ctx);
		update_ifs_map();
		ఒకవేళ (!(flag & FLAG_PARSE_SEMICOLON) || (flag & FLAG_REPARSING)) mapset((uchar *)";$&|", 0);
		inp->promptmode=1;
		rcode = parse_ప్రవాహం(&తాత్కాలిక, &ctx, inp,
				     flag & FLAG_CONT_ON_NEWLINE ? -1 : '\n');
#నిర్వచితమైతే __U_BOOT__
		ఒకవేళ (rcode == 1) flag_repeat = 0;
#అయితే_ముగింపు
		ఒకవేళ (rcode != 1 && ctx.old_flag != 0) {
			syntax();
#నిర్వచితమైతే __U_BOOT__
			flag_repeat = 0;
#అయితే_ముగింపు
		}
		ఒకవేళ (rcode != 1 && ctx.old_flag == 0) {
			పూర్తి_మాట(&తాత్కాలిక, &ctx);
			పూర్తి_pipe(&ctx,PIPE_SEQ);
#నిర్వచితం_కాకపోతే __U_BOOT__
			నిరంతరం_list(ctx.list_తల);
#లేకపోతే
			సంకేతం_2 = నిరంతరం_list(ctx.list_తల);
			ఒకవేళ (సంకేతం_2 == -2) {	/* exit */
				b_free(&తాత్కాలిక);
				సంకేతం_2 = 0;
				/* XXX hackish way to not allow exit from main loop */
				ఒకవేళ (inp->peek == file_peek) {
					రాయి("exit not allowed from main input shell.\n");
					కొనసాగు;
				}
				/*
				 * DANGER
				 * Return code -2 is special in this context,
				 * it indicates exit from inner pipe instead
				 * of return code itself, the return code is
				 * stored in 'last_return_code' variable!
				 * DANGER
				 */
				తిరిగి -2;
			}
			ఒకవేళ (సంకేతం_2 == -1)
			    flag_repeat = 0;
#అయితే_ముగింపు
		} లేకపోతే {
			ఒకవేళ (ctx.old_flag != 0) {
				విడుదల(ctx.దొంతర);
				b_reset(&తాత్కాలిక);
			}
#నిర్వచితమైతే __U_BOOT__
			ఒకవేళ (inp->__promptme == 0) రాయి("<INTERRUPT>\n");
			inp->__promptme = 1;
#అయితే_ముగింపు
			తాత్కాలిక.nonnull = 0;
			తాత్కాలిక.quote = 0;
			inp->p = ఖాళీ;
			free_pipe_list(ctx.list_తల,0);
		}
		b_free(&తాత్కాలిక);
	/* loop on syntax errors, return on EOF */
	} అయితే (rcode != -1 && !(flag & FLAG_EXIT_FROM_LOOP) &&
		(inp->peek != static_peek || b_peek(inp)));
#నిర్వచితం_కాకపోతే __U_BOOT__
	తిరిగి 0;
#లేకపోతే
	తిరిగి (సంకేతం_2 != 0) ? 1 : 0;
#అయితే_ముగింపు /* __U_BOOT__ */
}

#నిర్వచితం_కాకపోతే __U_BOOT__
స్థిర పూర్ణాంకం parse_string_outer(స్థిరరాశి అక్షరం *s, పూర్ణాంకం flag)
#లేకపోతే
పూర్ణాంకం parse_string_outer(స్థిరరాశి అక్షరం *s, పూర్ణాంకం flag)
#అయితే_ముగింపు	/* __U_BOOT__ */
{
	నిర్మాణం లోపల_str ఆదానం;
	పూర్ణాంకం rcode;
#నిర్వచితమైతే __U_BOOT__
	అక్షరం *p = ఖాళీ;
	ఒకవేళ (!s)
		తిరిగి 1;
	ఒకవేళ (!*s)
		తిరిగి 0;
	ఒకవేళ (!(p = strchr(s, '\n')) || *++p) {
		p = xmalloc(అక్షరశ్రేణి_పొడవు(s) + 2);
		strcpy(p, s);
		strcat(p, "\n");
		అమరిక_string_లోపల_str(&ఆదానం, p);
		rcode = parse_ప్రవాహం_outer(&ఆదానం, flag);
		విడుదల(p);
		తిరిగి rcode == -2 ? చివరి_return_సంకేతం : rcode;
	} లేకపోతే {
#అయితే_ముగింపు
	అమరిక_string_లోపల_str(&ఆదానం, s);
	rcode = parse_ప్రవాహం_outer(&ఆదానం, flag);
	తిరిగి rcode == -2 ? చివరి_return_సంకేతం : rcode;
#నిర్వచితమైతే __U_BOOT__
	}
#అయితే_ముగింపు
}

#నిర్వచితం_కాకపోతే __U_BOOT__
స్థిర పూర్ణాంకం parse_file_outer(FILE *f)
#లేకపోతే
పూర్ణాంకం parse_file_outer(శూన్య)
#అయితే_ముగింపు
{
	పూర్ణాంకం rcode;
	నిర్మాణం లోపల_str ఆదానం;
#నిర్వచితం_కాకపోతే __U_BOOT__
	అమరిక_file_లోపల_str(&ఆదానం, f);
#లేకపోతే
	అమరిక_file_లోపల_str(&ఆదానం);
#అయితే_ముగింపు
	rcode = parse_ప్రవాహం_outer(&ఆదానం, FLAG_PARSE_SEMICOLON);
	తిరిగి rcode == -2 ? చివరి_return_సంకేతం : rcode;
}

#నిర్వచితమైతే __U_BOOT__
పూర్ణాంకం u_boot_hush_ప్రారంభం(శూన్య)
{
	ఒకవేళ (శిఖరం_vars == ఖాళీ) {
		శిఖరం_vars = జ్ఞాపకం_ఇవ్వు(పరిమాణం(నిర్మాణం variables));
		శిఖరం_vars->name = "HUSH_VERSION";
		శిఖరం_vars->విలువ = "0.01";
		శిఖరం_vars->తదుపరి = ఖాళీ;
		శిఖరం_vars->flg_export = 0;
		శిఖరం_vars->flg_read_only = 1;
	}
	తిరిగి 0;
}

స్థిర శూన్య *xmalloc(size_t కొలత)
{
	శూన్య *p = ఖాళీ;

	ఒకవేళ (!(p = జ్ఞాపకం_ఇవ్వు(కొలత))) {
	    రాయి("ERROR : xmalloc failed\n");
	    కొరకు(;;);
	}
	తిరిగి p;
}

స్థిర శూన్య *xrealloc(శూన్య *ptr, size_t కొలత)
{
	శూన్య *p = ఖాళీ;

	ఒకవేళ (!(p = realloc(ptr, కొలత))) {
	    రాయి("ERROR : xrealloc failed\n");
	    కొరకు(;;);
	}
	తిరిగి p;
}
#అయితే_ముగింపు /* __U_BOOT__ */

#నిర్వచితం_కాకపోతే __U_BOOT__
/* Make sure we have a controlling tty.  If we get started under a job
 * aware app (like bash for example), make sure we are now in charge so
 * we don't fight over who gets the foreground */
స్థిర శూన్య అమరిక_పని_control(శూన్య)
{
	స్థిర pid_t shell_pgrp;
	/* Loop until we are in the foreground.  */
	అయితే (tcgetpgrp (shell_terminal) != (shell_pgrp = getpgrp ()))
		kill (- shell_pgrp, SIGTTIN);

	/* Ignore interactive and job-control signals.  */
	సంకేతం(SIGINT, SIG_IGN);
	సంకేతం(SIGQUIT, SIG_IGN);
	సంకేతం(SIGTERM, SIG_IGN);
	సంకేతం(SIGTSTP, SIG_IGN);
	సంకేతం(SIGTTIN, SIG_IGN);
	సంకేతం(SIGTTOU, SIG_IGN);
	సంకేతం(SIGCHLD, SIG_IGN);

	/* Put ourselves in our own process group.  */
	setsid();
	shell_pgrp = getpid ();
	setpgid (shell_pgrp, shell_pgrp);

	/* Grab control of the terminal.  */
	tcsetpgrp(shell_terminal, shell_pgrp);
}

పూర్ణాంకం hush_main(పూర్ణాంకం argc, అక్షరం * స్థిరరాశి *argv)
{
	పూర్ణాంకం opt;
	FILE *ఆదానం;
	అక్షరం **e = environ;

	/* XXX what should these be while sourcing /etc/profile? */
	global_argc = argc;
	global_argv = argv;

	/* (re?) initialize globals.  Sometimes hush_main() ends up calling
	 * hush_main(), therefore we cannot rely on the BSS to zero out this
	 * stuff.  Reset these to 0 every time. */
	ifs = ఖాళీ;
	/* map[] is taken care of with call to update_ifs_map() */
	fake_బహుళకం = 0;
	interactive = 0;
	close_me_తల = ఖాళీ;
	చివరి_bg_pid = 0;
	పని_list = ఖాళీ;
	చివరి_jobid = 0;

	/* Initialize some more globals to non-zero values */
	సమితి_cwd();
#నిర్వచితమైతే CONFIG_FEATURE_COMMAND_EDITING
	cmdedit_సమితి_initial_prompt();
#లేకపోతే
	PS1 = ఖాళీ;
#అయితే_ముగింపు
	PS2 = "> ";

	/* initialize our shell local variables with the values
	 * currently living in the environment */
	ఒకవేళ (e) {
		కొరకు (; *e; e++)
			సమితి_local_var(*e, 2);   /* without call putenv() */
	}

	చివరి_return_సంకేతం=EXIT_SUCCESS;

	ఒకవేళ (argv[0] && argv[0][0] == '-') {
		debug_printf("\nsourcing /etc/profile\n");
		ఒకవేళ ((ఆదానం = fopen("/etc/profile", "r")) != ఖాళీ) {
			mark_open(fileno(ఆదానం));
			parse_file_outer(ఆదానం);
			mark_closed(fileno(ఆదానం));
			fclose(ఆదానం);
		}
	}
	ఆదానం=stdin;

	అయితే ((opt = getopt(argc, argv, "c:xif")) > 0) {
		ఎంపిక (opt) {
			స్థితి 'c':
				{
					global_argv = argv+optind;
					global_argc = argc-optind;
					opt = parse_string_outer(optarg, FLAG_PARSE_SEMICOLON);
					వెళ్ళు final_return;
				}
				ఆపు;
			స్థితి 'i':
				interactive++;
				ఆపు;
			స్థితి 'f':
				fake_బహుళకం++;
				ఆపు;
			అప్రమేయ:
#నిర్వచితం_కాకపోతే BB_VER
				fprintf(stderr, "Usage: sh [FILE]...\n"
						"   or: sh -c command [args]...\n\n");
				నిష్క్రమించు(EXIT_FAILURE);
#లేకపోతే
				show_usage();
#అయితే_ముగింపు
		}
	}
	/* A shell is interactive if the `-i' flag was given, or if all of
	 * the following conditions are met:
	 *	  no -c command
	 *    no arguments remaining or the -s flag given
	 *    standard input is a terminal
	 *    standard output is a terminal
	 *    Refer to Posix.2, the description of the `sh' utility. */
	ఒకవేళ (argv[optind]==ఖాళీ && ఆదానం==stdin &&
			isatty(fileno(stdin)) && isatty(fileno(stdout))) {
		interactive++;
	}

	debug_printf("\ninteractive=%d\n", interactive);
	ఒకవేళ (interactive) {
		/* Looks like they want an interactive shell */
#నిర్వచితం_కాకపోతే CONFIG_FEATURE_SH_EXTRA_QUIET
		రాయి( "\n\n" BB_BANNER " hush - the humble shell v0.01 (testing)\n");
		రాయి( "Enter 'help' for a list of built-in commands.\n\n");
#అయితే_ముగింపు
		అమరిక_పని_control();
	}

	ఒకవేళ (argv[optind]==ఖాళీ) {
		opt=parse_file_outer(stdin);
		వెళ్ళు final_return;
	}

	debug_printf("\nrunning script '%s'\n", argv[optind]);
	global_argv = argv+optind;
	global_argc = argc-optind;
	ఆదానం = xfopen(argv[optind], "r");
	opt = parse_file_outer(ఆదానం);

#నిర్వచితమైతే CONFIG_FEATURE_CLEAN_పైకి
	fclose(ఆదానం);
	ఒకవేళ (cwd && cwd != unknown)
		విడుదల((అక్షరం*)cwd);
	{
		నిర్మాణం variables *cur, *tmp;
		కొరకు(cur = శిఖరం_vars; cur; cur = tmp) {
			tmp = cur->తదుపరి;
			ఒకవేళ (!cur->flg_read_only) {
				విడుదల(cur->name);
				విడుదల(cur->విలువ);
				విడుదల(cur);
			}
		}
	}
#అయితే_ముగింపు

final_return:
	తిరిగి(opt?opt:చివరి_return_సంకేతం);
}
#అయితే_ముగింపు

స్థిర అక్షరం *చొప్పించు_var_విలువ(అక్షరం *inp)
{
	తిరిగి చొప్పించు_var_విలువ_sub(inp, 0);
}

స్థిర అక్షరం *చొప్పించు_var_విలువ_sub(అక్షరం *inp, పూర్ణాంకం ట్యాగ్_subst)
{
	పూర్ణాంకం res_str_len = 0;
	పూర్ణాంకం len;
	పూర్ణాంకం పూర్తి = 0;
	అక్షరం *p, *p1, *res_str = ఖాళీ;

	అయితే ((p = strchr(inp, SPECIAL_VAR_చిహ్నం))) {
		/* check the beginning of the string for normal characters */
		ఒకవేళ (p != inp) {
			/* copy any characters to the result string */
			len = p - inp;
			res_str = xrealloc(res_str, (res_str_len + len));
			strncpy((res_str + res_str_len), inp, len);
			res_str_len += len;
		}
		inp = ++p;
		/* find the ending marker */
		p = strchr(inp, SPECIAL_VAR_చిహ్నం);
		*p = '\0';
		/* look up the value to substitute */
		ఒకవేళ ((p1 = lookup_param(inp))) {
			ఒకవేళ (ట్యాగ్_subst)
				len = res_str_len + అక్షరశ్రేణి_పొడవు(p1) + 2;
			లేకపోతే
				len = res_str_len + అక్షరశ్రేణి_పొడవు(p1);
			res_str = xrealloc(res_str, (1 + len));
			ఒకవేళ (ట్యాగ్_subst) {
				/*
				 * copy the variable value to the result
				 * string
				 */
				strcpy((res_str + res_str_len + 1), p1);

				/*
				 * mark the replaced text to be accepted as
				 * is
				 */
				res_str[res_str_len] = SUBSTED_VAR_చిహ్నం;
				res_str[res_str_len + 1 + అక్షరశ్రేణి_పొడవు(p1)] =
					SUBSTED_VAR_చిహ్నం;
			} లేకపోతే
				/*
				 * copy the variable value to the result
				 * string
				 */
				strcpy((res_str + res_str_len), p1);

			res_str_len = len;
		}
		*p = SPECIAL_VAR_చిహ్నం;
		inp = ++p;
		పూర్తి = 1;
	}
	ఒకవేళ (పూర్తి) {
		res_str = xrealloc(res_str, (1 + res_str_len + అక్షరశ్రేణి_పొడవు(inp)));
		strcpy((res_str + res_str_len), inp);
		అయితే ((p = strchr(res_str, '\n'))) {
			*p = ' ';
		}
	}
	తిరిగి (res_str == ఖాళీ) ? inp : res_str;
}

స్థిర అక్షరం **make_list_లోపల(అక్షరం **inp, అక్షరం *name)
{
	పూర్ణాంకం len, i;
	పూర్ణాంకం name_len = అక్షరశ్రేణి_పొడవు(name);
	పూర్ణాంకం n = 0;
	అక్షరం **list;
	అక్షరం *p1, *p2, *p3;

	/* create list of variable values */
	list = xmalloc(పరిమాణం(*list));
	కొరకు (i = 0; inp[i]; i++) {
		p3 = చొప్పించు_var_విలువ(inp[i]);
		p1 = p3;
		అయితే (*p1) {
			ఒకవేళ (*p1 == ' ') {
				p1++;
				కొనసాగు;
			}
			ఒకవేళ ((p2 = strchr(p1, ' '))) {
				len = p2 - p1;
			} లేకపోతే {
				len = అక్షరశ్రేణి_పొడవు(p1);
				p2 = p1 + len;
			}
			/* we use n + 2 in realloc for list,because we add
			 * new element and then we will add NULL element */
			list = xrealloc(list, పరిమాణం(*list) * (n + 2));
			list[n] = xmalloc(2 + name_len + len);
			strcpy(list[n], name);
			strcat(list[n], "=");
			strncat(list[n], p1, len);
			list[n++][name_len + len + 1] = '\0';
			p1 = p2;
		}
		ఒకవేళ (p3 != inp[i]) విడుదల(p3);
	}
	list[n] = ఖాళీ;
	తిరిగి list;
}

/*
 * Make new string for parser
 * inp     - array of argument strings to flatten
 * nonnull - indicates argument was quoted when originally parsed
 */
స్థిర అక్షరం *make_string(అక్షరం **inp, పూర్ణాంకం *nonnull)
{
	అక్షరం *p;
	అక్షరం *str = ఖాళీ;
	పూర్ణాంకం n;
	పూర్ణాంకం len = 2;
	అక్షరం *noeval_str;
	పూర్ణాంకం noeval = 0;

	noeval_str = get_local_var("HUSH_NO_EVAL");
	ఒకవేళ (noeval_str != ఖాళీ && *noeval_str != '0' && *noeval_str != '\0')
		noeval = 1;
	కొరకు (n = 0; inp[n]; n++) {
		p = చొప్పించు_var_విలువ_sub(inp[n], noeval);
		అక్షరం *new_str = xrealloc(str, (len + అక్షరశ్రేణి_పొడవు(p) + (2 * nonnull[n])));
		ఒకవేళ (!new_str) {
			విడుదల(str);
			ఒకవేళ (p != inp[n]) విడుదల(p);
			తిరిగి ఖాళీ;
		}
		str = new_str;
		ఒకవేళ (n) {
			strcat(str, " ");
		} లేకపోతే {
			*str = '\0';
		}
		ఒకవేళ (nonnull[n])
			strcat(str, "'");
		strcat(str, p);
		ఒకవేళ (nonnull[n])
			strcat(str, "'");
		len = అక్షరశ్రేణి_పొడవు(str) + 3;
		ఒకవేళ (p != inp[n]) విడుదల(p);
	}
	len = అక్షరశ్రేణి_పొడవు(str);
	*(str + len) = '\n';
	*(str + len + 1) = '\0';
	తిరిగి str;
}

#నిర్వచితమైతే __U_BOOT__
స్థిర పూర్ణాంకం do_showvar(నిర్మాణం cmd_tbl *cmdtp, పూర్ణాంకం flag, పూర్ణాంకం argc,
		      అక్షరం *స్థిరరాశి argv[])
{
	పూర్ణాంకం i, k;
	పూర్ణాంకం rcode = 0;
	నిర్మాణం variables *cur;

	ఒకవేళ (argc == 1) {		/* Print all env variables	*/
		కొరకు (cur = శిఖరం_vars; cur; cur = cur->తదుపరి) {
			రాయి ("%s=%s\n", cur->name, cur->విలువ);
			ఒకవేళ (ctrlc ()) {
				puts ("\n ** Abort\n");
				తిరిగి 1;
			}
		}
		తిరిగి 0;
	}
	కొరకు (i = 1; i < argc; ++i) {	/* print single env variables	*/
		అక్షరం *name = argv[i];

		k = -1;
		కొరకు (cur = శిఖరం_vars; cur; cur = cur->తదుపరి) {
			ఒకవేళ(strcmp (cur->name, name) == 0) {
				k = 0;
				రాయి ("%s=%s\n", cur->name, cur->విలువ);
			}
			ఒకవేళ (ctrlc ()) {
				puts ("\n ** Abort\n");
				తిరిగి 1;
			}
		}
		ఒకవేళ (k < 0) {
			రాయి ("## Error: \"%s\" not defined\n", name);
			rcode ++;
		}
	}
	తిరిగి rcode;
}

U_BOOT_CMD(
	showvar, CONFIG_SYS_MAXARGS, 1,	do_showvar,
	"print local hushshell variables",
	"\n    - print values of all hushshell variables\n"
	"showvar name ...\n"
	"    - print value of hushshell variable 'name'"
);

#అయితే_ముగింపు
/****************************************************************************/
