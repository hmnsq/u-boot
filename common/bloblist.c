// SPDX-License-Identifier: GPL-2.0+ BSD-3-Clause
/*
 * Copyright 2018 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#تعریف_کړه LOG_CATEGORY	LOGC_BLOBLIST

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

ساكن بنية نښان_name {
	تعداد bloblist_نښان_t نښان;
	ثابت حرف *name;
} نښان_name[] = {
	{ BLOBLISTT_VOID, "(void)" },

	/* BLOBLISTT_AREA_FIRMWARE_TOP */
	{ BLOBLISTT_CONTROL_FDT, "Control FDT" },
	{ BLOBLISTT_HOB_بلاک, "HOB block" },
	{ BLOBLISTT_HOB_LIST, "HOB list" },
	{ BLOBLISTT_ACPI_TABLES, "ACPI tables for x86" },
	{ BLOBLISTT_TPM_EVLOG, "TPM event log defined by TCG EFI" },
	{ BLOBLISTT_TPM_CRB_بنسټ, "TPM Command Response Buffer address" },
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

ثابت حرف *bloblist_نښان_name(تعداد bloblist_نښان_t نښان)
{
	صحيح i;

	لكل (i = 0; i < ARRAY_اندازه(نښان_name); i++) {
		إذا (نښان_name[i].نښان == نښان)
			أرجع نښان_name[i].name;
	}

	أرجع "invalid";
}

ساكن بنية bloblist_rec *bloblist_لومړی_blob(بنية bloblist_hdr *hdr)
{
	إذا (hdr->کارول_شوی_اندازه <= hdr->hdr_اندازه)
		أرجع عدم;
	أرجع (بنية bloblist_rec *)((فراغ *)hdr + hdr->hdr_اندازه);
}

ساكن مضمن uint rec_hdr_اندازه(بنية bloblist_rec *rec)
{
	أرجع (rec->نښان_and_hdr_اندازه & BLOBLISTR_HDR_اندازه_ماسک) >>
		BLOBLISTR_HDR_اندازه_ښويدنه;
}

ساكن مضمن uint rec_نښان(بنية bloblist_rec *rec)
{
	أرجع (rec->نښان_and_hdr_اندازه & BLOBLISTR_نښان_ماسک) >>
		BLOBLISTR_نښان_ښويدنه;
}

ساكن مضمن فراغ void_blob(بنية bloblist_rec *rec)
{
	إذا (rec_نښان(rec) == BLOBLISTT_VOID)
		أرجع;
	rec->نښان_and_hdr_اندازه = BLOBLISTT_VOID |
				حجم(*rec) << BLOBLISTR_HDR_اندازه_ښويدنه;
}

ساكن مضمن بنية bloblist_rec *rec_from_blob(فراغ *blob)
{
	أرجع (blob - حجم(بنية bloblist_rec));
}

ساكن ulong bloblist_blob_پای_ټکی_ofs(بنية bloblist_hdr *hdr,
				   بنية bloblist_rec *rec)
{
	ulong دننه_واټن;

	دننه_واټن = (فراغ *)rec - (فراغ *)hdr;
	/*
	 * The data section of next TE should start from an address aligned
	 * to 1 << hdr->align_log2.
	 */
	دننه_واټن += rec_hdr_اندازه(rec) + rec->اندازه;
	دننه_واټن = پړاو_پورته(دننه_واټن + rec_hdr_اندازه(rec), 1 << hdr->align_log2);
	دننه_واټن -= rec_hdr_اندازه(rec);

	أرجع دننه_واټن;
}

ساكن بنية bloblist_rec *bloblist_بل_blob(بنية bloblist_hdr *hdr,
					       بنية bloblist_rec *rec)
{
	ulong دننه_واټن = bloblist_blob_پای_ټکی_ofs(hdr, rec);

	إذا (دننه_واټن >= hdr->کارول_شوی_اندازه)
		أرجع عدم;
	أرجع (بنية bloblist_rec *)((فراغ *)hdr + دننه_واټن);
}

#تعریف_کړه foreach_rec(_rec, _hdr) \
	لكل (_rec = bloblist_لومړی_blob(_hdr); \
	     _rec; \
	     _rec = bloblist_بل_blob(_hdr, _rec))

ساكن بنية bloblist_rec *bloblist_findrec(uint نښان)
{
	بنية bloblist_hdr *hdr = gd->bloblist;
	بنية bloblist_rec *rec;

	إذا (!hdr)
		أرجع عدم;

	foreach_rec(rec, hdr) {
		إذا (rec_نښان(rec) == نښان)
			أرجع rec;
	}

	أرجع عدم;
}

ساكن صحيح bloblist_addrec(uint نښان, صحيح اندازه, صحيح align_log2,
			   بنية bloblist_rec **recp)
{
	بنية bloblist_hdr *hdr = gd->bloblist;
	بنية bloblist_rec *rec;
	phys_addr_t معلومات_پیل, aligned_پیل;
	phys_اندازه_t new_alloced;

	إذا (!align_log2)
		align_log2 = BLOBLIST_BLOB_ALIGN_LOG2;

	/* Figure out where the new data will start */
	معلومات_پیل = map_منزل_sysmem(hdr) + hdr->کارول_شوی_اندازه + حجم(*rec);

	/* Align the address and then calculate the offset from used size */
	aligned_پیل = ALIGN(معلومات_پیل, 1U << align_log2) - معلومات_پیل;

	/* If we need to create a dummy record, create it */
	إذا (aligned_پیل) {
		صحيح void_اندازه = aligned_پیل - حجم(*rec);
		بنية bloblist_rec *vrec;
		صحيح ret;

		ret = bloblist_addrec(BLOBLISTT_VOID, void_اندازه, 0, &vrec);
		إذا (ret)
			أرجع log_پیغام_ret("void", ret);

		/* start the record after that */
		معلومات_پیل = map_منزل_sysmem(hdr) + hdr->کارول_شوی_اندازه + حجم(*vrec);
	}

	/* Calculate the new allocated total */
	new_alloced = معلومات_پیل - map_منزل_sysmem(hdr) +
		ALIGN(اندازه, 1U << align_log2);

	إذا (new_alloced > hdr->ټول_اندازه) {
		log_err("Failed to allocate %x bytes\n", اندازه);
		log_err("Used size=%x, total size=%x\n",
			hdr->کارول_شوی_اندازه, hdr->ټول_اندازه);
		أرجع log_پیغام_ret("bloblist add", -ENOSPC);
	}
	rec = (فراغ *)hdr + hdr->کارول_شوی_اندازه;

	rec->نښان_and_hdr_اندازه = نښان | حجم(*rec) << BLOBLISTR_HDR_اندازه_ښويدنه;
	rec->اندازه = اندازه;

	/* Zero the record data */
	memset((فراغ *)rec + rec_hdr_اندازه(rec), '\0', rec->اندازه);

	hdr->کارول_شوی_اندازه = new_alloced;
	*recp = rec;

	أرجع 0;
}

ساكن صحيح bloblist_ensurerec(uint نښان, بنية bloblist_rec **recp, صحيح اندازه,
			      صحيح align_log2)
{
	بنية bloblist_rec *rec;

	rec = bloblist_findrec(نښان);
	إذا (rec) {
		إذا (اندازه && اندازه != rec->اندازه) {
			*recp = rec;
			أرجع -ESPIPE;
		}
	} وإلا {
		صحيح ret;

		ret = bloblist_addrec(نښان, اندازه, align_log2, &rec);
		إذا (ret)
			أرجع ret;
	}
	*recp = rec;

	أرجع 0;
}

ساكن صحيح bloblist_get_blob_معلومات_دننه_واټن(uint نښان)
{
	بدل (نښان) {
	حالة BLOBLISTT_FDT_OVERLAY:
		أرجع حجم(بنية dto_blob_hdr);
	/*
	 * return the data offset if it is not following the blob
	 * header immediately.
	 */
	}
	أرجع 0;
}

فراغ *bloblist_پیدا_کړه(uint نښان, صحيح اندازه)
{
	فراغ *blob = عدم;
	صحيح blob_اندازه;

	blob = bloblist_get_blob(نښان, &blob_اندازه);

	إذا (اندازه && اندازه != blob_اندازه)
		أرجع عدم;

	أرجع blob;
}

فراغ *bloblist_get_blob(uint نښان, صحيح *sizep)
{
	بنية bloblist_rec *rec;

	rec = bloblist_findrec(نښان);
	إذا (!rec)
		أرجع عدم;

	*sizep = rec->اندازه;

	أرجع (فراغ *)rec + rec_hdr_اندازه(rec);
}

صحيح bloblist_apply_blobs(uint نښان, صحيح (*func)(فراغ **معلومات, صحيح اندازه))
{
	بنية bloblist_hdr *hdr = gd->bloblist;
	بنية bloblist_rec *rec;

	إذا (!func || !hdr)
		أرجع -ENOENT;

	foreach_rec(rec, hdr) {
		/* Apply all blobs with the specified tag */
		إذا (rec_نښان(rec) == نښان) {
			صحيح ret;
			صحيح نښان = rec_نښان(rec);
			فراغ *blob = (فراغ *)rec + rec_hdr_اندازه(rec);
			صحيح dat_off = bloblist_get_blob_معلومات_دننه_واټن(نښان);

			blob += dat_off;
			ret = func(&blob, rec->اندازه - dat_off);
			إذا (ret) {
				log_err("Failed to apply blob with tag %d\n",
					نښان);
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

فراغ *bloblist_جمع_کړه(uint نښان, صحيح اندازه, صحيح align_log2)
{
	بنية bloblist_rec *rec;

	إذا (bloblist_addrec(نښان, اندازه, align_log2, &rec))
		أرجع عدم;

	أرجع (فراغ *)rec + rec_hdr_اندازه(rec);
}

صحيح bloblist_ensure_اندازه(uint نښان, صحيح اندازه, صحيح align_log2, فراغ **blobp)
{
	بنية bloblist_rec *rec;
	صحيح ret;

	ret = bloblist_ensurerec(نښان, &rec, اندازه, align_log2);
	إذا (ret)
		أرجع ret;
	*blobp = (فراغ *)rec + rec_hdr_اندازه(rec);

	أرجع 0;
}

فراغ *bloblist_ensure(uint نښان, صحيح اندازه)
{
	بنية bloblist_rec *rec;

	إذا (bloblist_ensurerec(نښان, &rec, اندازه, 0))
		أرجع عدم;

	أرجع (فراغ *)rec + rec_hdr_اندازه(rec);
}

صحيح bloblist_ensure_اندازه_ret(uint نښان, صحيح *sizep, فراغ **blobp)
{
	بنية bloblist_rec *rec;
	صحيح ret;

	ret = bloblist_ensurerec(نښان, &rec, *sizep, 0);
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
	ulong بل_ofs;	/* Offset of the record after @rec */

	expand_by = ALIGN(new_اندازه - rec->اندازه, BLOBLIST_BLOB_ALIGN);
	new_alloced = ALIGN(hdr->کارول_شوی_اندازه + expand_by, BLOBLIST_BLOB_ALIGN);
	إذا (new_اندازه < 0) {
		log_debug("Attempt to shrink blob size below 0 (%x)\n",
			  new_اندازه);
		أرجع log_پیغام_ret("size", -EINVAL);
	}
	إذا (new_alloced > hdr->ټول_اندازه) {
		log_err("Failed to allocate %x bytes\n", new_اندازه);
		log_err("Used size=%x, total size=%x\n",
			hdr->کارول_شوی_اندازه, hdr->ټول_اندازه);
		أرجع log_پیغام_ret("alloc", -ENOSPC);
	}

	/* Move the following blobs up or down, if this is not the last */
	بل_ofs = bloblist_blob_پای_ټکی_ofs(hdr, rec);
	إذا (بل_ofs != hdr->کارول_شوی_اندازه) {
		memmove((فراغ *)hdr + بل_ofs + expand_by,
			(فراغ *)hdr + بل_ofs, hdr->کارول_شوی_اندازه - بل_ofs);
	}
	hdr->کارول_شوی_اندازه = new_alloced;

	/* Zero the new part of the blob */
	إذا (expand_by > 0) {
		memset((فراغ *)rec + rec_hdr_اندازه(rec) + rec->اندازه, '\0',
		       new_اندازه - rec->اندازه);
	}

	/* Update the size of this blob */
	rec->اندازه = new_اندازه;

	أرجع 0;
}

صحيح bloblist_resize(uint نښان, صحيح new_اندازه)
{
	بنية bloblist_hdr *hdr = gd->bloblist;
	بنية bloblist_rec *rec;
	صحيح ret;

	rec = bloblist_findrec(نښان);
	إذا (!rec)
		أرجع log_پیغام_ret("find", -ENOENT);
	ret = bloblist_resize_rec(hdr, rec, new_اندازه);
	إذا (ret)
		أرجع log_پیغام_ret("resize", ret);

	أرجع 0;
}

ساكن u32 bloblist_calc_chksum(بنية bloblist_hdr *hdr)
{
	u8 chksum;

	chksum = جدول_compute_checksum(hdr, hdr->کارول_شوی_اندازه);
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
	hdr->کارول_شوی_اندازه = hdr->hdr_اندازه;
	hdr->ټول_اندازه = اندازه;
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
		أرجع log_پیغام_ret("Bad magic", -ENOENT);
	إذا (hdr->version != BLOBLIST_نسخه)
		أرجع log_پیغام_ret("Bad version", -EPROTONOSUPPORT);
	إذا (!hdr->ټول_اندازه || (اندازه && hdr->ټول_اندازه > اندازه))
		أرجع log_پیغام_ret("Bad total size", -EFBIG);
	إذا (hdr->کارول_شوی_اندازه > hdr->ټول_اندازه)
		أرجع log_پیغام_ret("Bad used size", -ENOENT);
	إذا (hdr->hdr_اندازه != حجم(بنية bloblist_hdr))
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

صحيح bloblist_پای(فراغ)
{
	بنية bloblist_hdr *hdr = gd->bloblist;

	hdr->chksum = bloblist_calc_chksum(hdr);
	log_debug("Finished bloblist size %lx at %lx\n", (ulong)hdr->کارول_شوی_اندازه,
		  (ulong)map_منزل_sysmem(hdr));

	أرجع 0;
}

ulong bloblist_get_بنسټ(فراغ)
{
	أرجع map_منزل_sysmem(gd->bloblist);
}

ulong bloblist_get_اندازه(فراغ)
{
	بنية bloblist_hdr *hdr = gd->bloblist;

	أرجع hdr->کارول_شوی_اندازه;
}

ulong bloblist_get_ټول_اندازه(فراغ)
{
	بنية bloblist_hdr *hdr = gd->bloblist;

	أرجع hdr->ټول_اندازه;
}

فراغ bloblist_get_stats(ulong *basep, ulong *tsizep, ulong *usizep)
{
	بنية bloblist_hdr *hdr = gd->bloblist;

	*basep = map_منزل_sysmem(gd->bloblist);
	*tsizep = hdr->ټول_اندازه;
	*usizep = hdr->کارول_شوی_اندازه;
}

ساكن فراغ show_ارزښت(ثابت حرف *prompt, ulong ارزښت)
{
	چاپ_کړه("%s:%*s %-5lx  ", prompt, 10 - (صحيح)مزي_اوږدوالی(prompt), "", ارزښت);
	print_اندازه(ارزښت, "\n");
}

فراغ bloblist_show_stats(فراغ)
{
	ulong بنسټ, tsize, usize;

	bloblist_get_stats(&بنسټ, &tsize, &usize);
	چاپ_کړه("base:       %lx\n", بنسټ);
	show_ارزښت("total size", tsize);
	show_ارزښت("used size", usize);
	show_ارزښت("free", tsize - usize);
}

فراغ bloblist_show_list(فراغ)
{
	بنية bloblist_hdr *hdr = gd->bloblist;
	بنية bloblist_rec *rec;

	چاپ_کړه("%-8s  %8s   Tag Name\n", "Address", "Size");
	لكل (rec = bloblist_لومړی_blob(hdr); rec;
	     rec = bloblist_بل_blob(hdr, rec)) {
		چاپ_کړه("%08lx  %8x  %4x %s\n",
		       (ulong)map_منزل_sysmem((فراغ *)rec + rec_hdr_اندازه(rec)),
		       rec->اندازه, rec_نښان(rec),
		       bloblist_نښان_name(rec_نښان(rec)));
	}
}

صحيح bloblist_reloc(فراغ *منزل, uint منزل_اندازه)
{
	بنية bloblist_hdr *hdr;

	إذا (!منزل_اندازه)
		أرجع 0;

	إذا (منزل_اندازه < gd->bloblist->ټول_اندازه)
		أرجع -ENOSPC;

	حافظه_نقل(منزل, gd->bloblist, gd->bloblist->ټول_اندازه);
	hdr = منزل;
	hdr->ټول_اندازه = منزل_اندازه;
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
		اذهب وموندل;

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
	إذا (xpl_is_لومړی_phase() || !IS_ENABLED(CONFIG_BLOBLIST_FIXED))
		أرجع false;

	/*
	 * Check for a valid list as the configured address.
	 */
	addr = IF_ENABLED_INT(CONFIG_BLOBLIST_FIXED,
			      CONFIG_BLOBLIST_ADDR);
	ret = bloblist_check(addr, CONFIG_BLOBLIST_اندازه);
	إذا (!ret)
		اذهب وموندل;

	log_debug("Bloblist at %lx not found (err=%d)\n", addr, ret);
	أرجع false;

وموندل:
#که_تعریف_شوی DEBUG
	bloblist_show_stats();
	bloblist_show_list();
#د_که_پای
	أرجع true;
}

صحيح bloblist_init(فراغ)
{
	صحيح ret;
	ulong addr = 0, اندازه = CONFIG_BLOBLIST_اندازه;

	إذا (gd->flags & GD_FLG_BLOBLIST_HANDOFF) {
		log_debug("Found existing bloblist size %x at %p\n",
			  gd->bloblist->ټول_اندازه, gd->bloblist);
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
			أرجع log_پیغام_ret("alloc", -ENOMEM);
		addr = map_منزل_sysmem(ptr);
	} وإلا
		addr = IF_ENABLED_INT(CONFIG_BLOBLIST_FIXED,
				      CONFIG_BLOBLIST_ADDR);

	log_debug("Creating new bloblist size %lx at %lx\n", اندازه,
		  addr);
	ret = bloblist_new(addr, اندازه, 0, 0);
	إذا (ret)
		أرجع log_پیغام_ret("ini", ret);

	أرجع 0;
}

صحيح bloblist_check_reg_conv(ulong rfdt, ulong rzero, ulong rsig, ulong xlist)
{
	u64 version = BLOBLIST_REGCONV_VER;
	ulong sigval;
	صحيح ret;

	إذا ((IS_ENABLED(CONFIG_64_بټ) && !IS_ENABLED(CONFIG_SPL_BUILD)) ||
			(IS_ENABLED(CONFIG_SPL_64_بټ) && IS_ENABLED(CONFIG_SPL_BUILD))) {
		sigval = ((BLOBLIST_MAGIC & ((1ULL << BLOBLIST_REGCONV_ښويدنه_64) - 1)) |
			 ((version  & BLOBLIST_REGCONV_ماسک) << BLOBLIST_REGCONV_ښويدنه_64));
	} وإلا {
		sigval = ((BLOBLIST_MAGIC & ((1UL << BLOBLIST_REGCONV_ښويدنه_32) - 1)) |
			 ((version  & BLOBLIST_REGCONV_ماسک) << BLOBLIST_REGCONV_ښويدنه_32));
	}

	إذا (rzero || rsig != sigval)
		أرجع -EIO;

	ret = bloblist_check(xlist, 0);
	إذا (ret)
		أرجع ret;

	إذا (rfdt != (ulong)bloblist_پیدا_کړه(BLOBLISTT_CONTROL_FDT, 0)) {
		/* Remove this bloblist from gd */
		gd->bloblist = عدم;
		gd->flags &= ~GD_FLG_BLOBLIST_HANDOFF;
		أرجع -EIO;
	}

	أرجع 0;
}
