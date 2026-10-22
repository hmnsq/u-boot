// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Add to readline cmdline-editing by
 * (C) Copyright 2005
 * JinHua Luo, GuangDong Linux Center, <luo.jinhua@gd-linux.com>
 */

#അന്തര്ഭാവ <bootretry.h>
#അന്തര്ഭാവ <cli.h>
#അന്തര്ഭാവ <command.h>
#അന്തര്ഭാവ <hang.h>
#അന്തര്ഭാവ <malloc.h>
#അന്തര്ഭാവ <time.h>
#അന്തര്ഭാവ <watchdog.h>
#അന്തര്ഭാവ <linux/errno.h>
#അന്തര്ഭാവ <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

സ്ഥിതം സ്ഥിരം അക്ഷരം erase_seq[] = "\b \b";	/* erase sequence */
സ്ഥിതം സ്ഥിരം അക്ഷരം   tab_seq[] = "        ";	/* used to expand TABs */

അക്ഷരം console_buffer[CONFIG_SYS_CBSIZE + 1];	/* console I/O buffer	*/

സ്ഥിതം അക്ഷരം *delete_char (അക്ഷരം *buffer, അക്ഷരം *p, പൂർണ്ണസംഖ്യ *colp, പൂർണ്ണസംഖ്യ *np, പൂർണ്ണസംഖ്യ plen)
{
	അക്ഷരം *s;

	എങ്കിൽ (*np == 0)
		തിരികെ p;

	എങ്കിൽ (*(--p) == '\t') {		/* will retype the whole line */
		ആകുമ്പോൾ (*colp > plen) {
			puts(erase_seq);
			(*colp)--;
		}
		വേണ്ടി (s = buffer; s < p; ++s) {
			എങ്കിൽ (*s == '\t') {
				puts(tab_seq + ((*colp) & 07));
				*colp += 8 - ((*colp) & 07);
			} അല്ലെങ്കിൽ {
				++(*colp);
				putc(*s);
			}
		}
	} അല്ലെങ്കിൽ {
		puts(erase_seq);
		(*colp)--;
	}
	(*np)--;

	തിരികെ p;
}

#ifdef CONFIG_CMDLINE_EDITING

/*
 * cmdline-editing related codes from vivi.
 * Author: Janghoon Lyu <nandy@mizi.com>
 */

#define putnstr(str, n)	printf("%.*s", (പൂർണ്ണസംഖ്യ)n, str)

#define CTL_BACKSPACE		('\b')
#define DEL			((അക്ഷരം)255)
#define DEL7			((അക്ഷരം)127)
#define CREAD_HIST_CHAR		('!')

#define getcmd_putch(ch)	putc(ch)
#define getcmd_getch()		getchar()
#define getcmd_cbeep()		getcmd_putch('\a')

#ifdef CONFIG_XPL_BUILD
#define HIST_MAX		3
#define HIST_SIZE		32
#അല്ലെങ്കിൽ
#define HIST_MAX		20
#define HIST_SIZE		CONFIG_SYS_CBSIZE
#endif

സ്ഥിതം പൂർണ്ണസംഖ്യ hist_max;
സ്ഥിതം പൂർണ്ണസംഖ്യ hist_add_idx;
സ്ഥിതം പൂർണ്ണസംഖ്യ hist_cur = -1;
സ്ഥിതം unsigned hist_അക്കം;

#ifndef CONFIG_CMD_HISTORY_USE_CALLOC
സ്ഥിതം അക്ഷരം hist_data[HIST_MAX][HIST_SIZE + 1];
#endif
സ്ഥിതം അക്ഷരം *hist_list[HIST_MAX];

#define add_idx_minus_one() ((hist_add_idx == 0) ? hist_max : hist_add_idx-1)

സ്ഥിതം ശൂന്യം getcmd_putchars(പൂർണ്ണസംഖ്യ എണ്ണം, പൂർണ്ണസംഖ്യ ch)
{
	പൂർണ്ണസംഖ്യ i;

	വേണ്ടി (i = 0; i < എണ്ണം; i++)
		getcmd_putch(ch);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ hist_init(ശൂന്യം)
{
	പൂർണ്ണസംഖ്യ i;

#ifndef CONFIG_CMD_HISTORY_USE_CALLOC
	വേണ്ടി (i = 0; i < HIST_MAX; i++) {
		hist_list[i] = hist_data[i];
		hist_list[i][0] = '\0';
	}
#അല്ലെങ്കിൽ
	unsigned അക്ഷരം *hist = calloc(HIST_MAX, HIST_SIZE + 1);
	എങ്കിൽ (!hist)
		panic("%s: calloc: out of memory!\n", __func__);

	വേണ്ടി (i = 0; i < HIST_MAX; i++)
		hist_list[i] = hist + (i * (HIST_SIZE + 1));
#endif

	hist_max = 0;
	hist_add_idx = 0;
	hist_cur = -1;
	hist_അക്കം = 0;

	തിരികെ 0;
}

സ്ഥിതം ശൂന്യം cread_add_to_hist(അക്ഷരം *വരി)
{
	strcpy(hist_list[hist_add_idx], വരി);

	എങ്കിൽ (++hist_add_idx >= HIST_MAX)
		hist_add_idx = 0;

	എങ്കിൽ (hist_add_idx > hist_max)
		hist_max = hist_add_idx;

	hist_അക്കം++;
}

സ്ഥിതം അക്ഷരം *hist_prev(ശൂന്യം)
{
	അക്ഷരം *ret;
	പൂർണ്ണസംഖ്യ old_cur;

	എങ്കിൽ (hist_cur < 0)
		തിരികെ ശൂന്യ;

	old_cur = hist_cur;
	എങ്കിൽ (--hist_cur < 0)
		hist_cur = hist_max;

	എങ്കിൽ (hist_cur == hist_add_idx) {
		hist_cur = old_cur;
		ret = ശൂന്യ;
	} അല്ലെങ്കിൽ {
		ret = hist_list[hist_cur];
	}

	തിരികെ ret;
}

സ്ഥിതം അക്ഷരം *hist_next(ശൂന്യം)
{
	അക്ഷരം *ret;

	എങ്കിൽ (hist_cur < 0)
		തിരികെ ശൂന്യ;

	എങ്കിൽ (hist_cur == hist_add_idx)
		തിരികെ ശൂന്യ;

	എങ്കിൽ (++hist_cur > hist_max)
		hist_cur = 0;

	എങ്കിൽ (hist_cur == hist_add_idx)
		ret = "";
	അല്ലെങ്കിൽ
		ret = hist_list[hist_cur];

	തിരികെ ret;
}

ശൂന്യം cread_print_hist_list(ശൂന്യം)
{
	പൂർണ്ണസംഖ്യ i;
	uint n;

	n = hist_അക്കം - hist_max;

	i = hist_add_idx + 1;
	ആകുമ്പോൾ (1) {
		എങ്കിൽ (i > hist_max)
			i = 0;
		എങ്കിൽ (i == hist_add_idx)
			നിർത്തുക;
		printf("%s\n", hist_list[i]);
		n++;
		i++;
	}
}

#define BEGINNING_OF_വരി() {			\
	ആകുമ്പോൾ (cls->അക്കം) {			\
		getcmd_putch(CTL_BACKSPACE);	\
		cls->അക്കം--;			\
	}					\
}

#define ERASE_TO_EOL() {				\
	എങ്കിൽ (cls->അക്കം < cls->eol_അക്കം) {		\
		printf("%*s", (പൂർണ്ണസംഖ്യ)(cls->eol_അക്കം - cls->അക്കം), ""); \
		ചെയ്യുക {					\
			getcmd_putch(CTL_BACKSPACE);	\
		} ആകുമ്പോൾ (--cls->eol_അക്കം > cls->അക്കം);	\
	}						\
}

#define REFRESH_TO_EOL() {				\
	എങ്കിൽ (cls->അക്കം < cls->eol_അക്കം) {			\
		uint wlen = cls->eol_അക്കം - cls->അക്കം;	\
		putnstr(buf + cls->അക്കം, wlen);		\
		cls->അക്കം = cls->eol_അക്കം;		\
	}						\
}

സ്ഥിതം ശൂന്യം cread_add_char(അക്ഷരം ichar, പൂർണ്ണസംഖ്യ തിരുകുക, uint *അക്കം,
			   uint *eol_അക്കം, അക്ഷരം *buf, uint len)
{
	uint wlen;

	/* room ??? */
	എങ്കിൽ (തിരുകുക || *അക്കം == *eol_അക്കം) {
		എങ്കിൽ (*eol_അക്കം > len - 1) {
			getcmd_cbeep();
			തിരികെ;
		}
		(*eol_അക്കം)++;
	}

	എങ്കിൽ (തിരുകുക) {
		wlen = *eol_അക്കം - *അക്കം;
		എങ്കിൽ (wlen > 1)
			memmove(&buf[*അക്കം+1], &buf[*അക്കം], wlen-1);

		buf[*അക്കം] = ichar;
		putnstr(buf + *അക്കം, wlen);
		(*അക്കം)++;
		ആകുമ്പോൾ (--wlen)
			getcmd_putch(CTL_BACKSPACE);
	} അല്ലെങ്കിൽ {
		/* echo the character */
		wlen = 1;
		buf[*അക്കം] = ichar;
		putnstr(buf + *അക്കം, wlen);
		(*അക്കം)++;
	}
}

സ്ഥിതം ശൂന്യം cread_add_str(അക്ഷരം *str, പൂർണ്ണസംഖ്യ strsize, പൂർണ്ണസംഖ്യ തിരുകുക,
			  uint *അക്കം, uint *eol_അക്കം, അക്ഷരം *buf, uint len)
{
	ആകുമ്പോൾ (strsize--) {
		cread_add_char(*str, തിരുകുക, അക്കം, eol_അക്കം, buf, len);
		str++;
	}
}

പൂർണ്ണസംഖ്യ cread_വരി_process_ch(ഘടന cli_വരി_state *cls, അക്ഷരം ichar)
{
	അക്ഷരം *buf = cls->buf;

	/* ichar=0x0 when error occurs in U-Boot getc */
	എങ്കിൽ (!ichar)
		തിരികെ -EAGAIN;

	എങ്കിൽ (ichar == '\n') {
		putc('\n');
		buf[cls->eol_അക്കം] = '\0';	/* terminate the string */
		തിരികെ 0;
	}

	തിരഞ്ഞെടുപ്പ് (ichar) {
	അവസ്ഥ CTL_CH('a'):
		BEGINNING_OF_വരി();
		നിർത്തുക;
	അവസ്ഥ CTL_CH('c'):	/* ^C - break */
		*buf = '\0';	/* discard input */
		തിരികെ -EINTR;
	അവസ്ഥ CTL_CH('f'):
		എങ്കിൽ (cls->അക്കം < cls->eol_അക്കം) {
			getcmd_putch(buf[cls->അക്കം]);
			cls->അക്കം++;
		}
		നിർത്തുക;
	അവസ്ഥ CTL_CH('b'):
		എങ്കിൽ (cls->അക്കം) {
			getcmd_putch(CTL_BACKSPACE);
			cls->അക്കം--;
		}
		നിർത്തുക;
	അവസ്ഥ CTL_CH('d'):
		എങ്കിൽ (cls->അക്കം < cls->eol_അക്കം) {
			uint wlen;

			wlen = cls->eol_അക്കം - cls->അക്കം - 1;
			എങ്കിൽ (wlen) {
				memmove(&buf[cls->അക്കം], &buf[cls->അക്കം + 1],
					wlen);
				putnstr(buf + cls->അക്കം, wlen);
			}

			getcmd_putch(' ');
			ചെയ്യുക {
				getcmd_putch(CTL_BACKSPACE);
			} ആകുമ്പോൾ (wlen--);
			cls->eol_അക്കം--;
		}
		നിർത്തുക;
	അവസ്ഥ CTL_CH('k'):
		ERASE_TO_EOL();
		നിർത്തുക;
	അവസ്ഥ CTL_CH('e'):
		REFRESH_TO_EOL();
		നിർത്തുക;
	അവസ്ഥ CTL_CH('o'):
		cls->തിരുകുക = !cls->തിരുകുക;
		നിർത്തുക;
	അവസ്ഥ CTL_CH('w'):
		എങ്കിൽ (cls->അക്കം) {
			uint base, wlen;

			വേണ്ടി (base = cls->അക്കം;
			     base > 0 && buf[base - 1] == ' ';)
				base--;
			വേണ്ടി (; base > 0 && buf[base - 1] != ' ';)
				base--;

			/* now delete chars from base to cls->num */
			wlen = cls->അക്കം - base;
			cls->eol_അക്കം -= wlen;
			memmove(&buf[base], &buf[cls->അക്കം],
				cls->eol_അക്കം - base + 1);
			cls->അക്കം = base;
			getcmd_putchars(wlen, CTL_BACKSPACE);
			puts(buf + base);
			getcmd_putchars(wlen, ' ');
			getcmd_putchars(wlen + cls->eol_അക്കം - cls->അക്കം,
					CTL_BACKSPACE);
		}
		നിർത്തുക;
	അവസ്ഥ CTL_CH('x'):
	അവസ്ഥ CTL_CH('u'):
		BEGINNING_OF_വരി();
		ERASE_TO_EOL();
		നിർത്തുക;
	അവസ്ഥ DEL:
	അവസ്ഥ DEL7:
	അവസ്ഥ 8:
		എങ്കിൽ (cls->അക്കം) {
			uint wlen;

			wlen = cls->eol_അക്കം - cls->അക്കം;
			cls->അക്കം--;
			memmove(&buf[cls->അക്കം], &buf[cls->അക്കം + 1], wlen);
			getcmd_putch(CTL_BACKSPACE);
			putnstr(buf + cls->അക്കം, wlen);
			getcmd_putch(' ');
			ചെയ്യുക {
				getcmd_putch(CTL_BACKSPACE);
			} ആകുമ്പോൾ (wlen--);
			cls->eol_അക്കം--;
		}
		നിർത്തുക;
	അവസ്ഥ CTL_CH('p'):
	അവസ്ഥ CTL_CH('n'):
		എങ്കിൽ (cls->history) {
			അക്ഷരം *hline;

			എങ്കിൽ (ichar == CTL_CH('p'))
				hline = hist_prev();
			അല്ലെങ്കിൽ
				hline = hist_next();

			എങ്കിൽ (!hline) {
				getcmd_cbeep();
				നിർത്തുക;
			}

			/* nuke the current line */
			/* first, go home */
			BEGINNING_OF_വരി();

			/* erase to end of line */
			ERASE_TO_EOL();

			/* copy new line into place and display */
			strcpy(buf, hline);
			cls->eol_അക്കം = strlen(buf);
			REFRESH_TO_EOL();
			നിർത്തുക;
		}
		നിർത്തുക;
	അവസ്ഥ '\t':
		എങ്കിൽ (IS_ENABLED(CONFIG_AUTO_COMPLETE) && cls->cmd_complete) {
			പൂർണ്ണസംഖ്യ num2, col;

			/* do not autocomplete when in the middle */
			എങ്കിൽ (cls->അക്കം < cls->eol_അക്കം) {
				getcmd_cbeep();
				നിർത്തുക;
			}

			buf[cls->അക്കം] = '\0';
			col = strlen(cls->prompt) + cls->eol_അക്കം;
			num2 = cls->അക്കം;
			എങ്കിൽ (cmd_auto_complete(cls->prompt, buf, &num2, &col)) {
				col = num2 - cls->അക്കം;
				cls->അക്കം += col;
				cls->eol_അക്കം += col;
			}
			നിർത്തുക;
		}
		fallthrough;
	സ്ഥിരസ്ഥിതി:
		cread_add_char(ichar, cls->തിരുകുക, &cls->അക്കം, &cls->eol_അക്കം,
			       buf, cls->len);
		നിർത്തുക;
	}

	/*
	 * keep the string terminated...if we added a char at the end then we
	 * want a \0 after it
	 */
	buf[cls->eol_അക്കം] = '\0';

	തിരികെ -EAGAIN;
}

ശൂന്യം cli_cread_init(ഘടന cli_വരി_state *cls, അക്ഷരം *buf, uint buf_size)
{
	പൂർണ്ണസംഖ്യ init_len = strlen(buf);

	memset(cls, '\0', വലുപ്പം(ഘടന cli_വരി_state));
	cls->തിരുകുക = true;
	cls->buf = buf;
	cls->len = buf_size;

	എങ്കിൽ (init_len)
		cread_add_str(buf, init_len, 0, &cls->അക്കം, &cls->eol_അക്കം, buf,
			      buf_size);
}

സ്ഥിതം പൂർണ്ണസംഖ്യ cread_വരി(സ്ഥിരം അക്ഷരം *സ്ഥിരം prompt, അക്ഷരം *buf, unsigned പൂർണ്ണസംഖ്യ *len,
		      പൂർണ്ണസംഖ്യ timeout)
{
	ഘടന cli_ch_state s_cch, *cch = &s_cch;
	ഘടന cli_വരി_state s_cls, *cls = &s_cls;
	അക്ഷരം ichar;
	പൂർണ്ണസംഖ്യ first = 1;

	cli_ch_init(cch);
	cli_cread_init(cls, buf, *len);
	cls->prompt = prompt;
	cls->history = true;
	cls->cmd_complete = true;

	ആകുമ്പോൾ (1) {
		പൂർണ്ണസംഖ്യ ret;

		/* Check for saved characters */
		ichar = cli_ch_process(cch, 0);

		എങ്കിൽ (!ichar) {
			എങ്കിൽ (bootretry_tstc_timeout())
				തിരികെ -2;	/* timed out */
			എങ്കിൽ (first && timeout) {
				u64 etime = endtick(timeout);

				ആകുമ്പോൾ (!tstc()) {	/* while no incoming data */
					എങ്കിൽ (get_ticks() >= etime)
						തിരികെ -2;	/* timed out */
					schedule();
				}
				first = 0;
			}

			ichar = getcmd_getch();
			ichar = cli_ch_process(cch, ichar);
		}

		ret = cread_വരി_process_ch(cls, ichar);
		എങ്കിൽ (ret == -EINTR)
			തിരികെ -1;
		അല്ലെങ്കിൽ എങ്കിൽ (!ret)
			നിർത്തുക;
	}
	*len = cls->eol_അക്കം;

	എങ്കിൽ (buf[0] && buf[0] != CREAD_HIST_CHAR)
		cread_add_to_hist(buf);
	hist_cur = hist_add_idx;

	തിരികെ 0;
}

#അല്ലെങ്കിൽ /* !CONFIG_CMDLINE_EDITING */

സ്ഥിതം വരിയിൽ പൂർണ്ണസംഖ്യ hist_init(ശൂന്യം)
{
	തിരികെ 0;
}

സ്ഥിതം പൂർണ്ണസംഖ്യ cread_വരി(സ്ഥിരം അക്ഷരം *സ്ഥിരം prompt, അക്ഷരം *buf, unsigned പൂർണ്ണസംഖ്യ *len,
		      പൂർണ്ണസംഖ്യ timeout)
{
	തിരികെ 0;
}

#endif /* CONFIG_CMDLINE_EDITING */

/****************************************************************************/

പൂർണ്ണസംഖ്യ cli_readline(സ്ഥിരം അക്ഷരം *സ്ഥിരം prompt)
{
	/*
	 * If console_buffer isn't 0-length the user will be prompted to modify
	 * it instead of entering it from scratch as desired.
	 */
	console_buffer[0] = '\0';

	തിരികെ cli_readline_into_buffer(prompt, console_buffer, 0);
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
സ്ഥിതം പൂർണ്ണസംഖ്യ cread_വരി_simple(സ്ഥിരം അക്ഷരം *സ്ഥിരം prompt, അക്ഷരം *p)
{
	അക്ഷരം *p_buf = p;
	പൂർണ്ണസംഖ്യ n = 0;		/* buffer index */
	പൂർണ്ണസംഖ്യ plen = 0;		/* prompt length */
	പൂർണ്ണസംഖ്യ col;		/* output column cnt */
	പൂർണ്ണസംഖ്യ c;

	/* print prompt */
	എങ്കിൽ (prompt) {
		plen = strlen(prompt);
		puts(prompt);
	}
	col = plen;

	വേണ്ടി (;;) {
		എങ്കിൽ (bootretry_tstc_timeout())
			തിരികെ -2;	/* timed out */
		schedule();	/* Trigger watchdog, if needed */

		c = getchar();

		/*
		 * Special character handling
		 */
		തിരഞ്ഞെടുപ്പ് (c) {
		അവസ്ഥ '\r':			/* Enter		*/
		അവസ്ഥ '\n':
			*p = '\0';
			puts("\r\n");
			തിരികെ p - p_buf;

		അവസ്ഥ '\0':			/* nul			*/
			തുടരുക;

		അവസ്ഥ 0x03:			/* ^C - break		*/
			p_buf[0] = '\0';	/* discard input */
			തിരികെ -1;

		അവസ്ഥ 0x15:			/* ^U - erase line	*/
			ആകുമ്പോൾ (col > plen) {
				puts(erase_seq);
				--col;
			}
			p = p_buf;
			n = 0;
			തുടരുക;

		അവസ്ഥ 0x17:			/* ^W - erase word	*/
			p = delete_char(p_buf, p, &col, &n, plen);
			ആകുമ്പോൾ ((n > 0) && (*p != ' '))
				p = delete_char(p_buf, p, &col, &n, plen);
			തുടരുക;

		അവസ്ഥ 0x08:			/* ^H  - backspace	*/
		അവസ്ഥ 0x7F:			/* DEL - backspace	*/
			p = delete_char(p_buf, p, &col, &n, plen);
			തുടരുക;

		സ്ഥിരസ്ഥിതി:
			/* Must be a normal character then */
			എങ്കിൽ (n >= CONFIG_SYS_CBSIZE - 2) { /* Buffer full */
				putc('\a');
				നിർത്തുക;
			}
			എങ്കിൽ (c == '\t') {	/* expand TABs */
				എങ്കിൽ (IS_ENABLED(CONFIG_AUTO_COMPLETE)) {
					/*
					 * if auto-completion triggered just
					 * continue
					 */
					*p = '\0';
					എങ്കിൽ (cmd_auto_complete(prompt,
							      console_buffer,
							      &n, &col)) {
						p = p_buf + n;	/* reset */
						തുടരുക;
					}
				}
				puts(tab_seq + (col & 07));
				col += 8 - (col & 07);
			} അല്ലെങ്കിൽ {
				അക്ഷരം __maybe_unused buf[2];

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
			നിർത്തുക;
		}
	}
}

പൂർണ്ണസംഖ്യ cli_readline_into_buffer(സ്ഥിരം അക്ഷരം *സ്ഥിരം prompt, അക്ഷരം *buffer,
			     പൂർണ്ണസംഖ്യ timeout)
{
	അക്ഷരം *p = buffer;
	uint len = CONFIG_SYS_CBSIZE;
	പൂർണ്ണസംഖ്യ rc;
	സ്ഥിതം പൂർണ്ണസംഖ്യ initted;

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
	എങ്കിൽ (IS_ENABLED(CONFIG_CMDLINE_EDITING) && (gd->flags & GD_FLG_RELOC)) {
		എങ്കിൽ (!initted) {
			rc = hist_init();
			എങ്കിൽ (rc == 0)
				initted = 1;
		}

		എങ്കിൽ (prompt)
			puts(prompt);

		rc = cread_വരി(prompt, p, &len, timeout);
		തിരികെ rc < 0 ? rc : len;

	} അല്ലെങ്കിൽ {
		തിരികെ cread_വരി_simple(prompt, p);
	}
}
