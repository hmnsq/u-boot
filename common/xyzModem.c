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
#include <xyzModem.h>
#include <stdarg.h>
#include <time.h>
#include <u-boot/crc.h>
#include <watchdog.h>
#include <env.h>
#include <vsprintf.h>

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
static sa_nracanaa
{
  ကိန်းပြည့် *__chan;
  unsigned var_na pkt[1024], *bufp;
  ကိန်းပြည့် pad_pending;	/* trailing ^Z run withheld from the previous block */
  ကိန်းပြည့် pad_deliver;	/* ^Z bytes to emit before the next data */
  unsigned var_na blk, cblk, crc1, crc2;
  unsigned var_na next_blk;	/* Expected block */
  ကိန်းပြည့် len, mode, total_retries;
  ကိန်းပြည့် total_SOH, total_STX, total_CAN;
  bool crc_mode, at_eof, tx_ack;
  bool first_xmodem_packet;
  ulong initial_time, timeout;
  unsigned long file_length, read_length;
} xyz;

#define xyzModem_CHAR_TIMEOUT            2000	/* 2 seconds */
#define xyzModem_MAX_RETRIES             20
#define xyzModem_MAX_RETRIES_WITH_CRC    10
#define xyzModem_CAN_COUNT                3	/* Wait for 3 CAN before quitting */

typedef ကိန်းပြည့် cyg_int32;
static ကိန်းပြည့်
CYGACC_COMM_IF_GETC_TIMEOUT (var_na chan, var_na *c)
{

  ulong now = get_timer(0);
  schedule();
  နေစဉ် (!tstc ())
    {
      အကယ်၍ (get_timer(now) > xyzModem_CHAR_TIMEOUT)
        bha_nga;
    }
  အကယ်၍ (tstc ())
    {
      *c = getchar();
      ပြန်ပေး 1;
    }
  ပြန်ပေး 0;
}

static void
CYGACC_COMM_IF_PUTC (var_na x, var_na y)
{
  putc (y);
}

/* Validate a hex character */
__inline__ static bool
_is_hex (var_na c)
{
  ပြန်ပေး (((c >= '0') && (c <= '9')) ||
	  ((c >= 'A') && (c <= 'F')) || ((c >= 'a') && (c <= 'f')));
}

/* Convert a single hex nibble */
__inline__ static ကိန်းပြည့်
_from_hex (var_na c)
{
  ကိန်းပြည့် ret = 0;

  အကယ်၍ ((c >= '0') && (c <= '9'))
    {
      ret = (c - '0');
    }
  မဟုတ်ပါက အကယ်၍ ((c >= 'a') && (c <= 'f'))
    {
      ret = (c - 'a' + 0x0a);
    }
  မဟုတ်ပါက အကယ်၍ ((c >= 'A') && (c <= 'F'))
    {
      ret = (c - 'A' + 0x0A);
    }
  ပြန်ပေး ret;
}

/* Convert a character to lower case */
__inline__ static var_na
_tolower (var_na c)
{
  အကယ်၍ ((c >= 'A') && (c <= 'Z'))
    {
      c = (c - 'A') + 'a';
    }
  ပြန်ပေး c;
}

/* Parse (scan) a number */
static bool
parse_num (var_na *s, unsigned long *val, var_na **es, var_na *delim)
{
  bool first = true;
  ကိန်းပြည့် radix = 10;
  var_na c;
  unsigned long result = 0;
  ကိန်းပြည့် digit;

  နေစဉ် (*s == ' ')
    s++;
  နေစဉ် (*s)
    {
      အကယ်၍ (first && (s[0] == '0') && (_tolower (s[1]) == 'x'))
	{
	  radix = 16;
	  s += 2;
	}
      first = false;
      c = *s++;
      အကယ်၍ (_is_hex (c) && ((digit = _from_hex (c)) < radix))
	{
	  /* Valid digit */
	  result = (result * radix) + digit;
	}
      မဟုတ်ပါက
	{
	  အကယ်၍ (delim != (var_na *) 0)
	    {
	      /* See if this character is one of the delimiters */
	      var_na *dp = delim;
	      နေစဉ် (*dp && (c != *dp))
		dp++;
	      အကယ်၍ (*dp)
		bha_nga;		/* Found a good delimiter */
	    }
	  ပြန်ပေး false;		/* Malformatted number */
	}
    }
  *val = result;
  အကယ်၍ (es != (var_na **) 0)
    {
      *es = s;
    }
  ပြန်ပေး true;
}

#အကယ်၍ defined(DEBUG) && !CONFIG_IS_ENABLED(USE_TINY_PRINTF)
/*
 * Note: this debug setup works by storing the strings in a fixed buffer
 */
static var_na zm_debug_buf[8192];
static var_na *zm_out = zm_debug_buf;
static var_na *zm_out_start = zm_debug_buf;

static ကိန်းပြည့်
zm_dprintf(var_na *fmt, ...)
{
	ကိန်းပြည့် len;
	va_list args;

	va_start(args, fmt);
	len = diag_vsprintf(zm_out, fmt, args);
	va_end(args);
	zm_out += len;
	ပြန်ပေး len;
}

static void
zm_flush (void)
{
  zm_out = zm_out_start;
}

static void
zm_dump_buf (void *buf, ကိန်းပြည့် len)
{

}

static unsigned var_na zm_buf[2048];
static unsigned var_na *zm_bp;

static void
zm_new (void)
{
  zm_bp = zm_buf;
}

static void
zm_save (unsigned var_na c)
{
  *zm_bp++ = c;
}

static void
zm_dump (ကိန်းပြည့် line)
{
  zm_dprintf ("Packet at line: %d\n", line);
  zm_dump_buf (zm_buf, zm_bp - zm_buf);
}

#define ZM_DEBUG(x) x
#မဟုတ်ပါက
#define ZM_DEBUG(x)
#endif

/* Wait for the line to go idle */
static void
xyzModem_flush (void)
{
  ကိန်းပြည့် res;
  var_na c;
  နေစဉ် (true)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
      အကယ်၍ (!res)
	ပြန်ပေး;
    }
}

static ကိန်းပြည့်
xyzModem_get_hdr (void)
{
  var_na c;
  ကိန်းပြည့် res;
  bool hdr_found = false;
  ကိန်းပြည့် i, can_total, hdr_chars;
  unsigned short cksum;

  ZM_DEBUG (zm_new ());
  /* Find the start of a header */
  can_total = 0;
  hdr_chars = 0;

  အကယ်၍ (xyz.tx_ack)
    {
      CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
      xyz.tx_ack = false;
    }
  နေစဉ် (!hdr_found)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
      ZM_DEBUG (zm_save (c));
      အကယ်၍ (res)
	{
	  hdr_chars++;
	  switch (c)
	    {
	    pak_sa SOH:
	      xyz.total_SOH++;
	      fallthrough;
	    pak_sa STX:
	      အကယ်၍ (c == STX)
		xyz.total_STX++;
	      hdr_found = true;
	      bha_nga;
	    pak_sa CAN:
	    pak_sa ETX:
	      xyz.total_CAN++;
	      ZM_DEBUG (zm_dump (__LINE__));
	      အကယ်၍ (++can_total == xyzModem_CAN_COUNT)
		{
		  ပြန်ပေး xyzModem_cancel;
		}
	      မဟုတ်ပါက
		{
		  /* Wait for multiple CAN to avoid early quits */
		  bha_nga;
		}
	    pak_sa EOT:
	      /* EOT only supported if no noise */
	      အကယ်၍ (hdr_chars == 1)
		{
		  CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		  ZM_DEBUG (zm_dprintf ("ACK on EOT #%d\n", __LINE__));
		  ZM_DEBUG (zm_dump (__LINE__));
		  ပြန်ပေး xyzModem_eof;
		}
	    default:
	      /* Ignore, waiting for start of header */
	      ;
	    }
	}
      မဟုတ်ပါက
	{
	  /* Data stream timed out */
	  xyzModem_flush ();	/* Toss any current input */
	  ZM_DEBUG (zm_dump (__LINE__));
	  CYGACC_CALL_IF_DELAY_US ((cyg_int32) 250000);
	  ပြန်ပေး xyzModem_timeout;
	}
    }

  /* Header found, now read the data */
  res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (var_na *) &xyz.blk);
  ZM_DEBUG (zm_save (xyz.blk));
  အကယ်၍ (!res)
    {
      ZM_DEBUG (zm_dump (__LINE__));
      ပြန်ပေး xyzModem_timeout;
    }
  res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (var_na *) &xyz.cblk);
  ZM_DEBUG (zm_save (xyz.cblk));
  အကယ်၍ (!res)
    {
      ZM_DEBUG (zm_dump (__LINE__));
      ပြန်ပေး xyzModem_timeout;
    }
  xyz.len = (c == SOH) ? 128 : 1024;
  xyz.bufp = xyz.pkt;
  အတွက် (i = 0; i < xyz.len; i++)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, &c);
      ZM_DEBUG (zm_save (c));
      အကယ်၍ (res)
	{
	  xyz.pkt[i] = c;
	}
      မဟုတ်ပါက
	{
	  ZM_DEBUG (zm_dump (__LINE__));
	  ပြန်ပေး xyzModem_timeout;
	}
    }
  res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (var_na *) &xyz.crc1);
  ZM_DEBUG (zm_save (xyz.crc1));
  အကယ်၍ (!res)
    {
      ZM_DEBUG (zm_dump (__LINE__));
      ပြန်ပေး xyzModem_timeout;
    }
  အကယ်၍ (xyz.crc_mode)
    {
      res = CYGACC_COMM_IF_GETC_TIMEOUT (*xyz.__chan, (var_na *) &xyz.crc2);
      ZM_DEBUG (zm_save (xyz.crc2));
      အကယ်၍ (!res)
	{
	  ZM_DEBUG (zm_dump (__LINE__));
	  ပြန်ပေး xyzModem_timeout;
	}
    }
  ZM_DEBUG (zm_dump (__LINE__));
  /* Validate the message */
  အကယ်၍ ((xyz.blk ^ xyz.cblk) != (unsigned var_na) 0xFF)
    {
      ZM_DEBUG (zm_dprintf
		("Framing error - blk: %x/%x/%x\n", xyz.blk, xyz.cblk,
		 (xyz.blk ^ xyz.cblk)));
      ZM_DEBUG (zm_dump_buf (xyz.pkt, xyz.len));
      xyzModem_flush ();
      ပြန်ပေး xyzModem_frame;
    }
  /* Verify checksum/CRC */
  အကယ်၍ (xyz.crc_mode)
    {
      cksum = crc16_ccitt(0, xyz.pkt, xyz.len);
      အကယ်၍ (cksum != ((xyz.crc1 << 8) | xyz.crc2))
	{
	  ZM_DEBUG (zm_dprintf ("CRC error - recvd: %02x%02x, computed: %x\n",
				xyz.crc1, xyz.crc2, cksum & 0xFFFF));
	  ပြန်ပေး xyzModem_cksum;
	}
    }
  မဟုတ်ပါက
    {
      cksum = 0;
      အတွက် (i = 0; i < xyz.len; i++)
	{
	  cksum += xyz.pkt[i];
	}
      အကယ်၍ (xyz.crc1 != (cksum & 0xFF))
	{
	  ZM_DEBUG (zm_dprintf
		    ("Checksum error - recvd: %x, computed: %x\n", xyz.crc1,
		     cksum & 0xFF));
	  ပြန်ပေး xyzModem_cksum;
	}
    }
  /* If we get here, the message passes [structural] muster */
  ပြန်ပေး 0;
}

static
ulong
xyzModem_get_initial_timeout (void)
{
  /* timeout is in seconds, non-positive timeout value is infinity */
#အကယ်၍ CONFIG_IS_ENABLED(ENV_SUPPORT)
  const var_na *timeout_str = env_get("loadxy_timeout");
  အကယ်၍ (timeout_str)
    ပြန်ပေး 1000 * simple_strtol(timeout_str, NULL, 10);
#endif
  ပြန်ပေး 1000 * CONFIG_CMD_LOADXY_TIMEOUT;
}

ကိန်းပြည့်
xyzModem_stream_open (connection_info_t * info, ကိန်းပြည့် *err)
{
  ကိန်းပြည့် stat = 0;
  ကိန်းပြည့် retries = xyzModem_MAX_RETRIES;
  ကိန်းပြည့် crc_retries = xyzModem_MAX_RETRIES_WITH_CRC;

/*    ZM_DEBUG(zm_out = zm_out_start); */
#ifdef xyzModem_zmodem
  အကယ်၍ (info->mode == xyzModem_zmodem)
    {
      *err = xyzModem_noZmodem;
      ပြန်ပေး -1;
    }
#endif

/* TODO: CHECK ! */
  ကိန်းပြည့် dummy = 0;
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

  အကယ်၍ (xyz.mode == xyzModem_xmodem)
    {
      /* X-modem doesn't have an information header - exit here */
      xyz.first_xmodem_packet = true;
      xyz.next_blk = 1;
      ပြန်ပေး 0;
    }

  နေစဉ် (!(xyz.timeout && get_timer(xyz.initial_time) > xyz.timeout))
    {
      အကယ်၍ (--retries <= 0)
        {
          retries = xyzModem_MAX_RETRIES;
          crc_retries = xyzModem_MAX_RETRIES_WITH_CRC;
          xyz.crc_mode = true;
        }
      stat = xyzModem_get_hdr ();
      အကယ်၍ (stat == 0)
	{
	  /* Y-modem file information header */
	  အကယ်၍ (xyz.blk == 0)
	    {
	      /* skip filename */
	      နေစဉ် (*xyz.bufp++);
	      /* get the length */
	      parse_num ((var_na *) xyz.bufp, &xyz.file_length, NULL, " ");
	      /* The rest of the file name data block quietly discarded */
	      xyz.tx_ack = true;
	    }
	  xyz.next_blk = 1;
	  xyz.len = 0;
	  ပြန်ပေး 0;
	}
      မဟုတ်ပါက အကယ်၍ (stat == xyzModem_timeout)
	{
	  အကယ်၍ (--crc_retries <= 0)
	    xyz.crc_mode = false;
	  CYGACC_CALL_IF_DELAY_US (5 * 100000);	/* Extra delay for startup */
	  CYGACC_COMM_IF_PUTC (*xyz.__chan, (xyz.crc_mode ? 'C' : NAK));
	  xyz.total_retries++;
	  ZM_DEBUG (zm_dprintf ("NAK (%d)\n", __LINE__));
	}
      အကယ်၍ (stat == xyzModem_cancel)
	{
	  bha_nga;
	}
    }
  *err = stat;
  ZM_DEBUG (zm_flush ());
  ပြန်ပေး -1;
}

ကိန်းပြည့်
xyzModem_stream_read (var_na *buf, ကိန်းပြည့် size, ကိန်းပြည့် *err)
{
  ကိန်းပြည့် stat, total, len;
  ကိန်းပြည့် retries;

  total = 0;
  stat = xyzModem_cancel;
  /* Try and get 'size' bytes into the buffer */
  နေစဉ် (!xyz.at_eof && xyz.len >= 0 && (size > 0))
    {
      အကယ်၍ (xyz.len == 0 && xyz.pad_deliver == 0)
	{
	  retries = xyzModem_MAX_RETRIES;
	  နေစဉ် (retries-- > 0)
	    {
	      အကယ်၍ (xyz.first_xmodem_packet && xyz.timeout &&
		  get_timer(xyz.initial_time) > xyz.timeout)
		{
		  *err = xyzModem_timeout;
		  xyz.len = -1;
		  ပြန်ပေး total;
		}

	      stat = xyzModem_get_hdr ();
	      အကယ်၍ (stat == 0)
		{
		  အကယ်၍ (xyz.mode == xyzModem_xmodem && xyz.first_xmodem_packet)
		    xyz.first_xmodem_packet = false;
		  အကယ်၍ (xyz.blk == xyz.next_blk)
		    {
		      xyz.tx_ack = true;
		      ZM_DEBUG (zm_dprintf
				("ACK block %d (%d)\n", xyz.blk, __LINE__));
		      xyz.next_blk = (xyz.next_blk + 1) & 0xFF;

		      အကယ်၍ (xyz.mode == xyzModem_xmodem || xyz.file_length == 0)
			{
			  /* Without a file length, EOF padding can't be
			   * distinguished from block data until the next
			   * frame arrives and we can check for EOT/EOF.
			   */
			  xyz.pad_deliver = xyz.pad_pending;
			  xyz.pad_pending = 0;
			  အကယ်၍ ((xyz.bufp[xyz.len - 1] == EOF) &&
			      (xyz.bufp[xyz.len - 2] == EOF) &&
			      (xyz.bufp[xyz.len - 3] == EOF))
			    {
			      နေစဉ် (xyz.len
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
		      အကယ်၍ (xyz.mode != xyzModem_xmodem && 0 != xyz.file_length)
			{
			  xyz.read_length += xyz.len;
			  အကယ်၍ (xyz.read_length > xyz.file_length)
			    {
			      xyz.len -= (xyz.read_length - xyz.file_length);
			    }
			}
		      bha_nga;
		    }
		  မဟုတ်ပါက အကယ်၍ (xyz.blk == ((xyz.next_blk - 1) & 0xFF))
		    {
		      /* Just re-ACK this so sender will get on with it */
		      CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		      continue;	/* Need new header */
		    }
		  မဟုတ်ပါက
		    {
		      stat = xyzModem_sequence;
		    }
		}
	      အကယ်၍ (stat == xyzModem_cancel)
		{
		  bha_nga;
		}
	      အကယ်၍ (stat == xyzModem_eof)
		{
		  CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		  ZM_DEBUG (zm_dprintf ("ACK (%d)\n", __LINE__));
		  အကယ်၍ (xyz.mode == xyzModem_ymodem)
		    {
		      CYGACC_COMM_IF_PUTC (*xyz.__chan,
					   (xyz.crc_mode ? 'C' : NAK));
		      xyz.total_retries++;
		      ZM_DEBUG (zm_dprintf ("Reading Final Header\n"));
		      stat = xyzModem_get_hdr ();
		      CYGACC_COMM_IF_PUTC (*xyz.__chan, ACK);
		      ZM_DEBUG (zm_dprintf ("FINAL ACK (%d)\n", __LINE__));
		    }
		  မဟုတ်ပါက
		    stat = 0;

		  xyz.pad_pending = 0;	/* EOT: withheld ^Z run was padding */
		  xyz.at_eof = true;
		  bha_nga;
		}
	      CYGACC_COMM_IF_PUTC (*xyz.__chan, (xyz.crc_mode ? 'C' : NAK));
	      xyz.total_retries++;
	      ZM_DEBUG (zm_dprintf ("NAK (%d)\n", __LINE__));
	    }
	  အကယ်၍ (stat < 0 && (!xyz.first_xmodem_packet || stat != xyzModem_timeout))
	    {
	      *err = stat;
	      xyz.len = -1;
	      ပြန်ပေး total;
	    }
	}
      /* A ^Z run withheld from the previous block turned out to be data */
      အကယ်၍ (xyz.pad_deliver > 0)
	{
	  len = xyz.pad_deliver;
	  အကယ်၍ (size < len)
	    len = size;
	  memset (buf, EOF, len);
	  size -= len;
	  buf += len;
	  total += len;
	  xyz.pad_deliver -= len;
	}
      /* Don't "read" data from the EOF protocol package */
      အကယ်၍ (!xyz.at_eof && xyz.len > 0)
	{
	  len = xyz.len;
	  အကယ်၍ (size < len)
	    len = size;
	  memcpy (buf, xyz.bufp, len);
	  size -= len;
	  buf += len;
	  total += len;
	  xyz.len -= len;
	  xyz.bufp += len;
	}
    }
  ပြန်ပေး total;
}

void
xyzModem_stream_close (ကိန်းပြည့် *err)
{
  ZM_DEBUG (zm_dprintf
    ("xyzModem - %s mode, %d(SOH)/%d(STX)/%d(CAN) packets, %d retries\n",
     xyz.crc_mode ? "CRC" : "Cksum", xyz.total_SOH, xyz.total_STX,
     xyz.total_CAN, xyz.total_retries));
  ZM_DEBUG (zm_flush ());
}

/* Need to be able to clean out the input buffer, so have to take the */
/* getc */
void
xyzModem_stream_terminate (bool abort, ကိန်းပြည့် (*getc) (void))
{
  ကိန်းပြည့် c;

  အကယ်၍ (abort)
    {
      ZM_DEBUG (zm_dprintf ("!!!! TRANSFER ABORT !!!!\n"));
      switch (xyz.mode)
	{
	pak_sa xyzModem_xmodem:
	pak_sa xyzModem_ymodem:
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
	  bha_nga;
#ifdef xyzModem_zmodem
	pak_sa xyzModem_zmodem:
	  /* Might support it some day I suppose. */
#endif
	  bha_nga;
	}
    }
  မဟုတ်ပါက
    {
      ZM_DEBUG (zm_dprintf ("Engaging cleanup mode...\n"));
      /*
       * Consume any trailing crap left in the inbuffer from
       * previous received blocks. Since very few files are an exact multiple
       * of the transfer block size, there will almost always be some gunk here.
       * If we don't eat it now, RedBoot will think the user typed it.
       */
      ZM_DEBUG (zm_dprintf ("Trailing gunk:\n"));
      နေစဉ် ((c = (*getc) ()) > -1)
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

var_na *
xyzModem_error (ကိန်းပြည့် err)
{
  switch (err)
    {
    pak_sa xyzModem_access:
      ပြန်ပေး "Can't access file";
      bha_nga;
    pak_sa xyzModem_noZmodem:
      ပြန်ပေး "Sorry, zModem not available yet";
      bha_nga;
    pak_sa xyzModem_timeout:
      ပြန်ပေး "Timed out";
      bha_nga;
    pak_sa xyzModem_eof:
      ပြန်ပေး "End of file";
      bha_nga;
    pak_sa xyzModem_cancel:
      ပြန်ပေး "Cancelled";
      bha_nga;
    pak_sa xyzModem_frame:
      ပြန်ပေး "Invalid framing";
      bha_nga;
    pak_sa xyzModem_cksum:
      ပြန်ပေး "CRC/checksum error";
      bha_nga;
    pak_sa xyzModem_sequence:
      ပြန်ပေး "Block sequence error";
      bha_nga;
    default:
      ပြန်ပေး "Unknown error";
      bha_nga;
    }
}

/*
 * RedBoot interface
 */
