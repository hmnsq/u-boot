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

स्थिर ध्रुव वर्ण erase_seq[] = "\b \b";	/* erase sequence */
स्थिर ध्रुव वर्ण   tab_seq[] = "        ";	/* used to expand TABs */

वर्ण console_buffer[CONFIG_SYS_CBSIZE + 1];	/* console I/O buffer	*/

स्थिर वर्ण *delete_char (वर्ण *buffer, वर्ण *p, पूर्णाङ्क *colp, पूर्णाङ्क *np, पूर्णाङ्क plen)
{
	वर्ण *s;

	सचे (*np == 0)
		पटिनिवत्त p;

	सचे (*(--p) == '\t') {		/* will retype the whole line */
		याव (*colp > plen) {
			puts(erase_seq);
			(*colp)--;
		}
		अत्थाय (s = buffer; s < p; ++s) {
			सचे (*s == '\t') {
				puts(tab_seq + ((*colp) & 07));
				*colp += 8 - ((*colp) & 07);
			} अञ्ञथा {
				++(*colp);
				putc(*s);
			}
		}
	} अञ्ञथा {
		puts(erase_seq);
		(*colp)--;
	}
	(*np)--;

	पटिनिवत्त p;
}

#ifdef CONFIG_CMDLINE_EDITING

/*
 * cmdline-editing related codes from vivi.
 * Author: Janghoon Lyu <nandy@mizi.com>
 */

#define putnstr(str, n)	printf("%.*s", (पूर्णाङ्क)n, str)

#define CTL_BACKSPACE		('\b')
#define DEL			((वर्ण)255)
#define DEL7			((वर्ण)127)
#define CREAD_HIST_CHAR		('!')

#define getcmd_putch(ch)	putc(ch)
#define getcmd_getch()		getchar()
#define getcmd_cbeep()		getcmd_putch('\a')

#ifdef CONFIG_XPL_BUILD
#define HIST_MAX		3
#define HIST_SIZE		32
#अञ्ञथा
#define HIST_MAX		20
#define HIST_SIZE		CONFIG_SYS_CBSIZE
#endif

स्थिर पूर्णाङ्क hist_max;
स्थिर पूर्णाङ्क hist_add_idx;
स्थिर पूर्णाङ्क hist_cur = -1;
स्थिर unsigned hist_अङ्कः;

#ifndef CONFIG_CMD_HISTORY_USE_CALLOC
स्थिर वर्ण hist_data[HIST_MAX][HIST_SIZE + 1];
#endif
स्थिर वर्ण *hist_list[HIST_MAX];

#define add_idx_minus_one() ((hist_add_idx == 0) ? hist_max : hist_add_idx-1)

स्थिर शून्य getcmd_putchars(पूर्णाङ्क गणनम्, पूर्णाङ्क ch)
{
	पूर्णाङ्क i;

	अत्थाय (i = 0; i < गणनम्; i++)
		getcmd_putch(ch);
}

स्थिर पूर्णाङ्क hist_init(शून्य)
{
	पूर्णाङ्क i;

#ifndef CONFIG_CMD_HISTORY_USE_CALLOC
	अत्थाय (i = 0; i < HIST_MAX; i++) {
		hist_list[i] = hist_data[i];
		hist_list[i][0] = '\0';
	}
#अञ्ञथा
	unsigned वर्ण *hist = calloc(HIST_MAX, HIST_SIZE + 1);
	सचे (!hist)
		panic("%s: calloc: out of memory!\n", __func__);

	अत्थाय (i = 0; i < HIST_MAX; i++)
		hist_list[i] = hist + (i * (HIST_SIZE + 1));
#endif

	hist_max = 0;
	hist_add_idx = 0;
	hist_cur = -1;
	hist_अङ्कः = 0;

	पटिनिवत्त 0;
}

स्थिर शून्य cread_add_to_hist(वर्ण *पङ्क्तिः)
{
	strcpy(hist_list[hist_add_idx], पङ्क्तिः);

	सचे (++hist_add_idx >= HIST_MAX)
		hist_add_idx = 0;

	सचे (hist_add_idx > hist_max)
		hist_max = hist_add_idx;

	hist_अङ्कः++;
}

स्थिर वर्ण *hist_prev(शून्य)
{
	वर्ण *ret;
	पूर्णाङ्क old_cur;

	सचे (hist_cur < 0)
		पटिनिवत्त रिक्त;

	old_cur = hist_cur;
	सचे (--hist_cur < 0)
		hist_cur = hist_max;

	सचे (hist_cur == hist_add_idx) {
		hist_cur = old_cur;
		ret = रिक्त;
	} अञ्ञथा {
		ret = hist_list[hist_cur];
	}

	पटिनिवत्त ret;
}

स्थिर वर्ण *hist_next(शून्य)
{
	वर्ण *ret;

	सचे (hist_cur < 0)
		पटिनिवत्त रिक्त;

	सचे (hist_cur == hist_add_idx)
		पटिनिवत्त रिक्त;

	सचे (++hist_cur > hist_max)
		hist_cur = 0;

	सचे (hist_cur == hist_add_idx)
		ret = "";
	अञ्ञथा
		ret = hist_list[hist_cur];

	पटिनिवत्त ret;
}

शून्य cread_print_hist_list(शून्य)
{
	पूर्णाङ्क i;
	uint n;

	n = hist_अङ्कः - hist_max;

	i = hist_add_idx + 1;
	याव (1) {
		सचे (i > hist_max)
			i = 0;
		सचे (i == hist_add_idx)
			भङ्ग;
		printf("%s\n", hist_list[i]);
		n++;
		i++;
	}
}

#define BEGINNING_OF_पङ्क्तिः() {			\
	याव (cls->अङ्कः) {			\
		getcmd_putch(CTL_BACKSPACE);	\
		cls->अङ्कः--;			\
	}					\
}

#define ERASE_TO_EOL() {				\
	सचे (cls->अङ्कः < cls->eol_अङ्कः) {		\
		printf("%*s", (पूर्णाङ्क)(cls->eol_अङ्कः - cls->अङ्कः), ""); \
		करोहि {					\
			getcmd_putch(CTL_BACKSPACE);	\
		} याव (--cls->eol_अङ्कः > cls->अङ्कः);	\
	}						\
}

#define REFRESH_TO_EOL() {				\
	सचे (cls->अङ्कः < cls->eol_अङ्कः) {			\
		uint wlen = cls->eol_अङ्कः - cls->अङ्कः;	\
		putnstr(buf + cls->अङ्कः, wlen);		\
		cls->अङ्कः = cls->eol_अङ्कः;		\
	}						\
}

स्थिर शून्य cread_add_char(वर्ण ichar, पूर्णाङ्क निवेशय, uint *अङ्कः,
			   uint *eol_अङ्कः, वर्ण *buf, uint len)
{
	uint wlen;

	/* room ??? */
	सचे (निवेशय || *अङ्कः == *eol_अङ्कः) {
		सचे (*eol_अङ्कः > len - 1) {
			getcmd_cbeep();
			पटिनिवत्त;
		}
		(*eol_अङ्कः)++;
	}

	सचे (निवेशय) {
		wlen = *eol_अङ्कः - *अङ्कः;
		सचे (wlen > 1)
			memmove(&buf[*अङ्कः+1], &buf[*अङ्कः], wlen-1);

		buf[*अङ्कः] = ichar;
		putnstr(buf + *अङ्कः, wlen);
		(*अङ्कः)++;
		याव (--wlen)
			getcmd_putch(CTL_BACKSPACE);
	} अञ्ञथा {
		/* echo the character */
		wlen = 1;
		buf[*अङ्कः] = ichar;
		putnstr(buf + *अङ्कः, wlen);
		(*अङ्कः)++;
	}
}

स्थिर शून्य cread_add_str(वर्ण *str, पूर्णाङ्क strsize, पूर्णाङ्क निवेशय,
			  uint *अङ्कः, uint *eol_अङ्कः, वर्ण *buf, uint len)
{
	याव (strsize--) {
		cread_add_char(*str, निवेशय, अङ्कः, eol_अङ्कः, buf, len);
		str++;
	}
}

पूर्णाङ्क cread_पङ्क्तिः_process_ch(संरचना cli_पङ्क्तिः_state *cls, वर्ण ichar)
{
	वर्ण *buf = cls->buf;

	/* ichar=0x0 when error occurs in U-Boot getc */
	सचे (!ichar)
		पटिनिवत्त -EAGAIN;

	सचे (ichar == '\n') {
		putc('\n');
		buf[cls->eol_अङ्कः] = '\0';	/* terminate the string */
		पटिनिवत्त 0;
	}

	विकल्प (ichar) {
	पक्ष CTL_CH('a'):
		BEGINNING_OF_पङ्क्तिः();
		भङ्ग;
	पक्ष CTL_CH('c'):	/* ^C - break */
		*buf = '\0';	/* discard input */
		पटिनिवत्त -EINTR;
	पक्ष CTL_CH('f'):
		सचे (cls->अङ्कः < cls->eol_अङ्कः) {
			getcmd_putch(buf[cls->अङ्कः]);
			cls->अङ्कः++;
		}
		भङ्ग;
	पक्ष CTL_CH('b'):
		सचे (cls->अङ्कः) {
			getcmd_putch(CTL_BACKSPACE);
			cls->अङ्कः--;
		}
		भङ्ग;
	पक्ष CTL_CH('d'):
		सचे (cls->अङ्कः < cls->eol_अङ्कः) {
			uint wlen;

			wlen = cls->eol_अङ्कः - cls->अङ्कः - 1;
			सचे (wlen) {
				memmove(&buf[cls->अङ्कः], &buf[cls->अङ्कः + 1],
					wlen);
				putnstr(buf + cls->अङ्कः, wlen);
			}

			getcmd_putch(' ');
			करोहि {
				getcmd_putch(CTL_BACKSPACE);
			} याव (wlen--);
			cls->eol_अङ्कः--;
		}
		भङ्ग;
	पक्ष CTL_CH('k'):
		ERASE_TO_EOL();
		भङ्ग;
	पक्ष CTL_CH('e'):
		REFRESH_TO_EOL();
		भङ्ग;
	पक्ष CTL_CH('o'):
		cls->निवेशय = !cls->निवेशय;
		भङ्ग;
	पक्ष CTL_CH('w'):
		सचे (cls->अङ्कः) {
			uint base, wlen;

			अत्थाय (base = cls->अङ्कः;
			     base > 0 && buf[base - 1] == ' ';)
				base--;
			अत्थाय (; base > 0 && buf[base - 1] != ' ';)
				base--;

			/* now delete chars from base to cls->num */
			wlen = cls->अङ्कः - base;
			cls->eol_अङ्कः -= wlen;
			memmove(&buf[base], &buf[cls->अङ्कः],
				cls->eol_अङ्कः - base + 1);
			cls->अङ्कः = base;
			getcmd_putchars(wlen, CTL_BACKSPACE);
			puts(buf + base);
			getcmd_putchars(wlen, ' ');
			getcmd_putchars(wlen + cls->eol_अङ्कः - cls->अङ्कः,
					CTL_BACKSPACE);
		}
		भङ्ग;
	पक्ष CTL_CH('x'):
	पक्ष CTL_CH('u'):
		BEGINNING_OF_पङ्क्तिः();
		ERASE_TO_EOL();
		भङ्ग;
	पक्ष DEL:
	पक्ष DEL7:
	पक्ष 8:
		सचे (cls->अङ्कः) {
			uint wlen;

			wlen = cls->eol_अङ्कः - cls->अङ्कः;
			cls->अङ्कः--;
			memmove(&buf[cls->अङ्कः], &buf[cls->अङ्कः + 1], wlen);
			getcmd_putch(CTL_BACKSPACE);
			putnstr(buf + cls->अङ्कः, wlen);
			getcmd_putch(' ');
			करोहि {
				getcmd_putch(CTL_BACKSPACE);
			} याव (wlen--);
			cls->eol_अङ्कः--;
		}
		भङ्ग;
	पक्ष CTL_CH('p'):
	पक्ष CTL_CH('n'):
		सचे (cls->history) {
			वर्ण *hline;

			सचे (ichar == CTL_CH('p'))
				hline = hist_prev();
			अञ्ञथा
				hline = hist_next();

			सचे (!hline) {
				getcmd_cbeep();
				भङ्ग;
			}

			/* nuke the current line */
			/* first, go home */
			BEGINNING_OF_पङ्क्तिः();

			/* erase to end of line */
			ERASE_TO_EOL();

			/* copy new line into place and display */
			strcpy(buf, hline);
			cls->eol_अङ्कः = strlen(buf);
			REFRESH_TO_EOL();
			भङ्ग;
		}
		भङ्ग;
	पक्ष '\t':
		सचे (IS_ENABLED(CONFIG_AUTO_COMPLETE) && cls->cmd_complete) {
			पूर्णाङ्क num2, col;

			/* do not autocomplete when in the middle */
			सचे (cls->अङ्कः < cls->eol_अङ्कः) {
				getcmd_cbeep();
				भङ्ग;
			}

			buf[cls->अङ्कः] = '\0';
			col = strlen(cls->prompt) + cls->eol_अङ्कः;
			num2 = cls->अङ्कः;
			सचे (cmd_auto_complete(cls->prompt, buf, &num2, &col)) {
				col = num2 - cls->अङ्कः;
				cls->अङ्कः += col;
				cls->eol_अङ्कः += col;
			}
			भङ्ग;
		}
		fallthrough;
	स्वभाव:
		cread_add_char(ichar, cls->निवेशय, &cls->अङ्कः, &cls->eol_अङ्कः,
			       buf, cls->len);
		भङ्ग;
	}

	/*
	 * keep the string terminated...if we added a char at the end then we
	 * want a \0 after it
	 */
	buf[cls->eol_अङ्कः] = '\0';

	पटिनिवत्त -EAGAIN;
}

शून्य cli_cread_init(संरचना cli_पङ्क्तिः_state *cls, वर्ण *buf, uint buf_size)
{
	पूर्णाङ्क init_len = strlen(buf);

	memset(cls, '\0', परिमाण(संरचना cli_पङ्क्तिः_state));
	cls->निवेशय = true;
	cls->buf = buf;
	cls->len = buf_size;

	सचे (init_len)
		cread_add_str(buf, init_len, 0, &cls->अङ्कः, &cls->eol_अङ्कः, buf,
			      buf_size);
}

स्थिर पूर्णाङ्क cread_पङ्क्तिः(ध्रुव वर्ण *ध्रुव prompt, वर्ण *buf, unsigned पूर्णाङ्क *len,
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

	याव (1) {
		पूर्णाङ्क ret;

		/* Check for saved characters */
		ichar = cli_ch_process(cch, 0);

		सचे (!ichar) {
			सचे (bootretry_tstc_timeout())
				पटिनिवत्त -2;	/* timed out */
			सचे (first && timeout) {
				u64 etime = endtick(timeout);

				याव (!tstc()) {	/* while no incoming data */
					सचे (get_ticks() >= etime)
						पटिनिवत्त -2;	/* timed out */
					schedule();
				}
				first = 0;
			}

			ichar = getcmd_getch();
			ichar = cli_ch_process(cch, ichar);
		}

		ret = cread_पङ्क्तिः_process_ch(cls, ichar);
		सचे (ret == -EINTR)
			पटिनिवत्त -1;
		अञ्ञथा सचे (!ret)
			भङ्ग;
	}
	*len = cls->eol_अङ्कः;

	सचे (buf[0] && buf[0] != CREAD_HIST_CHAR)
		cread_add_to_hist(buf);
	hist_cur = hist_add_idx;

	पटिनिवत्त 0;
}

#अञ्ञथा /* !CONFIG_CMDLINE_EDITING */

स्थिर अन्तःस्थ पूर्णाङ्क hist_init(शून्य)
{
	पटिनिवत्त 0;
}

स्थिर पूर्णाङ्क cread_पङ्क्तिः(ध्रुव वर्ण *ध्रुव prompt, वर्ण *buf, unsigned पूर्णाङ्क *len,
		      पूर्णाङ्क timeout)
{
	पटिनिवत्त 0;
}

#endif /* CONFIG_CMDLINE_EDITING */

/****************************************************************************/

पूर्णाङ्क cli_readline(ध्रुव वर्ण *ध्रुव prompt)
{
	/*
	 * If console_buffer isn't 0-length the user will be prompted to modify
	 * it instead of entering it from scratch as desired.
	 */
	console_buffer[0] = '\0';

	पटिनिवत्त cli_readline_into_buffer(prompt, console_buffer, 0);
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
	सचे (prompt) {
		plen = strlen(prompt);
		puts(prompt);
	}
	col = plen;

	अत्थाय (;;) {
		सचे (bootretry_tstc_timeout())
			पटिनिवत्त -2;	/* timed out */
		schedule();	/* Trigger watchdog, if needed */

		c = getchar();

		/*
		 * Special character handling
		 */
		विकल्प (c) {
		पक्ष '\r':			/* Enter		*/
		पक्ष '\n':
			*p = '\0';
			puts("\r\n");
			पटिनिवत्त p - p_buf;

		पक्ष '\0':			/* nul			*/
			अनुवर्त;

		पक्ष 0x03:			/* ^C - break		*/
			p_buf[0] = '\0';	/* discard input */
			पटिनिवत्त -1;

		पक्ष 0x15:			/* ^U - erase line	*/
			याव (col > plen) {
				puts(erase_seq);
				--col;
			}
			p = p_buf;
			n = 0;
			अनुवर्त;

		पक्ष 0x17:			/* ^W - erase word	*/
			p = delete_char(p_buf, p, &col, &n, plen);
			याव ((n > 0) && (*p != ' '))
				p = delete_char(p_buf, p, &col, &n, plen);
			अनुवर्त;

		पक्ष 0x08:			/* ^H  - backspace	*/
		पक्ष 0x7F:			/* DEL - backspace	*/
			p = delete_char(p_buf, p, &col, &n, plen);
			अनुवर्त;

		स्वभाव:
			/* Must be a normal character then */
			सचे (n >= CONFIG_SYS_CBSIZE - 2) { /* Buffer full */
				putc('\a');
				भङ्ग;
			}
			सचे (c == '\t') {	/* expand TABs */
				सचे (IS_ENABLED(CONFIG_AUTO_COMPLETE)) {
					/*
					 * if auto-completion triggered just
					 * continue
					 */
					*p = '\0';
					सचे (cmd_auto_complete(prompt,
							      console_buffer,
							      &n, &col)) {
						p = p_buf + n;	/* reset */
						अनुवर्त;
					}
				}
				puts(tab_seq + (col & 07));
				col += 8 - (col & 07);
			} अञ्ञथा {
				वर्ण __maybe_unused buf[2];

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
			भङ्ग;
		}
	}
}

पूर्णाङ्क cli_readline_into_buffer(ध्रुव वर्ण *ध्रुव prompt, वर्ण *buffer,
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
	सचे (IS_ENABLED(CONFIG_CMDLINE_EDITING) && (gd->flags & GD_FLG_RELOC)) {
		सचे (!initted) {
			rc = hist_init();
			सचे (rc == 0)
				initted = 1;
		}

		सचे (prompt)
			puts(prompt);

		rc = cread_पङ्क्तिः(prompt, p, &len, timeout);
		पटिनिवत्त rc < 0 ? rc : len;

	} अञ्ञथा {
		पटिनिवत्त cread_पङ्क्तिः_simple(prompt, p);
	}
}
