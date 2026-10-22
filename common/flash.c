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
flash_protect(ಪೂರ್ಣಾಂಕ flag, ulong from, ulong ಗಮ್ಯ, flash_info_t *info)
{
	ulong b_ಕೊನೆ;
	ಗಿಡ್ಡ s_ಕೊನೆ;
	ಪೂರ್ಣಾಂಕ i;

	/* Do nothing if input data is bad. */
	ಆದರೆ (!info || info->sector_ಎಣಿಕೆ == 0 || info->ಗಾತ್ರ_2 == 0 || ಗಮ್ಯ < from) {
		ಹಿಂತಿರುಗಿ;
	}

	s_ಕೊನೆ = info->sector_ಎಣಿಕೆ - 1;	/* index of last sector */
	b_ಕೊನೆ = info->ಆರಂಭ[0] + info->ಗಾತ್ರ_2 - 1;	/* bank end address */

	debug("%s %s: from 0x%08lX to 0x%08lX\n", __func__,
	      (flag & FLAG_PROTECT_ಗಣ) ? "ON" :
	      (flag & FLAG_PROTECT_CLEAR) ? "OFF" : "???",
	      from, ಗಮ್ಯ);

	/* There is nothing to do if we have no data about the flash
	 * or the protect range and flash range don't overlap.
	 */
	ಆದರೆ (info->flash_id == FLASH_UNKNOWN ||
	    ಗಮ್ಯ < info->ಆರಂಭ[0] || from > b_ಕೊನೆ) {
		ಹಿಂತಿರುಗಿ;
	}

	ಗಾಗಿ (i=0; i<info->sector_ಎಣಿಕೆ; ++i) {
		ulong ಕೊನೆ_2;		/* last address in current sect	*/

		ಕೊನೆ_2 = (i == s_ಕೊನೆ) ? b_ಕೊನೆ : info->ಆರಂಭ[i + 1] - 1;

		/* Update protection if any part of the sector
		 * is in the specified range.
		 */
		ಆದರೆ (from <= ಕೊನೆ_2 && ಗಮ್ಯ >= info->ಆರಂಭ[i]) {
			ಆದರೆ (flag & FLAG_PROTECT_CLEAR) {
#ಆದರೆ defined(CONFIG_SYS_FLASH_PROTECTION)
				flash_real_protect(info, i, 0);
#ಇಲ್ಲದಿದ್ದರೆ
				info->protect[i] = 0;
#ಆದರೆ_ಅಂತ್ಯ	/* CONFIG_SYS_FLASH_PROTECTION */
				debug("protect off %d\n", i);
			}
			ಇಲ್ಲದಿದ್ದರೆ ಆದರೆ (flag & FLAG_PROTECT_ಗಣ) {
#ಆದರೆ defined(CONFIG_SYS_FLASH_PROTECTION)
				flash_real_protect(info, i, 1);
#ಇಲ್ಲದಿದ್ದರೆ
				info->protect[i] = 1;
#ಆದರೆ_ಅಂತ್ಯ	/* CONFIG_SYS_FLASH_PROTECTION */
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
	ಪೂರ್ಣಾಂಕ i;

	ಗಾಗಿ (i = 0, info = &flash_info[0]; i < CFI_FLASH_BANKS; ++i, ++info) {
		ಆದರೆ (info->flash_id != FLASH_UNKNOWN &&
		    addr >= info->ಆರಂಭ[0] &&
		    /* WARNING - The '- 1' is needed if the flash
		     * is at the end of the address space, since
		     * info->start[0] + info->size wraps back to 0.
		     * Please don't change this unless you understand this.
		     */
		    addr <= info->ಆರಂಭ[0] + info->ಗಾತ್ರ_2 - 1) {
			ಹಿಂತಿರುಗಿ (info);
		}
	}

	ಹಿಂತಿರುಗಿ (ಖಾಲಿ);
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
ಪೂರ್ಣಾಂಕ
flash_write(ಅಕ್ಷರ *src, ulong addr, ulong cnt)
{
	ಪೂರ್ಣಾಂಕ i;
	ulong         ಕೊನೆ_2        = addr + cnt - 1;
	flash_info_t *info_ಮೊದಲ = addr2info(addr);
	flash_info_t *info_ಕೊನೆಯ  = addr2info(ಕೊನೆ_2);
	flash_info_t *info;
	__maybe_unused ಅಕ್ಷರ *src_orig = src;
	__maybe_unused ಅಕ್ಷರ *addr_orig = (ಅಕ್ಷರ *)addr;
	__maybe_unused ulong cnt_orig = cnt;

	ಆದರೆ (cnt == 0) {
		ಹಿಂತಿರುಗಿ (FL_ERR_OK);
	}

	ಆದರೆ (!info_ಮೊದಲ || !info_ಕೊನೆಯ) {
		ಹಿಂತಿರುಗಿ (FL_ERR_INVAL);
	}

	ಗಾಗಿ (info = info_ಮೊದಲ; info <= info_ಕೊನೆಯ; ++info) {
		ulong b_ಕೊನೆ = info->ಆರಂಭ[0] + info->ಗಾತ್ರ_2;	/* bank end addr */
		ಗಿಡ್ಡ s_ಕೊನೆ = info->sector_ಎಣಿಕೆ - 1;
		ಗಾಗಿ (i=0; i<info->sector_ಎಣಿಕೆ; ++i) {
			ulong e_addr = (i == s_ಕೊನೆ) ? b_ಕೊನೆ : info->ಆರಂಭ[i + 1];

			ಆದರೆ ((ಕೊನೆ_2 >= info->ಆರಂಭ[i]) && (addr < e_addr) &&
			    (info->protect[i] != 0) ) {
				ಹಿಂತಿರುಗಿ (FL_ERR_PROTECTED);
			}
		}
	}

	/* finally write data to flash */
	ಗಾಗಿ (info = info_ಮೊದಲ; info <= info_ಕೊನೆಯ && cnt>0; ++info) {
		ulong len;

		len = info->ಆರಂಭ[0] + info->ಗಾತ್ರ_2 - addr;
		ಆದರೆ (len > cnt)
			len = cnt;
		ಆದರೆ ((i = write_buff(info, (uchar *)src, addr, len)) != 0) {
			ಹಿಂತಿರುಗಿ (i);
		}
		cnt  -= len;
		addr += len;
		src  += len;
	}

#ಆದರೆ defined(CONFIG_FLASH_VERIFY)
	ಆದರೆ (memcmp(src_orig, addr_orig, cnt_orig)) {
		ಬರೆ("\nVerify failed!\n");
		ಹಿಂತಿರುಗಿ FL_ERR_PROG_ERROR;
	}
#ಆದರೆ_ಅಂತ್ಯ /* CONFIG_SYS_FLASH_VERIFY_AFTER_WRITE */

	ಹಿಂತಿರುಗಿ (FL_ERR_OK);
}

/*-----------------------------------------------------------------------
 */

ಶೂನ್ಯ flash_perror(ಪೂರ್ಣಾಂಕ err)
{
	ಆಯ್ಕೆ (err) {
	ಸ್ಥಿತಿ FL_ERR_OK:
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ FL_ERR_TIMEOUT:
		puts ("Timeout writing to Flash\n");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ FL_ERR_NOT_ERASED:
		puts ("Flash not Erased\n");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ FL_ERR_PROTECTED:
		puts ("Can't write to protected Flash sectors\n");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ FL_ERR_INVAL:
		puts ("Outside available Flash\n");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ FL_ERR_ALIGN:
		puts ("Start and/or end address not on sector boundary\n");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ FL_ERR_UNKNOWN_FLASH_VENDOR:
		puts ("Unknown Vendor of Flash\n");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ FL_ERR_UNKNOWN_FLASH_TYPE:
		puts ("Unknown Type of Flash\n");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ FL_ERR_PROG_ERROR:
		puts ("General Flash Programming Error\n");
		ನಿಲ್ಲಿಸು;
	ಸ್ಥಿತಿ FL_ERR_ABORTED:
		puts("Flash Programming Aborted\n");
		ನಿಲ್ಲಿಸು;
	ಪೂರ್ವನಿಯೋಜಿತ:
		ಬರೆ ("%s[%d] FIXME: rc=%d\n", __FILE__, __LINE__, err);
		ನಿಲ್ಲಿಸು;
	}
}
