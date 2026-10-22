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

#அந்தர்பாவ <asm/ptrace.h>

#அந்தர்பாவ <kgdb.h>
#அந்தர்பாவ <command.h>

#undef KGDB_DEBUG

/*
 * BUFMAX defines the maximum number of characters in inbound/outbound buffers
 */
#define BUFMAX 1024
நிலையான எழுத்து remcomInBuffer[BUFMAX];
நிலையான எழுத்து remcomOutBuffer[BUFMAX];
நிலையான எழுத்து remcomRegBuffer[BUFMAX];

நிலையான முழு initialized = 0;
நிலையான முழு kgdb_active;
நிலையான கட்டமைப்பு pt_regs entry_regs;
நிலையான long error_jmp_buf[BUFMAX/2];
நிலையான முழு longjmp_on_fault = 0;
#ifdef KGDB_DEBUG
நிலையான முழு kdebug = 1;
#endif

நிலையான மாறிலி எழுத்து hexchars[]="0123456789abcdef";

/* Convert ch from a hex digit to an int */
நிலையான முழு
hex(unsigned எழுத்து ch)
{
	எனில் (ch >= 'a' && ch <= 'f')
		திரும்பு ch-'a'+10;
	எனில் (ch >= '0' && ch <= '9')
		திரும்பு ch-'0';
	எனில் (ch >= 'A' && ch <= 'F')
		திரும்பு ch-'A'+10;
	திரும்பு -1;
}

/* Convert the memory pointed to by mem into hex, placing result in buf.
 * Return a pointer to the last char put in buf (null).
 */
நிலையான unsigned எழுத்து *
mem2hex(எழுத்து *mem, எழுத்து *buf, முழு மொத்தம்)
{
	எழுத்து *tmp;
	unsigned எழுத்து ch;

	/*
	 * We use the upper half of buf as an intermediate buffer for the
	 * raw memory copy.  Hex conversion will work against this one.
	 */
	tmp = buf + மொத்தம்;
	longjmp_on_fault = 1;

	memcpy(tmp, mem, மொத்தம்);

	போது (மொத்தம்-- > 0) {
		ch = *tmp++;
		*buf++ = hexchars[ch >> 4];
		*buf++ = hexchars[ch & 0xf];
	}
	*buf = 0;
	longjmp_on_fault = 0;
	திரும்பு (unsigned எழுத்து *)buf;
}

/* convert the hex array pointed to by buf into binary to be placed in mem
 * return a pointer to the character AFTER the last byte fetched from buf.
*/
நிலையான எழுத்து *
hex2mem(எழுத்து *buf, எழுத்து *mem, முழு மொத்தம்)
{
	முழு hex_மதிப்பு;
	எழுத்து *tmp_raw, *tmp_hex;

	/*
	 * We use the upper half of buf as an intermediate buffer for the
	 * raw memory that is converted from hex.
	 */
	tmp_raw = buf + மொத்தம் * 2;
	tmp_hex = tmp_raw - 1;

	longjmp_on_fault = 1;
	போது (tmp_hex >= buf) {
		tmp_raw--;
		hex_மதிப்பு = hex(*tmp_hex--);
		எனில் (hex_மதிப்பு < 0)
			kgdb_error(KGDBERR_NOTHEXDIG);
		*tmp_raw = hex_மதிப்பு;
		hex_மதிப்பு = hex(*tmp_hex--);
		எனில் (hex_மதிப்பு < 0)
			kgdb_error(KGDBERR_NOTHEXDIG);
		*tmp_raw |= hex_மதிப்பு << 4;

	}

	memcpy(mem, tmp_raw, மொத்தம்);

	kgdb_flush_cache_range((வெறுமை *)mem, (வெறுமை *)(mem+மொத்தம்));
	longjmp_on_fault = 0;

	திரும்பு buf;
}

/*
 * While we find nice hex chars, build an int.
 * Return number of chars processed.
 */
நிலையான முழு
hexToInt(எழுத்து **ptr, முழு *int_மதிப்பு)
{
	முழு இலக்கம்_Chars = 0;
	முழு hex_மதிப்பு;

	*int_மதிப்பு = 0;

	longjmp_on_fault = 1;
	போது (**ptr) {
		hex_மதிப்பு = hex(**ptr);
		எனில் (hex_மதிப்பு < 0)
			நிறுத்து;

		*int_மதிப்பு = (*int_மதிப்பு << 4) | hex_மதிப்பு;
		இலக்கம்_Chars ++;

		(*ptr)++;
	}
	longjmp_on_fault = 0;

	திரும்பு (இலக்கம்_Chars);
}

/* scan for the sequence $<data>#<checksum>     */
நிலையான வெறுமை
getpacket(எழுத்து *buffer)
{
	unsigned எழுத்து checksum;
	unsigned எழுத்து xmitcsum;
	முழு i;
	முழு மொத்தம்;
	unsigned எழுத்து ch;

	செய் {
		/* wait around for the start character, ignore all other
		 * characters */
		போது ((ch = (getDebugChar() & 0x7f)) != '$') {
#ifdef KGDB_DEBUG
			எனில் (kdebug)
				putc(ch);
#endif
			;
		}

		checksum = 0;
		xmitcsum = -1;

		மொத்தம் = 0;

		/* now, read until a # or end of buffer is found */
		போது (மொத்தம் < BUFMAX) {
			ch = getDebugChar() & 0x7f;
			எனில் (ch == '#')
				நிறுத்து;
			checksum = checksum + ch;
			buffer[மொத்தம்] = ch;
			மொத்தம் = மொத்தம் + 1;
		}

		எனில் (மொத்தம் >= BUFMAX)
			தொடர்;

		buffer[மொத்தம்] = 0;

		எனில் (ch == '#') {
			xmitcsum = hex(getDebugChar() & 0x7f) << 4;
			xmitcsum |= hex(getDebugChar() & 0x7f);
			எனில் (checksum != xmitcsum)
				putDebugChar('-');	/* failed checksum */
			இல்லையெனில் {
				putDebugChar('+'); /* successful transfer */
				/* if a sequence char is present, reply the ID */
				எனில் (buffer[2] == ':') {
					putDebugChar(buffer[0]);
					putDebugChar(buffer[1]);
					/* remove sequence chars from buffer */
					மொத்தம் = strlen(buffer);
					க்கு (i=3; i <= மொத்தம்; i++)
						buffer[i-3] = buffer[i];
				}
			}
		}
	} போது (checksum != xmitcsum);
}

/* send the packet in buffer.  */
நிலையான வெறுமை
putpacket(unsigned எழுத்து *buffer)
{
	unsigned எழுத்து checksum;
	முழு மொத்தம்;
	unsigned எழுத்து ch, recv;

	/*  $<packet info>#<checksum>. */
	செய் {
		putDebugChar('$');
		checksum = 0;
		மொத்தம் = 0;

		போது ((ch = buffer[மொத்தம்])) {
			putDebugChar(ch);
			checksum += ch;
			மொத்தம் += 1;
		}

		putDebugChar('#');
		putDebugChar(hexchars[checksum >> 4]);
		putDebugChar(hexchars[checksum & 0xf]);
		recv = getDebugChar();
	} போது ((recv & 0x7f) != '+');
}

/*
 * This function does all command processing for interfacing to gdb.
 */
நிலையான முழு
handle_exception (கட்டமைப்பு pt_regs *regs)
{
	முழு addr;
	முழு length;
	எழுத்து *ptr;
	kgdb_data kd;
	முழு i;

	எனில் (!initialized) {
		printf("kgdb: exception before kgdb is initialized! huh?\n");
		திரும்பு (0);
	}

	/* probably should check which exception occurred as well */
	எனில் (longjmp_on_fault) {
		longjmp_on_fault = 0;
		kgdb_longjmp(error_jmp_buf, KGDBERR_MEMFAULT);
		panic("kgdb longjump failed!\n");
	}

	எனில் (kgdb_active) {
		printf("kgdb: unexpected exception from within kgdb\n");
		திரும்பு (0);
	}
	kgdb_active = 1;

	kgdb_interruptible(0);

	printf("kgdb: handle_exception; trap [0x%x]\n", kgdb_trap(regs));

	எனில் (kgdb_setjmp(error_jmp_buf) != 0)
		panic("kgdb: error or fault in entry init!\n");

	kgdb_enter(regs, &kd);

	entry_regs = *regs;

	ptr = remcomOutBuffer;

	*ptr++ = 'T';

	*ptr++ = hexchars[kd.sigval >> 4];
	*ptr++ = hexchars[kd.sigval & 0xf];

	க்கு (i = 0; i < kd.nregs; i++) {
		kgdb_reg *rp = &kd.regs[i];

		*ptr++ = hexchars[rp->இலக்கம் >> 4];
		*ptr++ = hexchars[rp->இலக்கம் & 0xf];
		*ptr++ = ':';
		ptr = (எழுத்து *)mem2hex((எழுத்து *)&rp->val, ptr, 4);
		*ptr++ = ';';
	}

	*ptr = 0;

#ifdef KGDB_DEBUG
	எனில் (kdebug)
		printf("kgdb: remcomOutBuffer: %s\n", remcomOutBuffer);
#endif

	putpacket((unsigned எழுத்து *)&remcomOutBuffer);

	போது (1) {
		நிலையற்ற முழு errnum;

		remcomOutBuffer[0] = 0;

		getpacket(remcomInBuffer);
		ptr = &remcomInBuffer[1];

#ifdef KGDB_DEBUG
		எனில் (kdebug)
			printf("kgdb:  remcomInBuffer: %s\n", remcomInBuffer);
#endif

		errnum = kgdb_setjmp(error_jmp_buf);

		எனில் (errnum == 0) தேர்வு (remcomInBuffer[0]) {

		நிலை '?':               /* report most recent signal */
			remcomOutBuffer[0] = 'S';
			remcomOutBuffer[1] = hexchars[kd.sigval >> 4];
			remcomOutBuffer[2] = hexchars[kd.sigval & 0xf];
			remcomOutBuffer[3] = 0;
			நிறுத்து;

#ifdef KGDB_DEBUG
		நிலை 'd':
			/* toggle debug flag */
			kdebug ^= 1;
			நிறுத்து;
#endif

		நிலை 'g':	/* return the value of the CPU registers. */
			length = kgdb_getregs(regs, remcomRegBuffer, BUFMAX);
			mem2hex(remcomRegBuffer, remcomOutBuffer, length);
			நிறுத்து;

		நிலை 'G':   /* set the value of the CPU registers */
			length = strlen(ptr);
			எனில் ((length & 1) != 0) kgdb_error(KGDBERR_BADPARAMS);
			hex2mem(ptr, remcomRegBuffer, length/2);
			kgdb_putregs(regs, remcomRegBuffer, length/2);
			strcpy(remcomOutBuffer,"OK");
			நிறுத்து;

		நிலை 'm':	/* mAA..AA,LLLL  Read LLLL bytes at address AA..AA */
				/* Try to read %x,%x.  */

			எனில் (hexToInt(&ptr, &addr)
			    && *ptr++ == ','
			    && hexToInt(&ptr, &length))	{
				mem2hex((எழுத்து *)addr, remcomOutBuffer, length);
			} இல்லையெனில் {
				kgdb_error(KGDBERR_BADPARAMS);
			}
			நிறுத்து;

		நிலை 'M': /* MAA..AA,LLLL: Write LLLL bytes at address AA.AA return OK */
			/* Try to read '%x,%x:'.  */

			எனில் (hexToInt(&ptr, &addr)
			    && *ptr++ == ','
			    && hexToInt(&ptr, &length)
			    && *ptr++ == ':') {
				hex2mem(ptr, (எழுத்து *)addr, length);
				strcpy(remcomOutBuffer, "OK");
			} இல்லையெனில் {
				kgdb_error(KGDBERR_BADPARAMS);
			}
			நிறுத்து;

		நிலை 'k':    /* kill the program, actually return to monitor */
			kd.extype = KGDBEXIT_KILL;
			*regs = entry_regs;
			செல் doexit;

		நிலை 'C':    /* CSS  continue with signal SS */
			*ptr = '\0';	/* ignore the signal number for now */
			/* fall through */

		நிலை 'c':    /* cAA..AA  Continue; address AA..AA optional */
			/* try to read optional parameter, pc unchanged if no parm */
			kd.extype = KGDBEXIT_CONTINUE;

			எனில் (hexToInt(&ptr, &addr)) {
				kd.exaddr = addr;
				kd.extype |= KGDBEXIT_WITHADDR;
			}

			செல் doexit;

		நிலை 'S':    /* SSS  single step with signal SS */
			*ptr = '\0';	/* ignore the signal number for now */
			/* fall through */

		நிலை 's':
			kd.extype = KGDBEXIT_SINGLE;

			எனில் (hexToInt(&ptr, &addr)) {
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
			திரும்பு (1);

		நிலை 'r':		/* Reset (if user process..exit ???)*/
			panic("kgdb reset.");
			நிறுத்து;

		நிலை 'P':    /* Pr=v  set reg r to value v (r and v are hex) */
			எனில் (hexToInt(&ptr, &addr)
			    && *ptr++ == '='
			    && ((length = strlen(ptr)) & 1) == 0) {
				hex2mem(ptr, remcomRegBuffer, length/2);
				kgdb_putreg(regs, addr,
					remcomRegBuffer, length/2);
				strcpy(remcomOutBuffer,"OK");
			} இல்லையெனில் {
				kgdb_error(KGDBERR_BADPARAMS);
			}
			நிறுத்து;
		}			/* switch */

		எனில் (errnum != 0)
			sprintf(remcomOutBuffer, "E%02d", errnum);

#ifdef KGDB_DEBUG
		எனில் (kdebug)
			printf("kgdb: remcomOutBuffer: %s\n", remcomOutBuffer);
#endif

		/* reply to the request */
		putpacket((unsigned எழுத்து *)&remcomOutBuffer);

	} /* while(1) */
}

/*
 * kgdb_init must be called *after* the
 * monitor is relocated into ram
 */
முழு kgdb_init(வெறுமை)
{
	puts("KGDB:  ");

	kgdb_serial_init();
	debugger_exception_handler = handle_exception;
	initialized = 1;

	putDebugStr("kgdb ready\n");
	puts("ready\n");

	திரும்பு 0;
}

வெறுமை
kgdb_error(முழு errnum)
{
	longjmp_on_fault = 0;
	kgdb_longjmp(error_jmp_buf, errnum);
	panic("kgdb_error: longjmp failed!\n");
}

/* Output string in GDB O-packet format if GDB has connected. If nothing
   output, returns 0 (caller must then handle output). */
முழு
kgdb_output_string (மாறிலி எழுத்து* s, unsigned முழு மொத்தம்)
{
	எழுத்து buffer[512];

	மொத்தம் = (மொத்தம் <= (அளவு(buffer) / 2 - 2))
		? மொத்தம் : (அளவு(buffer) / 2 - 2);

	buffer[0] = 'O';
	mem2hex ((எழுத்து *)s, &buffer[1], மொத்தம்);
	putpacket((unsigned எழுத்து *)&buffer);

	திரும்பு 1;
}

வெறுமை
breakpoint(வெறுமை)
{
	எனில் (!initialized) {
		printf("breakpoint() called b4 kgdb init\n");
		திரும்பு;
	}

	kgdb_breakpoint(0, 0);
}

முழு
do_kgdb(கட்டமைப்பு cmd_tbl *cmdtp, முழு flag, முழு argc, எழுத்து *மாறிலி argv[])
{
    printf("Entering KGDB mode via exception handler...\n\n");
    kgdb_breakpoint(argc - 1, argv + 1);
    printf("\nReturned from KGDB mode\n");
    திரும்பு 0;
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
