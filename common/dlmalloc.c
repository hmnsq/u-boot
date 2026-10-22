// SPDX-License-Identifier: GPL-2.0+
/*
 * This code is based on a version (aka dlmalloc) of malloc/free/realloc written
 * by Doug Lea and released to the public domain, as explained at
 * http://creativecommons.org/publicdomain/zero/1.0/-
 *
 * The original code is available at http://gee.cs.oswego.edu/pub/misc/
 * as file malloc-2.6.6.c.
 */

#ଯଦି CONFIG_IS_ENABLED(UNIT_TEST)
#define DEBUG
#endif

#ଅନ୍ତର୍ଭାଵ <log.h>
#ଅନ୍ତର୍ଭାଵ <asm/global_data.h>

#ଅନ୍ତର୍ଭାଵ <malloc.h>
#ଅନ୍ତର୍ଭାଵ <mapmem.h>
#ଅନ୍ତର୍ଭାଵ <string.h>
#ଅନ୍ତର୍ଭାଵ <asm/io.h>
#ଅନ୍ତର୍ଭାଵ <valgrind/memcheck.h>

#ifdef DEBUG
#ଯଦି __STD_C
ସ୍ଥିର ଶୂନ୍ୟ malloc_update_mallinfo (ଶୂନ୍ୟ);
ଶୂନ୍ୟ malloc_stats (ଶୂନ୍ୟ);
#ଅନ୍ୟଥା
ସ୍ଥିର ଶୂନ୍ୟ malloc_update_mallinfo ();
ଶୂନ୍ୟ malloc_stats();
#endif
#endif	/* DEBUG */

DECLARE_GLOBAL_DATA_PTR;

#ifdef MCHECK_HEAP_PROTECTION
 #define STATIC_IF_MCHECK ସ୍ଥିର
 #undef MALLOC_COPY
 #undef MALLOC_ZERO
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ MALLOC_ZERO(ଶୂନ୍ୟ *p, size_t sz) { memset(p, 0, sz); }
ସ୍ଥିର ଧାଡ଼ିଭିତରେ ଶୂନ୍ୟ MALLOC_COPY(ଶୂନ୍ୟ *dest, ଅଚଳ ଶୂନ୍ୟ *src, size_t sz) { memcpy(dest, src, sz); }
#ଅନ୍ୟଥା
 #define STATIC_IF_MCHECK
 #define mALLOc_impl mALLOc
 #define fREe_impl fREe
 #define rEALLOc_impl rEALLOc
 #define mEMALIGn_impl mEMALIGn
 #define cALLOc_impl cALLOc
#endif

/*
  Emulation of sbrk for WIN32
  All code within the ifdef WIN32 is untested by me.

  Thanks to Martin Fong and others for supplying this.
*/

#ifdef WIN32

#define AlignPage(add) (((add) + (malloc_getpagesize-1)) & \
~(malloc_getpagesize-1))
#define AlignPage64K(add) (((add) + (0x10000 - 1)) & ~(0x10000 - 1))

/* resrve 64MB to insure large contiguous space */
#define RESERVED_SIZE (1024*1024*64)
#define NEXT_SIZE (2048*1024)
#define ଶୀର୍ଷ_MEMORY ((unsigned long)2*1024*1024*1024)

ସଂରଚନା GmListElement;
ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା GmListElement GmListElement;

ସଂରଚନା GmListElement
{
	GmListElement* next;
	ଶୂନ୍ୟ* base;
};

ସ୍ଥିର GmListElement* head = 0;
ସ୍ଥିର unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା gNextAddress = 0;
ସ୍ଥିର unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା gAddressBase = 0;
ସ୍ଥିର unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା gAllocatedSize = 0;

ସ୍ଥିର
GmListElement* makeGmListElement (ଶୂନ୍ୟ* bas)
{
	GmListElement* this;
	this = (GmListElement*)(ଶୂନ୍ୟ*)LocalAlloc (0, ଆକାର (GmListElement));
	ପ୍ରତିଜ୍ଞା (this);
	ଯଦି (this)
	{
		this->base = bas;
		this->next = head;
		head = this;
	}
	ଫେରନ୍ତୁ this;
}

ଶୂନ୍ୟ gcleanup (ଶୂନ୍ୟ)
{
	BOOL rval;
	ପ୍ରତିଜ୍ଞା ( (head == ଖାଲି) || (head->base == (ଶୂନ୍ୟ*)gAddressBase));
	ଯଦି (gAddressBase && (gNextAddress - gAddressBase))
	{
		rval = VirtualFree ((ଶୂନ୍ୟ*)gAddressBase,
							gNextAddress - gAddressBase,
							MEM_DECOMMIT);
	ପ୍ରତିଜ୍ଞା (rval);
	}
	ଯେତେବେଳେ (head)
	{
		GmListElement* next = head->next;
		rval = VirtualFree (head->base, 0, MEM_RELEASE);
		ପ୍ରତିଜ୍ଞା (rval);
		LocalFree (head);
		head = next;
	}
}

ସ୍ଥିର
ଶୂନ୍ୟ* findRegion (ଶୂନ୍ୟ* start_address, unsigned long size)
{
	MEMORY_BASIC_INFORMATION info;
	ଯଦି (size >= ଶୀର୍ଷ_MEMORY) ଫେରନ୍ତୁ ଖାଲି;

	ଯେତେବେଳେ ((unsigned long)start_address + size < ଶୀର୍ଷ_MEMORY)
	{
		VirtualQuery (start_address, &info, ଆକାର (info));
		ଯଦି ((info.State == MEM_FREE) && (info.RegionSize >= size))
			ଫେରନ୍ତୁ start_address;
		ଅନ୍ୟଥା
		{
			/* Requested region is not available so see if the */
			/* next region is available.  Set 'start_address' */
			/* to the next region and call 'VirtualQuery()' */
			/* again. */

			start_address = (ଅକ୍ଷର*)info.BaseAddress + info.RegionSize;

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

			start_address =
				(ଶୂନ୍ୟ *) AlignPage64K((unsigned long) start_address);
		}
	}
	ଫେରନ୍ତୁ ଖାଲି;

}

ଶୂନ୍ୟ* wsbrk (long size)
{
	ଶୂନ୍ୟ* tmp;
	ଯଦି (size > 0)
	{
		ଯଦି (gAddressBase == 0)
		{
			gAllocatedSize = max (RESERVED_SIZE, AlignPage (size));
			gNextAddress = gAddressBase =
				(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)VirtualAlloc (ଖାଲି, gAllocatedSize,
											MEM_RESERVE, PAGE_NOACCESS);
		} ଅନ୍ୟଥା ଯଦି (AlignPage (gNextAddress + size) > (gAddressBase +
gAllocatedSize))
		{
			long new_size = max (NEXT_SIZE, AlignPage (size));
			ଶୂନ୍ୟ* new_address = (ଶୂନ୍ୟ*)(gAddressBase+gAllocatedSize);
			କର
			{
				new_address = findRegion (new_address, new_size);

				ଯଦି (!new_address)
					ଫେରନ୍ତୁ (ଶୂନ୍ୟ*)-1;

				gAddressBase = gNextAddress =
					(unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)VirtualAlloc (new_address, new_size,
												MEM_RESERVE, PAGE_NOACCESS);
				/* repeat in case of race condition */
				/* The region that we found has been snagged */
				/* by another thread */
			}
			ଯେତେବେଳେ (gAddressBase == 0);

			ପ୍ରତିଜ୍ଞା (new_address == (ଶୂନ୍ୟ*)gAddressBase);

			gAllocatedSize = new_size;

			ଯଦି (!makeGmListElement ((ଶୂନ୍ୟ*)gAddressBase))
				ଫେରନ୍ତୁ (ଶୂନ୍ୟ*)-1;
		}
		ଯଦି ((size + gNextAddress) > AlignPage (gNextAddress))
		{
			ଶୂନ୍ୟ* res;
			res = VirtualAlloc ((ଶୂନ୍ୟ*)AlignPage (gNextAddress),
								(size + gNextAddress -
								 AlignPage (gNextAddress)),
								MEM_COMMIT, PAGE_READWRITE);
			ଯଦି (!res)
				ଫେରନ୍ତୁ (ଶୂନ୍ୟ*)-1;
		}
		tmp = (ଶୂନ୍ୟ*)gNextAddress;
		gNextAddress = (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)tmp + size;
		ଫେରନ୍ତୁ tmp;
	}
	ଅନ୍ୟଥା ଯଦି (size < 0)
	{
		unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା alignedGoal = AlignPage (gNextAddress + size);
		/* Trim by releasing the virtual memory */
		ଯଦି (alignedGoal >= gAddressBase)
		{
			VirtualFree ((ଶୂନ୍ୟ*)alignedGoal, gNextAddress - alignedGoal,
						 MEM_DECOMMIT);
			gNextAddress = gNextAddress + size;
			ଫେରନ୍ତୁ (ଶୂନ୍ୟ*)gNextAddress;
		}
		ଅନ୍ୟଥା
		{
			VirtualFree ((ଶୂନ୍ୟ*)gAddressBase, gNextAddress - gAddressBase,
						 MEM_DECOMMIT);
			gNextAddress = gAddressBase;
			ଫେରନ୍ତୁ (ଶୂନ୍ୟ*)-1;
		}
	}
	ଅନ୍ୟଥା
	{
		ଫେରନ୍ତୁ (ଶୂନ୍ୟ*)gNextAddress;
	}
}

#endif

/*
  Type declarations
*/

ସଂରଚନା malloc_chunk
{
  INTERNAL_SIZE_T prev_size; /* Size of previous chunk (if free). */
  INTERNAL_SIZE_T size;      /* Size in bytes, including overhead. */
  ସଂରଚନା malloc_chunk* fd;   /* double links -- used only if free. */
  ସଂରଚନା malloc_chunk* bk;
} __attribute__((__may_alias__)) ;

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା malloc_chunk* mchunkptr;

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

#define SIZE_SZ                (ଆକାର(INTERNAL_SIZE_T))
#define MALLOC_ALIGNMENT       (SIZE_SZ + SIZE_SZ)
#define MALLOC_ALIGN_MASK      (MALLOC_ALIGNMENT - 1)
#define MINSIZE                (ଆକାର(ସଂରଚନା malloc_chunk))

/* conversion from malloc headers to user pointers, and back */

#define chunk2mem(p)   ((Void_t*)((ଅକ୍ଷର*)(p) + 2*SIZE_SZ))
#define mem2chunk(mem) ((mchunkptr)((ଅକ୍ଷର*)(mem) - 2*SIZE_SZ))

/* pad request bytes into a usable size */

#define request2size(req) \
 ((((req) + (SIZE_SZ + MALLOC_ALIGN_MASK)) < \
  (MINSIZE + MALLOC_ALIGN_MASK)) ? MINSIZE : \
   (((req) + (SIZE_SZ + MALLOC_ALIGN_MASK)) & ~(MALLOC_ALIGN_MASK)))

/* Check if m has acceptable alignment */

#define aligned_OK(m)    (((unsigned long)((m)) & (MALLOC_ALIGN_MASK)) == 0)

/*
  Physical chunk operations
*/

/* size field is or'ed with PREV_INUSE when previous adjacent chunk in use */

#define PREV_INUSE 0x1

/* size field is or'ed with IS_MMAPPED if the chunk was obtained with mmap() */

#define IS_MMAPPED 0x2

/* Bits to mask off when extracting size */

#define SIZE_BITS (PREV_INUSE|IS_MMAPPED)

/* Ptr to next physical malloc_chunk. */

#define next_chunk(p) ((mchunkptr)( ((ଅକ୍ଷର*)(p)) + ((p)->size & ~PREV_INUSE) ))

/* Ptr to previous physical malloc_chunk */

#define prev_chunk(p)\
   ((mchunkptr)( ((ଅକ୍ଷର*)(p)) - ((p)->prev_size) ))

/* Treat space at ptr + offset as a chunk */

#define chunk_at_offset(p, s)  ((mchunkptr)(((ଅକ୍ଷର*)(p)) + (s)))

/*
  Dealing with use bits
*/

/* extract p's inuse bit */

#define inuse(p)\
((((mchunkptr)(((ଅକ୍ଷର*)(p))+((p)->size & ~PREV_INUSE)))->size) & PREV_INUSE)

/* extract inuse bit of previous chunk */

#define prev_inuse(p)  ((p)->size & PREV_INUSE)

/* check for mmap()'ed chunk */

#define chunk_is_mmapped(p) ((p)->size & IS_MMAPPED)

/* set/clear chunk as in use without otherwise disturbing */

#define set_inuse(p)\
((mchunkptr)(((ଅକ୍ଷର*)(p)) + ((p)->size & ~PREV_INUSE)))->size |= PREV_INUSE

#define clear_inuse(p)\
((mchunkptr)(((ଅକ୍ଷର*)(p)) + ((p)->size & ~PREV_INUSE)))->size &= ~(PREV_INUSE)

/* check/set/clear inuse bits in known places */

#define inuse_bit_at_offset(p, s)\
 (((mchunkptr)(((ଅକ୍ଷର*)(p)) + (s)))->size & PREV_INUSE)

#define set_inuse_bit_at_offset(p, s)\
 (((mchunkptr)(((ଅକ୍ଷର*)(p)) + (s)))->size |= PREV_INUSE)

#define clear_inuse_bit_at_offset(p, s)\
 (((mchunkptr)(((ଅକ୍ଷର*)(p)) + (s)))->size &= ~(PREV_INUSE))

/*
  Dealing with size fields
*/

/* Get size, ignoring use bits */

#define chunksize(p)          ((p)->size & ~(SIZE_BITS))

/* Set size at head, without disturbing its use bit */

#define set_head_size(p, s)   ((p)->size = (((p)->size & PREV_INUSE) | (s)))

/* Set size/use ignoring previous bits in header */

#define set_head(p, s)        ((p)->size = (s))

/* Set size at footer (only when chunk is not in use) */

#define set_foot(p, s)   (((mchunkptr)((ଅକ୍ଷର*)(p) + (s)))->prev_size = (s))

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

#define NAV             128   /* number of bins */

ପ୍ରକାରବ୍ୟାଖ୍ୟା ସଂରଚନା malloc_chunk* mbinptr;

/* access macros */

#define bin_at(i)      ((mbinptr)((ଅକ୍ଷର*)&(av_[2*(i) + 2]) - 2*SIZE_SZ))
#define next_bin(b)    ((mbinptr)((ଅକ୍ଷର*)(b) + 2 * ଆକାର(mbinptr)))
#define prev_bin(b)    ((mbinptr)((ଅକ୍ଷର*)(b) - 2 * ଆକାର(mbinptr)))

/*
   The first 2 bins are never indexed. The corresponding av_ cells are instead
   used for bookkeeping. This is not to save space, but to simplify
   indexing, maintain locality, and avoid some initialization tests.
*/

#define ଶୀର୍ଷ            (av_[2])          /* The topmost chunk */
#define last_remainder (bin_at(1))       /* remainder from last split */

/*
   Because top initially points to its own bin with initial
   zero size, thus forcing extension on the first malloc request,
   we avoid having any special code in malloc to check whether
   it even exists yet. But we still need to in malloc_extend_top.
*/

#define initial_ଶୀର୍ଷ    ((mchunkptr)(bin_at(0)))

/* Helper macro to initialize bins */

#define IAV(i)  bin_at(i), bin_at(i)

ସ୍ଥିର mbinptr av_[NAV * 2 + 2] = {
 ଖାଲି, ଖାଲି,
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

#ifdef CONFIG_SYS_MALLOC_DEFAULT_TO_INIT
ସ୍ଥିର ଶୂନ୍ୟ malloc_init(ଶୂନ୍ୟ);
#endif

ulong mem_malloc_start = 0;
ulong mem_malloc_end = 0;
ulong mem_malloc_brk = 0;

ସ୍ଥିର bool malloc_testing;	/* enable test mode */
ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା malloc_max_allocs;	/* return NULL after this many calls to malloc() */

ଶୂନ୍ୟ *sbrk(ptrdiff_t increment)
{
	ulong old = mem_malloc_brk;
	ulong new = old + increment;

	ଯଦି ((new < mem_malloc_start) || (new > mem_malloc_end))
		ଫେରନ୍ତୁ (ଶୂନ୍ୟ *)MORECORE_FAILURE;

	/*
	 * if we are giving memory back make sure we clear it out since
	 * we set MORECORE_CLEARS to 1
	 */
	ଯଦି (increment < 0)
		memset((ଶୂନ୍ୟ *)new, 0, -increment);

	mem_malloc_brk = new;

	ଫେରନ୍ତୁ (ଶୂନ୍ୟ *)old;
}

ଶୂନ୍ୟ mem_malloc_init(ulong start, ulong size)
{
	mem_malloc_start = (ulong)map_sysmem(start, size);
	mem_malloc_end = mem_malloc_start + size;
	mem_malloc_brk = mem_malloc_start;

#ifdef CONFIG_SYS_MALLOC_DEFAULT_TO_INIT
	malloc_init();
#endif

	debug("using memory %#lx-%#lx for malloc()\n", mem_malloc_start,
	      mem_malloc_end);
#ଯଦି CONFIG_IS_ENABLED(SYS_MALLOC_CLEAR_ON_INIT)
	memset((ଶୂନ୍ୟ *)mem_malloc_start, 0x0, size);
#endif
}

/* field-extraction macros */

#define first(b) ((b)->fd)
#define last(b)  ((b)->bk)

/*
  Indexing into bins
*/

#define bin_index(sz)                                                          \
(((((unsigned long)(sz)) >> 9) ==    0) ?       (((unsigned long)(sz)) >>  3): \
 ((((unsigned long)(sz)) >> 9) <=    4) ?  56 + (((unsigned long)(sz)) >>  6): \
 ((((unsigned long)(sz)) >> 9) <=   20) ?  91 + (((unsigned long)(sz)) >>  9): \
 ((((unsigned long)(sz)) >> 9) <=   84) ? 110 + (((unsigned long)(sz)) >> 12): \
 ((((unsigned long)(sz)) >> 9) <=  340) ? 119 + (((unsigned long)(sz)) >> 15): \
 ((((unsigned long)(sz)) >> 9) <= 1364) ? 124 + (((unsigned long)(sz)) >> 18): \
					  126)
/*
  bins for chunks < 512 are all spaced 8 bytes apart, and hold
  identically sized chunks. This is exploited in malloc.
*/

#define MAX_SMALLBIN         63
#define MAX_SMALLBIN_SIZE   512
#define SMALLBIN_WIDTH        8

#define smallbin_index(sz)  (((unsigned long)(sz)) >> 3)

/*
   Requests are `small' if both the corresponding and the next bin are small
*/

#define is_small_request(nb) (nb < MAX_SMALLBIN_SIZE - SMALLBIN_WIDTH)

/*
    To help compensate for the large number of bins, a one-level index
    structure is used for bin-by-bin searching.  `binblocks' is a
    one-word bitvector recording whether groups of BINBLOCKWIDTH bins
    have any (possibly) non-empty bins, so they can be skipped over
    all at once during during traversals. The bits are NOT always
    cleared as soon as all bins in a block are empty, but instead only
    when all are noticed to be empty during traversal in malloc.
*/

#define BINBLOCKWIDTH     4   /* bins per block */

#define binblocks_r     ((INTERNAL_SIZE_T)av_[1]) /* bitvector of nonempty blocks */
#define binblocks_w     (av_[1])

/* bin<->block macros */

#define idx2binblock(ix)    ((unsigned)1 << (ix / BINBLOCKWIDTH))
#define mark_binblock(ii)   (binblocks_w = (mbinptr)(binblocks_r | idx2binblock(ii)))
#define clear_binblock(ii)  (binblocks_w = (mbinptr)(binblocks_r & ~(idx2binblock(ii))))

/*  Other static bookkeeping data */

/* variables holding tunable values */

ସ୍ଥିର unsigned long trim_threshold   = DEFAULT_TRIM_THRESHOLD;
ସ୍ଥିର unsigned long ଶୀର୍ଷ_pad          = DEFAULT_ଶୀର୍ଷ_PAD;
ସ୍ଥିର unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା  n_mmaps_max      = DEFAULT_MMAP_MAX;
ସ୍ଥିର unsigned long mmap_threshold   = DEFAULT_MMAP_THRESHOLD;

/* The first value returned from sbrk */
ସ୍ଥିର ଅକ୍ଷର* sbrk_base = (ଅକ୍ଷର*)(-1);

/* The maximum memory obtained from system via sbrk */
ସ୍ଥିର unsigned long max_sbrked_mem = 0;

/* The maximum via either sbrk or mmap */
ସ୍ଥିର unsigned long max_total_mem = 0;

/* internal working copy of mallinfo */
ସ୍ଥିର ସଂରଚନା mallinfo current_mallinfo = {  0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

/* The total memory obtained from system via sbrk */
#define sbrked_mem  (current_mallinfo.arena)

/* Tracking mmaps */

#ifdef DEBUG
ସ୍ଥିର unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା n_mmaps = 0;
#endif	/* DEBUG */
ସ୍ଥିର unsigned long mmapped_mem = 0;
#ଯଦି HAVE_MMAP
ସ୍ଥିର unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା max_n_mmaps = 0;
ସ୍ଥିର unsigned long max_mmapped_mem = 0;
#endif

#ifdef CONFIG_SYS_MALLOC_DEFAULT_TO_INIT
ସ୍ଥିର ଶୂନ୍ୟ malloc_init(ଶୂନ୍ୟ)
{
	ପୂର୍ଣ୍ଣସଂଖ୍ୟା i, j;

	debug("bins (av_ array) are at %p\n", (ଶୂନ୍ୟ *)av_);

	av_[0] = ଖାଲି; av_[1] = ଖାଲି;
	ପାଇଁ (i = 2, j = 2; i < NAV * 2 + 2; i += 2, j++) {
		av_[i] = bin_at(j - 2);
		av_[i + 1] = bin_at(j - 2);

		/* Just print the first few bins so that
		 * we can see there are alright.
		 */
		ଯଦି (i < 10)
			debug("av_[%d]=%lx av_[%d]=%lx\n",
			      i, (ulong)av_[i],
			      i + 1, (ulong)av_[i + 1]);
	}

	/* Init the static bookkeeping as well */
	sbrk_base = (ଅକ୍ଷର *)(-1);
	max_sbrked_mem = 0;
	max_total_mem = 0;
#ifdef DEBUG
	memset((ଶୂନ୍ୟ *)&current_mallinfo, 0, ଆକାର(ସଂରଚନା mallinfo));
#endif
}
#endif

/*
  Debugging support
*/

#ifdef DEBUG

/*
  These routines make a number of assertions about the states
  of data structures that should be true at all times. If any
  are not true, it's very likely that a user program has somehow
  trashed memory. (It's also possible that there is a coding error
  in malloc. In which case, please report it!)
*/

#ଯଦି __STD_C
ସ୍ଥିର ଶୂନ୍ୟ do_check_chunk(mchunkptr p)
#ଅନ୍ୟଥା
ସ୍ଥିର ଶୂନ୍ୟ do_check_chunk(p) mchunkptr p;
#endif
{
  INTERNAL_SIZE_T sz = p->size & ~PREV_INUSE;

  /* No checkable chunk is mmapped */
  ପ୍ରତିଜ୍ଞା(!chunk_is_mmapped(p));

  /* Check for legal address ... */
  ପ୍ରତିଜ୍ଞା((ଅକ୍ଷର*)p >= sbrk_base);
  ଯଦି (p != ଶୀର୍ଷ)
    ପ୍ରତିଜ୍ଞା((ଅକ୍ଷର*)p + sz <= (ଅକ୍ଷର*)ଶୀର୍ଷ);
  ଅନ୍ୟଥା
    ପ୍ରତିଜ୍ଞା((ଅକ୍ଷର*)p + sz <= sbrk_base + sbrked_mem);

}

#ଯଦି __STD_C
ସ୍ଥିର ଶୂନ୍ୟ do_check_free_chunk(mchunkptr p)
#ଅନ୍ୟଥା
ସ୍ଥିର ଶୂନ୍ୟ do_check_free_chunk(p) mchunkptr p;
#endif
{
  INTERNAL_SIZE_T sz = p->size & ~PREV_INUSE;
  mchunkptr next = chunk_at_offset(p, sz);

  do_check_chunk(p);

  /* Check whether it claims to be free ... */
  ପ୍ରତିଜ୍ଞା(!inuse(p));

  /* Unless a special marker, must have OK fields */
  ଯଦି ((long)sz >= (long)MINSIZE)
  {
    ପ୍ରତିଜ୍ଞା((sz & MALLOC_ALIGN_MASK) == 0);
    ପ୍ରତିଜ୍ଞା(aligned_OK(chunk2mem(p)));
    /* ... matching footer field */
    ପ୍ରତିଜ୍ଞା(next->prev_size == sz);
    /* ... and is fully consolidated */
    ପ୍ରତିଜ୍ଞା(prev_inuse(p));
    ପ୍ରତିଜ୍ଞା (next == ଶୀର୍ଷ || inuse(next));

    /* ... and has minimally sane links */
    ପ୍ରତିଜ୍ଞା(p->fd->bk == p);
    ପ୍ରତିଜ୍ଞା(p->bk->fd == p);
  }
  ଅନ୍ୟଥା /* markers are always of size SIZE_SZ */
    ପ୍ରତିଜ୍ଞା(sz == SIZE_SZ);
}

#ଯଦି __STD_C
ସ୍ଥିର ଶୂନ୍ୟ do_check_inuse_chunk(mchunkptr p)
#ଅନ୍ୟଥା
ସ୍ଥିର ଶୂନ୍ୟ do_check_inuse_chunk(p) mchunkptr p;
#endif
{
  mchunkptr next = next_chunk(p);
  do_check_chunk(p);

  /* Check whether it claims to be in use ... */
  ପ୍ରତିଜ୍ଞା(inuse(p));

  /* ... and is surrounded by OK chunks.
    Since more things can be checked with free chunks than inuse ones,
    if an inuse chunk borders them and debug is on, it's worth doing them.
  */
  ଯଦି (!prev_inuse(p))
  {
    mchunkptr prv = prev_chunk(p);
    ପ୍ରତିଜ୍ଞା(next_chunk(prv) == p);
    do_check_free_chunk(prv);
  }
  ଯଦି (next == ଶୀର୍ଷ)
  {
    ପ୍ରତିଜ୍ଞା(prev_inuse(next));
    ପ୍ରତିଜ୍ଞା(chunksize(next) >= MINSIZE);
  }
  ଅନ୍ୟଥା ଯଦି (!inuse(next))
    do_check_free_chunk(next);

}

#ଯଦି __STD_C
ସ୍ଥିର ଶୂନ୍ୟ do_check_malloced_chunk(mchunkptr p, INTERNAL_SIZE_T s)
#ଅନ୍ୟଥା
ସ୍ଥିର ଶୂନ୍ୟ do_check_malloced_chunk(p, s) mchunkptr p; INTERNAL_SIZE_T s;
#endif
{
  INTERNAL_SIZE_T sz = p->size & ~PREV_INUSE;
  long room = sz - s;

  do_check_inuse_chunk(p);

  /* Legal size ... */
  ପ୍ରତିଜ୍ଞା((long)sz >= (long)MINSIZE);
  ପ୍ରତିଜ୍ଞା((sz & MALLOC_ALIGN_MASK) == 0);
  ପ୍ରତିଜ୍ଞା(room >= 0);
  ପ୍ରତିଜ୍ଞା(room < (long)MINSIZE);

  /* ... and alignment */
  ପ୍ରତିଜ୍ଞା(aligned_OK(chunk2mem(p)));

  /* ... and was allocated at front of an available chunk */
  ପ୍ରତିଜ୍ଞା(prev_inuse(p));

}

#define check_free_chunk(P)  do_check_free_chunk(P)
#define check_inuse_chunk(P) do_check_inuse_chunk(P)
#define check_chunk(P) do_check_chunk(P)
#define check_malloced_chunk(P,N) do_check_malloced_chunk(P,N)
#ଅନ୍ୟଥା
#define check_free_chunk(P)
#define check_inuse_chunk(P)
#define check_chunk(P)
#define check_malloced_chunk(P,N)
#endif

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

#define frontlink(P, S, IDX, BK, FD)                                          \
{                                                                             \
  ଯଦି (S < MAX_SMALLBIN_SIZE)                                                  \
  {                                                                           \
    IDX = smallbin_index(S);                                                  \
    mark_binblock(IDX);                                                       \
    BK = bin_at(IDX);                                                         \
    FD = BK->fd;                                                              \
    P->bk = BK;                                                               \
    P->fd = FD;                                                               \
    FD->bk = BK->fd = P;                                                      \
  }                                                                           \
  ଅନ୍ୟଥା                                                                        \
  {                                                                           \
    IDX = bin_index(S);                                                       \
    BK = bin_at(IDX);                                                         \
    FD = BK->fd;                                                              \
    ଯଦି (FD == BK) mark_binblock(IDX);                                         \
    ଅନ୍ୟଥା                                                                      \
    {                                                                         \
      ଯେତେବେଳେ (FD != BK && S < chunksize(FD)) FD = FD->fd;                      \
      BK = FD->bk;                                                            \
    }                                                                         \
    P->bk = BK;                                                               \
    P->fd = FD;                                                               \
    FD->bk = BK->fd = P;                                                      \
  }                                                                           \
}

/* take a chunk off a list */

#define unlink(P, BK, FD)                                                     \
{                                                                             \
  BK = P->bk;                                                                 \
  FD = P->fd;                                                                 \
  FD->bk = BK;                                                                \
  BK->fd = FD;                                                                \
}                                                                             \

/* Place p as the last remainder */

#define link_last_remainder(P)                                                \
{                                                                             \
  last_remainder->fd = last_remainder->bk =  P;                               \
  P->fd = P->bk = last_remainder;                                             \
}

/* Clear the last_remainder bin */

#define clear_last_remainder \
  (last_remainder->fd = last_remainder->bk = last_remainder)

/* Routines dealing with mmap(). */

#ଯଦି HAVE_MMAP

#ଯଦି __STD_C
ସ୍ଥିର mchunkptr mmap_chunk(size_t size)
#ଅନ୍ୟଥା
ସ୍ଥିର mchunkptr mmap_chunk(size) size_t size;
#endif
{
  size_t page_mask = malloc_getpagesize - 1;
  mchunkptr p;

#ifndef MAP_ANONYMOUS
  ସ୍ଥିର ପୂର୍ଣ୍ଣସଂଖ୍ୟା fd = -1;
#endif

  ଯଦି(n_mmaps >= n_mmaps_max) ଫେରନ୍ତୁ 0; /* too many regions */

  /* For mmapped chunks, the overhead is one SIZE_SZ unit larger, because
   * there is no following chunk whose prev_size field could be used.
   */
  size = (size + SIZE_SZ + page_mask) & ~page_mask;

#ifdef MAP_ANONYMOUS
  p = (mchunkptr)mmap(0, size, PROT_READ|PROT_WRITE,
		      MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
#ଅନ୍ୟଥା /* !MAP_ANONYMOUS */
  ଯଦି (fd < 0)
  {
    fd = open("/dev/zero", O_RDWR);
    ଯଦି(fd < 0) ଫେରନ୍ତୁ 0;
  }
  p = (mchunkptr)mmap(0, size, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0);
#endif

  ଯଦି(p == (mchunkptr)-1) ଫେରନ୍ତୁ 0;

  n_mmaps++;
  ଯଦି (n_mmaps > max_n_mmaps) max_n_mmaps = n_mmaps;

  /* We demand that eight bytes into a page must be 8-byte aligned. */
  ପ୍ରତିଜ୍ଞା(aligned_OK(chunk2mem(p)));

  /* The offset to the start of the mmapped region is stored
   * in the prev_size field of the chunk; normally it is zero,
   * but that can be changed in memalign().
   */
  p->prev_size = 0;
  set_head(p, size|IS_MMAPPED);

  mmapped_mem += size;
  ଯଦି ((unsigned long)mmapped_mem > (unsigned long)max_mmapped_mem)
    max_mmapped_mem = mmapped_mem;
  ଯଦି ((unsigned long)(mmapped_mem + sbrked_mem) > (unsigned long)max_total_mem)
    max_total_mem = mmapped_mem + sbrked_mem;
  ଫେରନ୍ତୁ p;
}

#ଯଦି __STD_C
ସ୍ଥିର ଶୂନ୍ୟ munmap_chunk(mchunkptr p)
#ଅନ୍ୟଥା
ସ୍ଥିର ଶୂନ୍ୟ munmap_chunk(p) mchunkptr p;
#endif
{
  INTERNAL_SIZE_T size = chunksize(p);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା ret;

  ପ୍ରତିଜ୍ଞା (chunk_is_mmapped(p));
  ପ୍ରତିଜ୍ଞା(! ((ଅକ୍ଷର*)p >= sbrk_base && (ଅକ୍ଷର*)p < sbrk_base + sbrked_mem));
  ପ୍ରତିଜ୍ଞା((n_mmaps > 0));
  ପ୍ରତିଜ୍ଞା(((p->prev_size + size) & (malloc_getpagesize-1)) == 0);

  n_mmaps--;
  mmapped_mem -= (size + p->prev_size);

  ret = munmap((ଅକ୍ଷର *)p - p->prev_size, size + p->prev_size);

  /* munmap returns non-zero on failure */
  ପ୍ରତିଜ୍ଞା(ret == 0);
}

#ଯଦି HAVE_MREMAP

#ଯଦି __STD_C
ସ୍ଥିର mchunkptr mremap_chunk(mchunkptr p, size_t new_size)
#ଅନ୍ୟଥା
ସ୍ଥିର mchunkptr mremap_chunk(p, new_size) mchunkptr p; size_t new_size;
#endif
{
  size_t page_mask = malloc_getpagesize - 1;
  INTERNAL_SIZE_T offset = p->prev_size;
  INTERNAL_SIZE_T size = chunksize(p);
  ଅକ୍ଷର *cp;

  ପ୍ରତିଜ୍ଞା (chunk_is_mmapped(p));
  ପ୍ରତିଜ୍ଞା(! ((ଅକ୍ଷର*)p >= sbrk_base && (ଅକ୍ଷର*)p < sbrk_base + sbrked_mem));
  ପ୍ରତିଜ୍ଞା((n_mmaps > 0));
  ପ୍ରତିଜ୍ଞା(((size + offset) & (malloc_getpagesize-1)) == 0);

  /* Note the extra SIZE_SZ overhead as in mmap_chunk(). */
  new_size = (new_size + offset + SIZE_SZ + page_mask) & ~page_mask;

  cp = (ଅକ୍ଷର *)mremap((ଅକ୍ଷର *)p - offset, size + offset, new_size, 1);

  ଯଦି (cp == (ଅକ୍ଷର *)-1) ଫେରନ୍ତୁ 0;

  p = (mchunkptr)(cp + offset);

  ପ୍ରତିଜ୍ଞା(aligned_OK(chunk2mem(p)));

  ପ୍ରତିଜ୍ଞା((p->prev_size == offset));
  set_head(p, (new_size - offset)|IS_MMAPPED);

  mmapped_mem -= size + offset;
  mmapped_mem += new_size;
  ଯଦି ((unsigned long)mmapped_mem > (unsigned long)max_mmapped_mem)
    max_mmapped_mem = mmapped_mem;
  ଯଦି ((unsigned long)(mmapped_mem + sbrked_mem) > (unsigned long)max_total_mem)
    max_total_mem = mmapped_mem + sbrked_mem;
  ଫେରନ୍ତୁ p;
}

#endif /* HAVE_MREMAP */

#endif /* HAVE_MMAP */

/*
  Extend the top-most chunk by obtaining memory from system.
  Main interface to sbrk (but see also malloc_trim).
*/

#ଯଦି __STD_C
ସ୍ଥିର ଶୂନ୍ୟ malloc_extend_ଶୀର୍ଷ(INTERNAL_SIZE_T nb)
#ଅନ୍ୟଥା
ସ୍ଥିର ଶୂନ୍ୟ malloc_extend_ଶୀର୍ଷ(nb) INTERNAL_SIZE_T nb;
#endif
{
  ଅକ୍ଷର*     brk;                  /* return value from sbrk */
  INTERNAL_SIZE_T front_misalign; /* unusable bytes at front of sbrked space */
  INTERNAL_SIZE_T correction;     /* bytes for 2nd sbrk call */
  ଅକ୍ଷର*     new_brk;              /* return of 2nd sbrk call */
  INTERNAL_SIZE_T ଶୀର୍ଷ_size;       /* new size of top chunk */

  mchunkptr old_ଶୀର୍ଷ     = ଶୀର୍ଷ;  /* Record state of old top */
  INTERNAL_SIZE_T old_ଶୀର୍ଷ_size = chunksize(old_ଶୀର୍ଷ);
  ଅକ୍ଷର*     old_end      = (ଅକ୍ଷର*)(chunk_at_offset(old_ଶୀର୍ଷ, old_ଶୀର୍ଷ_size));

  /* Pad request with top_pad plus minimal overhead */

  INTERNAL_SIZE_T    sbrk_size     = nb + ଶୀର୍ଷ_pad + MINSIZE;
  unsigned long pagesz    = malloc_getpagesize;

  /* If not the first time through, round to preserve page boundary */
  /* Otherwise, we need to correct to a page size below anyway. */
  /* (We also correct below if an intervening foreign sbrk call.) */

  ଯଦି (sbrk_base != (ଅକ୍ଷର*)(-1))
    sbrk_size = (sbrk_size + (pagesz - 1)) & ~(pagesz - 1);

  brk = (ଅକ୍ଷର*)(MORECORE (sbrk_size));

  /* Fail if sbrk failed or if a foreign sbrk call killed our space */
  ଯଦି (brk == (ଅକ୍ଷର*)(MORECORE_FAILURE) ||
      (brk < old_end && old_ଶୀର୍ଷ != initial_ଶୀର୍ଷ))
    ଫେରନ୍ତୁ;

  sbrked_mem += sbrk_size;

  ଯଦି (brk == old_end) /* can just add bytes to current top */
  {
    ଶୀର୍ଷ_size = sbrk_size + old_ଶୀର୍ଷ_size;
    set_head(ଶୀର୍ଷ, ଶୀର୍ଷ_size | PREV_INUSE);
  }
  ଅନ୍ୟଥା
  {
    ଯଦି (sbrk_base == (ଅକ୍ଷର*)(-1))  /* First time through. Record base */
      sbrk_base = brk;
    ଅନ୍ୟଥା  /* Someone else called sbrk().  Count those bytes as sbrked_mem. */
      sbrked_mem += brk - (ଅକ୍ଷର*)old_end;

    /* Guarantee alignment of first new chunk made from this space */
    front_misalign = (unsigned long)chunk2mem(brk) & MALLOC_ALIGN_MASK;
    ଯଦି (front_misalign > 0)
    {
      correction = (MALLOC_ALIGNMENT) - front_misalign;
      brk += correction;
    }
    ଅନ୍ୟଥା
      correction = 0;

    /* Guarantee the next brk will be at a page boundary */

    correction += ((((unsigned long)(brk + sbrk_size))+(pagesz-1)) &
		   ~(pagesz - 1)) - ((unsigned long)(brk + sbrk_size));

    /* Allocate correction */
    new_brk = (ଅକ୍ଷର*)(MORECORE (correction));
    ଯଦି (new_brk == (ଅକ୍ଷର*)(MORECORE_FAILURE)) ଫେରନ୍ତୁ;

    sbrked_mem += correction;

    ଶୀର୍ଷ = (mchunkptr)brk;
    ଶୀର୍ଷ_size = new_brk - brk + correction;
    set_head(ଶୀର୍ଷ, ଶୀର୍ଷ_size | PREV_INUSE);

    ଯଦି (old_ଶୀର୍ଷ != initial_ଶୀର୍ଷ)
    {

      /* There must have been an intervening foreign sbrk call. */
      /* A double fencepost is necessary to prevent consolidation */

      /* If not enough space to do this, then user did something very wrong */
      ଯଦି (old_ଶୀର୍ଷ_size < MINSIZE)
      {
	set_head(ଶୀର୍ଷ, PREV_INUSE); /* will force null return from malloc */
	ଫେରନ୍ତୁ;
      }

      /* Also keep size a multiple of MALLOC_ALIGNMENT */
      old_ଶୀର୍ଷ_size = (old_ଶୀର୍ଷ_size - 3*SIZE_SZ) & ~MALLOC_ALIGN_MASK;
      set_head_size(old_ଶୀର୍ଷ, old_ଶୀର୍ଷ_size);
      chunk_at_offset(old_ଶୀର୍ଷ, old_ଶୀର୍ଷ_size          )->size =
	SIZE_SZ|PREV_INUSE;
      chunk_at_offset(old_ଶୀର୍ଷ, old_ଶୀର୍ଷ_size + SIZE_SZ)->size =
	SIZE_SZ|PREV_INUSE;
      /* If possible, release the rest. */
      ଯଦି (old_ଶୀର୍ଷ_size >= MINSIZE)
	fREe(chunk2mem(old_ଶୀର୍ଷ));
    }
  }

  ଯଦି ((unsigned long)sbrked_mem > (unsigned long)max_sbrked_mem)
    max_sbrked_mem = sbrked_mem;
  ଯଦି ((unsigned long)(mmapped_mem + sbrked_mem) > (unsigned long)max_total_mem)
    max_total_mem = mmapped_mem + sbrked_mem;

  /* We always land on a page boundary */
  ପ୍ରତିଜ୍ଞା(((unsigned long)((ଅକ୍ଷର*)ଶୀର୍ଷ + ଶୀର୍ଷ_size) & (pagesz - 1)) == 0);
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
#ଯଦି __STD_C
Void_t* mALLOc_impl(size_t bytes)
#ଅନ୍ୟଥା
Void_t* mALLOc_impl(bytes) size_t bytes;
#endif
{
  mchunkptr victim;                  /* inspected/selected chunk */
  INTERNAL_SIZE_T victim_size;       /* its size */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା       idx;                     /* index for bin traversal */
  mbinptr   bin;                     /* associated bin */
  mchunkptr remainder;               /* remainder from a split */
  long      remainder_size;          /* its size */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା       remainder_index;         /* its bin index */
  unsigned long block;               /* block traverser bit */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା       startidx;                /* first bin of a traversed block */
  mchunkptr fwd;                     /* misc temp for linking */
  mchunkptr bck;                     /* misc temp for linking */
  mbinptr q;                         /* misc temp */

  INTERNAL_SIZE_T nb;

#ଯଦି CONFIG_IS_ENABLED(SYS_MALLOC_F)
	ଯଦି (!(gd->flags & GD_FLG_FULL_MALLOC_INIT))
		ଫେରନ୍ତୁ malloc_simple(bytes);
#endif

  ଯଦି (CONFIG_IS_ENABLED(UNIT_TEST) && malloc_testing) {
    ଯଦି (--malloc_max_allocs < 0)
      ଫେରନ୍ତୁ ଖାଲି;
  }

  /* check if mem_malloc_init() was run */
  ଯଦି ((mem_malloc_start == 0) && (mem_malloc_end == 0)) {
    /* not initialized yet */
    ଫେରନ୍ତୁ ଖାଲି;
  }

  ଯଦି (bytes > CONFIG_SYS_MALLOC_LEN || (long)bytes < 0)
     ଫେରନ୍ତୁ ଖାଲି;

  nb = request2size(bytes);  /* padded request size; */

  /* Check for exact match in a bin */

  ଯଦି (is_small_request(nb))  /* Faster version for small requests */
  {
    idx = smallbin_index(nb);

    /* No traversal or size check necessary for small bins.  */

    q = bin_at(idx);
    victim = last(q);

    /* Also scan the next one, since it would have a remainder < MINSIZE */
    ଯଦି (victim == q)
    {
      q = next_bin(q);
      victim = last(q);
    }
    ଯଦି (victim != q)
    {
      victim_size = chunksize(victim);
      unlink(victim, bck, fwd);
      set_inuse_bit_at_offset(victim, victim_size);
      check_malloced_chunk(victim, nb);
      VALGRIND_MALLOCLIKE_BLOCK(chunk2mem(victim), bytes, SIZE_SZ, false);
      ଫେରନ୍ତୁ chunk2mem(victim);
    }

    idx += 2; /* Set for bin scan below. We've already scanned 2 bins. */

  }
  ଅନ୍ୟଥା
  {
    idx = bin_index(nb);
    bin = bin_at(idx);

    ପାଇଁ (victim = last(bin); victim != bin; victim = victim->bk)
    {
      victim_size = chunksize(victim);
      remainder_size = victim_size - nb;

      ଯଦି (remainder_size >= (long)MINSIZE) /* too big */
      {
	--idx; /* adjust to rescan below after checking last remainder */
	ଭାଙ୍ଗ;
      }

      ଅନ୍ୟଥା ଯଦି (remainder_size >= 0) /* exact fit */
      {
	unlink(victim, bck, fwd);
	set_inuse_bit_at_offset(victim, victim_size);
	check_malloced_chunk(victim, nb);
        VALGRIND_MALLOCLIKE_BLOCK(chunk2mem(victim), bytes, SIZE_SZ, false);
	ଫେରନ୍ତୁ chunk2mem(victim);
      }
    }

    ++idx;

  }

  /* Try to use the last split-off remainder */

  ଯଦି ( (victim = last_remainder->fd) != last_remainder)
  {
    victim_size = chunksize(victim);
    remainder_size = victim_size - nb;

    ଯଦି (remainder_size >= (long)MINSIZE) /* re-split */
    {
      remainder = chunk_at_offset(victim, nb);
      set_head(victim, nb | PREV_INUSE);
      link_last_remainder(remainder);
      set_head(remainder, remainder_size | PREV_INUSE);
      set_foot(remainder, remainder_size);
      check_malloced_chunk(victim, nb);
      VALGRIND_MALLOCLIKE_BLOCK(chunk2mem(victim), bytes, SIZE_SZ, false);
      ଫେରନ୍ତୁ chunk2mem(victim);
    }

    clear_last_remainder;

    ଯଦି (remainder_size >= 0)  /* exhaust */
    {
      set_inuse_bit_at_offset(victim, victim_size);
      check_malloced_chunk(victim, nb);
      VALGRIND_MALLOCLIKE_BLOCK(chunk2mem(victim), bytes, SIZE_SZ, false);
      ଫେରନ୍ତୁ chunk2mem(victim);
    }

    /* Else place in bin */

    frontlink(victim, victim_size, remainder_index, bck, fwd);
  }

  /*
     If there are any possibly nonempty big-enough blocks,
     search for best fitting chunk by scanning bins in blockwidth units.
  */

  ଯଦି ( (block = idx2binblock(idx)) <= binblocks_r)
  {

    /* Get to the first marked block */

    ଯଦି ( (block & binblocks_r) == 0)
    {
      /* force to an even block boundary */
      idx = (idx & ~(BINBLOCKWIDTH - 1)) + BINBLOCKWIDTH;
      block <<= 1;
      ଯେତେବେଳେ ((block & binblocks_r) == 0)
      {
	idx += BINBLOCKWIDTH;
	block <<= 1;
      }
    }

    /* For each possibly nonempty block ... */
    ପାଇଁ (;;)
    {
      startidx = idx;          /* (track incomplete blocks) */
      q = bin = bin_at(idx);

      /* For each bin in this block ... */
      କର
      {
	/* Find and use first big enough chunk ... */

	ପାଇଁ (victim = last(bin); victim != bin; victim = victim->bk)
	{
	  victim_size = chunksize(victim);
	  remainder_size = victim_size - nb;

	  ଯଦି (remainder_size >= (long)MINSIZE) /* split */
	  {
	    remainder = chunk_at_offset(victim, nb);
	    set_head(victim, nb | PREV_INUSE);
	    unlink(victim, bck, fwd);
	    link_last_remainder(remainder);
	    set_head(remainder, remainder_size | PREV_INUSE);
	    set_foot(remainder, remainder_size);
	    check_malloced_chunk(victim, nb);
	    VALGRIND_MALLOCLIKE_BLOCK(chunk2mem(victim), bytes, SIZE_SZ, false);
	    ଫେରନ୍ତୁ chunk2mem(victim);
	  }

	  ଅନ୍ୟଥା ଯଦି (remainder_size >= 0)  /* take */
	  {
	    set_inuse_bit_at_offset(victim, victim_size);
	    unlink(victim, bck, fwd);
	    check_malloced_chunk(victim, nb);
	    VALGRIND_MALLOCLIKE_BLOCK(chunk2mem(victim), bytes, SIZE_SZ, false);
	    ଫେରନ୍ତୁ chunk2mem(victim);
	  }

	}

       bin = next_bin(bin);

      } ଯେତେବେଳେ ((++idx & (BINBLOCKWIDTH - 1)) != 0);

      /* Clear out the block bit. */

      କର   /* Possibly backtrack to try to clear a partial block */
      {
	ଯଦି ((startidx & (BINBLOCKWIDTH - 1)) == 0)
	{
	  av_[1] = (mbinptr)(binblocks_r & ~block);
	  ଭାଙ୍ଗ;
	}
	--startidx;
       q = prev_bin(q);
      } ଯେତେବେଳେ (first(q) == q);

      /* Get to the next possibly nonempty block */

      ଯଦି ( (block <<= 1) <= binblocks_r && (block != 0) )
      {
	ଯେତେବେଳେ ((block & binblocks_r) == 0)
	{
	  idx += BINBLOCKWIDTH;
	  block <<= 1;
	}
      }
      ଅନ୍ୟଥା
	ଭାଙ୍ଗ;
    }
  }

  /* Try to use top chunk */

  /* Require that there be a remainder, ensuring top always exists  */
  ଯଦି ( (remainder_size = chunksize(ଶୀର୍ଷ) - nb) < (long)MINSIZE)
  {

#ଯଦି HAVE_MMAP
    /* If big and would otherwise need to extend, try to use mmap instead */
    ଯଦି ((unsigned long)nb >= (unsigned long)mmap_threshold &&
	(victim = mmap_chunk(nb)))
      VALGRIND_MALLOCLIKE_BLOCK(chunk2mem(victim), bytes, SIZE_SZ, false);
      ଫେରନ୍ତୁ chunk2mem(victim);
#endif

    /* Try to extend */
    malloc_extend_ଶୀର୍ଷ(nb);
    ଯଦି ( (remainder_size = chunksize(ଶୀର୍ଷ) - nb) < (long)MINSIZE)
      ଫେରନ୍ତୁ ଖାଲି; /* propagate failure */
  }

  victim = ଶୀର୍ଷ;
  set_head(victim, nb | PREV_INUSE);
  ଶୀର୍ଷ = chunk_at_offset(victim, nb);
  set_head(ଶୀର୍ଷ, remainder_size | PREV_INUSE);
  check_malloced_chunk(victim, nb);
  VALGRIND_MALLOCLIKE_BLOCK(chunk2mem(victim), bytes, SIZE_SZ, false);
  ଫେରନ୍ତୁ chunk2mem(victim);

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
#ଯଦି __STD_C
ଶୂନ୍ୟ fREe_impl(Void_t* mem)
#ଅନ୍ୟଥା
ଶୂନ୍ୟ fREe_impl(mem) Void_t* mem;
#endif
{
  mchunkptr p;         /* chunk corresponding to mem */
  INTERNAL_SIZE_T hd;  /* its head field */
  INTERNAL_SIZE_T sz;  /* its size */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା       idx;       /* its bin index */
  mchunkptr next;      /* next contiguous chunk */
  INTERNAL_SIZE_T nextsz; /* its size */
  INTERNAL_SIZE_T prevsz; /* size of previous contiguous chunk */
  mchunkptr bck;       /* misc temp for linking */
  mchunkptr fwd;       /* misc temp for linking */
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା       islr;      /* track whether merging with last_remainder */

#ଯଦି CONFIG_IS_ENABLED(SYS_MALLOC_F)
	/* free() is a no-op - all the memory will be freed on relocation */
	ଯଦି (!(gd->flags & GD_FLG_FULL_MALLOC_INIT)) {
		VALGRIND_FREELIKE_BLOCK(mem, SIZE_SZ);
		ଫେରନ୍ତୁ;
	}
#endif

  ଯଦି (mem == ଖାଲି)                              /* free(0) has no effect */
    ଫେରନ୍ତୁ;

  p = mem2chunk(mem);
  hd = p->size;

#ଯଦି HAVE_MMAP
  ଯଦି (hd & IS_MMAPPED)                       /* release mmapped memory. */
  {
    munmap_chunk(p);
    ଫେରନ୍ତୁ;
  }
#endif

  check_inuse_chunk(p);

  sz = hd & ~PREV_INUSE;
  next = chunk_at_offset(p, sz);
  nextsz = chunksize(next);
  VALGRIND_FREELIKE_BLOCK(mem, SIZE_SZ);

  ଯଦି (next == ଶୀର୍ଷ)                            /* merge with top */
  {
    sz += nextsz;

    ଯଦି (!(hd & PREV_INUSE))                    /* consolidate backward */
    {
      prevsz = p->prev_size;
      p = chunk_at_offset(p, -((long) prevsz));
      sz += prevsz;
      unlink(p, bck, fwd);
    }

    set_head(p, sz | PREV_INUSE);
    ଶୀର୍ଷ = p;
    ଯଦି ((unsigned long)(sz) >= (unsigned long)trim_threshold)
      malloc_trim(ଶୀର୍ଷ_pad);
    ଫେରନ୍ତୁ;
  }

  set_head(next, nextsz);                    /* clear inuse bit */

  islr = 0;

  ଯଦି (!(hd & PREV_INUSE))                    /* consolidate backward */
  {
    prevsz = p->prev_size;
    p = chunk_at_offset(p, -((long) prevsz));
    sz += prevsz;

    ଯଦି (p->fd == last_remainder)             /* keep as last_remainder */
      islr = 1;
    ଅନ୍ୟଥା
      unlink(p, bck, fwd);
  }

  ଯଦି (!(inuse_bit_at_offset(next, nextsz)))   /* consolidate forward */
  {
    sz += nextsz;

    ଯଦି (!islr && next->fd == last_remainder)  /* re-insert last_remainder */
    {
      islr = 1;
      link_last_remainder(p);
    }
    ଅନ୍ୟଥା
      unlink(next, bck, fwd);
  }

  set_head(p, sz | PREV_INUSE);
  set_foot(p, sz);
  ଯଦି (!islr)
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
#ଯଦି __STD_C
Void_t* rEALLOc_impl(Void_t* oldmem, size_t bytes)
#ଅନ୍ୟଥା
Void_t* rEALLOc_impl(oldmem, bytes) Void_t* oldmem; size_t bytes;
#endif
{
  INTERNAL_SIZE_T    nb;      /* padded request size */

  mchunkptr oldp;             /* chunk corresponding to oldmem */
  INTERNAL_SIZE_T    oldsize; /* its size */

  mchunkptr newp;             /* chunk to return */
  INTERNAL_SIZE_T    newsize; /* its size */
  Void_t*   newmem;           /* corresponding user mem */

  mchunkptr next;             /* next contiguous chunk after oldp */
  INTERNAL_SIZE_T  nextsize;  /* its size */

  mchunkptr prev;             /* previous contiguous chunk before oldp */
  INTERNAL_SIZE_T  prevsize;  /* its size */

  mchunkptr remainder;        /* holds split off extra space from newp */
  INTERNAL_SIZE_T  remainder_size;   /* its size */

  mchunkptr bck;              /* misc temp for linking */
  mchunkptr fwd;              /* misc temp for linking */

#ifdef REALLOC_ZERO_BYTES_FREES
  ଯଦି (!bytes) {
	fREe_impl(oldmem);
	ଫେରନ୍ତୁ ଖାଲି;
  }
#endif

  ଯଦି (bytes > CONFIG_SYS_MALLOC_LEN || (long)bytes < 0)
     ଫେରନ୍ତୁ ଖାଲି;

  /* realloc of null is supposed to be same as malloc */
  ଯଦି (oldmem == ଖାଲି) ଫେରନ୍ତୁ mALLOc_impl(bytes);

#ଯଦି CONFIG_IS_ENABLED(SYS_MALLOC_F)
	ଯଦି (!(gd->flags & GD_FLG_FULL_MALLOC_INIT)) {
		/* This is harder to support and should not be needed */
		panic("pre-reloc realloc() is not supported");
	}
#endif
  ଯଦି (CONFIG_IS_ENABLED(UNIT_TEST) && malloc_testing) {
    ଯଦି (--malloc_max_allocs < 0)
      ଫେରନ୍ତୁ ଖାଲି;
  }

  newp    = oldp    = mem2chunk(oldmem);
  newsize = oldsize = chunksize(oldp);

  nb = request2size(bytes);

#ଯଦି HAVE_MMAP
  ଯଦି (chunk_is_mmapped(oldp))
  {
#ଯଦି HAVE_MREMAP
    newp = mremap_chunk(oldp, nb);
    ଯଦି(newp) ଫେରନ୍ତୁ chunk2mem(newp);
#endif
    /* Note the extra SIZE_SZ overhead. */
    ଯଦି(oldsize - SIZE_SZ >= nb) ଫେରନ୍ତୁ oldmem; /* do nothing */
    /* Must alloc, copy, free. */
    newmem = mALLOc_impl(bytes);
    ଯଦି (!newmem)
	ଫେରନ୍ତୁ ଖାଲି; /* propagate failure */
    MALLOC_COPY(newmem, oldmem, oldsize - 2*SIZE_SZ);
    munmap_chunk(oldp);
    ଫେରନ୍ତୁ newmem;
  }
#endif

  check_inuse_chunk(oldp);

  ଯଦି ((long)(oldsize) < (long)(nb))
  {

    /* Try expanding forward */

    next = chunk_at_offset(oldp, oldsize);
    ଯଦି (next == ଶୀର୍ଷ || !inuse(next))
    {
      nextsize = chunksize(next);

      /* Forward into top only if a remainder */
      ଯଦି (next == ଶୀର୍ଷ)
      {
	ଯଦି ((long)(nextsize + newsize) >= (long)(nb + MINSIZE))
	{
	  newsize += nextsize;
	  ଶୀର୍ଷ = chunk_at_offset(oldp, nb);
	  set_head(ଶୀର୍ଷ, (newsize - nb) | PREV_INUSE);
	  set_head_size(oldp, nb);
	  VALGRIND_RESIZEINPLACE_BLOCK(chunk2mem(oldp), 0, bytes, SIZE_SZ);
	  VALGRIND_MAKE_MEM_DEFINED(chunk2mem(oldp), bytes);
	  ଫେରନ୍ତୁ chunk2mem(oldp);
	}
      }

      /* Forward into next chunk */
      ଅନ୍ୟଥା ଯଦି (((long)(nextsize + newsize) >= (long)(nb)))
      {
	unlink(next, bck, fwd);
	newsize  += nextsize;
	VALGRIND_RESIZEINPLACE_BLOCK(chunk2mem(oldp), 0, bytes, SIZE_SZ);
	VALGRIND_MAKE_MEM_DEFINED(chunk2mem(oldp), bytes);
	ଯାଅ split;
      }
    }
    ଅନ୍ୟଥା
    {
      next = ଖାଲି;
      nextsize = 0;
    }

    /* Try shifting backwards. */

    ଯଦି (!prev_inuse(oldp))
    {
      prev = prev_chunk(oldp);
      prevsize = chunksize(prev);

      /* try forward + backward first to save a later consolidation */

      ଯଦି (next != ଖାଲି)
      {
	/* into top */
	ଯଦି (next == ଶୀର୍ଷ)
	{
	  ଯଦି ((long)(nextsize + prevsize + newsize) >= (long)(nb + MINSIZE))
	  {
	    unlink(prev, bck, fwd);
	    newp = prev;
	    newsize += prevsize + nextsize;
	    newmem = chunk2mem(newp);
	    VALGRIND_MALLOCLIKE_BLOCK(newmem, bytes, SIZE_SZ, false);
	    MALLOC_COPY(newmem, oldmem, oldsize - SIZE_SZ);
	    ଶୀର୍ଷ = chunk_at_offset(newp, nb);
	    set_head(ଶୀର୍ଷ, (newsize - nb) | PREV_INUSE);
	    set_head_size(newp, nb);
	    VALGRIND_FREELIKE_BLOCK(oldmem, SIZE_SZ);
	    ଫେରନ୍ତୁ newmem;
	  }
	}

	/* into next chunk */
	ଅନ୍ୟଥା ଯଦି (((long)(nextsize + prevsize + newsize) >= (long)(nb)))
	{
	  unlink(next, bck, fwd);
	  unlink(prev, bck, fwd);
	  newp = prev;
	  newsize += nextsize + prevsize;
	  newmem = chunk2mem(newp);
	  VALGRIND_MALLOCLIKE_BLOCK(newmem, bytes, SIZE_SZ, false);
	  MALLOC_COPY(newmem, oldmem, oldsize - SIZE_SZ);
	  ଯାଅ split;
	}
      }

      /* backward only */
      ଯଦି (prev != ଖାଲି && (long)(prevsize + newsize) >= (long)nb)
      {
	unlink(prev, bck, fwd);
	newp = prev;
	newsize += prevsize;
	newmem = chunk2mem(newp);
	VALGRIND_MALLOCLIKE_BLOCK(newmem, bytes, SIZE_SZ, false);
	MALLOC_COPY(newmem, oldmem, oldsize - SIZE_SZ);
	ଯାଅ split;
      }
    }

    /* Must allocate */

    newmem = mALLOc_impl (bytes);

    ଯଦି (newmem == ଖାଲି)  /* propagate failure */
      ଫେରନ୍ତୁ ଖାଲି;

    /* Avoid copy if newp is next chunk after oldp. */
    /* (This can only happen when new chunk is sbrk'ed.) */

    ଯଦି ( (newp = mem2chunk(newmem)) == next_chunk(oldp))
    {
      newsize += chunksize(newp);
      newp = oldp;
      ଯାଅ split;
    }

    /* Otherwise copy, free, and exit */
    MALLOC_COPY(newmem, oldmem, oldsize - SIZE_SZ);
    fREe_impl(oldmem);
    ଫେରନ୍ତୁ newmem;
  } ଅନ୍ୟଥା {
    VALGRIND_RESIZEINPLACE_BLOCK(oldmem, 0, bytes, SIZE_SZ);
    VALGRIND_MAKE_MEM_DEFINED(oldmem, bytes);
  }

 split:  /* split off extra room in old or expanded chunk */

  ଯଦି (newsize - nb >= MINSIZE) /* split off remainder */
  {
    remainder = chunk_at_offset(newp, nb);
    remainder_size = newsize - nb;
    set_head_size(newp, nb);
    set_head(remainder, remainder_size | PREV_INUSE);
    set_inuse_bit_at_offset(remainder, remainder_size);
    VALGRIND_MALLOCLIKE_BLOCK(chunk2mem(remainder), remainder_size, SIZE_SZ,
			      false);
    fREe_impl(chunk2mem(remainder)); /* let free() deal with it */
  }
  ଅନ୍ୟଥା
  {
    set_head_size(newp, newsize);
    set_inuse_bit_at_offset(newp, newsize);
  }

  check_inuse_chunk(newp);
  ଫେରନ୍ତୁ chunk2mem(newp);
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
#ଯଦି __STD_C
Void_t* mEMALIGn_impl(size_t alignment, size_t bytes)
#ଅନ୍ୟଥା
Void_t* mEMALIGn_impl(alignment, bytes) size_t alignment; size_t bytes;
#endif
{
  INTERNAL_SIZE_T    nb;      /* padded  request size */
  ଅକ୍ଷର*     m;                /* memory returned by malloc call */
  mchunkptr p;                /* corresponding chunk */
  ଅକ୍ଷର*     brk;              /* alignment point within p */
  mchunkptr newp;             /* chunk to return */
  INTERNAL_SIZE_T  newsize;   /* its size */
  INTERNAL_SIZE_T  leadsize;  /* leading space befor alignment point */
  mchunkptr remainder;        /* spare room at end to split off */
  long      remainder_size;   /* its size */

  ଯଦି (bytes > CONFIG_SYS_MALLOC_LEN || (long)bytes < 0)
     ଫେରନ୍ତୁ ଖାଲି;

#ଯଦି CONFIG_IS_ENABLED(SYS_MALLOC_F)
	ଯଦି (!(gd->flags & GD_FLG_FULL_MALLOC_INIT)) {
		ଫେରନ୍ତୁ memalign_simple(alignment, bytes);
	}
#endif

  /* If need less alignment than we give anyway, just relay to malloc */

  ଯଦି (alignment <= MALLOC_ALIGNMENT) ଫେରନ୍ତୁ mALLOc_impl(bytes);

  /* Otherwise, ensure that it is at least a minimum chunk size */

  ଯଦି (alignment <  MINSIZE) alignment = MINSIZE;

  /* Call malloc with worst case padding to hit alignment. */

  nb = request2size(bytes);
  m  = (ଅକ୍ଷର*)(mALLOc_impl(nb + alignment + MINSIZE));

  /*
  * The attempt to over-allocate (with a size large enough to guarantee the
  * ability to find an aligned region within allocated memory) failed.
  *
  * Try again, this time only allocating exactly the size the user wants. If
  * the allocation now succeeds and just happens to be aligned, we can still
  * fulfill the user's request.
  */
  ଯଦି (m == ଖାଲି) {
    size_t extra, extra2;
    /*
     * Use bytes not nb, since mALLOc internally calls request2size too, and
     * each call increases the size to allocate, to account for the header.
     */
    m  = (ଅକ୍ଷର*)(mALLOc_impl(bytes));
    /* Aligned -> return it */
    ଯଦି ((((unsigned long)(m)) % alignment) == 0)
      ଫେରନ୍ତୁ m;
    /*
     * Otherwise, try again, requesting enough extra space to be able to
     * acquire alignment.
     */
    fREe_impl(m);
    /* Add in extra bytes to match misalignment of unexpanded allocation */
    extra = alignment - (((unsigned long)(m)) % alignment);
    m  = (ଅକ୍ଷର*)(mALLOc_impl(bytes + extra));
    /*
     * m might not be the same as before. Validate that the previous value of
     * extra still works for the current value of m.
     * If (!m), extra2=alignment so 
     */
    ଯଦି (m) {
      extra2 = alignment - (((unsigned long)(m)) % alignment);
      ଯଦି (extra2 > extra) {
        fREe_impl(m);
        m = ଖାଲି;
      }
    }
    /* Fall through to original NULL check and chunk splitting logic */
  }

  ଯଦି (m == ଖାଲି) ଫେରନ୍ତୁ ଖାଲି; /* propagate failure */

  p = mem2chunk(m);

  ଯଦି ((((unsigned long)(m)) % alignment) == 0) /* aligned */
  {
#ଯଦି HAVE_MMAP
    ଯଦି(chunk_is_mmapped(p))
      ଫେରନ୍ତୁ chunk2mem(p); /* nothing more to do */
#endif
  }
  ଅନ୍ୟଥା /* misaligned */
  {
    /*
      Find an aligned spot inside chunk.
      Since we need to give back leading space in a chunk of at
      least MINSIZE, if the first calculation places us at
      a spot with less than MINSIZE leader, we can move to the
      next aligned spot -- we've allocated enough total room so that
      this is always possible.
    */

    brk = (ଅକ୍ଷର*)mem2chunk(((unsigned long)(m + alignment - 1)) & -((signed) alignment));
    ଯଦି ((long)(brk - (ଅକ୍ଷର*)(p)) < MINSIZE) brk = brk + alignment;

    newp = (mchunkptr)brk;
    leadsize = brk - (ଅକ୍ଷର*)(p);
    newsize = chunksize(p) - leadsize;

#ଯଦି HAVE_MMAP
    ଯଦି(chunk_is_mmapped(p))
    {
      newp->prev_size = p->prev_size + leadsize;
      set_head(newp, newsize|IS_MMAPPED);
      ଫେରନ୍ତୁ chunk2mem(newp);
    }
#endif

    /* give back leader, use the rest */

    set_head(newp, newsize | PREV_INUSE);
    set_inuse_bit_at_offset(newp, newsize);
    set_head_size(p, leadsize);
    fREe_impl(chunk2mem(p));
    p = newp;
    VALGRIND_MALLOCLIKE_BLOCK(chunk2mem(p), bytes, SIZE_SZ, false);

    ପ୍ରତିଜ୍ଞା (newsize >= nb && (((unsigned long)(chunk2mem(p))) % alignment) == 0);
  }

  /* Also give back spare room at the end */

  remainder_size = chunksize(p) - nb;

  ଯଦି (remainder_size >= (long)MINSIZE)
  {
    remainder = chunk_at_offset(p, nb);
    set_head(remainder, remainder_size | PREV_INUSE);
    set_head_size(p, nb);
    VALGRIND_MALLOCLIKE_BLOCK(chunk2mem(remainder), remainder_size, SIZE_SZ,
			      false);
    fREe_impl(chunk2mem(remainder));
  }

  check_inuse_chunk(p);
  ଫେରନ୍ତୁ chunk2mem(p);

}

/*
    valloc just invokes memalign with alignment argument equal
    to the page size of the system (or as near to this as can
    be figured out from all the includes/defines above.)
*/

#ଯଦି __STD_C
Void_t* vALLOc(size_t bytes)
#ଅନ୍ୟଥା
Void_t* vALLOc(bytes) size_t bytes;
#endif
{
  ଫେରନ୍ତୁ mEMALIGn (malloc_getpagesize, bytes);
}

/*
  pvalloc just invokes valloc for the nearest pagesize
  that will accommodate request
*/

#ଯଦି __STD_C
Void_t* pvALLOc(size_t bytes)
#ଅନ୍ୟଥା
Void_t* pvALLOc(bytes) size_t bytes;
#endif
{
  size_t pagesize = malloc_getpagesize;
  ଫେରନ୍ତୁ mEMALIGn (pagesize, (bytes + pagesize - 1) & ~(pagesize - 1));
}

/*

  calloc calls malloc, then zeroes out the allocated chunk.

*/

STATIC_IF_MCHECK
#ଯଦି __STD_C
Void_t* cALLOc_impl(size_t n, size_t elem_size)
#ଅନ୍ୟଥା
Void_t* cALLOc_impl(n, elem_size) size_t n; size_t elem_size;
#endif
{
  mchunkptr p;
  INTERNAL_SIZE_T csz;

  INTERNAL_SIZE_T sz = n * elem_size;

  /* check if expand_top called, in which case don't need to clear */
#ଯଦି CONFIG_IS_ENABLED(SYS_MALLOC_CLEAR_ON_INIT)
#ଯଦି MORECORE_CLEARS
  mchunkptr oldtop = ଶୀର୍ଷ;
  INTERNAL_SIZE_T oldtopsize = chunksize(ଶୀର୍ଷ);
#endif
#endif
  Void_t* mem = mALLOc_impl (sz);

  ଯଦି ((long)n < 0) ଫେରନ୍ତୁ ଖାଲି;

  ଯଦି (mem == ଖାଲି)
    ଫେରନ୍ତୁ ଖାଲି;
  ଅନ୍ୟଥା
  {
#ଯଦି CONFIG_IS_ENABLED(SYS_MALLOC_F)
	ଯଦି (!(gd->flags & GD_FLG_FULL_MALLOC_INIT)) {
		memset(mem, 0, sz);
		ଫେରନ୍ତୁ mem;
	}
#endif
    p = mem2chunk(mem);

    /* Two optional cases in which clearing not necessary */

#ଯଦି HAVE_MMAP
    ଯଦି (chunk_is_mmapped(p)) ଫେରନ୍ତୁ mem;
#endif

    csz = chunksize(p);

#ଯଦି CONFIG_IS_ENABLED(SYS_MALLOC_CLEAR_ON_INIT)
#ଯଦି MORECORE_CLEARS
    ଯଦି (p == oldtop && csz > oldtopsize)
    {
      /* clear only the bytes from non-freshly-sbrked memory */
      csz = oldtopsize;
    }
#endif
#endif

    MALLOC_ZERO(mem, csz - SIZE_SZ);
    VALGRIND_MAKE_MEM_DEFINED(mem, sz);
    ଫେରନ୍ତୁ mem;
  }
}

/*

  cfree just calls free. It is needed/defined on some systems
  that pair it with calloc, presumably for odd historical reasons.

*/

#ଯଦି !defined(INTERNAL_LINUX_C_LIB) || !defined(__ELF__)
#ଯଦି __STD_C
ଶୂନ୍ୟ cfree(Void_t *mem)
#ଅନ୍ୟଥା
ଶୂନ୍ୟ cfree(mem) Void_t *mem;
#endif
{
  fREe(mem);
}
#endif

#ifdef MCHECK_HEAP_PROTECTION
 #ଅନ୍ତର୍ଭାଵ "mcheck_core.inc.h"
 #ଯଦି !__STD_C
  #error "must have __STD_C"
 #endif

Void_t *mALLOc(size_t bytes)
{
	mcheck_pedantic_prehook();
	size_t fullsz = mcheck_alloc_prehook(bytes);
	ଶୂନ୍ୟ *p = mALLOc_impl(fullsz);

	ଯଦି (!p)
		ଫେରନ୍ତୁ p;
	ଫେରନ୍ତୁ mcheck_alloc_posthook(p, bytes);
}

ଶୂନ୍ୟ fREe(Void_t *mem) { fREe_impl(mcheck_free_prehook(mem)); }

Void_t *rEALLOc(Void_t *oldmem, size_t bytes)
{
	mcheck_pedantic_prehook();
	ଯଦି (bytes == 0) {
		ଯଦି (oldmem)
			fREe(oldmem);
		ଫେରନ୍ତୁ ଖାଲି;
	}

	ଯଦି (oldmem == ଖାଲି)
		ଫେରନ୍ତୁ mALLOc(bytes);

	ଶୂନ୍ୟ *p = mcheck_reallocfree_prehook(oldmem);
	size_t newsz = mcheck_alloc_prehook(bytes);

	p = rEALLOc_impl(p, newsz);
	ଯଦି (!p)
		ଫେରନ୍ତୁ p;
	ଫେରନ୍ତୁ mcheck_alloc_noclean_posthook(p, bytes);
}

Void_t *mEMALIGn(size_t alignment, size_t bytes)
{
	mcheck_pedantic_prehook();
	size_t fullsz = mcheck_memalign_prehook(alignment, bytes);
	ଶୂନ୍ୟ *p = mEMALIGn_impl(alignment, fullsz);

	ଯଦି (!p)
		ଫେରନ୍ତୁ p;
	ଫେରନ୍ତୁ mcheck_memalign_posthook(alignment, p, bytes);
}

// pvALLOc, vALLOc - redirect to mEMALIGn, defined here, so they need no wrapping.

Void_t *cALLOc(size_t n, size_t elem_size)
{
	mcheck_pedantic_prehook();
	// NB: here is no overflow check.
	size_t fullsz = mcheck_alloc_prehook(n * elem_size);
	ଶୂନ୍ୟ *p = cALLOc_impl(1, fullsz);

	ଯଦି (!p)
		ଫେରନ୍ତୁ p;
	ଫେରନ୍ତୁ mcheck_alloc_noclean_posthook(p, n * elem_size);
}

// mcheck API {
ପୂର୍ଣ୍ଣସଂଖ୍ୟା mcheck_pedantic(mcheck_abortfunc_t f)
{
	mcheck_initialize(f, 1);
	ଫେରନ୍ତୁ 0;
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା mcheck(mcheck_abortfunc_t f)
{
	mcheck_initialize(f, 0);
	ଫେରନ୍ତୁ 0;
}

ଶୂନ୍ୟ mcheck_check_all(ଶୂନ୍ୟ) { mcheck_pedantic_check(); }

ଗଣନା mcheck_status mprobe(ଶୂନ୍ୟ *__ptr) { ଫେରନ୍ତୁ mcheck_mprobe(__ptr); }
// mcheck API }
#endif

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

#ଯଦି __STD_C
ପୂର୍ଣ୍ଣସଂଖ୍ୟା malloc_trim(size_t pad)
#ଅନ୍ୟଥା
ପୂର୍ଣ୍ଣସଂଖ୍ୟା malloc_trim(pad) size_t pad;
#endif
{
  long  ଶୀର୍ଷ_size;        /* Amount of top-most memory */
  long  extra;           /* Amount to release */
  ଅକ୍ଷର* current_brk;     /* address returned by pre-check sbrk call */
  ଅକ୍ଷର* new_brk;         /* address returned by negative sbrk call */

  unsigned long pagesz = malloc_getpagesize;

  ଶୀର୍ଷ_size = chunksize(ଶୀର୍ଷ);
  extra = ((ଶୀର୍ଷ_size - pad - MINSIZE + (pagesz-1)) / pagesz - 1) * pagesz;

  ଯଦି (extra < (long)pagesz)  /* Not enough memory to release */
    ଫେରନ୍ତୁ 0;

  ଅନ୍ୟଥା
  {
    /* Test to make sure no one else called sbrk */
    current_brk = (ଅକ୍ଷର*)(MORECORE (0));
    ଯଦି (current_brk != (ଅକ୍ଷର*)(ଶୀର୍ଷ) + ଶୀର୍ଷ_size)
      ଫେରନ୍ତୁ 0;     /* Apparently we don't own memory; must fail */

    ଅନ୍ୟଥା
    {
      new_brk = (ଅକ୍ଷର*)(MORECORE (-extra));

      ଯଦି (new_brk == (ଅକ୍ଷର*)(MORECORE_FAILURE)) /* sbrk failed? */
      {
	/* Try to figure out what we have */
	current_brk = (ଅକ୍ଷର*)(MORECORE (0));
	ଶୀର୍ଷ_size = current_brk - (ଅକ୍ଷର*)ଶୀର୍ଷ;
	ଯଦି (ଶୀର୍ଷ_size >= (long)MINSIZE) /* if not, we are very very dead! */
	{
	  sbrked_mem = current_brk - sbrk_base;
	  set_head(ଶୀର୍ଷ, ଶୀର୍ଷ_size | PREV_INUSE);
	}
	check_chunk(ଶୀର୍ଷ);
	ଫେରନ୍ତୁ 0;
      }

      ଅନ୍ୟଥା
      {
	/* Success. Adjust top accordingly. */
	set_head(ଶୀର୍ଷ, (ଶୀର୍ଷ_size - extra) | PREV_INUSE);
	sbrked_mem -= extra;
	check_chunk(ଶୀର୍ଷ);
	ଫେରନ୍ତୁ 1;
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

#ଯଦି __STD_C
size_t malloc_usable_size(Void_t* mem)
#ଅନ୍ୟଥା
size_t malloc_usable_size(mem) Void_t* mem;
#endif
{
  mchunkptr p;
  ଯଦି (mem == ଖାଲି)
    ଫେରନ୍ତୁ 0;
  ଅନ୍ୟଥା
  {
    p = mem2chunk(mem);
    ଯଦି(!chunk_is_mmapped(p))
    {
      ଯଦି (!inuse(p)) ଫେରନ୍ତୁ 0;
      check_inuse_chunk(p);
      ଫେରନ୍ତୁ chunksize(p) - SIZE_SZ;
    }
    ଫେରନ୍ତୁ chunksize(p) - 2*SIZE_SZ;
  }
}

/* Utility to update current_mallinfo for malloc_stats and mallinfo() */

#ifdef DEBUG
ସ୍ଥିର ଶୂନ୍ୟ malloc_update_mallinfo(ଶୂନ୍ୟ)
{
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା i;
  mbinptr b;
  mchunkptr p;
#ifdef DEBUG
  mchunkptr q;
#endif

  INTERNAL_SIZE_T avail = chunksize(ଶୀର୍ଷ);
  ପୂର୍ଣ୍ଣସଂଖ୍ୟା   navail = ((long)(avail) >= (long)MINSIZE)? 1 : 0;

  ପାଇଁ (i = 1; i < NAV; ++i)
  {
    b = bin_at(i);
    ପାଇଁ (p = last(b); p != b; p = p->bk)
    {
#ifdef DEBUG
      check_free_chunk(p);
      ପାଇଁ (q = next_chunk(p);
	   q < ଶୀର୍ଷ && inuse(q) && (long)(chunksize(q)) >= (long)MINSIZE;
	   q = next_chunk(q))
	check_inuse_chunk(q);
#endif
      avail += chunksize(p);
      navail++;
    }
  }

  current_mallinfo.ordblks = navail;
  current_mallinfo.uordblks = sbrked_mem - avail;
  current_mallinfo.fordblks = avail;
  current_mallinfo.hblks = n_mmaps;
  current_mallinfo.hblkhd = mmapped_mem;
  current_mallinfo.keepcost = chunksize(ଶୀର୍ଷ);

}
#endif	/* DEBUG */

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

#ifdef DEBUG
ଶୂନ୍ୟ malloc_stats(ଶୂନ୍ୟ)
{
  malloc_update_mallinfo();
  printf("max system bytes = %10u\n",
	  (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(max_total_mem));
  printf("system bytes     = %10u\n",
	  (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(sbrked_mem + mmapped_mem));
  printf("in use bytes     = %10u\n",
	  (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)(current_mallinfo.uordblks + mmapped_mem));
#ଯଦି HAVE_MMAP
  printf("max mmap regions = %10u\n",
	  (unsigned ପୂର୍ଣ୍ଣସଂଖ୍ୟା)max_n_mmaps);
#endif
}
#endif	/* DEBUG */

/*
  mallinfo returns a copy of updated current mallinfo.
*/

#ifdef DEBUG
ସଂରଚନା mallinfo mALLINFo(ଶୂନ୍ୟ)
{
  malloc_update_mallinfo();
  ଫେରନ୍ତୁ current_mallinfo;
}
#endif	/* DEBUG */

/*
  mallopt:

    mallopt is the general SVID/XPG interface to tunable parameters.
    The format is to provide a (parameter-number, parameter-value) pair.
    mallopt then sets the corresponding parameter to the argument
    value if it can (i.e., so long as the value is meaningful),
    and returns 1 if successful else 0.

    See descriptions of tunable parameters above.

*/

#ଯଦି __STD_C
ପୂର୍ଣ୍ଣସଂଖ୍ୟା mALLOPt(ପୂର୍ଣ୍ଣସଂଖ୍ୟା param_ସଂଖ୍ୟା, ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୂଲ୍ୟ)
#ଅନ୍ୟଥା
ପୂର୍ଣ୍ଣସଂଖ୍ୟା mALLOPt(param_ସଂଖ୍ୟା, ମୂଲ୍ୟ) ପୂର୍ଣ୍ଣସଂଖ୍ୟା param_ସଂଖ୍ୟା; ପୂର୍ଣ୍ଣସଂଖ୍ୟା ମୂଲ୍ୟ;
#endif
{
  ବାଛ(param_ସଂଖ୍ୟା)
  {
    ସ୍ଥିତି M_TRIM_THRESHOLD:
      trim_threshold = ମୂଲ୍ୟ; ଫେରନ୍ତୁ 1;
    ସ୍ଥିତି M_TOP_PAD:
      ଶୀର୍ଷ_pad = ମୂଲ୍ୟ; ଫେରନ୍ତୁ 1;
    ସ୍ଥିତି M_MMAP_THRESHOLD:
      mmap_threshold = ମୂଲ୍ୟ; ଫେରନ୍ତୁ 1;
    ସ୍ଥିତି M_MMAP_MAX:
#ଯଦି HAVE_MMAP
      n_mmaps_max = ମୂଲ୍ୟ; ଫେରନ୍ତୁ 1;
#ଅନ୍ୟଥା
      ଯଦି (ମୂଲ୍ୟ != 0) ଫେରନ୍ତୁ 0; ଅନ୍ୟଥା  n_mmaps_max = ମୂଲ୍ୟ; ଫେରନ୍ତୁ 1;
#endif

    ପୂର୍ବନିର୍ଦ୍ଧାରିତ:
      ଫେରନ୍ତୁ 0;
  }
}

ପୂର୍ଣ୍ଣସଂଖ୍ୟା initf_malloc(ଶୂନ୍ୟ)
{
#ଯଦି CONFIG_IS_ENABLED(SYS_MALLOC_F)
	ପ୍ରତିଜ୍ଞା(gd->malloc_base);	/* Set up by crt0.S */
	gd->malloc_limit = CONFIG_VAL(SYS_MALLOC_F_LEN);
	gd->malloc_ptr = 0;
#endif

	ଫେରନ୍ତୁ 0;
}

ଶୂନ୍ୟ malloc_enable_testing(ପୂର୍ଣ୍ଣସଂଖ୍ୟା max_allocs)
{
	malloc_testing = true;
	malloc_max_allocs = max_allocs;
}

ଶୂନ୍ୟ malloc_disable_testing(ଶୂନ୍ୟ)
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
