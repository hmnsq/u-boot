// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Add to readline cmdline-editing by
 * (C) Copyright 2005
 * JinHua Luo, GuangDong Linux Center, <luo.jinhua@gd-linux.com>
 */

#ਅਨ੍ਤਰ੍ਭਾਵ <bootretry.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <cli.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <command.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <hang.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <malloc.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <time.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <watchdog.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/errno.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/global_data.h>

DECLARE_GLOBAL_ਅੰਕੜੇ_PTR;

ਸਥਿਰ ਅਚਲ ਅੱਖਰ erase_seq[] = "\b \b";	/* erase sequence */
ਸਥਿਰ ਅਚਲ ਅੱਖਰ   tab_seq[] = "        ";	/* used to expand TABs */

ਅੱਖਰ console_ਬਫਰ[CONFIG_SYS_CBSIZE + 1];	/* console I/O buffer	*/

ਸਥਿਰ ਅੱਖਰ *delete_char (ਅੱਖਰ *ਬਫਰ, ਅੱਖਰ *p, ਪੂਰਨ *colp, ਪੂਰਨ *np, ਪੂਰਨ plen)
{
	ਅੱਖਰ *s;

	ਜੇ (*np == 0)
		ਵਾਪਸ p;

	ਜੇ (*(--p) == '\t') {		/* will retype the whole line */
		ਜਦੋਂ (*colp > plen) {
			puts(erase_seq);
			(*colp)--;
		}
		ਲਈ (s = ਬਫਰ; s < p; ++s) {
			ਜੇ (*s == '\t') {
				puts(tab_seq + ((*colp) & 07));
				*colp += 8 - ((*colp) & 07);
			} ਨਹੀਂਤਾਂ {
				++(*colp);
				putc(*s);
			}
		}
	} ਨਹੀਂਤਾਂ {
		puts(erase_seq);
		(*colp)--;
	}
	(*np)--;

	ਵਾਪਸ p;
}

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_CMDLINE_EDITING

/*
 * cmdline-editing related codes from vivi.
 * Author: Janghoon Lyu <nandy@mizi.com>
 */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ putnstr(str, n)	ਲਿਖੋ("%.*s", (ਪੂਰਨ)n, str)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CTL_BACKSPACE		('\b')
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEL			((ਅੱਖਰ)255)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEL7			((ਅੱਖਰ)127)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CREAD_HIST_CHAR		('!')

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ getcmd_putch(ch)	putc(ch)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ getcmd_getch()		getchar()
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ getcmd_cbeep()		getcmd_putch('\a')

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_XPL_BUILD
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HIST_MAX		3
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HIST_ਮਾਪ		32
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HIST_MAX		20
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ HIST_ਮਾਪ		CONFIG_SYS_CBSIZE
#ਜੇ_ਅੰਤ

ਸਥਿਰ ਪੂਰਨ hist_max;
ਸਥਿਰ ਪੂਰਨ hist_ਜੋੜੋ_idx;
ਸਥਿਰ ਪੂਰਨ hist_cur = -1;
ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ hist_ਅੰਕ;

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ CONFIG_CMD_HISTORY_USE_CALLOC
ਸਥਿਰ ਅੱਖਰ hist_ਅੰਕੜੇ[HIST_MAX][HIST_ਮਾਪ + 1];
#ਜੇ_ਅੰਤ
ਸਥਿਰ ਅੱਖਰ *hist_list[HIST_MAX];

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਜੋੜੋ_idx_minus_one() ((hist_ਜੋੜੋ_idx == 0) ? hist_max : hist_ਜੋੜੋ_idx-1)

ਸਥਿਰ ਖਾਲੀ getcmd_putchars(ਪੂਰਨ ਗਿਣਤੀ_2, ਪੂਰਨ ch)
{
	ਪੂਰਨ i;

	ਲਈ (i = 0; i < ਗਿਣਤੀ_2; i++)
		getcmd_putch(ch);
}

ਸਥਿਰ ਪੂਰਨ hist_init(ਖਾਲੀ)
{
	ਪੂਰਨ i;

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ CONFIG_CMD_HISTORY_USE_CALLOC
	ਲਈ (i = 0; i < HIST_MAX; i++) {
		hist_list[i] = hist_ਅੰਕੜੇ[i];
		hist_list[i][0] = '\0';
	}
#ਨਹੀਂਤਾਂ
	ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *hist = ਸਿਫਰ_ਸਮ੍ਰਿਤੀ_ਦਿਓ(HIST_MAX, HIST_ਮਾਪ + 1);
	ਜੇ (!hist)
		panic("%s: calloc: out of memory!\n", __func__);

	ਲਈ (i = 0; i < HIST_MAX; i++)
		hist_list[i] = hist + (i * (HIST_ਮਾਪ + 1));
#ਜੇ_ਅੰਤ

	hist_max = 0;
	hist_ਜੋੜੋ_idx = 0;
	hist_cur = -1;
	hist_ਅੰਕ = 0;

	ਵਾਪਸ 0;
}

ਸਥਿਰ ਖਾਲੀ cread_ਜੋੜੋ_ਮੰਜ਼ਿਲ_hist(ਅੱਖਰ *ਸਤਰ)
{
	strcpy(hist_list[hist_ਜੋੜੋ_idx], ਸਤਰ);

	ਜੇ (++hist_ਜੋੜੋ_idx >= HIST_MAX)
		hist_ਜੋੜੋ_idx = 0;

	ਜੇ (hist_ਜੋੜੋ_idx > hist_max)
		hist_max = hist_ਜੋੜੋ_idx;

	hist_ਅੰਕ++;
}

ਸਥਿਰ ਅੱਖਰ *hist_prev(ਖਾਲੀ)
{
	ਅੱਖਰ *ret;
	ਪੂਰਨ old_cur;

	ਜੇ (hist_cur < 0)
		ਵਾਪਸ ਨਿਰਾ;

	old_cur = hist_cur;
	ਜੇ (--hist_cur < 0)
		hist_cur = hist_max;

	ਜੇ (hist_cur == hist_ਜੋੜੋ_idx) {
		hist_cur = old_cur;
		ret = ਨਿਰਾ;
	} ਨਹੀਂਤਾਂ {
		ret = hist_list[hist_cur];
	}

	ਵਾਪਸ ret;
}

ਸਥਿਰ ਅੱਖਰ *hist_ਅਗਲਾ(ਖਾਲੀ)
{
	ਅੱਖਰ *ret;

	ਜੇ (hist_cur < 0)
		ਵਾਪਸ ਨਿਰਾ;

	ਜੇ (hist_cur == hist_ਜੋੜੋ_idx)
		ਵਾਪਸ ਨਿਰਾ;

	ਜੇ (++hist_cur > hist_max)
		hist_cur = 0;

	ਜੇ (hist_cur == hist_ਜੋੜੋ_idx)
		ret = "";
	ਨਹੀਂਤਾਂ
		ret = hist_list[hist_cur];

	ਵਾਪਸ ret;
}

ਖਾਲੀ cread_print_hist_list(ਖਾਲੀ)
{
	ਪੂਰਨ i;
	uint n;

	n = hist_ਅੰਕ - hist_max;

	i = hist_ਜੋੜੋ_idx + 1;
	ਜਦੋਂ (1) {
		ਜੇ (i > hist_max)
			i = 0;
		ਜੇ (i == hist_ਜੋੜੋ_idx)
			ਰੋਕੋ;
		ਲਿਖੋ("%s\n", hist_list[i]);
		n++;
		i++;
	}
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BEGINNING_ਦਾ_ਸਤਰ() {			\
	ਜਦੋਂ (cls->ਅੰਕ) {			\
		getcmd_putch(CTL_BACKSPACE);	\
		cls->ਅੰਕ--;			\
	}					\
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ERASE_ਮੰਜ਼ਿਲ_EOL() {				\
	ਜੇ (cls->ਅੰਕ < cls->eol_ਅੰਕ) {		\
		ਲਿਖੋ("%*s", (ਪੂਰਨ)(cls->eol_ਅੰਕ - cls->ਅੰਕ), ""); \
		ਕਰੋ {					\
			getcmd_putch(CTL_BACKSPACE);	\
		} ਜਦੋਂ (--cls->eol_ਅੰਕ > cls->ਅੰਕ);	\
	}						\
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ REFRESH_ਮੰਜ਼ਿਲ_EOL() {				\
	ਜੇ (cls->ਅੰਕ < cls->eol_ਅੰਕ) {			\
		uint wlen = cls->eol_ਅੰਕ - cls->ਅੰਕ;	\
		putnstr(buf + cls->ਅੰਕ, wlen);		\
		cls->ਅੰਕ = cls->eol_ਅੰਕ;		\
	}						\
}

ਸਥਿਰ ਖਾਲੀ cread_ਜੋੜੋ_char(ਅੱਖਰ ichar, ਪੂਰਨ ਪਾਓ, uint *ਅੰਕ,
			   uint *eol_ਅੰਕ, ਅੱਖਰ *buf, uint len)
{
	uint wlen;

	/* room ??? */
	ਜੇ (ਪਾਓ || *ਅੰਕ == *eol_ਅੰਕ) {
		ਜੇ (*eol_ਅੰਕ > len - 1) {
			getcmd_cbeep();
			ਵਾਪਸ;
		}
		(*eol_ਅੰਕ)++;
	}

	ਜੇ (ਪਾਓ) {
		wlen = *eol_ਅੰਕ - *ਅੰਕ;
		ਜੇ (wlen > 1)
			memmove(&buf[*ਅੰਕ+1], &buf[*ਅੰਕ], wlen-1);

		buf[*ਅੰਕ] = ichar;
		putnstr(buf + *ਅੰਕ, wlen);
		(*ਅੰਕ)++;
		ਜਦੋਂ (--wlen)
			getcmd_putch(CTL_BACKSPACE);
	} ਨਹੀਂਤਾਂ {
		/* echo the character */
		wlen = 1;
		buf[*ਅੰਕ] = ichar;
		putnstr(buf + *ਅੰਕ, wlen);
		(*ਅੰਕ)++;
	}
}

ਸਥਿਰ ਖਾਲੀ cread_ਜੋੜੋ_str(ਅੱਖਰ *str, ਪੂਰਨ strsize, ਪੂਰਨ ਪਾਓ,
			  uint *ਅੰਕ, uint *eol_ਅੰਕ, ਅੱਖਰ *buf, uint len)
{
	ਜਦੋਂ (strsize--) {
		cread_ਜੋੜੋ_char(*str, ਪਾਓ, ਅੰਕ, eol_ਅੰਕ, buf, len);
		str++;
	}
}

ਪੂਰਨ cread_ਸਤਰ_process_ch(ਬਣਤਰ cli_ਸਤਰ_state *cls, ਅੱਖਰ ichar)
{
	ਅੱਖਰ *buf = cls->buf;

	/* ichar=0x0 when error occurs in U-Boot getc */
	ਜੇ (!ichar)
		ਵਾਪਸ -EAGAIN;

	ਜੇ (ichar == '\n') {
		putc('\n');
		buf[cls->eol_ਅੰਕ] = '\0';	/* terminate the string */
		ਵਾਪਸ 0;
	}

	ਚੋਣ (ichar) {
	ਹਾਲਤ CTL_CH('a'):
		BEGINNING_ਦਾ_ਸਤਰ();
		ਰੋਕੋ;
	ਹਾਲਤ CTL_CH('c'):	/* ^C - break */
		*buf = '\0';	/* discard input */
		ਵਾਪਸ -EINTR;
	ਹਾਲਤ CTL_CH('f'):
		ਜੇ (cls->ਅੰਕ < cls->eol_ਅੰਕ) {
			getcmd_putch(buf[cls->ਅੰਕ]);
			cls->ਅੰਕ++;
		}
		ਰੋਕੋ;
	ਹਾਲਤ CTL_CH('b'):
		ਜੇ (cls->ਅੰਕ) {
			getcmd_putch(CTL_BACKSPACE);
			cls->ਅੰਕ--;
		}
		ਰੋਕੋ;
	ਹਾਲਤ CTL_CH('d'):
		ਜੇ (cls->ਅੰਕ < cls->eol_ਅੰਕ) {
			uint wlen;

			wlen = cls->eol_ਅੰਕ - cls->ਅੰਕ - 1;
			ਜੇ (wlen) {
				memmove(&buf[cls->ਅੰਕ], &buf[cls->ਅੰਕ + 1],
					wlen);
				putnstr(buf + cls->ਅੰਕ, wlen);
			}

			getcmd_putch(' ');
			ਕਰੋ {
				getcmd_putch(CTL_BACKSPACE);
			} ਜਦੋਂ (wlen--);
			cls->eol_ਅੰਕ--;
		}
		ਰੋਕੋ;
	ਹਾਲਤ CTL_CH('k'):
		ERASE_ਮੰਜ਼ਿਲ_EOL();
		ਰੋਕੋ;
	ਹਾਲਤ CTL_CH('e'):
		REFRESH_ਮੰਜ਼ਿਲ_EOL();
		ਰੋਕੋ;
	ਹਾਲਤ CTL_CH('o'):
		cls->ਪਾਓ = !cls->ਪਾਓ;
		ਰੋਕੋ;
	ਹਾਲਤ CTL_CH('w'):
		ਜੇ (cls->ਅੰਕ) {
			uint ਅਧਾਰ, wlen;

			ਲਈ (ਅਧਾਰ = cls->ਅੰਕ;
			     ਅਧਾਰ > 0 && buf[ਅਧਾਰ - 1] == ' ';)
				ਅਧਾਰ--;
			ਲਈ (; ਅਧਾਰ > 0 && buf[ਅਧਾਰ - 1] != ' ';)
				ਅਧਾਰ--;

			/* now delete chars from base to cls->num */
			wlen = cls->ਅੰਕ - ਅਧਾਰ;
			cls->eol_ਅੰਕ -= wlen;
			memmove(&buf[ਅਧਾਰ], &buf[cls->ਅੰਕ],
				cls->eol_ਅੰਕ - ਅਧਾਰ + 1);
			cls->ਅੰਕ = ਅਧਾਰ;
			getcmd_putchars(wlen, CTL_BACKSPACE);
			puts(buf + ਅਧਾਰ);
			getcmd_putchars(wlen, ' ');
			getcmd_putchars(wlen + cls->eol_ਅੰਕ - cls->ਅੰਕ,
					CTL_BACKSPACE);
		}
		ਰੋਕੋ;
	ਹਾਲਤ CTL_CH('x'):
	ਹਾਲਤ CTL_CH('u'):
		BEGINNING_ਦਾ_ਸਤਰ();
		ERASE_ਮੰਜ਼ਿਲ_EOL();
		ਰੋਕੋ;
	ਹਾਲਤ DEL:
	ਹਾਲਤ DEL7:
	ਹਾਲਤ 8:
		ਜੇ (cls->ਅੰਕ) {
			uint wlen;

			wlen = cls->eol_ਅੰਕ - cls->ਅੰਕ;
			cls->ਅੰਕ--;
			memmove(&buf[cls->ਅੰਕ], &buf[cls->ਅੰਕ + 1], wlen);
			getcmd_putch(CTL_BACKSPACE);
			putnstr(buf + cls->ਅੰਕ, wlen);
			getcmd_putch(' ');
			ਕਰੋ {
				getcmd_putch(CTL_BACKSPACE);
			} ਜਦੋਂ (wlen--);
			cls->eol_ਅੰਕ--;
		}
		ਰੋਕੋ;
	ਹਾਲਤ CTL_CH('p'):
	ਹਾਲਤ CTL_CH('n'):
		ਜੇ (cls->history) {
			ਅੱਖਰ *hline;

			ਜੇ (ichar == CTL_CH('p'))
				hline = hist_prev();
			ਨਹੀਂਤਾਂ
				hline = hist_ਅਗਲਾ();

			ਜੇ (!hline) {
				getcmd_cbeep();
				ਰੋਕੋ;
			}

			/* nuke the current line */
			/* first, go home */
			BEGINNING_ਦਾ_ਸਤਰ();

			/* erase to end of line */
			ERASE_ਮੰਜ਼ਿਲ_EOL();

			/* copy new line into place and display */
			strcpy(buf, hline);
			cls->eol_ਅੰਕ = ਲੜੀ_ਲੰਬਾਈ(buf);
			REFRESH_ਮੰਜ਼ਿਲ_EOL();
			ਰੋਕੋ;
		}
		ਰੋਕੋ;
	ਹਾਲਤ '\t':
		ਜੇ (IS_ENABLED(CONFIG_AUTO_COMPLETE) && cls->cmd_complete) {
			ਪੂਰਨ num2, ਖਾਨਾ;

			/* do not autocomplete when in the middle */
			ਜੇ (cls->ਅੰਕ < cls->eol_ਅੰਕ) {
				getcmd_cbeep();
				ਰੋਕੋ;
			}

			buf[cls->ਅੰਕ] = '\0';
			ਖਾਨਾ = ਲੜੀ_ਲੰਬਾਈ(cls->prompt) + cls->eol_ਅੰਕ;
			num2 = cls->ਅੰਕ;
			ਜੇ (cmd_auto_complete(cls->prompt, buf, &num2, &ਖਾਨਾ)) {
				ਖਾਨਾ = num2 - cls->ਅੰਕ;
				cls->ਅੰਕ += ਖਾਨਾ;
				cls->eol_ਅੰਕ += ਖਾਨਾ;
			}
			ਰੋਕੋ;
		}
		fallthrough;
	ਮੂਲ:
		cread_ਜੋੜੋ_char(ichar, cls->ਪਾਓ, &cls->ਅੰਕ, &cls->eol_ਅੰਕ,
			       buf, cls->len);
		ਰੋਕੋ;
	}

	/*
	 * keep the string terminated...if we added a char at the end then we
	 * want a \0 after it
	 */
	buf[cls->eol_ਅੰਕ] = '\0';

	ਵਾਪਸ -EAGAIN;
}

ਖਾਲੀ cli_cread_init(ਬਣਤਰ cli_ਸਤਰ_state *cls, ਅੱਖਰ *buf, uint buf_ਮਾਪ)
{
	ਪੂਰਨ init_len = ਲੜੀ_ਲੰਬਾਈ(buf);

	memset(cls, '\0', ਆਕਾਰ(ਬਣਤਰ cli_ਸਤਰ_state));
	cls->ਪਾਓ = true;
	cls->buf = buf;
	cls->len = buf_ਮਾਪ;

	ਜੇ (init_len)
		cread_ਜੋੜੋ_str(buf, init_len, 0, &cls->ਅੰਕ, &cls->eol_ਅੰਕ, buf,
			      buf_ਮਾਪ);
}

ਸਥਿਰ ਪੂਰਨ cread_ਸਤਰ(ਅਚਲ ਅੱਖਰ *ਅਚਲ prompt, ਅੱਖਰ *buf, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *len,
		      ਪੂਰਨ timeout)
{
	ਬਣਤਰ cli_ch_state s_cch, *cch = &s_cch;
	ਬਣਤਰ cli_ਸਤਰ_state s_cls, *cls = &s_cls;
	ਅੱਖਰ ichar;
	ਪੂਰਨ ਪਹਿਲਾ = 1;

	cli_ch_init(cch);
	cli_cread_init(cls, buf, *len);
	cls->prompt = prompt;
	cls->history = true;
	cls->cmd_complete = true;

	ਜਦੋਂ (1) {
		ਪੂਰਨ ret;

		/* Check for saved characters */
		ichar = cli_ch_process(cch, 0);

		ਜੇ (!ichar) {
			ਜੇ (bootretry_tstc_timeout())
				ਵਾਪਸ -2;	/* timed out */
			ਜੇ (ਪਹਿਲਾ && timeout) {
				u64 etime = endtick(timeout);

				ਜਦੋਂ (!tstc()) {	/* while no incoming data */
					ਜੇ (get_ticks() >= etime)
						ਵਾਪਸ -2;	/* timed out */
					ਸਮਾਂ_ਸਾਰਣੀ();
				}
				ਪਹਿਲਾ = 0;
			}

			ichar = getcmd_getch();
			ichar = cli_ch_process(cch, ichar);
		}

		ret = cread_ਸਤਰ_process_ch(cls, ichar);
		ਜੇ (ret == -EINTR)
			ਵਾਪਸ -1;
		ਨਹੀਂਤਾਂ ਜੇ (!ret)
			ਰੋਕੋ;
	}
	*len = cls->eol_ਅੰਕ;

	ਜੇ (buf[0] && buf[0] != CREAD_HIST_CHAR)
		cread_ਜੋੜੋ_ਮੰਜ਼ਿਲ_hist(buf);
	hist_cur = hist_ਜੋੜੋ_idx;

	ਵਾਪਸ 0;
}

#ਨਹੀਂਤਾਂ /* !CONFIG_CMDLINE_EDITING */

ਸਥਿਰ ਸਤਰਅੰਦਰ ਪੂਰਨ hist_init(ਖਾਲੀ)
{
	ਵਾਪਸ 0;
}

ਸਥਿਰ ਪੂਰਨ cread_ਸਤਰ(ਅਚਲ ਅੱਖਰ *ਅਚਲ prompt, ਅੱਖਰ *buf, ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ *len,
		      ਪੂਰਨ timeout)
{
	ਵਾਪਸ 0;
}

#ਜੇ_ਅੰਤ /* CONFIG_CMDLINE_EDITING */

/****************************************************************************/

ਪੂਰਨ cli_readline(ਅਚਲ ਅੱਖਰ *ਅਚਲ prompt)
{
	/*
	 * If console_buffer isn't 0-length the user will be prompted to modify
	 * it instead of entering it from scratch as desired.
	 */
	console_ਬਫਰ[0] = '\0';

	ਵਾਪਸ cli_readline_into_ਬਫਰ(prompt, console_ਬਫਰ, 0);
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
ਸਥਿਰ ਪੂਰਨ cread_ਸਤਰ_simple(ਅਚਲ ਅੱਖਰ *ਅਚਲ prompt, ਅੱਖਰ *p)
{
	ਅੱਖਰ *p_buf = p;
	ਪੂਰਨ n = 0;		/* buffer index */
	ਪੂਰਨ plen = 0;		/* prompt length */
	ਪੂਰਨ ਖਾਨਾ;		/* output column cnt */
	ਪੂਰਨ c;

	/* print prompt */
	ਜੇ (prompt) {
		plen = ਲੜੀ_ਲੰਬਾਈ(prompt);
		puts(prompt);
	}
	ਖਾਨਾ = plen;

	ਲਈ (;;) {
		ਜੇ (bootretry_tstc_timeout())
			ਵਾਪਸ -2;	/* timed out */
		ਸਮਾਂ_ਸਾਰਣੀ();	/* Trigger watchdog, if needed */

		c = getchar();

		/*
		 * Special character handling
		 */
		ਚੋਣ (c) {
		ਹਾਲਤ '\r':			/* Enter		*/
		ਹਾਲਤ '\n':
			*p = '\0';
			puts("\r\n");
			ਵਾਪਸ p - p_buf;

		ਹਾਲਤ '\0':			/* nul			*/
			ਜਾਰੀ;

		ਹਾਲਤ 0x03:			/* ^C - break		*/
			p_buf[0] = '\0';	/* discard input */
			ਵਾਪਸ -1;

		ਹਾਲਤ 0x15:			/* ^U - erase line	*/
			ਜਦੋਂ (ਖਾਨਾ > plen) {
				puts(erase_seq);
				--ਖਾਨਾ;
			}
			p = p_buf;
			n = 0;
			ਜਾਰੀ;

		ਹਾਲਤ 0x17:			/* ^W - erase word	*/
			p = delete_char(p_buf, p, &ਖਾਨਾ, &n, plen);
			ਜਦੋਂ ((n > 0) && (*p != ' '))
				p = delete_char(p_buf, p, &ਖਾਨਾ, &n, plen);
			ਜਾਰੀ;

		ਹਾਲਤ 0x08:			/* ^H  - backspace	*/
		ਹਾਲਤ 0x7F:			/* DEL - backspace	*/
			p = delete_char(p_buf, p, &ਖਾਨਾ, &n, plen);
			ਜਾਰੀ;

		ਮੂਲ:
			/* Must be a normal character then */
			ਜੇ (n >= CONFIG_SYS_CBSIZE - 2) { /* Buffer full */
				putc('\a');
				ਰੋਕੋ;
			}
			ਜੇ (c == '\t') {	/* expand TABs */
				ਜੇ (IS_ENABLED(CONFIG_AUTO_COMPLETE)) {
					/*
					 * if auto-completion triggered just
					 * continue
					 */
					*p = '\0';
					ਜੇ (cmd_auto_complete(prompt,
							      console_ਬਫਰ,
							      &n, &ਖਾਨਾ)) {
						p = p_buf + n;	/* reset */
						ਜਾਰੀ;
					}
				}
				puts(tab_seq + (ਖਾਨਾ & 07));
				ਖਾਨਾ += 8 - (ਖਾਨਾ & 07);
			} ਨਹੀਂਤਾਂ {
				ਅੱਖਰ __maybe_unused buf[2];

				/*
				 * Echo input using puts() to force an LCD
				 * flush if we are using an LCD
				 */
				++ਖਾਨਾ;
				buf[0] = c;
				buf[1] = '\0';
				puts(buf);
			}
			*p++ = c;
			++n;
			ਰੋਕੋ;
		}
	}
}

ਪੂਰਨ cli_readline_into_ਬਫਰ(ਅਚਲ ਅੱਖਰ *ਅਚਲ prompt, ਅੱਖਰ *ਬਫਰ,
			     ਪੂਰਨ timeout)
{
	ਅੱਖਰ *p = ਬਫਰ;
	uint len = CONFIG_SYS_CBSIZE;
	ਪੂਰਨ rc;
	ਸਥਿਰ ਪੂਰਨ initted;

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
	ਜੇ (IS_ENABLED(CONFIG_CMDLINE_EDITING) && (gd->flags & GD_FLG_RELOC)) {
		ਜੇ (!initted) {
			rc = hist_init();
			ਜੇ (rc == 0)
				initted = 1;
		}

		ਜੇ (prompt)
			puts(prompt);

		rc = cread_ਸਤਰ(prompt, p, &len, timeout);
		ਵਾਪਸ rc < 0 ? rc : len;

	} ਨਹੀਂਤਾਂ {
		ਵਾਪਸ cread_ਸਤਰ_simple(prompt, p);
	}
}
