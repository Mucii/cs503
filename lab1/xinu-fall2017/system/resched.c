/* resched.c - resched, resched_cntl */

#include <xinu.h>

struct	defer	Defer;

/*------------------------------------------------------------------------
 *  aginscheduling  -  Reschedule processor to by aging scheduling
 *  *------------------------------------------------------------------------
 */
local pid32 aginscheduling(void){


	pid32 index; /* index of process we will go over */
	struct procent *ptnow; /* process we will go over */
	struct procent *ptold; /* current process*/
	int pscount; /* counts of ps process */
	int tscount; /* counts of ts process */
	pid32 psid; /* id of ps process to return */
	pid32 tsid; /* id of ts process to return */
	pri16 pslocal; /* ps prio */
	pri16 tslocal; /* ts prio */

	/* initialize */

	pscount=0;
	tscount=0;
	psid=NULLPROC;
	tsid=NULLPROC;
	tslocal=-1;
	pslocal=32767;

	/* first step of aging sche*/

	ptold=&proctab[currpid];
	if(ptold->prgroup==PROPORTIONALSHARE){
		psprio=psprioin; 
	}else{
		tsprio=tsprioin;
	}
	
	/* second step of aging sche, queue is incluede in queue.h */
	for(index=queuetab[queuehead(readylist)].qnext;index!=queuetail(readylist);index=queuetab[index].qnext){
		
		ptnow=&proctab[index];
		/* sheduling according to TS or PS */
		if(ptnow->prgroup==PROPORTIONALSHARE){
			if(index!=NULLPROC && index!=currpid){
				pscount+=1;
			}
			if(ptnow->pspi<pslocal && index!=NULLPROC){
				psid=index;
				pslocal=ptnow->pspi;
			}
		}else if(ptnow->prgroup==TSSCHED){
			if(index!=NULLPROC && index!=currpid){
				tscount+=1;
			}
			if(ptnow->prprio>tslocal && index!=NULLPROC){
				tsid=index;
				tslocal=ptnow->prprio;
			}

		}
	}

	psprio+=pscount;
	tsprio+=tscount;

	//kprintf("tslocal is %d\n\n", tslocal);

	//kprintf("tscount is %d\n\n", tscount);

	//kprintf("currid is %d\n\n",currpid);
	//
	
	//kprintf("psprio is %d\n",pslocal);

	if(psprio>=tsprio){
		if(psid!=NULLPROC){
		return psid;}else{return tsid;}		
	}else{
		if(tsid!=NULLPROC){
		return tsid;}else{return psid;}
	}
}

/*------------------------------------------------------------------------
 *  resched  -  Reschedule processor to highest priority eligible process
 *------------------------------------------------------------------------
 */
void	resched(void)		/* Assumes interrupts are disabled	*/
{
	struct procent *ptold;	/* Ptr to table entry for old process	*/
	struct procent *ptnew;	/* Ptr to table entry for new process	*/
	struct tsupdate *newts; /* update of ts level and quantum */
	pid32 oldpid;

	/* If rescheduling is deferred, record attempt and return */

	if (Defer.ndefers > 0) {
		Defer.attempt = TRUE;
		return;
	}

	/* Point to process table entry for the current (old) process */

	ptold = &proctab[currpid];
	oldpid = currpid;
	//kprintf("this is %d\n\n",preempt);
	/* First step of PS scheduling, change */
	//kprintf("process is %s\n\n",ptold->prname);
	if(ptold->prgroup==PROPORTIONALSHARE && currpid!=NULLPROC){
		
		/* set pi and prio*/
		ptold->pspi=ptold->pspi+(QUANTUM-preempt)*(100/ptold->psrate);
		ptold->prprio=32767-ptold->pspi;
		/* set block status*/
		if(preempt<=0){
			ptold->psblock=UNBLOCKED;
		}else{
			ptold->psblock=BLOCKED;
		}
	}else{
		//kprintf("%d\n\n",ptold->prprio);
		/* check IO or CPU and change priority*/
		newts=&tstable[ptold->prprio];
		if(preempt<=0){
			ptold->prprio=newts->ts_tqexp;
		}else{
			ptold->prprio=newts->ts_slepret;
		}
		//kprintf("%d\n\n",ptold->prprio);
	}

	if (ptold->prstate == PR_CURR) {  /* Process remains eligible */
		
		/*if (ptold->prprio > firstkey(readylist)) {
			return;
		}

		Old process will no longer remain current */

		ptold->prstate = PR_READY;
		insert(currpid, readylist, ptold->prprio);
	}

	/* Force context switch to highest priority ready process */

	currpid = getitem(aginscheduling()); /* remove current id from readylist*/
	ptnew = &proctab[currpid];

	/* Third step of PS scheduling*/
	if(ptnew->prgroup==PROPORTIONALSHARE){
		if(ptnew->psblock==BLOCKED){
			int T = clktime*1000+(1000-count1000);
			if(T>ptnew->pspi){
				ptnew->pspi=T;
				ptnew->prprio=32767-T;
			}
		}
		preempt = QUANTUM;
	}else{
		if(ptnew->tsnew==TSFIRST){
			preempt=QUANTUM;
			ptnew->tsnew=TSSECOND;
		}else{
			newts=&tstable[ptnew->prprio];
			preempt=newts->ts_quantum;
		}
	}


	ptnew->prstate = PR_CURR;

	//kprintf("process is %s\n\n",ptnew->prname);

	//kprintf("prio is %d\n\n", ptnew->prprio);

	//kprintf("psprio is %d\n\n",psprio);

	//kprintf("tsprio is %d\n\n",tsprio);

	//kprintf("process prio is %d\n\n",ptnew->prprio);

	/*kprintf("old process's state: %d\n\n",ptold->prstate);
	kprintf("new process's state: %d\n\n",ptnew->prstate);*/
	//kprintf("process is %d\n\n", ptnew->prgroup);
	/*kprintf("psprio is %d\n\n",psprio);
	kprintf("tsprio is %d\n\n",tsprio);*/

	/*do not need to switch*/
	if(oldpid==currpid && ptold->prstate == PR_CURR){
		return;
	}

	ctxsw(&ptold->prstkptr, &ptnew->prstkptr);
	/* Old process returns here when resumed */

	return;
}

/*------------------------------------------------------------------------
 *  resched_cntl  -  Control whether rescheduling is deferred or allowed
 *------------------------------------------------------------------------
 */
status	resched_cntl(		/* Assumes interrupts are disabled	*/
	  int32	defer		/* Either DEFER_START or DEFER_STOP	*/
	)
{
	switch (defer) {

	    case DEFER_START:	/* Handle a deferral request */

		if (Defer.ndefers++ == 0) {
			Defer.attempt = FALSE;
		}
		return OK;

	    case DEFER_STOP:	/* Handle end of deferral */
		if (Defer.ndefers <= 0) {
			return SYSERR;
		}
		if ( (--Defer.ndefers == 0) && Defer.attempt ) {
			resched();
		}
		return OK;

	    default:
		return SYSERR;
	}
}
