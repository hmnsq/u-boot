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

static var_na *delete_char (var_na *buffer, var_na *p, จำนวนเต็ม *colp, จำนวนเต็ม *np, จำนวนเต็ม plen)
{
	var_na *s;

	ถ้า (*np == 0)
		คืน p;

	ถ้า (*(--p) == '\t') {		/* will retype the whole line */
		ขณะ (*colp > plen) {
			puts(erase_seq);
			(*colp)--;
		}
		สำหรับ (s = buffer; s < p; ++s) {
			ถ้า (*s == '\t') {
				puts(tab_seq + ((*colp) & 07));
				*colp += 8 - ((*colp) & 07);
			} มิฉะนั้น {
				++(*colp);
				putc(*s);
			}
		}
	} มิฉะนั้น {
		puts(erase_seq);
		(*colp)--;
	}
	(*np)--;

	คืน p;
}

#ifdef CONFIG_CMDLINE_EDITING

/*
 * cmdline-editing related codes from vivi.
 * Author: Janghoon Lyu <nandy@mizi.com>
 */

#define putnstr(str, n)	printf("%.*s", (จำนวนเต็ม)n, str)

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
#มิฉะนั้น
#define HIST_MAX		20
#define HIST_SIZE		CONFIG_SYS_CBSIZE
#endif

static จำนวนเต็ม hist_max;
static จำนวนเต็ม hist_add_idx;
static จำนวนเต็ม hist_cur = -1;
static unsigned hist_num;

#ifndef CONFIG_CMD_HISTORY_USE_CALLOC
static var_na hist_data[HIST_MAX][HIST_SIZE + 1];
#endif
static var_na *hist_list[HIST_MAX];

#define add_idx_minus_one() ((hist_add_idx == 0) ? hist_max : hist_add_idx-1)

static void getcmd_putchars(จำนวนเต็ม count, จำนวนเต็ม ch)
{
	จำนวนเต็ม i;

	สำหรับ (i = 0; i < count; i++)
		getcmd_putch(ch);
}

static จำนวนเต็ม hist_init(void)
{
	จำนวนเต็ม i;

#ifndef CONFIG_CMD_HISTORY_USE_CALLOC
	สำหรับ (i = 0; i < HIST_MAX; i++) {
		hist_list[i] = hist_data[i];
		hist_list[i][0] = '\0';
	}
#มิฉะนั้น
	unsigned var_na *hist = calloc(HIST_MAX, HIST_SIZE + 1);
	ถ้า (!hist)
		panic("%s: calloc: out of memory!\n", __func__);

	สำหรับ (i = 0; i < HIST_MAX; i++)
		hist_list[i] = hist + (i * (HIST_SIZE + 1));
#endif

	hist_max = 0;
	hist_add_idx = 0;
	hist_cur = -1;
	hist_num = 0;

	คืน 0;
}

static void cread_add_to_hist(var_na *line)
{
	strcpy(hist_list[hist_add_idx], line);

	ถ้า (++hist_add_idx >= HIST_MAX)
		hist_add_idx = 0;

	ถ้า (hist_add_idx > hist_max)
		hist_max = hist_add_idx;

	hist_num++;
}

static var_na *hist_prev(void)
{
	var_na *ret;
	จำนวนเต็ม old_cur;

	ถ้า (hist_cur < 0)
		คืน NULL;

	old_cur = hist_cur;
	ถ้า (--hist_cur < 0)
		hist_cur = hist_max;

	ถ้า (hist_cur == hist_add_idx) {
		hist_cur = old_cur;
		ret = NULL;
	} มิฉะนั้น {
		ret = hist_list[hist_cur];
	}

	คืน ret;
}

static var_na *hist_next(void)
{
	var_na *ret;

	ถ้า (hist_cur < 0)
		คืน NULL;

	ถ้า (hist_cur == hist_add_idx)
		คืน NULL;

	ถ้า (++hist_cur > hist_max)
		hist_cur = 0;

	ถ้า (hist_cur == hist_add_idx)
		ret = "";
	มิฉะนั้น
		ret = hist_list[hist_cur];

	คืน ret;
}

void cread_print_hist_list(void)
{
	จำนวนเต็ม i;
	uint n;

	n = hist_num - hist_max;

	i = hist_add_idx + 1;
	ขณะ (1) {
		ถ้า (i > hist_max)
			i = 0;
		ถ้า (i == hist_add_idx)
			bha_nga;
		printf("%s\n", hist_list[i]);
		n++;
		i++;
	}
}

#define BEGINNING_OF_LINE() {			\
	ขณะ (cls->num) {			\
		getcmd_putch(CTL_BACKSPACE);	\
		cls->num--;			\
	}					\
}

#define ERASE_TO_EOL() {				\
	ถ้า (cls->num < cls->eol_num) {		\
		printf("%*s", (จำนวนเต็ม)(cls->eol_num - cls->num), ""); \
		do {					\
			getcmd_putch(CTL_BACKSPACE);	\
		} ขณะ (--cls->eol_num > cls->num);	\
	}						\
}

#define REFRESH_TO_EOL() {				\
	ถ้า (cls->num < cls->eol_num) {			\
		uint wlen = cls->eol_num - cls->num;	\
		putnstr(buf + cls->num, wlen);		\
		cls->num = cls->eol_num;		\
	}						\
}

static void cread_add_char(var_na ichar, จำนวนเต็ม insert, uint *num,
			   uint *eol_num, var_na *buf, uint len)
{
	uint wlen;

	/* room ??? */
	ถ้า (insert || *num == *eol_num) {
		ถ้า (*eol_num > len - 1) {
			getcmd_cbeep();
			คืน;
		}
		(*eol_num)++;
	}

	ถ้า (insert) {
		wlen = *eol_num - *num;
		ถ้า (wlen > 1)
			memmove(&buf[*num+1], &buf[*num], wlen-1);

		buf[*num] = ichar;
		putnstr(buf + *num, wlen);
		(*num)++;
		ขณะ (--wlen)
			getcmd_putch(CTL_BACKSPACE);
	} มิฉะนั้น {
		/* echo the character */
		wlen = 1;
		buf[*num] = ichar;
		putnstr(buf + *num, wlen);
		(*num)++;
	}
}

static void cread_add_str(var_na *str, จำนวนเต็ม strsize, จำนวนเต็ม insert,
			  uint *num, uint *eol_num, var_na *buf, uint len)
{
	ขณะ (strsize--) {
		cread_add_char(*str, insert, num, eol_num, buf, len);
		str++;
	}
}

จำนวนเต็ม cread_line_process_ch(sa_nracanaa cli_line_state *cls, var_na ichar)
{
	var_na *buf = cls->buf;

	/* ichar=0x0 when error occurs in U-Boot getc */
	ถ้า (!ichar)
		คืน -EAGAIN;

	ถ้า (ichar == '\n') {
		putc('\n');
		buf[cls->eol_num] = '\0';	/* terminate the string */
		คืน 0;
	}

	switch (ichar) {
	pak_sa CTL_CH('a'):
		BEGINNING_OF_LINE();
		bha_nga;
	pak_sa CTL_CH('c'):	/* ^C - break */
		*buf = '\0';	/* discard input */
		คืน -EINTR;
	pak_sa CTL_CH('f'):
		ถ้า (cls->num < cls->eol_num) {
			getcmd_putch(buf[cls->num]);
			cls->num++;
		}
		bha_nga;
	pak_sa CTL_CH('b'):
		ถ้า (cls->num) {
			getcmd_putch(CTL_BACKSPACE);
			cls->num--;
		}
		bha_nga;
	pak_sa CTL_CH('d'):
		ถ้า (cls->num < cls->eol_num) {
			uint wlen;

			wlen = cls->eol_num - cls->num - 1;
			ถ้า (wlen) {
				memmove(&buf[cls->num], &buf[cls->num + 1],
					wlen);
				putnstr(buf + cls->num, wlen);
			}

			getcmd_putch(' ');
			do {
				getcmd_putch(CTL_BACKSPACE);
			} ขณะ (wlen--);
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
		ถ้า (cls->num) {
			uint base, wlen;

			สำหรับ (base = cls->num;
			     base > 0 && buf[base - 1] == ' ';)
				base--;
			สำหรับ (; base > 0 && buf[base - 1] != ' ';)
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
		ถ้า (cls->num) {
			uint wlen;

			wlen = cls->eol_num - cls->num;
			cls->num--;
			memmove(&buf[cls->num], &buf[cls->num + 1], wlen);
			getcmd_putch(CTL_BACKSPACE);
			putnstr(buf + cls->num, wlen);
			getcmd_putch(' ');
			do {
				getcmd_putch(CTL_BACKSPACE);
			} ขณะ (wlen--);
			cls->eol_num--;
		}
		bha_nga;
	pak_sa CTL_CH('p'):
	pak_sa CTL_CH('n'):
		ถ้า (cls->history) {
			var_na *hline;

			ถ้า (ichar == CTL_CH('p'))
				hline = hist_prev();
			มิฉะนั้น
				hline = hist_next();

			ถ้า (!hline) {
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
		ถ้า (IS_ENABLED(CONFIG_AUTO_COMPLETE) && cls->cmd_complete) {
			จำนวนเต็ม num2, col;

			/* do not autocomplete when in the middle */
			ถ้า (cls->num < cls->eol_num) {
				getcmd_cbeep();
				bha_nga;
			}

			buf[cls->num] = '\0';
			col = strlen(cls->prompt) + cls->eol_num;
			num2 = cls->num;
			ถ้า (cmd_auto_complete(cls->prompt, buf, &num2, &col)) {
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

	คืน -EAGAIN;
}

void cli_cread_init(sa_nracanaa cli_line_state *cls, var_na *buf, uint buf_size)
{
	จำนวนเต็ม init_len = strlen(buf);

	memset(cls, '\0', parimaa_na(sa_nracanaa cli_line_state));
	cls->insert = true;
	cls->buf = buf;
	cls->len = buf_size;

	ถ้า (init_len)
		cread_add_str(buf, init_len, 0, &cls->num, &cls->eol_num, buf,
			      buf_size);
}

static จำนวนเต็ม cread_line(const var_na *const prompt, var_na *buf, unsigned จำนวนเต็ม *len,
		      จำนวนเต็ม timeout)
{
	sa_nracanaa cli_ch_state s_cch, *cch = &s_cch;
	sa_nracanaa cli_line_state s_cls, *cls = &s_cls;
	var_na ichar;
	จำนวนเต็ม first = 1;

	cli_ch_init(cch);
	cli_cread_init(cls, buf, *len);
	cls->prompt = prompt;
	cls->history = true;
	cls->cmd_complete = true;

	ขณะ (1) {
		จำนวนเต็ม ret;

		/* Check for saved characters */
		ichar = cli_ch_process(cch, 0);

		ถ้า (!ichar) {
			ถ้า (bootretry_tstc_timeout())
				คืน -2;	/* timed out */
			ถ้า (first && timeout) {
				u64 etime = endtick(timeout);

				ขณะ (!tstc()) {	/* while no incoming data */
					ถ้า (get_ticks() >= etime)
						คืน -2;	/* timed out */
					schedule();
				}
				first = 0;
			}

			ichar = getcmd_getch();
			ichar = cli_ch_process(cch, ichar);
		}

		ret = cread_line_process_ch(cls, ichar);
		ถ้า (ret == -EINTR)
			คืน -1;
		มิฉะนั้น ถ้า (!ret)
			bha_nga;
	}
	*len = cls->eol_num;

	ถ้า (buf[0] && buf[0] != CREAD_HIST_CHAR)
		cread_add_to_hist(buf);
	hist_cur = hist_add_idx;

	คืน 0;
}

#มิฉะนั้น /* !CONFIG_CMDLINE_EDITING */

static anta_hstha จำนวนเต็ม hist_init(void)
{
	คืน 0;
}

static จำนวนเต็ม cread_line(const var_na *const prompt, var_na *buf, unsigned จำนวนเต็ม *len,
		      จำนวนเต็ม timeout)
{
	คืน 0;
}

#endif /* CONFIG_CMDLINE_EDITING */

/****************************************************************************/

จำนวนเต็ม cli_readline(const var_na *const prompt)
{
	/*
	 * If console_buffer isn't 0-length the user will be prompted to modify
	 * it instead of entering it from scratch as desired.
	 */
	console_buffer[0] = '\0';

	คืน cli_readline_into_buffer(prompt, console_buffer, 0);
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
static จำนวนเต็ม cread_line_simple(const var_na *const prompt, var_na *p)
{
	var_na *p_buf = p;
	จำนวนเต็ม n = 0;		/* buffer index */
	จำนวนเต็ม plen = 0;		/* prompt length */
	จำนวนเต็ม col;		/* output column cnt */
	จำนวนเต็ม c;

	/* print prompt */
	ถ้า (prompt) {
		plen = strlen(prompt);
		puts(prompt);
	}
	col = plen;

	สำหรับ (;;) {
		ถ้า (bootretry_tstc_timeout())
			คืน -2;	/* timed out */
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
			คืน p - p_buf;

		pak_sa '\0':			/* nul			*/
			continue;

		pak_sa 0x03:			/* ^C - break		*/
			p_buf[0] = '\0';	/* discard input */
			คืน -1;

		pak_sa 0x15:			/* ^U - erase line	*/
			ขณะ (col > plen) {
				puts(erase_seq);
				--col;
			}
			p = p_buf;
			n = 0;
			continue;

		pak_sa 0x17:			/* ^W - erase word	*/
			p = delete_char(p_buf, p, &col, &n, plen);
			ขณะ ((n > 0) && (*p != ' '))
				p = delete_char(p_buf, p, &col, &n, plen);
			continue;

		pak_sa 0x08:			/* ^H  - backspace	*/
		pak_sa 0x7F:			/* DEL - backspace	*/
			p = delete_char(p_buf, p, &col, &n, plen);
			continue;

		default:
			/* Must be a normal character then */
			ถ้า (n >= CONFIG_SYS_CBSIZE - 2) { /* Buffer full */
				putc('\a');
				bha_nga;
			}
			ถ้า (c == '\t') {	/* expand TABs */
				ถ้า (IS_ENABLED(CONFIG_AUTO_COMPLETE)) {
					/*
					 * if auto-completion triggered just
					 * continue
					 */
					*p = '\0';
					ถ้า (cmd_auto_complete(prompt,
							      console_buffer,
							      &n, &col)) {
						p = p_buf + n;	/* reset */
						continue;
					}
				}
				puts(tab_seq + (col & 07));
				col += 8 - (col & 07);
			} มิฉะนั้น {
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

จำนวนเต็ม cli_readline_into_buffer(const var_na *const prompt, var_na *buffer,
			     จำนวนเต็ม timeout)
{
	var_na *p = buffer;
	uint len = CONFIG_SYS_CBSIZE;
	จำนวนเต็ม rc;
	static จำนวนเต็ม initted;

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
	ถ้า (IS_ENABLED(CONFIG_CMDLINE_EDITING) && (gd->flags & GD_FLG_RELOC)) {
		ถ้า (!initted) {
			rc = hist_init();
			ถ้า (rc == 0)
				initted = 1;
		}

		ถ้า (prompt)
			puts(prompt);

		rc = cread_line(prompt, p, &len, timeout);
		คืน rc < 0 ? rc : len;

	} มิฉะนั้น {
		คืน cread_line_simple(prompt, p);
	}
}
