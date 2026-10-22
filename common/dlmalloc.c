// SPDX-License-Identifier: GPL-2.0+
/*
 * This code is based on a version (aka dlmalloc) of malloc/free/realloc written
 * by Doug Lea and released to the public domain, as explained at
 * http://creativecommons.org/publicdomain/zero/1.0/-
 *
 * The original code is available at http://gee.cs.oswego.edu/pub/misc/
 * as file malloc-2.6.6.c.
 */

#यदि CONFIG_IS_ENABLED(UNIT_TEST)
#परिभाषय DEBUG
#यदि_अन्तः

#अन्तर्भाव <log.h>
#अन्तर्भाव <asm/global_data.h>

#अन्तर्भाव <malloc.h>
#अन्तर्भाव <mapmem.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <asm/io.h>
#अन्तर्भाव <valgrind/memcheck.h>

#यदि_परिभाषितम् DEBUG
#यदि __STD_C
स्थिर शून्य malloc_update_mallinfo (शून्य);
शून्य malloc_stats (शून्य);
#अन्यथा
स्थिर शून्य malloc_update_mallinfo ();
शून्य malloc_stats();
#यदि_अन्तः
#यदि_अन्तः	/* DEBUG */

DECLARE_GLOBAL_दत्त_PTR;

#यदि_परिभाषितम् MCHECK_HEAP_PROTECTION
 #परिभाषय STATIC_IF_MCHECK स्थिर
 #अपरिभाषय MALLOC_COPY
 #अपरिभाषय MALLOC_ZERO
स्थिर अन्तःस्थ शून्य MALLOC_ZERO(शून्य *p, size_t sz) { स्मृतिं_पूरय(p, 0, sz); }
स्थिर अन्तःस्थ शून्य MALLOC_COPY(शून्य *dest, ध्रुव शून्य *src, size_t sz) { स्मृतिं_प्रतिलिख(dest, src, sz); }
#अन्यथा
 #परिभाषय STATIC_IF_MCHECK
 #परिभाषय mALLOc_impl mALLOc
 #परिभाषय fREe_impl fREe
 #परिभाषय rEALLOc_impl rEALLOc
 #परिभाषय mEMALIGn_impl mEMALIGn
 #परिभाषय cALLOc_impl cALLOc
#यदि_अन्तः

/*
  Emulation of sbrk for WIN32
  All code within the ifdef WIN32 is untested by me.

  Thanks to Martin Fong and others for supplying this.
*/

#यदि_परिभाषितम् WIN32

#परिभाषय AlignPage(सङ्कलय) (((सङ्कलय) + (malloc_getpagesize-1)) & \
~(malloc_getpagesize-1))
#परिभाषय AlignPage64K(सङ्कलय) (((सङ्कलय) + (0x10000 - 1)) & ~(0x10000 - 1))

/* resrve 64MB to insure large contiguous space */
#परिभाषय RESERVED_आकार (1024*1024*64)
#परिभाषय अग्र_आकार (2048*1024)
#परिभाषय शिखरम्_स्मृतिः ((अचिह्नित दीर्घ)2*1024*1024*1024)

संरचना GmListElement;
प्रकारनिर्देश संरचना GmListElement GmListElement;

संरचना GmListElement
{
	GmListElement* अग्र;
	शून्य* आधारः;
};

स्थिर GmListElement* शीर्ष = 0;
स्थिर अचिह्नित पूर्णाङ्क g_अग्र_स्थानाङ्कः = 0;
स्थिर अचिह्नित पूर्णाङ्क g_स्थानाङ्कः_आधारः = 0;
स्थिर अचिह्नित पूर्णाङ्क g_Allocated_आकार = 0;

स्थिर
GmListElement* makeGmListElement (शून्य* bas)
{
	GmListElement* this;
	this = (GmListElement*)(शून्य*)LocalAlloc (0, परिमाण (GmListElement));
	प्रतिज्ञा (this);
	यदि (this)
	{
		this->आधारः = bas;
		this->अग्र = शीर्ष;
		शीर्ष = this;
	}
	प्रत्यागम this;
}

शून्य gcleanup (शून्य)
{
	BOOL rval;
	प्रतिज्ञा ( (शीर्ष == रिक्त) || (शीर्ष->आधारः == (शून्य*)g_स्थानाङ्कः_आधारः));
	यदि (g_स्थानाङ्कः_आधारः && (g_अग्र_स्थानाङ्कः - g_स्थानाङ्कः_आधारः))
	{
		rval = VirtualFree ((शून्य*)g_स्थानाङ्कः_आधारः,
							g_अग्र_स्थानाङ्कः - g_स्थानाङ्कः_आधारः,
							MEM_DECOMMIT);
	प्रतिज्ञा (rval);
	}
	यावत् (शीर्ष)
	{
		GmListElement* अग्र = शीर्ष->अग्र;
		rval = VirtualFree (शीर्ष->आधारः, 0, MEM_RELEASE);
		प्रतिज्ञा (rval);
		LocalFree (शीर्ष);
		शीर्ष = अग्र;
	}
}

स्थिर
शून्य* findRegion (शून्य* start_स्थानाङ्कः, अचिह्नित दीर्घ आकार)
{
	स्मृतिः_BASIC_INFORMATION info;
	यदि (आकार >= शिखरम्_स्मृतिः) प्रत्यागम रिक्त;

	यावत् ((अचिह्नित दीर्घ)start_स्थानाङ्कः + आकार < शिखरम्_स्मृतिः)
	{
		VirtualQuery (start_स्थानाङ्कः, &info, परिमाण (info));
		यदि ((info.State == MEM_FREE) && (info.Region_आकार >= आकार))
			प्रत्यागम start_स्थानाङ्कः;
		अन्यथा
		{
			/* Requested region is not available so see if the */
			/* next region is available.  Set 'start_address' */
			/* to the next region and call 'VirtualQuery()' */
			/* again. */

			start_स्थानाङ्कः = (वर्ण*)info.आधारः_स्थानाङ्कः + info.Region_आकार;

			/* Make sure we start looking for the next region */
			/* on the *next* 64K boundary.  Otherwise, even if */
			/* the new region is free according to */
			/* 'VirtualQuery()', the subsequent call to */
			/* 'VirtualAlloc()' (which follows the call to */
			/* this routine in 'wsbrk()') will round *down* */
			/* the requested address to a 64K boundary which */
			/* we already know is an address in the */
			/* unavailable region.  Thus, the subsequent call */
			/* to 'VirtualAlloc()' will fail and bring us back */
			/* here, causing us to go into an infinite loop. */

			start_स्थानाङ्कः =
				(शून्य *) AlignPage64K((अचिह्नित दीर्घ) start_स्थानाङ्कः);
		}
	}
	प्रत्यागम रिक्त;

}

शून्य* wsbrk (दीर्घ आकार)
{
	शून्य* tmp;
	यदि (आकार > 0)
	{
		यदि (g_स्थानाङ्कः_आधारः == 0)
		{
			g_Allocated_आकार = max (RESERVED_आकार, AlignPage (आकार));
			g_अग्र_स्थानाङ्कः = g_स्थानाङ्कः_आधारः =
				(अचिह्नित पूर्णाङ्क)VirtualAlloc (रिक्त, g_Allocated_आकार,
											MEM_RESERVE, PAGE_NOACCESS);
		} अन्यथा यदि (AlignPage (g_अग्र_स्थानाङ्कः + आकार) > (g_स्थानाङ्कः_आधारः +
g_Allocated_आकार))
		{
			दीर्घ new_आकार = max (अग्र_आकार, AlignPage (आकार));
			शून्य* new_स्थानाङ्कः = (शून्य*)(g_स्थानाङ्कः_आधारः+g_Allocated_आकार);
			कुरु
			{
				new_स्थानाङ्कः = findRegion (new_स्थानाङ्कः, new_आकार);

				यदि (!new_स्थानाङ्कः)
					प्रत्यागम (शून्य*)-1;

				g_स्थानाङ्कः_आधारः = g_अग्र_स्थानाङ्कः =
					(अचिह्नित पूर्णाङ्क)VirtualAlloc (new_स्थानाङ्कः, new_आकार,
												MEM_RESERVE, PAGE_NOACCESS);
				/* repeat in case of race condition */
				/* The region that we found has been snagged */
				/* by another thread */
			}
			यावत् (g_स्थानाङ्कः_आधारः == 0);

			प्रतिज्ञा (new_स्थानाङ्कः == (शून्य*)g_स्थानाङ्कः_आधारः);

			g_Allocated_आकार = new_आकार;

			यदि (!makeGmListElement ((शून्य*)g_स्थानाङ्कः_आधारः))
				प्रत्यागम (शून्य*)-1;
		}
		यदि ((आकार + g_अग्र_स्थानाङ्कः) > AlignPage (g_अग्र_स्थानाङ्कः))
		{
			शून्य* res;
			res = VirtualAlloc ((शून्य*)AlignPage (g_अग्र_स्थानाङ्कः),
								(आकार + g_अग्र_स्थानाङ्कः -
								 AlignPage (g_अग्र_स्थानाङ्कः)),
								MEM_COMMIT, PAGE_READWRITE);
			यदि (!res)
				प्रत्यागम (शून्य*)-1;
		}
		tmp = (शून्य*)g_अग्र_स्थानाङ्कः;
		g_अग्र_स्थानाङ्कः = (अचिह्नित पूर्णाङ्क)tmp + आकार;
		प्रत्यागम tmp;
	}
	अन्यथा यदि (आकार < 0)
	{
		अचिह्नित पूर्णाङ्क alignedGoal = AlignPage (g_अग्र_स्थानाङ्कः + आकार);
		/* Trim by releasing the virtual memory */
		यदि (alignedGoal >= g_स्थानाङ्कः_आधारः)
		{
			VirtualFree ((शून्य*)alignedGoal, g_अग्र_स्थानाङ्कः - alignedGoal,
						 MEM_DECOMMIT);
			g_अग्र_स्थानाङ्कः = g_अग्र_स्थानाङ्कः + आकार;
			प्रत्यागम (शून्य*)g_अग्र_स्थानाङ्कः;
		}
		अन्यथा
		{
			VirtualFree ((शून्य*)g_स्थानाङ्कः_आधारः, g_अग्र_स्थानाङ्कः - g_स्थानाङ्कः_आधारः,
						 MEM_DECOMMIT);
			g_अग्र_स्थानाङ्कः = g_स्थानाङ्कः_आधारः;
			प्रत्यागम (शून्य*)-1;
		}
	}
	अन्यथा
	{
		प्रत्यागम (शून्य*)g_अग्र_स्थानाङ्कः;
	}
}

#यदि_अन्तः

/*
  Type declarations
*/

संरचना malloc_chunk
{
  आन्तरिकम्_आकार_T पूर्व_आकार; /* Size of previous chunk (if free). */
  आन्तरिकम्_आकार_T आकार;      /* Size in bytes, including overhead. */
  संरचना malloc_chunk* fd;   /* double links -- used only if free. */
  संरचना malloc_chunk* bk;
} __attribute__((__may_alias__)) ;

प्रकारनिर्देश संरचना malloc_chunk* mchunkptr;

/*

   malloc_chunk details:

    (The following includes lightly edited explanations by Colin Plumb.)

    Chunks of memory are maintained using a `boundary tag' method as
    described in e.g., Knuth or Standish.  (See the paper by Paul
    Wilson ftp://ftp.cs.utexas.edu/pub/garbage/allocsrv.ps for a
    survey of such techniques.)  Sizes of free chunks are stored both
    in the front of each chunk and at the end.  This makes
    consolidating fragmented chunks into bigger chunks very fast.  The
    size fields also hold bits representing whether chunks are free or
    in use.

    An allocated chunk looks like this:

    chunk-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	    |             Size of previous chunk, if allocated            | |
	    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	    |             Size of chunk, in bytes                         |P|
      mem-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	    |             User data starts here...                          .
	    .                                                               .
	    .             (malloc_usable_space() bytes)                     .
	    .                                                               |
nextchunk-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	    |             Size of chunk                                     |
	    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

    Where "chunk" is the front of the chunk for the purpose of most of
    the malloc code, but "mem" is the pointer that is returned to the
    user.  "Nextchunk" is the beginning of the next contiguous chunk.

    Chunks always begin on even word boundries, so the mem portion
    (which is returned to the user) is also on an even word boundary, and
    thus double-word aligned.

    Free chunks are stored in circular doubly-linked lists, and look like this:

    chunk-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	    |             Size of previous chunk                            |
	    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    `head:' |             Size of chunk, in bytes                         |P|
      mem-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	    |             Forward pointer to next chunk in list             |
	    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	    |             Back pointer to previous chunk in list            |
	    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	    |             Unused space (may be 0 bytes long)                .
	    .                                                               .
	    .                                                               |

nextchunk-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
    `foot:' |             Size of chunk, in bytes                           |
	    +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

    The P (PREV_INUSE) bit, stored in the unused low-order bit of the
    chunk size (which is always a multiple of two words), is an in-use
    bit for the *previous* chunk.  If that bit is *clear*, then the
    word before the current chunk size contains the previous chunk
    size, and can be used to find the front of the previous chunk.
    (The very first chunk allocated always has this bit set,
    preventing access to non-existent (or non-owned) memory.)

    Note that the `foot' of the current chunk is actually represented
    as the prev_size of the NEXT chunk. (This makes it easier to
    deal with alignments etc).

    The two exceptions to all this are

     1. The special chunk `top', which doesn't bother using the
	trailing size field since there is no
	next contiguous chunk that would have to index off it. (After
	initialization, `top' is forced to always exist.  If it would
	become less than MINSIZE bytes long, it is replenished via
	malloc_extend_top.)

     2. Chunks allocated via mmap, which have the second-lowest-order
	bit (IS_MMAPPED) set in their size fields.  Because they are
	never merged or traversed from any other chunk, they have no
	foot size or inuse information.

    Available chunks are kept in any of several places (all declared below):

    * `av': An array of chunks serving as bin headers for consolidated
       chunks. Each bin is doubly linked.  The bins are approximately
       proportionally (log) spaced.  There are a lot of these bins
       (128). This may look excessive, but works very well in
       practice.  All procedures maintain the invariant that no
       consolidated chunk physically borders another one. Chunks in
       bins are kept in size order, with ties going to the
       approximately least recently used chunk.

       The chunks in each bin are maintained in decreasing sorted order by
       size.  This is irrelevant for the small bins, which all contain
       the same-sized chunks, but facilitates best-fit allocation for
       larger chunks. (These lists are just sequential. Keeping them in
       order almost never requires enough traversal to warrant using
       fancier ordered data structures.)  Chunks of the same size are
       linked with the most recently freed at the front, and allocations
       are taken from the back.  This results in LRU or FIFO allocation
       order, which tends to give each chunk an equal opportunity to be
       consolidated with adjacent freed chunks, resulting in larger free
       chunks and less fragmentation.

    * `top': The top-most available chunk (i.e., the one bordering the
       end of available memory) is treated specially. It is never
       included in any bin, is used only if no other chunk is
       available, and is released back to the system if it is very
       large (see M_TRIM_THRESHOLD).

    * `last_remainder': A bin holding only the remainder of the
       most recently split (non-top) chunk. This bin is checked
       before other non-fitting chunks, so as to provide better
       locality for runs of sequentially allocated chunks.

    *  Implicitly, through the host system's memory mapping tables.
       If supported, requests greater than a threshold are usually
       serviced via calls to mmap, and then later released via munmap.

*/

/*  sizes, alignments */

#परिभाषय आकार_SZ                (परिमाण(आन्तरिकम्_आकार_T))
#परिभाषय MALLOC_ALIGNMENT       (आकार_SZ + आकार_SZ)
#परिभाषय MALLOC_ALIGN_आवरणम्      (MALLOC_ALIGNMENT - 1)
#परिभाषय MINSIZE                (परिमाण(संरचना malloc_chunk))

/* conversion from malloc headers to user pointers, and back */

#परिभाषय chunk2mem(p)   ((Void_t*)((वर्ण*)(p) + 2*आकार_SZ))
#परिभाषय mem2chunk(mem) ((mchunkptr)((वर्ण*)(mem) - 2*आकार_SZ))

/* pad request bytes into a usable size */

#परिभाषय request2size(req) \
 ((((req) + (आकार_SZ + MALLOC_ALIGN_आवरणम्)) < \
  (MINSIZE + MALLOC_ALIGN_आवरणम्)) ? MINSIZE : \
   (((req) + (आकार_SZ + MALLOC_ALIGN_आवरणम्)) & ~(MALLOC_ALIGN_आवरणम्)))

/* Check if m has acceptable alignment */

#परिभाषय aligned_OK(m)    (((अचिह्नित दीर्घ)((m)) & (MALLOC_ALIGN_आवरणम्)) == 0)

/*
  Physical chunk operations
*/

/* size field is or'ed with PREV_INUSE when previous adjacent chunk in use */

#परिभाषय पूर्व_INUSE 0x1

/* size field is or'ed with IS_MMAPPED if the chunk was obtained with mmap() */

#परिभाषय IS_MMAPPED 0x2

/* Bits to mask off when extracting size */

#परिभाषय आकार_द्वयङ्काः (पूर्व_INUSE|IS_MMAPPED)

/* Ptr to next physical malloc_chunk. */

#परिभाषय अग्र_chunk(p) ((mchunkptr)( ((वर्ण*)(p)) + ((p)->आकार & ~पूर्व_INUSE) ))

/* Ptr to previous physical malloc_chunk */

#परिभाषय पूर्व_chunk(p)\
   ((mchunkptr)( ((वर्ण*)(p)) - ((p)->पूर्व_आकार) ))

/* Treat space at ptr + offset as a chunk */

#परिभाषय chunk_at_विस्थापनम्(p, s)  ((mchunkptr)(((वर्ण*)(p)) + (s)))

/*
  Dealing with use bits
*/

/* extract p's inuse bit */

#परिभाषय inuse(p)\
((((mchunkptr)(((वर्ण*)(p))+((p)->आकार & ~पूर्व_INUSE)))->आकार) & पूर्व_INUSE)

/* extract inuse bit of previous chunk */

#परिभाषय पूर्व_inuse(p)  ((p)->आकार & पूर्व_INUSE)

/* check for mmap()'ed chunk */

#परिभाषय chunk_is_mmapped(p) ((p)->आकार & IS_MMAPPED)

/* set/clear chunk as in use without otherwise disturbing */

#परिभाषय समुच्चयः_inuse(p)\
((mchunkptr)(((वर्ण*)(p)) + ((p)->आकार & ~पूर्व_INUSE)))->आकार |= पूर्व_INUSE

#परिभाषय clear_inuse(p)\
((mchunkptr)(((वर्ण*)(p)) + ((p)->आकार & ~पूर्व_INUSE)))->आकार &= ~(पूर्व_INUSE)

/* check/set/clear inuse bits in known places */

#परिभाषय inuse_bit_at_विस्थापनम्(p, s)\
 (((mchunkptr)(((वर्ण*)(p)) + (s)))->आकार & पूर्व_INUSE)

#परिभाषय समुच्चयः_inuse_bit_at_विस्थापनम्(p, s)\
 (((mchunkptr)(((वर्ण*)(p)) + (s)))->आकार |= पूर्व_INUSE)

#परिभाषय clear_inuse_bit_at_विस्थापनम्(p, s)\
 (((mchunkptr)(((वर्ण*)(p)) + (s)))->आकार &= ~(पूर्व_INUSE))

/*
  Dealing with size fields
*/

/* Get size, ignoring use bits */

#परिभाषय chunksize(p)          ((p)->आकार & ~(आकार_द्वयङ्काः))

/* Set size at head, without disturbing its use bit */

#परिभाषय समुच्चयः_शीर्ष_आकार(p, s)   ((p)->आकार = (((p)->आकार & पूर्व_INUSE) | (s)))

/* Set size/use ignoring previous bits in header */

#परिभाषय समुच्चयः_शीर्ष(p, s)        ((p)->आकार = (s))

/* Set size at footer (only when chunk is not in use) */

#परिभाषय समुच्चयः_foot(p, s)   (((mchunkptr)((वर्ण*)(p) + (s)))->पूर्व_आकार = (s))

/*
   Bins

    The bins, `av_' are an array of pairs of pointers serving as the
    heads of (initially empty) doubly-linked lists of chunks, laid out
    in a way so that each pair can be treated as if it were in a
    malloc_chunk. (This way, the fd/bk offsets for linking bin heads
    and chunks are the same).

    Bins for sizes < 512 bytes contain chunks of all the same size, spaced
    8 bytes apart. Larger bins are approximately logarithmically
    spaced. (See the table below.) The `av_' array is never mentioned
    directly in the code, but instead via bin access macros.

    Bin layout:

    64 bins of size       8
    32 bins of size      64
    16 bins of size     512
     8 bins of size    4096
     4 bins of size   32768
     2 bins of size  262144
     1 bin  of size what's left

    There is actually a little bit of slop in the numbers in bin_index
    for the sake of speed. This makes no difference elsewhere.

    The special chunks `top' and `last_remainder' get their own bins,
    (this is implemented via yet more trickery with the av_ array),
    although `top' is never properly linked to its bin since it is
    always handled specially.

*/

#परिभाषय NAV             128   /* number of bins */

प्रकारनिर्देश संरचना malloc_chunk* mbinptr;

/* access macros */

#परिभाषय bin_at(i)      ((mbinptr)((वर्ण*)&(av_[2*(i) + 2]) - 2*आकार_SZ))
#परिभाषय अग्र_bin(b)    ((mbinptr)((वर्ण*)(b) + 2 * परिमाण(mbinptr)))
#परिभाषय पूर्व_bin(b)    ((mbinptr)((वर्ण*)(b) - 2 * परिमाण(mbinptr)))

/*
   The first 2 bins are never indexed. The corresponding av_ cells are instead
   used for bookkeeping. This is not to save space, but to simplify
   indexing, maintain locality, and avoid some initialization tests.
*/

#परिभाषय शिखरम्            (av_[2])          /* The topmost chunk */
#परिभाषय चरमः_remainder (bin_at(1))       /* remainder from last split */

/*
   Because top initially points to its own bin with initial
   zero size, thus forcing extension on the first malloc request,
   we avoid having any special code in malloc to check whether
   it even exists yet. But we still need to in malloc_extend_top.
*/

#परिभाषय initial_शिखरम्    ((mchunkptr)(bin_at(0)))

/* Helper macro to initialize bins */

#परिभाषय IAV(i)  bin_at(i), bin_at(i)

स्थिर mbinptr av_[NAV * 2 + 2] = {
 रिक्त, रिक्त,
 IAV(0),   IAV(1),   IAV(2),   IAV(3),   IAV(4),   IAV(5),   IAV(6),   IAV(7),
 IAV(8),   IAV(9),   IAV(10),  IAV(11),  IAV(12),  IAV(13),  IAV(14),  IAV(15),
 IAV(16),  IAV(17),  IAV(18),  IAV(19),  IAV(20),  IAV(21),  IAV(22),  IAV(23),
 IAV(24),  IAV(25),  IAV(26),  IAV(27),  IAV(28),  IAV(29),  IAV(30),  IAV(31),
 IAV(32),  IAV(33),  IAV(34),  IAV(35),  IAV(36),  IAV(37),  IAV(38),  IAV(39),
 IAV(40),  IAV(41),  IAV(42),  IAV(43),  IAV(44),  IAV(45),  IAV(46),  IAV(47),
 IAV(48),  IAV(49),  IAV(50),  IAV(51),  IAV(52),  IAV(53),  IAV(54),  IAV(55),
 IAV(56),  IAV(57),  IAV(58),  IAV(59),  IAV(60),  IAV(61),  IAV(62),  IAV(63),
 IAV(64),  IAV(65),  IAV(66),  IAV(67),  IAV(68),  IAV(69),  IAV(70),  IAV(71),
 IAV(72),  IAV(73),  IAV(74),  IAV(75),  IAV(76),  IAV(77),  IAV(78),  IAV(79),
 IAV(80),  IAV(81),  IAV(82),  IAV(83),  IAV(84),  IAV(85),  IAV(86),  IAV(87),
 IAV(88),  IAV(89),  IAV(90),  IAV(91),  IAV(92),  IAV(93),  IAV(94),  IAV(95),
 IAV(96),  IAV(97),  IAV(98),  IAV(99),  IAV(100), IAV(101), IAV(102), IAV(103),
 IAV(104), IAV(105), IAV(106), IAV(107), IAV(108), IAV(109), IAV(110), IAV(111),
 IAV(112), IAV(113), IAV(114), IAV(115), IAV(116), IAV(117), IAV(118), IAV(119),
 IAV(120), IAV(121), IAV(122), IAV(123), IAV(124), IAV(125), IAV(126), IAV(127)
};

#यदि_परिभाषितम् CONFIG_SYS_MALLOC_DEFAULT_गन्तव्यम्_INIT
स्थिर शून्य malloc_init(शून्य);
#यदि_अन्तः

ulong mem_malloc_start = 0;
ulong mem_malloc_अन्तः = 0;
ulong mem_malloc_brk = 0;

स्थिर bool malloc_testing;	/* enable test mode */
स्थिर पूर्णाङ्क malloc_max_allocs;	/* return NULL after this many calls to malloc() */

शून्य *sbrk(ptrdiff_t increment)
{
	ulong old = mem_malloc_brk;
	ulong new = old + increment;

	यदि ((new < mem_malloc_start) || (new > mem_malloc_अन्तः))
		प्रत्यागम (शून्य *)MORECORE_FAILURE;

	/*
	 * if we are giving memory back make sure we clear it out since
	 * we set MORECORE_CLEARS to 1
	 */
	यदि (increment < 0)
		स्मृतिं_पूरय((शून्य *)new, 0, -increment);

	mem_malloc_brk = new;

	प्रत्यागम (शून्य *)old;
}

शून्य mem_malloc_init(ulong start, ulong आकार)
{
	mem_malloc_start = (ulong)map_sysmem(start, आकार);
	mem_malloc_अन्तः = mem_malloc_start + आकार;
	mem_malloc_brk = mem_malloc_start;

#यदि_परिभाषितम् CONFIG_SYS_MALLOC_DEFAULT_गन्तव्यम्_INIT
	malloc_init();
#यदि_अन्तः

	debug("using memory %#lx-%#lx for malloc()\n", mem_malloc_start,
	      mem_malloc_अन्तः);
#यदि CONFIG_IS_ENABLED(SYS_MALLOC_CLEAR_ON_INIT)
	स्मृतिं_पूरय((शून्य *)mem_malloc_start, 0x0, आकार);
#यदि_अन्तः
}

/* field-extraction macros */

#परिभाषय first(b) ((b)->fd)
#परिभाषय चरमः(b)  ((b)->bk)

/*
  Indexing into bins
*/

#परिभाषय bin_सूचकांक(sz)                                                          \
(((((अचिह्नित दीर्घ)(sz)) >> 9) ==    0) ?       (((अचिह्नित दीर्घ)(sz)) >>  3): \
 ((((अचिह्नित दीर्घ)(sz)) >> 9) <=    4) ?  56 + (((अचिह्नित दीर्घ)(sz)) >>  6): \
 ((((अचिह्नित दीर्घ)(sz)) >> 9) <=   20) ?  91 + (((अचिह्नित दीर्घ)(sz)) >>  9): \
 ((((अचिह्नित दीर्घ)(sz)) >> 9) <=   84) ? 110 + (((अचिह्नित दीर्घ)(sz)) >> 12): \
 ((((अचिह्नित दीर्घ)(sz)) >> 9) <=  340) ? 119 + (((अचिह्नित दीर्घ)(sz)) >> 15): \
 ((((अचिह्नित दीर्घ)(sz)) >> 9) <= 1364) ? 124 + (((अचिह्नित दीर्घ)(sz)) >> 18): \
					  126)
/*
  bins for chunks < 512 are all spaced 8 bytes apart, and hold
  identically sized chunks. This is exploited in malloc.
*/

#परिभाषय MAX_SMALLBIN         63
#परिभाषय MAX_SMALLBIN_आकार   512
#परिभाषय SMALLBIN_विस्तारः        8

#परिभाषय smallbin_सूचकांक(sz)  (((अचिह्नित दीर्घ)(sz)) >> 3)

/*
   Requests are `small' if both the corresponding and the next bin are small
*/

#परिभाषय is_small_request(nb) (nb < MAX_SMALLBIN_आकार - SMALLBIN_विस्तारः)

/*
    To help compensate for the large number of bins, a one-level index
    structure is used for bin-by-bin searching.  `binblocks' is a
    one-word bitvector recording whether groups of BINBLOCKWIDTH bins
    have any (possibly) non-empty bins, so they can be skipped over
    all at once during during traversals. The bits are NOT always
    cleared as soon as all bins in a block are empty, but instead only
    when all are noticed to be empty during traversal in malloc.
*/

#परिभाषय BINBLOCKWIDTH     4   /* bins per block */

#परिभाषय binblocks_r     ((आन्तरिकम्_आकार_T)av_[1]) /* bitvector of nonempty blocks */
#परिभाषय binblocks_w     (av_[1])

/* bin<->block macros */

#परिभाषय idx2binblock(ix)    ((अचिह्नित)1 << (ix / BINBLOCKWIDTH))
#परिभाषय mark_binblock(ii)   (binblocks_w = (mbinptr)(binblocks_r | idx2binblock(ii)))
#परिभाषय clear_binblock(ii)  (binblocks_w = (mbinptr)(binblocks_r & ~(idx2binblock(ii))))

/*  Other static bookkeeping data */

/* variables holding tunable values */

स्थिर अचिह्नित दीर्घ trim_देहली   = DEFAULT_TRIM_देहली;
स्थिर अचिह्नित दीर्घ शिखरम्_pad          = DEFAULT_शिखरम्_PAD;
स्थिर अचिह्नित पूर्णाङ्क  n_mmaps_max      = DEFAULT_MMAP_MAX;
स्थिर अचिह्नित दीर्घ mmap_देहली   = DEFAULT_MMAP_देहली;

/* The first value returned from sbrk */
स्थिर वर्ण* sbrk_आधारः = (वर्ण*)(-1);

/* The maximum memory obtained from system via sbrk */
स्थिर अचिह्नित दीर्घ max_sbrked_mem = 0;

/* The maximum via either sbrk or mmap */
स्थिर अचिह्नित दीर्घ max_योग_mem = 0;

/* internal working copy of mallinfo */
स्थिर संरचना mallinfo current_mallinfo = {  0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

/* The total memory obtained from system via sbrk */
#परिभाषय sbrked_mem  (current_mallinfo.arena)

/* Tracking mmaps */

#यदि_परिभाषितम् DEBUG
स्थिर अचिह्नित पूर्णाङ्क n_mmaps = 0;
#यदि_अन्तः	/* DEBUG */
स्थिर अचिह्नित दीर्घ mmapped_mem = 0;
#यदि HAVE_MMAP
स्थिर अचिह्नित पूर्णाङ्क max_n_mmaps = 0;
स्थिर अचिह्नित दीर्घ max_mmapped_mem = 0;
#यदि_अन्तः

#यदि_परिभाषितम् CONFIG_SYS_MALLOC_DEFAULT_गन्तव्यम्_INIT
स्थिर शून्य malloc_init(शून्य)
{
	पूर्णाङ्क i, j;

	debug("bins (av_ array) are at %p\n", (शून्य *)av_);

	av_[0] = रिक्त; av_[1] = रिक्त;
	प्रति (i = 2, j = 2; i < NAV * 2 + 2; i += 2, j++) {
		av_[i] = bin_at(j - 2);
		av_[i + 1] = bin_at(j - 2);

		/* Just print the first few bins so that
		 * we can see there are alright.
		 */
		यदि (i < 10)
			debug("av_[%d]=%lx av_[%d]=%lx\n",
			      i, (ulong)av_[i],
			      i + 1, (ulong)av_[i + 1]);
	}

	/* Init the static bookkeeping as well */
	sbrk_आधारः = (वर्ण *)(-1);
	max_sbrked_mem = 0;
	max_योग_mem = 0;
#यदि_परिभाषितम् DEBUG
	स्मृतिं_पूरय((शून्य *)&current_mallinfo, 0, परिमाण(संरचना mallinfo));
#यदि_अन्तः
}
#यदि_अन्तः

/*
  Debugging support
*/

#यदि_परिभाषितम् DEBUG

/*
  These routines make a number of assertions about the states
  of data structures that should be true at all times. If any
  are not true, it's very likely that a user program has somehow
  trashed memory. (It's also possible that there is a coding error
  in malloc. In which case, please report it!)
*/

#यदि __STD_C
स्थिर शून्य do_check_chunk(mchunkptr p)
#अन्यथा
स्थिर शून्य do_check_chunk(p) mchunkptr p;
#यदि_अन्तः
{
  आन्तरिकम्_आकार_T sz = p->आकार & ~पूर्व_INUSE;

  /* No checkable chunk is mmapped */
  प्रतिज्ञा(!chunk_is_mmapped(p));

  /* Check for legal address ... */
  प्रतिज्ञा((वर्ण*)p >= sbrk_आधारः);
  यदि (p != शिखरम्)
    प्रतिज्ञा((वर्ण*)p + sz <= (वर्ण*)शिखरम्);
  अन्यथा
    प्रतिज्ञा((वर्ण*)p + sz <= sbrk_आधारः + sbrked_mem);

}

#यदि __STD_C
स्थिर शून्य do_check_free_chunk(mchunkptr p)
#अन्यथा
स्थिर शून्य do_check_free_chunk(p) mchunkptr p;
#यदि_अन्तः
{
  आन्तरिकम्_आकार_T sz = p->आकार & ~पूर्व_INUSE;
  mchunkptr अग्र = chunk_at_विस्थापनम्(p, sz);

  do_check_chunk(p);

  /* Check whether it claims to be free ... */
  प्रतिज्ञा(!inuse(p));

  /* Unless a special marker, must have OK fields */
  यदि ((दीर्घ)sz >= (दीर्घ)MINSIZE)
  {
    प्रतिज्ञा((sz & MALLOC_ALIGN_आवरणम्) == 0);
    प्रतिज्ञा(aligned_OK(chunk2mem(p)));
    /* ... matching footer field */
    प्रतिज्ञा(अग्र->पूर्व_आकार == sz);
    /* ... and is fully consolidated */
    प्रतिज्ञा(पूर्व_inuse(p));
    प्रतिज्ञा (अग्र == शिखरम् || inuse(अग्र));

    /* ... and has minimally sane links */
    प्रतिज्ञा(p->fd->bk == p);
    प्रतिज्ञा(p->bk->fd == p);
  }
  अन्यथा /* markers are always of size SIZE_SZ */
    प्रतिज्ञा(sz == आकार_SZ);
}

#यदि __STD_C
स्थिर शून्य do_check_inuse_chunk(mchunkptr p)
#अन्यथा
स्थिर शून्य do_check_inuse_chunk(p) mchunkptr p;
#यदि_अन्तः
{
  mchunkptr अग्र = अग्र_chunk(p);
  do_check_chunk(p);

  /* Check whether it claims to be in use ... */
  प्रतिज्ञा(inuse(p));

  /* ... and is surrounded by OK chunks.
    Since more things can be checked with free chunks than inuse ones,
    if an inuse chunk borders them and debug is on, it's worth doing them.
  */
  यदि (!पूर्व_inuse(p))
  {
    mchunkptr prv = पूर्व_chunk(p);
    प्रतिज्ञा(अग्र_chunk(prv) == p);
    do_check_free_chunk(prv);
  }
  यदि (अग्र == शिखरम्)
  {
    प्रतिज्ञा(पूर्व_inuse(अग्र));
    प्रतिज्ञा(chunksize(अग्र) >= MINSIZE);
  }
  अन्यथा यदि (!inuse(अग्र))
    do_check_free_chunk(अग्र);

}

#यदि __STD_C
स्थिर शून्य do_check_malloced_chunk(mchunkptr p, आन्तरिकम्_आकार_T s)
#अन्यथा
स्थिर शून्य do_check_malloced_chunk(p, s) mchunkptr p; आन्तरिकम्_आकार_T s;
#यदि_अन्तः
{
  आन्तरिकम्_आकार_T sz = p->आकार & ~पूर्व_INUSE;
  दीर्घ अवकाशः = sz - s;

  do_check_inuse_chunk(p);

  /* Legal size ... */
  प्रतिज्ञा((दीर्घ)sz >= (दीर्घ)MINSIZE);
  प्रतिज्ञा((sz & MALLOC_ALIGN_आवरणम्) == 0);
  प्रतिज्ञा(अवकाशः >= 0);
  प्रतिज्ञा(अवकाशः < (दीर्घ)MINSIZE);

  /* ... and alignment */
  प्रतिज्ञा(aligned_OK(chunk2mem(p)));

  /* ... and was allocated at front of an available chunk */
  प्रतिज्ञा(पूर्व_inuse(p));

}

#परिभाषय check_free_chunk(P)  do_check_free_chunk(P)
#परिभाषय check_inuse_chunk(P) do_check_inuse_chunk(P)
#परिभाषय check_chunk(P) do_check_chunk(P)
#परिभाषय check_malloced_chunk(P,N) do_check_malloced_chunk(P,N)
#अन्यथा
#परिभाषय check_free_chunk(P)
#परिभाषय check_inuse_chunk(P)
#परिभाषय check_chunk(P)
#परिभाषय check_malloced_chunk(P,N)
#यदि_अन्तः

/*
  Macro-based internal utilities
*/

/*
  Linking chunks in bin lists.
  Call these only with variables, not arbitrary expressions, as arguments.
*/

/*
  Place chunk p of size s in its bin, in size order,
  putting it ahead of others of same size.
*/

#परिभाषय frontlink(P, S, IDX, BK, FD)                                          \
{                                                                             \
  यदि (S < MAX_SMALLBIN_आकार)                                                  \
  {                                                                           \
    IDX = smallbin_सूचकांक(S);                                                  \
    mark_binblock(IDX);                                                       \
    BK = bin_at(IDX);                                                         \
    FD = BK->fd;                                                              \
    P->bk = BK;                                                               \
    P->fd = FD;                                                               \
    FD->bk = BK->fd = P;                                                      \
  }                                                                           \
  अन्यथा                                                                        \
  {                                                                           \
    IDX = bin_सूचकांक(S);                                                       \
    BK = bin_at(IDX);                                                         \
    FD = BK->fd;                                                              \
    यदि (FD == BK) mark_binblock(IDX);                                         \
    अन्यथा                                                                      \
    {                                                                         \
      यावत् (FD != BK && S < chunksize(FD)) FD = FD->fd;                      \
      BK = FD->bk;                                                            \
    }                                                                         \
    P->bk = BK;                                                               \
    P->fd = FD;                                                               \
    FD->bk = BK->fd = P;                                                      \
  }                                                                           \
}

/* take a chunk off a list */

#परिभाषय unlink(P, BK, FD)                                                     \
{                                                                             \
  BK = P->bk;                                                                 \
  FD = P->fd;                                                                 \
  FD->bk = BK;                                                                \
  BK->fd = FD;                                                                \
}                                                                             \

/* Place p as the last remainder */

#परिभाषय link_चरमः_remainder(P)                                                \
{                                                                             \
  चरमः_remainder->fd = चरमः_remainder->bk =  P;                               \
  P->fd = P->bk = चरमः_remainder;                                             \
}

/* Clear the last_remainder bin */

#परिभाषय clear_चरमः_remainder \
  (चरमः_remainder->fd = चरमः_remainder->bk = चरमः_remainder)

/* Routines dealing with mmap(). */

#यदि HAVE_MMAP

#यदि __STD_C
स्थिर mchunkptr mmap_chunk(size_t आकार)
#अन्यथा
स्थिर mchunkptr mmap_chunk(आकार) size_t आकार;
#यदि_अन्तः
{
  size_t page_आवरणम् = malloc_getpagesize - 1;
  mchunkptr p;

#यदि_अपरिभाषितम् MAP_ANONYMOUS
  स्थिर पूर्णाङ्क fd = -1;
#यदि_अन्तः

  यदि(n_mmaps >= n_mmaps_max) प्रत्यागम 0; /* too many regions */

  /* For mmapped chunks, the overhead is one SIZE_SZ unit larger, because
   * there is no following chunk whose prev_size field could be used.
   */
  आकार = (आकार + आकार_SZ + page_आवरणम्) & ~page_आवरणम्;

#यदि_परिभाषितम् MAP_ANONYMOUS
  p = (mchunkptr)mmap(0, आकार, PROT_READ|PROT_WRITE,
		      MAP_वैयक्तिकम्|MAP_ANONYMOUS, -1, 0);
#अन्यथा /* !MAP_ANONYMOUS */
  यदि (fd < 0)
  {
    fd = उद्घाटय("/dev/zero", O_RDWR);
    यदि(fd < 0) प्रत्यागम 0;
  }
  p = (mchunkptr)mmap(0, आकार, PROT_READ|PROT_WRITE, MAP_वैयक्तिकम्, fd, 0);
#यदि_अन्तः

  यदि(p == (mchunkptr)-1) प्रत्यागम 0;

  n_mmaps++;
  यदि (n_mmaps > max_n_mmaps) max_n_mmaps = n_mmaps;

  /* We demand that eight bytes into a page must be 8-byte aligned. */
  प्रतिज्ञा(aligned_OK(chunk2mem(p)));

  /* The offset to the start of the mmapped region is stored
   * in the prev_size field of the chunk; normally it is zero,
   * but that can be changed in memalign().
   */
  p->पूर्व_आकार = 0;
  समुच्चयः_शीर्ष(p, आकार|IS_MMAPPED);

  mmapped_mem += आकार;
  यदि ((अचिह्नित दीर्घ)mmapped_mem > (अचिह्नित दीर्घ)max_mmapped_mem)
    max_mmapped_mem = mmapped_mem;
  यदि ((अचिह्नित दीर्घ)(mmapped_mem + sbrked_mem) > (अचिह्नित दीर्घ)max_योग_mem)
    max_योग_mem = mmapped_mem + sbrked_mem;
  प्रत्यागम p;
}

#यदि __STD_C
स्थिर शून्य munmap_chunk(mchunkptr p)
#अन्यथा
स्थिर शून्य munmap_chunk(p) mchunkptr p;
#यदि_अन्तः
{
  आन्तरिकम्_आकार_T आकार = chunksize(p);
  पूर्णाङ्क ret;

  प्रतिज्ञा (chunk_is_mmapped(p));
  प्रतिज्ञा(! ((वर्ण*)p >= sbrk_आधारः && (वर्ण*)p < sbrk_आधारः + sbrked_mem));
  प्रतिज्ञा((n_mmaps > 0));
  प्रतिज्ञा(((p->पूर्व_आकार + आकार) & (malloc_getpagesize-1)) == 0);

  n_mmaps--;
  mmapped_mem -= (आकार + p->पूर्व_आकार);

  ret = munmap((वर्ण *)p - p->पूर्व_आकार, आकार + p->पूर्व_आकार);

  /* munmap returns non-zero on failure */
  प्रतिज्ञा(ret == 0);
}

#यदि HAVE_MREMAP

#यदि __STD_C
स्थिर mchunkptr mremap_chunk(mchunkptr p, size_t new_आकार)
#अन्यथा
स्थिर mchunkptr mremap_chunk(p, new_आकार) mchunkptr p; size_t new_आकार;
#यदि_अन्तः
{
  size_t page_आवरणम् = malloc_getpagesize - 1;
  आन्तरिकम्_आकार_T विस्थापनम् = p->पूर्व_आकार;
  आन्तरिकम्_आकार_T आकार = chunksize(p);
  वर्ण *cp;

  प्रतिज्ञा (chunk_is_mmapped(p));
  प्रतिज्ञा(! ((वर्ण*)p >= sbrk_आधारः && (वर्ण*)p < sbrk_आधारः + sbrked_mem));
  प्रतिज्ञा((n_mmaps > 0));
  प्रतिज्ञा(((आकार + विस्थापनम्) & (malloc_getpagesize-1)) == 0);

  /* Note the extra SIZE_SZ overhead as in mmap_chunk(). */
  new_आकार = (new_आकार + विस्थापनम् + आकार_SZ + page_आवरणम्) & ~page_आवरणम्;

  cp = (वर्ण *)mremap((वर्ण *)p - विस्थापनम्, आकार + विस्थापनम्, new_आकार, 1);

  यदि (cp == (वर्ण *)-1) प्रत्यागम 0;

  p = (mchunkptr)(cp + विस्थापनम्);

  प्रतिज्ञा(aligned_OK(chunk2mem(p)));

  प्रतिज्ञा((p->पूर्व_आकार == विस्थापनम्));
  समुच्चयः_शीर्ष(p, (new_आकार - विस्थापनम्)|IS_MMAPPED);

  mmapped_mem -= आकार + विस्थापनम्;
  mmapped_mem += new_आकार;
  यदि ((अचिह्नित दीर्घ)mmapped_mem > (अचिह्नित दीर्घ)max_mmapped_mem)
    max_mmapped_mem = mmapped_mem;
  यदि ((अचिह्नित दीर्घ)(mmapped_mem + sbrked_mem) > (अचिह्नित दीर्घ)max_योग_mem)
    max_योग_mem = mmapped_mem + sbrked_mem;
  प्रत्यागम p;
}

#यदि_अन्तः /* HAVE_MREMAP */

#यदि_अन्तः /* HAVE_MMAP */

/*
  Extend the top-most chunk by obtaining memory from system.
  Main interface to sbrk (but see also malloc_trim).
*/

#यदि __STD_C
स्थिर शून्य malloc_extend_शिखरम्(आन्तरिकम्_आकार_T nb)
#अन्यथा
स्थिर शून्य malloc_extend_शिखरम्(nb) आन्तरिकम्_आकार_T nb;
#यदि_अन्तः
{
  वर्ण*     brk;                  /* return value from sbrk */
  आन्तरिकम्_आकार_T front_misalign; /* unusable bytes at front of sbrked space */
  आन्तरिकम्_आकार_T correction;     /* bytes for 2nd sbrk call */
  वर्ण*     new_brk;              /* return of 2nd sbrk call */
  आन्तरिकम्_आकार_T शिखरम्_आकार;       /* new size of top chunk */

  mchunkptr old_शिखरम्     = शिखरम्;  /* Record state of old top */
  आन्तरिकम्_आकार_T old_शिखरम्_आकार = chunksize(old_शिखरम्);
  वर्ण*     old_अन्तः      = (वर्ण*)(chunk_at_विस्थापनम्(old_शिखरम्, old_शिखरम्_आकार));

  /* Pad request with top_pad plus minimal overhead */

  आन्तरिकम्_आकार_T    sbrk_आकार     = nb + शिखरम्_pad + MINSIZE;
  अचिह्नित दीर्घ pagesz    = malloc_getpagesize;

  /* If not the first time through, round to preserve page boundary */
  /* Otherwise, we need to correct to a page size below anyway. */
  /* (We also correct below if an intervening foreign sbrk call.) */

  यदि (sbrk_आधारः != (वर्ण*)(-1))
    sbrk_आकार = (sbrk_आकार + (pagesz - 1)) & ~(pagesz - 1);

  brk = (वर्ण*)(MORECORE (sbrk_आकार));

  /* Fail if sbrk failed or if a foreign sbrk call killed our space */
  यदि (brk == (वर्ण*)(MORECORE_FAILURE) ||
      (brk < old_अन्तः && old_शिखरम् != initial_शिखरम्))
    प्रत्यागम;

  sbrked_mem += sbrk_आकार;

  यदि (brk == old_अन्तः) /* can just add bytes to current top */
  {
    शिखरम्_आकार = sbrk_आकार + old_शिखरम्_आकार;
    समुच्चयः_शीर्ष(शिखरम्, शिखरम्_आकार | पूर्व_INUSE);
  }
  अन्यथा
  {
    यदि (sbrk_आधारः == (वर्ण*)(-1))  /* First time through. Record base */
      sbrk_आधारः = brk;
    अन्यथा  /* Someone else called sbrk().  Count those bytes as sbrked_mem. */
      sbrked_mem += brk - (वर्ण*)old_अन्तः;

    /* Guarantee alignment of first new chunk made from this space */
    front_misalign = (अचिह्नित दीर्घ)chunk2mem(brk) & MALLOC_ALIGN_आवरणम्;
    यदि (front_misalign > 0)
    {
      correction = (MALLOC_ALIGNMENT) - front_misalign;
      brk += correction;
    }
    अन्यथा
      correction = 0;

    /* Guarantee the next brk will be at a page boundary */

    correction += ((((अचिह्नित दीर्घ)(brk + sbrk_आकार))+(pagesz-1)) &
		   ~(pagesz - 1)) - ((अचिह्नित दीर्घ)(brk + sbrk_आकार));

    /* Allocate correction */
    new_brk = (वर्ण*)(MORECORE (correction));
    यदि (new_brk == (वर्ण*)(MORECORE_FAILURE)) प्रत्यागम;

    sbrked_mem += correction;

    शिखरम् = (mchunkptr)brk;
    शिखरम्_आकार = new_brk - brk + correction;
    समुच्चयः_शीर्ष(शिखरम्, शिखरम्_आकार | पूर्व_INUSE);

    यदि (old_शिखरम् != initial_शिखरम्)
    {

      /* There must have been an intervening foreign sbrk call. */
      /* A double fencepost is necessary to prevent consolidation */

      /* If not enough space to do this, then user did something very wrong */
      यदि (old_शिखरम्_आकार < MINSIZE)
      {
	समुच्चयः_शीर्ष(शिखरम्, पूर्व_INUSE); /* will force null return from malloc */
	प्रत्यागम;
      }

      /* Also keep size a multiple of MALLOC_ALIGNMENT */
      old_शिखरम्_आकार = (old_शिखरम्_आकार - 3*आकार_SZ) & ~MALLOC_ALIGN_आवरणम्;
      समुच्चयः_शीर्ष_आकार(old_शिखरम्, old_शिखरम्_आकार);
      chunk_at_विस्थापनम्(old_शिखरम्, old_शिखरम्_आकार          )->आकार =
	आकार_SZ|पूर्व_INUSE;
      chunk_at_विस्थापनम्(old_शिखरम्, old_शिखरम्_आकार + आकार_SZ)->आकार =
	आकार_SZ|पूर्व_INUSE;
      /* If possible, release the rest. */
      यदि (old_शिखरम्_आकार >= MINSIZE)
	fREe(chunk2mem(old_शिखरम्));
    }
  }

  यदि ((अचिह्नित दीर्घ)sbrked_mem > (अचिह्नित दीर्घ)max_sbrked_mem)
    max_sbrked_mem = sbrked_mem;
  यदि ((अचिह्नित दीर्घ)(mmapped_mem + sbrked_mem) > (अचिह्नित दीर्घ)max_योग_mem)
    max_योग_mem = mmapped_mem + sbrked_mem;

  /* We always land on a page boundary */
  प्रतिज्ञा(((अचिह्नित दीर्घ)((वर्ण*)शिखरम् + शिखरम्_आकार) & (pagesz - 1)) == 0);
}

/* Main public routines */

/*
  Malloc Algorthim:

    The requested size is first converted into a usable form, `nb'.
    This currently means to add 4 bytes overhead plus possibly more to
    obtain 8-byte alignment and/or to obtain a size of at least
    MINSIZE (currently 16 bytes), the smallest allocatable size.
    (All fits are considered `exact' if they are within MINSIZE bytes.)

    From there, the first successful of the following steps is taken:

      1. The bin corresponding to the request size is scanned, and if
	 a chunk of exactly the right size is found, it is taken.

      2. The most recently remaindered chunk is used if it is big
	 enough.  This is a form of (roving) first fit, used only in
	 the absence of exact fits. Runs of consecutive requests use
	 the remainder of the chunk used for the previous such request
	 whenever possible. This limited use of a first-fit style
	 allocation strategy tends to give contiguous chunks
	 coextensive lifetimes, which improves locality and can reduce
	 fragmentation in the long run.

      3. Other bins are scanned in increasing size order, using a
	 chunk big enough to fulfill the request, and splitting off
	 any remainder.  This search is strictly by best-fit; i.e.,
	 the smallest (with ties going to approximately the least
	 recently used) chunk that fits is selected.

      4. If large enough, the chunk bordering the end of memory
	 (`top') is split off. (This use of `top' is in accord with
	 the best-fit search rule.  In effect, `top' is treated as
	 larger (and thus less well fitting) than any other available
	 chunk since it can be extended to be as large as necessary
	 (up to system limitations).

      5. If the request size meets the mmap threshold and the
	 system supports mmap, and there are few enough currently
	 allocated mmapped regions, and a call to mmap succeeds,
	 the request is allocated via direct memory mapping.

      6. Otherwise, the top of memory is extended by
	 obtaining more space from the system (normally using sbrk,
	 but definable to anything else via the MORECORE macro).
	 Memory is gathered from the system (in system page-sized
	 units) in a way that allows chunks obtained across different
	 sbrk calls to be consolidated, but does not require
	 contiguous memory. Thus, it should be safe to intersperse
	 mallocs with other sbrk calls.

      All allocations are made from the the `lowest' part of any found
      chunk. (The implementation invariant is that prev_inuse is
      always true of any allocated chunk; i.e., that each allocated
      chunk borders either a previously allocated and still in-use chunk,
      or the base of its memory arena.)

*/

STATIC_IF_MCHECK
#यदि __STD_C
Void_t* mALLOc_impl(size_t अष्टकानि)
#अन्यथा
Void_t* mALLOc_impl(अष्टकानि) size_t अष्टकानि;
#यदि_अन्तः
{
  mchunkptr victim;                  /* inspected/selected chunk */
  आन्तरिकम्_आकार_T victim_आकार;       /* its size */
  पूर्णाङ्क       idx;                     /* index for bin traversal */
  mbinptr   bin;                     /* associated bin */
  mchunkptr remainder;               /* remainder from a split */
  दीर्घ      remainder_आकार;          /* its size */
  पूर्णाङ्क       remainder_सूचकांक;         /* its bin index */
  अचिह्नित दीर्घ खण्डम्;               /* block traverser bit */
  पूर्णाङ्क       startidx;                /* first bin of a traversed block */
  mchunkptr fwd;                     /* misc temp for linking */
  mchunkptr bck;                     /* misc temp for linking */
  mbinptr q;                         /* misc temp */

  आन्तरिकम्_आकार_T nb;

#यदि CONFIG_IS_ENABLED(SYS_MALLOC_F)
	यदि (!(gd->flags & GD_FLG_पूर्णम्_MALLOC_INIT))
		प्रत्यागम malloc_simple(अष्टकानि);
#यदि_अन्तः

  यदि (CONFIG_IS_ENABLED(UNIT_TEST) && malloc_testing) {
    यदि (--malloc_max_allocs < 0)
      प्रत्यागम रिक्त;
  }

  /* check if mem_malloc_init() was run */
  यदि ((mem_malloc_start == 0) && (mem_malloc_अन्तः == 0)) {
    /* not initialized yet */
    प्रत्यागम रिक्त;
  }

  यदि (अष्टकानि > CONFIG_SYS_MALLOC_LEN || (दीर्घ)अष्टकानि < 0)
     प्रत्यागम रिक्त;

  nb = request2size(अष्टकानि);  /* padded request size; */

  /* Check for exact match in a bin */

  यदि (is_small_request(nb))  /* Faster version for small requests */
  {
    idx = smallbin_सूचकांक(nb);

    /* No traversal or size check necessary for small bins.  */

    q = bin_at(idx);
    victim = चरमः(q);

    /* Also scan the next one, since it would have a remainder < MINSIZE */
    यदि (victim == q)
    {
      q = अग्र_bin(q);
      victim = चरमः(q);
    }
    यदि (victim != q)
    {
      victim_आकार = chunksize(victim);
      unlink(victim, bck, fwd);
      समुच्चयः_inuse_bit_at_विस्थापनम्(victim, victim_आकार);
      check_malloced_chunk(victim, nb);
      VALGRIND_MALLOCLIKE_खण्डम्(chunk2mem(victim), अष्टकानि, आकार_SZ, false);
      प्रत्यागम chunk2mem(victim);
    }

    idx += 2; /* Set for bin scan below. We've already scanned 2 bins. */

  }
  अन्यथा
  {
    idx = bin_सूचकांक(nb);
    bin = bin_at(idx);

    प्रति (victim = चरमः(bin); victim != bin; victim = victim->bk)
    {
      victim_आकार = chunksize(victim);
      remainder_आकार = victim_आकार - nb;

      यदि (remainder_आकार >= (दीर्घ)MINSIZE) /* too big */
      {
	--idx; /* adjust to rescan below after checking last remainder */
	भङ्ग;
      }

      अन्यथा यदि (remainder_आकार >= 0) /* exact fit */
      {
	unlink(victim, bck, fwd);
	समुच्चयः_inuse_bit_at_विस्थापनम्(victim, victim_आकार);
	check_malloced_chunk(victim, nb);
        VALGRIND_MALLOCLIKE_खण्डम्(chunk2mem(victim), अष्टकानि, आकार_SZ, false);
	प्रत्यागम chunk2mem(victim);
      }
    }

    ++idx;

  }

  /* Try to use the last split-off remainder */

  यदि ( (victim = चरमः_remainder->fd) != चरमः_remainder)
  {
    victim_आकार = chunksize(victim);
    remainder_आकार = victim_आकार - nb;

    यदि (remainder_आकार >= (दीर्घ)MINSIZE) /* re-split */
    {
      remainder = chunk_at_विस्थापनम्(victim, nb);
      समुच्चयः_शीर्ष(victim, nb | पूर्व_INUSE);
      link_चरमः_remainder(remainder);
      समुच्चयः_शीर्ष(remainder, remainder_आकार | पूर्व_INUSE);
      समुच्चयः_foot(remainder, remainder_आकार);
      check_malloced_chunk(victim, nb);
      VALGRIND_MALLOCLIKE_खण्डम्(chunk2mem(victim), अष्टकानि, आकार_SZ, false);
      प्रत्यागम chunk2mem(victim);
    }

    clear_चरमः_remainder;

    यदि (remainder_आकार >= 0)  /* exhaust */
    {
      समुच्चयः_inuse_bit_at_विस्थापनम्(victim, victim_आकार);
      check_malloced_chunk(victim, nb);
      VALGRIND_MALLOCLIKE_खण्डम्(chunk2mem(victim), अष्टकानि, आकार_SZ, false);
      प्रत्यागम chunk2mem(victim);
    }

    /* Else place in bin */

    frontlink(victim, victim_आकार, remainder_सूचकांक, bck, fwd);
  }

  /*
     If there are any possibly nonempty big-enough blocks,
     search for best fitting chunk by scanning bins in blockwidth units.
  */

  यदि ( (खण्डम् = idx2binblock(idx)) <= binblocks_r)
  {

    /* Get to the first marked block */

    यदि ( (खण्डम् & binblocks_r) == 0)
    {
      /* force to an even block boundary */
      idx = (idx & ~(BINBLOCKWIDTH - 1)) + BINBLOCKWIDTH;
      खण्डम् <<= 1;
      यावत् ((खण्डम् & binblocks_r) == 0)
      {
	idx += BINBLOCKWIDTH;
	खण्डम् <<= 1;
      }
    }

    /* For each possibly nonempty block ... */
    प्रति (;;)
    {
      startidx = idx;          /* (track incomplete blocks) */
      q = bin = bin_at(idx);

      /* For each bin in this block ... */
      कुरु
      {
	/* Find and use first big enough chunk ... */

	प्रति (victim = चरमः(bin); victim != bin; victim = victim->bk)
	{
	  victim_आकार = chunksize(victim);
	  remainder_आकार = victim_आकार - nb;

	  यदि (remainder_आकार >= (दीर्घ)MINSIZE) /*विभज*/
	  {
	    remainder = chunk_at_विस्थापनम्(victim, nb);
	    समुच्चयः_शीर्ष(victim, nb | पूर्व_INUSE);
	    unlink(victim, bck, fwd);
	    link_चरमः_remainder(remainder);
	    समुच्चयः_शीर्ष(remainder, remainder_आकार | पूर्व_INUSE);
	    समुच्चयः_foot(remainder, remainder_आकार);
	    check_malloced_chunk(victim, nb);
	    VALGRIND_MALLOCLIKE_खण्डम्(chunk2mem(victim), अष्टकानि, आकार_SZ, false);
	    प्रत्यागम chunk2mem(victim);
	  }

	  अन्यथा यदि (remainder_आकार >= 0)  /* take */
	  {
	    समुच्चयः_inuse_bit_at_विस्थापनम्(victim, victim_आकार);
	    unlink(victim, bck, fwd);
	    check_malloced_chunk(victim, nb);
	    VALGRIND_MALLOCLIKE_खण्डम्(chunk2mem(victim), अष्टकानि, आकार_SZ, false);
	    प्रत्यागम chunk2mem(victim);
	  }

	}

       bin = अग्र_bin(bin);

      } यावत् ((++idx & (BINBLOCKWIDTH - 1)) != 0);

      /* Clear out the block bit. */

      कुरु   /* Possibly backtrack to try to clear a partial block */
      {
	यदि ((startidx & (BINBLOCKWIDTH - 1)) == 0)
	{
	  av_[1] = (mbinptr)(binblocks_r & ~खण्डम्);
	  भङ्ग;
	}
	--startidx;
       q = पूर्व_bin(q);
      } यावत् (first(q) == q);

      /* Get to the next possibly nonempty block */

      यदि ( (खण्डम् <<= 1) <= binblocks_r && (खण्डम् != 0) )
      {
	यावत् ((खण्डम् & binblocks_r) == 0)
	{
	  idx += BINBLOCKWIDTH;
	  खण्डम् <<= 1;
	}
      }
      अन्यथा
	भङ्ग;
    }
  }

  /* Try to use top chunk */

  /* Require that there be a remainder, ensuring top always exists  */
  यदि ( (remainder_आकार = chunksize(शिखरम्) - nb) < (दीर्घ)MINSIZE)
  {

#यदि HAVE_MMAP
    /* If big and would otherwise need to extend, try to use mmap instead */
    यदि ((अचिह्नित दीर्घ)nb >= (अचिह्नित दीर्घ)mmap_देहली &&
	(victim = mmap_chunk(nb)))
      VALGRIND_MALLOCLIKE_खण्डम्(chunk2mem(victim), अष्टकानि, आकार_SZ, false);
      प्रत्यागम chunk2mem(victim);
#यदि_अन्तः

    /* Try to extend */
    malloc_extend_शिखरम्(nb);
    यदि ( (remainder_आकार = chunksize(शिखरम्) - nb) < (दीर्घ)MINSIZE)
      प्रत्यागम रिक्त; /* propagate failure */
  }

  victim = शिखरम्;
  समुच्चयः_शीर्ष(victim, nb | पूर्व_INUSE);
  शिखरम् = chunk_at_विस्थापनम्(victim, nb);
  समुच्चयः_शीर्ष(शिखरम्, remainder_आकार | पूर्व_INUSE);
  check_malloced_chunk(victim, nb);
  VALGRIND_MALLOCLIKE_खण्डम्(chunk2mem(victim), अष्टकानि, आकार_SZ, false);
  प्रत्यागम chunk2mem(victim);

}

/*

  free() algorithm :

    cases:

       1. free(0) has no effect.

       2. If the chunk was allocated via mmap, it is release via munmap().

       3. If a returned chunk borders the current high end of memory,
	  it is consolidated into the top, and if the total unused
	  topmost memory exceeds the trim threshold, malloc_trim is
	  called.

       4. Other chunks are consolidated as they arrive, and
	  placed in corresponding bins. (This includes the case of
	  consolidating with the current `last_remainder').

*/

STATIC_IF_MCHECK
#यदि __STD_C
शून्य fREe_impl(Void_t* mem)
#अन्यथा
शून्य fREe_impl(mem) Void_t* mem;
#यदि_अन्तः
{
  mchunkptr p;         /* chunk corresponding to mem */
  आन्तरिकम्_आकार_T hd;  /* its head field */
  आन्तरिकम्_आकार_T sz;  /* its size */
  पूर्णाङ्क       idx;       /* its bin index */
  mchunkptr अग्र;      /* next contiguous chunk */
  आन्तरिकम्_आकार_T nextsz; /* its size */
  आन्तरिकम्_आकार_T prevsz; /* size of previous contiguous chunk */
  mchunkptr bck;       /* misc temp for linking */
  mchunkptr fwd;       /* misc temp for linking */
  पूर्णाङ्क       islr;      /* track whether merging with last_remainder */

#यदि CONFIG_IS_ENABLED(SYS_MALLOC_F)
	/* free() is a no-op - all the memory will be freed on relocation */
	यदि (!(gd->flags & GD_FLG_पूर्णम्_MALLOC_INIT)) {
		VALGRIND_FREELIKE_खण्डम्(mem, आकार_SZ);
		प्रत्यागम;
	}
#यदि_अन्तः

  यदि (mem == रिक्त)                              /* free(0) has no effect */
    प्रत्यागम;

  p = mem2chunk(mem);
  hd = p->आकार;

#यदि HAVE_MMAP
  यदि (hd & IS_MMAPPED)                       /* release mmapped memory. */
  {
    munmap_chunk(p);
    प्रत्यागम;
  }
#यदि_अन्तः

  check_inuse_chunk(p);

  sz = hd & ~पूर्व_INUSE;
  अग्र = chunk_at_विस्थापनम्(p, sz);
  nextsz = chunksize(अग्र);
  VALGRIND_FREELIKE_खण्डम्(mem, आकार_SZ);

  यदि (अग्र == शिखरम्)                            /* merge with top */
  {
    sz += nextsz;

    यदि (!(hd & पूर्व_INUSE))                    /* consolidate backward */
    {
      prevsz = p->पूर्व_आकार;
      p = chunk_at_विस्थापनम्(p, -((दीर्घ) prevsz));
      sz += prevsz;
      unlink(p, bck, fwd);
    }

    समुच्चयः_शीर्ष(p, sz | पूर्व_INUSE);
    शिखरम् = p;
    यदि ((अचिह्नित दीर्घ)(sz) >= (अचिह्नित दीर्घ)trim_देहली)
      malloc_trim(शिखरम्_pad);
    प्रत्यागम;
  }

  समुच्चयः_शीर्ष(अग्र, nextsz);                    /* clear inuse bit */

  islr = 0;

  यदि (!(hd & पूर्व_INUSE))                    /* consolidate backward */
  {
    prevsz = p->पूर्व_आकार;
    p = chunk_at_विस्थापनम्(p, -((दीर्घ) prevsz));
    sz += prevsz;

    यदि (p->fd == चरमः_remainder)             /* keep as last_remainder */
      islr = 1;
    अन्यथा
      unlink(p, bck, fwd);
  }

  यदि (!(inuse_bit_at_विस्थापनम्(अग्र, nextsz)))   /* consolidate forward */
  {
    sz += nextsz;

    यदि (!islr && अग्र->fd == चरमः_remainder)  /* re-insert last_remainder */
    {
      islr = 1;
      link_चरमः_remainder(p);
    }
    अन्यथा
      unlink(अग्र, bck, fwd);
  }

  समुच्चयः_शीर्ष(p, sz | पूर्व_INUSE);
  समुच्चयः_foot(p, sz);
  यदि (!islr)
    frontlink(p, sz, idx, bck, fwd);
}

/*

  Realloc algorithm:

    Chunks that were obtained via mmap cannot be extended or shrunk
    unless HAVE_MREMAP is defined, in which case mremap is used.
    Otherwise, if their reallocation is for additional space, they are
    copied.  If for less, they are just left alone.

    Otherwise, if the reallocation is for additional space, and the
    chunk can be extended, it is, else a malloc-copy-free sequence is
    taken.  There are several different ways that a chunk could be
    extended. All are tried:

       * Extending forward into following adjacent free chunk.
       * Shifting backwards, joining preceding adjacent space
       * Both shifting backwards and extending forward.
       * Extending into newly sbrked space

    Unless the #define REALLOC_ZERO_BYTES_FREES is set, realloc with a
    size argument of zero (re)allocates a minimum-sized chunk.

    If the reallocation is for less space, and the new request is for
    a `small' (<512 bytes) size, then the newly unused space is lopped
    off and freed.

    The old unix realloc convention of allowing the last-free'd chunk
    to be used as an argument to realloc is no longer supported.
    I don't know of any programs still relying on this feature,
    and allowing it would also allow too many other incorrect
    usages of realloc to be sensible.

*/

STATIC_IF_MCHECK
#यदि __STD_C
Void_t* rEALLOc_impl(Void_t* oldmem, size_t अष्टकानि)
#अन्यथा
Void_t* rEALLOc_impl(oldmem, अष्टकानि) Void_t* oldmem; size_t अष्टकानि;
#यदि_अन्तः
{
  आन्तरिकम्_आकार_T    nb;      /* padded request size */

  mchunkptr oldp;             /* chunk corresponding to oldmem */
  आन्तरिकम्_आकार_T    oldsize; /* its size */

  mchunkptr newp;             /* chunk to return */
  आन्तरिकम्_आकार_T    newsize; /* its size */
  Void_t*   newmem;           /* corresponding user mem */

  mchunkptr अग्र;             /* next contiguous chunk after oldp */
  आन्तरिकम्_आकार_T  nextsize;  /* its size */

  mchunkptr पूर्व;             /* previous contiguous chunk before oldp */
  आन्तरिकम्_आकार_T  prevsize;  /* its size */

  mchunkptr remainder;        /* holds split off extra space from newp */
  आन्तरिकम्_आकार_T  remainder_आकार;   /* its size */

  mchunkptr bck;              /* misc temp for linking */
  mchunkptr fwd;              /* misc temp for linking */

#यदि_परिभाषितम् REALLOC_ZERO_अष्टकानि_FREES
  यदि (!अष्टकानि) {
	fREe_impl(oldmem);
	प्रत्यागम रिक्त;
  }
#यदि_अन्तः

  यदि (अष्टकानि > CONFIG_SYS_MALLOC_LEN || (दीर्घ)अष्टकानि < 0)
     प्रत्यागम रिक्त;

  /* realloc of null is supposed to be same as malloc */
  यदि (oldmem == रिक्त) प्रत्यागम mALLOc_impl(अष्टकानि);

#यदि CONFIG_IS_ENABLED(SYS_MALLOC_F)
	यदि (!(gd->flags & GD_FLG_पूर्णम्_MALLOC_INIT)) {
		/* This is harder to support and should not be needed */
		panic("pre-reloc realloc() is not supported");
	}
#यदि_अन्तः
  यदि (CONFIG_IS_ENABLED(UNIT_TEST) && malloc_testing) {
    यदि (--malloc_max_allocs < 0)
      प्रत्यागम रिक्त;
  }

  newp    = oldp    = mem2chunk(oldmem);
  newsize = oldsize = chunksize(oldp);

  nb = request2size(अष्टकानि);

#यदि HAVE_MMAP
  यदि (chunk_is_mmapped(oldp))
  {
#यदि HAVE_MREMAP
    newp = mremap_chunk(oldp, nb);
    यदि(newp) प्रत्यागम chunk2mem(newp);
#यदि_अन्तः
    /* Note the extra SIZE_SZ overhead. */
    यदि(oldsize - आकार_SZ >= nb) प्रत्यागम oldmem; /* do nothing */
    /* Must alloc, copy, free. */
    newmem = mALLOc_impl(अष्टकानि);
    यदि (!newmem)
	प्रत्यागम रिक्त; /* propagate failure */
    MALLOC_COPY(newmem, oldmem, oldsize - 2*आकार_SZ);
    munmap_chunk(oldp);
    प्रत्यागम newmem;
  }
#यदि_अन्तः

  check_inuse_chunk(oldp);

  यदि ((दीर्घ)(oldsize) < (दीर्घ)(nb))
  {

    /* Try expanding forward */

    अग्र = chunk_at_विस्थापनम्(oldp, oldsize);
    यदि (अग्र == शिखरम् || !inuse(अग्र))
    {
      nextsize = chunksize(अग्र);

      /* Forward into top only if a remainder */
      यदि (अग्र == शिखरम्)
      {
	यदि ((दीर्घ)(nextsize + newsize) >= (दीर्घ)(nb + MINSIZE))
	{
	  newsize += nextsize;
	  शिखरम् = chunk_at_विस्थापनम्(oldp, nb);
	  समुच्चयः_शीर्ष(शिखरम्, (newsize - nb) | पूर्व_INUSE);
	  समुच्चयः_शीर्ष_आकार(oldp, nb);
	  VALGRIND_RESIZEINPLACE_खण्डम्(chunk2mem(oldp), 0, अष्टकानि, आकार_SZ);
	  VALGRIND_MAKE_MEM_DEFINED(chunk2mem(oldp), अष्टकानि);
	  प्रत्यागम chunk2mem(oldp);
	}
      }

      /* Forward into next chunk */
      अन्यथा यदि (((दीर्घ)(nextsize + newsize) >= (दीर्घ)(nb)))
      {
	unlink(अग्र, bck, fwd);
	newsize  += nextsize;
	VALGRIND_RESIZEINPLACE_खण्डम्(chunk2mem(oldp), 0, अष्टकानि, आकार_SZ);
	VALGRIND_MAKE_MEM_DEFINED(chunk2mem(oldp), अष्टकानि);
	गच्छ विभज;
      }
    }
    अन्यथा
    {
      अग्र = रिक्त;
      nextsize = 0;
    }

    /* Try shifting backwards. */

    यदि (!पूर्व_inuse(oldp))
    {
      पूर्व = पूर्व_chunk(oldp);
      prevsize = chunksize(पूर्व);

      /* try forward + backward first to save a later consolidation */

      यदि (अग्र != रिक्त)
      {
	/* into top */
	यदि (अग्र == शिखरम्)
	{
	  यदि ((दीर्घ)(nextsize + prevsize + newsize) >= (दीर्घ)(nb + MINSIZE))
	  {
	    unlink(पूर्व, bck, fwd);
	    newp = पूर्व;
	    newsize += prevsize + nextsize;
	    newmem = chunk2mem(newp);
	    VALGRIND_MALLOCLIKE_खण्डम्(newmem, अष्टकानि, आकार_SZ, false);
	    MALLOC_COPY(newmem, oldmem, oldsize - आकार_SZ);
	    शिखरम् = chunk_at_विस्थापनम्(newp, nb);
	    समुच्चयः_शीर्ष(शिखरम्, (newsize - nb) | पूर्व_INUSE);
	    समुच्चयः_शीर्ष_आकार(newp, nb);
	    VALGRIND_FREELIKE_खण्डम्(oldmem, आकार_SZ);
	    प्रत्यागम newmem;
	  }
	}

	/* into next chunk */
	अन्यथा यदि (((दीर्घ)(nextsize + prevsize + newsize) >= (दीर्घ)(nb)))
	{
	  unlink(अग्र, bck, fwd);
	  unlink(पूर्व, bck, fwd);
	  newp = पूर्व;
	  newsize += nextsize + prevsize;
	  newmem = chunk2mem(newp);
	  VALGRIND_MALLOCLIKE_खण्डम्(newmem, अष्टकानि, आकार_SZ, false);
	  MALLOC_COPY(newmem, oldmem, oldsize - आकार_SZ);
	  गच्छ विभज;
	}
      }

      /* backward only */
      यदि (पूर्व != रिक्त && (दीर्घ)(prevsize + newsize) >= (दीर्घ)nb)
      {
	unlink(पूर्व, bck, fwd);
	newp = पूर्व;
	newsize += prevsize;
	newmem = chunk2mem(newp);
	VALGRIND_MALLOCLIKE_खण्डम्(newmem, अष्टकानि, आकार_SZ, false);
	MALLOC_COPY(newmem, oldmem, oldsize - आकार_SZ);
	गच्छ विभज;
      }
    }

    /* Must allocate */

    newmem = mALLOc_impl (अष्टकानि);

    यदि (newmem == रिक्त)  /* propagate failure */
      प्रत्यागम रिक्त;

    /* Avoid copy if newp is next chunk after oldp. */
    /* (This can only happen when new chunk is sbrk'ed.) */

    यदि ( (newp = mem2chunk(newmem)) == अग्र_chunk(oldp))
    {
      newsize += chunksize(newp);
      newp = oldp;
      गच्छ विभज;
    }

    /* Otherwise copy, free, and exit */
    MALLOC_COPY(newmem, oldmem, oldsize - आकार_SZ);
    fREe_impl(oldmem);
    प्रत्यागम newmem;
  } अन्यथा {
    VALGRIND_RESIZEINPLACE_खण्डम्(oldmem, 0, अष्टकानि, आकार_SZ);
    VALGRIND_MAKE_MEM_DEFINED(oldmem, अष्टकानि);
  }

 विभज:  /* split off extra room in old or expanded chunk */

  यदि (newsize - nb >= MINSIZE) /* split off remainder */
  {
    remainder = chunk_at_विस्थापनम्(newp, nb);
    remainder_आकार = newsize - nb;
    समुच्चयः_शीर्ष_आकार(newp, nb);
    समुच्चयः_शीर्ष(remainder, remainder_आकार | पूर्व_INUSE);
    समुच्चयः_inuse_bit_at_विस्थापनम्(remainder, remainder_आकार);
    VALGRIND_MALLOCLIKE_खण्डम्(chunk2mem(remainder), remainder_आकार, आकार_SZ,
			      false);
    fREe_impl(chunk2mem(remainder)); /* let free() deal with it */
  }
  अन्यथा
  {
    समुच्चयः_शीर्ष_आकार(newp, newsize);
    समुच्चयः_inuse_bit_at_विस्थापनम्(newp, newsize);
  }

  check_inuse_chunk(newp);
  प्रत्यागम chunk2mem(newp);
}

/*

  memalign algorithm:

    memalign requests more than enough space from malloc, finds a spot
    within that chunk that meets the alignment request, and then
    possibly frees the leading and trailing space.

    The alignment argument must be a power of two. This property is not
    checked by memalign, so misuse may result in random runtime errors.

    8-byte alignment is guaranteed by normal malloc calls, so don't
    bother calling memalign with an argument of 8 or less.

    Overreliance on memalign is a sure way to fragment space.

*/

STATIC_IF_MCHECK
#यदि __STD_C
Void_t* mEMALIGn_impl(size_t alignment, size_t अष्टकानि)
#अन्यथा
Void_t* mEMALIGn_impl(alignment, अष्टकानि) size_t alignment; size_t अष्टकानि;
#यदि_अन्तः
{
  आन्तरिकम्_आकार_T    nb;      /* padded  request size */
  वर्ण*     m;                /* memory returned by malloc call */
  mchunkptr p;                /* corresponding chunk */
  वर्ण*     brk;              /* alignment point within p */
  mchunkptr newp;             /* chunk to return */
  आन्तरिकम्_आकार_T  newsize;   /* its size */
  आन्तरिकम्_आकार_T  leadsize;  /* leading space befor alignment point */
  mchunkptr remainder;        /* spare room at end to split off */
  दीर्घ      remainder_आकार;   /* its size */

  यदि (अष्टकानि > CONFIG_SYS_MALLOC_LEN || (दीर्घ)अष्टकानि < 0)
     प्रत्यागम रिक्त;

#यदि CONFIG_IS_ENABLED(SYS_MALLOC_F)
	यदि (!(gd->flags & GD_FLG_पूर्णम्_MALLOC_INIT)) {
		प्रत्यागम memalign_simple(alignment, अष्टकानि);
	}
#यदि_अन्तः

  /* If need less alignment than we give anyway, just relay to malloc */

  यदि (alignment <= MALLOC_ALIGNMENT) प्रत्यागम mALLOc_impl(अष्टकानि);

  /* Otherwise, ensure that it is at least a minimum chunk size */

  यदि (alignment <  MINSIZE) alignment = MINSIZE;

  /* Call malloc with worst case padding to hit alignment. */

  nb = request2size(अष्टकानि);
  m  = (वर्ण*)(mALLOc_impl(nb + alignment + MINSIZE));

  /*
  * The attempt to over-allocate (with a size large enough to guarantee the
  * ability to find an aligned region within allocated memory) failed.
  *
  * Try again, this time only allocating exactly the size the user wants. If
  * the allocation now succeeds and just happens to be aligned, we can still
  * fulfill the user's request.
  */
  यदि (m == रिक्त) {
    size_t extra, extra2;
    /*
     * Use bytes not nb, since mALLOc internally calls request2size too, and
     * each call increases the size to allocate, to account for the header.
     */
    m  = (वर्ण*)(mALLOc_impl(अष्टकानि));
    /* Aligned -> return it */
    यदि ((((अचिह्नित दीर्घ)(m)) % alignment) == 0)
      प्रत्यागम m;
    /*
     * Otherwise, try again, requesting enough extra space to be able to
     * acquire alignment.
     */
    fREe_impl(m);
    /* Add in extra bytes to match misalignment of unexpanded allocation */
    extra = alignment - (((अचिह्नित दीर्घ)(m)) % alignment);
    m  = (वर्ण*)(mALLOc_impl(अष्टकानि + extra));
    /*
     * m might not be the same as before. Validate that the previous value of
     * extra still works for the current value of m.
     * If (!m), extra2=alignment so 
     */
    यदि (m) {
      extra2 = alignment - (((अचिह्नित दीर्घ)(m)) % alignment);
      यदि (extra2 > extra) {
        fREe_impl(m);
        m = रिक्त;
      }
    }
    /* Fall through to original NULL check and chunk splitting logic */
  }

  यदि (m == रिक्त) प्रत्यागम रिक्त; /* propagate failure */

  p = mem2chunk(m);

  यदि ((((अचिह्नित दीर्घ)(m)) % alignment) == 0) /* aligned */
  {
#यदि HAVE_MMAP
    यदि(chunk_is_mmapped(p))
      प्रत्यागम chunk2mem(p); /* nothing more to do */
#यदि_अन्तः
  }
  अन्यथा /* misaligned */
  {
    /*
      Find an aligned spot inside chunk.
      Since we need to give back leading space in a chunk of at
      least MINSIZE, if the first calculation places us at
      a spot with less than MINSIZE leader, we can move to the
      next aligned spot -- we've allocated enough total room so that
      this is always possible.
    */

    brk = (वर्ण*)mem2chunk(((अचिह्नित दीर्घ)(m + alignment - 1)) & -((चिह्नित) alignment));
    यदि ((दीर्घ)(brk - (वर्ण*)(p)) < MINSIZE) brk = brk + alignment;

    newp = (mchunkptr)brk;
    leadsize = brk - (वर्ण*)(p);
    newsize = chunksize(p) - leadsize;

#यदि HAVE_MMAP
    यदि(chunk_is_mmapped(p))
    {
      newp->पूर्व_आकार = p->पूर्व_आकार + leadsize;
      समुच्चयः_शीर्ष(newp, newsize|IS_MMAPPED);
      प्रत्यागम chunk2mem(newp);
    }
#यदि_अन्तः

    /* give back leader, use the rest */

    समुच्चयः_शीर्ष(newp, newsize | पूर्व_INUSE);
    समुच्चयः_inuse_bit_at_विस्थापनम्(newp, newsize);
    समुच्चयः_शीर्ष_आकार(p, leadsize);
    fREe_impl(chunk2mem(p));
    p = newp;
    VALGRIND_MALLOCLIKE_खण्डम्(chunk2mem(p), अष्टकानि, आकार_SZ, false);

    प्रतिज्ञा (newsize >= nb && (((अचिह्नित दीर्घ)(chunk2mem(p))) % alignment) == 0);
  }

  /* Also give back spare room at the end */

  remainder_आकार = chunksize(p) - nb;

  यदि (remainder_आकार >= (दीर्घ)MINSIZE)
  {
    remainder = chunk_at_विस्थापनम्(p, nb);
    समुच्चयः_शीर्ष(remainder, remainder_आकार | पूर्व_INUSE);
    समुच्चयः_शीर्ष_आकार(p, nb);
    VALGRIND_MALLOCLIKE_खण्डम्(chunk2mem(remainder), remainder_आकार, आकार_SZ,
			      false);
    fREe_impl(chunk2mem(remainder));
  }

  check_inuse_chunk(p);
  प्रत्यागम chunk2mem(p);

}

/*
    valloc just invokes memalign with alignment argument equal
    to the page size of the system (or as near to this as can
    be figured out from all the includes/defines above.)
*/

#यदि __STD_C
Void_t* vALLOc(size_t अष्टकानि)
#अन्यथा
Void_t* vALLOc(अष्टकानि) size_t अष्टकानि;
#यदि_अन्तः
{
  प्रत्यागम mEMALIGn (malloc_getpagesize, अष्टकानि);
}

/*
  pvalloc just invokes valloc for the nearest pagesize
  that will accommodate request
*/

#यदि __STD_C
Void_t* pvALLOc(size_t अष्टकानि)
#अन्यथा
Void_t* pvALLOc(अष्टकानि) size_t अष्टकानि;
#यदि_अन्तः
{
  size_t pagesize = malloc_getpagesize;
  प्रत्यागम mEMALIGn (pagesize, (अष्टकानि + pagesize - 1) & ~(pagesize - 1));
}

/*

  calloc calls malloc, then zeroes out the allocated chunk.

*/

STATIC_IF_MCHECK
#यदि __STD_C
Void_t* cALLOc_impl(size_t n, size_t elem_आकार)
#अन्यथा
Void_t* cALLOc_impl(n, elem_आकार) size_t n; size_t elem_आकार;
#यदि_अन्तः
{
  mchunkptr p;
  आन्तरिकम्_आकार_T csz;

  आन्तरिकम्_आकार_T sz = n * elem_आकार;

  /* check if expand_top called, in which case don't need to clear */
#यदि CONFIG_IS_ENABLED(SYS_MALLOC_CLEAR_ON_INIT)
#यदि MORECORE_CLEARS
  mchunkptr oldtop = शिखरम्;
  आन्तरिकम्_आकार_T oldtopsize = chunksize(शिखरम्);
#यदि_अन्तः
#यदि_अन्तः
  Void_t* mem = mALLOc_impl (sz);

  यदि ((दीर्घ)n < 0) प्रत्यागम रिक्त;

  यदि (mem == रिक्त)
    प्रत्यागम रिक्त;
  अन्यथा
  {
#यदि CONFIG_IS_ENABLED(SYS_MALLOC_F)
	यदि (!(gd->flags & GD_FLG_पूर्णम्_MALLOC_INIT)) {
		स्मृतिं_पूरय(mem, 0, sz);
		प्रत्यागम mem;
	}
#यदि_अन्तः
    p = mem2chunk(mem);

    /* Two optional cases in which clearing not necessary */

#यदि HAVE_MMAP
    यदि (chunk_is_mmapped(p)) प्रत्यागम mem;
#यदि_अन्तः

    csz = chunksize(p);

#यदि CONFIG_IS_ENABLED(SYS_MALLOC_CLEAR_ON_INIT)
#यदि MORECORE_CLEARS
    यदि (p == oldtop && csz > oldtopsize)
    {
      /* clear only the bytes from non-freshly-sbrked memory */
      csz = oldtopsize;
    }
#यदि_अन्तः
#यदि_अन्तः

    MALLOC_ZERO(mem, csz - आकार_SZ);
    VALGRIND_MAKE_MEM_DEFINED(mem, sz);
    प्रत्यागम mem;
  }
}

/*

  cfree just calls free. It is needed/defined on some systems
  that pair it with calloc, presumably for odd historical reasons.

*/

#यदि !defined(आन्तरिकम्_LINUX_C_LIB) || !defined(__ELF__)
#यदि __STD_C
शून्य cfree(Void_t *mem)
#अन्यथा
शून्य cfree(mem) Void_t *mem;
#यदि_अन्तः
{
  fREe(mem);
}
#यदि_अन्तः

#यदि_परिभाषितम् MCHECK_HEAP_PROTECTION
 #अन्तर्भाव "mcheck_core.inc.h"
 #यदि !__STD_C
  #दोषः "must have __STD_C"
 #यदि_अन्तः

Void_t *mALLOc(size_t अष्टकानि)
{
	mcheck_pedantic_prehook();
	size_t fullsz = mcheck_alloc_prehook(अष्टकानि);
	शून्य *p = mALLOc_impl(fullsz);

	यदि (!p)
		प्रत्यागम p;
	प्रत्यागम mcheck_alloc_posthook(p, अष्टकानि);
}

शून्य fREe(Void_t *mem) { fREe_impl(mcheck_free_prehook(mem)); }

Void_t *rEALLOc(Void_t *oldmem, size_t अष्टकानि)
{
	mcheck_pedantic_prehook();
	यदि (अष्टकानि == 0) {
		यदि (oldmem)
			fREe(oldmem);
		प्रत्यागम रिक्त;
	}

	यदि (oldmem == रिक्त)
		प्रत्यागम mALLOc(अष्टकानि);

	शून्य *p = mcheck_reallocfree_prehook(oldmem);
	size_t newsz = mcheck_alloc_prehook(अष्टकानि);

	p = rEALLOc_impl(p, newsz);
	यदि (!p)
		प्रत्यागम p;
	प्रत्यागम mcheck_alloc_noclean_posthook(p, अष्टकानि);
}

Void_t *mEMALIGn(size_t alignment, size_t अष्टकानि)
{
	mcheck_pedantic_prehook();
	size_t fullsz = mcheck_memalign_prehook(alignment, अष्टकानि);
	शून्य *p = mEMALIGn_impl(alignment, fullsz);

	यदि (!p)
		प्रत्यागम p;
	प्रत्यागम mcheck_memalign_posthook(alignment, p, अष्टकानि);
}

// pvALLOc, vALLOc - redirect to mEMALIGn, defined here, so they need no wrapping.

Void_t *cALLOc(size_t n, size_t elem_आकार)
{
	mcheck_pedantic_prehook();
	// NB: here is no overflow check.
	size_t fullsz = mcheck_alloc_prehook(n * elem_आकार);
	शून्य *p = cALLOc_impl(1, fullsz);

	यदि (!p)
		प्रत्यागम p;
	प्रत्यागम mcheck_alloc_noclean_posthook(p, n * elem_आकार);
}

// mcheck API {
पूर्णाङ्क mcheck_pedantic(mcheck_abortfunc_t f)
{
	mcheck_initialize(f, 1);
	प्रत्यागम 0;
}

पूर्णाङ्क mcheck(mcheck_abortfunc_t f)
{
	mcheck_initialize(f, 0);
	प्रत्यागम 0;
}

शून्य mcheck_check_all(शून्य) { mcheck_pedantic_check(); }

गणना mcheck_status mprobe(शून्य *__ptr) { प्रत्यागम mcheck_mprobe(__ptr); }
// mcheck API }
#यदि_अन्तः

/*

    Malloc_trim gives memory back to the system (via negative
    arguments to sbrk) if there is unused memory at the `high' end of
    the malloc pool. You can call this after freeing large blocks of
    memory to potentially reduce the system-level memory requirements
    of a program. However, it cannot guarantee to reduce memory. Under
    some allocation patterns, some large free blocks of memory will be
    locked between two used chunks, so they cannot be given back to
    the system.

    The `pad' argument to malloc_trim represents the amount of free
    trailing space to leave untrimmed. If this argument is zero,
    only the minimum amount of memory to maintain internal data
    structures will be left (one page or less). Non-zero arguments
    can be supplied to maintain enough trailing space to service
    future expected allocations without having to re-obtain memory
    from the system.

    Malloc_trim returns 1 if it actually released any memory, else 0.

*/

#यदि __STD_C
पूर्णाङ्क malloc_trim(size_t pad)
#अन्यथा
पूर्णाङ्क malloc_trim(pad) size_t pad;
#यदि_अन्तः
{
  दीर्घ  शिखरम्_आकार;        /* Amount of top-most memory */
  दीर्घ  extra;           /* Amount to release */
  वर्ण* current_brk;     /* address returned by pre-check sbrk call */
  वर्ण* new_brk;         /* address returned by negative sbrk call */

  अचिह्नित दीर्घ pagesz = malloc_getpagesize;

  शिखरम्_आकार = chunksize(शिखरम्);
  extra = ((शिखरम्_आकार - pad - MINSIZE + (pagesz-1)) / pagesz - 1) * pagesz;

  यदि (extra < (दीर्घ)pagesz)  /* Not enough memory to release */
    प्रत्यागम 0;

  अन्यथा
  {
    /* Test to make sure no one else called sbrk */
    current_brk = (वर्ण*)(MORECORE (0));
    यदि (current_brk != (वर्ण*)(शिखरम्) + शिखरम्_आकार)
      प्रत्यागम 0;     /* Apparently we don't own memory; must fail */

    अन्यथा
    {
      new_brk = (वर्ण*)(MORECORE (-extra));

      यदि (new_brk == (वर्ण*)(MORECORE_FAILURE)) /* sbrk failed? */
      {
	/* Try to figure out what we have */
	current_brk = (वर्ण*)(MORECORE (0));
	शिखरम्_आकार = current_brk - (वर्ण*)शिखरम्;
	यदि (शिखरम्_आकार >= (दीर्घ)MINSIZE) /* if not, we are very very dead! */
	{
	  sbrked_mem = current_brk - sbrk_आधारः;
	  समुच्चयः_शीर्ष(शिखरम्, शिखरम्_आकार | पूर्व_INUSE);
	}
	check_chunk(शिखरम्);
	प्रत्यागम 0;
      }

      अन्यथा
      {
	/* Success. Adjust top accordingly. */
	समुच्चयः_शीर्ष(शिखरम्, (शिखरम्_आकार - extra) | पूर्व_INUSE);
	sbrked_mem -= extra;
	check_chunk(शिखरम्);
	प्रत्यागम 1;
      }
    }
  }
}

/*
  malloc_usable_size:

    This routine tells you how many bytes you can actually use in an
    allocated chunk, which may be more than you requested (although
    often not). You can use this many bytes without worrying about
    overwriting other allocated objects. Not a particularly great
    programming practice, but still sometimes useful.

*/

#यदि __STD_C
size_t malloc_usable_size(Void_t* mem)
#अन्यथा
size_t malloc_usable_size(mem) Void_t* mem;
#यदि_अन्तः
{
  mchunkptr p;
  यदि (mem == रिक्त)
    प्रत्यागम 0;
  अन्यथा
  {
    p = mem2chunk(mem);
    यदि(!chunk_is_mmapped(p))
    {
      यदि (!inuse(p)) प्रत्यागम 0;
      check_inuse_chunk(p);
      प्रत्यागम chunksize(p) - आकार_SZ;
    }
    प्रत्यागम chunksize(p) - 2*आकार_SZ;
  }
}

/* Utility to update current_mallinfo for malloc_stats and mallinfo() */

#यदि_परिभाषितम् DEBUG
स्थिर शून्य malloc_update_mallinfo(शून्य)
{
  पूर्णाङ्क i;
  mbinptr b;
  mchunkptr p;
#यदि_परिभाषितम् DEBUG
  mchunkptr q;
#यदि_अन्तः

  आन्तरिकम्_आकार_T avail = chunksize(शिखरम्);
  पूर्णाङ्क   navail = ((दीर्घ)(avail) >= (दीर्घ)MINSIZE)? 1 : 0;

  प्रति (i = 1; i < NAV; ++i)
  {
    b = bin_at(i);
    प्रति (p = चरमः(b); p != b; p = p->bk)
    {
#यदि_परिभाषितम् DEBUG
      check_free_chunk(p);
      प्रति (q = अग्र_chunk(p);
	   q < शिखरम् && inuse(q) && (दीर्घ)(chunksize(q)) >= (दीर्घ)MINSIZE;
	   q = अग्र_chunk(q))
	check_inuse_chunk(q);
#यदि_अन्तः
      avail += chunksize(p);
      navail++;
    }
  }

  current_mallinfo.ordblks = navail;
  current_mallinfo.uordblks = sbrked_mem - avail;
  current_mallinfo.fordblks = avail;
  current_mallinfo.hblks = n_mmaps;
  current_mallinfo.hblkhd = mmapped_mem;
  current_mallinfo.keepcost = chunksize(शिखरम्);

}
#यदि_अन्तः	/* DEBUG */

/*

  malloc_stats:

    Prints on the amount of space obtain from the system (both
    via sbrk and mmap), the maximum amount (which may be more than
    current if malloc_trim and/or munmap got called), the maximum
    number of simultaneous mmap regions used, and the current number
    of bytes allocated via malloc (or realloc, etc) but not yet
    freed. (Note that this is the number of bytes allocated, not the
    number requested. It will be larger than the number requested
    because of alignment and bookkeeping overhead.)

*/

#यदि_परिभाषितम् DEBUG
शून्य malloc_stats(शून्य)
{
  malloc_update_mallinfo();
  प्रारूप_लिख("max system bytes = %10u\n",
	  (अचिह्नित पूर्णाङ्क)(max_योग_mem));
  प्रारूप_लिख("system bytes     = %10u\n",
	  (अचिह्नित पूर्णाङ्क)(sbrked_mem + mmapped_mem));
  प्रारूप_लिख("in use bytes     = %10u\n",
	  (अचिह्नित पूर्णाङ्क)(current_mallinfo.uordblks + mmapped_mem));
#यदि HAVE_MMAP
  प्रारूप_लिख("max mmap regions = %10u\n",
	  (अचिह्नित पूर्णाङ्क)max_n_mmaps);
#यदि_अन्तः
}
#यदि_अन्तः	/* DEBUG */

/*
  mallinfo returns a copy of updated current mallinfo.
*/

#यदि_परिभाषितम् DEBUG
संरचना mallinfo mALLINFo(शून्य)
{
  malloc_update_mallinfo();
  प्रत्यागम current_mallinfo;
}
#यदि_अन्तः	/* DEBUG */

/*
  mallopt:

    mallopt is the general SVID/XPG interface to tunable parameters.
    The format is to provide a (parameter-number, parameter-value) pair.
    mallopt then sets the corresponding parameter to the argument
    value if it can (i.e., so long as the value is meaningful),
    and returns 1 if successful else 0.

    See descriptions of tunable parameters above.

*/

#यदि __STD_C
पूर्णाङ्क mALLOPt(पूर्णाङ्क param_सङ्ख्या, पूर्णाङ्क मानम्)
#अन्यथा
पूर्णाङ्क mALLOPt(param_सङ्ख्या, मानम्) पूर्णाङ्क param_सङ्ख्या; पूर्णाङ्क मानम्;
#यदि_अन्तः
{
  विकल्प(param_सङ्ख्या)
  {
    पक्ष M_TRIM_THRESHOLD:
      trim_देहली = मानम्; प्रत्यागम 1;
    पक्ष M_TOP_PAD:
      शिखरम्_pad = मानम्; प्रत्यागम 1;
    पक्ष M_MMAP_THRESHOLD:
      mmap_देहली = मानम्; प्रत्यागम 1;
    पक्ष M_MMAP_MAX:
#यदि HAVE_MMAP
      n_mmaps_max = मानम्; प्रत्यागम 1;
#अन्यथा
      यदि (मानम् != 0) प्रत्यागम 0; अन्यथा  n_mmaps_max = मानम्; प्रत्यागम 1;
#यदि_अन्तः

    स्वभाव:
      प्रत्यागम 0;
  }
}

पूर्णाङ्क initf_malloc(शून्य)
{
#यदि CONFIG_IS_ENABLED(SYS_MALLOC_F)
	प्रतिज्ञा(gd->malloc_आधारः);	/* Set up by crt0.S */
	gd->malloc_सीमा = CONFIG_VAL(SYS_MALLOC_F_LEN);
	gd->malloc_ptr = 0;
#यदि_अन्तः

	प्रत्यागम 0;
}

शून्य malloc_enable_testing(पूर्णाङ्क max_allocs)
{
	malloc_testing = true;
	malloc_max_allocs = max_allocs;
}

शून्य malloc_disable_testing(शून्य)
{
	malloc_testing = false;
}

/*

History:

    V2.6.6 Sun Dec  5 07:42:19 1999  Doug Lea  (dl at gee)
      * return null for negative arguments
      * Added Several WIN32 cleanups from Martin C. Fong <mcfong@yahoo.com>
	 * Add 'LACKS_SYS_PARAM_H' for those systems without 'sys/param.h'
	  (e.g. WIN32 platforms)
	 * Cleanup up header file inclusion for WIN32 platforms
	 * Cleanup code to avoid Microsoft Visual C++ compiler complaints
	 * Add 'USE_DL_PREFIX' to quickly allow co-existence with existing
	   memory allocation routines
	 * Set 'malloc_getpagesize' for WIN32 platforms (needs more work)
	 * Use 'assert' rather than 'ASSERT' in WIN32 code to conform to
	   usage of 'assert' in non-WIN32 code
	 * Improve WIN32 'sbrk()' emulation's 'findRegion()' routine to
	   avoid infinite loop
      * Always call 'fREe()' rather than 'free()'

    V2.6.5 Wed Jun 17 15:57:31 1998  Doug Lea  (dl at gee)
      * Fixed ordering problem with boundary-stamping

    V2.6.3 Sun May 19 08:17:58 1996  Doug Lea  (dl at gee)
      * Added pvalloc, as recommended by H.J. Liu
      * Added 64bit pointer support mainly from Wolfram Gloger
      * Added anonymously donated WIN32 sbrk emulation
      * Malloc, calloc, getpagesize: add optimizations from Raymond Nijssen
      * malloc_extend_top: fix mask error that caused wastage after
	foreign sbrks
      * Add linux mremap support code from HJ Liu

    V2.6.2 Tue Dec  5 06:52:55 1995  Doug Lea  (dl at gee)
      * Integrated most documentation with the code.
      * Add support for mmap, with help from
	Wolfram Gloger (Gloger@lrz.uni-muenchen.de).
      * Use last_remainder in more cases.
      * Pack bins using idea from  colin@nyx10.cs.du.edu
      * Use ordered bins instead of best-fit threshhold
      * Eliminate block-local decls to simplify tracing and debugging.
      * Support another case of realloc via move into top
      * Fix error occuring when initial sbrk_base not word-aligned.
      * Rely on page size for units instead of SBRK_UNIT to
	avoid surprises about sbrk alignment conventions.
      * Add mallinfo, mallopt. Thanks to Raymond Nijssen
	(raymond@es.ele.tue.nl) for the suggestion.
      * Add `pad' argument to malloc_trim and top_pad mallopt parameter.
      * More precautions for cases where other routines call sbrk,
	courtesy of Wolfram Gloger (Gloger@lrz.uni-muenchen.de).
      * Added macros etc., allowing use in linux libc from
	H.J. Lu (hjl@gnu.ai.mit.edu)
      * Inverted this history list

    V2.6.1 Sat Dec  2 14:10:57 1995  Doug Lea  (dl at gee)
      * Re-tuned and fixed to behave more nicely with V2.6.0 changes.
      * Removed all preallocation code since under current scheme
	the work required to undo bad preallocations exceeds
	the work saved in good cases for most test programs.
      * No longer use return list or unconsolidated bins since
	no scheme using them consistently outperforms those that don't
	given above changes.
      * Use best fit for very large chunks to prevent some worst-cases.
      * Added some support for debugging

    V2.6.0 Sat Nov  4 07:05:23 1995  Doug Lea  (dl at gee)
      * Removed footers when chunks are in use. Thanks to
	Paul Wilson (wilson@cs.texas.edu) for the suggestion.

    V2.5.4 Wed Nov  1 07:54:51 1995  Doug Lea  (dl at gee)
      * Added malloc_trim, with help from Wolfram Gloger
	(wmglo@Dent.MED.Uni-Muenchen.DE).

    V2.5.3 Tue Apr 26 10:16:01 1994  Doug Lea  (dl at g)

    V2.5.2 Tue Apr  5 16:20:40 1994  Doug Lea  (dl at g)
      * realloc: try to expand in both directions
      * malloc: swap order of clean-bin strategy;
      * realloc: only conditionally expand backwards
      * Try not to scavenge used bins
      * Use bin counts as a guide to preallocation
      * Occasionally bin return list chunks in first scan
      * Add a few optimizations from colin@nyx10.cs.du.edu

    V2.5.1 Sat Aug 14 15:40:43 1993  Doug Lea  (dl at g)
      * faster bin computation & slightly different binning
      * merged all consolidations to one part of malloc proper
	 (eliminating old malloc_find_space & malloc_clean_bin)
      * Scan 2 returns chunks (not just 1)
      * Propagate failure in realloc if malloc returns 0
      * Add stuff to allow compilation on non-ANSI compilers
	  from kpv@research.att.com

    V2.5 Sat Aug  7 07:41:59 1993  Doug Lea  (dl at g.oswego.edu)
      * removed potential for odd address access in prev_chunk
      * removed dependency on getpagesize.h
      * misc cosmetics and a bit more internal documentation
      * anticosmetics: mangled names in macros to evade debugger strangeness
      * tested on sparc, hp-700, dec-mips, rs6000
	  with gcc & native cc (hp, dec only) allowing
	  Detlefs & Zorn comparison study (in SIGPLAN Notices.)

    Trial version Fri Aug 28 13:14:29 1992  Doug Lea  (dl at g.oswego.edu)
      * Based loosely on libg++-1.2X malloc. (It retains some of the overall
	 structure of old version,  but most details differ.)

*/
