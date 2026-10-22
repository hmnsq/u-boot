// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Copyright 2022 Google LLC
 */

#समावेश <cli.h>
#समावेश <stdio.h>
#समावेश <string.h>
#समावेश <linux/errno.h>

/**
 * enum cli_esc_state_t - indicates what to do with an escape character
 *
 * @ESC_REJECT: Invalid escape sequence, so the esc_save[] characters are
 *	returned from each subsequent call to cli_ch_esc()
 * @ESC_SAVE: Character should be saved in esc_save until we have another one
 * @ESC_CONVERTED: Escape sequence has been completed and the resulting
 *	character is available
 */
क्रमागत cli_esc_state_t {
	ESC_REJECT,
	ESC_SAVE,
	ESC_CONVERTED
};

व्योम cli_ch_init(काष्ठा cli_ch_state *cch)
{
	स_रखो(cch, '\0', माप(*cch));
}

/**
 * cli_ch_esc() - Process a character in an ongoing escape sequence
 *
 * @cch: State information
 * @ichar: Character to process
 * @actp: Returns the action to take
 * Returns: Output character if *actp is ESC_CONVERTED, else 0
 */
अटल पूर्णांक cli_ch_esc(काष्ठा cli_ch_state *cch, पूर्णांक ichar,
		      क्रमागत cli_esc_state_t *actp)
{
	क्रमागत cli_esc_state_t act = ESC_REJECT;

	चयन (cch->esc_len) {
	हाल 1:
		अगर (ichar == '[' || ichar == 'O')
			act = ESC_SAVE;
		अन्यथा
			act = ESC_CONVERTED;
		अवरोध;
	हाल 2:
		चयन (ichar) {
		हाल 'D':	/* <- key */
			ichar = CTL_CH('b');
			act = ESC_CONVERTED;
			अवरोध;	/* pass off to ^B handler */
		हाल 'C':	/* -> key */
			ichar = CTL_CH('f');
			act = ESC_CONVERTED;
			अवरोध;	/* pass off to ^F handler */
		हाल 'H':	/* Home key */
			ichar = CTL_CH('a');
			act = ESC_CONVERTED;
			अवरोध;	/* pass off to ^A handler */
		हाल 'F':	/* End key */
			ichar = CTL_CH('e');
			act = ESC_CONVERTED;
			अवरोध;	/* pass off to ^E handler */
		हाल 'A':	/* up arrow */
			ichar = CTL_CH('p');
			act = ESC_CONVERTED;
			अवरोध;	/* pass off to ^P handler */
		हाल 'B':	/* down arrow */
			ichar = CTL_CH('n');
			act = ESC_CONVERTED;
			अवरोध;	/* pass off to ^N handler */
		हाल '1':
		हाल '2':
		हाल '3':
		हाल '4':
		हाल '7':
		हाल '8':
			अगर (cch->esc_save[1] == '[') {
				/* see if next character is ~ */
				act = ESC_SAVE;
			}
			अवरोध;
		}
		अवरोध;
	हाल 3:
		चयन (ichar) {
		हाल '~':
			चयन (cch->esc_save[2]) {
			हाल '3':	/* Delete key */
				ichar = CTL_CH('d');
				act = ESC_CONVERTED;
				अवरोध;	/* pass to ^D handler */
			हाल '1':	/* Home key */
			हाल '7':
				ichar = CTL_CH('a');
				act = ESC_CONVERTED;
				अवरोध;	/* pass to ^A handler */
			हाल '4':	/* End key */
			हाल '8':
				ichar = CTL_CH('e');
				act = ESC_CONVERTED;
				अवरोध;	/* pass to ^E handler */
			}
			अवरोध;
		हाल '0':
			अगर (cch->esc_save[2] == '2')
				act = ESC_SAVE;
			अवरोध;
		}
		अवरोध;
	हाल 4:
		चयन (ichar) {
		हाल '0':
		हाल '1':
			act = ESC_SAVE;
			अवरोध;		/* bracketed paste */
		}
		अवरोध;
	हाल 5:
		अगर (ichar == '~') {	/* bracketed paste */
			ichar = 0;
			act = ESC_CONVERTED;
		}
	}

	*actp = act;

	वापस ichar;
}

पूर्णांक cli_ch_process(काष्ठा cli_ch_state *cch, पूर्णांक ichar)
{
	/*
	 * ichar=0x0 when error occurs in U-Boot getchar() or when the caller
	 * wants to check if there are more characters saved in the escape
	 * sequence
	 */
	अगर (!ichar) {
		अगर (cch->emitting) {
			अगर (cch->emit_upto < cch->esc_len)
				वापस cch->esc_save[cch->emit_upto++];
			cch->emit_upto = 0;
			cch->emitting = false;
			cch->esc_len = 0;
		}
		वापस 0;
	} अन्यथा अगर (ichar == -ETIMEDOUT) {
		/*
		 * If we are in an escape sequence but nothing has followed the
		 * Escape character, then the user probably just pressed the
		 * Escape key. Return it and clear the sequence.
		 */
		अगर (cch->esc_len) {
			cch->esc_len = 0;
			वापस '\e';
		}

		/* Otherwise there is nothing to return */
		वापस 0;
	}

	अगर (ichar == '\n' || ichar == '\r')
		वापस '\n';

	/* handle standard linux xterm esc sequences for arrow key, etc. */
	अगर (cch->esc_len != 0) {
		क्रमागत cli_esc_state_t act;

		ichar = cli_ch_esc(cch, ichar, &act);

		चयन (act) {
		हाल ESC_SAVE:
			/* save this character and return nothing */
			cch->esc_save[cch->esc_len++] = ichar;
			ichar = 0;
			अवरोध;
		हाल ESC_REJECT:
			/*
			 * invalid escape sequence, start returning the
			 * characters in it
			 */
			cch->esc_save[cch->esc_len++] = ichar;
			ichar = cch->esc_save[cch->emit_upto++];
			cch->emitting = true;
			वापस ichar;
		हाल ESC_CONVERTED:
			/* valid escape sequence, return the resulting char */
			cch->esc_len = 0;
			अवरोध;
		}
	}

	अगर (ichar == '\e') {
		अगर (!cch->esc_len) {
			cch->esc_save[cch->esc_len] = ichar;
			cch->esc_len = 1;
		} अन्यथा {
			माला_दो("impossible condition #876\n");
			cch->esc_len = 0;
		}
		वापस 0;
	}

	वापस ichar;
}
