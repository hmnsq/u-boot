// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Add to readline cmdline-editing by
 * (C) Copyright 2005
 * JinHua Luo, GuangDong Linux Center, <luo.jinhua@gd-linux.com>
 */

#அந்தர்பாவ <bootretry.h>
#அந்தர்பாவ <cli.h>
#அந்தர்பாவ <command.h>
#அந்தர்பாவ <hang.h>
#அந்தர்பாவ <malloc.h>
#அந்தர்பாவ <time.h>
#அந்தர்பாவ <watchdog.h>
#அந்தர்பாவ <linux/errno.h>
#அந்தர்பாவ <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

நிலையான மாறிலி எழுத்து erase_seq[] = "\b \b";	/* erase sequence */
நிலையான மாறிலி எழுத்து   tab_seq[] = "        ";	/* used to expand TABs */

எழுத்து console_buffer[CONFIG_SYS_CBSIZE + 1];	/* console I/O buffer	*/

நிலையான எழுத்து *delete_char (எழுத்து *buffer, எழுத்து *p, முழு *colp, முழு *np, முழு plen)
{
	எழுத்து *s;

	எனில் (*np == 0)
		திரும்பு p;

	எனில் (*(--p) == '\t') {		/* will retype the whole line */
		போது (*colp > plen) {
			puts(erase_seq);
			(*colp)--;
		}
		க்கு (s = buffer; s < p; ++s) {
			எனில் (*s == '\t') {
				puts(tab_seq + ((*colp) & 07));
				*colp += 8 - ((*colp) & 07);
			} இல்லையெனில் {
				++(*colp);
				putc(*s);
			}
		}
	} இல்லையெனில் {
		puts(erase_seq);
		(*colp)--;
	}
	(*np)--;

	திரும்பு p;
}

#ifdef CONFIG_CMDLINE_EDITING

/*
 * cmdline-editing related codes from vivi.
 * Author: Janghoon Lyu <nandy@mizi.com>
 */

#define putnstr(str, n)	printf("%.*s", (முழு)n, str)

#define CTL_BACKSPACE		('\b')
#define DEL			((எழுத்து)255)
#define DEL7			((எழுத்து)127)
#define CREAD_HIST_CHAR		('!')

#define getcmd_putch(ch)	putc(ch)
#define getcmd_getch()		getchar()
#define getcmd_cbeep()		getcmd_putch('\a')

#ifdef CONFIG_XPL_BUILD
#define HIST_MAX		3
#define HIST_SIZE		32
#இல்லையெனில்
#define HIST_MAX		20
#define HIST_SIZE		CONFIG_SYS_CBSIZE
#endif

நிலையான முழு hist_max;
நிலையான முழு hist_add_idx;
நிலையான முழு hist_cur = -1;
நிலையான unsigned hist_இலக்கம்;

#ifndef CONFIG_CMD_HISTORY_USE_CALLOC
நிலையான எழுத்து hist_data[HIST_MAX][HIST_SIZE + 1];
#endif
நிலையான எழுத்து *hist_list[HIST_MAX];

#define add_idx_minus_one() ((hist_add_idx == 0) ? hist_max : hist_add_idx-1)

நிலையான வெறுமை getcmd_putchars(முழு மொத்தம், முழு ch)
{
	முழு i;

	க்கு (i = 0; i < மொத்தம்; i++)
		getcmd_putch(ch);
}

நிலையான முழு hist_init(வெறுமை)
{
	முழு i;

#ifndef CONFIG_CMD_HISTORY_USE_CALLOC
	க்கு (i = 0; i < HIST_MAX; i++) {
		hist_list[i] = hist_data[i];
		hist_list[i][0] = '\0';
	}
#இல்லையெனில்
	unsigned எழுத்து *hist = calloc(HIST_MAX, HIST_SIZE + 1);
	எனில் (!hist)
		panic("%s: calloc: out of memory!\n", __func__);

	க்கு (i = 0; i < HIST_MAX; i++)
		hist_list[i] = hist + (i * (HIST_SIZE + 1));
#endif

	hist_max = 0;
	hist_add_idx = 0;
	hist_cur = -1;
	hist_இலக்கம் = 0;

	திரும்பு 0;
}

நிலையான வெறுமை cread_add_to_hist(எழுத்து *வரி)
{
	strcpy(hist_list[hist_add_idx], வரி);

	எனில் (++hist_add_idx >= HIST_MAX)
		hist_add_idx = 0;

	எனில் (hist_add_idx > hist_max)
		hist_max = hist_add_idx;

	hist_இலக்கம்++;
}

நிலையான எழுத்து *hist_prev(வெறுமை)
{
	எழுத்து *ret;
	முழு old_cur;

	எனில் (hist_cur < 0)
		திரும்பு வெற்று;

	old_cur = hist_cur;
	எனில் (--hist_cur < 0)
		hist_cur = hist_max;

	எனில் (hist_cur == hist_add_idx) {
		hist_cur = old_cur;
		ret = வெற்று;
	} இல்லையெனில் {
		ret = hist_list[hist_cur];
	}

	திரும்பு ret;
}

நிலையான எழுத்து *hist_next(வெறுமை)
{
	எழுத்து *ret;

	எனில் (hist_cur < 0)
		திரும்பு வெற்று;

	எனில் (hist_cur == hist_add_idx)
		திரும்பு வெற்று;

	எனில் (++hist_cur > hist_max)
		hist_cur = 0;

	எனில் (hist_cur == hist_add_idx)
		ret = "";
	இல்லையெனில்
		ret = hist_list[hist_cur];

	திரும்பு ret;
}

வெறுமை cread_print_hist_list(வெறுமை)
{
	முழு i;
	uint n;

	n = hist_இலக்கம் - hist_max;

	i = hist_add_idx + 1;
	போது (1) {
		எனில் (i > hist_max)
			i = 0;
		எனில் (i == hist_add_idx)
			நிறுத்து;
		printf("%s\n", hist_list[i]);
		n++;
		i++;
	}
}

#define BEGINNING_OF_வரி() {			\
	போது (cls->இலக்கம்) {			\
		getcmd_putch(CTL_BACKSPACE);	\
		cls->இலக்கம்--;			\
	}					\
}

#define ERASE_TO_EOL() {				\
	எனில் (cls->இலக்கம் < cls->eol_இலக்கம்) {		\
		printf("%*s", (முழு)(cls->eol_இலக்கம் - cls->இலக்கம்), ""); \
		செய் {					\
			getcmd_putch(CTL_BACKSPACE);	\
		} போது (--cls->eol_இலக்கம் > cls->இலக்கம்);	\
	}						\
}

#define REFRESH_TO_EOL() {				\
	எனில் (cls->இலக்கம் < cls->eol_இலக்கம்) {			\
		uint wlen = cls->eol_இலக்கம் - cls->இலக்கம்;	\
		putnstr(buf + cls->இலக்கம், wlen);		\
		cls->இலக்கம் = cls->eol_இலக்கம்;		\
	}						\
}

நிலையான வெறுமை cread_add_char(எழுத்து ichar, முழு செருகு, uint *இலக்கம்,
			   uint *eol_இலக்கம், எழுத்து *buf, uint len)
{
	uint wlen;

	/* room ??? */
	எனில் (செருகு || *இலக்கம் == *eol_இலக்கம்) {
		எனில் (*eol_இலக்கம் > len - 1) {
			getcmd_cbeep();
			திரும்பு;
		}
		(*eol_இலக்கம்)++;
	}

	எனில் (செருகு) {
		wlen = *eol_இலக்கம் - *இலக்கம்;
		எனில் (wlen > 1)
			memmove(&buf[*இலக்கம்+1], &buf[*இலக்கம்], wlen-1);

		buf[*இலக்கம்] = ichar;
		putnstr(buf + *இலக்கம், wlen);
		(*இலக்கம்)++;
		போது (--wlen)
			getcmd_putch(CTL_BACKSPACE);
	} இல்லையெனில் {
		/* echo the character */
		wlen = 1;
		buf[*இலக்கம்] = ichar;
		putnstr(buf + *இலக்கம், wlen);
		(*இலக்கம்)++;
	}
}

நிலையான வெறுமை cread_add_str(எழுத்து *str, முழு strsize, முழு செருகு,
			  uint *இலக்கம், uint *eol_இலக்கம், எழுத்து *buf, uint len)
{
	போது (strsize--) {
		cread_add_char(*str, செருகு, இலக்கம், eol_இலக்கம், buf, len);
		str++;
	}
}

முழு cread_வரி_process_ch(கட்டமைப்பு cli_வரி_state *cls, எழுத்து ichar)
{
	எழுத்து *buf = cls->buf;

	/* ichar=0x0 when error occurs in U-Boot getc */
	எனில் (!ichar)
		திரும்பு -EAGAIN;

	எனில் (ichar == '\n') {
		putc('\n');
		buf[cls->eol_இலக்கம்] = '\0';	/* terminate the string */
		திரும்பு 0;
	}

	தேர்வு (ichar) {
	நிலை CTL_CH('a'):
		BEGINNING_OF_வரி();
		நிறுத்து;
	நிலை CTL_CH('c'):	/* ^C - break */
		*buf = '\0';	/* discard input */
		திரும்பு -EINTR;
	நிலை CTL_CH('f'):
		எனில் (cls->இலக்கம் < cls->eol_இலக்கம்) {
			getcmd_putch(buf[cls->இலக்கம்]);
			cls->இலக்கம்++;
		}
		நிறுத்து;
	நிலை CTL_CH('b'):
		எனில் (cls->இலக்கம்) {
			getcmd_putch(CTL_BACKSPACE);
			cls->இலக்கம்--;
		}
		நிறுத்து;
	நிலை CTL_CH('d'):
		எனில் (cls->இலக்கம் < cls->eol_இலக்கம்) {
			uint wlen;

			wlen = cls->eol_இலக்கம் - cls->இலக்கம் - 1;
			எனில் (wlen) {
				memmove(&buf[cls->இலக்கம்], &buf[cls->இலக்கம் + 1],
					wlen);
				putnstr(buf + cls->இலக்கம், wlen);
			}

			getcmd_putch(' ');
			செய் {
				getcmd_putch(CTL_BACKSPACE);
			} போது (wlen--);
			cls->eol_இலக்கம்--;
		}
		நிறுத்து;
	நிலை CTL_CH('k'):
		ERASE_TO_EOL();
		நிறுத்து;
	நிலை CTL_CH('e'):
		REFRESH_TO_EOL();
		நிறுத்து;
	நிலை CTL_CH('o'):
		cls->செருகு = !cls->செருகு;
		நிறுத்து;
	நிலை CTL_CH('w'):
		எனில் (cls->இலக்கம்) {
			uint base, wlen;

			க்கு (base = cls->இலக்கம்;
			     base > 0 && buf[base - 1] == ' ';)
				base--;
			க்கு (; base > 0 && buf[base - 1] != ' ';)
				base--;

			/* now delete chars from base to cls->num */
			wlen = cls->இலக்கம் - base;
			cls->eol_இலக்கம் -= wlen;
			memmove(&buf[base], &buf[cls->இலக்கம்],
				cls->eol_இலக்கம் - base + 1);
			cls->இலக்கம் = base;
			getcmd_putchars(wlen, CTL_BACKSPACE);
			puts(buf + base);
			getcmd_putchars(wlen, ' ');
			getcmd_putchars(wlen + cls->eol_இலக்கம் - cls->இலக்கம்,
					CTL_BACKSPACE);
		}
		நிறுத்து;
	நிலை CTL_CH('x'):
	நிலை CTL_CH('u'):
		BEGINNING_OF_வரி();
		ERASE_TO_EOL();
		நிறுத்து;
	நிலை DEL:
	நிலை DEL7:
	நிலை 8:
		எனில் (cls->இலக்கம்) {
			uint wlen;

			wlen = cls->eol_இலக்கம் - cls->இலக்கம்;
			cls->இலக்கம்--;
			memmove(&buf[cls->இலக்கம்], &buf[cls->இலக்கம் + 1], wlen);
			getcmd_putch(CTL_BACKSPACE);
			putnstr(buf + cls->இலக்கம், wlen);
			getcmd_putch(' ');
			செய் {
				getcmd_putch(CTL_BACKSPACE);
			} போது (wlen--);
			cls->eol_இலக்கம்--;
		}
		நிறுத்து;
	நிலை CTL_CH('p'):
	நிலை CTL_CH('n'):
		எனில் (cls->history) {
			எழுத்து *hline;

			எனில் (ichar == CTL_CH('p'))
				hline = hist_prev();
			இல்லையெனில்
				hline = hist_next();

			எனில் (!hline) {
				getcmd_cbeep();
				நிறுத்து;
			}

			/* nuke the current line */
			/* first, go home */
			BEGINNING_OF_வரி();

			/* erase to end of line */
			ERASE_TO_EOL();

			/* copy new line into place and display */
			strcpy(buf, hline);
			cls->eol_இலக்கம் = strlen(buf);
			REFRESH_TO_EOL();
			நிறுத்து;
		}
		நிறுத்து;
	நிலை '\t':
		எனில் (IS_ENABLED(CONFIG_AUTO_COMPLETE) && cls->cmd_complete) {
			முழு num2, col;

			/* do not autocomplete when in the middle */
			எனில் (cls->இலக்கம் < cls->eol_இலக்கம்) {
				getcmd_cbeep();
				நிறுத்து;
			}

			buf[cls->இலக்கம்] = '\0';
			col = strlen(cls->prompt) + cls->eol_இலக்கம்;
			num2 = cls->இலக்கம்;
			எனில் (cmd_auto_complete(cls->prompt, buf, &num2, &col)) {
				col = num2 - cls->இலக்கம்;
				cls->இலக்கம் += col;
				cls->eol_இலக்கம் += col;
			}
			நிறுத்து;
		}
		fallthrough;
	இயல்பு:
		cread_add_char(ichar, cls->செருகு, &cls->இலக்கம், &cls->eol_இலக்கம்,
			       buf, cls->len);
		நிறுத்து;
	}

	/*
	 * keep the string terminated...if we added a char at the end then we
	 * want a \0 after it
	 */
	buf[cls->eol_இலக்கம்] = '\0';

	திரும்பு -EAGAIN;
}

வெறுமை cli_cread_init(கட்டமைப்பு cli_வரி_state *cls, எழுத்து *buf, uint buf_size)
{
	முழு init_len = strlen(buf);

	memset(cls, '\0', அளவு(கட்டமைப்பு cli_வரி_state));
	cls->செருகு = true;
	cls->buf = buf;
	cls->len = buf_size;

	எனில் (init_len)
		cread_add_str(buf, init_len, 0, &cls->இலக்கம், &cls->eol_இலக்கம், buf,
			      buf_size);
}

நிலையான முழு cread_வரி(மாறிலி எழுத்து *மாறிலி prompt, எழுத்து *buf, unsigned முழு *len,
		      முழு timeout)
{
	கட்டமைப்பு cli_ch_state s_cch, *cch = &s_cch;
	கட்டமைப்பு cli_வரி_state s_cls, *cls = &s_cls;
	எழுத்து ichar;
	முழு first = 1;

	cli_ch_init(cch);
	cli_cread_init(cls, buf, *len);
	cls->prompt = prompt;
	cls->history = true;
	cls->cmd_complete = true;

	போது (1) {
		முழு ret;

		/* Check for saved characters */
		ichar = cli_ch_process(cch, 0);

		எனில் (!ichar) {
			எனில் (bootretry_tstc_timeout())
				திரும்பு -2;	/* timed out */
			எனில் (first && timeout) {
				u64 etime = endtick(timeout);

				போது (!tstc()) {	/* while no incoming data */
					எனில் (get_ticks() >= etime)
						திரும்பு -2;	/* timed out */
					schedule();
				}
				first = 0;
			}

			ichar = getcmd_getch();
			ichar = cli_ch_process(cch, ichar);
		}

		ret = cread_வரி_process_ch(cls, ichar);
		எனில் (ret == -EINTR)
			திரும்பு -1;
		இல்லையெனில் எனில் (!ret)
			நிறுத்து;
	}
	*len = cls->eol_இலக்கம்;

	எனில் (buf[0] && buf[0] != CREAD_HIST_CHAR)
		cread_add_to_hist(buf);
	hist_cur = hist_add_idx;

	திரும்பு 0;
}

#இல்லையெனில் /* !CONFIG_CMDLINE_EDITING */

நிலையான உள்வரி முழு hist_init(வெறுமை)
{
	திரும்பு 0;
}

நிலையான முழு cread_வரி(மாறிலி எழுத்து *மாறிலி prompt, எழுத்து *buf, unsigned முழு *len,
		      முழு timeout)
{
	திரும்பு 0;
}

#endif /* CONFIG_CMDLINE_EDITING */

/****************************************************************************/

முழு cli_readline(மாறிலி எழுத்து *மாறிலி prompt)
{
	/*
	 * If console_buffer isn't 0-length the user will be prompted to modify
	 * it instead of entering it from scratch as desired.
	 */
	console_buffer[0] = '\0';

	திரும்பு cli_readline_into_buffer(prompt, console_buffer, 0);
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
நிலையான முழு cread_வரி_simple(மாறிலி எழுத்து *மாறிலி prompt, எழுத்து *p)
{
	எழுத்து *p_buf = p;
	முழு n = 0;		/* buffer index */
	முழு plen = 0;		/* prompt length */
	முழு col;		/* output column cnt */
	முழு c;

	/* print prompt */
	எனில் (prompt) {
		plen = strlen(prompt);
		puts(prompt);
	}
	col = plen;

	க்கு (;;) {
		எனில் (bootretry_tstc_timeout())
			திரும்பு -2;	/* timed out */
		schedule();	/* Trigger watchdog, if needed */

		c = getchar();

		/*
		 * Special character handling
		 */
		தேர்வு (c) {
		நிலை '\r':			/* Enter		*/
		நிலை '\n':
			*p = '\0';
			puts("\r\n");
			திரும்பு p - p_buf;

		நிலை '\0':			/* nul			*/
			தொடர்;

		நிலை 0x03:			/* ^C - break		*/
			p_buf[0] = '\0';	/* discard input */
			திரும்பு -1;

		நிலை 0x15:			/* ^U - erase line	*/
			போது (col > plen) {
				puts(erase_seq);
				--col;
			}
			p = p_buf;
			n = 0;
			தொடர்;

		நிலை 0x17:			/* ^W - erase word	*/
			p = delete_char(p_buf, p, &col, &n, plen);
			போது ((n > 0) && (*p != ' '))
				p = delete_char(p_buf, p, &col, &n, plen);
			தொடர்;

		நிலை 0x08:			/* ^H  - backspace	*/
		நிலை 0x7F:			/* DEL - backspace	*/
			p = delete_char(p_buf, p, &col, &n, plen);
			தொடர்;

		இயல்பு:
			/* Must be a normal character then */
			எனில் (n >= CONFIG_SYS_CBSIZE - 2) { /* Buffer full */
				putc('\a');
				நிறுத்து;
			}
			எனில் (c == '\t') {	/* expand TABs */
				எனில் (IS_ENABLED(CONFIG_AUTO_COMPLETE)) {
					/*
					 * if auto-completion triggered just
					 * continue
					 */
					*p = '\0';
					எனில் (cmd_auto_complete(prompt,
							      console_buffer,
							      &n, &col)) {
						p = p_buf + n;	/* reset */
						தொடர்;
					}
				}
				puts(tab_seq + (col & 07));
				col += 8 - (col & 07);
			} இல்லையெனில் {
				எழுத்து __maybe_unused buf[2];

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
			நிறுத்து;
		}
	}
}

முழு cli_readline_into_buffer(மாறிலி எழுத்து *மாறிலி prompt, எழுத்து *buffer,
			     முழு timeout)
{
	எழுத்து *p = buffer;
	uint len = CONFIG_SYS_CBSIZE;
	முழு rc;
	நிலையான முழு initted;

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
	எனில் (IS_ENABLED(CONFIG_CMDLINE_EDITING) && (gd->flags & GD_FLG_RELOC)) {
		எனில் (!initted) {
			rc = hist_init();
			எனில் (rc == 0)
				initted = 1;
		}

		எனில் (prompt)
			puts(prompt);

		rc = cread_வரி(prompt, p, &len, timeout);
		திரும்பு rc < 0 ? rc : len;

	} இல்லையெனில் {
		திரும்பு cread_வரி_simple(prompt, p);
	}
}
