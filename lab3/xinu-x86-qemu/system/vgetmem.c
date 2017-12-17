/* vgetmem.c - vgetmem */

#include <xinu.h>

char  	*vgetmem(
	  uint32	nbytes		/* Size of memory requested	*/
	)
{
    intmask mask;
    struct	memblk	*prev, *curr, *leftover;
    struct procent *prptr;
    mask = disable();

    prptr = &proctab[currpid];

    if(nbytes == 0){
    	restore(mask);
		return (char *)SYSERR;
    }

    if(prptr->vcreate == 1){
    	struct memblk *vheap = (struct memblk*) VPN_TO_VD(VPN0);
    	vheap->mlength = (prptr->vsize * NBPG);
    	vheap->mnext = NULL;
    	prptr->vcreate = 0;
    }

    nbytes = (uint32) roundmb(nbytes);	/* Use memblk multiples	*/

    prev = &prptr->prvmem;
	curr = prev->mnext;

	//kprintf("length is %u\n", curr->mlength);
	//kprintf("length 1 is %u\n",prptr->vsize * NBPG);
	while (curr != NULL){
		if (curr->mlength == nbytes) {	/* Block is exact match	*/
			prev->mnext = curr->mnext;
			prptr->prvmem.mlength -= nbytes;
			restore(mask);
			return (char *)(curr);

		} else if (curr->mlength > nbytes) { /* Split big block	*/
			leftover = (struct memblk *)((uint32) curr + nbytes);
			prev->mnext = leftover;
			leftover->mnext = curr->mnext;
			leftover->mlength = curr->mlength - nbytes;
			prptr->prvmem.mlength -= nbytes;
			restore(mask);
			return (char *)(curr);
		} else {			/* Move to next block	*/
			prev = curr;
			curr = curr->mnext;
		}
	}
	restore(mask);
	return (char *)SYSERR;
}
