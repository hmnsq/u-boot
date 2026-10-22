// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright 2022 Google LLC
 */

#অন্তর্ভাব <cli.h>
#অন্তর্ভাব <stdio.h>
#অন্তর্ভাব <string.h>
#অন্তর্ভাব <linux/errno.h>

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

শূন্য cli_ch_init(কাঠামো cli_ch_state *cch)
{
	memset(cch, '\0', আকার(*cch));
}

/**
 * cli_ch_esc() - Process a character in an ongoing escape sequence
 *
 * @cch: State information
 * @ichar: Character to process
 * @actp: Returns the action to take
 * Returns: Output character if *actp is ESC_CONVERTED, else 0
 */
স্থির পূর্ণসংখ্যা cli_ch_esc(কাঠামো cli_ch_state *cch, পূর্ণসংখ্যা ichar,
		      গণনা cli_esc_state_t *actp)
{
	গণনা cli_esc_state_t act = ESC_REJECT;

	নির্বাচন (cch->esc_len) {
	ক্ষেত্র 1:
		যদি (ichar == '[' || ichar == 'O')
			act = ESC_SAVE;
		অন্যথা
			act = ESC_CONVERTED;
		বিরতি;
	ক্ষেত্র 2:
		নির্বাচন (ichar) {
		ক্ষেত্র 'D':	/* <- key */
			ichar = CTL_CH('b');
			act = ESC_CONVERTED;
			বিরতি;	/* pass off to ^B handler */
		ক্ষেত্র 'C':	/* -> key */
			ichar = CTL_CH('f');
			act = ESC_CONVERTED;
			বিরতি;	/* pass off to ^F handler */
		ক্ষেত্র 'H':	/* Home key */
			ichar = CTL_CH('a');
			act = ESC_CONVERTED;
			বিরতি;	/* pass off to ^A handler */
		ক্ষেত্র 'F':	/* End key */
			ichar = CTL_CH('e');
			act = ESC_CONVERTED;
			বিরতি;	/* pass off to ^E handler */
		ক্ষেত্র 'A':	/* up arrow */
			ichar = CTL_CH('p');
			act = ESC_CONVERTED;
			বিরতি;	/* pass off to ^P handler */
		ক্ষেত্র 'B':	/* down arrow */
			ichar = CTL_CH('n');
			act = ESC_CONVERTED;
			বিরতি;	/* pass off to ^N handler */
		ক্ষেত্র '1':
		ক্ষেত্র '2':
		ক্ষেত্র '3':
		ক্ষেত্র '4':
		ক্ষেত্র '7':
		ক্ষেত্র '8':
			যদি (cch->esc_save[1] == '[') {
				/* see if next character is ~ */
				act = ESC_SAVE;
			}
			বিরতি;
		}
		বিরতি;
	ক্ষেত্র 3:
		নির্বাচন (ichar) {
		ক্ষেত্র '~':
			নির্বাচন (cch->esc_save[2]) {
			ক্ষেত্র '3':	/* Delete key */
				ichar = CTL_CH('d');
				act = ESC_CONVERTED;
				বিরতি;	/* pass to ^D handler */
			ক্ষেত্র '1':	/* Home key */
			ক্ষেত্র '7':
				ichar = CTL_CH('a');
				act = ESC_CONVERTED;
				বিরতি;	/* pass to ^A handler */
			ক্ষেত্র '4':	/* End key */
			ক্ষেত্র '8':
				ichar = CTL_CH('e');
				act = ESC_CONVERTED;
				বিরতি;	/* pass to ^E handler */
			}
			বিরতি;
		ক্ষেত্র '0':
			যদি (cch->esc_save[2] == '2')
				act = ESC_SAVE;
			বিরতি;
		}
		বিরতি;
	ক্ষেত্র 4:
		নির্বাচন (ichar) {
		ক্ষেত্র '0':
		ক্ষেত্র '1':
			act = ESC_SAVE;
			বিরতি;		/* bracketed paste */
		}
		বিরতি;
	ক্ষেত্র 5:
		যদি (ichar == '~') {	/* bracketed paste */
			ichar = 0;
			act = ESC_CONVERTED;
		}
	}

	*actp = act;

	ফেরত ichar;
}

পূর্ণসংখ্যা cli_ch_process(কাঠামো cli_ch_state *cch, পূর্ণসংখ্যা ichar)
{
	/*
	 * ichar=0x0 when error occurs in U-Boot getchar() or when the caller
	 * wants to check if there are more characters saved in the escape
	 * sequence
	 */
	যদি (!ichar) {
		যদি (cch->emitting) {
			যদি (cch->emit_upto < cch->esc_len)
				ফেরত cch->esc_save[cch->emit_upto++];
			cch->emit_upto = 0;
			cch->emitting = false;
			cch->esc_len = 0;
		}
		ফেরত 0;
	} অন্যথা যদি (ichar == -ETIMEDOUT) {
		/*
		 * If we are in an escape sequence but nothing has followed the
		 * Escape character, then the user probably just pressed the
		 * Escape key. Return it and clear the sequence.
		 */
		যদি (cch->esc_len) {
			cch->esc_len = 0;
			ফেরত '\e';
		}

		/* Otherwise there is nothing to return */
		ফেরত 0;
	}

	যদি (ichar == '\n' || ichar == '\r')
		ফেরত '\n';

	/* handle standard linux xterm esc sequences for arrow key, etc. */
	যদি (cch->esc_len != 0) {
		গণনা cli_esc_state_t act;

		ichar = cli_ch_esc(cch, ichar, &act);

		নির্বাচন (act) {
		ক্ষেত্র ESC_SAVE:
			/* save this character and return nothing */
			cch->esc_save[cch->esc_len++] = ichar;
			ichar = 0;
			বিরতি;
		ক্ষেত্র ESC_REJECT:
			/*
			 * invalid escape sequence, start returning the
			 * characters in it
			 */
			cch->esc_save[cch->esc_len++] = ichar;
			ichar = cch->esc_save[cch->emit_upto++];
			cch->emitting = true;
			ফেরত ichar;
		ক্ষেত্র ESC_CONVERTED:
			/* valid escape sequence, return the resulting char */
			cch->esc_len = 0;
			বিরতি;
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
		ফেরত 0;
	}

	ফেরত ichar;
}
