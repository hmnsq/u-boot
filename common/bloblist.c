// SPDX-License-Identifier: GPL-2.0+ BSD-3-Clause
/*
 * Copyright 2018 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY	LOGC_BLOBLIST

#অন্তর্ভাব <bloblist.h>
#অন্তর্ভাব <display_options.h>
#অন্তর্ভাব <log.h>
#অন্তর্ভাব <malloc.h>
#অন্তর্ভাব <mapmem.h>
#অন্তর্ভাব <spl.h>
#অন্তর্ভাব <tables_csum.h>
#অন্তর্ভাব <asm/global_data.h>
#অন্তর্ভাব <u-boot/crc.h>

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

DECLARE_GLOBAL_তথ্য_PTR;

স্থির কাঠামো tag_name {
	গণনা bloblist_tag_t tag;
	ধ্রুবক অক্ষর *name;
} tag_name[] = {
	{ BLOBLISTT_VOID, "(void)" },

	/* BLOBLISTT_AREA_FIRMWARE_TOP */
	{ BLOBLISTT_CONTROL_FDT, "Control FDT" },
	{ BLOBLISTT_HOB_BLOCK, "HOB block" },
	{ BLOBLISTT_HOB_LIST, "HOB list" },
	{ BLOBLISTT_ACPI_TABLES, "ACPI tables for x86" },
	{ BLOBLISTT_TPM_EVLOG, "TPM event log defined by TCG EFI" },
	{ BLOBLISTT_TPM_CRB_BASE, "TPM Command Response Buffer address" },
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

ধ্রুবক অক্ষর *bloblist_tag_name(গণনা bloblist_tag_t tag)
{
	পূর্ণসংখ্যা i;

	জন্য (i = 0; i < ARRAY_আকার(tag_name); i++) {
		যদি (tag_name[i].tag == tag)
			ফেরত tag_name[i].name;
	}

	ফেরত "invalid";
}

স্থির কাঠামো bloblist_rec *bloblist_first_blob(কাঠামো bloblist_hdr *hdr)
{
	যদি (hdr->used_আকার <= hdr->hdr_আকার)
		ফেরত ফাঁকা;
	ফেরত (কাঠামো bloblist_rec *)((শূন্য *)hdr + hdr->hdr_আকার);
}

স্থির অন্তঃরেখ uint rec_hdr_আকার(কাঠামো bloblist_rec *rec)
{
	ফেরত (rec->tag_and_hdr_আকার & BLOBLISTR_HDR_আকার_MASK) >>
		BLOBLISTR_HDR_আকার_SHIFT;
}

স্থির অন্তঃরেখ uint rec_tag(কাঠামো bloblist_rec *rec)
{
	ফেরত (rec->tag_and_hdr_আকার & BLOBLISTR_TAG_MASK) >>
		BLOBLISTR_TAG_SHIFT;
}

স্থির অন্তঃরেখ শূন্য void_blob(কাঠামো bloblist_rec *rec)
{
	যদি (rec_tag(rec) == BLOBLISTT_VOID)
		ফেরত;
	rec->tag_and_hdr_আকার = BLOBLISTT_VOID |
				আকার(*rec) << BLOBLISTR_HDR_আকার_SHIFT;
}

স্থির অন্তঃরেখ কাঠামো bloblist_rec *rec_from_blob(শূন্য *blob)
{
	ফেরত (blob - আকার(কাঠামো bloblist_rec));
}

স্থির ulong bloblist_blob_end_ofs(কাঠামো bloblist_hdr *hdr,
				   কাঠামো bloblist_rec *rec)
{
	ulong offset;

	offset = (শূন্য *)rec - (শূন্য *)hdr;
	/*
	 * The data section of next TE should start from an address aligned
	 * to 1 << hdr->align_log2.
	 */
	offset += rec_hdr_আকার(rec) + rec->আকার_2;
	offset = round_up(offset + rec_hdr_আকার(rec), 1 << hdr->align_log2);
	offset -= rec_hdr_আকার(rec);

	ফেরত offset;
}

স্থির কাঠামো bloblist_rec *bloblist_পরবর্তী_blob(কাঠামো bloblist_hdr *hdr,
					       কাঠামো bloblist_rec *rec)
{
	ulong offset = bloblist_blob_end_ofs(hdr, rec);

	যদি (offset >= hdr->used_আকার)
		ফেরত ফাঁকা;
	ফেরত (কাঠামো bloblist_rec *)((শূন্য *)hdr + offset);
}

#define foreach_rec(_rec, _hdr) \
	জন্য (_rec = bloblist_first_blob(_hdr); \
	     _rec; \
	     _rec = bloblist_পরবর্তী_blob(_hdr, _rec))

স্থির কাঠামো bloblist_rec *bloblist_findrec(uint tag)
{
	কাঠামো bloblist_hdr *hdr = gd->bloblist;
	কাঠামো bloblist_rec *rec;

	যদি (!hdr)
		ফেরত ফাঁকা;

	foreach_rec(rec, hdr) {
		যদি (rec_tag(rec) == tag)
			ফেরত rec;
	}

	ফেরত ফাঁকা;
}

স্থির পূর্ণসংখ্যা bloblist_addrec(uint tag, পূর্ণসংখ্যা আকার_2, পূর্ণসংখ্যা align_log2,
			   কাঠামো bloblist_rec **recp)
{
	কাঠামো bloblist_hdr *hdr = gd->bloblist;
	কাঠামো bloblist_rec *rec;
	phys_addr_t তথ্য_start, aligned_start;
	phys_আকার_t new_alloced;

	যদি (!align_log2)
		align_log2 = BLOBLIST_BLOB_ALIGN_LOG2;

	/* Figure out where the new data will start */
	তথ্য_start = map_to_sysmem(hdr) + hdr->used_আকার + আকার(*rec);

	/* Align the address and then calculate the offset from used size */
	aligned_start = ALIGN(তথ্য_start, 1U << align_log2) - তথ্য_start;

	/* If we need to create a dummy record, create it */
	যদি (aligned_start) {
		পূর্ণসংখ্যা void_আকার = aligned_start - আকার(*rec);
		কাঠামো bloblist_rec *vrec;
		পূর্ণসংখ্যা ret;

		ret = bloblist_addrec(BLOBLISTT_VOID, void_আকার, 0, &vrec);
		যদি (ret)
			ফেরত log_বার্তা_ret("void", ret);

		/* start the record after that */
		তথ্য_start = map_to_sysmem(hdr) + hdr->used_আকার + আকার(*vrec);
	}

	/* Calculate the new allocated total */
	new_alloced = তথ্য_start - map_to_sysmem(hdr) +
		ALIGN(আকার_2, 1U << align_log2);

	যদি (new_alloced > hdr->যোগ_আকার) {
		log_err("Failed to allocate %x bytes\n", আকার_2);
		log_err("Used size=%x, total size=%x\n",
			hdr->used_আকার, hdr->যোগ_আকার);
		ফেরত log_বার্তা_ret("bloblist add", -ENOSPC);
	}
	rec = (শূন্য *)hdr + hdr->used_আকার;

	rec->tag_and_hdr_আকার = tag | আকার(*rec) << BLOBLISTR_HDR_আকার_SHIFT;
	rec->আকার_2 = আকার_2;

	/* Zero the record data */
	memset((শূন্য *)rec + rec_hdr_আকার(rec), '\0', rec->আকার_2);

	hdr->used_আকার = new_alloced;
	*recp = rec;

	ফেরত 0;
}

স্থির পূর্ণসংখ্যা bloblist_ensurerec(uint tag, কাঠামো bloblist_rec **recp, পূর্ণসংখ্যা আকার_2,
			      পূর্ণসংখ্যা align_log2)
{
	কাঠামো bloblist_rec *rec;

	rec = bloblist_findrec(tag);
	যদি (rec) {
		যদি (আকার_2 && আকার_2 != rec->আকার_2) {
			*recp = rec;
			ফেরত -ESPIPE;
		}
	} অন্যথা {
		পূর্ণসংখ্যা ret;

		ret = bloblist_addrec(tag, আকার_2, align_log2, &rec);
		যদি (ret)
			ফেরত ret;
	}
	*recp = rec;

	ফেরত 0;
}

স্থির পূর্ণসংখ্যা bloblist_get_blob_তথ্য_offset(uint tag)
{
	নির্বাচন (tag) {
	ক্ষেত্র BLOBLISTT_FDT_OVERLAY:
		ফেরত আকার(কাঠামো dto_blob_hdr);
	/*
	 * return the data offset if it is not following the blob
	 * header immediately.
	 */
	}
	ফেরত 0;
}

শূন্য *bloblist_find(uint tag, পূর্ণসংখ্যা আকার_2)
{
	শূন্য *blob = ফাঁকা;
	পূর্ণসংখ্যা blob_আকার;

	blob = bloblist_get_blob(tag, &blob_আকার);

	যদি (আকার_2 && আকার_2 != blob_আকার)
		ফেরত ফাঁকা;

	ফেরত blob;
}

শূন্য *bloblist_get_blob(uint tag, পূর্ণসংখ্যা *sizep)
{
	কাঠামো bloblist_rec *rec;

	rec = bloblist_findrec(tag);
	যদি (!rec)
		ফেরত ফাঁকা;

	*sizep = rec->আকার_2;

	ফেরত (শূন্য *)rec + rec_hdr_আকার(rec);
}

পূর্ণসংখ্যা bloblist_apply_blobs(uint tag, পূর্ণসংখ্যা (*func)(শূন্য **তথ্য, পূর্ণসংখ্যা আকার_2))
{
	কাঠামো bloblist_hdr *hdr = gd->bloblist;
	কাঠামো bloblist_rec *rec;

	যদি (!func || !hdr)
		ফেরত -ENOENT;

	foreach_rec(rec, hdr) {
		/* Apply all blobs with the specified tag */
		যদি (rec_tag(rec) == tag) {
			পূর্ণসংখ্যা ret;
			পূর্ণসংখ্যা tag = rec_tag(rec);
			শূন্য *blob = (শূন্য *)rec + rec_hdr_আকার(rec);
			পূর্ণসংখ্যা dat_off = bloblist_get_blob_তথ্য_offset(tag);

			blob += dat_off;
			ret = func(&blob, rec->আকার_2 - dat_off);
			যদি (ret) {
				log_err("Failed to apply blob with tag %d\n",
					tag);
				ফেরত ret;
			}

			rec = rec_from_blob(blob - dat_off);
			যদি (!rec) {
				log_err("Blob corrupted\n");
				ফেরত -ENOENT;
			}

			/* Mark applied blob record as void */
			void_blob(rec);
		}
	}

	ফেরত 0;
}

শূন্য *bloblist_add(uint tag, পূর্ণসংখ্যা আকার_2, পূর্ণসংখ্যা align_log2)
{
	কাঠামো bloblist_rec *rec;

	যদি (bloblist_addrec(tag, আকার_2, align_log2, &rec))
		ফেরত ফাঁকা;

	ফেরত (শূন্য *)rec + rec_hdr_আকার(rec);
}

পূর্ণসংখ্যা bloblist_ensure_আকার(uint tag, পূর্ণসংখ্যা আকার_2, পূর্ণসংখ্যা align_log2, শূন্য **blobp)
{
	কাঠামো bloblist_rec *rec;
	পূর্ণসংখ্যা ret;

	ret = bloblist_ensurerec(tag, &rec, আকার_2, align_log2);
	যদি (ret)
		ফেরত ret;
	*blobp = (শূন্য *)rec + rec_hdr_আকার(rec);

	ফেরত 0;
}

শূন্য *bloblist_ensure(uint tag, পূর্ণসংখ্যা আকার_2)
{
	কাঠামো bloblist_rec *rec;

	যদি (bloblist_ensurerec(tag, &rec, আকার_2, 0))
		ফেরত ফাঁকা;

	ফেরত (শূন্য *)rec + rec_hdr_আকার(rec);
}

পূর্ণসংখ্যা bloblist_ensure_আকার_ret(uint tag, পূর্ণসংখ্যা *sizep, শূন্য **blobp)
{
	কাঠামো bloblist_rec *rec;
	পূর্ণসংখ্যা ret;

	ret = bloblist_ensurerec(tag, &rec, *sizep, 0);
	যদি (ret == -ESPIPE)
		*sizep = rec->আকার_2;
	অন্যথা যদি (ret)
		ফেরত ret;
	*blobp = (শূন্য *)rec + rec_hdr_আকার(rec);

	ফেরত 0;
}

স্থির পূর্ণসংখ্যা bloblist_resize_rec(কাঠামো bloblist_hdr *hdr,
			       কাঠামো bloblist_rec *rec,
			       পূর্ণসংখ্যা new_আকার)
{
	পূর্ণসংখ্যা expand_by;	/* Number of bytes to expand by (-ve to contract) */
	পূর্ণসংখ্যা new_alloced;
	ulong পরবর্তী_ofs;	/* Offset of the record after @rec */

	expand_by = ALIGN(new_আকার - rec->আকার_2, BLOBLIST_BLOB_ALIGN);
	new_alloced = ALIGN(hdr->used_আকার + expand_by, BLOBLIST_BLOB_ALIGN);
	যদি (new_আকার < 0) {
		log_debug("Attempt to shrink blob size below 0 (%x)\n",
			  new_আকার);
		ফেরত log_বার্তা_ret("size", -EINVAL);
	}
	যদি (new_alloced > hdr->যোগ_আকার) {
		log_err("Failed to allocate %x bytes\n", new_আকার);
		log_err("Used size=%x, total size=%x\n",
			hdr->used_আকার, hdr->যোগ_আকার);
		ফেরত log_বার্তা_ret("alloc", -ENOSPC);
	}

	/* Move the following blobs up or down, if this is not the last */
	পরবর্তী_ofs = bloblist_blob_end_ofs(hdr, rec);
	যদি (পরবর্তী_ofs != hdr->used_আকার) {
		memmove((শূন্য *)hdr + পরবর্তী_ofs + expand_by,
			(শূন্য *)hdr + পরবর্তী_ofs, hdr->used_আকার - পরবর্তী_ofs);
	}
	hdr->used_আকার = new_alloced;

	/* Zero the new part of the blob */
	যদি (expand_by > 0) {
		memset((শূন্য *)rec + rec_hdr_আকার(rec) + rec->আকার_2, '\0',
		       new_আকার - rec->আকার_2);
	}

	/* Update the size of this blob */
	rec->আকার_2 = new_আকার;

	ফেরত 0;
}

পূর্ণসংখ্যা bloblist_resize(uint tag, পূর্ণসংখ্যা new_আকার)
{
	কাঠামো bloblist_hdr *hdr = gd->bloblist;
	কাঠামো bloblist_rec *rec;
	পূর্ণসংখ্যা ret;

	rec = bloblist_findrec(tag);
	যদি (!rec)
		ফেরত log_বার্তা_ret("find", -ENOENT);
	ret = bloblist_resize_rec(hdr, rec, new_আকার);
	যদি (ret)
		ফেরত log_বার্তা_ret("resize", ret);

	ফেরত 0;
}

স্থির u32 bloblist_calc_chksum(কাঠামো bloblist_hdr *hdr)
{
	u8 chksum;

	chksum = table_compute_checksum(hdr, hdr->used_আকার);
	chksum += hdr->chksum;

	ফেরত chksum;
}

পূর্ণসংখ্যা bloblist_new(ulong addr, uint আকার_2, uint flags, uint align_log2)
{
	কাঠামো bloblist_hdr *hdr;

	যদি (আকার_2 < আকার(*hdr))
		ফেরত log_ret(-ENOSPC);
	যদি (addr & (BLOBLIST_ALIGN - 1))
		ফেরত log_ret(-EFAULT);
	hdr = map_sysmem(addr, আকার_2);
	memset(hdr, '\0', আকার(*hdr));
	hdr->version = BLOBLIST_VERSION;
	hdr->hdr_আকার = আকার(*hdr);
	hdr->flags = flags;
	hdr->magic = BLOBLIST_MAGIC;
	hdr->used_আকার = hdr->hdr_আকার;
	hdr->যোগ_আকার = আকার_2;
	hdr->align_log2 = align_log2 ? align_log2 : BLOBLIST_BLOB_ALIGN_LOG2;
	hdr->chksum = 0;
	gd->bloblist = hdr;
	gd->flags |= GD_FLG_BLOBLIST_HANDOFF;

	ফেরত 0;
}

পূর্ণসংখ্যা bloblist_check(ulong addr, uint আকার_2)
{
	কাঠামো bloblist_hdr *hdr;
	u32 chksum;

	hdr = map_sysmem(addr, আকার(*hdr));
	যদি (hdr->magic != BLOBLIST_MAGIC)
		ফেরত log_বার্তা_ret("Bad magic", -ENOENT);
	যদি (hdr->version != BLOBLIST_VERSION)
		ফেরত log_বার্তা_ret("Bad version", -EPROTONOSUPPORT);
	যদি (!hdr->যোগ_আকার || (আকার_2 && hdr->যোগ_আকার > আকার_2))
		ফেরত log_বার্তা_ret("Bad total size", -EFBIG);
	যদি (hdr->used_আকার > hdr->যোগ_আকার)
		ফেরত log_বার্তা_ret("Bad used size", -ENOENT);
	যদি (hdr->hdr_আকার != আকার(কাঠামো bloblist_hdr))
		ফেরত log_বার্তা_ret("Bad header size", -ENOENT);

	chksum = bloblist_calc_chksum(hdr);
	যদি (hdr->chksum != chksum) {
		log_err("Checksum %x != %x\n", hdr->chksum, chksum);
		ফেরত log_বার্তা_ret("Bad checksum", -EIO);
	}
	gd->bloblist = hdr;
	gd->flags |= GD_FLG_BLOBLIST_HANDOFF;

	ফেরত 0;
}

পূর্ণসংখ্যা bloblist_finish(শূন্য)
{
	কাঠামো bloblist_hdr *hdr = gd->bloblist;

	hdr->chksum = bloblist_calc_chksum(hdr);
	log_debug("Finished bloblist size %lx at %lx\n", (ulong)hdr->used_আকার,
		  (ulong)map_to_sysmem(hdr));

	ফেরত 0;
}

ulong bloblist_get_base(শূন্য)
{
	ফেরত map_to_sysmem(gd->bloblist);
}

ulong bloblist_get_আকার(শূন্য)
{
	কাঠামো bloblist_hdr *hdr = gd->bloblist;

	ফেরত hdr->used_আকার;
}

ulong bloblist_get_যোগ_আকার(শূন্য)
{
	কাঠামো bloblist_hdr *hdr = gd->bloblist;

	ফেরত hdr->যোগ_আকার;
}

শূন্য bloblist_get_stats(ulong *basep, ulong *tsizep, ulong *usizep)
{
	কাঠামো bloblist_hdr *hdr = gd->bloblist;

	*basep = map_to_sysmem(gd->bloblist);
	*tsizep = hdr->যোগ_আকার;
	*usizep = hdr->used_আকার;
}

স্থির শূন্য show_মান(ধ্রুবক অক্ষর *prompt, ulong মান)
{
	printf("%s:%*s %-5lx  ", prompt, 10 - (পূর্ণসংখ্যা)strlen(prompt), "", মান);
	print_আকার(মান, "\n");
}

শূন্য bloblist_show_stats(শূন্য)
{
	ulong base, tsize, usize;

	bloblist_get_stats(&base, &tsize, &usize);
	printf("base:       %lx\n", base);
	show_মান("total size", tsize);
	show_মান("used size", usize);
	show_মান("free", tsize - usize);
}

শূন্য bloblist_show_list(শূন্য)
{
	কাঠামো bloblist_hdr *hdr = gd->bloblist;
	কাঠামো bloblist_rec *rec;

	printf("%-8s  %8s   Tag Name\n", "Address", "Size");
	জন্য (rec = bloblist_first_blob(hdr); rec;
	     rec = bloblist_পরবর্তী_blob(hdr, rec)) {
		printf("%08lx  %8x  %4x %s\n",
		       (ulong)map_to_sysmem((শূন্য *)rec + rec_hdr_আকার(rec)),
		       rec->আকার_2, rec_tag(rec),
		       bloblist_tag_name(rec_tag(rec)));
	}
}

পূর্ণসংখ্যা bloblist_reloc(শূন্য *to, uint to_আকার)
{
	কাঠামো bloblist_hdr *hdr;

	যদি (!to_আকার)
		ফেরত 0;

	যদি (to_আকার < gd->bloblist->যোগ_আকার)
		ফেরত -ENOSPC;

	memcpy(to, gd->bloblist, gd->bloblist->যোগ_আকার);
	hdr = to;
	hdr->যোগ_আকার = to_আকার;
	gd->bloblist = to;

	ফেরত 0;
}

/*
 * Weak default function for getting bloblist from boot args.
 */
পূর্ণসংখ্যা __weak xferlist_from_boot_arg(ulong __always_unused *addr)
{
	ফেরত -ENOENT;
}

bool bloblist_exists(শূন্য)
{
	পূর্ণসংখ্যা ret;
	ulong addr = 0;

	/* Check if a valid transfer list passed in */
	যদি (!xferlist_from_boot_arg(&addr))
		যাও found;

	/*
	 * If Firmware Handoff is mandatory but no transfer list is
	 * observed, report it as an error.
	 */
	যদি (IS_ENABLED(CONFIG_BLOBLIST_PASSAGE_MANDATORY))
		ফেরত false;

	/*
	 * We have checked for a valid transfer list being passed. At this
	 * point, if we do not have a fixed address for the bloblist, we cannot
	 * be provided with one.
	 */
	যদি (xpl_is_first_phase() || !IS_ENABLED(CONFIG_BLOBLIST_FIXED))
		ফেরত false;

	/*
	 * Check for a valid list as the configured address.
	 */
	addr = IF_ENABLED_INT(CONFIG_BLOBLIST_FIXED,
			      CONFIG_BLOBLIST_ADDR);
	ret = bloblist_check(addr, CONFIG_BLOBLIST_আকার);
	যদি (!ret)
		যাও found;

	log_debug("Bloblist at %lx not found (err=%d)\n", addr, ret);
	ফেরত false;

found:
#ifdef DEBUG
	bloblist_show_stats();
	bloblist_show_list();
#endif
	ফেরত true;
}

পূর্ণসংখ্যা bloblist_init(শূন্য)
{
	পূর্ণসংখ্যা ret;
	ulong addr = 0, আকার_2 = CONFIG_BLOBLIST_আকার;

	যদি (gd->flags & GD_FLG_BLOBLIST_HANDOFF) {
		log_debug("Found existing bloblist size %x at %p\n",
			  gd->bloblist->যোগ_আকার, gd->bloblist);
		ফেরত 0;
	}

	/*
	 * If Firmware Handoff is mandatory but no transfer list has been
	 * observed by fdtdec_setup, report it as an error.
	 */
	যদি (IS_ENABLED(CONFIG_BLOBLIST_PASSAGE_MANDATORY))
		ফেরত -ENOENT;

	/*
	 * If we don't have an existing bloblist, we either need
	 * to allocate one now, or initialize the fixed address
	 * space as a bloblist.
	 */
	যদি (CONFIG_IS_ENABLED(BLOBLIST_ALLOC)) {
		শূন্য *ptr = memalign(BLOBLIST_ALIGN, আকার_2);

		যদি (!ptr)
			ফেরত log_বার্তা_ret("alloc", -ENOMEM);
		addr = map_to_sysmem(ptr);
	} অন্যথা
		addr = IF_ENABLED_INT(CONFIG_BLOBLIST_FIXED,
				      CONFIG_BLOBLIST_ADDR);

	log_debug("Creating new bloblist size %lx at %lx\n", আকার_2,
		  addr);
	ret = bloblist_new(addr, আকার_2, 0, 0);
	যদি (ret)
		ফেরত log_বার্তা_ret("ini", ret);

	ফেরত 0;
}

পূর্ণসংখ্যা bloblist_check_reg_conv(ulong rfdt, ulong rzero, ulong rsig, ulong xlist)
{
	u64 version = BLOBLIST_REGCONV_VER;
	ulong sigval;
	পূর্ণসংখ্যা ret;

	যদি ((IS_ENABLED(CONFIG_64BIT) && !IS_ENABLED(CONFIG_SPL_BUILD)) ||
			(IS_ENABLED(CONFIG_SPL_64BIT) && IS_ENABLED(CONFIG_SPL_BUILD))) {
		sigval = ((BLOBLIST_MAGIC & ((1ULL << BLOBLIST_REGCONV_SHIFT_64) - 1)) |
			 ((version  & BLOBLIST_REGCONV_MASK) << BLOBLIST_REGCONV_SHIFT_64));
	} অন্যথা {
		sigval = ((BLOBLIST_MAGIC & ((1UL << BLOBLIST_REGCONV_SHIFT_32) - 1)) |
			 ((version  & BLOBLIST_REGCONV_MASK) << BLOBLIST_REGCONV_SHIFT_32));
	}

	যদি (rzero || rsig != sigval)
		ফেরত -EIO;

	ret = bloblist_check(xlist, 0);
	যদি (ret)
		ফেরত ret;

	যদি (rfdt != (ulong)bloblist_find(BLOBLISTT_CONTROL_FDT, 0)) {
		/* Remove this bloblist from gd */
		gd->bloblist = ফাঁকা;
		gd->flags &= ~GD_FLG_BLOBLIST_HANDOFF;
		ফেরত -EIO;
	}

	ফেরত 0;
}
