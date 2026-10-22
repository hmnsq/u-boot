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

static var_na *delete_char (var_na *buffer, var_na *p, ຈຳນວນເຕັມ *colp, ຈຳນວນເຕັມ *np, ຈຳນວນເຕັມ plen)
{
	var_na *s;

	ຖ້າ (*np == 0)
		ສົ່ງຄືນ p;

	ຖ້າ (*(--p) == '\t') {		/* will retype the whole line */
		ໃນຂະນະທີ່ (*colp > plen) {
			puts(erase_seq);
			(*colp)--;
		}
		ສຳລັບ (s = buffer; s < p; ++s) {
			ຖ້າ (*s == '\t') {
				puts(tab_seq + ((*colp) & 07));
				*colp += 8 - ((*colp) & 07);
			} ບໍ່ດັ່ງນັ້ນ {
				++(*colp);
				putc(*s);
			}
		}
	} ບໍ່ດັ່ງນັ້ນ {
		puts(erase_seq);
		(*colp)--;
	}
	(*np)--;

	ສົ່ງຄືນ p;
}

#ifdef CONFIG_CMDLINE_EDITING

/*
 * cmdline-editing related codes from vivi.
 * Author: Janghoon Lyu <nandy@mizi.com>
 */

#define putnstr(str, n)	printf("%.*s", (ຈຳນວນເຕັມ)n, str)

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
#ບໍ່ດັ່ງນັ້ນ
#define HIST_MAX		20
#define HIST_SIZE		CONFIG_SYS_CBSIZE
#endif

static ຈຳນວນເຕັມ hist_max;
static ຈຳນວນເຕັມ hist_add_idx;
static ຈຳນວນເຕັມ hist_cur = -1;
static unsigned hist_num;

#ifndef CONFIG_CMD_HISTORY_USE_CALLOC
static var_na hist_data[HIST_MAX][HIST_SIZE + 1];
#endif
static var_na *hist_list[HIST_MAX];

#define add_idx_minus_one() ((hist_add_idx == 0) ? hist_max : hist_add_idx-1)

static void getcmd_putchars(ຈຳນວນເຕັມ count, ຈຳນວນເຕັມ ch)
{
	ຈຳນວນເຕັມ i;

	ສຳລັບ (i = 0; i < count; i++)
		getcmd_putch(ch);
}

static ຈຳນວນເຕັມ hist_init(void)
{
	ຈຳນວນເຕັມ i;

#ifndef CONFIG_CMD_HISTORY_USE_CALLOC
	ສຳລັບ (i = 0; i < HIST_MAX; i++) {
		hist_list[i] = hist_data[i];
		hist_list[i][0] = '\0';
	}
#ບໍ່ດັ່ງນັ້ນ
	unsigned var_na *hist = calloc(HIST_MAX, HIST_SIZE + 1);
	ຖ້າ (!hist)
		panic("%s: calloc: out of memory!\n", __func__);

	ສຳລັບ (i = 0; i < HIST_MAX; i++)
		hist_list[i] = hist + (i * (HIST_SIZE + 1));
#endif

	hist_max = 0;
	hist_add_idx = 0;
	hist_cur = -1;
	hist_num = 0;

	ສົ່ງຄືນ 0;
}

static void cread_add_to_hist(var_na *line)
{
	strcpy(hist_list[hist_add_idx], line);

	ຖ້າ (++hist_add_idx >= HIST_MAX)
		hist_add_idx = 0;

	ຖ້າ (hist_add_idx > hist_max)
		hist_max = hist_add_idx;

	hist_num++;
}

static var_na *hist_prev(void)
{
	var_na *ret;
	ຈຳນວນເຕັມ old_cur;

	ຖ້າ (hist_cur < 0)
		ສົ່ງຄືນ NULL;

	old_cur = hist_cur;
	ຖ້າ (--hist_cur < 0)
		hist_cur = hist_max;

	ຖ້າ (hist_cur == hist_add_idx) {
		hist_cur = old_cur;
		ret = NULL;
	} ບໍ່ດັ່ງນັ້ນ {
		ret = hist_list[hist_cur];
	}

	ສົ່ງຄືນ ret;
}

static var_na *hist_next(void)
{
	var_na *ret;

	ຖ້າ (hist_cur < 0)
		ສົ່ງຄືນ NULL;

	ຖ້າ (hist_cur == hist_add_idx)
		ສົ່ງຄືນ NULL;

	ຖ້າ (++hist_cur > hist_max)
		hist_cur = 0;

	ຖ້າ (hist_cur == hist_add_idx)
		ret = "";
	ບໍ່ດັ່ງນັ້ນ
		ret = hist_list[hist_cur];

	ສົ່ງຄືນ ret;
}

void cread_print_hist_list(void)
{
	ຈຳນວນເຕັມ i;
	uint n;

	n = hist_num - hist_max;

	i = hist_add_idx + 1;
	ໃນຂະນະທີ່ (1) {
		ຖ້າ (i > hist_max)
			i = 0;
		ຖ້າ (i == hist_add_idx)
			bha_nga;
		printf("%s\n", hist_list[i]);
		n++;
		i++;
	}
}

#define BEGINNING_OF_LINE() {			\
	ໃນຂະນະທີ່ (cls->num) {			\
		getcmd_putch(CTL_BACKSPACE);	\
		cls->num--;			\
	}					\
}

#define ERASE_TO_EOL() {				\
	ຖ້າ (cls->num < cls->eol_num) {		\
		printf("%*s", (ຈຳນວນເຕັມ)(cls->eol_num - cls->num), ""); \
		do {					\
			getcmd_putch(CTL_BACKSPACE);	\
		} ໃນຂະນະທີ່ (--cls->eol_num > cls->num);	\
	}						\
}

#define REFRESH_TO_EOL() {				\
	ຖ້າ (cls->num < cls->eol_num) {			\
		uint wlen = cls->eol_num - cls->num;	\
		putnstr(buf + cls->num, wlen);		\
		cls->num = cls->eol_num;		\
	}						\
}

static void cread_add_char(var_na ichar, ຈຳນວນເຕັມ insert, uint *num,
			   uint *eol_num, var_na *buf, uint len)
{
	uint wlen;

	/* room ??? */
	ຖ້າ (insert || *num == *eol_num) {
		ຖ້າ (*eol_num > len - 1) {
			getcmd_cbeep();
			ສົ່ງຄືນ;
		}
		(*eol_num)++;
	}

	ຖ້າ (insert) {
		wlen = *eol_num - *num;
		ຖ້າ (wlen > 1)
			memmove(&buf[*num+1], &buf[*num], wlen-1);

		buf[*num] = ichar;
		putnstr(buf + *num, wlen);
		(*num)++;
		ໃນຂະນະທີ່ (--wlen)
			getcmd_putch(CTL_BACKSPACE);
	} ບໍ່ດັ່ງນັ້ນ {
		/* echo the character */
		wlen = 1;
		buf[*num] = ichar;
		putnstr(buf + *num, wlen);
		(*num)++;
	}
}

static void cread_add_str(var_na *str, ຈຳນວນເຕັມ strsize, ຈຳນວນເຕັມ insert,
			  uint *num, uint *eol_num, var_na *buf, uint len)
{
	ໃນຂະນະທີ່ (strsize--) {
		cread_add_char(*str, insert, num, eol_num, buf, len);
		str++;
	}
}

ຈຳນວນເຕັມ cread_line_process_ch(sa_nracanaa cli_line_state *cls, var_na ichar)
{
	var_na *buf = cls->buf;

	/* ichar=0x0 when error occurs in U-Boot getc */
	ຖ້າ (!ichar)
		ສົ່ງຄືນ -EAGAIN;

	ຖ້າ (ichar == '\n') {
		putc('\n');
		buf[cls->eol_num] = '\0';	/* terminate the string */
		ສົ່ງຄືນ 0;
	}

	switch (ichar) {
	pak_sa CTL_CH('a'):
		BEGINNING_OF_LINE();
		bha_nga;
	pak_sa CTL_CH('c'):	/* ^C - break */
		*buf = '\0';	/* discard input */
		ສົ່ງຄືນ -EINTR;
	pak_sa CTL_CH('f'):
		ຖ້າ (cls->num < cls->eol_num) {
			getcmd_putch(buf[cls->num]);
			cls->num++;
		}
		bha_nga;
	pak_sa CTL_CH('b'):
		ຖ້າ (cls->num) {
			getcmd_putch(CTL_BACKSPACE);
			cls->num--;
		}
		bha_nga;
	pak_sa CTL_CH('d'):
		ຖ້າ (cls->num < cls->eol_num) {
			uint wlen;

			wlen = cls->eol_num - cls->num - 1;
			ຖ້າ (wlen) {
				memmove(&buf[cls->num], &buf[cls->num + 1],
					wlen);
				putnstr(buf + cls->num, wlen);
			}

			getcmd_putch(' ');
			do {
				getcmd_putch(CTL_BACKSPACE);
			} ໃນຂະນະທີ່ (wlen--);
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
		ຖ້າ (cls->num) {
			uint base, wlen;

			ສຳລັບ (base = cls->num;
			     base > 0 && buf[base - 1] == ' ';)
				base--;
			ສຳລັບ (; base > 0 && buf[base - 1] != ' ';)
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
		ຖ້າ (cls->num) {
			uint wlen;

			wlen = cls->eol_num - cls->num;
			cls->num--;
			memmove(&buf[cls->num], &buf[cls->num + 1], wlen);
			getcmd_putch(CTL_BACKSPACE);
			putnstr(buf + cls->num, wlen);
			getcmd_putch(' ');
			do {
				getcmd_putch(CTL_BACKSPACE);
			} ໃນຂະນະທີ່ (wlen--);
			cls->eol_num--;
		}
		bha_nga;
	pak_sa CTL_CH('p'):
	pak_sa CTL_CH('n'):
		ຖ້າ (cls->history) {
			var_na *hline;

			ຖ້າ (ichar == CTL_CH('p'))
				hline = hist_prev();
			ບໍ່ດັ່ງນັ້ນ
				hline = hist_next();

			ຖ້າ (!hline) {
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
		ຖ້າ (IS_ENABLED(CONFIG_AUTO_COMPLETE) && cls->cmd_complete) {
			ຈຳນວນເຕັມ num2, col;

			/* do not autocomplete when in the middle */
			ຖ້າ (cls->num < cls->eol_num) {
				getcmd_cbeep();
				bha_nga;
			}

			buf[cls->num] = '\0';
			col = strlen(cls->prompt) + cls->eol_num;
			num2 = cls->num;
			ຖ້າ (cmd_auto_complete(cls->prompt, buf, &num2, &col)) {
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

	ສົ່ງຄືນ -EAGAIN;
}

void cli_cread_init(sa_nracanaa cli_line_state *cls, var_na *buf, uint buf_size)
{
	ຈຳນວນເຕັມ init_len = strlen(buf);

	memset(cls, '\0', parimaa_na(sa_nracanaa cli_line_state));
	cls->insert = true;
	cls->buf = buf;
	cls->len = buf_size;

	ຖ້າ (init_len)
		cread_add_str(buf, init_len, 0, &cls->num, &cls->eol_num, buf,
			      buf_size);
}

static ຈຳນວນເຕັມ cread_line(const var_na *const prompt, var_na *buf, unsigned ຈຳນວນເຕັມ *len,
		      ຈຳນວນເຕັມ timeout)
{
	sa_nracanaa cli_ch_state s_cch, *cch = &s_cch;
	sa_nracanaa cli_line_state s_cls, *cls = &s_cls;
	var_na ichar;
	ຈຳນວນເຕັມ first = 1;

	cli_ch_init(cch);
	cli_cread_init(cls, buf, *len);
	cls->prompt = prompt;
	cls->history = true;
	cls->cmd_complete = true;

	ໃນຂະນະທີ່ (1) {
		ຈຳນວນເຕັມ ret;

		/* Check for saved characters */
		ichar = cli_ch_process(cch, 0);

		ຖ້າ (!ichar) {
			ຖ້າ (bootretry_tstc_timeout())
				ສົ່ງຄືນ -2;	/* timed out */
			ຖ້າ (first && timeout) {
				u64 etime = endtick(timeout);

				ໃນຂະນະທີ່ (!tstc()) {	/* while no incoming data */
					ຖ້າ (get_ticks() >= etime)
						ສົ່ງຄືນ -2;	/* timed out */
					schedule();
				}
				first = 0;
			}

			ichar = getcmd_getch();
			ichar = cli_ch_process(cch, ichar);
		}

		ret = cread_line_process_ch(cls, ichar);
		ຖ້າ (ret == -EINTR)
			ສົ່ງຄືນ -1;
		ບໍ່ດັ່ງນັ້ນ ຖ້າ (!ret)
			bha_nga;
	}
	*len = cls->eol_num;

	ຖ້າ (buf[0] && buf[0] != CREAD_HIST_CHAR)
		cread_add_to_hist(buf);
	hist_cur = hist_add_idx;

	ສົ່ງຄືນ 0;
}

#ບໍ່ດັ່ງນັ້ນ /* !CONFIG_CMDLINE_EDITING */

static anta_hstha ຈຳນວນເຕັມ hist_init(void)
{
	ສົ່ງຄືນ 0;
}

static ຈຳນວນເຕັມ cread_line(const var_na *const prompt, var_na *buf, unsigned ຈຳນວນເຕັມ *len,
		      ຈຳນວນເຕັມ timeout)
{
	ສົ່ງຄືນ 0;
}

#endif /* CONFIG_CMDLINE_EDITING */

/****************************************************************************/

ຈຳນວນເຕັມ cli_readline(const var_na *const prompt)
{
	/*
	 * If console_buffer isn't 0-length the user will be prompted to modify
	 * it instead of entering it from scratch as desired.
	 */
	console_buffer[0] = '\0';

	ສົ່ງຄືນ cli_readline_into_buffer(prompt, console_buffer, 0);
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
static ຈຳນວນເຕັມ cread_line_simple(const var_na *const prompt, var_na *p)
{
	var_na *p_buf = p;
	ຈຳນວນເຕັມ n = 0;		/* buffer index */
	ຈຳນວນເຕັມ plen = 0;		/* prompt length */
	ຈຳນວນເຕັມ col;		/* output column cnt */
	ຈຳນວນເຕັມ c;

	/* print prompt */
	ຖ້າ (prompt) {
		plen = strlen(prompt);
		puts(prompt);
	}
	col = plen;

	ສຳລັບ (;;) {
		ຖ້າ (bootretry_tstc_timeout())
			ສົ່ງຄືນ -2;	/* timed out */
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
			ສົ່ງຄືນ p - p_buf;

		pak_sa '\0':			/* nul			*/
			continue;

		pak_sa 0x03:			/* ^C - break		*/
			p_buf[0] = '\0';	/* discard input */
			ສົ່ງຄືນ -1;

		pak_sa 0x15:			/* ^U - erase line	*/
			ໃນຂະນະທີ່ (col > plen) {
				puts(erase_seq);
				--col;
			}
			p = p_buf;
			n = 0;
			continue;

		pak_sa 0x17:			/* ^W - erase word	*/
			p = delete_char(p_buf, p, &col, &n, plen);
			ໃນຂະນະທີ່ ((n > 0) && (*p != ' '))
				p = delete_char(p_buf, p, &col, &n, plen);
			continue;

		pak_sa 0x08:			/* ^H  - backspace	*/
		pak_sa 0x7F:			/* DEL - backspace	*/
			p = delete_char(p_buf, p, &col, &n, plen);
			continue;

		default:
			/* Must be a normal character then */
			ຖ້າ (n >= CONFIG_SYS_CBSIZE - 2) { /* Buffer full */
				putc('\a');
				bha_nga;
			}
			ຖ້າ (c == '\t') {	/* expand TABs */
				ຖ້າ (IS_ENABLED(CONFIG_AUTO_COMPLETE)) {
					/*
					 * if auto-completion triggered just
					 * continue
					 */
					*p = '\0';
					ຖ້າ (cmd_auto_complete(prompt,
							      console_buffer,
							      &n, &col)) {
						p = p_buf + n;	/* reset */
						continue;
					}
				}
				puts(tab_seq + (col & 07));
				col += 8 - (col & 07);
			} ບໍ່ດັ່ງນັ້ນ {
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

ຈຳນວນເຕັມ cli_readline_into_buffer(const var_na *const prompt, var_na *buffer,
			     ຈຳນວນເຕັມ timeout)
{
	var_na *p = buffer;
	uint len = CONFIG_SYS_CBSIZE;
	ຈຳນວນເຕັມ rc;
	static ຈຳນວນເຕັມ initted;

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
	ຖ້າ (IS_ENABLED(CONFIG_CMDLINE_EDITING) && (gd->flags & GD_FLG_RELOC)) {
		ຖ້າ (!initted) {
			rc = hist_init();
			ຖ້າ (rc == 0)
				initted = 1;
		}

		ຖ້າ (prompt)
			puts(prompt);

		rc = cread_line(prompt, p, &len, timeout);
		ສົ່ງຄືນ rc < 0 ? rc : len;

	} ບໍ່ດັ່ງນັ້ນ {
		ສົ່ງຄືນ cread_line_simple(prompt, p);
	}
}
