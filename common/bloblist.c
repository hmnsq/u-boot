// SPDX-License-Identifier: GPL-2.0+ BSD-3-Clause
/*
 * Copyright 2018 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#define LOG_CATEGORY	LOGC_BLOBLIST

#अन्तर्भाव <bloblist.h>
#अन्तर्भाव <display_options.h>
#अन्तर्भाव <log.h>
#अन्तर्भाव <malloc.h>
#अन्तर्भाव <mapmem.h>
#अन्तर्भाव <spl.h>
#अन्तर्भाव <tables_csum.h>
#अन्तर्भाव <asm/global_data.h>
#अन्तर्भाव <u-boot/crc.h>

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

स्थिर संरचना tag_name {
	गणना bloblist_tag_t tag;
	अचर वर्ण *name;
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

अचर वर्ण *bloblist_tag_name(गणना bloblist_tag_t tag)
{
	पूर्णाङ्क i;

	लागि (i = 0; i < ARRAY_SIZE(tag_name); i++) {
		यदि (tag_name[i].tag == tag)
			फर्काऊ tag_name[i].name;
	}

	फर्काऊ "invalid";
}

स्थिर संरचना bloblist_rec *bloblist_first_blob(संरचना bloblist_hdr *hdr)
{
	यदि (hdr->used_size <= hdr->hdr_size)
		फर्काऊ रिक्त;
	फर्काऊ (संरचना bloblist_rec *)((शून्य *)hdr + hdr->hdr_size);
}

स्थिर अन्तःस्थ uint rec_hdr_size(संरचना bloblist_rec *rec)
{
	फर्काऊ (rec->tag_and_hdr_size & BLOBLISTR_HDR_SIZE_MASK) >>
		BLOBLISTR_HDR_SIZE_SHIFT;
}

स्थिर अन्तःस्थ uint rec_tag(संरचना bloblist_rec *rec)
{
	फर्काऊ (rec->tag_and_hdr_size & BLOBLISTR_TAG_MASK) >>
		BLOBLISTR_TAG_SHIFT;
}

स्थिर अन्तःस्थ शून्य void_blob(संरचना bloblist_rec *rec)
{
	यदि (rec_tag(rec) == BLOBLISTT_VOID)
		फर्काऊ;
	rec->tag_and_hdr_size = BLOBLISTT_VOID |
				परिमाण(*rec) << BLOBLISTR_HDR_SIZE_SHIFT;
}

स्थिर अन्तःस्थ संरचना bloblist_rec *rec_from_blob(शून्य *blob)
{
	फर्काऊ (blob - परिमाण(संरचना bloblist_rec));
}

स्थिर ulong bloblist_blob_end_ofs(संरचना bloblist_hdr *hdr,
				   संरचना bloblist_rec *rec)
{
	ulong offset;

	offset = (शून्य *)rec - (शून्य *)hdr;
	/*
	 * The data section of next TE should start from an address aligned
	 * to 1 << hdr->align_log2.
	 */
	offset += rec_hdr_size(rec) + rec->size;
	offset = round_up(offset + rec_hdr_size(rec), 1 << hdr->align_log2);
	offset -= rec_hdr_size(rec);

	फर्काऊ offset;
}

स्थिर संरचना bloblist_rec *bloblist_next_blob(संरचना bloblist_hdr *hdr,
					       संरचना bloblist_rec *rec)
{
	ulong offset = bloblist_blob_end_ofs(hdr, rec);

	यदि (offset >= hdr->used_size)
		फर्काऊ रिक्त;
	फर्काऊ (संरचना bloblist_rec *)((शून्य *)hdr + offset);
}

#define foreach_rec(_rec, _hdr) \
	लागि (_rec = bloblist_first_blob(_hdr); \
	     _rec; \
	     _rec = bloblist_next_blob(_hdr, _rec))

स्थिर संरचना bloblist_rec *bloblist_findrec(uint tag)
{
	संरचना bloblist_hdr *hdr = gd->bloblist;
	संरचना bloblist_rec *rec;

	यदि (!hdr)
		फर्काऊ रिक्त;

	foreach_rec(rec, hdr) {
		यदि (rec_tag(rec) == tag)
			फर्काऊ rec;
	}

	फर्काऊ रिक्त;
}

स्थिर पूर्णाङ्क bloblist_addrec(uint tag, पूर्णाङ्क size, पूर्णाङ्क align_log2,
			   संरचना bloblist_rec **recp)
{
	संरचना bloblist_hdr *hdr = gd->bloblist;
	संरचना bloblist_rec *rec;
	phys_addr_t data_start, aligned_start;
	phys_size_t new_alloced;

	यदि (!align_log2)
		align_log2 = BLOBLIST_BLOB_ALIGN_LOG2;

	/* Figure out where the new data will start */
	data_start = map_to_sysmem(hdr) + hdr->used_size + परिमाण(*rec);

	/* Align the address and then calculate the offset from used size */
	aligned_start = ALIGN(data_start, 1U << align_log2) - data_start;

	/* If we need to create a dummy record, create it */
	यदि (aligned_start) {
		पूर्णाङ्क void_size = aligned_start - परिमाण(*rec);
		संरचना bloblist_rec *vrec;
		पूर्णाङ्क ret;

		ret = bloblist_addrec(BLOBLISTT_VOID, void_size, 0, &vrec);
		यदि (ret)
			फर्काऊ log_सन्देश_ret("void", ret);

		/* start the record after that */
		data_start = map_to_sysmem(hdr) + hdr->used_size + परिमाण(*vrec);
	}

	/* Calculate the new allocated total */
	new_alloced = data_start - map_to_sysmem(hdr) +
		ALIGN(size, 1U << align_log2);

	यदि (new_alloced > hdr->total_size) {
		log_err("Failed to allocate %x bytes\n", size);
		log_err("Used size=%x, total size=%x\n",
			hdr->used_size, hdr->total_size);
		फर्काऊ log_सन्देश_ret("bloblist add", -ENOSPC);
	}
	rec = (शून्य *)hdr + hdr->used_size;

	rec->tag_and_hdr_size = tag | परिमाण(*rec) << BLOBLISTR_HDR_SIZE_SHIFT;
	rec->size = size;

	/* Zero the record data */
	स_रखो((शून्य *)rec + rec_hdr_size(rec), '\0', rec->size);

	hdr->used_size = new_alloced;
	*recp = rec;

	फर्काऊ 0;
}

स्थिर पूर्णाङ्क bloblist_ensurerec(uint tag, संरचना bloblist_rec **recp, पूर्णाङ्क size,
			      पूर्णाङ्क align_log2)
{
	संरचना bloblist_rec *rec;

	rec = bloblist_findrec(tag);
	यदि (rec) {
		यदि (size && size != rec->size) {
			*recp = rec;
			फर्काऊ -ESPIPE;
		}
	} अन्यथा {
		पूर्णाङ्क ret;

		ret = bloblist_addrec(tag, size, align_log2, &rec);
		यदि (ret)
			फर्काऊ ret;
	}
	*recp = rec;

	फर्काऊ 0;
}

स्थिर पूर्णाङ्क bloblist_get_blob_data_offset(uint tag)
{
	विकल्प (tag) {
	पक्ष BLOBLISTT_FDT_OVERLAY:
		फर्काऊ परिमाण(संरचना dto_blob_hdr);
	/*
	 * return the data offset if it is not following the blob
	 * header immediately.
	 */
	}
	फर्काऊ 0;
}

शून्य *bloblist_find(uint tag, पूर्णाङ्क size)
{
	शून्य *blob = रिक्त;
	पूर्णाङ्क blob_size;

	blob = bloblist_get_blob(tag, &blob_size);

	यदि (size && size != blob_size)
		फर्काऊ रिक्त;

	फर्काऊ blob;
}

शून्य *bloblist_get_blob(uint tag, पूर्णाङ्क *sizep)
{
	संरचना bloblist_rec *rec;

	rec = bloblist_findrec(tag);
	यदि (!rec)
		फर्काऊ रिक्त;

	*sizep = rec->size;

	फर्काऊ (शून्य *)rec + rec_hdr_size(rec);
}

पूर्णाङ्क bloblist_apply_blobs(uint tag, पूर्णाङ्क (*func)(शून्य **data, पूर्णाङ्क size))
{
	संरचना bloblist_hdr *hdr = gd->bloblist;
	संरचना bloblist_rec *rec;

	यदि (!func || !hdr)
		फर्काऊ -ENOENT;

	foreach_rec(rec, hdr) {
		/* Apply all blobs with the specified tag */
		यदि (rec_tag(rec) == tag) {
			पूर्णाङ्क ret;
			पूर्णाङ्क tag = rec_tag(rec);
			शून्य *blob = (शून्य *)rec + rec_hdr_size(rec);
			पूर्णाङ्क dat_off = bloblist_get_blob_data_offset(tag);

			blob += dat_off;
			ret = func(&blob, rec->size - dat_off);
			यदि (ret) {
				log_err("Failed to apply blob with tag %d\n",
					tag);
				फर्काऊ ret;
			}

			rec = rec_from_blob(blob - dat_off);
			यदि (!rec) {
				log_err("Blob corrupted\n");
				फर्काऊ -ENOENT;
			}

			/* Mark applied blob record as void */
			void_blob(rec);
		}
	}

	फर्काऊ 0;
}

शून्य *bloblist_add(uint tag, पूर्णाङ्क size, पूर्णाङ्क align_log2)
{
	संरचना bloblist_rec *rec;

	यदि (bloblist_addrec(tag, size, align_log2, &rec))
		फर्काऊ रिक्त;

	फर्काऊ (शून्य *)rec + rec_hdr_size(rec);
}

पूर्णाङ्क bloblist_ensure_size(uint tag, पूर्णाङ्क size, पूर्णाङ्क align_log2, शून्य **blobp)
{
	संरचना bloblist_rec *rec;
	पूर्णाङ्क ret;

	ret = bloblist_ensurerec(tag, &rec, size, align_log2);
	यदि (ret)
		फर्काऊ ret;
	*blobp = (शून्य *)rec + rec_hdr_size(rec);

	फर्काऊ 0;
}

शून्य *bloblist_ensure(uint tag, पूर्णाङ्क size)
{
	संरचना bloblist_rec *rec;

	यदि (bloblist_ensurerec(tag, &rec, size, 0))
		फर्काऊ रिक्त;

	फर्काऊ (शून्य *)rec + rec_hdr_size(rec);
}

पूर्णाङ्क bloblist_ensure_size_ret(uint tag, पूर्णाङ्क *sizep, शून्य **blobp)
{
	संरचना bloblist_rec *rec;
	पूर्णाङ्क ret;

	ret = bloblist_ensurerec(tag, &rec, *sizep, 0);
	यदि (ret == -ESPIPE)
		*sizep = rec->size;
	अन्यथा यदि (ret)
		फर्काऊ ret;
	*blobp = (शून्य *)rec + rec_hdr_size(rec);

	फर्काऊ 0;
}

स्थिर पूर्णाङ्क bloblist_resize_rec(संरचना bloblist_hdr *hdr,
			       संरचना bloblist_rec *rec,
			       पूर्णाङ्क new_size)
{
	पूर्णाङ्क expand_by;	/* Number of bytes to expand by (-ve to contract) */
	पूर्णाङ्क new_alloced;
	ulong next_ofs;	/* Offset of the record after @rec */

	expand_by = ALIGN(new_size - rec->size, BLOBLIST_BLOB_ALIGN);
	new_alloced = ALIGN(hdr->used_size + expand_by, BLOBLIST_BLOB_ALIGN);
	यदि (new_size < 0) {
		log_debug("Attempt to shrink blob size below 0 (%x)\n",
			  new_size);
		फर्काऊ log_सन्देश_ret("size", -EINVAL);
	}
	यदि (new_alloced > hdr->total_size) {
		log_err("Failed to allocate %x bytes\n", new_size);
		log_err("Used size=%x, total size=%x\n",
			hdr->used_size, hdr->total_size);
		फर्काऊ log_सन्देश_ret("alloc", -ENOSPC);
	}

	/* Move the following blobs up or down, if this is not the last */
	next_ofs = bloblist_blob_end_ofs(hdr, rec);
	यदि (next_ofs != hdr->used_size) {
		स_हटाओ((शून्य *)hdr + next_ofs + expand_by,
			(शून्य *)hdr + next_ofs, hdr->used_size - next_ofs);
	}
	hdr->used_size = new_alloced;

	/* Zero the new part of the blob */
	यदि (expand_by > 0) {
		स_रखो((शून्य *)rec + rec_hdr_size(rec) + rec->size, '\0',
		       new_size - rec->size);
	}

	/* Update the size of this blob */
	rec->size = new_size;

	फर्काऊ 0;
}

पूर्णाङ्क bloblist_resize(uint tag, पूर्णाङ्क new_size)
{
	संरचना bloblist_hdr *hdr = gd->bloblist;
	संरचना bloblist_rec *rec;
	पूर्णाङ्क ret;

	rec = bloblist_findrec(tag);
	यदि (!rec)
		फर्काऊ log_सन्देश_ret("find", -ENOENT);
	ret = bloblist_resize_rec(hdr, rec, new_size);
	यदि (ret)
		फर्काऊ log_सन्देश_ret("resize", ret);

	फर्काऊ 0;
}

स्थिर u32 bloblist_calc_chksum(संरचना bloblist_hdr *hdr)
{
	u8 chksum;

	chksum = table_compute_checksum(hdr, hdr->used_size);
	chksum += hdr->chksum;

	फर्काऊ chksum;
}

पूर्णाङ्क bloblist_new(ulong addr, uint size, uint flags, uint align_log2)
{
	संरचना bloblist_hdr *hdr;

	यदि (size < परिमाण(*hdr))
		फर्काऊ log_ret(-ENOSPC);
	यदि (addr & (BLOBLIST_ALIGN - 1))
		फर्काऊ log_ret(-EFAULT);
	hdr = map_sysmem(addr, size);
	स_रखो(hdr, '\0', परिमाण(*hdr));
	hdr->version = BLOBLIST_VERSION;
	hdr->hdr_size = परिमाण(*hdr);
	hdr->flags = flags;
	hdr->magic = BLOBLIST_MAGIC;
	hdr->used_size = hdr->hdr_size;
	hdr->total_size = size;
	hdr->align_log2 = align_log2 ? align_log2 : BLOBLIST_BLOB_ALIGN_LOG2;
	hdr->chksum = 0;
	gd->bloblist = hdr;
	gd->flags |= GD_FLG_BLOBLIST_HANDOFF;

	फर्काऊ 0;
}

पूर्णाङ्क bloblist_check(ulong addr, uint size)
{
	संरचना bloblist_hdr *hdr;
	u32 chksum;

	hdr = map_sysmem(addr, परिमाण(*hdr));
	यदि (hdr->magic != BLOBLIST_MAGIC)
		फर्काऊ log_सन्देश_ret("Bad magic", -ENOENT);
	यदि (hdr->version != BLOBLIST_VERSION)
		फर्काऊ log_सन्देश_ret("Bad version", -EPROTONOSUPPORT);
	यदि (!hdr->total_size || (size && hdr->total_size > size))
		फर्काऊ log_सन्देश_ret("Bad total size", -EFBIG);
	यदि (hdr->used_size > hdr->total_size)
		फर्काऊ log_सन्देश_ret("Bad used size", -ENOENT);
	यदि (hdr->hdr_size != परिमाण(संरचना bloblist_hdr))
		फर्काऊ log_सन्देश_ret("Bad header size", -ENOENT);

	chksum = bloblist_calc_chksum(hdr);
	यदि (hdr->chksum != chksum) {
		log_err("Checksum %x != %x\n", hdr->chksum, chksum);
		फर्काऊ log_सन्देश_ret("Bad checksum", -EIO);
	}
	gd->bloblist = hdr;
	gd->flags |= GD_FLG_BLOBLIST_HANDOFF;

	फर्काऊ 0;
}

पूर्णाङ्क bloblist_finish(शून्य)
{
	संरचना bloblist_hdr *hdr = gd->bloblist;

	hdr->chksum = bloblist_calc_chksum(hdr);
	log_debug("Finished bloblist size %lx at %lx\n", (ulong)hdr->used_size,
		  (ulong)map_to_sysmem(hdr));

	फर्काऊ 0;
}

ulong bloblist_get_base(शून्य)
{
	फर्काऊ map_to_sysmem(gd->bloblist);
}

ulong bloblist_get_size(शून्य)
{
	संरचना bloblist_hdr *hdr = gd->bloblist;

	फर्काऊ hdr->used_size;
}

ulong bloblist_get_total_size(शून्य)
{
	संरचना bloblist_hdr *hdr = gd->bloblist;

	फर्काऊ hdr->total_size;
}

शून्य bloblist_get_stats(ulong *basep, ulong *tsizep, ulong *usizep)
{
	संरचना bloblist_hdr *hdr = gd->bloblist;

	*basep = map_to_sysmem(gd->bloblist);
	*tsizep = hdr->total_size;
	*usizep = hdr->used_size;
}

स्थिर शून्य show_मान(अचर वर्ण *prompt, ulong मान)
{
	म_लिखो("%s:%*s %-5lx  ", prompt, 10 - (पूर्णाङ्क)म_माप(prompt), "", मान);
	print_size(मान, "\n");
}

शून्य bloblist_show_stats(शून्य)
{
	ulong base, tsize, usize;

	bloblist_get_stats(&base, &tsize, &usize);
	म_लिखो("base:       %lx\n", base);
	show_मान("total size", tsize);
	show_मान("used size", usize);
	show_मान("free", tsize - usize);
}

शून्य bloblist_show_list(शून्य)
{
	संरचना bloblist_hdr *hdr = gd->bloblist;
	संरचना bloblist_rec *rec;

	म_लिखो("%-8s  %8s   Tag Name\n", "Address", "Size");
	लागि (rec = bloblist_first_blob(hdr); rec;
	     rec = bloblist_next_blob(hdr, rec)) {
		म_लिखो("%08lx  %8x  %4x %s\n",
		       (ulong)map_to_sysmem((शून्य *)rec + rec_hdr_size(rec)),
		       rec->size, rec_tag(rec),
		       bloblist_tag_name(rec_tag(rec)));
	}
}

पूर्णाङ्क bloblist_reloc(शून्य *to, uint to_size)
{
	संरचना bloblist_hdr *hdr;

	यदि (!to_size)
		फर्काऊ 0;

	यदि (to_size < gd->bloblist->total_size)
		फर्काऊ -ENOSPC;

	स_नकल(to, gd->bloblist, gd->bloblist->total_size);
	hdr = to;
	hdr->total_size = to_size;
	gd->bloblist = to;

	फर्काऊ 0;
}

/*
 * Weak default function for getting bloblist from boot args.
 */
पूर्णाङ्क __weak xferlist_from_boot_arg(ulong __always_unused *addr)
{
	फर्काऊ -ENOENT;
}

bool bloblist_exists(शून्य)
{
	पूर्णाङ्क ret;
	ulong addr = 0;

	/* Check if a valid transfer list passed in */
	यदि (!xferlist_from_boot_arg(&addr))
		गच्छ found;

	/*
	 * If Firmware Handoff is mandatory but no transfer list is
	 * observed, report it as an error.
	 */
	यदि (IS_ENABLED(CONFIG_BLOBLIST_PASSAGE_MANDATORY))
		फर्काऊ false;

	/*
	 * We have checked for a valid transfer list being passed. At this
	 * point, if we do not have a fixed address for the bloblist, we cannot
	 * be provided with one.
	 */
	यदि (xpl_is_first_phase() || !IS_ENABLED(CONFIG_BLOBLIST_FIXED))
		फर्काऊ false;

	/*
	 * Check for a valid list as the configured address.
	 */
	addr = IF_ENABLED_INT(CONFIG_BLOBLIST_FIXED,
			      CONFIG_BLOBLIST_ADDR);
	ret = bloblist_check(addr, CONFIG_BLOBLIST_SIZE);
	यदि (!ret)
		गच्छ found;

	log_debug("Bloblist at %lx not found (err=%d)\n", addr, ret);
	फर्काऊ false;

found:
#ifdef DEBUG
	bloblist_show_stats();
	bloblist_show_list();
#endif
	फर्काऊ true;
}

पूर्णाङ्क bloblist_init(शून्य)
{
	पूर्णाङ्क ret;
	ulong addr = 0, size = CONFIG_BLOBLIST_SIZE;

	यदि (gd->flags & GD_FLG_BLOBLIST_HANDOFF) {
		log_debug("Found existing bloblist size %x at %p\n",
			  gd->bloblist->total_size, gd->bloblist);
		फर्काऊ 0;
	}

	/*
	 * If Firmware Handoff is mandatory but no transfer list has been
	 * observed by fdtdec_setup, report it as an error.
	 */
	यदि (IS_ENABLED(CONFIG_BLOBLIST_PASSAGE_MANDATORY))
		फर्काऊ -ENOENT;

	/*
	 * If we don't have an existing bloblist, we either need
	 * to allocate one now, or initialize the fixed address
	 * space as a bloblist.
	 */
	यदि (CONFIG_IS_ENABLED(BLOBLIST_ALLOC)) {
		शून्य *ptr = memalign(BLOBLIST_ALIGN, size);

		यदि (!ptr)
			फर्काऊ log_सन्देश_ret("alloc", -ENOMEM);
		addr = map_to_sysmem(ptr);
	} अन्यथा
		addr = IF_ENABLED_INT(CONFIG_BLOBLIST_FIXED,
				      CONFIG_BLOBLIST_ADDR);

	log_debug("Creating new bloblist size %lx at %lx\n", size,
		  addr);
	ret = bloblist_new(addr, size, 0, 0);
	यदि (ret)
		फर्काऊ log_सन्देश_ret("ini", ret);

	फर्काऊ 0;
}

पूर्णाङ्क bloblist_check_reg_conv(ulong rfdt, ulong rzero, ulong rsig, ulong xlist)
{
	u64 version = BLOBLIST_REGCONV_VER;
	ulong sigval;
	पूर्णाङ्क ret;

	यदि ((IS_ENABLED(CONFIG_64BIT) && !IS_ENABLED(CONFIG_SPL_BUILD)) ||
			(IS_ENABLED(CONFIG_SPL_64BIT) && IS_ENABLED(CONFIG_SPL_BUILD))) {
		sigval = ((BLOBLIST_MAGIC & ((1ULL << BLOBLIST_REGCONV_SHIFT_64) - 1)) |
			 ((version  & BLOBLIST_REGCONV_MASK) << BLOBLIST_REGCONV_SHIFT_64));
	} अन्यथा {
		sigval = ((BLOBLIST_MAGIC & ((1UL << BLOBLIST_REGCONV_SHIFT_32) - 1)) |
			 ((version  & BLOBLIST_REGCONV_MASK) << BLOBLIST_REGCONV_SHIFT_32));
	}

	यदि (rzero || rsig != sigval)
		फर्काऊ -EIO;

	ret = bloblist_check(xlist, 0);
	यदि (ret)
		फर्काऊ ret;

	यदि (rfdt != (ulong)bloblist_find(BLOBLISTT_CONTROL_FDT, 0)) {
		/* Remove this bloblist from gd */
		gd->bloblist = रिक्त;
		gd->flags &= ~GD_FLG_BLOBLIST_HANDOFF;
		फर्काऊ -EIO;
	}

	फर्काऊ 0;
}
