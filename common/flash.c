// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/* #define DEBUG */

#ಅನ್ತರ್ಭಾವ <flash.h>
#ಅನ್ತರ್ಭಾವ <log.h>
#ಅನ್ತರ್ಭಾವ <u-boot/uuid.h>
#ಅನ್ತರ್ಭಾವ <linux/string.h>

#ಅನ್ತರ್ಭಾವ <mtd/cfi_flash.h>

/*-----------------------------------------------------------------------
 * Functions
 */

/*-----------------------------------------------------------------------
 * Set protection status for monitor sectors
 *
 * The monitor is always located in the _first_ Flash bank.
 * If necessary you have to map the second bank at lower addresses.
 */
ಶೂನ್ಯ
flash_protect(ಪೂರ್ಣಾಙ್ಕ flag, ulong from, ulong to, flash_info_t *info)
{
	ulong b_end;
	short s_end;
	ಪೂರ್ಣಾಙ್ಕ i;

	/* Do nothing if input data is bad. */
	ಯದಿ (!info || info->sector_ಎಣಿಕೆ == 0 || info->size == 0 || to < from) {
		ಪ್ರತ್ಯಾಗಮ;
	}

	s_end = info->sector_ಎಣಿಕೆ - 1;	/* index of last sector */
	b_end = info->start[0] + info->size - 1;	/* bank end address */

	debug("%s %s: from 0x%08lX to 0x%08lX\n", __func__,
	      (flag & FLAG_PROTECT_SET) ? "ON" :
	      (flag & FLAG_PROTECT_CLEAR) ? "OFF" : "???",
	      from, to);

	/* There is nothing to do if we have no data about the flash
	 * or the protect range and flash range don't overlap.
	 */
	ಯದಿ (info->flash_id == FLASH_UNKNOWN ||
	    to < info->start[0] || from > b_end) {
		ಪ್ರತ್ಯಾಗಮ;
	}

	ಪ್ರತಿ (i=0; i<info->sector_ಎಣಿಕೆ; ++i) {
		ulong end;		/* last address in current sect	*/

		end = (i == s_end) ? b_end : info->start[i + 1] - 1;

		/* Update protection if any part of the sector
		 * is in the specified range.
		 */
		ಯದಿ (from <= end && to >= info->start[i]) {
			ಯದಿ (flag & FLAG_PROTECT_CLEAR) {
#ಯದಿ defined(CONFIG_SYS_FLASH_PROTECTION)
				flash_real_protect(info, i, 0);
#ಅನ್ಯಥಾ
				info->protect[i] = 0;
#endif	/* CONFIG_SYS_FLASH_PROTECTION */
				debug("protect off %d\n", i);
			}
			ಅನ್ಯಥಾ ಯದಿ (flag & FLAG_PROTECT_SET) {
#ಯದಿ defined(CONFIG_SYS_FLASH_PROTECTION)
				flash_real_protect(info, i, 1);
#ಅನ್ಯಥಾ
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
	ಪೂರ್ಣಾಙ್ಕ i;

	ಪ್ರತಿ (i = 0, info = &flash_info[0]; i < CFI_FLASH_BANKS; ++i, ++info) {
		ಯದಿ (info->flash_id != FLASH_UNKNOWN &&
		    addr >= info->start[0] &&
		    /* WARNING - The '- 1' is needed if the flash
		     * is at the end of the address space, since
		     * info->start[0] + info->size wraps back to 0.
		     * Please don't change this unless you understand this.
		     */
		    addr <= info->start[0] + info->size - 1) {
			ಪ್ರತ್ಯಾಗಮ (info);
		}
	}

	ಪ್ರತ್ಯಾಗಮ (ರಿಕ್ತ);
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
ಪೂರ್ಣಾಙ್ಕ
flash_write(ವರ್ಣ *src, ulong addr, ulong cnt)
{
	ಪೂರ್ಣಾಙ್ಕ i;
	ulong         end        = addr + cnt - 1;
	flash_info_t *info_first = addr2info(addr);
	flash_info_t *info_last  = addr2info(end);
	flash_info_t *info;
	__maybe_unused ವರ್ಣ *src_orig = src;
	__maybe_unused ವರ್ಣ *addr_orig = (ವರ್ಣ *)addr;
	__maybe_unused ulong cnt_orig = cnt;

	ಯದಿ (cnt == 0) {
		ಪ್ರತ್ಯಾಗಮ (FL_ERR_OK);
	}

	ಯದಿ (!info_first || !info_last) {
		ಪ್ರತ್ಯಾಗಮ (FL_ERR_INVAL);
	}

	ಪ್ರತಿ (info = info_first; info <= info_last; ++info) {
		ulong b_end = info->start[0] + info->size;	/* bank end addr */
		short s_end = info->sector_ಎಣಿಕೆ - 1;
		ಪ್ರತಿ (i=0; i<info->sector_ಎಣಿಕೆ; ++i) {
			ulong e_addr = (i == s_end) ? b_end : info->start[i + 1];

			ಯದಿ ((end >= info->start[i]) && (addr < e_addr) &&
			    (info->protect[i] != 0) ) {
				ಪ್ರತ್ಯಾಗಮ (FL_ERR_PROTECTED);
			}
		}
	}

	/* finally write data to flash */
	ಪ್ರತಿ (info = info_first; info <= info_last && cnt>0; ++info) {
		ulong len;

		len = info->start[0] + info->size - addr;
		ಯದಿ (len > cnt)
			len = cnt;
		ಯದಿ ((i = write_buff(info, (uchar *)src, addr, len)) != 0) {
			ಪ್ರತ್ಯಾಗಮ (i);
		}
		cnt  -= len;
		addr += len;
		src  += len;
	}

#ಯದಿ defined(CONFIG_FLASH_VERIFY)
	ಯದಿ (memcmp(src_orig, addr_orig, cnt_orig)) {
		printf("\nVerify failed!\n");
		ಪ್ರತ್ಯಾಗಮ FL_ERR_PROG_ERROR;
	}
#endif /* CONFIG_SYS_FLASH_VERIFY_AFTER_WRITE */

	ಪ್ರತ್ಯಾಗಮ (FL_ERR_OK);
}

/*-----------------------------------------------------------------------
 */

ಶೂನ್ಯ flash_perror(ಪೂರ್ಣಾಙ್ಕ err)
{
	ವಿಕಲ್ಪ (err) {
	ಪಕ್ಷ FL_ERR_OK:
		ಭಙ್ಗ;
	ಪಕ್ಷ FL_ERR_TIMEOUT:
		puts ("Timeout writing to Flash\n");
		ಭಙ್ಗ;
	ಪಕ್ಷ FL_ERR_NOT_ERASED:
		puts ("Flash not Erased\n");
		ಭಙ್ಗ;
	ಪಕ್ಷ FL_ERR_PROTECTED:
		puts ("Can't write to protected Flash sectors\n");
		ಭಙ್ಗ;
	ಪಕ್ಷ FL_ERR_INVAL:
		puts ("Outside available Flash\n");
		ಭಙ್ಗ;
	ಪಕ್ಷ FL_ERR_ALIGN:
		puts ("Start and/or end address not on sector boundary\n");
		ಭಙ್ಗ;
	ಪಕ್ಷ FL_ERR_UNKNOWN_FLASH_VENDOR:
		puts ("Unknown Vendor of Flash\n");
		ಭಙ್ಗ;
	ಪಕ್ಷ FL_ERR_UNKNOWN_FLASH_TYPE:
		puts ("Unknown Type of Flash\n");
		ಭಙ್ಗ;
	ಪಕ್ಷ FL_ERR_PROG_ERROR:
		puts ("General Flash Programming Error\n");
		ಭಙ್ಗ;
	ಪಕ್ಷ FL_ERR_ABORTED:
		puts("Flash Programming Aborted\n");
		ಭಙ್ಗ;
	ಸ್ವಭಾವ:
		printf ("%s[%d] FIXME: rc=%d\n", __FILE__, __LINE__, err);
		ಭಙ್ಗ;
	}
}
