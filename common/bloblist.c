// SPDX-License-Identifier: GPL-2.0+ BSD-3-Clause
/*
 * Copyright 2018 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#تعریف_کرو LOG_CATEGORY	LOGC_BLOBLIST

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

DECLARE_GLOBAL_انکڑے_PTR;

ساكن بنية ٹیگ_name {
	تعداد bloblist_ٹیگ_t ٹیگ;
	ثابت حرف *name;
} ٹیگ_name[] = {
	{ BLOBLISTT_VOID, "(void)" },

	/* BLOBLISTT_AREA_FIRMWARE_TOP */
	{ BLOBLISTT_CONTROL_FDT, "Control FDT" },
	{ BLOBLISTT_HOB_بلاک, "HOB block" },
	{ BLOBLISTT_HOB_LIST, "HOB list" },
	{ BLOBLISTT_ACPI_TABLES, "ACPI tables for x86" },
	{ BLOBLISTT_TPM_EVLOG, "TPM event log defined by TCG EFI" },
	{ BLOBLISTT_TPM_CRB_ادھار, "TPM Command Response Buffer address" },
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

ثابت حرف *bloblist_ٹیگ_name(تعداد bloblist_ٹیگ_t ٹیگ)
{
	صحيح i;

	لكل (i = 0; i < ARRAY_ناپ(ٹیگ_name); i++) {
		إذا (ٹیگ_name[i].ٹیگ == ٹیگ)
			أرجع ٹیگ_name[i].name;
	}

	أرجع "invalid";
}

ساكن بنية bloblist_rec *bloblist_پہلا_blob(بنية bloblist_hdr *hdr)
{
	إذا (hdr->ورتیا_ناپ <= hdr->hdr_ناپ)
		أرجع عدم;
	أرجع (بنية bloblist_rec *)((فراغ *)hdr + hdr->hdr_ناپ);
}

ساكن مضمن uint rec_hdr_ناپ(بنية bloblist_rec *rec)
{
	أرجع (rec->ٹیگ_and_hdr_ناپ & BLOBLISTR_HDR_ناپ_نقاب) >>
		BLOBLISTR_HDR_ناپ_کھسکاؤ;
}

ساكن مضمن uint rec_ٹیگ(بنية bloblist_rec *rec)
{
	أرجع (rec->ٹیگ_and_hdr_ناپ & BLOBLISTR_ٹیگ_نقاب) >>
		BLOBLISTR_ٹیگ_کھسکاؤ;
}

ساكن مضمن فراغ void_blob(بنية bloblist_rec *rec)
{
	إذا (rec_ٹیگ(rec) == BLOBLISTT_VOID)
		أرجع;
	rec->ٹیگ_and_hdr_ناپ = BLOBLISTT_VOID |
				حجم(*rec) << BLOBLISTR_HDR_ناپ_کھسکاؤ;
}

ساكن مضمن بنية bloblist_rec *rec_from_blob(فراغ *blob)
{
	أرجع (blob - حجم(بنية bloblist_rec));
}

ساكن ulong bloblist_blob_سرا_ofs(بنية bloblist_hdr *hdr,
				   بنية bloblist_rec *rec)
{
	ulong وتھ;

	وتھ = (فراغ *)rec - (فراغ *)hdr;
	/*
	 * The data section of next TE should start from an address aligned
	 * to 1 << hdr->align_log2.
	 */
	وتھ += rec_hdr_ناپ(rec) + rec->ناپ;
	وتھ = گیڑا_اتے(وتھ + rec_hdr_ناپ(rec), 1 << hdr->align_log2);
	وتھ -= rec_hdr_ناپ(rec);

	أرجع وتھ;
}

ساكن بنية bloblist_rec *bloblist_اگلا_blob(بنية bloblist_hdr *hdr,
					       بنية bloblist_rec *rec)
{
	ulong وتھ = bloblist_blob_سرا_ofs(hdr, rec);

	إذا (وتھ >= hdr->ورتیا_ناپ)
		أرجع عدم;
	أرجع (بنية bloblist_rec *)((فراغ *)hdr + وتھ);
}

#تعریف_کرو foreach_rec(_rec, _hdr) \
	لكل (_rec = bloblist_پہلا_blob(_hdr); \
	     _rec; \
	     _rec = bloblist_اگلا_blob(_hdr, _rec))

ساكن بنية bloblist_rec *bloblist_findrec(uint ٹیگ)
{
	بنية bloblist_hdr *hdr = gd->bloblist;
	بنية bloblist_rec *rec;

	إذا (!hdr)
		أرجع عدم;

	foreach_rec(rec, hdr) {
		إذا (rec_ٹیگ(rec) == ٹیگ)
			أرجع rec;
	}

	أرجع عدم;
}

ساكن صحيح bloblist_addrec(uint ٹیگ, صحيح ناپ, صحيح align_log2,
			   بنية bloblist_rec **recp)
{
	بنية bloblist_hdr *hdr = gd->bloblist;
	بنية bloblist_rec *rec;
	phys_addr_t انکڑے_شروع, aligned_شروع;
	phys_ناپ_t new_alloced;

	إذا (!align_log2)
		align_log2 = BLOBLIST_BLOB_ALIGN_LOG2;

	/* Figure out where the new data will start */
	انکڑے_شروع = map_منزل_sysmem(hdr) + hdr->ورتیا_ناپ + حجم(*rec);

	/* Align the address and then calculate the offset from used size */
	aligned_شروع = ALIGN(انکڑے_شروع, 1U << align_log2) - انکڑے_شروع;

	/* If we need to create a dummy record, create it */
	إذا (aligned_شروع) {
		صحيح void_ناپ = aligned_شروع - حجم(*rec);
		بنية bloblist_rec *vrec;
		صحيح ret;

		ret = bloblist_addrec(BLOBLISTT_VOID, void_ناپ, 0, &vrec);
		إذا (ret)
			أرجع log_پیغام_ret("void", ret);

		/* start the record after that */
		انکڑے_شروع = map_منزل_sysmem(hdr) + hdr->ورتیا_ناپ + حجم(*vrec);
	}

	/* Calculate the new allocated total */
	new_alloced = انکڑے_شروع - map_منزل_sysmem(hdr) +
		ALIGN(ناپ, 1U << align_log2);

	إذا (new_alloced > hdr->کل_ناپ) {
		log_err("Failed to allocate %x bytes\n", ناپ);
		log_err("Used size=%x, total size=%x\n",
			hdr->ورتیا_ناپ, hdr->کل_ناپ);
		أرجع log_پیغام_ret("bloblist add", -ENOSPC);
	}
	rec = (فراغ *)hdr + hdr->ورتیا_ناپ;

	rec->ٹیگ_and_hdr_ناپ = ٹیگ | حجم(*rec) << BLOBLISTR_HDR_ناپ_کھسکاؤ;
	rec->ناپ = ناپ;

	/* Zero the record data */
	memset((فراغ *)rec + rec_hdr_ناپ(rec), '\0', rec->ناپ);

	hdr->ورتیا_ناپ = new_alloced;
	*recp = rec;

	أرجع 0;
}

ساكن صحيح bloblist_ensurerec(uint ٹیگ, بنية bloblist_rec **recp, صحيح ناپ,
			      صحيح align_log2)
{
	بنية bloblist_rec *rec;

	rec = bloblist_findrec(ٹیگ);
	إذا (rec) {
		إذا (ناپ && ناپ != rec->ناپ) {
			*recp = rec;
			أرجع -ESPIPE;
		}
	} وإلا {
		صحيح ret;

		ret = bloblist_addrec(ٹیگ, ناپ, align_log2, &rec);
		إذا (ret)
			أرجع ret;
	}
	*recp = rec;

	أرجع 0;
}

ساكن صحيح bloblist_get_blob_انکڑے_وتھ(uint ٹیگ)
{
	بدل (ٹیگ) {
	حالة BLOBLISTT_FDT_OVERLAY:
		أرجع حجم(بنية dto_blob_hdr);
	/*
	 * return the data offset if it is not following the blob
	 * header immediately.
	 */
	}
	أرجع 0;
}

فراغ *bloblist_لبھو(uint ٹیگ, صحيح ناپ)
{
	فراغ *blob = عدم;
	صحيح blob_ناپ;

	blob = bloblist_get_blob(ٹیگ, &blob_ناپ);

	إذا (ناپ && ناپ != blob_ناپ)
		أرجع عدم;

	أرجع blob;
}

فراغ *bloblist_get_blob(uint ٹیگ, صحيح *sizep)
{
	بنية bloblist_rec *rec;

	rec = bloblist_findrec(ٹیگ);
	إذا (!rec)
		أرجع عدم;

	*sizep = rec->ناپ;

	أرجع (فراغ *)rec + rec_hdr_ناپ(rec);
}

صحيح bloblist_apply_blobs(uint ٹیگ, صحيح (*func)(فراغ **انکڑے, صحيح ناپ))
{
	بنية bloblist_hdr *hdr = gd->bloblist;
	بنية bloblist_rec *rec;

	إذا (!func || !hdr)
		أرجع -ENOENT;

	foreach_rec(rec, hdr) {
		/* Apply all blobs with the specified tag */
		إذا (rec_ٹیگ(rec) == ٹیگ) {
			صحيح ret;
			صحيح ٹیگ = rec_ٹیگ(rec);
			فراغ *blob = (فراغ *)rec + rec_hdr_ناپ(rec);
			صحيح dat_off = bloblist_get_blob_انکڑے_وتھ(ٹیگ);

			blob += dat_off;
			ret = func(&blob, rec->ناپ - dat_off);
			إذا (ret) {
				log_err("Failed to apply blob with tag %d\n",
					ٹیگ);
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

فراغ *bloblist_جوڑو(uint ٹیگ, صحيح ناپ, صحيح align_log2)
{
	بنية bloblist_rec *rec;

	إذا (bloblist_addrec(ٹیگ, ناپ, align_log2, &rec))
		أرجع عدم;

	أرجع (فراغ *)rec + rec_hdr_ناپ(rec);
}

صحيح bloblist_ensure_ناپ(uint ٹیگ, صحيح ناپ, صحيح align_log2, فراغ **blobp)
{
	بنية bloblist_rec *rec;
	صحيح ret;

	ret = bloblist_ensurerec(ٹیگ, &rec, ناپ, align_log2);
	إذا (ret)
		أرجع ret;
	*blobp = (فراغ *)rec + rec_hdr_ناپ(rec);

	أرجع 0;
}

فراغ *bloblist_ensure(uint ٹیگ, صحيح ناپ)
{
	بنية bloblist_rec *rec;

	إذا (bloblist_ensurerec(ٹیگ, &rec, ناپ, 0))
		أرجع عدم;

	أرجع (فراغ *)rec + rec_hdr_ناپ(rec);
}

صحيح bloblist_ensure_ناپ_ret(uint ٹیگ, صحيح *sizep, فراغ **blobp)
{
	بنية bloblist_rec *rec;
	صحيح ret;

	ret = bloblist_ensurerec(ٹیگ, &rec, *sizep, 0);
	إذا (ret == -ESPIPE)
		*sizep = rec->ناپ;
	وإلا إذا (ret)
		أرجع ret;
	*blobp = (فراغ *)rec + rec_hdr_ناپ(rec);

	أرجع 0;
}

ساكن صحيح bloblist_resize_rec(بنية bloblist_hdr *hdr,
			       بنية bloblist_rec *rec,
			       صحيح new_ناپ)
{
	صحيح expand_by;	/* Number of bytes to expand by (-ve to contract) */
	صحيح new_alloced;
	ulong اگلا_ofs;	/* Offset of the record after @rec */

	expand_by = ALIGN(new_ناپ - rec->ناپ, BLOBLIST_BLOB_ALIGN);
	new_alloced = ALIGN(hdr->ورتیا_ناپ + expand_by, BLOBLIST_BLOB_ALIGN);
	إذا (new_ناپ < 0) {
		log_debug("Attempt to shrink blob size below 0 (%x)\n",
			  new_ناپ);
		أرجع log_پیغام_ret("size", -EINVAL);
	}
	إذا (new_alloced > hdr->کل_ناپ) {
		log_err("Failed to allocate %x bytes\n", new_ناپ);
		log_err("Used size=%x, total size=%x\n",
			hdr->ورتیا_ناپ, hdr->کل_ناپ);
		أرجع log_پیغام_ret("alloc", -ENOSPC);
	}

	/* Move the following blobs up or down, if this is not the last */
	اگلا_ofs = bloblist_blob_سرا_ofs(hdr, rec);
	إذا (اگلا_ofs != hdr->ورتیا_ناپ) {
		memmove((فراغ *)hdr + اگلا_ofs + expand_by,
			(فراغ *)hdr + اگلا_ofs, hdr->ورتیا_ناپ - اگلا_ofs);
	}
	hdr->ورتیا_ناپ = new_alloced;

	/* Zero the new part of the blob */
	إذا (expand_by > 0) {
		memset((فراغ *)rec + rec_hdr_ناپ(rec) + rec->ناپ, '\0',
		       new_ناپ - rec->ناپ);
	}

	/* Update the size of this blob */
	rec->ناپ = new_ناپ;

	أرجع 0;
}

صحيح bloblist_resize(uint ٹیگ, صحيح new_ناپ)
{
	بنية bloblist_hdr *hdr = gd->bloblist;
	بنية bloblist_rec *rec;
	صحيح ret;

	rec = bloblist_findrec(ٹیگ);
	إذا (!rec)
		أرجع log_پیغام_ret("find", -ENOENT);
	ret = bloblist_resize_rec(hdr, rec, new_ناپ);
	إذا (ret)
		أرجع log_پیغام_ret("resize", ret);

	أرجع 0;
}

ساكن u32 bloblist_calc_chksum(بنية bloblist_hdr *hdr)
{
	u8 chksum;

	chksum = جدول_compute_checksum(hdr, hdr->ورتیا_ناپ);
	chksum += hdr->chksum;

	أرجع chksum;
}

صحيح bloblist_new(ulong addr, uint ناپ, uint flags, uint align_log2)
{
	بنية bloblist_hdr *hdr;

	إذا (ناپ < حجم(*hdr))
		أرجع log_ret(-ENOSPC);
	إذا (addr & (BLOBLIST_ALIGN - 1))
		أرجع log_ret(-EFAULT);
	hdr = map_sysmem(addr, ناپ);
	memset(hdr, '\0', حجم(*hdr));
	hdr->version = BLOBLIST_نسخہ;
	hdr->hdr_ناپ = حجم(*hdr);
	hdr->flags = flags;
	hdr->magic = BLOBLIST_MAGIC;
	hdr->ورتیا_ناپ = hdr->hdr_ناپ;
	hdr->کل_ناپ = ناپ;
	hdr->align_log2 = align_log2 ? align_log2 : BLOBLIST_BLOB_ALIGN_LOG2;
	hdr->chksum = 0;
	gd->bloblist = hdr;
	gd->flags |= GD_FLG_BLOBLIST_HANDOFF;

	أرجع 0;
}

صحيح bloblist_check(ulong addr, uint ناپ)
{
	بنية bloblist_hdr *hdr;
	u32 chksum;

	hdr = map_sysmem(addr, حجم(*hdr));
	إذا (hdr->magic != BLOBLIST_MAGIC)
		أرجع log_پیغام_ret("Bad magic", -ENOENT);
	إذا (hdr->version != BLOBLIST_نسخہ)
		أرجع log_پیغام_ret("Bad version", -EPROTONOSUPPORT);
	إذا (!hdr->کل_ناپ || (ناپ && hdr->کل_ناپ > ناپ))
		أرجع log_پیغام_ret("Bad total size", -EFBIG);
	إذا (hdr->ورتیا_ناپ > hdr->کل_ناپ)
		أرجع log_پیغام_ret("Bad used size", -ENOENT);
	إذا (hdr->hdr_ناپ != حجم(بنية bloblist_hdr))
		أرجع log_پیغام_ret("Bad header size", -ENOENT);

	chksum = bloblist_calc_chksum(hdr);
	إذا (hdr->chksum != chksum) {
		log_err("Checksum %x != %x\n", hdr->chksum, chksum);
		أرجع log_پیغام_ret("Bad checksum", -EIO);
	}
	gd->bloblist = hdr;
	gd->flags |= GD_FLG_BLOBLIST_HANDOFF;

	أرجع 0;
}

صحيح bloblist_انت(فراغ)
{
	بنية bloblist_hdr *hdr = gd->bloblist;

	hdr->chksum = bloblist_calc_chksum(hdr);
	log_debug("Finished bloblist size %lx at %lx\n", (ulong)hdr->ورتیا_ناپ,
		  (ulong)map_منزل_sysmem(hdr));

	أرجع 0;
}

ulong bloblist_get_ادھار(فراغ)
{
	أرجع map_منزل_sysmem(gd->bloblist);
}

ulong bloblist_get_ناپ(فراغ)
{
	بنية bloblist_hdr *hdr = gd->bloblist;

	أرجع hdr->ورتیا_ناپ;
}

ulong bloblist_get_کل_ناپ(فراغ)
{
	بنية bloblist_hdr *hdr = gd->bloblist;

	أرجع hdr->کل_ناپ;
}

فراغ bloblist_get_stats(ulong *basep, ulong *tsizep, ulong *usizep)
{
	بنية bloblist_hdr *hdr = gd->bloblist;

	*basep = map_منزل_sysmem(gd->bloblist);
	*tsizep = hdr->کل_ناپ;
	*usizep = hdr->ورتیا_ناپ;
}

ساكن فراغ show_قدر(ثابت حرف *prompt, ulong قدر)
{
	لکھو("%s:%*s %-5lx  ", prompt, 10 - (صحيح)لڑی_لمبائی(prompt), "", قدر);
	print_ناپ(قدر, "\n");
}

فراغ bloblist_show_stats(فراغ)
{
	ulong ادھار, tsize, usize;

	bloblist_get_stats(&ادھار, &tsize, &usize);
	لکھو("base:       %lx\n", ادھار);
	show_قدر("total size", tsize);
	show_قدر("used size", usize);
	show_قدر("free", tsize - usize);
}

فراغ bloblist_show_list(فراغ)
{
	بنية bloblist_hdr *hdr = gd->bloblist;
	بنية bloblist_rec *rec;

	لکھو("%-8s  %8s   Tag Name\n", "Address", "Size");
	لكل (rec = bloblist_پہلا_blob(hdr); rec;
	     rec = bloblist_اگلا_blob(hdr, rec)) {
		لکھو("%08lx  %8x  %4x %s\n",
		       (ulong)map_منزل_sysmem((فراغ *)rec + rec_hdr_ناپ(rec)),
		       rec->ناپ, rec_ٹیگ(rec),
		       bloblist_ٹیگ_name(rec_ٹیگ(rec)));
	}
}

صحيح bloblist_reloc(فراغ *منزل, uint منزل_ناپ)
{
	بنية bloblist_hdr *hdr;

	إذا (!منزل_ناپ)
		أرجع 0;

	إذا (منزل_ناپ < gd->bloblist->کل_ناپ)
		أرجع -ENOSPC;

	حافظہ_نقل(منزل, gd->bloblist, gd->bloblist->کل_ناپ);
	hdr = منزل;
	hdr->کل_ناپ = منزل_ناپ;
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
		اذهب لبھیا;

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
	إذا (xpl_is_پہلا_phase() || !IS_ENABLED(CONFIG_BLOBLIST_FIXED))
		أرجع false;

	/*
	 * Check for a valid list as the configured address.
	 */
	addr = IF_ENABLED_INT(CONFIG_BLOBLIST_FIXED,
			      CONFIG_BLOBLIST_ADDR);
	ret = bloblist_check(addr, CONFIG_BLOBLIST_ناپ);
	إذا (!ret)
		اذهب لبھیا;

	log_debug("Bloblist at %lx not found (err=%d)\n", addr, ret);
	أرجع false;

لبھیا:
#جے_تعریف_شدہ DEBUG
	bloblist_show_stats();
	bloblist_show_list();
#جے_انت
	أرجع true;
}

صحيح bloblist_init(فراغ)
{
	صحيح ret;
	ulong addr = 0, ناپ = CONFIG_BLOBLIST_ناپ;

	إذا (gd->flags & GD_FLG_BLOBLIST_HANDOFF) {
		log_debug("Found existing bloblist size %x at %p\n",
			  gd->bloblist->کل_ناپ, gd->bloblist);
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
		فراغ *ptr = memalign(BLOBLIST_ALIGN, ناپ);

		إذا (!ptr)
			أرجع log_پیغام_ret("alloc", -ENOMEM);
		addr = map_منزل_sysmem(ptr);
	} وإلا
		addr = IF_ENABLED_INT(CONFIG_BLOBLIST_FIXED,
				      CONFIG_BLOBLIST_ADDR);

	log_debug("Creating new bloblist size %lx at %lx\n", ناپ,
		  addr);
	ret = bloblist_new(addr, ناپ, 0, 0);
	إذا (ret)
		أرجع log_پیغام_ret("ini", ret);

	أرجع 0;
}

صحيح bloblist_check_reg_conv(ulong rfdt, ulong rzero, ulong rsig, ulong xlist)
{
	u64 version = BLOBLIST_REGCONV_VER;
	ulong sigval;
	صحيح ret;

	إذا ((IS_ENABLED(CONFIG_64_ثنائی_ہندسہ) && !IS_ENABLED(CONFIG_SPL_BUILD)) ||
			(IS_ENABLED(CONFIG_SPL_64_ثنائی_ہندسہ) && IS_ENABLED(CONFIG_SPL_BUILD))) {
		sigval = ((BLOBLIST_MAGIC & ((1ULL << BLOBLIST_REGCONV_کھسکاؤ_64) - 1)) |
			 ((version  & BLOBLIST_REGCONV_نقاب) << BLOBLIST_REGCONV_کھسکاؤ_64));
	} وإلا {
		sigval = ((BLOBLIST_MAGIC & ((1UL << BLOBLIST_REGCONV_کھسکاؤ_32) - 1)) |
			 ((version  & BLOBLIST_REGCONV_نقاب) << BLOBLIST_REGCONV_کھسکاؤ_32));
	}

	إذا (rzero || rsig != sigval)
		أرجع -EIO;

	ret = bloblist_check(xlist, 0);
	إذا (ret)
		أرجع ret;

	إذا (rfdt != (ulong)bloblist_لبھو(BLOBLISTT_CONTROL_FDT, 0)) {
		/* Remove this bloblist from gd */
		gd->bloblist = عدم;
		gd->flags &= ~GD_FLG_BLOBLIST_HANDOFF;
		أرجع -EIO;
	}

	أرجع 0;
}
