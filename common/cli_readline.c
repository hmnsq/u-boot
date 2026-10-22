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

DECLARE_GLOBAL_তথ্য_PTR;

স্থির ধ্রুবক অক্ষর erase_seq[] = "\b \b";	/* erase sequence */
স্থির ধ্রুবক অক্ষর   tab_seq[] = "        ";	/* used to expand TABs */

অক্ষর console_বাফার[CONFIG_SYS_CBSIZE + 1];	/* console I/O buffer	*/

স্থির অক্ষর *delete_char (অক্ষর *বাফার, অক্ষর *p, পূর্ণসংখ্যা *colp, পূর্ণসংখ্যা *np, পূর্ণসংখ্যা plen)
{
	অক্ষর *s;

	যদি (*np == 0)
		ফেরত p;

	যদি (*(--p) == '\t') {		/* will retype the whole line */
		যখন (*colp > plen) {
			puts(erase_seq);
			(*colp)--;
		}
		জন্য (s = বাফার; s < p; ++s) {
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

	ফেরত p;
}

#ifdef CONFIG_CMDLINE_EDITING

/*
 * cmdline-editing related codes from vivi.
 * Author: Janghoon Lyu <nandy@mizi.com>
 */

#define putnstr(str, n)	printf("%.*s", (পূর্ণসংখ্যা)n, str)

#define CTL_BACKSPACE		('\b')
#define DEL			((অক্ষর)255)
#define DEL7			((অক্ষর)127)
#define CREAD_HIST_CHAR		('!')

#define getcmd_putch(ch)	putc(ch)
#define getcmd_getch()		getchar()
#define getcmd_cbeep()		getcmd_putch('\a')

#ifdef CONFIG_XPL_BUILD
#define HIST_MAX		3
#define HIST_আকার		32
#অন্যথা
#define HIST_MAX		20
#define HIST_আকার		CONFIG_SYS_CBSIZE
#endif

স্থির পূর্ণসংখ্যা hist_max;
স্থির পূর্ণসংখ্যা hist_add_idx;
স্থির পূর্ণসংখ্যা hist_cur = -1;
স্থির unsigned hist_অঙ্ক;

#ifndef CONFIG_CMD_HISTORY_USE_CALLOC
স্থির অক্ষর hist_তথ্য[HIST_MAX][HIST_আকার + 1];
#endif
স্থির অক্ষর *hist_list[HIST_MAX];

#define add_idx_minus_one() ((hist_add_idx == 0) ? hist_max : hist_add_idx-1)

স্থির শূন্য getcmd_putchars(পূর্ণসংখ্যা গুনতি, পূর্ণসংখ্যা ch)
{
	পূর্ণসংখ্যা i;

	জন্য (i = 0; i < গুনতি; i++)
		getcmd_putch(ch);
}

স্থির পূর্ণসংখ্যা hist_init(শূন্য)
{
	পূর্ণসংখ্যা i;

#ifndef CONFIG_CMD_HISTORY_USE_CALLOC
	জন্য (i = 0; i < HIST_MAX; i++) {
		hist_list[i] = hist_তথ্য[i];
		hist_list[i][0] = '\0';
	}
#অন্যথা
	unsigned অক্ষর *hist = calloc(HIST_MAX, HIST_আকার + 1);
	যদি (!hist)
		panic("%s: calloc: out of memory!\n", __func__);

	জন্য (i = 0; i < HIST_MAX; i++)
		hist_list[i] = hist + (i * (HIST_আকার + 1));
#endif

	hist_max = 0;
	hist_add_idx = 0;
	hist_cur = -1;
	hist_অঙ্ক = 0;

	ফেরত 0;
}

স্থির শূন্য cread_add_to_hist(অক্ষর *সারি)
{
	strcpy(hist_list[hist_add_idx], সারি);

	যদি (++hist_add_idx >= HIST_MAX)
		hist_add_idx = 0;

	যদি (hist_add_idx > hist_max)
		hist_max = hist_add_idx;

	hist_অঙ্ক++;
}

স্থির অক্ষর *hist_পূর্ব(শূন্য)
{
	অক্ষর *ret;
	পূর্ণসংখ্যা old_cur;

	যদি (hist_cur < 0)
		ফেরত ফাঁকা;

	old_cur = hist_cur;
	যদি (--hist_cur < 0)
		hist_cur = hist_max;

	যদি (hist_cur == hist_add_idx) {
		hist_cur = old_cur;
		ret = ফাঁকা;
	} অন্যথা {
		ret = hist_list[hist_cur];
	}

	ফেরত ret;
}

স্থির অক্ষর *hist_পরবর্তী(শূন্য)
{
	অক্ষর *ret;

	যদি (hist_cur < 0)
		ফেরত ফাঁকা;

	যদি (hist_cur == hist_add_idx)
		ফেরত ফাঁকা;

	যদি (++hist_cur > hist_max)
		hist_cur = 0;

	যদি (hist_cur == hist_add_idx)
		ret = "";
	অন্যথা
		ret = hist_list[hist_cur];

	ফেরত ret;
}

শূন্য cread_print_hist_list(শূন্য)
{
	পূর্ণসংখ্যা i;
	uint n;

	n = hist_অঙ্ক - hist_max;

	i = hist_add_idx + 1;
	যখন (1) {
		যদি (i > hist_max)
			i = 0;
		যদি (i == hist_add_idx)
			বিরতি;
		printf("%s\n", hist_list[i]);
		n++;
		i++;
	}
}

#define BEGINNING_OF_সারি() {			\
	যখন (cls->অঙ্ক) {			\
		getcmd_putch(CTL_BACKSPACE);	\
		cls->অঙ্ক--;			\
	}					\
}

#define ERASE_TO_EOL() {				\
	যদি (cls->অঙ্ক < cls->eol_অঙ্ক) {		\
		printf("%*s", (পূর্ণসংখ্যা)(cls->eol_অঙ্ক - cls->অঙ্ক), ""); \
		করো {					\
			getcmd_putch(CTL_BACKSPACE);	\
		} যখন (--cls->eol_অঙ্ক > cls->অঙ্ক);	\
	}						\
}

#define REFRESH_TO_EOL() {				\
	যদি (cls->অঙ্ক < cls->eol_অঙ্ক) {			\
		uint wlen = cls->eol_অঙ্ক - cls->অঙ্ক;	\
		putnstr(buf + cls->অঙ্ক, wlen);		\
		cls->অঙ্ক = cls->eol_অঙ্ক;		\
	}						\
}

স্থির শূন্য cread_add_char(অক্ষর ichar, পূর্ণসংখ্যা ঢোকাও, uint *অঙ্ক,
			   uint *eol_অঙ্ক, অক্ষর *buf, uint len)
{
	uint wlen;

	/* room ??? */
	যদি (ঢোকাও || *অঙ্ক == *eol_অঙ্ক) {
		যদি (*eol_অঙ্ক > len - 1) {
			getcmd_cbeep();
			ফেরত;
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
		যখন (--wlen)
			getcmd_putch(CTL_BACKSPACE);
	} অন্যথা {
		/* echo the character */
		wlen = 1;
		buf[*অঙ্ক] = ichar;
		putnstr(buf + *অঙ্ক, wlen);
		(*অঙ্ক)++;
	}
}

স্থির শূন্য cread_add_str(অক্ষর *str, পূর্ণসংখ্যা strsize, পূর্ণসংখ্যা ঢোকাও,
			  uint *অঙ্ক, uint *eol_অঙ্ক, অক্ষর *buf, uint len)
{
	যখন (strsize--) {
		cread_add_char(*str, ঢোকাও, অঙ্ক, eol_অঙ্ক, buf, len);
		str++;
	}
}

পূর্ণসংখ্যা cread_সারি_process_ch(কাঠামো cli_সারি_state *cls, অক্ষর ichar)
{
	অক্ষর *buf = cls->buf;

	/* ichar=0x0 when error occurs in U-Boot getc */
	যদি (!ichar)
		ফেরত -EAGAIN;

	যদি (ichar == '\n') {
		putc('\n');
		buf[cls->eol_অঙ্ক] = '\0';	/* terminate the string */
		ফেরত 0;
	}

	নির্বাচন (ichar) {
	ক্ষেত্র CTL_CH('a'):
		BEGINNING_OF_সারি();
		বিরতি;
	ক্ষেত্র CTL_CH('c'):	/* ^C - break */
		*buf = '\0';	/* discard input */
		ফেরত -EINTR;
	ক্ষেত্র CTL_CH('f'):
		যদি (cls->অঙ্ক < cls->eol_অঙ্ক) {
			getcmd_putch(buf[cls->অঙ্ক]);
			cls->অঙ্ক++;
		}
		বিরতি;
	ক্ষেত্র CTL_CH('b'):
		যদি (cls->অঙ্ক) {
			getcmd_putch(CTL_BACKSPACE);
			cls->অঙ্ক--;
		}
		বিরতি;
	ক্ষেত্র CTL_CH('d'):
		যদি (cls->অঙ্ক < cls->eol_অঙ্ক) {
			uint wlen;

			wlen = cls->eol_অঙ্ক - cls->অঙ্ক - 1;
			যদি (wlen) {
				memmove(&buf[cls->অঙ্ক], &buf[cls->অঙ্ক + 1],
					wlen);
				putnstr(buf + cls->অঙ্ক, wlen);
			}

			getcmd_putch(' ');
			করো {
				getcmd_putch(CTL_BACKSPACE);
			} যখন (wlen--);
			cls->eol_অঙ্ক--;
		}
		বিরতি;
	ক্ষেত্র CTL_CH('k'):
		ERASE_TO_EOL();
		বিরতি;
	ক্ষেত্র CTL_CH('e'):
		REFRESH_TO_EOL();
		বিরতি;
	ক্ষেত্র CTL_CH('o'):
		cls->ঢোকাও = !cls->ঢোকাও;
		বিরতি;
	ক্ষেত্র CTL_CH('w'):
		যদি (cls->অঙ্ক) {
			uint base, wlen;

			জন্য (base = cls->অঙ্ক;
			     base > 0 && buf[base - 1] == ' ';)
				base--;
			জন্য (; base > 0 && buf[base - 1] != ' ';)
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
		বিরতি;
	ক্ষেত্র CTL_CH('x'):
	ক্ষেত্র CTL_CH('u'):
		BEGINNING_OF_সারি();
		ERASE_TO_EOL();
		বিরতি;
	ক্ষেত্র DEL:
	ক্ষেত্র DEL7:
	ক্ষেত্র 8:
		যদি (cls->অঙ্ক) {
			uint wlen;

			wlen = cls->eol_অঙ্ক - cls->অঙ্ক;
			cls->অঙ্ক--;
			memmove(&buf[cls->অঙ্ক], &buf[cls->অঙ্ক + 1], wlen);
			getcmd_putch(CTL_BACKSPACE);
			putnstr(buf + cls->অঙ্ক, wlen);
			getcmd_putch(' ');
			করো {
				getcmd_putch(CTL_BACKSPACE);
			} যখন (wlen--);
			cls->eol_অঙ্ক--;
		}
		বিরতি;
	ক্ষেত্র CTL_CH('p'):
	ক্ষেত্র CTL_CH('n'):
		যদি (cls->history) {
			অক্ষর *hline;

			যদি (ichar == CTL_CH('p'))
				hline = hist_পূর্ব();
			অন্যথা
				hline = hist_পরবর্তী();

			যদি (!hline) {
				getcmd_cbeep();
				বিরতি;
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
			বিরতি;
		}
		বিরতি;
	ক্ষেত্র '\t':
		যদি (IS_ENABLED(CONFIG_AUTO_COMPLETE) && cls->cmd_complete) {
			পূর্ণসংখ্যা num2, col;

			/* do not autocomplete when in the middle */
			যদি (cls->অঙ্ক < cls->eol_অঙ্ক) {
				getcmd_cbeep();
				বিরতি;
			}

			buf[cls->অঙ্ক] = '\0';
			col = strlen(cls->prompt) + cls->eol_অঙ্ক;
			num2 = cls->অঙ্ক;
			যদি (cmd_auto_complete(cls->prompt, buf, &num2, &col)) {
				col = num2 - cls->অঙ্ক;
				cls->অঙ্ক += col;
				cls->eol_অঙ্ক += col;
			}
			বিরতি;
		}
		fallthrough;
	পূর্বনির্ধারিত:
		cread_add_char(ichar, cls->ঢোকাও, &cls->অঙ্ক, &cls->eol_অঙ্ক,
			       buf, cls->len);
		বিরতি;
	}

	/*
	 * keep the string terminated...if we added a char at the end then we
	 * want a \0 after it
	 */
	buf[cls->eol_অঙ্ক] = '\0';

	ফেরত -EAGAIN;
}

শূন্য cli_cread_init(কাঠামো cli_সারি_state *cls, অক্ষর *buf, uint buf_আকার)
{
	পূর্ণসংখ্যা init_len = strlen(buf);

	memset(cls, '\0', আকার(কাঠামো cli_সারি_state));
	cls->ঢোকাও = true;
	cls->buf = buf;
	cls->len = buf_আকার;

	যদি (init_len)
		cread_add_str(buf, init_len, 0, &cls->অঙ্ক, &cls->eol_অঙ্ক, buf,
			      buf_আকার);
}

স্থির পূর্ণসংখ্যা cread_সারি(ধ্রুবক অক্ষর *ধ্রুবক prompt, অক্ষর *buf, unsigned পূর্ণসংখ্যা *len,
		      পূর্ণসংখ্যা timeout)
{
	কাঠামো cli_ch_state s_cch, *cch = &s_cch;
	কাঠামো cli_সারি_state s_cls, *cls = &s_cls;
	অক্ষর ichar;
	পূর্ণসংখ্যা first = 1;

	cli_ch_init(cch);
	cli_cread_init(cls, buf, *len);
	cls->prompt = prompt;
	cls->history = true;
	cls->cmd_complete = true;

	যখন (1) {
		পূর্ণসংখ্যা ret;

		/* Check for saved characters */
		ichar = cli_ch_process(cch, 0);

		যদি (!ichar) {
			যদি (bootretry_tstc_timeout())
				ফেরত -2;	/* timed out */
			যদি (first && timeout) {
				u64 etime = endtick(timeout);

				যখন (!tstc()) {	/* while no incoming data */
					যদি (get_ticks() >= etime)
						ফেরত -2;	/* timed out */
					schedule();
				}
				first = 0;
			}

			ichar = getcmd_getch();
			ichar = cli_ch_process(cch, ichar);
		}

		ret = cread_সারি_process_ch(cls, ichar);
		যদি (ret == -EINTR)
			ফেরত -1;
		অন্যথা যদি (!ret)
			বিরতি;
	}
	*len = cls->eol_অঙ্ক;

	যদি (buf[0] && buf[0] != CREAD_HIST_CHAR)
		cread_add_to_hist(buf);
	hist_cur = hist_add_idx;

	ফেরত 0;
}

#অন্যথা /* !CONFIG_CMDLINE_EDITING */

স্থির অন্তঃরেখ পূর্ণসংখ্যা hist_init(শূন্য)
{
	ফেরত 0;
}

স্থির পূর্ণসংখ্যা cread_সারি(ধ্রুবক অক্ষর *ধ্রুবক prompt, অক্ষর *buf, unsigned পূর্ণসংখ্যা *len,
		      পূর্ণসংখ্যা timeout)
{
	ফেরত 0;
}

#endif /* CONFIG_CMDLINE_EDITING */

/****************************************************************************/

পূর্ণসংখ্যা cli_readline(ধ্রুবক অক্ষর *ধ্রুবক prompt)
{
	/*
	 * If console_buffer isn't 0-length the user will be prompted to modify
	 * it instead of entering it from scratch as desired.
	 */
	console_বাফার[0] = '\0';

	ফেরত cli_readline_into_বাফার(prompt, console_বাফার, 0);
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
স্থির পূর্ণসংখ্যা cread_সারি_simple(ধ্রুবক অক্ষর *ধ্রুবক prompt, অক্ষর *p)
{
	অক্ষর *p_buf = p;
	পূর্ণসংখ্যা n = 0;		/* buffer index */
	পূর্ণসংখ্যা plen = 0;		/* prompt length */
	পূর্ণসংখ্যা col;		/* output column cnt */
	পূর্ণসংখ্যা c;

	/* print prompt */
	যদি (prompt) {
		plen = strlen(prompt);
		puts(prompt);
	}
	col = plen;

	জন্য (;;) {
		যদি (bootretry_tstc_timeout())
			ফেরত -2;	/* timed out */
		schedule();	/* Trigger watchdog, if needed */

		c = getchar();

		/*
		 * Special character handling
		 */
		নির্বাচন (c) {
		ক্ষেত্র '\r':			/* Enter		*/
		ক্ষেত্র '\n':
			*p = '\0';
			puts("\r\n");
			ফেরত p - p_buf;

		ক্ষেত্র '\0':			/* nul			*/
			চালু;

		ক্ষেত্র 0x03:			/* ^C - break		*/
			p_buf[0] = '\0';	/* discard input */
			ফেরত -1;

		ক্ষেত্র 0x15:			/* ^U - erase line	*/
			যখন (col > plen) {
				puts(erase_seq);
				--col;
			}
			p = p_buf;
			n = 0;
			চালু;

		ক্ষেত্র 0x17:			/* ^W - erase word	*/
			p = delete_char(p_buf, p, &col, &n, plen);
			যখন ((n > 0) && (*p != ' '))
				p = delete_char(p_buf, p, &col, &n, plen);
			চালু;

		ক্ষেত্র 0x08:			/* ^H  - backspace	*/
		ক্ষেত্র 0x7F:			/* DEL - backspace	*/
			p = delete_char(p_buf, p, &col, &n, plen);
			চালু;

		পূর্বনির্ধারিত:
			/* Must be a normal character then */
			যদি (n >= CONFIG_SYS_CBSIZE - 2) { /* Buffer full */
				putc('\a');
				বিরতি;
			}
			যদি (c == '\t') {	/* expand TABs */
				যদি (IS_ENABLED(CONFIG_AUTO_COMPLETE)) {
					/*
					 * if auto-completion triggered just
					 * continue
					 */
					*p = '\0';
					যদি (cmd_auto_complete(prompt,
							      console_বাফার,
							      &n, &col)) {
						p = p_buf + n;	/* reset */
						চালু;
					}
				}
				puts(tab_seq + (col & 07));
				col += 8 - (col & 07);
			} অন্যথা {
				অক্ষর __maybe_unused buf[2];

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
			বিরতি;
		}
	}
}

পূর্ণসংখ্যা cli_readline_into_বাফার(ধ্রুবক অক্ষর *ধ্রুবক prompt, অক্ষর *বাফার,
			     পূর্ণসংখ্যা timeout)
{
	অক্ষর *p = বাফার;
	uint len = CONFIG_SYS_CBSIZE;
	পূর্ণসংখ্যা rc;
	স্থির পূর্ণসংখ্যা initted;

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
		ফেরত rc < 0 ? rc : len;

	} অন্যথা {
		ফেরত cread_সারি_simple(prompt, p);
	}
}
