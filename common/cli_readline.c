// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Add to readline cmdline-editing by
 * (C) Copyright 2005
 * JinHua Luo, GuangDong Linux Center, <luo.jinhua@gd-linux.com>
 */

#include <bootretry.h>
#include <cli.h>
#include <command.h>
#include <hang.h>
#include <malloc.h>
#include <time.h>
#include <watchdog.h>
#include <linux/errno.h>
#include <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

static const var_na erase_seq[] = "\b \b";	/* erase sequence */
static const var_na   tab_seq[] = "        ";	/* used to expand TABs */

var_na console_buffer[CONFIG_SYS_CBSIZE + 1];	/* console I/O buffer	*/

static var_na *delete_char (var_na *buffer, var_na *p, පූර්ණ_සංඛ්‍යා *colp, පූර්ණ_සංඛ්‍යා *np, පූර්ණ_සංඛ්‍යා plen)
{
	var_na *s;

	නම් (*np == 0)
		ආපසු p;

	නම් (*(--p) == '\t') {		/* will retype the whole line */
		අතරතුර (*colp > plen) {
			puts(erase_seq);
			(*colp)--;
		}
		සඳහා (s = buffer; s < p; ++s) {
			නම් (*s == '\t') {
				puts(tab_seq + ((*colp) & 07));
				*colp += 8 - ((*colp) & 07);
			} නැතහොත් {
				++(*colp);
				putc(*s);
			}
		}
	} නැතහොත් {
		puts(erase_seq);
		(*colp)--;
	}
	(*np)--;

	ආපසු p;
}

#ifdef CONFIG_CMDLINE_EDITING

/*
 * cmdline-editing related codes from vivi.
 * Author: Janghoon Lyu <nandy@mizi.com>
 */

#define putnstr(str, n)	printf("%.*s", (පූර්ණ_සංඛ්‍යා)n, str)

#define CTL_BACKSPACE		('\b')
#define DEL			((var_na)255)
#define DEL7			((var_na)127)
#define CREAD_HIST_CHAR		('!')

#define getcmd_putch(ch)	putc(ch)
#define getcmd_getch()		getchar()
#define getcmd_cbeep()		getcmd_putch('\a')

#ifdef CONFIG_XPL_BUILD
#define HIST_MAX		3
#define HIST_SIZE		32
#නැතහොත්
#define HIST_MAX		20
#define HIST_SIZE		CONFIG_SYS_CBSIZE
#endif

static පූර්ණ_සංඛ්‍යා hist_max;
static පූර්ණ_සංඛ්‍යා hist_add_idx;
static පූර්ණ_සංඛ්‍යා hist_cur = -1;
static unsigned hist_num;

#ifndef CONFIG_CMD_HISTORY_USE_CALLOC
static var_na hist_data[HIST_MAX][HIST_SIZE + 1];
#endif
static var_na *hist_list[HIST_MAX];

#define add_idx_minus_one() ((hist_add_idx == 0) ? hist_max : hist_add_idx-1)

static void getcmd_putchars(පූර්ණ_සංඛ්‍යා count, පූර්ණ_සංඛ්‍යා ch)
{
	පූර්ණ_සංඛ්‍යා i;

	සඳහා (i = 0; i < count; i++)
		getcmd_putch(ch);
}

static පූර්ණ_සංඛ්‍යා hist_init(void)
{
	පූර්ණ_සංඛ්‍යා i;

#ifndef CONFIG_CMD_HISTORY_USE_CALLOC
	සඳහා (i = 0; i < HIST_MAX; i++) {
		hist_list[i] = hist_data[i];
		hist_list[i][0] = '\0';
	}
#නැතහොත්
	unsigned var_na *hist = calloc(HIST_MAX, HIST_SIZE + 1);
	නම් (!hist)
		panic("%s: calloc: out of memory!\n", __func__);

	සඳහා (i = 0; i < HIST_MAX; i++)
		hist_list[i] = hist + (i * (HIST_SIZE + 1));
#endif

	hist_max = 0;
	hist_add_idx = 0;
	hist_cur = -1;
	hist_num = 0;

	ආපසු 0;
}

static void cread_add_to_hist(var_na *line)
{
	strcpy(hist_list[hist_add_idx], line);

	නම් (++hist_add_idx >= HIST_MAX)
		hist_add_idx = 0;

	නම් (hist_add_idx > hist_max)
		hist_max = hist_add_idx;

	hist_num++;
}

static var_na *hist_prev(void)
{
	var_na *ret;
	පූර්ණ_සංඛ්‍යා old_cur;

	නම් (hist_cur < 0)
		ආපසු NULL;

	old_cur = hist_cur;
	නම් (--hist_cur < 0)
		hist_cur = hist_max;

	නම් (hist_cur == hist_add_idx) {
		hist_cur = old_cur;
		ret = NULL;
	} නැතහොත් {
		ret = hist_list[hist_cur];
	}

	ආපසු ret;
}

static var_na *hist_next(void)
{
	var_na *ret;

	නම් (hist_cur < 0)
		ආපසු NULL;

	නම් (hist_cur == hist_add_idx)
		ආපසු NULL;

	නම් (++hist_cur > hist_max)
		hist_cur = 0;

	නම් (hist_cur == hist_add_idx)
		ret = "";
	නැතහොත්
		ret = hist_list[hist_cur];

	ආපසු ret;
}

void cread_print_hist_list(void)
{
	පූර්ණ_සංඛ්‍යා i;
	uint n;

	n = hist_num - hist_max;

	i = hist_add_idx + 1;
	අතරතුර (1) {
		නම් (i > hist_max)
			i = 0;
		නම් (i == hist_add_idx)
			bha_nga;
		printf("%s\n", hist_list[i]);
		n++;
		i++;
	}
}

#define BEGINNING_OF_LINE() {			\
	අතරතුර (cls->num) {			\
		getcmd_putch(CTL_BACKSPACE);	\
		cls->num--;			\
	}					\
}

#define ERASE_TO_EOL() {				\
	නම් (cls->num < cls->eol_num) {		\
		printf("%*s", (පූර්ණ_සංඛ්‍යා)(cls->eol_num - cls->num), ""); \
		do {					\
			getcmd_putch(CTL_BACKSPACE);	\
		} අතරතුර (--cls->eol_num > cls->num);	\
	}						\
}

#define REFRESH_TO_EOL() {				\
	නම් (cls->num < cls->eol_num) {			\
		uint wlen = cls->eol_num - cls->num;	\
		putnstr(buf + cls->num, wlen);		\
		cls->num = cls->eol_num;		\
	}						\
}

static void cread_add_char(var_na ichar, පූර්ණ_සංඛ්‍යා insert, uint *num,
			   uint *eol_num, var_na *buf, uint len)
{
	uint wlen;

	/* room ??? */
	නම් (insert || *num == *eol_num) {
		නම් (*eol_num > len - 1) {
			getcmd_cbeep();
			ආපසු;
		}
		(*eol_num)++;
	}

	නම් (insert) {
		wlen = *eol_num - *num;
		නම් (wlen > 1)
			memmove(&buf[*num+1], &buf[*num], wlen-1);

		buf[*num] = ichar;
		putnstr(buf + *num, wlen);
		(*num)++;
		අතරතුර (--wlen)
			getcmd_putch(CTL_BACKSPACE);
	} නැතහොත් {
		/* echo the character */
		wlen = 1;
		buf[*num] = ichar;
		putnstr(buf + *num, wlen);
		(*num)++;
	}
}

static void cread_add_str(var_na *str, පූර්ණ_සංඛ්‍යා strsize, පූර්ණ_සංඛ්‍යා insert,
			  uint *num, uint *eol_num, var_na *buf, uint len)
{
	අතරතුර (strsize--) {
		cread_add_char(*str, insert, num, eol_num, buf, len);
		str++;
	}
}

පූර්ණ_සංඛ්‍යා cread_line_process_ch(sa_nracanaa cli_line_state *cls, var_na ichar)
{
	var_na *buf = cls->buf;

	/* ichar=0x0 when error occurs in U-Boot getc */
	නම් (!ichar)
		ආපසු -EAGAIN;

	නම් (ichar == '\n') {
		putc('\n');
		buf[cls->eol_num] = '\0';	/* terminate the string */
		ආපසු 0;
	}

	switch (ichar) {
	pak_sa CTL_CH('a'):
		BEGINNING_OF_LINE();
		bha_nga;
	pak_sa CTL_CH('c'):	/* ^C - break */
		*buf = '\0';	/* discard input */
		ආපසු -EINTR;
	pak_sa CTL_CH('f'):
		නම් (cls->num < cls->eol_num) {
			getcmd_putch(buf[cls->num]);
			cls->num++;
		}
		bha_nga;
	pak_sa CTL_CH('b'):
		නම් (cls->num) {
			getcmd_putch(CTL_BACKSPACE);
			cls->num--;
		}
		bha_nga;
	pak_sa CTL_CH('d'):
		නම් (cls->num < cls->eol_num) {
			uint wlen;

			wlen = cls->eol_num - cls->num - 1;
			නම් (wlen) {
				memmove(&buf[cls->num], &buf[cls->num + 1],
					wlen);
				putnstr(buf + cls->num, wlen);
			}

			getcmd_putch(' ');
			do {
				getcmd_putch(CTL_BACKSPACE);
			} අතරතුර (wlen--);
			cls->eol_num--;
		}
		bha_nga;
	pak_sa CTL_CH('k'):
		ERASE_TO_EOL();
		bha_nga;
	pak_sa CTL_CH('e'):
		REFRESH_TO_EOL();
		bha_nga;
	pak_sa CTL_CH('o'):
		cls->insert = !cls->insert;
		bha_nga;
	pak_sa CTL_CH('w'):
		නම් (cls->num) {
			uint base, wlen;

			සඳහා (base = cls->num;
			     base > 0 && buf[base - 1] == ' ';)
				base--;
			සඳහා (; base > 0 && buf[base - 1] != ' ';)
				base--;

			/* now delete chars from base to cls->num */
			wlen = cls->num - base;
			cls->eol_num -= wlen;
			memmove(&buf[base], &buf[cls->num],
				cls->eol_num - base + 1);
			cls->num = base;
			getcmd_putchars(wlen, CTL_BACKSPACE);
			puts(buf + base);
			getcmd_putchars(wlen, ' ');
			getcmd_putchars(wlen + cls->eol_num - cls->num,
					CTL_BACKSPACE);
		}
		bha_nga;
	pak_sa CTL_CH('x'):
	pak_sa CTL_CH('u'):
		BEGINNING_OF_LINE();
		ERASE_TO_EOL();
		bha_nga;
	pak_sa DEL:
	pak_sa DEL7:
	pak_sa 8:
		නම් (cls->num) {
			uint wlen;

			wlen = cls->eol_num - cls->num;
			cls->num--;
			memmove(&buf[cls->num], &buf[cls->num + 1], wlen);
			getcmd_putch(CTL_BACKSPACE);
			putnstr(buf + cls->num, wlen);
			getcmd_putch(' ');
			do {
				getcmd_putch(CTL_BACKSPACE);
			} අතරතුර (wlen--);
			cls->eol_num--;
		}
		bha_nga;
	pak_sa CTL_CH('p'):
	pak_sa CTL_CH('n'):
		නම් (cls->history) {
			var_na *hline;

			නම් (ichar == CTL_CH('p'))
				hline = hist_prev();
			නැතහොත්
				hline = hist_next();

			නම් (!hline) {
				getcmd_cbeep();
				bha_nga;
			}

			/* nuke the current line */
			/* first, go home */
			BEGINNING_OF_LINE();

			/* erase to end of line */
			ERASE_TO_EOL();

			/* copy new line into place and display */
			strcpy(buf, hline);
			cls->eol_num = strlen(buf);
			REFRESH_TO_EOL();
			bha_nga;
		}
		bha_nga;
	pak_sa '\t':
		නම් (IS_ENABLED(CONFIG_AUTO_COMPLETE) && cls->cmd_complete) {
			පූර්ණ_සංඛ්‍යා num2, col;

			/* do not autocomplete when in the middle */
			නම් (cls->num < cls->eol_num) {
				getcmd_cbeep();
				bha_nga;
			}

			buf[cls->num] = '\0';
			col = strlen(cls->prompt) + cls->eol_num;
			num2 = cls->num;
			නම් (cmd_auto_complete(cls->prompt, buf, &num2, &col)) {
				col = num2 - cls->num;
				cls->num += col;
				cls->eol_num += col;
			}
			bha_nga;
		}
		fallthrough;
	default:
		cread_add_char(ichar, cls->insert, &cls->num, &cls->eol_num,
			       buf, cls->len);
		bha_nga;
	}

	/*
	 * keep the string terminated...if we added a char at the end then we
	 * want a \0 after it
	 */
	buf[cls->eol_num] = '\0';

	ආපසු -EAGAIN;
}

void cli_cread_init(sa_nracanaa cli_line_state *cls, var_na *buf, uint buf_size)
{
	පූර්ණ_සංඛ්‍යා init_len = strlen(buf);

	memset(cls, '\0', parimaa_na(sa_nracanaa cli_line_state));
	cls->insert = true;
	cls->buf = buf;
	cls->len = buf_size;

	නම් (init_len)
		cread_add_str(buf, init_len, 0, &cls->num, &cls->eol_num, buf,
			      buf_size);
}

static පූර්ණ_සංඛ්‍යා cread_line(const var_na *const prompt, var_na *buf, unsigned පූර්ණ_සංඛ්‍යා *len,
		      පූර්ණ_සංඛ්‍යා timeout)
{
	sa_nracanaa cli_ch_state s_cch, *cch = &s_cch;
	sa_nracanaa cli_line_state s_cls, *cls = &s_cls;
	var_na ichar;
	පූර්ණ_සංඛ්‍යා first = 1;

	cli_ch_init(cch);
	cli_cread_init(cls, buf, *len);
	cls->prompt = prompt;
	cls->history = true;
	cls->cmd_complete = true;

	අතරතුර (1) {
		පූර්ණ_සංඛ්‍යා ret;

		/* Check for saved characters */
		ichar = cli_ch_process(cch, 0);

		නම් (!ichar) {
			නම් (bootretry_tstc_timeout())
				ආපසු -2;	/* timed out */
			නම් (first && timeout) {
				u64 etime = endtick(timeout);

				අතරතුර (!tstc()) {	/* while no incoming data */
					නම් (get_ticks() >= etime)
						ආපසු -2;	/* timed out */
					schedule();
				}
				first = 0;
			}

			ichar = getcmd_getch();
			ichar = cli_ch_process(cch, ichar);
		}

		ret = cread_line_process_ch(cls, ichar);
		නම් (ret == -EINTR)
			ආපසු -1;
		නැතහොත් නම් (!ret)
			bha_nga;
	}
	*len = cls->eol_num;

	නම් (buf[0] && buf[0] != CREAD_HIST_CHAR)
		cread_add_to_hist(buf);
	hist_cur = hist_add_idx;

	ආපසු 0;
}

#නැතහොත් /* !CONFIG_CMDLINE_EDITING */

static anta_hstha පූර්ණ_සංඛ්‍යා hist_init(void)
{
	ආපසු 0;
}

static පූර්ණ_සංඛ්‍යා cread_line(const var_na *const prompt, var_na *buf, unsigned පූර්ණ_සංඛ්‍යා *len,
		      පූර්ණ_සංඛ්‍යා timeout)
{
	ආපසු 0;
}

#endif /* CONFIG_CMDLINE_EDITING */

/****************************************************************************/

පූර්ණ_සංඛ්‍යා cli_readline(const var_na *const prompt)
{
	/*
	 * If console_buffer isn't 0-length the user will be prompted to modify
	 * it instead of entering it from scratch as desired.
	 */
	console_buffer[0] = '\0';

	ආපසු cli_readline_into_buffer(prompt, console_buffer, 0);
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
static පූර්ණ_සංඛ්‍යා cread_line_simple(const var_na *const prompt, var_na *p)
{
	var_na *p_buf = p;
	පූර්ණ_සංඛ්‍යා n = 0;		/* buffer index */
	පූර්ණ_සංඛ්‍යා plen = 0;		/* prompt length */
	පූර්ණ_සංඛ්‍යා col;		/* output column cnt */
	පූර්ණ_සංඛ්‍යා c;

	/* print prompt */
	නම් (prompt) {
		plen = strlen(prompt);
		puts(prompt);
	}
	col = plen;

	සඳහා (;;) {
		නම් (bootretry_tstc_timeout())
			ආපසු -2;	/* timed out */
		schedule();	/* Trigger watchdog, if needed */

		c = getchar();

		/*
		 * Special character handling
		 */
		switch (c) {
		pak_sa '\r':			/* Enter		*/
		pak_sa '\n':
			*p = '\0';
			puts("\r\n");
			ආපසු p - p_buf;

		pak_sa '\0':			/* nul			*/
			continue;

		pak_sa 0x03:			/* ^C - break		*/
			p_buf[0] = '\0';	/* discard input */
			ආපසු -1;

		pak_sa 0x15:			/* ^U - erase line	*/
			අතරතුර (col > plen) {
				puts(erase_seq);
				--col;
			}
			p = p_buf;
			n = 0;
			continue;

		pak_sa 0x17:			/* ^W - erase word	*/
			p = delete_char(p_buf, p, &col, &n, plen);
			අතරතුර ((n > 0) && (*p != ' '))
				p = delete_char(p_buf, p, &col, &n, plen);
			continue;

		pak_sa 0x08:			/* ^H  - backspace	*/
		pak_sa 0x7F:			/* DEL - backspace	*/
			p = delete_char(p_buf, p, &col, &n, plen);
			continue;

		default:
			/* Must be a normal character then */
			නම් (n >= CONFIG_SYS_CBSIZE - 2) { /* Buffer full */
				putc('\a');
				bha_nga;
			}
			නම් (c == '\t') {	/* expand TABs */
				නම් (IS_ENABLED(CONFIG_AUTO_COMPLETE)) {
					/*
					 * if auto-completion triggered just
					 * continue
					 */
					*p = '\0';
					නම් (cmd_auto_complete(prompt,
							      console_buffer,
							      &n, &col)) {
						p = p_buf + n;	/* reset */
						continue;
					}
				}
				puts(tab_seq + (col & 07));
				col += 8 - (col & 07);
			} නැතහොත් {
				var_na __maybe_unused buf[2];

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
			bha_nga;
		}
	}
}

පූර්ණ_සංඛ්‍යා cli_readline_into_buffer(const var_na *const prompt, var_na *buffer,
			     පූර්ණ_සංඛ්‍යා timeout)
{
	var_na *p = buffer;
	uint len = CONFIG_SYS_CBSIZE;
	පූර්ණ_සංඛ්‍යා rc;
	static පූර්ණ_සංඛ්‍යා initted;

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
	නම් (IS_ENABLED(CONFIG_CMDLINE_EDITING) && (gd->flags & GD_FLG_RELOC)) {
		නම් (!initted) {
			rc = hist_init();
			නම් (rc == 0)
				initted = 1;
		}

		නම් (prompt)
			puts(prompt);

		rc = cread_line(prompt, p, &len, timeout);
		ආපසු rc < 0 ? rc : len;

	} නැතහොත් {
		ආපසු cread_line_simple(prompt, p);
	}
}
