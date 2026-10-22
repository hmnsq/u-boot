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

DECLARE_GLOBAL_انکڑے_PTR;

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

#جے_تعریف_شدہ CONFIG_CMDLINE_EDITING

/*
 * cmdline-editing related codes from vivi.
 * Author: Janghoon Lyu <nandy@mizi.com>
 */

#تعریف_کرو putnstr(str, n)	لکھو("%.*s", (صحيح)n, str)

#تعریف_کرو CTL_BACKSPACE		('\b')
#تعریف_کرو DEL			((حرف)255)
#تعریف_کرو DEL7			((حرف)127)
#تعریف_کرو CREAD_HIST_CHAR		('!')

#تعریف_کرو getcmd_putch(ch)	putc(ch)
#تعریف_کرو getcmd_getch()		getchar()
#تعریف_کرو getcmd_cbeep()		getcmd_putch('\a')

#جے_تعریف_شدہ CONFIG_XPL_BUILD
#تعریف_کرو HIST_MAX		3
#تعریف_کرو HIST_ناپ		32
#وإلا
#تعریف_کرو HIST_MAX		20
#تعریف_کرو HIST_ناپ		CONFIG_SYS_CBSIZE
#جے_انت

ساكن صحيح hist_max;
ساكن صحيح hist_جوڑو_idx;
ساكن صحيح hist_cur = -1;
ساكن بنانشانی hist_عدد;

#جے_بنا_تعریف CONFIG_CMD_HISTORY_USE_CALLOC
ساكن حرف hist_انکڑے[HIST_MAX][HIST_ناپ + 1];
#جے_انت
ساكن حرف *hist_list[HIST_MAX];

#تعریف_کرو جوڑو_idx_minus_one() ((hist_جوڑو_idx == 0) ? hist_max : hist_جوڑو_idx-1)

ساكن فراغ getcmd_putchars(صحيح گنتی, صحيح ch)
{
	صحيح i;

	لكل (i = 0; i < گنتی; i++)
		getcmd_putch(ch);
}

ساكن صحيح hist_init(فراغ)
{
	صحيح i;

#جے_بنا_تعریف CONFIG_CMD_HISTORY_USE_CALLOC
	لكل (i = 0; i < HIST_MAX; i++) {
		hist_list[i] = hist_انکڑے[i];
		hist_list[i][0] = '\0';
	}
#وإلا
	بنانشانی حرف *hist = صفر_حافظہ_دیو(HIST_MAX, HIST_ناپ + 1);
	إذا (!hist)
		panic("%s: calloc: out of memory!\n", __func__);

	لكل (i = 0; i < HIST_MAX; i++)
		hist_list[i] = hist + (i * (HIST_ناپ + 1));
#جے_انت

	hist_max = 0;
	hist_جوڑو_idx = 0;
	hist_cur = -1;
	hist_عدد = 0;

	أرجع 0;
}

ساكن فراغ cread_جوڑو_منزل_hist(حرف *سطر)
{
	strcpy(hist_list[hist_جوڑو_idx], سطر);

	إذا (++hist_جوڑو_idx >= HIST_MAX)
		hist_جوڑو_idx = 0;

	إذا (hist_جوڑو_idx > hist_max)
		hist_max = hist_جوڑو_idx;

	hist_عدد++;
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

	إذا (hist_cur == hist_جوڑو_idx) {
		hist_cur = old_cur;
		ret = عدم;
	} وإلا {
		ret = hist_list[hist_cur];
	}

	أرجع ret;
}

ساكن حرف *hist_اگلا(فراغ)
{
	حرف *ret;

	إذا (hist_cur < 0)
		أرجع عدم;

	إذا (hist_cur == hist_جوڑو_idx)
		أرجع عدم;

	إذا (++hist_cur > hist_max)
		hist_cur = 0;

	إذا (hist_cur == hist_جوڑو_idx)
		ret = "";
	وإلا
		ret = hist_list[hist_cur];

	أرجع ret;
}

فراغ cread_print_hist_list(فراغ)
{
	صحيح i;
	uint n;

	n = hist_عدد - hist_max;

	i = hist_جوڑو_idx + 1;
	طالما (1) {
		إذا (i > hist_max)
			i = 0;
		إذا (i == hist_جوڑو_idx)
			اكسر;
		لکھو("%s\n", hist_list[i]);
		n++;
		i++;
	}
}

#تعریف_کرو BEGINNING_دا_سطر() {			\
	طالما (cls->عدد) {			\
		getcmd_putch(CTL_BACKSPACE);	\
		cls->عدد--;			\
	}					\
}

#تعریف_کرو ERASE_منزل_EOL() {				\
	إذا (cls->عدد < cls->eol_عدد) {		\
		لکھو("%*s", (صحيح)(cls->eol_عدد - cls->عدد), ""); \
		افعل {					\
			getcmd_putch(CTL_BACKSPACE);	\
		} طالما (--cls->eol_عدد > cls->عدد);	\
	}						\
}

#تعریف_کرو REFRESH_منزل_EOL() {				\
	إذا (cls->عدد < cls->eol_عدد) {			\
		uint wlen = cls->eol_عدد - cls->عدد;	\
		putnstr(buf + cls->عدد, wlen);		\
		cls->عدد = cls->eol_عدد;		\
	}						\
}

ساكن فراغ cread_جوڑو_char(حرف ichar, صحيح داخل_کرو, uint *عدد,
			   uint *eol_عدد, حرف *buf, uint len)
{
	uint wlen;

	/* room ??? */
	إذا (داخل_کرو || *عدد == *eol_عدد) {
		إذا (*eol_عدد > len - 1) {
			getcmd_cbeep();
			أرجع;
		}
		(*eol_عدد)++;
	}

	إذا (داخل_کرو) {
		wlen = *eol_عدد - *عدد;
		إذا (wlen > 1)
			memmove(&buf[*عدد+1], &buf[*عدد], wlen-1);

		buf[*عدد] = ichar;
		putnstr(buf + *عدد, wlen);
		(*عدد)++;
		طالما (--wlen)
			getcmd_putch(CTL_BACKSPACE);
	} وإلا {
		/* echo the character */
		wlen = 1;
		buf[*عدد] = ichar;
		putnstr(buf + *عدد, wlen);
		(*عدد)++;
	}
}

ساكن فراغ cread_جوڑو_str(حرف *str, صحيح strsize, صحيح داخل_کرو,
			  uint *عدد, uint *eol_عدد, حرف *buf, uint len)
{
	طالما (strsize--) {
		cread_جوڑو_char(*str, داخل_کرو, عدد, eol_عدد, buf, len);
		str++;
	}
}

صحيح cread_سطر_process_ch(بنية cli_سطر_state *cls, حرف ichar)
{
	حرف *buf = cls->buf;

	/* ichar=0x0 when error occurs in U-Boot getc */
	إذا (!ichar)
		أرجع -EAGAIN;

	إذا (ichar == '\n') {
		putc('\n');
		buf[cls->eol_عدد] = '\0';	/* terminate the string */
		أرجع 0;
	}

	بدل (ichar) {
	حالة CTL_CH('a'):
		BEGINNING_دا_سطر();
		اكسر;
	حالة CTL_CH('c'):	/* ^C - break */
		*buf = '\0';	/* discard input */
		أرجع -EINTR;
	حالة CTL_CH('f'):
		إذا (cls->عدد < cls->eol_عدد) {
			getcmd_putch(buf[cls->عدد]);
			cls->عدد++;
		}
		اكسر;
	حالة CTL_CH('b'):
		إذا (cls->عدد) {
			getcmd_putch(CTL_BACKSPACE);
			cls->عدد--;
		}
		اكسر;
	حالة CTL_CH('d'):
		إذا (cls->عدد < cls->eol_عدد) {
			uint wlen;

			wlen = cls->eol_عدد - cls->عدد - 1;
			إذا (wlen) {
				memmove(&buf[cls->عدد], &buf[cls->عدد + 1],
					wlen);
				putnstr(buf + cls->عدد, wlen);
			}

			getcmd_putch(' ');
			افعل {
				getcmd_putch(CTL_BACKSPACE);
			} طالما (wlen--);
			cls->eol_عدد--;
		}
		اكسر;
	حالة CTL_CH('k'):
		ERASE_منزل_EOL();
		اكسر;
	حالة CTL_CH('e'):
		REFRESH_منزل_EOL();
		اكسر;
	حالة CTL_CH('o'):
		cls->داخل_کرو = !cls->داخل_کرو;
		اكسر;
	حالة CTL_CH('w'):
		إذا (cls->عدد) {
			uint ادھار, wlen;

			لكل (ادھار = cls->عدد;
			     ادھار > 0 && buf[ادھار - 1] == ' ';)
				ادھار--;
			لكل (; ادھار > 0 && buf[ادھار - 1] != ' ';)
				ادھار--;

			/* now delete chars from base to cls->num */
			wlen = cls->عدد - ادھار;
			cls->eol_عدد -= wlen;
			memmove(&buf[ادھار], &buf[cls->عدد],
				cls->eol_عدد - ادھار + 1);
			cls->عدد = ادھار;
			getcmd_putchars(wlen, CTL_BACKSPACE);
			puts(buf + ادھار);
			getcmd_putchars(wlen, ' ');
			getcmd_putchars(wlen + cls->eol_عدد - cls->عدد,
					CTL_BACKSPACE);
		}
		اكسر;
	حالة CTL_CH('x'):
	حالة CTL_CH('u'):
		BEGINNING_دا_سطر();
		ERASE_منزل_EOL();
		اكسر;
	حالة DEL:
	حالة DEL7:
	حالة 8:
		إذا (cls->عدد) {
			uint wlen;

			wlen = cls->eol_عدد - cls->عدد;
			cls->عدد--;
			memmove(&buf[cls->عدد], &buf[cls->عدد + 1], wlen);
			getcmd_putch(CTL_BACKSPACE);
			putnstr(buf + cls->عدد, wlen);
			getcmd_putch(' ');
			افعل {
				getcmd_putch(CTL_BACKSPACE);
			} طالما (wlen--);
			cls->eol_عدد--;
		}
		اكسر;
	حالة CTL_CH('p'):
	حالة CTL_CH('n'):
		إذا (cls->history) {
			حرف *hline;

			إذا (ichar == CTL_CH('p'))
				hline = hist_prev();
			وإلا
				hline = hist_اگلا();

			إذا (!hline) {
				getcmd_cbeep();
				اكسر;
			}

			/* nuke the current line */
			/* first, go home */
			BEGINNING_دا_سطر();

			/* erase to end of line */
			ERASE_منزل_EOL();

			/* copy new line into place and display */
			strcpy(buf, hline);
			cls->eol_عدد = لڑی_لمبائی(buf);
			REFRESH_منزل_EOL();
			اكسر;
		}
		اكسر;
	حالة '\t':
		إذا (IS_ENABLED(CONFIG_AUTO_COMPLETE) && cls->cmd_complete) {
			صحيح num2, خانہ;

			/* do not autocomplete when in the middle */
			إذا (cls->عدد < cls->eol_عدد) {
				getcmd_cbeep();
				اكسر;
			}

			buf[cls->عدد] = '\0';
			خانہ = لڑی_لمبائی(cls->prompt) + cls->eol_عدد;
			num2 = cls->عدد;
			إذا (cmd_auto_complete(cls->prompt, buf, &num2, &خانہ)) {
				خانہ = num2 - cls->عدد;
				cls->عدد += خانہ;
				cls->eol_عدد += خانہ;
			}
			اكسر;
		}
		fallthrough;
	افتراضي:
		cread_جوڑو_char(ichar, cls->داخل_کرو, &cls->عدد, &cls->eol_عدد,
			       buf, cls->len);
		اكسر;
	}

	/*
	 * keep the string terminated...if we added a char at the end then we
	 * want a \0 after it
	 */
	buf[cls->eol_عدد] = '\0';

	أرجع -EAGAIN;
}

فراغ cli_cread_init(بنية cli_سطر_state *cls, حرف *buf, uint buf_ناپ)
{
	صحيح init_len = لڑی_لمبائی(buf);

	memset(cls, '\0', حجم(بنية cli_سطر_state));
	cls->داخل_کرو = true;
	cls->buf = buf;
	cls->len = buf_ناپ;

	إذا (init_len)
		cread_جوڑو_str(buf, init_len, 0, &cls->عدد, &cls->eol_عدد, buf,
			      buf_ناپ);
}

ساكن صحيح cread_سطر(ثابت حرف *ثابت prompt, حرف *buf, بنانشانی صحيح *len,
		      صحيح timeout)
{
	بنية cli_ch_state s_cch, *cch = &s_cch;
	بنية cli_سطر_state s_cls, *cls = &s_cls;
	حرف ichar;
	صحيح پہلا = 1;

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
			إذا (پہلا && timeout) {
				u64 etime = endtick(timeout);

				طالما (!tstc()) {	/* while no incoming data */
					إذا (get_ticks() >= etime)
						أرجع -2;	/* timed out */
					ویلا_سارنی();
				}
				پہلا = 0;
			}

			ichar = getcmd_getch();
			ichar = cli_ch_process(cch, ichar);
		}

		ret = cread_سطر_process_ch(cls, ichar);
		إذا (ret == -EINTR)
			أرجع -1;
		وإلا إذا (!ret)
			اكسر;
	}
	*len = cls->eol_عدد;

	إذا (buf[0] && buf[0] != CREAD_HIST_CHAR)
		cread_جوڑو_منزل_hist(buf);
	hist_cur = hist_جوڑو_idx;

	أرجع 0;
}

#وإلا /* !CONFIG_CMDLINE_EDITING */

ساكن مضمن صحيح hist_init(فراغ)
{
	أرجع 0;
}

ساكن صحيح cread_سطر(ثابت حرف *ثابت prompt, حرف *buf, بنانشانی صحيح *len,
		      صحيح timeout)
{
	أرجع 0;
}

#جے_انت /* CONFIG_CMDLINE_EDITING */

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
ساكن صحيح cread_سطر_simple(ثابت حرف *ثابت prompt, حرف *p)
{
	حرف *p_buf = p;
	صحيح n = 0;		/* buffer index */
	صحيح plen = 0;		/* prompt length */
	صحيح خانہ;		/* output column cnt */
	صحيح c;

	/* print prompt */
	إذا (prompt) {
		plen = لڑی_لمبائی(prompt);
		puts(prompt);
	}
	خانہ = plen;

	لكل (;;) {
		إذا (bootretry_tstc_timeout())
			أرجع -2;	/* timed out */
		ویلا_سارنی();	/* Trigger watchdog, if needed */

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
			طالما (خانہ > plen) {
				puts(erase_seq);
				--خانہ;
			}
			p = p_buf;
			n = 0;
			استمر;

		حالة 0x17:			/* ^W - erase word	*/
			p = delete_char(p_buf, p, &خانہ, &n, plen);
			طالما ((n > 0) && (*p != ' '))
				p = delete_char(p_buf, p, &خانہ, &n, plen);
			استمر;

		حالة 0x08:			/* ^H  - backspace	*/
		حالة 0x7F:			/* DEL - backspace	*/
			p = delete_char(p_buf, p, &خانہ, &n, plen);
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
							      &n, &خانہ)) {
						p = p_buf + n;	/* reset */
						استمر;
					}
				}
				puts(tab_seq + (خانہ & 07));
				خانہ += 8 - (خانہ & 07);
			} وإلا {
				حرف __maybe_unused buf[2];

				/*
				 * Echo input using puts() to force an LCD
				 * flush if we are using an LCD
				 */
				++خانہ;
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

		rc = cread_سطر(prompt, p, &len, timeout);
		أرجع rc < 0 ? rc : len;

	} وإلا {
		أرجع cread_سطر_simple(prompt, p);
	}
}
