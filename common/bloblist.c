// SPDX-License-Identifier: GPL-2.0+ BSD-3-Clause
/*
 * Copyright 2018 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY	LOGC_BLOBLIST

#அந்தர்பாவ <bloblist.h>
#அந்தர்பாவ <display_options.h>
#அந்தர்பாவ <log.h>
#அந்தர்பாவ <malloc.h>
#அந்தர்பாவ <mapmem.h>
#அந்தர்பாவ <spl.h>
#அந்தர்பாவ <tables_csum.h>
#அந்தர்பாவ <asm/global_data.h>
#அந்தர்பாவ <u-boot/crc.h>

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

நிலையான கட்டமைப்பு tag_name {
	எண்ணிக்கை bloblist_tag_t tag;
	மாறிலி எழுத்து *name;
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

மாறிலி எழுத்து *bloblist_tag_name(எண்ணிக்கை bloblist_tag_t tag)
{
	முழு i;

	க்கு (i = 0; i < ARRAY_SIZE(tag_name); i++) {
		எனில் (tag_name[i].tag == tag)
			திரும்பு tag_name[i].name;
	}

	திரும்பு "invalid";
}

நிலையான கட்டமைப்பு bloblist_rec *bloblist_first_blob(கட்டமைப்பு bloblist_hdr *hdr)
{
	எனில் (hdr->used_size <= hdr->hdr_size)
		திரும்பு வெற்று;
	திரும்பு (கட்டமைப்பு bloblist_rec *)((வெறுமை *)hdr + hdr->hdr_size);
}

நிலையான உள்வரி uint rec_hdr_size(கட்டமைப்பு bloblist_rec *rec)
{
	திரும்பு (rec->tag_and_hdr_size & BLOBLISTR_HDR_SIZE_MASK) >>
		BLOBLISTR_HDR_SIZE_SHIFT;
}

நிலையான உள்வரி uint rec_tag(கட்டமைப்பு bloblist_rec *rec)
{
	திரும்பு (rec->tag_and_hdr_size & BLOBLISTR_TAG_MASK) >>
		BLOBLISTR_TAG_SHIFT;
}

நிலையான உள்வரி வெறுமை void_blob(கட்டமைப்பு bloblist_rec *rec)
{
	எனில் (rec_tag(rec) == BLOBLISTT_VOID)
		திரும்பு;
	rec->tag_and_hdr_size = BLOBLISTT_VOID |
				அளவு(*rec) << BLOBLISTR_HDR_SIZE_SHIFT;
}

நிலையான உள்வரி கட்டமைப்பு bloblist_rec *rec_from_blob(வெறுமை *blob)
{
	திரும்பு (blob - அளவு(கட்டமைப்பு bloblist_rec));
}

நிலையான ulong bloblist_blob_end_ofs(கட்டமைப்பு bloblist_hdr *hdr,
				   கட்டமைப்பு bloblist_rec *rec)
{
	ulong offset;

	offset = (வெறுமை *)rec - (வெறுமை *)hdr;
	/*
	 * The data section of next TE should start from an address aligned
	 * to 1 << hdr->align_log2.
	 */
	offset += rec_hdr_size(rec) + rec->size;
	offset = round_up(offset + rec_hdr_size(rec), 1 << hdr->align_log2);
	offset -= rec_hdr_size(rec);

	திரும்பு offset;
}

நிலையான கட்டமைப்பு bloblist_rec *bloblist_next_blob(கட்டமைப்பு bloblist_hdr *hdr,
					       கட்டமைப்பு bloblist_rec *rec)
{
	ulong offset = bloblist_blob_end_ofs(hdr, rec);

	எனில் (offset >= hdr->used_size)
		திரும்பு வெற்று;
	திரும்பு (கட்டமைப்பு bloblist_rec *)((வெறுமை *)hdr + offset);
}

#define foreach_rec(_rec, _hdr) \
	க்கு (_rec = bloblist_first_blob(_hdr); \
	     _rec; \
	     _rec = bloblist_next_blob(_hdr, _rec))

நிலையான கட்டமைப்பு bloblist_rec *bloblist_findrec(uint tag)
{
	கட்டமைப்பு bloblist_hdr *hdr = gd->bloblist;
	கட்டமைப்பு bloblist_rec *rec;

	எனில் (!hdr)
		திரும்பு வெற்று;

	foreach_rec(rec, hdr) {
		எனில் (rec_tag(rec) == tag)
			திரும்பு rec;
	}

	திரும்பு வெற்று;
}

நிலையான முழு bloblist_addrec(uint tag, முழு size, முழு align_log2,
			   கட்டமைப்பு bloblist_rec **recp)
{
	கட்டமைப்பு bloblist_hdr *hdr = gd->bloblist;
	கட்டமைப்பு bloblist_rec *rec;
	phys_addr_t data_start, aligned_start;
	phys_size_t new_alloced;

	எனில் (!align_log2)
		align_log2 = BLOBLIST_BLOB_ALIGN_LOG2;

	/* Figure out where the new data will start */
	data_start = map_to_sysmem(hdr) + hdr->used_size + அளவு(*rec);

	/* Align the address and then calculate the offset from used size */
	aligned_start = ALIGN(data_start, 1U << align_log2) - data_start;

	/* If we need to create a dummy record, create it */
	எனில் (aligned_start) {
		முழு void_size = aligned_start - அளவு(*rec);
		கட்டமைப்பு bloblist_rec *vrec;
		முழு ret;

		ret = bloblist_addrec(BLOBLISTT_VOID, void_size, 0, &vrec);
		எனில் (ret)
			திரும்பு log_செய்தி_ret("void", ret);

		/* start the record after that */
		data_start = map_to_sysmem(hdr) + hdr->used_size + அளவு(*vrec);
	}

	/* Calculate the new allocated total */
	new_alloced = data_start - map_to_sysmem(hdr) +
		ALIGN(size, 1U << align_log2);

	எனில் (new_alloced > hdr->total_size) {
		log_err("Failed to allocate %x bytes\n", size);
		log_err("Used size=%x, total size=%x\n",
			hdr->used_size, hdr->total_size);
		திரும்பு log_செய்தி_ret("bloblist add", -ENOSPC);
	}
	rec = (வெறுமை *)hdr + hdr->used_size;

	rec->tag_and_hdr_size = tag | அளவு(*rec) << BLOBLISTR_HDR_SIZE_SHIFT;
	rec->size = size;

	/* Zero the record data */
	memset((வெறுமை *)rec + rec_hdr_size(rec), '\0', rec->size);

	hdr->used_size = new_alloced;
	*recp = rec;

	திரும்பு 0;
}

நிலையான முழு bloblist_ensurerec(uint tag, கட்டமைப்பு bloblist_rec **recp, முழு size,
			      முழு align_log2)
{
	கட்டமைப்பு bloblist_rec *rec;

	rec = bloblist_findrec(tag);
	எனில் (rec) {
		எனில் (size && size != rec->size) {
			*recp = rec;
			திரும்பு -ESPIPE;
		}
	} இல்லையெனில் {
		முழு ret;

		ret = bloblist_addrec(tag, size, align_log2, &rec);
		எனில் (ret)
			திரும்பு ret;
	}
	*recp = rec;

	திரும்பு 0;
}

நிலையான முழு bloblist_get_blob_data_offset(uint tag)
{
	தேர்வு (tag) {
	நிலை BLOBLISTT_FDT_OVERLAY:
		திரும்பு அளவு(கட்டமைப்பு dto_blob_hdr);
	/*
	 * return the data offset if it is not following the blob
	 * header immediately.
	 */
	}
	திரும்பு 0;
}

வெறுமை *bloblist_find(uint tag, முழு size)
{
	வெறுமை *blob = வெற்று;
	முழு blob_size;

	blob = bloblist_get_blob(tag, &blob_size);

	எனில் (size && size != blob_size)
		திரும்பு வெற்று;

	திரும்பு blob;
}

வெறுமை *bloblist_get_blob(uint tag, முழு *sizep)
{
	கட்டமைப்பு bloblist_rec *rec;

	rec = bloblist_findrec(tag);
	எனில் (!rec)
		திரும்பு வெற்று;

	*sizep = rec->size;

	திரும்பு (வெறுமை *)rec + rec_hdr_size(rec);
}

முழு bloblist_apply_blobs(uint tag, முழு (*func)(வெறுமை **data, முழு size))
{
	கட்டமைப்பு bloblist_hdr *hdr = gd->bloblist;
	கட்டமைப்பு bloblist_rec *rec;

	எனில் (!func || !hdr)
		திரும்பு -ENOENT;

	foreach_rec(rec, hdr) {
		/* Apply all blobs with the specified tag */
		எனில் (rec_tag(rec) == tag) {
			முழு ret;
			முழு tag = rec_tag(rec);
			வெறுமை *blob = (வெறுமை *)rec + rec_hdr_size(rec);
			முழு dat_off = bloblist_get_blob_data_offset(tag);

			blob += dat_off;
			ret = func(&blob, rec->size - dat_off);
			எனில் (ret) {
				log_err("Failed to apply blob with tag %d\n",
					tag);
				திரும்பு ret;
			}

			rec = rec_from_blob(blob - dat_off);
			எனில் (!rec) {
				log_err("Blob corrupted\n");
				திரும்பு -ENOENT;
			}

			/* Mark applied blob record as void */
			void_blob(rec);
		}
	}

	திரும்பு 0;
}

வெறுமை *bloblist_add(uint tag, முழு size, முழு align_log2)
{
	கட்டமைப்பு bloblist_rec *rec;

	எனில் (bloblist_addrec(tag, size, align_log2, &rec))
		திரும்பு வெற்று;

	திரும்பு (வெறுமை *)rec + rec_hdr_size(rec);
}

முழு bloblist_ensure_size(uint tag, முழு size, முழு align_log2, வெறுமை **blobp)
{
	கட்டமைப்பு bloblist_rec *rec;
	முழு ret;

	ret = bloblist_ensurerec(tag, &rec, size, align_log2);
	எனில் (ret)
		திரும்பு ret;
	*blobp = (வெறுமை *)rec + rec_hdr_size(rec);

	திரும்பு 0;
}

வெறுமை *bloblist_ensure(uint tag, முழு size)
{
	கட்டமைப்பு bloblist_rec *rec;

	எனில் (bloblist_ensurerec(tag, &rec, size, 0))
		திரும்பு வெற்று;

	திரும்பு (வெறுமை *)rec + rec_hdr_size(rec);
}

முழு bloblist_ensure_size_ret(uint tag, முழு *sizep, வெறுமை **blobp)
{
	கட்டமைப்பு bloblist_rec *rec;
	முழு ret;

	ret = bloblist_ensurerec(tag, &rec, *sizep, 0);
	எனில் (ret == -ESPIPE)
		*sizep = rec->size;
	இல்லையெனில் எனில் (ret)
		திரும்பு ret;
	*blobp = (வெறுமை *)rec + rec_hdr_size(rec);

	திரும்பு 0;
}

நிலையான முழு bloblist_resize_rec(கட்டமைப்பு bloblist_hdr *hdr,
			       கட்டமைப்பு bloblist_rec *rec,
			       முழு new_size)
{
	முழு expand_by;	/* Number of bytes to expand by (-ve to contract) */
	முழு new_alloced;
	ulong next_ofs;	/* Offset of the record after @rec */

	expand_by = ALIGN(new_size - rec->size, BLOBLIST_BLOB_ALIGN);
	new_alloced = ALIGN(hdr->used_size + expand_by, BLOBLIST_BLOB_ALIGN);
	எனில் (new_size < 0) {
		log_debug("Attempt to shrink blob size below 0 (%x)\n",
			  new_size);
		திரும்பு log_செய்தி_ret("size", -EINVAL);
	}
	எனில் (new_alloced > hdr->total_size) {
		log_err("Failed to allocate %x bytes\n", new_size);
		log_err("Used size=%x, total size=%x\n",
			hdr->used_size, hdr->total_size);
		திரும்பு log_செய்தி_ret("alloc", -ENOSPC);
	}

	/* Move the following blobs up or down, if this is not the last */
	next_ofs = bloblist_blob_end_ofs(hdr, rec);
	எனில் (next_ofs != hdr->used_size) {
		memmove((வெறுமை *)hdr + next_ofs + expand_by,
			(வெறுமை *)hdr + next_ofs, hdr->used_size - next_ofs);
	}
	hdr->used_size = new_alloced;

	/* Zero the new part of the blob */
	எனில் (expand_by > 0) {
		memset((வெறுமை *)rec + rec_hdr_size(rec) + rec->size, '\0',
		       new_size - rec->size);
	}

	/* Update the size of this blob */
	rec->size = new_size;

	திரும்பு 0;
}

முழு bloblist_resize(uint tag, முழு new_size)
{
	கட்டமைப்பு bloblist_hdr *hdr = gd->bloblist;
	கட்டமைப்பு bloblist_rec *rec;
	முழு ret;

	rec = bloblist_findrec(tag);
	எனில் (!rec)
		திரும்பு log_செய்தி_ret("find", -ENOENT);
	ret = bloblist_resize_rec(hdr, rec, new_size);
	எனில் (ret)
		திரும்பு log_செய்தி_ret("resize", ret);

	திரும்பு 0;
}

நிலையான u32 bloblist_calc_chksum(கட்டமைப்பு bloblist_hdr *hdr)
{
	u8 chksum;

	chksum = table_compute_checksum(hdr, hdr->used_size);
	chksum += hdr->chksum;

	திரும்பு chksum;
}

முழு bloblist_new(ulong addr, uint size, uint flags, uint align_log2)
{
	கட்டமைப்பு bloblist_hdr *hdr;

	எனில் (size < அளவு(*hdr))
		திரும்பு log_ret(-ENOSPC);
	எனில் (addr & (BLOBLIST_ALIGN - 1))
		திரும்பு log_ret(-EFAULT);
	hdr = map_sysmem(addr, size);
	memset(hdr, '\0', அளவு(*hdr));
	hdr->version = BLOBLIST_VERSION;
	hdr->hdr_size = அளவு(*hdr);
	hdr->flags = flags;
	hdr->magic = BLOBLIST_MAGIC;
	hdr->used_size = hdr->hdr_size;
	hdr->total_size = size;
	hdr->align_log2 = align_log2 ? align_log2 : BLOBLIST_BLOB_ALIGN_LOG2;
	hdr->chksum = 0;
	gd->bloblist = hdr;
	gd->flags |= GD_FLG_BLOBLIST_HANDOFF;

	திரும்பு 0;
}

முழு bloblist_check(ulong addr, uint size)
{
	கட்டமைப்பு bloblist_hdr *hdr;
	u32 chksum;

	hdr = map_sysmem(addr, அளவு(*hdr));
	எனில் (hdr->magic != BLOBLIST_MAGIC)
		திரும்பு log_செய்தி_ret("Bad magic", -ENOENT);
	எனில் (hdr->version != BLOBLIST_VERSION)
		திரும்பு log_செய்தி_ret("Bad version", -EPROTONOSUPPORT);
	எனில் (!hdr->total_size || (size && hdr->total_size > size))
		திரும்பு log_செய்தி_ret("Bad total size", -EFBIG);
	எனில் (hdr->used_size > hdr->total_size)
		திரும்பு log_செய்தி_ret("Bad used size", -ENOENT);
	எனில் (hdr->hdr_size != அளவு(கட்டமைப்பு bloblist_hdr))
		திரும்பு log_செய்தி_ret("Bad header size", -ENOENT);

	chksum = bloblist_calc_chksum(hdr);
	எனில் (hdr->chksum != chksum) {
		log_err("Checksum %x != %x\n", hdr->chksum, chksum);
		திரும்பு log_செய்தி_ret("Bad checksum", -EIO);
	}
	gd->bloblist = hdr;
	gd->flags |= GD_FLG_BLOBLIST_HANDOFF;

	திரும்பு 0;
}

முழு bloblist_finish(வெறுமை)
{
	கட்டமைப்பு bloblist_hdr *hdr = gd->bloblist;

	hdr->chksum = bloblist_calc_chksum(hdr);
	log_debug("Finished bloblist size %lx at %lx\n", (ulong)hdr->used_size,
		  (ulong)map_to_sysmem(hdr));

	திரும்பு 0;
}

ulong bloblist_get_base(வெறுமை)
{
	திரும்பு map_to_sysmem(gd->bloblist);
}

ulong bloblist_get_size(வெறுமை)
{
	கட்டமைப்பு bloblist_hdr *hdr = gd->bloblist;

	திரும்பு hdr->used_size;
}

ulong bloblist_get_total_size(வெறுமை)
{
	கட்டமைப்பு bloblist_hdr *hdr = gd->bloblist;

	திரும்பு hdr->total_size;
}

வெறுமை bloblist_get_stats(ulong *basep, ulong *tsizep, ulong *usizep)
{
	கட்டமைப்பு bloblist_hdr *hdr = gd->bloblist;

	*basep = map_to_sysmem(gd->bloblist);
	*tsizep = hdr->total_size;
	*usizep = hdr->used_size;
}

நிலையான வெறுமை show_மதிப்பு(மாறிலி எழுத்து *prompt, ulong மதிப்பு)
{
	printf("%s:%*s %-5lx  ", prompt, 10 - (முழு)strlen(prompt), "", மதிப்பு);
	print_size(மதிப்பு, "\n");
}

வெறுமை bloblist_show_stats(வெறுமை)
{
	ulong base, tsize, usize;

	bloblist_get_stats(&base, &tsize, &usize);
	printf("base:       %lx\n", base);
	show_மதிப்பு("total size", tsize);
	show_மதிப்பு("used size", usize);
	show_மதிப்பு("free", tsize - usize);
}

வெறுமை bloblist_show_list(வெறுமை)
{
	கட்டமைப்பு bloblist_hdr *hdr = gd->bloblist;
	கட்டமைப்பு bloblist_rec *rec;

	printf("%-8s  %8s   Tag Name\n", "Address", "Size");
	க்கு (rec = bloblist_first_blob(hdr); rec;
	     rec = bloblist_next_blob(hdr, rec)) {
		printf("%08lx  %8x  %4x %s\n",
		       (ulong)map_to_sysmem((வெறுமை *)rec + rec_hdr_size(rec)),
		       rec->size, rec_tag(rec),
		       bloblist_tag_name(rec_tag(rec)));
	}
}

முழு bloblist_reloc(வெறுமை *to, uint to_size)
{
	கட்டமைப்பு bloblist_hdr *hdr;

	எனில் (!to_size)
		திரும்பு 0;

	எனில் (to_size < gd->bloblist->total_size)
		திரும்பு -ENOSPC;

	memcpy(to, gd->bloblist, gd->bloblist->total_size);
	hdr = to;
	hdr->total_size = to_size;
	gd->bloblist = to;

	திரும்பு 0;
}

/*
 * Weak default function for getting bloblist from boot args.
 */
முழு __weak xferlist_from_boot_arg(ulong __always_unused *addr)
{
	திரும்பு -ENOENT;
}

bool bloblist_exists(வெறுமை)
{
	முழு ret;
	ulong addr = 0;

	/* Check if a valid transfer list passed in */
	எனில் (!xferlist_from_boot_arg(&addr))
		செல் found;

	/*
	 * If Firmware Handoff is mandatory but no transfer list is
	 * observed, report it as an error.
	 */
	எனில் (IS_ENABLED(CONFIG_BLOBLIST_PASSAGE_MANDATORY))
		திரும்பு false;

	/*
	 * We have checked for a valid transfer list being passed. At this
	 * point, if we do not have a fixed address for the bloblist, we cannot
	 * be provided with one.
	 */
	எனில் (xpl_is_first_phase() || !IS_ENABLED(CONFIG_BLOBLIST_FIXED))
		திரும்பு false;

	/*
	 * Check for a valid list as the configured address.
	 */
	addr = IF_ENABLED_INT(CONFIG_BLOBLIST_FIXED,
			      CONFIG_BLOBLIST_ADDR);
	ret = bloblist_check(addr, CONFIG_BLOBLIST_SIZE);
	எனில் (!ret)
		செல் found;

	log_debug("Bloblist at %lx not found (err=%d)\n", addr, ret);
	திரும்பு false;

found:
#ifdef DEBUG
	bloblist_show_stats();
	bloblist_show_list();
#endif
	திரும்பு true;
}

முழு bloblist_init(வெறுமை)
{
	முழு ret;
	ulong addr = 0, size = CONFIG_BLOBLIST_SIZE;

	எனில் (gd->flags & GD_FLG_BLOBLIST_HANDOFF) {
		log_debug("Found existing bloblist size %x at %p\n",
			  gd->bloblist->total_size, gd->bloblist);
		திரும்பு 0;
	}

	/*
	 * If Firmware Handoff is mandatory but no transfer list has been
	 * observed by fdtdec_setup, report it as an error.
	 */
	எனில் (IS_ENABLED(CONFIG_BLOBLIST_PASSAGE_MANDATORY))
		திரும்பு -ENOENT;

	/*
	 * If we don't have an existing bloblist, we either need
	 * to allocate one now, or initialize the fixed address
	 * space as a bloblist.
	 */
	எனில் (CONFIG_IS_ENABLED(BLOBLIST_ALLOC)) {
		வெறுமை *ptr = memalign(BLOBLIST_ALIGN, size);

		எனில் (!ptr)
			திரும்பு log_செய்தி_ret("alloc", -ENOMEM);
		addr = map_to_sysmem(ptr);
	} இல்லையெனில்
		addr = IF_ENABLED_INT(CONFIG_BLOBLIST_FIXED,
				      CONFIG_BLOBLIST_ADDR);

	log_debug("Creating new bloblist size %lx at %lx\n", size,
		  addr);
	ret = bloblist_new(addr, size, 0, 0);
	எனில் (ret)
		திரும்பு log_செய்தி_ret("ini", ret);

	திரும்பு 0;
}

முழு bloblist_check_reg_conv(ulong rfdt, ulong rzero, ulong rsig, ulong xlist)
{
	u64 version = BLOBLIST_REGCONV_VER;
	ulong sigval;
	முழு ret;

	எனில் ((IS_ENABLED(CONFIG_64BIT) && !IS_ENABLED(CONFIG_SPL_BUILD)) ||
			(IS_ENABLED(CONFIG_SPL_64BIT) && IS_ENABLED(CONFIG_SPL_BUILD))) {
		sigval = ((BLOBLIST_MAGIC & ((1ULL << BLOBLIST_REGCONV_SHIFT_64) - 1)) |
			 ((version  & BLOBLIST_REGCONV_MASK) << BLOBLIST_REGCONV_SHIFT_64));
	} இல்லையெனில் {
		sigval = ((BLOBLIST_MAGIC & ((1UL << BLOBLIST_REGCONV_SHIFT_32) - 1)) |
			 ((version  & BLOBLIST_REGCONV_MASK) << BLOBLIST_REGCONV_SHIFT_32));
	}

	எனில் (rzero || rsig != sigval)
		திரும்பு -EIO;

	ret = bloblist_check(xlist, 0);
	எனில் (ret)
		திரும்பு ret;

	எனில் (rfdt != (ulong)bloblist_find(BLOBLISTT_CONTROL_FDT, 0)) {
		/* Remove this bloblist from gd */
		gd->bloblist = வெற்று;
		gd->flags &= ~GD_FLG_BLOBLIST_HANDOFF;
		திரும்பு -EIO;
	}

	திரும்பு 0;
}
