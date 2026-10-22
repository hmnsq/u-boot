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

static char *delete_char (char *buffer, char *p, 整数 *colp, 整数 *np, 整数 plen)
{
	char *s;

	如果 (*np == 0)
		返回 p;

	如果 (*(--p) == '\t') {		/* will retype the whole line */
		当 (*colp > plen) {
			puts(erase_seq);
			(*colp)--;
		}
		为 (s = buffer; s < p; ++s) {
			如果 (*s == '\t') {
				puts(tab_seq + ((*colp) & 07));
				*colp += 8 - ((*colp) & 07);
			} 否则 {
				++(*colp);
				putc(*s);
			}
		}
	} 否则 {
		puts(erase_seq);
		(*colp)--;
	}
	(*np)--;

	返回 p;
}

#ifdef CONFIG_CMDLINE_EDITING

/*
 * cmdline-editing related codes from vivi.
 * Author: Janghoon Lyu <nandy@mizi.com>
 */

#define putnstr(str, n)	printf("%.*s", (整数)n, str)

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
#否则
#define HIST_MAX		20
#define HIST_SIZE		CONFIG_SYS_CBSIZE
#endif

static 整数 hist_max;
static 整数 hist_add_idx;
static 整数 hist_cur = -1;
static unsigned hist_num;

#ifndef CONFIG_CMD_HISTORY_USE_CALLOC
static char hist_data[HIST_MAX][HIST_SIZE + 1];
#endif
static char *hist_list[HIST_MAX];

#define add_idx_minus_one() ((hist_add_idx == 0) ? hist_max : hist_add_idx-1)

static void getcmd_putchars(整数 count, 整数 ch)
{
	整数 i;

	为 (i = 0; i < count; i++)
		getcmd_putch(ch);
}

static 整数 hist_init(void)
{
	整数 i;

#ifndef CONFIG_CMD_HISTORY_USE_CALLOC
	为 (i = 0; i < HIST_MAX; i++) {
		hist_list[i] = hist_data[i];
		hist_list[i][0] = '\0';
	}
#否则
	unsigned char *hist = calloc(HIST_MAX, HIST_SIZE + 1);
	如果 (!hist)
		panic("%s: calloc: out of memory!\n", __func__);

	为 (i = 0; i < HIST_MAX; i++)
		hist_list[i] = hist + (i * (HIST_SIZE + 1));
#endif

	hist_max = 0;
	hist_add_idx = 0;
	hist_cur = -1;
	hist_num = 0;

	返回 0;
}

static void cread_add_to_hist(char *line)
{
	strcpy(hist_list[hist_add_idx], line);

	如果 (++hist_add_idx >= HIST_MAX)
		hist_add_idx = 0;

	如果 (hist_add_idx > hist_max)
		hist_max = hist_add_idx;

	hist_num++;
}

static char *hist_prev(void)
{
	char *ret;
	整数 old_cur;

	如果 (hist_cur < 0)
		返回 NULL;

	old_cur = hist_cur;
	如果 (--hist_cur < 0)
		hist_cur = hist_max;

	如果 (hist_cur == hist_add_idx) {
		hist_cur = old_cur;
		ret = NULL;
	} 否则 {
		ret = hist_list[hist_cur];
	}

	返回 ret;
}

static char *hist_next(void)
{
	char *ret;

	如果 (hist_cur < 0)
		返回 NULL;

	如果 (hist_cur == hist_add_idx)
		返回 NULL;

	如果 (++hist_cur > hist_max)
		hist_cur = 0;

	如果 (hist_cur == hist_add_idx)
		ret = "";
	否则
		ret = hist_list[hist_cur];

	返回 ret;
}

void cread_print_hist_list(void)
{
	整数 i;
	uint n;

	n = hist_num - hist_max;

	i = hist_add_idx + 1;
	当 (1) {
		如果 (i > hist_max)
			i = 0;
		如果 (i == hist_add_idx)
			break;
		printf("%s\n", hist_list[i]);
		n++;
		i++;
	}
}

#define BEGINNING_OF_LINE() {			\
	当 (cls->num) {			\
		getcmd_putch(CTL_BACKSPACE);	\
		cls->num--;			\
	}					\
}

#define ERASE_TO_EOL() {				\
	如果 (cls->num < cls->eol_num) {		\
		printf("%*s", (整数)(cls->eol_num - cls->num), ""); \
		do {					\
			getcmd_putch(CTL_BACKSPACE);	\
		} 当 (--cls->eol_num > cls->num);	\
	}						\
}

#define REFRESH_TO_EOL() {				\
	如果 (cls->num < cls->eol_num) {			\
		uint wlen = cls->eol_num - cls->num;	\
		putnstr(buf + cls->num, wlen);		\
		cls->num = cls->eol_num;		\
	}						\
}

static void cread_add_char(char ichar, 整数 insert, uint *num,
			   uint *eol_num, char *buf, uint len)
{
	uint wlen;

	/* room ??? */
	如果 (insert || *num == *eol_num) {
		如果 (*eol_num > len - 1) {
			getcmd_cbeep();
			返回;
		}
		(*eol_num)++;
	}

	如果 (insert) {
		wlen = *eol_num - *num;
		如果 (wlen > 1)
			memmove(&buf[*num+1], &buf[*num], wlen-1);

		buf[*num] = ichar;
		putnstr(buf + *num, wlen);
		(*num)++;
		当 (--wlen)
			getcmd_putch(CTL_BACKSPACE);
	} 否则 {
		/* echo the character */
		wlen = 1;
		buf[*num] = ichar;
		putnstr(buf + *num, wlen);
		(*num)++;
	}
}

static void cread_add_str(char *str, 整数 strsize, 整数 insert,
			  uint *num, uint *eol_num, char *buf, uint len)
{
	当 (strsize--) {
		cread_add_char(*str, insert, num, eol_num, buf, len);
		str++;
	}
}

整数 cread_line_process_ch(struct cli_line_state *cls, char ichar)
{
	char *buf = cls->buf;

	/* ichar=0x0 when error occurs in U-Boot getc */
	如果 (!ichar)
		返回 -EAGAIN;

	如果 (ichar == '\n') {
		putc('\n');
		buf[cls->eol_num] = '\0';	/* terminate the string */
		返回 0;
	}

	switch (ichar) {
	case CTL_CH('a'):
		BEGINNING_OF_LINE();
		break;
	case CTL_CH('c'):	/* ^C - break */
		*buf = '\0';	/* discard input */
		返回 -EINTR;
	case CTL_CH('f'):
		如果 (cls->num < cls->eol_num) {
			getcmd_putch(buf[cls->num]);
			cls->num++;
		}
		break;
	case CTL_CH('b'):
		如果 (cls->num) {
			getcmd_putch(CTL_BACKSPACE);
			cls->num--;
		}
		break;
	case CTL_CH('d'):
		如果 (cls->num < cls->eol_num) {
			uint wlen;

			wlen = cls->eol_num - cls->num - 1;
			如果 (wlen) {
				memmove(&buf[cls->num], &buf[cls->num + 1],
					wlen);
				putnstr(buf + cls->num, wlen);
			}

			getcmd_putch(' ');
			do {
				getcmd_putch(CTL_BACKSPACE);
			} 当 (wlen--);
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
		如果 (cls->num) {
			uint base, wlen;

			为 (base = cls->num;
			     base > 0 && buf[base - 1] == ' ';)
				base--;
			为 (; base > 0 && buf[base - 1] != ' ';)
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
		如果 (cls->num) {
			uint wlen;

			wlen = cls->eol_num - cls->num;
			cls->num--;
			memmove(&buf[cls->num], &buf[cls->num + 1], wlen);
			getcmd_putch(CTL_BACKSPACE);
			putnstr(buf + cls->num, wlen);
			getcmd_putch(' ');
			do {
				getcmd_putch(CTL_BACKSPACE);
			} 当 (wlen--);
			cls->eol_num--;
		}
		break;
	case CTL_CH('p'):
	case CTL_CH('n'):
		如果 (cls->history) {
			char *hline;

			如果 (ichar == CTL_CH('p'))
				hline = hist_prev();
			否则
				hline = hist_next();

			如果 (!hline) {
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
		如果 (IS_ENABLED(CONFIG_AUTO_COMPLETE) && cls->cmd_complete) {
			整数 num2, col;

			/* do not autocomplete when in the middle */
			如果 (cls->num < cls->eol_num) {
				getcmd_cbeep();
				break;
			}

			buf[cls->num] = '\0';
			col = strlen(cls->prompt) + cls->eol_num;
			num2 = cls->num;
			如果 (cmd_auto_complete(cls->prompt, buf, &num2, &col)) {
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

	返回 -EAGAIN;
}

void cli_cread_init(struct cli_line_state *cls, char *buf, uint buf_size)
{
	整数 init_len = strlen(buf);

	memset(cls, '\0', sizeof(struct cli_line_state));
	cls->insert = true;
	cls->buf = buf;
	cls->len = buf_size;

	如果 (init_len)
		cread_add_str(buf, init_len, 0, &cls->num, &cls->eol_num, buf,
			      buf_size);
}

static 整数 cread_line(const char *const prompt, char *buf, unsigned 整数 *len,
		      整数 timeout)
{
	struct cli_ch_state s_cch, *cch = &s_cch;
	struct cli_line_state s_cls, *cls = &s_cls;
	char ichar;
	整数 first = 1;

	cli_ch_init(cch);
	cli_cread_init(cls, buf, *len);
	cls->prompt = prompt;
	cls->history = true;
	cls->cmd_complete = true;

	当 (1) {
		整数 ret;

		/* Check for saved characters */
		ichar = cli_ch_process(cch, 0);

		如果 (!ichar) {
			如果 (bootretry_tstc_timeout())
				返回 -2;	/* timed out */
			如果 (first && timeout) {
				u64 etime = endtick(timeout);

				当 (!tstc()) {	/* while no incoming data */
					如果 (get_ticks() >= etime)
						返回 -2;	/* timed out */
					schedule();
				}
				first = 0;
			}

			ichar = getcmd_getch();
			ichar = cli_ch_process(cch, ichar);
		}

		ret = cread_line_process_ch(cls, ichar);
		如果 (ret == -EINTR)
			返回 -1;
		否则 如果 (!ret)
			break;
	}
	*len = cls->eol_num;

	如果 (buf[0] && buf[0] != CREAD_HIST_CHAR)
		cread_add_to_hist(buf);
	hist_cur = hist_add_idx;

	返回 0;
}

#否则 /* !CONFIG_CMDLINE_EDITING */

static inline 整数 hist_init(void)
{
	返回 0;
}

static 整数 cread_line(const char *const prompt, char *buf, unsigned 整数 *len,
		      整数 timeout)
{
	返回 0;
}

#endif /* CONFIG_CMDLINE_EDITING */

/****************************************************************************/

整数 cli_readline(const char *const prompt)
{
	/*
	 * If console_buffer isn't 0-length the user will be prompted to modify
	 * it instead of entering it from scratch as desired.
	 */
	console_buffer[0] = '\0';

	返回 cli_readline_into_buffer(prompt, console_buffer, 0);
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
static 整数 cread_line_simple(const char *const prompt, char *p)
{
	char *p_buf = p;
	整数 n = 0;		/* buffer index */
	整数 plen = 0;		/* prompt length */
	整数 col;		/* output column cnt */
	整数 c;

	/* print prompt */
	如果 (prompt) {
		plen = strlen(prompt);
		puts(prompt);
	}
	col = plen;

	为 (;;) {
		如果 (bootretry_tstc_timeout())
			返回 -2;	/* timed out */
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
			返回 p - p_buf;

		case '\0':			/* nul			*/
			continue;

		case 0x03:			/* ^C - break		*/
			p_buf[0] = '\0';	/* discard input */
			返回 -1;

		case 0x15:			/* ^U - erase line	*/
			当 (col > plen) {
				puts(erase_seq);
				--col;
			}
			p = p_buf;
			n = 0;
			continue;

		case 0x17:			/* ^W - erase word	*/
			p = delete_char(p_buf, p, &col, &n, plen);
			当 ((n > 0) && (*p != ' '))
				p = delete_char(p_buf, p, &col, &n, plen);
			continue;

		case 0x08:			/* ^H  - backspace	*/
		case 0x7F:			/* DEL - backspace	*/
			p = delete_char(p_buf, p, &col, &n, plen);
			continue;

		default:
			/* Must be a normal character then */
			如果 (n >= CONFIG_SYS_CBSIZE - 2) { /* Buffer full */
				putc('\a');
				break;
			}
			如果 (c == '\t') {	/* expand TABs */
				如果 (IS_ENABLED(CONFIG_AUTO_COMPLETE)) {
					/*
					 * if auto-completion triggered just
					 * continue
					 */
					*p = '\0';
					如果 (cmd_auto_complete(prompt,
							      console_buffer,
							      &n, &col)) {
						p = p_buf + n;	/* reset */
						continue;
					}
				}
				puts(tab_seq + (col & 07));
				col += 8 - (col & 07);
			} 否则 {
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

整数 cli_readline_into_buffer(const char *const prompt, char *buffer,
			     整数 timeout)
{
	char *p = buffer;
	uint len = CONFIG_SYS_CBSIZE;
	整数 rc;
	static 整数 initted;

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
	如果 (IS_ENABLED(CONFIG_CMDLINE_EDITING) && (gd->flags & GD_FLG_RELOC)) {
		如果 (!initted) {
			rc = hist_init();
			如果 (rc == 0)
				initted = 1;
		}

		如果 (prompt)
			puts(prompt);

		rc = cread_line(prompt, p, &len, timeout);
		返回 rc < 0 ? rc : len;

	} 否则 {
		返回 cread_line_simple(prompt, p);
	}
}
