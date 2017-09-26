/* resched.c - resched, resched_cntl */

#include <xinu.h>

struct	defer	Defer;

static int tstab[60][3]={
{200, 0, 50},
{200, 0, 50},
{200, 0, 50},
{200, 0, 50},
{200, 0, 50},
{200, 0, 50},
{200, 0, 50},
{200, 0, 50},
{200, 0, 50},
{200, 0, 50},
{160, 0, 51},
{160, 1, 51},
{160, 2, 51},
{160, 3, 51},
{160, 4, 51},
{160, 5, 51},
{160, 6, 51},
{160, 7, 51},
{160, 8, 51},
{160, 9, 51},
{120, 10, 52},
{120, 11, 52},
{120, 12, 52},
{120, 13, 52},
{120, 14, 52},
{120, 15, 52},
{120, 16, 52},
{120, 17, 52},
{120, 18, 52},
{120, 19, 52},
{80, 20, 53},
{80, 21, 53},
{80, 22, 53},
{80, 23, 53},
{80, 24, 53},
{80, 25, 54},
{80, 26, 54},
{80, 27, 54},
{80, 28, 54},
{80, 29, 54},
{40, 30, 55},
{40, 31, 55},
{40, 32, 55},
{40, 33, 55},
{40, 34, 55},
{40, 35, 56},
{40, 36, 57},
{40, 37, 58},
{40, 38, 58},
{40, 39, 58},
{40, 40, 58},
{40, 41, 58},
{40, 42, 58},
{40, 43, 58},
{40, 44, 58},
{40, 45, 58},
{40, 46, 58},
{40, 47, 58},
{40, 48, 58},
{20, 49, 59}
};


/*------------------------------------------------------------------------
 *  aginscheduling  -  Reschedule processor to by aging scheduling
 *  *------------------------------------------------------------------------
 */
static pid32 aginscheduling(void){


	pid32 index; /* index of process we will go over */
	struct procent *ptnow; /* process we will go over */
	struct procent *ptold; /* current process*/
	int pscount; /* counts of ps process */
	int tscount; /* counts of ts process */

	/* initialize */

	pscount=0;
	tscount=0;

	/* first step of aging sche*/

	ptold=&proctab[currpid];
	if(ptold->prgroup==PROPORTIONALSHARE){
		psprio=psprioin; 
	}else{
		tsprio=tsprioin;
	}
	
	index=queuetab[queuehead(readylist)].qnext;

	/* second step of aging sche, queue is incluede in queue.h */
	while(index!=queuetail(readylist)){
		
		ptnow=&proctab[index];
		/* sheduling according to TS or PS */
		if(ptnow->prgroup==PROPORTIONALSHARE){
			if(index!=NULLPROC && index!=currpid){
				pscount+=1;
			}
		}else if(ptnow->prgroup==TSSCHED){
			if(index!=NULLPROC && index!=currpid){
				tscount+=1;
			}
		}
		index=queuetab[index].qnext;
	}

	psprio+=pscount;
	tsprio+=tscount;
	
	//kprintf("tscount is %d\n\n",tscount);
	//kprintf("pscount is %d\n\n",pscount);

	if(pscount*tscount==0){
		if(tscount!=0){
			return TSSCHED;
		}else{
			return PROPORTIONALSHARE; //NULLPROC is PROPORTIONALSHARE
		}
	}

	if(psprio>=tsprio){
		return PROPORTIONALSHARE;
	}else{
		return TSSCHED;
	}

}

static pid32 findid(int group){

	pid32 index; /* index of process we will go over */
	struct procent *ptnow; /* process we will go over */
	
	if(isempty(readylist)){
		return EMPTY;
	}

	index=queuetab[queuehead(readylist)].qnext;

	ptnow=&proctab[index];
	
	while(ptnow->prgroup!=group){
		index=queuetab[index].qnext;
		ptnow=&proctab[index];
	}

	return getitem(index); //remove current id from readylist
}

static void oldps(struct procent *ptold){

	/* set pi and prio*/
	ptold->pspi=ptold->pspi+(QUANTUM-preempt)*(100/ptold->psrate);
	ptold->prprio=2147483647-ptold->pspi;
	/* set block status*/
	if(preempt<=0 || ptold->prstate==PR_CURR || ptold->prstate==PR_READY){
		ptold->psblock=UNBLOCKED;
	}else{
		ptold->psblock=BLOCKED;
	}
	return;
}

static void oldts(struct procent *ptold){
	//kprintf("%d\n\n",ptold->prprio);
	/* check IO or CPU and change priority*/
	if(preempt<=0){
		ptold->prprio=tstab[ptold->prprio][1];
	}else{
		ptold->prprio=tstab[ptold->prprio][2];
	}
	//kprintf("%d\n\n",ptold->prprio);

	return;
}

static void newps(struct procent *ptnew){
	if(ptnew->psblock==BLOCKED){
		int T = clktime*1000+(1000-count1000);
		if(T>ptnew->pspi){
			ptnew->pspi=T;
			ptnew->prprio=2147483647-T;
		}
	}
	preempt = QUANTUM;

	return;
}

static void newts(struct procent *ptnew){
	if(ptnew->tsnew==TSFIRST){
		preempt=QUANTUM;
		ptnew->tsnew=TSSECOND;
	}else{
		preempt=tstab[ptnew->prprio][0];
	}
	return;
}

/*------------------------------------------------------------------------
 *  resched  -  Reschedule processor to highest priority eligible process
 *------------------------------------------------------------------------
 */
void	resched(void)		/* Assumes interrupts are disabled	*/
{
	struct procent *ptold;	/* Ptr to table entry for old process	*/
	struct procent *ptnew;	/* Ptr to table entry for new process	*/
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
		oldps(ptold);
	}else{
		oldts(ptold);
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

	currpid = findid(aginscheduling());
	ptnew = &proctab[currpid];

	//kprintf("preempt is %d\n\n", preempt);
	/* Third step of PS scheduling*/
	if(ptnew->prgroup==PROPORTIONALSHARE && currpid!=NULLPROC){
		newps(ptnew);
	}else{
		newts(ptnew);
	}


	ptnew->prstate = PR_CURR;

	//kprintf("process is %s \n\n",ptnew->prname);

	//kprintf("prio is %d\n\n", ptnew->prprio);

	//kprintf("psprio is %d\n\n",psprio);

	//kprintf("tsprio is %d\n\n",tsprio);

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
