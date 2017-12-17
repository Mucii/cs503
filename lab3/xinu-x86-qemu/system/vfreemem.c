/* vfreemem.c - vfreemem */

#include <xinu.h>

syscall	vfreemem(
	  char		*blkaddr,	/* Pointer to memory block	*/
	  uint32	nbytes		/* Size of block in bytes	*/
	)
{
  // Lab3 TODO.

	intmask	mask;			/* Saved interrupt mask		*/
	struct	memblk	*next, *prev, *block;
	uint32	top;

	mask = disable();

	struct procent *prptr = &proctab[currpid];

	// check whether nbytes != 0 or heap is not in private pages
	if ((nbytes == 0) || ((uint32) blkaddr < (uint32)(NBPG * 4096))
			  || ((uint32) blkaddr > (uint32)((prptr->vsize + 4096) * NBPG))){
		restore(mask);
		return SYSERR;
	}

	nbytes = (uint32) roundmb(nbytes);	/* Use memblk multiples	*/
	block = (struct memblk *)blkaddr;

	prev = &prptr->prvmem;			/* Walk along free list	*/
	next = prev->mnext;

	while ((next != NULL) && (next < block)) {
		prev = next;
		next = next->mnext;
	}

	if (prev == &prptr->prvmem) {		/* Compute top of previous block*/
		top = (uint32) NULL;
	} else {
		top = (uint32) prev + prev->mlength;
	}

	if (((prev != &prptr->prvmem) && (uint32) block < top)
	    || ((next != NULL)	&& (uint32) block+nbytes>(uint32)next)) {
		restore(mask);
		return SYSERR;
	}

	prptr->prvmem.mlength += nbytes;


	/* Either coalesce with previous block or add to free list */

	if (top == (uint32) block) { 	/* Coalesce with previous block	*/
		prev->mlength += nbytes;
		block = prev;
	} else {			/* Link into list as new node	*/
		block->mnext = next;
		block->mlength = nbytes;
		prev->mnext = block;
	}

	/* Coalesce with next block if adjacent */

	if (((uint32) block + block->mlength) == (uint32) next) {
		block->mlength += next->mlength;
		block->mnext = next->mnext;
	}

	restore(mask);
	return OK;
}
