// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright 2022 Google LLC
 */

#ಅನ್ತರ್ಭಾವ <cli.h>
#ಅನ್ತರ್ಭಾವ <stdio.h>
#ಅನ್ತರ್ಭಾವ <string.h>
#ಅನ್ತರ್ಭಾವ <linux/errno.h>

/**
 * enum cli_esc_state_t - indicates what to do with an escape character
 *
 * @ESC_REJECT: Invalid escape sequence, so the esc_save[] characters are
 *	returned from each subsequent call to cli_ch_esc()
 * @ESC_SAVE: Character should be saved in esc_save until we have another one
 * @ESC_CONVERTED: Escape sequence has been completed and the resulting
 *	character is available
 */
ಗಣನಾ cli_esc_state_t {
	ESC_REJECT,
	ESC_SAVE,
	ESC_CONVERTED
};

ಶೂನ್ಯ cli_ch_init(ಸಂರಚನಾ cli_ch_state *cch)
{
	memset(cch, '\0', ಪರಿಮಾಣ(*cch));
}

/**
 * cli_ch_esc() - Process a character in an ongoing escape sequence
 *
 * @cch: State information
 * @ichar: Character to process
 * @actp: Returns the action to take
 * Returns: Output character if *actp is ESC_CONVERTED, else 0
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ cli_ch_esc(ಸಂರಚನಾ cli_ch_state *cch, ಪೂರ್ಣಾಙ್ಕ ichar,
		      ಗಣನಾ cli_esc_state_t *actp)
{
	ಗಣನಾ cli_esc_state_t act = ESC_REJECT;

	ವಿಕಲ್ಪ (cch->esc_len) {
	ಪಕ್ಷ 1:
		ಯದಿ (ichar == '[' || ichar == 'O')
			act = ESC_SAVE;
		ಅನ್ಯಥಾ
			act = ESC_CONVERTED;
		ಭಙ್ಗ;
	ಪಕ್ಷ 2:
		ವಿಕಲ್ಪ (ichar) {
		ಪಕ್ಷ 'D':	/* <- key */
			ichar = CTL_CH('b');
			act = ESC_CONVERTED;
			ಭಙ್ಗ;	/* pass off to ^B handler */
		ಪಕ್ಷ 'C':	/* -> key */
			ichar = CTL_CH('f');
			act = ESC_CONVERTED;
			ಭಙ್ಗ;	/* pass off to ^F handler */
		ಪಕ್ಷ 'H':	/* Home key */
			ichar = CTL_CH('a');
			act = ESC_CONVERTED;
			ಭಙ್ಗ;	/* pass off to ^A handler */
		ಪಕ್ಷ 'F':	/* End key */
			ichar = CTL_CH('e');
			act = ESC_CONVERTED;
			ಭಙ್ಗ;	/* pass off to ^E handler */
		ಪಕ್ಷ 'A':	/* up arrow */
			ichar = CTL_CH('p');
			act = ESC_CONVERTED;
			ಭಙ್ಗ;	/* pass off to ^P handler */
		ಪಕ್ಷ 'B':	/* down arrow */
			ichar = CTL_CH('n');
			act = ESC_CONVERTED;
			ಭಙ್ಗ;	/* pass off to ^N handler */
		ಪಕ್ಷ '1':
		ಪಕ್ಷ '2':
		ಪಕ್ಷ '3':
		ಪಕ್ಷ '4':
		ಪಕ್ಷ '7':
		ಪಕ್ಷ '8':
			ಯದಿ (cch->esc_save[1] == '[') {
				/* see if next character is ~ */
				act = ESC_SAVE;
			}
			ಭಙ್ಗ;
		}
		ಭಙ್ಗ;
	ಪಕ್ಷ 3:
		ವಿಕಲ್ಪ (ichar) {
		ಪಕ್ಷ '~':
			ವಿಕಲ್ಪ (cch->esc_save[2]) {
			ಪಕ್ಷ '3':	/* Delete key */
				ichar = CTL_CH('d');
				act = ESC_CONVERTED;
				ಭಙ್ಗ;	/* pass to ^D handler */
			ಪಕ್ಷ '1':	/* Home key */
			ಪಕ್ಷ '7':
				ichar = CTL_CH('a');
				act = ESC_CONVERTED;
				ಭಙ್ಗ;	/* pass to ^A handler */
			ಪಕ್ಷ '4':	/* End key */
			ಪಕ್ಷ '8':
				ichar = CTL_CH('e');
				act = ESC_CONVERTED;
				ಭಙ್ಗ;	/* pass to ^E handler */
			}
			ಭಙ್ಗ;
		ಪಕ್ಷ '0':
			ಯದಿ (cch->esc_save[2] == '2')
				act = ESC_SAVE;
			ಭಙ್ಗ;
		}
		ಭಙ್ಗ;
	ಪಕ್ಷ 4:
		ವಿಕಲ್ಪ (ichar) {
		ಪಕ್ಷ '0':
		ಪಕ್ಷ '1':
			act = ESC_SAVE;
			ಭಙ್ಗ;		/* bracketed paste */
		}
		ಭಙ್ಗ;
	ಪಕ್ಷ 5:
		ಯದಿ (ichar == '~') {	/* bracketed paste */
			ichar = 0;
			act = ESC_CONVERTED;
		}
	}

	*actp = act;

	ಪ್ರತ್ಯಾಗಮ ichar;
}

ಪೂರ್ಣಾಙ್ಕ cli_ch_process(ಸಂರಚನಾ cli_ch_state *cch, ಪೂರ್ಣಾಙ್ಕ ichar)
{
	/*
	 * ichar=0x0 when error occurs in U-Boot getchar() or when the caller
	 * wants to check if there are more characters saved in the escape
	 * sequence
	 */
	ಯದಿ (!ichar) {
		ಯದಿ (cch->emitting) {
			ಯದಿ (cch->emit_upto < cch->esc_len)
				ಪ್ರತ್ಯಾಗಮ cch->esc_save[cch->emit_upto++];
			cch->emit_upto = 0;
			cch->emitting = false;
			cch->esc_len = 0;
		}
		ಪ್ರತ್ಯಾಗಮ 0;
	} ಅನ್ಯಥಾ ಯದಿ (ichar == -ETIMEDOUT) {
		/*
		 * If we are in an escape sequence but nothing has followed the
		 * Escape character, then the user probably just pressed the
		 * Escape key. Return it and clear the sequence.
		 */
		ಯದಿ (cch->esc_len) {
			cch->esc_len = 0;
			ಪ್ರತ್ಯಾಗಮ '\e';
		}

		/* Otherwise there is nothing to return */
		ಪ್ರತ್ಯಾಗಮ 0;
	}

	ಯದಿ (ichar == '\n' || ichar == '\r')
		ಪ್ರತ್ಯಾಗಮ '\n';

	/* handle standard linux xterm esc sequences for arrow key, etc. */
	ಯದಿ (cch->esc_len != 0) {
		ಗಣನಾ cli_esc_state_t act;

		ichar = cli_ch_esc(cch, ichar, &act);

		ವಿಕಲ್ಪ (act) {
		ಪಕ್ಷ ESC_SAVE:
			/* save this character and return nothing */
			cch->esc_save[cch->esc_len++] = ichar;
			ichar = 0;
			ಭಙ್ಗ;
		ಪಕ್ಷ ESC_REJECT:
			/*
			 * invalid escape sequence, start returning the
			 * characters in it
			 */
			cch->esc_save[cch->esc_len++] = ichar;
			ichar = cch->esc_save[cch->emit_upto++];
			cch->emitting = true;
			ಪ್ರತ್ಯಾಗಮ ichar;
		ಪಕ್ಷ ESC_CONVERTED:
			/* valid escape sequence, return the resulting char */
			cch->esc_len = 0;
			ಭಙ್ಗ;
		}
	}

	ಯದಿ (ichar == '\e') {
		ಯದಿ (!cch->esc_len) {
			cch->esc_save[cch->esc_len] = ichar;
			cch->esc_len = 1;
		} ಅನ್ಯಥಾ {
			puts("impossible condition #876\n");
			cch->esc_len = 0;
		}
		ಪ್ರತ್ಯಾಗಮ 0;
	}

	ಪ್ರತ್ಯಾಗಮ ichar;
}
