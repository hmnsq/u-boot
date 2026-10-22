// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Add to readline cmdline-editing by
 * (C) Copyright 2005
 * JinHua Luo, GuangDong Linux Center, <luo.jinhua@gd-linux.com>
 */

#અન્તર્ભાવ <bootretry.h>
#અન્તર્ભાવ <cli.h>
#અન્તર્ભાવ <command.h>
#અન્તર્ભાવ <hang.h>
#અન્તર્ભાવ <malloc.h>
#અન્તર્ભાવ <time.h>
#અન્તર્ભાવ <watchdog.h>
#અન્તર્ભાવ <linux/errno.h>
#અન્તર્ભાવ <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

સ્થિર અચલ અક્ષર erase_seq[] = "\b \b";	/* erase sequence */
સ્થિર અચલ અક્ષર   tab_seq[] = "        ";	/* used to expand TABs */

અક્ષર console_buffer[CONFIG_SYS_CBSIZE + 1];	/* console I/O buffer	*/

સ્થિર અક્ષર *delete_char (અક્ષર *buffer, અક્ષર *p, પૂર્ણાંક *colp, પૂર્ણાંક *np, પૂર્ણાંક plen)
{
	અક્ષર *s;

	જો (*np == 0)
		પાછા p;

	જો (*(--p) == '\t') {		/* will retype the whole line */
		જ્યારે (*colp > plen) {
			puts(erase_seq);
			(*colp)--;
		}
		માટે (s = buffer; s < p; ++s) {
			જો (*s == '\t') {
				puts(tab_seq + ((*colp) & 07));
				*colp += 8 - ((*colp) & 07);
			} નહીંતર {
				++(*colp);
				putc(*s);
			}
		}
	} નહીંતર {
		puts(erase_seq);
		(*colp)--;
	}
	(*np)--;

	પાછા p;
}

#ifdef CONFIG_CMDLINE_EDITING

/*
 * cmdline-editing related codes from vivi.
 * Author: Janghoon Lyu <nandy@mizi.com>
 */

#define putnstr(str, n)	printf("%.*s", (પૂર્ણાંક)n, str)

#define CTL_BACKSPACE		('\b')
#define DEL			((અક્ષર)255)
#define DEL7			((અક્ષર)127)
#define CREAD_HIST_CHAR		('!')

#define getcmd_putch(ch)	putc(ch)
#define getcmd_getch()		getchar()
#define getcmd_cbeep()		getcmd_putch('\a')

#ifdef CONFIG_XPL_BUILD
#define HIST_MAX		3
#define HIST_SIZE		32
#નહીંતર
#define HIST_MAX		20
#define HIST_SIZE		CONFIG_SYS_CBSIZE
#endif

સ્થિર પૂર્ણાંક hist_max;
સ્થિર પૂર્ણાંક hist_add_idx;
સ્થિર પૂર્ણાંક hist_cur = -1;
સ્થિર unsigned hist_અંક;

#ifndef CONFIG_CMD_HISTORY_USE_CALLOC
સ્થિર અક્ષર hist_data[HIST_MAX][HIST_SIZE + 1];
#endif
સ્થિર અક્ષર *hist_list[HIST_MAX];

#define add_idx_minus_one() ((hist_add_idx == 0) ? hist_max : hist_add_idx-1)

સ્થિર શૂન્ય getcmd_putchars(પૂર્ણાંક ગણતરી, પૂર્ણાંક ch)
{
	પૂર્ણાંક i;

	માટે (i = 0; i < ગણતરી; i++)
		getcmd_putch(ch);
}

સ્થિર પૂર્ણાંક hist_init(શૂન્ય)
{
	પૂર્ણાંક i;

#ifndef CONFIG_CMD_HISTORY_USE_CALLOC
	માટે (i = 0; i < HIST_MAX; i++) {
		hist_list[i] = hist_data[i];
		hist_list[i][0] = '\0';
	}
#નહીંતર
	unsigned અક્ષર *hist = calloc(HIST_MAX, HIST_SIZE + 1);
	જો (!hist)
		panic("%s: calloc: out of memory!\n", __func__);

	માટે (i = 0; i < HIST_MAX; i++)
		hist_list[i] = hist + (i * (HIST_SIZE + 1));
#endif

	hist_max = 0;
	hist_add_idx = 0;
	hist_cur = -1;
	hist_અંક = 0;

	પાછા 0;
}

સ્થિર શૂન્ય cread_add_to_hist(અક્ષર *લીટી)
{
	strcpy(hist_list[hist_add_idx], લીટી);

	જો (++hist_add_idx >= HIST_MAX)
		hist_add_idx = 0;

	જો (hist_add_idx > hist_max)
		hist_max = hist_add_idx;

	hist_અંક++;
}

સ્થિર અક્ષર *hist_prev(શૂન્ય)
{
	અક્ષર *ret;
	પૂર્ણાંક old_cur;

	જો (hist_cur < 0)
		પાછા ખાલી;

	old_cur = hist_cur;
	જો (--hist_cur < 0)
		hist_cur = hist_max;

	જો (hist_cur == hist_add_idx) {
		hist_cur = old_cur;
		ret = ખાલી;
	} નહીંતર {
		ret = hist_list[hist_cur];
	}

	પાછા ret;
}

સ્થિર અક્ષર *hist_next(શૂન્ય)
{
	અક્ષર *ret;

	જો (hist_cur < 0)
		પાછા ખાલી;

	જો (hist_cur == hist_add_idx)
		પાછા ખાલી;

	જો (++hist_cur > hist_max)
		hist_cur = 0;

	જો (hist_cur == hist_add_idx)
		ret = "";
	નહીંતર
		ret = hist_list[hist_cur];

	પાછા ret;
}

શૂન્ય cread_print_hist_list(શૂન્ય)
{
	પૂર્ણાંક i;
	uint n;

	n = hist_અંક - hist_max;

	i = hist_add_idx + 1;
	જ્યારે (1) {
		જો (i > hist_max)
			i = 0;
		જો (i == hist_add_idx)
			તોડો;
		printf("%s\n", hist_list[i]);
		n++;
		i++;
	}
}

#define BEGINNING_OF_લીટી() {			\
	જ્યારે (cls->અંક) {			\
		getcmd_putch(CTL_BACKSPACE);	\
		cls->અંક--;			\
	}					\
}

#define ERASE_TO_EOL() {				\
	જો (cls->અંક < cls->eol_અંક) {		\
		printf("%*s", (પૂર્ણાંક)(cls->eol_અંક - cls->અંક), ""); \
		કરો {					\
			getcmd_putch(CTL_BACKSPACE);	\
		} જ્યારે (--cls->eol_અંક > cls->અંક);	\
	}						\
}

#define REFRESH_TO_EOL() {				\
	જો (cls->અંક < cls->eol_અંક) {			\
		uint wlen = cls->eol_અંક - cls->અંક;	\
		putnstr(buf + cls->અંક, wlen);		\
		cls->અંક = cls->eol_અંક;		\
	}						\
}

સ્થિર શૂન્ય cread_add_char(અક્ષર ichar, પૂર્ણાંક ઉમેરો, uint *અંક,
			   uint *eol_અંક, અક્ષર *buf, uint len)
{
	uint wlen;

	/* room ??? */
	જો (ઉમેરો || *અંક == *eol_અંક) {
		જો (*eol_અંક > len - 1) {
			getcmd_cbeep();
			પાછા;
		}
		(*eol_અંક)++;
	}

	જો (ઉમેરો) {
		wlen = *eol_અંક - *અંક;
		જો (wlen > 1)
			memmove(&buf[*અંક+1], &buf[*અંક], wlen-1);

		buf[*અંક] = ichar;
		putnstr(buf + *અંક, wlen);
		(*અંક)++;
		જ્યારે (--wlen)
			getcmd_putch(CTL_BACKSPACE);
	} નહીંતર {
		/* echo the character */
		wlen = 1;
		buf[*અંક] = ichar;
		putnstr(buf + *અંક, wlen);
		(*અંક)++;
	}
}

સ્થિર શૂન્ય cread_add_str(અક્ષર *str, પૂર્ણાંક strsize, પૂર્ણાંક ઉમેરો,
			  uint *અંક, uint *eol_અંક, અક્ષર *buf, uint len)
{
	જ્યારે (strsize--) {
		cread_add_char(*str, ઉમેરો, અંક, eol_અંક, buf, len);
		str++;
	}
}

પૂર્ણાંક cread_લીટી_process_ch(સંરચના cli_લીટી_state *cls, અક્ષર ichar)
{
	અક્ષર *buf = cls->buf;

	/* ichar=0x0 when error occurs in U-Boot getc */
	જો (!ichar)
		પાછા -EAGAIN;

	જો (ichar == '\n') {
		putc('\n');
		buf[cls->eol_અંક] = '\0';	/* terminate the string */
		પાછા 0;
	}

	પસંદગી (ichar) {
	સ્થિતિ CTL_CH('a'):
		BEGINNING_OF_લીટી();
		તોડો;
	સ્થિતિ CTL_CH('c'):	/* ^C - break */
		*buf = '\0';	/* discard input */
		પાછા -EINTR;
	સ્થિતિ CTL_CH('f'):
		જો (cls->અંક < cls->eol_અંક) {
			getcmd_putch(buf[cls->અંક]);
			cls->અંક++;
		}
		તોડો;
	સ્થિતિ CTL_CH('b'):
		જો (cls->અંક) {
			getcmd_putch(CTL_BACKSPACE);
			cls->અંક--;
		}
		તોડો;
	સ્થિતિ CTL_CH('d'):
		જો (cls->અંક < cls->eol_અંક) {
			uint wlen;

			wlen = cls->eol_અંક - cls->અંક - 1;
			જો (wlen) {
				memmove(&buf[cls->અંક], &buf[cls->અંક + 1],
					wlen);
				putnstr(buf + cls->અંક, wlen);
			}

			getcmd_putch(' ');
			કરો {
				getcmd_putch(CTL_BACKSPACE);
			} જ્યારે (wlen--);
			cls->eol_અંક--;
		}
		તોડો;
	સ્થિતિ CTL_CH('k'):
		ERASE_TO_EOL();
		તોડો;
	સ્થિતિ CTL_CH('e'):
		REFRESH_TO_EOL();
		તોડો;
	સ્થિતિ CTL_CH('o'):
		cls->ઉમેરો = !cls->ઉમેરો;
		તોડો;
	સ્થિતિ CTL_CH('w'):
		જો (cls->અંક) {
			uint base, wlen;

			માટે (base = cls->અંક;
			     base > 0 && buf[base - 1] == ' ';)
				base--;
			માટે (; base > 0 && buf[base - 1] != ' ';)
				base--;

			/* now delete chars from base to cls->num */
			wlen = cls->અંક - base;
			cls->eol_અંક -= wlen;
			memmove(&buf[base], &buf[cls->અંક],
				cls->eol_અંક - base + 1);
			cls->અંક = base;
			getcmd_putchars(wlen, CTL_BACKSPACE);
			puts(buf + base);
			getcmd_putchars(wlen, ' ');
			getcmd_putchars(wlen + cls->eol_અંક - cls->અંક,
					CTL_BACKSPACE);
		}
		તોડો;
	સ્થિતિ CTL_CH('x'):
	સ્થિતિ CTL_CH('u'):
		BEGINNING_OF_લીટી();
		ERASE_TO_EOL();
		તોડો;
	સ્થિતિ DEL:
	સ્થિતિ DEL7:
	સ્થિતિ 8:
		જો (cls->અંક) {
			uint wlen;

			wlen = cls->eol_અંક - cls->અંક;
			cls->અંક--;
			memmove(&buf[cls->અંક], &buf[cls->અંક + 1], wlen);
			getcmd_putch(CTL_BACKSPACE);
			putnstr(buf + cls->અંક, wlen);
			getcmd_putch(' ');
			કરો {
				getcmd_putch(CTL_BACKSPACE);
			} જ્યારે (wlen--);
			cls->eol_અંક--;
		}
		તોડો;
	સ્થિતિ CTL_CH('p'):
	સ્થિતિ CTL_CH('n'):
		જો (cls->history) {
			અક્ષર *hline;

			જો (ichar == CTL_CH('p'))
				hline = hist_prev();
			નહીંતર
				hline = hist_next();

			જો (!hline) {
				getcmd_cbeep();
				તોડો;
			}

			/* nuke the current line */
			/* first, go home */
			BEGINNING_OF_લીટી();

			/* erase to end of line */
			ERASE_TO_EOL();

			/* copy new line into place and display */
			strcpy(buf, hline);
			cls->eol_અંક = strlen(buf);
			REFRESH_TO_EOL();
			તોડો;
		}
		તોડો;
	સ્થિતિ '\t':
		જો (IS_ENABLED(CONFIG_AUTO_COMPLETE) && cls->cmd_complete) {
			પૂર્ણાંક num2, col;

			/* do not autocomplete when in the middle */
			જો (cls->અંક < cls->eol_અંક) {
				getcmd_cbeep();
				તોડો;
			}

			buf[cls->અંક] = '\0';
			col = strlen(cls->prompt) + cls->eol_અંક;
			num2 = cls->અંક;
			જો (cmd_auto_complete(cls->prompt, buf, &num2, &col)) {
				col = num2 - cls->અંક;
				cls->અંક += col;
				cls->eol_અંક += col;
			}
			તોડો;
		}
		fallthrough;
	મૂળભૂત:
		cread_add_char(ichar, cls->ઉમેરો, &cls->અંક, &cls->eol_અંક,
			       buf, cls->len);
		તોડો;
	}

	/*
	 * keep the string terminated...if we added a char at the end then we
	 * want a \0 after it
	 */
	buf[cls->eol_અંક] = '\0';

	પાછા -EAGAIN;
}

શૂન્ય cli_cread_init(સંરચના cli_લીટી_state *cls, અક્ષર *buf, uint buf_size)
{
	પૂર્ણાંક init_len = strlen(buf);

	memset(cls, '\0', કદ(સંરચના cli_લીટી_state));
	cls->ઉમેરો = true;
	cls->buf = buf;
	cls->len = buf_size;

	જો (init_len)
		cread_add_str(buf, init_len, 0, &cls->અંક, &cls->eol_અંક, buf,
			      buf_size);
}

સ્થિર પૂર્ણાંક cread_લીટી(અચલ અક્ષર *અચલ prompt, અક્ષર *buf, unsigned પૂર્ણાંક *len,
		      પૂર્ણાંક timeout)
{
	સંરચના cli_ch_state s_cch, *cch = &s_cch;
	સંરચના cli_લીટી_state s_cls, *cls = &s_cls;
	અક્ષર ichar;
	પૂર્ણાંક first = 1;

	cli_ch_init(cch);
	cli_cread_init(cls, buf, *len);
	cls->prompt = prompt;
	cls->history = true;
	cls->cmd_complete = true;

	જ્યારે (1) {
		પૂર્ણાંક ret;

		/* Check for saved characters */
		ichar = cli_ch_process(cch, 0);

		જો (!ichar) {
			જો (bootretry_tstc_timeout())
				પાછા -2;	/* timed out */
			જો (first && timeout) {
				u64 etime = endtick(timeout);

				જ્યારે (!tstc()) {	/* while no incoming data */
					જો (get_ticks() >= etime)
						પાછા -2;	/* timed out */
					schedule();
				}
				first = 0;
			}

			ichar = getcmd_getch();
			ichar = cli_ch_process(cch, ichar);
		}

		ret = cread_લીટી_process_ch(cls, ichar);
		જો (ret == -EINTR)
			પાછા -1;
		નહીંતર જો (!ret)
			તોડો;
	}
	*len = cls->eol_અંક;

	જો (buf[0] && buf[0] != CREAD_HIST_CHAR)
		cread_add_to_hist(buf);
	hist_cur = hist_add_idx;

	પાછા 0;
}

#નહીંતર /* !CONFIG_CMDLINE_EDITING */

સ્થિર અંતઃરેખ પૂર્ણાંક hist_init(શૂન્ય)
{
	પાછા 0;
}

સ્થિર પૂર્ણાંક cread_લીટી(અચલ અક્ષર *અચલ prompt, અક્ષર *buf, unsigned પૂર્ણાંક *len,
		      પૂર્ણાંક timeout)
{
	પાછા 0;
}

#endif /* CONFIG_CMDLINE_EDITING */

/****************************************************************************/

પૂર્ણાંક cli_readline(અચલ અક્ષર *અચલ prompt)
{
	/*
	 * If console_buffer isn't 0-length the user will be prompted to modify
	 * it instead of entering it from scratch as desired.
	 */
	console_buffer[0] = '\0';

	પાછા cli_readline_into_buffer(prompt, console_buffer, 0);
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
સ્થિર પૂર્ણાંક cread_લીટી_simple(અચલ અક્ષર *અચલ prompt, અક્ષર *p)
{
	અક્ષર *p_buf = p;
	પૂર્ણાંક n = 0;		/* buffer index */
	પૂર્ણાંક plen = 0;		/* prompt length */
	પૂર્ણાંક col;		/* output column cnt */
	પૂર્ણાંક c;

	/* print prompt */
	જો (prompt) {
		plen = strlen(prompt);
		puts(prompt);
	}
	col = plen;

	માટે (;;) {
		જો (bootretry_tstc_timeout())
			પાછા -2;	/* timed out */
		schedule();	/* Trigger watchdog, if needed */

		c = getchar();

		/*
		 * Special character handling
		 */
		પસંદગી (c) {
		સ્થિતિ '\r':			/* Enter		*/
		સ્થિતિ '\n':
			*p = '\0';
			puts("\r\n");
			પાછા p - p_buf;

		સ્થિતિ '\0':			/* nul			*/
			ચાલુ;

		સ્થિતિ 0x03:			/* ^C - break		*/
			p_buf[0] = '\0';	/* discard input */
			પાછા -1;

		સ્થિતિ 0x15:			/* ^U - erase line	*/
			જ્યારે (col > plen) {
				puts(erase_seq);
				--col;
			}
			p = p_buf;
			n = 0;
			ચાલુ;

		સ્થિતિ 0x17:			/* ^W - erase word	*/
			p = delete_char(p_buf, p, &col, &n, plen);
			જ્યારે ((n > 0) && (*p != ' '))
				p = delete_char(p_buf, p, &col, &n, plen);
			ચાલુ;

		સ્થિતિ 0x08:			/* ^H  - backspace	*/
		સ્થિતિ 0x7F:			/* DEL - backspace	*/
			p = delete_char(p_buf, p, &col, &n, plen);
			ચાલુ;

		મૂળભૂત:
			/* Must be a normal character then */
			જો (n >= CONFIG_SYS_CBSIZE - 2) { /* Buffer full */
				putc('\a');
				તોડો;
			}
			જો (c == '\t') {	/* expand TABs */
				જો (IS_ENABLED(CONFIG_AUTO_COMPLETE)) {
					/*
					 * if auto-completion triggered just
					 * continue
					 */
					*p = '\0';
					જો (cmd_auto_complete(prompt,
							      console_buffer,
							      &n, &col)) {
						p = p_buf + n;	/* reset */
						ચાલુ;
					}
				}
				puts(tab_seq + (col & 07));
				col += 8 - (col & 07);
			} નહીંતર {
				અક્ષર __maybe_unused buf[2];

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
			તોડો;
		}
	}
}

પૂર્ણાંક cli_readline_into_buffer(અચલ અક્ષર *અચલ prompt, અક્ષર *buffer,
			     પૂર્ણાંક timeout)
{
	અક્ષર *p = buffer;
	uint len = CONFIG_SYS_CBSIZE;
	પૂર્ણાંક rc;
	સ્થિર પૂર્ણાંક initted;

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
	જો (IS_ENABLED(CONFIG_CMDLINE_EDITING) && (gd->flags & GD_FLG_RELOC)) {
		જો (!initted) {
			rc = hist_init();
			જો (rc == 0)
				initted = 1;
		}

		જો (prompt)
			puts(prompt);

		rc = cread_લીટી(prompt, p, &len, timeout);
		પાછા rc < 0 ? rc : len;

	} નહીંતર {
		પાછા cread_લીટી_simple(prompt, p);
	}
}
