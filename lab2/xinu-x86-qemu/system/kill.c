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
	struct  pipe_t *pipe;


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


	// this is to make sure we can have lastes pid which works in my shell.c
	if(!isbadpid(prptr->prparent)){
		if(proctab[prptr->prparent].prmsg<pid){
			proctab[prptr->prparent].prmsg=pid;
		}
	}

	for (i=0; i<3; i++) {
		close(prptr->prdesc[i]);
	}

	// check the owner
	for(pipid32 i = 0; i<MAXPIPES; i++){
		pipe = &pipe_tables[i];
		if(pipe->owner == pid){
			pipe->state = PIPE_FREE;
    		pipe->writer = -1;
    		pipe->reader = -1;
    		pipe->writerid = 0;
    		pipe->readerid = 0;
    		semdelete(pipe->writersem);
    		semdelete(pipe->readersem);
		} else if (pipe->reader==pid || pipe->writer == pid){
			//check the state and change it
   			if(pipe->state == PIPE_OTHER){
   				cleanup(i);
   				// this should not work because it has been cleaned
   			}else{
   				pipe->state = PIPE_OTHER;
   			}

   			if(pipe->writer == pid){
   				// disconnnect writer
   				pipe->writer = -1;
   				// signal the read process
   				if(semcount(pipe->readersem)<0){
   					signal(pipe->readersem);
   				}
   			}else{
   				// disconnnect writer
   				pipe->reader = -1;
   				// signal the read process
   				if(semcount(pipe->writersem)<0){
   					signal(pipe->writersem);
   				}
   			}
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
