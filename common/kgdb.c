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

#समावेश <asm/ptrace.h>

#समावेश <kgdb.h>
#समावेश <command.h>

#अघोषित KGDB_DEBUG

/*
 * BUFMAX defines the maximum number of characters in inbound/outbound buffers
 */
#घोषणा BUFMAX 1024
अटल अक्षर remcom_In_बफर[BUFMAX];
अटल अक्षर remcom_Out_बफर[BUFMAX];
अटल अक्षर remcom_Reg_बफर[BUFMAX];

अटल पूर्णांक initialized = 0;
अटल पूर्णांक kgdb_active;
अटल काष्ठा pt_regs entry_regs;
अटल दीर्घ error_jmp_buf[BUFMAX/2];
अटल पूर्णांक longjmp_on_fault = 0;
#अगर_घोषित KGDB_DEBUG
अटल पूर्णांक kdebug = 1;
#पूर्ण_अगर

अटल स्थिर अक्षर hexchars[]="0123456789abcdef";

/* Convert ch from a hex digit to an int */
अटल पूर्णांक
hex(अचिन्हित अक्षर ch)
{
	अगर (ch >= 'a' && ch <= 'f')
		वापस ch-'a'+10;
	अगर (ch >= '0' && ch <= '9')
		वापस ch-'0';
	अगर (ch >= 'A' && ch <= 'F')
		वापस ch-'A'+10;
	वापस -1;
}

/* Convert the memory pointed to by mem into hex, placing result in buf.
 * Return a pointer to the last char put in buf (null).
 */
अटल अचिन्हित अक्षर *
mem2hex(अक्षर *mem, अक्षर *buf, पूर्णांक गिनती)
{
	अक्षर *tmp;
	अचिन्हित अक्षर ch;

	/*
	 * We use the upper half of buf as an intermediate buffer for the
	 * raw memory copy.  Hex conversion will work against this one.
	 */
	tmp = buf + गिनती;
	longjmp_on_fault = 1;

	स_नकल(tmp, mem, गिनती);

	जबतक (गिनती-- > 0) {
		ch = *tmp++;
		*buf++ = hexchars[ch >> 4];
		*buf++ = hexchars[ch & 0xf];
	}
	*buf = 0;
	longjmp_on_fault = 0;
	वापस (अचिन्हित अक्षर *)buf;
}

/* convert the hex array pointed to by buf into binary to be placed in mem
 * return a pointer to the character AFTER the last byte fetched from buf.
*/
अटल अक्षर *
hex2mem(अक्षर *buf, अक्षर *mem, पूर्णांक गिनती)
{
	पूर्णांक hex_मूल्य;
	अक्षर *tmp_raw, *tmp_hex;

	/*
	 * We use the upper half of buf as an intermediate buffer for the
	 * raw memory that is converted from hex.
	 */
	tmp_raw = buf + गिनती * 2;
	tmp_hex = tmp_raw - 1;

	longjmp_on_fault = 1;
	जबतक (tmp_hex >= buf) {
		tmp_raw--;
		hex_मूल्य = hex(*tmp_hex--);
		अगर (hex_मूल्य < 0)
			kgdb_error(KGDBERR_NOTHEXDIG);
		*tmp_raw = hex_मूल्य;
		hex_मूल्य = hex(*tmp_hex--);
		अगर (hex_मूल्य < 0)
			kgdb_error(KGDBERR_NOTHEXDIG);
		*tmp_raw |= hex_मूल्य << 4;

	}

	स_नकल(mem, tmp_raw, गिनती);

	kgdb_flush_cache_range((व्योम *)mem, (व्योम *)(mem+गिनती));
	longjmp_on_fault = 0;

	वापस buf;
}

/*
 * While we find nice hex chars, build an int.
 * Return number of chars processed.
 */
अटल पूर्णांक
hexToInt(अक्षर **ptr, पूर्णांक *int_मूल्य)
{
	पूर्णांक अंक_Chars = 0;
	पूर्णांक hex_मूल्य;

	*int_मूल्य = 0;

	longjmp_on_fault = 1;
	जबतक (**ptr) {
		hex_मूल्य = hex(**ptr);
		अगर (hex_मूल्य < 0)
			अवरोध;

		*int_मूल्य = (*int_मूल्य << 4) | hex_मूल्य;
		अंक_Chars ++;

		(*ptr)++;
	}
	longjmp_on_fault = 0;

	वापस (अंक_Chars);
}

/* scan for the sequence $<data>#<checksum>     */
अटल व्योम
getpacket(अक्षर *बफर)
{
	अचिन्हित अक्षर checksum;
	अचिन्हित अक्षर xmitcsum;
	पूर्णांक i;
	पूर्णांक गिनती;
	अचिन्हित अक्षर ch;

	करो {
		/* wait around for the start character, ignore all other
		 * characters */
		जबतक ((ch = (getDebugChar() & 0x7f)) != '$') {
#अगर_घोषित KGDB_DEBUG
			अगर (kdebug)
				अ_दो(ch);
#पूर्ण_अगर
			;
		}

		checksum = 0;
		xmitcsum = -1;

		गिनती = 0;

		/* now, read until a # or end of buffer is found */
		जबतक (गिनती < BUFMAX) {
			ch = getDebugChar() & 0x7f;
			अगर (ch == '#')
				अवरोध;
			checksum = checksum + ch;
			बफर[गिनती] = ch;
			गिनती = गिनती + 1;
		}

		अगर (गिनती >= BUFMAX)
			जारी;

		बफर[गिनती] = 0;

		अगर (ch == '#') {
			xmitcsum = hex(getDebugChar() & 0x7f) << 4;
			xmitcsum |= hex(getDebugChar() & 0x7f);
			अगर (checksum != xmitcsum)
				putDebugChar('-');	/* failed checksum */
			अन्यथा {
				putDebugChar('+'); /* successful transfer */
				/* if a sequence char is present, reply the ID */
				अगर (बफर[2] == ':') {
					putDebugChar(बफर[0]);
					putDebugChar(बफर[1]);
					/* remove sequence chars from buffer */
					गिनती = म_माप(बफर);
					क्रम (i=3; i <= गिनती; i++)
						बफर[i-3] = बफर[i];
				}
			}
		}
	} जबतक (checksum != xmitcsum);
}

/* send the packet in buffer.  */
अटल व्योम
putpacket(अचिन्हित अक्षर *बफर)
{
	अचिन्हित अक्षर checksum;
	पूर्णांक गिनती;
	अचिन्हित अक्षर ch, recv;

	/*  $<packet info>#<checksum>. */
	करो {
		putDebugChar('$');
		checksum = 0;
		गिनती = 0;

		जबतक ((ch = बफर[गिनती])) {
			putDebugChar(ch);
			checksum += ch;
			गिनती += 1;
		}

		putDebugChar('#');
		putDebugChar(hexchars[checksum >> 4]);
		putDebugChar(hexchars[checksum & 0xf]);
		recv = getDebugChar();
	} जबतक ((recv & 0x7f) != '+');
}

/*
 * This function does all command processing for interfacing to gdb.
 */
अटल पूर्णांक
handle_exception (काष्ठा pt_regs *regs)
{
	पूर्णांक addr;
	पूर्णांक लंबाई;
	अक्षर *ptr;
	kgdb_तथ्य kd;
	पूर्णांक i;

	अगर (!initialized) {
		म_लिखो("kgdb: exception before kgdb is initialized! huh?\n");
		वापस (0);
	}

	/* probably should check which exception occurred as well */
	अगर (longjmp_on_fault) {
		longjmp_on_fault = 0;
		kgdb_longjmp(error_jmp_buf, KGDBERR_MEMFAULT);
		panic("kgdb longjump failed!\n");
	}

	अगर (kgdb_active) {
		म_लिखो("kgdb: unexpected exception from within kgdb\n");
		वापस (0);
	}
	kgdb_active = 1;

	kgdb_interruptible(0);

	म_लिखो("kgdb: handle_exception; trap [0x%x]\n", kgdb_trap(regs));

	अगर (kgdb_setjmp(error_jmp_buf) != 0)
		panic("kgdb: error or fault in entry init!\n");

	kgdb_enter(regs, &kd);

	entry_regs = *regs;

	ptr = remcom_Out_बफर;

	*ptr++ = 'T';

	*ptr++ = hexchars[kd.sigval >> 4];
	*ptr++ = hexchars[kd.sigval & 0xf];

	क्रम (i = 0; i < kd.nregs; i++) {
		kgdb_reg *rp = &kd.regs[i];

		*ptr++ = hexchars[rp->अंक >> 4];
		*ptr++ = hexchars[rp->अंक & 0xf];
		*ptr++ = ':';
		ptr = (अक्षर *)mem2hex((अक्षर *)&rp->val, ptr, 4);
		*ptr++ = ';';
	}

	*ptr = 0;

#अगर_घोषित KGDB_DEBUG
	अगर (kdebug)
		म_लिखो("kgdb: remcomOutBuffer: %s\n", remcom_Out_बफर);
#पूर्ण_अगर

	putpacket((अचिन्हित अक्षर *)&remcom_Out_बफर);

	जबतक (1) {
		अस्थिर पूर्णांक errnum;

		remcom_Out_बफर[0] = 0;

		getpacket(remcom_In_बफर);
		ptr = &remcom_In_बफर[1];

#अगर_घोषित KGDB_DEBUG
		अगर (kdebug)
			म_लिखो("kgdb:  remcomInBuffer: %s\n", remcom_In_बफर);
#पूर्ण_अगर

		errnum = kgdb_setjmp(error_jmp_buf);

		अगर (errnum == 0) चयन (remcom_In_बफर[0]) {

		हाल '?':               /* report most recent signal */
			remcom_Out_बफर[0] = 'S';
			remcom_Out_बफर[1] = hexchars[kd.sigval >> 4];
			remcom_Out_बफर[2] = hexchars[kd.sigval & 0xf];
			remcom_Out_बफर[3] = 0;
			अवरोध;

#अगर_घोषित KGDB_DEBUG
		हाल 'd':
			/* toggle debug flag */
			kdebug ^= 1;
			अवरोध;
#पूर्ण_अगर

		हाल 'g':	/* return the value of the CPU registers. */
			लंबाई = kgdb_getregs(regs, remcom_Reg_बफर, BUFMAX);
			mem2hex(remcom_Reg_बफर, remcom_Out_बफर, लंबाई);
			अवरोध;

		हाल 'G':   /* set the value of the CPU registers */
			लंबाई = म_माप(ptr);
			अगर ((लंबाई & 1) != 0) kgdb_error(KGDBERR_BADPARAMS);
			hex2mem(ptr, remcom_Reg_बफर, लंबाई/2);
			kgdb_putregs(regs, remcom_Reg_बफर, लंबाई/2);
			म_नकल(remcom_Out_बफर,"OK");
			अवरोध;

		हाल 'm':	/* mAA..AA,LLLL  Read LLLL bytes at address AA..AA */
				/* Try to read %x,%x.  */

			अगर (hexToInt(&ptr, &addr)
			    && *ptr++ == ','
			    && hexToInt(&ptr, &लंबाई))	{
				mem2hex((अक्षर *)addr, remcom_Out_बफर, लंबाई);
			} अन्यथा {
				kgdb_error(KGDBERR_BADPARAMS);
			}
			अवरोध;

		हाल 'M': /* MAA..AA,LLLL: Write LLLL bytes at address AA.AA return OK */
			/* Try to read '%x,%x:'.  */

			अगर (hexToInt(&ptr, &addr)
			    && *ptr++ == ','
			    && hexToInt(&ptr, &लंबाई)
			    && *ptr++ == ':') {
				hex2mem(ptr, (अक्षर *)addr, लंबाई);
				म_नकल(remcom_Out_बफर, "OK");
			} अन्यथा {
				kgdb_error(KGDBERR_BADPARAMS);
			}
			अवरोध;

		हाल 'k':    /* kill the program, actually return to monitor */
			kd.extype = KGDBEXIT_KILL;
			*regs = entry_regs;
			जाओ doexit;

		हाल 'C':    /* CSS  continue with signal SS */
			*ptr = '\0';	/* ignore the signal number for now */
			/* fall through */

		हाल 'c':    /* cAA..AA  Continue; address AA..AA optional */
			/* try to read optional parameter, pc unchanged if no parm */
			kd.extype = KGDBEXIT_CONTINUE;

			अगर (hexToInt(&ptr, &addr)) {
				kd.exaddr = addr;
				kd.extype |= KGDBEXIT_WITHADDR;
			}

			जाओ doexit;

		हाल 'S':    /* SSS  single step with signal SS */
			*ptr = '\0';	/* ignore the signal number for now */
			/* fall through */

		हाल 's':
			kd.extype = KGDBEXIT_SINGLE;

			अगर (hexToInt(&ptr, &addr)) {
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
			वापस (1);

		हाल 'r':		/* Reset (if user process..exit ???)*/
			panic("kgdb reset.");
			अवरोध;

		हाल 'P':    /* Pr=v  set reg r to value v (r and v are hex) */
			अगर (hexToInt(&ptr, &addr)
			    && *ptr++ == '='
			    && ((लंबाई = म_माप(ptr)) & 1) == 0) {
				hex2mem(ptr, remcom_Reg_बफर, लंबाई/2);
				kgdb_putreg(regs, addr,
					remcom_Reg_बफर, लंबाई/2);
				म_नकल(remcom_Out_बफर,"OK");
			} अन्यथा {
				kgdb_error(KGDBERR_BADPARAMS);
			}
			अवरोध;
		}			/* switch */

		अगर (errnum != 0)
			प्र_लिखो(remcom_Out_बफर, "E%02d", errnum);

#अगर_घोषित KGDB_DEBUG
		अगर (kdebug)
			म_लिखो("kgdb: remcomOutBuffer: %s\n", remcom_Out_बफर);
#पूर्ण_अगर

		/* reply to the request */
		putpacket((अचिन्हित अक्षर *)&remcom_Out_बफर);

	} /* while(1) */
}

/*
 * kgdb_init must be called *after* the
 * monitor is relocated into ram
 */
पूर्णांक kgdb_init(व्योम)
{
	माला_दो("KGDB:  ");

	kgdb_serial_init();
	debugger_exception_handler = handle_exception;
	initialized = 1;

	putDebugStr("kgdb ready\n");
	माला_दो("ready\n");

	वापस 0;
}

व्योम
kgdb_error(पूर्णांक errnum)
{
	longjmp_on_fault = 0;
	kgdb_longjmp(error_jmp_buf, errnum);
	panic("kgdb_error: longjmp failed!\n");
}

/* Output string in GDB O-packet format if GDB has connected. If nothing
   output, returns 0 (caller must then handle output). */
पूर्णांक
kgdb_output_string (स्थिर अक्षर* s, अचिन्हित पूर्णांक गिनती)
{
	अक्षर बफर[512];

	गिनती = (गिनती <= (माप(बफर) / 2 - 2))
		? गिनती : (माप(बफर) / 2 - 2);

	बफर[0] = 'O';
	mem2hex ((अक्षर *)s, &बफर[1], गिनती);
	putpacket((अचिन्हित अक्षर *)&बफर);

	वापस 1;
}

व्योम
breakpoint(व्योम)
{
	अगर (!initialized) {
		म_लिखो("breakpoint() called b4 kgdb init\n");
		वापस;
	}

	kgdb_breakpoint(0, 0);
}

पूर्णांक
do_kgdb(काष्ठा cmd_tbl *cmdtp, पूर्णांक flag, पूर्णांक argc, अक्षर *स्थिर argv[])
{
    म_लिखो("Entering KGDB mode via exception handler...\n\n");
    kgdb_breakpoint(argc - 1, argv + 1);
    म_लिखो("\nReturned from KGDB mode\n");
    वापस 0;
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
