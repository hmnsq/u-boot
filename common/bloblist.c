// SPDX-License-Identifier: GPL-2.0+ BSD-3-Clause
/*
 * Copyright 2018 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY	LOGC_BLOBLIST

#অন্তর্ভাৱ <bloblist.h>
#অন্তর্ভাৱ <display_options.h>
#অন্তর্ভাৱ <log.h>
#অন্তর্ভাৱ <malloc.h>
#অন্তর্ভাৱ <mapmem.h>
#অন্তর্ভাৱ <spl.h>
#অন্তর্ভাৱ <tables_csum.h>
#অন্তর্ভাৱ <asm/global_data.h>
#অন্তর্ভাৱ <u-boot/crc.h>

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

DECLARE_GLOBAL_DATA_PTR;

স্থির সংরচনা tag_name {
	গণনা bloblist_tag_t tag;
	ধ্রুৱ ৱর্ণ *name;
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

ধ্রুৱ ৱর্ণ *bloblist_tag_name(গণনা bloblist_tag_t tag)
{
	পূর্ণাঙ্ক i;

	বাবে (i = 0; i < ARRAY_SIZE(tag_name); i++) {
		যদি (tag_name[i].tag == tag)
			ঘূৰাই tag_name[i].name;
	}

	ঘূৰাই "invalid";
}

স্থির সংরচনা bloblist_rec *bloblist_first_blob(সংরচনা bloblist_hdr *hdr)
{
	যদি (hdr->used_size <= hdr->hdr_size)
		ঘূৰাই রিক্ত;
	ঘূৰাই (সংরচনা bloblist_rec *)((শূন্য *)hdr + hdr->hdr_size);
}

স্থির অন্তঃস্থ uint rec_hdr_size(সংরচনা bloblist_rec *rec)
{
	ঘূৰাই (rec->tag_and_hdr_size & BLOBLISTR_HDR_SIZE_MASK) >>
		BLOBLISTR_HDR_SIZE_SHIFT;
}

স্থির অন্তঃস্থ uint rec_tag(সংরচনা bloblist_rec *rec)
{
	ঘূৰাই (rec->tag_and_hdr_size & BLOBLISTR_TAG_MASK) >>
		BLOBLISTR_TAG_SHIFT;
}

স্থির অন্তঃস্থ শূন্য void_blob(সংরচনা bloblist_rec *rec)
{
	যদি (rec_tag(rec) == BLOBLISTT_VOID)
		ঘূৰাই;
	rec->tag_and_hdr_size = BLOBLISTT_VOID |
				পরিমাণ(*rec) << BLOBLISTR_HDR_SIZE_SHIFT;
}

স্থির অন্তঃস্থ সংরচনা bloblist_rec *rec_from_blob(শূন্য *blob)
{
	ঘূৰাই (blob - পরিমাণ(সংরচনা bloblist_rec));
}

স্থির ulong bloblist_blob_end_ofs(সংরচনা bloblist_hdr *hdr,
				   সংরচনা bloblist_rec *rec)
{
	ulong offset;

	offset = (শূন্য *)rec - (শূন্য *)hdr;
	/*
	 * The data section of next TE should start from an address aligned
	 * to 1 << hdr->align_log2.
	 */
	offset += rec_hdr_size(rec) + rec->size;
	offset = round_up(offset + rec_hdr_size(rec), 1 << hdr->align_log2);
	offset -= rec_hdr_size(rec);

	ঘূৰাই offset;
}

স্থির সংরচনা bloblist_rec *bloblist_next_blob(সংরচনা bloblist_hdr *hdr,
					       সংরচনা bloblist_rec *rec)
{
	ulong offset = bloblist_blob_end_ofs(hdr, rec);

	যদি (offset >= hdr->used_size)
		ঘূৰাই রিক্ত;
	ঘূৰাই (সংরচনা bloblist_rec *)((শূন্য *)hdr + offset);
}

#define foreach_rec(_rec, _hdr) \
	বাবে (_rec = bloblist_first_blob(_hdr); \
	     _rec; \
	     _rec = bloblist_next_blob(_hdr, _rec))

স্থির সংরচনা bloblist_rec *bloblist_findrec(uint tag)
{
	সংরচনা bloblist_hdr *hdr = gd->bloblist;
	সংরচনা bloblist_rec *rec;

	যদি (!hdr)
		ঘূৰাই রিক্ত;

	foreach_rec(rec, hdr) {
		যদি (rec_tag(rec) == tag)
			ঘূৰাই rec;
	}

	ঘূৰাই রিক্ত;
}

স্থির পূর্ণাঙ্ক bloblist_addrec(uint tag, পূর্ণাঙ্ক size, পূর্ণাঙ্ক align_log2,
			   সংরচনা bloblist_rec **recp)
{
	সংরচনা bloblist_hdr *hdr = gd->bloblist;
	সংরচনা bloblist_rec *rec;
	phys_addr_t data_start, aligned_start;
	phys_size_t new_alloced;

	যদি (!align_log2)
		align_log2 = BLOBLIST_BLOB_ALIGN_LOG2;

	/* Figure out where the new data will start */
	data_start = map_to_sysmem(hdr) + hdr->used_size + পরিমাণ(*rec);

	/* Align the address and then calculate the offset from used size */
	aligned_start = ALIGN(data_start, 1U << align_log2) - data_start;

	/* If we need to create a dummy record, create it */
	যদি (aligned_start) {
		পূর্ণাঙ্ক void_size = aligned_start - পরিমাণ(*rec);
		সংরচনা bloblist_rec *vrec;
		পূর্ণাঙ্ক ret;

		ret = bloblist_addrec(BLOBLISTT_VOID, void_size, 0, &vrec);
		যদি (ret)
			ঘূৰাই log_বাৰ্তা_ret("void", ret);

		/* start the record after that */
		data_start = map_to_sysmem(hdr) + hdr->used_size + পরিমাণ(*vrec);
	}

	/* Calculate the new allocated total */
	new_alloced = data_start - map_to_sysmem(hdr) +
		ALIGN(size, 1U << align_log2);

	যদি (new_alloced > hdr->total_size) {
		log_err("Failed to allocate %x bytes\n", size);
		log_err("Used size=%x, total size=%x\n",
			hdr->used_size, hdr->total_size);
		ঘূৰাই log_বাৰ্তা_ret("bloblist add", -ENOSPC);
	}
	rec = (শূন্য *)hdr + hdr->used_size;

	rec->tag_and_hdr_size = tag | পরিমাণ(*rec) << BLOBLISTR_HDR_SIZE_SHIFT;
	rec->size = size;

	/* Zero the record data */
	memset((শূন্য *)rec + rec_hdr_size(rec), '\0', rec->size);

	hdr->used_size = new_alloced;
	*recp = rec;

	ঘূৰাই 0;
}

স্থির পূর্ণাঙ্ক bloblist_ensurerec(uint tag, সংরচনা bloblist_rec **recp, পূর্ণাঙ্ক size,
			      পূর্ণাঙ্ক align_log2)
{
	সংরচনা bloblist_rec *rec;

	rec = bloblist_findrec(tag);
	যদি (rec) {
		যদি (size && size != rec->size) {
			*recp = rec;
			ঘূৰাই -ESPIPE;
		}
	} অন্যথা {
		পূর্ণাঙ্ক ret;

		ret = bloblist_addrec(tag, size, align_log2, &rec);
		যদি (ret)
			ঘূৰাই ret;
	}
	*recp = rec;

	ঘূৰাই 0;
}

স্থির পূর্ণাঙ্ক bloblist_get_blob_data_offset(uint tag)
{
	ৱিকল্প (tag) {
	পক্ষ BLOBLISTT_FDT_OVERLAY:
		ঘূৰাই পরিমাণ(সংরচনা dto_blob_hdr);
	/*
	 * return the data offset if it is not following the blob
	 * header immediately.
	 */
	}
	ঘূৰাই 0;
}

শূন্য *bloblist_find(uint tag, পূর্ণাঙ্ক size)
{
	শূন্য *blob = রিক্ত;
	পূর্ণাঙ্ক blob_size;

	blob = bloblist_get_blob(tag, &blob_size);

	যদি (size && size != blob_size)
		ঘূৰাই রিক্ত;

	ঘূৰাই blob;
}

শূন্য *bloblist_get_blob(uint tag, পূর্ণাঙ্ক *sizep)
{
	সংরচনা bloblist_rec *rec;

	rec = bloblist_findrec(tag);
	যদি (!rec)
		ঘূৰাই রিক্ত;

	*sizep = rec->size;

	ঘূৰাই (শূন্য *)rec + rec_hdr_size(rec);
}

পূর্ণাঙ্ক bloblist_apply_blobs(uint tag, পূর্ণাঙ্ক (*func)(শূন্য **data, পূর্ণাঙ্ক size))
{
	সংরচনা bloblist_hdr *hdr = gd->bloblist;
	সংরচনা bloblist_rec *rec;

	যদি (!func || !hdr)
		ঘূৰাই -ENOENT;

	foreach_rec(rec, hdr) {
		/* Apply all blobs with the specified tag */
		যদি (rec_tag(rec) == tag) {
			পূর্ণাঙ্ক ret;
			পূর্ণাঙ্ক tag = rec_tag(rec);
			শূন্য *blob = (শূন্য *)rec + rec_hdr_size(rec);
			পূর্ণাঙ্ক dat_off = bloblist_get_blob_data_offset(tag);

			blob += dat_off;
			ret = func(&blob, rec->size - dat_off);
			যদি (ret) {
				log_err("Failed to apply blob with tag %d\n",
					tag);
				ঘূৰাই ret;
			}

			rec = rec_from_blob(blob - dat_off);
			যদি (!rec) {
				log_err("Blob corrupted\n");
				ঘূৰাই -ENOENT;
			}

			/* Mark applied blob record as void */
			void_blob(rec);
		}
	}

	ঘূৰাই 0;
}

শূন্য *bloblist_add(uint tag, পূর্ণাঙ্ক size, পূর্ণাঙ্ক align_log2)
{
	সংরচনা bloblist_rec *rec;

	যদি (bloblist_addrec(tag, size, align_log2, &rec))
		ঘূৰাই রিক্ত;

	ঘূৰাই (শূন্য *)rec + rec_hdr_size(rec);
}

পূর্ণাঙ্ক bloblist_ensure_size(uint tag, পূর্ণাঙ্ক size, পূর্ণাঙ্ক align_log2, শূন্য **blobp)
{
	সংরচনা bloblist_rec *rec;
	পূর্ণাঙ্ক ret;

	ret = bloblist_ensurerec(tag, &rec, size, align_log2);
	যদি (ret)
		ঘূৰাই ret;
	*blobp = (শূন্য *)rec + rec_hdr_size(rec);

	ঘূৰাই 0;
}

শূন্য *bloblist_ensure(uint tag, পূর্ণাঙ্ক size)
{
	সংরচনা bloblist_rec *rec;

	যদি (bloblist_ensurerec(tag, &rec, size, 0))
		ঘূৰাই রিক্ত;

	ঘূৰাই (শূন্য *)rec + rec_hdr_size(rec);
}

পূর্ণাঙ্ক bloblist_ensure_size_ret(uint tag, পূর্ণাঙ্ক *sizep, শূন্য **blobp)
{
	সংরচনা bloblist_rec *rec;
	পূর্ণাঙ্ক ret;

	ret = bloblist_ensurerec(tag, &rec, *sizep, 0);
	যদি (ret == -ESPIPE)
		*sizep = rec->size;
	অন্যথা যদি (ret)
		ঘূৰাই ret;
	*blobp = (শূন্য *)rec + rec_hdr_size(rec);

	ঘূৰাই 0;
}

স্থির পূর্ণাঙ্ক bloblist_resize_rec(সংরচনা bloblist_hdr *hdr,
			       সংরচনা bloblist_rec *rec,
			       পূর্ণাঙ্ক new_size)
{
	পূর্ণাঙ্ক expand_by;	/* Number of bytes to expand by (-ve to contract) */
	পূর্ণাঙ্ক new_alloced;
	ulong next_ofs;	/* Offset of the record after @rec */

	expand_by = ALIGN(new_size - rec->size, BLOBLIST_BLOB_ALIGN);
	new_alloced = ALIGN(hdr->used_size + expand_by, BLOBLIST_BLOB_ALIGN);
	যদি (new_size < 0) {
		log_debug("Attempt to shrink blob size below 0 (%x)\n",
			  new_size);
		ঘূৰাই log_বাৰ্তা_ret("size", -EINVAL);
	}
	যদি (new_alloced > hdr->total_size) {
		log_err("Failed to allocate %x bytes\n", new_size);
		log_err("Used size=%x, total size=%x\n",
			hdr->used_size, hdr->total_size);
		ঘূৰাই log_বাৰ্তা_ret("alloc", -ENOSPC);
	}

	/* Move the following blobs up or down, if this is not the last */
	next_ofs = bloblist_blob_end_ofs(hdr, rec);
	যদি (next_ofs != hdr->used_size) {
		memmove((শূন্য *)hdr + next_ofs + expand_by,
			(শূন্য *)hdr + next_ofs, hdr->used_size - next_ofs);
	}
	hdr->used_size = new_alloced;

	/* Zero the new part of the blob */
	যদি (expand_by > 0) {
		memset((শূন্য *)rec + rec_hdr_size(rec) + rec->size, '\0',
		       new_size - rec->size);
	}

	/* Update the size of this blob */
	rec->size = new_size;

	ঘূৰাই 0;
}

পূর্ণাঙ্ক bloblist_resize(uint tag, পূর্ণাঙ্ক new_size)
{
	সংরচনা bloblist_hdr *hdr = gd->bloblist;
	সংরচনা bloblist_rec *rec;
	পূর্ণাঙ্ক ret;

	rec = bloblist_findrec(tag);
	যদি (!rec)
		ঘূৰাই log_বাৰ্তা_ret("find", -ENOENT);
	ret = bloblist_resize_rec(hdr, rec, new_size);
	যদি (ret)
		ঘূৰাই log_বাৰ্তা_ret("resize", ret);

	ঘূৰাই 0;
}

স্থির u32 bloblist_calc_chksum(সংরচনা bloblist_hdr *hdr)
{
	u8 chksum;

	chksum = table_compute_checksum(hdr, hdr->used_size);
	chksum += hdr->chksum;

	ঘূৰাই chksum;
}

পূর্ণাঙ্ক bloblist_new(ulong addr, uint size, uint flags, uint align_log2)
{
	সংরচনা bloblist_hdr *hdr;

	যদি (size < পরিমাণ(*hdr))
		ঘূৰাই log_ret(-ENOSPC);
	যদি (addr & (BLOBLIST_ALIGN - 1))
		ঘূৰাই log_ret(-EFAULT);
	hdr = map_sysmem(addr, size);
	memset(hdr, '\0', পরিমাণ(*hdr));
	hdr->version = BLOBLIST_VERSION;
	hdr->hdr_size = পরিমাণ(*hdr);
	hdr->flags = flags;
	hdr->magic = BLOBLIST_MAGIC;
	hdr->used_size = hdr->hdr_size;
	hdr->total_size = size;
	hdr->align_log2 = align_log2 ? align_log2 : BLOBLIST_BLOB_ALIGN_LOG2;
	hdr->chksum = 0;
	gd->bloblist = hdr;
	gd->flags |= GD_FLG_BLOBLIST_HANDOFF;

	ঘূৰাই 0;
}

পূর্ণাঙ্ক bloblist_check(ulong addr, uint size)
{
	সংরচনা bloblist_hdr *hdr;
	u32 chksum;

	hdr = map_sysmem(addr, পরিমাণ(*hdr));
	যদি (hdr->magic != BLOBLIST_MAGIC)
		ঘূৰাই log_বাৰ্তা_ret("Bad magic", -ENOENT);
	যদি (hdr->version != BLOBLIST_VERSION)
		ঘূৰাই log_বাৰ্তা_ret("Bad version", -EPROTONOSUPPORT);
	যদি (!hdr->total_size || (size && hdr->total_size > size))
		ঘূৰাই log_বাৰ্তা_ret("Bad total size", -EFBIG);
	যদি (hdr->used_size > hdr->total_size)
		ঘূৰাই log_বাৰ্তা_ret("Bad used size", -ENOENT);
	যদি (hdr->hdr_size != পরিমাণ(সংরচনা bloblist_hdr))
		ঘূৰাই log_বাৰ্তা_ret("Bad header size", -ENOENT);

	chksum = bloblist_calc_chksum(hdr);
	যদি (hdr->chksum != chksum) {
		log_err("Checksum %x != %x\n", hdr->chksum, chksum);
		ঘূৰাই log_বাৰ্তা_ret("Bad checksum", -EIO);
	}
	gd->bloblist = hdr;
	gd->flags |= GD_FLG_BLOBLIST_HANDOFF;

	ঘূৰাই 0;
}

পূর্ণাঙ্ক bloblist_finish(শূন্য)
{
	সংরচনা bloblist_hdr *hdr = gd->bloblist;

	hdr->chksum = bloblist_calc_chksum(hdr);
	log_debug("Finished bloblist size %lx at %lx\n", (ulong)hdr->used_size,
		  (ulong)map_to_sysmem(hdr));

	ঘূৰাই 0;
}

ulong bloblist_get_base(শূন্য)
{
	ঘূৰাই map_to_sysmem(gd->bloblist);
}

ulong bloblist_get_size(শূন্য)
{
	সংরচনা bloblist_hdr *hdr = gd->bloblist;

	ঘূৰাই hdr->used_size;
}

ulong bloblist_get_total_size(শূন্য)
{
	সংরচনা bloblist_hdr *hdr = gd->bloblist;

	ঘূৰাই hdr->total_size;
}

শূন্য bloblist_get_stats(ulong *basep, ulong *tsizep, ulong *usizep)
{
	সংরচনা bloblist_hdr *hdr = gd->bloblist;

	*basep = map_to_sysmem(gd->bloblist);
	*tsizep = hdr->total_size;
	*usizep = hdr->used_size;
}

স্থির শূন্য show_মান(ধ্রুৱ ৱর্ণ *prompt, ulong মান)
{
	printf("%s:%*s %-5lx  ", prompt, 10 - (পূর্ণাঙ্ক)strlen(prompt), "", মান);
	print_size(মান, "\n");
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
	সংরচনা bloblist_hdr *hdr = gd->bloblist;
	সংরচনা bloblist_rec *rec;

	printf("%-8s  %8s   Tag Name\n", "Address", "Size");
	বাবে (rec = bloblist_first_blob(hdr); rec;
	     rec = bloblist_next_blob(hdr, rec)) {
		printf("%08lx  %8x  %4x %s\n",
		       (ulong)map_to_sysmem((শূন্য *)rec + rec_hdr_size(rec)),
		       rec->size, rec_tag(rec),
		       bloblist_tag_name(rec_tag(rec)));
	}
}

পূর্ণাঙ্ক bloblist_reloc(শূন্য *to, uint to_size)
{
	সংরচনা bloblist_hdr *hdr;

	যদি (!to_size)
		ঘূৰাই 0;

	যদি (to_size < gd->bloblist->total_size)
		ঘূৰাই -ENOSPC;

	memcpy(to, gd->bloblist, gd->bloblist->total_size);
	hdr = to;
	hdr->total_size = to_size;
	gd->bloblist = to;

	ঘূৰাই 0;
}

/*
 * Weak default function for getting bloblist from boot args.
 */
পূর্ণাঙ্ক __weak xferlist_from_boot_arg(ulong __always_unused *addr)
{
	ঘূৰাই -ENOENT;
}

bool bloblist_exists(শূন্য)
{
	পূর্ণাঙ্ক ret;
	ulong addr = 0;

	/* Check if a valid transfer list passed in */
	যদি (!xferlist_from_boot_arg(&addr))
		গচ্ছ found;

	/*
	 * If Firmware Handoff is mandatory but no transfer list is
	 * observed, report it as an error.
	 */
	যদি (IS_ENABLED(CONFIG_BLOBLIST_PASSAGE_MANDATORY))
		ঘূৰাই false;

	/*
	 * We have checked for a valid transfer list being passed. At this
	 * point, if we do not have a fixed address for the bloblist, we cannot
	 * be provided with one.
	 */
	যদি (xpl_is_first_phase() || !IS_ENABLED(CONFIG_BLOBLIST_FIXED))
		ঘূৰাই false;

	/*
	 * Check for a valid list as the configured address.
	 */
	addr = IF_ENABLED_INT(CONFIG_BLOBLIST_FIXED,
			      CONFIG_BLOBLIST_ADDR);
	ret = bloblist_check(addr, CONFIG_BLOBLIST_SIZE);
	যদি (!ret)
		গচ্ছ found;

	log_debug("Bloblist at %lx not found (err=%d)\n", addr, ret);
	ঘূৰাই false;

found:
#ifdef DEBUG
	bloblist_show_stats();
	bloblist_show_list();
#endif
	ঘূৰাই true;
}

পূর্ণাঙ্ক bloblist_init(শূন্য)
{
	পূর্ণাঙ্ক ret;
	ulong addr = 0, size = CONFIG_BLOBLIST_SIZE;

	যদি (gd->flags & GD_FLG_BLOBLIST_HANDOFF) {
		log_debug("Found existing bloblist size %x at %p\n",
			  gd->bloblist->total_size, gd->bloblist);
		ঘূৰাই 0;
	}

	/*
	 * If Firmware Handoff is mandatory but no transfer list has been
	 * observed by fdtdec_setup, report it as an error.
	 */
	যদি (IS_ENABLED(CONFIG_BLOBLIST_PASSAGE_MANDATORY))
		ঘূৰাই -ENOENT;

	/*
	 * If we don't have an existing bloblist, we either need
	 * to allocate one now, or initialize the fixed address
	 * space as a bloblist.
	 */
	যদি (CONFIG_IS_ENABLED(BLOBLIST_ALLOC)) {
		শূন্য *ptr = memalign(BLOBLIST_ALIGN, size);

		যদি (!ptr)
			ঘূৰাই log_বাৰ্তা_ret("alloc", -ENOMEM);
		addr = map_to_sysmem(ptr);
	} অন্যথা
		addr = IF_ENABLED_INT(CONFIG_BLOBLIST_FIXED,
				      CONFIG_BLOBLIST_ADDR);

	log_debug("Creating new bloblist size %lx at %lx\n", size,
		  addr);
	ret = bloblist_new(addr, size, 0, 0);
	যদি (ret)
		ঘূৰাই log_বাৰ্তা_ret("ini", ret);

	ঘূৰাই 0;
}

পূর্ণাঙ্ক bloblist_check_reg_conv(ulong rfdt, ulong rzero, ulong rsig, ulong xlist)
{
	u64 version = BLOBLIST_REGCONV_VER;
	ulong sigval;
	পূর্ণাঙ্ক ret;

	যদি ((IS_ENABLED(CONFIG_64BIT) && !IS_ENABLED(CONFIG_SPL_BUILD)) ||
			(IS_ENABLED(CONFIG_SPL_64BIT) && IS_ENABLED(CONFIG_SPL_BUILD))) {
		sigval = ((BLOBLIST_MAGIC & ((1ULL << BLOBLIST_REGCONV_SHIFT_64) - 1)) |
			 ((version  & BLOBLIST_REGCONV_MASK) << BLOBLIST_REGCONV_SHIFT_64));
	} অন্যথা {
		sigval = ((BLOBLIST_MAGIC & ((1UL << BLOBLIST_REGCONV_SHIFT_32) - 1)) |
			 ((version  & BLOBLIST_REGCONV_MASK) << BLOBLIST_REGCONV_SHIFT_32));
	}

	যদি (rzero || rsig != sigval)
		ঘূৰাই -EIO;

	ret = bloblist_check(xlist, 0);
	যদি (ret)
		ঘূৰাই ret;

	যদি (rfdt != (ulong)bloblist_find(BLOBLISTT_CONTROL_FDT, 0)) {
		/* Remove this bloblist from gd */
		gd->bloblist = রিক্ত;
		gd->flags &= ~GD_FLG_BLOBLIST_HANDOFF;
		ঘূৰাই -EIO;
	}

	ঘূৰাই 0;
}
