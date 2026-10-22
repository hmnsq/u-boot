// SPDX-License-Identifier: GPL-2.0+ BSD-3-Clause
/*
 * Copyright 2018 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#تعریف_کن LOG_CATEGORY	LOGC_BLOBLIST

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

DECLARE_GLOBAL_داده_PTR;

ساكن بنية برچسب_name {
	تعداد bloblist_برچسب_t برچسب;
	ثابت حرف *name;
} برچسب_name[] = {
	{ BLOBLISTT_VOID, "(void)" },

	/* BLOBLISTT_AREA_FIRMWARE_TOP */
	{ BLOBLISTT_CONTROL_FDT, "Control FDT" },
	{ BLOBLISTT_HOB_بلوک, "HOB block" },
	{ BLOBLISTT_HOB_LIST, "HOB list" },
	{ BLOBLISTT_ACPI_TABLES, "ACPI tables for x86" },
	{ BLOBLISTT_TPM_EVLOG, "TPM event log defined by TCG EFI" },
	{ BLOBLISTT_TPM_CRB_پایه, "TPM Command Response Buffer address" },
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

ثابت حرف *bloblist_برچسب_name(تعداد bloblist_برچسب_t برچسب)
{
	صحيح i;

	لكل (i = 0; i < ARRAY_اندازه(برچسب_name); i++) {
		إذا (برچسب_name[i].برچسب == برچسب)
			أرجع برچسب_name[i].name;
	}

	أرجع "invalid";
}

ساكن بنية bloblist_rec *bloblist_اولی_blob(بنية bloblist_hdr *hdr)
{
	إذا (hdr->استفاده_شده_اندازه <= hdr->hdr_اندازه)
		أرجع عدم;
	أرجع (بنية bloblist_rec *)((فراغ *)hdr + hdr->hdr_اندازه);
}

ساكن مضمن uint rec_hdr_اندازه(بنية bloblist_rec *rec)
{
	أرجع (rec->برچسب_and_hdr_اندازه & BLOBLISTR_HDR_اندازه_ماسک) >>
		BLOBLISTR_HDR_اندازه_لغزش;
}

ساكن مضمن uint rec_برچسب(بنية bloblist_rec *rec)
{
	أرجع (rec->برچسب_and_hdr_اندازه & BLOBLISTR_برچسب_ماسک) >>
		BLOBLISTR_برچسب_لغزش;
}

ساكن مضمن فراغ void_blob(بنية bloblist_rec *rec)
{
	إذا (rec_برچسب(rec) == BLOBLISTT_VOID)
		أرجع;
	rec->برچسب_and_hdr_اندازه = BLOBLISTT_VOID |
				حجم(*rec) << BLOBLISTR_HDR_اندازه_لغزش;
}

ساكن مضمن بنية bloblist_rec *rec_from_blob(فراغ *blob)
{
	أرجع (blob - حجم(بنية bloblist_rec));
}

ساكن ulong bloblist_blob_انتها_ofs(بنية bloblist_hdr *hdr,
				   بنية bloblist_rec *rec)
{
	ulong جابه_جایی;

	جابه_جایی = (فراغ *)rec - (فراغ *)hdr;
	/*
	 * The data section of next TE should start from an address aligned
	 * to 1 << hdr->align_log2.
	 */
	جابه_جایی += rec_hdr_اندازه(rec) + rec->اندازه;
	جابه_جایی = دور_به_بالا(جابه_جایی + rec_hdr_اندازه(rec), 1 << hdr->align_log2);
	جابه_جایی -= rec_hdr_اندازه(rec);

	أرجع جابه_جایی;
}

ساكن بنية bloblist_rec *bloblist_بعدی_blob(بنية bloblist_hdr *hdr,
					       بنية bloblist_rec *rec)
{
	ulong جابه_جایی = bloblist_blob_انتها_ofs(hdr, rec);

	إذا (جابه_جایی >= hdr->استفاده_شده_اندازه)
		أرجع عدم;
	أرجع (بنية bloblist_rec *)((فراغ *)hdr + جابه_جایی);
}

#تعریف_کن foreach_rec(_rec, _hdr) \
	لكل (_rec = bloblist_اولی_blob(_hdr); \
	     _rec; \
	     _rec = bloblist_بعدی_blob(_hdr, _rec))

ساكن بنية bloblist_rec *bloblist_findrec(uint برچسب)
{
	بنية bloblist_hdr *hdr = gd->bloblist;
	بنية bloblist_rec *rec;

	إذا (!hdr)
		أرجع عدم;

	foreach_rec(rec, hdr) {
		إذا (rec_برچسب(rec) == برچسب)
			أرجع rec;
	}

	أرجع عدم;
}

ساكن صحيح bloblist_addrec(uint برچسب, صحيح اندازه, صحيح align_log2,
			   بنية bloblist_rec **recp)
{
	بنية bloblist_hdr *hdr = gd->bloblist;
	بنية bloblist_rec *rec;
	phys_addr_t داده_شروع, aligned_شروع;
	phys_اندازه_t new_alloced;

	إذا (!align_log2)
		align_log2 = BLOBLIST_BLOB_ALIGN_LOG2;

	/* Figure out where the new data will start */
	داده_شروع = map_مقصد_sysmem(hdr) + hdr->استفاده_شده_اندازه + حجم(*rec);

	/* Align the address and then calculate the offset from used size */
	aligned_شروع = ALIGN(داده_شروع, 1U << align_log2) - داده_شروع;

	/* If we need to create a dummy record, create it */
	إذا (aligned_شروع) {
		صحيح void_اندازه = aligned_شروع - حجم(*rec);
		بنية bloblist_rec *vrec;
		صحيح ret;

		ret = bloblist_addrec(BLOBLISTT_VOID, void_اندازه, 0, &vrec);
		إذا (ret)
			أرجع log_پیام_ret("void", ret);

		/* start the record after that */
		داده_شروع = map_مقصد_sysmem(hdr) + hdr->استفاده_شده_اندازه + حجم(*vrec);
	}

	/* Calculate the new allocated total */
	new_alloced = داده_شروع - map_مقصد_sysmem(hdr) +
		ALIGN(اندازه, 1U << align_log2);

	إذا (new_alloced > hdr->مجموع_اندازه) {
		log_err("Failed to allocate %x bytes\n", اندازه);
		log_err("Used size=%x, total size=%x\n",
			hdr->استفاده_شده_اندازه, hdr->مجموع_اندازه);
		أرجع log_پیام_ret("bloblist add", -ENOSPC);
	}
	rec = (فراغ *)hdr + hdr->استفاده_شده_اندازه;

	rec->برچسب_and_hdr_اندازه = برچسب | حجم(*rec) << BLOBLISTR_HDR_اندازه_لغزش;
	rec->اندازه = اندازه;

	/* Zero the record data */
	memset((فراغ *)rec + rec_hdr_اندازه(rec), '\0', rec->اندازه);

	hdr->استفاده_شده_اندازه = new_alloced;
	*recp = rec;

	أرجع 0;
}

ساكن صحيح bloblist_ensurerec(uint برچسب, بنية bloblist_rec **recp, صحيح اندازه,
			      صحيح align_log2)
{
	بنية bloblist_rec *rec;

	rec = bloblist_findrec(برچسب);
	إذا (rec) {
		إذا (اندازه && اندازه != rec->اندازه) {
			*recp = rec;
			أرجع -ESPIPE;
		}
	} وإلا {
		صحيح ret;

		ret = bloblist_addrec(برچسب, اندازه, align_log2, &rec);
		إذا (ret)
			أرجع ret;
	}
	*recp = rec;

	أرجع 0;
}

ساكن صحيح bloblist_get_blob_داده_جابه_جایی(uint برچسب)
{
	بدل (برچسب) {
	حالة BLOBLISTT_FDT_OVERLAY:
		أرجع حجم(بنية dto_blob_hdr);
	/*
	 * return the data offset if it is not following the blob
	 * header immediately.
	 */
	}
	أرجع 0;
}

فراغ *bloblist_پیدا_کن(uint برچسب, صحيح اندازه)
{
	فراغ *blob = عدم;
	صحيح blob_اندازه;

	blob = bloblist_get_blob(برچسب, &blob_اندازه);

	إذا (اندازه && اندازه != blob_اندازه)
		أرجع عدم;

	أرجع blob;
}

فراغ *bloblist_get_blob(uint برچسب, صحيح *sizep)
{
	بنية bloblist_rec *rec;

	rec = bloblist_findrec(برچسب);
	إذا (!rec)
		أرجع عدم;

	*sizep = rec->اندازه;

	أرجع (فراغ *)rec + rec_hdr_اندازه(rec);
}

صحيح bloblist_apply_blobs(uint برچسب, صحيح (*func)(فراغ **داده, صحيح اندازه))
{
	بنية bloblist_hdr *hdr = gd->bloblist;
	بنية bloblist_rec *rec;

	إذا (!func || !hdr)
		أرجع -ENOENT;

	foreach_rec(rec, hdr) {
		/* Apply all blobs with the specified tag */
		إذا (rec_برچسب(rec) == برچسب) {
			صحيح ret;
			صحيح برچسب = rec_برچسب(rec);
			فراغ *blob = (فراغ *)rec + rec_hdr_اندازه(rec);
			صحيح dat_off = bloblist_get_blob_داده_جابه_جایی(برچسب);

			blob += dat_off;
			ret = func(&blob, rec->اندازه - dat_off);
			إذا (ret) {
				log_err("Failed to apply blob with tag %d\n",
					برچسب);
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

فراغ *bloblist_جمع_کن(uint برچسب, صحيح اندازه, صحيح align_log2)
{
	بنية bloblist_rec *rec;

	إذا (bloblist_addrec(برچسب, اندازه, align_log2, &rec))
		أرجع عدم;

	أرجع (فراغ *)rec + rec_hdr_اندازه(rec);
}

صحيح bloblist_ensure_اندازه(uint برچسب, صحيح اندازه, صحيح align_log2, فراغ **blobp)
{
	بنية bloblist_rec *rec;
	صحيح ret;

	ret = bloblist_ensurerec(برچسب, &rec, اندازه, align_log2);
	إذا (ret)
		أرجع ret;
	*blobp = (فراغ *)rec + rec_hdr_اندازه(rec);

	أرجع 0;
}

فراغ *bloblist_ensure(uint برچسب, صحيح اندازه)
{
	بنية bloblist_rec *rec;

	إذا (bloblist_ensurerec(برچسب, &rec, اندازه, 0))
		أرجع عدم;

	أرجع (فراغ *)rec + rec_hdr_اندازه(rec);
}

صحيح bloblist_ensure_اندازه_ret(uint برچسب, صحيح *sizep, فراغ **blobp)
{
	بنية bloblist_rec *rec;
	صحيح ret;

	ret = bloblist_ensurerec(برچسب, &rec, *sizep, 0);
	إذا (ret == -ESPIPE)
		*sizep = rec->اندازه;
	وإلا إذا (ret)
		أرجع ret;
	*blobp = (فراغ *)rec + rec_hdr_اندازه(rec);

	أرجع 0;
}

ساكن صحيح bloblist_resize_rec(بنية bloblist_hdr *hdr,
			       بنية bloblist_rec *rec,
			       صحيح new_اندازه)
{
	صحيح expand_by;	/* Number of bytes to expand by (-ve to contract) */
	صحيح new_alloced;
	ulong بعدی_ofs;	/* Offset of the record after @rec */

	expand_by = ALIGN(new_اندازه - rec->اندازه, BLOBLIST_BLOB_ALIGN);
	new_alloced = ALIGN(hdr->استفاده_شده_اندازه + expand_by, BLOBLIST_BLOB_ALIGN);
	إذا (new_اندازه < 0) {
		log_debug("Attempt to shrink blob size below 0 (%x)\n",
			  new_اندازه);
		أرجع log_پیام_ret("size", -EINVAL);
	}
	إذا (new_alloced > hdr->مجموع_اندازه) {
		log_err("Failed to allocate %x bytes\n", new_اندازه);
		log_err("Used size=%x, total size=%x\n",
			hdr->استفاده_شده_اندازه, hdr->مجموع_اندازه);
		أرجع log_پیام_ret("alloc", -ENOSPC);
	}

	/* Move the following blobs up or down, if this is not the last */
	بعدی_ofs = bloblist_blob_انتها_ofs(hdr, rec);
	إذا (بعدی_ofs != hdr->استفاده_شده_اندازه) {
		memmove((فراغ *)hdr + بعدی_ofs + expand_by,
			(فراغ *)hdr + بعدی_ofs, hdr->استفاده_شده_اندازه - بعدی_ofs);
	}
	hdr->استفاده_شده_اندازه = new_alloced;

	/* Zero the new part of the blob */
	إذا (expand_by > 0) {
		memset((فراغ *)rec + rec_hdr_اندازه(rec) + rec->اندازه, '\0',
		       new_اندازه - rec->اندازه);
	}

	/* Update the size of this blob */
	rec->اندازه = new_اندازه;

	أرجع 0;
}

صحيح bloblist_resize(uint برچسب, صحيح new_اندازه)
{
	بنية bloblist_hdr *hdr = gd->bloblist;
	بنية bloblist_rec *rec;
	صحيح ret;

	rec = bloblist_findrec(برچسب);
	إذا (!rec)
		أرجع log_پیام_ret("find", -ENOENT);
	ret = bloblist_resize_rec(hdr, rec, new_اندازه);
	إذا (ret)
		أرجع log_پیام_ret("resize", ret);

	أرجع 0;
}

ساكن u32 bloblist_calc_chksum(بنية bloblist_hdr *hdr)
{
	u8 chksum;

	chksum = جدول_compute_checksum(hdr, hdr->استفاده_شده_اندازه);
	chksum += hdr->chksum;

	أرجع chksum;
}

صحيح bloblist_new(ulong addr, uint اندازه, uint flags, uint align_log2)
{
	بنية bloblist_hdr *hdr;

	إذا (اندازه < حجم(*hdr))
		أرجع log_ret(-ENOSPC);
	إذا (addr & (BLOBLIST_ALIGN - 1))
		أرجع log_ret(-EFAULT);
	hdr = map_sysmem(addr, اندازه);
	memset(hdr, '\0', حجم(*hdr));
	hdr->version = BLOBLIST_نسخه;
	hdr->hdr_اندازه = حجم(*hdr);
	hdr->flags = flags;
	hdr->magic = BLOBLIST_MAGIC;
	hdr->استفاده_شده_اندازه = hdr->hdr_اندازه;
	hdr->مجموع_اندازه = اندازه;
	hdr->align_log2 = align_log2 ? align_log2 : BLOBLIST_BLOB_ALIGN_LOG2;
	hdr->chksum = 0;
	gd->bloblist = hdr;
	gd->flags |= GD_FLG_BLOBLIST_HANDOFF;

	أرجع 0;
}

صحيح bloblist_check(ulong addr, uint اندازه)
{
	بنية bloblist_hdr *hdr;
	u32 chksum;

	hdr = map_sysmem(addr, حجم(*hdr));
	إذا (hdr->magic != BLOBLIST_MAGIC)
		أرجع log_پیام_ret("Bad magic", -ENOENT);
	إذا (hdr->version != BLOBLIST_نسخه)
		أرجع log_پیام_ret("Bad version", -EPROTONOSUPPORT);
	إذا (!hdr->مجموع_اندازه || (اندازه && hdr->مجموع_اندازه > اندازه))
		أرجع log_پیام_ret("Bad total size", -EFBIG);
	إذا (hdr->استفاده_شده_اندازه > hdr->مجموع_اندازه)
		أرجع log_پیام_ret("Bad used size", -ENOENT);
	إذا (hdr->hdr_اندازه != حجم(بنية bloblist_hdr))
		أرجع log_پیام_ret("Bad header size", -ENOENT);

	chksum = bloblist_calc_chksum(hdr);
	إذا (hdr->chksum != chksum) {
		log_err("Checksum %x != %x\n", hdr->chksum, chksum);
		أرجع log_پیام_ret("Bad checksum", -EIO);
	}
	gd->bloblist = hdr;
	gd->flags |= GD_FLG_BLOBLIST_HANDOFF;

	أرجع 0;
}

صحيح bloblist_پایان(فراغ)
{
	بنية bloblist_hdr *hdr = gd->bloblist;

	hdr->chksum = bloblist_calc_chksum(hdr);
	log_debug("Finished bloblist size %lx at %lx\n", (ulong)hdr->استفاده_شده_اندازه,
		  (ulong)map_مقصد_sysmem(hdr));

	أرجع 0;
}

ulong bloblist_get_پایه(فراغ)
{
	أرجع map_مقصد_sysmem(gd->bloblist);
}

ulong bloblist_get_اندازه(فراغ)
{
	بنية bloblist_hdr *hdr = gd->bloblist;

	أرجع hdr->استفاده_شده_اندازه;
}

ulong bloblist_get_مجموع_اندازه(فراغ)
{
	بنية bloblist_hdr *hdr = gd->bloblist;

	أرجع hdr->مجموع_اندازه;
}

فراغ bloblist_get_stats(ulong *basep, ulong *tsizep, ulong *usizep)
{
	بنية bloblist_hdr *hdr = gd->bloblist;

	*basep = map_مقصد_sysmem(gd->bloblist);
	*tsizep = hdr->مجموع_اندازه;
	*usizep = hdr->استفاده_شده_اندازه;
}

ساكن فراغ show_مقدار(ثابت حرف *prompt, ulong مقدار)
{
	چاپ_کن("%s:%*s %-5lx  ", prompt, 10 - (صحيح)طول_رشته(prompt), "", مقدار);
	print_اندازه(مقدار, "\n");
}

فراغ bloblist_show_stats(فراغ)
{
	ulong پایه, tsize, usize;

	bloblist_get_stats(&پایه, &tsize, &usize);
	چاپ_کن("base:       %lx\n", پایه);
	show_مقدار("total size", tsize);
	show_مقدار("used size", usize);
	show_مقدار("free", tsize - usize);
}

فراغ bloblist_show_list(فراغ)
{
	بنية bloblist_hdr *hdr = gd->bloblist;
	بنية bloblist_rec *rec;

	چاپ_کن("%-8s  %8s   Tag Name\n", "Address", "Size");
	لكل (rec = bloblist_اولی_blob(hdr); rec;
	     rec = bloblist_بعدی_blob(hdr, rec)) {
		چاپ_کن("%08lx  %8x  %4x %s\n",
		       (ulong)map_مقصد_sysmem((فراغ *)rec + rec_hdr_اندازه(rec)),
		       rec->اندازه, rec_برچسب(rec),
		       bloblist_برچسب_name(rec_برچسب(rec)));
	}
}

صحيح bloblist_reloc(فراغ *مقصد, uint مقصد_اندازه)
{
	بنية bloblist_hdr *hdr;

	إذا (!مقصد_اندازه)
		أرجع 0;

	إذا (مقصد_اندازه < gd->bloblist->مجموع_اندازه)
		أرجع -ENOSPC;

	کپی_حافظه(مقصد, gd->bloblist, gd->bloblist->مجموع_اندازه);
	hdr = مقصد;
	hdr->مجموع_اندازه = مقصد_اندازه;
	gd->bloblist = مقصد;

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
		اذهب یافت;

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
	إذا (xpl_is_اولی_phase() || !IS_ENABLED(CONFIG_BLOBLIST_FIXED))
		أرجع false;

	/*
	 * Check for a valid list as the configured address.
	 */
	addr = IF_ENABLED_INT(CONFIG_BLOBLIST_FIXED,
			      CONFIG_BLOBLIST_ADDR);
	ret = bloblist_check(addr, CONFIG_BLOBLIST_اندازه);
	إذا (!ret)
		اذهب یافت;

	log_debug("Bloblist at %lx not found (err=%d)\n", addr, ret);
	أرجع false;

یافت:
#اگر_تعریف_شده DEBUG
	bloblist_show_stats();
	bloblist_show_list();
#پایان_اگر
	أرجع true;
}

صحيح bloblist_init(فراغ)
{
	صحيح ret;
	ulong addr = 0, اندازه = CONFIG_BLOBLIST_اندازه;

	إذا (gd->flags & GD_FLG_BLOBLIST_HANDOFF) {
		log_debug("Found existing bloblist size %x at %p\n",
			  gd->bloblist->مجموع_اندازه, gd->bloblist);
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
		فراغ *ptr = memalign(BLOBLIST_ALIGN, اندازه);

		إذا (!ptr)
			أرجع log_پیام_ret("alloc", -ENOMEM);
		addr = map_مقصد_sysmem(ptr);
	} وإلا
		addr = IF_ENABLED_INT(CONFIG_BLOBLIST_FIXED,
				      CONFIG_BLOBLIST_ADDR);

	log_debug("Creating new bloblist size %lx at %lx\n", اندازه,
		  addr);
	ret = bloblist_new(addr, اندازه, 0, 0);
	إذا (ret)
		أرجع log_پیام_ret("ini", ret);

	أرجع 0;
}

صحيح bloblist_check_reg_conv(ulong rfdt, ulong rzero, ulong rsig, ulong xlist)
{
	u64 version = BLOBLIST_REGCONV_VER;
	ulong sigval;
	صحيح ret;

	إذا ((IS_ENABLED(CONFIG_64_بیت) && !IS_ENABLED(CONFIG_SPL_BUILD)) ||
			(IS_ENABLED(CONFIG_SPL_64_بیت) && IS_ENABLED(CONFIG_SPL_BUILD))) {
		sigval = ((BLOBLIST_MAGIC & ((1ULL << BLOBLIST_REGCONV_لغزش_64) - 1)) |
			 ((version  & BLOBLIST_REGCONV_ماسک) << BLOBLIST_REGCONV_لغزش_64));
	} وإلا {
		sigval = ((BLOBLIST_MAGIC & ((1UL << BLOBLIST_REGCONV_لغزش_32) - 1)) |
			 ((version  & BLOBLIST_REGCONV_ماسک) << BLOBLIST_REGCONV_لغزش_32));
	}

	إذا (rzero || rsig != sigval)
		أرجع -EIO;

	ret = bloblist_check(xlist, 0);
	إذا (ret)
		أرجع ret;

	إذا (rfdt != (ulong)bloblist_پیدا_کن(BLOBLISTT_CONTROL_FDT, 0)) {
		/* Remove this bloblist from gd */
		gd->bloblist = عدم;
		gd->flags &= ~GD_FLG_BLOBLIST_HANDOFF;
		أرجع -EIO;
	}

	أرجع 0;
}
