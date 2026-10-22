// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/* #define DEBUG */

#అన్తర్భావ <flash.h>
#అన్తర్భావ <log.h>
#అన్తర్భావ <u-boot/uuid.h>
#అన్తర్భావ <linux/string.h>

#అన్తర్భావ <mtd/cfi_flash.h>

/*-----------------------------------------------------------------------
 * Functions
 */

/*-----------------------------------------------------------------------
 * Set protection status for monitor sectors
 *
 * The monitor is always located in the _first_ Flash bank.
 * If necessary you have to map the second bank at lower addresses.
 */
శూన్య
flash_protect(పూర్ణాంకం flag, ulong from, ulong గమ్యం, flash_info_t *info)
{
	ulong b_చివర;
	పొట్టి s_చివర;
	పూర్ణాంకం i;

	/* Do nothing if input data is bad. */
	ఒకవేళ (!info || info->sector_లెక్క == 0 || info->కొలత == 0 || గమ్యం < from) {
		తిరిగి;
	}

	s_చివర = info->sector_లెక్క - 1;	/* index of last sector */
	b_చివర = info->ప్రారంభం[0] + info->కొలత - 1;	/* bank end address */

	debug("%s %s: from 0x%08lX to 0x%08lX\n", __func__,
	      (flag & FLAG_PROTECT_సమితి) ? "ON" :
	      (flag & FLAG_PROTECT_CLEAR) ? "OFF" : "???",
	      from, గమ్యం);

	/* There is nothing to do if we have no data about the flash
	 * or the protect range and flash range don't overlap.
	 */
	ఒకవేళ (info->flash_id == FLASH_UNKNOWN ||
	    గమ్యం < info->ప్రారంభం[0] || from > b_చివర) {
		తిరిగి;
	}

	కొరకు (i=0; i<info->sector_లెక్క; ++i) {
		ulong చివర_2;		/* last address in current sect	*/

		చివర_2 = (i == s_చివర) ? b_చివర : info->ప్రారంభం[i + 1] - 1;

		/* Update protection if any part of the sector
		 * is in the specified range.
		 */
		ఒకవేళ (from <= చివర_2 && గమ్యం >= info->ప్రారంభం[i]) {
			ఒకవేళ (flag & FLAG_PROTECT_CLEAR) {
#ఒకవేళ defined(CONFIG_SYS_FLASH_PROTECTION)
				flash_real_protect(info, i, 0);
#లేకపోతే
				info->protect[i] = 0;
#అయితే_ముగింపు	/* CONFIG_SYS_FLASH_PROTECTION */
				debug("protect off %d\n", i);
			}
			లేకపోతే ఒకవేళ (flag & FLAG_PROTECT_సమితి) {
#ఒకవేళ defined(CONFIG_SYS_FLASH_PROTECTION)
				flash_real_protect(info, i, 1);
#లేకపోతే
				info->protect[i] = 1;
#అయితే_ముగింపు	/* CONFIG_SYS_FLASH_PROTECTION */
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
	పూర్ణాంకం i;

	కొరకు (i = 0, info = &flash_info[0]; i < CFI_FLASH_BANKS; ++i, ++info) {
		ఒకవేళ (info->flash_id != FLASH_UNKNOWN &&
		    addr >= info->ప్రారంభం[0] &&
		    /* WARNING - The '- 1' is needed if the flash
		     * is at the end of the address space, since
		     * info->start[0] + info->size wraps back to 0.
		     * Please don't change this unless you understand this.
		     */
		    addr <= info->ప్రారంభం[0] + info->కొలత - 1) {
			తిరిగి (info);
		}
	}

	తిరిగి (ఖాళీ);
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
పూర్ణాంకం
flash_write(అక్షరం *src, ulong addr, ulong cnt)
{
	పూర్ణాంకం i;
	ulong         చివర_2        = addr + cnt - 1;
	flash_info_t *info_మొదటి = addr2info(addr);
	flash_info_t *info_చివరి  = addr2info(చివర_2);
	flash_info_t *info;
	__maybe_unused అక్షరం *src_orig = src;
	__maybe_unused అక్షరం *addr_orig = (అక్షరం *)addr;
	__maybe_unused ulong cnt_orig = cnt;

	ఒకవేళ (cnt == 0) {
		తిరిగి (FL_ERR_OK);
	}

	ఒకవేళ (!info_మొదటి || !info_చివరి) {
		తిరిగి (FL_ERR_INVAL);
	}

	కొరకు (info = info_మొదటి; info <= info_చివరి; ++info) {
		ulong b_చివర = info->ప్రారంభం[0] + info->కొలత;	/* bank end addr */
		పొట్టి s_చివర = info->sector_లెక్క - 1;
		కొరకు (i=0; i<info->sector_లెక్క; ++i) {
			ulong e_addr = (i == s_చివర) ? b_చివర : info->ప్రారంభం[i + 1];

			ఒకవేళ ((చివర_2 >= info->ప్రారంభం[i]) && (addr < e_addr) &&
			    (info->protect[i] != 0) ) {
				తిరిగి (FL_ERR_PROTECTED);
			}
		}
	}

	/* finally write data to flash */
	కొరకు (info = info_మొదటి; info <= info_చివరి && cnt>0; ++info) {
		ulong len;

		len = info->ప్రారంభం[0] + info->కొలత - addr;
		ఒకవేళ (len > cnt)
			len = cnt;
		ఒకవేళ ((i = write_buff(info, (uchar *)src, addr, len)) != 0) {
			తిరిగి (i);
		}
		cnt  -= len;
		addr += len;
		src  += len;
	}

#ఒకవేళ defined(CONFIG_FLASH_VERIFY)
	ఒకవేళ (memcmp(src_orig, addr_orig, cnt_orig)) {
		రాయి("\nVerify failed!\n");
		తిరిగి FL_ERR_PROG_ERROR;
	}
#అయితే_ముగింపు /* CONFIG_SYS_FLASH_VERIFY_AFTER_WRITE */

	తిరిగి (FL_ERR_OK);
}

/*-----------------------------------------------------------------------
 */

శూన్య flash_perror(పూర్ణాంకం err)
{
	ఎంపిక (err) {
	స్థితి FL_ERR_OK:
		ఆపు;
	స్థితి FL_ERR_TIMEOUT:
		puts ("Timeout writing to Flash\n");
		ఆపు;
	స్థితి FL_ERR_NOT_ERASED:
		puts ("Flash not Erased\n");
		ఆపు;
	స్థితి FL_ERR_PROTECTED:
		puts ("Can't write to protected Flash sectors\n");
		ఆపు;
	స్థితి FL_ERR_INVAL:
		puts ("Outside available Flash\n");
		ఆపు;
	స్థితి FL_ERR_ALIGN:
		puts ("Start and/or end address not on sector boundary\n");
		ఆపు;
	స్థితి FL_ERR_UNKNOWN_FLASH_VENDOR:
		puts ("Unknown Vendor of Flash\n");
		ఆపు;
	స్థితి FL_ERR_UNKNOWN_FLASH_TYPE:
		puts ("Unknown Type of Flash\n");
		ఆపు;
	స్థితి FL_ERR_PROG_ERROR:
		puts ("General Flash Programming Error\n");
		ఆపు;
	స్థితి FL_ERR_ABORTED:
		puts("Flash Programming Aborted\n");
		ఆపు;
	అప్రమేయ:
		రాయి ("%s[%d] FIXME: rc=%d\n", __FILE__, __LINE__, err);
		ఆపు;
	}
}
