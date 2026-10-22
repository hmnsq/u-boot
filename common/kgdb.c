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

#అన్తర్భావ <asm/ptrace.h>

#అన్తర్భావ <kgdb.h>
#అన్తర్భావ <command.h>

#నిర్వచనం_రద్దు KGDB_DEBUG

/*
 * BUFMAX defines the maximum number of characters in inbound/outbound buffers
 */
#నిర్వచించు BUFMAX 1024
స్థిర అక్షరం remcom_లోపల_బఫరు[BUFMAX];
స్థిర అక్షరం remcom_బహిర్గతం_బఫరు[BUFMAX];
స్థిర అక్షరం remcom_Reg_బఫరు[BUFMAX];

స్థిర పూర్ణాంకం initialized = 0;
స్థిర పూర్ణాంకం kgdb_చురుకైన;
స్థిర నిర్మాణం pt_regs నమోదు_regs;
స్థిర దీర్ఘ error_jmp_buf[BUFMAX/2];
స్థిర పూర్ణాంకం longjmp_on_fault = 0;
#నిర్వచితమైతే KGDB_DEBUG
స్థిర పూర్ణాంకం kdebug = 1;
#అయితే_ముగింపు

స్థిర స్థిరరాశి అక్షరం hexchars[]="0123456789abcdef";

/* Convert ch from a hex digit to an int */
స్థిర పూర్ణాంకం
hex(గుర్తులేని అక్షరం ch)
{
	ఒకవేళ (ch >= 'a' && ch <= 'f')
		తిరిగి ch-'a'+10;
	ఒకవేళ (ch >= '0' && ch <= '9')
		తిరిగి ch-'0';
	ఒకవేళ (ch >= 'A' && ch <= 'F')
		తిరిగి ch-'A'+10;
	తిరిగి -1;
}

/* Convert the memory pointed to by mem into hex, placing result in buf.
 * Return a pointer to the last char put in buf (null).
 */
స్థిర గుర్తులేని అక్షరం *
mem2hex(అక్షరం *mem, అక్షరం *buf, పూర్ణాంకం లెక్క)
{
	అక్షరం *tmp;
	గుర్తులేని అక్షరం ch;

	/*
	 * We use the upper half of buf as an intermediate buffer for the
	 * raw memory copy.  Hex conversion will work against this one.
	 */
	tmp = buf + లెక్క;
	longjmp_on_fault = 1;

	జ్ఞాపకం_నకలు(tmp, mem, లెక్క);

	అయితే (లెక్క-- > 0) {
		ch = *tmp++;
		*buf++ = hexchars[ch >> 4];
		*buf++ = hexchars[ch & 0xf];
	}
	*buf = 0;
	longjmp_on_fault = 0;
	తిరిగి (గుర్తులేని అక్షరం *)buf;
}

/* convert the hex array pointed to by buf into binary to be placed in mem
 * return a pointer to the character AFTER the last byte fetched from buf.
*/
స్థిర అక్షరం *
hex2mem(అక్షరం *buf, అక్షరం *mem, పూర్ణాంకం లెక్క)
{
	పూర్ణాంకం hex_విలువ;
	అక్షరం *tmp_raw, *tmp_hex;

	/*
	 * We use the upper half of buf as an intermediate buffer for the
	 * raw memory that is converted from hex.
	 */
	tmp_raw = buf + లెక్క * 2;
	tmp_hex = tmp_raw - 1;

	longjmp_on_fault = 1;
	అయితే (tmp_hex >= buf) {
		tmp_raw--;
		hex_విలువ = hex(*tmp_hex--);
		ఒకవేళ (hex_విలువ < 0)
			kgdb_error(KGDBERR_NOTHEXDIG);
		*tmp_raw = hex_విలువ;
		hex_విలువ = hex(*tmp_hex--);
		ఒకవేళ (hex_విలువ < 0)
			kgdb_error(KGDBERR_NOTHEXDIG);
		*tmp_raw |= hex_విలువ << 4;

	}

	జ్ఞాపకం_నకలు(mem, tmp_raw, లెక్క);

	kgdb_flush_cache_range((శూన్య *)mem, (శూన్య *)(mem+లెక్క));
	longjmp_on_fault = 0;

	తిరిగి buf;
}

/*
 * While we find nice hex chars, build an int.
 * Return number of chars processed.
 */
స్థిర పూర్ణాంకం
hex_గమ్యం_Int(అక్షరం **ptr, పూర్ణాంకం *int_విలువ)
{
	పూర్ణాంకం అంకె_Chars = 0;
	పూర్ణాంకం hex_విలువ;

	*int_విలువ = 0;

	longjmp_on_fault = 1;
	అయితే (**ptr) {
		hex_విలువ = hex(**ptr);
		ఒకవేళ (hex_విలువ < 0)
			ఆపు;

		*int_విలువ = (*int_విలువ << 4) | hex_విలువ;
		అంకె_Chars ++;

		(*ptr)++;
	}
	longjmp_on_fault = 0;

	తిరిగి (అంకె_Chars);
}

/* scan for the sequence $<data>#<checksum>     */
స్థిర శూన్య
getpacket(అక్షరం *బఫరు)
{
	గుర్తులేని అక్షరం checksum;
	గుర్తులేని అక్షరం xmitcsum;
	పూర్ణాంకం i;
	పూర్ణాంకం లెక్క;
	గుర్తులేని అక్షరం ch;

	చేయి {
		/* wait around for the start character, ignore all other
		 * characters */
		అయితే ((ch = (getDebugChar() & 0x7f)) != '$') {
#నిర్వచితమైతే KGDB_DEBUG
			ఒకవేళ (kdebug)
				putc(ch);
#అయితే_ముగింపు
			;
		}

		checksum = 0;
		xmitcsum = -1;

		లెక్క = 0;

		/* now, read until a # or end of buffer is found */
		అయితే (లెక్క < BUFMAX) {
			ch = getDebugChar() & 0x7f;
			ఒకవేళ (ch == '#')
				ఆపు;
			checksum = checksum + ch;
			బఫరు[లెక్క] = ch;
			లెక్క = లెక్క + 1;
		}

		ఒకవేళ (లెక్క >= BUFMAX)
			కొనసాగు;

		బఫరు[లెక్క] = 0;

		ఒకవేళ (ch == '#') {
			xmitcsum = hex(getDebugChar() & 0x7f) << 4;
			xmitcsum |= hex(getDebugChar() & 0x7f);
			ఒకవేళ (checksum != xmitcsum)
				putDebugChar('-');	/* failed checksum */
			లేకపోతే {
				putDebugChar('+'); /* successful transfer */
				/* if a sequence char is present, reply the ID */
				ఒకవేళ (బఫరు[2] == ':') {
					putDebugChar(బఫరు[0]);
					putDebugChar(బఫరు[1]);
					/* remove sequence chars from buffer */
					లెక్క = అక్షరశ్రేణి_పొడవు(బఫరు);
					కొరకు (i=3; i <= లెక్క; i++)
						బఫరు[i-3] = బఫరు[i];
				}
			}
		}
	} అయితే (checksum != xmitcsum);
}

/* send the packet in buffer.  */
స్థిర శూన్య
putpacket(గుర్తులేని అక్షరం *బఫరు)
{
	గుర్తులేని అక్షరం checksum;
	పూర్ణాంకం లెక్క;
	గుర్తులేని అక్షరం ch, recv;

	/*  $<packet info>#<checksum>. */
	చేయి {
		putDebugChar('$');
		checksum = 0;
		లెక్క = 0;

		అయితే ((ch = బఫరు[లెక్క])) {
			putDebugChar(ch);
			checksum += ch;
			లెక్క += 1;
		}

		putDebugChar('#');
		putDebugChar(hexchars[checksum >> 4]);
		putDebugChar(hexchars[checksum & 0xf]);
		recv = getDebugChar();
	} అయితే ((recv & 0x7f) != '+');
}

/*
 * This function does all command processing for interfacing to gdb.
 */
స్థిర పూర్ణాంకం
handle_exception (నిర్మాణం pt_regs *regs)
{
	పూర్ణాంకం addr;
	పూర్ణాంకం నిడివి;
	అక్షరం *ptr;
	kgdb_దత్తాంశం kd;
	పూర్ణాంకం i;

	ఒకవేళ (!initialized) {
		రాయి("kgdb: exception before kgdb is initialized! huh?\n");
		తిరిగి (0);
	}

	/* probably should check which exception occurred as well */
	ఒకవేళ (longjmp_on_fault) {
		longjmp_on_fault = 0;
		kgdb_longjmp(error_jmp_buf, KGDBERR_MEMFAULT);
		panic("kgdb longjump failed!\n");
	}

	ఒకవేళ (kgdb_చురుకైన) {
		రాయి("kgdb: unexpected exception from within kgdb\n");
		తిరిగి (0);
	}
	kgdb_చురుకైన = 1;

	kgdb_interruptible(0);

	రాయి("kgdb: handle_exception; trap [0x%x]\n", kgdb_trap(regs));

	ఒకవేళ (kgdb_setjmp(error_jmp_buf) != 0)
		panic("kgdb: error or fault in entry init!\n");

	kgdb_enter(regs, &kd);

	నమోదు_regs = *regs;

	ptr = remcom_బహిర్గతం_బఫరు;

	*ptr++ = 'T';

	*ptr++ = hexchars[kd.sigval >> 4];
	*ptr++ = hexchars[kd.sigval & 0xf];

	కొరకు (i = 0; i < kd.nregs; i++) {
		kgdb_reg *rp = &kd.regs[i];

		*ptr++ = hexchars[rp->అంకె >> 4];
		*ptr++ = hexchars[rp->అంకె & 0xf];
		*ptr++ = ':';
		ptr = (అక్షరం *)mem2hex((అక్షరం *)&rp->val, ptr, 4);
		*ptr++ = ';';
	}

	*ptr = 0;

#నిర్వచితమైతే KGDB_DEBUG
	ఒకవేళ (kdebug)
		రాయి("kgdb: remcomOutBuffer: %s\n", remcom_బహిర్గతం_బఫరు);
#అయితే_ముగింపు

	putpacket((గుర్తులేని అక్షరం *)&remcom_బహిర్గతం_బఫరు);

	అయితే (1) {
		అస్థిర పూర్ణాంకం errnum;

		remcom_బహిర్గతం_బఫరు[0] = 0;

		getpacket(remcom_లోపల_బఫరు);
		ptr = &remcom_లోపల_బఫరు[1];

#నిర్వచితమైతే KGDB_DEBUG
		ఒకవేళ (kdebug)
			రాయి("kgdb:  remcomInBuffer: %s\n", remcom_లోపల_బఫరు);
#అయితే_ముగింపు

		errnum = kgdb_setjmp(error_jmp_buf);

		ఒకవేళ (errnum == 0) ఎంపిక (remcom_లోపల_బఫరు[0]) {

		స్థితి '?':               /* report most recent signal */
			remcom_బహిర్గతం_బఫరు[0] = 'S';
			remcom_బహిర్గతం_బఫరు[1] = hexchars[kd.sigval >> 4];
			remcom_బహిర్గతం_బఫరు[2] = hexchars[kd.sigval & 0xf];
			remcom_బహిర్గతం_బఫరు[3] = 0;
			ఆపు;

#నిర్వచితమైతే KGDB_DEBUG
		స్థితి 'd':
			/* toggle debug flag */
			kdebug ^= 1;
			ఆపు;
#అయితే_ముగింపు

		స్థితి 'g':	/* return the value of the CPU registers. */
			నిడివి = kgdb_getregs(regs, remcom_Reg_బఫరు, BUFMAX);
			mem2hex(remcom_Reg_బఫరు, remcom_బహిర్గతం_బఫరు, నిడివి);
			ఆపు;

		స్థితి 'G':   /* set the value of the CPU registers */
			నిడివి = అక్షరశ్రేణి_పొడవు(ptr);
			ఒకవేళ ((నిడివి & 1) != 0) kgdb_error(KGDBERR_BADPARAMS);
			hex2mem(ptr, remcom_Reg_బఫరు, నిడివి/2);
			kgdb_putregs(regs, remcom_Reg_బఫరు, నిడివి/2);
			strcpy(remcom_బహిర్గతం_బఫరు,"OK");
			ఆపు;

		స్థితి 'm':	/* mAA..AA,LLLL  Read LLLL bytes at address AA..AA */
				/* Try to read %x,%x.  */

			ఒకవేళ (hex_గమ్యం_Int(&ptr, &addr)
			    && *ptr++ == ','
			    && hex_గమ్యం_Int(&ptr, &నిడివి))	{
				mem2hex((అక్షరం *)addr, remcom_బహిర్గతం_బఫరు, నిడివి);
			} లేకపోతే {
				kgdb_error(KGDBERR_BADPARAMS);
			}
			ఆపు;

		స్థితి 'M': /* MAA..AA,LLLL: Write LLLL bytes at address AA.AA return OK */
			/* Try to read '%x,%x:'.  */

			ఒకవేళ (hex_గమ్యం_Int(&ptr, &addr)
			    && *ptr++ == ','
			    && hex_గమ్యం_Int(&ptr, &నిడివి)
			    && *ptr++ == ':') {
				hex2mem(ptr, (అక్షరం *)addr, నిడివి);
				strcpy(remcom_బహిర్గతం_బఫరు, "OK");
			} లేకపోతే {
				kgdb_error(KGDBERR_BADPARAMS);
			}
			ఆపు;

		స్థితి 'k':    /* kill the program, actually return to monitor */
			kd.extype = KGDBEXIT_KILL;
			*regs = నమోదు_regs;
			వెళ్ళు doexit;

		స్థితి 'C':    /* CSS  continue with signal SS */
			*ptr = '\0';	/* ignore the signal number for now */
			/* fall through */

		స్థితి 'c':    /* cAA..AA  Continue; address AA..AA optional */
			/* try to read optional parameter, pc unchanged if no parm */
			kd.extype = KGDBEXIT_CONTINUE;

			ఒకవేళ (hex_గమ్యం_Int(&ptr, &addr)) {
				kd.exaddr = addr;
				kd.extype |= KGDBEXIT_WITHADDR;
			}

			వెళ్ళు doexit;

		స్థితి 'S':    /* SSS  single step with signal SS */
			*ptr = '\0';	/* ignore the signal number for now */
			/* fall through */

		స్థితి 's':
			kd.extype = KGDBEXIT_SINGLE;

			ఒకవేళ (hex_గమ్యం_Int(&ptr, &addr)) {
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
			kgdb_చురుకైన = 0;
			kgdb_interruptible(1);
			తిరిగి (1);

		స్థితి 'r':		/* Reset (if user process..exit ???)*/
			panic("kgdb reset.");
			ఆపు;

		స్థితి 'P':    /* Pr=v  set reg r to value v (r and v are hex) */
			ఒకవేళ (hex_గమ్యం_Int(&ptr, &addr)
			    && *ptr++ == '='
			    && ((నిడివి = అక్షరశ్రేణి_పొడవు(ptr)) & 1) == 0) {
				hex2mem(ptr, remcom_Reg_బఫరు, నిడివి/2);
				kgdb_putreg(regs, addr,
					remcom_Reg_బఫరు, నిడివి/2);
				strcpy(remcom_బహిర్గతం_బఫరు,"OK");
			} లేకపోతే {
				kgdb_error(KGDBERR_BADPARAMS);
			}
			ఆపు;
		}			/* switch */

		ఒకవేళ (errnum != 0)
			sprintf(remcom_బహిర్గతం_బఫరు, "E%02d", errnum);

#నిర్వచితమైతే KGDB_DEBUG
		ఒకవేళ (kdebug)
			రాయి("kgdb: remcomOutBuffer: %s\n", remcom_బహిర్గతం_బఫరు);
#అయితే_ముగింపు

		/* reply to the request */
		putpacket((గుర్తులేని అక్షరం *)&remcom_బహిర్గతం_బఫరు);

	} /* while(1) */
}

/*
 * kgdb_init must be called *after* the
 * monitor is relocated into ram
 */
పూర్ణాంకం kgdb_init(శూన్య)
{
	puts("KGDB:  ");

	kgdb_serial_init();
	debugger_exception_handler = handle_exception;
	initialized = 1;

	putDebugStr("kgdb ready\n");
	puts("ready\n");

	తిరిగి 0;
}

శూన్య
kgdb_error(పూర్ణాంకం errnum)
{
	longjmp_on_fault = 0;
	kgdb_longjmp(error_jmp_buf, errnum);
	panic("kgdb_error: longjmp failed!\n");
}

/* Output string in GDB O-packet format if GDB has connected. If nothing
   output, returns 0 (caller must then handle output). */
పూర్ణాంకం
kgdb_output_string (స్థిరరాశి అక్షరం* s, గుర్తులేని పూర్ణాంకం లెక్క)
{
	అక్షరం బఫరు[512];

	లెక్క = (లెక్క <= (పరిమాణం(బఫరు) / 2 - 2))
		? లెక్క : (పరిమాణం(బఫరు) / 2 - 2);

	బఫరు[0] = 'O';
	mem2hex ((అక్షరం *)s, &బఫరు[1], లెక్క);
	putpacket((గుర్తులేని అక్షరం *)&బఫరు);

	తిరిగి 1;
}

శూన్య
breakpoint(శూన్య)
{
	ఒకవేళ (!initialized) {
		రాయి("breakpoint() called b4 kgdb init\n");
		తిరిగి;
	}

	kgdb_breakpoint(0, 0);
}

పూర్ణాంకం
do_kgdb(నిర్మాణం cmd_tbl *cmdtp, పూర్ణాంకం flag, పూర్ణాంకం argc, అక్షరం *స్థిరరాశి argv[])
{
    రాయి("Entering KGDB mode via exception handler...\n\n");
    kgdb_breakpoint(argc - 1, argv + 1);
    రాయి("\nReturned from KGDB mode\n");
    తిరిగి 0;
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
