// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright 2022 Google LLC
 */

#અન્તર્ભાવ <cli.h>
#અન્તર્ભાવ <stdio.h>
#અન્તર્ભાવ <string.h>
#અન્તર્ભાવ <linux/errno.h>

/**
 * enum cli_esc_state_t - indicates what to do with an escape character
 *
 * @ESC_REJECT: Invalid escape sequence, so the esc_save[] characters are
 *	returned from each subsequent call to cli_ch_esc()
 * @ESC_SAVE: Character should be saved in esc_save until we have another one
 * @ESC_CONVERTED: Escape sequence has been completed and the resulting
 *	character is available
 */
ગણના cli_esc_state_t {
	ESC_REJECT,
	ESC_SAVE,
	ESC_CONVERTED
};

શૂન્ય cli_ch_init(સંરચના cli_ch_state *cch)
{
	memset(cch, '\0', કદ(*cch));
}

/**
 * cli_ch_esc() - Process a character in an ongoing escape sequence
 *
 * @cch: State information
 * @ichar: Character to process
 * @actp: Returns the action to take
 * Returns: Output character if *actp is ESC_CONVERTED, else 0
 */
સ્થિર પૂર્ણાંક cli_ch_esc(સંરચના cli_ch_state *cch, પૂર્ણાંક ichar,
		      ગણના cli_esc_state_t *actp)
{
	ગણના cli_esc_state_t act = ESC_REJECT;

	પસંદગી (cch->esc_len) {
	સ્થિતિ 1:
		જો (ichar == '[' || ichar == 'O')
			act = ESC_SAVE;
		નહીંતર
			act = ESC_CONVERTED;
		તોડો;
	સ્થિતિ 2:
		પસંદગી (ichar) {
		સ્થિતિ 'D':	/* <- key */
			ichar = CTL_CH('b');
			act = ESC_CONVERTED;
			તોડો;	/* pass off to ^B handler */
		સ્થિતિ 'C':	/* -> key */
			ichar = CTL_CH('f');
			act = ESC_CONVERTED;
			તોડો;	/* pass off to ^F handler */
		સ્થિતિ 'H':	/* Home key */
			ichar = CTL_CH('a');
			act = ESC_CONVERTED;
			તોડો;	/* pass off to ^A handler */
		સ્થિતિ 'F':	/* End key */
			ichar = CTL_CH('e');
			act = ESC_CONVERTED;
			તોડો;	/* pass off to ^E handler */
		સ્થિતિ 'A':	/* up arrow */
			ichar = CTL_CH('p');
			act = ESC_CONVERTED;
			તોડો;	/* pass off to ^P handler */
		સ્થિતિ 'B':	/* down arrow */
			ichar = CTL_CH('n');
			act = ESC_CONVERTED;
			તોડો;	/* pass off to ^N handler */
		સ્થિતિ '1':
		સ્થિતિ '2':
		સ્થિતિ '3':
		સ્થિતિ '4':
		સ્થિતિ '7':
		સ્થિતિ '8':
			જો (cch->esc_save[1] == '[') {
				/* see if next character is ~ */
				act = ESC_SAVE;
			}
			તોડો;
		}
		તોડો;
	સ્થિતિ 3:
		પસંદગી (ichar) {
		સ્થિતિ '~':
			પસંદગી (cch->esc_save[2]) {
			સ્થિતિ '3':	/* Delete key */
				ichar = CTL_CH('d');
				act = ESC_CONVERTED;
				તોડો;	/* pass to ^D handler */
			સ્થિતિ '1':	/* Home key */
			સ્થિતિ '7':
				ichar = CTL_CH('a');
				act = ESC_CONVERTED;
				તોડો;	/* pass to ^A handler */
			સ્થિતિ '4':	/* End key */
			સ્થિતિ '8':
				ichar = CTL_CH('e');
				act = ESC_CONVERTED;
				તોડો;	/* pass to ^E handler */
			}
			તોડો;
		સ્થિતિ '0':
			જો (cch->esc_save[2] == '2')
				act = ESC_SAVE;
			તોડો;
		}
		તોડો;
	સ્થિતિ 4:
		પસંદગી (ichar) {
		સ્થિતિ '0':
		સ્થિતિ '1':
			act = ESC_SAVE;
			તોડો;		/* bracketed paste */
		}
		તોડો;
	સ્થિતિ 5:
		જો (ichar == '~') {	/* bracketed paste */
			ichar = 0;
			act = ESC_CONVERTED;
		}
	}

	*actp = act;

	પાછા ichar;
}

પૂર્ણાંક cli_ch_process(સંરચના cli_ch_state *cch, પૂર્ણાંક ichar)
{
	/*
	 * ichar=0x0 when error occurs in U-Boot getchar() or when the caller
	 * wants to check if there are more characters saved in the escape
	 * sequence
	 */
	જો (!ichar) {
		જો (cch->emitting) {
			જો (cch->emit_upto < cch->esc_len)
				પાછા cch->esc_save[cch->emit_upto++];
			cch->emit_upto = 0;
			cch->emitting = false;
			cch->esc_len = 0;
		}
		પાછા 0;
	} નહીંતર જો (ichar == -ETIMEDOUT) {
		/*
		 * If we are in an escape sequence but nothing has followed the
		 * Escape character, then the user probably just pressed the
		 * Escape key. Return it and clear the sequence.
		 */
		જો (cch->esc_len) {
			cch->esc_len = 0;
			પાછા '\e';
		}

		/* Otherwise there is nothing to return */
		પાછા 0;
	}

	જો (ichar == '\n' || ichar == '\r')
		પાછા '\n';

	/* handle standard linux xterm esc sequences for arrow key, etc. */
	જો (cch->esc_len != 0) {
		ગણના cli_esc_state_t act;

		ichar = cli_ch_esc(cch, ichar, &act);

		પસંદગી (act) {
		સ્થિતિ ESC_SAVE:
			/* save this character and return nothing */
			cch->esc_save[cch->esc_len++] = ichar;
			ichar = 0;
			તોડો;
		સ્થિતિ ESC_REJECT:
			/*
			 * invalid escape sequence, start returning the
			 * characters in it
			 */
			cch->esc_save[cch->esc_len++] = ichar;
			ichar = cch->esc_save[cch->emit_upto++];
			cch->emitting = true;
			પાછા ichar;
		સ્થિતિ ESC_CONVERTED:
			/* valid escape sequence, return the resulting char */
			cch->esc_len = 0;
			તોડો;
		}
	}

	જો (ichar == '\e') {
		જો (!cch->esc_len) {
			cch->esc_save[cch->esc_len] = ichar;
			cch->esc_len = 1;
		} નહીંતર {
			puts("impossible condition #876\n");
			cch->esc_len = 0;
		}
		પાછા 0;
	}

	પાછા ichar;
}
