/* taken from arch/powerpc/kernel/ppc-stub.c */

/****************************************************************************

		THIS SOFTWARE IS NOT COPYRIGHTED

   HP offers the following for use in the public domain.  HP makes no
   warranty with regard to the software or its performance and the
   user accepts the software "AS IS" with all faults.

   HP DISCLAIMS ANY WARRANTIES, EXPRESS OR IMPLIED, WITH REGARD
   TO THIS SOFTWARE INCLUDING BUT NOT LIMITED TO THE WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.

****************************************************************************/

/****************************************************************************
 *  Header: remcom.c,v 1.34 91/03/09 12:29:49 glenne Exp $
 *
 *  Module name: remcom.c $
 *  Revision: 1.34 $
 *  Date: 91/03/09 12:29:49 $
 *  Contributor:     Lake Stevens Instrument Division$
 *
 *  Description:     low level support for gdb debugger. $
 *
 *  Considerations:  only works on target hardware $
 *
 *  Written by:      Glenn Engel $
 *  ModuleState:     Experimental $
 *
 *  NOTES:           See Below $
 *
 *  Modified for SPARC by Stu Grossman, Cygnus Support.
 *
 *  This code has been extensively tested on the Fujitsu SPARClite demo board.
 *
 *  To enable debugger support, two things need to happen.  One, a
 *  call to set_debug_traps() is necessary in order to allow any breakpoints
 *  or error conditions to be properly intercepted and reported to gdb.
 *  Two, a breakpoint needs to be generated to begin communication.  This
 *  is most easily accomplished by a call to breakpoint().  Breakpoint()
 *  simulates a breakpoint by executing a trap #1.
 *
 *************
 *
 *    The following gdb commands are supported:
 *
 * command          function                               Return value
 *
 *    g             return the value of the CPU registers  hex data or ENN
 *    G             set the value of the CPU registers     OK or ENN
 *    qOffsets      Get section offsets.  Reply is Text=xxx;Data=yyy;Bss=zzz
 *
 *    mAA..AA,LLLL  Read LLLL bytes at address AA..AA      hex data or ENN
 *    MAA..AA,LLLL: Write LLLL bytes at address AA.AA      OK or ENN
 *
 *    c             Resume at current address              SNN   ( signal NN)
 *    cAA..AA       Continue at address AA..AA             SNN
 *
 *    s             Step one instruction                   SNN
 *    sAA..AA       Step one instruction from AA..AA       SNN
 *
 *    k             kill
 *
 *    ?             What was the last sigval ?             SNN   (signal NN)
 *
 *    bBB..BB	    Set baud rate to BB..BB		   OK or BNN, then sets
 *							   baud rate
 *
 * All commands and responses are sent with a packet which includes a
 * checksum.  A packet consists of
 *
 * $<packet info>#<checksum>.
 *
 * where
 * <packet info> :: <characters representing the command or response>
 * <checksum>    :: <two hex digits computed as modulo 256 sum of <packetinfo>>
 *
 * When a packet is received, it is first acknowledged with either '+' or '-'.
 * '+' indicates a successful transfer.  '-' indicates a failed transfer.
 *
 * Example:
 *
 * Host:                  Reply:
 * $m0,10#2a               +$00010203040506070809101112131415#42
 *
 ****************************************************************************/

#अन्तर्भाव <asm/ptrace.h>

#अन्तर्भाव <kgdb.h>
#अन्तर्भाव <command.h>

#undef KGDB_DEBUG

/*
 * BUFMAX defines the maximum number of characters in inbound/outbound buffers
 */
#define BUFMAX 1024
स्थिर वर्ण remcomInBuffer[BUFMAX];
स्थिर वर्ण remcomOutBuffer[BUFMAX];
स्थिर वर्ण remcomRegBuffer[BUFMAX];

स्थिर पूर्णाङ्क initialized = 0;
स्थिर पूर्णाङ्क kgdb_active;
स्थिर संरचना pt_regs entry_regs;
स्थिर long error_jmp_buf[BUFMAX/2];
स्थिर पूर्णाङ्क longjmp_on_fault = 0;
#ifdef KGDB_DEBUG
स्थिर पूर्णाङ्क kdebug = 1;
#endif

स्थिर ध्रुव वर्ण hexchars[]="0123456789abcdef";

/* Convert ch from a hex digit to an int */
स्थिर पूर्णाङ्क
hex(unsigned वर्ण ch)
{
	जइ (ch >= 'a' && ch <= 'f')
		प्रत्यागम ch-'a'+10;
	जइ (ch >= '0' && ch <= '9')
		प्रत्यागम ch-'0';
	जइ (ch >= 'A' && ch <= 'F')
		प्रत्यागम ch-'A'+10;
	प्रत्यागम -1;
}

/* Convert the memory pointed to by mem into hex, placing result in buf.
 * Return a pointer to the last char put in buf (null).
 */
स्थिर unsigned वर्ण *
mem2hex(वर्ण *mem, वर्ण *buf, पूर्णाङ्क गणनम्)
{
	वर्ण *tmp;
	unsigned वर्ण ch;

	/*
	 * We use the upper half of buf as an intermediate buffer for the
	 * raw memory copy.  Hex conversion will work against this one.
	 */
	tmp = buf + गणनम्;
	longjmp_on_fault = 1;

	memcpy(tmp, mem, गणनम्);

	जाव (गणनम्-- > 0) {
		ch = *tmp++;
		*buf++ = hexchars[ch >> 4];
		*buf++ = hexchars[ch & 0xf];
	}
	*buf = 0;
	longjmp_on_fault = 0;
	प्रत्यागम (unsigned वर्ण *)buf;
}

/* convert the hex array pointed to by buf into binary to be placed in mem
 * return a pointer to the character AFTER the last byte fetched from buf.
*/
स्थिर वर्ण *
hex2mem(वर्ण *buf, वर्ण *mem, पूर्णाङ्क गणनम्)
{
	पूर्णाङ्क hex_मानम्;
	वर्ण *tmp_raw, *tmp_hex;

	/*
	 * We use the upper half of buf as an intermediate buffer for the
	 * raw memory that is converted from hex.
	 */
	tmp_raw = buf + गणनम् * 2;
	tmp_hex = tmp_raw - 1;

	longjmp_on_fault = 1;
	जाव (tmp_hex >= buf) {
		tmp_raw--;
		hex_मानम् = hex(*tmp_hex--);
		जइ (hex_मानम् < 0)
			kgdb_error(KGDBERR_NOTHEXDIG);
		*tmp_raw = hex_मानम्;
		hex_मानम् = hex(*tmp_hex--);
		जइ (hex_मानम् < 0)
			kgdb_error(KGDBERR_NOTHEXDIG);
		*tmp_raw |= hex_मानम् << 4;

	}

	memcpy(mem, tmp_raw, गणनम्);

	kgdb_flush_cache_range((शून्य *)mem, (शून्य *)(mem+गणनम्));
	longjmp_on_fault = 0;

	प्रत्यागम buf;
}

/*
 * While we find nice hex chars, build an int.
 * Return number of chars processed.
 */
स्थिर पूर्णाङ्क
hexToInt(वर्ण **ptr, पूर्णाङ्क *int_मानम्)
{
	पूर्णाङ्क अङ्कः_Chars = 0;
	पूर्णाङ्क hex_मानम्;

	*int_मानम् = 0;

	longjmp_on_fault = 1;
	जाव (**ptr) {
		hex_मानम् = hex(**ptr);
		जइ (hex_मानम् < 0)
			भङ्ग;

		*int_मानम् = (*int_मानम् << 4) | hex_मानम्;
		अङ्कः_Chars ++;

		(*ptr)++;
	}
	longjmp_on_fault = 0;

	प्रत्यागम (अङ्कः_Chars);
}

/* scan for the sequence $<data>#<checksum>     */
स्थिर शून्य
getpacket(वर्ण *buffer)
{
	unsigned वर्ण checksum;
	unsigned वर्ण xmitcsum;
	पूर्णाङ्क i;
	पूर्णाङ्क गणनम्;
	unsigned वर्ण ch;

	करेहि {
		/* wait around for the start character, ignore all other
		 * characters */
		जाव ((ch = (getDebugChar() & 0x7f)) != '$') {
#ifdef KGDB_DEBUG
			जइ (kdebug)
				putc(ch);
#endif
			;
		}

		checksum = 0;
		xmitcsum = -1;

		गणनम् = 0;

		/* now, read until a # or end of buffer is found */
		जाव (गणनम् < BUFMAX) {
			ch = getDebugChar() & 0x7f;
			जइ (ch == '#')
				भङ्ग;
			checksum = checksum + ch;
			buffer[गणनम्] = ch;
			गणनम् = गणनम् + 1;
		}

		जइ (गणनम् >= BUFMAX)
			अनुवर्त;

		buffer[गणनम्] = 0;

		जइ (ch == '#') {
			xmitcsum = hex(getDebugChar() & 0x7f) << 4;
			xmitcsum |= hex(getDebugChar() & 0x7f);
			जइ (checksum != xmitcsum)
				putDebugChar('-');	/* failed checksum */
			अण्णहा {
				putDebugChar('+'); /* successful transfer */
				/* if a sequence char is present, reply the ID */
				जइ (buffer[2] == ':') {
					putDebugChar(buffer[0]);
					putDebugChar(buffer[1]);
					/* remove sequence chars from buffer */
					गणनम् = strlen(buffer);
					प्रति (i=3; i <= गणनम्; i++)
						buffer[i-3] = buffer[i];
				}
			}
		}
	} जाव (checksum != xmitcsum);
}

/* send the packet in buffer.  */
स्थिर शून्य
putpacket(unsigned वर्ण *buffer)
{
	unsigned वर्ण checksum;
	पूर्णाङ्क गणनम्;
	unsigned वर्ण ch, recv;

	/*  $<packet info>#<checksum>. */
	करेहि {
		putDebugChar('$');
		checksum = 0;
		गणनम् = 0;

		जाव ((ch = buffer[गणनम्])) {
			putDebugChar(ch);
			checksum += ch;
			गणनम् += 1;
		}

		putDebugChar('#');
		putDebugChar(hexchars[checksum >> 4]);
		putDebugChar(hexchars[checksum & 0xf]);
		recv = getDebugChar();
	} जाव ((recv & 0x7f) != '+');
}

/*
 * This function does all command processing for interfacing to gdb.
 */
स्थिर पूर्णाङ्क
handle_exception (संरचना pt_regs *regs)
{
	पूर्णाङ्क addr;
	पूर्णाङ्क length;
	वर्ण *ptr;
	kgdb_data kd;
	पूर्णाङ्क i;

	जइ (!initialized) {
		printf("kgdb: exception before kgdb is initialized! huh?\n");
		प्रत्यागम (0);
	}

	/* probably should check which exception occurred as well */
	जइ (longjmp_on_fault) {
		longjmp_on_fault = 0;
		kgdb_longjmp(error_jmp_buf, KGDBERR_MEMFAULT);
		panic("kgdb longjump failed!\n");
	}

	जइ (kgdb_active) {
		printf("kgdb: unexpected exception from within kgdb\n");
		प्रत्यागम (0);
	}
	kgdb_active = 1;

	kgdb_interruptible(0);

	printf("kgdb: handle_exception; trap [0x%x]\n", kgdb_trap(regs));

	जइ (kgdb_setjmp(error_jmp_buf) != 0)
		panic("kgdb: error or fault in entry init!\n");

	kgdb_enter(regs, &kd);

	entry_regs = *regs;

	ptr = remcomOutBuffer;

	*ptr++ = 'T';

	*ptr++ = hexchars[kd.sigval >> 4];
	*ptr++ = hexchars[kd.sigval & 0xf];

	प्रति (i = 0; i < kd.nregs; i++) {
		kgdb_reg *rp = &kd.regs[i];

		*ptr++ = hexchars[rp->अङ्कः >> 4];
		*ptr++ = hexchars[rp->अङ्कः & 0xf];
		*ptr++ = ':';
		ptr = (वर्ण *)mem2hex((वर्ण *)&rp->val, ptr, 4);
		*ptr++ = ';';
	}

	*ptr = 0;

#ifdef KGDB_DEBUG
	जइ (kdebug)
		printf("kgdb: remcomOutBuffer: %s\n", remcomOutBuffer);
#endif

	putpacket((unsigned वर्ण *)&remcomOutBuffer);

	जाव (1) {
		चञ्चल पूर्णाङ्क errnum;

		remcomOutBuffer[0] = 0;

		getpacket(remcomInBuffer);
		ptr = &remcomInBuffer[1];

#ifdef KGDB_DEBUG
		जइ (kdebug)
			printf("kgdb:  remcomInBuffer: %s\n", remcomInBuffer);
#endif

		errnum = kgdb_setjmp(error_jmp_buf);

		जइ (errnum == 0) विकल्प (remcomInBuffer[0]) {

		पक्ष '?':               /* report most recent signal */
			remcomOutBuffer[0] = 'S';
			remcomOutBuffer[1] = hexchars[kd.sigval >> 4];
			remcomOutBuffer[2] = hexchars[kd.sigval & 0xf];
			remcomOutBuffer[3] = 0;
			भङ्ग;

#ifdef KGDB_DEBUG
		पक्ष 'd':
			/* toggle debug flag */
			kdebug ^= 1;
			भङ्ग;
#endif

		पक्ष 'g':	/* return the value of the CPU registers. */
			length = kgdb_getregs(regs, remcomRegBuffer, BUFMAX);
			mem2hex(remcomRegBuffer, remcomOutBuffer, length);
			भङ्ग;

		पक्ष 'G':   /* set the value of the CPU registers */
			length = strlen(ptr);
			जइ ((length & 1) != 0) kgdb_error(KGDBERR_BADPARAMS);
			hex2mem(ptr, remcomRegBuffer, length/2);
			kgdb_putregs(regs, remcomRegBuffer, length/2);
			strcpy(remcomOutBuffer,"OK");
			भङ्ग;

		पक्ष 'm':	/* mAA..AA,LLLL  Read LLLL bytes at address AA..AA */
				/* Try to read %x,%x.  */

			जइ (hexToInt(&ptr, &addr)
			    && *ptr++ == ','
			    && hexToInt(&ptr, &length))	{
				mem2hex((वर्ण *)addr, remcomOutBuffer, length);
			} अण्णहा {
				kgdb_error(KGDBERR_BADPARAMS);
			}
			भङ्ग;

		पक्ष 'M': /* MAA..AA,LLLL: Write LLLL bytes at address AA.AA return OK */
			/* Try to read '%x,%x:'.  */

			जइ (hexToInt(&ptr, &addr)
			    && *ptr++ == ','
			    && hexToInt(&ptr, &length)
			    && *ptr++ == ':') {
				hex2mem(ptr, (वर्ण *)addr, length);
				strcpy(remcomOutBuffer, "OK");
			} अण्णहा {
				kgdb_error(KGDBERR_BADPARAMS);
			}
			भङ्ग;

		पक्ष 'k':    /* kill the program, actually return to monitor */
			kd.extype = KGDBEXIT_KILL;
			*regs = entry_regs;
			गच्छ doexit;

		पक्ष 'C':    /* CSS  continue with signal SS */
			*ptr = '\0';	/* ignore the signal number for now */
			/* fall through */

		पक्ष 'c':    /* cAA..AA  Continue; address AA..AA optional */
			/* try to read optional parameter, pc unchanged if no parm */
			kd.extype = KGDBEXIT_CONTINUE;

			जइ (hexToInt(&ptr, &addr)) {
				kd.exaddr = addr;
				kd.extype |= KGDBEXIT_WITHADDR;
			}

			गच्छ doexit;

		पक्ष 'S':    /* SSS  single step with signal SS */
			*ptr = '\0';	/* ignore the signal number for now */
			/* fall through */

		पक्ष 's':
			kd.extype = KGDBEXIT_SINGLE;

			जइ (hexToInt(&ptr, &addr)) {
				kd.exaddr = addr;
				kd.extype |= KGDBEXIT_WITHADDR;
			}

		doexit:
/* Need to flush the instruction cache here, as we may have deposited a
 * breakpoint, and the icache probably has no way of knowing that a data ref to
 * some location may have changed something that is in the instruction cache.
 */
			kgdb_flush_cache_all();
			kgdb_exit(regs, &kd);
			kgdb_active = 0;
			kgdb_interruptible(1);
			प्रत्यागम (1);

		पक्ष 'r':		/* Reset (if user process..exit ???)*/
			panic("kgdb reset.");
			भङ्ग;

		पक्ष 'P':    /* Pr=v  set reg r to value v (r and v are hex) */
			जइ (hexToInt(&ptr, &addr)
			    && *ptr++ == '='
			    && ((length = strlen(ptr)) & 1) == 0) {
				hex2mem(ptr, remcomRegBuffer, length/2);
				kgdb_putreg(regs, addr,
					remcomRegBuffer, length/2);
				strcpy(remcomOutBuffer,"OK");
			} अण्णहा {
				kgdb_error(KGDBERR_BADPARAMS);
			}
			भङ्ग;
		}			/* switch */

		जइ (errnum != 0)
			sprintf(remcomOutBuffer, "E%02d", errnum);

#ifdef KGDB_DEBUG
		जइ (kdebug)
			printf("kgdb: remcomOutBuffer: %s\n", remcomOutBuffer);
#endif

		/* reply to the request */
		putpacket((unsigned वर्ण *)&remcomOutBuffer);

	} /* while(1) */
}

/*
 * kgdb_init must be called *after* the
 * monitor is relocated into ram
 */
पूर्णाङ्क kgdb_init(शून्य)
{
	puts("KGDB:  ");

	kgdb_serial_init();
	debugger_exception_handler = handle_exception;
	initialized = 1;

	putDebugStr("kgdb ready\n");
	puts("ready\n");

	प्रत्यागम 0;
}

शून्य
kgdb_error(पूर्णाङ्क errnum)
{
	longjmp_on_fault = 0;
	kgdb_longjmp(error_jmp_buf, errnum);
	panic("kgdb_error: longjmp failed!\n");
}

/* Output string in GDB O-packet format if GDB has connected. If nothing
   output, returns 0 (caller must then handle output). */
पूर्णाङ्क
kgdb_output_string (ध्रुव वर्ण* s, unsigned पूर्णाङ्क गणनम्)
{
	वर्ण buffer[512];

	गणनम् = (गणनम् <= (परिमाण(buffer) / 2 - 2))
		? गणनम् : (परिमाण(buffer) / 2 - 2);

	buffer[0] = 'O';
	mem2hex ((वर्ण *)s, &buffer[1], गणनम्);
	putpacket((unsigned वर्ण *)&buffer);

	प्रत्यागम 1;
}

शून्य
breakpoint(शून्य)
{
	जइ (!initialized) {
		printf("breakpoint() called b4 kgdb init\n");
		प्रत्यागम;
	}

	kgdb_breakpoint(0, 0);
}

पूर्णाङ्क
do_kgdb(संरचना cmd_tbl *cmdtp, पूर्णाङ्क flag, पूर्णाङ्क argc, वर्ण *ध्रुव argv[])
{
    printf("Entering KGDB mode via exception handler...\n\n");
    kgdb_breakpoint(argc - 1, argv + 1);
    printf("\nReturned from KGDB mode\n");
    प्रत्यागम 0;
}

U_BOOT_CMD(
	kgdb, CONFIG_SYS_MAXARGS, 1,	do_kgdb,
	"enter gdb remote debug mode",
	"[arg0 arg1 .. argN]\n"
	"    - executes a breakpoint so that kgdb mode is\n"
	"      entered via the exception handler. To return\n"
	"      to the monitor, the remote gdb debugger must\n"
	"      execute a \"continue\" or \"quit\" command.\n"
	"\n"
	"      if a program is loaded by the remote gdb, any args\n"
	"      passed to the kgdb command are given to the loaded\n"
	"      program if it is executed (see the \"hello_world\"\n"
	"      example program in the U-Boot examples directory)."
);
