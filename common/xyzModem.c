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
#ಅನ್ತರ್ಭಾವ <xyzModem.h>
#ಅನ್ತರ್ಭಾವ <stdarg.h>
#ಅನ್ತರ್ಭಾವ <time.h>
#ಅನ್ತರ್ಭಾವ <u-boot/crc.h>
#ಅನ್ತರ್ಭಾವ <watchdog.h>
#ಅನ್ತರ್ಭಾವ <env.h>
#ಅನ್ತರ್ಭಾವ <vsprintf.h>

/* Assumption - run xyzModem protocol over the console port */

/* Values magic to the protocol */
#ವ್ಯಾಖ್ಯಾನಿಸು SOH 0x01
#ವ್ಯಾಖ್ಯಾನಿಸು STX 0x02
#ವ್ಯಾಖ್ಯಾನಿಸು ETX 0x03		/* ^C for interrupt */
#ವ್ಯಾಖ್ಯಾನಿಸು EOT 0x04
#ವ್ಯಾಖ್ಯಾನಿಸು ACK 0x06
#ವ್ಯಾಖ್ಯಾನಿಸು BSP 0x08
#ವ್ಯಾಖ್ಯಾನಿಸು NAK 0x15
#ವ್ಯಾಖ್ಯಾನಿಸು CAN 0x18
#ವ್ಯಾಖ್ಯಾನಿಸು EOF 0x1A		/* ^Z for DOS officionados */

/* Data & state local to the protocol */
ಸ್ಥಿರ ರಚನೆ
{
  ಪೂರ್ಣಾಂಕ *__chan;
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ pkt[1024], *bufp;
  ಪೂರ್ಣಾಂಕ pad_pending;	/* trailing ^Z run withheld from the previous block */
  ಪೂರ್ಣಾಂಕ pad_deliver;	/* ^Z bytes to emit before the next data */
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ blk, cblk, crc1, crc2;
  ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ ಮುಂದಿನ_blk;	/* Expected block */
  ಪೂರ್ಣಾಂಕ len, ಬಹುಲಕ, ಒಟ್ಟು_retries;
  ಪೂರ್ಣಾಂಕ ಒಟ್ಟು_SOH, ಒಟ್ಟು_STX, ಒಟ್ಟು_CAN;
  bool ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_ಬಹುಲಕ, at_eof, tx_ack;
  bool ಮೊದಲ_xmodem_packet;
  ulong initial_ಸಮಯ, timeout;
  ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ file_ಉದ್ದಳತೆ, read_ಉದ್ದಳತೆ;
} xyz;

#ವ್ಯಾಖ್ಯಾನಿಸು xyzModem_CHAR_TIMEOUT            2000	/* 2 seconds */
#ವ್ಯಾಖ್ಯಾನಿಸು xyzModem_MAX_RETRIES             20
#ವ್ಯಾಖ್ಯಾನಿಸು xyz_Modem_MAX_RETRIES_WITH_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ    10
#ವ್ಯಾಖ್ಯಾನಿಸು xyz_Modem_CAN_ಎಣಿಕೆ                3	/* Wait for 3 CAN before quitting */

ಪ್ರಕಾರವ್ಯಾಖ್ಯೆ ಪೂರ್ಣಾಂಕ cyg_int32;
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
CYGACC_COMM_IF_GETC_TIMEOUT (ಅಕ್ಷರ chan, ಅಕ್ಷರ *c)
{

  ulong now = get_timer(0);
  ವೇಳಾಪಟ್ಟಿ();
  ಆಗ (!tstc ())
    {
      ಆದರೆ (get_timer(now) > xyzModem_CHAR_TIMEOUT)
        ನಿಲ್ಲಿಸು;
    }
  ಆದರೆ (tstc ())
    {
      *c = getchar();
      ಹಿಂತಿರುಗಿ 1;
    }
  ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ ಶೂನ್ಯ
CYGACC_COMM_IF_PUTC (ಅಕ್ಷರ x, ಅಕ್ಷರ y)
{
  putc (y);
}

/* Validate a hex character */
__inline__ ಸ್ಥಿರ bool
_is_hex (ಅಕ್ಷರ c)
{
  ಹಿಂತಿರುಗಿ (((c >= '0') && (c <= '9')) ||
	  ((c >= 'A') && (c <= 'F')) || ((c >= 'a') && (c <= 'f')));
}

/* Convert a single hex nibble */
__inline__ ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
_from_hex (ಅಕ್ಷರ c)
{
  ಪೂರ್ಣಾಂಕ ret = 0;

  ಆದರೆ ((c >= '0') && (c <= '9'))
    {
      ret = (c - '0');
    }
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((c >= 'a') && (c <= 'f'))
    {
      ret = (c - 'a' + 0x0a);
    }
  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ ((c >= 'A') && (c <= 'F'))
    {
      ret = (c - 'A' + 0x0A);
    }
  ಹಿಂತಿರುಗಿ ret;
}

/* Convert a character to lower case */
__inline__ ಸ್ಥಿರ ಅಕ್ಷರ
_tolower (ಅಕ್ಷರ c)
{
  ಆದರೆ ((c >= 'A') && (c <= 'Z'))
    {
      c = (c - 'A') + 'a';
    }
  ಹಿಂತಿರುಗಿ c;
}

/* Parse (scan) a number */
ಸ್ಥಿರ bool
parse_ಅಂಕೆ (ಅಕ್ಷರ *s, ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ *val, ಅಕ್ಷರ **es, ಅಕ್ಷರ *delim)
{
  bool ಮೊದಲ = true;
  ಪೂರ್ಣಾಂಕ radix = 10;
  ಅಕ್ಷರ c;
  ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ಫಲಿತಾಂಶ = 0;
  ಪೂರ್ಣಾಂಕ digit;

  ಆಗ (*s == ' ')
    s++;
  ಆಗ (*s)
    {
      ಆದರೆ (ಮೊದಲ && (s[0] == '0') && (_tolower (s[1]) == 'x'))
	{
	  radix = 16;
	  s += 2;
	}
      ಮೊದಲ = false;
      c = *s++;
      ಆದರೆ (_is_hex (c) && ((digit = _from_hex (c)) < radix))
	{
	  /* Valid digit */
	  ಫಲಿತಾಂಶ = (ಫಲಿತಾಂಶ * radix) + digit;
	}
      ಇಲ್ಲದಿದ್ದರೆ
	{
	  ಆದರೆ (delim != (ಅಕ್ಷರ *) 0)
	    {
	      /* See if this character is one of the delimiters */
	      ಅಕ್ಷರ *dp = delim;
	      ಆಗ (*dp && (c != *dp))
		dp++;
	      ಆದರೆ (*dp)
		ನಿಲ್ಲಿಸು;		/* Found a good delimiter */
	    }
	  ಹಿಂತಿರುಗಿ false;		/* Malformatted number */
	}
    }
  *val = ಫಲಿತಾಂಶ;
  ಆದರೆ (es != (ಅಕ್ಷರ **) 0)
    {
      *es = s;
    }
  ಹಿಂತಿರುಗಿ true;
}

#ಆದರೆ defined(DEBUG) && !CONFIG_IS_ENABLED(USE_TINY_PRINTF)
/*
 * Note: this debug setup works by storing the strings in a fixed buffer
 */
ಸ್ಥಿರ ಅಕ್ಷರ zm_debug_buf[8192];
ಸ್ಥಿರ ಅಕ್ಷರ *zm_ಹೊರಹೋಗು = zm_debug_buf;
ಸ್ಥಿರ ಅಕ್ಷರ *zm_ಹೊರಹೋಗು_ಆರಂಭ = zm_debug_buf;

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
zm_dprintf(ಅಕ್ಷರ *fmt, ...)
{
	ಪೂರ್ಣಾಂಕ len;
	va_list ವಾದಗಳು;

	va_ಆರಂಭ(ವಾದಗಳು, fmt);
	len = diag_vsprintf(zm_ಹೊರಹೋಗು, fmt, ವಾದಗಳು);
	va_ಕೊನೆ(ವಾದಗಳು);
	zm_ಹೊರಹೋಗು += len;
	ಹಿಂತಿರುಗಿ len;
}

ಸ್ಥಿರ ಶೂನ್ಯ
zm_flush (ಶೂನ್ಯ)
{
  zm_ಹೊರಹೋಗು = zm_ಹೊರಹೋಗು_ಆರಂಭ;
}

ಸ್ಥಿರ ಶೂನ್ಯ
zm_dump_buf (ಶೂನ್ಯ *buf, ಪೂರ್ಣಾಂಕ len)
{

}

ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ zm_buf[2048];
ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ *zm_bp;

ಸ್ಥಿರ ಶೂನ್ಯ
zm_new (ಶೂನ್ಯ)
{
  zm_bp = zm_buf;
}

ಸ್ಥಿರ ಶೂನ್ಯ
zm_save (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ c)
{
  *zm_bp++ = c;
}

ಸ್ಥಿರ ಶೂನ್ಯ
zm_dump (ಪೂರ್ಣಾಂಕ ಸಾಲು)
{
  zm_dprintf ("Packet at line: %d\n", ಸಾಲು);
  zm_dump_buf (zm_buf, zm_bp - zm_buf);
}

#ವ್ಯಾಖ್ಯಾನಿಸು ZM_DEBUG(x) x
#ಇಲ್ಲದಿದ್ದರೆ
#ವ್ಯಾಖ್ಯಾನಿಸು ZM_DEBUG(x)
#ಆದರೆ_ಅಂತ್ಯ

/* Wait for the line to go idle */
ಸ್ಥಿರ ಶೂನ್ಯ
xyzModem_flush (ಶೂನ್ಯ)
{
  ಪೂರ್ಣಾಂಕ res;
  ಅಕ್ಷರ c;
  ಆಗ (true)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
      ಆದರೆ (!res)
	ಹಿಂತಿರುಗಿ;
    }
}

ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ
xyzModem_get_hdr (ಶೂನ್ಯ)
{
  ಅಕ್ಷರ c;
  ಪೂರ್ಣಾಂಕ res;
  bool hdr_ಸಿಕ್ಕಿತು = false;
  ಪೂರ್ಣಾಂಕ i, can_ಒಟ್ಟು, hdr_chars;
  ಚಿಹ್ನೆರಹಿತ ಗಿಡ್ಡ cksum;

  ZM_DEBUG (zm_new ());
  /* Find the start of a header */
  can_ಒಟ್ಟು = 0;
  hdr_chars = 0;

  ಆದರೆ (xyz.tx_ack)
    {
      CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
      xyz.tx_ack = false;
    }
  ಆಗ (!hdr_ಸಿಕ್ಕಿತು)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
      ZM_DEBUG (zm_save (c));
      ಆದರೆ (res)
	{
	  hdr_chars++;
	  ಆಯ್ಕೆ (c)
	    {
	    ಸ್ಥಿತಿ SOH:
	      xyz.ಒಟ್ಟು_SOH++;
	      fallthrough;
	    ಸ್ಥಿತಿ STX:
	      ಆದರೆ (c == STX)
		xyz.ಒಟ್ಟು_STX++;
	      hdr_ಸಿಕ್ಕಿತು = true;
	      ನಿಲ್ಲಿಸು;
	    ಸ್ಥಿತಿ CAN:
	    ಸ್ಥಿತಿ ETX:
	      xyz.ಒಟ್ಟು_CAN++;
	      ZM_DEBUG (zm_dump (__LINE__));
	      ಆದರೆ (++can_ಒಟ್ಟು == xyz_Modem_CAN_ಎಣಿಕೆ)
		{
		  ಹಿಂತಿರುಗಿ xyzModem_cancel;
		}
	      ಇಲ್ಲದಿದ್ದರೆ
		{
		  /* Wait for multiple CAN to avoid early quits */
		  ನಿಲ್ಲಿಸು;
		}
	    ಸ್ಥಿತಿ EOT:
	      /* EOT only supported if no noise */
	      ಆದರೆ (hdr_chars == 1)
		{
		  CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		  ZM_DEBUG (zm_dprintf ("ACK on EOT #%d\n", __LINE__));
		  ZM_DEBUG (zm_dump (__LINE__));
		  ಹಿಂತಿರುಗಿ xyzModem_eof;
		}
	    ಪೂರ್ವನಿಯೋಜಿತ:
	      /* Ignore, waiting for start of header */
	      ;
	    }
	}
      ಇಲ್ಲದಿದ್ದರೆ
	{
	  /* Data stream timed out */
	  xyzModem_flush ();	/* Toss any current input */
	  ZM_DEBUG (zm_dump (__LINE__));
	  CYGACC_CALL_IF_DELAY_US ((cyg_int32) 250000);
	  ಹಿಂತಿರುಗಿ xyzModem_timeout;
	}
    }

  /* Header found, now read the data */
  res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (ಅಕ್ಷರ *) &xyz.blk);
  ZM_DEBUG (zm_save (xyz.blk));
  ಆದರೆ (!res)
    {
      ZM_DEBUG (zm_dump (__LINE__));
      ಹಿಂತಿರುಗಿ xyzModem_timeout;
    }
  res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (ಅಕ್ಷರ *) &xyz.cblk);
  ZM_DEBUG (zm_save (xyz.cblk));
  ಆದರೆ (!res)
    {
      ZM_DEBUG (zm_dump (__LINE__));
      ಹಿಂತಿರುಗಿ xyzModem_timeout;
    }
  xyz.len = (c == SOH) ? 128 : 1024;
  xyz.bufp = xyz.pkt;
  ಗಾಗಿ (i = 0; i < xyz.len; i++)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
      ZM_DEBUG (zm_save (c));
      ಆದರೆ (res)
	{
	  xyz.pkt[i] = c;
	}
      ಇಲ್ಲದಿದ್ದರೆ
	{
	  ZM_DEBUG (zm_dump (__LINE__));
	  ಹಿಂತಿರುಗಿ xyzModem_timeout;
	}
    }
  res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (ಅಕ್ಷರ *) &xyz.crc1);
  ZM_DEBUG (zm_save (xyz.crc1));
  ಆದರೆ (!res)
    {
      ZM_DEBUG (zm_dump (__LINE__));
      ಹಿಂತಿರುಗಿ xyzModem_timeout;
    }
  ಆದರೆ (xyz.ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_ಬಹುಲಕ)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (ಅಕ್ಷರ *) &xyz.crc2);
      ZM_DEBUG (zm_save (xyz.crc2));
      ಆದರೆ (!res)
	{
	  ZM_DEBUG (zm_dump (__LINE__));
	  ಹಿಂತಿರುಗಿ xyzModem_timeout;
	}
    }
  ZM_DEBUG (zm_dump (__LINE__));
  /* Validate the message */
  ಆದರೆ ((xyz.blk ^ xyz.cblk) != (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ) 0xFF)
    {
      ZM_DEBUG (zm_dprintf
		("Framing error - blk: %x/%x/%x\n", xyz.blk, xyz.cblk,
		 (xyz.blk ^ xyz.cblk)));
      ZM_DEBUG (zm_dump_buf (xyz.pkt, xyz.len));
      xyzModem_flush ();
      ಹಿಂತಿರುಗಿ xyz_Modem_ಚೌಕಟ್ಟು;
    }
  /* Verify checksum/CRC */
  ಆದರೆ (xyz.ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_ಬಹುಲಕ)
    {
      cksum = crc16_ccitt(0, xyz.pkt, xyz.len);
      ಆದರೆ (cksum != ((xyz.crc1 << 8) | xyz.crc2))
	{
	  ZM_DEBUG (zm_dprintf ("CRC error - recvd: %02x%02x, computed: %x\n",
				xyz.crc1, xyz.crc2, cksum & 0xFFFF));
	  ಹಿಂತಿರುಗಿ xyzModem_cksum;
	}
    }
  ಇಲ್ಲದಿದ್ದರೆ
    {
      cksum = 0;
      ಗಾಗಿ (i = 0; i < xyz.len; i++)
	{
	  cksum += xyz.pkt[i];
	}
      ಆದರೆ (xyz.crc1 != (cksum & 0xFF))
	{
	  ZM_DEBUG (zm_dprintf
		    ("Checksum error - recvd: %x, computed: %x\n", xyz.crc1,
		     cksum & 0xFF));
	  ಹಿಂತಿರುಗಿ xyzModem_cksum;
	}
    }
  /* If we get here, the message passes [structural] muster */
  ಹಿಂತಿರುಗಿ 0;
}

ಸ್ಥಿರ
ulong
xyzModem_get_initial_timeout (ಶೂನ್ಯ)
{
  /* timeout is in seconds, non-positive timeout value is infinity */
#ಆದರೆ CONFIG_IS_ENABLED(ENV_SUPPORT)
  ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ *timeout_str = env_get("loadxy_timeout");
  ಆದರೆ (timeout_str)
    ಹಿಂತಿರುಗಿ 1000 * simple_strtol(timeout_str, ಖಾಲಿ, 10);
#ಆದರೆ_ಅಂತ್ಯ
  ಹಿಂತಿರುಗಿ 1000 * CONFIG_CMD_LOADXY_TIMEOUT;
}

ಪೂರ್ಣಾಂಕ
xyz_Modem_ಹರಿವು_open (connection_info_t * info, ಪೂರ್ಣಾಂಕ *err)
{
  ಪೂರ್ಣಾಂಕ stat = 0;
  ಪೂರ್ಣಾಂಕ retries = xyzModem_MAX_RETRIES;
  ಪೂರ್ಣಾಂಕ ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_retries = xyz_Modem_MAX_RETRIES_WITH_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ;

/*    ZM_DEBUG(zm_out = zm_out_start); */
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ xyzModem_zmodem
  ಆದರೆ (info->ಬಹುಲಕ == xyzModem_zmodem)
    {
      *err = xyzModem_noZmodem;
      ಹಿಂತಿರುಗಿ -1;
    }
#ಆದರೆ_ಅಂತ್ಯ

/* TODO: CHECK ! */
  ಪೂರ್ಣಾಂಕ dummy = 0;
  xyz.__chan = &dummy;
  xyz.len = 0;
  xyz.pad_pending = 0;
  xyz.pad_deliver = 0;
  xyz.ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_ಬಹುಲಕ = true;
  xyz.at_eof = false;
  xyz.tx_ack = false;
  xyz.ಬಹುಲಕ = info->ಬಹುಲಕ;
  xyz.ಒಟ್ಟು_retries = 0;
  xyz.ಒಟ್ಟು_SOH = 0;
  xyz.ಒಟ್ಟು_STX = 0;
  xyz.ಒಟ್ಟು_CAN = 0;
  xyz.read_ಉದ್ದಳತೆ = 0;
  xyz.file_ಉದ್ದಳತೆ = 0;
  xyz.ಮೊದಲ_xmodem_packet = false;
  xyz.initial_ಸಮಯ = get_timer(0);
  xyz.timeout = xyzModem_get_initial_timeout();

  CYGACC_COMM_IF_PUTC (*xyz.__chan, (xyz.ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_ಬಹುಲಕ ? 'C' : NAK));

  ಆದರೆ (xyz.ಬಹುಲಕ == xyzModem_xmodem)
    {
      /* X-modem doesn't have an information header - exit here */
      xyz.ಮೊದಲ_xmodem_packet = true;
      xyz.ಮುಂದಿನ_blk = 1;
      ಹಿಂತಿರುಗಿ 0;
    }

  ಆಗ (!(xyz.timeout && get_timer(xyz.initial_ಸಮಯ) > xyz.timeout))
    {
      ಆದರೆ (--retries <= 0)
        {
          retries = xyzModem_MAX_RETRIES;
          ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_retries = xyz_Modem_MAX_RETRIES_WITH_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ;
          xyz.ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_ಬಹುಲಕ = true;
        }
      stat = xyzModem_get_hdr ();
      ಆದರೆ (stat == 0)
	{
	  /* Y-modem file information header */
	  ಆದರೆ (xyz.blk == 0)
	    {
	      /* skip filename */
	      ಆಗ (*xyz.bufp++);
	      /* get the length */
	      parse_ಅಂಕೆ ((ಅಕ್ಷರ *) xyz.bufp, &xyz.file_ಉದ್ದಳತೆ, ಖಾಲಿ, " ");
	      /* The rest of the file name data block quietly discarded */
	      xyz.tx_ack = true;
	    }
	  xyz.ಮುಂದಿನ_blk = 1;
	  xyz.len = 0;
	  ಹಿಂತಿರುಗಿ 0;
	}
      ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (stat == xyzModem_timeout)
	{
	  ಆದರೆ (--ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_retries <= 0)
	    xyz.ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_ಬಹುಲಕ = false;
	  CYGACC_CALL_IF_DELAY_US (5 * 100000);	/* Extra delay for startup */
	  CYGACC_COMM_IF_PUTC (*xyz.__chan, (xyz.ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_ಬಹುಲಕ ? 'C' : NAK));
	  xyz.ಒಟ್ಟು_retries++;
	  ZM_DEBUG (zm_dprintf ("NAK (%d)\n", __LINE__));
	}
      ಆದರೆ (stat == xyzModem_cancel)
	{
	  ನಿಲ್ಲಿಸು;
	}
    }
  *err = stat;
  ZM_DEBUG (zm_flush ());
  ಹಿಂತಿರುಗಿ -1;
}

ಪೂರ್ಣಾಂಕ
xyz_Modem_ಹರಿವು_read (ಅಕ್ಷರ *buf, ಪೂರ್ಣಾಂಕ ಗಾತ್ರ_2, ಪೂರ್ಣಾಂಕ *err)
{
  ಪೂರ್ಣಾಂಕ stat, ಒಟ್ಟು, len;
  ಪೂರ್ಣಾಂಕ retries;

  ಒಟ್ಟು = 0;
  stat = xyzModem_cancel;
  /* Try and get 'size' bytes into the buffer */
  ಆಗ (!xyz.at_eof && xyz.len >= 0 && (ಗಾತ್ರ_2 > 0))
    {
      ಆದರೆ (xyz.len == 0 && xyz.pad_deliver == 0)
	{
	  retries = xyzModem_MAX_RETRIES;
	  ಆಗ (retries-- > 0)
	    {
	      ಆದರೆ (xyz.ಮೊದಲ_xmodem_packet && xyz.timeout &&
		  get_timer(xyz.initial_ಸಮಯ) > xyz.timeout)
		{
		  *err = xyzModem_timeout;
		  xyz.len = -1;
		  ಹಿಂತಿರುಗಿ ಒಟ್ಟು;
		}

	      stat = xyzModem_get_hdr ();
	      ಆದರೆ (stat == 0)
		{
		  ಆದರೆ (xyz.ಬಹುಲಕ == xyzModem_xmodem && xyz.ಮೊದಲ_xmodem_packet)
		    xyz.ಮೊದಲ_xmodem_packet = false;
		  ಆದರೆ (xyz.blk == xyz.ಮುಂದಿನ_blk)
		    {
		      xyz.tx_ack = true;
		      ZM_DEBUG (zm_dprintf
				("ACK block %d (%d)\n", xyz.blk, __LINE__));
		      xyz.ಮುಂದಿನ_blk = (xyz.ಮುಂದಿನ_blk + 1) & 0xFF;

		      ಆದರೆ (xyz.ಬಹುಲಕ == xyzModem_xmodem || xyz.file_ಉದ್ದಳತೆ == 0)
			{
			  /* Without a file length, EOF padding can't be
			   * distinguished from block data until the next
			   * frame arrives and we can check for EOT/EOF.
			   */
			  xyz.pad_deliver = xyz.pad_pending;
			  xyz.pad_pending = 0;
			  ಆದರೆ ((xyz.bufp[xyz.len - 1] == EOF) &&
			      (xyz.bufp[xyz.len - 2] == EOF) &&
			      (xyz.bufp[xyz.len - 3] == EOF))
			    {
			      ಆಗ (xyz.len
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
		      ಆದರೆ (xyz.ಬಹುಲಕ != xyzModem_xmodem && 0 != xyz.file_ಉದ್ದಳತೆ)
			{
			  xyz.read_ಉದ್ದಳತೆ += xyz.len;
			  ಆದರೆ (xyz.read_ಉದ್ದಳತೆ > xyz.file_ಉದ್ದಳತೆ)
			    {
			      xyz.len -= (xyz.read_ಉದ್ದಳತೆ - xyz.file_ಉದ್ದಳತೆ);
			    }
			}
		      ನಿಲ್ಲಿಸು;
		    }
		  ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (xyz.blk == ((xyz.ಮುಂದಿನ_blk - 1) & 0xFF))
		    {
		      /* Just re-ACK this so sender will get on with it */
		      CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		      ಮುಂದುವರಿ;	/* Need new header */
		    }
		  ಇಲ್ಲದಿದ್ದರೆ
		    {
		      stat = xyzModem_sequence;
		    }
		}
	      ಆದರೆ (stat == xyzModem_cancel)
		{
		  ನಿಲ್ಲಿಸು;
		}
	      ಆದರೆ (stat == xyzModem_eof)
		{
		  CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		  ZM_DEBUG (zm_dprintf ("ACK (%d)\n", __LINE__));
		  ಆದರೆ (xyz.ಬಹುಲಕ == xyzModem_ymodem)
		    {
		      CYGACC_COMM_IF_PUTC (*xyz.__chan,
					   (xyz.ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_ಬಹುಲಕ ? 'C' : NAK));
		      xyz.ಒಟ್ಟು_retries++;
		      ZM_DEBUG (zm_dprintf ("Reading Final Header\n"));
		      stat = xyzModem_get_hdr ();
		      CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		      ZM_DEBUG (zm_dprintf ("FINAL ACK (%d)\n", __LINE__));
		    }
		  ಇಲ್ಲದಿದ್ದರೆ
		    stat = 0;

		  xyz.pad_pending = 0;	/* EOT: withheld ^Z run was padding */
		  xyz.at_eof = true;
		  ನಿಲ್ಲಿಸು;
		}
	      CYGACC_COMM_IF_PUTC (*xyz.__chan, (xyz.ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_ಬಹುಲಕ ? 'C' : NAK));
	      xyz.ಒಟ್ಟು_retries++;
	      ZM_DEBUG (zm_dprintf ("NAK (%d)\n", __LINE__));
	    }
	  ಆದರೆ (stat < 0 && (!xyz.ಮೊದಲ_xmodem_packet || stat != xyzModem_timeout))
	    {
	      *err = stat;
	      xyz.len = -1;
	      ಹಿಂತಿರುಗಿ ಒಟ್ಟು;
	    }
	}
      /* A ^Z run withheld from the previous block turned out to be data */
      ಆದರೆ (xyz.pad_deliver > 0)
	{
	  len = xyz.pad_deliver;
	  ಆದರೆ (ಗಾತ್ರ_2 < len)
	    len = ಗಾತ್ರ_2;
	  memset (buf, EOF, len);
	  ಗಾತ್ರ_2 -= len;
	  buf += len;
	  ಒಟ್ಟು += len;
	  xyz.pad_deliver -= len;
	}
      /* Don't "read" data from the EOF protocol package */
      ಆದರೆ (!xyz.at_eof && xyz.len > 0)
	{
	  len = xyz.len;
	  ಆದರೆ (ಗಾತ್ರ_2 < len)
	    len = ಗಾತ್ರ_2;
	  ಸ್ಮರಣೆ_ನಕಲು (buf, xyz.bufp, len);
	  ಗಾತ್ರ_2 -= len;
	  buf += len;
	  ಒಟ್ಟು += len;
	  xyz.len -= len;
	  xyz.bufp += len;
	}
    }
  ಹಿಂತಿರುಗಿ ಒಟ್ಟು;
}

ಶೂನ್ಯ
xyz_Modem_ಹರಿವು_close (ಪೂರ್ಣಾಂಕ *err)
{
  ZM_DEBUG (zm_dprintf
    ("xyzModem - %s mode, %d(SOH)/%d(STX)/%d(CAN) packets, %d retries\n",
     xyz.ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_ಬಹುಲಕ ? "CRC" : "Cksum", xyz.ಒಟ್ಟು_SOH, xyz.ಒಟ್ಟು_STX,
     xyz.ಒಟ್ಟು_CAN, xyz.ಒಟ್ಟು_retries));
  ZM_DEBUG (zm_flush ());
}

/* Need to be able to clean out the input buffer, so have to take the */
/* getc */
ಶೂನ್ಯ
xyz_Modem_ಹರಿವು_terminate (bool abort, ಪೂರ್ಣಾಂಕ (*getc) (ಶೂನ್ಯ))
{
  ಪೂರ್ಣಾಂಕ c;

  ಆದರೆ (abort)
    {
      ZM_DEBUG (zm_dprintf ("!!!! TRANSFER ABORT !!!!\n"));
      ಆಯ್ಕೆ (xyz.ಬಹುಲಕ)
	{
	ಸ್ಥಿತಿ xyzModem_xmodem:
	ಸ್ಥಿತಿ xyzModem_ymodem:
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
	  ನಿಲ್ಲಿಸು;
#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ xyzModem_zmodem
	ಸ್ಥಿತಿ xyzModem_zmodem:
	  /* Might support it some day I suppose. */
#ಆದರೆ_ಅಂತ್ಯ
	  ನಿಲ್ಲಿಸು;
	}
    }
  ಇಲ್ಲದಿದ್ದರೆ
    {
      ZM_DEBUG (zm_dprintf ("Engaging cleanup mode...\n"));
      /*
       * Consume any trailing crap left in the inbuffer from
       * previous received blocks. Since very few files are an exact multiple
       * of the transfer block size, there will almost always be some gunk here.
       * If we don't eat it now, RedBoot will think the user typed it.
       */
      ZM_DEBUG (zm_dprintf ("Trailing gunk:\n"));
      ಆಗ ((c = (*getc) ()) > -1)
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

ಅಕ್ಷರ *
xyzModem_error (ಪೂರ್ಣಾಂಕ err)
{
  ಆಯ್ಕೆ (err)
    {
    ಸ್ಥಿತಿ xyzModem_access:
      ಹಿಂತಿರುಗಿ "Can't access file";
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ xyzModem_noZmodem:
      ಹಿಂತಿರುಗಿ "Sorry, zModem not available yet";
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ xyzModem_timeout:
      ಹಿಂತಿರುಗಿ "Timed out";
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ xyzModem_eof:
      ಹಿಂತಿರುಗಿ "End of file";
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ xyzModem_cancel:
      ಹಿಂತಿರುಗಿ "Cancelled";
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ xyz_Modem_ಚೌಕಟ್ಟು:
      ಹಿಂತಿರುಗಿ "Invalid framing";
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ xyzModem_cksum:
      ಹಿಂತಿರುಗಿ "CRC/checksum error";
      ನಿಲ್ಲಿಸು;
    ಸ್ಥಿತಿ xyzModem_sequence:
      ಹಿಂತಿರುಗಿ "Block sequence error";
      ನಿಲ್ಲಿಸು;
    ಪೂರ್ವನಿಯೋಜಿತ:
      ಹಿಂತಿರುಗಿ "Unknown error";
      ನಿಲ್ಲಿಸು;
    }
}

/*
 * RedBoot interface
 */
