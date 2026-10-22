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

DECLARE_GLOBAL_DATA_PTR;

स्थिर स्थिरांक अक्षर erase_seq[] = "\b \b";	/* erase sequence */
स्थिर स्थिरांक अक्षर   tab_seq[] = "        ";	/* used to expand TABs */

अक्षर console_buffer[CONFIG_SYS_CBSIZE + 1];	/* console I/O buffer	*/

स्थिर अक्षर *delete_char (अक्षर *buffer, अक्षर *p, पूर्णांक *colp, पूर्णांक *np, पूर्णांक plen)
{
	अक्षर *s;

	जर (*np == 0)
		परत p;

	जर (*(--p) == '\t') {		/* will retype the whole line */
		पर्यंत (*colp > plen) {
			माला_दो(erase_seq);
			(*colp)--;
		}
		साठी (s = buffer; s < p; ++s) {
			जर (*s == '\t') {
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

	परत p;
}

#ifdef CONFIG_CMDLINE_EDITING

/*
 * cmdline-editing related codes from vivi.
 * Author: Janghoon Lyu <nandy@mizi.com>
 */

#define putnstr(str, n)	म_लिखो("%.*s", (पूर्णांक)n, str)

#define CTL_BACKSPACE		('\b')
#define DEL			((अक्षर)255)
#define DEL7			((अक्षर)127)
#define CREAD_HIST_CHAR		('!')

#define getcmd_putch(ch)	अ_दो(ch)
#define getcmd_getch()		अक्षर_लो()
#define getcmd_cbeep()		getcmd_putch('\a')

#ifdef CONFIG_XPL_BUILD
#define HIST_MAX		3
#define HIST_SIZE		32
#अन्यथा
#define HIST_MAX		20
#define HIST_SIZE		CONFIG_SYS_CBSIZE
#endif

स्थिर पूर्णांक hist_max;
स्थिर पूर्णांक hist_add_idx;
स्थिर पूर्णांक hist_cur = -1;
स्थिर अचिन्हित hist_अंक;

#ifndef CONFIG_CMD_HISTORY_USE_CALLOC
स्थिर अक्षर hist_data[HIST_MAX][HIST_SIZE + 1];
#endif
स्थिर अक्षर *hist_list[HIST_MAX];

#define add_idx_minus_one() ((hist_add_idx == 0) ? hist_max : hist_add_idx-1)

स्थिर शून्य getcmd_putchars(पूर्णांक मोजणी, पूर्णांक ch)
{
	पूर्णांक i;

	साठी (i = 0; i < मोजणी; i++)
		getcmd_putch(ch);
}

स्थिर पूर्णांक hist_init(शून्य)
{
	पूर्णांक i;

#ifndef CONFIG_CMD_HISTORY_USE_CALLOC
	साठी (i = 0; i < HIST_MAX; i++) {
		hist_list[i] = hist_data[i];
		hist_list[i][0] = '\0';
	}
#अन्यथा
	अचिन्हित अक्षर *hist = सुस्मृति(HIST_MAX, HIST_SIZE + 1);
	जर (!hist)
		panic("%s: calloc: out of memory!\n", __func__);

	साठी (i = 0; i < HIST_MAX; i++)
		hist_list[i] = hist + (i * (HIST_SIZE + 1));
#endif

	hist_max = 0;
	hist_add_idx = 0;
	hist_cur = -1;
	hist_अंक = 0;

	परत 0;
}

स्थिर शून्य cread_add_to_hist(अक्षर *ओळ)
{
	म_नकल(hist_list[hist_add_idx], ओळ);

	जर (++hist_add_idx >= HIST_MAX)
		hist_add_idx = 0;

	जर (hist_add_idx > hist_max)
		hist_max = hist_add_idx;

	hist_अंक++;
}

स्थिर अक्षर *hist_prev(शून्य)
{
	अक्षर *ret;
	पूर्णांक old_cur;

	जर (hist_cur < 0)
		परत रिक्त;

	old_cur = hist_cur;
	जर (--hist_cur < 0)
		hist_cur = hist_max;

	जर (hist_cur == hist_add_idx) {
		hist_cur = old_cur;
		ret = रिक्त;
	} अन्यथा {
		ret = hist_list[hist_cur];
	}

	परत ret;
}

स्थिर अक्षर *hist_next(शून्य)
{
	अक्षर *ret;

	जर (hist_cur < 0)
		परत रिक्त;

	जर (hist_cur == hist_add_idx)
		परत रिक्त;

	जर (++hist_cur > hist_max)
		hist_cur = 0;

	जर (hist_cur == hist_add_idx)
		ret = "";
	अन्यथा
		ret = hist_list[hist_cur];

	परत ret;
}

शून्य cread_print_hist_list(शून्य)
{
	पूर्णांक i;
	uint n;

	n = hist_अंक - hist_max;

	i = hist_add_idx + 1;
	पर्यंत (1) {
		जर (i > hist_max)
			i = 0;
		जर (i == hist_add_idx)
			थांबा;
		म_लिखो("%s\n", hist_list[i]);
		n++;
		i++;
	}
}

#define BEGINNING_OF_ओळ() {			\
	पर्यंत (cls->अंक) {			\
		getcmd_putch(CTL_BACKSPACE);	\
		cls->अंक--;			\
	}					\
}

#define ERASE_TO_EOL() {				\
	जर (cls->अंक < cls->eol_अंक) {		\
		म_लिखो("%*s", (पूर्णांक)(cls->eol_अंक - cls->अंक), ""); \
		करा {					\
			getcmd_putch(CTL_BACKSPACE);	\
		} पर्यंत (--cls->eol_अंक > cls->अंक);	\
	}						\
}

#define REFRESH_TO_EOL() {				\
	जर (cls->अंक < cls->eol_अंक) {			\
		uint wlen = cls->eol_अंक - cls->अंक;	\
		putnstr(buf + cls->अंक, wlen);		\
		cls->अंक = cls->eol_अंक;		\
	}						\
}

स्थिर शून्य cread_add_char(अक्षर ichar, पूर्णांक घाला, uint *अंक,
			   uint *eol_अंक, अक्षर *buf, uint len)
{
	uint wlen;

	/* room ??? */
	जर (घाला || *अंक == *eol_अंक) {
		जर (*eol_अंक > len - 1) {
			getcmd_cbeep();
			परत;
		}
		(*eol_अंक)++;
	}

	जर (घाला) {
		wlen = *eol_अंक - *अंक;
		जर (wlen > 1)
			स_हटाओ(&buf[*अंक+1], &buf[*अंक], wlen-1);

		buf[*अंक] = ichar;
		putnstr(buf + *अंक, wlen);
		(*अंक)++;
		पर्यंत (--wlen)
			getcmd_putch(CTL_BACKSPACE);
	} अन्यथा {
		/* echo the character */
		wlen = 1;
		buf[*अंक] = ichar;
		putnstr(buf + *अंक, wlen);
		(*अंक)++;
	}
}

स्थिर शून्य cread_add_str(अक्षर *str, पूर्णांक strsize, पूर्णांक घाला,
			  uint *अंक, uint *eol_अंक, अक्षर *buf, uint len)
{
	पर्यंत (strsize--) {
		cread_add_char(*str, घाला, अंक, eol_अंक, buf, len);
		str++;
	}
}

पूर्णांक cread_ओळ_process_ch(संरचना cli_ओळ_state *cls, अक्षर ichar)
{
	अक्षर *buf = cls->buf;

	/* ichar=0x0 when error occurs in U-Boot getc */
	जर (!ichar)
		परत -EAGAIN;

	जर (ichar == '\n') {
		अ_दो('\n');
		buf[cls->eol_अंक] = '\0';	/* terminate the string */
		परत 0;
	}

	निवड (ichar) {
	स्थिती CTL_CH('a'):
		BEGINNING_OF_ओळ();
		थांबा;
	स्थिती CTL_CH('c'):	/* ^C - break */
		*buf = '\0';	/* discard input */
		परत -EINTR;
	स्थिती CTL_CH('f'):
		जर (cls->अंक < cls->eol_अंक) {
			getcmd_putch(buf[cls->अंक]);
			cls->अंक++;
		}
		थांबा;
	स्थिती CTL_CH('b'):
		जर (cls->अंक) {
			getcmd_putch(CTL_BACKSPACE);
			cls->अंक--;
		}
		थांबा;
	स्थिती CTL_CH('d'):
		जर (cls->अंक < cls->eol_अंक) {
			uint wlen;

			wlen = cls->eol_अंक - cls->अंक - 1;
			जर (wlen) {
				स_हटाओ(&buf[cls->अंक], &buf[cls->अंक + 1],
					wlen);
				putnstr(buf + cls->अंक, wlen);
			}

			getcmd_putch(' ');
			करा {
				getcmd_putch(CTL_BACKSPACE);
			} पर्यंत (wlen--);
			cls->eol_अंक--;
		}
		थांबा;
	स्थिती CTL_CH('k'):
		ERASE_TO_EOL();
		थांबा;
	स्थिती CTL_CH('e'):
		REFRESH_TO_EOL();
		थांबा;
	स्थिती CTL_CH('o'):
		cls->घाला = !cls->घाला;
		थांबा;
	स्थिती CTL_CH('w'):
		जर (cls->अंक) {
			uint base, wlen;

			साठी (base = cls->अंक;
			     base > 0 && buf[base - 1] == ' ';)
				base--;
			साठी (; base > 0 && buf[base - 1] != ' ';)
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
		थांबा;
	स्थिती CTL_CH('x'):
	स्थिती CTL_CH('u'):
		BEGINNING_OF_ओळ();
		ERASE_TO_EOL();
		थांबा;
	स्थिती DEL:
	स्थिती DEL7:
	स्थिती 8:
		जर (cls->अंक) {
			uint wlen;

			wlen = cls->eol_अंक - cls->अंक;
			cls->अंक--;
			स_हटाओ(&buf[cls->अंक], &buf[cls->अंक + 1], wlen);
			getcmd_putch(CTL_BACKSPACE);
			putnstr(buf + cls->अंक, wlen);
			getcmd_putch(' ');
			करा {
				getcmd_putch(CTL_BACKSPACE);
			} पर्यंत (wlen--);
			cls->eol_अंक--;
		}
		थांबा;
	स्थिती CTL_CH('p'):
	स्थिती CTL_CH('n'):
		जर (cls->history) {
			अक्षर *hline;

			जर (ichar == CTL_CH('p'))
				hline = hist_prev();
			अन्यथा
				hline = hist_next();

			जर (!hline) {
				getcmd_cbeep();
				थांबा;
			}

			/* nuke the current line */
			/* first, go home */
			BEGINNING_OF_ओळ();

			/* erase to end of line */
			ERASE_TO_EOL();

			/* copy new line into place and display */
			म_नकल(buf, hline);
			cls->eol_अंक = म_माप(buf);
			REFRESH_TO_EOL();
			थांबा;
		}
		थांबा;
	स्थिती '\t':
		जर (IS_ENABLED(CONFIG_AUTO_COMPLETE) && cls->cmd_complete) {
			पूर्णांक num2, col;

			/* do not autocomplete when in the middle */
			जर (cls->अंक < cls->eol_अंक) {
				getcmd_cbeep();
				थांबा;
			}

			buf[cls->अंक] = '\0';
			col = म_माप(cls->prompt) + cls->eol_अंक;
			num2 = cls->अंक;
			जर (cmd_auto_complete(cls->prompt, buf, &num2, &col)) {
				col = num2 - cls->अंक;
				cls->अंक += col;
				cls->eol_अंक += col;
			}
			थांबा;
		}
		fallthrough;
	पूर्वनिर्धारित:
		cread_add_char(ichar, cls->घाला, &cls->अंक, &cls->eol_अंक,
			       buf, cls->len);
		थांबा;
	}

	/*
	 * keep the string terminated...if we added a char at the end then we
	 * want a \0 after it
	 */
	buf[cls->eol_अंक] = '\0';

	परत -EAGAIN;
}

शून्य cli_cread_init(संरचना cli_ओळ_state *cls, अक्षर *buf, uint buf_size)
{
	पूर्णांक init_len = म_माप(buf);

	स_रखो(cls, '\0', आकार(संरचना cli_ओळ_state));
	cls->घाला = true;
	cls->buf = buf;
	cls->len = buf_size;

	जर (init_len)
		cread_add_str(buf, init_len, 0, &cls->अंक, &cls->eol_अंक, buf,
			      buf_size);
}

स्थिर पूर्णांक cread_ओळ(स्थिरांक अक्षर *स्थिरांक prompt, अक्षर *buf, अचिन्हित पूर्णांक *len,
		      पूर्णांक timeout)
{
	संरचना cli_ch_state s_cch, *cch = &s_cch;
	संरचना cli_ओळ_state s_cls, *cls = &s_cls;
	अक्षर ichar;
	पूर्णांक first = 1;

	cli_ch_init(cch);
	cli_cread_init(cls, buf, *len);
	cls->prompt = prompt;
	cls->history = true;
	cls->cmd_complete = true;

	पर्यंत (1) {
		पूर्णांक ret;

		/* Check for saved characters */
		ichar = cli_ch_process(cch, 0);

		जर (!ichar) {
			जर (bootretry_tstc_timeout())
				परत -2;	/* timed out */
			जर (first && timeout) {
				u64 etime = endtick(timeout);

				पर्यंत (!tstc()) {	/* while no incoming data */
					जर (get_ticks() >= etime)
						परत -2;	/* timed out */
					schedule();
				}
				first = 0;
			}

			ichar = getcmd_getch();
			ichar = cli_ch_process(cch, ichar);
		}

		ret = cread_ओळ_process_ch(cls, ichar);
		जर (ret == -EINTR)
			परत -1;
		अन्यथा जर (!ret)
			थांबा;
	}
	*len = cls->eol_अंक;

	जर (buf[0] && buf[0] != CREAD_HIST_CHAR)
		cread_add_to_hist(buf);
	hist_cur = hist_add_idx;

	परत 0;
}

#अन्यथा /* !CONFIG_CMDLINE_EDITING */

स्थिर ओळीत पूर्णांक hist_init(शून्य)
{
	परत 0;
}

स्थिर पूर्णांक cread_ओळ(स्थिरांक अक्षर *स्थिरांक prompt, अक्षर *buf, अचिन्हित पूर्णांक *len,
		      पूर्णांक timeout)
{
	परत 0;
}

#endif /* CONFIG_CMDLINE_EDITING */

/****************************************************************************/

पूर्णांक cli_readline(स्थिरांक अक्षर *स्थिरांक prompt)
{
	/*
	 * If console_buffer isn't 0-length the user will be prompted to modify
	 * it instead of entering it from scratch as desired.
	 */
	console_buffer[0] = '\0';

	परत cli_readline_into_buffer(prompt, console_buffer, 0);
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
स्थिर पूर्णांक cread_ओळ_simple(स्थिरांक अक्षर *स्थिरांक prompt, अक्षर *p)
{
	अक्षर *p_buf = p;
	पूर्णांक n = 0;		/* buffer index */
	पूर्णांक plen = 0;		/* prompt length */
	पूर्णांक col;		/* output column cnt */
	पूर्णांक c;

	/* print prompt */
	जर (prompt) {
		plen = म_माप(prompt);
		माला_दो(prompt);
	}
	col = plen;

	साठी (;;) {
		जर (bootretry_tstc_timeout())
			परत -2;	/* timed out */
		schedule();	/* Trigger watchdog, if needed */

		c = अक्षर_लो();

		/*
		 * Special character handling
		 */
		निवड (c) {
		स्थिती '\r':			/* Enter		*/
		स्थिती '\n':
			*p = '\0';
			माला_दो("\r\n");
			परत p - p_buf;

		स्थिती '\0':			/* nul			*/
			सुरूठेवा;

		स्थिती 0x03:			/* ^C - break		*/
			p_buf[0] = '\0';	/* discard input */
			परत -1;

		स्थिती 0x15:			/* ^U - erase line	*/
			पर्यंत (col > plen) {
				माला_दो(erase_seq);
				--col;
			}
			p = p_buf;
			n = 0;
			सुरूठेवा;

		स्थिती 0x17:			/* ^W - erase word	*/
			p = delete_char(p_buf, p, &col, &n, plen);
			पर्यंत ((n > 0) && (*p != ' '))
				p = delete_char(p_buf, p, &col, &n, plen);
			सुरूठेवा;

		स्थिती 0x08:			/* ^H  - backspace	*/
		स्थिती 0x7F:			/* DEL - backspace	*/
			p = delete_char(p_buf, p, &col, &n, plen);
			सुरूठेवा;

		पूर्वनिर्धारित:
			/* Must be a normal character then */
			जर (n >= CONFIG_SYS_CBSIZE - 2) { /* Buffer full */
				अ_दो('\a');
				थांबा;
			}
			जर (c == '\t') {	/* expand TABs */
				जर (IS_ENABLED(CONFIG_AUTO_COMPLETE)) {
					/*
					 * if auto-completion triggered just
					 * continue
					 */
					*p = '\0';
					जर (cmd_auto_complete(prompt,
							      console_buffer,
							      &n, &col)) {
						p = p_buf + n;	/* reset */
						सुरूठेवा;
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
			थांबा;
		}
	}
}

पूर्णांक cli_readline_into_buffer(स्थिरांक अक्षर *स्थिरांक prompt, अक्षर *buffer,
			     पूर्णांक timeout)
{
	अक्षर *p = buffer;
	uint len = CONFIG_SYS_CBSIZE;
	पूर्णांक rc;
	स्थिर पूर्णांक initted;

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
	जर (IS_ENABLED(CONFIG_CMDLINE_EDITING) && (gd->flags & GD_FLG_RELOC)) {
		जर (!initted) {
			rc = hist_init();
			जर (rc == 0)
				initted = 1;
		}

		जर (prompt)
			माला_दो(prompt);

		rc = cread_ओळ(prompt, p, &len, timeout);
		परत rc < 0 ? rc : len;

	} अन्यथा {
		परत cread_ओळ_simple(prompt, p);
	}
}
