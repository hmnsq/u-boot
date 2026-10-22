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

#شامل <asm/ptrace.h>

#شامل <kgdb.h>
#شامل <command.h>

#تعریف_منسوخ KGDB_DEBUG

/*
 * BUFMAX defines the maximum number of characters in inbound/outbound buffers
 */
#تعریف_کرو BUFMAX 1024
ساکن حرف remcom_اندر_بفر[BUFMAX];
ساکن حرف remcom_برآمد_بفر[BUFMAX];
ساکن حرف remcom_Reg_بفر[BUFMAX];

ساکن صحیح initialized = 0;
ساکن صحیح kgdb_فعال;
ساکن ساخت pt_regs اندراج_regs;
ساکن طویل error_jmp_buf[BUFMAX/2];
ساکن صحیح longjmp_on_fault = 0;
#اگر_تعریف_شدہ KGDB_DEBUG
ساکن صحیح kdebug = 1;
#اگر_ختم

ساکن ثابت حرف hexchars[]="0123456789abcdef";

/* Convert ch from a hex digit to an int */
ساکن صحیح
hex(غیرعلامتی حرف ch)
{
	اگر (ch >= 'a' && ch <= 'f')
		واپس ch-'a'+10;
	اگر (ch >= '0' && ch <= '9')
		واپس ch-'0';
	اگر (ch >= 'A' && ch <= 'F')
		واپس ch-'A'+10;
	واپس -1;
}

/* Convert the memory pointed to by mem into hex, placing result in buf.
 * Return a pointer to the last char put in buf (null).
 */
ساکن غیرعلامتی حرف *
mem2hex(حرف *mem, حرف *buf, صحیح گنتی)
{
	حرف *tmp;
	غیرعلامتی حرف ch;

	/*
	 * We use the upper half of buf as an intermediate buffer for the
	 * raw memory copy.  Hex conversion will work against this one.
	 */
	tmp = buf + گنتی;
	longjmp_on_fault = 1;

	حافظہ_نقل(tmp, mem, گنتی);

	جب (گنتی-- > 0) {
		ch = *tmp++;
		*buf++ = hexchars[ch >> 4];
		*buf++ = hexchars[ch & 0xf];
	}
	*buf = 0;
	longjmp_on_fault = 0;
	واپس (غیرعلامتی حرف *)buf;
}

/* convert the hex array pointed to by buf into binary to be placed in mem
 * return a pointer to the character AFTER the last byte fetched from buf.
*/
ساکن حرف *
hex2mem(حرف *buf, حرف *mem, صحیح گنتی)
{
	صحیح hex_قدر;
	حرف *tmp_raw, *tmp_hex;

	/*
	 * We use the upper half of buf as an intermediate buffer for the
	 * raw memory that is converted from hex.
	 */
	tmp_raw = buf + گنتی * 2;
	tmp_hex = tmp_raw - 1;

	longjmp_on_fault = 1;
	جب (tmp_hex >= buf) {
		tmp_raw--;
		hex_قدر = hex(*tmp_hex--);
		اگر (hex_قدر < 0)
			kgdb_error(KGDBERR_NOTHEXDIG);
		*tmp_raw = hex_قدر;
		hex_قدر = hex(*tmp_hex--);
		اگر (hex_قدر < 0)
			kgdb_error(KGDBERR_NOTHEXDIG);
		*tmp_raw |= hex_قدر << 4;

	}

	حافظہ_نقل(mem, tmp_raw, گنتی);

	kgdb_flush_cache_range((خالی *)mem, (خالی *)(mem+گنتی));
	longjmp_on_fault = 0;

	واپس buf;
}

/*
 * While we find nice hex chars, build an int.
 * Return number of chars processed.
 */
ساکن صحیح
hex_مقصد_Int(حرف **ptr, صحیح *int_قدر)
{
	صحیح عدد_Chars = 0;
	صحیح hex_قدر;

	*int_قدر = 0;

	longjmp_on_fault = 1;
	جب (**ptr) {
		hex_قدر = hex(**ptr);
		اگر (hex_قدر < 0)
			توڑو;

		*int_قدر = (*int_قدر << 4) | hex_قدر;
		عدد_Chars ++;

		(*ptr)++;
	}
	longjmp_on_fault = 0;

	واپس (عدد_Chars);
}

/* scan for the sequence $<data>#<checksum>     */
ساکن خالی
getpacket(حرف *بفر)
{
	غیرعلامتی حرف checksum;
	غیرعلامتی حرف xmitcsum;
	صحیح i;
	صحیح گنتی;
	غیرعلامتی حرف ch;

	کرو {
		/* wait around for the start character, ignore all other
		 * characters */
		جب ((ch = (getDebugChar() & 0x7f)) != '$') {
#اگر_تعریف_شدہ KGDB_DEBUG
			اگر (kdebug)
				putc(ch);
#اگر_ختم
			;
		}

		checksum = 0;
		xmitcsum = -1;

		گنتی = 0;

		/* now, read until a # or end of buffer is found */
		جب (گنتی < BUFMAX) {
			ch = getDebugChar() & 0x7f;
			اگر (ch == '#')
				توڑو;
			checksum = checksum + ch;
			بفر[گنتی] = ch;
			گنتی = گنتی + 1;
		}

		اگر (گنتی >= BUFMAX)
			جاری;

		بفر[گنتی] = 0;

		اگر (ch == '#') {
			xmitcsum = hex(getDebugChar() & 0x7f) << 4;
			xmitcsum |= hex(getDebugChar() & 0x7f);
			اگر (checksum != xmitcsum)
				putDebugChar('-');	/* failed checksum */
			ورنہ {
				putDebugChar('+'); /* successful transfer */
				/* if a sequence char is present, reply the ID */
				اگر (بفر[2] == ':') {
					putDebugChar(بفر[0]);
					putDebugChar(بفر[1]);
					/* remove sequence chars from buffer */
					گنتی = لڑی_لمبائی(بفر);
					برائے (i=3; i <= گنتی; i++)
						بفر[i-3] = بفر[i];
				}
			}
		}
	} جب (checksum != xmitcsum);
}

/* send the packet in buffer.  */
ساکن خالی
putpacket(غیرعلامتی حرف *بفر)
{
	غیرعلامتی حرف checksum;
	صحیح گنتی;
	غیرعلامتی حرف ch, recv;

	/*  $<packet info>#<checksum>. */
	کرو {
		putDebugChar('$');
		checksum = 0;
		گنتی = 0;

		جب ((ch = بفر[گنتی])) {
			putDebugChar(ch);
			checksum += ch;
			گنتی += 1;
		}

		putDebugChar('#');
		putDebugChar(hexchars[checksum >> 4]);
		putDebugChar(hexchars[checksum & 0xf]);
		recv = getDebugChar();
	} جب ((recv & 0x7f) != '+');
}

/*
 * This function does all command processing for interfacing to gdb.
 */
ساکن صحیح
handle_exception (ساخت pt_regs *regs)
{
	صحیح addr;
	صحیح لمبائی;
	حرف *ptr;
	kgdb_ڈیٹا kd;
	صحیح i;

	اگر (!initialized) {
		لکھو("kgdb: exception before kgdb is initialized! huh?\n");
		واپس (0);
	}

	/* probably should check which exception occurred as well */
	اگر (longjmp_on_fault) {
		longjmp_on_fault = 0;
		kgdb_longjmp(error_jmp_buf, KGDBERR_MEMFAULT);
		panic("kgdb longjump failed!\n");
	}

	اگر (kgdb_فعال) {
		لکھو("kgdb: unexpected exception from within kgdb\n");
		واپس (0);
	}
	kgdb_فعال = 1;

	kgdb_interruptible(0);

	لکھو("kgdb: handle_exception; trap [0x%x]\n", kgdb_trap(regs));

	اگر (kgdb_setjmp(error_jmp_buf) != 0)
		panic("kgdb: error or fault in entry init!\n");

	kgdb_enter(regs, &kd);

	اندراج_regs = *regs;

	ptr = remcom_برآمد_بفر;

	*ptr++ = 'T';

	*ptr++ = hexchars[kd.sigval >> 4];
	*ptr++ = hexchars[kd.sigval & 0xf];

	برائے (i = 0; i < kd.nregs; i++) {
		kgdb_reg *rp = &kd.regs[i];

		*ptr++ = hexchars[rp->عدد >> 4];
		*ptr++ = hexchars[rp->عدد & 0xf];
		*ptr++ = ':';
		ptr = (حرف *)mem2hex((حرف *)&rp->val, ptr, 4);
		*ptr++ = ';';
	}

	*ptr = 0;

#اگر_تعریف_شدہ KGDB_DEBUG
	اگر (kdebug)
		لکھو("kgdb: remcomOutBuffer: %s\n", remcom_برآمد_بفر);
#اگر_ختم

	putpacket((غیرعلامتی حرف *)&remcom_برآمد_بفر);

	جب (1) {
		متلون صحیح errnum;

		remcom_برآمد_بفر[0] = 0;

		getpacket(remcom_اندر_بفر);
		ptr = &remcom_اندر_بفر[1];

#اگر_تعریف_شدہ KGDB_DEBUG
		اگر (kdebug)
			لکھو("kgdb:  remcomInBuffer: %s\n", remcom_اندر_بفر);
#اگر_ختم

		errnum = kgdb_setjmp(error_jmp_buf);

		اگر (errnum == 0) انتخاب (remcom_اندر_بفر[0]) {

		حالت '?':               /* report most recent signal */
			remcom_برآمد_بفر[0] = 'S';
			remcom_برآمد_بفر[1] = hexchars[kd.sigval >> 4];
			remcom_برآمد_بفر[2] = hexchars[kd.sigval & 0xf];
			remcom_برآمد_بفر[3] = 0;
			توڑو;

#اگر_تعریف_شدہ KGDB_DEBUG
		حالت 'd':
			/* toggle debug flag */
			kdebug ^= 1;
			توڑو;
#اگر_ختم

		حالت 'g':	/* return the value of the CPU registers. */
			لمبائی = kgdb_getregs(regs, remcom_Reg_بفر, BUFMAX);
			mem2hex(remcom_Reg_بفر, remcom_برآمد_بفر, لمبائی);
			توڑو;

		حالت 'G':   /* set the value of the CPU registers */
			لمبائی = لڑی_لمبائی(ptr);
			اگر ((لمبائی & 1) != 0) kgdb_error(KGDBERR_BADPARAMS);
			hex2mem(ptr, remcom_Reg_بفر, لمبائی/2);
			kgdb_putregs(regs, remcom_Reg_بفر, لمبائی/2);
			strcpy(remcom_برآمد_بفر,"OK");
			توڑو;

		حالت 'm':	/* mAA..AA,LLLL  Read LLLL bytes at address AA..AA */
				/* Try to read %x,%x.  */

			اگر (hex_مقصد_Int(&ptr, &addr)
			    && *ptr++ == ','
			    && hex_مقصد_Int(&ptr, &لمبائی))	{
				mem2hex((حرف *)addr, remcom_برآمد_بفر, لمبائی);
			} ورنہ {
				kgdb_error(KGDBERR_BADPARAMS);
			}
			توڑو;

		حالت 'M': /* MAA..AA,LLLL: Write LLLL bytes at address AA.AA return OK */
			/* Try to read '%x,%x:'.  */

			اگر (hex_مقصد_Int(&ptr, &addr)
			    && *ptr++ == ','
			    && hex_مقصد_Int(&ptr, &لمبائی)
			    && *ptr++ == ':') {
				hex2mem(ptr, (حرف *)addr, لمبائی);
				strcpy(remcom_برآمد_بفر, "OK");
			} ورنہ {
				kgdb_error(KGDBERR_BADPARAMS);
			}
			توڑو;

		حالت 'k':    /* kill the program, actually return to monitor */
			kd.extype = KGDBEXIT_KILL;
			*regs = اندراج_regs;
			جاؤ doexit;

		حالت 'C':    /* CSS  continue with signal SS */
			*ptr = '\0';	/* ignore the signal number for now */
			/* fall through */

		حالت 'c':    /* cAA..AA  Continue; address AA..AA optional */
			/* try to read optional parameter, pc unchanged if no parm */
			kd.extype = KGDBEXIT_CONTINUE;

			اگر (hex_مقصد_Int(&ptr, &addr)) {
				kd.exaddr = addr;
				kd.extype |= KGDBEXIT_WITHADDR;
			}

			جاؤ doexit;

		حالت 'S':    /* SSS  single step with signal SS */
			*ptr = '\0';	/* ignore the signal number for now */
			/* fall through */

		حالت 's':
			kd.extype = KGDBEXIT_SINGLE;

			اگر (hex_مقصد_Int(&ptr, &addr)) {
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
			kgdb_فعال = 0;
			kgdb_interruptible(1);
			واپس (1);

		حالت 'r':		/* Reset (if user process..exit ???)*/
			panic("kgdb reset.");
			توڑو;

		حالت 'P':    /* Pr=v  set reg r to value v (r and v are hex) */
			اگر (hex_مقصد_Int(&ptr, &addr)
			    && *ptr++ == '='
			    && ((لمبائی = لڑی_لمبائی(ptr)) & 1) == 0) {
				hex2mem(ptr, remcom_Reg_بفر, لمبائی/2);
				kgdb_putreg(regs, addr,
					remcom_Reg_بفر, لمبائی/2);
				strcpy(remcom_برآمد_بفر,"OK");
			} ورنہ {
				kgdb_error(KGDBERR_BADPARAMS);
			}
			توڑو;
		}			/* switch */

		اگر (errnum != 0)
			sprintf(remcom_برآمد_بفر, "E%02d", errnum);

#اگر_تعریف_شدہ KGDB_DEBUG
		اگر (kdebug)
			لکھو("kgdb: remcomOutBuffer: %s\n", remcom_برآمد_بفر);
#اگر_ختم

		/* reply to the request */
		putpacket((غیرعلامتی حرف *)&remcom_برآمد_بفر);

	} /* while(1) */
}

/*
 * kgdb_init must be called *after* the
 * monitor is relocated into ram
 */
صحیح kgdb_init(خالی)
{
	puts("KGDB:  ");

	kgdb_serial_init();
	debugger_exception_handler = handle_exception;
	initialized = 1;

	putDebugStr("kgdb ready\n");
	puts("ready\n");

	واپس 0;
}

خالی
kgdb_error(صحیح errnum)
{
	longjmp_on_fault = 0;
	kgdb_longjmp(error_jmp_buf, errnum);
	panic("kgdb_error: longjmp failed!\n");
}

/* Output string in GDB O-packet format if GDB has connected. If nothing
   output, returns 0 (caller must then handle output). */
صحیح
kgdb_output_string (ثابت حرف* s, غیرعلامتی صحیح گنتی)
{
	حرف بفر[512];

	گنتی = (گنتی <= (حجم(بفر) / 2 - 2))
		? گنتی : (حجم(بفر) / 2 - 2);

	بفر[0] = 'O';
	mem2hex ((حرف *)s, &بفر[1], گنتی);
	putpacket((غیرعلامتی حرف *)&بفر);

	واپس 1;
}

خالی
breakpoint(خالی)
{
	اگر (!initialized) {
		لکھو("breakpoint() called b4 kgdb init\n");
		واپس;
	}

	kgdb_breakpoint(0, 0);
}

صحیح
do_kgdb(ساخت cmd_tbl *cmdtp, صحیح flag, صحیح argc, حرف *ثابت argv[])
{
    لکھو("Entering KGDB mode via exception handler...\n\n");
    kgdb_breakpoint(argc - 1, argv + 1);
    لکھو("\nReturned from KGDB mode\n");
    واپس 0;
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
