// SPDX-License-Identifier: GPL-2.0+ BSD-3-Clause
/*
 * Copyright 2018 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY	LOGC_BLOBLIST

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

DECLARE_GLOBAL_DATA_PTR;

ساكن بنية tag_name {
	تعداد bloblist_tag_t tag;
	ثابت حرف *name;
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

ثابت حرف *bloblist_tag_name(تعداد bloblist_tag_t tag)
{
	صحيح i;

	עבור (i = 0; i < ARRAY_SIZE(tag_name); i++) {
		אם (tag_name[i].tag == tag)
			החזר tag_name[i].name;
	}

	החזר "invalid";
}

ساكن بنية bloblist_rec *bloblist_first_blob(بنية bloblist_hdr *hdr)
{
	אם (hdr->used_size <= hdr->hdr_size)
		החזר عدم;
	החזר (بنية bloblist_rec *)((فراغ *)hdr + hdr->hdr_size);
}

ساكن مضمن uint rec_hdr_size(بنية bloblist_rec *rec)
{
	החזר (rec->tag_and_hdr_size & BLOBLISTR_HDR_SIZE_MASK) >>
		BLOBLISTR_HDR_SIZE_SHIFT;
}

ساكن مضمن uint rec_tag(بنية bloblist_rec *rec)
{
	החזר (rec->tag_and_hdr_size & BLOBLISTR_TAG_MASK) >>
		BLOBLISTR_TAG_SHIFT;
}

ساكن مضمن فراغ void_blob(بنية bloblist_rec *rec)
{
	אם (rec_tag(rec) == BLOBLISTT_VOID)
		החזר;
	rec->tag_and_hdr_size = BLOBLISTT_VOID |
				حجم(*rec) << BLOBLISTR_HDR_SIZE_SHIFT;
}

ساكن مضمن بنية bloblist_rec *rec_from_blob(فراغ *blob)
{
	החזר (blob - حجم(بنية bloblist_rec));
}

ساكن ulong bloblist_blob_end_ofs(بنية bloblist_hdr *hdr,
				   بنية bloblist_rec *rec)
{
	ulong offset;

	offset = (فراغ *)rec - (فراغ *)hdr;
	/*
	 * The data section of next TE should start from an address aligned
	 * to 1 << hdr->align_log2.
	 */
	offset += rec_hdr_size(rec) + rec->size;
	offset = round_up(offset + rec_hdr_size(rec), 1 << hdr->align_log2);
	offset -= rec_hdr_size(rec);

	החזר offset;
}

ساكن بنية bloblist_rec *bloblist_next_blob(بنية bloblist_hdr *hdr,
					       بنية bloblist_rec *rec)
{
	ulong offset = bloblist_blob_end_ofs(hdr, rec);

	אם (offset >= hdr->used_size)
		החזר عدم;
	החזר (بنية bloblist_rec *)((فراغ *)hdr + offset);
}

#define foreach_rec(_rec, _hdr) \
	עבור (_rec = bloblist_first_blob(_hdr); \
	     _rec; \
	     _rec = bloblist_next_blob(_hdr, _rec))

ساكن بنية bloblist_rec *bloblist_findrec(uint tag)
{
	بنية bloblist_hdr *hdr = gd->bloblist;
	بنية bloblist_rec *rec;

	אם (!hdr)
		החזר عدم;

	foreach_rec(rec, hdr) {
		אם (rec_tag(rec) == tag)
			החזר rec;
	}

	החזר عدم;
}

ساكن صحيح bloblist_addrec(uint tag, صحيح size, صحيح align_log2,
			   بنية bloblist_rec **recp)
{
	بنية bloblist_hdr *hdr = gd->bloblist;
	بنية bloblist_rec *rec;
	phys_addr_t data_start, aligned_start;
	phys_size_t new_alloced;

	אם (!align_log2)
		align_log2 = BLOBLIST_BLOB_ALIGN_LOG2;

	/* Figure out where the new data will start */
	data_start = map_to_sysmem(hdr) + hdr->used_size + حجم(*rec);

	/* Align the address and then calculate the offset from used size */
	aligned_start = ALIGN(data_start, 1U << align_log2) - data_start;

	/* If we need to create a dummy record, create it */
	אם (aligned_start) {
		صحيح void_size = aligned_start - حجم(*rec);
		بنية bloblist_rec *vrec;
		صحيح ret;

		ret = bloblist_addrec(BLOBLISTT_VOID, void_size, 0, &vrec);
		אם (ret)
			החזר log_msg_ret("void", ret);

		/* start the record after that */
		data_start = map_to_sysmem(hdr) + hdr->used_size + حجم(*vrec);
	}

	/* Calculate the new allocated total */
	new_alloced = data_start - map_to_sysmem(hdr) +
		ALIGN(size, 1U << align_log2);

	אם (new_alloced > hdr->total_size) {
		log_err("Failed to allocate %x bytes\n", size);
		log_err("Used size=%x, total size=%x\n",
			hdr->used_size, hdr->total_size);
		החזר log_msg_ret("bloblist add", -ENOSPC);
	}
	rec = (فراغ *)hdr + hdr->used_size;

	rec->tag_and_hdr_size = tag | حجم(*rec) << BLOBLISTR_HDR_SIZE_SHIFT;
	rec->size = size;

	/* Zero the record data */
	memset((فراغ *)rec + rec_hdr_size(rec), '\0', rec->size);

	hdr->used_size = new_alloced;
	*recp = rec;

	החזר 0;
}

ساكن صحيح bloblist_ensurerec(uint tag, بنية bloblist_rec **recp, صحيح size,
			      صحيح align_log2)
{
	بنية bloblist_rec *rec;

	rec = bloblist_findrec(tag);
	אם (rec) {
		אם (size && size != rec->size) {
			*recp = rec;
			החזר -ESPIPE;
		}
	} אחרת {
		صحيح ret;

		ret = bloblist_addrec(tag, size, align_log2, &rec);
		אם (ret)
			החזר ret;
	}
	*recp = rec;

	החזר 0;
}

ساكن صحيح bloblist_get_blob_data_offset(uint tag)
{
	بدل (tag) {
	حالة BLOBLISTT_FDT_OVERLAY:
		החזר حجم(بنية dto_blob_hdr);
	/*
	 * return the data offset if it is not following the blob
	 * header immediately.
	 */
	}
	החזר 0;
}

فراغ *bloblist_find(uint tag, صحيح size)
{
	فراغ *blob = عدم;
	صحيح blob_size;

	blob = bloblist_get_blob(tag, &blob_size);

	אם (size && size != blob_size)
		החזר عدم;

	החזר blob;
}

فراغ *bloblist_get_blob(uint tag, صحيح *sizep)
{
	بنية bloblist_rec *rec;

	rec = bloblist_findrec(tag);
	אם (!rec)
		החזר عدم;

	*sizep = rec->size;

	החזר (فراغ *)rec + rec_hdr_size(rec);
}

صحيح bloblist_apply_blobs(uint tag, صحيح (*func)(فراغ **data, صحيح size))
{
	بنية bloblist_hdr *hdr = gd->bloblist;
	بنية bloblist_rec *rec;

	אם (!func || !hdr)
		החזר -ENOENT;

	foreach_rec(rec, hdr) {
		/* Apply all blobs with the specified tag */
		אם (rec_tag(rec) == tag) {
			صحيح ret;
			صحيح tag = rec_tag(rec);
			فراغ *blob = (فراغ *)rec + rec_hdr_size(rec);
			صحيح dat_off = bloblist_get_blob_data_offset(tag);

			blob += dat_off;
			ret = func(&blob, rec->size - dat_off);
			אם (ret) {
				log_err("Failed to apply blob with tag %d\n",
					tag);
				החזר ret;
			}

			rec = rec_from_blob(blob - dat_off);
			אם (!rec) {
				log_err("Blob corrupted\n");
				החזר -ENOENT;
			}

			/* Mark applied blob record as void */
			void_blob(rec);
		}
	}

	החזר 0;
}

فراغ *bloblist_add(uint tag, صحيح size, صحيح align_log2)
{
	بنية bloblist_rec *rec;

	אם (bloblist_addrec(tag, size, align_log2, &rec))
		החזר عدم;

	החזר (فراغ *)rec + rec_hdr_size(rec);
}

صحيح bloblist_ensure_size(uint tag, صحيح size, صحيح align_log2, فراغ **blobp)
{
	بنية bloblist_rec *rec;
	صحيح ret;

	ret = bloblist_ensurerec(tag, &rec, size, align_log2);
	אם (ret)
		החזר ret;
	*blobp = (فراغ *)rec + rec_hdr_size(rec);

	החזר 0;
}

فراغ *bloblist_ensure(uint tag, صحيح size)
{
	بنية bloblist_rec *rec;

	אם (bloblist_ensurerec(tag, &rec, size, 0))
		החזר عدم;

	החזר (فراغ *)rec + rec_hdr_size(rec);
}

صحيح bloblist_ensure_size_ret(uint tag, صحيح *sizep, فراغ **blobp)
{
	بنية bloblist_rec *rec;
	صحيح ret;

	ret = bloblist_ensurerec(tag, &rec, *sizep, 0);
	אם (ret == -ESPIPE)
		*sizep = rec->size;
	אחרת אם (ret)
		החזר ret;
	*blobp = (فراغ *)rec + rec_hdr_size(rec);

	החזר 0;
}

ساكن صحيح bloblist_resize_rec(بنية bloblist_hdr *hdr,
			       بنية bloblist_rec *rec,
			       صحيح new_size)
{
	صحيح expand_by;	/* Number of bytes to expand by (-ve to contract) */
	صحيح new_alloced;
	ulong next_ofs;	/* Offset of the record after @rec */

	expand_by = ALIGN(new_size - rec->size, BLOBLIST_BLOB_ALIGN);
	new_alloced = ALIGN(hdr->used_size + expand_by, BLOBLIST_BLOB_ALIGN);
	אם (new_size < 0) {
		log_debug("Attempt to shrink blob size below 0 (%x)\n",
			  new_size);
		החזר log_msg_ret("size", -EINVAL);
	}
	אם (new_alloced > hdr->total_size) {
		log_err("Failed to allocate %x bytes\n", new_size);
		log_err("Used size=%x, total size=%x\n",
			hdr->used_size, hdr->total_size);
		החזר log_msg_ret("alloc", -ENOSPC);
	}

	/* Move the following blobs up or down, if this is not the last */
	next_ofs = bloblist_blob_end_ofs(hdr, rec);
	אם (next_ofs != hdr->used_size) {
		memmove((فراغ *)hdr + next_ofs + expand_by,
			(فراغ *)hdr + next_ofs, hdr->used_size - next_ofs);
	}
	hdr->used_size = new_alloced;

	/* Zero the new part of the blob */
	אם (expand_by > 0) {
		memset((فراغ *)rec + rec_hdr_size(rec) + rec->size, '\0',
		       new_size - rec->size);
	}

	/* Update the size of this blob */
	rec->size = new_size;

	החזר 0;
}

صحيح bloblist_resize(uint tag, صحيح new_size)
{
	بنية bloblist_hdr *hdr = gd->bloblist;
	بنية bloblist_rec *rec;
	صحيح ret;

	rec = bloblist_findrec(tag);
	אם (!rec)
		החזר log_msg_ret("find", -ENOENT);
	ret = bloblist_resize_rec(hdr, rec, new_size);
	אם (ret)
		החזר log_msg_ret("resize", ret);

	החזר 0;
}

ساكن u32 bloblist_calc_chksum(بنية bloblist_hdr *hdr)
{
	u8 chksum;

	chksum = table_compute_checksum(hdr, hdr->used_size);
	chksum += hdr->chksum;

	החזר chksum;
}

صحيح bloblist_new(ulong addr, uint size, uint flags, uint align_log2)
{
	بنية bloblist_hdr *hdr;

	אם (size < حجم(*hdr))
		החזר log_ret(-ENOSPC);
	אם (addr & (BLOBLIST_ALIGN - 1))
		החזר log_ret(-EFAULT);
	hdr = map_sysmem(addr, size);
	memset(hdr, '\0', حجم(*hdr));
	hdr->version = BLOBLIST_VERSION;
	hdr->hdr_size = حجم(*hdr);
	hdr->flags = flags;
	hdr->magic = BLOBLIST_MAGIC;
	hdr->used_size = hdr->hdr_size;
	hdr->total_size = size;
	hdr->align_log2 = align_log2 ? align_log2 : BLOBLIST_BLOB_ALIGN_LOG2;
	hdr->chksum = 0;
	gd->bloblist = hdr;
	gd->flags |= GD_FLG_BLOBLIST_HANDOFF;

	החזר 0;
}

صحيح bloblist_check(ulong addr, uint size)
{
	بنية bloblist_hdr *hdr;
	u32 chksum;

	hdr = map_sysmem(addr, حجم(*hdr));
	אם (hdr->magic != BLOBLIST_MAGIC)
		החזר log_msg_ret("Bad magic", -ENOENT);
	אם (hdr->version != BLOBLIST_VERSION)
		החזר log_msg_ret("Bad version", -EPROTONOSUPPORT);
	אם (!hdr->total_size || (size && hdr->total_size > size))
		החזר log_msg_ret("Bad total size", -EFBIG);
	אם (hdr->used_size > hdr->total_size)
		החזר log_msg_ret("Bad used size", -ENOENT);
	אם (hdr->hdr_size != حجم(بنية bloblist_hdr))
		החזר log_msg_ret("Bad header size", -ENOENT);

	chksum = bloblist_calc_chksum(hdr);
	אם (hdr->chksum != chksum) {
		log_err("Checksum %x != %x\n", hdr->chksum, chksum);
		החזר log_msg_ret("Bad checksum", -EIO);
	}
	gd->bloblist = hdr;
	gd->flags |= GD_FLG_BLOBLIST_HANDOFF;

	החזר 0;
}

صحيح bloblist_finish(فراغ)
{
	بنية bloblist_hdr *hdr = gd->bloblist;

	hdr->chksum = bloblist_calc_chksum(hdr);
	log_debug("Finished bloblist size %lx at %lx\n", (ulong)hdr->used_size,
		  (ulong)map_to_sysmem(hdr));

	החזר 0;
}

ulong bloblist_get_base(فراغ)
{
	החזר map_to_sysmem(gd->bloblist);
}

ulong bloblist_get_size(فراغ)
{
	بنية bloblist_hdr *hdr = gd->bloblist;

	החזר hdr->used_size;
}

ulong bloblist_get_total_size(فراغ)
{
	بنية bloblist_hdr *hdr = gd->bloblist;

	החזר hdr->total_size;
}

فراغ bloblist_get_stats(ulong *basep, ulong *tsizep, ulong *usizep)
{
	بنية bloblist_hdr *hdr = gd->bloblist;

	*basep = map_to_sysmem(gd->bloblist);
	*tsizep = hdr->total_size;
	*usizep = hdr->used_size;
}

ساكن فراغ show_value(ثابت حرف *prompt, ulong value)
{
	printf("%s:%*s %-5lx  ", prompt, 10 - (صحيح)strlen(prompt), "", value);
	print_size(value, "\n");
}

فراغ bloblist_show_stats(فراغ)
{
	ulong base, tsize, usize;

	bloblist_get_stats(&base, &tsize, &usize);
	printf("base:       %lx\n", base);
	show_value("total size", tsize);
	show_value("used size", usize);
	show_value("free", tsize - usize);
}

فراغ bloblist_show_list(فراغ)
{
	بنية bloblist_hdr *hdr = gd->bloblist;
	بنية bloblist_rec *rec;

	printf("%-8s  %8s   Tag Name\n", "Address", "Size");
	עבור (rec = bloblist_first_blob(hdr); rec;
	     rec = bloblist_next_blob(hdr, rec)) {
		printf("%08lx  %8x  %4x %s\n",
		       (ulong)map_to_sysmem((فراغ *)rec + rec_hdr_size(rec)),
		       rec->size, rec_tag(rec),
		       bloblist_tag_name(rec_tag(rec)));
	}
}

صحيح bloblist_reloc(فراغ *to, uint to_size)
{
	بنية bloblist_hdr *hdr;

	אם (!to_size)
		החזר 0;

	אם (to_size < gd->bloblist->total_size)
		החזר -ENOSPC;

	memcpy(to, gd->bloblist, gd->bloblist->total_size);
	hdr = to;
	hdr->total_size = to_size;
	gd->bloblist = to;

	החזר 0;
}

/*
 * Weak default function for getting bloblist from boot args.
 */
صحيح __weak xferlist_from_boot_arg(ulong __always_unused *addr)
{
	החזר -ENOENT;
}

bool bloblist_exists(فراغ)
{
	صحيح ret;
	ulong addr = 0;

	/* Check if a valid transfer list passed in */
	אם (!xferlist_from_boot_arg(&addr))
		اذهب found;

	/*
	 * If Firmware Handoff is mandatory but no transfer list is
	 * observed, report it as an error.
	 */
	אם (IS_ENABLED(CONFIG_BLOBLIST_PASSAGE_MANDATORY))
		החזר false;

	/*
	 * We have checked for a valid transfer list being passed. At this
	 * point, if we do not have a fixed address for the bloblist, we cannot
	 * be provided with one.
	 */
	אם (xpl_is_first_phase() || !IS_ENABLED(CONFIG_BLOBLIST_FIXED))
		החזר false;

	/*
	 * Check for a valid list as the configured address.
	 */
	addr = IF_ENABLED_INT(CONFIG_BLOBLIST_FIXED,
			      CONFIG_BLOBLIST_ADDR);
	ret = bloblist_check(addr, CONFIG_BLOBLIST_SIZE);
	אם (!ret)
		اذهب found;

	log_debug("Bloblist at %lx not found (err=%d)\n", addr, ret);
	החזר false;

found:
#ifdef DEBUG
	bloblist_show_stats();
	bloblist_show_list();
#endif
	החזר true;
}

صحيح bloblist_init(فراغ)
{
	صحيح ret;
	ulong addr = 0, size = CONFIG_BLOBLIST_SIZE;

	אם (gd->flags & GD_FLG_BLOBLIST_HANDOFF) {
		log_debug("Found existing bloblist size %x at %p\n",
			  gd->bloblist->total_size, gd->bloblist);
		החזר 0;
	}

	/*
	 * If Firmware Handoff is mandatory but no transfer list has been
	 * observed by fdtdec_setup, report it as an error.
	 */
	אם (IS_ENABLED(CONFIG_BLOBLIST_PASSAGE_MANDATORY))
		החזר -ENOENT;

	/*
	 * If we don't have an existing bloblist, we either need
	 * to allocate one now, or initialize the fixed address
	 * space as a bloblist.
	 */
	אם (CONFIG_IS_ENABLED(BLOBLIST_ALLOC)) {
		فراغ *ptr = memalign(BLOBLIST_ALIGN, size);

		אם (!ptr)
			החזר log_msg_ret("alloc", -ENOMEM);
		addr = map_to_sysmem(ptr);
	} אחרת
		addr = IF_ENABLED_INT(CONFIG_BLOBLIST_FIXED,
				      CONFIG_BLOBLIST_ADDR);

	log_debug("Creating new bloblist size %lx at %lx\n", size,
		  addr);
	ret = bloblist_new(addr, size, 0, 0);
	אם (ret)
		החזר log_msg_ret("ini", ret);

	החזר 0;
}

صحيح bloblist_check_reg_conv(ulong rfdt, ulong rzero, ulong rsig, ulong xlist)
{
	u64 version = BLOBLIST_REGCONV_VER;
	ulong sigval;
	صحيح ret;

	אם ((IS_ENABLED(CONFIG_64BIT) && !IS_ENABLED(CONFIG_SPL_BUILD)) ||
			(IS_ENABLED(CONFIG_SPL_64BIT) && IS_ENABLED(CONFIG_SPL_BUILD))) {
		sigval = ((BLOBLIST_MAGIC & ((1ULL << BLOBLIST_REGCONV_SHIFT_64) - 1)) |
			 ((version  & BLOBLIST_REGCONV_MASK) << BLOBLIST_REGCONV_SHIFT_64));
	} אחרת {
		sigval = ((BLOBLIST_MAGIC & ((1UL << BLOBLIST_REGCONV_SHIFT_32) - 1)) |
			 ((version  & BLOBLIST_REGCONV_MASK) << BLOBLIST_REGCONV_SHIFT_32));
	}

	אם (rzero || rsig != sigval)
		החזר -EIO;

	ret = bloblist_check(xlist, 0);
	אם (ret)
		החזר ret;

	אם (rfdt != (ulong)bloblist_find(BLOBLISTT_CONTROL_FDT, 0)) {
		/* Remove this bloblist from gd */
		gd->bloblist = عدم;
		gd->flags &= ~GD_FLG_BLOBLIST_HANDOFF;
		החזר -EIO;
	}

	החזר 0;
}
