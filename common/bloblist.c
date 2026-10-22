// SPDX-License-Identifier: GPL-2.0+ BSD-3-Clause
/*
 * Copyright 2018 Google, Inc
 * Written by Simon Glass <sjg@chromium.org>
 */

#घोषणा LOG_CATEGORY	LOGC_BLOBLIST

#समावेश <bloblist.h>
#समावेश <display_options.h>
#समावेश <log.h>
#समावेश <malloc.h>
#समावेश <mapmem.h>
#समावेश <spl.h>
#समावेश <tables_csum.h>
#समावेश <asm/global_data.h>
#समावेश <u-boot/crc.h>

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

DECLARE_GLOBAL_तथ्य_PTR;

अटल काष्ठा tag_name {
	क्रमागत bloblist_tag_t tag;
	स्थिर अक्षर *name;
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

स्थिर अक्षर *bloblist_tag_name(क्रमागत bloblist_tag_t tag)
{
	पूर्णांक i;

	क्रम (i = 0; i < ARRAY_आकार(tag_name); i++) {
		अगर (tag_name[i].tag == tag)
			वापस tag_name[i].name;
	}

	वापस "invalid";
}

अटल काष्ठा bloblist_rec *bloblist_first_blob(काष्ठा bloblist_hdr *hdr)
{
	अगर (hdr->used_आकार <= hdr->hdr_आकार)
		वापस शून्य;
	वापस (काष्ठा bloblist_rec *)((व्योम *)hdr + hdr->hdr_आकार);
}

अटल अंतरभूत uint rec_hdr_आकार(काष्ठा bloblist_rec *rec)
{
	वापस (rec->tag_and_hdr_आकार & BLOBLISTR_HDR_आकार_MASK) >>
		BLOBLISTR_HDR_आकार_SHIFT;
}

अटल अंतरभूत uint rec_tag(काष्ठा bloblist_rec *rec)
{
	वापस (rec->tag_and_hdr_आकार & BLOBLISTR_TAG_MASK) >>
		BLOBLISTR_TAG_SHIFT;
}

अटल अंतरभूत व्योम void_blob(काष्ठा bloblist_rec *rec)
{
	अगर (rec_tag(rec) == BLOBLISTT_VOID)
		वापस;
	rec->tag_and_hdr_आकार = BLOBLISTT_VOID |
				माप(*rec) << BLOBLISTR_HDR_आकार_SHIFT;
}

अटल अंतरभूत काष्ठा bloblist_rec *rec_from_blob(व्योम *blob)
{
	वापस (blob - माप(काष्ठा bloblist_rec));
}

अटल ulong bloblist_blob_end_ofs(काष्ठा bloblist_hdr *hdr,
				   काष्ठा bloblist_rec *rec)
{
	ulong offset;

	offset = (व्योम *)rec - (व्योम *)hdr;
	/*
	 * The data section of next TE should start from an address aligned
	 * to 1 << hdr->align_log2.
	 */
	offset += rec_hdr_आकार(rec) + rec->आकार;
	offset = round_up(offset + rec_hdr_आकार(rec), 1 << hdr->align_log2);
	offset -= rec_hdr_आकार(rec);

	वापस offset;
}

अटल काष्ठा bloblist_rec *bloblist_अग्र_blob(काष्ठा bloblist_hdr *hdr,
					       काष्ठा bloblist_rec *rec)
{
	ulong offset = bloblist_blob_end_ofs(hdr, rec);

	अगर (offset >= hdr->used_आकार)
		वापस शून्य;
	वापस (काष्ठा bloblist_rec *)((व्योम *)hdr + offset);
}

#घोषणा foreach_rec(_rec, _hdr) \
	क्रम (_rec = bloblist_first_blob(_hdr); \
	     _rec; \
	     _rec = bloblist_अग्र_blob(_hdr, _rec))

अटल काष्ठा bloblist_rec *bloblist_findrec(uint tag)
{
	काष्ठा bloblist_hdr *hdr = gd->bloblist;
	काष्ठा bloblist_rec *rec;

	अगर (!hdr)
		वापस शून्य;

	foreach_rec(rec, hdr) {
		अगर (rec_tag(rec) == tag)
			वापस rec;
	}

	वापस शून्य;
}

अटल पूर्णांक bloblist_addrec(uint tag, पूर्णांक आकार, पूर्णांक align_log2,
			   काष्ठा bloblist_rec **recp)
{
	काष्ठा bloblist_hdr *hdr = gd->bloblist;
	काष्ठा bloblist_rec *rec;
	phys_addr_t तथ्य_start, aligned_start;
	phys_आकार_t new_alloced;

	अगर (!align_log2)
		align_log2 = BLOBLIST_BLOB_ALIGN_LOG2;

	/* Figure out where the new data will start */
	तथ्य_start = map_to_sysmem(hdr) + hdr->used_आकार + माप(*rec);

	/* Align the address and then calculate the offset from used size */
	aligned_start = ALIGN(तथ्य_start, 1U << align_log2) - तथ्य_start;

	/* If we need to create a dummy record, create it */
	अगर (aligned_start) {
		पूर्णांक void_आकार = aligned_start - माप(*rec);
		काष्ठा bloblist_rec *vrec;
		पूर्णांक ret;

		ret = bloblist_addrec(BLOBLISTT_VOID, void_आकार, 0, &vrec);
		अगर (ret)
			वापस log_संदेश_ret("void", ret);

		/* start the record after that */
		तथ्य_start = map_to_sysmem(hdr) + hdr->used_आकार + माप(*vrec);
	}

	/* Calculate the new allocated total */
	new_alloced = तथ्य_start - map_to_sysmem(hdr) +
		ALIGN(आकार, 1U << align_log2);

	अगर (new_alloced > hdr->योग_आकार) {
		log_err("Failed to allocate %x bytes\n", आकार);
		log_err("Used size=%x, total size=%x\n",
			hdr->used_आकार, hdr->योग_आकार);
		वापस log_संदेश_ret("bloblist add", -ENOSPC);
	}
	rec = (व्योम *)hdr + hdr->used_आकार;

	rec->tag_and_hdr_आकार = tag | माप(*rec) << BLOBLISTR_HDR_आकार_SHIFT;
	rec->आकार = आकार;

	/* Zero the record data */
	स_रखो((व्योम *)rec + rec_hdr_आकार(rec), '\0', rec->आकार);

	hdr->used_आकार = new_alloced;
	*recp = rec;

	वापस 0;
}

अटल पूर्णांक bloblist_ensurerec(uint tag, काष्ठा bloblist_rec **recp, पूर्णांक आकार,
			      पूर्णांक align_log2)
{
	काष्ठा bloblist_rec *rec;

	rec = bloblist_findrec(tag);
	अगर (rec) {
		अगर (आकार && आकार != rec->आकार) {
			*recp = rec;
			वापस -ESPIPE;
		}
	} अन्यथा {
		पूर्णांक ret;

		ret = bloblist_addrec(tag, आकार, align_log2, &rec);
		अगर (ret)
			वापस ret;
	}
	*recp = rec;

	वापस 0;
}

अटल पूर्णांक bloblist_get_blob_तथ्य_offset(uint tag)
{
	चयन (tag) {
	हाल BLOBLISTT_FDT_OVERLAY:
		वापस माप(काष्ठा dto_blob_hdr);
	/*
	 * return the data offset if it is not following the blob
	 * header immediately.
	 */
	}
	वापस 0;
}

व्योम *bloblist_find(uint tag, पूर्णांक आकार)
{
	व्योम *blob = शून्य;
	पूर्णांक blob_आकार;

	blob = bloblist_get_blob(tag, &blob_आकार);

	अगर (आकार && आकार != blob_आकार)
		वापस शून्य;

	वापस blob;
}

व्योम *bloblist_get_blob(uint tag, पूर्णांक *sizep)
{
	काष्ठा bloblist_rec *rec;

	rec = bloblist_findrec(tag);
	अगर (!rec)
		वापस शून्य;

	*sizep = rec->आकार;

	वापस (व्योम *)rec + rec_hdr_आकार(rec);
}

पूर्णांक bloblist_apply_blobs(uint tag, पूर्णांक (*func)(व्योम **तथ्य, पूर्णांक आकार))
{
	काष्ठा bloblist_hdr *hdr = gd->bloblist;
	काष्ठा bloblist_rec *rec;

	अगर (!func || !hdr)
		वापस -ENOENT;

	foreach_rec(rec, hdr) {
		/* Apply all blobs with the specified tag */
		अगर (rec_tag(rec) == tag) {
			पूर्णांक ret;
			पूर्णांक tag = rec_tag(rec);
			व्योम *blob = (व्योम *)rec + rec_hdr_आकार(rec);
			पूर्णांक dat_off = bloblist_get_blob_तथ्य_offset(tag);

			blob += dat_off;
			ret = func(&blob, rec->आकार - dat_off);
			अगर (ret) {
				log_err("Failed to apply blob with tag %d\n",
					tag);
				वापस ret;
			}

			rec = rec_from_blob(blob - dat_off);
			अगर (!rec) {
				log_err("Blob corrupted\n");
				वापस -ENOENT;
			}

			/* Mark applied blob record as void */
			void_blob(rec);
		}
	}

	वापस 0;
}

व्योम *bloblist_add(uint tag, पूर्णांक आकार, पूर्णांक align_log2)
{
	काष्ठा bloblist_rec *rec;

	अगर (bloblist_addrec(tag, आकार, align_log2, &rec))
		वापस शून्य;

	वापस (व्योम *)rec + rec_hdr_आकार(rec);
}

पूर्णांक bloblist_ensure_आकार(uint tag, पूर्णांक आकार, पूर्णांक align_log2, व्योम **blobp)
{
	काष्ठा bloblist_rec *rec;
	पूर्णांक ret;

	ret = bloblist_ensurerec(tag, &rec, आकार, align_log2);
	अगर (ret)
		वापस ret;
	*blobp = (व्योम *)rec + rec_hdr_आकार(rec);

	वापस 0;
}

व्योम *bloblist_ensure(uint tag, पूर्णांक आकार)
{
	काष्ठा bloblist_rec *rec;

	अगर (bloblist_ensurerec(tag, &rec, आकार, 0))
		वापस शून्य;

	वापस (व्योम *)rec + rec_hdr_आकार(rec);
}

पूर्णांक bloblist_ensure_आकार_ret(uint tag, पूर्णांक *sizep, व्योम **blobp)
{
	काष्ठा bloblist_rec *rec;
	पूर्णांक ret;

	ret = bloblist_ensurerec(tag, &rec, *sizep, 0);
	अगर (ret == -ESPIPE)
		*sizep = rec->आकार;
	अन्यथा अगर (ret)
		वापस ret;
	*blobp = (व्योम *)rec + rec_hdr_आकार(rec);

	वापस 0;
}

अटल पूर्णांक bloblist_resize_rec(काष्ठा bloblist_hdr *hdr,
			       काष्ठा bloblist_rec *rec,
			       पूर्णांक new_आकार)
{
	पूर्णांक expand_by;	/* Number of bytes to expand by (-ve to contract) */
	पूर्णांक new_alloced;
	ulong अग्र_ofs;	/* Offset of the record after @rec */

	expand_by = ALIGN(new_आकार - rec->आकार, BLOBLIST_BLOB_ALIGN);
	new_alloced = ALIGN(hdr->used_आकार + expand_by, BLOBLIST_BLOB_ALIGN);
	अगर (new_आकार < 0) {
		log_debug("Attempt to shrink blob size below 0 (%x)\n",
			  new_आकार);
		वापस log_संदेश_ret("size", -EINVAL);
	}
	अगर (new_alloced > hdr->योग_आकार) {
		log_err("Failed to allocate %x bytes\n", new_आकार);
		log_err("Used size=%x, total size=%x\n",
			hdr->used_आकार, hdr->योग_आकार);
		वापस log_संदेश_ret("alloc", -ENOSPC);
	}

	/* Move the following blobs up or down, if this is not the last */
	अग्र_ofs = bloblist_blob_end_ofs(hdr, rec);
	अगर (अग्र_ofs != hdr->used_आकार) {
		स_हटाओ((व्योम *)hdr + अग्र_ofs + expand_by,
			(व्योम *)hdr + अग्र_ofs, hdr->used_आकार - अग्र_ofs);
	}
	hdr->used_आकार = new_alloced;

	/* Zero the new part of the blob */
	अगर (expand_by > 0) {
		स_रखो((व्योम *)rec + rec_hdr_आकार(rec) + rec->आकार, '\0',
		       new_आकार - rec->आकार);
	}

	/* Update the size of this blob */
	rec->आकार = new_आकार;

	वापस 0;
}

पूर्णांक bloblist_resize(uint tag, पूर्णांक new_आकार)
{
	काष्ठा bloblist_hdr *hdr = gd->bloblist;
	काष्ठा bloblist_rec *rec;
	पूर्णांक ret;

	rec = bloblist_findrec(tag);
	अगर (!rec)
		वापस log_संदेश_ret("find", -ENOENT);
	ret = bloblist_resize_rec(hdr, rec, new_आकार);
	अगर (ret)
		वापस log_संदेश_ret("resize", ret);

	वापस 0;
}

अटल u32 bloblist_calc_chksum(काष्ठा bloblist_hdr *hdr)
{
	u8 chksum;

	chksum = table_compute_checksum(hdr, hdr->used_आकार);
	chksum += hdr->chksum;

	वापस chksum;
}

पूर्णांक bloblist_new(ulong addr, uint आकार, uint flags, uint align_log2)
{
	काष्ठा bloblist_hdr *hdr;

	अगर (आकार < माप(*hdr))
		वापस log_ret(-ENOSPC);
	अगर (addr & (BLOBLIST_ALIGN - 1))
		वापस log_ret(-EFAULT);
	hdr = map_sysmem(addr, आकार);
	स_रखो(hdr, '\0', माप(*hdr));
	hdr->version = BLOBLIST_VERSION;
	hdr->hdr_आकार = माप(*hdr);
	hdr->flags = flags;
	hdr->magic = BLOBLIST_MAGIC;
	hdr->used_आकार = hdr->hdr_आकार;
	hdr->योग_आकार = आकार;
	hdr->align_log2 = align_log2 ? align_log2 : BLOBLIST_BLOB_ALIGN_LOG2;
	hdr->chksum = 0;
	gd->bloblist = hdr;
	gd->flags |= GD_FLG_BLOBLIST_HANDOFF;

	वापस 0;
}

पूर्णांक bloblist_check(ulong addr, uint आकार)
{
	काष्ठा bloblist_hdr *hdr;
	u32 chksum;

	hdr = map_sysmem(addr, माप(*hdr));
	अगर (hdr->magic != BLOBLIST_MAGIC)
		वापस log_संदेश_ret("Bad magic", -ENOENT);
	अगर (hdr->version != BLOBLIST_VERSION)
		वापस log_संदेश_ret("Bad version", -EPROTONOSUPPORT);
	अगर (!hdr->योग_आकार || (आकार && hdr->योग_आकार > आकार))
		वापस log_संदेश_ret("Bad total size", -EFBIG);
	अगर (hdr->used_आकार > hdr->योग_आकार)
		वापस log_संदेश_ret("Bad used size", -ENOENT);
	अगर (hdr->hdr_आकार != माप(काष्ठा bloblist_hdr))
		वापस log_संदेश_ret("Bad header size", -ENOENT);

	chksum = bloblist_calc_chksum(hdr);
	अगर (hdr->chksum != chksum) {
		log_err("Checksum %x != %x\n", hdr->chksum, chksum);
		वापस log_संदेश_ret("Bad checksum", -EIO);
	}
	gd->bloblist = hdr;
	gd->flags |= GD_FLG_BLOBLIST_HANDOFF;

	वापस 0;
}

पूर्णांक bloblist_finish(व्योम)
{
	काष्ठा bloblist_hdr *hdr = gd->bloblist;

	hdr->chksum = bloblist_calc_chksum(hdr);
	log_debug("Finished bloblist size %lx at %lx\n", (ulong)hdr->used_आकार,
		  (ulong)map_to_sysmem(hdr));

	वापस 0;
}

ulong bloblist_get_base(व्योम)
{
	वापस map_to_sysmem(gd->bloblist);
}

ulong bloblist_get_आकार(व्योम)
{
	काष्ठा bloblist_hdr *hdr = gd->bloblist;

	वापस hdr->used_आकार;
}

ulong bloblist_get_योग_आकार(व्योम)
{
	काष्ठा bloblist_hdr *hdr = gd->bloblist;

	वापस hdr->योग_आकार;
}

व्योम bloblist_get_stats(ulong *basep, ulong *tsizep, ulong *usizep)
{
	काष्ठा bloblist_hdr *hdr = gd->bloblist;

	*basep = map_to_sysmem(gd->bloblist);
	*tsizep = hdr->योग_आकार;
	*usizep = hdr->used_आकार;
}

अटल व्योम show_मूल्य(स्थिर अक्षर *prompt, ulong मूल्य)
{
	म_लिखो("%s:%*s %-5lx  ", prompt, 10 - (पूर्णांक)म_माप(prompt), "", मूल्य);
	print_आकार(मूल्य, "\n");
}

व्योम bloblist_show_stats(व्योम)
{
	ulong base, tsize, usize;

	bloblist_get_stats(&base, &tsize, &usize);
	म_लिखो("base:       %lx\n", base);
	show_मूल्य("total size", tsize);
	show_मूल्य("used size", usize);
	show_मूल्य("free", tsize - usize);
}

व्योम bloblist_show_list(व्योम)
{
	काष्ठा bloblist_hdr *hdr = gd->bloblist;
	काष्ठा bloblist_rec *rec;

	म_लिखो("%-8s  %8s   Tag Name\n", "Address", "Size");
	क्रम (rec = bloblist_first_blob(hdr); rec;
	     rec = bloblist_अग्र_blob(hdr, rec)) {
		म_लिखो("%08lx  %8x  %4x %s\n",
		       (ulong)map_to_sysmem((व्योम *)rec + rec_hdr_आकार(rec)),
		       rec->आकार, rec_tag(rec),
		       bloblist_tag_name(rec_tag(rec)));
	}
}

पूर्णांक bloblist_reloc(व्योम *to, uint to_आकार)
{
	काष्ठा bloblist_hdr *hdr;

	अगर (!to_आकार)
		वापस 0;

	अगर (to_आकार < gd->bloblist->योग_आकार)
		वापस -ENOSPC;

	स_नकल(to, gd->bloblist, gd->bloblist->योग_आकार);
	hdr = to;
	hdr->योग_आकार = to_आकार;
	gd->bloblist = to;

	वापस 0;
}

/*
 * Weak default function for getting bloblist from boot args.
 */
पूर्णांक __weak xferlist_from_boot_arg(ulong __always_unused *addr)
{
	वापस -ENOENT;
}

bool bloblist_exists(व्योम)
{
	पूर्णांक ret;
	ulong addr = 0;

	/* Check if a valid transfer list passed in */
	अगर (!xferlist_from_boot_arg(&addr))
		जाओ found;

	/*
	 * If Firmware Handoff is mandatory but no transfer list is
	 * observed, report it as an error.
	 */
	अगर (IS_ENABLED(CONFIG_BLOBLIST_PASSAGE_MANDATORY))
		वापस false;

	/*
	 * We have checked for a valid transfer list being passed. At this
	 * point, if we do not have a fixed address for the bloblist, we cannot
	 * be provided with one.
	 */
	अगर (xpl_is_first_phase() || !IS_ENABLED(CONFIG_BLOBLIST_FIXED))
		वापस false;

	/*
	 * Check for a valid list as the configured address.
	 */
	addr = IF_ENABLED_INT(CONFIG_BLOBLIST_FIXED,
			      CONFIG_BLOBLIST_ADDR);
	ret = bloblist_check(addr, CONFIG_BLOBLIST_आकार);
	अगर (!ret)
		जाओ found;

	log_debug("Bloblist at %lx not found (err=%d)\n", addr, ret);
	वापस false;

found:
#अगर_घोषित DEBUG
	bloblist_show_stats();
	bloblist_show_list();
#पूर्ण_अगर
	वापस true;
}

पूर्णांक bloblist_init(व्योम)
{
	पूर्णांक ret;
	ulong addr = 0, आकार = CONFIG_BLOBLIST_आकार;

	अगर (gd->flags & GD_FLG_BLOBLIST_HANDOFF) {
		log_debug("Found existing bloblist size %x at %p\n",
			  gd->bloblist->योग_आकार, gd->bloblist);
		वापस 0;
	}

	/*
	 * If Firmware Handoff is mandatory but no transfer list has been
	 * observed by fdtdec_setup, report it as an error.
	 */
	अगर (IS_ENABLED(CONFIG_BLOBLIST_PASSAGE_MANDATORY))
		वापस -ENOENT;

	/*
	 * If we don't have an existing bloblist, we either need
	 * to allocate one now, or initialize the fixed address
	 * space as a bloblist.
	 */
	अगर (CONFIG_IS_ENABLED(BLOBLIST_ALLOC)) {
		व्योम *ptr = memalign(BLOBLIST_ALIGN, आकार);

		अगर (!ptr)
			वापस log_संदेश_ret("alloc", -ENOMEM);
		addr = map_to_sysmem(ptr);
	} अन्यथा
		addr = IF_ENABLED_INT(CONFIG_BLOBLIST_FIXED,
				      CONFIG_BLOBLIST_ADDR);

	log_debug("Creating new bloblist size %lx at %lx\n", आकार,
		  addr);
	ret = bloblist_new(addr, आकार, 0, 0);
	अगर (ret)
		वापस log_संदेश_ret("ini", ret);

	वापस 0;
}

पूर्णांक bloblist_check_reg_conv(ulong rfdt, ulong rzero, ulong rsig, ulong xlist)
{
	u64 version = BLOBLIST_REGCONV_VER;
	ulong sigval;
	पूर्णांक ret;

	अगर ((IS_ENABLED(CONFIG_64BIT) && !IS_ENABLED(CONFIG_SPL_BUILD)) ||
			(IS_ENABLED(CONFIG_SPL_64BIT) && IS_ENABLED(CONFIG_SPL_BUILD))) {
		sigval = ((BLOBLIST_MAGIC & ((1ULL << BLOBLIST_REGCONV_SHIFT_64) - 1)) |
			 ((version  & BLOBLIST_REGCONV_MASK) << BLOBLIST_REGCONV_SHIFT_64));
	} अन्यथा {
		sigval = ((BLOBLIST_MAGIC & ((1UL << BLOBLIST_REGCONV_SHIFT_32) - 1)) |
			 ((version  & BLOBLIST_REGCONV_MASK) << BLOBLIST_REGCONV_SHIFT_32));
	}

	अगर (rzero || rsig != sigval)
		वापस -EIO;

	ret = bloblist_check(xlist, 0);
	अगर (ret)
		वापस ret;

	अगर (rfdt != (ulong)bloblist_find(BLOBLISTT_CONTROL_FDT, 0)) {
		/* Remove this bloblist from gd */
		gd->bloblist = शून्य;
		gd->flags &= ~GD_FLG_BLOBLIST_HANDOFF;
		वापस -EIO;
	}

	वापस 0;
}
