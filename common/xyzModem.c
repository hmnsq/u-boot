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
#அந்தர்பாவ <xyzModem.h>
#அந்தர்பாவ <stdarg.h>
#அந்தர்பாவ <time.h>
#அந்தர்பாவ <u-boot/crc.h>
#அந்தர்பாவ <watchdog.h>
#அந்தர்பாவ <env.h>
#அந்தர்பாவ <vsprintf.h>

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
நிலையான கட்டமைப்பு
{
  முழு *__chan;
  unsigned எழுத்து pkt[1024], *bufp;
  முழு pad_pending;	/* trailing ^Z run withheld from the previous block */
  முழு pad_deliver;	/* ^Z bytes to emit before the next data */
  unsigned எழுத்து blk, cblk, crc1, crc2;
  unsigned எழுத்து next_blk;	/* Expected block */
  முழு len, mode, total_retries;
  முழு total_SOH, total_STX, total_CAN;
  bool crc_mode, at_eof, tx_ack;
  bool first_xmodem_packet;
  ulong initial_time, timeout;
  unsigned long file_length, read_length;
} xyz;

#define xyzModem_CHAR_TIMEOUT            2000	/* 2 seconds */
#define xyzModem_MAX_RETRIES             20
#define xyzModem_MAX_RETRIES_WITH_CRC    10
#define xyz_Modem_CAN_மொத்தம்                3	/* Wait for 3 CAN before quitting */

வகைவரை முழு cyg_int32;
நிலையான முழு
CYGACC_COMM_IF_GETC_TIMEOUT (எழுத்து chan, எழுத்து *c)
{

  ulong now = get_timer(0);
  schedule();
  போது (!tstc ())
    {
      எனில் (get_timer(now) > xyzModem_CHAR_TIMEOUT)
        நிறுத்து;
    }
  எனில் (tstc ())
    {
      *c = getchar();
      திரும்பு 1;
    }
  திரும்பு 0;
}

நிலையான வெறுமை
CYGACC_COMM_IF_PUTC (எழுத்து x, எழுத்து y)
{
  putc (y);
}

/* Validate a hex character */
__inline__ நிலையான bool
_is_hex (எழுத்து c)
{
  திரும்பு (((c >= '0') && (c <= '9')) ||
	  ((c >= 'A') && (c <= 'F')) || ((c >= 'a') && (c <= 'f')));
}

/* Convert a single hex nibble */
__inline__ நிலையான முழு
_from_hex (எழுத்து c)
{
  முழு ret = 0;

  எனில் ((c >= '0') && (c <= '9'))
    {
      ret = (c - '0');
    }
  இல்லையெனில் எனில் ((c >= 'a') && (c <= 'f'))
    {
      ret = (c - 'a' + 0x0a);
    }
  இல்லையெனில் எனில் ((c >= 'A') && (c <= 'F'))
    {
      ret = (c - 'A' + 0x0A);
    }
  திரும்பு ret;
}

/* Convert a character to lower case */
__inline__ நிலையான எழுத்து
_tolower (எழுத்து c)
{
  எனில் ((c >= 'A') && (c <= 'Z'))
    {
      c = (c - 'A') + 'a';
    }
  திரும்பு c;
}

/* Parse (scan) a number */
நிலையான bool
parse_இலக்கம் (எழுத்து *s, unsigned long *val, எழுத்து **es, எழுத்து *delim)
{
  bool first = true;
  முழு radix = 10;
  எழுத்து c;
  unsigned long result = 0;
  முழு digit;

  போது (*s == ' ')
    s++;
  போது (*s)
    {
      எனில் (first && (s[0] == '0') && (_tolower (s[1]) == 'x'))
	{
	  radix = 16;
	  s += 2;
	}
      first = false;
      c = *s++;
      எனில் (_is_hex (c) && ((digit = _from_hex (c)) < radix))
	{
	  /* Valid digit */
	  result = (result * radix) + digit;
	}
      இல்லையெனில்
	{
	  எனில் (delim != (எழுத்து *) 0)
	    {
	      /* See if this character is one of the delimiters */
	      எழுத்து *dp = delim;
	      போது (*dp && (c != *dp))
		dp++;
	      எனில் (*dp)
		நிறுத்து;		/* Found a good delimiter */
	    }
	  திரும்பு false;		/* Malformatted number */
	}
    }
  *val = result;
  எனில் (es != (எழுத்து **) 0)
    {
      *es = s;
    }
  திரும்பு true;
}

#எனில் defined(DEBUG) && !CONFIG_IS_ENABLED(USE_TINY_PRINTF)
/*
 * Note: this debug setup works by storing the strings in a fixed buffer
 */
நிலையான எழுத்து zm_debug_buf[8192];
நிலையான எழுத்து *zm_out = zm_debug_buf;
நிலையான எழுத்து *zm_out_start = zm_debug_buf;

நிலையான முழு
zm_dprintf(எழுத்து *fmt, ...)
{
	முழு len;
	va_list args;

	va_start(args, fmt);
	len = diag_vsprintf(zm_out, fmt, args);
	va_end(args);
	zm_out += len;
	திரும்பு len;
}

நிலையான வெறுமை
zm_flush (வெறுமை)
{
  zm_out = zm_out_start;
}

நிலையான வெறுமை
zm_dump_buf (வெறுமை *buf, முழு len)
{

}

நிலையான unsigned எழுத்து zm_buf[2048];
நிலையான unsigned எழுத்து *zm_bp;

நிலையான வெறுமை
zm_new (வெறுமை)
{
  zm_bp = zm_buf;
}

நிலையான வெறுமை
zm_save (unsigned எழுத்து c)
{
  *zm_bp++ = c;
}

நிலையான வெறுமை
zm_dump (முழு வரி)
{
  zm_dprintf ("Packet at line: %d\n", வரி);
  zm_dump_buf (zm_buf, zm_bp - zm_buf);
}

#define ZM_DEBUG(x) x
#இல்லையெனில்
#define ZM_DEBUG(x)
#endif

/* Wait for the line to go idle */
நிலையான வெறுமை
xyzModem_flush (வெறுமை)
{
  முழு res;
  எழுத்து c;
  போது (true)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
      எனில் (!res)
	திரும்பு;
    }
}

நிலையான முழு
xyzModem_get_hdr (வெறுமை)
{
  எழுத்து c;
  முழு res;
  bool hdr_found = false;
  முழு i, can_total, hdr_chars;
  unsigned short cksum;

  ZM_DEBUG (zm_new ());
  /* Find the start of a header */
  can_total = 0;
  hdr_chars = 0;

  எனில் (xyz.tx_ack)
    {
      CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
      xyz.tx_ack = false;
    }
  போது (!hdr_found)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
      ZM_DEBUG (zm_save (c));
      எனில் (res)
	{
	  hdr_chars++;
	  தேர்வு (c)
	    {
	    நிலை SOH:
	      xyz.total_SOH++;
	      fallthrough;
	    நிலை STX:
	      எனில் (c == STX)
		xyz.total_STX++;
	      hdr_found = true;
	      நிறுத்து;
	    நிலை CAN:
	    நிலை ETX:
	      xyz.total_CAN++;
	      ZM_DEBUG (zm_dump (__LINE__));
	      எனில் (++can_total == xyz_Modem_CAN_மொத்தம்)
		{
		  திரும்பு xyzModem_cancel;
		}
	      இல்லையெனில்
		{
		  /* Wait for multiple CAN to avoid early quits */
		  நிறுத்து;
		}
	    நிலை EOT:
	      /* EOT only supported if no noise */
	      எனில் (hdr_chars == 1)
		{
		  CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		  ZM_DEBUG (zm_dprintf ("ACK on EOT #%d\n", __LINE__));
		  ZM_DEBUG (zm_dump (__LINE__));
		  திரும்பு xyzModem_eof;
		}
	    இயல்பு:
	      /* Ignore, waiting for start of header */
	      ;
	    }
	}
      இல்லையெனில்
	{
	  /* Data stream timed out */
	  xyzModem_flush ();	/* Toss any current input */
	  ZM_DEBUG (zm_dump (__LINE__));
	  CYGACC_CALL_IF_DELAY_US ((cyg_int32) 250000);
	  திரும்பு xyzModem_timeout;
	}
    }

  /* Header found, now read the data */
  res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (எழுத்து *) &xyz.blk);
  ZM_DEBUG (zm_save (xyz.blk));
  எனில் (!res)
    {
      ZM_DEBUG (zm_dump (__LINE__));
      திரும்பு xyzModem_timeout;
    }
  res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (எழுத்து *) &xyz.cblk);
  ZM_DEBUG (zm_save (xyz.cblk));
  எனில் (!res)
    {
      ZM_DEBUG (zm_dump (__LINE__));
      திரும்பு xyzModem_timeout;
    }
  xyz.len = (c == SOH) ? 128 : 1024;
  xyz.bufp = xyz.pkt;
  க்கு (i = 0; i < xyz.len; i++)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
      ZM_DEBUG (zm_save (c));
      எனில் (res)
	{
	  xyz.pkt[i] = c;
	}
      இல்லையெனில்
	{
	  ZM_DEBUG (zm_dump (__LINE__));
	  திரும்பு xyzModem_timeout;
	}
    }
  res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (எழுத்து *) &xyz.crc1);
  ZM_DEBUG (zm_save (xyz.crc1));
  எனில் (!res)
    {
      ZM_DEBUG (zm_dump (__LINE__));
      திரும்பு xyzModem_timeout;
    }
  எனில் (xyz.crc_mode)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (எழுத்து *) &xyz.crc2);
      ZM_DEBUG (zm_save (xyz.crc2));
      எனில் (!res)
	{
	  ZM_DEBUG (zm_dump (__LINE__));
	  திரும்பு xyzModem_timeout;
	}
    }
  ZM_DEBUG (zm_dump (__LINE__));
  /* Validate the message */
  எனில் ((xyz.blk ^ xyz.cblk) != (unsigned எழுத்து) 0xFF)
    {
      ZM_DEBUG (zm_dprintf
		("Framing error - blk: %x/%x/%x\n", xyz.blk, xyz.cblk,
		 (xyz.blk ^ xyz.cblk)));
      ZM_DEBUG (zm_dump_buf (xyz.pkt, xyz.len));
      xyzModem_flush ();
      திரும்பு xyzModem_frame;
    }
  /* Verify checksum/CRC */
  எனில் (xyz.crc_mode)
    {
      cksum = crc16_ccitt(0, xyz.pkt, xyz.len);
      எனில் (cksum != ((xyz.crc1 << 8) | xyz.crc2))
	{
	  ZM_DEBUG (zm_dprintf ("CRC error - recvd: %02x%02x, computed: %x\n",
				xyz.crc1, xyz.crc2, cksum & 0xFFFF));
	  திரும்பு xyzModem_cksum;
	}
    }
  இல்லையெனில்
    {
      cksum = 0;
      க்கு (i = 0; i < xyz.len; i++)
	{
	  cksum += xyz.pkt[i];
	}
      எனில் (xyz.crc1 != (cksum & 0xFF))
	{
	  ZM_DEBUG (zm_dprintf
		    ("Checksum error - recvd: %x, computed: %x\n", xyz.crc1,
		     cksum & 0xFF));
	  திரும்பு xyzModem_cksum;
	}
    }
  /* If we get here, the message passes [structural] muster */
  திரும்பு 0;
}

நிலையான
ulong
xyzModem_get_initial_timeout (வெறுமை)
{
  /* timeout is in seconds, non-positive timeout value is infinity */
#எனில் CONFIG_IS_ENABLED(ENV_SUPPORT)
  மாறிலி எழுத்து *timeout_str = env_get("loadxy_timeout");
  எனில் (timeout_str)
    திரும்பு 1000 * simple_strtol(timeout_str, வெற்று, 10);
#endif
  திரும்பு 1000 * CONFIG_CMD_LOADXY_TIMEOUT;
}

முழு
xyzModem_stream_open (connection_info_t * info, முழு *err)
{
  முழு stat = 0;
  முழு retries = xyzModem_MAX_RETRIES;
  முழு crc_retries = xyzModem_MAX_RETRIES_WITH_CRC;

/*    ZM_DEBUG(zm_out = zm_out_start); */
#ifdef xyzModem_zmodem
  எனில் (info->mode == xyzModem_zmodem)
    {
      *err = xyzModem_noZmodem;
      திரும்பு -1;
    }
#endif

/* TODO: CHECK ! */
  முழு dummy = 0;
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

  எனில் (xyz.mode == xyzModem_xmodem)
    {
      /* X-modem doesn't have an information header - exit here */
      xyz.first_xmodem_packet = true;
      xyz.next_blk = 1;
      திரும்பு 0;
    }

  போது (!(xyz.timeout && get_timer(xyz.initial_time) > xyz.timeout))
    {
      எனில் (--retries <= 0)
        {
          retries = xyzModem_MAX_RETRIES;
          crc_retries = xyzModem_MAX_RETRIES_WITH_CRC;
          xyz.crc_mode = true;
        }
      stat = xyzModem_get_hdr ();
      எனில் (stat == 0)
	{
	  /* Y-modem file information header */
	  எனில் (xyz.blk == 0)
	    {
	      /* skip filename */
	      போது (*xyz.bufp++);
	      /* get the length */
	      parse_இலக்கம் ((எழுத்து *) xyz.bufp, &xyz.file_length, வெற்று, " ");
	      /* The rest of the file name data block quietly discarded */
	      xyz.tx_ack = true;
	    }
	  xyz.next_blk = 1;
	  xyz.len = 0;
	  திரும்பு 0;
	}
      இல்லையெனில் எனில் (stat == xyzModem_timeout)
	{
	  எனில் (--crc_retries <= 0)
	    xyz.crc_mode = false;
	  CYGACC_CALL_IF_DELAY_US (5 * 100000);	/* Extra delay for startup */
	  CYGACC_COMM_IF_PUTC (*xyz.__chan, (xyz.crc_mode ? 'C' : NAK));
	  xyz.total_retries++;
	  ZM_DEBUG (zm_dprintf ("NAK (%d)\n", __LINE__));
	}
      எனில் (stat == xyzModem_cancel)
	{
	  நிறுத்து;
	}
    }
  *err = stat;
  ZM_DEBUG (zm_flush ());
  திரும்பு -1;
}

முழு
xyzModem_stream_read (எழுத்து *buf, முழு size, முழு *err)
{
  முழு stat, total, len;
  முழு retries;

  total = 0;
  stat = xyzModem_cancel;
  /* Try and get 'size' bytes into the buffer */
  போது (!xyz.at_eof && xyz.len >= 0 && (size > 0))
    {
      எனில் (xyz.len == 0 && xyz.pad_deliver == 0)
	{
	  retries = xyzModem_MAX_RETRIES;
	  போது (retries-- > 0)
	    {
	      எனில் (xyz.first_xmodem_packet && xyz.timeout &&
		  get_timer(xyz.initial_time) > xyz.timeout)
		{
		  *err = xyzModem_timeout;
		  xyz.len = -1;
		  திரும்பு total;
		}

	      stat = xyzModem_get_hdr ();
	      எனில் (stat == 0)
		{
		  எனில் (xyz.mode == xyzModem_xmodem && xyz.first_xmodem_packet)
		    xyz.first_xmodem_packet = false;
		  எனில் (xyz.blk == xyz.next_blk)
		    {
		      xyz.tx_ack = true;
		      ZM_DEBUG (zm_dprintf
				("ACK block %d (%d)\n", xyz.blk, __LINE__));
		      xyz.next_blk = (xyz.next_blk + 1) & 0xFF;

		      எனில் (xyz.mode == xyzModem_xmodem || xyz.file_length == 0)
			{
			  /* Without a file length, EOF padding can't be
			   * distinguished from block data until the next
			   * frame arrives and we can check for EOT/EOF.
			   */
			  xyz.pad_deliver = xyz.pad_pending;
			  xyz.pad_pending = 0;
			  எனில் ((xyz.bufp[xyz.len - 1] == EOF) &&
			      (xyz.bufp[xyz.len - 2] == EOF) &&
			      (xyz.bufp[xyz.len - 3] == EOF))
			    {
			      போது (xyz.len
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
		      எனில் (xyz.mode != xyzModem_xmodem && 0 != xyz.file_length)
			{
			  xyz.read_length += xyz.len;
			  எனில் (xyz.read_length > xyz.file_length)
			    {
			      xyz.len -= (xyz.read_length - xyz.file_length);
			    }
			}
		      நிறுத்து;
		    }
		  இல்லையெனில் எனில் (xyz.blk == ((xyz.next_blk - 1) & 0xFF))
		    {
		      /* Just re-ACK this so sender will get on with it */
		      CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		      தொடர்;	/* Need new header */
		    }
		  இல்லையெனில்
		    {
		      stat = xyzModem_sequence;
		    }
		}
	      எனில் (stat == xyzModem_cancel)
		{
		  நிறுத்து;
		}
	      எனில் (stat == xyzModem_eof)
		{
		  CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		  ZM_DEBUG (zm_dprintf ("ACK (%d)\n", __LINE__));
		  எனில் (xyz.mode == xyzModem_ymodem)
		    {
		      CYGACC_COMM_IF_PUTC (*xyz.__chan,
					   (xyz.crc_mode ? 'C' : NAK));
		      xyz.total_retries++;
		      ZM_DEBUG (zm_dprintf ("Reading Final Header\n"));
		      stat = xyzModem_get_hdr ();
		      CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		      ZM_DEBUG (zm_dprintf ("FINAL ACK (%d)\n", __LINE__));
		    }
		  இல்லையெனில்
		    stat = 0;

		  xyz.pad_pending = 0;	/* EOT: withheld ^Z run was padding */
		  xyz.at_eof = true;
		  நிறுத்து;
		}
	      CYGACC_COMM_IF_PUTC (*xyz.__chan, (xyz.crc_mode ? 'C' : NAK));
	      xyz.total_retries++;
	      ZM_DEBUG (zm_dprintf ("NAK (%d)\n", __LINE__));
	    }
	  எனில் (stat < 0 && (!xyz.first_xmodem_packet || stat != xyzModem_timeout))
	    {
	      *err = stat;
	      xyz.len = -1;
	      திரும்பு total;
	    }
	}
      /* A ^Z run withheld from the previous block turned out to be data */
      எனில் (xyz.pad_deliver > 0)
	{
	  len = xyz.pad_deliver;
	  எனில் (size < len)
	    len = size;
	  memset (buf, EOF, len);
	  size -= len;
	  buf += len;
	  total += len;
	  xyz.pad_deliver -= len;
	}
      /* Don't "read" data from the EOF protocol package */
      எனில் (!xyz.at_eof && xyz.len > 0)
	{
	  len = xyz.len;
	  எனில் (size < len)
	    len = size;
	  memcpy (buf, xyz.bufp, len);
	  size -= len;
	  buf += len;
	  total += len;
	  xyz.len -= len;
	  xyz.bufp += len;
	}
    }
  திரும்பு total;
}

வெறுமை
xyzModem_stream_close (முழு *err)
{
  ZM_DEBUG (zm_dprintf
    ("xyzModem - %s mode, %d(SOH)/%d(STX)/%d(CAN) packets, %d retries\n",
     xyz.crc_mode ? "CRC" : "Cksum", xyz.total_SOH, xyz.total_STX,
     xyz.total_CAN, xyz.total_retries));
  ZM_DEBUG (zm_flush ());
}

/* Need to be able to clean out the input buffer, so have to take the */
/* getc */
வெறுமை
xyzModem_stream_terminate (bool abort, முழு (*getc) (வெறுமை))
{
  முழு c;

  எனில் (abort)
    {
      ZM_DEBUG (zm_dprintf ("!!!! TRANSFER ABORT !!!!\n"));
      தேர்வு (xyz.mode)
	{
	நிலை xyzModem_xmodem:
	நிலை xyzModem_ymodem:
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
	  நிறுத்து;
#ifdef xyzModem_zmodem
	நிலை xyzModem_zmodem:
	  /* Might support it some day I suppose. */
#endif
	  நிறுத்து;
	}
    }
  இல்லையெனில்
    {
      ZM_DEBUG (zm_dprintf ("Engaging cleanup mode...\n"));
      /*
       * Consume any trailing crap left in the inbuffer from
       * previous received blocks. Since very few files are an exact multiple
       * of the transfer block size, there will almost always be some gunk here.
       * If we don't eat it now, RedBoot will think the user typed it.
       */
      ZM_DEBUG (zm_dprintf ("Trailing gunk:\n"));
      போது ((c = (*getc) ()) > -1)
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

எழுத்து *
xyzModem_error (முழு err)
{
  தேர்வு (err)
    {
    நிலை xyzModem_access:
      திரும்பு "Can't access file";
      நிறுத்து;
    நிலை xyzModem_noZmodem:
      திரும்பு "Sorry, zModem not available yet";
      நிறுத்து;
    நிலை xyzModem_timeout:
      திரும்பு "Timed out";
      நிறுத்து;
    நிலை xyzModem_eof:
      திரும்பு "End of file";
      நிறுத்து;
    நிலை xyzModem_cancel:
      திரும்பு "Cancelled";
      நிறுத்து;
    நிலை xyzModem_frame:
      திரும்பு "Invalid framing";
      நிறுத்து;
    நிலை xyzModem_cksum:
      திரும்பு "CRC/checksum error";
      நிறுத்து;
    நிலை xyzModem_sequence:
      திரும்பு "Block sequence error";
      நிறுத்து;
    இயல்பு:
      திரும்பு "Unknown error";
      நிறுத்து;
    }
}

/*
 * RedBoot interface
 */
