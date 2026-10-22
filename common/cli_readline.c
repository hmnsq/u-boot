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

DECLARE_GLOBAL_معلومات_PTR;

ساكن ثابت حرف erase_seq[] = "\b \b";	/* erase sequence */
ساكن ثابت حرف   tab_seq[] = "        ";	/* used to expand TABs */

حرف console_بفر[CONFIG_SYS_CBSIZE + 1];	/* console I/O buffer	*/

ساكن حرف *delete_char (حرف *بفر, حرف *p, صحيح *colp, صحيح *np, صحيح plen)
{
	حرف *s;

	إذا (*np == 0)
		أرجع p;

	إذا (*(--p) == '\t') {		/* will retype the whole line */
		طالما (*colp > plen) {
			puts(erase_seq);
			(*colp)--;
		}
		لكل (s = بفر; s < p; ++s) {
			إذا (*s == '\t') {
				puts(tab_seq + ((*colp) & 07));
				*colp += 8 - ((*colp) & 07);
			} وإلا {
				++(*colp);
				putc(*s);
			}
		}
	} وإلا {
		puts(erase_seq);
		(*colp)--;
	}
	(*np)--;

	أرجع p;
}

#جيڪڏهن_وصف_ٿيل CONFIG_CMDLINE_EDITING

/*
 * cmdline-editing related codes from vivi.
 * Author: Janghoon Lyu <nandy@mizi.com>
 */

#وصف_ڪريو putnstr(str, n)	لکو("%.*s", (صحيح)n, str)

#وصف_ڪريو CTL_BACKSPACE		('\b')
#وصف_ڪريو DEL			((حرف)255)
#وصف_ڪريو DEL7			((حرف)127)
#وصف_ڪريو CREAD_HIST_CHAR		('!')

#وصف_ڪريو getcmd_putch(ch)	putc(ch)
#وصف_ڪريو getcmd_getch()		getchar()
#وصف_ڪريو getcmd_cbeep()		getcmd_putch('\a')

#جيڪڏهن_وصف_ٿيل CONFIG_XPL_BUILD
#وصف_ڪريو HIST_MAX		3
#وصف_ڪريو HIST_سائيز		32
#وإلا
#وصف_ڪريو HIST_MAX		20
#وصف_ڪريو HIST_سائيز		CONFIG_SYS_CBSIZE
#جيڪڏهن_پورو

ساكن صحيح hist_max;
ساكن صحيح hist_جوڙيو_idx;
ساكن صحيح hist_cur = -1;
ساكن بنانشان hist_انگ;

#جيڪڏهن_وصف_نه_ٿيل CONFIG_CMD_HISTORY_USE_CALLOC
ساكن حرف hist_معلومات[HIST_MAX][HIST_سائيز + 1];
#جيڪڏهن_پورو
ساكن حرف *hist_list[HIST_MAX];

#وصف_ڪريو جوڙيو_idx_minus_one() ((hist_جوڙيو_idx == 0) ? hist_max : hist_جوڙيو_idx-1)

ساكن فراغ getcmd_putchars(صحيح ڳڻپ, صحيح ch)
{
	صحيح i;

	لكل (i = 0; i < ڳڻپ; i++)
		getcmd_putch(ch);
}

ساكن صحيح hist_init(فراغ)
{
	صحيح i;

#جيڪڏهن_وصف_نه_ٿيل CONFIG_CMD_HISTORY_USE_CALLOC
	لكل (i = 0; i < HIST_MAX; i++) {
		hist_list[i] = hist_معلومات[i];
		hist_list[i][0] = '\0';
	}
#وإلا
	بنانشان حرف *hist = صفر_يادگيري_ڏيو(HIST_MAX, HIST_سائيز + 1);
	إذا (!hist)
		panic("%s: calloc: out of memory!\n", __func__);

	لكل (i = 0; i < HIST_MAX; i++)
		hist_list[i] = hist + (i * (HIST_سائيز + 1));
#جيڪڏهن_پورو

	hist_max = 0;
	hist_جوڙيو_idx = 0;
	hist_cur = -1;
	hist_انگ = 0;

	أرجع 0;
}

ساكن فراغ cread_جوڙيو_منزل_hist(حرف *سٽ)
{
	strcpy(hist_list[hist_جوڙيو_idx], سٽ);

	إذا (++hist_جوڙيو_idx >= HIST_MAX)
		hist_جوڙيو_idx = 0;

	إذا (hist_جوڙيو_idx > hist_max)
		hist_max = hist_جوڙيو_idx;

	hist_انگ++;
}

ساكن حرف *hist_prev(فراغ)
{
	حرف *ret;
	صحيح old_cur;

	إذا (hist_cur < 0)
		أرجع عدم;

	old_cur = hist_cur;
	إذا (--hist_cur < 0)
		hist_cur = hist_max;

	إذا (hist_cur == hist_جوڙيو_idx) {
		hist_cur = old_cur;
		ret = عدم;
	} وإلا {
		ret = hist_list[hist_cur];
	}

	أرجع ret;
}

ساكن حرف *hist_ايندڙ(فراغ)
{
	حرف *ret;

	إذا (hist_cur < 0)
		أرجع عدم;

	إذا (hist_cur == hist_جوڙيو_idx)
		أرجع عدم;

	إذا (++hist_cur > hist_max)
		hist_cur = 0;

	إذا (hist_cur == hist_جوڙيو_idx)
		ret = "";
	وإلا
		ret = hist_list[hist_cur];

	أرجع ret;
}

فراغ cread_print_hist_list(فراغ)
{
	صحيح i;
	uint n;

	n = hist_انگ - hist_max;

	i = hist_جوڙيو_idx + 1;
	طالما (1) {
		إذا (i > hist_max)
			i = 0;
		إذا (i == hist_جوڙيو_idx)
			اكسر;
		لکو("%s\n", hist_list[i]);
		n++;
		i++;
	}
}

#وصف_ڪريو BEGINNING_جو_سٽ() {			\
	طالما (cls->انگ) {			\
		getcmd_putch(CTL_BACKSPACE);	\
		cls->انگ--;			\
	}					\
}

#وصف_ڪريو ERASE_منزل_EOL() {				\
	إذا (cls->انگ < cls->eol_انگ) {		\
		لکو("%*s", (صحيح)(cls->eol_انگ - cls->انگ), ""); \
		افعل {					\
			getcmd_putch(CTL_BACKSPACE);	\
		} طالما (--cls->eol_انگ > cls->انگ);	\
	}						\
}

#وصف_ڪريو REFRESH_منزل_EOL() {				\
	إذا (cls->انگ < cls->eol_انگ) {			\
		uint wlen = cls->eol_انگ - cls->انگ;	\
		putnstr(buf + cls->انگ, wlen);		\
		cls->انگ = cls->eol_انگ;		\
	}						\
}

ساكن فراغ cread_جوڙيو_char(حرف ichar, صحيح وجهو, uint *انگ,
			   uint *eol_انگ, حرف *buf, uint len)
{
	uint wlen;

	/* room ??? */
	إذا (وجهو || *انگ == *eol_انگ) {
		إذا (*eol_انگ > len - 1) {
			getcmd_cbeep();
			أرجع;
		}
		(*eol_انگ)++;
	}

	إذا (وجهو) {
		wlen = *eol_انگ - *انگ;
		إذا (wlen > 1)
			memmove(&buf[*انگ+1], &buf[*انگ], wlen-1);

		buf[*انگ] = ichar;
		putnstr(buf + *انگ, wlen);
		(*انگ)++;
		طالما (--wlen)
			getcmd_putch(CTL_BACKSPACE);
	} وإلا {
		/* echo the character */
		wlen = 1;
		buf[*انگ] = ichar;
		putnstr(buf + *انگ, wlen);
		(*انگ)++;
	}
}

ساكن فراغ cread_جوڙيو_str(حرف *str, صحيح strsize, صحيح وجهو,
			  uint *انگ, uint *eol_انگ, حرف *buf, uint len)
{
	طالما (strsize--) {
		cread_جوڙيو_char(*str, وجهو, انگ, eol_انگ, buf, len);
		str++;
	}
}

صحيح cread_سٽ_process_ch(بنية cli_سٽ_state *cls, حرف ichar)
{
	حرف *buf = cls->buf;

	/* ichar=0x0 when error occurs in U-Boot getc */
	إذا (!ichar)
		أرجع -EAGAIN;

	إذا (ichar == '\n') {
		putc('\n');
		buf[cls->eol_انگ] = '\0';	/* terminate the string */
		أرجع 0;
	}

	بدل (ichar) {
	حالة CTL_CH('a'):
		BEGINNING_جو_سٽ();
		اكسر;
	حالة CTL_CH('c'):	/* ^C - break */
		*buf = '\0';	/* discard input */
		أرجع -EINTR;
	حالة CTL_CH('f'):
		إذا (cls->انگ < cls->eol_انگ) {
			getcmd_putch(buf[cls->انگ]);
			cls->انگ++;
		}
		اكسر;
	حالة CTL_CH('b'):
		إذا (cls->انگ) {
			getcmd_putch(CTL_BACKSPACE);
			cls->انگ--;
		}
		اكسر;
	حالة CTL_CH('d'):
		إذا (cls->انگ < cls->eol_انگ) {
			uint wlen;

			wlen = cls->eol_انگ - cls->انگ - 1;
			إذا (wlen) {
				memmove(&buf[cls->انگ], &buf[cls->انگ + 1],
					wlen);
				putnstr(buf + cls->انگ, wlen);
			}

			getcmd_putch(' ');
			افعل {
				getcmd_putch(CTL_BACKSPACE);
			} طالما (wlen--);
			cls->eol_انگ--;
		}
		اكسر;
	حالة CTL_CH('k'):
		ERASE_منزل_EOL();
		اكسر;
	حالة CTL_CH('e'):
		REFRESH_منزل_EOL();
		اكسر;
	حالة CTL_CH('o'):
		cls->وجهو = !cls->وجهو;
		اكسر;
	حالة CTL_CH('w'):
		إذا (cls->انگ) {
			uint بنياد, wlen;

			لكل (بنياد = cls->انگ;
			     بنياد > 0 && buf[بنياد - 1] == ' ';)
				بنياد--;
			لكل (; بنياد > 0 && buf[بنياد - 1] != ' ';)
				بنياد--;

			/* now delete chars from base to cls->num */
			wlen = cls->انگ - بنياد;
			cls->eol_انگ -= wlen;
			memmove(&buf[بنياد], &buf[cls->انگ],
				cls->eol_انگ - بنياد + 1);
			cls->انگ = بنياد;
			getcmd_putchars(wlen, CTL_BACKSPACE);
			puts(buf + بنياد);
			getcmd_putchars(wlen, ' ');
			getcmd_putchars(wlen + cls->eol_انگ - cls->انگ,
					CTL_BACKSPACE);
		}
		اكسر;
	حالة CTL_CH('x'):
	حالة CTL_CH('u'):
		BEGINNING_جو_سٽ();
		ERASE_منزل_EOL();
		اكسر;
	حالة DEL:
	حالة DEL7:
	حالة 8:
		إذا (cls->انگ) {
			uint wlen;

			wlen = cls->eol_انگ - cls->انگ;
			cls->انگ--;
			memmove(&buf[cls->انگ], &buf[cls->انگ + 1], wlen);
			getcmd_putch(CTL_BACKSPACE);
			putnstr(buf + cls->انگ, wlen);
			getcmd_putch(' ');
			افعل {
				getcmd_putch(CTL_BACKSPACE);
			} طالما (wlen--);
			cls->eol_انگ--;
		}
		اكسر;
	حالة CTL_CH('p'):
	حالة CTL_CH('n'):
		إذا (cls->history) {
			حرف *hline;

			إذا (ichar == CTL_CH('p'))
				hline = hist_prev();
			وإلا
				hline = hist_ايندڙ();

			إذا (!hline) {
				getcmd_cbeep();
				اكسر;
			}

			/* nuke the current line */
			/* first, go home */
			BEGINNING_جو_سٽ();

			/* erase to end of line */
			ERASE_منزل_EOL();

			/* copy new line into place and display */
			strcpy(buf, hline);
			cls->eol_انگ = لڙي_ڊيگهه(buf);
			REFRESH_منزل_EOL();
			اكسر;
		}
		اكسر;
	حالة '\t':
		إذا (IS_ENABLED(CONFIG_AUTO_COMPLETE) && cls->cmd_complete) {
			صحيح num2, کانو;

			/* do not autocomplete when in the middle */
			إذا (cls->انگ < cls->eol_انگ) {
				getcmd_cbeep();
				اكسر;
			}

			buf[cls->انگ] = '\0';
			کانو = لڙي_ڊيگهه(cls->prompt) + cls->eol_انگ;
			num2 = cls->انگ;
			إذا (cmd_auto_complete(cls->prompt, buf, &num2, &کانو)) {
				کانو = num2 - cls->انگ;
				cls->انگ += کانو;
				cls->eol_انگ += کانو;
			}
			اكسر;
		}
		fallthrough;
	افتراضي:
		cread_جوڙيو_char(ichar, cls->وجهو, &cls->انگ, &cls->eol_انگ,
			       buf, cls->len);
		اكسر;
	}

	/*
	 * keep the string terminated...if we added a char at the end then we
	 * want a \0 after it
	 */
	buf[cls->eol_انگ] = '\0';

	أرجع -EAGAIN;
}

فراغ cli_cread_init(بنية cli_سٽ_state *cls, حرف *buf, uint buf_سائيز)
{
	صحيح init_len = لڙي_ڊيگهه(buf);

	memset(cls, '\0', حجم(بنية cli_سٽ_state));
	cls->وجهو = true;
	cls->buf = buf;
	cls->len = buf_سائيز;

	إذا (init_len)
		cread_جوڙيو_str(buf, init_len, 0, &cls->انگ, &cls->eol_انگ, buf,
			      buf_سائيز);
}

ساكن صحيح cread_سٽ(ثابت حرف *ثابت prompt, حرف *buf, بنانشان صحيح *len,
		      صحيح timeout)
{
	بنية cli_ch_state s_cch, *cch = &s_cch;
	بنية cli_سٽ_state s_cls, *cls = &s_cls;
	حرف ichar;
	صحيح پهريون = 1;

	cli_ch_init(cch);
	cli_cread_init(cls, buf, *len);
	cls->prompt = prompt;
	cls->history = true;
	cls->cmd_complete = true;

	طالما (1) {
		صحيح ret;

		/* Check for saved characters */
		ichar = cli_ch_process(cch, 0);

		إذا (!ichar) {
			إذا (bootretry_tstc_timeout())
				أرجع -2;	/* timed out */
			إذا (پهريون && timeout) {
				u64 etime = endtick(timeout);

				طالما (!tstc()) {	/* while no incoming data */
					إذا (get_ticks() >= etime)
						أرجع -2;	/* timed out */
					وقت_جدول();
				}
				پهريون = 0;
			}

			ichar = getcmd_getch();
			ichar = cli_ch_process(cch, ichar);
		}

		ret = cread_سٽ_process_ch(cls, ichar);
		إذا (ret == -EINTR)
			أرجع -1;
		وإلا إذا (!ret)
			اكسر;
	}
	*len = cls->eol_انگ;

	إذا (buf[0] && buf[0] != CREAD_HIST_CHAR)
		cread_جوڙيو_منزل_hist(buf);
	hist_cur = hist_جوڙيو_idx;

	أرجع 0;
}

#وإلا /* !CONFIG_CMDLINE_EDITING */

ساكن مضمن صحيح hist_init(فراغ)
{
	أرجع 0;
}

ساكن صحيح cread_سٽ(ثابت حرف *ثابت prompt, حرف *buf, بنانشان صحيح *len,
		      صحيح timeout)
{
	أرجع 0;
}

#جيڪڏهن_پورو /* CONFIG_CMDLINE_EDITING */

/****************************************************************************/

صحيح cli_readline(ثابت حرف *ثابت prompt)
{
	/*
	 * If console_buffer isn't 0-length the user will be prompted to modify
	 * it instead of entering it from scratch as desired.
	 */
	console_بفر[0] = '\0';

	أرجع cli_readline_into_بفر(prompt, console_بفر, 0);
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
ساكن صحيح cread_سٽ_simple(ثابت حرف *ثابت prompt, حرف *p)
{
	حرف *p_buf = p;
	صحيح n = 0;		/* buffer index */
	صحيح plen = 0;		/* prompt length */
	صحيح کانو;		/* output column cnt */
	صحيح c;

	/* print prompt */
	إذا (prompt) {
		plen = لڙي_ڊيگهه(prompt);
		puts(prompt);
	}
	کانو = plen;

	لكل (;;) {
		إذا (bootretry_tstc_timeout())
			أرجع -2;	/* timed out */
		وقت_جدول();	/* Trigger watchdog, if needed */

		c = getchar();

		/*
		 * Special character handling
		 */
		بدل (c) {
		حالة '\r':			/* Enter		*/
		حالة '\n':
			*p = '\0';
			puts("\r\n");
			أرجع p - p_buf;

		حالة '\0':			/* nul			*/
			استمر;

		حالة 0x03:			/* ^C - break		*/
			p_buf[0] = '\0';	/* discard input */
			أرجع -1;

		حالة 0x15:			/* ^U - erase line	*/
			طالما (کانو > plen) {
				puts(erase_seq);
				--کانو;
			}
			p = p_buf;
			n = 0;
			استمر;

		حالة 0x17:			/* ^W - erase word	*/
			p = delete_char(p_buf, p, &کانو, &n, plen);
			طالما ((n > 0) && (*p != ' '))
				p = delete_char(p_buf, p, &کانو, &n, plen);
			استمر;

		حالة 0x08:			/* ^H  - backspace	*/
		حالة 0x7F:			/* DEL - backspace	*/
			p = delete_char(p_buf, p, &کانو, &n, plen);
			استمر;

		افتراضي:
			/* Must be a normal character then */
			إذا (n >= CONFIG_SYS_CBSIZE - 2) { /* Buffer full */
				putc('\a');
				اكسر;
			}
			إذا (c == '\t') {	/* expand TABs */
				إذا (IS_ENABLED(CONFIG_AUTO_COMPLETE)) {
					/*
					 * if auto-completion triggered just
					 * continue
					 */
					*p = '\0';
					إذا (cmd_auto_complete(prompt,
							      console_بفر,
							      &n, &کانو)) {
						p = p_buf + n;	/* reset */
						استمر;
					}
				}
				puts(tab_seq + (کانو & 07));
				کانو += 8 - (کانو & 07);
			} وإلا {
				حرف __maybe_unused buf[2];

				/*
				 * Echo input using puts() to force an LCD
				 * flush if we are using an LCD
				 */
				++کانو;
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

صحيح cli_readline_into_بفر(ثابت حرف *ثابت prompt, حرف *بفر,
			     صحيح timeout)
{
	حرف *p = بفر;
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
	إذا (IS_ENABLED(CONFIG_CMDLINE_EDITING) && (gd->flags & GD_FLG_RELOC)) {
		إذا (!initted) {
			rc = hist_init();
			إذا (rc == 0)
				initted = 1;
		}

		إذا (prompt)
			puts(prompt);

		rc = cread_سٽ(prompt, p, &len, timeout);
		أرجع rc < 0 ? rc : len;

	} وإلا {
		أرجع cread_سٽ_simple(prompt, p);
	}
}
