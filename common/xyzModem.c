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
#परिभाषय SOH 0x01
#परिभाषय STX 0x02
#परिभाषय ETX 0x03		/* ^C for interrupt */
#परिभाषय EOT 0x04
#परिभाषय ACK 0x06
#परिभाषय BSP 0x08
#परिभाषय NAK 0x15
#परिभाषय CAN 0x18
#परिभाषय EOF 0x1A		/* ^Z for DOS officionados */

/* Data & state local to the protocol */
स्थिर संरचना
{
  पूर्णाङ्क *__chan;
  अचिह्नित वर्ण pkt[1024], *bufp;
  पूर्णाङ्क pad_pending;	/* trailing ^Z run withheld from the previous block */
  पूर्णाङ्क pad_deliver;	/* ^Z bytes to emit before the next data */
  अचिह्नित वर्ण blk, cblk, crc1, crc2;
  अचिह्नित वर्ण अग्र_blk;	/* Expected block */
  पूर्णाङ्क len, बहुलकम्, योग_retries;
  पूर्णाङ्क योग_SOH, योग_STX, योग_CAN;
  bool crc_बहुलकम्, at_eof, tx_ack;
  bool first_xmodem_packet;
  ulong initial_time, timeout;
  अचिह्नित दीर्घ file_दीर्घता, read_दीर्घता;
} xyz;

#परिभाषय xyzModem_CHAR_TIMEOUT            2000	/* 2 seconds */
#परिभाषय xyzModem_MAX_RETRIES             20
#परिभाषय xyzModem_MAX_RETRIES_WITH_CRC    10
#परिभाषय xyz_Modem_CAN_गणनम्                3	/* Wait for 3 CAN before quitting */

प्रकारनिर्देश पूर्णाङ्क cyg_int32;
स्थिर पूर्णाङ्क
CYGACC_COMM_IF_GETC_TIMEOUT (वर्ण chan, वर्ण *c)
{

  ulong now = get_timer(0);
  कालसारणी();
  यावत् (!tstc ())
    {
      यदि (get_timer(now) > xyzModem_CHAR_TIMEOUT)
        भङ्ग;
    }
  यदि (tstc ())
    {
      *c = अक्षरं_पठ();
      प्रत्यागम 1;
    }
  प्रत्यागम 0;
}

स्थिर शून्य
CYGACC_COMM_IF_PUTC (वर्ण x, वर्ण y)
{
  putc (y);
}

/* Validate a hex character */
__inline__ स्थिर bool
_is_hex (वर्ण c)
{
  प्रत्यागम (((c >= '0') && (c <= '9')) ||
	  ((c >= 'A') && (c <= 'F')) || ((c >= 'a') && (c <= 'f')));
}

/* Convert a single hex nibble */
__inline__ स्थिर पूर्णाङ्क
_from_hex (वर्ण c)
{
  पूर्णाङ्क ret = 0;

  यदि ((c >= '0') && (c <= '9'))
    {
      ret = (c - '0');
    }
  अन्यथा यदि ((c >= 'a') && (c <= 'f'))
    {
      ret = (c - 'a' + 0x0a);
    }
  अन्यथा यदि ((c >= 'A') && (c <= 'F'))
    {
      ret = (c - 'A' + 0x0A);
    }
  प्रत्यागम ret;
}

/* Convert a character to lower case */
__inline__ स्थिर वर्ण
_tolower (वर्ण c)
{
  यदि ((c >= 'A') && (c <= 'Z'))
    {
      c = (c - 'A') + 'a';
    }
  प्रत्यागम c;
}

/* Parse (scan) a number */
स्थिर bool
parse_अङ्कः (वर्ण *s, अचिह्नित दीर्घ *val, वर्ण **es, वर्ण *delim)
{
  bool first = true;
  पूर्णाङ्क radix = 10;
  वर्ण c;
  अचिह्नित दीर्घ परिणाम = 0;
  पूर्णाङ्क digit;

  यावत् (*s == ' ')
    s++;
  यावत् (*s)
    {
      यदि (first && (s[0] == '0') && (_tolower (s[1]) == 'x'))
	{
	  radix = 16;
	  s += 2;
	}
      first = false;
      c = *s++;
      यदि (_is_hex (c) && ((digit = _from_hex (c)) < radix))
	{
	  /* Valid digit */
	  परिणाम = (परिणाम * radix) + digit;
	}
      अन्यथा
	{
	  यदि (delim != (वर्ण *) 0)
	    {
	      /* See if this character is one of the delimiters */
	      वर्ण *dp = delim;
	      यावत् (*dp && (c != *dp))
		dp++;
	      यदि (*dp)
		भङ्ग;		/* Found a good delimiter */
	    }
	  प्रत्यागम false;		/* Malformatted number */
	}
    }
  *val = परिणाम;
  यदि (es != (वर्ण **) 0)
    {
      *es = s;
    }
  प्रत्यागम true;
}

#यदि defined(DEBUG) && !CONFIG_IS_ENABLED(USE_TINY_PRINTF)
/*
 * Note: this debug setup works by storing the strings in a fixed buffer
 */
स्थिर वर्ण zm_debug_buf[8192];
स्थिर वर्ण *zm_बहिर्गमनम् = zm_debug_buf;
स्थिर वर्ण *zm_बहिर्गमनम्_start = zm_debug_buf;

स्थिर पूर्णाङ्क
zm_dprintf(वर्ण *fmt, ...)
{
	पूर्णाङ्क len;
	va_list तर्काः;

	va_start(तर्काः, fmt);
	len = diag_vsprintf(zm_बहिर्गमनम्, fmt, तर्काः);
	va_अन्तः(तर्काः);
	zm_बहिर्गमनम् += len;
	प्रत्यागम len;
}

स्थिर शून्य
zm_flush (शून्य)
{
  zm_बहिर्गमनम् = zm_बहिर्गमनम्_start;
}

स्थिर शून्य
zm_dump_buf (शून्य *buf, पूर्णाङ्क len)
{

}

स्थिर अचिह्नित वर्ण zm_buf[2048];
स्थिर अचिह्नित वर्ण *zm_bp;

स्थिर शून्य
zm_new (शून्य)
{
  zm_bp = zm_buf;
}

स्थिर शून्य
zm_save (अचिह्नित वर्ण c)
{
  *zm_bp++ = c;
}

स्थिर शून्य
zm_dump (पूर्णाङ्क पङ्क्तिः)
{
  zm_dprintf ("Packet at line: %d\n", पङ्क्तिः);
  zm_dump_buf (zm_buf, zm_bp - zm_buf);
}

#परिभाषय ZM_DEBUG(x) x
#अन्यथा
#परिभाषय ZM_DEBUG(x)
#यदि_अन्तः

/* Wait for the line to go idle */
स्थिर शून्य
xyzModem_flush (शून्य)
{
  पूर्णाङ्क res;
  वर्ण c;
  यावत् (true)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
      यदि (!res)
	प्रत्यागम;
    }
}

स्थिर पूर्णाङ्क
xyzModem_get_hdr (शून्य)
{
  वर्ण c;
  पूर्णाङ्क res;
  bool hdr_प्राप्तम् = false;
  पूर्णाङ्क i, can_योग, hdr_chars;
  अचिह्नित ह्रस्व cksum;

  ZM_DEBUG (zm_new ());
  /* Find the start of a header */
  can_योग = 0;
  hdr_chars = 0;

  यदि (xyz.tx_ack)
    {
      CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
      xyz.tx_ack = false;
    }
  यावत् (!hdr_प्राप्तम्)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
      ZM_DEBUG (zm_save (c));
      यदि (res)
	{
	  hdr_chars++;
	  विकल्प (c)
	    {
	    पक्ष SOH:
	      xyz.योग_SOH++;
	      fallthrough;
	    पक्ष STX:
	      यदि (c == STX)
		xyz.योग_STX++;
	      hdr_प्राप्तम् = true;
	      भङ्ग;
	    पक्ष CAN:
	    पक्ष ETX:
	      xyz.योग_CAN++;
	      ZM_DEBUG (zm_dump (__LINE__));
	      यदि (++can_योग == xyz_Modem_CAN_गणनम्)
		{
		  प्रत्यागम xyzModem_cancel;
		}
	      अन्यथा
		{
		  /* Wait for multiple CAN to avoid early quits */
		  भङ्ग;
		}
	    पक्ष EOT:
	      /* EOT only supported if no noise */
	      यदि (hdr_chars == 1)
		{
		  CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		  ZM_DEBUG (zm_dprintf ("ACK on EOT #%d\n", __LINE__));
		  ZM_DEBUG (zm_dump (__LINE__));
		  प्रत्यागम xyzModem_eof;
		}
	    स्वभाव:
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
	  प्रत्यागम xyzModem_timeout;
	}
    }

  /* Header found, now read the data */
  res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (वर्ण *) &xyz.blk);
  ZM_DEBUG (zm_save (xyz.blk));
  यदि (!res)
    {
      ZM_DEBUG (zm_dump (__LINE__));
      प्रत्यागम xyzModem_timeout;
    }
  res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (वर्ण *) &xyz.cblk);
  ZM_DEBUG (zm_save (xyz.cblk));
  यदि (!res)
    {
      ZM_DEBUG (zm_dump (__LINE__));
      प्रत्यागम xyzModem_timeout;
    }
  xyz.len = (c == SOH) ? 128 : 1024;
  xyz.bufp = xyz.pkt;
  प्रति (i = 0; i < xyz.len; i++)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
      ZM_DEBUG (zm_save (c));
      यदि (res)
	{
	  xyz.pkt[i] = c;
	}
      अन्यथा
	{
	  ZM_DEBUG (zm_dump (__LINE__));
	  प्रत्यागम xyzModem_timeout;
	}
    }
  res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (वर्ण *) &xyz.crc1);
  ZM_DEBUG (zm_save (xyz.crc1));
  यदि (!res)
    {
      ZM_DEBUG (zm_dump (__LINE__));
      प्रत्यागम xyzModem_timeout;
    }
  यदि (xyz.crc_बहुलकम्)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (वर्ण *) &xyz.crc2);
      ZM_DEBUG (zm_save (xyz.crc2));
      यदि (!res)
	{
	  ZM_DEBUG (zm_dump (__LINE__));
	  प्रत्यागम xyzModem_timeout;
	}
    }
  ZM_DEBUG (zm_dump (__LINE__));
  /* Validate the message */
  यदि ((xyz.blk ^ xyz.cblk) != (अचिह्नित वर्ण) 0xFF)
    {
      ZM_DEBUG (zm_dprintf
		("Framing error - blk: %x/%x/%x\n", xyz.blk, xyz.cblk,
		 (xyz.blk ^ xyz.cblk)));
      ZM_DEBUG (zm_dump_buf (xyz.pkt, xyz.len));
      xyzModem_flush ();
      प्रत्यागम xyzModem_frame;
    }
  /* Verify checksum/CRC */
  यदि (xyz.crc_बहुलकम्)
    {
      cksum = crc16_ccitt(0, xyz.pkt, xyz.len);
      यदि (cksum != ((xyz.crc1 << 8) | xyz.crc2))
	{
	  ZM_DEBUG (zm_dprintf ("CRC error - recvd: %02x%02x, computed: %x\n",
				xyz.crc1, xyz.crc2, cksum & 0xFFFF));
	  प्रत्यागम xyzModem_cksum;
	}
    }
  अन्यथा
    {
      cksum = 0;
      प्रति (i = 0; i < xyz.len; i++)
	{
	  cksum += xyz.pkt[i];
	}
      यदि (xyz.crc1 != (cksum & 0xFF))
	{
	  ZM_DEBUG (zm_dprintf
		    ("Checksum error - recvd: %x, computed: %x\n", xyz.crc1,
		     cksum & 0xFF));
	  प्रत्यागम xyzModem_cksum;
	}
    }
  /* If we get here, the message passes [structural] muster */
  प्रत्यागम 0;
}

स्थिर
ulong
xyzModem_get_initial_timeout (शून्य)
{
  /* timeout is in seconds, non-positive timeout value is infinity */
#यदि CONFIG_IS_ENABLED(ENV_SUPPORT)
  ध्रुव वर्ण *timeout_str = env_get("loadxy_timeout");
  यदि (timeout_str)
    प्रत्यागम 1000 * simple_strtol(timeout_str, रिक्त, 10);
#यदि_अन्तः
  प्रत्यागम 1000 * CONFIG_CMD_LOADXY_TIMEOUT;
}

पूर्णाङ्क
xyz_Modem_प्रवाहः_open (connection_info_t * info, पूर्णाङ्क *err)
{
  पूर्णाङ्क stat = 0;
  पूर्णाङ्क retries = xyzModem_MAX_RETRIES;
  पूर्णाङ्क crc_retries = xyzModem_MAX_RETRIES_WITH_CRC;

/*    ZM_DEBUG(zm_out = zm_out_start); */
#यदि_परिभाषितम् xyzModem_zmodem
  यदि (info->बहुलकम् == xyzModem_zmodem)
    {
      *err = xyzModem_noZmodem;
      प्रत्यागम -1;
    }
#यदि_अन्तः

/* TODO: CHECK ! */
  पूर्णाङ्क dummy = 0;
  xyz.__chan = &dummy;
  xyz.len = 0;
  xyz.pad_pending = 0;
  xyz.pad_deliver = 0;
  xyz.crc_बहुलकम् = true;
  xyz.at_eof = false;
  xyz.tx_ack = false;
  xyz.बहुलकम् = info->बहुलकम्;
  xyz.योग_retries = 0;
  xyz.योग_SOH = 0;
  xyz.योग_STX = 0;
  xyz.योग_CAN = 0;
  xyz.read_दीर्घता = 0;
  xyz.file_दीर्घता = 0;
  xyz.first_xmodem_packet = false;
  xyz.initial_time = get_timer(0);
  xyz.timeout = xyzModem_get_initial_timeout();

  CYGACC_COMM_IF_PUTC (*xyz.__chan, (xyz.crc_बहुलकम् ? 'C' : NAK));

  यदि (xyz.बहुलकम् == xyzModem_xmodem)
    {
      /* X-modem doesn't have an information header - exit here */
      xyz.first_xmodem_packet = true;
      xyz.अग्र_blk = 1;
      प्रत्यागम 0;
    }

  यावत् (!(xyz.timeout && get_timer(xyz.initial_time) > xyz.timeout))
    {
      यदि (--retries <= 0)
        {
          retries = xyzModem_MAX_RETRIES;
          crc_retries = xyzModem_MAX_RETRIES_WITH_CRC;
          xyz.crc_बहुलकम् = true;
        }
      stat = xyzModem_get_hdr ();
      यदि (stat == 0)
	{
	  /* Y-modem file information header */
	  यदि (xyz.blk == 0)
	    {
	      /* skip filename */
	      यावत् (*xyz.bufp++);
	      /* get the length */
	      parse_अङ्कः ((वर्ण *) xyz.bufp, &xyz.file_दीर्घता, रिक्त, " ");
	      /* The rest of the file name data block quietly discarded */
	      xyz.tx_ack = true;
	    }
	  xyz.अग्र_blk = 1;
	  xyz.len = 0;
	  प्रत्यागम 0;
	}
      अन्यथा यदि (stat == xyzModem_timeout)
	{
	  यदि (--crc_retries <= 0)
	    xyz.crc_बहुलकम् = false;
	  CYGACC_CALL_IF_DELAY_US (5 * 100000);	/* Extra delay for startup */
	  CYGACC_COMM_IF_PUTC (*xyz.__chan, (xyz.crc_बहुलकम् ? 'C' : NAK));
	  xyz.योग_retries++;
	  ZM_DEBUG (zm_dprintf ("NAK (%d)\n", __LINE__));
	}
      यदि (stat == xyzModem_cancel)
	{
	  भङ्ग;
	}
    }
  *err = stat;
  ZM_DEBUG (zm_flush ());
  प्रत्यागम -1;
}

पूर्णाङ्क
xyz_Modem_प्रवाहः_read (वर्ण *buf, पूर्णाङ्क आकार, पूर्णाङ्क *err)
{
  पूर्णाङ्क stat, योग_2, len;
  पूर्णाङ्क retries;

  योग_2 = 0;
  stat = xyzModem_cancel;
  /* Try and get 'size' bytes into the buffer */
  यावत् (!xyz.at_eof && xyz.len >= 0 && (आकार > 0))
    {
      यदि (xyz.len == 0 && xyz.pad_deliver == 0)
	{
	  retries = xyzModem_MAX_RETRIES;
	  यावत् (retries-- > 0)
	    {
	      यदि (xyz.first_xmodem_packet && xyz.timeout &&
		  get_timer(xyz.initial_time) > xyz.timeout)
		{
		  *err = xyzModem_timeout;
		  xyz.len = -1;
		  प्रत्यागम योग_2;
		}

	      stat = xyzModem_get_hdr ();
	      यदि (stat == 0)
		{
		  यदि (xyz.बहुलकम् == xyzModem_xmodem && xyz.first_xmodem_packet)
		    xyz.first_xmodem_packet = false;
		  यदि (xyz.blk == xyz.अग्र_blk)
		    {
		      xyz.tx_ack = true;
		      ZM_DEBUG (zm_dprintf
				("ACK block %d (%d)\n", xyz.blk, __LINE__));
		      xyz.अग्र_blk = (xyz.अग्र_blk + 1) & 0xFF;

		      यदि (xyz.बहुलकम् == xyzModem_xmodem || xyz.file_दीर्घता == 0)
			{
			  /* Without a file length, EOF padding can't be
			   * distinguished from block data until the next
			   * frame arrives and we can check for EOT/EOF.
			   */
			  xyz.pad_deliver = xyz.pad_pending;
			  xyz.pad_pending = 0;
			  यदि ((xyz.bufp[xyz.len - 1] == EOF) &&
			      (xyz.bufp[xyz.len - 2] == EOF) &&
			      (xyz.bufp[xyz.len - 3] == EOF))
			    {
			      यावत् (xyz.len
				     && (xyz.bufp[xyz.len - 1] == EOF))
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
		      यदि (xyz.बहुलकम् != xyzModem_xmodem && 0 != xyz.file_दीर्घता)
			{
			  xyz.read_दीर्घता += xyz.len;
			  यदि (xyz.read_दीर्घता > xyz.file_दीर्घता)
			    {
			      xyz.len -= (xyz.read_दीर्घता - xyz.file_दीर्घता);
			    }
			}
		      भङ्ग;
		    }
		  अन्यथा यदि (xyz.blk == ((xyz.अग्र_blk - 1) & 0xFF))
		    {
		      /* Just re-ACK this so sender will get on with it */
		      CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		      अनुवर्त;	/* Need new header */
		    }
		  अन्यथा
		    {
		      stat = xyzModem_sequence;
		    }
		}
	      यदि (stat == xyzModem_cancel)
		{
		  भङ्ग;
		}
	      यदि (stat == xyzModem_eof)
		{
		  CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		  ZM_DEBUG (zm_dprintf ("ACK (%d)\n", __LINE__));
		  यदि (xyz.बहुलकम् == xyzModem_ymodem)
		    {
		      CYGACC_COMM_IF_PUTC (*xyz.__chan,
					   (xyz.crc_बहुलकम् ? 'C' : NAK));
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
		  भङ्ग;
		}
	      CYGACC_COMM_IF_PUTC (*xyz.__chan, (xyz.crc_बहुलकम् ? 'C' : NAK));
	      xyz.योग_retries++;
	      ZM_DEBUG (zm_dprintf ("NAK (%d)\n", __LINE__));
	    }
	  यदि (stat < 0 && (!xyz.first_xmodem_packet || stat != xyzModem_timeout))
	    {
	      *err = stat;
	      xyz.len = -1;
	      प्रत्यागम योग_2;
	    }
	}
      /* A ^Z run withheld from the previous block turned out to be data */
      यदि (xyz.pad_deliver > 0)
	{
	  len = xyz.pad_deliver;
	  यदि (आकार < len)
	    len = आकार;
	  स्मृतिं_पूरय (buf, EOF, len);
	  आकार -= len;
	  buf += len;
	  योग_2 += len;
	  xyz.pad_deliver -= len;
	}
      /* Don't "read" data from the EOF protocol package */
      यदि (!xyz.at_eof && xyz.len > 0)
	{
	  len = xyz.len;
	  यदि (आकार < len)
	    len = आकार;
	  स्मृतिं_प्रतिलिख (buf, xyz.bufp, len);
	  आकार -= len;
	  buf += len;
	  योग_2 += len;
	  xyz.len -= len;
	  xyz.bufp += len;
	}
    }
  प्रत्यागम योग_2;
}

शून्य
xyz_Modem_प्रवाहः_close (पूर्णाङ्क *err)
{
  ZM_DEBUG (zm_dprintf
    ("xyzModem - %s mode, %d(SOH)/%d(STX)/%d(CAN) packets, %d retries\n",
     xyz.crc_बहुलकम् ? "CRC" : "Cksum", xyz.योग_SOH, xyz.योग_STX,
     xyz.योग_CAN, xyz.योग_retries));
  ZM_DEBUG (zm_flush ());
}

/* Need to be able to clean out the input buffer, so have to take the */
/* getc */
शून्य
xyz_Modem_प्रवाहः_terminate (bool abort, पूर्णाङ्क (*getc) (शून्य))
{
  पूर्णाङ्क c;

  यदि (abort)
    {
      ZM_DEBUG (zm_dprintf ("!!!! TRANSFER ABORT !!!!\n"));
      विकल्प (xyz.बहुलकम्)
	{
	पक्ष xyzModem_xmodem:
	पक्ष xyzModem_ymodem:
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
	  भङ्ग;
#यदि_परिभाषितम् xyzModem_zmodem
	पक्ष xyzModem_zmodem:
	  /* Might support it some day I suppose. */
#यदि_अन्तः
	  भङ्ग;
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
      यावत् ((c = (*getc) ()) > -1)
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

वर्ण *
xyzModem_error (पूर्णाङ्क err)
{
  विकल्प (err)
    {
    पक्ष xyzModem_access:
      प्रत्यागम "Can't access file";
      भङ्ग;
    पक्ष xyzModem_noZmodem:
      प्रत्यागम "Sorry, zModem not available yet";
      भङ्ग;
    पक्ष xyzModem_timeout:
      प्रत्यागम "Timed out";
      भङ्ग;
    पक्ष xyzModem_eof:
      प्रत्यागम "End of file";
      भङ्ग;
    पक्ष xyzModem_cancel:
      प्रत्यागम "Cancelled";
      भङ्ग;
    पक्ष xyzModem_frame:
      प्रत्यागम "Invalid framing";
      भङ्ग;
    पक्ष xyzModem_cksum:
      प्रत्यागम "CRC/checksum error";
      भङ्ग;
    पक्ष xyzModem_sequence:
      प्रत्यागम "Block sequence error";
      भङ्ग;
    स्वभाव:
      प्रत्यागम "Unknown error";
      भङ्ग;
    }
}

/*
 * RedBoot interface
 */
