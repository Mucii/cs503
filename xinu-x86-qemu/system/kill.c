/* kill.c - kill */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  kill  -  Kill a process and remove it from the system
 *------------------------------------------------------------------------
 */
syscall	kill(
	  pid32		pid		/* ID of process to kill	*/
	)
{
	intmask	mask;			/* Saved interrupt mask		*/
	struct	procent *prptr;		/* Ptr to process' table entry	*/
	int32	i;			/* Index into descriptors	*/

	mask = disable();
	if (isbadpid(pid) || (pid == NULLPROC)
	    || ((prptr = &proctab[pid])->prstate) == PR_FREE) {
		restore(mask);
		return SYSERR;
	}

	//kprintf("KILL : %d\n", pid);
	if (--prcount <= 1) {		/* Last user process completes	*/
		xdone();
	}
	
	send(prptr->prparent, pid);
	for (i=0; i<3; i++) {
		close(prptr->prdesc[i]);
	}

  // Lab3 TODO. Free frames as a process gets killed.
	// Frames should be released
	frame_t *frame_now;
	inverted_page *inverted_page_now;
	bs_map *curr_bs_map;
	for(i=0; i<NFRAMES; i++){
		//find frame
		inverted_page_now = &inverted_page_tab[i];
		frame_now = &frame_tab[i];
		if(inverted_page_now->pid == pid){
			//write back frame to bs
			if(frame_now->type == FRAME_PG){
				curr_bs_map = get_bs_map(pid, inverted_page_now->vpn);
				//kprintf("inverted vpn%d\n",inverted_page_now->vpn);
				//kprintf("bs map vpn%d\n",curr_bs_map->vpn);
				//kprintf("bs id%d\n",curr_bs_map->bs_id);
				open_bs(curr_bs_map->bs_id);
				write_bs((char*)((FRAME0 + i) * NBPG), curr_bs_map->bs_id, inverted_page_now->vpn - curr_bs_map->vpn);
				close_bs(curr_bs_map->bs_id);
			}

			// remove frame from fifo list
			if(currpolicy == FIFO){
				rm_frame_fifo(i);
			}

			// clean frame and inverted map
			// initial frame
			frame_now = &frame_tab[i];
			frame_now->frame_id = i;
			frame_now->state = FRAME_FREE;
			frame_now->type = -1;
			frame_now->next_frame = (frame_t *)NULL;


			// intial inverted page table
			inverted_page_now = &inverted_page_tab[i];
			inverted_page_now->refcount = 0;
			inverted_page_now->pid = -1;
			inverted_page_now->vpn = 0;

		}
	}

	// bs map should be released

	for(i=0; i<MAX_BS_ENTRIES; i++){
		if(bs_map_tab[i].pid == pid){
			close_bs(i);
            deallocate_bs(i);
			bs_map_tab[i].pid = -1;
			bs_map_tab[i].vpn = 0;
			bs_map_tab[i].npg = 0;
			bs_map_tab[i].bs_id = -1;
			bs_map_tab[i].allocated = FALSE;
		}
	}

	freestk(prptr->prstkbase, prptr->prstklen);

	switch (prptr->prstate) {
	case PR_CURR:
		prptr->prstate = PR_FREE;	/* Suicide */
		resched();

	case PR_SLEEP:
	case PR_RECTIM:
		unsleep(pid);
		prptr->prstate = PR_FREE;
		break;

	case PR_WAIT:
		semtab[prptr->prsem].scount++;
		/* Fall through */

	case PR_READY:
		getitem(pid);		/* Remove from queue */
		/* Fall through */

	default:
		prptr->prstate = PR_FREE;
	}

	restore(mask);
	return OK;
}
