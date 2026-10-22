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

DECLARE_GLOBAL_ದತ್ತಾಂಶ_PTR;

ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ erase_seq[] = "\b \b";	/* erase sequence */
ಸ್ಥಿರ ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ   tab_seq[] = "        ";	/* used to expand TABs */

ಅಕ್ಷರ console_ಬಫರ್[CONFIG_SYS_CBSIZE + 1];	/* console I/O buffer	*/

ಸ್ಥಿರ ಅಕ್ಷರ *delete_char (ಅಕ್ಷರ *ಬಫರ್, ಅಕ್ಷರ *p, ಪೂರ್ಣಾಂಕ *colp, ಪೂರ್ಣಾಂಕ *np, ಪೂರ್ಣಾಂಕ plen)
{
	ಅಕ್ಷರ *s;

	ಆದರೆ (*np == 0)
		ಹಿಂತಿರುಗಿ p;

	ಆದರೆ (*(--p) == '\t') {		/* will retype the whole line */
		ಆಗ (*colp > plen) {
			puts(erase_seq);
			(*colp)--;
		}
		ಗಾಗಿ (s = ಬಫರ್; s < p; ++s) {
			ಆದರೆ (*s == '\t') {
				puts(tab_seq + ((*colp) & 07));
				*colp += 8 - ((*colp) & 07);
			} ಇಲ್ಲದಿದ್ದರೆ {
				++(*colp);
				putc(*s);
			}
		}
	} ಇಲ್ಲದಿದ್ದರೆ {
		puts(erase_seq);
		(*colp)--;
	}
	(*np)--;

	ಹಿಂತಿರುಗಿ p;
}

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_CMDLINE_EDITING

/*
 * cmdline-editing related codes from vivi.
 * Author: Janghoon Lyu <nandy@mizi.com>
 */

#ವ್ಯಾಖ್ಯಾನಿಸು putnstr(str, n)	ಬರೆ("%.*s", (ಪೂರ್ಣಾಂಕ)n, str)

#ವ್ಯಾಖ್ಯಾನಿಸು CTL_BACKSPACE		('\b')
#ವ್ಯಾಖ್ಯಾನಿಸು DEL			((ಅಕ್ಷರ)255)
#ವ್ಯಾಖ್ಯಾನಿಸು DEL7			((ಅಕ್ಷರ)127)
#ವ್ಯಾಖ್ಯಾನಿಸು CREAD_HIST_CHAR		('!')

#ವ್ಯಾಖ್ಯಾನಿಸು getcmd_putch(ch)	putc(ch)
#ವ್ಯಾಖ್ಯಾನಿಸು getcmd_getch()		getchar()
#ವ್ಯಾಖ್ಯಾನಿಸು getcmd_cbeep()		getcmd_putch('\a')

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ CONFIG_XPL_BUILD
#ವ್ಯಾಖ್ಯಾನಿಸು HIST_MAX		3
#ವ್ಯಾಖ್ಯಾನಿಸು HIST_ಗಾತ್ರ		32
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು HIST_MAX		20
#ವ್ಯಾಖ್ಯಾನಿಸು HIST_ಗಾತ್ರ		CONFIG_SYS_CBSIZE
#ಆದರೆ_ಅಂತ್ಯ

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ hist_max;
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ hist_ಕೂಡಿಸು_idx;
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ hist_cur = -1;
ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ hist_ಅಂಕೆ;

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ CONFIG_CMD_HISTORY_USE_CALLOC
ಸ್ಥಿರ ಅಕ್ಷರ hist_ದತ್ತಾಂಶ[HIST_MAX][HIST_ಗಾತ್ರ + 1];
#ಆದರೆ_ಅಂತ್ಯ
ಸ್ಥಿರ ಅಕ್ಷರ *hist_list[HIST_MAX];

#ವ್ಯಾಖ್ಯಾನಿಸು ಕೂಡಿಸು_idx_minus_one() ((hist_ಕೂಡಿಸು_idx == 0) ? hist_max : hist_ಕೂಡಿಸು_idx-1)

ಸ್ಥಿರ ಶೂನ್ಯ getcmd_putchars(ಪೂರ್ಣಾಂಕ ಎಣಿಕೆ_2, ಪೂರ್ಣಾಂಕ ch)
{
	ಪೂರ್ಣಾಂಕ i;

	ಗಾಗಿ (i = 0; i < ಎಣಿಕೆ_2; i++)
		getcmd_putch(ch);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ hist_init(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಂಕ i;

#ವ್ಯಾಖ್ಯಾನಿತವಾಗದಿದ್ದರೆ CONFIG_CMD_HISTORY_USE_CALLOC
	ಗಾಗಿ (i = 0; i < HIST_MAX; i++) {
		hist_list[i] = hist_ದತ್ತಾಂಶ[i];
		hist_list[i][0] = '\0';
	}
#ಇಲ್ಲದಿದ್ದರೆ
	ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *hist = ಸೊನ್ನೆ_ಸ್ಮರಣೆ_ಕೊಡು(HIST_MAX, HIST_ಗಾತ್ರ + 1);
	ಆದರೆ (!hist)
		panic("%s: calloc: out of memory!\n", __func__);

	ಗಾಗಿ (i = 0; i < HIST_MAX; i++)
		hist_list[i] = hist + (i * (HIST_ಗಾತ್ರ + 1));
#ಆದರೆ_ಅಂತ್ಯ

	hist_max = 0;
	hist_ಕೂಡಿಸು_idx = 0;
	hist_cur = -1;
	hist_ಅಂಕೆ = 0;

	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಶೂನ್ಯ cread_ಕೂಡಿಸು_ಗಮ್ಯ_hist(ಅಕ್ಷರ *ಸಾಲು)
{
	strcpy(hist_list[hist_ಕೂಡಿಸು_idx], ಸಾಲು);

	ಆದರೆ (++hist_ಕೂಡಿಸು_idx >= HIST_MAX)
		hist_ಕೂಡಿಸು_idx = 0;

	ಆದರೆ (hist_ಕೂಡಿಸು_idx > hist_max)
		hist_max = hist_ಕೂಡಿಸು_idx;

	hist_ಅಂಕೆ++;
}

ಸ್ಥಿರ ಅಕ್ಷರ *hist_prev(ಶೂನ್ಯ)
{
	ಅಕ್ಷರ *ret;
	ಪೂರ್ಣಾಂಕ old_cur;

	ಆದರೆ (hist_cur < 0)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	old_cur = hist_cur;
	ಆದರೆ (--hist_cur < 0)
		hist_cur = hist_max;

	ಆದರೆ (hist_cur == hist_ಕೂಡಿಸು_idx) {
		hist_cur = old_cur;
		ret = ಖಾಲಿ;
	} ಇಲ್ಲದಿದ್ದರೆ {
		ret = hist_list[hist_cur];
	}

	ಹಿಂತಿರುಗಿ ret;
}

ಸ್ಥಿರ ಅಕ್ಷರ *hist_ಮುಂದಿನ(ಶೂನ್ಯ)
{
	ಅಕ್ಷರ *ret;

	ಆದರೆ (hist_cur < 0)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	ಆದರೆ (hist_cur == hist_ಕೂಡಿಸು_idx)
		ಹಿಂತಿರುಗಿ ಖಾಲಿ;

	ಆದರೆ (++hist_cur > hist_max)
		hist_cur = 0;

	ಆದರೆ (hist_cur == hist_ಕೂಡಿಸು_idx)
		ret = "";
	ಇಲ್ಲದಿದ್ದರೆ
		ret = hist_list[hist_cur];

	ಹಿಂತಿರುಗಿ ret;
}

ಶೂನ್ಯ cread_print_hist_list(ಶೂನ್ಯ)
{
	ಪೂರ್ಣಾಂಕ i;
	uint n;

	n = hist_ಅಂಕೆ - hist_max;

	i = hist_ಕೂಡಿಸು_idx + 1;
	ಆಗ (1) {
		ಆದರೆ (i > hist_max)
			i = 0;
		ಆದರೆ (i == hist_ಕೂಡಿಸು_idx)
			ನಿಲ್ಲಿಸು;
		ಬರೆ("%s\n", hist_list[i]);
		n++;
		i++;
	}
}

#ವ್ಯಾಖ್ಯಾನಿಸು BEGINNING_ದ_ಸಾಲು() {			\
	ಆಗ (cls->ಅಂಕೆ) {			\
		getcmd_putch(CTL_BACKSPACE);	\
		cls->ಅಂಕೆ--;			\
	}					\
}

#ವ್ಯಾಖ್ಯಾನಿಸು ERASE_ಗಮ್ಯ_EOL() {				\
	ಆದರೆ (cls->ಅಂಕೆ < cls->eol_ಅಂಕೆ) {		\
		ಬರೆ("%*s", (ಪೂರ್ಣಾಂಕ)(cls->eol_ಅಂಕೆ - cls->ಅಂಕೆ), ""); \
		ಮಾಡು {					\
			getcmd_putch(CTL_BACKSPACE);	\
		} ಆಗ (--cls->eol_ಅಂಕೆ > cls->ಅಂಕೆ);	\
	}						\
}

#ವ್ಯಾಖ್ಯಾನಿಸು REFRESH_ಗಮ್ಯ_EOL() {				\
	ಆದರೆ (cls->ಅಂಕೆ < cls->eol_ಅಂಕೆ) {			\
		uint wlen = cls->eol_ಅಂಕೆ - cls->ಅಂಕೆ;	\
		putnstr(buf + cls->ಅಂಕೆ, wlen);		\
		cls->ಅಂಕೆ = cls->eol_ಅಂಕೆ;		\
	}						\
}

ಸ್ಥಿರ ಶೂನ್ಯ cread_ಕೂಡಿಸು_char(ಅಕ್ಷರ ichar, ಪೂರ್ಣಾಂಕ ಸೇರಿಸು, uint *ಅಂಕೆ,
			   uint *eol_ಅಂಕೆ, ಅಕ್ಷರ *buf, uint len)
{
	uint wlen;

	/* room ??? */
	ಆದರೆ (ಸೇರಿಸು || *ಅಂಕೆ == *eol_ಅಂಕೆ) {
		ಆದರೆ (*eol_ಅಂಕೆ > len - 1) {
			getcmd_cbeep();
			ಹಿಂತಿರುಗಿ;
		}
		(*eol_ಅಂಕೆ)++;
	}

	ಆದರೆ (ಸೇರಿಸು) {
		wlen = *eol_ಅಂಕೆ - *ಅಂಕೆ;
		ಆದರೆ (wlen > 1)
			memmove(&buf[*ಅಂಕೆ+1], &buf[*ಅಂಕೆ], wlen-1);

		buf[*ಅಂಕೆ] = ichar;
		putnstr(buf + *ಅಂಕೆ, wlen);
		(*ಅಂಕೆ)++;
		ಆಗ (--wlen)
			getcmd_putch(CTL_BACKSPACE);
	} ಇಲ್ಲದಿದ್ದರೆ {
		/* echo the character */
		wlen = 1;
		buf[*ಅಂಕೆ] = ichar;
		putnstr(buf + *ಅಂಕೆ, wlen);
		(*ಅಂಕೆ)++;
	}
}

ಸ್ಥಿರ ಶೂನ್ಯ cread_ಕೂಡಿಸು_str(ಅಕ್ಷರ *str, ಪೂರ್ಣಾಂಕ strsize, ಪೂರ್ಣಾಂಕ ಸೇರಿಸು,
			  uint *ಅಂಕೆ, uint *eol_ಅಂಕೆ, ಅಕ್ಷರ *buf, uint len)
{
	ಆಗ (strsize--) {
		cread_ಕೂಡಿಸು_char(*str, ಸೇರಿಸು, ಅಂಕೆ, eol_ಅಂಕೆ, buf, len);
		str++;
	}
}

ಪೂರ್ಣಾಂಕ cread_ಸಾಲು_process_ch(ರಚನೆ cli_ಸಾಲು_state *cls, ಅಕ್ಷರ ichar)
{
	ಅಕ್ಷರ *buf = cls->buf;

	/* ichar=0x0 when error occurs in U-Boot getc */
	ಆದರೆ (!ichar)
		ಹಿಂತಿರುಗಿ -EAGAIN;

	ಆದರೆ (ichar == '\n') {
		putc('\n');
		buf[cls->eol_ಅಂಕೆ] = '\0';	/* terminate the string */
		ಹಿಂತಿರುಗಿ 0;
	}

	ಆಯ್ಕೆ (ichar) {
	ಸ್ಥಿತಿ CTL_CH('a'):
		BEGINNING_ದ_ಸಾಲು();
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CTL_CH('c'):	/* ^C - break */
		*buf = '\0';	/* discard input */
		ಹಿಂತಿರುಗಿ -EINTR;
	ಸ್ಥಿತಿ CTL_CH('f'):
		ಆದರೆ (cls->ಅಂಕೆ < cls->eol_ಅಂಕೆ) {
			getcmd_putch(buf[cls->ಅಂಕೆ]);
			cls->ಅಂಕೆ++;
		}
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CTL_CH('b'):
		ಆದರೆ (cls->ಅಂಕೆ) {
			getcmd_putch(CTL_BACKSPACE);
			cls->ಅಂಕೆ--;
		}
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CTL_CH('d'):
		ಆದರೆ (cls->ಅಂಕೆ < cls->eol_ಅಂಕೆ) {
			uint wlen;

			wlen = cls->eol_ಅಂಕೆ - cls->ಅಂಕೆ - 1;
			ಆದರೆ (wlen) {
				memmove(&buf[cls->ಅಂಕೆ], &buf[cls->ಅಂಕೆ + 1],
					wlen);
				putnstr(buf + cls->ಅಂಕೆ, wlen);
			}

			getcmd_putch(' ');
			ಮಾಡು {
				getcmd_putch(CTL_BACKSPACE);
			} ಆಗ (wlen--);
			cls->eol_ಅಂಕೆ--;
		}
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CTL_CH('k'):
		ERASE_ಗಮ್ಯ_EOL();
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CTL_CH('e'):
		REFRESH_ಗಮ್ಯ_EOL();
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CTL_CH('o'):
		cls->ಸೇರಿಸು = !cls->ಸೇರಿಸು;
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CTL_CH('w'):
		ಆದರೆ (cls->ಅಂಕೆ) {
			uint ತಳ, wlen;

			ಗಾಗಿ (ತಳ = cls->ಅಂಕೆ;
			     ತಳ > 0 && buf[ತಳ - 1] == ' ';)
				ತಳ--;
			ಗಾಗಿ (; ತಳ > 0 && buf[ತಳ - 1] != ' ';)
				ತಳ--;

			/* now delete chars from base to cls->num */
			wlen = cls->ಅಂಕೆ - ತಳ;
			cls->eol_ಅಂಕೆ -= wlen;
			memmove(&buf[ತಳ], &buf[cls->ಅಂಕೆ],
				cls->eol_ಅಂಕೆ - ತಳ + 1);
			cls->ಅಂಕೆ = ತಳ;
			getcmd_putchars(wlen, CTL_BACKSPACE);
			puts(buf + ತಳ);
			getcmd_putchars(wlen, ' ');
			getcmd_putchars(wlen + cls->eol_ಅಂಕೆ - cls->ಅಂಕೆ,
					CTL_BACKSPACE);
		}
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CTL_CH('x'):
	ಸ್ಥಿತಿ CTL_CH('u'):
		BEGINNING_ದ_ಸಾಲು();
		ERASE_ಗಮ್ಯ_EOL();
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ DEL:
	ಸ್ಥಿತಿ DEL7:
	ಸ್ಥಿತಿ 8:
		ಆದರೆ (cls->ಅಂಕೆ) {
			uint wlen;

			wlen = cls->eol_ಅಂಕೆ - cls->ಅಂಕೆ;
			cls->ಅಂಕೆ--;
			memmove(&buf[cls->ಅಂಕೆ], &buf[cls->ಅಂಕೆ + 1], wlen);
			getcmd_putch(CTL_BACKSPACE);
			putnstr(buf + cls->ಅಂಕೆ, wlen);
			getcmd_putch(' ');
			ಮಾಡು {
				getcmd_putch(CTL_BACKSPACE);
			} ಆಗ (wlen--);
			cls->eol_ಅಂಕೆ--;
		}
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ CTL_CH('p'):
	ಸ್ಥಿತಿ CTL_CH('n'):
		ಆದರೆ (cls->history) {
			ಅಕ್ಷರ *hline;

			ಆದರೆ (ichar == CTL_CH('p'))
				hline = hist_prev();
			ಇಲ್ಲದಿದ್ದರೆ
				hline = hist_ಮುಂದಿನ();

			ಆದರೆ (!hline) {
				getcmd_cbeep();
				ನಿಲ್ಲಿಸು;
			}

			/* nuke the current line */
			/* first, go home */
			BEGINNING_ದ_ಸಾಲು();

			/* erase to end of line */
			ERASE_ಗಮ್ಯ_EOL();

			/* copy new line into place and display */
			strcpy(buf, hline);
			cls->eol_ಅಂಕೆ = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(buf);
			REFRESH_ಗಮ್ಯ_EOL();
			ನಿಲ್ಲಿಸು;
		}
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ '\t':
		ಆದರೆ (IS_ENABLED(CONFIG_AUTO_COMPLETE) && cls->cmd_complete) {
			ಪೂರ್ಣಾಂಕ num2, ಲಂಬಸಾಲು;

			/* do not autocomplete when in the middle */
			ಆದರೆ (cls->ಅಂಕೆ < cls->eol_ಅಂಕೆ) {
				getcmd_cbeep();
				ನಿಲ್ಲಿಸು;
			}

			buf[cls->ಅಂಕೆ] = '\0';
			ಲಂಬಸಾಲು = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(cls->prompt) + cls->eol_ಅಂಕೆ;
			num2 = cls->ಅಂಕೆ;
			ಆದರೆ (cmd_auto_complete(cls->prompt, buf, &num2, &ಲಂಬಸಾಲು)) {
				ಲಂಬಸಾಲು = num2 - cls->ಅಂಕೆ;
				cls->ಅಂಕೆ += ಲಂಬಸಾಲು;
				cls->eol_ಅಂಕೆ += ಲಂಬಸಾಲು;
			}
			ನಿಲ್ಲಿಸು;
		}
		fallthrough;
	ಪೂರ್ವನಿಯೋಜಿತ:
		cread_ಕೂಡಿಸು_char(ichar, cls->ಸೇರಿಸು, &cls->ಅಂಕೆ, &cls->eol_ಅಂಕೆ,
			       buf, cls->len);
		ನಿಲ್ಲಿಸು;
	}

	/*
	 * keep the string terminated...if we added a char at the end then we
	 * want a \0 after it
	 */
	buf[cls->eol_ಅಂಕೆ] = '\0';

	ಹಿಂತಿರುಗಿ -EAGAIN;
}

ಶೂನ್ಯ cli_cread_init(ರಚನೆ cli_ಸಾಲು_state *cls, ಅಕ್ಷರ *buf, uint buf_ಗಾತ್ರ)
{
	ಪೂರ್ಣಾಂಕ init_len = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(buf);

	memset(cls, '\0', ಗಾತ್ರ(ರಚನೆ cli_ಸಾಲು_state));
	cls->ಸೇರಿಸು = true;
	cls->buf = buf;
	cls->len = buf_ಗಾತ್ರ;

	ಆದರೆ (init_len)
		cread_ಕೂಡಿಸು_str(buf, init_len, 0, &cls->ಅಂಕೆ, &cls->eol_ಅಂಕೆ, buf,
			      buf_ಗಾತ್ರ);
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ cread_ಸಾಲು(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ prompt, ಅಕ್ಷರ *buf, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *len,
		      ಪೂರ್ಣಾಂಕ timeout)
{
	ರಚನೆ cli_ch_state s_cch, *cch = &s_cch;
	ರಚನೆ cli_ಸಾಲು_state s_cls, *cls = &s_cls;
	ಅಕ್ಷರ ichar;
	ಪೂರ್ಣಾಂಕ ಮೊದಲ = 1;

	cli_ch_init(cch);
	cli_cread_init(cls, buf, *len);
	cls->prompt = prompt;
	cls->history = true;
	cls->cmd_complete = true;

	ಆಗ (1) {
		ಪೂರ್ಣಾಂಕ ret;

		/* Check for saved characters */
		ichar = cli_ch_process(cch, 0);

		ಆದರೆ (!ichar) {
			ಆದರೆ (bootretry_tstc_timeout())
				ಹಿಂತಿರುಗಿ -2;	/* timed out */
			ಆದರೆ (ಮೊದಲ && timeout) {
				u64 etime = endtick(timeout);

				ಆಗ (!tstc()) {	/* while no incoming data */
					ಆದರೆ (get_ticks() >= etime)
						ಹಿಂತಿರುಗಿ -2;	/* timed out */
					ವೇಳಾಪಟ್ಟಿ();
				}
				ಮೊದಲ = 0;
			}

			ichar = getcmd_getch();
			ichar = cli_ch_process(cch, ichar);
		}

		ret = cread_ಸಾಲು_process_ch(cls, ichar);
		ಆದರೆ (ret == -EINTR)
			ಹಿಂತಿರುಗಿ -1;
		ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (!ret)
			ನಿಲ್ಲಿಸು;
	}
	*len = cls->eol_ಅಂಕೆ;

	ಆದರೆ (buf[0] && buf[0] != CREAD_HIST_CHAR)
		cread_ಕೂಡಿಸು_ಗಮ್ಯ_hist(buf);
	hist_cur = hist_ಕೂಡಿಸು_idx;

	ಹಿಂತಿರುಗಿ 0;
}

#ಇಲ್ಲದಿದ್ದರೆ /* !CONFIG_CMDLINE_EDITING */

ಸ್ಥಿರ ಸಾಲೊಳಗೆ ಪೂರ್ಣಾಂಕ hist_init(ಶೂನ್ಯ)
{
	ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ cread_ಸಾಲು(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ prompt, ಅಕ್ಷರ *buf, ಚಿಹ್ನೆರಹಿತ ಪೂರ್ಣಾಂಕ *len,
		      ಪೂರ್ಣಾಂಕ timeout)
{
	ಹಿಂತಿರುಗಿ 0;
}

#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_CMDLINE_EDITING */

/****************************************************************************/

ಪೂರ್ಣಾಂಕ cli_readline(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ prompt)
{
	/*
	 * If console_buffer isn't 0-length the user will be prompted to modify
	 * it instead of entering it from scratch as desired.
	 */
	console_ಬಫರ್[0] = '\0';

	ಹಿಂತಿರುಗಿ cli_readline_into_ಬಫರ್(prompt, console_ಬಫರ್, 0);
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
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ cread_ಸಾಲು_simple(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ prompt, ಅಕ್ಷರ *p)
{
	ಅಕ್ಷರ *p_buf = p;
	ಪೂರ್ಣಾಂಕ n = 0;		/* buffer index */
	ಪೂರ್ಣಾಂಕ plen = 0;		/* prompt length */
	ಪೂರ್ಣಾಂಕ ಲಂಬಸಾಲು;		/* output column cnt */
	ಪೂರ್ಣಾಂಕ c;

	/* print prompt */
	ಆದರೆ (prompt) {
		plen = ಅಕ್ಷರಸರಣಿ_ಉದ್ದ(prompt);
		puts(prompt);
	}
	ಲಂಬಸಾಲು = plen;

	ಗಾಗಿ (;;) {
		ಆದರೆ (bootretry_tstc_timeout())
			ಹಿಂತಿರುಗಿ -2;	/* timed out */
		ವೇಳಾಪಟ್ಟಿ();	/* Trigger watchdog, if needed */

		c = getchar();

		/*
		 * Special character handling
		 */
		ಆಯ್ಕೆ (c) {
		ಸ್ಥಿತಿ '\r':			/* Enter		*/
		ಸ್ಥಿತಿ '\n':
			*p = '\0';
			puts("\r\n");
			ಹಿಂತಿರುಗಿ p - p_buf;

		ಸ್ಥಿತಿ '\0':			/* nul			*/
			ಮುಂದುವರಿ;

		ಸ್ಥಿತಿ 0x03:			/* ^C - break		*/
			p_buf[0] = '\0';	/* discard input */
			ಹಿಂತಿರುಗಿ -1;

		ಸ್ಥಿತಿ 0x15:			/* ^U - erase line	*/
			ಆಗ (ಲಂಬಸಾಲು > plen) {
				puts(erase_seq);
				--ಲಂಬಸಾಲು;
			}
			p = p_buf;
			n = 0;
			ಮುಂದುವರಿ;

		ಸ್ಥಿತಿ 0x17:			/* ^W - erase word	*/
			p = delete_char(p_buf, p, &ಲಂಬಸಾಲು, &n, plen);
			ಆಗ ((n > 0) && (*p != ' '))
				p = delete_char(p_buf, p, &ಲಂಬಸಾಲು, &n, plen);
			ಮುಂದುವರಿ;

		ಸ್ಥಿತಿ 0x08:			/* ^H  - backspace	*/
		ಸ್ಥಿತಿ 0x7F:			/* DEL - backspace	*/
			p = delete_char(p_buf, p, &ಲಂಬಸಾಲು, &n, plen);
			ಮುಂದುವರಿ;

		ಪೂರ್ವನಿಯೋಜಿತ:
			/* Must be a normal character then */
			ಆದರೆ (n >= CONFIG_SYS_CBSIZE - 2) { /* Buffer full */
				putc('\a');
				ನಿಲ್ಲಿಸು;
			}
			ಆದರೆ (c == '\t') {	/* expand TABs */
				ಆದರೆ (IS_ENABLED(CONFIG_AUTO_COMPLETE)) {
					/*
					 * if auto-completion triggered just
					 * continue
					 */
					*p = '\0';
					ಆದರೆ (cmd_auto_complete(prompt,
							      console_ಬಫರ್,
							      &n, &ಲಂಬಸಾಲು)) {
						p = p_buf + n;	/* reset */
						ಮುಂದುವರಿ;
					}
				}
				puts(tab_seq + (ಲಂಬಸಾಲು & 07));
				ಲಂಬಸಾಲು += 8 - (ಲಂಬಸಾಲು & 07);
			} ಇಲ್ಲದಿದ್ದರೆ {
				ಅಕ್ಷರ __maybe_unused buf[2];

				/*
				 * Echo input using puts() to force an LCD
				 * flush if we are using an LCD
				 */
				++ಲಂಬಸಾಲು;
				buf[0] = c;
				buf[1] = '\0';
				puts(buf);
			}
			*p++ = c;
			++n;
			ನಿಲ್ಲಿಸು;
		}
	}
}

ಪೂರ್ಣಾಂಕ cli_readline_into_ಬಫರ್(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *ಸ್ಥಿರಾಂಕ prompt, ಅಕ್ಷರ *ಬಫರ್,
			     ಪೂರ್ಣಾಂಕ timeout)
{
	ಅಕ್ಷರ *p = ಬಫರ್;
	uint len = CONFIG_SYS_CBSIZE;
	ಪೂರ್ಣಾಂಕ rc;
	ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ initted;

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
	ಆದರೆ (IS_ENABLED(CONFIG_CMDLINE_EDITING) && (gd->flags & GD_FLG_RELOC)) {
		ಆದರೆ (!initted) {
			rc = hist_init();
			ಆದರೆ (rc == 0)
				initted = 1;
		}

		ಆದರೆ (prompt)
			puts(prompt);

		rc = cread_ಸಾಲು(prompt, p, &len, timeout);
		ಹಿಂತಿರುಗಿ rc < 0 ? rc : len;

	} ಇಲ್ಲದಿದ್ದರೆ {
		ಹಿಂತಿರುಗಿ cread_ಸಾಲು_simple(prompt, p);
	}
}
