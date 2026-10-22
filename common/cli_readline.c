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

static char *delete_char (char *buffer, char *p, số_nguyên *colp, số_nguyên *np, số_nguyên plen)
{
	char *s;

	nếu (*np == 0)
		trả_về p;

	nếu (*(--p) == '\t') {		/* will retype the whole line */
		trong_khi (*colp > plen) {
			puts(erase_seq);
			(*colp)--;
		}
		cho (s = buffer; s < p; ++s) {
			nếu (*s == '\t') {
				puts(tab_seq + ((*colp) & 07));
				*colp += 8 - ((*colp) & 07);
			} không_thì {
				++(*colp);
				putc(*s);
			}
		}
	} không_thì {
		puts(erase_seq);
		(*colp)--;
	}
	(*np)--;

	trả_về p;
}

#ifdef CONFIG_CMDLINE_EDITING

/*
 * cmdline-editing related codes from vivi.
 * Author: Janghoon Lyu <nandy@mizi.com>
 */

#define putnstr(str, n)	printf("%.*s", (số_nguyên)n, str)

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
#không_thì
#define HIST_MAX		20
#define HIST_SIZE		CONFIG_SYS_CBSIZE
#endif

static số_nguyên hist_max;
static số_nguyên hist_add_idx;
static số_nguyên hist_cur = -1;
static unsigned hist_num;

#ifndef CONFIG_CMD_HISTORY_USE_CALLOC
static char hist_data[HIST_MAX][HIST_SIZE + 1];
#endif
static char *hist_list[HIST_MAX];

#define add_idx_minus_one() ((hist_add_idx == 0) ? hist_max : hist_add_idx-1)

static void getcmd_putchars(số_nguyên count, số_nguyên ch)
{
	số_nguyên i;

	cho (i = 0; i < count; i++)
		getcmd_putch(ch);
}

static số_nguyên hist_init(void)
{
	số_nguyên i;

#ifndef CONFIG_CMD_HISTORY_USE_CALLOC
	cho (i = 0; i < HIST_MAX; i++) {
		hist_list[i] = hist_data[i];
		hist_list[i][0] = '\0';
	}
#không_thì
	unsigned char *hist = calloc(HIST_MAX, HIST_SIZE + 1);
	nếu (!hist)
		panic("%s: calloc: out of memory!\n", __func__);

	cho (i = 0; i < HIST_MAX; i++)
		hist_list[i] = hist + (i * (HIST_SIZE + 1));
#endif

	hist_max = 0;
	hist_add_idx = 0;
	hist_cur = -1;
	hist_num = 0;

	trả_về 0;
}

static void cread_add_to_hist(char *line)
{
	strcpy(hist_list[hist_add_idx], line);

	nếu (++hist_add_idx >= HIST_MAX)
		hist_add_idx = 0;

	nếu (hist_add_idx > hist_max)
		hist_max = hist_add_idx;

	hist_num++;
}

static char *hist_prev(void)
{
	char *ret;
	số_nguyên old_cur;

	nếu (hist_cur < 0)
		trả_về NULL;

	old_cur = hist_cur;
	nếu (--hist_cur < 0)
		hist_cur = hist_max;

	nếu (hist_cur == hist_add_idx) {
		hist_cur = old_cur;
		ret = NULL;
	} không_thì {
		ret = hist_list[hist_cur];
	}

	trả_về ret;
}

static char *hist_next(void)
{
	char *ret;

	nếu (hist_cur < 0)
		trả_về NULL;

	nếu (hist_cur == hist_add_idx)
		trả_về NULL;

	nếu (++hist_cur > hist_max)
		hist_cur = 0;

	nếu (hist_cur == hist_add_idx)
		ret = "";
	không_thì
		ret = hist_list[hist_cur];

	trả_về ret;
}

void cread_print_hist_list(void)
{
	số_nguyên i;
	uint n;

	n = hist_num - hist_max;

	i = hist_add_idx + 1;
	trong_khi (1) {
		nếu (i > hist_max)
			i = 0;
		nếu (i == hist_add_idx)
			break;
		printf("%s\n", hist_list[i]);
		n++;
		i++;
	}
}

#define BEGINNING_OF_LINE() {			\
	trong_khi (cls->num) {			\
		getcmd_putch(CTL_BACKSPACE);	\
		cls->num--;			\
	}					\
}

#define ERASE_TO_EOL() {				\
	nếu (cls->num < cls->eol_num) {		\
		printf("%*s", (số_nguyên)(cls->eol_num - cls->num), ""); \
		do {					\
			getcmd_putch(CTL_BACKSPACE);	\
		} trong_khi (--cls->eol_num > cls->num);	\
	}						\
}

#define REFRESH_TO_EOL() {				\
	nếu (cls->num < cls->eol_num) {			\
		uint wlen = cls->eol_num - cls->num;	\
		putnstr(buf + cls->num, wlen);		\
		cls->num = cls->eol_num;		\
	}						\
}

static void cread_add_char(char ichar, số_nguyên insert, uint *num,
			   uint *eol_num, char *buf, uint len)
{
	uint wlen;

	/* room ??? */
	nếu (insert || *num == *eol_num) {
		nếu (*eol_num > len - 1) {
			getcmd_cbeep();
			trả_về;
		}
		(*eol_num)++;
	}

	nếu (insert) {
		wlen = *eol_num - *num;
		nếu (wlen > 1)
			memmove(&buf[*num+1], &buf[*num], wlen-1);

		buf[*num] = ichar;
		putnstr(buf + *num, wlen);
		(*num)++;
		trong_khi (--wlen)
			getcmd_putch(CTL_BACKSPACE);
	} không_thì {
		/* echo the character */
		wlen = 1;
		buf[*num] = ichar;
		putnstr(buf + *num, wlen);
		(*num)++;
	}
}

static void cread_add_str(char *str, số_nguyên strsize, số_nguyên insert,
			  uint *num, uint *eol_num, char *buf, uint len)
{
	trong_khi (strsize--) {
		cread_add_char(*str, insert, num, eol_num, buf, len);
		str++;
	}
}

số_nguyên cread_line_process_ch(struct cli_line_state *cls, char ichar)
{
	char *buf = cls->buf;

	/* ichar=0x0 when error occurs in U-Boot getc */
	nếu (!ichar)
		trả_về -EAGAIN;

	nếu (ichar == '\n') {
		putc('\n');
		buf[cls->eol_num] = '\0';	/* terminate the string */
		trả_về 0;
	}

	switch (ichar) {
	case CTL_CH('a'):
		BEGINNING_OF_LINE();
		break;
	case CTL_CH('c'):	/* ^C - break */
		*buf = '\0';	/* discard input */
		trả_về -EINTR;
	case CTL_CH('f'):
		nếu (cls->num < cls->eol_num) {
			getcmd_putch(buf[cls->num]);
			cls->num++;
		}
		break;
	case CTL_CH('b'):
		nếu (cls->num) {
			getcmd_putch(CTL_BACKSPACE);
			cls->num--;
		}
		break;
	case CTL_CH('d'):
		nếu (cls->num < cls->eol_num) {
			uint wlen;

			wlen = cls->eol_num - cls->num - 1;
			nếu (wlen) {
				memmove(&buf[cls->num], &buf[cls->num + 1],
					wlen);
				putnstr(buf + cls->num, wlen);
			}

			getcmd_putch(' ');
			do {
				getcmd_putch(CTL_BACKSPACE);
			} trong_khi (wlen--);
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
		nếu (cls->num) {
			uint base, wlen;

			cho (base = cls->num;
			     base > 0 && buf[base - 1] == ' ';)
				base--;
			cho (; base > 0 && buf[base - 1] != ' ';)
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
		nếu (cls->num) {
			uint wlen;

			wlen = cls->eol_num - cls->num;
			cls->num--;
			memmove(&buf[cls->num], &buf[cls->num + 1], wlen);
			getcmd_putch(CTL_BACKSPACE);
			putnstr(buf + cls->num, wlen);
			getcmd_putch(' ');
			do {
				getcmd_putch(CTL_BACKSPACE);
			} trong_khi (wlen--);
			cls->eol_num--;
		}
		break;
	case CTL_CH('p'):
	case CTL_CH('n'):
		nếu (cls->history) {
			char *hline;

			nếu (ichar == CTL_CH('p'))
				hline = hist_prev();
			không_thì
				hline = hist_next();

			nếu (!hline) {
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
		nếu (IS_ENABLED(CONFIG_AUTO_COMPLETE) && cls->cmd_complete) {
			số_nguyên num2, col;

			/* do not autocomplete when in the middle */
			nếu (cls->num < cls->eol_num) {
				getcmd_cbeep();
				break;
			}

			buf[cls->num] = '\0';
			col = strlen(cls->prompt) + cls->eol_num;
			num2 = cls->num;
			nếu (cmd_auto_complete(cls->prompt, buf, &num2, &col)) {
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

	trả_về -EAGAIN;
}

void cli_cread_init(struct cli_line_state *cls, char *buf, uint buf_size)
{
	số_nguyên init_len = strlen(buf);

	memset(cls, '\0', sizeof(struct cli_line_state));
	cls->insert = true;
	cls->buf = buf;
	cls->len = buf_size;

	nếu (init_len)
		cread_add_str(buf, init_len, 0, &cls->num, &cls->eol_num, buf,
			      buf_size);
}

static số_nguyên cread_line(const char *const prompt, char *buf, unsigned số_nguyên *len,
		      số_nguyên timeout)
{
	struct cli_ch_state s_cch, *cch = &s_cch;
	struct cli_line_state s_cls, *cls = &s_cls;
	char ichar;
	số_nguyên first = 1;

	cli_ch_init(cch);
	cli_cread_init(cls, buf, *len);
	cls->prompt = prompt;
	cls->history = true;
	cls->cmd_complete = true;

	trong_khi (1) {
		số_nguyên ret;

		/* Check for saved characters */
		ichar = cli_ch_process(cch, 0);

		nếu (!ichar) {
			nếu (bootretry_tstc_timeout())
				trả_về -2;	/* timed out */
			nếu (first && timeout) {
				u64 etime = endtick(timeout);

				trong_khi (!tstc()) {	/* while no incoming data */
					nếu (get_ticks() >= etime)
						trả_về -2;	/* timed out */
					schedule();
				}
				first = 0;
			}

			ichar = getcmd_getch();
			ichar = cli_ch_process(cch, ichar);
		}

		ret = cread_line_process_ch(cls, ichar);
		nếu (ret == -EINTR)
			trả_về -1;
		không_thì nếu (!ret)
			break;
	}
	*len = cls->eol_num;

	nếu (buf[0] && buf[0] != CREAD_HIST_CHAR)
		cread_add_to_hist(buf);
	hist_cur = hist_add_idx;

	trả_về 0;
}

#không_thì /* !CONFIG_CMDLINE_EDITING */

static inline số_nguyên hist_init(void)
{
	trả_về 0;
}

static số_nguyên cread_line(const char *const prompt, char *buf, unsigned số_nguyên *len,
		      số_nguyên timeout)
{
	trả_về 0;
}

#endif /* CONFIG_CMDLINE_EDITING */

/****************************************************************************/

số_nguyên cli_readline(const char *const prompt)
{
	/*
	 * If console_buffer isn't 0-length the user will be prompted to modify
	 * it instead of entering it from scratch as desired.
	 */
	console_buffer[0] = '\0';

	trả_về cli_readline_into_buffer(prompt, console_buffer, 0);
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
static số_nguyên cread_line_simple(const char *const prompt, char *p)
{
	char *p_buf = p;
	số_nguyên n = 0;		/* buffer index */
	số_nguyên plen = 0;		/* prompt length */
	số_nguyên col;		/* output column cnt */
	số_nguyên c;

	/* print prompt */
	nếu (prompt) {
		plen = strlen(prompt);
		puts(prompt);
	}
	col = plen;

	cho (;;) {
		nếu (bootretry_tstc_timeout())
			trả_về -2;	/* timed out */
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
			trả_về p - p_buf;

		case '\0':			/* nul			*/
			continue;

		case 0x03:			/* ^C - break		*/
			p_buf[0] = '\0';	/* discard input */
			trả_về -1;

		case 0x15:			/* ^U - erase line	*/
			trong_khi (col > plen) {
				puts(erase_seq);
				--col;
			}
			p = p_buf;
			n = 0;
			continue;

		case 0x17:			/* ^W - erase word	*/
			p = delete_char(p_buf, p, &col, &n, plen);
			trong_khi ((n > 0) && (*p != ' '))
				p = delete_char(p_buf, p, &col, &n, plen);
			continue;

		case 0x08:			/* ^H  - backspace	*/
		case 0x7F:			/* DEL - backspace	*/
			p = delete_char(p_buf, p, &col, &n, plen);
			continue;

		default:
			/* Must be a normal character then */
			nếu (n >= CONFIG_SYS_CBSIZE - 2) { /* Buffer full */
				putc('\a');
				break;
			}
			nếu (c == '\t') {	/* expand TABs */
				nếu (IS_ENABLED(CONFIG_AUTO_COMPLETE)) {
					/*
					 * if auto-completion triggered just
					 * continue
					 */
					*p = '\0';
					nếu (cmd_auto_complete(prompt,
							      console_buffer,
							      &n, &col)) {
						p = p_buf + n;	/* reset */
						continue;
					}
				}
				puts(tab_seq + (col & 07));
				col += 8 - (col & 07);
			} không_thì {
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

số_nguyên cli_readline_into_buffer(const char *const prompt, char *buffer,
			     số_nguyên timeout)
{
	char *p = buffer;
	uint len = CONFIG_SYS_CBSIZE;
	số_nguyên rc;
	static số_nguyên initted;

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
	nếu (IS_ENABLED(CONFIG_CMDLINE_EDITING) && (gd->flags & GD_FLG_RELOC)) {
		nếu (!initted) {
			rc = hist_init();
			nếu (rc == 0)
				initted = 1;
		}

		nếu (prompt)
			puts(prompt);

		rc = cread_line(prompt, p, &len, timeout);
		trả_về rc < 0 ? rc : len;

	} không_thì {
		trả_về cread_line_simple(prompt, p);
	}
}
