// SPDX-License-Identifier: eCos-2.0
/*
 *==========================================================================
 *
 *      xyzModem.c
 *
 *      RedBoot stream handler for xyzModem protocol
 *
 *==========================================================================
 *#####DESCRIPTIONBEGIN####
 *
 * Author(s):    gthomas
 * Contributors: gthomas, tsmith, Yoshinori Sato
 * Date:         2000-07-14
 * Purpose:
 * Description:
 *
 * This code is part of RedBoot (tm).
 *
 *####DESCRIPTIONEND####
 *
 *==========================================================================
 */
#समावेश <xyzModem.h>
#समावेश <stdarg.h>
#समावेश <time.h>
#समावेश <u-boot/crc.h>
#समावेश <watchdog.h>
#समावेश <env.h>
#समावेश <vsprintf.h>

/* Assumption - run xyzModem protocol over the console port */

/* Values magic to the protocol */
#घोषणा SOH 0x01
#घोषणा STX 0x02
#घोषणा ETX 0x03		/* ^C for interrupt */
#घोषणा EOT 0x04
#घोषणा ACK 0x06
#घोषणा BSP 0x08
#घोषणा NAK 0x15
#घोषणा CAN 0x18
#घोषणा खातापूर्ण 0x1A		/* ^Z for DOS officionados */

/* Data & state local to the protocol */
अटल काष्ठा
{
  पूर्णांक *__chan;
  अचिन्हित अक्षर pkt[1024], *bufp;
  पूर्णांक pad_pending;	/* trailing ^Z run withheld from the previous block */
  पूर्णांक pad_deliver;	/* ^Z bytes to emit before the next data */
  अचिन्हित अक्षर blk, cblk, crc1, crc2;
  अचिन्हित अक्षर अग्र_blk;	/* Expected block */
  पूर्णांक len, mode, योग_retries;
  पूर्णांक योग_SOH, योग_STX, योग_CAN;
  bool crc_mode, at_eof, tx_ack;
  bool first_xmodem_packet;
  ulong initial_time, timeout;
  अचिन्हित दीर्घ file_लंबाई, read_लंबाई;
} xyz;

#घोषणा xyzModem_CHAR_TIMEOUT            2000	/* 2 seconds */
#घोषणा xyzModem_MAX_RETRIES             20
#घोषणा xyzModem_MAX_RETRIES_WITH_CRC    10
#घोषणा xyz_Modem_CAN_गिनती                3	/* Wait for 3 CAN before quitting */

प्रकार पूर्णांक cyg_int32;
अटल पूर्णांक
CYGACC_COMM_IF_GETC_TIMEOUT (अक्षर chan, अक्षर *c)
{

  ulong now = get_timer(0);
  schedule();
  जबतक (!tstc ())
    {
      अगर (get_timer(now) > xyzModem_CHAR_TIMEOUT)
        अवरोध;
    }
  अगर (tstc ())
    {
      *c = अक्षर_लो();
      वापस 1;
    }
  वापस 0;
}

अटल व्योम
CYGACC_COMM_IF_PUTC (अक्षर x, अक्षर y)
{
  अ_दो (y);
}

/* Validate a hex character */
__inline__ अटल bool
_is_hex (अक्षर c)
{
  वापस (((c >= '0') && (c <= '9')) ||
	  ((c >= 'A') && (c <= 'F')) || ((c >= 'a') && (c <= 'f')));
}

/* Convert a single hex nibble */
__inline__ अटल पूर्णांक
_from_hex (अक्षर c)
{
  पूर्णांक ret = 0;

  अगर ((c >= '0') && (c <= '9'))
    {
      ret = (c - '0');
    }
  अन्यथा अगर ((c >= 'a') && (c <= 'f'))
    {
      ret = (c - 'a' + 0x0a);
    }
  अन्यथा अगर ((c >= 'A') && (c <= 'F'))
    {
      ret = (c - 'A' + 0x0A);
    }
  वापस ret;
}

/* Convert a character to lower case */
__inline__ अटल अक्षर
_tolower (अक्षर c)
{
  अगर ((c >= 'A') && (c <= 'Z'))
    {
      c = (c - 'A') + 'a';
    }
  वापस c;
}

/* Parse (scan) a number */
अटल bool
parse_अंक (अक्षर *s, अचिन्हित दीर्घ *val, अक्षर **es, अक्षर *delim)
{
  bool first = true;
  पूर्णांक radix = 10;
  अक्षर c;
  अचिन्हित दीर्घ परिणाम = 0;
  पूर्णांक digit;

  जबतक (*s == ' ')
    s++;
  जबतक (*s)
    {
      अगर (first && (s[0] == '0') && (_tolower (s[1]) == 'x'))
	{
	  radix = 16;
	  s += 2;
	}
      first = false;
      c = *s++;
      अगर (_is_hex (c) && ((digit = _from_hex (c)) < radix))
	{
	  /* Valid digit */
	  परिणाम = (परिणाम * radix) + digit;
	}
      अन्यथा
	{
	  अगर (delim != (अक्षर *) 0)
	    {
	      /* See if this character is one of the delimiters */
	      अक्षर *dp = delim;
	      जबतक (*dp && (c != *dp))
		dp++;
	      अगर (*dp)
		अवरोध;		/* Found a good delimiter */
	    }
	  वापस false;		/* Malformatted number */
	}
    }
  *val = परिणाम;
  अगर (es != (अक्षर **) 0)
    {
      *es = s;
    }
  वापस true;
}

#अगर defined(DEBUG) && !CONFIG_IS_ENABLED(USE_TINY_PRINTF)
/*
 * Note: this debug setup works by storing the strings in a fixed buffer
 */
अटल अक्षर zm_debug_buf[8192];
अटल अक्षर *zm_out = zm_debug_buf;
अटल अक्षर *zm_out_start = zm_debug_buf;

अटल पूर्णांक
zm_dprintf(अक्षर *fmt, ...)
{
	पूर्णांक len;
	बहु_सूची args;

	बहु_शुरू(args, fmt);
	len = diag_vsprintf(zm_out, fmt, args);
	बहु_पूर्ण(args);
	zm_out += len;
	वापस len;
}

अटल व्योम
zm_flush (व्योम)
{
  zm_out = zm_out_start;
}

अटल व्योम
zm_dump_buf (व्योम *buf, पूर्णांक len)
{

}

अटल अचिन्हित अक्षर zm_buf[2048];
अटल अचिन्हित अक्षर *zm_bp;

अटल व्योम
zm_new (व्योम)
{
  zm_bp = zm_buf;
}

अटल व्योम
zm_save (अचिन्हित अक्षर c)
{
  *zm_bp++ = c;
}

अटल व्योम
zm_dump (पूर्णांक पंक्ति)
{
  zm_dprintf ("Packet at line: %d\n", पंक्ति);
  zm_dump_buf (zm_buf, zm_bp - zm_buf);
}

#घोषणा ZM_DEBUG(x) x
#अन्यथा
#घोषणा ZM_DEBUG(x)
#पूर्ण_अगर

/* Wait for the line to go idle */
अटल व्योम
xyzModem_flush (व्योम)
{
  पूर्णांक res;
  अक्षर c;
  जबतक (true)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
      अगर (!res)
	वापस;
    }
}

अटल पूर्णांक
xyzModem_get_hdr (व्योम)
{
  अक्षर c;
  पूर्णांक res;
  bool hdr_found = false;
  पूर्णांक i, can_योग, hdr_chars;
  अचिन्हित लघु cksum;

  ZM_DEBUG (zm_new ());
  /* Find the start of a header */
  can_योग = 0;
  hdr_chars = 0;

  अगर (xyz.tx_ack)
    {
      CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
      xyz.tx_ack = false;
    }
  जबतक (!hdr_found)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
      ZM_DEBUG (zm_save (c));
      अगर (res)
	{
	  hdr_chars++;
	  चयन (c)
	    {
	    हाल SOH:
	      xyz.योग_SOH++;
	      fallthrough;
	    हाल STX:
	      अगर (c == STX)
		xyz.योग_STX++;
	      hdr_found = true;
	      अवरोध;
	    हाल CAN:
	    हाल ETX:
	      xyz.योग_CAN++;
	      ZM_DEBUG (zm_dump (__LINE__));
	      अगर (++can_योग == xyz_Modem_CAN_गिनती)
		{
		  वापस xyzModem_cancel;
		}
	      अन्यथा
		{
		  /* Wait for multiple CAN to avoid early quits */
		  अवरोध;
		}
	    हाल EOT:
	      /* EOT only supported if no noise */
	      अगर (hdr_chars == 1)
		{
		  CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		  ZM_DEBUG (zm_dprintf ("ACK on EOT #%d\n", __LINE__));
		  ZM_DEBUG (zm_dump (__LINE__));
		  वापस xyzModem_eof;
		}
	    शेष:
	      /* Ignore, waiting for start of header */
	      ;
	    }
	}
      अन्यथा
	{
	  /* Data stream timed out */
	  xyzModem_flush ();	/* Toss any current input */
	  ZM_DEBUG (zm_dump (__LINE__));
	  CYGACC_CALL_IF_DELAY_US ((cyg_int32) 250000);
	  वापस xyzModem_timeout;
	}
    }

  /* Header found, now read the data */
  res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (अक्षर *) &xyz.blk);
  ZM_DEBUG (zm_save (xyz.blk));
  अगर (!res)
    {
      ZM_DEBUG (zm_dump (__LINE__));
      वापस xyzModem_timeout;
    }
  res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (अक्षर *) &xyz.cblk);
  ZM_DEBUG (zm_save (xyz.cblk));
  अगर (!res)
    {
      ZM_DEBUG (zm_dump (__LINE__));
      वापस xyzModem_timeout;
    }
  xyz.len = (c == SOH) ? 128 : 1024;
  xyz.bufp = xyz.pkt;
  क्रम (i = 0; i < xyz.len; i++)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
      ZM_DEBUG (zm_save (c));
      अगर (res)
	{
	  xyz.pkt[i] = c;
	}
      अन्यथा
	{
	  ZM_DEBUG (zm_dump (__LINE__));
	  वापस xyzModem_timeout;
	}
    }
  res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (अक्षर *) &xyz.crc1);
  ZM_DEBUG (zm_save (xyz.crc1));
  अगर (!res)
    {
      ZM_DEBUG (zm_dump (__LINE__));
      वापस xyzModem_timeout;
    }
  अगर (xyz.crc_mode)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (अक्षर *) &xyz.crc2);
      ZM_DEBUG (zm_save (xyz.crc2));
      अगर (!res)
	{
	  ZM_DEBUG (zm_dump (__LINE__));
	  वापस xyzModem_timeout;
	}
    }
  ZM_DEBUG (zm_dump (__LINE__));
  /* Validate the message */
  अगर ((xyz.blk ^ xyz.cblk) != (अचिन्हित अक्षर) 0xFF)
    {
      ZM_DEBUG (zm_dprintf
		("Framing error - blk: %x/%x/%x\n", xyz.blk, xyz.cblk,
		 (xyz.blk ^ xyz.cblk)));
      ZM_DEBUG (zm_dump_buf (xyz.pkt, xyz.len));
      xyzModem_flush ();
      वापस xyzModem_frame;
    }
  /* Verify checksum/CRC */
  अगर (xyz.crc_mode)
    {
      cksum = crc16_ccitt(0, xyz.pkt, xyz.len);
      अगर (cksum != ((xyz.crc1 << 8) | xyz.crc2))
	{
	  ZM_DEBUG (zm_dprintf ("CRC error - recvd: %02x%02x, computed: %x\n",
				xyz.crc1, xyz.crc2, cksum & 0xFFFF));
	  वापस xyzModem_cksum;
	}
    }
  अन्यथा
    {
      cksum = 0;
      क्रम (i = 0; i < xyz.len; i++)
	{
	  cksum += xyz.pkt[i];
	}
      अगर (xyz.crc1 != (cksum & 0xFF))
	{
	  ZM_DEBUG (zm_dprintf
		    ("Checksum error - recvd: %x, computed: %x\n", xyz.crc1,
		     cksum & 0xFF));
	  वापस xyzModem_cksum;
	}
    }
  /* If we get here, the message passes [structural] muster */
  वापस 0;
}

अटल
ulong
xyzModem_get_initial_timeout (व्योम)
{
  /* timeout is in seconds, non-positive timeout value is infinity */
#अगर CONFIG_IS_ENABLED(ENV_SUPPORT)
  स्थिर अक्षर *timeout_str = env_get("loadxy_timeout");
  अगर (timeout_str)
    वापस 1000 * simple_strtol(timeout_str, शून्य, 10);
#पूर्ण_अगर
  वापस 1000 * CONFIG_CMD_LOADXY_TIMEOUT;
}

पूर्णांक
xyzModem_stream_open (connection_info_t * info, पूर्णांक *err)
{
  पूर्णांक stat = 0;
  पूर्णांक retries = xyzModem_MAX_RETRIES;
  पूर्णांक crc_retries = xyzModem_MAX_RETRIES_WITH_CRC;

/*    ZM_DEBUG(zm_out = zm_out_start); */
#अगर_घोषित xyzModem_zmodem
  अगर (info->mode == xyzModem_zmodem)
    {
      *err = xyzModem_noZmodem;
      वापस -1;
    }
#पूर्ण_अगर

/* TODO: CHECK ! */
  पूर्णांक dummy = 0;
  xyz.__chan = &dummy;
  xyz.len = 0;
  xyz.pad_pending = 0;
  xyz.pad_deliver = 0;
  xyz.crc_mode = true;
  xyz.at_eof = false;
  xyz.tx_ack = false;
  xyz.mode = info->mode;
  xyz.योग_retries = 0;
  xyz.योग_SOH = 0;
  xyz.योग_STX = 0;
  xyz.योग_CAN = 0;
  xyz.read_लंबाई = 0;
  xyz.file_लंबाई = 0;
  xyz.first_xmodem_packet = false;
  xyz.initial_time = get_timer(0);
  xyz.timeout = xyzModem_get_initial_timeout();

  CYGACC_COMM_IF_PUTC (*xyz.__chan, (xyz.crc_mode ? 'C' : NAK));

  अगर (xyz.mode == xyzModem_xmodem)
    {
      /* X-modem doesn't have an information header - exit here */
      xyz.first_xmodem_packet = true;
      xyz.अग्र_blk = 1;
      वापस 0;
    }

  जबतक (!(xyz.timeout && get_timer(xyz.initial_time) > xyz.timeout))
    {
      अगर (--retries <= 0)
        {
          retries = xyzModem_MAX_RETRIES;
          crc_retries = xyzModem_MAX_RETRIES_WITH_CRC;
          xyz.crc_mode = true;
        }
      stat = xyzModem_get_hdr ();
      अगर (stat == 0)
	{
	  /* Y-modem file information header */
	  अगर (xyz.blk == 0)
	    {
	      /* skip filename */
	      जबतक (*xyz.bufp++);
	      /* get the length */
	      parse_अंक ((अक्षर *) xyz.bufp, &xyz.file_लंबाई, शून्य, " ");
	      /* The rest of the file name data block quietly discarded */
	      xyz.tx_ack = true;
	    }
	  xyz.अग्र_blk = 1;
	  xyz.len = 0;
	  वापस 0;
	}
      अन्यथा अगर (stat == xyzModem_timeout)
	{
	  अगर (--crc_retries <= 0)
	    xyz.crc_mode = false;
	  CYGACC_CALL_IF_DELAY_US (5 * 100000);	/* Extra delay for startup */
	  CYGACC_COMM_IF_PUTC (*xyz.__chan, (xyz.crc_mode ? 'C' : NAK));
	  xyz.योग_retries++;
	  ZM_DEBUG (zm_dprintf ("NAK (%d)\n", __LINE__));
	}
      अगर (stat == xyzModem_cancel)
	{
	  अवरोध;
	}
    }
  *err = stat;
  ZM_DEBUG (zm_flush ());
  वापस -1;
}

पूर्णांक
xyzModem_stream_read (अक्षर *buf, पूर्णांक आकार, पूर्णांक *err)
{
  पूर्णांक stat, योग_2, len;
  पूर्णांक retries;

  योग_2 = 0;
  stat = xyzModem_cancel;
  /* Try and get 'size' bytes into the buffer */
  जबतक (!xyz.at_eof && xyz.len >= 0 && (आकार > 0))
    {
      अगर (xyz.len == 0 && xyz.pad_deliver == 0)
	{
	  retries = xyzModem_MAX_RETRIES;
	  जबतक (retries-- > 0)
	    {
	      अगर (xyz.first_xmodem_packet && xyz.timeout &&
		  get_timer(xyz.initial_time) > xyz.timeout)
		{
		  *err = xyzModem_timeout;
		  xyz.len = -1;
		  वापस योग_2;
		}

	      stat = xyzModem_get_hdr ();
	      अगर (stat == 0)
		{
		  अगर (xyz.mode == xyzModem_xmodem && xyz.first_xmodem_packet)
		    xyz.first_xmodem_packet = false;
		  अगर (xyz.blk == xyz.अग्र_blk)
		    {
		      xyz.tx_ack = true;
		      ZM_DEBUG (zm_dprintf
				("ACK block %d (%d)\n", xyz.blk, __LINE__));
		      xyz.अग्र_blk = (xyz.अग्र_blk + 1) & 0xFF;

		      अगर (xyz.mode == xyzModem_xmodem || xyz.file_लंबाई == 0)
			{
			  /* Without a file length, EOF padding can't be
			   * distinguished from block data until the next
			   * frame arrives and we can check for EOT/EOF.
			   */
			  xyz.pad_deliver = xyz.pad_pending;
			  xyz.pad_pending = 0;
			  अगर ((xyz.bufp[xyz.len - 1] == खातापूर्ण) &&
			      (xyz.bufp[xyz.len - 2] == खातापूर्ण) &&
			      (xyz.bufp[xyz.len - 3] == खातापूर्ण))
			    {
			      जबतक (xyz.len
				     && (xyz.bufp[xyz.len - 1] == खातापूर्ण))
				{
				  xyz.len--;
				  xyz.pad_pending++;
				}
			    }
			}

		      /*
		       * See if accumulated length exceeds that of the file.
		       * If so, reduce size (i.e., cut out pad bytes)
		       * Only do this for Y-modem (and Z-modem should it ever
		       * be supported since it can fall back to Y-modem mode).
		       */
		      अगर (xyz.mode != xyzModem_xmodem && 0 != xyz.file_लंबाई)
			{
			  xyz.read_लंबाई += xyz.len;
			  अगर (xyz.read_लंबाई > xyz.file_लंबाई)
			    {
			      xyz.len -= (xyz.read_लंबाई - xyz.file_लंबाई);
			    }
			}
		      अवरोध;
		    }
		  अन्यथा अगर (xyz.blk == ((xyz.अग्र_blk - 1) & 0xFF))
		    {
		      /* Just re-ACK this so sender will get on with it */
		      CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		      जारी;	/* Need new header */
		    }
		  अन्यथा
		    {
		      stat = xyzModem_sequence;
		    }
		}
	      अगर (stat == xyzModem_cancel)
		{
		  अवरोध;
		}
	      अगर (stat == xyzModem_eof)
		{
		  CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		  ZM_DEBUG (zm_dprintf ("ACK (%d)\n", __LINE__));
		  अगर (xyz.mode == xyzModem_ymodem)
		    {
		      CYGACC_COMM_IF_PUTC (*xyz.__chan,
					   (xyz.crc_mode ? 'C' : NAK));
		      xyz.योग_retries++;
		      ZM_DEBUG (zm_dprintf ("Reading Final Header\n"));
		      stat = xyzModem_get_hdr ();
		      CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		      ZM_DEBUG (zm_dprintf ("FINAL ACK (%d)\n", __LINE__));
		    }
		  अन्यथा
		    stat = 0;

		  xyz.pad_pending = 0;	/* EOT: withheld ^Z run was padding */
		  xyz.at_eof = true;
		  अवरोध;
		}
	      CYGACC_COMM_IF_PUTC (*xyz.__chan, (xyz.crc_mode ? 'C' : NAK));
	      xyz.योग_retries++;
	      ZM_DEBUG (zm_dprintf ("NAK (%d)\n", __LINE__));
	    }
	  अगर (stat < 0 && (!xyz.first_xmodem_packet || stat != xyzModem_timeout))
	    {
	      *err = stat;
	      xyz.len = -1;
	      वापस योग_2;
	    }
	}
      /* A ^Z run withheld from the previous block turned out to be data */
      अगर (xyz.pad_deliver > 0)
	{
	  len = xyz.pad_deliver;
	  अगर (आकार < len)
	    len = आकार;
	  स_रखो (buf, खातापूर्ण, len);
	  आकार -= len;
	  buf += len;
	  योग_2 += len;
	  xyz.pad_deliver -= len;
	}
      /* Don't "read" data from the EOF protocol package */
      अगर (!xyz.at_eof && xyz.len > 0)
	{
	  len = xyz.len;
	  अगर (आकार < len)
	    len = आकार;
	  स_नकल (buf, xyz.bufp, len);
	  आकार -= len;
	  buf += len;
	  योग_2 += len;
	  xyz.len -= len;
	  xyz.bufp += len;
	}
    }
  वापस योग_2;
}

व्योम
xyzModem_stream_close (पूर्णांक *err)
{
  ZM_DEBUG (zm_dprintf
    ("xyzModem - %s mode, %d(SOH)/%d(STX)/%d(CAN) packets, %d retries\n",
     xyz.crc_mode ? "CRC" : "Cksum", xyz.योग_SOH, xyz.योग_STX,
     xyz.योग_CAN, xyz.योग_retries));
  ZM_DEBUG (zm_flush ());
}

/* Need to be able to clean out the input buffer, so have to take the */
/* getc */
व्योम
xyzModem_stream_terminate (bool पात, पूर्णांक (*अ_लो) (व्योम))
{
  पूर्णांक c;

  अगर (पात)
    {
      ZM_DEBUG (zm_dprintf ("!!!! TRANSFER ABORT !!!!\n"));
      चयन (xyz.mode)
	{
	हाल xyzModem_xmodem:
	हाल xyzModem_ymodem:
	  /* The X/YMODEM Spec seems to suggest that multiple CAN followed by an equal */
	  /* number of Backspaces is a friendly way to get the other end to abort. */
	  CYGACC_COMM_IF_PUTC (*xyz.__chan, CAN);
	  CYGACC_COMM_IF_PUTC (*xyz.__chan, CAN);
	  CYGACC_COMM_IF_PUTC (*xyz.__chan, CAN);
	  CYGACC_COMM_IF_PUTC (*xyz.__chan, CAN);
	  CYGACC_COMM_IF_PUTC (*xyz.__chan, BSP);
	  CYGACC_COMM_IF_PUTC (*xyz.__chan, BSP);
	  CYGACC_COMM_IF_PUTC (*xyz.__chan, BSP);
	  CYGACC_COMM_IF_PUTC (*xyz.__chan, BSP);
	  /* Now consume the rest of what's waiting on the line. */
	  ZM_DEBUG (zm_dprintf ("Flushing serial line.\n"));
	  xyzModem_flush ();
	  xyz.at_eof = true;
	  अवरोध;
#अगर_घोषित xyzModem_zmodem
	हाल xyzModem_zmodem:
	  /* Might support it some day I suppose. */
#पूर्ण_अगर
	  अवरोध;
	}
    }
  अन्यथा
    {
      ZM_DEBUG (zm_dprintf ("Engaging cleanup mode...\n"));
      /*
       * Consume any trailing crap left in the inbuffer from
       * previous received blocks. Since very few files are an exact multiple
       * of the transfer block size, there will almost always be some gunk here.
       * If we don't eat it now, RedBoot will think the user typed it.
       */
      ZM_DEBUG (zm_dprintf ("Trailing gunk:\n"));
      जबतक ((c = (*अ_लो) ()) > -1)
        ;
      ZM_DEBUG (zm_dprintf ("\n"));
      /*
       * Make a small delay to give terminal programs like minicom
       * time to get control again after their file transfer program
       * exits.
       */
      CYGACC_CALL_IF_DELAY_US ((cyg_int32) 250000);
    }
}

अक्षर *
xyzModem_error (पूर्णांक err)
{
  चयन (err)
    {
    हाल xyzModem_access:
      वापस "Can't access file";
      अवरोध;
    हाल xyzModem_noZmodem:
      वापस "Sorry, zModem not available yet";
      अवरोध;
    हाल xyzModem_timeout:
      वापस "Timed out";
      अवरोध;
    हाल xyzModem_eof:
      वापस "End of file";
      अवरोध;
    हाल xyzModem_cancel:
      वापस "Cancelled";
      अवरोध;
    हाल xyzModem_frame:
      वापस "Invalid framing";
      अवरोध;
    हाल xyzModem_cksum:
      वापस "CRC/checksum error";
      अवरोध;
    हाल xyzModem_sequence:
      वापस "Block sequence error";
      अवरोध;
    शेष:
      वापस "Unknown error";
      अवरोध;
    }
}

/*
 * RedBoot interface
 */
