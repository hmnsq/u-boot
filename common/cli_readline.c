// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Add to readline cmdline-editing by
 * (C) Copyright 2005
 * JinHua Luo, GuangDong Linux Center, <luo.jinhua@gd-linux.com>
 */

#ଅନ୍ତର୍ଭାଵ <bootretry.h>
#ଅନ୍ତର୍ଭାଵ <cli.h>
#ଅନ୍ତର୍ଭାଵ <command.h>
#ଅନ୍ତର୍ଭାଵ <hang.h>
#ଅନ୍ତର୍ଭାଵ <malloc.h>
#ଅନ୍ତର୍ଭାଵ <time.h>
#ଅନ୍ତର୍ଭାଵ <watchdog.h>
#ଅନ୍ତର୍ଭାଵ <linux/errno.h>
#ଅନ୍ତର୍ଭାଵ <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର erase_seq[] = "\b \b";	/* erase sequence */
ସ୍ଥିର ଅଚଳ ଅକ୍ଷର   tab_seq[] = "        ";	/* used to expand TABs */

ଅକ୍ଷର console_buffer[CONFIG_SYS_CBSIZE + 1];	/* console I/O buffer	*/

ସ୍ଥିର ଅକ୍ଷର *delete_char (ଅକ୍ଷର *buffer, ଅକ୍ଷର *p, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *colp, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *np, ପୂର୍ଣ୍ଣସଂଖ୍ୟା plen)
{
	ଅକ୍ଷର *s;

	ଯଦି (*np == 0)
		ଫେରନ୍ତୁ p;

	ଯଦି (*(--p) == '\t') {		/* will retype the whole line */
		ଯେତେବେଳେ (*colp > plen) {
			puts(erase_seq);
			(*colp)--;
		}
		ପାଇଁ (s = buffer; s < p; ++s) {
			ଯଦି (*s == '\t') {
				puts(tab_seq + ((*colp) & 07));
				*colp += 8 - ((*colp) & 07);
			} ଅନ୍ୟଥା {
				++(*colp);
				putc(*s);
			}
		}
	} ଅନ୍ୟଥା {
		puts(erase_seq);
		(*colp)--;
	}
	(*np)--;

	ଫେରନ୍ତୁ p;
}

#ifdef CONFIG_CMDLINE_EDITING

/*
 * cmdline-editing related codes from vivi.
 * Author: Janghoon Lyu <nandy@mizi.com>
 */

#define putnstr(str, n)	printf("%.*s", (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)n, str)

#define CTL_BACKSPACE		('\b')
#define DEL			((ଅକ୍ଷର)255)
#define DEL7			((ଅକ୍ଷର)127)
#define CREAD_HIST_CHAR		('!')

#define getcmd_putch(ch)	putc(ch)
#define getcmd_getch()		getchar()
#define getcmd_cbeep()		getcmd_putch('\a')

#ifdef CONFIG_XPL_BUILD
#define HIST_MAX		3
#define HIST_SIZE		32
#ଅନ୍ୟଥା
#define HIST_MAX		20
#define HIST_SIZE		CONFIG_SYS_CBSIZE
#endif

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା hist_max;
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା hist_add_idx;
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା hist_cur = -1;
ସ୍ଥିର unsigned hist_ଅଙ୍କ;

#ifndef CONFIG_CMD_HISTORY_USE_CALLOC
ସ୍ଥିର ଅକ୍ଷର hist_data[HIST_MAX][HIST_SIZE + 1];
#endif
ସ୍ଥିର ଅକ୍ଷର *hist_list[HIST_MAX];

#define add_idx_minus_one() ((hist_add_idx == 0) ? hist_max : hist_add_idx-1)

ସ୍ଥିର ଶୂନ୍ୟ getcmd_putchars(ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ch)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

	ପାଇଁ (i = 0; i < ଗଣନା_2; i++)
		getcmd_putch(ch);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା hist_init(ଶୂନ୍ୟ)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

#ifndef CONFIG_CMD_HISTORY_USE_CALLOC
	ପାଇଁ (i = 0; i < HIST_MAX; i++) {
		hist_list[i] = hist_data[i];
		hist_list[i][0] = '\0';
	}
#ଅନ୍ୟଥା
	unsigned ଅକ୍ଷର *hist = calloc(HIST_MAX, HIST_SIZE + 1);
	ଯଦି (!hist)
		panic("%s: calloc: out of memory!\n", __func__);

	ପାଇଁ (i = 0; i < HIST_MAX; i++)
		hist_list[i] = hist + (i * (HIST_SIZE + 1));
#endif

	hist_max = 0;
	hist_add_idx = 0;
	hist_cur = -1;
	hist_ଅଙ୍କ = 0;

	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ଶୂନ୍ୟ cread_add_to_hist(ଅକ୍ଷର *ଧାଡ଼ି)
{
	strcpy(hist_list[hist_add_idx], ଧାଡ଼ି);

	ଯଦି (++hist_add_idx >= HIST_MAX)
		hist_add_idx = 0;

	ଯଦି (hist_add_idx > hist_max)
		hist_max = hist_add_idx;

	hist_ଅଙ୍କ++;
}

ସ୍ଥିର ଅକ୍ଷର *hist_prev(ଶୂନ୍ୟ)
{
	ଅକ୍ଷର *ret;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା old_cur;

	ଯଦି (hist_cur < 0)
		ଫେରନ୍ତୁ ଖାଲି;

	old_cur = hist_cur;
	ଯଦି (--hist_cur < 0)
		hist_cur = hist_max;

	ଯଦି (hist_cur == hist_add_idx) {
		hist_cur = old_cur;
		ret = ଖାଲି;
	} ଅନ୍ୟଥା {
		ret = hist_list[hist_cur];
	}

	ଫେରନ୍ତୁ ret;
}

ସ୍ଥିର ଅକ୍ଷର *hist_next(ଶୂନ୍ୟ)
{
	ଅକ୍ଷର *ret;

	ଯଦି (hist_cur < 0)
		ଫେରନ୍ତୁ ଖାଲି;

	ଯଦି (hist_cur == hist_add_idx)
		ଫେରନ୍ତୁ ଖାଲି;

	ଯଦି (++hist_cur > hist_max)
		hist_cur = 0;

	ଯଦି (hist_cur == hist_add_idx)
		ret = "";
	ଅନ୍ୟଥା
		ret = hist_list[hist_cur];

	ଫେରନ୍ତୁ ret;
}

ଶୂନ୍ୟ cread_print_hist_list(ଶୂନ୍ୟ)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	uint n;

	n = hist_ଅଙ୍କ - hist_max;

	i = hist_add_idx + 1;
	ଯେତେବେଳେ (1) {
		ଯଦି (i > hist_max)
			i = 0;
		ଯଦି (i == hist_add_idx)
			ଭାଙ୍ଗ;
		printf("%s\n", hist_list[i]);
		n++;
		i++;
	}
}

#define BEGINNING_OF_ଧାଡ଼ି() {			\
	ଯେତେବେଳେ (cls->ଅଙ୍କ) {			\
		getcmd_putch(CTL_BACKSPACE);	\
		cls->ଅଙ୍କ--;			\
	}					\
}

#define ERASE_TO_EOL() {				\
	ଯଦି (cls->ଅଙ୍କ < cls->eol_ଅଙ୍କ) {		\
		printf("%*s", (ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(cls->eol_ଅଙ୍କ - cls->ଅଙ୍କ), ""); \
		କର {					\
			getcmd_putch(CTL_BACKSPACE);	\
		} ଯେତେବେଳେ (--cls->eol_ଅଙ୍କ > cls->ଅଙ୍କ);	\
	}						\
}

#define REFRESH_TO_EOL() {				\
	ଯଦି (cls->ଅଙ୍କ < cls->eol_ଅଙ୍କ) {			\
		uint wlen = cls->eol_ଅଙ୍କ - cls->ଅଙ୍କ;	\
		putnstr(buf + cls->ଅଙ୍କ, wlen);		\
		cls->ଅଙ୍କ = cls->eol_ଅଙ୍କ;		\
	}						\
}

ସ୍ଥିର ଶୂନ୍ୟ cread_add_char(ଅକ୍ଷର ichar, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଭର୍ତ୍ତି, uint *ଅଙ୍କ,
			   uint *eol_ଅଙ୍କ, ଅକ୍ଷର *buf, uint len)
{
	uint wlen;

	/* room ??? */
	ଯଦି (ଭର୍ତ୍ତି || *ଅଙ୍କ == *eol_ଅଙ୍କ) {
		ଯଦି (*eol_ଅଙ୍କ > len - 1) {
			getcmd_cbeep();
			ଫେରନ୍ତୁ;
		}
		(*eol_ଅଙ୍କ)++;
	}

	ଯଦି (ଭର୍ତ୍ତି) {
		wlen = *eol_ଅଙ୍କ - *ଅଙ୍କ;
		ଯଦି (wlen > 1)
			memmove(&buf[*ଅଙ୍କ+1], &buf[*ଅଙ୍କ], wlen-1);

		buf[*ଅଙ୍କ] = ichar;
		putnstr(buf + *ଅଙ୍କ, wlen);
		(*ଅଙ୍କ)++;
		ଯେତେବେଳେ (--wlen)
			getcmd_putch(CTL_BACKSPACE);
	} ଅନ୍ୟଥା {
		/* echo the character */
		wlen = 1;
		buf[*ଅଙ୍କ] = ichar;
		putnstr(buf + *ଅଙ୍କ, wlen);
		(*ଅଙ୍କ)++;
	}
}

ସ୍ଥିର ଶୂନ୍ୟ cread_add_str(ଅକ୍ଷର *str, ପୂର୍ଣ୍ଣସଂଖ୍ୟା strsize, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଭର୍ତ୍ତି,
			  uint *ଅଙ୍କ, uint *eol_ଅଙ୍କ, ଅକ୍ଷର *buf, uint len)
{
	ଯେତେବେଳେ (strsize--) {
		cread_add_char(*str, ଭର୍ତ୍ତି, ଅଙ୍କ, eol_ଅଙ୍କ, buf, len);
		str++;
	}
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା cread_ଧାଡ଼ି_process_ch(ସଂରଚନା cli_ଧାଡ଼ି_state *cls, ଅକ୍ଷର ichar)
{
	ଅକ୍ଷର *buf = cls->buf;

	/* ichar=0x0 when error occurs in U-Boot getc */
	ଯଦି (!ichar)
		ଫେରନ୍ତୁ -EAGAIN;

	ଯଦି (ichar == '\n') {
		putc('\n');
		buf[cls->eol_ଅଙ୍କ] = '\0';	/* terminate the string */
		ଫେରନ୍ତୁ 0;
	}

	ବାଛ (ichar) {
	ସ୍ଥିତି CTL_CH('a'):
		BEGINNING_OF_ଧାଡ଼ି();
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CTL_CH('c'):	/* ^C - break */
		*buf = '\0';	/* discard input */
		ଫେରନ୍ତୁ -EINTR;
	ସ୍ଥିତି CTL_CH('f'):
		ଯଦି (cls->ଅଙ୍କ < cls->eol_ଅଙ୍କ) {
			getcmd_putch(buf[cls->ଅଙ୍କ]);
			cls->ଅଙ୍କ++;
		}
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CTL_CH('b'):
		ଯଦି (cls->ଅଙ୍କ) {
			getcmd_putch(CTL_BACKSPACE);
			cls->ଅଙ୍କ--;
		}
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CTL_CH('d'):
		ଯଦି (cls->ଅଙ୍କ < cls->eol_ଅଙ୍କ) {
			uint wlen;

			wlen = cls->eol_ଅଙ୍କ - cls->ଅଙ୍କ - 1;
			ଯଦି (wlen) {
				memmove(&buf[cls->ଅଙ୍କ], &buf[cls->ଅଙ୍କ + 1],
					wlen);
				putnstr(buf + cls->ଅଙ୍କ, wlen);
			}

			getcmd_putch(' ');
			କର {
				getcmd_putch(CTL_BACKSPACE);
			} ଯେତେବେଳେ (wlen--);
			cls->eol_ଅଙ୍କ--;
		}
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CTL_CH('k'):
		ERASE_TO_EOL();
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CTL_CH('e'):
		REFRESH_TO_EOL();
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CTL_CH('o'):
		cls->ଭର୍ତ୍ତି = !cls->ଭର୍ତ୍ତି;
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CTL_CH('w'):
		ଯଦି (cls->ଅଙ୍କ) {
			uint base, wlen;

			ପାଇଁ (base = cls->ଅଙ୍କ;
			     base > 0 && buf[base - 1] == ' ';)
				base--;
			ପାଇଁ (; base > 0 && buf[base - 1] != ' ';)
				base--;

			/* now delete chars from base to cls->num */
			wlen = cls->ଅଙ୍କ - base;
			cls->eol_ଅଙ୍କ -= wlen;
			memmove(&buf[base], &buf[cls->ଅଙ୍କ],
				cls->eol_ଅଙ୍କ - base + 1);
			cls->ଅଙ୍କ = base;
			getcmd_putchars(wlen, CTL_BACKSPACE);
			puts(buf + base);
			getcmd_putchars(wlen, ' ');
			getcmd_putchars(wlen + cls->eol_ଅଙ୍କ - cls->ଅଙ୍କ,
					CTL_BACKSPACE);
		}
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CTL_CH('x'):
	ସ୍ଥିତି CTL_CH('u'):
		BEGINNING_OF_ଧାଡ଼ି();
		ERASE_TO_EOL();
		ଭାଙ୍ଗ;
	ସ୍ଥିତି DEL:
	ସ୍ଥିତି DEL7:
	ସ୍ଥିତି 8:
		ଯଦି (cls->ଅଙ୍କ) {
			uint wlen;

			wlen = cls->eol_ଅଙ୍କ - cls->ଅଙ୍କ;
			cls->ଅଙ୍କ--;
			memmove(&buf[cls->ଅଙ୍କ], &buf[cls->ଅଙ୍କ + 1], wlen);
			getcmd_putch(CTL_BACKSPACE);
			putnstr(buf + cls->ଅଙ୍କ, wlen);
			getcmd_putch(' ');
			କର {
				getcmd_putch(CTL_BACKSPACE);
			} ଯେତେବେଳେ (wlen--);
			cls->eol_ଅଙ୍କ--;
		}
		ଭାଙ୍ଗ;
	ସ୍ଥିତି CTL_CH('p'):
	ସ୍ଥିତି CTL_CH('n'):
		ଯଦି (cls->history) {
			ଅକ୍ଷର *hline;

			ଯଦି (ichar == CTL_CH('p'))
				hline = hist_prev();
			ଅନ୍ୟଥା
				hline = hist_next();

			ଯଦି (!hline) {
				getcmd_cbeep();
				ଭାଙ୍ଗ;
			}

			/* nuke the current line */
			/* first, go home */
			BEGINNING_OF_ଧାଡ଼ି();

			/* erase to end of line */
			ERASE_TO_EOL();

			/* copy new line into place and display */
			strcpy(buf, hline);
			cls->eol_ଅଙ୍କ = strlen(buf);
			REFRESH_TO_EOL();
			ଭାଙ୍ଗ;
		}
		ଭାଙ୍ଗ;
	ସ୍ଥିତି '\t':
		ଯଦି (IS_ENABLED(CONFIG_AUTO_COMPLETE) && cls->cmd_complete) {
			ପୂର୍ଣ୍ଣସଂଖ୍ୟା num2, col;

			/* do not autocomplete when in the middle */
			ଯଦି (cls->ଅଙ୍କ < cls->eol_ଅଙ୍କ) {
				getcmd_cbeep();
				ଭାଙ୍ଗ;
			}

			buf[cls->ଅଙ୍କ] = '\0';
			col = strlen(cls->prompt) + cls->eol_ଅଙ୍କ;
			num2 = cls->ଅଙ୍କ;
			ଯଦି (cmd_auto_complete(cls->prompt, buf, &num2, &col)) {
				col = num2 - cls->ଅଙ୍କ;
				cls->ଅଙ୍କ += col;
				cls->eol_ଅଙ୍କ += col;
			}
			ଭାଙ୍ଗ;
		}
		fallthrough;
	ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
		cread_add_char(ichar, cls->ଭର୍ତ୍ତି, &cls->ଅଙ୍କ, &cls->eol_ଅଙ୍କ,
			       buf, cls->len);
		ଭାଙ୍ଗ;
	}

	/*
	 * keep the string terminated...if we added a char at the end then we
	 * want a \0 after it
	 */
	buf[cls->eol_ଅଙ୍କ] = '\0';

	ଫେରନ୍ତୁ -EAGAIN;
}

ଶୂନ୍ୟ cli_cread_init(ସଂରଚନା cli_ଧାଡ଼ି_state *cls, ଅକ୍ଷର *buf, uint buf_size)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା init_len = strlen(buf);

	memset(cls, '\0', ଆକାର(ସଂରଚନା cli_ଧାଡ଼ି_state));
	cls->ଭର୍ତ୍ତି = true;
	cls->buf = buf;
	cls->len = buf_size;

	ଯଦି (init_len)
		cread_add_str(buf, init_len, 0, &cls->ଅଙ୍କ, &cls->eol_ଅଙ୍କ, buf,
			      buf_size);
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା cread_ଧାଡ଼ି(ଅଚଳ ଅକ୍ଷର *ଅଚଳ prompt, ଅକ୍ଷର *buf, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *len,
		      ପୂର୍ଣ୍ଣସଂଖ୍ୟା timeout)
{
	ସଂରଚନା cli_ch_state s_cch, *cch = &s_cch;
	ସଂରଚନା cli_ଧାଡ଼ି_state s_cls, *cls = &s_cls;
	ଅକ୍ଷର ichar;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା first = 1;

	cli_ch_init(cch);
	cli_cread_init(cls, buf, *len);
	cls->prompt = prompt;
	cls->history = true;
	cls->cmd_complete = true;

	ଯେତେବେଳେ (1) {
		ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

		/* Check for saved characters */
		ichar = cli_ch_process(cch, 0);

		ଯଦି (!ichar) {
			ଯଦି (bootretry_tstc_timeout())
				ଫେରନ୍ତୁ -2;	/* timed out */
			ଯଦି (first && timeout) {
				u64 etime = endtick(timeout);

				ଯେତେବେଳେ (!tstc()) {	/* while no incoming data */
					ଯଦି (get_ticks() >= etime)
						ଫେରନ୍ତୁ -2;	/* timed out */
					schedule();
				}
				first = 0;
			}

			ichar = getcmd_getch();
			ichar = cli_ch_process(cch, ichar);
		}

		ret = cread_ଧାଡ଼ି_process_ch(cls, ichar);
		ଯଦି (ret == -EINTR)
			ଫେରନ୍ତୁ -1;
		ଅନ୍ୟଥା ଯଦି (!ret)
			ଭାଙ୍ଗ;
	}
	*len = cls->eol_ଅଙ୍କ;

	ଯଦି (buf[0] && buf[0] != CREAD_HIST_CHAR)
		cread_add_to_hist(buf);
	hist_cur = hist_add_idx;

	ଫେରନ୍ତୁ 0;
}

#ଅନ୍ୟଥା /* !CONFIG_CMDLINE_EDITING */

ସ୍ଥିର ଧାଡ଼ିଭିତରେ ପୂର୍ଣ୍ଣସଂଖ୍ୟା hist_init(ଶୂନ୍ୟ)
{
	ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା cread_ଧାଡ଼ି(ଅଚଳ ଅକ୍ଷର *ଅଚଳ prompt, ଅକ୍ଷର *buf, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା *len,
		      ପୂର୍ଣ୍ଣସଂଖ୍ୟା timeout)
{
	ଫେରନ୍ତୁ 0;
}

#endif /* CONFIG_CMDLINE_EDITING */

/****************************************************************************/

ପୂର୍ଣ୍ଣସଂଖ୍ୟା cli_readline(ଅଚଳ ଅକ୍ଷର *ଅଚଳ prompt)
{
	/*
	 * If console_buffer isn't 0-length the user will be prompted to modify
	 * it instead of entering it from scratch as desired.
	 */
	console_buffer[0] = '\0';

	ଫେରନ୍ତୁ cli_readline_into_buffer(prompt, console_buffer, 0);
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
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା cread_ଧାଡ଼ି_simple(ଅଚଳ ଅକ୍ଷର *ଅଚଳ prompt, ଅକ୍ଷର *p)
{
	ଅକ୍ଷର *p_buf = p;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା n = 0;		/* buffer index */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା plen = 0;		/* prompt length */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା col;		/* output column cnt */
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା c;

	/* print prompt */
	ଯଦି (prompt) {
		plen = strlen(prompt);
		puts(prompt);
	}
	col = plen;

	ପାଇଁ (;;) {
		ଯଦି (bootretry_tstc_timeout())
			ଫେରନ୍ତୁ -2;	/* timed out */
		schedule();	/* Trigger watchdog, if needed */

		c = getchar();

		/*
		 * Special character handling
		 */
		ବାଛ (c) {
		ସ୍ଥିତି '\r':			/* Enter		*/
		ସ୍ଥିତି '\n':
			*p = '\0';
			puts("\r\n");
			ଫେରନ୍ତୁ p - p_buf;

		ସ୍ଥିତି '\0':			/* nul			*/
			ଜାରି;

		ସ୍ଥିତି 0x03:			/* ^C - break		*/
			p_buf[0] = '\0';	/* discard input */
			ଫେରନ୍ତୁ -1;

		ସ୍ଥିତି 0x15:			/* ^U - erase line	*/
			ଯେତେବେଳେ (col > plen) {
				puts(erase_seq);
				--col;
			}
			p = p_buf;
			n = 0;
			ଜାରି;

		ସ୍ଥିତି 0x17:			/* ^W - erase word	*/
			p = delete_char(p_buf, p, &col, &n, plen);
			ଯେତେବେଳେ ((n > 0) && (*p != ' '))
				p = delete_char(p_buf, p, &col, &n, plen);
			ଜାରି;

		ସ୍ଥିତି 0x08:			/* ^H  - backspace	*/
		ସ୍ଥିତି 0x7F:			/* DEL - backspace	*/
			p = delete_char(p_buf, p, &col, &n, plen);
			ଜାରି;

		ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
			/* Must be a normal character then */
			ଯଦି (n >= CONFIG_SYS_CBSIZE - 2) { /* Buffer full */
				putc('\a');
				ଭାଙ୍ଗ;
			}
			ଯଦି (c == '\t') {	/* expand TABs */
				ଯଦି (IS_ENABLED(CONFIG_AUTO_COMPLETE)) {
					/*
					 * if auto-completion triggered just
					 * continue
					 */
					*p = '\0';
					ଯଦି (cmd_auto_complete(prompt,
							      console_buffer,
							      &n, &col)) {
						p = p_buf + n;	/* reset */
						ଜାରି;
					}
				}
				puts(tab_seq + (col & 07));
				col += 8 - (col & 07);
			} ଅନ୍ୟଥା {
				ଅକ୍ଷର __maybe_unused buf[2];

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
			ଭାଙ୍ଗ;
		}
	}
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା cli_readline_into_buffer(ଅଚଳ ଅକ୍ଷର *ଅଚଳ prompt, ଅକ୍ଷର *buffer,
			     ପୂର୍ଣ୍ଣସଂଖ୍ୟା timeout)
{
	ଅକ୍ଷର *p = buffer;
	uint len = CONFIG_SYS_CBSIZE;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା rc;
	ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା initted;

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
	ଯଦି (IS_ENABLED(CONFIG_CMDLINE_EDITING) && (gd->flags & GD_FLG_RELOC)) {
		ଯଦି (!initted) {
			rc = hist_init();
			ଯଦି (rc == 0)
				initted = 1;
		}

		ଯଦି (prompt)
			puts(prompt);

		rc = cread_ଧାଡ଼ି(prompt, p, &len, timeout);
		ଫେରନ୍ତୁ rc < 0 ? rc : len;

	} ଅନ୍ୟଥା {
		ଫେରନ୍ତୁ cread_ଧାଡ଼ି_simple(prompt, p);
	}
}
