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

#ضمن <asm/ptrace.h>

#ضمن <kgdb.h>
#ضمن <command.h>

#وصف_رد KGDB_DEBUG

/*
 * BUFMAX defines the maximum number of characters in inbound/outbound buffers
 */
#وصف_ڪريو BUFMAX 1024
ساكن حرف remcom_اندريون_بفر[BUFMAX];
ساكن حرف remcom_ٻاهريون_بفر[BUFMAX];
ساكن حرف remcom_Reg_بفر[BUFMAX];

ساكن صحيح initialized = 0;
ساكن صحيح kgdb_سرگرم;
ساكن بنية pt_regs داخلا_regs;
ساكن ڊگهو error_jmp_buf[BUFMAX/2];
ساكن صحيح longjmp_on_fault = 0;
#جيڪڏهن_وصف_ٿيل KGDB_DEBUG
ساكن صحيح kdebug = 1;
#جيڪڏهن_پورو

ساكن ثابت حرف hexchars[]="0123456789abcdef";

/* Convert ch from a hex digit to an int */
ساكن صحيح
hex(بنانشان حرف ch)
{
	إذا (ch >= 'a' && ch <= 'f')
		أرجع ch-'a'+10;
	إذا (ch >= '0' && ch <= '9')
		أرجع ch-'0';
	إذا (ch >= 'A' && ch <= 'F')
		أرجع ch-'A'+10;
	أرجع -1;
}

/* Convert the memory pointed to by mem into hex, placing result in buf.
 * Return a pointer to the last char put in buf (null).
 */
ساكن بنانشان حرف *
mem2hex(حرف *mem, حرف *buf, صحيح ڳڻپ)
{
	حرف *tmp;
	بنانشان حرف ch;

	/*
	 * We use the upper half of buf as an intermediate buffer for the
	 * raw memory copy.  Hex conversion will work against this one.
	 */
	tmp = buf + ڳڻپ;
	longjmp_on_fault = 1;

	يادگيري_نقل(tmp, mem, ڳڻپ);

	طالما (ڳڻپ-- > 0) {
		ch = *tmp++;
		*buf++ = hexchars[ch >> 4];
		*buf++ = hexchars[ch & 0xf];
	}
	*buf = 0;
	longjmp_on_fault = 0;
	أرجع (بنانشان حرف *)buf;
}

/* convert the hex array pointed to by buf into binary to be placed in mem
 * return a pointer to the character AFTER the last byte fetched from buf.
*/
ساكن حرف *
hex2mem(حرف *buf, حرف *mem, صحيح ڳڻپ)
{
	صحيح hex_قدر;
	حرف *tmp_raw, *tmp_hex;

	/*
	 * We use the upper half of buf as an intermediate buffer for the
	 * raw memory that is converted from hex.
	 */
	tmp_raw = buf + ڳڻپ * 2;
	tmp_hex = tmp_raw - 1;

	longjmp_on_fault = 1;
	طالما (tmp_hex >= buf) {
		tmp_raw--;
		hex_قدر = hex(*tmp_hex--);
		إذا (hex_قدر < 0)
			kgdb_error(KGDBERR_NOTHEXDIG);
		*tmp_raw = hex_قدر;
		hex_قدر = hex(*tmp_hex--);
		إذا (hex_قدر < 0)
			kgdb_error(KGDBERR_NOTHEXDIG);
		*tmp_raw |= hex_قدر << 4;

	}

	يادگيري_نقل(mem, tmp_raw, ڳڻپ);

	kgdb_flush_cache_range((فراغ *)mem, (فراغ *)(mem+ڳڻپ));
	longjmp_on_fault = 0;

	أرجع buf;
}

/*
 * While we find nice hex chars, build an int.
 * Return number of chars processed.
 */
ساكن صحيح
hex_منزل_Int(حرف **ptr, صحيح *int_قدر)
{
	صحيح انگ_Chars = 0;
	صحيح hex_قدر;

	*int_قدر = 0;

	longjmp_on_fault = 1;
	طالما (**ptr) {
		hex_قدر = hex(**ptr);
		إذا (hex_قدر < 0)
			اكسر;

		*int_قدر = (*int_قدر << 4) | hex_قدر;
		انگ_Chars ++;

		(*ptr)++;
	}
	longjmp_on_fault = 0;

	أرجع (انگ_Chars);
}

/* scan for the sequence $<data>#<checksum>     */
ساكن فراغ
getpacket(حرف *بفر)
{
	بنانشان حرف checksum;
	بنانشان حرف xmitcsum;
	صحيح i;
	صحيح ڳڻپ;
	بنانشان حرف ch;

	افعل {
		/* wait around for the start character, ignore all other
		 * characters */
		طالما ((ch = (getDebugChar() & 0x7f)) != '$') {
#جيڪڏهن_وصف_ٿيل KGDB_DEBUG
			إذا (kdebug)
				putc(ch);
#جيڪڏهن_پورو
			;
		}

		checksum = 0;
		xmitcsum = -1;

		ڳڻپ = 0;

		/* now, read until a # or end of buffer is found */
		طالما (ڳڻپ < BUFMAX) {
			ch = getDebugChar() & 0x7f;
			إذا (ch == '#')
				اكسر;
			checksum = checksum + ch;
			بفر[ڳڻپ] = ch;
			ڳڻپ = ڳڻپ + 1;
		}

		إذا (ڳڻپ >= BUFMAX)
			استمر;

		بفر[ڳڻپ] = 0;

		إذا (ch == '#') {
			xmitcsum = hex(getDebugChar() & 0x7f) << 4;
			xmitcsum |= hex(getDebugChar() & 0x7f);
			إذا (checksum != xmitcsum)
				putDebugChar('-');	/* failed checksum */
			وإلا {
				putDebugChar('+'); /* successful transfer */
				/* if a sequence char is present, reply the ID */
				إذا (بفر[2] == ':') {
					putDebugChar(بفر[0]);
					putDebugChar(بفر[1]);
					/* remove sequence chars from buffer */
					ڳڻپ = لڙي_ڊيگهه(بفر);
					لكل (i=3; i <= ڳڻپ; i++)
						بفر[i-3] = بفر[i];
				}
			}
		}
	} طالما (checksum != xmitcsum);
}

/* send the packet in buffer.  */
ساكن فراغ
putpacket(بنانشان حرف *بفر)
{
	بنانشان حرف checksum;
	صحيح ڳڻپ;
	بنانشان حرف ch, recv;

	/*  $<packet info>#<checksum>. */
	افعل {
		putDebugChar('$');
		checksum = 0;
		ڳڻپ = 0;

		طالما ((ch = بفر[ڳڻپ])) {
			putDebugChar(ch);
			checksum += ch;
			ڳڻپ += 1;
		}

		putDebugChar('#');
		putDebugChar(hexchars[checksum >> 4]);
		putDebugChar(hexchars[checksum & 0xf]);
		recv = getDebugChar();
	} طالما ((recv & 0x7f) != '+');
}

/*
 * This function does all command processing for interfacing to gdb.
 */
ساكن صحيح
handle_exception (بنية pt_regs *regs)
{
	صحيح addr;
	صحيح ڊيگهائي;
	حرف *ptr;
	kgdb_معلومات kd;
	صحيح i;

	إذا (!initialized) {
		لکو("kgdb: exception before kgdb is initialized! huh?\n");
		أرجع (0);
	}

	/* probably should check which exception occurred as well */
	إذا (longjmp_on_fault) {
		longjmp_on_fault = 0;
		kgdb_longjmp(error_jmp_buf, KGDBERR_MEMFAULT);
		panic("kgdb longjump failed!\n");
	}

	إذا (kgdb_سرگرم) {
		لکو("kgdb: unexpected exception from within kgdb\n");
		أرجع (0);
	}
	kgdb_سرگرم = 1;

	kgdb_interruptible(0);

	لکو("kgdb: handle_exception; trap [0x%x]\n", kgdb_trap(regs));

	إذا (kgdb_setjmp(error_jmp_buf) != 0)
		panic("kgdb: error or fault in entry init!\n");

	kgdb_enter(regs, &kd);

	داخلا_regs = *regs;

	ptr = remcom_ٻاهريون_بفر;

	*ptr++ = 'T';

	*ptr++ = hexchars[kd.sigval >> 4];
	*ptr++ = hexchars[kd.sigval & 0xf];

	لكل (i = 0; i < kd.nregs; i++) {
		kgdb_reg *rp = &kd.regs[i];

		*ptr++ = hexchars[rp->انگ >> 4];
		*ptr++ = hexchars[rp->انگ & 0xf];
		*ptr++ = ':';
		ptr = (حرف *)mem2hex((حرف *)&rp->val, ptr, 4);
		*ptr++ = ';';
	}

	*ptr = 0;

#جيڪڏهن_وصف_ٿيل KGDB_DEBUG
	إذا (kdebug)
		لکو("kgdb: remcomOutBuffer: %s\n", remcom_ٻاهريون_بفر);
#جيڪڏهن_پورو

	putpacket((بنانشان حرف *)&remcom_ٻاهريون_بفر);

	طالما (1) {
		متطاير صحيح errnum;

		remcom_ٻاهريون_بفر[0] = 0;

		getpacket(remcom_اندريون_بفر);
		ptr = &remcom_اندريون_بفر[1];

#جيڪڏهن_وصف_ٿيل KGDB_DEBUG
		إذا (kdebug)
			لکو("kgdb:  remcomInBuffer: %s\n", remcom_اندريون_بفر);
#جيڪڏهن_پورو

		errnum = kgdb_setjmp(error_jmp_buf);

		إذا (errnum == 0) بدل (remcom_اندريون_بفر[0]) {

		حالة '?':               /* report most recent signal */
			remcom_ٻاهريون_بفر[0] = 'S';
			remcom_ٻاهريون_بفر[1] = hexchars[kd.sigval >> 4];
			remcom_ٻاهريون_بفر[2] = hexchars[kd.sigval & 0xf];
			remcom_ٻاهريون_بفر[3] = 0;
			اكسر;

#جيڪڏهن_وصف_ٿيل KGDB_DEBUG
		حالة 'd':
			/* toggle debug flag */
			kdebug ^= 1;
			اكسر;
#جيڪڏهن_پورو

		حالة 'g':	/* return the value of the CPU registers. */
			ڊيگهائي = kgdb_getregs(regs, remcom_Reg_بفر, BUFMAX);
			mem2hex(remcom_Reg_بفر, remcom_ٻاهريون_بفر, ڊيگهائي);
			اكسر;

		حالة 'G':   /* set the value of the CPU registers */
			ڊيگهائي = لڙي_ڊيگهه(ptr);
			إذا ((ڊيگهائي & 1) != 0) kgdb_error(KGDBERR_BADPARAMS);
			hex2mem(ptr, remcom_Reg_بفر, ڊيگهائي/2);
			kgdb_putregs(regs, remcom_Reg_بفر, ڊيگهائي/2);
			strcpy(remcom_ٻاهريون_بفر,"OK");
			اكسر;

		حالة 'm':	/* mAA..AA,LLLL  Read LLLL bytes at address AA..AA */
				/* Try to read %x,%x.  */

			إذا (hex_منزل_Int(&ptr, &addr)
			    && *ptr++ == ','
			    && hex_منزل_Int(&ptr, &ڊيگهائي))	{
				mem2hex((حرف *)addr, remcom_ٻاهريون_بفر, ڊيگهائي);
			} وإلا {
				kgdb_error(KGDBERR_BADPARAMS);
			}
			اكسر;

		حالة 'M': /* MAA..AA,LLLL: Write LLLL bytes at address AA.AA return OK */
			/* Try to read '%x,%x:'.  */

			إذا (hex_منزل_Int(&ptr, &addr)
			    && *ptr++ == ','
			    && hex_منزل_Int(&ptr, &ڊيگهائي)
			    && *ptr++ == ':') {
				hex2mem(ptr, (حرف *)addr, ڊيگهائي);
				strcpy(remcom_ٻاهريون_بفر, "OK");
			} وإلا {
				kgdb_error(KGDBERR_BADPARAMS);
			}
			اكسر;

		حالة 'k':    /* kill the program, actually return to monitor */
			kd.extype = KGDBEXIT_KILL;
			*regs = داخلا_regs;
			اذهب doexit;

		حالة 'C':    /* CSS  continue with signal SS */
			*ptr = '\0';	/* ignore the signal number for now */
			/* fall through */

		حالة 'c':    /* cAA..AA  Continue; address AA..AA optional */
			/* try to read optional parameter, pc unchanged if no parm */
			kd.extype = KGDBEXIT_CONTINUE;

			إذا (hex_منزل_Int(&ptr, &addr)) {
				kd.exaddr = addr;
				kd.extype |= KGDBEXIT_WITHADDR;
			}

			اذهب doexit;

		حالة 'S':    /* SSS  single step with signal SS */
			*ptr = '\0';	/* ignore the signal number for now */
			/* fall through */

		حالة 's':
			kd.extype = KGDBEXIT_SINGLE;

			إذا (hex_منزل_Int(&ptr, &addr)) {
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
			kgdb_سرگرم = 0;
			kgdb_interruptible(1);
			أرجع (1);

		حالة 'r':		/* Reset (if user process..exit ???)*/
			panic("kgdb reset.");
			اكسر;

		حالة 'P':    /* Pr=v  set reg r to value v (r and v are hex) */
			إذا (hex_منزل_Int(&ptr, &addr)
			    && *ptr++ == '='
			    && ((ڊيگهائي = لڙي_ڊيگهه(ptr)) & 1) == 0) {
				hex2mem(ptr, remcom_Reg_بفر, ڊيگهائي/2);
				kgdb_putreg(regs, addr,
					remcom_Reg_بفر, ڊيگهائي/2);
				strcpy(remcom_ٻاهريون_بفر,"OK");
			} وإلا {
				kgdb_error(KGDBERR_BADPARAMS);
			}
			اكسر;
		}			/* switch */

		إذا (errnum != 0)
			sprintf(remcom_ٻاهريون_بفر, "E%02d", errnum);

#جيڪڏهن_وصف_ٿيل KGDB_DEBUG
		إذا (kdebug)
			لکو("kgdb: remcomOutBuffer: %s\n", remcom_ٻاهريون_بفر);
#جيڪڏهن_پورو

		/* reply to the request */
		putpacket((بنانشان حرف *)&remcom_ٻاهريون_بفر);

	} /* while(1) */
}

/*
 * kgdb_init must be called *after* the
 * monitor is relocated into ram
 */
صحيح kgdb_init(فراغ)
{
	puts("KGDB:  ");

	kgdb_serial_init();
	debugger_exception_handler = handle_exception;
	initialized = 1;

	putDebugStr("kgdb ready\n");
	puts("ready\n");

	أرجع 0;
}

فراغ
kgdb_error(صحيح errnum)
{
	longjmp_on_fault = 0;
	kgdb_longjmp(error_jmp_buf, errnum);
	panic("kgdb_error: longjmp failed!\n");
}

/* Output string in GDB O-packet format if GDB has connected. If nothing
   output, returns 0 (caller must then handle output). */
صحيح
kgdb_output_string (ثابت حرف* s, بنانشان صحيح ڳڻپ)
{
	حرف بفر[512];

	ڳڻپ = (ڳڻپ <= (حجم(بفر) / 2 - 2))
		? ڳڻپ : (حجم(بفر) / 2 - 2);

	بفر[0] = 'O';
	mem2hex ((حرف *)s, &بفر[1], ڳڻپ);
	putpacket((بنانشان حرف *)&بفر);

	أرجع 1;
}

فراغ
breakpoint(فراغ)
{
	إذا (!initialized) {
		لکو("breakpoint() called b4 kgdb init\n");
		أرجع;
	}

	kgdb_breakpoint(0, 0);
}

صحيح
do_kgdb(بنية cmd_tbl *cmdtp, صحيح flag, صحيح argc, حرف *ثابت argv[])
{
    لکو("Entering KGDB mode via exception handler...\n\n");
    kgdb_breakpoint(argc - 1, argv + 1);
    لکو("\nReturned from KGDB mode\n");
    أرجع 0;
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
