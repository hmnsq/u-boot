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

#ଅନ୍ତର୍ଭାଵ <asm/ptrace.h>

#ଅନ୍ତର୍ଭାଵ <kgdb.h>
#ଅନ୍ତର୍ଭାଵ <command.h>

#undef KGDB_DEBUG

/*
 * BUFMAX defines the maximum number of characters in inbound/outbound buffers
 */
#define BUFMAX 1024
ସ୍ଥିର ଅକ୍ଷର remcomInBuffer[BUFMAX];
ସ୍ଥିର ଅକ୍ଷର remcomOutBuffer[BUFMAX];
ସ୍ଥିର ଅକ୍ଷର remcomRegBuffer[BUFMAX];

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା initialized = 0;
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା kgdb_active;
ସ୍ଥିର ସଂରଚନା pt_regs entry_regs;
ସ୍ଥିର long error_jmp_buf[BUFMAX/2];
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା longjmp_on_fault = 0;
#ifdef KGDB_DEBUG
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା kdebug = 1;
#endif

ସ୍ଥିର ଅଚଳ ଅକ୍ଷର hexchars[]="0123456789abcdef";

/* Convert ch from a hex digit to an int */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
hex(unsigned ଅକ୍ଷର ch)
{
	ଯଦି (ch >= 'a' && ch <= 'f')
		ଫେରନ୍ତୁ ch-'a'+10;
	ଯଦି (ch >= '0' && ch <= '9')
		ଫେରନ୍ତୁ ch-'0';
	ଯଦି (ch >= 'A' && ch <= 'F')
		ଫେରନ୍ତୁ ch-'A'+10;
	ଫେରନ୍ତୁ -1;
}

/* Convert the memory pointed to by mem into hex, placing result in buf.
 * Return a pointer to the last char put in buf (null).
 */
ସ୍ଥିର unsigned ଅକ୍ଷର *
mem2hex(ଅକ୍ଷର *mem, ଅକ୍ଷର *buf, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2)
{
	ଅକ୍ଷର *tmp;
	unsigned ଅକ୍ଷର ch;

	/*
	 * We use the upper half of buf as an intermediate buffer for the
	 * raw memory copy.  Hex conversion will work against this one.
	 */
	tmp = buf + ଗଣନା_2;
	longjmp_on_fault = 1;

	memcpy(tmp, mem, ଗଣନା_2);

	ଯେତେବେଳେ (ଗଣନା_2-- > 0) {
		ch = *tmp++;
		*buf++ = hexchars[ch >> 4];
		*buf++ = hexchars[ch & 0xf];
	}
	*buf = 0;
	longjmp_on_fault = 0;
	ଫେରନ୍ତୁ (unsigned ଅକ୍ଷର *)buf;
}

/* convert the hex array pointed to by buf into binary to be placed in mem
 * return a pointer to the character AFTER the last byte fetched from buf.
*/
ସ୍ଥିର ଅକ୍ଷର *
hex2mem(ଅକ୍ଷର *buf, ଅକ୍ଷର *mem, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା hex_ମୂଲ୍ୟ;
	ଅକ୍ଷର *tmp_raw, *tmp_hex;

	/*
	 * We use the upper half of buf as an intermediate buffer for the
	 * raw memory that is converted from hex.
	 */
	tmp_raw = buf + ଗଣନା_2 * 2;
	tmp_hex = tmp_raw - 1;

	longjmp_on_fault = 1;
	ଯେତେବେଳେ (tmp_hex >= buf) {
		tmp_raw--;
		hex_ମୂଲ୍ୟ = hex(*tmp_hex--);
		ଯଦି (hex_ମୂଲ୍ୟ < 0)
			kgdb_error(KGDBERR_NOTHEXDIG);
		*tmp_raw = hex_ମୂଲ୍ୟ;
		hex_ମୂଲ୍ୟ = hex(*tmp_hex--);
		ଯଦି (hex_ମୂଲ୍ୟ < 0)
			kgdb_error(KGDBERR_NOTHEXDIG);
		*tmp_raw |= hex_ମୂଲ୍ୟ << 4;

	}

	memcpy(mem, tmp_raw, ଗଣନା_2);

	kgdb_flush_cache_range((ଶୂନ୍ୟ *)mem, (ଶୂନ୍ୟ *)(mem+ଗଣନା_2));
	longjmp_on_fault = 0;

	ଫେରନ୍ତୁ buf;
}

/*
 * While we find nice hex chars, build an int.
 * Return number of chars processed.
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
hexToInt(ଅକ୍ଷର **ptr, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *int_ମୂଲ୍ୟ)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଅଙ୍କ_Chars = 0;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା hex_ମୂଲ୍ୟ;

	*int_ମୂଲ୍ୟ = 0;

	longjmp_on_fault = 1;
	ଯେତେବେଳେ (**ptr) {
		hex_ମୂଲ୍ୟ = hex(**ptr);
		ଯଦି (hex_ମୂଲ୍ୟ < 0)
			ଭାଙ୍ଗ;

		*int_ମୂଲ୍ୟ = (*int_ମୂଲ୍ୟ << 4) | hex_ମୂଲ୍ୟ;
		ଅଙ୍କ_Chars ++;

		(*ptr)++;
	}
	longjmp_on_fault = 0;

	ଫେରନ୍ତୁ (ଅଙ୍କ_Chars);
}

/* scan for the sequence $<data>#<checksum>     */
ସ୍ଥିର ଶୂନ୍ୟ
getpacket(ଅକ୍ଷର *buffer)
{
	unsigned ଅକ୍ଷର checksum;
	unsigned ଅକ୍ଷର xmitcsum;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2;
	unsigned ଅକ୍ଷର ch;

	କର {
		/* wait around for the start character, ignore all other
		 * characters */
		ଯେତେବେଳେ ((ch = (getDebugChar() & 0x7f)) != '$') {
#ifdef KGDB_DEBUG
			ଯଦି (kdebug)
				putc(ch);
#endif
			;
		}

		checksum = 0;
		xmitcsum = -1;

		ଗଣନା_2 = 0;

		/* now, read until a # or end of buffer is found */
		ଯେତେବେଳେ (ଗଣନା_2 < BUFMAX) {
			ch = getDebugChar() & 0x7f;
			ଯଦି (ch == '#')
				ଭାଙ୍ଗ;
			checksum = checksum + ch;
			buffer[ଗଣନା_2] = ch;
			ଗଣନା_2 = ଗଣନା_2 + 1;
		}

		ଯଦି (ଗଣନା_2 >= BUFMAX)
			ଜାରି;

		buffer[ଗଣନା_2] = 0;

		ଯଦି (ch == '#') {
			xmitcsum = hex(getDebugChar() & 0x7f) << 4;
			xmitcsum |= hex(getDebugChar() & 0x7f);
			ଯଦି (checksum != xmitcsum)
				putDebugChar('-');	/* failed checksum */
			ଅନ୍ୟଥା {
				putDebugChar('+'); /* successful transfer */
				/* if a sequence char is present, reply the ID */
				ଯଦି (buffer[2] == ':') {
					putDebugChar(buffer[0]);
					putDebugChar(buffer[1]);
					/* remove sequence chars from buffer */
					ଗଣନା_2 = strlen(buffer);
					ପାଇଁ (i=3; i <= ଗଣନା_2; i++)
						buffer[i-3] = buffer[i];
				}
			}
		}
	} ଯେତେବେଳେ (checksum != xmitcsum);
}

/* send the packet in buffer.  */
ସ୍ଥିର ଶୂନ୍ୟ
putpacket(unsigned ଅକ୍ଷର *buffer)
{
	unsigned ଅକ୍ଷର checksum;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2;
	unsigned ଅକ୍ଷର ch, recv;

	/*  $<packet info>#<checksum>. */
	କର {
		putDebugChar('$');
		checksum = 0;
		ଗଣନା_2 = 0;

		ଯେତେବେଳେ ((ch = buffer[ଗଣନା_2])) {
			putDebugChar(ch);
			checksum += ch;
			ଗଣନା_2 += 1;
		}

		putDebugChar('#');
		putDebugChar(hexchars[checksum >> 4]);
		putDebugChar(hexchars[checksum & 0xf]);
		recv = getDebugChar();
	} ଯେତେବେଳେ ((recv & 0x7f) != '+');
}

/*
 * This function does all command processing for interfacing to gdb.
 */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
handle_exception (ସଂରଚନା pt_regs *regs)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା addr;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା length;
	ଅକ୍ଷର *ptr;
	kgdb_data kd;
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;

	ଯଦି (!initialized) {
		printf("kgdb: exception before kgdb is initialized! huh?\n");
		ଫେରନ୍ତୁ (0);
	}

	/* probably should check which exception occurred as well */
	ଯଦି (longjmp_on_fault) {
		longjmp_on_fault = 0;
		kgdb_longjmp(error_jmp_buf, KGDBERR_MEMFAULT);
		panic("kgdb longjump failed!\n");
	}

	ଯଦି (kgdb_active) {
		printf("kgdb: unexpected exception from within kgdb\n");
		ଫେରନ୍ତୁ (0);
	}
	kgdb_active = 1;

	kgdb_interruptible(0);

	printf("kgdb: handle_exception; trap [0x%x]\n", kgdb_trap(regs));

	ଯଦି (kgdb_setjmp(error_jmp_buf) != 0)
		panic("kgdb: error or fault in entry init!\n");

	kgdb_enter(regs, &kd);

	entry_regs = *regs;

	ptr = remcomOutBuffer;

	*ptr++ = 'T';

	*ptr++ = hexchars[kd.sigval >> 4];
	*ptr++ = hexchars[kd.sigval & 0xf];

	ପାଇଁ (i = 0; i < kd.nregs; i++) {
		kgdb_reg *rp = &kd.regs[i];

		*ptr++ = hexchars[rp->ଅଙ୍କ >> 4];
		*ptr++ = hexchars[rp->ଅଙ୍କ & 0xf];
		*ptr++ = ':';
		ptr = (ଅକ୍ଷର *)mem2hex((ଅକ୍ଷର *)&rp->val, ptr, 4);
		*ptr++ = ';';
	}

	*ptr = 0;

#ifdef KGDB_DEBUG
	ଯଦି (kdebug)
		printf("kgdb: remcomOutBuffer: %s\n", remcomOutBuffer);
#endif

	putpacket((unsigned ଅକ୍ଷର *)&remcomOutBuffer);

	ଯେତେବେଳେ (1) {
		ଅସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା errnum;

		remcomOutBuffer[0] = 0;

		getpacket(remcomInBuffer);
		ptr = &remcomInBuffer[1];

#ifdef KGDB_DEBUG
		ଯଦି (kdebug)
			printf("kgdb:  remcomInBuffer: %s\n", remcomInBuffer);
#endif

		errnum = kgdb_setjmp(error_jmp_buf);

		ଯଦି (errnum == 0) ବାଛ (remcomInBuffer[0]) {

		ସ୍ଥିତି '?':               /* report most recent signal */
			remcomOutBuffer[0] = 'S';
			remcomOutBuffer[1] = hexchars[kd.sigval >> 4];
			remcomOutBuffer[2] = hexchars[kd.sigval & 0xf];
			remcomOutBuffer[3] = 0;
			ଭାଙ୍ଗ;

#ifdef KGDB_DEBUG
		ସ୍ଥିତି 'd':
			/* toggle debug flag */
			kdebug ^= 1;
			ଭାଙ୍ଗ;
#endif

		ସ୍ଥିତି 'g':	/* return the value of the CPU registers. */
			length = kgdb_getregs(regs, remcomRegBuffer, BUFMAX);
			mem2hex(remcomRegBuffer, remcomOutBuffer, length);
			ଭାଙ୍ଗ;

		ସ୍ଥିତି 'G':   /* set the value of the CPU registers */
			length = strlen(ptr);
			ଯଦି ((length & 1) != 0) kgdb_error(KGDBERR_BADPARAMS);
			hex2mem(ptr, remcomRegBuffer, length/2);
			kgdb_putregs(regs, remcomRegBuffer, length/2);
			strcpy(remcomOutBuffer,"OK");
			ଭାଙ୍ଗ;

		ସ୍ଥିତି 'm':	/* mAA..AA,LLLL  Read LLLL bytes at address AA..AA */
				/* Try to read %x,%x.  */

			ଯଦି (hexToInt(&ptr, &addr)
			    && *ptr++ == ','
			    && hexToInt(&ptr, &length))	{
				mem2hex((ଅକ୍ଷର *)addr, remcomOutBuffer, length);
			} ଅନ୍ୟଥା {
				kgdb_error(KGDBERR_BADPARAMS);
			}
			ଭାଙ୍ଗ;

		ସ୍ଥିତି 'M': /* MAA..AA,LLLL: Write LLLL bytes at address AA.AA return OK */
			/* Try to read '%x,%x:'.  */

			ଯଦି (hexToInt(&ptr, &addr)
			    && *ptr++ == ','
			    && hexToInt(&ptr, &length)
			    && *ptr++ == ':') {
				hex2mem(ptr, (ଅକ୍ଷର *)addr, length);
				strcpy(remcomOutBuffer, "OK");
			} ଅନ୍ୟଥା {
				kgdb_error(KGDBERR_BADPARAMS);
			}
			ଭାଙ୍ଗ;

		ସ୍ଥିତି 'k':    /* kill the program, actually return to monitor */
			kd.extype = KGDBEXIT_KILL;
			*regs = entry_regs;
			ଯାଅ doexit;

		ସ୍ଥିତି 'C':    /* CSS  continue with signal SS */
			*ptr = '\0';	/* ignore the signal number for now */
			/* fall through */

		ସ୍ଥିତି 'c':    /* cAA..AA  Continue; address AA..AA optional */
			/* try to read optional parameter, pc unchanged if no parm */
			kd.extype = KGDBEXIT_CONTINUE;

			ଯଦି (hexToInt(&ptr, &addr)) {
				kd.exaddr = addr;
				kd.extype |= KGDBEXIT_WITHADDR;
			}

			ଯାଅ doexit;

		ସ୍ଥିତି 'S':    /* SSS  single step with signal SS */
			*ptr = '\0';	/* ignore the signal number for now */
			/* fall through */

		ସ୍ଥିତି 's':
			kd.extype = KGDBEXIT_SINGLE;

			ଯଦି (hexToInt(&ptr, &addr)) {
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
			ଫେରନ୍ତୁ (1);

		ସ୍ଥିତି 'r':		/* Reset (if user process..exit ???)*/
			panic("kgdb reset.");
			ଭାଙ୍ଗ;

		ସ୍ଥିତି 'P':    /* Pr=v  set reg r to value v (r and v are hex) */
			ଯଦି (hexToInt(&ptr, &addr)
			    && *ptr++ == '='
			    && ((length = strlen(ptr)) & 1) == 0) {
				hex2mem(ptr, remcomRegBuffer, length/2);
				kgdb_putreg(regs, addr,
					remcomRegBuffer, length/2);
				strcpy(remcomOutBuffer,"OK");
			} ଅନ୍ୟଥା {
				kgdb_error(KGDBERR_BADPARAMS);
			}
			ଭାଙ୍ଗ;
		}			/* switch */

		ଯଦି (errnum != 0)
			sprintf(remcomOutBuffer, "E%02d", errnum);

#ifdef KGDB_DEBUG
		ଯଦି (kdebug)
			printf("kgdb: remcomOutBuffer: %s\n", remcomOutBuffer);
#endif

		/* reply to the request */
		putpacket((unsigned ଅକ୍ଷର *)&remcomOutBuffer);

	} /* while(1) */
}

/*
 * kgdb_init must be called *after* the
 * monitor is relocated into ram
 */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା kgdb_init(ଶୂନ୍ୟ)
{
	puts("KGDB:  ");

	kgdb_serial_init();
	debugger_exception_handler = handle_exception;
	initialized = 1;

	putDebugStr("kgdb ready\n");
	puts("ready\n");

	ଫେରନ୍ତୁ 0;
}

ଶୂନ୍ୟ
kgdb_error(ପୂର୍ଣ୍ଣସଂଖ୍ୟା errnum)
{
	longjmp_on_fault = 0;
	kgdb_longjmp(error_jmp_buf, errnum);
	panic("kgdb_error: longjmp failed!\n");
}

/* Output string in GDB O-packet format if GDB has connected. If nothing
   output, returns 0 (caller must then handle output). */
ପୂର୍ଣ୍ଣସଂଖ୍ୟା
kgdb_output_string (ଅଚଳ ଅକ୍ଷର* s, unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଗଣନା_2)
{
	ଅକ୍ଷର buffer[512];

	ଗଣନା_2 = (ଗଣନା_2 <= (ଆକାର(buffer) / 2 - 2))
		? ଗଣନା_2 : (ଆକାର(buffer) / 2 - 2);

	buffer[0] = 'O';
	mem2hex ((ଅକ୍ଷର *)s, &buffer[1], ଗଣନା_2);
	putpacket((unsigned ଅକ୍ଷର *)&buffer);

	ଫେରନ୍ତୁ 1;
}

ଶୂନ୍ୟ
breakpoint(ଶୂନ୍ୟ)
{
	ଯଦି (!initialized) {
		printf("breakpoint() called b4 kgdb init\n");
		ଫେରନ୍ତୁ;
	}

	kgdb_breakpoint(0, 0);
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା
do_kgdb(ସଂରଚନା cmd_tbl *cmdtp, ପୂର୍ଣ୍ଣସଂଖ୍ୟା flag, ପୂର୍ଣ୍ଣସଂଖ୍ୟା argc, ଅକ୍ଷର *ଅଚଳ argv[])
{
    printf("Entering KGDB mode via exception handler...\n\n");
    kgdb_breakpoint(argc - 1, argv + 1);
    printf("\nReturned from KGDB mode\n");
    ଫେରନ୍ତୁ 0;
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
