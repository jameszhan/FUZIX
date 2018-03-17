#include <kernel.h>
#include <printf.h>

/*
 *	This is inspired by Dale Schumacher's public domain dlibs, but rewritten
 *	from scratch. It's a very simple, very compact and surprisingly efficient
 *	malloc/free/memavail
 *
 *	These functions should *not* be used by core kernel code. They are
 *	used to support flat address space machines. They can be used by 32bit
 *	specific code and drivers but care should be taken to avoid fragmentation
 *	and leaks.
 */

/*
 *	MAX_POOLS is the number of discontiguous memory pools we have.
 */

#if defined(CONFIG_32BIT)

#undef DEBUG_MEMORY

struct block
{
	struct block *next;
	size_t length; /* high bit set if used */
};

#define UNUSED(b) (!((b)->length & (1L<<31)))

static struct block start = { NULL, sizeof(struct block) };

/*
 * Add a memory block to the pool. Must be aligned to the alginment boundary
 * in use.
 */
void kmemaddblk(void *base, size_t size)
{
	struct block *b = &start;
	struct block *n = (struct block *) base;

	/* Run down the list until we find the last block. */

	while (b->next)
		b = b->next;

	/* Add the block. */

	b->next = n;
	n->next = NULL;
	n->length = size;
#if defined(DEBUG_MEMORY)
	kprintf("mem: add %x+%x\n", n, n->length);
#endif
}

/*
 * Find the smallest unused block containing at least length bytes.
 */
static struct block *find_smallest(size_t length)
{
	static struct block dummy = { NULL, 0x7fffffff };
	struct block *smallest = &dummy;
	struct block *b = &start;

	while (b->next)
	{
		if (UNUSED(b)
			&& (b->length >= length)
			&& (b->length < smallest->length))
		{
			smallest = b;
		}

		b = b->next;
	}

	return (b == &dummy) ? NULL : b;
}

/*
 * Split the supplied block into a used section and an unused section
 * immediately following it (if big enough).
 */
static void split_block(struct block *b, size_t size)
{
	int32_t newsize = b->length - size; /* might be negative */
	if (newsize > sizeof(struct block) * 4)
	{
		struct block *n = (struct block *)((uint8_t *)b + size);
#if defined(DEBUG_MEMORY)
		kprintf("mem: split %x+%x -> ", b, b->length);
#endif
		n->next = b->next;
		b->next = n;
		b->length = size;
		n->length = newsize;
#if defined(DEBUG_MEMORY)
		kprintf("%x+%x and %x+%x\n", b, b->length, n, n->length);
#endif
	}

	b->length |= 0x80000000;
}

/*
 * Allocate a block.
 */
void *kmalloc(size_t size)
{
	struct block *b;

	size = ALIGNUP(size) + sizeof(struct block);
	b = find_smallest(size);
	if (!b)
		return NULL;

	split_block(b, size);
#if defined(DEBUG_MEMORY)
	kprintf("mem: alloc %x+%x\n", b, b->length);
#endif
	return b + 1;
}

/*
 * Merge all adjacent free blocks in the chain.
 */
static void merge_all_blocks(void)
{
	struct block *b = &start;

	while (b->next)
	{
		struct block *n = b->next;

		if (UNUSED(b)
			&& UNUSED(n)
			&& (((uint8_t*)b + b->length) == (uint8_t*)n))
		{
			/* Two mergeable blocks are adjacent. */
#if defined(DEBUG_MEMORY)
			kprintf("mem: merge %x+%x and %x+%x -> ",
				b, b->length, n, n->length);
#endif
			b->next = n->next;
			b->length += n->length;
#if defined(DEBUG_MEMORY)
			kprintf("%x+%x\n", b, b->length);
#endif
		}
		else
		{
			/* Only move on to the next block if we're unable to merge
			 * this one. */
			b = n;
		}
	}
}

/*
 * Free a block. If there's a block immediately after this one, merge it.
 * (This maintains ordering within split blocks.)
 */
void kfree(void *p)
{
	struct block *b = (struct block *)p - 1;
#if defined(DEBUG_MEMORY)
	kprintf("mem: free %x+%x\n", b, b->length);
#endif

	if (UNUSED(b))
		panic(PANIC_BADFREE);
	
	b->length &= 0x7fffffff;
	merge_all_blocks();
}

#endif
