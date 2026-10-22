// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Add to readline cmdline-editing by
 * (C) Copyright 2005
 * JinHua Luo, GuangDong Linux Center, <luo.jinhua@gd-linux.com>
 */

#ಅನ್ತರ್ಭಾವ <bootretry.h>
#ಅನ್ತರ್ಭಾವ <cli.h>
#ಅನ್ತರ್ಭಾವ <command.h>
#ಅನ್ತರ್ಭಾವ <hang.h>
#ಅನ್ತರ್ಭಾವ <malloc.h>
#ಅನ್ತರ್ಭಾವ <time.h>
#ಅನ್ತರ್ಭಾವ <watchdog.h>
#ಅನ್ತರ್ಭಾವ <linux/errno.h>
#ಅನ್ತರ್ಭಾವ <asm/global_data.h>

DECLARE_GLOBAL_DATA_PTR;

ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ erase_seq[] = "\b \b";	/* erase sequence */
ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ   tab_seq[] = "        ";	/* used to expand TABs */

ವರ್ಣ console_buffer[CONFIG_SYS_CBSIZE + 1];	/* console I/O buffer	*/

ಸ್ಥಿರ ವರ್ಣ *delete_char (ವರ್ಣ *buffer, ವರ್ಣ *p, ಪೂರ್ಣಾಙ್ಕ *colp, ಪೂರ್ಣಾಙ್ಕ *np, ಪೂರ್ಣಾಙ್ಕ plen)
{
	ವರ್ಣ *s;

	ಯದಿ (*np == 0)
		ಪ್ರತ್ಯಾಗಮ p;

	ಯದಿ (*(--p) == '\t') {		/* will retype the whole line */
		ಯಾವತ್ (*colp > plen) {
			puts(erase_seq);
			(*colp)--;
		}
		ಪ್ರತಿ (s = buffer; s < p; ++s) {
			ಯದಿ (*s == '\t') {
				puts(tab_seq + ((*colp) & 07));
				*colp += 8 - ((*colp) & 07);
			} ಅನ್ಯಥಾ {
				++(*colp);
				putc(*s);
			}
		}
	} ಅನ್ಯಥಾ {
		puts(erase_seq);
		(*colp)--;
	}
	(*np)--;

	ಪ್ರತ್ಯಾಗಮ p;
}

#ifdef CONFIG_CMDLINE_EDITING

/*
 * cmdline-editing related codes from vivi.
 * Author: Janghoon Lyu <nandy@mizi.com>
 */

#define putnstr(str, n)	printf("%.*s", (ಪೂರ್ಣಾಙ್ಕ)n, str)

#define CTL_BACKSPACE		('\b')
#define DEL			((ವರ್ಣ)255)
#define DEL7			((ವರ್ಣ)127)
#define CREAD_HIST_CHAR		('!')

#define getcmd_putch(ch)	putc(ch)
#define getcmd_getch()		getchar()
#define getcmd_cbeep()		getcmd_putch('\a')

#ifdef CONFIG_XPL_BUILD
#define HIST_MAX		3
#define HIST_SIZE		32
#ಅನ್ಯಥಾ
#define HIST_MAX		20
#define HIST_SIZE		CONFIG_SYS_CBSIZE
#endif

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ hist_max;
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ hist_add_idx;
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ hist_cur = -1;
ಸ್ಥಿರ unsigned hist_ಅಂಕೆ;

#ifndef CONFIG_CMD_HISTORY_USE_CALLOC
ಸ್ಥಿರ ವರ್ಣ hist_data[HIST_MAX][HIST_SIZE + 1];
#endif
ಸ್ಥಿರ ವರ್ಣ *hist_list[HIST_MAX];

#define add_idx_minus_one() ((hist_add_idx == 0) ? hist_max : hist_add_idx-1)

ಸ್ಥಿರ ಶೂನ್ಯ getcmd_putchars(ಪೂರ್ಣಾಙ್ಕ ಎಣಿಕೆ, ಪೂರ್ಣಾಙ್ಕ ch)
{
	ಪೂರ್ಣಾಙ್ಕ i;

	ಪ್ರತಿ (i = 0; i < ಎಣಿಕೆ; i++)
		getcmd_putch(ch);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ hist_init(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಙ್ಕ i;

#ifndef CONFIG_CMD_HISTORY_USE_CALLOC
	ಪ್ರತಿ (i = 0; i < HIST_MAX; i++) {
		hist_list[i] = hist_data[i];
		hist_list[i][0] = '\0';
	}
#ಅನ್ಯಥಾ
	unsigned ವರ್ಣ *hist = calloc(HIST_MAX, HIST_SIZE + 1);
	ಯದಿ (!hist)
		panic("%s: calloc: out of memory!\n", __func__);

	ಪ್ರತಿ (i = 0; i < HIST_MAX; i++)
		hist_list[i] = hist + (i * (HIST_SIZE + 1));
#endif

	hist_max = 0;
	hist_add_idx = 0;
	hist_cur = -1;
	hist_ಅಂಕೆ = 0;

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಶೂನ್ಯ cread_add_to_hist(ವರ್ಣ *ಸಾಲು)
{
	strcpy(hist_list[hist_add_idx], ಸಾಲು);

	ಯದಿ (++hist_add_idx >= HIST_MAX)
		hist_add_idx = 0;

	ಯದಿ (hist_add_idx > hist_max)
		hist_max = hist_add_idx;

	hist_ಅಂಕೆ++;
}

ಸ್ಥಿರ ವರ್ಣ *hist_prev(ಶೂನ್ಯ)
{
	ವರ್ಣ *ret;
	ಪೂರ್ಣಾಙ್ಕ old_cur;

	ಯದಿ (hist_cur < 0)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	old_cur = hist_cur;
	ಯದಿ (--hist_cur < 0)
		hist_cur = hist_max;

	ಯದಿ (hist_cur == hist_add_idx) {
		hist_cur = old_cur;
		ret = ರಿಕ್ತ;
	} ಅನ್ಯಥಾ {
		ret = hist_list[hist_cur];
	}

	ಪ್ರತ್ಯಾಗಮ ret;
}

ಸ್ಥಿರ ವರ್ಣ *hist_next(ಶೂನ್ಯ)
{
	ವರ್ಣ *ret;

	ಯದಿ (hist_cur < 0)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	ಯದಿ (hist_cur == hist_add_idx)
		ಪ್ರತ್ಯಾಗಮ ರಿಕ್ತ;

	ಯದಿ (++hist_cur > hist_max)
		hist_cur = 0;

	ಯದಿ (hist_cur == hist_add_idx)
		ret = "";
	ಅನ್ಯಥಾ
		ret = hist_list[hist_cur];

	ಪ್ರತ್ಯಾಗಮ ret;
}

ಶೂನ್ಯ cread_print_hist_list(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಙ್ಕ i;
	uint n;

	n = hist_ಅಂಕೆ - hist_max;

	i = hist_add_idx + 1;
	ಯಾವತ್ (1) {
		ಯದಿ (i > hist_max)
			i = 0;
		ಯದಿ (i == hist_add_idx)
			ಭಙ್ಗ;
		printf("%s\n", hist_list[i]);
		n++;
		i++;
	}
}

#define BEGINNING_OF_ಸಾಲು() {			\
	ಯಾವತ್ (cls->ಅಂಕೆ) {			\
		getcmd_putch(CTL_BACKSPACE);	\
		cls->ಅಂಕೆ--;			\
	}					\
}

#define ERASE_TO_EOL() {				\
	ಯದಿ (cls->ಅಂಕೆ < cls->eol_ಅಂಕೆ) {		\
		printf("%*s", (ಪೂರ್ಣಾಙ್ಕ)(cls->eol_ಅಂಕೆ - cls->ಅಂಕೆ), ""); \
		ಕುರು {					\
			getcmd_putch(CTL_BACKSPACE);	\
		} ಯಾವತ್ (--cls->eol_ಅಂಕೆ > cls->ಅಂಕೆ);	\
	}						\
}

#define REFRESH_TO_EOL() {				\
	ಯದಿ (cls->ಅಂಕೆ < cls->eol_ಅಂಕೆ) {			\
		uint wlen = cls->eol_ಅಂಕೆ - cls->ಅಂಕೆ;	\
		putnstr(buf + cls->ಅಂಕೆ, wlen);		\
		cls->ಅಂಕೆ = cls->eol_ಅಂಕೆ;		\
	}						\
}

ಸ್ಥಿರ ಶೂನ್ಯ cread_add_char(ವರ್ಣ ichar, ಪೂರ್ಣಾಙ್ಕ ಸೇರಿಸು, uint *ಅಂಕೆ,
			   uint *eol_ಅಂಕೆ, ವರ್ಣ *buf, uint len)
{
	uint wlen;

	/* room ??? */
	ಯದಿ (ಸೇರಿಸು || *ಅಂಕೆ == *eol_ಅಂಕೆ) {
		ಯದಿ (*eol_ಅಂಕೆ > len - 1) {
			getcmd_cbeep();
			ಪ್ರತ್ಯಾಗಮ;
		}
		(*eol_ಅಂಕೆ)++;
	}

	ಯದಿ (ಸೇರಿಸು) {
		wlen = *eol_ಅಂಕೆ - *ಅಂಕೆ;
		ಯದಿ (wlen > 1)
			memmove(&buf[*ಅಂಕೆ+1], &buf[*ಅಂಕೆ], wlen-1);

		buf[*ಅಂಕೆ] = ichar;
		putnstr(buf + *ಅಂಕೆ, wlen);
		(*ಅಂಕೆ)++;
		ಯಾವತ್ (--wlen)
			getcmd_putch(CTL_BACKSPACE);
	} ಅನ್ಯಥಾ {
		/* echo the character */
		wlen = 1;
		buf[*ಅಂಕೆ] = ichar;
		putnstr(buf + *ಅಂಕೆ, wlen);
		(*ಅಂಕೆ)++;
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ cread_add_str(ವರ್ಣ *str, ಪೂರ್ಣಾಙ್ಕ strsize, ಪೂರ್ಣಾಙ್ಕ ಸೇರಿಸು,
			  uint *ಅಂಕೆ, uint *eol_ಅಂಕೆ, ವರ್ಣ *buf, uint len)
{
	ಯಾವತ್ (strsize--) {
		cread_add_char(*str, ಸೇರಿಸು, ಅಂಕೆ, eol_ಅಂಕೆ, buf, len);
		str++;
	}
}

ಪೂರ್ಣಾಙ್ಕ cread_ಸಾಲು_process_ch(ಸಂರಚನಾ cli_ಸಾಲು_state *cls, ವರ್ಣ ichar)
{
	ವರ್ಣ *buf = cls->buf;

	/* ichar=0x0 when error occurs in U-Boot getc */
	ಯದಿ (!ichar)
		ಪ್ರತ್ಯಾಗಮ -EAGAIN;

	ಯದಿ (ichar == '\n') {
		putc('\n');
		buf[cls->eol_ಅಂಕೆ] = '\0';	/* terminate the string */
		ಪ್ರತ್ಯಾಗಮ 0;
	}

	ವಿಕಲ್ಪ (ichar) {
	ಪಕ್ಷ CTL_CH('a'):
		BEGINNING_OF_ಸಾಲು();
		ಭಙ್ಗ;
	ಪಕ್ಷ CTL_CH('c'):	/* ^C - break */
		*buf = '\0';	/* discard input */
		ಪ್ರತ್ಯಾಗಮ -EINTR;
	ಪಕ್ಷ CTL_CH('f'):
		ಯದಿ (cls->ಅಂಕೆ < cls->eol_ಅಂಕೆ) {
			getcmd_putch(buf[cls->ಅಂಕೆ]);
			cls->ಅಂಕೆ++;
		}
		ಭಙ್ಗ;
	ಪಕ್ಷ CTL_CH('b'):
		ಯದಿ (cls->ಅಂಕೆ) {
			getcmd_putch(CTL_BACKSPACE);
			cls->ಅಂಕೆ--;
		}
		ಭಙ್ಗ;
	ಪಕ್ಷ CTL_CH('d'):
		ಯದಿ (cls->ಅಂಕೆ < cls->eol_ಅಂಕೆ) {
			uint wlen;

			wlen = cls->eol_ಅಂಕೆ - cls->ಅಂಕೆ - 1;
			ಯದಿ (wlen) {
				memmove(&buf[cls->ಅಂಕೆ], &buf[cls->ಅಂಕೆ + 1],
					wlen);
				putnstr(buf + cls->ಅಂಕೆ, wlen);
			}

			getcmd_putch(' ');
			ಕುರು {
				getcmd_putch(CTL_BACKSPACE);
			} ಯಾವತ್ (wlen--);
			cls->eol_ಅಂಕೆ--;
		}
		ಭಙ್ಗ;
	ಪಕ್ಷ CTL_CH('k'):
		ERASE_TO_EOL();
		ಭಙ್ಗ;
	ಪಕ್ಷ CTL_CH('e'):
		REFRESH_TO_EOL();
		ಭಙ್ಗ;
	ಪಕ್ಷ CTL_CH('o'):
		cls->ಸೇರಿಸು = !cls->ಸೇರಿಸು;
		ಭಙ್ಗ;
	ಪಕ್ಷ CTL_CH('w'):
		ಯದಿ (cls->ಅಂಕೆ) {
			uint base, wlen;

			ಪ್ರತಿ (base = cls->ಅಂಕೆ;
			     base > 0 && buf[base - 1] == ' ';)
				base--;
			ಪ್ರತಿ (; base > 0 && buf[base - 1] != ' ';)
				base--;

			/* now delete chars from base to cls->num */
			wlen = cls->ಅಂಕೆ - base;
			cls->eol_ಅಂಕೆ -= wlen;
			memmove(&buf[base], &buf[cls->ಅಂಕೆ],
				cls->eol_ಅಂಕೆ - base + 1);
			cls->ಅಂಕೆ = base;
			getcmd_putchars(wlen, CTL_BACKSPACE);
			puts(buf + base);
			getcmd_putchars(wlen, ' ');
			getcmd_putchars(wlen + cls->eol_ಅಂಕೆ - cls->ಅಂಕೆ,
					CTL_BACKSPACE);
		}
		ಭಙ್ಗ;
	ಪಕ್ಷ CTL_CH('x'):
	ಪಕ್ಷ CTL_CH('u'):
		BEGINNING_OF_ಸಾಲು();
		ERASE_TO_EOL();
		ಭಙ್ಗ;
	ಪಕ್ಷ DEL:
	ಪಕ್ಷ DEL7:
	ಪಕ್ಷ 8:
		ಯದಿ (cls->ಅಂಕೆ) {
			uint wlen;

			wlen = cls->eol_ಅಂಕೆ - cls->ಅಂಕೆ;
			cls->ಅಂಕೆ--;
			memmove(&buf[cls->ಅಂಕೆ], &buf[cls->ಅಂಕೆ + 1], wlen);
			getcmd_putch(CTL_BACKSPACE);
			putnstr(buf + cls->ಅಂಕೆ, wlen);
			getcmd_putch(' ');
			ಕುರು {
				getcmd_putch(CTL_BACKSPACE);
			} ಯಾವತ್ (wlen--);
			cls->eol_ಅಂಕೆ--;
		}
		ಭಙ್ಗ;
	ಪಕ್ಷ CTL_CH('p'):
	ಪಕ್ಷ CTL_CH('n'):
		ಯದಿ (cls->history) {
			ವರ್ಣ *hline;

			ಯದಿ (ichar == CTL_CH('p'))
				hline = hist_prev();
			ಅನ್ಯಥಾ
				hline = hist_next();

			ಯದಿ (!hline) {
				getcmd_cbeep();
				ಭಙ್ಗ;
			}

			/* nuke the current line */
			/* first, go home */
			BEGINNING_OF_ಸಾಲು();

			/* erase to end of line */
			ERASE_TO_EOL();

			/* copy new line into place and display */
			strcpy(buf, hline);
			cls->eol_ಅಂಕೆ = strlen(buf);
			REFRESH_TO_EOL();
			ಭಙ್ಗ;
		}
		ಭಙ್ಗ;
	ಪಕ್ಷ '\t':
		ಯದಿ (IS_ENABLED(CONFIG_AUTO_COMPLETE) && cls->cmd_complete) {
			ಪೂರ್ಣಾಙ್ಕ num2, col;

			/* do not autocomplete when in the middle */
			ಯದಿ (cls->ಅಂಕೆ < cls->eol_ಅಂಕೆ) {
				getcmd_cbeep();
				ಭಙ್ಗ;
			}

			buf[cls->ಅಂಕೆ] = '\0';
			col = strlen(cls->prompt) + cls->eol_ಅಂಕೆ;
			num2 = cls->ಅಂಕೆ;
			ಯದಿ (cmd_auto_complete(cls->prompt, buf, &num2, &col)) {
				col = num2 - cls->ಅಂಕೆ;
				cls->ಅಂಕೆ += col;
				cls->eol_ಅಂಕೆ += col;
			}
			ಭಙ್ಗ;
		}
		fallthrough;
	ಸ್ವಭಾವ:
		cread_add_char(ichar, cls->ಸೇರಿಸು, &cls->ಅಂಕೆ, &cls->eol_ಅಂಕೆ,
			       buf, cls->len);
		ಭಙ್ಗ;
	}

	/*
	 * keep the string terminated...if we added a char at the end then we
	 * want a \0 after it
	 */
	buf[cls->eol_ಅಂಕೆ] = '\0';

	ಪ್ರತ್ಯಾಗಮ -EAGAIN;
}

ಶೂನ್ಯ cli_cread_init(ಸಂರಚನಾ cli_ಸಾಲು_state *cls, ವರ್ಣ *buf, uint buf_size)
{
	ಪೂರ್ಣಾಙ್ಕ init_len = strlen(buf);

	memset(cls, '\0', ಪರಿಮಾಣ(ಸಂರಚನಾ cli_ಸಾಲು_state));
	cls->ಸೇರಿಸು = true;
	cls->buf = buf;
	cls->len = buf_size;

	ಯದಿ (init_len)
		cread_add_str(buf, init_len, 0, &cls->ಅಂಕೆ, &cls->eol_ಅಂಕೆ, buf,
			      buf_size);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ cread_ಸಾಲು(ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ prompt, ವರ್ಣ *buf, unsigned ಪೂರ್ಣಾಙ್ಕ *len,
		      ಪೂರ್ಣಾಙ್ಕ timeout)
{
	ಸಂರಚನಾ cli_ch_state s_cch, *cch = &s_cch;
	ಸಂರಚನಾ cli_ಸಾಲು_state s_cls, *cls = &s_cls;
	ವರ್ಣ ichar;
	ಪೂರ್ಣಾಙ್ಕ first = 1;

	cli_ch_init(cch);
	cli_cread_init(cls, buf, *len);
	cls->prompt = prompt;
	cls->history = true;
	cls->cmd_complete = true;

	ಯಾವತ್ (1) {
		ಪೂರ್ಣಾಙ್ಕ ret;

		/* Check for saved characters */
		ichar = cli_ch_process(cch, 0);

		ಯದಿ (!ichar) {
			ಯದಿ (bootretry_tstc_timeout())
				ಪ್ರತ್ಯಾಗಮ -2;	/* timed out */
			ಯದಿ (first && timeout) {
				u64 etime = endtick(timeout);

				ಯಾವತ್ (!tstc()) {	/* while no incoming data */
					ಯದಿ (get_ticks() >= etime)
						ಪ್ರತ್ಯಾಗಮ -2;	/* timed out */
					schedule();
				}
				first = 0;
			}

			ichar = getcmd_getch();
			ichar = cli_ch_process(cch, ichar);
		}

		ret = cread_ಸಾಲು_process_ch(cls, ichar);
		ಯದಿ (ret == -EINTR)
			ಪ್ರತ್ಯಾಗಮ -1;
		ಅನ್ಯಥಾ ಯದಿ (!ret)
			ಭಙ್ಗ;
	}
	*len = cls->eol_ಅಂಕೆ;

	ಯದಿ (buf[0] && buf[0] != CREAD_HIST_CHAR)
		cread_add_to_hist(buf);
	hist_cur = hist_add_idx;

	ಪ್ರತ್ಯಾಗಮ 0;
}

#ಅನ್ಯಥಾ /* !CONFIG_CMDLINE_EDITING */

ಸ್ಥಿರ ಅನ್ತಃಸ್ಥ ಪೂರ್ಣಾಙ್ಕ hist_init(ಶೂನ್ಯ)
{
	ಪ್ರತ್ಯಾಗಮ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ cread_ಸಾಲು(ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ prompt, ವರ್ಣ *buf, unsigned ಪೂರ್ಣಾಙ್ಕ *len,
		      ಪೂರ್ಣಾಙ್ಕ timeout)
{
	ಪ್ರತ್ಯಾಗಮ 0;
}

#endif /* CONFIG_CMDLINE_EDITING */

/****************************************************************************/

ಪೂರ್ಣಾಙ್ಕ cli_readline(ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ prompt)
{
	/*
	 * If console_buffer isn't 0-length the user will be prompted to modify
	 * it instead of entering it from scratch as desired.
	 */
	console_buffer[0] = '\0';

	ಪ್ರತ್ಯಾಗಮ cli_readline_into_buffer(prompt, console_buffer, 0);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ cread_ಸಾಲು_simple(ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ prompt, ವರ್ಣ *p)
{
	ವರ್ಣ *p_buf = p;
	ಪೂರ್ಣಾಙ್ಕ n = 0;		/* buffer index */
	ಪೂರ್ಣಾಙ್ಕ plen = 0;		/* prompt length */
	ಪೂರ್ಣಾಙ್ಕ col;		/* output column cnt */
	ಪೂರ್ಣಾಙ್ಕ c;

	/* print prompt */
	ಯದಿ (prompt) {
		plen = strlen(prompt);
		puts(prompt);
	}
	col = plen;

	ಪ್ರತಿ (;;) {
		ಯದಿ (bootretry_tstc_timeout())
			ಪ್ರತ್ಯಾಗಮ -2;	/* timed out */
		schedule();	/* Trigger watchdog, if needed */

		c = getchar();

		/*
		 * Special character handling
		 */
		ವಿಕಲ್ಪ (c) {
		ಪಕ್ಷ '\r':			/* Enter		*/
		ಪಕ್ಷ '\n':
			*p = '\0';
			puts("\r\n");
			ಪ್ರತ್ಯಾಗಮ p - p_buf;

		ಪಕ್ಷ '\0':			/* nul			*/
			ಅನುವರ್ತ;

		ಪಕ್ಷ 0x03:			/* ^C - break		*/
			p_buf[0] = '\0';	/* discard input */
			ಪ್ರತ್ಯಾಗಮ -1;

		ಪಕ್ಷ 0x15:			/* ^U - erase line	*/
			ಯಾವತ್ (col > plen) {
				puts(erase_seq);
				--col;
			}
			p = p_buf;
			n = 0;
			ಅನುವರ್ತ;

		ಪಕ್ಷ 0x17:			/* ^W - erase word	*/
			p = delete_char(p_buf, p, &col, &n, plen);
			ಯಾವತ್ ((n > 0) && (*p != ' '))
				p = delete_char(p_buf, p, &col, &n, plen);
			ಅನುವರ್ತ;

		ಪಕ್ಷ 0x08:			/* ^H  - backspace	*/
		ಪಕ್ಷ 0x7F:			/* DEL - backspace	*/
			p = delete_char(p_buf, p, &col, &n, plen);
			ಅನುವರ್ತ;

		ಸ್ವಭಾವ:
			/* Must be a normal character then */
			ಯದಿ (n >= CONFIG_SYS_CBSIZE - 2) { /* Buffer full */
				putc('\a');
				ಭಙ್ಗ;
			}
			ಯದಿ (c == '\t') {	/* expand TABs */
				ಯದಿ (IS_ENABLED(CONFIG_AUTO_COMPLETE)) {
					/*
					 * if auto-completion triggered just
					 * continue
					 */
					*p = '\0';
					ಯದಿ (cmd_auto_complete(prompt,
							      console_buffer,
							      &n, &col)) {
						p = p_buf + n;	/* reset */
						ಅನುವರ್ತ;
					}
				}
				puts(tab_seq + (col & 07));
				col += 8 - (col & 07);
			} ಅನ್ಯಥಾ {
				ವರ್ಣ __maybe_unused buf[2];

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
			ಭಙ್ಗ;
		}
	}
}

ಪೂರ್ಣಾಙ್ಕ cli_readline_into_buffer(ಧ್ರುವ ವರ್ಣ *ಧ್ರುವ prompt, ವರ್ಣ *buffer,
			     ಪೂರ್ಣಾಙ್ಕ timeout)
{
	ವರ್ಣ *p = buffer;
	uint len = CONFIG_SYS_CBSIZE;
	ಪೂರ್ಣಾಙ್ಕ rc;
	ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ initted;

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
	ಯದಿ (IS_ENABLED(CONFIG_CMDLINE_EDITING) && (gd->flags & GD_FLG_RELOC)) {
		ಯದಿ (!initted) {
			rc = hist_init();
			ಯದಿ (rc == 0)
				initted = 1;
		}

		ಯದಿ (prompt)
			puts(prompt);

		rc = cread_ಸಾಲು(prompt, p, &len, timeout);
		ಪ್ರತ್ಯಾಗಮ rc < 0 ? rc : len;

	} ಅನ್ಯಥಾ {
		ಪ್ರತ್ಯಾಗಮ cread_ಸಾಲು_simple(prompt, p);
	}
}
