// SPDX-License-Identifier: GPL-2.0+
/*
 * This code is based on a version (aka dlmalloc) of malloc/free/realloc written
 * by Doug Lea and released to the public domain, as explained at
 * http://creativecommons.org/publicdomain/zero/1.0/-
 *
 * The original code is available at http://gee.cs.oswego.edu/pub/misc/
 * as file malloc-2.6.6.c.
 */

#जर CONFIG_IS_ENABLED(UNIT_TEST)
#define DEBUG
#endif

#अन्तर्भाव <log.h>
#अन्तर्भाव <asm/global_data.h>

#अन्तर्भाव <malloc.h>
#अन्तर्भाव <mapmem.h>
#अन्तर्भाव <string.h>
#अन्तर्भाव <asm/io.h>
#अन्तर्भाव <valgrind/memcheck.h>

#ifdef DEBUG
#जर __STD_C
स्थिर शून्य malloc_update_mallinfo (शून्य);
शून्य malloc_stats (शून्य);
#अन्यथा
स्थिर शून्य malloc_update_mallinfo ();
शून्य malloc_stats();
#endif
#endif	/* DEBUG */

DECLARE_GLOBAL_DATA_PTR;

#ifdef MCHECK_HEAP_PROTECTION
 #define STATIC_IF_MCHECK स्थिर
 #undef MALLOC_COPY
 #undef MALLOC_ZERO
स्थिर ओळीत शून्य MALLOC_ZERO(शून्य *p, माप_प्रकार sz) { स_रखो(p, 0, sz); }
स्थिर ओळीत शून्य MALLOC_COPY(शून्य *dest, स्थिरांक शून्य *src, माप_प्रकार sz) { स_नकल(dest, src, sz); }
#अन्यथा
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
#define शीर्ष_MEMORY ((अचिन्हित दीर्घ)2*1024*1024*1024)

संरचना GmListElement;
प्रकारव्याख्या संरचना GmListElement GmListElement;

संरचना GmListElement
{
	GmListElement* next;
	शून्य* base;
};

स्थिर GmListElement* head = 0;
स्थिर अचिन्हित पूर्णांक gNextAddress = 0;
स्थिर अचिन्हित पूर्णांक gAddressBase = 0;
स्थिर अचिन्हित पूर्णांक gAllocatedSize = 0;

स्थिर
GmListElement* makeGmListElement (शून्य* bas)
{
	GmListElement* this;
	this = (GmListElement*)(शून्य*)LocalAlloc (0, आकार (GmListElement));
	प्रतिज्ञा (this);
	जर (this)
	{
		this->base = bas;
		this->next = head;
		head = this;
	}
	परत this;
}

शून्य gcleanup (शून्य)
{
	BOOL rval;
	प्रतिज्ञा ( (head == रिक्त) || (head->base == (शून्य*)gAddressBase));
	जर (gAddressBase && (gNextAddress - gAddressBase))
	{
		rval = VirtualFree ((शून्य*)gAddressBase,
							gNextAddress - gAddressBase,
							MEM_DECOMMIT);
	प्रतिज्ञा (rval);
	}
	पर्यंत (head)
	{
		GmListElement* next = head->next;
		rval = VirtualFree (head->base, 0, MEM_RELEASE);
		प्रतिज्ञा (rval);
		LocalFree (head);
		head = next;
	}
}

स्थिर
शून्य* findRegion (शून्य* start_address, अचिन्हित दीर्घ size)
{
	MEMORY_BASIC_INFORMATION info;
	जर (size >= शीर्ष_MEMORY) परत रिक्त;

	पर्यंत ((अचिन्हित दीर्घ)start_address + size < शीर्ष_MEMORY)
	{
		VirtualQuery (start_address, &info, आकार (info));
		जर ((info.State == MEM_FREE) && (info.RegionSize >= size))
			परत start_address;
		अन्यथा
		{
			/* Requested region is not available so see if the */
			/* next region is available.  Set 'start_address' */
			/* to the next region and call 'VirtualQuery()' */
			/* again. */

			start_address = (अक्षर*)info.BaseAddress + info.RegionSize;

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
				(शून्य *) AlignPage64K((अचिन्हित दीर्घ) start_address);
		}
	}
	परत रिक्त;

}

शून्य* wsbrk (दीर्घ size)
{
	शून्य* tmp;
	जर (size > 0)
	{
		जर (gAddressBase == 0)
		{
			gAllocatedSize = max (RESERVED_SIZE, AlignPage (size));
			gNextAddress = gAddressBase =
				(अचिन्हित पूर्णांक)VirtualAlloc (रिक्त, gAllocatedSize,
											MEM_RESERVE, PAGE_NOACCESS);
		} अन्यथा जर (AlignPage (gNextAddress + size) > (gAddressBase +
gAllocatedSize))
		{
			दीर्घ new_size = max (NEXT_SIZE, AlignPage (size));
			शून्य* new_address = (शून्य*)(gAddressBase+gAllocatedSize);
			करा
			{
				new_address = findRegion (new_address, new_size);

				जर (!new_address)
					परत (शून्य*)-1;

				gAddressBase = gNextAddress =
					(अचिन्हित पूर्णांक)VirtualAlloc (new_address, new_size,
												MEM_RESERVE, PAGE_NOACCESS);
				/* repeat in case of race condition */
				/* The region that we found has been snagged */
				/* by another thread */
			}
			पर्यंत (gAddressBase == 0);

			प्रतिज्ञा (new_address == (शून्य*)gAddressBase);

			gAllocatedSize = new_size;

			जर (!makeGmListElement ((शून्य*)gAddressBase))
				परत (शून्य*)-1;
		}
		जर ((size + gNextAddress) > AlignPage (gNextAddress))
		{
			शून्य* res;
			res = VirtualAlloc ((शून्य*)AlignPage (gNextAddress),
								(size + gNextAddress -
								 AlignPage (gNextAddress)),
								MEM_COMMIT, PAGE_READWRITE);
			जर (!res)
				परत (शून्य*)-1;
		}
		tmp = (शून्य*)gNextAddress;
		gNextAddress = (अचिन्हित पूर्णांक)tmp + size;
		परत tmp;
	}
	अन्यथा जर (size < 0)
	{
		अचिन्हित पूर्णांक alignedGoal = AlignPage (gNextAddress + size);
		/* Trim by releasing the virtual memory */
		जर (alignedGoal >= gAddressBase)
		{
			VirtualFree ((शून्य*)alignedGoal, gNextAddress - alignedGoal,
						 MEM_DECOMMIT);
			gNextAddress = gNextAddress + size;
			परत (शून्य*)gNextAddress;
		}
		अन्यथा
		{
			VirtualFree ((शून्य*)gAddressBase, gNextAddress - gAddressBase,
						 MEM_DECOMMIT);
			gNextAddress = gAddressBase;
			परत (शून्य*)-1;
		}
	}
	अन्यथा
	{
		परत (शून्य*)gNextAddress;
	}
}

#endif

/*
  Type declarations
*/

संरचना malloc_chunk
{
  INTERNAL_SIZE_T prev_size; /* Size of previous chunk (if free). */
  INTERNAL_SIZE_T size;      /* Size in bytes, including overhead. */
  संरचना malloc_chunk* fd;   /* double links -- used only if free. */
  संरचना malloc_chunk* bk;
} __attribute__((__may_alias__)) ;

प्रकारव्याख्या संरचना malloc_chunk* mchunkptr;

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

#define SIZE_SZ                (आकार(INTERNAL_SIZE_T))
#define MALLOC_ALIGNMENT       (SIZE_SZ + SIZE_SZ)
#define MALLOC_ALIGN_MASK      (MALLOC_ALIGNMENT - 1)
#define MINSIZE                (आकार(संरचना malloc_chunk))

/* conversion from malloc headers to user pointers, and back */

#define chunk2mem(p)   ((Void_t*)((अक्षर*)(p) + 2*SIZE_SZ))
#define mem2chunk(mem) ((mchunkptr)((अक्षर*)(mem) - 2*SIZE_SZ))

/* pad request bytes into a usable size */

#define request2size(req) \
 ((((req) + (SIZE_SZ + MALLOC_ALIGN_MASK)) < \
  (MINSIZE + MALLOC_ALIGN_MASK)) ? MINSIZE : \
   (((req) + (SIZE_SZ + MALLOC_ALIGN_MASK)) & ~(MALLOC_ALIGN_MASK)))

/* Check if m has acceptable alignment */

#define aligned_OK(m)    (((अचिन्हित दीर्घ)((m)) & (MALLOC_ALIGN_MASK)) == 0)

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

#define next_chunk(p) ((mchunkptr)( ((अक्षर*)(p)) + ((p)->size & ~PREV_INUSE) ))

/* Ptr to previous physical malloc_chunk */

#define prev_chunk(p)\
   ((mchunkptr)( ((अक्षर*)(p)) - ((p)->prev_size) ))

/* Treat space at ptr + offset as a chunk */

#define chunk_at_offset(p, s)  ((mchunkptr)(((अक्षर*)(p)) + (s)))

/*
  Dealing with use bits
*/

/* extract p's inuse bit */

#define inuse(p)\
((((mchunkptr)(((अक्षर*)(p))+((p)->size & ~PREV_INUSE)))->size) & PREV_INUSE)

/* extract inuse bit of previous chunk */

#define prev_inuse(p)  ((p)->size & PREV_INUSE)

/* check for mmap()'ed chunk */

#define chunk_is_mmapped(p) ((p)->size & IS_MMAPPED)

/* set/clear chunk as in use without otherwise disturbing */

#define set_inuse(p)\
((mchunkptr)(((अक्षर*)(p)) + ((p)->size & ~PREV_INUSE)))->size |= PREV_INUSE

#define clear_inuse(p)\
((mchunkptr)(((अक्षर*)(p)) + ((p)->size & ~PREV_INUSE)))->size &= ~(PREV_INUSE)

/* check/set/clear inuse bits in known places */

#define inuse_bit_at_offset(p, s)\
 (((mchunkptr)(((अक्षर*)(p)) + (s)))->size & PREV_INUSE)

#define set_inuse_bit_at_offset(p, s)\
 (((mchunkptr)(((अक्षर*)(p)) + (s)))->size |= PREV_INUSE)

#define clear_inuse_bit_at_offset(p, s)\
 (((mchunkptr)(((अक्षर*)(p)) + (s)))->size &= ~(PREV_INUSE))

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

#define set_foot(p, s)   (((mchunkptr)((अक्षर*)(p) + (s)))->prev_size = (s))

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

प्रकारव्याख्या संरचना malloc_chunk* mbinptr;

/* access macros */

#define bin_at(i)      ((mbinptr)((अक्षर*)&(av_[2*(i) + 2]) - 2*SIZE_SZ))
#define next_bin(b)    ((mbinptr)((अक्षर*)(b) + 2 * आकार(mbinptr)))
#define prev_bin(b)    ((mbinptr)((अक्षर*)(b) - 2 * आकार(mbinptr)))

/*
   The first 2 bins are never indexed. The corresponding av_ cells are instead
   used for bookkeeping. This is not to save space, but to simplify
   indexing, maintain locality, and avoid some initialization tests.
*/

#define शीर्ष            (av_[2])          /* The topmost chunk */
#define last_remainder (bin_at(1))       /* remainder from last split */

/*
   Because top initially points to its own bin with initial
   zero size, thus forcing extension on the first malloc request,
   we avoid having any special code in malloc to check whether
   it even exists yet. But we still need to in malloc_extend_top.
*/

#define initial_शीर्ष    ((mchunkptr)(bin_at(0)))

/* Helper macro to initialize bins */

#define IAV(i)  bin_at(i), bin_at(i)

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

#ifdef CONFIG_SYS_MALLOC_DEFAULT_TO_INIT
स्थिर शून्य malloc_init(शून्य);
#endif

ulong mem_malloc_start = 0;
ulong mem_malloc_end = 0;
ulong mem_malloc_brk = 0;

स्थिर bool malloc_testing;	/* enable test mode */
स्थिर पूर्णांक malloc_max_allocs;	/* return NULL after this many calls to malloc() */

शून्य *sbrk(सूचक_भेद_प्रकार increment)
{
	ulong old = mem_malloc_brk;
	ulong new = old + increment;

	जर ((new < mem_malloc_start) || (new > mem_malloc_end))
		परत (शून्य *)MORECORE_FAILURE;

	/*
	 * if we are giving memory back make sure we clear it out since
	 * we set MORECORE_CLEARS to 1
	 */
	जर (increment < 0)
		स_रखो((शून्य *)new, 0, -increment);

	mem_malloc_brk = new;

	परत (शून्य *)old;
}

शून्य mem_malloc_init(ulong start, ulong size)
{
	mem_malloc_start = (ulong)map_sysmem(start, size);
	mem_malloc_end = mem_malloc_start + size;
	mem_malloc_brk = mem_malloc_start;

#ifdef CONFIG_SYS_MALLOC_DEFAULT_TO_INIT
	malloc_init();
#endif

	debug("using memory %#lx-%#lx for malloc()\n", mem_malloc_start,
	      mem_malloc_end);
#जर CONFIG_IS_ENABLED(SYS_MALLOC_CLEAR_ON_INIT)
	स_रखो((शून्य *)mem_malloc_start, 0x0, size);
#endif
}

/* field-extraction macros */

#define first(b) ((b)->fd)
#define last(b)  ((b)->bk)

/*
  Indexing into bins
*/

#define bin_index(sz)                                                          \
(((((अचिन्हित दीर्घ)(sz)) >> 9) ==    0) ?       (((अचिन्हित दीर्घ)(sz)) >>  3): \
 ((((अचिन्हित दीर्घ)(sz)) >> 9) <=    4) ?  56 + (((अचिन्हित दीर्घ)(sz)) >>  6): \
 ((((अचिन्हित दीर्घ)(sz)) >> 9) <=   20) ?  91 + (((अचिन्हित दीर्घ)(sz)) >>  9): \
 ((((अचिन्हित दीर्घ)(sz)) >> 9) <=   84) ? 110 + (((अचिन्हित दीर्घ)(sz)) >> 12): \
 ((((अचिन्हित दीर्घ)(sz)) >> 9) <=  340) ? 119 + (((अचिन्हित दीर्घ)(sz)) >> 15): \
 ((((अचिन्हित दीर्घ)(sz)) >> 9) <= 1364) ? 124 + (((अचिन्हित दीर्घ)(sz)) >> 18): \
					  126)
/*
  bins for chunks < 512 are all spaced 8 bytes apart, and hold
  identically sized chunks. This is exploited in malloc.
*/

#define MAX_SMALLBIN         63
#define MAX_SMALLBIN_SIZE   512
#define SMALLBIN_WIDTH        8

#define smallbin_index(sz)  (((अचिन्हित दीर्घ)(sz)) >> 3)

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

#define idx2binblock(ix)    ((अचिन्हित)1 << (ix / BINBLOCKWIDTH))
#define mark_binblock(ii)   (binblocks_w = (mbinptr)(binblocks_r | idx2binblock(ii)))
#define clear_binblock(ii)  (binblocks_w = (mbinptr)(binblocks_r & ~(idx2binblock(ii))))

/*  Other static bookkeeping data */

/* variables holding tunable values */

स्थिर अचिन्हित दीर्घ trim_threshold   = DEFAULT_TRIM_THRESHOLD;
स्थिर अचिन्हित दीर्घ शीर्ष_pad          = DEFAULT_शीर्ष_PAD;
स्थिर अचिन्हित पूर्णांक  n_mmaps_max      = DEFAULT_MMAP_MAX;
स्थिर अचिन्हित दीर्घ mmap_threshold   = DEFAULT_MMAP_THRESHOLD;

/* The first value returned from sbrk */
स्थिर अक्षर* sbrk_base = (अक्षर*)(-1);

/* The maximum memory obtained from system via sbrk */
स्थिर अचिन्हित दीर्घ max_sbrked_mem = 0;

/* The maximum via either sbrk or mmap */
स्थिर अचिन्हित दीर्घ max_total_mem = 0;

/* internal working copy of mallinfo */
स्थिर संरचना mallinfo current_mallinfo = {  0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

/* The total memory obtained from system via sbrk */
#define sbrked_mem  (current_mallinfo.arena)

/* Tracking mmaps */

#ifdef DEBUG
स्थिर अचिन्हित पूर्णांक n_mmaps = 0;
#endif	/* DEBUG */
स्थिर अचिन्हित दीर्घ mmapped_mem = 0;
#जर HAVE_MMAP
स्थिर अचिन्हित पूर्णांक max_n_mmaps = 0;
स्थिर अचिन्हित दीर्घ max_mmapped_mem = 0;
#endif

#ifdef CONFIG_SYS_MALLOC_DEFAULT_TO_INIT
स्थिर शून्य malloc_init(शून्य)
{
	पूर्णांक i, j;

	debug("bins (av_ array) are at %p\n", (शून्य *)av_);

	av_[0] = रिक्त; av_[1] = रिक्त;
	साठी (i = 2, j = 2; i < NAV * 2 + 2; i += 2, j++) {
		av_[i] = bin_at(j - 2);
		av_[i + 1] = bin_at(j - 2);

		/* Just print the first few bins so that
		 * we can see there are alright.
		 */
		जर (i < 10)
			debug("av_[%d]=%lx av_[%d]=%lx\n",
			      i, (ulong)av_[i],
			      i + 1, (ulong)av_[i + 1]);
	}

	/* Init the static bookkeeping as well */
	sbrk_base = (अक्षर *)(-1);
	max_sbrked_mem = 0;
	max_total_mem = 0;
#ifdef DEBUG
	स_रखो((शून्य *)&current_mallinfo, 0, आकार(संरचना mallinfo));
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

#जर __STD_C
स्थिर शून्य do_check_chunk(mchunkptr p)
#अन्यथा
स्थिर शून्य do_check_chunk(p) mchunkptr p;
#endif
{
  INTERNAL_SIZE_T sz = p->size & ~PREV_INUSE;

  /* No checkable chunk is mmapped */
  प्रतिज्ञा(!chunk_is_mmapped(p));

  /* Check for legal address ... */
  प्रतिज्ञा((अक्षर*)p >= sbrk_base);
  जर (p != शीर्ष)
    प्रतिज्ञा((अक्षर*)p + sz <= (अक्षर*)शीर्ष);
  अन्यथा
    प्रतिज्ञा((अक्षर*)p + sz <= sbrk_base + sbrked_mem);

}

#जर __STD_C
स्थिर शून्य do_check_free_chunk(mchunkptr p)
#अन्यथा
स्थिर शून्य do_check_free_chunk(p) mchunkptr p;
#endif
{
  INTERNAL_SIZE_T sz = p->size & ~PREV_INUSE;
  mchunkptr next = chunk_at_offset(p, sz);

  do_check_chunk(p);

  /* Check whether it claims to be free ... */
  प्रतिज्ञा(!inuse(p));

  /* Unless a special marker, must have OK fields */
  जर ((दीर्घ)sz >= (दीर्घ)MINSIZE)
  {
    प्रतिज्ञा((sz & MALLOC_ALIGN_MASK) == 0);
    प्रतिज्ञा(aligned_OK(chunk2mem(p)));
    /* ... matching footer field */
    प्रतिज्ञा(next->prev_size == sz);
    /* ... and is fully consolidated */
    प्रतिज्ञा(prev_inuse(p));
    प्रतिज्ञा (next == शीर्ष || inuse(next));

    /* ... and has minimally sane links */
    प्रतिज्ञा(p->fd->bk == p);
    प्रतिज्ञा(p->bk->fd == p);
  }
  अन्यथा /* markers are always of size SIZE_SZ */
    प्रतिज्ञा(sz == SIZE_SZ);
}

#जर __STD_C
स्थिर शून्य do_check_inuse_chunk(mchunkptr p)
#अन्यथा
स्थिर शून्य do_check_inuse_chunk(p) mchunkptr p;
#endif
{
  mchunkptr next = next_chunk(p);
  do_check_chunk(p);

  /* Check whether it claims to be in use ... */
  प्रतिज्ञा(inuse(p));

  /* ... and is surrounded by OK chunks.
    Since more things can be checked with free chunks than inuse ones,
    if an inuse chunk borders them and debug is on, it's worth doing them.
  */
  जर (!prev_inuse(p))
  {
    mchunkptr prv = prev_chunk(p);
    प्रतिज्ञा(next_chunk(prv) == p);
    do_check_free_chunk(prv);
  }
  जर (next == शीर्ष)
  {
    प्रतिज्ञा(prev_inuse(next));
    प्रतिज्ञा(chunksize(next) >= MINSIZE);
  }
  अन्यथा जर (!inuse(next))
    do_check_free_chunk(next);

}

#जर __STD_C
स्थिर शून्य do_check_malloced_chunk(mchunkptr p, INTERNAL_SIZE_T s)
#अन्यथा
स्थिर शून्य do_check_malloced_chunk(p, s) mchunkptr p; INTERNAL_SIZE_T s;
#endif
{
  INTERNAL_SIZE_T sz = p->size & ~PREV_INUSE;
  दीर्घ room = sz - s;

  do_check_inuse_chunk(p);

  /* Legal size ... */
  प्रतिज्ञा((दीर्घ)sz >= (दीर्घ)MINSIZE);
  प्रतिज्ञा((sz & MALLOC_ALIGN_MASK) == 0);
  प्रतिज्ञा(room >= 0);
  प्रतिज्ञा(room < (दीर्घ)MINSIZE);

  /* ... and alignment */
  प्रतिज्ञा(aligned_OK(chunk2mem(p)));

  /* ... and was allocated at front of an available chunk */
  प्रतिज्ञा(prev_inuse(p));

}

#define check_free_chunk(P)  do_check_free_chunk(P)
#define check_inuse_chunk(P) do_check_inuse_chunk(P)
#define check_chunk(P) do_check_chunk(P)
#define check_malloced_chunk(P,N) do_check_malloced_chunk(P,N)
#अन्यथा
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
  जर (S < MAX_SMALLBIN_SIZE)                                                  \
  {                                                                           \
    IDX = smallbin_index(S);                                                  \
    mark_binblock(IDX);                                                       \
    BK = bin_at(IDX);                                                         \
    FD = BK->fd;                                                              \
    P->bk = BK;                                                               \
    P->fd = FD;                                                               \
    FD->bk = BK->fd = P;                                                      \
  }                                                                           \
  अन्यथा                                                                        \
  {                                                                           \
    IDX = bin_index(S);                                                       \
    BK = bin_at(IDX);                                                         \
    FD = BK->fd;                                                              \
    जर (FD == BK) mark_binblock(IDX);                                         \
    अन्यथा                                                                      \
    {                                                                         \
      पर्यंत (FD != BK && S < chunksize(FD)) FD = FD->fd;                      \
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

#जर HAVE_MMAP

#जर __STD_C
स्थिर mchunkptr mmap_chunk(माप_प्रकार size)
#अन्यथा
स्थिर mchunkptr mmap_chunk(size) माप_प्रकार size;
#endif
{
  माप_प्रकार page_mask = malloc_getpagesize - 1;
  mchunkptr p;

#ifndef MAP_ANONYMOUS
  स्थिर पूर्णांक fd = -1;
#endif

  जर(n_mmaps >= n_mmaps_max) परत 0; /* too many regions */

  /* For mmapped chunks, the overhead is one SIZE_SZ unit larger, because
   * there is no following chunk whose prev_size field could be used.
   */
  size = (size + SIZE_SZ + page_mask) & ~page_mask;

#ifdef MAP_ANONYMOUS
  p = (mchunkptr)mmap(0, size, PROT_READ|PROT_WRITE,
		      MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
#अन्यथा /* !MAP_ANONYMOUS */
  जर (fd < 0)
  {
    fd = खोलो("/dev/zero", O_RDWR);
    जर(fd < 0) परत 0;
  }
  p = (mchunkptr)mmap(0, size, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0);
#endif

  जर(p == (mchunkptr)-1) परत 0;

  n_mmaps++;
  जर (n_mmaps > max_n_mmaps) max_n_mmaps = n_mmaps;

  /* We demand that eight bytes into a page must be 8-byte aligned. */
  प्रतिज्ञा(aligned_OK(chunk2mem(p)));

  /* The offset to the start of the mmapped region is stored
   * in the prev_size field of the chunk; normally it is zero,
   * but that can be changed in memalign().
   */
  p->prev_size = 0;
  set_head(p, size|IS_MMAPPED);

  mmapped_mem += size;
  जर ((अचिन्हित दीर्घ)mmapped_mem > (अचिन्हित दीर्घ)max_mmapped_mem)
    max_mmapped_mem = mmapped_mem;
  जर ((अचिन्हित दीर्घ)(mmapped_mem + sbrked_mem) > (अचिन्हित दीर्घ)max_total_mem)
    max_total_mem = mmapped_mem + sbrked_mem;
  परत p;
}

#जर __STD_C
स्थिर शून्य munmap_chunk(mchunkptr p)
#अन्यथा
स्थिर शून्य munmap_chunk(p) mchunkptr p;
#endif
{
  INTERNAL_SIZE_T size = chunksize(p);
  पूर्णांक ret;

  प्रतिज्ञा (chunk_is_mmapped(p));
  प्रतिज्ञा(! ((अक्षर*)p >= sbrk_base && (अक्षर*)p < sbrk_base + sbrked_mem));
  प्रतिज्ञा((n_mmaps > 0));
  प्रतिज्ञा(((p->prev_size + size) & (malloc_getpagesize-1)) == 0);

  n_mmaps--;
  mmapped_mem -= (size + p->prev_size);

  ret = munmap((अक्षर *)p - p->prev_size, size + p->prev_size);

  /* munmap returns non-zero on failure */
  प्रतिज्ञा(ret == 0);
}

#जर HAVE_MREMAP

#जर __STD_C
स्थिर mchunkptr mremap_chunk(mchunkptr p, माप_प्रकार new_size)
#अन्यथा
स्थिर mchunkptr mremap_chunk(p, new_size) mchunkptr p; माप_प्रकार new_size;
#endif
{
  माप_प्रकार page_mask = malloc_getpagesize - 1;
  INTERNAL_SIZE_T offset = p->prev_size;
  INTERNAL_SIZE_T size = chunksize(p);
  अक्षर *cp;

  प्रतिज्ञा (chunk_is_mmapped(p));
  प्रतिज्ञा(! ((अक्षर*)p >= sbrk_base && (अक्षर*)p < sbrk_base + sbrked_mem));
  प्रतिज्ञा((n_mmaps > 0));
  प्रतिज्ञा(((size + offset) & (malloc_getpagesize-1)) == 0);

  /* Note the extra SIZE_SZ overhead as in mmap_chunk(). */
  new_size = (new_size + offset + SIZE_SZ + page_mask) & ~page_mask;

  cp = (अक्षर *)mremap((अक्षर *)p - offset, size + offset, new_size, 1);

  जर (cp == (अक्षर *)-1) परत 0;

  p = (mchunkptr)(cp + offset);

  प्रतिज्ञा(aligned_OK(chunk2mem(p)));

  प्रतिज्ञा((p->prev_size == offset));
  set_head(p, (new_size - offset)|IS_MMAPPED);

  mmapped_mem -= size + offset;
  mmapped_mem += new_size;
  जर ((अचिन्हित दीर्घ)mmapped_mem > (अचिन्हित दीर्घ)max_mmapped_mem)
    max_mmapped_mem = mmapped_mem;
  जर ((अचिन्हित दीर्घ)(mmapped_mem + sbrked_mem) > (अचिन्हित दीर्घ)max_total_mem)
    max_total_mem = mmapped_mem + sbrked_mem;
  परत p;
}

#endif /* HAVE_MREMAP */

#endif /* HAVE_MMAP */

/*
  Extend the top-most chunk by obtaining memory from system.
  Main interface to sbrk (but see also malloc_trim).
*/

#जर __STD_C
स्थिर शून्य malloc_extend_शीर्ष(INTERNAL_SIZE_T nb)
#अन्यथा
स्थिर शून्य malloc_extend_शीर्ष(nb) INTERNAL_SIZE_T nb;
#endif
{
  अक्षर*     brk;                  /* return value from sbrk */
  INTERNAL_SIZE_T front_misalign; /* unusable bytes at front of sbrked space */
  INTERNAL_SIZE_T correction;     /* bytes for 2nd sbrk call */
  अक्षर*     new_brk;              /* return of 2nd sbrk call */
  INTERNAL_SIZE_T शीर्ष_size;       /* new size of top chunk */

  mchunkptr old_शीर्ष     = शीर्ष;  /* Record state of old top */
  INTERNAL_SIZE_T old_शीर्ष_size = chunksize(old_शीर्ष);
  अक्षर*     old_end      = (अक्षर*)(chunk_at_offset(old_शीर्ष, old_शीर्ष_size));

  /* Pad request with top_pad plus minimal overhead */

  INTERNAL_SIZE_T    sbrk_size     = nb + शीर्ष_pad + MINSIZE;
  अचिन्हित दीर्घ pagesz    = malloc_getpagesize;

  /* If not the first time through, round to preserve page boundary */
  /* Otherwise, we need to correct to a page size below anyway. */
  /* (We also correct below if an intervening foreign sbrk call.) */

  जर (sbrk_base != (अक्षर*)(-1))
    sbrk_size = (sbrk_size + (pagesz - 1)) & ~(pagesz - 1);

  brk = (अक्षर*)(MORECORE (sbrk_size));

  /* Fail if sbrk failed or if a foreign sbrk call killed our space */
  जर (brk == (अक्षर*)(MORECORE_FAILURE) ||
      (brk < old_end && old_शीर्ष != initial_शीर्ष))
    परत;

  sbrked_mem += sbrk_size;

  जर (brk == old_end) /* can just add bytes to current top */
  {
    शीर्ष_size = sbrk_size + old_शीर्ष_size;
    set_head(शीर्ष, शीर्ष_size | PREV_INUSE);
  }
  अन्यथा
  {
    जर (sbrk_base == (अक्षर*)(-1))  /* First time through. Record base */
      sbrk_base = brk;
    अन्यथा  /* Someone else called sbrk().  Count those bytes as sbrked_mem. */
      sbrked_mem += brk - (अक्षर*)old_end;

    /* Guarantee alignment of first new chunk made from this space */
    front_misalign = (अचिन्हित दीर्घ)chunk2mem(brk) & MALLOC_ALIGN_MASK;
    जर (front_misalign > 0)
    {
      correction = (MALLOC_ALIGNMENT) - front_misalign;
      brk += correction;
    }
    अन्यथा
      correction = 0;

    /* Guarantee the next brk will be at a page boundary */

    correction += ((((अचिन्हित दीर्घ)(brk + sbrk_size))+(pagesz-1)) &
		   ~(pagesz - 1)) - ((अचिन्हित दीर्घ)(brk + sbrk_size));

    /* Allocate correction */
    new_brk = (अक्षर*)(MORECORE (correction));
    जर (new_brk == (अक्षर*)(MORECORE_FAILURE)) परत;

    sbrked_mem += correction;

    शीर्ष = (mchunkptr)brk;
    शीर्ष_size = new_brk - brk + correction;
    set_head(शीर्ष, शीर्ष_size | PREV_INUSE);

    जर (old_शीर्ष != initial_शीर्ष)
    {

      /* There must have been an intervening foreign sbrk call. */
      /* A double fencepost is necessary to prevent consolidation */

      /* If not enough space to do this, then user did something very wrong */
      जर (old_शीर्ष_size < MINSIZE)
      {
	set_head(शीर्ष, PREV_INUSE); /* will force null return from malloc */
	परत;
      }

      /* Also keep size a multiple of MALLOC_ALIGNMENT */
      old_शीर्ष_size = (old_शीर्ष_size - 3*SIZE_SZ) & ~MALLOC_ALIGN_MASK;
      set_head_size(old_शीर्ष, old_शीर्ष_size);
      chunk_at_offset(old_शीर्ष, old_शीर्ष_size          )->size =
	SIZE_SZ|PREV_INUSE;
      chunk_at_offset(old_शीर्ष, old_शीर्ष_size + SIZE_SZ)->size =
	SIZE_SZ|PREV_INUSE;
      /* If possible, release the rest. */
      जर (old_शीर्ष_size >= MINSIZE)
	fREe(chunk2mem(old_शीर्ष));
    }
  }

  जर ((अचिन्हित दीर्घ)sbrked_mem > (अचिन्हित दीर्घ)max_sbrked_mem)
    max_sbrked_mem = sbrked_mem;
  जर ((अचिन्हित दीर्घ)(mmapped_mem + sbrked_mem) > (अचिन्हित दीर्घ)max_total_mem)
    max_total_mem = mmapped_mem + sbrked_mem;

  /* We always land on a page boundary */
  प्रतिज्ञा(((अचिन्हित दीर्घ)((अक्षर*)शीर्ष + शीर्ष_size) & (pagesz - 1)) == 0);
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
#जर __STD_C
Void_t* mALLOc_impl(माप_प्रकार bytes)
#अन्यथा
Void_t* mALLOc_impl(bytes) माप_प्रकार bytes;
#endif
{
  mchunkptr victim;                  /* inspected/selected chunk */
  INTERNAL_SIZE_T victim_size;       /* its size */
  पूर्णांक       idx;                     /* index for bin traversal */
  mbinptr   bin;                     /* associated bin */
  mchunkptr remainder;               /* remainder from a split */
  दीर्घ      remainder_size;          /* its size */
  पूर्णांक       remainder_index;         /* its bin index */
  अचिन्हित दीर्घ block;               /* block traverser bit */
  पूर्णांक       startidx;                /* first bin of a traversed block */
  mchunkptr fwd;                     /* misc temp for linking */
  mchunkptr bck;                     /* misc temp for linking */
  mbinptr q;                         /* misc temp */

  INTERNAL_SIZE_T nb;

#जर CONFIG_IS_ENABLED(SYS_MALLOC_F)
	जर (!(gd->flags & GD_FLG_FULL_MALLOC_INIT))
		परत malloc_simple(bytes);
#endif

  जर (CONFIG_IS_ENABLED(UNIT_TEST) && malloc_testing) {
    जर (--malloc_max_allocs < 0)
      परत रिक्त;
  }

  /* check if mem_malloc_init() was run */
  जर ((mem_malloc_start == 0) && (mem_malloc_end == 0)) {
    /* not initialized yet */
    परत रिक्त;
  }

  जर (bytes > CONFIG_SYS_MALLOC_LEN || (दीर्घ)bytes < 0)
     परत रिक्त;

  nb = request2size(bytes);  /* padded request size; */

  /* Check for exact match in a bin */

  जर (is_small_request(nb))  /* Faster version for small requests */
  {
    idx = smallbin_index(nb);

    /* No traversal or size check necessary for small bins.  */

    q = bin_at(idx);
    victim = last(q);

    /* Also scan the next one, since it would have a remainder < MINSIZE */
    जर (victim == q)
    {
      q = next_bin(q);
      victim = last(q);
    }
    जर (victim != q)
    {
      victim_size = chunksize(victim);
      unlink(victim, bck, fwd);
      set_inuse_bit_at_offset(victim, victim_size);
      check_malloced_chunk(victim, nb);
      VALGRIND_MALLOCLIKE_BLOCK(chunk2mem(victim), bytes, SIZE_SZ, false);
      परत chunk2mem(victim);
    }

    idx += 2; /* Set for bin scan below. We've already scanned 2 bins. */

  }
  अन्यथा
  {
    idx = bin_index(nb);
    bin = bin_at(idx);

    साठी (victim = last(bin); victim != bin; victim = victim->bk)
    {
      victim_size = chunksize(victim);
      remainder_size = victim_size - nb;

      जर (remainder_size >= (दीर्घ)MINSIZE) /* too big */
      {
	--idx; /* adjust to rescan below after checking last remainder */
	थांबा;
      }

      अन्यथा जर (remainder_size >= 0) /* exact fit */
      {
	unlink(victim, bck, fwd);
	set_inuse_bit_at_offset(victim, victim_size);
	check_malloced_chunk(victim, nb);
        VALGRIND_MALLOCLIKE_BLOCK(chunk2mem(victim), bytes, SIZE_SZ, false);
	परत chunk2mem(victim);
      }
    }

    ++idx;

  }

  /* Try to use the last split-off remainder */

  जर ( (victim = last_remainder->fd) != last_remainder)
  {
    victim_size = chunksize(victim);
    remainder_size = victim_size - nb;

    जर (remainder_size >= (दीर्घ)MINSIZE) /* re-split */
    {
      remainder = chunk_at_offset(victim, nb);
      set_head(victim, nb | PREV_INUSE);
      link_last_remainder(remainder);
      set_head(remainder, remainder_size | PREV_INUSE);
      set_foot(remainder, remainder_size);
      check_malloced_chunk(victim, nb);
      VALGRIND_MALLOCLIKE_BLOCK(chunk2mem(victim), bytes, SIZE_SZ, false);
      परत chunk2mem(victim);
    }

    clear_last_remainder;

    जर (remainder_size >= 0)  /* exhaust */
    {
      set_inuse_bit_at_offset(victim, victim_size);
      check_malloced_chunk(victim, nb);
      VALGRIND_MALLOCLIKE_BLOCK(chunk2mem(victim), bytes, SIZE_SZ, false);
      परत chunk2mem(victim);
    }

    /* Else place in bin */

    frontlink(victim, victim_size, remainder_index, bck, fwd);
  }

  /*
     If there are any possibly nonempty big-enough blocks,
     search for best fitting chunk by scanning bins in blockwidth units.
  */

  जर ( (block = idx2binblock(idx)) <= binblocks_r)
  {

    /* Get to the first marked block */

    जर ( (block & binblocks_r) == 0)
    {
      /* force to an even block boundary */
      idx = (idx & ~(BINBLOCKWIDTH - 1)) + BINBLOCKWIDTH;
      block <<= 1;
      पर्यंत ((block & binblocks_r) == 0)
      {
	idx += BINBLOCKWIDTH;
	block <<= 1;
      }
    }

    /* For each possibly nonempty block ... */
    साठी (;;)
    {
      startidx = idx;          /* (track incomplete blocks) */
      q = bin = bin_at(idx);

      /* For each bin in this block ... */
      करा
      {
	/* Find and use first big enough chunk ... */

	साठी (victim = last(bin); victim != bin; victim = victim->bk)
	{
	  victim_size = chunksize(victim);
	  remainder_size = victim_size - nb;

	  जर (remainder_size >= (दीर्घ)MINSIZE) /* split */
	  {
	    remainder = chunk_at_offset(victim, nb);
	    set_head(victim, nb | PREV_INUSE);
	    unlink(victim, bck, fwd);
	    link_last_remainder(remainder);
	    set_head(remainder, remainder_size | PREV_INUSE);
	    set_foot(remainder, remainder_size);
	    check_malloced_chunk(victim, nb);
	    VALGRIND_MALLOCLIKE_BLOCK(chunk2mem(victim), bytes, SIZE_SZ, false);
	    परत chunk2mem(victim);
	  }

	  अन्यथा जर (remainder_size >= 0)  /* take */
	  {
	    set_inuse_bit_at_offset(victim, victim_size);
	    unlink(victim, bck, fwd);
	    check_malloced_chunk(victim, nb);
	    VALGRIND_MALLOCLIKE_BLOCK(chunk2mem(victim), bytes, SIZE_SZ, false);
	    परत chunk2mem(victim);
	  }

	}

       bin = next_bin(bin);

      } पर्यंत ((++idx & (BINBLOCKWIDTH - 1)) != 0);

      /* Clear out the block bit. */

      करा   /* Possibly backtrack to try to clear a partial block */
      {
	जर ((startidx & (BINBLOCKWIDTH - 1)) == 0)
	{
	  av_[1] = (mbinptr)(binblocks_r & ~block);
	  थांबा;
	}
	--startidx;
       q = prev_bin(q);
      } पर्यंत (first(q) == q);

      /* Get to the next possibly nonempty block */

      जर ( (block <<= 1) <= binblocks_r && (block != 0) )
      {
	पर्यंत ((block & binblocks_r) == 0)
	{
	  idx += BINBLOCKWIDTH;
	  block <<= 1;
	}
      }
      अन्यथा
	थांबा;
    }
  }

  /* Try to use top chunk */

  /* Require that there be a remainder, ensuring top always exists  */
  जर ( (remainder_size = chunksize(शीर्ष) - nb) < (दीर्घ)MINSIZE)
  {

#जर HAVE_MMAP
    /* If big and would otherwise need to extend, try to use mmap instead */
    जर ((अचिन्हित दीर्घ)nb >= (अचिन्हित दीर्घ)mmap_threshold &&
	(victim = mmap_chunk(nb)))
      VALGRIND_MALLOCLIKE_BLOCK(chunk2mem(victim), bytes, SIZE_SZ, false);
      परत chunk2mem(victim);
#endif

    /* Try to extend */
    malloc_extend_शीर्ष(nb);
    जर ( (remainder_size = chunksize(शीर्ष) - nb) < (दीर्घ)MINSIZE)
      परत रिक्त; /* propagate failure */
  }

  victim = शीर्ष;
  set_head(victim, nb | PREV_INUSE);
  शीर्ष = chunk_at_offset(victim, nb);
  set_head(शीर्ष, remainder_size | PREV_INUSE);
  check_malloced_chunk(victim, nb);
  VALGRIND_MALLOCLIKE_BLOCK(chunk2mem(victim), bytes, SIZE_SZ, false);
  परत chunk2mem(victim);

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
#जर __STD_C
शून्य fREe_impl(Void_t* mem)
#अन्यथा
शून्य fREe_impl(mem) Void_t* mem;
#endif
{
  mchunkptr p;         /* chunk corresponding to mem */
  INTERNAL_SIZE_T hd;  /* its head field */
  INTERNAL_SIZE_T sz;  /* its size */
  पूर्णांक       idx;       /* its bin index */
  mchunkptr next;      /* next contiguous chunk */
  INTERNAL_SIZE_T nextsz; /* its size */
  INTERNAL_SIZE_T prevsz; /* size of previous contiguous chunk */
  mchunkptr bck;       /* misc temp for linking */
  mchunkptr fwd;       /* misc temp for linking */
  पूर्णांक       islr;      /* track whether merging with last_remainder */

#जर CONFIG_IS_ENABLED(SYS_MALLOC_F)
	/* free() is a no-op - all the memory will be freed on relocation */
	जर (!(gd->flags & GD_FLG_FULL_MALLOC_INIT)) {
		VALGRIND_FREELIKE_BLOCK(mem, SIZE_SZ);
		परत;
	}
#endif

  जर (mem == रिक्त)                              /* free(0) has no effect */
    परत;

  p = mem2chunk(mem);
  hd = p->size;

#जर HAVE_MMAP
  जर (hd & IS_MMAPPED)                       /* release mmapped memory. */
  {
    munmap_chunk(p);
    परत;
  }
#endif

  check_inuse_chunk(p);

  sz = hd & ~PREV_INUSE;
  next = chunk_at_offset(p, sz);
  nextsz = chunksize(next);
  VALGRIND_FREELIKE_BLOCK(mem, SIZE_SZ);

  जर (next == शीर्ष)                            /* merge with top */
  {
    sz += nextsz;

    जर (!(hd & PREV_INUSE))                    /* consolidate backward */
    {
      prevsz = p->prev_size;
      p = chunk_at_offset(p, -((दीर्घ) prevsz));
      sz += prevsz;
      unlink(p, bck, fwd);
    }

    set_head(p, sz | PREV_INUSE);
    शीर्ष = p;
    जर ((अचिन्हित दीर्घ)(sz) >= (अचिन्हित दीर्घ)trim_threshold)
      malloc_trim(शीर्ष_pad);
    परत;
  }

  set_head(next, nextsz);                    /* clear inuse bit */

  islr = 0;

  जर (!(hd & PREV_INUSE))                    /* consolidate backward */
  {
    prevsz = p->prev_size;
    p = chunk_at_offset(p, -((दीर्घ) prevsz));
    sz += prevsz;

    जर (p->fd == last_remainder)             /* keep as last_remainder */
      islr = 1;
    अन्यथा
      unlink(p, bck, fwd);
  }

  जर (!(inuse_bit_at_offset(next, nextsz)))   /* consolidate forward */
  {
    sz += nextsz;

    जर (!islr && next->fd == last_remainder)  /* re-insert last_remainder */
    {
      islr = 1;
      link_last_remainder(p);
    }
    अन्यथा
      unlink(next, bck, fwd);
  }

  set_head(p, sz | PREV_INUSE);
  set_foot(p, sz);
  जर (!islr)
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
#जर __STD_C
Void_t* rEALLOc_impl(Void_t* oldmem, माप_प्रकार bytes)
#अन्यथा
Void_t* rEALLOc_impl(oldmem, bytes) Void_t* oldmem; माप_प्रकार bytes;
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
  जर (!bytes) {
	fREe_impl(oldmem);
	परत रिक्त;
  }
#endif

  जर (bytes > CONFIG_SYS_MALLOC_LEN || (दीर्घ)bytes < 0)
     परत रिक्त;

  /* realloc of null is supposed to be same as malloc */
  जर (oldmem == रिक्त) परत mALLOc_impl(bytes);

#जर CONFIG_IS_ENABLED(SYS_MALLOC_F)
	जर (!(gd->flags & GD_FLG_FULL_MALLOC_INIT)) {
		/* This is harder to support and should not be needed */
		panic("pre-reloc realloc() is not supported");
	}
#endif
  जर (CONFIG_IS_ENABLED(UNIT_TEST) && malloc_testing) {
    जर (--malloc_max_allocs < 0)
      परत रिक्त;
  }

  newp    = oldp    = mem2chunk(oldmem);
  newsize = oldsize = chunksize(oldp);

  nb = request2size(bytes);

#जर HAVE_MMAP
  जर (chunk_is_mmapped(oldp))
  {
#जर HAVE_MREMAP
    newp = mremap_chunk(oldp, nb);
    जर(newp) परत chunk2mem(newp);
#endif
    /* Note the extra SIZE_SZ overhead. */
    जर(oldsize - SIZE_SZ >= nb) परत oldmem; /* do nothing */
    /* Must alloc, copy, free. */
    newmem = mALLOc_impl(bytes);
    जर (!newmem)
	परत रिक्त; /* propagate failure */
    MALLOC_COPY(newmem, oldmem, oldsize - 2*SIZE_SZ);
    munmap_chunk(oldp);
    परत newmem;
  }
#endif

  check_inuse_chunk(oldp);

  जर ((दीर्घ)(oldsize) < (दीर्घ)(nb))
  {

    /* Try expanding forward */

    next = chunk_at_offset(oldp, oldsize);
    जर (next == शीर्ष || !inuse(next))
    {
      nextsize = chunksize(next);

      /* Forward into top only if a remainder */
      जर (next == शीर्ष)
      {
	जर ((दीर्घ)(nextsize + newsize) >= (दीर्घ)(nb + MINSIZE))
	{
	  newsize += nextsize;
	  शीर्ष = chunk_at_offset(oldp, nb);
	  set_head(शीर्ष, (newsize - nb) | PREV_INUSE);
	  set_head_size(oldp, nb);
	  VALGRIND_RESIZEINPLACE_BLOCK(chunk2mem(oldp), 0, bytes, SIZE_SZ);
	  VALGRIND_MAKE_MEM_DEFINED(chunk2mem(oldp), bytes);
	  परत chunk2mem(oldp);
	}
      }

      /* Forward into next chunk */
      अन्यथा जर (((दीर्घ)(nextsize + newsize) >= (दीर्घ)(nb)))
      {
	unlink(next, bck, fwd);
	newsize  += nextsize;
	VALGRIND_RESIZEINPLACE_BLOCK(chunk2mem(oldp), 0, bytes, SIZE_SZ);
	VALGRIND_MAKE_MEM_DEFINED(chunk2mem(oldp), bytes);
	जा split;
      }
    }
    अन्यथा
    {
      next = रिक्त;
      nextsize = 0;
    }

    /* Try shifting backwards. */

    जर (!prev_inuse(oldp))
    {
      prev = prev_chunk(oldp);
      prevsize = chunksize(prev);

      /* try forward + backward first to save a later consolidation */

      जर (next != रिक्त)
      {
	/* into top */
	जर (next == शीर्ष)
	{
	  जर ((दीर्घ)(nextsize + prevsize + newsize) >= (दीर्घ)(nb + MINSIZE))
	  {
	    unlink(prev, bck, fwd);
	    newp = prev;
	    newsize += prevsize + nextsize;
	    newmem = chunk2mem(newp);
	    VALGRIND_MALLOCLIKE_BLOCK(newmem, bytes, SIZE_SZ, false);
	    MALLOC_COPY(newmem, oldmem, oldsize - SIZE_SZ);
	    शीर्ष = chunk_at_offset(newp, nb);
	    set_head(शीर्ष, (newsize - nb) | PREV_INUSE);
	    set_head_size(newp, nb);
	    VALGRIND_FREELIKE_BLOCK(oldmem, SIZE_SZ);
	    परत newmem;
	  }
	}

	/* into next chunk */
	अन्यथा जर (((दीर्घ)(nextsize + prevsize + newsize) >= (दीर्घ)(nb)))
	{
	  unlink(next, bck, fwd);
	  unlink(prev, bck, fwd);
	  newp = prev;
	  newsize += nextsize + prevsize;
	  newmem = chunk2mem(newp);
	  VALGRIND_MALLOCLIKE_BLOCK(newmem, bytes, SIZE_SZ, false);
	  MALLOC_COPY(newmem, oldmem, oldsize - SIZE_SZ);
	  जा split;
	}
      }

      /* backward only */
      जर (prev != रिक्त && (दीर्घ)(prevsize + newsize) >= (दीर्घ)nb)
      {
	unlink(prev, bck, fwd);
	newp = prev;
	newsize += prevsize;
	newmem = chunk2mem(newp);
	VALGRIND_MALLOCLIKE_BLOCK(newmem, bytes, SIZE_SZ, false);
	MALLOC_COPY(newmem, oldmem, oldsize - SIZE_SZ);
	जा split;
      }
    }

    /* Must allocate */

    newmem = mALLOc_impl (bytes);

    जर (newmem == रिक्त)  /* propagate failure */
      परत रिक्त;

    /* Avoid copy if newp is next chunk after oldp. */
    /* (This can only happen when new chunk is sbrk'ed.) */

    जर ( (newp = mem2chunk(newmem)) == next_chunk(oldp))
    {
      newsize += chunksize(newp);
      newp = oldp;
      जा split;
    }

    /* Otherwise copy, free, and exit */
    MALLOC_COPY(newmem, oldmem, oldsize - SIZE_SZ);
    fREe_impl(oldmem);
    परत newmem;
  } अन्यथा {
    VALGRIND_RESIZEINPLACE_BLOCK(oldmem, 0, bytes, SIZE_SZ);
    VALGRIND_MAKE_MEM_DEFINED(oldmem, bytes);
  }

 split:  /* split off extra room in old or expanded chunk */

  जर (newsize - nb >= MINSIZE) /* split off remainder */
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
  अन्यथा
  {
    set_head_size(newp, newsize);
    set_inuse_bit_at_offset(newp, newsize);
  }

  check_inuse_chunk(newp);
  परत chunk2mem(newp);
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
#जर __STD_C
Void_t* mEMALIGn_impl(माप_प्रकार alignment, माप_प्रकार bytes)
#अन्यथा
Void_t* mEMALIGn_impl(alignment, bytes) माप_प्रकार alignment; माप_प्रकार bytes;
#endif
{
  INTERNAL_SIZE_T    nb;      /* padded  request size */
  अक्षर*     m;                /* memory returned by malloc call */
  mchunkptr p;                /* corresponding chunk */
  अक्षर*     brk;              /* alignment point within p */
  mchunkptr newp;             /* chunk to return */
  INTERNAL_SIZE_T  newsize;   /* its size */
  INTERNAL_SIZE_T  leadsize;  /* leading space befor alignment point */
  mchunkptr remainder;        /* spare room at end to split off */
  दीर्घ      remainder_size;   /* its size */

  जर (bytes > CONFIG_SYS_MALLOC_LEN || (दीर्घ)bytes < 0)
     परत रिक्त;

#जर CONFIG_IS_ENABLED(SYS_MALLOC_F)
	जर (!(gd->flags & GD_FLG_FULL_MALLOC_INIT)) {
		परत memalign_simple(alignment, bytes);
	}
#endif

  /* If need less alignment than we give anyway, just relay to malloc */

  जर (alignment <= MALLOC_ALIGNMENT) परत mALLOc_impl(bytes);

  /* Otherwise, ensure that it is at least a minimum chunk size */

  जर (alignment <  MINSIZE) alignment = MINSIZE;

  /* Call malloc with worst case padding to hit alignment. */

  nb = request2size(bytes);
  m  = (अक्षर*)(mALLOc_impl(nb + alignment + MINSIZE));

  /*
  * The attempt to over-allocate (with a size large enough to guarantee the
  * ability to find an aligned region within allocated memory) failed.
  *
  * Try again, this time only allocating exactly the size the user wants. If
  * the allocation now succeeds and just happens to be aligned, we can still
  * fulfill the user's request.
  */
  जर (m == रिक्त) {
    माप_प्रकार extra, extra2;
    /*
     * Use bytes not nb, since mALLOc internally calls request2size too, and
     * each call increases the size to allocate, to account for the header.
     */
    m  = (अक्षर*)(mALLOc_impl(bytes));
    /* Aligned -> return it */
    जर ((((अचिन्हित दीर्घ)(m)) % alignment) == 0)
      परत m;
    /*
     * Otherwise, try again, requesting enough extra space to be able to
     * acquire alignment.
     */
    fREe_impl(m);
    /* Add in extra bytes to match misalignment of unexpanded allocation */
    extra = alignment - (((अचिन्हित दीर्घ)(m)) % alignment);
    m  = (अक्षर*)(mALLOc_impl(bytes + extra));
    /*
     * m might not be the same as before. Validate that the previous value of
     * extra still works for the current value of m.
     * If (!m), extra2=alignment so 
     */
    जर (m) {
      extra2 = alignment - (((अचिन्हित दीर्घ)(m)) % alignment);
      जर (extra2 > extra) {
        fREe_impl(m);
        m = रिक्त;
      }
    }
    /* Fall through to original NULL check and chunk splitting logic */
  }

  जर (m == रिक्त) परत रिक्त; /* propagate failure */

  p = mem2chunk(m);

  जर ((((अचिन्हित दीर्घ)(m)) % alignment) == 0) /* aligned */
  {
#जर HAVE_MMAP
    जर(chunk_is_mmapped(p))
      परत chunk2mem(p); /* nothing more to do */
#endif
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

    brk = (अक्षर*)mem2chunk(((अचिन्हित दीर्घ)(m + alignment - 1)) & -((चिन्हित) alignment));
    जर ((दीर्घ)(brk - (अक्षर*)(p)) < MINSIZE) brk = brk + alignment;

    newp = (mchunkptr)brk;
    leadsize = brk - (अक्षर*)(p);
    newsize = chunksize(p) - leadsize;

#जर HAVE_MMAP
    जर(chunk_is_mmapped(p))
    {
      newp->prev_size = p->prev_size + leadsize;
      set_head(newp, newsize|IS_MMAPPED);
      परत chunk2mem(newp);
    }
#endif

    /* give back leader, use the rest */

    set_head(newp, newsize | PREV_INUSE);
    set_inuse_bit_at_offset(newp, newsize);
    set_head_size(p, leadsize);
    fREe_impl(chunk2mem(p));
    p = newp;
    VALGRIND_MALLOCLIKE_BLOCK(chunk2mem(p), bytes, SIZE_SZ, false);

    प्रतिज्ञा (newsize >= nb && (((अचिन्हित दीर्घ)(chunk2mem(p))) % alignment) == 0);
  }

  /* Also give back spare room at the end */

  remainder_size = chunksize(p) - nb;

  जर (remainder_size >= (दीर्घ)MINSIZE)
  {
    remainder = chunk_at_offset(p, nb);
    set_head(remainder, remainder_size | PREV_INUSE);
    set_head_size(p, nb);
    VALGRIND_MALLOCLIKE_BLOCK(chunk2mem(remainder), remainder_size, SIZE_SZ,
			      false);
    fREe_impl(chunk2mem(remainder));
  }

  check_inuse_chunk(p);
  परत chunk2mem(p);

}

/*
    valloc just invokes memalign with alignment argument equal
    to the page size of the system (or as near to this as can
    be figured out from all the includes/defines above.)
*/

#जर __STD_C
Void_t* vALLOc(माप_प्रकार bytes)
#अन्यथा
Void_t* vALLOc(bytes) माप_प्रकार bytes;
#endif
{
  परत mEMALIGn (malloc_getpagesize, bytes);
}

/*
  pvalloc just invokes valloc for the nearest pagesize
  that will accommodate request
*/

#जर __STD_C
Void_t* pvALLOc(माप_प्रकार bytes)
#अन्यथा
Void_t* pvALLOc(bytes) माप_प्रकार bytes;
#endif
{
  माप_प्रकार pagesize = malloc_getpagesize;
  परत mEMALIGn (pagesize, (bytes + pagesize - 1) & ~(pagesize - 1));
}

/*

  calloc calls malloc, then zeroes out the allocated chunk.

*/

STATIC_IF_MCHECK
#जर __STD_C
Void_t* cALLOc_impl(माप_प्रकार n, माप_प्रकार elem_size)
#अन्यथा
Void_t* cALLOc_impl(n, elem_size) माप_प्रकार n; माप_प्रकार elem_size;
#endif
{
  mchunkptr p;
  INTERNAL_SIZE_T csz;

  INTERNAL_SIZE_T sz = n * elem_size;

  /* check if expand_top called, in which case don't need to clear */
#जर CONFIG_IS_ENABLED(SYS_MALLOC_CLEAR_ON_INIT)
#जर MORECORE_CLEARS
  mchunkptr oldtop = शीर्ष;
  INTERNAL_SIZE_T oldtopsize = chunksize(शीर्ष);
#endif
#endif
  Void_t* mem = mALLOc_impl (sz);

  जर ((दीर्घ)n < 0) परत रिक्त;

  जर (mem == रिक्त)
    परत रिक्त;
  अन्यथा
  {
#जर CONFIG_IS_ENABLED(SYS_MALLOC_F)
	जर (!(gd->flags & GD_FLG_FULL_MALLOC_INIT)) {
		स_रखो(mem, 0, sz);
		परत mem;
	}
#endif
    p = mem2chunk(mem);

    /* Two optional cases in which clearing not necessary */

#जर HAVE_MMAP
    जर (chunk_is_mmapped(p)) परत mem;
#endif

    csz = chunksize(p);

#जर CONFIG_IS_ENABLED(SYS_MALLOC_CLEAR_ON_INIT)
#जर MORECORE_CLEARS
    जर (p == oldtop && csz > oldtopsize)
    {
      /* clear only the bytes from non-freshly-sbrked memory */
      csz = oldtopsize;
    }
#endif
#endif

    MALLOC_ZERO(mem, csz - SIZE_SZ);
    VALGRIND_MAKE_MEM_DEFINED(mem, sz);
    परत mem;
  }
}

/*

  cfree just calls free. It is needed/defined on some systems
  that pair it with calloc, presumably for odd historical reasons.

*/

#जर !defined(INTERNAL_LINUX_C_LIB) || !defined(__ELF__)
#जर __STD_C
शून्य cfree(Void_t *mem)
#अन्यथा
शून्य cfree(mem) Void_t *mem;
#endif
{
  fREe(mem);
}
#endif

#ifdef MCHECK_HEAP_PROTECTION
 #अन्तर्भाव "mcheck_core.inc.h"
 #जर !__STD_C
  #error "must have __STD_C"
 #endif

Void_t *mALLOc(माप_प्रकार bytes)
{
	mcheck_pedantic_prehook();
	माप_प्रकार fullsz = mcheck_alloc_prehook(bytes);
	शून्य *p = mALLOc_impl(fullsz);

	जर (!p)
		परत p;
	परत mcheck_alloc_posthook(p, bytes);
}

शून्य fREe(Void_t *mem) { fREe_impl(mcheck_free_prehook(mem)); }

Void_t *rEALLOc(Void_t *oldmem, माप_प्रकार bytes)
{
	mcheck_pedantic_prehook();
	जर (bytes == 0) {
		जर (oldmem)
			fREe(oldmem);
		परत रिक्त;
	}

	जर (oldmem == रिक्त)
		परत mALLOc(bytes);

	शून्य *p = mcheck_reallocfree_prehook(oldmem);
	माप_प्रकार newsz = mcheck_alloc_prehook(bytes);

	p = rEALLOc_impl(p, newsz);
	जर (!p)
		परत p;
	परत mcheck_alloc_noclean_posthook(p, bytes);
}

Void_t *mEMALIGn(माप_प्रकार alignment, माप_प्रकार bytes)
{
	mcheck_pedantic_prehook();
	माप_प्रकार fullsz = mcheck_memalign_prehook(alignment, bytes);
	शून्य *p = mEMALIGn_impl(alignment, fullsz);

	जर (!p)
		परत p;
	परत mcheck_memalign_posthook(alignment, p, bytes);
}

// pvALLOc, vALLOc - redirect to mEMALIGn, defined here, so they need no wrapping.

Void_t *cALLOc(माप_प्रकार n, माप_प्रकार elem_size)
{
	mcheck_pedantic_prehook();
	// NB: here is no overflow check.
	माप_प्रकार fullsz = mcheck_alloc_prehook(n * elem_size);
	शून्य *p = cALLOc_impl(1, fullsz);

	जर (!p)
		परत p;
	परत mcheck_alloc_noclean_posthook(p, n * elem_size);
}

// mcheck API {
पूर्णांक mcheck_pedantic(mcheck_abortfunc_t f)
{
	mcheck_initialize(f, 1);
	परत 0;
}

पूर्णांक mcheck(mcheck_abortfunc_t f)
{
	mcheck_initialize(f, 0);
	परत 0;
}

शून्य mcheck_check_all(शून्य) { mcheck_pedantic_check(); }

गणना mcheck_status mprobe(शून्य *__ptr) { परत mcheck_mprobe(__ptr); }
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

#जर __STD_C
पूर्णांक malloc_trim(माप_प्रकार pad)
#अन्यथा
पूर्णांक malloc_trim(pad) माप_प्रकार pad;
#endif
{
  दीर्घ  शीर्ष_size;        /* Amount of top-most memory */
  दीर्घ  extra;           /* Amount to release */
  अक्षर* current_brk;     /* address returned by pre-check sbrk call */
  अक्षर* new_brk;         /* address returned by negative sbrk call */

  अचिन्हित दीर्घ pagesz = malloc_getpagesize;

  शीर्ष_size = chunksize(शीर्ष);
  extra = ((शीर्ष_size - pad - MINSIZE + (pagesz-1)) / pagesz - 1) * pagesz;

  जर (extra < (दीर्घ)pagesz)  /* Not enough memory to release */
    परत 0;

  अन्यथा
  {
    /* Test to make sure no one else called sbrk */
    current_brk = (अक्षर*)(MORECORE (0));
    जर (current_brk != (अक्षर*)(शीर्ष) + शीर्ष_size)
      परत 0;     /* Apparently we don't own memory; must fail */

    अन्यथा
    {
      new_brk = (अक्षर*)(MORECORE (-extra));

      जर (new_brk == (अक्षर*)(MORECORE_FAILURE)) /* sbrk failed? */
      {
	/* Try to figure out what we have */
	current_brk = (अक्षर*)(MORECORE (0));
	शीर्ष_size = current_brk - (अक्षर*)शीर्ष;
	जर (शीर्ष_size >= (दीर्घ)MINSIZE) /* if not, we are very very dead! */
	{
	  sbrked_mem = current_brk - sbrk_base;
	  set_head(शीर्ष, शीर्ष_size | PREV_INUSE);
	}
	check_chunk(शीर्ष);
	परत 0;
      }

      अन्यथा
      {
	/* Success. Adjust top accordingly. */
	set_head(शीर्ष, (शीर्ष_size - extra) | PREV_INUSE);
	sbrked_mem -= extra;
	check_chunk(शीर्ष);
	परत 1;
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

#जर __STD_C
माप_प्रकार malloc_usable_size(Void_t* mem)
#अन्यथा
माप_प्रकार malloc_usable_size(mem) Void_t* mem;
#endif
{
  mchunkptr p;
  जर (mem == रिक्त)
    परत 0;
  अन्यथा
  {
    p = mem2chunk(mem);
    जर(!chunk_is_mmapped(p))
    {
      जर (!inuse(p)) परत 0;
      check_inuse_chunk(p);
      परत chunksize(p) - SIZE_SZ;
    }
    परत chunksize(p) - 2*SIZE_SZ;
  }
}

/* Utility to update current_mallinfo for malloc_stats and mallinfo() */

#ifdef DEBUG
स्थिर शून्य malloc_update_mallinfo(शून्य)
{
  पूर्णांक i;
  mbinptr b;
  mchunkptr p;
#ifdef DEBUG
  mchunkptr q;
#endif

  INTERNAL_SIZE_T avail = chunksize(शीर्ष);
  पूर्णांक   navail = ((दीर्घ)(avail) >= (दीर्घ)MINSIZE)? 1 : 0;

  साठी (i = 1; i < NAV; ++i)
  {
    b = bin_at(i);
    साठी (p = last(b); p != b; p = p->bk)
    {
#ifdef DEBUG
      check_free_chunk(p);
      साठी (q = next_chunk(p);
	   q < शीर्ष && inuse(q) && (दीर्घ)(chunksize(q)) >= (दीर्घ)MINSIZE;
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
  current_mallinfo.keepcost = chunksize(शीर्ष);

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
शून्य malloc_stats(शून्य)
{
  malloc_update_mallinfo();
  म_लिखो("max system bytes = %10u\n",
	  (अचिन्हित पूर्णांक)(max_total_mem));
  म_लिखो("system bytes     = %10u\n",
	  (अचिन्हित पूर्णांक)(sbrked_mem + mmapped_mem));
  म_लिखो("in use bytes     = %10u\n",
	  (अचिन्हित पूर्णांक)(current_mallinfo.uordblks + mmapped_mem));
#जर HAVE_MMAP
  म_लिखो("max mmap regions = %10u\n",
	  (अचिन्हित पूर्णांक)max_n_mmaps);
#endif
}
#endif	/* DEBUG */

/*
  mallinfo returns a copy of updated current mallinfo.
*/

#ifdef DEBUG
संरचना mallinfo mALLINFo(शून्य)
{
  malloc_update_mallinfo();
  परत current_mallinfo;
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

#जर __STD_C
पूर्णांक mALLOPt(पूर्णांक param_संख्या, पूर्णांक मूल्य)
#अन्यथा
पूर्णांक mALLOPt(param_संख्या, मूल्य) पूर्णांक param_संख्या; पूर्णांक मूल्य;
#endif
{
  निवड(param_संख्या)
  {
    स्थिती M_TRIM_THRESHOLD:
      trim_threshold = मूल्य; परत 1;
    स्थिती M_TOP_PAD:
      शीर्ष_pad = मूल्य; परत 1;
    स्थिती M_MMAP_THRESHOLD:
      mmap_threshold = मूल्य; परत 1;
    स्थिती M_MMAP_MAX:
#जर HAVE_MMAP
      n_mmaps_max = मूल्य; परत 1;
#अन्यथा
      जर (मूल्य != 0) परत 0; अन्यथा  n_mmaps_max = मूल्य; परत 1;
#endif

    पूर्वनिर्धारित:
      परत 0;
  }
}

पूर्णांक initf_malloc(शून्य)
{
#जर CONFIG_IS_ENABLED(SYS_MALLOC_F)
	प्रतिज्ञा(gd->malloc_base);	/* Set up by crt0.S */
	gd->malloc_limit = CONFIG_VAL(SYS_MALLOC_F_LEN);
	gd->malloc_ptr = 0;
#endif

	परत 0;
}

शून्य malloc_enable_testing(पूर्णांक max_allocs)
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
