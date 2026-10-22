// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Add to readline cmdline-editing by
 * (C) Copyright 2005
 * JinHua Luo, GuangDong Linux Center, <luo.jinhua@gd-linux.com>
 */

#समावेश <bootretry.h>
#समावेश <cli.h>
#समावेश <command.h>
#समावेश <hang.h>
#समावेश <malloc.h>
#समावेश <time.h>
#समावेश <watchdog.h>
#समावेश <linux/errno.h>
#समावेश <asm/global_data.h>

DECLARE_GLOBAL_तथ्य_PTR;

अटल स्थिर अक्षर erase_seq[] = "\b \b";	/* erase sequence */
अटल स्थिर अक्षर   tab_seq[] = "        ";	/* used to expand TABs */

अक्षर console_बफर[CONFIG_SYS_CBSIZE + 1];	/* console I/O buffer	*/

अटल अक्षर *delete_char (अक्षर *बफर, अक्षर *p, पूर्णांक *colp, पूर्णांक *np, पूर्णांक plen)
{
	अक्षर *s;

	अगर (*np == 0)
		वापस p;

	अगर (*(--p) == '\t') {		/* will retype the whole line */
		जबतक (*colp > plen) {
			माला_दो(erase_seq);
			(*colp)--;
		}
		क्रम (s = बफर; s < p; ++s) {
			अगर (*s == '\t') {
				माला_दो(tab_seq + ((*colp) & 07));
				*colp += 8 - ((*colp) & 07);
			} अन्यथा {
				++(*colp);
				अ_दो(*s);
			}
		}
	} अन्यथा {
		माला_दो(erase_seq);
		(*colp)--;
	}
	(*np)--;

	वापस p;
}

#अगर_घोषित CONFIG_CMDLINE_EDITING

/*
 * cmdline-editing related codes from vivi.
 * Author: Janghoon Lyu <nandy@mizi.com>
 */

#घोषणा putnstr(str, n)	म_लिखो("%.*s", (पूर्णांक)n, str)

#घोषणा CTL_BACKSPACE		('\b')
#घोषणा DEL			((अक्षर)255)
#घोषणा DEL7			((अक्षर)127)
#घोषणा CREAD_HIST_CHAR		('!')

#घोषणा getcmd_putch(ch)	अ_दो(ch)
#घोषणा getcmd_getch()		अक्षर_लो()
#घोषणा getcmd_cbeep()		getcmd_putch('\a')

#अगर_घोषित CONFIG_XPL_BUILD
#घोषणा HIST_MAX		3
#घोषणा HIST_आकार		32
#अन्यथा
#घोषणा HIST_MAX		20
#घोषणा HIST_आकार		CONFIG_SYS_CBSIZE
#पूर्ण_अगर

अटल पूर्णांक hist_max;
अटल पूर्णांक hist_add_idx;
अटल पूर्णांक hist_cur = -1;
अटल अचिन्हित hist_अंक;

#अगर_अघोषित CONFIG_CMD_HISTORY_USE_CALLOC
अटल अक्षर hist_तथ्य[HIST_MAX][HIST_आकार + 1];
#पूर्ण_अगर
अटल अक्षर *hist_list[HIST_MAX];

#घोषणा add_idx_minus_one() ((hist_add_idx == 0) ? hist_max : hist_add_idx-1)

अटल व्योम getcmd_putchars(पूर्णांक गिनती, पूर्णांक ch)
{
	पूर्णांक i;

	क्रम (i = 0; i < गिनती; i++)
		getcmd_putch(ch);
}

अटल पूर्णांक hist_init(व्योम)
{
	पूर्णांक i;

#अगर_अघोषित CONFIG_CMD_HISTORY_USE_CALLOC
	क्रम (i = 0; i < HIST_MAX; i++) {
		hist_list[i] = hist_तथ्य[i];
		hist_list[i][0] = '\0';
	}
#अन्यथा
	अचिन्हित अक्षर *hist = सुस्मृति(HIST_MAX, HIST_आकार + 1);
	अगर (!hist)
		panic("%s: calloc: out of memory!\n", __func__);

	क्रम (i = 0; i < HIST_MAX; i++)
		hist_list[i] = hist + (i * (HIST_आकार + 1));
#पूर्ण_अगर

	hist_max = 0;
	hist_add_idx = 0;
	hist_cur = -1;
	hist_अंक = 0;

	वापस 0;
}

अटल व्योम cread_add_to_hist(अक्षर *पंक्ति)
{
	म_नकल(hist_list[hist_add_idx], पंक्ति);

	अगर (++hist_add_idx >= HIST_MAX)
		hist_add_idx = 0;

	अगर (hist_add_idx > hist_max)
		hist_max = hist_add_idx;

	hist_अंक++;
}

अटल अक्षर *hist_पूर्व(व्योम)
{
	अक्षर *ret;
	पूर्णांक old_cur;

	अगर (hist_cur < 0)
		वापस शून्य;

	old_cur = hist_cur;
	अगर (--hist_cur < 0)
		hist_cur = hist_max;

	अगर (hist_cur == hist_add_idx) {
		hist_cur = old_cur;
		ret = शून्य;
	} अन्यथा {
		ret = hist_list[hist_cur];
	}

	वापस ret;
}

अटल अक्षर *hist_अग्र(व्योम)
{
	अक्षर *ret;

	अगर (hist_cur < 0)
		वापस शून्य;

	अगर (hist_cur == hist_add_idx)
		वापस शून्य;

	अगर (++hist_cur > hist_max)
		hist_cur = 0;

	अगर (hist_cur == hist_add_idx)
		ret = "";
	अन्यथा
		ret = hist_list[hist_cur];

	वापस ret;
}

व्योम cread_print_hist_list(व्योम)
{
	पूर्णांक i;
	uint n;

	n = hist_अंक - hist_max;

	i = hist_add_idx + 1;
	जबतक (1) {
		अगर (i > hist_max)
			i = 0;
		अगर (i == hist_add_idx)
			अवरोध;
		म_लिखो("%s\n", hist_list[i]);
		n++;
		i++;
	}
}

#घोषणा BEGINNING_OF_पंक्ति() {			\
	जबतक (cls->अंक) {			\
		getcmd_putch(CTL_BACKSPACE);	\
		cls->अंक--;			\
	}					\
}

#घोषणा ERASE_TO_EOL() {				\
	अगर (cls->अंक < cls->eol_अंक) {		\
		म_लिखो("%*s", (पूर्णांक)(cls->eol_अंक - cls->अंक), ""); \
		करो {					\
			getcmd_putch(CTL_BACKSPACE);	\
		} जबतक (--cls->eol_अंक > cls->अंक);	\
	}						\
}

#घोषणा REFRESH_TO_EOL() {				\
	अगर (cls->अंक < cls->eol_अंक) {			\
		uint wlen = cls->eol_अंक - cls->अंक;	\
		putnstr(buf + cls->अंक, wlen);		\
		cls->अंक = cls->eol_अंक;		\
	}						\
}

अटल व्योम cread_add_char(अक्षर ichar, पूर्णांक जोड़ो, uint *अंक,
			   uint *eol_अंक, अक्षर *buf, uint len)
{
	uint wlen;

	/* room ??? */
	अगर (जोड़ो || *अंक == *eol_अंक) {
		अगर (*eol_अंक > len - 1) {
			getcmd_cbeep();
			वापस;
		}
		(*eol_अंक)++;
	}

	अगर (जोड़ो) {
		wlen = *eol_अंक - *अंक;
		अगर (wlen > 1)
			स_हटाओ(&buf[*अंक+1], &buf[*अंक], wlen-1);

		buf[*अंक] = ichar;
		putnstr(buf + *अंक, wlen);
		(*अंक)++;
		जबतक (--wlen)
			getcmd_putch(CTL_BACKSPACE);
	} अन्यथा {
		/* echo the character */
		wlen = 1;
		buf[*अंक] = ichar;
		putnstr(buf + *अंक, wlen);
		(*अंक)++;
	}
}

अटल व्योम cread_add_str(अक्षर *str, पूर्णांक strsize, पूर्णांक जोड़ो,
			  uint *अंक, uint *eol_अंक, अक्षर *buf, uint len)
{
	जबतक (strsize--) {
		cread_add_char(*str, जोड़ो, अंक, eol_अंक, buf, len);
		str++;
	}
}

पूर्णांक cread_पंक्ति_process_ch(काष्ठा cli_पंक्ति_state *cls, अक्षर ichar)
{
	अक्षर *buf = cls->buf;

	/* ichar=0x0 when error occurs in U-Boot getc */
	अगर (!ichar)
		वापस -EAGAIN;

	अगर (ichar == '\n') {
		अ_दो('\n');
		buf[cls->eol_अंक] = '\0';	/* terminate the string */
		वापस 0;
	}

	चयन (ichar) {
	हाल CTL_CH('a'):
		BEGINNING_OF_पंक्ति();
		अवरोध;
	हाल CTL_CH('c'):	/* ^C - break */
		*buf = '\0';	/* discard input */
		वापस -EINTR;
	हाल CTL_CH('f'):
		अगर (cls->अंक < cls->eol_अंक) {
			getcmd_putch(buf[cls->अंक]);
			cls->अंक++;
		}
		अवरोध;
	हाल CTL_CH('b'):
		अगर (cls->अंक) {
			getcmd_putch(CTL_BACKSPACE);
			cls->अंक--;
		}
		अवरोध;
	हाल CTL_CH('d'):
		अगर (cls->अंक < cls->eol_अंक) {
			uint wlen;

			wlen = cls->eol_अंक - cls->अंक - 1;
			अगर (wlen) {
				स_हटाओ(&buf[cls->अंक], &buf[cls->अंक + 1],
					wlen);
				putnstr(buf + cls->अंक, wlen);
			}

			getcmd_putch(' ');
			करो {
				getcmd_putch(CTL_BACKSPACE);
			} जबतक (wlen--);
			cls->eol_अंक--;
		}
		अवरोध;
	हाल CTL_CH('k'):
		ERASE_TO_EOL();
		अवरोध;
	हाल CTL_CH('e'):
		REFRESH_TO_EOL();
		अवरोध;
	हाल CTL_CH('o'):
		cls->जोड़ो = !cls->जोड़ो;
		अवरोध;
	हाल CTL_CH('w'):
		अगर (cls->अंक) {
			uint base, wlen;

			क्रम (base = cls->अंक;
			     base > 0 && buf[base - 1] == ' ';)
				base--;
			क्रम (; base > 0 && buf[base - 1] != ' ';)
				base--;

			/* now delete chars from base to cls->num */
			wlen = cls->अंक - base;
			cls->eol_अंक -= wlen;
			स_हटाओ(&buf[base], &buf[cls->अंक],
				cls->eol_अंक - base + 1);
			cls->अंक = base;
			getcmd_putchars(wlen, CTL_BACKSPACE);
			माला_दो(buf + base);
			getcmd_putchars(wlen, ' ');
			getcmd_putchars(wlen + cls->eol_अंक - cls->अंक,
					CTL_BACKSPACE);
		}
		अवरोध;
	हाल CTL_CH('x'):
	हाल CTL_CH('u'):
		BEGINNING_OF_पंक्ति();
		ERASE_TO_EOL();
		अवरोध;
	हाल DEL:
	हाल DEL7:
	हाल 8:
		अगर (cls->अंक) {
			uint wlen;

			wlen = cls->eol_अंक - cls->अंक;
			cls->अंक--;
			स_हटाओ(&buf[cls->अंक], &buf[cls->अंक + 1], wlen);
			getcmd_putch(CTL_BACKSPACE);
			putnstr(buf + cls->अंक, wlen);
			getcmd_putch(' ');
			करो {
				getcmd_putch(CTL_BACKSPACE);
			} जबतक (wlen--);
			cls->eol_अंक--;
		}
		अवरोध;
	हाल CTL_CH('p'):
	हाल CTL_CH('n'):
		अगर (cls->history) {
			अक्षर *hline;

			अगर (ichar == CTL_CH('p'))
				hline = hist_पूर्व();
			अन्यथा
				hline = hist_अग्र();

			अगर (!hline) {
				getcmd_cbeep();
				अवरोध;
			}

			/* nuke the current line */
			/* first, go home */
			BEGINNING_OF_पंक्ति();

			/* erase to end of line */
			ERASE_TO_EOL();

			/* copy new line into place and display */
			म_नकल(buf, hline);
			cls->eol_अंक = म_माप(buf);
			REFRESH_TO_EOL();
			अवरोध;
		}
		अवरोध;
	हाल '\t':
		अगर (IS_ENABLED(CONFIG_AUTO_COMPLETE) && cls->cmd_complete) {
			पूर्णांक num2, col;

			/* do not autocomplete when in the middle */
			अगर (cls->अंक < cls->eol_अंक) {
				getcmd_cbeep();
				अवरोध;
			}

			buf[cls->अंक] = '\0';
			col = म_माप(cls->prompt) + cls->eol_अंक;
			num2 = cls->अंक;
			अगर (cmd_auto_complete(cls->prompt, buf, &num2, &col)) {
				col = num2 - cls->अंक;
				cls->अंक += col;
				cls->eol_अंक += col;
			}
			अवरोध;
		}
		fallthrough;
	शेष:
		cread_add_char(ichar, cls->जोड़ो, &cls->अंक, &cls->eol_अंक,
			       buf, cls->len);
		अवरोध;
	}

	/*
	 * keep the string terminated...if we added a char at the end then we
	 * want a \0 after it
	 */
	buf[cls->eol_अंक] = '\0';

	वापस -EAGAIN;
}

व्योम cli_cread_init(काष्ठा cli_पंक्ति_state *cls, अक्षर *buf, uint buf_आकार)
{
	पूर्णांक init_len = म_माप(buf);

	स_रखो(cls, '\0', माप(काष्ठा cli_पंक्ति_state));
	cls->जोड़ो = true;
	cls->buf = buf;
	cls->len = buf_आकार;

	अगर (init_len)
		cread_add_str(buf, init_len, 0, &cls->अंक, &cls->eol_अंक, buf,
			      buf_आकार);
}

अटल पूर्णांक cread_पंक्ति(स्थिर अक्षर *स्थिर prompt, अक्षर *buf, अचिन्हित पूर्णांक *len,
		      पूर्णांक timeout)
{
	काष्ठा cli_ch_state s_cch, *cch = &s_cch;
	काष्ठा cli_पंक्ति_state s_cls, *cls = &s_cls;
	अक्षर ichar;
	पूर्णांक first = 1;

	cli_ch_init(cch);
	cli_cread_init(cls, buf, *len);
	cls->prompt = prompt;
	cls->history = true;
	cls->cmd_complete = true;

	जबतक (1) {
		पूर्णांक ret;

		/* Check for saved characters */
		ichar = cli_ch_process(cch, 0);

		अगर (!ichar) {
			अगर (bootretry_tstc_timeout())
				वापस -2;	/* timed out */
			अगर (first && timeout) {
				u64 etime = endtick(timeout);

				जबतक (!tstc()) {	/* while no incoming data */
					अगर (get_ticks() >= etime)
						वापस -2;	/* timed out */
					schedule();
				}
				first = 0;
			}

			ichar = getcmd_getch();
			ichar = cli_ch_process(cch, ichar);
		}

		ret = cread_पंक्ति_process_ch(cls, ichar);
		अगर (ret == -EINTR)
			वापस -1;
		अन्यथा अगर (!ret)
			अवरोध;
	}
	*len = cls->eol_अंक;

	अगर (buf[0] && buf[0] != CREAD_HIST_CHAR)
		cread_add_to_hist(buf);
	hist_cur = hist_add_idx;

	वापस 0;
}

#अन्यथा /* !CONFIG_CMDLINE_EDITING */

अटल अंतरभूत पूर्णांक hist_init(व्योम)
{
	वापस 0;
}

अटल पूर्णांक cread_पंक्ति(स्थिर अक्षर *स्थिर prompt, अक्षर *buf, अचिन्हित पूर्णांक *len,
		      पूर्णांक timeout)
{
	वापस 0;
}

#पूर्ण_अगर /* CONFIG_CMDLINE_EDITING */

/****************************************************************************/

पूर्णांक cli_readline(स्थिर अक्षर *स्थिर prompt)
{
	/*
	 * If console_buffer isn't 0-length the user will be prompted to modify
	 * it instead of entering it from scratch as desired.
	 */
	console_बफर[0] = '\0';

	वापस cli_readline_into_बफर(prompt, console_बफर, 0);
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
अटल पूर्णांक cread_पंक्ति_simple(स्थिर अक्षर *स्थिर prompt, अक्षर *p)
{
	अक्षर *p_buf = p;
	पूर्णांक n = 0;		/* buffer index */
	पूर्णांक plen = 0;		/* prompt length */
	पूर्णांक col;		/* output column cnt */
	पूर्णांक c;

	/* print prompt */
	अगर (prompt) {
		plen = म_माप(prompt);
		माला_दो(prompt);
	}
	col = plen;

	क्रम (;;) {
		अगर (bootretry_tstc_timeout())
			वापस -2;	/* timed out */
		schedule();	/* Trigger watchdog, if needed */

		c = अक्षर_लो();

		/*
		 * Special character handling
		 */
		चयन (c) {
		हाल '\r':			/* Enter		*/
		हाल '\n':
			*p = '\0';
			माला_दो("\r\n");
			वापस p - p_buf;

		हाल '\0':			/* nul			*/
			जारी;

		हाल 0x03:			/* ^C - break		*/
			p_buf[0] = '\0';	/* discard input */
			वापस -1;

		हाल 0x15:			/* ^U - erase line	*/
			जबतक (col > plen) {
				माला_दो(erase_seq);
				--col;
			}
			p = p_buf;
			n = 0;
			जारी;

		हाल 0x17:			/* ^W - erase word	*/
			p = delete_char(p_buf, p, &col, &n, plen);
			जबतक ((n > 0) && (*p != ' '))
				p = delete_char(p_buf, p, &col, &n, plen);
			जारी;

		हाल 0x08:			/* ^H  - backspace	*/
		हाल 0x7F:			/* DEL - backspace	*/
			p = delete_char(p_buf, p, &col, &n, plen);
			जारी;

		शेष:
			/* Must be a normal character then */
			अगर (n >= CONFIG_SYS_CBSIZE - 2) { /* Buffer full */
				अ_दो('\a');
				अवरोध;
			}
			अगर (c == '\t') {	/* expand TABs */
				अगर (IS_ENABLED(CONFIG_AUTO_COMPLETE)) {
					/*
					 * if auto-completion triggered just
					 * continue
					 */
					*p = '\0';
					अगर (cmd_auto_complete(prompt,
							      console_बफर,
							      &n, &col)) {
						p = p_buf + n;	/* reset */
						जारी;
					}
				}
				माला_दो(tab_seq + (col & 07));
				col += 8 - (col & 07);
			} अन्यथा {
				अक्षर __maybe_unused buf[2];

				/*
				 * Echo input using puts() to force an LCD
				 * flush if we are using an LCD
				 */
				++col;
				buf[0] = c;
				buf[1] = '\0';
				माला_दो(buf);
			}
			*p++ = c;
			++n;
			अवरोध;
		}
	}
}

पूर्णांक cli_readline_into_बफर(स्थिर अक्षर *स्थिर prompt, अक्षर *बफर,
			     पूर्णांक timeout)
{
	अक्षर *p = बफर;
	uint len = CONFIG_SYS_CBSIZE;
	पूर्णांक rc;
	अटल पूर्णांक initted;

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
	अगर (IS_ENABLED(CONFIG_CMDLINE_EDITING) && (gd->flags & GD_FLG_RELOC)) {
		अगर (!initted) {
			rc = hist_init();
			अगर (rc == 0)
				initted = 1;
		}

		अगर (prompt)
			माला_दो(prompt);

		rc = cread_पंक्ति(prompt, p, &len, timeout);
		वापस rc < 0 ? rc : len;

	} अन्यथा {
		वापस cread_पंक्ति_simple(prompt, p);
	}
}
