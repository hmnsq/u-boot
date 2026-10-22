// SPDX-License-Identifier: GPL-2.0+
/*
 * This code is based on a version (aka dlmalloc) of malloc/free/realloc written
 * by Doug Lea and released to the public domain, as explained at
 * http://creativecommons.org/publicdomain/zero/1.0/-
 *
 * The original code is available at http://gee.cs.oswego.edu/pub/misc/
 * as file malloc-2.6.6.c.
 */

#ਜੇ CONFIG_IS_ENABLED(UNIT_TEST)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ DEBUG
#ਜੇ_ਅੰਤ

#ਅਨ੍ਤਰ੍ਭਾਵ <log.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/global_data.h>

#ਅਨ੍ਤਰ੍ਭਾਵ <malloc.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <mapmem.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <string.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <asm/io.h>
#ਅਨ੍ਤਰ੍ਭਾਵ <valgrind/memcheck.h>

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ DEBUG
#ਜੇ __STD_C
ਸਥਿਰ ਖਾਲੀ malloc_update_mallinfo (ਖਾਲੀ);
ਖਾਲੀ malloc_stats (ਖਾਲੀ);
#ਨਹੀਂਤਾਂ
ਸਥਿਰ ਖਾਲੀ malloc_update_mallinfo ();
ਖਾਲੀ malloc_stats();
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ	/* DEBUG */

DECLARE_GLOBAL_ਅੰਕੜੇ_PTR;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ MCHECK_ਢੇਰ_PROTECTION
 #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ STATIC_IF_MCHECK ਸਥਿਰ
 #ਪਰਿਭਾਸ਼ਾ_ਰੱਦ MALLOC_COPY
 #ਪਰਿਭਾਸ਼ਾ_ਰੱਦ MALLOC_ZERO
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ MALLOC_ZERO(ਖਾਲੀ *p, size_t sz) { memset(p, 0, sz); }
ਸਥਿਰ ਸਤਰਅੰਦਰ ਖਾਲੀ MALLOC_COPY(ਖਾਲੀ *dest, ਅਚਲ ਖਾਲੀ *src, size_t sz) { ਸਮ੍ਰਿਤੀ_ਨਕਲ(dest, src, sz); }
#ਨਹੀਂਤਾਂ
 #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ STATIC_IF_MCHECK
 #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ mALLOc_impl mALLOc
 #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ fREe_impl fREe
 #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ rEALLOc_impl rEALLOc
 #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ mEMALIGn_impl mEMALIGn
 #ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ cALLOc_impl cALLOc
#ਜੇ_ਅੰਤ

/*
  Emulation of sbrk for WIN32
  All code within the ifdef WIN32 is untested by me.

  Thanks to Martin Fong and others for supplying this.
*/

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ WIN32

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ Align_ਪੰਨਾ(ਜੋੜੋ) (((ਜੋੜੋ) + (malloc_getpagesize-1)) & \
~(malloc_getpagesize-1))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ AlignPage64K(ਜੋੜੋ) (((ਜੋੜੋ) + (0x10000 - 1)) & ~(0x10000 - 1))

/* resrve 64MB to insure large contiguous space */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RESERVED_ਮਾਪ (1024*1024*64)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਅਗਲਾ_ਮਾਪ (2048*1024)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸਿਖਰ_ਸਮ੍ਰਿਤੀ ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)2*1024*1024*1024)

ਬਣਤਰ GmListElement;
ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ GmListElement GmListElement;

ਬਣਤਰ GmListElement
{
	GmListElement* ਅਗਲਾ;
	ਖਾਲੀ* ਅਧਾਰ;
};

ਸਥਿਰ GmListElement* ਸਿਰ = 0;
ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ g_ਅਗਲਾ_ਪਤਾ = 0;
ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ g_ਪਤਾ_ਅਧਾਰ = 0;
ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ g_Allocated_ਮਾਪ = 0;

ਸਥਿਰ
GmListElement* makeGmListElement (ਖਾਲੀ* bas)
{
	GmListElement* this;
	this = (GmListElement*)(ਖਾਲੀ*)LocalAlloc (0, ਆਕਾਰ (GmListElement));
	ਪ੍ਰਤਿਜ੍ਞਾ (this);
	ਜੇ (this)
	{
		this->ਅਧਾਰ = bas;
		this->ਅਗਲਾ = ਸਿਰ;
		ਸਿਰ = this;
	}
	ਵਾਪਸ this;
}

ਖਾਲੀ gcleanup (ਖਾਲੀ)
{
	BOOL rval;
	ਪ੍ਰਤਿਜ੍ਞਾ ( (ਸਿਰ == ਨਿਰਾ) || (ਸਿਰ->ਅਧਾਰ == (ਖਾਲੀ*)g_ਪਤਾ_ਅਧਾਰ));
	ਜੇ (g_ਪਤਾ_ਅਧਾਰ && (g_ਅਗਲਾ_ਪਤਾ - g_ਪਤਾ_ਅਧਾਰ))
	{
		rval = VirtualFree ((ਖਾਲੀ*)g_ਪਤਾ_ਅਧਾਰ,
							g_ਅਗਲਾ_ਪਤਾ - g_ਪਤਾ_ਅਧਾਰ,
							MEM_DECOMMIT);
	ਪ੍ਰਤਿਜ੍ਞਾ (rval);
	}
	ਜਦੋਂ (ਸਿਰ)
	{
		GmListElement* ਅਗਲਾ = ਸਿਰ->ਅਗਲਾ;
		rval = VirtualFree (ਸਿਰ->ਅਧਾਰ, 0, MEM_RELEASE);
		ਪ੍ਰਤਿਜ੍ਞਾ (rval);
		LocalFree (ਸਿਰ);
		ਸਿਰ = ਅਗਲਾ;
	}
}

ਸਥਿਰ
ਖਾਲੀ* ਲੱਭੋ_Region (ਖਾਲੀ* ਸ਼ੁਰੂ_ਪਤਾ, ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਮਾਪ)
{
	ਸਮ੍ਰਿਤੀ_BASIC_INFORMATION info;
	ਜੇ (ਮਾਪ >= ਸਿਖਰ_ਸਮ੍ਰਿਤੀ) ਵਾਪਸ ਨਿਰਾ;

	ਜਦੋਂ ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)ਸ਼ੁਰੂ_ਪਤਾ + ਮਾਪ < ਸਿਖਰ_ਸਮ੍ਰਿਤੀ)
	{
		VirtualQuery (ਸ਼ੁਰੂ_ਪਤਾ, &info, ਆਕਾਰ (info));
		ਜੇ ((info.State == MEM_FREE) && (info.Region_ਮਾਪ >= ਮਾਪ))
			ਵਾਪਸ ਸ਼ੁਰੂ_ਪਤਾ;
		ਨਹੀਂਤਾਂ
		{
			/* Requested region is not available so see if the */
			/* next region is available.  Set 'start_address' */
			/* to the next region and call 'VirtualQuery()' */
			/* again. */

			ਸ਼ੁਰੂ_ਪਤਾ = (ਅੱਖਰ*)info.ਅਧਾਰ_ਪਤਾ + info.Region_ਮਾਪ;

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

			ਸ਼ੁਰੂ_ਪਤਾ =
				(ਖਾਲੀ *) AlignPage64K((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ) ਸ਼ੁਰੂ_ਪਤਾ);
		}
	}
	ਵਾਪਸ ਨਿਰਾ;

}

ਖਾਲੀ* wsbrk (ਲੰਮਾ ਮਾਪ)
{
	ਖਾਲੀ* tmp;
	ਜੇ (ਮਾਪ > 0)
	{
		ਜੇ (g_ਪਤਾ_ਅਧਾਰ == 0)
		{
			g_Allocated_ਮਾਪ = max (RESERVED_ਮਾਪ, Align_ਪੰਨਾ (ਮਾਪ));
			g_ਅਗਲਾ_ਪਤਾ = g_ਪਤਾ_ਅਧਾਰ =
				(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)VirtualAlloc (ਨਿਰਾ, g_Allocated_ਮਾਪ,
											MEM_RESERVE, ਪੰਨਾ_NOACCESS);
		} ਨਹੀਂਤਾਂ ਜੇ (Align_ਪੰਨਾ (g_ਅਗਲਾ_ਪਤਾ + ਮਾਪ) > (g_ਪਤਾ_ਅਧਾਰ +
g_Allocated_ਮਾਪ))
		{
			ਲੰਮਾ new_ਮਾਪ = max (ਅਗਲਾ_ਮਾਪ, Align_ਪੰਨਾ (ਮਾਪ));
			ਖਾਲੀ* new_ਪਤਾ = (ਖਾਲੀ*)(g_ਪਤਾ_ਅਧਾਰ+g_Allocated_ਮਾਪ);
			ਕਰੋ
			{
				new_ਪਤਾ = ਲੱਭੋ_Region (new_ਪਤਾ, new_ਮਾਪ);

				ਜੇ (!new_ਪਤਾ)
					ਵਾਪਸ (ਖਾਲੀ*)-1;

				g_ਪਤਾ_ਅਧਾਰ = g_ਅਗਲਾ_ਪਤਾ =
					(ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)VirtualAlloc (new_ਪਤਾ, new_ਮਾਪ,
												MEM_RESERVE, ਪੰਨਾ_NOACCESS);
				/* repeat in case of race condition */
				/* The region that we found has been snagged */
				/* by another thread */
			}
			ਜਦੋਂ (g_ਪਤਾ_ਅਧਾਰ == 0);

			ਪ੍ਰਤਿਜ੍ਞਾ (new_ਪਤਾ == (ਖਾਲੀ*)g_ਪਤਾ_ਅਧਾਰ);

			g_Allocated_ਮਾਪ = new_ਮਾਪ;

			ਜੇ (!makeGmListElement ((ਖਾਲੀ*)g_ਪਤਾ_ਅਧਾਰ))
				ਵਾਪਸ (ਖਾਲੀ*)-1;
		}
		ਜੇ ((ਮਾਪ + g_ਅਗਲਾ_ਪਤਾ) > Align_ਪੰਨਾ (g_ਅਗਲਾ_ਪਤਾ))
		{
			ਖਾਲੀ* res;
			res = VirtualAlloc ((ਖਾਲੀ*)Align_ਪੰਨਾ (g_ਅਗਲਾ_ਪਤਾ),
								(ਮਾਪ + g_ਅਗਲਾ_ਪਤਾ -
								 Align_ਪੰਨਾ (g_ਅਗਲਾ_ਪਤਾ)),
								MEM_COMMIT, ਪੰਨਾ_READWRITE);
			ਜੇ (!res)
				ਵਾਪਸ (ਖਾਲੀ*)-1;
		}
		tmp = (ਖਾਲੀ*)g_ਅਗਲਾ_ਪਤਾ;
		g_ਅਗਲਾ_ਪਤਾ = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)tmp + ਮਾਪ;
		ਵਾਪਸ tmp;
	}
	ਨਹੀਂਤਾਂ ਜੇ (ਮਾਪ < 0)
	{
		ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ alignedGoal = Align_ਪੰਨਾ (g_ਅਗਲਾ_ਪਤਾ + ਮਾਪ);
		/* Trim by releasing the virtual memory */
		ਜੇ (alignedGoal >= g_ਪਤਾ_ਅਧਾਰ)
		{
			VirtualFree ((ਖਾਲੀ*)alignedGoal, g_ਅਗਲਾ_ਪਤਾ - alignedGoal,
						 MEM_DECOMMIT);
			g_ਅਗਲਾ_ਪਤਾ = g_ਅਗਲਾ_ਪਤਾ + ਮਾਪ;
			ਵਾਪਸ (ਖਾਲੀ*)g_ਅਗਲਾ_ਪਤਾ;
		}
		ਨਹੀਂਤਾਂ
		{
			VirtualFree ((ਖਾਲੀ*)g_ਪਤਾ_ਅਧਾਰ, g_ਅਗਲਾ_ਪਤਾ - g_ਪਤਾ_ਅਧਾਰ,
						 MEM_DECOMMIT);
			g_ਅਗਲਾ_ਪਤਾ = g_ਪਤਾ_ਅਧਾਰ;
			ਵਾਪਸ (ਖਾਲੀ*)-1;
		}
	}
	ਨਹੀਂਤਾਂ
	{
		ਵਾਪਸ (ਖਾਲੀ*)g_ਅਗਲਾ_ਪਤਾ;
	}
}

#ਜੇ_ਅੰਤ

/*
  Type declarations
*/

ਬਣਤਰ malloc_chunk
{
  ਅੰਦਰੂਨੀ_ਮਾਪ_T prev_ਮਾਪ; /* Size of previous chunk (if free). */
  ਅੰਦਰੂਨੀ_ਮਾਪ_T ਮਾਪ;      /* Size in bytes, including overhead. */
  ਬਣਤਰ malloc_chunk* fd;   /* double links -- used only if free. */
  ਬਣਤਰ malloc_chunk* bk;
} __attribute__((__may_alias__)) ;

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ malloc_chunk* mchunkptr;

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

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਮਾਪ_SZ                (ਆਕਾਰ(ਅੰਦਰੂਨੀ_ਮਾਪ_T))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MALLOC_ALIGNMENT       (ਮਾਪ_SZ + ਮਾਪ_SZ)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MALLOC_ALIGN_ਨਕਾਬ      (MALLOC_ALIGNMENT - 1)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MINSIZE                (ਆਕਾਰ(ਬਣਤਰ malloc_chunk))

/* conversion from malloc headers to user pointers, and back */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ chunk2mem(p)   ((Void_t*)((ਅੱਖਰ*)(p) + 2*ਮਾਪ_SZ))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ mem2chunk(mem) ((mchunkptr)((ਅੱਖਰ*)(mem) - 2*ਮਾਪ_SZ))

/* pad request bytes into a usable size */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ request2size(req) \
 ((((req) + (ਮਾਪ_SZ + MALLOC_ALIGN_ਨਕਾਬ)) < \
  (MINSIZE + MALLOC_ALIGN_ਨਕਾਬ)) ? MINSIZE : \
   (((req) + (ਮਾਪ_SZ + MALLOC_ALIGN_ਨਕਾਬ)) & ~(MALLOC_ALIGN_ਨਕਾਬ)))

/* Check if m has acceptable alignment */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ aligned_OK(m)    (((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)((m)) & (MALLOC_ALIGN_ਨਕਾਬ)) == 0)

/*
  Physical chunk operations
*/

/* size field is or'ed with PREV_INUSE when previous adjacent chunk in use */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ PREV_INUSE 0x1

/* size field is or'ed with IS_MMAPPED if the chunk was obtained with mmap() */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ IS_MMAPPED 0x2

/* Bits to mask off when extracting size */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਮਾਪ_ਬਿੱਟ (PREV_INUSE|IS_MMAPPED)

/* Ptr to next physical malloc_chunk. */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਅਗਲਾ_chunk(p) ((mchunkptr)( ((ਅੱਖਰ*)(p)) + ((p)->ਮਾਪ & ~PREV_INUSE) ))

/* Ptr to previous physical malloc_chunk */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ prev_chunk(p)\
   ((mchunkptr)( ((ਅੱਖਰ*)(p)) - ((p)->prev_ਮਾਪ) ))

/* Treat space at ptr + offset as a chunk */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ chunk_at_ਵਿੱਥ(p, s)  ((mchunkptr)(((ਅੱਖਰ*)(p)) + (s)))

/*
  Dealing with use bits
*/

/* extract p's inuse bit */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ inuse(p)\
((((mchunkptr)(((ਅੱਖਰ*)(p))+((p)->ਮਾਪ & ~PREV_INUSE)))->ਮਾਪ) & PREV_INUSE)

/* extract inuse bit of previous chunk */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ prev_inuse(p)  ((p)->ਮਾਪ & PREV_INUSE)

/* check for mmap()'ed chunk */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ chunk_is_mmapped(p) ((p)->ਮਾਪ & IS_MMAPPED)

/* set/clear chunk as in use without otherwise disturbing */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸੈੱਟ_inuse(p)\
((mchunkptr)(((ਅੱਖਰ*)(p)) + ((p)->ਮਾਪ & ~PREV_INUSE)))->ਮਾਪ |= PREV_INUSE

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ clear_inuse(p)\
((mchunkptr)(((ਅੱਖਰ*)(p)) + ((p)->ਮਾਪ & ~PREV_INUSE)))->ਮਾਪ &= ~(PREV_INUSE)

/* check/set/clear inuse bits in known places */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ inuse_ਦੋਆਧਾਰੀ_ਅੰਕ_at_ਵਿੱਥ(p, s)\
 (((mchunkptr)(((ਅੱਖਰ*)(p)) + (s)))->ਮਾਪ & PREV_INUSE)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸੈੱਟ_inuse_ਦੋਆਧਾਰੀ_ਅੰਕ_at_ਵਿੱਥ(p, s)\
 (((mchunkptr)(((ਅੱਖਰ*)(p)) + (s)))->ਮਾਪ |= PREV_INUSE)

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ clear_inuse_ਦੋਆਧਾਰੀ_ਅੰਕ_at_ਵਿੱਥ(p, s)\
 (((mchunkptr)(((ਅੱਖਰ*)(p)) + (s)))->ਮਾਪ &= ~(PREV_INUSE))

/*
  Dealing with size fields
*/

/* Get size, ignoring use bits */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ chunksize(p)          ((p)->ਮਾਪ & ~(ਮਾਪ_ਬਿੱਟ))

/* Set size at head, without disturbing its use bit */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸੈੱਟ_ਸਿਰ_ਮਾਪ(p, s)   ((p)->ਮਾਪ = (((p)->ਮਾਪ & PREV_INUSE) | (s)))

/* Set size/use ignoring previous bits in header */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸੈੱਟ_ਸਿਰ(p, s)        ((p)->ਮਾਪ = (s))

/* Set size at footer (only when chunk is not in use) */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸੈੱਟ_foot(p, s)   (((mchunkptr)((ਅੱਖਰ*)(p) + (s)))->prev_ਮਾਪ = (s))

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

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ NAV             128   /* number of bins */

ਕਿਸਮਪਰਿਭਾਸ਼ਾ ਬਣਤਰ malloc_chunk* mbinptr;

/* access macros */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ bin_at(i)      ((mbinptr)((ਅੱਖਰ*)&(av_[2*(i) + 2]) - 2*ਮਾਪ_SZ))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਅਗਲਾ_bin(b)    ((mbinptr)((ਅੱਖਰ*)(b) + 2 * ਆਕਾਰ(mbinptr)))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ prev_bin(b)    ((mbinptr)((ਅੱਖਰ*)(b) - 2 * ਆਕਾਰ(mbinptr)))

/*
   The first 2 bins are never indexed. The corresponding av_ cells are instead
   used for bookkeeping. This is not to save space, but to simplify
   indexing, maintain locality, and avoid some initialization tests.
*/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਸਿਖਰ            (av_[2])          /* The topmost chunk */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਆਖਰੀ_remainder (bin_at(1))       /* remainder from last split */

/*
   Because top initially points to its own bin with initial
   zero size, thus forcing extension on the first malloc request,
   we avoid having any special code in malloc to check whether
   it even exists yet. But we still need to in malloc_extend_top.
*/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ initial_ਸਿਖਰ    ((mchunkptr)(bin_at(0)))

/* Helper macro to initialize bins */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ IAV(i)  bin_at(i), bin_at(i)

ਸਥਿਰ mbinptr av_[NAV * 2 + 2] = {
 ਨਿਰਾ, ਨਿਰਾ,
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

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SYS_MALLOC_DEFAULT_ਮੰਜ਼ਿਲ_INIT
ਸਥਿਰ ਖਾਲੀ malloc_init(ਖਾਲੀ);
#ਜੇ_ਅੰਤ

ulong mem_malloc_ਸ਼ੁਰੂ = 0;
ulong mem_malloc_ਸਿਰਾ = 0;
ulong mem_malloc_brk = 0;

ਸਥਿਰ bool malloc_testing;	/* enable test mode */
ਸਥਿਰ ਪੂਰਨ malloc_max_allocs;	/* return NULL after this many calls to malloc() */

ਖਾਲੀ *sbrk(ptrdiff_t increment)
{
	ulong old = mem_malloc_brk;
	ulong new = old + increment;

	ਜੇ ((new < mem_malloc_ਸ਼ੁਰੂ) || (new > mem_malloc_ਸਿਰਾ))
		ਵਾਪਸ (ਖਾਲੀ *)MORECORE_FAILURE;

	/*
	 * if we are giving memory back make sure we clear it out since
	 * we set MORECORE_CLEARS to 1
	 */
	ਜੇ (increment < 0)
		memset((ਖਾਲੀ *)new, 0, -increment);

	mem_malloc_brk = new;

	ਵਾਪਸ (ਖਾਲੀ *)old;
}

ਖਾਲੀ mem_malloc_init(ulong ਸ਼ੁਰੂ, ulong ਮਾਪ)
{
	mem_malloc_ਸ਼ੁਰੂ = (ulong)map_sysmem(ਸ਼ੁਰੂ, ਮਾਪ);
	mem_malloc_ਸਿਰਾ = mem_malloc_ਸ਼ੁਰੂ + ਮਾਪ;
	mem_malloc_brk = mem_malloc_ਸ਼ੁਰੂ;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SYS_MALLOC_DEFAULT_ਮੰਜ਼ਿਲ_INIT
	malloc_init();
#ਜੇ_ਅੰਤ

	debug("using memory %#lx-%#lx for malloc()\n", mem_malloc_ਸ਼ੁਰੂ,
	      mem_malloc_ਸਿਰਾ);
#ਜੇ CONFIG_IS_ENABLED(SYS_MALLOC_CLEAR_ON_INIT)
	memset((ਖਾਲੀ *)mem_malloc_ਸ਼ੁਰੂ, 0x0, ਮਾਪ);
#ਜੇ_ਅੰਤ
}

/* field-extraction macros */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਪਹਿਲਾ(b) ((b)->fd)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਆਖਰੀ(b)  ((b)->bk)

/*
  Indexing into bins
*/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ bin_ਸੂਚਕਾਂਕ(sz)                                                          \
(((((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)(sz)) >> 9) ==    0) ?       (((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)(sz)) >>  3): \
 ((((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)(sz)) >> 9) <=    4) ?  56 + (((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)(sz)) >>  6): \
 ((((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)(sz)) >> 9) <=   20) ?  91 + (((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)(sz)) >>  9): \
 ((((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)(sz)) >> 9) <=   84) ? 110 + (((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)(sz)) >> 12): \
 ((((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)(sz)) >> 9) <=  340) ? 119 + (((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)(sz)) >> 15): \
 ((((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)(sz)) >> 9) <= 1364) ? 124 + (((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)(sz)) >> 18): \
					  126)
/*
  bins for chunks < 512 are all spaced 8 bytes apart, and hold
  identically sized chunks. This is exploited in malloc.
*/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAX_SMALLBIN         63
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ MAX_SMALLBIN_ਮਾਪ   512
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ SMALLBIN_ਚੌੜਾਈ        8

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ smallbin_ਸੂਚਕਾਂਕ(sz)  (((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)(sz)) >> 3)

/*
   Requests are `small' if both the corresponding and the next bin are small
*/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ is_small_request(nb) (nb < MAX_SMALLBIN_ਮਾਪ - SMALLBIN_ਚੌੜਾਈ)

/*
    To help compensate for the large number of bins, a one-level index
    structure is used for bin-by-bin searching.  `binblocks' is a
    one-word bitvector recording whether groups of BINBLOCKWIDTH bins
    have any (possibly) non-empty bins, so they can be skipped over
    all at once during during traversals. The bits are NOT always
    cleared as soon as all bins in a block are empty, but instead only
    when all are noticed to be empty during traversal in malloc.
*/

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ BINBLOCKWIDTH     4   /* bins per block */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ binblocks_r     ((ਅੰਦਰੂਨੀ_ਮਾਪ_T)av_[1]) /* bitvector of nonempty blocks */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ binblocks_w     (av_[1])

/* bin<->block macros */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ idx2binblock(ix)    ((ਬਿਨਾਨਿਸ਼ਾਨੀ)1 << (ix / BINBLOCKWIDTH))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ mark_binblock(ii)   (binblocks_w = (mbinptr)(binblocks_r | idx2binblock(ii)))
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ clear_binblock(ii)  (binblocks_w = (mbinptr)(binblocks_r & ~(idx2binblock(ii))))

/*  Other static bookkeeping data */

/* variables holding tunable values */

ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ trim_ਹੱਦ   = DEFAULT_TRIM_ਹੱਦ;
ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਸਿਖਰ_pad          = DEFAULT_ਸਿਖਰ_PAD;
ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ  n_mmaps_max      = DEFAULT_MMAP_MAX;
ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ mmap_ਹੱਦ   = DEFAULT_MMAP_ਹੱਦ;

/* The first value returned from sbrk */
ਸਥਿਰ ਅੱਖਰ* sbrk_ਅਧਾਰ = (ਅੱਖਰ*)(-1);

/* The maximum memory obtained from system via sbrk */
ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ max_sbrked_mem = 0;

/* The maximum via either sbrk or mmap */
ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ max_ਕੁੱਲ_mem = 0;

/* internal working copy of mallinfo */
ਸਥਿਰ ਬਣਤਰ mallinfo current_mallinfo = {  0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

/* The total memory obtained from system via sbrk */
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ sbrked_mem  (current_mallinfo.arena)

/* Tracking mmaps */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ DEBUG
ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ n_mmaps = 0;
#ਜੇ_ਅੰਤ	/* DEBUG */
ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ mmapped_mem = 0;
#ਜੇ HAVE_MMAP
ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ max_n_mmaps = 0;
ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ max_mmapped_mem = 0;
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ CONFIG_SYS_MALLOC_DEFAULT_ਮੰਜ਼ਿਲ_INIT
ਸਥਿਰ ਖਾਲੀ malloc_init(ਖਾਲੀ)
{
	ਪੂਰਨ i, j;

	debug("bins (av_ array) are at %p\n", (ਖਾਲੀ *)av_);

	av_[0] = ਨਿਰਾ; av_[1] = ਨਿਰਾ;
	ਲਈ (i = 2, j = 2; i < NAV * 2 + 2; i += 2, j++) {
		av_[i] = bin_at(j - 2);
		av_[i + 1] = bin_at(j - 2);

		/* Just print the first few bins so that
		 * we can see there are alright.
		 */
		ਜੇ (i < 10)
			debug("av_[%d]=%lx av_[%d]=%lx\n",
			      i, (ulong)av_[i],
			      i + 1, (ulong)av_[i + 1]);
	}

	/* Init the static bookkeeping as well */
	sbrk_ਅਧਾਰ = (ਅੱਖਰ *)(-1);
	max_sbrked_mem = 0;
	max_ਕੁੱਲ_mem = 0;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ DEBUG
	memset((ਖਾਲੀ *)&current_mallinfo, 0, ਆਕਾਰ(ਬਣਤਰ mallinfo));
#ਜੇ_ਅੰਤ
}
#ਜੇ_ਅੰਤ

/*
  Debugging support
*/

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ DEBUG

/*
  These routines make a number of assertions about the states
  of data structures that should be true at all times. If any
  are not true, it's very likely that a user program has somehow
  trashed memory. (It's also possible that there is a coding error
  in malloc. In which case, please report it!)
*/

#ਜੇ __STD_C
ਸਥਿਰ ਖਾਲੀ do_check_chunk(mchunkptr p)
#ਨਹੀਂਤਾਂ
ਸਥਿਰ ਖਾਲੀ do_check_chunk(p) mchunkptr p;
#ਜੇ_ਅੰਤ
{
  ਅੰਦਰੂਨੀ_ਮਾਪ_T sz = p->ਮਾਪ & ~PREV_INUSE;

  /* No checkable chunk is mmapped */
  ਪ੍ਰਤਿਜ੍ਞਾ(!chunk_is_mmapped(p));

  /* Check for legal address ... */
  ਪ੍ਰਤਿਜ੍ਞਾ((ਅੱਖਰ*)p >= sbrk_ਅਧਾਰ);
  ਜੇ (p != ਸਿਖਰ)
    ਪ੍ਰਤਿਜ੍ਞਾ((ਅੱਖਰ*)p + sz <= (ਅੱਖਰ*)ਸਿਖਰ);
  ਨਹੀਂਤਾਂ
    ਪ੍ਰਤਿਜ੍ਞਾ((ਅੱਖਰ*)p + sz <= sbrk_ਅਧਾਰ + sbrked_mem);

}

#ਜੇ __STD_C
ਸਥਿਰ ਖਾਲੀ do_check_free_chunk(mchunkptr p)
#ਨਹੀਂਤਾਂ
ਸਥਿਰ ਖਾਲੀ do_check_free_chunk(p) mchunkptr p;
#ਜੇ_ਅੰਤ
{
  ਅੰਦਰੂਨੀ_ਮਾਪ_T sz = p->ਮਾਪ & ~PREV_INUSE;
  mchunkptr ਅਗਲਾ = chunk_at_ਵਿੱਥ(p, sz);

  do_check_chunk(p);

  /* Check whether it claims to be free ... */
  ਪ੍ਰਤਿਜ੍ਞਾ(!inuse(p));

  /* Unless a special marker, must have OK fields */
  ਜੇ ((ਲੰਮਾ)sz >= (ਲੰਮਾ)MINSIZE)
  {
    ਪ੍ਰਤਿਜ੍ਞਾ((sz & MALLOC_ALIGN_ਨਕਾਬ) == 0);
    ਪ੍ਰਤਿਜ੍ਞਾ(aligned_OK(chunk2mem(p)));
    /* ... matching footer field */
    ਪ੍ਰਤਿਜ੍ਞਾ(ਅਗਲਾ->prev_ਮਾਪ == sz);
    /* ... and is fully consolidated */
    ਪ੍ਰਤਿਜ੍ਞਾ(prev_inuse(p));
    ਪ੍ਰਤਿਜ੍ਞਾ (ਅਗਲਾ == ਸਿਖਰ || inuse(ਅਗਲਾ));

    /* ... and has minimally sane links */
    ਪ੍ਰਤਿਜ੍ਞਾ(p->fd->bk == p);
    ਪ੍ਰਤਿਜ੍ਞਾ(p->bk->fd == p);
  }
  ਨਹੀਂਤਾਂ /* markers are always of size SIZE_SZ */
    ਪ੍ਰਤਿਜ੍ਞਾ(sz == ਮਾਪ_SZ);
}

#ਜੇ __STD_C
ਸਥਿਰ ਖਾਲੀ do_check_inuse_chunk(mchunkptr p)
#ਨਹੀਂਤਾਂ
ਸਥਿਰ ਖਾਲੀ do_check_inuse_chunk(p) mchunkptr p;
#ਜੇ_ਅੰਤ
{
  mchunkptr ਅਗਲਾ = ਅਗਲਾ_chunk(p);
  do_check_chunk(p);

  /* Check whether it claims to be in use ... */
  ਪ੍ਰਤਿਜ੍ਞਾ(inuse(p));

  /* ... and is surrounded by OK chunks.
    Since more things can be checked with free chunks than inuse ones,
    if an inuse chunk borders them and debug is on, it's worth doing them.
  */
  ਜੇ (!prev_inuse(p))
  {
    mchunkptr prv = prev_chunk(p);
    ਪ੍ਰਤਿਜ੍ਞਾ(ਅਗਲਾ_chunk(prv) == p);
    do_check_free_chunk(prv);
  }
  ਜੇ (ਅਗਲਾ == ਸਿਖਰ)
  {
    ਪ੍ਰਤਿਜ੍ਞਾ(prev_inuse(ਅਗਲਾ));
    ਪ੍ਰਤਿਜ੍ਞਾ(chunksize(ਅਗਲਾ) >= MINSIZE);
  }
  ਨਹੀਂਤਾਂ ਜੇ (!inuse(ਅਗਲਾ))
    do_check_free_chunk(ਅਗਲਾ);

}

#ਜੇ __STD_C
ਸਥਿਰ ਖਾਲੀ do_check_malloced_chunk(mchunkptr p, ਅੰਦਰੂਨੀ_ਮਾਪ_T s)
#ਨਹੀਂਤਾਂ
ਸਥਿਰ ਖਾਲੀ do_check_malloced_chunk(p, s) mchunkptr p; ਅੰਦਰੂਨੀ_ਮਾਪ_T s;
#ਜੇ_ਅੰਤ
{
  ਅੰਦਰੂਨੀ_ਮਾਪ_T sz = p->ਮਾਪ & ~PREV_INUSE;
  ਲੰਮਾ ਗੁੰਜਾਇਸ਼ = sz - s;

  do_check_inuse_chunk(p);

  /* Legal size ... */
  ਪ੍ਰਤਿਜ੍ਞਾ((ਲੰਮਾ)sz >= (ਲੰਮਾ)MINSIZE);
  ਪ੍ਰਤਿਜ੍ਞਾ((sz & MALLOC_ALIGN_ਨਕਾਬ) == 0);
  ਪ੍ਰਤਿਜ੍ਞਾ(ਗੁੰਜਾਇਸ਼ >= 0);
  ਪ੍ਰਤਿਜ੍ਞਾ(ਗੁੰਜਾਇਸ਼ < (ਲੰਮਾ)MINSIZE);

  /* ... and alignment */
  ਪ੍ਰਤਿਜ੍ਞਾ(aligned_OK(chunk2mem(p)));

  /* ... and was allocated at front of an available chunk */
  ਪ੍ਰਤਿਜ੍ਞਾ(prev_inuse(p));

}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ check_free_chunk(P)  do_check_free_chunk(P)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ check_inuse_chunk(P) do_check_inuse_chunk(P)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ check_chunk(P) do_check_chunk(P)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ check_malloced_chunk(P,N) do_check_malloced_chunk(P,N)
#ਨਹੀਂਤਾਂ
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ check_free_chunk(P)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ check_inuse_chunk(P)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ check_chunk(P)
#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ check_malloced_chunk(P,N)
#ਜੇ_ਅੰਤ

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

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ frontlink(P, S, IDX, BK, FD)                                          \
{                                                                             \
  ਜੇ (S < MAX_SMALLBIN_ਮਾਪ)                                                  \
  {                                                                           \
    IDX = smallbin_ਸੂਚਕਾਂਕ(S);                                                  \
    mark_binblock(IDX);                                                       \
    BK = bin_at(IDX);                                                         \
    FD = BK->fd;                                                              \
    P->bk = BK;                                                               \
    P->fd = FD;                                                               \
    FD->bk = BK->fd = P;                                                      \
  }                                                                           \
  ਨਹੀਂਤਾਂ                                                                        \
  {                                                                           \
    IDX = bin_ਸੂਚਕਾਂਕ(S);                                                       \
    BK = bin_at(IDX);                                                         \
    FD = BK->fd;                                                              \
    ਜੇ (FD == BK) mark_binblock(IDX);                                         \
    ਨਹੀਂਤਾਂ                                                                      \
    {                                                                         \
      ਜਦੋਂ (FD != BK && S < chunksize(FD)) FD = FD->fd;                      \
      BK = FD->bk;                                                            \
    }                                                                         \
    P->bk = BK;                                                               \
    P->fd = FD;                                                               \
    FD->bk = BK->fd = P;                                                      \
  }                                                                           \
}

/* take a chunk off a list */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ unlink(P, BK, FD)                                                     \
{                                                                             \
  BK = P->bk;                                                                 \
  FD = P->fd;                                                                 \
  FD->bk = BK;                                                                \
  BK->fd = FD;                                                                \
}                                                                             \

/* Place p as the last remainder */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ link_ਆਖਰੀ_remainder(P)                                                \
{                                                                             \
  ਆਖਰੀ_remainder->fd = ਆਖਰੀ_remainder->bk =  P;                               \
  P->fd = P->bk = ਆਖਰੀ_remainder;                                             \
}

/* Clear the last_remainder bin */

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ clear_ਆਖਰੀ_remainder \
  (ਆਖਰੀ_remainder->fd = ਆਖਰੀ_remainder->bk = ਆਖਰੀ_remainder)

/* Routines dealing with mmap(). */

#ਜੇ HAVE_MMAP

#ਜੇ __STD_C
ਸਥਿਰ mchunkptr mmap_chunk(size_t ਮਾਪ)
#ਨਹੀਂਤਾਂ
ਸਥਿਰ mchunkptr mmap_chunk(ਮਾਪ) size_t ਮਾਪ;
#ਜੇ_ਅੰਤ
{
  size_t ਪੰਨਾ_ਨਕਾਬ = malloc_getpagesize - 1;
  mchunkptr p;

#ਜੇ_ਅਪਰਿਭਾਸ਼ਿਤ MAP_ANONYMOUS
  ਸਥਿਰ ਪੂਰਨ fd = -1;
#ਜੇ_ਅੰਤ

  ਜੇ(n_mmaps >= n_mmaps_max) ਵਾਪਸ 0; /* too many regions */

  /* For mmapped chunks, the overhead is one SIZE_SZ unit larger, because
   * there is no following chunk whose prev_size field could be used.
   */
  ਮਾਪ = (ਮਾਪ + ਮਾਪ_SZ + ਪੰਨਾ_ਨਕਾਬ) & ~ਪੰਨਾ_ਨਕਾਬ;

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ MAP_ANONYMOUS
  p = (mchunkptr)mmap(0, ਮਾਪ, PROT_READ|PROT_WRITE,
		      MAP_ਨਿੱਜੀ|MAP_ANONYMOUS, -1, 0);
#ਨਹੀਂਤਾਂ /* !MAP_ANONYMOUS */
  ਜੇ (fd < 0)
  {
    fd = open("/dev/zero", O_RDWR);
    ਜੇ(fd < 0) ਵਾਪਸ 0;
  }
  p = (mchunkptr)mmap(0, ਮਾਪ, PROT_READ|PROT_WRITE, MAP_ਨਿੱਜੀ, fd, 0);
#ਜੇ_ਅੰਤ

  ਜੇ(p == (mchunkptr)-1) ਵਾਪਸ 0;

  n_mmaps++;
  ਜੇ (n_mmaps > max_n_mmaps) max_n_mmaps = n_mmaps;

  /* We demand that eight bytes into a page must be 8-byte aligned. */
  ਪ੍ਰਤਿਜ੍ਞਾ(aligned_OK(chunk2mem(p)));

  /* The offset to the start of the mmapped region is stored
   * in the prev_size field of the chunk; normally it is zero,
   * but that can be changed in memalign().
   */
  p->prev_ਮਾਪ = 0;
  ਸੈੱਟ_ਸਿਰ(p, ਮਾਪ|IS_MMAPPED);

  mmapped_mem += ਮਾਪ;
  ਜੇ ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)mmapped_mem > (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)max_mmapped_mem)
    max_mmapped_mem = mmapped_mem;
  ਜੇ ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)(mmapped_mem + sbrked_mem) > (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)max_ਕੁੱਲ_mem)
    max_ਕੁੱਲ_mem = mmapped_mem + sbrked_mem;
  ਵਾਪਸ p;
}

#ਜੇ __STD_C
ਸਥਿਰ ਖਾਲੀ munmap_chunk(mchunkptr p)
#ਨਹੀਂਤਾਂ
ਸਥਿਰ ਖਾਲੀ munmap_chunk(p) mchunkptr p;
#ਜੇ_ਅੰਤ
{
  ਅੰਦਰੂਨੀ_ਮਾਪ_T ਮਾਪ = chunksize(p);
  ਪੂਰਨ ret;

  ਪ੍ਰਤਿਜ੍ਞਾ (chunk_is_mmapped(p));
  ਪ੍ਰਤਿਜ੍ਞਾ(! ((ਅੱਖਰ*)p >= sbrk_ਅਧਾਰ && (ਅੱਖਰ*)p < sbrk_ਅਧਾਰ + sbrked_mem));
  ਪ੍ਰਤਿਜ੍ਞਾ((n_mmaps > 0));
  ਪ੍ਰਤਿਜ੍ਞਾ(((p->prev_ਮਾਪ + ਮਾਪ) & (malloc_getpagesize-1)) == 0);

  n_mmaps--;
  mmapped_mem -= (ਮਾਪ + p->prev_ਮਾਪ);

  ret = munmap((ਅੱਖਰ *)p - p->prev_ਮਾਪ, ਮਾਪ + p->prev_ਮਾਪ);

  /* munmap returns non-zero on failure */
  ਪ੍ਰਤਿਜ੍ਞਾ(ret == 0);
}

#ਜੇ HAVE_MREMAP

#ਜੇ __STD_C
ਸਥਿਰ mchunkptr mremap_chunk(mchunkptr p, size_t new_ਮਾਪ)
#ਨਹੀਂਤਾਂ
ਸਥਿਰ mchunkptr mremap_chunk(p, new_ਮਾਪ) mchunkptr p; size_t new_ਮਾਪ;
#ਜੇ_ਅੰਤ
{
  size_t ਪੰਨਾ_ਨਕਾਬ = malloc_getpagesize - 1;
  ਅੰਦਰੂਨੀ_ਮਾਪ_T ਵਿੱਥ = p->prev_ਮਾਪ;
  ਅੰਦਰੂਨੀ_ਮਾਪ_T ਮਾਪ = chunksize(p);
  ਅੱਖਰ *cp;

  ਪ੍ਰਤਿਜ੍ਞਾ (chunk_is_mmapped(p));
  ਪ੍ਰਤਿਜ੍ਞਾ(! ((ਅੱਖਰ*)p >= sbrk_ਅਧਾਰ && (ਅੱਖਰ*)p < sbrk_ਅਧਾਰ + sbrked_mem));
  ਪ੍ਰਤਿਜ੍ਞਾ((n_mmaps > 0));
  ਪ੍ਰਤਿਜ੍ਞਾ(((ਮਾਪ + ਵਿੱਥ) & (malloc_getpagesize-1)) == 0);

  /* Note the extra SIZE_SZ overhead as in mmap_chunk(). */
  new_ਮਾਪ = (new_ਮਾਪ + ਵਿੱਥ + ਮਾਪ_SZ + ਪੰਨਾ_ਨਕਾਬ) & ~ਪੰਨਾ_ਨਕਾਬ;

  cp = (ਅੱਖਰ *)mremap((ਅੱਖਰ *)p - ਵਿੱਥ, ਮਾਪ + ਵਿੱਥ, new_ਮਾਪ, 1);

  ਜੇ (cp == (ਅੱਖਰ *)-1) ਵਾਪਸ 0;

  p = (mchunkptr)(cp + ਵਿੱਥ);

  ਪ੍ਰਤਿਜ੍ਞਾ(aligned_OK(chunk2mem(p)));

  ਪ੍ਰਤਿਜ੍ਞਾ((p->prev_ਮਾਪ == ਵਿੱਥ));
  ਸੈੱਟ_ਸਿਰ(p, (new_ਮਾਪ - ਵਿੱਥ)|IS_MMAPPED);

  mmapped_mem -= ਮਾਪ + ਵਿੱਥ;
  mmapped_mem += new_ਮਾਪ;
  ਜੇ ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)mmapped_mem > (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)max_mmapped_mem)
    max_mmapped_mem = mmapped_mem;
  ਜੇ ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)(mmapped_mem + sbrked_mem) > (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)max_ਕੁੱਲ_mem)
    max_ਕੁੱਲ_mem = mmapped_mem + sbrked_mem;
  ਵਾਪਸ p;
}

#ਜੇ_ਅੰਤ /* HAVE_MREMAP */

#ਜੇ_ਅੰਤ /* HAVE_MMAP */

/*
  Extend the top-most chunk by obtaining memory from system.
  Main interface to sbrk (but see also malloc_trim).
*/

#ਜੇ __STD_C
ਸਥਿਰ ਖਾਲੀ malloc_extend_ਸਿਖਰ(ਅੰਦਰੂਨੀ_ਮਾਪ_T nb)
#ਨਹੀਂਤਾਂ
ਸਥਿਰ ਖਾਲੀ malloc_extend_ਸਿਖਰ(nb) ਅੰਦਰੂਨੀ_ਮਾਪ_T nb;
#ਜੇ_ਅੰਤ
{
  ਅੱਖਰ*     brk;                  /* return value from sbrk */
  ਅੰਦਰੂਨੀ_ਮਾਪ_T front_misalign; /* unusable bytes at front of sbrked space */
  ਅੰਦਰੂਨੀ_ਮਾਪ_T correction;     /* bytes for 2nd sbrk call */
  ਅੱਖਰ*     new_brk;              /* return of 2nd sbrk call */
  ਅੰਦਰੂਨੀ_ਮਾਪ_T ਸਿਖਰ_ਮਾਪ;       /* new size of top chunk */

  mchunkptr old_ਸਿਖਰ     = ਸਿਖਰ;  /* Record state of old top */
  ਅੰਦਰੂਨੀ_ਮਾਪ_T old_ਸਿਖਰ_ਮਾਪ = chunksize(old_ਸਿਖਰ);
  ਅੱਖਰ*     old_ਸਿਰਾ      = (ਅੱਖਰ*)(chunk_at_ਵਿੱਥ(old_ਸਿਖਰ, old_ਸਿਖਰ_ਮਾਪ));

  /* Pad request with top_pad plus minimal overhead */

  ਅੰਦਰੂਨੀ_ਮਾਪ_T    sbrk_ਮਾਪ     = nb + ਸਿਖਰ_pad + MINSIZE;
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ pagesz    = malloc_getpagesize;

  /* If not the first time through, round to preserve page boundary */
  /* Otherwise, we need to correct to a page size below anyway. */
  /* (We also correct below if an intervening foreign sbrk call.) */

  ਜੇ (sbrk_ਅਧਾਰ != (ਅੱਖਰ*)(-1))
    sbrk_ਮਾਪ = (sbrk_ਮਾਪ + (pagesz - 1)) & ~(pagesz - 1);

  brk = (ਅੱਖਰ*)(MORECORE (sbrk_ਮਾਪ));

  /* Fail if sbrk failed or if a foreign sbrk call killed our space */
  ਜੇ (brk == (ਅੱਖਰ*)(MORECORE_FAILURE) ||
      (brk < old_ਸਿਰਾ && old_ਸਿਖਰ != initial_ਸਿਖਰ))
    ਵਾਪਸ;

  sbrked_mem += sbrk_ਮਾਪ;

  ਜੇ (brk == old_ਸਿਰਾ) /* can just add bytes to current top */
  {
    ਸਿਖਰ_ਮਾਪ = sbrk_ਮਾਪ + old_ਸਿਖਰ_ਮਾਪ;
    ਸੈੱਟ_ਸਿਰ(ਸਿਖਰ, ਸਿਖਰ_ਮਾਪ | PREV_INUSE);
  }
  ਨਹੀਂਤਾਂ
  {
    ਜੇ (sbrk_ਅਧਾਰ == (ਅੱਖਰ*)(-1))  /* First time through. Record base */
      sbrk_ਅਧਾਰ = brk;
    ਨਹੀਂਤਾਂ  /* Someone else called sbrk().  Count those bytes as sbrked_mem. */
      sbrked_mem += brk - (ਅੱਖਰ*)old_ਸਿਰਾ;

    /* Guarantee alignment of first new chunk made from this space */
    front_misalign = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)chunk2mem(brk) & MALLOC_ALIGN_ਨਕਾਬ;
    ਜੇ (front_misalign > 0)
    {
      correction = (MALLOC_ALIGNMENT) - front_misalign;
      brk += correction;
    }
    ਨਹੀਂਤਾਂ
      correction = 0;

    /* Guarantee the next brk will be at a page boundary */

    correction += ((((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)(brk + sbrk_ਮਾਪ))+(pagesz-1)) &
		   ~(pagesz - 1)) - ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)(brk + sbrk_ਮਾਪ));

    /* Allocate correction */
    new_brk = (ਅੱਖਰ*)(MORECORE (correction));
    ਜੇ (new_brk == (ਅੱਖਰ*)(MORECORE_FAILURE)) ਵਾਪਸ;

    sbrked_mem += correction;

    ਸਿਖਰ = (mchunkptr)brk;
    ਸਿਖਰ_ਮਾਪ = new_brk - brk + correction;
    ਸੈੱਟ_ਸਿਰ(ਸਿਖਰ, ਸਿਖਰ_ਮਾਪ | PREV_INUSE);

    ਜੇ (old_ਸਿਖਰ != initial_ਸਿਖਰ)
    {

      /* There must have been an intervening foreign sbrk call. */
      /* A double fencepost is necessary to prevent consolidation */

      /* If not enough space to do this, then user did something very wrong */
      ਜੇ (old_ਸਿਖਰ_ਮਾਪ < MINSIZE)
      {
	ਸੈੱਟ_ਸਿਰ(ਸਿਖਰ, PREV_INUSE); /* will force null return from malloc */
	ਵਾਪਸ;
      }

      /* Also keep size a multiple of MALLOC_ALIGNMENT */
      old_ਸਿਖਰ_ਮਾਪ = (old_ਸਿਖਰ_ਮਾਪ - 3*ਮਾਪ_SZ) & ~MALLOC_ALIGN_ਨਕਾਬ;
      ਸੈੱਟ_ਸਿਰ_ਮਾਪ(old_ਸਿਖਰ, old_ਸਿਖਰ_ਮਾਪ);
      chunk_at_ਵਿੱਥ(old_ਸਿਖਰ, old_ਸਿਖਰ_ਮਾਪ          )->ਮਾਪ =
	ਮਾਪ_SZ|PREV_INUSE;
      chunk_at_ਵਿੱਥ(old_ਸਿਖਰ, old_ਸਿਖਰ_ਮਾਪ + ਮਾਪ_SZ)->ਮਾਪ =
	ਮਾਪ_SZ|PREV_INUSE;
      /* If possible, release the rest. */
      ਜੇ (old_ਸਿਖਰ_ਮਾਪ >= MINSIZE)
	fREe(chunk2mem(old_ਸਿਖਰ));
    }
  }

  ਜੇ ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)sbrked_mem > (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)max_sbrked_mem)
    max_sbrked_mem = sbrked_mem;
  ਜੇ ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)(mmapped_mem + sbrked_mem) > (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)max_ਕੁੱਲ_mem)
    max_ਕੁੱਲ_mem = mmapped_mem + sbrked_mem;

  /* We always land on a page boundary */
  ਪ੍ਰਤਿਜ੍ਞਾ(((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)((ਅੱਖਰ*)ਸਿਖਰ + ਸਿਖਰ_ਮਾਪ) & (pagesz - 1)) == 0);
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
#ਜੇ __STD_C
Void_t* mALLOc_impl(size_t ਬਾਈਟਾਂ)
#ਨਹੀਂਤਾਂ
Void_t* mALLOc_impl(ਬਾਈਟਾਂ) size_t ਬਾਈਟਾਂ;
#ਜੇ_ਅੰਤ
{
  mchunkptr victim;                  /* inspected/selected chunk */
  ਅੰਦਰੂਨੀ_ਮਾਪ_T victim_ਮਾਪ;       /* its size */
  ਪੂਰਨ       idx;                     /* index for bin traversal */
  mbinptr   bin;                     /* associated bin */
  mchunkptr remainder;               /* remainder from a split */
  ਲੰਮਾ      remainder_ਮਾਪ;          /* its size */
  ਪੂਰਨ       remainder_ਸੂਚਕਾਂਕ;         /* its bin index */
  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਖੰਡ;               /* block traverser bit */
  ਪੂਰਨ       startidx;                /* first bin of a traversed block */
  mchunkptr fwd;                     /* misc temp for linking */
  mchunkptr bck;                     /* misc temp for linking */
  mbinptr q;                         /* misc temp */

  ਅੰਦਰੂਨੀ_ਮਾਪ_T nb;

#ਜੇ CONFIG_IS_ENABLED(SYS_MALLOC_F)
	ਜੇ (!(gd->flags & GD_FLG_ਭਰਿਆ_MALLOC_INIT))
		ਵਾਪਸ malloc_simple(ਬਾਈਟਾਂ);
#ਜੇ_ਅੰਤ

  ਜੇ (CONFIG_IS_ENABLED(UNIT_TEST) && malloc_testing) {
    ਜੇ (--malloc_max_allocs < 0)
      ਵਾਪਸ ਨਿਰਾ;
  }

  /* check if mem_malloc_init() was run */
  ਜੇ ((mem_malloc_ਸ਼ੁਰੂ == 0) && (mem_malloc_ਸਿਰਾ == 0)) {
    /* not initialized yet */
    ਵਾਪਸ ਨਿਰਾ;
  }

  ਜੇ (ਬਾਈਟਾਂ > CONFIG_SYS_MALLOC_LEN || (ਲੰਮਾ)ਬਾਈਟਾਂ < 0)
     ਵਾਪਸ ਨਿਰਾ;

  nb = request2size(ਬਾਈਟਾਂ);  /* padded request size; */

  /* Check for exact match in a bin */

  ਜੇ (is_small_request(nb))  /* Faster version for small requests */
  {
    idx = smallbin_ਸੂਚਕਾਂਕ(nb);

    /* No traversal or size check necessary for small bins.  */

    q = bin_at(idx);
    victim = ਆਖਰੀ(q);

    /* Also scan the next one, since it would have a remainder < MINSIZE */
    ਜੇ (victim == q)
    {
      q = ਅਗਲਾ_bin(q);
      victim = ਆਖਰੀ(q);
    }
    ਜੇ (victim != q)
    {
      victim_ਮਾਪ = chunksize(victim);
      unlink(victim, bck, fwd);
      ਸੈੱਟ_inuse_ਦੋਆਧਾਰੀ_ਅੰਕ_at_ਵਿੱਥ(victim, victim_ਮਾਪ);
      check_malloced_chunk(victim, nb);
      VALGRIND_MALLOCLIKE_ਖੰਡ(chunk2mem(victim), ਬਾਈਟਾਂ, ਮਾਪ_SZ, false);
      ਵਾਪਸ chunk2mem(victim);
    }

    idx += 2; /* Set for bin scan below. We've already scanned 2 bins. */

  }
  ਨਹੀਂਤਾਂ
  {
    idx = bin_ਸੂਚਕਾਂਕ(nb);
    bin = bin_at(idx);

    ਲਈ (victim = ਆਖਰੀ(bin); victim != bin; victim = victim->bk)
    {
      victim_ਮਾਪ = chunksize(victim);
      remainder_ਮਾਪ = victim_ਮਾਪ - nb;

      ਜੇ (remainder_ਮਾਪ >= (ਲੰਮਾ)MINSIZE) /* too big */
      {
	--idx; /* adjust to rescan below after checking last remainder */
	ਰੋਕੋ;
      }

      ਨਹੀਂਤਾਂ ਜੇ (remainder_ਮਾਪ >= 0) /* exact fit */
      {
	unlink(victim, bck, fwd);
	ਸੈੱਟ_inuse_ਦੋਆਧਾਰੀ_ਅੰਕ_at_ਵਿੱਥ(victim, victim_ਮਾਪ);
	check_malloced_chunk(victim, nb);
        VALGRIND_MALLOCLIKE_ਖੰਡ(chunk2mem(victim), ਬਾਈਟਾਂ, ਮਾਪ_SZ, false);
	ਵਾਪਸ chunk2mem(victim);
      }
    }

    ++idx;

  }

  /* Try to use the last split-off remainder */

  ਜੇ ( (victim = ਆਖਰੀ_remainder->fd) != ਆਖਰੀ_remainder)
  {
    victim_ਮਾਪ = chunksize(victim);
    remainder_ਮਾਪ = victim_ਮਾਪ - nb;

    ਜੇ (remainder_ਮਾਪ >= (ਲੰਮਾ)MINSIZE) /* re-split */
    {
      remainder = chunk_at_ਵਿੱਥ(victim, nb);
      ਸੈੱਟ_ਸਿਰ(victim, nb | PREV_INUSE);
      link_ਆਖਰੀ_remainder(remainder);
      ਸੈੱਟ_ਸਿਰ(remainder, remainder_ਮਾਪ | PREV_INUSE);
      ਸੈੱਟ_foot(remainder, remainder_ਮਾਪ);
      check_malloced_chunk(victim, nb);
      VALGRIND_MALLOCLIKE_ਖੰਡ(chunk2mem(victim), ਬਾਈਟਾਂ, ਮਾਪ_SZ, false);
      ਵਾਪਸ chunk2mem(victim);
    }

    clear_ਆਖਰੀ_remainder;

    ਜੇ (remainder_ਮਾਪ >= 0)  /* exhaust */
    {
      ਸੈੱਟ_inuse_ਦੋਆਧਾਰੀ_ਅੰਕ_at_ਵਿੱਥ(victim, victim_ਮਾਪ);
      check_malloced_chunk(victim, nb);
      VALGRIND_MALLOCLIKE_ਖੰਡ(chunk2mem(victim), ਬਾਈਟਾਂ, ਮਾਪ_SZ, false);
      ਵਾਪਸ chunk2mem(victim);
    }

    /* Else place in bin */

    frontlink(victim, victim_ਮਾਪ, remainder_ਸੂਚਕਾਂਕ, bck, fwd);
  }

  /*
     If there are any possibly nonempty big-enough blocks,
     search for best fitting chunk by scanning bins in blockwidth units.
  */

  ਜੇ ( (ਖੰਡ = idx2binblock(idx)) <= binblocks_r)
  {

    /* Get to the first marked block */

    ਜੇ ( (ਖੰਡ & binblocks_r) == 0)
    {
      /* force to an even block boundary */
      idx = (idx & ~(BINBLOCKWIDTH - 1)) + BINBLOCKWIDTH;
      ਖੰਡ <<= 1;
      ਜਦੋਂ ((ਖੰਡ & binblocks_r) == 0)
      {
	idx += BINBLOCKWIDTH;
	ਖੰਡ <<= 1;
      }
    }

    /* For each possibly nonempty block ... */
    ਲਈ (;;)
    {
      startidx = idx;          /* (track incomplete blocks) */
      q = bin = bin_at(idx);

      /* For each bin in this block ... */
      ਕਰੋ
      {
	/* Find and use first big enough chunk ... */

	ਲਈ (victim = ਆਖਰੀ(bin); victim != bin; victim = victim->bk)
	{
	  victim_ਮਾਪ = chunksize(victim);
	  remainder_ਮਾਪ = victim_ਮਾਪ - nb;

	  ਜੇ (remainder_ਮਾਪ >= (ਲੰਮਾ)MINSIZE) /*ਵੰਡੋ*/
	  {
	    remainder = chunk_at_ਵਿੱਥ(victim, nb);
	    ਸੈੱਟ_ਸਿਰ(victim, nb | PREV_INUSE);
	    unlink(victim, bck, fwd);
	    link_ਆਖਰੀ_remainder(remainder);
	    ਸੈੱਟ_ਸਿਰ(remainder, remainder_ਮਾਪ | PREV_INUSE);
	    ਸੈੱਟ_foot(remainder, remainder_ਮਾਪ);
	    check_malloced_chunk(victim, nb);
	    VALGRIND_MALLOCLIKE_ਖੰਡ(chunk2mem(victim), ਬਾਈਟਾਂ, ਮਾਪ_SZ, false);
	    ਵਾਪਸ chunk2mem(victim);
	  }

	  ਨਹੀਂਤਾਂ ਜੇ (remainder_ਮਾਪ >= 0)  /* take */
	  {
	    ਸੈੱਟ_inuse_ਦੋਆਧਾਰੀ_ਅੰਕ_at_ਵਿੱਥ(victim, victim_ਮਾਪ);
	    unlink(victim, bck, fwd);
	    check_malloced_chunk(victim, nb);
	    VALGRIND_MALLOCLIKE_ਖੰਡ(chunk2mem(victim), ਬਾਈਟਾਂ, ਮਾਪ_SZ, false);
	    ਵਾਪਸ chunk2mem(victim);
	  }

	}

       bin = ਅਗਲਾ_bin(bin);

      } ਜਦੋਂ ((++idx & (BINBLOCKWIDTH - 1)) != 0);

      /* Clear out the block bit. */

      ਕਰੋ   /* Possibly backtrack to try to clear a partial block */
      {
	ਜੇ ((startidx & (BINBLOCKWIDTH - 1)) == 0)
	{
	  av_[1] = (mbinptr)(binblocks_r & ~ਖੰਡ);
	  ਰੋਕੋ;
	}
	--startidx;
       q = prev_bin(q);
      } ਜਦੋਂ (ਪਹਿਲਾ(q) == q);

      /* Get to the next possibly nonempty block */

      ਜੇ ( (ਖੰਡ <<= 1) <= binblocks_r && (ਖੰਡ != 0) )
      {
	ਜਦੋਂ ((ਖੰਡ & binblocks_r) == 0)
	{
	  idx += BINBLOCKWIDTH;
	  ਖੰਡ <<= 1;
	}
      }
      ਨਹੀਂਤਾਂ
	ਰੋਕੋ;
    }
  }

  /* Try to use top chunk */

  /* Require that there be a remainder, ensuring top always exists  */
  ਜੇ ( (remainder_ਮਾਪ = chunksize(ਸਿਖਰ) - nb) < (ਲੰਮਾ)MINSIZE)
  {

#ਜੇ HAVE_MMAP
    /* If big and would otherwise need to extend, try to use mmap instead */
    ਜੇ ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)nb >= (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)mmap_ਹੱਦ &&
	(victim = mmap_chunk(nb)))
      VALGRIND_MALLOCLIKE_ਖੰਡ(chunk2mem(victim), ਬਾਈਟਾਂ, ਮਾਪ_SZ, false);
      ਵਾਪਸ chunk2mem(victim);
#ਜੇ_ਅੰਤ

    /* Try to extend */
    malloc_extend_ਸਿਖਰ(nb);
    ਜੇ ( (remainder_ਮਾਪ = chunksize(ਸਿਖਰ) - nb) < (ਲੰਮਾ)MINSIZE)
      ਵਾਪਸ ਨਿਰਾ; /* propagate failure */
  }

  victim = ਸਿਖਰ;
  ਸੈੱਟ_ਸਿਰ(victim, nb | PREV_INUSE);
  ਸਿਖਰ = chunk_at_ਵਿੱਥ(victim, nb);
  ਸੈੱਟ_ਸਿਰ(ਸਿਖਰ, remainder_ਮਾਪ | PREV_INUSE);
  check_malloced_chunk(victim, nb);
  VALGRIND_MALLOCLIKE_ਖੰਡ(chunk2mem(victim), ਬਾਈਟਾਂ, ਮਾਪ_SZ, false);
  ਵਾਪਸ chunk2mem(victim);

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
#ਜੇ __STD_C
ਖਾਲੀ fREe_impl(Void_t* mem)
#ਨਹੀਂਤਾਂ
ਖਾਲੀ fREe_impl(mem) Void_t* mem;
#ਜੇ_ਅੰਤ
{
  mchunkptr p;         /* chunk corresponding to mem */
  ਅੰਦਰੂਨੀ_ਮਾਪ_T hd;  /* its head field */
  ਅੰਦਰੂਨੀ_ਮਾਪ_T sz;  /* its size */
  ਪੂਰਨ       idx;       /* its bin index */
  mchunkptr ਅਗਲਾ;      /* next contiguous chunk */
  ਅੰਦਰੂਨੀ_ਮਾਪ_T nextsz; /* its size */
  ਅੰਦਰੂਨੀ_ਮਾਪ_T prevsz; /* size of previous contiguous chunk */
  mchunkptr bck;       /* misc temp for linking */
  mchunkptr fwd;       /* misc temp for linking */
  ਪੂਰਨ       islr;      /* track whether merging with last_remainder */

#ਜੇ CONFIG_IS_ENABLED(SYS_MALLOC_F)
	/* free() is a no-op - all the memory will be freed on relocation */
	ਜੇ (!(gd->flags & GD_FLG_ਭਰਿਆ_MALLOC_INIT)) {
		VALGRIND_FREELIKE_ਖੰਡ(mem, ਮਾਪ_SZ);
		ਵਾਪਸ;
	}
#ਜੇ_ਅੰਤ

  ਜੇ (mem == ਨਿਰਾ)                              /* free(0) has no effect */
    ਵਾਪਸ;

  p = mem2chunk(mem);
  hd = p->ਮਾਪ;

#ਜੇ HAVE_MMAP
  ਜੇ (hd & IS_MMAPPED)                       /* release mmapped memory. */
  {
    munmap_chunk(p);
    ਵਾਪਸ;
  }
#ਜੇ_ਅੰਤ

  check_inuse_chunk(p);

  sz = hd & ~PREV_INUSE;
  ਅਗਲਾ = chunk_at_ਵਿੱਥ(p, sz);
  nextsz = chunksize(ਅਗਲਾ);
  VALGRIND_FREELIKE_ਖੰਡ(mem, ਮਾਪ_SZ);

  ਜੇ (ਅਗਲਾ == ਸਿਖਰ)                            /* merge with top */
  {
    sz += nextsz;

    ਜੇ (!(hd & PREV_INUSE))                    /* consolidate backward */
    {
      prevsz = p->prev_ਮਾਪ;
      p = chunk_at_ਵਿੱਥ(p, -((ਲੰਮਾ) prevsz));
      sz += prevsz;
      unlink(p, bck, fwd);
    }

    ਸੈੱਟ_ਸਿਰ(p, sz | PREV_INUSE);
    ਸਿਖਰ = p;
    ਜੇ ((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)(sz) >= (ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)trim_ਹੱਦ)
      malloc_trim(ਸਿਖਰ_pad);
    ਵਾਪਸ;
  }

  ਸੈੱਟ_ਸਿਰ(ਅਗਲਾ, nextsz);                    /* clear inuse bit */

  islr = 0;

  ਜੇ (!(hd & PREV_INUSE))                    /* consolidate backward */
  {
    prevsz = p->prev_ਮਾਪ;
    p = chunk_at_ਵਿੱਥ(p, -((ਲੰਮਾ) prevsz));
    sz += prevsz;

    ਜੇ (p->fd == ਆਖਰੀ_remainder)             /* keep as last_remainder */
      islr = 1;
    ਨਹੀਂਤਾਂ
      unlink(p, bck, fwd);
  }

  ਜੇ (!(inuse_ਦੋਆਧਾਰੀ_ਅੰਕ_at_ਵਿੱਥ(ਅਗਲਾ, nextsz)))   /* consolidate forward */
  {
    sz += nextsz;

    ਜੇ (!islr && ਅਗਲਾ->fd == ਆਖਰੀ_remainder)  /* re-insert last_remainder */
    {
      islr = 1;
      link_ਆਖਰੀ_remainder(p);
    }
    ਨਹੀਂਤਾਂ
      unlink(ਅਗਲਾ, bck, fwd);
  }

  ਸੈੱਟ_ਸਿਰ(p, sz | PREV_INUSE);
  ਸੈੱਟ_foot(p, sz);
  ਜੇ (!islr)
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
#ਜੇ __STD_C
Void_t* rEALLOc_impl(Void_t* oldmem, size_t ਬਾਈਟਾਂ)
#ਨਹੀਂਤਾਂ
Void_t* rEALLOc_impl(oldmem, ਬਾਈਟਾਂ) Void_t* oldmem; size_t ਬਾਈਟਾਂ;
#ਜੇ_ਅੰਤ
{
  ਅੰਦਰੂਨੀ_ਮਾਪ_T    nb;      /* padded request size */

  mchunkptr oldp;             /* chunk corresponding to oldmem */
  ਅੰਦਰੂਨੀ_ਮਾਪ_T    oldsize; /* its size */

  mchunkptr newp;             /* chunk to return */
  ਅੰਦਰੂਨੀ_ਮਾਪ_T    newsize; /* its size */
  Void_t*   newmem;           /* corresponding user mem */

  mchunkptr ਅਗਲਾ;             /* next contiguous chunk after oldp */
  ਅੰਦਰੂਨੀ_ਮਾਪ_T  nextsize;  /* its size */

  mchunkptr prev;             /* previous contiguous chunk before oldp */
  ਅੰਦਰੂਨੀ_ਮਾਪ_T  prevsize;  /* its size */

  mchunkptr remainder;        /* holds split off extra space from newp */
  ਅੰਦਰੂਨੀ_ਮਾਪ_T  remainder_ਮਾਪ;   /* its size */

  mchunkptr bck;              /* misc temp for linking */
  mchunkptr fwd;              /* misc temp for linking */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ REALLOC_ZERO_ਬਾਈਟਾਂ_FREES
  ਜੇ (!ਬਾਈਟਾਂ) {
	fREe_impl(oldmem);
	ਵਾਪਸ ਨਿਰਾ;
  }
#ਜੇ_ਅੰਤ

  ਜੇ (ਬਾਈਟਾਂ > CONFIG_SYS_MALLOC_LEN || (ਲੰਮਾ)ਬਾਈਟਾਂ < 0)
     ਵਾਪਸ ਨਿਰਾ;

  /* realloc of null is supposed to be same as malloc */
  ਜੇ (oldmem == ਨਿਰਾ) ਵਾਪਸ mALLOc_impl(ਬਾਈਟਾਂ);

#ਜੇ CONFIG_IS_ENABLED(SYS_MALLOC_F)
	ਜੇ (!(gd->flags & GD_FLG_ਭਰਿਆ_MALLOC_INIT)) {
		/* This is harder to support and should not be needed */
		panic("pre-reloc realloc() is not supported");
	}
#ਜੇ_ਅੰਤ
  ਜੇ (CONFIG_IS_ENABLED(UNIT_TEST) && malloc_testing) {
    ਜੇ (--malloc_max_allocs < 0)
      ਵਾਪਸ ਨਿਰਾ;
  }

  newp    = oldp    = mem2chunk(oldmem);
  newsize = oldsize = chunksize(oldp);

  nb = request2size(ਬਾਈਟਾਂ);

#ਜੇ HAVE_MMAP
  ਜੇ (chunk_is_mmapped(oldp))
  {
#ਜੇ HAVE_MREMAP
    newp = mremap_chunk(oldp, nb);
    ਜੇ(newp) ਵਾਪਸ chunk2mem(newp);
#ਜੇ_ਅੰਤ
    /* Note the extra SIZE_SZ overhead. */
    ਜੇ(oldsize - ਮਾਪ_SZ >= nb) ਵਾਪਸ oldmem; /* do nothing */
    /* Must alloc, copy, free. */
    newmem = mALLOc_impl(ਬਾਈਟਾਂ);
    ਜੇ (!newmem)
	ਵਾਪਸ ਨਿਰਾ; /* propagate failure */
    MALLOC_COPY(newmem, oldmem, oldsize - 2*ਮਾਪ_SZ);
    munmap_chunk(oldp);
    ਵਾਪਸ newmem;
  }
#ਜੇ_ਅੰਤ

  check_inuse_chunk(oldp);

  ਜੇ ((ਲੰਮਾ)(oldsize) < (ਲੰਮਾ)(nb))
  {

    /* Try expanding forward */

    ਅਗਲਾ = chunk_at_ਵਿੱਥ(oldp, oldsize);
    ਜੇ (ਅਗਲਾ == ਸਿਖਰ || !inuse(ਅਗਲਾ))
    {
      nextsize = chunksize(ਅਗਲਾ);

      /* Forward into top only if a remainder */
      ਜੇ (ਅਗਲਾ == ਸਿਖਰ)
      {
	ਜੇ ((ਲੰਮਾ)(nextsize + newsize) >= (ਲੰਮਾ)(nb + MINSIZE))
	{
	  newsize += nextsize;
	  ਸਿਖਰ = chunk_at_ਵਿੱਥ(oldp, nb);
	  ਸੈੱਟ_ਸਿਰ(ਸਿਖਰ, (newsize - nb) | PREV_INUSE);
	  ਸੈੱਟ_ਸਿਰ_ਮਾਪ(oldp, nb);
	  VALGRIND_RESIZEINPLACE_ਖੰਡ(chunk2mem(oldp), 0, ਬਾਈਟਾਂ, ਮਾਪ_SZ);
	  VALGRIND_MAKE_MEM_DEFINED(chunk2mem(oldp), ਬਾਈਟਾਂ);
	  ਵਾਪਸ chunk2mem(oldp);
	}
      }

      /* Forward into next chunk */
      ਨਹੀਂਤਾਂ ਜੇ (((ਲੰਮਾ)(nextsize + newsize) >= (ਲੰਮਾ)(nb)))
      {
	unlink(ਅਗਲਾ, bck, fwd);
	newsize  += nextsize;
	VALGRIND_RESIZEINPLACE_ਖੰਡ(chunk2mem(oldp), 0, ਬਾਈਟਾਂ, ਮਾਪ_SZ);
	VALGRIND_MAKE_MEM_DEFINED(chunk2mem(oldp), ਬਾਈਟਾਂ);
	ਜਾਓ ਵੰਡੋ;
      }
    }
    ਨਹੀਂਤਾਂ
    {
      ਅਗਲਾ = ਨਿਰਾ;
      nextsize = 0;
    }

    /* Try shifting backwards. */

    ਜੇ (!prev_inuse(oldp))
    {
      prev = prev_chunk(oldp);
      prevsize = chunksize(prev);

      /* try forward + backward first to save a later consolidation */

      ਜੇ (ਅਗਲਾ != ਨਿਰਾ)
      {
	/* into top */
	ਜੇ (ਅਗਲਾ == ਸਿਖਰ)
	{
	  ਜੇ ((ਲੰਮਾ)(nextsize + prevsize + newsize) >= (ਲੰਮਾ)(nb + MINSIZE))
	  {
	    unlink(prev, bck, fwd);
	    newp = prev;
	    newsize += prevsize + nextsize;
	    newmem = chunk2mem(newp);
	    VALGRIND_MALLOCLIKE_ਖੰਡ(newmem, ਬਾਈਟਾਂ, ਮਾਪ_SZ, false);
	    MALLOC_COPY(newmem, oldmem, oldsize - ਮਾਪ_SZ);
	    ਸਿਖਰ = chunk_at_ਵਿੱਥ(newp, nb);
	    ਸੈੱਟ_ਸਿਰ(ਸਿਖਰ, (newsize - nb) | PREV_INUSE);
	    ਸੈੱਟ_ਸਿਰ_ਮਾਪ(newp, nb);
	    VALGRIND_FREELIKE_ਖੰਡ(oldmem, ਮਾਪ_SZ);
	    ਵਾਪਸ newmem;
	  }
	}

	/* into next chunk */
	ਨਹੀਂਤਾਂ ਜੇ (((ਲੰਮਾ)(nextsize + prevsize + newsize) >= (ਲੰਮਾ)(nb)))
	{
	  unlink(ਅਗਲਾ, bck, fwd);
	  unlink(prev, bck, fwd);
	  newp = prev;
	  newsize += nextsize + prevsize;
	  newmem = chunk2mem(newp);
	  VALGRIND_MALLOCLIKE_ਖੰਡ(newmem, ਬਾਈਟਾਂ, ਮਾਪ_SZ, false);
	  MALLOC_COPY(newmem, oldmem, oldsize - ਮਾਪ_SZ);
	  ਜਾਓ ਵੰਡੋ;
	}
      }

      /* backward only */
      ਜੇ (prev != ਨਿਰਾ && (ਲੰਮਾ)(prevsize + newsize) >= (ਲੰਮਾ)nb)
      {
	unlink(prev, bck, fwd);
	newp = prev;
	newsize += prevsize;
	newmem = chunk2mem(newp);
	VALGRIND_MALLOCLIKE_ਖੰਡ(newmem, ਬਾਈਟਾਂ, ਮਾਪ_SZ, false);
	MALLOC_COPY(newmem, oldmem, oldsize - ਮਾਪ_SZ);
	ਜਾਓ ਵੰਡੋ;
      }
    }

    /* Must allocate */

    newmem = mALLOc_impl (ਬਾਈਟਾਂ);

    ਜੇ (newmem == ਨਿਰਾ)  /* propagate failure */
      ਵਾਪਸ ਨਿਰਾ;

    /* Avoid copy if newp is next chunk after oldp. */
    /* (This can only happen when new chunk is sbrk'ed.) */

    ਜੇ ( (newp = mem2chunk(newmem)) == ਅਗਲਾ_chunk(oldp))
    {
      newsize += chunksize(newp);
      newp = oldp;
      ਜਾਓ ਵੰਡੋ;
    }

    /* Otherwise copy, free, and exit */
    MALLOC_COPY(newmem, oldmem, oldsize - ਮਾਪ_SZ);
    fREe_impl(oldmem);
    ਵਾਪਸ newmem;
  } ਨਹੀਂਤਾਂ {
    VALGRIND_RESIZEINPLACE_ਖੰਡ(oldmem, 0, ਬਾਈਟਾਂ, ਮਾਪ_SZ);
    VALGRIND_MAKE_MEM_DEFINED(oldmem, ਬਾਈਟਾਂ);
  }

 ਵੰਡੋ:  /* split off extra room in old or expanded chunk */

  ਜੇ (newsize - nb >= MINSIZE) /* split off remainder */
  {
    remainder = chunk_at_ਵਿੱਥ(newp, nb);
    remainder_ਮਾਪ = newsize - nb;
    ਸੈੱਟ_ਸਿਰ_ਮਾਪ(newp, nb);
    ਸੈੱਟ_ਸਿਰ(remainder, remainder_ਮਾਪ | PREV_INUSE);
    ਸੈੱਟ_inuse_ਦੋਆਧਾਰੀ_ਅੰਕ_at_ਵਿੱਥ(remainder, remainder_ਮਾਪ);
    VALGRIND_MALLOCLIKE_ਖੰਡ(chunk2mem(remainder), remainder_ਮਾਪ, ਮਾਪ_SZ,
			      false);
    fREe_impl(chunk2mem(remainder)); /* let free() deal with it */
  }
  ਨਹੀਂਤਾਂ
  {
    ਸੈੱਟ_ਸਿਰ_ਮਾਪ(newp, newsize);
    ਸੈੱਟ_inuse_ਦੋਆਧਾਰੀ_ਅੰਕ_at_ਵਿੱਥ(newp, newsize);
  }

  check_inuse_chunk(newp);
  ਵਾਪਸ chunk2mem(newp);
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
#ਜੇ __STD_C
Void_t* mEMALIGn_impl(size_t alignment, size_t ਬਾਈਟਾਂ)
#ਨਹੀਂਤਾਂ
Void_t* mEMALIGn_impl(alignment, ਬਾਈਟਾਂ) size_t alignment; size_t ਬਾਈਟਾਂ;
#ਜੇ_ਅੰਤ
{
  ਅੰਦਰੂਨੀ_ਮਾਪ_T    nb;      /* padded  request size */
  ਅੱਖਰ*     m;                /* memory returned by malloc call */
  mchunkptr p;                /* corresponding chunk */
  ਅੱਖਰ*     brk;              /* alignment point within p */
  mchunkptr newp;             /* chunk to return */
  ਅੰਦਰੂਨੀ_ਮਾਪ_T  newsize;   /* its size */
  ਅੰਦਰੂਨੀ_ਮਾਪ_T  leadsize;  /* leading space befor alignment point */
  mchunkptr remainder;        /* spare room at end to split off */
  ਲੰਮਾ      remainder_ਮਾਪ;   /* its size */

  ਜੇ (ਬਾਈਟਾਂ > CONFIG_SYS_MALLOC_LEN || (ਲੰਮਾ)ਬਾਈਟਾਂ < 0)
     ਵਾਪਸ ਨਿਰਾ;

#ਜੇ CONFIG_IS_ENABLED(SYS_MALLOC_F)
	ਜੇ (!(gd->flags & GD_FLG_ਭਰਿਆ_MALLOC_INIT)) {
		ਵਾਪਸ memalign_simple(alignment, ਬਾਈਟਾਂ);
	}
#ਜੇ_ਅੰਤ

  /* If need less alignment than we give anyway, just relay to malloc */

  ਜੇ (alignment <= MALLOC_ALIGNMENT) ਵਾਪਸ mALLOc_impl(ਬਾਈਟਾਂ);

  /* Otherwise, ensure that it is at least a minimum chunk size */

  ਜੇ (alignment <  MINSIZE) alignment = MINSIZE;

  /* Call malloc with worst case padding to hit alignment. */

  nb = request2size(ਬਾਈਟਾਂ);
  m  = (ਅੱਖਰ*)(mALLOc_impl(nb + alignment + MINSIZE));

  /*
  * The attempt to over-allocate (with a size large enough to guarantee the
  * ability to find an aligned region within allocated memory) failed.
  *
  * Try again, this time only allocating exactly the size the user wants. If
  * the allocation now succeeds and just happens to be aligned, we can still
  * fulfill the user's request.
  */
  ਜੇ (m == ਨਿਰਾ) {
    size_t extra, extra2;
    /*
     * Use bytes not nb, since mALLOc internally calls request2size too, and
     * each call increases the size to allocate, to account for the header.
     */
    m  = (ਅੱਖਰ*)(mALLOc_impl(ਬਾਈਟਾਂ));
    /* Aligned -> return it */
    ਜੇ ((((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)(m)) % alignment) == 0)
      ਵਾਪਸ m;
    /*
     * Otherwise, try again, requesting enough extra space to be able to
     * acquire alignment.
     */
    fREe_impl(m);
    /* Add in extra bytes to match misalignment of unexpanded allocation */
    extra = alignment - (((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)(m)) % alignment);
    m  = (ਅੱਖਰ*)(mALLOc_impl(ਬਾਈਟਾਂ + extra));
    /*
     * m might not be the same as before. Validate that the previous value of
     * extra still works for the current value of m.
     * If (!m), extra2=alignment so 
     */
    ਜੇ (m) {
      extra2 = alignment - (((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)(m)) % alignment);
      ਜੇ (extra2 > extra) {
        fREe_impl(m);
        m = ਨਿਰਾ;
      }
    }
    /* Fall through to original NULL check and chunk splitting logic */
  }

  ਜੇ (m == ਨਿਰਾ) ਵਾਪਸ ਨਿਰਾ; /* propagate failure */

  p = mem2chunk(m);

  ਜੇ ((((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)(m)) % alignment) == 0) /* aligned */
  {
#ਜੇ HAVE_MMAP
    ਜੇ(chunk_is_mmapped(p))
      ਵਾਪਸ chunk2mem(p); /* nothing more to do */
#ਜੇ_ਅੰਤ
  }
  ਨਹੀਂਤਾਂ /* misaligned */
  {
    /*
      Find an aligned spot inside chunk.
      Since we need to give back leading space in a chunk of at
      least MINSIZE, if the first calculation places us at
      a spot with less than MINSIZE leader, we can move to the
      next aligned spot -- we've allocated enough total room so that
      this is always possible.
    */

    brk = (ਅੱਖਰ*)mem2chunk(((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)(m + alignment - 1)) & -((ਨਿਸ਼ਾਨੀਵਾਲਾ) alignment));
    ਜੇ ((ਲੰਮਾ)(brk - (ਅੱਖਰ*)(p)) < MINSIZE) brk = brk + alignment;

    newp = (mchunkptr)brk;
    leadsize = brk - (ਅੱਖਰ*)(p);
    newsize = chunksize(p) - leadsize;

#ਜੇ HAVE_MMAP
    ਜੇ(chunk_is_mmapped(p))
    {
      newp->prev_ਮਾਪ = p->prev_ਮਾਪ + leadsize;
      ਸੈੱਟ_ਸਿਰ(newp, newsize|IS_MMAPPED);
      ਵਾਪਸ chunk2mem(newp);
    }
#ਜੇ_ਅੰਤ

    /* give back leader, use the rest */

    ਸੈੱਟ_ਸਿਰ(newp, newsize | PREV_INUSE);
    ਸੈੱਟ_inuse_ਦੋਆਧਾਰੀ_ਅੰਕ_at_ਵਿੱਥ(newp, newsize);
    ਸੈੱਟ_ਸਿਰ_ਮਾਪ(p, leadsize);
    fREe_impl(chunk2mem(p));
    p = newp;
    VALGRIND_MALLOCLIKE_ਖੰਡ(chunk2mem(p), ਬਾਈਟਾਂ, ਮਾਪ_SZ, false);

    ਪ੍ਰਤਿਜ੍ਞਾ (newsize >= nb && (((ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ)(chunk2mem(p))) % alignment) == 0);
  }

  /* Also give back spare room at the end */

  remainder_ਮਾਪ = chunksize(p) - nb;

  ਜੇ (remainder_ਮਾਪ >= (ਲੰਮਾ)MINSIZE)
  {
    remainder = chunk_at_ਵਿੱਥ(p, nb);
    ਸੈੱਟ_ਸਿਰ(remainder, remainder_ਮਾਪ | PREV_INUSE);
    ਸੈੱਟ_ਸਿਰ_ਮਾਪ(p, nb);
    VALGRIND_MALLOCLIKE_ਖੰਡ(chunk2mem(remainder), remainder_ਮਾਪ, ਮਾਪ_SZ,
			      false);
    fREe_impl(chunk2mem(remainder));
  }

  check_inuse_chunk(p);
  ਵਾਪਸ chunk2mem(p);

}

/*
    valloc just invokes memalign with alignment argument equal
    to the page size of the system (or as near to this as can
    be figured out from all the includes/defines above.)
*/

#ਜੇ __STD_C
Void_t* vALLOc(size_t ਬਾਈਟਾਂ)
#ਨਹੀਂਤਾਂ
Void_t* vALLOc(ਬਾਈਟਾਂ) size_t ਬਾਈਟਾਂ;
#ਜੇ_ਅੰਤ
{
  ਵਾਪਸ mEMALIGn (malloc_getpagesize, ਬਾਈਟਾਂ);
}

/*
  pvalloc just invokes valloc for the nearest pagesize
  that will accommodate request
*/

#ਜੇ __STD_C
Void_t* pvALLOc(size_t ਬਾਈਟਾਂ)
#ਨਹੀਂਤਾਂ
Void_t* pvALLOc(ਬਾਈਟਾਂ) size_t ਬਾਈਟਾਂ;
#ਜੇ_ਅੰਤ
{
  size_t pagesize = malloc_getpagesize;
  ਵਾਪਸ mEMALIGn (pagesize, (ਬਾਈਟਾਂ + pagesize - 1) & ~(pagesize - 1));
}

/*

  calloc calls malloc, then zeroes out the allocated chunk.

*/

STATIC_IF_MCHECK
#ਜੇ __STD_C
Void_t* cALLOc_impl(size_t n, size_t elem_ਮਾਪ)
#ਨਹੀਂਤਾਂ
Void_t* cALLOc_impl(n, elem_ਮਾਪ) size_t n; size_t elem_ਮਾਪ;
#ਜੇ_ਅੰਤ
{
  mchunkptr p;
  ਅੰਦਰੂਨੀ_ਮਾਪ_T csz;

  ਅੰਦਰੂਨੀ_ਮਾਪ_T sz = n * elem_ਮਾਪ;

  /* check if expand_top called, in which case don't need to clear */
#ਜੇ CONFIG_IS_ENABLED(SYS_MALLOC_CLEAR_ON_INIT)
#ਜੇ MORECORE_CLEARS
  mchunkptr oldtop = ਸਿਖਰ;
  ਅੰਦਰੂਨੀ_ਮਾਪ_T oldtopsize = chunksize(ਸਿਖਰ);
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ
  Void_t* mem = mALLOc_impl (sz);

  ਜੇ ((ਲੰਮਾ)n < 0) ਵਾਪਸ ਨਿਰਾ;

  ਜੇ (mem == ਨਿਰਾ)
    ਵਾਪਸ ਨਿਰਾ;
  ਨਹੀਂਤਾਂ
  {
#ਜੇ CONFIG_IS_ENABLED(SYS_MALLOC_F)
	ਜੇ (!(gd->flags & GD_FLG_ਭਰਿਆ_MALLOC_INIT)) {
		memset(mem, 0, sz);
		ਵਾਪਸ mem;
	}
#ਜੇ_ਅੰਤ
    p = mem2chunk(mem);

    /* Two optional cases in which clearing not necessary */

#ਜੇ HAVE_MMAP
    ਜੇ (chunk_is_mmapped(p)) ਵਾਪਸ mem;
#ਜੇ_ਅੰਤ

    csz = chunksize(p);

#ਜੇ CONFIG_IS_ENABLED(SYS_MALLOC_CLEAR_ON_INIT)
#ਜੇ MORECORE_CLEARS
    ਜੇ (p == oldtop && csz > oldtopsize)
    {
      /* clear only the bytes from non-freshly-sbrked memory */
      csz = oldtopsize;
    }
#ਜੇ_ਅੰਤ
#ਜੇ_ਅੰਤ

    MALLOC_ZERO(mem, csz - ਮਾਪ_SZ);
    VALGRIND_MAKE_MEM_DEFINED(mem, sz);
    ਵਾਪਸ mem;
  }
}

/*

  cfree just calls free. It is needed/defined on some systems
  that pair it with calloc, presumably for odd historical reasons.

*/

#ਜੇ !defined(ਅੰਦਰੂਨੀ_LINUX_C_LIB) || !defined(__ELF__)
#ਜੇ __STD_C
ਖਾਲੀ cfree(Void_t *mem)
#ਨਹੀਂਤਾਂ
ਖਾਲੀ cfree(mem) Void_t *mem;
#ਜੇ_ਅੰਤ
{
  fREe(mem);
}
#ਜੇ_ਅੰਤ

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ MCHECK_ਢੇਰ_PROTECTION
 #ਅਨ੍ਤਰ੍ਭਾਵ "mcheck_core.inc.h"
 #ਜੇ !__STD_C
  #ਗਲਤੀ "must have __STD_C"
 #ਜੇ_ਅੰਤ

Void_t *mALLOc(size_t ਬਾਈਟਾਂ)
{
	mcheck_pedantic_prehook();
	size_t fullsz = mcheck_alloc_prehook(ਬਾਈਟਾਂ);
	ਖਾਲੀ *p = mALLOc_impl(fullsz);

	ਜੇ (!p)
		ਵਾਪਸ p;
	ਵਾਪਸ mcheck_alloc_posthook(p, ਬਾਈਟਾਂ);
}

ਖਾਲੀ fREe(Void_t *mem) { fREe_impl(mcheck_free_prehook(mem)); }

Void_t *rEALLOc(Void_t *oldmem, size_t ਬਾਈਟਾਂ)
{
	mcheck_pedantic_prehook();
	ਜੇ (ਬਾਈਟਾਂ == 0) {
		ਜੇ (oldmem)
			fREe(oldmem);
		ਵਾਪਸ ਨਿਰਾ;
	}

	ਜੇ (oldmem == ਨਿਰਾ)
		ਵਾਪਸ mALLOc(ਬਾਈਟਾਂ);

	ਖਾਲੀ *p = mcheck_reallocfree_prehook(oldmem);
	size_t newsz = mcheck_alloc_prehook(ਬਾਈਟਾਂ);

	p = rEALLOc_impl(p, newsz);
	ਜੇ (!p)
		ਵਾਪਸ p;
	ਵਾਪਸ mcheck_alloc_noclean_posthook(p, ਬਾਈਟਾਂ);
}

Void_t *mEMALIGn(size_t alignment, size_t ਬਾਈਟਾਂ)
{
	mcheck_pedantic_prehook();
	size_t fullsz = mcheck_memalign_prehook(alignment, ਬਾਈਟਾਂ);
	ਖਾਲੀ *p = mEMALIGn_impl(alignment, fullsz);

	ਜੇ (!p)
		ਵਾਪਸ p;
	ਵਾਪਸ mcheck_memalign_posthook(alignment, p, ਬਾਈਟਾਂ);
}

// pvALLOc, vALLOc - redirect to mEMALIGn, defined here, so they need no wrapping.

Void_t *cALLOc(size_t n, size_t elem_ਮਾਪ)
{
	mcheck_pedantic_prehook();
	// NB: here is no overflow check.
	size_t fullsz = mcheck_alloc_prehook(n * elem_ਮਾਪ);
	ਖਾਲੀ *p = cALLOc_impl(1, fullsz);

	ਜੇ (!p)
		ਵਾਪਸ p;
	ਵਾਪਸ mcheck_alloc_noclean_posthook(p, n * elem_ਮਾਪ);
}

// mcheck API {
ਪੂਰਨ mcheck_pedantic(mcheck_abortfunc_t f)
{
	mcheck_initialize(f, 1);
	ਵਾਪਸ 0;
}

ਪੂਰਨ mcheck(mcheck_abortfunc_t f)
{
	mcheck_initialize(f, 0);
	ਵਾਪਸ 0;
}

ਖਾਲੀ mcheck_check_all(ਖਾਲੀ) { mcheck_pedantic_check(); }

ਗਿਣਤੀ mcheck_status mprobe(ਖਾਲੀ *__ptr) { ਵਾਪਸ mcheck_mprobe(__ptr); }
// mcheck API }
#ਜੇ_ਅੰਤ

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

#ਜੇ __STD_C
ਪੂਰਨ malloc_trim(size_t pad)
#ਨਹੀਂਤਾਂ
ਪੂਰਨ malloc_trim(pad) size_t pad;
#ਜੇ_ਅੰਤ
{
  ਲੰਮਾ  ਸਿਖਰ_ਮਾਪ;        /* Amount of top-most memory */
  ਲੰਮਾ  extra;           /* Amount to release */
  ਅੱਖਰ* current_brk;     /* address returned by pre-check sbrk call */
  ਅੱਖਰ* new_brk;         /* address returned by negative sbrk call */

  ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ pagesz = malloc_getpagesize;

  ਸਿਖਰ_ਮਾਪ = chunksize(ਸਿਖਰ);
  extra = ((ਸਿਖਰ_ਮਾਪ - pad - MINSIZE + (pagesz-1)) / pagesz - 1) * pagesz;

  ਜੇ (extra < (ਲੰਮਾ)pagesz)  /* Not enough memory to release */
    ਵਾਪਸ 0;

  ਨਹੀਂਤਾਂ
  {
    /* Test to make sure no one else called sbrk */
    current_brk = (ਅੱਖਰ*)(MORECORE (0));
    ਜੇ (current_brk != (ਅੱਖਰ*)(ਸਿਖਰ) + ਸਿਖਰ_ਮਾਪ)
      ਵਾਪਸ 0;     /* Apparently we don't own memory; must fail */

    ਨਹੀਂਤਾਂ
    {
      new_brk = (ਅੱਖਰ*)(MORECORE (-extra));

      ਜੇ (new_brk == (ਅੱਖਰ*)(MORECORE_FAILURE)) /* sbrk failed? */
      {
	/* Try to figure out what we have */
	current_brk = (ਅੱਖਰ*)(MORECORE (0));
	ਸਿਖਰ_ਮਾਪ = current_brk - (ਅੱਖਰ*)ਸਿਖਰ;
	ਜੇ (ਸਿਖਰ_ਮਾਪ >= (ਲੰਮਾ)MINSIZE) /* if not, we are very very dead! */
	{
	  sbrked_mem = current_brk - sbrk_ਅਧਾਰ;
	  ਸੈੱਟ_ਸਿਰ(ਸਿਖਰ, ਸਿਖਰ_ਮਾਪ | PREV_INUSE);
	}
	check_chunk(ਸਿਖਰ);
	ਵਾਪਸ 0;
      }

      ਨਹੀਂਤਾਂ
      {
	/* Success. Adjust top accordingly. */
	ਸੈੱਟ_ਸਿਰ(ਸਿਖਰ, (ਸਿਖਰ_ਮਾਪ - extra) | PREV_INUSE);
	sbrked_mem -= extra;
	check_chunk(ਸਿਖਰ);
	ਵਾਪਸ 1;
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

#ਜੇ __STD_C
size_t malloc_usable_size(Void_t* mem)
#ਨਹੀਂਤਾਂ
size_t malloc_usable_size(mem) Void_t* mem;
#ਜੇ_ਅੰਤ
{
  mchunkptr p;
  ਜੇ (mem == ਨਿਰਾ)
    ਵਾਪਸ 0;
  ਨਹੀਂਤਾਂ
  {
    p = mem2chunk(mem);
    ਜੇ(!chunk_is_mmapped(p))
    {
      ਜੇ (!inuse(p)) ਵਾਪਸ 0;
      check_inuse_chunk(p);
      ਵਾਪਸ chunksize(p) - ਮਾਪ_SZ;
    }
    ਵਾਪਸ chunksize(p) - 2*ਮਾਪ_SZ;
  }
}

/* Utility to update current_mallinfo for malloc_stats and mallinfo() */

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ DEBUG
ਸਥਿਰ ਖਾਲੀ malloc_update_mallinfo(ਖਾਲੀ)
{
  ਪੂਰਨ i;
  mbinptr b;
  mchunkptr p;
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ DEBUG
  mchunkptr q;
#ਜੇ_ਅੰਤ

  ਅੰਦਰੂਨੀ_ਮਾਪ_T avail = chunksize(ਸਿਖਰ);
  ਪੂਰਨ   navail = ((ਲੰਮਾ)(avail) >= (ਲੰਮਾ)MINSIZE)? 1 : 0;

  ਲਈ (i = 1; i < NAV; ++i)
  {
    b = bin_at(i);
    ਲਈ (p = ਆਖਰੀ(b); p != b; p = p->bk)
    {
#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ DEBUG
      check_free_chunk(p);
      ਲਈ (q = ਅਗਲਾ_chunk(p);
	   q < ਸਿਖਰ && inuse(q) && (ਲੰਮਾ)(chunksize(q)) >= (ਲੰਮਾ)MINSIZE;
	   q = ਅਗਲਾ_chunk(q))
	check_inuse_chunk(q);
#ਜੇ_ਅੰਤ
      avail += chunksize(p);
      navail++;
    }
  }

  current_mallinfo.ordblks = navail;
  current_mallinfo.uordblks = sbrked_mem - avail;
  current_mallinfo.fordblks = avail;
  current_mallinfo.hblks = n_mmaps;
  current_mallinfo.hblkhd = mmapped_mem;
  current_mallinfo.keepcost = chunksize(ਸਿਖਰ);

}
#ਜੇ_ਅੰਤ	/* DEBUG */

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

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ DEBUG
ਖਾਲੀ malloc_stats(ਖਾਲੀ)
{
  malloc_update_mallinfo();
  ਲਿਖੋ("max system bytes = %10u\n",
	  (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)(max_ਕੁੱਲ_mem));
  ਲਿਖੋ("system bytes     = %10u\n",
	  (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)(sbrked_mem + mmapped_mem));
  ਲਿਖੋ("in use bytes     = %10u\n",
	  (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)(current_mallinfo.uordblks + mmapped_mem));
#ਜੇ HAVE_MMAP
  ਲਿਖੋ("max mmap regions = %10u\n",
	  (ਬਿਨਾਨਿਸ਼ਾਨੀ ਪੂਰਨ)max_n_mmaps);
#ਜੇ_ਅੰਤ
}
#ਜੇ_ਅੰਤ	/* DEBUG */

/*
  mallinfo returns a copy of updated current mallinfo.
*/

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ DEBUG
ਬਣਤਰ mallinfo mALLINFo(ਖਾਲੀ)
{
  malloc_update_mallinfo();
  ਵਾਪਸ current_mallinfo;
}
#ਜੇ_ਅੰਤ	/* DEBUG */

/*
  mallopt:

    mallopt is the general SVID/XPG interface to tunable parameters.
    The format is to provide a (parameter-number, parameter-value) pair.
    mallopt then sets the corresponding parameter to the argument
    value if it can (i.e., so long as the value is meaningful),
    and returns 1 if successful else 0.

    See descriptions of tunable parameters above.

*/

#ਜੇ __STD_C
ਪੂਰਨ mALLOPt(ਪੂਰਨ param_ਸੰਖਿਆ, ਪੂਰਨ ਮੁੱਲ)
#ਨਹੀਂਤਾਂ
ਪੂਰਨ mALLOPt(param_ਸੰਖਿਆ, ਮੁੱਲ) ਪੂਰਨ param_ਸੰਖਿਆ; ਪੂਰਨ ਮੁੱਲ;
#ਜੇ_ਅੰਤ
{
  ਚੋਣ(param_ਸੰਖਿਆ)
  {
    ਹਾਲਤ M_TRIM_THRESHOLD:
      trim_ਹੱਦ = ਮੁੱਲ; ਵਾਪਸ 1;
    ਹਾਲਤ M_TOP_PAD:
      ਸਿਖਰ_pad = ਮੁੱਲ; ਵਾਪਸ 1;
    ਹਾਲਤ M_MMAP_THRESHOLD:
      mmap_ਹੱਦ = ਮੁੱਲ; ਵਾਪਸ 1;
    ਹਾਲਤ M_MMAP_MAX:
#ਜੇ HAVE_MMAP
      n_mmaps_max = ਮੁੱਲ; ਵਾਪਸ 1;
#ਨਹੀਂਤਾਂ
      ਜੇ (ਮੁੱਲ != 0) ਵਾਪਸ 0; ਨਹੀਂਤਾਂ  n_mmaps_max = ਮੁੱਲ; ਵਾਪਸ 1;
#ਜੇ_ਅੰਤ

    ਮੂਲ:
      ਵਾਪਸ 0;
  }
}

ਪੂਰਨ initf_malloc(ਖਾਲੀ)
{
#ਜੇ CONFIG_IS_ENABLED(SYS_MALLOC_F)
	ਪ੍ਰਤਿਜ੍ਞਾ(gd->malloc_ਅਧਾਰ);	/* Set up by crt0.S */
	gd->malloc_ਹੱਦਬੰਦੀ = CONFIG_VAL(SYS_MALLOC_F_LEN);
	gd->malloc_ptr = 0;
#ਜੇ_ਅੰਤ

	ਵਾਪਸ 0;
}

ਖਾਲੀ malloc_enable_testing(ਪੂਰਨ max_allocs)
{
	malloc_testing = true;
	malloc_max_allocs = max_allocs;
}

ਖਾਲੀ malloc_disable_testing(ਖਾਲੀ)
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
