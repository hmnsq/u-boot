// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright 2022 Google LLC
 */

#অন্তর্ভাৱ <cli.h>
#অন্তর্ভাৱ <stdio.h>
#অন্তর্ভাৱ <string.h>
#অন্তর্ভাৱ <linux/errno.h>

/**
 * enum cli_esc_state_t - indicates what to do with an escape character
 *
 * @ESC_REJECT: Invalid escape sequence, so the esc_save[] characters are
 *	returned from each subsequent call to cli_ch_esc()
 * @ESC_SAVE: Character should be saved in esc_save until we have another one
 * @ESC_CONVERTED: Escape sequence has been completed and the resulting
 *	character is available
 */
গণনা cli_esc_state_t {
	ESC_REJECT,
	ESC_SAVE,
	ESC_CONVERTED
};

শূন্য cli_ch_init(সংরচনা cli_ch_state *cch)
{
	memset(cch, '\0', পরিমাণ(*cch));
}

/**
 * cli_ch_esc() - Process a character in an ongoing escape sequence
 *
 * @cch: State information
 * @ichar: Character to process
 * @actp: Returns the action to take
 * Returns: Output character if *actp is ESC_CONVERTED, else 0
 */
স্থির পূর্ণাঙ্ক cli_ch_esc(সংরচনা cli_ch_state *cch, পূর্ণাঙ্ক ichar,
		      গণনা cli_esc_state_t *actp)
{
	গণনা cli_esc_state_t act = ESC_REJECT;

	ৱিকল্প (cch->esc_len) {
	পক্ষ 1:
		যদি (ichar == '[' || ichar == 'O')
			act = ESC_SAVE;
		অন্যথা
			act = ESC_CONVERTED;
		ভঙ্গ;
	পক্ষ 2:
		ৱিকল্প (ichar) {
		পক্ষ 'D':	/* <- key */
			ichar = CTL_CH('b');
			act = ESC_CONVERTED;
			ভঙ্গ;	/* pass off to ^B handler */
		পক্ষ 'C':	/* -> key */
			ichar = CTL_CH('f');
			act = ESC_CONVERTED;
			ভঙ্গ;	/* pass off to ^F handler */
		পক্ষ 'H':	/* Home key */
			ichar = CTL_CH('a');
			act = ESC_CONVERTED;
			ভঙ্গ;	/* pass off to ^A handler */
		পক্ষ 'F':	/* End key */
			ichar = CTL_CH('e');
			act = ESC_CONVERTED;
			ভঙ্গ;	/* pass off to ^E handler */
		পক্ষ 'A':	/* up arrow */
			ichar = CTL_CH('p');
			act = ESC_CONVERTED;
			ভঙ্গ;	/* pass off to ^P handler */
		পক্ষ 'B':	/* down arrow */
			ichar = CTL_CH('n');
			act = ESC_CONVERTED;
			ভঙ্গ;	/* pass off to ^N handler */
		পক্ষ '1':
		পক্ষ '2':
		পক্ষ '3':
		পক্ষ '4':
		পক্ষ '7':
		পক্ষ '8':
			যদি (cch->esc_save[1] == '[') {
				/* see if next character is ~ */
				act = ESC_SAVE;
			}
			ভঙ্গ;
		}
		ভঙ্গ;
	পক্ষ 3:
		ৱিকল্প (ichar) {
		পক্ষ '~':
			ৱিকল্প (cch->esc_save[2]) {
			পক্ষ '3':	/* Delete key */
				ichar = CTL_CH('d');
				act = ESC_CONVERTED;
				ভঙ্গ;	/* pass to ^D handler */
			পক্ষ '1':	/* Home key */
			পক্ষ '7':
				ichar = CTL_CH('a');
				act = ESC_CONVERTED;
				ভঙ্গ;	/* pass to ^A handler */
			পক্ষ '4':	/* End key */
			পক্ষ '8':
				ichar = CTL_CH('e');
				act = ESC_CONVERTED;
				ভঙ্গ;	/* pass to ^E handler */
			}
			ভঙ্গ;
		পক্ষ '0':
			যদি (cch->esc_save[2] == '2')
				act = ESC_SAVE;
			ভঙ্গ;
		}
		ভঙ্গ;
	পক্ষ 4:
		ৱিকল্প (ichar) {
		পক্ষ '0':
		পক্ষ '1':
			act = ESC_SAVE;
			ভঙ্গ;		/* bracketed paste */
		}
		ভঙ্গ;
	পক্ষ 5:
		যদি (ichar == '~') {	/* bracketed paste */
			ichar = 0;
			act = ESC_CONVERTED;
		}
	}

	*actp = act;

	ঘূৰাই ichar;
}

পূর্ণাঙ্ক cli_ch_process(সংরচনা cli_ch_state *cch, পূর্ণাঙ্ক ichar)
{
	/*
	 * ichar=0x0 when error occurs in U-Boot getchar() or when the caller
	 * wants to check if there are more characters saved in the escape
	 * sequence
	 */
	যদি (!ichar) {
		যদি (cch->emitting) {
			যদি (cch->emit_upto < cch->esc_len)
				ঘূৰাই cch->esc_save[cch->emit_upto++];
			cch->emit_upto = 0;
			cch->emitting = false;
			cch->esc_len = 0;
		}
		ঘূৰাই 0;
	} অন্যথা যদি (ichar == -ETIMEDOUT) {
		/*
		 * If we are in an escape sequence but nothing has followed the
		 * Escape character, then the user probably just pressed the
		 * Escape key. Return it and clear the sequence.
		 */
		যদি (cch->esc_len) {
			cch->esc_len = 0;
			ঘূৰাই '\e';
		}

		/* Otherwise there is nothing to return */
		ঘূৰাই 0;
	}

	যদি (ichar == '\n' || ichar == '\r')
		ঘূৰাই '\n';

	/* handle standard linux xterm esc sequences for arrow key, etc. */
	যদি (cch->esc_len != 0) {
		গণনা cli_esc_state_t act;

		ichar = cli_ch_esc(cch, ichar, &act);

		ৱিকল্প (act) {
		পক্ষ ESC_SAVE:
			/* save this character and return nothing */
			cch->esc_save[cch->esc_len++] = ichar;
			ichar = 0;
			ভঙ্গ;
		পক্ষ ESC_REJECT:
			/*
			 * invalid escape sequence, start returning the
			 * characters in it
			 */
			cch->esc_save[cch->esc_len++] = ichar;
			ichar = cch->esc_save[cch->emit_upto++];
			cch->emitting = true;
			ঘূৰাই ichar;
		পক্ষ ESC_CONVERTED:
			/* valid escape sequence, return the resulting char */
			cch->esc_len = 0;
			ভঙ্গ;
		}
	}

	যদি (ichar == '\e') {
		যদি (!cch->esc_len) {
			cch->esc_save[cch->esc_len] = ichar;
			cch->esc_len = 1;
		} অন্যথা {
			puts("impossible condition #876\n");
			cch->esc_len = 0;
		}
		ঘূৰাই 0;
	}

	ঘূৰাই ichar;
}
