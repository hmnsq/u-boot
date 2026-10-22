// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Add to readline cmdline-editing by
 * (C) Copyright 2005
 * JinHua Luo, GuangDong Linux Center, <luo.jinhua@gd-linux.com>
 */

#অন্তর্ভাব <bootretry.h>
#অন্তর্ভাব <cli.h>
#অন্তর্ভাব <command.h>
#অন্তর্ভাব <hang.h>
#অন্তর্ভাব <malloc.h>
#অন্তর্ভাব <time.h>
#অন্তর্ভাব <watchdog.h>
#অন্তর্ভাব <linux/errno.h>
#অন্তর্ভাব <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

স্থির ধ্রুব বর্ণ erase_seq[] = "\b \b";	/* erase sequence */
স্থির ধ্রুব বর্ণ   tab_seq[] = "        ";	/* used to expand TABs */

বর্ণ console_buffer[CONFIG_SYS_CBSIZE + 1];	/* console I/O buffer	*/

স্থির বর্ণ *delete_char (বর্ণ *buffer, বর্ণ *p, পূর্ণাঙ্ক *colp, পূর্ণাঙ্ক *np, পূর্ণাঙ্ক plen)
{
	বর্ণ *s;

	যদি (*np == 0)
		প্রত্যাগম p;

	যদি (*(--p) == '\t') {		/* will retype the whole line */
		যাবত্ (*colp > plen) {
			puts(erase_seq);
			(*colp)--;
		}
		প্রতি (s = buffer; s < p; ++s) {
			যদি (*s == '\t') {
				puts(tab_seq + ((*colp) & 07));
				*colp += 8 - ((*colp) & 07);
			} অন্যথা {
				++(*colp);
				putc(*s);
			}
		}
	} অন্যথা {
		puts(erase_seq);
		(*colp)--;
	}
	(*np)--;

	প্রত্যাগম p;
}

#ifdef CONFIG_CMDLINE_EDITING

/*
 * cmdline-editing related codes from vivi.
 * Author: Janghoon Lyu <nandy@mizi.com>
 */

#define putnstr(str, n)	printf("%.*s", (পূর্ণাঙ্ক)n, str)

#define CTL_BACKSPACE		('\b')
#define DEL			((বর্ণ)255)
#define DEL7			((বর্ণ)127)
#define CREAD_HIST_CHAR		('!')

#define getcmd_putch(ch)	putc(ch)
#define getcmd_getch()		getchar()
#define getcmd_cbeep()		getcmd_putch('\a')

#ifdef CONFIG_XPL_BUILD
#define HIST_MAX		3
#define HIST_SIZE		32
#অন্যথা
#define HIST_MAX		20
#define HIST_SIZE		CONFIG_SYS_CBSIZE
#endif

স্থির পূর্ণাঙ্ক hist_max;
স্থির পূর্ণাঙ্ক hist_add_idx;
স্থির পূর্ণাঙ্ক hist_cur = -1;
স্থির unsigned hist_অঙ্ক;

#ifndef CONFIG_CMD_HISTORY_USE_CALLOC
স্থির বর্ণ hist_data[HIST_MAX][HIST_SIZE + 1];
#endif
স্থির বর্ণ *hist_list[HIST_MAX];

#define add_idx_minus_one() ((hist_add_idx == 0) ? hist_max : hist_add_idx-1)

স্থির শূন্য getcmd_putchars(পূর্ণাঙ্ক গুনতি, পূর্ণাঙ্ক ch)
{
	পূর্ণাঙ্ক i;

	প্রতি (i = 0; i < গুনতি; i++)
		getcmd_putch(ch);
}

স্থির পূর্ণাঙ্ক hist_init(শূন্য)
{
	পূর্ণাঙ্ক i;

#ifndef CONFIG_CMD_HISTORY_USE_CALLOC
	প্রতি (i = 0; i < HIST_MAX; i++) {
		hist_list[i] = hist_data[i];
		hist_list[i][0] = '\0';
	}
#অন্যথা
	unsigned বর্ণ *hist = calloc(HIST_MAX, HIST_SIZE + 1);
	যদি (!hist)
		panic("%s: calloc: out of memory!\n", __func__);

	প্রতি (i = 0; i < HIST_MAX; i++)
		hist_list[i] = hist + (i * (HIST_SIZE + 1));
#endif

	hist_max = 0;
	hist_add_idx = 0;
	hist_cur = -1;
	hist_অঙ্ক = 0;

	প্রত্যাগম 0;
}

স্থির শূন্য cread_add_to_hist(বর্ণ *সারি)
{
	strcpy(hist_list[hist_add_idx], সারি);

	যদি (++hist_add_idx >= HIST_MAX)
		hist_add_idx = 0;

	যদি (hist_add_idx > hist_max)
		hist_max = hist_add_idx;

	hist_অঙ্ক++;
}

স্থির বর্ণ *hist_prev(শূন্য)
{
	বর্ণ *ret;
	পূর্ণাঙ্ক old_cur;

	যদি (hist_cur < 0)
		প্রত্যাগম রিক্ত;

	old_cur = hist_cur;
	যদি (--hist_cur < 0)
		hist_cur = hist_max;

	যদি (hist_cur == hist_add_idx) {
		hist_cur = old_cur;
		ret = রিক্ত;
	} অন্যথা {
		ret = hist_list[hist_cur];
	}

	প্রত্যাগম ret;
}

স্থির বর্ণ *hist_next(শূন্য)
{
	বর্ণ *ret;

	যদি (hist_cur < 0)
		প্রত্যাগম রিক্ত;

	যদি (hist_cur == hist_add_idx)
		প্রত্যাগম রিক্ত;

	যদি (++hist_cur > hist_max)
		hist_cur = 0;

	যদি (hist_cur == hist_add_idx)
		ret = "";
	অন্যথা
		ret = hist_list[hist_cur];

	প্রত্যাগম ret;
}

শূন্য cread_print_hist_list(শূন্য)
{
	পূর্ণাঙ্ক i;
	uint n;

	n = hist_অঙ্ক - hist_max;

	i = hist_add_idx + 1;
	যাবত্ (1) {
		যদি (i > hist_max)
			i = 0;
		যদি (i == hist_add_idx)
			ভঙ্গ;
		printf("%s\n", hist_list[i]);
		n++;
		i++;
	}
}

#define BEGINNING_OF_সারি() {			\
	যাবত্ (cls->অঙ্ক) {			\
		getcmd_putch(CTL_BACKSPACE);	\
		cls->অঙ্ক--;			\
	}					\
}

#define ERASE_TO_EOL() {				\
	যদি (cls->অঙ্ক < cls->eol_অঙ্ক) {		\
		printf("%*s", (পূর্ণাঙ্ক)(cls->eol_অঙ্ক - cls->অঙ্ক), ""); \
		কুরু {					\
			getcmd_putch(CTL_BACKSPACE);	\
		} যাবত্ (--cls->eol_অঙ্ক > cls->অঙ্ক);	\
	}						\
}

#define REFRESH_TO_EOL() {				\
	যদি (cls->অঙ্ক < cls->eol_অঙ্ক) {			\
		uint wlen = cls->eol_অঙ্ক - cls->অঙ্ক;	\
		putnstr(buf + cls->অঙ্ক, wlen);		\
		cls->অঙ্ক = cls->eol_অঙ্ক;		\
	}						\
}

স্থির শূন্য cread_add_char(বর্ণ ichar, পূর্ণাঙ্ক ঢোকাও, uint *অঙ্ক,
			   uint *eol_অঙ্ক, বর্ণ *buf, uint len)
{
	uint wlen;

	/* room ??? */
	যদি (ঢোকাও || *অঙ্ক == *eol_অঙ্ক) {
		যদি (*eol_অঙ্ক > len - 1) {
			getcmd_cbeep();
			প্রত্যাগম;
		}
		(*eol_অঙ্ক)++;
	}

	যদি (ঢোকাও) {
		wlen = *eol_অঙ্ক - *অঙ্ক;
		যদি (wlen > 1)
			memmove(&buf[*অঙ্ক+1], &buf[*অঙ্ক], wlen-1);

		buf[*অঙ্ক] = ichar;
		putnstr(buf + *অঙ্ক, wlen);
		(*অঙ্ক)++;
		যাবত্ (--wlen)
			getcmd_putch(CTL_BACKSPACE);
	} অন্যথা {
		/* echo the character */
		wlen = 1;
		buf[*অঙ্ক] = ichar;
		putnstr(buf + *অঙ্ক, wlen);
		(*অঙ্ক)++;
	}
}

স্থির শূন্য cread_add_str(বর্ণ *str, পূর্ণাঙ্ক strsize, পূর্ণাঙ্ক ঢোকাও,
			  uint *অঙ্ক, uint *eol_অঙ্ক, বর্ণ *buf, uint len)
{
	যাবত্ (strsize--) {
		cread_add_char(*str, ঢোকাও, অঙ্ক, eol_অঙ্ক, buf, len);
		str++;
	}
}

পূর্ণাঙ্ক cread_সারি_process_ch(সংরচনা cli_সারি_state *cls, বর্ণ ichar)
{
	বর্ণ *buf = cls->buf;

	/* ichar=0x0 when error occurs in U-Boot getc */
	যদি (!ichar)
		প্রত্যাগম -EAGAIN;

	যদি (ichar == '\n') {
		putc('\n');
		buf[cls->eol_অঙ্ক] = '\0';	/* terminate the string */
		প্রত্যাগম 0;
	}

	বিকল্প (ichar) {
	পক্ষ CTL_CH('a'):
		BEGINNING_OF_সারি();
		ভঙ্গ;
	পক্ষ CTL_CH('c'):	/* ^C - break */
		*buf = '\0';	/* discard input */
		প্রত্যাগম -EINTR;
	পক্ষ CTL_CH('f'):
		যদি (cls->অঙ্ক < cls->eol_অঙ্ক) {
			getcmd_putch(buf[cls->অঙ্ক]);
			cls->অঙ্ক++;
		}
		ভঙ্গ;
	পক্ষ CTL_CH('b'):
		যদি (cls->অঙ্ক) {
			getcmd_putch(CTL_BACKSPACE);
			cls->অঙ্ক--;
		}
		ভঙ্গ;
	পক্ষ CTL_CH('d'):
		যদি (cls->অঙ্ক < cls->eol_অঙ্ক) {
			uint wlen;

			wlen = cls->eol_অঙ্ক - cls->অঙ্ক - 1;
			যদি (wlen) {
				memmove(&buf[cls->অঙ্ক], &buf[cls->অঙ্ক + 1],
					wlen);
				putnstr(buf + cls->অঙ্ক, wlen);
			}

			getcmd_putch(' ');
			কুরু {
				getcmd_putch(CTL_BACKSPACE);
			} যাবত্ (wlen--);
			cls->eol_অঙ্ক--;
		}
		ভঙ্গ;
	পক্ষ CTL_CH('k'):
		ERASE_TO_EOL();
		ভঙ্গ;
	পক্ষ CTL_CH('e'):
		REFRESH_TO_EOL();
		ভঙ্গ;
	পক্ষ CTL_CH('o'):
		cls->ঢোকাও = !cls->ঢোকাও;
		ভঙ্গ;
	পক্ষ CTL_CH('w'):
		যদি (cls->অঙ্ক) {
			uint base, wlen;

			প্রতি (base = cls->অঙ্ক;
			     base > 0 && buf[base - 1] == ' ';)
				base--;
			প্রতি (; base > 0 && buf[base - 1] != ' ';)
				base--;

			/* now delete chars from base to cls->num */
			wlen = cls->অঙ্ক - base;
			cls->eol_অঙ্ক -= wlen;
			memmove(&buf[base], &buf[cls->অঙ্ক],
				cls->eol_অঙ্ক - base + 1);
			cls->অঙ্ক = base;
			getcmd_putchars(wlen, CTL_BACKSPACE);
			puts(buf + base);
			getcmd_putchars(wlen, ' ');
			getcmd_putchars(wlen + cls->eol_অঙ্ক - cls->অঙ্ক,
					CTL_BACKSPACE);
		}
		ভঙ্গ;
	পক্ষ CTL_CH('x'):
	পক্ষ CTL_CH('u'):
		BEGINNING_OF_সারি();
		ERASE_TO_EOL();
		ভঙ্গ;
	পক্ষ DEL:
	পক্ষ DEL7:
	পক্ষ 8:
		যদি (cls->অঙ্ক) {
			uint wlen;

			wlen = cls->eol_অঙ্ক - cls->অঙ্ক;
			cls->অঙ্ক--;
			memmove(&buf[cls->অঙ্ক], &buf[cls->অঙ্ক + 1], wlen);
			getcmd_putch(CTL_BACKSPACE);
			putnstr(buf + cls->অঙ্ক, wlen);
			getcmd_putch(' ');
			কুরু {
				getcmd_putch(CTL_BACKSPACE);
			} যাবত্ (wlen--);
			cls->eol_অঙ্ক--;
		}
		ভঙ্গ;
	পক্ষ CTL_CH('p'):
	পক্ষ CTL_CH('n'):
		যদি (cls->history) {
			বর্ণ *hline;

			যদি (ichar == CTL_CH('p'))
				hline = hist_prev();
			অন্যথা
				hline = hist_next();

			যদি (!hline) {
				getcmd_cbeep();
				ভঙ্গ;
			}

			/* nuke the current line */
			/* first, go home */
			BEGINNING_OF_সারি();

			/* erase to end of line */
			ERASE_TO_EOL();

			/* copy new line into place and display */
			strcpy(buf, hline);
			cls->eol_অঙ্ক = strlen(buf);
			REFRESH_TO_EOL();
			ভঙ্গ;
		}
		ভঙ্গ;
	পক্ষ '\t':
		যদি (IS_ENABLED(CONFIG_AUTO_COMPLETE) && cls->cmd_complete) {
			পূর্ণাঙ্ক num2, col;

			/* do not autocomplete when in the middle */
			যদি (cls->অঙ্ক < cls->eol_অঙ্ক) {
				getcmd_cbeep();
				ভঙ্গ;
			}

			buf[cls->অঙ্ক] = '\0';
			col = strlen(cls->prompt) + cls->eol_অঙ্ক;
			num2 = cls->অঙ্ক;
			যদি (cmd_auto_complete(cls->prompt, buf, &num2, &col)) {
				col = num2 - cls->অঙ্ক;
				cls->অঙ্ক += col;
				cls->eol_অঙ্ক += col;
			}
			ভঙ্গ;
		}
		fallthrough;
	স্বভাব:
		cread_add_char(ichar, cls->ঢোকাও, &cls->অঙ্ক, &cls->eol_অঙ্ক,
			       buf, cls->len);
		ভঙ্গ;
	}

	/*
	 * keep the string terminated...if we added a char at the end then we
	 * want a \0 after it
	 */
	buf[cls->eol_অঙ্ক] = '\0';

	প্রত্যাগম -EAGAIN;
}

শূন্য cli_cread_init(সংরচনা cli_সারি_state *cls, বর্ণ *buf, uint buf_size)
{
	পূর্ণাঙ্ক init_len = strlen(buf);

	memset(cls, '\0', পরিমাণ(সংরচনা cli_সারি_state));
	cls->ঢোকাও = true;
	cls->buf = buf;
	cls->len = buf_size;

	যদি (init_len)
		cread_add_str(buf, init_len, 0, &cls->অঙ্ক, &cls->eol_অঙ্ক, buf,
			      buf_size);
}

স্থির পূর্ণাঙ্ক cread_সারি(ধ্রুব বর্ণ *ধ্রুব prompt, বর্ণ *buf, unsigned পূর্ণাঙ্ক *len,
		      পূর্ণাঙ্ক timeout)
{
	সংরচনা cli_ch_state s_cch, *cch = &s_cch;
	সংরচনা cli_সারি_state s_cls, *cls = &s_cls;
	বর্ণ ichar;
	পূর্ণাঙ্ক first = 1;

	cli_ch_init(cch);
	cli_cread_init(cls, buf, *len);
	cls->prompt = prompt;
	cls->history = true;
	cls->cmd_complete = true;

	যাবত্ (1) {
		পূর্ণাঙ্ক ret;

		/* Check for saved characters */
		ichar = cli_ch_process(cch, 0);

		যদি (!ichar) {
			যদি (bootretry_tstc_timeout())
				প্রত্যাগম -2;	/* timed out */
			যদি (first && timeout) {
				u64 etime = endtick(timeout);

				যাবত্ (!tstc()) {	/* while no incoming data */
					যদি (get_ticks() >= etime)
						প্রত্যাগম -2;	/* timed out */
					schedule();
				}
				first = 0;
			}

			ichar = getcmd_getch();
			ichar = cli_ch_process(cch, ichar);
		}

		ret = cread_সারি_process_ch(cls, ichar);
		যদি (ret == -EINTR)
			প্রত্যাগম -1;
		অন্যথা যদি (!ret)
			ভঙ্গ;
	}
	*len = cls->eol_অঙ্ক;

	যদি (buf[0] && buf[0] != CREAD_HIST_CHAR)
		cread_add_to_hist(buf);
	hist_cur = hist_add_idx;

	প্রত্যাগম 0;
}

#অন্যথা /* !CONFIG_CMDLINE_EDITING */

স্থির অন্তঃস্থ পূর্ণাঙ্ক hist_init(শূন্য)
{
	প্রত্যাগম 0;
}

স্থির পূর্ণাঙ্ক cread_সারি(ধ্রুব বর্ণ *ধ্রুব prompt, বর্ণ *buf, unsigned পূর্ণাঙ্ক *len,
		      পূর্ণাঙ্ক timeout)
{
	প্রত্যাগম 0;
}

#endif /* CONFIG_CMDLINE_EDITING */

/****************************************************************************/

পূর্ণাঙ্ক cli_readline(ধ্রুব বর্ণ *ধ্রুব prompt)
{
	/*
	 * If console_buffer isn't 0-length the user will be prompted to modify
	 * it instead of entering it from scratch as desired.
	 */
	console_buffer[0] = '\0';

	প্রত্যাগম cli_readline_into_buffer(prompt, console_buffer, 0);
}

/**
 * cread_line_simple() - Simple (small) command-line reader
 *
 * This supports only basic editing, with no cursor movement
 *
 * @prompt: Prompt to display
 * @p: Text buffer to edit
 * Return: length of text buffer, or -1 if input was cannncelled (Ctrl-C)
 */
স্থির পূর্ণাঙ্ক cread_সারি_simple(ধ্রুব বর্ণ *ধ্রুব prompt, বর্ণ *p)
{
	বর্ণ *p_buf = p;
	পূর্ণাঙ্ক n = 0;		/* buffer index */
	পূর্ণাঙ্ক plen = 0;		/* prompt length */
	পূর্ণাঙ্ক col;		/* output column cnt */
	পূর্ণাঙ্ক c;

	/* print prompt */
	যদি (prompt) {
		plen = strlen(prompt);
		puts(prompt);
	}
	col = plen;

	প্রতি (;;) {
		যদি (bootretry_tstc_timeout())
			প্রত্যাগম -2;	/* timed out */
		schedule();	/* Trigger watchdog, if needed */

		c = getchar();

		/*
		 * Special character handling
		 */
		বিকল্প (c) {
		পক্ষ '\r':			/* Enter		*/
		পক্ষ '\n':
			*p = '\0';
			puts("\r\n");
			প্রত্যাগম p - p_buf;

		পক্ষ '\0':			/* nul			*/
			অনুবর্ত;

		পক্ষ 0x03:			/* ^C - break		*/
			p_buf[0] = '\0';	/* discard input */
			প্রত্যাগম -1;

		পক্ষ 0x15:			/* ^U - erase line	*/
			যাবত্ (col > plen) {
				puts(erase_seq);
				--col;
			}
			p = p_buf;
			n = 0;
			অনুবর্ত;

		পক্ষ 0x17:			/* ^W - erase word	*/
			p = delete_char(p_buf, p, &col, &n, plen);
			যাবত্ ((n > 0) && (*p != ' '))
				p = delete_char(p_buf, p, &col, &n, plen);
			অনুবর্ত;

		পক্ষ 0x08:			/* ^H  - backspace	*/
		পক্ষ 0x7F:			/* DEL - backspace	*/
			p = delete_char(p_buf, p, &col, &n, plen);
			অনুবর্ত;

		স্বভাব:
			/* Must be a normal character then */
			যদি (n >= CONFIG_SYS_CBSIZE - 2) { /* Buffer full */
				putc('\a');
				ভঙ্গ;
			}
			যদি (c == '\t') {	/* expand TABs */
				যদি (IS_ENABLED(CONFIG_AUTO_COMPLETE)) {
					/*
					 * if auto-completion triggered just
					 * continue
					 */
					*p = '\0';
					যদি (cmd_auto_complete(prompt,
							      console_buffer,
							      &n, &col)) {
						p = p_buf + n;	/* reset */
						অনুবর্ত;
					}
				}
				puts(tab_seq + (col & 07));
				col += 8 - (col & 07);
			} অন্যথা {
				বর্ণ __maybe_unused buf[2];

				/*
				 * Echo input using puts() to force an LCD
				 * flush if we are using an LCD
				 */
				++col;
				buf[0] = c;
				buf[1] = '\0';
				puts(buf);
			}
			*p++ = c;
			++n;
			ভঙ্গ;
		}
	}
}

পূর্ণাঙ্ক cli_readline_into_buffer(ধ্রুব বর্ণ *ধ্রুব prompt, বর্ণ *buffer,
			     পূর্ণাঙ্ক timeout)
{
	বর্ণ *p = buffer;
	uint len = CONFIG_SYS_CBSIZE;
	পূর্ণাঙ্ক rc;
	স্থির পূর্ণাঙ্ক initted;

	/*
	 * Say N to CMD_HISTORY_USE_CALLOC will skip runtime
	 * allocation for the history buffer and directly
	 * use an uninitialized static array as the buffer.
	 * Doing this might have better performance and not
	 * increase the binary file's size, as it only marks
	 * the size. However, the array is only writable after
	 * relocation to RAM. If u-boot is running from ROM
	 * all the time, consider say Y to CMD_HISTORY_USE_CALLOC
	 * or disable CMD_HISTORY.
	 */
	যদি (IS_ENABLED(CONFIG_CMDLINE_EDITING) && (gd->flags & GD_FLG_RELOC)) {
		যদি (!initted) {
			rc = hist_init();
			যদি (rc == 0)
				initted = 1;
		}

		যদি (prompt)
			puts(prompt);

		rc = cread_সারি(prompt, p, &len, timeout);
		প্রত্যাগম rc < 0 ? rc : len;

	} অন্যথা {
		প্রত্যাগম cread_সারি_simple(prompt, p);
	}
}
