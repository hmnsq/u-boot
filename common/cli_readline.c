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

static const char erase_seq[] = "\b \b";	/* erase sequence */
static const char   tab_seq[] = "        ";	/* used to expand TABs */

char console_buffer[CONFIG_SYS_CBSIZE + 1];	/* console I/O buffer	*/

static char *delete_char (char *buffer, char *p, цео *colp, цео *np, цео plen)
{
	char *s;

	ако (*np == 0)
		врати p;

	ако (*(--p) == '\t') {		/* will retype the whole line */
		док (*colp > plen) {
			puts(erase_seq);
			(*colp)--;
		}
		за (s = buffer; s < p; ++s) {
			ако (*s == '\t') {
				puts(tab_seq + ((*colp) & 07));
				*colp += 8 - ((*colp) & 07);
			} инако {
				++(*colp);
				putc(*s);
			}
		}
	} инако {
		puts(erase_seq);
		(*colp)--;
	}
	(*np)--;

	врати p;
}

#ifdef CONFIG_CMDLINE_EDITING

/*
 * cmdline-editing related codes from vivi.
 * Author: Janghoon Lyu <nandy@mizi.com>
 */

#define putnstr(str, n)	printf("%.*s", (цео)n, str)

#define CTL_BACKSPACE		('\b')
#define DEL			((char)255)
#define DEL7			((char)127)
#define CREAD_HIST_CHAR		('!')

#define getcmd_putch(ch)	putc(ch)
#define getcmd_getch()		getchar()
#define getcmd_cbeep()		getcmd_putch('\a')

#ifdef CONFIG_XPL_BUILD
#define HIST_MAX		3
#define HIST_SIZE		32
#инако
#define HIST_MAX		20
#define HIST_SIZE		CONFIG_SYS_CBSIZE
#endif

static цео hist_max;
static цео hist_add_idx;
static цео hist_cur = -1;
static unsigned hist_num;

#ifndef CONFIG_CMD_HISTORY_USE_CALLOC
static char hist_data[HIST_MAX][HIST_SIZE + 1];
#endif
static char *hist_list[HIST_MAX];

#define add_idx_minus_one() ((hist_add_idx == 0) ? hist_max : hist_add_idx-1)

static void getcmd_putchars(цео count, цео ch)
{
	цео i;

	за (i = 0; i < count; i++)
		getcmd_putch(ch);
}

static цео hist_init(void)
{
	цео i;

#ifndef CONFIG_CMD_HISTORY_USE_CALLOC
	за (i = 0; i < HIST_MAX; i++) {
		hist_list[i] = hist_data[i];
		hist_list[i][0] = '\0';
	}
#инако
	unsigned char *hist = calloc(HIST_MAX, HIST_SIZE + 1);
	ако (!hist)
		panic("%s: calloc: out of memory!\n", __func__);

	за (i = 0; i < HIST_MAX; i++)
		hist_list[i] = hist + (i * (HIST_SIZE + 1));
#endif

	hist_max = 0;
	hist_add_idx = 0;
	hist_cur = -1;
	hist_num = 0;

	врати 0;
}

static void cread_add_to_hist(char *line)
{
	strcpy(hist_list[hist_add_idx], line);

	ако (++hist_add_idx >= HIST_MAX)
		hist_add_idx = 0;

	ако (hist_add_idx > hist_max)
		hist_max = hist_add_idx;

	hist_num++;
}

static char *hist_prev(void)
{
	char *ret;
	цео old_cur;

	ако (hist_cur < 0)
		врати NULL;

	old_cur = hist_cur;
	ако (--hist_cur < 0)
		hist_cur = hist_max;

	ако (hist_cur == hist_add_idx) {
		hist_cur = old_cur;
		ret = NULL;
	} инако {
		ret = hist_list[hist_cur];
	}

	врати ret;
}

static char *hist_next(void)
{
	char *ret;

	ако (hist_cur < 0)
		врати NULL;

	ако (hist_cur == hist_add_idx)
		врати NULL;

	ако (++hist_cur > hist_max)
		hist_cur = 0;

	ако (hist_cur == hist_add_idx)
		ret = "";
	инако
		ret = hist_list[hist_cur];

	врати ret;
}

void cread_print_hist_list(void)
{
	цео i;
	uint n;

	n = hist_num - hist_max;

	i = hist_add_idx + 1;
	док (1) {
		ако (i > hist_max)
			i = 0;
		ако (i == hist_add_idx)
			break;
		printf("%s\n", hist_list[i]);
		n++;
		i++;
	}
}

#define BEGINNING_OF_LINE() {			\
	док (cls->num) {			\
		getcmd_putch(CTL_BACKSPACE);	\
		cls->num--;			\
	}					\
}

#define ERASE_TO_EOL() {				\
	ако (cls->num < cls->eol_num) {		\
		printf("%*s", (цео)(cls->eol_num - cls->num), ""); \
		do {					\
			getcmd_putch(CTL_BACKSPACE);	\
		} док (--cls->eol_num > cls->num);	\
	}						\
}

#define REFRESH_TO_EOL() {				\
	ако (cls->num < cls->eol_num) {			\
		uint wlen = cls->eol_num - cls->num;	\
		putnstr(buf + cls->num, wlen);		\
		cls->num = cls->eol_num;		\
	}						\
}

static void cread_add_char(char ichar, цео insert, uint *num,
			   uint *eol_num, char *buf, uint len)
{
	uint wlen;

	/* room ??? */
	ако (insert || *num == *eol_num) {
		ако (*eol_num > len - 1) {
			getcmd_cbeep();
			врати;
		}
		(*eol_num)++;
	}

	ако (insert) {
		wlen = *eol_num - *num;
		ако (wlen > 1)
			memmove(&buf[*num+1], &buf[*num], wlen-1);

		buf[*num] = ichar;
		putnstr(buf + *num, wlen);
		(*num)++;
		док (--wlen)
			getcmd_putch(CTL_BACKSPACE);
	} инако {
		/* echo the character */
		wlen = 1;
		buf[*num] = ichar;
		putnstr(buf + *num, wlen);
		(*num)++;
	}
}

static void cread_add_str(char *str, цео strsize, цео insert,
			  uint *num, uint *eol_num, char *buf, uint len)
{
	док (strsize--) {
		cread_add_char(*str, insert, num, eol_num, buf, len);
		str++;
	}
}

цео cread_line_process_ch(struct cli_line_state *cls, char ichar)
{
	char *buf = cls->buf;

	/* ichar=0x0 when error occurs in U-Boot getc */
	ако (!ichar)
		врати -EAGAIN;

	ако (ichar == '\n') {
		putc('\n');
		buf[cls->eol_num] = '\0';	/* terminate the string */
		врати 0;
	}

	switch (ichar) {
	case CTL_CH('a'):
		BEGINNING_OF_LINE();
		break;
	case CTL_CH('c'):	/* ^C - break */
		*buf = '\0';	/* discard input */
		врати -EINTR;
	case CTL_CH('f'):
		ако (cls->num < cls->eol_num) {
			getcmd_putch(buf[cls->num]);
			cls->num++;
		}
		break;
	case CTL_CH('b'):
		ако (cls->num) {
			getcmd_putch(CTL_BACKSPACE);
			cls->num--;
		}
		break;
	case CTL_CH('d'):
		ако (cls->num < cls->eol_num) {
			uint wlen;

			wlen = cls->eol_num - cls->num - 1;
			ако (wlen) {
				memmove(&buf[cls->num], &buf[cls->num + 1],
					wlen);
				putnstr(buf + cls->num, wlen);
			}

			getcmd_putch(' ');
			do {
				getcmd_putch(CTL_BACKSPACE);
			} док (wlen--);
			cls->eol_num--;
		}
		break;
	case CTL_CH('k'):
		ERASE_TO_EOL();
		break;
	case CTL_CH('e'):
		REFRESH_TO_EOL();
		break;
	case CTL_CH('o'):
		cls->insert = !cls->insert;
		break;
	case CTL_CH('w'):
		ако (cls->num) {
			uint base, wlen;

			за (base = cls->num;
			     base > 0 && buf[base - 1] == ' ';)
				base--;
			за (; base > 0 && buf[base - 1] != ' ';)
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
		break;
	case CTL_CH('x'):
	case CTL_CH('u'):
		BEGINNING_OF_LINE();
		ERASE_TO_EOL();
		break;
	case DEL:
	case DEL7:
	case 8:
		ако (cls->num) {
			uint wlen;

			wlen = cls->eol_num - cls->num;
			cls->num--;
			memmove(&buf[cls->num], &buf[cls->num + 1], wlen);
			getcmd_putch(CTL_BACKSPACE);
			putnstr(buf + cls->num, wlen);
			getcmd_putch(' ');
			do {
				getcmd_putch(CTL_BACKSPACE);
			} док (wlen--);
			cls->eol_num--;
		}
		break;
	case CTL_CH('p'):
	case CTL_CH('n'):
		ако (cls->history) {
			char *hline;

			ако (ichar == CTL_CH('p'))
				hline = hist_prev();
			инако
				hline = hist_next();

			ако (!hline) {
				getcmd_cbeep();
				break;
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
			break;
		}
		break;
	case '\t':
		ако (IS_ENABLED(CONFIG_AUTO_COMPLETE) && cls->cmd_complete) {
			цео num2, col;

			/* do not autocomplete when in the middle */
			ако (cls->num < cls->eol_num) {
				getcmd_cbeep();
				break;
			}

			buf[cls->num] = '\0';
			col = strlen(cls->prompt) + cls->eol_num;
			num2 = cls->num;
			ако (cmd_auto_complete(cls->prompt, buf, &num2, &col)) {
				col = num2 - cls->num;
				cls->num += col;
				cls->eol_num += col;
			}
			break;
		}
		fallthrough;
	default:
		cread_add_char(ichar, cls->insert, &cls->num, &cls->eol_num,
			       buf, cls->len);
		break;
	}

	/*
	 * keep the string terminated...if we added a char at the end then we
	 * want a \0 after it
	 */
	buf[cls->eol_num] = '\0';

	врати -EAGAIN;
}

void cli_cread_init(struct cli_line_state *cls, char *buf, uint buf_size)
{
	цео init_len = strlen(buf);

	memset(cls, '\0', sizeof(struct cli_line_state));
	cls->insert = true;
	cls->buf = buf;
	cls->len = buf_size;

	ако (init_len)
		cread_add_str(buf, init_len, 0, &cls->num, &cls->eol_num, buf,
			      buf_size);
}

static цео cread_line(const char *const prompt, char *buf, unsigned цео *len,
		      цео timeout)
{
	struct cli_ch_state s_cch, *cch = &s_cch;
	struct cli_line_state s_cls, *cls = &s_cls;
	char ichar;
	цео first = 1;

	cli_ch_init(cch);
	cli_cread_init(cls, buf, *len);
	cls->prompt = prompt;
	cls->history = true;
	cls->cmd_complete = true;

	док (1) {
		цео ret;

		/* Check for saved characters */
		ichar = cli_ch_process(cch, 0);

		ако (!ichar) {
			ако (bootretry_tstc_timeout())
				врати -2;	/* timed out */
			ако (first && timeout) {
				u64 etime = endtick(timeout);

				док (!tstc()) {	/* while no incoming data */
					ако (get_ticks() >= etime)
						врати -2;	/* timed out */
					schedule();
				}
				first = 0;
			}

			ichar = getcmd_getch();
			ichar = cli_ch_process(cch, ichar);
		}

		ret = cread_line_process_ch(cls, ichar);
		ако (ret == -EINTR)
			врати -1;
		инако ако (!ret)
			break;
	}
	*len = cls->eol_num;

	ако (buf[0] && buf[0] != CREAD_HIST_CHAR)
		cread_add_to_hist(buf);
	hist_cur = hist_add_idx;

	врати 0;
}

#инако /* !CONFIG_CMDLINE_EDITING */

static inline цео hist_init(void)
{
	врати 0;
}

static цео cread_line(const char *const prompt, char *buf, unsigned цео *len,
		      цео timeout)
{
	врати 0;
}

#endif /* CONFIG_CMDLINE_EDITING */

/****************************************************************************/

цео cli_readline(const char *const prompt)
{
	/*
	 * If console_buffer isn't 0-length the user will be prompted to modify
	 * it instead of entering it from scratch as desired.
	 */
	console_buffer[0] = '\0';

	врати cli_readline_into_buffer(prompt, console_buffer, 0);
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
static цео cread_line_simple(const char *const prompt, char *p)
{
	char *p_buf = p;
	цео n = 0;		/* buffer index */
	цео plen = 0;		/* prompt length */
	цео col;		/* output column cnt */
	цео c;

	/* print prompt */
	ако (prompt) {
		plen = strlen(prompt);
		puts(prompt);
	}
	col = plen;

	за (;;) {
		ако (bootretry_tstc_timeout())
			врати -2;	/* timed out */
		schedule();	/* Trigger watchdog, if needed */

		c = getchar();

		/*
		 * Special character handling
		 */
		switch (c) {
		case '\r':			/* Enter		*/
		case '\n':
			*p = '\0';
			puts("\r\n");
			врати p - p_buf;

		case '\0':			/* nul			*/
			continue;

		case 0x03:			/* ^C - break		*/
			p_buf[0] = '\0';	/* discard input */
			врати -1;

		case 0x15:			/* ^U - erase line	*/
			док (col > plen) {
				puts(erase_seq);
				--col;
			}
			p = p_buf;
			n = 0;
			continue;

		case 0x17:			/* ^W - erase word	*/
			p = delete_char(p_buf, p, &col, &n, plen);
			док ((n > 0) && (*p != ' '))
				p = delete_char(p_buf, p, &col, &n, plen);
			continue;

		case 0x08:			/* ^H  - backspace	*/
		case 0x7F:			/* DEL - backspace	*/
			p = delete_char(p_buf, p, &col, &n, plen);
			continue;

		default:
			/* Must be a normal character then */
			ако (n >= CONFIG_SYS_CBSIZE - 2) { /* Buffer full */
				putc('\a');
				break;
			}
			ако (c == '\t') {	/* expand TABs */
				ако (IS_ENABLED(CONFIG_AUTO_COMPLETE)) {
					/*
					 * if auto-completion triggered just
					 * continue
					 */
					*p = '\0';
					ако (cmd_auto_complete(prompt,
							      console_buffer,
							      &n, &col)) {
						p = p_buf + n;	/* reset */
						continue;
					}
				}
				puts(tab_seq + (col & 07));
				col += 8 - (col & 07);
			} инако {
				char __maybe_unused buf[2];

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
			break;
		}
	}
}

цео cli_readline_into_buffer(const char *const prompt, char *buffer,
			     цео timeout)
{
	char *p = buffer;
	uint len = CONFIG_SYS_CBSIZE;
	цео rc;
	static цео initted;

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
	ако (IS_ENABLED(CONFIG_CMDLINE_EDITING) && (gd->flags & GD_FLG_RELOC)) {
		ако (!initted) {
			rc = hist_init();
			ако (rc == 0)
				initted = 1;
		}

		ако (prompt)
			puts(prompt);

		rc = cread_line(prompt, p, &len, timeout);
		врати rc < 0 ? rc : len;

	} инако {
		врати cread_line_simple(prompt, p);
	}
}
