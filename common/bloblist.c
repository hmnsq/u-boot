// SPDX-License-Identifier: GPL-2.0+ BSD-3-Clause
/*
 * Copyright 2018 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY	LOGC_BLOBLIST

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

DECLARE_GLOBAL_DATA_PTR;

ایستا ساختار tag_name {
	شمارش bloblist_tag_t tag;
	ثابت نویسه *name;
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

ثابت نویسه *bloblist_tag_name(شمارش bloblist_tag_t tag)
{
	صحیح i;

	برای (i = 0; i < ARRAY_SIZE(tag_name); i++) {
		اگر (tag_name[i].tag == tag)
			بازگشت tag_name[i].name;
	}

	بازگشت "invalid";
}

ایستا ساختار bloblist_rec *bloblist_first_blob(ساختار bloblist_hdr *hdr)
{
	اگر (hdr->used_size <= hdr->hdr_size)
		بازگشت پوچ;
	بازگشت (ساختار bloblist_rec *)((تهی *)hdr + hdr->hdr_size);
}

ایستا درونخطی uint rec_hdr_size(ساختار bloblist_rec *rec)
{
	بازگشت (rec->tag_and_hdr_size & BLOBLISTR_HDR_SIZE_MASK) >>
		BLOBLISTR_HDR_SIZE_SHIFT;
}

ایستا درونخطی uint rec_tag(ساختار bloblist_rec *rec)
{
	بازگشت (rec->tag_and_hdr_size & BLOBLISTR_TAG_MASK) >>
		BLOBLISTR_TAG_SHIFT;
}

ایستا درونخطی تهی void_blob(ساختار bloblist_rec *rec)
{
	اگر (rec_tag(rec) == BLOBLISTT_VOID)
		بازگشت;
	rec->tag_and_hdr_size = BLOBLISTT_VOID |
				اندازه(*rec) << BLOBLISTR_HDR_SIZE_SHIFT;
}

ایستا درونخطی ساختار bloblist_rec *rec_from_blob(تهی *blob)
{
	بازگشت (blob - اندازه(ساختار bloblist_rec));
}

ایستا ulong bloblist_blob_end_ofs(ساختار bloblist_hdr *hdr,
				   ساختار bloblist_rec *rec)
{
	ulong offset;

	offset = (تهی *)rec - (تهی *)hdr;
	/*
	 * The data section of next TE should start from an address aligned
	 * to 1 << hdr->align_log2.
	 */
	offset += rec_hdr_size(rec) + rec->size;
	offset = round_up(offset + rec_hdr_size(rec), 1 << hdr->align_log2);
	offset -= rec_hdr_size(rec);

	بازگشت offset;
}

ایستا ساختار bloblist_rec *bloblist_next_blob(ساختار bloblist_hdr *hdr,
					       ساختار bloblist_rec *rec)
{
	ulong offset = bloblist_blob_end_ofs(hdr, rec);

	اگر (offset >= hdr->used_size)
		بازگشت پوچ;
	بازگشت (ساختار bloblist_rec *)((تهی *)hdr + offset);
}

#define foreach_rec(_rec, _hdr) \
	برای (_rec = bloblist_first_blob(_hdr); \
	     _rec; \
	     _rec = bloblist_next_blob(_hdr, _rec))

ایستا ساختار bloblist_rec *bloblist_findrec(uint tag)
{
	ساختار bloblist_hdr *hdr = gd->bloblist;
	ساختار bloblist_rec *rec;

	اگر (!hdr)
		بازگشت پوچ;

	foreach_rec(rec, hdr) {
		اگر (rec_tag(rec) == tag)
			بازگشت rec;
	}

	بازگشت پوچ;
}

ایستا صحیح bloblist_addrec(uint tag, صحیح size, صحیح align_log2,
			   ساختار bloblist_rec **recp)
{
	ساختار bloblist_hdr *hdr = gd->bloblist;
	ساختار bloblist_rec *rec;
	phys_addr_t data_start, aligned_start;
	phys_size_t new_alloced;

	اگر (!align_log2)
		align_log2 = BLOBLIST_BLOB_ALIGN_LOG2;

	/* Figure out where the new data will start */
	data_start = map_to_sysmem(hdr) + hdr->used_size + اندازه(*rec);

	/* Align the address and then calculate the offset from used size */
	aligned_start = ALIGN(data_start, 1U << align_log2) - data_start;

	/* If we need to create a dummy record, create it */
	اگر (aligned_start) {
		صحیح void_size = aligned_start - اندازه(*rec);
		ساختار bloblist_rec *vrec;
		صحیح ret;

		ret = bloblist_addrec(BLOBLISTT_VOID, void_size, 0, &vrec);
		اگر (ret)
			بازگشت log_پیام_ret("void", ret);

		/* start the record after that */
		data_start = map_to_sysmem(hdr) + hdr->used_size + اندازه(*vrec);
	}

	/* Calculate the new allocated total */
	new_alloced = data_start - map_to_sysmem(hdr) +
		ALIGN(size, 1U << align_log2);

	اگر (new_alloced > hdr->total_size) {
		log_err("Failed to allocate %x bytes\n", size);
		log_err("Used size=%x, total size=%x\n",
			hdr->used_size, hdr->total_size);
		بازگشت log_پیام_ret("bloblist add", -ENOSPC);
	}
	rec = (تهی *)hdr + hdr->used_size;

	rec->tag_and_hdr_size = tag | اندازه(*rec) << BLOBLISTR_HDR_SIZE_SHIFT;
	rec->size = size;

	/* Zero the record data */
	memset((تهی *)rec + rec_hdr_size(rec), '\0', rec->size);

	hdr->used_size = new_alloced;
	*recp = rec;

	بازگشت 0;
}

ایستا صحیح bloblist_ensurerec(uint tag, ساختار bloblist_rec **recp, صحیح size,
			      صحیح align_log2)
{
	ساختار bloblist_rec *rec;

	rec = bloblist_findrec(tag);
	اگر (rec) {
		اگر (size && size != rec->size) {
			*recp = rec;
			بازگشت -ESPIPE;
		}
	} وگرنه {
		صحیح ret;

		ret = bloblist_addrec(tag, size, align_log2, &rec);
		اگر (ret)
			بازگشت ret;
	}
	*recp = rec;

	بازگشت 0;
}

ایستا صحیح bloblist_get_blob_data_offset(uint tag)
{
	انتخاب (tag) {
	حالت BLOBLISTT_FDT_OVERLAY:
		بازگشت اندازه(ساختار dto_blob_hdr);
	/*
	 * return the data offset if it is not following the blob
	 * header immediately.
	 */
	}
	بازگشت 0;
}

تهی *bloblist_find(uint tag, صحیح size)
{
	تهی *blob = پوچ;
	صحیح blob_size;

	blob = bloblist_get_blob(tag, &blob_size);

	اگر (size && size != blob_size)
		بازگشت پوچ;

	بازگشت blob;
}

تهی *bloblist_get_blob(uint tag, صحیح *sizep)
{
	ساختار bloblist_rec *rec;

	rec = bloblist_findrec(tag);
	اگر (!rec)
		بازگشت پوچ;

	*sizep = rec->size;

	بازگشت (تهی *)rec + rec_hdr_size(rec);
}

صحیح bloblist_apply_blobs(uint tag, صحیح (*func)(تهی **data, صحیح size))
{
	ساختار bloblist_hdr *hdr = gd->bloblist;
	ساختار bloblist_rec *rec;

	اگر (!func || !hdr)
		بازگشت -ENOENT;

	foreach_rec(rec, hdr) {
		/* Apply all blobs with the specified tag */
		اگر (rec_tag(rec) == tag) {
			صحیح ret;
			صحیح tag = rec_tag(rec);
			تهی *blob = (تهی *)rec + rec_hdr_size(rec);
			صحیح dat_off = bloblist_get_blob_data_offset(tag);

			blob += dat_off;
			ret = func(&blob, rec->size - dat_off);
			اگر (ret) {
				log_err("Failed to apply blob with tag %d\n",
					tag);
				بازگشت ret;
			}

			rec = rec_from_blob(blob - dat_off);
			اگر (!rec) {
				log_err("Blob corrupted\n");
				بازگشت -ENOENT;
			}

			/* Mark applied blob record as void */
			void_blob(rec);
		}
	}

	بازگشت 0;
}

تهی *bloblist_add(uint tag, صحیح size, صحیح align_log2)
{
	ساختار bloblist_rec *rec;

	اگر (bloblist_addrec(tag, size, align_log2, &rec))
		بازگشت پوچ;

	بازگشت (تهی *)rec + rec_hdr_size(rec);
}

صحیح bloblist_ensure_size(uint tag, صحیح size, صحیح align_log2, تهی **blobp)
{
	ساختار bloblist_rec *rec;
	صحیح ret;

	ret = bloblist_ensurerec(tag, &rec, size, align_log2);
	اگر (ret)
		بازگشت ret;
	*blobp = (تهی *)rec + rec_hdr_size(rec);

	بازگشت 0;
}

تهی *bloblist_ensure(uint tag, صحیح size)
{
	ساختار bloblist_rec *rec;

	اگر (bloblist_ensurerec(tag, &rec, size, 0))
		بازگشت پوچ;

	بازگشت (تهی *)rec + rec_hdr_size(rec);
}

صحیح bloblist_ensure_size_ret(uint tag, صحیح *sizep, تهی **blobp)
{
	ساختار bloblist_rec *rec;
	صحیح ret;

	ret = bloblist_ensurerec(tag, &rec, *sizep, 0);
	اگر (ret == -ESPIPE)
		*sizep = rec->size;
	وگرنه اگر (ret)
		بازگشت ret;
	*blobp = (تهی *)rec + rec_hdr_size(rec);

	بازگشت 0;
}

ایستا صحیح bloblist_resize_rec(ساختار bloblist_hdr *hdr,
			       ساختار bloblist_rec *rec,
			       صحیح new_size)
{
	صحیح expand_by;	/* Number of bytes to expand by (-ve to contract) */
	صحیح new_alloced;
	ulong next_ofs;	/* Offset of the record after @rec */

	expand_by = ALIGN(new_size - rec->size, BLOBLIST_BLOB_ALIGN);
	new_alloced = ALIGN(hdr->used_size + expand_by, BLOBLIST_BLOB_ALIGN);
	اگر (new_size < 0) {
		log_debug("Attempt to shrink blob size below 0 (%x)\n",
			  new_size);
		بازگشت log_پیام_ret("size", -EINVAL);
	}
	اگر (new_alloced > hdr->total_size) {
		log_err("Failed to allocate %x bytes\n", new_size);
		log_err("Used size=%x, total size=%x\n",
			hdr->used_size, hdr->total_size);
		بازگشت log_پیام_ret("alloc", -ENOSPC);
	}

	/* Move the following blobs up or down, if this is not the last */
	next_ofs = bloblist_blob_end_ofs(hdr, rec);
	اگر (next_ofs != hdr->used_size) {
		memmove((تهی *)hdr + next_ofs + expand_by,
			(تهی *)hdr + next_ofs, hdr->used_size - next_ofs);
	}
	hdr->used_size = new_alloced;

	/* Zero the new part of the blob */
	اگر (expand_by > 0) {
		memset((تهی *)rec + rec_hdr_size(rec) + rec->size, '\0',
		       new_size - rec->size);
	}

	/* Update the size of this blob */
	rec->size = new_size;

	بازگشت 0;
}

صحیح bloblist_resize(uint tag, صحیح new_size)
{
	ساختار bloblist_hdr *hdr = gd->bloblist;
	ساختار bloblist_rec *rec;
	صحیح ret;

	rec = bloblist_findrec(tag);
	اگر (!rec)
		بازگشت log_پیام_ret("find", -ENOENT);
	ret = bloblist_resize_rec(hdr, rec, new_size);
	اگر (ret)
		بازگشت log_پیام_ret("resize", ret);

	بازگشت 0;
}

ایستا u32 bloblist_calc_chksum(ساختار bloblist_hdr *hdr)
{
	u8 chksum;

	chksum = table_compute_checksum(hdr, hdr->used_size);
	chksum += hdr->chksum;

	بازگشت chksum;
}

صحیح bloblist_new(ulong addr, uint size, uint flags, uint align_log2)
{
	ساختار bloblist_hdr *hdr;

	اگر (size < اندازه(*hdr))
		بازگشت log_ret(-ENOSPC);
	اگر (addr & (BLOBLIST_ALIGN - 1))
		بازگشت log_ret(-EFAULT);
	hdr = map_sysmem(addr, size);
	memset(hdr, '\0', اندازه(*hdr));
	hdr->version = BLOBLIST_VERSION;
	hdr->hdr_size = اندازه(*hdr);
	hdr->flags = flags;
	hdr->magic = BLOBLIST_MAGIC;
	hdr->used_size = hdr->hdr_size;
	hdr->total_size = size;
	hdr->align_log2 = align_log2 ? align_log2 : BLOBLIST_BLOB_ALIGN_LOG2;
	hdr->chksum = 0;
	gd->bloblist = hdr;
	gd->flags |= GD_FLG_BLOBLIST_HANDOFF;

	بازگشت 0;
}

صحیح bloblist_check(ulong addr, uint size)
{
	ساختار bloblist_hdr *hdr;
	u32 chksum;

	hdr = map_sysmem(addr, اندازه(*hdr));
	اگر (hdr->magic != BLOBLIST_MAGIC)
		بازگشت log_پیام_ret("Bad magic", -ENOENT);
	اگر (hdr->version != BLOBLIST_VERSION)
		بازگشت log_پیام_ret("Bad version", -EPROTONOSUPPORT);
	اگر (!hdr->total_size || (size && hdr->total_size > size))
		بازگشت log_پیام_ret("Bad total size", -EFBIG);
	اگر (hdr->used_size > hdr->total_size)
		بازگشت log_پیام_ret("Bad used size", -ENOENT);
	اگر (hdr->hdr_size != اندازه(ساختار bloblist_hdr))
		بازگشت log_پیام_ret("Bad header size", -ENOENT);

	chksum = bloblist_calc_chksum(hdr);
	اگر (hdr->chksum != chksum) {
		log_err("Checksum %x != %x\n", hdr->chksum, chksum);
		بازگشت log_پیام_ret("Bad checksum", -EIO);
	}
	gd->bloblist = hdr;
	gd->flags |= GD_FLG_BLOBLIST_HANDOFF;

	بازگشت 0;
}

صحیح bloblist_finish(تهی)
{
	ساختار bloblist_hdr *hdr = gd->bloblist;

	hdr->chksum = bloblist_calc_chksum(hdr);
	log_debug("Finished bloblist size %lx at %lx\n", (ulong)hdr->used_size,
		  (ulong)map_to_sysmem(hdr));

	بازگشت 0;
}

ulong bloblist_get_base(تهی)
{
	بازگشت map_to_sysmem(gd->bloblist);
}

ulong bloblist_get_size(تهی)
{
	ساختار bloblist_hdr *hdr = gd->bloblist;

	بازگشت hdr->used_size;
}

ulong bloblist_get_total_size(تهی)
{
	ساختار bloblist_hdr *hdr = gd->bloblist;

	بازگشت hdr->total_size;
}

تهی bloblist_get_stats(ulong *basep, ulong *tsizep, ulong *usizep)
{
	ساختار bloblist_hdr *hdr = gd->bloblist;

	*basep = map_to_sysmem(gd->bloblist);
	*tsizep = hdr->total_size;
	*usizep = hdr->used_size;
}

ایستا تهی show_مقدار(ثابت نویسه *prompt, ulong مقدار)
{
	printf("%s:%*s %-5lx  ", prompt, 10 - (صحیح)strlen(prompt), "", مقدار);
	print_size(مقدار, "\n");
}

تهی bloblist_show_stats(تهی)
{
	ulong base, tsize, usize;

	bloblist_get_stats(&base, &tsize, &usize);
	printf("base:       %lx\n", base);
	show_مقدار("total size", tsize);
	show_مقدار("used size", usize);
	show_مقدار("free", tsize - usize);
}

تهی bloblist_show_list(تهی)
{
	ساختار bloblist_hdr *hdr = gd->bloblist;
	ساختار bloblist_rec *rec;

	printf("%-8s  %8s   Tag Name\n", "Address", "Size");
	برای (rec = bloblist_first_blob(hdr); rec;
	     rec = bloblist_next_blob(hdr, rec)) {
		printf("%08lx  %8x  %4x %s\n",
		       (ulong)map_to_sysmem((تهی *)rec + rec_hdr_size(rec)),
		       rec->size, rec_tag(rec),
		       bloblist_tag_name(rec_tag(rec)));
	}
}

صحیح bloblist_reloc(تهی *to, uint to_size)
{
	ساختار bloblist_hdr *hdr;

	اگر (!to_size)
		بازگشت 0;

	اگر (to_size < gd->bloblist->total_size)
		بازگشت -ENOSPC;

	memcpy(to, gd->bloblist, gd->bloblist->total_size);
	hdr = to;
	hdr->total_size = to_size;
	gd->bloblist = to;

	بازگشت 0;
}

/*
 * Weak default function for getting bloblist from boot args.
 */
صحیح __weak xferlist_from_boot_arg(ulong __always_unused *addr)
{
	بازگشت -ENOENT;
}

bool bloblist_exists(تهی)
{
	صحیح ret;
	ulong addr = 0;

	/* Check if a valid transfer list passed in */
	اگر (!xferlist_from_boot_arg(&addr))
		برو found;

	/*
	 * If Firmware Handoff is mandatory but no transfer list is
	 * observed, report it as an error.
	 */
	اگر (IS_ENABLED(CONFIG_BLOBLIST_PASSAGE_MANDATORY))
		بازگشت false;

	/*
	 * We have checked for a valid transfer list being passed. At this
	 * point, if we do not have a fixed address for the bloblist, we cannot
	 * be provided with one.
	 */
	اگر (xpl_is_first_phase() || !IS_ENABLED(CONFIG_BLOBLIST_FIXED))
		بازگشت false;

	/*
	 * Check for a valid list as the configured address.
	 */
	addr = IF_ENABLED_INT(CONFIG_BLOBLIST_FIXED,
			      CONFIG_BLOBLIST_ADDR);
	ret = bloblist_check(addr, CONFIG_BLOBLIST_SIZE);
	اگر (!ret)
		برو found;

	log_debug("Bloblist at %lx not found (err=%d)\n", addr, ret);
	بازگشت false;

found:
#ifdef DEBUG
	bloblist_show_stats();
	bloblist_show_list();
#endif
	بازگشت true;
}

صحیح bloblist_init(تهی)
{
	صحیح ret;
	ulong addr = 0, size = CONFIG_BLOBLIST_SIZE;

	اگر (gd->flags & GD_FLG_BLOBLIST_HANDOFF) {
		log_debug("Found existing bloblist size %x at %p\n",
			  gd->bloblist->total_size, gd->bloblist);
		بازگشت 0;
	}

	/*
	 * If Firmware Handoff is mandatory but no transfer list has been
	 * observed by fdtdec_setup, report it as an error.
	 */
	اگر (IS_ENABLED(CONFIG_BLOBLIST_PASSAGE_MANDATORY))
		بازگشت -ENOENT;

	/*
	 * If we don't have an existing bloblist, we either need
	 * to allocate one now, or initialize the fixed address
	 * space as a bloblist.
	 */
	اگر (CONFIG_IS_ENABLED(BLOBLIST_ALLOC)) {
		تهی *ptr = memalign(BLOBLIST_ALIGN, size);

		اگر (!ptr)
			بازگشت log_پیام_ret("alloc", -ENOMEM);
		addr = map_to_sysmem(ptr);
	} وگرنه
		addr = IF_ENABLED_INT(CONFIG_BLOBLIST_FIXED,
				      CONFIG_BLOBLIST_ADDR);

	log_debug("Creating new bloblist size %lx at %lx\n", size,
		  addr);
	ret = bloblist_new(addr, size, 0, 0);
	اگر (ret)
		بازگشت log_پیام_ret("ini", ret);

	بازگشت 0;
}

صحیح bloblist_check_reg_conv(ulong rfdt, ulong rzero, ulong rsig, ulong xlist)
{
	u64 version = BLOBLIST_REGCONV_VER;
	ulong sigval;
	صحیح ret;

	اگر ((IS_ENABLED(CONFIG_64BIT) && !IS_ENABLED(CONFIG_SPL_BUILD)) ||
			(IS_ENABLED(CONFIG_SPL_64BIT) && IS_ENABLED(CONFIG_SPL_BUILD))) {
		sigval = ((BLOBLIST_MAGIC & ((1ULL << BLOBLIST_REGCONV_SHIFT_64) - 1)) |
			 ((version  & BLOBLIST_REGCONV_MASK) << BLOBLIST_REGCONV_SHIFT_64));
	} وگرنه {
		sigval = ((BLOBLIST_MAGIC & ((1UL << BLOBLIST_REGCONV_SHIFT_32) - 1)) |
			 ((version  & BLOBLIST_REGCONV_MASK) << BLOBLIST_REGCONV_SHIFT_32));
	}

	اگر (rzero || rsig != sigval)
		بازگشت -EIO;

	ret = bloblist_check(xlist, 0);
	اگر (ret)
		بازگشت ret;

	اگر (rfdt != (ulong)bloblist_find(BLOBLISTT_CONTROL_FDT, 0)) {
		/* Remove this bloblist from gd */
		gd->bloblist = پوچ;
		gd->flags &= ~GD_FLG_BLOBLIST_HANDOFF;
		بازگشت -EIO;
	}

	بازگشت 0;
}
