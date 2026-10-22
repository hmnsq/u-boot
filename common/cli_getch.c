// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright 2022 Google LLC
 */

#అన్తర్భావ <cli.h>
#అన్తర్భావ <stdio.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <linux/errno.h>

/**
 * enum cli_esc_state_t - indicates what to do with an escape character
 *
 * @ESC_REJECT: Invalid escape sequence, so the esc_save[] characters are
 *	returned from each subsequent call to cli_ch_esc()
 * @ESC_SAVE: Character should be saved in esc_save until we have another one
 * @ESC_CONVERTED: Escape sequence has been completed and the resulting
 *	character is available
 */
గణన cli_esc_state_t {
	ESC_REJECT,
	ESC_SAVE,
	ESC_CONVERTED
};

శూన్య cli_ch_init(నిర్మాణం cli_ch_state *cch)
{
	memset(cch, '\0', పరిమాణం(*cch));
}

/**
 * cli_ch_esc() - Process a character in an ongoing escape sequence
 *
 * @cch: State information
 * @ichar: Character to process
 * @actp: Returns the action to take
 * Returns: Output character if *actp is ESC_CONVERTED, else 0
 */
స్థిర పూర్ణాంకం cli_ch_esc(నిర్మాణం cli_ch_state *cch, పూర్ణాంకం ichar,
		      గణన cli_esc_state_t *actp)
{
	గణన cli_esc_state_t act = ESC_REJECT;

	ఎంపిక (cch->esc_len) {
	స్థితి 1:
		ఒకవేళ (ichar == '[' || ichar == 'O')
			act = ESC_SAVE;
		లేకపోతే
			act = ESC_CONVERTED;
		ఆపు;
	స్థితి 2:
		ఎంపిక (ichar) {
		స్థితి 'D':	/* <- key */
			ichar = CTL_CH('b');
			act = ESC_CONVERTED;
			ఆపు;	/* pass off to ^B handler */
		స్థితి 'C':	/* -> key */
			ichar = CTL_CH('f');
			act = ESC_CONVERTED;
			ఆపు;	/* pass off to ^F handler */
		స్థితి 'H':	/* Home key */
			ichar = CTL_CH('a');
			act = ESC_CONVERTED;
			ఆపు;	/* pass off to ^A handler */
		స్థితి 'F':	/* End key */
			ichar = CTL_CH('e');
			act = ESC_CONVERTED;
			ఆపు;	/* pass off to ^E handler */
		స్థితి 'A':	/* up arrow */
			ichar = CTL_CH('p');
			act = ESC_CONVERTED;
			ఆపు;	/* pass off to ^P handler */
		స్థితి 'B':	/* down arrow */
			ichar = CTL_CH('n');
			act = ESC_CONVERTED;
			ఆపు;	/* pass off to ^N handler */
		స్థితి '1':
		స్థితి '2':
		స్థితి '3':
		స్థితి '4':
		స్థితి '7':
		స్థితి '8':
			ఒకవేళ (cch->esc_save[1] == '[') {
				/* see if next character is ~ */
				act = ESC_SAVE;
			}
			ఆపు;
		}
		ఆపు;
	స్థితి 3:
		ఎంపిక (ichar) {
		స్థితి '~':
			ఎంపిక (cch->esc_save[2]) {
			స్థితి '3':	/* Delete key */
				ichar = CTL_CH('d');
				act = ESC_CONVERTED;
				ఆపు;	/* pass to ^D handler */
			స్థితి '1':	/* Home key */
			స్థితి '7':
				ichar = CTL_CH('a');
				act = ESC_CONVERTED;
				ఆపు;	/* pass to ^A handler */
			స్థితి '4':	/* End key */
			స్థితి '8':
				ichar = CTL_CH('e');
				act = ESC_CONVERTED;
				ఆపు;	/* pass to ^E handler */
			}
			ఆపు;
		స్థితి '0':
			ఒకవేళ (cch->esc_save[2] == '2')
				act = ESC_SAVE;
			ఆపు;
		}
		ఆపు;
	స్థితి 4:
		ఎంపిక (ichar) {
		స్థితి '0':
		స్థితి '1':
			act = ESC_SAVE;
			ఆపు;		/* bracketed paste */
		}
		ఆపు;
	స్థితి 5:
		ఒకవేళ (ichar == '~') {	/* bracketed paste */
			ichar = 0;
			act = ESC_CONVERTED;
		}
	}

	*actp = act;

	తిరిగి ichar;
}

పూర్ణాంకం cli_ch_process(నిర్మాణం cli_ch_state *cch, పూర్ణాంకం ichar)
{
	/*
	 * ichar=0x0 when error occurs in U-Boot getchar() or when the caller
	 * wants to check if there are more characters saved in the escape
	 * sequence
	 */
	ఒకవేళ (!ichar) {
		ఒకవేళ (cch->emitting) {
			ఒకవేళ (cch->emit_upto < cch->esc_len)
				తిరిగి cch->esc_save[cch->emit_upto++];
			cch->emit_upto = 0;
			cch->emitting = false;
			cch->esc_len = 0;
		}
		తిరిగి 0;
	} లేకపోతే ఒకవేళ (ichar == -ETIMEDOUT) {
		/*
		 * If we are in an escape sequence but nothing has followed the
		 * Escape character, then the user probably just pressed the
		 * Escape key. Return it and clear the sequence.
		 */
		ఒకవేళ (cch->esc_len) {
			cch->esc_len = 0;
			తిరిగి '\e';
		}

		/* Otherwise there is nothing to return */
		తిరిగి 0;
	}

	ఒకవేళ (ichar == '\n' || ichar == '\r')
		తిరిగి '\n';

	/* handle standard linux xterm esc sequences for arrow key, etc. */
	ఒకవేళ (cch->esc_len != 0) {
		గణన cli_esc_state_t act;

		ichar = cli_ch_esc(cch, ichar, &act);

		ఎంపిక (act) {
		స్థితి ESC_SAVE:
			/* save this character and return nothing */
			cch->esc_save[cch->esc_len++] = ichar;
			ichar = 0;
			ఆపు;
		స్థితి ESC_REJECT:
			/*
			 * invalid escape sequence, start returning the
			 * characters in it
			 */
			cch->esc_save[cch->esc_len++] = ichar;
			ichar = cch->esc_save[cch->emit_upto++];
			cch->emitting = true;
			తిరిగి ichar;
		స్థితి ESC_CONVERTED:
			/* valid escape sequence, return the resulting char */
			cch->esc_len = 0;
			ఆపు;
		}
	}

	ఒకవేళ (ichar == '\e') {
		ఒకవేళ (!cch->esc_len) {
			cch->esc_save[cch->esc_len] = ichar;
			cch->esc_len = 1;
		} లేకపోతే {
			puts("impossible condition #876\n");
			cch->esc_len = 0;
		}
		తిరిగి 0;
	}

	తిరిగి ichar;
}
