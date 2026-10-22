// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Add to readline cmdline-editing by
 * (C) Copyright 2005
 * JinHua Luo, GuangDong Linux Center, <luo.jinhua@gd-linux.com>
 */

#ضمن <bootretry.h>
#ضمن <cli.h>
#ضمن <command.h>
#ضمن <hang.h>
#ضمن <malloc.h>
#ضمن <time.h>
#ضمن <watchdog.h>
#ضمن <linux/errno.h>
#ضمن <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

ساكن ثابت حرف erase_seq[] = "\b \b";	/* erase sequence */
ساكن ثابت حرف   tab_seq[] = "        ";	/* used to expand TABs */

حرف console_buffer[CONFIG_SYS_CBSIZE + 1];	/* console I/O buffer	*/

ساكن حرف *delete_char (حرف *buffer, حرف *p, صحيح *colp, صحيح *np, صحيح plen)
{
	حرف *s;

	אם (*np == 0)
		החזר p;

	אם (*(--p) == '\t') {		/* will retype the whole line */
		כלעוד (*colp > plen) {
			puts(erase_seq);
			(*colp)--;
		}
		עבור (s = buffer; s < p; ++s) {
			אם (*s == '\t') {
				puts(tab_seq + ((*colp) & 07));
				*colp += 8 - ((*colp) & 07);
			} אחרת {
				++(*colp);
				putc(*s);
			}
		}
	} אחרת {
		puts(erase_seq);
		(*colp)--;
	}
	(*np)--;

	החזר p;
}

#ifdef CONFIG_CMDLINE_EDITING

/*
 * cmdline-editing related codes from vivi.
 * Author: Janghoon Lyu <nandy@mizi.com>
 */

#define putnstr(str, n)	printf("%.*s", (صحيح)n, str)

#define CTL_BACKSPACE		('\b')
#define DEL			((حرف)255)
#define DEL7			((حرف)127)
#define CREAD_HIST_CHAR		('!')

#define getcmd_putch(ch)	putc(ch)
#define getcmd_getch()		getchar()
#define getcmd_cbeep()		getcmd_putch('\a')

#ifdef CONFIG_XPL_BUILD
#define HIST_MAX		3
#define HIST_SIZE		32
#אחרת
#define HIST_MAX		20
#define HIST_SIZE		CONFIG_SYS_CBSIZE
#endif

ساكن صحيح hist_max;
ساكن صحيح hist_add_idx;
ساكن صحيح hist_cur = -1;
ساكن unsigned hist_num;

#ifndef CONFIG_CMD_HISTORY_USE_CALLOC
ساكن حرف hist_data[HIST_MAX][HIST_SIZE + 1];
#endif
ساكن حرف *hist_list[HIST_MAX];

#define add_idx_minus_one() ((hist_add_idx == 0) ? hist_max : hist_add_idx-1)

ساكن فراغ getcmd_putchars(صحيح count, صحيح ch)
{
	صحيح i;

	עבור (i = 0; i < count; i++)
		getcmd_putch(ch);
}

ساكن صحيح hist_init(فراغ)
{
	صحيح i;

#ifndef CONFIG_CMD_HISTORY_USE_CALLOC
	עבור (i = 0; i < HIST_MAX; i++) {
		hist_list[i] = hist_data[i];
		hist_list[i][0] = '\0';
	}
#אחרת
	unsigned حرف *hist = calloc(HIST_MAX, HIST_SIZE + 1);
	אם (!hist)
		panic("%s: calloc: out of memory!\n", __func__);

	עבור (i = 0; i < HIST_MAX; i++)
		hist_list[i] = hist + (i * (HIST_SIZE + 1));
#endif

	hist_max = 0;
	hist_add_idx = 0;
	hist_cur = -1;
	hist_num = 0;

	החזר 0;
}

ساكن فراغ cread_add_to_hist(حرف *line)
{
	strcpy(hist_list[hist_add_idx], line);

	אם (++hist_add_idx >= HIST_MAX)
		hist_add_idx = 0;

	אם (hist_add_idx > hist_max)
		hist_max = hist_add_idx;

	hist_num++;
}

ساكن حرف *hist_prev(فراغ)
{
	حرف *ret;
	صحيح old_cur;

	אם (hist_cur < 0)
		החזר عدم;

	old_cur = hist_cur;
	אם (--hist_cur < 0)
		hist_cur = hist_max;

	אם (hist_cur == hist_add_idx) {
		hist_cur = old_cur;
		ret = عدم;
	} אחרת {
		ret = hist_list[hist_cur];
	}

	החזר ret;
}

ساكن حرف *hist_next(فراغ)
{
	حرف *ret;

	אם (hist_cur < 0)
		החזר عدم;

	אם (hist_cur == hist_add_idx)
		החזר عدم;

	אם (++hist_cur > hist_max)
		hist_cur = 0;

	אם (hist_cur == hist_add_idx)
		ret = "";
	אחרת
		ret = hist_list[hist_cur];

	החזר ret;
}

فراغ cread_print_hist_list(فراغ)
{
	صحيح i;
	uint n;

	n = hist_num - hist_max;

	i = hist_add_idx + 1;
	כלעוד (1) {
		אם (i > hist_max)
			i = 0;
		אם (i == hist_add_idx)
			اكسر;
		printf("%s\n", hist_list[i]);
		n++;
		i++;
	}
}

#define BEGINNING_OF_LINE() {			\
	כלעוד (cls->num) {			\
		getcmd_putch(CTL_BACKSPACE);	\
		cls->num--;			\
	}					\
}

#define ERASE_TO_EOL() {				\
	אם (cls->num < cls->eol_num) {		\
		printf("%*s", (صحيح)(cls->eol_num - cls->num), ""); \
		עשה {					\
			getcmd_putch(CTL_BACKSPACE);	\
		} כלעוד (--cls->eol_num > cls->num);	\
	}						\
}

#define REFRESH_TO_EOL() {				\
	אם (cls->num < cls->eol_num) {			\
		uint wlen = cls->eol_num - cls->num;	\
		putnstr(buf + cls->num, wlen);		\
		cls->num = cls->eol_num;		\
	}						\
}

ساكن فراغ cread_add_char(حرف ichar, صحيح insert, uint *num,
			   uint *eol_num, حرف *buf, uint len)
{
	uint wlen;

	/* room ??? */
	אם (insert || *num == *eol_num) {
		אם (*eol_num > len - 1) {
			getcmd_cbeep();
			החזר;
		}
		(*eol_num)++;
	}

	אם (insert) {
		wlen = *eol_num - *num;
		אם (wlen > 1)
			memmove(&buf[*num+1], &buf[*num], wlen-1);

		buf[*num] = ichar;
		putnstr(buf + *num, wlen);
		(*num)++;
		כלעוד (--wlen)
			getcmd_putch(CTL_BACKSPACE);
	} אחרת {
		/* echo the character */
		wlen = 1;
		buf[*num] = ichar;
		putnstr(buf + *num, wlen);
		(*num)++;
	}
}

ساكن فراغ cread_add_str(حرف *str, صحيح strsize, صحيح insert,
			  uint *num, uint *eol_num, حرف *buf, uint len)
{
	כלעוד (strsize--) {
		cread_add_char(*str, insert, num, eol_num, buf, len);
		str++;
	}
}

صحيح cread_line_process_ch(بنية cli_line_state *cls, حرف ichar)
{
	حرف *buf = cls->buf;

	/* ichar=0x0 when error occurs in U-Boot getc */
	אם (!ichar)
		החזר -EAGAIN;

	אם (ichar == '\n') {
		putc('\n');
		buf[cls->eol_num] = '\0';	/* terminate the string */
		החזר 0;
	}

	بدل (ichar) {
	حالة CTL_CH('a'):
		BEGINNING_OF_LINE();
		اكسر;
	حالة CTL_CH('c'):	/* ^C - break */
		*buf = '\0';	/* discard input */
		החזר -EINTR;
	حالة CTL_CH('f'):
		אם (cls->num < cls->eol_num) {
			getcmd_putch(buf[cls->num]);
			cls->num++;
		}
		اكسر;
	حالة CTL_CH('b'):
		אם (cls->num) {
			getcmd_putch(CTL_BACKSPACE);
			cls->num--;
		}
		اكسر;
	حالة CTL_CH('d'):
		אם (cls->num < cls->eol_num) {
			uint wlen;

			wlen = cls->eol_num - cls->num - 1;
			אם (wlen) {
				memmove(&buf[cls->num], &buf[cls->num + 1],
					wlen);
				putnstr(buf + cls->num, wlen);
			}

			getcmd_putch(' ');
			עשה {
				getcmd_putch(CTL_BACKSPACE);
			} כלעוד (wlen--);
			cls->eol_num--;
		}
		اكسر;
	حالة CTL_CH('k'):
		ERASE_TO_EOL();
		اكسر;
	حالة CTL_CH('e'):
		REFRESH_TO_EOL();
		اكسر;
	حالة CTL_CH('o'):
		cls->insert = !cls->insert;
		اكسر;
	حالة CTL_CH('w'):
		אם (cls->num) {
			uint base, wlen;

			עבור (base = cls->num;
			     base > 0 && buf[base - 1] == ' ';)
				base--;
			עבור (; base > 0 && buf[base - 1] != ' ';)
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
		اكسر;
	حالة CTL_CH('x'):
	حالة CTL_CH('u'):
		BEGINNING_OF_LINE();
		ERASE_TO_EOL();
		اكسر;
	حالة DEL:
	حالة DEL7:
	حالة 8:
		אם (cls->num) {
			uint wlen;

			wlen = cls->eol_num - cls->num;
			cls->num--;
			memmove(&buf[cls->num], &buf[cls->num + 1], wlen);
			getcmd_putch(CTL_BACKSPACE);
			putnstr(buf + cls->num, wlen);
			getcmd_putch(' ');
			עשה {
				getcmd_putch(CTL_BACKSPACE);
			} כלעוד (wlen--);
			cls->eol_num--;
		}
		اكسر;
	حالة CTL_CH('p'):
	حالة CTL_CH('n'):
		אם (cls->history) {
			حرف *hline;

			אם (ichar == CTL_CH('p'))
				hline = hist_prev();
			אחרת
				hline = hist_next();

			אם (!hline) {
				getcmd_cbeep();
				اكسر;
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
			اكسر;
		}
		اكسر;
	حالة '\t':
		אם (IS_ENABLED(CONFIG_AUTO_COMPLETE) && cls->cmd_complete) {
			صحيح num2, col;

			/* do not autocomplete when in the middle */
			אם (cls->num < cls->eol_num) {
				getcmd_cbeep();
				اكسر;
			}

			buf[cls->num] = '\0';
			col = strlen(cls->prompt) + cls->eol_num;
			num2 = cls->num;
			אם (cmd_auto_complete(cls->prompt, buf, &num2, &col)) {
				col = num2 - cls->num;
				cls->num += col;
				cls->eol_num += col;
			}
			اكسر;
		}
		fallthrough;
	افتراضي:
		cread_add_char(ichar, cls->insert, &cls->num, &cls->eol_num,
			       buf, cls->len);
		اكسر;
	}

	/*
	 * keep the string terminated...if we added a char at the end then we
	 * want a \0 after it
	 */
	buf[cls->eol_num] = '\0';

	החזר -EAGAIN;
}

فراغ cli_cread_init(بنية cli_line_state *cls, حرف *buf, uint buf_size)
{
	صحيح init_len = strlen(buf);

	memset(cls, '\0', حجم(بنية cli_line_state));
	cls->insert = true;
	cls->buf = buf;
	cls->len = buf_size;

	אם (init_len)
		cread_add_str(buf, init_len, 0, &cls->num, &cls->eol_num, buf,
			      buf_size);
}

ساكن صحيح cread_line(ثابت حرف *ثابت prompt, حرف *buf, unsigned صحيح *len,
		      صحيح timeout)
{
	بنية cli_ch_state s_cch, *cch = &s_cch;
	بنية cli_line_state s_cls, *cls = &s_cls;
	حرف ichar;
	صحيح first = 1;

	cli_ch_init(cch);
	cli_cread_init(cls, buf, *len);
	cls->prompt = prompt;
	cls->history = true;
	cls->cmd_complete = true;

	כלעוד (1) {
		صحيح ret;

		/* Check for saved characters */
		ichar = cli_ch_process(cch, 0);

		אם (!ichar) {
			אם (bootretry_tstc_timeout())
				החזר -2;	/* timed out */
			אם (first && timeout) {
				u64 etime = endtick(timeout);

				כלעוד (!tstc()) {	/* while no incoming data */
					אם (get_ticks() >= etime)
						החזר -2;	/* timed out */
					schedule();
				}
				first = 0;
			}

			ichar = getcmd_getch();
			ichar = cli_ch_process(cch, ichar);
		}

		ret = cread_line_process_ch(cls, ichar);
		אם (ret == -EINTR)
			החזר -1;
		אחרת אם (!ret)
			اكسر;
	}
	*len = cls->eol_num;

	אם (buf[0] && buf[0] != CREAD_HIST_CHAR)
		cread_add_to_hist(buf);
	hist_cur = hist_add_idx;

	החזר 0;
}

#אחרת /* !CONFIG_CMDLINE_EDITING */

ساكن مضمن صحيح hist_init(فراغ)
{
	החזר 0;
}

ساكن صحيح cread_line(ثابت حرف *ثابت prompt, حرف *buf, unsigned صحيح *len,
		      صحيح timeout)
{
	החזר 0;
}

#endif /* CONFIG_CMDLINE_EDITING */

/****************************************************************************/

صحيح cli_readline(ثابت حرف *ثابت prompt)
{
	/*
	 * If console_buffer isn't 0-length the user will be prompted to modify
	 * it instead of entering it from scratch as desired.
	 */
	console_buffer[0] = '\0';

	החזר cli_readline_into_buffer(prompt, console_buffer, 0);
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
ساكن صحيح cread_line_simple(ثابت حرف *ثابت prompt, حرف *p)
{
	حرف *p_buf = p;
	صحيح n = 0;		/* buffer index */
	صحيح plen = 0;		/* prompt length */
	صحيح col;		/* output column cnt */
	صحيح c;

	/* print prompt */
	אם (prompt) {
		plen = strlen(prompt);
		puts(prompt);
	}
	col = plen;

	עבור (;;) {
		אם (bootretry_tstc_timeout())
			החזר -2;	/* timed out */
		schedule();	/* Trigger watchdog, if needed */

		c = getchar();

		/*
		 * Special character handling
		 */
		بدل (c) {
		حالة '\r':			/* Enter		*/
		حالة '\n':
			*p = '\0';
			puts("\r\n");
			החזר p - p_buf;

		حالة '\0':			/* nul			*/
			استمر;

		حالة 0x03:			/* ^C - break		*/
			p_buf[0] = '\0';	/* discard input */
			החזר -1;

		حالة 0x15:			/* ^U - erase line	*/
			כלעוד (col > plen) {
				puts(erase_seq);
				--col;
			}
			p = p_buf;
			n = 0;
			استمر;

		حالة 0x17:			/* ^W - erase word	*/
			p = delete_char(p_buf, p, &col, &n, plen);
			כלעוד ((n > 0) && (*p != ' '))
				p = delete_char(p_buf, p, &col, &n, plen);
			استمر;

		حالة 0x08:			/* ^H  - backspace	*/
		حالة 0x7F:			/* DEL - backspace	*/
			p = delete_char(p_buf, p, &col, &n, plen);
			استمر;

		افتراضي:
			/* Must be a normal character then */
			אם (n >= CONFIG_SYS_CBSIZE - 2) { /* Buffer full */
				putc('\a');
				اكسر;
			}
			אם (c == '\t') {	/* expand TABs */
				אם (IS_ENABLED(CONFIG_AUTO_COMPLETE)) {
					/*
					 * if auto-completion triggered just
					 * continue
					 */
					*p = '\0';
					אם (cmd_auto_complete(prompt,
							      console_buffer,
							      &n, &col)) {
						p = p_buf + n;	/* reset */
						استمر;
					}
				}
				puts(tab_seq + (col & 07));
				col += 8 - (col & 07);
			} אחרת {
				حرف __maybe_unused buf[2];

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
			اكسر;
		}
	}
}

صحيح cli_readline_into_buffer(ثابت حرف *ثابت prompt, حرف *buffer,
			     صحيح timeout)
{
	حرف *p = buffer;
	uint len = CONFIG_SYS_CBSIZE;
	صحيح rc;
	ساكن صحيح initted;

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
	אם (IS_ENABLED(CONFIG_CMDLINE_EDITING) && (gd->flags & GD_FLG_RELOC)) {
		אם (!initted) {
			rc = hist_init();
			אם (rc == 0)
				initted = 1;
		}

		אם (prompt)
			puts(prompt);

		rc = cread_line(prompt, p, &len, timeout);
		החזר rc < 0 ? rc : len;

	} אחרת {
		החזר cread_line_simple(prompt, p);
	}
}
