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
#અન્તર્ભાવ <xyzModem.h>
#અન્તર્ભાવ <stdarg.h>
#અન્તર્ભાવ <time.h>
#અન્તર્ભાવ <u-boot/crc.h>
#અન્તર્ભાવ <watchdog.h>
#અન્તર્ભાવ <env.h>
#અન્તર્ભાવ <vsprintf.h>

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
#define EOF 0x1A		/* ^Z for DOS officionados */

/* Data & state local to the protocol */
સ્થિર સંરચના
{
  પૂર્ણાંક *__chan;
  unsigned અક્ષર pkt[1024], *bufp;
  પૂર્ણાંક pad_pending;	/* trailing ^Z run withheld from the previous block */
  પૂર્ણાંક pad_deliver;	/* ^Z bytes to emit before the next data */
  unsigned અક્ષર blk, cblk, crc1, crc2;
  unsigned અક્ષર next_blk;	/* Expected block */
  પૂર્ણાંક len, mode, total_retries;
  પૂર્ણાંક total_SOH, total_STX, total_CAN;
  bool crc_mode, at_eof, tx_ack;
  bool first_xmodem_packet;
  ulong initial_time, timeout;
  unsigned long file_length, read_length;
} xyz;

#define xyzModem_CHAR_TIMEOUT            2000	/* 2 seconds */
#define xyzModem_MAX_RETRIES             20
#define xyzModem_MAX_RETRIES_WITH_CRC    10
#define xyz_Modem_CAN_ગણતરી                3	/* Wait for 3 CAN before quitting */

પ્રકારવ્યાખ્યા પૂર્ણાંક cyg_int32;
સ્થિર પૂર્ણાંક
CYGACC_COMM_IF_GETC_TIMEOUT (અક્ષર chan, અક્ષર *c)
{

  ulong now = get_timer(0);
  schedule();
  જ્યારે (!tstc ())
    {
      જો (get_timer(now) > xyzModem_CHAR_TIMEOUT)
        તોડો;
    }
  જો (tstc ())
    {
      *c = getchar();
      પાછા 1;
    }
  પાછા 0;
}

સ્થિર શૂન્ય
CYGACC_COMM_IF_PUTC (અક્ષર x, અક્ષર y)
{
  putc (y);
}

/* Validate a hex character */
__inline__ સ્થિર bool
_is_hex (અક્ષર c)
{
  પાછા (((c >= '0') && (c <= '9')) ||
	  ((c >= 'A') && (c <= 'F')) || ((c >= 'a') && (c <= 'f')));
}

/* Convert a single hex nibble */
__inline__ સ્થિર પૂર્ણાંક
_from_hex (અક્ષર c)
{
  પૂર્ણાંક ret = 0;

  જો ((c >= '0') && (c <= '9'))
    {
      ret = (c - '0');
    }
  નહીંતર જો ((c >= 'a') && (c <= 'f'))
    {
      ret = (c - 'a' + 0x0a);
    }
  નહીંતર જો ((c >= 'A') && (c <= 'F'))
    {
      ret = (c - 'A' + 0x0A);
    }
  પાછા ret;
}

/* Convert a character to lower case */
__inline__ સ્થિર અક્ષર
_tolower (અક્ષર c)
{
  જો ((c >= 'A') && (c <= 'Z'))
    {
      c = (c - 'A') + 'a';
    }
  પાછા c;
}

/* Parse (scan) a number */
સ્થિર bool
parse_અંક (અક્ષર *s, unsigned long *val, અક્ષર **es, અક્ષર *delim)
{
  bool first = true;
  પૂર્ણાંક radix = 10;
  અક્ષર c;
  unsigned long result = 0;
  પૂર્ણાંક digit;

  જ્યારે (*s == ' ')
    s++;
  જ્યારે (*s)
    {
      જો (first && (s[0] == '0') && (_tolower (s[1]) == 'x'))
	{
	  radix = 16;
	  s += 2;
	}
      first = false;
      c = *s++;
      જો (_is_hex (c) && ((digit = _from_hex (c)) < radix))
	{
	  /* Valid digit */
	  result = (result * radix) + digit;
	}
      નહીંતર
	{
	  જો (delim != (અક્ષર *) 0)
	    {
	      /* See if this character is one of the delimiters */
	      અક્ષર *dp = delim;
	      જ્યારે (*dp && (c != *dp))
		dp++;
	      જો (*dp)
		તોડો;		/* Found a good delimiter */
	    }
	  પાછા false;		/* Malformatted number */
	}
    }
  *val = result;
  જો (es != (અક્ષર **) 0)
    {
      *es = s;
    }
  પાછા true;
}

#જો defined(DEBUG) && !CONFIG_IS_ENABLED(USE_TINY_PRINTF)
/*
 * Note: this debug setup works by storing the strings in a fixed buffer
 */
સ્થિર અક્ષર zm_debug_buf[8192];
સ્થિર અક્ષર *zm_out = zm_debug_buf;
સ્થિર અક્ષર *zm_out_start = zm_debug_buf;

સ્થિર પૂર્ણાંક
zm_dprintf(અક્ષર *fmt, ...)
{
	પૂર્ણાંક len;
	va_list args;

	va_start(args, fmt);
	len = diag_vsprintf(zm_out, fmt, args);
	va_end(args);
	zm_out += len;
	પાછા len;
}

સ્થિર શૂન્ય
zm_flush (શૂન્ય)
{
  zm_out = zm_out_start;
}

સ્થિર શૂન્ય
zm_dump_buf (શૂન્ય *buf, પૂર્ણાંક len)
{

}

સ્થિર unsigned અક્ષર zm_buf[2048];
સ્થિર unsigned અક્ષર *zm_bp;

સ્થિર શૂન્ય
zm_new (શૂન્ય)
{
  zm_bp = zm_buf;
}

સ્થિર શૂન્ય
zm_save (unsigned અક્ષર c)
{
  *zm_bp++ = c;
}

સ્થિર શૂન્ય
zm_dump (પૂર્ણાંક લીટી)
{
  zm_dprintf ("Packet at line: %d\n", લીટી);
  zm_dump_buf (zm_buf, zm_bp - zm_buf);
}

#define ZM_DEBUG(x) x
#નહીંતર
#define ZM_DEBUG(x)
#endif

/* Wait for the line to go idle */
સ્થિર શૂન્ય
xyzModem_flush (શૂન્ય)
{
  પૂર્ણાંક res;
  અક્ષર c;
  જ્યારે (true)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
      જો (!res)
	પાછા;
    }
}

સ્થિર પૂર્ણાંક
xyzModem_get_hdr (શૂન્ય)
{
  અક્ષર c;
  પૂર્ણાંક res;
  bool hdr_found = false;
  પૂર્ણાંક i, can_total, hdr_chars;
  unsigned short cksum;

  ZM_DEBUG (zm_new ());
  /* Find the start of a header */
  can_total = 0;
  hdr_chars = 0;

  જો (xyz.tx_ack)
    {
      CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
      xyz.tx_ack = false;
    }
  જ્યારે (!hdr_found)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
      ZM_DEBUG (zm_save (c));
      જો (res)
	{
	  hdr_chars++;
	  પસંદગી (c)
	    {
	    સ્થિતિ SOH:
	      xyz.total_SOH++;
	      fallthrough;
	    સ્થિતિ STX:
	      જો (c == STX)
		xyz.total_STX++;
	      hdr_found = true;
	      તોડો;
	    સ્થિતિ CAN:
	    સ્થિતિ ETX:
	      xyz.total_CAN++;
	      ZM_DEBUG (zm_dump (__LINE__));
	      જો (++can_total == xyz_Modem_CAN_ગણતરી)
		{
		  પાછા xyzModem_cancel;
		}
	      નહીંતર
		{
		  /* Wait for multiple CAN to avoid early quits */
		  તોડો;
		}
	    સ્થિતિ EOT:
	      /* EOT only supported if no noise */
	      જો (hdr_chars == 1)
		{
		  CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		  ZM_DEBUG (zm_dprintf ("ACK on EOT #%d\n", __LINE__));
		  ZM_DEBUG (zm_dump (__LINE__));
		  પાછા xyzModem_eof;
		}
	    મૂળભૂત:
	      /* Ignore, waiting for start of header */
	      ;
	    }
	}
      નહીંતર
	{
	  /* Data stream timed out */
	  xyzModem_flush ();	/* Toss any current input */
	  ZM_DEBUG (zm_dump (__LINE__));
	  CYGACC_CALL_IF_DELAY_US ((cyg_int32) 250000);
	  પાછા xyzModem_timeout;
	}
    }

  /* Header found, now read the data */
  res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (અક્ષર *) &xyz.blk);
  ZM_DEBUG (zm_save (xyz.blk));
  જો (!res)
    {
      ZM_DEBUG (zm_dump (__LINE__));
      પાછા xyzModem_timeout;
    }
  res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (અક્ષર *) &xyz.cblk);
  ZM_DEBUG (zm_save (xyz.cblk));
  જો (!res)
    {
      ZM_DEBUG (zm_dump (__LINE__));
      પાછા xyzModem_timeout;
    }
  xyz.len = (c == SOH) ? 128 : 1024;
  xyz.bufp = xyz.pkt;
  માટે (i = 0; i < xyz.len; i++)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
      ZM_DEBUG (zm_save (c));
      જો (res)
	{
	  xyz.pkt[i] = c;
	}
      નહીંતર
	{
	  ZM_DEBUG (zm_dump (__LINE__));
	  પાછા xyzModem_timeout;
	}
    }
  res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (અક્ષર *) &xyz.crc1);
  ZM_DEBUG (zm_save (xyz.crc1));
  જો (!res)
    {
      ZM_DEBUG (zm_dump (__LINE__));
      પાછા xyzModem_timeout;
    }
  જો (xyz.crc_mode)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (અક્ષર *) &xyz.crc2);
      ZM_DEBUG (zm_save (xyz.crc2));
      જો (!res)
	{
	  ZM_DEBUG (zm_dump (__LINE__));
	  પાછા xyzModem_timeout;
	}
    }
  ZM_DEBUG (zm_dump (__LINE__));
  /* Validate the message */
  જો ((xyz.blk ^ xyz.cblk) != (unsigned અક્ષર) 0xFF)
    {
      ZM_DEBUG (zm_dprintf
		("Framing error - blk: %x/%x/%x\n", xyz.blk, xyz.cblk,
		 (xyz.blk ^ xyz.cblk)));
      ZM_DEBUG (zm_dump_buf (xyz.pkt, xyz.len));
      xyzModem_flush ();
      પાછા xyzModem_frame;
    }
  /* Verify checksum/CRC */
  જો (xyz.crc_mode)
    {
      cksum = crc16_ccitt(0, xyz.pkt, xyz.len);
      જો (cksum != ((xyz.crc1 << 8) | xyz.crc2))
	{
	  ZM_DEBUG (zm_dprintf ("CRC error - recvd: %02x%02x, computed: %x\n",
				xyz.crc1, xyz.crc2, cksum & 0xFFFF));
	  પાછા xyzModem_cksum;
	}
    }
  નહીંતર
    {
      cksum = 0;
      માટે (i = 0; i < xyz.len; i++)
	{
	  cksum += xyz.pkt[i];
	}
      જો (xyz.crc1 != (cksum & 0xFF))
	{
	  ZM_DEBUG (zm_dprintf
		    ("Checksum error - recvd: %x, computed: %x\n", xyz.crc1,
		     cksum & 0xFF));
	  પાછા xyzModem_cksum;
	}
    }
  /* If we get here, the message passes [structural] muster */
  પાછા 0;
}

સ્થિર
ulong
xyzModem_get_initial_timeout (શૂન્ય)
{
  /* timeout is in seconds, non-positive timeout value is infinity */
#જો CONFIG_IS_ENABLED(ENV_SUPPORT)
  અચલ અક્ષર *timeout_str = env_get("loadxy_timeout");
  જો (timeout_str)
    પાછા 1000 * simple_strtol(timeout_str, ખાલી, 10);
#endif
  પાછા 1000 * CONFIG_CMD_LOADXY_TIMEOUT;
}

પૂર્ણાંક
xyzModem_stream_open (connection_info_t * info, પૂર્ણાંક *err)
{
  પૂર્ણાંક stat = 0;
  પૂર્ણાંક retries = xyzModem_MAX_RETRIES;
  પૂર્ણાંક crc_retries = xyzModem_MAX_RETRIES_WITH_CRC;

/*    ZM_DEBUG(zm_out = zm_out_start); */
#ifdef xyzModem_zmodem
  જો (info->mode == xyzModem_zmodem)
    {
      *err = xyzModem_noZmodem;
      પાછા -1;
    }
#endif

/* TODO: CHECK ! */
  પૂર્ણાંક dummy = 0;
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

  જો (xyz.mode == xyzModem_xmodem)
    {
      /* X-modem doesn't have an information header - exit here */
      xyz.first_xmodem_packet = true;
      xyz.next_blk = 1;
      પાછા 0;
    }

  જ્યારે (!(xyz.timeout && get_timer(xyz.initial_time) > xyz.timeout))
    {
      જો (--retries <= 0)
        {
          retries = xyzModem_MAX_RETRIES;
          crc_retries = xyzModem_MAX_RETRIES_WITH_CRC;
          xyz.crc_mode = true;
        }
      stat = xyzModem_get_hdr ();
      જો (stat == 0)
	{
	  /* Y-modem file information header */
	  જો (xyz.blk == 0)
	    {
	      /* skip filename */
	      જ્યારે (*xyz.bufp++);
	      /* get the length */
	      parse_અંક ((અક્ષર *) xyz.bufp, &xyz.file_length, ખાલી, " ");
	      /* The rest of the file name data block quietly discarded */
	      xyz.tx_ack = true;
	    }
	  xyz.next_blk = 1;
	  xyz.len = 0;
	  પાછા 0;
	}
      નહીંતર જો (stat == xyzModem_timeout)
	{
	  જો (--crc_retries <= 0)
	    xyz.crc_mode = false;
	  CYGACC_CALL_IF_DELAY_US (5 * 100000);	/* Extra delay for startup */
	  CYGACC_COMM_IF_PUTC (*xyz.__chan, (xyz.crc_mode ? 'C' : NAK));
	  xyz.total_retries++;
	  ZM_DEBUG (zm_dprintf ("NAK (%d)\n", __LINE__));
	}
      જો (stat == xyzModem_cancel)
	{
	  તોડો;
	}
    }
  *err = stat;
  ZM_DEBUG (zm_flush ());
  પાછા -1;
}

પૂર્ણાંક
xyzModem_stream_read (અક્ષર *buf, પૂર્ણાંક size, પૂર્ણાંક *err)
{
  પૂર્ણાંક stat, total, len;
  પૂર્ણાંક retries;

  total = 0;
  stat = xyzModem_cancel;
  /* Try and get 'size' bytes into the buffer */
  જ્યારે (!xyz.at_eof && xyz.len >= 0 && (size > 0))
    {
      જો (xyz.len == 0 && xyz.pad_deliver == 0)
	{
	  retries = xyzModem_MAX_RETRIES;
	  જ્યારે (retries-- > 0)
	    {
	      જો (xyz.first_xmodem_packet && xyz.timeout &&
		  get_timer(xyz.initial_time) > xyz.timeout)
		{
		  *err = xyzModem_timeout;
		  xyz.len = -1;
		  પાછા total;
		}

	      stat = xyzModem_get_hdr ();
	      જો (stat == 0)
		{
		  જો (xyz.mode == xyzModem_xmodem && xyz.first_xmodem_packet)
		    xyz.first_xmodem_packet = false;
		  જો (xyz.blk == xyz.next_blk)
		    {
		      xyz.tx_ack = true;
		      ZM_DEBUG (zm_dprintf
				("ACK block %d (%d)\n", xyz.blk, __LINE__));
		      xyz.next_blk = (xyz.next_blk + 1) & 0xFF;

		      જો (xyz.mode == xyzModem_xmodem || xyz.file_length == 0)
			{
			  /* Without a file length, EOF padding can't be
			   * distinguished from block data until the next
			   * frame arrives and we can check for EOT/EOF.
			   */
			  xyz.pad_deliver = xyz.pad_pending;
			  xyz.pad_pending = 0;
			  જો ((xyz.bufp[xyz.len - 1] == EOF) &&
			      (xyz.bufp[xyz.len - 2] == EOF) &&
			      (xyz.bufp[xyz.len - 3] == EOF))
			    {
			      જ્યારે (xyz.len
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
		      જો (xyz.mode != xyzModem_xmodem && 0 != xyz.file_length)
			{
			  xyz.read_length += xyz.len;
			  જો (xyz.read_length > xyz.file_length)
			    {
			      xyz.len -= (xyz.read_length - xyz.file_length);
			    }
			}
		      તોડો;
		    }
		  નહીંતર જો (xyz.blk == ((xyz.next_blk - 1) & 0xFF))
		    {
		      /* Just re-ACK this so sender will get on with it */
		      CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		      ચાલુ;	/* Need new header */
		    }
		  નહીંતર
		    {
		      stat = xyzModem_sequence;
		    }
		}
	      જો (stat == xyzModem_cancel)
		{
		  તોડો;
		}
	      જો (stat == xyzModem_eof)
		{
		  CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		  ZM_DEBUG (zm_dprintf ("ACK (%d)\n", __LINE__));
		  જો (xyz.mode == xyzModem_ymodem)
		    {
		      CYGACC_COMM_IF_PUTC (*xyz.__chan,
					   (xyz.crc_mode ? 'C' : NAK));
		      xyz.total_retries++;
		      ZM_DEBUG (zm_dprintf ("Reading Final Header\n"));
		      stat = xyzModem_get_hdr ();
		      CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		      ZM_DEBUG (zm_dprintf ("FINAL ACK (%d)\n", __LINE__));
		    }
		  નહીંતર
		    stat = 0;

		  xyz.pad_pending = 0;	/* EOT: withheld ^Z run was padding */
		  xyz.at_eof = true;
		  તોડો;
		}
	      CYGACC_COMM_IF_PUTC (*xyz.__chan, (xyz.crc_mode ? 'C' : NAK));
	      xyz.total_retries++;
	      ZM_DEBUG (zm_dprintf ("NAK (%d)\n", __LINE__));
	    }
	  જો (stat < 0 && (!xyz.first_xmodem_packet || stat != xyzModem_timeout))
	    {
	      *err = stat;
	      xyz.len = -1;
	      પાછા total;
	    }
	}
      /* A ^Z run withheld from the previous block turned out to be data */
      જો (xyz.pad_deliver > 0)
	{
	  len = xyz.pad_deliver;
	  જો (size < len)
	    len = size;
	  memset (buf, EOF, len);
	  size -= len;
	  buf += len;
	  total += len;
	  xyz.pad_deliver -= len;
	}
      /* Don't "read" data from the EOF protocol package */
      જો (!xyz.at_eof && xyz.len > 0)
	{
	  len = xyz.len;
	  જો (size < len)
	    len = size;
	  memcpy (buf, xyz.bufp, len);
	  size -= len;
	  buf += len;
	  total += len;
	  xyz.len -= len;
	  xyz.bufp += len;
	}
    }
  પાછા total;
}

શૂન્ય
xyzModem_stream_close (પૂર્ણાંક *err)
{
  ZM_DEBUG (zm_dprintf
    ("xyzModem - %s mode, %d(SOH)/%d(STX)/%d(CAN) packets, %d retries\n",
     xyz.crc_mode ? "CRC" : "Cksum", xyz.total_SOH, xyz.total_STX,
     xyz.total_CAN, xyz.total_retries));
  ZM_DEBUG (zm_flush ());
}

/* Need to be able to clean out the input buffer, so have to take the */
/* getc */
શૂન્ય
xyzModem_stream_terminate (bool abort, પૂર્ણાંક (*getc) (શૂન્ય))
{
  પૂર્ણાંક c;

  જો (abort)
    {
      ZM_DEBUG (zm_dprintf ("!!!! TRANSFER ABORT !!!!\n"));
      પસંદગી (xyz.mode)
	{
	સ્થિતિ xyzModem_xmodem:
	સ્થિતિ xyzModem_ymodem:
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
	  તોડો;
#ifdef xyzModem_zmodem
	સ્થિતિ xyzModem_zmodem:
	  /* Might support it some day I suppose. */
#endif
	  તોડો;
	}
    }
  નહીંતર
    {
      ZM_DEBUG (zm_dprintf ("Engaging cleanup mode...\n"));
      /*
       * Consume any trailing crap left in the inbuffer from
       * previous received blocks. Since very few files are an exact multiple
       * of the transfer block size, there will almost always be some gunk here.
       * If we don't eat it now, RedBoot will think the user typed it.
       */
      ZM_DEBUG (zm_dprintf ("Trailing gunk:\n"));
      જ્યારે ((c = (*getc) ()) > -1)
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

અક્ષર *
xyzModem_error (પૂર્ણાંક err)
{
  પસંદગી (err)
    {
    સ્થિતિ xyzModem_access:
      પાછા "Can't access file";
      તોડો;
    સ્થિતિ xyzModem_noZmodem:
      પાછા "Sorry, zModem not available yet";
      તોડો;
    સ્થિતિ xyzModem_timeout:
      પાછા "Timed out";
      તોડો;
    સ્થિતિ xyzModem_eof:
      પાછા "End of file";
      તોડો;
    સ્થિતિ xyzModem_cancel:
      પાછા "Cancelled";
      તોડો;
    સ્થિતિ xyzModem_frame:
      પાછા "Invalid framing";
      તોડો;
    સ્થિતિ xyzModem_cksum:
      પાછા "CRC/checksum error";
      તોડો;
    સ્થિતિ xyzModem_sequence:
      પાછા "Block sequence error";
      તોડો;
    મૂળભૂત:
      પાછા "Unknown error";
      તોડો;
    }
}

/*
 * RedBoot interface
 */
