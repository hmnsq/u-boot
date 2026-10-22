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

#অন্তর্ভাব <asm/ptrace.h>

#অন্তর্ভাব <kgdb.h>
#অন্তর্ভাব <command.h>

#undef KGDB_DEBUG

/*
 * BUFMAX defines the maximum number of characters in inbound/outbound buffers
 */
#define BUFMAX 1024
স্থির অক্ষর remcom_In_বাফার[BUFMAX];
স্থির অক্ষর remcom_Out_বাফার[BUFMAX];
স্থির অক্ষর remcom_Reg_বাফার[BUFMAX];

স্থির পূর্ণসংখ্যা initialized = 0;
স্থির পূর্ণসংখ্যা kgdb_active;
স্থির কাঠামো pt_regs entry_regs;
স্থির long error_jmp_buf[BUFMAX/2];
স্থির পূর্ণসংখ্যা longjmp_on_fault = 0;
#ifdef KGDB_DEBUG
স্থির পূর্ণসংখ্যা kdebug = 1;
#endif

স্থির ধ্রুবক অক্ষর hexchars[]="0123456789abcdef";

/* Convert ch from a hex digit to an int */
স্থির পূর্ণসংখ্যা
hex(unsigned অক্ষর ch)
{
	যদি (ch >= 'a' && ch <= 'f')
		ফেরত ch-'a'+10;
	যদি (ch >= '0' && ch <= '9')
		ফেরত ch-'0';
	যদি (ch >= 'A' && ch <= 'F')
		ফেরত ch-'A'+10;
	ফেরত -1;
}

/* Convert the memory pointed to by mem into hex, placing result in buf.
 * Return a pointer to the last char put in buf (null).
 */
স্থির unsigned অক্ষর *
mem2hex(অক্ষর *mem, অক্ষর *buf, পূর্ণসংখ্যা গুনতি)
{
	অক্ষর *tmp;
	unsigned অক্ষর ch;

	/*
	 * We use the upper half of buf as an intermediate buffer for the
	 * raw memory copy.  Hex conversion will work against this one.
	 */
	tmp = buf + গুনতি;
	longjmp_on_fault = 1;

	memcpy(tmp, mem, গুনতি);

	যখন (গুনতি-- > 0) {
		ch = *tmp++;
		*buf++ = hexchars[ch >> 4];
		*buf++ = hexchars[ch & 0xf];
	}
	*buf = 0;
	longjmp_on_fault = 0;
	ফেরত (unsigned অক্ষর *)buf;
}

/* convert the hex array pointed to by buf into binary to be placed in mem
 * return a pointer to the character AFTER the last byte fetched from buf.
*/
স্থির অক্ষর *
hex2mem(অক্ষর *buf, অক্ষর *mem, পূর্ণসংখ্যা গুনতি)
{
	পূর্ণসংখ্যা hex_মান;
	অক্ষর *tmp_raw, *tmp_hex;

	/*
	 * We use the upper half of buf as an intermediate buffer for the
	 * raw memory that is converted from hex.
	 */
	tmp_raw = buf + গুনতি * 2;
	tmp_hex = tmp_raw - 1;

	longjmp_on_fault = 1;
	যখন (tmp_hex >= buf) {
		tmp_raw--;
		hex_মান = hex(*tmp_hex--);
		যদি (hex_মান < 0)
			kgdb_error(KGDBERR_NOTHEXDIG);
		*tmp_raw = hex_মান;
		hex_মান = hex(*tmp_hex--);
		যদি (hex_মান < 0)
			kgdb_error(KGDBERR_NOTHEXDIG);
		*tmp_raw |= hex_মান << 4;

	}

	memcpy(mem, tmp_raw, গুনতি);

	kgdb_flush_cache_range((শূন্য *)mem, (শূন্য *)(mem+গুনতি));
	longjmp_on_fault = 0;

	ফেরত buf;
}

/*
 * While we find nice hex chars, build an int.
 * Return number of chars processed.
 */
স্থির পূর্ণসংখ্যা
hexToInt(অক্ষর **ptr, পূর্ণসংখ্যা *int_মান)
{
	পূর্ণসংখ্যা অঙ্ক_Chars = 0;
	পূর্ণসংখ্যা hex_মান;

	*int_মান = 0;

	longjmp_on_fault = 1;
	যখন (**ptr) {
		hex_মান = hex(**ptr);
		যদি (hex_মান < 0)
			বিরতি;

		*int_মান = (*int_মান << 4) | hex_মান;
		অঙ্ক_Chars ++;

		(*ptr)++;
	}
	longjmp_on_fault = 0;

	ফেরত (অঙ্ক_Chars);
}

/* scan for the sequence $<data>#<checksum>     */
স্থির শূন্য
getpacket(অক্ষর *বাফার)
{
	unsigned অক্ষর checksum;
	unsigned অক্ষর xmitcsum;
	পূর্ণসংখ্যা i;
	পূর্ণসংখ্যা গুনতি;
	unsigned অক্ষর ch;

	করো {
		/* wait around for the start character, ignore all other
		 * characters */
		যখন ((ch = (getDebugChar() & 0x7f)) != '$') {
#ifdef KGDB_DEBUG
			যদি (kdebug)
				putc(ch);
#endif
			;
		}

		checksum = 0;
		xmitcsum = -1;

		গুনতি = 0;

		/* now, read until a # or end of buffer is found */
		যখন (গুনতি < BUFMAX) {
			ch = getDebugChar() & 0x7f;
			যদি (ch == '#')
				বিরতি;
			checksum = checksum + ch;
			বাফার[গুনতি] = ch;
			গুনতি = গুনতি + 1;
		}

		যদি (গুনতি >= BUFMAX)
			চালু;

		বাফার[গুনতি] = 0;

		যদি (ch == '#') {
			xmitcsum = hex(getDebugChar() & 0x7f) << 4;
			xmitcsum |= hex(getDebugChar() & 0x7f);
			যদি (checksum != xmitcsum)
				putDebugChar('-');	/* failed checksum */
			অন্যথা {
				putDebugChar('+'); /* successful transfer */
				/* if a sequence char is present, reply the ID */
				যদি (বাফার[2] == ':') {
					putDebugChar(বাফার[0]);
					putDebugChar(বাফার[1]);
					/* remove sequence chars from buffer */
					গুনতি = strlen(বাফার);
					জন্য (i=3; i <= গুনতি; i++)
						বাফার[i-3] = বাফার[i];
				}
			}
		}
	} যখন (checksum != xmitcsum);
}

/* send the packet in buffer.  */
স্থির শূন্য
putpacket(unsigned অক্ষর *বাফার)
{
	unsigned অক্ষর checksum;
	পূর্ণসংখ্যা গুনতি;
	unsigned অক্ষর ch, recv;

	/*  $<packet info>#<checksum>. */
	করো {
		putDebugChar('$');
		checksum = 0;
		গুনতি = 0;

		যখন ((ch = বাফার[গুনতি])) {
			putDebugChar(ch);
			checksum += ch;
			গুনতি += 1;
		}

		putDebugChar('#');
		putDebugChar(hexchars[checksum >> 4]);
		putDebugChar(hexchars[checksum & 0xf]);
		recv = getDebugChar();
	} যখন ((recv & 0x7f) != '+');
}

/*
 * This function does all command processing for interfacing to gdb.
 */
স্থির পূর্ণসংখ্যা
handle_exception (কাঠামো pt_regs *regs)
{
	পূর্ণসংখ্যা addr;
	পূর্ণসংখ্যা দৈর্ঘ্য;
	অক্ষর *ptr;
	kgdb_তথ্য kd;
	পূর্ণসংখ্যা i;

	যদি (!initialized) {
		printf("kgdb: exception before kgdb is initialized! huh?\n");
		ফেরত (0);
	}

	/* probably should check which exception occurred as well */
	যদি (longjmp_on_fault) {
		longjmp_on_fault = 0;
		kgdb_longjmp(error_jmp_buf, KGDBERR_MEMFAULT);
		panic("kgdb longjump failed!\n");
	}

	যদি (kgdb_active) {
		printf("kgdb: unexpected exception from within kgdb\n");
		ফেরত (0);
	}
	kgdb_active = 1;

	kgdb_interruptible(0);

	printf("kgdb: handle_exception; trap [0x%x]\n", kgdb_trap(regs));

	যদি (kgdb_setjmp(error_jmp_buf) != 0)
		panic("kgdb: error or fault in entry init!\n");

	kgdb_enter(regs, &kd);

	entry_regs = *regs;

	ptr = remcom_Out_বাফার;

	*ptr++ = 'T';

	*ptr++ = hexchars[kd.sigval >> 4];
	*ptr++ = hexchars[kd.sigval & 0xf];

	জন্য (i = 0; i < kd.nregs; i++) {
		kgdb_reg *rp = &kd.regs[i];

		*ptr++ = hexchars[rp->অঙ্ক >> 4];
		*ptr++ = hexchars[rp->অঙ্ক & 0xf];
		*ptr++ = ':';
		ptr = (অক্ষর *)mem2hex((অক্ষর *)&rp->val, ptr, 4);
		*ptr++ = ';';
	}

	*ptr = 0;

#ifdef KGDB_DEBUG
	যদি (kdebug)
		printf("kgdb: remcomOutBuffer: %s\n", remcom_Out_বাফার);
#endif

	putpacket((unsigned অক্ষর *)&remcom_Out_বাফার);

	যখন (1) {
		অস্থির পূর্ণসংখ্যা errnum;

		remcom_Out_বাফার[0] = 0;

		getpacket(remcom_In_বাফার);
		ptr = &remcom_In_বাফার[1];

#ifdef KGDB_DEBUG
		যদি (kdebug)
			printf("kgdb:  remcomInBuffer: %s\n", remcom_In_বাফার);
#endif

		errnum = kgdb_setjmp(error_jmp_buf);

		যদি (errnum == 0) নির্বাচন (remcom_In_বাফার[0]) {

		ক্ষেত্র '?':               /* report most recent signal */
			remcom_Out_বাফার[0] = 'S';
			remcom_Out_বাফার[1] = hexchars[kd.sigval >> 4];
			remcom_Out_বাফার[2] = hexchars[kd.sigval & 0xf];
			remcom_Out_বাফার[3] = 0;
			বিরতি;

#ifdef KGDB_DEBUG
		ক্ষেত্র 'd':
			/* toggle debug flag */
			kdebug ^= 1;
			বিরতি;
#endif

		ক্ষেত্র 'g':	/* return the value of the CPU registers. */
			দৈর্ঘ্য = kgdb_getregs(regs, remcom_Reg_বাফার, BUFMAX);
			mem2hex(remcom_Reg_বাফার, remcom_Out_বাফার, দৈর্ঘ্য);
			বিরতি;

		ক্ষেত্র 'G':   /* set the value of the CPU registers */
			দৈর্ঘ্য = strlen(ptr);
			যদি ((দৈর্ঘ্য & 1) != 0) kgdb_error(KGDBERR_BADPARAMS);
			hex2mem(ptr, remcom_Reg_বাফার, দৈর্ঘ্য/2);
			kgdb_putregs(regs, remcom_Reg_বাফার, দৈর্ঘ্য/2);
			strcpy(remcom_Out_বাফার,"OK");
			বিরতি;

		ক্ষেত্র 'm':	/* mAA..AA,LLLL  Read LLLL bytes at address AA..AA */
				/* Try to read %x,%x.  */

			যদি (hexToInt(&ptr, &addr)
			    && *ptr++ == ','
			    && hexToInt(&ptr, &দৈর্ঘ্য))	{
				mem2hex((অক্ষর *)addr, remcom_Out_বাফার, দৈর্ঘ্য);
			} অন্যথা {
				kgdb_error(KGDBERR_BADPARAMS);
			}
			বিরতি;

		ক্ষেত্র 'M': /* MAA..AA,LLLL: Write LLLL bytes at address AA.AA return OK */
			/* Try to read '%x,%x:'.  */

			যদি (hexToInt(&ptr, &addr)
			    && *ptr++ == ','
			    && hexToInt(&ptr, &দৈর্ঘ্য)
			    && *ptr++ == ':') {
				hex2mem(ptr, (অক্ষর *)addr, দৈর্ঘ্য);
				strcpy(remcom_Out_বাফার, "OK");
			} অন্যথা {
				kgdb_error(KGDBERR_BADPARAMS);
			}
			বিরতি;

		ক্ষেত্র 'k':    /* kill the program, actually return to monitor */
			kd.extype = KGDBEXIT_KILL;
			*regs = entry_regs;
			যাও doexit;

		ক্ষেত্র 'C':    /* CSS  continue with signal SS */
			*ptr = '\0';	/* ignore the signal number for now */
			/* fall through */

		ক্ষেত্র 'c':    /* cAA..AA  Continue; address AA..AA optional */
			/* try to read optional parameter, pc unchanged if no parm */
			kd.extype = KGDBEXIT_CONTINUE;

			যদি (hexToInt(&ptr, &addr)) {
				kd.exaddr = addr;
				kd.extype |= KGDBEXIT_WITHADDR;
			}

			যাও doexit;

		ক্ষেত্র 'S':    /* SSS  single step with signal SS */
			*ptr = '\0';	/* ignore the signal number for now */
			/* fall through */

		ক্ষেত্র 's':
			kd.extype = KGDBEXIT_SINGLE;

			যদি (hexToInt(&ptr, &addr)) {
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
			ফেরত (1);

		ক্ষেত্র 'r':		/* Reset (if user process..exit ???)*/
			panic("kgdb reset.");
			বিরতি;

		ক্ষেত্র 'P':    /* Pr=v  set reg r to value v (r and v are hex) */
			যদি (hexToInt(&ptr, &addr)
			    && *ptr++ == '='
			    && ((দৈর্ঘ্য = strlen(ptr)) & 1) == 0) {
				hex2mem(ptr, remcom_Reg_বাফার, দৈর্ঘ্য/2);
				kgdb_putreg(regs, addr,
					remcom_Reg_বাফার, দৈর্ঘ্য/2);
				strcpy(remcom_Out_বাফার,"OK");
			} অন্যথা {
				kgdb_error(KGDBERR_BADPARAMS);
			}
			বিরতি;
		}			/* switch */

		যদি (errnum != 0)
			sprintf(remcom_Out_বাফার, "E%02d", errnum);

#ifdef KGDB_DEBUG
		যদি (kdebug)
			printf("kgdb: remcomOutBuffer: %s\n", remcom_Out_বাফার);
#endif

		/* reply to the request */
		putpacket((unsigned অক্ষর *)&remcom_Out_বাফার);

	} /* while(1) */
}

/*
 * kgdb_init must be called *after* the
 * monitor is relocated into ram
 */
পূর্ণসংখ্যা kgdb_init(শূন্য)
{
	puts("KGDB:  ");

	kgdb_serial_init();
	debugger_exception_handler = handle_exception;
	initialized = 1;

	putDebugStr("kgdb ready\n");
	puts("ready\n");

	ফেরত 0;
}

শূন্য
kgdb_error(পূর্ণসংখ্যা errnum)
{
	longjmp_on_fault = 0;
	kgdb_longjmp(error_jmp_buf, errnum);
	panic("kgdb_error: longjmp failed!\n");
}

/* Output string in GDB O-packet format if GDB has connected. If nothing
   output, returns 0 (caller must then handle output). */
পূর্ণসংখ্যা
kgdb_output_string (ধ্রুবক অক্ষর* s, unsigned পূর্ণসংখ্যা গুনতি)
{
	অক্ষর বাফার[512];

	গুনতি = (গুনতি <= (আকার(বাফার) / 2 - 2))
		? গুনতি : (আকার(বাফার) / 2 - 2);

	বাফার[0] = 'O';
	mem2hex ((অক্ষর *)s, &বাফার[1], গুনতি);
	putpacket((unsigned অক্ষর *)&বাফার);

	ফেরত 1;
}

শূন্য
breakpoint(শূন্য)
{
	যদি (!initialized) {
		printf("breakpoint() called b4 kgdb init\n");
		ফেরত;
	}

	kgdb_breakpoint(0, 0);
}

পূর্ণসংখ্যা
do_kgdb(কাঠামো cmd_tbl *cmdtp, পূর্ণসংখ্যা flag, পূর্ণসংখ্যা argc, অক্ষর *ধ্রুবক argv[])
{
    printf("Entering KGDB mode via exception handler...\n\n");
    kgdb_breakpoint(argc - 1, argv + 1);
    printf("\nReturned from KGDB mode\n");
    ফেরত 0;
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
