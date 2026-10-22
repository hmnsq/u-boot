// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/* #define DEBUG */

#ਅਨ੍ਤਰ੍ਭਾਵ <flash.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <log.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <u-boot/uuid.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <linux/string.h>

#ਅਨ੍ਤਰ੍ਭਾਵ <mtd/cfi_flash.h>

/*-----------------------------------------------------------------------
 * Functions
 */

/*-----------------------------------------------------------------------
 * Set protection status for monitor sectors
 *
 * The monitor is always located in the _first_ Flash bank.
 * If necessary you have to map the second bank at lower addresses.
 */
ਖਾਲੀ
flash_protect(ਪੂਰਨ flag, ulong from, ulong ਮੰਜ਼ਿਲ, flash_info_t *info)
{
	ulong b_ਸਿਰਾ;
	ਛੋਟਾ s_ਸਿਰਾ;
	ਪੂਰਨ i;

	/* Do nothing if input data is bad. */
	ਜੇ (!info || info->sector_ਗਿਣਤੀ == 0 || info->ਮਾਪ == 0 || ਮੰਜ਼ਿਲ < from) {
		ਵਾਪਸ;
	}

	s_ਸਿਰਾ = info->sector_ਗਿਣਤੀ - 1;	/* index of last sector */
	b_ਸਿਰਾ = info->ਸ਼ੁਰੂ[0] + info->ਮਾਪ - 1;	/* bank end address */

	debug("%s %s: from 0x%08lX to 0x%08lX\n", __func__,
	      (flag & FLAG_PROTECT_ਸੈੱਟ) ? "ON" :
	      (flag & FLAG_PROTECT_CLEAR) ? "OFF" : "???",
	      from, ਮੰਜ਼ਿਲ);

	/* There is nothing to do if we have no data about the flash
	 * or the protect range and flash range don't overlap.
	 */
	ਜੇ (info->flash_id == FLASH_UNKNOWN ||
	    ਮੰਜ਼ਿਲ < info->ਸ਼ੁਰੂ[0] || from > b_ਸਿਰਾ) {
		ਵਾਪਸ;
	}

	ਲਈ (i=0; i<info->sector_ਗਿਣਤੀ; ++i) {
		ulong ਸਿਰਾ_2;		/* last address in current sect	*/

		ਸਿਰਾ_2 = (i == s_ਸਿਰਾ) ? b_ਸਿਰਾ : info->ਸ਼ੁਰੂ[i + 1] - 1;

		/* Update protection if any part of the sector
		 * is in the specified range.
		 */
		ਜੇ (from <= ਸਿਰਾ_2 && ਮੰਜ਼ਿਲ >= info->ਸ਼ੁਰੂ[i]) {
			ਜੇ (flag & FLAG_PROTECT_CLEAR) {
#ਜੇ defined(CONFIG_SYS_FLASH_PROTECTION)
				flash_real_protect(info, i, 0);
#ਨਹੀਂਤਾਂ
				info->protect[i] = 0;
#ਜੇ_ਅੰਤ	/* CONFIG_SYS_FLASH_PROTECTION */
				debug("protect off %d\n", i);
			}
			ਨਹੀਂਤਾਂ ਜੇ (flag & FLAG_PROTECT_ਸੈੱਟ) {
#ਜੇ defined(CONFIG_SYS_FLASH_PROTECTION)
				flash_real_protect(info, i, 1);
#ਨਹੀਂਤਾਂ
				info->protect[i] = 1;
#ਜੇ_ਅੰਤ	/* CONFIG_SYS_FLASH_PROTECTION */
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
	ਪੂਰਨ i;

	ਲਈ (i = 0, info = &flash_info[0]; i < CFI_FLASH_BANKS; ++i, ++info) {
		ਜੇ (info->flash_id != FLASH_UNKNOWN &&
		    addr >= info->ਸ਼ੁਰੂ[0] &&
		    /* WARNING - The '- 1' is needed if the flash
		     * is at the end of the address space, since
		     * info->start[0] + info->size wraps back to 0.
		     * Please don't change this unless you understand this.
		     */
		    addr <= info->ਸ਼ੁਰੂ[0] + info->ਮਾਪ - 1) {
			ਵਾਪਸ (info);
		}
	}

	ਵਾਪਸ (ਨਿਰਾ);
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
ਪੂਰਨ
flash_write(ਅੱਖਰ *src, ulong addr, ulong cnt)
{
	ਪੂਰਨ i;
	ulong         ਸਿਰਾ_2        = addr + cnt - 1;
	flash_info_t *info_ਪਹਿਲਾ = addr2info(addr);
	flash_info_t *info_ਆਖਰੀ  = addr2info(ਸਿਰਾ_2);
	flash_info_t *info;
	__maybe_unused ਅੱਖਰ *src_orig = src;
	__maybe_unused ਅੱਖਰ *addr_orig = (ਅੱਖਰ *)addr;
	__maybe_unused ulong cnt_orig = cnt;

	ਜੇ (cnt == 0) {
		ਵਾਪਸ (FL_ERR_OK);
	}

	ਜੇ (!info_ਪਹਿਲਾ || !info_ਆਖਰੀ) {
		ਵਾਪਸ (FL_ERR_INVAL);
	}

	ਲਈ (info = info_ਪਹਿਲਾ; info <= info_ਆਖਰੀ; ++info) {
		ulong b_ਸਿਰਾ = info->ਸ਼ੁਰੂ[0] + info->ਮਾਪ;	/* bank end addr */
		ਛੋਟਾ s_ਸਿਰਾ = info->sector_ਗਿਣਤੀ - 1;
		ਲਈ (i=0; i<info->sector_ਗਿਣਤੀ; ++i) {
			ulong e_addr = (i == s_ਸਿਰਾ) ? b_ਸਿਰਾ : info->ਸ਼ੁਰੂ[i + 1];

			ਜੇ ((ਸਿਰਾ_2 >= info->ਸ਼ੁਰੂ[i]) && (addr < e_addr) &&
			    (info->protect[i] != 0) ) {
				ਵਾਪਸ (FL_ERR_PROTECTED);
			}
		}
	}

	/* finally write data to flash */
	ਲਈ (info = info_ਪਹਿਲਾ; info <= info_ਆਖਰੀ && cnt>0; ++info) {
		ulong len;

		len = info->ਸ਼ੁਰੂ[0] + info->ਮਾਪ - addr;
		ਜੇ (len > cnt)
			len = cnt;
		ਜੇ ((i = write_buff(info, (uchar *)src, addr, len)) != 0) {
			ਵਾਪਸ (i);
		}
		cnt  -= len;
		addr += len;
		src  += len;
	}

#ਜੇ defined(CONFIG_FLASH_VERIFY)
	ਜੇ (memcmp(src_orig, addr_orig, cnt_orig)) {
		ਲਿਖੋ("\nVerify failed!\n");
		ਵਾਪਸ FL_ERR_PROG_ERROR;
	}
#ਜੇ_ਅੰਤ /* CONFIG_SYS_FLASH_VERIFY_AFTER_WRITE */

	ਵਾਪਸ (FL_ERR_OK);
}

/*-----------------------------------------------------------------------
 */

ਖਾਲੀ flash_perror(ਪੂਰਨ err)
{
	ਚੋਣ (err) {
	ਹਾਲਤ FL_ERR_OK:
		ਰੋਕੋ;
	ਹਾਲਤ FL_ERR_TIMEOUT:
		puts ("Timeout writing to Flash\n");
		ਰੋਕੋ;
	ਹਾਲਤ FL_ERR_NOT_ERASED:
		puts ("Flash not Erased\n");
		ਰੋਕੋ;
	ਹਾਲਤ FL_ERR_PROTECTED:
		puts ("Can't write to protected Flash sectors\n");
		ਰੋਕੋ;
	ਹਾਲਤ FL_ERR_INVAL:
		puts ("Outside available Flash\n");
		ਰੋਕੋ;
	ਹਾਲਤ FL_ERR_ALIGN:
		puts ("Start and/or end address not on sector boundary\n");
		ਰੋਕੋ;
	ਹਾਲਤ FL_ERR_UNKNOWN_FLASH_VENDOR:
		puts ("Unknown Vendor of Flash\n");
		ਰੋਕੋ;
	ਹਾਲਤ FL_ERR_UNKNOWN_FLASH_TYPE:
		puts ("Unknown Type of Flash\n");
		ਰੋਕੋ;
	ਹਾਲਤ FL_ERR_PROG_ERROR:
		puts ("General Flash Programming Error\n");
		ਰੋਕੋ;
	ਹਾਲਤ FL_ERR_ABORTED:
		puts("Flash Programming Aborted\n");
		ਰੋਕੋ;
	ਮੂਲ:
		ਲਿਖੋ ("%s[%d] FIXME: rc=%d\n", __FILE__, __LINE__, err);
		ਰੋਕੋ;
	}
}
