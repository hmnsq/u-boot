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

	إذا (*np == 0)
		أرجع p;

	إذا (*(--p) == '\t') {		/* will retype the whole line */
		طالما (*colp > plen) {
			puts(erase_seq);
			(*colp)--;
		}
		لكل (s = buffer; s < p; ++s) {
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
#وإلا
#define HIST_MAX		20
#define HIST_SIZE		CONFIG_SYS_CBSIZE
#endif

ساكن صحيح hist_max;
ساكن صحيح hist_add_idx;
ساكن صحيح hist_cur = -1;
ساكن unsigned hist_ژمارە;

#ifndef CONFIG_CMD_HISTORY_USE_CALLOC
ساكن حرف hist_data[HIST_MAX][HIST_SIZE + 1];
#endif
ساكن حرف *hist_list[HIST_MAX];

#define add_idx_minus_one() ((hist_add_idx == 0) ? hist_max : hist_add_idx-1)

ساكن فراغ getcmd_putchars(صحيح ژماردن, صحيح ch)
{
	صحيح i;

	لكل (i = 0; i < ژماردن; i++)
		getcmd_putch(ch);
}

ساكن صحيح hist_init(فراغ)
{
	صحيح i;

#ifndef CONFIG_CMD_HISTORY_USE_CALLOC
	لكل (i = 0; i < HIST_MAX; i++) {
		hist_list[i] = hist_data[i];
		hist_list[i][0] = '\0';
	}
#وإلا
	unsigned حرف *hist = calloc(HIST_MAX, HIST_SIZE + 1);
	إذا (!hist)
		panic("%s: calloc: out of memory!\n", __func__);

	لكل (i = 0; i < HIST_MAX; i++)
		hist_list[i] = hist + (i * (HIST_SIZE + 1));
#endif

	hist_max = 0;
	hist_add_idx = 0;
	hist_cur = -1;
	hist_ژمارە = 0;

	أرجع 0;
}

ساكن فراغ cread_add_to_hist(حرف *هێڵ)
{
	strcpy(hist_list[hist_add_idx], هێڵ);

	إذا (++hist_add_idx >= HIST_MAX)
		hist_add_idx = 0;

	إذا (hist_add_idx > hist_max)
		hist_max = hist_add_idx;

	hist_ژمارە++;
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

	إذا (hist_cur == hist_add_idx) {
		hist_cur = old_cur;
		ret = عدم;
	} وإلا {
		ret = hist_list[hist_cur];
	}

	أرجع ret;
}

ساكن حرف *hist_next(فراغ)
{
	حرف *ret;

	إذا (hist_cur < 0)
		أرجع عدم;

	إذا (hist_cur == hist_add_idx)
		أرجع عدم;

	إذا (++hist_cur > hist_max)
		hist_cur = 0;

	إذا (hist_cur == hist_add_idx)
		ret = "";
	وإلا
		ret = hist_list[hist_cur];

	أرجع ret;
}

فراغ cread_print_hist_list(فراغ)
{
	صحيح i;
	uint n;

	n = hist_ژمارە - hist_max;

	i = hist_add_idx + 1;
	طالما (1) {
		إذا (i > hist_max)
			i = 0;
		إذا (i == hist_add_idx)
			اكسر;
		printf("%s\n", hist_list[i]);
		n++;
		i++;
	}
}

#define BEGINNING_OF_هێڵ() {			\
	طالما (cls->ژمارە) {			\
		getcmd_putch(CTL_BACKSPACE);	\
		cls->ژمارە--;			\
	}					\
}

#define ERASE_TO_EOL() {				\
	إذا (cls->ژمارە < cls->eol_ژمارە) {		\
		printf("%*s", (صحيح)(cls->eol_ژمارە - cls->ژمارە), ""); \
		افعل {					\
			getcmd_putch(CTL_BACKSPACE);	\
		} طالما (--cls->eol_ژمارە > cls->ژمارە);	\
	}						\
}

#define REFRESH_TO_EOL() {				\
	إذا (cls->ژمارە < cls->eol_ژمارە) {			\
		uint wlen = cls->eol_ژمارە - cls->ژمارە;	\
		putnstr(buf + cls->ژمارە, wlen);		\
		cls->ژمارە = cls->eol_ژمارە;		\
	}						\
}

ساكن فراغ cread_add_char(حرف ichar, صحيح زیادکردن, uint *ژمارە,
			   uint *eol_ژمارە, حرف *buf, uint len)
{
	uint wlen;

	/* room ??? */
	إذا (زیادکردن || *ژمارە == *eol_ژمارە) {
		إذا (*eol_ژمارە > len - 1) {
			getcmd_cbeep();
			أرجع;
		}
		(*eol_ژمارە)++;
	}

	إذا (زیادکردن) {
		wlen = *eol_ژمارە - *ژمارە;
		إذا (wlen > 1)
			memmove(&buf[*ژمارە+1], &buf[*ژمارە], wlen-1);

		buf[*ژمارە] = ichar;
		putnstr(buf + *ژمارە, wlen);
		(*ژمارە)++;
		طالما (--wlen)
			getcmd_putch(CTL_BACKSPACE);
	} وإلا {
		/* echo the character */
		wlen = 1;
		buf[*ژمارە] = ichar;
		putnstr(buf + *ژمارە, wlen);
		(*ژمارە)++;
	}
}

ساكن فراغ cread_add_str(حرف *str, صحيح strsize, صحيح زیادکردن,
			  uint *ژمارە, uint *eol_ژمارە, حرف *buf, uint len)
{
	طالما (strsize--) {
		cread_add_char(*str, زیادکردن, ژمارە, eol_ژمارە, buf, len);
		str++;
	}
}

صحيح cread_هێڵ_process_ch(بنية cli_هێڵ_state *cls, حرف ichar)
{
	حرف *buf = cls->buf;

	/* ichar=0x0 when error occurs in U-Boot getc */
	إذا (!ichar)
		أرجع -EAGAIN;

	إذا (ichar == '\n') {
		putc('\n');
		buf[cls->eol_ژمارە] = '\0';	/* terminate the string */
		أرجع 0;
	}

	بدل (ichar) {
	حالة CTL_CH('a'):
		BEGINNING_OF_هێڵ();
		اكسر;
	حالة CTL_CH('c'):	/* ^C - break */
		*buf = '\0';	/* discard input */
		أرجع -EINTR;
	حالة CTL_CH('f'):
		إذا (cls->ژمارە < cls->eol_ژمارە) {
			getcmd_putch(buf[cls->ژمارە]);
			cls->ژمارە++;
		}
		اكسر;
	حالة CTL_CH('b'):
		إذا (cls->ژمارە) {
			getcmd_putch(CTL_BACKSPACE);
			cls->ژمارە--;
		}
		اكسر;
	حالة CTL_CH('d'):
		إذا (cls->ژمارە < cls->eol_ژمارە) {
			uint wlen;

			wlen = cls->eol_ژمارە - cls->ژمارە - 1;
			إذا (wlen) {
				memmove(&buf[cls->ژمارە], &buf[cls->ژمارە + 1],
					wlen);
				putnstr(buf + cls->ژمارە, wlen);
			}

			getcmd_putch(' ');
			افعل {
				getcmd_putch(CTL_BACKSPACE);
			} طالما (wlen--);
			cls->eol_ژمارە--;
		}
		اكسر;
	حالة CTL_CH('k'):
		ERASE_TO_EOL();
		اكسر;
	حالة CTL_CH('e'):
		REFRESH_TO_EOL();
		اكسر;
	حالة CTL_CH('o'):
		cls->زیادکردن = !cls->زیادکردن;
		اكسر;
	حالة CTL_CH('w'):
		إذا (cls->ژمارە) {
			uint base, wlen;

			لكل (base = cls->ژمارە;
			     base > 0 && buf[base - 1] == ' ';)
				base--;
			لكل (; base > 0 && buf[base - 1] != ' ';)
				base--;

			/* now delete chars from base to cls->num */
			wlen = cls->ژمارە - base;
			cls->eol_ژمارە -= wlen;
			memmove(&buf[base], &buf[cls->ژمارە],
				cls->eol_ژمارە - base + 1);
			cls->ژمارە = base;
			getcmd_putchars(wlen, CTL_BACKSPACE);
			puts(buf + base);
			getcmd_putchars(wlen, ' ');
			getcmd_putchars(wlen + cls->eol_ژمارە - cls->ژمارە,
					CTL_BACKSPACE);
		}
		اكسر;
	حالة CTL_CH('x'):
	حالة CTL_CH('u'):
		BEGINNING_OF_هێڵ();
		ERASE_TO_EOL();
		اكسر;
	حالة DEL:
	حالة DEL7:
	حالة 8:
		إذا (cls->ژمارە) {
			uint wlen;

			wlen = cls->eol_ژمارە - cls->ژمارە;
			cls->ژمارە--;
			memmove(&buf[cls->ژمارە], &buf[cls->ژمارە + 1], wlen);
			getcmd_putch(CTL_BACKSPACE);
			putnstr(buf + cls->ژمارە, wlen);
			getcmd_putch(' ');
			افعل {
				getcmd_putch(CTL_BACKSPACE);
			} طالما (wlen--);
			cls->eol_ژمارە--;
		}
		اكسر;
	حالة CTL_CH('p'):
	حالة CTL_CH('n'):
		إذا (cls->history) {
			حرف *hline;

			إذا (ichar == CTL_CH('p'))
				hline = hist_prev();
			وإلا
				hline = hist_next();

			إذا (!hline) {
				getcmd_cbeep();
				اكسر;
			}

			/* nuke the current line */
			/* first, go home */
			BEGINNING_OF_هێڵ();

			/* erase to end of line */
			ERASE_TO_EOL();

			/* copy new line into place and display */
			strcpy(buf, hline);
			cls->eol_ژمارە = strlen(buf);
			REFRESH_TO_EOL();
			اكسر;
		}
		اكسر;
	حالة '\t':
		إذا (IS_ENABLED(CONFIG_AUTO_COMPLETE) && cls->cmd_complete) {
			صحيح num2, col;

			/* do not autocomplete when in the middle */
			إذا (cls->ژمارە < cls->eol_ژمارە) {
				getcmd_cbeep();
				اكسر;
			}

			buf[cls->ژمارە] = '\0';
			col = strlen(cls->prompt) + cls->eol_ژمارە;
			num2 = cls->ژمارە;
			إذا (cmd_auto_complete(cls->prompt, buf, &num2, &col)) {
				col = num2 - cls->ژمارە;
				cls->ژمارە += col;
				cls->eol_ژمارە += col;
			}
			اكسر;
		}
		fallthrough;
	افتراضي:
		cread_add_char(ichar, cls->زیادکردن, &cls->ژمارە, &cls->eol_ژمارە,
			       buf, cls->len);
		اكسر;
	}

	/*
	 * keep the string terminated...if we added a char at the end then we
	 * want a \0 after it
	 */
	buf[cls->eol_ژمارە] = '\0';

	أرجع -EAGAIN;
}

فراغ cli_cread_init(بنية cli_هێڵ_state *cls, حرف *buf, uint buf_size)
{
	صحيح init_len = strlen(buf);

	memset(cls, '\0', حجم(بنية cli_هێڵ_state));
	cls->زیادکردن = true;
	cls->buf = buf;
	cls->len = buf_size;

	إذا (init_len)
		cread_add_str(buf, init_len, 0, &cls->ژمارە, &cls->eol_ژمارە, buf,
			      buf_size);
}

ساكن صحيح cread_هێڵ(ثابت حرف *ثابت prompt, حرف *buf, unsigned صحيح *len,
		      صحيح timeout)
{
	بنية cli_ch_state s_cch, *cch = &s_cch;
	بنية cli_هێڵ_state s_cls, *cls = &s_cls;
	حرف ichar;
	صحيح first = 1;

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
			إذا (first && timeout) {
				u64 etime = endtick(timeout);

				طالما (!tstc()) {	/* while no incoming data */
					إذا (get_ticks() >= etime)
						أرجع -2;	/* timed out */
					schedule();
				}
				first = 0;
			}

			ichar = getcmd_getch();
			ichar = cli_ch_process(cch, ichar);
		}

		ret = cread_هێڵ_process_ch(cls, ichar);
		إذا (ret == -EINTR)
			أرجع -1;
		وإلا إذا (!ret)
			اكسر;
	}
	*len = cls->eol_ژمارە;

	إذا (buf[0] && buf[0] != CREAD_HIST_CHAR)
		cread_add_to_hist(buf);
	hist_cur = hist_add_idx;

	أرجع 0;
}

#وإلا /* !CONFIG_CMDLINE_EDITING */

ساكن مضمن صحيح hist_init(فراغ)
{
	أرجع 0;
}

ساكن صحيح cread_هێڵ(ثابت حرف *ثابت prompt, حرف *buf, unsigned صحيح *len,
		      صحيح timeout)
{
	أرجع 0;
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

	أرجع cli_readline_into_buffer(prompt, console_buffer, 0);
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
ساكن صحيح cread_هێڵ_simple(ثابت حرف *ثابت prompt, حرف *p)
{
	حرف *p_buf = p;
	صحيح n = 0;		/* buffer index */
	صحيح plen = 0;		/* prompt length */
	صحيح col;		/* output column cnt */
	صحيح c;

	/* print prompt */
	إذا (prompt) {
		plen = strlen(prompt);
		puts(prompt);
	}
	col = plen;

	لكل (;;) {
		إذا (bootretry_tstc_timeout())
			أرجع -2;	/* timed out */
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
			أرجع p - p_buf;

		حالة '\0':			/* nul			*/
			استمر;

		حالة 0x03:			/* ^C - break		*/
			p_buf[0] = '\0';	/* discard input */
			أرجع -1;

		حالة 0x15:			/* ^U - erase line	*/
			طالما (col > plen) {
				puts(erase_seq);
				--col;
			}
			p = p_buf;
			n = 0;
			استمر;

		حالة 0x17:			/* ^W - erase word	*/
			p = delete_char(p_buf, p, &col, &n, plen);
			طالما ((n > 0) && (*p != ' '))
				p = delete_char(p_buf, p, &col, &n, plen);
			استمر;

		حالة 0x08:			/* ^H  - backspace	*/
		حالة 0x7F:			/* DEL - backspace	*/
			p = delete_char(p_buf, p, &col, &n, plen);
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
							      console_buffer,
							      &n, &col)) {
						p = p_buf + n;	/* reset */
						استمر;
					}
				}
				puts(tab_seq + (col & 07));
				col += 8 - (col & 07);
			} وإلا {
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
	إذا (IS_ENABLED(CONFIG_CMDLINE_EDITING) && (gd->flags & GD_FLG_RELOC)) {
		إذا (!initted) {
			rc = hist_init();
			إذا (rc == 0)
				initted = 1;
		}

		إذا (prompt)
			puts(prompt);

		rc = cread_هێڵ(prompt, p, &len, timeout);
		أرجع rc < 0 ? rc : len;

	} وإلا {
		أرجع cread_هێڵ_simple(prompt, p);
	}
}
