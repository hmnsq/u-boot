// SPDX-License-Identifier: GPL-2.0+
/*
 * This code is based on a version (aka dlmalloc) of malloc/free/realloc written
 * by Doug Lea and released to the public domain, as explained at
 * http://creativecommons.org/publicdomain/zero/1.0/-
 *
 * The original code is available at http://gee.cs.oswego.edu/pub/misc/
 * as file malloc-2.6.6.c.
 */

#ఒకవేళ CONFIG_IS_ENABLED(UNIT_TEST)
#నిర్వచించు DEBUG
#అయితే_ముగింపు

#అన్తర్భావ <log.h>
#అన్తర్భావ <asm/global_data.h>

#అన్తర్భావ <malloc.h>
#అన్తర్భావ <mapmem.h>
#అన్తర్భావ <string.h>
#అన్తర్భావ <asm/io.h>
#అన్తర్భావ <valgrind/memcheck.h>

#నిర్వచితమైతే DEBUG
#ఒకవేళ __STD_C
స్థిర శూన్య malloc_update_mallinfo (శూన్య);
శూన్య malloc_stats (శూన్య);
#లేకపోతే
స్థిర శూన్య malloc_update_mallinfo ();
శూన్య malloc_stats();
#అయితే_ముగింపు
#అయితే_ముగింపు	/* DEBUG */

DECLARE_GLOBAL_దత్తాంశం_PTR;

#నిర్వచితమైతే MCHECK_కుప్ప_PROTECTION
 #నిర్వచించు STATIC_IF_MCHECK స్థిర
 #నిర్వచనం_రద్దు MALLOC_COPY
 #నిర్వచనం_రద్దు MALLOC_ZERO
స్థిర అంతఃవరుస శూన్య MALLOC_ZERO(శూన్య *p, size_t sz) { memset(p, 0, sz); }
స్థిర అంతఃవరుస శూన్య MALLOC_COPY(శూన్య *dest, స్థిరరాశి శూన్య *src, size_t sz) { జ్ఞాపకం_నకలు(dest, src, sz); }
#లేకపోతే
 #నిర్వచించు STATIC_IF_MCHECK
 #నిర్వచించు mALLOc_impl mALLOc
 #నిర్వచించు fREe_impl fREe
 #నిర్వచించు rEALLOc_impl rEALLOc
 #నిర్వచించు mEMALIGn_impl mEMALIGn
 #నిర్వచించు cALLOc_impl cALLOc
#అయితే_ముగింపు

/*
  Emulation of sbrk for WIN32
  All code within the ifdef WIN32 is untested by me.

  Thanks to Martin Fong and others for supplying this.
*/

#నిర్వచితమైతే WIN32

#నిర్వచించు Align_పుట(కూడు) (((కూడు) + (malloc_getpagesize-1)) & \
~(malloc_getpagesize-1))
#నిర్వచించు AlignPage64K(కూడు) (((కూడు) + (0x10000 - 1)) & ~(0x10000 - 1))

/* resrve 64MB to insure large contiguous space */
#నిర్వచించు RESERVED_కొలత (1024*1024*64)
#నిర్వచించు తదుపరి_కొలత (2048*1024)
#నిర్వచించు శిఖరం_జ్ఞాపకం ((గుర్తులేని దీర్ఘ)2*1024*1024*1024)

నిర్మాణం GmListElement;
రకనిర్వచనం నిర్మాణం GmListElement GmListElement;

నిర్మాణం GmListElement
{
	GmListElement* తదుపరి;
	శూన్య* పునాది;
};

స్థిర GmListElement* తల = 0;
స్థిర గుర్తులేని పూర్ణాంకం g_తదుపరి_చిరునామా = 0;
స్థిర గుర్తులేని పూర్ణాంకం g_చిరునామా_పునాది = 0;
స్థిర గుర్తులేని పూర్ణాంకం g_Allocated_కొలత = 0;

స్థిర
GmListElement* makeGmListElement (శూన్య* bas)
{
	GmListElement* this;
	this = (GmListElement*)(శూన్య*)LocalAlloc (0, పరిమాణం (GmListElement));
	ప్రతిజ్ఞా (this);
	ఒకవేళ (this)
	{
		this->పునాది = bas;
		this->తదుపరి = తల;
		తల = this;
	}
	తిరిగి this;
}

శూన్య gcleanup (శూన్య)
{
	BOOL rval;
	ప్రతిజ్ఞా ( (తల == ఖాళీ) || (తల->పునాది == (శూన్య*)g_చిరునామా_పునాది));
	ఒకవేళ (g_చిరునామా_పునాది && (g_తదుపరి_చిరునామా - g_చిరునామా_పునాది))
	{
		rval = VirtualFree ((శూన్య*)g_చిరునామా_పునాది,
							g_తదుపరి_చిరునామా - g_చిరునామా_పునాది,
							MEM_DECOMMIT);
	ప్రతిజ్ఞా (rval);
	}
	అయితే (తల)
	{
		GmListElement* తదుపరి = తల->తదుపరి;
		rval = VirtualFree (తల->పునాది, 0, MEM_RELEASE);
		ప్రతిజ్ఞా (rval);
		LocalFree (తల);
		తల = తదుపరి;
	}
}

స్థిర
శూన్య* కనుగొను_Region (శూన్య* ప్రారంభం_చిరునామా, గుర్తులేని దీర్ఘ కొలత)
{
	జ్ఞాపకం_BASIC_INFORMATION info;
	ఒకవేళ (కొలత >= శిఖరం_జ్ఞాపకం) తిరిగి ఖాళీ;

	అయితే ((గుర్తులేని దీర్ఘ)ప్రారంభం_చిరునామా + కొలత < శిఖరం_జ్ఞాపకం)
	{
		VirtualQuery (ప్రారంభం_చిరునామా, &info, పరిమాణం (info));
		ఒకవేళ ((info.State == MEM_FREE) && (info.Region_కొలత >= కొలత))
			తిరిగి ప్రారంభం_చిరునామా;
		లేకపోతే
		{
			/* Requested region is not available so see if the */
			/* next region is available.  Set 'start_address' */
			/* to the next region and call 'VirtualQuery()' */
			/* again. */

			ప్రారంభం_చిరునామా = (అక్షరం*)info.పునాది_చిరునామా + info.Region_కొలత;

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

			ప్రారంభం_చిరునామా =
				(శూన్య *) AlignPage64K((గుర్తులేని దీర్ఘ) ప్రారంభం_చిరునామా);
		}
	}
	తిరిగి ఖాళీ;

}

శూన్య* wsbrk (దీర్ఘ కొలత)
{
	శూన్య* tmp;
	ఒకవేళ (కొలత > 0)
	{
		ఒకవేళ (g_చిరునామా_పునాది == 0)
		{
			g_Allocated_కొలత = max (RESERVED_కొలత, Align_పుట (కొలత));
			g_తదుపరి_చిరునామా = g_చిరునామా_పునాది =
				(గుర్తులేని పూర్ణాంకం)VirtualAlloc (ఖాళీ, g_Allocated_కొలత,
											MEM_RESERVE, పుట_NOACCESS);
		} లేకపోతే ఒకవేళ (Align_పుట (g_తదుపరి_చిరునామా + కొలత) > (g_చిరునామా_పునాది +
g_Allocated_కొలత))
		{
			దీర్ఘ new_కొలత = max (తదుపరి_కొలత, Align_పుట (కొలత));
			శూన్య* new_చిరునామా = (శూన్య*)(g_చిరునామా_పునాది+g_Allocated_కొలత);
			చేయి
			{
				new_చిరునామా = కనుగొను_Region (new_చిరునామా, new_కొలత);

				ఒకవేళ (!new_చిరునామా)
					తిరిగి (శూన్య*)-1;

				g_చిరునామా_పునాది = g_తదుపరి_చిరునామా =
					(గుర్తులేని పూర్ణాంకం)VirtualAlloc (new_చిరునామా, new_కొలత,
												MEM_RESERVE, పుట_NOACCESS);
				/* repeat in case of race condition */
				/* The region that we found has been snagged */
				/* by another thread */
			}
			అయితే (g_చిరునామా_పునాది == 0);

			ప్రతిజ్ఞా (new_చిరునామా == (శూన్య*)g_చిరునామా_పునాది);

			g_Allocated_కొలత = new_కొలత;

			ఒకవేళ (!makeGmListElement ((శూన్య*)g_చిరునామా_పునాది))
				తిరిగి (శూన్య*)-1;
		}
		ఒకవేళ ((కొలత + g_తదుపరి_చిరునామా) > Align_పుట (g_తదుపరి_చిరునామా))
		{
			శూన్య* res;
			res = VirtualAlloc ((శూన్య*)Align_పుట (g_తదుపరి_చిరునామా),
								(కొలత + g_తదుపరి_చిరునామా -
								 Align_పుట (g_తదుపరి_చిరునామా)),
								MEM_COMMIT, పుట_READWRITE);
			ఒకవేళ (!res)
				తిరిగి (శూన్య*)-1;
		}
		tmp = (శూన్య*)g_తదుపరి_చిరునామా;
		g_తదుపరి_చిరునామా = (గుర్తులేని పూర్ణాంకం)tmp + కొలత;
		తిరిగి tmp;
	}
	లేకపోతే ఒకవేళ (కొలత < 0)
	{
		గుర్తులేని పూర్ణాంకం alignedGoal = Align_పుట (g_తదుపరి_చిరునామా + కొలత);
		/* Trim by releasing the virtual memory */
		ఒకవేళ (alignedGoal >= g_చిరునామా_పునాది)
		{
			VirtualFree ((శూన్య*)alignedGoal, g_తదుపరి_చిరునామా - alignedGoal,
						 MEM_DECOMMIT);
			g_తదుపరి_చిరునామా = g_తదుపరి_చిరునామా + కొలత;
			తిరిగి (శూన్య*)g_తదుపరి_చిరునామా;
		}
		లేకపోతే
		{
			VirtualFree ((శూన్య*)g_చిరునామా_పునాది, g_తదుపరి_చిరునామా - g_చిరునామా_పునాది,
						 MEM_DECOMMIT);
			g_తదుపరి_చిరునామా = g_చిరునామా_పునాది;
			తిరిగి (శూన్య*)-1;
		}
	}
	లేకపోతే
	{
		తిరిగి (శూన్య*)g_తదుపరి_చిరునామా;
	}
}

#అయితే_ముగింపు

/*
  Type declarations
*/

నిర్మాణం malloc_chunk
{
  అంతర్గత_కొలత_T prev_కొలత; /* Size of previous chunk (if free). */
  అంతర్గత_కొలత_T కొలత;      /* Size in bytes, including overhead. */
  నిర్మాణం malloc_chunk* fd;   /* double links -- used only if free. */
  నిర్మాణం malloc_chunk* bk;
} __attribute__((__may_alias__)) ;

రకనిర్వచనం నిర్మాణం malloc_chunk* mchunkptr;

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

#నిర్వచించు కొలత_SZ                (పరిమాణం(అంతర్గత_కొలత_T))
#నిర్వచించు MALLOC_ALIGNMENT       (కొలత_SZ + కొలత_SZ)
#నిర్వచించు MALLOC_ALIGN_ముసుగు      (MALLOC_ALIGNMENT - 1)
#నిర్వచించు MINSIZE                (పరిమాణం(నిర్మాణం malloc_chunk))

/* conversion from malloc headers to user pointers, and back */

#నిర్వచించు chunk2mem(p)   ((Void_t*)((అక్షరం*)(p) + 2*కొలత_SZ))
#నిర్వచించు mem2chunk(mem) ((mchunkptr)((అక్షరం*)(mem) - 2*కొలత_SZ))

/* pad request bytes into a usable size */

#నిర్వచించు request2size(req) \
 ((((req) + (కొలత_SZ + MALLOC_ALIGN_ముసుగు)) < \
  (MINSIZE + MALLOC_ALIGN_ముసుగు)) ? MINSIZE : \
   (((req) + (కొలత_SZ + MALLOC_ALIGN_ముసుగు)) & ~(MALLOC_ALIGN_ముసుగు)))

/* Check if m has acceptable alignment */

#నిర్వచించు aligned_OK(m)    (((గుర్తులేని దీర్ఘ)((m)) & (MALLOC_ALIGN_ముసుగు)) == 0)

/*
  Physical chunk operations
*/

/* size field is or'ed with PREV_INUSE when previous adjacent chunk in use */

#నిర్వచించు PREV_INUSE 0x1

/* size field is or'ed with IS_MMAPPED if the chunk was obtained with mmap() */

#నిర్వచించు IS_MMAPPED 0x2

/* Bits to mask off when extracting size */

#నిర్వచించు కొలత_బిట్లు (PREV_INUSE|IS_MMAPPED)

/* Ptr to next physical malloc_chunk. */

#నిర్వచించు తదుపరి_chunk(p) ((mchunkptr)( ((అక్షరం*)(p)) + ((p)->కొలత & ~PREV_INUSE) ))

/* Ptr to previous physical malloc_chunk */

#నిర్వచించు prev_chunk(p)\
   ((mchunkptr)( ((అక్షరం*)(p)) - ((p)->prev_కొలత) ))

/* Treat space at ptr + offset as a chunk */

#నిర్వచించు chunk_at_స్థానభ్రంశం(p, s)  ((mchunkptr)(((అక్షరం*)(p)) + (s)))

/*
  Dealing with use bits
*/

/* extract p's inuse bit */

#నిర్వచించు inuse(p)\
((((mchunkptr)(((అక్షరం*)(p))+((p)->కొలత & ~PREV_INUSE)))->కొలత) & PREV_INUSE)

/* extract inuse bit of previous chunk */

#నిర్వచించు prev_inuse(p)  ((p)->కొలత & PREV_INUSE)

/* check for mmap()'ed chunk */

#నిర్వచించు chunk_is_mmapped(p) ((p)->కొలత & IS_MMAPPED)

/* set/clear chunk as in use without otherwise disturbing */

#నిర్వచించు సమితి_inuse(p)\
((mchunkptr)(((అక్షరం*)(p)) + ((p)->కొలత & ~PREV_INUSE)))->కొలత |= PREV_INUSE

#నిర్వచించు clear_inuse(p)\
((mchunkptr)(((అక్షరం*)(p)) + ((p)->కొలత & ~PREV_INUSE)))->కొలత &= ~(PREV_INUSE)

/* check/set/clear inuse bits in known places */

#నిర్వచించు inuse_బిట్టు_at_స్థానభ్రంశం(p, s)\
 (((mchunkptr)(((అక్షరం*)(p)) + (s)))->కొలత & PREV_INUSE)

#నిర్వచించు సమితి_inuse_బిట్టు_at_స్థానభ్రంశం(p, s)\
 (((mchunkptr)(((అక్షరం*)(p)) + (s)))->కొలత |= PREV_INUSE)

#నిర్వచించు clear_inuse_బిట్టు_at_స్థానభ్రంశం(p, s)\
 (((mchunkptr)(((అక్షరం*)(p)) + (s)))->కొలత &= ~(PREV_INUSE))

/*
  Dealing with size fields
*/

/* Get size, ignoring use bits */

#నిర్వచించు chunksize(p)          ((p)->కొలత & ~(కొలత_బిట్లు))

/* Set size at head, without disturbing its use bit */

#నిర్వచించు సమితి_తల_కొలత(p, s)   ((p)->కొలత = (((p)->కొలత & PREV_INUSE) | (s)))

/* Set size/use ignoring previous bits in header */

#నిర్వచించు సమితి_తల(p, s)        ((p)->కొలత = (s))

/* Set size at footer (only when chunk is not in use) */

#నిర్వచించు సమితి_foot(p, s)   (((mchunkptr)((అక్షరం*)(p) + (s)))->prev_కొలత = (s))

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

#నిర్వచించు NAV             128   /* number of bins */

రకనిర్వచనం నిర్మాణం malloc_chunk* mbinptr;

/* access macros */

#నిర్వచించు bin_at(i)      ((mbinptr)((అక్షరం*)&(av_[2*(i) + 2]) - 2*కొలత_SZ))
#నిర్వచించు తదుపరి_bin(b)    ((mbinptr)((అక్షరం*)(b) + 2 * పరిమాణం(mbinptr)))
#నిర్వచించు prev_bin(b)    ((mbinptr)((అక్షరం*)(b) - 2 * పరిమాణం(mbinptr)))

/*
   The first 2 bins are never indexed. The corresponding av_ cells are instead
   used for bookkeeping. This is not to save space, but to simplify
   indexing, maintain locality, and avoid some initialization tests.
*/

#నిర్వచించు శిఖరం            (av_[2])          /* The topmost chunk */
#నిర్వచించు చివరి_remainder (bin_at(1))       /* remainder from last split */

/*
   Because top initially points to its own bin with initial
   zero size, thus forcing extension on the first malloc request,
   we avoid having any special code in malloc to check whether
   it even exists yet. But we still need to in malloc_extend_top.
*/

#నిర్వచించు initial_శిఖరం    ((mchunkptr)(bin_at(0)))

/* Helper macro to initialize bins */

#నిర్వచించు IAV(i)  bin_at(i), bin_at(i)

స్థిర mbinptr av_[NAV * 2 + 2] = {
 ఖాళీ, ఖాళీ,
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

#నిర్వచితమైతే CONFIG_SYS_MALLOC_DEFAULT_గమ్యం_INIT
స్థిర శూన్య malloc_init(శూన్య);
#అయితే_ముగింపు

ulong mem_malloc_ప్రారంభం = 0;
ulong mem_malloc_చివర = 0;
ulong mem_malloc_brk = 0;

స్థిర bool malloc_testing;	/* enable test mode */
స్థిర పూర్ణాంకం malloc_max_allocs;	/* return NULL after this many calls to malloc() */

శూన్య *sbrk(ptrdiff_t increment)
{
	ulong old = mem_malloc_brk;
	ulong new = old + increment;

	ఒకవేళ ((new < mem_malloc_ప్రారంభం) || (new > mem_malloc_చివర))
		తిరిగి (శూన్య *)MORECORE_FAILURE;

	/*
	 * if we are giving memory back make sure we clear it out since
	 * we set MORECORE_CLEARS to 1
	 */
	ఒకవేళ (increment < 0)
		memset((శూన్య *)new, 0, -increment);

	mem_malloc_brk = new;

	తిరిగి (శూన్య *)old;
}

శూన్య mem_malloc_init(ulong ప్రారంభం, ulong కొలత)
{
	mem_malloc_ప్రారంభం = (ulong)map_sysmem(ప్రారంభం, కొలత);
	mem_malloc_చివర = mem_malloc_ప్రారంభం + కొలత;
	mem_malloc_brk = mem_malloc_ప్రారంభం;

#నిర్వచితమైతే CONFIG_SYS_MALLOC_DEFAULT_గమ్యం_INIT
	malloc_init();
#అయితే_ముగింపు

	debug("using memory %#lx-%#lx for malloc()\n", mem_malloc_ప్రారంభం,
	      mem_malloc_చివర);
#ఒకవేళ CONFIG_IS_ENABLED(SYS_MALLOC_CLEAR_ON_INIT)
	memset((శూన్య *)mem_malloc_ప్రారంభం, 0x0, కొలత);
#అయితే_ముగింపు
}

/* field-extraction macros */

#నిర్వచించు మొదటి(b) ((b)->fd)
#నిర్వచించు చివరి(b)  ((b)->bk)

/*
  Indexing into bins
*/

#నిర్వచించు bin_సూచీ(sz)                                                          \
(((((గుర్తులేని దీర్ఘ)(sz)) >> 9) ==    0) ?       (((గుర్తులేని దీర్ఘ)(sz)) >>  3): \
 ((((గుర్తులేని దీర్ఘ)(sz)) >> 9) <=    4) ?  56 + (((గుర్తులేని దీర్ఘ)(sz)) >>  6): \
 ((((గుర్తులేని దీర్ఘ)(sz)) >> 9) <=   20) ?  91 + (((గుర్తులేని దీర్ఘ)(sz)) >>  9): \
 ((((గుర్తులేని దీర్ఘ)(sz)) >> 9) <=   84) ? 110 + (((గుర్తులేని దీర్ఘ)(sz)) >> 12): \
 ((((గుర్తులేని దీర్ఘ)(sz)) >> 9) <=  340) ? 119 + (((గుర్తులేని దీర్ఘ)(sz)) >> 15): \
 ((((గుర్తులేని దీర్ఘ)(sz)) >> 9) <= 1364) ? 124 + (((గుర్తులేని దీర్ఘ)(sz)) >> 18): \
					  126)
/*
  bins for chunks < 512 are all spaced 8 bytes apart, and hold
  identically sized chunks. This is exploited in malloc.
*/

#నిర్వచించు MAX_SMALLBIN         63
#నిర్వచించు MAX_SMALLBIN_కొలత   512
#నిర్వచించు SMALLBIN_వెడల్పు        8

#నిర్వచించు smallbin_సూచీ(sz)  (((గుర్తులేని దీర్ఘ)(sz)) >> 3)

/*
   Requests are `small' if both the corresponding and the next bin are small
*/

#నిర్వచించు is_small_request(nb) (nb < MAX_SMALLBIN_కొలత - SMALLBIN_వెడల్పు)

/*
    To help compensate for the large number of bins, a one-level index
    structure is used for bin-by-bin searching.  `binblocks' is a
    one-word bitvector recording whether groups of BINBLOCKWIDTH bins
    have any (possibly) non-empty bins, so they can be skipped over
    all at once during during traversals. The bits are NOT always
    cleared as soon as all bins in a block are empty, but instead only
    when all are noticed to be empty during traversal in malloc.
*/

#నిర్వచించు BINBLOCKWIDTH     4   /* bins per block */

#నిర్వచించు binblocks_r     ((అంతర్గత_కొలత_T)av_[1]) /* bitvector of nonempty blocks */
#నిర్వచించు binblocks_w     (av_[1])

/* bin<->block macros */

#నిర్వచించు idx2binblock(ix)    ((గుర్తులేని)1 << (ix / BINBLOCKWIDTH))
#నిర్వచించు mark_binblock(ii)   (binblocks_w = (mbinptr)(binblocks_r | idx2binblock(ii)))
#నిర్వచించు clear_binblock(ii)  (binblocks_w = (mbinptr)(binblocks_r & ~(idx2binblock(ii))))

/*  Other static bookkeeping data */

/* variables holding tunable values */

స్థిర గుర్తులేని దీర్ఘ trim_పరిమితి   = DEFAULT_TRIM_పరిమితి;
స్థిర గుర్తులేని దీర్ఘ శిఖరం_pad          = DEFAULT_శిఖరం_PAD;
స్థిర గుర్తులేని పూర్ణాంకం  n_mmaps_max      = DEFAULT_MMAP_MAX;
స్థిర గుర్తులేని దీర్ఘ mmap_పరిమితి   = DEFAULT_MMAP_పరిమితి;

/* The first value returned from sbrk */
స్థిర అక్షరం* sbrk_పునాది = (అక్షరం*)(-1);

/* The maximum memory obtained from system via sbrk */
స్థిర గుర్తులేని దీర్ఘ max_sbrked_mem = 0;

/* The maximum via either sbrk or mmap */
స్థిర గుర్తులేని దీర్ఘ max_మొత్తం_mem = 0;

/* internal working copy of mallinfo */
స్థిర నిర్మాణం mallinfo current_mallinfo = {  0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

/* The total memory obtained from system via sbrk */
#నిర్వచించు sbrked_mem  (current_mallinfo.arena)

/* Tracking mmaps */

#నిర్వచితమైతే DEBUG
స్థిర గుర్తులేని పూర్ణాంకం n_mmaps = 0;
#అయితే_ముగింపు	/* DEBUG */
స్థిర గుర్తులేని దీర్ఘ mmapped_mem = 0;
#ఒకవేళ HAVE_MMAP
స్థిర గుర్తులేని పూర్ణాంకం max_n_mmaps = 0;
స్థిర గుర్తులేని దీర్ఘ max_mmapped_mem = 0;
#అయితే_ముగింపు

#నిర్వచితమైతే CONFIG_SYS_MALLOC_DEFAULT_గమ్యం_INIT
స్థిర శూన్య malloc_init(శూన్య)
{
	పూర్ణాంకం i, j;

	debug("bins (av_ array) are at %p\n", (శూన్య *)av_);

	av_[0] = ఖాళీ; av_[1] = ఖాళీ;
	కొరకు (i = 2, j = 2; i < NAV * 2 + 2; i += 2, j++) {
		av_[i] = bin_at(j - 2);
		av_[i + 1] = bin_at(j - 2);

		/* Just print the first few bins so that
		 * we can see there are alright.
		 */
		ఒకవేళ (i < 10)
			debug("av_[%d]=%lx av_[%d]=%lx\n",
			      i, (ulong)av_[i],
			      i + 1, (ulong)av_[i + 1]);
	}

	/* Init the static bookkeeping as well */
	sbrk_పునాది = (అక్షరం *)(-1);
	max_sbrked_mem = 0;
	max_మొత్తం_mem = 0;
#నిర్వచితమైతే DEBUG
	memset((శూన్య *)&current_mallinfo, 0, పరిమాణం(నిర్మాణం mallinfo));
#అయితే_ముగింపు
}
#అయితే_ముగింపు

/*
  Debugging support
*/

#నిర్వచితమైతే DEBUG

/*
  These routines make a number of assertions about the states
  of data structures that should be true at all times. If any
  are not true, it's very likely that a user program has somehow
  trashed memory. (It's also possible that there is a coding error
  in malloc. In which case, please report it!)
*/

#ఒకవేళ __STD_C
స్థిర శూన్య do_check_chunk(mchunkptr p)
#లేకపోతే
స్థిర శూన్య do_check_chunk(p) mchunkptr p;
#అయితే_ముగింపు
{
  అంతర్గత_కొలత_T sz = p->కొలత & ~PREV_INUSE;

  /* No checkable chunk is mmapped */
  ప్రతిజ్ఞా(!chunk_is_mmapped(p));

  /* Check for legal address ... */
  ప్రతిజ్ఞా((అక్షరం*)p >= sbrk_పునాది);
  ఒకవేళ (p != శిఖరం)
    ప్రతిజ్ఞా((అక్షరం*)p + sz <= (అక్షరం*)శిఖరం);
  లేకపోతే
    ప్రతిజ్ఞా((అక్షరం*)p + sz <= sbrk_పునాది + sbrked_mem);

}

#ఒకవేళ __STD_C
స్థిర శూన్య do_check_free_chunk(mchunkptr p)
#లేకపోతే
స్థిర శూన్య do_check_free_chunk(p) mchunkptr p;
#అయితే_ముగింపు
{
  అంతర్గత_కొలత_T sz = p->కొలత & ~PREV_INUSE;
  mchunkptr తదుపరి = chunk_at_స్థానభ్రంశం(p, sz);

  do_check_chunk(p);

  /* Check whether it claims to be free ... */
  ప్రతిజ్ఞా(!inuse(p));

  /* Unless a special marker, must have OK fields */
  ఒకవేళ ((దీర్ఘ)sz >= (దీర్ఘ)MINSIZE)
  {
    ప్రతిజ్ఞా((sz & MALLOC_ALIGN_ముసుగు) == 0);
    ప్రతిజ్ఞా(aligned_OK(chunk2mem(p)));
    /* ... matching footer field */
    ప్రతిజ్ఞా(తదుపరి->prev_కొలత == sz);
    /* ... and is fully consolidated */
    ప్రతిజ్ఞా(prev_inuse(p));
    ప్రతిజ్ఞా (తదుపరి == శిఖరం || inuse(తదుపరి));

    /* ... and has minimally sane links */
    ప్రతిజ్ఞా(p->fd->bk == p);
    ప్రతిజ్ఞా(p->bk->fd == p);
  }
  లేకపోతే /* markers are always of size SIZE_SZ */
    ప్రతిజ్ఞా(sz == కొలత_SZ);
}

#ఒకవేళ __STD_C
స్థిర శూన్య do_check_inuse_chunk(mchunkptr p)
#లేకపోతే
స్థిర శూన్య do_check_inuse_chunk(p) mchunkptr p;
#అయితే_ముగింపు
{
  mchunkptr తదుపరి = తదుపరి_chunk(p);
  do_check_chunk(p);

  /* Check whether it claims to be in use ... */
  ప్రతిజ్ఞా(inuse(p));

  /* ... and is surrounded by OK chunks.
    Since more things can be checked with free chunks than inuse ones,
    if an inuse chunk borders them and debug is on, it's worth doing them.
  */
  ఒకవేళ (!prev_inuse(p))
  {
    mchunkptr prv = prev_chunk(p);
    ప్రతిజ్ఞా(తదుపరి_chunk(prv) == p);
    do_check_free_chunk(prv);
  }
  ఒకవేళ (తదుపరి == శిఖరం)
  {
    ప్రతిజ్ఞా(prev_inuse(తదుపరి));
    ప్రతిజ్ఞా(chunksize(తదుపరి) >= MINSIZE);
  }
  లేకపోతే ఒకవేళ (!inuse(తదుపరి))
    do_check_free_chunk(తదుపరి);

}

#ఒకవేళ __STD_C
స్థిర శూన్య do_check_malloced_chunk(mchunkptr p, అంతర్గత_కొలత_T s)
#లేకపోతే
స్థిర శూన్య do_check_malloced_chunk(p, s) mchunkptr p; అంతర్గత_కొలత_T s;
#అయితే_ముగింపు
{
  అంతర్గత_కొలత_T sz = p->కొలత & ~PREV_INUSE;
  దీర్ఘ స్థలం = sz - s;

  do_check_inuse_chunk(p);

  /* Legal size ... */
  ప్రతిజ్ఞా((దీర్ఘ)sz >= (దీర్ఘ)MINSIZE);
  ప్రతిజ్ఞా((sz & MALLOC_ALIGN_ముసుగు) == 0);
  ప్రతిజ్ఞా(స్థలం >= 0);
  ప్రతిజ్ఞా(స్థలం < (దీర్ఘ)MINSIZE);

  /* ... and alignment */
  ప్రతిజ్ఞా(aligned_OK(chunk2mem(p)));

  /* ... and was allocated at front of an available chunk */
  ప్రతిజ్ఞా(prev_inuse(p));

}

#నిర్వచించు check_free_chunk(P)  do_check_free_chunk(P)
#నిర్వచించు check_inuse_chunk(P) do_check_inuse_chunk(P)
#నిర్వచించు check_chunk(P) do_check_chunk(P)
#నిర్వచించు check_malloced_chunk(P,N) do_check_malloced_chunk(P,N)
#లేకపోతే
#నిర్వచించు check_free_chunk(P)
#నిర్వచించు check_inuse_chunk(P)
#నిర్వచించు check_chunk(P)
#నిర్వచించు check_malloced_chunk(P,N)
#అయితే_ముగింపు

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

#నిర్వచించు frontlink(P, S, IDX, BK, FD)                                          \
{                                                                             \
  ఒకవేళ (S < MAX_SMALLBIN_కొలత)                                                  \
  {                                                                           \
    IDX = smallbin_సూచీ(S);                                                  \
    mark_binblock(IDX);                                                       \
    BK = bin_at(IDX);                                                         \
    FD = BK->fd;                                                              \
    P->bk = BK;                                                               \
    P->fd = FD;                                                               \
    FD->bk = BK->fd = P;                                                      \
  }                                                                           \
  లేకపోతే                                                                        \
  {                                                                           \
    IDX = bin_సూచీ(S);                                                       \
    BK = bin_at(IDX);                                                         \
    FD = BK->fd;                                                              \
    ఒకవేళ (FD == BK) mark_binblock(IDX);                                         \
    లేకపోతే                                                                      \
    {                                                                         \
      అయితే (FD != BK && S < chunksize(FD)) FD = FD->fd;                      \
      BK = FD->bk;                                                            \
    }                                                                         \
    P->bk = BK;                                                               \
    P->fd = FD;                                                               \
    FD->bk = BK->fd = P;                                                      \
  }                                                                           \
}

/* take a chunk off a list */

#నిర్వచించు unlink(P, BK, FD)                                                     \
{                                                                             \
  BK = P->bk;                                                                 \
  FD = P->fd;                                                                 \
  FD->bk = BK;                                                                \
  BK->fd = FD;                                                                \
}                                                                             \

/* Place p as the last remainder */

#నిర్వచించు link_చివరి_remainder(P)                                                \
{                                                                             \
  చివరి_remainder->fd = చివరి_remainder->bk =  P;                               \
  P->fd = P->bk = చివరి_remainder;                                             \
}

/* Clear the last_remainder bin */

#నిర్వచించు clear_చివరి_remainder \
  (చివరి_remainder->fd = చివరి_remainder->bk = చివరి_remainder)

/* Routines dealing with mmap(). */

#ఒకవేళ HAVE_MMAP

#ఒకవేళ __STD_C
స్థిర mchunkptr mmap_chunk(size_t కొలత)
#లేకపోతే
స్థిర mchunkptr mmap_chunk(కొలత) size_t కొలత;
#అయితే_ముగింపు
{
  size_t పుట_ముసుగు = malloc_getpagesize - 1;
  mchunkptr p;

#నిర్వచితం_కాకపోతే MAP_ANONYMOUS
  స్థిర పూర్ణాంకం fd = -1;
#అయితే_ముగింపు

  ఒకవేళ(n_mmaps >= n_mmaps_max) తిరిగి 0; /* too many regions */

  /* For mmapped chunks, the overhead is one SIZE_SZ unit larger, because
   * there is no following chunk whose prev_size field could be used.
   */
  కొలత = (కొలత + కొలత_SZ + పుట_ముసుగు) & ~పుట_ముసుగు;

#నిర్వచితమైతే MAP_ANONYMOUS
  p = (mchunkptr)mmap(0, కొలత, PROT_READ|PROT_WRITE,
		      MAP_వ్యక్తిగత|MAP_ANONYMOUS, -1, 0);
#లేకపోతే /* !MAP_ANONYMOUS */
  ఒకవేళ (fd < 0)
  {
    fd = open("/dev/zero", O_RDWR);
    ఒకవేళ(fd < 0) తిరిగి 0;
  }
  p = (mchunkptr)mmap(0, కొలత, PROT_READ|PROT_WRITE, MAP_వ్యక్తిగత, fd, 0);
#అయితే_ముగింపు

  ఒకవేళ(p == (mchunkptr)-1) తిరిగి 0;

  n_mmaps++;
  ఒకవేళ (n_mmaps > max_n_mmaps) max_n_mmaps = n_mmaps;

  /* We demand that eight bytes into a page must be 8-byte aligned. */
  ప్రతిజ్ఞా(aligned_OK(chunk2mem(p)));

  /* The offset to the start of the mmapped region is stored
   * in the prev_size field of the chunk; normally it is zero,
   * but that can be changed in memalign().
   */
  p->prev_కొలత = 0;
  సమితి_తల(p, కొలత|IS_MMAPPED);

  mmapped_mem += కొలత;
  ఒకవేళ ((గుర్తులేని దీర్ఘ)mmapped_mem > (గుర్తులేని దీర్ఘ)max_mmapped_mem)
    max_mmapped_mem = mmapped_mem;
  ఒకవేళ ((గుర్తులేని దీర్ఘ)(mmapped_mem + sbrked_mem) > (గుర్తులేని దీర్ఘ)max_మొత్తం_mem)
    max_మొత్తం_mem = mmapped_mem + sbrked_mem;
  తిరిగి p;
}

#ఒకవేళ __STD_C
స్థిర శూన్య munmap_chunk(mchunkptr p)
#లేకపోతే
స్థిర శూన్య munmap_chunk(p) mchunkptr p;
#అయితే_ముగింపు
{
  అంతర్గత_కొలత_T కొలత = chunksize(p);
  పూర్ణాంకం ret;

  ప్రతిజ్ఞా (chunk_is_mmapped(p));
  ప్రతిజ్ఞా(! ((అక్షరం*)p >= sbrk_పునాది && (అక్షరం*)p < sbrk_పునాది + sbrked_mem));
  ప్రతిజ్ఞా((n_mmaps > 0));
  ప్రతిజ్ఞా(((p->prev_కొలత + కొలత) & (malloc_getpagesize-1)) == 0);

  n_mmaps--;
  mmapped_mem -= (కొలత + p->prev_కొలత);

  ret = munmap((అక్షరం *)p - p->prev_కొలత, కొలత + p->prev_కొలత);

  /* munmap returns non-zero on failure */
  ప్రతిజ్ఞా(ret == 0);
}

#ఒకవేళ HAVE_MREMAP

#ఒకవేళ __STD_C
స్థిర mchunkptr mremap_chunk(mchunkptr p, size_t new_కొలత)
#లేకపోతే
స్థిర mchunkptr mremap_chunk(p, new_కొలత) mchunkptr p; size_t new_కొలత;
#అయితే_ముగింపు
{
  size_t పుట_ముసుగు = malloc_getpagesize - 1;
  అంతర్గత_కొలత_T స్థానభ్రంశం = p->prev_కొలత;
  అంతర్గత_కొలత_T కొలత = chunksize(p);
  అక్షరం *cp;

  ప్రతిజ్ఞా (chunk_is_mmapped(p));
  ప్రతిజ్ఞా(! ((అక్షరం*)p >= sbrk_పునాది && (అక్షరం*)p < sbrk_పునాది + sbrked_mem));
  ప్రతిజ్ఞా((n_mmaps > 0));
  ప్రతిజ్ఞా(((కొలత + స్థానభ్రంశం) & (malloc_getpagesize-1)) == 0);

  /* Note the extra SIZE_SZ overhead as in mmap_chunk(). */
  new_కొలత = (new_కొలత + స్థానభ్రంశం + కొలత_SZ + పుట_ముసుగు) & ~పుట_ముసుగు;

  cp = (అక్షరం *)mremap((అక్షరం *)p - స్థానభ్రంశం, కొలత + స్థానభ్రంశం, new_కొలత, 1);

  ఒకవేళ (cp == (అక్షరం *)-1) తిరిగి 0;

  p = (mchunkptr)(cp + స్థానభ్రంశం);

  ప్రతిజ్ఞా(aligned_OK(chunk2mem(p)));

  ప్రతిజ్ఞా((p->prev_కొలత == స్థానభ్రంశం));
  సమితి_తల(p, (new_కొలత - స్థానభ్రంశం)|IS_MMAPPED);

  mmapped_mem -= కొలత + స్థానభ్రంశం;
  mmapped_mem += new_కొలత;
  ఒకవేళ ((గుర్తులేని దీర్ఘ)mmapped_mem > (గుర్తులేని దీర్ఘ)max_mmapped_mem)
    max_mmapped_mem = mmapped_mem;
  ఒకవేళ ((గుర్తులేని దీర్ఘ)(mmapped_mem + sbrked_mem) > (గుర్తులేని దీర్ఘ)max_మొత్తం_mem)
    max_మొత్తం_mem = mmapped_mem + sbrked_mem;
  తిరిగి p;
}

#అయితే_ముగింపు /* HAVE_MREMAP */

#అయితే_ముగింపు /* HAVE_MMAP */

/*
  Extend the top-most chunk by obtaining memory from system.
  Main interface to sbrk (but see also malloc_trim).
*/

#ఒకవేళ __STD_C
స్థిర శూన్య malloc_extend_శిఖరం(అంతర్గత_కొలత_T nb)
#లేకపోతే
స్థిర శూన్య malloc_extend_శిఖరం(nb) అంతర్గత_కొలత_T nb;
#అయితే_ముగింపు
{
  అక్షరం*     brk;                  /* return value from sbrk */
  అంతర్గత_కొలత_T front_misalign; /* unusable bytes at front of sbrked space */
  అంతర్గత_కొలత_T correction;     /* bytes for 2nd sbrk call */
  అక్షరం*     new_brk;              /* return of 2nd sbrk call */
  అంతర్గత_కొలత_T శిఖరం_కొలత;       /* new size of top chunk */

  mchunkptr old_శిఖరం     = శిఖరం;  /* Record state of old top */
  అంతర్గత_కొలత_T old_శిఖరం_కొలత = chunksize(old_శిఖరం);
  అక్షరం*     old_చివర      = (అక్షరం*)(chunk_at_స్థానభ్రంశం(old_శిఖరం, old_శిఖరం_కొలత));

  /* Pad request with top_pad plus minimal overhead */

  అంతర్గత_కొలత_T    sbrk_కొలత     = nb + శిఖరం_pad + MINSIZE;
  గుర్తులేని దీర్ఘ pagesz    = malloc_getpagesize;

  /* If not the first time through, round to preserve page boundary */
  /* Otherwise, we need to correct to a page size below anyway. */
  /* (We also correct below if an intervening foreign sbrk call.) */

  ఒకవేళ (sbrk_పునాది != (అక్షరం*)(-1))
    sbrk_కొలత = (sbrk_కొలత + (pagesz - 1)) & ~(pagesz - 1);

  brk = (అక్షరం*)(MORECORE (sbrk_కొలత));

  /* Fail if sbrk failed or if a foreign sbrk call killed our space */
  ఒకవేళ (brk == (అక్షరం*)(MORECORE_FAILURE) ||
      (brk < old_చివర && old_శిఖరం != initial_శిఖరం))
    తిరిగి;

  sbrked_mem += sbrk_కొలత;

  ఒకవేళ (brk == old_చివర) /* can just add bytes to current top */
  {
    శిఖరం_కొలత = sbrk_కొలత + old_శిఖరం_కొలత;
    సమితి_తల(శిఖరం, శిఖరం_కొలత | PREV_INUSE);
  }
  లేకపోతే
  {
    ఒకవేళ (sbrk_పునాది == (అక్షరం*)(-1))  /* First time through. Record base */
      sbrk_పునాది = brk;
    లేకపోతే  /* Someone else called sbrk().  Count those bytes as sbrked_mem. */
      sbrked_mem += brk - (అక్షరం*)old_చివర;

    /* Guarantee alignment of first new chunk made from this space */
    front_misalign = (గుర్తులేని దీర్ఘ)chunk2mem(brk) & MALLOC_ALIGN_ముసుగు;
    ఒకవేళ (front_misalign > 0)
    {
      correction = (MALLOC_ALIGNMENT) - front_misalign;
      brk += correction;
    }
    లేకపోతే
      correction = 0;

    /* Guarantee the next brk will be at a page boundary */

    correction += ((((గుర్తులేని దీర్ఘ)(brk + sbrk_కొలత))+(pagesz-1)) &
		   ~(pagesz - 1)) - ((గుర్తులేని దీర్ఘ)(brk + sbrk_కొలత));

    /* Allocate correction */
    new_brk = (అక్షరం*)(MORECORE (correction));
    ఒకవేళ (new_brk == (అక్షరం*)(MORECORE_FAILURE)) తిరిగి;

    sbrked_mem += correction;

    శిఖరం = (mchunkptr)brk;
    శిఖరం_కొలత = new_brk - brk + correction;
    సమితి_తల(శిఖరం, శిఖరం_కొలత | PREV_INUSE);

    ఒకవేళ (old_శిఖరం != initial_శిఖరం)
    {

      /* There must have been an intervening foreign sbrk call. */
      /* A double fencepost is necessary to prevent consolidation */

      /* If not enough space to do this, then user did something very wrong */
      ఒకవేళ (old_శిఖరం_కొలత < MINSIZE)
      {
	సమితి_తల(శిఖరం, PREV_INUSE); /* will force null return from malloc */
	తిరిగి;
      }

      /* Also keep size a multiple of MALLOC_ALIGNMENT */
      old_శిఖరం_కొలత = (old_శిఖరం_కొలత - 3*కొలత_SZ) & ~MALLOC_ALIGN_ముసుగు;
      సమితి_తల_కొలత(old_శిఖరం, old_శిఖరం_కొలత);
      chunk_at_స్థానభ్రంశం(old_శిఖరం, old_శిఖరం_కొలత          )->కొలత =
	కొలత_SZ|PREV_INUSE;
      chunk_at_స్థానభ్రంశం(old_శిఖరం, old_శిఖరం_కొలత + కొలత_SZ)->కొలత =
	కొలత_SZ|PREV_INUSE;
      /* If possible, release the rest. */
      ఒకవేళ (old_శిఖరం_కొలత >= MINSIZE)
	fREe(chunk2mem(old_శిఖరం));
    }
  }

  ఒకవేళ ((గుర్తులేని దీర్ఘ)sbrked_mem > (గుర్తులేని దీర్ఘ)max_sbrked_mem)
    max_sbrked_mem = sbrked_mem;
  ఒకవేళ ((గుర్తులేని దీర్ఘ)(mmapped_mem + sbrked_mem) > (గుర్తులేని దీర్ఘ)max_మొత్తం_mem)
    max_మొత్తం_mem = mmapped_mem + sbrked_mem;

  /* We always land on a page boundary */
  ప్రతిజ్ఞా(((గుర్తులేని దీర్ఘ)((అక్షరం*)శిఖరం + శిఖరం_కొలత) & (pagesz - 1)) == 0);
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
#ఒకవేళ __STD_C
Void_t* mALLOc_impl(size_t బైట్లు)
#లేకపోతే
Void_t* mALLOc_impl(బైట్లు) size_t బైట్లు;
#అయితే_ముగింపు
{
  mchunkptr victim;                  /* inspected/selected chunk */
  అంతర్గత_కొలత_T victim_కొలత;       /* its size */
  పూర్ణాంకం       idx;                     /* index for bin traversal */
  mbinptr   bin;                     /* associated bin */
  mchunkptr remainder;               /* remainder from a split */
  దీర్ఘ      remainder_కొలత;          /* its size */
  పూర్ణాంకం       remainder_సూచీ;         /* its bin index */
  గుర్తులేని దీర్ఘ ఖండం;               /* block traverser bit */
  పూర్ణాంకం       startidx;                /* first bin of a traversed block */
  mchunkptr fwd;                     /* misc temp for linking */
  mchunkptr bck;                     /* misc temp for linking */
  mbinptr q;                         /* misc temp */

  అంతర్గత_కొలత_T nb;

#ఒకవేళ CONFIG_IS_ENABLED(SYS_MALLOC_F)
	ఒకవేళ (!(gd->flags & GD_FLG_నిండిన_MALLOC_INIT))
		తిరిగి malloc_simple(బైట్లు);
#అయితే_ముగింపు

  ఒకవేళ (CONFIG_IS_ENABLED(UNIT_TEST) && malloc_testing) {
    ఒకవేళ (--malloc_max_allocs < 0)
      తిరిగి ఖాళీ;
  }

  /* check if mem_malloc_init() was run */
  ఒకవేళ ((mem_malloc_ప్రారంభం == 0) && (mem_malloc_చివర == 0)) {
    /* not initialized yet */
    తిరిగి ఖాళీ;
  }

  ఒకవేళ (బైట్లు > CONFIG_SYS_MALLOC_LEN || (దీర్ఘ)బైట్లు < 0)
     తిరిగి ఖాళీ;

  nb = request2size(బైట్లు);  /* padded request size; */

  /* Check for exact match in a bin */

  ఒకవేళ (is_small_request(nb))  /* Faster version for small requests */
  {
    idx = smallbin_సూచీ(nb);

    /* No traversal or size check necessary for small bins.  */

    q = bin_at(idx);
    victim = చివరి(q);

    /* Also scan the next one, since it would have a remainder < MINSIZE */
    ఒకవేళ (victim == q)
    {
      q = తదుపరి_bin(q);
      victim = చివరి(q);
    }
    ఒకవేళ (victim != q)
    {
      victim_కొలత = chunksize(victim);
      unlink(victim, bck, fwd);
      సమితి_inuse_బిట్టు_at_స్థానభ్రంశం(victim, victim_కొలత);
      check_malloced_chunk(victim, nb);
      VALGRIND_MALLOCLIKE_ఖండం(chunk2mem(victim), బైట్లు, కొలత_SZ, false);
      తిరిగి chunk2mem(victim);
    }

    idx += 2; /* Set for bin scan below. We've already scanned 2 bins. */

  }
  లేకపోతే
  {
    idx = bin_సూచీ(nb);
    bin = bin_at(idx);

    కొరకు (victim = చివరి(bin); victim != bin; victim = victim->bk)
    {
      victim_కొలత = chunksize(victim);
      remainder_కొలత = victim_కొలత - nb;

      ఒకవేళ (remainder_కొలత >= (దీర్ఘ)MINSIZE) /* too big */
      {
	--idx; /* adjust to rescan below after checking last remainder */
	ఆపు;
      }

      లేకపోతే ఒకవేళ (remainder_కొలత >= 0) /* exact fit */
      {
	unlink(victim, bck, fwd);
	సమితి_inuse_బిట్టు_at_స్థానభ్రంశం(victim, victim_కొలత);
	check_malloced_chunk(victim, nb);
        VALGRIND_MALLOCLIKE_ఖండం(chunk2mem(victim), బైట్లు, కొలత_SZ, false);
	తిరిగి chunk2mem(victim);
      }
    }

    ++idx;

  }

  /* Try to use the last split-off remainder */

  ఒకవేళ ( (victim = చివరి_remainder->fd) != చివరి_remainder)
  {
    victim_కొలత = chunksize(victim);
    remainder_కొలత = victim_కొలత - nb;

    ఒకవేళ (remainder_కొలత >= (దీర్ఘ)MINSIZE) /* re-split */
    {
      remainder = chunk_at_స్థానభ్రంశం(victim, nb);
      సమితి_తల(victim, nb | PREV_INUSE);
      link_చివరి_remainder(remainder);
      సమితి_తల(remainder, remainder_కొలత | PREV_INUSE);
      సమితి_foot(remainder, remainder_కొలత);
      check_malloced_chunk(victim, nb);
      VALGRIND_MALLOCLIKE_ఖండం(chunk2mem(victim), బైట్లు, కొలత_SZ, false);
      తిరిగి chunk2mem(victim);
    }

    clear_చివరి_remainder;

    ఒకవేళ (remainder_కొలత >= 0)  /* exhaust */
    {
      సమితి_inuse_బిట్టు_at_స్థానభ్రంశం(victim, victim_కొలత);
      check_malloced_chunk(victim, nb);
      VALGRIND_MALLOCLIKE_ఖండం(chunk2mem(victim), బైట్లు, కొలత_SZ, false);
      తిరిగి chunk2mem(victim);
    }

    /* Else place in bin */

    frontlink(victim, victim_కొలత, remainder_సూచీ, bck, fwd);
  }

  /*
     If there are any possibly nonempty big-enough blocks,
     search for best fitting chunk by scanning bins in blockwidth units.
  */

  ఒకవేళ ( (ఖండం = idx2binblock(idx)) <= binblocks_r)
  {

    /* Get to the first marked block */

    ఒకవేళ ( (ఖండం & binblocks_r) == 0)
    {
      /* force to an even block boundary */
      idx = (idx & ~(BINBLOCKWIDTH - 1)) + BINBLOCKWIDTH;
      ఖండం <<= 1;
      అయితే ((ఖండం & binblocks_r) == 0)
      {
	idx += BINBLOCKWIDTH;
	ఖండం <<= 1;
      }
    }

    /* For each possibly nonempty block ... */
    కొరకు (;;)
    {
      startidx = idx;          /* (track incomplete blocks) */
      q = bin = bin_at(idx);

      /* For each bin in this block ... */
      చేయి
      {
	/* Find and use first big enough chunk ... */

	కొరకు (victim = చివరి(bin); victim != bin; victim = victim->bk)
	{
	  victim_కొలత = chunksize(victim);
	  remainder_కొలత = victim_కొలత - nb;

	  ఒకవేళ (remainder_కొలత >= (దీర్ఘ)MINSIZE) /*విభజించు*/
	  {
	    remainder = chunk_at_స్థానభ్రంశం(victim, nb);
	    సమితి_తల(victim, nb | PREV_INUSE);
	    unlink(victim, bck, fwd);
	    link_చివరి_remainder(remainder);
	    సమితి_తల(remainder, remainder_కొలత | PREV_INUSE);
	    సమితి_foot(remainder, remainder_కొలత);
	    check_malloced_chunk(victim, nb);
	    VALGRIND_MALLOCLIKE_ఖండం(chunk2mem(victim), బైట్లు, కొలత_SZ, false);
	    తిరిగి chunk2mem(victim);
	  }

	  లేకపోతే ఒకవేళ (remainder_కొలత >= 0)  /* take */
	  {
	    సమితి_inuse_బిట్టు_at_స్థానభ్రంశం(victim, victim_కొలత);
	    unlink(victim, bck, fwd);
	    check_malloced_chunk(victim, nb);
	    VALGRIND_MALLOCLIKE_ఖండం(chunk2mem(victim), బైట్లు, కొలత_SZ, false);
	    తిరిగి chunk2mem(victim);
	  }

	}

       bin = తదుపరి_bin(bin);

      } అయితే ((++idx & (BINBLOCKWIDTH - 1)) != 0);

      /* Clear out the block bit. */

      చేయి   /* Possibly backtrack to try to clear a partial block */
      {
	ఒకవేళ ((startidx & (BINBLOCKWIDTH - 1)) == 0)
	{
	  av_[1] = (mbinptr)(binblocks_r & ~ఖండం);
	  ఆపు;
	}
	--startidx;
       q = prev_bin(q);
      } అయితే (మొదటి(q) == q);

      /* Get to the next possibly nonempty block */

      ఒకవేళ ( (ఖండం <<= 1) <= binblocks_r && (ఖండం != 0) )
      {
	అయితే ((ఖండం & binblocks_r) == 0)
	{
	  idx += BINBLOCKWIDTH;
	  ఖండం <<= 1;
	}
      }
      లేకపోతే
	ఆపు;
    }
  }

  /* Try to use top chunk */

  /* Require that there be a remainder, ensuring top always exists  */
  ఒకవేళ ( (remainder_కొలత = chunksize(శిఖరం) - nb) < (దీర్ఘ)MINSIZE)
  {

#ఒకవేళ HAVE_MMAP
    /* If big and would otherwise need to extend, try to use mmap instead */
    ఒకవేళ ((గుర్తులేని దీర్ఘ)nb >= (గుర్తులేని దీర్ఘ)mmap_పరిమితి &&
	(victim = mmap_chunk(nb)))
      VALGRIND_MALLOCLIKE_ఖండం(chunk2mem(victim), బైట్లు, కొలత_SZ, false);
      తిరిగి chunk2mem(victim);
#అయితే_ముగింపు

    /* Try to extend */
    malloc_extend_శిఖరం(nb);
    ఒకవేళ ( (remainder_కొలత = chunksize(శిఖరం) - nb) < (దీర్ఘ)MINSIZE)
      తిరిగి ఖాళీ; /* propagate failure */
  }

  victim = శిఖరం;
  సమితి_తల(victim, nb | PREV_INUSE);
  శిఖరం = chunk_at_స్థానభ్రంశం(victim, nb);
  సమితి_తల(శిఖరం, remainder_కొలత | PREV_INUSE);
  check_malloced_chunk(victim, nb);
  VALGRIND_MALLOCLIKE_ఖండం(chunk2mem(victim), బైట్లు, కొలత_SZ, false);
  తిరిగి chunk2mem(victim);

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
#ఒకవేళ __STD_C
శూన్య fREe_impl(Void_t* mem)
#లేకపోతే
శూన్య fREe_impl(mem) Void_t* mem;
#అయితే_ముగింపు
{
  mchunkptr p;         /* chunk corresponding to mem */
  అంతర్గత_కొలత_T hd;  /* its head field */
  అంతర్గత_కొలత_T sz;  /* its size */
  పూర్ణాంకం       idx;       /* its bin index */
  mchunkptr తదుపరి;      /* next contiguous chunk */
  అంతర్గత_కొలత_T nextsz; /* its size */
  అంతర్గత_కొలత_T prevsz; /* size of previous contiguous chunk */
  mchunkptr bck;       /* misc temp for linking */
  mchunkptr fwd;       /* misc temp for linking */
  పూర్ణాంకం       islr;      /* track whether merging with last_remainder */

#ఒకవేళ CONFIG_IS_ENABLED(SYS_MALLOC_F)
	/* free() is a no-op - all the memory will be freed on relocation */
	ఒకవేళ (!(gd->flags & GD_FLG_నిండిన_MALLOC_INIT)) {
		VALGRIND_FREELIKE_ఖండం(mem, కొలత_SZ);
		తిరిగి;
	}
#అయితే_ముగింపు

  ఒకవేళ (mem == ఖాళీ)                              /* free(0) has no effect */
    తిరిగి;

  p = mem2chunk(mem);
  hd = p->కొలత;

#ఒకవేళ HAVE_MMAP
  ఒకవేళ (hd & IS_MMAPPED)                       /* release mmapped memory. */
  {
    munmap_chunk(p);
    తిరిగి;
  }
#అయితే_ముగింపు

  check_inuse_chunk(p);

  sz = hd & ~PREV_INUSE;
  తదుపరి = chunk_at_స్థానభ్రంశం(p, sz);
  nextsz = chunksize(తదుపరి);
  VALGRIND_FREELIKE_ఖండం(mem, కొలత_SZ);

  ఒకవేళ (తదుపరి == శిఖరం)                            /* merge with top */
  {
    sz += nextsz;

    ఒకవేళ (!(hd & PREV_INUSE))                    /* consolidate backward */
    {
      prevsz = p->prev_కొలత;
      p = chunk_at_స్థానభ్రంశం(p, -((దీర్ఘ) prevsz));
      sz += prevsz;
      unlink(p, bck, fwd);
    }

    సమితి_తల(p, sz | PREV_INUSE);
    శిఖరం = p;
    ఒకవేళ ((గుర్తులేని దీర్ఘ)(sz) >= (గుర్తులేని దీర్ఘ)trim_పరిమితి)
      malloc_trim(శిఖరం_pad);
    తిరిగి;
  }

  సమితి_తల(తదుపరి, nextsz);                    /* clear inuse bit */

  islr = 0;

  ఒకవేళ (!(hd & PREV_INUSE))                    /* consolidate backward */
  {
    prevsz = p->prev_కొలత;
    p = chunk_at_స్థానభ్రంశం(p, -((దీర్ఘ) prevsz));
    sz += prevsz;

    ఒకవేళ (p->fd == చివరి_remainder)             /* keep as last_remainder */
      islr = 1;
    లేకపోతే
      unlink(p, bck, fwd);
  }

  ఒకవేళ (!(inuse_బిట్టు_at_స్థానభ్రంశం(తదుపరి, nextsz)))   /* consolidate forward */
  {
    sz += nextsz;

    ఒకవేళ (!islr && తదుపరి->fd == చివరి_remainder)  /* re-insert last_remainder */
    {
      islr = 1;
      link_చివరి_remainder(p);
    }
    లేకపోతే
      unlink(తదుపరి, bck, fwd);
  }

  సమితి_తల(p, sz | PREV_INUSE);
  సమితి_foot(p, sz);
  ఒకవేళ (!islr)
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
#ఒకవేళ __STD_C
Void_t* rEALLOc_impl(Void_t* oldmem, size_t బైట్లు)
#లేకపోతే
Void_t* rEALLOc_impl(oldmem, బైట్లు) Void_t* oldmem; size_t బైట్లు;
#అయితే_ముగింపు
{
  అంతర్గత_కొలత_T    nb;      /* padded request size */

  mchunkptr oldp;             /* chunk corresponding to oldmem */
  అంతర్గత_కొలత_T    oldsize; /* its size */

  mchunkptr newp;             /* chunk to return */
  అంతర్గత_కొలత_T    newsize; /* its size */
  Void_t*   newmem;           /* corresponding user mem */

  mchunkptr తదుపరి;             /* next contiguous chunk after oldp */
  అంతర్గత_కొలత_T  nextsize;  /* its size */

  mchunkptr prev;             /* previous contiguous chunk before oldp */
  అంతర్గత_కొలత_T  prevsize;  /* its size */

  mchunkptr remainder;        /* holds split off extra space from newp */
  అంతర్గత_కొలత_T  remainder_కొలత;   /* its size */

  mchunkptr bck;              /* misc temp for linking */
  mchunkptr fwd;              /* misc temp for linking */

#నిర్వచితమైతే REALLOC_ZERO_బైట్లు_FREES
  ఒకవేళ (!బైట్లు) {
	fREe_impl(oldmem);
	తిరిగి ఖాళీ;
  }
#అయితే_ముగింపు

  ఒకవేళ (బైట్లు > CONFIG_SYS_MALLOC_LEN || (దీర్ఘ)బైట్లు < 0)
     తిరిగి ఖాళీ;

  /* realloc of null is supposed to be same as malloc */
  ఒకవేళ (oldmem == ఖాళీ) తిరిగి mALLOc_impl(బైట్లు);

#ఒకవేళ CONFIG_IS_ENABLED(SYS_MALLOC_F)
	ఒకవేళ (!(gd->flags & GD_FLG_నిండిన_MALLOC_INIT)) {
		/* This is harder to support and should not be needed */
		panic("pre-reloc realloc() is not supported");
	}
#అయితే_ముగింపు
  ఒకవేళ (CONFIG_IS_ENABLED(UNIT_TEST) && malloc_testing) {
    ఒకవేళ (--malloc_max_allocs < 0)
      తిరిగి ఖాళీ;
  }

  newp    = oldp    = mem2chunk(oldmem);
  newsize = oldsize = chunksize(oldp);

  nb = request2size(బైట్లు);

#ఒకవేళ HAVE_MMAP
  ఒకవేళ (chunk_is_mmapped(oldp))
  {
#ఒకవేళ HAVE_MREMAP
    newp = mremap_chunk(oldp, nb);
    ఒకవేళ(newp) తిరిగి chunk2mem(newp);
#అయితే_ముగింపు
    /* Note the extra SIZE_SZ overhead. */
    ఒకవేళ(oldsize - కొలత_SZ >= nb) తిరిగి oldmem; /* do nothing */
    /* Must alloc, copy, free. */
    newmem = mALLOc_impl(బైట్లు);
    ఒకవేళ (!newmem)
	తిరిగి ఖాళీ; /* propagate failure */
    MALLOC_COPY(newmem, oldmem, oldsize - 2*కొలత_SZ);
    munmap_chunk(oldp);
    తిరిగి newmem;
  }
#అయితే_ముగింపు

  check_inuse_chunk(oldp);

  ఒకవేళ ((దీర్ఘ)(oldsize) < (దీర్ఘ)(nb))
  {

    /* Try expanding forward */

    తదుపరి = chunk_at_స్థానభ్రంశం(oldp, oldsize);
    ఒకవేళ (తదుపరి == శిఖరం || !inuse(తదుపరి))
    {
      nextsize = chunksize(తదుపరి);

      /* Forward into top only if a remainder */
      ఒకవేళ (తదుపరి == శిఖరం)
      {
	ఒకవేళ ((దీర్ఘ)(nextsize + newsize) >= (దీర్ఘ)(nb + MINSIZE))
	{
	  newsize += nextsize;
	  శిఖరం = chunk_at_స్థానభ్రంశం(oldp, nb);
	  సమితి_తల(శిఖరం, (newsize - nb) | PREV_INUSE);
	  సమితి_తల_కొలత(oldp, nb);
	  VALGRIND_RESIZEINPLACE_ఖండం(chunk2mem(oldp), 0, బైట్లు, కొలత_SZ);
	  VALGRIND_MAKE_MEM_DEFINED(chunk2mem(oldp), బైట్లు);
	  తిరిగి chunk2mem(oldp);
	}
      }

      /* Forward into next chunk */
      లేకపోతే ఒకవేళ (((దీర్ఘ)(nextsize + newsize) >= (దీర్ఘ)(nb)))
      {
	unlink(తదుపరి, bck, fwd);
	newsize  += nextsize;
	VALGRIND_RESIZEINPLACE_ఖండం(chunk2mem(oldp), 0, బైట్లు, కొలత_SZ);
	VALGRIND_MAKE_MEM_DEFINED(chunk2mem(oldp), బైట్లు);
	వెళ్ళు విభజించు;
      }
    }
    లేకపోతే
    {
      తదుపరి = ఖాళీ;
      nextsize = 0;
    }

    /* Try shifting backwards. */

    ఒకవేళ (!prev_inuse(oldp))
    {
      prev = prev_chunk(oldp);
      prevsize = chunksize(prev);

      /* try forward + backward first to save a later consolidation */

      ఒకవేళ (తదుపరి != ఖాళీ)
      {
	/* into top */
	ఒకవేళ (తదుపరి == శిఖరం)
	{
	  ఒకవేళ ((దీర్ఘ)(nextsize + prevsize + newsize) >= (దీర్ఘ)(nb + MINSIZE))
	  {
	    unlink(prev, bck, fwd);
	    newp = prev;
	    newsize += prevsize + nextsize;
	    newmem = chunk2mem(newp);
	    VALGRIND_MALLOCLIKE_ఖండం(newmem, బైట్లు, కొలత_SZ, false);
	    MALLOC_COPY(newmem, oldmem, oldsize - కొలత_SZ);
	    శిఖరం = chunk_at_స్థానభ్రంశం(newp, nb);
	    సమితి_తల(శిఖరం, (newsize - nb) | PREV_INUSE);
	    సమితి_తల_కొలత(newp, nb);
	    VALGRIND_FREELIKE_ఖండం(oldmem, కొలత_SZ);
	    తిరిగి newmem;
	  }
	}

	/* into next chunk */
	లేకపోతే ఒకవేళ (((దీర్ఘ)(nextsize + prevsize + newsize) >= (దీర్ఘ)(nb)))
	{
	  unlink(తదుపరి, bck, fwd);
	  unlink(prev, bck, fwd);
	  newp = prev;
	  newsize += nextsize + prevsize;
	  newmem = chunk2mem(newp);
	  VALGRIND_MALLOCLIKE_ఖండం(newmem, బైట్లు, కొలత_SZ, false);
	  MALLOC_COPY(newmem, oldmem, oldsize - కొలత_SZ);
	  వెళ్ళు విభజించు;
	}
      }

      /* backward only */
      ఒకవేళ (prev != ఖాళీ && (దీర్ఘ)(prevsize + newsize) >= (దీర్ఘ)nb)
      {
	unlink(prev, bck, fwd);
	newp = prev;
	newsize += prevsize;
	newmem = chunk2mem(newp);
	VALGRIND_MALLOCLIKE_ఖండం(newmem, బైట్లు, కొలత_SZ, false);
	MALLOC_COPY(newmem, oldmem, oldsize - కొలత_SZ);
	వెళ్ళు విభజించు;
      }
    }

    /* Must allocate */

    newmem = mALLOc_impl (బైట్లు);

    ఒకవేళ (newmem == ఖాళీ)  /* propagate failure */
      తిరిగి ఖాళీ;

    /* Avoid copy if newp is next chunk after oldp. */
    /* (This can only happen when new chunk is sbrk'ed.) */

    ఒకవేళ ( (newp = mem2chunk(newmem)) == తదుపరి_chunk(oldp))
    {
      newsize += chunksize(newp);
      newp = oldp;
      వెళ్ళు విభజించు;
    }

    /* Otherwise copy, free, and exit */
    MALLOC_COPY(newmem, oldmem, oldsize - కొలత_SZ);
    fREe_impl(oldmem);
    తిరిగి newmem;
  } లేకపోతే {
    VALGRIND_RESIZEINPLACE_ఖండం(oldmem, 0, బైట్లు, కొలత_SZ);
    VALGRIND_MAKE_MEM_DEFINED(oldmem, బైట్లు);
  }

 విభజించు:  /* split off extra room in old or expanded chunk */

  ఒకవేళ (newsize - nb >= MINSIZE) /* split off remainder */
  {
    remainder = chunk_at_స్థానభ్రంశం(newp, nb);
    remainder_కొలత = newsize - nb;
    సమితి_తల_కొలత(newp, nb);
    సమితి_తల(remainder, remainder_కొలత | PREV_INUSE);
    సమితి_inuse_బిట్టు_at_స్థానభ్రంశం(remainder, remainder_కొలత);
    VALGRIND_MALLOCLIKE_ఖండం(chunk2mem(remainder), remainder_కొలత, కొలత_SZ,
			      false);
    fREe_impl(chunk2mem(remainder)); /* let free() deal with it */
  }
  లేకపోతే
  {
    సమితి_తల_కొలత(newp, newsize);
    సమితి_inuse_బిట్టు_at_స్థానభ్రంశం(newp, newsize);
  }

  check_inuse_chunk(newp);
  తిరిగి chunk2mem(newp);
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
#ఒకవేళ __STD_C
Void_t* mEMALIGn_impl(size_t alignment, size_t బైట్లు)
#లేకపోతే
Void_t* mEMALIGn_impl(alignment, బైట్లు) size_t alignment; size_t బైట్లు;
#అయితే_ముగింపు
{
  అంతర్గత_కొలత_T    nb;      /* padded  request size */
  అక్షరం*     m;                /* memory returned by malloc call */
  mchunkptr p;                /* corresponding chunk */
  అక్షరం*     brk;              /* alignment point within p */
  mchunkptr newp;             /* chunk to return */
  అంతర్గత_కొలత_T  newsize;   /* its size */
  అంతర్గత_కొలత_T  leadsize;  /* leading space befor alignment point */
  mchunkptr remainder;        /* spare room at end to split off */
  దీర్ఘ      remainder_కొలత;   /* its size */

  ఒకవేళ (బైట్లు > CONFIG_SYS_MALLOC_LEN || (దీర్ఘ)బైట్లు < 0)
     తిరిగి ఖాళీ;

#ఒకవేళ CONFIG_IS_ENABLED(SYS_MALLOC_F)
	ఒకవేళ (!(gd->flags & GD_FLG_నిండిన_MALLOC_INIT)) {
		తిరిగి memalign_simple(alignment, బైట్లు);
	}
#అయితే_ముగింపు

  /* If need less alignment than we give anyway, just relay to malloc */

  ఒకవేళ (alignment <= MALLOC_ALIGNMENT) తిరిగి mALLOc_impl(బైట్లు);

  /* Otherwise, ensure that it is at least a minimum chunk size */

  ఒకవేళ (alignment <  MINSIZE) alignment = MINSIZE;

  /* Call malloc with worst case padding to hit alignment. */

  nb = request2size(బైట్లు);
  m  = (అక్షరం*)(mALLOc_impl(nb + alignment + MINSIZE));

  /*
  * The attempt to over-allocate (with a size large enough to guarantee the
  * ability to find an aligned region within allocated memory) failed.
  *
  * Try again, this time only allocating exactly the size the user wants. If
  * the allocation now succeeds and just happens to be aligned, we can still
  * fulfill the user's request.
  */
  ఒకవేళ (m == ఖాళీ) {
    size_t extra, extra2;
    /*
     * Use bytes not nb, since mALLOc internally calls request2size too, and
     * each call increases the size to allocate, to account for the header.
     */
    m  = (అక్షరం*)(mALLOc_impl(బైట్లు));
    /* Aligned -> return it */
    ఒకవేళ ((((గుర్తులేని దీర్ఘ)(m)) % alignment) == 0)
      తిరిగి m;
    /*
     * Otherwise, try again, requesting enough extra space to be able to
     * acquire alignment.
     */
    fREe_impl(m);
    /* Add in extra bytes to match misalignment of unexpanded allocation */
    extra = alignment - (((గుర్తులేని దీర్ఘ)(m)) % alignment);
    m  = (అక్షరం*)(mALLOc_impl(బైట్లు + extra));
    /*
     * m might not be the same as before. Validate that the previous value of
     * extra still works for the current value of m.
     * If (!m), extra2=alignment so 
     */
    ఒకవేళ (m) {
      extra2 = alignment - (((గుర్తులేని దీర్ఘ)(m)) % alignment);
      ఒకవేళ (extra2 > extra) {
        fREe_impl(m);
        m = ఖాళీ;
      }
    }
    /* Fall through to original NULL check and chunk splitting logic */
  }

  ఒకవేళ (m == ఖాళీ) తిరిగి ఖాళీ; /* propagate failure */

  p = mem2chunk(m);

  ఒకవేళ ((((గుర్తులేని దీర్ఘ)(m)) % alignment) == 0) /* aligned */
  {
#ఒకవేళ HAVE_MMAP
    ఒకవేళ(chunk_is_mmapped(p))
      తిరిగి chunk2mem(p); /* nothing more to do */
#అయితే_ముగింపు
  }
  లేకపోతే /* misaligned */
  {
    /*
      Find an aligned spot inside chunk.
      Since we need to give back leading space in a chunk of at
      least MINSIZE, if the first calculation places us at
      a spot with less than MINSIZE leader, we can move to the
      next aligned spot -- we've allocated enough total room so that
      this is always possible.
    */

    brk = (అక్షరం*)mem2chunk(((గుర్తులేని దీర్ఘ)(m + alignment - 1)) & -((గుర్తుగల) alignment));
    ఒకవేళ ((దీర్ఘ)(brk - (అక్షరం*)(p)) < MINSIZE) brk = brk + alignment;

    newp = (mchunkptr)brk;
    leadsize = brk - (అక్షరం*)(p);
    newsize = chunksize(p) - leadsize;

#ఒకవేళ HAVE_MMAP
    ఒకవేళ(chunk_is_mmapped(p))
    {
      newp->prev_కొలత = p->prev_కొలత + leadsize;
      సమితి_తల(newp, newsize|IS_MMAPPED);
      తిరిగి chunk2mem(newp);
    }
#అయితే_ముగింపు

    /* give back leader, use the rest */

    సమితి_తల(newp, newsize | PREV_INUSE);
    సమితి_inuse_బిట్టు_at_స్థానభ్రంశం(newp, newsize);
    సమితి_తల_కొలత(p, leadsize);
    fREe_impl(chunk2mem(p));
    p = newp;
    VALGRIND_MALLOCLIKE_ఖండం(chunk2mem(p), బైట్లు, కొలత_SZ, false);

    ప్రతిజ్ఞా (newsize >= nb && (((గుర్తులేని దీర్ఘ)(chunk2mem(p))) % alignment) == 0);
  }

  /* Also give back spare room at the end */

  remainder_కొలత = chunksize(p) - nb;

  ఒకవేళ (remainder_కొలత >= (దీర్ఘ)MINSIZE)
  {
    remainder = chunk_at_స్థానభ్రంశం(p, nb);
    సమితి_తల(remainder, remainder_కొలత | PREV_INUSE);
    సమితి_తల_కొలత(p, nb);
    VALGRIND_MALLOCLIKE_ఖండం(chunk2mem(remainder), remainder_కొలత, కొలత_SZ,
			      false);
    fREe_impl(chunk2mem(remainder));
  }

  check_inuse_chunk(p);
  తిరిగి chunk2mem(p);

}

/*
    valloc just invokes memalign with alignment argument equal
    to the page size of the system (or as near to this as can
    be figured out from all the includes/defines above.)
*/

#ఒకవేళ __STD_C
Void_t* vALLOc(size_t బైట్లు)
#లేకపోతే
Void_t* vALLOc(బైట్లు) size_t బైట్లు;
#అయితే_ముగింపు
{
  తిరిగి mEMALIGn (malloc_getpagesize, బైట్లు);
}

/*
  pvalloc just invokes valloc for the nearest pagesize
  that will accommodate request
*/

#ఒకవేళ __STD_C
Void_t* pvALLOc(size_t బైట్లు)
#లేకపోతే
Void_t* pvALLOc(బైట్లు) size_t బైట్లు;
#అయితే_ముగింపు
{
  size_t pagesize = malloc_getpagesize;
  తిరిగి mEMALIGn (pagesize, (బైట్లు + pagesize - 1) & ~(pagesize - 1));
}

/*

  calloc calls malloc, then zeroes out the allocated chunk.

*/

STATIC_IF_MCHECK
#ఒకవేళ __STD_C
Void_t* cALLOc_impl(size_t n, size_t elem_కొలత)
#లేకపోతే
Void_t* cALLOc_impl(n, elem_కొలత) size_t n; size_t elem_కొలత;
#అయితే_ముగింపు
{
  mchunkptr p;
  అంతర్గత_కొలత_T csz;

  అంతర్గత_కొలత_T sz = n * elem_కొలత;

  /* check if expand_top called, in which case don't need to clear */
#ఒకవేళ CONFIG_IS_ENABLED(SYS_MALLOC_CLEAR_ON_INIT)
#ఒకవేళ MORECORE_CLEARS
  mchunkptr oldtop = శిఖరం;
  అంతర్గత_కొలత_T oldtopsize = chunksize(శిఖరం);
#అయితే_ముగింపు
#అయితే_ముగింపు
  Void_t* mem = mALLOc_impl (sz);

  ఒకవేళ ((దీర్ఘ)n < 0) తిరిగి ఖాళీ;

  ఒకవేళ (mem == ఖాళీ)
    తిరిగి ఖాళీ;
  లేకపోతే
  {
#ఒకవేళ CONFIG_IS_ENABLED(SYS_MALLOC_F)
	ఒకవేళ (!(gd->flags & GD_FLG_నిండిన_MALLOC_INIT)) {
		memset(mem, 0, sz);
		తిరిగి mem;
	}
#అయితే_ముగింపు
    p = mem2chunk(mem);

    /* Two optional cases in which clearing not necessary */

#ఒకవేళ HAVE_MMAP
    ఒకవేళ (chunk_is_mmapped(p)) తిరిగి mem;
#అయితే_ముగింపు

    csz = chunksize(p);

#ఒకవేళ CONFIG_IS_ENABLED(SYS_MALLOC_CLEAR_ON_INIT)
#ఒకవేళ MORECORE_CLEARS
    ఒకవేళ (p == oldtop && csz > oldtopsize)
    {
      /* clear only the bytes from non-freshly-sbrked memory */
      csz = oldtopsize;
    }
#అయితే_ముగింపు
#అయితే_ముగింపు

    MALLOC_ZERO(mem, csz - కొలత_SZ);
    VALGRIND_MAKE_MEM_DEFINED(mem, sz);
    తిరిగి mem;
  }
}

/*

  cfree just calls free. It is needed/defined on some systems
  that pair it with calloc, presumably for odd historical reasons.

*/

#ఒకవేళ !defined(అంతర్గత_LINUX_C_LIB) || !defined(__ELF__)
#ఒకవేళ __STD_C
శూన్య cfree(Void_t *mem)
#లేకపోతే
శూన్య cfree(mem) Void_t *mem;
#అయితే_ముగింపు
{
  fREe(mem);
}
#అయితే_ముగింపు

#నిర్వచితమైతే MCHECK_కుప్ప_PROTECTION
 #అన్తర్భావ "mcheck_core.inc.h"
 #ఒకవేళ !__STD_C
  #దోషం "must have __STD_C"
 #అయితే_ముగింపు

Void_t *mALLOc(size_t బైట్లు)
{
	mcheck_pedantic_prehook();
	size_t fullsz = mcheck_alloc_prehook(బైట్లు);
	శూన్య *p = mALLOc_impl(fullsz);

	ఒకవేళ (!p)
		తిరిగి p;
	తిరిగి mcheck_alloc_posthook(p, బైట్లు);
}

శూన్య fREe(Void_t *mem) { fREe_impl(mcheck_free_prehook(mem)); }

Void_t *rEALLOc(Void_t *oldmem, size_t బైట్లు)
{
	mcheck_pedantic_prehook();
	ఒకవేళ (బైట్లు == 0) {
		ఒకవేళ (oldmem)
			fREe(oldmem);
		తిరిగి ఖాళీ;
	}

	ఒకవేళ (oldmem == ఖాళీ)
		తిరిగి mALLOc(బైట్లు);

	శూన్య *p = mcheck_reallocfree_prehook(oldmem);
	size_t newsz = mcheck_alloc_prehook(బైట్లు);

	p = rEALLOc_impl(p, newsz);
	ఒకవేళ (!p)
		తిరిగి p;
	తిరిగి mcheck_alloc_noclean_posthook(p, బైట్లు);
}

Void_t *mEMALIGn(size_t alignment, size_t బైట్లు)
{
	mcheck_pedantic_prehook();
	size_t fullsz = mcheck_memalign_prehook(alignment, బైట్లు);
	శూన్య *p = mEMALIGn_impl(alignment, fullsz);

	ఒకవేళ (!p)
		తిరిగి p;
	తిరిగి mcheck_memalign_posthook(alignment, p, బైట్లు);
}

// pvALLOc, vALLOc - redirect to mEMALIGn, defined here, so they need no wrapping.

Void_t *cALLOc(size_t n, size_t elem_కొలత)
{
	mcheck_pedantic_prehook();
	// NB: here is no overflow check.
	size_t fullsz = mcheck_alloc_prehook(n * elem_కొలత);
	శూన్య *p = cALLOc_impl(1, fullsz);

	ఒకవేళ (!p)
		తిరిగి p;
	తిరిగి mcheck_alloc_noclean_posthook(p, n * elem_కొలత);
}

// mcheck API {
పూర్ణాంకం mcheck_pedantic(mcheck_abortfunc_t f)
{
	mcheck_initialize(f, 1);
	తిరిగి 0;
}

పూర్ణాంకం mcheck(mcheck_abortfunc_t f)
{
	mcheck_initialize(f, 0);
	తిరిగి 0;
}

శూన్య mcheck_check_all(శూన్య) { mcheck_pedantic_check(); }

గణన mcheck_status mprobe(శూన్య *__ptr) { తిరిగి mcheck_mprobe(__ptr); }
// mcheck API }
#అయితే_ముగింపు

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

#ఒకవేళ __STD_C
పూర్ణాంకం malloc_trim(size_t pad)
#లేకపోతే
పూర్ణాంకం malloc_trim(pad) size_t pad;
#అయితే_ముగింపు
{
  దీర్ఘ  శిఖరం_కొలత;        /* Amount of top-most memory */
  దీర్ఘ  extra;           /* Amount to release */
  అక్షరం* current_brk;     /* address returned by pre-check sbrk call */
  అక్షరం* new_brk;         /* address returned by negative sbrk call */

  గుర్తులేని దీర్ఘ pagesz = malloc_getpagesize;

  శిఖరం_కొలత = chunksize(శిఖరం);
  extra = ((శిఖరం_కొలత - pad - MINSIZE + (pagesz-1)) / pagesz - 1) * pagesz;

  ఒకవేళ (extra < (దీర్ఘ)pagesz)  /* Not enough memory to release */
    తిరిగి 0;

  లేకపోతే
  {
    /* Test to make sure no one else called sbrk */
    current_brk = (అక్షరం*)(MORECORE (0));
    ఒకవేళ (current_brk != (అక్షరం*)(శిఖరం) + శిఖరం_కొలత)
      తిరిగి 0;     /* Apparently we don't own memory; must fail */

    లేకపోతే
    {
      new_brk = (అక్షరం*)(MORECORE (-extra));

      ఒకవేళ (new_brk == (అక్షరం*)(MORECORE_FAILURE)) /* sbrk failed? */
      {
	/* Try to figure out what we have */
	current_brk = (అక్షరం*)(MORECORE (0));
	శిఖరం_కొలత = current_brk - (అక్షరం*)శిఖరం;
	ఒకవేళ (శిఖరం_కొలత >= (దీర్ఘ)MINSIZE) /* if not, we are very very dead! */
	{
	  sbrked_mem = current_brk - sbrk_పునాది;
	  సమితి_తల(శిఖరం, శిఖరం_కొలత | PREV_INUSE);
	}
	check_chunk(శిఖరం);
	తిరిగి 0;
      }

      లేకపోతే
      {
	/* Success. Adjust top accordingly. */
	సమితి_తల(శిఖరం, (శిఖరం_కొలత - extra) | PREV_INUSE);
	sbrked_mem -= extra;
	check_chunk(శిఖరం);
	తిరిగి 1;
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

#ఒకవేళ __STD_C
size_t malloc_usable_size(Void_t* mem)
#లేకపోతే
size_t malloc_usable_size(mem) Void_t* mem;
#అయితే_ముగింపు
{
  mchunkptr p;
  ఒకవేళ (mem == ఖాళీ)
    తిరిగి 0;
  లేకపోతే
  {
    p = mem2chunk(mem);
    ఒకవేళ(!chunk_is_mmapped(p))
    {
      ఒకవేళ (!inuse(p)) తిరిగి 0;
      check_inuse_chunk(p);
      తిరిగి chunksize(p) - కొలత_SZ;
    }
    తిరిగి chunksize(p) - 2*కొలత_SZ;
  }
}

/* Utility to update current_mallinfo for malloc_stats and mallinfo() */

#నిర్వచితమైతే DEBUG
స్థిర శూన్య malloc_update_mallinfo(శూన్య)
{
  పూర్ణాంకం i;
  mbinptr b;
  mchunkptr p;
#నిర్వచితమైతే DEBUG
  mchunkptr q;
#అయితే_ముగింపు

  అంతర్గత_కొలత_T avail = chunksize(శిఖరం);
  పూర్ణాంకం   navail = ((దీర్ఘ)(avail) >= (దీర్ఘ)MINSIZE)? 1 : 0;

  కొరకు (i = 1; i < NAV; ++i)
  {
    b = bin_at(i);
    కొరకు (p = చివరి(b); p != b; p = p->bk)
    {
#నిర్వచితమైతే DEBUG
      check_free_chunk(p);
      కొరకు (q = తదుపరి_chunk(p);
	   q < శిఖరం && inuse(q) && (దీర్ఘ)(chunksize(q)) >= (దీర్ఘ)MINSIZE;
	   q = తదుపరి_chunk(q))
	check_inuse_chunk(q);
#అయితే_ముగింపు
      avail += chunksize(p);
      navail++;
    }
  }

  current_mallinfo.ordblks = navail;
  current_mallinfo.uordblks = sbrked_mem - avail;
  current_mallinfo.fordblks = avail;
  current_mallinfo.hblks = n_mmaps;
  current_mallinfo.hblkhd = mmapped_mem;
  current_mallinfo.keepcost = chunksize(శిఖరం);

}
#అయితే_ముగింపు	/* DEBUG */

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

#నిర్వచితమైతే DEBUG
శూన్య malloc_stats(శూన్య)
{
  malloc_update_mallinfo();
  రాయి("max system bytes = %10u\n",
	  (గుర్తులేని పూర్ణాంకం)(max_మొత్తం_mem));
  రాయి("system bytes     = %10u\n",
	  (గుర్తులేని పూర్ణాంకం)(sbrked_mem + mmapped_mem));
  రాయి("in use bytes     = %10u\n",
	  (గుర్తులేని పూర్ణాంకం)(current_mallinfo.uordblks + mmapped_mem));
#ఒకవేళ HAVE_MMAP
  రాయి("max mmap regions = %10u\n",
	  (గుర్తులేని పూర్ణాంకం)max_n_mmaps);
#అయితే_ముగింపు
}
#అయితే_ముగింపు	/* DEBUG */

/*
  mallinfo returns a copy of updated current mallinfo.
*/

#నిర్వచితమైతే DEBUG
నిర్మాణం mallinfo mALLINFo(శూన్య)
{
  malloc_update_mallinfo();
  తిరిగి current_mallinfo;
}
#అయితే_ముగింపు	/* DEBUG */

/*
  mallopt:

    mallopt is the general SVID/XPG interface to tunable parameters.
    The format is to provide a (parameter-number, parameter-value) pair.
    mallopt then sets the corresponding parameter to the argument
    value if it can (i.e., so long as the value is meaningful),
    and returns 1 if successful else 0.

    See descriptions of tunable parameters above.

*/

#ఒకవేళ __STD_C
పూర్ణాంకం mALLOPt(పూర్ణాంకం param_సంఖ్య, పూర్ణాంకం విలువ)
#లేకపోతే
పూర్ణాంకం mALLOPt(param_సంఖ్య, విలువ) పూర్ణాంకం param_సంఖ్య; పూర్ణాంకం విలువ;
#అయితే_ముగింపు
{
  ఎంపిక(param_సంఖ్య)
  {
    స్థితి M_TRIM_THRESHOLD:
      trim_పరిమితి = విలువ; తిరిగి 1;
    స్థితి M_TOP_PAD:
      శిఖరం_pad = విలువ; తిరిగి 1;
    స్థితి M_MMAP_THRESHOLD:
      mmap_పరిమితి = విలువ; తిరిగి 1;
    స్థితి M_MMAP_MAX:
#ఒకవేళ HAVE_MMAP
      n_mmaps_max = విలువ; తిరిగి 1;
#లేకపోతే
      ఒకవేళ (విలువ != 0) తిరిగి 0; లేకపోతే  n_mmaps_max = విలువ; తిరిగి 1;
#అయితే_ముగింపు

    అప్రమేయ:
      తిరిగి 0;
  }
}

పూర్ణాంకం initf_malloc(శూన్య)
{
#ఒకవేళ CONFIG_IS_ENABLED(SYS_MALLOC_F)
	ప్రతిజ్ఞా(gd->malloc_పునాది);	/* Set up by crt0.S */
	gd->malloc_హద్దు = CONFIG_VAL(SYS_MALLOC_F_LEN);
	gd->malloc_ptr = 0;
#అయితే_ముగింపు

	తిరిగి 0;
}

శూన్య malloc_enable_testing(పూర్ణాంకం max_allocs)
{
	malloc_testing = true;
	malloc_max_allocs = max_allocs;
}

శూన్య malloc_disable_testing(శూన్య)
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
