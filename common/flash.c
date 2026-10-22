// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 */

/* #define DEBUG */

#अन्तर्भाव <flash.h>
#अन्तर्भाव <log.h>
#अन्तर्भाव <u-boot/uuid.h>
#अन्तर्भाव <linux/string.h>

#अन्तर्भाव <mtd/cfi_flash.h>

/*-----------------------------------------------------------------------
 * Functions
 */

/*-----------------------------------------------------------------------
 * Set protection status for monitor sectors
 *
 * The monitor is always located in the _first_ Flash bank.
 * If necessary you have to map the second bank at lower addresses.
 */
शून्य
flash_protect(पूर्णाङ्क flag, ulong from, ulong to, flash_info_t *info)
{
	ulong b_end;
	short s_end;
	पूर्णाङ्क i;

	/* Do nothing if input data is bad. */
	सचे (!info || info->sector_गणनम् == 0 || info->size == 0 || to < from) {
		पटिनिवत्त;
	}

	s_end = info->sector_गणनम् - 1;	/* index of last sector */
	b_end = info->start[0] + info->size - 1;	/* bank end address */

	debug("%s %s: from 0x%08lX to 0x%08lX\n", __func__,
	      (flag & FLAG_PROTECT_SET) ? "ON" :
	      (flag & FLAG_PROTECT_CLEAR) ? "OFF" : "???",
	      from, to);

	/* There is nothing to do if we have no data about the flash
	 * or the protect range and flash range don't overlap.
	 */
	सचे (info->flash_id == FLASH_UNKNOWN ||
	    to < info->start[0] || from > b_end) {
		पटिनिवत्त;
	}

	अत्थाय (i=0; i<info->sector_गणनम्; ++i) {
		ulong end;		/* last address in current sect	*/

		end = (i == s_end) ? b_end : info->start[i + 1] - 1;

		/* Update protection if any part of the sector
		 * is in the specified range.
		 */
		सचे (from <= end && to >= info->start[i]) {
			सचे (flag & FLAG_PROTECT_CLEAR) {
#सचे defined(CONFIG_SYS_FLASH_PROTECTION)
				flash_real_protect(info, i, 0);
#अञ्ञथा
				info->protect[i] = 0;
#endif	/* CONFIG_SYS_FLASH_PROTECTION */
				debug("protect off %d\n", i);
			}
			अञ्ञथा सचे (flag & FLAG_PROTECT_SET) {
#सचे defined(CONFIG_SYS_FLASH_PROTECTION)
				flash_real_protect(info, i, 1);
#अञ्ञथा
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
	पूर्णाङ्क i;

	अत्थाय (i = 0, info = &flash_info[0]; i < CFI_FLASH_BANKS; ++i, ++info) {
		सचे (info->flash_id != FLASH_UNKNOWN &&
		    addr >= info->start[0] &&
		    /* WARNING - The '- 1' is needed if the flash
		     * is at the end of the address space, since
		     * info->start[0] + info->size wraps back to 0.
		     * Please don't change this unless you understand this.
		     */
		    addr <= info->start[0] + info->size - 1) {
			पटिनिवत्त (info);
		}
	}

	पटिनिवत्त (रिक्त);
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
पूर्णाङ्क
flash_write(वर्ण *src, ulong addr, ulong cnt)
{
	पूर्णाङ्क i;
	ulong         end        = addr + cnt - 1;
	flash_info_t *info_first = addr2info(addr);
	flash_info_t *info_last  = addr2info(end);
	flash_info_t *info;
	__maybe_unused वर्ण *src_orig = src;
	__maybe_unused वर्ण *addr_orig = (वर्ण *)addr;
	__maybe_unused ulong cnt_orig = cnt;

	सचे (cnt == 0) {
		पटिनिवत्त (FL_ERR_OK);
	}

	सचे (!info_first || !info_last) {
		पटिनिवत्त (FL_ERR_INVAL);
	}

	अत्थाय (info = info_first; info <= info_last; ++info) {
		ulong b_end = info->start[0] + info->size;	/* bank end addr */
		short s_end = info->sector_गणनम् - 1;
		अत्थाय (i=0; i<info->sector_गणनम्; ++i) {
			ulong e_addr = (i == s_end) ? b_end : info->start[i + 1];

			सचे ((end >= info->start[i]) && (addr < e_addr) &&
			    (info->protect[i] != 0) ) {
				पटिनिवत्त (FL_ERR_PROTECTED);
			}
		}
	}

	/* finally write data to flash */
	अत्थाय (info = info_first; info <= info_last && cnt>0; ++info) {
		ulong len;

		len = info->start[0] + info->size - addr;
		सचे (len > cnt)
			len = cnt;
		सचे ((i = write_buff(info, (uchar *)src, addr, len)) != 0) {
			पटिनिवत्त (i);
		}
		cnt  -= len;
		addr += len;
		src  += len;
	}

#सचे defined(CONFIG_FLASH_VERIFY)
	सचे (memcmp(src_orig, addr_orig, cnt_orig)) {
		printf("\nVerify failed!\n");
		पटिनिवत्त FL_ERR_PROG_ERROR;
	}
#endif /* CONFIG_SYS_FLASH_VERIFY_AFTER_WRITE */

	पटिनिवत्त (FL_ERR_OK);
}

/*-----------------------------------------------------------------------
 */

शून्य flash_perror(पूर्णाङ्क err)
{
	विकल्प (err) {
	पक्ष FL_ERR_OK:
		भङ्ग;
	पक्ष FL_ERR_TIMEOUT:
		puts ("Timeout writing to Flash\n");
		भङ्ग;
	पक्ष FL_ERR_NOT_ERASED:
		puts ("Flash not Erased\n");
		भङ्ग;
	पक्ष FL_ERR_PROTECTED:
		puts ("Can't write to protected Flash sectors\n");
		भङ्ग;
	पक्ष FL_ERR_INVAL:
		puts ("Outside available Flash\n");
		भङ्ग;
	पक्ष FL_ERR_ALIGN:
		puts ("Start and/or end address not on sector boundary\n");
		भङ्ग;
	पक्ष FL_ERR_UNKNOWN_FLASH_VENDOR:
		puts ("Unknown Vendor of Flash\n");
		भङ्ग;
	पक्ष FL_ERR_UNKNOWN_FLASH_TYPE:
		puts ("Unknown Type of Flash\n");
		भङ्ग;
	पक्ष FL_ERR_PROG_ERROR:
		puts ("General Flash Programming Error\n");
		भङ्ग;
	पक्ष FL_ERR_ABORTED:
		puts("Flash Programming Aborted\n");
		भङ्ग;
	स्वभाव:
		printf ("%s[%d] FIXME: rc=%d\n", __FILE__, __LINE__, err);
		भङ्ग;
	}
}
