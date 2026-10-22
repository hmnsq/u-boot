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
#ਅਨ੍ਤਰ੍ਭਾਵ <xyzModem.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <stdarg.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <time.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <u-boot/crc.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <watchdog.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <env.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <vsprintf.h>

/* Assumption - run xyzModem protocol over the console port */

/* Values magic to the protocol */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SOH 0x01
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ STX 0x02
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ETX 0x03		/* ^C for interrupt */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EOT 0x04
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ACK 0x06
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BSP 0x08
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NAK 0x15
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ CAN 0x18
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ EOF 0x1A		/* ^Z for DOS officionados */

/* Data & state local to the protocol */
ਸਥਿਰ ਬਣਤਰ
{
  ਪੂਰਨ *__chan;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ pkt[1024], *bufp;
  ਪੂਰਨ pad_pending;	/* trailing ^Z run withheld from the previous block */
  ਪੂਰਨ pad_deliver;	/* ^Z bytes to emit before the next data */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ blk, cblk, crc1, crc2;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ ਅਗਲਾ_blk;	/* Expected block */
  ਪੂਰਨ len, ਬਹੁਲਕ, ਕੁੱਲ_retries;
  ਪੂਰਨ ਕੁੱਲ_SOH, ਕੁੱਲ_STX, ਕੁੱਲ_CAN;
  bool ਚੱਕਰੀ_ਜਾਂਚ_ਬਹੁਲਕ, at_eof, tx_ack;
  bool ਪਹਿਲਾ_xmodem_packet;
  ulong initial_ਸਮਾਂ, timeout;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ file_ਲੰਮਾਈ, read_ਲੰਮਾਈ;
} xyz;

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ xyzModem_CHAR_TIMEOUT            2000	/* 2 seconds */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ xyzModem_MAX_RETRIES             20
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ xyz_Modem_MAX_RETRIES_WITH_ਚੱਕਰੀ_ਜਾਂਚ    10
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ xyz_Modem_CAN_ਗਿਣਤੀ                3	/* Wait for 3 CAN before quitting */

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਪੂਰਨ cyg_int32;
ਸਥਿਰ ਪੂਰਨ
CYGACC_COMM_IF_GETC_TIMEOUT (ਅੱਖਰ chan, ਅੱਖਰ *c)
{

  ulong now = get_timer(0);
  ਸਮਾਂ_ਸਾਰਣੀ();
  ਜਦੋਂ (!tstc ())
    {
      ਜੇ (get_timer(now) > xyzModem_CHAR_TIMEOUT)
        ਰੋਕੋ;
    }
  ਜੇ (tstc ())
    {
      *c = getchar();
      ਵਾਪਸ 1;
    }
  ਵਾਪਸ 0;
}

ਸਥਿਰ ਖਾਲੀ
CYGACC_COMM_IF_PUTC (ਅੱਖਰ x, ਅੱਖਰ y)
{
  putc (y);
}

/* Validate a hex character */
__inline__ ਸਥਿਰ bool
_is_hex (ਅੱਖਰ c)
{
  ਵਾਪਸ (((c >= '0') && (c <= '9')) ||
	  ((c >= 'A') && (c <= 'F')) || ((c >= 'a') && (c <= 'f')));
}

/* Convert a single hex nibble */
__inline__ ਸਥਿਰ ਪੂਰਨ
_from_hex (ਅੱਖਰ c)
{
  ਪੂਰਨ ret = 0;

  ਜੇ ((c >= '0') && (c <= '9'))
    {
      ret = (c - '0');
    }
  ਨਹੀਂਤਾਂ ਜੇ ((c >= 'a') && (c <= 'f'))
    {
      ret = (c - 'a' + 0x0a);
    }
  ਨਹੀਂਤਾਂ ਜੇ ((c >= 'A') && (c <= 'F'))
    {
      ret = (c - 'A' + 0x0A);
    }
  ਵਾਪਸ ret;
}

/* Convert a character to lower case */
__inline__ ਸਥਿਰ ਅੱਖਰ
_tolower (ਅੱਖਰ c)
{
  ਜੇ ((c >= 'A') && (c <= 'Z'))
    {
      c = (c - 'A') + 'a';
    }
  ਵਾਪਸ c;
}

/* Parse (scan) a number */
ਸਥਿਰ bool
parse_ਅੰਕ (ਅੱਖਰ *s, ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ *val, ਅੱਖਰ **es, ਅੱਖਰ *delim)
{
  bool ਪਹਿਲਾ = true;
  ਪੂਰਨ radix = 10;
  ਅੱਖਰ c;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਨਤੀਜਾ = 0;
  ਪੂਰਨ digit;

  ਜਦੋਂ (*s == ' ')
    s++;
  ਜਦੋਂ (*s)
    {
      ਜੇ (ਪਹਿਲਾ && (s[0] == '0') && (_tolower (s[1]) == 'x'))
	{
	  radix = 16;
	  s += 2;
	}
      ਪਹਿਲਾ = false;
      c = *s++;
      ਜੇ (_is_hex (c) && ((digit = _from_hex (c)) < radix))
	{
	  /* Valid digit */
	  ਨਤੀਜਾ = (ਨਤੀਜਾ * radix) + digit;
	}
      ਨਹੀਂਤਾਂ
	{
	  ਜੇ (delim != (ਅੱਖਰ *) 0)
	    {
	      /* See if this character is one of the delimiters */
	      ਅੱਖਰ *dp = delim;
	      ਜਦੋਂ (*dp && (c != *dp))
		dp++;
	      ਜੇ (*dp)
		ਰੋਕੋ;		/* Found a good delimiter */
	    }
	  ਵਾਪਸ false;		/* Malformatted number */
	}
    }
  *val = ਨਤੀਜਾ;
  ਜੇ (es != (ਅੱਖਰ **) 0)
    {
      *es = s;
    }
  ਵਾਪਸ true;
}

#ਜੇ defined(DEBUG) && !CONFIG_IS_ENABLED(USE_TINY_PRINTF)
/*
 * Note: this debug setup works by storing the strings in a fixed buffer
 */
ਸਥਿਰ ਅੱਖਰ zm_debug_buf[8192];
ਸਥਿਰ ਅੱਖਰ *zm_ਬਾਹਰਲਾ = zm_debug_buf;
ਸਥਿਰ ਅੱਖਰ *zm_ਬਾਹਰਲਾ_ਸ਼ੁਰੂ = zm_debug_buf;

ਸਥਿਰ ਪੂਰਨ
zm_dprintf(ਅੱਖਰ *fmt, ...)
{
	ਪੂਰਨ len;
	va_list ਦਲੀਲਾਂ;

	va_ਸ਼ੁਰੂ(ਦਲੀਲਾਂ, fmt);
	len = diag_vsprintf(zm_ਬਾਹਰਲਾ, fmt, ਦਲੀਲਾਂ);
	va_ਸਿਰਾ(ਦਲੀਲਾਂ);
	zm_ਬਾਹਰਲਾ += len;
	ਵਾਪਸ len;
}

ਸਥਿਰ ਖਾਲੀ
zm_flush (ਖਾਲੀ)
{
  zm_ਬਾਹਰਲਾ = zm_ਬਾਹਰਲਾ_ਸ਼ੁਰੂ;
}

ਸਥਿਰ ਖਾਲੀ
zm_dump_buf (ਖਾਲੀ *buf, ਪੂਰਨ len)
{

}

ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ zm_buf[2048];
ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ *zm_bp;

ਸਥਿਰ ਖਾਲੀ
zm_new (ਖਾਲੀ)
{
  zm_bp = zm_buf;
}

ਸਥਿਰ ਖਾਲੀ
zm_save (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ c)
{
  *zm_bp++ = c;
}

ਸਥਿਰ ਖਾਲੀ
zm_dump (ਪੂਰਨ ਸਤਰ)
{
  zm_dprintf ("Packet at line: %d\n", ਸਤਰ);
  zm_dump_buf (zm_buf, zm_bp - zm_buf);
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ZM_DEBUG(x) x
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ZM_DEBUG(x)
#ਜੇ_ਅੰਤ

/* Wait for the line to go idle */
ਸਥਿਰ ਖਾਲੀ
xyzModem_flush (ਖਾਲੀ)
{
  ਪੂਰਨ res;
  ਅੱਖਰ c;
  ਜਦੋਂ (true)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
      ਜੇ (!res)
	ਵਾਪਸ;
    }
}

ਸਥਿਰ ਪੂਰਨ
xyzModem_get_hdr (ਖਾਲੀ)
{
  ਅੱਖਰ c;
  ਪੂਰਨ res;
  bool hdr_ਮਿਲਿਆ = false;
  ਪੂਰਨ i, can_ਕੁੱਲ, hdr_chars;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਛੋਟਾ cksum;

  ZM_DEBUG (zm_new ());
  /* Find the start of a header */
  can_ਕੁੱਲ = 0;
  hdr_chars = 0;

  ਜੇ (xyz.tx_ack)
    {
      CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
      xyz.tx_ack = false;
    }
  ਜਦੋਂ (!hdr_ਮਿਲਿਆ)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
      ZM_DEBUG (zm_save (c));
      ਜੇ (res)
	{
	  hdr_chars++;
	  ਚੋਣ (c)
	    {
	    ਹਾਲਤ SOH:
	      xyz.ਕੁੱਲ_SOH++;
	      fallthrough;
	    ਹਾਲਤ STX:
	      ਜੇ (c == STX)
		xyz.ਕੁੱਲ_STX++;
	      hdr_ਮਿਲਿਆ = true;
	      ਰੋਕੋ;
	    ਹਾਲਤ CAN:
	    ਹਾਲਤ ETX:
	      xyz.ਕੁੱਲ_CAN++;
	      ZM_DEBUG (zm_dump (__LINE__));
	      ਜੇ (++can_ਕੁੱਲ == xyz_Modem_CAN_ਗਿਣਤੀ)
		{
		  ਵਾਪਸ xyzModem_cancel;
		}
	      ਨਹੀਂਤਾਂ
		{
		  /* Wait for multiple CAN to avoid early quits */
		  ਰੋਕੋ;
		}
	    ਹਾਲਤ EOT:
	      /* EOT only supported if no noise */
	      ਜੇ (hdr_chars == 1)
		{
		  CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		  ZM_DEBUG (zm_dprintf ("ACK on EOT #%d\n", __LINE__));
		  ZM_DEBUG (zm_dump (__LINE__));
		  ਵਾਪਸ xyzModem_eof;
		}
	    ਮੂਲ:
	      /* Ignore, waiting for start of header */
	      ;
	    }
	}
      ਨਹੀਂਤਾਂ
	{
	  /* Data stream timed out */
	  xyzModem_flush ();	/* Toss any current input */
	  ZM_DEBUG (zm_dump (__LINE__));
	  CYGACC_CALL_IF_DELAY_US ((cyg_int32) 250000);
	  ਵਾਪਸ xyzModem_timeout;
	}
    }

  /* Header found, now read the data */
  res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (ਅੱਖਰ *) &xyz.blk);
  ZM_DEBUG (zm_save (xyz.blk));
  ਜੇ (!res)
    {
      ZM_DEBUG (zm_dump (__LINE__));
      ਵਾਪਸ xyzModem_timeout;
    }
  res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (ਅੱਖਰ *) &xyz.cblk);
  ZM_DEBUG (zm_save (xyz.cblk));
  ਜੇ (!res)
    {
      ZM_DEBUG (zm_dump (__LINE__));
      ਵਾਪਸ xyzModem_timeout;
    }
  xyz.len = (c == SOH) ? 128 : 1024;
  xyz.bufp = xyz.pkt;
  ਲਈ (i = 0; i < xyz.len; i++)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
      ZM_DEBUG (zm_save (c));
      ਜੇ (res)
	{
	  xyz.pkt[i] = c;
	}
      ਨਹੀਂਤਾਂ
	{
	  ZM_DEBUG (zm_dump (__LINE__));
	  ਵਾਪਸ xyzModem_timeout;
	}
    }
  res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (ਅੱਖਰ *) &xyz.crc1);
  ZM_DEBUG (zm_save (xyz.crc1));
  ਜੇ (!res)
    {
      ZM_DEBUG (zm_dump (__LINE__));
      ਵਾਪਸ xyzModem_timeout;
    }
  ਜੇ (xyz.ਚੱਕਰੀ_ਜਾਂਚ_ਬਹੁਲਕ)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (ਅੱਖਰ *) &xyz.crc2);
      ZM_DEBUG (zm_save (xyz.crc2));
      ਜੇ (!res)
	{
	  ZM_DEBUG (zm_dump (__LINE__));
	  ਵਾਪਸ xyzModem_timeout;
	}
    }
  ZM_DEBUG (zm_dump (__LINE__));
  /* Validate the message */
  ਜੇ ((xyz.blk ^ xyz.cblk) != (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ) 0xFF)
    {
      ZM_DEBUG (zm_dprintf
		("Framing error - blk: %x/%x/%x\n", xyz.blk, xyz.cblk,
		 (xyz.blk ^ xyz.cblk)));
      ZM_DEBUG (zm_dump_buf (xyz.pkt, xyz.len));
      xyzModem_flush ();
      ਵਾਪਸ xyz_Modem_ਚੌਖਟਾ;
    }
  /* Verify checksum/CRC */
  ਜੇ (xyz.ਚੱਕਰੀ_ਜਾਂਚ_ਬਹੁਲਕ)
    {
      cksum = crc16_ccitt(0, xyz.pkt, xyz.len);
      ਜੇ (cksum != ((xyz.crc1 << 8) | xyz.crc2))
	{
	  ZM_DEBUG (zm_dprintf ("CRC error - recvd: %02x%02x, computed: %x\n",
				xyz.crc1, xyz.crc2, cksum & 0xFFFF));
	  ਵਾਪਸ xyzModem_cksum;
	}
    }
  ਨਹੀਂਤਾਂ
    {
      cksum = 0;
      ਲਈ (i = 0; i < xyz.len; i++)
	{
	  cksum += xyz.pkt[i];
	}
      ਜੇ (xyz.crc1 != (cksum & 0xFF))
	{
	  ZM_DEBUG (zm_dprintf
		    ("Checksum error - recvd: %x, computed: %x\n", xyz.crc1,
		     cksum & 0xFF));
	  ਵਾਪਸ xyzModem_cksum;
	}
    }
  /* If we get here, the message passes [structural] muster */
  ਵਾਪਸ 0;
}

ਸਥਿਰ
ulong
xyzModem_get_initial_timeout (ਖਾਲੀ)
{
  /* timeout is in seconds, non-positive timeout value is infinity */
#ਜੇ CONFIG_IS_ENABLED(ENV_SUPPORT)
  ਅਚਲ ਅੱਖਰ *timeout_str = env_get("loadxy_timeout");
  ਜੇ (timeout_str)
    ਵਾਪਸ 1000 * simple_strtol(timeout_str, ਨਿਰਾ, 10);
#ਜੇ_ਅੰਤ
  ਵਾਪਸ 1000 * CONFIG_CMD_LOADXY_TIMEOUT;
}

ਪੂਰਨ
xyz_Modem_ਧਾਰਾ_open (connection_info_t * info, ਪੂਰਨ *err)
{
  ਪੂਰਨ stat = 0;
  ਪੂਰਨ retries = xyzModem_MAX_RETRIES;
  ਪੂਰਨ ਚੱਕਰੀ_ਜਾਂਚ_retries = xyz_Modem_MAX_RETRIES_WITH_ਚੱਕਰੀ_ਜਾਂਚ;

/*    ZM_DEBUG(zm_out = zm_out_start); */
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ xyzModem_zmodem
  ਜੇ (info->ਬਹੁਲਕ == xyzModem_zmodem)
    {
      *err = xyzModem_noZmodem;
      ਵਾਪਸ -1;
    }
#ਜੇ_ਅੰਤ

/* TODO: CHECK ! */
  ਪੂਰਨ dummy = 0;
  xyz.__chan = &dummy;
  xyz.len = 0;
  xyz.pad_pending = 0;
  xyz.pad_deliver = 0;
  xyz.ਚੱਕਰੀ_ਜਾਂਚ_ਬਹੁਲਕ = true;
  xyz.at_eof = false;
  xyz.tx_ack = false;
  xyz.ਬਹੁਲਕ = info->ਬਹੁਲਕ;
  xyz.ਕੁੱਲ_retries = 0;
  xyz.ਕੁੱਲ_SOH = 0;
  xyz.ਕੁੱਲ_STX = 0;
  xyz.ਕੁੱਲ_CAN = 0;
  xyz.read_ਲੰਮਾਈ = 0;
  xyz.file_ਲੰਮਾਈ = 0;
  xyz.ਪਹਿਲਾ_xmodem_packet = false;
  xyz.initial_ਸਮਾਂ = get_timer(0);
  xyz.timeout = xyzModem_get_initial_timeout();

  CYGACC_COMM_IF_PUTC (*xyz.__chan, (xyz.ਚੱਕਰੀ_ਜਾਂਚ_ਬਹੁਲਕ ? 'C' : NAK));

  ਜੇ (xyz.ਬਹੁਲਕ == xyzModem_xmodem)
    {
      /* X-modem doesn't have an information header - exit here */
      xyz.ਪਹਿਲਾ_xmodem_packet = true;
      xyz.ਅਗਲਾ_blk = 1;
      ਵਾਪਸ 0;
    }

  ਜਦੋਂ (!(xyz.timeout && get_timer(xyz.initial_ਸਮਾਂ) > xyz.timeout))
    {
      ਜੇ (--retries <= 0)
        {
          retries = xyzModem_MAX_RETRIES;
          ਚੱਕਰੀ_ਜਾਂਚ_retries = xyz_Modem_MAX_RETRIES_WITH_ਚੱਕਰੀ_ਜਾਂਚ;
          xyz.ਚੱਕਰੀ_ਜਾਂਚ_ਬਹੁਲਕ = true;
        }
      stat = xyzModem_get_hdr ();
      ਜੇ (stat == 0)
	{
	  /* Y-modem file information header */
	  ਜੇ (xyz.blk == 0)
	    {
	      /* skip filename */
	      ਜਦੋਂ (*xyz.bufp++);
	      /* get the length */
	      parse_ਅੰਕ ((ਅੱਖਰ *) xyz.bufp, &xyz.file_ਲੰਮਾਈ, ਨਿਰਾ, " ");
	      /* The rest of the file name data block quietly discarded */
	      xyz.tx_ack = true;
	    }
	  xyz.ਅਗਲਾ_blk = 1;
	  xyz.len = 0;
	  ਵਾਪਸ 0;
	}
      ਨਹੀਂਤਾਂ ਜੇ (stat == xyzModem_timeout)
	{
	  ਜੇ (--ਚੱਕਰੀ_ਜਾਂਚ_retries <= 0)
	    xyz.ਚੱਕਰੀ_ਜਾਂਚ_ਬਹੁਲਕ = false;
	  CYGACC_CALL_IF_DELAY_US (5 * 100000);	/* Extra delay for startup */
	  CYGACC_COMM_IF_PUTC (*xyz.__chan, (xyz.ਚੱਕਰੀ_ਜਾਂਚ_ਬਹੁਲਕ ? 'C' : NAK));
	  xyz.ਕੁੱਲ_retries++;
	  ZM_DEBUG (zm_dprintf ("NAK (%d)\n", __LINE__));
	}
      ਜੇ (stat == xyzModem_cancel)
	{
	  ਰੋਕੋ;
	}
    }
  *err = stat;
  ZM_DEBUG (zm_flush ());
  ਵਾਪਸ -1;
}

ਪੂਰਨ
xyz_Modem_ਧਾਰਾ_read (ਅੱਖਰ *buf, ਪੂਰਨ ਮਾਪ, ਪੂਰਨ *err)
{
  ਪੂਰਨ stat, ਕੁੱਲ, len;
  ਪੂਰਨ retries;

  ਕੁੱਲ = 0;
  stat = xyzModem_cancel;
  /* Try and get 'size' bytes into the buffer */
  ਜਦੋਂ (!xyz.at_eof && xyz.len >= 0 && (ਮਾਪ > 0))
    {
      ਜੇ (xyz.len == 0 && xyz.pad_deliver == 0)
	{
	  retries = xyzModem_MAX_RETRIES;
	  ਜਦੋਂ (retries-- > 0)
	    {
	      ਜੇ (xyz.ਪਹਿਲਾ_xmodem_packet && xyz.timeout &&
		  get_timer(xyz.initial_ਸਮਾਂ) > xyz.timeout)
		{
		  *err = xyzModem_timeout;
		  xyz.len = -1;
		  ਵਾਪਸ ਕੁੱਲ;
		}

	      stat = xyzModem_get_hdr ();
	      ਜੇ (stat == 0)
		{
		  ਜੇ (xyz.ਬਹੁਲਕ == xyzModem_xmodem && xyz.ਪਹਿਲਾ_xmodem_packet)
		    xyz.ਪਹਿਲਾ_xmodem_packet = false;
		  ਜੇ (xyz.blk == xyz.ਅਗਲਾ_blk)
		    {
		      xyz.tx_ack = true;
		      ZM_DEBUG (zm_dprintf
				("ACK block %d (%d)\n", xyz.blk, __LINE__));
		      xyz.ਅਗਲਾ_blk = (xyz.ਅਗਲਾ_blk + 1) & 0xFF;

		      ਜੇ (xyz.ਬਹੁਲਕ == xyzModem_xmodem || xyz.file_ਲੰਮਾਈ == 0)
			{
			  /* Without a file length, EOF padding can't be
			   * distinguished from block data until the next
			   * frame arrives and we can check for EOT/EOF.
			   */
			  xyz.pad_deliver = xyz.pad_pending;
			  xyz.pad_pending = 0;
			  ਜੇ ((xyz.bufp[xyz.len - 1] == EOF) &&
			      (xyz.bufp[xyz.len - 2] == EOF) &&
			      (xyz.bufp[xyz.len - 3] == EOF))
			    {
			      ਜਦੋਂ (xyz.len
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
		      ਜੇ (xyz.ਬਹੁਲਕ != xyzModem_xmodem && 0 != xyz.file_ਲੰਮਾਈ)
			{
			  xyz.read_ਲੰਮਾਈ += xyz.len;
			  ਜੇ (xyz.read_ਲੰਮਾਈ > xyz.file_ਲੰਮਾਈ)
			    {
			      xyz.len -= (xyz.read_ਲੰਮਾਈ - xyz.file_ਲੰਮਾਈ);
			    }
			}
		      ਰੋਕੋ;
		    }
		  ਨਹੀਂਤਾਂ ਜੇ (xyz.blk == ((xyz.ਅਗਲਾ_blk - 1) & 0xFF))
		    {
		      /* Just re-ACK this so sender will get on with it */
		      CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		      ਜਾਰੀ;	/* Need new header */
		    }
		  ਨਹੀਂਤਾਂ
		    {
		      stat = xyzModem_sequence;
		    }
		}
	      ਜੇ (stat == xyzModem_cancel)
		{
		  ਰੋਕੋ;
		}
	      ਜੇ (stat == xyzModem_eof)
		{
		  CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		  ZM_DEBUG (zm_dprintf ("ACK (%d)\n", __LINE__));
		  ਜੇ (xyz.ਬਹੁਲਕ == xyzModem_ymodem)
		    {
		      CYGACC_COMM_IF_PUTC (*xyz.__chan,
					   (xyz.ਚੱਕਰੀ_ਜਾਂਚ_ਬਹੁਲਕ ? 'C' : NAK));
		      xyz.ਕੁੱਲ_retries++;
		      ZM_DEBUG (zm_dprintf ("Reading Final Header\n"));
		      stat = xyzModem_get_hdr ();
		      CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		      ZM_DEBUG (zm_dprintf ("FINAL ACK (%d)\n", __LINE__));
		    }
		  ਨਹੀਂਤਾਂ
		    stat = 0;

		  xyz.pad_pending = 0;	/* EOT: withheld ^Z run was padding */
		  xyz.at_eof = true;
		  ਰੋਕੋ;
		}
	      CYGACC_COMM_IF_PUTC (*xyz.__chan, (xyz.ਚੱਕਰੀ_ਜਾਂਚ_ਬਹੁਲਕ ? 'C' : NAK));
	      xyz.ਕੁੱਲ_retries++;
	      ZM_DEBUG (zm_dprintf ("NAK (%d)\n", __LINE__));
	    }
	  ਜੇ (stat < 0 && (!xyz.ਪਹਿਲਾ_xmodem_packet || stat != xyzModem_timeout))
	    {
	      *err = stat;
	      xyz.len = -1;
	      ਵਾਪਸ ਕੁੱਲ;
	    }
	}
      /* A ^Z run withheld from the previous block turned out to be data */
      ਜੇ (xyz.pad_deliver > 0)
	{
	  len = xyz.pad_deliver;
	  ਜੇ (ਮਾਪ < len)
	    len = ਮਾਪ;
	  memset (buf, EOF, len);
	  ਮਾਪ -= len;
	  buf += len;
	  ਕੁੱਲ += len;
	  xyz.pad_deliver -= len;
	}
      /* Don't "read" data from the EOF protocol package */
      ਜੇ (!xyz.at_eof && xyz.len > 0)
	{
	  len = xyz.len;
	  ਜੇ (ਮਾਪ < len)
	    len = ਮਾਪ;
	  ਸਮ੍ਰਿਤੀ_ਨਕਲ (buf, xyz.bufp, len);
	  ਮਾਪ -= len;
	  buf += len;
	  ਕੁੱਲ += len;
	  xyz.len -= len;
	  xyz.bufp += len;
	}
    }
  ਵਾਪਸ ਕੁੱਲ;
}

ਖਾਲੀ
xyz_Modem_ਧਾਰਾ_close (ਪੂਰਨ *err)
{
  ZM_DEBUG (zm_dprintf
    ("xyzModem - %s mode, %d(SOH)/%d(STX)/%d(CAN) packets, %d retries\n",
     xyz.ਚੱਕਰੀ_ਜਾਂਚ_ਬਹੁਲਕ ? "CRC" : "Cksum", xyz.ਕੁੱਲ_SOH, xyz.ਕੁੱਲ_STX,
     xyz.ਕੁੱਲ_CAN, xyz.ਕੁੱਲ_retries));
  ZM_DEBUG (zm_flush ());
}

/* Need to be able to clean out the input buffer, so have to take the */
/* getc */
ਖਾਲੀ
xyz_Modem_ਧਾਰਾ_terminate (bool abort, ਪੂਰਨ (*getc) (ਖਾਲੀ))
{
  ਪੂਰਨ c;

  ਜੇ (abort)
    {
      ZM_DEBUG (zm_dprintf ("!!!! TRANSFER ABORT !!!!\n"));
      ਚੋਣ (xyz.ਬਹੁਲਕ)
	{
	ਹਾਲਤ xyzModem_xmodem:
	ਹਾਲਤ xyzModem_ymodem:
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
	  ਰੋਕੋ;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ xyzModem_zmodem
	ਹਾਲਤ xyzModem_zmodem:
	  /* Might support it some day I suppose. */
#ਜੇ_ਅੰਤ
	  ਰੋਕੋ;
	}
    }
  ਨਹੀਂਤਾਂ
    {
      ZM_DEBUG (zm_dprintf ("Engaging cleanup mode...\n"));
      /*
       * Consume any trailing crap left in the inbuffer from
       * previous received blocks. Since very few files are an exact multiple
       * of the transfer block size, there will almost always be some gunk here.
       * If we don't eat it now, RedBoot will think the user typed it.
       */
      ZM_DEBUG (zm_dprintf ("Trailing gunk:\n"));
      ਜਦੋਂ ((c = (*getc) ()) > -1)
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

ਅੱਖਰ *
xyzModem_error (ਪੂਰਨ err)
{
  ਚੋਣ (err)
    {
    ਹਾਲਤ xyzModem_access:
      ਵਾਪਸ "Can't access file";
      ਰੋਕੋ;
    ਹਾਲਤ xyzModem_noZmodem:
      ਵਾਪਸ "Sorry, zModem not available yet";
      ਰੋਕੋ;
    ਹਾਲਤ xyzModem_timeout:
      ਵਾਪਸ "Timed out";
      ਰੋਕੋ;
    ਹਾਲਤ xyzModem_eof:
      ਵਾਪਸ "End of file";
      ਰੋਕੋ;
    ਹਾਲਤ xyzModem_cancel:
      ਵਾਪਸ "Cancelled";
      ਰੋਕੋ;
    ਹਾਲਤ xyz_Modem_ਚੌਖਟਾ:
      ਵਾਪਸ "Invalid framing";
      ਰੋਕੋ;
    ਹਾਲਤ xyzModem_cksum:
      ਵਾਪਸ "CRC/checksum error";
      ਰੋਕੋ;
    ਹਾਲਤ xyzModem_sequence:
      ਵਾਪਸ "Block sequence error";
      ਰੋਕੋ;
    ਮੂਲ:
      ਵਾਪਸ "Unknown error";
      ਰੋਕੋ;
    }
}

/*
 * RedBoot interface
 */
