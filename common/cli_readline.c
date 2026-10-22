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

static char *delete_char (char *buffer, char *p, int *colp, int *np, int plen)
{
	char *s;

	if (*np == 0)
		return p;

	if (*(--p) == '\t') {		/* will retype the whole line */
		while (*colp > plen) {
			puts(erase_seq);
			(*colp)--;
		}
		for (s = buffer; s < p; ++s) {
			if (*s == '\t') {
				puts(tab_seq + ((*colp) & 07));
				*colp += 8 - ((*colp) & 07);
			} else {
				++(*colp);
				putc(*s);
			}
		}
	} else {
		puts(erase_seq);
		(*colp)--;
	}
	(*np)--;

	return p;
}

#ifdef CONFIG_CMDLINE_EDITING

/*
 * cmdline-editing related codes from vivi.
 * Author: Janghoon Lyu <nandy@mizi.com>
 */

#define putnstr(str, n)	printf("%.*s", (int)n, str)

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
#else
#define HIST_MAX		20
#define HIST_SIZE		CONFIG_SYS_CBSIZE
#endif

static int hist_max;
static int hist_add_idx;
static int hist_cur = -1;
static unsigned hist_नुमेरो;

#ifndef CONFIG_CMD_HISTORY_USE_CALLOC
static char hist_data[HIST_MAX][HIST_SIZE + 1];
#endif
static char *hist_list[HIST_MAX];

#define add_idx_minus_one() ((hist_add_idx == 0) ? hist_max : hist_add_idx-1)

static void getcmd_putchars(int बिलांग, int ch)
{
	int i;

	for (i = 0; i < बिलांग; i++)
		getcmd_putch(ch);
}

static int hist_init(void)
{
	int i;

#ifndef CONFIG_CMD_HISTORY_USE_CALLOC
	for (i = 0; i < HIST_MAX; i++) {
		hist_list[i] = hist_data[i];
		hist_list[i][0] = '\0';
	}
#else
	unsigned char *hist = calloc(HIST_MAX, HIST_SIZE + 1);
	if (!hist)
		panic("%s: calloc: out of memory!\n", __func__);

	for (i = 0; i < HIST_MAX; i++)
		hist_list[i] = hist + (i * (HIST_SIZE + 1));
#endif

	hist_max = 0;
	hist_add_idx = 0;
	hist_cur = -1;
	hist_नुमेरो = 0;

	return 0;
}

static void cread_add_to_hist(char *लिन्या)
{
	strcpy(hist_list[hist_add_idx], लिन्या);

	if (++hist_add_idx >= HIST_MAX)
		hist_add_idx = 0;

	if (hist_add_idx > hist_max)
		hist_max = hist_add_idx;

	hist_नुमेरो++;
}

static char *hist_prev(void)
{
	char *ret;
	int old_cur;

	if (hist_cur < 0)
		return NULL;

	old_cur = hist_cur;
	if (--hist_cur < 0)
		hist_cur = hist_max;

	if (hist_cur == hist_add_idx) {
		hist_cur = old_cur;
		ret = NULL;
	} else {
		ret = hist_list[hist_cur];
	}

	return ret;
}

static char *hist_next(void)
{
	char *ret;

	if (hist_cur < 0)
		return NULL;

	if (hist_cur == hist_add_idx)
		return NULL;

	if (++hist_cur > hist_max)
		hist_cur = 0;

	if (hist_cur == hist_add_idx)
		ret = "";
	else
		ret = hist_list[hist_cur];

	return ret;
}

void cread_इलिम्बाग_hist_list(void)
{
	int i;
	uint n;

	n = hist_नुमेरो - hist_max;

	i = hist_add_idx + 1;
	while (1) {
		if (i > hist_max)
			i = 0;
		if (i == hist_add_idx)
			break;
		printf("%s\n", hist_list[i]);
		n++;
		i++;
	}
}

#define BEGINNING_OF_लिन्या() {			\
	while (cls->नुमेरो) {			\
		getcmd_putch(CTL_BACKSPACE);	\
		cls->नुमेरो--;			\
	}					\
}

#define ERASE_TO_EOL() {				\
	if (cls->नुमेरो < cls->eol_नुमेरो) {		\
		printf("%*s", (int)(cls->eol_नुमेरो - cls->नुमेरो), ""); \
		do {					\
			getcmd_putch(CTL_BACKSPACE);	\
		} while (--cls->eol_नुमेरो > cls->नुमेरो);	\
	}						\
}

#define REFRESH_TO_EOL() {				\
	if (cls->नुमेरो < cls->eol_नुमेरो) {			\
		uint wlen = cls->eol_नुमेरो - cls->नुमेरो;	\
		putnstr(buf + cls->नुमेरो, wlen);		\
		cls->नुमेरो = cls->eol_नुमेरो;		\
	}						\
}

static void cread_add_char(char ichar, int इसिंगित, uint *नुमेरो,
			   uint *eol_नुमेरो, char *buf, uint len)
{
	uint wlen;

	/* room ??? */
	if (इसिंगित || *नुमेरो == *eol_नुमेरो) {
		if (*eol_नुमेरो > len - 1) {
			getcmd_cbeep();
			return;
		}
		(*eol_नुमेरो)++;
	}

	if (इसिंगित) {
		wlen = *eol_नुमेरो - *नुमेरो;
		if (wlen > 1)
			memmove(&buf[*नुमेरो+1], &buf[*नुमेरो], wlen-1);

		buf[*नुमेरो] = ichar;
		putnstr(buf + *नुमेरो, wlen);
		(*नुमेरो)++;
		while (--wlen)
			getcmd_putch(CTL_BACKSPACE);
	} else {
		/* echo the character */
		wlen = 1;
		buf[*नुमेरो] = ichar;
		putnstr(buf + *नुमेरो, wlen);
		(*नुमेरो)++;
	}
}

static void cread_add_str(char *str, int strsize, int इसिंगित,
			  uint *नुमेरो, uint *eol_नुमेरो, char *buf, uint len)
{
	while (strsize--) {
		cread_add_char(*str, इसिंगित, नुमेरो, eol_नुमेरो, buf, len);
		str++;
	}
}

int cread_लिन्या_process_ch(struct cli_लिन्या_state *cls, char ichar)
{
	char *buf = cls->buf;

	/* ichar=0x0 when error occurs in U-Boot getc */
	if (!ichar)
		return -EAGAIN;

	if (ichar == '\n') {
		putc('\n');
		buf[cls->eol_नुमेरो] = '\0';	/* terminate the string */
		return 0;
	}

	switch (ichar) {
	case CTL_CH('a'):
		BEGINNING_OF_लिन्या();
		break;
	case CTL_CH('c'):	/* ^C - break */
		*buf = '\0';	/* discard input */
		return -EINTR;
	case CTL_CH('f'):
		if (cls->नुमेरो < cls->eol_नुमेरो) {
			getcmd_putch(buf[cls->नुमेरो]);
			cls->नुमेरो++;
		}
		break;
	case CTL_CH('b'):
		if (cls->नुमेरो) {
			getcmd_putch(CTL_BACKSPACE);
			cls->नुमेरो--;
		}
		break;
	case CTL_CH('d'):
		if (cls->नुमेरो < cls->eol_नुमेरो) {
			uint wlen;

			wlen = cls->eol_नुमेरो - cls->नुमेरो - 1;
			if (wlen) {
				memmove(&buf[cls->नुमेरो], &buf[cls->नुमेरो + 1],
					wlen);
				putnstr(buf + cls->नुमेरो, wlen);
			}

			getcmd_putch(' ');
			do {
				getcmd_putch(CTL_BACKSPACE);
			} while (wlen--);
			cls->eol_नुमेरो--;
		}
		break;
	case CTL_CH('k'):
		ERASE_TO_EOL();
		break;
	case CTL_CH('e'):
		REFRESH_TO_EOL();
		break;
	case CTL_CH('o'):
		cls->इसिंगित = !cls->इसिंगित;
		break;
	case CTL_CH('w'):
		if (cls->नुमेरो) {
			uint base, wlen;

			for (base = cls->नुमेरो;
			     base > 0 && buf[base - 1] == ' ';)
				base--;
			for (; base > 0 && buf[base - 1] != ' ';)
				base--;

			/* now delete chars from base to cls->num */
			wlen = cls->नुमेरो - base;
			cls->eol_नुमेरो -= wlen;
			memmove(&buf[base], &buf[cls->नुमेरो],
				cls->eol_नुमेरो - base + 1);
			cls->नुमेरो = base;
			getcmd_putchars(wlen, CTL_BACKSPACE);
			puts(buf + base);
			getcmd_putchars(wlen, ' ');
			getcmd_putchars(wlen + cls->eol_नुमेरो - cls->नुमेरो,
					CTL_BACKSPACE);
		}
		break;
	case CTL_CH('x'):
	case CTL_CH('u'):
		BEGINNING_OF_लिन्या();
		ERASE_TO_EOL();
		break;
	case DEL:
	case DEL7:
	case 8:
		if (cls->नुमेरो) {
			uint wlen;

			wlen = cls->eol_नुमेरो - cls->नुमेरो;
			cls->नुमेरो--;
			memmove(&buf[cls->नुमेरो], &buf[cls->नुमेरो + 1], wlen);
			getcmd_putch(CTL_BACKSPACE);
			putnstr(buf + cls->नुमेरो, wlen);
			getcmd_putch(' ');
			do {
				getcmd_putch(CTL_BACKSPACE);
			} while (wlen--);
			cls->eol_नुमेरो--;
		}
		break;
	case CTL_CH('p'):
	case CTL_CH('n'):
		if (cls->history) {
			char *hline;

			if (ichar == CTL_CH('p'))
				hline = hist_prev();
			else
				hline = hist_next();

			if (!hline) {
				getcmd_cbeep();
				break;
			}

			/* nuke the current line */
			/* first, go home */
			BEGINNING_OF_लिन्या();

			/* erase to end of line */
			ERASE_TO_EOL();

			/* copy new line into place and display */
			strcpy(buf, hline);
			cls->eol_नुमेरो = strlen(buf);
			REFRESH_TO_EOL();
			break;
		}
		break;
	case '\t':
		if (IS_ENABLED(CONFIG_AUTO_COMPLETE) && cls->cmd_complete) {
			int num2, col;

			/* do not autocomplete when in the middle */
			if (cls->नुमेरो < cls->eol_नुमेरो) {
				getcmd_cbeep();
				break;
			}

			buf[cls->नुमेरो] = '\0';
			col = strlen(cls->prompt) + cls->eol_नुमेरो;
			num2 = cls->नुमेरो;
			if (cmd_auto_complete(cls->prompt, buf, &num2, &col)) {
				col = num2 - cls->नुमेरो;
				cls->नुमेरो += col;
				cls->eol_नुमेरो += col;
			}
			break;
		}
		fallthrough;
	default:
		cread_add_char(ichar, cls->इसिंगित, &cls->नुमेरो, &cls->eol_नुमेरो,
			       buf, cls->len);
		break;
	}

	/*
	 * keep the string terminated...if we added a char at the end then we
	 * want a \0 after it
	 */
	buf[cls->eol_नुमेरो] = '\0';

	return -EAGAIN;
}

void cli_cread_init(struct cli_लिन्या_state *cls, char *buf, uint buf_size)
{
	int init_len = strlen(buf);

	memset(cls, '\0', sizeof(struct cli_लिन्या_state));
	cls->इसिंगित = true;
	cls->buf = buf;
	cls->len = buf_size;

	if (init_len)
		cread_add_str(buf, init_len, 0, &cls->नुमेरो, &cls->eol_नुमेरो, buf,
			      buf_size);
}

static int cread_लिन्या(const char *const prompt, char *buf, unsigned int *len,
		      int timeout)
{
	struct cli_ch_state s_cch, *cch = &s_cch;
	struct cli_लिन्या_state s_cls, *cls = &s_cls;
	char ichar;
	int first = 1;

	cli_ch_init(cch);
	cli_cread_init(cls, buf, *len);
	cls->prompt = prompt;
	cls->history = true;
	cls->cmd_complete = true;

	while (1) {
		int ret;

		/* Check for saved characters */
		ichar = cli_ch_process(cch, 0);

		if (!ichar) {
			if (bootretry_tstc_timeout())
				return -2;	/* timed out */
			if (first && timeout) {
				u64 etime = endtick(timeout);

				while (!tstc()) {	/* while no incoming data */
					if (get_ticks() >= etime)
						return -2;	/* timed out */
					schedule();
				}
				first = 0;
			}

			ichar = getcmd_getch();
			ichar = cli_ch_process(cch, ichar);
		}

		ret = cread_लिन्या_process_ch(cls, ichar);
		if (ret == -EINTR)
			return -1;
		else if (!ret)
			break;
	}
	*len = cls->eol_नुमेरो;

	if (buf[0] && buf[0] != CREAD_HIST_CHAR)
		cread_add_to_hist(buf);
	hist_cur = hist_add_idx;

	return 0;
}

#else /* !CONFIG_CMDLINE_EDITING */

static inline int hist_init(void)
{
	return 0;
}

static int cread_लिन्या(const char *const prompt, char *buf, unsigned int *len,
		      int timeout)
{
	return 0;
}

#endif /* CONFIG_CMDLINE_EDITING */

/****************************************************************************/

int cli_readline(const char *const prompt)
{
	/*
	 * If console_buffer isn't 0-length the user will be prompted to modify
	 * it instead of entering it from scratch as desired.
	 */
	console_buffer[0] = '\0';

	return cli_readline_into_buffer(prompt, console_buffer, 0);
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
static int cread_लिन्या_simple(const char *const prompt, char *p)
{
	char *p_buf = p;
	int n = 0;		/* buffer index */
	int plen = 0;		/* prompt length */
	int col;		/* output column cnt */
	int c;

	/* print prompt */
	if (prompt) {
		plen = strlen(prompt);
		puts(prompt);
	}
	col = plen;

	for (;;) {
		if (bootretry_tstc_timeout())
			return -2;	/* timed out */
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
			return p - p_buf;

		case '\0':			/* nul			*/
			continue;

		case 0x03:			/* ^C - break		*/
			p_buf[0] = '\0';	/* discard input */
			return -1;

		case 0x15:			/* ^U - erase line	*/
			while (col > plen) {
				puts(erase_seq);
				--col;
			}
			p = p_buf;
			n = 0;
			continue;

		case 0x17:			/* ^W - erase word	*/
			p = delete_char(p_buf, p, &col, &n, plen);
			while ((n > 0) && (*p != ' '))
				p = delete_char(p_buf, p, &col, &n, plen);
			continue;

		case 0x08:			/* ^H  - backspace	*/
		case 0x7F:			/* DEL - backspace	*/
			p = delete_char(p_buf, p, &col, &n, plen);
			continue;

		default:
			/* Must be a normal character then */
			if (n >= CONFIG_SYS_CBSIZE - 2) { /* Buffer full */
				putc('\a');
				break;
			}
			if (c == '\t') {	/* expand TABs */
				if (IS_ENABLED(CONFIG_AUTO_COMPLETE)) {
					/*
					 * if auto-completion triggered just
					 * continue
					 */
					*p = '\0';
					if (cmd_auto_complete(prompt,
							      console_buffer,
							      &n, &col)) {
						p = p_buf + n;	/* reset */
						continue;
					}
				}
				puts(tab_seq + (col & 07));
				col += 8 - (col & 07);
			} else {
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

int cli_readline_into_buffer(const char *const prompt, char *buffer,
			     int timeout)
{
	char *p = buffer;
	uint len = CONFIG_SYS_CBSIZE;
	int rc;
	static int initted;

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
	if (IS_ENABLED(CONFIG_CMDLINE_EDITING) && (gd->flags & GD_FLG_RELOC)) {
		if (!initted) {
			rc = hist_init();
			if (rc == 0)
				initted = 1;
		}

		if (prompt)
			puts(prompt);

		rc = cread_लिन्या(prompt, p, &len, timeout);
		return rc < 0 ? rc : len;

	} else {
		return cread_लिन्या_simple(prompt, p);
	}
}
