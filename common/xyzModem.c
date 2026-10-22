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
#अन्तर्भाव <xyzModem.h>
#अन्तर्भाव <stdarg.h>
#अन्तर्भाव <time.h>
#अन्तर्भाव <u-boot/crc.h>
#अन्तर्भाव <watchdog.h>
#अन्तर्भाव <env.h>
#अन्तर्भाव <vsprintf.h>

/* Assumption - run xyzModem protocol over the console port */

/* Values magic to the protocol */
#define SOH 0x01
#define STX 0x02
#define ETX 0x03		/* ^C for interrupt */
#define EOT 0x04
#define ACK 0x06
#define BSP 0x08
#define NAK 0x15
#define CAN 0x18
#define खातापूर्ण 0x1A		/* ^Z for DOS officionados */

/* Data & state local to the protocol */
स्थिर संरचना
{
  पूर्णांक *__chan;
  अचिन्हित अक्षर pkt[1024], *bufp;
  पूर्णांक pad_pending;	/* trailing ^Z run withheld from the previous block */
  पूर्णांक pad_deliver;	/* ^Z bytes to emit before the next data */
  अचिन्हित अक्षर blk, cblk, crc1, crc2;
  अचिन्हित अक्षर next_blk;	/* Expected block */
  पूर्णांक len, mode, total_retries;
  पूर्णांक total_SOH, total_STX, total_CAN;
  bool crc_mode, at_eof, tx_ack;
  bool first_xmodem_packet;
  ulong initial_time, timeout;
  अचिन्हित दीर्घ file_length, read_length;
} xyz;

#define xyzModem_CHAR_TIMEOUT            2000	/* 2 seconds */
#define xyzModem_MAX_RETRIES             20
#define xyzModem_MAX_RETRIES_WITH_CRC    10
#define xyz_Modem_CAN_मोजणी                3	/* Wait for 3 CAN before quitting */

प्रकारव्याख्या पूर्णांक cyg_int32;
स्थिर पूर्णांक
CYGACC_COMM_IF_GETC_TIMEOUT (अक्षर chan, अक्षर *c)
{

  ulong now = get_timer(0);
  schedule();
  पर्यंत (!tstc ())
    {
      जर (get_timer(now) > xyzModem_CHAR_TIMEOUT)
        थांबा;
    }
  जर (tstc ())
    {
      *c = अक्षर_लो();
      परत 1;
    }
  परत 0;
}

स्थिर शून्य
CYGACC_COMM_IF_PUTC (अक्षर x, अक्षर y)
{
  अ_दो (y);
}

/* Validate a hex character */
__inline__ स्थिर bool
_is_hex (अक्षर c)
{
  परत (((c >= '0') && (c <= '9')) ||
	  ((c >= 'A') && (c <= 'F')) || ((c >= 'a') && (c <= 'f')));
}

/* Convert a single hex nibble */
__inline__ स्थिर पूर्णांक
_from_hex (अक्षर c)
{
  पूर्णांक ret = 0;

  जर ((c >= '0') && (c <= '9'))
    {
      ret = (c - '0');
    }
  अन्यथा जर ((c >= 'a') && (c <= 'f'))
    {
      ret = (c - 'a' + 0x0a);
    }
  अन्यथा जर ((c >= 'A') && (c <= 'F'))
    {
      ret = (c - 'A' + 0x0A);
    }
  परत ret;
}

/* Convert a character to lower case */
__inline__ स्थिर अक्षर
_tolower (अक्षर c)
{
  जर ((c >= 'A') && (c <= 'Z'))
    {
      c = (c - 'A') + 'a';
    }
  परत c;
}

/* Parse (scan) a number */
स्थिर bool
parse_अंक (अक्षर *s, अचिन्हित दीर्घ *val, अक्षर **es, अक्षर *delim)
{
  bool first = true;
  पूर्णांक radix = 10;
  अक्षर c;
  अचिन्हित दीर्घ result = 0;
  पूर्णांक digit;

  पर्यंत (*s == ' ')
    s++;
  पर्यंत (*s)
    {
      जर (first && (s[0] == '0') && (_tolower (s[1]) == 'x'))
	{
	  radix = 16;
	  s += 2;
	}
      first = false;
      c = *s++;
      जर (_is_hex (c) && ((digit = _from_hex (c)) < radix))
	{
	  /* Valid digit */
	  result = (result * radix) + digit;
	}
      अन्यथा
	{
	  जर (delim != (अक्षर *) 0)
	    {
	      /* See if this character is one of the delimiters */
	      अक्षर *dp = delim;
	      पर्यंत (*dp && (c != *dp))
		dp++;
	      जर (*dp)
		थांबा;		/* Found a good delimiter */
	    }
	  परत false;		/* Malformatted number */
	}
    }
  *val = result;
  जर (es != (अक्षर **) 0)
    {
      *es = s;
    }
  परत true;
}

#जर defined(DEBUG) && !CONFIG_IS_ENABLED(USE_TINY_PRINTF)
/*
 * Note: this debug setup works by storing the strings in a fixed buffer
 */
स्थिर अक्षर zm_debug_buf[8192];
स्थिर अक्षर *zm_out = zm_debug_buf;
स्थिर अक्षर *zm_out_start = zm_debug_buf;

स्थिर पूर्णांक
zm_dprintf(अक्षर *fmt, ...)
{
	पूर्णांक len;
	बहु_सूची args;

	बहु_शुरू(args, fmt);
	len = diag_vsprintf(zm_out, fmt, args);
	बहु_पूर्ण(args);
	zm_out += len;
	परत len;
}

स्थिर शून्य
zm_flush (शून्य)
{
  zm_out = zm_out_start;
}

स्थिर शून्य
zm_dump_buf (शून्य *buf, पूर्णांक len)
{

}

स्थिर अचिन्हित अक्षर zm_buf[2048];
स्थिर अचिन्हित अक्षर *zm_bp;

स्थिर शून्य
zm_new (शून्य)
{
  zm_bp = zm_buf;
}

स्थिर शून्य
zm_save (अचिन्हित अक्षर c)
{
  *zm_bp++ = c;
}

स्थिर शून्य
zm_dump (पूर्णांक ओळ)
{
  zm_dprintf ("Packet at line: %d\n", ओळ);
  zm_dump_buf (zm_buf, zm_bp - zm_buf);
}

#define ZM_DEBUG(x) x
#अन्यथा
#define ZM_DEBUG(x)
#endif

/* Wait for the line to go idle */
स्थिर शून्य
xyzModem_flush (शून्य)
{
  पूर्णांक res;
  अक्षर c;
  पर्यंत (true)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
      जर (!res)
	परत;
    }
}

स्थिर पूर्णांक
xyzModem_get_hdr (शून्य)
{
  अक्षर c;
  पूर्णांक res;
  bool hdr_found = false;
  पूर्णांक i, can_total, hdr_chars;
  अचिन्हित लघु cksum;

  ZM_DEBUG (zm_new ());
  /* Find the start of a header */
  can_total = 0;
  hdr_chars = 0;

  जर (xyz.tx_ack)
    {
      CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
      xyz.tx_ack = false;
    }
  पर्यंत (!hdr_found)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
      ZM_DEBUG (zm_save (c));
      जर (res)
	{
	  hdr_chars++;
	  निवड (c)
	    {
	    स्थिती SOH:
	      xyz.total_SOH++;
	      fallthrough;
	    स्थिती STX:
	      जर (c == STX)
		xyz.total_STX++;
	      hdr_found = true;
	      थांबा;
	    स्थिती CAN:
	    स्थिती ETX:
	      xyz.total_CAN++;
	      ZM_DEBUG (zm_dump (__LINE__));
	      जर (++can_total == xyz_Modem_CAN_मोजणी)
		{
		  परत xyzModem_cancel;
		}
	      अन्यथा
		{
		  /* Wait for multiple CAN to avoid early quits */
		  थांबा;
		}
	    स्थिती EOT:
	      /* EOT only supported if no noise */
	      जर (hdr_chars == 1)
		{
		  CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		  ZM_DEBUG (zm_dprintf ("ACK on EOT #%d\n", __LINE__));
		  ZM_DEBUG (zm_dump (__LINE__));
		  परत xyzModem_eof;
		}
	    पूर्वनिर्धारित:
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
	  परत xyzModem_timeout;
	}
    }

  /* Header found, now read the data */
  res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (अक्षर *) &xyz.blk);
  ZM_DEBUG (zm_save (xyz.blk));
  जर (!res)
    {
      ZM_DEBUG (zm_dump (__LINE__));
      परत xyzModem_timeout;
    }
  res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (अक्षर *) &xyz.cblk);
  ZM_DEBUG (zm_save (xyz.cblk));
  जर (!res)
    {
      ZM_DEBUG (zm_dump (__LINE__));
      परत xyzModem_timeout;
    }
  xyz.len = (c == SOH) ? 128 : 1024;
  xyz.bufp = xyz.pkt;
  साठी (i = 0; i < xyz.len; i++)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
      ZM_DEBUG (zm_save (c));
      जर (res)
	{
	  xyz.pkt[i] = c;
	}
      अन्यथा
	{
	  ZM_DEBUG (zm_dump (__LINE__));
	  परत xyzModem_timeout;
	}
    }
  res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (अक्षर *) &xyz.crc1);
  ZM_DEBUG (zm_save (xyz.crc1));
  जर (!res)
    {
      ZM_DEBUG (zm_dump (__LINE__));
      परत xyzModem_timeout;
    }
  जर (xyz.crc_mode)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (अक्षर *) &xyz.crc2);
      ZM_DEBUG (zm_save (xyz.crc2));
      जर (!res)
	{
	  ZM_DEBUG (zm_dump (__LINE__));
	  परत xyzModem_timeout;
	}
    }
  ZM_DEBUG (zm_dump (__LINE__));
  /* Validate the message */
  जर ((xyz.blk ^ xyz.cblk) != (अचिन्हित अक्षर) 0xFF)
    {
      ZM_DEBUG (zm_dprintf
		("Framing error - blk: %x/%x/%x\n", xyz.blk, xyz.cblk,
		 (xyz.blk ^ xyz.cblk)));
      ZM_DEBUG (zm_dump_buf (xyz.pkt, xyz.len));
      xyzModem_flush ();
      परत xyzModem_frame;
    }
  /* Verify checksum/CRC */
  जर (xyz.crc_mode)
    {
      cksum = crc16_ccitt(0, xyz.pkt, xyz.len);
      जर (cksum != ((xyz.crc1 << 8) | xyz.crc2))
	{
	  ZM_DEBUG (zm_dprintf ("CRC error - recvd: %02x%02x, computed: %x\n",
				xyz.crc1, xyz.crc2, cksum & 0xFFFF));
	  परत xyzModem_cksum;
	}
    }
  अन्यथा
    {
      cksum = 0;
      साठी (i = 0; i < xyz.len; i++)
	{
	  cksum += xyz.pkt[i];
	}
      जर (xyz.crc1 != (cksum & 0xFF))
	{
	  ZM_DEBUG (zm_dprintf
		    ("Checksum error - recvd: %x, computed: %x\n", xyz.crc1,
		     cksum & 0xFF));
	  परत xyzModem_cksum;
	}
    }
  /* If we get here, the message passes [structural] muster */
  परत 0;
}

स्थिर
ulong
xyzModem_get_initial_timeout (शून्य)
{
  /* timeout is in seconds, non-positive timeout value is infinity */
#जर CONFIG_IS_ENABLED(ENV_SUPPORT)
  स्थिरांक अक्षर *timeout_str = env_get("loadxy_timeout");
  जर (timeout_str)
    परत 1000 * simple_strtol(timeout_str, रिक्त, 10);
#endif
  परत 1000 * CONFIG_CMD_LOADXY_TIMEOUT;
}

पूर्णांक
xyzModem_stream_open (connection_info_t * info, पूर्णांक *err)
{
  पूर्णांक stat = 0;
  पूर्णांक retries = xyzModem_MAX_RETRIES;
  पूर्णांक crc_retries = xyzModem_MAX_RETRIES_WITH_CRC;

/*    ZM_DEBUG(zm_out = zm_out_start); */
#ifdef xyzModem_zmodem
  जर (info->mode == xyzModem_zmodem)
    {
      *err = xyzModem_noZmodem;
      परत -1;
    }
#endif

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
  xyz.total_retries = 0;
  xyz.total_SOH = 0;
  xyz.total_STX = 0;
  xyz.total_CAN = 0;
  xyz.read_length = 0;
  xyz.file_length = 0;
  xyz.first_xmodem_packet = false;
  xyz.initial_time = get_timer(0);
  xyz.timeout = xyzModem_get_initial_timeout();

  CYGACC_COMM_IF_PUTC (*xyz.__chan, (xyz.crc_mode ? 'C' : NAK));

  जर (xyz.mode == xyzModem_xmodem)
    {
      /* X-modem doesn't have an information header - exit here */
      xyz.first_xmodem_packet = true;
      xyz.next_blk = 1;
      परत 0;
    }

  पर्यंत (!(xyz.timeout && get_timer(xyz.initial_time) > xyz.timeout))
    {
      जर (--retries <= 0)
        {
          retries = xyzModem_MAX_RETRIES;
          crc_retries = xyzModem_MAX_RETRIES_WITH_CRC;
          xyz.crc_mode = true;
        }
      stat = xyzModem_get_hdr ();
      जर (stat == 0)
	{
	  /* Y-modem file information header */
	  जर (xyz.blk == 0)
	    {
	      /* skip filename */
	      पर्यंत (*xyz.bufp++);
	      /* get the length */
	      parse_अंक ((अक्षर *) xyz.bufp, &xyz.file_length, रिक्त, " ");
	      /* The rest of the file name data block quietly discarded */
	      xyz.tx_ack = true;
	    }
	  xyz.next_blk = 1;
	  xyz.len = 0;
	  परत 0;
	}
      अन्यथा जर (stat == xyzModem_timeout)
	{
	  जर (--crc_retries <= 0)
	    xyz.crc_mode = false;
	  CYGACC_CALL_IF_DELAY_US (5 * 100000);	/* Extra delay for startup */
	  CYGACC_COMM_IF_PUTC (*xyz.__chan, (xyz.crc_mode ? 'C' : NAK));
	  xyz.total_retries++;
	  ZM_DEBUG (zm_dprintf ("NAK (%d)\n", __LINE__));
	}
      जर (stat == xyzModem_cancel)
	{
	  थांबा;
	}
    }
  *err = stat;
  ZM_DEBUG (zm_flush ());
  परत -1;
}

पूर्णांक
xyzModem_stream_read (अक्षर *buf, पूर्णांक size, पूर्णांक *err)
{
  पूर्णांक stat, total, len;
  पूर्णांक retries;

  total = 0;
  stat = xyzModem_cancel;
  /* Try and get 'size' bytes into the buffer */
  पर्यंत (!xyz.at_eof && xyz.len >= 0 && (size > 0))
    {
      जर (xyz.len == 0 && xyz.pad_deliver == 0)
	{
	  retries = xyzModem_MAX_RETRIES;
	  पर्यंत (retries-- > 0)
	    {
	      जर (xyz.first_xmodem_packet && xyz.timeout &&
		  get_timer(xyz.initial_time) > xyz.timeout)
		{
		  *err = xyzModem_timeout;
		  xyz.len = -1;
		  परत total;
		}

	      stat = xyzModem_get_hdr ();
	      जर (stat == 0)
		{
		  जर (xyz.mode == xyzModem_xmodem && xyz.first_xmodem_packet)
		    xyz.first_xmodem_packet = false;
		  जर (xyz.blk == xyz.next_blk)
		    {
		      xyz.tx_ack = true;
		      ZM_DEBUG (zm_dprintf
				("ACK block %d (%d)\n", xyz.blk, __LINE__));
		      xyz.next_blk = (xyz.next_blk + 1) & 0xFF;

		      जर (xyz.mode == xyzModem_xmodem || xyz.file_length == 0)
			{
			  /* Without a file length, EOF padding can't be
			   * distinguished from block data until the next
			   * frame arrives and we can check for EOT/EOF.
			   */
			  xyz.pad_deliver = xyz.pad_pending;
			  xyz.pad_pending = 0;
			  जर ((xyz.bufp[xyz.len - 1] == खातापूर्ण) &&
			      (xyz.bufp[xyz.len - 2] == खातापूर्ण) &&
			      (xyz.bufp[xyz.len - 3] == खातापूर्ण))
			    {
			      पर्यंत (xyz.len
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
		      जर (xyz.mode != xyzModem_xmodem && 0 != xyz.file_length)
			{
			  xyz.read_length += xyz.len;
			  जर (xyz.read_length > xyz.file_length)
			    {
			      xyz.len -= (xyz.read_length - xyz.file_length);
			    }
			}
		      थांबा;
		    }
		  अन्यथा जर (xyz.blk == ((xyz.next_blk - 1) & 0xFF))
		    {
		      /* Just re-ACK this so sender will get on with it */
		      CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		      सुरूठेवा;	/* Need new header */
		    }
		  अन्यथा
		    {
		      stat = xyzModem_sequence;
		    }
		}
	      जर (stat == xyzModem_cancel)
		{
		  थांबा;
		}
	      जर (stat == xyzModem_eof)
		{
		  CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		  ZM_DEBUG (zm_dprintf ("ACK (%d)\n", __LINE__));
		  जर (xyz.mode == xyzModem_ymodem)
		    {
		      CYGACC_COMM_IF_PUTC (*xyz.__chan,
					   (xyz.crc_mode ? 'C' : NAK));
		      xyz.total_retries++;
		      ZM_DEBUG (zm_dprintf ("Reading Final Header\n"));
		      stat = xyzModem_get_hdr ();
		      CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		      ZM_DEBUG (zm_dprintf ("FINAL ACK (%d)\n", __LINE__));
		    }
		  अन्यथा
		    stat = 0;

		  xyz.pad_pending = 0;	/* EOT: withheld ^Z run was padding */
		  xyz.at_eof = true;
		  थांबा;
		}
	      CYGACC_COMM_IF_PUTC (*xyz.__chan, (xyz.crc_mode ? 'C' : NAK));
	      xyz.total_retries++;
	      ZM_DEBUG (zm_dprintf ("NAK (%d)\n", __LINE__));
	    }
	  जर (stat < 0 && (!xyz.first_xmodem_packet || stat != xyzModem_timeout))
	    {
	      *err = stat;
	      xyz.len = -1;
	      परत total;
	    }
	}
      /* A ^Z run withheld from the previous block turned out to be data */
      जर (xyz.pad_deliver > 0)
	{
	  len = xyz.pad_deliver;
	  जर (size < len)
	    len = size;
	  स_रखो (buf, खातापूर्ण, len);
	  size -= len;
	  buf += len;
	  total += len;
	  xyz.pad_deliver -= len;
	}
      /* Don't "read" data from the EOF protocol package */
      जर (!xyz.at_eof && xyz.len > 0)
	{
	  len = xyz.len;
	  जर (size < len)
	    len = size;
	  स_नकल (buf, xyz.bufp, len);
	  size -= len;
	  buf += len;
	  total += len;
	  xyz.len -= len;
	  xyz.bufp += len;
	}
    }
  परत total;
}

शून्य
xyzModem_stream_close (पूर्णांक *err)
{
  ZM_DEBUG (zm_dprintf
    ("xyzModem - %s mode, %d(SOH)/%d(STX)/%d(CAN) packets, %d retries\n",
     xyz.crc_mode ? "CRC" : "Cksum", xyz.total_SOH, xyz.total_STX,
     xyz.total_CAN, xyz.total_retries));
  ZM_DEBUG (zm_flush ());
}

/* Need to be able to clean out the input buffer, so have to take the */
/* getc */
शून्य
xyzModem_stream_terminate (bool पात, पूर्णांक (*अ_लो) (शून्य))
{
  पूर्णांक c;

  जर (पात)
    {
      ZM_DEBUG (zm_dprintf ("!!!! TRANSFER ABORT !!!!\n"));
      निवड (xyz.mode)
	{
	स्थिती xyzModem_xmodem:
	स्थिती xyzModem_ymodem:
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
	  थांबा;
#ifdef xyzModem_zmodem
	स्थिती xyzModem_zmodem:
	  /* Might support it some day I suppose. */
#endif
	  थांबा;
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
      पर्यंत ((c = (*अ_लो) ()) > -1)
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
  निवड (err)
    {
    स्थिती xyzModem_access:
      परत "Can't access file";
      थांबा;
    स्थिती xyzModem_noZmodem:
      परत "Sorry, zModem not available yet";
      थांबा;
    स्थिती xyzModem_timeout:
      परत "Timed out";
      थांबा;
    स्थिती xyzModem_eof:
      परत "End of file";
      थांबा;
    स्थिती xyzModem_cancel:
      परत "Cancelled";
      थांबा;
    स्थिती xyzModem_frame:
      परत "Invalid framing";
      थांबा;
    स्थिती xyzModem_cksum:
      परत "CRC/checksum error";
      थांबा;
    स्थिती xyzModem_sequence:
      परत "Block sequence error";
      थांबा;
    पूर्वनिर्धारित:
      परत "Unknown error";
      थांबा;
    }
}

/*
 * RedBoot interface
 */
