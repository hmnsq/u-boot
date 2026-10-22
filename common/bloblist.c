// SPDX-License-Identifier: GPL-2.0+ BSD-3-Clause
/*
 * Copyright 2018 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#تعریف_کرو LOG_CATEGORY	LOGC_BLOBLIST

#شامل <bloblist.h>
#شامل <display_options.h>
#شامل <log.h>
#شامل <malloc.h>
#شامل <mapmem.h>
#شامل <spl.h>
#شامل <tables_csum.h>
#شامل <asm/global_data.h>
#شامل <u-boot/crc.h>

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

DECLARE_GLOBAL_ڈیٹا_PTR;

ساکن ساخت لیبل_name {
	شمار bloblist_لیبل_t لیبل;
	ثابت حرف *name;
} لیبل_name[] = {
	{ BLOBLISTT_VOID, "(void)" },

	/* BLOBLISTT_AREA_FIRMWARE_TOP */
	{ BLOBLISTT_CONTROL_FDT, "Control FDT" },
	{ BLOBLISTT_HOB_بلاک, "HOB block" },
	{ BLOBLISTT_HOB_LIST, "HOB list" },
	{ BLOBLISTT_ACPI_TABLES, "ACPI tables for x86" },
	{ BLOBLISTT_TPM_EVLOG, "TPM event log defined by TCG EFI" },
	{ BLOBLISTT_TPM_CRB_بنیاد, "TPM Command Response Buffer address" },
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

ثابت حرف *bloblist_لیبل_name(شمار bloblist_لیبل_t لیبل)
{
	صحیح i;

	برائے (i = 0; i < ARRAY_سائز(لیبل_name); i++) {
		اگر (لیبل_name[i].لیبل == لیبل)
			واپس لیبل_name[i].name;
	}

	واپس "invalid";
}

ساکن ساخت bloblist_rec *bloblist_پہلا_blob(ساخت bloblist_hdr *hdr)
{
	اگر (hdr->مستعمل_سائز <= hdr->hdr_سائز)
		واپس تہی;
	واپس (ساخت bloblist_rec *)((خالی *)hdr + hdr->hdr_سائز);
}

ساکن اندرونی uint rec_hdr_سائز(ساخت bloblist_rec *rec)
{
	واپس (rec->لیبل_and_hdr_سائز & BLOBLISTR_HDR_سائز_نقاب) >>
		BLOBLISTR_HDR_سائز_سرکاؤ;
}

ساکن اندرونی uint rec_لیبل(ساخت bloblist_rec *rec)
{
	واپس (rec->لیبل_and_hdr_سائز & BLOBLISTR_لیبل_نقاب) >>
		BLOBLISTR_لیبل_سرکاؤ;
}

ساکن اندرونی خالی void_blob(ساخت bloblist_rec *rec)
{
	اگر (rec_لیبل(rec) == BLOBLISTT_VOID)
		واپس;
	rec->لیبل_and_hdr_سائز = BLOBLISTT_VOID |
				حجم(*rec) << BLOBLISTR_HDR_سائز_سرکاؤ;
}

ساکن اندرونی ساخت bloblist_rec *rec_from_blob(خالی *blob)
{
	واپس (blob - حجم(ساخت bloblist_rec));
}

ساکن ulong bloblist_blob_انتہا_ofs(ساخت bloblist_hdr *hdr,
				   ساخت bloblist_rec *rec)
{
	ulong انحراف;

	انحراف = (خالی *)rec - (خالی *)hdr;
	/*
	 * The data section of next TE should start from an address aligned
	 * to 1 << hdr->align_log2.
	 */
	انحراف += rec_hdr_سائز(rec) + rec->سائز;
	انحراف = باری_اوپر(انحراف + rec_hdr_سائز(rec), 1 << hdr->align_log2);
	انحراف -= rec_hdr_سائز(rec);

	واپس انحراف;
}

ساکن ساخت bloblist_rec *bloblist_اگلا_blob(ساخت bloblist_hdr *hdr,
					       ساخت bloblist_rec *rec)
{
	ulong انحراف = bloblist_blob_انتہا_ofs(hdr, rec);

	اگر (انحراف >= hdr->مستعمل_سائز)
		واپس تہی;
	واپس (ساخت bloblist_rec *)((خالی *)hdr + انحراف);
}

#تعریف_کرو foreach_rec(_rec, _hdr) \
	برائے (_rec = bloblist_پہلا_blob(_hdr); \
	     _rec; \
	     _rec = bloblist_اگلا_blob(_hdr, _rec))

ساکن ساخت bloblist_rec *bloblist_findrec(uint لیبل)
{
	ساخت bloblist_hdr *hdr = gd->bloblist;
	ساخت bloblist_rec *rec;

	اگر (!hdr)
		واپس تہی;

	foreach_rec(rec, hdr) {
		اگر (rec_لیبل(rec) == لیبل)
			واپس rec;
	}

	واپس تہی;
}

ساکن صحیح bloblist_addrec(uint لیبل, صحیح سائز, صحیح align_log2,
			   ساخت bloblist_rec **recp)
{
	ساخت bloblist_hdr *hdr = gd->bloblist;
	ساخت bloblist_rec *rec;
	phys_addr_t ڈیٹا_آغاز, aligned_آغاز;
	phys_سائز_t new_alloced;

	اگر (!align_log2)
		align_log2 = BLOBLIST_BLOB_ALIGN_LOG2;

	/* Figure out where the new data will start */
	ڈیٹا_آغاز = map_مقصد_sysmem(hdr) + hdr->مستعمل_سائز + حجم(*rec);

	/* Align the address and then calculate the offset from used size */
	aligned_آغاز = ALIGN(ڈیٹا_آغاز, 1U << align_log2) - ڈیٹا_آغاز;

	/* If we need to create a dummy record, create it */
	اگر (aligned_آغاز) {
		صحیح void_سائز = aligned_آغاز - حجم(*rec);
		ساخت bloblist_rec *vrec;
		صحیح ret;

		ret = bloblist_addrec(BLOBLISTT_VOID, void_سائز, 0, &vrec);
		اگر (ret)
			واپس log_پیغام_ret("void", ret);

		/* start the record after that */
		ڈیٹا_آغاز = map_مقصد_sysmem(hdr) + hdr->مستعمل_سائز + حجم(*vrec);
	}

	/* Calculate the new allocated total */
	new_alloced = ڈیٹا_آغاز - map_مقصد_sysmem(hdr) +
		ALIGN(سائز, 1U << align_log2);

	اگر (new_alloced > hdr->کل_سائز) {
		log_err("Failed to allocate %x bytes\n", سائز);
		log_err("Used size=%x, total size=%x\n",
			hdr->مستعمل_سائز, hdr->کل_سائز);
		واپس log_پیغام_ret("bloblist add", -ENOSPC);
	}
	rec = (خالی *)hdr + hdr->مستعمل_سائز;

	rec->لیبل_and_hdr_سائز = لیبل | حجم(*rec) << BLOBLISTR_HDR_سائز_سرکاؤ;
	rec->سائز = سائز;

	/* Zero the record data */
	memset((خالی *)rec + rec_hdr_سائز(rec), '\0', rec->سائز);

	hdr->مستعمل_سائز = new_alloced;
	*recp = rec;

	واپس 0;
}

ساکن صحیح bloblist_ensurerec(uint لیبل, ساخت bloblist_rec **recp, صحیح سائز,
			      صحیح align_log2)
{
	ساخت bloblist_rec *rec;

	rec = bloblist_findrec(لیبل);
	اگر (rec) {
		اگر (سائز && سائز != rec->سائز) {
			*recp = rec;
			واپس -ESPIPE;
		}
	} ورنہ {
		صحیح ret;

		ret = bloblist_addrec(لیبل, سائز, align_log2, &rec);
		اگر (ret)
			واپس ret;
	}
	*recp = rec;

	واپس 0;
}

ساکن صحیح bloblist_get_blob_ڈیٹا_انحراف(uint لیبل)
{
	انتخاب (لیبل) {
	حالت BLOBLISTT_FDT_OVERLAY:
		واپس حجم(ساخت dto_blob_hdr);
	/*
	 * return the data offset if it is not following the blob
	 * header immediately.
	 */
	}
	واپس 0;
}

خالی *bloblist_ڈھونڈو(uint لیبل, صحیح سائز)
{
	خالی *blob = تہی;
	صحیح blob_سائز;

	blob = bloblist_get_blob(لیبل, &blob_سائز);

	اگر (سائز && سائز != blob_سائز)
		واپس تہی;

	واپس blob;
}

خالی *bloblist_get_blob(uint لیبل, صحیح *sizep)
{
	ساخت bloblist_rec *rec;

	rec = bloblist_findrec(لیبل);
	اگر (!rec)
		واپس تہی;

	*sizep = rec->سائز;

	واپس (خالی *)rec + rec_hdr_سائز(rec);
}

صحیح bloblist_apply_blobs(uint لیبل, صحیح (*func)(خالی **ڈیٹا, صحیح سائز))
{
	ساخت bloblist_hdr *hdr = gd->bloblist;
	ساخت bloblist_rec *rec;

	اگر (!func || !hdr)
		واپس -ENOENT;

	foreach_rec(rec, hdr) {
		/* Apply all blobs with the specified tag */
		اگر (rec_لیبل(rec) == لیبل) {
			صحیح ret;
			صحیح لیبل = rec_لیبل(rec);
			خالی *blob = (خالی *)rec + rec_hdr_سائز(rec);
			صحیح dat_off = bloblist_get_blob_ڈیٹا_انحراف(لیبل);

			blob += dat_off;
			ret = func(&blob, rec->سائز - dat_off);
			اگر (ret) {
				log_err("Failed to apply blob with tag %d\n",
					لیبل);
				واپس ret;
			}

			rec = rec_from_blob(blob - dat_off);
			اگر (!rec) {
				log_err("Blob corrupted\n");
				واپس -ENOENT;
			}

			/* Mark applied blob record as void */
			void_blob(rec);
		}
	}

	واپس 0;
}

خالی *bloblist_جوڑو(uint لیبل, صحیح سائز, صحیح align_log2)
{
	ساخت bloblist_rec *rec;

	اگر (bloblist_addrec(لیبل, سائز, align_log2, &rec))
		واپس تہی;

	واپس (خالی *)rec + rec_hdr_سائز(rec);
}

صحیح bloblist_ensure_سائز(uint لیبل, صحیح سائز, صحیح align_log2, خالی **blobp)
{
	ساخت bloblist_rec *rec;
	صحیح ret;

	ret = bloblist_ensurerec(لیبل, &rec, سائز, align_log2);
	اگر (ret)
		واپس ret;
	*blobp = (خالی *)rec + rec_hdr_سائز(rec);

	واپس 0;
}

خالی *bloblist_ensure(uint لیبل, صحیح سائز)
{
	ساخت bloblist_rec *rec;

	اگر (bloblist_ensurerec(لیبل, &rec, سائز, 0))
		واپس تہی;

	واپس (خالی *)rec + rec_hdr_سائز(rec);
}

صحیح bloblist_ensure_سائز_ret(uint لیبل, صحیح *sizep, خالی **blobp)
{
	ساخت bloblist_rec *rec;
	صحیح ret;

	ret = bloblist_ensurerec(لیبل, &rec, *sizep, 0);
	اگر (ret == -ESPIPE)
		*sizep = rec->سائز;
	ورنہ اگر (ret)
		واپس ret;
	*blobp = (خالی *)rec + rec_hdr_سائز(rec);

	واپس 0;
}

ساکن صحیح bloblist_resize_rec(ساخت bloblist_hdr *hdr,
			       ساخت bloblist_rec *rec,
			       صحیح new_سائز)
{
	صحیح expand_by;	/* Number of bytes to expand by (-ve to contract) */
	صحیح new_alloced;
	ulong اگلا_ofs;	/* Offset of the record after @rec */

	expand_by = ALIGN(new_سائز - rec->سائز, BLOBLIST_BLOB_ALIGN);
	new_alloced = ALIGN(hdr->مستعمل_سائز + expand_by, BLOBLIST_BLOB_ALIGN);
	اگر (new_سائز < 0) {
		log_debug("Attempt to shrink blob size below 0 (%x)\n",
			  new_سائز);
		واپس log_پیغام_ret("size", -EINVAL);
	}
	اگر (new_alloced > hdr->کل_سائز) {
		log_err("Failed to allocate %x bytes\n", new_سائز);
		log_err("Used size=%x, total size=%x\n",
			hdr->مستعمل_سائز, hdr->کل_سائز);
		واپس log_پیغام_ret("alloc", -ENOSPC);
	}

	/* Move the following blobs up or down, if this is not the last */
	اگلا_ofs = bloblist_blob_انتہا_ofs(hdr, rec);
	اگر (اگلا_ofs != hdr->مستعمل_سائز) {
		memmove((خالی *)hdr + اگلا_ofs + expand_by,
			(خالی *)hdr + اگلا_ofs, hdr->مستعمل_سائز - اگلا_ofs);
	}
	hdr->مستعمل_سائز = new_alloced;

	/* Zero the new part of the blob */
	اگر (expand_by > 0) {
		memset((خالی *)rec + rec_hdr_سائز(rec) + rec->سائز, '\0',
		       new_سائز - rec->سائز);
	}

	/* Update the size of this blob */
	rec->سائز = new_سائز;

	واپس 0;
}

صحیح bloblist_resize(uint لیبل, صحیح new_سائز)
{
	ساخت bloblist_hdr *hdr = gd->bloblist;
	ساخت bloblist_rec *rec;
	صحیح ret;

	rec = bloblist_findrec(لیبل);
	اگر (!rec)
		واپس log_پیغام_ret("find", -ENOENT);
	ret = bloblist_resize_rec(hdr, rec, new_سائز);
	اگر (ret)
		واپس log_پیغام_ret("resize", ret);

	واپس 0;
}

ساکن u32 bloblist_calc_chksum(ساخت bloblist_hdr *hdr)
{
	u8 chksum;

	chksum = جدول_compute_checksum(hdr, hdr->مستعمل_سائز);
	chksum += hdr->chksum;

	واپس chksum;
}

صحیح bloblist_new(ulong addr, uint سائز, uint flags, uint align_log2)
{
	ساخت bloblist_hdr *hdr;

	اگر (سائز < حجم(*hdr))
		واپس log_ret(-ENOSPC);
	اگر (addr & (BLOBLIST_ALIGN - 1))
		واپس log_ret(-EFAULT);
	hdr = map_sysmem(addr, سائز);
	memset(hdr, '\0', حجم(*hdr));
	hdr->version = BLOBLIST_نسخہ;
	hdr->hdr_سائز = حجم(*hdr);
	hdr->flags = flags;
	hdr->magic = BLOBLIST_MAGIC;
	hdr->مستعمل_سائز = hdr->hdr_سائز;
	hdr->کل_سائز = سائز;
	hdr->align_log2 = align_log2 ? align_log2 : BLOBLIST_BLOB_ALIGN_LOG2;
	hdr->chksum = 0;
	gd->bloblist = hdr;
	gd->flags |= GD_FLG_BLOBLIST_HANDOFF;

	واپس 0;
}

صحیح bloblist_check(ulong addr, uint سائز)
{
	ساخت bloblist_hdr *hdr;
	u32 chksum;

	hdr = map_sysmem(addr, حجم(*hdr));
	اگر (hdr->magic != BLOBLIST_MAGIC)
		واپس log_پیغام_ret("Bad magic", -ENOENT);
	اگر (hdr->version != BLOBLIST_نسخہ)
		واپس log_پیغام_ret("Bad version", -EPROTONOSUPPORT);
	اگر (!hdr->کل_سائز || (سائز && hdr->کل_سائز > سائز))
		واپس log_پیغام_ret("Bad total size", -EFBIG);
	اگر (hdr->مستعمل_سائز > hdr->کل_سائز)
		واپس log_پیغام_ret("Bad used size", -ENOENT);
	اگر (hdr->hdr_سائز != حجم(ساخت bloblist_hdr))
		واپس log_پیغام_ret("Bad header size", -ENOENT);

	chksum = bloblist_calc_chksum(hdr);
	اگر (hdr->chksum != chksum) {
		log_err("Checksum %x != %x\n", hdr->chksum, chksum);
		واپس log_پیغام_ret("Bad checksum", -EIO);
	}
	gd->bloblist = hdr;
	gd->flags |= GD_FLG_BLOBLIST_HANDOFF;

	واپس 0;
}

صحیح bloblist_اختتام(خالی)
{
	ساخت bloblist_hdr *hdr = gd->bloblist;

	hdr->chksum = bloblist_calc_chksum(hdr);
	log_debug("Finished bloblist size %lx at %lx\n", (ulong)hdr->مستعمل_سائز,
		  (ulong)map_مقصد_sysmem(hdr));

	واپس 0;
}

ulong bloblist_get_بنیاد(خالی)
{
	واپس map_مقصد_sysmem(gd->bloblist);
}

ulong bloblist_get_سائز(خالی)
{
	ساخت bloblist_hdr *hdr = gd->bloblist;

	واپس hdr->مستعمل_سائز;
}

ulong bloblist_get_کل_سائز(خالی)
{
	ساخت bloblist_hdr *hdr = gd->bloblist;

	واپس hdr->کل_سائز;
}

خالی bloblist_get_stats(ulong *basep, ulong *tsizep, ulong *usizep)
{
	ساخت bloblist_hdr *hdr = gd->bloblist;

	*basep = map_مقصد_sysmem(gd->bloblist);
	*tsizep = hdr->کل_سائز;
	*usizep = hdr->مستعمل_سائز;
}

ساکن خالی show_قدر(ثابت حرف *prompt, ulong قدر)
{
	لکھو("%s:%*s %-5lx  ", prompt, 10 - (صحیح)لڑی_لمبائی(prompt), "", قدر);
	print_سائز(قدر, "\n");
}

خالی bloblist_show_stats(خالی)
{
	ulong بنیاد, tsize, usize;

	bloblist_get_stats(&بنیاد, &tsize, &usize);
	لکھو("base:       %lx\n", بنیاد);
	show_قدر("total size", tsize);
	show_قدر("used size", usize);
	show_قدر("free", tsize - usize);
}

خالی bloblist_show_list(خالی)
{
	ساخت bloblist_hdr *hdr = gd->bloblist;
	ساخت bloblist_rec *rec;

	لکھو("%-8s  %8s   Tag Name\n", "Address", "Size");
	برائے (rec = bloblist_پہلا_blob(hdr); rec;
	     rec = bloblist_اگلا_blob(hdr, rec)) {
		لکھو("%08lx  %8x  %4x %s\n",
		       (ulong)map_مقصد_sysmem((خالی *)rec + rec_hdr_سائز(rec)),
		       rec->سائز, rec_لیبل(rec),
		       bloblist_لیبل_name(rec_لیبل(rec)));
	}
}

صحیح bloblist_reloc(خالی *مقصد, uint مقصد_سائز)
{
	ساخت bloblist_hdr *hdr;

	اگر (!مقصد_سائز)
		واپس 0;

	اگر (مقصد_سائز < gd->bloblist->کل_سائز)
		واپس -ENOSPC;

	حافظہ_نقل(مقصد, gd->bloblist, gd->bloblist->کل_سائز);
	hdr = مقصد;
	hdr->کل_سائز = مقصد_سائز;
	gd->bloblist = مقصد;

	واپس 0;
}

/*
 * Weak default function for getting bloblist from boot args.
 */
صحیح __weak xferlist_from_boot_arg(ulong __always_unused *addr)
{
	واپس -ENOENT;
}

bool bloblist_exists(خالی)
{
	صحیح ret;
	ulong addr = 0;

	/* Check if a valid transfer list passed in */
	اگر (!xferlist_from_boot_arg(&addr))
		جاؤ ملا;

	/*
	 * If Firmware Handoff is mandatory but no transfer list is
	 * observed, report it as an error.
	 */
	اگر (IS_ENABLED(CONFIG_BLOBLIST_PASSAGE_MANDATORY))
		واپس false;

	/*
	 * We have checked for a valid transfer list being passed. At this
	 * point, if we do not have a fixed address for the bloblist, we cannot
	 * be provided with one.
	 */
	اگر (xpl_is_پہلا_phase() || !IS_ENABLED(CONFIG_BLOBLIST_FIXED))
		واپس false;

	/*
	 * Check for a valid list as the configured address.
	 */
	addr = IF_ENABLED_INT(CONFIG_BLOBLIST_FIXED,
			      CONFIG_BLOBLIST_ADDR);
	ret = bloblist_check(addr, CONFIG_BLOBLIST_سائز);
	اگر (!ret)
		جاؤ ملا;

	log_debug("Bloblist at %lx not found (err=%d)\n", addr, ret);
	واپس false;

ملا:
#اگر_تعریف_شدہ DEBUG
	bloblist_show_stats();
	bloblist_show_list();
#اگر_ختم
	واپس true;
}

صحیح bloblist_init(خالی)
{
	صحیح ret;
	ulong addr = 0, سائز = CONFIG_BLOBLIST_سائز;

	اگر (gd->flags & GD_FLG_BLOBLIST_HANDOFF) {
		log_debug("Found existing bloblist size %x at %p\n",
			  gd->bloblist->کل_سائز, gd->bloblist);
		واپس 0;
	}

	/*
	 * If Firmware Handoff is mandatory but no transfer list has been
	 * observed by fdtdec_setup, report it as an error.
	 */
	اگر (IS_ENABLED(CONFIG_BLOBLIST_PASSAGE_MANDATORY))
		واپس -ENOENT;

	/*
	 * If we don't have an existing bloblist, we either need
	 * to allocate one now, or initialize the fixed address
	 * space as a bloblist.
	 */
	اگر (CONFIG_IS_ENABLED(BLOBLIST_ALLOC)) {
		خالی *ptr = memalign(BLOBLIST_ALIGN, سائز);

		اگر (!ptr)
			واپس log_پیغام_ret("alloc", -ENOMEM);
		addr = map_مقصد_sysmem(ptr);
	} ورنہ
		addr = IF_ENABLED_INT(CONFIG_BLOBLIST_FIXED,
				      CONFIG_BLOBLIST_ADDR);

	log_debug("Creating new bloblist size %lx at %lx\n", سائز,
		  addr);
	ret = bloblist_new(addr, سائز, 0, 0);
	اگر (ret)
		واپس log_پیغام_ret("ini", ret);

	واپس 0;
}

صحیح bloblist_check_reg_conv(ulong rfdt, ulong rzero, ulong rsig, ulong xlist)
{
	u64 version = BLOBLIST_REGCONV_VER;
	ulong sigval;
	صحیح ret;

	اگر ((IS_ENABLED(CONFIG_64_ثنائی_ہندسہ) && !IS_ENABLED(CONFIG_SPL_BUILD)) ||
			(IS_ENABLED(CONFIG_SPL_64_ثنائی_ہندسہ) && IS_ENABLED(CONFIG_SPL_BUILD))) {
		sigval = ((BLOBLIST_MAGIC & ((1ULL << BLOBLIST_REGCONV_سرکاؤ_64) - 1)) |
			 ((version  & BLOBLIST_REGCONV_نقاب) << BLOBLIST_REGCONV_سرکاؤ_64));
	} ورنہ {
		sigval = ((BLOBLIST_MAGIC & ((1UL << BLOBLIST_REGCONV_سرکاؤ_32) - 1)) |
			 ((version  & BLOBLIST_REGCONV_نقاب) << BLOBLIST_REGCONV_سرکاؤ_32));
	}

	اگر (rzero || rsig != sigval)
		واپس -EIO;

	ret = bloblist_check(xlist, 0);
	اگر (ret)
		واپس ret;

	اگر (rfdt != (ulong)bloblist_ڈھونڈو(BLOBLISTT_CONTROL_FDT, 0)) {
		/* Remove this bloblist from gd */
		gd->bloblist = تہی;
		gd->flags &= ~GD_FLG_BLOBLIST_HANDOFF;
		واپس -EIO;
	}

	واپس 0;
}
