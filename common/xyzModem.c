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
#شامل <xyzModem.h>
#شامل <stdarg.h>
#شامل <time.h>
#شامل <u-boot/crc.h>
#شامل <watchdog.h>
#شامل <env.h>
#شامل <vsprintf.h>

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
ایستا ساختار
{
  صحیح *__chan;
  unsigned نویسه pkt[1024], *bufp;
  صحیح pad_pending;	/* trailing ^Z run withheld from the previous block */
  صحیح pad_deliver;	/* ^Z bytes to emit before the next data */
  unsigned نویسه blk, cblk, crc1, crc2;
  unsigned نویسه next_blk;	/* Expected block */
  صحیح len, mode, total_retries;
  صحیح total_SOH, total_STX, total_CAN;
  bool crc_mode, at_eof, tx_ack;
  bool first_xmodem_packet;
  ulong initial_time, timeout;
  unsigned long file_length, read_length;
} xyz;

#define xyzModem_CHAR_TIMEOUT            2000	/* 2 seconds */
#define xyzModem_MAX_RETRIES             20
#define xyzModem_MAX_RETRIES_WITH_CRC    10
#define xyz_Modem_CAN_شمار                3	/* Wait for 3 CAN before quitting */

تعریفنوع صحیح cyg_int32;
ایستا صحیح
CYGACC_COMM_IF_GETC_TIMEOUT (نویسه chan, نویسه *c)
{

  ulong now = get_timer(0);
  schedule();
  وقتی (!tstc ())
    {
      اگر (get_timer(now) > xyzModem_CHAR_TIMEOUT)
        بشکن;
    }
  اگر (tstc ())
    {
      *c = getchar();
      بازگشت 1;
    }
  بازگشت 0;
}

ایستا تهی
CYGACC_COMM_IF_PUTC (نویسه x, نویسه y)
{
  putc (y);
}

/* Validate a hex character */
__inline__ ایستا bool
_is_hex (نویسه c)
{
  بازگشت (((c >= '0') && (c <= '9')) ||
	  ((c >= 'A') && (c <= 'F')) || ((c >= 'a') && (c <= 'f')));
}

/* Convert a single hex nibble */
__inline__ ایستا صحیح
_from_hex (نویسه c)
{
  صحیح ret = 0;

  اگر ((c >= '0') && (c <= '9'))
    {
      ret = (c - '0');
    }
  وگرنه اگر ((c >= 'a') && (c <= 'f'))
    {
      ret = (c - 'a' + 0x0a);
    }
  وگرنه اگر ((c >= 'A') && (c <= 'F'))
    {
      ret = (c - 'A' + 0x0A);
    }
  بازگشت ret;
}

/* Convert a character to lower case */
__inline__ ایستا نویسه
_tolower (نویسه c)
{
  اگر ((c >= 'A') && (c <= 'Z'))
    {
      c = (c - 'A') + 'a';
    }
  بازگشت c;
}

/* Parse (scan) a number */
ایستا bool
parse_رقم (نویسه *s, unsigned long *val, نویسه **es, نویسه *delim)
{
  bool first = true;
  صحیح radix = 10;
  نویسه c;
  unsigned long result = 0;
  صحیح digit;

  وقتی (*s == ' ')
    s++;
  وقتی (*s)
    {
      اگر (first && (s[0] == '0') && (_tolower (s[1]) == 'x'))
	{
	  radix = 16;
	  s += 2;
	}
      first = false;
      c = *s++;
      اگر (_is_hex (c) && ((digit = _from_hex (c)) < radix))
	{
	  /* Valid digit */
	  result = (result * radix) + digit;
	}
      وگرنه
	{
	  اگر (delim != (نویسه *) 0)
	    {
	      /* See if this character is one of the delimiters */
	      نویسه *dp = delim;
	      وقتی (*dp && (c != *dp))
		dp++;
	      اگر (*dp)
		بشکن;		/* Found a good delimiter */
	    }
	  بازگشت false;		/* Malformatted number */
	}
    }
  *val = result;
  اگر (es != (نویسه **) 0)
    {
      *es = s;
    }
  بازگشت true;
}

#اگر defined(DEBUG) && !CONFIG_IS_ENABLED(USE_TINY_PRINTF)
/*
 * Note: this debug setup works by storing the strings in a fixed buffer
 */
ایستا نویسه zm_debug_buf[8192];
ایستا نویسه *zm_out = zm_debug_buf;
ایستا نویسه *zm_out_start = zm_debug_buf;

ایستا صحیح
zm_dprintf(نویسه *fmt, ...)
{
	صحیح len;
	va_list args;

	va_start(args, fmt);
	len = diag_vsprintf(zm_out, fmt, args);
	va_end(args);
	zm_out += len;
	بازگشت len;
}

ایستا تهی
zm_flush (تهی)
{
  zm_out = zm_out_start;
}

ایستا تهی
zm_dump_buf (تهی *buf, صحیح len)
{

}

ایستا unsigned نویسه zm_buf[2048];
ایستا unsigned نویسه *zm_bp;

ایستا تهی
zm_new (تهی)
{
  zm_bp = zm_buf;
}

ایستا تهی
zm_save (unsigned نویسه c)
{
  *zm_bp++ = c;
}

ایستا تهی
zm_dump (صحیح سطر)
{
  zm_dprintf ("Packet at line: %d\n", سطر);
  zm_dump_buf (zm_buf, zm_bp - zm_buf);
}

#define ZM_DEBUG(x) x
#وگرنه
#define ZM_DEBUG(x)
#endif

/* Wait for the line to go idle */
ایستا تهی
xyzModem_flush (تهی)
{
  صحیح res;
  نویسه c;
  وقتی (true)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
      اگر (!res)
	بازگشت;
    }
}

ایستا صحیح
xyzModem_get_hdr (تهی)
{
  نویسه c;
  صحیح res;
  bool hdr_found = false;
  صحیح i, can_total, hdr_chars;
  unsigned short cksum;

  ZM_DEBUG (zm_new ());
  /* Find the start of a header */
  can_total = 0;
  hdr_chars = 0;

  اگر (xyz.tx_ack)
    {
      CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
      xyz.tx_ack = false;
    }
  وقتی (!hdr_found)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
      ZM_DEBUG (zm_save (c));
      اگر (res)
	{
	  hdr_chars++;
	  انتخاب (c)
	    {
	    حالت SOH:
	      xyz.total_SOH++;
	      fallthrough;
	    حالت STX:
	      اگر (c == STX)
		xyz.total_STX++;
	      hdr_found = true;
	      بشکن;
	    حالت CAN:
	    حالت ETX:
	      xyz.total_CAN++;
	      ZM_DEBUG (zm_dump (__LINE__));
	      اگر (++can_total == xyz_Modem_CAN_شمار)
		{
		  بازگشت xyzModem_cancel;
		}
	      وگرنه
		{
		  /* Wait for multiple CAN to avoid early quits */
		  بشکن;
		}
	    حالت EOT:
	      /* EOT only supported if no noise */
	      اگر (hdr_chars == 1)
		{
		  CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		  ZM_DEBUG (zm_dprintf ("ACK on EOT #%d\n", __LINE__));
		  ZM_DEBUG (zm_dump (__LINE__));
		  بازگشت xyzModem_eof;
		}
	    پیشفرض:
	      /* Ignore, waiting for start of header */
	      ;
	    }
	}
      وگرنه
	{
	  /* Data stream timed out */
	  xyzModem_flush ();	/* Toss any current input */
	  ZM_DEBUG (zm_dump (__LINE__));
	  CYGACC_CALL_IF_DELAY_US ((cyg_int32) 250000);
	  بازگشت xyzModem_timeout;
	}
    }

  /* Header found, now read the data */
  res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (نویسه *) &xyz.blk);
  ZM_DEBUG (zm_save (xyz.blk));
  اگر (!res)
    {
      ZM_DEBUG (zm_dump (__LINE__));
      بازگشت xyzModem_timeout;
    }
  res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (نویسه *) &xyz.cblk);
  ZM_DEBUG (zm_save (xyz.cblk));
  اگر (!res)
    {
      ZM_DEBUG (zm_dump (__LINE__));
      بازگشت xyzModem_timeout;
    }
  xyz.len = (c == SOH) ? 128 : 1024;
  xyz.bufp = xyz.pkt;
  برای (i = 0; i < xyz.len; i++)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
      ZM_DEBUG (zm_save (c));
      اگر (res)
	{
	  xyz.pkt[i] = c;
	}
      وگرنه
	{
	  ZM_DEBUG (zm_dump (__LINE__));
	  بازگشت xyzModem_timeout;
	}
    }
  res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (نویسه *) &xyz.crc1);
  ZM_DEBUG (zm_save (xyz.crc1));
  اگر (!res)
    {
      ZM_DEBUG (zm_dump (__LINE__));
      بازگشت xyzModem_timeout;
    }
  اگر (xyz.crc_mode)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (نویسه *) &xyz.crc2);
      ZM_DEBUG (zm_save (xyz.crc2));
      اگر (!res)
	{
	  ZM_DEBUG (zm_dump (__LINE__));
	  بازگشت xyzModem_timeout;
	}
    }
  ZM_DEBUG (zm_dump (__LINE__));
  /* Validate the message */
  اگر ((xyz.blk ^ xyz.cblk) != (unsigned نویسه) 0xFF)
    {
      ZM_DEBUG (zm_dprintf
		("Framing error - blk: %x/%x/%x\n", xyz.blk, xyz.cblk,
		 (xyz.blk ^ xyz.cblk)));
      ZM_DEBUG (zm_dump_buf (xyz.pkt, xyz.len));
      xyzModem_flush ();
      بازگشت xyzModem_frame;
    }
  /* Verify checksum/CRC */
  اگر (xyz.crc_mode)
    {
      cksum = crc16_ccitt(0, xyz.pkt, xyz.len);
      اگر (cksum != ((xyz.crc1 << 8) | xyz.crc2))
	{
	  ZM_DEBUG (zm_dprintf ("CRC error - recvd: %02x%02x, computed: %x\n",
				xyz.crc1, xyz.crc2, cksum & 0xFFFF));
	  بازگشت xyzModem_cksum;
	}
    }
  وگرنه
    {
      cksum = 0;
      برای (i = 0; i < xyz.len; i++)
	{
	  cksum += xyz.pkt[i];
	}
      اگر (xyz.crc1 != (cksum & 0xFF))
	{
	  ZM_DEBUG (zm_dprintf
		    ("Checksum error - recvd: %x, computed: %x\n", xyz.crc1,
		     cksum & 0xFF));
	  بازگشت xyzModem_cksum;
	}
    }
  /* If we get here, the message passes [structural] muster */
  بازگشت 0;
}

ایستا
ulong
xyzModem_get_initial_timeout (تهی)
{
  /* timeout is in seconds, non-positive timeout value is infinity */
#اگر CONFIG_IS_ENABLED(ENV_SUPPORT)
  ثابت نویسه *timeout_str = env_get("loadxy_timeout");
  اگر (timeout_str)
    بازگشت 1000 * simple_strtol(timeout_str, پوچ, 10);
#endif
  بازگشت 1000 * CONFIG_CMD_LOADXY_TIMEOUT;
}

صحیح
xyzModem_stream_open (connection_info_t * info, صحیح *err)
{
  صحیح stat = 0;
  صحیح retries = xyzModem_MAX_RETRIES;
  صحیح crc_retries = xyzModem_MAX_RETRIES_WITH_CRC;

/*    ZM_DEBUG(zm_out = zm_out_start); */
#ifdef xyzModem_zmodem
  اگر (info->mode == xyzModem_zmodem)
    {
      *err = xyzModem_noZmodem;
      بازگشت -1;
    }
#endif

/* TODO: CHECK ! */
  صحیح dummy = 0;
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

  اگر (xyz.mode == xyzModem_xmodem)
    {
      /* X-modem doesn't have an information header - exit here */
      xyz.first_xmodem_packet = true;
      xyz.next_blk = 1;
      بازگشت 0;
    }

  وقتی (!(xyz.timeout && get_timer(xyz.initial_time) > xyz.timeout))
    {
      اگر (--retries <= 0)
        {
          retries = xyzModem_MAX_RETRIES;
          crc_retries = xyzModem_MAX_RETRIES_WITH_CRC;
          xyz.crc_mode = true;
        }
      stat = xyzModem_get_hdr ();
      اگر (stat == 0)
	{
	  /* Y-modem file information header */
	  اگر (xyz.blk == 0)
	    {
	      /* skip filename */
	      وقتی (*xyz.bufp++);
	      /* get the length */
	      parse_رقم ((نویسه *) xyz.bufp, &xyz.file_length, پوچ, " ");
	      /* The rest of the file name data block quietly discarded */
	      xyz.tx_ack = true;
	    }
	  xyz.next_blk = 1;
	  xyz.len = 0;
	  بازگشت 0;
	}
      وگرنه اگر (stat == xyzModem_timeout)
	{
	  اگر (--crc_retries <= 0)
	    xyz.crc_mode = false;
	  CYGACC_CALL_IF_DELAY_US (5 * 100000);	/* Extra delay for startup */
	  CYGACC_COMM_IF_PUTC (*xyz.__chan, (xyz.crc_mode ? 'C' : NAK));
	  xyz.total_retries++;
	  ZM_DEBUG (zm_dprintf ("NAK (%d)\n", __LINE__));
	}
      اگر (stat == xyzModem_cancel)
	{
	  بشکن;
	}
    }
  *err = stat;
  ZM_DEBUG (zm_flush ());
  بازگشت -1;
}

صحیح
xyzModem_stream_read (نویسه *buf, صحیح size, صحیح *err)
{
  صحیح stat, total, len;
  صحیح retries;

  total = 0;
  stat = xyzModem_cancel;
  /* Try and get 'size' bytes into the buffer */
  وقتی (!xyz.at_eof && xyz.len >= 0 && (size > 0))
    {
      اگر (xyz.len == 0 && xyz.pad_deliver == 0)
	{
	  retries = xyzModem_MAX_RETRIES;
	  وقتی (retries-- > 0)
	    {
	      اگر (xyz.first_xmodem_packet && xyz.timeout &&
		  get_timer(xyz.initial_time) > xyz.timeout)
		{
		  *err = xyzModem_timeout;
		  xyz.len = -1;
		  بازگشت total;
		}

	      stat = xyzModem_get_hdr ();
	      اگر (stat == 0)
		{
		  اگر (xyz.mode == xyzModem_xmodem && xyz.first_xmodem_packet)
		    xyz.first_xmodem_packet = false;
		  اگر (xyz.blk == xyz.next_blk)
		    {
		      xyz.tx_ack = true;
		      ZM_DEBUG (zm_dprintf
				("ACK block %d (%d)\n", xyz.blk, __LINE__));
		      xyz.next_blk = (xyz.next_blk + 1) & 0xFF;

		      اگر (xyz.mode == xyzModem_xmodem || xyz.file_length == 0)
			{
			  /* Without a file length, EOF padding can't be
			   * distinguished from block data until the next
			   * frame arrives and we can check for EOT/EOF.
			   */
			  xyz.pad_deliver = xyz.pad_pending;
			  xyz.pad_pending = 0;
			  اگر ((xyz.bufp[xyz.len - 1] == EOF) &&
			      (xyz.bufp[xyz.len - 2] == EOF) &&
			      (xyz.bufp[xyz.len - 3] == EOF))
			    {
			      وقتی (xyz.len
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
		      اگر (xyz.mode != xyzModem_xmodem && 0 != xyz.file_length)
			{
			  xyz.read_length += xyz.len;
			  اگر (xyz.read_length > xyz.file_length)
			    {
			      xyz.len -= (xyz.read_length - xyz.file_length);
			    }
			}
		      بشکن;
		    }
		  وگرنه اگر (xyz.blk == ((xyz.next_blk - 1) & 0xFF))
		    {
		      /* Just re-ACK this so sender will get on with it */
		      CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		      ادامه;	/* Need new header */
		    }
		  وگرنه
		    {
		      stat = xyzModem_sequence;
		    }
		}
	      اگر (stat == xyzModem_cancel)
		{
		  بشکن;
		}
	      اگر (stat == xyzModem_eof)
		{
		  CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		  ZM_DEBUG (zm_dprintf ("ACK (%d)\n", __LINE__));
		  اگر (xyz.mode == xyzModem_ymodem)
		    {
		      CYGACC_COMM_IF_PUTC (*xyz.__chan,
					   (xyz.crc_mode ? 'C' : NAK));
		      xyz.total_retries++;
		      ZM_DEBUG (zm_dprintf ("Reading Final Header\n"));
		      stat = xyzModem_get_hdr ();
		      CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		      ZM_DEBUG (zm_dprintf ("FINAL ACK (%d)\n", __LINE__));
		    }
		  وگرنه
		    stat = 0;

		  xyz.pad_pending = 0;	/* EOT: withheld ^Z run was padding */
		  xyz.at_eof = true;
		  بشکن;
		}
	      CYGACC_COMM_IF_PUTC (*xyz.__chan, (xyz.crc_mode ? 'C' : NAK));
	      xyz.total_retries++;
	      ZM_DEBUG (zm_dprintf ("NAK (%d)\n", __LINE__));
	    }
	  اگر (stat < 0 && (!xyz.first_xmodem_packet || stat != xyzModem_timeout))
	    {
	      *err = stat;
	      xyz.len = -1;
	      بازگشت total;
	    }
	}
      /* A ^Z run withheld from the previous block turned out to be data */
      اگر (xyz.pad_deliver > 0)
	{
	  len = xyz.pad_deliver;
	  اگر (size < len)
	    len = size;
	  memset (buf, EOF, len);
	  size -= len;
	  buf += len;
	  total += len;
	  xyz.pad_deliver -= len;
	}
      /* Don't "read" data from the EOF protocol package */
      اگر (!xyz.at_eof && xyz.len > 0)
	{
	  len = xyz.len;
	  اگر (size < len)
	    len = size;
	  memcpy (buf, xyz.bufp, len);
	  size -= len;
	  buf += len;
	  total += len;
	  xyz.len -= len;
	  xyz.bufp += len;
	}
    }
  بازگشت total;
}

تهی
xyzModem_stream_close (صحیح *err)
{
  ZM_DEBUG (zm_dprintf
    ("xyzModem - %s mode, %d(SOH)/%d(STX)/%d(CAN) packets, %d retries\n",
     xyz.crc_mode ? "CRC" : "Cksum", xyz.total_SOH, xyz.total_STX,
     xyz.total_CAN, xyz.total_retries));
  ZM_DEBUG (zm_flush ());
}

/* Need to be able to clean out the input buffer, so have to take the */
/* getc */
تهی
xyzModem_stream_terminate (bool abort, صحیح (*getc) (تهی))
{
  صحیح c;

  اگر (abort)
    {
      ZM_DEBUG (zm_dprintf ("!!!! TRANSFER ABORT !!!!\n"));
      انتخاب (xyz.mode)
	{
	حالت xyzModem_xmodem:
	حالت xyzModem_ymodem:
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
	  بشکن;
#ifdef xyzModem_zmodem
	حالت xyzModem_zmodem:
	  /* Might support it some day I suppose. */
#endif
	  بشکن;
	}
    }
  وگرنه
    {
      ZM_DEBUG (zm_dprintf ("Engaging cleanup mode...\n"));
      /*
       * Consume any trailing crap left in the inbuffer from
       * previous received blocks. Since very few files are an exact multiple
       * of the transfer block size, there will almost always be some gunk here.
       * If we don't eat it now, RedBoot will think the user typed it.
       */
      ZM_DEBUG (zm_dprintf ("Trailing gunk:\n"));
      وقتی ((c = (*getc) ()) > -1)
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

نویسه *
xyzModem_error (صحیح err)
{
  انتخاب (err)
    {
    حالت xyzModem_access:
      بازگشت "Can't access file";
      بشکن;
    حالت xyzModem_noZmodem:
      بازگشت "Sorry, zModem not available yet";
      بشکن;
    حالت xyzModem_timeout:
      بازگشت "Timed out";
      بشکن;
    حالت xyzModem_eof:
      بازگشت "End of file";
      بشکن;
    حالت xyzModem_cancel:
      بازگشت "Cancelled";
      بشکن;
    حالت xyzModem_frame:
      بازگشت "Invalid framing";
      بشکن;
    حالت xyzModem_cksum:
      بازگشت "CRC/checksum error";
      بشکن;
    حالت xyzModem_sequence:
      بازگشت "Block sequence error";
      بشکن;
    پیشفرض:
      بازگشت "Unknown error";
      بشکن;
    }
}

/*
 * RedBoot interface
 */
