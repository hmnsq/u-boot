// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Add to readline cmdline-editing by
 * (C) Copyright 2005
 * JinHua Luo, GuangDong Linux Center, <luo.jinhua@gd-linux.com>
 */

#అన్తర్భావ <bootretry.h>
#అన్తర్భావ <cli.h>
#అన్తర్భావ <command.h>
#అన్తర్భావ <hang.h>
#అన్తర్భావ <malloc.h>
#అన్తర్భావ <time.h>
#అన్తర్భావ <watchdog.h>
#అన్తర్భావ <linux/errno.h>
#అన్తర్భావ <asm/global_data.h>

DECLARE_GLOBAL_దత్తాంశం_PTR;

స్థిర స్థిరరాశి అక్షరం erase_seq[] = "\b \b";	/* erase sequence */
స్థిర స్థిరరాశి అక్షరం   tab_seq[] = "        ";	/* used to expand TABs */

అక్షరం console_బఫరు[CONFIG_SYS_CBSIZE + 1];	/* console I/O buffer	*/

స్థిర అక్షరం *delete_char (అక్షరం *బఫరు, అక్షరం *p, పూర్ణాంకం *colp, పూర్ణాంకం *np, పూర్ణాంకం plen)
{
	అక్షరం *s;

	ఒకవేళ (*np == 0)
		తిరిగి p;

	ఒకవేళ (*(--p) == '\t') {		/* will retype the whole line */
		అయితే (*colp > plen) {
			puts(erase_seq);
			(*colp)--;
		}
		కొరకు (s = బఫరు; s < p; ++s) {
			ఒకవేళ (*s == '\t') {
				puts(tab_seq + ((*colp) & 07));
				*colp += 8 - ((*colp) & 07);
			} లేకపోతే {
				++(*colp);
				putc(*s);
			}
		}
	} లేకపోతే {
		puts(erase_seq);
		(*colp)--;
	}
	(*np)--;

	తిరిగి p;
}

#నిర్వచితమైతే CONFIG_CMDLINE_EDITING

/*
 * cmdline-editing related codes from vivi.
 * Author: Janghoon Lyu <nandy@mizi.com>
 */

#నిర్వచించు putnstr(str, n)	రాయి("%.*s", (పూర్ణాంకం)n, str)

#నిర్వచించు CTL_BACKSPACE		('\b')
#నిర్వచించు DEL			((అక్షరం)255)
#నిర్వచించు DEL7			((అక్షరం)127)
#నిర్వచించు CREAD_HIST_CHAR		('!')

#నిర్వచించు getcmd_putch(ch)	putc(ch)
#నిర్వచించు getcmd_getch()		getchar()
#నిర్వచించు getcmd_cbeep()		getcmd_putch('\a')

#నిర్వచితమైతే CONFIG_XPL_BUILD
#నిర్వచించు HIST_MAX		3
#నిర్వచించు HIST_కొలత		32
#లేకపోతే
#నిర్వచించు HIST_MAX		20
#నిర్వచించు HIST_కొలత		CONFIG_SYS_CBSIZE
#అయితే_ముగింపు

స్థిర పూర్ణాంకం hist_max;
స్థిర పూర్ణాంకం hist_కూడు_idx;
స్థిర పూర్ణాంకం hist_cur = -1;
స్థిర గుర్తులేని hist_అంకె;

#నిర్వచితం_కాకపోతే CONFIG_CMD_HISTORY_USE_CALLOC
స్థిర అక్షరం hist_దత్తాంశం[HIST_MAX][HIST_కొలత + 1];
#అయితే_ముగింపు
స్థిర అక్షరం *hist_list[HIST_MAX];

#నిర్వచించు కూడు_idx_minus_one() ((hist_కూడు_idx == 0) ? hist_max : hist_కూడు_idx-1)

స్థిర శూన్య getcmd_putchars(పూర్ణాంకం లెక్క, పూర్ణాంకం ch)
{
	పూర్ణాంకం i;

	కొరకు (i = 0; i < లెక్క; i++)
		getcmd_putch(ch);
}

స్థిర పూర్ణాంకం hist_init(శూన్య)
{
	పూర్ణాంకం i;

#నిర్వచితం_కాకపోతే CONFIG_CMD_HISTORY_USE_CALLOC
	కొరకు (i = 0; i < HIST_MAX; i++) {
		hist_list[i] = hist_దత్తాంశం[i];
		hist_list[i][0] = '\0';
	}
#లేకపోతే
	గుర్తులేని అక్షరం *hist = సున్నా_జ్ఞాపకం_ఇవ్వు(HIST_MAX, HIST_కొలత + 1);
	ఒకవేళ (!hist)
		panic("%s: calloc: out of memory!\n", __func__);

	కొరకు (i = 0; i < HIST_MAX; i++)
		hist_list[i] = hist + (i * (HIST_కొలత + 1));
#అయితే_ముగింపు

	hist_max = 0;
	hist_కూడు_idx = 0;
	hist_cur = -1;
	hist_అంకె = 0;

	తిరిగి 0;
}

స్థిర శూన్య cread_కూడు_గమ్యం_hist(అక్షరం *పంక్తి)
{
	strcpy(hist_list[hist_కూడు_idx], పంక్తి);

	ఒకవేళ (++hist_కూడు_idx >= HIST_MAX)
		hist_కూడు_idx = 0;

	ఒకవేళ (hist_కూడు_idx > hist_max)
		hist_max = hist_కూడు_idx;

	hist_అంకె++;
}

స్థిర అక్షరం *hist_prev(శూన్య)
{
	అక్షరం *ret;
	పూర్ణాంకం old_cur;

	ఒకవేళ (hist_cur < 0)
		తిరిగి ఖాళీ;

	old_cur = hist_cur;
	ఒకవేళ (--hist_cur < 0)
		hist_cur = hist_max;

	ఒకవేళ (hist_cur == hist_కూడు_idx) {
		hist_cur = old_cur;
		ret = ఖాళీ;
	} లేకపోతే {
		ret = hist_list[hist_cur];
	}

	తిరిగి ret;
}

స్థిర అక్షరం *hist_తదుపరి(శూన్య)
{
	అక్షరం *ret;

	ఒకవేళ (hist_cur < 0)
		తిరిగి ఖాళీ;

	ఒకవేళ (hist_cur == hist_కూడు_idx)
		తిరిగి ఖాళీ;

	ఒకవేళ (++hist_cur > hist_max)
		hist_cur = 0;

	ఒకవేళ (hist_cur == hist_కూడు_idx)
		ret = "";
	లేకపోతే
		ret = hist_list[hist_cur];

	తిరిగి ret;
}

శూన్య cread_print_hist_list(శూన్య)
{
	పూర్ణాంకం i;
	uint n;

	n = hist_అంకె - hist_max;

	i = hist_కూడు_idx + 1;
	అయితే (1) {
		ఒకవేళ (i > hist_max)
			i = 0;
		ఒకవేళ (i == hist_కూడు_idx)
			ఆపు;
		రాయి("%s\n", hist_list[i]);
		n++;
		i++;
	}
}

#నిర్వచించు BEGINNING_యొక్క_పంక్తి() {			\
	అయితే (cls->అంకె) {			\
		getcmd_putch(CTL_BACKSPACE);	\
		cls->అంకె--;			\
	}					\
}

#నిర్వచించు ERASE_గమ్యం_EOL() {				\
	ఒకవేళ (cls->అంకె < cls->eol_అంకె) {		\
		రాయి("%*s", (పూర్ణాంకం)(cls->eol_అంకె - cls->అంకె), ""); \
		చేయి {					\
			getcmd_putch(CTL_BACKSPACE);	\
		} అయితే (--cls->eol_అంకె > cls->అంకె);	\
	}						\
}

#నిర్వచించు REFRESH_గమ్యం_EOL() {				\
	ఒకవేళ (cls->అంకె < cls->eol_అంకె) {			\
		uint wlen = cls->eol_అంకె - cls->అంకె;	\
		putnstr(buf + cls->అంకె, wlen);		\
		cls->అంకె = cls->eol_అంకె;		\
	}						\
}

స్థిర శూన్య cread_కూడు_char(అక్షరం ichar, పూర్ణాంకం చొప్పించు, uint *అంకె,
			   uint *eol_అంకె, అక్షరం *buf, uint len)
{
	uint wlen;

	/* room ??? */
	ఒకవేళ (చొప్పించు || *అంకె == *eol_అంకె) {
		ఒకవేళ (*eol_అంకె > len - 1) {
			getcmd_cbeep();
			తిరిగి;
		}
		(*eol_అంకె)++;
	}

	ఒకవేళ (చొప్పించు) {
		wlen = *eol_అంకె - *అంకె;
		ఒకవేళ (wlen > 1)
			memmove(&buf[*అంకె+1], &buf[*అంకె], wlen-1);

		buf[*అంకె] = ichar;
		putnstr(buf + *అంకె, wlen);
		(*అంకె)++;
		అయితే (--wlen)
			getcmd_putch(CTL_BACKSPACE);
	} లేకపోతే {
		/* echo the character */
		wlen = 1;
		buf[*అంకె] = ichar;
		putnstr(buf + *అంకె, wlen);
		(*అంకె)++;
	}
}

స్థిర శూన్య cread_కూడు_str(అక్షరం *str, పూర్ణాంకం strsize, పూర్ణాంకం చొప్పించు,
			  uint *అంకె, uint *eol_అంకె, అక్షరం *buf, uint len)
{
	అయితే (strsize--) {
		cread_కూడు_char(*str, చొప్పించు, అంకె, eol_అంకె, buf, len);
		str++;
	}
}

పూర్ణాంకం cread_పంక్తి_process_ch(నిర్మాణం cli_పంక్తి_state *cls, అక్షరం ichar)
{
	అక్షరం *buf = cls->buf;

	/* ichar=0x0 when error occurs in U-Boot getc */
	ఒకవేళ (!ichar)
		తిరిగి -EAGAIN;

	ఒకవేళ (ichar == '\n') {
		putc('\n');
		buf[cls->eol_అంకె] = '\0';	/* terminate the string */
		తిరిగి 0;
	}

	ఎంపిక (ichar) {
	స్థితి CTL_CH('a'):
		BEGINNING_యొక్క_పంక్తి();
		ఆపు;
	స్థితి CTL_CH('c'):	/* ^C - break */
		*buf = '\0';	/* discard input */
		తిరిగి -EINTR;
	స్థితి CTL_CH('f'):
		ఒకవేళ (cls->అంకె < cls->eol_అంకె) {
			getcmd_putch(buf[cls->అంకె]);
			cls->అంకె++;
		}
		ఆపు;
	స్థితి CTL_CH('b'):
		ఒకవేళ (cls->అంకె) {
			getcmd_putch(CTL_BACKSPACE);
			cls->అంకె--;
		}
		ఆపు;
	స్థితి CTL_CH('d'):
		ఒకవేళ (cls->అంకె < cls->eol_అంకె) {
			uint wlen;

			wlen = cls->eol_అంకె - cls->అంకె - 1;
			ఒకవేళ (wlen) {
				memmove(&buf[cls->అంకె], &buf[cls->అంకె + 1],
					wlen);
				putnstr(buf + cls->అంకె, wlen);
			}

			getcmd_putch(' ');
			చేయి {
				getcmd_putch(CTL_BACKSPACE);
			} అయితే (wlen--);
			cls->eol_అంకె--;
		}
		ఆపు;
	స్థితి CTL_CH('k'):
		ERASE_గమ్యం_EOL();
		ఆపు;
	స్థితి CTL_CH('e'):
		REFRESH_గమ్యం_EOL();
		ఆపు;
	స్థితి CTL_CH('o'):
		cls->చొప్పించు = !cls->చొప్పించు;
		ఆపు;
	స్థితి CTL_CH('w'):
		ఒకవేళ (cls->అంకె) {
			uint పునాది, wlen;

			కొరకు (పునాది = cls->అంకె;
			     పునాది > 0 && buf[పునాది - 1] == ' ';)
				పునాది--;
			కొరకు (; పునాది > 0 && buf[పునాది - 1] != ' ';)
				పునాది--;

			/* now delete chars from base to cls->num */
			wlen = cls->అంకె - పునాది;
			cls->eol_అంకె -= wlen;
			memmove(&buf[పునాది], &buf[cls->అంకె],
				cls->eol_అంకె - పునాది + 1);
			cls->అంకె = పునాది;
			getcmd_putchars(wlen, CTL_BACKSPACE);
			puts(buf + పునాది);
			getcmd_putchars(wlen, ' ');
			getcmd_putchars(wlen + cls->eol_అంకె - cls->అంకె,
					CTL_BACKSPACE);
		}
		ఆపు;
	స్థితి CTL_CH('x'):
	స్థితి CTL_CH('u'):
		BEGINNING_యొక్క_పంక్తి();
		ERASE_గమ్యం_EOL();
		ఆపు;
	స్థితి DEL:
	స్థితి DEL7:
	స్థితి 8:
		ఒకవేళ (cls->అంకె) {
			uint wlen;

			wlen = cls->eol_అంకె - cls->అంకె;
			cls->అంకె--;
			memmove(&buf[cls->అంకె], &buf[cls->అంకె + 1], wlen);
			getcmd_putch(CTL_BACKSPACE);
			putnstr(buf + cls->అంకె, wlen);
			getcmd_putch(' ');
			చేయి {
				getcmd_putch(CTL_BACKSPACE);
			} అయితే (wlen--);
			cls->eol_అంకె--;
		}
		ఆపు;
	స్థితి CTL_CH('p'):
	స్థితి CTL_CH('n'):
		ఒకవేళ (cls->history) {
			అక్షరం *hline;

			ఒకవేళ (ichar == CTL_CH('p'))
				hline = hist_prev();
			లేకపోతే
				hline = hist_తదుపరి();

			ఒకవేళ (!hline) {
				getcmd_cbeep();
				ఆపు;
			}

			/* nuke the current line */
			/* first, go home */
			BEGINNING_యొక్క_పంక్తి();

			/* erase to end of line */
			ERASE_గమ్యం_EOL();

			/* copy new line into place and display */
			strcpy(buf, hline);
			cls->eol_అంకె = అక్షరశ్రేణి_పొడవు(buf);
			REFRESH_గమ్యం_EOL();
			ఆపు;
		}
		ఆపు;
	స్థితి '\t':
		ఒకవేళ (IS_ENABLED(CONFIG_AUTO_COMPLETE) && cls->cmd_complete) {
			పూర్ణాంకం num2, నిలువు;

			/* do not autocomplete when in the middle */
			ఒకవేళ (cls->అంకె < cls->eol_అంకె) {
				getcmd_cbeep();
				ఆపు;
			}

			buf[cls->అంకె] = '\0';
			నిలువు = అక్షరశ్రేణి_పొడవు(cls->prompt) + cls->eol_అంకె;
			num2 = cls->అంకె;
			ఒకవేళ (cmd_auto_complete(cls->prompt, buf, &num2, &నిలువు)) {
				నిలువు = num2 - cls->అంకె;
				cls->అంకె += నిలువు;
				cls->eol_అంకె += నిలువు;
			}
			ఆపు;
		}
		fallthrough;
	అప్రమేయ:
		cread_కూడు_char(ichar, cls->చొప్పించు, &cls->అంకె, &cls->eol_అంకె,
			       buf, cls->len);
		ఆపు;
	}

	/*
	 * keep the string terminated...if we added a char at the end then we
	 * want a \0 after it
	 */
	buf[cls->eol_అంకె] = '\0';

	తిరిగి -EAGAIN;
}

శూన్య cli_cread_init(నిర్మాణం cli_పంక్తి_state *cls, అక్షరం *buf, uint buf_కొలత)
{
	పూర్ణాంకం init_len = అక్షరశ్రేణి_పొడవు(buf);

	memset(cls, '\0', పరిమాణం(నిర్మాణం cli_పంక్తి_state));
	cls->చొప్పించు = true;
	cls->buf = buf;
	cls->len = buf_కొలత;

	ఒకవేళ (init_len)
		cread_కూడు_str(buf, init_len, 0, &cls->అంకె, &cls->eol_అంకె, buf,
			      buf_కొలత);
}

స్థిర పూర్ణాంకం cread_పంక్తి(స్థిరరాశి అక్షరం *స్థిరరాశి prompt, అక్షరం *buf, గుర్తులేని పూర్ణాంకం *len,
		      పూర్ణాంకం timeout)
{
	నిర్మాణం cli_ch_state s_cch, *cch = &s_cch;
	నిర్మాణం cli_పంక్తి_state s_cls, *cls = &s_cls;
	అక్షరం ichar;
	పూర్ణాంకం మొదటి = 1;

	cli_ch_init(cch);
	cli_cread_init(cls, buf, *len);
	cls->prompt = prompt;
	cls->history = true;
	cls->cmd_complete = true;

	అయితే (1) {
		పూర్ణాంకం ret;

		/* Check for saved characters */
		ichar = cli_ch_process(cch, 0);

		ఒకవేళ (!ichar) {
			ఒకవేళ (bootretry_tstc_timeout())
				తిరిగి -2;	/* timed out */
			ఒకవేళ (మొదటి && timeout) {
				u64 etime = endtick(timeout);

				అయితే (!tstc()) {	/* while no incoming data */
					ఒకవేళ (get_ticks() >= etime)
						తిరిగి -2;	/* timed out */
					కాలపట్టిక();
				}
				మొదటి = 0;
			}

			ichar = getcmd_getch();
			ichar = cli_ch_process(cch, ichar);
		}

		ret = cread_పంక్తి_process_ch(cls, ichar);
		ఒకవేళ (ret == -EINTR)
			తిరిగి -1;
		లేకపోతే ఒకవేళ (!ret)
			ఆపు;
	}
	*len = cls->eol_అంకె;

	ఒకవేళ (buf[0] && buf[0] != CREAD_HIST_CHAR)
		cread_కూడు_గమ్యం_hist(buf);
	hist_cur = hist_కూడు_idx;

	తిరిగి 0;
}

#లేకపోతే /* !CONFIG_CMDLINE_EDITING */

స్థిర అంతఃవరుస పూర్ణాంకం hist_init(శూన్య)
{
	తిరిగి 0;
}

స్థిర పూర్ణాంకం cread_పంక్తి(స్థిరరాశి అక్షరం *స్థిరరాశి prompt, అక్షరం *buf, గుర్తులేని పూర్ణాంకం *len,
		      పూర్ణాంకం timeout)
{
	తిరిగి 0;
}

#అయితే_ముగింపు /* CONFIG_CMDLINE_EDITING */

/****************************************************************************/

పూర్ణాంకం cli_readline(స్థిరరాశి అక్షరం *స్థిరరాశి prompt)
{
	/*
	 * If console_buffer isn't 0-length the user will be prompted to modify
	 * it instead of entering it from scratch as desired.
	 */
	console_బఫరు[0] = '\0';

	తిరిగి cli_readline_into_బఫరు(prompt, console_బఫరు, 0);
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
స్థిర పూర్ణాంకం cread_పంక్తి_simple(స్థిరరాశి అక్షరం *స్థిరరాశి prompt, అక్షరం *p)
{
	అక్షరం *p_buf = p;
	పూర్ణాంకం n = 0;		/* buffer index */
	పూర్ణాంకం plen = 0;		/* prompt length */
	పూర్ణాంకం నిలువు;		/* output column cnt */
	పూర్ణాంకం c;

	/* print prompt */
	ఒకవేళ (prompt) {
		plen = అక్షరశ్రేణి_పొడవు(prompt);
		puts(prompt);
	}
	నిలువు = plen;

	కొరకు (;;) {
		ఒకవేళ (bootretry_tstc_timeout())
			తిరిగి -2;	/* timed out */
		కాలపట్టిక();	/* Trigger watchdog, if needed */

		c = getchar();

		/*
		 * Special character handling
		 */
		ఎంపిక (c) {
		స్థితి '\r':			/* Enter		*/
		స్థితి '\n':
			*p = '\0';
			puts("\r\n");
			తిరిగి p - p_buf;

		స్థితి '\0':			/* nul			*/
			కొనసాగు;

		స్థితి 0x03:			/* ^C - break		*/
			p_buf[0] = '\0';	/* discard input */
			తిరిగి -1;

		స్థితి 0x15:			/* ^U - erase line	*/
			అయితే (నిలువు > plen) {
				puts(erase_seq);
				--నిలువు;
			}
			p = p_buf;
			n = 0;
			కొనసాగు;

		స్థితి 0x17:			/* ^W - erase word	*/
			p = delete_char(p_buf, p, &నిలువు, &n, plen);
			అయితే ((n > 0) && (*p != ' '))
				p = delete_char(p_buf, p, &నిలువు, &n, plen);
			కొనసాగు;

		స్థితి 0x08:			/* ^H  - backspace	*/
		స్థితి 0x7F:			/* DEL - backspace	*/
			p = delete_char(p_buf, p, &నిలువు, &n, plen);
			కొనసాగు;

		అప్రమేయ:
			/* Must be a normal character then */
			ఒకవేళ (n >= CONFIG_SYS_CBSIZE - 2) { /* Buffer full */
				putc('\a');
				ఆపు;
			}
			ఒకవేళ (c == '\t') {	/* expand TABs */
				ఒకవేళ (IS_ENABLED(CONFIG_AUTO_COMPLETE)) {
					/*
					 * if auto-completion triggered just
					 * continue
					 */
					*p = '\0';
					ఒకవేళ (cmd_auto_complete(prompt,
							      console_బఫరు,
							      &n, &నిలువు)) {
						p = p_buf + n;	/* reset */
						కొనసాగు;
					}
				}
				puts(tab_seq + (నిలువు & 07));
				నిలువు += 8 - (నిలువు & 07);
			} లేకపోతే {
				అక్షరం __maybe_unused buf[2];

				/*
				 * Echo input using puts() to force an LCD
				 * flush if we are using an LCD
				 */
				++నిలువు;
				buf[0] = c;
				buf[1] = '\0';
				puts(buf);
			}
			*p++ = c;
			++n;
			ఆపు;
		}
	}
}

పూర్ణాంకం cli_readline_into_బఫరు(స్థిరరాశి అక్షరం *స్థిరరాశి prompt, అక్షరం *బఫరు,
			     పూర్ణాంకం timeout)
{
	అక్షరం *p = బఫరు;
	uint len = CONFIG_SYS_CBSIZE;
	పూర్ణాంకం rc;
	స్థిర పూర్ణాంకం initted;

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
	ఒకవేళ (IS_ENABLED(CONFIG_CMDLINE_EDITING) && (gd->flags & GD_FLG_RELOC)) {
		ఒకవేళ (!initted) {
			rc = hist_init();
			ఒకవేళ (rc == 0)
				initted = 1;
		}

		ఒకవేళ (prompt)
			puts(prompt);

		rc = cread_పంక్తి(prompt, p, &len, timeout);
		తిరిగి rc < 0 ? rc : len;

	} లేకపోతే {
		తిరిగి cread_పంక్తి_simple(prompt, p);
	}
}
