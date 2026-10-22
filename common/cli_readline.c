// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Add to readline cmdline-editing by
 * (C) Copyright 2005
 * JinHua Luo, GuangDong Linux Center, <luo.jinhua@gd-linux.com>
 */

#अन्तर्भाव <bootretry.h>
#अन्तर्भाव <cli.h>
#अन्तर्भाव <command.h>
#अन्तर्भाव <hang.h>
#अन्तर्भाव <malloc.h>
#अन्तर्भाव <time.h>
#अन्तर्भाव <watchdog.h>
#अन्तर्भाव <linux/errno.h>
#अन्तर्भाव <asm/global_data.h>

DECLARE_GLOBAL_दत्त_PTR;

स्थिर ध्रुव वर्ण erase_seq[] = "\b \b";	/* erase sequence */
स्थिर ध्रुव वर्ण   tab_seq[] = "        ";	/* used to expand TABs */

वर्ण console_बफर[CONFIG_SYS_CBSIZE + 1];	/* console I/O buffer	*/

स्थिर वर्ण *delete_char (वर्ण *बफर, वर्ण *p, पूर्णाङ्क *colp, पूर्णाङ्क *np, पूर्णाङ्क plen)
{
	वर्ण *s;

	यदि (*np == 0)
		प्रत्यागम p;

	यदि (*(--p) == '\t') {		/* will retype the whole line */
		यावत् (*colp > plen) {
			पङ्क्तिं_लिख(erase_seq);
			(*colp)--;
		}
		प्रति (s = बफर; s < p; ++s) {
			यदि (*s == '\t') {
				पङ्क्तिं_लिख(tab_seq + ((*colp) & 07));
				*colp += 8 - ((*colp) & 07);
			} अन्यथा {
				++(*colp);
				putc(*s);
			}
		}
	} अन्यथा {
		पङ्क्तिं_लिख(erase_seq);
		(*colp)--;
	}
	(*np)--;

	प्रत्यागम p;
}

#यदि_परिभाषितम् CONFIG_CMDLINE_EDITING

/*
 * cmdline-editing related codes from vivi.
 * Author: Janghoon Lyu <nandy@mizi.com>
 */

#परिभाषय putnstr(str, n)	प्रारूप_लिख("%.*s", (पूर्णाङ्क)n, str)

#परिभाषय CTL_BACKSPACE		('\b')
#परिभाषय DEL			((वर्ण)255)
#परिभाषय DEL7			((वर्ण)127)
#परिभाषय CREAD_HIST_CHAR		('!')

#परिभाषय getcmd_putch(ch)	putc(ch)
#परिभाषय getcmd_getch()		अक्षरं_पठ()
#परिभाषय getcmd_cbeep()		getcmd_putch('\a')

#यदि_परिभाषितम् CONFIG_XPL_BUILD
#परिभाषय HIST_MAX		3
#परिभाषय HIST_आकार		32
#अन्यथा
#परिभाषय HIST_MAX		20
#परिभाषय HIST_आकार		CONFIG_SYS_CBSIZE
#यदि_अन्तः

स्थिर पूर्णाङ्क hist_max;
स्थिर पूर्णाङ्क hist_सङ्कलय_idx;
स्थिर पूर्णाङ्क hist_cur = -1;
स्थिर अचिह्नित hist_अङ्कः;

#यदि_अपरिभाषितम् CONFIG_CMD_HISTORY_USE_CALLOC
स्थिर वर्ण hist_दत्त[HIST_MAX][HIST_आकार + 1];
#यदि_अन्तः
स्थिर वर्ण *hist_list[HIST_MAX];

#परिभाषय सङ्कलय_idx_minus_one() ((hist_सङ्कलय_idx == 0) ? hist_max : hist_सङ्कलय_idx-1)

स्थिर शून्य getcmd_putchars(पूर्णाङ्क गणनम्, पूर्णाङ्क ch)
{
	पूर्णाङ्क i;

	प्रति (i = 0; i < गणनम्; i++)
		getcmd_putch(ch);
}

स्थिर पूर्णाङ्क hist_init(शून्य)
{
	पूर्णाङ्क i;

#यदि_अपरिभाषितम् CONFIG_CMD_HISTORY_USE_CALLOC
	प्रति (i = 0; i < HIST_MAX; i++) {
		hist_list[i] = hist_दत्त[i];
		hist_list[i][0] = '\0';
	}
#अन्यथा
	अचिह्नित वर्ण *hist = शून्यस्मृतिं_देहि(HIST_MAX, HIST_आकार + 1);
	यदि (!hist)
		panic("%s: calloc: out of memory!\n", __func__);

	प्रति (i = 0; i < HIST_MAX; i++)
		hist_list[i] = hist + (i * (HIST_आकार + 1));
#यदि_अन्तः

	hist_max = 0;
	hist_सङ्कलय_idx = 0;
	hist_cur = -1;
	hist_अङ्कः = 0;

	प्रत्यागम 0;
}

स्थिर शून्य cread_सङ्कलय_गन्तव्यम्_hist(वर्ण *पङ्क्तिः)
{
	वाक्यं_प्रतिलिख(hist_list[hist_सङ्कलय_idx], पङ्क्तिः);

	यदि (++hist_सङ्कलय_idx >= HIST_MAX)
		hist_सङ्कलय_idx = 0;

	यदि (hist_सङ्कलय_idx > hist_max)
		hist_max = hist_सङ्कलय_idx;

	hist_अङ्कः++;
}

स्थिर वर्ण *hist_पूर्व(शून्य)
{
	वर्ण *ret;
	पूर्णाङ्क old_cur;

	यदि (hist_cur < 0)
		प्रत्यागम रिक्त;

	old_cur = hist_cur;
	यदि (--hist_cur < 0)
		hist_cur = hist_max;

	यदि (hist_cur == hist_सङ्कलय_idx) {
		hist_cur = old_cur;
		ret = रिक्त;
	} अन्यथा {
		ret = hist_list[hist_cur];
	}

	प्रत्यागम ret;
}

स्थिर वर्ण *hist_अग्र(शून्य)
{
	वर्ण *ret;

	यदि (hist_cur < 0)
		प्रत्यागम रिक्त;

	यदि (hist_cur == hist_सङ्कलय_idx)
		प्रत्यागम रिक्त;

	यदि (++hist_cur > hist_max)
		hist_cur = 0;

	यदि (hist_cur == hist_सङ्कलय_idx)
		ret = "";
	अन्यथा
		ret = hist_list[hist_cur];

	प्रत्यागम ret;
}

शून्य cread_print_hist_list(शून्य)
{
	पूर्णाङ्क i;
	uint n;

	n = hist_अङ्कः - hist_max;

	i = hist_सङ्कलय_idx + 1;
	यावत् (1) {
		यदि (i > hist_max)
			i = 0;
		यदि (i == hist_सङ्कलय_idx)
			भङ्ग;
		प्रारूप_लिख("%s\n", hist_list[i]);
		n++;
		i++;
	}
}

#परिभाषय BEGINNING_सम्बन्धि_पङ्क्तिः() {			\
	यावत् (cls->अङ्कः) {			\
		getcmd_putch(CTL_BACKSPACE);	\
		cls->अङ्कः--;			\
	}					\
}

#परिभाषय ERASE_गन्तव्यम्_EOL() {				\
	यदि (cls->अङ्कः < cls->eol_अङ्कः) {		\
		प्रारूप_लिख("%*s", (पूर्णाङ्क)(cls->eol_अङ्कः - cls->अङ्कः), ""); \
		कुरु {					\
			getcmd_putch(CTL_BACKSPACE);	\
		} यावत् (--cls->eol_अङ्कः > cls->अङ्कः);	\
	}						\
}

#परिभाषय REFRESH_गन्तव्यम्_EOL() {				\
	यदि (cls->अङ्कः < cls->eol_अङ्कः) {			\
		uint wlen = cls->eol_अङ्कः - cls->अङ्कः;	\
		putnstr(buf + cls->अङ्कः, wlen);		\
		cls->अङ्कः = cls->eol_अङ्कः;		\
	}						\
}

स्थिर शून्य cread_सङ्कलय_char(वर्ण ichar, पूर्णाङ्क निवेशय, uint *अङ्कः,
			   uint *eol_अङ्कः, वर्ण *buf, uint len)
{
	uint wlen;

	/* room ??? */
	यदि (निवेशय || *अङ्कः == *eol_अङ्कः) {
		यदि (*eol_अङ्कः > len - 1) {
			getcmd_cbeep();
			प्रत्यागम;
		}
		(*eol_अङ्कः)++;
	}

	यदि (निवेशय) {
		wlen = *eol_अङ्कः - *अङ्कः;
		यदि (wlen > 1)
			memmove(&buf[*अङ्कः+1], &buf[*अङ्कः], wlen-1);

		buf[*अङ्कः] = ichar;
		putnstr(buf + *अङ्कः, wlen);
		(*अङ्कः)++;
		यावत् (--wlen)
			getcmd_putch(CTL_BACKSPACE);
	} अन्यथा {
		/* echo the character */
		wlen = 1;
		buf[*अङ्कः] = ichar;
		putnstr(buf + *अङ्कः, wlen);
		(*अङ्कः)++;
	}
}

स्थिर शून्य cread_सङ्कलय_str(वर्ण *str, पूर्णाङ्क strsize, पूर्णाङ्क निवेशय,
			  uint *अङ्कः, uint *eol_अङ्कः, वर्ण *buf, uint len)
{
	यावत् (strsize--) {
		cread_सङ्कलय_char(*str, निवेशय, अङ्कः, eol_अङ्कः, buf, len);
		str++;
	}
}

पूर्णाङ्क cread_पङ्क्तिः_process_ch(संरचना cli_पङ्क्तिः_state *cls, वर्ण ichar)
{
	वर्ण *buf = cls->buf;

	/* ichar=0x0 when error occurs in U-Boot getc */
	यदि (!ichar)
		प्रत्यागम -EAGAIN;

	यदि (ichar == '\n') {
		putc('\n');
		buf[cls->eol_अङ्कः] = '\0';	/* terminate the string */
		प्रत्यागम 0;
	}

	विकल्प (ichar) {
	पक्ष CTL_CH('a'):
		BEGINNING_सम्बन्धि_पङ्क्तिः();
		भङ्ग;
	पक्ष CTL_CH('c'):	/* ^C - break */
		*buf = '\0';	/* discard input */
		प्रत्यागम -EINTR;
	पक्ष CTL_CH('f'):
		यदि (cls->अङ्कः < cls->eol_अङ्कः) {
			getcmd_putch(buf[cls->अङ्कः]);
			cls->अङ्कः++;
		}
		भङ्ग;
	पक्ष CTL_CH('b'):
		यदि (cls->अङ्कः) {
			getcmd_putch(CTL_BACKSPACE);
			cls->अङ्कः--;
		}
		भङ्ग;
	पक्ष CTL_CH('d'):
		यदि (cls->अङ्कः < cls->eol_अङ्कः) {
			uint wlen;

			wlen = cls->eol_अङ्कः - cls->अङ्कः - 1;
			यदि (wlen) {
				memmove(&buf[cls->अङ्कः], &buf[cls->अङ्कः + 1],
					wlen);
				putnstr(buf + cls->अङ्कः, wlen);
			}

			getcmd_putch(' ');
			कुरु {
				getcmd_putch(CTL_BACKSPACE);
			} यावत् (wlen--);
			cls->eol_अङ्कः--;
		}
		भङ्ग;
	पक्ष CTL_CH('k'):
		ERASE_गन्तव्यम्_EOL();
		भङ्ग;
	पक्ष CTL_CH('e'):
		REFRESH_गन्तव्यम्_EOL();
		भङ्ग;
	पक्ष CTL_CH('o'):
		cls->निवेशय = !cls->निवेशय;
		भङ्ग;
	पक्ष CTL_CH('w'):
		यदि (cls->अङ्कः) {
			uint आधारः, wlen;

			प्रति (आधारः = cls->अङ्कः;
			     आधारः > 0 && buf[आधारः - 1] == ' ';)
				आधारः--;
			प्रति (; आधारः > 0 && buf[आधारः - 1] != ' ';)
				आधारः--;

			/* now delete chars from base to cls->num */
			wlen = cls->अङ्कः - आधारः;
			cls->eol_अङ्कः -= wlen;
			memmove(&buf[आधारः], &buf[cls->अङ्कः],
				cls->eol_अङ्कः - आधारः + 1);
			cls->अङ्कः = आधारः;
			getcmd_putchars(wlen, CTL_BACKSPACE);
			पङ्क्तिं_लिख(buf + आधारः);
			getcmd_putchars(wlen, ' ');
			getcmd_putchars(wlen + cls->eol_अङ्कः - cls->अङ्कः,
					CTL_BACKSPACE);
		}
		भङ्ग;
	पक्ष CTL_CH('x'):
	पक्ष CTL_CH('u'):
		BEGINNING_सम्बन्धि_पङ्क्तिः();
		ERASE_गन्तव्यम्_EOL();
		भङ्ग;
	पक्ष DEL:
	पक्ष DEL7:
	पक्ष 8:
		यदि (cls->अङ्कः) {
			uint wlen;

			wlen = cls->eol_अङ्कः - cls->अङ्कः;
			cls->अङ्कः--;
			memmove(&buf[cls->अङ्कः], &buf[cls->अङ्कः + 1], wlen);
			getcmd_putch(CTL_BACKSPACE);
			putnstr(buf + cls->अङ्कः, wlen);
			getcmd_putch(' ');
			कुरु {
				getcmd_putch(CTL_BACKSPACE);
			} यावत् (wlen--);
			cls->eol_अङ्कः--;
		}
		भङ्ग;
	पक्ष CTL_CH('p'):
	पक्ष CTL_CH('n'):
		यदि (cls->history) {
			वर्ण *hline;

			यदि (ichar == CTL_CH('p'))
				hline = hist_पूर्व();
			अन्यथा
				hline = hist_अग्र();

			यदि (!hline) {
				getcmd_cbeep();
				भङ्ग;
			}

			/* nuke the current line */
			/* first, go home */
			BEGINNING_सम्बन्धि_पङ्क्तिः();

			/* erase to end of line */
			ERASE_गन्तव्यम्_EOL();

			/* copy new line into place and display */
			वाक्यं_प्रतिलिख(buf, hline);
			cls->eol_अङ्कः = वाक्यदैर्घ्यम्(buf);
			REFRESH_गन्तव्यम्_EOL();
			भङ्ग;
		}
		भङ्ग;
	पक्ष '\t':
		यदि (IS_ENABLED(CONFIG_AUTO_COMPLETE) && cls->cmd_complete) {
			पूर्णाङ्क num2, col;

			/* do not autocomplete when in the middle */
			यदि (cls->अङ्कः < cls->eol_अङ्कः) {
				getcmd_cbeep();
				भङ्ग;
			}

			buf[cls->अङ्कः] = '\0';
			col = वाक्यदैर्घ्यम्(cls->prompt) + cls->eol_अङ्कः;
			num2 = cls->अङ्कः;
			यदि (cmd_auto_complete(cls->prompt, buf, &num2, &col)) {
				col = num2 - cls->अङ्कः;
				cls->अङ्कः += col;
				cls->eol_अङ्कः += col;
			}
			भङ्ग;
		}
		fallthrough;
	स्वभाव:
		cread_सङ्कलय_char(ichar, cls->निवेशय, &cls->अङ्कः, &cls->eol_अङ्कः,
			       buf, cls->len);
		भङ्ग;
	}

	/*
	 * keep the string terminated...if we added a char at the end then we
	 * want a \0 after it
	 */
	buf[cls->eol_अङ्कः] = '\0';

	प्रत्यागम -EAGAIN;
}

शून्य cli_cread_init(संरचना cli_पङ्क्तिः_state *cls, वर्ण *buf, uint buf_आकार)
{
	पूर्णाङ्क init_len = वाक्यदैर्घ्यम्(buf);

	स्मृतिं_पूरय(cls, '\0', परिमाण(संरचना cli_पङ्क्तिः_state));
	cls->निवेशय = true;
	cls->buf = buf;
	cls->len = buf_आकार;

	यदि (init_len)
		cread_सङ्कलय_str(buf, init_len, 0, &cls->अङ्कः, &cls->eol_अङ्कः, buf,
			      buf_आकार);
}

स्थिर पूर्णाङ्क cread_पङ्क्तिः(ध्रुव वर्ण *ध्रुव prompt, वर्ण *buf, अचिह्नित पूर्णाङ्क *len,
		      पूर्णाङ्क timeout)
{
	संरचना cli_ch_state s_cch, *cch = &s_cch;
	संरचना cli_पङ्क्तिः_state s_cls, *cls = &s_cls;
	वर्ण ichar;
	पूर्णाङ्क first = 1;

	cli_ch_init(cch);
	cli_cread_init(cls, buf, *len);
	cls->prompt = prompt;
	cls->history = true;
	cls->cmd_complete = true;

	यावत् (1) {
		पूर्णाङ्क ret;

		/* Check for saved characters */
		ichar = cli_ch_process(cch, 0);

		यदि (!ichar) {
			यदि (bootretry_tstc_timeout())
				प्रत्यागम -2;	/* timed out */
			यदि (first && timeout) {
				u64 etime = endtick(timeout);

				यावत् (!tstc()) {	/* while no incoming data */
					यदि (get_ticks() >= etime)
						प्रत्यागम -2;	/* timed out */
					कालसारणी();
				}
				first = 0;
			}

			ichar = getcmd_getch();
			ichar = cli_ch_process(cch, ichar);
		}

		ret = cread_पङ्क्तिः_process_ch(cls, ichar);
		यदि (ret == -EINTR)
			प्रत्यागम -1;
		अन्यथा यदि (!ret)
			भङ्ग;
	}
	*len = cls->eol_अङ्कः;

	यदि (buf[0] && buf[0] != CREAD_HIST_CHAR)
		cread_सङ्कलय_गन्तव्यम्_hist(buf);
	hist_cur = hist_सङ्कलय_idx;

	प्रत्यागम 0;
}

#अन्यथा /* !CONFIG_CMDLINE_EDITING */

स्थिर अन्तःस्थ पूर्णाङ्क hist_init(शून्य)
{
	प्रत्यागम 0;
}

स्थिर पूर्णाङ्क cread_पङ्क्तिः(ध्रुव वर्ण *ध्रुव prompt, वर्ण *buf, अचिह्नित पूर्णाङ्क *len,
		      पूर्णाङ्क timeout)
{
	प्रत्यागम 0;
}

#यदि_अन्तः /* CONFIG_CMDLINE_EDITING */

/****************************************************************************/

पूर्णाङ्क cli_readline(ध्रुव वर्ण *ध्रुव prompt)
{
	/*
	 * If console_buffer isn't 0-length the user will be prompted to modify
	 * it instead of entering it from scratch as desired.
	 */
	console_बफर[0] = '\0';

	प्रत्यागम cli_readline_into_बफर(prompt, console_बफर, 0);
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
स्थिर पूर्णाङ्क cread_पङ्क्तिः_simple(ध्रुव वर्ण *ध्रुव prompt, वर्ण *p)
{
	वर्ण *p_buf = p;
	पूर्णाङ्क n = 0;		/* buffer index */
	पूर्णाङ्क plen = 0;		/* prompt length */
	पूर्णाङ्क col;		/* output column cnt */
	पूर्णाङ्क c;

	/* print prompt */
	यदि (prompt) {
		plen = वाक्यदैर्घ्यम्(prompt);
		पङ्क्तिं_लिख(prompt);
	}
	col = plen;

	प्रति (;;) {
		यदि (bootretry_tstc_timeout())
			प्रत्यागम -2;	/* timed out */
		कालसारणी();	/* Trigger watchdog, if needed */

		c = अक्षरं_पठ();

		/*
		 * Special character handling
		 */
		विकल्प (c) {
		पक्ष '\r':			/* Enter		*/
		पक्ष '\n':
			*p = '\0';
			पङ्क्तिं_लिख("\r\n");
			प्रत्यागम p - p_buf;

		पक्ष '\0':			/* nul			*/
			अनुवर्त;

		पक्ष 0x03:			/* ^C - break		*/
			p_buf[0] = '\0';	/* discard input */
			प्रत्यागम -1;

		पक्ष 0x15:			/* ^U - erase line	*/
			यावत् (col > plen) {
				पङ्क्तिं_लिख(erase_seq);
				--col;
			}
			p = p_buf;
			n = 0;
			अनुवर्त;

		पक्ष 0x17:			/* ^W - erase word	*/
			p = delete_char(p_buf, p, &col, &n, plen);
			यावत् ((n > 0) && (*p != ' '))
				p = delete_char(p_buf, p, &col, &n, plen);
			अनुवर्त;

		पक्ष 0x08:			/* ^H  - backspace	*/
		पक्ष 0x7F:			/* DEL - backspace	*/
			p = delete_char(p_buf, p, &col, &n, plen);
			अनुवर्त;

		स्वभाव:
			/* Must be a normal character then */
			यदि (n >= CONFIG_SYS_CBSIZE - 2) { /* Buffer full */
				putc('\a');
				भङ्ग;
			}
			यदि (c == '\t') {	/* expand TABs */
				यदि (IS_ENABLED(CONFIG_AUTO_COMPLETE)) {
					/*
					 * if auto-completion triggered just
					 * continue
					 */
					*p = '\0';
					यदि (cmd_auto_complete(prompt,
							      console_बफर,
							      &n, &col)) {
						p = p_buf + n;	/* reset */
						अनुवर्त;
					}
				}
				पङ्क्तिं_लिख(tab_seq + (col & 07));
				col += 8 - (col & 07);
			} अन्यथा {
				वर्ण __maybe_unused buf[2];

				/*
				 * Echo input using puts() to force an LCD
				 * flush if we are using an LCD
				 */
				++col;
				buf[0] = c;
				buf[1] = '\0';
				पङ्क्तिं_लिख(buf);
			}
			*p++ = c;
			++n;
			भङ्ग;
		}
	}
}

पूर्णाङ्क cli_readline_into_बफर(ध्रुव वर्ण *ध्रुव prompt, वर्ण *बफर,
			     पूर्णाङ्क timeout)
{
	वर्ण *p = बफर;
	uint len = CONFIG_SYS_CBSIZE;
	पूर्णाङ्क rc;
	स्थिर पूर्णाङ्क initted;

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
	यदि (IS_ENABLED(CONFIG_CMDLINE_EDITING) && (gd->flags & GD_FLG_RELOC)) {
		यदि (!initted) {
			rc = hist_init();
			यदि (rc == 0)
				initted = 1;
		}

		यदि (prompt)
			पङ्क्तिं_लिख(prompt);

		rc = cread_पङ्क्तिः(prompt, p, &len, timeout);
		प्रत्यागम rc < 0 ? rc : len;

	} अन्यथा {
		प्रत्यागम cread_पङ्क्तिः_simple(prompt, p);
	}
}
