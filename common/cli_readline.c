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

	यदि (*np == 0)
		प्रत्यागम p;

	यदि (*(--p) == '\t') {		/* will retype the whole line */
		यावत् (*colp > plen) {
			puts(erase_seq);
			(*colp)--;
		}
		प्रति (s = buffer; s < p; ++s) {
			यदि (*s == '\t') {
				puts(tab_seq + ((*colp) & 07));
				*colp += 8 - ((*colp) & 07);
			} अन्यथा {
				++(*colp);
				putc(*s);
			}
		}
	} अन्यथा {
		puts(erase_seq);
		(*colp)--;
	}
	(*np)--;

	प्रत्यागम p;
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
#अन्यथा
#define HIST_MAX		20
#define HIST_SIZE		CONFIG_SYS_CBSIZE
#endif

स्थिर पूर्णाङ्क hist_max;
स्थिर पूर्णाङ्क hist_add_idx;
स्थिर पूर्णाङ्क hist_cur = -1;
स्थिर unsigned hist_अंक;

#ifndef CONFIG_CMD_HISTORY_USE_CALLOC
स्थिर वर्ण hist_data[HIST_MAX][HIST_SIZE + 1];
#endif
स्थिर वर्ण *hist_list[HIST_MAX];

#define add_idx_minus_one() ((hist_add_idx == 0) ? hist_max : hist_add_idx-1)

स्थिर शून्य getcmd_putchars(पूर्णाङ्क गिनती, पूर्णाङ्क ch)
{
	पूर्णाङ्क i;

	प्रति (i = 0; i < गिनती; i++)
		getcmd_putch(ch);
}

स्थिर पूर्णाङ्क hist_init(शून्य)
{
	पूर्णाङ्क i;

#ifndef CONFIG_CMD_HISTORY_USE_CALLOC
	प्रति (i = 0; i < HIST_MAX; i++) {
		hist_list[i] = hist_data[i];
		hist_list[i][0] = '\0';
	}
#अन्यथा
	unsigned वर्ण *hist = calloc(HIST_MAX, HIST_SIZE + 1);
	यदि (!hist)
		panic("%s: calloc: out of memory!\n", __func__);

	प्रति (i = 0; i < HIST_MAX; i++)
		hist_list[i] = hist + (i * (HIST_SIZE + 1));
#endif

	hist_max = 0;
	hist_add_idx = 0;
	hist_cur = -1;
	hist_अंक = 0;

	प्रत्यागम 0;
}

स्थिर शून्य cread_add_to_hist(वर्ण *पंक्ति)
{
	strcpy(hist_list[hist_add_idx], पंक्ति);

	यदि (++hist_add_idx >= HIST_MAX)
		hist_add_idx = 0;

	यदि (hist_add_idx > hist_max)
		hist_max = hist_add_idx;

	hist_अंक++;
}

स्थिर वर्ण *hist_prev(शून्य)
{
	वर्ण *ret;
	पूर्णाङ्क old_cur;

	यदि (hist_cur < 0)
		प्रत्यागम रिक्त;

	old_cur = hist_cur;
	यदि (--hist_cur < 0)
		hist_cur = hist_max;

	यदि (hist_cur == hist_add_idx) {
		hist_cur = old_cur;
		ret = रिक्त;
	} अन्यथा {
		ret = hist_list[hist_cur];
	}

	प्रत्यागम ret;
}

स्थिर वर्ण *hist_next(शून्य)
{
	वर्ण *ret;

	यदि (hist_cur < 0)
		प्रत्यागम रिक्त;

	यदि (hist_cur == hist_add_idx)
		प्रत्यागम रिक्त;

	यदि (++hist_cur > hist_max)
		hist_cur = 0;

	यदि (hist_cur == hist_add_idx)
		ret = "";
	अन्यथा
		ret = hist_list[hist_cur];

	प्रत्यागम ret;
}

शून्य cread_print_hist_list(शून्य)
{
	पूर्णाङ्क i;
	uint n;

	n = hist_अंक - hist_max;

	i = hist_add_idx + 1;
	यावत् (1) {
		यदि (i > hist_max)
			i = 0;
		यदि (i == hist_add_idx)
			भङ्ग;
		printf("%s\n", hist_list[i]);
		n++;
		i++;
	}
}

#define BEGINNING_OF_पंक्ति() {			\
	यावत् (cls->अंक) {			\
		getcmd_putch(CTL_BACKSPACE);	\
		cls->अंक--;			\
	}					\
}

#define ERASE_TO_EOL() {				\
	यदि (cls->अंक < cls->eol_अंक) {		\
		printf("%*s", (पूर्णाङ्क)(cls->eol_अंक - cls->अंक), ""); \
		कुरु {					\
			getcmd_putch(CTL_BACKSPACE);	\
		} यावत् (--cls->eol_अंक > cls->अंक);	\
	}						\
}

#define REFRESH_TO_EOL() {				\
	यदि (cls->अंक < cls->eol_अंक) {			\
		uint wlen = cls->eol_अंक - cls->अंक;	\
		putnstr(buf + cls->अंक, wlen);		\
		cls->अंक = cls->eol_अंक;		\
	}						\
}

स्थिर शून्य cread_add_char(वर्ण ichar, पूर्णाङ्क जोड़ो, uint *अंक,
			   uint *eol_अंक, वर्ण *buf, uint len)
{
	uint wlen;

	/* room ??? */
	यदि (जोड़ो || *अंक == *eol_अंक) {
		यदि (*eol_अंक > len - 1) {
			getcmd_cbeep();
			प्रत्यागम;
		}
		(*eol_अंक)++;
	}

	यदि (जोड़ो) {
		wlen = *eol_अंक - *अंक;
		यदि (wlen > 1)
			memmove(&buf[*अंक+1], &buf[*अंक], wlen-1);

		buf[*अंक] = ichar;
		putnstr(buf + *अंक, wlen);
		(*अंक)++;
		यावत् (--wlen)
			getcmd_putch(CTL_BACKSPACE);
	} अन्यथा {
		/* echo the character */
		wlen = 1;
		buf[*अंक] = ichar;
		putnstr(buf + *अंक, wlen);
		(*अंक)++;
	}
}

स्थिर शून्य cread_add_str(वर्ण *str, पूर्णाङ्क strsize, पूर्णाङ्क जोड़ो,
			  uint *अंक, uint *eol_अंक, वर्ण *buf, uint len)
{
	यावत् (strsize--) {
		cread_add_char(*str, जोड़ो, अंक, eol_अंक, buf, len);
		str++;
	}
}

पूर्णाङ्क cread_पंक्ति_process_ch(संरचना cli_पंक्ति_state *cls, वर्ण ichar)
{
	वर्ण *buf = cls->buf;

	/* ichar=0x0 when error occurs in U-Boot getc */
	यदि (!ichar)
		प्रत्यागम -EAGAIN;

	यदि (ichar == '\n') {
		putc('\n');
		buf[cls->eol_अंक] = '\0';	/* terminate the string */
		प्रत्यागम 0;
	}

	विकल्प (ichar) {
	पक्ष CTL_CH('a'):
		BEGINNING_OF_पंक्ति();
		भङ्ग;
	पक्ष CTL_CH('c'):	/* ^C - break */
		*buf = '\0';	/* discard input */
		प्रत्यागम -EINTR;
	पक्ष CTL_CH('f'):
		यदि (cls->अंक < cls->eol_अंक) {
			getcmd_putch(buf[cls->अंक]);
			cls->अंक++;
		}
		भङ्ग;
	पक्ष CTL_CH('b'):
		यदि (cls->अंक) {
			getcmd_putch(CTL_BACKSPACE);
			cls->अंक--;
		}
		भङ्ग;
	पक्ष CTL_CH('d'):
		यदि (cls->अंक < cls->eol_अंक) {
			uint wlen;

			wlen = cls->eol_अंक - cls->अंक - 1;
			यदि (wlen) {
				memmove(&buf[cls->अंक], &buf[cls->अंक + 1],
					wlen);
				putnstr(buf + cls->अंक, wlen);
			}

			getcmd_putch(' ');
			कुरु {
				getcmd_putch(CTL_BACKSPACE);
			} यावत् (wlen--);
			cls->eol_अंक--;
		}
		भङ्ग;
	पक्ष CTL_CH('k'):
		ERASE_TO_EOL();
		भङ्ग;
	पक्ष CTL_CH('e'):
		REFRESH_TO_EOL();
		भङ्ग;
	पक्ष CTL_CH('o'):
		cls->जोड़ो = !cls->जोड़ो;
		भङ्ग;
	पक्ष CTL_CH('w'):
		यदि (cls->अंक) {
			uint base, wlen;

			प्रति (base = cls->अंक;
			     base > 0 && buf[base - 1] == ' ';)
				base--;
			प्रति (; base > 0 && buf[base - 1] != ' ';)
				base--;

			/* now delete chars from base to cls->num */
			wlen = cls->अंक - base;
			cls->eol_अंक -= wlen;
			memmove(&buf[base], &buf[cls->अंक],
				cls->eol_अंक - base + 1);
			cls->अंक = base;
			getcmd_putchars(wlen, CTL_BACKSPACE);
			puts(buf + base);
			getcmd_putchars(wlen, ' ');
			getcmd_putchars(wlen + cls->eol_अंक - cls->अंक,
					CTL_BACKSPACE);
		}
		भङ्ग;
	पक्ष CTL_CH('x'):
	पक्ष CTL_CH('u'):
		BEGINNING_OF_पंक्ति();
		ERASE_TO_EOL();
		भङ्ग;
	पक्ष DEL:
	पक्ष DEL7:
	पक्ष 8:
		यदि (cls->अंक) {
			uint wlen;

			wlen = cls->eol_अंक - cls->अंक;
			cls->अंक--;
			memmove(&buf[cls->अंक], &buf[cls->अंक + 1], wlen);
			getcmd_putch(CTL_BACKSPACE);
			putnstr(buf + cls->अंक, wlen);
			getcmd_putch(' ');
			कुरु {
				getcmd_putch(CTL_BACKSPACE);
			} यावत् (wlen--);
			cls->eol_अंक--;
		}
		भङ्ग;
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
				भङ्ग;
			}

			/* nuke the current line */
			/* first, go home */
			BEGINNING_OF_पंक्ति();

			/* erase to end of line */
			ERASE_TO_EOL();

			/* copy new line into place and display */
			strcpy(buf, hline);
			cls->eol_अंक = strlen(buf);
			REFRESH_TO_EOL();
			भङ्ग;
		}
		भङ्ग;
	पक्ष '\t':
		यदि (IS_ENABLED(CONFIG_AUTO_COMPLETE) && cls->cmd_complete) {
			पूर्णाङ्क num2, col;

			/* do not autocomplete when in the middle */
			यदि (cls->अंक < cls->eol_अंक) {
				getcmd_cbeep();
				भङ्ग;
			}

			buf[cls->अंक] = '\0';
			col = strlen(cls->prompt) + cls->eol_अंक;
			num2 = cls->अंक;
			यदि (cmd_auto_complete(cls->prompt, buf, &num2, &col)) {
				col = num2 - cls->अंक;
				cls->अंक += col;
				cls->eol_अंक += col;
			}
			भङ्ग;
		}
		fallthrough;
	स्वभाव:
		cread_add_char(ichar, cls->जोड़ो, &cls->अंक, &cls->eol_अंक,
			       buf, cls->len);
		भङ्ग;
	}

	/*
	 * keep the string terminated...if we added a char at the end then we
	 * want a \0 after it
	 */
	buf[cls->eol_अंक] = '\0';

	प्रत्यागम -EAGAIN;
}

शून्य cli_cread_init(संरचना cli_पंक्ति_state *cls, वर्ण *buf, uint buf_size)
{
	पूर्णाङ्क init_len = strlen(buf);

	memset(cls, '\0', परिमाण(संरचना cli_पंक्ति_state));
	cls->जोड़ो = true;
	cls->buf = buf;
	cls->len = buf_size;

	यदि (init_len)
		cread_add_str(buf, init_len, 0, &cls->अंक, &cls->eol_अंक, buf,
			      buf_size);
}

स्थिर पूर्णाङ्क cread_पंक्ति(ध्रुव वर्ण *ध्रुव prompt, वर्ण *buf, unsigned पूर्णाङ्क *len,
		      पूर्णाङ्क timeout)
{
	संरचना cli_ch_state s_cch, *cch = &s_cch;
	संरचना cli_पंक्ति_state s_cls, *cls = &s_cls;
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
					schedule();
				}
				first = 0;
			}

			ichar = getcmd_getch();
			ichar = cli_ch_process(cch, ichar);
		}

		ret = cread_पंक्ति_process_ch(cls, ichar);
		यदि (ret == -EINTR)
			प्रत्यागम -1;
		अन्यथा यदि (!ret)
			भङ्ग;
	}
	*len = cls->eol_अंक;

	यदि (buf[0] && buf[0] != CREAD_HIST_CHAR)
		cread_add_to_hist(buf);
	hist_cur = hist_add_idx;

	प्रत्यागम 0;
}

#अन्यथा /* !CONFIG_CMDLINE_EDITING */

स्थिर अन्तःस्थ पूर्णाङ्क hist_init(शून्य)
{
	प्रत्यागम 0;
}

स्थिर पूर्णाङ्क cread_पंक्ति(ध्रुव वर्ण *ध्रुव prompt, वर्ण *buf, unsigned पूर्णाङ्क *len,
		      पूर्णाङ्क timeout)
{
	प्रत्यागम 0;
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

	प्रत्यागम cli_readline_into_buffer(prompt, console_buffer, 0);
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
स्थिर पूर्णाङ्क cread_पंक्ति_simple(ध्रुव वर्ण *ध्रुव prompt, वर्ण *p)
{
	वर्ण *p_buf = p;
	पूर्णाङ्क n = 0;		/* buffer index */
	पूर्णाङ्क plen = 0;		/* prompt length */
	पूर्णाङ्क col;		/* output column cnt */
	पूर्णाङ्क c;

	/* print prompt */
	यदि (prompt) {
		plen = strlen(prompt);
		puts(prompt);
	}
	col = plen;

	प्रति (;;) {
		यदि (bootretry_tstc_timeout())
			प्रत्यागम -2;	/* timed out */
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
			प्रत्यागम p - p_buf;

		पक्ष '\0':			/* nul			*/
			अनुवर्त;

		पक्ष 0x03:			/* ^C - break		*/
			p_buf[0] = '\0';	/* discard input */
			प्रत्यागम -1;

		पक्ष 0x15:			/* ^U - erase line	*/
			यावत् (col > plen) {
				puts(erase_seq);
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
							      console_buffer,
							      &n, &col)) {
						p = p_buf + n;	/* reset */
						अनुवर्त;
					}
				}
				puts(tab_seq + (col & 07));
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
	यदि (IS_ENABLED(CONFIG_CMDLINE_EDITING) && (gd->flags & GD_FLG_RELOC)) {
		यदि (!initted) {
			rc = hist_init();
			यदि (rc == 0)
				initted = 1;
		}

		यदि (prompt)
			puts(prompt);

		rc = cread_पंक्ति(prompt, p, &len, timeout);
		प्रत्यागम rc < 0 ? rc : len;

	} अन्यथा {
		प्रत्यागम cread_पंक्ति_simple(prompt, p);
	}
}
