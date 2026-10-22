// SPDX-License-Identifier: GPL-2.0+ BSD-3-Clause
/*
 * Copyright 2018 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY	LOGC_BLOBLIST

#અન્તર્ભાવ <bloblist.h>
#અન્તર્ભાવ <display_options.h>
#અન્તર્ભાવ <log.h>
#અન્તર્ભાવ <malloc.h>
#અન્તર્ભાવ <mapmem.h>
#અન્તર્ભાવ <spl.h>
#અન્તર્ભાવ <tables_csum.h>
#અન્તર્ભાવ <asm/global_data.h>
#અન્તર્ભાવ <u-boot/crc.h>

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

સ્થિર સંરચના tag_name {
	ગણના bloblist_tag_t tag;
	અચલ અક્ષર *name;
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

અચલ અક્ષર *bloblist_tag_name(ગણના bloblist_tag_t tag)
{
	પૂર્ણાંક i;

	માટે (i = 0; i < ARRAY_SIZE(tag_name); i++) {
		જો (tag_name[i].tag == tag)
			પાછા tag_name[i].name;
	}

	પાછા "invalid";
}

સ્થિર સંરચના bloblist_rec *bloblist_first_blob(સંરચના bloblist_hdr *hdr)
{
	જો (hdr->used_size <= hdr->hdr_size)
		પાછા ખાલી;
	પાછા (સંરચના bloblist_rec *)((શૂન્ય *)hdr + hdr->hdr_size);
}

સ્થિર અંતઃરેખ uint rec_hdr_size(સંરચના bloblist_rec *rec)
{
	પાછા (rec->tag_and_hdr_size & BLOBLISTR_HDR_SIZE_MASK) >>
		BLOBLISTR_HDR_SIZE_SHIFT;
}

સ્થિર અંતઃરેખ uint rec_tag(સંરચના bloblist_rec *rec)
{
	પાછા (rec->tag_and_hdr_size & BLOBLISTR_TAG_MASK) >>
		BLOBLISTR_TAG_SHIFT;
}

સ્થિર અંતઃરેખ શૂન્ય void_blob(સંરચના bloblist_rec *rec)
{
	જો (rec_tag(rec) == BLOBLISTT_VOID)
		પાછા;
	rec->tag_and_hdr_size = BLOBLISTT_VOID |
				કદ(*rec) << BLOBLISTR_HDR_SIZE_SHIFT;
}

સ્થિર અંતઃરેખ સંરચના bloblist_rec *rec_from_blob(શૂન્ય *blob)
{
	પાછા (blob - કદ(સંરચના bloblist_rec));
}

સ્થિર ulong bloblist_blob_end_ofs(સંરચના bloblist_hdr *hdr,
				   સંરચના bloblist_rec *rec)
{
	ulong offset;

	offset = (શૂન્ય *)rec - (શૂન્ય *)hdr;
	/*
	 * The data section of next TE should start from an address aligned
	 * to 1 << hdr->align_log2.
	 */
	offset += rec_hdr_size(rec) + rec->size;
	offset = round_up(offset + rec_hdr_size(rec), 1 << hdr->align_log2);
	offset -= rec_hdr_size(rec);

	પાછા offset;
}

સ્થિર સંરચના bloblist_rec *bloblist_next_blob(સંરચના bloblist_hdr *hdr,
					       સંરચના bloblist_rec *rec)
{
	ulong offset = bloblist_blob_end_ofs(hdr, rec);

	જો (offset >= hdr->used_size)
		પાછા ખાલી;
	પાછા (સંરચના bloblist_rec *)((શૂન્ય *)hdr + offset);
}

#define foreach_rec(_rec, _hdr) \
	માટે (_rec = bloblist_first_blob(_hdr); \
	     _rec; \
	     _rec = bloblist_next_blob(_hdr, _rec))

સ્થિર સંરચના bloblist_rec *bloblist_findrec(uint tag)
{
	સંરચના bloblist_hdr *hdr = gd->bloblist;
	સંરચના bloblist_rec *rec;

	જો (!hdr)
		પાછા ખાલી;

	foreach_rec(rec, hdr) {
		જો (rec_tag(rec) == tag)
			પાછા rec;
	}

	પાછા ખાલી;
}

સ્થિર પૂર્ણાંક bloblist_addrec(uint tag, પૂર્ણાંક size, પૂર્ણાંક align_log2,
			   સંરચના bloblist_rec **recp)
{
	સંરચના bloblist_hdr *hdr = gd->bloblist;
	સંરચના bloblist_rec *rec;
	phys_addr_t data_start, aligned_start;
	phys_size_t new_alloced;

	જો (!align_log2)
		align_log2 = BLOBLIST_BLOB_ALIGN_LOG2;

	/* Figure out where the new data will start */
	data_start = map_to_sysmem(hdr) + hdr->used_size + કદ(*rec);

	/* Align the address and then calculate the offset from used size */
	aligned_start = ALIGN(data_start, 1U << align_log2) - data_start;

	/* If we need to create a dummy record, create it */
	જો (aligned_start) {
		પૂર્ણાંક void_size = aligned_start - કદ(*rec);
		સંરચના bloblist_rec *vrec;
		પૂર્ણાંક ret;

		ret = bloblist_addrec(BLOBLISTT_VOID, void_size, 0, &vrec);
		જો (ret)
			પાછા log_સંદેશ_ret("void", ret);

		/* start the record after that */
		data_start = map_to_sysmem(hdr) + hdr->used_size + કદ(*vrec);
	}

	/* Calculate the new allocated total */
	new_alloced = data_start - map_to_sysmem(hdr) +
		ALIGN(size, 1U << align_log2);

	જો (new_alloced > hdr->total_size) {
		log_err("Failed to allocate %x bytes\n", size);
		log_err("Used size=%x, total size=%x\n",
			hdr->used_size, hdr->total_size);
		પાછા log_સંદેશ_ret("bloblist add", -ENOSPC);
	}
	rec = (શૂન્ય *)hdr + hdr->used_size;

	rec->tag_and_hdr_size = tag | કદ(*rec) << BLOBLISTR_HDR_SIZE_SHIFT;
	rec->size = size;

	/* Zero the record data */
	memset((શૂન્ય *)rec + rec_hdr_size(rec), '\0', rec->size);

	hdr->used_size = new_alloced;
	*recp = rec;

	પાછા 0;
}

સ્થિર પૂર્ણાંક bloblist_ensurerec(uint tag, સંરચના bloblist_rec **recp, પૂર્ણાંક size,
			      પૂર્ણાંક align_log2)
{
	સંરચના bloblist_rec *rec;

	rec = bloblist_findrec(tag);
	જો (rec) {
		જો (size && size != rec->size) {
			*recp = rec;
			પાછા -ESPIPE;
		}
	} નહીંતર {
		પૂર્ણાંક ret;

		ret = bloblist_addrec(tag, size, align_log2, &rec);
		જો (ret)
			પાછા ret;
	}
	*recp = rec;

	પાછા 0;
}

સ્થિર પૂર્ણાંક bloblist_get_blob_data_offset(uint tag)
{
	પસંદગી (tag) {
	સ્થિતિ BLOBLISTT_FDT_OVERLAY:
		પાછા કદ(સંરચના dto_blob_hdr);
	/*
	 * return the data offset if it is not following the blob
	 * header immediately.
	 */
	}
	પાછા 0;
}

શૂન્ય *bloblist_find(uint tag, પૂર્ણાંક size)
{
	શૂન્ય *blob = ખાલી;
	પૂર્ણાંક blob_size;

	blob = bloblist_get_blob(tag, &blob_size);

	જો (size && size != blob_size)
		પાછા ખાલી;

	પાછા blob;
}

શૂન્ય *bloblist_get_blob(uint tag, પૂર્ણાંક *sizep)
{
	સંરચના bloblist_rec *rec;

	rec = bloblist_findrec(tag);
	જો (!rec)
		પાછા ખાલી;

	*sizep = rec->size;

	પાછા (શૂન્ય *)rec + rec_hdr_size(rec);
}

પૂર્ણાંક bloblist_apply_blobs(uint tag, પૂર્ણાંક (*func)(શૂન્ય **data, પૂર્ણાંક size))
{
	સંરચના bloblist_hdr *hdr = gd->bloblist;
	સંરચના bloblist_rec *rec;

	જો (!func || !hdr)
		પાછા -ENOENT;

	foreach_rec(rec, hdr) {
		/* Apply all blobs with the specified tag */
		જો (rec_tag(rec) == tag) {
			પૂર્ણાંક ret;
			પૂર્ણાંક tag = rec_tag(rec);
			શૂન્ય *blob = (શૂન્ય *)rec + rec_hdr_size(rec);
			પૂર્ણાંક dat_off = bloblist_get_blob_data_offset(tag);

			blob += dat_off;
			ret = func(&blob, rec->size - dat_off);
			જો (ret) {
				log_err("Failed to apply blob with tag %d\n",
					tag);
				પાછા ret;
			}

			rec = rec_from_blob(blob - dat_off);
			જો (!rec) {
				log_err("Blob corrupted\n");
				પાછા -ENOENT;
			}

			/* Mark applied blob record as void */
			void_blob(rec);
		}
	}

	પાછા 0;
}

શૂન્ય *bloblist_add(uint tag, પૂર્ણાંક size, પૂર્ણાંક align_log2)
{
	સંરચના bloblist_rec *rec;

	જો (bloblist_addrec(tag, size, align_log2, &rec))
		પાછા ખાલી;

	પાછા (શૂન્ય *)rec + rec_hdr_size(rec);
}

પૂર્ણાંક bloblist_ensure_size(uint tag, પૂર્ણાંક size, પૂર્ણાંક align_log2, શૂન્ય **blobp)
{
	સંરચના bloblist_rec *rec;
	પૂર્ણાંક ret;

	ret = bloblist_ensurerec(tag, &rec, size, align_log2);
	જો (ret)
		પાછા ret;
	*blobp = (શૂન્ય *)rec + rec_hdr_size(rec);

	પાછા 0;
}

શૂન્ય *bloblist_ensure(uint tag, પૂર્ણાંક size)
{
	સંરચના bloblist_rec *rec;

	જો (bloblist_ensurerec(tag, &rec, size, 0))
		પાછા ખાલી;

	પાછા (શૂન્ય *)rec + rec_hdr_size(rec);
}

પૂર્ણાંક bloblist_ensure_size_ret(uint tag, પૂર્ણાંક *sizep, શૂન્ય **blobp)
{
	સંરચના bloblist_rec *rec;
	પૂર્ણાંક ret;

	ret = bloblist_ensurerec(tag, &rec, *sizep, 0);
	જો (ret == -ESPIPE)
		*sizep = rec->size;
	નહીંતર જો (ret)
		પાછા ret;
	*blobp = (શૂન્ય *)rec + rec_hdr_size(rec);

	પાછા 0;
}

સ્થિર પૂર્ણાંક bloblist_resize_rec(સંરચના bloblist_hdr *hdr,
			       સંરચના bloblist_rec *rec,
			       પૂર્ણાંક new_size)
{
	પૂર્ણાંક expand_by;	/* Number of bytes to expand by (-ve to contract) */
	પૂર્ણાંક new_alloced;
	ulong next_ofs;	/* Offset of the record after @rec */

	expand_by = ALIGN(new_size - rec->size, BLOBLIST_BLOB_ALIGN);
	new_alloced = ALIGN(hdr->used_size + expand_by, BLOBLIST_BLOB_ALIGN);
	જો (new_size < 0) {
		log_debug("Attempt to shrink blob size below 0 (%x)\n",
			  new_size);
		પાછા log_સંદેશ_ret("size", -EINVAL);
	}
	જો (new_alloced > hdr->total_size) {
		log_err("Failed to allocate %x bytes\n", new_size);
		log_err("Used size=%x, total size=%x\n",
			hdr->used_size, hdr->total_size);
		પાછા log_સંદેશ_ret("alloc", -ENOSPC);
	}

	/* Move the following blobs up or down, if this is not the last */
	next_ofs = bloblist_blob_end_ofs(hdr, rec);
	જો (next_ofs != hdr->used_size) {
		memmove((શૂન્ય *)hdr + next_ofs + expand_by,
			(શૂન્ય *)hdr + next_ofs, hdr->used_size - next_ofs);
	}
	hdr->used_size = new_alloced;

	/* Zero the new part of the blob */
	જો (expand_by > 0) {
		memset((શૂન્ય *)rec + rec_hdr_size(rec) + rec->size, '\0',
		       new_size - rec->size);
	}

	/* Update the size of this blob */
	rec->size = new_size;

	પાછા 0;
}

પૂર્ણાંક bloblist_resize(uint tag, પૂર્ણાંક new_size)
{
	સંરચના bloblist_hdr *hdr = gd->bloblist;
	સંરચના bloblist_rec *rec;
	પૂર્ણાંક ret;

	rec = bloblist_findrec(tag);
	જો (!rec)
		પાછા log_સંદેશ_ret("find", -ENOENT);
	ret = bloblist_resize_rec(hdr, rec, new_size);
	જો (ret)
		પાછા log_સંદેશ_ret("resize", ret);

	પાછા 0;
}

સ્થિર u32 bloblist_calc_chksum(સંરચના bloblist_hdr *hdr)
{
	u8 chksum;

	chksum = table_compute_checksum(hdr, hdr->used_size);
	chksum += hdr->chksum;

	પાછા chksum;
}

પૂર્ણાંક bloblist_new(ulong addr, uint size, uint flags, uint align_log2)
{
	સંરચના bloblist_hdr *hdr;

	જો (size < કદ(*hdr))
		પાછા log_ret(-ENOSPC);
	જો (addr & (BLOBLIST_ALIGN - 1))
		પાછા log_ret(-EFAULT);
	hdr = map_sysmem(addr, size);
	memset(hdr, '\0', કદ(*hdr));
	hdr->version = BLOBLIST_VERSION;
	hdr->hdr_size = કદ(*hdr);
	hdr->flags = flags;
	hdr->magic = BLOBLIST_MAGIC;
	hdr->used_size = hdr->hdr_size;
	hdr->total_size = size;
	hdr->align_log2 = align_log2 ? align_log2 : BLOBLIST_BLOB_ALIGN_LOG2;
	hdr->chksum = 0;
	gd->bloblist = hdr;
	gd->flags |= GD_FLG_BLOBLIST_HANDOFF;

	પાછા 0;
}

પૂર્ણાંક bloblist_check(ulong addr, uint size)
{
	સંરચના bloblist_hdr *hdr;
	u32 chksum;

	hdr = map_sysmem(addr, કદ(*hdr));
	જો (hdr->magic != BLOBLIST_MAGIC)
		પાછા log_સંદેશ_ret("Bad magic", -ENOENT);
	જો (hdr->version != BLOBLIST_VERSION)
		પાછા log_સંદેશ_ret("Bad version", -EPROTONOSUPPORT);
	જો (!hdr->total_size || (size && hdr->total_size > size))
		પાછા log_સંદેશ_ret("Bad total size", -EFBIG);
	જો (hdr->used_size > hdr->total_size)
		પાછા log_સંદેશ_ret("Bad used size", -ENOENT);
	જો (hdr->hdr_size != કદ(સંરચના bloblist_hdr))
		પાછા log_સંદેશ_ret("Bad header size", -ENOENT);

	chksum = bloblist_calc_chksum(hdr);
	જો (hdr->chksum != chksum) {
		log_err("Checksum %x != %x\n", hdr->chksum, chksum);
		પાછા log_સંદેશ_ret("Bad checksum", -EIO);
	}
	gd->bloblist = hdr;
	gd->flags |= GD_FLG_BLOBLIST_HANDOFF;

	પાછા 0;
}

પૂર્ણાંક bloblist_finish(શૂન્ય)
{
	સંરચના bloblist_hdr *hdr = gd->bloblist;

	hdr->chksum = bloblist_calc_chksum(hdr);
	log_debug("Finished bloblist size %lx at %lx\n", (ulong)hdr->used_size,
		  (ulong)map_to_sysmem(hdr));

	પાછા 0;
}

ulong bloblist_get_base(શૂન્ય)
{
	પાછા map_to_sysmem(gd->bloblist);
}

ulong bloblist_get_size(શૂન્ય)
{
	સંરચના bloblist_hdr *hdr = gd->bloblist;

	પાછા hdr->used_size;
}

ulong bloblist_get_total_size(શૂન્ય)
{
	સંરચના bloblist_hdr *hdr = gd->bloblist;

	પાછા hdr->total_size;
}

શૂન્ય bloblist_get_stats(ulong *basep, ulong *tsizep, ulong *usizep)
{
	સંરચના bloblist_hdr *hdr = gd->bloblist;

	*basep = map_to_sysmem(gd->bloblist);
	*tsizep = hdr->total_size;
	*usizep = hdr->used_size;
}

સ્થિર શૂન્ય show_કિંમત(અચલ અક્ષર *prompt, ulong કિંમત)
{
	printf("%s:%*s %-5lx  ", prompt, 10 - (પૂર્ણાંક)strlen(prompt), "", કિંમત);
	print_size(કિંમત, "\n");
}

શૂન્ય bloblist_show_stats(શૂન્ય)
{
	ulong base, tsize, usize;

	bloblist_get_stats(&base, &tsize, &usize);
	printf("base:       %lx\n", base);
	show_કિંમત("total size", tsize);
	show_કિંમત("used size", usize);
	show_કિંમત("free", tsize - usize);
}

શૂન્ય bloblist_show_list(શૂન્ય)
{
	સંરચના bloblist_hdr *hdr = gd->bloblist;
	સંરચના bloblist_rec *rec;

	printf("%-8s  %8s   Tag Name\n", "Address", "Size");
	માટે (rec = bloblist_first_blob(hdr); rec;
	     rec = bloblist_next_blob(hdr, rec)) {
		printf("%08lx  %8x  %4x %s\n",
		       (ulong)map_to_sysmem((શૂન્ય *)rec + rec_hdr_size(rec)),
		       rec->size, rec_tag(rec),
		       bloblist_tag_name(rec_tag(rec)));
	}
}

પૂર્ણાંક bloblist_reloc(શૂન્ય *to, uint to_size)
{
	સંરચના bloblist_hdr *hdr;

	જો (!to_size)
		પાછા 0;

	જો (to_size < gd->bloblist->total_size)
		પાછા -ENOSPC;

	memcpy(to, gd->bloblist, gd->bloblist->total_size);
	hdr = to;
	hdr->total_size = to_size;
	gd->bloblist = to;

	પાછા 0;
}

/*
 * Weak default function for getting bloblist from boot args.
 */
પૂર્ણાંક __weak xferlist_from_boot_arg(ulong __always_unused *addr)
{
	પાછા -ENOENT;
}

bool bloblist_exists(શૂન્ય)
{
	પૂર્ણાંક ret;
	ulong addr = 0;

	/* Check if a valid transfer list passed in */
	જો (!xferlist_from_boot_arg(&addr))
		જાઓ found;

	/*
	 * If Firmware Handoff is mandatory but no transfer list is
	 * observed, report it as an error.
	 */
	જો (IS_ENABLED(CONFIG_BLOBLIST_PASSAGE_MANDATORY))
		પાછા false;

	/*
	 * We have checked for a valid transfer list being passed. At this
	 * point, if we do not have a fixed address for the bloblist, we cannot
	 * be provided with one.
	 */
	જો (xpl_is_first_phase() || !IS_ENABLED(CONFIG_BLOBLIST_FIXED))
		પાછા false;

	/*
	 * Check for a valid list as the configured address.
	 */
	addr = IF_ENABLED_INT(CONFIG_BLOBLIST_FIXED,
			      CONFIG_BLOBLIST_ADDR);
	ret = bloblist_check(addr, CONFIG_BLOBLIST_SIZE);
	જો (!ret)
		જાઓ found;

	log_debug("Bloblist at %lx not found (err=%d)\n", addr, ret);
	પાછા false;

found:
#ifdef DEBUG
	bloblist_show_stats();
	bloblist_show_list();
#endif
	પાછા true;
}

પૂર્ણાંક bloblist_init(શૂન્ય)
{
	પૂર્ણાંક ret;
	ulong addr = 0, size = CONFIG_BLOBLIST_SIZE;

	જો (gd->flags & GD_FLG_BLOBLIST_HANDOFF) {
		log_debug("Found existing bloblist size %x at %p\n",
			  gd->bloblist->total_size, gd->bloblist);
		પાછા 0;
	}

	/*
	 * If Firmware Handoff is mandatory but no transfer list has been
	 * observed by fdtdec_setup, report it as an error.
	 */
	જો (IS_ENABLED(CONFIG_BLOBLIST_PASSAGE_MANDATORY))
		પાછા -ENOENT;

	/*
	 * If we don't have an existing bloblist, we either need
	 * to allocate one now, or initialize the fixed address
	 * space as a bloblist.
	 */
	જો (CONFIG_IS_ENABLED(BLOBLIST_ALLOC)) {
		શૂન્ય *ptr = memalign(BLOBLIST_ALIGN, size);

		જો (!ptr)
			પાછા log_સંદેશ_ret("alloc", -ENOMEM);
		addr = map_to_sysmem(ptr);
	} નહીંતર
		addr = IF_ENABLED_INT(CONFIG_BLOBLIST_FIXED,
				      CONFIG_BLOBLIST_ADDR);

	log_debug("Creating new bloblist size %lx at %lx\n", size,
		  addr);
	ret = bloblist_new(addr, size, 0, 0);
	જો (ret)
		પાછા log_સંદેશ_ret("ini", ret);

	પાછા 0;
}

પૂર્ણાંક bloblist_check_reg_conv(ulong rfdt, ulong rzero, ulong rsig, ulong xlist)
{
	u64 version = BLOBLIST_REGCONV_VER;
	ulong sigval;
	પૂર્ણાંક ret;

	જો ((IS_ENABLED(CONFIG_64BIT) && !IS_ENABLED(CONFIG_SPL_BUILD)) ||
			(IS_ENABLED(CONFIG_SPL_64BIT) && IS_ENABLED(CONFIG_SPL_BUILD))) {
		sigval = ((BLOBLIST_MAGIC & ((1ULL << BLOBLIST_REGCONV_SHIFT_64) - 1)) |
			 ((version  & BLOBLIST_REGCONV_MASK) << BLOBLIST_REGCONV_SHIFT_64));
	} નહીંતર {
		sigval = ((BLOBLIST_MAGIC & ((1UL << BLOBLIST_REGCONV_SHIFT_32) - 1)) |
			 ((version  & BLOBLIST_REGCONV_MASK) << BLOBLIST_REGCONV_SHIFT_32));
	}

	જો (rzero || rsig != sigval)
		પાછા -EIO;

	ret = bloblist_check(xlist, 0);
	જો (ret)
		પાછા ret;

	જો (rfdt != (ulong)bloblist_find(BLOBLISTT_CONTROL_FDT, 0)) {
		/* Remove this bloblist from gd */
		gd->bloblist = ખાલી;
		gd->flags &= ~GD_FLG_BLOBLIST_HANDOFF;
		પાછા -EIO;
	}

	પાછા 0;
}
