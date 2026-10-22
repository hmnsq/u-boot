// SPDX-License-Identifier: GPL-2.0+
/*
 * This code is based on a version (aka dlmalloc) of malloc/free/realloc written
 * by Doug Lea and released to the public domain, as explained at
 * http://creativecommons.org/publicdomain/zero/1.0/-
 *
 * The original code is available at http://gee.cs.oswego.edu/pub/misc/
 * as file malloc-2.6.6.c.
 */

#अगर CONFIG_IS_ENABLED(UNIT_TEST)
#घोषणा DEBUG
#पूर्ण_अगर

#समावेश <log.h>
#समावेश <asm/global_data.h>

#समावेश <malloc.h>
#समावेश <mapmem.h>
#समावेश <string.h>
#समावेश <asm/io.h>
#समावेश <valgrind/memcheck.h>

#अगर_घोषित DEBUG
#अगर __STD_C
अटल व्योम malloc_update_mallinfo (व्योम);
व्योम malloc_stats (व्योम);
#अन्यथा
अटल व्योम malloc_update_mallinfo ();
व्योम malloc_stats();
#पूर्ण_अगर
#पूर्ण_अगर	/* DEBUG */

DECLARE_GLOBAL_तथ्य_PTR;

#अगर_घोषित MCHECK_HEAP_PROTECTION
 #घोषणा STATIC_IF_MCHECK अटल
 #अघोषित MALLOC_COPY
 #अघोषित MALLOC_ZERO
अटल अंतरभूत व्योम MALLOC_ZERO(व्योम *p, माप_प्रकार sz) { स_रखो(p, 0, sz); }
अटल अंतरभूत व्योम MALLOC_COPY(व्योम *dest, स्थिर व्योम *src, माप_प्रकार sz) { स_नकल(dest, src, sz); }
#अन्यथा
 #घोषणा STATIC_IF_MCHECK
 #घोषणा mALLOc_impl mALLOc
 #घोषणा fREe_impl fREe
 #घोषणा rEALLOc_impl rEALLOc
 #घोषणा mEMALIGn_impl mEMALIGn
 #घोषणा cALLOc_impl cALLOc
#पूर्ण_अगर

/*
  Emulation of sbrk for WIN32
  All code within the ifdef WIN32 is untested by me.

  Thanks to Martin Fong and others for supplying this.
*/

#अगर_घोषित WIN32

#घोषणा AlignPage(add) (((add) + (malloc_getpagesize-1)) & \
~(malloc_getpagesize-1))
#घोषणा AlignPage64K(add) (((add) + (0x10000 - 1)) & ~(0x10000 - 1))

/* resrve 64MB to insure large contiguous space */
#घोषणा RESERVED_आकार (1024*1024*64)
#घोषणा अग्र_आकार (2048*1024)
#घोषणा शीर्ष_MEMORY ((अचिन्हित दीर्घ)2*1024*1024*1024)

काष्ठा GmListElement;
प्रकार काष्ठा GmListElement GmListElement;

काष्ठा GmListElement
{
	GmListElement* अग्र;
	व्योम* base;
};

अटल GmListElement* शीर्ष_2 = 0;
अटल अचिन्हित पूर्णांक g_अग्र_Address = 0;
अटल अचिन्हित पूर्णांक gAddressBase = 0;
अटल अचिन्हित पूर्णांक g_Allocated_आकार = 0;

अटल
GmListElement* makeGmListElement (व्योम* bas)
{
	GmListElement* this;
	this = (GmListElement*)(व्योम*)LocalAlloc (0, माप (GmListElement));
	निश्चित (this);
	अगर (this)
	{
		this->base = bas;
		this->अग्र = शीर्ष_2;
		शीर्ष_2 = this;
	}
	वापस this;
}

व्योम gcleanup (व्योम)
{
	BOOL rval;
	निश्चित ( (शीर्ष_2 == शून्य) || (शीर्ष_2->base == (व्योम*)gAddressBase));
	अगर (gAddressBase && (g_अग्र_Address - gAddressBase))
	{
		rval = VirtualFree ((व्योम*)gAddressBase,
							g_अग्र_Address - gAddressBase,
							MEM_DECOMMIT);
	निश्चित (rval);
	}
	जबतक (शीर्ष_2)
	{
		GmListElement* अग्र = शीर्ष_2->अग्र;
		rval = VirtualFree (शीर्ष_2->base, 0, MEM_RELEASE);
		निश्चित (rval);
		LocalFree (शीर्ष_2);
		शीर्ष_2 = अग्र;
	}
}

अटल
व्योम* findRegion (व्योम* start_address, अचिन्हित दीर्घ आकार)
{
	MEMORY_BASIC_INFORMATION info;
	अगर (आकार >= शीर्ष_MEMORY) वापस शून्य;

	जबतक ((अचिन्हित दीर्घ)start_address + आकार < शीर्ष_MEMORY)
	{
		VirtualQuery (start_address, &info, माप (info));
		अगर ((info.State == MEM_FREE) && (info.Region_आकार >= आकार))
			वापस start_address;
		अन्यथा
		{
			/* Requested region is not available so see if the */
			/* next region is available.  Set 'start_address' */
			/* to the next region and call 'VirtualQuery()' */
			/* again. */

			start_address = (अक्षर*)info.BaseAddress + info.Region_आकार;

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
				(व्योम *) AlignPage64K((अचिन्हित दीर्घ) start_address);
		}
	}
	वापस शून्य;

}

व्योम* wsbrk (दीर्घ आकार)
{
	व्योम* tmp;
	अगर (आकार > 0)
	{
		अगर (gAddressBase == 0)
		{
			g_Allocated_आकार = max (RESERVED_आकार, AlignPage (आकार));
			g_अग्र_Address = gAddressBase =
				(अचिन्हित पूर्णांक)VirtualAlloc (शून्य, g_Allocated_आकार,
											MEM_RESERVE, PAGE_NOACCESS);
		} अन्यथा अगर (AlignPage (g_अग्र_Address + आकार) > (gAddressBase +
g_Allocated_आकार))
		{
			दीर्घ new_आकार = max (अग्र_आकार, AlignPage (आकार));
			व्योम* new_address = (व्योम*)(gAddressBase+g_Allocated_आकार);
			करो
			{
				new_address = findRegion (new_address, new_आकार);

				अगर (!new_address)
					वापस (व्योम*)-1;

				gAddressBase = g_अग्र_Address =
					(अचिन्हित पूर्णांक)VirtualAlloc (new_address, new_आकार,
												MEM_RESERVE, PAGE_NOACCESS);
				/* repeat in case of race condition */
				/* The region that we found has been snagged */
				/* by another thread */
			}
			जबतक (gAddressBase == 0);

			निश्चित (new_address == (व्योम*)gAddressBase);

			g_Allocated_आकार = new_आकार;

			अगर (!makeGmListElement ((व्योम*)gAddressBase))
				वापस (व्योम*)-1;
		}
		अगर ((आकार + g_अग्र_Address) > AlignPage (g_अग्र_Address))
		{
			व्योम* res;
			res = VirtualAlloc ((व्योम*)AlignPage (g_अग्र_Address),
								(आकार + g_अग्र_Address -
								 AlignPage (g_अग्र_Address)),
								MEM_COMMIT, PAGE_READWRITE);
			अगर (!res)
				वापस (व्योम*)-1;
		}
		tmp = (व्योम*)g_अग्र_Address;
		g_अग्र_Address = (अचिन्हित पूर्णांक)tmp + आकार;
		वापस tmp;
	}
	अन्यथा अगर (आकार < 0)
	{
		अचिन्हित पूर्णांक alignedGoal = AlignPage (g_अग्र_Address + आकार);
		/* Trim by releasing the virtual memory */
		अगर (alignedGoal >= gAddressBase)
		{
			VirtualFree ((व्योम*)alignedGoal, g_अग्र_Address - alignedGoal,
						 MEM_DECOMMIT);
			g_अग्र_Address = g_अग्र_Address + आकार;
			वापस (व्योम*)g_अग्र_Address;
		}
		अन्यथा
		{
			VirtualFree ((व्योम*)gAddressBase, g_अग्र_Address - gAddressBase,
						 MEM_DECOMMIT);
			g_अग्र_Address = gAddressBase;
			वापस (व्योम*)-1;
		}
	}
	अन्यथा
	{
		वापस (व्योम*)g_अग्र_Address;
	}
}

#पूर्ण_अगर

/*
  Type declarations
*/

काष्ठा malloc_chunk
{
  INTERNAL_आकार_T पूर्व_आकार; /* Size of previous chunk (if free). */
  INTERNAL_आकार_T आकार;      /* Size in bytes, including overhead. */
  काष्ठा malloc_chunk* fd;   /* double links -- used only if free. */
  काष्ठा malloc_chunk* bk;
} __attribute__((__may_alias__)) ;

प्रकार काष्ठा malloc_chunk* mchunkptr;

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

#घोषणा आकार_SZ                (माप(INTERNAL_आकार_T))
#घोषणा MALLOC_ALIGNMENT       (आकार_SZ + आकार_SZ)
#घोषणा MALLOC_ALIGN_MASK      (MALLOC_ALIGNMENT - 1)
#घोषणा MINSIZE                (माप(काष्ठा malloc_chunk))

/* conversion from malloc headers to user pointers, and back */

#घोषणा chunk2mem(p)   ((Void_t*)((अक्षर*)(p) + 2*आकार_SZ))
#घोषणा mem2chunk(mem) ((mchunkptr)((अक्षर*)(mem) - 2*आकार_SZ))

/* pad request bytes into a usable size */

#घोषणा request2size(req) \
 ((((req) + (आकार_SZ + MALLOC_ALIGN_MASK)) < \
  (MINSIZE + MALLOC_ALIGN_MASK)) ? MINSIZE : \
   (((req) + (आकार_SZ + MALLOC_ALIGN_MASK)) & ~(MALLOC_ALIGN_MASK)))

/* Check if m has acceptable alignment */

#घोषणा aligned_OK(m)    (((अचिन्हित दीर्घ)((m)) & (MALLOC_ALIGN_MASK)) == 0)

/*
  Physical chunk operations
*/

/* size field is or'ed with PREV_INUSE when previous adjacent chunk in use */

#घोषणा पूर्व_INUSE 0x1

/* size field is or'ed with IS_MMAPPED if the chunk was obtained with mmap() */

#घोषणा IS_MMAPPED 0x2

/* Bits to mask off when extracting size */

#घोषणा आकार_BITS (पूर्व_INUSE|IS_MMAPPED)

/* Ptr to next physical malloc_chunk. */

#घोषणा अग्र_chunk(p) ((mchunkptr)( ((अक्षर*)(p)) + ((p)->आकार & ~पूर्व_INUSE) ))

/* Ptr to previous physical malloc_chunk */

#घोषणा पूर्व_chunk(p)\
   ((mchunkptr)( ((अक्षर*)(p)) - ((p)->पूर्व_आकार) ))

/* Treat space at ptr + offset as a chunk */

#घोषणा chunk_at_offset(p, s)  ((mchunkptr)(((अक्षर*)(p)) + (s)))

/*
  Dealing with use bits
*/

/* extract p's inuse bit */

#घोषणा inuse(p)\
((((mchunkptr)(((अक्षर*)(p))+((p)->आकार & ~पूर्व_INUSE)))->आकार) & पूर्व_INUSE)

/* extract inuse bit of previous chunk */

#घोषणा पूर्व_inuse(p)  ((p)->आकार & पूर्व_INUSE)

/* check for mmap()'ed chunk */

#घोषणा chunk_is_mmapped(p) ((p)->आकार & IS_MMAPPED)

/* set/clear chunk as in use without otherwise disturbing */

#घोषणा set_inuse(p)\
((mchunkptr)(((अक्षर*)(p)) + ((p)->आकार & ~पूर्व_INUSE)))->आकार |= पूर्व_INUSE

#घोषणा clear_inuse(p)\
((mchunkptr)(((अक्षर*)(p)) + ((p)->आकार & ~पूर्व_INUSE)))->आकार &= ~(पूर्व_INUSE)

/* check/set/clear inuse bits in known places */

#घोषणा inuse_bit_at_offset(p, s)\
 (((mchunkptr)(((अक्षर*)(p)) + (s)))->आकार & पूर्व_INUSE)

#घोषणा set_inuse_bit_at_offset(p, s)\
 (((mchunkptr)(((अक्षर*)(p)) + (s)))->आकार |= पूर्व_INUSE)

#घोषणा clear_inuse_bit_at_offset(p, s)\
 (((mchunkptr)(((अक्षर*)(p)) + (s)))->आकार &= ~(पूर्व_INUSE))

/*
  Dealing with size fields
*/

/* Get size, ignoring use bits */

#घोषणा chunksize(p)          ((p)->आकार & ~(आकार_BITS))

/* Set size at head, without disturbing its use bit */

#घोषणा set_शीर्ष_आकार(p, s)   ((p)->आकार = (((p)->आकार & पूर्व_INUSE) | (s)))

/* Set size/use ignoring previous bits in header */

#घोषणा set_शीर्ष(p, s)        ((p)->आकार = (s))

/* Set size at footer (only when chunk is not in use) */

#घोषणा set_foot(p, s)   (((mchunkptr)((अक्षर*)(p) + (s)))->पूर्व_आकार = (s))

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

#घोषणा NAV             128   /* number of bins */

प्रकार काष्ठा malloc_chunk* mbinptr;

/* access macros */

#घोषणा bin_at(i)      ((mbinptr)((अक्षर*)&(av_[2*(i) + 2]) - 2*आकार_SZ))
#घोषणा अग्र_bin(b)    ((mbinptr)((अक्षर*)(b) + 2 * माप(mbinptr)))
#घोषणा पूर्व_bin(b)    ((mbinptr)((अक्षर*)(b) - 2 * माप(mbinptr)))

/*
   The first 2 bins are never indexed. The corresponding av_ cells are instead
   used for bookkeeping. This is not to save space, but to simplify
   indexing, maintain locality, and avoid some initialization tests.
*/

#घोषणा शीर्ष            (av_[2])          /* The topmost chunk */
#घोषणा last_remainder (bin_at(1))       /* remainder from last split */

/*
   Because top initially points to its own bin with initial
   zero size, thus forcing extension on the first malloc request,
   we avoid having any special code in malloc to check whether
   it even exists yet. But we still need to in malloc_extend_top.
*/

#घोषणा initial_शीर्ष    ((mchunkptr)(bin_at(0)))

/* Helper macro to initialize bins */

#घोषणा IAV(i)  bin_at(i), bin_at(i)

अटल mbinptr av_[NAV * 2 + 2] = {
 शून्य, शून्य,
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

#अगर_घोषित CONFIG_SYS_MALLOC_DEFAULT_TO_INIT
अटल व्योम malloc_init(व्योम);
#पूर्ण_अगर

ulong mem_malloc_start = 0;
ulong mem_malloc_end = 0;
ulong mem_malloc_brk = 0;

अटल bool malloc_testing;	/* enable test mode */
अटल पूर्णांक malloc_max_allocs;	/* return NULL after this many calls to malloc() */

व्योम *sbrk(सूचक_भेद_प्रकार increment)
{
	ulong old = mem_malloc_brk;
	ulong new = old + increment;

	अगर ((new < mem_malloc_start) || (new > mem_malloc_end))
		वापस (व्योम *)MORECORE_FAILURE;

	/*
	 * if we are giving memory back make sure we clear it out since
	 * we set MORECORE_CLEARS to 1
	 */
	अगर (increment < 0)
		स_रखो((व्योम *)new, 0, -increment);

	mem_malloc_brk = new;

	वापस (व्योम *)old;
}

व्योम mem_malloc_init(ulong start, ulong आकार)
{
	mem_malloc_start = (ulong)map_sysmem(start, आकार);
	mem_malloc_end = mem_malloc_start + आकार;
	mem_malloc_brk = mem_malloc_start;

#अगर_घोषित CONFIG_SYS_MALLOC_DEFAULT_TO_INIT
	malloc_init();
#पूर्ण_अगर

	debug("using memory %#lx-%#lx for malloc()\n", mem_malloc_start,
	      mem_malloc_end);
#अगर CONFIG_IS_ENABLED(SYS_MALLOC_CLEAR_ON_INIT)
	स_रखो((व्योम *)mem_malloc_start, 0x0, आकार);
#पूर्ण_अगर
}

/* field-extraction macros */

#घोषणा first(b) ((b)->fd)
#घोषणा last(b)  ((b)->bk)

/*
  Indexing into bins
*/

#घोषणा bin_सूचकांक(sz)                                                          \
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

#घोषणा MAX_SMALLBIN         63
#घोषणा MAX_SMALLBIN_आकार   512
#घोषणा SMALLBIN_WIDTH        8

#घोषणा smallbin_सूचकांक(sz)  (((अचिन्हित दीर्घ)(sz)) >> 3)

/*
   Requests are `small' if both the corresponding and the next bin are small
*/

#घोषणा is_small_request(nb) (nb < MAX_SMALLBIN_आकार - SMALLBIN_WIDTH)

/*
    To help compensate for the large number of bins, a one-level index
    structure is used for bin-by-bin searching.  `binblocks' is a
    one-word bitvector recording whether groups of BINBLOCKWIDTH bins
    have any (possibly) non-empty bins, so they can be skipped over
    all at once during during traversals. The bits are NOT always
    cleared as soon as all bins in a block are empty, but instead only
    when all are noticed to be empty during traversal in malloc.
*/

#घोषणा BINBLOCKWIDTH     4   /* bins per block */

#घोषणा binblocks_r     ((INTERNAL_आकार_T)av_[1]) /* bitvector of nonempty blocks */
#घोषणा binblocks_w     (av_[1])

/* bin<->block macros */

#घोषणा idx2binblock(ix)    ((अचिन्हित)1 << (ix / BINBLOCKWIDTH))
#घोषणा mark_binblock(ii)   (binblocks_w = (mbinptr)(binblocks_r | idx2binblock(ii)))
#घोषणा clear_binblock(ii)  (binblocks_w = (mbinptr)(binblocks_r & ~(idx2binblock(ii))))

/*  Other static bookkeeping data */

/* variables holding tunable values */

अटल अचिन्हित दीर्घ trim_threshold   = DEFAULT_TRIM_THRESHOLD;
अटल अचिन्हित दीर्घ शीर्ष_pad          = DEFAULT_शीर्ष_PAD;
अटल अचिन्हित पूर्णांक  n_mmaps_max      = DEFAULT_MMAP_MAX;
अटल अचिन्हित दीर्घ mmap_threshold   = DEFAULT_MMAP_THRESHOLD;

/* The first value returned from sbrk */
अटल अक्षर* sbrk_base = (अक्षर*)(-1);

/* The maximum memory obtained from system via sbrk */
अटल अचिन्हित दीर्घ max_sbrked_mem = 0;

/* The maximum via either sbrk or mmap */
अटल अचिन्हित दीर्घ max_योग_mem = 0;

/* internal working copy of mallinfo */
अटल काष्ठा mallinfo current_mallinfo = {  0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

/* The total memory obtained from system via sbrk */
#घोषणा sbrked_mem  (current_mallinfo.arena)

/* Tracking mmaps */

#अगर_घोषित DEBUG
अटल अचिन्हित पूर्णांक n_mmaps = 0;
#पूर्ण_अगर	/* DEBUG */
अटल अचिन्हित दीर्घ mmapped_mem = 0;
#अगर HAVE_MMAP
अटल अचिन्हित पूर्णांक max_n_mmaps = 0;
अटल अचिन्हित दीर्घ max_mmapped_mem = 0;
#पूर्ण_अगर

#अगर_घोषित CONFIG_SYS_MALLOC_DEFAULT_TO_INIT
अटल व्योम malloc_init(व्योम)
{
	पूर्णांक i, j;

	debug("bins (av_ array) are at %p\n", (व्योम *)av_);

	av_[0] = शून्य; av_[1] = शून्य;
	क्रम (i = 2, j = 2; i < NAV * 2 + 2; i += 2, j++) {
		av_[i] = bin_at(j - 2);
		av_[i + 1] = bin_at(j - 2);

		/* Just print the first few bins so that
		 * we can see there are alright.
		 */
		अगर (i < 10)
			debug("av_[%d]=%lx av_[%d]=%lx\n",
			      i, (ulong)av_[i],
			      i + 1, (ulong)av_[i + 1]);
	}

	/* Init the static bookkeeping as well */
	sbrk_base = (अक्षर *)(-1);
	max_sbrked_mem = 0;
	max_योग_mem = 0;
#अगर_घोषित DEBUG
	स_रखो((व्योम *)&current_mallinfo, 0, माप(काष्ठा mallinfo));
#पूर्ण_अगर
}
#पूर्ण_अगर

/*
  Debugging support
*/

#अगर_घोषित DEBUG

/*
  These routines make a number of assertions about the states
  of data structures that should be true at all times. If any
  are not true, it's very likely that a user program has somehow
  trashed memory. (It's also possible that there is a coding error
  in malloc. In which case, please report it!)
*/

#अगर __STD_C
अटल व्योम do_check_chunk(mchunkptr p)
#अन्यथा
अटल व्योम do_check_chunk(p) mchunkptr p;
#पूर्ण_अगर
{
  INTERNAL_आकार_T sz = p->आकार & ~पूर्व_INUSE;

  /* No checkable chunk is mmapped */
  निश्चित(!chunk_is_mmapped(p));

  /* Check for legal address ... */
  निश्चित((अक्षर*)p >= sbrk_base);
  अगर (p != शीर्ष)
    निश्चित((अक्षर*)p + sz <= (अक्षर*)शीर्ष);
  अन्यथा
    निश्चित((अक्षर*)p + sz <= sbrk_base + sbrked_mem);

}

#अगर __STD_C
अटल व्योम do_check_free_chunk(mchunkptr p)
#अन्यथा
अटल व्योम do_check_free_chunk(p) mchunkptr p;
#पूर्ण_अगर
{
  INTERNAL_आकार_T sz = p->आकार & ~पूर्व_INUSE;
  mchunkptr अग्र = chunk_at_offset(p, sz);

  do_check_chunk(p);

  /* Check whether it claims to be free ... */
  निश्चित(!inuse(p));

  /* Unless a special marker, must have OK fields */
  अगर ((दीर्घ)sz >= (दीर्घ)MINSIZE)
  {
    निश्चित((sz & MALLOC_ALIGN_MASK) == 0);
    निश्चित(aligned_OK(chunk2mem(p)));
    /* ... matching footer field */
    निश्चित(अग्र->पूर्व_आकार == sz);
    /* ... and is fully consolidated */
    निश्चित(पूर्व_inuse(p));
    निश्चित (अग्र == शीर्ष || inuse(अग्र));

    /* ... and has minimally sane links */
    निश्चित(p->fd->bk == p);
    निश्चित(p->bk->fd == p);
  }
  अन्यथा /* markers are always of size SIZE_SZ */
    निश्चित(sz == आकार_SZ);
}

#अगर __STD_C
अटल व्योम do_check_inuse_chunk(mchunkptr p)
#अन्यथा
अटल व्योम do_check_inuse_chunk(p) mchunkptr p;
#पूर्ण_अगर
{
  mchunkptr अग्र = अग्र_chunk(p);
  do_check_chunk(p);

  /* Check whether it claims to be in use ... */
  निश्चित(inuse(p));

  /* ... and is surrounded by OK chunks.
    Since more things can be checked with free chunks than inuse ones,
    if an inuse chunk borders them and debug is on, it's worth doing them.
  */
  अगर (!पूर्व_inuse(p))
  {
    mchunkptr prv = पूर्व_chunk(p);
    निश्चित(अग्र_chunk(prv) == p);
    do_check_free_chunk(prv);
  }
  अगर (अग्र == शीर्ष)
  {
    निश्चित(पूर्व_inuse(अग्र));
    निश्चित(chunksize(अग्र) >= MINSIZE);
  }
  अन्यथा अगर (!inuse(अग्र))
    do_check_free_chunk(अग्र);

}

#अगर __STD_C
अटल व्योम do_check_malloced_chunk(mchunkptr p, INTERNAL_आकार_T s)
#अन्यथा
अटल व्योम do_check_malloced_chunk(p, s) mchunkptr p; INTERNAL_आकार_T s;
#पूर्ण_अगर
{
  INTERNAL_आकार_T sz = p->आकार & ~पूर्व_INUSE;
  दीर्घ room = sz - s;

  do_check_inuse_chunk(p);

  /* Legal size ... */
  निश्चित((दीर्घ)sz >= (दीर्घ)MINSIZE);
  निश्चित((sz & MALLOC_ALIGN_MASK) == 0);
  निश्चित(room >= 0);
  निश्चित(room < (दीर्घ)MINSIZE);

  /* ... and alignment */
  निश्चित(aligned_OK(chunk2mem(p)));

  /* ... and was allocated at front of an available chunk */
  निश्चित(पूर्व_inuse(p));

}

#घोषणा check_free_chunk(P)  do_check_free_chunk(P)
#घोषणा check_inuse_chunk(P) do_check_inuse_chunk(P)
#घोषणा check_chunk(P) do_check_chunk(P)
#घोषणा check_malloced_chunk(P,N) do_check_malloced_chunk(P,N)
#अन्यथा
#घोषणा check_free_chunk(P)
#घोषणा check_inuse_chunk(P)
#घोषणा check_chunk(P)
#घोषणा check_malloced_chunk(P,N)
#पूर्ण_अगर

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

#घोषणा frontlink(P, S, IDX, BK, FD)                                          \
{                                                                             \
  अगर (S < MAX_SMALLBIN_आकार)                                                  \
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
    अगर (FD == BK) mark_binblock(IDX);                                         \
    अन्यथा                                                                      \
    {                                                                         \
      जबतक (FD != BK && S < chunksize(FD)) FD = FD->fd;                      \
      BK = FD->bk;                                                            \
    }                                                                         \
    P->bk = BK;                                                               \
    P->fd = FD;                                                               \
    FD->bk = BK->fd = P;                                                      \
  }                                                                           \
}

/* take a chunk off a list */

#घोषणा unlink(P, BK, FD)                                                     \
{                                                                             \
  BK = P->bk;                                                                 \
  FD = P->fd;                                                                 \
  FD->bk = BK;                                                                \
  BK->fd = FD;                                                                \
}                                                                             \

/* Place p as the last remainder */

#घोषणा link_last_remainder(P)                                                \
{                                                                             \
  last_remainder->fd = last_remainder->bk =  P;                               \
  P->fd = P->bk = last_remainder;                                             \
}

/* Clear the last_remainder bin */

#घोषणा clear_last_remainder \
  (last_remainder->fd = last_remainder->bk = last_remainder)

/* Routines dealing with mmap(). */

#अगर HAVE_MMAP

#अगर __STD_C
अटल mchunkptr mmap_chunk(माप_प्रकार आकार)
#अन्यथा
अटल mchunkptr mmap_chunk(आकार) माप_प्रकार आकार;
#पूर्ण_अगर
{
  माप_प्रकार page_mask = malloc_getpagesize - 1;
  mchunkptr p;

#अगर_अघोषित MAP_ANONYMOUS
  अटल पूर्णांक fd = -1;
#पूर्ण_अगर

  अगर(n_mmaps >= n_mmaps_max) वापस 0; /* too many regions */

  /* For mmapped chunks, the overhead is one SIZE_SZ unit larger, because
   * there is no following chunk whose prev_size field could be used.
   */
  आकार = (आकार + आकार_SZ + page_mask) & ~page_mask;

#अगर_घोषित MAP_ANONYMOUS
  p = (mchunkptr)mmap(0, आकार, PROT_READ|PROT_WRITE,
		      MAP_PRIVATE|MAP_ANONYMOUS, -1, 0);
#अन्यथा /* !MAP_ANONYMOUS */
  अगर (fd < 0)
  {
    fd = खोलो("/dev/zero", O_RDWR);
    अगर(fd < 0) वापस 0;
  }
  p = (mchunkptr)mmap(0, आकार, PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0);
#पूर्ण_अगर

  अगर(p == (mchunkptr)-1) वापस 0;

  n_mmaps++;
  अगर (n_mmaps > max_n_mmaps) max_n_mmaps = n_mmaps;

  /* We demand that eight bytes into a page must be 8-byte aligned. */
  निश्चित(aligned_OK(chunk2mem(p)));

  /* The offset to the start of the mmapped region is stored
   * in the prev_size field of the chunk; normally it is zero,
   * but that can be changed in memalign().
   */
  p->पूर्व_आकार = 0;
  set_शीर्ष(p, आकार|IS_MMAPPED);

  mmapped_mem += आकार;
  अगर ((अचिन्हित दीर्घ)mmapped_mem > (अचिन्हित दीर्घ)max_mmapped_mem)
    max_mmapped_mem = mmapped_mem;
  अगर ((अचिन्हित दीर्घ)(mmapped_mem + sbrked_mem) > (अचिन्हित दीर्घ)max_योग_mem)
    max_योग_mem = mmapped_mem + sbrked_mem;
  वापस p;
}

#अगर __STD_C
अटल व्योम munmap_chunk(mchunkptr p)
#अन्यथा
अटल व्योम munmap_chunk(p) mchunkptr p;
#पूर्ण_अगर
{
  INTERNAL_आकार_T आकार = chunksize(p);
  पूर्णांक ret;

  निश्चित (chunk_is_mmapped(p));
  निश्चित(! ((अक्षर*)p >= sbrk_base && (अक्षर*)p < sbrk_base + sbrked_mem));
  निश्चित((n_mmaps > 0));
  निश्चित(((p->पूर्व_आकार + आकार) & (malloc_getpagesize-1)) == 0);

  n_mmaps--;
  mmapped_mem -= (आकार + p->पूर्व_आकार);

  ret = munmap((अक्षर *)p - p->पूर्व_आकार, आकार + p->पूर्व_आकार);

  /* munmap returns non-zero on failure */
  निश्चित(ret == 0);
}

#अगर HAVE_MREMAP

#अगर __STD_C
अटल mchunkptr mremap_chunk(mchunkptr p, माप_प्रकार new_आकार)
#अन्यथा
अटल mchunkptr mremap_chunk(p, new_आकार) mchunkptr p; माप_प्रकार new_आकार;
#पूर्ण_अगर
{
  माप_प्रकार page_mask = malloc_getpagesize - 1;
  INTERNAL_आकार_T offset = p->पूर्व_आकार;
  INTERNAL_आकार_T आकार = chunksize(p);
  अक्षर *cp;

  निश्चित (chunk_is_mmapped(p));
  निश्चित(! ((अक्षर*)p >= sbrk_base && (अक्षर*)p < sbrk_base + sbrked_mem));
  निश्चित((n_mmaps > 0));
  निश्चित(((आकार + offset) & (malloc_getpagesize-1)) == 0);

  /* Note the extra SIZE_SZ overhead as in mmap_chunk(). */
  new_आकार = (new_आकार + offset + आकार_SZ + page_mask) & ~page_mask;

  cp = (अक्षर *)mremap((अक्षर *)p - offset, आकार + offset, new_आकार, 1);

  अगर (cp == (अक्षर *)-1) वापस 0;

  p = (mchunkptr)(cp + offset);

  निश्चित(aligned_OK(chunk2mem(p)));

  निश्चित((p->पूर्व_आकार == offset));
  set_शीर्ष(p, (new_आकार - offset)|IS_MMAPPED);

  mmapped_mem -= आकार + offset;
  mmapped_mem += new_आकार;
  अगर ((अचिन्हित दीर्घ)mmapped_mem > (अचिन्हित दीर्घ)max_mmapped_mem)
    max_mmapped_mem = mmapped_mem;
  अगर ((अचिन्हित दीर्घ)(mmapped_mem + sbrked_mem) > (अचिन्हित दीर्घ)max_योग_mem)
    max_योग_mem = mmapped_mem + sbrked_mem;
  वापस p;
}

#पूर्ण_अगर /* HAVE_MREMAP */

#पूर्ण_अगर /* HAVE_MMAP */

/*
  Extend the top-most chunk by obtaining memory from system.
  Main interface to sbrk (but see also malloc_trim).
*/

#अगर __STD_C
अटल व्योम malloc_extend_शीर्ष(INTERNAL_आकार_T nb)
#अन्यथा
अटल व्योम malloc_extend_शीर्ष(nb) INTERNAL_आकार_T nb;
#पूर्ण_अगर
{
  अक्षर*     brk;                  /* return value from sbrk */
  INTERNAL_आकार_T front_misalign; /* unusable bytes at front of sbrked space */
  INTERNAL_आकार_T correction;     /* bytes for 2nd sbrk call */
  अक्षर*     new_brk;              /* return of 2nd sbrk call */
  INTERNAL_आकार_T शीर्ष_आकार;       /* new size of top chunk */

  mchunkptr old_शीर्ष     = शीर्ष;  /* Record state of old top */
  INTERNAL_आकार_T old_शीर्ष_आकार = chunksize(old_शीर्ष);
  अक्षर*     old_end      = (अक्षर*)(chunk_at_offset(old_शीर्ष, old_शीर्ष_आकार));

  /* Pad request with top_pad plus minimal overhead */

  INTERNAL_आकार_T    sbrk_आकार     = nb + शीर्ष_pad + MINSIZE;
  अचिन्हित दीर्घ pagesz    = malloc_getpagesize;

  /* If not the first time through, round to preserve page boundary */
  /* Otherwise, we need to correct to a page size below anyway. */
  /* (We also correct below if an intervening foreign sbrk call.) */

  अगर (sbrk_base != (अक्षर*)(-1))
    sbrk_आकार = (sbrk_आकार + (pagesz - 1)) & ~(pagesz - 1);

  brk = (अक्षर*)(MORECORE (sbrk_आकार));

  /* Fail if sbrk failed or if a foreign sbrk call killed our space */
  अगर (brk == (अक्षर*)(MORECORE_FAILURE) ||
      (brk < old_end && old_शीर्ष != initial_शीर्ष))
    वापस;

  sbrked_mem += sbrk_आकार;

  अगर (brk == old_end) /* can just add bytes to current top */
  {
    शीर्ष_आकार = sbrk_आकार + old_शीर्ष_आकार;
    set_शीर्ष(शीर्ष, शीर्ष_आकार | पूर्व_INUSE);
  }
  अन्यथा
  {
    अगर (sbrk_base == (अक्षर*)(-1))  /* First time through. Record base */
      sbrk_base = brk;
    अन्यथा  /* Someone else called sbrk().  Count those bytes as sbrked_mem. */
      sbrked_mem += brk - (अक्षर*)old_end;

    /* Guarantee alignment of first new chunk made from this space */
    front_misalign = (अचिन्हित दीर्घ)chunk2mem(brk) & MALLOC_ALIGN_MASK;
    अगर (front_misalign > 0)
    {
      correction = (MALLOC_ALIGNMENT) - front_misalign;
      brk += correction;
    }
    अन्यथा
      correction = 0;

    /* Guarantee the next brk will be at a page boundary */

    correction += ((((अचिन्हित दीर्घ)(brk + sbrk_आकार))+(pagesz-1)) &
		   ~(pagesz - 1)) - ((अचिन्हित दीर्घ)(brk + sbrk_आकार));

    /* Allocate correction */
    new_brk = (अक्षर*)(MORECORE (correction));
    अगर (new_brk == (अक्षर*)(MORECORE_FAILURE)) वापस;

    sbrked_mem += correction;

    शीर्ष = (mchunkptr)brk;
    शीर्ष_आकार = new_brk - brk + correction;
    set_शीर्ष(शीर्ष, शीर्ष_आकार | पूर्व_INUSE);

    अगर (old_शीर्ष != initial_शीर्ष)
    {

      /* There must have been an intervening foreign sbrk call. */
      /* A double fencepost is necessary to prevent consolidation */

      /* If not enough space to do this, then user did something very wrong */
      अगर (old_शीर्ष_आकार < MINSIZE)
      {
	set_शीर्ष(शीर्ष, पूर्व_INUSE); /* will force null return from malloc */
	वापस;
      }

      /* Also keep size a multiple of MALLOC_ALIGNMENT */
      old_शीर्ष_आकार = (old_शीर्ष_आकार - 3*आकार_SZ) & ~MALLOC_ALIGN_MASK;
      set_शीर्ष_आकार(old_शीर्ष, old_शीर्ष_आकार);
      chunk_at_offset(old_शीर्ष, old_शीर्ष_आकार          )->आकार =
	आकार_SZ|पूर्व_INUSE;
      chunk_at_offset(old_शीर्ष, old_शीर्ष_आकार + आकार_SZ)->आकार =
	आकार_SZ|पूर्व_INUSE;
      /* If possible, release the rest. */
      अगर (old_शीर्ष_आकार >= MINSIZE)
	fREe(chunk2mem(old_शीर्ष));
    }
  }

  अगर ((अचिन्हित दीर्घ)sbrked_mem > (अचिन्हित दीर्घ)max_sbrked_mem)
    max_sbrked_mem = sbrked_mem;
  अगर ((अचिन्हित दीर्घ)(mmapped_mem + sbrked_mem) > (अचिन्हित दीर्घ)max_योग_mem)
    max_योग_mem = mmapped_mem + sbrked_mem;

  /* We always land on a page boundary */
  निश्चित(((अचिन्हित दीर्घ)((अक्षर*)शीर्ष + शीर्ष_आकार) & (pagesz - 1)) == 0);
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
#अगर __STD_C
Void_t* mALLOc_impl(माप_प्रकार bytes)
#अन्यथा
Void_t* mALLOc_impl(bytes) माप_प्रकार bytes;
#पूर्ण_अगर
{
  mchunkptr victim;                  /* inspected/selected chunk */
  INTERNAL_आकार_T victim_आकार;       /* its size */
  पूर्णांक       idx;                     /* index for bin traversal */
  mbinptr   bin;                     /* associated bin */
  mchunkptr remainder;               /* remainder from a split */
  दीर्घ      remainder_आकार;          /* its size */
  पूर्णांक       remainder_सूचकांक;         /* its bin index */
  अचिन्हित दीर्घ block;               /* block traverser bit */
  पूर्णांक       startidx;                /* first bin of a traversed block */
  mchunkptr fwd;                     /* misc temp for linking */
  mchunkptr bck;                     /* misc temp for linking */
  mbinptr q;                         /* misc temp */

  INTERNAL_आकार_T nb;

#अगर CONFIG_IS_ENABLED(SYS_MALLOC_F)
	अगर (!(gd->flags & GD_FLG_FULL_MALLOC_INIT))
		वापस malloc_simple(bytes);
#पूर्ण_अगर

  अगर (CONFIG_IS_ENABLED(UNIT_TEST) && malloc_testing) {
    अगर (--malloc_max_allocs < 0)
      वापस शून्य;
  }

  /* check if mem_malloc_init() was run */
  अगर ((mem_malloc_start == 0) && (mem_malloc_end == 0)) {
    /* not initialized yet */
    वापस शून्य;
  }

  अगर (bytes > CONFIG_SYS_MALLOC_LEN || (दीर्घ)bytes < 0)
     वापस शून्य;

  nb = request2size(bytes);  /* padded request size; */

  /* Check for exact match in a bin */

  अगर (is_small_request(nb))  /* Faster version for small requests */
  {
    idx = smallbin_सूचकांक(nb);

    /* No traversal or size check necessary for small bins.  */

    q = bin_at(idx);
    victim = last(q);

    /* Also scan the next one, since it would have a remainder < MINSIZE */
    अगर (victim == q)
    {
      q = अग्र_bin(q);
      victim = last(q);
    }
    अगर (victim != q)
    {
      victim_आकार = chunksize(victim);
      unlink(victim, bck, fwd);
      set_inuse_bit_at_offset(victim, victim_आकार);
      check_malloced_chunk(victim, nb);
      VALGRIND_MALLOCLIKE_BLOCK(chunk2mem(victim), bytes, आकार_SZ, false);
      वापस chunk2mem(victim);
    }

    idx += 2; /* Set for bin scan below. We've already scanned 2 bins. */

  }
  अन्यथा
  {
    idx = bin_सूचकांक(nb);
    bin = bin_at(idx);

    क्रम (victim = last(bin); victim != bin; victim = victim->bk)
    {
      victim_आकार = chunksize(victim);
      remainder_आकार = victim_आकार - nb;

      अगर (remainder_आकार >= (दीर्घ)MINSIZE) /* too big */
      {
	--idx; /* adjust to rescan below after checking last remainder */
	अवरोध;
      }

      अन्यथा अगर (remainder_आकार >= 0) /* exact fit */
      {
	unlink(victim, bck, fwd);
	set_inuse_bit_at_offset(victim, victim_आकार);
	check_malloced_chunk(victim, nb);
        VALGRIND_MALLOCLIKE_BLOCK(chunk2mem(victim), bytes, आकार_SZ, false);
	वापस chunk2mem(victim);
      }
    }

    ++idx;

  }

  /* Try to use the last split-off remainder */

  अगर ( (victim = last_remainder->fd) != last_remainder)
  {
    victim_आकार = chunksize(victim);
    remainder_आकार = victim_आकार - nb;

    अगर (remainder_आकार >= (दीर्घ)MINSIZE) /* re-split */
    {
      remainder = chunk_at_offset(victim, nb);
      set_शीर्ष(victim, nb | पूर्व_INUSE);
      link_last_remainder(remainder);
      set_शीर्ष(remainder, remainder_आकार | पूर्व_INUSE);
      set_foot(remainder, remainder_आकार);
      check_malloced_chunk(victim, nb);
      VALGRIND_MALLOCLIKE_BLOCK(chunk2mem(victim), bytes, आकार_SZ, false);
      वापस chunk2mem(victim);
    }

    clear_last_remainder;

    अगर (remainder_आकार >= 0)  /* exhaust */
    {
      set_inuse_bit_at_offset(victim, victim_आकार);
      check_malloced_chunk(victim, nb);
      VALGRIND_MALLOCLIKE_BLOCK(chunk2mem(victim), bytes, आकार_SZ, false);
      वापस chunk2mem(victim);
    }

    /* Else place in bin */

    frontlink(victim, victim_आकार, remainder_सूचकांक, bck, fwd);
  }

  /*
     If there are any possibly nonempty big-enough blocks,
     search for best fitting chunk by scanning bins in blockwidth units.
  */

  अगर ( (block = idx2binblock(idx)) <= binblocks_r)
  {

    /* Get to the first marked block */

    अगर ( (block & binblocks_r) == 0)
    {
      /* force to an even block boundary */
      idx = (idx & ~(BINBLOCKWIDTH - 1)) + BINBLOCKWIDTH;
      block <<= 1;
      जबतक ((block & binblocks_r) == 0)
      {
	idx += BINBLOCKWIDTH;
	block <<= 1;
      }
    }

    /* For each possibly nonempty block ... */
    क्रम (;;)
    {
      startidx = idx;          /* (track incomplete blocks) */
      q = bin = bin_at(idx);

      /* For each bin in this block ... */
      करो
      {
	/* Find and use first big enough chunk ... */

	क्रम (victim = last(bin); victim != bin; victim = victim->bk)
	{
	  victim_आकार = chunksize(victim);
	  remainder_आकार = victim_आकार - nb;

	  अगर (remainder_आकार >= (दीर्घ)MINSIZE) /* split */
	  {
	    remainder = chunk_at_offset(victim, nb);
	    set_शीर्ष(victim, nb | पूर्व_INUSE);
	    unlink(victim, bck, fwd);
	    link_last_remainder(remainder);
	    set_शीर्ष(remainder, remainder_आकार | पूर्व_INUSE);
	    set_foot(remainder, remainder_आकार);
	    check_malloced_chunk(victim, nb);
	    VALGRIND_MALLOCLIKE_BLOCK(chunk2mem(victim), bytes, आकार_SZ, false);
	    वापस chunk2mem(victim);
	  }

	  अन्यथा अगर (remainder_आकार >= 0)  /* take */
	  {
	    set_inuse_bit_at_offset(victim, victim_आकार);
	    unlink(victim, bck, fwd);
	    check_malloced_chunk(victim, nb);
	    VALGRIND_MALLOCLIKE_BLOCK(chunk2mem(victim), bytes, आकार_SZ, false);
	    वापस chunk2mem(victim);
	  }

	}

       bin = अग्र_bin(bin);

      } जबतक ((++idx & (BINBLOCKWIDTH - 1)) != 0);

      /* Clear out the block bit. */

      करो   /* Possibly backtrack to try to clear a partial block */
      {
	अगर ((startidx & (BINBLOCKWIDTH - 1)) == 0)
	{
	  av_[1] = (mbinptr)(binblocks_r & ~block);
	  अवरोध;
	}
	--startidx;
       q = पूर्व_bin(q);
      } जबतक (first(q) == q);

      /* Get to the next possibly nonempty block */

      अगर ( (block <<= 1) <= binblocks_r && (block != 0) )
      {
	जबतक ((block & binblocks_r) == 0)
	{
	  idx += BINBLOCKWIDTH;
	  block <<= 1;
	}
      }
      अन्यथा
	अवरोध;
    }
  }

  /* Try to use top chunk */

  /* Require that there be a remainder, ensuring top always exists  */
  अगर ( (remainder_आकार = chunksize(शीर्ष) - nb) < (दीर्घ)MINSIZE)
  {

#अगर HAVE_MMAP
    /* If big and would otherwise need to extend, try to use mmap instead */
    अगर ((अचिन्हित दीर्घ)nb >= (अचिन्हित दीर्घ)mmap_threshold &&
	(victim = mmap_chunk(nb)))
      VALGRIND_MALLOCLIKE_BLOCK(chunk2mem(victim), bytes, आकार_SZ, false);
      वापस chunk2mem(victim);
#पूर्ण_अगर

    /* Try to extend */
    malloc_extend_शीर्ष(nb);
    अगर ( (remainder_आकार = chunksize(शीर्ष) - nb) < (दीर्घ)MINSIZE)
      वापस शून्य; /* propagate failure */
  }

  victim = शीर्ष;
  set_शीर्ष(victim, nb | पूर्व_INUSE);
  शीर्ष = chunk_at_offset(victim, nb);
  set_शीर्ष(शीर्ष, remainder_आकार | पूर्व_INUSE);
  check_malloced_chunk(victim, nb);
  VALGRIND_MALLOCLIKE_BLOCK(chunk2mem(victim), bytes, आकार_SZ, false);
  वापस chunk2mem(victim);

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
#अगर __STD_C
व्योम fREe_impl(Void_t* mem)
#अन्यथा
व्योम fREe_impl(mem) Void_t* mem;
#पूर्ण_अगर
{
  mchunkptr p;         /* chunk corresponding to mem */
  INTERNAL_आकार_T hd;  /* its head field */
  INTERNAL_आकार_T sz;  /* its size */
  पूर्णांक       idx;       /* its bin index */
  mchunkptr अग्र;      /* next contiguous chunk */
  INTERNAL_आकार_T nextsz; /* its size */
  INTERNAL_आकार_T prevsz; /* size of previous contiguous chunk */
  mchunkptr bck;       /* misc temp for linking */
  mchunkptr fwd;       /* misc temp for linking */
  पूर्णांक       islr;      /* track whether merging with last_remainder */

#अगर CONFIG_IS_ENABLED(SYS_MALLOC_F)
	/* free() is a no-op - all the memory will be freed on relocation */
	अगर (!(gd->flags & GD_FLG_FULL_MALLOC_INIT)) {
		VALGRIND_FREELIKE_BLOCK(mem, आकार_SZ);
		वापस;
	}
#पूर्ण_अगर

  अगर (mem == शून्य)                              /* free(0) has no effect */
    वापस;

  p = mem2chunk(mem);
  hd = p->आकार;

#अगर HAVE_MMAP
  अगर (hd & IS_MMAPPED)                       /* release mmapped memory. */
  {
    munmap_chunk(p);
    वापस;
  }
#पूर्ण_अगर

  check_inuse_chunk(p);

  sz = hd & ~पूर्व_INUSE;
  अग्र = chunk_at_offset(p, sz);
  nextsz = chunksize(अग्र);
  VALGRIND_FREELIKE_BLOCK(mem, आकार_SZ);

  अगर (अग्र == शीर्ष)                            /* merge with top */
  {
    sz += nextsz;

    अगर (!(hd & पूर्व_INUSE))                    /* consolidate backward */
    {
      prevsz = p->पूर्व_आकार;
      p = chunk_at_offset(p, -((दीर्घ) prevsz));
      sz += prevsz;
      unlink(p, bck, fwd);
    }

    set_शीर्ष(p, sz | पूर्व_INUSE);
    शीर्ष = p;
    अगर ((अचिन्हित दीर्घ)(sz) >= (अचिन्हित दीर्घ)trim_threshold)
      malloc_trim(शीर्ष_pad);
    वापस;
  }

  set_शीर्ष(अग्र, nextsz);                    /* clear inuse bit */

  islr = 0;

  अगर (!(hd & पूर्व_INUSE))                    /* consolidate backward */
  {
    prevsz = p->पूर्व_आकार;
    p = chunk_at_offset(p, -((दीर्घ) prevsz));
    sz += prevsz;

    अगर (p->fd == last_remainder)             /* keep as last_remainder */
      islr = 1;
    अन्यथा
      unlink(p, bck, fwd);
  }

  अगर (!(inuse_bit_at_offset(अग्र, nextsz)))   /* consolidate forward */
  {
    sz += nextsz;

    अगर (!islr && अग्र->fd == last_remainder)  /* re-insert last_remainder */
    {
      islr = 1;
      link_last_remainder(p);
    }
    अन्यथा
      unlink(अग्र, bck, fwd);
  }

  set_शीर्ष(p, sz | पूर्व_INUSE);
  set_foot(p, sz);
  अगर (!islr)
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
#अगर __STD_C
Void_t* rEALLOc_impl(Void_t* oldmem, माप_प्रकार bytes)
#अन्यथा
Void_t* rEALLOc_impl(oldmem, bytes) Void_t* oldmem; माप_प्रकार bytes;
#पूर्ण_अगर
{
  INTERNAL_आकार_T    nb;      /* padded request size */

  mchunkptr oldp;             /* chunk corresponding to oldmem */
  INTERNAL_आकार_T    oldsize; /* its size */

  mchunkptr newp;             /* chunk to return */
  INTERNAL_आकार_T    newsize; /* its size */
  Void_t*   newmem;           /* corresponding user mem */

  mchunkptr अग्र;             /* next contiguous chunk after oldp */
  INTERNAL_आकार_T  nextsize;  /* its size */

  mchunkptr पूर्व;             /* previous contiguous chunk before oldp */
  INTERNAL_आकार_T  prevsize;  /* its size */

  mchunkptr remainder;        /* holds split off extra space from newp */
  INTERNAL_आकार_T  remainder_आकार;   /* its size */

  mchunkptr bck;              /* misc temp for linking */
  mchunkptr fwd;              /* misc temp for linking */

#अगर_घोषित REALLOC_ZERO_BYTES_FREES
  अगर (!bytes) {
	fREe_impl(oldmem);
	वापस शून्य;
  }
#पूर्ण_अगर

  अगर (bytes > CONFIG_SYS_MALLOC_LEN || (दीर्घ)bytes < 0)
     वापस शून्य;

  /* realloc of null is supposed to be same as malloc */
  अगर (oldmem == शून्य) वापस mALLOc_impl(bytes);

#अगर CONFIG_IS_ENABLED(SYS_MALLOC_F)
	अगर (!(gd->flags & GD_FLG_FULL_MALLOC_INIT)) {
		/* This is harder to support and should not be needed */
		panic("pre-reloc realloc() is not supported");
	}
#पूर्ण_अगर
  अगर (CONFIG_IS_ENABLED(UNIT_TEST) && malloc_testing) {
    अगर (--malloc_max_allocs < 0)
      वापस शून्य;
  }

  newp    = oldp    = mem2chunk(oldmem);
  newsize = oldsize = chunksize(oldp);

  nb = request2size(bytes);

#अगर HAVE_MMAP
  अगर (chunk_is_mmapped(oldp))
  {
#अगर HAVE_MREMAP
    newp = mremap_chunk(oldp, nb);
    अगर(newp) वापस chunk2mem(newp);
#पूर्ण_अगर
    /* Note the extra SIZE_SZ overhead. */
    अगर(oldsize - आकार_SZ >= nb) वापस oldmem; /* do nothing */
    /* Must alloc, copy, free. */
    newmem = mALLOc_impl(bytes);
    अगर (!newmem)
	वापस शून्य; /* propagate failure */
    MALLOC_COPY(newmem, oldmem, oldsize - 2*आकार_SZ);
    munmap_chunk(oldp);
    वापस newmem;
  }
#पूर्ण_अगर

  check_inuse_chunk(oldp);

  अगर ((दीर्घ)(oldsize) < (दीर्घ)(nb))
  {

    /* Try expanding forward */

    अग्र = chunk_at_offset(oldp, oldsize);
    अगर (अग्र == शीर्ष || !inuse(अग्र))
    {
      nextsize = chunksize(अग्र);

      /* Forward into top only if a remainder */
      अगर (अग्र == शीर्ष)
      {
	अगर ((दीर्घ)(nextsize + newsize) >= (दीर्घ)(nb + MINSIZE))
	{
	  newsize += nextsize;
	  शीर्ष = chunk_at_offset(oldp, nb);
	  set_शीर्ष(शीर्ष, (newsize - nb) | पूर्व_INUSE);
	  set_शीर्ष_आकार(oldp, nb);
	  VALGRIND_RESIZEINPLACE_BLOCK(chunk2mem(oldp), 0, bytes, आकार_SZ);
	  VALGRIND_MAKE_MEM_DEFINED(chunk2mem(oldp), bytes);
	  वापस chunk2mem(oldp);
	}
      }

      /* Forward into next chunk */
      अन्यथा अगर (((दीर्घ)(nextsize + newsize) >= (दीर्घ)(nb)))
      {
	unlink(अग्र, bck, fwd);
	newsize  += nextsize;
	VALGRIND_RESIZEINPLACE_BLOCK(chunk2mem(oldp), 0, bytes, आकार_SZ);
	VALGRIND_MAKE_MEM_DEFINED(chunk2mem(oldp), bytes);
	जाओ split;
      }
    }
    अन्यथा
    {
      अग्र = शून्य;
      nextsize = 0;
    }

    /* Try shifting backwards. */

    अगर (!पूर्व_inuse(oldp))
    {
      पूर्व = पूर्व_chunk(oldp);
      prevsize = chunksize(पूर्व);

      /* try forward + backward first to save a later consolidation */

      अगर (अग्र != शून्य)
      {
	/* into top */
	अगर (अग्र == शीर्ष)
	{
	  अगर ((दीर्घ)(nextsize + prevsize + newsize) >= (दीर्घ)(nb + MINSIZE))
	  {
	    unlink(पूर्व, bck, fwd);
	    newp = पूर्व;
	    newsize += prevsize + nextsize;
	    newmem = chunk2mem(newp);
	    VALGRIND_MALLOCLIKE_BLOCK(newmem, bytes, आकार_SZ, false);
	    MALLOC_COPY(newmem, oldmem, oldsize - आकार_SZ);
	    शीर्ष = chunk_at_offset(newp, nb);
	    set_शीर्ष(शीर्ष, (newsize - nb) | पूर्व_INUSE);
	    set_शीर्ष_आकार(newp, nb);
	    VALGRIND_FREELIKE_BLOCK(oldmem, आकार_SZ);
	    वापस newmem;
	  }
	}

	/* into next chunk */
	अन्यथा अगर (((दीर्घ)(nextsize + prevsize + newsize) >= (दीर्घ)(nb)))
	{
	  unlink(अग्र, bck, fwd);
	  unlink(पूर्व, bck, fwd);
	  newp = पूर्व;
	  newsize += nextsize + prevsize;
	  newmem = chunk2mem(newp);
	  VALGRIND_MALLOCLIKE_BLOCK(newmem, bytes, आकार_SZ, false);
	  MALLOC_COPY(newmem, oldmem, oldsize - आकार_SZ);
	  जाओ split;
	}
      }

      /* backward only */
      अगर (पूर्व != शून्य && (दीर्घ)(prevsize + newsize) >= (दीर्घ)nb)
      {
	unlink(पूर्व, bck, fwd);
	newp = पूर्व;
	newsize += prevsize;
	newmem = chunk2mem(newp);
	VALGRIND_MALLOCLIKE_BLOCK(newmem, bytes, आकार_SZ, false);
	MALLOC_COPY(newmem, oldmem, oldsize - आकार_SZ);
	जाओ split;
      }
    }

    /* Must allocate */

    newmem = mALLOc_impl (bytes);

    अगर (newmem == शून्य)  /* propagate failure */
      वापस शून्य;

    /* Avoid copy if newp is next chunk after oldp. */
    /* (This can only happen when new chunk is sbrk'ed.) */

    अगर ( (newp = mem2chunk(newmem)) == अग्र_chunk(oldp))
    {
      newsize += chunksize(newp);
      newp = oldp;
      जाओ split;
    }

    /* Otherwise copy, free, and exit */
    MALLOC_COPY(newmem, oldmem, oldsize - आकार_SZ);
    fREe_impl(oldmem);
    वापस newmem;
  } अन्यथा {
    VALGRIND_RESIZEINPLACE_BLOCK(oldmem, 0, bytes, आकार_SZ);
    VALGRIND_MAKE_MEM_DEFINED(oldmem, bytes);
  }

 split:  /* split off extra room in old or expanded chunk */

  अगर (newsize - nb >= MINSIZE) /* split off remainder */
  {
    remainder = chunk_at_offset(newp, nb);
    remainder_आकार = newsize - nb;
    set_शीर्ष_आकार(newp, nb);
    set_शीर्ष(remainder, remainder_आकार | पूर्व_INUSE);
    set_inuse_bit_at_offset(remainder, remainder_आकार);
    VALGRIND_MALLOCLIKE_BLOCK(chunk2mem(remainder), remainder_आकार, आकार_SZ,
			      false);
    fREe_impl(chunk2mem(remainder)); /* let free() deal with it */
  }
  अन्यथा
  {
    set_शीर्ष_आकार(newp, newsize);
    set_inuse_bit_at_offset(newp, newsize);
  }

  check_inuse_chunk(newp);
  वापस chunk2mem(newp);
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
#अगर __STD_C
Void_t* mEMALIGn_impl(माप_प्रकार alignment, माप_प्रकार bytes)
#अन्यथा
Void_t* mEMALIGn_impl(alignment, bytes) माप_प्रकार alignment; माप_प्रकार bytes;
#पूर्ण_अगर
{
  INTERNAL_आकार_T    nb;      /* padded  request size */
  अक्षर*     m;                /* memory returned by malloc call */
  mchunkptr p;                /* corresponding chunk */
  अक्षर*     brk;              /* alignment point within p */
  mchunkptr newp;             /* chunk to return */
  INTERNAL_आकार_T  newsize;   /* its size */
  INTERNAL_आकार_T  leadsize;  /* leading space befor alignment point */
  mchunkptr remainder;        /* spare room at end to split off */
  दीर्घ      remainder_आकार;   /* its size */

  अगर (bytes > CONFIG_SYS_MALLOC_LEN || (दीर्घ)bytes < 0)
     वापस शून्य;

#अगर CONFIG_IS_ENABLED(SYS_MALLOC_F)
	अगर (!(gd->flags & GD_FLG_FULL_MALLOC_INIT)) {
		वापस memalign_simple(alignment, bytes);
	}
#पूर्ण_अगर

  /* If need less alignment than we give anyway, just relay to malloc */

  अगर (alignment <= MALLOC_ALIGNMENT) वापस mALLOc_impl(bytes);

  /* Otherwise, ensure that it is at least a minimum chunk size */

  अगर (alignment <  MINSIZE) alignment = MINSIZE;

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
  अगर (m == शून्य) {
    माप_प्रकार extra, extra2;
    /*
     * Use bytes not nb, since mALLOc internally calls request2size too, and
     * each call increases the size to allocate, to account for the header.
     */
    m  = (अक्षर*)(mALLOc_impl(bytes));
    /* Aligned -> return it */
    अगर ((((अचिन्हित दीर्घ)(m)) % alignment) == 0)
      वापस m;
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
    अगर (m) {
      extra2 = alignment - (((अचिन्हित दीर्घ)(m)) % alignment);
      अगर (extra2 > extra) {
        fREe_impl(m);
        m = शून्य;
      }
    }
    /* Fall through to original NULL check and chunk splitting logic */
  }

  अगर (m == शून्य) वापस शून्य; /* propagate failure */

  p = mem2chunk(m);

  अगर ((((अचिन्हित दीर्घ)(m)) % alignment) == 0) /* aligned */
  {
#अगर HAVE_MMAP
    अगर(chunk_is_mmapped(p))
      वापस chunk2mem(p); /* nothing more to do */
#पूर्ण_अगर
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
    अगर ((दीर्घ)(brk - (अक्षर*)(p)) < MINSIZE) brk = brk + alignment;

    newp = (mchunkptr)brk;
    leadsize = brk - (अक्षर*)(p);
    newsize = chunksize(p) - leadsize;

#अगर HAVE_MMAP
    अगर(chunk_is_mmapped(p))
    {
      newp->पूर्व_आकार = p->पूर्व_आकार + leadsize;
      set_शीर्ष(newp, newsize|IS_MMAPPED);
      वापस chunk2mem(newp);
    }
#पूर्ण_अगर

    /* give back leader, use the rest */

    set_शीर्ष(newp, newsize | पूर्व_INUSE);
    set_inuse_bit_at_offset(newp, newsize);
    set_शीर्ष_आकार(p, leadsize);
    fREe_impl(chunk2mem(p));
    p = newp;
    VALGRIND_MALLOCLIKE_BLOCK(chunk2mem(p), bytes, आकार_SZ, false);

    निश्चित (newsize >= nb && (((अचिन्हित दीर्घ)(chunk2mem(p))) % alignment) == 0);
  }

  /* Also give back spare room at the end */

  remainder_आकार = chunksize(p) - nb;

  अगर (remainder_आकार >= (दीर्घ)MINSIZE)
  {
    remainder = chunk_at_offset(p, nb);
    set_शीर्ष(remainder, remainder_आकार | पूर्व_INUSE);
    set_शीर्ष_आकार(p, nb);
    VALGRIND_MALLOCLIKE_BLOCK(chunk2mem(remainder), remainder_आकार, आकार_SZ,
			      false);
    fREe_impl(chunk2mem(remainder));
  }

  check_inuse_chunk(p);
  वापस chunk2mem(p);

}

/*
    valloc just invokes memalign with alignment argument equal
    to the page size of the system (or as near to this as can
    be figured out from all the includes/defines above.)
*/

#अगर __STD_C
Void_t* vALLOc(माप_प्रकार bytes)
#अन्यथा
Void_t* vALLOc(bytes) माप_प्रकार bytes;
#पूर्ण_अगर
{
  वापस mEMALIGn (malloc_getpagesize, bytes);
}

/*
  pvalloc just invokes valloc for the nearest pagesize
  that will accommodate request
*/

#अगर __STD_C
Void_t* pvALLOc(माप_प्रकार bytes)
#अन्यथा
Void_t* pvALLOc(bytes) माप_प्रकार bytes;
#पूर्ण_अगर
{
  माप_प्रकार pagesize = malloc_getpagesize;
  वापस mEMALIGn (pagesize, (bytes + pagesize - 1) & ~(pagesize - 1));
}

/*

  calloc calls malloc, then zeroes out the allocated chunk.

*/

STATIC_IF_MCHECK
#अगर __STD_C
Void_t* cALLOc_impl(माप_प्रकार n, माप_प्रकार elem_आकार)
#अन्यथा
Void_t* cALLOc_impl(n, elem_आकार) माप_प्रकार n; माप_प्रकार elem_आकार;
#पूर्ण_अगर
{
  mchunkptr p;
  INTERNAL_आकार_T csz;

  INTERNAL_आकार_T sz = n * elem_आकार;

  /* check if expand_top called, in which case don't need to clear */
#अगर CONFIG_IS_ENABLED(SYS_MALLOC_CLEAR_ON_INIT)
#अगर MORECORE_CLEARS
  mchunkptr oldtop = शीर्ष;
  INTERNAL_आकार_T oldtopsize = chunksize(शीर्ष);
#पूर्ण_अगर
#पूर्ण_अगर
  Void_t* mem = mALLOc_impl (sz);

  अगर ((दीर्घ)n < 0) वापस शून्य;

  अगर (mem == शून्य)
    वापस शून्य;
  अन्यथा
  {
#अगर CONFIG_IS_ENABLED(SYS_MALLOC_F)
	अगर (!(gd->flags & GD_FLG_FULL_MALLOC_INIT)) {
		स_रखो(mem, 0, sz);
		वापस mem;
	}
#पूर्ण_अगर
    p = mem2chunk(mem);

    /* Two optional cases in which clearing not necessary */

#अगर HAVE_MMAP
    अगर (chunk_is_mmapped(p)) वापस mem;
#पूर्ण_अगर

    csz = chunksize(p);

#अगर CONFIG_IS_ENABLED(SYS_MALLOC_CLEAR_ON_INIT)
#अगर MORECORE_CLEARS
    अगर (p == oldtop && csz > oldtopsize)
    {
      /* clear only the bytes from non-freshly-sbrked memory */
      csz = oldtopsize;
    }
#पूर्ण_अगर
#पूर्ण_अगर

    MALLOC_ZERO(mem, csz - आकार_SZ);
    VALGRIND_MAKE_MEM_DEFINED(mem, sz);
    वापस mem;
  }
}

/*

  cfree just calls free. It is needed/defined on some systems
  that pair it with calloc, presumably for odd historical reasons.

*/

#अगर !defined(INTERNAL_LINUX_C_LIB) || !defined(__ELF__)
#अगर __STD_C
व्योम cfree(Void_t *mem)
#अन्यथा
व्योम cfree(mem) Void_t *mem;
#पूर्ण_अगर
{
  fREe(mem);
}
#पूर्ण_अगर

#अगर_घोषित MCHECK_HEAP_PROTECTION
 #समावेश "mcheck_core.inc.h"
 #अगर !__STD_C
  #त्रुटि "must have __STD_C"
 #पूर्ण_अगर

Void_t *mALLOc(माप_प्रकार bytes)
{
	mcheck_pedantic_prehook();
	माप_प्रकार fullsz = mcheck_alloc_prehook(bytes);
	व्योम *p = mALLOc_impl(fullsz);

	अगर (!p)
		वापस p;
	वापस mcheck_alloc_posthook(p, bytes);
}

व्योम fREe(Void_t *mem) { fREe_impl(mcheck_free_prehook(mem)); }

Void_t *rEALLOc(Void_t *oldmem, माप_प्रकार bytes)
{
	mcheck_pedantic_prehook();
	अगर (bytes == 0) {
		अगर (oldmem)
			fREe(oldmem);
		वापस शून्य;
	}

	अगर (oldmem == शून्य)
		वापस mALLOc(bytes);

	व्योम *p = mcheck_reallocfree_prehook(oldmem);
	माप_प्रकार newsz = mcheck_alloc_prehook(bytes);

	p = rEALLOc_impl(p, newsz);
	अगर (!p)
		वापस p;
	वापस mcheck_alloc_noclean_posthook(p, bytes);
}

Void_t *mEMALIGn(माप_प्रकार alignment, माप_प्रकार bytes)
{
	mcheck_pedantic_prehook();
	माप_प्रकार fullsz = mcheck_memalign_prehook(alignment, bytes);
	व्योम *p = mEMALIGn_impl(alignment, fullsz);

	अगर (!p)
		वापस p;
	वापस mcheck_memalign_posthook(alignment, p, bytes);
}

// pvALLOc, vALLOc - redirect to mEMALIGn, defined here, so they need no wrapping.

Void_t *cALLOc(माप_प्रकार n, माप_प्रकार elem_आकार)
{
	mcheck_pedantic_prehook();
	// NB: here is no overflow check.
	माप_प्रकार fullsz = mcheck_alloc_prehook(n * elem_आकार);
	व्योम *p = cALLOc_impl(1, fullsz);

	अगर (!p)
		वापस p;
	वापस mcheck_alloc_noclean_posthook(p, n * elem_आकार);
}

// mcheck API {
पूर्णांक mcheck_pedantic(mcheck_abortfunc_t f)
{
	mcheck_initialize(f, 1);
	वापस 0;
}

पूर्णांक mcheck(mcheck_abortfunc_t f)
{
	mcheck_initialize(f, 0);
	वापस 0;
}

व्योम mcheck_check_all(व्योम) { mcheck_pedantic_check(); }

क्रमागत mcheck_status mprobe(व्योम *__ptr) { वापस mcheck_mprobe(__ptr); }
// mcheck API }
#पूर्ण_अगर

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

#अगर __STD_C
पूर्णांक malloc_trim(माप_प्रकार pad)
#अन्यथा
पूर्णांक malloc_trim(pad) माप_प्रकार pad;
#पूर्ण_अगर
{
  दीर्घ  शीर्ष_आकार;        /* Amount of top-most memory */
  दीर्घ  extra;           /* Amount to release */
  अक्षर* current_brk;     /* address returned by pre-check sbrk call */
  अक्षर* new_brk;         /* address returned by negative sbrk call */

  अचिन्हित दीर्घ pagesz = malloc_getpagesize;

  शीर्ष_आकार = chunksize(शीर्ष);
  extra = ((शीर्ष_आकार - pad - MINSIZE + (pagesz-1)) / pagesz - 1) * pagesz;

  अगर (extra < (दीर्घ)pagesz)  /* Not enough memory to release */
    वापस 0;

  अन्यथा
  {
    /* Test to make sure no one else called sbrk */
    current_brk = (अक्षर*)(MORECORE (0));
    अगर (current_brk != (अक्षर*)(शीर्ष) + शीर्ष_आकार)
      वापस 0;     /* Apparently we don't own memory; must fail */

    अन्यथा
    {
      new_brk = (अक्षर*)(MORECORE (-extra));

      अगर (new_brk == (अक्षर*)(MORECORE_FAILURE)) /* sbrk failed? */
      {
	/* Try to figure out what we have */
	current_brk = (अक्षर*)(MORECORE (0));
	शीर्ष_आकार = current_brk - (अक्षर*)शीर्ष;
	अगर (शीर्ष_आकार >= (दीर्घ)MINSIZE) /* if not, we are very very dead! */
	{
	  sbrked_mem = current_brk - sbrk_base;
	  set_शीर्ष(शीर्ष, शीर्ष_आकार | पूर्व_INUSE);
	}
	check_chunk(शीर्ष);
	वापस 0;
      }

      अन्यथा
      {
	/* Success. Adjust top accordingly. */
	set_शीर्ष(शीर्ष, (शीर्ष_आकार - extra) | पूर्व_INUSE);
	sbrked_mem -= extra;
	check_chunk(शीर्ष);
	वापस 1;
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

#अगर __STD_C
माप_प्रकार malloc_usable_size(Void_t* mem)
#अन्यथा
माप_प्रकार malloc_usable_size(mem) Void_t* mem;
#पूर्ण_अगर
{
  mchunkptr p;
  अगर (mem == शून्य)
    वापस 0;
  अन्यथा
  {
    p = mem2chunk(mem);
    अगर(!chunk_is_mmapped(p))
    {
      अगर (!inuse(p)) वापस 0;
      check_inuse_chunk(p);
      वापस chunksize(p) - आकार_SZ;
    }
    वापस chunksize(p) - 2*आकार_SZ;
  }
}

/* Utility to update current_mallinfo for malloc_stats and mallinfo() */

#अगर_घोषित DEBUG
अटल व्योम malloc_update_mallinfo(व्योम)
{
  पूर्णांक i;
  mbinptr b;
  mchunkptr p;
#अगर_घोषित DEBUG
  mchunkptr q;
#पूर्ण_अगर

  INTERNAL_आकार_T avail = chunksize(शीर्ष);
  पूर्णांक   navail = ((दीर्घ)(avail) >= (दीर्घ)MINSIZE)? 1 : 0;

  क्रम (i = 1; i < NAV; ++i)
  {
    b = bin_at(i);
    क्रम (p = last(b); p != b; p = p->bk)
    {
#अगर_घोषित DEBUG
      check_free_chunk(p);
      क्रम (q = अग्र_chunk(p);
	   q < शीर्ष && inuse(q) && (दीर्घ)(chunksize(q)) >= (दीर्घ)MINSIZE;
	   q = अग्र_chunk(q))
	check_inuse_chunk(q);
#पूर्ण_अगर
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
#पूर्ण_अगर	/* DEBUG */

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

#अगर_घोषित DEBUG
व्योम malloc_stats(व्योम)
{
  malloc_update_mallinfo();
  म_लिखो("max system bytes = %10u\n",
	  (अचिन्हित पूर्णांक)(max_योग_mem));
  म_लिखो("system bytes     = %10u\n",
	  (अचिन्हित पूर्णांक)(sbrked_mem + mmapped_mem));
  म_लिखो("in use bytes     = %10u\n",
	  (अचिन्हित पूर्णांक)(current_mallinfo.uordblks + mmapped_mem));
#अगर HAVE_MMAP
  म_लिखो("max mmap regions = %10u\n",
	  (अचिन्हित पूर्णांक)max_n_mmaps);
#पूर्ण_अगर
}
#पूर्ण_अगर	/* DEBUG */

/*
  mallinfo returns a copy of updated current mallinfo.
*/

#अगर_घोषित DEBUG
काष्ठा mallinfo mALLINFo(व्योम)
{
  malloc_update_mallinfo();
  वापस current_mallinfo;
}
#पूर्ण_अगर	/* DEBUG */

/*
  mallopt:

    mallopt is the general SVID/XPG interface to tunable parameters.
    The format is to provide a (parameter-number, parameter-value) pair.
    mallopt then sets the corresponding parameter to the argument
    value if it can (i.e., so long as the value is meaningful),
    and returns 1 if successful else 0.

    See descriptions of tunable parameters above.

*/

#अगर __STD_C
पूर्णांक mALLOPt(पूर्णांक param_संख्या, पूर्णांक मूल्य)
#अन्यथा
पूर्णांक mALLOPt(param_संख्या, मूल्य) पूर्णांक param_संख्या; पूर्णांक मूल्य;
#पूर्ण_अगर
{
  चयन(param_संख्या)
  {
    हाल M_TRIM_THRESHOLD:
      trim_threshold = मूल्य; वापस 1;
    हाल M_TOP_PAD:
      शीर्ष_pad = मूल्य; वापस 1;
    हाल M_MMAP_THRESHOLD:
      mmap_threshold = मूल्य; वापस 1;
    हाल M_MMAP_MAX:
#अगर HAVE_MMAP
      n_mmaps_max = मूल्य; वापस 1;
#अन्यथा
      अगर (मूल्य != 0) वापस 0; अन्यथा  n_mmaps_max = मूल्य; वापस 1;
#पूर्ण_अगर

    शेष:
      वापस 0;
  }
}

पूर्णांक initf_malloc(व्योम)
{
#अगर CONFIG_IS_ENABLED(SYS_MALLOC_F)
	निश्चित(gd->malloc_base);	/* Set up by crt0.S */
	gd->malloc_limit = CONFIG_VAL(SYS_MALLOC_F_LEN);
	gd->malloc_ptr = 0;
#पूर्ण_अगर

	वापस 0;
}

व्योम malloc_enable_testing(पूर्णांक max_allocs)
{
	malloc_testing = true;
	malloc_max_allocs = max_allocs;
}

व्योम malloc_disable_testing(व्योम)
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
