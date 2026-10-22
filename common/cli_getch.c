// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright 2022 Google LLC
 */

#ଅନ୍ତର୍ଭାଵ <cli.h>
#ଅନ୍ତର୍ଭାଵ <stdio.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <linux/errno.h>

/**
 * enum cli_esc_state_t - indicates what to do with an escape character
 *
 * @ESC_REJECT: Invalid escape sequence, so the esc_save[] characters are
 *	returned from each subsequent call to cli_ch_esc()
 * @ESC_SAVE: Character should be saved in esc_save until we have another one
 * @ESC_CONVERTED: Escape sequence has been completed and the resulting
 *	character is available
 */
ଗଣନା cli_esc_state_t {
	ESC_REJECT,
	ESC_SAVE,
	ESC_CONVERTED
};

ଶୂନ୍ୟ cli_ch_init(ସଂରଚନା cli_ch_state *cch)
{
	memset(cch, '\0', ଆକାର(*cch));
}

/**
 * cli_ch_esc() - Process a character in an ongoing escape sequence
 *
 * @cch: State information
 * @ichar: Character to process
 * @actp: Returns the action to take
 * Returns: Output character if *actp is ESC_CONVERTED, else 0
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା cli_ch_esc(ସଂରଚନା cli_ch_state *cch, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ichar,
		      ଗଣନା cli_esc_state_t *actp)
{
	ଗଣନା cli_esc_state_t act = ESC_REJECT;

	ବାଛ (cch->esc_len) {
	ସ୍ଥିତି 1:
		ଯଦି (ichar == '[' || ichar == 'O')
			act = ESC_SAVE;
		ଅନ୍ୟଥା
			act = ESC_CONVERTED;
		ଭାଙ୍ଗ;
	ସ୍ଥିତି 2:
		ବାଛ (ichar) {
		ସ୍ଥିତି 'D':	/* <- key */
			ichar = CTL_CH('b');
			act = ESC_CONVERTED;
			ଭାଙ୍ଗ;	/* pass off to ^B handler */
		ସ୍ଥିତି 'C':	/* -> key */
			ichar = CTL_CH('f');
			act = ESC_CONVERTED;
			ଭାଙ୍ଗ;	/* pass off to ^F handler */
		ସ୍ଥିତି 'H':	/* Home key */
			ichar = CTL_CH('a');
			act = ESC_CONVERTED;
			ଭାଙ୍ଗ;	/* pass off to ^A handler */
		ସ୍ଥିତି 'F':	/* End key */
			ichar = CTL_CH('e');
			act = ESC_CONVERTED;
			ଭାଙ୍ଗ;	/* pass off to ^E handler */
		ସ୍ଥିତି 'A':	/* up arrow */
			ichar = CTL_CH('p');
			act = ESC_CONVERTED;
			ଭାଙ୍ଗ;	/* pass off to ^P handler */
		ସ୍ଥିତି 'B':	/* down arrow */
			ichar = CTL_CH('n');
			act = ESC_CONVERTED;
			ଭାଙ୍ଗ;	/* pass off to ^N handler */
		ସ୍ଥିତି '1':
		ସ୍ଥିତି '2':
		ସ୍ଥିତି '3':
		ସ୍ଥିତି '4':
		ସ୍ଥିତି '7':
		ସ୍ଥିତି '8':
			ଯଦି (cch->esc_save[1] == '[') {
				/* see if next character is ~ */
				act = ESC_SAVE;
			}
			ଭାଙ୍ଗ;
		}
		ଭାଙ୍ଗ;
	ସ୍ଥିତି 3:
		ବାଛ (ichar) {
		ସ୍ଥିତି '~':
			ବାଛ (cch->esc_save[2]) {
			ସ୍ଥିତି '3':	/* Delete key */
				ichar = CTL_CH('d');
				act = ESC_CONVERTED;
				ଭାଙ୍ଗ;	/* pass to ^D handler */
			ସ୍ଥିତି '1':	/* Home key */
			ସ୍ଥିତି '7':
				ichar = CTL_CH('a');
				act = ESC_CONVERTED;
				ଭାଙ୍ଗ;	/* pass to ^A handler */
			ସ୍ଥିତି '4':	/* End key */
			ସ୍ଥିତି '8':
				ichar = CTL_CH('e');
				act = ESC_CONVERTED;
				ଭାଙ୍ଗ;	/* pass to ^E handler */
			}
			ଭାଙ୍ଗ;
		ସ୍ଥିତି '0':
			ଯଦି (cch->esc_save[2] == '2')
				act = ESC_SAVE;
			ଭାଙ୍ଗ;
		}
		ଭାଙ୍ଗ;
	ସ୍ଥିତି 4:
		ବାଛ (ichar) {
		ସ୍ଥିତି '0':
		ସ୍ଥିତି '1':
			act = ESC_SAVE;
			ଭାଙ୍ଗ;		/* bracketed paste */
		}
		ଭାଙ୍ଗ;
	ସ୍ଥିତି 5:
		ଯଦି (ichar == '~') {	/* bracketed paste */
			ichar = 0;
			act = ESC_CONVERTED;
		}
	}

	*actp = act;

	ଫେରନ୍ତୁ ichar;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା cli_ch_process(ସଂରଚନା cli_ch_state *cch, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ichar)
{
	/*
	 * ichar=0x0 when error occurs in U-Boot getchar() or when the caller
	 * wants to check if there are more characters saved in the escape
	 * sequence
	 */
	ଯଦି (!ichar) {
		ଯଦି (cch->emitting) {
			ଯଦି (cch->emit_upto < cch->esc_len)
				ଫେରନ୍ତୁ cch->esc_save[cch->emit_upto++];
			cch->emit_upto = 0;
			cch->emitting = false;
			cch->esc_len = 0;
		}
		ଫେରନ୍ତୁ 0;
	} ଅନ୍ୟଥା ଯଦି (ichar == -ETIMEDOUT) {
		/*
		 * If we are in an escape sequence but nothing has followed the
		 * Escape character, then the user probably just pressed the
		 * Escape key. Return it and clear the sequence.
		 */
		ଯଦି (cch->esc_len) {
			cch->esc_len = 0;
			ଫେରନ୍ତୁ '\e';
		}

		/* Otherwise there is nothing to return */
		ଫେରନ୍ତୁ 0;
	}

	ଯଦି (ichar == '\n' || ichar == '\r')
		ଫେରନ୍ତୁ '\n';

	/* handle standard linux xterm esc sequences for arrow key, etc. */
	ଯଦି (cch->esc_len != 0) {
		ଗଣନା cli_esc_state_t act;

		ichar = cli_ch_esc(cch, ichar, &act);

		ବାଛ (act) {
		ସ୍ଥିତି ESC_SAVE:
			/* save this character and return nothing */
			cch->esc_save[cch->esc_len++] = ichar;
			ichar = 0;
			ଭାଙ୍ଗ;
		ସ୍ଥିତି ESC_REJECT:
			/*
			 * invalid escape sequence, start returning the
			 * characters in it
			 */
			cch->esc_save[cch->esc_len++] = ichar;
			ichar = cch->esc_save[cch->emit_upto++];
			cch->emitting = true;
			ଫେରନ୍ତୁ ichar;
		ସ୍ଥିତି ESC_CONVERTED:
			/* valid escape sequence, return the resulting char */
			cch->esc_len = 0;
			ଭାଙ୍ଗ;
		}
	}

	ଯଦି (ichar == '\e') {
		ଯଦି (!cch->esc_len) {
			cch->esc_save[cch->esc_len] = ichar;
			cch->esc_len = 1;
		} ଅନ୍ୟଥା {
			puts("impossible condition #876\n");
			cch->esc_len = 0;
		}
		ଫେରନ୍ତୁ 0;
	}

	ଫେରନ୍ତୁ ichar;
}
