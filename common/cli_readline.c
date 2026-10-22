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

DECLARE_GLOBAL_ڈیٹا_PTR;

ساکن ثابت حرف erase_seq[] = "\b \b";	/* erase sequence */
ساکن ثابت حرف   tab_seq[] = "        ";	/* used to expand TABs */

حرف console_بفر[CONFIG_SYS_CBSIZE + 1];	/* console I/O buffer	*/

ساکن حرف *delete_char (حرف *بفر, حرف *p, صحیح *colp, صحیح *np, صحیح plen)
{
	حرف *s;

	اگر (*np == 0)
		واپس p;

	اگر (*(--p) == '\t') {		/* will retype the whole line */
		جب (*colp > plen) {
			puts(erase_seq);
			(*colp)--;
		}
		برائے (s = بفر; s < p; ++s) {
			اگر (*s == '\t') {
				puts(tab_seq + ((*colp) & 07));
				*colp += 8 - ((*colp) & 07);
			} ورنہ {
				++(*colp);
				putc(*s);
			}
		}
	} ورنہ {
		puts(erase_seq);
		(*colp)--;
	}
	(*np)--;

	واپس p;
}

#اگر_تعریف_شدہ CONFIG_CMDLINE_EDITING

/*
 * cmdline-editing related codes from vivi.
 * Author: Janghoon Lyu <nandy@mizi.com>
 */

#تعریف_کرو putnstr(str, n)	لکھو("%.*s", (صحیح)n, str)

#تعریف_کرو CTL_BACKSPACE		('\b')
#تعریف_کرو DEL			((حرف)255)
#تعریف_کرو DEL7			((حرف)127)
#تعریف_کرو CREAD_HIST_CHAR		('!')

#تعریف_کرو getcmd_putch(ch)	putc(ch)
#تعریف_کرو getcmd_getch()		getchar()
#تعریف_کرو getcmd_cbeep()		getcmd_putch('\a')

#اگر_تعریف_شدہ CONFIG_XPL_BUILD
#تعریف_کرو HIST_MAX		3
#تعریف_کرو HIST_سائز		32
#ورنہ
#تعریف_کرو HIST_MAX		20
#تعریف_کرو HIST_سائز		CONFIG_SYS_CBSIZE
#اگر_ختم

ساکن صحیح hist_max;
ساکن صحیح hist_جوڑو_idx;
ساکن صحیح hist_cur = -1;
ساکن غیرعلامتی hist_عدد;

#اگر_غیر_تعریف_شدہ CONFIG_CMD_HISTORY_USE_CALLOC
ساکن حرف hist_ڈیٹا[HIST_MAX][HIST_سائز + 1];
#اگر_ختم
ساکن حرف *hist_list[HIST_MAX];

#تعریف_کرو جوڑو_idx_minus_one() ((hist_جوڑو_idx == 0) ? hist_max : hist_جوڑو_idx-1)

ساکن خالی getcmd_putchars(صحیح گنتی, صحیح ch)
{
	صحیح i;

	برائے (i = 0; i < گنتی; i++)
		getcmd_putch(ch);
}

ساکن صحیح hist_init(خالی)
{
	صحیح i;

#اگر_غیر_تعریف_شدہ CONFIG_CMD_HISTORY_USE_CALLOC
	برائے (i = 0; i < HIST_MAX; i++) {
		hist_list[i] = hist_ڈیٹا[i];
		hist_list[i][0] = '\0';
	}
#ورنہ
	غیرعلامتی حرف *hist = صفر_حافظہ_دو(HIST_MAX, HIST_سائز + 1);
	اگر (!hist)
		panic("%s: calloc: out of memory!\n", __func__);

	برائے (i = 0; i < HIST_MAX; i++)
		hist_list[i] = hist + (i * (HIST_سائز + 1));
#اگر_ختم

	hist_max = 0;
	hist_جوڑو_idx = 0;
	hist_cur = -1;
	hist_عدد = 0;

	واپس 0;
}

ساکن خالی cread_جوڑو_مقصد_hist(حرف *سطر)
{
	strcpy(hist_list[hist_جوڑو_idx], سطر);

	اگر (++hist_جوڑو_idx >= HIST_MAX)
		hist_جوڑو_idx = 0;

	اگر (hist_جوڑو_idx > hist_max)
		hist_max = hist_جوڑو_idx;

	hist_عدد++;
}

ساکن حرف *hist_پچھلا(خالی)
{
	حرف *ret;
	صحیح old_cur;

	اگر (hist_cur < 0)
		واپس تہی;

	old_cur = hist_cur;
	اگر (--hist_cur < 0)
		hist_cur = hist_max;

	اگر (hist_cur == hist_جوڑو_idx) {
		hist_cur = old_cur;
		ret = تہی;
	} ورنہ {
		ret = hist_list[hist_cur];
	}

	واپس ret;
}

ساکن حرف *hist_اگلا(خالی)
{
	حرف *ret;

	اگر (hist_cur < 0)
		واپس تہی;

	اگر (hist_cur == hist_جوڑو_idx)
		واپس تہی;

	اگر (++hist_cur > hist_max)
		hist_cur = 0;

	اگر (hist_cur == hist_جوڑو_idx)
		ret = "";
	ورنہ
		ret = hist_list[hist_cur];

	واپس ret;
}

خالی cread_print_hist_list(خالی)
{
	صحیح i;
	uint n;

	n = hist_عدد - hist_max;

	i = hist_جوڑو_idx + 1;
	جب (1) {
		اگر (i > hist_max)
			i = 0;
		اگر (i == hist_جوڑو_idx)
			توڑو;
		لکھو("%s\n", hist_list[i]);
		n++;
		i++;
	}
}

#تعریف_کرو BEGINNING_کا_سطر() {			\
	جب (cls->عدد) {			\
		getcmd_putch(CTL_BACKSPACE);	\
		cls->عدد--;			\
	}					\
}

#تعریف_کرو ERASE_مقصد_EOL() {				\
	اگر (cls->عدد < cls->eol_عدد) {		\
		لکھو("%*s", (صحیح)(cls->eol_عدد - cls->عدد), ""); \
		کرو {					\
			getcmd_putch(CTL_BACKSPACE);	\
		} جب (--cls->eol_عدد > cls->عدد);	\
	}						\
}

#تعریف_کرو REFRESH_مقصد_EOL() {				\
	اگر (cls->عدد < cls->eol_عدد) {			\
		uint wlen = cls->eol_عدد - cls->عدد;	\
		putnstr(buf + cls->عدد, wlen);		\
		cls->عدد = cls->eol_عدد;		\
	}						\
}

ساکن خالی cread_جوڑو_char(حرف ichar, صحیح داخل_کرو, uint *عدد,
			   uint *eol_عدد, حرف *buf, uint len)
{
	uint wlen;

	/* room ??? */
	اگر (داخل_کرو || *عدد == *eol_عدد) {
		اگر (*eol_عدد > len - 1) {
			getcmd_cbeep();
			واپس;
		}
		(*eol_عدد)++;
	}

	اگر (داخل_کرو) {
		wlen = *eol_عدد - *عدد;
		اگر (wlen > 1)
			memmove(&buf[*عدد+1], &buf[*عدد], wlen-1);

		buf[*عدد] = ichar;
		putnstr(buf + *عدد, wlen);
		(*عدد)++;
		جب (--wlen)
			getcmd_putch(CTL_BACKSPACE);
	} ورنہ {
		/* echo the character */
		wlen = 1;
		buf[*عدد] = ichar;
		putnstr(buf + *عدد, wlen);
		(*عدد)++;
	}
}

ساکن خالی cread_جوڑو_str(حرف *str, صحیح strsize, صحیح داخل_کرو,
			  uint *عدد, uint *eol_عدد, حرف *buf, uint len)
{
	جب (strsize--) {
		cread_جوڑو_char(*str, داخل_کرو, عدد, eol_عدد, buf, len);
		str++;
	}
}

صحیح cread_سطر_process_ch(ساخت cli_سطر_state *cls, حرف ichar)
{
	حرف *buf = cls->buf;

	/* ichar=0x0 when error occurs in U-Boot getc */
	اگر (!ichar)
		واپس -EAGAIN;

	اگر (ichar == '\n') {
		putc('\n');
		buf[cls->eol_عدد] = '\0';	/* terminate the string */
		واپس 0;
	}

	انتخاب (ichar) {
	حالت CTL_CH('a'):
		BEGINNING_کا_سطر();
		توڑو;
	حالت CTL_CH('c'):	/* ^C - break */
		*buf = '\0';	/* discard input */
		واپس -EINTR;
	حالت CTL_CH('f'):
		اگر (cls->عدد < cls->eol_عدد) {
			getcmd_putch(buf[cls->عدد]);
			cls->عدد++;
		}
		توڑو;
	حالت CTL_CH('b'):
		اگر (cls->عدد) {
			getcmd_putch(CTL_BACKSPACE);
			cls->عدد--;
		}
		توڑو;
	حالت CTL_CH('d'):
		اگر (cls->عدد < cls->eol_عدد) {
			uint wlen;

			wlen = cls->eol_عدد - cls->عدد - 1;
			اگر (wlen) {
				memmove(&buf[cls->عدد], &buf[cls->عدد + 1],
					wlen);
				putnstr(buf + cls->عدد, wlen);
			}

			getcmd_putch(' ');
			کرو {
				getcmd_putch(CTL_BACKSPACE);
			} جب (wlen--);
			cls->eol_عدد--;
		}
		توڑو;
	حالت CTL_CH('k'):
		ERASE_مقصد_EOL();
		توڑو;
	حالت CTL_CH('e'):
		REFRESH_مقصد_EOL();
		توڑو;
	حالت CTL_CH('o'):
		cls->داخل_کرو = !cls->داخل_کرو;
		توڑو;
	حالت CTL_CH('w'):
		اگر (cls->عدد) {
			uint بنیاد, wlen;

			برائے (بنیاد = cls->عدد;
			     بنیاد > 0 && buf[بنیاد - 1] == ' ';)
				بنیاد--;
			برائے (; بنیاد > 0 && buf[بنیاد - 1] != ' ';)
				بنیاد--;

			/* now delete chars from base to cls->num */
			wlen = cls->عدد - بنیاد;
			cls->eol_عدد -= wlen;
			memmove(&buf[بنیاد], &buf[cls->عدد],
				cls->eol_عدد - بنیاد + 1);
			cls->عدد = بنیاد;
			getcmd_putchars(wlen, CTL_BACKSPACE);
			puts(buf + بنیاد);
			getcmd_putchars(wlen, ' ');
			getcmd_putchars(wlen + cls->eol_عدد - cls->عدد,
					CTL_BACKSPACE);
		}
		توڑو;
	حالت CTL_CH('x'):
	حالت CTL_CH('u'):
		BEGINNING_کا_سطر();
		ERASE_مقصد_EOL();
		توڑو;
	حالت DEL:
	حالت DEL7:
	حالت 8:
		اگر (cls->عدد) {
			uint wlen;

			wlen = cls->eol_عدد - cls->عدد;
			cls->عدد--;
			memmove(&buf[cls->عدد], &buf[cls->عدد + 1], wlen);
			getcmd_putch(CTL_BACKSPACE);
			putnstr(buf + cls->عدد, wlen);
			getcmd_putch(' ');
			کرو {
				getcmd_putch(CTL_BACKSPACE);
			} جب (wlen--);
			cls->eol_عدد--;
		}
		توڑو;
	حالت CTL_CH('p'):
	حالت CTL_CH('n'):
		اگر (cls->history) {
			حرف *hline;

			اگر (ichar == CTL_CH('p'))
				hline = hist_پچھلا();
			ورنہ
				hline = hist_اگلا();

			اگر (!hline) {
				getcmd_cbeep();
				توڑو;
			}

			/* nuke the current line */
			/* first, go home */
			BEGINNING_کا_سطر();

			/* erase to end of line */
			ERASE_مقصد_EOL();

			/* copy new line into place and display */
			strcpy(buf, hline);
			cls->eol_عدد = لڑی_لمبائی(buf);
			REFRESH_مقصد_EOL();
			توڑو;
		}
		توڑو;
	حالت '\t':
		اگر (IS_ENABLED(CONFIG_AUTO_COMPLETE) && cls->cmd_complete) {
			صحیح num2, ستون;

			/* do not autocomplete when in the middle */
			اگر (cls->عدد < cls->eol_عدد) {
				getcmd_cbeep();
				توڑو;
			}

			buf[cls->عدد] = '\0';
			ستون = لڑی_لمبائی(cls->prompt) + cls->eol_عدد;
			num2 = cls->عدد;
			اگر (cmd_auto_complete(cls->prompt, buf, &num2, &ستون)) {
				ستون = num2 - cls->عدد;
				cls->عدد += ستون;
				cls->eol_عدد += ستون;
			}
			توڑو;
		}
		fallthrough;
	طےشدہ:
		cread_جوڑو_char(ichar, cls->داخل_کرو, &cls->عدد, &cls->eol_عدد,
			       buf, cls->len);
		توڑو;
	}

	/*
	 * keep the string terminated...if we added a char at the end then we
	 * want a \0 after it
	 */
	buf[cls->eol_عدد] = '\0';

	واپس -EAGAIN;
}

خالی cli_cread_init(ساخت cli_سطر_state *cls, حرف *buf, uint buf_سائز)
{
	صحیح init_len = لڑی_لمبائی(buf);

	memset(cls, '\0', حجم(ساخت cli_سطر_state));
	cls->داخل_کرو = true;
	cls->buf = buf;
	cls->len = buf_سائز;

	اگر (init_len)
		cread_جوڑو_str(buf, init_len, 0, &cls->عدد, &cls->eol_عدد, buf,
			      buf_سائز);
}

ساکن صحیح cread_سطر(ثابت حرف *ثابت prompt, حرف *buf, غیرعلامتی صحیح *len,
		      صحیح timeout)
{
	ساخت cli_ch_state s_cch, *cch = &s_cch;
	ساخت cli_سطر_state s_cls, *cls = &s_cls;
	حرف ichar;
	صحیح پہلا = 1;

	cli_ch_init(cch);
	cli_cread_init(cls, buf, *len);
	cls->prompt = prompt;
	cls->history = true;
	cls->cmd_complete = true;

	جب (1) {
		صحیح ret;

		/* Check for saved characters */
		ichar = cli_ch_process(cch, 0);

		اگر (!ichar) {
			اگر (bootretry_tstc_timeout())
				واپس -2;	/* timed out */
			اگر (پہلا && timeout) {
				u64 etime = endtick(timeout);

				جب (!tstc()) {	/* while no incoming data */
					اگر (get_ticks() >= etime)
						واپس -2;	/* timed out */
					جدول_وقت();
				}
				پہلا = 0;
			}

			ichar = getcmd_getch();
			ichar = cli_ch_process(cch, ichar);
		}

		ret = cread_سطر_process_ch(cls, ichar);
		اگر (ret == -EINTR)
			واپس -1;
		ورنہ اگر (!ret)
			توڑو;
	}
	*len = cls->eol_عدد;

	اگر (buf[0] && buf[0] != CREAD_HIST_CHAR)
		cread_جوڑو_مقصد_hist(buf);
	hist_cur = hist_جوڑو_idx;

	واپس 0;
}

#ورنہ /* !CONFIG_CMDLINE_EDITING */

ساکن اندرونی صحیح hist_init(خالی)
{
	واپس 0;
}

ساکن صحیح cread_سطر(ثابت حرف *ثابت prompt, حرف *buf, غیرعلامتی صحیح *len,
		      صحیح timeout)
{
	واپس 0;
}

#اگر_ختم /* CONFIG_CMDLINE_EDITING */

/****************************************************************************/

صحیح cli_readline(ثابت حرف *ثابت prompt)
{
	/*
	 * If console_buffer isn't 0-length the user will be prompted to modify
	 * it instead of entering it from scratch as desired.
	 */
	console_بفر[0] = '\0';

	واپس cli_readline_into_بفر(prompt, console_بفر, 0);
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
ساکن صحیح cread_سطر_simple(ثابت حرف *ثابت prompt, حرف *p)
{
	حرف *p_buf = p;
	صحیح n = 0;		/* buffer index */
	صحیح plen = 0;		/* prompt length */
	صحیح ستون;		/* output column cnt */
	صحیح c;

	/* print prompt */
	اگر (prompt) {
		plen = لڑی_لمبائی(prompt);
		puts(prompt);
	}
	ستون = plen;

	برائے (;;) {
		اگر (bootretry_tstc_timeout())
			واپس -2;	/* timed out */
		جدول_وقت();	/* Trigger watchdog, if needed */

		c = getchar();

		/*
		 * Special character handling
		 */
		انتخاب (c) {
		حالت '\r':			/* Enter		*/
		حالت '\n':
			*p = '\0';
			puts("\r\n");
			واپس p - p_buf;

		حالت '\0':			/* nul			*/
			جاری;

		حالت 0x03:			/* ^C - break		*/
			p_buf[0] = '\0';	/* discard input */
			واپس -1;

		حالت 0x15:			/* ^U - erase line	*/
			جب (ستون > plen) {
				puts(erase_seq);
				--ستون;
			}
			p = p_buf;
			n = 0;
			جاری;

		حالت 0x17:			/* ^W - erase word	*/
			p = delete_char(p_buf, p, &ستون, &n, plen);
			جب ((n > 0) && (*p != ' '))
				p = delete_char(p_buf, p, &ستون, &n, plen);
			جاری;

		حالت 0x08:			/* ^H  - backspace	*/
		حالت 0x7F:			/* DEL - backspace	*/
			p = delete_char(p_buf, p, &ستون, &n, plen);
			جاری;

		طےشدہ:
			/* Must be a normal character then */
			اگر (n >= CONFIG_SYS_CBSIZE - 2) { /* Buffer full */
				putc('\a');
				توڑو;
			}
			اگر (c == '\t') {	/* expand TABs */
				اگر (IS_ENABLED(CONFIG_AUTO_COMPLETE)) {
					/*
					 * if auto-completion triggered just
					 * continue
					 */
					*p = '\0';
					اگر (cmd_auto_complete(prompt,
							      console_بفر,
							      &n, &ستون)) {
						p = p_buf + n;	/* reset */
						جاری;
					}
				}
				puts(tab_seq + (ستون & 07));
				ستون += 8 - (ستون & 07);
			} ورنہ {
				حرف __maybe_unused buf[2];

				/*
				 * Echo input using puts() to force an LCD
				 * flush if we are using an LCD
				 */
				++ستون;
				buf[0] = c;
				buf[1] = '\0';
				puts(buf);
			}
			*p++ = c;
			++n;
			توڑو;
		}
	}
}

صحیح cli_readline_into_بفر(ثابت حرف *ثابت prompt, حرف *بفر,
			     صحیح timeout)
{
	حرف *p = بفر;
	uint len = CONFIG_SYS_CBSIZE;
	صحیح rc;
	ساکن صحیح initted;

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
		واپس rc < 0 ? rc : len;

	} ورنہ {
		واپس cread_سطر_simple(prompt, p);
	}
}
