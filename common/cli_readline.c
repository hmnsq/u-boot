// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Add to readline cmdline-editing by
 * (C) Copyright 2005
 * JinHua Luo, GuangDong Linux Center, <luo.jinhua@gd-linux.com>
 */

#شامل <bootretry.h>
#شامل <cli.h>
#شامل <command.h>
#شامل <hang.h>
#شامل <malloc.h>
#شامل <time.h>
#شامل <watchdog.h>
#شامل <linux/errno.h>
#شامل <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

ایستا ثابت نویسه erase_seq[] = "\b \b";	/* erase sequence */
ایستا ثابت نویسه   tab_seq[] = "        ";	/* used to expand TABs */

نویسه console_buffer[CONFIG_SYS_CBSIZE + 1];	/* console I/O buffer	*/

ایستا نویسه *delete_char (نویسه *buffer, نویسه *p, صحیح *colp, صحیح *np, صحیح plen)
{
	نویسه *s;

	اگر (*np == 0)
		بازگشت p;

	اگر (*(--p) == '\t') {		/* will retype the whole line */
		وقتی (*colp > plen) {
			puts(erase_seq);
			(*colp)--;
		}
		برای (s = buffer; s < p; ++s) {
			اگر (*s == '\t') {
				puts(tab_seq + ((*colp) & 07));
				*colp += 8 - ((*colp) & 07);
			} وگرنه {
				++(*colp);
				putc(*s);
			}
		}
	} وگرنه {
		puts(erase_seq);
		(*colp)--;
	}
	(*np)--;

	بازگشت p;
}

#ifdef CONFIG_CMDLINE_EDITING

/*
 * cmdline-editing related codes from vivi.
 * Author: Janghoon Lyu <nandy@mizi.com>
 */

#define putnstr(str, n)	printf("%.*s", (صحیح)n, str)

#define CTL_BACKSPACE		('\b')
#define DEL			((نویسه)255)
#define DEL7			((نویسه)127)
#define CREAD_HIST_CHAR		('!')

#define getcmd_putch(ch)	putc(ch)
#define getcmd_getch()		getchar()
#define getcmd_cbeep()		getcmd_putch('\a')

#ifdef CONFIG_XPL_BUILD
#define HIST_MAX		3
#define HIST_SIZE		32
#وگرنه
#define HIST_MAX		20
#define HIST_SIZE		CONFIG_SYS_CBSIZE
#endif

ایستا صحیح hist_max;
ایستا صحیح hist_add_idx;
ایستا صحیح hist_cur = -1;
ایستا unsigned hist_رقم;

#ifndef CONFIG_CMD_HISTORY_USE_CALLOC
ایستا نویسه hist_data[HIST_MAX][HIST_SIZE + 1];
#endif
ایستا نویسه *hist_list[HIST_MAX];

#define add_idx_minus_one() ((hist_add_idx == 0) ? hist_max : hist_add_idx-1)

ایستا تهی getcmd_putchars(صحیح شمار, صحیح ch)
{
	صحیح i;

	برای (i = 0; i < شمار; i++)
		getcmd_putch(ch);
}

ایستا صحیح hist_init(تهی)
{
	صحیح i;

#ifndef CONFIG_CMD_HISTORY_USE_CALLOC
	برای (i = 0; i < HIST_MAX; i++) {
		hist_list[i] = hist_data[i];
		hist_list[i][0] = '\0';
	}
#وگرنه
	unsigned نویسه *hist = calloc(HIST_MAX, HIST_SIZE + 1);
	اگر (!hist)
		panic("%s: calloc: out of memory!\n", __func__);

	برای (i = 0; i < HIST_MAX; i++)
		hist_list[i] = hist + (i * (HIST_SIZE + 1));
#endif

	hist_max = 0;
	hist_add_idx = 0;
	hist_cur = -1;
	hist_رقم = 0;

	بازگشت 0;
}

ایستا تهی cread_add_to_hist(نویسه *سطر)
{
	strcpy(hist_list[hist_add_idx], سطر);

	اگر (++hist_add_idx >= HIST_MAX)
		hist_add_idx = 0;

	اگر (hist_add_idx > hist_max)
		hist_max = hist_add_idx;

	hist_رقم++;
}

ایستا نویسه *hist_prev(تهی)
{
	نویسه *ret;
	صحیح old_cur;

	اگر (hist_cur < 0)
		بازگشت پوچ;

	old_cur = hist_cur;
	اگر (--hist_cur < 0)
		hist_cur = hist_max;

	اگر (hist_cur == hist_add_idx) {
		hist_cur = old_cur;
		ret = پوچ;
	} وگرنه {
		ret = hist_list[hist_cur];
	}

	بازگشت ret;
}

ایستا نویسه *hist_next(تهی)
{
	نویسه *ret;

	اگر (hist_cur < 0)
		بازگشت پوچ;

	اگر (hist_cur == hist_add_idx)
		بازگشت پوچ;

	اگر (++hist_cur > hist_max)
		hist_cur = 0;

	اگر (hist_cur == hist_add_idx)
		ret = "";
	وگرنه
		ret = hist_list[hist_cur];

	بازگشت ret;
}

تهی cread_print_hist_list(تهی)
{
	صحیح i;
	uint n;

	n = hist_رقم - hist_max;

	i = hist_add_idx + 1;
	وقتی (1) {
		اگر (i > hist_max)
			i = 0;
		اگر (i == hist_add_idx)
			بشکن;
		printf("%s\n", hist_list[i]);
		n++;
		i++;
	}
}

#define BEGINNING_OF_سطر() {			\
	وقتی (cls->رقم) {			\
		getcmd_putch(CTL_BACKSPACE);	\
		cls->رقم--;			\
	}					\
}

#define ERASE_TO_EOL() {				\
	اگر (cls->رقم < cls->eol_رقم) {		\
		printf("%*s", (صحیح)(cls->eol_رقم - cls->رقم), ""); \
		انجامبده {					\
			getcmd_putch(CTL_BACKSPACE);	\
		} وقتی (--cls->eol_رقم > cls->رقم);	\
	}						\
}

#define REFRESH_TO_EOL() {				\
	اگر (cls->رقم < cls->eol_رقم) {			\
		uint wlen = cls->eol_رقم - cls->رقم;	\
		putnstr(buf + cls->رقم, wlen);		\
		cls->رقم = cls->eol_رقم;		\
	}						\
}

ایستا تهی cread_add_char(نویسه ichar, صحیح درج, uint *رقم,
			   uint *eol_رقم, نویسه *buf, uint len)
{
	uint wlen;

	/* room ??? */
	اگر (درج || *رقم == *eol_رقم) {
		اگر (*eol_رقم > len - 1) {
			getcmd_cbeep();
			بازگشت;
		}
		(*eol_رقم)++;
	}

	اگر (درج) {
		wlen = *eol_رقم - *رقم;
		اگر (wlen > 1)
			memmove(&buf[*رقم+1], &buf[*رقم], wlen-1);

		buf[*رقم] = ichar;
		putnstr(buf + *رقم, wlen);
		(*رقم)++;
		وقتی (--wlen)
			getcmd_putch(CTL_BACKSPACE);
	} وگرنه {
		/* echo the character */
		wlen = 1;
		buf[*رقم] = ichar;
		putnstr(buf + *رقم, wlen);
		(*رقم)++;
	}
}

ایستا تهی cread_add_str(نویسه *str, صحیح strsize, صحیح درج,
			  uint *رقم, uint *eol_رقم, نویسه *buf, uint len)
{
	وقتی (strsize--) {
		cread_add_char(*str, درج, رقم, eol_رقم, buf, len);
		str++;
	}
}

صحیح cread_سطر_process_ch(ساختار cli_سطر_state *cls, نویسه ichar)
{
	نویسه *buf = cls->buf;

	/* ichar=0x0 when error occurs in U-Boot getc */
	اگر (!ichar)
		بازگشت -EAGAIN;

	اگر (ichar == '\n') {
		putc('\n');
		buf[cls->eol_رقم] = '\0';	/* terminate the string */
		بازگشت 0;
	}

	انتخاب (ichar) {
	حالت CTL_CH('a'):
		BEGINNING_OF_سطر();
		بشکن;
	حالت CTL_CH('c'):	/* ^C - break */
		*buf = '\0';	/* discard input */
		بازگشت -EINTR;
	حالت CTL_CH('f'):
		اگر (cls->رقم < cls->eol_رقم) {
			getcmd_putch(buf[cls->رقم]);
			cls->رقم++;
		}
		بشکن;
	حالت CTL_CH('b'):
		اگر (cls->رقم) {
			getcmd_putch(CTL_BACKSPACE);
			cls->رقم--;
		}
		بشکن;
	حالت CTL_CH('d'):
		اگر (cls->رقم < cls->eol_رقم) {
			uint wlen;

			wlen = cls->eol_رقم - cls->رقم - 1;
			اگر (wlen) {
				memmove(&buf[cls->رقم], &buf[cls->رقم + 1],
					wlen);
				putnstr(buf + cls->رقم, wlen);
			}

			getcmd_putch(' ');
			انجامبده {
				getcmd_putch(CTL_BACKSPACE);
			} وقتی (wlen--);
			cls->eol_رقم--;
		}
		بشکن;
	حالت CTL_CH('k'):
		ERASE_TO_EOL();
		بشکن;
	حالت CTL_CH('e'):
		REFRESH_TO_EOL();
		بشکن;
	حالت CTL_CH('o'):
		cls->درج = !cls->درج;
		بشکن;
	حالت CTL_CH('w'):
		اگر (cls->رقم) {
			uint base, wlen;

			برای (base = cls->رقم;
			     base > 0 && buf[base - 1] == ' ';)
				base--;
			برای (; base > 0 && buf[base - 1] != ' ';)
				base--;

			/* now delete chars from base to cls->num */
			wlen = cls->رقم - base;
			cls->eol_رقم -= wlen;
			memmove(&buf[base], &buf[cls->رقم],
				cls->eol_رقم - base + 1);
			cls->رقم = base;
			getcmd_putchars(wlen, CTL_BACKSPACE);
			puts(buf + base);
			getcmd_putchars(wlen, ' ');
			getcmd_putchars(wlen + cls->eol_رقم - cls->رقم,
					CTL_BACKSPACE);
		}
		بشکن;
	حالت CTL_CH('x'):
	حالت CTL_CH('u'):
		BEGINNING_OF_سطر();
		ERASE_TO_EOL();
		بشکن;
	حالت DEL:
	حالت DEL7:
	حالت 8:
		اگر (cls->رقم) {
			uint wlen;

			wlen = cls->eol_رقم - cls->رقم;
			cls->رقم--;
			memmove(&buf[cls->رقم], &buf[cls->رقم + 1], wlen);
			getcmd_putch(CTL_BACKSPACE);
			putnstr(buf + cls->رقم, wlen);
			getcmd_putch(' ');
			انجامبده {
				getcmd_putch(CTL_BACKSPACE);
			} وقتی (wlen--);
			cls->eol_رقم--;
		}
		بشکن;
	حالت CTL_CH('p'):
	حالت CTL_CH('n'):
		اگر (cls->history) {
			نویسه *hline;

			اگر (ichar == CTL_CH('p'))
				hline = hist_prev();
			وگرنه
				hline = hist_next();

			اگر (!hline) {
				getcmd_cbeep();
				بشکن;
			}

			/* nuke the current line */
			/* first, go home */
			BEGINNING_OF_سطر();

			/* erase to end of line */
			ERASE_TO_EOL();

			/* copy new line into place and display */
			strcpy(buf, hline);
			cls->eol_رقم = strlen(buf);
			REFRESH_TO_EOL();
			بشکن;
		}
		بشکن;
	حالت '\t':
		اگر (IS_ENABLED(CONFIG_AUTO_COMPLETE) && cls->cmd_complete) {
			صحیح num2, col;

			/* do not autocomplete when in the middle */
			اگر (cls->رقم < cls->eol_رقم) {
				getcmd_cbeep();
				بشکن;
			}

			buf[cls->رقم] = '\0';
			col = strlen(cls->prompt) + cls->eol_رقم;
			num2 = cls->رقم;
			اگر (cmd_auto_complete(cls->prompt, buf, &num2, &col)) {
				col = num2 - cls->رقم;
				cls->رقم += col;
				cls->eol_رقم += col;
			}
			بشکن;
		}
		fallthrough;
	پیشفرض:
		cread_add_char(ichar, cls->درج, &cls->رقم, &cls->eol_رقم,
			       buf, cls->len);
		بشکن;
	}

	/*
	 * keep the string terminated...if we added a char at the end then we
	 * want a \0 after it
	 */
	buf[cls->eol_رقم] = '\0';

	بازگشت -EAGAIN;
}

تهی cli_cread_init(ساختار cli_سطر_state *cls, نویسه *buf, uint buf_size)
{
	صحیح init_len = strlen(buf);

	memset(cls, '\0', اندازه(ساختار cli_سطر_state));
	cls->درج = true;
	cls->buf = buf;
	cls->len = buf_size;

	اگر (init_len)
		cread_add_str(buf, init_len, 0, &cls->رقم, &cls->eol_رقم, buf,
			      buf_size);
}

ایستا صحیح cread_سطر(ثابت نویسه *ثابت prompt, نویسه *buf, unsigned صحیح *len,
		      صحیح timeout)
{
	ساختار cli_ch_state s_cch, *cch = &s_cch;
	ساختار cli_سطر_state s_cls, *cls = &s_cls;
	نویسه ichar;
	صحیح first = 1;

	cli_ch_init(cch);
	cli_cread_init(cls, buf, *len);
	cls->prompt = prompt;
	cls->history = true;
	cls->cmd_complete = true;

	وقتی (1) {
		صحیح ret;

		/* Check for saved characters */
		ichar = cli_ch_process(cch, 0);

		اگر (!ichar) {
			اگر (bootretry_tstc_timeout())
				بازگشت -2;	/* timed out */
			اگر (first && timeout) {
				u64 etime = endtick(timeout);

				وقتی (!tstc()) {	/* while no incoming data */
					اگر (get_ticks() >= etime)
						بازگشت -2;	/* timed out */
					schedule();
				}
				first = 0;
			}

			ichar = getcmd_getch();
			ichar = cli_ch_process(cch, ichar);
		}

		ret = cread_سطر_process_ch(cls, ichar);
		اگر (ret == -EINTR)
			بازگشت -1;
		وگرنه اگر (!ret)
			بشکن;
	}
	*len = cls->eol_رقم;

	اگر (buf[0] && buf[0] != CREAD_HIST_CHAR)
		cread_add_to_hist(buf);
	hist_cur = hist_add_idx;

	بازگشت 0;
}

#وگرنه /* !CONFIG_CMDLINE_EDITING */

ایستا درونخطی صحیح hist_init(تهی)
{
	بازگشت 0;
}

ایستا صحیح cread_سطر(ثابت نویسه *ثابت prompt, نویسه *buf, unsigned صحیح *len,
		      صحیح timeout)
{
	بازگشت 0;
}

#endif /* CONFIG_CMDLINE_EDITING */

/****************************************************************************/

صحیح cli_readline(ثابت نویسه *ثابت prompt)
{
	/*
	 * If console_buffer isn't 0-length the user will be prompted to modify
	 * it instead of entering it from scratch as desired.
	 */
	console_buffer[0] = '\0';

	بازگشت cli_readline_into_buffer(prompt, console_buffer, 0);
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
ایستا صحیح cread_سطر_simple(ثابت نویسه *ثابت prompt, نویسه *p)
{
	نویسه *p_buf = p;
	صحیح n = 0;		/* buffer index */
	صحیح plen = 0;		/* prompt length */
	صحیح col;		/* output column cnt */
	صحیح c;

	/* print prompt */
	اگر (prompt) {
		plen = strlen(prompt);
		puts(prompt);
	}
	col = plen;

	برای (;;) {
		اگر (bootretry_tstc_timeout())
			بازگشت -2;	/* timed out */
		schedule();	/* Trigger watchdog, if needed */

		c = getchar();

		/*
		 * Special character handling
		 */
		انتخاب (c) {
		حالت '\r':			/* Enter		*/
		حالت '\n':
			*p = '\0';
			puts("\r\n");
			بازگشت p - p_buf;

		حالت '\0':			/* nul			*/
			ادامه;

		حالت 0x03:			/* ^C - break		*/
			p_buf[0] = '\0';	/* discard input */
			بازگشت -1;

		حالت 0x15:			/* ^U - erase line	*/
			وقتی (col > plen) {
				puts(erase_seq);
				--col;
			}
			p = p_buf;
			n = 0;
			ادامه;

		حالت 0x17:			/* ^W - erase word	*/
			p = delete_char(p_buf, p, &col, &n, plen);
			وقتی ((n > 0) && (*p != ' '))
				p = delete_char(p_buf, p, &col, &n, plen);
			ادامه;

		حالت 0x08:			/* ^H  - backspace	*/
		حالت 0x7F:			/* DEL - backspace	*/
			p = delete_char(p_buf, p, &col, &n, plen);
			ادامه;

		پیشفرض:
			/* Must be a normal character then */
			اگر (n >= CONFIG_SYS_CBSIZE - 2) { /* Buffer full */
				putc('\a');
				بشکن;
			}
			اگر (c == '\t') {	/* expand TABs */
				اگر (IS_ENABLED(CONFIG_AUTO_COMPLETE)) {
					/*
					 * if auto-completion triggered just
					 * continue
					 */
					*p = '\0';
					اگر (cmd_auto_complete(prompt,
							      console_buffer,
							      &n, &col)) {
						p = p_buf + n;	/* reset */
						ادامه;
					}
				}
				puts(tab_seq + (col & 07));
				col += 8 - (col & 07);
			} وگرنه {
				نویسه __maybe_unused buf[2];

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
			بشکن;
		}
	}
}

صحیح cli_readline_into_buffer(ثابت نویسه *ثابت prompt, نویسه *buffer,
			     صحیح timeout)
{
	نویسه *p = buffer;
	uint len = CONFIG_SYS_CBSIZE;
	صحیح rc;
	ایستا صحیح initted;

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
	اگر (IS_ENABLED(CONFIG_CMDLINE_EDITING) && (gd->flags & GD_FLG_RELOC)) {
		اگر (!initted) {
			rc = hist_init();
			اگر (rc == 0)
				initted = 1;
		}

		اگر (prompt)
			puts(prompt);

		rc = cread_سطر(prompt, p, &len, timeout);
		بازگشت rc < 0 ? rc : len;

	} وگرنه {
		بازگشت cread_سطر_simple(prompt, p);
	}
}
