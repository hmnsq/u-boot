// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/* #define DEBUG */

#അന്തര്ഭാവ <flash.h>
#അന്തര്ഭാവ <log.h>
#അന്തര്ഭാവ <u-boot/uuid.h>
#അന്തര്ഭാവ <linux/string.h>

#അന്തര്ഭാവ <mtd/cfi_flash.h>

/*-----------------------------------------------------------------------
 * Functions
 */

/*-----------------------------------------------------------------------
 * Set protection status for monitor sectors
 *
 * The monitor is always located in the _first_ Flash bank.
 * If necessary you have to map the second bank at lower addresses.
 */
ശൂന്യം
flash_protect(പൂർണ്ണസംഖ്യ flag, ulong from, ulong to, flash_info_t *info)
{
	ulong b_end;
	short s_end;
	പൂർണ്ണസംഖ്യ i;

	/* Do nothing if input data is bad. */
	എങ്കിൽ (!info || info->sector_എണ്ണം == 0 || info->size == 0 || to < from) {
		തിരികെ;
	}

	s_end = info->sector_എണ്ണം - 1;	/* index of last sector */
	b_end = info->start[0] + info->size - 1;	/* bank end address */

	debug("%s %s: from 0x%08lX to 0x%08lX\n", __func__,
	      (flag & FLAG_PROTECT_SET) ? "ON" :
	      (flag & FLAG_PROTECT_CLEAR) ? "OFF" : "???",
	      from, to);

	/* There is nothing to do if we have no data about the flash
	 * or the protect range and flash range don't overlap.
	 */
	എങ്കിൽ (info->flash_id == FLASH_UNKNOWN ||
	    to < info->start[0] || from > b_end) {
		തിരികെ;
	}

	വേണ്ടി (i=0; i<info->sector_എണ്ണം; ++i) {
		ulong end;		/* last address in current sect	*/

		end = (i == s_end) ? b_end : info->start[i + 1] - 1;

		/* Update protection if any part of the sector
		 * is in the specified range.
		 */
		എങ്കിൽ (from <= end && to >= info->start[i]) {
			എങ്കിൽ (flag & FLAG_PROTECT_CLEAR) {
#എങ്കിൽ defined(CONFIG_SYS_FLASH_PROTECTION)
				flash_real_protect(info, i, 0);
#അല്ലെങ്കിൽ
				info->protect[i] = 0;
#endif	/* CONFIG_SYS_FLASH_PROTECTION */
				debug("protect off %d\n", i);
			}
			അല്ലെങ്കിൽ എങ്കിൽ (flag & FLAG_PROTECT_SET) {
#എങ്കിൽ defined(CONFIG_SYS_FLASH_PROTECTION)
				flash_real_protect(info, i, 1);
#അല്ലെങ്കിൽ
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
	പൂർണ്ണസംഖ്യ i;

	വേണ്ടി (i = 0, info = &flash_info[0]; i < CFI_FLASH_BANKS; ++i, ++info) {
		എങ്കിൽ (info->flash_id != FLASH_UNKNOWN &&
		    addr >= info->start[0] &&
		    /* WARNING - The '- 1' is needed if the flash
		     * is at the end of the address space, since
		     * info->start[0] + info->size wraps back to 0.
		     * Please don't change this unless you understand this.
		     */
		    addr <= info->start[0] + info->size - 1) {
			തിരികെ (info);
		}
	}

	തിരികെ (ശൂന്യ);
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
പൂർണ്ണസംഖ്യ
flash_write(അക്ഷരം *src, ulong addr, ulong cnt)
{
	പൂർണ്ണസംഖ്യ i;
	ulong         end        = addr + cnt - 1;
	flash_info_t *info_first = addr2info(addr);
	flash_info_t *info_last  = addr2info(end);
	flash_info_t *info;
	__maybe_unused അക്ഷരം *src_orig = src;
	__maybe_unused അക്ഷരം *addr_orig = (അക്ഷരം *)addr;
	__maybe_unused ulong cnt_orig = cnt;

	എങ്കിൽ (cnt == 0) {
		തിരികെ (FL_ERR_OK);
	}

	എങ്കിൽ (!info_first || !info_last) {
		തിരികെ (FL_ERR_INVAL);
	}

	വേണ്ടി (info = info_first; info <= info_last; ++info) {
		ulong b_end = info->start[0] + info->size;	/* bank end addr */
		short s_end = info->sector_എണ്ണം - 1;
		വേണ്ടി (i=0; i<info->sector_എണ്ണം; ++i) {
			ulong e_addr = (i == s_end) ? b_end : info->start[i + 1];

			എങ്കിൽ ((end >= info->start[i]) && (addr < e_addr) &&
			    (info->protect[i] != 0) ) {
				തിരികെ (FL_ERR_PROTECTED);
			}
		}
	}

	/* finally write data to flash */
	വേണ്ടി (info = info_first; info <= info_last && cnt>0; ++info) {
		ulong len;

		len = info->start[0] + info->size - addr;
		എങ്കിൽ (len > cnt)
			len = cnt;
		എങ്കിൽ ((i = write_buff(info, (uchar *)src, addr, len)) != 0) {
			തിരികെ (i);
		}
		cnt  -= len;
		addr += len;
		src  += len;
	}

#എങ്കിൽ defined(CONFIG_FLASH_VERIFY)
	എങ്കിൽ (memcmp(src_orig, addr_orig, cnt_orig)) {
		printf("\nVerify failed!\n");
		തിരികെ FL_ERR_PROG_ERROR;
	}
#endif /* CONFIG_SYS_FLASH_VERIFY_AFTER_WRITE */

	തിരികെ (FL_ERR_OK);
}

/*-----------------------------------------------------------------------
 */

ശൂന്യം flash_perror(പൂർണ്ണസംഖ്യ err)
{
	തിരഞ്ഞെടുപ്പ് (err) {
	അവസ്ഥ FL_ERR_OK:
		നിർത്തുക;
	അവസ്ഥ FL_ERR_TIMEOUT:
		puts ("Timeout writing to Flash\n");
		നിർത്തുക;
	അവസ്ഥ FL_ERR_NOT_ERASED:
		puts ("Flash not Erased\n");
		നിർത്തുക;
	അവസ്ഥ FL_ERR_PROTECTED:
		puts ("Can't write to protected Flash sectors\n");
		നിർത്തുക;
	അവസ്ഥ FL_ERR_INVAL:
		puts ("Outside available Flash\n");
		നിർത്തുക;
	അവസ്ഥ FL_ERR_ALIGN:
		puts ("Start and/or end address not on sector boundary\n");
		നിർത്തുക;
	അവസ്ഥ FL_ERR_UNKNOWN_FLASH_VENDOR:
		puts ("Unknown Vendor of Flash\n");
		നിർത്തുക;
	അവസ്ഥ FL_ERR_UNKNOWN_FLASH_TYPE:
		puts ("Unknown Type of Flash\n");
		നിർത്തുക;
	അവസ്ഥ FL_ERR_PROG_ERROR:
		puts ("General Flash Programming Error\n");
		നിർത്തുക;
	അവസ്ഥ FL_ERR_ABORTED:
		puts("Flash Programming Aborted\n");
		നിർത്തുക;
	സ്ഥിരസ്ഥിതി:
		printf ("%s[%d] FIXME: rc=%d\n", __FILE__, __LINE__, err);
		നിർത്തുക;
	}
}
