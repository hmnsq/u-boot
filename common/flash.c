// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/* #define DEBUG */

#અન્તર્ભાવ <flash.h>
#અન્તર્ભાવ <log.h>
#અન્તર્ભાવ <u-boot/uuid.h>
#અન્તર્ભાવ <linux/string.h>

#અન્તર્ભાવ <mtd/cfi_flash.h>

/*-----------------------------------------------------------------------
 * Functions
 */

/*-----------------------------------------------------------------------
 * Set protection status for monitor sectors
 *
 * The monitor is always located in the _first_ Flash bank.
 * If necessary you have to map the second bank at lower addresses.
 */
શૂન્ય
flash_protect(પૂર્ણાંક flag, ulong from, ulong to, flash_info_t *info)
{
	ulong b_end;
	short s_end;
	પૂર્ણાંક i;

	/* Do nothing if input data is bad. */
	જો (!info || info->sector_ગણતરી == 0 || info->size == 0 || to < from) {
		પાછા;
	}

	s_end = info->sector_ગણતરી - 1;	/* index of last sector */
	b_end = info->start[0] + info->size - 1;	/* bank end address */

	debug("%s %s: from 0x%08lX to 0x%08lX\n", __func__,
	      (flag & FLAG_PROTECT_SET) ? "ON" :
	      (flag & FLAG_PROTECT_CLEAR) ? "OFF" : "???",
	      from, to);

	/* There is nothing to do if we have no data about the flash
	 * or the protect range and flash range don't overlap.
	 */
	જો (info->flash_id == FLASH_UNKNOWN ||
	    to < info->start[0] || from > b_end) {
		પાછા;
	}

	માટે (i=0; i<info->sector_ગણતરી; ++i) {
		ulong end;		/* last address in current sect	*/

		end = (i == s_end) ? b_end : info->start[i + 1] - 1;

		/* Update protection if any part of the sector
		 * is in the specified range.
		 */
		જો (from <= end && to >= info->start[i]) {
			જો (flag & FLAG_PROTECT_CLEAR) {
#જો defined(CONFIG_SYS_FLASH_PROTECTION)
				flash_real_protect(info, i, 0);
#નહીંતર
				info->protect[i] = 0;
#endif	/* CONFIG_SYS_FLASH_PROTECTION */
				debug("protect off %d\n", i);
			}
			નહીંતર જો (flag & FLAG_PROTECT_SET) {
#જો defined(CONFIG_SYS_FLASH_PROTECTION)
				flash_real_protect(info, i, 1);
#નહીંતર
				info->protect[i] = 1;
#endif	/* CONFIG_SYS_FLASH_PROTECTION */
				debug("protect on %d\n", i);
			}
		}
	}
}

/*-----------------------------------------------------------------------
 */

flash_info_t *
addr2info(ulong addr)
{
	flash_info_t *info;
	પૂર્ણાંક i;

	માટે (i = 0, info = &flash_info[0]; i < CFI_FLASH_BANKS; ++i, ++info) {
		જો (info->flash_id != FLASH_UNKNOWN &&
		    addr >= info->start[0] &&
		    /* WARNING - The '- 1' is needed if the flash
		     * is at the end of the address space, since
		     * info->start[0] + info->size wraps back to 0.
		     * Please don't change this unless you understand this.
		     */
		    addr <= info->start[0] + info->size - 1) {
			પાછા (info);
		}
	}

	પાછા (ખાલી);
}

/*-----------------------------------------------------------------------
 * Copy memory to flash.
 * Make sure all target addresses are within Flash bounds,
 * and no protected sectors are hit.
 * Returns:
 * FL_ERR_OK          0 - OK
 * FL_ERR_TIMEOUT     1 - write timeout
 * FL_ERR_NOT_ERASED  2 - Flash not erased
 * FL_ERR_PROTECTED   4 - target range includes protected sectors
 * FL_ERR_INVAL       8 - target address not in Flash memory
 * FL_ERR_ALIGN       16 - target address not aligned on boundary
 *			   (only some targets require alignment)
 */
પૂર્ણાંક
flash_write(અક્ષર *src, ulong addr, ulong cnt)
{
	પૂર્ણાંક i;
	ulong         end        = addr + cnt - 1;
	flash_info_t *info_first = addr2info(addr);
	flash_info_t *info_last  = addr2info(end);
	flash_info_t *info;
	__maybe_unused અક્ષર *src_orig = src;
	__maybe_unused અક્ષર *addr_orig = (અક્ષર *)addr;
	__maybe_unused ulong cnt_orig = cnt;

	જો (cnt == 0) {
		પાછા (FL_ERR_OK);
	}

	જો (!info_first || !info_last) {
		પાછા (FL_ERR_INVAL);
	}

	માટે (info = info_first; info <= info_last; ++info) {
		ulong b_end = info->start[0] + info->size;	/* bank end addr */
		short s_end = info->sector_ગણતરી - 1;
		માટે (i=0; i<info->sector_ગણતરી; ++i) {
			ulong e_addr = (i == s_end) ? b_end : info->start[i + 1];

			જો ((end >= info->start[i]) && (addr < e_addr) &&
			    (info->protect[i] != 0) ) {
				પાછા (FL_ERR_PROTECTED);
			}
		}
	}

	/* finally write data to flash */
	માટે (info = info_first; info <= info_last && cnt>0; ++info) {
		ulong len;

		len = info->start[0] + info->size - addr;
		જો (len > cnt)
			len = cnt;
		જો ((i = write_buff(info, (uchar *)src, addr, len)) != 0) {
			પાછા (i);
		}
		cnt  -= len;
		addr += len;
		src  += len;
	}

#જો defined(CONFIG_FLASH_VERIFY)
	જો (memcmp(src_orig, addr_orig, cnt_orig)) {
		printf("\nVerify failed!\n");
		પાછા FL_ERR_PROG_ERROR;
	}
#endif /* CONFIG_SYS_FLASH_VERIFY_AFTER_WRITE */

	પાછા (FL_ERR_OK);
}

/*-----------------------------------------------------------------------
 */

શૂન્ય flash_perror(પૂર્ણાંક err)
{
	પસંદગી (err) {
	સ્થિતિ FL_ERR_OK:
		તોડો;
	સ્થિતિ FL_ERR_TIMEOUT:
		puts ("Timeout writing to Flash\n");
		તોડો;
	સ્થિતિ FL_ERR_NOT_ERASED:
		puts ("Flash not Erased\n");
		તોડો;
	સ્થિતિ FL_ERR_PROTECTED:
		puts ("Can't write to protected Flash sectors\n");
		તોડો;
	સ્થિતિ FL_ERR_INVAL:
		puts ("Outside available Flash\n");
		તોડો;
	સ્થિતિ FL_ERR_ALIGN:
		puts ("Start and/or end address not on sector boundary\n");
		તોડો;
	સ્થિતિ FL_ERR_UNKNOWN_FLASH_VENDOR:
		puts ("Unknown Vendor of Flash\n");
		તોડો;
	સ્થિતિ FL_ERR_UNKNOWN_FLASH_TYPE:
		puts ("Unknown Type of Flash\n");
		તોડો;
	સ્થિતિ FL_ERR_PROG_ERROR:
		puts ("General Flash Programming Error\n");
		તોડો;
	સ્થિતિ FL_ERR_ABORTED:
		puts("Flash Programming Aborted\n");
		તોડો;
	મૂળભૂત:
		printf ("%s[%d] FIXME: rc=%d\n", __FILE__, __LINE__, err);
		તોડો;
	}
}
