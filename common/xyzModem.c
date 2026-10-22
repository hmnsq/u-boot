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
#అన్తర్భావ <xyzModem.h>
#అన్తర్భావ <stdarg.h>
#అన్తర్భావ <time.h>
#అన్తర్భావ <u-boot/crc.h>
#అన్తర్భావ <watchdog.h>
#అన్తర్భావ <env.h>
#అన్తర్భావ <vsprintf.h>

/* Assumption - run xyzModem protocol over the console port */

/* Values magic to the protocol */
#నిర్వచించు SOH 0x01
#నిర్వచించు STX 0x02
#నిర్వచించు ETX 0x03		/* ^C for interrupt */
#నిర్వచించు EOT 0x04
#నిర్వచించు ACK 0x06
#నిర్వచించు BSP 0x08
#నిర్వచించు NAK 0x15
#నిర్వచించు CAN 0x18
#నిర్వచించు EOF 0x1A		/* ^Z for DOS officionados */

/* Data & state local to the protocol */
స్థిర నిర్మాణం
{
  పూర్ణాంకం *__chan;
  గుర్తులేని అక్షరం pkt[1024], *bufp;
  పూర్ణాంకం pad_pending;	/* trailing ^Z run withheld from the previous block */
  పూర్ణాంకం pad_deliver;	/* ^Z bytes to emit before the next data */
  గుర్తులేని అక్షరం blk, cblk, crc1, crc2;
  గుర్తులేని అక్షరం తదుపరి_blk;	/* Expected block */
  పూర్ణాంకం len, బహుళకం, మొత్తం_retries;
  పూర్ణాంకం మొత్తం_SOH, మొత్తం_STX, మొత్తం_CAN;
  bool చక్రీయ_తనిఖీ_బహుళకం, at_eof, tx_ack;
  bool మొదటి_xmodem_packet;
  ulong initial_సమయం, timeout;
  గుర్తులేని దీర్ఘ file_నిడివి, read_నిడివి;
} xyz;

#నిర్వచించు xyzModem_CHAR_TIMEOUT            2000	/* 2 seconds */
#నిర్వచించు xyzModem_MAX_RETRIES             20
#నిర్వచించు xyz_Modem_MAX_RETRIES_WITH_చక్రీయ_తనిఖీ    10
#నిర్వచించు xyz_Modem_CAN_లెక్క                3	/* Wait for 3 CAN before quitting */

రకనిర్వచనం పూర్ణాంకం cyg_int32;
స్థిర పూర్ణాంకం
CYGACC_COMM_IF_GETC_TIMEOUT (అక్షరం chan, అక్షరం *c)
{

  ulong now = get_timer(0);
  కాలపట్టిక();
  అయితే (!tstc ())
    {
      ఒకవేళ (get_timer(now) > xyzModem_CHAR_TIMEOUT)
        ఆపు;
    }
  ఒకవేళ (tstc ())
    {
      *c = getchar();
      తిరిగి 1;
    }
  తిరిగి 0;
}

స్థిర శూన్య
CYGACC_COMM_IF_PUTC (అక్షరం x, అక్షరం y)
{
  putc (y);
}

/* Validate a hex character */
__inline__ స్థిర bool
_is_hex (అక్షరం c)
{
  తిరిగి (((c >= '0') && (c <= '9')) ||
	  ((c >= 'A') && (c <= 'F')) || ((c >= 'a') && (c <= 'f')));
}

/* Convert a single hex nibble */
__inline__ స్థిర పూర్ణాంకం
_from_hex (అక్షరం c)
{
  పూర్ణాంకం ret = 0;

  ఒకవేళ ((c >= '0') && (c <= '9'))
    {
      ret = (c - '0');
    }
  లేకపోతే ఒకవేళ ((c >= 'a') && (c <= 'f'))
    {
      ret = (c - 'a' + 0x0a);
    }
  లేకపోతే ఒకవేళ ((c >= 'A') && (c <= 'F'))
    {
      ret = (c - 'A' + 0x0A);
    }
  తిరిగి ret;
}

/* Convert a character to lower case */
__inline__ స్థిర అక్షరం
_tolower (అక్షరం c)
{
  ఒకవేళ ((c >= 'A') && (c <= 'Z'))
    {
      c = (c - 'A') + 'a';
    }
  తిరిగి c;
}

/* Parse (scan) a number */
స్థిర bool
parse_అంకె (అక్షరం *s, గుర్తులేని దీర్ఘ *val, అక్షరం **es, అక్షరం *delim)
{
  bool మొదటి = true;
  పూర్ణాంకం radix = 10;
  అక్షరం c;
  గుర్తులేని దీర్ఘ ఫలితం = 0;
  పూర్ణాంకం digit;

  అయితే (*s == ' ')
    s++;
  అయితే (*s)
    {
      ఒకవేళ (మొదటి && (s[0] == '0') && (_tolower (s[1]) == 'x'))
	{
	  radix = 16;
	  s += 2;
	}
      మొదటి = false;
      c = *s++;
      ఒకవేళ (_is_hex (c) && ((digit = _from_hex (c)) < radix))
	{
	  /* Valid digit */
	  ఫలితం = (ఫలితం * radix) + digit;
	}
      లేకపోతే
	{
	  ఒకవేళ (delim != (అక్షరం *) 0)
	    {
	      /* See if this character is one of the delimiters */
	      అక్షరం *dp = delim;
	      అయితే (*dp && (c != *dp))
		dp++;
	      ఒకవేళ (*dp)
		ఆపు;		/* Found a good delimiter */
	    }
	  తిరిగి false;		/* Malformatted number */
	}
    }
  *val = ఫలితం;
  ఒకవేళ (es != (అక్షరం **) 0)
    {
      *es = s;
    }
  తిరిగి true;
}

#ఒకవేళ defined(DEBUG) && !CONFIG_IS_ENABLED(USE_TINY_PRINTF)
/*
 * Note: this debug setup works by storing the strings in a fixed buffer
 */
స్థిర అక్షరం zm_debug_buf[8192];
స్థిర అక్షరం *zm_బహిర్గతం = zm_debug_buf;
స్థిర అక్షరం *zm_బహిర్గతం_ప్రారంభం = zm_debug_buf;

స్థిర పూర్ణాంకం
zm_dprintf(అక్షరం *fmt, ...)
{
	పూర్ణాంకం len;
	va_list వాదనలు;

	va_ప్రారంభం(వాదనలు, fmt);
	len = diag_vsprintf(zm_బహిర్గతం, fmt, వాదనలు);
	va_చివర(వాదనలు);
	zm_బహిర్గతం += len;
	తిరిగి len;
}

స్థిర శూన్య
zm_flush (శూన్య)
{
  zm_బహిర్గతం = zm_బహిర్గతం_ప్రారంభం;
}

స్థిర శూన్య
zm_dump_buf (శూన్య *buf, పూర్ణాంకం len)
{

}

స్థిర గుర్తులేని అక్షరం zm_buf[2048];
స్థిర గుర్తులేని అక్షరం *zm_bp;

స్థిర శూన్య
zm_new (శూన్య)
{
  zm_bp = zm_buf;
}

స్థిర శూన్య
zm_save (గుర్తులేని అక్షరం c)
{
  *zm_bp++ = c;
}

స్థిర శూన్య
zm_dump (పూర్ణాంకం పంక్తి)
{
  zm_dprintf ("Packet at line: %d\n", పంక్తి);
  zm_dump_buf (zm_buf, zm_bp - zm_buf);
}

#నిర్వచించు ZM_DEBUG(x) x
#లేకపోతే
#నిర్వచించు ZM_DEBUG(x)
#అయితే_ముగింపు

/* Wait for the line to go idle */
స్థిర శూన్య
xyzModem_flush (శూన్య)
{
  పూర్ణాంకం res;
  అక్షరం c;
  అయితే (true)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
      ఒకవేళ (!res)
	తిరిగి;
    }
}

స్థిర పూర్ణాంకం
xyzModem_get_hdr (శూన్య)
{
  అక్షరం c;
  పూర్ణాంకం res;
  bool hdr_దొరికింది = false;
  పూర్ణాంకం i, can_మొత్తం, hdr_chars;
  గుర్తులేని పొట్టి cksum;

  ZM_DEBUG (zm_new ());
  /* Find the start of a header */
  can_మొత్తం = 0;
  hdr_chars = 0;

  ఒకవేళ (xyz.tx_ack)
    {
      CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
      xyz.tx_ack = false;
    }
  అయితే (!hdr_దొరికింది)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
      ZM_DEBUG (zm_save (c));
      ఒకవేళ (res)
	{
	  hdr_chars++;
	  ఎంపిక (c)
	    {
	    స్థితి SOH:
	      xyz.మొత్తం_SOH++;
	      fallthrough;
	    స్థితి STX:
	      ఒకవేళ (c == STX)
		xyz.మొత్తం_STX++;
	      hdr_దొరికింది = true;
	      ఆపు;
	    స్థితి CAN:
	    స్థితి ETX:
	      xyz.మొత్తం_CAN++;
	      ZM_DEBUG (zm_dump (__LINE__));
	      ఒకవేళ (++can_మొత్తం == xyz_Modem_CAN_లెక్క)
		{
		  తిరిగి xyzModem_cancel;
		}
	      లేకపోతే
		{
		  /* Wait for multiple CAN to avoid early quits */
		  ఆపు;
		}
	    స్థితి EOT:
	      /* EOT only supported if no noise */
	      ఒకవేళ (hdr_chars == 1)
		{
		  CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		  ZM_DEBUG (zm_dprintf ("ACK on EOT #%d\n", __LINE__));
		  ZM_DEBUG (zm_dump (__LINE__));
		  తిరిగి xyzModem_eof;
		}
	    అప్రమేయ:
	      /* Ignore, waiting for start of header */
	      ;
	    }
	}
      లేకపోతే
	{
	  /* Data stream timed out */
	  xyzModem_flush ();	/* Toss any current input */
	  ZM_DEBUG (zm_dump (__LINE__));
	  CYGACC_CALL_IF_DELAY_US ((cyg_int32) 250000);
	  తిరిగి xyzModem_timeout;
	}
    }

  /* Header found, now read the data */
  res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (అక్షరం *) &xyz.blk);
  ZM_DEBUG (zm_save (xyz.blk));
  ఒకవేళ (!res)
    {
      ZM_DEBUG (zm_dump (__LINE__));
      తిరిగి xyzModem_timeout;
    }
  res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (అక్షరం *) &xyz.cblk);
  ZM_DEBUG (zm_save (xyz.cblk));
  ఒకవేళ (!res)
    {
      ZM_DEBUG (zm_dump (__LINE__));
      తిరిగి xyzModem_timeout;
    }
  xyz.len = (c == SOH) ? 128 : 1024;
  xyz.bufp = xyz.pkt;
  కొరకు (i = 0; i < xyz.len; i++)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
      ZM_DEBUG (zm_save (c));
      ఒకవేళ (res)
	{
	  xyz.pkt[i] = c;
	}
      లేకపోతే
	{
	  ZM_DEBUG (zm_dump (__LINE__));
	  తిరిగి xyzModem_timeout;
	}
    }
  res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (అక్షరం *) &xyz.crc1);
  ZM_DEBUG (zm_save (xyz.crc1));
  ఒకవేళ (!res)
    {
      ZM_DEBUG (zm_dump (__LINE__));
      తిరిగి xyzModem_timeout;
    }
  ఒకవేళ (xyz.చక్రీయ_తనిఖీ_బహుళకం)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (అక్షరం *) &xyz.crc2);
      ZM_DEBUG (zm_save (xyz.crc2));
      ఒకవేళ (!res)
	{
	  ZM_DEBUG (zm_dump (__LINE__));
	  తిరిగి xyzModem_timeout;
	}
    }
  ZM_DEBUG (zm_dump (__LINE__));
  /* Validate the message */
  ఒకవేళ ((xyz.blk ^ xyz.cblk) != (గుర్తులేని అక్షరం) 0xFF)
    {
      ZM_DEBUG (zm_dprintf
		("Framing error - blk: %x/%x/%x\n", xyz.blk, xyz.cblk,
		 (xyz.blk ^ xyz.cblk)));
      ZM_DEBUG (zm_dump_buf (xyz.pkt, xyz.len));
      xyzModem_flush ();
      తిరిగి xyz_Modem_చట్రం;
    }
  /* Verify checksum/CRC */
  ఒకవేళ (xyz.చక్రీయ_తనిఖీ_బహుళకం)
    {
      cksum = crc16_ccitt(0, xyz.pkt, xyz.len);
      ఒకవేళ (cksum != ((xyz.crc1 << 8) | xyz.crc2))
	{
	  ZM_DEBUG (zm_dprintf ("CRC error - recvd: %02x%02x, computed: %x\n",
				xyz.crc1, xyz.crc2, cksum & 0xFFFF));
	  తిరిగి xyzModem_cksum;
	}
    }
  లేకపోతే
    {
      cksum = 0;
      కొరకు (i = 0; i < xyz.len; i++)
	{
	  cksum += xyz.pkt[i];
	}
      ఒకవేళ (xyz.crc1 != (cksum & 0xFF))
	{
	  ZM_DEBUG (zm_dprintf
		    ("Checksum error - recvd: %x, computed: %x\n", xyz.crc1,
		     cksum & 0xFF));
	  తిరిగి xyzModem_cksum;
	}
    }
  /* If we get here, the message passes [structural] muster */
  తిరిగి 0;
}

స్థిర
ulong
xyzModem_get_initial_timeout (శూన్య)
{
  /* timeout is in seconds, non-positive timeout value is infinity */
#ఒకవేళ CONFIG_IS_ENABLED(ENV_SUPPORT)
  స్థిరరాశి అక్షరం *timeout_str = env_get("loadxy_timeout");
  ఒకవేళ (timeout_str)
    తిరిగి 1000 * simple_strtol(timeout_str, ఖాళీ, 10);
#అయితే_ముగింపు
  తిరిగి 1000 * CONFIG_CMD_LOADXY_TIMEOUT;
}

పూర్ణాంకం
xyz_Modem_ప్రవాహం_open (connection_info_t * info, పూర్ణాంకం *err)
{
  పూర్ణాంకం stat = 0;
  పూర్ణాంకం retries = xyzModem_MAX_RETRIES;
  పూర్ణాంకం చక్రీయ_తనిఖీ_retries = xyz_Modem_MAX_RETRIES_WITH_చక్రీయ_తనిఖీ;

/*    ZM_DEBUG(zm_out = zm_out_start); */
#నిర్వచితమైతే xyzModem_zmodem
  ఒకవేళ (info->బహుళకం == xyzModem_zmodem)
    {
      *err = xyzModem_noZmodem;
      తిరిగి -1;
    }
#అయితే_ముగింపు

/* TODO: CHECK ! */
  పూర్ణాంకం dummy = 0;
  xyz.__chan = &dummy;
  xyz.len = 0;
  xyz.pad_pending = 0;
  xyz.pad_deliver = 0;
  xyz.చక్రీయ_తనిఖీ_బహుళకం = true;
  xyz.at_eof = false;
  xyz.tx_ack = false;
  xyz.బహుళకం = info->బహుళకం;
  xyz.మొత్తం_retries = 0;
  xyz.మొత్తం_SOH = 0;
  xyz.మొత్తం_STX = 0;
  xyz.మొత్తం_CAN = 0;
  xyz.read_నిడివి = 0;
  xyz.file_నిడివి = 0;
  xyz.మొదటి_xmodem_packet = false;
  xyz.initial_సమయం = get_timer(0);
  xyz.timeout = xyzModem_get_initial_timeout();

  CYGACC_COMM_IF_PUTC (*xyz.__chan, (xyz.చక్రీయ_తనిఖీ_బహుళకం ? 'C' : NAK));

  ఒకవేళ (xyz.బహుళకం == xyzModem_xmodem)
    {
      /* X-modem doesn't have an information header - exit here */
      xyz.మొదటి_xmodem_packet = true;
      xyz.తదుపరి_blk = 1;
      తిరిగి 0;
    }

  అయితే (!(xyz.timeout && get_timer(xyz.initial_సమయం) > xyz.timeout))
    {
      ఒకవేళ (--retries <= 0)
        {
          retries = xyzModem_MAX_RETRIES;
          చక్రీయ_తనిఖీ_retries = xyz_Modem_MAX_RETRIES_WITH_చక్రీయ_తనిఖీ;
          xyz.చక్రీయ_తనిఖీ_బహుళకం = true;
        }
      stat = xyzModem_get_hdr ();
      ఒకవేళ (stat == 0)
	{
	  /* Y-modem file information header */
	  ఒకవేళ (xyz.blk == 0)
	    {
	      /* skip filename */
	      అయితే (*xyz.bufp++);
	      /* get the length */
	      parse_అంకె ((అక్షరం *) xyz.bufp, &xyz.file_నిడివి, ఖాళీ, " ");
	      /* The rest of the file name data block quietly discarded */
	      xyz.tx_ack = true;
	    }
	  xyz.తదుపరి_blk = 1;
	  xyz.len = 0;
	  తిరిగి 0;
	}
      లేకపోతే ఒకవేళ (stat == xyzModem_timeout)
	{
	  ఒకవేళ (--చక్రీయ_తనిఖీ_retries <= 0)
	    xyz.చక్రీయ_తనిఖీ_బహుళకం = false;
	  CYGACC_CALL_IF_DELAY_US (5 * 100000);	/* Extra delay for startup */
	  CYGACC_COMM_IF_PUTC (*xyz.__chan, (xyz.చక్రీయ_తనిఖీ_బహుళకం ? 'C' : NAK));
	  xyz.మొత్తం_retries++;
	  ZM_DEBUG (zm_dprintf ("NAK (%d)\n", __LINE__));
	}
      ఒకవేళ (stat == xyzModem_cancel)
	{
	  ఆపు;
	}
    }
  *err = stat;
  ZM_DEBUG (zm_flush ());
  తిరిగి -1;
}

పూర్ణాంకం
xyz_Modem_ప్రవాహం_read (అక్షరం *buf, పూర్ణాంకం కొలత, పూర్ణాంకం *err)
{
  పూర్ణాంకం stat, మొత్తం, len;
  పూర్ణాంకం retries;

  మొత్తం = 0;
  stat = xyzModem_cancel;
  /* Try and get 'size' bytes into the buffer */
  అయితే (!xyz.at_eof && xyz.len >= 0 && (కొలత > 0))
    {
      ఒకవేళ (xyz.len == 0 && xyz.pad_deliver == 0)
	{
	  retries = xyzModem_MAX_RETRIES;
	  అయితే (retries-- > 0)
	    {
	      ఒకవేళ (xyz.మొదటి_xmodem_packet && xyz.timeout &&
		  get_timer(xyz.initial_సమయం) > xyz.timeout)
		{
		  *err = xyzModem_timeout;
		  xyz.len = -1;
		  తిరిగి మొత్తం;
		}

	      stat = xyzModem_get_hdr ();
	      ఒకవేళ (stat == 0)
		{
		  ఒకవేళ (xyz.బహుళకం == xyzModem_xmodem && xyz.మొదటి_xmodem_packet)
		    xyz.మొదటి_xmodem_packet = false;
		  ఒకవేళ (xyz.blk == xyz.తదుపరి_blk)
		    {
		      xyz.tx_ack = true;
		      ZM_DEBUG (zm_dprintf
				("ACK block %d (%d)\n", xyz.blk, __LINE__));
		      xyz.తదుపరి_blk = (xyz.తదుపరి_blk + 1) & 0xFF;

		      ఒకవేళ (xyz.బహుళకం == xyzModem_xmodem || xyz.file_నిడివి == 0)
			{
			  /* Without a file length, EOF padding can't be
			   * distinguished from block data until the next
			   * frame arrives and we can check for EOT/EOF.
			   */
			  xyz.pad_deliver = xyz.pad_pending;
			  xyz.pad_pending = 0;
			  ఒకవేళ ((xyz.bufp[xyz.len - 1] == EOF) &&
			      (xyz.bufp[xyz.len - 2] == EOF) &&
			      (xyz.bufp[xyz.len - 3] == EOF))
			    {
			      అయితే (xyz.len
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
		      ఒకవేళ (xyz.బహుళకం != xyzModem_xmodem && 0 != xyz.file_నిడివి)
			{
			  xyz.read_నిడివి += xyz.len;
			  ఒకవేళ (xyz.read_నిడివి > xyz.file_నిడివి)
			    {
			      xyz.len -= (xyz.read_నిడివి - xyz.file_నిడివి);
			    }
			}
		      ఆపు;
		    }
		  లేకపోతే ఒకవేళ (xyz.blk == ((xyz.తదుపరి_blk - 1) & 0xFF))
		    {
		      /* Just re-ACK this so sender will get on with it */
		      CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		      కొనసాగు;	/* Need new header */
		    }
		  లేకపోతే
		    {
		      stat = xyzModem_sequence;
		    }
		}
	      ఒకవేళ (stat == xyzModem_cancel)
		{
		  ఆపు;
		}
	      ఒకవేళ (stat == xyzModem_eof)
		{
		  CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		  ZM_DEBUG (zm_dprintf ("ACK (%d)\n", __LINE__));
		  ఒకవేళ (xyz.బహుళకం == xyzModem_ymodem)
		    {
		      CYGACC_COMM_IF_PUTC (*xyz.__chan,
					   (xyz.చక్రీయ_తనిఖీ_బహుళకం ? 'C' : NAK));
		      xyz.మొత్తం_retries++;
		      ZM_DEBUG (zm_dprintf ("Reading Final Header\n"));
		      stat = xyzModem_get_hdr ();
		      CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		      ZM_DEBUG (zm_dprintf ("FINAL ACK (%d)\n", __LINE__));
		    }
		  లేకపోతే
		    stat = 0;

		  xyz.pad_pending = 0;	/* EOT: withheld ^Z run was padding */
		  xyz.at_eof = true;
		  ఆపు;
		}
	      CYGACC_COMM_IF_PUTC (*xyz.__chan, (xyz.చక్రీయ_తనిఖీ_బహుళకం ? 'C' : NAK));
	      xyz.మొత్తం_retries++;
	      ZM_DEBUG (zm_dprintf ("NAK (%d)\n", __LINE__));
	    }
	  ఒకవేళ (stat < 0 && (!xyz.మొదటి_xmodem_packet || stat != xyzModem_timeout))
	    {
	      *err = stat;
	      xyz.len = -1;
	      తిరిగి మొత్తం;
	    }
	}
      /* A ^Z run withheld from the previous block turned out to be data */
      ఒకవేళ (xyz.pad_deliver > 0)
	{
	  len = xyz.pad_deliver;
	  ఒకవేళ (కొలత < len)
	    len = కొలత;
	  memset (buf, EOF, len);
	  కొలత -= len;
	  buf += len;
	  మొత్తం += len;
	  xyz.pad_deliver -= len;
	}
      /* Don't "read" data from the EOF protocol package */
      ఒకవేళ (!xyz.at_eof && xyz.len > 0)
	{
	  len = xyz.len;
	  ఒకవేళ (కొలత < len)
	    len = కొలత;
	  జ్ఞాపకం_నకలు (buf, xyz.bufp, len);
	  కొలత -= len;
	  buf += len;
	  మొత్తం += len;
	  xyz.len -= len;
	  xyz.bufp += len;
	}
    }
  తిరిగి మొత్తం;
}

శూన్య
xyz_Modem_ప్రవాహం_close (పూర్ణాంకం *err)
{
  ZM_DEBUG (zm_dprintf
    ("xyzModem - %s mode, %d(SOH)/%d(STX)/%d(CAN) packets, %d retries\n",
     xyz.చక్రీయ_తనిఖీ_బహుళకం ? "CRC" : "Cksum", xyz.మొత్తం_SOH, xyz.మొత్తం_STX,
     xyz.మొత్తం_CAN, xyz.మొత్తం_retries));
  ZM_DEBUG (zm_flush ());
}

/* Need to be able to clean out the input buffer, so have to take the */
/* getc */
శూన్య
xyz_Modem_ప్రవాహం_terminate (bool abort, పూర్ణాంకం (*getc) (శూన్య))
{
  పూర్ణాంకం c;

  ఒకవేళ (abort)
    {
      ZM_DEBUG (zm_dprintf ("!!!! TRANSFER ABORT !!!!\n"));
      ఎంపిక (xyz.బహుళకం)
	{
	స్థితి xyzModem_xmodem:
	స్థితి xyzModem_ymodem:
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
	  ఆపు;
#నిర్వచితమైతే xyzModem_zmodem
	స్థితి xyzModem_zmodem:
	  /* Might support it some day I suppose. */
#అయితే_ముగింపు
	  ఆపు;
	}
    }
  లేకపోతే
    {
      ZM_DEBUG (zm_dprintf ("Engaging cleanup mode...\n"));
      /*
       * Consume any trailing crap left in the inbuffer from
       * previous received blocks. Since very few files are an exact multiple
       * of the transfer block size, there will almost always be some gunk here.
       * If we don't eat it now, RedBoot will think the user typed it.
       */
      ZM_DEBUG (zm_dprintf ("Trailing gunk:\n"));
      అయితే ((c = (*getc) ()) > -1)
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

అక్షరం *
xyzModem_error (పూర్ణాంకం err)
{
  ఎంపిక (err)
    {
    స్థితి xyzModem_access:
      తిరిగి "Can't access file";
      ఆపు;
    స్థితి xyzModem_noZmodem:
      తిరిగి "Sorry, zModem not available yet";
      ఆపు;
    స్థితి xyzModem_timeout:
      తిరిగి "Timed out";
      ఆపు;
    స్థితి xyzModem_eof:
      తిరిగి "End of file";
      ఆపు;
    స్థితి xyzModem_cancel:
      తిరిగి "Cancelled";
      ఆపు;
    స్థితి xyz_Modem_చట్రం:
      తిరిగి "Invalid framing";
      ఆపు;
    స్థితి xyzModem_cksum:
      తిరిగి "CRC/checksum error";
      ఆపు;
    స్థితి xyzModem_sequence:
      తిరిగి "Block sequence error";
      ఆపు;
    అప్రమేయ:
      తిరిగి "Unknown error";
      ఆపు;
    }
}

/*
 * RedBoot interface
 */
