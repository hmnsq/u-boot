// SPDX-License-Identifier: GPL-2.0+ BSD-3-Clause
/*
 * Copyright 2018 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#وصف_ڪريو LOG_CATEGORY	LOGC_BLOBLIST

#ضمن <bloblist.h>
#ضمن <display_options.h>
#ضمن <log.h>
#ضمن <malloc.h>
#ضمن <mapmem.h>
#ضمن <spl.h>
#ضمن <tables_csum.h>
#ضمن <asm/global_data.h>
#ضمن <u-boot/crc.h>

/*
 * A bloblist is a single contiguous chunk of memory with a header
 * (struct bloblist_hdr) and a number of blobs in it.
 *
 * Each blob starts on a BLOBLIST_ALIGN boundary relative to the start of the
 * bloblist and consists of a struct bloblist_rec, some padding to the required
 * alignment for the blog and then the actual data. The padding ensures that the
 * start address of the data in each blob is aligned as required. Note that
 * each blob's *data* is aligned to BLOBLIST_ALIGN regardless of the alignment
 * of the bloblist itself or the blob header.
 */

DECLARE_GLOBAL_معلومات_PTR;

ساكن بنية ٽيگ_name {
	تعداد bloblist_ٽيگ_t ٽيگ;
	ثابت حرف *name;
} ٽيگ_name[] = {
	{ BLOBLISTT_VOID, "(void)" },

	/* BLOBLISTT_AREA_FIRMWARE_TOP */
	{ BLOBLISTT_CONTROL_FDT, "Control FDT" },
	{ BLOBLISTT_HOB_بلاڪ, "HOB block" },
	{ BLOBLISTT_HOB_LIST, "HOB list" },
	{ BLOBLISTT_ACPI_TABLES, "ACPI tables for x86" },
	{ BLOBLISTT_TPM_EVLOG, "TPM event log defined by TCG EFI" },
	{ BLOBLISTT_TPM_CRB_بنياد, "TPM Command Response Buffer address" },
	{ BLOBLISTT_FDT_OVERLAY, "DT overlay" },

	/* BLOBLISTT_AREA_FIRMWARE */
	{ BLOBLISTT_TPM2_TCG_LOG, "TPM v2 log space" },
	{ BLOBLISTT_TCPA_LOG, "TPM log space" },
	{ BLOBLISTT_ACPI_GNVS, "ACPI GNVS" },

	/* BLOBLISTT_AREA_TF */
	{ BLOBLISTT_OPTEE_PAGABLE_PART, "OP-TEE pagable part" },

	/* BLOBLISTT_AREA_OTHER */
	{ BLOBLISTT_INTEL_VBT, "Intel Video-BIOS table" },
	{ BLOBLISTT_SMBIOS_TABLES, "SMBIOS tables for x86" },
	{ BLOBLISTT_VBOOT_CTX, "Chrome OS vboot context" },

	/* BLOBLISTT_PROJECT_AREA */
	{ BLOBLISTT_U_BOOT_SPL_HANDOFF, "SPL hand-off" },
	{ BLOBLISTT_VBE, "VBE" },
	{ BLOBLISTT_U_BOOT_VIDEO, "SPL video handoff" },

	/* BLOBLISTT_VENDOR_AREA */
};

ثابت حرف *bloblist_ٽيگ_name(تعداد bloblist_ٽيگ_t ٽيگ)
{
	صحيح i;

	لكل (i = 0; i < ARRAY_سائيز(ٽيگ_name); i++) {
		إذا (ٽيگ_name[i].ٽيگ == ٽيگ)
			أرجع ٽيگ_name[i].name;
	}

	أرجع "invalid";
}

ساكن بنية bloblist_rec *bloblist_پهريون_blob(بنية bloblist_hdr *hdr)
{
	إذا (hdr->استعمال_ٿيل_سائيز <= hdr->hdr_سائيز)
		أرجع عدم;
	أرجع (بنية bloblist_rec *)((فراغ *)hdr + hdr->hdr_سائيز);
}

ساكن مضمن uint rec_hdr_سائيز(بنية bloblist_rec *rec)
{
	أرجع (rec->ٽيگ_and_hdr_سائيز & BLOBLISTR_HDR_سائيز_نقاب) >>
		BLOBLISTR_HDR_سائيز_سرڪايو;
}

ساكن مضمن uint rec_ٽيگ(بنية bloblist_rec *rec)
{
	أرجع (rec->ٽيگ_and_hdr_سائيز & BLOBLISTR_ٽيگ_نقاب) >>
		BLOBLISTR_ٽيگ_سرڪايو;
}

ساكن مضمن فراغ void_blob(بنية bloblist_rec *rec)
{
	إذا (rec_ٽيگ(rec) == BLOBLISTT_VOID)
		أرجع;
	rec->ٽيگ_and_hdr_سائيز = BLOBLISTT_VOID |
				حجم(*rec) << BLOBLISTR_HDR_سائيز_سرڪايو;
}

ساكن مضمن بنية bloblist_rec *rec_from_blob(فراغ *blob)
{
	أرجع (blob - حجم(بنية bloblist_rec));
}

ساكن ulong bloblist_blob_پڇاڙي_ofs(بنية bloblist_hdr *hdr,
				   بنية bloblist_rec *rec)
{
	ulong وٿي;

	وٿي = (فراغ *)rec - (فراغ *)hdr;
	/*
	 * The data section of next TE should start from an address aligned
	 * to 1 << hdr->align_log2.
	 */
	وٿي += rec_hdr_سائيز(rec) + rec->سائيز;
	وٿي = ڦيرو_مٿي(وٿي + rec_hdr_سائيز(rec), 1 << hdr->align_log2);
	وٿي -= rec_hdr_سائيز(rec);

	أرجع وٿي;
}

ساكن بنية bloblist_rec *bloblist_ايندڙ_blob(بنية bloblist_hdr *hdr,
					       بنية bloblist_rec *rec)
{
	ulong وٿي = bloblist_blob_پڇاڙي_ofs(hdr, rec);

	إذا (وٿي >= hdr->استعمال_ٿيل_سائيز)
		أرجع عدم;
	أرجع (بنية bloblist_rec *)((فراغ *)hdr + وٿي);
}

#وصف_ڪريو foreach_rec(_rec, _hdr) \
	لكل (_rec = bloblist_پهريون_blob(_hdr); \
	     _rec; \
	     _rec = bloblist_ايندڙ_blob(_hdr, _rec))

ساكن بنية bloblist_rec *bloblist_findrec(uint ٽيگ)
{
	بنية bloblist_hdr *hdr = gd->bloblist;
	بنية bloblist_rec *rec;

	إذا (!hdr)
		أرجع عدم;

	foreach_rec(rec, hdr) {
		إذا (rec_ٽيگ(rec) == ٽيگ)
			أرجع rec;
	}

	أرجع عدم;
}

ساكن صحيح bloblist_addrec(uint ٽيگ, صحيح سائيز, صحيح align_log2,
			   بنية bloblist_rec **recp)
{
	بنية bloblist_hdr *hdr = gd->bloblist;
	بنية bloblist_rec *rec;
	phys_addr_t معلومات_شروعات, aligned_شروعات;
	phys_سائيز_t new_alloced;

	إذا (!align_log2)
		align_log2 = BLOBLIST_BLOB_ALIGN_LOG2;

	/* Figure out where the new data will start */
	معلومات_شروعات = map_منزل_sysmem(hdr) + hdr->استعمال_ٿيل_سائيز + حجم(*rec);

	/* Align the address and then calculate the offset from used size */
	aligned_شروعات = ALIGN(معلومات_شروعات, 1U << align_log2) - معلومات_شروعات;

	/* If we need to create a dummy record, create it */
	إذا (aligned_شروعات) {
		صحيح void_سائيز = aligned_شروعات - حجم(*rec);
		بنية bloblist_rec *vrec;
		صحيح ret;

		ret = bloblist_addrec(BLOBLISTT_VOID, void_سائيز, 0, &vrec);
		إذا (ret)
			أرجع log_نياپو_ret("void", ret);

		/* start the record after that */
		معلومات_شروعات = map_منزل_sysmem(hdr) + hdr->استعمال_ٿيل_سائيز + حجم(*vrec);
	}

	/* Calculate the new allocated total */
	new_alloced = معلومات_شروعات - map_منزل_sysmem(hdr) +
		ALIGN(سائيز, 1U << align_log2);

	إذا (new_alloced > hdr->ڪل_سائيز) {
		log_err("Failed to allocate %x bytes\n", سائيز);
		log_err("Used size=%x, total size=%x\n",
			hdr->استعمال_ٿيل_سائيز, hdr->ڪل_سائيز);
		أرجع log_نياپو_ret("bloblist add", -ENOSPC);
	}
	rec = (فراغ *)hdr + hdr->استعمال_ٿيل_سائيز;

	rec->ٽيگ_and_hdr_سائيز = ٽيگ | حجم(*rec) << BLOBLISTR_HDR_سائيز_سرڪايو;
	rec->سائيز = سائيز;

	/* Zero the record data */
	memset((فراغ *)rec + rec_hdr_سائيز(rec), '\0', rec->سائيز);

	hdr->استعمال_ٿيل_سائيز = new_alloced;
	*recp = rec;

	أرجع 0;
}

ساكن صحيح bloblist_ensurerec(uint ٽيگ, بنية bloblist_rec **recp, صحيح سائيز,
			      صحيح align_log2)
{
	بنية bloblist_rec *rec;

	rec = bloblist_findrec(ٽيگ);
	إذا (rec) {
		إذا (سائيز && سائيز != rec->سائيز) {
			*recp = rec;
			أرجع -ESPIPE;
		}
	} وإلا {
		صحيح ret;

		ret = bloblist_addrec(ٽيگ, سائيز, align_log2, &rec);
		إذا (ret)
			أرجع ret;
	}
	*recp = rec;

	أرجع 0;
}

ساكن صحيح bloblist_get_blob_معلومات_وٿي(uint ٽيگ)
{
	بدل (ٽيگ) {
	حالة BLOBLISTT_FDT_OVERLAY:
		أرجع حجم(بنية dto_blob_hdr);
	/*
	 * return the data offset if it is not following the blob
	 * header immediately.
	 */
	}
	أرجع 0;
}

فراغ *bloblist_ڳوليو(uint ٽيگ, صحيح سائيز)
{
	فراغ *blob = عدم;
	صحيح blob_سائيز;

	blob = bloblist_get_blob(ٽيگ, &blob_سائيز);

	إذا (سائيز && سائيز != blob_سائيز)
		أرجع عدم;

	أرجع blob;
}

فراغ *bloblist_get_blob(uint ٽيگ, صحيح *sizep)
{
	بنية bloblist_rec *rec;

	rec = bloblist_findrec(ٽيگ);
	إذا (!rec)
		أرجع عدم;

	*sizep = rec->سائيز;

	أرجع (فراغ *)rec + rec_hdr_سائيز(rec);
}

صحيح bloblist_apply_blobs(uint ٽيگ, صحيح (*func)(فراغ **معلومات, صحيح سائيز))
{
	بنية bloblist_hdr *hdr = gd->bloblist;
	بنية bloblist_rec *rec;

	إذا (!func || !hdr)
		أرجع -ENOENT;

	foreach_rec(rec, hdr) {
		/* Apply all blobs with the specified tag */
		إذا (rec_ٽيگ(rec) == ٽيگ) {
			صحيح ret;
			صحيح ٽيگ = rec_ٽيگ(rec);
			فراغ *blob = (فراغ *)rec + rec_hdr_سائيز(rec);
			صحيح dat_off = bloblist_get_blob_معلومات_وٿي(ٽيگ);

			blob += dat_off;
			ret = func(&blob, rec->سائيز - dat_off);
			إذا (ret) {
				log_err("Failed to apply blob with tag %d\n",
					ٽيگ);
				أرجع ret;
			}

			rec = rec_from_blob(blob - dat_off);
			إذا (!rec) {
				log_err("Blob corrupted\n");
				أرجع -ENOENT;
			}

			/* Mark applied blob record as void */
			void_blob(rec);
		}
	}

	أرجع 0;
}

فراغ *bloblist_جوڙيو(uint ٽيگ, صحيح سائيز, صحيح align_log2)
{
	بنية bloblist_rec *rec;

	إذا (bloblist_addrec(ٽيگ, سائيز, align_log2, &rec))
		أرجع عدم;

	أرجع (فراغ *)rec + rec_hdr_سائيز(rec);
}

صحيح bloblist_ensure_سائيز(uint ٽيگ, صحيح سائيز, صحيح align_log2, فراغ **blobp)
{
	بنية bloblist_rec *rec;
	صحيح ret;

	ret = bloblist_ensurerec(ٽيگ, &rec, سائيز, align_log2);
	إذا (ret)
		أرجع ret;
	*blobp = (فراغ *)rec + rec_hdr_سائيز(rec);

	أرجع 0;
}

فراغ *bloblist_ensure(uint ٽيگ, صحيح سائيز)
{
	بنية bloblist_rec *rec;

	إذا (bloblist_ensurerec(ٽيگ, &rec, سائيز, 0))
		أرجع عدم;

	أرجع (فراغ *)rec + rec_hdr_سائيز(rec);
}

صحيح bloblist_ensure_سائيز_ret(uint ٽيگ, صحيح *sizep, فراغ **blobp)
{
	بنية bloblist_rec *rec;
	صحيح ret;

	ret = bloblist_ensurerec(ٽيگ, &rec, *sizep, 0);
	إذا (ret == -ESPIPE)
		*sizep = rec->سائيز;
	وإلا إذا (ret)
		أرجع ret;
	*blobp = (فراغ *)rec + rec_hdr_سائيز(rec);

	أرجع 0;
}

ساكن صحيح bloblist_resize_rec(بنية bloblist_hdr *hdr,
			       بنية bloblist_rec *rec,
			       صحيح new_سائيز)
{
	صحيح expand_by;	/* Number of bytes to expand by (-ve to contract) */
	صحيح new_alloced;
	ulong ايندڙ_ofs;	/* Offset of the record after @rec */

	expand_by = ALIGN(new_سائيز - rec->سائيز, BLOBLIST_BLOB_ALIGN);
	new_alloced = ALIGN(hdr->استعمال_ٿيل_سائيز + expand_by, BLOBLIST_BLOB_ALIGN);
	إذا (new_سائيز < 0) {
		log_debug("Attempt to shrink blob size below 0 (%x)\n",
			  new_سائيز);
		أرجع log_نياپو_ret("size", -EINVAL);
	}
	إذا (new_alloced > hdr->ڪل_سائيز) {
		log_err("Failed to allocate %x bytes\n", new_سائيز);
		log_err("Used size=%x, total size=%x\n",
			hdr->استعمال_ٿيل_سائيز, hdr->ڪل_سائيز);
		أرجع log_نياپو_ret("alloc", -ENOSPC);
	}

	/* Move the following blobs up or down, if this is not the last */
	ايندڙ_ofs = bloblist_blob_پڇاڙي_ofs(hdr, rec);
	إذا (ايندڙ_ofs != hdr->استعمال_ٿيل_سائيز) {
		memmove((فراغ *)hdr + ايندڙ_ofs + expand_by,
			(فراغ *)hdr + ايندڙ_ofs, hdr->استعمال_ٿيل_سائيز - ايندڙ_ofs);
	}
	hdr->استعمال_ٿيل_سائيز = new_alloced;

	/* Zero the new part of the blob */
	إذا (expand_by > 0) {
		memset((فراغ *)rec + rec_hdr_سائيز(rec) + rec->سائيز, '\0',
		       new_سائيز - rec->سائيز);
	}

	/* Update the size of this blob */
	rec->سائيز = new_سائيز;

	أرجع 0;
}

صحيح bloblist_resize(uint ٽيگ, صحيح new_سائيز)
{
	بنية bloblist_hdr *hdr = gd->bloblist;
	بنية bloblist_rec *rec;
	صحيح ret;

	rec = bloblist_findrec(ٽيگ);
	إذا (!rec)
		أرجع log_نياپو_ret("find", -ENOENT);
	ret = bloblist_resize_rec(hdr, rec, new_سائيز);
	إذا (ret)
		أرجع log_نياپو_ret("resize", ret);

	أرجع 0;
}

ساكن u32 bloblist_calc_chksum(بنية bloblist_hdr *hdr)
{
	u8 chksum;

	chksum = جدول_compute_checksum(hdr, hdr->استعمال_ٿيل_سائيز);
	chksum += hdr->chksum;

	أرجع chksum;
}

صحيح bloblist_new(ulong addr, uint سائيز, uint flags, uint align_log2)
{
	بنية bloblist_hdr *hdr;

	إذا (سائيز < حجم(*hdr))
		أرجع log_ret(-ENOSPC);
	إذا (addr & (BLOBLIST_ALIGN - 1))
		أرجع log_ret(-EFAULT);
	hdr = map_sysmem(addr, سائيز);
	memset(hdr, '\0', حجم(*hdr));
	hdr->version = BLOBLIST_نسخو;
	hdr->hdr_سائيز = حجم(*hdr);
	hdr->flags = flags;
	hdr->magic = BLOBLIST_MAGIC;
	hdr->استعمال_ٿيل_سائيز = hdr->hdr_سائيز;
	hdr->ڪل_سائيز = سائيز;
	hdr->align_log2 = align_log2 ? align_log2 : BLOBLIST_BLOB_ALIGN_LOG2;
	hdr->chksum = 0;
	gd->bloblist = hdr;
	gd->flags |= GD_FLG_BLOBLIST_HANDOFF;

	أرجع 0;
}

صحيح bloblist_check(ulong addr, uint سائيز)
{
	بنية bloblist_hdr *hdr;
	u32 chksum;

	hdr = map_sysmem(addr, حجم(*hdr));
	إذا (hdr->magic != BLOBLIST_MAGIC)
		أرجع log_نياپو_ret("Bad magic", -ENOENT);
	إذا (hdr->version != BLOBLIST_نسخو)
		أرجع log_نياپو_ret("Bad version", -EPROTONOSUPPORT);
	إذا (!hdr->ڪل_سائيز || (سائيز && hdr->ڪل_سائيز > سائيز))
		أرجع log_نياپو_ret("Bad total size", -EFBIG);
	إذا (hdr->استعمال_ٿيل_سائيز > hdr->ڪل_سائيز)
		أرجع log_نياپو_ret("Bad used size", -ENOENT);
	إذا (hdr->hdr_سائيز != حجم(بنية bloblist_hdr))
		أرجع log_نياپو_ret("Bad header size", -ENOENT);

	chksum = bloblist_calc_chksum(hdr);
	إذا (hdr->chksum != chksum) {
		log_err("Checksum %x != %x\n", hdr->chksum, chksum);
		أرجع log_نياپو_ret("Bad checksum", -EIO);
	}
	gd->bloblist = hdr;
	gd->flags |= GD_FLG_BLOBLIST_HANDOFF;

	أرجع 0;
}

صحيح bloblist_پڄاڻي(فراغ)
{
	بنية bloblist_hdr *hdr = gd->bloblist;

	hdr->chksum = bloblist_calc_chksum(hdr);
	log_debug("Finished bloblist size %lx at %lx\n", (ulong)hdr->استعمال_ٿيل_سائيز,
		  (ulong)map_منزل_sysmem(hdr));

	أرجع 0;
}

ulong bloblist_get_بنياد(فراغ)
{
	أرجع map_منزل_sysmem(gd->bloblist);
}

ulong bloblist_get_سائيز(فراغ)
{
	بنية bloblist_hdr *hdr = gd->bloblist;

	أرجع hdr->استعمال_ٿيل_سائيز;
}

ulong bloblist_get_ڪل_سائيز(فراغ)
{
	بنية bloblist_hdr *hdr = gd->bloblist;

	أرجع hdr->ڪل_سائيز;
}

فراغ bloblist_get_stats(ulong *basep, ulong *tsizep, ulong *usizep)
{
	بنية bloblist_hdr *hdr = gd->bloblist;

	*basep = map_منزل_sysmem(gd->bloblist);
	*tsizep = hdr->ڪل_سائيز;
	*usizep = hdr->استعمال_ٿيل_سائيز;
}

ساكن فراغ show_قدر(ثابت حرف *prompt, ulong قدر)
{
	لکو("%s:%*s %-5lx  ", prompt, 10 - (صحيح)لڙي_ڊيگهه(prompt), "", قدر);
	print_سائيز(قدر, "\n");
}

فراغ bloblist_show_stats(فراغ)
{
	ulong بنياد, tsize, usize;

	bloblist_get_stats(&بنياد, &tsize, &usize);
	لکو("base:       %lx\n", بنياد);
	show_قدر("total size", tsize);
	show_قدر("used size", usize);
	show_قدر("free", tsize - usize);
}

فراغ bloblist_show_list(فراغ)
{
	بنية bloblist_hdr *hdr = gd->bloblist;
	بنية bloblist_rec *rec;

	لکو("%-8s  %8s   Tag Name\n", "Address", "Size");
	لكل (rec = bloblist_پهريون_blob(hdr); rec;
	     rec = bloblist_ايندڙ_blob(hdr, rec)) {
		لکو("%08lx  %8x  %4x %s\n",
		       (ulong)map_منزل_sysmem((فراغ *)rec + rec_hdr_سائيز(rec)),
		       rec->سائيز, rec_ٽيگ(rec),
		       bloblist_ٽيگ_name(rec_ٽيگ(rec)));
	}
}

صحيح bloblist_reloc(فراغ *منزل, uint منزل_سائيز)
{
	بنية bloblist_hdr *hdr;

	إذا (!منزل_سائيز)
		أرجع 0;

	إذا (منزل_سائيز < gd->bloblist->ڪل_سائيز)
		أرجع -ENOSPC;

	يادگيري_نقل(منزل, gd->bloblist, gd->bloblist->ڪل_سائيز);
	hdr = منزل;
	hdr->ڪل_سائيز = منزل_سائيز;
	gd->bloblist = منزل;

	أرجع 0;
}

/*
 * Weak default function for getting bloblist from boot args.
 */
صحيح __weak xferlist_from_boot_arg(ulong __always_unused *addr)
{
	أرجع -ENOENT;
}

bool bloblist_exists(فراغ)
{
	صحيح ret;
	ulong addr = 0;

	/* Check if a valid transfer list passed in */
	إذا (!xferlist_from_boot_arg(&addr))
		اذهب مليو;

	/*
	 * If Firmware Handoff is mandatory but no transfer list is
	 * observed, report it as an error.
	 */
	إذا (IS_ENABLED(CONFIG_BLOBLIST_PASSAGE_MANDATORY))
		أرجع false;

	/*
	 * We have checked for a valid transfer list being passed. At this
	 * point, if we do not have a fixed address for the bloblist, we cannot
	 * be provided with one.
	 */
	إذا (xpl_is_پهريون_phase() || !IS_ENABLED(CONFIG_BLOBLIST_FIXED))
		أرجع false;

	/*
	 * Check for a valid list as the configured address.
	 */
	addr = IF_ENABLED_INT(CONFIG_BLOBLIST_FIXED,
			      CONFIG_BLOBLIST_ADDR);
	ret = bloblist_check(addr, CONFIG_BLOBLIST_سائيز);
	إذا (!ret)
		اذهب مليو;

	log_debug("Bloblist at %lx not found (err=%d)\n", addr, ret);
	أرجع false;

مليو:
#جيڪڏهن_وصف_ٿيل DEBUG
	bloblist_show_stats();
	bloblist_show_list();
#جيڪڏهن_پورو
	أرجع true;
}

صحيح bloblist_init(فراغ)
{
	صحيح ret;
	ulong addr = 0, سائيز = CONFIG_BLOBLIST_سائيز;

	إذا (gd->flags & GD_FLG_BLOBLIST_HANDOFF) {
		log_debug("Found existing bloblist size %x at %p\n",
			  gd->bloblist->ڪل_سائيز, gd->bloblist);
		أرجع 0;
	}

	/*
	 * If Firmware Handoff is mandatory but no transfer list has been
	 * observed by fdtdec_setup, report it as an error.
	 */
	إذا (IS_ENABLED(CONFIG_BLOBLIST_PASSAGE_MANDATORY))
		أرجع -ENOENT;

	/*
	 * If we don't have an existing bloblist, we either need
	 * to allocate one now, or initialize the fixed address
	 * space as a bloblist.
	 */
	إذا (CONFIG_IS_ENABLED(BLOBLIST_ALLOC)) {
		فراغ *ptr = memalign(BLOBLIST_ALIGN, سائيز);

		إذا (!ptr)
			أرجع log_نياپو_ret("alloc", -ENOMEM);
		addr = map_منزل_sysmem(ptr);
	} وإلا
		addr = IF_ENABLED_INT(CONFIG_BLOBLIST_FIXED,
				      CONFIG_BLOBLIST_ADDR);

	log_debug("Creating new bloblist size %lx at %lx\n", سائيز,
		  addr);
	ret = bloblist_new(addr, سائيز, 0, 0);
	إذا (ret)
		أرجع log_نياپو_ret("ini", ret);

	أرجع 0;
}

صحيح bloblist_check_reg_conv(ulong rfdt, ulong rzero, ulong rsig, ulong xlist)
{
	u64 version = BLOBLIST_REGCONV_VER;
	ulong sigval;
	صحيح ret;

	إذا ((IS_ENABLED(CONFIG_64_ٻٽو_انگ) && !IS_ENABLED(CONFIG_SPL_BUILD)) ||
			(IS_ENABLED(CONFIG_SPL_64_ٻٽو_انگ) && IS_ENABLED(CONFIG_SPL_BUILD))) {
		sigval = ((BLOBLIST_MAGIC & ((1ULL << BLOBLIST_REGCONV_سرڪايو_64) - 1)) |
			 ((version  & BLOBLIST_REGCONV_نقاب) << BLOBLIST_REGCONV_سرڪايو_64));
	} وإلا {
		sigval = ((BLOBLIST_MAGIC & ((1UL << BLOBLIST_REGCONV_سرڪايو_32) - 1)) |
			 ((version  & BLOBLIST_REGCONV_نقاب) << BLOBLIST_REGCONV_سرڪايو_32));
	}

	إذا (rzero || rsig != sigval)
		أرجع -EIO;

	ret = bloblist_check(xlist, 0);
	إذا (ret)
		أرجع ret;

	إذا (rfdt != (ulong)bloblist_ڳوليو(BLOBLISTT_CONTROL_FDT, 0)) {
		/* Remove this bloblist from gd */
		gd->bloblist = عدم;
		gd->flags &= ~GD_FLG_BLOBLIST_HANDOFF;
		أرجع -EIO;
	}

	أرجع 0;
}
