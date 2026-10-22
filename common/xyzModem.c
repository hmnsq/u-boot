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
#ଅନ୍ତର୍ଭାଵ <xyzModem.h>
#ଅନ୍ତର୍ଭାଵ <stdarg.h>
#ଅନ୍ତର୍ଭାଵ <time.h>
#ଅନ୍ତର୍ଭାଵ <u-boot/crc.h>
#ଅନ୍ତର୍ଭାଵ <watchdog.h>
#ଅନ୍ତର୍ଭାଵ <env.h>
#ଅନ୍ତର୍ଭାଵ <vsprintf.h>

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
ସ୍ଥିର ସଂରଚନା
{
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା *__chan;
  unsigned ଅକ୍ଷର pkt[1024], *bufp;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା pad_pending;	/* trailing ^Z run withheld from the previous block */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା pad_deliver;	/* ^Z bytes to emit before the next data */
  unsigned ଅକ୍ଷର blk, cblk, crc1, crc2;
  unsigned ଅକ୍ଷର next_blk;	/* Expected block */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା len, mode, total_retries;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା total_SOH, total_STX, total_CAN;
  bool crc_mode, at_eof, tx_ack;
  bool first_xmodem_packet;
  ulong initial_time, timeout;
  unsigned long file_length, read_length;
} xyz;

#define xyzModem_CHAR_TIMEOUT            2000	/* 2 seconds */
#define xyzModem_MAX_RETRIES             20
#define xyzModem_MAX_RETRIES_WITH_CRC    10
#define xyz_Modem_CAN_ଗଣନା                3	/* Wait for 3 CAN before quitting */

ପ୍ରକାରବ୍ୟାଖ୍ୟା ପୂର୍ଣ୍ଣସଂଖ୍ୟା cyg_int32;
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
CYGACC_COMM_IF_GETC_TIMEOUT (ଅକ୍ଷର chan, ଅକ୍ଷର *c)
{

  ulong now = get_timer(0);
  schedule();
  ଯେତେବେଳେ (!tstc ())
    {
      ଯଦି (get_timer(now) > xyzModem_CHAR_TIMEOUT)
        ଭାଙ୍ଗ;
    }
  ଯଦି (tstc ())
    {
      *c = getchar();
      ଫେରନ୍ତୁ 1;
    }
  ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର ଶୂନ୍ୟ
CYGACC_COMM_IF_PUTC (ଅକ୍ଷର x, ଅକ୍ଷର y)
{
  putc (y);
}

/* Validate a hex character */
__inline__ ସ୍ଥିର bool
_is_hex (ଅକ୍ଷର c)
{
  ଫେରନ୍ତୁ (((c >= '0') && (c <= '9')) ||
	  ((c >= 'A') && (c <= 'F')) || ((c >= 'a') && (c <= 'f')));
}

/* Convert a single hex nibble */
__inline__ ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
_from_hex (ଅକ୍ଷର c)
{
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret = 0;

  ଯଦି ((c >= '0') && (c <= '9'))
    {
      ret = (c - '0');
    }
  ଅନ୍ୟଥା ଯଦି ((c >= 'a') && (c <= 'f'))
    {
      ret = (c - 'a' + 0x0a);
    }
  ଅନ୍ୟଥା ଯଦି ((c >= 'A') && (c <= 'F'))
    {
      ret = (c - 'A' + 0x0A);
    }
  ଫେରନ୍ତୁ ret;
}

/* Convert a character to lower case */
__inline__ ସ୍ଥିର ଅକ୍ଷର
_tolower (ଅକ୍ଷର c)
{
  ଯଦି ((c >= 'A') && (c <= 'Z'))
    {
      c = (c - 'A') + 'a';
    }
  ଫେରନ୍ତୁ c;
}

/* Parse (scan) a number */
ସ୍ଥିର bool
parse_ଅଙ୍କ (ଅକ୍ଷର *s, unsigned long *val, ଅକ୍ଷର **es, ଅକ୍ଷର *delim)
{
  bool first = true;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା radix = 10;
  ଅକ୍ଷର c;
  unsigned long result = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା digit;

  ଯେତେବେଳେ (*s == ' ')
    s++;
  ଯେତେବେଳେ (*s)
    {
      ଯଦି (first && (s[0] == '0') && (_tolower (s[1]) == 'x'))
	{
	  radix = 16;
	  s += 2;
	}
      first = false;
      c = *s++;
      ଯଦି (_is_hex (c) && ((digit = _from_hex (c)) < radix))
	{
	  /* Valid digit */
	  result = (result * radix) + digit;
	}
      ଅନ୍ୟଥା
	{
	  ଯଦି (delim != (ଅକ୍ଷର *) 0)
	    {
	      /* See if this character is one of the delimiters */
	      ଅକ୍ଷର *dp = delim;
	      ଯେତେବେଳେ (*dp && (c != *dp))
		dp++;
	      ଯଦି (*dp)
		ଭାଙ୍ଗ;		/* Found a good delimiter */
	    }
	  ଫେରନ୍ତୁ false;		/* Malformatted number */
	}
    }
  *val = result;
  ଯଦି (es != (ଅକ୍ଷର **) 0)
    {
      *es = s;
    }
  ଫେରନ୍ତୁ true;
}

#ଯଦି defined(DEBUG) && !CONFIG_IS_ENABLED(USE_TINY_PRINTF)
/*
 * Note: this debug setup works by storing the strings in a fixed buffer
 */
ସ୍ଥିର ଅକ୍ଷର zm_debug_buf[8192];
ସ୍ଥିର ଅକ୍ଷର *zm_out = zm_debug_buf;
ସ୍ଥିର ଅକ୍ଷର *zm_out_start = zm_debug_buf;

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
zm_dprintf(ଅକ୍ଷର *fmt, ...)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା len;
	va_list args;

	va_start(args, fmt);
	len = diag_vsprintf(zm_out, fmt, args);
	va_end(args);
	zm_out += len;
	ଫେରନ୍ତୁ len;
}

ସ୍ଥିର ଶୂନ୍ୟ
zm_flush (ଶୂନ୍ୟ)
{
  zm_out = zm_out_start;
}

ସ୍ଥିର ଶୂନ୍ୟ
zm_dump_buf (ଶୂନ୍ୟ *buf, ପୂର୍ଣ୍ଣସଂଖ୍ୟା len)
{

}

ସ୍ଥିର unsigned ଅକ୍ଷର zm_buf[2048];
ସ୍ଥିର unsigned ଅକ୍ଷର *zm_bp;

ସ୍ଥିର ଶୂନ୍ୟ
zm_new (ଶୂନ୍ୟ)
{
  zm_bp = zm_buf;
}

ସ୍ଥିର ଶୂନ୍ୟ
zm_save (unsigned ଅକ୍ଷର c)
{
  *zm_bp++ = c;
}

ସ୍ଥିର ଶୂନ୍ୟ
zm_dump (ପୂର୍ଣ୍ଣସଂଖ୍ୟା ଧାଡ଼ି)
{
  zm_dprintf ("Packet at line: %d\n", ଧାଡ଼ି);
  zm_dump_buf (zm_buf, zm_bp - zm_buf);
}

#define ZM_DEBUG(x) x
#ଅନ୍ୟଥା
#define ZM_DEBUG(x)
#endif

/* Wait for the line to go idle */
ସ୍ଥିର ଶୂନ୍ୟ
xyzModem_flush (ଶୂନ୍ୟ)
{
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା res;
  ଅକ୍ଷର c;
  ଯେତେବେଳେ (true)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
      ଯଦି (!res)
	ଫେରନ୍ତୁ;
    }
}

ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା
xyzModem_get_hdr (ଶୂନ୍ୟ)
{
  ଅକ୍ଷର c;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା res;
  bool hdr_found = false;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, can_total, hdr_chars;
  unsigned short cksum;

  ZM_DEBUG (zm_new ());
  /* Find the start of a header */
  can_total = 0;
  hdr_chars = 0;

  ଯଦି (xyz.tx_ack)
    {
      CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
      xyz.tx_ack = false;
    }
  ଯେତେବେଳେ (!hdr_found)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
      ZM_DEBUG (zm_save (c));
      ଯଦି (res)
	{
	  hdr_chars++;
	  ବାଛ (c)
	    {
	    ସ୍ଥିତି SOH:
	      xyz.total_SOH++;
	      fallthrough;
	    ସ୍ଥିତି STX:
	      ଯଦି (c == STX)
		xyz.total_STX++;
	      hdr_found = true;
	      ଭାଙ୍ଗ;
	    ସ୍ଥିତି CAN:
	    ସ୍ଥିତି ETX:
	      xyz.total_CAN++;
	      ZM_DEBUG (zm_dump (__LINE__));
	      ଯଦି (++can_total == xyz_Modem_CAN_ଗଣନା)
		{
		  ଫେରନ୍ତୁ xyzModem_cancel;
		}
	      ଅନ୍ୟଥା
		{
		  /* Wait for multiple CAN to avoid early quits */
		  ଭାଙ୍ଗ;
		}
	    ସ୍ଥିତି EOT:
	      /* EOT only supported if no noise */
	      ଯଦି (hdr_chars == 1)
		{
		  CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		  ZM_DEBUG (zm_dprintf ("ACK on EOT #%d\n", __LINE__));
		  ZM_DEBUG (zm_dump (__LINE__));
		  ଫେରନ୍ତୁ xyzModem_eof;
		}
	    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
	      /* Ignore, waiting for start of header */
	      ;
	    }
	}
      ଅନ୍ୟଥା
	{
	  /* Data stream timed out */
	  xyzModem_flush ();	/* Toss any current input */
	  ZM_DEBUG (zm_dump (__LINE__));
	  CYGACC_CALL_IF_DELAY_US ((cyg_int32) 250000);
	  ଫେରନ୍ତୁ xyzModem_timeout;
	}
    }

  /* Header found, now read the data */
  res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (ଅକ୍ଷର *) &xyz.blk);
  ZM_DEBUG (zm_save (xyz.blk));
  ଯଦି (!res)
    {
      ZM_DEBUG (zm_dump (__LINE__));
      ଫେରନ୍ତୁ xyzModem_timeout;
    }
  res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (ଅକ୍ଷର *) &xyz.cblk);
  ZM_DEBUG (zm_save (xyz.cblk));
  ଯଦି (!res)
    {
      ZM_DEBUG (zm_dump (__LINE__));
      ଫେରନ୍ତୁ xyzModem_timeout;
    }
  xyz.len = (c == SOH) ? 128 : 1024;
  xyz.bufp = xyz.pkt;
  ପାଇଁ (i = 0; i < xyz.len; i++)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
      ZM_DEBUG (zm_save (c));
      ଯଦି (res)
	{
	  xyz.pkt[i] = c;
	}
      ଅନ୍ୟଥା
	{
	  ZM_DEBUG (zm_dump (__LINE__));
	  ଫେରନ୍ତୁ xyzModem_timeout;
	}
    }
  res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (ଅକ୍ଷର *) &xyz.crc1);
  ZM_DEBUG (zm_save (xyz.crc1));
  ଯଦି (!res)
    {
      ZM_DEBUG (zm_dump (__LINE__));
      ଫେରନ୍ତୁ xyzModem_timeout;
    }
  ଯଦି (xyz.crc_mode)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (ଅକ୍ଷର *) &xyz.crc2);
      ZM_DEBUG (zm_save (xyz.crc2));
      ଯଦି (!res)
	{
	  ZM_DEBUG (zm_dump (__LINE__));
	  ଫେରନ୍ତୁ xyzModem_timeout;
	}
    }
  ZM_DEBUG (zm_dump (__LINE__));
  /* Validate the message */
  ଯଦି ((xyz.blk ^ xyz.cblk) != (unsigned ଅକ୍ଷର) 0xFF)
    {
      ZM_DEBUG (zm_dprintf
		("Framing error - blk: %x/%x/%x\n", xyz.blk, xyz.cblk,
		 (xyz.blk ^ xyz.cblk)));
      ZM_DEBUG (zm_dump_buf (xyz.pkt, xyz.len));
      xyzModem_flush ();
      ଫେରନ୍ତୁ xyzModem_frame;
    }
  /* Verify checksum/CRC */
  ଯଦି (xyz.crc_mode)
    {
      cksum = crc16_ccitt(0, xyz.pkt, xyz.len);
      ଯଦି (cksum != ((xyz.crc1 << 8) | xyz.crc2))
	{
	  ZM_DEBUG (zm_dprintf ("CRC error - recvd: %02x%02x, computed: %x\n",
				xyz.crc1, xyz.crc2, cksum & 0xFFFF));
	  ଫେରନ୍ତୁ xyzModem_cksum;
	}
    }
  ଅନ୍ୟଥା
    {
      cksum = 0;
      ପାଇଁ (i = 0; i < xyz.len; i++)
	{
	  cksum += xyz.pkt[i];
	}
      ଯଦି (xyz.crc1 != (cksum & 0xFF))
	{
	  ZM_DEBUG (zm_dprintf
		    ("Checksum error - recvd: %x, computed: %x\n", xyz.crc1,
		     cksum & 0xFF));
	  ଫେରନ୍ତୁ xyzModem_cksum;
	}
    }
  /* If we get here, the message passes [structural] muster */
  ଫେରନ୍ତୁ 0;
}

ସ୍ଥିର
ulong
xyzModem_get_initial_timeout (ଶୂନ୍ୟ)
{
  /* timeout is in seconds, non-positive timeout value is infinity */
#ଯଦି CONFIG_IS_ENABLED(ENV_SUPPORT)
  ଅଚଳ ଅକ୍ଷର *timeout_str = env_get("loadxy_timeout");
  ଯଦି (timeout_str)
    ଫେରନ୍ତୁ 1000 * simple_strtol(timeout_str, ଖାଲି, 10);
#endif
  ଫେରନ୍ତୁ 1000 * CONFIG_CMD_LOADXY_TIMEOUT;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା
xyzModem_stream_open (connection_info_t * info, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *err)
{
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା stat = 0;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା retries = xyzModem_MAX_RETRIES;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା crc_retries = xyzModem_MAX_RETRIES_WITH_CRC;

/*    ZM_DEBUG(zm_out = zm_out_start); */
#ifdef xyzModem_zmodem
  ଯଦି (info->mode == xyzModem_zmodem)
    {
      *err = xyzModem_noZmodem;
      ଫେରନ୍ତୁ -1;
    }
#endif

/* TODO: CHECK ! */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା dummy = 0;
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

  ଯଦି (xyz.mode == xyzModem_xmodem)
    {
      /* X-modem doesn't have an information header - exit here */
      xyz.first_xmodem_packet = true;
      xyz.next_blk = 1;
      ଫେରନ୍ତୁ 0;
    }

  ଯେତେବେଳେ (!(xyz.timeout && get_timer(xyz.initial_time) > xyz.timeout))
    {
      ଯଦି (--retries <= 0)
        {
          retries = xyzModem_MAX_RETRIES;
          crc_retries = xyzModem_MAX_RETRIES_WITH_CRC;
          xyz.crc_mode = true;
        }
      stat = xyzModem_get_hdr ();
      ଯଦି (stat == 0)
	{
	  /* Y-modem file information header */
	  ଯଦି (xyz.blk == 0)
	    {
	      /* skip filename */
	      ଯେତେବେଳେ (*xyz.bufp++);
	      /* get the length */
	      parse_ଅଙ୍କ ((ଅକ୍ଷର *) xyz.bufp, &xyz.file_length, ଖାଲି, " ");
	      /* The rest of the file name data block quietly discarded */
	      xyz.tx_ack = true;
	    }
	  xyz.next_blk = 1;
	  xyz.len = 0;
	  ଫେରନ୍ତୁ 0;
	}
      ଅନ୍ୟଥା ଯଦି (stat == xyzModem_timeout)
	{
	  ଯଦି (--crc_retries <= 0)
	    xyz.crc_mode = false;
	  CYGACC_CALL_IF_DELAY_US (5 * 100000);	/* Extra delay for startup */
	  CYGACC_COMM_IF_PUTC (*xyz.__chan, (xyz.crc_mode ? 'C' : NAK));
	  xyz.total_retries++;
	  ZM_DEBUG (zm_dprintf ("NAK (%d)\n", __LINE__));
	}
      ଯଦି (stat == xyzModem_cancel)
	{
	  ଭାଙ୍ଗ;
	}
    }
  *err = stat;
  ZM_DEBUG (zm_flush ());
  ଫେରନ୍ତୁ -1;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା
xyzModem_stream_read (ଅକ୍ଷର *buf, ପୂର୍ଣ୍ଣସଂଖ୍ୟା size, ପୂର୍ଣ୍ଣସଂଖ୍ୟା *err)
{
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା stat, total, len;
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା retries;

  total = 0;
  stat = xyzModem_cancel;
  /* Try and get 'size' bytes into the buffer */
  ଯେତେବେଳେ (!xyz.at_eof && xyz.len >= 0 && (size > 0))
    {
      ଯଦି (xyz.len == 0 && xyz.pad_deliver == 0)
	{
	  retries = xyzModem_MAX_RETRIES;
	  ଯେତେବେଳେ (retries-- > 0)
	    {
	      ଯଦି (xyz.first_xmodem_packet && xyz.timeout &&
		  get_timer(xyz.initial_time) > xyz.timeout)
		{
		  *err = xyzModem_timeout;
		  xyz.len = -1;
		  ଫେରନ୍ତୁ total;
		}

	      stat = xyzModem_get_hdr ();
	      ଯଦି (stat == 0)
		{
		  ଯଦି (xyz.mode == xyzModem_xmodem && xyz.first_xmodem_packet)
		    xyz.first_xmodem_packet = false;
		  ଯଦି (xyz.blk == xyz.next_blk)
		    {
		      xyz.tx_ack = true;
		      ZM_DEBUG (zm_dprintf
				("ACK block %d (%d)\n", xyz.blk, __LINE__));
		      xyz.next_blk = (xyz.next_blk + 1) & 0xFF;

		      ଯଦି (xyz.mode == xyzModem_xmodem || xyz.file_length == 0)
			{
			  /* Without a file length, EOF padding can't be
			   * distinguished from block data until the next
			   * frame arrives and we can check for EOT/EOF.
			   */
			  xyz.pad_deliver = xyz.pad_pending;
			  xyz.pad_pending = 0;
			  ଯଦି ((xyz.bufp[xyz.len - 1] == EOF) &&
			      (xyz.bufp[xyz.len - 2] == EOF) &&
			      (xyz.bufp[xyz.len - 3] == EOF))
			    {
			      ଯେତେବେଳେ (xyz.len
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
		      ଯଦି (xyz.mode != xyzModem_xmodem && 0 != xyz.file_length)
			{
			  xyz.read_length += xyz.len;
			  ଯଦି (xyz.read_length > xyz.file_length)
			    {
			      xyz.len -= (xyz.read_length - xyz.file_length);
			    }
			}
		      ଭାଙ୍ଗ;
		    }
		  ଅନ୍ୟଥା ଯଦି (xyz.blk == ((xyz.next_blk - 1) & 0xFF))
		    {
		      /* Just re-ACK this so sender will get on with it */
		      CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		      ଜାରି;	/* Need new header */
		    }
		  ଅନ୍ୟଥା
		    {
		      stat = xyzModem_sequence;
		    }
		}
	      ଯଦି (stat == xyzModem_cancel)
		{
		  ଭାଙ୍ଗ;
		}
	      ଯଦି (stat == xyzModem_eof)
		{
		  CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		  ZM_DEBUG (zm_dprintf ("ACK (%d)\n", __LINE__));
		  ଯଦି (xyz.mode == xyzModem_ymodem)
		    {
		      CYGACC_COMM_IF_PUTC (*xyz.__chan,
					   (xyz.crc_mode ? 'C' : NAK));
		      xyz.total_retries++;
		      ZM_DEBUG (zm_dprintf ("Reading Final Header\n"));
		      stat = xyzModem_get_hdr ();
		      CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		      ZM_DEBUG (zm_dprintf ("FINAL ACK (%d)\n", __LINE__));
		    }
		  ଅନ୍ୟଥା
		    stat = 0;

		  xyz.pad_pending = 0;	/* EOT: withheld ^Z run was padding */
		  xyz.at_eof = true;
		  ଭାଙ୍ଗ;
		}
	      CYGACC_COMM_IF_PUTC (*xyz.__chan, (xyz.crc_mode ? 'C' : NAK));
	      xyz.total_retries++;
	      ZM_DEBUG (zm_dprintf ("NAK (%d)\n", __LINE__));
	    }
	  ଯଦି (stat < 0 && (!xyz.first_xmodem_packet || stat != xyzModem_timeout))
	    {
	      *err = stat;
	      xyz.len = -1;
	      ଫେରନ୍ତୁ total;
	    }
	}
      /* A ^Z run withheld from the previous block turned out to be data */
      ଯଦି (xyz.pad_deliver > 0)
	{
	  len = xyz.pad_deliver;
	  ଯଦି (size < len)
	    len = size;
	  memset (buf, EOF, len);
	  size -= len;
	  buf += len;
	  total += len;
	  xyz.pad_deliver -= len;
	}
      /* Don't "read" data from the EOF protocol package */
      ଯଦି (!xyz.at_eof && xyz.len > 0)
	{
	  len = xyz.len;
	  ଯଦି (size < len)
	    len = size;
	  memcpy (buf, xyz.bufp, len);
	  size -= len;
	  buf += len;
	  total += len;
	  xyz.len -= len;
	  xyz.bufp += len;
	}
    }
  ଫେରନ୍ତୁ total;
}

ଶୂନ୍ୟ
xyzModem_stream_close (ପୂର୍ଣ୍ଣସଂଖ୍ୟା *err)
{
  ZM_DEBUG (zm_dprintf
    ("xyzModem - %s mode, %d(SOH)/%d(STX)/%d(CAN) packets, %d retries\n",
     xyz.crc_mode ? "CRC" : "Cksum", xyz.total_SOH, xyz.total_STX,
     xyz.total_CAN, xyz.total_retries));
  ZM_DEBUG (zm_flush ());
}

/* Need to be able to clean out the input buffer, so have to take the */
/* getc */
ଶୂନ୍ୟ
xyzModem_stream_terminate (bool abort, ପୂର୍ଣ୍ଣସଂଖ୍ୟା (*getc) (ଶୂନ୍ୟ))
{
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା c;

  ଯଦି (abort)
    {
      ZM_DEBUG (zm_dprintf ("!!!! TRANSFER ABORT !!!!\n"));
      ବାଛ (xyz.mode)
	{
	ସ୍ଥିତି xyzModem_xmodem:
	ସ୍ଥିତି xyzModem_ymodem:
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
	  ଭାଙ୍ଗ;
#ifdef xyzModem_zmodem
	ସ୍ଥିତି xyzModem_zmodem:
	  /* Might support it some day I suppose. */
#endif
	  ଭାଙ୍ଗ;
	}
    }
  ଅନ୍ୟଥା
    {
      ZM_DEBUG (zm_dprintf ("Engaging cleanup mode...\n"));
      /*
       * Consume any trailing crap left in the inbuffer from
       * previous received blocks. Since very few files are an exact multiple
       * of the transfer block size, there will almost always be some gunk here.
       * If we don't eat it now, RedBoot will think the user typed it.
       */
      ZM_DEBUG (zm_dprintf ("Trailing gunk:\n"));
      ଯେତେବେଳେ ((c = (*getc) ()) > -1)
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

ଅକ୍ଷର *
xyzModem_error (ପୂର୍ଣ୍ଣସଂଖ୍ୟା err)
{
  ବାଛ (err)
    {
    ସ୍ଥିତି xyzModem_access:
      ଫେରନ୍ତୁ "Can't access file";
      ଭାଙ୍ଗ;
    ସ୍ଥିତି xyzModem_noZmodem:
      ଫେରନ୍ତୁ "Sorry, zModem not available yet";
      ଭାଙ୍ଗ;
    ସ୍ଥିତି xyzModem_timeout:
      ଫେରନ୍ତୁ "Timed out";
      ଭାଙ୍ଗ;
    ସ୍ଥିତି xyzModem_eof:
      ଫେରନ୍ତୁ "End of file";
      ଭାଙ୍ଗ;
    ସ୍ଥିତି xyzModem_cancel:
      ଫେରନ୍ତୁ "Cancelled";
      ଭାଙ୍ଗ;
    ସ୍ଥିତି xyzModem_frame:
      ଫେରନ୍ତୁ "Invalid framing";
      ଭାଙ୍ଗ;
    ସ୍ଥିତି xyzModem_cksum:
      ଫେରନ୍ତୁ "CRC/checksum error";
      ଭାଙ୍ଗ;
    ସ୍ଥିତି xyzModem_sequence:
      ଫେରନ୍ତୁ "Block sequence error";
      ଭାଙ୍ଗ;
    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      ଫେରନ୍ତୁ "Unknown error";
      ଭାଙ୍ଗ;
    }
}

/*
 * RedBoot interface
 */
