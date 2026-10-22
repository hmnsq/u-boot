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

स्थिर अचर वर्ण erase_seq[] = "\b \b";	/* erase sequence */
स्थिर अचर वर्ण   tab_seq[] = "        ";	/* used to expand TABs */

वर्ण console_buffer[CONFIG_SYS_CBSIZE + 1];	/* console I/O buffer	*/

स्थिर वर्ण *delete_char (वर्ण *buffer, वर्ण *p, पूर्णाङ्क *colp, पूर्णाङ्क *np, पूर्णाङ्क plen)
{
	वर्ण *s;

	यदि (*np == 0)
		फर्काऊ p;

	यदि (*(--p) == '\t') {		/* will retype the whole line */
		जबसम्म (*colp > plen) {
			माला_दो(erase_seq);
			(*colp)--;
		}
		लागि (s = buffer; s < p; ++s) {
			यदि (*s == '\t') {
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

	फर्काऊ p;
}

#ifdef CONFIG_CMDLINE_EDITING

/*
 * cmdline-editing related codes from vivi.
 * Author: Janghoon Lyu <nandy@mizi.com>
 */

#define putnstr(str, n)	म_लिखो("%.*s", (पूर्णाङ्क)n, str)

#define CTL_BACKSPACE		('\b')
#define DEL			((वर्ण)255)
#define DEL7			((वर्ण)127)
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

स्थिर पूर्णाङ्क hist_max;
स्थिर पूर्णाङ्क hist_add_idx;
स्थिर पूर्णाङ्क hist_cur = -1;
स्थिर अचिन्हित hist_अङ्क;

#ifndef CONFIG_CMD_HISTORY_USE_CALLOC
स्थिर वर्ण hist_data[HIST_MAX][HIST_SIZE + 1];
#endif
स्थिर वर्ण *hist_list[HIST_MAX];

#define add_idx_minus_one() ((hist_add_idx == 0) ? hist_max : hist_add_idx-1)

स्थिर शून्य getcmd_putchars(पूर्णाङ्क गन्ती, पूर्णाङ्क ch)
{
	पूर्णाङ्क i;

	लागि (i = 0; i < गन्ती; i++)
		getcmd_putch(ch);
}

स्थिर पूर्णाङ्क hist_init(शून्य)
{
	पूर्णाङ्क i;

#ifndef CONFIG_CMD_HISTORY_USE_CALLOC
	लागि (i = 0; i < HIST_MAX; i++) {
		hist_list[i] = hist_data[i];
		hist_list[i][0] = '\0';
	}
#अन्यथा
	अचिन्हित वर्ण *hist = सुस्मृति(HIST_MAX, HIST_SIZE + 1);
	यदि (!hist)
		panic("%s: calloc: out of memory!\n", __func__);

	लागि (i = 0; i < HIST_MAX; i++)
		hist_list[i] = hist + (i * (HIST_SIZE + 1));
#endif

	hist_max = 0;
	hist_add_idx = 0;
	hist_cur = -1;
	hist_अङ्क = 0;

	फर्काऊ 0;
}

स्थिर शून्य cread_add_to_hist(वर्ण *पङ्क्ति)
{
	म_नकल(hist_list[hist_add_idx], पङ्क्ति);

	यदि (++hist_add_idx >= HIST_MAX)
		hist_add_idx = 0;

	यदि (hist_add_idx > hist_max)
		hist_max = hist_add_idx;

	hist_अङ्क++;
}

स्थिर वर्ण *hist_prev(शून्य)
{
	वर्ण *ret;
	पूर्णाङ्क old_cur;

	यदि (hist_cur < 0)
		फर्काऊ रिक्त;

	old_cur = hist_cur;
	यदि (--hist_cur < 0)
		hist_cur = hist_max;

	यदि (hist_cur == hist_add_idx) {
		hist_cur = old_cur;
		ret = रिक्त;
	} अन्यथा {
		ret = hist_list[hist_cur];
	}

	फर्काऊ ret;
}

स्थिर वर्ण *hist_next(शून्य)
{
	वर्ण *ret;

	यदि (hist_cur < 0)
		फर्काऊ रिक्त;

	यदि (hist_cur == hist_add_idx)
		फर्काऊ रिक्त;

	यदि (++hist_cur > hist_max)
		hist_cur = 0;

	यदि (hist_cur == hist_add_idx)
		ret = "";
	अन्यथा
		ret = hist_list[hist_cur];

	फर्काऊ ret;
}

शून्य cread_print_hist_list(शून्य)
{
	पूर्णाङ्क i;
	uint n;

	n = hist_अङ्क - hist_max;

	i = hist_add_idx + 1;
	जबसम्म (1) {
		यदि (i > hist_max)
			i = 0;
		यदि (i == hist_add_idx)
			रोक;
		म_लिखो("%s\n", hist_list[i]);
		n++;
		i++;
	}
}

#define BEGINNING_OF_पङ्क्ति() {			\
	जबसम्म (cls->अङ्क) {			\
		getcmd_putch(CTL_BACKSPACE);	\
		cls->अङ्क--;			\
	}					\
}

#define ERASE_TO_EOL() {				\
	यदि (cls->अङ्क < cls->eol_अङ्क) {		\
		म_लिखो("%*s", (पूर्णाङ्क)(cls->eol_अङ्क - cls->अङ्क), ""); \
		गर {					\
			getcmd_putch(CTL_BACKSPACE);	\
		} जबसम्म (--cls->eol_अङ्क > cls->अङ्क);	\
	}						\
}

#define REFRESH_TO_EOL() {				\
	यदि (cls->अङ्क < cls->eol_अङ्क) {			\
		uint wlen = cls->eol_अङ्क - cls->अङ्क;	\
		putnstr(buf + cls->अङ्क, wlen);		\
		cls->अङ्क = cls->eol_अङ्क;		\
	}						\
}

स्थिर शून्य cread_add_char(वर्ण ichar, पूर्णाङ्क घुसाउ, uint *अङ्क,
			   uint *eol_अङ्क, वर्ण *buf, uint len)
{
	uint wlen;

	/* room ??? */
	यदि (घुसाउ || *अङ्क == *eol_अङ्क) {
		यदि (*eol_अङ्क > len - 1) {
			getcmd_cbeep();
			फर्काऊ;
		}
		(*eol_अङ्क)++;
	}

	यदि (घुसाउ) {
		wlen = *eol_अङ्क - *अङ्क;
		यदि (wlen > 1)
			स_हटाओ(&buf[*अङ्क+1], &buf[*अङ्क], wlen-1);

		buf[*अङ्क] = ichar;
		putnstr(buf + *अङ्क, wlen);
		(*अङ्क)++;
		जबसम्म (--wlen)
			getcmd_putch(CTL_BACKSPACE);
	} अन्यथा {
		/* echo the character */
		wlen = 1;
		buf[*अङ्क] = ichar;
		putnstr(buf + *अङ्क, wlen);
		(*अङ्क)++;
	}
}

स्थिर शून्य cread_add_str(वर्ण *str, पूर्णाङ्क strsize, पूर्णाङ्क घुसाउ,
			  uint *अङ्क, uint *eol_अङ्क, वर्ण *buf, uint len)
{
	जबसम्म (strsize--) {
		cread_add_char(*str, घुसाउ, अङ्क, eol_अङ्क, buf, len);
		str++;
	}
}

पूर्णाङ्क cread_पङ्क्ति_process_ch(संरचना cli_पङ्क्ति_state *cls, वर्ण ichar)
{
	वर्ण *buf = cls->buf;

	/* ichar=0x0 when error occurs in U-Boot getc */
	यदि (!ichar)
		फर्काऊ -EAGAIN;

	यदि (ichar == '\n') {
		अ_दो('\n');
		buf[cls->eol_अङ्क] = '\0';	/* terminate the string */
		फर्काऊ 0;
	}

	विकल्प (ichar) {
	पक्ष CTL_CH('a'):
		BEGINNING_OF_पङ्क्ति();
		रोक;
	पक्ष CTL_CH('c'):	/* ^C - break */
		*buf = '\0';	/* discard input */
		फर्काऊ -EINTR;
	पक्ष CTL_CH('f'):
		यदि (cls->अङ्क < cls->eol_अङ्क) {
			getcmd_putch(buf[cls->अङ्क]);
			cls->अङ्क++;
		}
		रोक;
	पक्ष CTL_CH('b'):
		यदि (cls->अङ्क) {
			getcmd_putch(CTL_BACKSPACE);
			cls->अङ्क--;
		}
		रोक;
	पक्ष CTL_CH('d'):
		यदि (cls->अङ्क < cls->eol_अङ्क) {
			uint wlen;

			wlen = cls->eol_अङ्क - cls->अङ्क - 1;
			यदि (wlen) {
				स_हटाओ(&buf[cls->अङ्क], &buf[cls->अङ्क + 1],
					wlen);
				putnstr(buf + cls->अङ्क, wlen);
			}

			getcmd_putch(' ');
			गर {
				getcmd_putch(CTL_BACKSPACE);
			} जबसम्म (wlen--);
			cls->eol_अङ्क--;
		}
		रोक;
	पक्ष CTL_CH('k'):
		ERASE_TO_EOL();
		रोक;
	पक्ष CTL_CH('e'):
		REFRESH_TO_EOL();
		रोक;
	पक्ष CTL_CH('o'):
		cls->घुसाउ = !cls->घुसाउ;
		रोक;
	पक्ष CTL_CH('w'):
		यदि (cls->अङ्क) {
			uint base, wlen;

			लागि (base = cls->अङ्क;
			     base > 0 && buf[base - 1] == ' ';)
				base--;
			लागि (; base > 0 && buf[base - 1] != ' ';)
				base--;

			/* now delete chars from base to cls->num */
			wlen = cls->अङ्क - base;
			cls->eol_अङ्क -= wlen;
			स_हटाओ(&buf[base], &buf[cls->अङ्क],
				cls->eol_अङ्क - base + 1);
			cls->अङ्क = base;
			getcmd_putchars(wlen, CTL_BACKSPACE);
			माला_दो(buf + base);
			getcmd_putchars(wlen, ' ');
			getcmd_putchars(wlen + cls->eol_अङ्क - cls->अङ्क,
					CTL_BACKSPACE);
		}
		रोक;
	पक्ष CTL_CH('x'):
	पक्ष CTL_CH('u'):
		BEGINNING_OF_पङ्क्ति();
		ERASE_TO_EOL();
		रोक;
	पक्ष DEL:
	पक्ष DEL7:
	पक्ष 8:
		यदि (cls->अङ्क) {
			uint wlen;

			wlen = cls->eol_अङ्क - cls->अङ्क;
			cls->अङ्क--;
			स_हटाओ(&buf[cls->अङ्क], &buf[cls->अङ्क + 1], wlen);
			getcmd_putch(CTL_BACKSPACE);
			putnstr(buf + cls->अङ्क, wlen);
			getcmd_putch(' ');
			गर {
				getcmd_putch(CTL_BACKSPACE);
			} जबसम्म (wlen--);
			cls->eol_अङ्क--;
		}
		रोक;
	पक्ष CTL_CH('p'):
	पक्ष CTL_CH('n'):
		यदि (cls->history) {
			वर्ण *hline;

			यदि (ichar == CTL_CH('p'))
				hline = hist_prev();
			अन्यथा
				hline = hist_next();

			यदि (!hline) {
				getcmd_cbeep();
				रोक;
			}

			/* nuke the current line */
			/* first, go home */
			BEGINNING_OF_पङ्क्ति();

			/* erase to end of line */
			ERASE_TO_EOL();

			/* copy new line into place and display */
			म_नकल(buf, hline);
			cls->eol_अङ्क = म_माप(buf);
			REFRESH_TO_EOL();
			रोक;
		}
		रोक;
	पक्ष '\t':
		यदि (IS_ENABLED(CONFIG_AUTO_COMPLETE) && cls->cmd_complete) {
			पूर्णाङ्क num2, col;

			/* do not autocomplete when in the middle */
			यदि (cls->अङ्क < cls->eol_अङ्क) {
				getcmd_cbeep();
				रोक;
			}

			buf[cls->अङ्क] = '\0';
			col = म_माप(cls->prompt) + cls->eol_अङ्क;
			num2 = cls->अङ्क;
			यदि (cmd_auto_complete(cls->prompt, buf, &num2, &col)) {
				col = num2 - cls->अङ्क;
				cls->अङ्क += col;
				cls->eol_अङ्क += col;
			}
			रोक;
		}
		fallthrough;
	स्वभाव:
		cread_add_char(ichar, cls->घुसाउ, &cls->अङ्क, &cls->eol_अङ्क,
			       buf, cls->len);
		रोक;
	}

	/*
	 * keep the string terminated...if we added a char at the end then we
	 * want a \0 after it
	 */
	buf[cls->eol_अङ्क] = '\0';

	फर्काऊ -EAGAIN;
}

शून्य cli_cread_init(संरचना cli_पङ्क्ति_state *cls, वर्ण *buf, uint buf_size)
{
	पूर्णाङ्क init_len = म_माप(buf);

	स_रखो(cls, '\0', परिमाण(संरचना cli_पङ्क्ति_state));
	cls->घुसाउ = true;
	cls->buf = buf;
	cls->len = buf_size;

	यदि (init_len)
		cread_add_str(buf, init_len, 0, &cls->अङ्क, &cls->eol_अङ्क, buf,
			      buf_size);
}

स्थिर पूर्णाङ्क cread_पङ्क्ति(अचर वर्ण *अचर prompt, वर्ण *buf, अचिन्हित पूर्णाङ्क *len,
		      पूर्णाङ्क timeout)
{
	संरचना cli_ch_state s_cch, *cch = &s_cch;
	संरचना cli_पङ्क्ति_state s_cls, *cls = &s_cls;
	वर्ण ichar;
	पूर्णाङ्क first = 1;

	cli_ch_init(cch);
	cli_cread_init(cls, buf, *len);
	cls->prompt = prompt;
	cls->history = true;
	cls->cmd_complete = true;

	जबसम्म (1) {
		पूर्णाङ्क ret;

		/* Check for saved characters */
		ichar = cli_ch_process(cch, 0);

		यदि (!ichar) {
			यदि (bootretry_tstc_timeout())
				फर्काऊ -2;	/* timed out */
			यदि (first && timeout) {
				u64 etime = endtick(timeout);

				जबसम्म (!tstc()) {	/* while no incoming data */
					यदि (get_ticks() >= etime)
						फर्काऊ -2;	/* timed out */
					schedule();
				}
				first = 0;
			}

			ichar = getcmd_getch();
			ichar = cli_ch_process(cch, ichar);
		}

		ret = cread_पङ्क्ति_process_ch(cls, ichar);
		यदि (ret == -EINTR)
			फर्काऊ -1;
		अन्यथा यदि (!ret)
			रोक;
	}
	*len = cls->eol_अङ्क;

	यदि (buf[0] && buf[0] != CREAD_HIST_CHAR)
		cread_add_to_hist(buf);
	hist_cur = hist_add_idx;

	फर्काऊ 0;
}

#अन्यथा /* !CONFIG_CMDLINE_EDITING */

स्थिर अन्तःस्थ पूर्णाङ्क hist_init(शून्य)
{
	फर्काऊ 0;
}

स्थिर पूर्णाङ्क cread_पङ्क्ति(अचर वर्ण *अचर prompt, वर्ण *buf, अचिन्हित पूर्णाङ्क *len,
		      पूर्णाङ्क timeout)
{
	फर्काऊ 0;
}

#endif /* CONFIG_CMDLINE_EDITING */

/****************************************************************************/

पूर्णाङ्क cli_readline(अचर वर्ण *अचर prompt)
{
	/*
	 * If console_buffer isn't 0-length the user will be prompted to modify
	 * it instead of entering it from scratch as desired.
	 */
	console_buffer[0] = '\0';

	फर्काऊ cli_readline_into_buffer(prompt, console_buffer, 0);
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
स्थिर पूर्णाङ्क cread_पङ्क्ति_simple(अचर वर्ण *अचर prompt, वर्ण *p)
{
	वर्ण *p_buf = p;
	पूर्णाङ्क n = 0;		/* buffer index */
	पूर्णाङ्क plen = 0;		/* prompt length */
	पूर्णाङ्क col;		/* output column cnt */
	पूर्णाङ्क c;

	/* print prompt */
	यदि (prompt) {
		plen = म_माप(prompt);
		माला_दो(prompt);
	}
	col = plen;

	लागि (;;) {
		यदि (bootretry_tstc_timeout())
			फर्काऊ -2;	/* timed out */
		schedule();	/* Trigger watchdog, if needed */

		c = अक्षर_लो();

		/*
		 * Special character handling
		 */
		विकल्प (c) {
		पक्ष '\r':			/* Enter		*/
		पक्ष '\n':
			*p = '\0';
			माला_दो("\r\n");
			फर्काऊ p - p_buf;

		पक्ष '\0':			/* nul			*/
			जारी;

		पक्ष 0x03:			/* ^C - break		*/
			p_buf[0] = '\0';	/* discard input */
			फर्काऊ -1;

		पक्ष 0x15:			/* ^U - erase line	*/
			जबसम्म (col > plen) {
				माला_दो(erase_seq);
				--col;
			}
			p = p_buf;
			n = 0;
			जारी;

		पक्ष 0x17:			/* ^W - erase word	*/
			p = delete_char(p_buf, p, &col, &n, plen);
			जबसम्म ((n > 0) && (*p != ' '))
				p = delete_char(p_buf, p, &col, &n, plen);
			जारी;

		पक्ष 0x08:			/* ^H  - backspace	*/
		पक्ष 0x7F:			/* DEL - backspace	*/
			p = delete_char(p_buf, p, &col, &n, plen);
			जारी;

		स्वभाव:
			/* Must be a normal character then */
			यदि (n >= CONFIG_SYS_CBSIZE - 2) { /* Buffer full */
				अ_दो('\a');
				रोक;
			}
			यदि (c == '\t') {	/* expand TABs */
				यदि (IS_ENABLED(CONFIG_AUTO_COMPLETE)) {
					/*
					 * if auto-completion triggered just
					 * continue
					 */
					*p = '\0';
					यदि (cmd_auto_complete(prompt,
							      console_buffer,
							      &n, &col)) {
						p = p_buf + n;	/* reset */
						जारी;
					}
				}
				माला_दो(tab_seq + (col & 07));
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
				माला_दो(buf);
			}
			*p++ = c;
			++n;
			रोक;
		}
	}
}

पूर्णाङ्क cli_readline_into_buffer(अचर वर्ण *अचर prompt, वर्ण *buffer,
			     पूर्णाङ्क timeout)
{
	वर्ण *p = buffer;
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
			माला_दो(prompt);

		rc = cread_पङ्क्ति(prompt, p, &len, timeout);
		फर्काऊ rc < 0 ? rc : len;

	} अन्यथा {
		फर्काऊ cread_पङ्क्ति_simple(prompt, p);
	}
}
