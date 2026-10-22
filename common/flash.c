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
flash_protect(पूर्णाङ्क flag, ulong from, ulong गन्तव्यम्, flash_info_t *info)
{
	ulong b_अन्तः;
	ह्रस्व s_अन्तः;
	पूर्णाङ्क i;

	/* Do nothing if input data is bad. */
	यदि (!info || info->sector_गणनम् == 0 || info->आकार == 0 || गन्तव्यम् < from) {
		प्रत्यागम;
	}

	s_अन्तः = info->sector_गणनम् - 1;	/* index of last sector */
	b_अन्तः = info->start[0] + info->आकार - 1;	/* bank end address */

	debug("%s %s: from 0x%08lX to 0x%08lX\n", __func__,
	      (flag & FLAG_PROTECT_समुच्चयः) ? "ON" :
	      (flag & FLAG_PROTECT_CLEAR) ? "OFF" : "???",
	      from, गन्तव्यम्);

	/* There is nothing to do if we have no data about the flash
	 * or the protect range and flash range don't overlap.
	 */
	यदि (info->flash_id == FLASH_UNKNOWN ||
	    गन्तव्यम् < info->start[0] || from > b_अन्तः) {
		प्रत्यागम;
	}

	प्रति (i=0; i<info->sector_गणनम्; ++i) {
		ulong अन्तः_2;		/* last address in current sect	*/

		अन्तः_2 = (i == s_अन्तः) ? b_अन्तः : info->start[i + 1] - 1;

		/* Update protection if any part of the sector
		 * is in the specified range.
		 */
		यदि (from <= अन्तः_2 && गन्तव्यम् >= info->start[i]) {
			यदि (flag & FLAG_PROTECT_CLEAR) {
#यदि defined(CONFIG_SYS_FLASH_PROTECTION)
				flash_real_protect(info, i, 0);
#अन्यथा
				info->protect[i] = 0;
#यदि_अन्तः	/* CONFIG_SYS_FLASH_PROTECTION */
				debug("protect off %d\n", i);
			}
			अन्यथा यदि (flag & FLAG_PROTECT_समुच्चयः) {
#यदि defined(CONFIG_SYS_FLASH_PROTECTION)
				flash_real_protect(info, i, 1);
#अन्यथा
				info->protect[i] = 1;
#यदि_अन्तः	/* CONFIG_SYS_FLASH_PROTECTION */
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

	प्रति (i = 0, info = &flash_info[0]; i < CFI_FLASH_BANKS; ++i, ++info) {
		यदि (info->flash_id != FLASH_UNKNOWN &&
		    addr >= info->start[0] &&
		    /* WARNING - The '- 1' is needed if the flash
		     * is at the end of the address space, since
		     * info->start[0] + info->size wraps back to 0.
		     * Please don't change this unless you understand this.
		     */
		    addr <= info->start[0] + info->आकार - 1) {
			प्रत्यागम (info);
		}
	}

	प्रत्यागम (रिक्त);
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
	ulong         अन्तः_2        = addr + cnt - 1;
	flash_info_t *info_first = addr2info(addr);
	flash_info_t *info_चरमः  = addr2info(अन्तः_2);
	flash_info_t *info;
	__maybe_unused वर्ण *src_orig = src;
	__maybe_unused वर्ण *addr_orig = (वर्ण *)addr;
	__maybe_unused ulong cnt_orig = cnt;

	यदि (cnt == 0) {
		प्रत्यागम (FL_ERR_OK);
	}

	यदि (!info_first || !info_चरमः) {
		प्रत्यागम (FL_ERR_INVAL);
	}

	प्रति (info = info_first; info <= info_चरमः; ++info) {
		ulong b_अन्तः = info->start[0] + info->आकार;	/* bank end addr */
		ह्रस्व s_अन्तः = info->sector_गणनम् - 1;
		प्रति (i=0; i<info->sector_गणनम्; ++i) {
			ulong e_addr = (i == s_अन्तः) ? b_अन्तः : info->start[i + 1];

			यदि ((अन्तः_2 >= info->start[i]) && (addr < e_addr) &&
			    (info->protect[i] != 0) ) {
				प्रत्यागम (FL_ERR_PROTECTED);
			}
		}
	}

	/* finally write data to flash */
	प्रति (info = info_first; info <= info_चरमः && cnt>0; ++info) {
		ulong len;

		len = info->start[0] + info->आकार - addr;
		यदि (len > cnt)
			len = cnt;
		यदि ((i = write_buff(info, (uchar *)src, addr, len)) != 0) {
			प्रत्यागम (i);
		}
		cnt  -= len;
		addr += len;
		src  += len;
	}

#यदि defined(CONFIG_FLASH_VERIFY)
	यदि (memcmp(src_orig, addr_orig, cnt_orig)) {
		प्रारूप_लिख("\nVerify failed!\n");
		प्रत्यागम FL_ERR_PROG_ERROR;
	}
#यदि_अन्तः /* CONFIG_SYS_FLASH_VERIFY_AFTER_WRITE */

	प्रत्यागम (FL_ERR_OK);
}

/*-----------------------------------------------------------------------
 */

शून्य flash_perror(पूर्णाङ्क err)
{
	विकल्प (err) {
	पक्ष FL_ERR_OK:
		भङ्ग;
	पक्ष FL_ERR_TIMEOUT:
		पङ्क्तिं_लिख ("Timeout writing to Flash\n");
		भङ्ग;
	पक्ष FL_ERR_NOT_ERASED:
		पङ्क्तिं_लिख ("Flash not Erased\n");
		भङ्ग;
	पक्ष FL_ERR_PROTECTED:
		पङ्क्तिं_लिख ("Can't write to protected Flash sectors\n");
		भङ्ग;
	पक्ष FL_ERR_INVAL:
		पङ्क्तिं_लिख ("Outside available Flash\n");
		भङ्ग;
	पक्ष FL_ERR_ALIGN:
		पङ्क्तिं_लिख ("Start and/or end address not on sector boundary\n");
		भङ्ग;
	पक्ष FL_ERR_UNKNOWN_FLASH_VENDOR:
		पङ्क्तिं_लिख ("Unknown Vendor of Flash\n");
		भङ्ग;
	पक्ष FL_ERR_UNKNOWN_FLASH_TYPE:
		पङ्क्तिं_लिख ("Unknown Type of Flash\n");
		भङ्ग;
	पक्ष FL_ERR_PROG_ERROR:
		पङ्क्तिं_लिख ("General Flash Programming Error\n");
		भङ्ग;
	पक्ष FL_ERR_ABORTED:
		पङ्क्तिं_लिख("Flash Programming Aborted\n");
		भङ्ग;
	स्वभाव:
		प्रारूप_लिख ("%s[%d] FIXME: rc=%d\n", __FILE__, __LINE__, err);
		भङ्ग;
	}
}
