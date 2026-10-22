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

#ಅನ್ತರ್ಭಾವ <asm/ptrace.h>

#ಅನ್ತರ್ಭಾವ <kgdb.h>
#ಅನ್ತರ್ಭಾವ <command.h>

#undef KGDB_DEBUG

/*
 * BUFMAX defines the maximum number of characters in inbound/outbound buffers
 */
#define BUFMAX 1024
ಸ್ಥಿರ ವರ್ಣ remcomInBuffer[BUFMAX];
ಸ್ಥಿರ ವರ್ಣ remcomOutBuffer[BUFMAX];
ಸ್ಥಿರ ವರ್ಣ remcomRegBuffer[BUFMAX];

ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ initialized = 0;
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ kgdb_active;
ಸ್ಥಿರ ಸಂರಚನಾ pt_regs entry_regs;
ಸ್ಥಿರ long error_jmp_buf[BUFMAX/2];
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ longjmp_on_fault = 0;
#ifdef KGDB_DEBUG
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ kdebug = 1;
#endif

ಸ್ಥಿರ ಧ್ರುವ ವರ್ಣ hexchars[]="0123456789abcdef";

/* Convert ch from a hex digit to an int */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
hex(unsigned ವರ್ಣ ch)
{
	ಯದಿ (ch >= 'a' && ch <= 'f')
		ಪ್ರತ್ಯಾಗಮ ch-'a'+10;
	ಯದಿ (ch >= '0' && ch <= '9')
		ಪ್ರತ್ಯಾಗಮ ch-'0';
	ಯದಿ (ch >= 'A' && ch <= 'F')
		ಪ್ರತ್ಯಾಗಮ ch-'A'+10;
	ಪ್ರತ್ಯಾಗಮ -1;
}

/* Convert the memory pointed to by mem into hex, placing result in buf.
 * Return a pointer to the last char put in buf (null).
 */
ಸ್ಥಿರ unsigned ವರ್ಣ *
mem2hex(ವರ್ಣ *mem, ವರ್ಣ *buf, ಪೂರ್ಣಾಙ್ಕ ಎಣಿಕೆ)
{
	ವರ್ಣ *tmp;
	unsigned ವರ್ಣ ch;

	/*
	 * We use the upper half of buf as an intermediate buffer for the
	 * raw memory copy.  Hex conversion will work against this one.
	 */
	tmp = buf + ಎಣಿಕೆ;
	longjmp_on_fault = 1;

	memcpy(tmp, mem, ಎಣಿಕೆ);

	ಯಾವತ್ (ಎಣಿಕೆ-- > 0) {
		ch = *tmp++;
		*buf++ = hexchars[ch >> 4];
		*buf++ = hexchars[ch & 0xf];
	}
	*buf = 0;
	longjmp_on_fault = 0;
	ಪ್ರತ್ಯಾಗಮ (unsigned ವರ್ಣ *)buf;
}

/* convert the hex array pointed to by buf into binary to be placed in mem
 * return a pointer to the character AFTER the last byte fetched from buf.
*/
ಸ್ಥಿರ ವರ್ಣ *
hex2mem(ವರ್ಣ *buf, ವರ್ಣ *mem, ಪೂರ್ಣಾಙ್ಕ ಎಣಿಕೆ)
{
	ಪೂರ್ಣಾಙ್ಕ hex_ಮೌಲ್ಯ;
	ವರ್ಣ *tmp_raw, *tmp_hex;

	/*
	 * We use the upper half of buf as an intermediate buffer for the
	 * raw memory that is converted from hex.
	 */
	tmp_raw = buf + ಎಣಿಕೆ * 2;
	tmp_hex = tmp_raw - 1;

	longjmp_on_fault = 1;
	ಯಾವತ್ (tmp_hex >= buf) {
		tmp_raw--;
		hex_ಮೌಲ್ಯ = hex(*tmp_hex--);
		ಯದಿ (hex_ಮೌಲ್ಯ < 0)
			kgdb_error(KGDBERR_NOTHEXDIG);
		*tmp_raw = hex_ಮೌಲ್ಯ;
		hex_ಮೌಲ್ಯ = hex(*tmp_hex--);
		ಯದಿ (hex_ಮೌಲ್ಯ < 0)
			kgdb_error(KGDBERR_NOTHEXDIG);
		*tmp_raw |= hex_ಮೌಲ್ಯ << 4;

	}

	memcpy(mem, tmp_raw, ಎಣಿಕೆ);

	kgdb_flush_cache_range((ಶೂನ್ಯ *)mem, (ಶೂನ್ಯ *)(mem+ಎಣಿಕೆ));
	longjmp_on_fault = 0;

	ಪ್ರತ್ಯಾಗಮ buf;
}

/*
 * While we find nice hex chars, build an int.
 * Return number of chars processed.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
hexToInt(ವರ್ಣ **ptr, ಪೂರ್ಣಾಙ್ಕ *int_ಮೌಲ್ಯ)
{
	ಪೂರ್ಣಾಙ್ಕ ಅಂಕೆ_Chars = 0;
	ಪೂರ್ಣಾಙ್ಕ hex_ಮೌಲ್ಯ;

	*int_ಮೌಲ್ಯ = 0;

	longjmp_on_fault = 1;
	ಯಾವತ್ (**ptr) {
		hex_ಮೌಲ್ಯ = hex(**ptr);
		ಯದಿ (hex_ಮೌಲ್ಯ < 0)
			ಭಙ್ಗ;

		*int_ಮೌಲ್ಯ = (*int_ಮೌಲ್ಯ << 4) | hex_ಮೌಲ್ಯ;
		ಅಂಕೆ_Chars ++;

		(*ptr)++;
	}
	longjmp_on_fault = 0;

	ಪ್ರತ್ಯಾಗಮ (ಅಂಕೆ_Chars);
}

/* scan for the sequence $<data>#<checksum>     */
ಸ್ಥಿರ ಶೂನ್ಯ
getpacket(ವರ್ಣ *buffer)
{
	unsigned ವರ್ಣ checksum;
	unsigned ವರ್ಣ xmitcsum;
	ಪೂರ್ಣಾಙ್ಕ i;
	ಪೂರ್ಣಾಙ್ಕ ಎಣಿಕೆ;
	unsigned ವರ್ಣ ch;

	ಕುರು {
		/* wait around for the start character, ignore all other
		 * characters */
		ಯಾವತ್ ((ch = (getDebugChar() & 0x7f)) != '$') {
#ifdef KGDB_DEBUG
			ಯದಿ (kdebug)
				putc(ch);
#endif
			;
		}

		checksum = 0;
		xmitcsum = -1;

		ಎಣಿಕೆ = 0;

		/* now, read until a # or end of buffer is found */
		ಯಾವತ್ (ಎಣಿಕೆ < BUFMAX) {
			ch = getDebugChar() & 0x7f;
			ಯದಿ (ch == '#')
				ಭಙ್ಗ;
			checksum = checksum + ch;
			buffer[ಎಣಿಕೆ] = ch;
			ಎಣಿಕೆ = ಎಣಿಕೆ + 1;
		}

		ಯದಿ (ಎಣಿಕೆ >= BUFMAX)
			ಅನುವರ್ತ;

		buffer[ಎಣಿಕೆ] = 0;

		ಯದಿ (ch == '#') {
			xmitcsum = hex(getDebugChar() & 0x7f) << 4;
			xmitcsum |= hex(getDebugChar() & 0x7f);
			ಯದಿ (checksum != xmitcsum)
				putDebugChar('-');	/* failed checksum */
			ಅನ್ಯಥಾ {
				putDebugChar('+'); /* successful transfer */
				/* if a sequence char is present, reply the ID */
				ಯದಿ (buffer[2] == ':') {
					putDebugChar(buffer[0]);
					putDebugChar(buffer[1]);
					/* remove sequence chars from buffer */
					ಎಣಿಕೆ = strlen(buffer);
					ಪ್ರತಿ (i=3; i <= ಎಣಿಕೆ; i++)
						buffer[i-3] = buffer[i];
				}
			}
		}
	} ಯಾವತ್ (checksum != xmitcsum);
}

/* send the packet in buffer.  */
ಸ್ಥಿರ ಶೂನ್ಯ
putpacket(unsigned ವರ್ಣ *buffer)
{
	unsigned ವರ್ಣ checksum;
	ಪೂರ್ಣಾಙ್ಕ ಎಣಿಕೆ;
	unsigned ವರ್ಣ ch, recv;

	/*  $<packet info>#<checksum>. */
	ಕುರು {
		putDebugChar('$');
		checksum = 0;
		ಎಣಿಕೆ = 0;

		ಯಾವತ್ ((ch = buffer[ಎಣಿಕೆ])) {
			putDebugChar(ch);
			checksum += ch;
			ಎಣಿಕೆ += 1;
		}

		putDebugChar('#');
		putDebugChar(hexchars[checksum >> 4]);
		putDebugChar(hexchars[checksum & 0xf]);
		recv = getDebugChar();
	} ಯಾವತ್ ((recv & 0x7f) != '+');
}

/*
 * This function does all command processing for interfacing to gdb.
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಙ್ಕ
handle_exception (ಸಂರಚನಾ pt_regs *regs)
{
	ಪೂರ್ಣಾಙ್ಕ addr;
	ಪೂರ್ಣಾಙ್ಕ length;
	ವರ್ಣ *ptr;
	kgdb_data kd;
	ಪೂರ್ಣಾಙ್ಕ i;

	ಯದಿ (!initialized) {
		printf("kgdb: exception before kgdb is initialized! huh?\n");
		ಪ್ರತ್ಯಾಗಮ (0);
	}

	/* probably should check which exception occurred as well */
	ಯದಿ (longjmp_on_fault) {
		longjmp_on_fault = 0;
		kgdb_longjmp(error_jmp_buf, KGDBERR_MEMFAULT);
		panic("kgdb longjump failed!\n");
	}

	ಯದಿ (kgdb_active) {
		printf("kgdb: unexpected exception from within kgdb\n");
		ಪ್ರತ್ಯಾಗಮ (0);
	}
	kgdb_active = 1;

	kgdb_interruptible(0);

	printf("kgdb: handle_exception; trap [0x%x]\n", kgdb_trap(regs));

	ಯದಿ (kgdb_setjmp(error_jmp_buf) != 0)
		panic("kgdb: error or fault in entry init!\n");

	kgdb_enter(regs, &kd);

	entry_regs = *regs;

	ptr = remcomOutBuffer;

	*ptr++ = 'T';

	*ptr++ = hexchars[kd.sigval >> 4];
	*ptr++ = hexchars[kd.sigval & 0xf];

	ಪ್ರತಿ (i = 0; i < kd.nregs; i++) {
		kgdb_reg *rp = &kd.regs[i];

		*ptr++ = hexchars[rp->ಅಂಕೆ >> 4];
		*ptr++ = hexchars[rp->ಅಂಕೆ & 0xf];
		*ptr++ = ':';
		ptr = (ವರ್ಣ *)mem2hex((ವರ್ಣ *)&rp->val, ptr, 4);
		*ptr++ = ';';
	}

	*ptr = 0;

#ifdef KGDB_DEBUG
	ಯದಿ (kdebug)
		printf("kgdb: remcomOutBuffer: %s\n", remcomOutBuffer);
#endif

	putpacket((unsigned ವರ್ಣ *)&remcomOutBuffer);

	ಯಾವತ್ (1) {
		ಚಞ್ಚಲ ಪೂರ್ಣಾಙ್ಕ errnum;

		remcomOutBuffer[0] = 0;

		getpacket(remcomInBuffer);
		ptr = &remcomInBuffer[1];

#ifdef KGDB_DEBUG
		ಯದಿ (kdebug)
			printf("kgdb:  remcomInBuffer: %s\n", remcomInBuffer);
#endif

		errnum = kgdb_setjmp(error_jmp_buf);

		ಯದಿ (errnum == 0) ವಿಕಲ್ಪ (remcomInBuffer[0]) {

		ಪಕ್ಷ '?':               /* report most recent signal */
			remcomOutBuffer[0] = 'S';
			remcomOutBuffer[1] = hexchars[kd.sigval >> 4];
			remcomOutBuffer[2] = hexchars[kd.sigval & 0xf];
			remcomOutBuffer[3] = 0;
			ಭಙ್ಗ;

#ifdef KGDB_DEBUG
		ಪಕ್ಷ 'd':
			/* toggle debug flag */
			kdebug ^= 1;
			ಭಙ್ಗ;
#endif

		ಪಕ್ಷ 'g':	/* return the value of the CPU registers. */
			length = kgdb_getregs(regs, remcomRegBuffer, BUFMAX);
			mem2hex(remcomRegBuffer, remcomOutBuffer, length);
			ಭಙ್ಗ;

		ಪಕ್ಷ 'G':   /* set the value of the CPU registers */
			length = strlen(ptr);
			ಯದಿ ((length & 1) != 0) kgdb_error(KGDBERR_BADPARAMS);
			hex2mem(ptr, remcomRegBuffer, length/2);
			kgdb_putregs(regs, remcomRegBuffer, length/2);
			strcpy(remcomOutBuffer,"OK");
			ಭಙ್ಗ;

		ಪಕ್ಷ 'm':	/* mAA..AA,LLLL  Read LLLL bytes at address AA..AA */
				/* Try to read %x,%x.  */

			ಯದಿ (hexToInt(&ptr, &addr)
			    && *ptr++ == ','
			    && hexToInt(&ptr, &length))	{
				mem2hex((ವರ್ಣ *)addr, remcomOutBuffer, length);
			} ಅನ್ಯಥಾ {
				kgdb_error(KGDBERR_BADPARAMS);
			}
			ಭಙ್ಗ;

		ಪಕ್ಷ 'M': /* MAA..AA,LLLL: Write LLLL bytes at address AA.AA return OK */
			/* Try to read '%x,%x:'.  */

			ಯದಿ (hexToInt(&ptr, &addr)
			    && *ptr++ == ','
			    && hexToInt(&ptr, &length)
			    && *ptr++ == ':') {
				hex2mem(ptr, (ವರ್ಣ *)addr, length);
				strcpy(remcomOutBuffer, "OK");
			} ಅನ್ಯಥಾ {
				kgdb_error(KGDBERR_BADPARAMS);
			}
			ಭಙ್ಗ;

		ಪಕ್ಷ 'k':    /* kill the program, actually return to monitor */
			kd.extype = KGDBEXIT_KILL;
			*regs = entry_regs;
			ಗಚ್ಛ doexit;

		ಪಕ್ಷ 'C':    /* CSS  continue with signal SS */
			*ptr = '\0';	/* ignore the signal number for now */
			/* fall through */

		ಪಕ್ಷ 'c':    /* cAA..AA  Continue; address AA..AA optional */
			/* try to read optional parameter, pc unchanged if no parm */
			kd.extype = KGDBEXIT_CONTINUE;

			ಯದಿ (hexToInt(&ptr, &addr)) {
				kd.exaddr = addr;
				kd.extype |= KGDBEXIT_WITHADDR;
			}

			ಗಚ್ಛ doexit;

		ಪಕ್ಷ 'S':    /* SSS  single step with signal SS */
			*ptr = '\0';	/* ignore the signal number for now */
			/* fall through */

		ಪಕ್ಷ 's':
			kd.extype = KGDBEXIT_SINGLE;

			ಯದಿ (hexToInt(&ptr, &addr)) {
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
			ಪ್ರತ್ಯಾಗಮ (1);

		ಪಕ್ಷ 'r':		/* Reset (if user process..exit ???)*/
			panic("kgdb reset.");
			ಭಙ್ಗ;

		ಪಕ್ಷ 'P':    /* Pr=v  set reg r to value v (r and v are hex) */
			ಯದಿ (hexToInt(&ptr, &addr)
			    && *ptr++ == '='
			    && ((length = strlen(ptr)) & 1) == 0) {
				hex2mem(ptr, remcomRegBuffer, length/2);
				kgdb_putreg(regs, addr,
					remcomRegBuffer, length/2);
				strcpy(remcomOutBuffer,"OK");
			} ಅನ್ಯಥಾ {
				kgdb_error(KGDBERR_BADPARAMS);
			}
			ಭಙ್ಗ;
		}			/* switch */

		ಯದಿ (errnum != 0)
			sprintf(remcomOutBuffer, "E%02d", errnum);

#ifdef KGDB_DEBUG
		ಯದಿ (kdebug)
			printf("kgdb: remcomOutBuffer: %s\n", remcomOutBuffer);
#endif

		/* reply to the request */
		putpacket((unsigned ವರ್ಣ *)&remcomOutBuffer);

	} /* while(1) */
}

/*
 * kgdb_init must be called *after* the
 * monitor is relocated into ram
 */
ಪೂರ್ಣಾಙ್ಕ kgdb_init(ಶೂನ್ಯ)
{
	puts("KGDB:  ");

	kgdb_serial_init();
	debugger_exception_handler = handle_exception;
	initialized = 1;

	putDebugStr("kgdb ready\n");
	puts("ready\n");

	ಪ್ರತ್ಯಾಗಮ 0;
}

ಶೂನ್ಯ
kgdb_error(ಪೂರ್ಣಾಙ್ಕ errnum)
{
	longjmp_on_fault = 0;
	kgdb_longjmp(error_jmp_buf, errnum);
	panic("kgdb_error: longjmp failed!\n");
}

/* Output string in GDB O-packet format if GDB has connected. If nothing
   output, returns 0 (caller must then handle output). */
ಪೂರ್ಣಾಙ್ಕ
kgdb_output_string (ಧ್ರುವ ವರ್ಣ* s, unsigned ಪೂರ್ಣಾಙ್ಕ ಎಣಿಕೆ)
{
	ವರ್ಣ buffer[512];

	ಎಣಿಕೆ = (ಎಣಿಕೆ <= (ಪರಿಮಾಣ(buffer) / 2 - 2))
		? ಎಣಿಕೆ : (ಪರಿಮಾಣ(buffer) / 2 - 2);

	buffer[0] = 'O';
	mem2hex ((ವರ್ಣ *)s, &buffer[1], ಎಣಿಕೆ);
	putpacket((unsigned ವರ್ಣ *)&buffer);

	ಪ್ರತ್ಯಾಗಮ 1;
}

ಶೂನ್ಯ
breakpoint(ಶೂನ್ಯ)
{
	ಯದಿ (!initialized) {
		printf("breakpoint() called b4 kgdb init\n");
		ಪ್ರತ್ಯಾಗಮ;
	}

	kgdb_breakpoint(0, 0);
}

ಪೂರ್ಣಾಙ್ಕ
do_kgdb(ಸಂರಚನಾ cmd_tbl *cmdtp, ಪೂರ್ಣಾಙ್ಕ flag, ಪೂರ್ಣಾಙ್ಕ argc, ವರ್ಣ *ಧ್ರುವ argv[])
{
    printf("Entering KGDB mode via exception handler...\n\n");
    kgdb_breakpoint(argc - 1, argv + 1);
    printf("\nReturned from KGDB mode\n");
    ಪ್ರತ್ಯಾಗಮ 0;
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
