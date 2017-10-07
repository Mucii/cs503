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
    // LAB2: TODO: Modify this function to clean-up the pipe.

	intmask	mask;			/* Saved interrupt mask		*/
	struct	procent *prptr;		/* Ptr to process's table entry	*/
	int32	i;			/* Index into descriptors	*/
	pipid32 pipid;
	struct  pipe_t *pipe;
	struct procent *proc;

	mask = disable();
	if (isbadpid(pid) || (pid == NULLPROC)
	    || ((prptr = &proctab[pid])->prstate) == PR_FREE) {
		restore(mask);
		return SYSERR;
	}

	if (--prcount <= 1) {		/* Last user process completes	*/
		xdone();
	}

	send(prptr->prparent, pid);
	for (i=0; i<3; i++) {
		//check whether the process is the writer or reader
		if(prptr->prdesc[i] >= PIPELINE0){
			pipid = did32_to_pipid32(prptr->prdesc[i]);
			pipe = &pipe_tables[pipid];
			
			// this can only happen when otherside has alreadly finished

			if(pipe->writer!=pid){
				proc = &proctab[pipe->writer];
				if(proc->prstate == PR_FREE){ 
					pipe->state=PIPE_OTHER;
					pipdisconnect(prptr->prdesc[i]);
				}
			}

			if(pipe->reader!=pid){
				proc = &proctab[pipe->reader];
				if(proc->prstate == PR_FREE){
					pipe->state=PIPE_OTHER;
					pipdisconnect(prptr->prdesc[i]);
				}
			}
		}
		close(prptr->prdesc[i]);
	}

	// check the owner
	for(pipid32 i = 0; i<MAXPIPES; i++){
		if(pipe_tables[i].owner == pid){
			pipdelete(pipid32_to_did32(i));
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
